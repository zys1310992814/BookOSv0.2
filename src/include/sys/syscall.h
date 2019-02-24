#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <types.h>

/*BASIC*/
#define _NR_WRITE	0
#define _NR_MALLOC	1
#define _NR_FREE	2
#define _NR_EXIT	3
#define _NR_GETCHAR	4
#define _NR_PUTCHAR	5
#define _NR_EXECV	6
#define _NR_WAIT	7
#define _NR_CLEAR	8
#define _NR_PS		9

/*FS*/
#define _NR_FOPEN	10
#define _NR_FCLOSE	11
#define _NR_FREAD	12
#define _NR_FWRITE	13
#define _NR_LSEEK	14
#define _NR_FSTAT	15
#define _NR_UNLINK	16
#define _NR_OPENDIR	17
#define _NR_CLOSEDIR 18
#define _NR_READDIR	19
#define _NR_REWINDDIR	20
#define _NR_MKDIR	21
#define _NR_RMDIR	22
#define _NR_CHDIR	23
#define _NR_GETCWD	24
#define _NR_RENAME	25
#define _NR_COPY	26
#define _NR_MOVE	27
#define _NR_ACCESS	28

/*SYSTEM*/
#define _NR_REBOOT	30
#define _NR_GETTIME	31
#define _NR_GET_MOUSE_POS	32
#define _NR_GET_MOUSE_BTN	33
#define _NR_GET_SCREEN		34
#define _NR_GET_MEMORY		35



/*GUI*/
#define _NR_INIT_GRAPHIC	40
#define _NR_GRAPH_POINT	41
#define _NR_GRAPH_REFRESH	42
#define _NR_GRAPH_LINE	43
#define _NR_GRAPH_RECT	44
#define _NR_GRAPH_TEXT	45
#define _NR_GRAPHIC_EXIT	46
#define _NR_GRAP_BUFFER	47
#define _NR_GRAP_WORD	48

/*PIPE*/
#define _NR_NEW_PIPE	49
#define _NR_WRITE_PIPE	50
#define _NR_READ_PIPE	51
#define _NR_CLOSE_PIPE	52
#define _NR_INIT_PIPE   53

#define MAX_SYS_CALLS 60

extern sys_call_t sys_call_table[MAX_SYS_CALLS];

void init_syscall();//初始化中断调用

/*interrupt.asm*/
void intrrupt_sys_call();//中断调用服务程序
void sendrec();


#endif

