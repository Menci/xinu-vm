/* suspend.c - suspend */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  suspend  -  Suspend a process, placing it in hibernation
 *------------------------------------------------------------------------
 */
syscall	suspend(
	  pid32		pid		/* ID of process to suspend	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	ProcessEntry *prptr;		/* Ptr to process's table entry	*/
	pri16	prio;			/* Priority to return		*/

	mask = disable();
	if (isBadProcessID(pid) || (pid == NULLPROC)) {
		restore(mask);
		return SYSERR;
	}

	/* Only suspend a process that is current or ready */

	prptr = &processTable[pid];
	if ((prptr->state != PR_CURR) && (prptr->state != PR_READY)) {
		restore(mask);
		return SYSERR;
	}
	if (prptr->state == PR_READY) {
		removeFromQueue(pid);		    /* Remove a ready process	*/
					    /*   from the ready list	*/
		prptr->state = PR_SUSP;
	} else {
		prptr->state = PR_SUSP;   /* Mark the current process	*/
		reschedule();		    /*   suspended and reschedule.	*/
	}
	prio = prptr->priority;
	restore(mask);
	return prio;
}
