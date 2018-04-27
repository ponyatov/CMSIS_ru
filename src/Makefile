ELF = hello.elf

include ../mk.mk

LIB = ../lib/startup_stm32l073xx.o ../lib/libSTM32.a

hello.elf: hello.c $(LIB)
	$(CC) $(CFLAGS) -Wl,-T,../ld/$(MCU).ld -o $@ $< -lSTM32 

$(LIB):
	cd ../lib ; $(MAKE) TOPDIR=$(TOPDIR)
