.intel_syntax noprefix

# noreturn ring3(user space args...)
# edi: user space eip
.globl ring3
ring3:
	# set ds, es, fs, gs
	mov ax, (4 * 8) | 3
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	# sysexit sets cs & ss

	# set target eip & esp
	mov edx, edi # edx -> eip
	mov ecx, esp # esp -> esp

	# clear registers
	mov eax, 0
	mov ebx, 0
	mov edi, 0
	mov esi, 0
	mov ebp, 0

	sysexit
