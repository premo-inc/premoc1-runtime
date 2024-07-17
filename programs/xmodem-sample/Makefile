PULP_APP = sample
PULP_APP_SRCS = sample.c
PULP_CFLAGS = -Os

PULP_ARCH_CFLAGS    =  -march=rv32imc -DRV_ISA_RV32
PULP_ARCH_LDFLAGS   =  -march=rv32imc
PULP_ARCH_OBJDFLAGS = -Mmarch=rv32imc

PULP_LINKER = link_xmodem.ld

include $(PULPRT_HOME)/rules/pulp.mk

clean:
	rm -f $(PULP_APP).raw
	rm -f $(PULP_APP).cv
	rm -f convert_raw
	rm -rf build

all:
	$(PULP_RISCV_GCC_TOOLCHAIN)/bin/riscv32-unknown-elf-objcopy -O binary build/$(PULP_APP)/$(PULP_APP) $(PULP_APP).raw --only-section=.text --only-section=.rodata --only-section=.data --only-section=.vectors --only-section=.preinit_array --only-section=.init_array --only-section=.fini_array

write:
	python3 ../../scripts/xmodem_uploader.py $(PORT) $(PULP_APP).raw

