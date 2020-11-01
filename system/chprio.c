/* chprio.c - chprio */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  chprio  -  Change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
pri16	chprio(
	  pid32		pid,		/* ID of process to change	*/
	  pri16		newprio		/* New priority			*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	ProcessEntry *prptr;		/* Ptr to process's table entry	*/
	pri16	oldprio;		/* Priority to return		*/

	mask = disable();
	if (isBadProcessID(pid)) {
		restore(mask);
		return (pri16) SYSERR;
	}
	prptr = &processTable[pid];
	oldprio = prptr->priority;
	prptr->priority = newprio;
	restore(mask);
	return oldprio;
}
