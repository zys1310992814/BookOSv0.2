#ifndef _GUI_COLOR_H_
#define _GUI_COLOR_H_

#include <sys/config.h>

#define ARGB16(a,r,g,b) ((a << 16) | ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3))

#define ARGB24(a,r,g,b) ((a<<24) | (r<<16) | (g<<8) | b)

#ifdef _VIDEO_16
	#define ARGB(a,r,g,b) ARGB16(a,r,g,b)
	#define RGB(r,g,b) ARGB16(255,r,g,b)
#elif defined _VIDEO_24
	#define ARGB(a,r,g,b) ARGB24(a,r,g,b)
	#define RGB(r,g,b) ARGB24(255,r,g,b)
#endif

#define COLOR_RED RGB(255,0,0)
#define COLOR_GREEN RGB(0,255,0)
#define COLOR_BLUE RGB(0,0,255)
#define COLOR_WHITE RGB(255,255,255)
#define COLOR_BLACK RGB(0,0,0)
#define COLOR_GRAY RGB(195,195,195)
#define COLOR_LEAD RGB(127,127,127)
#define COLOR_YELLOW RGB(255,255,127)

#endif

