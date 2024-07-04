#include <stdio.h>
#include <pulp.h>

#define UART0_PER_ID 0

int main()
{
	uart_open(UART0_PER_ID, 115200);

	char STARTUP_MESSAGE[] = "UART Mirror\n";
	uart_write(UART0_PER_ID, STARTUP_MESSAGE, sizeof(STARTUP_MESSAGE));

	uint8_t key;
	do {
		uart_read(UART0_PER_ID, &key, 1);
		uart_write(UART0_PER_ID, &key, 1);
	} while (key != 'q');

	uart_close(UART0_PER_ID);

	return 0;
}
