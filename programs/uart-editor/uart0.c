#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "pulp.h"
#include "strtol.h"

#define BUFFER_SIZE 16

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

enum
{
	COMMAND_NONE,
	COMMAND_MEM_READ,
	COMMAND_MEM_WRITE,
	COMMAND_JUMP,
	COMMAND_I2C_READ,
	COMMAND_I2C_WRITE,
	COMMAND_SPI_READ,
	COMMAND_SPI_WRITE,
	COMMAND_MAX
};

void SendMessage(uint8_t *buffer, size_t length)
{
	archi_write32(UART0_TX_SADDR, buffer);
	archi_write32(UART0_TX_SIZE, length);
	archi_write32(UART0_TX_CFG, (0 << 6) |	   // CLR
									(0 << 5) | // pending
									(1 << 4) | // en
									(0 << 0)   // continous
	);

	while ((archi_read32(UART0_TX_CFG) & 0x10) != 0)
		;
}

void ReceiveMessage(uint8_t *buffer, size_t length)
{
	archi_write32(UART0_RX_SADDR, buffer);
	archi_write32(UART0_RX_SIZE, length);
	archi_write32(UART0_RX_CFG, (0 << 6) |	   // CLR
									(0 << 5) | // pending
									(1 << 4) | // en
									(0 << 0)   // continous
	);

	while ((archi_read32(UART0_RX_CFG) & 0x10) != 0)
		;
}

void AppendString(int *counter, char *str, uint32_t *data, char key, const int MAX_STR_LENGTH, char *flag)
{
	if (*counter < MAX_STR_LENGTH && key != ' ' && key != '\r' && key != '\n')
	{
		str[(*counter)] = key;
		(*counter)++;
	}
	else
	{
		if (*counter != 0)
		{
			str[(*counter)] = '\0';
			*counter = 0;
			*data = strtol(str, NULL, 0);

			if (flag != NULL)
			{
				*flag = key == 'p' ? 1 : -1;
			}
		}
	}
}

int main()
{
	i2c_open();

	// Entering UART0 test
	archi_write32(DMA_CTRL_CFG_CG, 0x00000001);
	archi_write32(UART0_SETUP, (0x00d6 << 16) | // DIV			// 115,200 bits/s
								   (1 << 9) |	// RX_EN
								   (1 << 8) |	// TX_EN
								   (0 << 5) |	// Clean FIFO
								   (0 << 4) |	// Polling EN
								   (0 << 3) |	// STOP bit		// 1bit
								   (3 << 1) |	// Bit length	// 8bit
								   (0 << 0)		// parity en	// None
	);

	SendMessage("UART Editor\n", 13);

	uint8_t command = 0;
	uint32_t addr = 0;
	uint32_t size = 0;
	char flush = 0;
	const int MAX_STR_LENGTH = 16;
	char addr_str[MAX_STR_LENGTH];
	char size_str[MAX_STR_LENGTH];
	char data_str[MAX_STR_LENGTH];
	int str_counter = 0;
	uint8_t message[64] = "";
	uint32_t message_count = 0;
	uint8_t response[64] = "";
	while (1)
	{
		uint8_t key;
		ReceiveMessage(&key, 1);

		if (command != COMMAND_NONE)
		{
			if (addr == 0)
			{
				AppendString(&str_counter, addr_str, &addr, key, MAX_STR_LENGTH, NULL);
			}
			else if (size == 0 && command != COMMAND_JUMP)
			{
				AppendString(&str_counter, size_str, &size, key, MAX_STR_LENGTH, NULL);
			}
			else if (flush != 1 && (command == COMMAND_MEM_WRITE || command == COMMAND_I2C_WRITE))
			{
				AppendString(&str_counter, data_str, &message + message_count, key, MAX_STR_LENGTH, &flush);
				if (flush == -1)
				{
					message_count++;
					flush = 0;
				}
			}
			else
			{
				flush = 1;
			}

			if (flush == 1)
			{
				switch (command)
				{
				case COMMAND_MEM_READ:
					for (int i = 0; i < size; i++) {
						sprintf(message, "0x%02X ", *(uint8_t*)addr);
						SendMessage(message, strlen(message));
					}
					SendMessage("\n", 1);
					break;
				case COMMAND_MEM_WRITE:
					memcpy((uint8_t*)addr, message, message_count);
					break;
				case COMMAND_JUMP:
					// Jump
					(*(uint32_t *)0x1c004000) = addr;
					asm volatile("lui t0, 0x1c004");
					asm volatile("lw  t1, 0(t0)");
					asm volatile("jalr ra, t1");
					// If you want to return uart editor, you must append this code into end of your program...
					/*
					  asm volatile("lui t0, 0x1c000");
					  asm volatile("jr t1");
					*/
					break;
				case COMMAND_I2C_READ:
					i2c_read((uint8_t)(addr & 0xff), size, response);
					for (int i = 0; i < size; i++) {
						sprintf(message, "0x%02X ", response[i]);
						SendMessage(message, strlen(message));
					}
					SendMessage("\n", 1);
					break;
				case COMMAND_I2C_WRITE:
					i2c_write((uint8_t)(addr & 0xff), size, message);
					break;
				}

				command = COMMAND_NONE;
				size = 0;
				addr = 0;
				message_count = 0;
				flush = 0;
			}
		}
		else
		{
			switch (key)
			{
			case 'm':
				command = COMMAND_MEM_READ;
				break;
			case 'M':
				command = COMMAND_MEM_WRITE;
				break;
			case 'j':
				command = COMMAND_JUMP;
				break;
			case 'i':
				command = COMMAND_I2C_READ;
				break;
			case 'I':
				command = COMMAND_I2C_WRITE;
				break;
			case 's':
				command = COMMAND_I2C_READ;
				break;
			case 'S':
				command = COMMAND_I2C_WRITE;
				break;
			case ' ':
			case '\r':
			case '\n':
				break;
			default:
				command = COMMAND_NONE;
			}
		}
	}

	i2c_close();

	return 0;
}
