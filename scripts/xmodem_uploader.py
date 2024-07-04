import sys
import serial
import time
from xmodem import XMODEM

args = sys.argv
PORT = args[1]
PATH = args[2]

ser = serial.Serial(PORT, baudrate=115200, timeout=0.1)
def getc(size, timeout=1):
    return ser.read(size)

def putc(data, timeout=1):
    return ser.write(data)

modem = XMODEM(getc, putc)

stream = open(PATH, 'rb')

print('Upload succeed' if modem.send(stream) else 'Upload failed')
