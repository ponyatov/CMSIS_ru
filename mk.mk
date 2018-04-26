# shared Makefile part

TARGET = arm-none-eabi

## GNU gcc C compiler
CC = $(TARGET)-gcc
## linker
LD = $(TARGET)-ld
## assembler
AS = $(TARGET)-as
## objdump: dump object files in .ELF format
OBJDUMP = $(TARGET)-objdump

# debug symbols enabled
CFLAGS += -g
# optimization 
CFLAGS += 