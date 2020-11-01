/* stacktrace.c - stacktrace */
#include <xinu.h>

#define STKDETAIL

/*------------------------------------------------------------------------
 * stacktrace - print a stack backtrace for a process
 *------------------------------------------------------------------------
 */
syscall stacktrace(int pid)
{
	struct ProcessEntry	*proc = &processTable[pid];
	unsigned long	*sp, *fp;

	if (pid != 0 && isBadProcessID(pid))
		return SYSERR;
	if (pid == currentProcess) {
		asm("movl %%esp, %0\n" :"=r"(sp));
		asm("movl %%ebp, %0\n" :"=r"(fp));
	} else {
		sp = (unsigned long *)proc->stackPointer;
		fp = sp + 2; 		/* where doContextSwitch leaves it */
	}
	kprintf("sp %X fp %X proc->stackPointerBase %X\n", sp, fp, proc->stackPointerBase);
#ifdef STKDETAIL
	while (sp < (unsigned long *)proc->stackPointerBase) {
		for (; sp < fp; sp++)
			kprintf("DATA (%08X) %08X (%d)\n", sp, *sp, *sp);
		if (*sp == STACKMAGIC)
			break;
		kprintf("\nFP   (%08X) %08X (%d)\n", sp, *sp, *sp);
		fp = (unsigned long *) *sp++;
		if (fp <= sp) {
			kprintf("bad stack, fp (%08X) <= sp (%08X)\n", fp, sp);
			return SYSERR;
		}
		kprintf("RET  0x%X\n", *sp);
		sp++;
	}
	kprintf("STACKMAGIC (should be %X): %X\n", STACKMAGIC, *sp);
	if (sp != (unsigned long *)proc->stackPointerBase) {
		kprintf("unexpected short stack\n");
		return SYSERR;
	}
#endif
	return OK;
}