/*	
 * Copyright 2001-2010 Georges Menie (www.menie.org)
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California, Berkeley nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* this code needs standard functions memcpy() and memset()
   and input/output functions _inbyte() and _outbyte().

   the prototypes of the input/output functions are:
     int _inbyte(unsigned short timeout); // msec timeout
     void _outbyte(int c);

 */

#include "crc16.h"
#include "xmodem.h"
#include <pulp.h>

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

#define DLY_1S 1000
#define MAXRETRANS 25

static int _inbyte(unsigned short timeout) {
	static const int channel = UDMA_EVENT_ID(ARCHI_UDMA_UART_ID(UART0_PER_ID));
	static const unsigned int base = (ARCHI_SOC_PERIPHERALS_ADDR + ARCHI_UDMA_OFFSET + UDMA_PERIPH_OFFSET(channel>>1) + UDMA_CHANNEL_OFFSET(channel&1));
	static int res = 0;
	static int rx_busy = 0;

	if (!rx_busy) {
		plp_udma_enqueue(base, (int)(&res), 1, UDMA_CHANNEL_CFG_EN | UDMA_CHANNEL_CFG_SIZE_8);
		rx_busy = 1;
	}

	uint32_t timer_cfg = \
		(0 << 31) | \
		(0xf9 << 8) | /* pval */ \
		(0 << 7) | /* FLL */ \
		(1 << 6) | /* Pen (pval enable) */ \
		(0 << 5) | /* One shot (should be 0) */ \
		(0 << 4) | /* Cycle mode */ \
		(1 << 3) | /* Input event */ \
		(0 << 2) | /* IRQ Enable */ \
		(1 << 1) | /* Timer Reset */ \
		(1 << 0) /* Timer Enable */;
	timer_cfg_lo_set(ARCHI_FC_TIMER_ADDR, timer_cfg); /* Set Config */
	timer_cnt_lo_set(ARCHI_FC_TIMER_ADDR, 0); /* Set initial timer value */
	timer_start_lo_set(ARCHI_FC_TIMER_ADDR, 1); /* Start timer */

	while (timer_cnt_lo_get(ARCHI_FC_TIMER_ADDR) < 100 * timeout)
	{
		if (!plp_udma_busy(UDMA_UART_RX_ADDR(ARCHI_UDMA_UART_ID(UART0_PER_ID) - ARCHI_UDMA_UART_ID(0))))
		{
			rx_busy = 0;
			return res;
		}
	}

	return -1;
}

static void _outbyte(int c) {
	uart_write(UART0_PER_ID, &c, 1);
}

static int check(int crc, const unsigned char *buf, int sz)
{
	if (crc) {
		unsigned short crc = crc16_ccitt(buf, sz);
		unsigned short tcrc = (buf[sz]<<8)+buf[sz+1];
		if (crc == tcrc)
			return 1;
	}
	else {
		int i;
		unsigned char cks = 0;
		for (i = 0; i < sz; ++i) {
			cks += buf[i];
		}
		if (cks == buf[sz])
		return 1;
	}

	return 0;
}

int xmodemReceive(unsigned char *dest, int destsz)
{
	unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
	unsigned char *p;
	int bufsz, crc = 0;
	unsigned char trychar = 'C';
	unsigned char packetno = 1;
	int i, c, len = 0;
	int retrans = MAXRETRANS;

	for (;;) {
		for (;;) {
			if (trychar) _outbyte(trychar);
			if ((c = _inbyte((DLY_1S)>>2)) >= 0) {
				switch (c) {
				case SOH:
					bufsz = 128;
					goto start_recv;
				case STX:
					bufsz = 1024;
					goto start_recv;
				case EOT:
					_outbyte(ACK);
					return len; /* normal end */
				case CAN:
					if ((c = _inbyte(DLY_1S)) == CAN) {
						_outbyte(ACK);
						return -1; /* canceled by remote */
					}
					break;
				default:
					break;
				}
			}
		}

		if (trychar == 'C') { trychar = NAK; continue; }
		_outbyte(CAN);
		_outbyte(CAN);
		_outbyte(CAN);
		return -2; /* sync error */

	start_recv:
		if (trychar == 'C') crc = 1;
		trychar = 0;

		int rsize = (bufsz+(crc?1:0)+3);
		xbuff[0] = c;
		uart_read(0, xbuff + 1, rsize);

		if (xbuff[1] == (unsigned char)(~xbuff[2]) && 
			(xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno-1) &&
			check(crc, &xbuff[3], bufsz)) {
			if (xbuff[1] == packetno)	{
				register int count = destsz - len;
				if (count > bufsz) count = bufsz;
				if (count > 0) {
					memcpy (&dest[len], &xbuff[3], count);
					len += count;
				}
				++packetno;
				retrans = MAXRETRANS+1;
			}
			if (--retrans <= 0) {
				_outbyte(CAN);
				_outbyte(CAN);
				_outbyte(CAN);
				return -3; /* too many retry error */
			}
			_outbyte(ACK);
			continue;
		}
	reject:
		_outbyte(NAK);
	}
}

