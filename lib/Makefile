## build static libraries for supported MCUs

include ../mk.mk

.PHONY: lib
lib: startup_stm32l073xx.o system_stm32l0xx.o

%.o: ../src/STM32/%.s
	$(CC) $(CFLAGS) -c -o $@ $<
%.o: ../src/STM32/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
