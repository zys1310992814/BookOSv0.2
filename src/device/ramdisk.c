/*
File:		device/ramdisk.c
Contains:	driver for ramdisk
Auther:		Hu Zicheng
Time:		2019/1/29
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <stdio.h>
#include <string.h>
#include <sys/ramdisk.h>
#include <sys/page.h>
#include <sys/console.h>

struct ramdisk ramdisk;

void init_ramdisk()
{
	printk("=====ramdisk=====\n");
	
	ramdisk.start_addr = (uint8 *)RAMDISK_START;
	ramdisk.size = RAMDISK_SIZE;
	
	ramdisk.blocks = RAMDISK_SIZE/RAM_BLOCK_SIZE;
	
	printk("zone:%d MB blocks:%x start:%x\n", \
	ramdisk.size/MB, ramdisk.blocks, (uint32)ramdisk.start_addr);
	
	map_pages((uint32)ramdisk.start_addr,ramdisk.size/PAGE_SIZE);
	
	void *wbuf = (void *)0x10000;
	ramdisk_write(0, wbuf, 1);
	
	
	char rbuf[512];
	
	
	ramdisk_read(0, rbuf, 1);
	
	printk("%c %c %c\n",rbuf[0],rbuf[1],rbuf[3]);
	
	
	
	//while(1);
}

void ramdisk_read(uint32 lba, void *buf, uint32 counts)
{
	if(lba > ramdisk.blocks){
		return;
	}
	uint8 *addr = (uint8 *)(ramdisk.start_addr + lba*RAM_BLOCK_SIZE);
	memcpy(buf, addr, counts*RAM_BLOCK_SIZE);
}

void ramdisk_write(uint32 lba, void *buf, uint32 counts)
{
	if(lba > ramdisk.blocks){
		return;
	}
	uint8 *addr = (uint8 *)(ramdisk.start_addr + lba*RAM_BLOCK_SIZE);
	memcpy(addr, buf, counts*RAM_BLOCK_SIZE);
}

