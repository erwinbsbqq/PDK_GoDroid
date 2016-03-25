

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/leds.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <ali_front_panel_common.h>
#include <linux/ali_transport.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include "ali_m36_pan_ch455.h"
#include "../aliir/ali_ir_g2.h"
#include <asm/mach-ali/m36_gpio.h>
#include <ali_reg.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <ali_front_panel_common.h>
#include <ali_board_config.h>


#define ALI_PANEL_INFO	"ALi Panel(CH455) Driver"

//0	[INTENS]	[7SEG]	[SLEEP]	0	[ENA]
//0	000		0		0		0	1
#define CH455_MODE	0x01

#define SETING_ADDR	0x48
static u8 g_dig0_addr = 0x68;
static u8 g_dig1_addr = 0x6e;	/* M3701C/H, 0x6a */
static u8 g_dig2_addr = 0x6c;
static u8 g_dig3_addr = 0x6a;	/* M3701C/H, 0x6e */
#define KEY_ADDR	0x4f

#define CH455_KEY_STATUS_MASK	0x40
#define CH455_KEY_ROW_MASK		0x38
#define CH455_KEY_COLUMN_MASK	0x03

#define CH455_STATUS_UP		0
#define CH455_STATUS_DOWN		1

/* ESC command: 27 (ESC code), PAN_ESC_CMD_xx (CMD type), param1, param2 */
#define PAN_ESC_CMD_LBD			'L'		/* LBD operate command */
#define PAN_ESC_CMD_LBD_FUNCA	0		/* Extend function LBD A */
#define PAN_ESC_CMD_LBD_FUNCB	1		/* Extend function LBD B */
#define PAN_ESC_CMD_LBD_FUNCC	2		/* Extend function LBD C */
#define PAN_ESC_CMD_LBD_FUNCD	3		/* Extend function LBD D */
#define PAN_ESC_CMD_LBD_LEVEL		5		/* Level status LBD, no used */

#define PAN_ESC_CMD_LED			'E'		/* LED operate command */
#define PAN_ESC_CMD_LED_LEVEL		0		/* Level status LED */

#define PAN_ESC_CMD_LBD_ON		1		/* Set LBD to turn on status */
#define PAN_ESC_CMD_LBD_OFF		0		/* Set LBD to turn off status */

#define PAN_MAX_LED_NUM			4

#define PAN_MAX_CHAR_LIST_NUM	70


#define REPEAT_DELAY			300		/* ms */
#define REPEAT_INTERVAL			250		/* ms */
#define TIMER_DELAY				100		/* ms */
#ifdef SUCCESS
	#undef SUCCESS
	#define SUCCESS 0
#else
	#define SUCCESS 0
#endif

#ifdef ERR_FAILURE
	#undef ERR_FAILURE
	#define ERR_FAILURE -1
#else
	#define ERR_FAILURE -1
#endif

#ifdef ARRAY_SIZE
	#undef ARRAY_SIZE
	#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#else
	#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

struct led_bitmap
{
	u8 character;
	u8 bitmap;
};

enum value_key_press
{
	KEY_RELEASE		= 0,
	KEY_PRESSED		= 1,
	KEY_REPEAT		= 2 
};

#define PAN_KEY_INVALID			0xFFFFFFFF

static struct led_bitmap bitmap_table[PAN_MAX_CHAR_LIST_NUM * 2] =
{
	{'.', 0x80}, /* Let's put the dot bitmap into the table */
	{'0', 0x3f}, {'1', 0x06}, {'2', 0x5b}, {'3', 0x4f}, 
	{'4', 0x66}, {'5', 0x6d}, {'6', 0x7d}, {'7', 0x07}, 
	{'8', 0x7f}, {'9', 0x6f}, {'a', 0x5f}, {'A', 0x77}, 
	{'b', 0x7c}, {'B', 0x7c}, {'c', 0x39}, {'C', 0x39}, 
	{'d', 0x5e}, {'D', 0x5e}, {'e', 0x79}, {'E', 0x79}, 
	{'f', 0x71}, {'F', 0x71}, {'g', 0x6f}, {'G', 0x3d}, 
	{'h', 0x76}, {'H', 0x76}, {'i', 0x04}, {'I', 0x30}, 
	{'j', 0x0e}, {'J', 0x0e}, {'l', 0x38}, {'L', 0x38}, 
	{'n', 0x54}, {'N', 0x37}, {'o', 0x5c}, {'O', 0x3f}, 
	{'p', 0x73}, {'P', 0x73}, {'q', 0x67}, {'Q', 0x67}, 
	{'r', 0x50}, {'R', 0x77}, {'s', 0x6d}, {'S', 0x6d}, 
	{'t', 0x78}, {'T', 0x31}, {'u', 0x3e}, {'U', 0x3e}, 
	{'y', 0x6e}, {'Y', 0x6e}, {'z', 0x5b}, {'Z', 0x5b}, 
	{':', 0x80}, {'-', 0x40}, {'_', 0x08}, {' ', 0x00},
};

#define PAN_CH455_CHAR_LIST_NUM sizeof(bitmap_table)/sizeof(struct led_bitmap)
static u8 g_display_backup[PAN_MAX_LED_NUM][PAN_MAX_LED_NUM] = 
{
	{0, 0}, 
	{0, 0}, 
	{0, 0}, 
	{0, 0}, 
};

/*
ali_ch455_key_map.code :
row       : bit0-bit2
column : bit3-bit4
*/
/*
static const struct ali_fp_key_map_t ali_ch455_key_map[] =
{
	{0, KEY_MENU},		
	{1, KEY_RIGHT},		
	{2, KEY_DOWN},
	{3, KEY_ENTER},
	{4, KEY_LEFT},		
	{5, KEY_UP},
	{6, KEY_RESERVED},
	{7, KEY_RESERVED},
};
*/
static const struct ali_fp_key_map_t ali_linux_pan_key_map[] =
{
	{(PAN_ALI_HKEY_UP), 		KEY_UP},
	{(PAN_ALI_HKEY_LEFT),		KEY_LEFT},
	{(PAN_ALI_HKEY_ENTER),		KEY_ENTER},
	{(PAN_ALI_HKEY_RIGHT),		KEY_RIGHT},
	{(PAN_ALI_HKEY_DOWN),		KEY_DOWN},	
	{(PAN_ALI_HKEY_MENU),		KEY_MENU},		
};

static struct ali_fp_key_map_cfg g_ch455_key_map;



static struct input_dev *ch455_input;

struct ch455_device {
	struct timer_list timer;
	struct mutex lock;
	u32	i2c_id;
	u8 gpio_i2c;
	u8	mode;	

	struct task_struct *thread_id;
	int run;
	struct led_bitmap	*bitmap_list;
	u32 bitmap_len;

	u8	mask_status;			/* key status bit mask */
	u8	mask_row;			/* key code row bit mask */
	u8	mask_column;		/* key code column bit mask */
	
	u32	key_cnt;	
	u32	keypress_cnt;
	u32	keypress_intv;		/* Continue press key interval */
	u32	keypress_bak;		/* Pressed key saver */	
	u8	bak_status;

	u8 	lbd_func_flag;

	u32	(*read8)(struct ch455_device *pch455,  u8 dig_addr, u8 *data);
	u32	(*write8)(struct ch455_device *pch455, u8 dig_addr, u8 data);
    u8	cur_bitmap[PAN_MAX_LED_NUM];
};


static unsigned long pan_key_received_count = 0;
static unsigned long pan_key_mapped_count = 0;
static unsigned char pan_debug = 0;
static unsigned long ch455_repeat_delay_rfk = 600;
static unsigned long ch455_repeat_interval_rfk = 350;
static unsigned long ch455_repeat_width_rfk = 0;
static unsigned int ch455_rfk_port = 0;
static struct proc_dir_entry *proc_ch455;
/* 	ch455_i2c_info :
	byte0, I2C ID; 
	byte1, 1 gpio_i2c, 0 i2c 
	byte2, reserve; 
	byte3, reserve; 
*/
static unsigned int  ch455_i2c_info = 0;


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


int ali_i2c_scb_write(u32 id, u8 slv_addr, u8 *data, int len);
int ali_i2c_scb_read(u32 id, u8 slv_addr, u8 *data, int len);
static int  ch455_mode_set(struct ch455_device *pch455);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
#include <linux/seq_file.h>
static int ch455_read_proc(struct seq_file *m, void *v)
{
	seq_printf(m, "%s", CH455_DEV_NAME);
	return 0;
}

static int proc_panel_open(struct inode *inode, struct file *file)
{
	return single_open_size(file, ch455_read_proc, PDE_DATA(inode), 256);
}


static const struct file_operations proc_panel_fops = {
	.open		= proc_panel_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#else
static int ch455_read_proc (char *buffer, char **start, off_t off, int count,
			  int *eof, void *data_unused)
{
	int len = 0;    

	
	if (off > 0)
	{
		return 0;
	}
	len = sprintf(buffer, "%s", CH455_DEV_NAME);
	*start = buffer;	
	*eof = 1;

        	return len;
}
#endif

static int ali_ch455_send_msg(unsigned long  code, int ir_protocol, int ir_state)
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
	
	PANEL_PRINTK("[ %s %d ], rfk port %d send msg :\n", __FUNCTION__, __LINE__, ch455_rfk_port);
	for (i=0; i<sizeof(msg); i++)
	{
		PANEL_PRINTK("%02x ", msg[i]);
	}
	PANEL_PRINTK("\n");	
	
	ret = ali_transport_send_msg(ch455_rfk_port, &msg, sizeof(msg));
	if (-1 == ret)
	{
		PANEL_ERR_PRINTK("[ %s %d ], rfk port %d send msg fail!\n", __FUNCTION__, __LINE__, ch455_rfk_port);
	}

	
	return ret;
}


#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)
static long ch455_ioctl(struct file * file, unsigned int cmd, unsigned long param)
#else
static int ch455_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long param)
#endif
{
	unsigned long reg[2] = {0, 0};	
	struct ch455_device *pch455 = file->private_data;	
	
	
	PANEL_PRINTK("[ %s ]: cmd %d\n", __FUNCTION__, cmd);	
	switch (cmd)
	{
		case PAN_DRIVER_READ_REG: 			
			if (0 != (copy_from_user(&reg, (void*)param, sizeof(reg))))
			{
				PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}			
			
			reg[1] = __REG32ALI(reg[0]);				
			if (0 != copy_to_user((void*)param, &reg, sizeof(reg)))
			{
				PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
        				return -EFAULT;
			}
			
			PANEL_PRINTK("[ %s %d ]: read 0x%08x = 0x%08x\n\n", 
				__FUNCTION__,  __LINE__, (unsigned int)reg[0], (unsigned int)reg[1]);					
			
			break;		

		case PAN_DRIVER_WRITE_REG: 				
			if (0 != (copy_from_user(&reg, (void*)param, sizeof(reg))))
			{
				PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}					
			
			__REG32ALI(reg[0]) = reg[1];					
			PANEL_PRINTK("[ %s %d ]: write 0x%08x = 0x%08x\n", 
				__FUNCTION__,  __LINE__, (unsigned int)reg[0], (unsigned int)(__REG32ALI(reg[0])));							
			
			break;
			
		case ALI_FP_CONFIG_KEY_MAP:
		{
			unsigned char * map_entry;
			struct ali_fp_key_map_cfg key_map;
			int i;
			
			if (0 != (copy_from_user(&key_map, (void*)param, sizeof(key_map))))
			{
				PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}			
			
			PANEL_PRINTK("[ %s %d ], phy_code = %ld\n", __FUNCTION__, __LINE__, key_map.phy_code);			
			
			g_ch455_key_map.phy_code = key_map.phy_code;
			if (2 != g_ch455_key_map.phy_code)
			{
				map_entry = kzalloc(key_map.map_len, GFP_KERNEL);
				if(NULL==map_entry)
				{
					PANEL_ERR_PRINTK("[ %s ]: Fail, not enouth memory!\n", __FUNCTION__);
					return -ENOMEM;				
				}
				
				if (0 != (copy_from_user(map_entry, key_map.map_entry, key_map.map_len)))
				{
					PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
					return -EFAULT;
				}				
				key_map.map_entry = map_entry;
				key_map.unit_num = (key_map.map_len/key_map.unit_len);
				if(key_map.unit_num>KEY_CNT)
				{
					key_map.unit_num = KEY_CNT;
				}
				if(g_ch455_key_map.map_entry)
				{
					kfree(g_ch455_key_map.map_entry);
				}
				g_ch455_key_map.map_entry = NULL;
				
				for (i = 0; i < key_map.unit_num; ++i)
				{
					unsigned short key;

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
					
					__set_bit(key%KEY_CNT, ch455_input->keybit);
				}
				
				g_ch455_key_map.map_len = key_map.map_len;
				g_ch455_key_map.unit_len = key_map.unit_len;
				g_ch455_key_map.phy_code = key_map.phy_code;
				g_ch455_key_map.unit_num = key_map.unit_num;
				g_ch455_key_map.map_entry = key_map.map_entry;
				if (0 != pan_debug)
				{
					int i;
					PANEL_PRINTK("\n[ %s ]: \n", __FUNCTION__);
					for(i=0; i<g_ch455_key_map.unit_num; i++){
						unsigned char * buf = &map_entry[i*g_ch455_key_map.unit_len];
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
			if (0 != copy_to_user((void*)param, &pan_key_received_count, sizeof(pan_key_received_count)))
			{
				PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
        				return -EFAULT;
			}	
			
			PANEL_PRINTK("[ %s ]: pan_key_received_count = %ld\n", __FUNCTION__, pan_key_received_count);	
		}
		break;

		case ALI_FP_GET_PAN_KEY_MAPPED_COUNT:
		{									
			if (0 != copy_to_user((void*)param, &pan_key_mapped_count, sizeof(pan_key_mapped_count)))
			{
				PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
        				return -EFAULT;
			}
			
			PANEL_PRINTK("[ %s ]: pan_key_mapped_count = %ld\n", __FUNCTION__, pan_key_mapped_count);	
		}
		break;

		case ALI_FP_SET_PAN_KEY_DEBUG:
		{					
			if (0 != copy_from_user(&pan_debug, (void*)param, sizeof(pan_debug)))			
			{
				PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
        				return -EFAULT;
			}	
			
			PANEL_PRINTK("[ %s ]: pan_debug = %d\n", __FUNCTION__, pan_debug);			
		}
		break;		

		case ALI_FP_SET_SOCKPORT:
		{				
			if (0 != (copy_from_user(&ch455_rfk_port, (void*)param, sizeof(ch455_rfk_port))))
			{
				PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}	
			
			PANEL_PRINTK("[ %s ]: ch455_rfk_port = %d\n", __FUNCTION__, ch455_rfk_port);				
		}
		break;	

		case PAN_DRIVER_SET_I2C:
		{				
			if (0 != (copy_from_user(&ch455_i2c_info, (void*)param, sizeof(ch455_i2c_info))))
			{
				PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}						
			
			pch455->i2c_id = ch455_i2c_info & 0xff;
			pch455->gpio_i2c = (u8)((ch455_i2c_info & 0xff00) >> 8);
			
			PANEL_PRINTK("[ %s %d ]: ch455_i2c_info = 0x%08x, i2c_id = %d, gpio_i2c = %d\n", 
				__FUNCTION__, __LINE__, ch455_i2c_info, pch455->i2c_id, pch455->gpio_i2c);	
	
			ch455_mode_set(pch455);
			PANEL_PRINTK("[ %s %d ]: i2c_id = %d\n", __FUNCTION__, __LINE__, pch455->i2c_id);	
		}
		break;	

		case PAN_DRIVER_SET_DIG_ADDR:
		{		
			unsigned int dig_addr = 0;
			
			if (0 != (copy_from_user(&dig_addr, (void*)param, sizeof(dig_addr))))
			{
				PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}						
			
			g_dig0_addr= dig_addr & 0xff;
			g_dig1_addr = (u8)((dig_addr & 0xff00) >> 8);
			g_dig2_addr = (u8)((dig_addr & 0xff0000) >> 16);
			g_dig3_addr = (u8)((dig_addr & 0xff000000) >> 24);

			g_display_backup[0][0] = g_dig0_addr;
			g_display_backup[1][0] = g_dig1_addr;
			g_display_backup[2][0] = g_dig2_addr;
			g_display_backup[3][0] = g_dig3_addr;
			
			PANEL_PRINTK("[ %s %d ]: dig_addr = 0x%08x[0x%02x 0x%02x 0x%02x 0x%02x]\n", 
				__FUNCTION__, __LINE__, dig_addr, g_dig0_addr, g_dig1_addr,
				g_dig2_addr, g_dig3_addr);					
		}
		break;	

		case PAN_DRIVER_SET_LED_BITMAP:
		{
			struct ali_fp_bit_map_cfg bit_map;
			u32 i;
			
			if (0 != (copy_from_user(&bit_map, (void*)param, sizeof(bit_map))))
			{
				PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}			
			
			PANEL_PRINTK("[ %s %d ], map_len = %ld\n", __FUNCTION__, __LINE__, bit_map.map_len);
			if (bit_map.map_len > (PAN_MAX_CHAR_LIST_NUM * 2))
			{
				pch455->bitmap_len = PAN_MAX_CHAR_LIST_NUM * 2;
				PANEL_PRINTK("[ %s %d ], map_len > %d\n", __FUNCTION__, __LINE__, (PAN_MAX_CHAR_LIST_NUM * 2));
			}
			else
			{
				pch455->bitmap_len = bit_map.map_len;
			}
			PANEL_PRINTK("[ %s %d ], bitmap_len = %d\n", __FUNCTION__, __LINE__, pch455->bitmap_len);

			memset(pch455->bitmap_list, 0x00, sizeof(bitmap_table));
			if (0 != (copy_from_user(pch455->bitmap_list, bit_map.map_entry, pch455->bitmap_len)))
			{
				PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}				
			
			if (0 != pan_debug)
			{
				for (i=0; i<pch455->bitmap_len; i++)
				{
					PANEL_PRINTK("'%c'    %02x\n", 						
						pch455->bitmap_list[i].character,pch455->bitmap_list[i].bitmap);					
				}
				PANEL_PRINTK("\n");
			}				
		}
		break;	

		case ALI_FP_SET_REPEAT_INTERVAL:
		{				
			unsigned long rep[2] = {0, 0};
			
			if (0 != (copy_from_user(&rep, (void*)param, sizeof(rep))))
			{
				PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}

			ch455_repeat_delay_rfk = rep[0];
			ch455_repeat_interval_rfk = rep[1];			
			
			PANEL_PRINTK("[ %s ]: ch455_repeat_delay_rfk = %ld\n", __FUNCTION__, ch455_repeat_delay_rfk);	
			PANEL_PRINTK("[ %s ]: ch455_repeat_interval_rfk = %ld\n", __FUNCTION__, ch455_repeat_interval_rfk);				
		}
		break;		
			
		default:
			return -EPERM;
			break;
	}

	return 0;
}


static u32 read8(struct ch455_device *pch455,  u8 dig_addr, u8 *data)
{	
	u32 re = (u32)ERR_FAILURE;		
	unsigned char offset = dig_addr;
	struct i2c_adapter *adapter;
    	struct i2c_msg msgs[] = {   { .addr = dig_addr>>1, .flags = 0, .len = 0, .buf = &offset },
                                { .addr = dig_addr>>1, .flags	= I2C_M_RD,  .len = 1,  .buf = data    } };
	int result = 0;	
	
	mutex_lock(&pch455->lock);

	if (1 == pch455->gpio_i2c)
	{
		adapter = i2c_get_adapter(pch455->i2c_id);	
	    	if(adapter)
	    	{				
	        	if((result = i2c_transfer(adapter, msgs, 2)) != 2)
	        	{
		         PANEL_PRINTK("[ %s %d ], i2c_transfer fail, result = %d, name = %s, id = %d, nr = %d, class = 0x%08x\n", 
				 	__FUNCTION__, __LINE__, result, adapter->name, adapter->nr, adapter->nr, adapter->class);
		         re = (u32)ERR_FAILURE;
	        	}    
			else
			{
				re = (u32)SUCCESS;
			}	
	    	}  
		else
		{
			PANEL_PRINTK("[ %s %d ], adapter is NULL, i2c_id = %d\n", __FUNCTION__, __LINE__, pch455->i2c_id);
		}	
	}
	else
	{
		re = (u32)ali_i2c_scb_read(pch455->i2c_id, dig_addr, data, 1);
	}
	

	mutex_unlock(&pch455->lock);

	//PANEL_PRINTK("[ %s ], id(%d), addr(0x%02x), data(0x%02x).\n", __FUNCTION__, pch455->i2c_id, dig_addr, *data);

	return re;
}

static u32 write8(struct ch455_device *pch455, u8 dig_addr, u8 data)
{
	u32 re = (u32)ERR_FAILURE;	
	unsigned char buf[10];
    	struct i2c_adapter *adapter;
    	struct i2c_msg msgs = { .addr = dig_addr>>1, .flags = 0, .len = 1,  .buf = buf };
	int result = 0;

	
	
	//PANEL_PRINTK("[ %s ], id = %d, addr = 0x%02x, data = 0x%02x.\n", __FUNCTION__, pch455->i2c_id, dig_addr, data);

	mutex_lock(&pch455->lock);

	if (1 == pch455->gpio_i2c)
	{
		buf[0] = data;	
	    	adapter = i2c_get_adapter(pch455->i2c_id);		
	    	if(adapter)
	    	{						
		        	if((result = i2c_transfer(adapter, &msgs, 1)) != 1)
		        	{
		         		PANEL_PRINTK("[ %s %d ], i2c_transfer fail, result = %d, name = %s, id = %d, nr = %d, class = 0x%08x\n", 
					 	__FUNCTION__, __LINE__, result, adapter->name, adapter->nr, adapter->nr, adapter->class);
		            	re = (u32)ERR_FAILURE;
		        	}    
			else
			{
				re = (u32)SUCCESS;
			}	
		}
		else
		{
			PANEL_PRINTK("[ %s %d ], adapter is NULL, i2c_id = %d\n", __FUNCTION__, __LINE__, pch455->i2c_id);
		}
	}
	else
	{
		re = (u32)ali_i2c_scb_write(pch455->i2c_id, dig_addr, &data, 1);
	}

	mutex_unlock(&pch455->lock);

	return re;
}


static int  ch455_mode_set(struct ch455_device *pch455) 
{
	int re = ERR_FAILURE;

	if(SUCCESS != (re=pch455->write8(pch455, SETING_ADDR, pch455->mode)))
	{
		PANEL_PRINTK("[ %s ], Failed, re = %d\n", __FUNCTION__, re);
	}	

	return re;
}


static int ch455_key_default_mapping (unsigned long ir_code)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(ali_linux_pan_key_map); i++) {
		if (ali_linux_pan_key_map[i].code== ir_code) 
		{
			return ali_linux_pan_key_map[i].key;
		}
	}

	return -1;
}



static int ch455_key_mapping(unsigned long phy_code)
{
	int i;
	int logic_code = 0;

	
	if ((NULL==g_ch455_key_map.map_entry) || (3 == g_ch455_key_map.phy_code))
	{		
		return ch455_key_default_mapping(phy_code);		
	}
	
	for(i = 0; i<g_ch455_key_map.unit_num; i++)
	{
		unsigned long my_code;
		
		
		if((unsigned long)(&g_ch455_key_map.map_entry[i*g_ch455_key_map.unit_len]) & 0x3)
		{
			my_code = g_ch455_key_map.map_entry[i*g_ch455_key_map.unit_len];			
			my_code |= (g_ch455_key_map.map_entry[i*g_ch455_key_map.unit_len+1])<<8;			
			my_code |= (g_ch455_key_map.map_entry[i*g_ch455_key_map.unit_len+2])<<16;			
			my_code |= (g_ch455_key_map.map_entry[i*g_ch455_key_map.unit_len+3])<<24;			
		}
		else
		{
			my_code = *((unsigned long *)(&g_ch455_key_map.map_entry[i*g_ch455_key_map.unit_len]));
		}
		
		if(phy_code == my_code)
		{			
			if(1 == g_ch455_key_map.phy_code)
			{				
				PANEL_PRINTK("[ %s ], phy code index %d\n", __FUNCTION__, i);				
				return i;
			}
			else
			{
				logic_code =  g_ch455_key_map.map_entry[i*g_ch455_key_map.unit_len+4]; 				
				logic_code |= (g_ch455_key_map.map_entry[i*g_ch455_key_map.unit_len+5])<<8;									
				PANEL_PRINTK("logic code 0x%08x\n", logic_code);				
				
				return logic_code;
			}
		}			
	}

	
	return -1;
}


int ch455_timer(void *param)
{
	struct ch455_device *pch455 = (struct ch455_device *)param;
	u8 row, column, status;	
	u8 data = 0xff;	
	u32 re = SUCCESS;
	s32 mode_flag = 0;
	s32 key_index = 0;
	unsigned long code = 0;	
	static unsigned long last_repeat_tick = 0;	
	unsigned long last_repeat_width;
	unsigned long repeat_tick = 0;
	u8 i = 0;	
	u8 count = 0;	
	u8 count_fail = 0;
	u8 sleep_flag = 0;


	while (!kthread_should_stop())
	{			
		re = pch455->read8(pch455,  KEY_ADDR, &data);		/* about 1ms */
		if(SUCCESS != re)
		{
			mode_flag = 1;
			PANEL_PRINTK("CH455: Scan keyboard failed!re = %d.\n", re);
			goto ch455_sleep;
		}
		else
		{	
			//bit 7 should always be 0, bit 2 should always be 1.
			if(((data & 0x80) != 0) || ((data & 0x04) == 0))
			{
				//PANEL_PRINTK("%s()=>Read bad key code!data = 0x%2x.\n", __FUNCTION__, data);
				goto ch455_sleep;
			}
			else
			{
				/* bit7 = 0, bit2 = 1 */
				column = data & pch455->mask_column;		//(bit 0~1)
				row = (data & pch455->mask_row) >> 3;	//(bit 3~5)
				status = (data & pch455->mask_status) >> 6;  // bit 6		
				code = 0xFFFF0000 | (unsigned long)((column << 3) | row);
			}		
		}
	
	
		if (pch455->bak_status == CH455_STATUS_UP)	// up : 0; down : 1.
		{		
			if (status == CH455_STATUS_UP)
			{
				goto ch455_sleep;
			}
			/* step 1 */
			pan_key_received_count++;

			if(2 != g_ch455_key_map.phy_code)
			{
				key_index = ch455_key_mapping(code);
				if (key_index >= 0)
				{
					pan_key_mapped_count++;
					input_report_key(ch455_input, key_index, KEY_PRESSED);
					input_sync(ch455_input);						
									
					PANEL_PRINTK("[ %s %d ], key 0x%08x(0x%08x) pressed.\n", __FUNCTION__, __LINE__, 
						key_index, (unsigned int)code);
								
				}
				else
				{
					PANEL_ERR_PRINTK("[ %s %d ] : Not matched key for panel code = 0x%08x \n", 
						__FUNCTION__, __LINE__, (unsigned int)code);				
				}
			}
			else
			{					
				last_repeat_tick = jiffies;
				ali_ch455_send_msg(code, IR_TYPE_UNDEFINE, KEY_PRESSED);
			}
			
			pch455->key_cnt ++;
			pch455->bak_status = status;
			pch455->keypress_cnt = 0;
		}
		else			//pch455->bak_status == CH455_STATUS_DOWN
		{		 
			if (status == CH455_STATUS_UP)
			{	
				/* step 2 */
				pch455->key_cnt = 1;
				pch455->bak_status = status;		

				if(2 != g_ch455_key_map.phy_code)
				{
					key_index = ch455_key_mapping(code);
					if (key_index >= 0)
					{						
						input_report_key(ch455_input, key_index, KEY_RELEASE);
						input_sync(ch455_input);								
						
						PANEL_PRINTK("[ %s %d ], key 0x%08x(0x%08x) pressed.\n", __FUNCTION__, __LINE__, 
							key_index, (unsigned int)code);					
										
					}	
					else
					{
						PANEL_ERR_PRINTK("[ %s %d ] : Not matched key for panel code = 0x%08x \n", 
							__FUNCTION__, __LINE__, (unsigned int)code);				
					}	
				}
				else
				{
					ch455_repeat_width_rfk = ch455_repeat_delay_rfk;
					ali_ch455_send_msg(code, IR_TYPE_UNDEFINE, KEY_RELEASE);	
				}			
			}
			else
			{	
				/* step 2, repeat */		
				if(2 == g_ch455_key_map.phy_code)
				{				
					repeat_tick = jiffies;					
					last_repeat_width = (unsigned long)(((long)repeat_tick - (long)last_repeat_tick));							
					if (last_repeat_width > ch455_repeat_width_rfk)
					{		
						last_repeat_tick = repeat_tick;
						ch455_repeat_width_rfk = ch455_repeat_interval_rfk;
						ali_ch455_send_msg(code, IR_TYPE_UNDEFINE, KEY_REPEAT);							
					}
				}
			}
		}
			
		ch455_sleep:
			
		if ((SUCCESS == re) && (1 == mode_flag))
		{
			ch455_mode_set(pch455);		
			mode_flag = 0;
			
			for (i=0; i<PAN_MAX_LED_NUM; i++)
			{
				if (SUCCESS != (re = pch455->write8(pch455, g_display_backup[i][0], g_display_backup[i][1])))
				{
					PANEL_ERR_PRINTK("[ %s %d ], Display LED failed. re = %d.\n", __FUNCTION__, __LINE__, re);					
				}
			}
		}		

		
		if (SUCCESS == re)						/* patch for FD650 */
		{
			sleep_flag = 0;
			
			count++;
			if (count >= (3000/TIMER_DELAY))	/* at least 3s */ 	
			{
				count = 0;
				
				ch455_mode_set(pch455);					
				for (i=0; i<PAN_MAX_LED_NUM; i++)
				{
					if (SUCCESS != (re = pch455->write8(pch455, g_display_backup[i][0], g_display_backup[i][1])))
					{
						PANEL_PRINTK("[ %s %d ], Display LED failed. re = %d.\n", __FUNCTION__, __LINE__, re);					
					}
				}			
			}	
		}
		else
		{
			if (1 == sleep_flag)
			{					
				ssleep(5);			/* sleep here to reduce cpu usage */
			}
			else
			{
				count_fail++;				
				if (count_fail >= 2)
				{
					count_fail = 0;
					sleep_flag = 1;							
				}
			}			
		}				
		
		msleep(TIMER_DELAY);		
	}


	return 0;
}


static int ch455_start_timer(struct ch455_device *pch455)
{	
	pch455->thread_id = kthread_create(ch455_timer, (void *)pch455, "ali_ch455");
	if(IS_ERR(pch455->thread_id)){
		PANEL_ERR_PRINTK("ch455 kthread create fail\n");
		pch455->thread_id = NULL;
		return -1;
	}
	wake_up_process(pch455->thread_id);
	
	return 0;
}

static u8 ch455_bitmap(struct ch455_device *pch455, u8 c)
{
	u32 len = pch455->bitmap_len;
	struct led_bitmap node;
	u8 bitmap = 0;
	u32 i = 0;

	for (i=0; i<len; i++)
	{
		node = pch455->bitmap_list[i];
		if (node.character == c)
		{
			bitmap = (u8)node.bitmap;
			break;
		}
	}

	if(i == len)
	{
		PANEL_ERR_PRINTK("ch455_bitmap()=>Character not found.\n");
		return 0;
	}

	return bitmap;	
}


static ssize_t ch455_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *ppos)
{
	struct ch455_device *pch455 = file->private_data;
	u8 pdata = 0;
	u8 bitmap, temp;
	u32 re;
	u8 flag=0;
	char *data;
	u8 addr[4]={g_dig0_addr, g_dig1_addr, g_dig2_addr, g_dig3_addr};
	s32 i = 0, num = -1;
	
	
	data = (char *)kzalloc(count, GFP_KERNEL);
	if (!data)
	{
		PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}

	/* read list of keychords from userspace */
	if (copy_from_user(data, buffer, count)) {
		kfree(data);
		PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
		return -EFAULT;
	}

	if (data[pdata] == 27)			/* ESC command */
	{
		switch(data[pdata + 1])
		{
			case PAN_ESC_CMD_LBD:
				temp = data[pdata + 2];
				if(temp == PAN_ESC_CMD_LBD_FUNCA
					||temp == PAN_ESC_CMD_LBD_FUNCB
					||temp == PAN_ESC_CMD_LBD_FUNCC
					||temp == PAN_ESC_CMD_LBD_FUNCD)
				{
					u8 update_led = 0;
					if(data[pdata + 3] == PAN_ESC_CMD_LBD_ON)
					{
						update_led = (pch455->lbd_func_flag & (1<<(temp-PAN_ESC_CMD_LBD_FUNCA))) ? 0:1;
						pch455->lbd_func_flag |= 1<<(temp-PAN_ESC_CMD_LBD_FUNCA);
					}
					else
					{
						update_led = (pch455->lbd_func_flag & (1<<(temp-PAN_ESC_CMD_LBD_FUNCA))) ? 1:0;
						pch455->lbd_func_flag &= ~(1<<(temp-PAN_ESC_CMD_LBD_FUNCA));
					}
                    
					if(update_led)
					{
						u8 da=pch455->cur_bitmap[temp];
						if(data[pdata + 3] == PAN_ESC_CMD_LBD_ON)
						{
							da |= 0x80; 
						}
						
						if (SUCCESS != (re = pch455->write8(pch455, addr[temp], da)))
						{
								kfree(data);
								PANEL_ERR_PRINTK("ch455_write()=>Display LED failed. re = %d.\n", re);
								return -EFAULT;
						}
						g_display_backup[temp-PAN_ESC_CMD_LBD_FUNCA][1] = da;
					}
				}
				break;

			default:
				break;
		}
	}
	else
	{
		if(count>PAN_MAX_LED_NUM)
		{			
			for(i=0;i<count;i++)
			{
				if(data[i] == ':' || data[i] == '.')
				{
					if(num>=0)
					{
						flag |= 1<<num;
					}
				}
				else
				{
					num++;
					data[num]=data[i];
				}
			}
			count = PAN_MAX_LED_NUM;
		}
		while (pdata < count)
		{
			temp = data[pdata];
			bitmap = ch455_bitmap(pch455, temp);
			pch455->cur_bitmap[pdata] = bitmap;

			if(flag & (1<<pdata))
				bitmap |= 0x80;
			else
				bitmap |= (pch455->lbd_func_flag & (1<<pdata)) ? 0x80 : 0;
			

			if (SUCCESS != (re = pch455->write8(pch455, addr[pdata], bitmap)))
			{
				kfree(data);
				PANEL_ERR_PRINTK("[ %s %d ], Display LED failed. re = %d.\n", __FUNCTION__, __LINE__, re);
				return -EFAULT;
			}
			
			g_display_backup[pdata][1] = bitmap;
			pdata ++;
		}
	}	
	

	kfree(data);
	return 0;
}

static int ch455_open(struct inode *inode, struct file *file)
{
	struct ch455_device *pch455;
	int i = 0;
	int ret = SUCCESS;

	
	pch455 = kzalloc(sizeof(struct ch455_device), GFP_KERNEL);
	if (!pch455)
	{
		PANEL_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}
	
	//Set i2c type id.
	ch455_i2c_info = 0x00000100 | g_gpio_i2c_panel_id; // use gpio i2c default.
	pch455->i2c_id = ch455_i2c_info & 0xff; 
	pch455->gpio_i2c = (u8)((ch455_i2c_info & 0xff00) >> 8);
	//Config CH455 working mode.
	pch455->mode = CH455_MODE;
	
	//Set bit mask for keyboard scan.
	pch455->mask_status = CH455_KEY_STATUS_MASK;
	pch455->mask_row = CH455_KEY_ROW_MASK;
	pch455->mask_column = CH455_KEY_COLUMN_MASK;

	pch455->key_cnt = 1;
	pch455->keypress_cnt = 0;
	
	//Set repeat key interval to 300 ms.
	pch455->keypress_intv = 3;
	
	pch455->keypress_bak = PAN_KEY_INVALID;
	
	//Set back status to up.
 	pch455->bak_status = CH455_STATUS_UP; 
	
	pch455->bitmap_len = PAN_CH455_CHAR_LIST_NUM;
	pch455->bitmap_list = &(bitmap_table[0]);
	
	memset(&g_ch455_key_map, 0x00, sizeof(g_ch455_key_map));
	g_ch455_key_map.phy_code = 3;	/* use default key map*/
	
	for (i = 0; i < ARRAY_SIZE(ali_linux_pan_key_map); i++) 
	{			
		__set_bit(ali_linux_pan_key_map[i].key%KEY_CNT, ch455_input->keybit);
	}	
	
		pch455->read8 = read8;
		pch455->write8 = write8;
	mutex_init(&pch455->lock);
	
	ch455_mode_set(pch455);		
	file->private_data = pch455;
	ch455_start_timer(pch455);	
    for(i=0; i<PAN_MAX_LED_NUM; i++)
    {
		pch455->cur_bitmap[i] = ch455_bitmap(pch455, ' ');
	}

	g_display_backup[0][0] = g_dig0_addr;
	g_display_backup[1][0] = g_dig1_addr;
	g_display_backup[2][0] = g_dig2_addr;
	g_display_backup[3][0] = g_dig3_addr;	
	

	return ret;
}


static int ch455_release(struct inode *inode, struct file *file)
{
	struct ch455_device *pch455 = file->private_data;

	if (pch455->thread_id)
	{
		kthread_stop(pch455->thread_id);
		pch455->thread_id = NULL;		
	}
	msleep(150);
	kfree(pch455);
	
	return 0;
}


static const struct file_operations ch455_fops = {
	.owner		= THIS_MODULE,
	.open		= ch455_open,
	.release	= ch455_release,
	.write		= ch455_write,
	#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)
	.unlocked_ioctl = ch455_ioctl,
	#else
	.ioctl			= ch455_ioctl,
	#endif
	
};

static struct miscdevice ch455_misc = {
	.fops		= &ch455_fops,
	.name		= CH455_DEV_NAME,
	.minor		= MISC_DYNAMIC_MINOR,
};


static int ali_m36_ch455_init(void)
{
	int ret = 0;	

	//printk("Welcome, %s\n", ALI_PANEL_INFO);

	ret = misc_register(&ch455_misc);
	if (ret != 0) {
		PANEL_ERR_PRINTK(KERN_ERR "CH455: cannot register miscdev(err=%d)\n", ret);
		goto fail_misc;
	}

	ch455_input = input_allocate_device();
	if (!ch455_input) {
		PANEL_ERR_PRINTK(KERN_ERR "CH455: not enough memory for input device\n");
		ret = -ENOMEM;
		goto fail_input_alloc;
	}

	ch455_input->name = CH455_DEV_INPUT_NAME;
	ch455_input->phys = CH455_DEV_INPUT_NAME;
	ch455_input->id.bustype = BUS_HOST; //BUS_I2C;
	ch455_input->id.vendor = 0x0001;
	ch455_input->id.product = 0x0003;
	ch455_input->id.version = 0x0100;
	ch455_input->evbit[0] = BIT_MASK(EV_KEY);
	ch455_input->evbit[0] |= BIT_MASK(EV_REP);
	ret = input_register_device(ch455_input);
	if (ret)
	{
		goto fail_input_reg;
	}	

	ch455_input->rep[REP_DELAY] = REPEAT_DELAY;
	ch455_input->rep[REP_PERIOD] = REPEAT_INTERVAL;

	memset((void *)(&g_ch455_key_map), 0, sizeof(g_ch455_key_map));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	proc_create("panel", 0, NULL, &proc_panel_fops);
#else
	if (NULL != (proc_ch455 = create_proc_entry( "panel", 0, NULL )))
		proc_ch455->read_proc = ch455_read_proc;	
#endif
	return 0;

fail_input_reg:		input_free_device(ch455_input);
fail_input_alloc:	misc_deregister(&ch455_misc);
fail_misc:

	return ret;
}


static void __exit ali_m36_ch455_exit(void)
{
	//printk("Goodbye, %s\n", ALI_PANEL_INFO);
	
	input_unregister_device(ch455_input);
	input_free_device(ch455_input);
	ch455_input = NULL;
	
	misc_deregister(&ch455_misc);

	if(g_ch455_key_map.map_entry)
	{
		kfree(g_ch455_key_map.map_entry);
	}

	if (proc_ch455)
	{
		remove_proc_entry( "panel", NULL);
	}
	
	//printk("OK!\n");
}

module_init(ali_m36_ch455_init);
module_exit(ali_m36_ch455_exit);

MODULE_AUTHOR("Mao Feng");
MODULE_DESCRIPTION("ALi CH455 panel driver");
MODULE_LICENSE("GPL");

