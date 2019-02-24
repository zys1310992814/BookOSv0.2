/*
File:		kernel/thread.c
Contains:	kernel thread 
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/thread.h>
#include <sys/console.h>
#include <sys/memory.h>
#include <sys/page.h>
#include <sys/x86.h>
#include <sys/debug.h>
#include <sys/keyboard.h>
#include <sys/tss.h>
#include <sys/descriptor.h>
#include <sys/process.h>
#include <sys/kernel.h>
#include <sys/clock.h>
#include <sys/graphic.h>

extern int thread_opened;

struct thread thread_table[MAX_THREADS];
struct thread *thread_ready[MAX_THREADS];

uint32 thread_this, thread_runnings, thread_cur_level;

struct thread *main_thread;
struct thread *user_init_thread;
struct thread *idle_thread;

struct thread_bus thread_bus;

static void make_main_thread();
static void kernel_thread(thread_func *function, void *arg);
static void idle();

extern void switch_to(struct thread *cur, struct thread *next);

void thread_a(void *arg);
void thread_b(void *arg);
void process_a(void);
void process_b(void);
void thread_c(void *arg);
int test_va, test_vb;

void init_thread()
{
	printk(">init thread\n");
	int i;
	//no 0 pid 
	for(i = 0; i < MAX_THREADS; i++){
		thread_table[i].status = 0;
		thread_table[i].pid = i+1;
	}
	thread_this = 0;
	thread_runnings = 0;
	thread_cur_level = 0;
	init_tss();
	
	test_va = test_vb = 0;
	
	thread_bus.main = 0;
	thread_bus.idle = 0;
	thread_bus.keyboard = 0;
	thread_bus.mouse = 0;
	thread_bus.clock = 0;
	thread_bus.gui = 0;

	//主线程
	make_main_thread();
	//执行init进程
	//idle进程
	idle_thread = thread_start("idle", 1, idle, NULL);
	
	/*
	初始化设备线程
	*/
	
	thread_opened = 1;
	
	printk("run in main thread\n");
	
}


void init()
{
	//printf("running in init\n");

	execv("/boshell",NULL);
	int status;
	int pid;

	while(1){
		/*等待用户退出的进程*/
		pid = _wait(&status);
		if(pid != -1){
			//printk("init: wait:%d status:%d\n",pid, status);
		}
		
	}
}

void thread_test()
{
	
	thread_start("thread b", 5, thread_b, "arg B");
	/*
	process_execute(process_a, "a");
	process_execute(process_b, "b");
	*/
}


void thread_a(void *arg)
{
	printk("run in thread a\n");
	char *para = arg;
	
	printk(para);
	int i = 0;
	while(1){
		//io_sti();
		i++;
		if(i%0x100000 == 0){
			printk("A ");
		}
	}
}

void thread_b(void *arg)
{
	printk("run in thread b\n");
	char *para = arg;
	
	printk(para);
	int i = 0;
	while(1){
		//io_sti();
		i++;
		if(i%0x100000 == 0){
			printk("B ");
		}
	}
}
void process_a(void)
{
	printf("run in process a\n");
	
	char *a = malloc(32);
	*a = 0xfa;
	
	printf("malloc a:%x data:%x\n", a, *a);
	
	free(a);
	int i = 0;
	while(1){
		i++;
		test_va++;
		if(i%0x100000 == 0){
			printf("%d ", test_va);
		}
	}
}

void process_b(void)
{
	printf("run in process b\n");
	char *b = malloc(40960);
	*b = 0x5a;
	printf("malloc b:%x data:%x\n", b, *b);
	free(b);
	int i = 0;
	while(1){
		i++;
		test_vb++;
		if(i%0x100000 == 0){
			printf("%d ", test_vb);
		}
	}
}

struct thread *alloc_thread()
{
	int i;
	for(i = 0; i < MAX_THREADS; i++){
		if(thread_table[i].status == 0){
			return &thread_table[i];
		}
	}
	return NULL;
}


void free_thread(struct thread *thread)
{
	int i;
	struct thread *th;

	for(i = 0; i < MAX_THREADS; i++){
		th = &thread_table[i];
		if(th == thread){
			th->status = 0;
		}
	}
}


struct thread *thread_current()
{
	return thread_ready[thread_this];
}

void thread_add(struct thread *thread)
{
	thread_ready[thread_runnings] = thread;
	thread_runnings++;
	
}

struct thread *next_thread()
{
	struct thread *thread;
	thread_this++;
	if(thread_this == thread_runnings){
		thread_this = 0;
	}
	thread = thread_ready[thread_this];
	return thread;
}

struct thread *thread_pick()
{
	struct thread *thread;
	while(1){
		thread = next_thread();
		
		if(thread->status == THREAD_READY){
			break;
		}
	}
	
	return thread;
}

bool thread_ready_empty()
{
	int i;
	struct thread *thread;

	for(i = 0; i < thread_runnings; i++){
		thread = thread_ready[i];
		if(thread->status == THREAD_READY){
			return false;
		}
	}
	return true;
}

bool thread_find(struct thread *thread)
{
	int i;
	struct thread *th;

	for(i = 0; i < thread_runnings; i++){
		th = thread_ready[i];
		if(th == thread){
			return true;
		}
	}
	return false;
}


void thread_remove(struct thread *thread)
{
	int i;
	//搜索
	for(i = 0; i < thread_runnings; i++){
		if(thread_ready[i] == thread){
			break;
		}
	}
	//调整
	thread_runnings--;
	if(i < thread_this){
		thread_this--;
	}
	//改变位置，覆盖thread
	for(; i < thread_runnings; i++){
		thread_ready[i] = thread_ready[i+1]; 
	}
}

void thread_recover()
{
	int i;
	struct thread *thread;
	
	for(i = 0; i < thread_runnings; i++){
		thread = thread_ready[i];
		if(thread->status == THREAD_HANGING){
			//printk("found a died process:%s\n", thread->name);
			/*回收资源*/
			thread_kill(thread);
			
			/*退出运行*/
			thread_exit(thread);
			
			break;
		}
	}
}

void print_threads()
{
	int i;
	struct thread *thread;
	printk("=====Thread info=====\n");
	for(i = 0; i < thread_runnings; i++){
		thread = thread_ready[i];
		printk("name:%s  pid:%d  status:%d  ticks:%d\n", thread->name, thread->pid, thread->status, thread->run_ticks);
		
	}
}

void thread_chain_init(struct thread_chain *thread_chain)
{
	
	int i;
	for(i = 0; i < MAX_THREADS; i++){
		thread_chain->thread[i] = NULL;
	}
	thread_chain->in = 0;
	thread_chain->out = 0;
}

void thread_chain_put(struct thread_chain *thread_chain, struct thread *thread)
{
	
	thread_chain->thread[thread_chain->in] = thread;
	thread_chain->in++;
	if(thread_chain->in == MAX_THREADS){
		thread_chain->in = 0;
	}
	
}

struct thread *thread_chain_get(struct thread_chain *thread_chain)
{
	struct thread *thread;
	//PICK一个处于ready的任务
	
	thread = thread_chain->thread[thread_chain->out];
	thread_chain->out++;
	if(thread_chain->out == MAX_THREADS){
		thread_chain->out = 0;
	}
	return thread;
}

bool thread_chain_empty(struct thread_chain *thread_chain)
{
	if(thread_chain->in == thread_chain->out){
		return true;
	}
	return false;
}

static void kernel_thread(thread_func *function, void *arg)
{
	io_sti();
	function(arg);
}

void thread_create(struct thread *thread, thread_func function, void *arg)
{
	thread->self_stack -= sizeof(struct intr_stack);
	
	thread->self_stack -= sizeof(struct thread_stack);
	struct thread_stack *thread_stack = (struct thread_stack *)thread->self_stack;
	thread_stack->eip = kernel_thread;
	thread_stack->function = function;
	thread_stack->arg = arg;
	thread_stack->ebp = thread_stack->ebx =\
	thread_stack->esi = thread_stack->edi = 0;
}

void thread_init(struct thread *thread, char *name, int prio)
{
	int pid = thread->pid;
	memset(thread, 0, sizeof(struct thread));
	thread->pid = pid;
	strcpy(thread->name, name);
	if(thread == main_thread){
		thread->status = THREAD_RUNNING;
	}else{
		thread->status = THREAD_READY;
	}
	
	thread->kernel_stack = thread->self_stack = (uint8 *)(kmalloc(PAGE_SIZE)+PAGE_SIZE);
	thread->priority = prio;
	
	/*改写level*/
	if(prio > thread_cur_level){
		thread_cur_level = prio;
	}
	thread->ticks = prio;
	thread->run_ticks = 0;
	thread->pdir = NULL;
	
	thread->parent_pid = 0;
	thread->bag = NULL;
	
	//默认都是根目录
	strcpy(thread->cwd, "/");
	
	thread->vidbuf = NULL;
	
	thread->stack_magic = 0x19980325;
}

struct thread *thread_start(char *name, int prio, thread_func function, void *arg)
{
	struct thread *thread = alloc_thread();
	thread_init(thread, name, prio);
	thread_create(thread, function, arg);
	
	thread_add(thread);
	
	return thread;
} 

static void make_main_thread()
{
	main_thread = alloc_thread();
	thread_init(main_thread, "main", 2);

	thread_add(main_thread);
}

void thread_block(int status)
{
	assert((status == THREAD_BLOCKED) ||\
	(status == THREAD_WAITING) ||\
	(status == THREAD_HANGING));
	
	int old_status = io_load_eflags();
	io_cli();
	struct thread *cur = thread_current();
	cur->status = status;
	schedule();
	io_store_eflags(old_status);
}

void thread_unblock(struct thread *thread)
{
	int old_status = io_load_eflags();
	io_cli();
	
	assert((thread->status == THREAD_BLOCKED) ||\
	(thread->status == THREAD_WAITING) ||\
	(thread->status == THREAD_HANGING));
	
	if(thread->status != THREAD_READY){
		
		thread->status = THREAD_READY;
	}
	io_store_eflags(old_status);
}

static void idle()
{
	thread_bus.idle = 1;
	while(1){
		thread_block(THREAD_BLOCKED);
		io_sti();
		io_hlt();
	}
}

void schedule()
{
	struct thread *cur = thread_current();
	
	if(cur->status == THREAD_RUNNING){
		cur->ticks = cur->priority;
		cur->status = THREAD_READY;
	}else{
		
	}
	
	if(thread_ready_empty()){
		thread_unblock(idle_thread);
	}
	struct thread *next;
	
	next = thread_pick();
	
	next->status = THREAD_RUNNING;
	if(next != cur){
		process_activate(next);
	}
		//printk("%s -> %s\n", cur->name, next->name);
	switch_to(cur, next);
	//}
	
}

void thread_yield()
{
	struct thread *cur = thread_current();
	int old_status = io_load_eflags();
	io_cli();
	cur->status = THREAD_READY;
	schedule();
	io_store_eflags(old_status);
}


void thread_init_bitmap(struct thread *thread)
{
	thread->vir_mem_bitmap.bits = (uint8_t*)kmalloc(VIR_MEM_BITMAP_SIZE);
	thread->vir_mem_bitmap.btmp_bytes_len = VIR_MEM_BITMAP_SIZE;
	bitmap_init(&thread->vir_mem_bitmap);
}

void init_thread_memory_manage(struct thread *thread)
{
	int i;
	/*为结构体分配空间*/
	thread->mm = (struct memory_manage *)kmalloc(sizeof(struct memory_manage));
	
	if(thread->mm == NULL){
		panic("thread memory manage failed!");
	}
	
	memset(thread->mm, 0, sizeof(struct memory_manage));
	for(i = 0; i < MEMORY_BLOCKS; i++){	
		thread->mm->free_blocks[i].size = 0;	//大小是页的数量
		thread->mm->free_blocks[i].flags = MEMORY_BLOCK_FREE;
	}
}

void thread_exit(struct thread *thread)
{
	
	int old_status = io_load_eflags();
	io_cli();
	thread->status = THREAD_DIED;
	
	//从队列中移除
	if(thread_find(thread)){
		thread_remove(thread);
	}
	
	if(thread->pdir != NULL){
		kfree(thread->pdir);
	}
	
	if(thread != main_thread){
		//printk("release %s\n", thread->name);
		//释放内核栈
		kfree(thread->kernel_stack - PAGE_SIZE);
		
		//释放结构体
		free_thread(thread);
		
	}
	
	io_store_eflags(old_status);
}

struct thread *pid2thread(int pid)
{
	int i;
	struct thread *thread;
	for(i = 0; i < thread_runnings; i++){
		thread = thread_ready[i];
		if(thread->pid == pid){
			return thread;
		}
	}
	return NULL;
}

static void release_thread_resource(struct thread *thread)
{
	//释放程序自身占用的储存空间，代码，数据，栈
	uint32 *pdir_vaddr = thread->pdir;
	uint16 user_pde_nr = 1023, pde_idx = 512;
	uint32 pde = 0;
	uint32 *var_pde_ptr = NULL;
	
	uint16 user_pte_nr = 1024, pte_idx = 0;
	uint32 pte = 0;
	uint32 *var_pte_ptr = NULL;
	
	uint32 *first_pte_vaddr_in_pde = NULL;
	
	uint32 pg_phy_addr = 0;
	
	while(pde_idx < user_pde_nr){
		//获取页目录项地址
		var_pde_ptr = pdir_vaddr + pde_idx;
		//获取内容
		pde = *var_pde_ptr;
		if(pde & 0x01){
			//页目录P为1
			first_pte_vaddr_in_pde = pte_ptr(pde_idx*0x400000);
			//获取页表地址
			pte_idx = 0;
			while(pte_idx < user_pte_nr){
				//获取页目表项地址
				var_pte_ptr = first_pte_vaddr_in_pde + pte_idx;
				//获取页表内容
				pte = *var_pte_ptr;
				if(pte & 0x01){
					//页表P为1
					//获取物理页的地址（去掉属性就是地址）
					pg_phy_addr = pte & 0xfffff000;
					free_mem_page(pg_phy_addr);
					//printk("phy:%x\n", pg_phy_addr);
				}
				pte_idx++;
			}
			pg_phy_addr = pde & 0xfffff000;
			free_mem_page(pg_phy_addr);
		}
		pde_idx++;
	}

	//释放位图和内存管理
	//printk("release mm start ");
	kfree(thread->mm);
	//printk("end\n");
	//printk("release bits start ");
	kfree(thread->vir_mem_bitmap.bits);
	//printk("end\n");
	kfree(thread->bag);
	
}

void sys_exit(int status)
{
	struct thread *child_thread = thread_current();
	child_thread->exit_status = status;
	
	release_thread_resource(child_thread);
	
	//死亡，等待被关闭
	//child_thread->status = THREAD_DIED;
	//printk("thread %s exit!\n", child_thread->name);
	//thread_block(THREAD_BLOCKED);
	
	if(child_thread->parent_pid != -1){
		struct thread *parent_thread = pid2thread(child_thread->parent_pid);
		//printk("parent is %s\n", parent_thread->name);
		
		if(parent_thread->status == THREAD_WAITING){
			//printk("parent wait\n");
			
			//父进程在等他
			thread_unblock(parent_thread);
		}else{
			//printk("parent no wait\n");
			
			//父进程没有等他，让maind的recover来处理
			child_thread->parent_pid = -1;
		}
	}
	thread_block(THREAD_HANGING);
}

void thread_kill(struct thread *thread)
{
	//改变成挂起状态不让他执行
	thread->status = THREAD_HANGING;
	
	//把用户空间的代码。数据，栈的页目录信息复制到内核的用户部分，在release中来释放他们
	memcpy((void *)(PAGE_DIR_VIR_ADDR+2048), (void *)&thread->pdir[512], 2044);
	thread->exit_status = 1;
	
	release_thread_resource(thread);

	//修改parent_id后就变成孤儿了,就没有父进程等待，也没有thread_main等待，在thread_exit直接把它退出
	if(thread->parent_pid != -1){
		struct thread *parent_thread = pid2thread(thread->parent_pid);
		//printk("parent is %s\n", parent_thread->name);
		if(parent_thread->status == THREAD_WAITING){
			//printk("parent wait\n");
			//父进程在等他
			thread_unblock(parent_thread);
		}else{
			//printk("parent no wait\n");
			
			//父进程没有等他，让thread_exit来结束他的生命
			thread->parent_pid = -1;
		}
	}
	
}

int sys_wait(int32 *status)
{
	struct thread *parent_thread = thread_current();
	struct thread *child_thread;
	int have_child;
	
	int i;
	//printk("parent %s waitting...\n", parent_thread->name);
	while(1){
		have_child = 0;
		//是否有子进程
		for(i = 0; i < thread_runnings; i++){
			child_thread = thread_ready[i];
			if(child_thread->parent_pid == parent_thread->pid){
				have_child = 1;
				//printk("parent %s waitting...\n", parent_thread->name);
				break;
			}
		}
		if(have_child){
			//如果有子进程退出
			if(child_thread->status == THREAD_HANGING){
				//printk("child %s exit!\n", child_thread->name);
	
				*status = child_thread->exit_status;
				
				int32 child_pid = child_thread->pid;
				
				thread_exit(child_thread);
				
				return child_pid;
			}else{
				thread_block(THREAD_WAITING);
			}
		}else{
			return -1;
		}
	}
}

void thread_graphic_exit(struct thread *thread)
{
	//将当前进程的vidbuf的清空的buffer
	graph_draw_rect(thread->vidbuf->buffer, 0, 0, video_info.width, video_info.height, COLOR_BLACK);
	//释放
	free_vidbuf(thread->vidbuf);
	//默认切换到console
	switch_video_buffer(vidbuf_console);
}

