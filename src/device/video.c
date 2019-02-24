/*
File:		device/video.c
Contains:	driver for video
Auther:		Hu Zicheng
Time:		2019/1/29
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/config.h>
#include <sys/video.h>
#include <sys/console.h>
#include <string.h>
#include <sys/memory.h>
#include <sys/font.h>
#include <sys/image.h>

struct video_info video_info;

void init_video()
{
	//uint32_t *vram_addr;
	//直接从地址获取
	video_info.bits_per_pixe = *((uint16_t *)VIDEO_INFO_ADDR);
	video_info.width = (uint32_t )(*((uint16_t *)(VIDEO_INFO_ADDR+2)));
	video_info.height = (uint32_t )(*((uint16_t *)(VIDEO_INFO_ADDR+4)));
	
	video_info.pixel_width = video_info.bits_per_pixe/8;
	
	//先获取地址，在转换成指针
	//vram_addr = (uint32_t *)(VIDEO_INFO_ADDR+6);
	video_info.vram = (uint8_t *) VIDEO_VRAM_ADDR;
	
	if(video_info.bits_per_pixe == 16){
		
		vram_write_pixel = vram_write_pixel_16bits;
		vram_read_pixel = vram_read_pixel_16bits;
		
	}else if(video_info.bits_per_pixe == 24){
		
		vram_write_pixel = vram_write_pixel_24bits;
		vram_read_pixel = vram_read_pixel_24bits;
	}
	display_start_logo();
}

void vram_write_pixel_24bits(int32 x, int32 y, uint32 color)
{
	if(x < 0 || y < 0 || x >= video_info.width || y >= video_info.height){
		return;
	}
	
	uint8 *vram = (uint8 *)(video_info.vram + ((y*video_info.width+x))*video_info.pixel_width);
	
	#ifdef _DEVICE_VIDEO_
		*vram++ = color&0xff;
		*vram++ = (color>>8)&0xff;
		*vram = (color>>16)&0xff;
	#endif
}

void vram_write_pixel_16bits(int32 x, int32 y,uint32 color)
{
	if(x < 0 || y < 0 || x >= video_info.width || y >= video_info.height){
		return;
	}
	uint8 *vram = (uint8 *)(video_info.vram + ((y*video_info.width+x))*video_info.pixel_width);
	
	#ifdef _DEVICE_VIDEO_
		*vram++ = color&0xff;
		*vram = (color>>8)&0xff;
	#endif
}


void vram_read_pixel_16bits(int32 x, int32 y, uint32 *color)
{
	
	if(x < 0 || y < 0 || x >= video_info.width || y >= video_info.height){
		return;
	}
	uint8 *vram = (uint8 *)(video_info.vram + ((y*video_info.width+x))*video_info.pixel_width);
	
	uint8 buf[4];
	buf[0] = *vram++;
	buf[1] = *vram;
	buf[2] = 0x00;
	buf[3] = 0x00;
	
	*color = *((uint32 *)buf);
}

void vram_read_pixel_24bits(int32 x, int32 y, uint32 *color)
{
	if(x < 0 || y < 0 || x >= video_info.width || y >= video_info.height){
		return;
	}
	
	uint8 *vram = (uint8 *)(video_info.vram + ((y*video_info.width+x))*video_info.pixel_width);
	
	uint8 buf[4];
	buf[0] = *vram++;
	buf[1] = *vram++;
	buf[2] = *vram;
	buf[3] = 0x00;
	
	*color = *((uint32 *)buf);
}


/*these are used for start logo*/
void vram_draw_word_bits(int x, int y , uint8 *ascii, uint32 color)
{
	int i;
	char d;
	for (i = 0; i < 16; i++) {
		d = ascii[i];
		if ((d & 0x80) != 0) { vram_write_pixel(x + 0, y + i, color); }
		if ((d & 0x40) != 0) { vram_write_pixel(x + 1, y + i, color); }
		if ((d & 0x20) != 0) { vram_write_pixel(x + 2, y + i, color); }
		if ((d & 0x10) != 0) { vram_write_pixel(x + 3, y + i, color); }
		if ((d & 0x08) != 0) { vram_write_pixel(x + 4, y + i, color); }
		if ((d & 0x04) != 0) { vram_write_pixel(x + 5, y + i, color); }
		if ((d & 0x02) != 0) { vram_write_pixel(x + 6, y + i, color); }
		if ((d & 0x01) != 0) { vram_write_pixel(x + 7, y + i, color); }
	}
}

void vram_draw_word(int x, int y, char ch, uint32 color)
{
	vram_draw_word_bits(x, y, standard_font + ch * 16, color);
}

void vram_draw_string(int x, int y, char *s, uint32 color)
{
	while(*s != 0x00){
		vram_draw_word(x, y, *s, color);
		x += 8;
		s++;
	}
}

void video_clean_screen()
{
	int x, y;
	for(y = 0; y < video_info.height; y++){	//high*2才能写完全部，不然只有一半
		for(x = 0; x < video_info.width; x++){
			vram_write_pixel(x, y, COLOR_BLACK);
		}
	}
}

void sys_get_screen(int *width, int *height)
{
	*width = video_info.width;
	*height = video_info.height;
}