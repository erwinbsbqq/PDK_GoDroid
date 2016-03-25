/*******************************************************************************

File name   : ali_venc.c

Description : Video driver stack LINUX platform OS driver source file

Author      : Vic Zhang <Vic.Zhang@Alitech.com>

Create date : Oct 18, 2013

COPYRIGHT (C) ALi Corporation 2013

Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 18, 2013                   Created               V0.1             Vic.Zhang

*******************************************************************************/
#include <linux/version.h> 
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
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/semaphore.h>
#include <rpc_hld/ali_rpc_venc.h>
#include <linux/ali_venc.h>
#include <ali_board_config.h>

#include <linux/syscalls.h>
#include <linux/module.h>



#if 0
#define VENC_PRF(arg, value...)  \
			{\
				printk("<0>""kernel debug : file : %s function : %s line %d\n", __FILE__, __FUNCTION__, __LINE__);\
				printk("<0>"arg, ##value);\
				printk("<0>""kernel debug done\n\n");\
			}
#else
#define VENC_PRF(...) 	do{}while(0)
#endif

/* Per-device (per-venc) structure */
struct venc_dev
{
    struct cdev cdev;
    int venc_number;
    char name[10];
    int status;
    int open_count;
    unsigned long _MM_VENC_PRIVATE_BUFFER_;
    unsigned long _MM_VENC_PRIVATE_BUFFER_SIZE_;
    unsigned long _MM_VENC_SBM_BUFFER_;
    unsigned long _MM_VENC_SBM_BUFFER_SIZE_;
};

#define BS_LIMIT_SIZE   0x300000    /* Limit the size of bitstream is 0x300000. */
static unsigned long bs_phy_addr = 0;

#define DEVICE_NAME                "ali_venc"
static dev_t venc_dev_number;       /* Allocated device number */
static struct class *venc_class;    /* Tie with the device model */
static struct device *venc_device;
static struct venc_dev *venc_priv[VENC_NUM];
static struct semaphore m_venc_sem;

#if 0

static int KernelWriteFile(const unsigned long inAddr, const int lenData)
{
    int fd;

    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);

    fd = sys_open("/tmp/vic", O_CREAT | O_RDWR, 0);
    if ( fd >= 0)
    {
        sys_write(fd, (unsigned char*)inAddr, lenData);

    }
    else 
    {
        return -1;    
    }
    sys_close(fd);

    set_fs(old_fs);

    return 0;

}

#endif

/*******************************************************************************
Name        : venc_open
Description : Be invoked to open encoder device in the low level.
Parameters  :
Assumptions :
Limitations :
Returns     :
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Nov 12, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Nov 12, 2013                   Created               V0.1             Vic.Zhang
*******************************************************************************/
static int venc_open(struct inode *inode, struct file *file)
{
    struct venc_dev *venc_dev_p;

    down(&m_venc_sem);

    /* Get the per-device structure that contains this cdev */
    venc_dev_p = container_of(inode->i_cdev, struct venc_dev, cdev);

    /* Easy access to venc_dev_p from the entry points */
    file->private_data = venc_dev_p;

    if(venc_dev_p->open_count == 0)
    {
        /* Initialize some fields */
        venc_dev_p->status = 0;
    }

    venc_dev_p->open_count++;

    venc_dev_p->_MM_VENC_PRIVATE_BUFFER_SIZE_ = 0x1000000; //16M
    venc_dev_p->_MM_VENC_SBM_BUFFER_SIZE_ = 0x600000; //6M
    venc_dev_p->_MM_VENC_PRIVATE_BUFFER_ = __G_ALI_MM_VENC_MEM_START_ADDR;
    if (__G_ALI_MM_VENC_MEM_SIZE < 0x1600000)
    {
        VENC_PRF("Allocated %p memeory buffer is small.\n", (void *)__G_ALI_MM_VENC_MEM_SIZE);
        venc_dev_p->_MM_VENC_SBM_BUFFER_SIZE_ = __G_ALI_MM_VENC_MEM_SIZE - venc_dev_p->_MM_VENC_PRIVATE_BUFFER_SIZE_;
    }
    venc_dev_p->_MM_VENC_SBM_BUFFER_ = __G_ALI_MM_VENC_MEM_START_ADDR + venc_dev_p->_MM_VENC_PRIVATE_BUFFER_SIZE_;
    /*
    VENC_PRF("Encoder%d open. \nPrivateStart 0x%p, size is 0x%p\nSBM start 0x%p, size is 0x%p\n",
                venc_dev_p->venc_number, venc_dev_p->_MM_VENC_PRIVATE_BUFFER_, venc_dev_p->_MM_VENC_PRIVATE_BUFFER_SIZE_,
                venc_dev_p->_MM_VENC_SBM_BUFFER_, venc_dev_p->_MM_VENC_SBM_BUFFER_SIZE_);
    */
    up(&m_venc_sem);

    return 0;
}


/*******************************************************************************
Name        : venc_release
Description : Be invoked to release the encoder device in the low level.
Parameters  :
Assumptions :
Limitations :
Returns     :
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Nov 12, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Nov 12, 2013                   Created               V0.1             Vic.Zhang
*******************************************************************************/
static int venc_release(struct inode *inode, struct file *file)
{
    struct venc_dev *venc_dev_p = file->private_data;

    down(&m_venc_sem);

    venc_dev_p->open_count--;

    if(venc_dev_p->open_count == 0)
    {
        if(venc_dev_p->status != 0)
        {
        	venc_dev_p->status = 0;
        }
    }
    else if(venc_dev_p->open_count < 0)
    {
	    VENC_PRF("Encoder%d open count fail %d.\n", venc_dev_p->venc_number, venc_dev_p->open_count);
    }

    //VENC_PRF("Encoder%d open count %d\n", venc_dev_p->venc_number, venc_dev_p->open_count);

    up(&m_venc_sem);

    return 0;
}


/*******************************************************************************
Name        : venc_mmap
Description : Be invoked to dump buffer in the low level.
Parameters  :
Assumptions :
Limitations :
Returns     :
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Nov 12, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Nov 12, 2013                   Created               V0.1             Vic.Zhang
*******************************************************************************/
static int venc_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned int size = vma->vm_end - vma->vm_start;
	unsigned int pos;

	if(size > BS_LIMIT_SIZE)
	{
		VENC_PRF("fail size %x mem_len %x\n", (int)size, (int)BS_LIMIT_SIZE);
		return -EINVAL;
	}

	pos = (unsigned long)(bs_phy_addr & 0x1FFFFFFF);
	vma->vm_flags |= VM_IO;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if ( remap_pfn_range(vma,vma->vm_start, (pos>>PAGE_SHIFT), vma->vm_end - vma->vm_start, vma->vm_page_prot) )
	{
		VENC_PRF("remap pfn fail\n");
		return -EINVAL;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
#else
	vma->vm_flags |= VM_RESERVED | VM_IO;
#endif
    return 0;
}

/*******************************************************************************
Name        : venc_read
Description : Reserved to read buffer in the low level for upper users.
Parameters  :
Assumptions :
Limitations :
Returns     :
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Nov 12, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Nov 12, 2013                   Created               V0.1             Vic.Zhang
*******************************************************************************/
static ssize_t venc_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{

#if 0
    struct venc_dev *venc_dev_p = file->private_data;
	int ret = 0, req_size = count, read_size = 0;


    if(down_interruptible(&m_venc_sem))
    {

        VENC_PRF("ali encoder down sem fail.\n");
        return -1;
    }

    if(venc_dev_p->open_count <= 0)
    {
        up(&m_venc_sem);

        VENC_PRF("Encoder don't be opened.\n");
        return -1;
    }

    up(&m_venc_sem);
    return read_size;
#endif

    return 0;
}

/*******************************************************************************
Name        : venc_write
Description : Reserved to write buffer in the low level for upper users.
Parameters  :
Assumptions :
Limitations :
Returns     :
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Nov 12, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Nov 12, 2013                   Created               V0.1             Vic.Zhang
*******************************************************************************/
static ssize_t venc_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
#if 0
    struct venc_dev *venc_dev_p = file->private_data;
	int ret = 0, req_size = count, write_count = 0;

    if(down_interruptible(&m_venc_sem))
    {
        VENC_PRF("ali encoder down sem fail.\n");
        return -1;
    }

    if((venc_dev_p->open_count <= 0) || (venc_dev_p->status == 0))
    {
        up(&m_venc_sem);

        VENC_PRF("Encoder don't be opened.\n");
        return -1;
    }

    up(&m_venc_sem);

    return write_count;
#endif

    return 0;
}


/*******************************************************************************
Name        : venc_ioctl
Description : Control encoder driver directly in the low level for user.
Parameters  :
Assumptions :
Limitations :
Returns     :
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Nov 12, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Nov 12, 2013                   Created               V0.1             Vic.Zhang
*******************************************************************************/
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long venc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int venc_ioctl(struct inode *node, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
    struct venc_dev *venc_dev_p = file->private_data;
    int ret = 0;
    unsigned long retNum = 0;

    down(&m_venc_sem);

    if(venc_dev_p->open_count <= 0)
    {
        up(&m_venc_sem);

        VENC_PRF("Encoder don't be opened.\n");
        return -1;
    }

    //VENC_PRF("IOCTL 0x%p. right is 0x%p.\n", cmd, VENC_IOWR_SBMMEM);
    switch(cmd)
    {
		case VENC_IOWR_CREATE:
		{
			struct videnc_see_config venc_info;

			if(1 == venc_dev_p->status)
            {
				VENC_PRF("Encoder%d has been created before.\n", venc_dev_p->venc_number);
				ret = -1;
				goto EXIT;
			}
            /* First fetch SBM idx. */
            retNum = copy_from_user((void *)&venc_info, (void *)arg, sizeof(venc_info));
            if (0 != retNum)
            {
                VENC_PRF("Fail to read videnc_see_config %d.\n", (int)retNum);
                ret = -1;
                goto EXIT;
            }
			venc_info.buffer_addr = venc_dev_p->_MM_VENC_PRIVATE_BUFFER_;
            venc_info.buffer_size = venc_dev_p->_MM_VENC_PRIVATE_BUFFER_SIZE_;
            venc_remote_init();
            venc_dev_p->status = 1;
			break;
		}

        case VENC_IOWR_START:
        {
            struct videnc_see_config venc_info;

            if(1 == venc_dev_p->status)
            {
                VENC_PRF("Encoder%d has been started before.\n", venc_dev_p->venc_number);
                ret = -1;
                goto EXIT;
            }
            /* First fetch SBM idx. */
            retNum = copy_from_user((void *)&venc_info, (void *)arg, sizeof(venc_info));
            if (0 != retNum)
            {
                VENC_PRF("Fail to read videnc_see_config %d.\n", (int)retNum);
                ret = -1;
                goto EXIT;
            }
            venc_info.buffer_addr = venc_dev_p->_MM_VENC_PRIVATE_BUFFER_;
            venc_info.buffer_size = venc_dev_p->_MM_VENC_PRIVATE_BUFFER_SIZE_;
            venc_remote_start(&venc_info);
            venc_dev_p->status = 1;
            break;
        }

        case VENC_IOWR_RELEASE:
        {
            if(venc_dev_p->status == 0)
            {
                VENC_PRF("Encoder%d has not been created.\n", (int)venc_dev_p->venc_number);
                ret = -1;
                goto EXIT;
            }
            venc_remote_release();
            venc_dev_p->status = 0;
            break;
        }

        case VENC_IOWR_STOP:
        {
            unsigned int valid_size = 0;
            if(venc_dev_p->status == 0)
            {
                VENC_PRF("Encoder%d has not been created.\n", (int)venc_dev_p->venc_number);
                ret = -1;
                goto EXIT;
            }

            retNum = copy_from_user((void *)&valid_size, (void *)arg, sizeof(int));
            venc_remote_stop();
            //venc_dev_p->status = 0;
            break;
        }

        case VENC_IOWR_ENCODE:
        {
            struct videnc_trigger_para venc_info;

            if(venc_dev_p->status == 0)
            {
                VENC_PRF("Encoder%d has not been created.\n", (int)venc_dev_p->venc_number);
                ret = -1;
                goto EXIT;
            }

            retNum = copy_from_user((void *)&venc_info, (void *)arg, sizeof(venc_info));
            if (0 != retNum)
            {
                VENC_PRF("Fail to read videnc_trigger_para %d.\n", (int)retNum);
                ret = -1;
                goto EXIT;
            }
            venc_info.encoder_ID = 0; /* Encoder x is selected to handle the job. */
            venc_remote_encode(&venc_info);

            break;
        }

        case VENC_IOWR_SBMMEM:
        {
            /* Local device allocate the static sbm memory. */
            struct videnc_config venc_info;

            venc_info.buffer_addr = venc_dev_p->_MM_VENC_SBM_BUFFER_;
            venc_info.buffer_size = venc_dev_p->_MM_VENC_SBM_BUFFER_SIZE_;

            VENC_PRF("In VENC_IOWR_SBMMEM ioctl:start 0x%p, size 0x%p.\n",
                            (void *)venc_info.buffer_addr, (void *)venc_info.buffer_size);

            retNum = copy_to_user((void *)arg, (void *)&venc_info, sizeof(venc_info));
            if (0 != retNum)
            {
                VENC_PRF("Fail to write into SBM para %d.\n", (int)retNum);
                ret = -1;
                goto EXIT;
            }

            break;
        }

        case VENC_IOWR_SELFMEM:
        {
            /* Local device allocate the static venc private memory. */
            struct videnc_config venc_info;

            venc_info.buffer_addr = venc_dev_p->_MM_VENC_PRIVATE_BUFFER_;
            venc_info.buffer_size = venc_dev_p->_MM_VENC_PRIVATE_BUFFER_SIZE_;

            VENC_PRF("In VENC_IOWR_SELFMEM ioctl:start 0x%p, size 0x%p.\n",
                                        (void *)venc_info.buffer_addr, (void *)venc_info.buffer_size);

            retNum = copy_to_user((void *)arg, (void *)&venc_info, sizeof(venc_info));
            if (0 != retNum)
            {
                VENC_PRF("Fail to write encoder slef mem in userspace %d.\n", (int)retNum);
                ret = -1;
                goto EXIT;
            }

            break;
        }
        case VENC_IOWR_BS_MAPADDR:
        {
            unsigned long addr_map;
            retNum = copy_from_user((void *)&addr_map, (void *)arg, sizeof(unsigned long));
            if (0 != retNum)
            {
                VENC_PRF("Fail to write encoder BS_MAPADDR in userspace %d.\n", (int)retNum);
                ret = -1;
                goto EXIT;
            }
            bs_phy_addr = addr_map;
            /*
            VENC_PRF("bs addr(0x%p) data: %p, %p, %p, %p, %p, %p, %p, %p, %p, %p\n", bs_phy_addr,
                    *((unsigned char*)bs_phy_addr), *((unsigned char*)(bs_phy_addr+1)), *((unsigned char*)(bs_phy_addr+2)),
                    *((unsigned char*)(bs_phy_addr+3)),
                    *((unsigned char*)(bs_phy_addr+4)), *((unsigned char*)(bs_phy_addr+5)), *((unsigned char*)(bs_phy_addr+6)),
                    *((unsigned char*)(bs_phy_addr+7)),
                    *((unsigned char*)(bs_phy_addr+10238)), *((unsigned char*)(bs_phy_addr+10239)));
            //*/
            //KernelWriteFile(bs_phy_addr, 74184);

            break;
        }

        default:
            break;
    }

EXIT:
    up(&m_venc_sem);
    return ret;
}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations venc_fops = {
    .owner    =   THIS_MODULE,     /* Owner */
    .open     =   venc_open,        /* Open method */
    .release  =   venc_release,     /* Release method */
    .read     =   venc_read,        /* Read method */
    .write    =   venc_write,       /* Write method */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
    .unlocked_ioctl = venc_ioctl,
#else
    .ioctl    =   venc_ioctl,       /* Ioctl method */
#endif
    .mmap     =   venc_mmap,

};

static int __init venc_init(void)
{
    int i, ret;

    /* Request dynamic allocation of a device major number */
    if (alloc_chrdev_region(&venc_dev_number, 0,
                            VENC_NUM, DEVICE_NAME) < 0)
    {
        VENC_PRF(KERN_DEBUG "Can't register device\n");
        return -1;
    }

    /* Populate sysfs entries */
    venc_class = class_create(THIS_MODULE, DEVICE_NAME);

    for (i=0; i<VENC_NUM; i++)
    {
        /* Allocate memory for the per-device structure */
        venc_priv[i] = kmalloc(sizeof(struct venc_dev), GFP_KERNEL);
        if (!venc_priv[i])
        {
            VENC_PRF("Bad Kmalloc\n");
            return -ENOMEM;
        }
        memset(venc_priv[i], 0, sizeof(struct venc_dev));

        sprintf(venc_priv[i]->name, "ali_venc%d", i);

        /* Arrange the Encoder number to correlate this device
           with the corresponding video encoder */
        venc_priv[i]->venc_number = i;
	    venc_priv[i]->status = 0;

        /* Connect the file operations with the cdev */
        cdev_init(&venc_priv[i]->cdev, &venc_fops);
        venc_priv[i]->cdev.owner = THIS_MODULE;

        /* Connect the major/minor number to the cdev */
        ret = cdev_add(&venc_priv[i]->cdev, (venc_dev_number + i), 1);
        if (ret)
        {
            VENC_PRF("Bad cdev.\n");
            return ret;
        }

    	venc_device = device_create(venc_class, NULL, MKDEV(MAJOR(venc_dev_number), i),
                                   NULL, "ali_venc%d", i);
    	if(venc_device == NULL)
        {
    		VENC_PRF("video encoder create device fail.\n");
    		return 1;
    	}
    }

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
    sema_init(&m_venc_sem, 1);
#else
    init_MUTEX(&m_venc_sem);
#endif

    /* RPC create the SEE Encoder objects.*/
    venc_remote_init();

    return 0;

}

/* Driver Exit */
void __exit venc_exit(void)
{
    int i;

    /* Release the major number */
    unregister_chrdev_region(venc_dev_number, VENC_NUM);

    for (i=0; i<VENC_NUM; i++)
    {
        device_destroy(venc_class, MKDEV(MAJOR(venc_dev_number), i));
        /* Remove the cdev */
        cdev_del(&venc_priv[i]->cdev);
        kfree(venc_priv[i]);
    }

    /* Destroy cmos_class */
    class_destroy(venc_class);

    return;
}

module_init(venc_init);
module_exit(venc_exit);

MODULE_AUTHOR("ALi (Zhuhai) Corporation");
MODULE_DESCRIPTION("ali video encoder driver");
MODULE_LICENSE("GPL");

