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
	msBeforePreempt = 0;
	processTable[currentProcessID].currentTimeSlice = 0;
	reschedule();

	restore(mask);
	return OK;
}
