/*******************************************************************************

File name   : ali_rpc_venc.c

Description : Video driver stack LINUX platform OS call-SEE driver source file

Author      : Vic Zhang <Vic.Zhang@Alitech.com>

Create date : Oct 23, 2013

COPYRIGHT (C) ALi Corporation 2013

Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 23, 2013                   Created               V0.1             Vic.Zhang

*******************************************************************************/

#include <linux/version.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
//#include <linux/ali_image.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/div64.h>
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
#include <linux/ali_rpc.h>

#include <rpc_hld/ali_rpc_venc.h>

#include <ali_rpcng.h>



int venc_remote_init()
{
    Int32 ret;

    ret = RpcCallCompletion(RPC_venc_init, NULL);
    return ret;
}

int venc_remote_start(struct videnc_see_config *cfg)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_VidencSeeConfig, sizeof(VidencSeeConfig_t), cfg);
    ret = RpcCallCompletion(RPC_venc_start, &p1, NULL);

    return ret;
}

int venc_remote_release()
{
    Int32 ret;

    ret = RpcCallCompletion(RPC_venc_release, NULL);
    return ret;
}

int venc_remote_stop()
{
    Int32 ret;

    ret = RpcCallCompletion(RPC_venc_stop, NULL);
    return ret;
}

int venc_remote_encode(struct videnc_trigger_para *cfg)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_VidencTriggerPara, sizeof(VidencTriggerPara_t), cfg);
    ret = RpcCallCompletion(RPC_venc_encode, &p1, NULL);

    return ret;
}
