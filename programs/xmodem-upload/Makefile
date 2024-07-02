PULP_APP = xmodem-upload
PULP_APP_SRCS = main.c xmodem.c crc16.c
PULP_CFLAGS = -Os

PULP_ARCH_CFLAGS    =  -march=rv32imc -DRV_ISA_RV32
PULP_ARCH_LDFLAGS   =  -march=rv32imc
PULP_ARCH_OBJDFLAGS = -Mmarch=rv32imc

PULP_LINKER = link_flash.ld

include $(PULPRT_HOME)/rules/pulp.mk

clean:
	rm -f $(PULP_APP).raw
	rm -f $(PULP_APP).cv
	rm -f convert_raw
	rm -rf build

# Write raw program (load to ram)
write:
	$(PULP_RISCV_GCC_TOOLCHAIN)/bin/riscv32-unknown-elf-objcopy -O binary build/$(PULP_APP)/$(PULP_APP) $(PULP_APP).raw --only-section=.text --only-section=.rodata --only-section=.data --only-section=.vectors --only-section=.preinit_array --only-section=.init_array --only-section=.fini_array
	gcc ../../scripts/convert_raw.c -o ../../scripts/convert_raw
	../../scripts/convert_raw $(PULP_APP).raw $(PULP_APP).cv
	python3 ../../scripts/write_telnet.py localhost 4444 0x1a000080 ./$(PULP_APP).cv
