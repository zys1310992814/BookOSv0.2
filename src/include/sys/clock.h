#ifndef _DEVICE_CLOCK_H_
#define _DEVICE_CLOCK_H_
#include <types.h>
#include <stdint.h>
#include <time.h>
#include <sys/ioqueue.h>

#define PIT_CTRL	0x0043
//控制端口
#define PIT_CNT0	0x0040
//数据端口
#define TIMER_FREQ     1193200	
#define HZ             100	//1000 快速 100 普通0.001
#define CLOCK_IRQ 0
//时钟中断的irq号

void IRQ_clock();
//汇编处理
void clock_handler(int irq);

struct clock
{
	
	uint32_t ticks;
	uint32_t last_ticks;
	bool can_schdule;
	
	struct ioqueue *ioqueue;
	
};

extern struct clock clock;
extern struct time time;

void print_time();

void init_clock(void);
void ticks_to_sleep(u32 ticks);
void msec_sleep(u32 msec);

struct time *sys_gettime(struct time *tm);

void thread_clock(void *arg);
int sys_get_ticks();
#endif

