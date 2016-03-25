/*
 *      Alitech GPIO PWM Driver
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <ali_gpio.h>
#include <linux/mutex.h>
#include <asm/mach-ali/m36_gpio.h>
#include <ali_reg.h>
#include <asm/uaccess.h>

#include <asm/mach/time.h>
#include <asm/sched_clock.h>
#include <ali_interrupt.h>

#include "ali_pwm.h"

#define DRV_VERSION     "1.0.0"
#define ALI_PWM_DEV_NAME	"ali_pwm"

#define PWM_DEBUG	(0)

#define PWM_PRINT(fmt,args...)	do { if(PWM_DEBUG) printk("%s,%d,"fmt, __FUNCTION__,__LINE__,##args);} while(0)

#define ALI_SBTIMER_BASE VIRT_SOUTHBRIDGE+0xA00
#define REG_SB_TM0_AIM_SECOND  ALI_SBTIMER_BASE+0x00 //32bit
#define REG_SB_TM0_AIM_MS      ALI_SBTIMER_BASE+0x04 //16bit
#define REG_SB_TM0_TICK_VALUE  ALI_SBTIMER_BASE+0x06 //16bit
#define REG_SB_TM0CTRL ALI_SBTIMER_BASE+0x08   //8bit
#define REG_SB_TM0_CUR_MS      ALI_SBTIMER_BASE+0x0A //16bit
#define REG_SB_TM0_CUR_SECOND  ALI_SBTIMER_BASE+0x0C //32bit

#define REG_SB_TM2CNT	       ALI_SBTIMER_BASE+0x20 //32bit
#define REG_SB_TM2CTRL 		ALI_SBTIMER_BASE+0x28   //8bit
#define SB_TM2_TICK	(32000/27)//(1185)	//ns,27MHz/32

#define REG_SB_TM3CNT	       ALI_SBTIMER_BASE+0x30 //32bit
#define REG_SB_TM3CTRL 		ALI_SBTIMER_BASE+0x38   //8bit
#define SB_TM3_TICK	(32000/27)	//ns,27MHz/32

#define REG_SB_TM4CNT		ALI_SBTIMER_BASE+0x40  //32bit
#define REG_SB_TM4CMP		ALI_SBTIMER_BASE+0x44  //32bit
#define REG_SB_TM4CTRL		ALI_SBTIMER_BASE+0x48  //8bit

#define REG_SB_TM5CNT	       ALI_SBTIMER_BASE+0x50 //32bit
#define REG_SB_TM5CMP		ALI_SBTIMER_BASE+0x54  //32bit
#define REG_SB_TM5CTRL 		ALI_SBTIMER_BASE+0x58   //8bit
#define SB_TM5_TICK	(16000/27)	//ns,27MHz/16

#define REG_SB_TM6CNT	       ALI_SBTIMER_BASE+0x60 //32bit
#define REG_SB_TM6CTRL 		ALI_SBTIMER_BASE+0x68   //8bit
#define SB_TM6_TICK	(1000/27)	//ns,27MHz

#define REG_SB_TM7CNT	       ALI_SBTIMER_BASE+0x70 //32bit
#define REG_SB_TM7CTRL 		ALI_SBTIMER_BASE+0x78   //8bit
#define SB_TM7_TICK	(1000/27)	//ns,27MHz


static pwm_data data[5];

#define PWM_CNT (sizeof(data)/sizeof(data[0]))

irqreturn_t pwm_isr(int irq, void *dev_id) {

	int handle;

	//PWM_PRINT("irq[%d]\n", irq);
	
	switch(irq) {
		case INT_ALI_RTC_TM2://SB_TM2
			handle = 0;
			break;
		case INT_ALI_RTC_TM3://SB_TM3
			handle = 1;
			break;
		case INT_ALI_RTC_TM5://SB_TM5
			handle = 2;
			break;
		case INT_ALI_RTC_TM6://SB_TM6
			handle = 3;
			break;
		case INT_ALI_RTC_TM7://SB_TM7
			handle = 4;
			break;
		default:
			handle = -1;
			break;
	}

	if(-1 == handle) 
		return IRQ_HANDLED;

	switch(irq) {
		case INT_ALI_RTC_TM2://SB_TM2
			{
				//clear SB_TM2_CTRL
				writeb(0, REG_SB_TM2CTRL);				

				if(data[handle].status & 0x1) {
					//op gpio
					ali_gpio_direction_output(data[handle].gpio, (data[handle].status & (1<<2)) ? 0 : 1);
					
					long tick = (data[handle].status & (1<<2)) ? data[handle].req_off : data[handle].req_on;					
					
					int tick_cnt = tick * 1000L / SB_TM2_TICK;
					if(!tick_cnt)
						tick_cnt = 1;
					unsigned long cnt = 0xFFFFFFFF - tick_cnt;					
					writel(cnt, REG_SB_TM2CNT);

					unsigned char ctl = readb(REG_SB_TM2CTRL);
					
					//counting frequency 27MHz/32				
					ctl &= ~(1<<0 | 1<<1);

					//enable SB_TM2_Interrupt
					ctl |= (1<<4);
					
					//enable SB_TM2
					ctl |= (1<<2);

					writeb(ctl, REG_SB_TM2CTRL);

					if(data[handle].status & (1<<2))
						data[handle].status &= ~(1<<2);
					else
						data[handle].status |= (1<<2);
				}
				else {
					ali_gpio_direction_output(data[handle].gpio, 0);
				}

			}
			break;
		case INT_ALI_RTC_TM3://SB_TM3
			{
				//clear SB_TM3_CTRL
				writeb(0, REG_SB_TM3CTRL);

				if(data[handle].status & 0x1) {
					//op gpio
					ali_gpio_direction_output(data[handle].gpio, (data[handle].status & (1<<2)) ? 0 : 1);
					
					long tick = (data[handle].status & (1<<2)) ? data[handle].req_off : data[handle].req_on;					
					int tick_cnt = tick * 1000L / SB_TM3_TICK;
					unsigned long cnt = 0xFFFFFFFF - tick_cnt;
					if(!tick_cnt)
						tick_cnt = 1;
					writel(cnt, REG_SB_TM3CNT);

					unsigned char ctl = readb(REG_SB_TM3CTRL);
					
					//counting frequency 27MHz/32				
					ctl &= ~(1<<0 | 1<<1);

					//enable SB_TM3_Interrupt
					ctl |= (1<<4);
					
					//enable SB_TM3
					ctl |= (1<<2);

					writeb(ctl, REG_SB_TM3CTRL);

					if(data[handle].status & (1<<2))
						data[handle].status &= ~(1<<2);
					else
						data[handle].status |= (1<<2);
				}
				else {
					ali_gpio_direction_output(data[handle].gpio, 0);
				}

			}
			break;
		case INT_ALI_RTC_TM5://SB_TM5
			{
				//clear SB_TM5_CTRL
				writeb(0, REG_SB_TM5CTRL);

				if(data[handle].status & 0x1) {
					//op gpio
					ali_gpio_direction_output(data[handle].gpio, (data[handle].status & (1<<2)) ? 0 : 1);
					
					long tick = (data[handle].status & (1<<2)) ? data[handle].req_off : data[handle].req_on;					
					int tick_cnt = tick * 1000L / SB_TM5_TICK;
					
					writel(0, REG_SB_TM5CNT);
					writel(tick_cnt, REG_SB_TM5CMP);

					unsigned char ctl = readb(REG_SB_TM5CTRL);

					//counting frequency 27MHz/16
					ctl &= ~(1<<0 | 1<<1);
				
					//enable SB_TM5_Interrupt
					ctl |= (1<<4);
					
					//enable SB_TM3
					ctl |= (1<<2);

					writeb(ctl, REG_SB_TM5CTRL);

					if(data[handle].status & (1<<2))
						data[handle].status &= ~(1<<2);
					else
						data[handle].status |= (1<<2);
				}
				else {
					ali_gpio_direction_output(data[handle].gpio, 0);
				}

			}
			break;
		case INT_ALI_RTC_TM6://SB_TM6
			{
				//clear SB_TM6_CTRL
				writeb(0, REG_SB_TM6CTRL);
				
				if(data[handle].status & 0x1) {
					//op gpio
					ali_gpio_direction_output(data[handle].gpio, (data[handle].status & (1<<2)) ? 0 : 1);
					
					long tick = (data[handle].status & (1<<2)) ? data[handle].req_off : data[handle].req_on;					
					int tick_cnt = tick * 1000L / SB_TM6_TICK;
					if(!tick_cnt)
						tick_cnt = 1;
					unsigned long cnt = 0xFFFFFFFF - tick_cnt;					
					writel(cnt, REG_SB_TM6CNT);

					unsigned char ctl = readb(REG_SB_TM6CTRL);					

					//enable SB_TM6_Interrupt
					ctl |= (1<<4);
					
					//enable SB_TM6
					ctl |= (1<<2);

					writeb(ctl, REG_SB_TM6CTRL);

					if(data[handle].status & (1<<2))
						data[handle].status &= ~(1<<2);
					else
						data[handle].status |= (1<<2);
				}
				else {
					ali_gpio_direction_output(data[handle].gpio, 0);
				}

			}
			break;
		case INT_ALI_RTC_TM7://SB_TM7
			{
				//clear SB_TM7_CTRL
				writeb(0, REG_SB_TM7CTRL);
				
				if(data[handle].status & 0x1) {
					//op gpio
					ali_gpio_direction_output(data[handle].gpio, (data[handle].status & (1<<2)) ? 0 : 1);
					
					long tick = (data[handle].status & (1<<2)) ? data[handle].req_off : data[handle].req_on;					
					int tick_cnt = tick * 1000L / SB_TM7_TICK;
					if(!tick_cnt)
						tick_cnt = 1;
					unsigned long cnt = 0xFFFFFFFF - tick_cnt;					
					writel(cnt, REG_SB_TM7CNT);

					unsigned char ctl = readb(REG_SB_TM7CTRL);					

					//enable SB_TM7_Interrupt
					ctl |= (1<<4);
					
					//enable SB_TM7
					ctl |= (1<<2);

					writeb(ctl, REG_SB_TM7CTRL);

					if(data[handle].status & (1<<2))
						data[handle].status &= ~(1<<2);
					else
						data[handle].status |= (1<<2);
				}
				else {
					ali_gpio_direction_output(data[handle].gpio, 0);
				}

			}
			break;
		default:
			break;
	}

	return IRQ_HANDLED;
}

static void sb_tmx_start(int index, long tick) {
	int ret;
	//currently,we use SB_TM2,SB_TM3,SB_TM5,SB_TM6,SB_TM7,SB_TM4 is used for kernel timer
	switch(index) {
		case 0://SB_TM2
			{				
				writeb(0, REG_SB_TM2CTRL);
				/*ret = request_irq(INT_ALI_RTC_TM2, (irq_handler_t)pwm_isr, 0, ALI_PWM_TM2_NAME, NULL); 
				if(ret != 0) {
					PWM_PRINT("request_irq INT_ALI_RTC_TM2 fail,ret[%d]",ret);
					return;
				}*/

				//set SB_TM2_CNT
				int tick_cnt = data[index].req_on * 1000L / SB_TM2_TICK;
				unsigned long cnt = 0xFFFFFFFF - tick_cnt;
				if(!tick_cnt)
					tick_cnt = 1;				
				writel(cnt, REG_SB_TM2CNT);

				unsigned char ctl = readb(REG_SB_TM2CTRL);
				
				//counting frequency 27MHz/32				
				ctl &= ~(1<<0 | 1<<1);

				//enable SB_TM2_Interrupt
				ctl |= (1<<4);
				
				//enable SB_TM2
				ctl |= (1<<2);

				//op gpio
				ali_gpio_direction_output(data[index].gpio, 1);

				writeb(ctl, REG_SB_TM2CTRL);

			}
			break;
		case 1://SB_TM3
			{				
				writeb(0, REG_SB_TM3CTRL);
				
				/*ret = request_irq(INT_ALI_RTC_TM3, (irq_handler_t)pwm_isr, 0, ALI_PWM_TM3_NAME, NULL);
				if(ret != 0) {
					PWM_PRINT("request_irq INT_ALI_RTC_TM3 fail,ret[%d]\n",ret);
					return;
				}*/

				//set SB_TM3_CNT
				int tick_cnt = data[index].req_on * 1000L / SB_TM3_TICK;
				if(!tick_cnt)
					tick_cnt = 1;
				unsigned long cnt = 0xFFFFFFFF - tick_cnt;
				writel(cnt, REG_SB_TM3CNT);

				unsigned char ctl = readb(REG_SB_TM3CTRL);
				
				//counting frequency 27MHz/32				
				ctl &= ~(1<<0 | 1<<1);

				//enable SB_TM3_Interrupt
				ctl |= (1<<4);
				
				//enable SB_TM3
				ctl |= (1<<2);

				//op gpio
				ali_gpio_direction_output(data[index].gpio, 1);

				writeb(ctl, REG_SB_TM3CTRL);

			}
			break;
		case 2://SB_TM5
			{				
				writeb(0, REG_SB_TM5CTRL);

				//set SB_TM5_CNT
				int tick_cnt = data[index].req_on * 1000L / SB_TM5_TICK;
				
				writel(0, REG_SB_TM5CNT);
				writel(tick_cnt, REG_SB_TM5CMP);

				unsigned char ctl = readb(REG_SB_TM5CTRL);
				
				//counting frequency 27MHz/32				
				ctl &= ~(1<<0 | 1<<1);

				//enable SB_TM5_Interrupt
				ctl |= (1<<4);
				
				//enable SB_TM5
				ctl |= (1<<2);

				//op gpio
				ali_gpio_direction_output(data[index].gpio, 1);

				writeb(ctl, REG_SB_TM5CTRL);

			}
			break;
		case 3://SB_TM6
			{				
				writeb(0, REG_SB_TM6CTRL);

				//set SB_TM6_CNT
				int tick_cnt = data[index].req_on * 1000L / SB_TM6_TICK;
				if(!tick_cnt)
					tick_cnt = 1;
				unsigned long cnt = 0xFFFFFFFF - tick_cnt;
				writel(cnt, REG_SB_TM6CNT);

				unsigned char ctl = readb(REG_SB_TM6CTRL);
				
				//enable SB_TM6_Interrupt
				ctl |= (1<<4);
				
				//enable SB_TM6
				ctl |= (1<<2);

				//op gpio
				ali_gpio_direction_output(data[index].gpio, 1);

				writeb(ctl, REG_SB_TM6CTRL);
			}
			break;
		case 4://SB_TM7
			{				
				writeb(0, REG_SB_TM7CTRL);

				//set SB_TM7_CNT
				int tick_cnt = data[index].req_on * 1000L / SB_TM7_TICK;
				if(!tick_cnt)
					tick_cnt = 1;
				unsigned long cnt = 0xFFFFFFFF - tick_cnt;
				writel(cnt, REG_SB_TM7CNT);

				unsigned char ctl = readb(REG_SB_TM7CTRL);
				
				//enable SB_TM7_Interrupt
				ctl |= (1<<4);
				
				//enable SB_TM7
				ctl |= (1<<2);

				//op gpio
				ali_gpio_direction_output(data[index].gpio, 1);
				PWM_PRINT("cnt[0x%x],ctl[0x%x]\n",cnt,ctl);
				writeb(ctl, REG_SB_TM7CTRL);
			}
			break;
		default:
			break;
	}	
	
	return;
}

static void sb_tmx_stop(int index) {
	int ret;

	PWM_PRINT("stop sb tm[%d]\n",index);
	
	//currently,we use SB_TM2,SB_TM3,SB_TM5,SB_TM6,SB_TM7,SB_TM4 is used for kernel timer
	switch(index) {
		case 0://SB_TM2
			{
				//clear SB_TM2_CTRL
				if(data[index].status & 0x1) {
					writeb(0, REG_SB_TM2CTRL);				
					//free_irq(INT_ALI_RTC_TM2, NULL);
				}
			}
			break;
		case 1://SB_TM3
			if(data[index].status & 0x1) {
				writeb(0, REG_SB_TM3CTRL);
			}
			break;
		case 2://SB_TM5
			if(data[index].status & 0x1) {
				writeb(0, REG_SB_TM5CTRL);
			}
			break;
		case 3://SB_TM6
			if(data[index].status & 0x1) {
				writeb(0, REG_SB_TM6CTRL);
			}
			break;
		case 4://SB_TM7
			if(data[index].status & 0x1) {
				writeb(0, REG_SB_TM7CTRL);
			}
			break;
		default:
			break;
	}	
	
	return;
}

static int ali_pwm_open(struct inode *inode, struct file *file)
{
	PWM_PRINT("Enter\n");
	
	return 0;
}

static int ali_pwm_get(struct file *file)
{
	int ret = -1;
	int i;
	
	for(i=0; i<PWM_CNT; i++) 
	{
		if(1 == i || 3 == i || 4 == i) {
			//SB_TM3 is used by ali_pmu for rtc time
			//SB_TM6/SB_TM7 can't work
			continue;
		}
		if(!(data[i].status & (1<<3))) {
			ret = i;
			data[i].status |= (1<<3);
			data[i].file = (void*)file;
			break;
		}
	}

	PWM_PRINT("ret[%d]\n", ret);
	
	return ret;
}

static void ali_pwm_start(int handle)
{
	int ret = -1;
	int i;

	PWM_PRINT("handle[%d]\n", handle);
	
	if(handle >= PWM_CNT) {
		PWM_PRINT("invalid handle[%d]\n", handle);
		return;
	}

	//set start flag
	data[handle].status |= 0x1;

	//set gpio on/off flag
	data[handle].status |= (1<<2);

	//in case use flag is released by other fd
	data[handle].status |= (1<<3);

	sb_tmx_start(handle, data[handle].req_on);
	
	return;
}

static void ali_pwm_set_param(pwm_param *p)
{		
	PWM_PRINT("Enter,handle[%d],gpio[%d],dutyCycle[%d],req_on[%ld],req_off[%ld]\n",p->handle,p->gpio,p->dutyCycle,p->req_on,p->req_off);

	if(p->handle >= PWM_CNT) {
		PWM_PRINT("invalid handle[%d]\n", p->handle);
		return;
	}
	
	data[p->handle].status |= 0x2;
	data[p->handle].gpio = p->gpio;
	data[p->handle].req_on = p->req_on;
	data[p->handle].req_off = p->req_off;

	//in case use flag is released by other fd
	data[p->handle].status |= (1<<3);
	
	return;
}

static void ali_pwm_stop(int handle)
{
	int ret = -1;
	int i;

	PWM_PRINT("handle[%d]\n", handle);
	
	if(handle >= PWM_CNT) {
		PWM_PRINT("invalid handle[%d]\n", handle);
		return;
	}

	sb_tmx_stop(handle);
	
	data[handle].status &= ~0x1;

	ali_gpio_direction_output(data[handle].gpio, 0);
	
	return;
}

static void ali_pwm_release(int handle)
{
	PWM_PRINT("handle[%d]\n", handle);
	
	if(handle >= PWM_CNT) {
		PWM_PRINT("invalid handle[%d]\n", handle);
		return;
	}

	data[handle].status &= ~(1<<3);
	data[handle].file = 0;
}

static int ali_pwm_close(struct inode *inode, struct file *file)
{
	int i;
	
	PWM_PRINT("Enter\n");

	for(i=0; i<PWM_CNT; i++) {
		if(data[i].file == (void *)file) {
			ali_pwm_stop(i);
			ali_pwm_release(i);
		}
	}
	
	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)
static long ali_pwm_ioctl(struct file * file, unsigned int cmd, unsigned long param)
#else
static int ali_pwm_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long param)
#endif
{
	long ret = 0;
	
	PWM_PRINT("Enter,cmd[%d]\n",cmd);

	switch(cmd) {
		case PWM_GET:
			{
				int handle = ali_pwm_get(file);
				
				if (0 != copy_to_user((void*)param, &handle, sizeof(handle)))
				{
					PWM_PRINT("copy handle fail!\n");
	        		ret = -EFAULT;
				}
			}
			break;
		case PWM_START:
		case PWM_START_FORCE:
			{
				int handle;

				if (0 != (copy_from_user(&handle, (void*)param, sizeof(handle))))
				{
					PWM_PRINT("copy param fail!\n");
					ret = -EFAULT;
					break;
				}

				ali_pwm_start(handle);
			}
			break;
		case PWM_SET_PARAM:
			{
				pwm_param paramK;
				if (0 != (copy_from_user(&paramK, (void*)param, sizeof(pwm_param))))
				{
					PWM_PRINT("copy param fail!\n");
					ret = -EFAULT;
					break;
				}
				ali_pwm_set_param(&paramK);
			}
			break;
		case PWM_STOP:
			{
				int handle;

				if (0 != (copy_from_user(&handle, (void*)param, sizeof(handle))))
				{
					PWM_PRINT("copy param fail!\n");
					ret = -EFAULT;
					break;
				}

				ali_pwm_stop(handle);
			}
			break;
		case PWM_RELEASE:
			{
				int handle;

				if (0 != (copy_from_user(&handle, (void*)param, sizeof(handle))))
				{
					PWM_PRINT("copy param fail!\n");
					ret = -EFAULT;
					break;
				}

				ali_pwm_release(handle);
			}
			break;
		case PWM_RESET:
			break;
		default:
			ret = -EPERM;
			break;
	}

	return ret;
}

static const struct file_operations ali_pwm_fops = {
	.owner = THIS_MODULE,
	.open  = ali_pwm_open,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)
	.unlocked_ioctl = ali_pwm_ioctl,
#else
	.ioctl			= ali_pwm_ioctl,
#endif
	.release = ali_pwm_close,
};

static struct miscdevice ali_pwm_misc = {
	.fops = &ali_pwm_fops,
	.name = ALI_PWM_DEV_NAME,
	.minor = MISC_DYNAMIC_MINOR,
};

static int __init ali_pwm_probe(struct platform_device *pdev)
{	
	int ret;
	//struct device *dev = &pdev->dev;	
	//const struct gpio_keys_platform_data *pdata = dev_get_platdata(dev);
	
	ret = misc_register(&ali_pwm_misc);
	if (ret < 0) {
		PWM_PRINT("ali_keypad_misc register failure,err[%d]\n",ret);
		return ret;
	}

	memset(data, 0, sizeof(data));
	
	data[0].irq = INT_ALI_RTC_TM2;
	data[1].irq = INT_ALI_RTC_TM3;
	data[2].irq = INT_ALI_RTC_TM5;
	data[3].irq = INT_ALI_RTC_TM6;
	data[4].irq = INT_ALI_RTC_TM7;


	ret = request_irq(INT_ALI_RTC_TM2, (irq_handler_t)pwm_isr, 0, ALI_PWM_DEV_NAME, NULL);
	if(ret != 0) {
		PWM_PRINT("request_irq INT_ALI_RTC_TM2 fail,ret[%d]\n",ret);	
	}
	
	/*
	//SB_TM3 is used by ali_pmu for rtc time,if no user need,we can use here
	ret = request_irq(INT_ALI_RTC_TM3, (irq_handler_t)pwm_isr, 0, ALI_PWM_DEV_NAME, NULL);
	if(ret != 0) {
		PWM_PRINT("request_irq INT_ALI_RTC_TM3 fail,ret[%d]\n",ret);	
	}*/
	
	
	ret = request_irq(INT_ALI_RTC_TM5, (irq_handler_t)pwm_isr, 0, ALI_PWM_DEV_NAME, NULL);
	if(ret != 0) {
		PWM_PRINT("request_irq INT_ALI_RTC_TM5 fail,ret[%d]\n",ret);	
	}

	/*//currently not work
	ret = request_irq(INT_ALI_RTC_TM6, (irq_handler_t)pwm_isr, 0, ALI_PWM_DEV_NAME, NULL);
	if(ret != 0) {
		PWM_PRINT("request_irq INT_ALI_RTC_TM6 fail,ret[%d]\n",ret);	
	}
	ret = request_irq(INT_ALI_RTC_TM7, (irq_handler_t)pwm_isr, 0, ALI_PWM_DEV_NAME, NULL);
	if(ret != 0) {
		PWM_PRINT("request_irq INT_ALI_RTC_TM7 fail,ret[%d]\n",ret);	
	}*/
	
	PWM_PRINT("initialized\n");
	
	return 0;
}

static int ali_pwm_remove(struct platform_device *pdev)
{
	misc_deregister(&ali_pwm_misc);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver ali_pwm_driver = {
	.driver = {
		.name = ALI_PWM_DEV_NAME,
		.owner = THIS_MODULE,
	},
	.probe = ali_pwm_probe,
	.remove = __devexit_p(ali_pwm_remove),
};

static int __init ali_pwm_driver_init(void)
{
	return platform_driver_register(&ali_pwm_driver);
}

static void __exit ali_pwm_driver_exit(void)
{
	platform_driver_unregister(&ali_pwm_driver);
}

module_init(ali_pwm_driver_init);
module_exit(ali_pwm_driver_exit);

MODULE_DESCRIPTION("Alitech PWM Driver");
MODULE_AUTHOR("Bobby Zhou");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
