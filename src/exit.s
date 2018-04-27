/* _exit function stub */

  .syntax unified

		.global exit
exit:
		cpsid if			// disable interrupts
		b exit
