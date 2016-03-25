/*****************************************************************************
 *    Ali Corp. All Rights Reserved. 2013 Copyright (C)
 *
 *    Driver for ALi HEC
 *
 *    File:
 *
 *    Description:
 *         
 *    History:
 *           Date            Athor        Version        Reason
 *       ============    =============   =========   =================
 *   1.  July.15.2013      corey@sct      Ver 0.1       Create file.
 *
 *   Copyright 2013 ALi Limited
 *   Copyright (C) 2013 ALi Corp.
 * *****************************************************************************/

#include <linux/version.h>
#include <asm/uaccess.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
#include <linux/slab.h>
#endif

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
#include <linux/workqueue.h>
#include <asm/irq.h>
#include <linux/i2c.h>
#include <linux/list.h>
#include <linux/version.h>

#include <asm/mach-ali/typedef.h>
#include <ali_reg.h>

#define POLL_HDMI_STATUS
//#define USING_3LINE_I2C
#if 1
#define PRINTK_HEC        printk
#else
#define PRINTK_HEC(...)   do{} while(0)
#endif

#ifdef POLL_HDMI_STATUS
struct ali_hec_poll {
	struct workqueue_struct *wqueue;
	struct work_struct work;
};
#else
#define ALI_HEC_IRQ              XX
#define ALI_HEC_DRIVER_NAME      "ali_hec"
#define ALI_HEC_GPIO_CONN_PIN    XX
#define ALI_HEC_GPIO_DISC_PIN    XX

struct ali_hec_port {
	int irq;
	int gpio_nu;
	irqreturn_t (*handler)(int nu, void *param);
};
#endif

#define SOC_GET_BYTE(i)            __REG8ALI(i)        //(*(volatile unsigned char *)(i))
#define SOC_SET_BYTE(i,d)          __REG8ALI(i) = (d)  //(*(volatile unsigned char *)(i)) = (d)
#define SOC_GET_LONG(i)            __REG32ALI(i)
#define SOC_SET_LONG(i,d)          __REG32ALI(i) = (d)

#define PHYS_SOC_BASE_ADDR         (0x18000000)
#define PHYS_HDMI_BASE_ADDR        (0x1802A000)
#define PHYS_I2C3_BASE_ADDR        (0x18018D00)

#define SUCCESS       0
#define HEC_I2C_ID    3
#define HEC_I2C_ADDR  0x70

#define CBB_REG 0
#define CBO_REG 1
#define LHA_REG 2
#define DTB_REG 3

extern int ali_i2c_scb_write_read(unsigned int id, unsigned char slv_addr, unsigned char *data, int wlen, int rlen);
extern int ali_i2c_scb_write(unsigned int id, unsigned char slv_addr, unsigned char *data, int len);

static int hybrid_read(unsigned char iTypeId, unsigned char iAdr, unsigned char bRegAdr, unsigned char *pData, unsigned char bLen)
{
	int bRet;

	pData[0] = bRegAdr;

#ifndef USING_3LINE_I2C
	bRet = ali_i2c_scb_write_read(iTypeId, iAdr, pData, 1, bLen);
#else
	bRet = HEC_READ(iAdr, pData, 1, bLen);
#endif
	return bRet;
}

static int hybrid_write(unsigned char iTypeId, unsigned char iAdr, unsigned char bRegAdr, unsigned char *pData, unsigned char bLen)
{
	unsigned char bTemp[bLen+1];
	int i;

	bTemp[0] = bRegAdr;
	for (i=1; i<=bLen; i++)
	{
		bTemp[i] = pData[i-1];
	}
#ifndef USING_3LINE_I2C
	return ali_i2c_scb_write(iTypeId, iAdr, bTemp, bLen + 1);
#else
	return HEC_WRITE(iAdr, bTemp, bLen+1);
#endif
}

static int powerdown_hybrid(unsigned char i2c_type_id, unsigned char i2c_addr)
{
	int re;
	unsigned char data[8];
	/*step 1: power up*/
	PRINTK_HEC("[HEC] power down hybrid\n");
	re = hybrid_read(i2c_type_id, i2c_addr, CBB_REG, data, 2);
	if (re != SUCCESS)
	{
		PRINTK_HEC("  read CBB err\n");
		return -1;
	}   
	data[1] &= ~(1<<1);  //VBGEN
	data[1] &= ~(1<<0);  //CALEN
	re = hybrid_write(i2c_type_id, i2c_addr, CBB_REG, data, 2);
	if (re != SUCCESS)
	{
		PRINTK_HEC("  write CBB err\n");
		return -1;
	}
	/*step 2: LHA clear*/
	data[0] = 0;
	data[1] = 0;
	re = hybrid_write(i2c_type_id, i2c_addr, LHA_REG, data, 2);
	if (re != SUCCESS)
	{
		PRINTK_HEC("  write LHA err\n");
		return -1;
	}
	return 0;
}

extern void hectrl_bymac(u32 regpos, u16 value);
static int powerup_hecphy(void)
{
	unsigned short data;
	PRINTK_HEC("\n[HEC] power up mac\n");
	data = 0x00;
	data |= (0x21<<8);
	hectrl_bymac(0, data);

	return 0;
}

static int powerdown_hecphy(unsigned char i2c_type_id, unsigned char i2c_addr)
{
	unsigned short data;
	PRINTK_HEC("\n[HEC] power down mac\n");
	data = 0x00;
	data |= (0x29<<8);
	hectrl_bymac(0, data);

	powerdown_hybrid(i2c_type_id, i2c_addr);
	return 0;
}

static int calibra_hybrid(unsigned char i2c_type_id, unsigned char i2c_addr)
{
	int re;
	unsigned char data[8];
	/*step 1: power up*/
	PRINTK_HEC("[HEC] step 1: power up\n");
	re = hybrid_read(i2c_type_id, i2c_addr, CBB_REG, data, 2);
	if (re != SUCCESS)
	{
		PRINTK_HEC("  read CBB err\n");
		return -1;
	}
	data[1] |= (1<<1);  //VBGEN
	data[1] |= (1<<0);  //CALEN
	re = hybrid_write(i2c_type_id, i2c_addr, CBB_REG, data, 2);
	if (re != SUCCESS)
	{
		PRINTK_HEC("  write CBB err\n");
		return -1;
	}
	/*setp 2: read DTB*/
	PRINTK_HEC("[HEC] step 2: read DTB\n");
	re = hybrid_read(i2c_type_id, i2c_addr, DTB_REG, data, 2);
	if (re != SUCCESS)
	{
		PRINTK_HEC("  read DTB err\n");
		return -1;
	}
	if (0x01 != (data[1]&0x03))
	{
		PRINTK_HEC("  data[1] data[0] %02x %02x\n", data[1], data[0]);
		PRINTK_HEC("  CALERR: watch CALEND bit:0-1\n");
		//return -1;
	}
	/*step 3: read CBO*/
	PRINTK_HEC("[HEC] step 3: read CBO\n");
	re = hybrid_read(i2c_type_id, i2c_addr, CBO_REG, data, 2);
	if (re != SUCCESS)
	{
		PRINTK_HEC("  read CBO err\n");
		return -1;
	}
	if (!(data[1]&0x08))
	{
		PRINTK_HEC("  data[1] data[0] %02x %02x\n", data[1], data[0]);
		PRINTK_HEC("  watch VBGOK bit:7\n");
		return -1;
	}
	/*step 4: set VBGEN*/
	PRINTK_HEC("[HEC] step 4: set VBGEN\n");
	re = hybrid_read(i2c_type_id, i2c_addr, CBB_REG, data, 2);
	if (re != SUCCESS)
	{
		PRINTK_HEC("  read CBB err2\n");
		return -1;
	}
	data[1] |= (1<<1);  //VBGEN
	re = hybrid_write(i2c_type_id, i2c_addr, CBB_REG, data, 2);
	if (re != SUCCESS)
	{
		PRINTK_HEC("  write CBB err2\n");
		return -1;
	}
	/*step 5: target operation mode*/
	PRINTK_HEC("[HEC] step 5: target op mode\n");
	/*re = hybrid_read(i2c_type_id, i2c_addr, LHA_REG, data, 2);
	if (re != SUCCESS)
	{
	    PRINTK_HEC("[HEC] read LHA err\n");
	    return -1;
	}*/
	data[0] = 0;
	data[1] = 0x07;
	re = hybrid_write(i2c_type_id, i2c_addr, LHA_REG, data, 2);
	if (re != SUCCESS)
	{
		PRINTK_HEC("  write LHA err\n");
		return -1;
	}

	PRINTK_HEC("[HEC] calibration DONE!\n");
	return 0;
}

static int detect_hdmi_conn(void)
{
	unsigned char data;
	//read hdmi status
	//system_reg_read(0x2A008, &data, 1);
	data = SOC_GET_BYTE(PHYS_HDMI_BASE_ADDR+8);
	return (data&0x01);
	/*if (data&0x01)
	{
		PRINTK_HEC("[DE] hdmi detect bit set\n");
		return 1;
	}
	PRINTK_HEC("[DE] hdmi NOT link\n");
	return 0*/;
}

static int cdccec_negotiation(void)
{
	PRINTK_HEC("[HEC] hdmi CDC/CEC negotiation(fake)...\n");
	return 1;
}

static int enable_hec_work(unsigned char i2c_type_id, unsigned char i2c_addr)
{
	//if (detect_hdmi_conn())
	{
		if (cdccec_negotiation())
		{
			powerup_hecphy();
			calibra_hybrid(i2c_type_id, i2c_addr);
			return 0;
		}
	}
	return -1;
}

#ifdef POLL_HDMI_STATUS
struct ali_hec_poll g_hec_poll;
int g_hec_inited = 0;

static void ali_hec_poll_thread(struct work_struct *work)
{
	while (1)
	{
		if (detect_hdmi_conn())
		{
			if (!g_hec_inited)
			{
				enable_hec_work(HEC_I2C_ID, HEC_I2C_ADDR);
				g_hec_inited = 1;
			}
		}
		else
		{
			if (g_hec_inited)
			{
				powerdown_hecphy(HEC_I2C_ID, HEC_I2C_ADDR);
				g_hec_inited = 0;
			}
		}
		msleep(2000);
	}
}
#else
static irqreturn_t ali_hec_conn_interrupt(int irq, void *dev_id)
{
	if (1 != get_gpio_interrupt_status((((struct ali_hec_port *)dev_id)->gpio_nu)+128))  //belong to the 4th gpio
	{
		return IRQ_HANDLED;
	}

	printk("hec found...\n");
	enable_hec_work(HEC_I2C_ID, HEC_I2C_ADDR);
	return IRQ_HANDLED;
}

static irqreturn_t ali_hec_disc_interrupt(int irq, void *dev_id)
{
	if (1 != get_gpio_interrupt_status((((struct ali_hec_port *)dev_id)->gpio_nu)+128))  //belong to the 4th gpio
	{
		return IRQ_HANDLED;
	}

	printk("hec remove...\n");
	powerdown_hecphy(HEC_I2C_ID, HEC_I2C_ADDR);
	return IRQ_HANDLED;
}

static struct ali_hec_port g_hec_port[2] = {
	{ALI_HEC_IRQ, ALI_HEC_GPIO_CONN_PIN, ali_hec_conn_interrupt},
	{ALI_HEC_IRQ, ALI_HEC_GPIO_DISC_PIN, ali_hec_disc_interrupt},
};

int ali_hecirq_request(struct ali_hec_port *port)
{
	int ret;
    /*
     * Allocate the IRQ
     */
	ret = request_irq(port->irq, port->handler, IRQF_SHARED, ALI_HEC_DRIVER_NAME, port);
	if (ret) {
		printk("[ERR] ali_hec: Can't get irq\n");
	}
	return ret;
}
#endif

static int ali_hec_startup(void)
{
	int ret;
    
	SOC_SET_BYTE((PHYS_I2C3_BASE_ADDR+0x2D), 0x01);
	SOC_SET_BYTE((PHYS_I2C3_BASE_ADDR+0x2C), (HEC_I2C_ADDR>>1));
	printk("[HEC] i2c3 trig: %d, set addr is 0x%02x\n", SOC_GET_BYTE(PHYS_I2C3_BASE_ADDR+0x2D), SOC_GET_BYTE(PHYS_I2C3_BASE_ADDR+0x2C));
#ifdef POLL_HDMI_STATUS
	g_hec_poll.wqueue = create_workqueue("hec");
	if (!(g_hec_poll.wqueue))
	{
		PRINTK_HEC("[ERR] Failed to allocate work queue\n");
		ret =  -1;
	}

	INIT_WORK(&g_hec_poll.work, ali_hec_poll_thread);
	queue_work(g_hec_poll.wqueue, &g_hec_poll.work);
#else
	ret = ali_hecirq_request(&g_hec_port[0]);
	ret |= ali_hecirq_request(&g_hec_port[1]);
#endif
	return ret;
}

static void ali_hec_cleanup(void)
{
#ifdef POLL_HDMI_STATUS
	destroy_workqueue(g_hec_poll.wqueue);
#else
	free_irq(g_hec_port[0].irq, &g_hec_port[0]);
	free_irq(g_hec_port[1].irq, &g_hec_port[1]);
#endif
}

/*
static void hdmireginit(void)
{
	printk("----------------hdmireginit----------------\n");
    SOC_SET_BYTE((PHYS_SOC_BASE_ADDR+0x78), 0x22);
	printk("0\n");
    SOC_SET_BYTE((PHYS_SOC_BASE_ADDR+0x6E), 0x03);
	printk("1\n");
    SOC_SET_BYTE((PHYS_SOC_BASE_ADDR+0x6E), 0x01);
	printk("2\n");
    SOC_SET_BYTE((0x18018000+0xD2C), 0x55);
	printk("3\n");
    SOC_SET_LONG((0x18018000+0xD04), 0x3C3C00AA);
	printk("4\n");
    SOC_SET_LONG((0x18018000+0xD08), 0x30303030);
	printk("5\n");
    SOC_SET_LONG((0x18018000+0xD00), 0xFF0100C0);
	printk("6\n");
    SOC_SET_BYTE((0x18018000+0xD0C), 0x83);
	printk("7\n");
    SOC_SET_BYTE((0x18018000+0xD10), 0x06);
	printk("8\n");
    SOC_SET_BYTE((0x18018000+0xD10), 0x01);
	printk("9\n");
    SOC_SET_BYTE((0x18018000+0xD10), 0x40);
	printk("10\n");
    SOC_SET_LONG((0x18018000+0xD00), 0xFF0100C1);
	printk("11\n");
    SOC_SET_BYTE((PHYS_SOC_BASE_ADDR+0x6E), 0x00);
	printk("12\n");
    SOC_SET_BYTE((PHYS_SOC_BASE_ADDR+0x6E), 0x04);
	printk("13\n");
    SOC_SET_BYTE((PHYS_HDMI_BASE_ADDR+0x99), 0x8B);
	printk("14\n");
    SOC_SET_BYTE((PHYS_HDMI_BASE_ADDR+0x96), 0x03);
	printk("15\n--------------------------------\n");
}
*/

static int __init ali_hec_init(void)
{
	//hdmireginit();
	return ali_hec_startup();
}

static void __exit ali_hec_exit(void)
{
	ali_hec_cleanup();
}

module_init(ali_hec_init);
module_exit(ali_hec_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Corey");
MODULE_DESCRIPTION("Ali HEC driver");

