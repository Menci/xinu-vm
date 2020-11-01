/* getprio.c - getprio */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  getprio  -  Return the scheduling priority of a process
 *------------------------------------------------------------------------
 */
syscall	getprio(
	  pid32		pid		/* Process ID			*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	uint32	prio;			/* Priority to return		*/

	mask = disable();
	if (isBadProcessID(pid)) {
		restore(mask);
		return SYSERR;
	}
	prio = processTable[pid].priority;
	restore(mask);
	return prio;
}
