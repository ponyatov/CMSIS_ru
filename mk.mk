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

## linker flags

# optimization
LDFLAGS += --gc-collect
