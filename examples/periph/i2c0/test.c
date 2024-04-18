#include <stdio.h>
#include <stdint.h>
#include "pulp.h"

#define DMA_CTRL_CFG_CG 0x1A102000

// No I2C1 for premo c1 board
#define I2C_BASE_ADDR 0x1A102280
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

#define I2C_CMD_START 0x0	  // I2C Start of Transfer command.
#define I2C_CMD_STOP 0x2	  // I2C End of Transfer command.
#define I2C_CMD_RD_ACK 0x4	  // I2C receive data and acknowledge command.
#define I2C_CMD_RD_NACK 0x6	  // I2C receive data and not acknowledge command.
#define I2C_CMD_WR 0x8		  // I2C send data and wait acknowledge command.
#define I2C_CMD_WAIT 0xA	  // I2C wait dummy cycles command.
#define I2C_CMD_RPT 0xC		  // I2C next command repeat command.
#define I2C_CMD_CFG_CMD 0xE	  // I2C configuration command.
#define I2C_CMD_WAIT_EV 0x1	  // I2C wait uDMA external event command.
#define I2C_CMD_WRB 0x7		  // I2C write byte command
#define I2C_CMD_EOT 0x9		  // Signal end of transfer (next transfer may be en-queued)
#define I2C_CMD_SETUP_UCA 0x3 // Setup RX or TX channel start address
#define I2C_CMD_SETUP_UCS 0x5 // Setup RX or TX channel transfer size+enable channel

// avoid collision with the same name

uint8_t i2c_tx_buffer[256];
uint8_t i2c_rx_buffer[256];
uint32_t i2c_cmd_buffer[16];

#define I2C_MAX_BAUDRATE 100000
#define MASTER_FREQ 25000000
#define ZERO 0x00000000

void i2c_open()
{
	// enable I2C clock gating
	uint32_t dma_ctrl_cfg_cg = archi_read32(DMA_CTRL_CFG_CG);
	printf("get dma_ctrl_cfg: 0x%x\n", dma_ctrl_cfg_cg);
	archi_write32(DMA_CTRL_CFG_CG, (
									   ZERO | (1 << 4) // I2C
									   ) |
									   dma_ctrl_cfg_cg);
	// archi_write32(DMA_CTRL_CFG_CG,0xFFFF);
	dma_ctrl_cfg_cg = archi_read32(DMA_CTRL_CFG_CG);
	printf("set dma_ctrl_cfg result: 0x%x\n", dma_ctrl_cfg_cg);
}

void i2c_close()
{
	// disable I2C clock gating
	uint32_t dma_ctrl_cfg_cg = archi_read32(DMA_CTRL_CFG_CG);
	archi_write32(DMA_CTRL_CFG_CG, 0xFFFFFFEF & dma_ctrl_cfg_cg);
	dma_ctrl_cfg_cg = archi_read32(DMA_CTRL_CFG_CG);
	printf("set dma_ctrl_cfg: 0x%x\n", dma_ctrl_cfg_cg);
}

void i2c_read(uint8_t addr, uint8_t length, uint8_t *data)
{
	// only i2c0, no i2c1 for premo c1 board
	int seq_index = 0;
	int _div = 25000000 / 100000;
	i2c_cmd_buffer[seq_index++] = ((I2C_CMD_CFG_CMD << 28) |
								   _div // clock divider position 0-15
	);
	i2c_cmd_buffer[seq_index++] = ((I2C_CMD_START << 28));
	i2c_cmd_buffer[seq_index++] = ((I2C_CMD_WRB << 28) | addr << 1 | 0x1);

	if (length > 1)
	{
		i2c_cmd_buffer[seq_index++] = ((I2C_CMD_RPT << 28) | length - 1);
		i2c_cmd_buffer[seq_index++] = ((I2C_CMD_RD_ACK << 28));
	}
	i2c_cmd_buffer[seq_index++] = ((I2C_CMD_RD_NACK << 28));
	i2c_cmd_buffer[seq_index++] = ((I2C_CMD_STOP << 28));

	// set RX buffer
	archi_write32(I2C_RX_SADDR, data);
	archi_write32(I2C_RX_SIZE, length);
	archi_write32(I2C_RX_CFG, (0 << 6) |	 // CLR
								  (0 << 5) | // pending (read only)
								  (1 << 4) | // en
								  (0 << 0)	 // continous
	);

	// set command buffer
	archi_write32(I2C_CMD_SADDR, i2c_cmd_buffer);
	archi_write32(I2C_CMD_SIZE, seq_index << 2);
	archi_write32(I2C_CMD_CFG_ADDR, (0 << 6) |	   // CLR
										(0 << 5) | // pending (read only)
										(1 << 4) | // en
										(0 << 0)   // continous
	);

	while ((archi_read32(I2C_RX_CFG) & 0x10) != 0);

	while ((archi_read32(I2C_CMD_CFG_ADDR) & 0x10) != 0);

	// debug print
	for (int i = 0; i < length; i++)
	{
		printf("data[%d] = 0x%x\n", i, data[i]);
	}
}


void i2c_write(uint8_t addr, uint8_t length, uint8_t *data)
{
	// only i2c0, no i2c1 for premo c1 board

	// set command buffer
	int seq_index = 0;
	int _div = 25000000 / 100000;

	for (int i = 0; i < length; i++)
	{
		i2c_tx_buffer[i] = data[i];
	}

	i2c_cmd_buffer[seq_index++] = ((I2C_CMD_CFG_CMD << 28) |
								   _div // clock divider position 0-15
	);

	i2c_cmd_buffer[seq_index++] = ((I2C_CMD_START << 28));
	i2c_cmd_buffer[seq_index++] = ((I2C_CMD_WRB << 28) | addr << 1 | 0x0);
	for (int i = 0; i < length; i++)
	{
		i2c_cmd_buffer[seq_index++] = ((I2C_CMD_WRB << 28) | data[i]);
	}

	i2c_cmd_buffer[seq_index++] = ((I2C_CMD_STOP << 28));

	for (int i = 0; i < seq_index; i++)
	{
		printf("cmd[%d] = 0x%x\n", i, i2c_cmd_buffer[i]);
	}

	// set tx buffer
	printf("write tx buffer 0x%x\n", i2c_tx_buffer);
	archi_write32(I2C_TX_SADDR, i2c_tx_buffer);
	// printf("0x%x\n", archi_read32(I2C_TX_SADDR));

	printf("write tx size 0x%x\n", length);
	archi_write32(I2C_TX_SIZE, length);
	// printf("0x%x\n", archi_read32(I2C_TX_SIZE));

	printf("write tx cfg 0x%x\n", (0 << 6) |	 // CLR
									  (0 << 5) | // pending (read only)
									  (1 << 4) | // en
									  (0 << 0)	 // continous
	);
	archi_write32(I2C_TX_CFG, (0 << 6) |	 // CLR
								  (0 << 5) | // pending (read only)
								  (1 << 4) | // en
								  (0 << 0)	 // continous
	);
	// printf("0x%x\n", archi_read32(I2C_TX_CFG));

	printf("write cmd buffer 0x%x\n", i2c_cmd_buffer);
	archi_write32(I2C_CMD_SADDR, i2c_cmd_buffer);
	// printf("0x%x\n", archi_read32(I2C_CMD_SADDR));

	printf("write cmd size %d (10)\n", seq_index << 2);
	archi_write32(I2C_CMD_SIZE, seq_index << 2);
	// printf("0x%x\n", archi_read32(I2C_CMD_SIZE));

	printf("write cmd cfg 0x%x\n", (0 << 6) |	  // CLR
									   (0 << 5) | // pending (read only)
									   (1 << 4) | // en
									   (0 << 0)	  // continous
	);
	archi_write32(I2C_CMD_CFG_ADDR, (0 << 6) |	   // CLR
										(0 << 5) | // pending (read only)
										(1 << 4) | // en
										(0 << 0)   // continous
	);
	// printf("0x%x\n", archi_read32(I2C_CMD_CFG_ADDR));

	printf("0x%x\n", archi_read32(I2C_CMD_CFG_ADDR));
	printf("0x%x\n", archi_read32(I2C_TX_CFG));
}


//============================
//
//	main
//
//============================
int main()
{
	printf("Entering I2C test\n");

	// RESET
	// archi_write32(I2C_SETUP, 0x00000001);
	// archi_write32(I2C_SETUP, 0x00000000);
	i2c_open();
	for (int i = 0; i < 100000; i++)
		;

	uint8_t data[3];
	data[0] = 0x11;
	data[1] = 0x55;
	data[2] = 0xff;

	i2c_write(0x13, 3, data);
	//i2c_read(0x13, 3, i2c_rx_buffer);

	// i2c_close();
	return 0;
}
