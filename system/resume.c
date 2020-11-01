/* resume.c - resume */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  resume  -  Unsuspend a process, making it ready
 *------------------------------------------------------------------------
 */
pri16	resume(
	  pid32		pid		/* ID of process to unsuspend	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	ProcessEntry *prptr;		/* Ptr to process's table entry	*/
	pri16	prio;			/* Priority to return		*/

	mask = disable();
	if (isBadProcessID(pid)) {
		restore(mask);
		return (pri16)SYSERR;
	}
	prptr = &processTable[pid];
	if (prptr->state != PR_SUSP) {
		restore(mask);
		return (pri16)SYSERR;
	}
	prio = prptr->priority;		/* Record priority to return	*/
	ready(pid);
	restore(mask);
	return prio;
}
