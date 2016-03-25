/*
 *  Copyright (c) 2012, ALi Corporation.
 *  All rights reserved.
 *
 *      File:           ali_hwdma.h
 *      Version:        1.0
 *      Date:           2012-03-09
 *      Description:    ali bdma module operation define header file.
 */

#ifndef _LINUX_ALI_HWDMA_H
#define _LINUX_ALI_HWDMA_H

#include <linux/ali_bdma.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>

#include <linux/alihwdma-ioctl.h>
//#include "alihwdma-ioctl.h"


#define DEBUG_LEVEL 1
#define MAX_PDT_NUM			0x0000000A // 10 process is premitted
#define ISR_DEBUG 1 // debug level for printk in ISR


#define ALIDMA_PRINTK(dbg_level, fmt, args...)              \
    if(dbg_level < DEBUG_LEVEL){                             \
    	printk("ALi DMA : <%s> "fmt , __FUNCTION__, ##args);}

#define ALIDMA_ERR_PRINTK(fmt, args...)                     \
        {printk("ALi HW DMA error :\n"); printk(" <%s> "fmt , __FUNCTION__,##args);}

struct cmdQ_buf{
	BDMAuint             vir_adr;
	BDMAuint             phy_adr;
};


#define CMDQ_BANK_SWITCH \
	struct cmdQ_buf cmdQ;\
	if (cmdQ_cnt==0){\
		cmdQ = cmdQ_2bank[0];\
		cmdQ_cnt=1;\
	}else if(cmdQ_cnt==1){\
		cmdQ = cmdQ_2bank[1];\
		cmdQ_cnt=0;\
	}
/*#define CMDQ_BANK_SWITCH \
	struct cmdQ_buf cmdQ;\
	if (cmdQ_cnt==0){\
		cmdQ = (BDMAint*)cmdQ_2bank[0];\
		cmdQ_cnt =1;\
	}else if (cmdQ_cnt==1){\
		cmdQ = (BDMAint*)cmdQ_2bank[1];\
		cmdQ_cnt=2;\
	}else if (cmdQ_cnt==2){\
		cmdQ = (BDMAint*)cmdQ_2bank[2];\
		cmdQ_cnt=0;\
	}*/


struct ali_dma_dev {
    struct cdev         cdev;
    struct semaphore    hw_sema;
};

struct ali_dma_directoryEntry {
    BDMAuint pde;
	bool bdma_block_mode;	
};

struct ali_bdma_ops {
    BDMAint (*address_process)(struct ali_dma_directoryEntry* entry, BDMAuint virtual_address, BDMAuint size);
    BDMAint (*hw_start)(struct ali_dma_dev *, struct ali_dma_directoryEntry* , ali_hwdma_job* );
    void    (*clean)(struct ali_dma_directoryEntry* entry , ali_hwdma_job* job);
    BDMAint (*hw_start_2D)(struct ali_dma_dev *, struct ali_dma_directoryEntry* , ali_hwdma_2Djob* );

};

void bdma_ops_mode_setup(struct ali_bdma_ops* bdma_ops, bool UsingCmdQ, bool UsingGenTable);

#endif
