/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2011 Copyright (C)
*
*    File:    sto_nand.c
*
*    Description:    This file contains all functions definition
*		             of nand flash operation from user space.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Nov.24.2011      Randy.Qiu       Ver 0.1       Create file.
*****************************************************************************/
#include <adr_retcode.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>


//#include <linux/compiler.h>
#include <mtd/mtd-user.h>

#include <osal/osal.h>

#include <hld/sto/adr_sto_nand.h>
//#include <api/libc/log.h>

#define NANDINDEX_PMILOC                1
#define NAND_TASK_SUPPORT_NUM_MAX		256

#if 1
#define STONAND_ERROR(args...)		do{}while(0)
#define STONAND_NOTICE(args...)		do{}while(0)
#define STONAND_DEBUG(args...)		do{}while(0)
#define STONAND_PRINTF(args...)		do{}while(0)
#define STONAND_DUMP(addr, len)		do{}while(0)
#else
#define STONAND_ERROR(args...)	amslog_error(LOG_STBINFO,args)
#define STONAND_NOTICE(args...)	amslog_notice(LOG_STBINFO,args)
#define STONAND_DEBUG(args...)	amslog_debug(LOG_STBINFO,args)
#define STONAND_PRINTF(args...)	amslog_printf(LOG_STBINFO,args)
#define STONAND_DUMP(addr, len)	amslog_dump(LOG_STBINFO,addr, len)
#endif

#define LONG_LIT(addr)		(((addr)[0])|((addr)[1]<<8)|((addr)[2]<<16)|((addr)[3]<<24))
#define LONG_BIT(addr)		(((addr)[0]<<24)|((addr)[1]<<16)|((addr)[2]<<8)|((addr)[3]))

typedef struct {
	INT32 				id[NAND_TASK_SUPPORT_NUM_MAX];
	UINT32		            curr_addr[NAND_TASK_SUPPORT_NUM_MAX];	/* Current operate address */
	struct mtd_info_user 	*baseinfo;		                        /* nand Device totol space size, in K bytes */
	int                     mtdfd;                                  /* file descriptor of nand device with MTD */ 
	int                  flags;                                   /* nand device open flags */
}nand_curinfo_t;

static nand_curinfo_t nandinfo_arry[MAX_PARTITION_NUM] = {
	{.mtdfd = -1, .baseinfo=0},	// 0
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},	// 10
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},	// 20
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},
	{.mtdfd = -1, .baseinfo=0},	// 30
};

UINT16 get_id_index(UINT8 index, INT32 pid)
{
	UINT16 i = 0;
	nand_curinfo_t* pNandInfo = NULL;

	pNandInfo = &nandinfo_arry[index];

	for(i = 0; i < NAND_TASK_SUPPORT_NUM_MAX; i++)
	{
		if(pNandInfo->id[i] == pid)
			break;
	}

	return i;
}
	
void printsize (uint32_t x)
{
	int i;
	static const char *flags = "KMGT";
	
	STONAND_PRINTF ("%u ",x);

	for (i = 0; x >= 1024 && flags[i] != '\0'; i++) 
		x /= 1024;

	i--;

	if (i >= 0) 
		STONAND_PRINTF ("(%u%c)",x,flags[i]);
}

int showmtdinfo (struct mtd_info_user *mtd)
{
	int i,err;

	STONAND_PRINTF ("\nmtd.type = ");
	switch (mtd->type)
	{
		case MTD_ABSENT:
			STONAND_PRINTF ("MTD_ABSENT");
			break;
		case MTD_RAM:
			STONAND_PRINTF ("MTD_RAM");
			break;
		case MTD_ROM:
			STONAND_PRINTF ("MTD_ROM");
			break;
		case MTD_NORFLASH:
			STONAND_PRINTF ("MTD_NORFLASH");
			break;
		case MTD_NANDFLASH:
			STONAND_PRINTF ("MTD_NANDFLASH");
			break;
		case MTD_DATAFLASH:
			STONAND_PRINTF ("MTD_DATAFLASH");
			break;
		case MTD_UBIVOLUME:
			STONAND_PRINTF ("MTD_UBIVOLUME");
		default:
			STONAND_PRINTF ("(unknown type - new MTD API maybe?)");
	}

	STONAND_PRINTF ("\nmtd.flags = ");
	if (mtd->flags == MTD_CAP_ROM)
		STONAND_PRINTF ("MTD_CAP_ROM");
	else if (mtd->flags == MTD_CAP_RAM)
		STONAND_PRINTF ("MTD_CAP_RAM");
	else if (mtd->flags == MTD_CAP_NORFLASH)
		STONAND_PRINTF ("MTD_CAP_NORFLASH");
	else if (mtd->flags == MTD_CAP_NANDFLASH)
		STONAND_PRINTF ("MTD_CAP_NANDFLASH");
	/*else if (mtd->flags == MTD_WRITEABLE)    //same with MTD_CAP_NANDFLASH
		STONAND_PRINTF ("MTD_WRITEABLE");*/
	else
	{
		int first = 1;
		static struct
		{
			const char *name;
			int value;
		} flags[] =
		{
			{ "MTD_WRITEABLE", MTD_WRITEABLE },
			{ "MTD_BIT_WRITEABLE", MTD_BIT_WRITEABLE },
			{ "MTD_NO_ERASE", MTD_NO_ERASE },
			{ "MTD_POWERUP_LOCK", MTD_POWERUP_LOCK },
			{ NULL, -1 }
		};
		for (i = 0; flags[i].name != NULL; i++)
			if (mtd->flags & flags[i].value)
			{
				if (first)
				{
					STONAND_PRINTF (flags[i].name);
					first = 0;
				}
				else STONAND_PRINTF (" | %s",flags[i].name);
			}
	}

	STONAND_PRINTF ("\nmtd.oobsize = ");
	printsize (mtd->oobsize);

	STONAND_PRINTF ("\nmtd.size = ");
	printsize (mtd->size);

	STONAND_PRINTF ("\nmtd.erasesize = ");
	printsize (mtd->erasesize);

	STONAND_PRINTF ("\nmtd.writesize = ");
	printsize (mtd->writesize);
	
	STONAND_PRINTF ("\n");

	return (0);
}

/*
 * whether visit nand device from upper layer or not
*/
static int nand_is_ready(UINT8 index)
{
	int ret;
	
	if (index >= MAX_PARTITION_NUM)
		return -1;
	
	nand_curinfo_t* pNandInfo = &nandinfo_arry[index];
	if (pNandInfo->mtdfd == -1)
		ret = -1;
	else
		ret = 1;

	return ret;
}

/*
 * whether visit nand device from upper layer or not
*/
static int nandinfo_is_ready(UINT8 index)
{
	int ret;
	
	if (index >= MAX_PARTITION_NUM)
		return -1;
	
	nand_curinfo_t* pNandInfo = &nandinfo_arry[index];
	if (pNandInfo->baseinfo== 0)
		ret = -1;
	else
		ret = 1;

	return ret;
}

/*
 * reuturn   : -1 ==> fail
 *             else ==> the real block index of all nand device
 *
*/
static int nand_get_blockno(UINT8 index, UINT32 start)
{
	if(-1 == nand_is_ready(index))
		return -1;
	
	nand_curinfo_t* pNandInfo = &nandinfo_arry[index];
	int block_index = start/(pNandInfo->baseinfo->erasesize);//index is from 0 to large..
	return block_index;
}

/*
 * reuturn   : -1 ==> fail
 *             else ==> the block size of nand device
 *
*/
static int nand_get_blocksize(UINT8 index)
{
	if(-1 == nand_is_ready(index))
		return -1;
	
	nand_curinfo_t* pNandInfo = &nandinfo_arry[index];
	return pNandInfo->baseinfo->erasesize;
}

static int nand_block_isbad(UINT8 index, UINT32 offset)
{
	nand_curinfo_t* pNandInfo;
	loff_t ofs;
	int ret;

	//STONAND_DEBUG("ENTER, index:%d, offset:0x%x\n",index,offset);
		
	if(nand_is_ready(index) < 0)
	{
		STONAND_ERROR("Nand device %d is not ready, please open it first!\n",index);
		return -1;
	}

	pNandInfo = &nandinfo_arry[index];

	ofs = offset;
	ret = ioctl(pNandInfo->mtdfd,MEMGETBADBLOCK,(u_long)&ofs);
	if (ret < 0)
	{
		STONAND_ERROR("Block(offset:0x%x) Is Bad error...\n", offset);
		return -1;
	}
	else if (ret == 0)
	{
//		STONAND_DEBUG("part[%d] offset(0x%x) is good block\n", index, offset);
		/* good block */
		return 0;
	}
	else
	{
		STONAND_DEBUG("part[%d] offset(0x%x) is bad block\n", index, offset);
		/* bad block */
		return -1;
	}		
}

static int nand_get_actual_offset(UINT8 index, UINT32 valid_off)
{
	nand_curinfo_t* pNandInfo;
	UINT32 blocksize;
	UINT32 block_no, block_off;
	UINT32 offset;
	UINT32 blidx;

//	STONAND_DEBUG("ENTER valid_off:0x%x\n",valid_off);
	
	if(nand_is_ready(index) < 0)
	{
		STONAND_ERROR("Nand device %d is not ready, please open it first!\n",index);
		return -1;
	}

	pNandInfo = &nandinfo_arry[index];

	blocksize = pNandInfo->baseinfo->erasesize;

	if (valid_off >= pNandInfo->baseinfo->size)
	{
		STONAND_ERROR("valid_off(0x%x) invalid\n", valid_off);
		return -1;
	}

	block_no = valid_off/blocksize;
	block_off = valid_off%blocksize;
	
	blidx = 0;
	for (offset=0; offset<pNandInfo->baseinfo->size; offset+=blocksize)
	{
		/* bad block, skip it */
		if (nand_block_isbad(index, offset) < 0)
			continue;

		/* find right block */
		if (blidx == block_no)
			break;

		/* find next good block */
		blidx++;
	}

	if (blidx != block_no)
	{
		STONAND_ERROR("can not find block_no(%d) invalid\n", block_no);
		return -1;
	}

	offset += block_off;

//	STONAND_DEBUG("find part[%d] valid offset(x%x)-->offset(0x%x)\n", 
//		index,valid_off,offset);

	return offset;
}

static int nand_get_valid_offset(UINT8 index, UINT32 addr)
{
	nand_curinfo_t* pNandInfo;
	UINT32 blocksize;
	UINT32 block_no, block_off;
	UINT32 offset, valid_offset;
	UINT32 blidx;

//	STONAND_DEBUG("ENTER addr:0x%x\n",addr);
	
	if(nand_is_ready(index) < 0)
	{
		STONAND_ERROR("Nand device %d is not ready, please open it first!\n",index);
		return -1;
	}

	pNandInfo = &nandinfo_arry[index];

	blocksize = pNandInfo->baseinfo->erasesize;

	if (addr >= pNandInfo->baseinfo->size)
	{
		STONAND_ERROR("addr(0x%x) invalid\n", addr);
		return -1;
	}

	block_no = addr/blocksize;
	block_off = addr%blocksize;
	
	blidx = 0;
	for (offset=0; offset<addr; offset+=blocksize)
	{
		/* bad block, skip it */
		if (nand_block_isbad(index, offset) < 0)
			continue;

		/* find next good block */
		blidx++;
	}

	valid_offset = blidx*blocksize + block_off;

//	STONAND_DEBUG("find part[%d] offset(x%x)-->valid offset(0x%x)\n", 
//		index,addr, valid_offset);

	return valid_offset;
}

/*
 * parameter : flags ==> O_RDONLY, O_WRONLY, O_RDWR, ...
 * reuturn   : -1 ==> fail
 *             else ==> success
 *
*/
int nand_open(UINT8 index, int flags)
{	
	int pid = 0, id_idx = 0;
	int mtdfd = -1;
	char mtdname[64] = {0};
	nand_curinfo_t* pNandInfo = NULL;
	char flag_str[16];
	int i = 0;
	

	if (index >= MAX_PARTITION_NUM)
	{
		STONAND_ERROR("no part[%d], max part num:%d\n",index, MAX_PARTITION_NUM);
		return -1;
	}

	switch (flags)
	{
	case O_RDONLY:
		strcpy(flag_str, "O_RDONLY");
		break;
	case O_RDWR:
		strcpy(flag_str, "O_RDWR");
		break;
	default:
		strcpy(flag_str, "other");		
	}
	
	//STONAND_DEBUG("open part[%d] %s\n", index, flag_str);

	pNandInfo = &nandinfo_arry[index];
	
	if(1 == nand_is_ready(index))//success if this mtd device is ready
	{
		if (pNandInfo->flags == flags)
		{
			pid = getpid();
			id_idx = get_id_index(index, pid);
			if(id_idx>=NAND_TASK_SUPPORT_NUM_MAX)
			{
				//new thread open nand
				id_idx = get_id_index(index, -1);
				if(id_idx>=NAND_TASK_SUPPORT_NUM_MAX)
				{
					close(pNandInfo->mtdfd);
					pNandInfo->mtdfd = -1;	
					goto open_nand;
				}
				else
				{
					pNandInfo->id[id_idx] = pid; 
				}
			}
			else
			{
				
			}
			
			return pNandInfo->mtdfd;
		}
		else
		{
			close(pNandInfo->mtdfd);
			pNandInfo->mtdfd = -1;				
		}
	}

open_nand:
	sprintf(mtdname,"/dev/mtd%d", index);
	mtdfd = open(mtdname, flags | O_CLOEXEC);

	if(-1 != mtdfd)
	{
		for(i = 0; i < NAND_TASK_SUPPORT_NUM_MAX; i++)
		{
			pNandInfo->id[i] = -1;
		}
		pNandInfo->mtdfd = mtdfd;
		pNandInfo->flags = flags;

		if(nandinfo_is_ready(index) < 0)
		{		
			pNandInfo->baseinfo = (struct mtd_info_user*)malloc(sizeof(struct mtd_info_user));
			if(ioctl(mtdfd, MEMGETINFO, pNandInfo->baseinfo))
			{
				STONAND_ERROR("[ERR] %s MEMGETINFO fail!\n", __FUNCTION__);
				return -1;
			}
		}

		//showmtdinfo(pNandInfo->baseinfo);
		pid = getpid();
		id_idx = get_id_index(index, -1);
		STONAND_DEBUG("pid:%d, id index: %d\n",pid, id_idx);

		if(id_idx>=NAND_TASK_SUPPORT_NUM_MAX)
		{
			//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
			return -1;
		}
		pNandInfo->id[id_idx] = pid;
		pNandInfo->curr_addr[id_idx] = 0;
		lseek(mtdfd, pNandInfo->curr_addr[id_idx], SEEK_SET);
	}
	else
	{
		STONAND_ERROR("open part[%d] fail, ret:%d\n", index, mtdfd);
	}

	//STONAND_DEBUG("open fd:0x%x\n",pNandInfo->mtdfd);

	return mtdfd;
}

/*
 * Name      : nand_len
 * parameter : 
 *		index: part index
 * reuturn   : -1 ==> failure
 *             otherwise ==> part valid len
*/
int nand_len(UINT8 index)
{
	int id = 0, pid = 0;
	int block_size = 0;
	nand_curinfo_t* pNandInfo = &nandinfo_arry[index];
	struct mtd_info_user *mtdinfo;
	UINT32 data_start, data_end;
	int valid_len;

	STONAND_DEBUG("get part[%d] len\n",index);
	
	if(-1 == nand_is_ready(index))
		return -1;

	pid = getpid();
	id = get_id_index(index, pid);
	if(id>=NAND_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		return -1;
	}

	block_size = nand_get_blocksize(index);
	mtdinfo = pNandInfo->baseinfo;

	data_start = 0;
	data_end = mtdinfo->size;
	valid_len = 0;

	STONAND_DEBUG("part[%d] size:0x%08x\n",index, mtdinfo->size);
	
	while(data_start < data_end)
	{
		//test the current block is bad block or not
		if (nand_block_isbad(index, data_start) == 0)
		{
			valid_len += block_size;
		}

		data_start += block_size;
	}

	STONAND_DEBUG("part[%d] validlen:0x%08x\n",index, valid_len);

	return valid_len;
}

/*
 * Name      : nand_lseek(void* buf, UINT32 size)
 * parameter : whenence => SEEK_SET, SEEK_CUR, SEEK_END
 *			offset: align to pagesize
 * reuturn   : -1 ==> failure
 *             else ==> position offset from the beginning
 *
*/
int nand_lseek(UINT8 index, int offset, int whenence)
{
	int id = 0, pid = 0;
	int new_addr;
	int mtdsize;
	nand_curinfo_t* pNandInfo;
	UINT32 addr;

	if(nand_is_ready(index) < 0)
	{
		STONAND_ERROR("Nand device %d is not ready, please open it first!\n",index);
		return -1;
	}

	pid = getpid();
	id = get_id_index(index, pid);
	if(id>=NAND_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		return -1;
	}

	pNandInfo = &nandinfo_arry[index];

	if (offset%pNandInfo->baseinfo->writesize)
	{
		STONAND_ERROR("lseek offset[0x%x] not align to pagesize[0x%x]\n",
			offset, pNandInfo->baseinfo->writesize);
		return -1;	
	}
	
	mtdsize = pNandInfo->baseinfo->size;
	
	switch(whenence)
	{
		case SEEK_SET:
			/* Great than totol size, seek to end */
			if (offset >= mtdsize)
			{
				pNandInfo->curr_addr[id] = mtdsize - 1;
			}
			/* Common seek */
			else if (offset >= 0)
			{
				new_addr = nand_get_actual_offset(index, offset);
			    	pNandInfo->curr_addr[id] = new_addr;
			}
			break;

		case SEEK_CUR:
			addr = nand_get_valid_offset(index, pNandInfo->curr_addr[id]);
			new_addr = nand_get_actual_offset(index, addr + offset);
			
			/* Less than base address, seek to begin */
			if (new_addr < 0)
			{
				pNandInfo->curr_addr[id] = 0;
			}
			/* Great than totol size, seek to end */
			else if (new_addr >= mtdsize)
			{
				pNandInfo->curr_addr[id] = mtdsize - 1;
			}
			/* Common seek */
			else
			{
				pNandInfo->curr_addr[id] = new_addr;
			}
			break;

		case SEEK_END:
			new_addr = mtdsize + offset - 1;
			
			/* Less than base address, seek to begin */
			if (new_addr < 0)
			{
				pNandInfo->curr_addr[id] = 0;
			}
			/* Common seek */
			else if (offset <= 0)
			{
				pNandInfo->curr_addr[id] = new_addr;
			}
			break;
			
		default:
			STONAND_ERROR("please check your whenence parameter!\n");
			return -1;		
	}

	//STONAND_DEBUG("seek part[%d] 0x%x\n", index,pNandInfo->curr_addr[id]);
	
	return pNandInfo->curr_addr[id];
}

/*
 * Name      : nand_erase
 * parameter : 
 *		index: part index
 *		block_no: -1, erase all partition; >=0, erase a special block
 * reuturn   : -1 ==> failure
 *             0 ==> success
*/
int nand_erase(UINT8 index, INT32 block_no)
{
	erase_info_t fls_erase;
	nand_curinfo_t* pNandInfo;
	struct mtd_info_user *mtdinfo;		
	UINT32 blocksize, i;
	int ret;

	STONAND_DEBUG("erase part[%d] block[%d]\n",index, block_no);
	
	if(nand_is_ready(index) < 0)
	{
		STONAND_ERROR("Nand device %d is not ready, please open it first!\n",index);
		return -1;
	}

	pNandInfo = &nandinfo_arry[index];
	mtdinfo = pNandInfo->baseinfo;

	blocksize = mtdinfo->erasesize;

	if (block_no < 0)
	{
		/* erase all partition */

		for (i=0; i<mtdinfo->size/blocksize; i++)
		{
			fls_erase.start  = i*blocksize;	
			fls_erase.length = blocksize;
			ioctl(pNandInfo->mtdfd, MEMERASE,&fls_erase);	
		}				
	}
	else
	{
		/* erase a special block */
		fls_erase.start  = nand_get_actual_offset(index, blocksize*block_no);	
		fls_erase.length = blocksize;
	
		ret = ioctl(pNandInfo->mtdfd, MEMERASE,&fls_erase);
		if(ret < 0)
		{
			STONAND_ERROR("Erase block at 0x%x failed!\n", fls_erase.start);
			return -1;
		}
	}
	return 0;
}

/*
 * Name      : nand_read
 * parameter : 
 *		index: part index
 *		buf: read buffer
 *		size: read size, align to pagesize
 * reuturn   : -1 ==> failure
 *             else ==> bytes actually read
 *
*/
int nand_read(UINT8 index, void* buf, UINT32 size)
{
	int ret = -1;
	int id = 0, pid = 0;
	UINT32 data_start, data_end;
	UINT32 cur_blstart, cur_blend;
	UINT32 cur_opstart, cur_opend, cur_oplen;
	UINT32 rsize = 0;
	int block_no = 0;
	int block_size = 0;
	UINT8* pBuf = buf;
	nand_curinfo_t* pNandInfo = &nandinfo_arry[index];
	struct mtd_info_user *mtdinfo;
	
	if(NULL == buf)
		return -1;

	if(-1 == nand_is_ready(index))
		return -1;

	pid = getpid();
	id = get_id_index(index, pid);
	if(id>=NAND_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		return -1;
	}

	//STONAND_DEBUG("taskid:%d, cur_addr:0x%x, len:0x%x\n",id, pNandInfo->curr_addr[id],size);

	mtdinfo = pNandInfo->baseinfo;
	
	if (size%mtdinfo->writesize)
	{
		STONAND_ERROR("read size[0x%x] not align to pagesize[0x%x]\n",
			size, mtdinfo->writesize);
		return -1;	
	}
	
	data_start = pNandInfo->curr_addr[id];
	data_end   = data_start + size;
	data_end = data_end > mtdinfo->size ? mtdinfo->size : data_end;

	cur_opend = data_end;
	
	block_no = nand_get_blockno(index, data_start);
	block_size = nand_get_blocksize(index);
	if(-1 == block_no || -1 == block_size)
	{
		STONAND_ERROR("Get block info error!\n");
		return -1;
	}

	cur_opstart = data_start;
	while(cur_opstart < data_end)
	{
		//test the current block is bad block or not
		if (nand_block_isbad(index, cur_opstart) < 0)
		{
			cur_opstart += block_size;
			data_end += block_size;
			data_end = data_end > mtdinfo->size ? mtdinfo->size : data_end;		
			block_no++;
			continue;
		}
		cur_blstart = block_size * block_no;
		cur_blend   = cur_blstart + block_size;
		cur_opend =  cur_blend > data_end ? data_end : cur_blend;
		cur_oplen = cur_opend - cur_opstart;

		STONAND_DEBUG("read part[%d] from(0x%x) len(0x%x)\n",index,cur_opstart, cur_oplen);
		lseek(pNandInfo->mtdfd, cur_opstart, SEEK_SET);
		ret = read(pNandInfo->mtdfd, pBuf, cur_oplen);
		if(-1 == ret)
		{
			STONAND_ERROR("Read nand at 0x%x failed!\n", cur_opstart);
			return -1;
		}

		//STONAND_DUMP(pBuf, cur_oplen);

		pBuf += ret;	
		rsize += ret;

		cur_opstart += cur_oplen;
		block_no++;	
	}

	pNandInfo->curr_addr[id] = cur_opend;			

	return rsize;
}

/*
 * Name      : nand_write(void* buf, UINT32 size)
 * parameter : 
 *		index: part index
 *		buf: write buffer
 *		size: write size, align to pagesize
 * reuturn   : -1 ==> failure
 *             else ==> bytes actually write
 * write notic:
 *		1. write size must align to pagesize
 *		2. be sure write area is free
 *		3. can not skip page to write 
 *		(e.g. must write page0->page1->page2->..., forbidden page0->page2->page1)
*/
int nand_write(UINT8 index, void* buf, UINT32 size)
{
	int block_no = 0;
	int block_size = 0;
	int ret = -1;
	int id = 0, pid = 0;
	UINT32 cur_blstart, cur_blend;
	UINT32 cur_opstart, cur_opend, cur_oplen;
	UINT32 data_start, data_end;//the wanting start & end offset of nand flash
	UINT8* pTmp = NULL;
	UINT32 wsize = 0;
	nand_curinfo_t* pNandInfo = &nandinfo_arry[index];
	struct mtd_info_user *mtdinfo;
	UINT32 oplen = 0;
	
	if(NULL == buf)
		return -1;

	pid = getpid();
	id = get_id_index(index, pid);
	if(id>=NAND_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		return -1;
	}

	//STONAND_DEBUG("taskid:%d, cur_addr:0x%x, len:0x%x\n",id, pNandInfo->curr_addr[id],size);

	if(-1 == nand_is_ready(index))
		return -1;

	mtdinfo = pNandInfo->baseinfo;

	if (size%mtdinfo->writesize)
	{
		STONAND_ERROR("write size[0x%x] not align to pagesize[0x%x]\n",
			size, mtdinfo->writesize);
		return -1;	
	}

	data_start = pNandInfo->curr_addr[id];
	data_end = data_start + size;
	data_end = data_end > mtdinfo->size ? mtdinfo->size : data_end;

	block_no = nand_get_blockno(index, data_start);
	block_size = nand_get_blocksize(index);
	if(-1 == block_no || -1 == block_size)
	{
		STONAND_ERROR("Get block info error!\n");
		return -1;
	}
	
	pTmp = buf;
	cur_opstart = data_start;
	while(cur_opstart < data_end)
	{
		//test the current block is bad block or not
		if (nand_block_isbad(index, cur_opstart) < 0)
		{
			cur_opstart += block_size;
			data_end += block_size;
			data_end = data_end > mtdinfo->size ? mtdinfo->size : data_end;			
			block_no++;
			continue;
		}
		
		cur_blstart = block_size * block_no;
		cur_blend   = cur_blstart + block_size;
		cur_opend =  cur_blend > data_end ? data_end : cur_blend;
		cur_oplen = cur_opend - cur_opstart;
		
		STONAND_DEBUG("write data, from(0x%x), len(0x%x)\n",cur_opstart, cur_oplen);	

		//STONAND_DUMP(pTmp, 64);

		//write the whole block back
		lseek(pNandInfo->mtdfd, cur_opstart, SEEK_SET);
		ret = write(pNandInfo->mtdfd, pTmp, cur_oplen);							
		if(-1 == ret)
		{
			STONAND_ERROR("Write back to nand at 0x%x failed!\n", cur_opstart);
			return -1;
		}
		wsize += ret;

		cur_opstart += cur_oplen;
		pTmp += cur_oplen;
		block_no++;
	}

	oplen = data_end - data_start;
	pNandInfo->curr_addr[id] += oplen;

	return wsize;
}

/*
 * Name      : nand_read
 * parameter : 
 *		index: part index
 *		buf: read buffer
 *		size: read size, align to pagesize
 * reuturn   : -1 ==> failure
 *             else ==> bytes actually read
 *
*/
int nand_physical_read(UINT8 index, void* buf, UINT32 start,UINT32 size)
{
	int ret = -1;
	int id = 0, pid = 0;
	UINT32 data_start, data_end;
	UINT32 cur_blstart, cur_blend;
	UINT32 cur_opstart, cur_opend, cur_oplen;
	UINT32 rsize = 0;
	int block_no = 0;
	int block_size = 0;
	UINT8* pBuf = buf;
	nand_curinfo_t* pNandInfo = &nandinfo_arry[index];
	struct mtd_info_user *mtdinfo;
	
	if(NULL == buf)
		return -1;

	if(-1 == nand_is_ready(index))
		return -1;

	pid = getpid();
	id = get_id_index(index, pid);
	if(id>=NAND_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		return -1;
	}

	//STONAND_DEBUG("taskid:%d, cur_addr:0x%x, len:0x%x\n",id, pNandInfo->curr_addr[id],size);

	mtdinfo = pNandInfo->baseinfo;
	
	if (start%mtdinfo->writesize || size%mtdinfo->writesize)
	{
		STONAND_ERROR("start[0x%x] or size[0x%x] not align to pagesize[0x%x]\n",
			start, size, mtdinfo->writesize);
		return -1;	
	}
	
	data_start = start;
	data_end   = data_start + size;
	data_end = data_end > mtdinfo->size ? mtdinfo->size : data_end;

	cur_opend = data_end;
	
	block_no = nand_get_blockno(index, data_start);
	block_size = nand_get_blocksize(index);
	if(-1 == block_no || -1 == block_size)
	{
		STONAND_ERROR("Get block info error!\n");
		return -1;
	}

	cur_opstart = data_start;
	while(cur_opstart < data_end)
	{
		//test the current block is bad block or not
		if (nand_block_isbad(index, cur_opstart) < 0)
		{
			return -1;
		}
		cur_blstart = block_size * block_no;
		cur_blend   = cur_blstart + block_size;
		cur_opend =  cur_blend > data_end ? data_end : cur_blend;
		cur_oplen = cur_opend - cur_opstart;

		STONAND_DEBUG("read part[%d] from(0x%x) len(0x%x)\n",index,cur_opstart, cur_oplen);
		lseek(pNandInfo->mtdfd, cur_opstart, SEEK_SET);
		ret = read(pNandInfo->mtdfd, pBuf, cur_oplen);
		if(-1 == ret)
		{
			STONAND_ERROR("Read nand at 0x%x failed!\n", cur_opstart);
			return -1;
		}

		//STONAND_DUMP(pBuf, cur_oplen);

		pBuf += ret;	
		rsize += ret;

		cur_opstart += cur_oplen;
		block_no++;	
	}

	return rsize;
}

int nand_physical_write(UINT8 index, void* buf, UINT32 start,UINT32 size)
{
	int block_no = 0;
	int block_size = 0;
	int ret = -1;
	int id = 0, pid = 0;
	UINT32 cur_blstart, cur_blend;
	UINT32 cur_opstart, cur_opend, cur_oplen;
	UINT32 data_start, data_end;//the wanting start & end offset of nand flash
	UINT8* pTmp = NULL;
	UINT32 wsize = 0;
	nand_curinfo_t* pNandInfo = &nandinfo_arry[index];
	struct mtd_info_user *mtdinfo;
	UINT32 oplen = 0;
	
	if(NULL == buf)
		return -1;

	pid = getpid();
	id = get_id_index(index, pid);
	if(id>=NAND_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		return -1;
	}

	//STONAND_DEBUG("taskid:%d, cur_addr:0x%x, len:0x%x\n",id, pNandInfo->curr_addr[id],size);

	if(-1 == nand_is_ready(index))
		return -1;

	mtdinfo = pNandInfo->baseinfo;

	if (start%mtdinfo->writesize || size%mtdinfo->writesize)
	{
		STONAND_ERROR("write start[0x%x] or size[0x%x] not align to pagesize[0x%x]\n",
			start, size, mtdinfo->writesize);
		return -1;	
	}

	data_start = start;
	data_end = data_start + size;
	data_end = data_end > mtdinfo->size ? mtdinfo->size : data_end;

	block_no = nand_get_blockno(index, data_start);
	block_size = nand_get_blocksize(index);
	if(-1 == block_no || -1 == block_size)
	{
		STONAND_ERROR("Get block info error!\n");
		return -1;
	}
	
	pTmp = buf;
	cur_opstart = data_start;
	while(cur_opstart < data_end)
	{
		//test the current block is bad block or not
		if (nand_block_isbad(index, cur_opstart) < 0)
		{
			return -1;
		}
		
		cur_blstart = block_size * block_no;
		cur_blend   = cur_blstart + block_size;
		cur_opend =  cur_blend > data_end ? data_end : cur_blend;
		cur_oplen = cur_opend - cur_opstart;
		
		STONAND_DEBUG("write data, from(0x%x), len(0x%x)\n",cur_opstart, cur_oplen);	

		//STONAND_DUMP(pTmp, 64);

		//write the whole block back
		lseek(pNandInfo->mtdfd, cur_opstart, SEEK_SET);
		ret = write(pNandInfo->mtdfd, pTmp, cur_oplen);							
		if(-1 == ret)
		{
			STONAND_ERROR("Write back to nand at 0x%x failed!\n", cur_opstart);
			return -1;
		}
		wsize += ret;

		cur_opstart += cur_oplen;
		pTmp += cur_oplen;
		block_no++;
	}

	return wsize;
}

int nand_physical_erase(UINT8 index, INT32 block_no)
{
	erase_info_t fls_erase;
	nand_curinfo_t* pNandInfo;
	struct mtd_info_user *mtdinfo;		
	UINT32 blocksize, i;
	int ret;

	STONAND_DEBUG("erase part[%d] block[%d]\n",index, block_no);
	
	if(nand_is_ready(index) < 0)
	{
		STONAND_ERROR("Nand device %d is not ready, please open it first!\n",index);
		return -1;
	}

	pNandInfo = &nandinfo_arry[index];
	mtdinfo = pNandInfo->baseinfo;

	blocksize = mtdinfo->erasesize;

	if (block_no < 0)
	{
		/* erase all partition */

		for (i=0; i<mtdinfo->size/blocksize; i++)
		{
			fls_erase.start  = i*blocksize;	
			fls_erase.length = blocksize;
			ioctl(pNandInfo->mtdfd, MEMERASE,&fls_erase);	
		}				
	}
	else
	{
		/* erase a special block */
		fls_erase.start  = blocksize*block_no;	
		fls_erase.length = blocksize;
	
		ret = ioctl(pNandInfo->mtdfd, MEMERASE,&fls_erase);
		if(ret < 0)
		{
			STONAND_ERROR("Erase block at 0x%x failed!\n", fls_erase.start);
			return -1;
		}
	}
	return 0;
}
int nand_ioctl(UINT8 index, INT32 cmd, UINT32 param)
{
	int ret = 1;
	nand_curinfo_t* pNandInfo;
	
	switch(cmd)
	{
 		case GETMTDBASICINFO:			
			pNandInfo = &nandinfo_arry[index];			

			if (nandinfo_is_ready(index) < 0)
			{
				if (nand_open(index, O_RDONLY) > 0)
				{
				    nand_close(index);
			        *(struct mtd_info_user*)param = *pNandInfo->baseinfo;
				}
				else
				{
					ret = -1;
				}
			}
			else
			{
				*(struct mtd_info_user*)param = *pNandInfo->baseinfo;
			}
			//STONAND_DEBUG("baseinfo:0x%x\n",pNandInfo->baseinfo);
			//*(struct mtd_info_user*)param = *pNandInfo->baseinfo;
			break;
		default:
			break;			
	}

	return ret;
}


/*
 * parameter : 
 * reuturn   : -1 ==> failure
 *              1 ==> success
 *
*/
int nand_close(UINT8 index)
{
	int ret = 1;
	nand_curinfo_t* pNandInfo = NULL;

	//STONAND_DEBUG("close part[%d]\n",index);
	if(1 == nand_is_ready(index))
	{
		pNandInfo = &nandinfo_arry[index];
		//STONAND_DEBUG("close fd:0x%x\n",pNandInfo->mtdfd);
		ret = close(pNandInfo->mtdfd);
		if(-1 == ret)
		{
			STONAND_ERROR("close part[%d] fail!\n",index);
			return -1;
		}
		pNandInfo->mtdfd = -1;		
	}
	
	return ret;
}

/*
 * Name      : nand_check(UINT8 index,UINT32 start, UINT32 length)
 * parameter : start => start address to check nand flash have bad block or not
 * reuturn   : -1 ==> have bad block
 *             else ==> 0
 *
 */
int nand_check(UINT8 index, UINT32 start, UINT32 length)
{
	int block_size = 0;
	int i = 0;
	int bBB = 0;
	int check_num = 0;
	UINT32 check_start = 0;
	long long block_start = 0;

	if (nand_is_ready(index) < 0)
	{   
		STONAND_ERROR("Nand device is not ready, please open it first!\n");
		return -1; 
	}   

	nand_curinfo_t *pNandInfo = &nandinfo_arry[index];
	/*if (NULL == pNandInfo)
	{   
		STONAND_ERROR("%s NULL == pNandInfo \n", __FUNCTION__);
		return -1; 
	}*/

	block_size = nand_get_blocksize(index);
	if (-1 == block_size || block_size == 0)
	{   
		STONAND_ERROR("Get block info error!block_size=0x%x\n", block_size);
		return -1; 
	}   

	check_num = length/block_size;
	check_start = start;

	STONAND_PRINTF("%s start=0x%x length=%d check_num=%d\n", __FUNCTION__, start, length, check_num);
	for (i=0; i<check_num; i++)
	{   
		block_start = check_start;
		bBB = ioctl(pNandInfo->mtdfd, MEMGETBADBLOCK, &block_start);
		if (bBB)
		{
			STONAND_ERROR("%s check_start=0x%x badblock_num=%d\n",__FUNCTION__ ,check_start,(check_start/block_size));
			return -1;
		}
		check_start += block_size;
	}
	return 0;
}

unsigned long long nand_get_mtds_size(void)
{
	int mtdsz = 0; 
	int i = 0;
	struct mtd_info_user info_user = {0};

	for (i=0; i<MAX_PARTITION_NUM; i++) 
	{
		info_user.size = 0;
		if (nand_ioctl(i, GETMTDBASICINFO, (UINT32)&info_user) < 0) 
		{    
			STONAND_ERROR("can not get part[%d] info\n", i);
		}
		else
		{
			mtdsz += info_user.size;
		}
	}    
	return mtdsz;
}

