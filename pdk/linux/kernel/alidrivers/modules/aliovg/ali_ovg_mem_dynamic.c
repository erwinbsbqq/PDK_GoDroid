
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


struct mem_entry {
	unsigned int phy_addr;
	unsigned int size;
	unsigned int kernel_virt_addr;
	unsigned int usr_virt_addr;
	struct list_head next;
      const char *poolName;
};



void *ovg_dma_alloc_coherent(struct device *dev, size_t size,dma_addr_t * dma_handle, gfp_t gfp);
unsigned int ovg_allocateBuffer(struct ali_openVG_dev* dev,struct ali_memAllocParam* out, bool* reserved);
void ovg_clean(struct mem_entry *entry);

void ovg_add2List(unsigned int addr_kernel, struct file *filp
	, struct ali_memAllocParam* out)
{
	struct ali_openVG_handle* handle = ((struct ali_openVG_handle*)filp->private_data);
	struct list_head *head = handle->mem_list;
	struct mem_entry* info;

#ifdef	LINUX_VERSION_3_5_2	
	out->virt_addr = vm_mmap(filp, 0, out->size , PROT_READ|PROT_WRITE, MAP_SHARED
							, out->phy_addr);	
#else
	down_write(&current->mm->mmap_sem);
	out->virt_addr = do_mmap(filp, 0, out->size , PROT_READ|PROT_WRITE, MAP_SHARED
							, out->phy_addr);	
	up_write(&current->mm->mmap_sem); 
#endif							
	info = kmalloc(sizeof(struct mem_entry), GFP_KERNEL);
	ovg_memory_allocate_dbg(sizeof(struct mem_entry));

	if(info) {
		info->phy_addr = out->phy_addr;
		info->size = out->size;
		info->kernel_virt_addr = addr_kernel;
		info->usr_virt_addr = out->virt_addr;
		info->poolName = out->name;
		list_add(&(info->next), head);
	}

}

unsigned int ovg_dynamicalloc(struct ali_openVG_dev* dev, struct file *filp	
,struct ali_memAllocParam* out)
{	
	unsigned int addr = 0;	
	
	addr = (unsigned int)ovg_dma_alloc_coherent(NULL, out->size
	, (dma_addr_t*)&(out->phy_addr), GFP_KERNEL |  __GFP_NOWARN);
	
	ovg_memory_allocate_dbg(out->size);

	OVG_DBG_PRINTK(KERN_EMERG,"dynamic allocation,allocated addr = %x,  phy = %x, size = %d\n"
		, addr, out->phy_addr, out->size);

	ovg_add2List(addr, filp, out);
	return addr;
}


void ovg_clean(struct mem_entry* entry)
{
	do_munmap(current->mm, entry->usr_virt_addr, entry->size);
	dma_free_coherent(NULL, entry->size, (void*)entry->kernel_virt_addr,(dma_addr_t)(entry->phy_addr));
	
	OVG_DBG_PRINTK(KERN_ERR,"buffer freed, addr = %x,  phy = %x, size = %d\n"
		, entry->usr_virt_addr, entry->phy_addr, entry->size);
		
	ovg_memory_release_dbg(entry->size);
}


void ovg_freeMem(struct ali_memAllocParam* allocParam, void* data)
{
	struct mem_entry *entry, *pre_entry; 
	struct list_head *head = (struct list_head*)data;
	
	list_for_each_entry_safe(entry, pre_entry, head, next) {
		if(entry->phy_addr == allocParam->phy_addr) {
			ovg_clean(entry);
			list_del(&(entry->next));
			kfree(entry);
			ovg_memory_release_dbg(sizeof(struct mem_entry));
		}
	}

}

void dynamic_session_begin(struct file *filp)
{
/*DO not need to handle anything*/
}

void dynamic_session_end(struct file *filp)
{
/*Clear dynamic buufer list*/
	struct mem_entry *entry, *pre_entry; 
	struct ali_openVG_handle* handle = ((struct ali_openVG_handle*)filp->private_data);
	struct list_head *head = handle->mem_list;

	if(!list_empty(head)) {
		list_for_each_entry_safe(entry,pre_entry, head, next) {
			ovg_clean(entry);
			list_del(&(entry->next));
			kfree(entry);
			ovg_memory_release_dbg(sizeof(struct mem_entry));
		}		
	}  
}

bool isValiPhy(unsigned int address , unsigned int size, void* data)
{
	struct mem_entry *entry;
	struct list_head *head = (struct list_head*)data;

	list_for_each_entry(entry, head, next)
		if(entry->phy_addr == address && entry->size >= size ) 
			return true;
	return false;
}
unsigned int getPhy(unsigned int address, void* data)
{
	struct mem_entry *entry, *pre_entry;
	struct list_head *head = (struct list_head*)data;

	list_for_each_entry_safe(entry, pre_entry, head, next)
	{
		if(entry->usr_virt_addr == address) 
			return entry->phy_addr;
	}
	return 0;
}

void memInit(void)
{
/*
	openVG_cache = kmem_cache_create("openVG", sizeof(struct mem_entry), 0,0, NULL);
	unsigned int index;

	for(index=0; index < MEM_BUF_SIZE ; index++)
	{
		memory_array[index].phy_addr = ALI_MM_CODEC_FB_START_ADDR + index*MEM_SLOT_SIZE;
		memory_array[index].in_use = NO_USED;
	}*/
}


struct ali_ovg_memops memfp = 
{
	.ovg_modelInit = memInit,
	.ovg_session_begin = dynamic_session_begin,
	.ovg_session_end = dynamic_session_end,		
	.ovg_alloc = ovg_dynamicalloc,
	.ovg_free = ovg_freeMem,
	.ovg_isValidPhy = isValiPhy,
	.ovg_getPhy = getPhy,
	.next = NULL,

};
/*

void memPool_modelInit(void)
{

}

void memPool_modelUnload(void)
{


}

void memOS_session_begin(struct file *filp)
{
	OVG_PRINTK("enter \n");
}

void memOS_session_end(struct file *filp)
{
	OVG_PRINTK("enter \n");

}
_ali_err  memOS_alloc(struct ali_mem_block** block,struct file *filp
	, struct vm_area_struct *vma, unsigned int request_block)
{

	return _ALI_OVG_ERR_FAIL;
}
void memOS_free(struct ali_mem_block* block)
{


}
struct ali_ovg_mmuops memOS_ops = 
{
	.ovg_modelInit = memOS_modelInit,
	.ovg_modelUnload = memOS_modelUnload,
	.ovg_session_begin = memOS_session_begin,
	.ovg_session_end = memOS_session_end,
	.ovg_mmualloc = memOS_alloc,
	.ovg_mmufree = memOS_free,
};


struct ali_ovg_mmuAllocator memOS_allocator= 
{
	.ops = &memOS_ops,
	.bank =NULL,
	.next = NULL,

};
*/



