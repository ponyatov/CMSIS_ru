S += startup_stm32l073xx.s 
C += system_stm32l0xx.c
H += $(TOPDIR)/include/STM32/system_stm32l0xx.h

H += $(TOPDIR)/include/STM32/stm32l0xx.h $(TOPDIR)/include/STM32/stm32l073xx.h
H += $(TOPDIR)/include/STM32/core_cm0plus.h $(TOPDIR)/include/STM32/cmsis_gcc.h
H += $(TOPDIR)/include/STM32/core_cmInstr.h $(TOPDIR)/include/STM32/core_cmFunc.h

.PHONY: files
files: $(S) $(C) $(H)

%.s: ./STM32Cube_FW_L0_V1.10.0/Drivers/CMSIS/Device/ST/STM32L0xx/Source/Templates/gcc/%.s
	cp $< $@
%.c: ./STM32Cube_FW_L0_V1.10.0/Drivers/CMSIS/Device/ST/STM32L0xx/Source/Templates/%.c
	cp $< $@
$(TOPDIR)/include/STM32/%.h: ./STM32Cube_FW_L0_V1.10.0/Drivers/CMSIS/Device/ST/STM32L0xx/Include/%.h
	cp $< $@
$(TOPDIR)/include/STM32/%.h: ./STM32Cube_FW_L0_V1.10.0/Drivers/CMSIS/Include/%.h
	cp $< $@
