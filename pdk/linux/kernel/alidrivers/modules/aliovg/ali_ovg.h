#ifndef _LINUX_ALI_OVG_H
#define _LINUX_ALI_OVG_H
   
//#define ALI_OVG_DEBUG
//#define ALI_OVG_DUMP

//#define OVG_DBG_HIGH		1
//#define OVG_DBG_NORMAL	2
//#define OVG_DBG_LOW			3
//#define OVG_DBG_MEMORY	0
//#define OVG_DBG_DEBUG_LEVEL	OVG_DBG_MEMORY


//#define ALI_3BANK
   
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/kernel.h>
//#include <linux/aliovg-ioctl.h>
#include "aliovg-ioctl.h"



typedef unsigned int UINT32;
#define OVG_PAGE_PRESENT 0x01

#if MEMORY_DEBUG
	extern unsigned int memory_usage;
	extern unsigned int memory_release;
	#define ovg_memory_allocate_dbg(val)  memory_usage+=val	
	#define ovg_memory_release_dbg(val)  memory_release+=val	
#else
	#define ovg_memory_allocate_dbg(val) 
	#define ovg_memory_release_dbg(val)
#endif
	
typedef enum {
	ali_W_TESS,
	ali_W_RAST,
	ali_W_IMAGE,
	ali_W_HW
} ali_wait_type;

typedef enum {
	NO_USED = 0,
	IN_USED = 1,
}ali_ovg_mem;

typedef enum {
	OS_MEMORY = 1,
	RESERVED_MEMORY = 2,
	
}ali_mem_type;


typedef enum {
	_ALI_OVG_OK = 0,
	_ALI_OVG_ERR_OOM = 1,
	_ALI_OVG_ERR_FAIL = 2,

}_ali_err;


typedef enum {
	_ali_OVG_2048K = 21,
	_ali_OVG_1024K = 20,
	_ali_OVG_512K = 19,
	_ali_OVG_256K = 18,
	_ali_OVG_128K = 17,	
	_ali_OVG_64K = 16,
	_ali_OVG_32K = 15,	
	_ali_OVG_16K = 14,
	_ali_OVG_8K = 13,	
	_ali_OVG_4K = 12,
	_ali_OVG_MAX = _ali_OVG_2048K,
	_ali_OVG_MIN = _ali_OVG_4K,
}_ali_ovg_mem_mode;

#define RAST_FINISH_MASK  	0x00000001
#define TESS_FINISH_MASK  	0x00000001
#define IMAGE_FINISH_MASK 	0x80000000
#define HW_FINISH_MASK		0x00000005

#define MAX_PDT_NUM			0x0000000A
/*
Note:
	User driver can decide current run is using 3 Bank cmdq or 2 Bank cmdq.
3 Bank cmdq: Using 3 cmd q to interleave control HW
2 Bank cmdq: Using 2 cmd q to interleave control HW
	Using 2 bank must wait HW finish (Due to memory issue), then enable next one.
	Using 3 bank can wait HW cmdQ finish only, then enable next one.
*/
#ifdef ALI_3BANK
#define VG_HQ_READY 		0x40 
#define VG_MQ_READY 		0x20
#define VG_LQ_READY		0x10 
#else 
#define VG_HQ_READY 		0x44 
#define VG_MQ_READY 		0x24 
#define VG_LQ_READY		0x14 
#endif

#define VG_HQ_EN 		0x40
#define VG_MQ_EN 		0x20
#define VG_LQ_EN 		0x10
#define VG_CMDQ_EN  	0x01

#define WINDING_BUF_INIT_BGN		0x80000000
#define WINDING_BUF_INIT_DONE		0x40000000
#define VF_CACHE_RST				0x00000004
#define VF_RD_CACHE_CLEAN			0x00000010
#define VF_WR_CACHE_CLEAN			0x00000100

#define VG_TIMEOUT_INTERVAL 1
#define VG_RESERVED_BUFFER_SIZE 3


//#define VG_MEMORY_BLOCK_MODE	_ali_OVG_2048K	/*Block size = 2MB */
#define VG_MEMORY_BLOCK_MODE	_ali_OVG_4K	/*Block size = 4K */

#define OVG_PRINTK(fmt, args...) \
	printk("ALi OpenVG : <%s> "fmt , __FUNCTION__, ##args)	

#ifdef ALI_OVG_DEBUG
#define OVG_DBG_PRINTK(level, fmt, args...) \
	printk(level "ALi OpenVG : <%s> "fmt , __FUNCTION__, ##args)

#else
#define OVG_DBG_PRINTK(fmt, args...)
#endif

#define is_dynamic_allocate(size)	(1 << (get_order(size) + 1))
#define ovg_chkHW(reg, val)  ((ALI_OPENVG_GET_UINT32(reg))& val)

#define ovg_getHWReg(cmd) \
	(cmd == ALI_OPENVG_TessWaitTessFinish) ? VG_TESS_CTRL_INDX : \
	(cmd == ALI_OPENVG_RastWaitRastFinish) ? VG_RAST_CTRL_INDX : \
	(cmd == ALI_OPENVG_ImageWaitImageFinish) ? VG_IMAGE_CTR_INDX : \
	VG_CMD_Q_CTRL_INDX
	
#define ovg_getHWCond(cmd) \
	(cmd == ALI_OPENVG_TessWaitTessFinish) ? TESS_FINISH_MASK : \
	(cmd == ALI_OPENVG_RastWaitRastFinish) ? RAST_FINISH_MASK : \
	(cmd == ALI_OPENVG_ImageWaitImageFinish) ? IMAGE_FINISH_MASK : \
	HW_FINISH_MASK

#ifdef ALI_OVG_DUMP
extern void ovg_record(unsigned int index);
extern void ovg_stop(unsigned int index);
extern void ovg_dump(void);

#define ovg_srecord(cmd) ovg_record(cmd)
#define ovg_erecord(cmd) ovg_stop(cmd)
#define ovg_precord() ovg_dump()
#else
#define ovg_srecord(cmd)  
#define ovg_erecord(cmd) 
#define ovg_precord()
#endif

struct ALiPools{
    const char *name;
    u32 size;
    unsigned int phy_addr;
};

struct ovg_cmdInfo {
	unsigned int QAddr;
	unsigned int QSize;
	unsigned int QEn;
	unsigned int QReady;
};

struct ali_openVG_dev {
	struct cdev		cdev;
	struct semaphore hw_sema;			/*semaphore control for hw usage*/
	struct semaphore mem_sema;			/*semaphore control for reserved mem usage*/
};

struct ali_openVG_handle{
	struct list_head *mem_list;	
	struct list_head *allocated_list;		
	unsigned int *pdt;  					/*page directory table*/
	struct ali_mem_buffer	*buffer_temp;
};

struct ali_model_dirtyList{
	struct list_head mem_list;
	unsigned int address;
};


struct ali_mem_block{
	struct list_head next;
	struct ali_mem_block* nextBlock;
	bool used;
	unsigned int phy_addr;
	unsigned int virt;
	unsigned int size;
	ali_mem_type resourceType;
	void (*block_free) (struct ali_mem_block* block);
};

struct ali_mem_buffer{
	struct list_head physical_list;
	unsigned int cpu_ptr;
	unsigned int ali_virtual;
	unsigned int size;
	unsigned int mapped_size;
	unsigned int pid;
	unsigned int tgid;
	struct list_head next;
};

struct ali_ovg_memops
{
	void (*ovg_modelInit)(void);
	void (*ovg_session_begin)(struct file *filp);
	void (*ovg_session_end)(struct file *filp);	
	unsigned int (*ovg_alloc)(struct ali_openVG_dev* dev,struct file *filp, struct ali_memAllocParam* out);
	void (*ovg_free) (struct ali_memAllocParam* param, void* private_data);
	bool (*ovg_isValidPhy)(unsigned int, unsigned int,void* private_data);
	unsigned int (*ovg_getPhy)(unsigned int, void* private_data);

	struct ali_ovg_memops* next;
};

struct ali_ovg_mmuops
{
	void (*ovg_modelInit)(void);
	void (*ovg_modelUnload)(void);
	void (*ovg_session_begin)(struct file *filp);
	void (*ovg_session_end)(struct file *filp);	
	_ali_err (*ovg_mmualloc)(struct ali_mem_block** block,struct file *filp, struct vm_area_struct *vma, unsigned int request_block);
	void (*ovg_mmufree) (struct ali_mem_block* block);
	void (*ovg_mmuoom_handler) (unsigned int*, unsigned int*);
	
};

struct ali_ovg_addressHandler
{
	unsigned int (*ovg_map)(unsigned int map_virtual, struct ali_mem_block **, void *data);//struct vm_area_struct *vma);
	unsigned int (*ovg_unmap)(unsigned int map_virtual, struct ali_mem_block **, void *data);//struct vm_area_struct *vma);
	unsigned int (*ovg_isValidForHWMMU)(unsigned int map_virtual, unsigned int map_size);
};


struct ali_ovg_memBank
{
	unsigned int size;
	unsigned int start_address;
	const char *name;
	struct list_head first_free[_ali_OVG_MAX - _ali_OVG_MIN +1 ];
//	struct ali_mem_block* first_free;
};

struct ali_ovg_mmuAllocator
{
	struct ali_ovg_mmuops *ops;
	struct ali_ovg_memBank *bank;
	struct ali_ovg_mmuAllocator *next;	
};

struct ali_ovg_mmuSubsystem
{
	struct ali_ovg_addressHandler* os_handler;
	struct ali_ovg_addressHandler* ovg_handler;
	struct ali_ovg_mmuAllocator* first_allocator;
	struct semaphore mem_sema;	
};

struct ali_ovg_memSubsystem
{
	struct ali_ovg_mmuAllocator* first_allocator;
	struct semaphore mem_sema;	
};

struct ali_ovg_pdtRecord
{
	unsigned int *pdt;
	unsigned int tgid;
	unsigned int ref;
	struct ali_ovg_pdtRecord *next;
};

/*file operation*/
int ali_openVG_open(struct inode *inode, struct file *filp);
int ali_openVG_release(struct inode *inode, struct file *filp);
#ifdef LINUX_VERSION_3_5_2
int ali_openVG_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#else
int ali_openVG_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#endif
int ali_openVG_mmap(struct file* filp, struct vm_area_struct *vma);

/*internal use*/
int ali_ovg_enableQ(struct ali_openVG_dev*, unsigned long arg, unsigned int);
int ali_ovg_alloc(struct ali_openVG_dev*, struct file *filp, unsigned long arg);
int ali_ovg_free(unsigned long arg, struct file *filp);
int ali_ovg_setData(unsigned long arg, struct list_head* head);
int ali_ovg_getData(unsigned long arg, struct list_head* head);
int ali_ovg_reset(void);
int ali_ovg_getHWRegister(unsigned long arg);
int ali_ovg_realloc(struct ali_openVG_dev*, struct file *filp, unsigned long arg);
void ali_ovg_mmuRelease(struct file *filp);
int ali_ovg_alloc_shared(unsigned long arg);
int ali_ovg_free_shared(unsigned long arg);

// Allen Added for set specific register value 20110329
int ali_ovg_setHWRegister(unsigned long arg);
int ali_ovg_virt2phy(unsigned long arg, struct list_head *head);
int ali_ovg_waitHWFinish(unsigned int cmd, unsigned long arg);
int ali_ovg_pfnMapTable(struct ali_openVG_dev*, struct file *filp, unsigned long arg);
int ali_ovg_genPageTable(struct file *filp, unsigned long arg);
int ovg_mmu_map(struct file* filp, struct vm_area_struct *vma);
void ovg_mmu_table_aliVirtual(unsigned int virt_addr, unsigned int length
	, unsigned int mapped_virt, unsigned int* pgd);
bool ovg_mmu_table(unsigned int , unsigned int , unsigned int , unsigned int* );
bool ovg_mmu_clean(unsigned int , unsigned int, unsigned int* );
void ovg_mmu_destroy(unsigned int* );
int ovg_wait(unsigned int reg, unsigned int val);
int ovg_make_pages_present(unsigned long addr, unsigned long end);
void *ovg_dma_alloc_coherent(struct device *dev, size_t size,
	dma_addr_t * dma_handle, gfp_t gfp);
_ali_err ovg_mmu_irq_remap(unsigned int fault_address);
_ali_err  reserved_mmualloc(struct ali_mem_block** block,struct file *filp, struct vm_area_struct *vma, unsigned int size_mode);
unsigned int os_address_map(unsigned int map_virtual, struct ali_mem_block ** block, void *data);
unsigned int ovg_address_map(unsigned int map_virtual, struct ali_mem_block **block, void *data);
	
#endif
