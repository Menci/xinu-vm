/* reschedule.c - reschedule, reschedule_cntl */

#include <xinu.h>

struct	defer	Defer;

#define kprintf(...)

uint32 rescheduleCount;

/*------------------------------------------------------------------------
 *  reschedule  -  rescheduleule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	reschedule(void)		/* Assumes interrupts are disabled	*/
{
	/* If rescheduleuling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	kprintf("scheduler: %u-th reschedule\n", ++rescheduleCount);

	/* Point to process table entry for the current (old) process */

	struct ProcessEntry *oldProcess = &processTable[currentProcessID];

	// If this is NOT a preempt, calculate the used amount of its time slice
	// bool8 isPreempt = msBeforePreempt == 0;
	int32 usedCpuTime = TIME_SLICE_UNIT - msBeforePreempt;
	oldProcess->totalCpuTime += usedCpuTime;

	// The null process's time slice remains -1
	// Decrease other processes' time slice
	if (oldProcess->timeSlice != -1) {
		oldProcess->currentTimeSlice -= usedCpuTime;
		if (oldProcess->currentTimeSlice < 0)
			oldProcess->currentTimeSlice = 0;
		kprintf("scheduler: the remaining time slice of old process [%s] is %dms\n", oldProcess->processName, oldProcess->currentTimeSlice);
	}

	if (oldProcess->state == PR_CURR) {
		// Remains runnable
		oldProcess->state = PR_READY;
		queueInsert(currentProcessID, readyQueue, oldProcess->priority);
	}

	// Find the runnable process with max priority
	pid32 newProcessID = -1;
	bool8 hasReadyProcess = FALSE;
	for (qid16 i = getQueueFirst(readyQueue); i != getQueueTail(readyQueue); i = queueTable[i].next) {
		pid32 processID = i;
		struct ProcessEntry *process = &processTable[processID];

		hasReadyProcess = TRUE;

		// "!= 0" includes null process -1
		if (process->state == PR_READY && process->currentTimeSlice != 0) {
			if (newProcessID == -1 || process->priority > processTable[newProcessID].priority)
				newProcessID = processID;
		}
	}

	if (processTable[newProcessID].timeSlice == -1) {
		// Only null process runnable
		kprintf("scheduler: only null process runnable\n");
		
		if (hasReadyProcess) {
			kprintf("scheduler: >> has ready process, assigning new time slices\n");

			// If any process is ready, assign new time slices
			// Other waiting processes have old time slice
			// Find current max priority process among all
			newProcessID = -1;
			for (qid16 i = getQueueFirst(readyQueue); i != getQueueTail(readyQueue); i = queueTable[i].next) {
				pid32 processID = i;
				struct ProcessEntry *process = &processTable[processID];

				if (process->state == PR_READY) {
					kprintf(
						"scheduler: >> >> assigning new time slice %dms to process [%s]\n",
						processTable[processID].timeSlice,
						processTable[processID].processName
					);

					process->currentTimeSlice = process->timeSlice;
					process->timeSliceReassignCount++;
					if (newProcessID == -1 || process->priority > processTable[newProcessID].priority)
						newProcessID = processID;
				} else {
					kprintf("scheduler: >> process [%s] is not ready\n", process->processName);
				}
			}

			kprintf("scheduler: >> now ready process with max priority is [%s]\n", processTable[newProcessID].processName);
		}
	}

	struct ProcessEntry *newProcess = &processTable[newProcessID];

	kprintf("scheduler: the next process to run is [%s]\n", newProcess->processName);

	// If any process other than the null process is runnable, it will be stored in newProcess
	// Otherwise newProcess = null process

	// Dequeue the newProcess
	removeFromQueue(newProcessID);

	// Run the new process
	currentProcessID = newProcessID;
	newProcess->state = PR_CURR;

	// // If switching to a new process, or preempt to a new process
	// if (newProcess != oldProcess || isPreempt)
		msBeforePreempt = TIME_SLICE_UNIT;

	if (oldProcess != newProcess)
		doContextSwitch(&oldProcess->stackCurrent, &newProcess->stackCurrent, newProcess->pageDirectoryPhysicalAddress, oldProcess->state == PR_FREE);

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
