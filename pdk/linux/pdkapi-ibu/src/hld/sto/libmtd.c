#include <hld_cfg.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <osal/osal.h>

#include <linux/types.h>
//#include <generated/compiler.h>
#include <mtd/mtd-user.h>
#include <hld/sto/sto_nand.h>
#include <hld/sto/nand_api.h>
#include "libmtd.h"

#if 1
#define LIBMTD_ERROR(args...)	do{}while(0)
#define LIBMTD_NOTICE(args...)	do{}while(0)
#define LIBMTD_DEBUG(args...)	do{}while(0)
#else
#include <api/libc/log.h>
#define LIBMTD_ERROR(args...)	amslog_error(LOG_STBINFO,args)
#define LIBMTD_NOTICE(args...)	amslog_notice(LOG_STBINFO,args)
#define LIBMTD_DEBUG(args...)	amslog_debug(LOG_STBINFO,args)
#endif

int mtd_getPartInfo(UINT32 part_idx, struct mtd_part_info *info)
{
	int ret;
	struct mtd_info_user info_user;
	
	ret = nand_ioctl(part_idx, GETMTDBASICINFO, (UINT32)&info_user);

	info->size = info_user.size;
	info->erasesize = info_user.erasesize;
	info->writesize = info_user.writesize;
	
	return ret;
}

int _mtd_read(UINT32 part_idx, UINT32 from, UINT32 len, UINT8 *buf)
{
	int ret;
	UINT32 blocksize;

	nand_lseek(part_idx, from, SEEK_SET);
	ret = nand_read(part_idx, buf, len);
	if(ret < 0)
		return -1;

	return ret;
}

int _mtd_write(UINT32 part_idx, UINT32 to, UINT32 len, UINT8 *buf)
{
	int ret;
	UINT32 blocksize;

	nand_lseek(part_idx, to, SEEK_SET);
	ret = nand_write(part_idx, buf, len);
	if(ret < 0)
		return -1;

	return ret;
}

int mtd_write_partition(UINT32 part_idx, UINT8 *buf, UINT32 size)
{
	struct mtd_part_info info;
	UINT32 to;
	UINT32 len;
	
	mtd_getPartInfo(part_idx, &info);

	to = 0;
	len = (info.size < size) ? info.size : size;

	return _mtd_write(part_idx, to, len, buf);
}

int mtd_read_partition(UINT32 part_idx, UINT8 *buf, UINT32 size)
{
	struct mtd_part_info info;
	UINT32 from;
	UINT32 len;

	mtd_getPartInfo(part_idx, &info);

	from = 0;
	len = (info.size < size) ? info.size : size;

	return _mtd_read(part_idx, from, len, buf);
}

/* erase a part */
INT32 mtd_erase_partition(UINT32 part_idx)
{
	//return nand_erase(part_idx, -1);
}

/* write data to block[block_idx] */	
 INT32 mtd_write_block(UINT32 part_idx,UINT32 block_idx, UINT8 *buf)
{
	struct mtd_part_info info;
	UINT32 offset;

	mtd_getPartInfo(part_idx, &info);

	offset = block_idx*info.erasesize;
	if (_mtd_write(part_idx, offset, info.erasesize, buf) != (INT32)info.erasesize)
	{
		LIBMTD_ERROR("save block %d fail\n",block_idx);
		return -1;
	}

	return 0;
}

/* read block[block_idx] to buf */	
INT32 mtd_read_block(UINT32 part_idx,UINT32 block_idx, UINT8 *buf)
{
	struct mtd_part_info info;
	UINT32 offset;

	mtd_getPartInfo(part_idx, &info);

	offset = block_idx*info.erasesize;
	if (_mtd_read(part_idx, offset, info.erasesize, buf) != (INT32)info.erasesize)
	{
		LIBMTD_ERROR("load block %d fail\n",block_idx);
		return -1;		
	}

	return 0;
}

/* erase block[block_idx] in part */
INT32 mtd_erase_block(UINT32 part_idx,UINT32 block_idx)
{
	//return nand_erase(part_idx, block_idx);	
}

/* write data to sector[sector_idx] of block[block_idx] */	
INT32 mtd_write_pages(UINT32 part_idx,UINT32 block_idx, UINT32 start_page_idx,UINT8 *buf, UINT32 page_num)
{
	struct mtd_part_info info;
	UINT32 offset;
	UINT32 sector_size;

	mtd_getPartInfo(part_idx, &info);

	offset = block_idx*info.erasesize+start_page_idx*info.writesize;
	sector_size = info.writesize*page_num;
	if (_mtd_write(part_idx, offset, sector_size, buf) != (INT32)sector_size)
	{
		LIBMTD_ERROR("save pages to block:%d page:%d num:%d fail\n",
			block_idx,start_page_idx,page_num);
		return -1;
	}
	
	return 0;	
}

/* write data to sector[sector_idx] of block[block_idx] */	
INT32 mtd_write_sector(UINT32 part_idx,UINT32 block_idx, UINT32 sector_idx,UINT8 *sector, UINT32 sector_size)
{
	struct mtd_part_info info;
	UINT32 offset;

	mtd_getPartInfo(part_idx, &info);

	offset = block_idx*info.erasesize+sector_idx*sector_size;
	if (_mtd_write(part_idx, offset, sector_size, sector) != (INT32)sector_size)
	{
		LIBMTD_ERROR("save sector to block %d sector %d fail\n",block_idx,sector_idx);
		return -1;
	}
	
	return 0;	
}

/* fetch data to sector[sector_idx] of block[block_idx] */	
INT32 mtd_read_pages(UINT32 part_idx,UINT32 block_idx, UINT32 start_page_idx,UINT8 *buf, UINT32 page_num)
{
	struct mtd_part_info info;
	UINT32 offset;
	UINT32 sector_size;

	mtd_getPartInfo(part_idx, &info);

	offset = block_idx*info.erasesize+start_page_idx*info.writesize;
	sector_size = info.writesize*page_num;
	
	if (_mtd_read(part_idx, offset, sector_size, buf) != (INT32)sector_size)
	{
		LIBMTD_ERROR("load pages from block:%d page:%d num:%d fail\n",
			block_idx,start_page_idx,page_num);
		
		return -1;		
	}

	return 0;	
}

/* fetch data to sector[sector_idx] of block[block_idx] */	
INT32 mtd_read_sector(UINT32 part_idx,UINT32 block_idx, UINT32 sector_idx,UINT8 *sector, UINT32 sector_size)
{
	struct mtd_part_info info;
	UINT32 offset;

	mtd_getPartInfo(part_idx, &info);

	offset = block_idx*info.erasesize+sector_idx*sector_size;
	if (_mtd_read(part_idx, offset, sector_size, sector) != (INT32)sector_size)
	{
		LIBMTD_ERROR("load sector from block %d sector %d fail\n",block_idx,sector_idx);
		return -1;		
	}

	return 0;	
}

