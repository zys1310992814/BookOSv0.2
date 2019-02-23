/*
File:		device/keyboard.c
Contains:	driver program for keyboard
Auther:		Hu Zicheng
Time:		2019/1/29
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/keyboard.h>
#include <sys/descriptor.h>
#include <sys/x86.h>
#include <sys/console.h>
#include <sys/keymap.h>
#include <sys/ioqueue.h>
#include <sys/thread.h>
#include <sys/graphic.h>

uint8_t get_byte_from_kbuf() ;

static void kb_wait();
static void kb_ack();
static void set_leds();

void key_char_process(uint32_t key);
void put_key(uint32_t key);
void wait_KBC_sendready(void);

struct keyboard keyboard;

void init_keyboard()
{
	
	keyboard.code_with_E0 = 0;
	
	keyboard.shift_l	= keyboard.shift_r = 0;
	keyboard.alt_l	= keyboard.alt_r   = 0;
	keyboard.ctrl_l	= keyboard.ctrl_r  = 0;
	
	keyboard.caps_lock   = 0;
	keyboard.num_lock    = 1;
	keyboard.scroll_lock = 0;
	
	keyboard.key = -1;
	
	keyboard.ioqueue = create_ioqueue();
	ioqueue_init(keyboard.ioqueue);
	
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	
	put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
	enable_irq(KEYBOARD_IRQ);
	
	printk(">init keyboard.\n");
}

void thread_keyboard(void *arg)
{
	//printk("running in keyboard\n");
	thread_bus.keyboard = 1;
	
	while(1){
		keyboard_analysis();
		
		
	}
}

void keyboard_handler(int32_t irq)
{
	uint8_t scan_code = io_in8(KB_DATA);
	ioqueue_put(keyboard.ioqueue, scan_code);
	
}

void wait_KBC_sendready()
{
	/*等待键盘控制电路准备完毕*/
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}



/*
把数据交给keyboard处理
*/

void keyboard_analysis()
{
	uint8_t scan_code;
	int make;
	
	uint32_t key = 0;
	uint32_t* keyrow;

		keyboard.code_with_E0 = 0;

		scan_code = get_byte_from_kbuf();
		
		if(scan_code == 0xe1){
			int i;
			uint8_t pausebrk_scode[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
			int is_pausebreak = 1;
			for(i=1;i<6;i++){
				if (get_byte_from_kbuf() != pausebrk_scode[i]) {
					is_pausebreak = 0;
					break;
				}
			}
			if (is_pausebreak) {
				key = PAUSEBREAK;
			}
		} else if(scan_code == 0xe0){
			scan_code = get_byte_from_kbuf();

			//PrintScreen 被按下
			if (scan_code == 0x2A) {
				if (get_byte_from_kbuf() == 0xE0) {
					if (get_byte_from_kbuf() == 0x37) {
						key = PRINTSCREEN;
						make = 1;
					}
				}
			}
			//PrintScreen 被释放
			if (scan_code == 0xB7) {
				if (get_byte_from_kbuf() == 0xE0) {
					if (get_byte_from_kbuf() == 0xAA) {
						key = PRINTSCREEN;
						make = 0;
					}
				}
			}
			//不是PrintScreen, 此时scan_code为0xE0紧跟的那个值. 
			if (key == 0) {
				keyboard.code_with_E0 = 1;
			}
		}if ((key != PAUSEBREAK) && (key != PRINTSCREEN)) {
			make = (scan_code & FLAG_BREAK ? 0 : 1);

			//先定位到 keymap 中的行 
			keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];
			
			keyboard.column = 0;
			int caps = keyboard.shift_l || keyboard.shift_r;
			if (keyboard.caps_lock) {
				if ((keyrow[0] >= 'a') && (keyrow[0] <= 'z')){
					caps = !caps;
				}
			}
			if (caps) {
				keyboard.column = 1;
			}

			if (keyboard.code_with_E0) {
				keyboard.column = 2;
			}
			
			key = keyrow[keyboard.column];
			
			switch(key) {
			case SHIFT_L:
				keyboard.shift_l = make;
				break;
			case SHIFT_R:
				keyboard.shift_r = make;
				break;
			case CTRL_L:
				keyboard.ctrl_l = make;
				break;
			case CTRL_R:
				keyboard.ctrl_r = make;
				break;
			case ALT_L:
				keyboard.alt_l = make;
				break;
			case ALT_R:
				keyboard.alt_l = make;
				break;
			case CAPS_LOCK:
				if (make) {
					keyboard.caps_lock   = !keyboard.caps_lock;
					set_leds();
				}
				break;
			case NUM_LOCK:
				if (make) {
					keyboard.num_lock    = !keyboard.num_lock;
					set_leds();
				}
				break;
			case SCROLL_LOCK:
				if (make) {
					keyboard.scroll_lock = !keyboard.scroll_lock;
					set_leds();
				}
				break;	
			default:
				break;
			}
			
			if (make) { //忽略 Break Code
				int pad = 0;

				//首先处理小键盘
				if ((key >= PAD_SLASH) && (key <= PAD_9)) {
					pad = 1;
					switch(key) {
					case PAD_SLASH:
						key = '/';
						break;
					case PAD_STAR:
						key = '*';
						break;
					case PAD_MINUS:
						key = '-';
						break;
					case PAD_PLUS:
						key = '+';
						break;
					case PAD_ENTER:
						key = ENTER;
						break;
					default:
						if (keyboard.num_lock &&
						    (key >= PAD_0) &&
						    (key <= PAD_9)) 
						{
							key = key - PAD_0 + '0';
						}else if (keyboard.num_lock &&
							(key == PAD_DOT)) 
						{
							key = '.';
						}else{
							switch(key) {
							case PAD_HOME:
								key = HOME;
								
								break;
							case PAD_END:
								key = END;
								
								break;
							case PAD_PAGEUP:
								key = PAGEUP;
								
								break;
							case PAD_PAGEDOWN:
								key = PAGEDOWN;
								
								break;
							case PAD_INS:
								key = INSERT;
								break;
							case PAD_UP:
								key = UP;
								break;
							case PAD_DOWN:
								key = DOWN;
								break;
							case PAD_LEFT:
								key = LEFT;
								break;
							case PAD_RIGHT:
								key = RIGHT;
								break;
							case PAD_DOT:
								key = DELETE;
								break;
							default:
								break;
							}
						}
						break;
					}
				}
				
				key |= keyboard.shift_l	? FLAG_SHIFT_L	: 0;
				key |= keyboard.shift_r	? FLAG_SHIFT_R	: 0;
				key |= keyboard.ctrl_l	? FLAG_CTRL_L	: 0;
				key |= keyboard.ctrl_r	? FLAG_CTRL_R	: 0;
				key |= keyboard.alt_l	? FLAG_ALT_L	: 0;
				key |= keyboard.alt_r	? FLAG_ALT_R	: 0;
				key |= pad      ? FLAG_PAD      : 0;
				key_char_process(key);
			}else{
				//key_char_process(0);
				//key_char_process(key);
			}
		}
}

void key_char_process(uint32_t key)
{
	//char output[2] = {'\0', '\0'};

	if (!(key & FLAG_EXT)) {
		
		put_key(key);
	} else {
		
		int raw_code = key & MASK_RAW;
		
		switch(raw_code) {
			case ENTER:
				put_key('\n');
				break;
			case BACKSPACE:
				put_key( '\b');
				break;
			case TAB:
				if(key & FLAG_SHIFT_L || key & FLAG_SHIFT_R){
					//在shell和应用程序之间切换
					struct video_buffer *vidbuf = get_next_vidbuf();
					switch_video_buffer(vidbuf);
					
				}else{
					put_key( '\t');
				}
				break;
			case F1:
				put_key( F1);
				/*if(key & FLAG_ALT_L || key & FLAG_ALT_R){
					select_console(0);
				}*/
				break;
			case F2:
				put_key( F2);
				/*if(key & FLAG_ALT_L || key & FLAG_ALT_R){
					select_console(1);
				}*/
				break;
				
			case F3:
				//shft +f4 关闭程序
				put_key( F3);
				/*if(key & FLAG_ALT_L || key & FLAG_ALT_R){
					if(current_console->id == CONSOLE_TEXT && current_console->status == CONSOLE_ACTIVE){
						console_close();
					}
				}*/
				break;
			case F4:
				//alt +f4 关闭程序
				if(key & FLAG_SHIFT_L || key & FLAG_SHIFT_R){
					if(current_vidbuf != vidbuf_console){
						current_vidbuf->thread->status = THREAD_HANGING;
						
						thread_graphic_exit(current_vidbuf->thread);
					}
				}
				//put_key( F4);
				break;	
			case F5:
				put_key( F5);
				break;	  
			case F6:
				put_key( F6);
				break;	   
			case F7:
				put_key( F7);
				break;	  
			case F8:
				put_key( F8);
				break;	  
			case F9:
				put_key( F9);
				break;	  
			case F10:
				put_key( F10);
				break;	  
			case F11:
				put_key( F11);
				break;	  
			case F12:  
				put_key( F12);
				break;
			case ESC:
				put_key(ESC);
				
				break;	
			case UP:
				put_key(UP);
				/*if(key & FLAG_CTRL_L || key & FLAG_CTRL_R){
					scroll_screen(current_console, SCREEN_UP);
				}else{
					put_key( UP);
				}*/
				break;
			case DOWN:
				put_key(DOWN);
				/*if(key & FLAG_CTRL_L || key & FLAG_CTRL_R){
					scroll_screen(current_console, SCREEN_DOWN);
				}else{
					put_key(DOWN);
				}*/
				break;
			case LEFT:
				
				put_key(LEFT);
				break;
			case RIGHT:
				put_key(RIGHT);
				break;
			case PAGEUP:
				put_key(PAGEUP);
				break;
			case PAGEDOWN:
				put_key(PAGEDOWN);
				break;	
			case HOME:
				put_key(HOME);
				break;	
			case END:
				put_key(END);
				break;	
			case INSERT:
				put_key(INSERT);
				break;	
			case DELETE:
				put_key(DELETE);
				break;	
			default:
				break;
		}
	}
}

void put_key(uint32_t key)
{
	/*if(current_console->status == CONSOLE_ACTIVE){
		current_console->keybord_data = key;
	}*/
	keyboard.key = key;
}

int sys_get_key()
{
	struct thread *cur = thread_current();
	
	int key = -1;
	if(cur->vidbuf == current_vidbuf || current_vidbuf == vidbuf_console){
		key = keyboard.key;
		keyboard.key = -1;
	}
	return key; 
}

//从键盘缓冲区中读取下一个字节
uint8_t get_byte_from_kbuf()       
{
	return (uint8_t )ioqueue_get(keyboard.ioqueue);
}
// 等待 8042 的输入缓冲区空
static void kb_wait()
{
	uint8_t kb_stat;

	do {
		kb_stat = io_in8(KB_CMD);
	} while (kb_stat & 0x02);
}

static void kb_ack()
{
	uint8_t kb_read;

	do {
		kb_read = io_in8(KB_DATA);
	} while ((kb_read =! KB_ACK));
	
}

static void set_leds()
{
	uint8_t leds = (keyboard.caps_lock << 2) | (keyboard.num_lock << 1) | keyboard.scroll_lock;

	kb_wait();
	io_out8(KB_DATA, LED_CODE);
	kb_ack();

	kb_wait();
	io_out8(KB_DATA, leds);
	kb_ack();
}

