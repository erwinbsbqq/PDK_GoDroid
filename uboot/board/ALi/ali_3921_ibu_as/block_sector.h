#ifndef BLOCK_SECTOR_H
#define BLOCK_SECTOR_H

int _sector_crc_check(unsigned char *sector, unsigned int sector_size);

int load_valid_block(unsigned int part_idx, unsigned char *buf, unsigned int *buf_len);

int save_block(unsigned int part_idx, unsigned char *buf, unsigned int buf_len);

#endif
