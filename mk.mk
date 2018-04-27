# shared Makefile part

TARGET = arm-none-eabi

## GNU gcc C compiler
CC = $(TARGET)-gcc
## linker
LD = $(TARGET)-ld
## assembler
AS = $(TARGET)-as
## dump object files in .ELF format (headers, sections,..)
OBJDUMP = $(TARGET)-objdump
## convert obejct file formats (elf->hex)
OBJCOPY = $(TARGET)-objcopy

## C compiler flags

# debug symbols enabled
CFLAGS += -g
# optimization 
CFLAGS += -Os -ffunction-sections
# include directories
CFLAGS += -I$(TOPDIR)/include
# libraries
CFLAGS += -L$(TOPDIR)/lib

## target MCU compiler flags
#  any Cortex-M
CFLAGS += -mthumb -D$(MCU) -DSTM32L073xx -DSTM32L0
# generic Cortex-M0
CFLAGS += -mfloat-abi=soft -mcpu=cortex-m0

## linker flags

# optimization
LDFLAGS += --gc-collect

## emulator/debugger

QEMU = qemu-system-arm -M lm3s811evb -S -gdb tcp::4242 -kernel
GDB  = $(TARGET)-gdb

.PHONY: default
default: $(ELF)

.PHONY: emu
emu: $(ELF)
	$(QEMU) $<

.PHONY: deb
deb: $(ELF)
	ddd --debugger "$(GDB) -x ram.gdb $<"
