/*
File:		fs/fatxe.c
Contains:	fatxe file system 
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <string.h>
#include <unistd.h>
#include <sys/harddisk.h>
#include <sys/fatxe.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <sys/fat.h>
#include <sys/vga.h>
#include <sys/console.h>
#include <sys/gui.h>
#include <sys/ramdisk.h>
#include <sys/thread.h>

uint32_t fat_start_lba;
uint32_t fat_sectors;
uint32_t data_start_lba;
uint32_t root_dir_start_lba;

uint32_t sector_per_cluster;

struct dbr dbr;
struct fsinfo fsinfo;

struct dir dir_root;

static void init_dbr(uint32 blocks);
static void init_fsinfo();

void sync_fsinfo();

char *sync_buffer;

void init_fs(char *device, uint32 blocks)
{
	set_device(device);
	/*
	if(!set_current_hd("hd-0")){
		printk("set hd-0 sucess!\n");
	}
*/
	fs_format(blocks);
	
}


void fs_format(uint32 blocks)
{
	//uint8_t *sync_buffer = (uint8_t *)kmalloc(1024);
	init_dbr(blocks);
	init_fsinfo();
	init_fat();
	
	init_directory();
	
	init_file_table();
	
	open_root_dir();
}	

void set_device(char *name)
{
	if(!strcmp(name, "ramdisk")){
		block_read = ramdisk_read;
		block_write = ramdisk_write;
	}else if(!strcmp(name, "harddisk")){
		block_read = hd_read_sectors;
		block_write = hd_write_sectors;
	}
}


/* 更改当前工作目录为绝对路径path,成功则返回0,失败返回-1 */
int32_t sys_chdir(const char* path)
{
	struct thread *cur = thread_current();
	int32_t ret = -1;
   
	struct file_search_record record;  
	memset(&record, 0, sizeof(struct file_search_record));
	int found = search_file(path, &record);
	//printk("sys_chdir: %s is found!\n", path);
	//找到该目录
	if (found == 1) {
		//printk("sys_chdir: %s is found!\n", path);
		if(record.child_dir->attributes&ATTR_DIRECTORY){
			//task_current()->cwd = inode_no;
			strcpy(cur->cwd, path);
			//printk("sys_chdir: task pwd %s \n", task->cwd);
			ret = 0;
		}
	}else {	//没有找到
		printk("chdir: %s isn't exist!\n", path);
	}
	//close_dir_entry(record.parent_dir);
	close_dir_entry(record.child_dir);
	return ret;
}

int sys_getcwd(char* buf, uint32_t size)
{
	struct thread *cur = thread_current();
	memcpy(buf, cur->cwd, size);
	return 0;
}

/*******************************************************************************
*Function name: sys_copy
*Description: Copy a file to other path in kernel state
*Input: src_path(char *) The file we want to copy
*		dst_path(char *) Where we will copy to
*Output: None
*Return: (integer) if 0, we move sucess, if -1, we move faild!
*Auther: Eric hu
*Date: 2018/11/23
*Version: 0.1
*******************************************************************************/
int32_t sys_copy(char* src_path, char* dst_path)
{
	//printk("copy: %s to %s\n", src_path, dst_path);
   //we can't copy '/'
	if (!strcmp(src_path, "/")) {
      printk("copy: can't copy / !\n");
      return -1;
	}
	int32_t ret = -1;	// 默认返回值
	//寻找源路径
	struct file_search_record src_record;
	// 记得初始化或清0,否则栈中信息不知道是什么
	memset(&src_record, 0, sizeof(struct file_search_record));   
	int src_found = search_file(src_path, &src_record);
	//寻找目的路径
	struct file_search_record dst_record;
	// 记得初始化或清0,否则栈中信息不知道是什么
	memset(&dst_record, 0, sizeof(struct file_search_record));
	int dst_found = search_file(dst_path, &dst_record);
	
	// src child ->file eg： /abc  -> abc = （file）
	if(src_found){
		//printk("copy: src file found!\n");
		//printk("parent name:%s \n",src_record.parent_dir->name);
		//关闭父目录，我们不需要
		close_dir_entry(src_record.parent_dir);

		//printk("file name:%s \n",src_record.child_dir->name);
	}else{
		printk("copy: src file not found!\n");
		ret = -1;
		//回滚到第0步
		goto rollback_0;
	}
	//dst child ->dir	eg: /def -> def = （dir）
	if(dst_found){
		//printk("copy: dst file found!\n");

		//如果复制到根目录下
		if(!strcmp(dst_path, "/")){
			//把根目录当作子目录，也就是说，在根目录下面创建文件
			dst_record.child_dir = &root_dir;
		}else{
			//printk("parent dir %s \n", src_record.parent_dir->name);
			//如果不是在根目录下面，肯定有父目录，所以关闭它，因为我们不需要它
			close_dir_entry(src_record.parent_dir);
		}
		//printk("file name:%s \n",dst_record.child_dir->name);
	}else{
		printk("copy: dst file not found!\n");
		ret = -1;
		//回滚到第1步，释放src对应的子目录
		goto rollback_1;
	}
	//对文件属性进行判断
	//源文件如果是目录就返回
	if(src_record.child_dir->attributes&ATTR_DIRECTORY){
		printk("src file can't be directory!\n");
		ret = -1;
		//回滚到第2步，释放src和dst对应的子目录
		goto rollback_2;
	}
	//目的文件如果是一个文件（非目录），就返回，因为不可能把文件复制到一个文件下面
	if(!(dst_record.child_dir->attributes&ATTR_DIRECTORY)){
		printk("dst file must be directory!\n");
		ret = -1;
		//回滚到第2步，释放src和dst对应的子目录
		goto rollback_2;
	}
	/*file in = src file, file_out = dst file
	构建file_in，用于读取对应文件的数据
	file_out用于创建一个文件后，往里面写入数据
	*/
	struct file file_in, *file_out;
	int fd_out;	//获取创建文件的fd
	//create new file
	fd_out =  file_create(dst_record.child_dir, src_record.child_dir->name, src_record.child_dir->attributes);
	
	if(fd_out != -1){
		//创建成功就获取对于的file结构体，复制文件信息成功
		file_out = get_file_though_fd(fd_out);
	}else{
		printk("create file faild\n");
		ret = -1;
		//回滚到第2步，释放src和dst对应的子目录
		goto rollback_2;
	}
	//构建输入文件信息
	file_in.fd_pos = 0;
	file_in.fd_dir = src_record.child_dir;
	//printk("file size:%d\n", src_record.child_dir->size);
	//根据输入文件的大小分配文件数据缓冲区
	uint8_t *in_buf = (uint8_t *)kmalloc(file_in.fd_dir->size);
	if(in_buf == NULL){
		printk("kmalloc in buf faild!\n");
		ret = -1;
		//回滚到第2步，释放src和dst对应的子目录
		goto rollback_2;
	}
	//到这里，肯定两个文件都找到了
	if(src_found && dst_found){
		//从源文件读取数据
		if(file_read(&file_in, in_buf, file_in.fd_dir->size) != -1){
			//如果读取成功了，我们就把缓冲区里面的数据写入到输出文件
			//write data from src file
			if(file_write(file_out, in_buf, file_in.fd_dir->size) == file_in.fd_dir->size){
				//写入的数据大小和我们要求的一样，说明没有错误，复制文件数据成功
				//printk("copy file sucess!\n");
				ret = 0;
				//成功并且释放in_buf
				goto rollback_3;
			}else{
				printk("file write faild!\n");
				ret = -1;
				//失败并且释放in_buf
				goto rollback_3;
			}
		}else{
			printk("file read faild!\n");
			ret = -1;
			//失败并且释放in_buf
			goto rollback_3;
		}
	}
rollback_3:
	//释放in_buf
	kfree(in_buf);
rollback_2:
	//关闭目的文件的子目录
	close_dir_entry(dst_record.child_dir);
rollback_1:
	//关闭源文件的子目录
	close_dir_entry(src_record.child_dir);
rollback_0:
	//根据不同的返回信息返回不同的值
	return ret;
}
/*******************************************************************************
*Function name: sys_move
*Description: Move a file or directory to other path in kernel state
*Input: oldpath(char *) The file or direcotry we want to move
*		newpath(char *) Where we will move to
*Output: None
*Return: (integer) if 0, we move sucess, if -1, we move faild!
*Auther: Eric hu
*Date: 2018/11/22
*Version: 0.1
*******************************************************************************/
int32_t sys_move(char* oldpath, char* newpath)
{
	printk("move: %s to %s\n", oldpath, newpath);
   /*we can't move /*/
	if (!strcmp(oldpath, "/")) {
      printk("move: can't move / !\n");
      return -1;
	}

	int32_t ret = -1;	// 默认返回值
	struct file_search_record record;
	memset(&record, 0, sizeof(struct file_search_record));   // 记得初始化或清0,否则栈中信息不知道是什么
	int found = search_file(oldpath, &record);

	uint8_t *io_buf = (uint8_t *)kmalloc(SECTOR_SIZE);

	if (found == 1) {
		//find a file/dir that we want.
		printk("move: find oldpath %s is ok!\n", record.child_dir->name);
		
		if (!strcmp(newpath, "/")) {
			//move to root dir
			printk("move: newpath is / .\n");
			
			//clean old dir entry
			if(empty_dir_entry(record.parent_dir, record.child_dir, (void *)io_buf)){
				memset(io_buf, 0, SECTOR_SIZE);
				printk("move: empty %s under %s success!\n", record.child_dir->name, record.parent_dir->name);
				//write dir entry into root dir
				if(sync_dir_entry(&root_dir, record.child_dir, (void *)io_buf)){
					printk("move: sync %s under %s success!\n", record.child_dir->name, root_dir.name);
					
					//close child dir
					close_dir_entry(record.child_dir);
					
					ret = 0;
				}else{
					printk("move: sync %s under %s faild!\n", record.child_dir->name, root_dir.name);
					//close child dir
					close_dir_entry(record.child_dir);

					ret = -1;
				}
			}else{
				printk("move: empty %s under %s faild!\n", record.child_dir->name, root_dir.name);

				//close child dir
				close_dir_entry(record.child_dir);

				ret = -1;
			}
		}else{
			//det isn't root dir 
			printk("move: find oldpath %s is ok!\n", record.parent_dir->name);
			//find newpath
			struct file_search_record record2;
			memset(&record2, 0, sizeof(struct file_search_record));   // 记得初始化或清0,否则栈中信息不知道是什么
			int found2 = search_file(newpath, &record2);

			if(found2 == 1){
				printk("move: find newpath dir %s is ok!\n", record2.child_dir->name);
				if(record2.child_dir->attributes&ATTR_DIRECTORY){
					printk("move: newpath %s is dir.\n", record2.child_dir->name);
					
					//clean old dir entry
					if(empty_dir_entry(record.parent_dir, record.child_dir, (void *)io_buf)){
						memset(io_buf, 0, SECTOR_SIZE);
						printk("move: empty %s under %s success!\n", record.child_dir->name, root_dir.name);
						//write dir entry into root dir
						if(sync_dir_entry(record2.child_dir, record.child_dir, (void *)io_buf)){
							printk("move: sync %s under %s success!\n", record.child_dir->name, record2.child_dir->name);
							ret = 0;
						}else{
							printk("move: sync %s under %s faild!\n", record.child_dir->name, root_dir.name);
							ret = -1;
						}
					}else{
						printk("move: empty %s under %s faild!\n", record.child_dir->name, root_dir.name);
						ret = -1;
					}
				}else{
					printk("move: newpath %s is file, move faild!\n", record2.child_dir->name);
					ret = -1;
				}
				//close new path 
				close_dir_entry(record2.child_dir);
				close_dir_entry(record2.parent_dir);
				//close old path 
				close_dir_entry(record.child_dir);
				close_dir_entry(record.parent_dir);

			}else{
				printk("move: newpath %s not found\n", oldpath);
				//not found, just close parent dir
				close_dir_entry(record2.parent_dir);

				//close old path 
				close_dir_entry(record.child_dir);
				close_dir_entry(record.parent_dir);

				ret = -1;

			}
		}
	} else {
		printk("move: oldpath %s not found\n", oldpath);
		//close old path 
		close_dir_entry(record.parent_dir);

		ret = -1;
	}
	//free io buf
	kfree(io_buf);
	return ret;
}
/*******************************************************************************
*Function name: is_file_exist
*Description: Check file or directory whether exits in fs.
*Input: path(const char*) The file or direcotry we want to check
*Output: None
*Return: (integer) if 0, we file exist, if -1, we file not exist!
*Auther: HZC
*Date: 2019/2/10
*Version: 0.2
*******************************************************************************/
int32_t is_file_exist(const char* path)
{
   /* 若直接查看根目录'/' */
   if (!strcmp(path, "/")) {
      return 0;
   }
   int32_t ret = -1;	// 默认返回值
   struct file_search_record record;
   memset(&record, 0, sizeof(struct file_search_record));   // 记得初始化或清0,否则栈中信息不知道是什么
   int found = search_file(path, &record);
   if (found == 1) {
		close_dir_entry(record.child_dir);
		ret = 0;
   } else {
      printk("exist: %s not exist\n", path);
   }
	close_dir_entry(record.parent_dir);
   return ret;
}

/*******************************************************************************
*Function name: is_file_execute
*Description: Check file or directory can execute in fs.
*Input: path(const char*) The file or direcotry we want to check
*Output: None
*Return: (integer) if 0, our file can execute, if -1, our file can not execute!
*Auther: HZC
*Date: 2018/2/10
*Version: 0.1
*******************************************************************************/
int32_t is_file_execute(const char* path)
{
   /* 若直接查看根目录'/' */
   if (!strcmp(path, "/")) {
      return 0;
   }
   int32_t ret = -1;	// 默认返回值
   struct file_search_record record;
   memset(&record, 0, sizeof(struct file_search_record));   // 记得初始化或清0,否则栈中信息不知道是什么
   int found = search_file(path, &record);
	if (found == 1) {
		
		if((!(record.child_dir->attributes&ATTR_DIRECTORY)) && (record.child_dir->attributes&ATTR_NORMAL)) {
			/*not dir but file*/
			ret = 0;
		}
		close_dir_entry(record.child_dir);
   } else {
      printk("exist: %s not exist\n", path);
   }
	close_dir_entry(record.parent_dir);
   return ret;
}

int sys_access(const char *pathname, int mode)
{
	int ret = -1;
	switch(mode){
		case F_OK:
			if(is_file_exist(pathname) == 0){
				ret = 0;
			}
			break;
		case X_OK:
			if(is_file_execute(pathname) == 0){
				ret = 0;
			}
			break;
		case W_OK:
			
			break;
		case R_OK:
			
			break;
		default:
			printk("access mode unknown!\n");
			break;
	}
	return ret;
}


int32_t sys_stat(char* path, struct stat* buf)
{
   /* 若直接查看根目录'/' */
   if (!strcmp(path, "/")) {
      buf->st_filetype = ATTR_DIRECTORY;
      buf->st_cluster = root_dir.high_cluster<<16|root_dir.low_cluster;
      buf->st_size = root_dir.size;
      return 0;
   }

   int32_t ret = -1;	// 默认返回值
   struct file_search_record record;
   memset(&record, 0, sizeof(struct file_search_record));   // 记得初始化或清0,否则栈中信息不知道是什么
   int found = search_file(path, &record);
   if (found == 1) {
		if (record.child_dir->attributes&ATTR_DIRECTORY) {
			//printk("dir file!\n");
			buf->st_filetype = ATTR_DIRECTORY;
		}else if (record.child_dir->attributes&ATTR_NORMAL) {
			//printk("normal file!\n");
			buf->st_filetype = ATTR_NORMAL;
		}
		buf->st_size = record.child_dir->size;
		buf->st_cluster = record.child_dir->high_cluster<<16|record.child_dir->low_cluster;
		//close_dir_entry(record.parent_dir);
		close_dir_entry(record.child_dir);
		ret = 0;
   } else {
      printk("stat: %s not found\n", path);
   }
	close_dir_entry(record.parent_dir);
   return ret;
}

void sys_close_dir(struct dir* dir)
{
	if(dir == NULL || dir == &dir_root){	// can't close root dir
		
		return;
	}
	//free all
	if(dir->dir_ptr != NULL){
		kfree(dir->dir_ptr);
	}
	kfree(dir);
}
void open_root_dir() 
{
	memset(&dir_root,0,sizeof(struct dir));
	dir_root.dir_ptr = &root_dir;
	dir_root.dir_pos = 0;
	
	uint32_t cluster = dir_root.dir_ptr->high_cluster<<16|dir_root.dir_ptr->low_cluster;
	block_read(cluster_to_lba(cluster), dir_root.dir_buf, 1);
	//printk("child:%s cluster:%x\n", dir_root.dir_ptr->name,cluster);
}
void sys_rewinddir(struct dir* dir)
{
   dir->dir_pos = 0;
}

void sys_ls(char *path, int detail)
{
	//打开根目录
	struct dir *dir = sys_opendir(path);

	//重定位
	sys_rewinddir(dir);
	//读取一个目录项
	struct dir_entry* de;
	char type;
	
	de = sys_readdir(dir);
	while(de != NULL){
		if(de->name[0]){	//显示有名字的
			if(detail){
				
				if(de->attributes&ATTR_DIRECTORY){
					type = 'd';
				}else{
					type = '-';
				}
				printk("%d/%d/%d ",
					DATA16_TO_DATE_YEA(de->create_date),
					DATA16_TO_DATE_MON(de->create_date),
					DATA16_TO_DATE_DAY(de->create_date));
				printk("%d:%d:%d ",
					DATA16_TO_TIME_HOU(de->create_time),
					DATA16_TO_TIME_MIN(de->create_time),
					DATA16_TO_TIME_SEC(de->create_time));
				printk("%c %d %s \n", type, de->size, de->name);
				
			}else{
				printk("%s ", de->name);
			}
			
		}
		de = sys_readdir(dir);
	}
	sys_close_dir(dir);
}

struct dir_entry* sys_readdir(struct dir* dir)
{
	if(dir == NULL){	
		return NULL;
	}
	struct dir_entry *de = (struct dir_entry *)kmalloc(sizeof(struct dir_entry));
	if(de == NULL){	
		return NULL;
	}
	de = (struct dir_entry *)&dir->dir_buf[dir->dir_pos];
	if(de->name[0] == 0){
		return NULL;
	}
	
	dir->dir_pos += 32;
	if(dir->dir_pos > 512){
		//dir->dir_pos = 0;
		return NULL;
	}
	return de;
}

struct dir* sys_opendir(const char* name)
{
	//if it's root , we return root dir
	if (name[0] == '/' && name[1] == 0) {
		uint32_t cluster = dir_root.dir_ptr->high_cluster<<16|dir_root.dir_ptr->low_cluster;
		
		block_read(cluster_to_lba(cluster), dir_root.dir_buf, 1);
		return &dir_root;
	}
	struct file_search_record record;
	
	int found = search_file(name, &record);
	
	struct dir *dir = NULL;
	uint32_t cluster;
	memset(dir,0,sizeof(struct dir));
	if(found == 1){	//fount
		if (record.child_dir->attributes&ATTR_NORMAL) {	///dir
			//printk("%s is regular file!\n", name);
			
			close_dir_entry(record.child_dir);
		} else if (record.child_dir->attributes&ATTR_DIRECTORY) {
			//printk("%s is dir file!\n", name);
			dir = (struct dir *)kmalloc(sizeof(struct dir));
			
			if(dir == NULL){
				close_dir_entry(record.parent_dir);
				close_dir_entry(record.child_dir);
				
				return NULL;
			}
			dir->dir_ptr = record.child_dir;
			dir->dir_pos = 0;
			
			cluster = dir->dir_ptr->high_cluster<<16|dir->dir_ptr->low_cluster;
			block_read(cluster_to_lba(cluster), dir->dir_buf, 1);
			//printk("child:%s cluster:%x\n", dir->dir_ptr->name,cluster);
		}
	}else{
		printk("in path %s not exist\n", name); 
	}
	close_dir_entry(record.parent_dir);

	return dir;
}

int32_t sys_lseek(int32_t fd, int32_t offset, uint8_t whence)
{
	if (fd < 0) {
		printk("lseek: fd error\n");
		return -1;
	}

	struct file* pf = &file_table[fd];
	
	//printk("seek file %s\n",pf->fd_dir->name);
	int32_t new_pos = 0;   //new pos must < file size
	int32_t file_size = (int32_t)pf->fd_dir->size;
	
	switch (whence) {
		case SEEK_SET: 
			new_pos = offset; 
			break;
		case SEEK_CUR: 
			new_pos = (int32_t)pf->fd_pos + offset; 
			break;
		case SEEK_END: 
			new_pos = file_size + offset;
			break;
		default :
			printk("lseek: unknown whence!\n");
	
			break;
	}
	
	if (new_pos < 0 || new_pos > file_size) {	 
		return -1;
	}
	
	pf->fd_pos = new_pos;
	return pf->fd_pos;
}

int32_t sys_write(int32_t fd, void* buf, uint32_t count)
{
	if (fd < 0) {
		printk("write: fd error\n");
		return -1;
	}
	if(count == 0) {
		printk("swrite: count zero\n");
		return -1;
	}
	
    struct file* wr_file = &file_table[fd];
    if(wr_file->fd_flags & O_WRONLY || wr_file->fd_flags & O_RDWR){
		uint32_t bytes_written  = file_write(wr_file, buf, count);
		return bytes_written;
	} else {
		printk("write: not allowed to write file without flag O_RDWR or O_WRONLY\n");
		return -1;
	}
}

int32_t sys_read(int32_t fd, void* buf, uint32_t count)
{
	if (fd < 0) {
		printk("read: fd error\n");
		return -1;
	}
	if (count == 0) {
		printk("read: count zero\n");
		return -1;
	}
	
	struct file* rd_file = &file_table[fd];
	
	//printk("Read file name %s\n", rd_file->fd_dir->name);
    if(rd_file->fd_flags & O_RDONLY || rd_file->fd_flags & O_RDWR){
		uint32_t bytes_road  = file_read(rd_file, buf, count);
		return bytes_road;
	} else {
		printk("read: not allowed to read file without flag O_RDONLY or O_WRONLY\n");
		return -1;
	}
}

int32_t sys_fgetch(int32_t fd)
{
	if (fd < 0) {
		printk("read: fd error\n");
		return -1;
	}

	struct file* rd_file = &file_table[fd];
	int ch = 0;
	//printk("Read file name %s\n", rd_file->fd_dir->name);
    if(rd_file->fd_flags & O_RDONLY || rd_file->fd_flags & O_RDWR){
		int32_t bytes_read  = file_read(rd_file, &ch, 1);
		if(bytes_read == -1 || bytes_read == 0){
			return -1;
		}else{
			return ch;
		}
	} else {
		printk("read: not allowed to read file without flag O_RDONLY or O_WRONLY\n");
		return -1;
	}
}

int32_t sys_fputch(int32_t ch,int32_t fd)
{
	if (fd < 0) {
		printk("write: fd error\n");
		return -1;
	}

	int _ch = ch;
    struct file* wr_file = &file_table[fd];
    if(wr_file->fd_flags & O_WRONLY || wr_file->fd_flags & O_RDWR){
		uint32_t bytes_written  = file_write(wr_file, &_ch, 1);
		if(bytes_written == -1 || bytes_written == 0){
			return -1;
		}else{
			return 0;
		}
	} else {
		printk("write: not allowed to write file without flag O_RDWR or O_WRONLY\n");
		return -1;
	}
}

/* 将最上层路径名称解析出来*/
char* path_parse(char* pathname, char* name_store) 
{
   if (pathname[0] == '/') {   // 根目录不需要单独解析
    /* 路径中出现1个或多个连续的字符'/',将这些'/'跳过,如"///a/b" */
       while(*(++pathname) == '/');
   }

   /* 开始一般的路径解析 */
   while (*pathname != '/' && *pathname != 0) {
      *name_store++ = *pathname++;
   }
   if (pathname[0] == 0) {   // 若路径字符串为空则返回NULL
      return NULL;
   }
   return pathname; 
}

int32_t sys_open(const char *pathname,uint8_t flags)
{
	//printk("open:path %s flags %x\n", pathname, flags);
	
	struct file_search_record record;
	int32_t fd = -1;	   //default not found
	char name[DE_NAME_LEN];
	int reopen = 0;	//file has exist,but operate has O_CREAT
	memset(name,0,DE_NAME_LEN);
	
	//printk("Open file name %s\n", pathname);
	
	int found = search_file(pathname, &record);
	//printk("sys_open:path %s flags %x\n", pathname, flags);
	
	if(path_to_name(pathname, name)){
		//printk("path:%s to name:%s error!\n", pathname, name);
	}else{
		//printk("path:%s to name:%s success!\n", pathname, name);
	}
	//printk("@ 1\n");
	if(found == 1){	//fount
		//printk("parent:%s attr:%x\n", record.parent_dir->name, record.child_dir->attributes);
		//printk("found!\n");
		if(record.child_dir->attributes & ATTR_DIRECTORY){	//found a dir
			printk("open: can't open a direcotry with open(), use opendir() to instead!\n");
			close_dir_entry(record.parent_dir);
			close_dir_entry(record.child_dir);
			//printk("Open file name %s faild 1 end\n", pathname);
			return -1;
		}
		if ((flags & O_CREAT)) {  // file we create has exist!
			//printk("open: %s has already exist!\n", pathname);
			if((flags & O_RDONLY) || (flags & O_WRONLY) || (flags & O_RDWR)){	//just create
				reopen = 1;
				//printk("%s %s can be reopen.\n", pathname, name);
			}else{
				
				close_dir_entry(record.parent_dir);
				close_dir_entry(record.child_dir);
				printk("open: file name %s error!\n", pathname);
				return -1;
			}
		}
	}else {
		//printk("not found!\n");
		if (!found && !(flags & O_CREAT)) {	//not found ,not create
			printk("open: path %s, file isn't exist!\n", pathname);
			close_dir_entry(record.parent_dir);
			close_dir_entry(record.child_dir);
			//printk("Open file name %s faild 3 end\n", pathname);
			return -1;
		}
	}
	//printk("@ 2\n");
	//set attributes
	uint8_t attr = ATTR_NORMAL;
	
	if(flags & O_RDONLY){
		attr |= ATTR_READONLY;
	}else if(flags & O_WRONLY){
		attr |= ATTR_WRITEONLY;
	}else if(flags & O_RDWR){
		attr |= ATTR_RDWR;
	}
	
	if(flags & O_CREAT) {	
		if(!reopen){	//only not found
			//printk("now creating file...\n");
			fd = file_create(record.parent_dir, name, attr);
			//printk("now the fd is %d\n", fd);
			if(fd != -1){	//no error,
				close_dir_entry(record.child_dir);
			}else{	//error, we don't need to close child dir, we had close it in create
				close_dir_entry(record.parent_dir);
			}
			//printk("Open file name %s end\n", pathname);
			
			return fd;
		}
		//found, do nothing
	}
	//
	if((flags & O_RDONLY) || (flags & O_WRONLY) || (flags & O_RDWR)){
		// open exsit file
		//O_RDONLY,O_WRONLY,O_RDWR
		//printk("now opening file...\n");
		fd = file_open(record.parent_dir, name);
		if(fd != -1){	//no error,
			close_dir_entry(record.child_dir);
		}else{	//error, we don't need to close child dir, we had close it in create
			close_dir_entry(record.parent_dir);
		}
		/*
		close_dir_entry(record.parent_dir);
		close_dir_entry(record.child_dir);*/
	}
	/*
	switch (flags & O_CREAT) {
		case O_CREAT:
			printk("creating file\n");
			fd = file_create(record.parent_dir, name, attr);
			close_dir_entry(record.parent_dir);
			break;
		default:
			// open exsit file
			//O_RDONLY,O_WRONLY,O_RDWR
			fd = file_open(record.parent_dir, name);
			close_dir_entry(record.parent_dir);
			break;
	}*/
	//printk("Open file name %s end of ok\n", pathname);
	//printk("now the fd is %d\n", fd);
	return fd;
	
	/*
	//not found , not create, return -1
	if (!found && !(flags & O_CREAT)) {
		printk("path %s, file is't exist\n", pathname);
		close_dir_entry(record.parent_dir);
		return -1;
	} else if (found && (flags & O_CREAT)) {  // file we create has exist!
		printk("%s has already exist!\n", pathname);
		close_dir_entry(record.parent_dir);
		return -1;
	}*/
	
}
/*
only file ,not dir
*/
int32_t sys_unlink(const char* pathname)
{
	//1.file is exist？
	struct file_search_record record;
	int found = search_file(pathname, &record);
	if(found == 1){	//fount
		//printk("parent:%s attr:%x\n", record.parent_dir->name, record.child_dir->attributes);
		
		if(record.child_dir->attributes & ATTR_DIRECTORY){	//found a dir
			printk("unlink: can't delete a direcotry with unlink(), use rmdir() to instead!\n");
			close_dir_entry(record.parent_dir);
			close_dir_entry(record.child_dir);
			return -1;
		}
		//found a file
	}else {
		printk("unlink: file %s not found!\n", pathname);
		close_dir_entry(record.parent_dir);
		close_dir_entry(record.child_dir);
		return -1;
	}
	
	//2.file is in file table
	uint32_t file_idx = 0;
	while (file_idx < MAX_FILE_OPEN) {
		//if name is same and cluster same, that the file we want
		if (file_table[file_idx].fd_dir != NULL &&\
			strcmp(record.child_dir->name, file_table[file_idx].fd_dir->name) == 0 &&\
			(record.child_dir->high_cluster<<16|record.child_dir->low_cluster) == (file_table[file_idx].fd_dir->high_cluster<<16|file_table[file_idx].fd_dir->low_cluster))
		{
			break;
		}
		file_idx++;
	}
	if (file_idx < MAX_FILE_OPEN) {
		close_dir_entry(record.parent_dir);
		close_dir_entry(record.child_dir);
		printk("unlink: file %s is in use, not allow to delete!\n", pathname);
		return -1;
	}
	
	void *io_buf = (void *)kmalloc(SECTOR_SIZE);
	if (io_buf == NULL) {
		close_dir_entry(record.parent_dir);
		close_dir_entry(record.child_dir);
		printk("unlink: malloc for io_buf failed\n");
		return -1;
	}
	//3.empty file's sector and cluster
	release_dir_entry(record.child_dir, io_buf);
	//printk("sys_unlink: release file data success!\n");
	
	//4.del file in parent dir
	if(empty_dir_entry(record.parent_dir, record.child_dir, io_buf)){
		printk("unlink: delete file %s/%s.\n",record.parent_dir->name, record.child_dir->name);
		kfree(io_buf);
		close_dir_entry(record.parent_dir);
		close_dir_entry(record.child_dir);
		//printk("unlink file %s success!\n", pathname);
		return 0;
	}
	kfree(io_buf);
	close_dir_entry(record.parent_dir);
	close_dir_entry(record.child_dir);
	printk("unlink: delete file %s/%s faild!\n",record.parent_dir->name, record.child_dir->name);
	return -1;
}

/*
close file which fd point to, return 0 is ok, error -1
*/
int32_t sys_close(int32_t fd)
{
	int32_t ret = -1;   // defaut -1,error
	if (fd >= 0) {
		ret = file_close(&file_table[fd]);
		//printk("close fd:%d success!\n", fd);
	}else{
		printk("close: fd %d error!\n", fd);
	}
	
	return ret;
}

/*
get the name from path
*/
int path_to_name(const char *pathname, char *name_buf)
{
	char *p = (char *)pathname;
	char deep = 0;
	char name[DE_NAME_LEN];
	int i,j;
	
	if(*p != '/'){	//First must be /
		return 1;
	}
	//Count how many dir 
	while(*p){
		if(*p == '/'){
			deep++;
		}
		p++;
	}
	//printk("deep:%d \n",deep);
	p = (char *)pathname;
	for(i = 0; i < deep; i++){
		
		memset(name,0,DE_NAME_LEN);
		
		p++;	//skip '/'
		j = 0;
		//get a dir name
		while(*p != '/' && j < DE_NAME_LEN){	//if not arrive next '/'
			name[j] = *p;	// transform to A~Z
			j++;
			p++;
		}
		name[j] = 0;
		//printk("name:%s %d\n",name, i);
		if(name[0] == 0){	//no name
			return 1;
		}
		
		if(i == deep-1){	//name is what we need
			j = 0;
			while(name[j]){	//if not arrive next '/'
				name_buf[j] = name[j];	// transform to A~Z
				j++;
			}
			name_buf[j] = 0;
			return 0;	//success
		}
	}
	return 1;
}

static void init_fsinfo()
{
	memset(&fsinfo, 0, sizeof(struct fsinfo ));
	//clean_sector(1);
	block_read(1, &fsinfo, 1);
	if(fsinfo.fsinfo_signature != 0x61417272){
		//Init fsinfo
		fsinfo.extended_flags = 0x41615252;
		fsinfo.fsinfo_signature = 0x61417272;
		fsinfo.free_cluster_number = 0xffffffff;
		fsinfo.next_available_cluster = 0x00000002;	//Start at cluster 2 
		
		block_write(1, &fsinfo, 1);
	}
	printk("fsinfo: next available cluster:%d\n",fsinfo.next_available_cluster);
}

static void init_dbr(uint32 blocks)
{
	memset(&dbr, 0, sizeof(struct dbr ));
	//clean_sector(0);
	block_read(0, &dbr, 1);
	
	if(dbr.magic_flags != 0xaa55){
		
		dbr.bpb.byte_per_sector = 0x200;
		dbr.bpb.sector_per_cluster = 0x01;
		dbr.bpb.reserved_sector = 0x20;
		dbr.bpb.fats = 0x01;
		dbr.bpb.media_descriptor = 0xf8;
		dbr.bpb.totla_sector = blocks;
		
		dbr.bpb.fat_sectors = (dbr.bpb.totla_sector - dbr.bpb.reserved_sector)/128;	//Every fat has how many sectors
		dbr.bpb.hidden_sector = 0;
		
		dbr.ext_bpb.physical_drive_number = 0x80;
		strcpy(dbr.ext_bpb.system_ID, "FATXE");
		strcpy(dbr.oem_name, "Ratel   ");
		dbr.magic_flags = 0xaa55;
		block_write(0, &dbr, 1);

	}
	
	sector_per_cluster = dbr.bpb.sector_per_cluster;
	
	
	printk("dbr: byte per sector:%x sector per cluster:%x\n\
    reserved sector:%x fats:%d fat sectors:%d\n",\
		dbr.bpb.byte_per_sector,\
		dbr.bpb.sector_per_cluster,\
		dbr.bpb.reserved_sector,\
		dbr.bpb.fats,\
		dbr.bpb.fat_sectors);
}

/*
*sync fsinfo into disk
*/

void sync_fsinfo()
{
	block_write(1, &fsinfo, 1);
}
