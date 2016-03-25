#ifndef _ALI_TRNG_LLD_H_
#define _ALI_TRNG_LLD_H_

#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_hld.h>
#include <linux/ali_trng.h>

#if 1
#define ALI_TRNG_DEBUG(...)
#else
#define ALI_TRNG_DEBUG(fmt, arg...) \
    do { \
        printk("ALI_TRNG: In %s "fmt, __func__, ##arg); \
    } while (0)
#endif

#define ALI_TRNG_DEBUG_ERR(fmt, arg...) \
    do { \
        printk("ALI_TRNG: In %s "fmt, __func__, ##arg); \
    } while (0)
    
#if 1
#define ALI_TRNG_RPC_DEBUG(...)
#else
#define ALI_TRNG_RPC_DEBUG(fmt, arg...) \
    do { \
        printk("ALI_TRNG: In %s "fmt, __func__, ##arg); \
    } while (0)

#endif

#define TRNG_IO_CMD(cmd)  (cmd & 0xFF)

enum TRNG_SW_FUNC
{
    FUNC_TRNG_GENERATE_BYTE = 0,
    FUNC_TRNG_GENERATE_64BITS,
    FUNC_TRNG_SEE_GET64BIT,
};

#define ALI_TRNG_DEV "ali_trng_0"

#define ALI_TRNG_KERNEL_BUFFER_SIZE (1024)

struct ali_m36_trng_dev
{
	struct semaphore ioctl_sem;
	__u8 kernel_output_buffer[ALI_TRNG_MAX_GROUP*16*8]; 
	__u32 is_used;
	dev_t  devt;
	struct class *dev_class;
	struct cdev cdev;
};


#endif


