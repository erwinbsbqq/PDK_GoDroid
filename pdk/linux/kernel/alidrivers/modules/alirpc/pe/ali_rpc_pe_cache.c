/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_rpc_pe_cache.c
 *  (I)
 *  Description: pe cache Remote Call Process API
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
//#include <linux/smp_lock.h>
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
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/ali_rpc.h>

#include <rpc_hld/ali_rpc_pe.h>

#include "../ali_rpc.h"

#if 0
#define PRF printk
#else
#define PRF(...) 	do{}while(0)
#endif

#define PE_CACHE_MUTEX_CREATE ali_rpc_mutex_create
#define PE_CACHE_MUTEX_DELETE ali_rpc_mutex_delete
#define PE_CACHE_MUTEX_LOCK ali_rpc_mutex_lock
#define PE_CACHE_MUTEX_UNLOCK ali_rpc_mutex_unlock

static int m_rpc_pe_cache_init = 0;

static pe_cache pe_cache_info[PE_CACHE_NUM];
static pe_cache_ex * pe_cache_info_ex = NULL;

static pe_cache_cmd *pe_cache_cmd_buf = NULL;
static int pe_cache_mutex = -1;
static int pe_cache_res_data_size[PE_CACHE_NUM] = {0,};
static int pe_cache_res_real_size[PE_CACHE_NUM] = {0, };
static INT64 pe_cache_total_size[PE_CACHE_NUM] = {0,};
static int pe_cache_new_data[PE_CACHE_NUM] = {0,};

int ali_rpc_pe_cache_open_flag(void)
{
	if(pe_cache_cmd_buf->status == PE_CACHE_CMD_STATUS_NEW)
	{
		if(pe_cache_cmd_buf->type == PE_CACHE_CMD_OPEN)
			return 1;
	}

	return 0;
}

int ali_rpc_pe_cache_get_cmd(unsigned long *par1, unsigned long *par2
	, unsigned long *par3, unsigned long *par4)
{
	int cmd = PE_CACHE_CMD_NULL;

	if(pe_cache_cmd_buf->status == PE_CACHE_CMD_STATUS_NEW)
	{
		cmd = pe_cache_cmd_buf->type;
		switch(cmd)
		{
			case PE_CACHE_CMD_OPEN:
				*par1 = pe_cache_cmd_buf->param[0];
				*par2 = pe_cache_cmd_buf->param[1];
				*par3 = pe_cache_cmd_buf->param[2];
				*par4 = pe_cache_cmd_buf->param[3];
				break;
			case PE_CACHE_CMD_CLOSE:
				*par1 = pe_cache_cmd_buf->param[0];
				break;
			case PE_CACHE_CMD_SEEK:
				*par1 = pe_cache_cmd_buf->param[0];
				*par2 = pe_cache_cmd_buf->param[1];
				*par3 = pe_cache_cmd_buf->param[2];
				*par4 = pe_cache_cmd_buf->param[3];
				break;
			default:
				break;
		}
		PRF("%s : get the new pe cache cmd %d\n", __FUNCTION__, cmd);
	}	

	return cmd;
}

void ali_rpc_pe_cache_finish_cmd(unsigned long ret_value)
{
	pe_cache_cmd_buf->param[0] = ret_value;
	pe_cache_cmd_buf->status = PE_CACHE_CMD_STATUS_IMPLEMENTED;
}

int ali_rpc_pe_cache_request(int index, void **buf_start, int *buf_size)
{
	pe_cache_ex *info = &pe_cache_info_ex[index];	
	pe_cache_ex pc;
	int free_size = 0;
	int req_size = *buf_size;

	PE_CACHE_MUTEX_LOCK(info->mutex, ALI_RPC_MAX_TIMEOUT);
		
	if(info->status != PE_CACHE_OPENED)
	{	
		PE_CACHE_MUTEX_UNLOCK(info->mutex);
	
		return PE_CACHE_REQ_FAIL;		
	}

	memcpy(&pc, info, sizeof(pe_cache_ex));
	
	PE_CACHE_MUTEX_UNLOCK(info->mutex);

	if(pc.sub_status == PE_FILE_READING)
	{
		free_size = pc.cache_size - pc.data_len;
		if(free_size > pe_cache_res_data_size[index])
		{
			if(pc.wr_pos >= pc.rd_pos)
			{
				free_size = pc.rd_pos + 1 - pe_cache_res_data_size[index];
				*buf_size = pc.cache_size - pc.wr_pos;
				//if(free_size < 0)
				//	*buf_size += free_size;
			}
			else
				*buf_size = pc.rd_pos - pc.wr_pos - pe_cache_res_data_size[index];
			if(*buf_size > pe_cache_info[index].block_size)
				*buf_size = pe_cache_info[index].block_size;

			if(req_size < *buf_size)
				*buf_size = req_size;

			*buf_start = pc.cache_buff + pc.wr_pos;
			
			//PRF("req ok %d\n", *buf_size);
			return PE_CACHE_REQ_OK;
		}	
		
		return PE_CACHE_REQ_BUSY;
	}
	
	return PE_CACHE_REQ_FAIL;
}

void ali_rpc_pe_cache_update(int index, int update_size)
{
	pe_cache_ex *info = &pe_cache_info_ex[index];	
	
	PE_CACHE_MUTEX_LOCK(info->mutex, ALI_RPC_MAX_TIMEOUT);	

	if(info->status != PE_CACHE_OPENED)
	{	
		PE_CACHE_MUTEX_UNLOCK(info->mutex);
	
		return;		
	}
	
	if(info->sub_status == PE_FILE_READING)
	{
		info->wr_pos += update_size;
		while(info->wr_pos >= info->cache_size)
			info->wr_pos -= info->cache_size;

		info->data_len += update_size;
		info->file_offset += update_size;

		pe_cache_new_data[index] = 1;
		
		if(pe_cache_res_real_size[index] < pe_cache_res_data_size[index])
		{
			if(pe_cache_total_size[index] < info->cache_size)
				pe_cache_total_size[index] += update_size;

			if(pe_cache_total_size[index] > info->cache_size)
				pe_cache_total_size[index] = info->cache_size;

			pe_cache_res_real_size[index] = pe_cache_total_size[index] - info->data_len;
			if(pe_cache_res_real_size[index] > pe_cache_res_data_size[index])
				pe_cache_res_real_size[index] = pe_cache_res_data_size[index];

			PRF("%s : idx %d total cache %d res real size %d\n", __FUNCTION__, index, *(int *)&pe_cache_total_size[index]
				, pe_cache_res_real_size[index]);
		}
		
		if(pe_cache_info[index].file_size != 0)
		{
			if(info->file_offset >= pe_cache_info[index].file_size)
			{
				info->sub_status = PE_FILE_EOF;
				PRF("pe cache file end\n");
			}
		}
	}

	PE_CACHE_MUTEX_UNLOCK(info->mutex);		
}

void ali_rpc_pe_cache_file_end(int index)
{
	pe_cache_ex *info = &pe_cache_info_ex[index];	
	
	PE_CACHE_MUTEX_LOCK(info->mutex, ALI_RPC_MAX_TIMEOUT);

	info->sub_status = PE_FILE_EOF;

	PE_CACHE_MUTEX_UNLOCK(info->mutex);			
}

void ali_rpc_pe_cache_file_fail(int index)
{
	pe_cache_ex *info = &pe_cache_info_ex[index];	
	
	PE_CACHE_MUTEX_LOCK(info->mutex, ALI_RPC_MAX_TIMEOUT);

	info->sub_status = PE_FILE_FAILED;

	PE_CACHE_MUTEX_UNLOCK(info->mutex);	
}

int ali_rpc_pe_cache_open(void *file, long long file_size, void *buf, int size, int block_size, int res_size)
{
	pe_cache *info = pe_cache_info;
	pe_cache_ex *info_ex = pe_cache_info_ex;
	int i = 0;

	for(i = 0;i < PE_CACHE_NUM;i++,info++,info_ex++)
	{
		if(info_ex->status == PE_CACHE_CLOSED)
		{
			pe_cache_res_data_size[i] = res_size;
			pe_cache_res_real_size[i] = 0;
			pe_cache_total_size[i] = 0;
			pe_cache_new_data[i] = 0;
			
			if(buf == NULL)
			{
				PRF("%s : allocate the pe cache %d buf %d\n", __FUNCTION__, i, size);
				buf = kmalloc(size, GFP_KERNEL);
				info->internal_cache = 1;
			}
			else
				info->internal_cache = 0;

	        	info->file_read = (void *)1;
			info->file_seek = (void *)1;
			info->file_eof = (void *)1;
			info->file_tell = (void *)1;
			info->file_close = (void *)1;
			info->file_size = file_size;
			info->block_size = block_size;

			info_ex->file_offset = 0;
			info_ex->cache_buff = buf;
			info_ex->cache_size = size;
			info_ex->data_len = 0;
			info_ex->rd_pos = 0;
			info_ex->wr_pos = 0;
			info_ex->status = PE_CACHE_OPENED;
			info_ex->sub_status = PE_FILE_READING;
			break;
		}
	}

	PRF("%s : open pe cache id %d\n", __FUNCTION__, i);

	return (i >= PE_CACHE_NUM ? -1 : i);
}

void ali_rpc_pe_cache_close(int index)
{
	pe_cache *info = &pe_cache_info[index];	
	pe_cache_ex *info_ex = &pe_cache_info_ex[index];
	
	PE_CACHE_MUTEX_LOCK(info_ex->mutex, ALI_RPC_MAX_TIMEOUT);

	if(info_ex->status == PE_CACHE_OPENED)	
	{
		if(info->internal_cache)
			kfree(info_ex->cache_buff);

		info_ex->status = PE_CACHE_CLOSED;
	}

	PRF("%s : close pe cache %d\n", __FUNCTION__, index);
	
	PE_CACHE_MUTEX_UNLOCK(info_ex->mutex);		
}

int ali_rpc_pe_cache_seek(int index, long long offset, int where)
{
	pe_cache_ex *pc = &pe_cache_info_ex[index];
	int ret = PE_CACHE_SEEK_OK;
	INT64 cache_start_offset;
	INT64 cache_end_offset;
	
	PE_CACHE_MUTEX_LOCK(pc->mutex, ALI_RPC_MAX_TIMEOUT);

	if((pc->status == PE_CACHE_OPENED)
		&& (pc->sub_status != PE_FILE_FAILED))
	{

		if(pe_cache_res_real_size[index] < pe_cache_res_data_size[index])
		{
			pe_cache_res_real_size[index] = pe_cache_total_size[index] - pc->data_len;
			if(pe_cache_new_data[index])
			{
				if(pe_cache_res_real_size[index] > pe_cache_res_data_size[index])
					pe_cache_res_real_size[index] = pe_cache_res_data_size[index];
			}
			
			//PRF("%s : total cache %d res real size %d\n", __FUNCTION__, *(int *)&pe_cache_total_size[index]
			//	, pe_cache_res_real_size[index]);			
		}
		
		pe_cache_new_data[index] = 0;
		cache_start_offset = pc->file_offset - (INT64)(pc->data_len);
		cache_end_offset = pc->file_offset;		

#if 0
		PRF("%s : flag %d offset %x%x start %x%x end %x%x\n", __FUNCTION__, where
			, *(((int *)&offset) + 1), *(int *)&offset
			, *(((int *)&cache_start_offset) + 1), *(int *)&cache_start_offset
			, *(((int *)&cache_end_offset) + 1), *(int *)&cache_end_offset);
#endif
		
		switch(where)
		{
			case SEEK_SET:
				break;
			case SEEK_CUR:
				offset += cache_start_offset;
				break;
			case SEEK_END:
				offset += pe_cache_info[index].file_size;				
				break;
			default:
				offset = -1;
				break;		
		}
		
		if((offset < 0) 
			|| ((offset > pe_cache_info[index].file_size) && (pe_cache_info[index].file_size != 0)))
		{
			PRF("%s : fail\n", __FUNCTION__);
			ret = PE_CACHE_SEEK_FAIL;	
		}
		
		if(ret != PE_CACHE_SEEK_FAIL)
		{
			cache_start_offset -= pe_cache_res_real_size[index];

			if((offset < cache_start_offset) || (offset > cache_end_offset))
			{
				pe_cache_total_size[index] = 0;
				pe_cache_res_real_size[index] = 0;
				
				pc->data_len = 0;
				pc->rd_pos = 0;
				pc->wr_pos = 0;
				pc->file_offset = offset;

				ret = PE_CACHE_SEEK_MORE;
				//PRF("pe seek mode offset %x%x\n",  *(((int *)&pc->file_offset) + 1), *(int *)&pc->file_offset);
			}
			else
			{	
				cache_end_offset = offset - cache_start_offset;
				if(cache_end_offset >= pe_cache_res_real_size[index])
				{
					cache_end_offset -= pe_cache_res_real_size[index];
					pc->rd_pos += cache_end_offset;
					while(pc->rd_pos >= pc->cache_size)
						pc->rd_pos -= pc->cache_size;
					
					pc->data_len -= cache_end_offset;		
					
					pe_cache_res_real_size[index] += cache_end_offset;
					//if(pe_cache_res_real_size > pe_cache_res_data_size)
					//	pe_cache_res_real_size = pe_cache_res_data_size;					
				}
				else
				{
					cache_end_offset = pe_cache_res_real_size[index] - cache_end_offset;
					if(pc->rd_pos >= cache_end_offset)
						pc->rd_pos -= cache_end_offset;
					else
						pc->rd_pos = pc->rd_pos + pc->cache_size - cache_end_offset;

					pc->data_len += cache_end_offset;

					pe_cache_res_real_size[index] -= cache_end_offset;					
				}

				ret = PE_CACHE_SEEK_OK;

#if 0				
				PRF("pe seek ok rd %d data len %d res %d\n", (int)pc->rd_pos, (int)pc->data_len
					, pe_cache_res_real_size[index]);
#endif
			}				

			pc->sub_status = PE_FILE_READING;	
				
			if(pe_cache_info[index].file_size != 0)
			{
				if(pc->file_offset >= pe_cache_info[index].file_size)
				{
					pc->sub_status = PE_FILE_EOF;
					PRF("pe cache file end\n");
				}
			}		
		}
	}

	PE_CACHE_MUTEX_UNLOCK(pc->mutex);	
	
	return ret;
}

int ali_rpc_pe_cache_need_new_data(int index)
{
	pe_cache_ex *info = &pe_cache_info_ex[index];		
	int ret = 0;

	PE_CACHE_MUTEX_LOCK(info->mutex, ALI_RPC_MAX_TIMEOUT);	

	if(info->status == PE_CACHE_OPENED)
	{
		if(info->sub_status == PE_FILE_READING)
		{
			if((info->cache_size - info->data_len) > pe_cache_res_data_size[index])
				ret = 1;
		}
	}
	
	PE_CACHE_MUTEX_UNLOCK(info->mutex);
	
	return ret;
}

int ali_rpc_pe_cache_init(int mutex_id)
{
	int i = 0;
	
	if(m_rpc_pe_cache_init)
		return 0;

	PRF("size pe cache info %d\n", sizeof(*pe_cache_info));
	pe_cache_info_ex = ali_rpc_malloc_shared_mm(sizeof(pe_cache_ex) * PE_CACHE_NUM);
	if(pe_cache_info_ex == NULL)
	{
		PRF("get the rpc pe cache info fail\n");
		return -1;
	}
	memset((void *)pe_cache_info_ex, 0, sizeof(pe_cache_ex) * PE_CACHE_NUM);
	memset((void *)pe_cache_info, 0, sizeof(pe_cache) * PE_CACHE_NUM);
	
	for(i = 0;i < PE_CACHE_NUM;i++)
	{
		pe_cache_info_ex[i].mutex = PE_CACHE_MUTEX_CREATE();
		if(pe_cache_info_ex[i].mutex <= 0)
		{
			PRF("create pe cache mutex fail\n");
			goto FAIL;
		}
	}

	pe_cache_cmd_buf = ali_rpc_malloc_shared_mm(sizeof(*pe_cache_cmd_buf));
	if(pe_cache_cmd_buf == NULL)
	{
		PRF("get the rpc pe cache cmd buf fail\n");
		goto FAIL;
	}
	
	pe_cache_cmd_buf->status = PE_CACHE_CMD_STATUS_IMPLEMENTED;
	pe_cache_cmd_buf->type = PE_CACHE_CMD_NULL;

	pe_cache_mutex = mutex_id;
	m_rpc_pe_cache_init = 1;
	PRF("%s : done\n", __FUNCTION__);

	video_engine_pe_cache_init(pe_cache_info_ex, pe_cache_cmd_buf, pe_cache_mutex);	
	
	return 0;

FAIL:
	if(pe_cache_info_ex->mutex > 0)
	{
		PE_CACHE_MUTEX_DELETE(pe_cache_info_ex->mutex);
	}

	if(pe_cache_cmd_buf != NULL)
		ali_rpc_free_shared_mm((void *)pe_cache_cmd_buf, sizeof(*pe_cache_cmd_buf));

	if(pe_cache_info_ex != NULL)
		ali_rpc_free_shared_mm((void *)pe_cache_info_ex, sizeof(*pe_cache_info_ex));
		
	PRF("%s : fail\n", __FUNCTION__);
	return -1;
}

void ali_rpc_pe_cache_release(void)
{
	int i = 0;

    if (!m_rpc_pe_cache_init)
        return;

	// Roman add at 20100705 to fix bug:
	// jpeg+mp3 burning test crash.
	video_engine_pe_cache_release();

	if(pe_cache_cmd_buf != NULL)
		ali_rpc_free_shared_mm((void *)pe_cache_cmd_buf, sizeof(*pe_cache_cmd_buf));


	if(pe_cache_info_ex != NULL)
	{
		for(i = 0;i < PE_CACHE_NUM;i++)
		{
			if(pe_cache_info_ex[i].mutex > 0)
			{
				PE_CACHE_MUTEX_DELETE(pe_cache_info_ex[i].mutex);
			}
		}
		ali_rpc_free_shared_mm((void *)pe_cache_info_ex, sizeof(*pe_cache_info_ex) * PE_CACHE_NUM);
	}

	m_rpc_pe_cache_init = 0;
	
	PRF("%s : done\n", __FUNCTION__);
}

