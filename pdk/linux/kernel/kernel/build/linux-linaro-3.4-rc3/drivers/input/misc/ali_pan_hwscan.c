/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:    pan_hwscan.c
*
*    Description:    This file contains all functions definition
*		             of Front Panel driver.
*
*    Note:
*    .This driver is used to support all types of front panel device, so we use
*    "data driving" in the code. All HW depended case are all intergreted into
*    data structure.
*    .About the "COM" and "LED" number, some panel's LED number less than COM for
*    them use the extact COM for signal level or colon flag. We just let the
*    bitmap buffer last valiable byte for the colon flag, and second last
*    valiable byte for the signal level display.
*    .Even HW scan module support inerrupt to report key status change, we don't
*    use it for we need support repeat key. We use 1mS interrupt for key detect.
*    .This driver can support remote machine's panel through UART interface,
*    we immplement this function by eROM communication commands. You can control
*    the remote machine's front panel by use the second panel device handle.
*    History:
*           Date            Athor        Version          Reason
*	    ======= ======== ========   =========	=================
*	1.   2010.1.8     Martin Xia   Ver 0.1          Create file.
*	2.	 2010.1.13	  Elliott Hsu  Ver 0.2			Update for pan work.
*****************************************************************************/

#include <asm/mach-ali/m36_gpio.h>
#include "ali_pan_hwscan.h"
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/delay.h>		
#include <linux/miscdevice.h>
#include <asm/mach-ali/m36_irq.h>
#include <linux/kthread.h>
#include <linux/ali_front_panel.h>
#include <linux/ali_transport.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <ali_reg.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include "ali_ir_g2.h"


struct pan_hw_info ali_pan_hw_info =
{
	0,				/* type_kb : 2; Key board (array) type */
	1,				/* type_scan : 1; 0: Slot scan, 1: Shadow scan */
	1,				/* type_key: 1; Key exit or not */
	1,				/* type_irp: 3; 0: not IRP, 1: NEC, 2: LAB */
	0,				/* type_mcu: 1; MCU exit or not */
	4,				/* num_com: 4; Number of com PIN, 0 to 8 */   
	1,				/* Position of colon flag, 0 to 7 */
	1,				/* num_scan: 2; Number of scan PIN, 0 to 2 */
	0,				/* rsvd_bits:6; Reserved bits */
	0,              			/* rsvd byte for align pan_info */
	{0, HAL_GPIO_O_DIR,     GPIO_NOT_USED},		/* LATCH PIN */
	{0, HAL_GPIO_O_DIR, 	63},					/* CLOCK PIN */
	{1, HAL_GPIO_O_DIR, 	61},					/* DATA PIN */
	{{0, HAL_GPIO_I_DIR, 	65},					/* SCAN1 PIN */	
	{0, HAL_GPIO_I_DIR,       GPIO_NOT_USED}},	/* SCAN2 PIN */
	{{0, HAL_GPIO_O_DIR, 	57},					/* COM1 PIN */
	{0, HAL_GPIO_O_DIR, 	58},					/* COM2 PIN */
	{0, HAL_GPIO_O_DIR, 	59},					/* COM3 PIN */
	{0, HAL_GPIO_O_DIR, 	60},					/* COM4 PIN */
	{0, HAL_GPIO_O_DIR, 	GPIO_NOT_USED},		/* COM5 PIN */
	{0, HAL_GPIO_O_DIR, 	GPIO_NOT_USED},		/* COM6 PIN */
	{0, HAL_GPIO_O_DIR, 	GPIO_NOT_USED},		/* COM7 PIN */
	{0, HAL_GPIO_O_DIR, 	GPIO_NOT_USED}},	/* COM8 PIN */
	{{1, HAL_GPIO_O_DIR, 	68},					/* POWER PIN, 0: Standby, 1: Work */
	{0, HAL_GPIO_O_DIR, 	64},					/* LOCK PIN */
	{0, HAL_GPIO_O_DIR, 	GPIO_NOT_USED},		/* Extend function LBD */
	{0, HAL_GPIO_O_DIR, 	GPIO_NOT_USED}},	/* Extend function LBD */
	{0, HAL_GPIO_O_DIR, 	GPIO_NOT_USED},        	/* rsvd extend function LBD */      
	300,										/* Intv repeat first */
	250,										/* Intv repeat */
	0,										/* Intv release, 0: disable release key */
	//NULL,	 	 	 	 	 	 			/* hook_scan() callback */
	//NULL,	 	 	 	 	 	 			/* hook_show() callback */
};


/* HW Scan register define */
#define	R_DOUT		(HW_SCAN_REG_BASE + 0x00)	/* Data output register */
#define	R_SRATE	(HW_SCAN_REG_BASE + 0x10)	/* Scan rate register */
#define	R_DEVCLK	(HW_SCAN_REG_BASE + 0x12)	/* Device clock control register */
#define	R_SHIFT		(HW_SCAN_REG_BASE + 0x14)	/* Shift control register */
#define	R_KEYMODE	(HW_SCAN_REG_BASE + 0x15)	/* Key scan mode register */
#define	R_INTMASK	(HW_SCAN_REG_BASE + 0x16)	/* Interrupt mask register */
#define	R_INTPOS	(HW_SCAN_REG_BASE + 0x18)	/* Interrupt positive setting register */
#define	R_INTNEG	(HW_SCAN_REG_BASE + 0x1a)	/* Interrupt negative setting register */
#define	R_BUTTON	(HW_SCAN_REG_BASE + 0x1c)	/* Button press value register */
#define	R_INTSTAT	(HW_SCAN_REG_BASE + 0x1e)	/* Interrupt status register */
#define	R_COMCFG	(HW_SCAN_REG_BASE + 0x20)	/* COM PIN configuration base register */
#define	R_KEYCFG	(HW_SCAN_REG_BASE + 0x28)	/* KEY PIN configuration base register */
#define	R_DATCFG	(HW_SCAN_REG_BASE + 0x2a)	/* DATA PIN configuration register */
#define	R_CLKCFG	(HW_SCAN_REG_BASE + 0x2b)	/* CLOCK PIN configuration register */
#define	R_LATCFG	(HW_SCAN_REG_BASE + 0x2c)	/* LATCH PIN configuration register */
#define     R_COM7THADDR  (HW_SCAN_REG_BASE + 0x60)	/* com 7th address configuration base register */
#define	R_OTHER7THADDR	(HW_SCAN_REG_BASE + 0x61)	/* other 7th address configuration base register */

#define	HW_SCAN_RATE		2000/* LED lamping time in HW_SCAN_CLOCK */
#define	HW_SCAN_CLOCK		32	/* Fclock = MEM_FREQ/(4*HW_SCAN_CLOCK) */

#define ALI_SOC_BASE 0x18000000

#ifndef UPGRADE_FORMAT
#define UPGRADE_FORMAT  0
#endif
#ifndef BOOT_UPG
#define BOOT_UPG        1
#endif

/*this array defines number to digital conversion
 *
 *      ###b7##
 *    b2#     #b6
 *      ###b1##
 *    b3#     #b5
 *      ###b4##  #b0#
 */
struct led_bitmap ali_hwscan_bitmap_table[] =
{
	{'.', 0xfe}, /* Let's put the dot bitmap into the table */
	{'0', 0x03}, {'1', 0x9f}, {'2', 0x25}, {'3', 0x0d},
	{'4', 0x99}, {'5', 0x49}, {'6', 0x41}, {'7', 0x1f},
	{'8', 0x01}, {'9', 0x09}, {'a', 0x11}, {'A', 0x11},
	{'b', 0xc1}, {'B', 0xc1}, {'c', 0x63}, {'C', 0x63},
	{'d', 0x85}, {'D', 0x85}, {'e', 0x61}, {'E', 0x61},
	{'f', 0x71}, {'F', 0x71}, {'g', 0x09}, {'G', 0x43},
	{'h', 0x91}, {'H', 0x91}, {'i', 0xdf}, {'I', 0xf3},
	{'j', 0x8f}, {'J', 0x8f}, {'l', 0xe3}, {'L', 0xe3},
	{'n', 0xd5}, {'N', 0x13}, {'o', 0xc5}, {'O', 0x03},
	{'p', 0x31}, {'P', 0x31}, {'q', 0x19}, {'Q', 0x19},
	{'r', 0xf5}, {'R', 0x11}, {'s', 0x49}, {'S', 0x49},
	{'t', 0xe1}, {'T', 0x73}, {'u', 0x83}, {'U', 0x83},
	{'y', 0x89}, {'Y', 0x89}, {'z', 0x25}, {'Z', 0x25},
	{':', 0x3f}, {'-', 0xfd}, {'_', 0xef}, {' ', 0xff},
	// special display for IRD
	{0x10, 0x98},{0x11, 0xf7}, {0x12, 0xfb}, {0x13, 0x7f},
	{0x14, 0xbf}, {0x15, 0xdf},{0x16, 0xef},
};

static struct ali_fp_key_map_cfg g_hwscan_key_map;
static unsigned long pan_key_received_count = 0;
static unsigned long pan_key_mapped_count = 0;
static unsigned char pan_debug = 0;
static unsigned long hwscan_repeat_delay_rfk = 600;
static unsigned long hwscan_repeat_interval_rfk = 350;
static unsigned long hwscan_repeat_width_rfk = 0;
static unsigned int    hwscan_rfk_port = 0;
static struct proc_dir_entry *proc_hwscan;
struct input_dev *g_hwscan_input = NULL;

#define PANEL_PRINTK(fmt, args...)			\
{										\
	if (0 !=  pan_debug)					\
	{									\
		printk(fmt, ##args);					\
	}									\
}

#define PANEL_ERR_PRINTK(fmt, args...)		\
{										\
	printk(fmt, ##args);						\
}


long  pan_hwscan_open(struct ali_pan_hwscan_private *tp);
long  pan_hwscan_close(struct ali_pan_hwscan_private *tp);


static int hwscan_read_proc (char *buffer, char **start, off_t off, int count,
			  int *eof, void *data_unused)
{
	int len = 0;    

	
	if (off > 0)
	{
		return 0;
	}
	len = sprintf(buffer, "%s", HWSCAN_DEV_NAME);
	*start = buffer;	
	*eof = 1;

        	return len;
}



static int ali_hwscan_send_msg(unsigned long  code, int ir_protocol, int ir_state)
{
	unsigned char msg[16];
	unsigned char i = 0;
	int ret = -1;

	
	memset(msg, 0x00, sizeof(msg));
	msg[4] = (unsigned char)(code >> 24);
	msg[5] = (unsigned char)(code >> 16);
	msg[6] = (unsigned char)(code >> 8);
	msg[7] = (unsigned char)(code);

	msg[8] = (unsigned char)(ir_protocol >> 24);
	msg[9] = (unsigned char)(ir_protocol >> 16);
	msg[10] = (unsigned char)(ir_protocol >> 8);
	msg[11] = (unsigned char)(ir_protocol);

	msg[12] = (unsigned char)(ir_state >> 24);
	msg[13] = (unsigned char)(ir_state >> 16);
	msg[14] = (unsigned char)(ir_state >> 8);
	msg[15] = (unsigned char)(ir_state);

	
	PANEL_PRINTK("[ %s %d ], rfk port %d send msg :\n", __FUNCTION__, __LINE__, hwscan_rfk_port);
	for (i=0; i<sizeof(msg); i++)
	{
		PANEL_PRINTK("%02x ", msg[i]);
	}
	PANEL_PRINTK("\n");
	
	
	
	ret = ali_transport_send_msg(hwscan_rfk_port, &msg, sizeof(msg));
	if (-1 == ret)
	{
		PANEL_ERR_PRINTK("[ %s %d ], rfk port %d send msg fail!\n", __FUNCTION__, __LINE__, hwscan_rfk_port);
	}

	
	return ret;
}


/* Register operation code for local panel */
static unsigned char read8_local(unsigned long addr)
{
	return (__REG8ALI(addr));
}

static unsigned short read16_local(unsigned long addr)
{
	return (__REG16ALI(addr));
}

static void write8_local(unsigned long addr, unsigned char val)
{
	((__REG8ALI(addr))=(val));
}

static void write16_local(unsigned long addr, unsigned short val)
{
	((__REG16ALI(addr))=(val));
}


static void gpio_dir_set_local(long pos, long dir)
{
	gpio_direction_input(pos);
}
static void gpio_val_set_local(long pos, long val)
{	
	gpio_direction_output(pos, val);
}

unsigned long HW_SCAN_REG_BASE;
static unsigned short ali_hwscan_gpio_set_value = 0;

//static void  pan_hwscan_interrupt(void *dev_id);
static int pan_hwscan_start_timer(struct ali_pan_hwscan_private *tp);

void pan_hwscan_595_gpio_set(struct ali_pan_hwscan_private *tp, unsigned short value, unsigned short gpio_mask)
{
	struct pan_hw_info *hp = tp->hw_info;
	
	ali_hwscan_gpio_set_value &= (~gpio_mask);
	ali_hwscan_gpio_set_value |= value; 	
	tp->write8(R_DOUT + (hp->num_com*2+1), ali_hwscan_gpio_set_value);
}


static void pan_hwscan_info_dump(void)
{
	int i = 0;

	
	PANEL_PRINTK("[ %s %d ]\n", __FUNCTION__, __LINE__);
	PANEL_PRINTK("type_kb = %d\n",  ali_pan_hw_info.type_kb);
	PANEL_PRINTK("type_scan = %d\n",  ali_pan_hw_info.type_scan);
	PANEL_PRINTK("type_key = %d\n",  ali_pan_hw_info.type_key);
	PANEL_PRINTK("type_irp = %d\n",  ali_pan_hw_info.type_irp);
	PANEL_PRINTK("type_mcu = %d\n",  ali_pan_hw_info.type_mcu);
	PANEL_PRINTK("num_com = %d\n",  ali_pan_hw_info.num_com);
	PANEL_PRINTK("pos_colon = %d\n",  ali_pan_hw_info.pos_colon);
	PANEL_PRINTK("num_scan = %d\n",  ali_pan_hw_info.num_scan);
	PANEL_PRINTK("rsvd_bits = %d\n",  ali_pan_hw_info.rsvd_bits);
	PANEL_PRINTK("rsvd_byte = %d\n",  ali_pan_hw_info.rsvd_byte);
	
	PANEL_PRINTK("flatch.polar = %d\n",  ali_pan_hw_info.flatch.polar);
	PANEL_PRINTK("flatch.io = %d\n",  ali_pan_hw_info.flatch.io);
	PANEL_PRINTK("flatch.position = %d\n",  ali_pan_hw_info.flatch.position);
	
	PANEL_PRINTK("fclock.polar = %d\n",  ali_pan_hw_info.fclock.polar);
	PANEL_PRINTK("fclock.io = %d\n",  ali_pan_hw_info.fclock.io);
	PANEL_PRINTK("fclock.position = %d\n",  ali_pan_hw_info.fclock.position);

	PANEL_PRINTK("fdata.polar = %d\n",  ali_pan_hw_info.fdata.polar);
	PANEL_PRINTK("fdata.io = %d\n",  ali_pan_hw_info.fdata.io);
	PANEL_PRINTK("fdata.position = %d\n",  ali_pan_hw_info.fdata.position);

	PANEL_PRINTK("scan[0].polar = %d\n",  ali_pan_hw_info.scan[0].polar);
	PANEL_PRINTK("scan[0].io = %d\n",  ali_pan_hw_info.scan[0].io);
	PANEL_PRINTK("scan[0].position = %d\n",  ali_pan_hw_info.scan[0].position);

	PANEL_PRINTK("scan[1].polar = %d\n",  ali_pan_hw_info.scan[1].polar);
	PANEL_PRINTK("scan[1].io = %d\n",  ali_pan_hw_info.scan[1].io);
	PANEL_PRINTK("scan[1].position = %d\n",  ali_pan_hw_info.scan[1].position);

	for (i=0; i<8; i++)
	{
		PANEL_PRINTK("com[%d].polar = %d\n",  i, ali_pan_hw_info.com[i].polar);
		PANEL_PRINTK("com[%d].io = %d\n",  i, ali_pan_hw_info.com[i].io);
		PANEL_PRINTK("com[%d].position = %d\n",  i, ali_pan_hw_info.com[i].position);
	}

	for (i=0; i<4; i++)
	{
		PANEL_PRINTK("lbd[%d].polar = %d\n",  i, ali_pan_hw_info.lbd[i].polar);
		PANEL_PRINTK("lbd[%d].io = %d\n",  i, ali_pan_hw_info.lbd[i].io);
		PANEL_PRINTK("lbd[%d].position = %d\n",  i, ali_pan_hw_info.lbd[i].position);
	}

	PANEL_PRINTK("rsvd_hw.polar = %d\n",  ali_pan_hw_info.rsvd_hw.polar);
	PANEL_PRINTK("rsvd_hw.io = %d\n",  ali_pan_hw_info.rsvd_hw.io);
	PANEL_PRINTK("rsvd_hw.position = %d\n",  ali_pan_hw_info.rsvd_hw.position);

	PANEL_PRINTK("delay = %ld\n",  ali_pan_hw_info.delay);	
	PANEL_PRINTK("interval = %ld\n",  ali_pan_hw_info.interval);
	PANEL_PRINTK("release = %ld\n",  ali_pan_hw_info.release);	
}


#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)
static long pan_hwscan_ioctl(struct file * file, unsigned int cmd, unsigned long param)
#else
static int pan_hwscan_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long param)
#endif
{
	int i = 0;	
	unsigned long reg[2] = {0, 0};
	unsigned char * map_entry = NULL;
	struct ali_fp_key_map_cfg key_map;
	unsigned short key = 0;
	struct ali_pan_hwscan_private *tp = file->private_data;
	
	
	PANEL_PRINTK("[ %s %d ]: cmd %d\n", __FUNCTION__, __LINE__, cmd);	
	
	switch (cmd)
	{
		case PAN_DRIVER_SET_HW_INFO:
		{			
			if (0 != copy_from_user(&ali_pan_hw_info, (void*)param, sizeof(ali_pan_hw_info)))			
			{
				PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
        				return -EFAULT;
			}	
		
			pan_hwscan_info_dump();	
			pan_hwscan_close(tp);			
			pan_hwscan_open(tp);			
		}
		break;

		case PAN_DRIVER_GET_HW_INFO:
			if (0 != copy_to_user((void*)param, &ali_pan_hw_info, sizeof(ali_pan_hw_info)))
			{
				PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
        				return -EFAULT;
			}	
			break;
			
		case PAN_DRIVER_SUSPEND:
			break;
			
		case PAN_DRIVER_RESUME:
			break;		
			
		case PAN_DRIVER_READ_LNB_POWER:
			break;
			
		case PAN_DRIVER_SET_GPIO: 
			//pan_hwscan_595_gpio_set(tp, param&0xff, (param>>8)&0xff);
			break;
			
		case PAN_DRIVER_GET_GPIO:
			*(unsigned long*)param = ali_hwscan_gpio_set_value;
			break;
			
		case PAN_DRIVER_STANDBY:
			// In low power mode, cpu & mem clock is 12MHz, south bridge clock is 27MHz/64
			// So, scan rate and scan clock need to adjust.
			//tp->write16(R_SRATE, 1000);
			//tp->write16(R_DEVCLK, 0xC000 | 3);
			break;

		case PAN_DRIVER_READ_REG: 			
			if (0 != (copy_from_user(&reg, (void*)param, sizeof(reg))))
			{
				PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}			
			
			reg[1] = __REG32ALI(reg[0]);			
			PANEL_PRINTK("[ %s %d ]: read 0x%08x = 0x%08x\n\n", 
				__FUNCTION__,  __LINE__, (unsigned int)reg[0], (unsigned int)reg[1]);				
			
			if (0 != copy_to_user((void*)param, &reg, sizeof(reg)))
			{
				PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
        				return -EFAULT;
			}				
			
			break;		

		case PAN_DRIVER_WRITE_REG: 				
			if (0 != (copy_from_user(&reg, (void*)param, sizeof(reg))))
			{
				PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}		
			
			__REG32ALI(reg[0]) = reg[1];						
			PANEL_PRINTK("[ %s %d ]: write 0x%08x = 0x%08x\n", 
				__FUNCTION__,  __LINE__, (unsigned int)reg[0], (unsigned int)(__REG32ALI(reg[0])));							
			
			break;

		case ALI_FP_CONFIG_KEY_MAP:
		{					
			if (0 != (copy_from_user(&key_map, (void*)param, sizeof(key_map))))
			{			
				PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}			
			
			PANEL_PRINTK("[ %s %d ], phy_code = %ld\n", __FUNCTION__, __LINE__, key_map.phy_code);			
			
			g_hwscan_key_map.phy_code = key_map.phy_code;
			if (2 != g_hwscan_key_map.phy_code)
			{				
				map_entry = kzalloc(key_map.map_len, GFP_KERNEL);
				if(NULL==map_entry)
				{
					PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
					return -ENOMEM;				
				}				
				
				if (0 != (copy_from_user(map_entry, key_map.map_entry, key_map.map_len)))
				{
					PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
					return -EFAULT;
				}	
				
				key_map.map_entry = map_entry;				
				key_map.unit_num = (key_map.map_len/key_map.unit_len);				
				if(key_map.unit_num>KEY_CNT)
				{
					key_map.unit_num = KEY_CNT;
				}
				
				if(g_hwscan_key_map.map_entry)
				{
					kfree(g_hwscan_key_map.map_entry);
				}
				
				g_hwscan_key_map.map_entry = NULL;
				
				for (i = 0; i < key_map.unit_num; ++i)
				{
					/*
					phy_code : 0, logic; 1, index.
					*/
					if(1 == key_map.phy_code)
					{
						key = (unsigned short)i;						
					}
					else
					{						
						key = key_map.map_entry[i*key_map.unit_len+4];						
						key |= (key_map.map_entry[i*key_map.unit_len+5])<<8;						
					}				
					
					__set_bit(key%KEY_CNT, tp->ali_pan_hwscan_input->keybit);
				}
				
				g_hwscan_key_map.map_len = key_map.map_len;
				g_hwscan_key_map.unit_len = key_map.unit_len;
				g_hwscan_key_map.phy_code = key_map.phy_code;
				g_hwscan_key_map.unit_num = key_map.unit_num;
				g_hwscan_key_map.map_entry = key_map.map_entry;
				if (0 != pan_debug)
				{									
					for(i=0; i<g_hwscan_key_map.unit_num; i++){
						unsigned char * buf = &map_entry[i*g_hwscan_key_map.unit_len];
						unsigned long phy ;
						unsigned short log;
						phy = buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24);
						log = buf[4]|(buf[5]<<8);
						PANEL_PRINTK("%08x	%04x\n", (int)phy, log);
					}
					PANEL_PRINTK("\n");
				}
			}			
		}
		break;
		
		case ALI_FP_GET_PAN_KEY_RECEIVED_COUNT:
		{					
			PANEL_PRINTK("[ %s ]: pan_key_received_count = %ld\n", __FUNCTION__, pan_key_received_count);			
			if (0 != copy_to_user((void*)param, &pan_key_received_count, sizeof(pan_key_received_count)))
			{
				PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
        				return -EFAULT;
			}	
		}
		break;

		case ALI_FP_GET_PAN_KEY_MAPPED_COUNT:
		{				
			PANEL_PRINTK("[ %s ]: pan_key_mapped_count = %ld\n", __FUNCTION__, pan_key_mapped_count);			
			if (0 != copy_to_user((void*)param, &pan_key_mapped_count, sizeof(pan_key_mapped_count)))
			{
				PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
        				return -EFAULT;
			}	
		}
		break;

		case ALI_FP_SET_PAN_KEY_DEBUG:
		{					
			if (0 != copy_from_user(&pan_debug, (void*)param, sizeof(pan_debug)))			
			{
				PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
        				return -EFAULT;
			}	
			
			PANEL_PRINTK("[ %s ]: pan_debug = %d\n", __FUNCTION__, pan_debug);			
		}
		break;		

		case ALI_FP_SET_SOCKPORT:
		{				
			if (0 != (copy_from_user(&hwscan_rfk_port, (void*)param, sizeof(hwscan_rfk_port))))
			{
				PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}
			
			PANEL_PRINTK("[ %s ]: hwscan_rfk_port = %d\n", __FUNCTION__, hwscan_rfk_port);				
		}
		break;	
		
			
		default:			
			return -EPERM;
			break;
	}

	return 0;
}

/*
* Name          :   pan_hwscan_open()
* Description   :   Open front panel
* Parameter     :   struct pan_device *dev
* Return        :   void
*/
long  pan_hwscan_open(struct ali_pan_hwscan_private *tp)
{
	struct pan_hw_info *hp = tp->hw_info;
	unsigned int i = 0;
	unsigned short com_data = 0, led_data = 0;
	unsigned short value = 0,t_value = 0;	
	
	
	HW_SCAN_REG_BASE = 0x18018400;

	tp->ali_pan_hwscan_input->rep[REP_DELAY] = tp->hw_info->delay;
	tp->ali_pan_hwscan_input->rep[REP_PERIOD] = tp->hw_info->interval;

	tp->key_cnt = 0;
	tp->keypress_cnt = 0;
	tp->keypress_intv = hp->delay;
	tp->keypress_bak = PAN_KEY_INVALID;	

	
	for (i = 0; i < 4; i++)
	{		
		tp->gpio_val_set(hp->lbd[i].position, !hp->lbd[i].polar);
	}
	
	/* Init HW registers */
	if (hp->type_kb & 0x02)
	{
		for (i = 0; i < 4; i++)
		{
			if (hp->lbd[i].polar == 0)	/* Set to invalidate status */
			{
				led_data |= (1 << hp->lbd[i].position);
			}
		}
		led_data &= 0xff;
	}
	/* Clear all data buffer & set COM */
	for (i = 0; i < 8; i++)				
	{
		if (hp->type_kb & 0x01)
		{
			com_data = 1 << hp->com[i].position;
			if (hp->com[i].polar == 0)
			{
				com_data ^= 0xff;
			}
		}
		com_data &= 0xff;
		tp->write16(R_DOUT + (i << 1), (tp->blankmap << 8) | com_data | led_data);
	}
	/* Clear all GPIO seting */
	for (i = 0; i < 13; i++)			
	{
		tp->write8(R_COMCFG + i, 0);
	}
	
	/* Set COM GPIO */
	for (i = 0; i < hp->num_com; i++)
	{
		if (hp->com[i].io == HAL_GPIO_O_DIR)
		{
			/* write bit0-bit5 to R_COMCFG */
			tp->write8(R_COMCFG + i, (hp->com[i].position) | (hp->com[i].polar << 6) | 0x80);		
			/* write bit6 of hp->com[i].position to R_COM7THADDR */
			tp->write8(R_COM7THADDR, ((hp->com[i].position & 0x40) >> 6) << i);	
		}
	}	
	/* Set shifter GPIO */
	if (hp->fdata.io == HAL_GPIO_O_DIR)
	{
		/* write bit0-bit5 to R_DATCFG */
		tp->write8(R_DATCFG, (hp->fdata.position) | (~(hp->fdata.polar) << 6) | 0x80);
		/* write bit6 of hp->fdata.position to bit2 of R_OTHER7THADDR */		
		tp->write8(R_OTHER7THADDR, (hp->fdata.position & 0x40) >> 4);		
	}
	
	//tp->gpio_dir_set(hp->fdata.position, hp->fdata.io);

	if (hp->fclock.io == HAL_GPIO_O_DIR)
	{
		/* write bit0-bit5 to R_CLKCFG */
		tp->write8(R_CLKCFG, (hp->fclock.position) | (hp->fclock.polar << 6) | 0x80);
		/* write bit6 of hp->fclock.position to bit3 of R_OTHER7THADDR */
		tp->write8(R_OTHER7THADDR, (hp->fclock.position & 0x40) >> 3);			
	}
	
	//tp->gpio_dir_set(hp->fclock.position, hp->fclock.io);

	if (hp->flatch.io == HAL_GPIO_O_DIR && hp->flatch.position != GPIO_NOT_USED)
	{
		/* write bit0-bit5 to R_LATCFG */
		tp->write8(R_LATCFG, (hp->flatch.position) | (hp->flatch.polar << 6) | 0x80);
		/* write bit6 of hp->flatch.position to bit4 of R_OTHER7THADDR */
		tp->write8(R_OTHER7THADDR, (hp->flatch.position & 0x40) >> 2);	
	}

	
	/* Set scan information and enable LED display */
	tp->write8(R_SHIFT, (hp->type_scan ? hp->num_com : hp->num_com - 1) << 4);	/* Set COM number */
	tp->write8(R_KEYMODE, hp->type_scan);		/* Set keypad type */
	tp->write16(R_SRATE, HW_SCAN_RATE);			/* Set scan rate */
	tp->write16(R_INTMASK, 0);					/* All bit interupt disable */
	tp->write16(R_DEVCLK, 0xC000 | HW_SCAN_CLOCK);	/* Set scan clock & enable it */
	/* Set key scan GPIO */
	
	if (hp->type_key != 0)						/* Need scan key or not */
	{		
		for (i = 0; i < hp->num_scan; i++)		/* Set KEY GPIO */
		{			
			if (hp->scan[i].io == HAL_GPIO_I_DIR)
			{
				tp->gpio_dir_set(hp->scan[i].position, hp->scan[i].io);
				/* write bit0-bit5 to R_KEYCFG */
				tp->write8(R_KEYCFG + i, (hp->scan[i].position) | (hp->scan[i].polar << 6) | 0x80);	
				/* write bit6 of hp->scan[i].position to bit0 and bit1 of R_OTHER7THADDR */
				tp->write8(R_OTHER7THADDR, ((hp->scan[i].position & 0x40) >> 6) << i);
			}
		}	
	}
	/* Generate active key bit mask */
	if (hp->type_scan == 0)
	{
		tp->maskkey = (hp->num_scan == 1) ? 0x0001 : 0x0003;
		for (i = 0; i < hp->num_com - 1; i++)
		{
			tp->maskkey |= (tp->maskkey << 2);
		}
	}
	else
	{
		tp->maskkey = (hp->num_scan == 1) ? 0x00ff : 0xffff;
	}
	
	/* Generate idel key value */
	if (hp->num_scan != 0 && hp->scan[0].polar == 0)
	{
		tp->idlekey = tp->maskkey;
	}
	
	if (hp->type_scan == 1)
	{
		if (hp->scan[0].polar)
		{
			value = 0x00;
			t_value = 0x01;
		}
		else
		{
			value = 0xff;
			t_value = 0xfe;
		}
		
		tp->write8(R_DOUT + (hp->num_com << 1), value);
		tp->write8(R_DOUT + (hp->num_com << 1) + 1, t_value);
	}
	
	pan_hwscan_start_timer(tp);
	
	return 0;
}

/*
* Name          :   pan_hwscan_close()
* Description   :   close front panel
* Parameter     :   struct pan_device *dev
* Return        :   void
*/
long  pan_hwscan_close(struct ali_pan_hwscan_private *tp)
{
	struct pan_hw_info *hp = tp->hw_info;
	int i = 0;


	/* Set default bitmap in buffer. At same clear display. */
	for (i = 0; i < 16; i++)					/* Clear all data buffer */
	{
		tp->write8(R_DOUT + i, tp->blankmap);
	}
	/* A delay to let shifter send blank bitmap out
	    to avoid flash while closing panel (by trueve_hu) */
	//udelay(10000);	
	msleep(10);
	
	for (i = 0; i < 13; i++)					/* Clear all GPIO seting */
	{
		tp->write8(R_COMCFG + i, 0);
	}
	tp->write16(R_INTMASK, 0);
	tp->write16(R_DEVCLK, 0);					/* Disable device */

	/* Un-register key scan interrupt ISR */
	if (tp->is_local == 1)				/* Unregiste IRQ when local device */
	{
		free_irq(M36_IRQ_PANEL, (void*)tp);
	}
	
	for (i = 0; i < hp->num_com; i++)
	{
		tp->gpio_val_set(hp->com[i].position, !hp->com[i].polar);
	}

	//tp->run = 0;
	if (tp->thread_id)
	{
		kthread_stop(tp->thread_id);
		tp->thread_id = NULL;
	}
	
	return 0;
}


static int hwscan_key_mapping(unsigned long phy_code)
{
	int i = 0;
	int logic_code = 0;

	
	if(NULL== g_hwscan_key_map.map_entry)
	{		
		return -1;		
	}
	
	for(i = 0; i<g_hwscan_key_map.unit_num; i++)
	{
		unsigned long my_code;
		
		
		if((unsigned long)(&g_hwscan_key_map.map_entry[i*g_hwscan_key_map.unit_len]) & 0x3)
		{
			my_code = g_hwscan_key_map.map_entry[i*g_hwscan_key_map.unit_len];			
			my_code |= (g_hwscan_key_map.map_entry[i*g_hwscan_key_map.unit_len+1])<<8;			
			my_code |= (g_hwscan_key_map.map_entry[i*g_hwscan_key_map.unit_len+2])<<16;			
			my_code |= (g_hwscan_key_map.map_entry[i*g_hwscan_key_map.unit_len+3])<<24;			
		}
		else
		{
			my_code = *((unsigned long *)(&g_hwscan_key_map.map_entry[i*g_hwscan_key_map.unit_len]));
		}
		//PANEL_PRINTK("my_code = 0x%08x\n\n", my_code);
		if(phy_code == my_code)
		{			
			if(1 == g_hwscan_key_map.phy_code)
			{				
				PANEL_PRINTK("[ %s ], phy code index %d\n", __FUNCTION__, i);
				
				return i;
			}
			else
			{
				logic_code =  g_hwscan_key_map.map_entry[i*g_hwscan_key_map.unit_len+4]; 				
				logic_code |= (g_hwscan_key_map.map_entry[i*g_hwscan_key_map.unit_len+5])<<8;						
				
				PANEL_PRINTK("logic code 0x%08x\n", logic_code);				
				
				return logic_code;
			}
		}	
	}

	
	return -1;
}


/*
* Name          :   pan_hwscan_timer()
* Description   :   front panel poll function
* Parameter     :   None
* Return        :   void
*/
int pan_hwscan_timer(void *param)
{
	struct ali_pan_hwscan_private *tp = (struct ali_pan_hwscan_private *)param;
	struct pan_hw_info *hp = tp->hw_info;
	unsigned long keypress = PAN_KEY_INVALID;
	unsigned short key_temp = 0;
	static unsigned char cnt = 0;
	int key_index = 0;
	static unsigned long last_repeat_tick = 0;	
	unsigned long last_repeat_width = 0;
	unsigned long repeat_tick = 0;


	while (!kthread_should_stop())
	{
		keypress = PAN_KEY_INVALID;
		key_temp = tp->read16(R_BUTTON);

	
		/* key_temp = 0x0000, first time. default is 0x00ff */
		/* tp->maskkey = 0x00ff */
		/* idlekey = 0x00ff */
		/* hp->scan[0].polar = 0 */
		/* keypress = 0xffff0001, key_temp = 0x00fe */
		if ((key_temp & tp->maskkey) != tp->idlekey)
		{
			keypress = 0xffff0000 | ((hp->scan[0].polar ? key_temp : ~key_temp) & tp->maskkey);
		}
	
		#if 0
		if (tp->hw_info->hook_scan)
		{
			keypress = tp->hw_info->hook_scan(tp, keypress);
		}
		#endif
	

		/* Some key input */
		if (keypress != PAN_KEY_INVALID)
		{					
			if (tp->keypress_bak == keypress)
			{		
				/* If is the same key, count it */
				tp->keypress_cnt++;				
				
				if (tp->keypress_cnt == 2)
				{			
					if(2 != g_hwscan_key_map.phy_code)
					{
						key_index = hwscan_key_mapping(tp->keypress_bak);
						if (key_index >= 0)
						{		
							pan_key_mapped_count ++;

							/*ali_hwscan_key_map[key_index].key*/
							input_report_key(tp->ali_pan_hwscan_input, (unsigned int)key_index, PAN_KEY_PRESSED);
							input_sync(tp->ali_pan_hwscan_input);
							
							PANEL_PRINTK("[ %s %d ] : key_index %d(0x%08x) pressed \n", __FUNCTION__, __LINE__, 
								key_index, (unsigned int)tp->keypress_bak);
							
						}
						else
						{
							if (tp->keypress_bak != 0xffff00ff)
							{
								PANEL_ERR_PRINTK("[ %s %d ] : Not matched key for panel code = 0x%x \n", 
									__FUNCTION__, __LINE__, (unsigned int)tp->keypress_bak);	
							}
						}				
					}
					else
					{
						last_repeat_tick = jiffies;
						ali_hwscan_send_msg(tp->keypress_bak, IR_TYPE_UNDEFINE, PAN_KEY_PRESSED);
					}

				}
				else
				{
					/* step 2, repeat */		
					if(2 == g_hwscan_key_map.phy_code)
					{				
						repeat_tick = jiffies;					
						last_repeat_width = (unsigned long)(((long)repeat_tick - (long)last_repeat_tick));							
						if (last_repeat_width > hwscan_repeat_width_rfk)
						{		
							last_repeat_tick = repeat_tick;
							hwscan_repeat_width_rfk = hwscan_repeat_interval_rfk;
							ali_hwscan_send_msg(tp->keypress_bak, IR_TYPE_UNDEFINE, PAN_KEY_REPEAT);							
						}
					}
				}

			}	
			else
			{
				/* Else is a new key, backup it */				
				pan_key_received_count ++;				
				tp->keypress_bak = keypress;
				tp->keypress_cnt = 1;				
			}
		}
		/* No key input, if the same key switch to invalid, reset it */
		else if (tp->keypress_bak != PAN_KEY_INVALID)
		{
			//PANEL_PRINTK("pan_hwscan no key input\n");
			if (cnt++ > 2)
			{						
				if(2 != g_hwscan_key_map.phy_code)
				{
					key_index = hwscan_key_mapping(tp->keypress_bak);
					if (key_index >= 0)
					{				
						input_report_key(tp->ali_pan_hwscan_input, (unsigned int)key_index, PAN_KEY_RELEASE);
						input_sync(tp->ali_pan_hwscan_input);
						
						PANEL_PRINTK("[ %s %d ], key_index %d(0x%08x) release \n", __FUNCTION__, __LINE__, 
							key_index, (unsigned int)tp->keypress_bak);						
					}
					else
					{
						if (tp->keypress_bak != 0xffff00ff)
						{
							PANEL_ERR_PRINTK("[ %s %d ] : Not matched key for panel code = 0x%x \n", 
								__FUNCTION__, __LINE__, (unsigned int)tp->keypress_bak);
						}
					}
				}
				else
				{
					hwscan_repeat_width_rfk = hwscan_repeat_delay_rfk;
					ali_hwscan_send_msg(tp->keypress_bak, IR_TYPE_UNDEFINE, PAN_KEY_RELEASE);	
				}			

				
				tp->keypress_bak = PAN_KEY_INVALID;
				tp->keypress_cnt = 0;				
				cnt = 0;
			}
		}
			
		
		msleep(10);
	}


	return 0;
}

/*
* Name          :   pan_hwscan_start_timer()
* Description   :   front panel start poll function
* Parameter     :   None
* Return        :   void
*/
static int pan_hwscan_start_timer(struct ali_pan_hwscan_private *tp)
{
	//tp->run = 1;
	tp->thread_id = kthread_create(pan_hwscan_timer, (void *)tp, "ali_pan_hwscan");
	if(IS_ERR(tp->thread_id)){
		PANEL_ERR_PRINTK("pan hwscan kthread create fail\n");
		tp->thread_id = NULL;
		return -1;
	}
	wake_up_process(tp->thread_id);
	
	return 0;
}

/*
* Name          :   pan_hwscan_esc_command()
* Description   :   Do ESC command
* Parameter     :   unsigned short* data			: Command data
*                   unsigned long limit_len	: Length limit
* Return        :   unsigned long				: command length
*/
static unsigned long pan_hwscan_esc_command(struct ali_pan_hwscan_private *tp, unsigned char *data, unsigned long limit_len)
{
	struct pan_hw_info *hp = tp->hw_info;
	struct pan_gpio_info *gp = NULL;
	unsigned long dp = 0;
	unsigned long addr = 0;
	int i = 0;

	
	PANEL_PRINTK("ali_pan_hwscan %s %d, data = %d %d %d %d\n",  
		__FUNCTION__, __LINE__, data[0], data[1], data[2], data[3] );	

	/* Search ESC command untill no-ESC or reached data limit */
	for (dp = 0; dp < limit_len && data[dp] == 27; dp += 4)
	{	
		/* LBD operate command */
		if (PAN_ESC_CMD_LBD == data[dp + 1] || 'l' == data[dp + 1])
		{
			gp = &hp->lbd[data[dp + 2]];
			if (hp->type_kb & 0x02)	/* LED PIN is controlled by shifter */
			{				
				for (i = 0; i < hp->num_com; i++)
				{
					addr = R_DOUT + (i << 1) + (data[dp + 2] << 1);
					tp->write8(addr, (tp->read8(addr) & ~(1 << gp->position)) |
						   ((~(gp->polar ^ data[dp + 3]) & 1) << gp->position));
				}
			} 
			else
			{
				if (data[dp + 3] == PAN_ESC_CMD_LBD_ON)
				{					
					tp->gpio_val_set(gp->position, gp->polar);
				}
				else if (data[dp + 3] == PAN_ESC_CMD_LBD_OFF)
				{					
					tp->gpio_val_set(gp->position, !gp->polar);
				}
			}
		} 
		else if (PAN_ESC_CMD_LED == data[dp + 1] || 'e' == data[dp + 1])
		{			
			tp->write8(R_DOUT + (data[dp + 2] << 1) + 1, data[dp + 3]);
		}
		#if ((UPGRADE_FORMAT & BOOT_UPG) == BOOT_UPG)
		else if (PAN_ESC_CMD_STANDBY_LED == data[dp + 1] || 'p' == data[dp + 1])
		{
			tp->gpio_val_set(TUNER_POWER_DOWN, data[dp + 3]);
		}
		#endif
	}

	return dp;
}

/*
* Name          :   pan_hwscan_char_map()
* Description   :   Do string display
* Parameter     :   unsigned short* data			: Command data
*                   unsigned long limit_len	: Length limit
* Return        :   unsigned long				: string length
*/
static unsigned long pan_hwscan_char_map(struct ali_pan_hwscan_private *tp, unsigned char *data, unsigned long len)
{
	struct pan_hw_info *hp = tp->hw_info;
	unsigned long pdata = 0, pbuff = 0;
	int j = 0;
	unsigned char dot_flag = 0;
	unsigned char bmp = 0;
	long led_num = 0;

	pdata = 0;
	pbuff = 0;
	led_num = tp->hw_info->num_com;
	if(hp->pos_colon < 8)
	{
		tp->write8(R_DOUT + (hp->pos_colon << 1) + 1, tp->blankmap);
	}
	/* Search all string sector in data untill ESC or pbuf full */
	while (pdata < len && data[pdata] != 27 && pbuff < led_num)
	{
		if (data[pdata] == ':')		/* Process colon charactor */
		{
			dot_flag = 1;
			pdata++;
			continue;
		}
		if (data[pdata] == '.')		/* Process dot charactor */
		{
			tp->write8(R_DOUT + (pbuff << 1) + 1, tp->dotmap);
			pdata++;
			pbuff++;
			continue;
		}
		/* Generate the bitmap */
		for (j = 0; j < tp->bitmap_len; j++)
		{
			if(data[pdata] == tp->bitmap_list[j].character)
			{
				if (data[pdata + 1] == '.' && (pdata + 1) < len)
				{
					unsigned long k, l = 0;
					for(k=0; k<8; k++)
					{
						if(tp->dotmap&(1<<k))
						{
							l++;
						}
					}
					if(l==7)
					{
						tp->write8(R_DOUT + (pbuff << 1) + 1, tp->bitmap_list[j].bitmap & tp->dotmap);
					}
					else
					{
						tp->write8(R_DOUT + (pbuff << 1) + 1, tp->bitmap_list[j].bitmap | tp->dotmap);
					}
				}
				else
				{
		            		bmp = tp->bitmap_list[j].bitmap;
					tp->write8(R_DOUT + (pbuff << 1) + 1, bmp);
				}
				break;
			}

		}
		/* Move to next position */
		pbuff++;
		if (data[pdata + 1] == '.' && (pdata + 1) < len)
		{
			pdata += 2;
		}
		else
		{
			pdata += 1;
		}
	}
	if (dot_flag == 1)
	{
		tp->write8(R_DOUT + (hp->pos_colon << 1) + 1, tp->read8(R_DOUT + (hp->pos_colon << 1) + 1) & tp->colonmap);
	}
	
	/* Display buffer full, stop display process */
	if (data[pdata] != 27 && pbuff == led_num)
	{
		return len;
	}
	
	return pdata;
}

/*
* Name          :   pan_hwscan_display()
* Description   :   Set display data
* Parameter     :   char* disp
* Return        :   void
*/
void pan_hwscan_display(struct ali_pan_hwscan_private *tp, char *data, unsigned long len)
{
	unsigned long pdata = 0;
	//unsigned long *x1 = 0;


	#if 0
	if (tp->hw_info->hook_show)
	{
		len = tp->hw_info->hook_show(tp, data, len);
	}
	#endif

	while (pdata < len)
	{
		if (data[pdata] == 27)			/* ESC command */
		{
			pdata += pan_hwscan_esc_command(tp, &(data[pdata]), len - pdata);
		}
		else							/* String display */
		{
			pdata += pan_hwscan_char_map(tp, &(data[pdata]), len - pdata);
		}
	}


	#if 0
	x1=(unsigned long *)HW_SCAN_REG_BASE;	
	{
		tp->write8(R_DOUT + 1, 0x99);
		tp->write8(R_DOUT + 3, 0x71);
		tp->write8(R_DOUT + 5, 0x1f);
		tp->write8(R_DOUT + 7, 0x91);
		PANEL_PRINTK("x1=%x, x1+1=%x, x1+2=%x, x1+3=%x\n", x1,x1+1,x1+2,x1+3);
		PANEL_PRINTK("string=%s B8018400=%x, B8018404=%x, B8018408=%x, B801840C=%x\n", data, *x1,*(x1+1), *(x1+2),*(x1+3));
		PANEL_PRINTK("string=%s B8018410=%x, B8018414=%x, B8018418=%x, B801841C=%x\n", data, *(x1+4),*(x1+5), *(x1+6),*(x1+7));
		PANEL_PRINTK("string=%s B8018420=%x, B8018424=%x, B8018428=%x, B801842C=%x\n", data, *(x1+8),*(x1+9), *(x1+10),*(x1+11));
		asm("sdbbp");
	}
	#endif
}

/////////////////////////////////////////////////////////////////////////////
// ali_pan_hwscan_open
//
// Description:
//	Init hardware scan driver module. 
//
// Arguments:
//	?
//	
// Return Value:
//	0 - Success
//	other - Error code from kernel_thread
//
/////////////////////////////////////////////////////////////////////////////

int ali_pan_hwscan_open(struct inode *inode, struct file *file)
{
	long ret = 0;
	long index = 0;	
	struct ali_pan_hwscan_private *tp = NULL;
	

	tp = kzalloc(sizeof(struct ali_pan_hwscan_private), GFP_KERNEL);
	if (!tp)
	{		
		PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}


	tp->ali_pan_hwscan_input = g_hwscan_input;
	tp->hw_info = &ali_pan_hw_info;
	tp->bitmap_len = PAN_HWSCAN_CHAR_LIST_NUM;
	tp->bitmap_list = &(ali_hwscan_bitmap_table[0]);	

	/* Set default bitmap in buffer. At same time get the dot bitmap. */
	for (index = 0; index < tp->bitmap_len; index++)
	{
		if (' ' == tp->bitmap_list[index].character)
		{
			tp->blankmap = tp->bitmap_list[index].bitmap;
		}
		else if ('.' == tp->bitmap_list[index].character)
		{
			tp->dotmap = tp->bitmap_list[index].bitmap;
		}
		else if (':' == tp->bitmap_list[index].character)
		{
			tp->colonmap = tp->bitmap_list[index].bitmap;
		}
	}
	/* Set register operation function handle */
	tp->is_local = 1;
	tp->read8 = read8_local;
	tp->read16 = read16_local;
	tp->write8 = write8_local;
	tp->write16 = write16_local;
	tp->gpio_dir_set = gpio_dir_set_local;
	tp->gpio_val_set = gpio_val_set_local;

	pan_hwscan_open(tp);
	
	file->private_data = tp;
	
	return ret;

}
/////////////////////////////////////////////////////////////////////////////
// ali_pan_hwscan_write
//
// Description:
//	Init hardware scan driver module. 
//
// Arguments:
//	?
//	
// Return Value:
//	0 - Success
//	other - Error code from kernel_thread
//
/////////////////////////////////////////////////////////////////////////////
static ssize_t ali_pan_hwscan_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *ppos)
{
	struct ali_pan_hwscan_private *tp = NULL;
	char *data = NULL;

	//PANEL_PRINTK("ali_pan_hwscan_write()=>start.\n");

	tp = file->private_data;
	
	data = (char *)kzalloc(count, GFP_KERNEL);
	if (!data)
	{
		PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}
	
	/* read list of keychords from userspace */
	if (copy_from_user(data, buffer, count)) 
	{
		kfree(data);
		PANEL_ERR_PRINTK("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		return -EFAULT;
	}

	
	//PANEL_PRINTK("ali_pan_hwscan %s %d, data=%s, count=%d\n", __FUNCTION__, __LINE__, data, count );

	pan_hwscan_display(tp, data, count);
	kfree(data);

	//PANEL_PRINTK("ali_pan_hwscan_write()=>end.\n");
	
	return 0;

}
/////////////////////////////////////////////////////////////////////////////
// ali_pan_hwscan_close
//
// Description:
//	Init hardware scan driver module. 
//
// Arguments:
//	?
//	
// Return Value:
//	0 - Success
//	other - Error code from kernel_thread
//
/////////////////////////////////////////////////////////////////////////////
static int ali_pan_hwscan_close(struct inode *inode, struct file *file)
{
	struct ali_pan_hwscan_private *tp = NULL;	

	
	PANEL_PRINTK("[ %s ], start.\n", __FUNCTION__);	

	tp = file->private_data;
	pan_hwscan_close(tp);
	kfree(tp);
	
	PANEL_PRINTK("[ %s ], end.\n", __FUNCTION__);	
	
	return 0;
}

static const struct file_operations ali_pan_hwscan_fops = {
	.owner		= THIS_MODULE,
	.open		= ali_pan_hwscan_open,
	.release		= ali_pan_hwscan_close,
	.write		= ali_pan_hwscan_write,
	#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)
	.unlocked_ioctl = pan_hwscan_ioctl,
	#else
	.ioctl			= pan_hwscan_ioctl,
	#endif
};

static struct miscdevice ali_pan_hwscan_misc = {
	.fops		= &ali_pan_hwscan_fops,
	.name		= HWSCAN_DEV_NAME,
	.minor		= MISC_DYNAMIC_MINOR,
};

/////////////////////////////////////////////////////////////////////////////
// ali_pan_hwscan_init
//
// Description:
//	Init hardware scan driver module. 
//
// Arguments:
//	?
//	
// Return Value:
//	0 - Success
//	other - Error code from kernel_thread
//
/////////////////////////////////////////////////////////////////////////////
static int __devinit ali_pan_hwscan_init(void)
{
	int ret = -1;	
	

	//printk("Welcome, ALi Panel Hardware Scan Driver ");

	g_hwscan_input = input_allocate_device();
	if (!g_hwscan_input)
	{
		ret = -ENOMEM;
		PANEL_ERR_PRINTK(KERN_ERR "ALi Panel HW Scan: not enough memory for input device\n");
		return ret;
	}

	g_hwscan_input->name = HWSCAN_DEV_INPUT_NAME;
	g_hwscan_input->phys = HWSCAN_DEV_INPUT_NAME;
	g_hwscan_input->id.bustype = BUS_HOST; 
	g_hwscan_input->id.vendor = 0x0001;
	g_hwscan_input->id.product = 0x0002;
	g_hwscan_input->id.version = 0x0100;
	
	__set_bit(EV_KEY, g_hwscan_input->evbit);
	__set_bit(EV_REP, g_hwscan_input->evbit);	

	ret = input_register_device(g_hwscan_input);
	if (ret)
	{
		input_free_device(g_hwscan_input);
		PANEL_ERR_PRINTK(KERN_ERR "ALi Panel HW Scan: input_register_device() failed!\n");
	}	

	ret = misc_register(&ali_pan_hwscan_misc);
	if (ret != 0) 
	{
		PANEL_ERR_PRINTK(KERN_ERR "ALi Panel HW Scan: cannot register miscdev(err=%d)\n", ret);
		return ret;
	}

	memset((void *)(&g_hwscan_key_map), 0, sizeof(g_hwscan_key_map));

	if (NULL != (proc_hwscan = create_proc_entry( "panel", 0, NULL )))
	{
		proc_hwscan->read_proc = hwscan_read_proc;
	}


	//printk("OK!\n");

	return ret;
}


/////////////////////////////////////////////////////////////////////////////
// ali_pan_hwscan_exit
//
// Description:
//	Cleanup hardware scan driver module. 
//
// Arguments:
//	?
//	
// Return Value:
//	0 - Success
//	other - Error code from kernel_thread
//
/////////////////////////////////////////////////////////////////////////////
static void __exit ali_pan_hwscan_exit(void)
{
	//printk("Goodbye, ALi Panel Hardware Scan Driver ");
	
	misc_deregister(&ali_pan_hwscan_misc);
	input_unregister_device(g_hwscan_input);
	input_free_device(g_hwscan_input);
	g_hwscan_input = NULL;	
	if(g_hwscan_key_map.map_entry)
	{
		kfree(g_hwscan_key_map.map_entry);
	}

	if (proc_hwscan)
	{
		remove_proc_entry( "panel", NULL);
	}

	
	//printk("OK!\n");
}


module_init(ali_pan_hwscan_init);
module_exit(ali_pan_hwscan_exit);

MODULE_AUTHOR("Martin Xia");
MODULE_DESCRIPTION("ALi STB Hardware Panel Scan Driver");
MODULE_LICENSE("Dual BSD/GPL");

