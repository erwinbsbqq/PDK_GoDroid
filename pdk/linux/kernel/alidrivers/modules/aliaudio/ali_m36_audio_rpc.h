/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2003 Copyright (C)
*
*	 Driver for ALi M36 audio device driver
*
*    File:    Linux\kernel\drivers\media\dvb\ali_audio
*
*    Description:    This file contains all globe micros and functions declare
*		             of Ali M36 audio device.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	2009-11-30     Eric Li  Ver 0.1    Create file.
*
*  	Copyright 2009 ALi Limited
*  	Copyright (C) 2009 ALi Corp.
*****************************************************************************/
#ifndef _ALI_M36_AUDIO_RPC_H
#define _ALI_M36_AUDIO_RPC_H
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/cdev.h>
//#include <linux/kthread.h>
#include <linux/workqueue.h>
#include <asm/irq.h>

#include <asm/mach-ali/typedef.h>
//#include <asm/mach-ali/m3602.h>
//#include <asm/mach-ali/gpio.h>

//#include <linux/ali_rpc.h>
#include <dvb_audio.h>
#include <rpc_hld/ali_rpc_hld_snd.h>
#include <rpc_hld/ali_rpc_hld_deca.h>

//#include "dvbdev.h"


#if 0
#define PRINTK_INFO(x...) printk(KERN_INFO x)
#else
#define PRINTK_INFO(x...) do{}while(0)
#endif

#define ALI_AUDIO_DEVICE_NAME 	"ali_m36_audio"


struct ali_audio_device
{
	struct cdev cdev;
	struct deca_device *deca_dev;
	struct snd_device *snd_dev;
        int open_count;
        int deca_open_flag;
        int snd_open_flag;
        struct semaphore sem;

	ali_audio_ctrl_blk audio_cb;
	int cb_avail; 	
};

#endif

