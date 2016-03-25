/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_pe_image.c
 *  (I)
 *  Description: ali pe image engine for media player library
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.07.05			Sam			Create
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
#include <ali_pe_common.h>
#include <rpc_hld/ali_rpc_pe.h>
#include <ali_cache.h>
#include "ali_pe.h"

#define LOCK_MUTEX	\
	if(down_interruptible(&m_image_sem)){ \
		PE_PRF("ali_pe down sem fail\n"); \
		return -1; \
	}

#define UNLOCK_MUTEX \
	up(&m_image_sem)

extern volatile unsigned long *g_ali_pe_rpc_arg[MAX_PE_RPC_ARG_NUM];
extern volatile int g_ali_pe_rpc_arg_size[MAX_PE_RPC_ARG_NUM];

static struct semaphore m_image_sem;

static int real_cb_enable = 0;
static int image_cb_pending = 0;
static unsigned long image_cb_par1 = 0;
static unsigned long image_cb_par2 = 0;
static int real_cb(unsigned long type, unsigned long param)
{
	LOCK_MUTEX;

	image_cb_pending = 1;
	image_cb_par1 = type;
	image_cb_par2 = param;
	
	UNLOCK_MUTEX;
}

static void img_cb(unsigned long type, unsigned long param)
{
	PE_PRF("%s : type %d, param %d\n", __FUNCTION__, (int)type, (int)param);		
	if(real_cb_enable)
		real_cb(type, param);
}

int ali_pe_image_routine(struct ali_pe_info *info)
{
	LOCK_MUTEX;
	
	if(image_cb_pending){		
		image_cb_pending = 0;
		info->image.par1 = image_cb_par1;
		info->image.par2 = image_cb_par2;
		
		info->flag |= ALI_PE_CMD_FLAG_IMAGE;
	}
	
	UNLOCK_MUTEX;

	return 0;
}

static int io_control(void)
{
	int ret = 0;

	PE_PRF("pe image io cmd %d\n", *(int *)g_ali_pe_rpc_arg[0]);
	switch(*(UINT32 *)g_ali_pe_rpc_arg[0])
	{
		case IMAGEDEC_IO_CMD_CFG_DEO_FRM:
		{
#if 0			
			ret = imagedec_ioctl(*(unsigned long *)g_ali_pe_rpc_arg[0]
				, *(unsigned long *)g_ali_pe_rpc_arg[1]
				, (unsigned long)g_ali_pe_rpc_arg[2]);
#endif
			break;
		}
		default:
			ret = imagedec_ioctl(*(unsigned long *)g_ali_pe_rpc_arg[0]
				, *(unsigned long *)g_ali_pe_rpc_arg[1]
				, *(unsigned long *)g_ali_pe_rpc_arg[2]);			
			break;
	}
	return ret;
}

int ali_pe_image_operation(int API_idx)
{
#define RPC_IMAGE_ENGINE_INIT					1
#define RPC_IMAGE_ENGINE_CLEANUP				2
#define RPC_IMAGE_ENGINE_ROTATE				3
#define RPC_IMAGE_ENGINE_DECODE				4
#define RPC_IMAGE_ENGINE_SHOW				5
#define RPC_IMAGE_ENGINE_ABOUT				6
#define RPC_IMAGE_ENGINE_ZOOM					7
#define RPC_IMAGE_ENGINE_GET_INFO			8
#define RPC_IMAGE_ENGINE_IOCTL				9

	int ret = 0;

	PE_PRF("%s : api idx %d\n", __FUNCTION__, API_idx);
	switch(API_idx)
	{
		case RPC_IMAGE_ENGINE_INIT:
		{
extern unsigned long __G_ALI_MM_VIDEO_SIZE;			
extern unsigned long __G_ALI_MM_VIDEO_START_ADDR;
extern unsigned long g_ali_pe_buf_size;
extern unsigned long g_ali_pe_buf_start_addr;

#define __MM_FB0_Y_LEN			(1920*1088+1024)//(736*576+512)	//for high definition jpg decode
#define __MM_FB1_Y_LEN			__MM_FB0_Y_LEN
#define __MM_FB2_Y_LEN			__MM_FB0_Y_LEN

#define __MM_FB0_C_LEN			(__MM_FB0_Y_LEN/2)
#define __MM_FB1_C_LEN			__MM_FB0_C_LEN
#define __MM_FB2_C_LEN			__MM_FB0_C_LEN

#define __MM_FB3_Y_LEN			(736*576+1024)
#define __MM_FB3_C_LEN			(__MM_FB3_Y_LEN/2)
#define __MM_FB4_Y_LEN			__MM_FB3_Y_LEN
#define __MM_FB4_C_LEN			__MM_FB3_C_LEN
#define __MM_FB5_Y_LEN          __MM_FB3_Y_LEN
#define __MM_FB5_C_LEN          __MM_FB3_C_LEN
#define __MM_FB6_Y_LEN          __MM_FB3_Y_LEN
#define __MM_FB6_C_LEN          __MM_FB3_C_LEN

#define __MM_FB_LEN			    		0x19c6200
#define __MM_MAF_LEN					0xd0000
#define __MM_DVW_LEN					0
#define __MM_FB_START_ADDR			((__G_ALI_MM_VIDEO_START_ADDR + __G_ALI_MM_VIDEO_SIZE - __MM_FB_LEN)&0XFFFFFF00)
#define __MM_FB0_Y_START_ADDR   (__MM_FB_START_ADDR)
#define __MM_FB0_C_START_ADDR   (__MM_FB0_Y_START_ADDR+__MM_FB0_Y_LEN)

#define __MM_FB1_Y_START_ADDR   ((__MM_FB0_C_START_ADDR+__MM_FB0_C_LEN)&0XFFFFFE00)
#define __MM_FB1_C_START_ADDR   ((__MM_FB1_Y_START_ADDR+__MM_FB1_Y_LEN)&0XFFFFFE00)

#define __MM_FB2_Y_START_ADDR   ((__MM_FB1_C_START_ADDR+__MM_FB1_C_LEN)&0XFFFFFE00)
#define __MM_FB2_C_START_ADDR   ((__MM_FB2_Y_START_ADDR+__MM_FB2_Y_LEN)&0XFFFFFE00)

#define __MM_FB3_Y_START_ADDR   ((__MM_FB2_C_START_ADDR+__MM_FB2_C_LEN)&0XFFFFFE00)
#define __MM_FB3_C_START_ADDR   ((__MM_FB3_Y_START_ADDR+__MM_FB3_Y_LEN)&0XFFFFFE00)
#define __MM_FB4_Y_START_ADDR	((__MM_FB3_C_START_ADDR+__MM_FB3_C_LEN)&0XFFFFFE00)
#define __MM_FB4_C_START_ADDR   ((__MM_FB4_Y_START_ADDR+__MM_FB4_Y_LEN)&0XFFFFFE00)
#define __MM_FB5_Y_START_ADDR	((__MM_FB4_C_START_ADDR+__MM_FB4_C_LEN)&0XFFFFFE00)
#define __MM_FB5_C_START_ADDR   ((__MM_FB5_Y_START_ADDR+__MM_FB5_Y_LEN)&0XFFFFFE00)
#define __MM_FB6_Y_START_ADDR	((__MM_FB5_C_START_ADDR+__MM_FB5_C_LEN)&0XFFFFFE00)
#define __MM_FB6_C_START_ADDR   ((__MM_FB6_Y_START_ADDR+__MM_FB6_Y_LEN)&0XFFFFFE00)

			struct pe_image_cfg *cfg = (struct pe_image_cfg *)g_ali_pe_rpc_arg[0];

			cfg->frm_y_size	= __MM_FB0_Y_LEN;
			cfg->frm_y_addr	= __MM_FB0_Y_START_ADDR;
			cfg->frm_c_size	= __MM_FB0_C_LEN;
			cfg->frm_c_addr	= __MM_FB0_C_START_ADDR;
			cfg->frm2_y_size = __MM_FB1_Y_LEN;
			cfg->frm2_y_addr = __MM_FB1_Y_START_ADDR;
			cfg->frm2_c_size = __MM_FB1_C_LEN;
			cfg->frm2_c_addr = __MM_FB1_C_START_ADDR;
			cfg->frm3_y_size = __MM_FB2_Y_LEN;
			cfg->frm3_y_addr = __MM_FB2_Y_START_ADDR;
			cfg->frm3_c_size = __MM_FB2_C_LEN;
			cfg->frm3_c_addr = __MM_FB2_C_START_ADDR;			
			cfg->decoder_buf = (UINT8 *)g_ali_pe_buf_start_addr;
			cfg->decoder_buf_len = g_ali_pe_buf_size;
		
			if(cfg->mp_cb != NULL)
				real_cb_enable = 1;
			else
				real_cb_enable = 0;		
			ret = image_engine_init((struct pe_image_cfg *)g_ali_pe_rpc_arg[0]);
			break;
		}
		case RPC_IMAGE_ENGINE_CLEANUP:
			ret = image_engine_cleanup();
			break;
		case RPC_IMAGE_ENGINE_ROTATE:
			ret = image_engine_rotate(*(unsigned char *)g_ali_pe_rpc_arg[0]);
			break;		
		case RPC_IMAGE_ENGINE_DECODE:		
			__CACHE_FLUSH_ALI((unsigned long)g_ali_pe_rpc_arg[0], g_ali_pe_rpc_arg_size[0]);
			ret = image_engine_decode((char *)((unsigned long)g_ali_pe_rpc_arg[0] | 0xA0000000)
				, (struct image_config *)g_ali_pe_rpc_arg[1]);
			break;
		case RPC_IMAGE_ENGINE_SHOW:
			ret = image_engine_show();
			break;
		case RPC_IMAGE_ENGINE_ABOUT:
			ret = image_engine_abort();
			break;
		case RPC_IMAGE_ENGINE_ZOOM:
			ret = image_engine_zoom((struct Rect *)g_ali_pe_rpc_arg[0]
				, (struct Rect *)g_ali_pe_rpc_arg[1]);
			break;
		case RPC_IMAGE_ENGINE_GET_INFO:
			__CACHE_FLUSH_ALI((unsigned long)g_ali_pe_rpc_arg[0], g_ali_pe_rpc_arg_size[0]);
			ret = image_engine_get_info((char *)((unsigned long)g_ali_pe_rpc_arg[0] | 0xA0000000)
				, (struct image_info *)g_ali_pe_rpc_arg[1]);
			break;
		case RPC_IMAGE_ENGINE_IOCTL:
			ret = io_control();
			break;
		default:
			ret = -1;
			break;
	}
	
	return ret;
}

void ali_pe_image_init(struct ali_pe_info *info)
{
        sema_init(&m_image_sem, 1);
	ali_rpc_register_callback(ALI_RPC_CB_IMG, img_cb);	
}

