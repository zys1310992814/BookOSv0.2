/*
File:		device/mouse.c
Contains:	driver program for mouse
Auther:		Hu Zicheng
Time:		2019/2/12
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/mouse.h>
#include <sys/keyboard.h>
#include <sys/descriptor.h>
#include <sys/memory.h>
#include <sys/clock.h>
#include <sys/x86.h>
#include <sys/video.h>
#include <sys/gui.h>
#include <sys/console.h>
#include <sys/thread.h>

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

struct mouse mouse;
/*
extern struct task *task_mouse;
extern struct layer *layer_desktop;
*/
void init_mouse()
{
	mouse.x = mouse.old_x = 0;		//鼠标信息初始化
	mouse.y = mouse.old_y = 0;
	
	mouse.key_left = MOUSE_IDLE;
	mouse.key_right = MOUSE_IDLE;
	mouse.key_middle = MOUSE_IDLE;
	
	mouse.key_left_continue = MOUSE_IDLE;
	mouse.key_right_continue = MOUSE_IDLE;
	mouse.key_middle_continue = MOUSE_IDLE;

	mouse.key_occur = 0;
	mouse.status = MOUSE_NORMAL;
	
	mouse.ioqueue = create_ioqueue();
	ioqueue_init(mouse.ioqueue);
	
	//初始化鼠标中断
	put_irq_handler(MOUSE_IRQ, mouse_handler);
	//打开从片
	enable_irq(CASCADE_IRQ);
	//打开鼠标中断
	enable_irq(MOUSE_IRQ);
	//激活鼠标前要设定键盘相关信息
	enable_mouse(&mouse);
	
	printk(">init mouse.\n");
	
}

void thread_mouse(void *arg)
{
	//printk("running in mouse\n");
	thread_bus.mouse = 1;
	
	while(1){
		mouse_analysis();
		
	}
}


void enable_mouse(struct mouse *mouse)
{
	/* 激活鼠标 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	/*键盘控制器返回ACK(0xfa)*/
	mouse->phase = 0;
	
	return;
}

void mouse_handler(int32_t irq)
{
	uint8_t data;
	data = io_in8(PORT_KEYDAT);
	ioqueue_put(mouse.ioqueue, data);
}

/*
put button info in buf
buf[0]:left
buf[1]:middle
buf[2]:right
*/

void sys_get_mouse_button(char buf[])
{
	buf[0] = mouse.key_left;
	buf[1] = mouse.key_middle;
	buf[2] = mouse.key_right;
}

/*put x, y in buf*/
int sys_get_mouse_position(int *x, int *y)
{
	/*type:0 获取相对于屏幕的坐标*/
	*x = mouse.x;
	*y = mouse.y;
	return 1;
}

int32_t get_mouse_x()
{
	return mouse.x;
}
int32_t get_mouse_y()
{
	return mouse.y;
}

void mouse_analysis()
{
	int data = ioqueue_get(mouse.ioqueue);
	if(mouse_read(&mouse, data) != 0) {
		//对原始数据操作
		mouse.x += mouse.x_increase;
		mouse.y += mouse.y_increase;
		
		if (mouse.x < 0) {
			mouse.x = 0;
		}
		if (mouse.y < 0) {
			mouse.y = 0;
		}
		if (mouse.x > video_info.width - 1) {
			mouse.x = video_info.width - 1;
		}
		if (mouse.y >= video_info.height - 1) {
			mouse.y = video_info.height - 1;
		}
		
		if((mouse.button & 0x01) != 0 ){
			if(mouse.key_left == MOUSE_IDLE){
				mouse.key_left = MOUSE_DOWN;
			}
		}else{
			if(mouse.key_left == MOUSE_DOWN){
				mouse.key_left = MOUSE_UP;
			}else{
				mouse.key_left = MOUSE_IDLE;
			}
		}
		if((mouse.button & 0x02) != 0){
			if(mouse.key_right == MOUSE_IDLE){
				mouse.key_right = MOUSE_DOWN;
			}
		}else{
			if(mouse.key_right == MOUSE_DOWN){
				mouse.key_right = MOUSE_UP;
			}else{
				mouse.key_right = MOUSE_IDLE;
			}
		}
		
		if((mouse.button & 0x04) != 0){
			if(mouse.key_middle == MOUSE_IDLE){
				mouse.key_middle = MOUSE_DOWN;
			}
		}else{
			
			if(mouse.key_middle == MOUSE_DOWN){
				mouse.key_middle = MOUSE_UP;
			}else{
				mouse.key_middle = MOUSE_IDLE;
			}
		}
		//unit_gui.mouse_event = 1;
		//printk("[mouse]x:%d y:%d\n", mouse.x,mouse.y);
	}
}

int mouse_read(struct mouse *mouse, unsigned char data)
{
	if (mouse->phase == 0) {
		if (data == 0xfa) {
			mouse->phase = 1;
		}
		return 0;
	}
	if (mouse->phase == 1) {
		if ((data & 0xc8) == 0x08) {
			mouse->read_buf[0] = data;
			mouse->phase = 2;
		}
		return 0;
	}
	if (mouse->phase == 2) {
		mouse->read_buf[1] = data;
		mouse->phase = 3;
		return 0;
	}
	if (mouse->phase == 3) {
		mouse->read_buf[2] = data;
		mouse->phase = 1;
		mouse->button = mouse->read_buf[0] & 0x07;
		mouse->x_increase = mouse->read_buf[1];
		mouse->y_increase = mouse->read_buf[2];
		if ((mouse->read_buf[0] & 0x10) != 0) {
			mouse->x_increase |= 0xffffff00;
		}
		if ((mouse->read_buf[0] & 0x20) != 0) {
			mouse->y_increase |= 0xffffff00;
		}
		mouse->y_increase = - mouse->y_increase;
		return 1;
	}
	return -1; 
}