;kernel/start.asm
;description: after loader, it will be threre
;auther: huzicheng
;time: 2019/2/20
;Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
;E-mail: 2323168280@qq.com

KERNEL_STACK_TOP EQU 0x009fc00

[bits 32]
[section .text]

extern main	

global _start
_start:
	;put 'K'
	mov byte [0x000b8000+160*3+0], 'K'
	mov byte [0x000b8000+160*3+1], 0X07
	mov byte [0x000b8000+160*3+2], 'E'
	mov byte [0x000b8000+160*3+3], 0X07
	mov byte [0x000b8000+160*3+4], 'R'
	mov byte [0x000b8000+160*3+5], 0X07
	mov byte [0x000b8000+160*3+6], 'N'
	mov byte [0x000b8000+160*3+7], 0X07
	mov byte [0x000b8000+160*3+8], 'E'
	mov byte [0x000b8000+160*3+9], 0X07
	mov byte [0x000b8000+160*3+10], 'L'
	mov byte [0x000b8000+160*3+11], 0X07
	
	;init all segment registeres
	mov ax, 0x10	;the data 
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax 
	mov ss, ax 
	mov esp, KERNEL_STACK_TOP
	
	;jmp $
	call main					;into c mian

stop_run:
	hlt
	jmp stop_run
jmp $	
