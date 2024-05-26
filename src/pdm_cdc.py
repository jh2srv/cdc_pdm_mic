import serial
import serial.threaded
import time 
class SerialReader(serial.threaded.Protocol):
    def __init__(self, filename = 'test.bin'):
        self.data_bin = open(filename, "wb")

    def __call__(self):
        return self

    def data_received(self, data):
        self.data_bin.write(data)
        
    def close_bin(self):
        self.data_bin.close()
    # def set_file(filename):

    def __del__(self):        
        self.data_bin.close()

# Change port to the name of your device (e.g. /dev/ttyUSB0 on GNU/Linux or COM3 on Windows).
port = 'COM8'
ser = serial.Serial(port)
# the data will be save in a file
serial_reader = SerialReader('test.bin')

with serial.threaded.ReaderThread(ser, serial_reader) as protocol:
    # save some data for 10 seconds
    time.sleep(10)
 