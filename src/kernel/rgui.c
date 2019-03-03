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
#include <rgui_pixel_data.h>
#include <sys/system.h>
#include <stdio.h>
void sys_set_only_color_sheet(struct sheet* sheet,uint32_t color);
void keyborad_listen();
struct gui_window *window_list[MAX_WINDOW_NUMBER];
struct sheet *system_sheets[MAX_SHEET_NUMBER];
struct sheet *topest_system_sheet;   //提供给系统，始终在最高层，主要用来实现鼠标等功能
struct buffer_sheet result_sheet;  //最终渲染的图层
int window_number = 0;
int sheet_number = 0;
struct Vector2 default_window_size;
//只在本文件调用的函数要提前在这儿声明才能调用
void rand_buffer();
struct sheet* new_sheet();
void sys_set_only_color_sheet(struct sheet* sheet,uint32_t color);
void keyborad_listen();
void mouse_listen();
void mouse_rander();
void call_mouse_listeners(int flag);
void call_keyboard_listeners(int flag);
//这里是一些关于监听的绑定
keyborad_linstener top_keyboard_reader[LISTENER_GROUP_MAX];
mouse_linstener top_mouse_reader[LISTENER_GROUP_MAX];
void sys_init_gui_system(){
    int x;
    printf("video_info :(%d,%d)",video_info.width,video_info.height);
    fouce_write("RGUI is initing");
    default_window_size.x = 100;
    default_window_size.y = 100;
    topest_system_sheet = new_sheet();
    //初始化一个图层
    system_sheets[0] = (struct sheet*)kmalloc(sizeof(struct sheet));
    //开始对图层进行初始化 为图层数据申请内存
    sheet_number ++;
    system_sheets[0] = new_sheet();
    system_sheets[0]->used = true;
    sys_set_only_color_sheet(system_sheets[0],COLOR_BLUE);
    //初始化result_sheet
    result_sheet.buffer_pixel = (struct buffer_pixel**)kmalloc(sizeof(struct buffer_pixel*) * video_info.width);
    for(x = 0;x < video_info.width;x ++){
        result_sheet.buffer_pixel[x] = (struct buffer_pixel*)kmalloc(sizeof(struct buffer_pixel) * video_info.height);
    }
    //vram_draw_string(video_info.width / 2,video_info.height / 2,"Hello World! initing....",COLOR_WHITE);
    //创建键盘监听线程
    thread_start("rgui_keyborad_listen",2,keyborad_listen,NULL);
    thread_start("rgui_mouse_listen",2,mouse_listen,NULL);
    thread_start("mouse_rander",1,mouse_rander,NULL);
    fouce_write("finished starting thread.Start randing...");
    //thread_start("test_rand",0,test,NULL);
    sys_redraw();
    //vram_draw_string(0,0,"Finished binding.",COLOR_WHITE);
    return;
}
struct sheet* new_sheet(){
    int x;
    struct sheet* temp = (struct sheet*)kmalloc(sizeof(struct sheet));
    temp->sheet_data = (pixel**)kmalloc(sizeof(pixel*) * video_info.width);
    for(x = 0;x < video_info.width;x ++){
        temp->sheet_data[x] = (pixel*)kmalloc(sizeof(pixel) * video_info.height);
    }
    return temp;
}
void mouse_rander(){
    //开始渲染鼠标
    int x = 0,y = 0,past_x = 0,past_y = 0;
    for(;;){
        sys_get_mouse_position(&x,&y);
        if(x == past_x && y == past_y){
            continue;
        }
        //销毁之前的鼠标影子
        int temp_x,temp_y;
        for(temp_x = 0;temp_x < MOUSE_SIZE_X && (temp_x + past_x) < video_info.width;temp_x ++){
            for(temp_y = 0;temp_y < MOUSE_SIZE_Y && (temp_y + past_y) < video_info.height;temp_y ++){
                topest_system_sheet->sheet_data[temp_x + past_x][temp_y + past_y].used = false;
                //printf("killed(%d,%d)",temp_x,temp_y);
            }
        }
        sys_redraw_rect(past_x,past_y,MOUSE_SIZE_X,MOUSE_SIZE_Y);
        //开始渲染到对应位置
        for(temp_x = 0;temp_x < MOUSE_SIZE_X && x + temp_x < video_info.width;temp_x ++){
            for(temp_y = 0;temp_y < MOUSE_SIZE_Y && y + temp_y < video_info.height;temp_y ++){
                topest_system_sheet->sheet_data[x + temp_x][y + temp_y].used = true;
                topest_system_sheet->sheet_data[x + temp_x][y + temp_y].color = MOUSE_COLOR;
            }
        }
        sys_redraw_rect(x,y,MOUSE_SIZE_X,MOUSE_SIZE_Y);
        past_x = x;
        past_y = y;
    }
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
void sys_redraw_rect(int x,int y,int width,int height){    //渲染矩形区域
    int range_x,range_y,range_sheet;
    for(range_sheet = MAX_SHEET_NUMBER - 1;range_sheet >= 0;range_sheet --){
        if(system_sheets[range_sheet] == NULL){ //遍历完成.
            continue;
        }
        for(range_x = 0;range_x < width && (x + range_x) < video_info.width;range_x ++){
            for(range_y = 0;range_y < height && (y + range_y) < video_info.height;range_y ++){
                if(system_sheets[range_sheet]->sheet_data[x + range_x][y + range_y].used == true){
                    result_sheet.buffer_pixel[x + range_x][y + range_y].used = true;
                    result_sheet.buffer_pixel[x + range_x][y + range_y].belong = system_sheets[range_sheet];
                    result_sheet.buffer_pixel[x + range_x][y + range_y].color = system_sheets[range_sheet]->sheet_data[x + range_x][y + range_y].color;
                    continue;
                }
            }
        }
    }
    //渲染最顶端图层
    for(range_x = 0;range_x < width && (x + range_x) < video_info.width;range_x ++){
            for(range_y = 0;range_y < height && (y + range_y) < video_info.height;range_y ++){
                if(topest_system_sheet->sheet_data[x + range_x][y + range_y].used == true){
                    result_sheet.buffer_pixel[x + range_x][y + range_y].used = true;
                    result_sheet.buffer_pixel[x + range_x][y + range_y].color = topest_system_sheet->sheet_data[x + range_x][y + range_y].color;
                    continue;
                }
            }
    }
    rand_buffer_rect(x,y,width,height);
    return;
}
void sys_redraw(){  //立即刷新屏幕 完全刷新效率低，慎用。
    int range_sheet;
    for(range_sheet = MAX_SHEET_NUMBER - 1;range_sheet >= 0;range_sheet --){
        if(system_sheets[range_sheet] == NULL){
            continue;
        }
        //开始遍历这个sheet
        sys_fulldraw_sheet(system_sheets[range_sheet],&result_sheet);
    }
    //最后单独绘制强制顶层的图层
    sys_fulldraw_sheet(topest_system_sheet,&result_sheet);
    rand_buffer();
    return;
}
void rand_buffer_rect(int x,int y,int width,int height){
    int temp_x,temp_y;
    for(temp_x = 0;temp_x < width && temp_x + x < video_info.width;temp_x ++){
        for(temp_y = 0;temp_y < video_info.height && temp_y + y < video_info.height;temp_y ++){
            if(result_sheet.buffer_pixel[temp_x + x][temp_y + y].used == true){
                vram_write_pixel_24bits(temp_x + x,temp_y + y,result_sheet.buffer_pixel[temp_x + x][temp_y + y].color);
            }
        }
    }
    return;
}
void rand_buffer(){
    int x,y;
    for(x = 0;x < video_info.width;x ++){
        for(y = 0;y < video_info.height;y ++){
            if(result_sheet.buffer_pixel[x][y].used == true){
                vram_write_pixel_24bits(x,y,result_sheet.buffer_pixel[x][y].color);
            }
        }
    }
    return;
}
void sys_clean_view(){
    int x,y;
    for(x = 0;x < video_info.width;x ++){
        for(y = 0;y < video_info.height;y ++){
            vram_write_pixel(x,y,COLOR_BLUE);
        }
    }
}
void sys_set_only_color_sheet(struct sheet* sheet,uint32_t color){
    int x,y;
    for(x = 0;x < video_info.width;x ++){
        for(y = 0;y < video_info.height;y ++){
            sheet->sheet_data[x][y].used = true;
            sheet->sheet_data[x][y].color = color;
        }
    }
}
void sys_fulldraw_sheet(struct sheet* sheet,struct buffer_sheet* target_sheet){
    //开始绘制
    int x,y;
    for(x = 0;x < video_info.width;x ++){
        for(y = 0;y < video_info.height;y ++){
            if(sheet->sheet_data[x][y].used == true){
               // printf("\n got true(%d,%d)",x,y);
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
void copy_sheet(struct sheet* source,struct sheet* target){ //得保证图层已经初始化，该函数不做初始化。
    int x,y;
    for(x = 0;x < video_info.width;x ++){
        for(y = 0;y < video_info.height;y ++){
            target->sheet_data[x][y].used = source->sheet_data[x][y].used;
            target->sheet_data[x][y].color = source->sheet_data[x][y].color;
        }
    }
    return;
}
#define LINE_LONG 10
int position = 0;
void fouce_write(char* data){ //直接在左上角输出文本
    if(position + LINE_LONG >= video_info.height){
        position = 0;
    }
    vram_draw_string(0,position,data,COLOR_WHITE);
    
    position += LINE_LONG;
}