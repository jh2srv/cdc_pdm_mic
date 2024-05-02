# MEMS PDM Microphone Data Acquisition with Raspberry Pi Pico

## Overview
This repository contains code for reading raw data from a MEMS PDM (Pulse Density Modulation) microphone using a Raspberry Pi Pico microcontroller. The raw data obtained from the microphone is transmitted over serial communication using USB CDC (Communications Device Class) protocol. This project is particularly useful for testing different decimation filters for processing PDM signals.

## Features
- Reads raw data from a MEMS PDM microphone.
- Transfers data over serial communication using USB CDC.
- Data is sent in 1-bit format, encapsulated in packets of 8 bits.
- Utilizes Raspberry Pi Pico as the microcontroller.
- Provides a platform for testing various decimation filters for PDM signals.
- Uses TinyUSB library for USB CDC functionality.

## Usage
1. **Hardware Setup**: Connect the MEMS PDM microphone to the Raspberry Pi Pico according to the provided specifications.
| Raspberry Pi Pico / RP2040 | PDM Microphone |
| -------------------------- | ----------------- |
| 3.3V | VCC |
| GND | GND |
| GPIO 17 | DAT |
| GPIO 16 | CLK |
2. **Software Setup**: Clone this repository to your Raspberry Pi Pico development environment.
3. **Compilation**:
    - Ensure you have the Raspberry Pi's Pico SDK and required toolchains installed, such as TinyUSB. Refer to the [official Raspberry Pi Pico documentation](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) for detailed instructions.
    - Set the `PICO_SDK` environment variable to the path containing the Pico SDK.
    - Use the following commands to compile the code:
      ```bash
      mkdir build
      cd build
      cmake -G "NMake Makefiles" .. -DPICO_BOARD=pico -DFAMILY=rp2040 -DBOARD=raspberry_pi_pico
      nmake
      ```
5. **Flash the UF2 file to Raspberry Pi Pico**: Flash the compiled code onto the Raspberry Pi Pico microcontroller.
6. **Connect to Serial Terminal**: Use a serial terminal application to connect to the Raspberry Pi Pico over USB CDC. (Ensure appropriate drivers are installed if needed.)
7. **View Data**: Once connected, you should start receiving raw data from the MEMS PDM microphone.

## Dependencies
- [TinyUSB](https://github.com/hathach/tinyusb): A small and efficient USB stack for USB-enabled microcontrollers. Used for USB CDC communication.

## License
This project is licensed under the [MIT License](LICENSE).

## Contact
For any inquiries or suggestions, feel free to reach out to [your-email@example.com].
