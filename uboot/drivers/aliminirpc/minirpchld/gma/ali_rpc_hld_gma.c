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
#include <linux/version.h>
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
#include <linux/smp_lock.h>
#endif
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

//#include "../ali_rpc.h"
#include <ali_rpcng.h>

int gma_attach_m36f(int layer_num)
{
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, sizeof(Int32), &layer_num);

	return RpcCallCompletion(RPC_Svc_gma_attach,&p1,NULL);
}

RET_CODE gma_open(struct gma_device *dev, GMA_LAYER_PARS *pars)
{
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
	RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_GMA_LAYER_PARS, sizeof(GMA_LAYER_PARS), pars);

	return RpcCallCompletion(RPC_Svc_gma_open,&p1,&p2,NULL);
}

void gma_close(struct gma_device *dev)
{
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);

	return RpcCallCompletion(RPC_Svc_gma_close,&p1,NULL);
}

RET_CODE gma_io_control(struct gma_device *dev, UINT32 cmd, UINT32 arg)
{
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(void *), &dev);
	RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &cmd);
	RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &arg);

	switch(cmd)
	{
		case GMA_IO_SET_GLOBAL_ALPHA:
			break;
		case GMA_IO_SET_ENHANCE_PAR:
			RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_GMA_ENH_PARS, sizeof(GMA_ENH_PARS), arg);
			break;
        case GMA_IO_SET_REGION_BY:
            break;
		default:
			return RET_FAILURE;			
	}

	return RpcCallCompletion(RPC_Svc_gma_io_control,&p1,&p2,&p3,NULL);
}

RET_CODE gma_show(struct gma_device *dev, int on)
{
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(void *), &dev);
	RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_INT32, sizeof(int), &on);

	return RpcCallCompletion(RPC_Svc_gma_show,&p1,&p2,NULL);
}

RET_CODE gma_scale(struct gma_device *dev, struct scale_pars *pars)
{
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(void *), &dev);
	RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_GMA_SCALE_PARS, sizeof(GMA_SCALE_PARS), pars);

	return RpcCallCompletion(RPC_Svc_gma_scale,&p1,&p2,NULL);
}

RET_CODE gma_set_pallette(struct gma_device *dev, struct pallette_pars *pars)
{
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(void *), &dev);
	RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_GMA_PAL_PARS, sizeof(GMA_PAL_PARS), pars);

	return RpcCallCompletion(RPC_Svc_gma_set_pallette,&p1,&p2,NULL);
}

RET_CODE gma_get_pallette(struct gma_device *dev, struct pallette_pars *pars)
{
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(void *), &dev);
	RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_GMA_PAL_PARS, sizeof(GMA_PAL_PARS), pars);

	return RpcCallCompletion(RPC_Svc_gma_get_pallette,&p1,&p2,NULL);
}

RET_CODE gma_create_region(struct gma_device *dev, int reg_id, struct region_pars *pars)
{
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(void *), &dev);
	RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_INT32, sizeof(int), &reg_id);
	RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_GMA_REGION_PARS, sizeof(GMA_REGION_PARS), pars);

	return RpcCallCompletion(RPC_Svc_gma_create_region,&p1,&p2,&p3,NULL);
}

void gma_delete_region(struct gma_device *dev, int reg_id)
{
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(void *), &dev);
	RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_INT32, sizeof(int), &reg_id);

	RpcCallCompletion(RPC_Svc_gma_delete_region,&p1,&p2,NULL);
}

RET_CODE gma_get_region_info(struct gma_device *dev, int reg_id, struct region_pars *pars)
{
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(void *), &dev);
	RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_INT32, sizeof(int), &reg_id);
	RPC_PARAM_CREATE(p3, PARAM_OUT, PARAM_GMA_REGION_PARS, sizeof(GMA_REGION_PARS), pars);

	return RpcCallCompletion(RPC_Svc_gma_get_region_info,&p1,&p2,&p3,NULL);
}

RET_CODE gma_set_region_info(struct gma_device *dev, int reg_id, struct region_pars *pars)
{
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(void *), &dev);
	RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_INT32, sizeof(int), &reg_id);
	RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_GMA_REGION_PARS, sizeof(GMA_REGION_PARS), pars);

	return RpcCallCompletion(RPC_Svc_gma_set_region_info,&p1,&p2,&p3,NULL);
}

RET_CODE gma_region_show(struct gma_device *dev, int reg_id, int on)
{
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(void *), &dev);
	RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_INT32, sizeof(int), &reg_id);
	RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_INT32, sizeof(int), &on);

	return RpcCallCompletion(RPC_Svc_gma_region_show,&p1,&p2,&p3,NULL);
}

