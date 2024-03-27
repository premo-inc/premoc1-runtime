#include <stdio.h>
#include <pulp.h>
#include <stdint.h>
#include "pulp.h"

#define BUFFER_SIZE 256

#define DMA_CTRL_CFG_CG 0x1A102000

#define UART0_PER_ID 0
#define UART0_BASE_ADDR (0x1A102000 + (UART0_PER_ID + 1) * 0x80)
#define UART0_RX_SADDR (UART0_BASE_ADDR + 0x00)
#define UART0_RX_SIZE (UART0_BASE_ADDR + 0x04)
#define UART0_RX_CFG (UART0_BASE_ADDR + 0x08)
#define UART0_TX_SADDR (UART0_BASE_ADDR + 0x10)
#define UART0_TX_SIZE (UART0_BASE_ADDR + 0x14)
#define UART0_TX_CFG (UART0_BASE_ADDR + 0x18)
#define UART0_STATUS (UART0_BASE_ADDR + 0x20)
#define UART0_SETUP (UART0_BASE_ADDR + 0x24)
#define UART0_ERROR (UART0_BASE_ADDR + 0x28)
#define UART0_IRQ_EN (UART0_BASE_ADDR + 0x2C)
#define UART0_VALID (UART0_BASE_ADDR + 0x30)
#define UART0_DATA (UART0_BASE_ADDR + 0x34)
#define UART0_APB_ADDR (UART0_BASE_ADDR + 0x40)
#define UART0_APB_DATA (UART0_BASE_ADDR + 0x44)
#define UART0_APB_CMD (UART0_BASE_ADDR + 0x48)

uint8_t *tx_buffer = (uint8_t *)0x1C008000;
uint8_t *rx_buffer = (uint8_t *)0x1C008100;

int main()
{
	printf("Entering UART0 test\n");

	archi_write32(DMA_CTRL_CFG_CG, 0x00000001);

	archi_write32(UART0_SETUP, (0x00d6 << 16) | // DIV 6b:230.4kbps, d6:115.2kbps
								   (1 << 9) |	// RX_EN
								   (1 << 8) |	// TX_EN
								   (0 << 5) |	// Clean FIFO
								   (0 << 4) |	// Polling EN
								   (1 << 3) |	// STOP bit
								   (3 << 1) |	// Bit length 0:5bit, 1:6bit, 2:7bit, 3:8bit
								   (1 << 0)		// parity en
	);

	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		// tx_buffer[i] = 0;
		tx_buffer[i] = 0x5A;
		// tx_buffer[i] = 0xAA;
		rx_buffer[i] = 0;
	}

	archi_write32(UART0_RX_SADDR, rx_buffer);
	archi_write32(UART0_RX_SIZE, 256);
	archi_write32(UART0_RX_CFG, (0 << 6) |	   // CLR
									(0 << 5) | // pending
									(1 << 4) | // en
									(0 << 0)   // continous
	);

	archi_write32(UART0_TX_SADDR, tx_buffer);
	archi_write32(UART0_TX_SIZE, 256);
	archi_write32(UART0_TX_CFG, (0 << 6) |	   // CLR
									(0 << 5) | // pending
									(1 << 4) | // en
									(0 << 0)   // continous
	);

	while ((archi_read32(UART0_TX_CFG) & 0x10) != 0)
	{
	};

	printf("TX done.\n");

	while ((archi_read32(UART0_RX_CFG) & 0x10) != 0)
	{
	};

	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		if (i % 16 == 0)
			printf("%02x : ", i);
		printf("%02x%s", rx_buffer[i], i % 16 == 15 ? "\n" : " ");
	}

	return 0;
}
