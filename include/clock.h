#ifndef _clock_H
#define _clock_H
/* clock.h */

/* Intel 8254-2 clock chip constants */

#define	CLOCKBASE	0x40		/* I/O base port of clock chip	*/
#define	CLOCK0		CLOCKBASE
#define	CLKCNTL		(CLOCKBASE+3)	/* chip CSW I/O port		*/


#define CLKTICKS_PER_SEC  1000	/* clock timer resolution		*/

extern	uint32	uptimeSeconds;	/* current time in secs since boot	*/
extern  uint32	msSinceLastSecond;        /* ms since last clock tick             */

extern	qid16	sleepQueue;		/* queue for sleeping processes		*/
extern	uint32	msBeforePreempt;	/* msBeforePreemption counter			*/

#endif

