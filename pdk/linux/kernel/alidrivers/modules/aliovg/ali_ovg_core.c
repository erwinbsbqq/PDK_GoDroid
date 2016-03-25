
#if	MEMORY_DEBUG
unsigned int memory_usage = 0 ;
unsigned int memory_release = 0;
#endif

#include "ali_ovg.h"
#include "ali_ovg_reg.h"


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
#include <linux/version.h>

#include "ali_ovg_platform.h"

#if !ALI_OPENVG_GPL
#include <linux/wrapper.h>
#endif

#include <linux/interrupt.h>
#if (defined LINUX_VERSION_28 || defined LINUX_VERSION_3_5_2)
#include <asm/mach-ali/m36_irq.h>
#define OVG_IRQ_NUM M36_IRQ_GE
#else
#include <asm/mach-ali/m39xx.h>
#define OVG_IRQ_NUM M39_IRQ_GE
#endif
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>

extern struct ali_ovg_memops memfp;
extern struct ali_ovg_addressHandler os_handler;
extern struct ali_ovg_addressHandler ovg_handler;
//extern struct ali_ovg_mmuAllocator os_allocator;
//extern struct ali_ovg_mmuAllocator pool_allocator;
extern struct ali_ovg_mmuAllocator mmu_allocator;
/*Kernel version info*/
char    KRL_Version[ ] = "ALiOVG_KERNEL_DRIVER_20130426";

#define DEVICE_NAME "openVG"
#define NUM_DEVICE 1


static struct class *ali_ovg_class;

int ali_ovg_major_number = 100;
int ali_ovg_minor_number = 0;
struct ali_openVG_dev *ali_ovg_devices;
struct ali_ovg_mmuSubsystem mmuSubsystem;

struct ali_ovg_pdtRecord pdtRecord[MAX_PDT_NUM];
int init_pdtRecord = 0;
#if UTILIZE_BOARDSETTING
unsigned OVG_BOARD_ADDR = 0xA7F9036C;
module_param(OVG_BOARD_ADDR, int, S_IRUGO);
#endif

#ifdef SHARED_MEM
struct list_head *shmem_list;
struct semaphore shm_sema;
#endif

static struct file_operations ali_openVG_fops = {
	.owner 		= THIS_MODULE, 					/*owner*/
	.open 		= ali_openVG_open,				/*open system call*/
	.release 		= ali_openVG_release,			/*release system call*/
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
         .unlocked_ioctl 		= ali_openVG_ioctl,				/*ioctl system call*/
#else
	.ioctl 		= ali_openVG_ioctl,				/*ioctl system call*/
#endif
	.mmap		= ali_openVG_mmap,				/*mmap system call*/
};

int ali_openVG_open(struct inode *inode, struct file *filp)
{

	struct ali_openVG_handle *ovg_handle;
#if MMU_ENABLED	
	struct ali_ovg_mmuAllocator *temp;
	int i;
#endif	
	int ret = 0;

	ovg_handle = kmalloc(sizeof(struct ali_openVG_handle), GFP_KERNEL);
	
	ovg_memory_allocate_dbg(sizeof(struct ali_openVG_handle));
	
	OVG_PRINTK("Ovg kernel driver open ! \n");

	if(ovg_handle){
		filp->private_data = ovg_handle;
		ovg_handle->mem_list = kmalloc(sizeof(struct list_head), GFP_KERNEL);
		ovg_handle->allocated_list = kmalloc(sizeof(struct list_head), GFP_KERNEL);

#ifdef SHARED_MEM
		if(!shmem_list)
		{
			shmem_list = kmalloc(sizeof(struct list_head), GFP_KERNEL);
			init_MUTEX(&(shm_sema));
		}
#endif		
		ovg_handle->buffer_temp = NULL;

		ovg_memory_allocate_dbg(sizeof(struct list_head));
		ovg_memory_allocate_dbg(sizeof(struct list_head));
		
		if(ovg_handle->mem_list && ovg_handle->allocated_list){
			INIT_LIST_HEAD(ovg_handle->mem_list);
			INIT_LIST_HEAD(ovg_handle->allocated_list);
		}else{      
			ret = -EFAULT;
			kfree(ovg_handle);
		}
#ifdef SHARED_MEM
		if(shmem_list){
			INIT_LIST_HEAD(shmem_list);
		}else{      
			ret = -EFAULT;
			kfree(ovg_handle);
		}
#endif

	}else
		ret = -EFAULT;
	/*memory session begin*/
	if(memfp.ovg_session_begin)
		memfp.ovg_session_begin(filp);
#if MMU_ENABLED || MMU_DBG_ENABLED
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
		ovg_handle->pdt = (unsigned int*)dma_alloc_coherent(NULL, PAGE_SIZE, &ret, GFP_KERNEL);
	
		ovg_memory_allocate_dbg(PAGE_SIZE);
	
		if(ovg_handle->pdt)
			ret = 0;
		memset(ovg_handle->pdt , 0, PAGE_SIZE);

		for(i = 0; i < MAX_PDT_NUM; i++)
		{
			if(pdtRecord[i].ref == 0)
			{
				pdtRecord[i].pdt = ovg_handle->pdt;
				pdtRecord[i].tgid = current->tgid;
				pdtRecord[i].ref = 1;
				break;
			}
		}
		if(i == MAX_PDT_NUM)
			printk("pdtRecord table full \n");
	}
	else
	{
		ovg_handle->pdt = pdtRecord[i].pdt;
		pdtRecord[i].ref++;

		ovg_memory_allocate_dbg(PAGE_SIZE);
		
		ret = 0;
	}

	OVG_PRINTK("page directory address = %x, current = %d \n", (UINT32)ovg_handle->pdt, current->pid);
#endif

#if MMU_ENABLED
	temp = mmuSubsystem.first_allocator;
	while(temp){
		if(temp->ops->ovg_session_begin)
			temp->ops->ovg_session_begin(filp);
		temp = temp->next;
	}
#endif		
	return ret;

}

int ali_openVG_release(struct inode *inode, struct file *filp)
{
	struct ali_openVG_handle *handle = filp->private_data;
	struct list_head* head = handle->mem_list;
	struct list_head* allocated_head = handle->allocated_list;	
#if MMU_ENABLED		
	struct ali_ovg_mmuAllocator *temp;	
	int i;
#endif

	ovg_precord();
	
	OVG_PRINTK("Ovg kernel driver close ! \n");
	/*memory session begin*/

	if(memfp.ovg_session_end)
		memfp.ovg_session_end(filp);

#if MMU_ENABLED
	temp = mmuSubsystem.first_allocator;
	while(temp){
		if(temp->ops->ovg_session_end)
			temp->ops->ovg_session_end(filp);
		temp = temp->next;
	}

	ali_ovg_mmuRelease(filp);

#endif

#if MMU_ENABLED || MMU_DBG_ENABLED
	for(i = 0; i < MAX_PDT_NUM; i++)
		if(pdtRecord[i].tgid == current->tgid)
			break;
	if(i == MAX_PDT_NUM)
	{
		printk("No pdt found\n");
	}
	pdtRecord[i].ref --;
	if(pdtRecord[i].ref == 0)
	{
		dma_free_coherent(NULL, PAGE_SIZE, handle->pdt, ((unsigned int)handle->pdt & 0x1FFFFFFF));
		ovg_memory_release_dbg(PAGE_SIZE);
		pdtRecord[i].tgid = 0;
		pdtRecord[i].pdt = 0;
	}

#endif	

	kfree(head);
	kfree(allocated_head);
	
	kfree(handle);
	
	ovg_memory_release_dbg(sizeof(struct list_head));
	ovg_memory_release_dbg(sizeof(struct list_head));
	ovg_memory_release_dbg(sizeof(struct ali_openVG_handle));
	
#if MEMORY_DEBUG
	OVG_PRINTK("memory allocate %d , memory release %d, leak %d \n"
		,memory_usage, memory_release, (memory_usage - memory_release) );
#endif	
	return 0;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
int ali_openVG_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
int ali_openVG_ioctl( struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif                                      
{
	/*driver relative data*/
	struct ali_openVG_dev *dev=ali_ovg_devices;
	struct ali_openVG_handle* handle = ((struct ali_openVG_handle*)filp->private_data);
	struct list_head *head = handle->mem_list;
	//struct list_head *allocated = handle->allocated_list;	
	/*parameter relative data*/
	int result = 0;

	ovg_srecord(_IOC_NR(cmd) -1);

	switch(cmd) {
		case ALI_OPENVG_EnableCMDQ:
//			dev = container_of(inode->i_cdev, struct ali_openVG_dev, cdev);
			result = ali_ovg_enableQ(dev, arg, (unsigned int)(handle->pdt));
		break;
		case ALI_OPENVG_TessWaitTessFinish:
		case ALI_OPENVG_RastWaitRastFinish:
		case ALI_OPENVG_ImageWaitImageFinish:
		case ALI_OPENVG_HWWaitHWFinish:
			result = ali_ovg_waitHWFinish(cmd, arg);
		break;

		case ALI_OPENVG_MemoryAlloc:
//			dev = container_of(inode->i_cdev, struct ali_openVG_dev, cdev);
			result = ali_ovg_alloc(dev, filp, arg);
		break;
		
		case ALI_OPENVG_MemoryFree:
			result = ali_ovg_free(arg, filp);
		break;
		
		case ALI_OPENVG_SetData:
			result = ali_ovg_setData(arg, head);
		break;
		
		case ALI_OPENVG_GetData:
			result = ali_ovg_getData(arg, head);
		break;
		
		case ALI_OPENVG_Reset:
			result = ali_ovg_reset();
		break;
		
		case ALI_OPENVG_GetHWRegister:
			result = ali_ovg_getHWRegister(arg);
		break;
		
		case ALI_OPENVG_SetHWRegister:
			result = ali_ovg_setHWRegister(arg);
		break;		

		case ALI_OPENVG_Virt2Phy:
			result = ali_ovg_virt2phy(arg, head);
		break;

		case ALI_OPENVG_PFNMapTable:
//			dev = container_of(inode->i_cdev, struct ali_openVG_dev, cdev);
			result = ali_ovg_pfnMapTable(dev, filp, arg);
		break;

		case ALI_OPENVG_GetPDTBaseAddr:
			if(ALI_OPENVG_GET_UINT32(VG_ID) >= 0x00000002)
				result = copy_to_user((unsigned int __user*)arg, &(handle->pdt), sizeof(unsigned int));
			else{
				OVG_PRINTK("hey, MMU is not supported on this HW version \n");
				result = -EFAULT;
			}	
		break;

		case ALI_OPENVG_GEN_PageTable:
				result = ali_ovg_genPageTable(filp, arg);
		break;
		
		case ALI_OPENVG_Realloc:
			if(ALI_OPENVG_GET_UINT32(VG_ID) >= 0x00000002){
//				dev = container_of(inode->i_cdev, struct ali_openVG_dev, cdev);
				result = ali_ovg_realloc(dev, filp, arg);
			}else{
				OVG_PRINTK("hey, MMU is not supported on this HW version \n");
				result = -EFAULT;
			}	
		break;	
		
		case ALI_OPENVG_DestroyAllMemory:
			OVG_PRINTK("Destroy all openVG memory buffers, be careful!\n");
			if(memfp.ovg_session_end)
				memfp.ovg_session_end(filp);
			result = 0;	
		break;	
		
		case ALI_OPENVG_DumpMemory:
			#if MEMORY_DEBUG
				OVG_PRINTK("memory allocate %d , memory release %d, leak %d \n"
				,memory_usage, memory_release, (memory_usage - memory_release) );
			#endif	
			result = 0;
		break;		

		case ALI_OPENVG_MemoryAlloc_Shared:
			result = ali_ovg_alloc_shared(arg);
		break;

		case ALI_OPENVG_MemoryFree_Shared:
			result = ali_ovg_free_shared(arg);
		break;

		default:
			OVG_PRINTK("Driver Error: Unknow Command ID %d \n", cmd);
			result = -EFAULT;
	}
	if(!result)
		ovg_erecord(_IOC_NR(cmd)-1);
	return result;
}

int ali_openVG_mmap(struct file* filp, struct vm_area_struct *vma)
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
#if MMU_ENABLED		
		ovg_srecord(18);
		if ( ovg_mmu_map(filp, vma) == _ALI_OVG_ERR_OOM )
			return -EAGAIN;
		ovg_erecord(18);
#endif		
	}	
	return 0;
	
}

#ifdef ENABLE_IRQ
static irqreturn_t ovg_irq_handle(int a , void* b,  struct pt_regs* c )
{
//	unsigned int fault_address;
	if(ALI_OPENVG_GET_UINT32(VG_MMU_INTERRUPT_STATUS)){
		OVG_PRINTK("Irq handle, MMU not valid \n");
	
#if REALLOC_ENABLED		
	{
		unsigned int fault_address = 0; 
		if(ALI_OPENVG_GET_UINT32(VG_MMU_STATUS_TEWR) & 0xC0)			
			fault_address = ALI_OPENVG_GET_UINT32(VG_MMU_STATUS_TEWR) & 0xFFFFF000;
		else if(ALI_OPENVG_GET_UINT32(VG_MMU_STATUS_RASTRD) & 0xC0)
			fault_address = ALI_OPENVG_GET_UINT32(VG_MMU_STATUS_RASTRD) & 0xFFFFF000;
		else if(ALI_OPENVG_GET_UINT32(VG_MMU_STATUS_RASTWR) & 0xC0)
			fault_address = ALI_OPENVG_GET_UINT32(VG_MMU_STATUS_RASTWR) & 0xFFFFF000;

		OVG_PRINTK("Irq handle, tessellation MMU not valid, address = %x \n", fault_address);
		if (ovg_mmu_irq_remap(fault_address) == _ALI_OVG_OK)
			ALI_OPENVG_GET_UINT32(VG_MMU_STATUS_TERD) |= 0x01;
	}
#else
		while(1);
#endif		
	}

	
	
	(ALI_OPENVG_GET_UINT32(VG_CMD_Q_CTRL_INDX))	|= 0x00000100;
	(ALI_OPENVG_GET_UINT32(VG_CMD_Q_CTRL_INDX)) &= ~0x00000100;
	return IRQ_HANDLED;
}
#endif
#ifdef CONFIG_PM	
static int aliovg_suspend(struct platform_device * dev, pm_message_t state)
{
	UINT32 temp;
	
	while(ovg_chkHW(VG_CMD_Q_CTRL_INDX, 0x14));

	temp = VG_HWCLOCK;
	temp = temp | (1 << 4);
	VG_HWCLOCK = temp;
	OVG_DBG_PRINTK(KERN_ERR, " Kernel Driver suspend called 0x64 = %x \n", VG_HWCLOCK);
	return 0;
}

static int aliovg_resume(struct platform_device * dev)
{
	UINT32 temp;
	temp = VG_HWCLOCK;
	temp = temp & ~(1 << 4);
	VG_HWCLOCK = temp;
	OVG_DBG_PRINTK(KERN_ERR, " Kernel Driver resume called register 0x64 = %x \n", VG_HWCLOCK);
	return 0;
}
#endif 
static int aliovg_probe(struct platform_device * dev)
{
	OVG_DBG_PRINTK(KERN_ERR, " Kernel Driver probe called - test\n");
	return 0;
}


static struct platform_driver aliovg_driver = {
	.probe = aliovg_probe,
#ifdef CONFIG_PM		
	.suspend = aliovg_suspend,
	.resume = aliovg_resume,
#endif 	
	.driver ={
		.name	= "openVG",
		.owner	= THIS_MODULE,	
	}
};

struct platform_device *aliovg_plat_dev;

static int __init ali_openVG_init(void)
{
	int result = 0;
	dev_t dev;
#if MMU_ENABLED		
	struct ali_ovg_mmuAllocator *temp;
#endif
	
	OVG_PRINTK(" Kernel Driver Module INIT, MMU version  %s \n",KRL_Version);
#if UTILIZE_BOARDSETTING
    aliPoolsID[0].name = "FB1";
	aliPoolsID[0].size = OVG_BOARD_ADDR+4;
	aliPoolsID[0].phy_addr = OVG_BOARD_ADDR;
	OVG_PRINTK(" aliPoolsID[].phy_addr  %08x \n",aliPoolsID[0].phy_addr); //aliPoolsID[0].size
#endif	
#if ALI_OPENVG_GPL

	aliovg_plat_dev = platform_device_register_simple("openVG", 0, 0, 0);

	result = platform_driver_register(&aliovg_driver);
	if(result < 0){
		OVG_PRINTK(" Driver Error: can't register platform driver \n");
	}
	
#else

	aliovg_plat_dev = _platform_device_register_simple("openVG", 0, 0, 0);

	result = _platform_driver_register(&aliovg_driver);
	if(result < 0){
		OVG_PRINTK(" Driver Error: can't register platform driver \n");
	}	
#endif

	dev = MKDEV(ali_ovg_major_number, ali_ovg_minor_number);
	
	/*register major number or dynamic allocate major number*/
	if(ali_ovg_major_number)		
		result = register_chrdev_region(dev, NUM_DEVICE, DEVICE_NAME);
	
	if (!ali_ovg_major_number || result < 0) {
		result = alloc_chrdev_region(&dev, ali_ovg_minor_number, NUM_DEVICE, DEVICE_NAME);
		ali_ovg_major_number = MAJOR(dev);
	}
	
	if(result < 0) {
		OVG_PRINTK(" Driver Error: can't get major \n");
		return result;
	}
#if ALI_OPENVG_GPL
	/* class created (post to /proc/devices) */
	ali_ovg_class = class_create(THIS_MODULE, DEVICE_NAME);
	
	if(IS_ERR(ali_ovg_class)) {
		OVG_PRINTK(" Driver Error: can't create class\n");
		return -EFAULT;
	}
	if(ali_ovg_class)
  		device_create(ali_ovg_class, NULL, dev, NULL, DEVICE_NAME);
#else
	/* class created (post to /proc/devices) */
	ali_ovg_class = _class_create(THIS_MODULE, DEVICE_NAME);
	
	if(IS_ERR(ali_ovg_class)) {
		OVG_PRINTK(" Driver Error: can't create class\n");
		return -EFAULT;
	}
	if(ali_ovg_class)
  		_device_create(ali_ovg_class, NULL, dev, NULL, DEVICE_NAME);
#endif
	
	/* allocate ali_vg_dev, and assign for each ali_vg_dev_test object*/
	ali_ovg_devices = kzalloc(sizeof(struct ali_openVG_dev), GFP_KERNEL);
	
	if(!ali_ovg_devices) {
		OVG_PRINTK(" Driver Error: alloc fail\n");
		return -ENOMEM;
	}

	init_MUTEX(&(ali_ovg_devices->hw_sema));
	init_MUTEX(&(ali_ovg_devices->mem_sema));	

	/* init */
	cdev_init(&(ali_ovg_devices->cdev), &ali_openVG_fops);
	ali_ovg_devices->cdev.owner = THIS_MODULE;

	if(cdev_add(&(ali_ovg_devices->cdev), dev , 1)) {
		OVG_PRINTK(" Driver Error: cdev_add fail \n");
		return -EFAULT;			
	}

	/*original without mmu*/
	if(memfp.ovg_modelInit)
		memfp.ovg_modelInit();
		
#if MMU_ENABLED	
	/*Initial state of mmu subsystem*/
	mmuSubsystem.os_handler = &os_handler;
	mmuSubsystem.ovg_handler = &ovg_handler;
	mmuSubsystem.first_allocator= &mmu_allocator;

	init_MUTEX(&(mmuSubsystem.mem_sema));
	temp  = mmuSubsystem.first_allocator;

	while(temp){
		if(temp->ops->ovg_modelInit)
			temp->ops->ovg_modelInit();
		temp = temp->next;
	}
	
#ifdef ENABLE_IRQ
	result = request_irq(OVG_IRQ_NUM, ovg_irq_handle, 0 , "openVG", NULL);
	if (result != 0)
		OVG_PRINTK(" request irq fail \n");
	OVG_PRINTK(" request irq \n");
#endif		
#endif

	
		
	return 0;
}

static void __exit ali_openVG_exit(void)
{
	dev_t dev = MKDEV(ali_ovg_major_number, ali_ovg_minor_number);
	struct ali_ovg_mmuAllocator *temp;

	OVG_PRINTK(" DRIVER EXIT called \n");
#ifdef ENABLE_IRQ
	free_irq(OVG_IRQ_NUM,NULL);
#endif	
	temp  = mmuSubsystem.first_allocator;

	while(temp){
		if(temp->ops->ovg_modelUnload)
			temp->ops->ovg_modelUnload();
		temp = temp->next;
	}


	if(ali_ovg_devices) {
		cdev_del(&(ali_ovg_devices->cdev));		
		kfree(ali_ovg_devices);
	}

#if ALI_OPENVG_GPL
	device_destroy(ali_ovg_class, dev);
	class_destroy(ali_ovg_class);
#else
	_device_destroy(ali_ovg_class, dev);
	_class_destroy(ali_ovg_class);
#endif 


	unregister_chrdev_region(dev, 1);	
	
#if ALI_OPENVG_GPL	
	platform_driver_unregister(&aliovg_driver);
	platform_device_unregister(aliovg_plat_dev);
#else
	_platform_driver_unregister(&aliovg_driver);
	_platform_device_unregister(aliovg_plat_dev);
#endif

	
}

module_init(ali_openVG_init);
module_exit(ali_openVG_exit);

#if ALI_OPENVG_GPL
MODULE_LICENSE("GPL");
#else
MODULE_LICENSE("Propretiry");
#endif
MODULE_AUTHOR("ALi Corporation");
MODULE_DESCRIPTION("ali openVG kernel device driver");



