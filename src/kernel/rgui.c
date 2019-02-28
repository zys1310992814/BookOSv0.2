/*
File:		kernel/gui.c
Contains:	window and something about gui.
Auther:		Dorbmon
Time:		2019/2/24
Details:    RGUI hasn't been finished.

Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		1263592223@qq.com
*/
#include <sys/rgui.h>
#include <stdint.h>
#include <sys/memory.h>
#include <sys/video.h>
#include <sys/color.h>
#include <sys/font.h>
#include <sys/thread.h>
#include <sys/keyboard.h>
#include <sys/mouse.h>
void sys_set_only_color_sheet(struct sheet* sheet,uint32_t color);
void keyborad_listen();
struct gui_window *window_list[MAX_WINDOW_NUMBER];
struct sheet *system_sheets[MAX_SHEET_NUMBER];
struct sheet topest_system_sheet;   //提供给系统，始终在最高层，主要用来实现鼠标等功能
struct buffer_sheet* result_sheet;  //最终渲染的图层
uint32_t window_number = 0;
uint32_t sheet_number = 0;
struct Vector2 default_window_size;
//只在本文件调用的函数要提前在这儿声明才能调用
void sys_set_only_color_sheet(struct sheet* sheet,uint32_t color);
void keyborad_listen();
void mouse_listen();
void call_mouse_listeners(int flag);
void call_keyboard_listeners(int flag);
//这里是一些关于监听的绑定
keyborad_linstener top_keyboard_reader[LISTENER_GROUP_MAX];
mouse_linstener top_mouse_reader[LISTENER_GROUP_MAX];
void sys_init_gui_system(){
    init_font();
    default_window_size.x = 100;
    default_window_size.y = 100;
    //初始化一个图层
    system_sheets[0] = (struct sheet*)kmalloc(sizeof(struct sheet));
    //开始对图层进行初始化 为图层数据申请内存
    sheet_number ++;
    system_sheets[0]->used = true;
    system_sheets[0]->sheet_data = (pixel**)kmalloc(sizeof(pixel*) * video_info.width);
    
    uint32_t x = 0;
    
    for(;x < video_info.width;x ++){
        system_sheets[0]->sheet_data[x] = (pixel*)kmalloc(sizeof(pixel) * video_info.height);
    }
    sys_set_only_color_sheet(system_sheets[0],COLOR_GREEN);
    sys_redraw();
    vram_draw_string(video_info.width / 2,video_info.height / 2,"Hello World! initing....",COLOR_BLACK);

    //创建键盘监听线程
    thread_start("rgui_keyborad_listen",2,keyborad_listen,NULL);
    thread_start("rgui_mouse_listen",2,mouse_listen,NULL);
    return;
}
void mouse_listen(){
    bool right_was_down = false;
    bool left_was_down = false;
    bool middle_was_down = false;
    char mouse_data[3];
    for(;;){
        if(top_mouse_reader == NULL){
            continue;
        }
        sys_get_mouse_button(mouse_data);
        if(mouse_data[2] == MOUSE_UP){
            if(right_was_down == true){   //曾经按下，则为松开事件
                //top_mouse_reader(RIGHT_MOUSE_UP);
                call_mouse_listeners(RIGHT_MOUSE_UP);
            }
        }
        if(mouse_data[2] == MOUSE_DOWN && right_was_down == false){
            call_mouse_listeners(RIGHT_MOUSE_DOWN);
        }
        if(mouse_data[1] == MOUSE_UP){
            if(middle_was_down == true){   //曾经按下，则为松开事件
                call_mouse_listeners(MIDDLE_MOUSE_UP);
            }
        }
        if(mouse_data[1] == MOUSE_DOWN && middle_was_down == false){
            call_mouse_listeners(MIDDLE_MOUSE_DOWN);
        }
        if(mouse_data[0] == MOUSE_UP){
            if(left_was_down == true){   //曾经按下，则为松开事件
                call_mouse_listeners(LEFT_MOUSE_UP);
            }
        }
        if(mouse_data[0] == MOUSE_DOWN && left_was_down == false){
            call_mouse_listeners(LEFT_MOUSE_DOWN);
        }
    }
    
}
void keyborad_listen(){
    int key = 0;
    for(;;){
        key = sys_get_key();
        if(key == -1){
            continue;
        }
        //如果不为空，则调用用户的监听函数
        if(top_keyboard_reader != NULL){
            call_keyboard_listeners(key);
        }
        continue;
    }
}
uint32_t sys_new_window(){  //返回窗口句柄
    if(window_number == MAX_WINDOW_NUMBER){
        return 0;
    }
    //Start initing window.
    window_list[window_number] = (struct gui_window*)kmalloc(sizeof(struct gui_window));
    window_list[window_number]->window_title = "new window";
    window_list[window_number]->window_size = default_window_size;
    window_list[window_number]->showable = true;
    window_number ++;
    return (window_number - 1);
}
void sys_redraw(){  //立即刷新屏幕 完全刷新效率低，慎用。
    //从下而上绘制像素
    uint32_t now = sheet_number;
    //video_clean_screen();
    struct buffer_sheet buffer_sheet;
    //开始初始化图层
    buffer_sheet.buffer_pixel =  (struct buffer_pixel**)kmalloc(sizeof(struct buffer_pixel*) * video_info.width);
    int x;
    for(x = 0;x < video_info.width;x ++){
        buffer_sheet.buffer_pixel[x] = (struct buffer_pixel*)kmalloc(sizeof(struct buffer_pixel) * video_info.height);
    }
    for(;now > 0;now --){
        sys_fulldraw_sheet(system_sheets[now - 1],&buffer_sheet);
    }
    result_sheet = &buffer_sheet;
    //开始渲染到屏幕上
    int y;
    for(x = 0;x < video_info.width;x ++){
        for(y = 0;y < video_info.height;y ++){
            vram_write_pixel_24bits(x,y,result_sheet->buffer_pixel[x][y].color);
        }
    }
    return;
}
void sys_clean_view(){
    uint32_t x = 0;
    uint32_t y = 0;
    for(x = 0;x < video_info.width;x ++){
        for(y = 0;y < video_info.height;y ++){
            //vram_write_pixel_24bits(x,y,sheet->sheet_data[x][y].color);
            vram_write_pixel(x,y,COLOR_BLUE);
            //vram_write_pixel_24bits(x,y,0xFFFFF);
        }
    }
}
void sys_set_only_color_sheet(struct sheet* sheet,uint32_t color){
    int x,y;
    for(x = 0;x < video_info.width;x ++){
        for(y = 0;y < video_info.height;y ++){
            //vram_write_pixel_24bits(x,y,sheet->sheet_data[x][y].color);
            sheet->sheet_data[x][y].used = true;
            sheet->sheet_data[x][y].color = color;
            //vram_write_pixel(x,y,COLOR_RED);
            //vram_write_pixel_24bits(x,y,0xFFFFF);
        }
    }
}
void sys_fulldraw_sheet(struct sheet* sheet,struct buffer_sheet* target_sheet){
    //开始绘制
    int x,y;
    for(x = 0;x < video_info.width;x ++){
        for(y = 0;y < video_info.height;y ++){
            if(sheet->sheet_data[x][y].used){
                target_sheet->buffer_pixel[x][y].used = true;
                target_sheet->buffer_pixel[x][y].color = sheet->sheet_data[x][y].color;
            }
        }
    }
    return;
}
void call_mouse_listeners(int flag){
    int n;
    for(n = 0;n < LISTENER_GROUP_MAX;n ++){
        if(top_mouse_reader[n] == NULL){
            break;
        }
        top_mouse_reader[n](flag);
    }
    return;
}
void call_keyboard_listeners(int flag){
    int n;
    for(n = 0;n < LISTENER_GROUP_MAX;n ++){
        if(top_keyboard_reader[n] == NULL){
            break;
        }
        top_keyboard_reader[n](flag);
    }
    return;
}