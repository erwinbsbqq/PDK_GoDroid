/*
 *  Copyright (c) 2012, ALi Corporation.
 *  All rights reserved.
 *
 *      File:           ali_hwdma_util.c
 *      Version:        1.0
 *      Date:           2012-03-09
 *      Description:    ali bdma module operation implement source file.
 */

#include "ali_hwdma.h"
#include "ali_hwdma_reg.h"

#include <linux/slab.h>
#include <linux/pagemap.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/timer.h>

#include <asm/cacheflush.h>
#ifdef CONFIG_ARM
#include <asm/pgtable.h>
#include <asm/pgtable-2level.h>
#define _PAGE_PRESENT               (1<<0)  /* implemented in software */
#else
#include <asm/pgtable.h>
#endif

#define PAGE_ADDRESS_FIX 1

extern unsigned int bdma_hw_version;
#ifdef ISR_ENABLE
//extern struct completion bdma_completion;
extern volatile u32 ali_dma_isr_cnt;
extern volatile u32 ali_dma_issue_isr_cnt;
#endif
//extern bool bdma_block_mode;

struct cmdQ_buf cmdQ_2bank[2];//BDMAuint  cmdQ_2bank[2];
BDMAuint  cmdQ_len, cmdQ_cnt = 0;
BDMAuint   MMU_flag_prev = 0, MMU_flag = 0;

#if BDMA_BUGFIX
extern struct work_struct work_queue_delay_read_reg;
#endif

void dump_reg(void); // Used for debug 20130326, AllenChen
BDMAint bdma_make_pages_present(unsigned long addr, unsigned long end)
{
    BDMAint ret, len, write;
    struct vm_area_struct * vma;

    vma = find_vma(current->mm, addr);
    if (!vma) {
        ALIDMA_ERR_PRINTK("Can't find vma of addr(0x%.08x) \n", (unsigned int) addr);
        return -ENOMEM;
    }
    
	if((vma->vm_flags & (VM_IO | VM_PFNMAP)))
		return 0;
		
    write = (vma->vm_flags & VM_WRITE) != 0; // Check Write able
    BUG_ON(addr >= end);
    BUG_ON(end > vma->vm_end);
    len = DIV_ROUND_UP(end, PAGE_SIZE) - addr/PAGE_SIZE;

    down_read(&(current->mm)->mmap_sem);
    ret = get_user_pages(current, current->mm, addr, len, write, 0, NULL, NULL); // addr need to page alignment ?
    up_read(&(current->mm)->mmap_sem);

    if (ret < 0)
        return ret; 	
    return ret == len ? 0 : -EFAULT;
}

#if 0 // disable getting page table from system. It may mismatch with HW format.
BDMAint get_from_system(struct ali_dma_directoryEntry* entry, BDMAuint addr, BDMAuint size)
{
    BDMAuint first_pde, last_pde, virt_temp;
    BDMAuint first_pte, last_pte;
    BDMAuint* pgd_tbladdr = (BDMAuint*)entry->pde;
    BDMAint i,j;

    first_pde = pgd_index(addr);
    last_pde = pgd_index(addr + size - 1);

    ALIDMA_PRINTK(3, " address = %x , fir pgd  = %d , last pgd = %d , size = %d, pgd = %x\n", addr, first_pde, last_pde, size, entry->pde);

    virt_temp = addr;

    if( bdma_make_pages_present(addr, addr+size) != 0) {
        ALIDMA_ERR_PRINTK("bdma_make_pages_present Error\n");
        return -EFAULT;
    }

    for(i = first_pde; i <= last_pde ; i++)
    {
        BDMAuint* pteaddr = NULL;
        pgd_tbladdr[pgd_index(virt_temp)] = ((pgd_val(*pgd_offset(current->mm, virt_temp)))| 0x00000001) & 0x1FFFFFFF;
        flush_data_cache_page(pgd_val(*pgd_offset(current->mm,virt_temp)));

        pteaddr = (BDMAuint*)((BDMAuint)(pgd_tbladdr[pgd_index(virt_temp)] & 0xFFFFFFFE) | 0xA0000000);

        first_pte = __pte_offset(virt_temp);
        last_pte = (i < last_pde) ? 1023 : __pte_offset(addr + size - 1);

        ALIDMA_PRINTK(3, " address = %x ,  first pte  = %d , last pte = %d\n", virt_temp, first_pte, last_pte);

        for(j=first_pte ; j <= last_pte ; j++)
        {
//          if(!(pteaddr[__pte_offset(virt_temp)] & _PAGE_MODIFIED))
            flush_data_cache_page(virt_temp & 0xFFFFF000);
            ALIDMA_PRINTK(3, " address = %x ,pgd index = %d, pte_addr = %x, pte_val = %x\n", virt_temp ,j , (u32)pteaddr, pteaddr[__pte_offset(virt_temp)]);

            virt_temp+=PAGE_SIZE;
        }
    }

    return 0;
}
#endif

/* fill page directory called by gen_page_table */
pmd_t*  fill_pageDirectory_bdma(BDMAint i, BDMAuint virt_temp, BDMAuint* pgd_tbladdr)
{
    BDMAuint addr;
    pgd_t *pgdp;
    pud_t *pudp;
    pmd_t *pmdp;

    if(!(*(pgd_tbladdr+i) & _PAGE_PRESENT))
    {
        unsigned int *temp;
        temp = dma_alloc_coherent(NULL, PAGE_SIZE, &addr, GFP_KERNEL);
        memset(temp , 0, PAGE_SIZE);
        pgd_tbladdr[i] = addr | _PAGE_PRESENT;
    }
#ifdef BDMA_DUMP
	printk("pgd_tbladdr[%d] at %08x, value %08x\n",i,(unsigned int)(&pgd_tbladdr[i])&0x1FFFFFFF,pgd_tbladdr[i]);
#endif
    pgdp = pgd_offset(current->mm, virt_temp );
    pudp = pud_offset(pgdp, virt_temp);
    pmdp = pmd_offset(pudp, virt_temp);
    return pmdp;
}

/* fill page table called by gen_page_table */
void fill_pageTable_bdma(BDMAint j, pmd_t * pmdp, BDMAuint virt_temp, BDMAuint* pte_tbladdr)
{
    pte_t *ptep;
#ifdef PAGE_ADDRESS_FIX
	struct page* pageX;
    ALIDMA_PRINTK(7, "pte_addr = %x  \n", (BDMAuint)pte_tbladdr);
    ptep = pte_offset_kernel(pmdp, virt_temp);
	pageX = pte_page(*ptep);
	pte_tbladdr[j] = page_to_phys(pageX)|1;
#ifdef BDMA_DUMP
	printk("pte_tbladdr[%d] at %08x, value %08x\n",j,(unsigned int)(&pte_tbladdr[j])&0x1FFFFFFF,page_to_phys(pageX)|1);
#endif
#else
    ALIDMA_PRINTK(7, "pte_addr = %x  \n", (BDMAuint)pte_tbladdr);

    ptep = pte_offset(pmdp, virt_temp);
    pte_tbladdr[j] = (BDMAuint)pte_val(*ptep);
#endif

    ALIDMA_PRINTK(7, " address = %x ,pte index = %d, pte_val = %x\n", virt_temp ,j , (BDMAuint)pte_val(*ptep));

}

BDMAint gen_page_table(struct ali_dma_directoryEntry* entry, BDMAuint addr, BDMAuint size)
{
    BDMAuint first_pde, last_pde, virt_temp;
    BDMAuint first_pte, last_pte;
    BDMAuint* pgd_tbladdr = (BDMAuint*)entry->pde;
    BDMAint i,j;
	BDMAint PDE_order = 32 - PGDIR_SHIFT;
	BDMAint PTE_order = PGDIR_SHIFT - PAGE_SHIFT;
	pmd_t *pmd;
#ifdef CONFIG_ARM
	BDMAint pte_off;
	pgd_t *pgd;
	pte_t* pte;
	struct page *page;
	pgd = pgd_offset(current->mm, virt_temp);
	pmd = pmd_offset(pgd, virt_temp);
	ALIDMA_PRINTK(3, "-------------- pdg = %08x, pmd = %08x --------------\n",pgd, pmd);
	first_pde = pgd_index(addr);
	last_pde = pgd_index(addr + size - 1);

	ALIDMA_PRINTK(3, " address = %x , fir pgd  = %d, last pgd = %d, size = %d, pgd = %x\n", addr, first_pde, last_pde, size, entry->pde);
	
	virt_temp = addr;
	ALIDMA_PRINTK(3, " addr = %08x size = %08x\n", addr,size);
	if( bdma_make_pages_present(addr, addr+size) != 0) {
		ALIDMA_ERR_PRINTK("bdma_make_pages_present Error\n");
		return -EFAULT;
	}

	for(i = first_pde; i <= last_pde ; i++)
	{
		pmd = fill_pageDirectory_bdma(i>>1, virt_temp, pgd_tbladdr);

		if (i&1)	pte_off = (1<<PT_LEN) ;
		else		pte_off = 0;

		first_pte = pte_index(virt_temp) + pte_off;
		last_pte = ((i < last_pde) ? ((1<<PT_LEN)-1) : pte_index(addr + size - 1) ) + pte_off;

		ALIDMA_PRINTK(3, "------------------- pgd_tbladdr[%d]  = %08x  ",i>>1, pgd_tbladdr[i>>1] );
		ALIDMA_PRINTK(3, " first pte  = %d , last pte = %d ---------------------\n", first_pte, last_pte);

		BDMAuint *pte_tbl = __va(pgd_tbladdr[i>>1] & 0xFFFFF000);
       	for(j=first_pte ; j <= last_pte ; j++)
	       {
			//BDMAuint *pte_tbl = ioremap_nocache(pgd_tbladdr[i] & 0xFFFFFFFE, PAGE_SIZE);
			pgd = pgd_offset(current->mm, virt_temp);
			pmd = pmd_offset(pgd, virt_temp);
			pte = pte_offset_kernel(pmd, virt_temp);
			flush_dcache_page( pte_page(*pte) );
			fill_pageTable_bdma(j, pmd, virt_temp, pte_tbl);
			virt_temp+=PAGE_SIZE;
			iounmap(pte_tbl);
		}
		dmac_flush_range((void *)(pte_tbl+first_pte), (void *)(pte_tbl+last_pte+1));
		ALIDMA_PRINTK(1, "pte table %08x entry %d ~ %d  flush %08x - %08x \n",(int)pte_tbl, first_pte, last_pte,  (int)(pte_tbl+first_pte), (int)(pte_tbl+last_pte+1));		
	}
#else //#ifdef CONFIG_ARM
    first_pde = pgd_index(addr);
    last_pde = pgd_index(addr + size - 1);

    ALIDMA_PRINTK(7, " address = %x, fir pgd  = %d, last pgd = %d, size = %d, pgd = %x\n", addr, first_pde, last_pde, size, entry->pde);
    ALIDMA_PRINTK(7,"PGDIR_SHIFT = %d, PAGE_SHIFT = %d PDE_order=%d PTE_order=%d\n",PGDIR_SHIFT, PAGE_SHIFT, PDE_order,PTE_order);

    virt_temp = addr;

    if( bdma_make_pages_present(addr, addr+size) != 0) {
        ALIDMA_ERR_PRINTK("bdma_make_pages_present Error\n");
       	return -EFAULT;
    }

    for(i = first_pde; i <= last_pde ; i++)
    {
        pmd = fill_pageDirectory_bdma(i, virt_temp, pgd_tbladdr);

        first_pte = __pte_offset(virt_temp);
        last_pte = (i < last_pde) ? ((1<<PDE_order)-1) : __pte_offset(addr + size - 1);

        ALIDMA_PRINTK(7, " address = %x ,  first pte  = %d , last pte = %d\n", virt_temp, first_pte, last_pte);

        for(j=first_pte ; j <= last_pte ; j++)
        {
            BDMAuint *pte_tbl = ioremap_nocache(pgd_tbladdr[i] & 0xFFFFFFFE, PAGE_SIZE);
            BDMAuint temp = virt_temp & 0xFFFFF000;
            flush_data_cache_page(temp);
            fill_pageTable_bdma(j, pmd, virt_temp, pte_tbl);
            virt_temp+=PAGE_SIZE;
            iounmap(pte_tbl);
        }
    }
#endif //#ifdef CONFIG_ARM
    return 0;
}

void ali_dma_directoryClean(BDMAuint* pde, BDMAuint addr, BDMAuint length)
{
    BDMAuint first_pde = pgd_index(addr);
    BDMAuint last_pde = pgd_index(addr + length - 1);
    BDMAuint temp;

    for(temp = first_pde ; temp <= last_pde ; temp++) {
        if((*(pde+temp) & _PAGE_PRESENT)){
            dma_free_coherent(NULL, PAGE_SIZE, ioremap(pde[temp] & 0xFFFFFFFE, PAGE_SIZE), (pde[temp] & 0xFFFFFFFE));
            *(pde+temp)  = 0;
        }
    }
}

void ali_dma_clean(struct ali_dma_directoryEntry* entry, ali_hwdma_job *job)
{
    ali_dma_directoryClean((BDMAuint*)entry->pde, job->source_data, job->copy_length);
    ali_dma_directoryClean((BDMAuint*)entry->pde, job->destination_address, job->copy_length);
}

BDMAint ali_dma_cmdIO(struct ali_dma_dev *dev, struct ali_dma_directoryEntry* entry, ali_hwdma_job* job)
{
    BDMAuint write_pg_index, cmd_length = 0;
    long long left = 0;
    BDMAuint *wrBuffer;
	
    /* switch cmdQ buffer in 2 bank */
	CMDQ_BANK_SWITCH
	
#ifdef CONFIG_ARM
	wrBuffer = (unsigned int*)((unsigned int)cmdQ.vir_adr);
#else
	//wrBuffer = (unsigned int*)(((unsigned int)cmdQ & 0x1FFFFFFF) | 0xA0000000);
	wrBuffer = (unsigned int*)ioremap_nocache(cmdQ.phy_adr, cmdQ_len);
#endif
	ALIDMA_PRINTK(4, "cmdQ BA = %08x %08x cmdQ_cnt %08x\n",cmdQ.vir_adr, wrBuffer, cmdQ_cnt);	
#if BDMA_DEBUG
//printk("BDMA_CMD_CTRL = %08x\n",ALI_HWDMA_GET_UINT32(BDMA_CMD_CTRL)); cmd_length++;
		CmdQWriteReg(wrBuffer, BDMA_DMA_RD,  0x33334444 ); cmd_length++;
		CmdQCheckReg(wrBuffer, BDMA_CMD_CTRL, VG_BUSY_BIT, EQUAL_ZERO); cmd_length++;
		CmdQWriteReg(wrBuffer, BDMA_MMU_RD_PARAM, 1); cmd_length++;
		CmdQWriteReg(wrBuffer, BDMA_MMU_WR_PARAM, 1); cmd_length++;
//		CmdQWriteReg(wrBuffer, BDMA_MMU_RD, entry->pde & 0x1FFFFFFF); cmd_length++;
//		CmdQWriteReg(wrBuffer, BDMA_MMU_WR, entry->pde & 0x1FFFFFFF); cmd_length++;
		CmdQWriteReg(wrBuffer, BDMA_DMA_RD,  job->source_data ); cmd_length++;
		CmdQWriteReg(wrBuffer, BDMA_DMA_WR,  job->destination_address); cmd_length++;
//		CmdQWriteReg(wrBuffer, BDMA_DMA_RD,  0x33333333 ); cmd_length++;
//		CmdQWriteReg(wrBuffer, BDMA_DMA_WR,  0x44444444); cmd_length++;

		CmdQWriteReg(wrBuffer, BDMA_DMA_LEN, job->copy_length); cmd_length++;
		CmdQWriteReg(wrBuffer, BDMA_DMA_CTRL, 0x11110000); cmd_length++;
		CmdQWriteReg(wrBuffer, BDMA_DMA_CTRL, 0x33330000|1); cmd_length++;
		//CmdQWriteReg(wrBuffer, BDMA_DMA_CTRL, 0xffffffff); cmd_length++;
		ALI_HWDMA_SET_UINT32(((unsigned int)cmdQ & 0x1FFFFFFF), BDMA_CMD_BA);
		ALI_HWDMA_SET_UINT32(cmd_length,       BDMA_CMD_LEN);
		ALI_HWDMA_SET_UINT32(CMD_QUEUE_ENABLE, BDMA_CMD_CTRL);
#if BDMA_BUGFIX
		schedule_delayed_work(&work_queue_delay_read_reg, 5);
#endif
#else
		CmdQCheckReg(wrBuffer, BDMA_DMA_CTRL, BDMA_ENABLE_BIT, EQUAL_ZERO);cmd_length ++;
    /* Check OVG Busy (Used for communication with OVG module) */
    if ((job->type == ALIBDMA_OVG_BLIT)||(job->ovg_sync)) {
#if BDMA_BUGFIX
		CmdQWriteReg(wrBuffer, BDMA_DMA_RD,  0x33334444 ); cmd_length++;
#endif
		CmdQCheckReg(wrBuffer, BDMA_CMD_CTRL, VG_BUSY_BIT, EQUAL_ZERO);
        cmd_length++;
    }
	if((job->type == ALIBDMA_SET)&&(job->mmu_mode == MMU_SRC_ON_DST_ON)) // Avoid to set SRC MMU in  memset case 20130109 Allen
		job->mmu_mode = MMU_SRC_OFF_DST_ON;

    /* Setup MMU Register Command Queue */
    switch(job->mmu_mode) {
        case MMU_SRC_ON_DST_ON:
#ifdef CONFIG_ARM
		CmdQWriteReg(wrBuffer, BDMA_MMU_RD, __pa(PD_TABLE_BA(entry->pde, job->source_data)));
		CmdQWriteReg(wrBuffer, BDMA_MMU_WR, __pa(PD_TABLE_BA(entry->pde, job->destination_address)));
#else
            CmdQWriteReg(wrBuffer, BDMA_MMU_RD, entry->pde & 0x1FFFFFFF);
            CmdQWriteReg(wrBuffer, BDMA_MMU_WR, entry->pde & 0x1FFFFFFF);
#endif
            //ALI_HWDMA_SET_UINT32(0,     BDMA_MMU_RD_PARAM);
            //ALI_HWDMA_SET_UINT32(0,     BDMA_MMU_WR_PARAM);
		CmdQWriteReg(wrBuffer, BDMA_MMU_RD_PARAM, 0);
		CmdQWriteReg(wrBuffer, BDMA_MMU_WR_PARAM, 0);
		cmd_length  += 4;
			MMU_flag = 1;
            break;
        case MMU_SRC_OFF_DST_ON:
#ifdef CONFIG_ARM
		CmdQWriteReg(wrBuffer, BDMA_MMU_RD, __pa(PD_TABLE_BA(entry->pde, job->source_data))); cmd_length ++;
		CmdQWriteReg(wrBuffer, BDMA_MMU_WR, __pa(PD_TABLE_BA(entry->pde, job->destination_address)));			
#else
            CmdQWriteReg(wrBuffer, BDMA_MMU_RD, entry->pde & 0x1FFFFFFF); cmd_length ++;
            CmdQWriteReg(wrBuffer, BDMA_MMU_WR, entry->pde & 0x1FFFFFFF);
#endif
            //ALI_HWDMA_SET_UINT32(0x01 , BDMA_MMU_RD_PARAM);
            //ALI_HWDMA_SET_UINT32(0,     BDMA_MMU_WR_PARAM);
		CmdQWriteReg(wrBuffer, BDMA_MMU_RD_PARAM, 1);
		CmdQWriteReg(wrBuffer, BDMA_MMU_WR_PARAM, 0);
		cmd_length  += 3;
			MMU_flag = 1;			
            break;
        case MMU_SRC_ON_DST_OFF:
#ifdef CONFIG_ARM
		CmdQWriteReg(wrBuffer, BDMA_MMU_RD, __pa(PD_TABLE_BA(entry->pde, job->source_data)));
		CmdQWriteReg(wrBuffer, BDMA_MMU_WR, __pa(PD_TABLE_BA(entry->pde, job->destination_address))); cmd_length ++;
#else
            CmdQWriteReg(wrBuffer, BDMA_MMU_RD, entry->pde & 0x1FFFFFFF);
            CmdQWriteReg(wrBuffer, BDMA_MMU_WR, entry->pde & 0x1FFFFFFF); cmd_length ++;
#endif
			//ALI_HWDMA_SET_UINT32(0 ,    BDMA_MMU_RD_PARAM);
			//ALI_HWDMA_SET_UINT32(0x01,  BDMA_MMU_WR_PARAM);
			CmdQWriteReg(wrBuffer, BDMA_MMU_RD_PARAM, 0);
			CmdQWriteReg(wrBuffer, BDMA_MMU_WR_PARAM, 1);
			cmd_length  += 3;
			MMU_flag = 1;			
            break;
        case MMU_SRC_OFF_DST_OFF:
			//ALI_HWDMA_SET_UINT32(0x01 , BDMA_MMU_RD_PARAM);
			//ALI_HWDMA_SET_UINT32(0x01 , BDMA_MMU_WR_PARAM);
			CmdQWriteReg(wrBuffer, BDMA_MMU_RD_PARAM, 1);
			CmdQWriteReg(wrBuffer, BDMA_MMU_WR_PARAM, 1);
			cmd_length  += 2;
			MMU_flag = 0;
        break;
    }

	if((job->type == ALIBDMA_COPY) || (job->type == ALIBDMA_OVG_BLIT)) {
		if(bdma_hw_version == BDMA_VER_S3701C){
			/*******************************************************************************************************
	    	 * Bug List I.    DRAM buffer,
			 * Solution:
			 *   In kernel driver, need to use command queue mode and can only copy one page in one hardware job.
	    	 *   So if we want to copy 2 pages, driver needs to enable hardware two times to finish.
			 *******************************************************************************************************/
	    	write_pg_index = 0;
			left = job->copy_length;
			while(left > 0)
	    	{
			    CmdQWriteReg(wrBuffer, BDMA_DMA_RD,  job->source_data         + write_pg_index*PAGE_SIZE);
			    CmdQWriteReg(wrBuffer, BDMA_DMA_WR,  job->destination_address + write_pg_index*PAGE_SIZE);
		    	CmdQWriteReg(wrBuffer, BDMA_DMA_LEN, (left > PAGE_SIZE) ? PAGE_SIZE : left);
			    CmdQCheckReg(wrBuffer, BDMA_DMA_CTRL, BDMA_ENABLE_BIT, EQUAL_ZERO);				
	    	    CmdQWriteReg(wrBuffer, BDMA_DMA_CTRL, BDMA_ENABLE);
			    left = left - PAGE_SIZE;
		    	cmd_length  += 5;
	    	    write_pg_index++;
			}       	
		}else{
			CmdQWriteReg(wrBuffer, BDMA_DMA_RD,  TRANSFER_VIR_2LEV(job->source_data) );
			CmdQWriteReg(wrBuffer, BDMA_DMA_WR,  TRANSFER_VIR_2LEV(job->destination_address));
#ifdef C3821_Verify
			if (TRANSFER_VIR_2LEV(job->source_data)&0x10)			printk("WRAP MODE case triggered BDMA_DMA_RD!!!\n");
			if (TRANSFER_VIR_2LEV(job->destination_address)&0x10)	printk("WRAP MODE case triggered BDMA_DMA_WR!!!\n");
#endif
			CmdQWriteReg(wrBuffer, BDMA_DMA_LEN, job->copy_length);
			CmdQWriteReg(wrBuffer, BDMA_DMA_CTRL, BDMA_ENABLE);
			CmdQCheckReg(wrBuffer, BDMA_DMA_CTRL, BDMA_ENABLE_BIT, EQUAL_ZERO);
			cmd_length  += 5;
		}
	}
	else if(job->type == ALIBDMA_SET) {
		CmdQWriteReg(wrBuffer, BDMA_DMA_RD,  0x00000000 ); cmd_length ++;
		CmdQWriteReg(wrBuffer, BDMA_DMA_WR,  TRANSFER_VIR_2LEV(job->destination_address));
#ifdef C3821_Verify
		if ((TRANSFER_VIR_2LEV(job->destination_address)&0x10))	printk("WRAP MODE case triggered BDMA_DMA_WR!!!\n");
#endif
		CmdQWriteReg(wrBuffer, BDMA_DMA_LEN, job->copy_length);
        cmd_length  += 2;

		switch(job->bits_pixel)
		{
			case ALIBDMA_8_BIT:
				CmdQWriteReg(wrBuffer, BDMA_SYS_CTRL, (ALI_HWDMA_GET_UINT32(BDMA_SYS_CTRL) & (~MEM_SET_VAL_31_24_MASK)) | (0<< MEM_SET_VAL_31_24_BIT) );cmd_length ++;
				CmdQWriteReg(wrBuffer, BDMA_DMA_CTRL, MEM_SET_LEN_1_BYTE | MEM_SET_ENABLE | BDMA_ENABLE | ((job->source_data << MEM_SET_VAL_23_0_BIT) & MEM_SET_VAL_23_0_MASK) );
				cmd_length ++;
				break;
			case ALIBDMA_16_BIT:
				CmdQWriteReg(wrBuffer, BDMA_SYS_CTRL, (ALI_HWDMA_GET_UINT32(BDMA_SYS_CTRL) & (~MEM_SET_VAL_31_24_MASK)) | (0<< MEM_SET_VAL_31_24_BIT) );cmd_length ++;
				CmdQWriteReg(wrBuffer, BDMA_DMA_CTRL, MEM_SET_LEN_2_BYTE | MEM_SET_ENABLE | BDMA_ENABLE | ((job->source_data << MEM_SET_VAL_23_0_BIT) & MEM_SET_VAL_23_0_MASK) );
				cmd_length ++;
				break;
			case ALIBDMA_24_BIT:
				ALIDMA_ERR_PRINTK("3 Bytes Value Memset is not allowed\n");
//				kfree(cmdQ);
				return -EPERM;
				break;
			case ALIBDMA_32_BIT:            	
				CmdQWriteReg(wrBuffer, BDMA_SYS_CTRL, (ALI_HWDMA_GET_UINT32(BDMA_SYS_CTRL) & (~MEM_SET_VAL_31_24_MASK)) | ((job->source_data >> 24)<< MEM_SET_VAL_31_24_BIT) );
				CmdQWriteReg(wrBuffer, BDMA_DMA_CTRL, MEM_SET_LEN_4_BYTE | MEM_SET_ENABLE | BDMA_ENABLE | ((job->source_data << MEM_SET_VAL_23_0_BIT) & MEM_SET_VAL_23_0_MASK) );
				cmd_length +=2;
				break;
		}

        CmdQCheckReg(wrBuffer, BDMA_DMA_CTRL, BDMA_ENABLE_BIT, EQUAL_ZERO);
        cmd_length ++;
//	    while( ALI_HWDMA_GET_UINT32(BDMA_DMA_CTRL)&0x1 );
    }
/*******************************************************************************************************
 * Bug List II.   Command queue read error
 * Solution:
 *   In kernel driver, need to fill 0 for two commands after the last command in queue buffer.
 *    Example, we have a copy job, and this copy job need command queue size 10, then software need to
 *    allocate size 12, and file position 11 & 12 as 0x0.
 *******************************************************************************************************/
#ifndef C3503_FIX
        CmdQCheckReg(wrBuffer, BDMA_DMA_CTRL, BDMA_ENABLE_BIT, EQUAL_ZERO);//cmd_length ++;
        CmdQCheckReg(wrBuffer, BDMA_DMA_CTRL, BDMA_ENABLE_BIT, EQUAL_ZERO);//cmd_length ++;
        CmdQCheckReg(wrBuffer, BDMA_DMA_CTRL, BDMA_ENABLE_BIT, EQUAL_ZERO);//cmd_length ++;
#endif
#ifdef CONFIG_ARM
	dmac_flush_range((void *)cmdQ.vir_adr, (void *)( cmdQ.vir_adr+((cmdQ_len+1)<<1)));
	while(ALI_HWDMA_GET_UINT32(BDMA_CMD_CTRL)&0x1);  /* Wait until last cmdQ is done */
	ALI_HWDMA_SET_UINT32(__pa((unsigned int)cmdQ.vir_adr), BDMA_CMD_BA);
#else
	while(ALI_HWDMA_GET_UINT32(BDMA_CMD_CTRL)&0x1);  /* Wait until last cmdQ is done */
	ALI_HWDMA_SET_UINT32(cmdQ.phy_adr, BDMA_CMD_BA);
#ifdef C3821_Verify
	if (cmdQ.phy_adr&0x10)		printk("WRAP MODE case triggered !!! CMDQ BASE\n");
#endif
#endif
	ALI_HWDMA_SET_UINT32(cmd_length,       BDMA_CMD_LEN);
    ALI_HWDMA_SET_UINT32(CMD_QUEUE_ENABLE, BDMA_CMD_CTRL);
	MMU_flag = MMU_flag_prev;
#ifdef ISR_ENABLE	
	ali_dma_issue_isr_cnt++;
#endif
	if(entry->bdma_block_mode)
	{	
	    /* Wait for command finish. */
		if(job->type != ALIBDMA_OVG_BLIT){
			BDMAuint timeout = jiffies + WAIT_TICKS;			
#ifdef ISR_ENABLE			
			#if 1
			while(ali_dma_isr_cnt!=ali_dma_issue_isr_cnt){
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
			if(wait_for_completion_interruptible_timeout(&bdma_completion, WAIT_TICKS) == 0 )
			{
				ALIDMA_PRINTK(0, "BDMA HW cmdq time out (blocked)..............copy size %d\n",job->copy_length);
				ALIDMA_PRINTK(4, "KERNEL : wait finish 1D BDMA_DMA_CTRL %08x BDMA_CMD_CTRL %08x\n:",ALI_HWDMA_GET_UINT32(BDMA_DMA_CTRL),ALI_HWDMA_GET_UINT32(BDMA_CMD_CTRL));
				//kfree(cmdQ); This free is done in ISR
				//cmdQ = NULL;
				return -1;                  /*time out*/
			}
			#endif
#else
			while((((ALI_HWDMA_GET_UINT32(BDMA_DMA_CTRL)) & 0x01) != 0) ||
				(((ALI_HWDMA_GET_UINT32(BDMA_CMD_CTRL)) & 0x01) != 0)){
				ALIDMA_PRINTK(4, "KERNEL :jiffies %08x WAIT_TICKS %08x wait finish 1D BDMA_DMA_CTRL %08x BDMA_CMD_CTRL %08x\n:",jiffies, timeout, ALI_HWDMA_GET_UINT32(BDMA_DMA_CTRL),ALI_HWDMA_GET_UINT32(BDMA_CMD_CTRL));
				if(time_after(jiffies, timeout)) {
					printk("BDMA HW cmdq time out...............\n");
		 			return -1;					/*time out*/
			 	}
			}
#endif
#ifdef BDMA_DUMP
	dump_reg();
#endif
		}
	}
#if BDMA_BUGFIX
	else {
		schedule_delayed_work(&work_queue_delay_read_reg, 5);
	}
#endif
#endif
    return 0;
}

BDMAint ali_dma_registerIO(struct ali_dma_dev *dev, struct ali_dma_directoryEntry* entry, ali_hwdma_job* job)
{
    BDMAuint write_pg_index, cmd_length = 0;
    long long left = 0;
	BDMAuint timeout;

    if(down_interruptible( &(dev->hw_sema) ))
        return -1;

    switch(job->mmu_mode) {
        case MMU_SRC_ON_DST_ON:
            ALI_HWDMA_SET_UINT32((entry->pde & 0x1FFFFFFF), BDMA_MMU_RD);
            ALI_HWDMA_SET_UINT32((entry->pde & 0x1FFFFFFF), BDMA_MMU_WR);
            ALI_HWDMA_SET_UINT32(0 ,    BDMA_MMU_RD_PARAM);
            ALI_HWDMA_SET_UINT32(0,     BDMA_MMU_WR_PARAM);
            break;
        case MMU_SRC_OFF_DST_ON:
            ALI_HWDMA_SET_UINT32((entry->pde & 0x1FFFFFFF), BDMA_MMU_WR);
            ALI_HWDMA_SET_UINT32(0x01 , BDMA_MMU_RD_PARAM);
            ALI_HWDMA_SET_UINT32(0,     BDMA_MMU_WR_PARAM);
            break;
        case MMU_SRC_ON_DST_OFF:
            ALI_HWDMA_SET_UINT32((entry->pde & 0x1FFFFFFF), BDMA_MMU_RD);
            ALI_HWDMA_SET_UINT32(0 ,    BDMA_MMU_RD_PARAM);
            ALI_HWDMA_SET_UINT32(0x01,  BDMA_MMU_WR_PARAM);
            break;
        case MMU_SRC_OFF_DST_OFF:
            ALI_HWDMA_SET_UINT32(0x01 , BDMA_MMU_RD_PARAM);
            ALI_HWDMA_SET_UINT32(0x01 , BDMA_MMU_WR_PARAM);
            break;
    }


    if((bdma_hw_version == BDMA_VER_S3701C) && 
    	((job->type == ALIBDMA_COPY) || (job->type == ALIBDMA_OVG_BLIT)))
    {                                       
	    /*******************************************************************************************************
	     * Bug List I.    DRAM buffer,      
	     * Solution:                        
	     *   In kernel driver, need to use command queue mode and can only copy one page in one hardware job.
	     *   So if we want to copy 2 pages, driver needs to enable hardware two times to finish.
	     *******************************************************************************************************/
        write_pg_index = 0;                 
        left = job->copy_length;            
        while(left > 0)                     
        {                                   
            ALI_HWDMA_SET_UINT32(job->source_data + write_pg_index*PAGE_SIZE, BDMA_DMA_RD);
            ALI_HWDMA_SET_UINT32(job->destination_address + write_pg_index*PAGE_SIZE, BDMA_DMA_WR);
            ALI_HWDMA_SET_UINT32((left > PAGE_SIZE) ? PAGE_SIZE : left, BDMA_DMA_LEN);
            ALI_HWDMA_SET_UINT32(BDMA_ENABLE, BDMA_DMA_CTRL);
                                            
			/* Wait for command finish. */  
#ifdef ISR_ENABLE
			#if 1
			ali_dma_issue_isr_cnt++;
			timeout = jiffies + WAIT_TICKS;
			while(ali_dma_isr_cnt!=ali_dma_issue_isr_cnt){
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
			if(wait_for_completion_interruptible_timeout(&bdma_completion, WAIT_TICKS) == 0 )
			{                               
			    printk("BDMA HW RegisterIO time out...............\n");
  		        up(&(dev->hw_sema) );       
			    return -1;                  /*time out*/
				                            
			}                               
			#endif
#else
			timeout = jiffies + WAIT_TICKS;
			while((((ALI_HWDMA_GET_UINT32(BDMA_DMA_CTRL)) & 0x01) != 0) ||
				(((ALI_HWDMA_GET_UINT32(BDMA_CMD_CTRL)) & 0x01) != 0)){
				if(time_after(jiffies, timeout)) {
					printk("BDMA HW cmdq time out...............\n");
			 		return -1;					/*time out*/
			 	}
			}
#endif			
            left = left - PAGE_SIZE;        
            cmd_length  += 5;               
            write_pg_index++;               
        }
        up(&(dev->hw_sema) );
        return 0;             
    }                                       

    if((job->type == ALIBDMA_COPY) || (job->type == ALIBDMA_OVG_BLIT)) {
        ALI_HWDMA_SET_UINT32(job->source_data, BDMA_DMA_RD);
        ALI_HWDMA_SET_UINT32(job->destination_address, BDMA_DMA_WR);
        ALI_HWDMA_SET_UINT32(job->copy_length, BDMA_DMA_LEN);
        ALI_HWDMA_SET_UINT32(BDMA_ENABLE, BDMA_DMA_CTRL);
    }
    else if(job->type == ALIBDMA_SET) {
        ALI_HWDMA_SET_UINT32(job->destination_address, BDMA_DMA_WR);
        ALI_HWDMA_SET_UINT32(job->copy_length, BDMA_DMA_LEN);

        switch(job->bits_pixel)
        {
            case ALIBDMA_8_BIT:
                ALI_HWDMA_SET_UINT32(MEM_SET_LEN_1_BYTE | MEM_SET_ENABLE | BDMA_ENABLE | ((job->source_data << MEM_SET_VAL_23_0_BIT) & MEM_SET_VAL_23_0_MASK), BDMA_DMA_CTRL);
                break;
            case ALIBDMA_16_BIT:
                ALI_HWDMA_SET_UINT32(MEM_SET_LEN_2_BYTE | MEM_SET_ENABLE | BDMA_ENABLE | ((job->source_data << MEM_SET_VAL_23_0_BIT) & MEM_SET_VAL_23_0_MASK), BDMA_DMA_CTRL);
                break;
            case ALIBDMA_24_BIT:
                ALIDMA_ERR_PRINTK("3 Bytes Value Memset is not allowed\n");
                return -EPERM;
                break;
            case ALIBDMA_32_BIT:         	
                ALI_HWDMA_SET_UINT32((ALI_HWDMA_GET_UINT32(BDMA_SYS_CTRL) & (~MEM_SET_VAL_31_24_MASK)) | ((job->source_data >> 24)<< MEM_SET_VAL_31_24_BIT), BDMA_SYS_CTRL);
                ALI_HWDMA_SET_UINT32(MEM_SET_LEN_4_BYTE | MEM_SET_ENABLE | BDMA_ENABLE | ((job->source_data << MEM_SET_VAL_23_0_BIT) & MEM_SET_VAL_23_0_MASK), BDMA_DMA_CTRL);
                break;
        }              
    }
#ifdef ISR_ENABLE	
	ali_dma_issue_isr_cnt++;
#endif
	if(entry->bdma_block_mode)
	{
		/* Wait for command finish. */
#ifdef ISR_ENABLE
		#if 1
			timeout = jiffies + WAIT_TICKS;		
			while(ali_dma_isr_cnt!=ali_dma_issue_isr_cnt){
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
		if(wait_for_completion_interruptible_timeout(&bdma_completion, WAIT_TICKS) == 0 )
		{
		    printk("BDMA HW RegisterIO time out...............\n");
		     up(&(dev->hw_sema) );
		    return -1;                  /*time out*/
		}  
		#endif
#else
		timeout = jiffies + WAIT_TICKS;
		while((((ALI_HWDMA_GET_UINT32(BDMA_DMA_CTRL)) & 0x01) != 0) ||
			(((ALI_HWDMA_GET_UINT32(BDMA_CMD_CTRL)) & 0x01) != 0)){
			if(time_after(jiffies, timeout)) {
				printk("BDMA HW cmdq time out...............\n");
		 		return -1;					/*time out*/
		 	}
		}
#endif
	}
	up(&(dev->hw_sema) );
	return 0;
}

/***************************************************************************************************************
 * Case 4: New added feature -> Blit_2D:
 * 1.   set the bit[4] of 0xB800_a020
 * 2.   compute the new dma_lens,  the formula is width * byte_per_pixel
 * 3.   set the src offset_x and src offset_y and dest offset_x and dest offset_y to the corresponding address.
 * 4.   set picture height and byte_per_pixel to 0xb800_a050
 * 5.   set src base to the original rd base and dest base to the original wr base
 * 6.   cmdq or io enable and check finish
 ***************************************************************************************************************/
BDMAint ali_dma_cmdIO_2D(struct ali_dma_dev *dev, struct ali_dma_directoryEntry* entry, ali_hwdma_2Djob* job)
{
    BDMAuint cmd_length = 0;

    BDMAuint *wrBuffer;

    /* switch cmdQ buffer in 2 bank */
	CMDQ_BANK_SWITCH
	
#ifdef CONFIG_ARM
	wrBuffer = (unsigned int*)((unsigned int)cmdQ.vir_adr);
#else	
   // wrBuffer = (unsigned int*)(((unsigned int)cmdQ & 0x1FFFFFFF) | 0xA0000000);
	wrBuffer = (unsigned int*)ioremap_nocache(cmdQ.phy_adr, cmdQ_len);
#endif
	ALIDMA_PRINTK(4, "cmdQ BA = %08x %08x cmdQ_cnt %08x\n",cmdQ.vir_adr, wrBuffer, cmdQ_cnt);	



	CmdQCheckReg(wrBuffer, BDMA_DMA_CTRL, BDMA_ENABLE_BIT, EQUAL_ZERO);cmd_length ++;
    /* Check OVG Busy (Used for communication with OVG module) */
    if ((job->type == ALIBDMA_OVG_BLIT)||(job->ovg_sync)) {
#if BDMA_BUGFIX
		CmdQWriteReg(wrBuffer, BDMA_DMA_RD,  0x33334444 ); cmd_length++;
#endif
        CmdQCheckReg(wrBuffer, BDMA_CMD_CTRL, VG_BUSY_BIT, EQUAL_ZERO);
        cmd_length++;
    }

	if((job->type == ALIBDMA_SET)&&(job->mmu_mode == MMU_SRC_ON_DST_ON)) // Avoid to set SRC MMU in  memset case 20130109 Allen
		job->mmu_mode = MMU_SRC_OFF_DST_ON;
    /* Setup MMU Register Command Queue */
    switch(job->mmu_mode) {
        case MMU_SRC_ON_DST_ON:
#ifdef CONFIG_ARM
			CmdQWriteReg(wrBuffer, BDMA_MMU_RD, __pa(PD_TABLE_BA(entry->pde, job->source_data)));
			CmdQWriteReg(wrBuffer, BDMA_MMU_WR, __pa(PD_TABLE_BA(entry->pde, job->destination_address)));
#else
            CmdQWriteReg(wrBuffer, BDMA_MMU_RD, entry->pde & 0x1FFFFFFF);
            CmdQWriteReg(wrBuffer, BDMA_MMU_WR, entry->pde & 0x1FFFFFFF);
#endif
            //ALI_HWDMA_SET_UINT32(0,     BDMA_MMU_RD_PARAM);
            //ALI_HWDMA_SET_UINT32(0,     BDMA_MMU_WR_PARAM);
			CmdQWriteReg(wrBuffer, BDMA_MMU_RD_PARAM, 0);
			CmdQWriteReg(wrBuffer, BDMA_MMU_WR_PARAM, 0);
			cmd_length  += 4;
			MMU_flag = 1;
            break;
        case MMU_SRC_OFF_DST_ON:
#ifdef CONFIG_ARM
			CmdQWriteReg(wrBuffer, BDMA_MMU_RD, __pa(PD_TABLE_BA(entry->pde, job->source_data)));cmd_length ++;
			CmdQWriteReg(wrBuffer, BDMA_MMU_WR, __pa(PD_TABLE_BA(entry->pde, job->destination_address)));			
#else
            CmdQWriteReg(wrBuffer, BDMA_MMU_RD, entry->pde & 0x1FFFFFFF);cmd_length ++;
            CmdQWriteReg(wrBuffer, BDMA_MMU_WR, entry->pde & 0x1FFFFFFF);
#endif
			//ALI_HWDMA_SET_UINT32(0x01 , BDMA_MMU_RD_PARAM);
			//ALI_HWDMA_SET_UINT32(0,     BDMA_MMU_WR_PARAM);
			CmdQWriteReg(wrBuffer, BDMA_MMU_RD_PARAM, 1);
			CmdQWriteReg(wrBuffer, BDMA_MMU_WR_PARAM, 0);
			cmd_length  += 3;
			MMU_flag = 1;			
            break;
        case MMU_SRC_ON_DST_OFF:
#ifdef CONFIG_ARM
			CmdQWriteReg(wrBuffer, BDMA_MMU_RD, __pa(PD_TABLE_BA(entry->pde, job->source_data)));
			CmdQWriteReg(wrBuffer, BDMA_MMU_WR, __pa(PD_TABLE_BA(entry->pde, job->destination_address)));cmd_length ++;
#else
            CmdQWriteReg(wrBuffer, BDMA_MMU_RD, entry->pde & 0x1FFFFFFF);
            CmdQWriteReg(wrBuffer, BDMA_MMU_WR, entry->pde & 0x1FFFFFFF);cmd_length ++;
#endif
			//ALI_HWDMA_SET_UINT32(0 ,    BDMA_MMU_RD_PARAM);
			//ALI_HWDMA_SET_UINT32(0x01,  BDMA_MMU_WR_PARAM);
			CmdQWriteReg(wrBuffer, BDMA_MMU_RD_PARAM, 0);
			CmdQWriteReg(wrBuffer, BDMA_MMU_WR_PARAM, 1);
			cmd_length  += 3;
			MMU_flag = 1;			
            break;
        case MMU_SRC_OFF_DST_OFF:
			//ALI_HWDMA_SET_UINT32(0x01 , BDMA_MMU_RD_PARAM);
			//ALI_HWDMA_SET_UINT32(0x01 , BDMA_MMU_WR_PARAM);
			CmdQWriteReg(wrBuffer, BDMA_MMU_RD_PARAM, 1);
			CmdQWriteReg(wrBuffer, BDMA_MMU_WR_PARAM, 1);
			cmd_length  += 2;
			MMU_flag = 0;
        break;
    }

	if((job->type == ALIBDMA_COPY) || (job->type == ALIBDMA_OVG_BLIT)) {
        // set src base to the original rd base and dest base to the original wr base
		CmdQWriteReg(wrBuffer, BDMA_DMA_RD,  TRANSFER_VIR_2LEV(job->source_data) );
		CmdQWriteReg(wrBuffer, BDMA_DMA_WR,  TRANSFER_VIR_2LEV(job->destination_address));
        // compute the new dma_lens,  the formula is width * byte_per_pixel
        CmdQWriteReg(wrBuffer, BDMA_DMA_LEN, job->width * job->bits_pixel);
        // set the src offset_x / src offset_y and dest offset_x / dest offset_y to the corresponding address.
        CmdQWriteReg(wrBuffer, BLIT_2D_SRC_OFFSET_Y_X_REG, ((job->src_y<<SRC_OFFSET_Y_BIT)&SRC_OFFSET_Y_MASK)  | ((job->src_x<<SRC_OFFSET_X_BIT)&SRC_OFFSET_X_MASK) );
        CmdQWriteReg(wrBuffer, BLIT_2D_DST_OFFSET_Y_X_REG, ((job->dst_y<<DST_OFFSET_Y_BIT)&DST_OFFSET_Y_MASK)  | ((job->dst_x<<DST_OFFSET_X_BIT)&DST_OFFSET_X_MASK) );
        CmdQWriteReg(wrBuffer, BLIT_2D_SRC_DST_STRIDE_REG, ((job->dst_stride<<DST_STRIDE_BIT)&DST_STRIDE_MASK) | ((job->src_stride<<SRC_STRIDE_BIT)&SRC_STRIDE_MASK) );
        // set picture height and byte_per_pixel to 0xb800_a050
        CmdQWriteReg(wrBuffer, BLIT_2D_HEIGHT_SEL_REG, ((job->height<<BLIT_2D_HEIGHT_BIT)&BLIT_2D_HEIGHT_MASK) |((job->bits_pixel -1)) );
        // set the bit[4] of 0xB800_a020, cmdq or io enable and check finish
        CmdQWriteReg(wrBuffer, BDMA_DMA_CTRL,  BLIT_2D_ENABLE | BDMA_ENABLE );
		CmdQCheckReg(wrBuffer, BDMA_DMA_CTRL, BDMA_ENABLE_BIT, EQUAL_ZERO);
        cmd_length  += 9;

    }
    else if(job->type == ALIBDMA_SET) {
		CmdQWriteReg(wrBuffer, BDMA_DMA_RD,  0x00000000 );cmd_length ++;
	 CmdQWriteReg(wrBuffer, BDMA_DMA_WR,  TRANSFER_VIR_2LEV(job->destination_address));
        CmdQWriteReg(wrBuffer, BDMA_DMA_LEN, job->width * job->bits_pixel);
        CmdQWriteReg(wrBuffer, BLIT_2D_HEIGHT_SEL_REG, ((job->height<<BLIT_2D_HEIGHT_BIT)&BLIT_2D_HEIGHT_MASK) |((job->bits_pixel -1)) );
        CmdQWriteReg(wrBuffer, BLIT_2D_DST_OFFSET_Y_X_REG, ((job->dst_y<<DST_OFFSET_Y_BIT)&DST_OFFSET_Y_MASK)  | ((job->dst_x<<DST_OFFSET_X_BIT)&DST_OFFSET_X_MASK) );
        CmdQWriteReg(wrBuffer, BLIT_2D_SRC_DST_STRIDE_REG, (job->dst_stride<<DST_STRIDE_BIT)&DST_STRIDE_MASK );
        cmd_length  += 5;

        switch(job->bits_pixel)
        {
            case ALIBDMA_8_BIT:
				CmdQWriteReg(wrBuffer, BDMA_SYS_CTRL, (ALI_HWDMA_GET_UINT32(BDMA_SYS_CTRL) & (~MEM_SET_VAL_31_24_MASK)) | (0<< MEM_SET_VAL_31_24_BIT) );cmd_length ++;
                CmdQWriteReg(wrBuffer, BDMA_DMA_CTRL, MEM_SET_LEN_1_BYTE | BLIT_2D_ENABLE | MEM_SET_ENABLE | BDMA_ENABLE | ((job->source_data << MEM_SET_VAL_23_0_BIT) & MEM_SET_VAL_23_0_MASK) );
                cmd_length ++;
                break;
            case ALIBDMA_16_BIT:
				CmdQWriteReg(wrBuffer, BDMA_SYS_CTRL, (ALI_HWDMA_GET_UINT32(BDMA_SYS_CTRL) & (~MEM_SET_VAL_31_24_MASK)) | (0<< MEM_SET_VAL_31_24_BIT) );cmd_length ++;
                CmdQWriteReg(wrBuffer, BDMA_DMA_CTRL, MEM_SET_LEN_2_BYTE | BLIT_2D_ENABLE | MEM_SET_ENABLE | BDMA_ENABLE | ((job->source_data << MEM_SET_VAL_23_0_BIT) & MEM_SET_VAL_23_0_MASK) );
                cmd_length ++;
                break;
            case ALIBDMA_24_BIT:
                ALIDMA_ERR_PRINTK("3 Bytes Value Memset is not allowed\n");
//                kfree(cmdQ);
                return -EPERM;
                break;
            case ALIBDMA_32_BIT:            	
				CmdQWriteReg(wrBuffer, BDMA_SYS_CTRL, (ALI_HWDMA_GET_UINT32(BDMA_SYS_CTRL) & (~MEM_SET_VAL_31_24_MASK)) | ((job->source_data >> 24)<< MEM_SET_VAL_31_24_BIT) );         	
                CmdQWriteReg(wrBuffer, BDMA_DMA_CTRL, MEM_SET_LEN_4_BYTE | BLIT_2D_ENABLE | MEM_SET_ENABLE | BDMA_ENABLE | ((job->source_data << MEM_SET_VAL_23_0_BIT) & MEM_SET_VAL_23_0_MASK) );
                cmd_length +=2;
                break;
        }
        CmdQCheckReg(wrBuffer, BDMA_DMA_CTRL, BDMA_ENABLE_BIT, EQUAL_ZERO);
        cmd_length ++;
//	    while( ALI_HWDMA_GET_UINT32(BDMA_DMA_CTRL)&0x1 );		
    }

#if 0
/*******************************************************************************************************
 * Bug List II.   Command queue read error
 * Solution:
 *   In kernel driver, need to fill 0 for two commands after the last command in queue buffer.
 *    Example, we have a copy job, and this copy job need command queue size 10, then software need to
 *    allocate size 12, and file position 11 & 12 as 0x0.
 *******************************************************************************************************/
    cmd_length +=2;
#endif
#ifdef CONFIG_ARM
	dmac_flush_range((void *)cmdQ.vir_adr, (void *)( cmdQ.vir_adr+((cmdQ_len+1)<<1)));
	while( ALI_HWDMA_GET_UINT32(BDMA_CMD_CTRL)&0x1 );  /* Wait until last cmdQ is done */
	ALI_HWDMA_SET_UINT32(__pa((unsigned int)cmdQ.vir_adr), BDMA_CMD_BA);
#else
    while( ALI_HWDMA_GET_UINT32(BDMA_CMD_CTRL)&0x1 );  /* Wait until last cmdQ is done */
	ALI_HWDMA_SET_UINT32(cmdQ.phy_adr, BDMA_CMD_BA);
#endif
    ALI_HWDMA_SET_UINT32(cmd_length,       BDMA_CMD_LEN);
    ALI_HWDMA_SET_UINT32(CMD_QUEUE_ENABLE, BDMA_CMD_CTRL);
	MMU_flag_prev = MMU_flag;
#ifdef ISR_ENABLE	
	ali_dma_issue_isr_cnt++;
#endif
#ifdef BDMA_DUMP
	dump_reg();
#endif
	if(entry->bdma_block_mode)
	{
	    /* Wait for command finish. */
	if(job->type != ALIBDMA_OVG_BLIT){
#ifdef ISR_ENABLE
		#if 1
			BDMAuint timeout = jiffies + WAIT_TICKS;		
			while(ali_dma_isr_cnt!=ali_dma_issue_isr_cnt){
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
	    if(wait_for_completion_interruptible_timeout(&bdma_completion, WAIT_TICKS) == 0 )
	    {
		ALIDMA_PRINTK(0, "BDMA HW cmdq time out (blocked)...............\n");
	        //kfree(cmdQ); This free is done in ISR
		//cmdQ = NULL;
	        return -1;                  /*time out*/
	    }
		#endif
#else
		BDMAuint timeout = jiffies + WAIT_TICKS;
		while((((ALI_HWDMA_GET_UINT32(BDMA_DMA_CTRL)) & 0x01) != 0) ||
			(((ALI_HWDMA_GET_UINT32(BDMA_CMD_CTRL)) & 0x01) != 0)){
			ALIDMA_PRINTK(4, "KERNEL :jiffies %08x WAIT_TICKS %08x wait finish 2D BDMA_DMA_CTRL %08x BDMA_CMD_CTRL %08x\n:",jiffies, timeout, ALI_HWDMA_GET_UINT32(BDMA_DMA_CTRL),ALI_HWDMA_GET_UINT32(BDMA_CMD_CTRL));
			if(time_after(jiffies, timeout)) {
				printk("BDMA HW cmdq time out...............\n");
		 		return -1;					/*time out*/
		 	}
		}
#endif
	}	
	}
#if BDMA_BUGFIX
	else {
		schedule_delayed_work(&work_queue_delay_read_reg, 5);
	}
#endif
    return 0;
}

BDMAint ali_dma_registerIO_2D(struct ali_dma_dev *dev, struct ali_dma_directoryEntry* entry, ali_hwdma_2Djob* job)
{
    if(down_interruptible( &(dev->hw_sema) ))
        return -1;

    switch(job->mmu_mode) {
        case MMU_SRC_ON_DST_ON:
            ALI_HWDMA_SET_UINT32((entry->pde & 0x1FFFFFFF), BDMA_MMU_RD);
            ALI_HWDMA_SET_UINT32((entry->pde & 0x1FFFFFFF), BDMA_MMU_WR);
            ALI_HWDMA_SET_UINT32(0 ,    BDMA_MMU_RD_PARAM);
            ALI_HWDMA_SET_UINT32(0,     BDMA_MMU_WR_PARAM);
            break;
        case MMU_SRC_OFF_DST_ON:
            ALI_HWDMA_SET_UINT32((entry->pde & 0x1FFFFFFF), BDMA_MMU_WR);
            ALI_HWDMA_SET_UINT32(0x01 , BDMA_MMU_RD_PARAM);
            ALI_HWDMA_SET_UINT32(0,     BDMA_MMU_WR_PARAM);
            break;
        case MMU_SRC_ON_DST_OFF:
            ALI_HWDMA_SET_UINT32((entry->pde & 0x1FFFFFFF), BDMA_MMU_RD);
            ALI_HWDMA_SET_UINT32(0 ,    BDMA_MMU_RD_PARAM);
            ALI_HWDMA_SET_UINT32(0x01,  BDMA_MMU_WR_PARAM);
            break;
        case MMU_SRC_OFF_DST_OFF:
            ALI_HWDMA_SET_UINT32(0x01 , BDMA_MMU_RD_PARAM);
            ALI_HWDMA_SET_UINT32(0x01 , BDMA_MMU_WR_PARAM);
            break;
    }

    if((job->type == ALIBDMA_COPY)) {
        // compute the new dma_lens,  the formula is width * byte_per_pixel
        ALI_HWDMA_SET_UINT32(job->width * job->bits_pixel, BDMA_DMA_LEN);

        // set the src offset_x / src offset_y and dest offset_x / dest offset_y to the corresponding address.
        ALI_HWDMA_SET_UINT32(((job->src_y<<SRC_OFFSET_Y_BIT)&SRC_OFFSET_Y_MASK)  | ((job->src_x<<SRC_OFFSET_X_BIT)&SRC_OFFSET_X_MASK), BLIT_2D_SRC_OFFSET_Y_X_REG);
        ALI_HWDMA_SET_UINT32(((job->dst_y<<DST_OFFSET_Y_BIT)&DST_OFFSET_Y_MASK)  | ((job->dst_x<<DST_OFFSET_X_BIT)&DST_OFFSET_X_MASK), BLIT_2D_DST_OFFSET_Y_X_REG);
        ALI_HWDMA_SET_UINT32(((job->dst_stride<<DST_STRIDE_BIT)&DST_STRIDE_MASK) | ((job->src_stride<<SRC_STRIDE_BIT)&SRC_STRIDE_MASK), BLIT_2D_SRC_DST_STRIDE_REG);

        // set picture height and byte_per_pixel to 0xb800_a050
        ALI_HWDMA_SET_UINT32(((job->height<<BLIT_2D_HEIGHT_BIT)&BLIT_2D_HEIGHT_MASK) |((job->bits_pixel -1)), BLIT_2D_HEIGHT_SEL_REG);

        // set src base to the original rd base and dest base to the original wr base
        ALI_HWDMA_SET_UINT32(job->source_data, BDMA_DMA_RD);
        ALI_HWDMA_SET_UINT32(job->destination_address, BDMA_DMA_WR);

        // set the bit[4] of 0xB800_a020, cmdq or io enable and check finish
        ALI_HWDMA_SET_UINT32(BLIT_2D_ENABLE | BDMA_ENABLE , BDMA_DMA_CTRL);
    }
    else if(job->type == ALIBDMA_SET) {
        ALI_HWDMA_SET_UINT32(job->destination_address, BDMA_DMA_WR);
        ALI_HWDMA_SET_UINT32(job->width * job->bits_pixel, BDMA_DMA_LEN);
        ALI_HWDMA_SET_UINT32(((job->height<<BLIT_2D_HEIGHT_BIT)&BLIT_2D_HEIGHT_MASK) |((job->bits_pixel -1)), BLIT_2D_HEIGHT_SEL_REG);
        ALI_HWDMA_SET_UINT32(((job->dst_y<<DST_OFFSET_Y_BIT)&DST_OFFSET_Y_MASK)  | ((job->dst_x<<DST_OFFSET_X_BIT)&DST_OFFSET_X_MASK), BLIT_2D_DST_OFFSET_Y_X_REG);
        ALI_HWDMA_SET_UINT32(((job->dst_stride<<DST_STRIDE_BIT)&DST_STRIDE_MASK), BLIT_2D_SRC_DST_STRIDE_REG);

        switch(job->bits_pixel)
        {
            case ALIBDMA_8_BIT:
                ALI_HWDMA_SET_UINT32(MEM_SET_LEN_1_BYTE | BLIT_2D_ENABLE | MEM_SET_ENABLE | BDMA_ENABLE | ((job->source_data << MEM_SET_VAL_23_0_BIT) & MEM_SET_VAL_23_0_MASK), BDMA_DMA_CTRL);
                break;
            case ALIBDMA_16_BIT:
                ALI_HWDMA_SET_UINT32(MEM_SET_LEN_2_BYTE | BLIT_2D_ENABLE | MEM_SET_ENABLE | BDMA_ENABLE | ((job->source_data << MEM_SET_VAL_23_0_BIT) & MEM_SET_VAL_23_0_MASK), BDMA_DMA_CTRL);
                break;
            case ALIBDMA_24_BIT:
                ALIDMA_ERR_PRINTK("3 Bytes Value Memset is not allowed\n");
                return -EPERM;
                break;
            case ALIBDMA_32_BIT:
                ALI_HWDMA_SET_UINT32(( ALI_HWDMA_GET_UINT32(BDMA_SYS_CTRL) & (~MEM_SET_VAL_31_24_MASK) ) | ((job->source_data >> 24)<< MEM_SET_VAL_31_24_BIT), BDMA_SYS_CTRL);
                ALI_HWDMA_SET_UINT32(MEM_SET_LEN_4_BYTE | BLIT_2D_ENABLE | MEM_SET_ENABLE | BDMA_ENABLE | ((job->source_data << MEM_SET_VAL_23_0_BIT) & MEM_SET_VAL_23_0_MASK), BDMA_DMA_CTRL);
                break;
        }
    }

#ifdef ISR_ENABLE	
	ali_dma_issue_isr_cnt++;
#endif
	if(entry->bdma_block_mode)
	{
#ifdef ISR_ENABLE
		#if 1
			BDMAuint timeout = jiffies + WAIT_TICKS;		
			while(ali_dma_isr_cnt!=ali_dma_issue_isr_cnt){
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
		/* Wait for command finish. */
		if(wait_for_completion_interruptible_timeout(&bdma_completion, WAIT_TICKS) == 0 )
		{
		    printk("BDMA HW RegisterIO_2D time out...............\n");
		     up(&(dev->hw_sema) );
		    return -1;                  /*time out*/
		}  
		#endif
#else
		BDMAuint timeout = jiffies + WAIT_TICKS;
		while((((ALI_HWDMA_GET_UINT32(BDMA_DMA_CTRL)) & 0x01) != 0) ||
			(((ALI_HWDMA_GET_UINT32(BDMA_CMD_CTRL)) & 0x01) != 0)){
			if(time_after(jiffies, timeout)) {
				printk("BDMA HW cmdq time out...............\n");
		 		return -1;					/*time out*/
		 	}
		}
#endif		
	}
	up(&(dev->hw_sema) );
    return 0;
}

//void dummy(struct ali_dma_directoryEntry* entry, ali_hwdma_job *job) { return; }

void bdma_ops_mode_setup(struct ali_bdma_ops* bdma_ops, bool UsingCmdQ, bool UsingGenTable)
{
	bdma_ops->address_process   = gen_page_table;//bdma_ops->address_process   = (UsingGenTable) ? (gen_page_table)  : (get_from_system);
    bdma_ops->hw_start          = (UsingCmdQ)     ? (ali_dma_cmdIO)   : (ali_dma_registerIO);
	bdma_ops->clean             = ali_dma_clean; //bdma_ops->clean             = (UsingGenTable) ? (ali_dma_clean)   : (dummy);
    bdma_ops->hw_start_2D       = (UsingCmdQ)     ? (ali_dma_cmdIO_2D): (ali_dma_registerIO_2D);

    return;
}

struct ali_bdma_ops bdma_ops = {
#if 1 //#if HWDMA_GENTABLE // Always enable HWDMA_GENTABLE
        .address_process = gen_page_table,
        .clean           = ali_dma_clean,
#else
		.address_process = gen_page_table, // always gen table
		.clean           = ali_dma_clean,
#endif
#if HWDMA_CMDQ_MODE
        .hw_start        = ali_dma_cmdIO,
        .hw_start_2D     = ali_dma_cmdIO_2D,
#else
        .hw_start        = ali_dma_registerIO,
        .hw_start_2D     = ali_dma_registerIO_2D,
#endif
};


