#include <pulp.h>
#include "xmodem.h"

#define archi_write32(add, val_) (*(volatile unsigned int *)(long)(add) = (val_))
#define archi_read32(add) (*(volatile unsigned int *)(long)(add))

#define SRAM_BASE 0x1C004000
#define FLASH_BASE 0x1A000000
#define PROGRAM_BASE 0x1A001400

#define P_FLCR 0x1A121000 // FLASH Control Register
#define P_FLEB 0x1A121004 // FLASH Erase Block Register
#define P_FLPT 0x1A121008 // FLASH Program Timer
#define P_FLET 0x1A12100C // FLASH Erase Timer

#define LP_CMD_PAGE_ERASE 0x55555555
#define LP_CMD_CHIP_ERASE 0xAAAAAAAA
#define LP_CMD_FORMAT 0x12345678

void wait_flash_busy()
{
	while (1)
	{
		long rdata = archi_read32(P_FLCR);
		if ((rdata & 0x80000000) == 0)
			break;
	}
}

int main(void)
{
	int psize;

	// XModem receive
	uart_open(UART0_PER_ID, 115200);
	psize = xmodemReceive((char *)SRAM_BASE, 0x4000);
	uart_close(UART0_PER_ID);

	archi_write32(P_FLCR, 0x00000040);
	archi_write32(P_FLCR, 0x00000041);
	archi_write32(P_FLEB, 0xffffffe0);
	archi_write32(P_FLCR, 0x00000040);
	archi_write32(P_FLCR, 0x00000042);
	archi_write32(FLASH_BASE, 0xAAAAAAAA);
	wait_flash_busy();

	archi_write32(P_FLCR, 0x00000040);
	archi_write32(P_FLCR, 0x00000041);
	archi_write32(P_FLPT, 0x000001f4); // FLASH Program Timer

	archi_write32(PROGRAM_BASE, psize);
	wait_flash_busy();
	for (uint32_t addr = 0; addr < psize; addr += sizeof(uint32_t))
	{
		archi_write32(PROGRAM_BASE + addr + 4, archi_read32(SRAM_BASE + addr));
		wait_flash_busy();
	}

	archi_write32(P_FLCR, 0x00000000);

	// Reboot
	asm volatile("lui t0, 0x1a000");
	asm volatile("addi t0, t0, 0x80");
	asm volatile("jr t0");

	return 0;
}
