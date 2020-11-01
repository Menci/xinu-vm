/* receive.c - receive */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  receive  -  Wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */
umsg32	receive(void)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	ProcessEntry *prptr;		/* Ptr to process's table entry	*/
	umsg32	msg;			/* Message to return		*/

	mask = disable();
	prptr = &processTable[currentProcess];
	if (prptr->hasMessageToReceive == FALSE) {
		prptr->state = PR_RECV;
		reschedule();		/* Block until message arrives	*/
	}
	msg = prptr->messageToReceive;		/* Retrieve message		*/
	prptr->hasMessageToReceive = FALSE;	/* Reset message flag		*/
	restore(mask);
	return msg;
}
