/*
File:		device/clock.c
Contains:	driver for clock
Auther:		Hu Zicheng
Time:		2019/1/29
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/clock.h>
#include <sys/x86.h>
#include <sys/descriptor.h>
#include <sys/memory.h>
#include <sys/8259a.h>
#include <sys/console.h>
#include <sys/cmos.h>
#include <sys/thread.h>
#include <math.h>

struct clock clock;

struct time time;

/*
clock_switch will ofen use in clock interruption, we statement here
*/

void clock_change_date_time();

void init_clock(void)
{

	//0.001秒触发一次中断
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, (unsigned char) (TIMER_FREQ/HZ));
	io_out8(PIT_CNT0, (unsigned char) ((TIMER_FREQ/HZ) >> 8));

	clock.last_ticks = clock.ticks = 0;
	
	clock.can_schdule = true;
	
	clock.ioqueue = create_ioqueue();
	ioqueue_init(clock.ioqueue);
	
	//用一个循环让秒相等
	do{
		time.year = get_year();
		time.month = get_mon_hex();
		time.day = get_day_of_month();
		
		time.hour = get_hour_hex();
		time.minute =  get_min_hex8();
		time.second = get_sec_hex();
		
		//time.week_day = get_day_of_week();
		
		//trans time to 24 format
		/*if(time.hour >= 16){
			time.hour -= 16;
		}else{
			time.hour += 8;
		}*/
	}while(time.second != get_sec_hex());

	put_irq_handler(CLOCK_IRQ, clock_handler);
	
	enable_irq(CLOCK_IRQ);
	printk(">init clock\n");

	//print_time();
	
}

void clock_handler(int irq)
{
	struct thread *thread = thread_current();
	clock.ticks++;
	if(clock.ticks%100 == 0){
		ioqueue_put(clock.ioqueue, 0);
	}	
	//clock_change_date_time();
	if(thread->ticks == 0){
		schedule();
	}else{
		thread->ticks--;
		thread->run_ticks++;
	}
}

void thread_clock(void *arg)
{
	//printk("running in clock\n");
	thread_bus.clock = 1;
	
	while(1){
		ioqueue_get(clock.ioqueue);	
		clock_change_date_time();
		
	}
}

int sys_get_ticks()
{
	return clock.ticks;
}

void clock_change_date_time()
{
	time.second++;
	if(time.second >= 60){
		time.minute++;
		time.second = 0;
		if(time.minute >= 60){
			time.hour++;
			time.minute = 0;
			if(time.hour >= 24){
				time.day++;
				time.hour = 0;
				//现在开始就要判断平年和闰年，每个月的月数之类的
				if(time.day > 30){
					time.month++;
					time.day = 1;
					
					if(time.month > 12){
						time.year++;	//到年之后就不用调整了
						time.month = 1;
					}
				}
			}
		}
	}
}

void print_time()
{
	printk("=====Time&Date=====\n");
	printk("Time:%d:%d:%d Date:%d/%d/%d ", \
		time.hour, time.minute, time.second,\
		time.year, time.month, time.day);
	
	/*char *weekday[7];
	weekday[0] = "Monday";
	weekday[1] = "Tuesday";
	weekday[2] = "Wednesday";
	weekday[3] = "Thursday";
	weekday[4] = "Friday";
	weekday[5] = "Saturday";
	weekday[6] = "Sunday";
	printk("weekday:%d", time.week_day);
	*/
}


void ticks_to_sleep(u32 ticks)
{
	u32 t = clock.ticks;
	while(clock.ticks - t < ticks){
		/*sleep*/
		thread_yield();
	}
}

void msec_sleep(u32 msec)
{
	u32 t = DIV_ROUND_UP(msec, 10);
	ticks_to_sleep(t);
}

struct time *sys_gettime(struct time *tm)
{
	if(tm == NULL){
		return &time;
	}else{
		tm->second = time.second;
		tm->minute = time.minute;
		tm->hour = time.hour;
		tm->day = time.day;
		tm->month = time.month;
		tm->year = time.year;
		tm->second = time.second;
		tm->week_day = time.week_day;
		tm->year_day = time.year_day;
		tm->is_dst = time.is_dst;
		return tm;
	}
}
