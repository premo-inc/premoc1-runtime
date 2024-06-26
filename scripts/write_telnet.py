import sys
import time
import telnetlib

# Telnet Timeout
TIMEOUT = 1

args = sys.argv

# Parameters
HOST = args[1]
PORT = args[2]
START_ADDRESS = int(args[3], 0)
RAW_DATA_PATH = args[4]

# Read raw data
raw_data = open(RAW_DATA_PATH, 'rb')
data = raw_data.read()
data_size = len(data)

# Start Telnet Section
tn = telnetlib.Telnet(HOST, PORT)

# Erase FLASH
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000040' + b'\n')
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000041' + b'\n')

tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121004 32 0xffffffff' + b'\n')
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000040' + b'\n')
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000042' + b'\n')
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a000000 32 0xaaaaaaaa' + b'\n')
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000040' + b'\n')
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000000' + b'\n')

tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000041' + b'\n')
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000040' + b'\n')

time.sleep(1)

# Write Program
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000040' + b'\n')
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000041' + b'\n')

address = 0
while address < data_size:
  tn.expect([b'>'], TIMEOUT)
  tn.write(b'write_memory ' + bytes(hex(START_ADDRESS + address).encode('ascii')) + b' 32 ' + bytes(hex(int.from_bytes(data[address:address+4], byteorder='little')).encode('ascii')) + b'\n')
  address = address + 4
  # time.sleep(0.025)

tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000041' + b'\n')
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000040' + b'\n')

# Verify Program
address = 0
tn.expect([b'>'], TIMEOUT)
tn.write(b'read_memory ' + bytes(hex(START_ADDRESS + address).encode('ascii')) + b' 32 ' + bytes(hex(data_size // 4).encode('ascii')) + b'\n')
tn.read_until(b'\n', TIMEOUT) # Skip input command
output = (tn.read_until(b'>', TIMEOUT * 10).decode('utf-8')).split()

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

# Finish Telnet Section
tn.close()

