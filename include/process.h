#ifndef _process_H
#define _process_H
/* process.h - isBadProcessID */

/* Maximum number of processes in the system */

#ifndef NPROC
#define	NPROC		8
#endif		

/* Process state constants */

#define	PR_FREE		0	/* Process table entry is unused	*/
#define	PR_CURR		1	/* Process is currently running		*/
#define	PR_READY	2	/* Process is on ready queue		*/
#define	PR_RECV		3	/* Process waiting for message		*/
#define	PR_SLEEP	4	/* Process is sleeping			*/
#define	PR_SUSP		5	/* Process is suspended			*/
#define	PR_WAIT		6	/* Process is on semaphore queue	*/
#define	PR_RECTIM	7	/* Process is receiving with timeout	*/

/* Miscellaneous process definitions */

#define	PNMLEN		16	/* Length of process "name"		*/
#define	NULLPROC	0	/* ID of the null process		*/

/* Process initialization constants */

#define	INITSTK		65536	/* Initial process stack size		*/
#define	INITPRIO	20	/* Initial process priority		*/
#define	INIT_TIME_SLICE	20	/* Initial process priority		*/
#define	INITRET		userret	/* Address to which process returns	*/

/* Inline code to check process ID (assumes interrupts are disabled)	*/

#define	isBadProcessID(x)	( ((pid32)(x) < 0) || \
			  ((pid32)(x) >= NPROC) || \
			  (processTable[(x)].state == PR_FREE))

/* Number of device descriptors a process can have open */

#define NDESC		5	/* must be odd to make ProcessEntry 4N bytes	*/

/* Definition of the process table (multiple of 32 bits) */

struct ProcessEntry {		/* Entry in the process table		*/
	uint16	state;	/* Process state: PR_CURR, etc.		*/
	pri16	priority;		/* Process priority			*/
	int32	timeSlice;
	int32	currentTimeSlice;
	int32   timeSliceReassignCount;
	int32   totalCpuTime;
	char	*stackPointer;	/* Saved stack pointer			*/
	char	*stackPointerBase;	/* Base of run time stack		*/
	uint32	stackSize;	/* Stack length in bytes		*/
	char	processName[PNMLEN];	/* Process name				*/
	sid32	waitingSemaphore;		/* Semaphore on which process waits	*/
	pid32	parentProcess;	/* ID of the creating process		*/
	umsg32	messageToReceive;		/* Message sent to this process		*/
	bool8	hasMessageToReceive;	/* Nonzero iff msg is valid		*/
	int16	descriptors[NDESC];	/* Device descriptors for process	*/
};

/* Marker for the top of a process stack (used to help detect overflow)	*/
#define	STACKMAGIC	0x0A0AAAA9

extern	struct	ProcessEntry processTable[];
extern	int32	processCount;	/* Currently active processes		*/
extern	pid32	currentProcessID;	/* Currently executing process		*/

#endif

