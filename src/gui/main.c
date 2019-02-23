/*
File:		gui/main.c
Contains:	gui main init
Auther:		Hu Zicheng
Time:		2019/2/13
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/video.h>
#include <sys/gui.h>
#include <sys/graphic.h>
#include <sys/mouse.h>
#include <sys/font.h>
#include <sys/clock.h>
#include <sys/console.h>
#include <sys/color.h>
#include <graphic.h>
#include <sys/process.h>
#include <sys/kernel.h>
#include <sys/board.h>

void init_terminal();
void terminal_char(char ch);
void terminal_scroll();

struct video_buffer vidbuf_table[MAX_VIDEO_BUF];

struct thread *thread_graph;

struct video_buffer *vidbuf_console;
struct video_buffer *current_vidbuf;

void init_gui()
{
	init_font();
	
	int i;
	for(i = 0; i < MAX_VIDEO_BUF; i++){
		vidbuf_table[i].buffer = kmalloc(video_info.width*video_info.height*video_info.pixel_width);
		vidbuf_table[i].flags = 0;
		vidbuf_table[i].id = i;
		
	}
	vidbuf_console = alloc_vidbuf();
	vidbuf_console->buffer = kmalloc(video_info.width*video_info.height*video_info.pixel_width);
	
	current_vidbuf = vidbuf_console;
	init_terminal();
	
}

struct video_buffer *alloc_vidbuf()
{
	int i;
	for(i = 0; i < MAX_VIDEO_BUF; i++){
		if(vidbuf_table[i].flags == 0){
			vidbuf_table[i].flags = 1;
			return &vidbuf_table[i];
		}
	}
	return NULL;
}

void free_vidbuf(struct video_buffer *vidbuf)
{
	vidbuf->flags = 0;
}


struct video_buffer *get_next_vidbuf()
{

	struct video_buffer *vidbuf = NULL;
	int id = current_vidbuf->id;
	while(1){
		//切换到下一个id
		id++;
		if(id >= MAX_VIDEO_BUF){
			id = 0;
		}
		
		//获取一个vidbuf
		vidbuf = &vidbuf_table[id];
		//是一个使用中的，切换过去
		if(vidbuf->flags == 1){
			//退出循环
			break;
		}
	}
	return vidbuf;
}

struct terminal terminal;

void init_terminal()
{
	terminal.x = 0;
	terminal.y = 0;
	
	terminal.x_size = video_info.width/CHAR_WIDTH;
	terminal.y_size = video_info.height/CHAR_HEIGHT;
	
	terminal.cursor_color = RGB(230, 230, 0);
	terminal.font_color = RGB(220, 220, 220);
	terminal.bc_color = RGB(0,50,100);
	
	graph_draw_rect(vidbuf_console->buffer, 0, 0, video_info.width, video_info.height, terminal.bc_color);
	
	graph_draw_rect(vidbuf_console->buffer, terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, terminal.cursor_color);
	
	graph_refresh(vidbuf_console->buffer, 0, 0, video_info.width, video_info.height);
	
	/*指向图形模式的文字显示*/
	display_char_func = terminal_char;
	sys_clean_screen = terminal_clean;

}

void terminal_char(char ch)
{
	
	switch(ch){
		case '\n':
			graph_draw_rect(vidbuf_console->buffer, terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, terminal.bc_color);
			graph_refresh(vidbuf_console->buffer, terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, terminal.x*CHAR_WIDTH+CHAR_WIDTH, terminal.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		
			terminal.x = 0;
			terminal.y++;
			if(terminal.y >= terminal.y_size){
				//滚屏
				terminal_scroll();
			}
			graph_draw_rect(vidbuf_console->buffer, terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, terminal.cursor_color);
			graph_refresh(vidbuf_console->buffer, terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, terminal.x*CHAR_WIDTH+CHAR_WIDTH, terminal.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
			break;
		case '\b':
			graph_draw_rect(vidbuf_console->buffer, terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, terminal.bc_color);
			graph_refresh(vidbuf_console->buffer, terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, terminal.x*CHAR_WIDTH+CHAR_WIDTH, terminal.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
			terminal.x--;
			if(terminal.x < 0){	
				terminal.x = 0;
			}
			graph_draw_rect(vidbuf_console->buffer, terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, terminal.cursor_color);
			graph_refresh(vidbuf_console->buffer, terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, terminal.x*CHAR_WIDTH+CHAR_WIDTH, terminal.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
			break;
		default: 
			graph_draw_rect(vidbuf_console->buffer, terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, terminal.bc_color);
			graph_draw_word(vidbuf_console->buffer, terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, terminal.font_color, ch);
			graph_refresh(vidbuf_console->buffer, terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, terminal.x*CHAR_WIDTH+CHAR_WIDTH, terminal.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
			
			terminal.x++;
			if(terminal.x >= terminal.x_size){
				terminal.x = 0;
				terminal.y++;
			}
			if(terminal.y >=  terminal.y_size){
				//滚屏
				terminal_scroll();
			}
			graph_draw_rect(vidbuf_console->buffer,terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, terminal.cursor_color);
			graph_refresh(vidbuf_console->buffer, terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, terminal.x*CHAR_WIDTH+CHAR_WIDTH, terminal.y*CHAR_HEIGHT+CHAR_HEIGHT);
			break;
	}
}

void terminal_clean()
{
	terminal.x = 0;
	terminal.y = 0;
	graph_draw_rect(vidbuf_console->buffer, 0, 0, terminal.x_size*CHAR_WIDTH, terminal.x_size*CHAR_HEIGHT, terminal.bc_color);
	graph_draw_rect(vidbuf_console->buffer, terminal.x*CHAR_WIDTH, terminal.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, terminal.cursor_color);
	graph_refresh(vidbuf_console->buffer, 0, 0, terminal.x_size*CHAR_WIDTH, terminal.y_size*CHAR_HEIGHT);
}

void terminal_scroll()
{
	//复制原来的数据
	uint32 *src = (uint32 *)(vidbuf_console->buffer + CHAR_HEIGHT*video_info.width*video_info.pixel_width);
	uint32 *dst = (uint32 *)(vidbuf_console->buffer + video_info.width*video_info.pixel_width);
	int size = terminal.y_size*CHAR_HEIGHT*terminal.x_size*CHAR_WIDTH*video_info.pixel_width/4;
	while(size > 0){
		*dst++ = *src++;
		size--;
	}
	//填充最后一行
	terminal.y--;
	/*最后可能存在一行多一点，但是也当做一行，所以这里填充2行的，让剩余部分也填充*/
	graph_draw_rect(vidbuf_console->buffer,0,terminal.y*CHAR_HEIGHT,terminal.x_size*CHAR_WIDTH,CHAR_HEIGHT*2, terminal.bc_color);
	//刷新
	graph_refresh(vidbuf_console->buffer, 0, 0, terminal.x_size*CHAR_WIDTH, terminal.y_size*CHAR_HEIGHT);
}
