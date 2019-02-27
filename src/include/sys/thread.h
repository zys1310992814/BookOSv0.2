#ifndef _THREAD_H_
#define	_THREAD_H_

#include <types.h>
#include <stdint.h>
#include <sys/memory.h>
#include <sys/bitmap.h>
#include <sys/gui.h>

typedef void thread_func(void *);

#define THREAD_RUNNING 	1
#define THREAD_READY 	2
#define THREAD_BLOCKED	3
#define THREAD_WAITING	4
#define THREAD_HANGING  5
#define THREAD_DIED		6


#define MAX_THREADS		32

#define MAX_PATH_LEN 128

#define THREAD_DEFAULT_PRO 2

struct intr_stack
{
	uint32 edi;
	uint32 esi;
	uint32 ebp;
	uint32 esp_dummy;
	uint32 ebx;
	uint32 edx;
	uint32 ecx;
	uint32 eax;
	uint32 gs;
	uint32 fs;
	uint32 es;
	uint32 ds;
	
	void (*eip)(void);
	uint32 cs;
	uint32 eflags;
	void *esp;
	uint32 ss;
};

struct thread_stack
{
	uint32 ebp;
	uint32 ebx;
	uint32 edi;
	uint32 esi;
	
	/*
	第一次执行时指向kernel_thread
	其它时候指向switch_to的返回地址
	*/
	void (*eip)(thread_func *func, void *arg);
	
	void (*unused_retaddr);
	thread_func *function;
	void *arg;
};

struct thread
{
	uint8 *self_stack;
	uint8 *kernel_stack;
	
	uint32 status;
	uint32 priority;
	uint32 ticks;
	
	uint32 run_ticks;
	
	char name[16];
	
	int pid;
	int parent_pid;
	
	uint32 *pdir;
	
	struct bitmap vir_mem_bitmap;
	
	struct memory_manage *mm;
	
	int32 exit_status;
	
	char *bag;
	
	char cwd[MAX_PATH_LEN];		//当前工作路径,指针
	
	struct video_buffer *vidbuf;
	
	uint32 stack_magic;
};

struct thread_chain
{
	struct thread *thread[MAX_THREADS];
	uint32 in, out;
};

extern struct thread *user_init_thread;

/*
thread bus
record thread has already init
*/
struct thread_bus
{
	char main, idle;	/*sys*/
	char keyboard, mouse, clock; /*device*/
	char gui;	/*module*/
};

extern struct thread_bus thread_bus;
extern struct thread *thread_graph;



void thread_test();

void init_thread();
struct thread *alloc_thread();
void thread_add(struct thread *thread);
struct thread *thread_pick();
struct thread *thread_current();
bool thread_ready_empty();
int thread_find_running(int levle);
struct thread *next_thread();

void thread_create(struct thread *thread, thread_func function, void *arg);
void thread_init(struct thread *thread, char *name, int prio);
struct thread *thread_start(char *name, int prio, thread_func function, void *arg);

void thread_block(int status);
void thread_unblock(struct thread *thread);
void schedule();

void thread_yield();

void thread_chain_init(struct thread_chain *thread_chain);
void thread_chain_put(struct thread_chain *thread_chain, struct thread *thread);
struct thread *thread_chain_get(struct thread_chain *thread_chain);
bool thread_chain_empty(struct thread_chain *thread_chain);
void thread_init_bitmap(struct thread *thread);
void init_thread_memory_manage(struct thread *thread);
bool thread_find(struct thread *thread);
void thread_remove(struct thread *thread);
struct thread *pid2thread(int pid);
void free_thread(struct thread *thread);
void thread_recover();
void print_threads();

void thread_exit(struct thread *thread);
void thread_kill(struct thread *thread);
void sys_exit(int status);
int sys_wait(int32 *status);

void thread_graphic_exit(struct thread *thread);
int sys_get_pid();

#endif
