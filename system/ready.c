/* ready.c - ready */

#include <xinu.h>

qid16	readyQueue;			/* Index of ready list		*/

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	register struct ProcessEntry *prptr;

	if (isBadProcessID(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */

	prptr = &processTable[pid];
	prptr->state = PR_READY;
	queueInsert(pid, readyQueue, prptr->priority);
	reschedule();

	return OK;
}
