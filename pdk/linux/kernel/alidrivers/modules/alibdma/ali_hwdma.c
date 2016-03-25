/*
 *  Copyright (c) 2012, ALi Corporation.
 *  All rights reserved.
 *
 *      File:           ali_hwdma.c
 *      Version:        1.0
 *      Date:           2012-03-09
 *      Description:    ali bdma module init/exit main source file.
 */

#include "ali_hwdma.h"
#include "ali_hwdma_reg.h"

#include <linux/dma-mapping.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/mman.h>
#include <linux/param.h>
#include <linux/semaphore.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <asm/cacheflush.h>
#include <asm/pgtable.h>
#define DEVICE_NAME             "hwDMA"
#define NUM_DEVICE              1

#define BDMA_KRNL_DRV_VERSION   "ALi BDMA Kernel Driver Version ["__DATE__" "__TIME__"]"

/* Module Parameter */
static int UsingCmdQ        			= HWDMA_CMDQ_MODE;  // HWDMA_CMDQ_MODE define by Makefile
static int UsingGenTable    			= HWDMA_GENTABLE;   // HWDMA_GENTABLE define by Makefile
#ifdef HWDMA_LATENCY
static unsigned char DefaultBDMALatency	= HWDMA_LATENCY;
#else
static unsigned char DefaultBDMALatency	= 0;
#endif

module_param(UsingCmdQ, bool, S_IRUGO);
module_param(UsingGenTable, bool, S_IRUGO);
module_param(DefaultBDMALatency, byte, S_IRUGO);

unsigned int bdma_hw_version = 0;
//bool bdma_block_mode = true;   // Lucas add for OVG Async operation (11/2/2012), this global var is not suitable for multithread control, 20130328 AllenChen

static struct class *ali_dma;
BDMAint ali_dma_major_number = 101;
BDMAint ali_dma_minor_number = 0;
struct ali_dma_dev *ali_dma_devices;

void bdma_irq_do_tasklet(unsigned long interrupt_staus);

#ifdef ISR_ENABLE
DECLARE_TASKLET(bdma_tasklet, bdma_irq_do_tasklet, 0);
//DECLARE_COMPLETION(bdma_completion);
volatile u32 ali_dma_isr_cnt = 0;
volatile u32 ali_dma_issue_isr_cnt = 0;
#endif

extern struct ali_bdma_ops bdma_ops;

struct ali_dma_directoryEntry* entry_globaltmp; // For Interrupt handler used ?
struct ali_dma_pdtRecord pdtRecord[MAX_PDT_NUM];
int init_pdtRecord = 0;

extern struct cmdQ_buf cmdQ_2bank[2];//BDMAuint *cmdQ_2bank[2];
extern BDMAuint  cmdQ_len, cmdQ_cnt;
static irqreturn_t bdma_irq_handle(int irq, void *dev_id);
#if BDMA_BUGFIX
void Delay_Read_handle(unsigned int cnt);
DECLARE_DELAYED_WORK(work_queue_delay_read_reg, Delay_Read_handle);


void Delay_Read_handle(unsigned int cnt)
{
	unsigned delay_cnt=0;
	cnt = 1000;
	ALIDMA_PRINTK(2, "...................Delay_Read_handle start...............\n");
	while(ALI_HWDMA_GET_UINT32(BDMA_DMA_RD)== 0x33334444)
	{
		delay_cnt++;
		if (delay_cnt > cnt){
			ALIDMA_PRINTK(4, "Delay_Read_handle time out...............\n");
			break;
		}
	}
	ALIDMA_PRINTK(2, "Delay_Read_handle delay_cnt %08x\n",delay_cnt);
}
#endif

BDMAint ali_dma_open(struct inode *inode, struct file *filp)
{
    dma_addr_t dma_phy_addr;
	int i;
    struct ali_dma_directoryEntry* entry = kmalloc(sizeof(struct ali_dma_directoryEntry), GFP_KERNEL);

    if(!entry)
    {
        ALIDMA_ERR_PRINTK("kmalloc for struct ali_dma_directoryEntry failed \n");
        return -ENOMEM;
    }
    entry->bdma_block_mode = true;

    /* create struct ali_dma_directoryEntry */
#ifdef PDE_MULTI
//=====================================================================================================
	if(init_pdtRecord == 0)
	{
		for(i = 0; i < MAX_PDT_NUM; i++)
		{
			pdtRecord[i].pdt = 0;
			pdtRecord[i].tgid = 0;
			pdtRecord[i].ref = 0;
		}
		init_pdtRecord = 1;
	}

	for(i = 0; i < MAX_PDT_NUM; i++)
		if(pdtRecord[i].tgid == current->tgid)
			break;

	if(i == MAX_PDT_NUM)
	{
#ifdef CONFIG_ARM
    entry->pde = (unsigned int) dma_alloc_coherent(NULL, PAGE_SIZE<<pgd_BA_offset, &dma_phy_addr, GFP_KERNEL|GFP_DMA );
    ALIDMA_PRINTK(0, "Pde address = %08x, phy = %08x\n",entry->pde, __pa(entry->pde));
#else
    entry->pde = (unsigned int) dma_alloc_coherent(NULL, PAGE_SIZE, &dma_phy_addr, GFP_KERNEL|GFP_DMA );
#endif
    ALIDMA_PRINTK(0, "New process PDE alloc : %08x\n", entry->pde);
    if(!entry->pde)
    {
        ALIDMA_ERR_PRINTK("dma_alloc_coherent failed \n");
        kfree(entry);
        return -ENOMEM;
	    }else
#ifdef CONFIG_ARM	    
		memset(entry->pde , 0, PAGE_SIZE<<pgd_BA_offset);
#else
		memset(entry->pde , 0, PAGE_SIZE);
#endif

		for(i = 0; i < MAX_PDT_NUM; i++)
		{
			if(pdtRecord[i].ref == 0)
			{
				pdtRecord[i].pdt = entry->pde;
				pdtRecord[i].tgid = current->tgid;
				pdtRecord[i].ref = 1;
				break;
			}
		}
		if(i == MAX_PDT_NUM)
			ALIDMA_PRINTK(0, "pdtRecord table full \n");
	}
	else
	{
		printk("PDE thread alloc!\n");	
		entry->pde = pdtRecord[i].pdt;
		pdtRecord[i].ref++;
    }

	ALIDMA_PRINTK(0, "page directory address = %x, current = %d \n", (unsigned int)entry->pde, current->pid);
//=====================================================================================================
#else
#ifdef CONFIG_ARM
    entry->pde = (unsigned int) dma_alloc_coherent(NULL, PAGE_SIZE<<pgd_BA_offset, &dma_phy_addr, GFP_KERNEL|GFP_DMA );
    ALIDMA_PRINTK(0, "Pde address = %08x, phy = %08x\n",entry->pde, __pa(entry->pde));
#else
    entry->pde = (unsigned int) dma_alloc_coherent(NULL, PAGE_SIZE, &dma_phy_addr, GFP_KERNEL|GFP_DMA );
#endif
    if(!entry->pde)
    {
        ALIDMA_ERR_PRINTK("dma_alloc_coherent failed \n");
        kfree(entry);
        return -ENOMEM;
    }
#endif
    filp->private_data = entry;

    ALIDMA_PRINTK(4, " device opened \n");
    ALIDMA_PRINTK(4, " device opened, page directory virtual address = %08x, physical address = %08x\n", entry->pde, dma_phy_addr);

    return 0;
}

BDMAint ali_dma_release(struct inode *inode, struct file *filp)
{
	int i;
    struct ali_dma_directoryEntry* entry = (struct ali_dma_directoryEntry*) filp->private_data;

#ifdef PDE_MULTI
	for(i = 0; i < MAX_PDT_NUM; i++)
		if(pdtRecord[i].tgid == current->tgid)
			break;
	if(i == MAX_PDT_NUM)
	{
		ALIDMA_PRINTK(0, "No pdt found in release\n");
	}
	pdtRecord[i].ref --;
	if(pdtRecord[i].ref == 0)
	{
    dma_free_coherent(NULL, PAGE_SIZE, (void*)(entry->pde), (dma_addr_t)(entry->pde & 0x1FFFFFFF));
		pdtRecord[i].tgid = 0;
		pdtRecord[i].pdt = 0;
	}
#else

    dma_free_coherent(NULL, PAGE_SIZE, (void*)(entry->pde), (dma_addr_t)(entry->pde & 0x1FFFFFFF));
#endif
    kfree(entry);

    ALIDMA_PRINTK(4, " device closed \n");

    return 0;
}

BDMAint ali_dma_set_latency(unsigned char latency)
{
	ALI_HWDMA_SET_UINT32((ALI_HWDMA_GET_UINT32(BDMA_SYS_CTRL) & ~LATENCY_MASK) | (latency << LATENCY_BIT ), BDMA_SYS_CTRL);
	return 0;
}

BDMAint ali_dma_reset(void)
{
	//DMA_HWRESET = 0x00004000;
	//DMA_HWRESET = 0x00000000;

	//iowrite32(0x00000010 , GE_HWRESET);
	//iowrite32(0x00000000 , GE_HWRESET);
	iowrite32(ioread32(DMA_HWRESET)|0x00004000 , (void __iomem *)DMA_HWRESET);
	//printk("DMA_HWRESET = 0x%x\n", DMA_HWRESET);
	//printk("ioread HWRESET = 0x%x\n", ioread32(DMA_HWRESET));
	iowrite32(ioread32(DMA_HWRESET)&0xFFFFBFFF , (void __iomem *)DMA_HWRESET);		//printk("ioread HWRESET = 0x%x\n", ioread32(DMA_HWRESET));
	printk("ali_dma_reset finish\n");

	memset((BDMAuint *)(cmdQ_2bank[0].vir_adr) , 0, cmdQ_len);
	memset((BDMAuint *)(cmdQ_2bank[1].vir_adr) , 0, cmdQ_len);
	return 0;
}

BDMAint ali_dma_set_block_mode(struct file *filp, bool mode)
{
	struct ali_dma_directoryEntry* entry = (struct ali_dma_directoryEntry*)filp->private_data;
	entry->bdma_block_mode = mode;
   	ALIDMA_PRINTK(0, " bdma_block_mode set to %s entry %x\n", (entry->bdma_block_mode) ? "enable":"disable", entry );
   	return 0;
}

#if 1//#if BDMA_DEBUG
void dump_reg(void)
{
	int i;
	unsigned int* wrBuffer;
	int cmd_cnt = ALI_HWDMA_GET_UINT32(0x8);	
#ifdef CONFIG_ARM	
	wrBuffer = (unsigned int*)(__va(ALI_HWDMA_GET_UINT32(4)));
#else
	wrBuffer = (unsigned int*)((ALI_HWDMA_GET_UINT32(4) & 0x1FFFFFFF) | 0xA0000000);
#endif
	printk("CMDQ PHY BA : %08x\n", ALI_HWDMA_GET_UINT32(4));
	//printk("BDMA cmd buffer length = %08x\n",ALI_HWDMA_GET_UINT32(4));

	for(i=0;i<cmd_cnt;i++){
		printk(" KERNEL cmd %02d : %08x ",i, wrBuffer[2*i]);
		printk(" %08x\n",wrBuffer[2*i+1]);
	}
#if 1
	for (i=0;i<28;i++){
//	for (i=26;i<28;i++){
			ALIDMA_PRINTK(0, "0x%08x = %08x\n",i*4, ALI_HWDMA_GET_UINT32(i*4));		
			//printk("0x%08x = %08x\n",i*4, ALI_HWDMA_GET_UINT32(i*4));		
	}
#endif	
}
#endif	
BDMAint ali_dma_wait_finish_timeout(struct file *filp, unsigned int timeout)
{
	struct ali_dma_directoryEntry* entry = (struct ali_dma_directoryEntry*)filp->private_data;
	timeout = jiffies + timeout;	
	if(entry->bdma_block_mode == false) // Async mode.
	{
		/* Wait for command finish. */
#ifdef ISR_ENABLE		
		#if 1
		int test_cnt = 0;
		
		while(ali_dma_isr_cnt!=ali_dma_issue_isr_cnt){
			ALIDMA_PRINTK(ISR_DEBUG, "ali_dma_isr_cnt %d ... \n",ali_dma_isr_cnt);
			if(time_after(jiffies, timeout)) {
				printk("BDMA HW cmdq time out...............\n");
				ali_dma_isr_cnt = 0;
				ali_dma_issue_isr_cnt = 0;
				return -1;					/*time out*/
			}
		}
		ali_dma_isr_cnt = 0;
		ali_dma_issue_isr_cnt = 0;
		#else
		if(wait_for_completion_interruptible_timeout(&bdma_completion, timeout) == 0 )
		{
#if BDMA_DEBUG
			dump_reg();
#endif
			ALIDMA_PRINTK(0, "ali_dma_wait_finish_timeout time out...............\n");
			init_completion(&bdma_completion);
		    return -1;                  /*time out*/
		}
#if BDMA_DEBUG
		dump_reg();
#endif
		init_completion(&bdma_completion);
	#endif
#else
		while((((ALI_HWDMA_GET_UINT32(BDMA_DMA_CTRL)) & 0x01) != 0) ||
				(((ALI_HWDMA_GET_UINT32(BDMA_CMD_CTRL)) & 0x01) != 0)){
			if(time_after(jiffies, timeout)) {
				ALIDMA_PRINTK(0, "BDMA HW cmdq time out...............\n");
		 		return -1;					/*time out*/
		 	}
		}
#endif
		ALIDMA_PRINTK(1, " ali_dma_wait_finish_timeout done!\n");		
		return 0;
	}
	else
	{
   		ALIDMA_PRINTK(0, " BDMA block mode is enable, you should not ALI_HWDMA_WAIT_FINISH_TIMEOUT.\n");
		return 0;
	}
}

BDMAint ali_dma_start(struct ali_dma_dev *dev, struct file *filp, unsigned long arg)
{
    ali_hwdma_job job;
    struct ali_dma_directoryEntry* entry = (struct ali_dma_directoryEntry*)filp->private_data;

    if( copy_from_user(&job, (ali_hwdma_job __user *)arg, sizeof(ali_hwdma_job)) != 0 )
    {
        ALIDMA_ERR_PRINTK(" copy_from_user fail\n");
        return -ENOMEM;
    }
#ifdef BDMA_DUMP
printk("----------------------------------------------\n");
#endif

    entry_globaltmp = entry;

    if((job.type == ALIBDMA_COPY) || (job.type == ALIBDMA_OVG_BLIT)){
#ifdef BDMA_DUMP
	printk("1D memcpy : \n");
#endif
        switch(job.mmu_mode)
        {
            case MMU_SRC_ON_DST_ON:
#ifdef BDMA_DUMP
	printk("MMU_SRC_ON_DST_ON : SRC table gen \n");
#endif
                if( bdma_ops.address_process(entry, job.source_data, job.copy_length) != 0) {
                        ALIDMA_ERR_PRINTK("bdma_ops.address_process (source) Error\n");
                        return -EFAULT;
                }
#ifdef BDMA_DUMP
	printk("MMU_SRC_ON_DST_ON : DST table gen \n");
#endif
                if(bdma_ops.address_process(entry, job.destination_address, job.copy_length) !=0) {
                        ALIDMA_ERR_PRINTK("bdma_ops.address_process (destination) Error\n");
                        return -EFAULT;
                }
                break;
            case MMU_SRC_ON_DST_OFF:
#ifdef BDMA_DUMP
	printk("MMU_SRC_ON_DST_OFF : SRC table gen \n");
#endif
                if( bdma_ops.address_process(entry, job.source_data, job.copy_length) != 0) {
                        ALIDMA_ERR_PRINTK("bdma_ops.address_process (source) Error\n");
                        return -EFAULT;
                }
                break;
            case MMU_SRC_OFF_DST_ON:
#ifdef BDMA_DUMP
	printk("MMU_SRC_OFF_DST_ON : DST table gen \n");
#endif
                if(bdma_ops.address_process(entry, job.destination_address, job.copy_length) !=0) {
                        ALIDMA_ERR_PRINTK("bdma_ops.address_process (destination) Error\n");
                        return -EFAULT;
                }
                break;
            case MMU_SRC_OFF_DST_OFF: // Do not thing.
                break;
        }
    }
    else if (job.type == ALIBDMA_SET) { // Only need to care of Destination.
#ifdef BDMA_DUMP
	printk("1D memset : \n");
#endif
        if(job.mmu_mode == MMU_SRC_OFF_DST_ON || job.mmu_mode == MMU_SRC_ON_DST_ON){
#ifdef BDMA_DUMP
	printk("MMU_SRC_OFF_DST_ON : DST table gen \n");
#endif				
            if(bdma_ops.address_process(entry, job.destination_address, job.copy_length) !=0) {
                ALIDMA_ERR_PRINTK("bdma_ops.address_process Error\n");
				ali_dma_reset();
                return -EFAULT;
            }
    }
    }
    else {
        ALIDMA_ERR_PRINTK("Unknown Job type of ALI_HWDMA_START\n");
        return -EPERM;
    }

    if( bdma_ops.hw_start(dev, entry, &job) != 0) {
        ALIDMA_ERR_PRINTK("bdma_ops.hw_start Fail\n");
		ali_dma_reset();		
        return -EFAULT;
    }

    // Remove the comment to do clean ?
    //bdma_ops.clean(entry, &job);

    return 0;
}

BDMAint ali_dma_2d_start(struct ali_dma_dev *dev, struct file *filp, unsigned long arg)
{
    ali_hwdma_2Djob job;
    struct ali_dma_directoryEntry* entry = (struct ali_dma_directoryEntry*)filp->private_data;

    if( copy_from_user(&job, (ali_hwdma_2Djob __user *)arg , sizeof(ali_hwdma_2Djob) ) != 0 )
    {
        ALIDMA_ERR_PRINTK(" copy_from_user fail\n");
        return -ENOMEM;
    }
#ifdef BDMA_DUMP
printk("----------------------------------------------\n");
#endif

    if(job.mmu_mode != MMU_SRC_OFF_DST_OFF) {
        BDMAuint src_copy_size = (job.src_y + job.height) * job.src_stride;
        BDMAuint dst_copy_size = (job.dst_y + job.height) * job.dst_stride;
        ALIDMA_PRINTK(1, "src copy size = %d, dst copy size = %d \n", src_copy_size, dst_copy_size);

        if(job.type == ALIBDMA_COPY){
#ifdef BDMA_DUMP
	printk("2D memcpy : \n");
#endif
            switch(job.mmu_mode)
            {
                case MMU_SRC_ON_DST_ON:
#ifdef BDMA_DUMP
	printk("MMU_SRC_ON_DST_ON : SRC table gen \n");
#endif
                    if( bdma_ops.address_process(entry, job.source_data, src_copy_size) != 0 ) {
                        ALIDMA_ERR_PRINTK("bdma_ops.address_process (source) Error\n");
                        return -EFAULT;
                    }
#ifdef BDMA_DUMP
	printk("MMU_SRC_ON_DST_ON : DST table gen \n");
#endif
                    if( bdma_ops.address_process(entry, job.destination_address, dst_copy_size)!= 0 ) {
                        ALIDMA_ERR_PRINTK("bdma_ops.address_process (destination) Error\n");
                        return -EFAULT;
                    }
                    break;
                case MMU_SRC_ON_DST_OFF:
#ifdef BDMA_DUMP
	printk("MMU_SRC_ON_DST_OFF : SRC table gen \n");
#endif
                    if( bdma_ops.address_process(entry, job.source_data, src_copy_size) != 0 ) {
                        ALIDMA_ERR_PRINTK("bdma_ops.address_process (source) Error\n");
                        return -EFAULT;
                    }
                    break;
                case MMU_SRC_OFF_DST_ON:
#ifdef BDMA_DUMP
	printk("MMU_SRC_OFF_DST_ON : DST table gen \n");
#endif
                    if( bdma_ops.address_process(entry, job.destination_address, dst_copy_size) != 0 ) {
                        ALIDMA_ERR_PRINTK("bdma_ops.address_process (destination) Error\n");
                        return -EFAULT;
                    }
                    break;
                case MMU_SRC_OFF_DST_OFF: // Do not thing.
                    break;
            }
        }
        else if (job.type == ALIBDMA_SET) { // Only need to care of Destination.
#ifdef BDMA_DUMP
	printk("2D memset : \n");
#endif
            if(job.mmu_mode == MMU_SRC_OFF_DST_ON || job.mmu_mode == MMU_SRC_ON_DST_ON)
#ifdef BDMA_DUMP
	printk("MMU_SRC_OFF_DST_ON : DST table gen \n");
#endif				
                if( bdma_ops.address_process(entry, job.destination_address, dst_copy_size) != 0 ) {
                    ALIDMA_ERR_PRINTK("bdma_ops.address_process Error\n");
					ali_dma_reset();					
                    return -EFAULT;
                }
        }
        else if(job.type == ALIBDMA_OVG_BLIT) {
            ALIDMA_ERR_PRINTK("Not support ALIBDMA_OVG_BLIT Job type of ALI_HWDMA_2D_START\n");
            return -EPERM;
        }
        else {
            ALIDMA_ERR_PRINTK("Unknown Job type of ALI_HWDMA_2D_START\n");
            return -EPERM;
        }
    }
#ifdef BDMA_DUMP	
	else {// 
		if(job.type == ALIBDMA_COPY)
			printk("2D memcpy : \n");
		else if(job.type == ALIBDMA_SET)
			printk("2D memset : \n");
    }
#endif
    if( bdma_ops.hw_start_2D(dev, entry, &job) != 0) {
        ALIDMA_ERR_PRINTK("bdma_ops.hw_start Fail\n");
		ali_dma_reset();		
        return -EFAULT;
    }

    // Remove the comment to do clean ?
    //bdma_ops.clean(entry, &job);

    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
	int ali_dma_ioctl(struct file *filp , BDMAuint cmd, unsigned long arg)
#else
	int ali_dma_ioctl(struct inode *inode, struct file *filp, BDMAuint cmd, unsigned long arg)
#endif
{
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)	
	struct inode *inode = filp->f_mapping->host;
#endif	

    /*parameter relative data*/
    struct ali_dma_dev *dev = container_of(inode->i_cdev, struct ali_dma_dev, cdev);
    BDMAint result;

    switch (cmd)
    {
        case ALI_HWDMA_START:
			if(down_interruptible( &(dev->hw_sema) ))	return -1;
            result = ali_dma_start(dev, filp, arg);
			up(&(dev->hw_sema) );
            break;
        case ALI_HWDMA_2D_START:
			if(down_interruptible( &(dev->hw_sema) ))	return -1;
            result = ali_dma_2d_start(dev, filp, arg);
			up(&(dev->hw_sema) );
            break;
        case ALI_HWDMA_Reset:
            result = ali_dma_reset();
            break;
        case ALI_HWDMA_SET_LATENCY:
        	result = ali_dma_set_latency((unsigned char) arg);
        	break;
        /* Lucas add for OVG Async operation (11/2/2012) */
        case ALI_HWDMA_SET_BLOCK_MODE:
			if(down_interruptible( &(dev->hw_sema) ))	return -1;
			result = ali_dma_set_block_mode(filp,(bool) arg);
			up(&(dev->hw_sema) );
        	break;
        case ALI_HWDMA_WAIT_FINISH_TIMEOUT:   
			if(down_interruptible( &(dev->hw_sema) ))	return -1;
			result = ali_dma_wait_finish_timeout(filp, (unsigned int) arg);
			up(&(dev->hw_sema) );
        	break;
        default:
            ALIDMA_ERR_PRINTK("Unknow Command ID %d \n",  cmd);
            result = -EFAULT;
            break;
    }

    return result;
}
/*-----------------------------------------------------------------*/
/*  Used to mmap HW IO address into user vitual space              */
/*-----------------------------------------------------------------*/
int ali_dma_mmap(struct file* filp, struct vm_area_struct *vma)
{
	

	if(vma->vm_pgoff) {
		/*HW register map & physical continuous buffer map*/
		vma->vm_flags |= VM_IO;
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
		if(remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff
			, vma->vm_end - vma->vm_start, vma->vm_page_prot))
			return -EAGAIN;
	}
	else{
		printk("None mmap path for BDMA !!!\n");
	}	
	return 0;
	
}

static struct file_operations ali_dma_fops = {
    .owner      = THIS_MODULE,                  /* owner */
    .open       = ali_dma_open,                 /* open system call */
    .release    = ali_dma_release,              /* release system call */
    
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
    .unlocked_ioctl  = ali_dma_ioctl,                /* ioctl system call */
#else
    .ioctl      = ali_dma_ioctl,                /* ioctl system call */
#endif
	.mmap		= ali_dma_mmap,				/*mmap system call*/

};

#ifdef ISR_ENABLE
void bdma_irq_do_tasklet(unsigned long interrupt_staus)
{
    //ALIDMA_PRINTK(0, " Interrupt Status 0x%.08x \n", (unsigned int) interrupt_staus );

    if(interrupt_staus & INT_RD_PDE_NOT_VALID)
        ALIDMA_ERR_PRINTK("INT_RD_PDE_NOT_VALID\n");
         
    if(interrupt_staus & INT_RD_PTE_NOT_VALID)
        ALIDMA_ERR_PRINTK("INT_RD_PTE_NOT_VALID\n");
        
    if(interrupt_staus & INT_WR_PDE_NOT_VALID)
        ALIDMA_ERR_PRINTK("INT_WR_PDE_NOT_VALID\n");
        
    if(interrupt_staus & INT_WR_PTE_NOT_VALID)
        ALIDMA_ERR_PRINTK("INT_WR_PTE_NOT_VALID\n");
        
    if(interrupt_staus & INT_CMD_FORMAT_ERROR)
        ALIDMA_ERR_PRINTK("INT_CMD_FORMAT_ERROR\n");
        
    if(interrupt_staus & INT_CMD_QUEUE_FINISH)
        ALIDMA_PRINTK(1, "INT_CMD_QUEUE_FINISH \n");
        
    if(interrupt_staus & INT_BDMA_FINISH)
    {
	ALIDMA_PRINTK(1, "INT_BDMA_FINISH ... \n");
//		complete(&bdma_completion);
		ali_dma_isr_cnt++;
    }
    
    return;
}

static irqreturn_t bdma_irq_handle(int irq, void *dev_id)
{
    bdma_tasklet.data = ALI_HWDMA_GET_UINT32(BDMA_INT_STATUS_MASK);

    ALI_HWDMA_SET_UINT32(0x7F, BDMA_INT_STATUS_MASK);
    tasklet_schedule(&bdma_tasklet);
    return IRQ_HANDLED;
}
#endif
static int __init ali_dma_init(void)
{
    BDMAint result = 0, i, ret;
    dev_t dev;

    ALIDMA_PRINTK(0, " ALi hw DMA Kernel Driver Module INIT 20130419 : multi-thread support\n");
    ALIDMA_PRINTK(0, " Version: %s \n", BDMA_KRNL_DRV_VERSION);
    ALIDMA_PRINTK(0, " UsingCmdQ = %d, UsingGenTable = %d\n", UsingCmdQ, UsingGenTable);
    ALIDMA_PRINTK(0, " PGDIR_SHIFT = %d, PAGE_SHIFT = %d\n", PGDIR_SHIFT, PAGE_SHIFT);

    /* register major number or dynamic allocate major number */
    if(ali_dma_major_number)    // manual asign device major number
    {
        dev = MKDEV(ali_dma_major_number, ali_dma_minor_number);
        result = register_chrdev_region(dev, NUM_DEVICE, DEVICE_NAME);
    }
    else                        // kernel asign device major number
    {
        result = alloc_chrdev_region(&dev, ali_dma_minor_number, NUM_DEVICE, DEVICE_NAME);
        ali_dma_major_number = MAJOR(dev);
    }

    if (result < 0)
    {
        ALIDMA_ERR_PRINTK(" register_chrdev_region or alloc_chrdev_region failed ! \n");
        return result;
    }

    /* allocate ali_vg_dev, and assign for each ali_vg_dev_test object*/
    ali_dma_devices = kzalloc(sizeof(struct ali_dma_dev), GFP_KERNEL);
    if(!ali_dma_devices)
    {
        ALIDMA_ERR_PRINTK(" ali_dma_devices alloc fail\n");
        unregister_chrdev_region(dev, 1);
        return -ENOMEM;
    }

    /* init */
    cdev_init(&(ali_dma_devices->cdev), &ali_dma_fops);
    ali_dma_devices->cdev.owner = THIS_MODULE;

    if(cdev_add(&(ali_dma_devices->cdev), dev , 1))
    {
        ALIDMA_ERR_PRINTK(" cdev_add fail \n");
        kfree(ali_dma_devices);
        unregister_chrdev_region(dev, 1);
        return -EFAULT;
    }

    /* class created (post to /proc/devices) */
    ali_dma = class_create(THIS_MODULE, DEVICE_NAME);
    if(IS_ERR(ali_dma))
    {
        ALIDMA_ERR_PRINTK(" class_create Class device fail \n");
        cdev_del(&(ali_dma_devices->cdev));
        kfree(ali_dma_devices);
        unregister_chrdev_region(dev, 1);
        return -EFAULT;
    }

    device_create(ali_dma, NULL, dev, NULL, DEVICE_NAME);

#ifndef init_MUTEX
	sema_init(&(ali_dma_devices->hw_sema),1); 
#else
    init_MUTEX(&(ali_dma_devices->hw_sema));
#endif

#ifdef ISR_ENABLE
    /* request irq */		
    //result = request_irq(IRQ_BDMA, bdma_irq_handle, IRQF_DISABLED , DEVICE_NAME, NULL);
    ALIDMA_PRINTK(0, " Enable ISR ....\n");    
    result = request_irq(IRQ_BDMA, bdma_irq_handle, 0 , "hwdma", NULL);
    if(result != 0)
    {
        ALIDMA_ERR_PRINTK(" request irq fail \n");
        device_destroy(ali_dma, dev);
        class_destroy(ali_dma);
        cdev_del(&(ali_dma_devices->cdev));
        kfree(ali_dma_devices);
        unregister_chrdev_region(dev, 1);
        return result;
    }

//	init_completion(&bdma_completion);
#endif		
    /* re-configure ali_bdma_ops function point if UsingCmdQ/UsingGenTable not equal to Makefile define */
    if((UsingCmdQ != HWDMA_CMDQ_MODE) || (UsingGenTable != HWDMA_GENTABLE))
        bdma_ops_mode_setup(&bdma_ops, UsingCmdQ, UsingGenTable);

    /* Modify the DMA Latency hardware default value (priority need to lower than DE) */
	ali_dma_set_latency(DefaultBDMALatency);

#if BDMA_BUGFIX
	//queue = create_singlethread_workqueue("BDMA_queue");
	//INIT_WORK(&work_queue_delay_read_reg, Delay_Read_handle);
	//mod_timer(&timer,jiffies+5);
#endif	

    /* Check BDMA HW Version */
#ifdef FPGA_MODE
    bdma_hw_version = BDMA_VER_C3701C;
#else
 #ifdef C3821_VERIFY
    bdma_hw_version = ALI_HWDMA_GET_UINT32(BDMA_VERSION_REG)>>9; // C3821, using io(0x18) bit 9 as version bit
 #else
    bdma_hw_version = ALI_HWDMA_GET_UINT32(BDMA_VERSION_REG); // before C3821 io(0x60) bit 0  is the version bit
    bdma_hw_version = BDMA_VER_C3701C;                        // Fix the IC version for s3921
 #endif
#endif
    ALIDMA_PRINTK(0, " BDMA HW Version = 0x%.08x %s\n", bdma_hw_version, (bdma_hw_version == 0) ? "S3701C" : ((bdma_hw_version == 1) ? "C3701C" : "Unknown Version"));

	if(bdma_hw_version == BDMA_VER_S3701C){		
		cmdQ_len = PAGE_SIZE;			
	}else{
#ifdef C3821_VERIFY
		cmdQ_len = PAGE_SIZE;
#else
		cmdQ_len = PAGE_SIZE;//5*(64/8) + 3*(64/8)+ 10*(64/8);
#endif
	}	
	cmdQ_2bank[0].vir_adr = (BDMAint)dma_alloc_coherent(NULL, cmdQ_len, &(cmdQ_2bank[0].phy_adr), GFP_KERNEL|GFP_DMA);
	cmdQ_2bank[1].vir_adr = (BDMAint)dma_alloc_coherent(NULL, cmdQ_len, &(cmdQ_2bank[1].phy_adr), GFP_KERNEL|GFP_DMA);
	ALIDMA_PRINTK(0,"Allocate 2 bank cmdQ %08x,%08x cmdQ_len = %d\n",cmdQ_2bank[0].vir_adr,cmdQ_2bank[1].vir_adr,cmdQ_len);
    /* Reset BDMA */
	ali_dma_reset();

    ALIDMA_PRINTK(0, " ALi hw DMA Kernel Driver Module INIT Done\n");	
 #ifdef C3821_VERIFY
	cmdQ_2bank[0].vir_adr|= 0x10;
	cmdQ_2bank[1].vir_adr|= 0x10;	
	cmdQ_2bank[0].phy_adr|= 0x10;
	cmdQ_2bank[1].phy_adr|= 0x10;	

    ALIDMA_PRINTK(0, ".................... BDMA IO test case begin .................\n");	
	ALIDMA_PRINTK(0, "1 BDMA_SYS_CTRL %08x\n", ALI_HWDMA_GET_UINT32(BDMA_SYS_CTRL));
//    ALI_HWDMA_SET_UINT32(0x00, BDMA_SYS_CTRL);	
//	ALIDMA_PRINTK(0, "2 BDMA_SYS_CTRL %08x\n", ALI_HWDMA_GET_UINT32(BDMA_SYS_CTRL));	

    ALI_HWDMA_SET_UINT32(0x55, BDMA_INT_STATUS_MASK);
	ALIDMA_PRINTK(0, "0 Set BDMA_INT_STATUS_MASK %08x , and readback %08x \n",0x55,ALI_HWDMA_GET_UINT32(BDMA_INT_STATUS_MASK));

    ALI_HWDMA_SET_UINT32(0x2A, BDMA_INT_STATUS_MASK);
	ALIDMA_PRINTK(0, "1 Set BDMA_INT_STATUS_MASK %08x , and readback %08x \n",0x2A,ALI_HWDMA_GET_UINT32(BDMA_INT_STATUS_MASK));
    ALIDMA_PRINTK(0, ".................... BDMA IO test case end .................\n");	
#endif
    return 0;

}

static void __exit ali_dma_exit(void)
{
    dev_t dev = MKDEV(ali_dma_major_number, ali_dma_minor_number);

    ALIDMA_PRINTK(0, " DRIVER EXIT called \n");

#ifdef ISR_ENABLE
    free_irq(IRQ_BDMA, NULL);
#endif
    device_destroy(ali_dma, dev);
    class_destroy(ali_dma);
    if(ali_dma_devices)
    {
        cdev_del(&(ali_dma_devices->cdev));
        kfree(ali_dma_devices);
    }
#if BDMA_BUGFIX	
	cancel_delayed_work(&work_queue_delay_read_reg);
	flush_scheduled_work();
#endif
	dma_free_coherent(NULL, cmdQ_len, cmdQ_2bank[0].vir_adr, cmdQ_2bank[0].phy_adr);
	dma_free_coherent(NULL, cmdQ_len, cmdQ_2bank[1].vir_adr, cmdQ_2bank[1].phy_adr);
	ALIDMA_PRINTK(0,"Release 2 bank cmdQ !\n");	
    unregister_chrdev_region(dev, 1);
    return;
}

module_init(ali_dma_init);
module_exit(ali_dma_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ALi");
MODULE_VERSION(BDMA_KRNL_DRV_VERSION);
MODULE_DESCRIPTION("ALi BDMA Kernel Device Driver");



