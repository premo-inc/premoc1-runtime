#include "common.h"

static void dump_reg(const int hw)
{
  printf("\n");
  printf("HSS_SETUP     : 0x%08x\n", archi_read32(HSS_SETUP));

  if (hw == 0) {
    printf("HSS_TX_SADDR  : 0x%08x\n", archi_read32(HSS_TX_SADDR));
    printf("HSS_TX_SIZE   : 0x%08x\n", archi_read32(HSS_TX_SIZE));
    printf("HSS_TX_CFG    : 0x%08x\n", archi_read32(HSS_TX_CFG));
  }

  if (hw == 1) {
    printf("UART1_SETUP   : 0x%08x\n", archi_read32(UART1_SETUP));
    printf("UART1_TX_SADDR: 0x%08x\n", archi_read32(UART1_TX_SADDR));
    printf("UART1_TX_SIZE : 0x%08x\n", archi_read32(UART1_TX_SIZE));
    printf("UART1_TX_CFG  : 0x%08x\n", archi_read32(UART1_TX_CFG));
  }
}

static void loop(const int hw, const char *buffer, const int buffer_size)
{
  while (1) {
    printf("\n[Ctrl-C] Finish  [D] Toggle Debug TX  [I] Toggle TX INV EN  [SPACE] DUMP\n");

    char key = uart_getchar();

    if (key == 0x03) {
      break;
    }

    if (key == 'd' || key == 'D') {
      archi_write32(HSS_SETUP, archi_read32(HSS_SETUP) ^ (1 << 13));
      printf("\nDebug TX = %d\n", (archi_read32(HSS_SETUP) & (1 << 13)) >> 13);
    }

    if (key == 'i' || key == 'I') {
      archi_write32(HSS_SETUP, archi_read32(HSS_SETUP) ^ (1 << 11));
      printf("\nTX INV EN = %d\n", (archi_read32(HSS_SETUP) & (1 << 11)) >> 11);
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

void test_tx(char *buffer, const int buffer_size, const int bootsel)
{
  int hw;
  hw = select_hw();

  int rate;
  rate = select_rate(hw, bootsel);

  printf("\nSelect data\n\n");
  printf("0. 5A 5A ... 5A 5A\n");
  printf("1. 00 00 ... 00 00\n");
  printf("2. FF FF ... FF FF\n");
  printf("3. 00 01 ... FE FF\n");

  int data;
  data = uart_getnum(0, 3);

  if (data == 0) {
    memset(buffer, 0x5A, buffer_size);
  }
  if (data == 1) {
    memset(buffer, 0x00, buffer_size);
  }
  if (data == 2) {
    memset(buffer, 0xFF, buffer_size);
  }
  if (data == 3) {
    int i;
    for (i = 0; i < buffer_size; i++) {
      buffer[i] = i & 0xFF;
    }
  }

  if (hw == 0) {
    archi_write32(HSS_TX_SADDR, buffer); // 16bit offset to l2_data
    archi_write32(HSS_TX_SIZE, buffer_size);

    archi_write32(HSS_SETUP,
		  (rate << 16) | // CLKDIV 0:100MHz 1:50MHz 2:25MHz
		  (0    << 15) | // ALT UART EN
		  (0    << 14) | // Debug RX
		  (0    << 13) | // Debug TX
		  (0    << 12) | // RX INV_EN
		  (0    << 11) | // TX INV_EN
		  (1    << 10) | // Loopback Enable (0:Enable)
		  (0    <<  9) | // RX_EN
		  (1    <<  8) | // TX_EN
		  (0    <<  5) | // Clean RX FIFO
		  (0    <<  4) | // Polling Enable
		  (0    <<  3) | // Stop bit (0:1bit 1:2bit）
		  (1    <<  1) | // 8B10B Enable
		  (0    <<  0)); // Parity Enable

    dump_reg(hw);

    printf("\nSTART HSS TX\n");

    archi_write32(HSS_TX_CFG,
		  (0 << 6) | // CLR
		  (0 << 5) | // pending
		  (1 << 4) | // en
		  (1 << 0)); // continous

    loop(hw, buffer, buffer_size);

    printf("\nSTOP HSS TX\n");

    archi_write32(HSS_TX_CFG,
		  (1 << 6) | // CLR
		  (0 << 5) | // pending
		  (0 << 4) | // en
		  (0 << 0)); // continous

    dump_reg(hw);
  }

  if (hw == 1) {
    archi_write32(UART1_TX_SADDR, buffer); // 16bit offset to l2_data
    archi_write32(UART1_TX_SIZE, buffer_size);

    archi_write32(HSS_SETUP,
		  (0    << 16) | // CLKDIV 0:100MHz 1:50MHz 2:25MHz
		  (1    << 15) | // ALT UART EN
		  (0    << 14) | // Debug RX
		  (0    << 13) | // Debug TX
		  (0    << 12) | // RX INV_EN
		  (0    << 11) | // TX INV_EN
		  (1    << 10) | // Loopback Enable (0:Enable)
		  (0    <<  9) | // RX_EN
		  (1    <<  8) | // TX_EN
		  (0    <<  5) | // Clean RX FIFO
		  (0    <<  4) | // Polling Enable
		  (0    <<  3) | // Stop bit (0:1bit 1:2bit）
		  (0    <<  1) | // 8B10B Enable
		  (0    <<  0)); // Parity Enable

    archi_write32(UART1_SETUP,
		  (rate << 16) | // DIV d6の時115kHz 
		  (0    <<  9) | // RX_EN
		  (1    <<  8) | // TX_EN
		  (0    <<  5) | // Clean RX FIFO
		  (0    <<  4) | // Polling Enable
		  (0    <<  3) | // Stop bit (0:1bit 1:2bit)
		  (2    <<  1) | // Bit (0:5bit 1:6bit 2:7bit 3:8bit)
		  (1	<<  0)); // Parity Enable

    dump_reg(hw);

    printf("\n-- START UART TX\n");

    archi_write32(UART1_TX_CFG,
		  (0 << 6) | // CLR
		  (0 << 5) | // pending
		  (1 << 4) | // en
		  (1 << 0)); // continous

    loop(hw, buffer, buffer_size);

    printf("\n-- STOP UART TX\n");

    archi_write32(UART1_TX_CFG,
		  (1 << 6) | // CLR
		  (0 << 5) | // pending
		  (0 << 4) | // en
		  (0 << 0)); // continous

    dump_reg(hw);
  }
}
