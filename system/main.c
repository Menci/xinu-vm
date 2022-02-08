#include <xinu.h>
// #include <memory.h>

process main(void)
{

	/* Run the Xinu shell */

	syscall_recvclr();
	syscall_resume(syscall_create(shell, 50, 50, "shell", 1, CONSOLE));

	/* Wait for shell to exit and recreate it */

	while (TRUE) {
		syscall_receive();
		syscall_sleepms(200);
		syscall_printf("\n\nMain process recreating shell\n\n");
		syscall_resume(syscall_create(shell, 20, 50, "shell", 1, CONSOLE));
	}
	return OK;
    
}
