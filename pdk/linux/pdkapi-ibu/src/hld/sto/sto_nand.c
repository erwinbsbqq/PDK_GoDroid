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

#include <hld/sto/sto_nand.h>

#include <retcode.h>
//#include <api/libc/alloc.h>
//#include <api/libc/string.h>
//corei#include <api/libc/printf.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <compiler_common.h>
#include <mtd/mtd-user.h>
#include <ali_mtd_common.h>

#include <osal/osal.h>

#define NANDINDEX_PMILOC                1
#define NAND_TASK_SUPPORT_NUM_MAX		256
//#define NAND_PRINTF(...)		do{}while(0)
#define NAND_PRINTF     printf

typedef struct {
	UINT32		            curr_addr[NAND_TASK_SUPPORT_NUM_MAX];	/* Current operate address */
	struct mtd_info_user 	baseinfo;		                        /* nand Device totol space size, in K bytes */
	int                     mtdfd;                                  /* file descriptor of nand device with MTD */ 
	//UINT16                  flag;                                   /* nand device marked as be ready or not */
}nand_curinfo_t;

//static nand_curinfo_t* pNandInfo = NULL;
static nand_curinfo_t* nandinfo_arry[4] = {NULL};


/*
 * whether visit nand device from upper layer or not
*/
static int nand_is_ready(UINT8 index)
{
	nand_curinfo_t* pNandInfo = nandinfo_arry[index];
	int ret = (NULL == pNandInfo) ? -1: 1;
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
	
	nand_curinfo_t* pNandInfo = nandinfo_arry[index];
	int block_index = start/(pNandInfo->baseinfo.erasesize);//index is from 0 to large..
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
	
	nand_curinfo_t* pNandInfo = nandinfo_arry[index];
	return pNandInfo->baseinfo.erasesize;
}


/*
 * parameter : flags ==> O_RDONLY, O_WRONLY, O_RDWR, ...
 * reuturn   : -1 ==> fail
 *             else ==> success
 *
*/
int nand_open(UINT8 index, int flags)
{	
	int id = 0;
	int mtdfd = -1;
	char mtdname[64] = {0};
	nand_curinfo_t* pNandInfo = NULL;

	if(1 == nand_is_ready(index))//success if this mtd device is ready
		return nandinfo_arry[index]->mtdfd;
	
	pNandInfo = (nand_curinfo_t*)malloc(sizeof(nand_curinfo_t));
	if(NULL == pNandInfo)
	{
	        NAND_PRINTF("%s malloc(sizeof(nand_curinfo_t)) failed!\n",__FUNCTION__);
		return -1;
	}
	memset(pNandInfo, 0x0, sizeof(nand_curinfo_t));
	
	sprintf(mtdname,"/dev/mtd%d", index);
	mtdfd = open(mtdname,flags);
	if(-1 != mtdfd)
	{
	        
		pNandInfo->mtdfd = mtdfd;
                
		ioctl(mtdfd, MEMGETINFO, &(pNandInfo->baseinfo));
		/*
		if ((id = osal_task_get_current_id()) == OSAL_INVALID_ID)
			id = 0;
		if(id>=NAND_TASK_SUPPORT_NUM_MAX)
		{
			//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
			ASSERT(0);
		}
		pNandInfo->curr_addr[id] = 0;
		lseek(mtdfd, pNandInfo->curr_addr[id], SEEK_SET);
		*/
		nandinfo_arry[index] = pNandInfo;
                NAND_PRINTF("%s pNandInfo->mtdfd=0x%x\n",__FUNCTION__ ,nandinfo_arry[index]->mtdfd);
	}
        else
        {
            NAND_PRINTF("%s open %s failed!\n",__FUNCTION__,mtdname);
        }

	return mtdfd;
}


/*
 * Name      : nand_lseek(void* buf, UINT32 size)
 * parameter : whenence => SEEK_SET, SEEK_CUR, SEEK_END
 * reuturn   : -1 ==> failure
 *             else ==> position offset from the beginning
 *
*/
int nand_lseek(UINT8 index, int offset, int whenence)
{
	int id = 0;
	int new_addr;
	
	if(nand_is_ready(index) < 0)
	{
		NAND_PRINTF("Nand device is not ready, please open it first!\n");
		return -1;
	}

	nand_curinfo_t* pNandInfo = nandinfo_arry[index];

	if ((id = osal_task_get_current_id()) == OSAL_INVALID_ID)
	{
	        //NAND_PRINTF("%s osal_task_get_current_id == OSAL_INVALID_ID !\n",__FUNCTION__);
		id = 0;
	}
	if(id>=NAND_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		NAND_PRINTF("%s id>=NAND_TASK_SUPPORT_NUM_MAX\n",__FUNCTION__);
                return -1;
		//ASSERT(0);
	}

	switch(whenence)
	{
		case SEEK_SET:
			/* Great than totol size, seek to end */
			if (offset >= pNandInfo->baseinfo.size)
			{
			    pNandInfo->curr_addr[id] = pNandInfo->baseinfo.size - 1;
			}
			/* Common seek */
			else if (offset >= 0)
			{
			    pNandInfo->curr_addr[id] = offset;
			}
			break;

		case SEEK_CUR:
			new_addr = pNandInfo->curr_addr[id] + offset;
			/* Less than base address, seek to begin */
			if (new_addr < 0)
			{
			    pNandInfo->curr_addr[id] = 0;
			}
			/* Great than totol size, seek to end */
			else if (new_addr >= pNandInfo->baseinfo.size)
			{
			    pNandInfo->curr_addr[id] = pNandInfo->baseinfo.size - 1;
			}
			/* Common seek */
			else
			{
			    pNandInfo->curr_addr[id] = new_addr;
			}
			break;

		case SEEK_END:
			new_addr = pNandInfo->baseinfo.size + offset - 1;
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
			NAND_PRINTF("please check your whenence parameter!\n");
			return -1;		
	}
	
	return pNandInfo->curr_addr[id];
}

/*
 * Name      : nand_check(UINT8 index,UINT32 start, UINT32 length)
 * parameter : start => start address to check nand flash have bad block or not
 * reuturn   : -1 ==> have bad block
 *             else ==> 0
 *
*/
int nand_check(UINT8 index,UINT32 start, UINT32 length)
{
    int block_size=0;
    int i=0;
    int bBB=0;
    int check_num=0;
    UINT32 check_start=0;
    long long block_start=0;

    if(nand_is_ready(index) < 0)
    {
        NAND_PRINTF("Nand device is not ready, please open it first!\n");
        return -1;
    }

    nand_curinfo_t* pNandInfo = nandinfo_arry[index];

    if(NULL == pNandInfo)
    {
        NAND_PRINTF("%s NULL == pNandInfo \n",__FUNCTION__);
        return -1;
    }

    block_size = nand_get_blocksize(index);
    if(-1 == block_size ||block_size==0)
    {
    	NAND_PRINTF("Get block info error!block_size=0x%x\n",block_size);
    	return -1;
    }

    check_num=length/block_size;
    check_start = start;

    NAND_PRINTF("%s start=0x%x length=%d check_num=%d\n",__FUNCTION__ ,start,length,check_num);
    for(i=0;i<check_num;i++)
    {
        block_start = check_start;

        bBB = ioctl(pNandInfo->mtdfd,MEMGETBADBLOCK,&block_start);
        if(bBB)
        {               
            NAND_PRINTF("%s check_start=0x%x badblock_num=%d\n",__FUNCTION__ ,check_start,(check_start/block_size));
            return -1;
        }
        check_start +=block_size;
    }

    return 0;
}

/*
 * Name      : nand_erase(UINT8 index,UINT32 start, UINT32 length)
 * parameter : start => start address to erase
 * reuturn   : -1 ==> failure
 *             else ==> 0
 *
*/
int nand_erase(UINT8 index,UINT32 start, UINT32 length)
{
    int ret=0;
    erase_info_t fls_erase;
    int block_size=0;
    int i=0;
    int bBB=0;
    int erase_num=0;
    UINT32 erase_start=0;
    long long block_start=0;

    if(nand_is_ready(index) < 0)
    {
        NAND_PRINTF("Nand device is not ready, please open it first!\n");
        return -1;
    }

    nand_curinfo_t* pNandInfo = nandinfo_arry[index];

    if(NULL == pNandInfo)
    {
        NAND_PRINTF("%s NULL == pNandInfo \n",__FUNCTION__);
        return -1;
    }

    block_size = nand_get_blocksize(index);
    if(-1 == block_size ||block_size==0)
    {
    	NAND_PRINTF("Get block info error!block_size=0x%x\n",block_size);
    	return -1;
    }

    memset(&fls_erase,0,sizeof(erase_info_t));

    erase_num=length/block_size;
    erase_start = start;
    fls_erase.length = block_size;
    NAND_PRINTF("%s start=0x%x length=%d erase_num=%d\n",__FUNCTION__ ,start,length,erase_num);
    for(i=0;i<erase_num;i++)
    {
        fls_erase.start  = erase_start;
        block_start = erase_start;

        bBB = ioctl(pNandInfo->mtdfd,MEMGETBADBLOCK,&block_start);
        if(bBB)
        {               
            erase_start +=block_size;
            continue;
        }
        ret = ioctl(pNandInfo->mtdfd, MEMERASE,&fls_erase);
        //if(-1 == ret)
        if(ret)
        {
            NAND_PRINTF("%s Erase erea at 0x%x failed!length=%d\n",__FUNCTION__ ,fls_erase.start,fls_erase.length);
            //return -2;//for bad block process
        }
        erase_start +=block_size;
    }

    return 0;
}



/*
 * Name      : nand_read(void* buf, UINT32 size)
 * parameter : 
 * reuturn   : -1 ==> failure
 *             else ==> bytes actually read
 *
*/
int nand_read(UINT8 index, void* buf, UINT32 size)
{
	int ret = -1;
	int id = 0;
	int bBB = 0; //0=>not bad block;1=>is bad block
	long long data_start, data_end;
	long long cur_blstart, cur_blend;
	long long cur_opstart, cur_opend, cur_oplen;
	UINT32 rsize = 0;
	int block_no = 0;
	int block_size = 0;
	UINT8* pBuf = buf;
	nand_curinfo_t* pNandInfo = nandinfo_arry[index];
	
	if(NULL == pNandInfo || NULL == buf)
	{
	        NAND_PRINTF("%s NULL == pNandInfo || NULL == buf\n",__FUNCTION__);
		return -1;
	}
	if ((id = osal_task_get_current_id()) == OSAL_INVALID_ID)
	{
	        //NAND_PRINTF("%s osal_task_get_current_id == OSAL_INVALID_ID !\n",__FUNCTION__);
	        //return -1;
		id = 0;
	}
	if(id>=NAND_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		 NAND_PRINTF("%s id>=NAND_TASK_SUPPORT_NUM_MAX\n",__FUNCTION__);
	        return -1;
		//ASSERT(0);
	}

	data_start = pNandInfo->curr_addr[id];
	data_end   = data_start + size;
	data_end = data_end > pNandInfo->baseinfo.size ? pNandInfo->baseinfo.size : data_end;

	block_no = nand_get_blockno(index, data_start);
	block_size = nand_get_blocksize(index);
	if(-1 == block_no || -1 == block_size ||block_size==0)
	{
		NAND_PRINTF("Get block info error!block_no=%d block_size=0x%x\n",block_no,block_size);
		return -1;
	}

	cur_opstart = data_start;
	while(cur_opstart < data_end)
	{
		//test the current block is bad block or not
		bBB = ioctl(pNandInfo->mtdfd,MEMGETBADBLOCK,&cur_opstart);
		if(bBB)
		{
		        NAND_PRINTF("%s bBB=0x%x MEMGETBADBLOCK at %lld badblock_num=%d\n",__FUNCTION__ ,bBB,cur_opstart,cur_opstart/block_size);
		        return -2;//for bad block process
			//cur_opstart += block_size;
			//data_end += block_size;
			//block_no++;
			//continue;
		}
		cur_blstart = block_size * block_no;
		cur_blend   = cur_blstart + block_size;
		cur_opend =  cur_blend > data_end ? data_end : cur_blend;
		cur_oplen = cur_opend - cur_opstart;

		lseek(pNandInfo->mtdfd, cur_opstart, SEEK_SET);
		ret = read(pNandInfo->mtdfd, pBuf, cur_oplen);
		if(-1 == ret)
		{
			NAND_PRINTF("Read nand at 0x%x failed!\n", cur_blstart);
			return -1;
		}
		
		pBuf += ret;	
		rsize += ret;

		cur_opstart = cur_opend;
		block_no++;	
	}

	pNandInfo->curr_addr[id] = cur_opend;		

	return rsize;
	/*
	lseek(pNandInfo->mtdfd, pNandInfo->curr_addr[id], SEEK_SET);	
	ret = read(pNandInfo->mtdfd, buf, size);	
	if(-1 != ret)	
	{	
		pNandInfo->curr_addr[id] += ret;
	}
	return ret;*/
}

/*
 * Name      : nand_write(void* buf, UINT32 size)
 * parameter : 
 * reuturn   : -1 ==> failure
 *             else ==> bytes actually write
 *
*/
int nand_write(UINT8 index, void* buf, UINT32 size)
{
	int block_no = 0;
	int block_size = 0;
	int ret = -1;
	int id = 0;
	long long cur_blstart, cur_blend;
	long long cur_opstart, cur_opend;//the mtd ioctl must use long long
	long long data_start, data_end;//the wanting start & end offset of nand flash
	UINT8* pBuf = NULL;
	UINT8* pTmp = NULL;
	UINT32 bbnum = 0;
	erase_info_t fls_erase;
	UINT32 wsize = 0;
	int bBB = 0;//whether it is bad block or not: 1=>bad;0=>not bad
	nand_curinfo_t* pNandInfo = nandinfo_arry[index];
	UINT32 oplen = 0;
	
	if(NULL == pNandInfo || NULL == buf)
	{
	        NAND_PRINTF("%s NULL == pNandInfo || NULL == buf\n",__FUNCTION__);
		return -1;
	}

	if ((id = osal_task_get_current_id()) == OSAL_INVALID_ID)
	{
	        //NAND_PRINTF("%s osal_task_get_current_id == OSAL_INVALID_ID !\n",__FUNCTION__);
	        //return -1;
		id = 0;
	}
	if(id>=NAND_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		 NAND_PRINTF("%s id>=NAND_TASK_SUPPORT_NUM_MAX\n",__FUNCTION__);
	        return -1;
		//ASSERT(0);
	}


	data_start = pNandInfo->curr_addr[id];
	data_end   = data_start + size;
	data_end = data_end > pNandInfo->baseinfo.size ? pNandInfo->baseinfo.size : data_end;

	block_no = nand_get_blockno(index, data_start);
	block_size = nand_get_blocksize(index);
	if(-1 == block_no || -1 == block_size ||block_size==0)
	{
		NAND_PRINTF("Get block info error!block_no=%d block_size=0x%x\n",block_no,block_size);
		return -1;
	}

	pBuf = (UINT8*)malloc(block_size);
	if(NULL == pBuf)
	{
		NAND_PRINTF("malloc memory error!\n");
		return -1;
	}
	pTmp = pBuf;

	//UINT8* pBuf2 = (UINT8*)malloc(block_size);
	
	cur_opstart = data_start;
	while(cur_opstart < data_end)
	{
		//test the current block is bad block or not
		bBB = ioctl(pNandInfo->mtdfd,MEMGETBADBLOCK,&cur_opstart);
		if( bBB)
		{
		        NAND_PRINTF("%s bBB=0x%x MEMGETBADBLOCK at %lld badblock_num=%d\n",__FUNCTION__ ,bBB,cur_opstart,cur_opstart/block_size);
                        if(pBuf)
		            free(pBuf);
		        return -2;//for bad block process
			//cur_opstart += block_size;
			//data_end += block_size;
			//block_no++;
			//bbnum++;
			//continue;
		}
	
		cur_blstart = block_size * block_no;
		cur_blend   = cur_blstart + block_size;
		cur_opend =  cur_blend > data_end ? data_end : cur_blend;

		//read a whole block into memory and modify it
		lseek(pNandInfo->mtdfd, cur_blstart, SEEK_SET);
		ret = read(pNandInfo->mtdfd, pBuf, block_size);
		if(-1 == ret)
		{
			NAND_PRINTF("Read block at 0x%x failed!\n", cur_blstart);
                        if(pBuf)
                            free(pBuf);
			return -1;
		}
		pTmp = pBuf + cur_opstart % block_size;
		while(pTmp < pBuf+cur_opend-cur_opstart)
		{
			//*pTmp++ = *(UINT8*)buf++;
			*(pTmp++) = *(UINT8*)(buf++);
		}
		//erase the whole block
		fls_erase.start  = cur_blstart;
		fls_erase.length = block_size;
		ret = ioctl(pNandInfo->mtdfd, MEMERASE,&fls_erase);
		if(-1 == ret)
		{
			NAND_PRINTF("Erase block at 0x%x failed!badblock_num=%d\n", cur_blstart,cur_blstart/block_size);
                        if(pBuf)
                            free(pBuf);
                        return -2;//for bad block process
                        #if 0
			ret = ioctl(pNandInfo->mtdfd, MEMSETBADBLOCK,&cur_opstart);
			if(-1 == ret)
			{
				NAND_PRINTF("Set bad block at 0x%x failed!\n", cur_blstart);
				return -1;
			}
			cur_opstart += block_size;
			data_end += block_size;
			block_no++;
			bbnum++;
			continue;
                        #endif
		}
		//write the whole block back
		lseek(pNandInfo->mtdfd, cur_blstart, SEEK_SET);
		ret = write(pNandInfo->mtdfd, pBuf, block_size);
		/*//for test
		lseek(pNandInfo->mtdfd, cur_blstart, SEEK_SET);
		ret = read(pNandInfo->mtdfd, pBuf2, block_size);
		if(0 != memcmp(pBuf,pBuf2,block_size))
			SDBBP();
		//test end*/
		
		if(-1 == ret)
		{
			NAND_PRINTF("Write back to nand at 0x%x failed!\n", cur_blstart);
                        if(pBuf)
                            free(pBuf);
			return -1;
		}
		wsize += ret;

		cur_opstart = cur_opend;
		block_no++;
	}

	oplen = data_end - data_start;
	pNandInfo->curr_addr[id] += oplen;//pNandInfo->curr_addr[id] = cur_opend;

	if(pBuf)
		free(pBuf);
	pBuf = pTmp = NULL;

	return oplen;//return wsize;
}


int nand_ioctl(UINT8 index, INT32 cmd, UINT32 param)
{
	int ret = 1;
        struct part_check_user part_user;
        struct part_user_info *nand_part_info=NULL;
	
	if(nand_is_ready(index) < 0)
	{
		NAND_PRINTF("NAND device is not ready!\n");
		return -1;
	}
	
	nand_curinfo_t* pNandInfo = nandinfo_arry[index];
	switch(cmd)
	{
		case MEMGETPMI: //mtd will explain this case
			ret = ioctl(nandinfo_arry[NANDINDEX_PMILOC]->mtdfd, cmd, param);
			if(-1 == ret)
			{
				NAND_PRINTF("copy PMI info to user space failed!\n");
			}
			break;
		case MEMWRITEPMI:
			ret = ioctl(nandinfo_arry[NANDINDEX_PMILOC]->mtdfd, cmd, param);
			if(-1 == ret)
			{
				NAND_PRINTF("Set PMI info to nand flash failed!\n");
			}
                        break;
		case GETMTDBASICINFO:
			*(struct mtd_info_user*)param = pNandInfo->baseinfo;
			break;
		case MEMGETBADBLOCK:
			ret = ioctl(pNandInfo->mtdfd, cmd, param);// 1 => bad block ;0 => good block
			break;
                case MEMRESETNANDSTATS:                          
                         part_user.part_num_1 = 0xFF;
                         part_user.part_num_2 = 0xFF;
                         ret = ioctl(pNandInfo->mtdfd, cmd, &part_user);
                         NAND_PRINTF("%s ioctl MEMRESETNANDSTATS ret%d\n",__FUNCTION__,ret);
                        break;
                case ALI_UPDATE_MTDINFO:    
                        nand_part_info = (struct part_user_info *)param;
                        ret = ioctl(pNandInfo->mtdfd, ALI_UPDATE_MTDINFO, nand_part_info);
                        printf("ALI_UPDATE_MTDINFO retioctl = %d \n", ret);
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
	
	if(1 == nand_is_ready(index))
	{
		pNandInfo = nandinfo_arry[index];
		ret = close(pNandInfo->mtdfd);
		if(-1 == ret)
			return ret;
		
		free(pNandInfo);
		pNandInfo = NULL;
	}
	
	return ret;
}


