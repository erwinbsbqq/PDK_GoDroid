/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: shm_comm.c
 *
 *  Description: shm_comm device low level implementation file.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.02.01      David.L         0.1.000         Initial
 *  2.  2013/05/25      Tony.Zh         0.2.000         Bug fix 
 ****************************************************************************/

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/pgtable.h>
#include <linux/semaphore.h>

#include "mcomm.h"
#include "mcomm_dev.h"
#include "shm_comm_hw.h"
#include <ali_shm.h>

int g_sendflag = 0; /*Added by tony*/

/**
 * David - 
 * This file simulates Marvell's arch/arm/mach-feroceon-mv78xx0, implementing
 * the infrastructure mailbox hardware context exchanging. Therefore, this
 * module notifies and acks the handshake companying with the mailbox context,
 * which is different from the original.
 */

static struct ali_mcomm_struct {
	unsigned int localId;
	unsigned int remoteId;
    spinlock_t   lock;
    struct semaphore syncSem;
    MBX_HAL      mbx;
} ali_mcomm_data;

static pgprot_t mcomm_ali_mmap_pgprot(struct vm_area_struct *vma)
{
	return pgprot_noncached(vma->vm_page_prot);
}

static void __iomem *mcomm_ali_ioremap(unsigned long phys_addr, size_t size)
{
	return ioremap_nocache(phys_addr, size);
}

static void mcomm_ali_send(u32 core_id, u32 ctxt)
{
    unsigned long flags;
    struct timeval start, end;
    int interval;
    int ret;

    switch (core_id) {
    case SEE_CPU_ID:

//	printk("<MAIN>mbx snd  mailbox0 pre!\n");
	do_gettimeofday(&start);

        /*Changed from spin_lock to spin_lock_irqsave by tony*/
        spin_lock_irqsave(&ali_mcomm_data.lock, flags);

        /* Main send context by mailbox0. */
	g_sendflag = 1;
        ali_mcomm_data.mbx.tx(MBX0, ctxt);

        spin_unlock_irqrestore(&ali_mcomm_data.lock, flags);


	ret = down_timeout(&ali_mcomm_data.syncSem, msecs_to_jiffies(10000)); /*Wait 10000 ms*/
	if (ret)
		printk("down_timeout occurs: %ld\n", msecs_to_jiffies(10000));
	g_sendflag = 0;

	do_gettimeofday(&end);
        interval = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
	if(interval>9000000)
	{
		printk("send mailbox0 time cost warning! interval:%d\n", interval);
		//BUG_ON(1);
	}

        break;

    default:
        break;
    }
}

static void mcomm_ali_ack(u32 core_id, u32 ctxt)
{
    unsigned long flags;

    switch (core_id) {
    case SEE_CPU_ID:
	/*Changed from spin_lock to spin_lock_irqsave by tony*/
	spin_lock_irqsave(&ali_mcomm_data.lock, flags);

        /* Ack the peer by mailbox1. */
        ali_mcomm_data.mbx.tx(MBX1, ctxt);

	spin_unlock_irqrestore(&ali_mcomm_data.lock, flags);
        break;

    default:
        break;
    }
}

/**
 * This function called in a irq routine, therefor have to use spin_lock_irqsave
 * and spin_unlock_irqrestore, since there is no way to wake up a tasklet with
 * another parameter.
 */
static u32 mcomm_ali_recv(u32 mbx_index)
{
    u32 ctxt;
    unsigned long flags;
    
    spin_lock_irqsave(&ali_mcomm_data.lock, flags);
    ctxt = ali_mcomm_data.mbx.rx(mbx_index);
    spin_unlock_irqrestore(&ali_mcomm_data.lock, flags);
    return ctxt;
}

static void mcomm_ali_sync(void)
{
   if(g_sendflag)
    	up(&ali_mcomm_data.syncSem);
}

static unsigned long mcomm_ali_cpuid(u32 remote)
{
    if (remote)
        return ali_mcomm_data.remoteId;

	return ali_mcomm_data.localId;
}

static struct mcomm_platform_ops mcomm_ali_ops = {
	.mmap_pgprot = mcomm_ali_mmap_pgprot,
	.map = mcomm_ali_ioremap,
	.send = mcomm_ali_send,
    .ack = mcomm_ali_ack,
	.recv = mcomm_ali_recv,
    .sync = mcomm_ali_sync,
	.cpuid = mcomm_ali_cpuid,
};


static void mcomm_ali_hwinit(void)
{
    unsigned long flags;
    
    spin_lock_irqsave(&ali_mcomm_data.lock, flags);

    ali_mcomm_data.mbx.init();

    spin_unlock_irqrestore(&ali_mcomm_data.lock, flags);
}

void rpc_ali_modinit(void);
static int __devinit mcomm_probe(struct platform_device *pdev)
{
	struct resource *mem;
	struct resource *irq;
	int ret;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem)
		return -EINVAL;

	irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!irq)
		return -EINVAL;

	mcomm_ali_hwinit();

	//return mcomm_new_region(&pdev->dev, mem, irq);
	ret = mcomm_new_region(&pdev->dev, mem, irq);

#if defined(__MCAPI_TEST__)

    rc = PR_ThreadCreate(&mcapi_task_id, MCAPI_Hi, 0, 0, 0, NULL);
    if (rc) {
        PR_LOG("Thread create MCAPI_Hi error.\n");
    }
#else
    rpc_ali_modinit();

#endif /* __MCAPI_TEST__ */

	return ret;
}

static void mcomm_ali_hwuninit(void)
{
    unsigned long flags;
    
    spin_lock_irqsave(&ali_mcomm_data.lock, flags);

    ali_mcomm_data.mbx.deinit();

    spin_unlock_irqrestore(&ali_mcomm_data.lock, flags);
}

static int mcomm_remove(struct platform_device *pdev)
{
	mcomm_remove_region(&pdev->dev);
	mcomm_ali_hwuninit();

	return 0;
}

static struct platform_driver mcomm_driver = {
	.probe = mcomm_probe,
	.remove = mcomm_remove,
	.driver = {
		   .name = "mcomm",
	}
};

extern unsigned long __G_ALI_MM_PRIVATE_AREA_SIZE;
extern unsigned long __G_ALI_MM_PRIVATE_AREA_START_ADDR;

extern unsigned long __G_ALI_MM_SHARED_MEM_SIZE;
extern unsigned long __G_ALI_MM_SHARED_MEM_START_ADDR;

extern unsigned long __G_ALI_MM_MCAPI_MEM_SIZE;
extern unsigned long __G_ALI_MM_MCAPI_MEM_START_ADDR;

#define SEE_RUN_ADDR (__G_ALI_MM_PRIVATE_AREA_START_ADDR + 0x200)

void  g_see_boot()
{
	u32 i=0;
	u32 addr = SEE_RUN_ADDR;
	
	addr = (addr & 0x0FFFFFFF) | 0xA0000000;
	printk("[%s]--%d  addr=0x%08x\n",__func__,__LINE__,addr);
	*(volatile u32 *)__REGALIRAW(0x18000220) &= ~0x2;
	*(volatile u32 *)__REGALIRAW(0x18000200) = addr; //init see
	for(i=0;i<900000;i++)
	{;}
	*(volatile u32 *)__REGALIRAW(0x18000220) |= 0x2; 
}

#define DRAM_SPLIT_CTRL_BASE  __REGALIRAW(0x18041000)
#define PVT_S_ADDR 0x10
#define PVT_E_ADDR 0x14
#define SHR_S_ADDR 0x18
#define SHR_E_ADDR 0x1c

#define SET_DWORD(addr, d)         (*(volatile u32 *)(addr)) = (d)
#define GET_DWORD(addr)            (*(volatile u32 *)(addr))

static void install_memory(void)
{
     u32 priv_mem_base_addr;
    u32 priv_mem_len;
    u32 share_mem_base_addr; 	
    u32 share_mem_len;			
    u32 arg_base;
    void *pbuf;

    priv_mem_base_addr = __G_ALI_MM_PRIVATE_AREA_START_ADDR;
    priv_mem_len = __G_ALI_MM_PRIVATE_AREA_SIZE;
    share_mem_base_addr =  __G_ALI_MM_SHARED_MEM_START_ADDR;
    share_mem_len = __G_ALI_MM_SHARED_MEM_SIZE;

    //printk("priv_mem_base_addr:0x%x, priv_mem_len:0x%x, share_mem_base_addr:0x%x, __G_ALI_MM_MCAPI_MEM_START_ADDR:0x%x, __G_ALI_MM_MCAPI_MEM_LEN:0x%x\n"
   //		, priv_mem_base_addr, priv_mem_len, share_mem_base_addr, __G_ALI_MM_MCAPI_MEM_START_ADDR, __G_ALI_MM_MCAPI_MEM_SIZE);
    // Store MCAPI share memory start addr at __SEE_RAM_BASE -24
    // Store MCAPI share memory end addr at __SEE_RAM_BASE -20 
    // Store private memory start addr at   __SEE_RAM_BASE -16
    // Store private memory end addr at     __SEE_RAM_BASE -12 
    // Store share memory start addr at __SEE_RAM_BASE - 8 
    // Store share memory end addr at   __SEE_RAM_BASE - 4 
    arg_base = __VMEMALI(priv_mem_base_addr+0x200 -16);
    /*Added by tony*/
    SET_DWORD((arg_base-8), (__G_ALI_MM_MCAPI_MEM_START_ADDR&0x1fffffff));
    SET_DWORD((arg_base-4), ((__G_ALI_MM_MCAPI_MEM_START_ADDR&0x1fffffff)+__G_ALI_MM_MCAPI_MEM_SIZE));
    pbuf = (u32 *)(__G_ALI_MM_MCAPI_MEM_START_ADDR);
    //printk("^^^^^^^^^^^^^^^^^MAIN pbuf:0x%x\n", pbuf);
    memset(pbuf, 0, __G_ALI_MM_MCAPI_MEM_SIZE);
    /*Added end*/
    SET_DWORD(arg_base, (priv_mem_base_addr&0x1fffffff));   
    SET_DWORD((arg_base+4), ((priv_mem_base_addr&0x1fffffff)+priv_mem_len));    
    SET_DWORD((arg_base+8), (share_mem_base_addr&0x1fffffff));    
    SET_DWORD((arg_base+12), ((share_mem_base_addr&0x1fffffff)+share_mem_len));  
    if(GET_DWORD(DRAM_SPLIT_CTRL_BASE+PVT_S_ADDR) == 0)
    {
    	 share_mem_base_addr =  __G_ALI_MM_SHARED_MEM_START_ADDR;
    	 share_mem_len = __G_ALI_MM_SHARED_MEM_SIZE + __G_ALI_MM_MCAPI_MEM_SIZE;
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+PVT_S_ADDR, priv_mem_base_addr&0x1fffffff);	
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+PVT_E_ADDR, ((priv_mem_base_addr&0x1fffffff)+priv_mem_len));    
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+SHR_S_ADDR, share_mem_base_addr&0x1fffffff);    
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+SHR_E_ADDR, ((share_mem_base_addr&0x1fffffff)+share_mem_len));	
    }	
}

static void print_memory(void)
{
	u32 priv_mem_base_addr;
	u32 priv_mem_len;
	u32 share_mem_base_addr;	
	u32 share_mem_len;	

	priv_mem_base_addr = __G_ALI_MM_PRIVATE_AREA_START_ADDR;
    priv_mem_len = __G_ALI_MM_PRIVATE_AREA_SIZE;
    share_mem_base_addr =  __G_ALI_MM_SHARED_MEM_START_ADDR;
    share_mem_len = __G_ALI_MM_SHARED_MEM_SIZE;

#if 0
    printk("\n/**************RPC addr as follow:*****************/\n");
	printk("priv_mem_base_addr:0x%x\n",priv_mem_base_addr);
	printk("priv_mem_end_addr:0x%x\n", priv_mem_base_addr + priv_mem_len);
	printk("share_mem_base_addr:0x%x\n",share_mem_base_addr);
	printk("share_mem_end_addr:0x%x\n",share_mem_base_addr + share_mem_len);
	printk("__G_ALI_MM_MCAPI_MEM_START_ADDR:0x%x\n",__G_ALI_MM_MCAPI_MEM_START_ADDR);
	printk("__G_MM_MCAPI_MEM_END_ADDR:0x%x\n",__G_ALI_MM_MCAPI_MEM_START_ADDR + __G_ALI_MM_MCAPI_MEM_SIZE);
    printk("/***************************************************/\n\n");
#endif	
}

#if 0
static void prior_see_boot(void)
{
    unsigned long share_mem_base_addr;

    share_mem_base_addr = __VMEMALI(__G_ALI_MM_SHARED_MEM_START_ADDR);

    /** Before boot see, MUST initialize shm head fields first.
     */
    memset((char *)share_mem_base_addr, 0, 4*1024);
}
#endif

#if defined(__MCAPI_TEST__)

#include "pr.h"

void MCAPI_Hi(void *arg);
static Thread mcapi_task_id;

#endif

void stop_watchdog();
extern int ali_rpc_init_mutex(void);

static int __init mcomm_ali_modinit(void)
{
	int rc;


   // printk("\n******MAIN mcomm_ali_modinit entered! *****\n");
    /*here stopping watchdog Just for GDB debug usage*/
#if 1 
    stop_watchdog();
#endif
    ali_mcomm_data.localId = MAIN_CPU_ID;
    ali_mcomm_data.remoteId = SEE_CPU_ID;
    spin_lock_init(&ali_mcomm_data.lock);
    //sema_init(&ali_mcomm_data.syncSem, 1);
    sema_init(&ali_mcomm_data.syncSem, 0);
    ali_mcomm_data.mbx.init = MbxHwMain_Init;
    ali_mcomm_data.mbx.deinit = MbxHwMain_Deinit;
    ali_mcomm_data.mbx.tx = MbxHwMain_Tx;
    ali_mcomm_data.mbx.rx = MbxHwMain_Rx;

    /*Masked for s3921 prototype debug*/
    ali_rpc_init_mutex();

#ifdef __BOOT_SEE_IN_UBOOT__
	  print_memory();
	  //printk("\n******MAIN will not install_memory! *****\n");
#else
    printk("\n******MAIN will install_memory! *****\n");
    install_memory();
#endif
    
    //printk("MAIN, try to bootup the see!\n");
    //prior_see_boot();
    //see_boot(SEE_RUN_ADDR);  /*Let rpc module statup the see*/
    //printk("MAIN,  bootup the see completed!\n");

	rc = mcomm_init(&mcomm_ali_ops, THIS_MODULE);
	if (rc) {
		printk(KERN_ERR "%s: Failed to initialize mcomm driver.\n", __func__);
		goto out1;
	}

	rc = platform_driver_register(&mcomm_driver);
	if (rc) {
		printk(KERN_ERR "%s: Failed to register platform driver.\n", __func__);
		goto out2;
	}

	/* Finally, register an mcomm device. We can only have one, so if there's
	 * an error we should just give up. */
	rc = mcomm_pdev_add();
	if (rc)
		goto out3;
/*
#if defined(__MCAPI_TEST__)
    
    rc = PR_ThreadCreate(&mcapi_task_id, MCAPI_Hi, 0, 0, 0, NULL);
    if (rc) {
        PR_LOG("Thread create MCAPI_Hi error.\n");
    }
#else
    rpc_ali_modinit();

#endif 
	//PR_LOG("mcomm_ali_modinit end\n");
*/
	return 0;

out3:
	platform_driver_unregister(&mcomm_driver);
out2:
	mcomm_exit();
out1:
	return rc;
}
//module_init(mcomm_ali_modinit);
rootfs_initcall(mcomm_ali_modinit);

static void mcomm_ali_modexit(void)
{
	mcomm_pdev_release();
	platform_driver_unregister(&mcomm_driver);
	mcomm_exit();
}
module_exit(mcomm_ali_modexit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("David.L");
MODULE_DESCRIPTION("ALi shared memory communication support for multi-core shared memory channel");

