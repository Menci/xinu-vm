/* clkhandler.c - clkhandler */

#include <xinu.h>

/*------------------------------------------------------------------------
 * clkhandler - high level clock interrupt handler
 *------------------------------------------------------------------------
 */
void	clkhandler(void)
{
	/* Decrement the ms counter, and see if a second has passed */

	if((++msSinceLastSecond) >= 1000) {

		/* One second has passed, so increment seconds count */

		uptimeSeconds++;

		/* Reset the local ms counter for the next second */

		msSinceLastSecond = 0;
	}

	/* Handle sleeping processes if any exist */

	if(!queueIsEmpty(sleepQueue)) {

		/* Decrement the delay for the first process on the	*/
		/*   sleep queue, and awaken if the count reaches zero	*/

		if((--queueTable[getQueueFirst(sleepQueue)].key) <= 0) {
			wakeup();
		}
	}

	/* Decrement the msBeforePreemption counter, and rescheduleule when the */
	/*   remaining time reaches zero			     */

	if((--msBeforePreempt) <= 0) {
		msBeforePreempt = TIME_SLICE_UNIT;
		reschedule();
	}
}
