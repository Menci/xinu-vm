/* queueInsertd.c - queueInsertd */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  queueInsertd  -  Insert a process in delta list using delay as the key
 *------------------------------------------------------------------------
 */
status	queueInsertd(			/* Assumes interrupts disabled	*/
	  pid32		pid,		/* ID of process to queueInsert	*/
	  qid16		q,		/* ID of queue to use		*/
	  int32		key		/* Delay from "now" (in ms.)	*/
	)
{
	int32	next;			/* Runs through the delta list	*/
	int32	prev;			/* Follows next through the list*/

	if (isBadQueueID(q) || isBadProcessID(pid)) {
		return SYSERR;
	}

	prev = getQueueHead(q);
	next = queueTable[getQueueHead(q)].next;
	while ((next != getQueueTail(q)) && (queueTable[next].key <= key)) {
		key -= queueTable[next].key;
		prev = next;
		next = queueTable[next].next;
	}

	/* Insert new node between prev and next nodes */

	queueTable[pid].next = next;
	queueTable[pid].prev = prev;
	queueTable[pid].key = key;
	queueTable[prev].next = pid;
	queueTable[next].prev = pid;
	if (next != getQueueTail(q)) {
		queueTable[next].key -= key;
	}

	return OK;
}
