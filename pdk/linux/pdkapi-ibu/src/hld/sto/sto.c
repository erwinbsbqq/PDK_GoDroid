/*****************************************************************************
* Ali Corp. All Rights Reserved. 2002 - 2003 Copyright (C)
*
* File: sto.c
*
* Description: This file contains all functions definition
* of storage device driver.
* History:
* Date Athor Version Reason
* ============ ============= ========= =================
* 1. May.29.2003 Justin Wu Ver 0.1 Create file.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
//corei#include <api/libc/alloc.h>
//#include <api/libc/string.h>
//#include <api/libc/printf.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
//corei#include <err/errno.h>

#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>

#include <compiler_common.h>
#include <mtd/mtd-user.h>
#include <ali_mtd_common.h>

#include <hld/hld_dev.h>

#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>

#define STO_PRINTF(...) do{}while(0)
#define DATABASE_NAND

//#define STO_PRINTF printf
//#define DEBUGLINE() do{printf("%s: %d\n", __FUNCTION__, __LINE__);}while(0)
//#define STO_PRINTF printf
struct sto_device* m_sto_dev = NULL;

static OSAL_ID nor_mutex_id = OSAL_INVALID_ID;
static OSAL_ID nand_mutex_id = OSAL_INVALID_ID;
static char sto_sflash_local_name[HLD_MAX_NAME_SIZE] = "STO_SFLASH_0";
static char sto_nand_local_name[HLD_MAX_NAME_SIZE] = "STO_NAND_0";

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

#define FLASH_ADDR_MASK 0x3FFFFFF
static UINT8 ali_fls_verify[4096];
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
			ret = 1; // that means flash content all "1", we could write directly
			if (dst & src)
			{
				ret = 2;// that means flash content not all "1", Need erase;
				//STO_PRINTF("fls need erase, ret: %d\n",ret);
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
			ret = 2;//Need erase;
			//STO_PRINTF("fls need erase, ret: %d\n",ret);
			return ret;
		}
	};

	//STO_PRINTF("fls do not need erase, ret: %d\n", ret);
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
	unsigned long sector_start, sector_end, sector_size;
	int ret;
	erase_info_t fls_erase;
	//struct flash_private *tp = (struct flash_private *)dev->priv;
	//STO_PRINTF("%s\n", __FUNCTION__);

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
	STO_PRINTF("%s: target_start %08x\n", __FUNCTION__, target_start);
	sector_no = ioctl((int)dev->priv,ALI_FLASH_SECTOR_ALIGN, target_start);
	STO_PRINTF("%s: sector_no %08x\n", __FUNCTION__, sector_no);
	sector_start = ioctl((int)dev->priv,ALI_FLASH_SECTOR_START, sector_no);
	STO_PRINTF("%s: sector_start %08x\n", __FUNCTION__, sector_start);
	while (target_start < target_end)
	{
		/* get information about current sector */
		sector_size = ioctl((int)dev->priv,ALI_FLASH_SECTOR_SIZE, sector_no);
		sector_end = sector_start + sector_size;
		STO_PRINTF("%s: sector_size %08x, \n", __FUNCTION__, sector_size);
		/* get information about current operation */
		oper_start = target_start > sector_start ? target_start : sector_start;
		oper_end = target_end < sector_end ? target_end : sector_end;
		oper_len = oper_end - oper_start;
		oper_p = data;

		pre_fix_l = post_fix_l = 0;
		/* check flash operation */
		STO_PRINTF("%s: oper_start %08x, oper_len %08x\n", __FUNCTION__, oper_start, oper_len);
		ret=sto_flash_verify(dev, oper_start, data, oper_len);
		switch (ret)
		{
		case 2: /* need erase */

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
						oper_p = data;
					else
						oper_len = p - oper_p;
					oper_start = sector_start;
				}
				else
				{
					ret=1; // need to patch linux compile optimization.
				}
				/* erase sector */
				//MUTEX_ENTER();
				//rcode = tp->erase_sector(sector_start);
				STO_PRINTF("sto change data: erase sector: 0x%08x, len:%08x\n", sector_start, sector_size);
				fls_erase.start=sector_start;
				fls_erase.length=sector_size;
				rcode=ioctl((int)dev->priv, MEMERASE, &fls_erase);
				//MUTEX_LEAVE();
				if(SUCCESS!=rcode)
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
		case 1:/* need write */
			//MUTEX_ENTER();
			//by eric, patch linux compile optimization.
			if((1==ret)&&(oper_start&0x03))
			{
				p=buffer;
				oper_p=buffer;
				sto_get_data(dev, p, (oper_start&(~0x3)), (oper_start&(0x3)));
				p+=(oper_start&(0x3));
				memcpy(p, data, oper_len);
				oper_len+=(oper_start&(0x3));
				pre_fix_l=(oper_start&(0x3));
			}
			STO_PRINTF("%s: seek 0x%08x\n", __FUNCTION__, (oper_start&(~0x3)));
			sto_mutex_enter(dev);
			lseek((int)dev->priv, (oper_start&(~0x3)), SEEK_SET);
			STO_PRINTF("%s: write oper_start %08x, oper_len %08x, oper_p %08x\n", __FUNCTION__, (oper_start&(~0x3)), oper_len, oper_p);
			rcode = write((int)dev->priv, oper_p, oper_len);
			sto_mutex_exit(dev);
			//MUTEX_LEAVE();
			if (rcode < 0)
			{
				//printf("ret: %d ", ret);
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
		if(oper_len>=pre_fix_l)
			oper_len -= pre_fix_l;
		if(oper_len>=post_fix_l)
			oper_len -= post_fix_l;
		target_start += oper_len;
		data += oper_len;
		sector_start = sector_end;
		sector_no++;
	}

	STO_PRINTF("%s: return %08x\n", __FUNCTION__, len);
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
	unsigned long oper_start_tmp,oper_len_tmp;
	int ret;
	int rcode;
	erase_info_t fls_erase;
	STO_PRINTF("%s start:0x%08x, len:%08x\n",__FUNCTION__, target_start, len_in_k);
	//struct flash_private *tp = (struct flash_private *)dev->priv;

	/* check input data validation */
	if (target_start >= dev->totol_size)
	{
		STO_PRINTF("%s, offset error.0x%08x\n",__FUNCTION__, target_start);
		return ERR_FLASH_OFFSET_ERROR;
	}
	/* limit input data to proper value */
	target_end = target_start + (len_in_k << 10);
	if (target_end > dev->totol_size)
	{
		target_end = dev->totol_size;
	}
	/* begin to erase sector by sector */
	sector_no = ioctl((int)dev->priv,ALI_FLASH_SECTOR_ALIGN, target_start);//sto_flash_sector_align(dev,target_start);
	sector_start = ioctl((int)dev->priv,ALI_FLASH_SECTOR_START, sector_no);//sto_flash_sector_start(dev,sector_no);
	STO_PRINTF("target_start:0x%08x, target_end:0x%08x\n",target_start, target_end);
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
		STO_PRINTF("%d, oper_start:0x%08x, oper_end:0x%08x, oper_len:0x%08x\n",__LINE__, oper_start, oper_end, oper_len);
		if (oper_len != sector_size)
		{
			p = oper_p = dev->sector_buffer;
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
		STO_PRINTF("sto erase sec addr 0x%08x: l 0x%08x\n", sector_start, sector_size);
		rcode=ioctl((int)dev->priv, MEMERASE, &fls_erase);
		STO_PRINTF("%d, oper_start:0x%08x, oper_end:0x%08x, oper_len:0x%08x\n",__LINE__, oper_start, oper_end, oper_len);

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
			sto_mutex_enter(dev);
			lseek((int)dev->priv, oper_start_tmp, SEEK_SET);
			ret = write((int)dev->priv, oper_p, oper_len_tmp);
			sto_mutex_exit(dev);
			//MUTEX_LEAVE();
			if (ret < 0)
			{
				STO_PRINTF("sto write back addr 0x%08x, len:0x%08x, failed.\n", oper_start_tmp, oper_len_tmp);
				return ERR_FLASH_WRITE_FAIL;
			}
		}
		STO_PRINTF("%d, oper_start:0x%08x, oper_end:0x%08x, oper_len:0x%08x\n",__LINE__, oper_start, oper_end, oper_len);
		/* advance to next data block (sector) */
		target_start += oper_len;
		sector_start = sector_end;
		sector_no++;
	}

	return SUCCESS;
}

static int sto_nand_erase(struct sto_device *dev, UINT32 offset, INT32 len)
{
	INT32 ret = -1;
	char *data = NULL;

	data = (char *)malloc(len * 1024);
	if (NULL == data)
	{
		printf("malloc error!\n");
		return !SUCCESS;
	}
	memset(data, 0xff, len * 1024);
	sto_mutex_enter(dev);
	lseek((int)dev->priv, offset, SEEK_SET);

	ret = write((int)dev->priv, data, len * 1024);
	if (ret != len * 1024)
	{
		//break;
		sto_mutex_exit(dev);
		free(data);
		data = NULL;
		return !SUCCESS;
	}

	fsync((int)dev->priv);
	sto_mutex_exit(dev);

	free(data);
	data = NULL;
	if (ret >= 0)
		return SUCCESS;
	else
		return !SUCCESS;
}

void sto_mutex_enter(struct sto_device *dev)
{
	if (dev == NULL)
	{
		printf("%s Bad Device\n", __FUNCTION__);
		return;
	}

	if (HLD_DEV_TYPE_NOR == dev->type)
	{
		if(OSAL_INVALID_ID == nor_mutex_id)
		{
			nor_mutex_id = osal_mutex_create();
			ASSERT(nor_mutex_id!=OSAL_INVALID_ID);
		}
		osal_mutex_lock(nor_mutex_id, OSAL_WAIT_FOREVER_TIME);
	}
	else if (HLD_DEV_TYPE_STO == dev->type)
	{
		if(OSAL_INVALID_ID == nand_mutex_id)
		{
			nand_mutex_id = osal_mutex_create();
			ASSERT(nand_mutex_id!=OSAL_INVALID_ID);
		}
		osal_mutex_lock(nand_mutex_id, OSAL_WAIT_FOREVER_TIME);
	}
	else
	{
		printf("Unknown Device Type!\n");
		return;
	}
}

void sto_mutex_exit(struct sto_device *dev)
{
	if (NULL == dev)
	{
		printf("%s Bad Device\n", __FUNCTION__);
		return;
	}

	if (HLD_DEV_TYPE_NOR == dev->type)
	{
		osal_mutex_unlock(nor_mutex_id);
	}
	else if (HLD_DEV_TYPE_STO == dev->type)
	{
		osal_mutex_unlock(nand_mutex_id);
	}
	else
	{
		printf("%s UnKnown Device Type!\n", __FUNCTION__);
	}

}

#if 1
static char * data_base_dir ="/usr/app/" ;
static char * data_base_dir_bak ="/usr/app_bak/" ;
static char * data_base_name ="user_data.bin" ;
static char * data_base_name_bak ="user_data_bak.bin" ;

static char * data_base_nand_partition ="/dev/mtdblock6" ;



INT32 sto_nand_backup_db()
{
	INT32 result = SUCCESS;
	char sys_cmd[128];
	sto_close(m_sto_dev);

	return SUCCESS;
	/*
	sto_close(m_sto_dev);
	sprintf(sys_cmd, "rm -rf %s%s\n", data_base_dir, data_base_name_bak);
	printf(sys_cmd);
	system(sys_cmd);
	sprintf(sys_cmd, "cp %s%s %s%s\n", data_base_dir, data_base_name, data_base_dir, data_base_name_bak);
	printf(sys_cmd);
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
	printf(sys_cmd);
	system(sys_cmd);
	sprintf(sys_cmd, "mount -t yaffs2 -o inband-tags %s %s\n", data_base_nand_partition, data_base_dir_bak);
	printf(sys_cmd);
	system(sys_cmd);
	sprintf(sys_cmd, "rm -rf %s%s\n", data_base_dir, data_base_name);
	printf(sys_cmd);
	system(sys_cmd);
	sprintf(sys_cmd, "cp %s%s %s%s\n", data_base_dir_bak, data_base_name, data_base_dir, data_base_name);
	printf(sys_cmd);
	system(sys_cmd);
	sprintf(sys_cmd, "rm -rf %s%s\n", data_base_dir, data_base_dir_bak);
	printf(sys_cmd);
	system(sys_cmd);
	sprintf(sys_cmd, "umount %s\n", data_base_dir_bak);
	printf(sys_cmd);
	system(sys_cmd);
	sprintf(sys_cmd, "rm -rf %s\n", data_base_dir_bak);
	printf(sys_cmd);
	system(sys_cmd);
	}
	else
	{
	sprintf(sys_cmd, "rm -rf %s%s\n", data_base_dir, data_base_name_bak);
	printf(sys_cmd);
	system(sys_cmd);
	}
	#endif
	sto_open(m_sto_dev);

	return result;
	*/
}
#endif
/*
* Name : sto_open()
* Description : Open a storage device
* Parameter : struct sto_device *dev : Device to be openned
* Return : INT32 : Return value
*
*/
static char * data_base_path ="/usr/app/user_data.bin" ;
//static char * data_base_path = "/demo_test/user_data.bin";

INT32 sto_open(struct sto_device *dev)
{
	INT32 result = SUCCESS;
	int fls_hdl = 0;
	FILE* n_hdl = NULL;
	struct mtd_info_user fls_info;
	off_t offset = 0;

	char sflash_devnum[32] = {0};
	char mtd_devnum[64] = {0};

	/* If openned already, exit */
	if (dev->flags & HLD_DEV_STATS_UP)
	{
		//STO_PRINTF("sto_open: warning - device %s openned already!\n", dev->name);
		return SUCCESS;
	}

#ifdef DATABASE_NAND
	if (HLD_DEV_TYPE_NOR == dev->type)
#endif
	{
		//fls_hdl=open("/dev/mtd11", O_RDWR);
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
		else
		{
			STO_PRINTF("open flash handle success\n");
		}

		nor_mutex_id = osal_mutex_create();
		if (OSAL_INVALID_ID == nor_mutex_id)
		{
			close(fls_hdl);
			return RET_FAILURE;
		}

		dev->priv=(void *)fls_hdl;
		ioctl(fls_hdl, MEMGETINFO, &fls_info);
		dev->totol_size = fls_info.size;
		STO_PRINTF("%s: totol_siz %08x\n", __FUNCTION__, dev->totol_size);
		dev->flag=(STO_FLAG_AUTO_ERASE|STO_FLAG_SAVE_REST);
		dev->sector_buffer=malloc(0x10000);
	}
#ifdef DATABASE_NAND
	else if (HLD_DEV_TYPE_STO == dev->type)
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

		dev->priv=(void *)fls_hdl;
		offset = lseek(fls_hdl, 0, SEEK_END);
		if (-1 != offset)
			dev->totol_size = offset;//ftell(nand_hdl);
		else
		{
			STO_PRINTF("Bad Flash Image File!\n");
			return RET_FAILURE;
		}
		lseek(fls_hdl, 0, SEEK_SET);
		STO_PRINTF("%s: totol_size: %08x\n", __FUNCTION__, dev->totol_size);
		dev->flag = (STO_FLAG_AUTO_ERASE | STO_FLAG_SAVE_REST);
		dev->priv = (void *)fls_hdl;//NULL;
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
#endif
	}
	else
	{
		STO_PRINTF("Invalid Device Type!\n");
		return RET_FAILURE;
	}
#endif

	dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
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
		// break;
	}
	while(1);

#endif

	m_sto_dev = dev;
	return SUCCESS;
}

/*
* Name : sto_close()
* Description : Close a storage device
* Parameter : struct sto_device *dev : Device to be closed
* Return : INT32 : Return value
*
*/
INT32 sto_close(struct sto_device *dev)
{
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		//STO_PRINTF("sto_close: warning - device %s closed already!\n", dev->name);
		return SUCCESS;
	}

	dev->flag = 0;
#ifdef DATABASE_NAND
	if (HLD_DEV_TYPE_NOR == dev->type)
#endif
	{
		if (NULL != dev->sector_buffer)
			free(dev->sector_buffer);
		dev->sector_buffer=NULL;

		osal_mutex_delete(nor_mutex_id);
		nor_mutex_id = OSAL_INVALID_ID;
	}
#ifdef DATABASE_NAND//corei
	else
#endif
	{
		osal_mutex_delete(nand_mutex_id);
		nand_mutex_id = OSAL_INVALID_ID;
	}

	if (NULL != dev->priv)
		close((int)dev->priv);
	dev->priv=NULL;

	/* Update flags */
	dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

	return SUCCESS;
}

/*
* Name : sto_lseek()
* Description : Long seek current operation point
* Parameter : struct sto_device *dev : Device
* INT32 offset : Offset of seek
* int start : Start base position
* Return : INT32 : Postion
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
		id = 0;
	if(id>=STO_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		ASSERT(0);
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
		if (new_addr < 0)
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
		if (new_addr < 0)
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
* Name : sto_write()
* Description : Write data into storage
* Parameter : struct sto_device *dev : Device
* UINT8 *data : Data to be write
* UINT32 len : Data length
* Return : INT32 : Write data length
*
*/
INT32 sto_write(struct sto_device *dev, UINT8 *data, INT32 len)
{
	INT32 ret;
	int id;

	STO_PRINTF("Enter sto_write(), len: %x\n", len);

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		STO_PRINTF("Sto device not running\n");
		return ERR_DEV_ERROR;
	}

	if ((id = osal_task_get_current_id()) == OSAL_INVALID_ID)
		id = 0;
	if(id>=STO_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		ASSERT(0);
	}

#ifdef DATABASE_NAND
	if (HLD_DEV_TYPE_NOR == dev->type)
#endif
	{
		//lseek((int)dev->priv, dev->curr_addr[id], SEEK_SET);
		//ret = write((int)dev->priv, data, len);
		ret = sto_flash_change(dev, dev->flag | (dev->curr_addr[id] & \
			FLASH_ADDR_MASK), data, len, dev->sector_buffer);
	}
#ifdef DATABASE_NAND
	else if (HLD_DEV_TYPE_STO == dev->type)
	{
#if 1
		sto_mutex_enter(dev);
		lseek((int)dev->priv, dev->curr_addr[id], SEEK_SET);
		ret = write((int)dev->priv, data, len);
		fsync((int)dev->priv);
		sto_mutex_exit(dev);
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
		dev->curr_addr[id] += ret;
		STO_PRINTF("Leave sto_write(), len: %x\n", len);
		return ret;
	}
	else
		return ERR_FAILURE;
}

/*
* Name : sto_read()
* Description : Read data from storage
* Parameter : struct sto_device *dev : Device
* UINT8 *data : Data read out
* UINT32 len : Data length
* Return : INT32 : Read data length
*
*/
INT32 sto_read(struct sto_device *dev, UINT8 *data, INT32 len)
{
	int id;
	int rcode;
	INT32 len2 = len;
	UINT32 base_addr = dev->base_addr;
	ID tmutex;

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
		ASSERT(0);
	}
	if (dev->curr_addr[id] + len > dev->totol_size)
	{
		len = dev->totol_size - dev->curr_addr[id];
		len2 =len -1;
	}

	//MUTEX_ENTER();
	sto_mutex_enter(dev);
	lseek((int)dev->priv, dev->curr_addr[id], SEEK_SET);
	rcode=read((int)dev->priv, data, len);
	//rcode = tp->read(data,(UINT8 *)(base_addr+dev->curr_addr[id]),len);
	//MUTEX_LEAVE();
	sto_mutex_exit(dev);

	dev->curr_addr[id] += len2;

	STO_PRINTF("Leave sto_read() rc: %x\n", rcode);
	return rcode;
}

/*
* Name : sto_io_control()
* Description : Do IO control
* Parameter : struct sto_device *dev : Device
* INT32 cmd : IO command
* UINT32 param : Param
* Return : INT32 : Result
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
	if (HLD_DEV_TYPE_STO == dev->type)
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
	case STO_DRIVER_SECTOR_ERASE:/* Auto erase sectors */
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
		// Oncer.Yu 20111108: we have this "sector_buffer" been malloced
		// inside sto.c already, so I think we don't need get it from outside...
		// if (NULL != dev->sector_buffer)
		// free(dev->sector_buffer);
		// dev->sector_buffer = (unsigned char *)param;
		return SUCCESS;
	default:
		break;
	}

	return SUCCESS;
}

/*
* Name : sto_put_data()
* Description : Write data into storage
* Parameter : struct sto_device *dev : Device
* UINT8 *data : Data to be write
* UINT32 len : Data length
* Return : INT32 : Write data length
*
*/
INT32 sto_put_data(struct sto_device *dev, UINT32 offset, UINT8 *data, INT32 len)
{
	INT32 ret = 0;

	STO_PRINTF("Enter sto_put_data(), len: %x\n", len);
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

#ifdef DATABASE_NAND
	if (HLD_DEV_TYPE_NOR == dev->type)
#endif
	{
		//STO_PRINTF("%s: ofst %08x, l %08x, data %08x\n", __FUNCTION__, offset, len, data);
		return sto_flash_change(dev, dev->flag | (offset & FLASH_ADDR_MASK), \
			data, len, dev->sector_buffer);
	}
#ifdef DATABASE_NAND
	else if (HLD_DEV_TYPE_STO == dev->type)
	{
		sto_mutex_enter(dev);
		lseek((int)dev->priv, offset, SEEK_SET);
		//return fwrite(data, len, 1, (FILE *)dev->priv);
		ret = write((int)dev->priv, data, len);
		fsync((int)dev->priv);
		sto_mutex_exit(dev);
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
* Name : sto_otp_get_data()
* Description : Read data from storage
* Parameter : struct sto_device *dev : Device
* UINT32 len : Data length
* UINT8 sector_index : sector 0,1,2
* UINT8 *data : Data to be read
* Return : INT32 : Read data length
*
*/
INT32 sto_otp_get_data(struct sto_device *dev, UINT8 *data, UINT32 offset, INT32 len,UINT8 sector_index)
{

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

	sto_mutex_enter(dev);
	//printf(" sto_get_data(),offset: %x\n", offset);
	lseek((int)dev->priv, (offset+(sector_index+1)*0x1000), SEEK_SET);
	ioctl((int)dev->priv,ALI_FLASH_SECTOR_OTP_RW, 0);
	ret_len+=read((int)dev->priv, data, len);
	sto_mutex_exit(dev);

	STO_PRINTF("Leave sto_get_data(), ret_len: %x\n", ret_len);
	return ret_len;

	// return 1;
}
/*
* Name : sto_otp_get_status()
* Description : Read data from storage
* Parameter : struct sto_device *dev : Device
* Return : UINT8 : otp status
* otp_status = 1,flash otp sector1 is locked
* otp_status = 2,flash otp sector2 is locked
* otp_status = 4,flash otp sector3 is locked
* otp_status = 7,all flash otp sector(1,2,3) is locked
*/
UINT8 sto_otp_get_status( struct sto_device *dev )
{
	UINT8 otp_status = 0;
	if (HLD_DEV_TYPE_NOR == dev->type)
	{
		ioctl((int)dev->priv,ALI_FALSH_SECTOR_OTP_GET_STATUS, &otp_status);
		STO_PRINTF("otp_status [0x%d]\n",otp_status);
		return otp_status;
	}
	return 0;
}
/*
* Name : sto_otp_set_status()
* Description : Read data from storage
* Parameter : struct sto_device *dev : Device
* sector_index :otp sector sector_index
* Return : void
* sector_index = 0,locked flash otp sector1
* sector_index = 1,locked flash otp sector2
* sector_index = 2,locked flash otp sector3
*notice:if set otp,flash otp sector cann't write and erase.
*/
void sto_otp_set_status( struct sto_device *dev,UINT8 sector_index)
{

	if (HLD_DEV_TYPE_NOR == dev->type)
	{
		ioctl((int)dev->priv,ALI_FALSH_SECTOR_OTP_SET_STATUS, &sector_index);
		STO_PRINTF("sector_index [0x%d]\n",sector_index);
	}
}
/*
* Name : sto_otp_put_data()
* Description : Write data into storage
* Parameter : struct sto_device *dev : Device
* UINT8 *data : Data to be write
* UINT32 len : Data length
* UINT8 sector_index :sector0,sector 1,sector2
* Return : INT32 : Write data length
*
*/
INT32 sto_otp_put_data(struct sto_device *dev, UINT32 offset, UINT8 *data, INT32 len,UINT8 sector_index)
{
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
	if (HLD_DEV_TYPE_NOR == dev->type)
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

}
/*
* Name : sto_get_data()
* Description : Read data from storage
* Parameter : struct sto_device *dev : Device
* UINT32 len : Data length
* UINT8 *data : Data to be read
* Return : INT32 : Read data length
*
*/
INT32 sto_get_data(struct sto_device *dev, UINT8 *data, UINT32 offset, INT32 len)
{
	int ret_len=0;

	STO_PRINTF("Enter sto_get_data(), len: %x\n", len);
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if(len==0)
		return 0;
	if ((INT32)offset + len > (INT32)dev->totol_size)
		len = dev->totol_size - offset;

	sto_mutex_enter(dev);
	lseek((int)dev->priv, offset, SEEK_SET);
	ret_len+=read((int)dev->priv, data, len);
	sto_mutex_exit(dev);

	STO_PRINTF("Leave sto_get_data(), ret_len: %x\n", ret_len);
	return ret_len;

}

INT32 sto_local_sflash_attach(struct sto_flash_info *param)
{
	struct sto_device *dev;
	unsigned int ret;

	dev = dev_alloc(sto_sflash_local_name, HLD_DEV_TYPE_NOR, sizeof(struct sto_device));
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
	if(dev->flags & (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING))
	{
		STO_PRINTF("Error, STO device still running.\n");
		return !SUCCESS;
	}
	dev_free(dev);

	return SUCCESS;
}

INT32 sto_nand_attach()
{
	struct sto_device *dev;

	// dev = dev_alloc(sto_nand_local_name, HLD_DEV_TYPE_NAND, sizeof(struct sto_device));
	dev = dev_alloc(sto_nand_local_name, HLD_DEV_TYPE_STO, sizeof(struct sto_device));
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
	if(dev->flags & (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING))
	{
		STO_PRINTF("Error, STO device still running.\n");
		return !SUCCESS;
	}

	dev_free(dev);
	return SUCCESS;
}

