/* send.c - send */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  send  -  Pass a message to a process and start recipient if waiting
 *------------------------------------------------------------------------
 */
syscall	send(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32	msg		/* Contents of message		*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	ProcessEntry *prptr;		/* Ptr to process's table entry	*/

	mask = disable();
	if (isBadProcessID(pid)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &processTable[pid];
	if (prptr->hasMessageToReceive) {
		restore(mask);
		return SYSERR;
	}
	prptr->messageToReceive = msg;		/* Deliver message		*/
	prptr->hasMessageToReceive = TRUE;		/* Indicate message is waiting	*/

	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->state == PR_RECV) {
		ready(pid);
	} else if (prptr->state == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
	restore(mask);		/* Restore interrupts */
	return OK;
}
