#ifndef xmodem_h
#define xmodem_h
// #pragma once

#define UART0_PER_ID 0

int xmodemReceive(unsigned char *dest, int destsz);

#endif