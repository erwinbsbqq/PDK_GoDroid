#include <jffs2/load_kernel.h>
#include <common.h>
#include <asm/io.h>
#include <boot_common.h>
#include <asm/dma-mapping.h>
#include <linux/err.h>
#include <nand.h>
#include "ali_nand.h"
#include "block_sector.h"
#include <alidefinition/adf_boot.h>

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8  uint8_t

#define MULTI_COMP_KERNEL_INDEX		0
#define MULTI_COMP_RAMDISK_INDEX	1
#define MULTI_COMP_SEE_INDEX		2
#define MULTI_COMP_AE_INDEX			3

int get_part_by_name_nand(const char *partName, loff_t *start, size_t *size)
{
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;
	int ret;

	ret = find_dev_and_part(partName, &dev, &pnum, &part);
	if (ret)
		return -1;

	*start = (loff_t)(part->offset);
	*size = (size_t)(part->size);

	return pnum;
}

int load_part_nand(const char *partName, u_char *LoadAddr)
{
	loff_t start;
	size_t size;
	nand_info_t *nand = &nand_info[nand_curr_device];

	//printf("Load part '%s', start:0x%llx, size:0x%x\n", partName, start, size);

	if (get_part_by_name(partName, &start, &size) < 0)
	{
		printf("<%s>(%d): '%s' is not exist\n", __FUNCTION__, __LINE__, partName);
		return -1;
	}

	if (nand_read_skip_bad(nand, start, &size, LoadAddr) < 0)
	{
		printf("nand_read_skip_bad error <load_part>\n");
		return -2;
	}

	return size;
}

int load_part_nand_ext(const char *partName, u_char *LoadAddr, loff_t offset, size_t len)
{
	loff_t start;
	size_t size;
	nand_info_t *nand = &nand_info[nand_curr_device];
	size_t blocksize = nand->erasesize;
	size_t pagesize = nand->writesize;
	loff_t blockoffset, badblocksize = 0;

	//printf("Load part ext '%s', offset:0x%llx, len:0x%x\n", partName, offset, len);

	if (get_part_by_name(partName, &start, &size) < 0)
	{
		printf("<%s>(%d): '%s' is not exist\n", __FUNCTION__, __LINE__, partName);
		return -1;
	}

	if ((offset + len) > (loff_t)size)
	{
		printf("<%s>(%d): '%s' exceeds part size(offset=0x%llx len=0x%x partsize=0x%x)\n", __FUNCTION__, __LINE__, partName, offset, len, size);
		return -2;
	}

	/* Check LoadAddr 32 bytes aligned */
	if (((u32)LoadAddr & 0x1F) != 0)
	{
		printf("<%s>(%d): '%s' read data buffer(0x%08x) NOT 32 bytes aligned!\n", __FUNCTION__, __LINE__, partName, LoadAddr);
		return -3;
	}

	/* Skip bad blocks from part start to 'offset' */
	for (blockoffset = 0; blockoffset < (offset & (blocksize - 1)); blockoffset += blocksize)
	{
		if (nand_block_isbad(nand, start + blockoffset))
			badblocksize += blocksize;
	}

	offset += (start + badblocksize);

	if ((offset & (pagesize - 1)) != 0)
	{
		loff_t offset_aligned;
		size_t diff_before, diff_after, copy_size;
		u_char * align_buf, *align_buf_tmp;

		if ((align_buf_tmp = malloc(pagesize + 0x20)) == (void *)0)
		{
			printf("<%s>(%d): malloc for '%s' data align buffer error\n", __FUNCTION__, __LINE__, partName);
			return -4;
		}
		align_buf = (u_char *)(((u32)align_buf_tmp + (u32)0x1F) & (~(u32)0x1F));

		offset_aligned = offset & ~(pagesize - 1);
		diff_before = offset - offset_aligned;
		diff_after = pagesize - diff_before;
		copy_size = (offset + len > offset_aligned + pagesize) ? diff_after : len;

		if (nand_read_skip_bad(nand, offset_aligned, &pagesize, align_buf) < 0)
		{
			printf("<%s>(%d): '%s' read data step 1 error @0x%llx\n", __FUNCTION__, __LINE__, partName, offset_aligned);
			free(align_buf_tmp);
			return -5;
		}

		memcpy(LoadAddr, align_buf + diff_before, copy_size);
		free(align_buf_tmp);

		if (offset + len > offset_aligned + pagesize)
		{
			size_t rest_len = len - diff_after;
			if (nand_read_skip_bad(nand, offset_aligned + pagesize, &rest_len, LoadAddr + diff_after) < 0)
			{
				printf("<%s>(%d): '%s' read data step 2 error @0x%llx\n", __FUNCTION__, __LINE__, partName, offset_aligned + pagesize);
				return -6;
			}
		}
	}
	else
	{
		if (nand_read_skip_bad(nand, offset, &len, LoadAddr) < 0)
		{
			printf("<%s>(%d): '%s' read data error @0x%llx\n", __FUNCTION__, __LINE__, partName, offset);
			return -7;
		}
	}

	return len;
}

int load_part_sector_nand(const char *partName, u_char *LoadAddr, int bufLen)
{
	nand_info_t *nand = &nand_info[nand_curr_device];
	loff_t start;
	size_t size;
	loff_t offset;
	size_t len;
	u_char *datbuf, *datbuf_tmp;	
	int blockNum;
	int dataLen;
	int i, ret;

	get_part_by_name(partName, &start, &size);

	blockNum = size/nand->erasesize;
	//adds for cache line size align
	datbuf_tmp = (u_char *)malloc(nand->erasesize + 0x20);
	datbuf = (u8 *)(((u32)datbuf_tmp + (u32)0x1F) & (~(u32)0x1F));

	for (i = 0; i < blockNum; i++)
	{
		offset = start + i * nand->erasesize;
		len = nand->erasesize;
		if (nand_block_isbad(nand, offset & ~(nand->erasesize - 1)))
		{
			printf("block is bad, offset:0x%llx\n", offset);
			continue;
		}

		if (nand_read(nand, offset, &len, datbuf))
		{
			printf("nand_read error <load_part_sector>\n");
			free(datbuf_tmp);
			return -1;
		}

		/* check block data */
		dataLen = _sector_crc_check(datbuf, nand->erasesize);
		if (dataLen > 0)
			break;
	}

	if (i == blockNum)
	{
		printf("can not find sector in part %s\n",partName);
		free(datbuf_tmp);			
		return -1;
	}

	if (bufLen > dataLen - 12)
    {
		printf("sector format is not matched, size:0x%x, datalen:0x%x\n", bufLen, dataLen - 12);
		ret = dataLen - 12;
	}
	else
	{
		ret = bufLen;
	}

	memcpy(LoadAddr, datbuf + 12, ret);

	free(datbuf_tmp);
	return ret;
}

