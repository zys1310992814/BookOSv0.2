#ifndef _FATXE_H_
#define _FATXE_H_

#include <types.h>
#include <sys/file.h>


struct bpb
{
	uint16_t byte_per_sector;
	uint8_t sector_per_cluster;
	uint16_t reserved_sector;
	uint8_t fats;
	uint16_t root_entries;
	uint16_t sectors;
	uint8_t media_descriptor;
	uint16_t fat_sectors;
	uint16_t sectors_per_track;
	uint16_t number_of_head;
	uint32_t hidden_sector;
	uint32_t totla_sector;
}__attribute__ ((packed));

struct ext_bpb
{
	uint8_t physical_drive_number;
	uint8_t reserved;
	uint8_t extended_boot_signature;
	uint32_t volume_serial_number;
	int8_t volume_lable[11];
	int8_t system_ID[8];
}__attribute__ ((packed));
/*fsinfo is at 1 sector*/
struct fsinfo
{
	uint32_t extended_flags;
	uint8_t reserved[480];
	uint32_t fsinfo_signature;
	uint32_t free_cluster_number;
	uint32_t next_available_cluster;
	
	uint8_t unuserd[14];
	uint16_t magic_flags;
}__attribute__ ((packed));

/*
*Dos Boot Record struct
*/
struct dbr
{
	uint8_t	jump[3]; 	//jmp 
	int8_t	oem_name[8];	//OEM name and version
	struct bpb bpb;			//BIOS parameter block
	struct ext_bpb ext_bpb;	//Bootsector Extension
	uint8_t	boot_code[448];	//pad 
	uint16_t	magic_flags;	//0xaa55
}__attribute__ ((packed));

#define MAX_PATH_LEN 128

#define MAX_FILE_OPEN 32

/*global value*/
extern uint32_t fat_start_lba;
extern uint32_t fat_sectors;
extern uint32_t data_start_lba;
extern uint32_t root_dir_start_lba;

extern uint32_t sector_per_cluster;

extern struct dbr dbr;
extern struct fsinfo fsinfo;

extern struct dir_entry root_dir;

extern char *sync_buffer;
extern struct file file_table[];

void (*block_read)(uint32 lba, void *buf, uint32 counts);
void (*block_write)(uint32 lba, void *buf, uint32 counts);

void set_device(char *name);
void write_bin();

void sync_fsinfo();

/*fatxe.c*/
void init_fs(char *device, uint32 blocks);
void fs_format(uint32 blocks);
int path_to_name(const char *pathname, char *name_buf);
void open_root_dir();
char* path_parse(char* pathname, char* name_store);
int32_t sys_open(const char *pathname,uint8_t flags);
int32_t sys_close(int32_t fd);
int32_t sys_unlink(const char* pathname);
int32_t sys_write(int32_t fd, void* buf, uint32_t count);
int32_t sys_read(int32_t fd, void* buf, uint32_t count);
int32_t sys_lseek(int32_t fd, int32_t offset, uint8_t whence);
struct dir* sys_opendir(const char* name);
struct dir_entry* sys_readdir(struct dir* dir);
void sys_rewinddir(struct dir* dir);
void sys_close_dir(struct dir* dir);
int32_t sys_chdir(const char* path);
int32_t sys_stat(char* path, struct stat* buf);

void sys_ls(char *path, int detail);

int32_t sys_getcwd(char* buf, uint32_t size);

int32_t sys_fgetch(int32_t fd);
int32_t sys_fputch(int32_t ch,int32_t fd);

int32_t sys_rename(const char *pathname, char *new_name);
int32_t sys_move(char* oldpath, char* newpath);
int32_t sys_copy(char* src_path, char* dst_path);

int32_t is_file_exist(const char* path);
int32_t is_file_execute(const char* path);

int sys_access(const char *pathname, int mode);


#endif
