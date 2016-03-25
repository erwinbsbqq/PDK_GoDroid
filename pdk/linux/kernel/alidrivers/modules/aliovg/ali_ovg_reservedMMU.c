
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

#include "ali_ovg_platform.h"



unsigned int bank_num = _ali_OVG_MAX - _ali_OVG_MIN + 1;
extern struct ali_ovg_mmuAllocator os_allocator;
extern struct ali_ovg_mmuSubsystem mmuSubsystem;

struct ali_ovg_memBank reserved_bank;

struct list_head dirtyList;
#ifdef	LINUX_VERSION_3_5_2
static DEFINE_SPINLOCK(ovg_lock);
#else
spinlock_t ovg_lock = SPIN_LOCK_UNLOCKED;
#endif

void reserved_modelInit(void)
{
	unsigned int bank_size = OVG_BOARD_GET_WORD(aliPoolsID[0].size) - MEMPOOL_MEMORY_BANK_LEN;
	unsigned int block_size = bank_size>> VG_MEMORY_BLOCK_MODE;
	//struct ali_mem_block *allBlock = kmalloc(block_size * sizeof(struct ali_mem_block), GFP_KERNEL |  __GFP_NOWARN | __GFP_NORETRY);
	struct ali_mem_block *allBlock = kmalloc(block_size * sizeof(struct ali_mem_block), GFP_KERNEL |  __GFP_NOWARN);
	unsigned int index = 0;
	struct ali_model_dirtyList *dirty;

	if(!allBlock){
		OVG_PRINTK("kmalloc fail \n");
		return;
	}
	
	/*1) init memory bank*/
	reserved_bank.name = "FB1-mmu usage";
	reserved_bank.size = bank_size;
	reserved_bank.start_address = 
		(OVG_BOARD_GET_WORD(aliPoolsID[0].phy_addr) & 0x1FFFFFFF) + MEMPOOL_MEMORY_BANK_LEN;
	OVG_DBG_PRINTK(KERN_ERR,"bank size 1= %d \n", bank_num);
	for(index = 0 ; index < bank_num ; index++)
		INIT_LIST_HEAD(&(reserved_bank.first_free[index]));
	INIT_LIST_HEAD(&dirtyList);
	OVG_DBG_PRINTK("bank size 2= %d \n", bank_num);
	/*2) create first_free, and block list*/
	index = 0;
	while(index < block_size){
		OVG_DBG_PRINTK(KERN_ERR,"index = %d \n", index);
		allBlock[index].phy_addr = reserved_bank.start_address + index*(1 << VG_MEMORY_BLOCK_MODE);
		allBlock[index].size = (1 << VG_MEMORY_BLOCK_MODE);
		allBlock[index].used = 0;
		allBlock[index].resourceType = RESERVED_MEMORY;
		list_add(&(allBlock[index].next), &(reserved_bank.first_free[bank_num-1]));
		index++;
	}

	OVG_DBG_PRINTK(KERN_ERR,"enter, index = %d, init done. start address = %x \n", index, reserved_bank.start_address);
	//dirty = kmalloc(sizeof(struct ali_model_dirtyList), GFP_KERNEL |  __GFP_NOWARN | __GFP_NORETRY);
	dirty = kmalloc(sizeof(struct ali_model_dirtyList), GFP_KERNEL |  __GFP_NOWARN);
	if(!dirty){
		OVG_PRINTK("terrible kmalloc fail \n");
		return;
	}
	dirty->address = (unsigned int)allBlock;
	list_add(&(dirty->mem_list), &dirtyList);

	spin_lock_init(&ovg_lock);

	//OVG_PRINTK("enter %x,  \n", (reserved_bank.first_free)->phy_addr, );
}
void reserved_session_begin(struct file *filp)
{
	OVG_DBG_PRINTK(KERN_ERR,"enter \n");
}

void reserved_session_end(struct file *filp)
{
	OVG_DBG_PRINTK(KERN_ERR,"enter \n");

}

unsigned int get_sizeOrder(unsigned int size)
{
	unsigned int order = (size >> PAGE_SHIFT);
	if(order == 512)
		return 9;
	else if(order == 256)
		return 8;
	else if(order == 128)
		return 7;
	else if(order == 64)
		return 6;
	else if(order == 32)
		return 5;
	else if(order == 16)
		return 4;
	else if(order == 8)
		return 3;
	else if(order == 4)
		return 2;
	else if(order == 2)
		return 1;

	return 0;


}

void reserved_mmufree(struct ali_mem_block* block)
{
	unsigned int size_mode = get_sizeOrder(block->size);
	list_add_tail(&(block->next), &(reserved_bank.first_free[size_mode]));
	OVG_DBG_PRINTK(KERN_ALERT,"freed block address = %x \n", block->phy_addr);
}

static volatile unsigned long shared_flag = 0;

int sys_test_and_set(void)
{	
	while(test_and_set_bit(0, &shared_flag));
		//schedule();	

	return 0;
}
 
int  sys_test_and_clear(void)
{	
	if(test_and_clear_bit(0, &shared_flag) == 0)		
		printk("test_and_clear error, already set 0 \n");	
	return 0;

}
_ali_err get_more_free_blocks(unsigned int size_mode)
{
	unsigned int temp_mode = size_mode;
	unsigned int block_size = (1 << (size_mode + PAGE_SHIFT));
	
	struct ali_mem_block *block;
	unsigned int block_count;
	struct ali_mem_block *allBlock;
	struct ali_model_dirtyList *dirty;
	
	//OVG_PRINTK("size_mode = %d, block size = %d  \n", size_mode, block_size);

	while((temp_mode+1) < bank_num)
	{
		unsigned int index = 0;
		temp_mode++;
		if(list_empty(&(reserved_bank.first_free[temp_mode])))
			continue;
		block = list_first_entry(&(reserved_bank.first_free[temp_mode]), struct ali_mem_block, next);
		list_del(&(block->next));
		block_count = ( (block->size) / block_size);
		
		//allBlock = kmalloc(block_count * sizeof(struct ali_mem_block), GFP_KERNEL |  __GFP_NOWARN | __GFP_NORETRY);
		allBlock = kmalloc(block_count * sizeof(struct ali_mem_block), GFP_KERNEL |  __GFP_NOWARN);
		if(!allBlock){
			OVG_PRINTK("kmalloc fail \n");
			break;
		}
		
		ovg_memory_allocate_dbg(block_count * sizeof(struct ali_mem_block));	

		while(index < block_count){
			allBlock[index].phy_addr = (block->phy_addr) + index*block_size;
			allBlock[index].size = block_size;
			allBlock[index].used = 0;
			allBlock[index].resourceType = RESERVED_MEMORY;
			list_add(&(allBlock[index].next), &(reserved_bank.first_free[size_mode]));
			index++;
		}

		//dirty = kmalloc(sizeof(struct ali_model_dirtyList), GFP_KERNEL |  __GFP_NOWARN | __GFP_NORETRY);
		dirty = kmalloc(sizeof(struct ali_model_dirtyList), GFP_KERNEL |  __GFP_NOWARN);
		if(!dirty){
			OVG_PRINTK("kmalloc fail\n");
			break;
		}	
		
		ovg_memory_allocate_dbg(sizeof(struct ali_model_dirtyList));
		
		dirty->address = (unsigned int)allBlock;
		list_add(&(dirty->mem_list), &dirtyList);

		
		return _ALI_OVG_OK;
		
	}
	return _ALI_OVG_ERR_FAIL;
}
_ali_err  reserved_mmualloc(struct ali_mem_block** block,struct file *filp
	, struct vm_area_struct *vma, unsigned int size_mode)
{

	spin_lock(&ovg_lock);
	if(list_empty(&(reserved_bank.first_free[size_mode]))){
		if((size_mode== (_ali_OVG_MAX-1)) 
			|| get_more_free_blocks(size_mode)){
			spin_unlock(&ovg_lock);
			if(size_mode==0)
				return _ALI_OVG_ERR_FAIL;
			else
				return _ALI_OVG_ERR_OOM;
		}	
	}	
	*block = list_first_entry(&(reserved_bank.first_free[size_mode]), struct ali_mem_block, next);
	list_del(&((*block)->next));
	(*block)->block_free = reserved_mmufree;
	(*block)->resourceType = RESERVED_MEMORY;
	spin_unlock(&ovg_lock);
	return _ALI_OVG_OK;
}

void reserved_modelUnload(void)
{
	struct ali_model_dirtyList *entry, *pre_entry;
	list_for_each_entry_safe(entry, pre_entry, &dirtyList, mem_list){
		kfree((void*)entry->address);
		list_del(&(entry->mem_list));
		kfree(entry);
	}

}

void reserved_oom_handler(unsigned int* request_size, unsigned int* current_order)
{
	*current_order = *current_order - 1;
	*request_size = (1 << (*current_order));
}

struct ali_ovg_mmuops reserved_mmu = 
{
	.ovg_modelInit = reserved_modelInit,
	.ovg_modelUnload = reserved_modelUnload,
	.ovg_session_begin = reserved_session_begin,
	.ovg_session_end = reserved_session_end,
	.ovg_mmualloc = reserved_mmualloc,
	.ovg_mmufree = reserved_mmufree,
	.ovg_mmuoom_handler = reserved_oom_handler,
};


struct ali_ovg_mmuAllocator mmu_allocator = 
{
	.ops = &reserved_mmu,
	.bank =&reserved_bank ,
	.next = &os_allocator, //NULL,//
};



