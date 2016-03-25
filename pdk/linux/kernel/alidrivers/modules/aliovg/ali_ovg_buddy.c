
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

#include "ali_ovg_mempool.h"

struct ALiPools aliPoolsID[]={
    //{"FB1", ALI_MM_CODEC_FB_MEM_LEN, ALI_MM_CODEC_FB_START_ADDR},
    {"FB1", 16*1024*1024, ALI_MM_CODEC_FB_START_ADDR},
};
struct list_head pools[ALI_MAX_MEMPOOLS];

#define ALI_SLOT_NUM 13

#include <asm/tlbflush.h>
#include <asm/cacheflush.h>

struct ali_buddy_block {
	struct list_head next;
	bool used;
	unsigned int phy_addr;
	unsigned int size;
	unsigned int kernel_virt_addr;
	unsigned int usr_virt_addr;

};

struct ali_mem_list {
	struct list_head block_list;

};

struct ali_mem_bank {
	struct ali_mem_list* free_list;
	const char* description;
	unsigned int base_addr;
	unsigned int size;
	unsigned int min_order;
	unsigned int max_order;
	
	/*next is used for having multiple reserved memory */
	struct list_head next;

};


unsigned int maximum_order_which_fits(unsigned int size)
{
	unsigned int order = 0;

	if (0 < size)
	{
		for ( order = sizeof(unsigned int)*8 - 1; ((1UL<<order) & size) == 0; --order)
		/* nothing */;

		/* check if size is pow2, if not we need increment order by one */
		if (0 != (size & ((1UL<<order)-1))) ++order;
	}
	return order;
}

void* get_buddy(struct ali_buddy_block* block, struct ali_buddy_block** buddy
	, struct ali_mem_bank* bank, unsigned int index)
{
	struct list_head head = ((struct ali_mem_list*)((bank->free_list)+index))->block_list;
	struct ali_buddy_block *entry, *pre;
	bool found = false;
	unsigned int order = index+bank->min_order; 
	if(  head.next == &head)
	{
		OVG_DBG_PRINTK(KERN_ERR, "No buddy avaliable!, current_order = %d, phy = %x, index = %d \n"
			, block->size, block->phy_addr, index);
	
		return 0;
	}

	unsigned int buddy_phy = (( (block->phy_addr >> PAGE_SHIFT) ^ (1 << index) ) < (block->phy_addr >> PAGE_SHIFT) )?
							block->phy_addr - (1 << order) : block->phy_addr + (1 << order);
	struct list_head* temp;
	for(temp = head.next; (unsigned int)temp != (unsigned int)((bank->free_list)+index)
		; temp = temp->next) {
		entry = list_entry(temp, struct ali_buddy_block, next);
		OVG_DBG_PRINTK(KERN_ERR,"want = %x, Loop got entry phy address = %x\n"
			,buddy_phy, entry->phy_addr);
		if(abs(entry->phy_addr - block->phy_addr) == (1 << order)) {
			found = true;
			
			OVG_DBG_PRINTK(KERN_ERR,"Found!!!, buddy phy = %x , size = %d \n"
				, entry->phy_addr, entry->size);

			return container_of(temp, struct ali_buddy_block, next);
		}	
	}
	return 0;
	
}

void buddy_merge(struct ali_buddy_block *block, struct ali_buddy_block* bank)
{
	unsigned int current_order, index;

	current_order = block->size;
	block->used = false;

	while(current_order <= bank->max_order)
	{
		struct ali_buddy_block* buddy;
		/*Empty for the current slot*/
		index = (current_order>= bank->min_order) ? current_order - bank->min_order : 0;

		buddy = get_buddy(block, &buddy, bank, index);
		if(!buddy)
			break;
		
		OVG_DBG_PRINTK(KERN_ERR,"We got buddy! size = %d, phy = %x, current block = %x \n"
				, buddy->size, buddy->phy_addr, block->phy_addr);
		list_del( &(buddy->next) );
		kfree(buddy);
		block->size = block->size + 1;
		block->phy_addr = (block->phy_addr < buddy->phy_addr) ? block->phy_addr : buddy->phy_addr;
		current_order++;

	}
	index = (current_order>= bank->min_order) ? current_order - bank->min_order : 0;
	list_add(&(block->next), &(((struct ali_mem_list*)((bank->free_list)+(index)))->block_list));


}

void buddy_free(struct ali_memAllocParam* allocParam, void* data)
{
	struct ali_mem_bank* bank;
	struct list_head *head = (struct list_head*)data;
	struct ali_buddy_block *block, *pre;

	bank = container_of(pools[0].next, struct ali_mem_bank, next);

	list_for_each_entry_safe(block, pre, head, next) {
		if(block->phy_addr == allocParam->phy_addr) {
			OVG_DBG_PRINTK(KERN_ERR,"Buffer freed, physical address = %x , size = %d, block size = %d\n"
				, allocParam->phy_addr, allocParam->size, block->size);
			list_del(&(block->next));
			buddy_merge(block, bank);
		}
	}
	
}

unsigned int buddy_alloc(struct ali_openVG_dev* dev, struct file *filp	
,struct ali_memAllocParam* out)
{    
	struct ali_mem_bank* bank;
	struct ali_buddy_block *block;
	unsigned int addr = 0;    
	unsigned int request_order, current_order, index;
	struct ali_openVG_handle* handle = ((struct ali_openVG_handle*)filp->private_data);
	struct list_head *head = handle->mem_list;
	
	bank = container_of(pools[0].next, struct ali_mem_bank, next);
	
  	/* get order of request size*/
	if(maximum_order_which_fits(out->size) >= bank->min_order)
		request_order = maximum_order_which_fits(out->size);
	else
		request_order = bank->min_order;

	for(current_order = request_order ; current_order <= bank->max_order; current_order++) {
		index = (current_order>= bank->min_order) ? current_order - bank->min_order : 0;
		/*Empty for the current slot*/
		if(  (((struct ali_mem_list*)((bank->free_list)+index))->block_list).next == 
			&(((struct ali_mem_list*)((bank->free_list)+index))->block_list))
		{
			OVG_DBG_PRINTK(KERN_ERR,"skipped!, current_order = %d, request_order = %d ,index = %d \n"
				, current_order, request_order, index);
			continue;
		}
		
		OVG_DBG_PRINTK(KERN_ERR,"We can allocate!, current_order = %d, request_order = %d ,index = %d \n"
			, current_order, request_order, index);

		block = container_of((((struct ali_mem_list*)((bank->free_list)+index))->block_list).next 
			, struct ali_buddy_block, next);
		
		OVG_DBG_PRINTK(KERN_ERR,"block is phyaddr = %x , size = %d \n"
			, block->phy_addr, block->size);

		addr = block->phy_addr + (1 << block->size);

		while(current_order > request_order)
		{
			/*get buddy and add to next order list*/
			struct ali_buddy_block* buddy;
			
			current_order--;
			buddy = kmalloc(sizeof(struct ali_buddy_block), GFP_KERNEL);
			buddy->used = false;
			buddy->size = current_order;
			buddy->phy_addr = (block->phy_addr + addr) >> 1;
		

			OVG_DBG_PRINTK(KERN_ERR,"buddy is phyaddr = %x , size = %d \n"
			, buddy->phy_addr, buddy->size);

			index = (current_order>= bank->min_order) ? current_order - bank->min_order : 0;
			list_add(&(buddy->next),&(((struct ali_mem_list*)((bank->free_list)+(index)))->block_list));
			addr = buddy->phy_addr ;
		}

		block->used = true;
		block->size = current_order;
#ifdef	LINUX_VERSION_3_5_2	
		block->usr_virt_addr = vm_mmap(filp, 0, (1 << block->size) , PROT_READ|PROT_WRITE, MAP_SHARED
							, (unsigned int)block->phy_addr);	
#else
		down_write(&current->mm->mmap_sem);
		block->usr_virt_addr = do_mmap(filp, 0, (1 << block->size) , PROT_READ|PROT_WRITE, MAP_SHARED
							, (unsigned int)block->phy_addr);
		up_write(&current->mm->mmap_sem); 
#endif
		out->phy_addr = block->phy_addr;
		out->virt_addr = block->usr_virt_addr;
		//out->size = (1 << block->size);
		
		list_del(&(block->next));

		list_add(&(block->next), head);

		break;		
	}

	if(current_order > bank->max_order)
		return 0;
	return block->phy_addr;
}


void buddy_session_begin(struct file *filp)
{
/*Do not need to handle anything*/
}

void buddy_session_end(struct file *filp)
{
/*Clean resource*/
	struct ali_buddy_block *entry, *pre_entry; 
	struct ali_openVG_handle* handle = ((struct ali_openVG_handle*)filp->private_data);
	struct list_head *head = handle->mem_list;
	struct ali_mem_bank* bank;

	bank = container_of(pools[0].next, struct ali_mem_bank, next);

	if(!list_empty(head)) {
		list_for_each_entry_safe(entry,pre_entry, head, next) {
			OVG_DBG_PRINTK(KERN_ERR,"Buffer freed, physical address = %x , size = %d, block size = %d\n"
				, entry->phy_addr, entry->size, entry->size);
			list_del(&(entry->next));
			buddy_merge(entry, bank);


		}		
	}  
}

bool buddy_chkisValidPhy(unsigned int address , unsigned int size, void* data)
{
	struct ali_buddy_block *entry;
	struct list_head *head = (struct list_head*)data;

	list_for_each_entry(entry, head, next)
		if(entry->phy_addr == address && (1 << entry->size) >= size ) 
			return true;
	return false;
}

unsigned int buddy_getPhy(unsigned int address, void* data)
{
	struct ali_buddy_block *entry, *pre_entry;
	struct list_head *head = (struct list_head*)data;

	list_for_each_entry_safe(entry, pre_entry, head, next)
	{
		if(entry->usr_virt_addr == address) 
			return entry->phy_addr;
	}
	return 0;

}


void initial_buddy(void)
{
/*init memory bank*/
	struct ali_mem_bank	*bank;
	struct ali_buddy_block	*block;
	int i, j;

	for (i=0; i<ALI_MAX_MEMPOOLS; i++) {
		INIT_LIST_HEAD(&pools[i]);
		bank = kmalloc(sizeof(struct ali_mem_bank), GFP_KERNEL);
		bank->description= aliPoolsID[i].name;
		bank->size = aliPoolsID[i].size; 
		bank->base_addr=aliPoolsID[i].phy_addr;
		bank->min_order = maximum_order_which_fits(PAGE_SIZE);
		bank->max_order = maximum_order_which_fits(aliPoolsID[i].size);
		bank->free_list = kmalloc(ALI_SLOT_NUM*sizeof(struct ali_mem_list), GFP_KERNEL);

		for(j = 0; j < ALI_SLOT_NUM ; j++) 
			INIT_LIST_HEAD(&(((struct ali_mem_list*)((bank->free_list)+(j)))->block_list));
		list_add(&(bank->next), &pools[i]);
	}

/*init memory slot distrubution*/	
	block = kmalloc(sizeof(struct ali_buddy_block), GFP_KERNEL);
	block->used = false;
	block->size = maximum_order_which_fits(aliPoolsID[0].size);
	block->phy_addr = bank->base_addr & 0x1FFFFFFF;
	list_add(&(block->next),&(((struct ali_mem_list*)((bank->free_list)+(bank->max_order-bank->min_order)))->block_list));
}

struct ali_ovg_memops memfp = 
{
	.ovg_modelInit = initial_buddy,
	.ovg_session_begin = buddy_session_begin,
	.ovg_session_end = buddy_session_end,
	.ovg_alloc = buddy_alloc,
	.ovg_free = buddy_free,
	.ovg_isValidPhy = buddy_chkisValidPhy,
	.ovg_getPhy = buddy_getPhy,
	
};

