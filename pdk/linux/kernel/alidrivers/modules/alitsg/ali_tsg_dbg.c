/******************************************************************************
*
*       ALI IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR ALI DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, ALI IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  ALI EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2005-2014 ALI Inc.
*       All rights reserved.
*
******************************************************************************/

/*****************************************************************************/
/**
 * MODIFICATION HISTORY:
 *
 * Ver   Who          Date     Changes
 * ----- ------------ -------- -----------------------------------------------
 * 1.00a Jingang Chu  06/11/14 First release
 *
 *****************************************************************************/

 /***************************** Include Files *********************************/
 
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/printk.h> 
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/slab.h> 
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/debugfs.h>
#include <asm/uaccess.h>
#include <ali_tsg_common.h>
#include <ali_reg.h>
#include <asm/io.h>
#include "ali_tsg.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

static struct dentry *ali_tsg_dbg_debugfs_root;
static __u32 ali_tsg_dbg_pid_out = 0xFFFF;
static __u32 ali_tsg_dbg_last_conti_out = 0;
static __u32 ali_tsg_dbg_pid_in = 0xFFFF;
static __u32 ali_tsg_dbg_last_conti_in = 0;
static __u32 ali_tsg_dbg_show_en = 0;
static __u32 ali_tsg_dbg_xfer_irq_cnt = 0;
static __u32 ali_tsg_dbg_syncerr_irq_cnt = 0;

/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
__s32 ali_tsg_dbg_chk_conti
(
   __u32   pid,
    __u8  *data_addr,
    __u32  len,
    __u8  *note,
    __u32 *last_conti
)
{
    __u32  cur_pid;
    __u8  *pkt_addr;
    __u32  cur_conti_cnt;
    __u32  exp_conti_cnt;
    __u32  pkt_idx;

    pkt_idx = 0;
    
    for (pkt_addr = data_addr; pkt_addr < data_addr + len; pkt_addr += 188)
    {    
        if (0x47 != pkt_addr[0])
        {
            printk("%s,TS sync error,byte 0:0x%x,pkt_idx:%d\n",
                   note, pkt_addr[0], pkt_idx);
        }
        
        cur_pid = ((pkt_addr[1] & 0x1F) << 8) | pkt_addr[2];
        
        if (cur_pid == pid)
        {      
            cur_conti_cnt = pkt_addr[3] & 0x0F;

            exp_conti_cnt = (*last_conti + 1) & 0x0F;
            if ((exp_conti_cnt != cur_conti_cnt) &&
                (*last_conti != cur_conti_cnt))
            {
                printk("%s,TS lost,PID:%d,exp:%d,cur:%d,pkt_idx:%d\n",
                       note, cur_pid, exp_conti_cnt, cur_conti_cnt, pkt_idx);
            }
    
            *last_conti = cur_conti_cnt;
        }

        pkt_idx++;
    }
    
    return(0);
}

/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
__s32 ali_tsg_dbg_chk_conti_out
(
    __u8  *data_addr,
    __u32  len,
    __u8  *note
)
{
    if (ali_tsg_dbg_pid_out >= 0x1FFF)
    {
        ali_tsg_dbg_last_conti_out = 0;
        
        return(-__LINE__);
    }
    
    ali_tsg_dbg_chk_conti(ali_tsg_dbg_pid_out, data_addr, len, note,
		                  &ali_tsg_dbg_last_conti_out);

    return(0);
}

/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
__s32 ali_tsg_dbg_chk_conti_in
(
    const char __user *buf,
    size_t             count,
    __u8              *note
)
{
    __u8 *data_addr;

    if (ali_tsg_dbg_pid_in >= 0x1FFF)
    {
        ali_tsg_dbg_last_conti_in = 0;
    
        return(-__LINE__);
    }
    
    data_addr = kmalloc(count, GFP_KERNEL);

    if(NULL == data_addr)
    {
        printk("%s,%d,kmalloc(%d) failed, could not do continuty checking\n",
               __FUNCTION__, __LINE__, count);
    
        return(-__LINE__);
    }   

    if (copy_from_user(data_addr, buf, count))
    {    
        kfree(data_addr);
        
        printk("%s,%d,copy_from_user(%d) failed, could not do continuty checking\n",
               __FUNCTION__, __LINE__, count); 
        
        return(-__LINE__);
    }   

    ali_tsg_dbg_chk_conti(ali_tsg_dbg_pid_in, data_addr, count, note,
		                  &ali_tsg_dbg_last_conti_in);

    kfree(data_addr);   

    return(0);  
}

/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
__s32 ali_tsg_dbg_show(char *fmt, ...)
{
    va_list args;
    __s32 r;

    if (1 != ali_tsg_dbg_show_en)
    {
        return(-__LINE__);

    }
 
    va_start(args, fmt);
    r = vprintk(fmt, args);
    va_end(args);

    return(r);
}

/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
__s32 ali_tsg_dbg_syncerr_irq_inc
(
    void
)
{
    ali_tsg_dbg_syncerr_irq_cnt++;

    return(ali_tsg_dbg_syncerr_irq_cnt);
}

/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
__s32 ali_tsg_dbg_xfer_irq_cnt_inc
(
    void
)
{
    ali_tsg_dbg_xfer_irq_cnt++;

    return(ali_tsg_dbg_xfer_irq_cnt);
}

/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
__s32 ali_tsg_dbg_exit
(
    void 
)
{
    debugfs_remove_recursive(ali_tsg_dbg_debugfs_root);
    ali_tsg_dbg_debugfs_root = NULL;
    ali_tsg_dbg_pid_out = 0xFFFF;
    ali_tsg_dbg_pid_in = 0xFFFF;
    ali_tsg_dbg_last_conti_out = 0;
    ali_tsg_dbg_last_conti_in = 0;
    ali_tsg_dbg_show_en = 0;
    ali_tsg_dbg_xfer_irq_cnt = 0;
    ali_tsg_dbg_syncerr_irq_cnt = 0;
    
    return(0);
}

/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
__s32 ali_tsg_dbg_init
(
    void 
)
{
    struct dentry *fs_entry;

    ali_tsg_dbg_show("%s,%d,Go\n", __FUNCTION__, __LINE__);
    
    ali_tsg_dbg_debugfs_root = debugfs_create_dir("ali_tsg", NULL);
    
    if (!ali_tsg_dbg_debugfs_root)
    {
        ali_tsg_dbg_show("%s,%d\n", __FUNCTION__, __LINE__);
        return(-ENOENT);
    }

    /* For TSG input pid continuty counter checking.
	*/
    fs_entry = debugfs_create_u32("dbg_pid_out", 0644, ali_tsg_dbg_debugfs_root,
                                  &ali_tsg_dbg_pid_out);
    if (!fs_entry)
    {
        ali_tsg_dbg_show("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }

    /* For TSG output pid continuty counter checking.
	*/
    fs_entry = debugfs_create_u32("dbg_pid_in", 0644, ali_tsg_dbg_debugfs_root,
                                  &ali_tsg_dbg_pid_in);
    if (!fs_entry)
    {
        ali_tsg_dbg_show("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }

    /* For TSG debug show enable/disable.
	*/
    fs_entry = debugfs_create_u32("dbg_show_en", 0644, ali_tsg_dbg_debugfs_root,
                                  &ali_tsg_dbg_show_en);
    if (!fs_entry)
    {
        ali_tsg_dbg_show("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }

    fs_entry = debugfs_create_u32("dbg_xfer_irq_cnt", 0644,
                                  ali_tsg_dbg_debugfs_root, &ali_tsg_dbg_xfer_irq_cnt);
    if (!fs_entry)
    {
        ali_tsg_dbg_show("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }

    fs_entry = debugfs_create_u32("dbg_syncerr_irq_cnt", 0644, 
                                  ali_tsg_dbg_debugfs_root,
                                  &ali_tsg_dbg_syncerr_irq_cnt);
    if (!fs_entry)
    {
        ali_tsg_dbg_show("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }   
    
    ali_tsg_dbg_show("%s,%d,succeed.\n", __FUNCTION__, __LINE__);
    return(0);

Fail:
    ali_tsg_dbg_show("%s,%d,failed\n", __FUNCTION__, __LINE__);
    ali_tsg_dbg_exit();
    return(-ENOENT);    
}





