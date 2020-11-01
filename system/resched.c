/* reschedule.c - reschedule, reschedule_cntl */

#include <xinu.h>

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  reschedule  -  rescheduleule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	reschedule(void)		/* Assumes interrupts are disabled	*/
{
	struct ProcessEntry *oldProcess;	/* Ptr to table entry for old process	*/
	struct ProcessEntry *newProcess;	/* Ptr to table entry for new process	*/

	/* If rescheduleuling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	oldProcess = &processTable[currentProcess];

	if (oldProcess->state == PR_CURR) {  /* Process remains eligible */
		if (oldProcess->priority > getQueueFirstKey(readyQueue)) {
			return;
		}

		/* Old process will no longer remain current */

		oldProcess->state = PR_READY;
		queueInsert(currentProcess, readyQueue, oldProcess->priority);
	}

	/* Force context switch to highest priority ready process */

	currentProcess = dequeue(readyQueue);
	newProcess = &processTable[currentProcess];
	newProcess->state = PR_CURR;
	msBeforePreempt = TIME_SLICE_UNIT;		/* Reset time slice for process	*/
	doContextSwitch(&oldProcess->stackPointer, &newProcess->stackPointer);

	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  reschedule_cntl  -  Control whether rescheduleuling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	reschedule_cntl(		/* Assumes interrupts are disabled	*/
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
			reschedule();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
