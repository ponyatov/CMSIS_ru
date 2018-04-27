/* RAM-only debugging */

SECTIONS
{

	.vector : {
		*(.isr_vector)					/* interrupt table must be first */
	} > FLASH
	
	.text : {
		. = ALIGN(8);
		*(.text*)						/* all machine code sections */
		*(.eh_frame*)					/* C++ exceptions */
		KEEP (*(.init))					/* old-style startup code */
		KEEP (*(.fini))
		KEEP (*(.init_array*))			/* initializer/finalizers */
		KEEP (*(.fini_array*))
		. = ALIGN(8);
	} > RAM
	
	.rodata : {
		. = ALIGN(8);
		*(.rodata*)						/* constants to flash */
		. = ALIGN(8);
	}
	
}
