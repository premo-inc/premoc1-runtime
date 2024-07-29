#include "common.h"

#define BUFFER_SIZE 256

static PI_L2 char buffer[BUFFER_SIZE];

int main()
{
  int bootsel = archi_read32(SOC_BOOTSEL);

  archi_write32(DMA_CTRL_CFG_CG, archi_read32(DMA_CTRL_CFG_CG) | (1 << 5) | (1 << 1));

  while (1) {
    printf("\n<<< TX/RX TEST >>>\n");

    printf("\nBOOTSEL0 = %d\nBOOTSEL1 = %d\n", bootsel & 0x1, (bootsel & 0x2) >> 1);

    printf("\nSelect TX/RX\n");
    printf("0. TX\n");
    printf("1. RX\n");

    int mode = uart_getnum(0, 1);

    if (mode == 0) {
      test_tx(buffer, BUFFER_SIZE, bootsel);
    }

    if (mode == 1) {
      test_rx(buffer, BUFFER_SIZE, bootsel);
    }
  }

  return 0;
}
