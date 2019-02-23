#ifndef _DEVICE_VIDEO_H_
#define _DEVICE_VIDEO_H_

#include <types.h>
#include <stdint.h>
#include <sys/config.h>
#include <sys/color.h>

#define VIDEO_INFO_ADDR 0x800 

/*
	video_info_INFO_ADDR = vodeo color 
	video_info_INFO_ADDR+2 = vodeo wide
	video_info_INFO_ADDR+4 = vodeo high
	video_info_INFO_ADDR+6 = vodeo ram
*/
#define VIDEO_VRAM_ADDR 0x400000

struct video_info {
	uint16_t bits_per_pixe;		//2字节
	uint32_t width, height;	//2字节+2字节
	uint8_t *vram;				//指针
	uint8 pixel_width;		//2字节
};

extern struct video_info video_info;

void init_video(void);

/*写到gui缓冲区*/

void (*vram_write_pixel)(int32 x, int32 y, uint32 color);
void (*vram_read_pixel)(int32 x, int32 y, uint32 *color);

void vram_write_pixel_24bits(int32 x, int32 y, uint32 color);
void vram_write_pixel_16bits(int32 x, int32 y,uint32 color);

void vram_read_pixel_16bits(int32 x, int32 y, uint32 *color);
void vram_read_pixel_24bits(int32 x, int32 y, uint32 *color);


void vram_draw_word_bits(int x, int y , uint8 *ascii, uint32 color);
void vram_draw_word(int x, int y, char ch, uint32 color);
void vram_draw_string(int x, int y, char *s, uint32 color);

void video_clean_screen();
void sys_get_screen(int *width, int *height);

#endif

