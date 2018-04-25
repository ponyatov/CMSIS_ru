/* LM3S811 /qemu-arm/ */

MEMORY
{
	FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 64K
	  RAM (rwx) : ORIGIN = 0x20000000, LENGTH =  8K
}

SECTIONS
{

	.isr_vector : {
		. = ALIGN(8);
		_sint = .;
		KEEP( *(.isr_vector) )		/* interrupt vector table */
		. = ALIGN(8);
		_eint = .;
	} > FLASH						/* must be load to ROM:00000000 */
	
}

INCLUDE ld/ANY_RAM.ld
INCLUDE ld/ANY.ld
