/* create.c - create, newpid */

#include <xinu.h>

local	int newpid();

/*------------------------------------------------------------------------
 *  create  -  Create a process to start running a function on x86
 *------------------------------------------------------------------------
 */
pid32	create(
	  void		*funcaddr,	/* Address of the function	*/
	  pri16		priority,	/* Process priority > 0		*/
	  int32		prtimeslice,	/* Process prtimeslice > 0		*/
	  char		*name,		/* Name (for debugging)		*/
	  uint32	nargs,		/* Number of args that follow	*/
	  ...
	)
{
	uint32		savsp;
	intmask 	mask;    	/* Interrupt mask		*/
	pid32		pid;		/* Stores new process id	*/
	struct	procent	*prptr;		/* Pointer to proc. table entry */
	int32		i;
	uint32		*a;		/* Points to list of args	*/

	mask = disable();
	if ((priority < 1) || ((pid = newpid()) == SYSERR)) {
		restore(mask);
		return SYSERR;
	}

	prcount++;
	prptr = &proctab[pid];

	/* Initialize process table entry for new process */
	prptr->prstate = PR_SUSP;	/* Initial state is suspended	*/
	prptr->prprio = priority;
	prptr->prtimeslice = prtimeslice;
	prptr->prcurrtimeslice = prtimeslice;
	prptr->prtimeslicecnt = 0;
	prptr->prtottime = 0;
	prptr->pageDirectoryPhysicalAddress = allocateVirtualMemorySpace();
	prptr->heapSize = 0;
	prptr->prstkbase = VM_STACK_VIRTUAL_ADDRESS_HIGH;
	prptr->prstklen = VM_STACK_SIZE_PER_PROCESS;
	prptr->prname[PNMLEN-1] = NULLCH;
	for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=name[i])!=NULLCH; i++)
		;
	prptr->prsem = -1;
	prptr->prparent = (pid32)getpid();
	prptr->prhasmsg = FALSE;

	/* Set up stdin, stdout, and stderr descriptors for the shell	*/
	prptr->prdesc[0] = CONSOLE;
	prptr->prdesc[1] = CONSOLE;
	prptr->prdesc[2] = CONSOLE;

	/* Initialize stack as if the process was called		*/

	static uint32 initialContentOfStackLastPage[VM_PAGE_SIZE / sizeof(uint32)];
	uint32 *stackBufferAddress = initialContentOfStackLastPage + VM_PAGE_SIZE / sizeof(uint32);
	uint32 *stackVirtualAddress = VM_STACK_VIRTUAL_ADDRESS_HIGH;
	stackBufferAddress--;
	stackVirtualAddress--;

	*stackBufferAddress = STACKMAGIC;
	savsp = (uint32)stackVirtualAddress;

	/* Push arguments */
	a = (uint32 *)(&nargs + 1);	/* Start of args		*/
	a += nargs -1;			/* Last argument		*/
	for ( ; nargs > 0 ; nargs--) {	/* Machine dependent; copy args	*/
		--stackVirtualAddress;
		*--stackBufferAddress = *a--;	/* onto created process's stack	*/
	}
	--stackVirtualAddress;
	*--stackBufferAddress = (long)INITRET;	/* Push on return address	*/

	/* The following entries on the stack must match what ctxsw	*/
	/*   expects a saved process state to contain: ret address,	*/
	/*   ebp, interrupt mask, flags, registers, and an old SP	*/

	--stackVirtualAddress;
	*--stackBufferAddress = (long)funcaddr;	/* Make the stack look like it's*/
					/*   half-way through a call to	*/
					/*   ctxsw that "returns" to the*/
					/*   new process		*/
	--stackVirtualAddress;
	*--stackBufferAddress = savsp;		/* This will be register ebp	*/
					/*   for process exit		*/
	savsp = (uint32) stackVirtualAddress;		/* Start of frame for ctxsw	*/
	--stackVirtualAddress;
	*--stackBufferAddress = 0x00000200;		/* New process runs with	*/
					/*   interrupts enabled		*/

	/* Basically, the following emulates an x86 "pushal" instruction*/

	--stackVirtualAddress;
	*--stackBufferAddress = 0;			/* %eax */
	--stackVirtualAddress;
	*--stackBufferAddress = 0;			/* %ecx */
	--stackVirtualAddress;
	*--stackBufferAddress = 0;			/* %edx */
	--stackVirtualAddress;
	*--stackBufferAddress = 0;			/* %ebx */
	--stackVirtualAddress;
	*--stackBufferAddress = 0;			/* %esp; value filled in below	*/
	uint32 *pushSpBufferAddress = stackBufferAddress;			/* Remember this location	*/
	--stackVirtualAddress;
	*--stackBufferAddress = savsp;		/* %ebp (while finishing ctxsw)	*/
	--stackVirtualAddress;
	*--stackBufferAddress = 0;			/* %esi */
	--stackVirtualAddress;
	*--stackBufferAddress = 0;			/* %edi */
	*stackBufferAddress = (unsigned long) (prptr->prstkptr = (char *)stackVirtualAddress);

	// Initialize the stack
	allocateVirtualMemoryPages(prptr->pageDirectoryPhysicalAddress, VM_STACK_VIRTUAL_PAGE_ID_BEGIN, VM_STACK_PAGES_PER_PROCESS);
	writeToAnotherVirtualMemorySpacePage(prptr->pageDirectoryPhysicalAddress, VM_STACK_VIRTUAL_PAGE_ID_END - 1, initialContentOfStackLastPage);

	restore(mask);
	return pid;
}

/*------------------------------------------------------------------------
 *  newpid  -  Obtain a new (free) process ID
 *------------------------------------------------------------------------
 */
local	pid32	newpid(void)
{
	uint32	i;			/* Iterate through all processes*/
	static	pid32 nextpid = 1;	/* Position in table to try or	*/
					/*   one beyond end of table	*/

	/* Check all NPROC slots */

	for (i = 0; i < NPROC; i++) {
		nextpid %= NPROC;	/* Wrap around to beginning */
		if (proctab[nextpid].prstate == PR_FREE) {
			return nextpid++;
		} else {
			nextpid++;
		}
	}kprintf("newpid error\n");
	return (pid32) SYSERR;
}
