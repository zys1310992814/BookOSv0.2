/*
File:		kernel/process.c
Contains:	user process
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/


#include <string.h>
#include <sys/process.h>
#include <sys/memory.h>
#include <sys/page.h>
#include <sys/x86.h>
#include <sys/debug.h>
#include <sys/descriptor.h>
#include <sys/tss.h>
#include <sys/console.h>
#include <sys/fatxe.h>
#include <sys/dir.h>
#include <unistd.h>

extern void thread_intr_exit(struct intr_stack *proc_stack);
void process_use_bitmap(struct thread *thread, int count);

void start_process(void *filename)
{
	void *function = filename;
	struct thread *cur = thread_current();
	cur->self_stack += sizeof(struct thread_stack);
	struct intr_stack *proc_stack = (struct intr_stack *)cur->self_stack;
	proc_stack->edi = proc_stack->esi = \
	proc_stack->ebp = proc_stack->esp_dummy = 0;
	
	proc_stack->ebx = proc_stack->edx = \
	proc_stack->ecx = proc_stack->eax = 0;
	
	proc_stack->gs = proc_stack->ds = \
	proc_stack->es = proc_stack->fs = KERNEL_DATA_SEL;
	
	proc_stack->cs = KERNEL_CODE_SEL;
	proc_stack->eip = function;
	
	proc_stack->eflags = (EFLAGS_MBS|EFLAGS_IF_1|EFLAGS_IOPL_0);
	
	proc_stack->esp = (void *)((uint32)thread_use_vaddr(USER_STACK3_ADDR) + PAGE_SIZE*2);
	//proc_stack->esp = (void *)(kmalloc(PAGE_SIZE) + PAGE_SIZE);
	
	proc_stack->ss = KERNEL_STACK_SEL;
	
	thread_intr_exit(proc_stack);
}

void page_dir_activate(struct thread *thread)
{
	uint32 pagedir_phy_addr = PAGE_DIR_PHY_ADDR;
	
	if(thread->pdir != NULL){
		pagedir_phy_addr = addr_v2p((uint32) thread->pdir);
	}
	write_cr3(pagedir_phy_addr);
}

void process_activate(struct thread *thread)
{
	assert(thread != NULL);
	page_dir_activate(thread);
	
	if(thread->pdir){
		update_tss_esp(thread);
	}
}

uint32 *create_page_dir()
{
	uint32 *page_dir_vaddr = (uint32 *)kmalloc(PAGE_SIZE);
	if(page_dir_vaddr == NULL){
		printk("create_page_dir: kmalloc failed!\n");
		return NULL;
	}
	memcpy((void *)page_dir_vaddr, (void *)PAGE_DIR_VIR_ADDR, 2048);
	
	uint32 new_page_dir_phy_addr = addr_v2p((uint32) page_dir_vaddr);
	page_dir_vaddr[1023] = new_page_dir_phy_addr|0x07;
	
	return page_dir_vaddr;
}

int process_execute(void *filename, char *name)
{
	struct thread *thread = alloc_thread();
	thread_init(thread, name, THREAD_DEFAULT_PRO);
	//user vir bitmap
	thread_init_bitmap(thread);
	
	thread_create(thread, start_process, filename);
	thread->pdir = create_page_dir();
	init_thread_memory_manage(thread);
	
	int old_status = io_load_eflags();
	io_cli();
	
	thread_add(thread);
	
	io_store_eflags(old_status);
	return thread->pid;
}

int sys_execv(char *path, char *argv[])
{
	struct thread *parent_thread = thread_current();
	
	struct thread *thread = alloc_thread();
	
	char name[DE_NAME_LEN];
	memset(name,0,DE_NAME_LEN);
	path_to_name(path, name);
	
	/*检测是否是后台程序*/
	int daemon = 0;
	int arg_idx = 0;
	if(argv != NULL){
		while(argv[arg_idx] != NULL){
			if(!strcmp(argv[arg_idx], "&")){
				daemon = 1;
			}
			arg_idx++;
		}
	}
	

	/*检测文件是否存在，是否是可执行文件*/

	if(sys_access(path, F_OK) == -1){
		printk("exec: path:%s file %s not found!\n", path, name);
		return -1;
	}
	if(sys_access(path, X_OK) == -1){
		printk("exec: path:%s file %s can't execute!\n", path, name);
		return -1;
	}
	
	thread_init(thread, name, THREAD_DEFAULT_PRO);
	//user vir bitmap
	thread_init_bitmap(thread);
	
	//做成包，然后传入进去
	thread->bag = kmalloc(PAGE_SIZE);
	if(thread->bag == NULL){
		return -1;
	}
	
	*((int *)thread->bag) = make_arguments(thread->bag+4, argv);
	
	strcpy(thread->bag+(4096-128), path);
	
	thread_create(thread, load_process, NULL);
	thread->pdir = create_page_dir();
	
	if(!daemon){
		//不是后台程序
		thread->parent_pid = parent_thread->pid;
	}else{
		//后台程序，父进程不管他（弃婴-.-）
		thread->parent_pid = -1;
	}
	
	init_thread_memory_manage(thread);
	
	int old_status = io_load_eflags();
	io_cli();
	//printk("ready to run!\n");
	thread_add(thread);
	
	io_store_eflags(old_status);
	
	return thread->pid;
}

void load_process(void *path)
{
	struct thread *cur = thread_current();
	
	cur->self_stack += sizeof(struct thread_stack);
	struct intr_stack *proc_stack = (struct intr_stack *)cur->self_stack;
	proc_stack->edi = proc_stack->esi = \
	proc_stack->ebp = proc_stack->esp_dummy = 0;
	
	proc_stack->ebx = proc_stack->edx = \
	proc_stack->ecx = proc_stack->eax = 0;
	
	proc_stack->gs = proc_stack->ds = \
	proc_stack->es = proc_stack->fs = KERNEL_DATA_SEL;
	
	proc_stack->cs = KERNEL_CODE_SEL;
	
	proc_stack->eflags = (EFLAGS_MBS|EFLAGS_IF_1|EFLAGS_IOPL_0);
	
	proc_stack->esp = (void *)((uint32)thread_use_vaddr(USER_STACK3_ADDR) + PAGE_SIZE);
	//proc_stack->esp = (void *)(kmalloc(PAGE_SIZE*2) + PAGE_SIZE*2);
	
	proc_stack->ss = KERNEL_STACK_SEL;
	//解析包
	
	//load file 
	char *pathname = (char *)cur->bag+(4096-128);
	//printk("load file %s \n", pathname);
	
	int32_t fd = sys_open(pathname, O_RDONLY);
	if (fd == -1) {
		printk("execute: open %s faild!\n",pathname);
		return;
	}
	//get file by fd
	struct file *file = get_file_though_fd(fd);

	//5.link pages with parent vir memory.
	//how many pages should we occupy.
	uint32_t pages = (file->fd_dir->size-1)/PAGE_SIZE + 1;
	
	//vir addr map
	uint8_t *vaddr = (uint8_t *)USER_START_ADDR;
	
	//映射进入用户空间
	map_pages((uint32 )vaddr,pages);
	
	//6.read data form disk into vir memory.
	sys_lseek(fd, 0, SEEK_SET);
	sys_read(fd, (void*)vaddr, file->fd_dir->size);
	
	sys_close(fd);

	process_use_bitmap(cur, pages);
	
	proc_stack->eip = (void *)vaddr;
	proc_stack->ebx = (uint32 )cur->bag+4;
	proc_stack->ecx = *((int *)cur->bag);
	
	//while(1);
	thread_intr_exit(proc_stack);
}

void process_use_bitmap(struct thread *thread, int count)
{
	int i;
	int idx = bitmap_scan(&thread->vir_mem_bitmap, count);
	for(i = 0; i < count; i++){
		bitmap_set(&thread->vir_mem_bitmap, idx + i, 1);
	}
}

int32_t make_arguments(char *buf, char **argv)
{
	//11.make arguments
	uint32_t argc = 0;
	//把参数放到栈中去
	char *arg_stack = (char *)buf;
	if(argv != NULL){
		
		//先复制下来
		while (argv[argc]) {  
			argc++;
		}
		//printk("argc %d\n", argc);
		//构建整个栈
		
		//临时字符串
		int str_stack[STACK_ARGC_MAX/4];
		
		int stack_len = 0;
		//先预留出字符串的指针
		int i;
		for(i = 0; i < argc; i++){
			stack_len += 4;
		}
		//printk("stack_len %d\n", stack_len);
		int idx, len;
		
		for(idx = 0; idx < argc; idx++){
			len = strlen(argv[idx]);
			memcpy(arg_stack + stack_len, argv[idx], len);
			str_stack[idx] = (int )(arg_stack + stack_len);
			//printk("str ptr %x\n",str_stack[idx]);
			arg_stack[stack_len + len + 1] = 0;
			stack_len += len + 1;
			//printk("len %d pos %d\n",len, stack_len);
		}
		//转换成int指针
		int *p = (int *)arg_stack;
		//重新填写地址
		for(idx = 0; idx < argc; idx++){
			p[idx] = str_stack[idx];
			//printk("ptr %x\n",p[idx]);
		}
		//p[argc] = NULL;
		//指向地址
		//char **argv2 = (char **)arg_stack;
		
		//argv2[argc] = NULL;
		//打印参数
		for (i = 0; i < argc; i++) {  
			//printk("args:%s\n",argv2[i]);
		}
		
	}
	return argc;
}

