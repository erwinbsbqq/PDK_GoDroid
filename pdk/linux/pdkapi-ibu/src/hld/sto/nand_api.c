#include <hld_cfg.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <hld/sto/nand_api.h>

static NAND_INFO *g_nand_info=NULL;

#if (SYS_OS_MODULE==LINUX_2_6_28)
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libmtd.h"

#if 1
#define NANDAPI_ERROR(args...)	do{}while(0)
#define NANDAPI_NOTICE(args...)	do{}while(0)
#define NANDAPI_DEBUG(args...)	do{}while(0)
#define NANDAPI_PRINTF(args...)	do{}while(0)
#define NANDAPI_DUMP(addr, len)	do{}while(0)
#else

#include <api/libc/log.h>
#define NANDAPI_ERROR(args...)	amslog_error(LOG_STBINFO,args)
#define NANDAPI_NOTICE(args...)	amslog_notice(LOG_STBINFO,args)
#define NANDAPI_DEBUG(args...)	amslog_debug(LOG_STBINFO,args)
#define NANDAPI_PRINTF(args...)	amslog_printf(LOG_STBINFO,args)
#define NANDAPI_DUMP(addr, len)	amslog_dump(LOG_STBINFO,addr, len)

#endif

UINT32 NF_align_to_page(UINT32 part_idx, UINT32 size)
{
	struct mtd_part_info info;
	UINT32 alignsize;
	UINT32 pagesize;

	if (nand_open(part_idx, O_RDONLY) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}
	
	mtd_getPartInfo(part_idx, &info);
	pagesize = info.writesize;
	alignsize = (size+pagesize-1)/pagesize*pagesize;
	return alignsize;
}

INT32 NF_get_part_info(UINT32 part_idx, struct mtd_part_info *info)
{
	if (nand_open(part_idx, O_RDONLY) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	return mtd_getPartInfo(part_idx, info);
}

NAND_INFO * NF_get_nand_info(void)
{
	struct mtd_part_info info;
	UINT32 alignsize;
	UINT32 pagesize;
		
	if (g_nand_info != NULL)
	{
		goto RETURN;
	}

	NF_get_part_info(0, &info);

	g_nand_info = (NAND_INFO *)malloc(sizeof(NAND_INFO));

	g_nand_info->erasesize = info.erasesize;
	g_nand_info->writesize = info.writesize;
	g_nand_info->size = info.size;
	g_nand_info->startAddr = 0;

RETURN:	
	return g_nand_info;
}

/* write data to part[part_idx] */	
 INT32 NF_save_partition(UINT32 part_idx, UINT8 *buf, UINT32 size)
{
	if (nand_open(part_idx, O_RDWR) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	return mtd_write_partition(part_idx, buf, size);	
}

 INT32 NF_erase_partition(UINT32 part_idx)
{
	if (nand_open(part_idx, O_RDWR) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	return mtd_erase_partition(part_idx);
}

/* read part[part_idx] to buf */	
INT32 NF_load_partition(UINT32 part_idx, UINT8 *buf, UINT32 size)
{
	if (nand_open(part_idx, O_RDONLY) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	return mtd_read_partition(part_idx, buf, size);	
}

/* write data to block[block_idx] */	
INT32 NF_save_block(UINT32 part_idx,UINT32 block_idx, UINT8 *buf)
{
	if (nand_open(part_idx, O_RDWR) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	return mtd_write_block(part_idx, block_idx, buf);	
}

/* read block[block_idx] to buf */	
INT32 NF_load_block(UINT32 part_idx,UINT32 block_idx, UINT8 *buf)
{
	if (nand_open(part_idx, O_RDONLY) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	return mtd_read_block(part_idx, block_idx, buf);	
}

/* erase block[block_idx] in part */
INT32 NF_erase_block(UINT32 part_idx,UINT32 block_idx)
{
	if (nand_open(part_idx, O_RDWR) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	return mtd_erase_block(part_idx, block_idx);	
}

/* write data to page[page_idx] of block[block_idx] */	
INT32 NF_write_page(UINT32 part_idx,UINT32 block_idx, UINT32 page_idx,UINT8 *buf)
{
	if (nand_open(part_idx, O_RDWR) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	return mtd_write_pages(part_idx,block_idx, page_idx,buf, 1);
}

/* read page[page_idx] of block[block_idx] to buf */	
INT32 NF_read_page(UINT32 part_idx,UINT32 block_idx, UINT32 page_idx,UINT8 *buf)
{
	if (nand_open(part_idx, O_RDONLY) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	return mtd_read_pages(part_idx,block_idx, page_idx,buf, 1);
}

/* write data to sector[sector_idx] of block[block_idx] */	
INT32 NF_write_sector(UINT32 part_idx,UINT32 block_idx, UINT32 sector_idx,UINT8 *sector, UINT32 sector_size)
{
	struct mtd_part_info info;
	UINT32 page_idx;
	UINT32 page_num;

	if (nand_open(part_idx, O_RDWR) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	mtd_getPartInfo(part_idx, &info);

	page_num = sector_size/info.writesize;
	page_idx = sector_idx*page_num;
		
	return mtd_write_pages(part_idx,block_idx, page_idx,sector, page_num);
	
}

/* fetch data to sector[sector_idx] of block[block_idx] */	
INT32 NF_read_sector(UINT32 part_idx,UINT32 block_idx, UINT32 sector_idx,UINT8 *sector, UINT32 sector_size)
{
	struct mtd_part_info info;
	UINT32 page_idx;
	UINT32 page_num;

	if (nand_open(part_idx, O_RDONLY) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	mtd_getPartInfo(part_idx, &info);

	page_num = sector_size/info.writesize;
	page_idx = sector_idx*page_num;
		
	return mtd_read_pages(part_idx,block_idx, page_idx,sector, page_num);
}

/* write data to part[part_idx] without erase */	
INT32 NF_write_part(UINT32 part_idx,UINT32 to, UINT32 len,UINT8 *buf)
{
	INT32 ret=0;
	struct mtd_part_info info;	
	UINT32 pagesize;
	UINT32 unaligned_off;
	UINT32 start_addr;
	UINT32 oplen;
	UINT32 buf_len = 0;
	UINT8 *pageBuf=0;
	INT32 write_len;

	if (nand_open(part_idx, O_RDWR) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	mtd_getPartInfo(part_idx, &info);
	pagesize = info.writesize;

	pageBuf = (UINT8*)malloc(pagesize);
	write_len = 0;
		
	/* if to addr is not aligned to pagesize */
	unaligned_off = to%pagesize;
	if (unaligned_off)
	{
		start_addr = to/pagesize*pagesize;
		oplen = pagesize;

		NANDAPI_DEBUG("to addr is not aligned to pagesize\n");

		memset(pageBuf, 0xFF, pagesize);
		
		/* copy write data to page buf */
		if (len < pagesize-unaligned_off)
			buf_len = len;
		else
			buf_len = pagesize-unaligned_off;

		NANDAPI_DEBUG("copy buf len:0x%x to page buffer:0x%x\n", buf_len, unaligned_off);
		memcpy(pageBuf+unaligned_off, buf, buf_len);
		
		/* write the page */
		ret = _mtd_write(part_idx, start_addr, oplen, pageBuf);
		if (ret<0)
		{
			goto RETURN;
		}
		write_len += buf_len;
		
	}

	if (buf_len >= len)
	{
		NANDAPI_DEBUG("write ok\n");	
		goto RETURN;
	}

	/* write aligned buf */
	start_addr = (to+pagesize-1)/pagesize*pagesize;
	oplen = (len - buf_len)/pagesize*pagesize;	
	if (oplen > 0)
	{		
		NANDAPI_DEBUG("write alinged buf addr:0x%x, len:0x%x\n",start_addr, oplen);

		/* write the block */
		ret = _mtd_write(part_idx, start_addr, oplen, buf+buf_len);
		if (ret<0)
		{
			goto RETURN;
		}				
		write_len += ret;

		start_addr += oplen;
		buf_len += oplen;
	}

	/* if rest len is not aligned to pagesize */
	if (buf_len < len)
	{
		oplen = pagesize;	

		NANDAPI_DEBUG("rest len is not aligned to pagesize\n");

		memset(pageBuf, 0xFF, pagesize);

		NANDAPI_DEBUG("copy buf:0x%x, len:0x%x to page buffer\n", buf_len, len-buf_len);
		memcpy(pageBuf, buf+buf_len, len-buf_len);
		
		/* write the block */
		ret = _mtd_write(part_idx, start_addr, oplen, pageBuf);
		if (ret<0)
		{
			goto RETURN;
		}			
		write_len += len-buf_len;		
	}

RETURN:
#if 0	
	if(nand_close(part_idx)<0)
	{
		NANDAPI_ERROR("close mtd%d fail\n", part_idx);
	}
#endif

	if (pageBuf)
		free(pageBuf);
	
	if (ret<0)
		return ret;

	NANDAPI_DEBUG("write without erase ok, ask len:0x%x, write len:0x%x\n",len, write_len);

	return write_len;
}

/* write data to part[part_idx] with erase */	
INT32 NF_write_part_erase(UINT32 part_idx,UINT32 to, UINT32 len,UINT8 *buf)
{
	INT32 ret=0;
	struct mtd_part_info info;	
	UINT32 blocksize;
	UINT32 unaligned_off;
	UINT32 start_addr;
	UINT32 oplen, rest_len;
	UINT32 buf_len = 0;
	UINT8 *blockBuf=0;
	INT32 write_len;

	if (nand_open(part_idx, O_RDWR) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	mtd_getPartInfo(part_idx, &info);
	blocksize = info.erasesize;

	blockBuf = (UINT8*)malloc(blocksize);
	write_len = 0;
	
	/* if to addr is not aligned to blocksize */
	unaligned_off = to%blocksize;
	if (unaligned_off)
	{
		/* read a block data first */
		start_addr = to/blocksize*blocksize;
		oplen = blocksize;

		NANDAPI_DEBUG("to addr is not aligned to blocksize\n");
		
		ret = _mtd_read(part_idx, start_addr, oplen, blockBuf);
		if (ret<0)
		{
			goto RETURN;
		}

		/* copy write data to block buf */
		if (len < blocksize-unaligned_off)
			buf_len = len;
		else
			buf_len = blocksize-unaligned_off;

		NANDAPI_DEBUG("copy buf len:0x%x to block buffer:0x%x\n", buf_len, unaligned_off);
		memcpy(blockBuf+unaligned_off, buf, buf_len);

		/* erase the block */
		ret = mtd_erase_block(part_idx, start_addr/blocksize);
		if (ret<0)
		{
			goto RETURN;
		}

		/* write the block */
		ret = _mtd_write(part_idx, start_addr, oplen, blockBuf);
		if (ret<0)
		{
			goto RETURN;
		}
		write_len += buf_len;
	}

	if (buf_len >= len)
	{
		NANDAPI_DEBUG("write with erase ok\n");	
		goto RETURN;
	}

	/* write aligned buf */
	start_addr = (to+blocksize-1)/blocksize*blocksize;
	rest_len = (len - buf_len)/blocksize*blocksize;	
	NANDAPI_DEBUG("write alinged buf addr:0x%x, len:0x%x\n",start_addr, rest_len);
	while (rest_len > 0)
	{		
		/* erase the block */
		ret = mtd_erase_block(part_idx, start_addr/blocksize);
		if (ret<0)
		{
			goto RETURN;
		}

		/* write the block */
		ret = _mtd_write(part_idx, start_addr, blocksize, buf+buf_len);
		if (ret<0)
		{
			goto RETURN;
		}				
		write_len += ret;
			
		start_addr += blocksize;
		buf_len += blocksize;
		rest_len -= blocksize;
	}

	/* if rest len is not aligned to blocksize */
	if (buf_len < len)
	{
		oplen = blocksize;	

		NANDAPI_DEBUG("rest len is not aligned to blocksize\n");

		/* read a block data first */
		ret = _mtd_read(part_idx, start_addr, oplen, blockBuf);
		if (ret<0)
		{
			goto RETURN;
		}

		NANDAPI_DEBUG("copy buf:0x%x, len:0x%x to block buffer\n", buf_len, len-buf_len);
		memcpy(blockBuf, buf+buf_len, len-buf_len);

		/* erase the block */
		ret = mtd_erase_block(part_idx, start_addr/blocksize);
		if (ret<0)
		{
			goto RETURN;
		}

		/* write the block */
		ret = _mtd_write(part_idx, start_addr, oplen, blockBuf);
		if (ret<0)
		{
			goto RETURN;
		}				
		write_len += len-buf_len;
	}

RETURN:
#if 0		
	if(nand_close(part_idx)<0)
	{
		NANDAPI_ERROR("close mtd%d fail\n", part_idx);
	}
#endif

	if (blockBuf)
		free(blockBuf);
	
	if (ret<0)
		return ret;

	NANDAPI_DEBUG("write with erase ok, ask len:0x%x, write len:0x%x\n",len, write_len);

	return write_len;
}

/* read data from part[part_idx] */	
INT32 NF_read_part(UINT32 part_idx,UINT32 from, UINT32 len,UINT8 *buf)
{
	INT32 ret=0;
	struct mtd_part_info info;	
	UINT32 pagesize;
	UINT32 unaligned_off;
	UINT32 start_addr;
	UINT32 oplen;
	UINT32 buf_len = 0;
	UINT8 *pageBuf=0;
	INT32 read_len;

	NANDAPI_DEBUG("read part%d from:0x%x, len:0x%x, buf:0x%x\n", 
		part_idx, from, len, buf);

	if (nand_open(part_idx, O_RDONLY) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}
	
	mtd_getPartInfo(part_idx, &info);
	pagesize = info.writesize;

	pageBuf = (UINT8*)malloc(pagesize);

	read_len = 0;
	
	/* if from addr is not aligned to pagesize */
	unaligned_off = from%pagesize;

	if (unaligned_off)
	{
		start_addr = from/pagesize*pagesize;
		oplen = pagesize;

		NANDAPI_DEBUG("from addr is not aligned to pagesize\n");
		
		ret = _mtd_read(part_idx, start_addr, oplen, pageBuf);
		if (ret<0)
		{
			goto RETURN;
		}

		if (len < pagesize-unaligned_off)
			buf_len = len;
		else
			buf_len = pagesize-unaligned_off;

		read_len += buf_len;

		NANDAPI_DEBUG("copy page buf off:0x%x, len:0x%x to buffer\n", unaligned_off,buf_len);
		memcpy(buf, pageBuf+unaligned_off, buf_len);
	}

	if (buf_len >= len)
	{
		NANDAPI_DEBUG("read ok\n");	
		goto RETURN;
	}

	/* read aligned buf */
	start_addr = (from+pagesize-1)/pagesize*pagesize;
	oplen = (len - buf_len)/pagesize*pagesize;
	if (oplen>0)
	{

		NANDAPI_DEBUG("read alinged buf addr:0x%x, len:0x%x\n",start_addr, oplen);
		
		ret = _mtd_read(part_idx, start_addr, oplen, buf+buf_len);
		if (ret<0)
		{
			goto RETURN;
		}
		read_len += ret;
		buf_len += oplen;
	}

	/* if rest len is not aligned to pagesize */
	if (buf_len < len)
	{
		start_addr += oplen;
		oplen = pagesize;	

		NANDAPI_DEBUG("rest len is not aligned to pagesize\n");
		
		ret = _mtd_read(part_idx, start_addr, oplen, pageBuf);
		if (ret<0)
		{
			goto RETURN;
		}
		read_len += len-buf_len;

		NANDAPI_DEBUG("copy page buf len:0x%x to buffer[0x%x]\n", len-buf_len, buf_len);

		memcpy(buf+buf_len, pageBuf, len-buf_len);
	}
	
RETURN:
#if 0	
	if(nand_close(part_idx)<0)
	{
		NANDAPI_ERROR("close mtd%d fail\n", part_idx);
	}
#endif

	if (pageBuf)
		free(pageBuf);

	if (ret<0)
		return ret;

	NANDAPI_DEBUG("read ok, ask len:0x%x, read len:0x%x\n",len, read_len);

	return read_len;
}

/* erase physical block[block_idx] in part */
INT32 NF_erase_block_physical(UINT32 part_idx,UINT32 block_idx)
{
	NANDAPI_DEBUG("phyerase part[%d] block[%d]\n",part_idx, block_idx);
		
	if (nand_open(part_idx, O_RDWR) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	return nand_physical_erase(part_idx, block_idx);	
}

/* read data from part[part_idx] physical address*/	
INT32 NF_read_part_physical(UINT32 part_idx,UINT32 from, UINT32 len,UINT8 *buf)
{
	INT32 ret=0;
	struct mtd_part_info info;	
	UINT32 pagesize;
	UINT32 unaligned_off;
	UINT32 start_addr;
	UINT32 oplen;
	UINT32 buf_len = 0;
	UINT8 *pageBuf=0;
	INT32 read_len;

	NANDAPI_DEBUG("read part%d from:0x%x, len:0x%x, buf:0x%x\n", 
		part_idx, from, len, buf);

	if (nand_open(part_idx, O_RDONLY) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}
	
	mtd_getPartInfo(part_idx, &info);

	if (from+len > info.size)
	{
		NANDAPI_ERROR("read space[0x%x-0x%x] overflow mtd%d space\n",
			from,from+len,part_idx);
		return -1;
	}
		
	pagesize = info.writesize;

	pageBuf = (UINT8*)malloc(pagesize);

	read_len = 0;
	
	/* if from addr is not aligned to pagesize */
	unaligned_off = from%pagesize;

	if (unaligned_off)
	{
		start_addr = from/pagesize*pagesize;
		oplen = pagesize;

		NANDAPI_DEBUG("from addr is not aligned to pagesize\n");
		
		ret = nand_physical_read(part_idx, pageBuf, start_addr, oplen);
		if (ret<0)
		{
			goto RETURN;
		}

		if (len < pagesize-unaligned_off)
			buf_len = len;
		else
			buf_len = pagesize-unaligned_off;

		read_len += buf_len;

		NANDAPI_DEBUG("copy page buf off:0x%x, len:0x%x to buffer\n", unaligned_off,buf_len);
		memcpy(buf, pageBuf+unaligned_off, buf_len);
	}

	if (buf_len >= len)
	{
		NANDAPI_DEBUG("read ok\n");	
		goto RETURN;
	}

	/* read aligned buf */
	start_addr = (from+pagesize-1)/pagesize*pagesize;
	oplen = (len - buf_len)/pagesize*pagesize;
	if (oplen>0)
	{

		NANDAPI_DEBUG("read alinged buf addr:0x%x, len:0x%x\n",start_addr, oplen);

		ret = nand_physical_read(part_idx, buf+buf_len, start_addr, oplen);
		if (ret<0)
		{
			goto RETURN;
		}
		read_len += ret;
		buf_len += oplen;
	}

	/* if rest len is not aligned to pagesize */
	if (buf_len < len)
	{
		start_addr += oplen;
		oplen = pagesize;	

		NANDAPI_DEBUG("rest len is not aligned to pagesize\n");

		ret = nand_physical_read(part_idx, pageBuf, start_addr, oplen);		
		if (ret<0)
		{
			goto RETURN;
		}
		read_len += len-buf_len;

		NANDAPI_DEBUG("copy page buf len:0x%x to buffer[0x%x]\n", len-buf_len, buf_len);

		memcpy(buf+buf_len, pageBuf, len-buf_len);
	}
	
RETURN:
	if (pageBuf)
		free(pageBuf);

	if (ret<0)
		return ret;

	NANDAPI_DEBUG("phy read ok, ask len:0x%x, read len:0x%x\n",len, read_len);

	return read_len;
}

/* wtire data to part[part_idx] physical address without erase*/	
INT32 NF_write_part_physical(UINT32 part_idx,UINT32 to, UINT32 len,UINT8 *buf)
{
	INT32 ret=0;
	struct mtd_part_info info;	
	UINT32 pagesize;
	UINT32 unaligned_off;
	UINT32 start_addr;
	UINT32 oplen;
	UINT32 buf_len = 0;
	UINT8 *pageBuf=0;
	INT32 write_len;

	NANDAPI_DEBUG("phywrite part%d to:0x%x, len:0x%x, buf:0x%x\n", 
		part_idx, to, len, buf);

	if (nand_open(part_idx, O_RDWR) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	mtd_getPartInfo(part_idx, &info);
	pagesize = info.writesize;

	pageBuf = (UINT8*)malloc(pagesize);
	write_len = 0;
		
	/* if to addr is not aligned to pagesize */
	unaligned_off = to%pagesize;
	if (unaligned_off)
	{
		start_addr = to/pagesize*pagesize;
		oplen = pagesize;

		NANDAPI_DEBUG("to addr is not aligned to pagesize\n");

		memset(pageBuf, 0xFF, pagesize);
		
		/* copy write data to page buf */
		if (len < pagesize-unaligned_off)
			buf_len = len;
		else
			buf_len = pagesize-unaligned_off;

		NANDAPI_DEBUG("copy buf len:0x%x to page buffer:0x%x\n", buf_len, unaligned_off);
		memcpy(pageBuf+unaligned_off, buf, buf_len);
		
		/* write the page */
		ret = nand_physical_write(part_idx, pageBuf, start_addr, oplen);		
		if (ret<0)
		{
			goto RETURN;
		}
		write_len += buf_len;
		
	}

	if (buf_len >= len)
	{
		NANDAPI_DEBUG("write ok\n");	
		goto RETURN;
	}

	/* write aligned buf */
	start_addr = (to+pagesize-1)/pagesize*pagesize;
	oplen = (len - buf_len)/pagesize*pagesize;	
	if (oplen > 0)
	{		
		NANDAPI_DEBUG("write alinged buf addr:0x%x, len:0x%x\n",start_addr, oplen);

		/* write the block */
		ret = nand_physical_write(part_idx, buf+buf_len, start_addr, oplen);				
		if (ret<0)
		{
			goto RETURN;
		}				
		write_len += ret;

		start_addr += oplen;
		buf_len += oplen;
	}

	/* if rest len is not aligned to pagesize */
	if (buf_len < len)
	{
		oplen = pagesize;	

		NANDAPI_DEBUG("rest len is not aligned to pagesize\n");

		memset(pageBuf, 0xFF, pagesize);

		NANDAPI_DEBUG("copy buf:0x%x, len:0x%x to page buffer\n", buf_len, len-buf_len);
		memcpy(pageBuf, buf+buf_len, len-buf_len);
		
		/* write the block */
		ret = nand_physical_write(part_idx, pageBuf, start_addr, oplen);						
		if (ret<0)
		{
			goto RETURN;
		}			
		write_len += len-buf_len;		
	}

RETURN:
#if 0	
	if(nand_close(part_idx)<0)
	{
		NANDAPI_ERROR("close mtd%d fail\n", part_idx);
	}
#endif

	if (pageBuf)
		free(pageBuf);
	
	if (ret<0)
		return ret;

	//NANDAPI_DEBUG("write without erase ok, ask len:0x%x, write len:0x%x\n",len, write_len);

	return write_len;
}

/* wtire data to part[part_idx] physical address with erase*/	
INT32 NF_write_part_physical_erase(UINT32 part_idx,UINT32 to, UINT32 len,UINT8 *buf)
{
	INT32 ret=0;
	struct mtd_part_info info;	
	UINT32 blocksize;
	UINT32 unaligned_off;
	UINT32 start_addr;
	UINT32 oplen, rest_len;
	UINT32 buf_len = 0;
	UINT8 *blockBuf=0;
	INT32 write_len;

	if (nand_open(part_idx, O_RDWR) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	mtd_getPartInfo(part_idx, &info);

	if (to+len > info.size)
	{
		NANDAPI_ERROR("write space[0x%x-0x%x] overflow mtd%d space\n",
			to,to+len,part_idx);
		return -1;
	}
	
	blocksize = info.erasesize;

	blockBuf = (UINT8*)malloc(blocksize);
	write_len = 0;
	
	/* if to addr is not aligned to blocksize */
	unaligned_off = to%blocksize;
	if (unaligned_off)
	{
		/* read a block data first */
		start_addr = to/blocksize*blocksize;
		oplen = blocksize;

		NANDAPI_DEBUG("to addr is not aligned to blocksize\n");
		
		ret = nand_physical_read(part_idx, blockBuf, start_addr, oplen);
		if (ret<0)
		{
			goto RETURN;
		}

		/* copy write data to block buf */
		if (len < blocksize-unaligned_off)
			buf_len = len;
		else
			buf_len = blocksize-unaligned_off;

		NANDAPI_DEBUG("copy buf len:0x%x to block buffer:0x%x\n", buf_len, unaligned_off);
		memcpy(blockBuf+unaligned_off, buf, buf_len);

		/* erase the block */
		ret = nand_physical_erase(part_idx, start_addr/blocksize);
		if (ret<0)
		{
			goto RETURN;
		}

		/* write the block */
		ret = nand_physical_write(part_idx, blockBuf, start_addr, oplen);
		if (ret<0)
		{
			goto RETURN;
		}
		write_len += buf_len;
	}

	if (buf_len >= len)
	{
		NANDAPI_DEBUG("write with erase ok\n");	
		goto RETURN;
	}

	/* write aligned buf */
	start_addr = (to+blocksize-1)/blocksize*blocksize;
	rest_len = (len - buf_len)/blocksize*blocksize;	
	NANDAPI_DEBUG("write alinged buf addr:0x%x, len:0x%x\n",start_addr, rest_len);
	while (rest_len > 0)
	{		
		/* erase the block */
		ret = nand_physical_erase(part_idx, start_addr/blocksize);
		if (ret<0)
		{
			goto RETURN;
		}

		/* write the block */
		ret = nand_physical_write(part_idx, buf+buf_len, start_addr, blocksize);
		if (ret<0)
		{
			goto RETURN;
		}				
		write_len += ret;
			
		start_addr += blocksize;
		buf_len += blocksize;
		rest_len -= blocksize;
	}

	/* if rest len is not aligned to blocksize */
	if (buf_len < len)
	{
		oplen = blocksize;	

		NANDAPI_DEBUG("rest len is not aligned to blocksize\n");

		/* read a block data first */
		ret = nand_physical_read(part_idx, blockBuf, start_addr, oplen);
		if (ret<0)
		{
			goto RETURN;
		}

		NANDAPI_DEBUG("copy buf:0x%x, len:0x%x to block buffer\n", buf_len, len-buf_len);
		memcpy(blockBuf, buf+buf_len, len-buf_len);

		/* erase the block */
		ret = nand_physical_erase(part_idx, start_addr/blocksize);
		if (ret<0)
		{
			goto RETURN;
		}

		/* write the block */
		ret = nand_physical_write(part_idx, buf+buf_len, start_addr, blocksize);		
		if (ret<0)
		{
			goto RETURN;
		}				
		write_len += len-buf_len;
	}

RETURN:
#if 0		
	if(nand_close(part_idx)<0)
	{
		NANDAPI_ERROR("close mtd%d fail\n", part_idx);
	}
#endif

	if (blockBuf)
		free(blockBuf);
	
	if (ret<0)
		return ret;

	NANDAPI_DEBUG("write with erase ok, ask len:0x%x, write len:0x%x\n",len, write_len);

	return write_len;
}

int NF_get_part_valid_len(UINT32 part_idx)
{
	if (nand_open(part_idx, O_RDONLY) < 0)
	{
		NANDAPI_ERROR("nand_open mtd%d fail\n",part_idx);
		return -1;
	}

	return nand_len(part_idx);
}

#else
#define NANDAPI_ERROR	printf
#define NANDAPI_NOTICE	PRINTF//printf
#define NANDAPI_DEBUG	PRINTF//printf

static struct ali_nand_device *nand_dev;

INT32 NF_nand_dev_init(struct ali_nand_device *dev)
{
	nand_dev = dev;
}

NAND_INFO * NF_get_nand_info(void)
{
	struct ali_nand_info info;
	UINT32 alignsize;
	UINT32 pagesize;
		
	if (g_nand_info != NULL)
	{
		goto RETURN;
	}

	NF_GetNandInfo(nand_dev, &info);

	g_nand_info = (NAND_INFO *)malloc(sizeof(NAND_INFO));

	g_nand_info->erasesize = info.blocksize;
	g_nand_info->writesize = info.pagesize;
	g_nand_info->size = info.blocksize * info.blockPerChip;
	g_nand_info->startAddr = info.startAddr;

	NANDAPI_DEBUG("startAddr:0x%08x,size:0x%08x\n ",
		g_nand_info->startAddr,g_nand_info->size);
	
RETURN:	
	return g_nand_info;
}

UINT32 NF_align_to_page(UINT32 part_idx, UINT32 size)
{
	struct ali_nand_info info;
	UINT32 alignsize;
	UINT32 pagesize;
	
	NF_GetNandInfo(nand_dev, &info);
	pagesize = info.pagesize;
	alignsize = (size+pagesize-1)/pagesize*pagesize;
	return alignsize;
}

INT32 NF_get_part_info(UINT32 part_idx, struct mtd_part_info *info)
{
	struct ali_nand_info nand_info;

	NF_GetNandInfo(nand_dev, &nand_info);

	if (part_idx >= nand_info.partitionNum)
		return -1;
	
	info->size = nand_info.partInfo[part_idx].len;
	info->erasesize = nand_info.blocksize;
	info->writesize = nand_info.pagesize;
	
	return 0;
}

/* write data to part[part_idx] */	
 INT32 NF_save_partition(UINT32 part_idx, UINT8 *buf, UINT32 size)
{
	return NF_SavePartition (nand_dev, part_idx, buf, size);
}

/* read part[part_idx] to buf */	
INT32 NF_load_partition(UINT32 part_idx, UINT8 *buf, UINT32 size)
{
	return NF_LoadPartition (nand_dev, part_idx, buf, size);
}

/* write data to block[block_idx] */	
 INT32 NF_save_block(UINT32 part_idx,UINT32 block_idx, UINT8 *buf)
{	
	return NF_write_block (nand_dev, part_idx, block_idx, buf);
}

/* read block[block_idx] to buf */	
INT32 NF_load_block(UINT32 part_idx,UINT32 block_idx, UINT8 *buf)
{	
	return NF_read_block(nand_dev, part_idx, block_idx, buf);
}

/* erase block[block_idx] in part */
INT32 NF_erase_block(UINT32 part_idx,UINT32 block_idx)
{
	return NF_erase_block_ext (nand_dev, part_idx, block_idx);
}

/* write data to page[page_idx] of block[block_idx] */	
INT32 NF_write_page(UINT32 part_idx,UINT32 block_idx, UINT32 page_idx,UINT8 *buf)
{
	struct ali_nand_info info;
	UINT32 byte_offset;

	NF_GetNandInfo(nand_dev, &info);

	byte_offset = block_idx*info.blocksize+page_idx*info.pagesize;

	return NF_SavePartition_ext (nand_dev, part_idx,byte_offset,info.pagesize, buf);	
}

/* read page[page_idx] of block[block_idx] to buf */	
INT32 NF_read_page(UINT32 part_idx,UINT32 block_idx, UINT32 page_idx,UINT8 *buf)
{
	struct ali_nand_info info;
	UINT32 byte_offset;

	NF_GetNandInfo(nand_dev, &info);

	byte_offset = block_idx*info.blocksize+page_idx*info.pagesize;

	return NF_LoadPartition_ext (nand_dev, part_idx,byte_offset,info.pagesize, buf);	
}

/* write data to sector[sector_idx] of block[block_idx] */	
INT32 NF_write_sector(UINT32 part_idx,UINT32 block_idx, UINT32 sector_idx,UINT8 *sector, UINT32 sector_size)
{
	struct ali_nand_info info;
	UINT32 byte_offset;

	NF_GetNandInfo(nand_dev, &info);

	byte_offset = block_idx*info.blocksize+sector_idx*sector_size;

	return NF_SavePartition_ext (nand_dev, part_idx,byte_offset,sector_size, sector);	
}

/* fetch data to sector[sector_idx] of block[block_idx] */	
INT32 NF_read_sector(UINT32 part_idx,UINT32 block_idx, UINT32 sector_idx,UINT8 *sector, UINT32 sector_size)
{
	struct ali_nand_info info;
	UINT32 byte_offset;

	NF_GetNandInfo(nand_dev, &info);

	byte_offset = block_idx*info.blocksize+sector_idx*sector_size;

	return NF_LoadPartition_ext (nand_dev, part_idx,byte_offset,sector_size, sector);	
}

/* write data to part[part_idx] */	
INT32 NF_write_part(UINT32 part_idx,UINT32 to, UINT32 len,UINT8 *buf)
{
	return NF_SavePartition_ext (nand_dev, part_idx,to,len, buf);	
}

/* write data to part[part_idx] */	
INT32 NF_write_part_erase(UINT32 part_idx,UINT32 to, UINT32 len,UINT8 *buf)
{
	return NF_SavePartition_ext (nand_dev, part_idx,to,len, buf);	
}

/* read data from part[part_idx] */	
INT32 NF_read_part(UINT32 part_idx,UINT32 from, UINT32 len,UINT8 *buf)
{
	return NF_LoadPartition_ext (nand_dev, part_idx,from,len, buf);	
}

/* read data from part[part_idx] physical address*/	
INT32 NF_read_part_physical(UINT32 part_idx,UINT32 from, UINT32 len,UINT8 *buf)
{
	INT32 ret=0;
	struct ali_nand_info info;
	UINT32 pagesize, blocksize;
	UINT32 unaligned_off;
	UINT32 start_addr;
	UINT32 oplen;
	UINT32 buf_len = 0;
	UINT8 *pageBuf=0;
	UINT32 lba_per_block,lba_per_page;
	UINT32 block_no;
	UINT32 start_lba,num_lba;

	NANDAPI_DEBUG("read part%d from:0x%x, len:0x%x, buf:0x%x\n", 
		part_idx, from, len, buf);

	NF_GetNandInfo(nand_dev, &info);
	pagesize = info.pagesize;
	blocksize = info.blocksize;

	lba_per_block = blocksize >> 9;
	lba_per_page = pagesize>>9;


	pageBuf = (UINT8*)malloc(pagesize);
	
	/* if from addr is not aligned to pagesize */
	unaligned_off = from%pagesize;
	if (unaligned_off)
	{
		start_addr = from/pagesize*pagesize;
		oplen = pagesize;

		NANDAPI_DEBUG("from addr is not aligned to pagesize\n");

		block_no = start_addr/blocksize;

		ret = NF_BlockIsBad(nand_dev, part_idx, block_no*lba_per_block);
		if (ret<0)
		{
			goto RETURN;
		}

		start_lba = start_addr/pagesize*lba_per_page;
		num_lba = oplen/pagesize*lba_per_page;
		ret = NF_PhysicalRead_Ext(nand_dev, part_idx, start_lba, num_lba, pageBuf);		
		if (ret<0)
		{
			goto RETURN;
		}

		if (len < pagesize-unaligned_off)
			buf_len = len;
		else
			buf_len = pagesize-unaligned_off;

		NANDAPI_DEBUG("copy page buf off:0x%x, len:0x%x to buffer\n", unaligned_off,buf_len);
		memcpy(buf, pageBuf+unaligned_off, buf_len);
	}

	if (buf_len >= len)
	{
		NANDAPI_DEBUG("read ok\n");	
		goto RETURN;
	}

	/* read aligned buf */
	start_addr = (from+pagesize-1)/pagesize*pagesize;
	oplen = (len - buf_len)/pagesize*pagesize;
	if (oplen>0)
	{

		NANDAPI_DEBUG("read alinged buf addr:0x%x, len:0x%x\n",start_addr, oplen);

		block_no = start_addr/blocksize;

		ret = NF_BlockIsBad(nand_dev, part_idx, block_no*lba_per_block);
		if (ret<0)
		{
			goto RETURN;
		}

		start_lba = start_addr/pagesize*lba_per_page;
		num_lba = oplen/pagesize*lba_per_page;
		ret = NF_PhysicalRead_Ext(nand_dev, part_idx, start_lba, num_lba, buf+buf_len);		
		if (ret<0)
		{
			goto RETURN;
		}

		buf_len += oplen;
	}

	/* if rest len is not aligned to pagesize */
	if (buf_len < len)
	{
		start_addr += oplen;
		oplen = pagesize;	

		NANDAPI_DEBUG("rest len is not aligned to pagesize\n");

		block_no = start_addr/blocksize;

		ret = NF_BlockIsBad(nand_dev, part_idx, block_no*lba_per_block);
		if (ret<0)
		{
			goto RETURN;
		}

		start_lba = start_addr/pagesize*lba_per_page;
		num_lba = oplen/pagesize*lba_per_page;
		ret = NF_PhysicalRead_Ext(nand_dev, part_idx, start_lba, num_lba, pageBuf);		
		if (ret<0)
		{
			goto RETURN;
		}

		NANDAPI_DEBUG("copy page buf len:0x%x to buffer[0x%x]\n", len-buf_len, buf_len);

		memcpy(buf+buf_len, pageBuf, len-buf_len);
	}

	NANDAPI_DEBUG("read ok\n");
	
RETURN:
	if (pageBuf)
		free(pageBuf);
	return ret;
}

int NF_get_part_valid_len(UINT32 part_idx)
{
	struct mtd_part_info *info;
	int valid_len;
	UINT32 block_size;
	UINT32 lba_per_block;
	UINT32 block_no;
	
	NF_get_part_info(part_idx, info);
	block_size = info->erasesize;
	lba_per_block = block_size >> 9;
	
	for(block_no=0; block_no<info->size/block_size; block_no++)
	{
		//test the current block is bad block or not
		if (NF_BlockIsBad(nand_dev, part_idx, block_no*lba_per_block) >= 0)
		{
			valid_len += block_size;
		}
	}

	return valid_len;
}

struct mtd_partition *NF_get_PMI_parts(struct ali_nand_device *dev)
{
	struct ali_nand_host *host;
	struct mtd_partition *pmi_parts=NULL;

	if (dev==NULL)
	{
		goto RETURN;
	}

	host = (struct ali_nand_host *)dev->priv;
	if (host==NULL)
	{
		goto RETURN;
	}

	pmi_parts = host->parts;
	
RETURN:		
	return pmi_parts;
}

#endif

INT32 NF_erase_part(UINT32 part_idx,UINT32 from, UINT32 len)
{
	struct mtd_part_info info;
	UINT32 erasesize;
	UINT32 i;

	if (NF_get_part_info(0, &info) < 0)
	{
		NANDAPI_ERROR("can not get part info\n");		
	}

	erasesize = info.erasesize;
	if (from%erasesize ||len%erasesize)
	{
		NANDAPI_ERROR("logic NF_erase_part: addr or size not align to blocksize\n");
		return -1;
	}

	
	for (i=from/erasesize; i<(from+len)/erasesize; i++)
	{
		NANDAPI_DEBUG("erase part[%d] block[%d]\n",part_idx,i);
		NF_erase_block(part_idx, i);
	}

	return 0;
}

INT32 NF_get_PMI(UINT8* PMI_buf, UINT32 pmi_len) 
{
	INT32 ret = -1;
	struct mtd_part_info info;
	UINT8 *kbuf = 0;
	UINT32 start_addr;
	UINT32 i;

	if (NF_get_part_info(0, &info) < 0)
	{
		NANDAPI_ERROR("Ask get part%d info fail\n",0);
		return -1;
	}

	kbuf = PMI_buf;

	/* read mtd0 page[0], page[256], page[512], page[768] */	
	for (i=0; i<4; i++)
	{
		start_addr = i*256*info.writesize;

		/* check ddr&miniboot block is bad block or not */
		ret = NF_read_part_physical(0, start_addr+info.erasesize, pmi_len, kbuf);
		if (ret < 0)
			continue;

		/* read pmi data */
		ret = NF_read_part_physical(0, start_addr, pmi_len, kbuf);
		if (ret < 0)
			continue;
		
		/*compare PMI tag */
		if (kbuf[14]==0x55 && kbuf[15]==0xAA)
		{
			break;
		}
	}

	if (i >= 4)
	{
		NANDAPI_ERROR("can not find valid PMI\n");	
		return -1;
	}
	
	NANDAPI_DEBUG("find valid PMI %d\n",i);
	return i;
}

INT32 NF_check_PMI_count(UINT32 pmi_len, UINT32 *pmi_state)
{
	INT32 ret = 0;
	INT32 pmi_ok = 0;
	struct mtd_part_info info;
	UINT8 *kbuf = NULL;
	UINT32 start_addr;
	UINT32 state=0;
	UINT32 i;
	 
	if (NF_get_part_info(0, &info) < 0)
	{
		NANDAPI_ERROR("Ask get part%d info fail\n",0);
		return -1;
	}

	kbuf = (UINT8*)malloc(pmi_len);
	if(NULL == kbuf)
	{
		NANDAPI_ERROR("cannot malloc for kbuf\n");
		return -1;
	}
	 
	/* read mtd0 page[0], page[256], page[512], page[768] */ 
	for (i=0; i<4; i++)
	{	 	
		start_addr = i*256*info.writesize;
		/* check ddr&miniboot block is bad block or not */
		ret = NF_read_part_physical(0, start_addr+info.erasesize, pmi_len, kbuf);
		if (ret < 0)
			continue;

		/* read pmi data */
		ret = NF_read_part_physical(0, start_addr, pmi_len, kbuf);
		if (ret < 0)
			continue;

		pmi_ok++;
		state |= (1<<(2*i));
	
		/*compare PMI tag */
		if (kbuf[14]==0x55 && kbuf[15]==0xAA)
		{
			state |= (1<<(2*i+1));		
		}
		
	 }

	if(kbuf)
	{
		free(kbuf);
		kbuf = NULL;
	}
	
	NANDAPI_DEBUG("valid PMI count %d, state: %x\n",pmi_ok, state);

	*pmi_state = state;
	
	return pmi_ok;
}

#define PMI_LEN 1024*2  // 2k

#define CONFIG_OFS	16
#define PARTITION_NUM 256
#define PARITION0_INDEX 260
#define PARTITIONS_NAME	512	//partition name 512-1023

/* private rawdata update */
INT32 NF_write_PMI_partition(UINT8 *buffer_in, UINT32 buf_len)
{
	INT32 ret = 0;
	INT32 i = 0;
	struct mtd_part_info info;
	UINT32 start_addr, len;
	UINT32 pagesize, blocksize;
	UINT8 *pmi_buf = NULL;
	UINT32 pmi_count;
	UINT32 pmi_state, state;
	INT8 pmi_idx[4];
	UINT32 j,k;

	if (buffer_in==NULL)
	{
		NANDAPI_ERROR("buffer_in==NULL\n");
		ret = -1;
		goto RETURN;		
	}
		
	if (NF_get_part_info(0, &info) < 0)
	{
		NANDAPI_ERROR("Ask get part%d info fail\n",0);
		ret = -1;
		goto RETURN;		
	}

	pagesize = info.writesize;
	blocksize = info.erasesize;

	if (buf_len <= (3*256*pagesize+blocksize))
	{
		NANDAPI_ERROR("buf_len(0x%x) <= 0x%x, pmi partition data invalid.\n", 
			buf_len, (3*256*pagesize+blocksize));
		ret = -1;
		goto RETURN;		
	}

	pmi_count = NF_check_PMI_count(PMI_LEN, &pmi_state);
	if (pmi_count < 2)
	{
		NANDAPI_ERROR("PMI count %d < 2, can not update PMI data\n",pmi_count);
		ret = 0;
		goto RETURN;		
	}

	for (i = 0; i < 4; i++)
	{
		pmi_idx[i] = -1;	
	}
	
	j = 0;
	k = 3;
	for (i=3; i>=0; i--)
	{
		state = (pmi_state >> (2*i)) & 0x3;

		if (state == 1)
			pmi_idx[j++] = i;
		else if (state  == 3)
			pmi_idx[k--] = i;		
	}
	
	pmi_buf = malloc(PMI_LEN);
	if(NF_get_PMI(pmi_buf,PMI_LEN) < 0)
	{
		NANDAPI_ERROR("can not find valid PMI\n");
		ret = -1;
		goto RETURN;		
	}

	ret  = -1;

	for (i = 0; i < 4; i++)
	{
		NANDAPI_DEBUG("update PMI[%d]\n", pmi_idx[i]);
		
		if (pmi_idx[i] < 0)
			continue;
		
		start_addr = pmi_idx[i]*256*pagesize;

		/* get read clock */
		memcpy(&pmi_buf[0x34], &buffer_in[start_addr+0x34], 4);

		/* get write clock */
		memcpy(&pmi_buf[0x38], &buffer_in[start_addr+0x38], 4);

		/* get DDR start */
		memcpy(&pmi_buf[0x4C], &buffer_in[start_addr+0x4c], 4);

		/* get DDR len */
		memcpy(&pmi_buf[0x50], &buffer_in[start_addr+0x50], 4);
		
		/* get miniboot start */
		memcpy(&pmi_buf[0x64], &buffer_in[start_addr+0x64], 4);

		/* get miniboot len */
		memcpy(&pmi_buf[0x68], &buffer_in[start_addr+0x68], 4);
		
		/* copy PMI to image, only reserve 0-0x100 space */
		memcpy(&buffer_in[start_addr], &pmi_buf[0], PARTITION_NUM);

		/* erase pmi data */
		if (NF_erase_block_physical(0, start_addr/blocksize) < 0)
		{
			NANDAPI_NOTICE("erase PMI %d  fail.\n", i);
			continue;
		}

		/* erase ddr/miniboot */
		if (NF_erase_block_physical(0, start_addr/blocksize + 1) < 0)
		{
			NANDAPI_NOTICE("erase DDR&miniboot %d fail.\n", i);
			continue;
		}

		/* write  ddr/miniboot */
		if (start_addr + 2*blocksize < buf_len)
			len = blocksize;
		else
			len = buf_len - start_addr - blocksize;
	        if (NF_write_part_physical(0, start_addr+blocksize, len, &buffer_in[start_addr+blocksize])<0)
		{
			NANDAPI_NOTICE("write DDR&miniboot %d fail.\n", i);
			continue;
		}

		/* write  pmi */
		len = blocksize;		
	        if (NF_write_part_physical(0, start_addr, len, &buffer_in[start_addr])<0)
		{
			NANDAPI_NOTICE("write PMI %d fail.\n", i);
			continue;
		}	

		/* write one PMI+DDR&miniboot OK */
		ret  = 0;
	}

RETURN:
	if (pmi_buf)
		free(pmi_buf);
	
	NANDAPI_DEBUG("Leave!\n");
	return ret;
}

INT32 NF_get_PMI_part_by_name(char *part_name)
{
	UINT32 part_num;
	UINT32 i;
	UINT8 pmi_buf[2048];

	if(NF_get_PMI(pmi_buf,2048) < 0)
	{
		NANDAPI_ERROR("can not find valid PMI\n");
		return -1;	
	}

	part_num=	(((INT32) pmi_buf[PARTITION_NUM+3]) << 24)
			|	(((INT32) pmi_buf[PARTITION_NUM+2]) << 16)
			|	(((INT32) pmi_buf[PARTITION_NUM+1]) << 8) 
			|	 pmi_buf[PARTITION_NUM];

	for (i=0; i<part_num; i++)
	{
		if (strcmp(&pmi_buf[PARTITIONS_NAME+i*16], part_name)==0)
		{
			NANDAPI_DEBUG("Find %s in PMI part%d\n", part_name, i);
			return i;			
		}
	}

	NANDAPI_DEBUG("Can not find %s in PMI parts\n", part_name);
	return -1;
}

INT32 NF_get_nand_size(void)
{
	UINT8 pmi_buf[2048];
	UINT32 wBlksPerChip, wBytesPerPage, wPagesPerBlock;
	UINT32 chipsize;

	if(NF_get_PMI(pmi_buf,2048) < 0)
	{
		NANDAPI_ERROR("can not find valid PMI\n");
		return -1;	
	}

	wBlksPerChip = *(UINT32 *) &pmi_buf[CONFIG_OFS + 0];
	wBytesPerPage = *(UINT32 *) &pmi_buf[CONFIG_OFS + 4];
	wPagesPerBlock = *(UINT32*) &pmi_buf[CONFIG_OFS + 8];	

    	chipsize = wBlksPerChip * wBytesPerPage * wPagesPerBlock;

	NANDAPI_DEBUG("nand flash size: 0x%x\n", chipsize);
	
	return chipsize;
}


