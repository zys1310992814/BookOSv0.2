/*
File:		kernel/tss.c
Contains:	init tss
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/


#include <string.h>
#include <sys/tss.h>
#include <sys/descriptor.h>
#include <sys/x86.h>
#include <sys/console.h>

struct tss tss;

void init_tss()
{
	memset(&tss, 0, sizeof(tss));
	tss.ss0 = KERNEL_DATA_SEL;
	tss.iobase = sizeof(tss);
	load_tr(KERNEL_TSS_SEL);
	printk(">init tss.\n");
}

/*
	把任务的页目录表放在tss中，以后便于写入
*/
void update_tss_esp(struct thread *thread)
{
	tss.esp0 = (uint32 *)thread->kernel_stack;
	
}