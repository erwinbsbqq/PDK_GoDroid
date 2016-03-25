/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_pe_cache.c
 *  (I)
 *  Description: ali pe cache for media player library
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.06.28			Sam			Create
 ****************************************************************************/
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/vt.h>
#include <linux/init.h>
#include <linux/linux_logo.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/console.h>
#include <linux/kmod.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/efi.h>
#include <linux/fb.h>
#include <linux/ali_rpc.h>
#include <ali_pe_common.h>
#include <asm/io.h>
#include <rpc_hld/ali_rpc_pe.h>
#include <ali_cache.h>

#include "ali_pe.h"

static int m_mutex_id = -1;

#define LOCK_PE_CACHE		ali_rpc_mutex_lock(m_mutex_id, ALI_RPC_MAX_TIMEOUT)
#define UNLOCK_PE_CACHE	ali_rpc_mutex_unlock(m_mutex_id)

#define RES_SIZE				10240

void ali_pe_cache_mon_routine(struct ali_pe_info *info, enum ali_pe_status status)
{
	struct cache_info *c_info = NULL;
	int i = 0;
	
	LOCK_PE_CACHE;	

	if((status != ALI_PE_WORKING) && ali_rpc_pe_cache_open_flag())
	{
		//PE_PRF("%s : open flag is pending\n", __FUNCTION__);
		
		info->flag |= ALI_PE_CMD_FLAG_WORK;
		
		UNLOCK_PE_CACHE;
		return;
	}

	for(i = 0;i < ALI_PE_CACHE_NUM;i++)
	{
		if(ali_rpc_pe_cache_need_new_data(i)){
			c_info = &info->cache[i];
			if(!c_info->wait_new_data && ((info->flag & ALI_PE_CMD_FLAG_CACHE) == 0)){
				info->cache_pending = i;
				info->flag |= ALI_PE_CMD_FLAG_CACHE;				
				c_info->cmd = ALIPE_CACHE_REQ_NEW_DATA;
				
				PE_PRF("idx %d set pe cache new data cmd\n", i);
				
				c_info->wait_new_data = 1;
				break;
			}
		}
	}

	UNLOCK_PE_CACHE;	
}

int ali_pe_cache_routine(struct ali_pe_info *info)
{
	struct cache_info *c_info = NULL;	
	unsigned long par1, par2, par3, par4;
	int cmd = PE_CACHE_CMD_NULL;
	int index = 0;
	int ret = 0;

	LOCK_PE_CACHE;	
	
	cmd = ali_rpc_pe_cache_get_cmd(&par1, &par2, &par3, &par4);
	switch(cmd){
		case PE_CACHE_CMD_OPEN:
			index = ali_rpc_pe_cache_open((void *)par1,info->file_size_pending, (void *)par2, par3, par4, RES_SIZE);
			ali_rpc_pe_cache_finish_cmd((unsigned long)index);
			if(index >= 0)
			{
				info->cache_pending = index;
				info->flag |= ALI_PE_CMD_FLAG_CACHE;		
				c_info = &info->cache[index];		
				c_info->busy = 1;
				c_info->wait_new_data = 1;
				c_info->cmd = ALIPE_CACHE_OPEN;
				c_info->par1 = par1;
				c_info->par2 = par2;
				c_info->par3 = par3;
				c_info->par4 = par4;
				ret = 1;
				//PE_PRF("set pe cache open cmd\n");			
			}
			else
			{
				PE_PRF("pe cache open fail\n");
			}
			break;
		case PE_CACHE_CMD_CLOSE:
			index = par1;
			//if(index >= 0) cpp test: index >= 0 always true
			{
				ali_rpc_pe_cache_close(par1);
				ali_rpc_pe_cache_finish_cmd(0);
			
				info->cache_pending = index;
				info->flag |= ALI_PE_CMD_FLAG_CACHE;	
				c_info = &info->cache[index];			
				c_info->busy = 0;				
				c_info->cmd = ALIPE_CACHE_CLOSE;					
				ret = 1;						
				//PE_PRF("set pe cache close cmd\n");			
			}
            #if 0
			else
			{
				PE_PRF("pe cache close fail\n");
			}
            #endif
			break;
		case PE_CACHE_CMD_SEEK:
		{
			INT64 offset = 0;
			UINT32 *poffset = NULL;

			index = par1;
			poffset = (UINT32 *)&offset;
			*poffset = par2;
			*(poffset + 1) = par3;
			ret = ali_rpc_pe_cache_seek(index, offset, par4);
			ali_rpc_pe_cache_finish_cmd(ret == PE_CACHE_SEEK_FAIL ? -1 : 0);			

			if(ret == PE_CACHE_SEEK_MORE){
				/* set the pars of seek operation to be sent to the application
				in the user space */
				info->cache_pending = index;
				info->flag |= ALI_PE_CMD_FLAG_CACHE;
				c_info = &info->cache[index];				
				c_info->cmd = ALIPE_CACHE_SEEK;
				c_info->busy = 2;
				c_info->par2 = par2;
				c_info->par3 = par3;
				c_info->par4 = par4;
				ret = 1;				
				PE_PRF("idx %d set pe cache seek cmd\n", index);
			}
			else
				ret = 0;
				
			//PE_PRF("pe seek offset %x%x flag %d\n", (int)par3, (int)par2, (int)par4);			
			break;
		}
		default:
			break;
	}

	UNLOCK_PE_CACHE;
	
	return ret;
}

int ali_pe_cache_write(struct ali_pe_info *info, void *buf , int size)
{
	void *buf_start = NULL;
	int req_size = size;
	int write_count = 0;

	LOCK_PE_CACHE;

	if(info->cache[info->current_cache_idx].busy != 1)
	{
		PE_PRF("pe write fail cache %d busy %d\n", info->current_cache_idx, info->cache[info->current_cache_idx].busy);
		UNLOCK_PE_CACHE;		
		return 0;
	}

	if(info->cache[info->current_cache_idx].wait_new_data)
		info->cache[info->current_cache_idx].wait_new_data = 0;
	
	do{
		if(ali_rpc_pe_cache_request(info->current_cache_idx, &buf_start, &req_size) 
			== PE_CACHE_REQ_OK){

			copy_from_user(buf_start, buf, req_size);
			
			__CACHE_FLUSH_ALI((unsigned long)buf_start, req_size);
			
			ali_rpc_pe_cache_update(info->current_cache_idx, req_size);
			
			buf += req_size;
			write_count += req_size;
			size -= req_size;
			req_size = size;
		}
		else
			break;
	}while(size > 0);

	UNLOCK_PE_CACHE;

	return write_count;
}

void ali_pe_cache_init(struct ali_pe_info *info)
{
	if(info->cache_init == 1)
		return;
	
	memset((void *)info->cache, 0, sizeof(struct cache_info) * ALI_PE_CACHE_NUM);
	
	m_mutex_id = ali_rpc_mutex_create();
	if(m_mutex_id <= 0)
		return;

	if(ali_rpc_pe_cache_init(m_mutex_id) < 0){
		PE_PRF("init rpc pe cache fail\n");
		return;
	}
	
	info->cache_init = 1;
}

void ali_pe_cache_release(struct ali_pe_info *info)
{
	ali_rpc_pe_cache_release();
	
	if(m_mutex_id > 0){
		ali_rpc_mutex_delete(m_mutex_id);
		m_mutex_id = -1;
	}

	info->cache_init = 0;	
}

