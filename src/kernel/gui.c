/*
File:		kernel/gui.c
Contains:	window and something about gui.
Auther:		Dorbmon
Time:		2019/2/24
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		1263592223@qq.com
*/
#include <include/gui.h>
#include <stdint.h>
#include <sys/memory.h>
struct gui_window *window_list[MAX_WINDOW_NUMBER];
uint32_t window_number = 0;
struct Vector2 default_window_size;
void sys_init_gui_system(){
    default_window_size.x = 100;
    default_window_size.y = 100;
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
void sys_redraw(){  //立即刷新屏幕
    
}