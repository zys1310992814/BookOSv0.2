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
#include <sys/color.h>
#include <sys/font.h>
#include <sys/thread.h>
#include <sys/keyboard.h>

struct gui_window *window_list[MAX_WINDOW_NUMBER];
struct sheet *system_sheets[MAX_SHEET_NUMBER];
uint32_t window_number = 0;
uint32_t sheet_number = 0;
struct Vector2 default_window_size;

//只在本文件调用的函数要提前在这儿声明才能调用
void sys_set_only_color_sheet(struct sheet* sheet,uint32_t color);
void keyborad_listen();

//这里是一些关于监听的绑定
keyborad_linstener top_keyboard_reader;
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
    return;
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
            top_keyboard_reader(key);
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
    for(;now > 0;now --){
        sys_fulldraw_sheet(system_sheets[now - 1]);
    }
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
void sys_fulldraw_sheet(struct sheet* sheet){
    //开始绘制
    int x,y;
    /*for(y = 0; y < video_info.height; y++){	//high*2才能写完全部，不然只有一半
		for(x = 0; x < video_info.width; x++){
			vram_write_pixel(x, y, COLOR_BLUE);
		}
	}*/
    for(x = 0;x < video_info.width;x ++){
        for(y = 0;y < video_info.height;y ++){
            //vram_write_pixel_24bits(x,y,sheet->sheet_data[x][y].color);
            if(sheet->sheet_data[x][y].used){
                vram_write_pixel_24bits(x,y,sheet->sheet_data[x][y].color);
            }
            //vram_write_pixel(x,y,COLOR_RED);
            //vram_write_pixel_24bits(x,y,0xFFFFF);
        }
    }
    //vram_draw_string(video_info.width / 2,video_info.height / 2,"Hello World",COLOR_BLACK);
    return;
}