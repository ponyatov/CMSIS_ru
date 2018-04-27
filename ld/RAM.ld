/* RAM-only debugging */

SECTIONS
{
	.text : {
		. = ALIGN(8);
		*(.text*)						/* all machine code sections */
		*(.eh_frame*)					/* C++ exceptions */
		. = ALIGN(8);
	} > RAM
}
