#ifndef _resched_H
#define _resched_H
/* resched.h */

/* Constants and variables related to deferred rescheduleuling */

#define	DEFER_START	1	/* Start deferred rescehduling		*/
#define	DEFER_STOP	2	/* Stop  deferred rescehduling		*/

/* Structure that collects items related to deferred rescheduleuling	*/

struct	defer	{
	int32	ndefers;	/* Number of outstanding defers 	*/
	bool8	attempt;	/* Was reschedule called during the	*/
				/*   deferral period?			*/
};

extern	struct	defer	Defer;

#endif

