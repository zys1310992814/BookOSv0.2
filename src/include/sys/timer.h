#ifndef _TIMER_H_
#define _TIMER_H_
#include <types.h>
#include <sys/ioqueue.h>

/*最多有多少个定时器*/
#define MAX_TIMER_NR 32

/*用枚举的方式来表示定时器的状态*/
enum timer_status
{
	TIMER_UNUSED,
	TIMER_USING,
	TIMER_ACTIVE,
};

/*
#define TIMER_UNUSED 	0
#define TIMER_ALLOCED	1
#define TIMER_ACTIVE	2
*/

/*最大的一个定时器的ticks值*/
#define MAX_TIMER_TICKS	 0xffffff00

struct timer
{
	uint32 ticks;	//到达该定时器的ticks
	enum timer_status status;	//定时器状态
	uint32 occured;		//定时器超时的标志
	struct timer *next;	//下一个定时器的结构体指针
};

struct timer_module
{
	uint32 next_ticks;	//下一个即将到达的定时器的ticks
	struct timer *next_timer;	//下一个定时器的指针
	struct timer timer_table[MAX_TIMER_NR];	//把定时器做成一个数组保存
	struct ioqueue *ioqueue;	//定时器模块的环形缓冲
};
/*导出定时器模块*/
extern struct timer_module *timer_module;

/*定时器环境*/
void init_timer();
void thread_timer(void *arg);

/*对于定时器的操作*/
struct timer *timer_alloc(void);
void timer_free(struct timer *timer);
int timer_settime(struct timer *timer, uint32 timeout);
int timer_cancel(struct timer *timer);
int timer_occur(struct timer *timer);

#endif

