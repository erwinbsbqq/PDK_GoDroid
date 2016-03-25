/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 - 2003 Copyright (C)
*
*    File:    sto.c
*
*    Description:    This file contains all functions definition
*		             of storage device driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	May.29.2003      Justin Wu       Ver 0.1    Create file.
*****************************************************************************/

#include <adr_retcode.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <err/errno.h>

#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>

//#include <linux/compiler.h>
#include <mtd/mtd-user.h>
#include <hld/adr_hld_dev.h>

#include <hld/sto/adr_sto.h>
#include <hld/sto/adr_sto_dev.h>

#if 0
#define STO_PRINTF(fmt, args...) ADR_DBG_PRINT(STO, fmt, ##args)
#else
#define STO_PRINTF(...)		do{}while(0)
#endif

///////////////////////// NOTE /////////////////////////
/* HLD_DEV_TYPE_STO  stand for nor flash
 * HLD_DEV_TYPE_NAND stand for nand flash
 * */
///////////////////////// NOTE /////////////////////////

struct sto_device *m_sto_dev = NULL;
OSAL_ID sto_mutex_id = OSAL_INVALID_ID;
static char sto_sflash_local_name[HLD_MAX_NAME_SIZE] = "STO_SFLASH_0";

static char *grep_last_strkey(const char *strHead, const char *strTail, const char *strKey)
{
	unsigned int pos, keyLen;

	if (strHead == NULL || strTail == NULL || strKey == NULL)
	{
		return NULL;
	}

	keyLen = strlen(strKey);
	pos = (unsigned int)strTail - keyLen;
	while (strncmp((UINT8 *)pos, strKey, keyLen) != 0 && pos >= (unsigned int)strHead)
	{
	    --pos;
	}
	if (pos < (unsigned int)strHead)
	{
	    return NULL;
	}
	STO_PRINTF("[APPMTD]commence string: %s\n", pos);
	return (char *)pos;
}

static int find_sflash_mtd_devnum(const char *mtd_name, char *mtd_strnum, unsigned int maxlen)
{
	char *come = NULL;
	char *to = NULL;
	int fd = -1, rd = 0;
	int ret = -ERR_FAILURE;
	unsigned int len;
	unsigned char buffer[1024];

	if (NULL == mtd_name)
	{
		STO_PRINTF("[ %s %d ], device name is NULL\n", __FUNCTION__, __LINE__);
		return ERR_NO_DEV;
	}

	memset(buffer, 0x0, sizeof(buffer));	
	fd = open("/proc/mtd", O_RDONLY|O_NONBLOCK| O_CLOEXEC);
	if (fd < 0)
	{
		STO_PRINTF("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		return ERR_FAILURE;
	}	

	rd = read(fd, buffer, sizeof(buffer));
	if (-1 == rd)
	{
		STO_PRINTF("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		ret = ERR_FAILURE;
		goto END;		
	}

	come = strstr(buffer, mtd_name);
	if (NULL == come)	
	{
		STO_PRINTF("[ %s %d ], Fail!, dev name = %s\n", __FUNCTION__, __LINE__, mtd_name);
		ret = ERR_FAILURE;
		goto END;
	}

	come = grep_last_strkey(buffer, come, "mtd");
	if (NULL == come)
	{
		STO_PRINTF("[ %s %d ], Fail!, dev name = %s\n", __FUNCTION__, __LINE__, mtd_name);
		ret = ERR_FAILURE;
		goto END;
	}

	to = strchr(come, ':');
	if (NULL == to)
	{
		STO_PRINTF("[ %s %d ], Fail!, dev name = %s\n", __FUNCTION__, __LINE__, mtd_name);
		ret = ERR_FAILURE;
		goto END;
	}

	len = to - come;
	if (len > (maxlen-1))
	{
		len = maxlen - 1;
	}
	strncpy(mtd_strnum, come, len);
	mtd_strnum[len] = '\0';
	STO_PRINTF("[APPMTD] %s, strlen = %d\n", mtd_strnum, len);

	ret = SUCCESS;

END:
	close(fd);

	return ret;
}

#define FLASH_ADDR_MASK     0x3FFFFFF
static UINT8 ali_fls_verify[4096];
static UINT8 *ali_def_sector_buf = NULL;
static int sto_flash_verify(struct sto_device *dev, UINT32 offset, UINT8 *data, UINT32 len)
{
	int ret = 0;
	UINT32 i;
	UINT8 dst, src;

	while(len>4096)
	{
		//STO_PRINTF("\t%s %d\n",__FUNCTION__, __LINE__);
		sto_get_data(dev, ali_fls_verify, offset, 4096);
		for (i=0; i<4096; i++)
		{
			dst = ali_fls_verify[i];
			src = *data++;
			dst ^= src;
			if (dst == 0)
				continue;
			ret = 1;
			if (dst & src)
			{
				ret = 2;  //Need erase;
				STO_PRINTF("fls need erase, ret: %d\n",ret);
				return ret;
			}
		};
		len-=4096;
		offset+=4096;
	}

	//STO_PRINTF("\t%s %d\n",__FUNCTION__, __LINE__);
	sto_get_data(dev, ali_fls_verify, offset, len);
	for (i=0; i<len; i++)
	{
		dst = ali_fls_verify[i];
		src = *data++;
		dst ^= src;
		if (dst == 0)
			continue;
		ret = 1;
		if (dst & src)
		{
			ret = 2;  //Need erase;
			STO_PRINTF("fls need erase, ret: %d\n",ret);
			return ret;
		}
	};

	STO_PRINTF("fls do not need erase, ret: %d\n", ret);
	return ret;
}

static int sto_flash_change(struct sto_device *dev, unsigned long flag, 
	unsigned char *data, unsigned int len, unsigned char *buffer)
{
	int rcode;
	unsigned char *p;
	unsigned char *oper_p;
	unsigned long oper_start, oper_end, oper_len, l, pre_fix_l, post_fix_l;
	unsigned long target_start, target_end;
	unsigned int sector_no;
	unsigned long sector_start, sector_end, sector_size, data_tail;
	int ret;
	erase_info_t fls_erase;
	//struct flash_private *tp = (struct flash_private *)dev->priv;
	
	data_tail = (unsigned long)data + len;
	/* first of all, try directly write */
	target_start = flag & FLASH_ADDR_MASK;
		
#if 0//for sflash, the write operation will never compare the final content, it's dangerous.		
	MUTEX_ENTER();
	ret = tp->write(target_start, data, len);
	MUTEX_LEAVE();
	if (ret == 0)
	{
		return len;
	}
#endif
	STO_PRINTF("%s dev->sector_buffer = 0x%08x \n",__FUNCTION__,buffer);
	/* check input data validation */
	if (target_start >= dev->totol_size)
	{
		STO_PRINTF("%s, offset error.0x%08x\n",__FUNCTION__, target_start);
		return ERR_FLASH_OFFSET_ERROR;
	}
	/* limit input data to proper value */
	target_end = target_start + len;
		
	if (target_end > dev->totol_size)
	{
		target_end = dev->totol_size;
		len = target_end - target_start;
	}
	/* begin to write sector by sector */
	//STO_PRINTF("%s: target_start 0x%08x\n\n", __FUNCTION__, target_start);
	//STO_PRINTF("%s: target_end = 0x%08x\n\n",__FUNCTION__,target_end);	
	
	//STO_PRINTF("%s: ALI_FLASH_SECTOR_ALIGN %d\n", __FUNCTION__, ALI_FLASH_SECTOR_ALIGN);
	sector_no = ioctl((int)dev->priv,ALI_FLASH_SECTOR_ALIGN, target_start);
	//STO_PRINTF("%s: sector_no 0x%08x\n\n", __FUNCTION__, sector_no);
	
	//STO_PRINTF("%s: ALI_FLASH_SECTOR_START %d\n", __FUNCTION__, ALI_FLASH_SECTOR_START);
	sector_start = ioctl((int)dev->priv,ALI_FLASH_SECTOR_START, sector_no);
	//STO_PRINTF("%s: sector_start 0x%08x\n\n", __FUNCTION__, sector_start);
	
	
	while (target_start < target_end)
	{
		/* get information about current sector */
		//STO_PRINTF("%s: ALI_FLASH_SECTOR_SIZE %d\n", __FUNCTION__, ALI_FLASH_SECTOR_SIZE);
		sector_size = ioctl((int)dev->priv,ALI_FLASH_SECTOR_SIZE, sector_no);
		
		sector_end = sector_start + sector_size;
		STO_PRINTF("%s: sector_size 0x%08x, \n\n", __FUNCTION__, sector_size);
		/* get information about current operation */
		oper_start = target_start > sector_start ? target_start : sector_start;
		oper_end = target_end < sector_end ? target_end : sector_end;
		oper_len = oper_end - oper_start;
		STO_PRINTF("%s: oper_start 0x%08x, oper_len 0x%08x\n\n", __FUNCTION__, oper_start, oper_len);
		oper_p = data;		
		
		pre_fix_l = post_fix_l = 0;
		/* check flash operation */
		ret = sto_flash_verify(dev, oper_start, data, oper_len);
				
		switch (ret)
		{
			case 2:  /* need erase */
				
				if (flag & STO_FLAG_AUTO_ERASE)
				{
					/* check if need combine data */
					if (oper_len != sector_size)
					{
						p = oper_p = buffer;
						/* copy pre-fix block from flash */
						pre_fix_l = l = oper_start - sector_start;
						if (l != 0)
						{
							/* check if combination buffer existed */
							if (oper_p == NULL)
							{
								STO_PRINTF("%s, error! No operate buffer.\n",__FUNCTION__);
								return ERR_FLASH_NO_BUFFER;
							}
							sto_get_data(dev, p, sector_start, l);
							p += l;
						}
						if (oper_p != NULL)
						{
							/* copy real data */
							memcpy(p, data, oper_len);
							p += oper_len;
						}
						if (flag & STO_FLAG_SAVE_REST)
						{
							/* copy post-fix block from flash */
							post_fix_l = l = sector_end - oper_end;
							if (l != 0)
							{
								/* check if combination buffer existed */
								if (oper_p == NULL)
								{
									STO_PRINTF("%s, error! No operate buffer.\n",__FUNCTION__);
									return ERR_FLASH_NO_BUFFER;
								}
								sto_get_data(dev, p, oper_end, l);
								p += l;
							}
						}
						/* update operation information */
						if (oper_p == NULL)
						{
							oper_p = data;
						}
						else
						{
							oper_len = p - oper_p;
						}
						oper_start = sector_start;
					}
					else
					{
						ret = 1; // need to patch linux compile optimization.
					}
					/* erase sector */
					//MUTEX_ENTER();
					//rcode = tp->erase_sector(sector_start);
					STO_PRINTF("sto change data: erase sector: 0x%08x, len:%08x\n", sector_start, sector_size);
					fls_erase.start = sector_start;
					fls_erase.length = sector_size;
					rcode = ioctl((int)dev->priv, MEMERASE, &fls_erase);
					//MUTEX_LEAVE();
					if (SUCCESS != rcode)
					{
						STO_PRINTF("sto change data: erase sector: 0x%08x, len:%08x failed.\n", sector_start, sector_size);
						return ERR_FLASH_NEED_ERASE;
					}
				}
				else
				{
					STO_PRINTF("sto change data: need set auto erase flag.\n");
					return ERR_FLASH_NEED_ERASE;
				}
			case 1:  /* need write */
				//MUTEX_ENTER();
				//by eric, patch linux compile optimization.
				if ((1 == ret) && (oper_start&0x03))
				{
					p = buffer;
					oper_p = buffer;
					sto_get_data(dev, p, (oper_start&(~0x3)), (oper_start&(0x3)));
					p += (oper_start&(0x3));
					memcpy(p, data, oper_len);
					oper_len += (oper_start&(0x3));
					pre_fix_l = (oper_start&(0x3));
				}
				STO_PRINTF("%s: seek 0x%08x\n", __FUNCTION__, (oper_start&(~0x3)));
				lseek((int)dev->priv, (oper_start&(~0x3)), SEEK_SET);
				STO_PRINTF("%s: write  oper_start 0x%08x, oper_len 0x%08x, oper_p 0x%08x\n", __FUNCTION__, (oper_start&(~0x3)), oper_len, oper_p);
				rcode = write((int)dev->priv, oper_p, oper_len);   // write
				//MUTEX_LEAVE();
				if (rcode < 0)
				{
					//libc_printf("ret: %d   ", ret);
					STO_PRINTF("sto change data: write 0x%08x, len:0x%08x failed.\n", oper_p, oper_len);
					return ERR_FLASH_WRITE_FAIL;
				}
				/* verify the result */
				/*ret = tp->verify(oper_start, oper_p, oper_len);
				if (ret != 0)
					return ERR_FLASH_WRITE_FAIL;
				*/
			break;
		}

		/* advance to next data block (sector) */
		if (oper_len >= pre_fix_l)
		{
			oper_len -= pre_fix_l;
		}
		if (oper_len >= post_fix_l)
		{
			oper_len -= post_fix_l;
		}
		
		target_start += oper_len;
		if (target_start >= (dev->totol_size))  //check if target_start overflow: unsigned long
		{
			break;
		}
		if (((unsigned long)data+oper_len) >= data_tail)    //check if data pointer overflow
		{
			break;
		}
		
		data += oper_len;
		sector_start = sector_end;
		sector_no++;
	}
	STO_PRINTF("%s: return 0x%08x(%d)\n", __FUNCTION__, len, len);
	return len;
}

static INT32 sto_flash_auto_sec_erase(struct sto_device *dev, 
      		UINT32 target_start, INT32 len_in_k)
{
	unsigned char *p;
	unsigned char *oper_p;
	unsigned long oper_start, oper_end, oper_len, l;
	unsigned long target_end, sector_start;
	unsigned int sector_no;
	unsigned long sector_end, sector_size;
	unsigned long oper_start_tmp = 0, oper_len_tmp = 0;
	int ret;
	int rcode;
	erase_info_t fls_erase;
	STO_PRINTF("%s start:0x%08x, len: 0x%08x\n\n",__FUNCTION__, target_start, len_in_k);
	//struct flash_private *tp = (struct flash_private *)dev->priv;

	/* check input data validation */
	if (target_start >= dev->totol_size)
	{
		STO_PRINTF("%s, offset error.0x%08x\n\n",__FUNCTION__, target_start);
		return ERR_FLASH_OFFSET_ERROR;
	}
	/* limit input data to proper value */
	target_end = target_start + (len_in_k << 10);
	if (target_end > dev->totol_size)
	{
		target_end = dev->totol_size;
	}
	/* begin to erase sector by sector */
	sector_no = ioctl((int)dev->priv,ALI_FLASH_SECTOR_ALIGN, target_start); //cari chen
	sector_start = ioctl((int)dev->priv,ALI_FLASH_SECTOR_START, sector_no); //cari chen
	STO_PRINTF("target_start:0x%08x, target_end:0x%08x\n\n",target_start, target_end);
	while (target_start < target_end)
	{
		/* get information about current sector */
		sector_size = ioctl((int)dev->priv,ALI_FLASH_SECTOR_SIZE, sector_no);//sto_flash_sector_size(dev,sector_no);
		sector_end = sector_start + sector_size;
		/* get information about current operation */
		oper_start = target_start > sector_start ? target_start : sector_start;
		oper_end = target_end < sector_end ? target_end : sector_end;
		oper_len = oper_end - oper_start;
		oper_p = NULL;
		/* check if need keep data */
		STO_PRINTF("%d, oper_start:0x%08x, oper_end:0x%08x, oper_len:0x%08x\n\n",__LINE__, oper_start, oper_end, oper_len);
		if (oper_len != sector_size)
		{
			p = oper_p = dev->sector_buffer;
			STO_PRINTF("%s dev->sector_buffer = 0x%08x \n",__FUNCTION__,dev->sector_buffer);
			/* copy pre-fix block from flash */
			l = oper_start - sector_start;
			if (l != 0)
			{
				/* check if combination buffer existed */
				if (oper_p == NULL)
				{
					STO_PRINTF("%s, error! No operate buffer.\n",__FUNCTION__);
					return ERR_FLASH_NO_BUFFER;
				}
				//STO_PRINTF("\t%s %d\n",__FUNCTION__, __LINE__);
				sto_get_data(dev, p, sector_start, l);
				
				p += l;
			}
			if (oper_p != NULL)
			{
				/* set erase area */
				memset(p, 0xFF, oper_len);
				p += oper_len;
			}
			if (dev->flag & STO_FLAG_SAVE_REST)
			{
				/* copy post-fix block from flash */
				l = sector_end - oper_end;
				if (l != 0)
				{
					/* check if combination buffer existed */
					if (oper_p == NULL)
					{
						STO_PRINTF("%s, error! No operate buffer.\n",__FUNCTION__);
						return ERR_FLASH_NO_BUFFER;
					}
					//STO_PRINTF("\t%s %d\n",__FUNCTION__, __LINE__);
					sto_get_data(dev, p, oper_end, l);

					p += l;
				}
			}
			/* update operation information */
			if (oper_p != NULL)
				oper_len_tmp = p - oper_p;
			oper_start_tmp = sector_start;
		}
		//MUTEX_ENTER();
		/* erase sector */
		//rcode = tp->erase_sector(sector_start);
		fls_erase.start=sector_start;
		fls_erase.length=sector_size;
		STO_PRINTF("sto erase sec addr 0x%08x: l 0x%08x\n\n", sector_start, sector_size);
		rcode=ioctl((int)dev->priv, MEMERASE, &fls_erase);
		STO_PRINTF("%d, oper_start:0x%08x, oper_end:0x%08x, oper_len:0x%08x\n\n",__LINE__, oper_start, oper_end, oper_len);
		
		//MUTEX_LEAVE();
		if(SUCCESS!=rcode)
		{
			STO_PRINTF("sto erase sec addr %08x, failed.\n", sector_start);
			return ERR_FLASH_NEED_ERASE;
		}
		/* check if need write back */
		if((oper_p != NULL)&&(0!=oper_len_tmp))
		{
			//MUTEX_ENTER();
			//ret = tp->write(oper_start, oper_p, oper_len
			STO_PRINTF("sto write back addr 0x%08x, len:0x%08x.\n", oper_start_tmp, oper_len_tmp);
			lseek((int)dev->priv, oper_start_tmp, SEEK_SET);
			ret = write((int)dev->priv, oper_p, oper_len_tmp);
			//MUTEX_LEAVE();
			if (ret < 0)
			{
				STO_PRINTF("sto write back addr 0x%08x, len:0x%08x, failed.\n", oper_start_tmp, oper_len_tmp);
				return ERR_FLASH_WRITE_FAIL;
			}
		}
		STO_PRINTF("%d, oper_start:0x%08x, oper_end:0x%08x, oper_len:0x%08x\n\n",__LINE__, oper_start, oper_end, oper_len);
		/* advance to next data block (sector) */
		target_start += oper_len;
		if (target_start >= (dev->totol_size))    ////check if target_start overflow: UINT32
		{
			break;
		}
		
		sector_start = sector_end;
		sector_no++;
	}

	return SUCCESS;
}


void sto_mutex_enter(void)
{
	if(OSAL_INVALID_ID==sto_mutex_id)
	{
		sto_mutex_id = osal_mutex_create();
		if(sto_mutex_id==OSAL_INVALID_ID)
			return;
	}
	osal_mutex_lock(sto_mutex_id, OSAL_WAIT_FOREVER_TIME);
}

void sto_mutex_exit(void)
{
	osal_mutex_unlock(sto_mutex_id);
}


/*
 * 	Name		:   sto_open()
 *	Description	:   Open a storage device
 *	Parameter	:	struct sto_device *dev		: Device to be openned
 *	Return		:	INT32 						: Return value
 *
 */
INT32 sto_open(struct sto_device *dev)
{
	INT32 result = SUCCESS;
	int fls_hdl = 0;
	char sflash_devnum[32] = {0};
	char mtd_devnum[64] = {0};
	struct mtd_info_user fls_info;
	
	if (NULL == dev)
	{   
		STO_PRINTF("sto hld device is NULL!\n");
		return RET_FAILURE;
	}

	/* If openned already, exit */
	if (dev->flags & HLD_DEV_STATS_UP)
	{
		//STO_PRINTF("sto_open: warning - device %s openned already!\n", dev->name);
		return SUCCESS;
	}

#ifdef DATABASE_NAND
	if (HLD_DEV_TYPE_STO == dev->type)
#endif
	{
		//fls_hdl = open("/dev/mtd6", O_RDWR);
		if (SUCCESS == find_sflash_mtd_devnum("sflash", sflash_devnum, 32))
		{
			sprintf(mtd_devnum, "/dev/%s", sflash_devnum);
			fls_hdl = open(mtd_devnum, O_RDWR| O_CLOEXEC);
		}
		else
		{
			STO_PRINTF("find sflash mtd devnum fail\n");
			return RET_FAILURE;
		}

		if (fls_hdl <= 0)
		{
			STO_PRINTF("open flash handle fail\n");
			return RET_FAILURE;
		}	

		STO_PRINTF("\n%s : open flash handle success\n",__FUNCTION__);

		ioctl(fls_hdl, MEMGETINFO, &fls_info);
		dev->priv          = (void *)fls_hdl;
		dev->totol_size    = fls_info.size;
		dev->flag          = (STO_FLAG_AUTO_ERASE|STO_FLAG_SAVE_REST);
		ali_def_sector_buf = malloc(0x10000);
		dev->sector_buffer = ali_def_sector_buf;
		dev->flags        |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
	}
#ifdef DATABASE_NAND
	else if (HLD_DEV_TYPE_NAND == dev->type)
	{
#if 1
		fls_hdl = open(data_base_path, O_RDWR | O_SYNC);
		if (0 > fls_hdl)
		{
			STO_PRINTF("open nand handle failed!\n");
			return RET_FAILURE;
		}
		nand_mutex_id = osal_mutex_create();
		if (OSAL_INVALID_ID == nand_mutex_id)
		{
			close(fls_hdl);
			return RET_FAILURE;
		}

		dev->priv = (void *)fls_hdl;
		offset = lseek(fls_hdl, 0, SEEK_END);
		if (-1 != offset)
		{
			dev->totol_size = offset;//ftell(nand_hdl);
		}
		else
		{
			STO_PRINTF("Bad Flash Image File!\n");
			return RET_FAILURE;
		}

		lseek(fls_hdl, 0, SEEK_SET);
		STO_PRINTF("%s: totol_size: %08x\n", __FUNCTION__, dev->totol_size);
		dev->flag = (STO_FLAG_AUTO_ERASE | STO_FLAG_SAVE_REST);
		dev->priv = (void *)fls_hdl;//NULL;

		dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
#else
		n_hdl = fopen(data_base_path, "r+");
		if (NULL == n_hdl)
		{
			STO_PRINTF("open nand handle failed!\n");
			return RET_FAILURE;
		}
		fseek(n_hdl, 0, SEEK_END);
		dev->totol_size = ftell(n_hdl);
		dev->flag = (STO_FLAG_AUTO_ERASE | STO_FLAG_SAVE_REST);
		fclose(n_hdl);

		dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
#endif
	}
	else
	{
		STO_PRINTF("Invalid Device Type!\n");
		return RET_FAILURE;
	}
#endif

	//STO_PRINTF("%s: totol_siz %08x\n", __FUNCTION__, dev->totol_size);
#if 0
	UINT32 cnt;
	erase_info_t fls_erase;
	cnt=0;
	while(cnt<0x800000)
	{
		int i;
		STO_PRINTF("0x%08x: ",cnt);
		//fread(buf_tmp, 16, 1, fls_hdl);
		fls_erase.start=cnt;
		fls_erase.length=65536;
		ioctl(fls_hdl, MEMERASE, &fls_erase);
		cnt+=65536;
		/*for(i=0; i<16; i++)
		{
			STO_PRINTF("%02x ",buf_tmp[i]);
		}*/
		STO_PRINTF("\n");

		//if(cnt>=0x200000)
		//	break;
	}
	while(1);

#endif

	m_sto_dev = dev;
	return SUCCESS;
}

/*
 * 	Name		:   sto_close()
 *	Description	:   Close a storage device
 *	Parameter	:	struct sto_device *dev		: Device to be closed
 *	Return		:	INT32 						: Return value
 *
 */
INT32 sto_close(struct sto_device *dev)
{
	if (NULL == dev)
    {
        STO_PRINTF("sto hld device is NULL already!\n");
        return SUCCESS;
	}

	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		//STO_PRINTF("sto_close: warning - device %s closed already!\n", dev->name);
		return SUCCESS;
	}

	//free(dev->sector_buffer);
#ifdef DATABASE_NAND    
	if (HLD_DEV_TYPE_STO == dev->type)
#endif
	{
		free(ali_def_sector_buf);
		ali_def_sector_buf = NULL;
		dev->sector_buffer = NULL;
	}
	dev->flag = 0;

	close((int)dev->priv);
	dev->priv = NULL;

	osal_mutex_delete(sto_mutex_id);
	sto_mutex_id = OSAL_INVALID_ID;

	/* Update flags */
	dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

	return SUCCESS;
}

/*
 * 	Name		:   sto_lseek()
 *	Description	:   Long seek current operation point
 *	Parameter	:	struct sto_device *dev		: Device
 *					INT32 offset				: Offset of seek
 *					int start					: Start base position
 *	Return		:	INT32 						: Postion
 *
 */
INT32 sto_lseek(struct sto_device *dev, INT32 offset, int origin)
{
	INT32 new_addr;
	int id;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if ((id = osal_task_get_current_id()) == OSAL_INVALID_ID)
	{
		id = 0;
	}
	if (id >= STO_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		return ERR_DEV_ERROR;
	}
	switch (origin)
	{
		case STO_LSEEK_SET:
			/* Great than totol size, seek to end */
			if (offset >= (INT32)dev->totol_size)
			{
			    dev->curr_addr[id] = dev->totol_size - 1;
			}
			/* Common seek */
			else if (offset >= 0)
			{
			    dev->curr_addr[id] = offset;
			}
			break;

		case STO_LSEEK_CUR:
			new_addr = dev->curr_addr[id] + offset;
			/* Less than base address, seek to begin */
			if (new_addr <= 0)
			{
			    dev->curr_addr[id] = 0;
			}
			/* Great than totol size, seek to end */
			else if (new_addr >= (INT32)dev->totol_size)
			{
			    dev->curr_addr[id] = dev->totol_size - 1;
			}
			/* Common seek */
			else
			{
			    dev->curr_addr[id] = new_addr;
			}
			break;

		case STO_LSEEK_END:
			new_addr = dev->totol_size + offset - 1;
			/* Less than base address, seek to begin */
			if (new_addr <= 0)
			{
			    dev->curr_addr[id] = 0;
			}
			/* Common seek */
			else if (offset <= 0)
			{
			    dev->curr_addr[id] = new_addr;
			}
			break;
	}
	return dev->curr_addr[id];
}

/*
 * 	Name		:   sto_write()
 *	Description	:   Write data into storage
 *	Parameter	:	struct sto_device *dev		: Device
 *					UINT8 *data					: Data to be write
 *					UINT32 len					: Data length
 *	Return		:	INT32 						: Write data length
 *
 */
INT32 sto_write(struct sto_device *dev, UINT8 *data, INT32 len)
{
	INT32 ret;
	int id;
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if ((id = osal_task_get_current_id()) == OSAL_INVALID_ID)
		id = 0;
		
	if(id>=STO_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		return ERR_DEV_ERROR;
	}

	//lseek((int)dev->priv, dev->curr_addr[id], SEEK_SET);
	//ret = write((int)dev->priv, data, len);
#ifdef DATABASE_NAND
	if (HLD_DEV_TYPE_STO == dev->type)
#endif
	{
		STO_PRINTF("%s dev->sector_buffer = 0x%08x \n",__FUNCTION__,dev->sector_buffer);
		ret = sto_flash_change(dev, dev->flag | (dev->curr_addr[id] & 
					FLASH_ADDR_MASK), data, len, dev->sector_buffer);
	}
#ifdef DATABASE_NAND
	else if (HLD_DEV_TYPE_NAND == dev->type)
	{
#if 1		
		sto_mutex_enter();
		lseek((int)dev->priv, dev->curr_addr[id], SEEK_SET);
		ret = write((int)dev->priv, data, len);
		fsync((int)dev->priv);
		sto_mutex_exit();
#else
		FILE *n_hdl = fopen(data_base_path, "r+");
		if (NULL == n_hdl)
		{
			STO_PRINTF("Open NandFlash File Failed!\n");
			return RET_FAILURE;
		}
		fseek(n_hdl, dev->curr_addr[id], SEEK_SET);
		ret = fwrite(data, len, 1, n_hdl);
		fclose(n_hdl);
#endif
	}
	else
	{
		STO_PRINTF("Invalid Device Type!\n");
		return RET_FAILURE;
	}
#endif

	if (ret > 0)
	{
		dev->curr_addr[id] += ret;   // cari chen , just for test
		STO_PRINTF("Leave sto_write(), len: %x\n", len);
	}

	return ret;
}

/*
 * 	Name		:   sto_read()
 *	Description	:   Read data from storage
 *	Parameter	:	struct sto_device *dev		: Device
 *					UINT8 *data					: Data read out
 *					UINT32 len					: Data length
 *	Return		:	INT32 						: Read data length
 *
 */
INT32 sto_read(struct sto_device *dev, UINT8 *data, INT32 len)
{
	int id;
	int rcode;
	INT32 len2 = len;
	UINT32 base_addr = dev->base_addr;
	/* If device not running, exit */
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if ((id = osal_task_get_current_id()) == OSAL_INVALID_ID)
		id = 0;
		
	if(id>=STO_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		return ERR_DEV_ERROR;
	}
	if (dev->curr_addr[id] + len > dev->totol_size)
	{
		len = dev->totol_size - dev->curr_addr[id];
		len2 =len -1;
	}
	STO_PRINTF("%s dev->sector_buffer = 0x%08x \n",__FUNCTION__,dev->sector_buffer);	
	//MUTEX_ENTER();
	lseek((int)dev->priv, dev->curr_addr[id], SEEK_SET);
	
	rcode=read((int)dev->priv, data, len);

	//STO_PRINTF("CARI CHEN , rcode = %d \n",rcode);
	
	//rcode = tp->read(data,(UINT8 *)(base_addr+dev->curr_addr[id]),len);
	//MUTEX_LEAVE();
	
	//STO_PRINTF("Before read, dev->curr_addr[%d] = 0x%08x \n",id,dev->curr_addr[id]);
	
	dev->curr_addr[id] += len2;   // // cari chen , just for test
		
	return rcode;
}

/*
 * 	Name		:   sto_io_control()
 *	Description	:   Do IO control
 *	Parameter	:	struct sto_device *dev		: Device
 *					INT32 cmd					: IO command
 *					UINT32 param				: Param
 *	Return		:	INT32 						: Result
 *
 */
INT32 sto_io_control(struct sto_device *dev, INT32 cmd, UINT32 param)
{
	UINT32 start_offset;
	INT32 len_in_k,rcode;

	erase_info_t fls_erase;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

#ifdef DATABASE_NAND    
	if (HLD_DEV_TYPE_NAND == dev->type)
	{
		switch (cmd)
		{
			case STO_DRIVER_SECTOR_ERASE:
				start_offset = (param >> 10);
				len_in_k = (param & 0x3ff);// << 10;
				return sto_nand_erase(dev, start_offset, len_in_k);
			case STO_DRIVER_SECTOR_ERASE_EXT:
				start_offset = ((UINT32 *)param)[0];
				len_in_k = (((UINT32 *)param)[1]);// << 10;
				return sto_nand_erase(dev, start_offset, len_in_k);
			default:
				return SUCCESS;
		}
	}
#endif

	/*common IO_Control of sto device*/
	switch (cmd)
	{
		/*case STO_DRIVER_DEVICE_ERASE:
			//MUTEX_ENTER();
			rcode=tp->erase_chip();
			//MUTEX_LEAVE();
			if(remote_flash_read_tmo == 0)
			return SUCCESS;
			else{
			if(SUCCESS!=rcode)
				return ERR_FLASH_NEED_ERASE;
			else
				return SUCCESS;
			};
		*/
		case STO_DRIVER_SECTOR_ERASE: /* Auto erase sectors */
			/* Uper 22 bits of MSB is start offset */
			start_offset = (param >> 10);
			/* Lower 10 bits of LSB is length in K bytes*/
			len_in_k = (param & 0x3ff);
			return sto_flash_auto_sec_erase(dev, start_offset, 
			  len_in_k);
		case STO_DRIVER_SECTOR_ERASE_EXT: /* Auto erase sectors */
			start_offset = ((UINT32 *)param)[0];
			len_in_k = ((UINT32 *)param)[1];
			return sto_flash_auto_sec_erase(dev, start_offset, 
			  len_in_k);
		case STO_DRIVER_SET_FLAG:
			dev->flag = param;
			return SUCCESS;

		case STO_DRIVER_SECTOR_BUFFER:
			dev->sector_buffer = (unsigned char *)param;
			STO_PRINTF("%s dev->sector_buffer = 0x%08x \n",__FUNCTION__,dev->sector_buffer);
			return SUCCESS;
		default:
			break;
	}
	return SUCCESS;
}

/*
 * 	Name		:   sto_put_data()
 *	Description	:   Write data into storage
 *	Parameter	:	struct sto_device *dev		: Device
 *					UINT8 *data					: Data to be write
 *					UINT32 len					: Data length
 *	Return		:	INT32 						: Write data length
 *
 */
INT32 sto_put_data(struct sto_device *dev, UINT32 offset, UINT8 *data, INT32 len)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

#ifdef DATABASE_NAND
	if (HLD_DEV_TYPE_STO == dev->type)
#endif
	{
		//STO_PRINTF("%s: ofst %08x, l %08x, data %08x\n", __FUNCTION__, offset, len, data);
		return sto_flash_change(dev, dev->flag | (offset & FLASH_ADDR_MASK), 
				data, len, dev->sector_buffer);
	}

#ifdef DATABASE_NAND
	else if (HLD_DEV_TYPE_NAND == dev->type)
	{
		sto_mutex_enter();
		lseek((int)dev->priv, offset, SEEK_SET);
		//return fwrite(data, len, 1, (FILE *)dev->priv);
		ret = write((int)dev->priv, data, len);
		fsync((int)dev->priv);
		sto_mutex_exit();
		return ret;
	}
	else
	{
		STO_PRINTF("Invalid Device Type!\n");
		return RET_FAILURE;
	}
#endif    
}

/*
 * 	Name		:   sto_get_data()
 *	Description	:   Read data from storage
 *	Parameter	:	struct sto_device *dev		: Device
 *					UINT32 len					: Data length
 *					UINT8 *data					: Data to be read
 *	Return		:	INT32 						: Read data length
 *
 */
INT32 sto_get_data(struct sto_device *dev, UINT8 *data, UINT32 offset, INT32 len)
{	
	int ret_len=0;
	/* If device not running, exit */
		
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if(len==0)
		return 0;

	if ((INT32)offset + len > (INT32)dev->totol_size)
		len = dev->totol_size - offset;
	/*while(len>0x10000)
	{
		lseek((int)dev->priv, offset, SEEK_SET);
		ret_len+=read((int)dev->priv, data, 0x10000);

		len-=0x10000;
		offset+=0x10000;
		data+=0x10000;
	}*/
	lseek((int)dev->priv, offset, SEEK_SET);
	ret_len+=read((int)dev->priv, data, len);
	return 	ret_len;
	
}

INT32 sto_local_sflash_attach(struct sto_flash_info *param)
{
	struct sto_device *dev = (struct sto_device *)dev_get_by_name(sto_sflash_local_name);
    if (dev)
    {
        STO_PRINTF("Note: sto device named %s has already exist!\n", sto_sflash_local_name);
	    return SUCCESS;
    }
	
	dev = dev_alloc(sto_sflash_local_name, HLD_DEV_TYPE_STO, sizeof(struct sto_device));
	if (dev == NULL)
	{
		STO_PRINTF("Error: Alloc storage device error!\n");
		return ERR_NO_MEM;
	}

	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS) {
		STO_PRINTF("Error: Register Flash storage device error!\n");
		dev_free(dev);
		return ERR_NO_DEV;
	}

	return SUCCESS;
}

void flash_info_sl_init()
{
}

INT32 sto_local_sflash_dettach(struct sto_device *dev)
{
	struct sto_device *cur_dev = (struct sto_device *)dev_get_by_name(sto_sflash_local_name);
	if (NULL == cur_dev)
	{    
		STO_PRINTF("Note: sto device named %s not exist already!\n", sto_sflash_local_name);
		return SUCCESS;
	}
	if (NULL == dev)
	{   
		STO_PRINTF("sto hld device is NULL already!\n");
		return SUCCESS;
	}

	if (dev->flags & (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING))
	{
		STO_PRINTF("Error, STO device still running.\n");
		return !(SUCCESS);
	}
	dev_free(dev);
	return SUCCESS;
}

INT32 sto_support_giga_ext(struct sto_device *dev)
{
	UINT32 cfg = (UINT32)(FLASHCMD_EXT_OFFSETBIT_FOR_GIGA);
	return ioctl((int)(dev->priv), ALI_FLASH_CONFIG_CMD_EXT_GROUP, cfg);
}

INT32 sto_cancel_giga_ext(struct sto_device *dev)
{
	UINT32 cfg = (UINT32)(FLASHCMD_EXT_OFFSETBIT_FOR_GIGA | (1<<8));
	return ioctl((int)(dev->priv), ALI_FLASH_CONFIG_CMD_EXT_GROUP, cfg);
}

INT32 sto_erase_giga_as(struct sto_device *dev)
{
	return ioctl((int)(dev->priv), ALI_GIGA_FLASH_ESR);
}

INT32 sto_read_giga_customid(struct sto_device *dev, UINT8 *buf)
{
	return ioctl((int)(dev->priv), ALI_GIGA_FLASH_READ_CSTMID, buf);
}

INT32 sto_read_giga_as(struct sto_device *dev, UINT32 *gt)
{
	return ioctl((int)(dev->priv), ALI_GIGA_FLASH_RSR, gt);
}

INT32 sto_write_giga_as(struct sto_device *dev, UINT32 *gt)
{
	return ioctl((int)(dev->priv), ALI_GIGA_FLASH_PSR, gt);
}

INT32 sto_lock_giga_as(struct sto_device *dev)
{
	return ioctl((int)(dev->priv), ALI_GIGA_FLASH_LOCKSR);
}


/***************** nand attach *****************/
static char sto_nand_local_name[HLD_MAX_NAME_SIZE] = "STO_NAND_0";

INT32 sto_nand_attach(void)
{
	struct sto_device *dev;

	dev = dev_alloc(sto_nand_local_name, HLD_DEV_TYPE_NAND, sizeof(struct sto_device));
	//dev = dev_alloc(sto_nand_local_name, HLD_DEV_TYPE_STO, sizeof(struct sto_device));
	if (dev == NULL)
	{    
		STO_PRINTF("Error: Alloc NandFlash device error!\n");
		return ERR_NO_MEM;
	}    

	/* Add this device to queue*/
	if (dev_register(dev) != SUCCESS)
	{    
		STO_PRINTF("Error: Register Nand Device error!\n");
		dev_free(dev);
		return ERR_NO_DEV;
	}    

	return SUCCESS;
}

INT32 sto_nand_dettach(struct sto_device *dev)
{
	if (dev->flags & (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING))
	{    
		STO_PRINTF("Error, STO device still running.\n");
		return !(SUCCESS);
	}

	dev_free(dev);
	return SUCCESS;
}

static int sto_nand_erase(struct sto_device *dev, UINT32 offset, INT32 len)
{
	INT32 ret = -1;
	char *data = NULL;

	data = (char *)malloc(len * 1024);
	if (NULL == data)
	{
		STO_PRINTF("malloc error!\n");
		return !(SUCCESS);
	}

	memset(data, 0xff, len * 1024);	
	sto_mutex_enter();
	lseek((int)dev->priv, offset, SEEK_SET);

	ret = write((int)dev->priv, data, len * 1024);
	if (ret != len * 1024)
	{
		//break;
		sto_mutex_exit();
		free(data);
		data = NULL;
		return !(SUCCESS);
	}

	fsync((int)dev->priv);
	sto_mutex_exit();

	free(data);
	data = NULL;
	if (ret >= 0)
	{
		return SUCCESS;
	}
	else
	{
		return !(SUCCESS);
	}
}


static char *data_base_dir = "/usr/app/";
static char *data_base_dir_bak = "/usr/app_bak/";
static char *data_base_name = "user_data.bin";
static char *data_base_name_bak = "user_data_bak.bin";
static char *data_base_nand_partition = "/dev/mtdblock6";

static char *data_base_path = "/usr/app/user_data.bin";
//static char *data_base_path = "/demo_test/user_data.bin";

INT32 sto_nand_backup_db(void)
{
	INT32 result = SUCCESS;
	char sys_cmd[128];
	sto_close(m_sto_dev);

	return SUCCESS;
	/*    
     sto_close(m_sto_dev);
     sprintf(sys_cmd, "rm -rf %s%s\n", data_base_dir, data_base_name_bak);
     STO_PRINTF(sys_cmd);
     system(sys_cmd);
     sprintf(sys_cmd, "cp %s%s %s%s\n", data_base_dir, data_base_name, data_base_dir, data_base_name_bak);
     STO_PRINTF(sys_cmd);
     system(sys_cmd);

     return result;    
    */    
}

INT32 sto_nand_refresh_db(BOOL bUpgradeSuccess)
{
	INT32 result = SUCCESS;
	char sys_cmd[128];

	sto_open(m_sto_dev);
	return SUCCESS;

	/*
    if (bUpgradeSuccess)
    {
        sprintf(sys_cmd, "mkdir -p %s\n", data_base_dir_bak);
	    STO_PRINTF(sys_cmd);
	    system(sys_cmd);
	    sprintf(sys_cmd, "mount -t yaffs2 -o inband-tags %s %s\n", data_base_nand_partition, data_base_dir_bak);
	    STO_PRINTF(sys_cmd);
	    system(sys_cmd);
	    sprintf(sys_cmd, "rm -rf %s%s\n", data_base_dir, data_base_name);
	    STO_PRINTF(sys_cmd);
	    system(sys_cmd);
	    sprintf(sys_cmd, "cp %s%s %s%s\n", data_base_dir_bak, data_base_name, data_base_dir, data_base_name);
	    STO_PRINTF(sys_cmd);
	    system(sys_cmd);
	    sprintf(sys_cmd, "rm -rf %s%s\n", data_base_dir, data_base_dir_bak);
	    STO_PRINTF(sys_cmd);
	    system(sys_cmd);
	    sprintf(sys_cmd, "umount %s\n", data_base_dir_bak);
	    STO_PRINTF(sys_cmd);
	    system(sys_cmd);
	    sprintf(sys_cmd, "rm -rf %s\n", data_base_dir_bak);
	    STO_PRINTF(sys_cmd);
	    system(sys_cmd);
	 }
	 else
	 {
	    sprintf(sys_cmd, "rm -rf %s%s\n", data_base_dir, data_base_name_bak);
	    STO_PRINTF(sys_cmd);
	    system(sys_cmd);
	 }
	 #endif
	 sto_open(m_sto_dev);
	 return result;    
	 */        
}

/*
 * 	Name		:   sto_otp_get_data()
 *	Description	:   Read data from storage
 *	Parameter	:	struct sto_device *dev		: Device
 *					UINT32 len					: Data length
 *                  UINT8 sector_index          : sector 0,1,2
 *					UINT8 *data					: Data to be read
 *	Return		:	INT32 						: Read data length
 *
 */
INT32 sto_otp_get_data(struct sto_device *dev, UINT8 *data, UINT32 offset, INT32 len,UINT8 sector_index)
{
#if 0
	INT32 ret_len=0;
	STO_PRINTF("Enter sto_get_data(), len: %x\n", len);
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if(len==0)
		return 0;
	if ((INT32)offset + len > 256)
		len = dev->totol_size - offset;

	sto_mutex_enter();
	//libc_printf(" sto_get_data(),offset: %x\n", offset);
	lseek((int)dev->priv, (offset+(sector_index+1)*0x1000), SEEK_SET);
	ioctl((int)dev->priv, ALI_FLASH_SECTOR_OTP_RW, 0);
	ret_len += read((int)dev->priv, data, len);
	sto_mutex_exit();

	STO_PRINTF("Leave sto_get_data(), ret_len: %x\n", ret_len);
	return ret_len;
#endif
}

/*
 * 	Name		:   sto_otp_get_status()
 *	Description	:   Read data from storage
 *	Parameter	:	struct sto_device *dev		: Device
 *	Return		:	UINT8						: otp status
 *        otp_status = 1,flash otp sector1 is locked
 *        otp_status = 2,flash otp sector2 is locked
 *        otp_status = 4,flash otp sector3 is locked
 *        otp_status = 7,all flash otp sector(1,2,3) is locked
 */
UINT8 sto_otp_get_status( struct sto_device *dev )
{
#if 0
	UINT8 otp_status = 0;
	if (HLD_DEV_TYPE_STO == dev->type)
	{
		ioctl((int)dev->priv,ALI_FALSH_SECTOR_OTP_GET_STATUS, &otp_status);
		STO_PRINTF("otp_status [0x%d]\n",otp_status);
		return otp_status;
	}
	return 0;
#endif
}

/*
 * 	Name		:   sto_otp_set_status()
 *	Description	:   Read data from storage
 *	Parameter	:	struct sto_device *dev		: Device
 *                  sector_index                :otp sector sector_index
 *	Return		:	void
 *        sector_index = 0,locked flash otp sector1 
 *        sector_index = 1,locked flash otp sector2 
 *        sector_index = 2,locked flash otp sector3 
 *notice:if set otp,flash otp sector cann't write and erase.      
 */
void sto_otp_set_status( struct sto_device *dev,UINT8 sector_index)
{
#if 0
	if (HLD_DEV_TYPE_STO == dev->type)
	{
		ioctl((int)dev->priv, ALI_FALSH_SECTOR_OTP_SET_STATUS, &sector_index);
		STO_PRINTF("sector_index [0x%d]\n",sector_index);
	}
#endif
}

/*
 * 	Name		:   sto_otp_put_data()
 *	Description	:   Write data into storage
 *	Parameter	:	struct sto_device *dev		: Device
 *					UINT8 *data					: Data to be write
 *					UINT32 len					: Data length
 *                  UINT8 sector_index          :sector0,sector 1,sector2
 *	Return		:	INT32 						: Write data length
 *
 */
INT32 sto_otp_put_data(struct sto_device *dev, UINT32 offset, UINT8 *data, INT32 len,UINT8 sector_index)
{
#if 0
	INT32 ret = 0;
	UINT8 *save_data_left = NULL;
	UINT8 *save_data_right = NULL;
	STO_PRINTF("Enter sto_put_data(), len: %x\n", len);
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}
	if(sector_index>2)
	{
		sector_index=0;
	}

#ifdef DATABASE_NAND
	if (HLD_DEV_TYPE_STO == dev->type)
#endif     
	{

		if(offset+len>256)
		{
			len =256-offset;
		}
		if((offset+len<256)&&(len>0))
		{

			STO_PRINTF("\n before ui save_data_left=[0x%x]\n",(UINT32)save_data_left);
			STO_PRINTF("\n before ui save_data_right=[0x%x]\n",(UINT32)save_data_right);
			if(offset!=0)
			{
				save_data_left=(UINT8*)malloc(offset);
			}

			save_data_right = (UINT8*)malloc(256-offset-len);

			if(offset!=0)
			{
				ioctl((int)dev->priv,ALI_FLASH_SECTOR_OTP_RW, 0);
				sto_otp_get_data(dev,save_data_left,0,offset,0);
			}
			ioctl((int)dev->priv,ALI_FLASH_SECTOR_OTP_RW, 0);
			sto_otp_get_data(dev,save_data_right,(offset+len),(256-offset-len),0);
			STO_PRINTF("a ui save_data_left=[0x%x]\n",(UINT32)save_data_left);
			STO_PRINTF("a ui save_data_right=[0x%x]\n",(UINT32)save_data_right);

			//ioctl((int)dev->priv,ALI_FLASH_SECTOR_OTP_READ, 0);
			ioctl((int)dev->priv,ALI_FLASH_SECTOR_OTP_ERASE, sector_index);
			if(offset!=0)
			{
				ioctl((int)dev->priv,ALI_FLASH_SECTOR_OTP_RW, 0);
				lseek((int)dev->priv, ((sector_index+1)*0x1000), SEEK_SET);
				write((int)dev->priv, save_data_left, offset);
			}

			ioctl((int)dev->priv,ALI_FLASH_SECTOR_OTP_RW, 0);
			lseek((int)dev->priv, (offset+(sector_index+1)*0x1000), SEEK_SET);
			write((int)dev->priv, data, len);

			ioctl((int)dev->priv,ALI_FLASH_SECTOR_OTP_RW, 0);
			lseek((int)dev->priv, (offset+len+(sector_index+1)*0x1000), SEEK_SET);
			write((int)dev->priv, save_data_right,256-offset-len);
			if(offset!=0)
			{
				free(save_data_left);
			}
			free(save_data_right);
		}
		return SUCCESS;
	}
#endif
}
