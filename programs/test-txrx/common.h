#ifndef __COMMON_H__
#define __COMMON_H__

#include <pulp.h>
#include <stdio.h>

#define SOC_BOOTSEL 0x1A1040C4

#define DMA_CTRL_CFG_CG	0x1A102000

#define HSS_PER_ID	5
#define HSS_BASE_ADDR	(0x1A102000+(HSS_PER_ID+1)*0x80)
#define HSS_RX_SADDR	(HSS_BASE_ADDR+0x00)
#define HSS_RX_SIZE	(HSS_BASE_ADDR+0x04)
#define HSS_RX_CFG	(HSS_BASE_ADDR+0x08)
#define HSS_TX_SADDR	(HSS_BASE_ADDR+0x10)
#define HSS_TX_SIZE	(HSS_BASE_ADDR+0x14)
#define HSS_TX_CFG	(HSS_BASE_ADDR+0x18)
#define HSS_STATUS	(HSS_BASE_ADDR+0x20)
#define HSS_SETUP	(HSS_BASE_ADDR+0x24)
#define HSS_ERROR	(HSS_BASE_ADDR+0x28)
#define HSS_IRQ_EN	(HSS_BASE_ADDR+0x2C)
#define HSS_VALID	(HSS_BASE_ADDR+0x30)
#define HSS_DATA	(HSS_BASE_ADDR+0x34)
#define HSS_APB_ADDR	(HSS_BASE_ADDR+0x40)
#define HSS_APB_DATA	(HSS_BASE_ADDR+0x44)
#define HSS_APB_CMD	(HSS_BASE_ADDR+0x48)

#define UART1_PER_ID	1
#define UART1_BASE_ADDR	(0x1A102000+(UART1_PER_ID+1)*0x80)
#define UART1_RX_SADDR	(UART1_BASE_ADDR+0x00)
#define UART1_RX_SIZE	(UART1_BASE_ADDR+0x04)
#define UART1_RX_CFG	(UART1_BASE_ADDR+0x08)
#define UART1_TX_SADDR	(UART1_BASE_ADDR+0x10)
#define UART1_TX_SIZE	(UART1_BASE_ADDR+0x14)
#define UART1_TX_CFG	(UART1_BASE_ADDR+0x18)
#define UART1_STATUS	(UART1_BASE_ADDR+0x20)
#define UART1_SETUP	(UART1_BASE_ADDR+0x24)
#define UART1_ERROR	(UART1_BASE_ADDR+0x28)
#define UART1_IRQ_EN	(UART1_BASE_ADDR+0x2C)
#define UART1_VALID	(UART1_BASE_ADDR+0x30)
#define UART1_DATA	(UART1_BASE_ADDR+0x34)
#define UART1_APB_ADDR	(UART1_BASE_ADDR+0x40)
#define UART1_APB_DATA	(UART1_BASE_ADDR+0x44)
#define UART1_APB_CMD	(UART1_BASE_ADDR+0x48)

static inline char uart_getchar()
{
  char c;

  uart_read(CONFIG_IO_UART_ITF, &c, 1);

  return c;
}

int uart_getnum(const int min, const int max);
int select_hw();
int select_rate(const int hw, const int bootsel);

void test_tx(char *buffer, const int buffer_size, const int bootsel);
void test_rx(char *buffer, const int buffer_size, const int bootsel);

#endif
