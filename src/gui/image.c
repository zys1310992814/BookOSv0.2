/*
File:		gui/image.c
Contains:	bitmap image for gui
Auther:		Hu Zicheng
Time:		2019/2/13
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/image.h>
#include <sys/video.h>
#include <sys/logo.h>
#include <string.h>
#include <sys/const.h>

#define LOGO_WIDTH 104
#define LOGO_HEIGHT 27

void display_start_logo()
{
	uint8 r,g,b;
	int x, y;
	int offx = video_info.width/2 - LOGO_WIDTH/2, offy = video_info.height/2 - LOGO_HEIGHT/2;
	uint8 *p = (uint8 *)logo_image_data;
	for(y = 0; y < LOGO_HEIGHT; y++){
		for(x = 0; x < LOGO_WIDTH; x++){
			b = *p++;
			g = *p++;
			r = *p++;
			vram_write_pixel(offx+x,offy + y,RGB(r,g,b));
		}
	}
	
	char *str = OS_NAME;
	offx = video_info.width/2 - strlen(str)*8/2;
	offy = video_info.height/2 + 100;
	vram_draw_string(offx, offy, str, COLOR_WHITE);
	
	str = OS_VERSION;
	offx = video_info.width/2 - strlen(str)*8/2;
	offy += 20;
	vram_draw_string(offx, offy, str, COLOR_WHITE);
	
	str = OS_COPYRIGHT;
	offx = video_info.width/2 - strlen(str)*8/2;
	offy += 20;
	vram_draw_string(offx, offy, str, COLOR_WHITE);
	
	
}
