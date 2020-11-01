/* recvclr.c - recvclr */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  recvclr  -  Clear incoming message, and return message if one waiting
 *------------------------------------------------------------------------
 */
umsg32	recvclr(void)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	ProcessEntry *prptr;		/* Ptr to process's table entry	*/
	umsg32	msg;			/* Message to return		*/

	mask = disable();
	prptr = &processTable[currentProcess];
	if (prptr->hasMessageToReceive == TRUE) {
		msg = prptr->messageToReceive;	/* Retrieve message		*/
		prptr->hasMessageToReceive = FALSE;/* Reset message flag		*/
	} else {
		msg = OK;
	}
	restore(mask);
	return msg;
}
