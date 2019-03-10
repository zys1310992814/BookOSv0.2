#ifndef _GUI_SYSTEM_
#define _GUI_SYSTEM_
#define MAX_WINDOW_NUMBER 100   //最大窗口数
#define MAX_SHEET_NUMBER 100    //最大渲染队列长度
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
#define FPS 60  //每秒渲染60次
#define MAX_WINDOW_EVENT_LISTNER 50 //窗口最多能有50个监听器


struct Vector2 {
    int x;
    int y;
};
typedef struct{
    bool used;
    int color;
} pixel;  //储存像素颜色
struct sheet {  //图层
    bool used;
    pixel **sheet_data; //二维数组
    struct gui_window* window;
    int window_id;
};
typedef void(*window_event_listener)(int key,int *datas);   //第二个参数其实是一个数组，表达数据。因为事件不同，数据长度也不同 相关设置在RGUI_DATA里有描述

struct gui_window {
    char* window_title;
    struct Vector2 window_size;
    struct Vector2 window_position; 
    bool showable;
    struct sheet* sheet;
    window_event_listener window_event_listeners[MAX_WINDOW_EVENT_LISTNER];
};
enum rand_type{
    rect,point      
};

struct buffer_pixel {
    bool used;
    int color;
    struct sheet* belong;  //图层来源。
};
struct buffer_sheet {   //缓冲图层，也是最后渲染的总图层。
    struct buffer_pixel **buffer_pixel; //二维数组
};
void add_point_to_rand_list(int x,int y);
void init_gui_system();
int sys_new_window();
void sys_redraw();
void swap_sheet(int a_id,int b_id);
void delete_sheet(int sheet_id);
void copy_sheet(struct sheet* source,struct sheet* target);
void sys_fulldraw_sheet(struct sheet* sheet,struct buffer_sheet* target_sheet);
void sys_clean_view();
void sys_redraw_rect(int x,int y,int width,int height);
void rand_buffer_rect(int x,int y,int width,int height);
void init_window_system();
void resize_window(int window_id,int width,int height);
void call_window_event_listeners(int window_id,int event_id,int *datas);
struct sheet* new_sheet();
int add_sheet(struct sheet* sheet);
void fouce_write(char* data);
bool if_sheet_full();
typedef void(*keyborad_linstener)(int key);
typedef void(*mouse_linstener)(int code);
void move_window(int window_id,int x,int y);
bool bind_window(int window_id,window_event_listener listener);
void fouce_write(char* data);
struct sheet* get_window_sheet(int window_id);
void rgui_draw_rect(struct sheet* sheet,int x,int y,int width,int height,int color);
#endif