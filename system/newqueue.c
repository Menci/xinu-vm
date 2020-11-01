/* newqueue.c - newqueue */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  newqueue  -  Allocate and initialize a queue in the global queue table
 *------------------------------------------------------------------------
 */
qid16	newqueue(void)
{
	static qid16	nextqid=NPROC;	/* Next list in queueTable to use	*/
	qid16		q;		/* ID of allocated queue 	*/

	q = nextqid;
	if (q >= NQENT) {		/* Check for table overflow	*/
		return SYSERR;
	}

	nextqid += 2;			/* Increment index for next call*/

	/* Initialize head and tail nodes to form an empty queue */

	queueTable[getQueueHead(q)].next = getQueueTail(q);
	queueTable[getQueueHead(q)].prev = EMPTY;
	queueTable[getQueueHead(q)].key  = MAXKEY;
	queueTable[getQueueTail(q)].next = EMPTY;
	queueTable[getQueueTail(q)].prev = getQueueHead(q);
	queueTable[getQueueTail(q)].key  = MINKEY;
	return q;
}
