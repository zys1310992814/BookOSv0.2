#ifndef _GUI_SYSTEM_
#define _GUI_SYSTEM_
#define MAX_WINDOW_NUMBER 100   //最大窗口数
#define MAX_SHEET_NUMBER 256    //最大图层数
#include <stdint.h>
#include <types.h>

#define LISTENER_GROUP_MAX 10   //鼠标和键盘的监听者数总数最多为20


#define NOMAL 0
#define RIGHT_MOUSE_UP 1
#define RIGHT_MOUSE_DOWN 2
#define LEFT_MOUSE_UP 1
#define LEFT_MOUSE_DOWN 2
#define MIDDLE_MOUSE_UP 1
#define MIDDLE_MOUSE_DOWN 2




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
struct buffer_pixel {
    bool used;
    uint32_t color;
    struct sheet* belong;  //图层来源。
};
struct buffer_sheet {   //缓冲图层，也是最后渲染的总图层。
    pixel **buffer_pixel; //二维数组
};
void sys_init_gui_system();
uint32_t sys_new_window();
void sys_redraw();
void sys_fulldraw_sheet(struct sheet* sheet,struct sheet target_sheet);
void sys_clean_view();
typedef void(*keyborad_linstener)(int key);
typedef void(*mouse_linstener)(int code);
#endif