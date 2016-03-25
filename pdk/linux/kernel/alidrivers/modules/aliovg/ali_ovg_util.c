
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

extern struct ali_ovg_memops memfp;
extern struct ali_ovg_mmuSubsystem mmuSubsystem;

#ifdef SHARED_MEM
extern struct list_head *shmem_list;
extern struct semaphore shm_sema;
#endif

struct ovg_cmdInfo vgLowQ = {
	VG_LOW_PRI_CMD_Q_BA_INDX,
	VG_LOW_PRI_CMD_NUMBER_INDX,
	VG_LQ_EN,
	VG_LQ_READY
};
struct ovg_cmdInfo vgMedQ = {
	VG_MED_PRI_CMD_Q_BA_INDX,
	VG_MED_PRI_CMD_NUMBER_INDX,
	VG_MQ_EN,
	VG_MQ_READY
};
struct ovg_cmdInfo vgHighQ = {
	VG_HIGH_PRI_CMD_Q_BA_INDX,
	VG_HIGH_PRI_CMD_NUMBER_INDX,
	VG_HQ_EN,
	VG_HQ_READY
};

#ifdef SHARED_MEM
struct shmem_entry {
	unsigned int phy_addr;
	unsigned int size;
	unsigned int key;
	unsigned int ref;
	struct list_head next;
};
#endif

#define ovg_getCmdInfo(param) \
	(param->qtype == ali_Q_TYPE_LOW) ? &vgLowQ : \
	(param->qtype == ali_Q_TYPE_MED) ? &vgMedQ : \
	&vgHighQ;


int ovg_enableCmdQ(struct ali_enableQParam* param, unsigned int pdt)
{
	unsigned long timeout = 0;
	struct ovg_cmdInfo* info;

	info = ovg_getCmdInfo(param);

	timeout = jiffies + HZ;
	while(ovg_chkHW(VG_CMD_Q_CTRL_INDX, info->QReady) ) {
		if(time_after(jiffies, timeout)) {
			OVG_PRINTK("command q wait time out...............\n");
			//OVG_PRINTK("current 0x268 address assign = %x \n", ALI_OPENVG_GET_UINT32(0x268));
	 		return -1;					/*time out*/
	 	}
	}
#ifdef S3701_PLATFORM
	ndelay(100); // add for fix MMU bug 2012.07.27
#endif
#if MMU_ENABLED	
	if(ALI_OPENVG_GET_UINT32(VG_ID) >= 0x2){
		//OVG_PRINTK("address assign = %x , cuurent id = %d \n", pdt, current->pid);
		ALI_OPENVG_SET_UINT32(0x268, pdt & 0x1FFFFFFF);
#ifdef C3701_PLATFORM
		ALI_OPENVG_SET_UINT32(0x270, 0x3FFF);	
#endif		
#ifdef S3701_PLATFORM
		ALI_OPENVG_SET_UINT32(0x270, 0x3FF);	
#endif
	}  
#endif	
	/*Command info*/
	ALI_OPENVG_SET_UINT32(info->QAddr, param->qAddr);
	ALI_OPENVG_SET_UINT32(info->QSize, param->cmdSize);
	/*Enable HW*/
	ALI_OPENVG_GET_UINT32(VG_CMD_Q_CTRL_INDX) |= info->QEn;
	ALI_OPENVG_GET_UINT32(VG_CMD_Q_CTRL_INDX) |= VG_CMDQ_EN;	

	return 1;
}

int ali_ovg_enableQ(struct ali_openVG_dev* dev, unsigned long  arg, unsigned int pgd)
{
	struct ali_enableQParam cmdQparam;
	int result;
	copy_from_user(&cmdQparam, (void __user *)arg
								, sizeof(struct ali_enableQParam) );

	if(!dev) 
		return -ENODEV;

	if(cmdQparam.qAddr == 0 || cmdQparam.cmdSize == 0) 
		return -EINVAL;
	
	if(down_interruptible(&(dev->hw_sema))) {
		return -EBUSY;
	}else {
		result = (ovg_enableCmdQ(&cmdQparam, pgd) > 0) ? 0 : -EBUSY;
		up(&(dev->hw_sema));
	}

	return result;
}

int ovg_wait(unsigned int reg, unsigned int val)
{
	unsigned long timeout = 0;

	timeout = jiffies + HZ;//(unsigned long)(VG_TIMEOUT_INTERVAL*(float)HZ);
	while(ovg_chkHW(reg, val)){
		if(time_after(jiffies, timeout)) {
			OVG_PRINTK("wait HW reg %x time out............... \n", reg);	
			
			return -1;
		}	
	}
	return 1;
}


int ali_ovg_reset(void)
{
	VG_HWRESET = 0x00000010;
	VG_HWRESET = 0x00000000;

	(ALI_OPENVG_GET_UINT32(VG_CTRL_INDX))	|= VF_CACHE_RST;
	(ALI_OPENVG_GET_UINT32(VG_CTRL_INDX)) &= ~VF_CACHE_RST;
	
	(ALI_OPENVG_GET_UINT32(VG_CTRL_INDX)) |= VF_RD_CACHE_CLEAN;
	(ALI_OPENVG_GET_UINT32(VG_CTRL_INDX)) |= VF_WR_CACHE_CLEAN;	
	
	ALI_OPENVG_GET_UINT32(VG_RAST_CTRL_INDX) |= WINDING_BUF_INIT_BGN;
	while(!(ALI_OPENVG_GET_UINT32(VG_RAST_CTRL_INDX)&WINDING_BUF_INIT_DONE));

	if(ALI_OPENVG_GET_UINT32(VG_ID) >= 0x00000002){
		ALI_OPENVG_SET_UINT32(0x274, 0x02);
		ALI_OPENVG_SET_UINT32(0x278, 0x02);
		ALI_OPENVG_SET_UINT32(0x27C, 0x02);
		ALI_OPENVG_SET_UINT32(0x284, 0x02);
		ALI_OPENVG_SET_UINT32(0x288, 0x02);

		if(ALI_OPENVG_GET_UINT32(0x274) & 0x02)
			ALI_OPENVG_SET_UINT32(0x274, 0x0);
		if(ALI_OPENVG_GET_UINT32(0x278) & 0x02)
			ALI_OPENVG_SET_UINT32(0x278, 0x0);	
		if(ALI_OPENVG_GET_UINT32(0x27C) & 0x02)
			ALI_OPENVG_SET_UINT32(0x27C, 0x0);
		if(ALI_OPENVG_GET_UINT32(0x284) & 0x02)
			ALI_OPENVG_SET_UINT32(0x284, 0x0);
		if(ALI_OPENVG_GET_UINT32(0x288) & 0x02)
			ALI_OPENVG_SET_UINT32(0x288, 0x0);
	}

	OVG_DBG_PRINTK(KERN_CRIT,"ali_ovg_reset finish VG_RAST_CTRL_INDX\n");
	return 0;
	
}

int ali_ovg_setData(unsigned long  arg, struct list_head* head)
{
	struct ali_setDataParam setParam;

	/* copy from user to get parameter*/
	copy_from_user(&setParam, (void __user *)arg
					, sizeof(struct ali_setDataParam));

	if(memfp.ovg_isValidPhy)
		if (memfp.ovg_isValidPhy(setParam.destination,setParam.size, head))
			return copy_from_user((void *)ioremap_nocache(setParam.destination, setParam.size)
				, (void __user *)setParam.source, setParam.size);

	OVG_PRINTK("Driver Error: Unknown destination address \n");
	return -EFAULT;
}

int ali_ovg_getData(unsigned long  arg, struct list_head* head)
{
	struct ali_setDataParam getParam;

	/* copy from user to get parameter*/
	copy_from_user(&getParam, (void __user *)arg
					, sizeof(struct ali_setDataParam));

	if(memfp.ovg_isValidPhy)
		if (memfp.ovg_isValidPhy(getParam.source, getParam.size, head))
			return copy_to_user((void __user *)getParam.destination
				, (void *)ioremap_nocache(getParam.source, getParam.size), getParam.size);

	OVG_PRINTK("Driver Error: Unknown destination address \n");
	return -EFAULT;
}
  
int ali_ovg_getHWRegister(unsigned long arg) 
{
	unsigned int index;
	/* copy from user to get parameter*/
	copy_from_user(&index, (unsigned int __user *)arg
					, sizeof(unsigned int));
	index = ALI_OPENVG_GET_UINT32(index);
	return copy_to_user((unsigned int __user *)arg, &index, sizeof(unsigned int));

}
// Allen Added for set specific register value 20110329
int ali_ovg_setHWRegister(unsigned long  arg)
{
	struct ali_regParam param;
	copy_from_user(&param, (void __user *)arg
					, sizeof(struct ali_regParam));
	ALI_OPENVG_GET_UINT32(param.rAddr) = param.rValue;
	return 0;
}

int ali_ovg_virt2phy(unsigned long  arg, struct list_head *head)
{
	struct ali_memAllocParam allocParam;

	copy_from_user(&allocParam, (void __user *)arg
				   , sizeof(struct ali_memAllocParam));

	if(memfp.ovg_getPhy)
		allocParam.phy_addr = memfp.ovg_getPhy(allocParam.virt_addr, head);
			return copy_to_user((struct ali_memAllocParam __user *)arg, &allocParam,
						sizeof(struct ali_memAllocParam));
	return -EFAULT;
}

int ali_ovg_waitHWFinish(unsigned int cmd, unsigned long arg)
{
	return (ovg_wait(ovg_getHWReg(cmd), ovg_getHWCond(cmd) ) > 0) 
		? 0 : -EAGAIN;
}

int ali_ovg_alloc(struct ali_openVG_dev* dev, struct file *filp, unsigned long  arg)
{
	struct ali_memAllocParam allocParam;
	unsigned int addr_kernel = 0;
	
	/* copy_from_user to get parameter */
	copy_from_user(&allocParam, (void __user *)arg
					, sizeof(struct ali_memAllocParam) );	
	
	OVG_DBG_PRINTK(KERN_ALERT,"Buffer allocated, size = %d\n", allocParam.size);
	/* allocate memory buffer*/
	if(memfp.ovg_alloc)
		addr_kernel = memfp.ovg_alloc(dev, filp, &allocParam);
	
	if(!addr_kernel){
		OVG_PRINTK("Error: OpenVG Out-of-memory \n");
		return -ENOMEM;
	}
	
#if MMU_DBG_ENABLED

	ovg_mmu_table(allocParam.virt_addr, allocParam.size, allocParam.phy_addr
	, ((struct ali_openVG_handle*)(filp->private_data))->pdt);

#endif	
	
	OVG_DBG_PRINTK(KERN_EMERG,"alloc buffer phy=0x%x , virt = %x , size = %d \n", 
					allocParam.phy_addr, allocParam.virt_addr, allocParam.size);
					
	copy_to_user((struct ali_memAllocParam __user *)arg, &allocParam
				, sizeof(struct ali_memAllocParam));	
	
	return 0;
	
}

int ali_ovg_alloc_shared(unsigned long arg)
{
	struct shmem_entry *entry;
	struct list_head *head;
	struct ali_memAllocParam allocParam;
	bool found = 0;
	gfp_t gfp;
	void *ret;

	/* copy_from_user to get parameter */
	copy_from_user(&allocParam, (void __user *)arg
					, sizeof(struct ali_memAllocParam) );	
	
	down(&shm_sema);
	
	head = shmem_list;
	list_for_each_entry(entry, head, next)
	{
		if(entry->key == allocParam.key)
		{
			allocParam.phy_addr = (entry->phy_addr & 0x1FFFFFFF);
			entry->ref++;
			found = 1;
			break;
		}
	}
	if(!found)
	{
		gfp = 	GFP_KERNEL |  __GFP_NOWARN;
		gfp &= ~(__GFP_DMA | __GFP_DMA32 | __GFP_HIGHMEM);
		//gfp |= __GFP_NORETRY;
		ret = (void *) __get_free_pages(gfp, get_order(allocParam.size));

		allocParam.phy_addr = (unsigned int)((unsigned int)ret & 0x1FFFFFFF);

		entry = kmalloc(sizeof(struct shmem_entry), GFP_KERNEL);
		ovg_memory_allocate_dbg(sizeof(struct shmem_entry));
		if(entry)
		{
			entry->key = allocParam.key;
			entry->phy_addr = (unsigned int)ret;
			entry->size = allocParam.size;
			entry->ref = 1;
			list_add(&(entry->next), head);
		}
	}
	up(&shm_sema);

	OVG_DBG_PRINTK(KERN_ALERT,"Shared Buffer allocated, size = %d\n", allocParam.size);
	
#if MMU_DBG_ENABLED

	ovg_mmu_table(allocParam.virt_addr, allocParam.size, allocParam.phy_addr
	, ((struct ali_openVG_handle*)(filp->private_data))->pdt);

#endif	
	
	OVG_DBG_PRINTK(KERN_EMERG,"alloc buffer phy=0x%x , virt = %x , size = %d \n", 
					allocParam.phy_addr, allocParam.virt_addr, allocParam.size);
					
	copy_to_user((struct ali_memAllocParam __user *)arg, &allocParam
				, sizeof(struct ali_memAllocParam));	
	
	return 0;
	
}

void ali_ovg_mmuRelease(struct file *filp)
{
	struct ali_mem_buffer *buffer, *pre_buffer; 
	struct ali_mem_block *block, *pre_block; 
	struct ali_openVG_handle* handle = ((struct ali_openVG_handle*)filp->private_data);
	struct list_head *head = handle->allocated_list;

	list_for_each_entry_safe(buffer, pre_buffer, head, next) {
		list_for_each_entry_safe(block, pre_block, &(buffer->physical_list), next){
			list_del(&(block->next));
			block->block_free(block);
			if(block->resourceType == OS_MEMORY){
				kfree(block);
				ovg_memory_release_dbg(sizeof(struct ali_mem_block));
			}	
		}
		do_munmap(current->mm, buffer->cpu_ptr , buffer->mapped_size);
		list_del(&(buffer->next));
		kfree(buffer);
		ovg_memory_release_dbg(sizeof(struct ali_mem_buffer));
	}
	
	//ovg_mmu_destroy(handle->pdt);
	
}

int ali_ovg_free(unsigned long  arg, struct file *filp)
{
	struct ali_memAllocParam allocParam;
	struct ali_openVG_handle* handle = ((struct ali_openVG_handle*)filp->private_data);
	struct list_head *head = handle->mem_list;
	
	copy_from_user(&allocParam, (void __user *)arg
				   , sizeof(struct ali_memAllocParam));

	OVG_DBG_PRINTK(KERN_EMERG,"Buffer free, user virt = 0X%x, size = %d, phy = 0X%x \n"
	, allocParam.virt_addr, allocParam.size, allocParam.phy_addr);

/*	
#if MMU_DBG_ENABLED
	ovg_mmu_clean(allocParam.virt_addr, allocParam.size
	, handle->pdt);
#endif
*/
	if(memfp.ovg_free)
		memfp.ovg_free(&allocParam, head);
	
	return 0;	
}

int ali_ovg_free_shared(unsigned long  arg)
{
	struct shmem_entry *entry, *pre_entry;
	struct ali_memAllocParam allocParam;
	struct list_head *head;
	bool found;

	copy_from_user(&allocParam, (void __user *)arg
				   , sizeof(struct ali_memAllocParam));

	OVG_DBG_PRINTK(KERN_EMERG,"Buffer free, user virt = 0X%x, size = %d, phy = 0X%x \n"
	, allocParam.virt_addr, allocParam.size, allocParam.phy_addr);

	down(&shm_sema);
	
	head = shmem_list;
	list_for_each_entry_safe(entry, pre_entry, head, next)
	{
		if(entry->key == allocParam.key)
		{
			found = 1;
			entry->ref--;
			if(!entry->ref)
			{
				free_pages(entry->phy_addr, get_order(entry->size));
				list_del(&(entry)->next);
			}
			break;
		}
	}
	up(&shm_sema);
	
	if(!found)
	{
		printk("no found shared mem key %d, free shared mem failed\n", allocParam.key);
		return -ENOMEM;
	}
	
	return 0;	
}

void ali_ovg_dumpTable(unsigned int start_addr, unsigned int page_num, unsigned int* tbladdr)
{
	pgd_t *pgdp;	
	pud_t *pudp;	
	pmd_t *pmdp;	
	pte_t *ptep;
#ifdef PAGE_ADDRESS_FIX	
	struct page* pageX;
#endif
	int j;

	for(j=0;j<page_num;j++)
	{
		unsigned int temp = start_addr + (j << PAGE_SHIFT);
		unsigned int tbladdr_temp;
//		unsigned int pte_val;
		pgdp = pgd_offset(current->mm, temp );
		pudp = pud_offset(pgdp, temp);				
		pmdp = pmd_offset(pudp, temp);				
#ifdef PAGE_ADDRESS_FIX
		ptep = pte_offset_kernel(pmdp, temp);				
		pageX = pte_page(*ptep);
		if(tbladdr)
		{
			tbladdr_temp = page_to_phys(pageX);
			if(tbladdr_temp > 0x18000000) tbladdr_temp -= 0x20000000;
			tbladdr[j] = tbladdr_temp;
		}
#else
		ptep = pte_offset(pmdp, temp);
		//pte_val = (unsigned int)pte_val(*ptep) & PAGE_MASK;
		if(tbladdr)
		{
			tbladdr_temp = (unsigned int)pte_val(*ptep) & PAGE_MASK;
			if(tbladdr_temp > 0x18000000) tbladdr_temp -= 0x20000000
			tbladdr[j] = tbladdr_temp;
		}
#endif		
		else
			OVG_DBG_PRINTK(KERN_ERR,"invalud tbladdr[%d] !!!!!!!\n",j);
 
		if(pte_dirty(*ptep))
			flush_data_cache_page(temp);

		OVG_DBG_PRINTK(KERN_ERR,"tbladdr = %x , tbladdr in kernel = %x %x, offset page = %x\n"
			, (unsigned int)tbladdr, tbladdr[j], (unsigned int)pte_val(*ptep), temp);
	}


}

int ali_ovg_pfnMapTable(struct ali_openVG_dev* dev, struct file *filp, unsigned long  arg)
{

	struct ali_pfnMap pfnMap;
	unsigned int offset_byte, page_num,  *tbladdr; 

	copy_from_user(&pfnMap, (void __user *)arg
					, sizeof(struct ali_pfnMap));

	offset_byte = pfnMap.virt_startAddr & (PAGE_SIZE -1);
	page_num = (offset_byte+pfnMap.mapped_size+PAGE_SIZE-1)/PAGE_SIZE;

	tbladdr = kmalloc(page_num * sizeof(unsigned int), GFP_KERNEL);
	
	ovg_memory_allocate_dbg(sizeof(unsigned int));
	
	ovg_make_pages_present(pfnMap.virt_startAddr, pfnMap.virt_startAddr+pfnMap.mapped_size);
	ali_ovg_dumpTable(pfnMap.virt_startAddr & PAGE_MASK, page_num, tbladdr);
	
#if MMU_ENABLED
	ovg_mmu_table(pfnMap.virt_startAddr, pfnMap.mapped_size, 0
	, ((struct ali_openVG_handle*)(filp->private_data))->pdt);
#endif

	copy_to_user((void __user *)pfnMap.tblAddr, (void*)tbladdr
					, page_num * sizeof(unsigned int));

	kfree(tbladdr);
	ovg_memory_release_dbg(sizeof(unsigned int));
	return 0;
	
}

int ali_ovg_genPageTable(struct file *filp, unsigned long arg)
{
	struct ali_mem_block* block = kmalloc(sizeof(struct ali_mem_block), GFP_KERNEL);
	struct ali_openVG_handle* handle = ((struct ali_openVG_handle*)filp->private_data);
	struct ali_addressGen param;
	
	copy_from_user(&param, (void __user *)arg
					, sizeof(struct ali_addressGen));
	
	block->size = param.size;
	ovg_make_pages_present(param.virtual_address, param.virtual_address+param.size);	
	mmuSubsystem.ovg_handler->ovg_map(param.virtual_address, &block, handle->pdt);
	
	kfree(block);
	
	return 0;
}

int ali_ovg_realloc(struct ali_openVG_dev *dev, struct file *filp, unsigned long arg)
{
	struct ali_memMMUAlloc alloc;
	//struct ali_mem_buffer *buffer = kmalloc(sizeof(struct ali_mem_buffer), GFP_KERNEL |  __GFP_NOWARN | __GFP_NORETRY);
	struct ali_mem_buffer *buffer = kmalloc(sizeof(struct ali_mem_buffer), GFP_KERNEL |  __GFP_NOWARN);
	struct ali_openVG_handle* handle = ((struct ali_openVG_handle*)filp->private_data);
	struct list_head *head = handle->allocated_list;
	
	copy_from_user(&alloc, (void __user *)arg
					, sizeof(struct ali_memMMUAlloc));

	if(alloc.min_mapped_size != alloc.size)
		OVG_PRINTK("Realloc, request = %d , mim mapped = %d \n",alloc.size, alloc.min_mapped_size);

	if(!buffer){
		OVG_PRINTK("kmalloc fail\n");
		return -ENOMEM;
	}	

	if( (alloc.size & (PAGE_SIZE-1)) != 0)
		alloc.size = ((alloc.size + PAGE_SIZE) & ~(PAGE_SIZE-1));

	if( (alloc.min_mapped_size & (PAGE_SIZE-1)) != 0)
		alloc.min_mapped_size = ((alloc.min_mapped_size + PAGE_SIZE) & ~(PAGE_SIZE-1));	

	buffer->size = alloc.size;
	buffer->mapped_size = alloc.min_mapped_size;

	handle->buffer_temp = buffer;
	INIT_LIST_HEAD(&(buffer->physical_list));
	
#ifdef LINUX_VERSION_3_5_2
	alloc.virt_addr = vm_mmap(filp, 0, alloc.size 
								, PROT_READ|PROT_WRITE
								, MAP_SHARED
								, 0);	
#else
	down_write(&current->mm->mmap_sem);
	alloc.virt_addr = do_mmap(filp, 0, alloc.size 
								, PROT_READ|PROT_WRITE
								, MAP_SHARED
								, 0);
	up_write(&current->mm->mmap_sem); 	
#endif	
	
	//alloc.ali_virt_addr = (alloc.virt_addr & 0x0FFFFFFF ) | 0xC0000000;
	OVG_PRINTK("ali virtual %x\n", alloc.ali_virt_addr);
	
	if(alloc.ali_virt_addr)
		ovg_mmu_table_aliVirtual(alloc.ali_virt_addr, alloc.min_mapped_size, alloc.virt_addr, handle->pdt);
	
	buffer->ali_virtual = alloc.ali_virt_addr;
	buffer->cpu_ptr = alloc.virt_addr;
	buffer->mapped_size = alloc.min_mapped_size;
	buffer->size = alloc.size;

	list_add(&(buffer->next), head);
	handle->buffer_temp = NULL;

	return copy_to_user((struct ali_memMMUAlloc __user *)arg, &alloc
						, sizeof(struct ali_memMMUAlloc));


}
/*
int ali_ovg_mmuFree(unsigned long arg, struct file *filp)
{
	struct ali_memMMUAlloc param;
	struct ali_mem_buffer *buffer, *pre_buffer; 
	struct ali_mem_block *block, *pre_block; 
	struct ali_openVG_handle* handle = ((struct ali_openVG_handle*)filp->private_data);
	struct list_head *head = handle->allocated_list;

	OVG_PRINTK("Driver error, this is not supported currently! \n");
	return -1;
	
	copy_from_user(&param, (void __user *)arg
					, sizeof(struct ali_memMMUAlloc));
	
	list_for_each_entry_safe(buffer, pre_buffer, head, next) {
		if(buffer->cpu_ptr== param.virt_addr) {
			list_for_each_entry_safe(block, pre_block, &(buffer->physical_list), next){
				list_del(&(block->next));
				block->block_free(block);				
				if(block->resourceType == OS_MEMORY){
					kfree(block);
					ovg_memory_release_dbg(sizeof(struct ali_mem_block));
				}	
			}
			do_munmap(current->mm, buffer->cpu_ptr , buffer->mapped_size);
			list_del(&(buffer->next));

			kfree(buffer);
			ovg_memory_release_dbg(sizeof(struct ali_mem_buffer));
			break;
		}
	}
	return 0;
}

*/
void *ovg_dma_alloc_coherent(struct device *dev, size_t size,
	dma_addr_t * dma_handle, gfp_t gfp)
{
	void *ret;
	gfp &= ~(__GFP_DMA | __GFP_DMA32 | __GFP_HIGHMEM);
	//gfp |= __GFP_NORETRY;

	ret = (void *) __get_free_pages(gfp, get_order(size));

	if (ret) {		
		*dma_handle = plat_map_dma_mem(dev, ret, size);

		if (!plat_device_is_coherent(dev)) {
			dma_cache_wback_inv((unsigned long) ret, size);
			ret = UNCAC_ADDR(ret);
		}
	}
	return ret;
}


int ovg_make_pages_present(unsigned long addr, unsigned long end)
{
	int ret, len, write;
	struct vm_area_struct * vma;

	vma = find_vma(current->mm, addr);
	if (!vma)
		return -ENOMEM;
	write = (vma->vm_flags & VM_WRITE) != 0;
	BUG_ON(addr >= end);
	BUG_ON(end > vma->vm_end);
	len = DIV_ROUND_UP(end, PAGE_SIZE) - addr/PAGE_SIZE;
	ret = get_user_pages(current, current->mm, addr,
			len, write, 0, NULL, NULL);
	if (ret < 0)
		return ret;
	return ret == len ? 0 : -EFAULT;
}

