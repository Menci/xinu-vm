/* signal.c - signal */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  signal  -  Signal a semaphore, releasing a process if one is waiting
 *------------------------------------------------------------------------
 */
syscall	signal(
	  sid32		sem		/* ID of semaphore to signal	*/
	)
{
	intmask mask;			/* Saved interrupt mask		*/
	struct	SemaphoreEntry *semptr;		/* Ptr to sempahore table entry	*/

	mask = disable();
	if (isBadSemaphoreID(sem)) {
		restore(mask);
		return SYSERR;
	}
	semptr= &semtab[sem];
	if (semptr->state == S_FREE) {
		restore(mask);
		return SYSERR;
	}
	if ((semptr->count++) < 0) {	/* Release a waiting process */
		ready(dequeue(semptr->waitingProcessQueue));
	}
	restore(mask);
	return OK;
}
