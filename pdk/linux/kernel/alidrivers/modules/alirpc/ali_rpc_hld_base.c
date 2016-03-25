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
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_hld.h>

#include "ali_rpc.h"

static struct remote_hld_device *remote_hld_device_base = NULL;

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

static INT32 remote_hld_dev_add(struct hld_device *buf, UINT32 dev_addr)
{
	UINT32 count;
	struct remote_hld_device *dev, *dp;

	dev = (struct remote_hld_device *)kmalloc(sizeof(struct remote_hld_device), GFP_KERNEL);

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
			RPC_PRF("hld_dev_add error: device %s same name!\n", dev->name);
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
			RPC_PRF("too many hld dev \n");
			return -1;
		}

		dp->next = (struct remote_hld_device *)dev;
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

	RPC_PRF("%s : ok dev %x remote dev %x\n", __FUNCTION__, (int)dev, (int)dev_addr);
	return 0;
}

static INT32 remote_hld_dev_remove(struct hld_device *dev)
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
					dp->next = (struct remote_hld_device *)dev->next;
					break;
				}
			}
		}
	}

	return 0;
}

static void remote_hld_dev_memcpy(void *dest, const void *src, unsigned int len)
{
	// keep it NULL. it is only active in the SEE cpu
	do{}while(0);
}

static void remote_hld_see_init(void *addr)
{
    // keep it NULL. it is only active in the SEE cpu
	do{}while(0);
}

enum HLD_DEV_FUNC{
    FUNC_HLD_DEV_ADD = 0,   
    FUNC_HLD_DEV_REMOVE,   
    FUNC_HLD_MEM_CPY,
    FUNC_SEE_STANDBY,
    FUNC_HLD_GET_SEE_VER,
    FUNC_HLD_DISABLE_SEE_PRINTF,
    FUNC_HLD_HIT_SEE_HEART,
    FUNC_HLD_ENABLE_SEE_EXCEPTION,
    FUNC_HLD_SHOW_SEE_PLUGIN_INFO,
    FUNC_HLD_SEE_INIT,
    FUNC_HLD_VDEC_CB,
    FUNC_HLD_VPO_CB,
    FUNC_HLD_VPO_HDMI_CB,
    FUNC_HLD_SND_HDMI_CB,
    FUNC_HLD_SND_SPEC_CB,
    FUNC_HLD_IMG_CB,
    FUNC_HLD_VDE_CB,
    FUNC_HLD_MUS_CB,
};

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

static void empty(void)
{
	do{}while(0);
}

static UINT32 hld_dev_entry[] = 
{
	(UINT32)remote_hld_dev_add,
	(UINT32)remote_hld_dev_remove,
	(UINT32)remote_hld_dev_memcpy,
	(UINT32)empty,
	(UINT32)empty,
	(UINT32)empty,
	(UINT32)empty,
	(UINT32)empty,
	(UINT32)empty,
       (UINT32)remote_hld_see_init,
	(UINT32)hld_vdec_cb,
	(UINT32)hld_vpo_cb,
	(UINT32)hld_vpo_hdmi_cb,
	(UINT32)hld_snd_hdmi_cb,
	(UINT32)hld_snd_spec_cb,	
	(UINT32)hld_img_cb,
	(UINT32)hld_vde_cb,
	(UINT32)hld_mus_cb,	
};

static UINT32 desc_hld_dev[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(struct hld_device)),
	1, DESC_P_PARA(0, 0, 0), 
	//desc of pointer ret
	0,                          
	0,
};

void ali_rpc_hld_base_callee(UINT8 *msg)
{
    	ali_rpc_ret((unsigned long)hld_dev_entry, msg);
}

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

INT32 hld_dev_add_remote(struct hld_device *dev, UINT32 dev_addr)
{
    
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_BASE_MODULE<<24)|(2<<16)|FUNC_HLD_DEV_ADD, desc_hld_dev);
	
}

INT32 hld_dev_remove_remote(struct hld_device *dev)
{

	jump_to_func(NULL, ali_rpc_call, dev, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_DEV_REMOVE, desc_hld_dev);

}

void hld_dev_memcpy_ex(void *dest, const void *src, unsigned int len)
{
    	jump_to_func(NULL, ali_rpc_call, dest, (HLD_BASE_MODULE<<24)|(3<<16)|FUNC_HLD_MEM_CPY, NULL);
}

void hld_dev_memcpy(void *dest, const void *src, unsigned int len)
{
    dma_cache_wback((unsigned long)src, len);
    if(*(unsigned char *)(src+len-1) != *(volatile unsigned char*)((UINT32)(src+len-1)|0xa0000000))
    {
        //make sure data is flushed into cache before send to SEE
        asm volatile(".word 0x7000003f; nop; nop");
    }
    hld_dev_memcpy_ex(dest, src, len);
}


void hld_dev_see_init(void *addr)
{
	jump_to_func(NULL, ali_rpc_call, addr, (HLD_BASE_MODULE << 24) | (1 << 16) | FUNC_HLD_SEE_INIT, NULL);
}

UINT32 see_standby(UINT32 status)
{

    jump_to_func(NULL, ali_rpc_call, status, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_SEE_STANDBY, NULL);

}


UINT32 hld_get_see_version(UINT8 *dest)
{
	
    	jump_to_func(NULL, ali_rpc_call, dest, (HLD_BASE_MODULE << 24) | (1<< 16) | FUNC_HLD_GET_SEE_VER, 
			NULL);

}

void hld_disable_see_printf(unsigned long disable)
{
	jump_to_func(NULL, ali_rpc_call, disable, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_DISABLE_SEE_PRINTF, NULL);
}

void hld_hit_see_heart(void)
{
	jump_to_func(NULL, ali_rpc_call, NULL, (HLD_BASE_MODULE<<24)|(0<<16)|FUNC_HLD_HIT_SEE_HEART, NULL);
}

void hld_enable_see_exception(void)
{
	jump_to_func(NULL, ali_rpc_call, NULL, (HLD_BASE_MODULE<<24)|(0<<16)|FUNC_HLD_ENABLE_SEE_EXCEPTION, NULL);
}

void hld_show_see_plugin_info(void)
{
	jump_to_func(NULL, ali_rpc_call, NULL, (HLD_BASE_MODULE<<24)|(1<<16)|FUNC_HLD_SHOW_SEE_PLUGIN_INFO, 
		NULL);

}
