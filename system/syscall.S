.intel_syntax noprefix

# eax: syscall id
# ebx: arguments array
# ecx: arguments count
# edx: pointer to write return value
# edi: return eip
# esi: return esp
.globl syscall_dispatch
syscall_dispatch:
	push edi # save return eip
	push esi # save return esp
	push edx # save pointer to return value

	# load syscall_function = syscalls[id]
	mov edx, offset syscalls
	mov eax, dword ptr [edx + eax * 4]

	# push arguments
	imul edx, ecx, -1
	mov ebp, esp # backup esp to ebp
	lea esp, dword ptr [esp + edx * 4]
	mov edi, esp
	mov esi, ebx
	cld
	rep movsd

	# call target syscall function
	call eax
	mov esp, ebp

	# Write return value
	pop ebx # load pointer to return value
	mov dword ptr [ebx], eax

	# Clear kernel space state
	mov ebx, 0
	mov edi, 0
	mov esi, 0
	mov ebp, 0

	pop ecx # load return esp
	pop edx # load return eip

	# edx -> eip
	# ecx -> esp
	sysexit

# syscall_entry(
#   syscall id (=> eax),
#   arguments array (=> ebx),
#   arguments count (=> ecx),
#   pointer to write return value (=> edx),
# )
.globl syscall_entry
syscall_entry:
	push ebp
	mov ebp, esp

	# save user space state
	pusha

	# set syscall arguments
	mov eax, dword ptr [ebp + 8]
	mov ebx, dword ptr [ebp + 12]
	mov ecx, dword ptr [ebp + 16]
	mov edx, dword ptr [ebp + 20]
	mov edi, offset syscall_return
	mov esi, esp

	# do syscall
	sysenter

syscall_return:
	# restore user space state
	popa

	# ebp cleared by kernel
	pop ebp
	ret
