/* queueInsert.c - queueInsert */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  queueInsert  -  Insert a process into a queue in descending key order
 *------------------------------------------------------------------------
 */
status	queueInsert(
	  pid32		pid,		/* ID of process to queueInsert	*/
	  qid16		q,		/* ID of queue to use		*/
	  int32		key		/* Key for the queueInserted process	*/
	)
{
	qid16	curr;			/* Runs through items in a queue*/
	qid16	prev;			/* Holds previous node index	*/

	if (isBadQueueID(q) || isBadProcessID(pid)) {
		return SYSERR;
	}

	curr = getQueueFirst(q);
	while (queueTable[curr].key >= key) {
		curr = queueTable[curr].next;
	}

	/* Insert process between curr node and previous node */

	prev = queueTable[curr].prev;	/* Get index of previous node	*/
	queueTable[pid].next = curr;
	queueTable[pid].prev = prev;
	queueTable[pid].key = key;
	queueTable[prev].next = pid;
	queueTable[curr].prev = pid;
	return OK;
}
