#include <xinu.h>
// #include <memory.h>

process main(void)
{

	/* Run the Xinu shell */

	recvclr();
	resume(create(shell, 50, 50, "shell", 1, CONSOLE));

	/* Wait for shell to exit and recreate it */

	while (TRUE) {
		receive();
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		resume(create(shell, 20, 50, "shell", 1, CONSOLE));
	}
	return OK;
    
}
