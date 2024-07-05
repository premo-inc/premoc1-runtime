#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define BOOT_SELECTOR

FILE *in, *out;
uint32_t write_size = 0;

void DataWrite(uint32_t data) {
  fwrite(&data, sizeof(data), 1, out);
  write_size += sizeof(data);
}

// Use register: t0, t1; t1 = save address; t1 += 4
void WriteWordToRAM(uint32_t data) {
  int negative = (data & 0x00000800) == 0x800 ? 0x1000 : 0;
  DataWrite(((data & 0xfffff000) + negative) | 0x2b7); // lui t0, $(value[32:12] + 1)
  DataWrite(((data & 0x00000fff) << 20)| 0x28293); // addi t0, t0, $(value[11:0])
  DataWrite(0x00000013); // nop
  DataWrite(0x00532023); // sw t0, 0(t1)
  DataWrite(0x00430313); // addi t1, t1, 4
}

int main(int argc, char *argv[]) {
  if (argc != 3)
    return -1;

  in = fopen(argv[1], "rb");
  out = fopen(argv[2], "wb");
  if (!in || !out)
    return -1;

  uint32_t fsize;
  fseek(in, 0, SEEK_END);
  fsize = (uint32_t)ftell(in);
  fseek(in, 0, SEEK_SET);

  DataWrite(0x1c008337); // lui t1, 0x1c008

  uint32_t RAM[] = {
#ifdef BOOT_SELECTOR
    0x1a1013b7, // lui t2, 0x1a101
    0x0ff00313, // addi t1, x0, 0xff
    0x00638023, // sb t1, 0(t2)
    0x00638223, // sb t1, 4(t2)
    0x00000013, // nop
    0x00838303, // lb t1, 8(t2)
    0x00137313, // andi t1, t1, 1
    0x00c31313, // slli t1, t1, 12
    0x006282b3, // add t0, t0, t1
#endif
    0x0002a383, // lw t2, 0(t0)
    0x005383b3, // add t2, t2, t0
    0x0042a303, // lw t1, 4(t0) <----|
    0x00428293, // addi t0, t0, 4    |
    0x006e2023, // sw t1, 0(t3)      |
    0x004e0e13, // addi t3, t3, 4    |
    0xfe7298e3, // bne t0, t2, -16 --|
    0x1c000337, // lui t1, 0x1c000
    0x00030067, // jr t1
  };

  for (int i = 0; i < sizeof(RAM) / sizeof(RAM[0]); i++) {
    WriteWordToRAM(RAM[i]);
  }

  DataWrite(0x1a0002b7); // lui t0, 0x1a000
  DataWrite(0x40028293); // addi t0, t0, 0x400 /* t0 = program address, 0x400 = 0x80 + 0x380 */
  DataWrite(0x00000013); // nop
  int negative = (fsize & 0x00000800) == 0x800 ? 0x1000 : 0;
  DataWrite(((fsize & 0xfffff000) + negative) | 0x3b7); // lui t2, $(value[32:12] + 1)
  DataWrite(((fsize & 0x00000fff) << 20)| 0x38393); // addi t2, t2, $(value[11:0])
  DataWrite(0x1c000e37); // lui t3, 0x1c000
  DataWrite(0x1c008337); // lui t1, 0x1c008
  DataWrite(0x00030067); // jr t1

  while (write_size < 0x380) {
    DataWrite(0);
  }

  DataWrite(fsize); // Program size

  uint32_t read;
  while (fread(&read, sizeof(read), 1, in) > 0) {
    DataWrite(read);
  }

  fclose(in);
  fclose(out);

  return 0;
}

