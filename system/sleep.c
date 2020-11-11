/* sleep.c - sleep sleepms */

#include <xinu.h>

// #undef kprintf

#define	MAXSECONDS	2147483		/* Max seconds per 32-bit msec	*/

/*------------------------------------------------------------------------
 *  sleep  -  Delay the calling process n seconds
 *------------------------------------------------------------------------
 */
syscall	sleep(
	  int32	delay		/* Time to delay in seconds	*/
	)
{
	if ( (delay < 0) || (delay > MAXSECONDS) ) {
		return SYSERR;
	}
	sleepms(1000*delay);
	return OK;
}

/*------------------------------------------------------------------------
 *  sleepms  -  Delay the calling process n milliseconds
 *------------------------------------------------------------------------
 */
syscall	sleepms(
	  int32	delay			/* Time to delay in msec.	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/

	if (delay < 0) {
		return SYSERR;
	}

	mask = disable();

	kprintf("sleepms: process [%s] entered sleep\n", processTable[currentProcessID].processName);

	if (delay > 0) {
		/* Delay calling process */

		if (queueInsertd(currentProcessID, sleepQueue, delay) == SYSERR) {
			restore(mask);
			return SYSERR;
		}

		processTable[currentProcessID].state = PR_SLEEP;
		reschedule();
	}

	restore(mask);
	return OK;
}
