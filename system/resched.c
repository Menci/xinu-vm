/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;

#define kprintf(...)

uint32 resched_cnt;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	kprintf("scheduler: %u-th reschedule\n", ++resched_cnt);

	/* Point to process table entry for the current (old) process */

	struct procent *ptold = &proctab[currpid];

	// If this is NOT a preempt, calculate the used amount of its time slice
	// bool8 isPreempt = preempt == 0;
	int32 timeused = QUANTUM - preempt;
	ptold->prtottime += timeused;

	// The null process's time slice remains -1
	// Decrease other processes' time slice
	if (ptold->prtimeslice != -1) {
		ptold->prcurrtimeslice -= timeused;
		if (ptold->prcurrtimeslice < 0)
			ptold->prcurrtimeslice = 0;
		kprintf("scheduler: the remaining time slice of old process [%s] is %dms\n", ptold->prname, ptold->prcurrtimeslice);
	}

	if (ptold->prstate == PR_CURR) {
		// Remains runnable
		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
	}

	// Find the runnable process with max priority
	pid32 newpid = -1;
	bool8 hasready = FALSE;
	for (qid16 i = firstid(readylist); i != queuetail(readylist); i = queuetab[i].qnext) {
		pid32 pid = i;
		struct procent *prptr = &proctab[pid];

		hasready = TRUE;

		// "!= 0" includes null process -1
		if (prptr->prstate == PR_READY && prptr->prcurrtimeslice != 0) {
			if (newpid == -1 || prptr->prprio > proctab[newpid].prprio)
				newpid = pid;
		}
	}

	if (proctab[newpid].prtimeslice == -1) {
		// Only null process runnable
		kprintf("scheduler: only null process runnable\n");

		if (hasready) {
			kprintf("scheduler: >> has ready process, assigning new time slices\n");

			// If any process is ready, assign new time slices
			// Other waiting processes have old time slice
			// Find current max priority process among all
			newpid = -1;
			for (qid16 i = firstid(readylist); i != queuetail(readylist); i = queuetab[i].qnext) {
				pid32 pid = i;
				struct procent *prptr = &proctab[pid];

				if (prptr->prstate == PR_READY) {
					kprintf(
						"scheduler: >> >> assigning new time slice %dms to process [%s]\n",
						proctab[pid].prtimeslice,
						proctab[pid].prname
					);

					prptr->prcurrtimeslice = prptr->prtimeslice;
					prptr->prtimeslicecnt++;
					if (newpid == -1 || prptr->prprio > proctab[newpid].prprio)
						newpid = pid;
				} else {
					kprintf("scheduler: >> process [%s] is not ready\n", prptr->prname);
				}
			}

			kprintf("scheduler: >> now ready process with max priority is [%s]\n", proctab[newpid].prname);
		}
	}

	struct procent *ptnew = &proctab[newpid];

	kprintf("scheduler: the next process to run is [%s]\n", ptnew->prname);

	// If any process other than the null process is runnable, it will be stored in ptnew
	// Otherwise ptnew = null process

	// Dequeue the ptnew
	getitem(newpid);

	// Run the new process
	currpid = newpid;
	ptnew->prstate = PR_CURR;

	// // If switching to a new process, or preempt to a new process
	// if (ptnew != ptold || isPreempt)
		preempt = QUANTUM;

	if (ptold != ptnew) {
		extern void setkstk(void *);
		setkstk(ptnew->prkstkbase);
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr, ptnew->pageDirectoryPhysicalAddress, ptold->prstate == PR_FREE);
	}

	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
