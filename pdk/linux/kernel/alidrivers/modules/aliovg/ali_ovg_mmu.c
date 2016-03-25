
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

extern struct ali_ovg_mmuSubsystem mmuSubsystem;

void ovg_vm_open(struct vm_area_struct * vma)
{
}
void ovg_vm_close(struct vm_area_struct * vma)
{
	struct ali_mem_buffer *buffer, *pre_buffer; 
	struct ali_mem_block *block, *pre_block; 
	struct list_head *head = (struct list_head *)vma->vm_private_data;
	
	list_for_each_entry_safe(buffer, pre_buffer, head, next) {
		//if((buffer->cpu_ptr== vma->vm_start) && (buffer->pid == current->pid)) {
		if((buffer->cpu_ptr== vma->vm_start) && (buffer->tgid == current->tgid)) {
			list_for_each_entry_safe(block, pre_block, &(buffer->physical_list), next){
				list_del(&(block->next));
				block->block_free(block);				
				if(block->resourceType == OS_MEMORY){
					kfree(block);
					ovg_memory_release_dbg(sizeof(struct ali_mem_block));
				}	
			}
			list_del(&(buffer->next));

			kfree(buffer);
			ovg_memory_release_dbg(sizeof(struct ali_mem_buffer));
			break;
		}
	}
}

struct vm_operations_struct ovg_vm_ops = 
{
 .open = ovg_vm_open,
 .close = ovg_vm_close,

};

pmd_t fillPageDirectory(int i, unsigned int virt_temp, unsigned int* pgd_tbladdr)
{
	/*fill page directory*/

	unsigned int addr;
	pgd_t *pgdp;	
	pud_t *pudp;	
	pmd_t *pmdp;	

	if (!(*(pgd_tbladdr+i) & _PAGE_PRESENT)) {
		unsigned int *temp;
		temp = dma_alloc_coherent(NULL, PAGE_SIZE, &addr, GFP_KERNEL);
		ovg_memory_allocate_dbg(PAGE_SIZE);
		memset(temp , 0, PAGE_SIZE);
		pgd_tbladdr[i] =addr | _PAGE_PRESENT;		
		OVG_DBG_PRINTK(KERN_ERR, "Allocated one page table = %x , index = %d , phy addr = %x\n"
			, (unsigned int)temp, i, addr);
	}

	pgdp = pgd_offset(current->mm, virt_temp );
	pudp = pud_offset(pgdp, virt_temp);				
	pmdp = pmd_offset(pudp, virt_temp);	
	return *pmdp;

}

void fillPageTable(int j, pmd_t * pmdp, UINT32 virt_temp, UINT32* pte_tbladdr)
{
	unsigned int tbladdr_temp;
	pte_t *ptep;
#ifdef PAGE_ADDRESS_FIX
	struct page* pageX;
	ptep = pte_offset_kernel(pmdp, virt_temp);				
	pageX = pte_page(*ptep);
#else		
	ptep = pte_offset(pmdp, virt_temp);
#endif

	if(pte_tbladdr[j] != 0)
		OVG_DBG_PRINTK(KERN_ERR, "ERROR!!!!! fillPageTable get a entry is not empty\n");
	
#ifdef PAGE_ADDRESS_FIX	
	tbladdr_temp = page_to_phys(pageX);
#else
	tbladdr_temp = (UINT32)pte_val(*ptep);
#endif
	if(tbladdr_temp > 0x18000000) tbladdr_temp -= 0x20000000;
	*(pte_tbladdr + j) = tbladdr_temp;
	OVG_DBG_PRINTK(KERN_ERR, " address = %x ,pte index = %d, pte table addr = %x ,pte_val = %x, table = %x\n"
		, virt_temp ,j , (unsigned int)pte_tbladdr, *(pte_tbladdr + j), pte_tbladdr[j] );

}

void ovg_mmu_table_aliVirtual(unsigned int virt_addr, unsigned int length
	, unsigned int mapped_virt, unsigned int* pgd)
{

	unsigned int first_pde, last_pde, virt_temp, virt_fill_addr;
	unsigned int first_pte, last_pte; 

	int i,j, times;
	pmd_t pmd;
	
	times = 0;
	OVG_DBG_PRINTK(KERN_ALERT, "address = %x , size = %d, mapped_virt = %x \n", virt_addr, length, mapped_virt);	
	
	while(times < 2){
		virt_temp = mapped_virt;
		virt_fill_addr = virt_addr + times*length;
		first_pde = pgd_index(virt_fill_addr);
		last_pde = pgd_index(virt_fill_addr + length - 1);
		
		for (i = first_pde; i <= last_pde ; i++) {
			pmd = fillPageDirectory(i, virt_temp, pgd);
			
			first_pte = __pte_offset(virt_fill_addr);
			last_pte = (i < last_pde) ? 1023 : __pte_offset(virt_fill_addr+length - 1);
			
			OVG_DBG_PRINTK(KERN_ALERT, " address = %x , fill addr=%x,pgd = %d, first pte  = %d , last pte = %d\n"
				, virt_temp, virt_fill_addr, i,  first_pte, last_pte);	
			
			for (j=first_pte ; j <= last_pte ; j++)	{
				UINT32 *pte_tbl = ioremap_nocache(pgd[i] & 0xFFFFFFFE, PAGE_SIZE);
				//UINT32 temp = virt_temp & 0xFFFFF000;
				//flush_data_cache_page(temp);
				fillPageTable(j, &pmd, virt_temp	, pte_tbl);
				virt_temp+=PAGE_SIZE;
				virt_fill_addr+=PAGE_SIZE;
				iounmap(pte_tbl);
			}	
		}
		times++;
	}

	
	
}

bool ovg_mmu_table(unsigned int virt_addr, unsigned int length
	, unsigned int mapped_phy, unsigned int* pgd)
{

	struct ali_mem_block block;
	struct ali_mem_block *tmp = &block;
	block.size = length;
	
	ovg_make_pages_present(virt_addr, virt_addr+length);
	mmuSubsystem.ovg_handler->ovg_map(virt_addr, &tmp, pgd);
	
	return true;
}

bool ovg_mmu_clean(unsigned int virt_addr, unsigned int length, unsigned int* pgd)
{
	unsigned int first_pde, last_pde, virt_temp;
	unsigned int first_pte, last_pte; 

	int i,j;
	
	first_pde = pgd_index(virt_addr);
	last_pde = pgd_index(virt_addr + length - 1);
	
	OVG_DBG_PRINTK(KERN_ALERT," address = %x , fir pgd  = %d , last pgd = %d , size = %d, pgd = %x\n"
				, virt_addr, first_pde, last_pde, length, (unsigned int)pgd);

	virt_temp = virt_addr;

	for (i = first_pde; i <= last_pde ; i++) {
		first_pte = __pte_offset(virt_temp);
		last_pte = (i < last_pde) ? 1023 : __pte_offset(virt_addr+length - 1);
		
		OVG_DBG_PRINTK(KERN_ERR, " address = %x ,  first pte  = %d , last pte = %d\n"
			, virt_temp, first_pte, last_pte);	

		/*May have bug, when this page entry is stilled be mapped by others*/

		for (j=first_pte ; j <= last_pte ; j++)	{
			unsigned int *ptetbl = ioremap_nocache(pgd[i] & 0xFFFFFFFE, sizeof(UINT32));
			ptetbl[j] = 0;
			virt_temp+=PAGE_SIZE;
			iounmap(ptetbl);
		}	
	}
	return true;

}
void ovg_mmu_destroy(unsigned int * pgd)
{
	unsigned int i;
	for(i=0 ; i < 1024 ; i++) {
		if(pgd[i]!=0) {
			unsigned int *ptetbl;
			OVG_DBG_PRINTK(KERN_ALERT, "free_pages address = %x , mapped = %x, index = %d\n"
				, pgd[i], (unsigned int)(ioremap_nocache(pgd[i] & 0xFFFFFFFE, PAGE_SIZE)), i);
			ptetbl = ioremap_nocache(pgd[i] & 0xFFFFFFFE, sizeof(UINT32));
			dma_free_coherent(NULL, PAGE_SIZE, ptetbl, pgd[i] & 0xFFFFFFFE);
			ovg_memory_release_dbg(PAGE_SIZE);
			iounmap(ptetbl);
		}	
	}
}
#if REALLOC_ENABLED	

unsigned int ovg_address_map_fill(unsigned int map_virtual, struct ali_mem_block **block, void *data)
{
	unsigned int *pgd = (unsigned int*)data;
/*

	unsigned int *pgt = NULL;
	unsigned int addr;
	unsigned int loop_time = ((*block)->size) / PAGE_SIZE;
	unsigned int map_physical = ((*block)->phy_addr) ;

	unsigned int pgd_index;// = pgd_index(map_virtual);
	unsigned int pte_index;// = __pte_offset(map_virtual);

	while(loop_time > 0 ){
		pgd_index = pgd_index(map_virtual);
		pte_index = __pte_offset(map_virtual);

		if ( !(*(pgd+ pgd_index) )){
			unsigned int *temp;
			temp = dma_alloc_coherent(NULL, PAGE_SIZE, &addr, GFP_KERNEL);
			//memset(temp , 0, PAGE_SIZE);
			//addr = (reserved_mmualloc(NULL,NULL,NULL))->phy_addr;
			pgd[pgd_index] =addr | OVG_PAGE_PRESENT;	
		}

	//	pgt = phys_to_virt(pgd[pgd_index(map_virtual)] & 0xFFFFFFFE);

		pgt = (unsigned int *)((pgd[pgd_index] & 0xFFFFFFFE) | 0xA0000000);
		pgt[pte_index] = map_physical | OVG_PAGE_PRESENT;

		//OVG_PRINTK("page table content = %x , mapped virtual = %x \n"
		//	, pgt[pte_index] , map_virtual);
		
		loop_time--;
		map_virtual+=PAGE_SIZE;
		map_physical+=PAGE_SIZE;
		

	}
	

	return 0;
	*/
	mmuSubsystem.os_handler->ovg_map(map_virtual, block, find_vma(current->mm, map_virtual));
	mmuSubsystem.ovg_handler->ovg_map(map_virtual, block, pgd);
	return 0;
}
_ali_err ovg_mmu_irq_remap(unsigned int fault_address)
{
	_ali_err err = _ALI_OVG_OK;
	struct ali_ovg_mmuAllocator *active = mmuSubsystem.first_allocator;
	unsigned int request_size = PAGE_SIZE;
	unsigned int left = PAGE_SIZE;

	while(active && (left > 0)){
		struct ali_mem_block* block = NULL;

		err = active->ops->ovg_mmualloc(&block, NULL, NULL, request_size);
		if(err == _ALI_OVG_OK){
			ovg_address_map_fill(fault_address, &block, (void*)ALI_OPENVG_GET_UINT32(0x268));
		}else{
			switch(err){
				case _ALI_OVG_ERR_FAIL:
					if(block && (block->resourceType == OS_MEMORY)){
						kfree(block);
						ovg_memory_release_dbg(sizeof(struct ali_mem_block));
					}	
					active = active->next;
					request_size = PAGE_SIZE;

				break;	
				default:
					OVG_PRINTK("Ba ba, error !!! \n");
				break;
			}
		}

	}

	return err;

}
#endif
int ovg_mmu_map(struct file* filp, struct vm_area_struct *vma)
{	
	
	unsigned int virtual_addr = vma->vm_start;
	unsigned int current_order = VG_MEMORY_BLOCK_MODE;
	unsigned int request_block_size = (1 << current_order);

	_ali_err err;
	struct ali_ovg_mmuAllocator *active = mmuSubsystem.first_allocator;
	struct ali_openVG_handle* handle = ((struct ali_openVG_handle*)filp->private_data);
	struct ali_mem_buffer *buffer = handle->buffer_temp;
	long left ;
	struct ali_mem_block* block = NULL;

	if(!buffer){
		//buffer = kmalloc(sizeof(struct ali_mem_buffer), GFP_KERNEL |  __GFP_NOWARN | __GFP_NORETRY);
		buffer = kmalloc(sizeof(struct ali_mem_buffer), GFP_KERNEL |  __GFP_NOWARN);
		
		ovg_memory_allocate_dbg(sizeof(struct ali_mem_buffer));
		
		buffer->size = vma->vm_end - vma->vm_start;
		buffer->mapped_size= vma->vm_end - vma->vm_start;
		buffer->cpu_ptr = virtual_addr;
	
		buffer->pid = current->pid;
		buffer->tgid = current->tgid;

		if( (buffer->size & (PAGE_SIZE-1)) != 0)
			buffer->size = ((buffer->size + PAGE_SIZE) & ~(PAGE_SIZE-1));

		if( (buffer->mapped_size & (PAGE_SIZE-1)) != 0)
			buffer->mapped_size = ((buffer->mapped_size + PAGE_SIZE) & ~(PAGE_SIZE-1));	

		INIT_LIST_HEAD(&(buffer->physical_list));
		list_add(&(buffer->next), handle->allocated_list);

		
	
	}
		


	left = buffer->mapped_size;
	while(active && (left > 0)){
		block = NULL;
		
		
		while(left < request_block_size){
			current_order--;			
			request_block_size =(1 << (current_order));

		}	
			

		OVG_DBG_PRINTK(KERN_ERR,"left = %d , current_order = %d \n", (unsigned int)left, current_order);

		err = active->ops->ovg_mmualloc(&block, filp, vma, (current_order  - _ali_OVG_MIN ));


		if(err == _ALI_OVG_OK){
			mmuSubsystem.os_handler->ovg_map(virtual_addr, &block, vma);
			mmuSubsystem.ovg_handler->ovg_map(virtual_addr, &block, handle->pdt);
			
			
			list_add(&(block->next), &(buffer->physical_list));

			OVG_DBG_PRINTK(KERN_ALERT,"physical address = %x , size = %d, virtual addr = %x  \n"
				, block->phy_addr , block->size, virtual_addr);

			left = left - request_block_size;
			virtual_addr = virtual_addr + request_block_size;
		} else {
			switch(err) {
				case _ALI_OVG_ERR_FAIL:
					OVG_DBG_PRINTK(KERN_ALERT,"_ALI_OVG_ERR_FAIL\n");
					if(block && (block->resourceType == OS_MEMORY)){
						kfree(block);
						ovg_memory_release_dbg(sizeof(struct ali_mem_block));
					}	
					active = active->next;
					//request_block_size = PAGE_SIZE;
					//current_order = _ali_OVG_MIN;
					if(active)
						active->ops->ovg_mmuoom_handler(&request_block_size, &current_order);
					

				break;	
				case _ALI_OVG_ERR_OOM:
					OVG_DBG_PRINTK(KERN_ALERT,"_ALI_OVG_ERR_OOM\n");
					//request_block_size = PAGE_SIZE;
					//current_order = _ali_OVG_MIN;
					//request_block_size = (1 << (--current_order));
					active->ops->ovg_mmuoom_handler(&request_block_size, &current_order);
				break;
				default:
					OVG_DBG_PRINTK(KERN_ERR,"default\n");
				break;
			}
			
		}
		
	}

	vma->vm_ops = &ovg_vm_ops;
	vma->vm_private_data = handle->allocated_list;
	if(left > 0){
		OVG_PRINTK("Oops, Out of memory! \n");
		if(buffer)
			kfree(buffer);
		return _ALI_OVG_ERR_OOM; // added by Allen
	}	
	
	return 0;

}

unsigned int os_address_map(unsigned int map_virtual, struct ali_mem_block ** block
	, void *data)
{
	
	struct vm_area_struct *vma = (struct vm_area_struct *)(data);
	
	vma->vm_flags |= VM_IO;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if(remap_pfn_range(vma, map_virtual, ((*block)->phy_addr)>> PAGE_SHIFT 
		, (*block)->size, vma->vm_page_prot))
		return -EAGAIN;
	
	return 0;
}

unsigned int os_address_munmap(unsigned int map_virtual, struct ali_mem_block **block, void *data)
{


	return 0;
}

unsigned int ovg_address_check(unsigned int map_virtual, unsigned int map_size)
{
	pgd_t *pgdp;	
	pud_t *pudp;	
	pmd_t *pmdp;	
	pte_t *ptep;
	int page_num = map_size/ PAGE_SIZE;
	int j;

	for(j=0;j<=page_num;j++)
	{
		unsigned int temp = map_virtual + (j << PAGE_SHIFT);
		unsigned int pte_val;
		pgdp = pgd_offset(current->mm, temp );
		pudp = pud_offset(pgdp, temp);				
		pmdp = pmd_offset(pudp, temp);				
		ptep = pte_offset(pmdp, temp);
		pte_val = (unsigned int)pte_val(*ptep);
		if(pte_dirty(*ptep))
			flush_data_cache_page(temp);
		
		if(!(pte_val & 0x01))
			return -1;
	}

	return 0;


}
#ifdef PAGE_ADDRESS_FIX
pmd_t fill_pageDirectory(int i, unsigned int virt_temp, unsigned int* pgd_tbladdr)
{
    unsigned int addr;
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
    pgdp = pgd_offset(current->mm, virt_temp );
    pudp = pud_offset(pgdp, virt_temp);
    pmdp = pmd_offset(pudp, virt_temp);
    return *pmdp;
}

void fill_pageTable(int j, pmd_t * pmdp, unsigned int virt_temp, unsigned int* pte_tbladdr)
{
    unsigned int tbladdr_temp;
    pte_t *ptep;
	struct page* pageX;	
	OVG_DBG_PRINTK(KERN_ALERT,"pte_addr = %x  \n", (unsigned int)pte_tbladdr);
    ptep = pte_offset_kernel(pmdp, virt_temp);
//    pte_tbladdr[j] = (unsigned int)pte_val(*ptep);
	pageX = pte_page(*ptep);
//	pte_tbladdr[j] = (page_to_phys(pageX) & 0x1FFFFFFF)|1;
	tbladdr_temp = page_to_phys(pageX);
	if(tbladdr_temp > 0x18000000) tbladdr_temp -= 0x20000000;
	pte_tbladdr[j] = tbladdr_temp | 1;

    OVG_DBG_PRINTK(KERN_ALERT, " address = %x ,pte index = %d, pte_val = %x\n", virt_temp ,j , pte_tbladdr[j]);

}
#endif
unsigned int ovg_address_map(unsigned int map_virtual, struct ali_mem_block **block, void *data)
{

	unsigned int *pgd = (unsigned int*)data;


	unsigned int first_pgd_index = pgd_index(map_virtual);
	unsigned int last_pgd_index = pgd_index(map_virtual + ((*block)->size) -1);

#ifndef PAGE_ADDRESS_FIX // Generate pdt table only, and pgt use the system one
	unsigned int loop_time = first_pgd_index;
	unsigned int pgdp;
	
	OVG_DBG_PRINTK(KERN_ALERT,"virtual = %x , start = %d , end =%d \n", map_virtual,first_pgd_index,last_pgd_index);
	while(loop_time <= last_pgd_index){
		pgdp = (pgd_val(*pgd_offset(current->mm, map_virtual)));
		flush_data_cache_page(pgdp);
		pgd[loop_time] = (virt_to_phys((void*)pgdp) | OVG_PAGE_PRESENT);
		map_virtual += (1 << 22);
		loop_time++;
		
	}
#else // Generate the whole table (pdt & pgt)
	unsigned int end_adr = map_virtual + ((*block)->size) -1;
//	unsigned int *temp;
	int i,j;
	pmd_t pmd;
	unsigned int first_pte, last_pte;
	
	OVG_DBG_PRINTK(KERN_ALERT,"virtual = %x , start = %d , end =%d \n", map_virtual,first_pgd_index,last_pgd_index);	

    for(i = first_pgd_index; i <= last_pgd_index ; i++)
    {
        pmd = fill_pageDirectory(i, map_virtual, pgd);

        first_pte = __pte_offset(map_virtual);
        last_pte = (i < last_pgd_index) ? 1023 : __pte_offset(end_adr);

       	for(j=first_pte ; j <= last_pte ; j++){
            unsigned int *pte_tbl = ioremap_nocache(pgd[i] & 0xFFFFFFFE, PAGE_SIZE);
			unsigned int  temp = map_virtual & 0xFFFFF000;
            flush_data_cache_page(temp);
			fill_pageTable(j, &pmd, map_virtual, pte_tbl);
			map_virtual += PAGE_SIZE;
            iounmap(pte_tbl);			
		}

	}
#endif
	return 0;


}

unsigned int ovg_address_munmap(unsigned int map_virtual, struct ali_mem_block **block, void *data)
{
	unsigned int *pgd = (unsigned int*)data;
	unsigned int *pgt = NULL;

#ifndef PAGE_ADDRESS_FIX // Generate pdt table only, and pgt use the system one
	pgt = phys_to_virt(pgd[pgd_index(map_virtual)] & 0xFFFFFFFE);
	pgt[__pte_offset(map_virtual)] = 0;
#else // Generate the whole table (pdt & pgt)
	unsigned int first_pgd_index = pgd_index(map_virtual);
	unsigned int last_pgd_index = pgd_index(map_virtual + ((*block)->size) -1);
	unsigned int end_adr = map_virtual + ((*block)->size) -1;
//	unsigned int *temp;
	int i,j;
	unsigned int first_pte, last_pte;
	OVG_DBG_PRINTK(KERN_ALERT,"virtual = %x , start = %d , end =%d \n", map_virtual,first_pgd_index,last_pgd_index);		

    for(i = first_pgd_index; i <= last_pgd_index ; i++)
    {
        first_pte = __pte_offset(map_virtual);
		last_pte = (i < last_pgd_index) ? 1023 : __pte_offset(end_adr);


		pgt = phys_to_virt(pgd[pgd_index(map_virtual)] & 0xFFFFFFFE);
		if ((first_pte==0)&&(last_pte==1023)){
			dma_free_coherent(NULL, PAGE_SIZE, pgt, ((unsigned int)pgt & 0x1FFFFFFF));
			pgd[i] = 0;
			map_virtual+=(PAGE_SIZE<<10);
		}
		else{
			unsigned int rest_flag = 0;			
			for(j=first_pte;j<=last_pte;j++){
				pgt[j] &= 0xFFFFFFFE; // clear presetn bit
	            map_virtual+=PAGE_SIZE;				
			}

			j=0;
			do{
				if(pgt[j]&1){
					rest_flag = 1;
					break;
				}
				if (j==first_pte) j=last_pte; // check 0 ~ first_pte & last_pte ~ 1023
				j++;	
			}while(j<1023);
			if (rest_flag==0)
				dma_free_coherent(NULL, PAGE_SIZE, pgt, ((unsigned int)pgt & 0x1FFFFFFF));
				pgd[i] = 0;			
		}
			
	}
#endif	
	
	return 0;
}
struct ali_ovg_addressHandler ovg_handler = 
{
	.ovg_map = ovg_address_map,
	.ovg_unmap = ovg_address_munmap,
	.ovg_isValidForHWMMU = ovg_address_check,
};

struct ali_ovg_addressHandler os_handler = 
{
	.ovg_map = os_address_map,
	.ovg_unmap = os_address_munmap,
};
