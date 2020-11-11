/* wait.c - wait */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  wait  -  Cause current process to wait on a semaphore
 *------------------------------------------------------------------------
 */
syscall	wait(
	  sid32		sem		/* Semaphore on which to wait  */
	)
{
	intmask mask;			/* Saved interrupt mask		*/
	struct	ProcessEntry *prptr;		/* Ptr to process's table entry	*/
	struct	SemaphoreEntry *semptr;		/* Ptr to sempahore table entry	*/

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

	if (--(semptr->count) < 0) {		/* If caller must block	*/
		prptr = &processTable[currentProcessID];
		prptr->state = PR_WAIT;	/* Set state to waiting	*/
		prptr->waitingSemaphore = sem;		/* Record semaphore ID	*/
		enqueue(currentProcessID,semptr->waitingProcessQueue);/* Enqueue on semaphore	*/
		reschedule();			/*   and rescheduleule	*/
	}

	restore(mask);
	return OK;
}
