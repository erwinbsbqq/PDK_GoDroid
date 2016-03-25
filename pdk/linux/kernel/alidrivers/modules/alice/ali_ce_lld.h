#ifndef _ALI_CE_LLD_H_
#define _ALI_CE_LLD_H_

#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_hld.h>
#include <linux/ali_ce.h>


#if 1
#define ALI_CE_DEBUG(...)
#else
#define ALI_CE_DEBUG(fmt, arg...) \
    do { \
        printk("ALI_CE: In %s %d "fmt, __func__, __LINE__, ##arg); \
    } while (0)
#endif

#define ALI_CE_DEBUG_ERR(fmt, arg...) \
    do { \
        printk("ALI_CE: In %s %d "fmt, __func__, __LINE__, ##arg); \
    } while (0)
    
#define ALI_CE_RPC_ERR         printk
#define ALI_CE_RPC_ERR_DUMP(data,len) \
    do{ \
        int i, l=(len); \
        for(i=0; i<l; i++){ \
            ALI_CE_RPC_ERR("0x%x,",*(data+i)); \
            if((i+1)%16==0) \
                ALI_CE_RPC_ERR("\n");\
        }\
    }while(0)


#if 1
#define ALI_CE_RPC_DEBUG(...)  do{}while(0);
#define ALI_CE_RPC_DUMP(...)   do{}while(0);
#else
#define ALI_CE_RPC_DEBUG(fmt, arg...) \
    do { \
        printk("ALI_CE_RPC: In %s %d "fmt, __func__,__LINE__, ##arg); \
    } while (0)
#define ALI_CE_RPC_DUMP(data,len) \
    do{ \
        int i, l=(len); \
        for(i=0; i<l; i++){ \
            printk("0x%x,",*(data+i)); \
            if((i+1)%16==0) \
                printk("\n");\
        }\
    }while(0)
#endif

#define ALI_CE_DEV "ali_ce_0"

enum HLD_CE_FUNC
{
	FUNC_CE_ATTACH = 0,
	FUNC_CE_SET_AKSV,
	FUNC_PATCH_HDMI,
	FUNC_CE_DETACH, 
	FUNC_CE_GENERATE,
	FUNC_CE_LOAD,
	FUNC_CE_IOCTL,
	FUNC_CE_GENERATE_CW_KEY,
	FUNC_CE_GENERATE_SINGLE_LEVEL_KEY 
};

#define ALI_CE_KERNEL_BUFFER_SIZE 0x100000   

#define ALI_CE_REC_MAX_ITEM 0x82

enum ali_ce_resource_record
{
	ALI_CE_RESOURCE_BUSY,
	ALI_CE_RESOURCE_IDLE,
};


struct ali_ce_rec
{
	__u32 filp;
	__u32 pos;
};

struct ali_m36_ce_dev
{
	struct semaphore ioctl_sem;
	void *see_ce_id;
	__u32 is_used;
	dev_t  devt;
	struct class *dev_class;
	struct cdev cdev;	
	struct ali_ce_rec rec[ALI_CE_REC_MAX_ITEM];
};


int ali_ce_umemcpy(void *dest, const void *src, __u32 n);
void ali_m36_ce_see_init(void);
void ali_m36_ce_see_uninit(void);


#endif
