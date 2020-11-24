/* doContextSwitch.S - doContextSwitch (for x86) */

		.text
		.globl	doContextSwitch

/*------------------------------------------------------------------------
 * doContextSwitch -  X86 context switch;
 * the call is doContextSwitch(&old_sp, &new_sp, &new_page_directory, deallocateOldVirtualMemorySpace)
 *------------------------------------------------------------------------
 */
doContextSwitch:
		pushl	%ebp		/* Push ebp onto stack		*/
		movl	%esp, %ebp	/* Record current SP in ebp	*/
		pushfl			/* Push flags onto the stack	*/
		pushal			/* Push general regs. on stack	*/

		/* Save old segment registers here, if multiple allowed */

		movl	8(%ebp), %eax	/* Get mem location in which to	*/
					/*   save the old process's SP	*/
		movl	%esp, (%eax)	/* Save old process's SP	*/
		movl	12(%ebp), %ebx	/* Get location from which to	*/
					/*   restore new process's SP	*/
		movl    16(%ebp), %eax
		movl	%cr3, %ecx  /* Save old page directory */
		movl	20(%ebp), %edx /* Save whether to deallocate the old virtual memory space */
		movl    %eax, %cr3  /* Switch to new page directory */

		/* The next instruction switches from the old process's	*/
		/*   stack to the new process's stack.			*/

		movl	(%ebx), %esp	/* Pop up new process's SP	*/

		/* Check whether the old virtual memory space needs to be deallocated */
		testl	$1, %edx		/* whether to deallocate the old virtual memory space is in edx */
		jz		.skipDeallocate
		push	%ecx			/* Old virtual memory space is in %ecx */
		call	deallocateVirtualMemorySpace
		addl	$4, %esp

.skipDeallocate:
		/* Restore new seg. registers here, if multiple allowed */

		popal			/* Restore general registers	*/
		movl	4(%esp), %ebp	/* Pick up ebp before restoring	*/
					/*   interrupts			*/
		popfl			/* Restore interrupt mask	*/
		add	$4, %esp		/* Skip saved value of ebp	*/
		ret			/* Return to new process	*/
