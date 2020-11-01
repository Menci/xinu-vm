/* semdelete.c - semdelete */

#include <xinu.h>

/*------------------------------------------------------------------------
 * semdelete  -  Delete a semaphore by releasing its table entry
 *------------------------------------------------------------------------
 */
syscall	semdelete(
	  sid32		sem		/* ID of semaphore to delete	*/
	)
{
	intmask mask;			/* Saved interrupt mask		*/
	struct	SemaphoreEntry *semptr;		/* Ptr to semaphore table entry	*/

	mask = disable();
	if (isBadSemaphoreID(sem)) {
		restore(mask);
		return SYSERR;
	}
	
	semptr = &semtab[sem];
	if (semptr->state == S_FREE) {
		restore(mask);
		return SYSERR;
	}
	semptr->state = S_FREE;

	reschedule_cntl(DEFER_START);
	while (semptr->count++ < 0) {	/* Free all waiting processes	*/
		ready(queuePopFirst(semptr->waitingProcessQueue));
	}
	reschedule_cntl(DEFER_STOP);
	restore(mask);
	return OK;
}
