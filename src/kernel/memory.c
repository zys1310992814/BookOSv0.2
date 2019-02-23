/*
File:		kernel/memory.c
Contains:	memory management
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <string.h>
#include <math.h>
#include <sys/memory.h>
#include <sys/ards.h>
#include <sys/console.h>
#include <sys/page.h>
#include <sys/bitmap.h>
#include <sys/thread.h>
#include <sys/debug.h>

uint32_t memory_total_size;
int memory_used_size;
extern struct bitmap phy_mem_bitmap;
extern struct bitmap vir_mem_bitmap;

//内存管理在内存中，页中分配，不在内核中占空间
struct memory_manage *memory_manage;

//static void memory_test();
void init_memory()
{
	memory_total_size = 0;
	//开始获取
	init_ards();
	
	if(memory_total_size > 0XFFFFFFF0){
		memory_total_size = 0XFFFFFFF0;
	}
	/*总共-固定占用 = 空闲大小*/
	int page_bytes = (memory_total_size-PHY_MEM_BASE_ADDR)/(PAGE_SIZE*8);
	
	phy_mem_bitmap.bits = (uint8_t*)PHY_MEM_BITMAP;
	phy_mem_bitmap.btmp_bytes_len = page_bytes;
	//b1
	
	vir_mem_bitmap.bits = (uint8_t*)VIR_MEM_BITMAP;
	vir_mem_bitmap.btmp_bytes_len = PHY_MEM_BITMAP_SIZE-PHY_MEM_BASE_ADDR/(PAGE_SIZE*8);
	//b2
	
	//memset(vir_mem_bitmap.bits, 0, phy_mem_bitmap.btmp_bytes_len);
	
	bitmap_init(&phy_mem_bitmap);
	//while(1);
	bitmap_init(&vir_mem_bitmap);
	//b3
	init_memory_manage( );
	
	
	
	
	//clean low 2G
	//memset((void *)(PAGE_DIR_VIR_ADDR+4),0,2044);
	//b4
	//
	
	printk(">Init memory.\n");
	
	//memory_test();
	

}

void sys_get_memory(int *size, int *free)
{
	*size = memory_total_size;
	*free = get_free_memory();
}


/*
static void memory_test()
{
	
	char *addr = kernel_alloc_page(1);
	printk("addr:%x\n", addr);
	
	
	
	uint32_t *a;
	a = kmalloc(32);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(32);
	printk("%x\n", a);
	a = kmalloc(1024);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(1024);
	printk("%x\n", a);
	a = kmalloc(4096);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(4096);
	printk("%x\n", a);
	a = kmalloc(1024*1024);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(1024*1024);
	printk("%x\n", a);
	a = kmalloc(10*1024*1024);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(10*1024*1024);
	printk("%x\n", a);
}
*/
void init_memory_manage(void )
{
	int i;
	uint32_t memory_manage_pages = DIV_ROUND_UP(sizeof(struct memory_manage),PAGE_SIZE);
	/*为结构体分配空间*/
	memory_manage = (struct memory_manage *)kernel_alloc_page(memory_manage_pages);
	if(memory_manage == NULL){
		panic("memory manage failed!");
	}
	memset(memory_manage, 0, memory_manage_pages*PAGE_SIZE);
	for(i = 0; i < MEMORY_BLOCKS; i++){	
		memory_manage->free_blocks[i].size = 0;	//大小是页的数量
		memory_manage->free_blocks[i].flags = MEMORY_BLOCK_FREE;
	}
}

void *kmalloc(uint32_t size)
{
	int i;
	uint32_t address;
	uint32_t break_size;//要打碎成什么大小
	uint32_t break_cnt;//要打碎成几块
	void *new_address;
	
	//大于1024字节就用页
	if(size >= 2048){
		int pages = DIV_ROUND_UP(size, PAGE_SIZE);	//一共占多少个页
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到
				address = (uint32_t )kernel_alloc_page(pages);	//分配页
				memory_manage->free_blocks[i].address = address;	
				memory_manage->free_blocks[i].size = pages;	//大小是页的数量
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_USING;
				memory_manage->free_blocks[i].mode = MEMORY_BLOCK_MODE_BIG;
				//printk("Found pages ");
				//printk("Alloc:%x idx:%d\n", address,i);
				return (void *)address;
			}
		}
	}else if(0 < size &&size <= 2048){	//size <= 2048
		//对齐判断，要打散成多大
		if(0 < size && size <= 32){
			break_size = 32;
		}else if(32 < size && size <= 64){
			break_size = 64;
		}else if(64 < size && size <= 128){
			break_size = 128;
		}else if(128 < size && size <= 256){
			break_size = 256;
		}else if(256 < size && size <= 512){
			break_size = 512;
		}else if(512 < size && size <= 1024){
			break_size = 1024;
		}else if(1024 < size && size <= 2048){
			break_size = 2048;
		}
		//第一次寻找，如果在块中没有找到，就打散一个页
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].size == break_size && memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到
				address = memory_manage->free_blocks[i].address;
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found broken ");
				//printk("Alloc:%x idx:%d\n", address,i);
				return (void *)address;
			}
		}
		//如果都没有找到，分配一个页，然后打散
		//分配一个页，用来被打散
		new_address = kernel_alloc_page(1);
		break_cnt = PAGE_SIZE/break_size;
		
		//打散成break_cnt个
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到一个可以被使用的
				//地址增加
				
				//设置最终地址
				memory_manage->free_blocks[i].address = (uint32_t)new_address;
				new_address += break_size;
				//设置size
				memory_manage->free_blocks[i].size = break_size;
				//设置为可以分配
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_FREE;
				//设置为小块模式
				memory_manage->free_blocks[i].mode = MEMORY_BLOCK_MODE_SMALL;
				break_cnt--;
				if(break_cnt <= 0){
					break;
				}
			}
		}
		//打散后的寻找
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].size == break_size && memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到
				address = memory_manage->free_blocks[i].address;
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found new broken ");
				//printk("Alloc:%x idx:%d\n", address,i);
				return (void *)address;
			}
		}
	}
	//size=0或者没有找到
	return NULL;	//失败
}

int kfree(void *address)
{
	int i;
	uint32_t addr = (uint32_t )address;
	for(i = 0; i < MEMORY_BLOCKS; i++){
		if(memory_manage->free_blocks[i].address == addr && memory_manage->free_blocks[i].flags == MEMORY_BLOCK_USING){	//找到
			if(memory_manage->free_blocks[i].mode == MEMORY_BLOCK_MODE_BIG){
				kernel_free_page(memory_manage->free_blocks[i].address, memory_manage->free_blocks[i].size);
				memory_manage->free_blocks[i].size = 0;		//只有大块才需要重新设置size
			}else if(memory_manage->free_blocks[i].mode == MEMORY_BLOCK_MODE_SMALL){
				//小块内存就清空就是了
				memset((void *)memory_manage->free_blocks[i].address, 0, memory_manage->free_blocks[i].size);
				//存在一种情况，那就是所有被打散的内存都被释放后，可能需要释放那个页，目前还没有考虑它
				//小块不需要设置大小，因为就是打散了的块
			}
			memory_manage->free_blocks[i].flags = MEMORY_BLOCK_FREE;
			
			//printk("Free:%x idx:%d\n", address,i);
			return 0;
		}
	}
	
	return -1;	//失败
}

void *sys_memory_alloc(uint32_t size)
{
	int i;
	uint32_t address;
	uint32_t break_size;//要打碎成什么大小
	uint32_t break_cnt;//要打碎成几块
	void *new_address;
	struct thread *thread = thread_current();
	struct memory_manage *thread_mm = thread->mm;

	//大于1024字节就用页
	if(size >= 2048){
		int pages = DIV_ROUND_UP(size, PAGE_SIZE);	//一共占多少个页
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(thread_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到
				address = (uint32_t )thread_alloc_page(thread, pages);	//分配页
				thread_mm->free_blocks[i].address = address;	
				thread_mm->free_blocks[i].size = pages;	//大小是页的数量
				thread_mm->free_blocks[i].flags = MEMORY_BLOCK_USING;
				thread_mm->free_blocks[i].mode = MEMORY_BLOCK_MODE_BIG;
				//printk("Found pages ");
				//printk("1 Alloc:%x idx:%d size:%d size:%d \n", address,i, thread_mm->free_blocks[i].size, size);
				return (void *)address;
			}
		}
	}else if(0 < size &&size <= 2048){	//size <= 2048
		
		//对齐判断，要打散成多大
		if(0 < size && size <= 32){
			break_size = 32;
		}else if(32 < size && size <= 64){
			break_size = 64;
		}else if(64 < size && size <= 128){
			break_size = 128;
		}else if(128 < size && size <= 256){
			break_size = 256;
		}else if(256 < size && size <= 512){
			break_size = 512;
		}else if(512 < size && size <= 1024){
			break_size = 1024;
		}else if(1024 < size && size <= 2048){
			break_size = 2048;
		}
		
		//第一次寻找，如果在块中没有找到，就打散一个页
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(thread_mm->free_blocks[i].size == break_size && thread_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到
				address = thread_mm->free_blocks[i].address;
				thread_mm->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found broken ");
				//printk("2 Alloc:%x idx:%d size:%d size:%d \n", address,i, thread_mm->free_blocks[i].size, size);
				return (void *)address;
			}
		}
		//如果都没有找到，分配一个页，然后打散
		//分配一个页，用来被打散
		new_address = thread_alloc_page(thread, 1);
		break_cnt = PAGE_SIZE/break_size;
		//printk("*addr %x broken into %d parts\n", new_address, break_cnt);
		//打散成break_cnt个
		for(i = 0; i < MEMORY_BLOCKS; i++){
			
			if(thread_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到一个可以被使用的
				//地址增加
				//设置最终地址
				thread_mm->free_blocks[i].address = (uint32_t)new_address;
				new_address += break_size;
				//设置size
				thread_mm->free_blocks[i].size = break_size;
				//设置为可以分配
				thread_mm->free_blocks[i].flags = MEMORY_BLOCK_FREE;
				//设置为小块模式
				thread_mm->free_blocks[i].mode = MEMORY_BLOCK_MODE_SMALL;
				break_cnt--;
				if(break_cnt <= 0){
					break;
				}
			}
		}
		//打散后的寻找
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(thread_mm->free_blocks[i].size == break_size && thread_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到
				address = thread_mm->free_blocks[i].address;
				thread_mm->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found new broken ");
				//printk("3 Alloc:%x idx:%d size:%d size:%d \n", address,i, thread_mm->free_blocks[i].size, size);
				return (void *)address;
			}
		}
	}
	//size=0或者没有找到
	return NULL;	//失败
}

int sys_memory_free(void *address)
{
	int i;
	uint32_t addr = (uint32_t )address;
	struct thread *thread = thread_current();
	struct memory_manage *thread_mm = thread->mm;

	for(i = 0; i < MEMORY_BLOCKS; i++){
		if(thread_mm->free_blocks[i].address == addr && thread_mm->free_blocks[i].flags == MEMORY_BLOCK_USING){	//找到
			if(thread_mm->free_blocks[i].mode == MEMORY_BLOCK_MODE_BIG){
				thread_free_page(thread, thread_mm->free_blocks[i].address, thread_mm->free_blocks[i].size);
				thread_mm->free_blocks[i].size = 0;		//只有大块才需要重新设置size
			}else if(thread_mm->free_blocks[i].mode == MEMORY_BLOCK_MODE_SMALL){
				//小块内存就清空就是了
				memset((void *)thread_mm->free_blocks[i].address, 0, thread_mm->free_blocks[i].size);
				//存在一种情况，那就是所有被打散的内存都被释放后，可能需要释放那个页，目前还没有考虑它
				//小块不需要设置大小，因为就是打散了的块
			}
			thread_mm->free_blocks[i].flags = MEMORY_BLOCK_FREE;
			
			//printk("Free:%x idx:%d\n", address,i);
			return 0;
		}
	}
	//printk("not found that address %x\n", address);
	return -1;	//失败
}

