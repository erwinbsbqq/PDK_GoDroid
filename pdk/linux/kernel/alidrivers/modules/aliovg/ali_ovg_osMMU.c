
#include "ali_ovg.h"
#include "ali_ovg_reg.h"

#include <linux/pagemap.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/list.h>

#include <dma-coherence.h>


#include <asm/tlbflush.h>
#include <asm/cacheflush.h>

void os_mmu_modelInit(void)
{
	OVG_DBG_PRINTK(KERN_ERR,"enter \n");
}
void os_mmu_session_begin(struct file *filp)
{
	OVG_DBG_PRINTK(KERN_ERR,"enter \n");
}

void os_mmu_session_end(struct file *filp)
{
	OVG_DBG_PRINTK(KERN_ERR,"enter \n");

}

void os_mmu_mmufree(struct ali_mem_block* block)
{
	dma_free_coherent(NULL, block->size, (void*)block->virt,(dma_addr_t)(block->phy_addr));
	ovg_memory_release_dbg(block->size);
}

_ali_err os_mmu_mmualloc(struct ali_mem_block** block,struct file *filp
	, struct vm_area_struct *vma, unsigned int request_block)
{
	
	//(*block) = kmalloc(sizeof(struct ali_mem_block), GFP_KERNEL |  __GFP_NOWARN | __GFP_NORETRY);
	(*block) = kmalloc(sizeof(struct ali_mem_block), GFP_KERNEL |  __GFP_NOWARN);
	
	ovg_memory_allocate_dbg(sizeof(struct ali_mem_block));
	
	if(!(*block))
		return _ALI_OVG_ERR_FAIL;
	
	(*block)->size = (1 << (request_block + PAGE_SHIFT)); //(1 << VG_MEMORY_BLOCK_MODE);
	(*block)->virt = (unsigned int)ovg_dma_alloc_coherent(NULL, (*block)->size
	, (dma_addr_t*)&((*block)->phy_addr), GFP_KERNEL |  __GFP_NOWARN);
	
	ovg_memory_allocate_dbg((*block)->size);
	
	(*block)->used = 1;
	(*block)->block_free = os_mmu_mmufree;
	(*block)->resourceType = OS_MEMORY;
	
	if((*block)->virt)
		return _ALI_OVG_OK;
	else{
		kfree(*block);
		
		ovg_memory_release_dbg(sizeof(struct ali_mem_block));
		
		if(request_block == 0)
			return _ALI_OVG_ERR_FAIL;
		else
			return _ALI_OVG_ERR_OOM;
	}
		
}


void os_mmu_modelUnload(void)
{


}
void os_mmu_oom_handler(unsigned int* request_size, unsigned int* current_order)
{
	*request_size = PAGE_SIZE;
	*current_order = _ali_OVG_MIN;
}
struct ali_ovg_mmuops os_mmu = 
{
	.ovg_modelInit = os_mmu_modelInit,
	.ovg_modelUnload = os_mmu_modelUnload,
	.ovg_session_begin = os_mmu_session_begin,
	.ovg_session_end = os_mmu_session_end,
	.ovg_mmualloc = os_mmu_mmualloc,
	.ovg_mmufree = os_mmu_mmufree,
	.ovg_mmuoom_handler = os_mmu_oom_handler,
};

#ifdef MEMORY_POOL_OS
struct ali_ovg_mmuAllocator os_allocator = /* os_allocator= */
#else
struct ali_ovg_mmuAllocator mmu_allocator =
#endif
{
	.ops = &os_mmu,
	.bank = NULL,
	.next = NULL,

};



