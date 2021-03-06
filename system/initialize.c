/* initialize.c - nulluser, sysinit */

/* Handle system initialization and become the null process */

#include <xinu.h>
#include <string.h>

extern	void	start(void);	/* Start of Xinu code			*/
extern	void	*_end;		/* End of Xinu code			*/

/* Function prototypes */

extern	void main(void);	/* Main is the first process created	*/
static	void sysinit(); 	/* Internal system initialization	*/
extern	void meminit(void);	/* Initializes the free memory list	*/
local	process startup(void);	/* Process to finish startup tasks	*/

/* Declarations of major kernel variables */

struct	ProcessEntry	processTable[NPROC];	/* Process table			*/
struct	SemaphoreEntry	semtab[NSEM];	/* Semaphore table			*/

/* Active system status */

int	processCount;		/* Total number of live processes	*/
pid32	currentProcessID;		/* ID of currently executing process	*/

/* Control sequence to reset the console colors and cusor positiion	*/

#define	CONSOLE_RESET	" \033[0m\033[2J\033[;H"

/*------------------------------------------------------------------------
 * nulluser - initialize the system and become the null process
 *
 * Note: execution begins here after the C run-time environment has been
 * established.  Interrupts are initially DISABLED, and must eventually
 * be enabled explicitly.  The code turns itself into the null process
 * after initialization.  Because it must always remain ready to execute,
 * the null process cannot execute code that might cause it to be
 * suspended, wait for a semaphore, put to sleep, or exit.  In
 * particular, the code must not perform I/O except for polled versions
 * such as kprintf.
 *------------------------------------------------------------------------
 */

void	nulluser()
{	
	/* Initialize the system */

	sysinit();

	/* Output Xinu memory layout */

	kprintf("%10d bytes of Xinu code.\n",
		(uint32)&etext - (uint32)&text);
	kprintf("           [0x%08X to 0x%08X]\n",
		(uint32)&text, (uint32)&etext - 1);
	kprintf("%10d bytes of data.\n",
		(uint32)&ebss - (uint32)&data);
	kprintf("           [0x%08X to 0x%08X]\n\n",
		(uint32)&data, (uint32)&ebss - 1);

	/* Enable interrupts */

	enable();

	/* Create a process to finish startup and start main */

	resume(create((void *)startup, INITPRIO, INIT_TIME_SLICE,
					"Startup process", 0, NULL));

	/* Become the Null process (i.e., guarantee that the CPU has	*/
	/*  something to run when no other process is ready to execute)	*/

	while (TRUE) {

		/* Halt until there is an external interrupt */

		asm volatile ("hlt");
	}

}


/*------------------------------------------------------------------------
 *
 * startup  -  Finish startup takss that cannot be run from the Null
 *		  process and then create and resumethe main process
 *
 *------------------------------------------------------------------------
 */
local process	startup(void)
{
	/* Create a process to execute function main() */

	resume(create((void *)main, INITPRIO, INIT_TIME_SLICE,
					"Main process", 0, NULL));

	/* Startup process exits at this point */

	return OK;
}


/*------------------------------------------------------------------------
 *
 * sysinit  -  Initialize all Xinu data structures and devices
 *
 *------------------------------------------------------------------------
 */
static	void	sysinit()
{
	int32	i;
	struct	ProcessEntry	*process;		/* Ptr to process table entry	*/
	struct	SemaphoreEntry	*semptr;	/* Ptr to semaphore table entry	*/

	/* Platform Specific Initialization */

	platinit();

	/* Reset the console */

	kprintf(CONSOLE_RESET);
	kprintf("\n%s\n\n", VERSION);

	/* Initialize the interrupt vectors */

	initevec();
	
	/* Initialize virtual memory */
	
	meminit();
	PageDirectory initializationPageDirectoryPhysicalAddress = initializeVirtualMemory();

	/* Initialize system variables */

	/* Count the Null process as the first process in the system */

	processCount = 1;

	/* Scheduling is not currently blocked */

	Defer.ndefers = 0;

	/* Initialize process table entries free */

	for (i = 0; i < NPROC; i++) {
		process = &processTable[i];
		process->state = PR_FREE;
		process->processName[0] = NULLCH;
		process->stackEnd = NULL;
		process->priority = 0;
	}

	/* Initialize the Null process entry */	

	process = &processTable[NULLPROC];
	process->state = PR_CURR;
	process->priority = 0;
	process->timeSlice = process->currentTimeSlice = -1;
	process->timeSliceReassignCount = 0;
	process->totalCpuTime = 0;
	process->pageDirectoryPhysicalAddress = initializationPageDirectoryPhysicalAddress;
	strncpy(process->processName, "prnull", 7);
	process->stackEnd = VM_STACK_VIRTUAL_ADDRESS_HIGH;
	process->stackSize = VM_STACK_SIZE_PER_PROCESS;
	allocateVirtualMemoryPages(initializationPageDirectoryPhysicalAddress, VM_STACK_VIRTUAL_PAGE_ID_BEGIN, VM_STACK_PAGES_PER_PROCESS);
	process->stackCurrent = 0;
	currentProcessID = NULLPROC;
	
	/* Initialize semaphores */

	for (i = 0; i < NSEM; i++) {
		semptr = &semtab[i];
		semptr->state = S_FREE;
		semptr->count = 0;
		semptr->waitingProcessQueue = newqueue();
	}

	/* Initialize buffer pools */

	bufinit();

	/* Create a ready list for processes */

	readyQueue = newqueue();

	/* Initialize the real time clock */

	clkinit();

	for (i = 0; i < NDEVS; i++) {
		init(i);
	}
	return;
}

int32	stop(char *s)
{
	kprintf("%s\n", s);
	kprintf("looping... press reset\n");
	while(1)
		/* Empty */;
}

int32	delay(int n)
{
	DELAY(n);
	return OK;
}
