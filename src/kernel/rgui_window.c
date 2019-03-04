/*
File:		kernel/rgui_window.c
Contains:	window.
Auther:		Dorbmon
Time:		2019/3/4
Details:    RGUI hasn't been finished.

Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		1263592223@qq.com
*/
#include <sys/rgui.h>
#include <sys/memory.h>
#include <sys/video.h>
#include <stdio.h>
struct Vector2 default_window_size;
struct gui_window *window_list[MAX_WINDOW_NUMBER];
int window_number = 0;
void init_window(int window_id);
int sys_new_window(){  //返回窗口句柄
    if(window_number == MAX_WINDOW_NUMBER){
        return 0;
    }
    if(if_sheet_full()){
        return -1;  //图层满了，无法创建窗口
    }
    //Start initing window.
    window_list[window_number] = (struct gui_window*)kmalloc(sizeof(struct gui_window));
    window_list[window_number]->window_title = "new window";
    window_list[window_number]->window_size = default_window_size;
    window_list[window_number]->showable = true;
    window_list[window_number]->sheet = new_sheet();
    window_list[window_number]->sheet->used = true;
    window_list[window_number]->window_position.x = 10;
    window_list[window_number]->window_position.y = 10;
    add_sheet(window_list[window_number]->sheet);
    init_window(window_number);
    window_number ++;
    return (window_number - 1);
}
void init_window_system(){
    default_window_size.x = 100;
    default_window_size.y = 100;
    return;
}
void init_window(int window_id){ //初始化一个窗口 绘制窗口边框
    if(window_list[window_id] == NULL){ //窗口不存在则返回
        return;
    }
    struct gui_window* target_window = window_list[window_id];
    int temp_x,temp_y;
    for(temp_x = 0;temp_x < target_window->window_size.x && (temp_x + target_window->window_position.x) < video_info.width;temp_x ++){
        for(temp_y = 0;temp_y < target_window->window_size.y && (temp_y + target_window->window_position.y) < video_info.height;temp_y ++){
            target_window->sheet->sheet_data[target_window->window_position.x + temp_x][target_window->window_position.y + temp_y].used = true;
            target_window->sheet->sheet_data[target_window->window_position.x + temp_x][target_window->window_position.y + temp_y].color = COLOR_RED;
        }
    }
    sys_redraw_rect(target_window->window_position.x,target_window->window_position.y,target_window->window_size.x,target_window->window_size.y);
    return;
}
void resize_window(int window_id,int width,int height){
    //先清除之前的窗口
    if(window_list[window_id] == NULL){ //窗口不存在则返回
        return;
    }
    struct gui_window* target_window = window_list[window_id];
    int temp_x,temp_y;
    for(temp_x = 0;temp_x < target_window->window_size.x && (temp_x + target_window->window_position.x) < video_info.width;temp_x ++){
        for(temp_y = 0;temp_y < target_window->window_size.y && (temp_y + target_window->window_position.y) < video_info.height;temp_y ++){
            target_window->sheet->sheet_data[target_window->window_position.x + temp_x][target_window->window_position.y + temp_y].used = false;
        }
    }
    //重新绘制之前的部分
    sys_redraw_rect(target_window->window_position.x,target_window->window_position.y,target_window->window_size.x,target_window->window_size.y);
    target_window->window_size.x = width;
    target_window->window_size.y = height;
    init_window(window_id);
    return;
}
inline void write_pixel_window(int window_id,int x,int y,int color){    //相对于窗口的左上角进行写入
    if(window_list[window_id] == NULL){ //窗口不存在则返回
        return;
    }
    struct Vector2 window_position = window_list[window_id]->window_position;
    if(x + window_position.x > video_info.width || y + window_position.y > video_info.height){
        return;
    }
    window_list[window_id]->sheet->sheet_data[x + window_position.x][y + window_position.y].color = color;
    window_list[window_id]->sheet->sheet_data[x + window_position.x][y + window_position.y].used = true;
    return;
}
void move_window(int window_id,int x,int y){
   //先清除之前的窗口
    if(window_list[window_id] == NULL){ //窗口不存在则返回
        return;
    }
    struct gui_window* target_window = window_list[window_id];
    int temp_x,temp_y;
    for(temp_x = 0;temp_x < target_window->window_size.x && (temp_x + target_window->window_position.x) < video_info.width;temp_x ++){
        for(temp_y = 0;temp_y < target_window->window_size.y && (temp_y + target_window->window_position.y) < video_info.height;temp_y ++){
            target_window->sheet->sheet_data[target_window->window_position.x + temp_x][target_window->window_position.y + temp_y].used = false;
        }
    }
    //重新绘制之前的部分
    sys_redraw_rect(target_window->window_position.x,target_window->window_position.y,target_window->window_size.x,target_window->window_size.y);
    target_window->window_position.x = x;
    target_window->window_position.y = y;
    init_window(window_id);
    return;
}