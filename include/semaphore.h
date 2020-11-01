#ifndef _semaphore_H
#define _semaphore_H
/* semaphore.h - isBadSemaphoreID */

#ifndef	NSEM
#define	NSEM		120	/* Number of semaphores, if not defined	*/
#endif

/* Semaphore state definitions */

#define	S_FREE	0		/* Semaphore table entry is available	*/
#define	S_USED	1		/* Semaphore table entry is in use	*/

/* Semaphore table entry */
struct	SemaphoreEntry	{
	byte	state;		/* Whether entry is S_FREE or S_USED	*/
	int32	count;		/* Count for the semaphore		*/
	qid16	waitingProcessQueue;		/* Queue of processes that are waiting	*/
				/*     on the semaphore			*/
};

extern	struct	SemaphoreEntry semtab[];

#define	isBadSemaphoreID(s)	((int32)(s) < 0 || (s) >= NSEM)

#endif

