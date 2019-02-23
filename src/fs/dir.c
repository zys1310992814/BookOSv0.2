/*
File:		fs/dir.c
Contains:	fatxe's dir manage
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
#include <sys/clock.h>
#include <string.h>

struct dir_entry root_dir;

void init_directory()
{
	
	uint32_t *buffer = (uint32_t *)kmalloc(SECTOR_SIZE);
	
	if(buffer == NULL){
		return;
	}
	
	root_dir_start_lba = data_start_lba;
	
	memset(buffer, 0, SECTOR_SIZE);
	
	block_read(root_dir_start_lba, buffer, 1);
	
	memcpy(&root_dir, buffer, sizeof(struct dir_entry));
	if(strcmp(root_dir.name, ".") != 0){
		
		memset(&root_dir,0,sizeof(struct dir_entry));
		strcpy(root_dir.name,".");
		root_dir.attributes = ATTR_DIRECTORY;
		uint32_t cluster = get_a_cluster();
		root_dir.high_cluster = (cluster&0xffff0000)>>16;
		root_dir.low_cluster = cluster&0x00000ffff;
		root_dir.size = 0;
		
		root_dir.create_date = DATE_TO_DATA16(time.year, time.month, time.day);
		root_dir.create_time =  TIME_TO_DATA16(time.hour, time.minute, time.second);
		
		root_dir.modify_date = root_dir.create_date;
		root_dir.modify_time = root_dir.create_time;
		
		root_dir.access_date = root_dir.create_date;

		
		memset(buffer,0,SECTOR_SIZE);
		memcpy(buffer, &root_dir, sizeof(struct dir_entry));
		block_write(data_start_lba, buffer, 1);
	}
	
	printk("time %d/%d/%d",
	DATA16_TO_DATE_YEA(root_dir.create_date),
	DATA16_TO_DATE_MON(root_dir.create_date),
	DATA16_TO_DATE_DAY(root_dir.create_date));
		printk(" %d:%d:%d\n",
	DATA16_TO_TIME_HOU(root_dir.create_time),
	DATA16_TO_TIME_MIN(root_dir.create_time),
	DATA16_TO_TIME_SEC(root_dir.create_time));
	
	kfree(buffer);
	//uint32_t lba = cluster_to_lba(root_dir_entry.high_cluster<<16|root_dir_entry.low_cluster);
	//printk("DIR:/%s cluster:%d lba:%d\n",root_dir_entry.name, lba_to_cluster(lba), lba);
}

int32_t sys_mkdir(const char *pathname)
{
	char *p = (char *)pathname;
	char deep = 0;
	char name[DE_NAME_LEN];
	int i,j;
	struct dir_entry parent_dir, child_dir;
	
	uint32_t *buffer = (uint32_t *)kmalloc(SECTOR_SIZE);

	//printk("[mkdir]: dir %s\n",pathname );
	
	if(buffer == NULL){
		return -1;
	}
	if(*p != '/'){	//First must be /
		return -1;
	}
	//Count how many dir 
	while(*p){
		if(*p == '/'){
			deep++;
		}
		p++;
	}

	p = (char *)pathname;
	for(i = 0; i < deep; i++){
		p++;	//skip '/'
		/*for(j = 0; j < 8; j++){
			name[j] = 0x20;		//fill name with ' '
		}*/
		memset(name,0,DE_NAME_LEN);
		j = 0;
		//get a dir name
		while(*p != '/' && j < DE_NAME_LEN){	//if not arrive next '/'
			name[j] = *p;	// transform to A~Z
			j++;
			p++;
		}
		if(name[0] == 0){	//no name
			return -1;
		}
		if(i == 0){	//root dir 
			
			//get root dir entry
			
			if(search_dir_entry(&root_dir,name, &child_dir)){	//find
				memcpy(&parent_dir, &child_dir, sizeof(struct dir_entry));	//child_dir become parent_dir
				//printk("sys_mkdir:deep:%d path:%s name:%s has exsit! find\n",deep, pathname, name);
			}else{
				//printk("sys_mkdir:deep:%d path:%s name:%s not exsit! not find\n",deep, pathname, name);
				
				if(i == deep - 1){
					//printk("sys_mkdir: create path:%s name:%s\n", pathname, name);
					//create 
					create_dir_entry(&child_dir, name, ATTR_DIRECTORY);
					//we need create . and .. under child 
					struct dir_entry dir_0, dir_1;
					
					
					// . point to child_dir .. point to parent_dir
					//copy data
					copy_dir_entry(&dir_0, &child_dir);
					//reset name
					memset(dir_0.name,0,DE_NAME_LEN);
					//set name
					strcpy(dir_0.name, ".");

					copy_dir_entry(&dir_1, &root_dir);
					memset(dir_1.name,0,DE_NAME_LEN);
					strcpy(dir_1.name, "..");

					//create_dir_entry(&dir_0, ".","");
					//create_dir_entry(&dir_1, "..","");
					sync_dir_entry(&child_dir, &dir_0, buffer);
					sync_dir_entry(&child_dir, &dir_1, buffer);
					//sync_dir_info(&root_dir, buffer);
					if(sync_dir_entry(&root_dir, &child_dir, buffer)){	//successed
						//printk("[sync] success!\n");
						
						kfree(buffer);
						return 0;
					}
					
				}else{
					printk("mkdir:can create path:%s name:%s\n", pathname, name);
					kfree(buffer);
					return -1;
				}
				
			}
		}else{	//if not under the root dir 
			
			//parent_dir we have gotten under root dir
			
			if(search_dir_entry(&parent_dir,name, &child_dir)){	//find
				memcpy(&parent_dir, &child_dir, sizeof(struct dir_entry));	//child_dir become parent_dir
				//printk("sys_mkdir:deep:%d path:%s name:%s has exsit!\n",deep, pathname, name);
			}else{
				//printk("sys_mkdir:deep:%d path:%s name:%s not exsit!\n",deep, pathname, name);
				if(i == deep - 1){	//create it
					create_dir_entry(&child_dir, name, ATTR_DIRECTORY);
					//we need create . and .. under child 
					struct dir_entry dir_0, dir_1;
					
					// . point to child_dir .. point to parent_dir
					//copy data
					copy_dir_entry(&dir_0, &child_dir);
					//reset name
					memset(dir_0.name,0,DE_NAME_LEN);
					//set name
					strcpy(dir_0.name, ".");

					copy_dir_entry(&dir_1, &parent_dir);
					memset(dir_1.name,0,DE_NAME_LEN);
					strcpy(dir_1.name, "..");

					sync_dir_entry(&child_dir, &dir_0, buffer);
					sync_dir_entry(&child_dir, &dir_1, buffer);
					
					if(sync_dir_entry(&parent_dir, &child_dir, buffer)){	//successed
						//同步自己
						kfree(buffer);
						return 0;
					}
				}else{
					printk("mkdir:can create path:%s name:%s\n", pathname, name);
					kfree(buffer);
					return -1;
				}
			}
		}
	}
	kfree(buffer);
	return -1;
}

int32_t sys_rename(const char *pathname, char *new_name)
{
	char *p = (char *)pathname;
	char deep = 0;
	char name[DE_NAME_LEN];
	int i,j;
	struct dir_entry parent_dir, child_dir;
	uint32_t *buffer = (uint32_t *)kmalloc(SECTOR_SIZE);

	if(buffer == NULL){
		return -1;
	}
	if(*p != '/'){	//First must be /
		return -1;
	}
	//Count how many dir 
	while(*p){
		if(*p == '/'){
			deep++;
		}
		p++;
	}

	p = (char *)pathname;
	for(i = 0; i < deep; i++){
		p++;	//skip '/'
		/*for(j = 0; j < 8; j++){
			name[j] = 0x20;		//fill name with ' '
		}*/
		memset(name,0,DE_NAME_LEN);
		j = 0;
		//get a dir name
		while(*p != '/' && j < DE_NAME_LEN){	//if not arrive next '/'
			name[j] = *p;	// transform to A~Z
			j++;
			p++;
		}
		
		if(name[0] == 0){	//no name
			return -1;
		}
		if(i == 0){	//root dir 
			
			
			//get root dir entry
			if(search_dir_entry(&root_dir,name, &child_dir)){	//find
				
				//printk("sys_rmdir:deep:%d path:%s name:%s has exsit!\n",deep, pathname, name);
				//now we found the dir
				if(i == deep - 1){	//dir is what we want and !!!it must be empty!!!
					
					
					//rename it
					//1.copy new name
					strcpy(child_dir.name, new_name);

					//2.free dir entry in parent
					if(sync_dir_entry(&root_dir, &child_dir, buffer)){
						//同步自己
						
						kfree(buffer);
						return 0;
					}
				}else{// not ,so we save parent for next time search
					memcpy(&parent_dir, &child_dir, sizeof(struct dir_entry));	//child_dir become parent_dir
					printk("rename:can't rename path:%s name:%s\n", pathname, name);
				}
			}else{
				printk("rename:path:%s name:%s not exist!\n", pathname, name);
			}
		}else{	//if not under the root dir 
			
			//parent_dir we have gotten under root dir
			if(search_dir_entry(&parent_dir,name, &child_dir)){	//find
				
				//printk("sys_rmdir:deep:%d path:%s name:%s has exsit!\n",deep, pathname, name);
				//now we found the dir
				if(i == deep - 1){	//dir is what we want and !!!it must be empty!!!
					
					//rename it
					//1.copy new name
					strcpy(child_dir.name, new_name);

					//2.free dir entry in parent
					if(sync_dir_entry(&parent_dir, &child_dir, buffer) ){
						//同步自己
						
						kfree(buffer);
						return 0;
					}
				}else{// not ,so we save parent for next time search
					memcpy(&parent_dir, &child_dir, sizeof(struct dir_entry));	//child_dir become parent_dir
					printk("rename:can't rename path:%s name:%s\n", pathname, name);
					kfree(buffer);
					return -1;
				}
			}else{
				printk("rename:path:%s name:%s not exist!\n", pathname, name);
			}
		}
	}
	kfree(buffer);
	return -1;
}

int32_t sys_rmdir(const char *pathname)
{
	char *p = (char *)pathname;
	char deep = 0;
	char name[DE_NAME_LEN];
	int i,j;
	struct dir_entry parent_dir, child_dir;
	uint32_t *buffer = (uint32_t *)kmalloc(SECTOR_SIZE);

	if(buffer == NULL){
		return -1;
	}
	if(*p != '/'){	//First must be /
		return -1;
	}
	//Count how many dir 
	while(*p){
		if(*p == '/'){
			deep++;
		}
		p++;
	}

	p = (char *)pathname;
	for(i = 0; i < deep; i++){
		p++;	//skip '/'
		/*for(j = 0; j < 8; j++){
			name[j] = 0x20;		//fill name with ' '
		}*/
		memset(name,0,DE_NAME_LEN);
		j = 0;
		//get a dir name
		while(*p != '/' && j < DE_NAME_LEN){	//if not arrive next '/'
			name[j] = *p;	// transform to A~Z
			j++;
			p++;
		}
		
		if(name[0] == 0){	//no name
			return -1;
		}
		if(i == 0){	//root dir 
			
			
			//get root dir entry
			
			if(search_dir_entry(&root_dir,name, &child_dir)){	//find
				
				//printk("sys_rmdir:deep:%d path:%s name:%s has exsit!\n",deep, pathname, name);
				//now we found the dir
				if(i == deep - 1){	//dir is what we want and !!!it must be empty!!!
					
		
					//rm it
					//printk("sys_rmdir:rm path:%s name:%s\n", pathname, name);
					//1.free all cluster and sector
					release_dir_entry(&child_dir, (char *)buffer);
					
					//2.free dir entry in parent
					if(empty_dir_entry(&root_dir, &child_dir, buffer)){
						//同步自己
						
						kfree(buffer);
						return 0;
					}
				}else{// not ,so we save parent for next time search
					memcpy(&parent_dir, &child_dir, sizeof(struct dir_entry));	//child_dir become parent_dir
					printk("rmdir:can't rm path:%s name:%s\n", pathname, name);
				}
			}else{
				printk("rmdir:path:%s name:%s not exist!\n", pathname, name);
			}
		}else{	//if not under the root dir 
			
			//parent_dir we have gotten under root dir
			if(search_dir_entry(&parent_dir,name, &child_dir)){	//find
				
				//printk("sys_rmdir:deep:%d path:%s name:%s has exsit!\n",deep, pathname, name);
				//now we found the dir
				if(i == deep - 1){	//dir is what we want and !!!it must be empty!!!
					
					//rm it
					//printk("sys_rmdir:rm path:%s name:%s\n", pathname, name);
					
					//1.free all cluster and sector
					release_dir_entry(&child_dir, (char *)buffer);
					
					//2.free dir entry in parent
					if(empty_dir_entry(&parent_dir, &child_dir, buffer) ){
						//同步自己
						
						kfree(buffer);
						return 0;
					}
				}else{// not ,so we save parent for next time search
					memcpy(&parent_dir, &child_dir, sizeof(struct dir_entry));	//child_dir become parent_dir
					printk("rmdir:can't rm path:%s name:%s\n", pathname, name);
					kfree(buffer);
					return -1;
				}
			}else{
				printk("rmdir:path:%s name:%s not exist!\n", pathname, name);
			}
		}
	}
	kfree(buffer);
	return -1;
}

void create_dir_entry(struct dir_entry *dir, char *name, uint8_t attributes)
{
	uint32_t cluster;

	memset(dir, 0, sizeof(struct dir_entry));
	strcpy(dir->name, name);
	
	dir->attributes = attributes;
	cluster = get_a_cluster();
	dir->high_cluster = (cluster&0xffff0000)>>16;
	dir->low_cluster = (cluster&0x0000ffff);
	dir->size = 0;
	//printk("create_dir_entry:name:%s cluster:%x\n",dir->name, cluster);
	//时间和日期操作
	
	dir->create_date = DATE_TO_DATA16(time.year, time.month, time.day);
	dir->create_time =  TIME_TO_DATA16(time.hour, time.minute, time.second);
	
	dir->modify_date = dir->create_date;
	dir->modify_time = dir->create_time;
	
	dir->access_date = dir->create_date;

}

void copy_dir_entry(struct dir_entry *det, struct dir_entry *src)
{
	memset(det, 0, sizeof(struct dir_entry));
	strcpy(det->name, src->name);

	det->attributes = src->attributes;
	det->high_cluster = src->high_cluster;
	det->low_cluster = src->low_cluster;
	det->size = src->size;
	
	//时间和日期操作
	det->create_date = src->create_date;
	det->create_time =  src->create_time;
	
	det->modify_date = src->modify_date;
	det->modify_time = src->modify_time;
	
	det->access_date = src->access_date;
	
	//memcpy(det, src,sizeof(struct dir_entry));
	//set new time and date
}

bool search_dir_entry(struct dir_entry *parent_dir,char *name, struct dir_entry *dir)
{
	uint32_t cluster = (parent_dir->high_cluster<<16)|parent_dir->low_cluster;
	
	
	uint32_t sector = cluster_to_lba(cluster);
	uint8_t *buffer = (uint8_t *)kmalloc(SECTOR_SIZE);
	
	uint32_t offset;
	//printk("1");
	while(cluster != CLUSTER_EOFS && cluster != CLUSTER_BAD){
		//a sector
		memset(buffer, 0, SECTOR_SIZE);
		block_read(sector,buffer,1);
		
		for(offset = 0; offset < 512; offset+=sizeof(struct dir_entry)){
			struct dir_entry *item = (struct dir_entry *)(buffer+offset);
			if((item->name[0] != 0x00)){
				if(strcmp(name, item->name) == 0){	//find
					//printk("file %sn", item->name);
					
					memcpy(dir, item, sizeof(struct dir_entry));				
					kfree(buffer);
					return true;
				}
			}
		}
		cluster = get_next_cluster(cluster);
		sector = cluster_to_lba(cluster);
	}
	kfree(buffer);
	return false;
}

bool sync_dir_entry(struct dir_entry* parent_dir, struct dir_entry* dir, void* io_buf)
{
	uint32_t cluster,sector;
	uint8_t *buf = io_buf;
	struct dir_entry *item;
	uint32_t offset;
	cluster = (parent_dir->high_cluster<<16)|parent_dir->low_cluster;
	sector = cluster_to_lba(cluster);
	
	//同步的时候来获得修改时间
	dir->modify_date = DATE_TO_DATA16(time.year, time.month, time.day);
	dir->modify_time =  TIME_TO_DATA16(time.hour, time.minute, time.second);
	//printk("parent:%s child:%s ",parent_dir->name, dir->name);
	
	//printk("cluster:%x sector:%d\n", cluster, sector);

	while(cluster != CLUSTER_EOFS && cluster != CLUSTER_BAD){
		memset(buf, 0, SECTOR_SIZE);
		block_read(sector, buf, 1);
		
		for(offset = 0; offset < 512; offset+=32){
			item = (struct dir_entry *)(&buf[offset]);
			

			if(!strcmp(item->name, dir->name)){
				//find old, they had same name
				
				memcpy(item, dir,sizeof(struct dir_entry));	//copy data
				block_write(sector, buf, 1);
				//printk("sync_dir_entry:child:%s parent:%s old dir\n",item->name, parent_dir->name);
				return true;
			}else if(item->name[0] != 0x00 &&
				item->low_cluster == dir->low_cluster && item->high_cluster == dir->high_cluster && 
			 	item->create_time == dir->create_time && item->create_date == dir->create_date){
				//item had name, item's cluster&date&time equ dir's cluster&date&time, taht is say, they are same but name differnt.
				memcpy(item, dir,sizeof(struct dir_entry));	//copy data
				block_write(sector, buf, 1);
				//printk("sync_dir_entry:child:%s parent:%s old dir\n",item->name, parent_dir->name);
				return true;
			}else if(item->name[0] == 0x00){
				//find a empty, we create a new dir entry
				memcpy(item, dir,sizeof(struct dir_entry));	//copy data
				block_write(sector, buf, 1);
				//printk("sync_dir_entry:child:%s parent:%s new dir\n",item->name, parent_dir->name);
				return true;
			}
		}
		cluster = get_next_cluster(cluster);
		sector = cluster_to_lba(cluster);
		
	}
	return false;
}

/*
we will empty a dir entry under parent
*/
bool empty_dir_entry(struct dir_entry* parent_dir, struct dir_entry* dir, void* io_buf)
{
	uint32_t cluster,sector;
	uint8_t *buf = io_buf;
	struct dir_entry *item;
	uint32_t offset;
	char name[DE_NAME_LEN];
	cluster = (parent_dir->high_cluster<<16)|parent_dir->low_cluster;
	sector = cluster_to_lba(cluster);
	while(cluster != CLUSTER_EOFS && cluster != CLUSTER_BAD){
		memset(buf, 0, SECTOR_SIZE);
		block_read(sector, buf, 1);
		for(offset = 0; offset < 512; offset+=32){
			item = (struct dir_entry *)(&buf[offset]);
			if(strcmp(item->name, dir->name) == 0){	//find the dir
				
				memset(name,0,DE_NAME_LEN);
				strcpy(name, item->name);

				memset(item, 0, sizeof(struct dir_entry));	//write a empty dir entry
				block_write(sector, buf, 1);
				//printk("empty_dir_entry: %s parent:%s\n", name, parent_dir->name);
				return true;
			}
		}
		cluster = get_next_cluster(cluster);
		sector = cluster_to_lba(cluster);
	}
	return false;
}

/*
realease all cluster and sector that the dir entry had used.
*/
void release_dir_entry(struct dir_entry *de, char *buffer)
{
	uint32_t sector, cluster, next_cluster;
	uint32_t cluster_in_item;
	
	next_cluster = 0;
	cluster = (de->high_cluster<<16)|de->low_cluster;	//get first cluster
	while(cluster != CLUSTER_EOFS && cluster != CLUSTER_FREE && cluster != CLUSTER_BAD && cluster != CLUSTER_RESERVE){
		// clean sector
		sector = cluster_to_lba(cluster);
		memset(buffer, 0, SECTOR_SIZE);						
		block_write(sector, buffer, 1);	
		//printk("clean a sector:%d cluster:%d\n", sector, cluster);
		cluster_in_item = read_fat_item(cluster); //read a item
		// has data , can do next step, or not, stop
		if(cluster_in_item != CLUSTER_EOFS && cluster_in_item != CLUSTER_FREE && cluster_in_item != CLUSTER_BAD && cluster_in_item != CLUSTER_RESERVE){		
			next_cluster = get_next_cluster(cluster);	//for next time			
			//printk("get new cluster:%x under:%x\n", next_cluster, cluster);
		}
		//free this cluster item
		free_a_cluster(cluster);
		cluster = next_cluster;
	}
}

void close_dir_entry(struct dir_entry *de)
{
	if(de == &root_dir || de == NULL){	//can't close root dir
		return;
	}
	kfree(de);
}

bool load_dir_entry(struct dir_entry *parent_dir, char *name, struct dir_entry *child_dir, void* io_buf)
{
	uint32_t cluster,sector;
	uint8_t *buf = (uint8_t *)io_buf;
	struct dir_entry *item;
	uint32_t offset;

	cluster = (parent_dir->high_cluster<<16)|parent_dir->low_cluster;
	sector = cluster_to_lba(cluster);
	
	//同步的时候来获得修改时间
	uint16_t access_date = DATE_TO_DATA16(time.year, time.month, time.day);
	
	while(cluster != CLUSTER_EOFS && cluster != CLUSTER_BAD){
		memset(buf, 0, SECTOR_SIZE);
		block_read(sector, buf, 1);
		for(offset = 0; offset < 512; offset+=32){
			item = (struct dir_entry *)(&buf[offset]);
			if(strcmp(item->name, name) == 0){	//find the dir
				//把访问日期写入要加载的文件
				item->access_date = access_date;
				//写入磁盘
				block_write(sector, buf, 1);
				
				memcpy(child_dir, item, sizeof(struct dir_entry));
				//memset(item, 0, sizeof(struct dir_entry));	//write a empty dir entry
				//block_write(sector, buf, 1);
				//printk("load_dir_entry: %s parent:%s\n", name, parent_dir->name);
				return true;
			}
		}
		cluster = get_next_cluster(cluster);
		sector = cluster_to_lba(cluster);
	}
	return false;
}

