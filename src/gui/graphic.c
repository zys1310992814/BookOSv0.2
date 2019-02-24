/*
File:		gui/graphic.c
Contains:	draw graph 
Auther:		Hu Zicheng
Time:		2019/2/13
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/graphic.h>
#include <sys/video.h>
#include <sys/font.h>
#include <sys/thread.h>
#include <sys/console.h>
#include <sys/gui.h>
#include <string.h>
#include <sys/x86.h>
#include <sys/board.h>

void sys_graph_point(struct guiatom *atom)
{
	struct thread *cur = thread_current();
	
	graph_write_pixel(cur->vidbuf->buffer, atom->x, atom->y, atom->color);
	/*struct thread *cur = thread_current();
	
	window_draw_pixel(cur->layer, atom->x, atom->y, atom->color);*/
}

void sys_graph_refresh(struct guiatom *atom)
{
	struct thread *cur = thread_current();
	
	graph_refresh(cur->vidbuf->buffer, atom->x0, atom->y0, atom->x1, atom->y1);
	/*struct thread *cur = thread_current();
	
	window_refresh(cur->layer, atom->x0, atom->y0, atom->x1, atom->y1);*/
}

void sys_graph_line(struct guiatom *atom)
{
	struct thread *cur = thread_current();
	
	graph_draw_line(cur->vidbuf->buffer, atom->x0, atom->y0, atom->x1, atom->y1, atom->color);
	/*struct thread *cur = thread_current();
	window_draw_line(cur->layer, atom->x0, atom->y0, atom->x1, atom->y1, atom->color);*/
}

void sys_graph_rect(struct guiatom *atom)
{
	struct thread *cur = thread_current();
	
	graph_draw_rect(cur->vidbuf->buffer, atom->x, atom->y, atom->width, atom->height, atom->color);
	/*struct thread *cur = thread_current();
	window_draw_rect(cur->layer, atom->x, atom->y, atom->width, atom->height, atom->color);*/
}

void sys_graph_text(struct guiatom *atom)
{
	struct thread *cur = thread_current();
	
	graph_draw_string(cur->vidbuf->buffer, atom->x, atom->y, atom->string, atom->color);
	/*struct thread *cur = thread_current();
	window_draw_string(cur->layer, atom->x, atom->y, atom->string, atom->color);*/
}

void sys_graph_char(struct guiatom *atom)
{
	struct thread *cur = thread_current();
	
	graph_draw_word(cur->vidbuf->buffer, atom->x, atom->y, atom->color, atom->word);
	/*struct thread *cur = thread_current();
	window_draw_string(cur->layer, atom->x, atom->y, atom->string, atom->color);*/
}
void sys_graph_buffer(struct guiatom *atom)
{
	struct thread *cur = thread_current();
	
	graph_draw_buffer(cur->vidbuf->buffer, atom->x, atom->y, atom->width, atom->height, atom->buffer);
	/*struct thread *cur = thread_current();
	window_draw_string(cur->layer, atom->x, atom->y, atom->string, atom->color);*/
}

void graph_read_pixel(uint8 *buffer, int x, int y, uint32 *color)
{
	
	if(x < 0 || y < 0 || x >= video_info.width || y >= video_info.height){
		return;
	}
	uint8 *buf = (uint8 *)(buffer + ((y*video_info.width+x))*video_info.pixel_width);
	
	uint8 cbuf[4];
	#ifdef _VIDEO_16
		cbuf[0] = *buf++;
		cbuf[1] = *buf++;
		cbuf[2] = *buf;
		cbuf[3] = 0;
	#elif defined _VIDEO_24
		cbuf[0] = *buf++;
		cbuf[1] = *buf;
		cbuf[2] = 0;
		cbuf[3] = 0;
	#endif
	
	*color = *((uint32 *)cbuf);
}

void graph_write_pixel(uint8 *buffer, int x, int y, uint32 color)
{
	
	if(x < 0 || y < 0 || x >= video_info.width || y >= video_info.height){
		return;
	}
	uint8 *buf = (uint8 *)(buffer + ((y*video_info.width+x))*video_info.pixel_width);
	
	#ifdef _VIDEO_16
		*buf++ = color&0xff;
		*buf++ = (color>>8)&0xff;
	#elif defined _VIDEO_24
		*buf++ = color&0xff;
		*buf++ = (color>>8)&0xff;
		*buf = (color>>16)&0xff;
	#endif
}

void graph_draw_rect(uint8 *buffer, int x, int y, uint32 width,uint32 height, uint32 color)
{
	/*wide-video wide*/
	uint32 x0, y0;
	for (y0 = 0; y0 < height; y0++) {
		for (x0 = 0; x0 < width; x0++) {
			graph_write_pixel(buffer, x + x0,y + y0, color);
		}
	}
}

void graph_draw_char_bit(uint8 *buffer, int x, int y , uint32 color, uint8 *ascii)
{
	int i;
	char d /* data */;
	for (i = 0; i < 16; i++) {
		d = ascii[i];
		if ((d & 0x80) != 0) { graph_write_pixel(buffer, x + 0, y + i, color); }
		if ((d & 0x40) != 0) { graph_write_pixel(buffer, x + 1, y + i, color); }
		if ((d & 0x20) != 0) { graph_write_pixel(buffer, x + 2, y + i, color); }
		if ((d & 0x10) != 0) { graph_write_pixel(buffer, x + 3, y + i, color); }
		if ((d & 0x08) != 0) { graph_write_pixel(buffer, x + 4, y + i, color); }
		if ((d & 0x04) != 0) { graph_write_pixel(buffer, x + 5, y + i, color); }
		if ((d & 0x02) != 0) { graph_write_pixel(buffer, x + 6, y + i, color); }
		if ((d & 0x01) != 0) { graph_write_pixel(buffer, x + 7, y + i, color); }
	}
	
}

void graph_draw_word(uint8 *buffer, int x,int y,uint32 color, char ch)
{
	graph_draw_char_bit(buffer, x, y, color, current_font->addr + ch * current_font->height);
	
}

void graph_draw_string(uint8 *buffer, int x,int y, char *s, uint32 color)
{
	for (; *s != 0x00; s++) {
		graph_draw_word(buffer, x, y, color, *s);
		x += current_font->width;
	}
}

void graph_draw_line(uint8 *buffer,int x0, int y0, int x1, int y1, uint32 color)
{
	int i, x, y, len, dx, dy;
	dx = x1 - x0;
	dy = y1 - y0;
	
	x = x0 << 10;
	y = y0 << 10;
	
	if(dx < 0){
		dx = -dx;
	}
	if(dy < 0){
		dy = -dy;
	}
	if(dx >= dy ){
		len = dx + 1;
		if(x0 > x1){
			dx = -1024;
		} else {
			dx = 1024;
			
		}
		if(y0 <= y1){
			dy = ((y1 - y0 + 1) << 10)/len;
		} else {
			dy = ((y1 - y0 - 1) << 10)/len;
		}
		
		
	}else{
		len = dy + 1;
		if(y0 > y1){
			dy = -1024;
		} else {
			dy = 1024;
			
		}
		if(x0 <= x1){
			dx = ((x1 - x0 + 1) << 10)/len;
		} else {
			dx = ((x1 - x0 - 1) << 10)/len;
		}	
	}
	for(i = 0; i < len; i++){
		//buf[((y >> 10)*wide + (x >> 10))*3] = color;
		graph_write_pixel(buffer,(x >> 10), (y >> 10), color);
		
		x += dx;
		y += dy;
	}
}

void graph_draw_buffer(uint8 *buffer, int x,int y, int width, int height, uint8 *data_buf)
{
	/*data_buf保存的是rgb为单位的数据*/
	
	int dbx, dby;	/*data buf x,y*/
	
	uint8 *buf;
	uint32 color;
	
	for(dby = 0; dby < height; dby++){
		for(dbx = 0; dbx < width; dbx++){
			//获取数据指针
			buf = (uint8 *)(data_buf + (dby * width + dbx)*3);
			//做成颜色
			color = RGB(buf[2], buf[1], buf[0]);
			//写入缓冲区
			graph_write_pixel(buffer, x + dbx, y + dby, color);
		}
	}
}


void graph_refresh(uint8 *buffer, int x0, int y0, int x1,int y1)
{
	/*不是当前缓冲区就退出,不刷新到屏幕上*/
	if(current_vidbuf->buffer != buffer){
		return;
	}
	int x, y;
	uint32 color;
	
	if(x0 < 0){x0 = 0;}
	if(y0 < 0){y0 = 0;}
	if(x1 > video_info.width){x1 = video_info.width;}
	if(y1 > video_info.height){y1 = video_info.height;}
	
	for(y = y0; y < y1; y++){	//high*2才能写完全部，不然只有一半
		for(x = x0; x < x1; x++){
			graph_read_pixel(buffer, x, y, &color);
			vram_write_pixel(x, y, color);
		}
	}
}

void switch_video_buffer(struct video_buffer *vidbuf)
{
	//如果相同就直接退出
	if(current_vidbuf == vidbuf){
		return;
	}
	
	/*先把原来buf在屏幕上去除*/
	video_clean_screen();
	
	/*切换缓冲区*/
	current_vidbuf = vidbuf;
	//显示到屏幕上
	graph_refresh(current_vidbuf->buffer, 0, 0, video_info.width,video_info.height);
}

int sys_init_graphic(void )
{
	struct thread *cur = thread_current();
	
	cur->vidbuf = alloc_vidbuf();
	if(cur->vidbuf == NULL){
		printk("no vidbuf left, please close some window.\n");
		return -1;
	}
	cur->vidbuf->thread = cur;
	//thread_graph = cur;
	/*切换到图形*/
	switch_video_buffer(cur->vidbuf);
	//将缓图形冲区清空
	graph_draw_rect(cur->vidbuf->buffer, 0, 0, video_info.width, video_info.height, COLOR_BLACK);
	return 0;
}

void sys_graphic_exit(void )
{
	struct thread *cur = thread_current();
	//将当前进程的vidbuf的清空的buffer
	graph_draw_rect(cur->vidbuf->buffer, 0, 0, video_info.width, video_info.height, COLOR_BLACK);
	//释放
	free_vidbuf(cur->vidbuf);
	//默认切换到console
	switch_video_buffer(vidbuf_console);
}
