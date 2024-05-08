#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <pulp.h>
#include "strtol.h"

#define UART0_PER_ID 0

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

void AppendString(int *counter, char *str, uint32_t *data, char key, const int MAX_STR_LENGTH, char *flag)
{
	if (*counter < MAX_STR_LENGTH && key != ' ' && key != '\r' && key != '\n')
	{
		str[*counter] = key;
		(*counter)++;
	}
	else
	{
		if (*counter != 0)
		{
			str[*counter] = '\0';
			*counter = 0;
			*data = ustrtol(str, NULL, 0);

			if (flag != NULL)
			{
				*flag = 1;
			}
		}
	}
}

int main()
{
	i2c_open();
	uart_open(UART0_PER_ID, 115200);

	char STARTUP_MESSAGE[] = "UART Editor\n";
	uart_write(UART0_PER_ID, STARTUP_MESSAGE, sizeof(STARTUP_MESSAGE));

	uint8_t command = 0;
	uint32_t addr = 0;
	uint32_t size = 0;
	char flush = 0;
	char flag = 0;
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
		uart_read(UART0_PER_ID, &key, 1);
		uart_write(UART0_PER_ID, &key, 1);

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
			else if (message_count < size && (command == COMMAND_MEM_WRITE || command == COMMAND_I2C_WRITE || command == COMMAND_SPI_WRITE))
			{
				AppendString(&str_counter, data_str, message + message_count, key, MAX_STR_LENGTH, &flag);
				if (flag == 1)
				{
					message_count += 4;
					flag = 0;
				}
			}
			else
			{
				flush = 1;
			}

			if (flush == 1)
			{
				uart_write(UART0_PER_ID, "Accept", 7);
				switch (command)
				{
				case COMMAND_MEM_READ:
					for (int i = 0; i < size; i += 4) {
						sprintf(message, "0x%08X ", *((uint32_t*)((uint8_t*)addr + i)));
						uart_write(UART0_PER_ID, message, strlen(message));
					}
					uart_write(UART0_PER_ID, "\n", 1);
					break;
				case COMMAND_MEM_WRITE:
					memcpy((uint8_t*)addr, message, message_count);
					break;
				case COMMAND_JUMP:
					// Jump
					(*(uint32_t*)0x1c004000) = addr;
					asm volatile("lui t0, 0x1c004");
					asm volatile("lw  t1, 0(t0)");
					asm volatile("jalr ra, t1");
					// If you want to return uart editor, you must append this code into end of your program...
					/*
					  asm volatile("lui t0, 0x1c000");
					  asm volatile("jr t0");
					*/
					break;
				case COMMAND_I2C_READ:
					i2c_read((uint8_t)(addr & 0xff), size, response);
					for (int i = 0; i < size; i++) {
						sprintf(message, "0x%02X ", response[i]);
						uart_write(UART0_PER_ID, message, strlen(message));
					}
					uart_write(UART0_PER_ID, "\n", 1);
					break;
				case COMMAND_I2C_WRITE:
					i2c_write((uint8_t)(addr & 0xff), size, message);
					break;
				case COMMAND_SPI_READ:
					// spim_reads(1, (uint8_t)(addr & 0xff), size, response);
					for (int i = 0; i < size; i++) {
						sprintf(message, "0x%02X ", response[i]);
						uart_write(UART0_PER_ID, message, strlen(message));
					}
					uart_write(UART0_PER_ID, "\n", 1);
					break;
				case COMMAND_SPI_WRITE:
					// spim_writes(1, (uint8_t)(addr & 0xff), size, message);
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

	uart_close(UART0_PER_ID);
	i2c_close();

	return 0;
}
