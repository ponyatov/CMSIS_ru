target remote :4242
displ /i $pc
displ /x $sp
displ
b Reset_Handler
b SystemInit
