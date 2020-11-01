/* getitem.c - queuePopFirst, queuePopLast, getitem */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  queuePopFirst  -  Remove a process from the front of a queue
 *------------------------------------------------------------------------
 */
pid32	queuePopFirst(
	  qid16		q		/* ID of queue from which to	*/
	)				/* Remove a process (assumed	*/
					/*   valid with no check)	*/
{
	pid32	head;

	if (queueIsEmpty(q)) {
		return EMPTY;
	}

	head = getQueueHead(q);
	return getitem(queueTable[head].next);
}

/*------------------------------------------------------------------------
 *  queuePopLast  -  Remove a process from end of queue
 *------------------------------------------------------------------------
 */
pid32	queuePopLast(
	  qid16		q		/* ID of queue from which to	*/
	)				/* Remove a process (assumed	*/
					/*   valid with no check)	*/
{
	pid32 tail;

	if (queueIsEmpty(q)) {
		return EMPTY;
	}

	tail = getQueueTail(q);
	return getitem(queueTable[tail].prev);
}

/*------------------------------------------------------------------------
 *  getitem  -  Remove a process from an arbitrary point in a queue
 *------------------------------------------------------------------------
 */
pid32	getitem(
	  pid32		pid		/* ID of process to remove	*/
	)
{
	pid32	prev, next;

	next = queueTable[pid].next;	/* Following node in list	*/
	prev = queueTable[pid].prev;	/* Previous node in list	*/
	queueTable[prev].next = next;
	queueTable[next].prev = prev;
	return pid;
}
