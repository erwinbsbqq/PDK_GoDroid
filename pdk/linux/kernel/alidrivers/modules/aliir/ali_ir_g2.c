#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <ali_front_panel_common.h>
#include <asm/irq.h>
#if defined(CONFIG_ALI_CHIP_M3921)
#include <ali_interrupt.h>
#else
#include <asm/mach-ali/m36_irq.h>
#endif

#include <linux/ali_transport.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <ali_reg.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include "ali_ir_g2.h"
#include <ali_ir_common.h>
#include <linux/platform_device.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
#include <linux/sched/rt.h>
#endif

#define ALI_IRC_DEVICE_NAME 	"ali_ir"
#define IR_RLC_SIZE			256
#define VALUE_CLK_CYC		8		/* Work clock cycle, in uS */
#define VALUE_TOUT			24000	/* Timeout threshold, in uS */
#define VALUE_NOISETHR		80		/* Noise threshold, in uS */

#define IR_KEY_INVALID			0xFFFFFFFF
#define IR_KEY_TEST_BASE		0xFFFFFFF0
#define INTV_REPEAT_FIRST			100		/* in mini second */
#define INTV_REPEAT					100		/* in mini second */
#define IR_TIMER_DELAY 				10
#define IR_KEY_RELEASE			0
#define IR_KEY_PRESSED			1
#define IR_KEY_REPEAT				2

#define READ_INF_BYTE(addr)			(__REG8ALI(addr))
#define WRITE_INF_BYTE(addr,data)	(__REG8ALI(addr) = data)

#define INFRA_IRCCFG				(g_ir_base_addr + 0x00)
#define INFRA_FIFOCTRL				(g_ir_base_addr + 0x01)
#define INFRA_TIMETHR				(g_ir_base_addr + 0x02)
#define INFRA_NOISETHR				(g_ir_base_addr + 0x03)
#define INFRA_IER					(g_ir_base_addr + 0x06)
#define INFRA_ISR					(g_ir_base_addr + 0x07)
#define INFRA_RLCBYTE				(g_ir_base_addr + 0x08)

#define IR_STATE_INITED		(0x1<<0)
#define IR_STATE_OPENED	(0x1<<1)

#ifdef ARRAY_SIZE
	#undef ARRAY_SIZE
	#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#else
	#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif


static unsigned long g_ir_base_addr = 0x18018100;
static unsigned long g_ir_rst_addr = 0x18000080;
static unsigned long g_ir_rst_msk = (0x1<<19);
static unsigned long g_ir_gating_addr = 0x18000060;
static unsigned long g_ir_gating_msk = (0x1<<19);
#if defined(CONFIG_ALI_CHIP_M3921)
static unsigned long g_ir_irq_num = INT_ALI_IRC;
#else
static unsigned long g_ir_irq_num = M36_SYS_IRQ_BASE + 19;
#endif
static unsigned short g_ir_bufferin = 0;
static unsigned short g_ir_bufferout = 0;
static unsigned char g_ir_infra_buffer[IR_RLC_SIZE];
static unsigned long  g_ir_last_act_code = IR_KEY_INVALID;
static unsigned long  g_ir_code = IR_KEY_INVALID;
static int g_ir_protocol_type = IR_TYPE_UNDEFINE;
static int  g_ir_state = IR_KEY_RELEASE;
static unsigned long g_ir_repeat_width = 0;
static unsigned long g_ir_repeat_delay_rfk = 600;
static unsigned long g_ir_repeat_interval_rfk = 350;
static unsigned long g_ir_repeat_width_rfk = 0;
static unsigned long g_ir_key_state = IR_KEY_RELEASE;
static unsigned long g_ir_last_act_tick = 0;
static int g_ir_match = -1;
static unsigned long g_ir_state_machine = 0;
static unsigned long g_ir_now = 0;
static unsigned long g_ir_int_count = 0;
static unsigned long g_ir_key_received_count = 0;
static unsigned long g_ir_key_mapped_count = 0;
static unsigned char g_ir_debug = 0;
static unsigned int g_ir_rfk_port = 0;
static struct pan_standby_key  g_ir_standby_key = {0, 0};
static struct pan_standby_key  g_ir_resume_key = {0, 0};
static struct task_struct * g_ir_timer_thread_id = NULL;
static struct work_struct g_ir_wq;
static struct input_dev *g_ir_input = NULL;
static struct ali_fp_key_map_cfg g_ir_key_map;


//see emulate_mouse().
#define ACTION_UP   0
#define ACTION_DOWN 1

//default use KEY_F12 to enable/disable emulate mouse.
#define EMULATE_MOUSE_KEY					KEY_F12
//default use KEY_F11 to emulate mouse's left button key.
#define EMULATE_MOUSE_LEFT_BUTTON_KEY		KEY_F11

static bool g_ir_emulate_mouse = false;
//emulate mouse left button status
static bool g_left_button_down = false;
static int g_emulate_mouse_speed = 30;	//default - 30

struct pan_ir_endian g_ir_endian = {0, IR_TYPE_UNDEFINE, 0, 0};

static const struct linux_ir_key_map_t ali_linux_key_map[] =
{
	{(IR_ALI01_HKEY_MENU),		KEY_MENU},
	{(IR_ALI01_HKEY_RIGHT),		KEY_RIGHT},
	{(IR_ALI01_HKEY_DOWN),		KEY_DOWN},
	{(IR_ALI01_HKEY_ENTER),		KEY_ENTER},
	{(IR_ALI01_HKEY_LEFT),		KEY_LEFT},
	{(IR_ALI01_HKEY_UP),		KEY_UP},	
	
	{(IR_ALI01_HKEY_0),			KEY_0},
	{(IR_ALI01_HKEY_1),			KEY_1},
	{(IR_ALI01_HKEY_2),			KEY_2},
	{(IR_ALI01_HKEY_3),			KEY_3},
	{(IR_ALI01_HKEY_4),			KEY_4},
	{(IR_ALI01_HKEY_5),			KEY_5},
	{(IR_ALI01_HKEY_6),			KEY_6},
	{(IR_ALI01_HKEY_7),			KEY_7},
	{(IR_ALI01_HKEY_8),			KEY_8},
	{(IR_ALI01_HKEY_9),			KEY_9},	

	{(IR_ALI01_HKEY_NEWS),		KEY_NEWS},	
	{(IR_ALI01_HKEY_MAIL),		KEY_MAIL}, 
	{(IR_ALI01_HKEY_P_UP),		KEY_PAGEDOWN},
	{(IR_ALI01_HKEY_P_DOWN),	KEY_PAGEUP},
	{(IR_ALI01_HKEY_TEXT),		KEY_TEXT},
	{(IR_ALI01_HKEY_POWER),		KEY_POWER},
	{(IR_ALI01_HKEY_V_UP),		KEY_VOLUMEUP},
	{(IR_ALI01_HKEY_V_DOWN),	KEY_VOLUMEDOWN},
	{(IR_ALI01_HKEY_C_UP),		KEY_PREVIOUS},
	{(IR_ALI01_HKEY_C_DOWN),	KEY_NEXT},
	{(IR_ALI01_HKEY_AUDIO),		KEY_AUDIO},
	{(IR_ALI01_HKEY_SUBTITLE),	KEY_SUBTITLE},		
	{(IR_ALI01_HKEY_FIND),		KEY_FIND},
	{(IR_ALI01_HKEY_MUTE),		KEY_MUTE},
	{(IR_ALI01_HKEY_PAUSE),		KEY_PAUSE},		
	{(IR_ALI01_HKEY_INFOR),		KEY_INFO},
	{(IR_ALI01_HKEY_EXIT),		KEY_ESC},	
	{(IR_ALI01_HKEY_TVRADIO),	KEY_RADIO},
	{(IR_ALI01_HKEY_FAV),		KEY_FAVORITES},
	{(IR_ALI01_HKEY_RECORD),	KEY_RECORD},
	{(IR_ALI01_HKEY_PVR_INFO),	KEY_PVR},
	{(IR_ALI01_HKEY_PIP_LIST),	KEY_PIP_LIST},
	{(IR_ALI01_HKEY_SWAP),		KEY_SWAP},
	{(IR_ALI01_HKEY_USBREMOVE),	KEY_USBREMOVE},
	{(IR_ALI01_HKEY_PLAY),		KEY_PLAY},
	{(IR_ALI01_HKEY_FB),		KEY_FRAMEBACK},
	{(IR_ALI01_HKEY_FF),		KEY_FRAMEFORWARD},
	{(IR_ALI01_HKEY_SLOW),		KEY_SLOW},
	{(IR_ALI01_HKEY_STOP),		KEY_STOP},
	{(IR_ALI01_HKEY_ZOOM),		KEY_ZOOM},
	{(IR_ALI01_HKEY_EPG),		KEY_EPG},	
	{(IR_ALI01_HKEY_RECALL),	KEY_LAST},
	{(IR_ALI01_HKEY_RED),		KEY_RED},//red
	{(IR_ALI01_HKEY_GREEN),		KEY_GREEN},//green
	{(IR_ALI01_HKEY_YELLOW), 	KEY_YELLOW},//yellow
	{(IR_ALI01_HKEY_BLUE),		KEY_BLUE},//blue	
	{(IR_ALI01_HKEY_FILELIST),	KEY_FILE}, 
	{(IR_ALI01_HKEY_MOVE), 		KEY_MOVE},
	{(IR_ALI01_HKEY_V_FORMAT), 	KEY_SCREEN},
	{(IR_ALI01_HKEY_SLEEP), 	KEY_SLEEP},
	{(IR_ALI01_HKEY_DVRLIST), 	KEY_DVRLIST},
	{(IR_ALI01_HKEY_REVSLOW), 	KEY_REVSLOW},	
	{(IR_ALI01_HKEY_REPEATAB), 	KEY_AB},
};

#define IR_PRINTK(fmt, args...)			\
{										\
	if (0 !=  g_ir_debug)					\
	{									\
		printk(fmt, ##args);			\
	}									\
}

#define IR_ERR_PRINTK(fmt, args...)		\
{										\
	printk(fmt, ##args);				\
}

unsigned long irc_pulse_to_code(unsigned long pulse_width, unsigned long pulse_polarity);
int irc_get_ir_protocol_type(void);
static int ali_ir_open(struct inode *inode, struct file *file);
static int ali_ir_release(struct inode *inode, struct file *file);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)
static long ali_ir_ioctl(struct file * file, unsigned int cmd, unsigned long param);
#else
static int ali_ir_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long param);
#endif
static int  ali_ir_probe(struct platform_device *pdev);
static int ali_ir_remove(struct platform_device *pdev);
static int ali_ir_resume(struct platform_device *pdev);
static int ali_ir_suspend(struct platform_device *pdev, pm_message_t state);

extern unsigned short key_cnt;


static const struct file_operations g_ir_fops = {
	.owner		= THIS_MODULE,
	.open		= ali_ir_open,	
	.release		= ali_ir_release,
	#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)
	.unlocked_ioctl = ali_ir_ioctl,
	#else
	.ioctl			= ali_ir_ioctl,
	#endif
};

static struct miscdevice g_ir_misc = {
	.fops		= &g_ir_fops,
	.name		= ALI_IRC_DEVICE_NAME,
	.minor		= MISC_DYNAMIC_MINOR,
};

static struct platform_driver g_ir_driver =
{
	.driver		= {
		.name	= ALI_IRC_DEVICE_NAME,
		.owner	= THIS_MODULE,
	},
	.probe		= ali_ir_probe,
	.remove		= ali_ir_remove,
	.suspend	= ali_ir_suspend,
	.resume		= ali_ir_resume,
};


static struct  platform_device g_ir_device ={
	.name		= ALI_IRC_DEVICE_NAME,
	.id		= 0,
	.dev = {
		.driver = &g_ir_driver.driver,
	}
};


struct pan_ir_endian * get_ir_endian(void)
{
	return &g_ir_endian;
}


unsigned char get_ir_debug(void)
{
	return g_ir_debug;
}


static int ali_ir_send_msg(unsigned long  code, int ir_protocol, int ir_state)
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

	if (0 != g_ir_debug)
	{		
		for (i=0; i<sizeof(msg); i++) 
		{
			if (0 == (i%4))
			{
				IR_PRINTK("    ");
			}
			IR_PRINTK("%02x", msg[i]);			
		}		
		IR_PRINTK("\n");
	}
	
	
	ret = ali_transport_send_msg(g_ir_rfk_port, &msg, sizeof(msg));
	if (-1 == ret)
	{
		IR_ERR_PRINTK("[ %s %d ], rfk port %d send msg fail!\n", __FUNCTION__, __LINE__, g_ir_rfk_port);
	}

	return ret;
}

static void ali_ir_hw_ip_identify(void)
{
	unsigned long chip_ver = __REG32ALI(0x18000000);
	switch(chip_ver>>16)
	{
		case 0x3701:
		case 0x3603:	
			g_ir_base_addr = 0x18018100;
			g_ir_rst_addr = 0x18000080;
			g_ir_rst_msk = (0x1<<19);
			g_ir_gating_addr = 0x18000060;
			g_ir_gating_msk = (0x1<<19);
			g_ir_irq_num = 19;
			break;
		default:
			break;
	}
}

static void ali_ir_hw_gating(unsigned char gating)
{
	if(gating)
		__REG32ALI(g_ir_gating_addr) |= g_ir_gating_msk;
	else
		__REG32ALI(g_ir_gating_addr) &= (~g_ir_gating_msk);
}


#if 0
static void ali_ir_hw_reset(void)
{
	unsigned long tick;
	__REG32ALI(g_ir_rst_addr) |= g_ir_rst_msk;
	tick = jiffies;
	while(jiffies>(tick+1))
		break;
	__REG32ALI(g_ir_rst_addr) &= (~g_ir_rst_msk);
}
#endif

static int ali_ir_default_mapping (unsigned long ir_code)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(ali_linux_key_map); i++) {
		if (ali_linux_key_map[i].ir_code == ir_code) 
		{
			return ali_linux_key_map[i].key;
		}
	}

	return -1;
}


static int ali_ir_mapping(unsigned long code)
{
	int i = 0;
	int logic_code = 0;
	unsigned long my_code;
	unsigned char * map_entry = NULL;	
	unsigned long unit_len = 0;
	unsigned long unit_num = 0;
	unsigned long phy_code = 0;


	if ((NULL == g_ir_key_map.map_entry) || (3 == g_ir_key_map.phy_code))
	{		
		return ali_ir_default_mapping(code);
	}

	map_entry = g_ir_key_map.map_entry;	
	unit_len = g_ir_key_map.unit_len;
	unit_num = g_ir_key_map.unit_num;
	phy_code = g_ir_key_map.phy_code;
	
	for(i = 0; i<unit_num; i++)
	{	
		if((unsigned long)(&map_entry[i*unit_len]) & 0x3)
		{
			my_code = map_entry[i*unit_len];			
			my_code |= (map_entry[i*unit_len+1])<<8;			
			my_code |= (map_entry[i*unit_len+2])<<16;			
			my_code |= (map_entry[i*unit_len+3])<<24;			
		}
		else
		{
			my_code = *((unsigned long *)(&map_entry[i*unit_len]));
		}
		
		if(code == my_code)
		{
			g_ir_key_mapped_count ++;
			
			if(1 == phy_code)
			{				
				//IR_PRINTK("[ %s ], phy code index %d\n", __FUNCTION__, i);				
				return i;
			}
			else if (0 == phy_code)
			{
				logic_code =  map_entry[i*unit_len+4]; 				
				logic_code |= (map_entry[i*unit_len+5])<<8;						
				//IR_PRINTK("logic code 0x%08x\n", logic_code);				
				
				return logic_code;
			}
		}	
	}
	return -1;
}


static void check_standby_key(struct input_dev * input, unsigned char data)
{
	static unsigned int standby_cnt = 0;
	static unsigned char standby_flag = 0;
	static unsigned long low_tick = 0;
	unsigned long tick = 0;
	unsigned long width;
	int match = -1;
	

	if (1 != g_ir_standby_key.enable)
	{
		return;
	}

	
	if (0x7f == data)
	{
		standby_cnt++;		
	}
	else
	{	
		standby_cnt = 0;		
		if (1 == standby_flag)
		{
			standby_flag = 0;
			tick = jiffies;
			width = (unsigned long)(((long)tick - (long)low_tick) * 1000 / HZ);
			IR_PRINTK("[ %s %d],  low_tick = %ld, tick = %ld, width = %ld\n", __FUNCTION__, __LINE__, 
				low_tick, tick, width); 
			if (width >= 1000)
			{
				goto SEND_STANDBY_KEY;
			}
		}
	}
	
	//IR_PRINTK("[ %s %d ], data = 0x%02x, standby_cnt = %d\n", __FUNCTION__, __LINE__, data, standby_cnt);

	if (standby_cnt >= (VALUE_TOUT/VALUE_CLK_CYC/0x7f)) /* detect low count */
	{
		standby_flag = 1;
		low_tick = jiffies;
		IR_PRINTK("[ %s %d ],  low_tick = %ld standby_cnt = %d\n", __FUNCTION__, __LINE__, low_tick, standby_cnt);		
	}
	
	return ;
	
	SEND_STANDBY_KEY:	
	
	IR_ERR_PRINTK("[ %s %d ], key 0x%08x pressed, width = %ld\n", 
		__FUNCTION__, __LINE__, (unsigned int)g_ir_standby_key.key, width);	
	if(2 != g_ir_key_map.phy_code)
	{
		match = ali_ir_mapping(g_ir_standby_key.key);	
		if (match >= 0)
		{		
			g_ir_match = match;
			input_report_key(input, match, IR_KEY_PRESSED);							
			input_sync(input);
			input_report_key(input, match, IR_KEY_RELEASE);
			input_sync(input);						
			
			IR_PRINTK("[ %s ],  ir code %08x pressed\n", __FUNCTION__, match);				
		} 
		else
		{
			IR_ERR_PRINTK("[ %s ], No match key for ir code = 0x%08x\n", __FUNCTION__, (int)g_ir_standby_key.key);				
		}
	}
	else
	{					
		g_ir_code = g_ir_standby_key.key;
		g_ir_protocol_type = IR_TYPE_UNDEFINE;
		g_ir_state = IR_KEY_PRESSED;
		IR_PRINTK("[ %s %d ]\n", __FUNCTION__, __LINE__);
		schedule_work(&g_ir_wq);			
	}				
}	

//
// emulate key -> mouse
//
// EMULATE_MOUSE_KEY: emulate mouse enable/disable.
// KEY_UP/KEY_DOWN/KEY_LEFTKEY_RIGHT: emulate mouse movement.
// KEY_SELECT: emulate mouse left button.
//            method 1: key down -> left button down
//                      key up   -> left button up
// EMULATE_MOUSE_LEFT_BUTTON_KEY: emulate mouse left button.
//            method 2: can emulate Dragging.
//                      1st time key down/up -> left button down
//                      2nd time key down/up -> left button up
//
static bool emulate_mouse(struct input_dev* input, int key_code, int action)
{
	unsigned int code;
	int value;
	static int repeat_cnt = 0;	//repeat key count as scale

	// enable/disable emulate mouse
	if (key_code == EMULATE_MOUSE_KEY && action == IR_KEY_PRESSED)
	{
		g_ir_emulate_mouse = !g_ir_emulate_mouse;
		IR_PRINTK("ali_ir: emulate mouse - %s\n",g_ir_emulate_mouse?"enabled!":"disabled!");
		repeat_cnt = 0;	//reset it.

		//when quit emulate mouse, and left button is down,
		//shall send a left button up event!
		if (!g_ir_emulate_mouse)
		{
			if (g_left_button_down)
			{
				code = BTN_LEFT;
				value = ACTION_UP;
				IR_PRINTK("ali_ir: key(%d,%d) -> mouse rel(code %d, value %d)\n",key_code,action,code,value);
				input_report_key(input, code, value);
				input_sync(input);
			}
		}
		return false;
	}

	if (!g_ir_emulate_mouse)
		return false;

	//patch: when g_left_button_down is true & press EXIT(KEY_HOMEPAGE) key, need reverse g_left_button_down.
	if (key_code == KEY_HOMEPAGE && action == IR_KEY_PRESSED)
	{
		if (g_left_button_down)  //release drag
		{
			code = BTN_LEFT;
			value = ACTION_UP;
			IR_PRINTK("ali_ir: key(%d,%d) -> mouse rel(code %d, value %d)\n",key_code,action,code,value);
			input_report_key(input, code, value);
			input_sync(input);
			g_left_button_down = false;
		}
	}

	if (key_code == KEY_UP
		|| key_code == KEY_DOWN
		|| key_code == KEY_LEFT
		|| key_code == KEY_RIGHT)
	{
		if (action == IR_KEY_PRESSED || action == IR_KEY_REPEAT)
		{
			if (action == IR_KEY_REPEAT)
			{
				repeat_cnt ++;
				IR_PRINTK("ali_ir: emulate mouse repeat(%d)\n",repeat_cnt);
			}
			else
				repeat_cnt = 0;	//reset it.

			//value = 2 + repeat_cnt;
			//if (value > 25)
			//	value = 25;
			value = g_emulate_mouse_speed;
			switch (key_code)
			{
				case KEY_UP:
					code = REL_Y;
					value = -value;
					break;
				case KEY_DOWN:
					code = REL_Y;
					break;
				case KEY_LEFT:
					code = REL_X;
					value = -value;
					break;
				case KEY_RIGHT:
					code = REL_X;
					break;
			}
			unsigned long tick = jiffies;
			IR_PRINTK("ali_ir: key(%d,%d) -> mouse rel(code %d, value %d) @%ld\n",key_code,action,code,value,tick*1000/HZ);
			input_report_rel(input, code, value);
			input_sync(input);
		}
		else
			repeat_cnt = 0;	//not IR_KEY_REPEAT, reset it.

		return true;
	}
	else if (key_code == KEY_SELECT)
	{
//emulate mouse left button - 1:
//key down -> left button down
//key up   -> left button up
		if (action != IR_KEY_REPEAT)
		{
			//patch: when g_left_button_down is true & press KEY_SELECT key, need reverse g_left_button_down.
			if (g_left_button_down)
			{
				if (action == IR_KEY_RELEASE)	//release drag
				{
					code = BTN_LEFT;
					value = ACTION_UP;
					IR_PRINTK("ali_ir: key(%d,%d) -> mouse rel(code %d, value %d)\n",key_code,action,code,value);
					input_report_key(input, code, value);
					input_sync(input);

					g_left_button_down = !g_left_button_down;
				}
				else
				{
					;//ignore
				}
			}
			else
			{
			code = BTN_LEFT;
			value = action;
			IR_PRINTK("ali_ir: key(%d,%d) -> mouse rel(code %d, value %d)\n",key_code,action,code,value);
			input_report_key(input, code, value);
			input_sync(input);
		}
		}

		repeat_cnt = 0;	//other key, reset it.
		return true;
	}
	else if (key_code == EMULATE_MOUSE_LEFT_BUTTON_KEY)
	{
//emulate mouse left button - 2:
//to emulate Dragging action.
//1st time key down/up -> left button down
//2nd time key down/up -> left button up
		if (action != IR_KEY_REPEAT && action == IR_KEY_PRESSED)
		{
			if (g_left_button_down)		//release drag
			{
				code = BTN_LEFT;
				value = ACTION_UP;
				IR_PRINTK("ali_ir: key(%d,%d) -> mouse rel(code %d, value %d)\n",key_code,action,code,value);
				input_report_key(input, code, value);
				input_sync(input);
			}
			else		//drag
			{
				code = BTN_LEFT;
				value = ACTION_DOWN;
				IR_PRINTK("ali_ir: key(%d,%d) -> mouse rel(code %d, value %d)\n",key_code,action,code,value);
				input_report_key(input, code, value);
				input_sync(input);
			}
			g_left_button_down = !g_left_button_down;
		}

		repeat_cnt = 0;	//other key, reset it.
		return true;
	}

	repeat_cnt = 0;	//other key, reset it.
	return false;
}

static void generate_code(struct input_dev * input, unsigned long tick)
{
	static unsigned long last_tick = 0;	
	unsigned long code, pulse_width, last_width, pulse_polarity;
	unsigned char data;	
	unsigned long repeat_tick = 0;
	static unsigned long last_repeat_tick = 0;
	unsigned long last_repeat_width;	
	unsigned long phy_code = g_ir_key_map.phy_code;
	int match = -1;

	
	if(NULL== input)
	{
		return;
	}
	
	/* The time in idle */
	irc_pulse_to_code(((long)tick - (long)last_tick) * 1000 / HZ * 1000, 1);	/* The time in idle */

	last_tick = tick;
	pulse_width = 0;	
	//IR_PRINTK("===1, key_cnt = %d, %ld\n", key_cnt, (unsigned long)(tick * 1000 / HZ));		
	
	while (g_ir_bufferin != g_ir_bufferout)
	{
		data = g_ir_infra_buffer[g_ir_bufferout];
		pulse_width += ((data & 0x7f) * VALUE_CLK_CYC);	/* Pulse width */
		pulse_polarity = (data & 0x80) ? 1 : 0;
		g_ir_bufferout = ((g_ir_bufferout + 1) & (IR_RLC_SIZE - 1));/* Next data */	
		
		check_standby_key(input, data);
		
		/* Long pulse */			
		if ((!((data ^ g_ir_infra_buffer[g_ir_bufferout]) & 0x80)) && (g_ir_bufferout != g_ir_bufferin))
		{			
			continue;
		}		
		
		code = irc_pulse_to_code(pulse_width, pulse_polarity);				
		if ((code != IR_KEY_INVALID) && (code < IR_KEY_TEST_BASE))
		{
			g_ir_key_received_count ++;
			last_width = (unsigned long)(((long)tick - (long)g_ir_last_act_tick) * 1000 / HZ);			

			if(key_cnt == 0)		/* Receive a new key */
			{
				g_ir_last_act_code = IR_KEY_INVALID;
                g_ir_repeat_width = INTV_REPEAT_FIRST;
                g_ir_repeat_width_rfk = g_ir_repeat_delay_rfk;	
			}
			//else if(key_cnt == 1)	/* Receive a continous key */
			//{
			//	g_ir_repeat_width = INTV_REPEAT_FIRST;
			//}
			else
			{
				g_ir_repeat_width = INTV_REPEAT;
				//20140225, no modify here, let repeat to modify it.
				//g_ir_repeat_width_rfk = g_ir_repeat_interval_rfk;	
			}
			
			//IR_PRINTK("===3, [ %s ], last_width %ld\n", __FUNCTION__, last_width);
		    if ((g_ir_last_act_code != code) || (last_width > g_ir_repeat_width) || (key_cnt >= 1))	
			{						
				key_cnt++;
				g_ir_last_act_code = code;
				g_ir_last_act_tick = /* tick */ jiffies;		
				//IR_PRINTK("===4, [ %s ], g_ir_last_act_tick %ld, key_cnt %d\n", __FUNCTION__, (unsigned long)(g_ir_last_act_tick * 1000 / HZ), key_cnt);
				if (key_cnt == 1) 
				{					
					//IR_PRINTK("ir phy code %08x\n", (int)g_ir_last_act_code);
					if(2 != phy_code)
					{
						match = ali_ir_mapping(g_ir_last_act_code);							
						if (match >= 0)
						{
							g_ir_match = match;
							g_ir_key_state = IR_KEY_PRESSED;
							//for emulate mouse
							last_repeat_tick = /*tick*/jiffies;
							if (emulate_mouse(input, match, g_ir_key_state))
							{
							}
							else
							{
								input_report_key(input, match, IR_KEY_PRESSED);								
								input_sync(input);									
							}
							IR_PRINTK("[ %s ],  ir code %08x pressed, tick %ld\n", 
								__FUNCTION__, match, (unsigned long)(jiffies * 1000 / HZ));							
						} 
						else
						{
							key_cnt = 0;							
							IR_ERR_PRINTK("[ %s ], No match key for ir code = 0x%08x\n", 
								__FUNCTION__, (int)g_ir_last_act_code);	
							if (IR_KEY_RELEASE != g_ir_key_state)
							{
								g_ir_key_state = IR_KEY_RELEASE;
								if (g_ir_match >= 0)
								{
									//for emulate mouse
									if (emulate_mouse(input, g_ir_match, g_ir_key_state))
									{
									}
									else
									{
										input_report_key(input, g_ir_match, IR_KEY_RELEASE);
										input_sync(input);	
									}
									IR_PRINTK("[ %s ],  ir code %08x release, tick %ld\n", __FUNCTION__, g_ir_match, (unsigned long)(jiffies * 1000 / HZ));	
								}
							}
						}
					}
					else
					{								
						g_ir_key_state = IR_KEY_PRESSED;
						last_repeat_tick = /*tick*/jiffies;							
						g_ir_code = g_ir_last_act_code;
						g_ir_protocol_type = irc_get_ir_protocol_type();
						g_ir_state = IR_KEY_PRESSED;
						schedule_work(&g_ir_wq);
					}					
				} 
				else if (key_cnt > 1)
				{			
					g_ir_key_state = IR_KEY_PRESSED;					

					if(2 == phy_code)
					{
						repeat_tick = jiffies;					
						last_repeat_width = (unsigned long)(((long)repeat_tick - (long)last_repeat_tick));							
						if (last_repeat_width > g_ir_repeat_width_rfk)
						{		
							last_repeat_tick = repeat_tick;												
							g_ir_code = g_ir_last_act_code;
							g_ir_protocol_type = irc_get_ir_protocol_type();
							g_ir_state = IR_KEY_REPEAT;
							schedule_work(&g_ir_wq);
						}
					}
					else	//emulate mouse
					{
						repeat_tick = jiffies;					
						last_repeat_width = (unsigned long)(((long)repeat_tick - (long)last_repeat_tick));							
						if (last_repeat_width > g_ir_repeat_width_rfk)
						{
							IR_PRINTK("g_ir_repeat_width_rfk = %d\n", g_ir_repeat_width_rfk);
							emulate_mouse(input, g_ir_match, IR_KEY_REPEAT);
							last_repeat_tick = repeat_tick;
							g_ir_repeat_width_rfk = g_ir_repeat_interval_rfk;
						}
					}
				}
			}
		}
		
		pulse_width = 0;
	}
    	return;
}


static void ir_wq_handler (void *data)
{
	//generate_code(g_ir_input, g_ir_now);	
	ali_ir_send_msg(g_ir_code, g_ir_protocol_type, g_ir_state);		
}


static int ali_ir_timer(void *param)
{		
	struct sched_param param_pthread = { .sched_priority = MAX_RT_PRIO-1 };	
	
	
	sched_setscheduler(current, SCHED_FIFO, &param_pthread);
	
	while (!kthread_should_stop())
	{
		msleep(IR_TIMER_DELAY);			
		
		if((IR_KEY_PRESSED == g_ir_key_state) && (key_cnt >= 1))
		{
			unsigned long tick;
			unsigned long last_width;
			
			tick = jiffies;
			last_width = (unsigned long)(((long)tick - (long)g_ir_last_act_tick) * 1000 / HZ);								

			if(last_width>((g_ir_repeat_width * 3)/2))
			{					
				key_cnt = 0;		
				g_ir_key_state = IR_KEY_RELEASE;		
				if(2 != g_ir_key_map.phy_code)
				{				
					if (g_ir_match >= 0)
					{
						struct input_dev * input = (struct input_dev *)param;

						//for emulate mouse
						if (emulate_mouse(input, g_ir_match, g_ir_key_state))
						{
						}
						else
						{
							input_report_key(input, g_ir_match, IR_KEY_RELEASE);
							input_sync(input);
						}
						
						IR_PRINTK("[ %s ],  ir code %08x release, tick %ld\n", __FUNCTION__, g_ir_match, (unsigned long)(jiffies * 1000 / HZ));												
					}	
				}
				else
				{					
					ali_ir_send_msg(g_ir_last_act_code, irc_get_ir_protocol_type(), IR_KEY_RELEASE);						
				}						
			}
		}
	}

	
	return 0;
}

static irqreturn_t ali_ir_interrupt(int irq, void *dev_id)
{	
	volatile unsigned char status, num, num1;
	
	status = (READ_INF_BYTE(INFRA_ISR) & 3);
	WRITE_INF_BYTE(INFRA_ISR, status);

	
	switch (status) 
	{
		case 0x01:		/* If FIFO trigger, copy data to buffer */
		case 0x02:		/* If timeout, generate IR code in HSR */
		case 0x03:		/* FIFO trigger and timeout */
			do {
				num1 = num = READ_INF_BYTE(INFRA_FIFOCTRL) & 0x7F;
				while (num > 0) {
					/* Put RLC to buffer */
					g_ir_infra_buffer[g_ir_bufferin++] = READ_INF_BYTE(INFRA_RLCBYTE);
					g_ir_bufferin &= (IR_RLC_SIZE - 1);
					num--;
				};
			} while (num1 > 0);
			break;
		default:
			break;
	}

	if (0 != (status & 0x02)) {
		g_ir_now = jiffies;
		g_ir_int_count ++;			
		//schedule_work(&g_ir_wq);
		generate_code(g_ir_input, g_ir_now);									
	}

	return IRQ_HANDLED;
}

static int ali_ir_open(struct inode *inode, struct file *file)
{	
	IR_PRINTK("[ %s] ", __FUNCTION__);
	
	if(g_ir_state_machine&IR_STATE_OPENED){
		IR_PRINTK(KERN_ERR "Repeatedly open ali ir driver !\n");
		goto ok;
	}
	ali_ir_hw_gating(0);
	g_ir_repeat_width = INTV_REPEAT_FIRST;
	g_ir_bufferin = g_ir_bufferout = 0;
	WRITE_INF_BYTE(INFRA_IRCCFG, 0);
	/* Working clock expressions:
     	* (SB_CLK / (32 * CLK_SEL)) = 1 / VALUE_CLK_CYC, SB_CLK = 12MHz
     	* => CLK_SEL = (SB_CLK * VALUE_CLK_CYC / 32)
     	*/
    	WRITE_INF_BYTE(INFRA_IRCCFG, 0x80 | ((12 * VALUE_CLK_CYC) >> 5));

    	/* FIFO threshold */
    	WRITE_INF_BYTE(INFRA_FIFOCTRL, 0xA0);	/* 32 bytes */

    	/* Timeout threshold expressions:
     	* ((TIMETHR + 1) * 128 * VALUE_CLK_CYC) = VALUE_TOUT
     	* => TIMETHR = (VALUE_TOUT / (128 * VALUE_CLK_CYC)) - 1
     	*/
    	WRITE_INF_BYTE(INFRA_TIMETHR, (VALUE_TOUT / (VALUE_CLK_CYC << 7) - 1));

    	/* Noise pulse timeout expressions:
     	* Value = VALUE_NOISETHR / VALUE_CLK_CYC
     	*/
    	WRITE_INF_BYTE(INFRA_NOISETHR, VALUE_NOISETHR / VALUE_CLK_CYC);

	INIT_WORK(&g_ir_wq, (void *)ir_wq_handler);
	
	if (request_irq(g_ir_irq_num, ali_ir_interrupt, 0, "ali_ir", (void *)g_ir_input) < 0) 	
	{		
		IR_ERR_PRINTK(KERN_ERR "Failed to register ali ir irq 0x%08x\n", (unsigned int)g_ir_irq_num);		
		goto err1;
	}
	
	g_ir_timer_thread_id = kthread_create(
		ali_ir_timer, (void *)g_ir_input, "ali_ir_timer_thread");
	if(IS_ERR(g_ir_timer_thread_id))
	{
		IR_ERR_PRINTK(KERN_ERR "ir timer kthread create fail\n");
		g_ir_timer_thread_id = NULL;
		goto err2;
	}
	
	wake_up_process(g_ir_timer_thread_id);	
	
	/* Ensure no pending interrupt */
    	WRITE_INF_BYTE(INFRA_ISR, 3);

    	/* Enable IRC Interrupt */
    	WRITE_INF_BYTE(INFRA_IER, 3);

	memset(&g_ir_key_map, 0x00, sizeof(g_ir_key_map));
	g_ir_key_map.phy_code = 3;	/* use default key map*/
	g_ir_state_machine |= IR_STATE_OPENED;
ok:		
	IR_PRINTK(" OK\n");
	
	return 0;
	
err2:
	free_irq((g_ir_irq_num), (void *)g_ir_input);
	
err1:	
	IR_ERR_PRINTK("\n[ %s] Fail!\n", __FUNCTION__);
		
	
	return -EINVAL;	
}

static int ali_ir_release(struct inode *inode, struct file *file)
{	
	IR_PRINTK("[ %s %d ]\n", __FUNCTION__, __LINE__);
	
	if(!(g_ir_state_machine&IR_STATE_OPENED)){
		IR_PRINTK(KERN_ERR "Repeatedly close ali ir driver !\n");
		goto ok;
	}	
	
	free_irq(g_ir_irq_num, g_ir_input);	

	if (g_ir_timer_thread_id)
	{
		kthread_stop(g_ir_timer_thread_id);	
		g_ir_timer_thread_id = NULL;
	}
	
	ali_ir_hw_gating(1);	
	
	g_ir_state_machine &= (~IR_STATE_OPENED);
ok:		
	IR_PRINTK(" OK\n");
	
	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)
static long ali_ir_ioctl(struct file * file, unsigned int cmd, unsigned long param)
#else
static int ali_ir_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long param)
#endif
{
	int rlt = 0;

	
	if(!(g_ir_state_machine&IR_STATE_OPENED)){
		IR_ERR_PRINTK(KERN_ERR "ali ir not opened, ioctl invalid !\n");
		rlt = -EPERM;
		goto end;
	}
	

	IR_PRINTK("[ %s ]: cmd %d\n", __FUNCTION__, cmd);	
	switch(cmd)
	{
		case PAN_DRIVER_IR_SET_STANDBY_KEY: 				
			if (0 != (copy_from_user(&g_ir_standby_key, (void*)param, sizeof(g_ir_standby_key))))
			{
				return -EFAULT;
			}		
							
			IR_PRINTK("[ %s %d ]: g_ir_standby_key enable = %d, key = 0x%08x\n", 
				__FUNCTION__,  __LINE__, g_ir_standby_key.enable, (unsigned int)g_ir_standby_key.key);						
			
			break;
			
		case ALI_FP_CONFIG_KEY_MAP:
		{
			unsigned char * map_entry;
			struct ali_fp_key_map_cfg key_map;
			int i;
			
			if (0 != (copy_from_user(&key_map, (void*)param, sizeof(key_map))))
			{
				return -EFAULT;
			}
			
			IR_PRINTK("[ %s %d ], phy_code = %d\n", __FUNCTION__, __LINE__, (int)key_map.phy_code);			
			g_ir_key_map.phy_code = key_map.phy_code;	
			if (2 != g_ir_key_map.phy_code)
			{
				map_entry = kzalloc(key_map.map_len, GFP_KERNEL);
				if(NULL == map_entry)
				{
					IR_ERR_PRINTK("[ %s ]: ALI_IR_CONFIG_KEY_MAP Fail, not enouth memory!\n", __FUNCTION__);
					rlt = -ENOMEM;
					goto end;
				}
				if (0 != (copy_from_user(map_entry, key_map.map_entry, key_map.map_len)))
				{
					return -EFAULT;
				}
				key_map.map_entry = map_entry;
				key_map.unit_num = (key_map.map_len/key_map.unit_len);
				if(key_map.unit_num>KEY_CNT)
				{
					key_map.unit_num = KEY_CNT;
				}
				if(g_ir_key_map.map_entry)
				{
					kfree(g_ir_key_map.map_entry);
					g_ir_key_map.map_entry = NULL;
				}				
				
				for (i = 0; i < key_map.unit_num; ++i)
				{
					unsigned short key;

					/* phy_code : 0, logic; 1, index. */
					if(1 == key_map.phy_code){
						key = (unsigned short)i;
					}
					else{
						key = key_map.map_entry[i*key_map.unit_len+4];
						key |= (key_map.map_entry[i*key_map.unit_len+5])<<8;
					}
					
					__set_bit(key%KEY_CNT, g_ir_input->keybit);
				}
				
				g_ir_key_map.map_len = key_map.map_len;
				g_ir_key_map.unit_len = key_map.unit_len;
				g_ir_key_map.phy_code = key_map.phy_code;
				g_ir_key_map.unit_num = key_map.unit_num;
				g_ir_key_map.map_entry = key_map.map_entry;
				if (0 != g_ir_debug)
				{
					int i;
					IR_PRINTK("[ %s ]: \n", __FUNCTION__);
					for(i=0; i<g_ir_key_map.unit_num; i++){
						unsigned char * buf = &map_entry[i*g_ir_key_map.unit_len];
						unsigned long phy ;
						unsigned short log;
						phy = buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24);
						log = buf[4]|(buf[5]<<8);
						IR_PRINTK("%08x	%04x\n", (int)phy, log);
					}
					IR_PRINTK("\n");
				}
			}			
		}
		break;		

		case ALI_FP_GET_IR_INT_COUNT:
		{			
			IR_PRINTK("[ %s ]: g_ir_int_count = %ld\n", __FUNCTION__, g_ir_int_count);			
			if (0 != copy_to_user((void*)param, &g_ir_int_count, sizeof(g_ir_int_count)))
			{
        				return -EFAULT;
			}
		}
		break;

		case ALI_FP_GET_IR_KEY_RECEIVED_COUNT:
		{			
			IR_PRINTK("[ %s ]: ir_key_recv_count = %ld\n", __FUNCTION__, g_ir_key_received_count);			
			if (0 != copy_to_user((void*)param, &g_ir_key_received_count, sizeof(g_ir_key_received_count)))
			{
        				return -EFAULT;
			}
		}
		break;

		case ALI_FP_GET_IR_KEY_MAPPED_COUNT:
		{			
			IR_PRINTK("[ %s ]: g_ir_key_mapped_count = %ld\n", __FUNCTION__, g_ir_key_mapped_count);			
			if (0 != copy_to_user((void*)param, &g_ir_key_mapped_count, sizeof(g_ir_key_mapped_count)))
			{
        				return -EFAULT;
			}
		}
		break;

		case ALI_FP_SET_IR_KEY_DEBUG:
		{				
			if (0 != (copy_from_user(&g_ir_debug, (void*)param, sizeof(g_ir_debug))))
			{
				return -EFAULT;
			}
			
			IR_PRINTK("[ %s ]: g_ir_debug = %d\n", __FUNCTION__, g_ir_debug);				
		}
		break;

		case ALI_FP_SET_SOCKPORT:
		{				
			if (0 != (copy_from_user(&g_ir_rfk_port, (void*)param, sizeof(g_ir_rfk_port))))
			{
				return -EFAULT;
			}
			
			IR_PRINTK("[ %s ]: g_ir_rfk_port = %d\n", __FUNCTION__, g_ir_rfk_port);				
		}
		break;	

		case ALI_FP_SET_REPEAT_INTERVAL:
		{				
			unsigned long rep[2] = {0, 0};
			
			if (0 != (copy_from_user(&rep, (void*)param, sizeof(rep))))
			{
				return -EFAULT;
			}

			g_ir_repeat_delay_rfk = rep[0];
			g_ir_repeat_interval_rfk = rep[1];			
			
			IR_PRINTK("[ %s ]: g_ir_repeat_delay_rfk = %ld\n", __FUNCTION__, g_ir_repeat_delay_rfk);	
			IR_PRINTK("[ %s ]: g_ir_repeat_interval_rfk = %ld\n", __FUNCTION__, g_ir_repeat_interval_rfk);				
		}
		break;

		// set emulate mouse speed.
		case PAN_DRIVER_IR_SET_EMOUSE:
		{
			int speed;
			if (0 != (copy_from_user(&speed, (void*)param, sizeof(speed))))
			{
				return -EFAULT;
			}

			if (speed > 0 && speed <= 50)
			{
				g_emulate_mouse_speed = speed;
				IR_PRINTK("[ %s ]: g_emulate_mouse_speed = %ld\n", __FUNCTION__, g_emulate_mouse_speed);
			}
			else
			{
				IR_ERR_PRINTK("[ %s ]: bad speed = %ld\n", __FUNCTION__, speed);
			}
		}
		break;

		case PAN_DRIVER_IR_SET_ENDIAN: 				
			if (0 != (copy_from_user(&g_ir_endian, (void*)param, sizeof(g_ir_endian))))
			{
				return -EFAULT;
			}		
							
			IR_PRINTK("[ %s %d ]: endian enable = %d, protocol = %d, bit_msb_first = %d, byte_msb_first = %d\n", 
				__FUNCTION__,  __LINE__, g_ir_endian.enable, g_ir_endian.protocol,
				g_ir_endian.bit_msb_first, g_ir_endian.byte_msb_first);						
			
			break;
		case PAN_DRIVER_IR_SET_RESUME_KEY: 				
			if (0 != (copy_from_user(&g_ir_resume_key, (void*)param, sizeof(g_ir_resume_key))))
			{
				return -EFAULT;
			}		
							
			IR_PRINTK("[ %s %d ]: g_ir_resume_key enable = %d, key = 0x%08x\n", 
				__FUNCTION__,  __LINE__, g_ir_resume_key.enable, (unsigned int)g_ir_resume_key.key);						
			
			break;
		
		
		default:
			rlt = -EPERM;
			break;
	}

end:
	return rlt;
}


static int  ali_ir_probe(struct platform_device *pdev)
{	
	IR_PRINTK("[ %s %d ]", __FUNCTION__, __LINE__);		
	return 0;
}

static int ali_ir_remove(struct platform_device *pdev)
{	
	IR_PRINTK("[ %s %d ]", __FUNCTION__, __LINE__);	
	return 0;
}


static int ali_ir_resume(struct platform_device *pdev)
{	
	int match = -1;

	
	if (1 != g_ir_resume_key.enable)
	{
		return 0;
	}
	
	IR_PRINTK("[ %s %d ], send resume key 0x%08x\n", 
		__FUNCTION__, __LINE__, (unsigned int)g_ir_resume_key.key);	
		
	if(2 != g_ir_key_map.phy_code)
	{
		match = ali_ir_mapping(g_ir_resume_key.key);	
		if (match >= 0)
		{		
			g_ir_match = match;
			input_report_key(g_ir_input, match, IR_KEY_PRESSED);							
			input_sync(g_ir_input);
			input_report_key(g_ir_input, match, IR_KEY_RELEASE);
			input_sync(g_ir_input);						
			
			IR_PRINTK("[ %s ],  ir code %08x pressed\n", __FUNCTION__, match);				
		} 
		else
		{
			IR_ERR_PRINTK("[ %s ], No match key for ir code = 0x%08x\n", __FUNCTION__, (int)g_ir_resume_key.key);				
		}
	}
	else
	{					
		g_ir_code = g_ir_resume_key.key;
		g_ir_protocol_type = IR_TYPE_UNDEFINE;
		g_ir_state = IR_KEY_PRESSED;
		IR_PRINTK("[ %s %d ]\n", __FUNCTION__, __LINE__);
		schedule_work(&g_ir_wq);			
	}	
   	return 0;
}

static int ali_ir_suspend(struct platform_device *pdev, pm_message_t state)
{	
	IR_PRINTK("[ %s %d ]", __FUNCTION__, __LINE__);	
	return 0;
}


/* Driver Initialization */
static int __devinit ali_ir_init(void) 
{
	int ret = 0;	
	int i = 0;

	
	IR_PRINTK("[ %s ] ", __FUNCTION__);	
	
	if(0 != platform_driver_register(&g_ir_driver))
	{
			printk(KERN_ERR"register driver error!!\n");
			return -1;
	}
	platform_device_register(&g_ir_device);

	ret = misc_register(&g_ir_misc);
	if (ret != 0) {
		IR_ERR_PRINTK(KERN_ERR " cannot register ali ir miscdev(err=%d)\n", ret);
		goto fail_misc;
	}

	g_ir_input = input_allocate_device();
	if (!g_ir_input) {
		IR_ERR_PRINTK(KERN_ERR " not enough memory for ali ir input device\n");
		ret = -ENOMEM;
		goto fail_input_alloc;
	}

	g_ir_input->name = "ali_ir";
	g_ir_input->phys = "ali_ir/input0";
	g_ir_input->id.bustype = BUS_HOST; //BUS_I2C;
	g_ir_input->id.vendor = 0x0001;
	g_ir_input->id.product = 0x0001;
	g_ir_input->id.version = 0x0100;
	__set_bit(EV_KEY, g_ir_input->evbit);
	__set_bit(EV_REP, g_ir_input->evbit);
//emulate mouse
	printk("ali_ir: set bit EV_REL\n");
	__set_bit(EV_REL, g_ir_input->evbit);
	__set_bit(REL_X, g_ir_input->relbit);
	__set_bit(REL_Y, g_ir_input->relbit);

	ret = input_register_device(g_ir_input);
	if (ret){
		IR_ERR_PRINTK(KERN_ERR " fail register for ali ir input device\n");
		goto fail_input_reg;
	}
	
	g_ir_input->rep[REP_DELAY] = INTV_REPEAT_FIRST;
	g_ir_input->rep[REP_PERIOD] = INTV_REPEAT;		
	for (i = 0; i < ARRAY_SIZE(ali_linux_key_map); i++) 
	{			
		__set_bit(ali_linux_key_map[i].key%KEY_CNT, g_ir_input->keybit);
	}
	
	g_ir_state_machine = IR_STATE_INITED;
	ali_ir_hw_ip_identify();
	memset((void *)(&g_ir_key_map), 0, sizeof(g_ir_key_map));
	memset((void *)(&g_ir_standby_key), 0x00, sizeof(g_ir_standby_key));
	memset((void *)(&g_ir_resume_key), 0x00, sizeof(g_ir_resume_key));	
	IR_PRINTK(" OK!\n");
	
	return 0;

fail_input_reg:		
	input_free_device(g_ir_input);
fail_input_alloc:	
	misc_deregister(&g_ir_misc);
fail_misc:
	return ret;
}


static void __exit ali_ir_exit(void)
{
	IR_PRINTK("%s: ", __FUNCTION__);
	
	if(g_ir_state_machine&IR_STATE_OPENED){
		IR_ERR_PRINTK(KERN_ERR " ali ir not closed\n");
		ali_ir_release(0, 0);
	}
	
	platform_device_unregister(&g_ir_device);	
	platform_driver_unregister(&g_ir_driver);
	
	input_unregister_device(g_ir_input);
	input_free_device(g_ir_input);
	g_ir_input = NULL;
	misc_deregister(&g_ir_misc);
	if(g_ir_key_map.map_entry)
	{
		kfree(g_ir_key_map.map_entry);
		g_ir_key_map.map_entry = NULL;
	}
	g_ir_state_machine = 0;
	
	IR_PRINTK(" OK!\n");
}


module_init(ali_ir_init);
module_exit(ali_ir_exit);

MODULE_AUTHOR("Goliath Peng");
MODULE_DESCRIPTION("ALi IR Driver");
MODULE_LICENSE("GPL");

