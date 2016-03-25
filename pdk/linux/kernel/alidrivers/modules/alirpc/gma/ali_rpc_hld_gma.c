/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_rpc_hld_gma.c
 *  (I)
 *  Description: hld gma remote process call api
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.07.12			Sam			Create
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

#include <rpc_hld/ali_rpc_hld_gma.h>

#include "../ali_rpc.h"

int gma_attach_m36f(int layer_num)
{
		

	jump_to_func(NULL, ali_rpc_call, layer_num, (LLD_GMA_M36F_MODULE<<24)|(1<<16)|FUNC_GMA_M36F_ATTACH, NULL);	

	
}

UINT32 desc_gma_open[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct layer_pars)),
  1, DESC_P_PARA(0, 1, 0), 
  //desc of pointer ret
  0,                          
  0,
};

UINT32 desc_gma_scale[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct scale_pars)),
  1, DESC_P_PARA(0, 1, 0), 
  //desc of pointer ret
  0,                          
  0,
};

UINT32 desc_gma_set_pallette[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct pallette_pars)),
  1, DESC_P_PARA(0, 1, 0), 
  //desc of pointer ret
  0,                          
  0,
};

UINT32 desc_gma_get_pallette[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct pallette_pars)),
  1, DESC_P_PARA(0, 1, 0), 
  //desc of pointer ret
  0,                          
  0,
};

UINT32 desc_gma_create_region[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct region_pars)),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

UINT32 desc_gma_set_region_info[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct region_pars)),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

UINT32 desc_gma_get_region_info[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct region_pars)),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

UINT32 desc_gma_set_enhance_info[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct gma_enhance_pars)),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

RET_CODE gma_open(struct gma_device *dev, struct layer_pars *pars)
{
		
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_GMA_MODULE<<24)|(2<<16)|FUNC_GMA_OPEN, desc_gma_open);

	
}

void gma_close(struct gma_device *dev)
{
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_GMA_MODULE<<24)|(1<<16)|FUNC_GMA_CLOSE, NULL);  
}

RET_CODE gma_io_control(struct gma_device *dev, UINT32 cmd, UINT32 arg)
{
			
	UINT32 *desc = NULL;

	switch(cmd)
	{
		case GMA_IO_SET_GLOBAL_ALPHA:
			break;
		case GMA_IO_SET_ENHANCE_PAR:
			desc = desc_gma_set_enhance_info;
			break;
		case GMA_IO_SET_REGION_BY:
			break;
		default:
			return RET_FAILURE;			
	}
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_GMA_MODULE<<24)|(3<<16)|FUNC_GMA_IO_CONTROL, desc);
	
	
}

RET_CODE gma_show(struct gma_device *dev, int on)
{
		
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_GMA_MODULE<<24)|(2<<16)|FUNC_GMA_SHOW, NULL);  

	
}

RET_CODE gma_scale(struct gma_device *dev, struct scale_pars *pars)
{
		
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_GMA_MODULE<<24)|(2<<16)|FUNC_GMA_SCALE, desc_gma_scale);  

	
}

RET_CODE gma_set_pallette(struct gma_device *dev, struct pallette_pars *pars)
{
		
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_GMA_MODULE<<24)|(2<<16)|FUNC_GMA_SET_PALLETTE, desc_gma_set_pallette);  

	
}

RET_CODE gma_get_pallette(struct gma_device *dev, struct pallette_pars *pars)
{
		
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_GMA_MODULE<<24)|(2<<16)|FUNC_GMA_GET_PALLETTE, desc_gma_get_pallette);  

	
}

RET_CODE gma_create_region(struct gma_device *dev, int reg_id, struct region_pars *pars)
{
		
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_GMA_MODULE<<24)|(3<<16)|FUNC_GMA_CREATE_REGION, desc_gma_create_region);  

	
}

void gma_delete_region(struct gma_device *dev, int reg_id)
{
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_GMA_MODULE<<24)|(2<<16)|FUNC_GMA_DELETE_REGION, NULL);  
}

RET_CODE gma_get_region_info(struct gma_device *dev, int reg_id, struct region_pars *pars)
{
		
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_GMA_MODULE<<24)|(3<<16)|FUNC_GMA_GET_REGION_INFO, desc_gma_get_region_info);  

		
}

RET_CODE gma_set_region_info(struct gma_device *dev, int reg_id, struct region_pars *pars)
{
		
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_GMA_MODULE<<24)|(3<<16)|FUNC_GMA_SET_REGION_INFO, desc_gma_set_region_info);  

	
}

RET_CODE gma_region_show(struct gma_device *dev, int reg_id, int on)
{
		
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_GMA_MODULE<<24)|(3<<16)|FUNC_GMA_REGION_SHOW, NULL);  

	
}

