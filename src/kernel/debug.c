/*
File:		kernel/debug.c
Contains:	debug for kernel
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/debug.h>
#include <sys/console.h>
#include <sys/vga.h>
#include <sys/gui.h>
#include <stdarg.h>
#include <stdio.h>

//停机并输出大量信息
void panic(const char *fmt, ...)
{
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	vsprintf(buf, fmt, arg);
	console_set_color(MAKE_COLOR(BLACK,RED));
	printk("\n>panic: %s", buf);
	
	while(1);
	/* should never arrive here */
	__asm__ __volatile__("ud2");
}
//断言
void assertion_failure(char *exp, char *file, char *base_file, int line)
{
	console_set_color(MAKE_COLOR(BLACK,RED));
	printk("\nassert(%s) failed:\nfile: %s\nbase_file: %s\nln%d",
	exp, file, base_file, line);

	spin(">assertion_failure()");

	/* should never arrive here */
        __asm__ __volatile__("ud2");
}
//停机显示函数名
void spin(char * func_name)
{
	printk("\nspinning in %s", func_name);
	while(1);
}
