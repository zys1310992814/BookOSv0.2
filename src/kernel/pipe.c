/*
File:		kernel/pipe.c
Contains:	System Pipe
Auther:		Dorbmon
Time:		2019/2/24
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		1263592223@qq.com
*/
#include <stdint.h>
#include <sys/memory.h>
#include <sys/sync.h>
#include <stdio.h>
#include <sys/pipe.h>

struct pipe_list {
    struct pipe *pipe_first;
    struct pipe *last;
    uint32_t pipe_number;
};

struct pipe {
    uint64_t pipe_id;
    uint32_t data_size;
    void* data_area;
    struct pipe *next;
    struct lock lock;
};
struct pipe_list *pipes;

void init_pipe(){
    pipes = (struct pipe_list*)kmalloc(sizeof(struct pipe_list));
    pipes->pipe_number = 0;
}
uint64_t sys_new_pipe(uint32_t size){   //即时分配
    pipes->pipe_number ++;
    //首先判断表中是否有了管道
    if(pipes->last == 0){
        pipes->pipe_first = kmalloc(sizeof(struct pipe));
        pipes->last = pipes->pipe_first;
    }else{
        pipes->last->next = kmalloc(sizeof(struct pipe));
        pipes->last = pipes->last->next;
    }
    pipes->last->pipe_id = pipes->pipe_number;
    //分配缓存区
    pipes->last->data_size = size;
    pipes->last->data_area = kmalloc(size);
    lock_init(&pipes->last->lock);
    return pipes->pipe_number - 1;
}
bool sys_write_pipe(uint64_t pipe_id,void* data,uint32_t size){
    if(pipes->pipe_number < pipe_id){
        return false;
    }
    //开始遍历，获取管道引用
    struct pipe* target_pipe = pipes->pipe_first;
    uint64_t now = 0;
    for(;now < pipe_id;now ++){
        target_pipe = target_pipe->next;
    }
    //开始拷贝数据
    if(size > target_pipe->data_size){
        return false;
    }
    lock_acquire(&target_pipe->lock);
    memcpy(target_pipe->data_area,data,size);
    lock_release(&target_pipe->lock);
    return true;
}
bool sys_read_pipe(uint64_t pipe_id,void* buffer){
    if(pipes->pipe_number < pipe_id){
        return false;
    }
    //开始遍历，获取管道引用
    struct pipe* target_pipe = pipes->pipe_first;
    uint64_t now = 0;
    for(;now < pipe_id;now ++){
        target_pipe = target_pipe->next;
    }
    //开始拷贝数据
    lock_acquire(&target_pipe->lock);
    memcpy(buffer,target_pipe->data_area,target_pipe->data_size);
    lock_release(&target_pipe->lock);
    return true;
}
bool sys_close_pipe(uint64_t pipe_id){
    if(pipes->pipe_number < pipe_id){
        return false;
    }
    //开始遍历，获取管道引用
    struct pipe* target_pipe_last = pipes->pipe_first;
    uint64_t now = 0;
    for(;now < (pipe_id - 1);now ++){
        target_pipe_last = target_pipe_last->next;
    }
    lock_acquire(&target_pipe_last->lock);
    //开始销毁对象
    if(target_pipe_last == 0){
        kfree(target_pipe_last);
        pipes->pipe_number--;
        return true;
    }
    struct pipe* target_pipe = target_pipe_last->next;
    target_pipe_last->next = target_pipe ->next;
    kfree(target_pipe);
    pipes->pipe_number--;
    return true;
}