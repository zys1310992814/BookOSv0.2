/*
File:		kernel/rgui_rand_list.c
Contains:	RGUI rand list
Auther:		Dorbmon
Time:		2019/3/4

Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		1263592223@qq.com
*/


/*
十分抱歉，这里还没有想好怎么高效处理，所以先不写了。



*/
#include <sys/rgui.h>
#include <sys/memory.h>
#include <sys/sync.h>
struct rand_node {  //渲染节点
    enum rand_type type;
    struct Vector2 data;
    struct Vector2 position;
    struct rand_node* next;
};


struct rand_list {
    int now_size;   //记录现在的尺寸大小
    struct rand_node* first;
    struct rand_node* last;
    struct rand_node* now;  //指向正在使用的一项
    struct lock rand_lock;
};
struct rand_list rand_list;
void init_rand_list(int length){  //初始化渲染队列
    lock_init(&rand_list.rand_lock);
    lock_acquire(&rand_list.rand_lock);
    if(length != 0){    //需要预先申请
        int range;
        for(range = 0;range < length;range ++){
            if(rand_list.first == NULL){ //列表为空
                rand_list.first = (struct rand_node*)kmalloc(sizeof(struct rand_node));
                rand_list.last = rand_list.first;
                continue;
            }
            //不为空，则在末尾增加
            rand_list.last->next = (struct rand_node*)kmalloc(sizeof(struct rand_node));
            rand_list.last = rand_list.last->next;
        }
        rand_list.now = rand_list.first;
    }
    lock_release(&rand_list.rand_lock);
    return;
}
void insert_rand_list(enum rand_type type,struct Vector2 position,struct Vector2 Data){ //插入到渲染队列底部

}