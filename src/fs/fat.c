/*
File:		fs/fat.c
Contains:	fatxe's fat manage
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



void init_fat()
{
	//Therr is no partiton
	fat_start_lba = dbr.bpb.reserved_sector;
	//clean_sector(fat_start_lba);
	fat_sectors = dbr.bpb.fat_sectors;
	data_start_lba = fat_start_lba + dbr.bpb.fats*fat_sectors;
	
	uint32_t *buffer = (uint32_t *)kmalloc(SECTOR_SIZE);
	if(buffer == NULL){
		return;
	}
	memset(buffer,0,SECTOR_SIZE);
	block_read(fat_start_lba, buffer, 1);
	buffer[0] = 0x0ffffff8;
	buffer[1] = 0x0fffffff;
	block_write(fat_start_lba, buffer, 1);
	printk("fat: fat lba:%d data start:%d\n",\
	fat_start_lba,\
	data_start_lba);
}

/*
*Get a fat cluster, if ok.return cluster, or not CLUSTER_EOFS;
*/
uint32_t get_a_cluster()
{
	uint32_t cluster = fsinfo.next_available_cluster;
	if(cluster < 2){
		return CLUSTER_EOFS;
	}
	//Set next available cluster;
	uint32_t *buffer = (uint32_t *)kmalloc(SECTOR_SIZE);
	if(buffer == NULL){
		return 0;
	}
	int i,cnt;
	for(cnt = 0; cnt < fat_sectors; cnt++){	//Loop fat_sectors
		block_read(fat_start_lba + cnt, buffer, 1);
		for(i = 0; i < SECTOR_SIZE/4; i++){
			if(buffer[i] == CLUSTER_FREE && i != cluster){ //Get a free fat item
				fsinfo.next_available_cluster = cnt*SECTOR_SIZE/4 + i;
				write_fat_item(cluster, CLUSTER_EOFS);	//For default, we will write the cluster with CLUSTER_EOFS
				sync_fsinfo();
				kfree(buffer);
				return cluster;
			}
		}
	}
	kfree(buffer);
	return CLUSTER_EOFS;
}

/*
*Free a fat cluster, if ok.return cluster, or not 0;
*/
uint32_t free_a_cluster(uint32_t cluster)
{
	if(cluster < 2){
		return CLUSTER_EOFS;
	}
	write_fat_item(cluster, CLUSTER_FREE);
	fsinfo.next_available_cluster = cluster;
	sync_fsinfo();
	
	return cluster;
}

/*
*Write a fat item, if ok.return cluster, or not 0;
*/
uint32_t write_fat_item(uint32_t cluster, uint32_t data)
{
	uint32_t sector;
	uint32_t offset;
	if(cluster < 2){
		return CLUSTER_EOFS;
	}
	offset = cluster/128;
	sector = fat_start_lba + offset;	//Get sector
	
	uint32_t *buffer = (uint32_t *)kmalloc(SECTOR_SIZE);
	if(buffer == NULL){
		return 0;
	}
	block_read(sector, buffer, 1);
	offset = cluster%128;
	buffer[offset] = data;
	block_write(sector, buffer, 1);
	//printk("write_fat_item:fat cluster:%d sector:%d offset:%d\n",cluster,sector,offset);
	kfree(buffer);
	return ((sector-fat_start_lba)*128+offset);
}

/*
*Read a fat item, if ok.return cluster, or not 0;
*/
uint32_t read_fat_item(uint32_t cluster)
{
	uint32_t sector;
	uint32_t offset;
	uint32_t data;
	if(cluster < 2){
		return CLUSTER_EOFS;
	}
	offset = cluster/128;
	sector = fat_start_lba + offset;	//Get sector
	
	uint32_t *buffer = (uint32_t *)kmalloc(SECTOR_SIZE);
	if(buffer == NULL){
		return 0;
	}
	block_read(sector, buffer, 1);
	offset = cluster%128;
	data = buffer[offset];
	kfree(buffer);
	return data;
}

/*
*Read a fat item, if ok.return cluster, or not 0;
*/
uint32_t clean_sector(uint32_t lba)
{
	uint32_t *buffer = (uint32_t *)kmalloc(SECTOR_SIZE);
	if(buffer == NULL){
		return 0;
	}
	memset(buffer,0,SECTOR_SIZE);
	block_write(lba, buffer, 1);
	kfree(buffer);
	return 1;
}

uint32_t cluster_to_lba(uint32_t cluster)
{
	uint32_t sector = data_start_lba + (cluster-2)*sector_per_cluster;
	return sector;
}

uint32_t lba_to_cluster(uint32_t lba)
{
	uint32_t cluster = (lba - data_start_lba)/sector_per_cluster + 2;
	return cluster;
}

uint8_t load_part_cluster(uint32_t cluster, uint32_t part, uint8_t* buffer)
{
	uint32_t sector;
	sector = data_start_lba + ((uint32_t)(cluster-2)) * sector_per_cluster;
	block_read(sector + part, buffer,1);
	return 1;
}

uint32_t get_next_cluster(uint32_t cluster)
{
	uint32_t sector;
	uint32_t offset;
	if(cluster < 2){
		return CLUSTER_EOFS;
	}
	uint32_t *buffer = (uint32_t *)kmalloc(SECTOR_SIZE);
	if(buffer == NULL){
		return CLUSTER_EOFS;
	}
	
	offset = cluster/128;
	sector = fat_start_lba + offset;	//Get sector
	memset(buffer, 0, SECTOR_SIZE);
	
	block_read(sector, buffer, 1);
	
	offset = cluster%128;
	
	sector = buffer[offset];	//get data
	//printk("get_next_cluster:cluster:%x next:%x\n",cluster, sector);
	kfree(buffer);
	return sector;	//return cluster
}

/*
get all clusters by a start cluster
*/
uint32_t get_left_clusters(uint32_t cluster)
{
	int i = 0;
	uint32_t clus;
	clus = get_next_cluster(cluster);
	while(clus != CLUSTER_EOFS && clus != CLUSTER_BAD && clus != CLUSTER_FREE){
		i++;
		clus = get_next_cluster(clus);
	}
	return i;
}
