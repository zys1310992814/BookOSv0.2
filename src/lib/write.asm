[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_WRITE EQU 0

global write
write:
	mov eax, _NR_WRITE
	mov ebx, [esp + 4]		;第一个参数
	int INT_VECTOR_SYS_CALL
	ret
	