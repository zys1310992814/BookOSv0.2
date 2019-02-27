/*
File:		kernel/syscall.c
Contains:	syscall set here 
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/syscall.h>
#include <sys/console.h>
#include <sys/memory.h>
#include <sys/keyboard.h>
#include <sys/debug.h>
#include <sys/thread.h>
#include <sys/process.h>
#include <sys/fatxe.h>
#include <sys/dir.h>
#include <sys/system.h>
#include <sys/clock.h>
#include <sys/graphic.h>
#include <sys/mouse.h>
#include <sys/video.h>
#include <sys/pipe.h>

/**
	添加一个系统调用步骤：
	1.sys_call_table中添加函数名
	2.系统调用函数体和头文件
	3.在user的syscall中添加中断调用和头文件
*/

sys_call_t sys_call_table[MAX_SYS_CALLS];

void init_syscall()
{
	sys_call_table[_NR_WRITE] = sys_write_str;
	sys_call_table[_NR_MALLOC] = sys_memory_alloc;
	sys_call_table[_NR_FREE] = sys_memory_free;
	sys_call_table[_NR_EXIT] = sys_exit;
	sys_call_table[_NR_GETCHAR] = sys_get_key;
	sys_call_table[_NR_PUTCHAR] = sys_writ_char;
	sys_call_table[_NR_EXECV] = sys_execv;
	sys_call_table[_NR_WAIT] = sys_wait;
	sys_call_table[_NR_CLEAR] = sys_clean_screen;
	sys_call_table[_NR_PS] = print_threads;
	
	sys_call_table[_NR_FOPEN] = sys_open;
	sys_call_table[_NR_FCLOSE] = sys_close;
	sys_call_table[_NR_FREAD] = sys_read;
	sys_call_table[_NR_FWRITE] = sys_write;
	sys_call_table[_NR_LSEEK] = sys_lseek;
	sys_call_table[_NR_FSTAT] = sys_stat;
	sys_call_table[_NR_UNLINK] = sys_unlink;
	sys_call_table[_NR_OPENDIR] = sys_opendir;
	sys_call_table[_NR_CLOSEDIR] = sys_close_dir;
	sys_call_table[_NR_READDIR] = sys_readdir;
	sys_call_table[_NR_REWINDDIR] = sys_rewinddir;
	sys_call_table[_NR_MKDIR] = sys_mkdir;
	sys_call_table[_NR_RMDIR] = sys_rmdir;
	sys_call_table[_NR_CHDIR] = sys_chdir;
	sys_call_table[_NR_GETCWD] = sys_getcwd;
	sys_call_table[_NR_RENAME] = sys_rename;
	sys_call_table[_NR_COPY] = sys_copy;
	sys_call_table[_NR_MOVE] = sys_move;
	sys_call_table[_NR_ACCESS] = sys_access;
	
	sys_call_table[_NR_REBOOT] = sys_reboot;
	sys_call_table[_NR_GETTIME] = sys_gettime;
	sys_call_table[_NR_GET_MOUSE_POS] = sys_get_mouse_position;
	sys_call_table[_NR_GET_MOUSE_BTN] = sys_get_mouse_button;
	sys_call_table[_NR_GET_SCREEN] = sys_get_screen;
	sys_call_table[_NR_GET_MEMORY] = sys_get_memory;
	sys_call_table[_NR_GET_PID] = sys_get_pid;
	
	
	
	sys_call_table[_NR_INIT_GRAPHIC] = sys_init_graphic;
	sys_call_table[_NR_GRAPH_POINT] = sys_graph_point;
	sys_call_table[_NR_GRAPH_REFRESH] = sys_graph_refresh;
	sys_call_table[_NR_GRAPH_LINE] = sys_graph_line;
	sys_call_table[_NR_GRAPH_RECT] = sys_graph_rect;
	sys_call_table[_NR_GRAPH_TEXT] = sys_graph_text;
	sys_call_table[_NR_GRAPHIC_EXIT] = sys_graphic_exit;
	sys_call_table[_NR_GRAP_BUFFER] = sys_graph_buffer;
	sys_call_table[_NR_GRAP_WORD] = sys_graph_char;

	sys_call_table[_NR_NEW_PIPE] = sys_new_pipe;
	sys_call_table[_NR_READ_PIPE] = sys_read_pipe;
	sys_call_table[_NR_WRITE_PIPE] = sys_write_pipe;
	sys_call_table[_NR_CLOSE_PIPE] = sys_close_pipe;
	
	
}
