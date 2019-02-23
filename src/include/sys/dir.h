#ifndef _FATXE_DIR_H_
#define _FATXE_DIR_H_

#include <types.h>

#define ATTR_UNKNOWN 0x00	//Nrmal file
#define ATTR_READONLY 0x01	//File is readonly
#define ATTR_WRITEONLY 0x02	//File is writeonly
#define ATTR_RDWR 0x04	//File is a system file
#define ATTR_ARCHIVE 0x08	//Entry is a directory name
#define ATTR_DIRECTORY 0x10	//File is new or modified
#define ATTR_HIDDEN 0x20	//File is new or modified
#define ATTR_SYSTEM 0x40	//File is new or modified
#define ATTR_NORMAL 0X80	//File is new or modified

#define DE_NAME_LEN 11	//File is new or modified
#define MAX_FILE_NAME_LEN DE_NAME_LEN

/**
日期和时间的转换
*/

#define TIME_TO_DATA16(hou, min, sec) ((unsigned short)(((hou&0x1f)<<11)|((min&0x3f)<<5)|((sec/2)&0x1f)))

#define DATA16_TO_TIME_HOU(data) ((unsigned int)((data>>11)&0x1f))
#define DATA16_TO_TIME_MIN(data) ((unsigned int)((data>>5)&0x3f))
#define DATA16_TO_TIME_SEC(data) ((unsigned int)((data&0x1f) *2))

#define DATE_TO_DATA16(yea, mon, day) ((unsigned short)((((yea-1980)&0x7f)<<9)|((mon&0xf)<<5)|(day&0x1f)))

#define DATA16_TO_DATE_YEA(data) ((unsigned int)(((data>>9)&0x7f)+1980))
#define DATA16_TO_DATE_MON(data) ((unsigned int)((data>>5)&0xf))
#define DATA16_TO_DATE_DAY(data) ((unsigned int)(data&0x1f))

/* dir entry */
struct dir_entry
{
	int8_t name[DE_NAME_LEN];
	uint8_t attributes;
	uint8_t reserved;
	uint8_t create_hundreadth;
	uint16_t create_time;
	uint16_t create_date;
	uint16_t access_date;
	uint16_t high_cluster;
	uint16_t modify_time;
	uint16_t modify_date;
	uint16_t low_cluster;
	uint32_t size;
}__attribute__ ((packed));

struct dir 
{
   struct dir_entry* dir_ptr;
   uint32_t dir_pos;
   uint8_t dir_buf[512];
}__attribute__ ((packed));

/*dir.c*/
void init_directory();

int32_t sys_mkdir(const char *pathname);
int32_t sys_rmdir(const char *pathname);

bool search_dir_entry(struct dir_entry *parent_dir,char *name, struct dir_entry *dir);
void create_dir_entry(struct dir_entry *dir, char *name, uint8_t attributes);
bool sync_dir_entry(struct dir_entry* parent_dir, struct dir_entry* dir, void* io_buf);
bool empty_dir_entry(struct dir_entry* parent_dir, struct dir_entry* dir, void* io_buf);
void copy_dir_entry(struct dir_entry *det, struct dir_entry *src);
void release_dir_entry(struct dir_entry *de, char *buffer);
void close_dir_entry(struct dir_entry *de);
bool load_dir_entry(struct dir_entry *parent_dir, char *name, struct dir_entry *child_dir, void* io_buf);

#endif

