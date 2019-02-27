[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_GET_PID equ 36

global get_pid
get_pid:
	mov eax, _NR_GET_PID
	int INT_VECTOR_SYS_CALL
	ret
