/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_rpc_hld_base.c
 *  (I)
 *  Description: it is a virtual module to be compitalbe with TDS prj. disable all the
 * 			callback function
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.04.01			Sam			Create
 ****************************************************************************/
/*
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
*/
//#include <ali/basic_types.h>
//#include <ali/hld_dev.h>

#include <linux/ali_rpc.h>

#include <rpc_hld/ali_rpc_hld.h>
#include <ali_minirpc_service.h>
#include <ali_rpcng.h>

//#include "ali_rpc.h"
//#include <ali_rpcng.h>

//static struct remote_hld_device *remote_hld_device_base = NULL;

struct remote_hld_device *remote_hld_device_base = NULL;

/*
 * 	Name		:   hld_dev_get_by_name()
 *	Description	:   Get a device from device link list by device name.
 *	Parameter	:   INT8 *name					: Device name
 *	Return		:	void *						: Device founded
 *
 */
void *hld_dev_get_by_name(INT8 *name)
{
    	struct remote_hld_device *remote_dev;

	/* Remote device */
	if(remote_hld_device_base)
	{
		for (remote_dev = remote_hld_device_base; remote_dev != NULL; 
			remote_dev = (struct remote_hld_device *)remote_dev->next)
		{
			/* Find the device */
			if (strcmp(remote_dev->name, name) == 0)
			{
				return remote_dev->remote;
			}
		}
	}
	
	return NULL;
}

/*
 * 	Name		:   hld_dev_get_by_type()
 *	Description	:   Get a device from device link list by device type.
 *	Parameter	:   INT32 type					: Device type
 *					void *sdev					: Start search nod
 *	Return		:	void *						: Device founded
 *
 */
void *hld_dev_get_by_type(void *sdev, UINT32 type)
{
    	struct remote_hld_device *remote_dev;

	/* Remote device */
	if(remote_hld_device_base)
	{
		for (remote_dev = remote_hld_device_base; remote_dev != NULL; 
			remote_dev = (struct remote_hld_device *)remote_dev->next)
		{
			/* Find the device */
			if ((remote_dev->type & HLD_DEV_TYPE_MASK) == type)
			{
				return remote_dev->remote;
			}
		}
	}
	
	return NULL;
}

/*
 * 	Name		:   hld_dev_get_by_id()
 *	Description	:   Get a device from device link list by device ID.
 *	Parameter	:   UINT32 type					: Device type
 *					UINT16 id					: Device id
 *	Return		:	void *						: Device founded
 *
 */
void *hld_dev_get_by_id(UINT32 type, UINT16 id)
{
    	struct remote_hld_device *remote_dev;

	/* Remote device */
	if(remote_hld_device_base)
	{
		for (remote_dev = remote_hld_device_base; remote_dev != NULL; 
			remote_dev = (struct remote_hld_device *)remote_dev->next)
		{
			if (remote_dev->type == (type | id))
				return remote_dev->remote;
		}
	}
	
	return NULL;
}

INT32 remote_hld_dev_add(struct hld_device *buf, UINT32 dev_addr)
{
	UINT32 count;
	struct remote_hld_device *dev, *dp;

	dev = (struct remote_hld_device *)malloc(sizeof(struct remote_hld_device));

	memcpy(dev, buf, sizeof(struct hld_device));
	dev->remote = (struct hld_device *)(dev_addr);
	dev->next = NULL;
	if(remote_hld_device_base == NULL)
	{
		remote_hld_device_base = dev;
	}
	else
	{
		if (hld_dev_get_by_name(dev->name) != NULL)
		{
			printf("hld_dev_add error: device %s same name!\n", dev->name);
			return -1;
		}

		/* Add this device to device list */
		/* Move to tail */
		for (dp = remote_hld_device_base, count = 0; dp->next != NULL; 
			dp = (struct remote_hld_device *)dp->next)
		{
			count++;
		}

		if (count >= HLD_MAX_DEV_NUMBER)
		{
			printf("too many hld dev \n");
			return -1;
		}

		dp->next = (struct hld_device *)dev;
	}

	/*if(remote_hld_device_base)
	{
		dp = remote_hld_device_base;
		do
		{
			libc_printf("dev name: %s, type: 0x%x, dev: 0x%x, remote: 0x%x\n", dp->name, dp->type, dp, dp->remote);
			dp = dp->next;
		}while (dp != NULL);
	}*/

	printf("%s : ok dev %x remote dev %x dev->type: %x \n", __FUNCTION__, (int)dev, (int)dev_addr,dev->type);
	return 0;
}

INT32 RPC_hld_dev_add_remote(Param *arg1,Param *arg2)
{
    //struct hld_device *p1 = (struct hld_device *)(*(UINT32 *)arg1->pData);
    struct hld_device_rpc *p1 = (struct hld_device_rpc *)arg1->pData;  
    UINT32 p2 = *(UINT32 *)arg2->pData;
    struct hld_device hld_dev;

    hld_dev.next = (struct hld_device *)p1->HLD_DEV;
    hld_dev.type = p1->type;
    memcpy(hld_dev.name,p1->name,16);

    printf("hld_dev->type = 0x%x\n",hld_dev.type);
    
    remote_hld_dev_add(&hld_dev,p2);
}
EXPORT_RPC(RPC_hld_dev_add_remote);

RET_CODE  hld_dev_get_remote(INT8 *name)
{
	int ret = -1;
	struct hld_device_rpc hld_dev_rpc;
	UINT32 dev_addr;
	struct hld_device hld_dev;


	memcpy(hld_dev_rpc.name,name,16);

	MINIRPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_MINIRPC_Hld_device_rpc, sizeof(struct hld_device_rpc), &hld_dev_rpc);
	MINIRPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_MINIRPC_UINT32, sizeof(UINT32), &dev_addr);

	//    RpcCallCompletion(RPC_hld_dev_add_remote,&p1,&p2,NULL);
	ret = RpcCallCompletion(RPC_hld_dev_get_remote,&p1,&p2,NULL);


	hld_dev.next = (struct hld_device *)hld_dev_rpc.HLD_DEV;
	hld_dev.type = hld_dev_rpc.type;
	memcpy(hld_dev.name,hld_dev_rpc.name,16);
	
	printf("remote_hld_dev_add: %s, type:0x%lx   dev_addr:0x%lx \n",hld_dev.name,hld_dev.type,dev_addr);
	remote_hld_dev_add(&hld_dev_rpc,dev_addr);

	return ret;
}

INT32 remote_hld_dev_remove(struct hld_device *dev)
{
	struct remote_hld_device *dp;

	/* If dev in dev_queue, delete it from queue, else free it directly */
	if (remote_hld_device_base != NULL)
	{
		if (strcmp(remote_hld_device_base->name, dev->name) == 0)
		{
			remote_hld_device_base = (struct remote_hld_device *)dev->next;
		} 
		else
		{
			for (dp = remote_hld_device_base; dp->next != NULL; dp = (struct remote_hld_device *)dp->next)
			{
				if (strcmp(dp->next->name, dev->name) == 0)
				{
					dp->next = dev->next;
					break;
				}
			}
		}
	}

	return 0;
}

static INT32 RPC_Svc_remote_hld_dev_remove(Param *arg1)
{
    struct hld_device *dev;

    if(!arg1)
    {
        Log(LOG_ERR,"<%s>Line:%d received NULL argument!\n",__FUNCTION__,__LINE__);
        return -1;
    }
    dev = (INT32*)arg1->pData;

    return remote_hld_dev_remove(dev);
}
EXPORT_RPC(RPC_Svc_remote_hld_dev_remove);

static ali_rpc_cb_routine m_vdec_cb = NULL;
static void hld_vdec_cb(UINT32 uParam)
{
	if(m_vdec_cb)
		m_vdec_cb(0, uParam);
}

static ali_rpc_cb_routine m_vpo_cb = NULL;
static void hld_vpo_cb(UINT32 uParam)
{
	if(m_vpo_cb)
		m_vpo_cb(0, uParam);
}

static ali_rpc_cb_routine m_vpo_hdmi_cb = NULL;
static void hld_vpo_hdmi_cb(UINT32 uParam)
{
	if(m_vpo_hdmi_cb)
		m_vpo_hdmi_cb(0, uParam);
}

static ali_rpc_cb_routine m_snd_hdmi_cb = NULL;	
static void hld_snd_hdmi_cb(UINT32 uParam)
{
	if(m_snd_hdmi_cb)
		m_snd_hdmi_cb(0, uParam);
}

static ali_rpc_cb_routine m_snd_spec_cb = NULL;
static void hld_snd_spec_cb(UINT32 uParam)
{
	if(m_snd_spec_cb)
		m_snd_spec_cb(0, uParam);
}

static ali_rpc_cb_routine m_img_cb = NULL;
static void hld_img_cb(unsigned long type, unsigned long param)
{
	if(m_img_cb)
		m_img_cb(type, param);
}	

static ali_rpc_cb_routine m_vde_cb = NULL;
static void hld_vde_cb(unsigned long type, unsigned long param)
{
	if(m_vde_cb)
		m_vde_cb(type, param);
}

static ali_rpc_cb_routine m_mus_cb = NULL;
static void hld_mus_cb(unsigned long type, unsigned long param)
{
	if(m_mus_cb)
		m_mus_cb(type, param);
}

static INT32 RPC_Svc_hld_vdec_callback(Param *arg1)
{
    UINT32 pParam;

    if(!arg1)
    {
        Log(LOG_ERR,"<%s>Line:%d received NULL argument!\n",__FUNCTION__,__LINE__);
        return -1;
    }
    pParam = *(INT32*)arg1->pData;

    hld_vdec_cb(pParam);
}
EXPORT_RPC(RPC_Svc_hld_vdec_callback);

static INT32 RPC_Svc_hld_vpo_callback(Param *arg1)
{
    UINT32 pParam;

    if(!arg1)
    {
        Log(LOG_ERR,"<%s>Line:%d received NULL argument!\n",__FUNCTION__,__LINE__);
        return -1;
    }
    pParam = *(INT32 *)arg1->pData;

    hld_vpo_cb(pParam);
}
EXPORT_RPC(RPC_Svc_hld_vpo_callback);

static INT32 RPC_Svc_hld_vpo_hdmi_callback(Param *arg1)
{
    UINT32 pParam;

    if(!arg1)
    {
        Log(LOG_ERR,"<%s>Line:%d received NULL argument!\n",__FUNCTION__,__LINE__);
        return -1;
    }
    pParam = (INT32*)arg1->pData;

    hld_vpo_hdmi_cb(pParam);
}
EXPORT_RPC(RPC_Svc_hld_vpo_hdmi_callback);

static INT32 RPC_Svc_hld_snd_hdmi_callback(Param *arg1)
{
    UINT32 pParam;

    if(!arg1)
    {
        Log(LOG_ERR,"<%s>Line:%d received NULL argument!\n",__FUNCTION__,__LINE__);
        return -1;
    }
    pParam = (INT32*)arg1->pData;

    hld_snd_hdmi_cb(pParam);
}
EXPORT_RPC(RPC_Svc_hld_snd_hdmi_callback);

static INT32 RPC_Svc_hld_img_callback(Param *arg1,Param *arg2)
{
    unsigned long ptype;
    unsigned long pparam;

    if(!arg1)
    {
        Log(LOG_ERR,"<%s>Line:%d received NULL argument!\n",__FUNCTION__,__LINE__);
        return -1;
    }
    ptype = *(INT32*)arg1->pData;

    if(!arg2)
    {
        Log(LOG_ERR,"<%s>Line:%d received NULL argument!\n",__FUNCTION__,__LINE__);
        return -1;
    }
    pparam = *(INT32*)arg2->pData;

    hld_img_cb(ptype, pparam);
}
EXPORT_RPC(RPC_Svc_hld_img_callback);

static INT32 RPC_Svc_hld_vde_callback(Param *arg1,Param *arg2)
{
    unsigned long ptype;
    unsigned long pparam;

    if(!arg1)
    {
        Log(LOG_ERR,"<%s>Line:%d received NULL argument!\n",__FUNCTION__,__LINE__);
        return -1;
    }
    ptype = *(INT32*)arg1->pData;

    if(!arg2)
    {
        Log(LOG_ERR,"<%s>Line:%d received NULL argument!\n",__FUNCTION__,__LINE__);
        return -1;
    }
    pparam = *(INT32*)arg2->pData;

    hld_vde_cb(ptype, pparam);
}
EXPORT_RPC(RPC_Svc_hld_vde_callback);

static INT32 RPC_Svc_hld_music_callback(Param *arg1,Param *arg2)
{
    unsigned long ptype;
    unsigned long pparam;

    if(!arg1)
    {
        Log(LOG_ERR,"<%s>Line:%d received NULL argument!\n",__FUNCTION__,__LINE__);
        return -1;
    }
    ptype = *(unsigned long*)arg1->pData;

    if(!arg2)
    {
        Log(LOG_ERR,"<%s>Line:%d received NULL argument!\n",__FUNCTION__,__LINE__);
        return -1;
    }
    pparam = *(unsigned long*)arg2->pData;

    hld_mus_cb(ptype, pparam);
}
EXPORT_RPC(RPC_Svc_hld_music_callback);

void ali_rpc_register_callback(enum ALI_RPC_CB_TYPE type, void *cb_func)
{
	switch(type)
	{
		case ALI_RPC_CB_VDEC:
			m_vdec_cb = (ali_rpc_cb_routine)cb_func;
			break;
		case ALI_RPC_CB_VPO:
			m_vpo_cb = (ali_rpc_cb_routine)cb_func;
			break;			
		case ALI_RPC_CB_VPO_HDMI:
			m_vpo_hdmi_cb = (ali_rpc_cb_routine)cb_func;
			break;
		case ALI_RPC_CB_SND_HDMI:
			m_snd_hdmi_cb = (ali_rpc_cb_routine)cb_func;
			break;
		case ALI_RPC_CB_SNC_SPC:
			m_snd_spec_cb = (ali_rpc_cb_routine)cb_func;
			break;
		case ALI_RPC_CB_IMG:
			m_img_cb = (ali_rpc_cb_routine)cb_func;
			break;			
		case ALI_RPC_CB_VDE:
			m_vde_cb = (ali_rpc_cb_routine)cb_func;			
			break;	
		case ALI_RPC_CB_MUS:
			m_mus_cb = (ali_rpc_cb_routine)cb_func;			
			break;			
		default:
			break;
	}
}

void hld_dev_memcpy(void *dest, const void *src, unsigned int len)
{
    MINIRPC_PARAM_CREATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, 4, &dest);
    MINIRPC_PARAM_CREATE(p2, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(UINT32), &src);
    MINIRPC_PARAM_CREATE(p3, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(UINT32), &len);
	
    RpcCallCompletion(RPC_remote_hld_dev_memcpy,&p1,&p2,&p3,NULL);
}

void hld_dev_see_init(void *addr)
{
   MINIRPC_PARAM_CREATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, 4, &addr);
    RpcCallCompletion(RPC_remote_hld_see_init,&p1,NULL);

}

UINT32 see_standby(UINT32 status)
{
	MINIRPC_PARAM_CREATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(UINT32), &status);
	return RpcCallCompletion(RPC_Svc_see_standby,&p1,NULL);
}


UINT32 hld_get_see_version(UINT8 *dest)
{

	INT32 ret;
	#define SEE_VER_MAX_LEN 128
	MINIRPC_PARAM_CREATE(p1, PARAM_IN, PARAM_MINIRPC_OPAQUE, (SEE_VER_MAX_LEN-1), dest);

	ret = RpcCallCompletion(RPC_Svc_GetSeeVer,&p1,NULL);
    	return(ret);
}

void hld_disable_see_printf(unsigned long disable)
{
	MINIRPC_PARAM_CREATE(p1, PARAM_IN, PARAM_MINIRPC_ULONG, sizeof(unsigned long), &disable);

	RpcCallCompletion(RPC_Svc_disable_see_printf,&p1,NULL);
}

void hld_hit_see_heart(void)
{
	RpcCallCompletion(RPC_Svc_HitSeeHeart,NULL);
}

void hld_enable_see_exception(void)
{
	//jump_to_func(NULL, ali_rpc_call, NULL, (HLD_BASE_MODULE<<24)|(0<<16)|FUNC_HLD_ENABLE_SEE_EXCEPTION, NULL);
	RpcCallCompletion(RPC_Svc_EnableSeeException,NULL);
}


/*
* main cpu trig the see
*/
UINT32 hld_main_notify_see_trig(UINT32 param)
{
	MINIRPC_PARAM_CREATE(p1, PARAM_IN, PARAM_MINIRPC_ULONG, sizeof(unsigned long), &param);

	RpcCallCompletion(RPC_hld_main_notify_see_trig,&p1,NULL);
//    jump_to_func(NULL, ali_rpc_call, param, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_NOTIFY_SEE_TRIG, NULL);
}

UINT32 remote_hld_notify_see_trig(UINT32 param)
{
    return RET_FAILURE;
}
