/*
File:		init/main.c
Contains:	kernel main entry
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/descriptor.h>
#include <sys/console.h>
#include <sys/page.h>
#include <sys/memory.h>
#include <sys/cpu.h>
#include <sys/clock.h>
#include <sys/keyboard.h>
#include <sys/harddisk.h>
#include <sys/video.h>
#include <sys/thread.h>
#include <sys/gui.h>
#include <sys/x86.h>
#include <sys/syscall.h>
#include <sys/harddisk.h>
#include <sys/ramdisk.h>
#include <sys/fatxe.h>
#include <sys/dir.h>
#include <sys/debug.h>
#include <sys/kernel.h>
#include <sys/process.h>
#include <sys/gui.h>
#include <sys/mouse.h>
#include <sys/image.h>
#include <sys/graphic.h>
#include <sys/pipe.h>
#define WRITE_DISK 1

#define WRITE_ID 5

#if WRITE_ID == 1
	#define WRITE_NAME "/test"
	#define FILE_SECTORS 20
#elif WRITE_ID == 2
	#define WRITE_NAME "/boshell"
	#define FILE_SECTORS 40
#elif WRITE_ID == 3
	#define WRITE_NAME "/tinytext"
	#define FILE_SECTORS 40
#elif WRITE_ID == 4
	#define WRITE_NAME "/brainfuck"
	#define FILE_SECTORS 20
#elif WRITE_ID == 5
	#define WRITE_NAME "/piper"
	#define FILE_SECTORS 20
#endif

#define APP_PHY_ADDR 0x60000

int main()
{

	init_page();
	init_descriptor();
	init_console();
	init_memory();
	init_video();
	
	init_cpu();
	init_syscall();
	init_thread();
	
	init_clock();
	io_sti();
	
	/*初始化设备驱动*/
	init_harddisk();
	
	//选择硬盘hda
	set_current_hd("hda");
	//在hda上初始化一个文件系统
	init_fs("harddisk",current_hd->partition.all_sectors);
	
	//最后选择操作的设备
	//set_device("harddisk");
	//init_ramdisk();
	
	//在ramdisk上面初始化一个文件系统
	//init_fs("ramdisk",ramdisk.blocks);
	
	if(WRITE_DISK){
		write_bin();
	}
	sys_ls("/",0);
	
	init_keyboard();
	init_mouse();
	
	thread_start("keyboard", 2, thread_keyboard, NULL);
	thread_start("mouse", 2, thread_mouse, NULL);
	thread_start("clock", 2, thread_clock, NULL);
	
	init_pipe();
	/*初始化图形界面后才显示图形，不然都是开机界面*/
	init_gui();
	
	process_execute(init, "init");
	
	thread_bus.main = 1;
	while(1){
		/*如果有进程被击杀，在这里回收
		通常是被鼠标或者键盘强制停止的进程
		*/
		thread_recover();
		
		/*0.1秒检测一次*/
		msec_sleep(100);
	}
	return 0;
}

void write_bin()
{
	printk("write_bin start...\n");
	
	char *app_addr = (char *)APP_PHY_ADDR;
	printk("app:%x\n", *app_addr);
	int fd;
	int written;
	
	fd = sys_open(WRITE_NAME, O_CREAT|O_RDWR);
	written = sys_write(fd, app_addr, FILE_SECTORS*SECTOR_SIZE);
	printk("write bin %s size:%d success!\n",WRITE_NAME, written);
	sys_close(fd);
}
