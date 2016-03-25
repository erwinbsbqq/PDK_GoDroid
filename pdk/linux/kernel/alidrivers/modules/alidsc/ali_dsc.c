#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/semaphore.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/pagemap.h>

#include "ali_dsc_lld.h"

long ali_m36_dsc_ioctl(struct file *filp, __u32 cmd, unsigned long arg);
int ali_m36_dsc_open(struct inode *inode, struct file  *file);
int ali_m36_dsc_release(struct inode *inode, struct file  *file);
int ali_m36_dsc_mmap(struct file *filp, struct vm_area_struct *vma);

struct ali_m36_dsc_dev g_ali_dsc_devices[1];

static struct file_operations g_ali_dsc_fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = ali_m36_dsc_ioctl,
	.open = ali_m36_dsc_open,
	.release = ali_m36_dsc_release,
	.mmap = ali_m36_dsc_mmap,
};

static void ali_dsc_record_stream_id(struct file *filp, __u32 id, __u32 cmd)
{
	struct ali_m36_dsc_dev *dev = (struct ali_m36_dsc_dev *)filp->private_data;
	__u32 i = 0;
	__u32 id_exist = 0;

	if(id>ALI_DSC_STREAM_ID_MAX)
	{
		return;
	}

	for(i=0;i<ALI_DSC_LLD_MAX_ITEM;i++)
	{
		if(dev->rec[i].stream_id.id == (__u32)id)
		{
			dev->rec[i].stream_id.filp = (__u32)filp;
			id_exist = 1;
			break;
		}
	}
	
	if(ALI_DSC_RESOURCE_BUSY == cmd)
	{
		if(id_exist)
		{
			return;
		}
		
		for(i=0;i<ALI_DSC_LLD_MAX_ITEM;i++)
		{
			if(!dev->rec[i].stream_id.filp)
			{
				dev->rec[i].stream_id.filp = (__u32)filp;
				dev->rec[i].stream_id.id = id;
				break;
			}
		}
	}
	else if(ALI_DSC_RESOURCE_IDLE == cmd)
	{
		if(i<ALI_DSC_LLD_MAX_ITEM)
		{
			dev->rec[i].stream_id.filp = 0;
		}
	}
}

static void ali_dsc_record_subdev_id(struct file *filp, __u32 id, __u32 mode, __u32 cmd)
{
	struct ali_m36_dsc_dev *dev = (struct ali_m36_dsc_dev *)filp->private_data;
	__u32 i = 0;
	__u32 id_exist = 0;
	
	if((id>VIRTUAL_DEV_NUM) || (mode > CSA))
	{
		return;
	}	

	for(i=0;i<ALI_DSC_LLD_MAX_ITEM;i++)
	{
		if( (dev->rec[i].dev_id.id == (__u32)id) && \
			(dev->rec[i].dev_id.mode == (__u32)mode) )
		{
			dev->rec[i].dev_id.filp = (__u32)filp;
			id_exist = 1;
			break;
		}
	}	
	
	if(ALI_DSC_RESOURCE_BUSY == cmd)
	{
		if(id_exist)
		{
			return;
		}
		
		for(i=0;i<ALI_DSC_LLD_MAX_ITEM;i++)
		{
			if(!dev->rec[i].dev_id.filp)
			{
				dev->rec[i].dev_id.filp = (__u32)filp;
				dev->rec[i].dev_id.id = id;
				dev->rec[i].dev_id.mode = mode;
				break;
			}
		}
	}
	else if(ALI_DSC_RESOURCE_IDLE == cmd)
	{
		if(i<ALI_DSC_LLD_MAX_ITEM)
		{
			dev->rec[i].dev_id.filp = 0;
		}
	}
}

static void ali_dsc_record_pvr_key_id(struct file *filp, __u32 id, __u32 cmd)
{
	struct ali_m36_dsc_dev *dev = (struct ali_m36_dsc_dev *)filp->private_data;
	__u32 i = 0;
	__u32 id_exist = 0;

	if(id > ALI_DSC_STREAM_ID_MAX)
	{
		return;
	}

	for(i=0;i<ALI_DSC_LLD_MAX_ITEM;i++)
	{
		if(dev->rec[i].pvr_key_id.id == (__u32)id)
		{
			dev->rec[i].pvr_key_id.filp = (__u32)filp;
			id_exist = 1;
			break;
		}
	}
	
	if(ALI_DSC_RESOURCE_BUSY == cmd)
	{
		if(id_exist)
		{
			return;
		}
		
		for(i=0;i<ALI_DSC_LLD_MAX_ITEM;i++)
		{
			if(!dev->rec[i].pvr_key_id.filp)
			{
				dev->rec[i].pvr_key_id.filp = (__u32)filp;
				dev->rec[i].pvr_key_id.id = id;
				break;
			}
		}
	}
	else if(ALI_DSC_RESOURCE_IDLE == cmd)
	{
		if(i < ALI_DSC_LLD_MAX_ITEM)
		{
			dev->rec[i].pvr_key_id.filp = 0;
		}
	}
}

long ali_m36_dsc_ioctl
(

    struct file   *filp,
    __u32   cmd,
    unsigned long  arg
)
{
	int ret = RET_SUCCESS;
	struct ali_m36_dsc_dev *dsc_dev = NULL;
	ALI_DSC_ID_INFO dsc_id;
	__u32 see_id = 0;
	ALI_DSC_KREC_MEM krec_mem;
	DSC_VER_CHK_PARAM ver_mem;
	DSC_PVR_KEY_PARAM pvr_key;
	ALI_RE_ENCRYPT re_encrypt;
	ALI_DSC_RAM_MON ram_mon;
	__u8 data[ALI_DSC_SEE_DRV_VER_SIZE];
	__u32 i = 0;
	__u32 kmem_num = 1;

	dsc_dev = (struct ali_m36_dsc_dev *)filp->private_data;
	if (NULL == dsc_dev)
	{
		ALI_DSC_DEBUG_ERR("filp->private_data is NULL\n");
		return(-EIO);
	}
	
	if(down_interruptible(&dsc_dev->ioctl_sem))
	{
		ALI_DSC_DEBUG_ERR("get ioctl_sem error\n");
		return(-EINTR);
	}

	ALI_DSC_DEBUG("cmd = %x\n",cmd);
	
	switch(DSC_IO_CMD(cmd)) 
	{
		case DSC_IO_CMD(IO_REQUEST_KREC_SPACE):
		{
			ret = ali_dsc_umemcpy(&krec_mem, (__user ALI_DSC_KREC_MEM *)arg, \
									sizeof(ALI_DSC_KREC_MEM));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			
			kmem_num = (krec_mem.size / MEM_ALLOC_UNIT) + ((krec_mem.size % MEM_ALLOC_UNIT)?1:0);
			if (kmem_num > (DSC_MEM_BLOCK_COUNT)) 
			{
				ALI_DSC_DEBUG_ERR("Err: IO_REQUEST_KREC_SPACE size err\n");
				ret = RET_FAILURE;
				goto DONE;    
			}
			
			kmem_num = !kmem_num?1:((1<<kmem_num) -1);
			
			for(i = 0; i < (DSC_MEM_BLOCK_COUNT); i++)
			{
				if(!(dsc_dev->kmem.bitmap & (__u32)(kmem_num << i)))
				{
					dsc_dev->kmem.addr[i] = (__u32)dsc_dev->kmem.kcpu_addr + MEM_ALLOC_UNIT * i;
					dsc_dev->kmem.bitmap |= (__u32)(kmem_num << i);
					krec_mem.pa_mem = (void *)((__u32)dsc_dev->kmem.kcpu_addr + MEM_ALLOC_UNIT * i);
					ALI_DSC_DEBUG("request memory, addr=%x, bitmap=%x\n", \
										(__u32)krec_mem.pa_mem, dsc_dev->kmem.bitmap);
					break;
				}
			}

			if(i == (DSC_MEM_BLOCK_COUNT))
			{
				ALI_DSC_DEBUG_ERR("cannot get dsc_kmem\n");
				ret = RET_FAILURE;
				goto DONE;
			}
			
			ret = ali_dsc_umemcpy((__user ALI_DSC_KREC_MEM *)arg, &krec_mem, \
										sizeof(ALI_DSC_KREC_MEM));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			break;
		}
		
		case DSC_IO_CMD(IO_RELEASE_KREC_SPACE):
		{
			ret = ali_dsc_umemcpy(&krec_mem, (__user ALI_DSC_KREC_MEM *)arg, \
									sizeof(ALI_DSC_KREC_MEM));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			
			kmem_num = (krec_mem.size / MEM_ALLOC_UNIT) + ((krec_mem.size % MEM_ALLOC_UNIT)?1:0);
			if (kmem_num > (DSC_MEM_BLOCK_COUNT)) 
			{
				ALI_DSC_DEBUG_ERR("Err: IO_RELEASE_KREC_SPACE size err\n");
				ret = RET_FAILURE;
				goto DONE;  
			}
			
			kmem_num = !kmem_num?1:((1<<kmem_num) -1);
			
			if (NULL != krec_mem.pa_mem)
			{
				for(i = 0; i < (DSC_MEM_BLOCK_COUNT); i++)
				{
					if((__u32)dsc_dev->kmem.addr[i] == (__u32)krec_mem.pa_mem)
					{
						dsc_dev->kmem.bitmap &= ~(__u32)(kmem_num << i);
						dsc_dev->kmem.addr[i] = 0;
						ALI_DSC_DEBUG("release memory, addr=%x, bitmap=%x\n", \
										(__u32)krec_mem.pa_mem, dsc_dev->kmem.bitmap);
						break;
					}
				}
				if(i == (DSC_MEM_BLOCK_COUNT))
				{
					ALI_DSC_DEBUG_ERR("cannot release dsc_kmem\n");
					ret = RET_FAILURE;
					goto DONE;
				}
			}
			else
			{
				ALI_DSC_DEBUG_ERR("invalid mem addr\n");
				ret = RET_FAILURE;
				goto DONE;	
			}
			break;
		}
		
		case DSC_IO_CMD(IO_REQUEST_KREC_ADDR):
		{
			ret = ali_dsc_umemcpy((void __user *)arg, &dsc_dev->kmem.kcpu_addr, sizeof(__u32));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			break;
		}
		
		case DSC_IO_CMD(IO_GET_DEV_HLD):
		{
			ret = ali_dsc_umemcpy((void __user *)arg, &dsc_dev->see_dsc_id, sizeof(__u32)); 
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			break;
		 }
		
		case DSC_IO_CMD(IO_PARSE_DMX_ID_SET_CMD):  
		case DSC_IO_CMD(IO_DSC_SET_CLR_CMDQ_EN):
		case DSC_IO_CMD(IO_DSC_DELETE_HANDLE_CMD):
		case DSC_IO_CMD(IO_DSC_FIXED_DECRYPTION):
		case DSC_IO_CMD(IO_DSC_SYS_UK_FW):
		{
			ret = ali_dsc_ioctl(dsc_dev->see_dsc_id, DSC_IO_CMD(cmd), (__u32)arg);
			if((DSC_IO_CMD(IO_DSC_DELETE_HANDLE_CMD) == DSC_IO_CMD(cmd)) \
				&& (RET_SUCCESS == ret))
			{
				ali_dsc_record_handle_id(filp, (__u32)arg, ALI_DSC_RESOURCE_IDLE);
			}
			break;
		}

		case DSC_IO_CMD(IO_DSC_GET_DRIVER_VERSION):
		{
			memset(data, 0x00, sizeof(data));
			ret = ali_dsc_ioctl(dsc_dev->see_dsc_id, DSC_IO_CMD(cmd), (__u32)data);
			ali_dsc_umemcpy((void *)arg, data, sizeof(data));
			break;
		}

		case DSC_IO_CMD(IO_DSC_SET_PVR_KEY_PARAM):
		{
			ret = ali_dsc_umemcpy(&pvr_key, (__user DSC_PVR_KEY_PARAM *)arg, \
									sizeof(DSC_PVR_KEY_PARAM));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			ret = ali_dsc_ioctl(dsc_dev->see_dsc_id, DSC_IO_CMD(cmd), (__u32)&pvr_key);
			if(0 == ret)
			{
				ali_dsc_record_pvr_key_id(filp, (__u32)pvr_key.stream_id, ALI_DSC_RESOURCE_BUSY);
			}
			break;
		}
		
		case DSC_IO_CMD(IO_DSC_SET_PVR_KEY_IDLE):
		{
			ret = ali_dsc_ioctl(dsc_dev->see_dsc_id, DSC_IO_CMD(cmd), (__u32)arg);
			if(0 == ret)
			{
				ali_dsc_record_pvr_key_id(filp, (__u32)arg, ALI_DSC_RESOURCE_IDLE);
			}
			break;
		}
		
		case DSC_IO_CMD(IO_DSC_VER_CHECK):
		{
			ret = ali_dsc_umemcpy(&ver_mem, (__user DSC_VER_CHK_PARAM *)arg, \
									sizeof(DSC_VER_CHK_PARAM));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			ret = ali_dsc_umemcpy(dsc_dev->kernel_input_buffer, (__user __u8 *)ver_mem.input_mem, \
									ver_mem.len);
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			ver_mem.input_mem = (__u32)UC(dsc_dev->kernel_input_buffer);
			ret = ali_dsc_ioctl(dsc_dev->see_dsc_id, DSC_IO_CMD(cmd), (__u32)&ver_mem);
			break;
		}
		
		case DSC_IO_CMD(IO_PARSE_DMX_ID_GET_CMD) :  
		case DSC_IO_CMD(IO_DSC_GET_DES_HANDLE)   :  
		case DSC_IO_CMD(IO_DSC_GET_AES_HANDLE)   :  
		case DSC_IO_CMD(IO_DSC_GET_CSA_HANDLE)   :  
		case DSC_IO_CMD(IO_DSC_GET_SHA_HANDLE)   :  
		{
			ret = ali_dsc_ioctl(dsc_dev->see_dsc_id, DSC_IO_CMD(cmd), (__u32)(&see_id));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:get id/handle, ret:%d\n", ret);
				goto DONE;
			}
			
			ret = ali_dsc_umemcpy((void __user *)(arg), (void *)&see_id, sizeof(__u32));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}       
			break;
		}
		
		case DSC_IO_CMD(ALI_DSC_IO_TRIG_RAM_MON):
		{
			ret = ali_dsc_umemcpy((void *)&ram_mon, (void __user *)arg, \
									sizeof(ALI_DSC_RAM_MON));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			ret = ali_trig_ram_mon((__u32)ram_mon.start_addr, \
								(__u32)ram_mon.end_addr, \
								(__u32)ram_mon.interval, \
								(__u32)ram_mon.sha_mode, \
								(int)ram_mon.dis_or_en);
			break;	
		}

		case DSC_IO_CMD(ALI_DSC_IO_DEENCRYPT):  
		{
			ret = ali_dsc_umemcpy((void *)&re_encrypt, (void __user *)arg, \
									sizeof(ALI_RE_ENCRYPT));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}         

			if(re_encrypt.length*TS_PACKET_SIZE>PURE_DATA_MAX_SIZE)
			{
				ALI_DSC_DEBUG_ERR("Err: length outside\n");
				ret = ALI_DSC_ERROR_INVALID_PARAMETERS;
				goto DONE;
			}

			ret = ali_dsc_umemcpy(dsc_dev->kernel_input_buffer, \
								(void __user *)(re_encrypt.input), \
								(re_encrypt.length*TS_PACKET_SIZE));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}

			ret = ali_DeEncrypt((DEEN_CONFIG *)re_encrypt.p_deen, \
							dsc_dev->kernel_input_buffer, \
							dsc_dev->kernel_output_buffer, \
							(__u32)re_encrypt.length);
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_DeEncrypt() failed, ret:%d\n", ret);
				goto DONE;
			}

			ret = ali_dsc_umemcpy((void __user *)(re_encrypt.output), \
								dsc_dev->kernel_output_buffer, \
								(re_encrypt.length*TS_PACKET_SIZE)); 
			if (0 != ret)
			{
				ALI_DSC_DEBUG("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			break;  
		}

		case DSC_IO_CMD(ALI_DSC_IO_GET_FREE_STREAM_ID):
		{
			ret = ali_dsc_umemcpy(&dsc_id, \
								(void __user *)arg, \
								sizeof(ALI_DSC_ID_INFO));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			
			see_id = ali_dsc_get_free_stream_id((enum DMA_MODE)dsc_id.mode);
			ret = ali_dsc_umemcpy((void __user *)(dsc_id.id_number), \
								&see_id, \
								sizeof(__u32)); 
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, %x id=%d ret:%d\n", \
								dsc_id.id_number, see_id, ret);
				goto DONE;
			}
			if(ALI_INVALID_CRYPTO_STREAM_ID == see_id)
			{
				ret = see_id;
				goto DONE;
			}
			ali_dsc_record_stream_id(filp, (__u32)see_id, ALI_DSC_RESOURCE_BUSY);
			break;
		}        

		case DSC_IO_CMD(ALI_DSC_IO_GET_FREE_SUB_DEVICE_ID):
		{
			ret = ali_dsc_umemcpy(&dsc_id, \
								(void __user *)arg, \
								sizeof(ALI_DSC_ID_INFO));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}     

			see_id = ali_dsc_get_free_sub_device_id((__u8)dsc_id.mode);

			ret = ali_dsc_umemcpy((void __user *)(dsc_id.id_number), &see_id, \
								sizeof(__u32));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, %x id=%d ret:%d\n", \
				               		dsc_id.id_number, see_id, ret);
				goto DONE;
			}      
			if(ALI_INVALID_DSC_SUB_DEV_ID == see_id)
			{
				ret = see_id;
				goto DONE;
			}
			ali_dsc_record_subdev_id(filp, (__u32)see_id, \
								(__u32)dsc_id.mode, ALI_DSC_RESOURCE_BUSY);
			break;
		}
					
		case DSC_IO_CMD(ALI_DSC_IO_SET_SUB_DEVICE_ID_IDLE):
		{
			ret = ali_dsc_umemcpy(&dsc_id, \
								(void __user *)arg, \
								sizeof(ALI_DSC_ID_INFO));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			
			ret = ali_dsc_set_sub_device_id_idle((__u8)dsc_id.mode,dsc_id.id_number);
			if(0 == ret)
			{
				ali_dsc_record_subdev_id(filp, (__u32)dsc_id.id_number, \
					(__u32)dsc_id.mode, ALI_DSC_RESOURCE_IDLE);
			}
								
			break;
		}
		
		case DSC_IO_CMD(ALI_DSC_IO_SET_STREAM_ID_IDLE):
		{ 
			ret = ali_dsc_set_stream_id_idle((__u32)arg);
			if(0 == ret)
			{
				ali_dsc_record_stream_id(filp, (__u32)arg, ALI_DSC_RESOURCE_IDLE);
			}
			break;
		}
		
		case DSC_IO_CMD(ALI_DSC_IO_SET_STREAM_ID_USED):
		{
			ali_dsc_set_stream_id_used((__u32)arg);
			ali_dsc_record_stream_id(filp, (__u32)arg, ALI_DSC_RESOURCE_BUSY);
			break;
		}
		
		case DSC_IO_CMD(ALI_DSC_IO_SET_SUB_DEVICE_ID_USED):
		{
			ret = ali_dsc_umemcpy(&dsc_id, (void __user *)arg, \
									sizeof(ALI_DSC_ID_INFO));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}   
			ret = ali_dsc_set_sub_device_id_used(dsc_id.mode, dsc_id.id_number);
			if(0 == ret)
			{
				ali_dsc_record_subdev_id(filp, (__u32)dsc_id.id_number, \
					(__u32)dsc_id.mode, ALI_DSC_RESOURCE_BUSY);
			}
			break;
		}    

		default:
		{
			ALI_DSC_DEBUG_ERR("Err:Parameter Error, cmd=0x%x\n",cmd);
			ret = ALI_DSC_ERROR_OPERATION_NOT_SUPPORTED;
			break;
		}
	}
	
DONE:
	up(&dsc_dev->ioctl_sem);
	return(ret);
}



int ali_m36_dsc_open
(
	struct inode *inode,
	struct file  *file
)
{
	struct ali_m36_dsc_dev *dsc = NULL;

	dsc = container_of(inode->i_cdev, struct ali_m36_dsc_dev, cdev);

	if (NULL == dsc || down_interruptible(&dsc->ioctl_sem))
	{
		ALI_DSC_DEBUG("get ioctl_sem error\n");      
		return(-EINTR);
	}
	
	file->private_data = (void *)dsc;	

	up(&dsc->ioctl_sem);

	return (0);
}



int ali_m36_dsc_release
(
	struct inode *inode,
	struct file  *file
)
{
	struct ali_m36_dsc_dev *dev = NULL;
	__u32 i = 0;

	dev = file->private_data;
	//dsc = container_of(inode->i_cdev, struct ali_m36_dsc_dev, cdev);

	if(NULL == dev)
	{
		ALI_DSC_DEBUG_ERR("Err: get dsc dev failed\n");
		return(-EIO);
	}
	
	down(&dev->ioctl_sem);

	for(i=0;i<ALI_DSC_LLD_MAX_ITEM;i++)
	{
		if(dev->rec[i].stream_id.filp == (__u32)file)
		{
			ali_dsc_set_stream_id_idle((__u32)dev->rec[i].stream_id.id);
			dev->rec[i].stream_id.filp = 0;
			ALI_DSC_DEBUG_ERR("dev->rec[%d].stream_id.id = %d\n",i,dev->rec[i].stream_id.id);
		}
		
		if(dev->rec[i].dev_id.filp == (__u32)file)
		{
			ali_dsc_set_sub_device_id_idle((__u8)dev->rec[i].dev_id.mode,dev->rec[i].dev_id.id);
			ALI_DSC_DEBUG_ERR("dev->rec[%d].dev_id.id = %d,dev->rec[%d].dev_id.mode = %d\n", \
				i,dev->rec[i].dev_id.id,i,dev->rec[i].dev_id.mode);
			dev->rec[i].dev_id.filp = 0;
		}

		if(dev->rec[i].pvr_key_id.filp == (__u32)file)
		{
			ali_dsc_ioctl(dev->see_dsc_id, IO_DSC_SET_PVR_KEY_IDLE, dev->rec[i].pvr_key_id.id);
			ALI_DSC_DEBUG_ERR("dev->rec[%d].pvr_key_id.id = %d\n",i,dev->rec[i].pvr_key_id.id);
			dev->rec[i].pvr_key_id.filp = 0;
		}
	}

	file->private_data = NULL;
	
	up(&dev->ioctl_sem);
	return(0);
}

int ali_m36_dsc_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret = -1;
	struct ali_m36_dsc_dev *dsc = filp->private_data;

	if(NULL == dsc)
	{
		return -EIO;
	}
	
	ret = remap_pfn_range(vma, vma->vm_start, \
						virt_to_phys((void *)(dsc->kmem.kcpu_addr)) >> PAGE_SHIFT, \
						vma->vm_end -  vma->vm_start,  PAGE_SHARED);
	if(ret != 0)
	{
		return -EAGAIN;
	}
	
	return 0;
}


static int __init ali_m36_dsc_init
(
    void
)
{
	int result = -1;
	struct device *clsdev = NULL;
	struct ali_m36_dsc_dev *dsc = g_ali_dsc_devices;
	struct class *ali_dsc_class = NULL;
#ifdef CONFIG_MIPS	
	__u32 dsc_virt_addr = 0;
	//__u32 dsc_page = 0;
#endif
	memset(dsc, 0x00, sizeof(struct ali_m36_dsc_dev));
	
	sema_init(&dsc->ioctl_sem, 1);

	result = alloc_chrdev_region(&dsc->devt, 0, 1, ALI_DSC_DEV);
	if (result < 0)
	{
		ALI_DSC_DEBUG_ERR("Dsc kernel error\n");
		goto fail;
	}

	cdev_init(&(dsc->cdev), &g_ali_dsc_fops);

	dsc->cdev.owner = THIS_MODULE;

	result = cdev_add(&dsc->cdev, dsc->devt, 1);
	if (result < 0)
	{
		ALI_DSC_DEBUG_ERR("Dsc kernel error:cdev_add() failed,result:%d\n", result);
		goto fail;
	}

	ali_dsc_class = class_create(THIS_MODULE, "ali_dsc_class");
	if (IS_ERR(ali_dsc_class))
	{
		result = PTR_ERR(ali_dsc_class);
		goto fail;
	}
	dsc->dev_class = ali_dsc_class;

	clsdev = device_create(ali_dsc_class, NULL, dsc->devt, dsc, ALI_DSC_DEV);
	if (IS_ERR(clsdev))
	{
		ALI_DSC_DEBUG_ERR("ali_dsc_0 device_create() failed!\n");
		result = PTR_ERR(clsdev);
		goto fail;
	}
	dsc->dev = clsdev;
	//open see module for dsc
	//ali_m36_dsc_see_init();
	
	dsc->see_dsc_id = hld_dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	if (NULL == dsc->see_dsc_id)
	{
		ALI_DSC_DEBUG_ERR("Dsc kernel error\n");
		return(ALI_DSC_ERROR_INVALID_DEV);
	}


#if defined(CONFIG_ARM)
	dsc->kernel_input_buffer = (__u8 *)__G_ALI_MM_DSC_MEM_START_ADDR;
#else
	dsc_virt_addr = (__u32)kmalloc(ALI_DSC_KERNEL_BUFFER_SIZE*2+0x1f, GFP_KERNEL);

	dsc->kernel_input_buffer = (__u8 *)((dsc_virt_addr&((__u32)~0x1f)) + 0x20);//align cache line

	if ((NULL == dsc->kernel_input_buffer))
	{
		ALI_DSC_DEBUG_ERR("Dsc kernel error:kmalloc fail\n");
		return(-ENOMEM);
	}
#endif	
	dsc->kernel_output_buffer = dsc->kernel_input_buffer + 	ALI_DSC_KERNEL_BUFFER_SIZE;
	dsc->dsc_key = dsc->kernel_output_buffer - ALI_DSC_KERNEL_KEY_SIZE;

#ifdef CONFIG_MIPS // has bug when 512MB DDR ??
	dsc->dev->coherent_dma_mask = ALI_DSC_COHERENT_DMA_MASK;
	dsc->kmem.kcpu_addr = dma_alloc_coherent(dsc->dev,ALI_DSC_KERNEL_BUFFER_SIZE, \
					&dsc->kmem.kdma_addr,GFP_KERNEL | GFP_DMA);
	if(NULL == dsc->kmem.kcpu_addr)
	{
		ALI_DSC_DEBUG_ERR("Dsc kernel error - dma_alloc_coherent failed\n");
		goto fail;
	}
#else
	dsc->kmem.kcpu_addr = kmalloc(ALI_DSC_KERNEL_BUFFER_SIZE, GFP_KERNEL | GFP_DMA);
	if(NULL == dsc->kmem.kcpu_addr)
	{
		ALI_DSC_DEBUG_ERR("Dsc kernel error - dma_alloc_coherent failed\n");
		goto fail;
	}
	arm_remap_lowmem_nocache(__pa(dsc->kmem.kcpu_addr), ALI_DSC_KERNEL_BUFFER_SIZE);
#endif

	ALI_DSC_DEBUG("\n\nkcpu_addr = 0x%p, kdma_addr = 0x%x\n\n", dsc->kmem.kcpu_addr, dsc->kmem.kdma_addr);
	ALI_DSC_DEBUG("\n\n__pa of kcpu_addr = 0x%x\n\n", __pa(dsc->kmem.kcpu_addr));

	return(0);

fail:
	return(-1);
}

static void __exit ali_m36_dsc_exit(void)
{
	struct ali_m36_dsc_dev *dsc = g_ali_dsc_devices;
	ALI_DSC_DEBUG("DSC kernel exit\n");

	unregister_chrdev_region(dsc->devt, 1);
	device_destroy(dsc->dev_class, dsc->devt);
	cdev_del(&dsc->cdev);    
	class_destroy(dsc->dev_class);
	dma_free_coherent(dsc->dev, ALI_DSC_KERNEL_BUFFER_SIZE, \
			dsc->kmem.kcpu_addr, dsc->kmem.kdma_addr);
#if !defined(CONFIG_ARM)
	kfree(dsc->kernel_input_buffer);
#endif
}

device_initcall(ali_m36_dsc_init);
module_exit(ali_m36_dsc_exit);

MODULE_AUTHOR("ALi (Zhuhai) Corporation");
MODULE_DESCRIPTION("ALi Descramble Scramble Core");
MODULE_LICENSE("GPL");


