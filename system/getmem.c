/* getmem.c - getmem */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  getmem  -  Allocate heap storage, returning lowest word address
 *------------------------------------------------------------------------
 */
char  	*getmem(
	  uint32	nbytes		/* Size of memory requested	*/
	)
{
	kprintf("getmem: not implemented\n");
	return (char *)SYSERR;
}
