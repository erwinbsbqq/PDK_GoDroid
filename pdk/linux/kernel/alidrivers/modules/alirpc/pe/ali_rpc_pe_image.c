/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_rpc_pe_image.c
 *  (I)
 *  Description: pe image Remote Call Process API
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

typedef struct Imagedec_frm_t
{
	UINT8 *frm_y_addr;
	UINT32 frm_y_len;
	UINT8 *frm_c_addr;
	UINT32 frm_c_len;
	UINT32 busy;
}Imagedec_frm,*pImagedec_frm;

static UINT32 desc_ie_init[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct pe_image_cfg)), 
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_ie_decode[] = 
{ //desc of pointer para
  2, DESC_STATIC_STRU(0, sizeof(struct image_config)), DESC_STATIC_STRU(1, sizeof(struct image_slideshow_effect)), 
  2, DESC_P_PARA(0, 1, 0), DESC_P_STRU(1, 0, 1, offsetof(struct image_config, effect)),
  //desc of pointer ret
  0,                          
  0,
};


static UINT32 desc_ie_zoom[] = 
{ //desc of pointer para
  2, DESC_STATIC_STRU(0, sizeof(struct Rect)), DESC_STATIC_STRU(1, sizeof(struct Rect)), 
  2, DESC_P_PARA(0, 0, 0), DESC_P_PARA(1, 1, 1),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_ie_get_info[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct image_info)),  
  1, DESC_P_PARA(0, 1, 0),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_ie_ioctrl_cfg_deo_frm[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(Imagedec_frm)), 
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};

int image_engine_init(struct pe_image_cfg *pe_image_cfg)
{
		
	
	jump_to_func(NULL, ali_rpc_call, pe_image_cfg, (LIB_PE_IMAGE_ENGINE_MODULE<<24)|(1<<16)|FUNC_IMAGE_ENGINE_INIT, desc_ie_init);

	
}

int image_engine_cleanup(void)
{
		
	
	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_IMAGE_ENGINE_MODULE<<24)|(0<<16)|FUNC_IMAGE_ENGINE_CLEANUP, NULL);

		
}

int image_engine_rotate(unsigned char rotate_angle)
{
		

	jump_to_func(NULL, ali_rpc_call, rotate_angle, (LIB_PE_IMAGE_ENGINE_MODULE<<24)|(1<<16)|FUNC_IMAGE_ENGINE_ROTATE, NULL);

	
}

int image_engine_decode(char *filename, struct image_config *cfg)
{
		

	jump_to_func(NULL, ali_rpc_call, filename, (LIB_PE_IMAGE_ENGINE_MODULE<<24)|(2<<16)|FUNC_IMAGE_ENGINE_DECODE, desc_ie_decode);

		
}

int image_engine_show(void)
{
		

	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_IMAGE_ENGINE_MODULE<<24)|(0<<16)|FUNC_IMAGE_ENGINE_SHOW, NULL);

			
}

int image_engine_abort(void)
{
		

	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_IMAGE_ENGINE_MODULE<<24)|(0<<16)|FUNC_IMAGE_ENGINE_ABORT, NULL);

		
}

int image_engine_zoom(struct Rect *dstRect, struct Rect *srcRect)
{
		

	jump_to_func(NULL, ali_rpc_call, dstRect, (LIB_PE_IMAGE_ENGINE_MODULE<<24)|(2<<16)|FUNC_IMAGE_ENGINE_ZOOM, desc_ie_zoom);

		
}

int image_engine_get_info(char *filename, struct image_info *info)
{
	

	jump_to_func(NULL, ali_rpc_call, filename, (LIB_PE_IMAGE_ENGINE_MODULE<<24)|(2<<16)|FUNC_IMAGE_ENGINE_GET_INFO, desc_ie_get_info);

			
}

BOOL imagedec_ioctl(unsigned long id,UINT32 cmd, UINT32 para)
{
	
	
	UINT32 *desc = NULL;

	switch(cmd)
	{
		case IMAGEDEC_IO_CMD_CFG_DEO_FRM:
			desc = desc_ie_ioctrl_cfg_deo_frm;
			break;		
		case IMAGEDEC_IO_CMD_OPEN_HW_ACCE:
		case IMAGEDEC_IO_CMD_FILL_LOGO:
		case IMAGEDEC_IO_CMD_CLEAN_FRM:
		case IMAGEDEC_IO_CMD_NEED_LOGO:
		default:
			desc = NULL;
			break;
	}

	jump_to_func(NULL, ali_rpc_call, id, (LIB_PE_IMAGE_ENGINE_MODULE<<24)|(3<<16)|FUNC_IMAGE_ENGINE_IO_CONTROL, desc);

			
}

