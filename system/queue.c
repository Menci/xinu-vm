/* queue.c - enqueue, dequeue */

#include <xinu.h>

struct QueueEntry	queueTable[NQENT];	/* Table of process queues	*/

/*------------------------------------------------------------------------
 *  enqueue  -  Insert a process at the tail of a queue
 *------------------------------------------------------------------------
 */
pid32	enqueue(
	  pid32		pid,		/* ID of process to queueInsert	*/
	  qid16		q		/* ID of queue to use		*/
	)
{
	qid16	tail, prev;		/* Tail & previous node indexes	*/

	if (isBadQueueID(q) || isBadProcessID(pid)) {
		return SYSERR;
	}

	tail = getQueueTail(q);
	prev = queueTable[tail].prev;

	queueTable[pid].next  = tail;	/* Insert just before tail node	*/
	queueTable[pid].prev  = prev;
	queueTable[prev].next = pid;
	queueTable[tail].prev = pid;
	return pid;
}

/*------------------------------------------------------------------------
 *  dequeue  -  Remove and return the first process on a list
 *------------------------------------------------------------------------
 */
pid32	dequeue(
	  qid16		q		/* ID of queue to use		*/
	)
{
	pid32	pid;			/* ID of process removed	*/

	if (isBadQueueID(q)) {
		return SYSERR;
	} else if (queueIsEmpty(q)) {
		return EMPTY;
	}

	pid = queuePopFirst(q);
	queueTable[pid].prev = EMPTY;
	queueTable[pid].next = EMPTY;
	return pid;
}
