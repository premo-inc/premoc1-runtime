import sys
import time
import serial

# Telnet Timeout
TIMEOUT = 1

args = sys.argv

# Parameters
PORT = args[1]
START_ADDRESS = int(args[2], 0)
RAW_DATA_PATH = args[3]

# Read raw data
raw_data = open(RAW_DATA_PATH, 'rb')
data = raw_data.read()
data_size = len(data)

# Start UART
uart = serial.Serial(port=PORT, baudrate=115200, timeout=TIMEOUT, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_TWO)

# dummy command
uart.write(b'm 0x1c000000 4 \r\n')

address = 0
while address < data_size:
  uart.read_until(b'>')
  uart.write(b'M ' + bytes(hex(START_ADDRESS + address).encode('ascii')) + b' 4 ' + bytes(hex(int.from_bytes(data[address:address+4], byteorder='little')).encode('ascii')) + b'\r\n')
  uart.flush()
  address = address + 4
  # time.sleep(0.01)

# Verify Program
address = 0
uart.read_until(b'>')
uart.write(b'm ' + bytes(hex(START_ADDRESS + address).encode('ascii')) + b' ' + bytes(hex(data_size).encode('ascii')) + b'\r\n')
uart.readline() # Skip input command
output = (uart.readline().decode('utf-8')).split()

error_count = 0
while address < data_size:
  w = int.from_bytes(data[address:address+4], byteorder='little')
  r = int(output[address // 4], 0)
  if w != r:
    error_count = error_count + 1
    print('Verify error ... Address: {}, Expected data: {}, Output data: {}'.format(hex(START_ADDRESS + address), w, r))
  address = address + 4

if error_count == 0:
  print('Verify succeed')
else:
  print('Verify failed: {} errors'.format(error_count))

# Jump
uart.write(b'j ' + bytes(hex(START_ADDRESS).encode('ascii')) + b'\r\n')

time.sleep(0.1)

# Finish UART
uart.close()

