#include <stdio.h>
#include <pulp.h>

#define UART0_PER_ID 0

int main()
{
	uart_open(UART0_PER_ID, 115200);

	char STARTUP_MESSAGE[] = "UART Mirror\n";
	uart_write(UART0_PER_ID, STARTUP_MESSAGE, sizeof(STARTUP_MESSAGE));

	uint8_t key;
	while (1) {
		uart_read(UART0_PER_ID, &key, 1);
		uart_write(UART0_PER_ID, &key, 1);
	}

	return 0;
}
