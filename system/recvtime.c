/* recvtime.c - recvtime */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  recvtime  -  Wait specified time to receive a message and return
 *------------------------------------------------------------------------
 */
umsg32	recvtime(
	  int32		maxwait		/* Ticks to wait before timeout */
        )
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	ProcessEntry	*prptr;		/* Tbl entry of current process	*/
	umsg32	msg;			/* Message to return		*/

	if (maxwait < 0) {
		return SYSERR;
	}
	mask = disable();

	/* Schedule wakeup and place process in timed-receive state */

	prptr = &processTable[currentProcessID];
	if (prptr->hasMessageToReceive == FALSE) {	/* Delay if no message waiting	*/
		if (queueInsertd(currentProcessID,sleepQueue,maxwait) == SYSERR) {
			restore(mask);
			return SYSERR;
		}
		prptr->state = PR_RECTIM;
		reschedule();
	}

	/* Either message arrived or timer expired */

	if (prptr->hasMessageToReceive) {
		msg = prptr->messageToReceive;	/* Retrieve message		*/
		prptr->hasMessageToReceive = FALSE;/* Reset message indicator	*/
	} else {
		msg = TIMEOUT;
	}
	restore(mask);
	return msg;
}
