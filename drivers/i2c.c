#include <stdio.h>
#include "pulp.h"

uint32_t i2c_cmd_buffer[16];

static void i2c_wait_rx_done()
{
  while ((archi_read32(UDMA_I2C_RX_CFG_ADDR(0)) & 0x10) != 0)
    ;
}

static void i2c_wait_cmd_done()
{
  while ((archi_read32(UDMA_I2C_CMD_CFG_ADDR(0)) & 0x10) != 0)
    ;
}

void i2c_open()
{
  int periph_id = ARCHI_UDMA_I2C_ID(0);
  plp_udma_cg_set(plp_udma_cg_get() | (1 << periph_id));
}

void i2c_close()
{
  int periph_id = ARCHI_UDMA_I2C_ID(0);
  plp_udma_cg_set(plp_udma_cg_get() & ~(1 << periph_id));
}

void i2c_read(uint8_t addr, uint8_t length, uint8_t *data)
{
  // only i2c0, no i2c1 for premo c1 board
  int seq_index = 0;
  int _div = 25000000 / 100000;
  i2c_cmd_buffer[seq_index++] = ((I2C_CMD_CFG) |
                                 _div // clock divider position 0-15
  );
  i2c_cmd_buffer[seq_index++] = ((I2C_CMD_START));
  i2c_cmd_buffer[seq_index++] = ((I2C_CMD_WRB) | addr << 1 | 0x1);

  if (length > 1)
  {
    i2c_cmd_buffer[seq_index++] = ((I2C_CMD_RPT) | length - 1);
    i2c_cmd_buffer[seq_index++] = ((I2C_CMD_RD_ACK));
  }
  i2c_cmd_buffer[seq_index++] = ((I2C_CMD_RD_NACK));
  i2c_cmd_buffer[seq_index++] = ((I2C_CMD_STOP));

  // set RX buffer
  archi_write32(UDMA_I2C_RX_ADDR(0), data);
  archi_write32(UDMA_I2C_RX_SIZE_ADDR(0), length);
  archi_write32(UDMA_I2C_RX_CFG_ADDR(0), (0 << 6) |     // CLR
                                             (0 << 5) | // pending (read only)
                                             (1 << 4) | // en
                                             (0 << 0)   // continous
  );

  // set command buffer
  archi_write32(UDMA_I2C_CMD_ADDR(0), i2c_cmd_buffer);
  archi_write32(UDMA_I2C_CMD_SIZE_ADDR(0), seq_index << 2);
  archi_write32(UDMA_I2C_CMD_CFG_ADDR(0), (0 << 6) |     // CLR
                                              (0 << 5) | // pending (read only)
                                              (1 << 4) | // en
                                              (0 << 0)   // continous
  );

  i2c_wait_rx_done();

  i2c_wait_cmd_done();

  // debug print
  for (int i = 0; i < length; i++)
  {
    printf("data[%d] = 0x%x\n", i, data[i]);
  }
}

void i2c_write(uint8_t addr, uint8_t length, uint8_t *data)
{
  // only i2c0, no i2c1 for premo c1 board
  int seq_index = 0;
  int _div = 25000000 / 100000;

  i2c_cmd_buffer[seq_index++] = ((I2C_CMD_CFG) |
                                 _div // clock divider position 0-15
  );

  i2c_cmd_buffer[seq_index++] = ((I2C_CMD_START));
  i2c_cmd_buffer[seq_index++] = ((I2C_CMD_WRB) | addr << 1 | 0x0);
  for (int i = 0; i < length; i++)
  {
    i2c_cmd_buffer[seq_index++] = ((I2C_CMD_WRB) | data[i]);
  }

  i2c_cmd_buffer[seq_index++] = ((I2C_CMD_STOP));

  for (int i = 0; i < seq_index; i++)
  {
    printf("cmd[%d] = 0x%x\n", i, i2c_cmd_buffer[i]);
  }

  archi_write32(UDMA_I2C_CMD_ADDR(0), i2c_cmd_buffer);
  archi_write32(UDMA_I2C_CMD_SIZE_ADDR(0), seq_index << 2);
  archi_write32(UDMA_I2C_CMD_CFG_ADDR(0), (0 << 6) |     // CLR
                                              (0 << 5) | // pending (read only)
                                              (1 << 4) | // en
                                              (0 << 0)   // continous
  );

  i2c_wait_cmd_done();
}
