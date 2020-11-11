/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	ProcessEntry *prptr;		/* Ptr to process's table entry	*/
	int32	i;			/* Index into descriptors	*/

	mask = disable();
	if (isBadProcessID(pid) || (pid == NULLPROC)
	    || ((prptr = &processTable[pid])->state) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	if (--processCount <= 1) {		/* Last user process completes	*/
		xdone();
	}

	send(prptr->parentProcess, pid);
	for (i=0; i<3; i++) {
		close(prptr->descriptors[i]);
	}
	freestk(prptr->stackPointerBase, prptr->stackSize);

	switch (prptr->state) {
	case PR_CURR:
		prptr->state = PR_FREE;	/* Suicide */
		reschedule();

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->state = PR_FREE;
		break;

	case PR_WAIT:
		semtab[prptr->waitingSemaphore].count++;
		/* Fall through */

	case PR_READY:
		removeFromQueue(pid);		/* Remove from queue */
		/* Fall through */

	default:
		prptr->state = PR_FREE;
	}

	restore(mask);
	return OK;
}
