#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/dvb/frontend.h>
#include <linux/version.h>
#include <asm/irq.h>

#include <linux/i2c.h>
#include <asm/mach-ali/typedef.h>
#include "ali_scart_mx9671.h"

int scart_major =   0;
int scart_minor =   0;

//static UINT16 g_mx_volume = 0;
static struct scart_mx9671_private *ali_scart_private;

int ali_i2c_scb_write(u32 id, u8 slv_addr, u8 *data, int len);
int ali_i2c_scb_read(u32 id, u8 slv_addr, u8 *data, int len);
int ali_i2c_scb_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int len);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long scart_mx9671_ioctl (struct file *filp, unsigned int cmd, unsigned long param)
#else
static int scart_mx9671_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long param)
#endif
{
	struct scart_mx9671_private *tp = filp->private_data;
	
	int result = 0 ;
	unsigned char reg_dirty_list[MX_MAX_REG_LEN];
	unsigned char reg_offset, reg_tgt_val;
	unsigned char tmp_buf[4];
	unsigned char write = 0;
				
	memset(reg_dirty_list, 0, MX_MAX_REG_LEN);
	switch(cmd)
	{	
	case SCART_TV_MODE:
		reg_offset = 7;
		if(TV_MODE_RGB==param)
			reg_tgt_val = 3<<3;
		else if(TV_MODE_CVBS==param)
			reg_tgt_val = 0<<3;
		else
			reg_tgt_val = 2<<3;
		tp->reg_val[reg_offset] &= (~(0x03<<3));
		tp->reg_val[reg_offset] |= reg_tgt_val;
		reg_dirty_list[reg_offset] = 1;
		write = 1;	
		break;
		
	case SCART_TV_ASPECT:	
		reg_offset = 7;
		if(ASPECT_4_3==param)
			reg_tgt_val = 3;
		else if(ASPECT_16_9==param)
			reg_tgt_val = 1;
		else
			reg_tgt_val = 0;
		tp->reg_val[reg_offset] &= (~0x03);
		tp->reg_val[reg_offset] |= reg_tgt_val;
		reg_dirty_list[reg_offset] = 1;
		write = 1;
		break;
		
	case SCART_VCR_ASPECT:		
		reg_offset = 9;
		if(ASPECT_4_3==param)
			reg_tgt_val = 3;
		else if(ASPECT_16_9==param)
			reg_tgt_val = 1;
		else
			reg_tgt_val = 0;
		tp->reg_val[reg_offset] &= (~0x03);
		tp->reg_val[reg_offset] |= reg_tgt_val;
		reg_dirty_list[reg_offset] = 1;
		write = 1;
		break;
		
	case SCART_TV_SOURCE:
		if(SOURCE_STB_IN==param)
		{
			reg_offset = 1;//Audio
			reg_tgt_val = 0;
			tp->reg_val[reg_offset] &= (~0x3);
			tp->reg_val[reg_offset] |= reg_tgt_val;
			reg_dirty_list[reg_offset] = 1;

			reg_offset = 6;//video 
			reg_tgt_val = 0;	
      tp->reg_val[reg_offset] &= (~0x1f);
			tp->reg_val[reg_offset] |= reg_tgt_val;
			reg_dirty_list[reg_offset] = 1;            
		}
		else if(SOURCE_VCR_IN==param)
		{
			reg_offset = 1;//audio
			reg_tgt_val = 0x01;
			tp->reg_val[reg_offset] &= (~0x03);
			tp->reg_val[reg_offset] |= reg_tgt_val;
			reg_dirty_list[reg_offset] = 1;

			reg_offset = 6;//video
			reg_tgt_val = 0x02|0x08;
			tp->reg_val[reg_offset] &= (~0x1f);
			tp->reg_val[reg_offset] |= reg_tgt_val;
			reg_dirty_list[reg_offset] = 1;

            tmp_buf[0] = 0x0e;
            ali_i2c_scb_write_read(tp->i2c_type_id, tp->base_addr, tmp_buf, 1, 1);

            tmp_buf[0] = (tmp_buf[0] & 0x0C) >> 2;

            reg_offset = 7;//Set TV FS
            //reg_tgt_val = 0x12;/////| tmp_buf[0];
            reg_tgt_val = 0x10 | tmp_buf[0];
            tp->reg_val[reg_offset] &= ~0x1B;////(~(0x3<<3));
            tp->reg_val[reg_offset] |= reg_tgt_val;
            reg_dirty_list[reg_offset] = 1;
            
		}
		else
		{
      reg_offset = 1;//Audio
			reg_tgt_val = 2;
			tp->reg_val[reg_offset] &= (~0x3);
			tp->reg_val[reg_offset] |= reg_tgt_val;
			reg_dirty_list[reg_offset] = 1;

			reg_offset = 6;//video 
			reg_tgt_val = 0x03|0x10;	
      tp->reg_val[reg_offset] &= (~0x1f);
			tp->reg_val[reg_offset] |= reg_tgt_val;
			reg_dirty_list[reg_offset] = 1;
		}
		write = 1;
		break;
		
	case SCART_VCR_SOURCE:
		if(SOURCE_STB_IN==param)
		{
			reg_offset = 1;//Audio
			reg_tgt_val = 0<<2;
			tp->reg_val[reg_offset] &= (~(0x3<<2));
			tp->reg_val[reg_offset] |= reg_tgt_val;
			reg_dirty_list[reg_offset] = 1;

			reg_offset = 8;//vcr video only cvbs.RGB only use input function 
			reg_tgt_val = 0;
			tp->reg_val[reg_offset] &= (~0x7);
			tp->reg_val[reg_offset] |= reg_tgt_val;
			reg_dirty_list[reg_offset] = 1;

		}
		else if(SOURCE_TV_IN==param)
		{
			reg_offset = 1;//Audio
			reg_tgt_val = 2<<2;
			tp->reg_val[reg_offset] &= (~(0x3<<2));
			tp->reg_val[reg_offset] |= reg_tgt_val;
			reg_dirty_list[reg_offset] = 1;

			reg_offset = 8;//video
			reg_tgt_val = 3;
			tp->reg_val[reg_offset] &= (~(0x7));
			tp->reg_val[reg_offset] |= reg_tgt_val;
			reg_dirty_list[reg_offset] = 1;
		
		}
		else
		{
      reg_offset = 1;//Audio
			reg_tgt_val = 1<<2;
			tp->reg_val[reg_offset] &= (~(0x3<<2));
			tp->reg_val[reg_offset] |= reg_tgt_val;
			reg_dirty_list[reg_offset] = 1;

			reg_offset = 8;//video
			reg_tgt_val = 2;
			tp->reg_val[reg_offset] &= (~(0x7));
			tp->reg_val[reg_offset] |= reg_tgt_val;
			reg_dirty_list[reg_offset] = 1;
		}
		write = 1;
		break;	
			
	case SCART_CHK_STATE:
        write = 0;
		if(param)
		{
			*((unsigned int *)param) = 0;
			tmp_buf[0] = 0x0e;
			ali_i2c_scb_write_read(tp->i2c_type_id, tp->base_addr, tmp_buf, 1, 1);

            tmp_buf[1] = 0x0f;
            ali_i2c_scb_write_read(tp->i2c_type_id, tp->base_addr, tmp_buf+1, 1, 1);
            
            tmp_buf[2] = 0x07;
            ali_i2c_scb_write_read(tp->i2c_type_id, tp->base_addr, tmp_buf+2, 1, 1);

			if(tmp_buf[0]&0x04)
			{
				*((unsigned int *)param) |= SCART_STATE_VCR_IN;
                if((tmp_buf[2] & 0x03) != ((tmp_buf[0] & 0x0C)>>2))
                {
                   reg_offset = 7;
                   reg_tgt_val = ((tmp_buf[0] & 0x0C)>>2);
                   tp->reg_val[reg_offset] &= ~0x3;
                   tp->reg_val[reg_offset] |= reg_tgt_val;
                   reg_dirty_list[reg_offset] = 1; 
                   write = 1;                    
                }
			}
			if(tmp_buf[0]&0x01)
				*((unsigned int *)param) |= SCART_STATE_TV_IN;
		}
		
		break;
	case SCART_AUDIO_MUTE:
		{
			unsigned char orig_vol;
			reg_offset = 2;
			reg_dirty_list[reg_offset] = 1;
      tp->reg_val[reg_offset] &= ~0x01;
      if(param)
			{
         reg_tgt_val = 0;    
       }
			else
			{
         reg_tgt_val = 1;
			}
      tp->reg_val[reg_offset] |= reg_tgt_val;
      write = 1;
		}
		break;

  case SCART_ENTRY_STADNBY:
     reg_offset = 16;
     if(param == 0)
         reg_tgt_val = 1<<6;
     else
         reg_tgt_val = 3<<6;
        
    tp->reg_val[reg_offset] &= (~(0x3<<6));
		tp->reg_val[reg_offset] |= reg_tgt_val;
		reg_dirty_list[reg_offset] = 1;
		write = 1;        
    break;
    
	default: 
		result = -EFAULT;
		break;
	}
	if(result == 0)
	{
		if(write)
		{
			for(reg_offset = 0; reg_offset<MX_MAX_REG_LEN; reg_offset++)
			{
				if(reg_dirty_list[reg_offset])
				{
					tmp_buf[0] = reg_offset;
					tmp_buf[1] = tp->reg_val[reg_offset];
					ali_i2c_scb_write(tp->i2c_type_id, tp->base_addr, tmp_buf, 2);
				}
			}
		}
	}
	return result;
}

static int scart_mx9671_release(struct inode *inode, struct file *filp)
{
	struct scart_mx9671_private *dev = filp->private_data;
	
	unsigned char tmp_buf[MX_MAX_REG_LEN];
	
	tmp_buf[0] = 16;
	tmp_buf[1] = 0x40;
	
	ali_i2c_scb_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);

	return 0;
}

static int scart_mx9671_open(struct inode *inode, struct file *filp)
{
	struct scart_mx9671_private *dev; /* device information */

	dev = container_of(inode->i_cdev, struct scart_mx9671_private, cdev);
	filp->private_data = dev; /* for other methods */
	
	unsigned char tmp_buf[16];
	unsigned short lsr_id;
	unsigned int i;
	unsigned int scart_state = 0;

  tmp_buf[0] = 0x10;
	tmp_buf[1] = dev->reg_val[tmp_buf[0]];
	ali_i2c_scb_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 16

	tmp_buf[0] = 0x00;
	tmp_buf[1] = dev->reg_val[tmp_buf[0]];
	ali_i2c_scb_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 0

  tmp_buf[0] = 0x01;
	tmp_buf[1] = dev->reg_val[tmp_buf[0]];
	ali_i2c_scb_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 1

  tmp_buf[0] = 0x06;
	tmp_buf[1] = dev->reg_val[tmp_buf[0]];
	ali_i2c_scb_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 6

  tmp_buf[0] = 0x07;
	tmp_buf[1] = dev->reg_val[tmp_buf[0]];
	ali_i2c_scb_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 7

  tmp_buf[0] = 0x08;
	tmp_buf[1] = dev->reg_val[tmp_buf[0]];
	ali_i2c_scb_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 8

  tmp_buf[0] = 0x09;
	tmp_buf[1] = dev->reg_val[tmp_buf[0]];
	ali_i2c_scb_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 9

  tmp_buf[0] = 0x0D;
	tmp_buf[1] = dev->reg_val[tmp_buf[0]];
	ali_i2c_scb_write(dev->i2c_type_id, dev->base_addr, tmp_buf, 2);//write reg 13
	
	return 0;
}

/*
 * The file operations for the keypad device
 */
struct file_operations scart_dev_fops = {
	.owner =	THIS_MODULE,
		
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	.unlocked_ioctl = scart_mx9671_ioctl,
#else
	.ioctl =	scart_mx9671_ioctl,
#endif	

	.open =		scart_mx9671_open,
	.release =	scart_mx9671_release,
};

/*
 * Set up the char_dev structure for this device.
 */
struct class *ali_scart_class;
struct device *ali_scart_dev_node;

int __init scart_init(void)
{
	int result;
	dev_t devno = 0;

/*
 * Get a range of minor numbers to work with, asking for a dynamic
 * major unless directed otherwise at load time.
 */
	if (scart_major) {
		devno = MKDEV(scart_major, scart_minor);
		result = register_chrdev_region(devno, 1, ALI_SCART_DEVICE_NAME);
	} else {
		result = alloc_chrdev_region(&devno, scart_minor, 1,
				ALI_SCART_DEVICE_NAME);
		scart_major = MAJOR(devno);
	}
	if (result < 0) {
		printk(KERN_WARNING "scart: can't get major %d\n", scart_major);
		return result;
	}

   /* 
	 * allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	ali_scart_private = kmalloc(sizeof(struct scart_mx9671_private), GFP_KERNEL);
	if (!ali_scart_private) {
		return -ENOMEM;
	}
	memset(ali_scart_private, 0, sizeof(struct scart_mx9671_private));

	ali_scart_private->reg_val[0] = 0x40;
	ali_scart_private->reg_val[1] = 0x10;
	ali_scart_private->reg_val[6] = 0x00;
	ali_scart_private->reg_val[7] = 0x01;
	ali_scart_private->reg_val[8] = 0x98;
	ali_scart_private->reg_val[9] = 0x06;  
  ali_scart_private->reg_val[13] = 0xFE;
  ali_scart_private->reg_val[16] = 0xC0;
  
  ali_scart_private->i2c_type_id = SCART_I2C_ID;
  ali_scart_private->base_addr = ALI_SCART_ADDR;

	
	cdev_init(&ali_scart_private->cdev, &scart_dev_fops);
	ali_scart_private->cdev.owner = THIS_MODULE;
	result = cdev_add (&ali_scart_private->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (result) {
		printk(KERN_NOTICE "Error %d adding scart", result);
		goto err1;
	}
		
	/* creating your own class */
  ali_scart_class =class_create(THIS_MODULE, "ali_scart_class");
  if(IS_ERR(ali_scart_class)) {
  	printk("Err: failed in creating class.\n");	
    result = PTR_ERR(ali_scart_class);
		goto err2;
  }
  
 /* register your own device in sysfs, and this will cause udevd to create corresponding device node */
  ali_scart_dev_node = device_create(ali_scart_class, NULL, devno, NULL, ALI_SCART_DEVICE_NAME);
  if (IS_ERR(ali_scart_dev_node)) {
		printk(KERN_ERR "device_create() failed!\n");	
		result = PTR_ERR(ali_scart_dev_node);
		goto err3;
	}

	return 0;
	
err3:
	class_destroy(ali_scart_class);
err2:
	cdev_del(&ali_scart_private->cdev);
err1:
	kfree(ali_scart_private);
	return result;
}

void __exit scart_cleanup(void)
{
	dev_t devno = MKDEV(scart_major, scart_minor);

	/* cleanup_module is never called if registering failed */	
	if(ali_scart_dev_node != NULL)
		device_del(ali_scart_dev_node);

	if(ali_scart_class != NULL)
		class_destroy(ali_scart_class);
		
	cdev_del(&ali_scart_private->cdev);
	kfree(ali_scart_private);
	unregister_chrdev_region(devno, 1);
}

module_init(scart_init);
module_exit(scart_cleanup);

MODULE_AUTHOR("ALi Corp ZhuHai Driver Team, Mark Lee");
MODULE_DESCRIPTION("SCART Driver for ALI M3603 Demo Board");
MODULE_LICENSE("GPL");
