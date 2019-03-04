#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <sys/system.h>
#include <sys/pipe.h>

void *malloc(int size);
void free(void *ptr);

void get_memory(int *size, int *free);

int execv(char *path, char *argv[]);

void exit(int status);
int _wait(int *status);

void clear();
void ps();

void reboot(int reboot_type);

int rand();
void srand(unsigned int seed);

uint64_t new_pipe(uint32_t size);
bool write_pipe(uint64_t pipe_id,void* data,uint32_t size);
bool read_pipe(uint64_t pipe_id,void* buffer);
bool close_pipe(uint64_t pipe_id);

int get_pid();
int get_ticks();
void sleep(u32 msec);

#endif
