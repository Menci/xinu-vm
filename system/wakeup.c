/* wakeup.c - wakeup */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  wakeup  -  Called by clock interrupt handler to awaken processes
 *------------------------------------------------------------------------
 */
void	wakeup(void)
{
	/* Awaken all processes that have no more time to sleep */

	reschedule_cntl(DEFER_START);
	while (queueIsNotEmpty(sleepQueue) && (getQueueFirstKey(sleepQueue) <= 0)) {
		ready(dequeue(sleepQueue));
	}

	reschedule_cntl(DEFER_STOP);
	return;
}
