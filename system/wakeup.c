/* wakeup.c - wakeup */

#include <xinu.h>

// #undef kprintf

/*------------------------------------------------------------------------
 *  wakeup  -  Called by clock interrupt handler to awaken processes
 *------------------------------------------------------------------------
 */
void	wakeup(void)
{
	/* Awaken all processes that have no more time to sleep */

	reschedule_cntl(DEFER_START);
	while (queueIsNotEmpty(sleepQueue) && (getQueueFirstKey(sleepQueue) <= 0)) {
		pid32 pid = dequeue(sleepQueue);
		kprintf("wakeup: process [%s] waken up\n", processTable[pid].processName);
		ready(pid);
	}

	reschedule_cntl(DEFER_STOP);
	return;
}
