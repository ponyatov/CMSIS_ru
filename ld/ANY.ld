/* Any CMSIS-compliant Cortex-M MCU must use this configs */

ENTRY(Reset_Handler)					/* reset handler (info for gdb) */

SECTIONS
{

	.data : {
		. = ALIGN(8);
		_sdata = .;						/* [s]tart [data] */
		*(.data*)
		_edata = .;						/* [e]nd [data] */
		. = ALIGN(8);
	} > RAM
	
	_sidata = LOADADDR(.data);			/* addr of .data in Flash (LMA) */
	
	.bss : {
		. = ALIGN(8);
		_sbss = . ;
		__bss_start__ = . ;
		*(.bss*)
		*(COMMON*)
		_ebss = . ;
		__bss_start__ = . ;
		. = ALIGN(8);
	} > RAM
	
	_estack = ORIGIN(RAM)+LENGTH(RAM);	/* end of .stack: for SP register */
	
	/DISCARD/ : {						/* drop unwanted sections */
	}

}
