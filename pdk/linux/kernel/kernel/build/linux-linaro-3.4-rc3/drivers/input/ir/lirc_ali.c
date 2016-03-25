/*
 * =====================================================================================
 *
 *       Filename:  lirc_ali.c
 *
 *    Description:  device driver for LIRC 
 *
 *        Version:  1.1
 *        Created:  09/28/2010 09:40:47 AM
 *       Revision:  none
 *       Compiler:  mips-linux-gcc
 *
 *         Author:  Yilin/Terry Lee
 *        Company:  Ali
 *
 * =====================================================================================
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <linux/version.h>
#include <linux/module.h>
//#include <linux/autoconf.h>
#include <generated/autoconf.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/serial_reg.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <asm/system.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/fcntl.h>
#else
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/fcntl.h>
#endif

#include <linux/sched.h>
#include <linux/time.h>
#include <linux/timer.h>
//#include <linux/wrapper.h>

#if defined(CONFIG_ALI_CHIP_M3921)
#include <ali_interrupt.h>
#else
#include <asm/mach-ali/m36_irq.h>
#endif

#include "./lirc_dev/lirc.h"
#include "./lirc_dev/lirc_dev.h"


#define IR_CHECK_STANDBY_KEY

#ifdef IR_CHECK_STANDBY_KEY
#include <linux/input.h>      /*  For input sub system */
#include <linux/miscdevice.h>
#endif

#include <ali_reg.h>


#define ALI_IRC_BASE				0x18018100 /* Memory base for ALI_M3602_IRC */
#define INFRA_IRCCFG				(ALI_IRC_BASE + 0x00)
#define INFRA_FIFOCTRL				(ALI_IRC_BASE + 0x01)
#define INFRA_TIMETHR				(ALI_IRC_BASE + 0x02)
#define INFRA_NOISETHR				(ALI_IRC_BASE + 0x03)
#define INFRA_IER					(ALI_IRC_BASE + 0x06)
#define INFRA_ISR					(ALI_IRC_BASE + 0x07)
#define INFRA_RLCBYTE				(ALI_IRC_BASE + 0x08)

#if defined(CONFIG_ALI_CHIP_M3921)
static unsigned long g_ir_irq_num = INT_ALI_IRC;
#else
static unsigned long g_ir_irq_num = M36_SYS_IRQ_BASE + 19;
#endif

#define IR_RLC_SIZE					256
#define VALUE_CLK_CYC				8		/* Work clock cycle, in uS */
//richard
#define VALUE_TOUT					24000    // 9500	/* Timeout threshold, in uS */
#define VALUE_NOISETHR				80     // 400		/* Noise threshold, in uS */

#define IRC_EN                      (1 << 7) 
#define INT_STATE_FIFO              (1 << 0) 
#define INT_STATE_TIMEOUT           (1 << 1) 
#define FIFO_RESET                  (1 << 7)
#define FIFO_THRESHOLD              (32)

#define PULSE_LIRC_T_BIT  0x01000000
#define PULSE_LIRC_T_MASK 0x00FFFFFF
#define PULSE_INFRA_T_BIT  0x80
#define PULSE_INFRA_T_MASK 0x7F

#define output_level   "<1>"
#define debug_out printk

#define READ_INF_BYTE(addr)			(__REG8ALI(addr))
#define WRITE_INF_BYTE(addr,data)	(__REG8ALI(addr) = data)

#ifdef IR_CHECK_STANDBY_KEY
#define IR_KEY_RELEASE			0
#define IR_KEY_PRESSED			1
#endif

static unsigned char bufferin = 0;
static unsigned char bufferout = 0;
static unsigned char rx_tail = 0;
static unsigned char rx_head = 0;
static unsigned char infra_buffer[IR_RLC_SIZE];
static lirc_t rx_buf[IR_RLC_SIZE];

#ifdef IR_CHECK_STANDBY_KEY
struct input_dev *g_lirc_input = NULL;
#endif
static DECLARE_WAIT_QUEUE_HEAD(lirc_read_queue);
spinlock_t lock;

/****************Function declare*********************/
static int ir_open(struct inode *inode, struct file *file);
static ssize_t ir_read(struct file *file, char *buf, size_t count, loff_t * ppos);
static ssize_t ir_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static int ir_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);
#else
static int ir_ioctl(struct inode *node, struct file *filep, unsigned int cmd, unsigned long arg);
#endif
static unsigned int ir_poll(struct file *file, struct poll_table_struct *pt);
static int ir_release(struct inode *inode, struct file *file);

static void read_fifo(void);
static void Irbuf_convert_to_lirctbuf(void);
static void add_read_queue(lirc_t val);

static irqreturn_t ali_m36_ir_interrupt(int irq, void *dev_id);



static struct file_operations ir_fops = {
    .open    = ir_open,
    .read    = ir_read,
	.write   = ir_write,
	#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	.unlocked_ioctl = ir_ioctl,
	#else
    .ioctl   = ir_ioctl,
    #endif
	.poll    = ir_poll,
    .release = ir_release,
};

static unsigned char lirc_debug = 0;


#define ALI_IR_PRINK(fmt, args...)			\
{										\
	if (0 !=  lirc_debug)					\
	{									\
		printk(fmt, ##args);					\
	}									\
}

#define ALI_IR_ERR_PRINK(fmt, args...)		\
{										\
	printk(fmt, ##args);						\
}


static int set_use_inc(void *data)
{	
	return 0;
}

static void set_use_dec(void *data)
{
}

static int add_to_buf(void *data, struct lirc_buffer *buf)
{	
	return 0;
}

static struct lirc_driver driver = {
    .name            = "lirc",
    .minor           = -1,
    .code_length     = 1,
    .sample_rate     = 0,  
	.data            = NULL,
    //.get_queue       = NULL,
    .add_to_buf      = add_to_buf,
	.set_use_inc     = set_use_inc,
	.set_use_dec     = set_use_dec,
    .fops            = &ir_fops,
    .owner           = THIS_MODULE,
};
/*
 *	ali_init_irc
 * 	Configure the IRC register.
 *
 */
static void __devinit ali_init_irc(void)
{	
	/* Working clock expressions:
	 * (SB_CLK / (32 * CLK_SEL)) = 1 / VALUE_CLK_CYC, SB_CLK = 12MHz
	 * => CLK_SEL = (SB_CLK * VALUE_CLK_CYC / 32)
	 */	
	WRITE_INF_BYTE(INFRA_IRCCFG, 0);	
	WRITE_INF_BYTE(INFRA_IRCCFG, IRC_EN | ((12 * VALUE_CLK_CYC) >> 5));	

	/* FIFO threshold */
	WRITE_INF_BYTE(INFRA_FIFOCTRL, FIFO_RESET | FIFO_THRESHOLD);	

	/* Timeout threshold expressions:
	 * ((TIMETHR + 1) * 128 * VALUE_CLK_CYC) = VALUE_TOUT
	 * => TIMETHR = (VALUE_TOUT / (128 * VALUE_CLK_CYC)) - 1
        	g_current_led_standby_gpio_addr_pin.key3_standby_gpio_pin ,\
	 */
	WRITE_INF_BYTE(INFRA_TIMETHR, (VALUE_TOUT / (VALUE_CLK_CYC << 7) - 1));	

	/* Noise pulse timeout expressions:
	 * Value = VALUE_NOISETHR / VALUE_CLK_CYC
	 */
	WRITE_INF_BYTE(INFRA_NOISETHR, VALUE_NOISETHR / VALUE_CLK_CYC);	

	/* Ensure no pending interrupt */
	WRITE_INF_BYTE(INFRA_ISR, INT_STATE_TIMEOUT | INT_STATE_FIFO);  	

	/* Enable IRC Interrupt: timeout and fifo full */
	WRITE_INF_BYTE(INFRA_IER, INT_STATE_TIMEOUT | INT_STATE_FIFO);  	

	return;
}


static int  ali_ir_probe(struct platform_device *pdev)
{	
	printk(output_level"can come to the probe");	
	debug_out(output_level "Driver module initialize ok.\n");
	return 0;
}
static int ali_ir_remove(struct platform_device *pdev)
{	
	debug_out(output_level "Ali ir driver exit.\n");
	return 0;
}

static int ali_ir_resume(struct platform_device *pdev)
{
	//printk(output_level"we come to the ir resume!!\nwe init the ir again!!\n");
	static int ret;

	
	bufferin = bufferout = 0;
	rx_tail = 0;
	rx_head = 0;
	ret = 0;
	memset(infra_buffer, 0, IR_RLC_SIZE);
	memset(rx_buf, 0, IR_RLC_SIZE);
	ali_init_irc();
	ret = request_irq(g_ir_irq_num, ali_m36_ir_interrupt, IRQF_DISABLED, "ali_m36_ir", NULL);
	if (ret < 0) {
		printk(KERN_ERR "Failed to register ali_m36_ir interrupt %d \n", g_ir_irq_num);
		free_irq(g_ir_irq_num, NULL);
        return ret;
    }
   return 0;
}
static int ali_ir_suspend(struct platform_device *pdev, pm_message_t state)
{	
	printk(output_level"come to the ir suspend!!\nhere we shutdown the ir interuput!!\n");
	/* disable IR timeout & fifo interrupt */
	WRITE_INF_BYTE(INFRA_ISR, 0x03);     
	WRITE_INF_BYTE(INFRA_IER, 0x00);      

	free_irq(g_ir_irq_num, NULL);
	return 0;
}
static struct platform_driver ali_ir_driver =
{
	.driver		= {
		.name	= driver.name,
		.owner	= THIS_MODULE,
	},
	.probe		= ali_ir_probe,
	.remove		= ali_ir_remove,
	.suspend	= ali_ir_suspend,
	.resume		= ali_ir_resume,
};


static struct  platform_device ali_ir_device ={
	.name		= driver.name,
	.id		= 0,
	.dev = {
		.driver = &ali_ir_driver.driver,
	}
};


static void add_read_queue(lirc_t val)
{
	unsigned char new_rx_tail;

	
	new_rx_tail = (rx_tail + 1) & (IR_RLC_SIZE - 1);
	if (new_rx_tail == rx_head) {
		/*
		*  rx_buf[] overflow, in most case means upper layer
		*  read is stopped, just ignore or read again.
		*/
		return;
	}
	
	rx_buf[rx_tail] = val;
	rx_tail = new_rx_tail;
	
	wake_up_interruptible(&lirc_read_queue);
}


static void read_fifo(void)
{
	volatile unsigned char num;

	
	num = READ_INF_BYTE(INFRA_FIFOCTRL) & PULSE_INFRA_T_MASK;
	while (num-- > 0) {
		infra_buffer[bufferin++] = READ_INF_BYTE(INFRA_RLCBYTE);
		bufferin &= (IR_RLC_SIZE - 1);
	};
}


#ifdef IR_CHECK_STANDBY_KEY
static void check_standby_key(struct input_dev * input, unsigned char data)
{
	static unsigned int standby_cnt = 0;	
	static unsigned char standby_flag = 0;
	static unsigned long low_tick = 0;
	unsigned long tick = 0;
	unsigned long width;


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
			ALI_IR_PRINK("[ %s %d],  low_tick = %ld, tick = %ld, width = %ld\n", __FUNCTION__, __LINE__, 
				low_tick, tick, width);	
			if (width >= 800)
			{
				goto SEND_STANDBY_KEY;
			}
		}
	}

	//ALI_IR_PRINK("[ %s %d ], data = 0x%02x, standby_cnt = %d\n", __FUNCTION__, __LINE__, data, standby_cnt);

	if (standby_cnt >= (VALUE_TOUT/VALUE_CLK_CYC/0x7f))	/* detect low count */
	{
		standby_flag = 1;
		low_tick = jiffies;
		ALI_IR_PRINK("[ %s %d ],  low_tick = %ld standby_cnt = %d\n", __FUNCTION__, __LINE__, low_tick, standby_cnt);		
	}
	
	return ;

SEND_STANDBY_KEY:	
	
	input_report_key(input, KEY_HOME, IR_KEY_PRESSED);							
	input_sync(input);
	input_report_key(input, KEY_HOME, IR_KEY_RELEASE);
	input_sync(input);		
	
	ALI_IR_ERR_PRINK("[ %s %d ], key 0x%08x pressed, width = %ld\n", __FUNCTION__, __LINE__, KEY_HOME, width);	
		
}
#endif


/*	
 *   This function convert IR raw data to lirc_t data.
 *   infra_buffer[] --> rx_buf[]
 *
 *   RLC data format:
 *   7 bit:	polarity of the signal.
 *	 0-6 bits:length of the signal in us.
 *
 *   LIRC_T data format:
 *	 24 bit:polarity of the signal.
 *	 0-23 bits:	length of the signal in us
 *
 */
static void Irbuf_convert_to_lirctbuf(void)
{
	lirc_t val = 0;
	unsigned long pulse_width = 0;	
	unsigned char data;	

	
	while (bufferin != bufferout) {  
		data = infra_buffer[bufferout];
		//printk("\n-----infra_buffer[bufferout]=%c-----\n",infra_buffer[bufferout]);

		if (!(data & PULSE_INFRA_T_BIT)) {              /* Data is a pulse  */
			val |= PULSE_LIRC_T_BIT;
		}
		
		pulse_width += ((data & PULSE_INFRA_T_MASK) * VALUE_CLK_CYC); 	
		bufferout = ((bufferout + 1) & (IR_RLC_SIZE - 1));		

		#ifdef IR_CHECK_STANDBY_KEY
		check_standby_key(g_lirc_input, data);
		#endif

		/* Long pulse */
		if (((!((data ^ infra_buffer[bufferout]) & PULSE_INFRA_T_BIT)) && (bufferout != bufferin))) { 
			continue;
		}

		/* software denoise, eg 0x0 0x01000000 etc.*/
		if ((pulse_width & PULSE_LIRC_T_MASK) == 0) { 
			continue;
		}

		if (bufferout == bufferin) {
			break;
		}

		val |= (pulse_width & PULSE_LIRC_T_MASK);
		//printk("\n======val = 0x%x=====\n",val);
		add_read_queue(val);
		
		data = 0;
		val = 0;
		pulse_width = 0;
	}
}

static long delta(struct timeval *tv1, struct timeval *tv2)
{
	 unsigned long deltv;

 	
	 deltv = tv2->tv_sec - tv1->tv_sec;
	 if (deltv > 15) {
		 deltv = 0xFFFFFF;
	 } else {
		 deltv = deltv * 1000000 + tv2->tv_usec - tv1->tv_usec;
	 }

	 return deltv;
}


 /*
  *	ali_m36_ir_interrupt
  * IR Interrupt Handler
  *
  */
static irqreturn_t ali_m36_ir_interrupt(int irq, void *dev_id)
{
	volatile unsigned char int_status;
	static struct timeval last_tv;
	struct timeval curr_tv;
	unsigned long deltv;
	lirc_t val;
	static unsigned char last_int_status = INT_STATE_TIMEOUT;
	unsigned long acc_repeat_width = 9000 + 2250 + 560;  /*leading code and repeat pulse*/

	//spin_lock(&lock);	
	int_status = READ_INF_BYTE(INFRA_ISR) & 0x03; 
	WRITE_INF_BYTE(INFRA_ISR, int_status);
	
	switch (int_status) {
	case INT_STATE_TIMEOUT:
		if (INT_STATE_TIMEOUT == last_int_status) {    /* Repeat key case */
			do_gettimeofday(&curr_tv);
			deltv = delta(&last_tv, &curr_tv);
			deltv -= acc_repeat_width;
			val = deltv & PULSE_LIRC_T_MASK;
			add_read_queue(val);		
		}
		read_fifo();   			        	/* Fetch data to IR buffer  */
		Irbuf_convert_to_lirctbuf();	    /* Convert IR buffer to LIRC_T buf  */
		do_gettimeofday(&last_tv);
		break;

	case INT_STATE_FIFO:
		if (INT_STATE_TIMEOUT == last_int_status) {   /* New key case */
			do_gettimeofday(&curr_tv);
			deltv = delta(&last_tv, &curr_tv);
			val = deltv & PULSE_LIRC_T_MASK;
			add_read_queue(val);
		}
		read_fifo();   				        /* Fetch data to IR buffer  */
		break;

	default:
		break;
	}

	last_int_status = int_status;
	//spin_unlock(&lock);

	return IRQ_HANDLED;
}


static int ir_open(struct inode *inode, struct file *file)
{	
	bufferin = bufferout = 0;
	rx_head = rx_tail = 0;

	ali_init_irc();
	spin_lock_init(&lock);

	return 0;
}


static int ir_release(struct inode *inode, struct file *file)
{	
	return 0;
}


static ssize_t ir_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	int n = 0;
	int retval = 0;	
	

	while (n < count) {
		if ((file->f_flags & O_NONBLOCK) && (rx_head == rx_tail)) {
			retval = -EAGAIN;
			break;
		}
		retval = wait_event_interruptible(lirc_read_queue, rx_head!=rx_tail);
		if (retval) {
	  	  break;
		}
		if (copy_to_user((void *)buf + n, (void *)(rx_buf + rx_head), sizeof(lirc_t))) {
			retval = -EFAULT;
			break;
		}
		rx_head = (rx_head + 1) & (IR_RLC_SIZE - 1);
		n += sizeof(lirc_t);
	}
	 
	if (n) {
		return n;
	}
	
	return retval;
}


ssize_t ir_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{	
	return 0;
}


unsigned int ir_poll(struct file *file, struct poll_table_struct *pt)
{	
	poll_wait(file, &lirc_read_queue, pt);
	if (rx_tail != rx_head) {
		return POLLIN | POLLRDNORM;
	}
	return 0;
}


#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static int ir_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
#else
static int ir_ioctl(struct inode *node, struct file *filep, unsigned int cmd, unsigned long arg)
#endif
{
	s32 retval = 0;
	u32 value = 0;	

	
	if (cmd == LIRC_GET_FEATURES) {
		value = LIRC_CAN_SEND_PULSE | LIRC_CAN_REC_MODE2;
	} else if (cmd == LIRC_GET_SEND_MODE) {
		value = LIRC_MODE_PULSE;
	} else if (cmd == LIRC_GET_REC_MODE) {
		value = LIRC_MODE_MODE2;
	}

	switch (cmd) {
	case LIRC_GET_FEATURES:
	case LIRC_GET_SEND_MODE:
	case LIRC_GET_REC_MODE:
		retval = put_user(value, (unsigned long *)arg);
		break;
	case LIRC_SET_SEND_MODE:
	case LIRC_SET_REC_MODE:
		retval = get_user(value, (unsigned long *)arg);
		break;	
	default:
		retval = -ENOIOCTLCMD;
		break;
	}

	if (retval) {
		return retval;
	}
	if (cmd == LIRC_SET_REC_MODE) {
		if (value != LIRC_MODE_MODE2) {
			retval = -ENOSYS;
		}
	} else if (cmd == LIRC_SET_SEND_MODE) {
		if (value != LIRC_MODE_PULSE) {
			retval = -ENOSYS;
		}
	}

	return retval;
}


#ifdef IR_CHECK_STANDBY_KEY
static s32 ali_keypad_open(struct inode *inode, struct file *file)
{
    	ALI_IR_PRINK("%s\n", __func__);
    	return 0;
}


static s32 ali_keypad_release(struct inode *inode, struct file *file)
{
    	ALI_IR_PRINK("%s\n", __func__);
    	return 0;
}


static const struct file_operations ali_keypad_fops = {
    .owner = THIS_MODULE,      
    .open  = ali_keypad_open,
    .release = ali_keypad_release,
};


#define 		KEY_DEV_NAME                       		"ali_keypad_dev"
static struct miscdevice ali_keypad_misc = {
	.fops			= &ali_keypad_fops,
	.name		= KEY_DEV_NAME,
	.minor		= MISC_DYNAMIC_MINOR,
};
#endif


static int __init ir_init(void)
{
	int ret;

	
	if(0 != platform_driver_register(&ali_ir_driver))
	{
			printk(KERN_ERR"register driver error!!\n");
			return -1;
	}
	platform_device_register(&ali_ir_device);

	#ifdef IR_CHECK_STANDBY_KEY
	g_lirc_input = input_allocate_device();
	if (!g_lirc_input)
	{
		ret = -ENOMEM;
		ALI_IR_ERR_PRINK(KERN_ERR "ALi Panel HW Scan: not enough memory for input device\n");
		return ret;
	}

	g_lirc_input->name = "ali_keypad_standby";
    	g_lirc_input->phys = "ali_keypad_standby/input1";
	g_lirc_input->id.bustype = BUS_HOST; 
	g_lirc_input->id.vendor = 0x0001;
	g_lirc_input->id.product = 0x0002;
	g_lirc_input->id.version = 0x0100;
	
	__set_bit(EV_KEY, g_lirc_input->evbit);
	__set_bit(EV_REP, g_lirc_input->evbit);	

	/*  Set support key  */
   	 __set_bit(KEY_HOME & KEY_MAX, g_lirc_input->keybit);    	

	ret = input_register_device(g_lirc_input);
	if (ret)
	{
		input_free_device(g_lirc_input);
		ALI_IR_ERR_PRINK(KERN_ERR "ALi Panel HW Scan: input_register_device() failed!\n");
	}	
	#endif
	
	driver.minor = lirc_register_driver(&driver);
	if (driver.minor < 0) {
		printk(KERN_ERR "lirc_register_driver() failed! \n");
		return -EIO;
	}

	ret = request_irq(g_ir_irq_num, ali_m36_ir_interrupt, IRQF_DISABLED, "ali_m36_ir", NULL);
	if (ret < 0) {
		printk(KERN_ERR "Failed to register ali_m36_ir interrupt %d \n", g_ir_irq_num);
		free_irq(g_ir_irq_num, NULL);
		return ret;
	}

	init_waitqueue_head(&lirc_read_queue);

	#ifdef IR_CHECK_STANDBY_KEY
	ret = misc_register(&ali_keypad_misc);
	if (ret != 0) 
	{
		ALI_IR_ERR_PRINK(KERN_ERR "[ %s %d ], cannot register miscdev(err=%d)\n", 
			__FUNCTION__, __LINE__, ret);
		return ret;
	}	
	#endif
	

	return 0;
}


static void __exit ir_exit(void)
{	
	free_irq(g_ir_irq_num, NULL);
	
	#ifdef IR_CHECK_STANDBY_KEY
	input_unregister_device(g_lirc_input);
	input_free_device(g_lirc_input);
	g_lirc_input = NULL;	
	misc_deregister(&ali_keypad_misc);	
	#endif
	
	platform_device_unregister(&ali_ir_device);	
	platform_driver_unregister(&ali_ir_driver);
	lirc_unregister_driver(driver.minor);
}


module_init(ir_init);
module_exit(ir_exit);

MODULE_AUTHOR("ALi (Zhuhai) corperation");
MODULE_LICENSE("proprietary");
MODULE_VERSION("V1.0");


