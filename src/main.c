/* 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "pdm_microphone.pio.h"
#include "pico/stdlib.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+
#define PIO_DATA_PIN      17
#define PIO_CLK_PIN       16
#define SAMPLE_RATE       25000
#define PDM_DECIMATION    64
#define SYS_CLK_MHZ 192
#define RAW_BUFFER_COUNT  2
#define RAW_BUFFER_SIZE   512
/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_SENDING_DATA = 100,
  BLINK_NOT_MOUNTED = 500,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void cdc_task_buf(uint8_t* buf);
void dma_handler(void);
void dma_pio_init(void);
void dma_pio_start(void);

int dma_channel;
uint8_t* raw_buffer[RAW_BUFFER_COUNT];
volatile uint8_t raw_buffer_write_index;
volatile uint8_t raw_buffer_read_index;
volatile bool flag_mic_started = false;
/*------------- MAIN -------------*/
int main(void)
{
  board_init();
  set_sys_clock_khz(SYS_CLK_MHZ * 1000, true);
  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  dma_pio_init();
  dma_pio_start();

  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();
  }

  return 0;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

void tud_cdc_tx_complete_cb(uint8_t itf)
{
  blink_interval_ms = BLINK_MOUNTED;
}



//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void cdc_task_buf(uint8_t* buf)
{
  if ( tud_cdc_connected() )
  {
    blink_interval_ms = BLINK_SENDING_DATA;
    tud_cdc_n_write(0, buf, RAW_BUFFER_SIZE);
    
  }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  (void) itf;
  (void) rts;

  // TODO set some indicator
  if ( dtr )
  {
    // Terminal connected
    
  }else
  {
    blink_interval_ms = BLINK_MOUNTED;
    // Terminal disconnected
  }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
  (void) itf;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}


//--------------------------------------------------------------------+
// DMA + PIO STUFF
//--------------------------------------------------------------------+
void dma_pio_init()
{
  // Set up a PIO state machine to serialise our bits
  uint offset = pio_add_program(pio0, &pdm_microphone_data_program);

  // 4.0 is the number of instructions in PIO Program
  float clk_div = clock_get_hz(clk_sys) / (SAMPLE_RATE * PDM_DECIMATION * 4.0); 

  raw_buffer[0] = malloc(RAW_BUFFER_SIZE * sizeof(uint8_t));
  raw_buffer[1] = malloc(RAW_BUFFER_SIZE * sizeof(uint8_t));

  pdm_microphone_data_init(pio0, 0, offset, clk_div, PIO_DATA_PIN, PIO_CLK_PIN);
  // Configure a channel to write the same word (32 bits) repeatedly to PIO0
  // SM0's TX FIFO, paced by the data request signal from that peripheral.
  dma_channel = dma_claim_unused_channel(true);
  dma_channel_config c = dma_channel_get_default_config(dma_channel);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, true);
  channel_config_set_dreq(&c, pio_get_dreq(pio0, 0, false));
  dma_channel_configure(
      dma_channel,
      &c,
      raw_buffer[0], // Write address (only need to set this once)
      &pio0->rxf[0],             //
      RAW_BUFFER_SIZE, // Write the same value many times, then halt and interrupt
      false             // Don't start yet
  );

}

void dma_pio_start()
{

  // Tell the DMA to raise IRQ line 0 when the channel finishes a block
  dma_channel_set_irq0_enabled(dma_channel, true);

  // Configure the processor to run dma_handler() when DMA IRQ 0 is asserted
  irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
  irq_set_enabled(DMA_IRQ_0, true);

  pio_sm_set_enabled(
      pio0,
      0,
      true
  );

  raw_buffer_write_index = 0;
  raw_buffer_read_index = 0;

  // clear irq
  dma_hw->ints0 = 1u << dma_channel;
  dma_channel_transfer_to_buffer_now(
      dma_channel,
      raw_buffer[0],
      RAW_BUFFER_SIZE
  );
}

void dma_handler() {
  // clear irq
  dma_hw->ints0 = 1u << dma_channel;
  
  // get the current buffer index
  raw_buffer_read_index = raw_buffer_write_index;
  flag_mic_started = true;
  // get the next capture index to send the dma to start
  raw_buffer_write_index = (raw_buffer_write_index + 1) % RAW_BUFFER_COUNT;

  dma_channel_set_write_addr(
      dma_channel,
      raw_buffer[raw_buffer_write_index],
      true
  );

  cdc_task_buf(raw_buffer[raw_buffer_read_index]);

}
