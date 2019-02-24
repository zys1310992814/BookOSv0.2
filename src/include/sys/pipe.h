#ifndef _LIB_PIPE_
#define _LIB_PIPE_

#include <stdint.h>
#include <sys/memory.h>
#include <sys/sync.h>
#include <string.h>
void init_pipe();
uint64_t sys_new_pipe(uint32_t size);
bool sys_write_pipe(uint64_t pipe_id,void* data,uint32_t size);
bool sys_read_pipe(uint64_t pipe_id,void* buffer);
bool sys_close_pipe(uint64_t pipe_id);

#endif