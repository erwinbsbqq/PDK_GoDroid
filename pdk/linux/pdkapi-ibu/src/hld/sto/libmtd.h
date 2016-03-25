#ifndef _LIB_MTD_H_
#define _LIB_MTD_H_

int mtd_write_partition(UINT32 part_idx, UINT8 *buf, UINT32 size);

int mtd_read_partition(UINT32 part_idx, UINT8 *buf, UINT32 size);

/* write data to block[block_idx] */	
 INT32 mtd_write_block(UINT32 part_idx,UINT32 block_idx, UINT8 *buf);

/* read block[block_idx] to buf */	
INT32 mtd_read_block(UINT32 part_idx,UINT32 block_idx, UINT8 *buf);

/* erase block[block_idx] in part */
INT32 mtd_erase_block(UINT32 part_idx,UINT32 block_idx);

/* write data to sector[sector_idx] of block[block_idx] */	
INT32 mtd_write_sector(UINT32 part_idx,UINT32 block_idx, UINT32 sector_idx,UINT8 *sector, UINT32 sector_size);

/* fetch data to sector[sector_idx] of block[block_idx] */	
INT32 mtd_read_sector(UINT32 part_idx,UINT32 block_idx, UINT32 sector_idx,UINT8 *sector, UINT32 sector_size);

#endif //_LIB_MTD_H_