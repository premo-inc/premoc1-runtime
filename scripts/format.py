import sys
import time
import telnetlib

# Telnet Timeout
TIMEOUT = 1

args = sys.argv

# Parameters
HOST = args[1]
PORT = args[2]

# Start Telnet Section
tn = telnetlib.Telnet(HOST, PORT)

# Erase FLASH
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000040' + b'\n')
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a12100C 32 0x0007a120' + b'\n')
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x000000c0' + b'\n')
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a000000 32 0x1234567' + b'\n')

# wait for busy bit to 0
while True:
  tn.expect([b'>'], TIMEOUT)
  tn.write(b'read_memory 0x1a121000 32 1' + b'\n')
  tn.read_until(b'\n', TIMEOUT) # Skip input command
  output = int(tn.read_until(b'\n', TIMEOUT).decode('utf-8'), 0)
  if (output >> 31) == 0:
    break

tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000040' + b'\n')
tn.expect([b'>'], TIMEOUT)
tn.write(b'write_memory 0x1a121000 32 0x00000000' + b'\n')

# Finish Telnet Section
tn.close()

