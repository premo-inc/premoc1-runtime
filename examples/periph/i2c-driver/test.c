/**
 * for pulpissimo on zedboard
 */

#include <stdio.h>
#include <stdint.h>
#include "pulp.h"

int main()
{
	printf("Entering I2C test\n");

	// RESET
	// archi_write32(I2C_SETUP, 0x00000001);
	// archi_write32(I2C_SETUP, 0x00000000);
	i2c_open();

	uint8_t data[3];
	uint8_t rx_data[3];
	data[0] = 0x11;
	data[1] = 0x55;
	data[2] = 0xff;

	i2c_write(0x13, 3, data);
	i2c_read(0x13, 3, rx_data);

	i2c_close();
	return 0;
}
