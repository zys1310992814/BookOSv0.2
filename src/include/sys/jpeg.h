#ifndef _GUI_JPEG_H_
#define _GUI_JPEG_H_

#include <types.h>

/* jpeg.c */
struct DLL_STRPICENV {	/* 64KB */
	int *work;
};

struct jpeg_RGB {
	unsigned char b, g, r, t;
};

struct jpeg_RGB *jpeg_load_file(char *name, int *info, struct jpeg_RGB *picbuf);

#endif //_JPEG_H_