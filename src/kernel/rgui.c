/*
File:		kernel/gui.c
Contains:	window and something about gui.
Auther:		Dorbmon
Time:		2019/2/24
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		1263592223@qq.com
*/
#include <sys/rgui.h>
#include <stdint.h>
#include <sys/memory.h>
#include <sys/video.h>
struct gui_window *window_list[MAX_WINDOW_NUMBER];
struct sheet *system_sheets[MAX_SHEET_NUMBER];
uint32_t window_number = 0;
uint32_t sheet_number = 0;
struct Vector2 default_window_size;
void sys_init_gui_system(){
    default_window_size.x = 100;
    default_window_size.y = 100;
    //初始化一个图层
    system_sheets[0] = (struct sheet*)kmalloc(sizeof(struct sheet));
    //开始对图层进行初始化 为图层数据申请内存
    system_sheets[0]->used = true;
    system_sheets[0]->sheet_data = (pixel**)kmalloc(sizeof(pixel*) * video_info.width);
    uint32_t x = 0;
    for(;x < video_info.width;x ++){
        system_sheets[0]->sheet_data[x] = (pixel*)kmalloc(sizeof(pixel) * video_info.height);
    }
    sys_redraw();
    return;
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
void sys_redraw(){  //立即刷新屏幕 完全刷新效率低，慎用！！！
    //从下而上绘制像素
    uint32_t now = sheet_number;
    for(;now > 0;now --){
        sys_fulldraw_sheet(system_sheets[now]);
    }
}
void sys_fulldraw_sheet(struct sheet* sheet){
    //开始绘制
    uint32_t x = 0;
    uint32_t y = 0;
    for(;x < video_info.width;x ++){
        for(;y < video_info.height;y ++){
            vram_write_pixel_24bits(x,y,sheet->sheet_data[x][y].color);
        }
    }
    return;
}