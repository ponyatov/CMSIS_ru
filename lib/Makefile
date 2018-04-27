## build static libraries for supported MCUs

include ../mk.mk

.PHONY: lib
lib: startup_stm32l073xx.o libSTM32.a

libSTM32.a: system_stm32l0xx.o exit.o
	$(AR) cr $@ $^

%.o: ../src/STM32/%.s
	$(CC) $(CFLAGS) -c -o $@ $<
%.o: ../src/STM32/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: ../src/%.s
	$(CC) $(CFLAGS) -c -o $@ $<
%.o: ../src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
