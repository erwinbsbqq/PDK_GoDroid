#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/ali_front_panel.h>
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


#if 1
#define ALI_IR_PRINT			printk
#else
#define ALI_IR_PRINT(...)		do{}while(0)
#endif


#define ALI_IRC_DEVICE_NAME 	"ali_ir_g2"
#define IR_RLC_SIZE			256
#define VALUE_CLK_CYC		8		/* Work clock cycle, in uS */
#define VALUE_TOUT			24000	/* Timeout threshold, in uS */
#define VALUE_NOISETHR		80		/* Noise threshold, in uS */

#define IR_KEY_INVALID			0xFFFFFFFF
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
//static unsigned long g_ir_rep_delay = INTV_REPEAT_FIRST;
//static unsigned long g_ir_rep_interval = INTV_REPEAT;
static unsigned short bufferin = 0;
static unsigned short bufferout = 0;
static unsigned char infra_buffer[IR_RLC_SIZE];
static unsigned long  last_act_code = IR_KEY_INVALID;
static unsigned long ir_repeat_width;
static unsigned long ir_repeat_delay_rfk = 600;
static unsigned long ir_repeat_interval_rfk = 350;
static unsigned long ir_repeat_width_rfk = 0;
static unsigned long key_state = IR_KEY_RELEASE;
static unsigned long last_act_tick = 0;
static int i_match = -1;
static unsigned long g_ir_state_machine = 0;
static unsigned long now;
static unsigned long ir_int_count = 0;
static unsigned long ir_key_received_count = 0;
static unsigned long ir_key_mapped_count = 0;
static unsigned char ir_debug = 0;
static unsigned int ir_rfk_port = 0;
static struct pan_standby_key  standby_key = {0, 0};
static struct task_struct * g_ir_timer_thread_id = NULL;
static struct work_struct g_ir_wq;
static struct input_dev *ali_ir_input = NULL;
static struct ali_fp_key_map_cfg g_key_map;


unsigned long irc_pulse_to_code(unsigned long pulse_width, unsigned long pulse_polarity);
int irc_get_ir_protocol_type(void);
extern unsigned short key_cnt;


unsigned char get_ir_debug(void)
{
	return ir_debug;
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

	if (0 != ir_debug)
	{
		ALI_IR_PRINT("[ %s %d ], rfk port %d send msg :\n", __FUNCTION__, __LINE__, ir_rfk_port);
		for (i=0; i<sizeof(msg); i++)
		{
			ALI_IR_PRINT("%02x ", msg[i]);
		}
		ALI_IR_PRINT("\n");
	}
	
	
	ret = ali_transport_send_msg(ir_rfk_port, &msg, sizeof(msg));
	if (-1 == ret)
	{
		ALI_IR_PRINT("[ %s %d ], rfk port %d send msg fail!\n", __FUNCTION__, __LINE__, ir_rfk_port);
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


static int ali_ir_mapping(unsigned long phy_code)
{
	int i;
	int logic_code = 0;

	
	if(NULL==g_key_map.map_entry)
	{
		return -1;
	}
	
	for(i = 0; i<g_key_map.unit_num; i++)
	{
		unsigned long my_code;
		
		
		if((unsigned long)(&g_key_map.map_entry[i*g_key_map.unit_len]) & 0x3)
		{
			my_code = g_key_map.map_entry[i*g_key_map.unit_len];			
			my_code |= (g_key_map.map_entry[i*g_key_map.unit_len+1])<<8;			
			my_code |= (g_key_map.map_entry[i*g_key_map.unit_len+2])<<16;			
			my_code |= (g_key_map.map_entry[i*g_key_map.unit_len+3])<<24;			
		}
		else
		{
			my_code = *((unsigned long *)(&g_key_map.map_entry[i*g_key_map.unit_len]));
		}
		//ALI_IR_PRINT("ali_ir: my_code = 0x%08x\n\n", my_code);
		if(phy_code == my_code)
		{
			ir_key_mapped_count ++;
			
			if(1 == g_key_map.phy_code)
			{
				if (0 != ir_debug)
				{
					ALI_IR_PRINT("[ %s ], phy code index %d\n", __FUNCTION__, i);
				}
				return i;
			}
			else if (0 == g_key_map.phy_code)
			{
				logic_code =  g_key_map.map_entry[i*g_key_map.unit_len+4]; 				
				logic_code |= (g_key_map.map_entry[i*g_key_map.unit_len+5])<<8;				
				
				if (0 != ir_debug)
				{
					ALI_IR_PRINT("logic code 0x%08x\n", logic_code);
				}
				
				return logic_code;
			}
		}	
	}
	return -1;
}


static void check_standby_key(struct input_dev * input, unsigned char data)
{
	static unsigned int standby_cnt = 0;
	


	if (0x7f == data)
	{
		standby_cnt++;		
	}
	else
	{	
		standby_cnt = 0;
	}

	if (standby_cnt >= 15)
	{		
		standby_cnt = 0;

		if (1 == standby_key.enable)
		{
			if(2 != g_key_map.phy_code)
			{
				i_match = ali_ir_mapping(standby_key.key);	
				if (i_match >= 0)
				{						
					input_report_key(input, i_match, IR_KEY_PRESSED);							
					input_sync(input);
					input_report_key(input, i_match, IR_KEY_RELEASE);
					input_sync(input);		
					
					if (0 != ir_debug)
					{
						ALI_IR_PRINT("[ %s ],  ir code %08x pressed\n", __FUNCTION__, i_match);
					}
				} 
				else
				{
					ALI_IR_PRINT("[ %s ], No match key for ir code = 0x%08x\n", __FUNCTION__, (int)standby_key.key);				
				}
			}
			else
			{			
				ali_ir_send_msg(standby_key.key, IR_TYPE_UNDEFINE, IR_KEY_PRESSED);	
				ali_ir_send_msg(standby_key.key, IR_TYPE_UNDEFINE, IR_KEY_RELEASE);
			}	
		}
	}	
}


static void generate_code(struct input_dev * input, unsigned long tick)
{
	static unsigned long last_tick = 0;	
	unsigned long code, pulse_width, last_width, pulse_polarity;
	unsigned char data;	
	unsigned long repeat_tick = 0;
	static unsigned long last_repeat_tick = 0;
	unsigned long last_repeat_width;	

	
	if(NULL== input)
	{
		return;
	}
	
	irc_pulse_to_code(((long)tick - (long)last_tick) * 1000 / HZ * 1000, 1);	/* The time in idle */

	last_tick = tick;
	pulse_width = 0;	
	//ALI_IR_PRINT("1, key_cnt = %d, %ld\n", key_cnt, (unsigned long)(tick * 1000 / HZ));		
	
	while (bufferin != bufferout)
	{
		data = infra_buffer[bufferout];
		pulse_width += ((data & 0x7f) * VALUE_CLK_CYC);	/* Pulse width */
		pulse_polarity = (data & 0x80) ? 1 : 0;
		bufferout = ((bufferout + 1) & (IR_RLC_SIZE - 1));/* Next data */	
		
		check_standby_key(input, data);
		
		/* Long pulse */			
		if ((!((data ^ infra_buffer[bufferout]) & 0x80)) && (bufferout != bufferin))
		{			
			continue;
		}		
		
		code = irc_pulse_to_code(pulse_width, pulse_polarity);				
		if ((code != IR_KEY_INVALID) && (code < 0xfffffff0))
		{
			ir_key_received_count ++;
			last_width = (unsigned long)(((long)tick - (long)last_act_tick) * 1000 / HZ);			

			if(key_cnt == 0)		/* Receive a new key */
			{
				last_act_code = IR_KEY_INVALID;
                			ir_repeat_width = INTV_REPEAT_FIRST;
			}
			//else if(key_cnt == 1)	/* Receive a continous key */
			//{
			//	ir_repeat_width = INTV_REPEAT_FIRST;
			//}
			else
			{
				ir_repeat_width = INTV_REPEAT;
			}
			
			
		    	if (last_act_code != code || last_width > ir_repeat_width)		    	
			{						
				key_cnt++;
				last_act_code = code;
				last_act_tick = tick;						
				if (key_cnt == 1) 
				{
					//ALI_IR_PRINT("ir phy code %08x\n", (int)last_act_code);
					if(2 != g_key_map.phy_code)
					{
						i_match = ali_ir_mapping(last_act_code);	
						if (i_match >= 0)
						{
							key_state = IR_KEY_PRESSED;
							input_report_key(input, i_match, IR_KEY_PRESSED);							
							input_sync(input);
							if (0 != ir_debug)
							{
								ALI_IR_PRINT("[ %s ],  ir code %08x pressed\n", __FUNCTION__, i_match);
							}
						} 
						else
						{
							ALI_IR_PRINT("[ %s ], No match key for ir code = 0x%08x\n", __FUNCTION__, (int)last_act_code);												
						}
					}
					else
					{
						key_state = IR_KEY_PRESSED;
						last_repeat_tick = tick;							
						ali_ir_send_msg(last_act_code, irc_get_ir_protocol_type(), IR_KEY_PRESSED);						
					}
					
					
				} 
				else if (key_cnt > 1)
				{							
					key_state = IR_KEY_PRESSED;					

					if(2 == g_key_map.phy_code)
					{
						repeat_tick = jiffies;					
						last_repeat_width = (unsigned long)(((long)repeat_tick - (long)last_repeat_tick));							
						if (last_repeat_width > ir_repeat_width_rfk)
						{		
							last_repeat_tick = repeat_tick;
							ir_repeat_width_rfk = ir_repeat_interval_rfk;							
							ali_ir_send_msg(last_act_code, irc_get_ir_protocol_type(), IR_KEY_REPEAT);								
						}
					}
				}
			}
		}
        	#if 0
		else
		{			
			last_act_tick = tick;				
		}
		#endif
		pulse_width = 0;
	}
    	return;
}


static void ir_wq_handler (void *data)
{
	generate_code(ali_ir_input, now);	
}


static int ali_ir_timer(void *param)
{		
	while (!kthread_should_stop())
	{
		msleep(IR_TIMER_DELAY);			
		
		if((IR_KEY_PRESSED == key_state) && (key_cnt >= 1))
		{
			unsigned long tick;
			unsigned long last_width;
			
			tick = jiffies;
			last_width = (unsigned long)(((long)tick - (long)last_act_tick) * 1000 / HZ);								

			if(last_width>((ir_repeat_width * 4)/2))					
			{					
				if(2 != g_key_map.phy_code)
				{				
					if (i_match >= 0)
					{
						struct input_dev * input = (struct input_dev *)param;
						
						input_report_key(input, i_match, IR_KEY_RELEASE);
						input_sync(input);
						
						if (0 != ir_debug)
						{
							ALI_IR_PRINT("[ %s ],  ir code %08x release\n", __FUNCTION__, i_match);
						}						
					}	
				}
				else
				{
					ir_repeat_width_rfk = ir_repeat_delay_rfk;					
					ali_ir_send_msg(last_act_code, irc_get_ir_protocol_type(), IR_KEY_RELEASE);						
				}

				key_state = IR_KEY_RELEASE;				
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
		case 0x02:		/* If timeout, generate IR code in HSR */
		case 0x01:		/* If FIFO trigger, copy data to buffer */
			do {
				num1 = num = READ_INF_BYTE(INFRA_FIFOCTRL) & 0x7F;
				while (num > 0) {
					/* Put RLC to buffer */
					infra_buffer[bufferin++] = READ_INF_BYTE(INFRA_RLCBYTE);
					bufferin &= (IR_RLC_SIZE - 1);
					num--;
				};
			} while (num1 > 0);
			break;
		default:
			break;
	}

	if (status == 0x02) {
		now = jiffies;
		ir_int_count ++;					
		//generate_code((struct input_dev *)dev_id, now);
		schedule_work(&g_ir_wq);
	}

	return IRQ_HANDLED;
}

static int ali_ir_open(struct inode *inode, struct file *file)
{
	if(0 != ir_debug){
		ALI_IR_PRINT("[ %s] ", __FUNCTION__);
	}
	if(g_ir_state_machine&IR_STATE_OPENED){
		ALI_IR_PRINT(KERN_ERR "Repeatedly open ali ir driver !\n");
		goto ok;
	}
	ali_ir_hw_gating(0);
	ir_repeat_width = INTV_REPEAT_FIRST;
	bufferin = bufferout = 0;
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
	
	if (request_irq(g_ir_irq_num, ali_ir_interrupt, 0, "ali_ir", (void *)ali_ir_input) < 0) 	
	{		
		ALI_IR_PRINT(KERN_ERR "Failed to register ali ir irq 0x%08x\n", (unsigned int)g_ir_irq_num);		
		goto err1;
	}
	
	g_ir_timer_thread_id = kthread_create(
		ali_ir_timer, (void *)ali_ir_input, "ali_ir_timer_thread");
	if(IS_ERR(g_ir_timer_thread_id))
	{
		ALI_IR_PRINT(KERN_ERR "ir timer kthread create fail\n");
		g_ir_timer_thread_id = NULL;
		goto err2;
	}
	
	wake_up_process(g_ir_timer_thread_id);	
	
	/* Ensure no pending interrupt */
    	WRITE_INF_BYTE(INFRA_ISR, 3);

    	/* Enable IRC Interrupt */
    	WRITE_INF_BYTE(INFRA_IER, 3);

	g_ir_state_machine |= IR_STATE_OPENED;
ok:	
	if(0 != ir_debug){
		ALI_IR_PRINT(" OK\n");
	}
	return 0;
	
err2:
	free_irq((g_ir_irq_num), (void *)ali_ir_input);
	
err1:	
	ALI_IR_PRINT("\n[ %s] Fail!\n", __FUNCTION__);
		
	
	return -EINVAL;	
}

static int ali_ir_release(struct inode *inode, struct file *file)
{
	if(0 != ir_debug)
	{
		ALI_IR_PRINT("[ %s %d ]\n", __FUNCTION__, __LINE__);
	}
	if(!(g_ir_state_machine&IR_STATE_OPENED)){
		ALI_IR_PRINT(KERN_ERR "Repeatedly close ali ir driver !\n");
		goto ok;
	}	
	
	free_irq(g_ir_irq_num, ali_ir_input);	

	if (g_ir_timer_thread_id)
	{
		kthread_stop(g_ir_timer_thread_id);	
		g_ir_timer_thread_id = NULL;
	}
	
	ali_ir_hw_gating(1);	
	
	g_ir_state_machine &= (~IR_STATE_OPENED);
ok:	
	if(0 != ir_debug){
		ALI_IR_PRINT(" OK\n");
	}
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
		ALI_IR_PRINT(KERN_ERR "ali ir not opened, ioctl invalid !\n");
		rlt = -EPERM;
		goto end;
	}

	if (0 != ir_debug)
	{
		ALI_IR_PRINT("[ %s ]: cmd %d\n", __FUNCTION__, cmd);
	}
	
	switch(cmd)
	{
		case PAN_DRIVER_IR_SET_STANDBY_KEY: 				
			if (0 != (copy_from_user(&standby_key, (void*)param, sizeof(standby_key))))
			{
				return -EFAULT;
			}			
			
			if (0 != ir_debug)
			{				
				ALI_IR_PRINT("[ %s %d ]: standby_key enable = %d, key = 0x%08x\n", 
					__FUNCTION__,  __LINE__, standby_key.enable, (unsigned int)standby_key.key);	
			}			
			
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
			if (0 != ir_debug)
			{
				ALI_IR_PRINT("[ %s %d ], phy_code = %d\n", __FUNCTION__, __LINE__, 
						(int)key_map.phy_code);
			}

			g_key_map.phy_code = key_map.phy_code;			
			if (2 != g_key_map.phy_code)
			{
				map_entry = kzalloc(key_map.map_len, GFP_KERNEL);
				if(NULL==map_entry)
				{
					ALI_IR_PRINT("[ %s ]: ALI_IR_CONFIG_KEY_MAP Fail, not enouth memory!\n", __FUNCTION__);
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
				if(g_key_map.map_entry)
				{
					kfree(g_key_map.map_entry);
				}
				g_key_map.map_entry = NULL;
				
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
					
					__set_bit(key%KEY_CNT, ali_ir_input->keybit);
				}
				
				g_key_map.map_len = key_map.map_len;
				g_key_map.unit_len = key_map.unit_len;
				g_key_map.phy_code = key_map.phy_code;
				g_key_map.unit_num = key_map.unit_num;
				g_key_map.map_entry = key_map.map_entry;
				if (0 != ir_debug)
				{
					int i;
					ALI_IR_PRINT("[ %s ]: \n", __FUNCTION__);
					for(i=0; i<g_key_map.unit_num; i++){
						unsigned char * buf = &map_entry[i*g_key_map.unit_len];
						unsigned long phy ;
						unsigned short log;
						phy = buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24);
						log = buf[4]|(buf[5]<<8);
						ALI_IR_PRINT("%08x	%04x\n", (int)phy, log);
					}
					ALI_IR_PRINT("\n");
				}
			}			
		}
		break;		

		case ALI_FP_GET_IR_INT_COUNT:
		{
			if (0 != ir_debug)
			{
				ALI_IR_PRINT("[ %s ]: ir_int_count = %ld\n", __FUNCTION__, ir_int_count);
			}
			if (0 != copy_to_user((void*)param, &ir_int_count, sizeof(ir_int_count)))
			{
        				return -EFAULT;
			}
		}
		break;

		case ALI_FP_GET_IR_KEY_RECEIVED_COUNT:
		{
			if (0 != ir_debug)
			{
				ALI_IR_PRINT("[ %s ]: ir_key_recv_count = %ld\n", __FUNCTION__, ir_key_received_count);
			}
			if (0 != copy_to_user((void*)param, &ir_key_received_count, sizeof(ir_key_received_count)))
			{
        				return -EFAULT;
			}
		}
		break;

		case ALI_FP_GET_IR_KEY_MAPPED_COUNT:
		{
			if (0 != ir_debug)
			{
				ALI_IR_PRINT("[ %s ]: ir_key_mapped_count = %ld\n", __FUNCTION__, ir_key_mapped_count);
			}
			if (0 != copy_to_user((void*)param, &ir_key_mapped_count, sizeof(ir_key_mapped_count)))
			{
        				return -EFAULT;
			}
		}
		break;

		case ALI_FP_SET_IR_KEY_DEBUG:
		{				
			if (0 != (copy_from_user(&ir_debug, (void*)param, sizeof(ir_debug))))
			{
				return -EFAULT;
			}
			if (0 != ir_debug)
			{
				ALI_IR_PRINT("[ %s ]: ir_debug = %d\n", __FUNCTION__, ir_debug);	
			}
		}
		break;

		case ALI_FP_SET_SOCKPORT:
		{				
			if (0 != (copy_from_user(&ir_rfk_port, (void*)param, sizeof(ir_rfk_port))))
			{
				return -EFAULT;
			}
			if (0 != ir_debug)
			{
				ALI_IR_PRINT("[ %s ]: ir_rfk_port = %d\n", __FUNCTION__, ir_rfk_port);	
			}
		}
		break;	

		case ALI_FP_SET_REPEAT_INTERVAL:
		{				
			unsigned long rep[2] = {0, 0};
			
			if (0 != (copy_from_user(&rep, (void*)param, sizeof(rep))))
			{
				return -EFAULT;
			}

			ir_repeat_delay_rfk = rep[0];
			ir_repeat_interval_rfk = rep[1];
			
			if (0 != ir_debug)
			{
				ALI_IR_PRINT("[ %s ]: ir_repeat_delay_rfk = %ld\n", __FUNCTION__, ir_repeat_delay_rfk);	
				ALI_IR_PRINT("[ %s ]: ir_repeat_interval_rfk = %ld\n", __FUNCTION__, ir_repeat_interval_rfk);	
			}
		}
		break;
		
		
		default:
			rlt = -EPERM;
			break;
	}

end:
	return rlt;
}

static const struct file_operations ali_ir_fops = {
	.owner		= THIS_MODULE,
	.open		= ali_ir_open,	
	.release		= ali_ir_release,
	#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)
	.unlocked_ioctl = ali_ir_ioctl,
	#else
	.ioctl			= ali_ir_ioctl,
	#endif
};

static struct miscdevice ali_ir_misc = {
	.fops		= &ali_ir_fops,
	.name		= ALI_IRC_DEVICE_NAME,
	.minor		= MISC_DYNAMIC_MINOR,
};


/* Driver Initialization */
static int __devinit ali_ir_init(void) 
{
	int ret = 0;	

	
	if(0 != ir_debug){
		ALI_IR_PRINT("[ %s ] ", __FUNCTION__);
	}

	ret = misc_register(&ali_ir_misc);
	if (ret != 0) {
		ALI_IR_PRINT(KERN_ERR " cannot register ali ir miscdev(err=%d)\n", ret);
		goto fail_misc;
	}

	ali_ir_input = input_allocate_device();
	if (!ali_ir_input) {
		ALI_IR_PRINT(KERN_ERR " not enough memory for ali ir input device\n");
		ret = -ENOMEM;
		goto fail_input_alloc;
	}

	ali_ir_input->name = "ali_ir";
	ali_ir_input->phys = "ali_ir/input0";
	ali_ir_input->id.bustype = BUS_HOST; //BUS_I2C;
	ali_ir_input->id.vendor = 0x0001;
	ali_ir_input->id.product = 0x0001;
	ali_ir_input->id.version = 0x0100;
	__set_bit(EV_KEY, ali_ir_input->evbit);
	__set_bit(EV_REP, ali_ir_input->evbit);
	ret = input_register_device(ali_ir_input);
	if (ret){
		ALI_IR_PRINT(KERN_ERR " fail register for ali ir input device\n");
		goto fail_input_reg;
	}
	
	ali_ir_input->rep[REP_DELAY] = INTV_REPEAT_FIRST;
	ali_ir_input->rep[REP_PERIOD] = INTV_REPEAT;	
	if(0 != ir_debug){
		ALI_IR_PRINT(" OK!\n");
	}
	g_ir_state_machine = IR_STATE_INITED;
	ali_ir_hw_ip_identify();
	memset((void *)(&g_key_map), 0, sizeof(g_key_map));
	memset((void *)(&standby_key), 0x00, sizeof(standby_key));
	return 0;

fail_input_reg:		
	input_free_device(ali_ir_input);
fail_input_alloc:	
	misc_deregister(&ali_ir_misc);
fail_misc:
	return ret;
}

static void __exit ali_ir_exit(void)
{
	if(0 != ir_debug){
		ALI_IR_PRINT("%s: ", __FUNCTION__);
	}
	if(g_ir_state_machine&IR_STATE_OPENED){
		ALI_IR_PRINT(KERN_ERR " ali ir not closed\n");
		ali_ir_release(0, 0);
	}
	input_unregister_device(ali_ir_input);
	input_free_device(ali_ir_input);
	ali_ir_input = NULL;
	misc_deregister(&ali_ir_misc);
	if(g_key_map.map_entry)
		kfree(g_key_map.map_entry);
	g_ir_state_machine = 0;
	if(0 != ir_debug){
		ALI_IR_PRINT(" OK!\n");
	}
}


module_init(ali_ir_init);
module_exit(ali_ir_exit);

MODULE_AUTHOR("Goliath Peng");
MODULE_DESCRIPTION("ALi IR Driver");
MODULE_LICENSE("GPL");

