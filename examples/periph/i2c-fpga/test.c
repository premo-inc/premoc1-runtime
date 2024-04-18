/**
 * for pulpissimo(v6.0.0) on zedboard
 * 動かないかもしれない。
*/

#include <stdio.h>
#include <pulp.h>
#include <stdint.h>
#include "pulp.h"

#define DMA_CTRL_CFG_CG 0x1A102000

// I2C1: I2C_BASE_ADDR + 0x80
#define I2C_BASE_ADDR 0x1A102180
#define I2C_RX_SADDR (I2C_BASE_ADDR + 0x00)
#define I2C_RX_SIZE (I2C_BASE_ADDR + 0x04)
#define I2C_RX_CFG (I2C_BASE_ADDR + 0x08)
#define I2C_TX_SADDR (I2C_BASE_ADDR + 0x10)
#define I2C_TX_SIZE (I2C_BASE_ADDR + 0x14)
#define I2C_TX_CFG (I2C_BASE_ADDR + 0x18)
#define I2C_CMD_SADDR (I2C_BASE_ADDR + 0x20)
#define I2C_CMD_SIZE (I2C_BASE_ADDR + 0x24)
#define I2C_CMD_CFG_ADDR (I2C_BASE_ADDR + 0x28)
#define I2C_STATUS (I2C_BASE_ADDR + 0x30)
#define I2C_SETUP (I2C_BASE_ADDR + 0x34)

#define I2C_CMD_START 0x00	// I2C Start of Transfer command.
#define I2C_CMD_STOP 0x20	// I2C End of Transfer command.
#define I2C_CMD_RD_ACK 0x40	// I2C receive data and acknowledge command.
#define I2C_CMD_RD_NACK 0x60 // I2C receive data and not acknowledge command.
#define I2C_CMD_WR 0x80		// I2C send data and wait acknowledge command.
#define I2C_CMD_WAIT 0xA0	// I2C wait dummy cycles command.
#define I2C_CMD_RPT 0xC0		// I2C next command repeat command.
#define I2C_CMD_CFG 0xE0		// I2C configuration command.
#define I2C_CMD_WAIT_EV 0x10 // I2C wait uDMA external event command.

// avoid collision with the same name
uint8_t aaaaa_cmd_buffer_8[16];
uint8_t aaaaa_tx_buffer[256];
uint8_t aaaaa_rx_buffer[256];

// uint8_t *aaaaa_cmd_buffer_8 = (uint8_t *)0x1C00A000;
// uint8_t *aaaaa_tx_buffer = (uint8_t *)0x1C00A100;
// uint8_t *aaaaa_rx_buffer = (uint8_t *)0x1C00A140;

#define I2C_MAX_BAUDRATE 100000
#define MASTER_FREQ 25000000
#define ZERO 0x00000000
static int i2c_get_div()
{
	int i2c_freq = I2C_MAX_BAUDRATE;
		// Round-up the divider to obtain an SPI frequency which is below the maximum
	int div = (MASTER_FREQ + i2c_freq - 1) / i2c_freq;
	// The SPIM always divide by 2 once we activate the divider, thus increase by 1
	// in case it is even to not go avove the max frequency.
	if (div & 1)
		div += 1;
	div >>= 1;
	return div;
}

void i2c_open() {
	// enable I2C clock gating
	uint32_t dma_ctrl_cfg_cg = archi_read32(DMA_CTRL_CFG_CG);
	printf("get dma_ctrl_cfg: 0x%x\n", dma_ctrl_cfg_cg);
	archi_write32(DMA_CTRL_CFG_CG,(
		ZERO | (1 << 2) // I2C
	) | dma_ctrl_cfg_cg);
	// archi_write32(DMA_CTRL_CFG_CG,0xFFFF);
	dma_ctrl_cfg_cg = archi_read32(DMA_CTRL_CFG_CG);
	printf("set dma_ctrl_cfg: 0x%x\n", dma_ctrl_cfg_cg);
}

void i2c_close() {
	// disable I2C clock gating
	uint32_t dma_ctrl_cfg_cg = archi_read32(DMA_CTRL_CFG_CG);
	archi_write32(DMA_CTRL_CFG_CG,0xFFFFFFEF & dma_ctrl_cfg_cg);
	dma_ctrl_cfg_cg = archi_read32(DMA_CTRL_CFG_CG);
	printf("set dma_ctrl_cfg: 0x%x\n", dma_ctrl_cfg_cg);
}

void i2c_read(uint8_t addr, uint8_t length, uint8_t *data)
{
	// only i2c0, no i2c1 for premo c1 board

	int seq_index = 0;
	int _div = 25000000 / 100000;
	aaaaa_cmd_buffer_8[seq_index++] = I2C_CMD_CFG;
	aaaaa_cmd_buffer_8[seq_index++] = (_div >> 8) & 0xFF;
	aaaaa_cmd_buffer_8[seq_index++] = (_div & 0xFF);
	aaaaa_cmd_buffer_8[seq_index++] = I2C_CMD_START;
	aaaaa_cmd_buffer_8[seq_index++] = I2C_CMD_WR;
	aaaaa_cmd_buffer_8[seq_index++] = addr << 1 | 0x1;
	if (length > 1)
	{
		aaaaa_cmd_buffer_8[seq_index++] = I2C_CMD_RPT;
		aaaaa_cmd_buffer_8[seq_index++] = length - 1;
		aaaaa_cmd_buffer_8[seq_index++] = I2C_CMD_RD_ACK;
	}
	aaaaa_cmd_buffer_8[seq_index++] = I2C_CMD_RD_NACK;
	aaaaa_cmd_buffer_8[seq_index++] = I2C_CMD_STOP;

	// set RX buffer
	archi_write32(I2C_RX_SADDR, data);
	archi_write32(I2C_RX_SIZE, length);
	archi_write32(I2C_RX_CFG, (0 << 6) |	 // CLR
								 (0 << 5) | // pending (read only)
								 (1 << 4) | // en
								 (0 << 0)   // continous
	);

	// set command buffer
	archi_write32(I2C_CMD_SADDR, aaaaa_cmd_buffer_8);
	archi_write32(I2C_CMD_SIZE, seq_index);
	archi_write32(I2C_CMD_CFG_ADDR, (0 << 6) |	 // CLR
								(0 << 5) | // pending (read only)
								(1 << 4) | // en
								(0 << 0)   // continous
	);

	while ((archi_read32(I2C_RX_CFG) & 0x10) != 0)
	{
	};

	while ((archi_read32(I2C_CMD_CFG_ADDR) & 0x10) != 0)
	{
	};

	// debug print
	for (int i = 0; i < length; i++)
	{
		printf("data[%d] = %x\n", i, data[i]);
	}
}

void i2c_write(uint8_t addr, uint8_t length, uint8_t *data)
{
	// only i2c0, no i2c1 for premo c1 board

	// set command buffer
	int seq_index = 0;
	int _div = 10000000 / 1000000;
	aaaaa_cmd_buffer_8[seq_index++] = I2C_CMD_CFG;
	aaaaa_cmd_buffer_8[seq_index++] = (_div & 0xFF); //250
	aaaaa_cmd_buffer_8[seq_index++] = (_div >> 8) & 0xFF; //0
	aaaaa_cmd_buffer_8[seq_index++] = I2C_CMD_START;
	aaaaa_cmd_buffer_8[seq_index++] = I2C_CMD_WR;
	aaaaa_cmd_buffer_8[seq_index++] = addr << 1 | 0x0;

	// aaaaa_cmd_buffer_8[seq_index++] = I2C_CMD_RPT;
	// aaaaa_cmd_buffer_8[seq_index++] = length;
	// aaaaa_cmd_buffer_8[seq_index++] = I2C_CMD_WR;
	// for (int i = 0; i < length; i++)
	// {
	// 	aaaaa_cmd_buffer_8[seq_index++] = data[i];
	// }

	for (int i = 0; i < seq_index; i++)
	{
		printf("cmd[%d] = %x\n", i, aaaaa_cmd_buffer_8[i]);
	}



	printf("write cmd buffer %x\n", aaaaa_cmd_buffer_8);
	archi_write32(I2C_TX_SADDR, aaaaa_cmd_buffer_8);
	printf("%x\n", archi_read32(I2C_TX_SADDR));

	printf("write cmd size %x\n", seq_index);
	archi_write32(I2C_TX_SIZE, seq_index);
	printf("%x\n", archi_read32(I2C_TX_SIZE));

	printf("write cmd cfg %x\n", (0 << 6) |	 // CLR
								(0 << 5) | // pending (read only)
								(1 << 4) | // en
								(0 << 0)   // continous
	);
	archi_write32(I2C_TX_CFG, (0 << 6) |	 // CLR
								(0 << 5) | // pending (read only)
								(1 << 4) | // en
								(0 << 0)   // continous
	);
	printf("%x\n", archi_read32(I2C_TX_CFG));

	while ((archi_read32(I2C_TX_CFG) & 0x10) != 0);

}

//============================
//
//	main
//
//============================
int main()
{
	printf("%x\n", archi_read32(I2C_CMD_CFG_ADDR));


	printf("Entering I2C test\n");

	// RESET
	// archi_write32(I2C_SETUP, 0x00000001);
	// archi_write32(I2C_SETUP, 0x00000000);
	i2c_open();
	for (int i = 0; i < 100000; i++);

	aaaaa_tx_buffer[0] = 0x11;
	aaaaa_tx_buffer[1] = 0x55;
	aaaaa_tx_buffer[2] = 0xff;
	
	i2c_write(0x13, 3, aaaaa_tx_buffer);
	// i2c_read(0x13, 3, aaaaa_rx_buffer);


	// i2c_close();
	return 0;
}
