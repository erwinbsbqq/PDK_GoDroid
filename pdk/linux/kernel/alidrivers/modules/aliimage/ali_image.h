#ifndef __MEDIA_IMAGE_ALI_IMAGE_H
#define __MEDIA_IMAGE_ALI_IMAGE_H

#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
//#include <linux/smp_lock.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/vt.h>
#include <linux/init.h>
#include <linux/linux_logo.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/console.h>
#include <linux/kmod.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/efi.h>
#include <linux/fb.h>

#include <linux/ali_rpc.h>
#include <linux/ali_transport.h>

#include <ali_image_common.h>

#if 1

#define IMAGE_PRF	if(debug_flag) printk
#else
#define IMAGE_PRF(...) 	do{}while(0)
#endif


#define __MM_FB0_Y_LEN			(1920*1088+1024)//(736*576+512)	//for high definition jpg decode
#define __MM_FB1_Y_LEN			__MM_FB0_Y_LEN
#define __MM_FB2_Y_LEN			__MM_FB0_Y_LEN

#define __MM_FB0_C_LEN			(__MM_FB0_Y_LEN/2)
#define __MM_FB1_C_LEN			__MM_FB0_C_LEN
#define __MM_FB2_C_LEN			__MM_FB0_C_LEN

#define __MM_FB3_Y_LEN			(736*576+1024)
#define __MM_FB3_C_LEN			(__MM_FB3_Y_LEN/2)
#define __MM_FB4_Y_LEN			__MM_FB3_Y_LEN
#define __MM_FB4_C_LEN			__MM_FB3_C_LEN
#define __MM_FB5_Y_LEN          		__MM_FB3_Y_LEN
#define __MM_FB5_C_LEN          		__MM_FB3_C_LEN
#define __MM_FB6_Y_LEN         		__MM_FB3_Y_LEN
#define __MM_FB6_C_LEN          		__MM_FB3_C_LEN

#define __MM_FB_LEN			    		0x19c6200

#define MAX_VIDEO_RPC_ARG_NUM		4

/* private information about the this module */
struct ali_image_info {
    struct cdev cdev;
    //struct image_device *cur_dev;
	struct semaphore sem;
    unsigned long mem_addr;
    unsigned long mem_size;
    unsigned long priv_mem_addr;
    unsigned long priv_mem_size;
    volatile unsigned long *rpc_arg[MAX_VIDEO_RPC_ARG_NUM];
    volatile int rpc_arg_size[MAX_VIDEO_RPC_ARG_NUM];
    int dst_pid;
    int open_count;
};

#endif
