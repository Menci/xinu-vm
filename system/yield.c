/* yield.c - yield */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  yield  -  Voluntarily relinquish the CPU (end a timeslice)
 *------------------------------------------------------------------------
 */
syscall	yield(void)
{
	intmask	mask;			/* Saved interrupt mask		*/

	mask = disable();

	// Force a preemption
	preempt = 0;
	proctab[currpid].prcurrtimeslice = 0;
	resched();

	restore(mask);
	return OK;
}
