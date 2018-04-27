include ../mk.mk

LIB = $(TOPDIR)/lib/startup_stm32l073xx.o $(TOPDIR)/lib/system_stm32l0xx.o

hello.elf: hello.c $(LIB)
	$(CC) $(CFLAGS) -Wl,-T,../ld/$(MCU).ld -o $@ $<

$(LIB):
	cd $(TOPDIR)/lib ; $(MAKE) TOPDIR=$(TOPDIR)
