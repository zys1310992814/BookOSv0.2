;kernel/kernel.asm
;description: kernel func
;auther: huzicheng
;time: 2019/2/20
;copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
;E-mail: 2323168280@qq.com


[section .text]
[bits 32]
global x86_cpuid
x86_cpuid:	; void x86_cpuid(int id_eax, int id_ecx, int *eax, int *ebx, int *ecx, int *edx);
	pushad
	mov		eax,[esp+36]		; id_eax
	mov		ecx,[esp+40]		; id_ecx
	db		0x0F, 0xA2			; CPUID
	mov edi,[esp+44]
	mov [edi], eax
	mov edi,[esp+48]
	mov dword [edi], ebx
	mov edi,[esp+52]
	mov dword [edi], ecx
	mov edi,[esp+56]
	mov dword [edi], edx
	popad
	ret
global run_thread
run_thread:
	mov esp, [esp + 4]
	pop ebp
	pop ebx
	pop edi
	pop esi
	ret
	
global switch_to
switch_to:
	push esi
	push edi
	push ebx 
	push ebp
	
	mov eax, [esp + 20]
	mov [eax], esp
	
	mov eax, [esp + 24]
	mov esp, [eax]
	
	pop ebp
	pop ebx
	pop edi
	pop esi
	ret
	