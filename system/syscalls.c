#include <xinu.h>

// Syscall list to dispatch in kernel space

const void *syscalls[] = {
	NULL,
	&create,		// 1
	&resume,		// 2
	&recvclr,		// 3
	&receive,		// 4
	&sleepms,		// 5
	&sleep,			// 6
	&fprintf,		// 7
	&printf,		// 8
	&fscanf,		// 9
	&read,			// 10
	&open,			// 11
	&control,		// 12
	&writeshargs,	// 13
	&kill,			// 14
	&getpid,		// 15
	NULL,
};

// Syscall wrapper for doing syscall in user space

uint32 do_syscall(uint32 id, uint32 args_count, ...) {
	extern void syscall_entry(uint32 id, void **args, uint32 args_count, uint32 *return_value);

	uint32 return_value;
	syscall_entry(id, 1 + (void **)&args_count, args_count, &return_value);

	return return_value;
}
