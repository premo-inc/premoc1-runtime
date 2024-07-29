#include "common.h"

int uart_getnum(const int min, const int max)
{
  printf("> ");

  int num;
  do {
    num = uart_getchar() - '0';
  } while (num < min || num > max);

  printf("%d\n", num);

  return num;
}

int select_hw()
{
  int hw;

  printf("\nSelect HW\n\n");
  printf("0. HSS\n");
  printf("1. UART1\n");

  hw = uart_getnum(0, 1);

  return hw;
}

int select_rate(const int hw, const int bootsel)
{
  int rate;

  if (hw == 0) {
    printf("\nSelect HSS rate\n\n");

    int i;
    for (i = 0; i < 3; i++) {
      printf("%d. %s/%d M\n", i, (bootsel & 0x1 == 1) ? "25" : "100", 1 << i);
    }

    rate = uart_getnum(0, 2);
  }

  if (hw == 1) {
    printf("\nSelect UART rate\n\n");

    int i;
    for (i = 0; i < 6; i++) {
      printf("%d. %s/%d M\n", i, (bootsel & 0x1 == 1) ? "1.5625" : "25", 1 << (i + 1));
    }

    rate = uart_getnum(0, 5);

    rate = 0x1 << (rate + 1);
  }

  return rate;
}
