#include <stdio.h>
#include <pulp.h>

#define UART0_PER_ID 0

int main()
{
	i2c_open();
	uart_open(UART0_PER_ID, 9600);

	char STARTUP_MESSAGE[] = "UART Mirror\n";
	uart_write(UART0_PER_ID, STARTUP_MESSAGE, sizeof(STARTUP_MESSAGE));

	while (1)
	{
		uint8_t key;
		uart_read(UART0_PER_ID, &key, 1);
		uart_write(UART0_PER_ID, &key, 1);
	}

	uart_close(UART0_PER_ID);
	i2c_close();

	return 0;
}
