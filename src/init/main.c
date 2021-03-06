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
#include <sys/rgui.h>
#include <sys/timer.h>

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
void test();
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

	/*keyboard线程是对键盘数据的基本处理，编程可以识别的数据，然后自己可以在这基础上再次拓展*/
	thread_start("keyboard", 1, thread_keyboard, NULL);	//RGUI接管键盘数据处理。
	thread_start("mouse", 1, thread_mouse, NULL);         
	/*把clock改成timer，用定时器的方式来进行时钟更新*/
	thread_start("timer", 2, thread_timer, NULL);

	/*初始化管道系统，用于进程间通讯*/
	init_pipe();
	/*初始化图形界面后才显示图形，不然都是开机界面*/
	init_gui();

	/*init_gui_system();
	test();
	sys_clean_screen();*/
	process_execute(init, "init");
	thread_bus.main = 1;
	int temp = 0;
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
void event(int key,int *datas){
	if(key == MOUSE_DOWN){
		if(datas[0] == 1){	//左键按下
			fouce_write("got click");
		}
	}
}
void test(){	//本函数用来测试RGUI相关功能

	int window = sys_new_window();
	resize_window(window,100,100);
	move_window(window,100,100);
	bind_window(window,event);
	rgui_draw_rect(get_window_sheet(window),0,0,100,100,COLOR_GREEN);
}