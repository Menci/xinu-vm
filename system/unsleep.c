/* unsleep.c - unsleep */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  unsleep  -  Internal function to remove a process from the sleep
 *		    queue prematurely.  The caller must adjust the delay
 *		    of successive processes.
 *------------------------------------------------------------------------
 */
status	unsleep(
	  pid32		pid		/* ID of process to remove	*/
        )
{
	intmask	mask;			/* Saved interrupt mask		*/
        struct	ProcessEntry	*prptr;		/* Ptr to process's table entry	*/

        pid32	pidnext;		/* ID of process on sleep queue	*/
					/*   that follows the process	*/
					/*   which is being removed	*/

	mask = disable();

	if (isBadProcessID(pid)) {
		restore(mask);
		return SYSERR;
	}

	/* Verify that candidate process is on the sleep queue */

	prptr = &processTable[pid];
	if ((prptr->state!=PR_SLEEP) && (prptr->state!=PR_RECTIM)) {
		restore(mask);
		return SYSERR;
	}

	/* Increment delay of next process if such a process exists */

	pidnext = queueTable[pid].next;
	if (pidnext < NPROC) {
		queueTable[pidnext].key += queueTable[pid].key;
	}

	removeFromQueue(pid);			/* Unlink process from queue */
	restore(mask);
	return OK;
}
