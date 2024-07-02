#include <pulp.h>
#include "xmodem.h"

int main(void)
{
	uart_open(UART0_PER_ID, 115200);
	xmodemReceive((char *)0x1c004000, 0x4000);
	uart_close(UART0_PER_ID);

	// Jump
	(*(uint32_t*)0x1c002000) = 0x1c004000;
	asm volatile("lui t0, 0x1c002");
	asm volatile("lw  t1, 0(t0)");
	asm volatile("jalr ra, t1");

	return 0;
}
