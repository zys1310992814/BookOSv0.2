######
#Makefile for kernel
#File: kernel/Makefile
#Contains: kernel makefile
#Auther: Hu Zicheng
#Time: 2019/1/24
#Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
#E-mail: 2323168280@qq.com
######

#The tools name
NASM 		= nasm
CC			= gcc

#The file path
INCLUDE_PATH = ../include/

#flags
ASM_FLAGS	= -I $(INCLUDE_PATH) -f elf
C_FLAGS	= -I $(INCLUDE_PATH) -c -fno-builtin -Wall -Wunused

OBJS =  _start.o\
		x86.o\
		descriptor.o\
		8259a.o\
		interrupt.o\
		console.o\
		bitmap.o\
		page.o\
		kernel.o\
		memory.o\
		ards.o\
		cpu.o\
		ioqueue.o\
		debug.o\
		cmos.o\
		syscall.o\
		thread.o\
		sync.o\
		tss.o\
		process.o\
		system.o
		
		
		
		
		
		
		
		
#First read here
.PHONY: all

all: compile

#Compile files
compile: $(OBJS)

#Clean temporary files
clean:
	-rm *.o
	-rm kernel.elf

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
	