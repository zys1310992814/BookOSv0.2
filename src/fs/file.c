/*
File:		fs/file.c
Contains:	fatxe's file manage
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/


#include <sys/harddisk.h>
#include <sys/fatxe.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <sys/fat.h>
#include <sys/console.h>
#include <sys/x86.h>
#include <string.h>

/* file table */
struct file file_table[MAX_FILE_OPEN];

void init_file_table()
{
	uint32_t fd_idx = 0;
	while (fd_idx < MAX_FILE_OPEN) {
		file_table[fd_idx].fd_dir = NULL;
		file_table[fd_idx].fd_parent = NULL;
		fd_idx++;
	}
}

struct file *get_file_though_fd(int fd)
{
	if(fd < 0 || fd >= MAX_FILE_OPEN){
		return NULL;
	}
	return &file_table[fd];
}

/* 从文件表file_table中获取一个空闲位,成功返回下标,失败返回-1 */
int32_t alloc_in_file_table(void)
{
	uint32_t fd_idx = 0;
	while (fd_idx < MAX_FILE_OPEN) {
		if (file_table[fd_idx].fd_dir == NULL && file_table[fd_idx].fd_parent == NULL) {
			break;
		}
		fd_idx++;
	}
	if (fd_idx == MAX_FILE_OPEN) {
		printk("error: alloc_in_file_table:exceed max open files\n");
		return -1;
	}
	return fd_idx;
}

int32_t file_open(struct dir_entry *parent_dir, char *name)
{
	//1.read from disk
	uint32_t *buffer = (uint32_t *)kmalloc(SECTOR_SIZE);
	if(buffer == NULL){
		return -1;
	}
	
	struct dir_entry *child_dir = (struct dir_entry *)kmalloc(sizeof(struct dir_entry));
	if(child_dir == NULL){
		kfree(buffer);
		return -1;
	}
	memset(buffer,0,SECTOR_SIZE);
	if(load_dir_entry(parent_dir, name,child_dir, buffer)){	//success
		int fd_idx = alloc_in_file_table();
		if (fd_idx == -1) {
			kfree(buffer);
			kfree(child_dir);
			return -1;
		}
		file_table[fd_idx].fd_dir = child_dir;
		file_table[fd_idx].fd_parent = parent_dir;
		file_table[fd_idx].fd_pos = 0;
		file_table[fd_idx].fd_flags = child_dir->attributes;
		//we can't free child_dir, because we will use it in fd
		kfree(buffer);
		return fd_idx;
	}
	return -1;
}

int32_t file_create(struct dir_entry *parent_dir, char *name, uint8_t attr)
{
	/*1.create file*/
	struct dir_entry *child_dir = (struct dir_entry *)kmalloc(sizeof(struct dir_entry));
	if(child_dir == NULL){
		return -1;
	}
	create_dir_entry(child_dir, name, attr);
	
	uint8_t *buffer = (uint8_t *)kmalloc(SECTOR_SIZE);
	if(buffer == NULL){
		kfree(child_dir);
		return -1;
	}
	/*2.find a child dir entry under parent*/
	if(sync_dir_entry(parent_dir, child_dir, buffer)){	//success
		//printk("file_create: new file:%s dir:%s\n", child_dir->name, parent_dir->name);
		/*4.set fd*/
		int fd_idx = alloc_in_file_table();
		if (fd_idx == -1) {
			kfree(child_dir);
			kfree(buffer);
			return -1;
		}
		
		file_table[fd_idx].fd_parent = parent_dir;
		file_table[fd_idx].fd_dir = child_dir;
		file_table[fd_idx].fd_pos = 0;
		uint8_t flags;
		if(child_dir->attributes & ATTR_READONLY){
			flags = ATTR_READONLY;
		}else if(child_dir->attributes & ATTR_WRITEONLY){
			flags = ATTR_WRITEONLY;
		}else if(child_dir->attributes & ATTR_RDWR){
			flags = ATTR_RDWR;
		}
		
		file_table[fd_idx].fd_flags = flags;
		//we can't free child_dir, because we will use it in fd
		kfree(buffer);
		return fd_idx;
	}else{
		kfree(child_dir);
		kfree(buffer);
		return -1;
	}
}

int search_file(const char* pathname, struct file_search_record *record)
{
	char *p = (char *)pathname;
	char deep = 0;
	char name[DE_NAME_LEN];
	int i,j;
	
	struct dir_entry *parent_dir = (struct dir_entry *)kmalloc(SECTOR_SIZE);
	struct dir_entry *child_dir = (struct dir_entry *)kmalloc(SECTOR_SIZE);
	
	memset(parent_dir,0, SECTOR_SIZE);
	memset(child_dir,0, SECTOR_SIZE);
	
	if(*p != '/'){	//First must be /
		return 0;
	}
	//Count how many dir 
	while(*p){
		if(*p == '/'){
			deep++;
		}
		p++;
	}
	
	//printk("search ing!\n");
	
	p = (char *)pathname;
	for(i = 0; i < deep; i++){
		p++;	//skip '/'

		memset(name,0,DE_NAME_LEN);
		j = 0;
		//get a dir name
		while(*p != '/' && j < DE_NAME_LEN){	//if not arrive next '/'
			name[j] = *p;	// transform to A~Z
			j++;
			p++;
		}
		
		if(j == 0){	//no name
			printk("no name\n");
			return 0;
		}
		//判断是不是只有根目录
		if(deep == 1 && pathname[0] == '/' && pathname[1] == 0){
			printk("search: pathname is only root dir %s\n", pathname);
			return 1;
		}
		
		if(i == 0){	//root dir 
			//printk("in root dir\n");
	
			//get root dir entry
			if(search_dir_entry(&root_dir,name, child_dir)){	//find
				//printk("search:deep:%d path:%s name:%s has exsit!\n",deep, pathname, name);
				//printk("search:child name:%s attr:%x\n",child_dir->name,child_dir->attributes);
				
				if(i == deep - 1){	//finally
					//printk("search:found!\n");
					
					record->parent_dir = &root_dir;
					
					record->child_dir = child_dir;
					kfree(parent_dir);
					
					return 1;
				}else{
					//printk("search:continue!\n");
					memcpy(parent_dir, child_dir, sizeof(struct dir_entry));	//child_dir become parent_dir
				}
				//printk("find\n");
	
				
			}else{
				//printk("not found\n");
	
				//printk("search:error!\n");
				record->parent_dir = &root_dir;
				record->child_dir = NULL;
				
				kfree(parent_dir);
				kfree(child_dir);
				
				return 0;
			}
		}else{	//if not under the root dir 
			
			//parent_dir we have gotten under root dir
			//get root dir entry
			if(search_dir_entry(parent_dir,name, child_dir)){	//find
				//printk("search:deep:%d path:%s name:%s has exsit!\n",deep, pathname, name);
				//printk("search:child name:%s attr:%x\n",child_dir->name,child_dir->attributes);
				
				if(i == deep - 1){	//finally
					//printk("search:found!\n");
					
					record->parent_dir = parent_dir;
					record->child_dir = child_dir;
					
					
					return 1;
				}else{
					//printk("search:continue!\n");
					memcpy(parent_dir, child_dir, sizeof(struct dir_entry));	//child_dir become parent_dir
				}
			}else{
				record->parent_dir = parent_dir;
				record->child_dir = NULL;	//error
				
				//printk("search:error!\n");
				kfree(child_dir);
				return 0;
			}
		}
	}
	return 0;
	
}
/*
close file
*/
int32_t file_close(struct file* file)
{
   if (file == NULL) {
      return -1;
   }
   file->fd_pos = 0;
	file->fd_flags = 0;
   close_dir_entry(file->fd_dir);
   close_dir_entry(file->fd_parent);
   file->fd_dir = NULL;   //free file struct
    file->fd_parent = NULL;   //free file struct
   
   return 0;
}

int32_t file_write(struct file* file, void* buf, uint32_t count)
{
	//if pos > file size, we should correcte it
	if(file->fd_pos > file->fd_dir->size){
		file->fd_pos = file->fd_dir->size;
	}
	
	uint8_t* io_buf = (uint8_t* )kmalloc(SECTOR_SIZE);
	if (io_buf == NULL) {
		printk("file_write: kmalloc for io_buf failed\n");
		return -1;
	}
	
	uint32_t cluster = (file->fd_dir->high_cluster<<16)|file->fd_dir->low_cluster;
	//printk("file:%s size:%d cluster:%x\n", file->fd_dir->name, file->fd_dir->size, cluster);
	
	//1.we need how many cluster
	uint32_t all_clusters_old = get_left_clusters(cluster);
	//printk("file_write:name:%s clusters:%d\n",file->fd_dir->name, all_clusters_old);
	
	//we will beyond how many bytes, we need add how many bytes beyond file size
	uint32_t will_beyond_bytes;	
	if(file->fd_pos < file->fd_dir->size){//pos is in file size
		if((file->fd_pos + count) < file->fd_dir->size){	
			will_beyond_bytes = 0;
			//printk("below pos start:%d pos end:%d cunt:%d\n",file->fd_pos, file->fd_pos + count, count);
		}else{	//end >= size
			will_beyond_bytes = count - (file->fd_dir->size - file->fd_pos);
			//printk("above pos start:%d pos end:%d cunt:%d\n",file->fd_pos, file->fd_pos + count, count);
		}
	}else{	//pos is equal to size
		will_beyond_bytes = count;
	}
	uint32_t will_beyond_sectors = 0;	//default is 0 sectors
	if(will_beyond_bytes > 0){//if have beyond bytes
		will_beyond_sectors = (will_beyond_bytes-1)/SECTOR_SIZE + 1;
	}
	
	//printk("will_beyond_bytes:%x will_beyond_sectors:%d\n",will_beyond_bytes, will_beyond_sectors);

	uint32_t all_clusters_new;

	if(all_clusters_old == 0){
		all_clusters_new = all_clusters_old + will_beyond_sectors;
	}else{
		all_clusters_new = all_clusters_old + will_beyond_sectors + 1;
	}
	
	//printk("all_clusters_new:%d\n", all_clusters_new);
	
	uint32_t *sector_table = (uint32_t *)kmalloc(all_clusters_new*4);
	memset(sector_table,0, all_clusters_new*4);
	//now we collect all sectors into sector_table
	//first get old cluster
	int i;
	uint32_t idx_in_st = 0;	//idx in sector table
	
	uint32_t last_cluster = cluster;	//record last cluster
	if(all_clusters_old == 0){	//no data
		sector_table[idx_in_st] = cluster_to_lba(cluster);
		last_cluster = cluster;
		cluster = get_next_cluster(cluster);
		//printk("collect:sector_table[%d] = %d\n", idx_in_st, sector_table[idx_in_st]);
		idx_in_st++;
	}else{
		for(i = 0; i < all_clusters_old+1; i++){
			sector_table[idx_in_st] = cluster_to_lba(cluster);
			last_cluster = cluster;
			cluster = get_next_cluster(cluster);
			//printk("collect:sector_table[%d] = %d\n", idx_in_st, sector_table[idx_in_st]);
			
			idx_in_st++;
		}
	}
	
	//printk("collect old over idx_in_st:%d\n", idx_in_st);
	uint32_t new_cluster;
	//get new cluster
	for(i = idx_in_st; i < all_clusters_new; i++){
		//alloc new cluster
		new_cluster = get_a_cluster();
		//link with last cluster
		write_fat_item(last_cluster, new_cluster);
		
		//printk("get new cluster:%x->%x last cluster:%x->%x\n", new_cluster, read_fat_item(new_cluster), last_cluster, read_fat_item(last_cluster));
		//collect
		sector_table[idx_in_st] = cluster_to_lba(new_cluster);
		last_cluster = new_cluster;
		
		idx_in_st++;
	}
	
	//printk("collect new over idx_in_st:%d\n", idx_in_st);
	for(i = 0; i < idx_in_st; i++){
		//printk("sector_table[%d]=%d\n", i, sector_table[i]);
	}
	//panic("test!!");
	
	uint8_t* src = buf;        // 用src指向buf中待写入的数据 
	uint32_t bytes_written = 0;	    // 用来记录已写入数据大小
	uint32_t size_left = count;	    // 用来记录未写入数据大小
	uint32_t sec_idx = 0;	      // 用来索引扇区
	uint32_t sec_lba;	      // 扇区地址
	uint32_t sec_off_bytes;    // 扇区内字节偏移量
	uint32_t sec_left_bytes;   // 扇区内剩余字节量
	uint32_t chunk_size;	      // 每次写入硬盘的数据块大小
	
	bool first_write_sector = true;
	bool change_file_size = false;
	if(file->fd_pos + count > file->fd_dir->size){
		change_file_size = true;
	}
	while (bytes_written < count) {      // 直到写完所有数据
		memset(io_buf, 0, SECTOR_SIZE);

		sec_lba = sector_table[sec_idx];	//get sector
		
		//get remainder of pos = pos/512
		sec_off_bytes = file->fd_pos % SECTOR_SIZE;	
		sec_left_bytes = SECTOR_SIZE - sec_off_bytes;
		
		chunk_size = size_left < sec_left_bytes ? size_left : sec_left_bytes;
		
		//printk("sector:%d off:%d left:%d chunk:%d\n", sec_lba, sec_off_bytes, sec_left_bytes, chunk_size);
		
		if (first_write_sector) {	//we need to keep first sector not
			block_read(sec_lba, io_buf, 1);
			first_write_sector = false;
			//printk("first write, need to read old data!\n");
		}
		memcpy(io_buf + sec_off_bytes, src, chunk_size);
		block_write(sec_lba, io_buf, 1);

		src += chunk_size;   // 将指针推移到下个新数据
		file->fd_pos += chunk_size;   
		if(change_file_size == true){
			file->fd_dir->size  = file->fd_pos;    // 更新文件大小
		}
		bytes_written += chunk_size;
		size_left -= chunk_size;
		sec_idx++;
	}
	
	
	//write dir
	//printk("parent:%s child:%s\n", file->fd_parent->name, file->fd_dir->name);
	memset(io_buf,0,SECTOR_SIZE);
	if(sync_dir_entry(file->fd_parent, file->fd_dir, io_buf)){
		kfree(sector_table);
		kfree(io_buf);
		return bytes_written;
	}else{
		kfree(sector_table);
		kfree(io_buf);
		return -1;
	}
}

int32_t file_read(struct file* file, void* buf, uint32_t count)
{
	uint32_t size = count, size_left = count;
	//printk("pos:%d count:%d size:%d\n", file->fd_pos, count, file->fd_dir->size);

	//check read bytes
	if ((file->fd_pos + count) > file->fd_dir->size){
		size = file->fd_dir->size - file->fd_pos;
		size_left = size;
		//printk("size_left:%d\n", size_left);
		
		if (size == 0) {	   // 若到文件尾则返回-1
			return -1;
		}
	}
	
	uint8_t* io_buf = (uint8_t* )kmalloc(SECTOR_SIZE);
	if (io_buf == NULL) {
		printk("file_write: kmalloc for io_buf failed\n");
		return -1;
	}
	
	uint32_t sector_read_start_idx = file->fd_pos / SECTOR_SIZE;		       // 数据所在块的起始地址
	uint32_t sector_read_end_idx = (file->fd_pos + size) / SECTOR_SIZE;	       // 数据所在块的终止地址
	uint32_t read_sectors = (sector_read_end_idx - sector_read_start_idx) + 1;	       // 如增量为0,表示数据在同一扇区

	uint32_t cluster = (file->fd_dir->high_cluster<<16)|file->fd_dir->low_cluster;
	//printk("file:%s size:%d cluster:%x\n", file->fd_dir->name, file->fd_dir->size, cluster);
	
	//1.we need how many cluster
	uint32_t all_clusters_old = get_left_clusters(cluster);
	//printk("file_write:name:%s clusters:%d\n",file->fd_dir->name, all_clusters_old);

	uint32_t all_clusters_new;
	//get max file clusters
	all_clusters_new = all_clusters_old + 1;
	
	//printk("all_clusters_new:%d\n", all_clusters_new);
	
	uint32_t *sector_table = (uint32_t *)kmalloc(all_clusters_new*4);
	memset(sector_table,0, all_clusters_new*4);
	//now we collect all sectors into sector_table
	int i;
	uint32_t idx_in_st = 0;	//idx in sector table
	for(i = 0; i < all_clusters_new && read_sectors > 0; i++){
		sector_table[idx_in_st] = cluster_to_lba(cluster);
		cluster = get_next_cluster(cluster);
		//printk("collect:sector_table[%d] = %d\n", idx_in_st, sector_table[idx_in_st]);
		idx_in_st++;
		read_sectors--;
	}
	
	//printk("collect new over idx_in_st:%d\n", idx_in_st);
	for(i = 0; i < idx_in_st; i++){
		//printk("sector_table[%d]=%d\n", i, sector_table[i]);
	}
	//panic("test!!");
	
	uint8_t* dst = buf;        // 用src指向buf中待写入的数据 
	uint32_t bytes_read = 0;	    // 用来记录已写入数据大小
	uint32_t sec_idx = 0;	      // 用来索引扇区
	uint32_t sec_lba;	      // 扇区地址
	uint32_t sec_off_bytes;    // 扇区内字节偏移量
	uint32_t sec_left_bytes;   // 扇区内剩余字节量
	uint32_t chunk_size;	      // 每次写入硬盘的数据块大小
	
	while (bytes_read < size) {      // 直到写完所有数据
		//memset(io_buf, 0, SECTOR_SIZE);

		sec_lba = sector_table[sec_idx];	//get sector
		
		//get remainder of pos = pos/512
		sec_off_bytes = file->fd_pos % SECTOR_SIZE;	
		sec_left_bytes = SECTOR_SIZE - sec_off_bytes;
		
		chunk_size = size_left < sec_left_bytes ? size_left : sec_left_bytes;
		
		//printk("sector:%d off:%d left:%d chunk:%d\n", sec_lba, sec_off_bytes, sec_left_bytes, chunk_size);

		memset(io_buf, 0, SECTOR_SIZE);
		block_read(sec_lba, io_buf, 1);
		
		memcpy(dst, io_buf + sec_off_bytes, chunk_size);
		
		dst += chunk_size;
		file->fd_pos += chunk_size;
		bytes_read += chunk_size;
		size_left -= chunk_size;
		sec_idx++;
	}
	
	kfree(sector_table);
	kfree(io_buf);
	return bytes_read;
}

