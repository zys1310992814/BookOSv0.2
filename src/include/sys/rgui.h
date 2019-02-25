#ifndef _GUI_SYSTEM_
#define _GUI_SYSTEM_
#define MAX_WINDOW_NUMBER 100   //最大窗口数
#define MAX_SHEET_NUMBER 256    //最大图层数
#include <stdint.h>
#include <types.h>
struct Vector2 {
    uint32_t x;
    uint32_t y;
};
struct gui_window {
    char* window_title;
    struct Vector2 window_size;
    struct Vector2 window_position;
    bool showable;
};
typedef struct{
    bool used;
    uint32_t color;
} pixel;  //储存像素颜色
struct sheet {  //图层
    bool used;
    pixel **sheet_data; //二维数组
};
void sys_init_gui_system();
uint32_t sys_new_window();
void sys_redraw();
void sys_fulldraw_sheet(struct sheet* sheet);
void sys_clean_view();
#endif