[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_NEW_PIPE EQU 49
_NR_WRITE_PIPE EQU 50
_NR_READ_PIPE EQU 51
_NR_CLOSE_PIPE EQU 52

global new_pipe
new_pipe:
	mov eax, _NR_NEW_PIPE
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
global write_pipe
write_pipe:
	mov eax, _NR_WRITE_PIPE
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	mov edx, [esp + 12]
	int INT_VECTOR_SYS_CALL
	ret
global read_pipe
read_pipe:
	mov eax, _NR_READ_PIPE
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret
global close_pipe
close_pipe:
	mov eax, _NR_CLOSE_PIPE
	mov ecx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
