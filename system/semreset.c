/* semreset.c - semreset */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  semreset  -  Reset a semaphore's count and release waiting processes
 *------------------------------------------------------------------------
 */
syscall	semreset(
	  sid32		sem,		/* ID of semaphore to reset	*/
	  int32		count		/* New count (must be >= 0)	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	SemaphoreEntry *semptr;		/* Ptr to semaphore table entry */
	qid16	semqueue;		/* Semaphore's process queue ID	*/
	pid32	pid;			/* ID of a waiting process	*/

	mask = disable();

	if (count < 0 || isBadSemaphoreID(sem) || semtab[sem].state==S_FREE) {
		restore(mask);
		return SYSERR;
	}
	
	semptr = &semtab[sem];
	semqueue = semptr->waitingProcessQueue;	/* Free any waiting processes */
	reschedule_cntl(DEFER_START);
	while ((pid=queuePopFirst(semqueue)) != EMPTY)
		ready(pid);
	semptr->count = count;		/* Reset count as specified */
	reschedule_cntl(DEFER_STOP);
	restore(mask);
	return OK;
}
