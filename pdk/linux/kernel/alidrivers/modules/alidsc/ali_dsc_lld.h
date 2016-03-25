#ifndef _ALI_DSC_LLD_H_
#define _ALI_DSC_LLD_H_

#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_hld.h>
#include <linux/ali_dsc.h>
#include <linux/ali_ce.h>
#include <ali_otp_common.h>

//#include <rpc_hld/ali_rpc_hld_dsc.h>

#define ALI_DSC_DEV "ali_dsc_0"
#define ALI_CSA_DEV "ali_csa_0"
#define ALI_AES_DEV "ali_aes_0"
#define ALI_DES_DEV "ali_des_0"
#define ALI_SHA_DEV "ali_sha_0"

#define ALI_DSC_KERNEL_BUFFER_SIZE PURE_DATA_MAX_SIZE   
#define ALI_DSC_KERNEL_KEY_SIZE (1024*20)   
#define ALI_DSC_LLD_MAX_ITEM (64)
#define ALI_DSC_COHERENT_DMA_MASK (0xFFFFFFFF)

#define ALI_DSC_STREAM_ID_MAX 0xf

#define ALI_DSC_SEE_DRV_VER_SIZE 20

#define CACHE_LINE_SIZE  32
#define CACHE_L_ALIGN(x) (((x)%CACHE_LINE_SIZE)?((x)+CACHE_LINE_SIZE-((x)%CACHE_LINE_SIZE)):(x))
#define CACHE_A_ALIGN(x) ((void *)((__u32)x & (~(__u32)(CACHE_LINE_SIZE - 1))))

#undef UC
#define UC(x) ((void *)(((__u32)(x)&0xBFFFFFFF)|0xa0000000)) //Un-cached memory addr for see#if 1

#define TS_PACKET_SIZE 188

#if 1
#define ALI_DSC_DEBUG(...)
#else
#define ALI_DSC_DEBUG(fmt, arg...) \
    do { \
        printk("ALI_DSC: In %s %d "fmt, __func__, __LINE__, ##arg); \
    } while (0)
#endif

#define ALI_DSC_DEBUG_ERR(fmt, arg...) \
    do { \
        printk("ALI_DSC: In %s %d "fmt, __func__, __LINE__, ##arg); \
    } while (0)

#if 1
#define ALI_DSC_DUMP(...)
#else
#define ALI_DSC_DUMP(data,len) \
    do{ \
        int i, l=(len); \
        for(i=0; i<l; i++){ \
            printk("0x%02X,",*(__u8 *)((__u32)data+i)); \
            if((i+1)%16==0) \
                printk("\n");\
        }\
        printk("\n");\
    }while(0)
#endif

enum ali_dsc_resource_record
{
	ALI_DSC_RESOURCE_BUSY,
	ALI_DSC_RESOURCE_IDLE,
};

enum HLD_DSC_FUNC{
	/*TRNG*/
	FUNC_TRNG_GENERATE_BYTE = 0,
	FUNC_TRNG_GENERATE_64BITS,
	FUNC_TRNG_SEE_GET64BIT,

	/*DES*/
	FUNC_DES_DECRYPT,
	FUNC_DES_ENCRYPT,
	FUNC_DES_IOCTL,
	/*AES*/
	FUNC_AES_DECRYPT,
	FUNC_AES_ENCRYPT,
	FUNC_AES_IOCTL,
	/*CSA*/
	FUNC_CSA_DECRYPT,
	FUNC_CSA_IOCTL,
	/*SHA*/
	FUNC_SHA_DIGEST,
	FUNC_SHA_IOCTL,

	FUNC_DSC_ATTACH,
	FUNC_DSC_DETACH,
	FUNC_DSC_IOCTL,

	FUNC_TRIG_RAM_MON,
	FUNC_DEENCRYPT,
	FUNC_GET_FREE_STREAM_ID,
	FUNC_SET_STREAM_ID_IDLE,
	FUNC_GET_FREE_SUB_DEVICE_ID,
	FUNC_SET_SUB_DEVICE_ID_IDLE,
	FUNC_SET_STREAM_ID_USED,
	FUNC_SET_SUB_DEVICE_ID_USED,
	FUNC_DEAL_QUANTUM_MIXED_TS,
};

struct ali_m36_sha_dev
{
	struct semaphore *ioctl_sem;
	void *see_sha_id[VIRTUAL_DEV_NUM];
	__u8 *kernel_input_buffer;
	__u8 kernel_output_buffer[SHA512_HASH_SIZE];
	__u32 is_used;
	__u32 sha_dma_mode[VIRTUAL_DEV_NUM];
	__u32 sha_ex_buf[VIRTUAL_DEV_NUM];
	dev_t  devt;
	struct class *dev_class;
	struct cdev cdev;
};

struct ali_m36_des_dev
{
	struct semaphore *ioctl_sem;
	void *see_des_id[VIRTUAL_DEV_NUM];
	__u8 *kernel_input_buffer;
	__u8 *kernel_output_buffer;
	__u32 is_used;
	dev_t  devt;
	struct class *dev_class;
	struct cdev cdev;
	void *g_dev;
};

struct ali_m36_aes_dev
{
	struct semaphore *ioctl_sem;
	void *see_aes_id[VIRTUAL_DEV_NUM];
	__u8 *kernel_input_buffer;
	__u8 *kernel_output_buffer;
	__u32 is_used;
	dev_t  devt;
	struct class *dev_class;
	struct cdev cdev;
	void *g_dev;
};

struct ali_m36_csa_dev
{
	struct semaphore *ioctl_sem;
	void *see_csa_id[VIRTUAL_DEV_NUM];
	__u8 *kernel_input_buffer;
	__u8 *kernel_output_buffer;
	__u32 is_used;
	dev_t  devt;
	struct class *dev_class;
	struct cdev cdev;
	void *g_dev;
};

struct ali_dsc_rec
{
	struct 
	{
		__u32 filp;
		__u32 id;
	}stream_id;
	
	struct 
	{
		__u32 filp;
		__u32 id;
		__u32 mode;
	}dev_id;

	struct
	{
		__u32 filp;
		__u32 handle;
	}handle_id;	

	struct
	{
		__u32 filp;
		__u32 id;
	}pvr_key_id;		
};

struct ali_dsc_kmem
{
	__u32 addr[DSC_MEM_BLOCK_COUNT];
	__u32 bitmap;
	void *kcpu_addr;
	dma_addr_t kdma_addr;
};

struct ali_m36_dsc_dev
{
	struct semaphore ioctl_sem;
	void *see_dsc_id;
	__u8 *kernel_input_buffer;
	__u8 *kernel_output_buffer;  
	__u8 *dsc_key;
	__u32 is_used;
	dev_t  devt;
	struct class *dev_class;
	struct cdev cdev;
	struct device *dev;
	struct ali_dsc_rec rec[ALI_DSC_LLD_MAX_ITEM];
	struct ali_dsc_kmem kmem;
};


typedef struct
{
	__u16 pid_ptr[ALI_DSC_LLD_MAX_ITEM];
	union
	{
		AES_KEY_PARAM p_aes_key_info[ALI_DSC_LLD_MAX_ITEM];       
		CSA_KEY_PARAM p_csa_key_info[ALI_DSC_LLD_MAX_ITEM];
		DES_KEY_PARAM p_des_key_info[ALI_DSC_LLD_MAX_ITEM];
	}key_ptr[1];
	
	union
	{
		AES_IV_INFO p_aes_iv_info[ALI_DSC_LLD_MAX_ITEM];
		DES_IV_INFO p_des_iv_info[ALI_DSC_LLD_MAX_ITEM];
	}iv_ptr[1];
	__u8 ctr_ptr[16];
}DSC_CLR_KEY;

extern struct ali_m36_dsc_dev g_ali_dsc_devices[1];

extern unsigned long __G_ALI_MM_DSC_MEM_START_ADDR;
extern void arm_remap_lowmem_nocache(unsigned int start, unsigned int len);

int ali_dsc_umemcpy(void *dest, const void *src, __u32 n);
void ali_m36_dsc_see_init(void);
void ali_m36_dsc_see_uninit(void);
void ali_dsc_record_handle_id(struct file *filp, __u32 handle, __u32 cmd);

#endif


