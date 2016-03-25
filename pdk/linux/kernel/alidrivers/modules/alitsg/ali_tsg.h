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

#ifndef __ALI_M36_TSG_H
#define __ALI_M36_TSG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/* Debug print, not needed in most cases.
 */
#if 0
#define ALI_TSG_DEBUG printk
#else
#define ALI_TSG_DEBUG(...)
#endif

/* Physical to virtual address and vise visa.
 */
#define ALI_PHYS2VIRT(x) (x - ALI_REGS_PHYS_BASE + ALI_REGS_VIRT_BASE)
#define ALI_VIRT2PHYS(x) (x - ALI_REGS_VIRT_BASE + ALI_REGS_PHYS_BASE)


/* HW register base adddress. 
 */
#define ALI_TSG_REG_BASE    0x18026000


/* HW registers' offset to ALI_TSG_REG_BASE
 */
#define ALI_TSG_REG_HW_BUF_ADDR      0x00
#define ALI_TSG_REG_HW_BUF_LEN       0x08
#define ALI_TSG_REG_DMA_CTRL         0x0C
#define ALI_TSG_REG_DMA_STAT         0x0E
#define ALI_TSG_REG_DMA_CUR_ADDR     0x10
#define ALI_TSG_REG_SYNC_BYTE        0x20
#define ALI_TSG_REG_GLB_CTRL         0x24
#define ALI_TSG_REG_INT_CTRL         0x28
#define ALI_TSG_REG_INT_STAT         0x29
#define ALI_TSG_REG_DUMMY_INSERT_CFG 0x30

/* Device name length.
 */
#define ALI_TSG_NAME_MAX_LEN 16


/* ALI TSG has a HW restriction:
 * 1, data must be xfered in unit of 16 bytes(128 bits for C3921);
 * 2, data better be xfered in unit of 188 bytes(TS packet).
 * Hence, the xfer unit must be rounded in unit if 752 byte
 * (Least Common Multiple of 128 and 16).
 */
#define ALI_TSG_XFER_UNIT_LEN 752

/* Round to a multiple of 47K to best fit ALI_PVR VOBU size, and also
 * be a multiple of ALI_TSG_XFER_UNIT_LEN to satisfy data maintaining
 * algorithm of TSG driver.
 */
#define ALI_TSG_HW_BUF_UNIT_LEN (47 * 1024)


/* Default TSG HW clock setting.
 */
#define ALI_TSG_DEFAULT_CLK_SETTING 0x18

/* TSG device status.
 */
enum ALI_TSG_STATUS
{
    ALI_TSG_STATUS_IDLE = 0,
    ALI_TSG_STATUS_RUN
};

/**************************** Type Definitions *******************************/

/* TSG driver internal representaion.
 */
struct ali_tsg_dev
{
    __s8 name[ALI_TSG_NAME_MAX_LEN];
    enum ALI_TSG_STATUS status;

	/* HW register base address.
	*/
    void __iomem *base;

	/* Linux char device ID.
	*/
    dev_t dev_id;
	
    /* Linux char device for TSG.
    */
    struct cdev cdev;
	
    /* Protect againest concurrent device access by user space.
     * (Race condition for mulitple userland process accessing same TSG
     *  device node).
     */
    struct mutex io_mutex; 

    /* HW clock settings.
    */
    __u8 hw_clk;

    /* HW irq number.
    */	
    __u32 irq_num;

    /* HW data buffer, must be virtual address which does not use cache.
    */
    __u8 *buf;
    __u32 buf_len;
    __u32 buf_rd;
    __u32 buf_wr;
};

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/*
 * Linux interface functions implemented in ali_tsg_debug.c
 */
__s32 ali_tsg_close(struct inode * inode, struct file * file);
__s32 ali_tsg_open(struct inode * inode, struct file * file);

ssize_t ali_tsg_usr_wr(struct file * file, const char __user * buf, 
	                   size_t count, loff_t * ppos);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long ali_tsg_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#else
long ali_tsg_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#endif

/*
 * Debug functions implemented in ali_tsg_debug.c
 */
__s32 ali_tsg_dbg_chk_conti_out(__u8 *addr, __u32 len, __u8* note);
__s32 ali_tsg_dbg_chk_conti_in(const char __user *buf, size_t count, __u8* note);
__s32 ali_tsg_dbg_show(char *fmt, ...);
__s32 ali_tsg_dbg_syncerr_irq_inc(void);
__s32 ali_tsg_dbg_xfer_irq_cnt_inc(void);
__s32 ali_tsg_dbg_init(void);
__s32 ali_tsg_dbg_exit(void);

#ifdef __cplusplus
}
#endif

#endif

