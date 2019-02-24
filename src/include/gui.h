#ifndef _GUI_SYSTEM_
#define _GUI_SYSTEM_
#define MAX_WINDOW_NUMBER 100   //最大窗口数
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
#endif