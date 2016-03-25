/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_pe.c
 *  (I)
 *  Description: ali play engine for media player library
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.06.28			Sam			Create
 ****************************************************************************/
#include <linux/version.h>
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
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
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/ali_transport.h>
#include <ali_pe_common.h>
#include "ali_pe.h"

volatile unsigned long *g_ali_pe_rpc_arg[MAX_PE_RPC_ARG_NUM];
volatile int g_ali_pe_rpc_arg_size[MAX_PE_RPC_ARG_NUM];

unsigned long g_ali_pe_buf_size = 0;
unsigned long g_ali_pe_buf_start_addr = 0;

static int g_ali_pe_live = 0;
static int g_ali_pe_open_cnt = 0;

static struct ali_pe_info *m_pe_info = NULL;

#define LOCK_MUTEX	\
	if(down_interruptible(&info->semaphore)){ \
		PE_PRF("ali_pe down sem fail\n"); \
		return -1; \
	}

#define UNLOCK_MUTEX \
	up(&info->semaphore)
	
static void send_cache_msg(struct ali_pe_info *info)
{
	struct cache_info *cache = NULL;
	unsigned char msg[20];
	int msg_size = 20;

	cache = &info->cache[info->cache_pending];
	msg[0] = ALIPE_MSG_CACHE;
	msg[1] = cache->cmd;
	msg[2] = 17;
	msg[3] = info->cache_pending;
	memcpy((void *)(msg + 4), (void *)&(cache->par1), 16);

	//PE_PRF("%s : in idx %d\n", __FUNCTION__, info->cache_pending);
	ali_transport_send_msg(info->port_id, msg, msg_size);
}

static void send_image_msg(struct ali_pe_info *info)
{
	struct pe_image_info *image = &info->image;
	unsigned char msg[11];
	int msg_size = 11;
	
	msg[0] = ALIPE_MSG_IMAGE;
	msg[1] = 0;
	msg[2] = 8;
	memcpy((void *)(msg + 3), (void *)&(image->par1), 8);

	PE_PRF("%s : in\n", __FUNCTION__);	
	ali_transport_send_msg(info->port_id, msg, msg_size);
}

static void send_music_msg(struct ali_pe_info *info)
{
	struct pe_music_info *music = &info->music;
	unsigned char msg[11];
	int msg_size = 11;

	msg[0] = ALIPE_MSG_MUSIC;
	msg[1] = 0;
	msg[2] = 8;
	memcpy((void *)(msg + 3), (void *)&(music->par1), 8);

	PE_PRF("%s : in\n", __FUNCTION__);	
	ali_transport_send_msg(info->port_id, msg, msg_size);
}

static void send_video_msg(struct ali_pe_info *info)
{
	struct pe_video_info *video = &info->video;
	unsigned char msg[11];
	int msg_size = 11;
	
	msg[0] = ALIPE_MSG_VIDEO;
	msg[1] = 0;
	msg[2] = 8;
	memcpy((void *)(msg + 3), (void *)&(video->par1), 8);

	PE_PRF("%s : in\n", __FUNCTION__);	
	ali_transport_send_msg(info->port_id, msg, msg_size);
}

static void send_msg(struct ali_pe_info *info, unsigned long flag)
{
	if(flag & ALI_PE_CMD_FLAG_CACHE)
		send_cache_msg(info);

	if(flag & ALI_PE_CMD_FLAG_IMAGE)
		send_image_msg(info);
		
	if(flag & ALI_PE_CMD_FLAG_MUSIC)
		send_music_msg(info);

	if(flag & ALI_PE_CMD_FLAG_VIDEO)
		send_video_msg(info);	
}

static int run_routine(struct ali_pe_info *info)
{
	int ret = 0;
	
	ret += ali_pe_cache_routine(info);
	
	return ret;
}

static void moniter_routine(struct ali_pe_info *info, enum ali_pe_status status)
{
	ali_pe_cache_mon_routine(info, status);
	ali_pe_image_routine(info);
	ali_pe_music_routine(info);
	ali_pe_video_routine(info);
}

static int pe_thread(void *par)
{
	struct ali_pe_info *info = NULL;
	enum ali_pe_status status;	
	int ret = 0;

	while(!kthread_should_stop() && g_ali_pe_live){
		//PE_PRF("enter pe thread status %d\n", info->status);
		info = (struct ali_pe_info *)m_pe_info;
		if(info->flag != 0){
			//PE_PRF("%s : get new msg %x\n", __FUNCTION__, (int)info->flag);
			send_msg(info, info->flag);			

			if(info->flag & ALI_PE_CMD_FLAG_WORK)
			{
				LOCK_MUTEX;
				//PE_PRF("%s : let pe work\n", __FUNCTION__);
				info->status = ALI_PE_WORKING;
				UNLOCK_MUTEX;				
			}
			
			info->flag = 0;
		}

		LOCK_MUTEX;
		status = info->status;		
		UNLOCK_MUTEX;

		ret = 0;
		if(status == ALI_PE_WORKING)
			ret = run_routine(info);
		if(ret > 0){
			int i = 0;
			
			LOCK_MUTEX;
			
			for(i = 0;i < ALI_PE_CACHE_NUM;i++)
				if(info->cache[i].busy)
					break;
				
			if(i >= ALI_PE_CACHE_NUM){
				PE_PRF("pause the pe thread\n");
				status = info->status = ALI_PE_PAUSED;
			}
			
			UNLOCK_MUTEX;
		}
		
		moniter_routine(info, status);
		
		if(info->flag == 0)
		{
			if(status == ALI_PE_WORKING)
				sleep_on_timeout(&info->wait_que, 5);
			else
				sleep_on_timeout(&info->wait_que, 10);
		}
	}
	
	return -1;
}

static ssize_t ali_pe_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	struct ali_pe_info *info = (struct ali_pe_info *)file->private_data;
	enum ali_pe_status status;
	int ret = 0;

	LOCK_MUTEX;
	status = info->status;
	UNLOCK_MUTEX;

	if(status == ALI_PE_WORKING){
		ret = ali_pe_cache_write(info, (void *)buf, count);
		
		//PE_PRF("pe cache write %d\n", ret);
		return ret;
	}

	return -1;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long ali_pe_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int ali_pe_ioctl(struct inode *node,struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	struct ali_pe_info *info = (struct ali_pe_info *)file->private_data;
	int ret = 0;
	
	switch(cmd){
		case ALIPEIO_MISC_TRANSPORT_ID:
			info->port_id = (int)arg;
			break;
		case ALIPEIO_MISC_SET_FILE_SIZE:
			copy_from_user((void *)&(info->file_size_pending), (void *)arg
				, sizeof(info->file_size_pending));
			PE_PRF("pe file size %x%x\n",  *(((int *)&info->file_size_pending) + 1), *(int *)&info->file_size_pending);
			break;
		case ALIPEIO_MISC_PAUSE_MODULE:
			LOCK_MUTEX;			
			PE_PRF("pe egnine pause\n");
			info->status = ALI_PE_PAUSED;
			UNLOCK_MUTEX;
			break;
		case ALIPEIO_MISC_RESUME_MODULE:
			LOCK_MUTEX;
			PE_PRF("pe engine resume idx %d\n", (int)arg);
			if(arg == 0xFF)
				info->status = ALI_PE_WORKING;
			else
			{
				info->cache[arg].busy = 1;
			}
			UNLOCK_MUTEX;
			break;
		case ALIPEIO_MISC_FREE_CACHE:
			LOCK_MUTEX;			
			if(arg < ALI_PE_CACHE_NUM)
			{
				PE_PRF("free cache %d done\n", (int)arg);
				info->cache[arg].busy = 0;							
			}
			else
			{
				PE_PRF("free cache id %d fail\n", (int)arg);
			}
			UNLOCK_MUTEX;				
			break;
		case ALIPEIO_MISC_RPC_OPERATION:
		{
			struct ali_pe_rpc_pars pars;
			int i = 0;

			if(down_interruptible(&info->rpc_sem)){ 
				PE_PRF("ali_pe down sem fail\n"); 
				return -1; 
			}		
			
			copy_from_user((void *)&pars, (void *)arg, sizeof(pars));
			for(i = 0;i < pars.arg_num;i++)
			{
				g_ali_pe_rpc_arg_size[i] = pars.arg[i].arg_size;				
				if(g_ali_pe_rpc_arg_size[i] > 0)
				{
#if 0					
					g_ali_pe_rpc_arg[i] = kmalloc(g_ali_pe_rpc_arg_size[i]
						, GFP_KERNEL);
#endif
					if(g_ali_pe_rpc_arg[i] == NULL)
					{
						PE_PRF("allocate rpc arg buf fail\n");
						ret = -ENOMEM;
						goto RPC_EXIT;			
					}
					
					copy_from_user((void *)(g_ali_pe_rpc_arg[i]), pars.arg[i].arg, g_ali_pe_rpc_arg_size[i]);
				}
			}
			
			switch(pars.type)
			{
				case RPC_IMAGE:
					ret = ali_pe_image_operation(pars.API_ID);
					break;
				case RPC_MUSIC:
					ret = ali_pe_music_operation(pars.API_ID);
					break;	
				case RPC_VIDEO:
					ret = ali_pe_video_operation(pars.API_ID);
					break;
				default:
					ret = -1;
					break;
			}
			
RPC_EXIT:			
			for(i = 0;i < pars.arg_num;i++)
			{
				if(g_ali_pe_rpc_arg_size[i] > 0)
				{
					if(pars.arg[i].out)
						copy_to_user(pars.arg[i].arg, (void *)(g_ali_pe_rpc_arg[i]), g_ali_pe_rpc_arg_size[i]);
				}

#if 0				
				if(g_ali_pe_rpc_arg[i] != NULL)
					kfree((void *)(g_ali_pe_rpc_arg[i]));
#endif				
			}

			up(&info->rpc_sem);
			
			break;
		}
		case ALIPEIO_MISC_SELECT_CACHE:
			info->current_cache_idx = (int)arg;
			break;
		case ALIPEIO_MISC_GET_FREE_CACHE:
		{
			int i = 0;

			for(i = 0;i < ALI_PE_CACHE_NUM;i++)
			{
				if(!info->cache[i].busy)
					break;
			}
			
			if(i < ALI_PE_CACHE_NUM)
			{
				PE_PRF("get the free cache %d\n",i);
				ret = i;
			}
			else
			{
				PE_PRF("get the free cache fail\n");
				ret = -1;
			}
			break;
		}
		case ALIPEIO_MISC_GET_PE_MEM_INFO:
		{
			struct ali_pe_mem_info info;

			memset((void *)&info, 0, sizeof(info));
			info.mem_size = g_ali_pe_buf_size;
			info.mem_start = (void *)(g_ali_pe_buf_start_addr & 0x0FFFFFFF);
			copy_to_user((void *)arg, (void *)&info, sizeof(info));
			break;
		}
		default:
			ret = -1;
			break;
	}
		
	return ret;
}

static int ali_pe_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long start = vma->vm_start;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long pos;

	if (size > g_ali_pe_buf_size) {
		PE_PRF("fail size %x smem_len %x\n", (int)size, (int)g_ali_pe_buf_size);
		return -EINVAL;
	}

	pos = (unsigned long)(g_ali_pe_buf_start_addr & 0x0FFFFFFF);
	if (remap_pfn_range(vma, start, (pos>>PAGE_SHIFT), size, vma->vm_page_prot)) {
		PE_PRF("remap pfn fail\n");
		return -EINVAL;
	}
	
	PE_PRF("virtual add %x phy addr %x\n", (int)start, (int)pos);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
#else	
	vma->vm_flags |= VM_RESERVED | VM_IO;
#endif
	return 0;	
}

static int ali_pe_open(struct inode *inode, struct file *file)
{
	struct ali_pe_info *info = NULL;
	int ret = 0;
	int i = 0;

	if(g_ali_pe_open_cnt != 0)
	{
		file->private_data = (void *)m_pe_info;	
        g_ali_pe_open_cnt++;
		return 0;
	}
	
	m_pe_info = info = kmalloc(sizeof(*info), GFP_KERNEL);
	if(info == NULL){
		PE_PRF("kmalloc pe info fail\n");
		return -1;
	}
	memset((void *)info, 0, sizeof(*info));
	
	sema_init(&info->semaphore, 1);
	sema_init(&info->rpc_sem, 1);
	init_waitqueue_head(&info->wait_que);	

	ali_pe_cache_init(info);
	ali_pe_image_init(info);
	ali_pe_music_init(info);
	ali_pe_video_init(info);
	
	info->thread_pe = kthread_create(
		pe_thread, (void *)info, "ali_pe");
	if(IS_ERR(info->thread_pe)){
		PE_PRF("pe kthread create fail\n");
		ret = -EBUSY;
		goto EXIT;
	}
	wake_up_process(info->thread_pe);

	if(g_ali_pe_rpc_arg[0] == NULL){
		for(i = 0;i < MAX_PE_RPC_ARG_NUM;i++){
			g_ali_pe_rpc_arg[i] = kmalloc(MAX_PE_RPC_ARG_SIZE, GFP_KERNEL);
			if(g_ali_pe_rpc_arg[i] == NULL){
				PE_PRF("ali pe malloc rpc arg buf fail\n");
				ret = -ENOMEM;
				goto EXIT;
			}
		}
	}
	
	info->status = ALI_PE_INITED;
	
	file->private_data = (void *)info;

    g_ali_pe_open_cnt++;
	
EXIT:	
	PE_PRF("open the ali pe done %d\n", ret);	
	return ret;
}

static int ali_pe_release(struct inode *inode, struct file *file)
{
	struct ali_pe_info *info = (struct ali_pe_info *)file->private_data;
	int i = 0;

    if(g_ali_pe_open_cnt == 0)
        return 0;

    g_ali_pe_open_cnt--;

	if(g_ali_pe_open_cnt == 0){
		if(g_ali_pe_rpc_arg[0] != NULL){
			for(i = 0;i < MAX_PE_RPC_ARG_NUM;i++){
				if(g_ali_pe_rpc_arg[i] != NULL){
					kfree(g_ali_pe_rpc_arg[i]);
					g_ali_pe_rpc_arg[i] = NULL;
				}
			}
		}
	
		// kthread_stop(info->thread_pe);  /* Deadlock here, don't know why */
		g_ali_pe_live = 0;
		ali_pe_cache_release(info);
		kfree(info);
		m_pe_info = NULL;
	}

	return 0;
}

static const struct file_operations ali_pe_fops = {
	.owner =	THIS_MODULE,
	.write = ali_pe_write,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	.unlocked_ioctl = ali_pe_ioctl,
#else
	.ioctl = ali_pe_ioctl,
#endif	
	.mmap = ali_pe_mmap,
	.open = ali_pe_open,
	.release = ali_pe_release,
};

static dev_t m_pe_dev_t = 0;
static struct cdev *m_pe_cdev = NULL;
static struct class *m_pe_class = NULL;
static struct device *m_pe_device = NULL;

extern unsigned long __G_ALI_MM_APE_MEM_SIZE;
extern unsigned long __G_ALI_MM_APE_MEM_START_ADDR;

static int __init ali_pe_init(void)
{
	int i  = 0;
	
	if(alloc_chrdev_region(&m_pe_dev_t, 0, 1, "ali_pe") < 0){
		PE_PRF("allocate ali pe dev_t fail\n");
		return 0;
	}

	m_pe_cdev = cdev_alloc();
	if(m_pe_cdev == NULL){
		PE_PRF("ali pe cdev alloc fail\n");
		return 0;
	}
	
	m_pe_cdev->ops = &ali_pe_fops;
	m_pe_cdev->owner = ali_pe_fops.owner;
	kobject_set_name(&m_pe_cdev->kobj, "%s", "ali_pe");
	if(cdev_add(m_pe_cdev, m_pe_dev_t, 1) < 0){
		PE_PRF("ali pe cdev add fail\n");
		goto FAIL;
	}

	m_pe_class = class_create(THIS_MODULE, "ali_pe");
	if(m_pe_class == NULL){
		PE_PRF("ali pe create class fail\n");
		goto FAIL;
	}
	
	m_pe_device  = device_create(m_pe_class, NULL, m_pe_dev_t, NULL
		, "ali_pe0");
	if(m_pe_device == NULL){
		PE_PRF("ali pe create device fail\n");
		goto FAIL;
	}
	
	g_ali_pe_buf_size = __G_ALI_MM_APE_MEM_SIZE;
	g_ali_pe_buf_start_addr = __G_ALI_MM_APE_MEM_START_ADDR;

	for(i = 0;i < MAX_PE_RPC_ARG_NUM;i++)
		g_ali_pe_rpc_arg[i] = NULL;
	
	PE_PRF("init ali pe done major %d\n", MAJOR(m_pe_dev_t));
	return 0;
	
FAIL:
	if(m_pe_cdev != NULL)
		cdev_del(m_pe_cdev);
	if(m_pe_dev_t != 0)
		unregister_chrdev_region(m_pe_dev_t, 1);
	if(m_pe_device != NULL)
		device_del(m_pe_device);
	if(m_pe_class != NULL)
		class_destroy(m_pe_class);

	PE_PRF("init ali pe fail\n");	
	return 0;
}

 static void __exit ali_pe_exit(void)
{
	if(m_pe_device != NULL)
		device_del(m_pe_device);

	if(m_pe_class != NULL)
		class_destroy(m_pe_class);

	if(m_pe_cdev != NULL)
		cdev_del(m_pe_cdev);

	if(m_pe_dev_t != 0)
		unregister_chrdev_region(m_pe_dev_t, 1);
}

 module_init(ali_pe_init);
 module_exit(ali_pe_exit);
 
MODULE_AUTHOR("ALi (Shanghai) Corporation");
MODULE_DESCRIPTION("ali play engine driver");
MODULE_LICENSE("GPL");

