#ifndef _FATXE_FAT_H_
#define _FATXE_FAT_H_

#include <types.h>

#define CLUSTER_FREE 0
#define CLUSTER_BAD  0Xfffffff6	//Bad cluser
#define CLUSTER_RESERVE  0Xfffffff7	//Bad cluser
#define CLUSTER_EOFS 0Xfffffff8	//Start of eof cluster range
#define CLUSTER_EOFE 0Xffffffff	//End of eof cluster range

/*fat.c*/
void init_fat();

uint32_t get_a_cluster();
uint32_t free_a_cluster(uint32_t cluster);

uint32_t write_fat_item(uint32_t cluster, uint32_t data);
uint32_t read_fat_item(uint32_t cluster);


uint32_t clean_sector(uint32_t lba);
uint32_t cluster_to_lba(uint32_t cluster);
uint32_t lba_to_cluster(uint32_t lba);

uint32_t get_left_clusters(uint32_t cluster);
uint32_t get_next_cluster(uint32_t cluster);

#endif

