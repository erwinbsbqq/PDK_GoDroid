
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
#include <linux/string.h>

#include <dma-coherence.h>

#include "ali_ovg_mempool.h"
#include "ali_ovg_platform.h"

//#define OVG_CLOSE_ISSUE

struct list_head pools[ALI_MAX_MEMPOOLS];
extern struct ali_ovg_memops os_memfp;
#if MMU_ENABLED
extern struct ali_ovg_memBank reserved_bank;
#endif
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>

void merge(struct ali_mempool_desc *desc, struct ali_mempool_desc *neighbor)
{	
	neighbor->size += desc->size;	
	neighbor->phys = neighbor->phys < desc->phys ? neighbor->phys : desc->phys;	
	list_del(&desc->list);
}

void ali_mempool_free(int pool, struct ali_memAllocParam *allocParam)
{	
	struct ali_mempool_desc* desc = (struct ali_mempool_desc*)allocParam->desc;	
	struct ali_mempool_desc* neighbor;	
	struct list_head *tmp;	

	desc->in_use = 0;	
	/*	 
	* Free up the provided region's descriptor and check to see if it can	
	* be merged with any of its neighbors.  This gives us a better chance	 
	* of having a large enough buffer for the next allocation.	 
	*/	
	tmp = desc->list.prev;	
	if (tmp != &pools[pool]) {		
		neighbor = list_entry(tmp, struct ali_mempool_desc, list);		
		if (!neighbor->in_use) {			
			merge(desc, neighbor);			
			kfree(desc);		
			
			ovg_memory_release_dbg(sizeof(struct ali_mempool_desc));
				
			allocParam->desc = NULL;			
			desc = neighbor;		
			}	
		}	
	tmp = desc->list.next;
	if (tmp != &pools[pool]) {		
		neighbor = list_entry(tmp, struct ali_mempool_desc, list);		
		if (!neighbor->in_use) {			
			merge(desc, neighbor);			
			kfree(desc);	
			
			ovg_memory_release_dbg(sizeof(struct ali_mempool_desc));
					
			allocParam->desc = NULL;		
		}	
	}
}

void ali_mempool_release(int pool,struct ali_memAllocParam* allocParam)
{    
  	struct ali_mempool_desc *match_desc, *tmp;    
	struct ali_memAllocParam alloc;    

	/* search all list in pool, and fine the same phy_addr*/ 
	
	list_for_each_entry_safe(match_desc, tmp, &pools[pool], list){    
		if (((unsigned int)match_desc->phys & 0x1FFFFFFF)== allocParam->phy_addr){   

			OVG_DBG_PRINTK(KERN_ALERT, "free buffer from pool[%s], phy=0x%x, user virtual=0x%x  size =%d \n"
				, aliPoolsID[pool].name, allocParam->phy_addr, match_desc->user_virt, match_desc->size);  
			
			match_desc->in_use = 0;            
			alloc.desc = match_desc;    
			ali_mempool_free(pool, &alloc);  
			down_write(&current->mm->mmap_sem);
			do_munmap(current->mm, match_desc->user_virt, match_desc->size);
			up_write(&current->mm->mmap_sem);
			break;        
		}    
	}
}

void ali_mempool_clean(struct ali_memAllocParam* allocParam, void* data)
{
	int i=0;            
	
	if(!allocParam->name)
	{
		OVG_PRINTK("Error : ali_memAllocParam has NULL name\n");
		return;
	}
	for (i=0; i<ALI_MAX_MEMPOOLS; i++){                
		if (!strcmp(allocParam->name, aliPoolsID[i].name))                    
			break;            
	}
	if(i < ALI_MAX_MEMPOOLS)
		ali_mempool_release(i, allocParam);    
	else
		OVG_PRINTK("Error : free buffer from pool fail, phy=0x%x, size = %d, name = %s \n"
				, (unsigned int)allocParam->phy_addr, allocParam->size, allocParam->name);
}

int ali_mempool_alloc(int pool, struct ali_memAllocParam* out, struct file *filp){
	struct ali_mempool_desc *first_free;	
	struct ali_mempool_desc *new_desc;	
	bool found = false;    
	/* check 16 byte */    
	if (out->size % (4*1024) != 0){        
		int size = ((out->size>>12)+1)<<12;        
		out->size = size;    
	}	
	
	list_for_each_entry(first_free, &pools[pool], list) {		
		if (!first_free->in_use && first_free->size >= out->size) {			
			found = true;			
			break;		
		}	
	}	
	if (!found)		
		return -ENOMEM;	
	/*	 
	* If we get here then we have a good pool.  Split it if the allocation	 
	* does not use all of it.	 
	*/	
	
	if (first_free->size > out->size) {		
		new_desc = kmalloc(sizeof(new_desc), GFP_KERNEL);
		
		ovg_memory_allocate_dbg(sizeof(new_desc));
		
		new_desc->name = first_free->name;		
		new_desc->size = first_free->size - out->size;		
		new_desc->in_use = 0;		
		new_desc->phys = first_free->phys + out->size;		
		new_desc->private_data = 0;
		list_add(&new_desc->list, &first_free->list);		
		first_free->size = out->size;	
	}    
	
	first_free->in_use = true;    
	out->phy_addr = (unsigned int)
	first_free->phys & 0x1FFFFFFF; 

#ifdef LINUX_VERSION_3_5_2
	first_free->user_virt = vm_mmap(filp, 0, out->size , PROT_READ|PROT_WRITE, MAP_SHARED
							, (unsigned int)out->phy_addr);
#else 
	down_write(&current->mm->mmap_sem);
	first_free->user_virt = do_mmap(filp, 0, out->size , PROT_READ|PROT_WRITE, MAP_SHARED
							, (unsigned int)out->phy_addr);
	up_write(&current->mm->mmap_sem);
#endif
	
	first_free->private_data = (unsigned int)filp;
	out->desc = (void *)first_free;    
	out->name = first_free->name;	
	out->virt_addr = first_free->user_virt;
	
	return 0;
}

#if MMU_ENABLED
unsigned int ovg_poolalloc(struct ali_openVG_dev* dev, struct file *filp	
, struct ali_memAllocParam* out){    
	unsigned int addr = 0;    
	int i=0;    // check if memory pool is available to use.    
	int request_block_size = 0;

	int current_order;
	struct ali_mem_block* block = NULL;	
	struct vm_area_struct * vma;
	_ali_err err;	
	//unsigned int virtual_addr = vma->vm_start;
	struct ali_openVG_handle* handle = ((struct ali_openVG_handle*)filp->private_data);
	struct ali_mem_buffer *buffer = handle->buffer_temp;	
	vma = find_vma(current->mm, addr);	

	current_order = _ali_OVG_MIN;
	while(out->size > request_block_size){
		current_order++;			
		request_block_size =(1 << (current_order));
	}	


	if(!buffer){
		//buffer = kmalloc(sizeof(struct ali_mem_buffer), GFP_KERNEL |  __GFP_NOWARN | __GFP_NORETRY);
		buffer = kmalloc(sizeof(struct ali_mem_buffer), GFP_KERNEL |  __GFP_NOWARN);
		
		ovg_memory_allocate_dbg(sizeof(struct ali_mem_buffer));
		
		buffer->size = request_block_size;
		buffer->mapped_size= request_block_size;
		//buffer->cpu_ptr = virtual_addr;
	
		if( (buffer->size & (PAGE_SIZE-1)) != 0)
			buffer->size = ((buffer->size + PAGE_SIZE) & ~(PAGE_SIZE-1));

		if( (buffer->mapped_size & (PAGE_SIZE-1)) != 0)
			buffer->mapped_size = ((buffer->mapped_size + PAGE_SIZE) & ~(PAGE_SIZE-1));	

		INIT_LIST_HEAD(&(buffer->physical_list));
		list_add(&(buffer->next), handle->allocated_list);

	}
	
	
	if (!down_interruptible(&(dev->mem_sema))){
		for (i=0; i< ALI_MAX_MEMPOOLS; i++) {  
			err = reserved_mmualloc(&block, filp, vma, (current_order  - _ali_OVG_MIN ));			
//printk("end of reserved_mmualloc err = %08x\n",err);			
			if(err == _ALI_OVG_OK){
				//os_address_map(virtual_addr, &block, vma);
				//ovg_address_map(virtual_addr, &block, handle->pdt);
#ifdef	LINUX_VERSION_3_5_2
				buffer->cpu_ptr = vm_mmap(filp, 0, request_block_size , PROT_READ|PROT_WRITE, MAP_SHARED
							, block->phy_addr);
#else				
				down_write(&current->mm->mmap_sem);
				buffer->cpu_ptr = do_mmap(filp, 0, request_block_size , PROT_READ|PROT_WRITE, MAP_SHARED
							, block->phy_addr);
				up_write(&current->mm->mmap_sem);
#endif							
				list_add(&(block->next), &(buffer->physical_list));
//printk("end of list_add(&(block->next), &(buffer->physical_list));");
			}else
				return 0;

			//addr = out->phy_addr | 0xA0000000;
			addr = block->phy_addr | 0xA0000000;                
//printk("Physical allocate addr =%08x\n",addr);
		}
		up(&(dev->mem_sema));    
	}

	out->size = request_block_size;
	out->phy_addr = block->phy_addr;
	out->virt_addr = block->virt;	
    out->desc = (void *)buffer;
	return addr;

}
#else
unsigned int ovg_poolalloc(struct ali_openVG_dev* dev, struct file *filp	
, struct ali_memAllocParam* out){    

	unsigned int addr = 0;    
	int i=0;    // check if memory pool is available to use.    
	if (!down_interruptible(&(dev->mem_sema)))
        {        
		for (i=0; i< ALI_MAX_MEMPOOLS; i++) {  
			
			if (!ali_mempool_alloc(i, out, filp)){                
				addr = out->phy_addr | 0xA0000000;                
				OVG_DBG_PRINTK(KERN_ALERT,"alloc buffer from pool[%s], phy=0x%x , virt = %x \n", 
					aliPoolsID[i].name,  out->phy_addr, out->virt_addr);                
				break;            
			}        
		}             
		up(&(dev->mem_sema));    
	}    
	return addr;
}
#endif

void pool_session_begin(struct file *filp)
{
/*DO not handle anything here*/
#ifdef OVG_CLOSE_ISSUE
	struct ali_mempool_desc *match_desc, *tmp;   
	struct ali_memAllocParam alloc;    
	int i=0;

	OVG_DBG_PRINTK(KERN_ERR,"session begin to free buffer from pool\n");

	for (i=0; i<ALI_MAX_MEMPOOLS; i++){
		list_for_each_entry_safe(match_desc, tmp, &pools[i] , list){
			if(match_desc->size == MEMPOOL_MEMORY_BANK_LEN)
				break; 				// init case 
			match_desc->in_use = 0;            
			alloc.desc = match_desc;            
			ali_mempool_free(i, &alloc);   
			OVG_DBG_PRINTK(KERN_ERR,"session begin free buffer from pool[%s], phy=0x%x, size = %d\n"
				, aliPoolsID[i].name, (unsigned int)match_desc->phys, match_desc->size);           
		}    
	}
#endif
}

void pool_session_end(struct file *filp)
{
#ifndef OVG_CLOSE_ISSUE
	struct ali_mempool_desc *match_desc, *tmp;   
	struct ali_memAllocParam alloc;    
	int i=0;

	OVG_DBG_PRINTK(KERN_ERR,"session end to free buffer from pool, memory range from %x , to %x \n"
		, aliPoolsID[0].phy_addr, aliPoolsID[0].phy_addr+aliPoolsID[0].size);

	for (i=0; i<ALI_MAX_MEMPOOLS; i++){
		list_for_each_entry_safe(match_desc, tmp, &pools[i] , list){
			if(match_desc->size ==MEMPOOL_MEMORY_BANK_LEN)
				break; 				// init case 

			/*Check if this memory block in the current process*/
			if((match_desc->private_data != (unsigned int)filp) &&
				(match_desc->private_data != 0) )
				continue;
			match_desc->in_use = 0;            
			alloc.desc = match_desc;            
			ali_mempool_free(i, &alloc);   
			OVG_DBG_PRINTK(KERN_ERR,"session end free buffer from pool phy=0x%x, size = %d\n"
				, (unsigned int)match_desc->phys, match_desc->size);           
		}    
	}
#endif
}

bool isValiPhyFromPool(unsigned int address , unsigned int size, void* data)
{
	struct ali_mempool_desc *match_desc;   
	
	int i=0;
	for (i=0; i<ALI_MAX_MEMPOOLS; i++){
		list_for_each_entry(match_desc, &pools[i] , list){
				OVG_DBG_PRINTK(KERN_ERR,"alloc buffer from %x, phy=0x%x \n", 
					(unsigned int)match_desc->phys,  address);         
				if(((unsigned int)match_desc->phys & 0x1FFFFFFF) == address 
					&& match_desc->size >= size ) 
					return true;
			}
	}
	
	return false;
}

unsigned int getPhyFromPool(unsigned int address, void* data)
{
	struct ali_mempool_desc *match_desc;   
	
	int i=0;
	for (i=0; i<ALI_MAX_MEMPOOLS; i++){
		list_for_each_entry(match_desc, &pools[i] , list)
			if(match_desc->user_virt == address) 
				return (unsigned int)match_desc->phys;
	}
	return 0;
}


void initial_pools(void)
{
	struct ali_mempool_desc *new_des;
	int i;

extern unsigned int g_ali_ovg_phy_addr;
extern unsigned int g_ali_ovg_mem_size;
	aliPoolsID[0].phy_addr = g_ali_ovg_phy_addr;
	aliPoolsID[0].size     = g_ali_ovg_mem_size;

	for (i=0; i<ALI_MAX_MEMPOOLS; i++){
		INIT_LIST_HEAD(&pools[i]);
		new_des = kmalloc(sizeof(new_des), GFP_KERNEL);
		new_des->name = aliPoolsID[i].name;
		new_des->size = aliPoolsID[i].size;
		new_des->phys = (void *)OVG_BOARD_GET_WORD(aliPoolsID[i].phy_addr);
		new_des->in_use = 0;
		new_des->private_data = 0;
		list_add(&new_des->list, &pools[i]);
	}
	OVG_PRINTK("model init  length = %x, start addr = %x \n"
		,  (unsigned int)new_des->size, (unsigned int)new_des->phys);
    
}

struct ali_ovg_memops memfp = 
{
	.ovg_modelInit = initial_pools,
	.ovg_session_begin = pool_session_begin,
	.ovg_session_end = pool_session_end,
	.ovg_alloc = ovg_poolalloc,
	.ovg_free = ali_mempool_clean,
	.ovg_isValidPhy = isValiPhyFromPool,
	.ovg_getPhy = getPhyFromPool,
	.next = NULL,
};
/*
void memPool_modelInit(void)
{

}

void memPool_modelUnload(void)
{


}

void memPool_session_begin(struct file *filp)
{
	OVG_PRINTK("enter \n");
}

void memPool_session_end(struct file *filp)
{
	OVG_PRINTK("enter \n");

}
_ali_err  memPool_alloc(struct ali_mem_block** block,struct file *filp
	, struct vm_area_struct *vma, unsigned int request_block)
{

	return _ALI_OVG_ERR_FAIL;
}
void memPool_free(struct ali_mem_block* block)
{


}
struct ali_ovg_mmuops memPool_ops = 
{
	.ovg_modelInit = memPool_modelInit,
	.ovg_modelUnload = memPool_modelUnload,
	.ovg_session_begin = memPool_session_begin,
	.ovg_session_end = memPool_session_end,
	.ovg_mmualloc = memPool_alloc,
	.ovg_mmufree = memPool_free,
};


struct ali_ovg_mmuAllocator memPool_allocator= 
{
	.ops = &memPool_ops,
	.bank =&memPool_bank ,
	.next = &memOs_allocator,

};
*/


