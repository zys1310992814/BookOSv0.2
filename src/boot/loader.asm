;boot/loader.asm
;description: loade kernel and set basic info
;auther: Hu Zicheng
;time: 2018/1/23
;copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
;E-mail: 2323168280@qq.com

%include "const.inc"

org 0x90000
[bits 16]
align 16
	jmp LoaderStart
;Global Descriptor Table,GDT
GDT:
	;0:void
	dd		0x00000000
	dd		0x00000000
	;1:4GB(flat-mode) code segment 0
	dd		0x0000ffff
	dd		0x00cf9A00
	;2:4GB(flat-mode) data segment 0
	dd		0x0000ffff
	dd		0x00cf9200
	
GdtLen equ $ - GDT	
Gdt48:
	dw	(GdtLen-1)
	dd	GDT
	
LoaderStart:
	mov ax, cs
	mov ds, ax 
	mov ss, ax
	mov sp, 0
	mov ax, 0xb800
	mov es, ax
	
	;show 'LOADER'
	mov byte [es:160+0],'L'
	mov byte [es:160+1],0x07
	mov byte [es:160+2],'O'
	mov byte [es:160+3],0x07
	mov byte [es:160+4],'A'
	mov byte [es:160+5],0x07
	mov byte [es:160+6],'D'
	mov byte [es:160+7],0x07
	mov byte [es:160+8],'E'
	mov byte [es:160+9],0x07
	mov byte [es:160+10],'R'
	mov byte [es:160+11],0x07
	
LoadeKernel:

	;loade kernel
	;first block 128 sectors
	mov ax, KERNEL_SEG
	mov si, KERNEL_OFF
	mov cx, BLOCK_SIZE
	call LoadeBlock
	
	;second block 128 sectors
	add ax, 0x1000
	mov cx, BLOCK_SIZE
	call LoadeBlock
	
	;third block 128 sectors
	add ax, 0x1000
	mov cx, BLOCK_SIZE
	call LoadeBlock
LoadeApp:
	;loade app
	mov ax, APP_SEG
	mov si, APP_OFF
	mov cx, BLOCK_SIZE
	call LoadeBlock
	
	add ax, 0x1000
	mov cx, BLOCK_SIZE
	call LoadeBlock

	
	call KillMotor
	
;检测内存
CheckMemory:
	xor ebx, ebx 
	mov edx, 0x534d4150
	mov di, 0
	mov ax, ARDS_SEG 
	mov es, ax
	mov word [es:ARDS_ZONE_SIZE-4], 0
.E820MemGetLoop:
	mov eax, 0x0000e820
	mov ecx, 20
	int 0x15
	jc .E820CheckFailed
	add di, cx 
	inc word [es:ARDS_ZONE_SIZE-4]
	cmp ebx, 0
	jnz .E820MemGetLoop
	jmp VideoGraphciSet
.E820CheckFailed:
	jmp $

VideoGraphciSet:
	;检查VBE是否存在
	;缓冲区 0x90000开始
	mov	ax,VBEINFO_SEG	
	mov	es,ax
	mov	di,0
	mov	ax,0x4f00	;检查VBE存在功能，指定ax=0x4f00
	int	0x10
	cmp	ax,0x004f	;ax=0x004f 存在
	jne	.VideoError
	
	;检查VBE版本，必须是VBE 2.0及其以上
	mov	ax,[es:di+4]
	cmp	ax,0x0200
	jb	.VideoError			; if (ax < 0x0200) goto screen_default

	;获取画面信息， 256字节
	;cx=输入检查的模式
	;[es:di+0x00]	模式属性	bit7是1就能加上0x4000，便于操作
	;[es:di+0x12]	x的分辨率	宽度
	;[es:di+0x14]	y的分辨率	高度
	;[es:di+0x19]	颜色数		8位，16位，24位，32位
	;[es:di+0x1b]	颜色的指定方法 	调色板等
	;[es:di+0x28]	VRAM 地址
	
	mov ax, VBEMODE_SEG
	mov es, ax
	
	mov	cx,VBEMODE	;cx=模式号
	mov	ax,0x4f01	;获取画面模式功能，指定ax=0x4f01
	int	0x10
	cmp	ax,0x004f	;ax=0x004f 指定的这种模式可以使用
	jne	.VideoError

	;切换到指定的模式
	mov	BX,VBEMODE+0x4000	;bx=模式号
	mov	ax,0x4f02	;切换模式模式功能，指定ax=0x4f01
	int	0x10
	
	mov ax, VIDEO_INFO_SEG
	mov ds, ax
	
	xor ax, ax
	mov	al,[es:di+0x19]
	mov	[BITS_PER_PIXE],ax ;保存颜色位数
	mov	ax,[es:di+0x12]
	mov	[VIDEO_WIDTH],ax		;保存x分辨率 宽度
	mov	ax,[es:di+0x14]
	mov	[VIDEO_HEIGHT],ax		;保存y分辨率 高度
	mov	eax,[es:di+0x28]
	mov	[VIDEO_RAM],eax		;保存VRAM地址
	
	;reset ds
	mov ax, cs
	mov ds, ax
	
	JMP	SetProtectMode

	;切换失败切换到默认的模式
.VideoError:
	jmp $

SetProtectMode:
	;close the interruption
	cli
	;load GDTR
	lgdt	[Gdt48]
	
	;enable A20 line
	in		al,0x92
	or		al,2
	out		0x92,al
	;set CR0 bit PE
	mov		eax,cr0
	or		eax,1
	mov		cr0,eax
	
	;far jump:to clean the cs
	jmp	dword 0x08:Flush
	
;si=LBA address, from 0
;cx=sectors
;es:dx=buffer address	
;this function was borrowed from internet
ReadSectors:
	push ax 
	push cx 
	push dx 
	push bx 
	
	mov ax, si 
	xor dx, dx 
	mov bx, 18
	
	div bx 
	inc dx 
	mov cl, dl 
	xor dx, dx 
	mov bx, 2
	
	div bx 
	
	mov dh, dl
	xor dl, dl 
	mov ch, al 
	pop bx 
.RP:
	mov al, 0x01
	mov ah, 0x02 
	int 0x13 
	jc .RP 
	pop dx
	pop cx 
	pop ax
	ret

;ax = 写入的段偏移
;si = 扇区LBA地址
;cx = 扇区数
LoadeBlock:
	mov es, ax
	xor bx, bx 
.loop:
	call ReadSectors
	add bx, 512
	inc si 
	loop .loop
	ret	

;don't use floppy from now on
KillMotor:
	push dx
	mov	dx, 03F2h
	mov	al, 0
	out	dx, al
	pop	dx
	ret
	
[bits 32]
align 32
Flush:
	;init all segment registeres
	mov ax, 0x10	;the data 
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax 
	mov ss, ax 
	mov esp, LOADER_STACK_TOP
	
	;put 'P'
	mov byte [0xb8000+160*2+0], 'P'
	mov byte [0xb8000+160*2+1], 0X07
	mov byte [0xb8000+160*2+2], 'R'
	mov byte [0xb8000+160*2+3], 0X07
	mov byte [0xb8000+160*2+4], 'O'
	mov byte [0xb8000+160*2+5], 0X07
	mov byte [0xb8000+160*2+6], 'T'
	mov byte [0xb8000+160*2+7], 0X07
	mov byte [0xb8000+160*2+8], 'E'
	mov byte [0xb8000+160*2+9], 0X07
	mov byte [0xb8000+160*2+10], 'C'
	mov byte [0xb8000+160*2+11], 0X07
	mov byte [0xb8000+160*2+12], 'T'
	mov byte [0xb8000+160*2+13], 0X07
	
	call ReadKernel
	
	jmp 0X08:KERNEL_START_ADDR
	
	push eax
	jmp $

; 遍历每一个 Program Header，根据 Program Header 中的信息来确定把什么放进内存，放到什么位置，以及放多少。
ReadKernel:
	xor	esi, esi
	mov	cx, word [KERNEL_PHY_ADDR + 2Ch]; ┓ ecx <- pELFHdr->e_phnum
	movzx	ecx, cx					;
	mov	esi, [KERNEL_PHY_ADDR + 1Ch]	; esi <- pELFHdr->e_phoff
	add	esi, KERNEL_PHY_ADDR		; esi <- OffsetOfKernel + pELFHdr->e_phoff
.begin:
	mov	eax, [esi + 0]
	cmp	eax, 0				; PT_NULL
	jz	.unaction
	push	dword [esi + 010h]		; size	┓
	mov	eax, [esi + 04h]		;	┃
	add	eax, KERNEL_PHY_ADDR	;	┣ ::memcpy(	(void*)(pPHdr->p_vaddr),
	push	eax				; src	┃		uchCode + pPHdr->p_offset,
	push	dword [esi + 08h]		; dst	┃		pPHdr->p_filesz;
	call	memcpy				;	┃
	add	esp, 12				;	┛
.unaction:
	add	esi, 020h			; esi += pELFHdr->e_phentsize
	dec	ecx
	jnz	.begin
	ret
	
memcpy:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	esi, [ebp + 12]	; Source
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	al, [ds:esi]		; ┓
	inc	esi			; ┃
					; ┣ 逐字节移动
	mov	byte [es:edi], al	; ┃
	inc	edi			; ┛

	dec	ecx		; 计数器减一
	jmp	.1		; 循环
.2:
	mov	eax, [ebp + 8]	; 返回值

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回	
	
;fill it with 1kb
times (4096-($-$$)) db 0
