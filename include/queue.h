#ifndef _queue_H
#define _queue_H
/* queue.h - getQueueFirst, getQueueFirstKey, queueIsEmpty, getQueueLastKey, queueIsNotEmpty		*/

/* Queue structure declarations, constants, and inline functions	*/

/* Default # of queue entries: 1 per process plus 2 for ready list plus	*/
/*			2 for sleep list plus 2 per semaphore		*/
#ifndef NQENT
#define NQENT	(NPROC + 4 + NSEM + NSEM)
#endif

#define	EMPTY	(-1)		/* Null value for next or prev index	*/
#define	MAXKEY	0x7FFFFFFF	/* Max key that can be stored in queue	*/
#define	MINKEY	0x80000000	/* Min key that can be stored in queue	*/

struct	QueueEntry	{		/* One per process plus two per list	*/
	int32	key;		/* Key on which the queue is ordered	*/
	qid16	next;		/* Index of next process or tail	*/
	qid16	prev;		/* Index of previous process or head	*/
};

extern	struct QueueEntry	queueTable[];

/* Inline queue manipulation functions */

#define	getQueueHead(q)	(q)
#define	getQueueTail(q)	((q) + 1)
#define	getQueueFirst(q)	(queueTable[getQueueHead(q)].next)
#define	getQueueLast(q)	(queueTable[getQueueTail(q)].prev)
#define	queueIsEmpty(q)	(getQueueFirst(q) >= NPROC)
#define	queueIsNotEmpty(q)	(getQueueFirst(q) <  NPROC)
#define	getQueueFirstKey(q)	(queueTable[getQueueFirst(q)].key)
#define	getQueueLastKey(q)	(queueTable[ getQueueLast(q)].key)

/* Inline to check queue id assumes interrupts are disabled */

#define	isBadQueueID(x)	(((int32)(x) < NPROC) || (int32)(x) >= NQENT-1)

#endif

