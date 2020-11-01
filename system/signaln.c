/* signaln.c - signaln */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  signaln  -  Signal a semaphore n times, releasing n waiting processes
 *------------------------------------------------------------------------
 */
syscall	signaln(
	  sid32		sem,		/* ID of semaphore to signal	*/
	  int32		count		/* Number of times to signal	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	SemaphoreEntry	*semptr;	/* Ptr to sempahore table entry */

	mask = disable();
	if (isBadSemaphoreID(sem) || (count < 0)) {
		restore(mask);
		return SYSERR;
	}
	semptr = &semtab[sem];
	if (semptr->state == S_FREE) {
		restore(mask);
		return SYSERR;
	}

	reschedule_cntl(DEFER_START);
	for (; count > 0; count--) {
		if ((semptr->count++) < 0) {
			ready(dequeue(semptr->waitingProcessQueue));
		}
	}
	reschedule_cntl(DEFER_STOP);
	restore(mask);
	return OK;
}
