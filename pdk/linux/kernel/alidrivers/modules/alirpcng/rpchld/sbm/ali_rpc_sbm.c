/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: ali_rpc_sbm.c
 *
 *  Description: ali share buffer memory for cpu & see access
 *
 *  History:
 *      Date             Author         Version      Comment
 *      ======           ======          =====       =======
 *  1.  2011.08.03       Dylan.Yang     0.1.000     First version Created
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
#include <rpc_hld/ali_rpc_sbm.h>

//#include "../ali_rpc.h"
#include <ali_sbm_common.h>

#include <ali_rpcng.h>

int sbm_see_create(int sbm_idx, int sbm_mode, void *sbm_init)
{
#if !defined(CONFIG_ALI_RPCNG)
	register RET_CODE ret asm("$2");

	jump_to_func(NULL, ali_rpc_call, sbm_idx, (LLD_SBM_MODULE<<24)|(3<<16)|FUNC_SBM_SEE_CREATE, NULL);	

	return ret;
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, sizeof(Int32), &sbm_idx);
	RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_INT32, sizeof(Int32), &sbm_mode);
	RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, sizeof(void *), &sbm_init);

	return RpcCallCompletion(RPC_Svc_SbmSeeCreate,&p1,&p2,&p3,NULL);
#endif
}

int sbm_see_destroy(int sbm_idx, int sbm_mode)
{
#if !defined(CONFIG_ALI_RPCNG)    
	register RET_CODE ret asm("$2");	
	
	jump_to_func(NULL, ali_rpc_call, sbm_idx, (LLD_SBM_MODULE<<24)|(2<<16)|FUNC_SBM_SEE_DESTROY, NULL);  

	return ret;
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, sizeof(Int32), &sbm_idx);
	RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_INT32, sizeof(Int32), &sbm_mode);

	return RpcCallCompletion(RPC_Svc_SbmSeeDestroy,&p1,&p2,NULL);
#endif
}
 
