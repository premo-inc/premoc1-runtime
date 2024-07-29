#include "common.h"

static void dump_reg(const int hw)
{
  printf("\n");
  printf("HSS_SETUP     : 0x%08x\n", archi_read32(HSS_SETUP));

  if (hw == 0) {
    printf("HSS_RX_SADDR  : 0x%08x\n", archi_read32(HSS_RX_SADDR));
    printf("HSS_RX_SIZE   : 0x%08x\n", archi_read32(HSS_RX_SIZE));
    printf("HSS_RX_CFG    : 0x%08x\n", archi_read32(HSS_RX_CFG));
    printf("HSS_STATUS    : 0x%08x\n", archi_read32(HSS_STATUS));
    printf("HSS_ERROR     : 0x%08x\n", archi_read32(HSS_ERROR));
  }

  if (hw == 1) {
    printf("UART1_SETUP   : 0x%08x\n", archi_read32(UART1_SETUP));
    printf("UART1_RX_SADDR: 0x%08x\n", archi_read32(UART1_RX_SADDR));
    printf("UART1_RX_SIZE : 0x%08x\n", archi_read32(UART1_RX_SIZE));
    printf("UART1_RX_CFG  : 0x%08x\n", archi_read32(UART1_RX_CFG));
    printf("UART1_STATUS  : 0x%08x\n", archi_read32(UART1_STATUS));
    printf("UART1_ERROR   : 0x%08x\n", archi_read32(UART1_ERROR));
  }
}

static void loop(const int hw, const char *buffer, const int buffer_size)
{
  while (1) {
    printf("\n[Ctrl-C] Finish  [D] Toggle Debug RX  [I] Toggle RX INV EN  [SPACE] DUMP\n");

    char key = uart_getchar();

    if (key == 0x03) {
      break;
    }

    if (key == 'd' || key == 'D') {
      archi_write32(HSS_SETUP, archi_read32(HSS_SETUP) ^ (1 << 14));
      printf("\nDebug RX = %d\n", (archi_read32(HSS_SETUP) & (1 << 14)) >> 14);
    }

    if (key == 'i' || key == 'I') {
      archi_write32(HSS_SETUP, archi_read32(HSS_SETUP) ^ (1 << 12));
      printf("\nRX INV EN = %d\n", (archi_read32(HSS_SETUP) & (1 << 12)) >> 12);
    }

    if (key == ' ') {
      dump_reg(hw);

      int i;
      for (i = 0; i < buffer_size; i++) {
	if (i % 16 == 0) {
	  printf("\n");
	}
	printf("%02x ", buffer[i]);
      }

      printf("\n");
    }
  }
}

void test_rx(char *buffer, const int buffer_size, const int bootsel)
{
  int hw;
  hw = select_hw();

  int rate;
  rate = select_rate(hw, bootsel);

  memset(buffer, 0, buffer_size);

  if (hw == 0) {
    archi_write32(HSS_RX_SADDR, buffer); // 16bit offset to l2_data
    archi_write32(HSS_RX_SIZE, buffer_size);

    archi_write32(HSS_SETUP,
		  (rate << 16) | // CLKDIV 0:100MHz 1:50MHz 2:25MHz
		  (0    << 15) | // ALT UART EN
		  (0    << 14) | // Debug RX
		  (0    << 13) | // Debug TX
		  (0    << 12) | // RX INV_EN
		  (0    << 11) | // TX INV_EN
		  (1    << 10) | // Loopback Enable (0:Enable)
		  (1    <<  9) | // RX_EN
		  (0    <<  8) | // TX_EN
		  (0    <<  5) | // Clean RX FIFO
		  (0    <<  4) | // Polling Enable
		  (0    <<  3) | // Stop bit (0:1bit 1:2bit）
		  (1    <<  1) | // 8B10B Enable
		  (0    <<  0)); // Parity Enable

    dump_reg(hw);

    printf("\nSTART HSS RX\n");

    archi_write32(HSS_RX_CFG,
		  (0 << 6) | // CLR
		  (0 << 5) | // pending
		  (1 << 4) | // en
		  (1 << 0)); // continous

    loop(hw, buffer, buffer_size);
      
    printf("\nSTOP HSS RX\n");

    archi_write32(HSS_RX_CFG,
		  (1 << 6) | // CLR
		  (0 << 5) | // pending
		  (0 << 4) | // en
		  (0 << 0)); // continous

    dump_reg(hw);
  }

  if (hw == 1) {
    archi_write32(UART1_RX_SADDR, buffer); // 16bit offset to l2_data
    archi_write32(UART1_RX_SIZE, buffer_size);

    archi_write32(HSS_SETUP,
		  (0    << 16) | // CLKDIV 0:100MHz 1:50MHz 2:25MHz
		  (1    << 15) | // ALT UART EN
		  (0    << 14) | // Debug RX
		  (0    << 13) | // Debug TX
		  (0    << 12) | // RX INV_EN
		  (0    << 11) | // TX INV_EN
		  (1    << 10) | // Loopback Enable (0:Enable)
		  (1    <<  9) | // RX_EN
		  (0    <<  8) | // TX_EN
		  (0    <<  5) | // Clean RX FIFO
		  (0    <<  4) | // Polling Enable
		  (0    <<  3) | // Stop bit (0:1bit 1:2bit）
		  (0    <<  1) | // 8B10B Enable
		  (0    <<  0)); // Parity Enable

    archi_write32(UART1_SETUP,
		  (rate << 16) | // DIV d6の時115kHz 
		  (1    <<  9) | // RX_EN
		  (0    <<  8) | // TX_EN
		  (0    <<  5) | // Clean RX FIFO
		  (0    <<  4) | // Polling Enable
		  (0    <<  3) | // Stop bit (0:1bit 1:2bit)
		  (3    <<  1) | // Bit (0:5bit 1:6bit 2:7bit 3:8bit)
		  (0	<<  0)); // Parity Enable

    dump_reg(hw);

    printf("\n-- START UART RX\n");

    archi_write32(UART1_RX_CFG,
		  (0 << 6) | // CLR
		  (0 << 5) | // pending
		  (1 << 4) | // en
		  (1 << 0)); // continous

    loop(hw, buffer, buffer_size);
      
    printf("\n-- STOP UART RX\n");

    archi_write32(UART1_RX_CFG,
		  (1 << 6) | // CLR
		  (0 << 5) | // pending
		  (0 << 4) | // en
		  (0 << 0)); // continous

    dump_reg(hw);
  }
}
