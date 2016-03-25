/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2006 Copyright (C)
*
*    File:    cic_m3602.c
*
*    Description:    This file contains all globe micros and functions declare
*		             of M3602 Dual CI controler.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	July.17.2006      Joey Gao      Ver 0.1    Create file.
*
*****************************************************************************/

#include "ali_m36_cic.h"


#define ALI_CIC_DEVICE_NAME  "ali_m36_cic"

static struct cic_m36_private ali_m36_cic_private;
static ca_msg_t ali_ci_msg;
static struct mutex ali_ci_mutex;

/*
static INT32 cic_m3602_open(struct cic_device *dev, void (*callback)(int slot));
static INT32 cic_m3602_close(struct cic_device *dev);
static INT32 cic_m3602_ioctl(struct cic_device *dev, INT32 cmd, UINT32 param);
static INT32 cic_m3602_read(struct cic_device *dev, int slot, UINT16 size, UINT8 *buffer);
static INT32 cic_m3602_write(struct cic_device *dev, int slot, UINT16 size, UINT8 *buffer);
*/

/*
 * 	Name		:   cic_m3602_interrupt()
 *	Description	:   m3602 CI controler interrupt handle.
 *	Parameter	:	struct cic_device *dev	: Devcie handle
 *	Return		:	INT32				: return value
 *
 */
static irqreturn_t cic_m3602_interrupt(int irq, UINT32 param)
{
	struct cic_m36_private *priv=((struct ali_cic_device *)param)->priv;
	if(NULL==priv)
		return 0;
	UINT32 ioaddr = priv->base_addr;
	UINT8  status;
	INT32 i;
	UINT8 ci_slot_num = 2;

	do
	{
		for (i=0; i<ci_slot_num; i++)
		{
			status = (INPUT_UINT8(ioaddr + (i * 0x40) + R_CSCR) & 0x0c);
			CIC_PRINTF("%s %d, base:0x%08x, status:0x%02x\n",__FUNCTION__, irq, ioaddr, status);
			if ((status &= INPUT_UINT8(ioaddr + (i * 0x40) + R_MICR)) == 0)
			{
				continue;
			}

			/* Card detect changed interrupt */
			if ((status & 0x08) != 0)
			{
				switch (INPUT_UINT8(ioaddr + (i * 0x40) + R_IFXSR) & 0x0C)
				{
					case 0x0c:	/* Card insetted */						
						OUTPUT_UINT8(ioaddr + R_MER, 0x41);
						CIC_PRINTF("29e CI slot %d card insert!\n", i);
						break;
					case 0x00:	/* Card removed */
						if(((INPUT_UINT8(ioaddr + + R_IFXSR) & 0x0C)==0)&&
							((INPUT_UINT8(ioaddr + 0x40+ R_IFXSR) & 0x0C)==0))
						OUTPUT_UINT8(ioaddr + R_MER, 0x00);
						CIC_PRINTF("29e CI slot %d card remove!\n", i);
						break;
					default:	/* Only one of CD1 and CD2 changed */
						break;
				}
				if (priv->callback != NULL)
				{
					/* Callback CI stack to tell the slot actived. Current slot 0 */
					;//osal_interrupt_register_hsr((((struct cic_m36_private *)param)->callback), (UINT32)i);
				}
			}

			/* Card IREQ interrupt */
			if ((status & 0x04) != 0)
			{
//				PRINTF("IREQ interrupt\n");
				/* Do nothing */
			}
		}
		break;
	} while (1);
	return IRQ_HANDLED;
}


/*
 * 	Name		:   cic_m3602_read()
 *	Description	:   m3602 CAM read.
 *	Parameter	:	struct cic_device *dev	: Devcie handle
 *					UINT16 size				: Read data size
 *					UINT8 *buffer			: Read data buffer
 *	Return		:	INT32					: return value
 *
 */
static INT32 cic_m3602_read(struct cic_m36_private *tp, int slot, UINT16 size,
							UINT8 *buffer)
{
	//struct cic_m3602_private *tp = (struct cic_m3602_private *)dev->priv;
	int i;
	CIC_PRINTF("%s: slot %d,  size %d\n", __FUNCTION__, slot, size);

	mutex_lock(&ali_ci_mutex);
	for (i = 0; i < size; i++)
	{
		buffer[i] = INPUT_UINT8(tp->reg_data);
		CIC_PRINTF("\t rd_mem: 0x%02x\n", buffer[i]);
	}
	mutex_unlock(&ali_ci_mutex);

	return SUCCESS;
}

/*
 * 	Name		:   cic_m3602_write()
 *	Description	:   m3602 CAM write.
 *	Parameter	:	struct cic_device *dev	: Devcie handle
 *					UINT16 size				: Write data size
 *					UINT8 *buffer			: Wrtie data buffer
 *	Return		:	INT32					: return value
 *
 */
static INT32 cic_m3602_write(struct cic_m36_private *tp, int slot, UINT16 size,
							 UINT8 *buffer)
{
	//struct cic_m3602_private *tp = (struct cic_m3602_private *)dev->priv;
	int i;
	CIC_PRINTF("%s: slot %d,  size %d\n", __FUNCTION__, slot, size);

	mutex_lock(&ali_ci_mutex);
	/* Write data to CI card */
	for (i = 0; i < size; i++)
	{
		OUTPUT_UINT8(tp->reg_data, buffer[i]);
		CIC_PRINTF("\t wt_mem: 0x%02x\n", buffer[i]);
	}
	mutex_unlock(&ali_ci_mutex);
	return SUCCESS;
}

/*
 * 	Name		:   cic_m3602_ioctl()
 *	Description	:   m3602 CI controler IO control function.
 *	Parameter	:	struct cic_device *dev	: Devcie handle
 *					INT32 cmd				: IO command
 *					UINT32 param			: Command parameter
 *	Return		:	INT32					: return value
 *
 */
static INT32 cic_m3602_readmem(struct cic_m36_private *priv, ca_msg_t *param)
{
	//UINT32 memaddr = dev->base_addr + R_MBASE + (param->addr << 1);
	UINT16 addr=param->type&0xffff;
	UINT32 memaddr = priv->base_addr + R_MBASE + (addr << 1);
	int i;
	CIC_PRINTF("ci rd mem: slot %d, addr %08x, size %d\n", param->index, addr, param->length);
	if (addr + param->length>= 0x2000)
		return ERR_FAILUE;

	mutex_lock(&ali_ci_mutex);
	for (i = 0; i < param->length; i++)
	{
		param->msg[i] = INPUT_UINT8(memaddr + (i << 1));
		CIC_PRINTF("\t rd_mem: 0x%02x\n", param->msg[i]);
	}
	mutex_unlock(&ali_ci_mutex);
	return SUCCESS;
}

static INT32 cic_m3602_writemem(struct cic_m36_private *priv, ca_msg_t *param)
{
	//UINT32 memaddr = dev->base_addr + R_MBASE + (param->addr << 1);
	UINT16 addr=param->type&0xffff;
	UINT32 memaddr = priv->base_addr + R_MBASE + (addr << 1);
	int i;
	CIC_PRINTF("ci wr mem: slot %d, addr %08x, size %d\n", param->index, addr, param->length);
	if (addr + param->length >= 0x2000)
		return ERR_FAILUE;
	mutex_lock(&ali_ci_mutex);
	for (i = 0; i < param->length; i++)
	{
		OUTPUT_UINT8(memaddr + (i << 1), param->msg[i]);
		CIC_PRINTF("\t wt_mem: 0x%02x\n", param->msg[i]);
	}
	mutex_unlock(&ali_ci_mutex);
	return SUCCESS;
}

static INT32 cic_m3602_readio(struct cic_m36_private *priv, ca_msg_t *param)
{
	//struct cic_m3602_private *tp = (struct cic_m3602_private *)dev->priv;
	UINT16 reg=param->type>>16;
	static UINT32 rd_io_cnt = 0;
	if((rd_io_cnt++)>100)
	{	
		rd_io_cnt = 0;
		CIC_PRINTF("ci rd io: slot %d, reg %d\n", param->index, reg);
	}
	mutex_lock(&ali_ci_mutex);
	switch (reg)
	{
	case CIC_DATA:
		param->msg[0] = INPUT_UINT8(priv->reg_data);
		break;
	case CIC_CSR:
		param->msg[0] = INPUT_UINT8(priv->reg_cs);
		break;
	case CIC_SIZELS:
		param->msg[0] = INPUT_UINT8(priv->reg_szl);
		break;
	case CIC_SIZEMS:
		param->msg[0] = INPUT_UINT8(priv->reg_szh);
		break;
	default :
		mutex_unlock(&ali_ci_mutex);
		return ERR_FAILUE;
		break;
	}
	mutex_unlock(&ali_ci_mutex);
	CIC_PRINTF("ci rd io success: slot %d, reg %d, 0x%02x\n", param->index, reg, param->msg[0]);
	return SUCCESS;
}

static INT32 cic_m3602_writeio(struct cic_m36_private *priv, ca_msg_t *param)
{
	//struct cic_m3602_private *tp = (struct cic_m3602_private *)dev->priv;
	UINT16 reg=param->type>>16;
	static UINT32 wr_io_cnt = 0;
	if((wr_io_cnt++)>100)
	{	
		wr_io_cnt = 0;
		CIC_PRINTF("ci wr io: slot %d, reg %d, 0x%02x\n", param->index, reg, param->msg[0]);
	}
	mutex_lock(&ali_ci_mutex);
	switch (reg)
	{
	case CIC_DATA:
		OUTPUT_UINT8(priv->reg_data, param->msg[0]);
		break;
	case CIC_CSR:
		OUTPUT_UINT8(priv->reg_cs, param->msg[0]);
		break;
	case CIC_SIZELS:
		OUTPUT_UINT8(priv->reg_szl, param->msg[0]);
		break;
	case CIC_SIZEMS:
		OUTPUT_UINT8(priv->reg_szh, param->msg[0]);
		break;
	default :
		mutex_unlock(&ali_ci_mutex);
		return ERR_FAILUE;
		break;
	}
	mutex_unlock(&ali_ci_mutex);
	CIC_PRINTF("ci wr io success: slot %d, reg %d, 0x%02x\n", param->index, reg, param->msg[0]);
	return SUCCESS;
}

static INT32 cic_m3602_tsignal(struct cic_m36_private *priv, ca_slot_info_t *param)
{
	//struct cic_m3602_private *tp = (struct cic_m3602_private *)dev->priv;
	UINT32 ioaddr = priv->base_addr;

	mutex_lock(&ali_ci_mutex);
	switch (param->type)
	{
	case CIC_CARD_DETECT:
		param->flags = ((INPUT_UINT8(ioaddr + (param->num * 0x40) + R_IFXSR) & 0x0C) >> 2);// CARD A||B IS ALREADY INSERTED
		//CIC_PRINTF("CIC_CARD_DETECT, status: 0x%02x\n", param->flags);	
		break;
	case CIC_CARD_READY:
		param->flags = ((INPUT_UINT8(ioaddr + (param->num * 0x40) + R_IFXSR) & 0x6C) == 0x6C);//CARD A||B POWER ON AND READY
		//CIC_PRINTF("CIC_CARD_READY, status: 0x%02x\n", param->flags);	
		break;
	default:
		break;
	}
	mutex_unlock(&ali_ci_mutex);
	
	return SUCCESS;
}

static INT32 cic_m3602_ssignal(struct cic_m36_private *priv, ca_slot_info_t *param)
{
	//struct cic_m36_private *tp = (struct cic_m3602_private *)dev->priv;
	UINT32 ioaddr = priv->base_addr;
	mutex_lock(&ali_ci_mutex);
	switch (param->type)
	{
	case CIC_ENSTREAM:	/* Enable stream (1) or not (0) */
		if (param->flags)
		{
			CIC_PRINTF("TS pass slot %d\n", param->num);
			/* Set TS pass CAM card */
			if (param->num == 0)//slot 0 pass stream.
			{
			OUTPUT_UINT8(ioaddr + R_TSCR1,
			  INPUT_UINT8(ioaddr + R_TSCR1) & ~RB_TSCR_CIBYPASS);
			OUTPUT_UINT8(ioaddr + R_CISEL+2,
			  INPUT_UINT8(ioaddr + R_CISEL+2) & (~0x80));
			}
			else if (param->num == 1)//slot 1 pass stream.
			{
			OUTPUT_UINT8(ioaddr + R_CISEL,
			  INPUT_UINT8(ioaddr + R_CISEL) & ~RB_TSCR_CIBYPASS);
			OUTPUT_UINT8(ioaddr + R_CISEL+2,
			  INPUT_UINT8(ioaddr + R_CISEL+2) & (~0x40));
			}
		} else
		{
			/* Set TS bypass CAM card */
			CIC_PRINTF("TS bypass slot %d\n", param->num);
			if (param->num == 0)//slot 0 pass stream.
			{
			OUTPUT_UINT8(ioaddr + R_TSCR1,
			  INPUT_UINT8(ioaddr + R_TSCR1)|RB_TSCR_CIBYPASS);
			}
			else if (param->num == 1)//slot 1 pass stream.
			{
			OUTPUT_UINT8(ioaddr + R_CISEL,
			  INPUT_UINT8(ioaddr + R_CISEL)|RB_TSCR_CIBYPASS);
			}
		}
		break;
	case CIC_ENSLOT:	/* Enable slot */
		if (param->flags)
		{
			/* Enable power supply */
			CIC_PRINTF("CI en slot %d power\n", param->num );
			if (param->num == 0)//slot 0.
			{
				/* Make sure auto clear power supply */
				OUTPUT_UINT8(ioaddr + R_EIR, RE_EXTCR1);
				OUTPUT_UINT8(ioaddr + R_EDR, ((INPUT_UINT8(ioaddr + R_EDR) & 0xfc) | 0x02));
			}
			else if (param->num == 1)//slot 1.
			{
				/* Make sure auto clear power supply */
				OUTPUT_UINT8(ioaddr + R_EIR, RE_EXTCR1);
				OUTPUT_UINT8(ioaddr + R_EDR, ((INPUT_UINT8(ioaddr + R_EDR) & 0xf3) | 0x08));
			}
			
			//just provide Vcc, not enable the output here.
			OUTPUT_UINT8(ioaddr + (param->num * 0x40) + R_PWRCR, 0x90);
		}
		else
		{
			/* Disable power supply */
			CIC_PRINTF("CI dis power\n");
			OUTPUT_UINT8(ioaddr + (param->num * 0x40) + R_PWRCR, 0x00);
		}
		break;
	case CIC_RSTSLOT:	/* Reset slot (0) or not (1) */
		CIC_PRINTF("CI reset slot %d\n", param->num);
		OUTPUT_UINT8(ioaddr + (param->num * 0x40) + R_IGCR, (!param->flags & 1) << 7);
		break;
	case CIC_IOMEM:		/* Switch IO (1) or Mem space (0) */
		/* Do nothing for IO and mem space all accessable at sametime */
		break;
	case CIC_SLOTSEL:	/* Select slot */
		CIC_PRINTF("CI select slot %d\n", param->num);
		OUTPUT_UINT8(ioaddr + R_CISEL, ((INPUT_UINT8(ioaddr + R_CISEL) & 0xfd) | ((param->num == 0) ? 0x00:0x02)));
		break;
	default:
		break;
	}
	mutex_unlock(&ali_ci_mutex);

	return SUCCESS;
}

static int ali_m36_cic_open(struct inode *inode, struct file *file);
static int ali_m36_cic_release(struct inode *inode, struct file *file);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long ali_m36_cic_ioctl(struct file *file, unsigned int cmd, void *parg);
#else
static int ali_m36_cic_ioctl(struct inode *inode, struct file *file,
			   unsigned int cmd, void *parg);
#endif

/******************************************************************************
 * driver registration
 ******************************************************************************/

static struct file_operations ali_m36_cic_fops = {
	.owner		= THIS_MODULE,
	// .write		= ali_m36_cic_write,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	.unlocked_ioctl = ali_m36_cic_ioctl,
#else
	.ioctl		= ali_m36_cic_ioctl,
#endif	
	.open		= ali_m36_cic_open,
	.release	=  ali_m36_cic_release,
	//.poll		= dvb_cic_poll,
};

struct ali_cic_device ali_m36_cic_dev;

struct class *ali_m36_cic_class;
struct device *ali_m36_cic_dev_node;

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long ali_m36_cic_ioctl(struct file *file, unsigned int cmd, void *parg)
#else
static int ali_m36_cic_ioctl(struct inode *inode, struct file *file,
			   unsigned int cmd, unsigned long parg)
#endif			   
{
	struct ali_cic_device *dev = file->private_data;
	struct cic_m36_private *tp = NULL;

	if (NULL == dev || NULL == dev->priv)
		return -EINVAL;

	tp = dev->priv;

	//unsigned long arg = (unsigned long) parg;
	unsigned int flag;
	int ret = 0;

	if(cmd!=CA_GET_SLOT_INFO)
		PRINTK_INFO("%s: %p, cmd=%04x\n", __FUNCTION__, dev, cmd);

	switch (cmd) 
	{
	case CA_GET_CAP:
		break;
	case CA_GET_SLOT_INFO:
	{
		ca_slot_info_t info;
		copy_from_user(&info, (void *)parg, sizeof(ca_slot_info_t));
		ret=cic_m3602_tsignal(tp, &info);
		copy_to_user((void *)parg, &info, sizeof(ca_slot_info_t));
		break;
	}
	case CA_SET_SLOT_INFO:
	{
		ca_slot_info_t info;
		copy_from_user(&info, (void *)parg, sizeof(ca_slot_info_t));
		ret=cic_m3602_ssignal(tp, &info);
		break;
	}
	case CA_GET_MSG:
		copy_from_user(&ali_ci_msg, (void *)parg, 12);
		flag=ali_ci_msg.type>>16;
		switch(flag)
		{
		case CIC_DATA:			/* CI data register */
		case CIC_CSR:			/* CI command/stauts register */
		case CIC_SIZELS:			/* CI size register low bytes */
		case CIC_SIZEMS:		/* CI size register high bytes */
			ret=cic_m3602_readio(tp, &ali_ci_msg);
			break;
		case CIC_MEMORY:		/* CI memory space*/
			ret=cic_m3602_readmem(tp, &ali_ci_msg);
			break;
		case CIC_BLOCK:
			ret=cic_m3602_read(tp, ali_ci_msg.index, ali_ci_msg.length, ali_ci_msg.msg);
			break;
		}
		copy_to_user((void *)parg, &ali_ci_msg, sizeof(ca_msg_t));
		break;
	case CA_SEND_MSG:
		copy_from_user(&ali_ci_msg, (void *)parg, sizeof(ca_msg_t));
		flag=ali_ci_msg.type>>16;
		switch(flag)
		{
		case CIC_DATA:			/* CI data register */
		case CIC_CSR:			/* CI command/stauts register */
		case CIC_SIZELS:			/* CI size register low bytes */
		case CIC_SIZEMS:		/* CI size register high bytes */
			ret=cic_m3602_writeio(tp, &ali_ci_msg);
			break;
		case CIC_MEMORY:		/* CI memory space*/
			ret=cic_m3602_writemem(tp, &ali_ci_msg);
			break;
		case CIC_BLOCK:
			ret=cic_m3602_write(tp, ali_ci_msg.index, ali_ci_msg.length, ali_ci_msg.msg);
			break;
		}
		break;
	default:
		ret=-ENOIOCTLCMD;
		break;
	}
	return ret;
}

static int ali_m36_cic_open(struct inode *inode, struct file *file)
{
    if (ali_m36_cic_dev.in_use)
      return -EBUSY;

	UINT32 cic_base_address = 0xb801a000;
	ali_m36_cic_dev.priv=&ali_m36_cic_private;
	memset(&ali_m36_cic_private, 0, sizeof(struct cic_m36_private));
	ali_m36_cic_private.base_addr=0xb801a000;
	ali_m36_cic_private.slot_1_int_num = 17;
	ali_m36_cic_private.slot_2_int_num = 40;

	/* Setup CI command interface registers */
	ali_m36_cic_private.reg_data = cic_base_address + R_IOBASE;
	ali_m36_cic_private.reg_cs   = cic_base_address + R_IOBASE + 1;
	ali_m36_cic_private.reg_szl  = cic_base_address + R_IOBASE + 2;
	ali_m36_cic_private.reg_szh  = cic_base_address + R_IOBASE + 3;

	if (request_irq(ali_m36_cic_private.slot_1_int_num, cic_m3602_interrupt, 0, 
			ALI_CIC_DEVICE_NAME, &ali_m36_cic_dev) < 0) {
		PRINTK_INFO("Failed to register ali_m36_cic A interrupt %d", ali_m36_cic_private.slot_1_int_num);
		PRINTK_INFO("register IRQ failed.\n");
		return -EAGAIN;
	}
	else
	{
		PRINTK_INFO("register CI IRQ %d success.\n", ali_m36_cic_private.slot_1_int_num);
	}
	
	if (request_irq(ali_m36_cic_private.slot_2_int_num, cic_m3602_interrupt, 0, 
			ALI_CIC_DEVICE_NAME, &ali_m36_cic_dev) < 0) {
		PRINTK_INFO("Failed to register ali_m36_cic B interrupt %d", ali_m36_cic_private.slot_2_int_num);
		PRINTK_INFO("register IRQ failed.\n");
		return -EAGAIN;
	}
	else
	{
		PRINTK_INFO("register CI IRQ %d success.\n", ali_m36_cic_private.slot_2_int_num);
	}
	/* Enable interrupt */
	OUTPUT_UINT8(cic_base_address + R_MICR, 0x88);
	OUTPUT_UINT8(cic_base_address + 0x40 + R_MICR, 0x88);
	struct cic_m36_private *tp;
	
	tp=&ali_m36_cic_private;
	
	file->private_data=(void*)&ali_m36_cic_dev;

#if 0	//should take pesi's board for some test
	OUTPUT_UINT8(tp->base_addr + R_STM0, 0x01);
	OUTPUT_UINT8(tp->base_addr + R_CTM0, 0x04);
	OUTPUT_UINT8(tp->base_addr + R_RTM0, 0x01);
	OUTPUT_UINT8(tp->base_addr + R_STM1, 0x01);
	OUTPUT_UINT8(tp->base_addr + R_CTM1, 0x04);
	OUTPUT_UINT8(tp->base_addr + R_RTM1, 0x01);
#else
	OUTPUT_UINT8(tp->base_addr+ R_STM0, 0x03);
	OUTPUT_UINT8(tp->base_addr + R_CTM0, 0x06);
	OUTPUT_UINT8(tp->base_addr + R_RTM0, 0x03);
	OUTPUT_UINT8(tp->base_addr + R_STM1, 0x03);
	OUTPUT_UINT8(tp->base_addr + R_CTM1, 0x06);
	OUTPUT_UINT8(tp->base_addr + R_RTM1, 0x03);
#endif

	/* Enable memory map 0. (CAM need only 1 memory windows).
	 * Enable IO map 0. (CAM need only 1 IO windows).
	 * HW hase set right configuration value when reset card.
	 */

	//set card b setting according to card a.	 
	OUTPUT_UINT8(tp->base_addr + R_MER, (INPUT_UINT8(tp->base_addr + R_MER) & 0xbe) | 0x41);
	/* Access attribute memory space for CIS */
	OUTPUT_UINT8(tp->base_addr + R_MMOAR0 + 1,
	  (INPUT_UINT8(tp->base_addr + R_MMOAR0 + 1) & 0xbf) | 0x40);

	ali_m36_cic_dev.in_use=1;
	return 0;
}

static int ali_m36_cic_release(struct inode *inode, struct file *file)
{
	struct ali_cic_device *dev = file->private_data;
	struct cic_m36_private *tp = NULL;

	if (NULL == dev || NULL == dev->priv)
		return -EINVAL;
	tp = dev->priv;
	free_irq(ali_m36_cic_private.slot_1_int_num, &ali_m36_cic_dev);
	free_irq(ali_m36_cic_private.slot_2_int_num, &ali_m36_cic_dev);

	OUTPUT_UINT8(tp->base_addr+ R_TSCR1,
		INPUT_UINT8(tp->base_addr + R_TSCR1) | RB_TSCR_CIBYPASS);

	/* Disable power supply */
	OUTPUT_UINT8(tp->base_addr + R_PWRCR, 0x00);
	
	dev->in_use=0;
	dev->priv=NULL;
	return 0;
}


static int __devinit ali_m36_cic_dev_init(void)
{
	int ret;
	INT32   result;
	dev_t devno;
	UINT32 cic_base_address = 0xb801a000;

	PRINTK_INFO("%s start.\n", __FUNCTION__);
	ret=alloc_chrdev_region(&devno, 0, 1, ALI_CIC_DEVICE_NAME);
	if(ret<0)
	{
		PRINTK_INFO("Alloc device region failed, err: %d.\n",ret);
		return ret;
	}
	memset(&ali_m36_cic_dev, 0, sizeof(struct ali_cic_device));
	cdev_init(&ali_m36_cic_dev.cdev, &ali_m36_cic_fops);
	ali_m36_cic_dev.cdev.owner=THIS_MODULE;
	ali_m36_cic_dev.cdev.ops=&ali_m36_cic_fops;
	ret=cdev_add(&ali_m36_cic_dev.cdev, devno, 1);
	if(ret)
	{
		PRINTK_INFO("Alloc CI controller device failed, err: %d.\n", ret);
		return ret;
	}
	
	PRINTK_INFO("register CI controller device end.\n");


	ali_m36_cic_class = class_create(THIS_MODULE, "ali_m36_cic_class");

	if (IS_ERR(ali_m36_cic_class))
	{
		result = PTR_ERR(ali_m36_cic_class);

		goto err1;
	}

	ali_m36_cic_dev_node = device_create(ali_m36_cic_class, NULL, devno, &ali_m36_cic_dev, 
                           ALI_CIC_DEVICE_NAME);
         if (IS_ERR(ali_m36_cic_dev_node))
    {
		printk(KERN_ERR "device_create() failed!\n");

		result = PTR_ERR(ali_m36_cic_dev_node);

		goto err2;
	}

	memset(&ali_m36_cic_private, 0, sizeof(struct cic_m36_private));
	
	cic_base_address=0xb801a000;
	ali_m36_cic_private.base_addr=0xb801a000;
	ali_m36_cic_private.slot_1_int_num = 17;
	ali_m36_cic_private.slot_2_int_num = 40;

	/* Setup CI command interface registers */
	ali_m36_cic_private.reg_data = cic_base_address + R_IOBASE;
	ali_m36_cic_private.reg_cs   = cic_base_address + R_IOBASE + 1;
	ali_m36_cic_private.reg_szl  = cic_base_address + R_IOBASE + 2;
	ali_m36_cic_private.reg_szh  = cic_base_address + R_IOBASE + 3;

	if (request_irq(ali_m36_cic_private.slot_1_int_num, cic_m3602_interrupt, 0, 
			ALI_CIC_DEVICE_NAME, &ali_m36_cic_dev) < 0) {
		PRINTK_INFO("Failed to register ali_m36_cic A interrupt %d", ali_m36_cic_private.slot_1_int_num);
		PRINTK_INFO("register IRQ failed.\n");
		goto err2;
	}
	else
	{
		PRINTK_INFO("register CI IRQ %d success.\n", ali_m36_cic_private.slot_1_int_num);
	}
	
	/**/
	if (request_irq(ali_m36_cic_private.slot_2_int_num, cic_m3602_interrupt, 0, 
			ALI_CIC_DEVICE_NAME, &ali_m36_cic_dev) < 0) {
		PRINTK_INFO("Failed to register ali_m36_cic B interrupt %d", ali_m36_cic_private.slot_2_int_num);
		PRINTK_INFO("register IRQ failed.\n");
		goto err3;
	}
	else
	{
		PRINTK_INFO("register CI IRQ %d success.\n", ali_m36_cic_private.slot_2_int_num);
	}
	
	/* Enable interrupt */
	OUTPUT_UINT8(cic_base_address + R_MICR, 0x88);
	OUTPUT_UINT8(cic_base_address + 0x40 + R_MICR, 0x88);


	mutex_init(&ali_ci_mutex);

	PRINTK_INFO("Ali CI controller device register success.\n");
	
	return ret;
err3:
	free_irq(ali_m36_cic_private.slot_1_int_num, &ali_m36_cic_dev);
err2:
	class_destroy(ali_m36_cic_class);
err1:
	cdev_del(&ali_m36_cic_dev.cdev);
	//kfree(priv);
	PRINTK_INFO("Ali CI controller device register Failed!\n");
	return -EINVAL;
}


static void __exit ali_m36_cic_dev_exit(void)
{
	UINT32 cic_base_address = 0xb801a000;
	/* Disable interrupt */
	OUTPUT_UINT8(cic_base_address + R_MICR, 0x00);
	OUTPUT_UINT8(cic_base_address + 0x40 + R_MICR, 0x00);


	free_irq(ali_m36_cic_private.slot_1_int_num, &ali_m36_cic_dev);
	free_irq(ali_m36_cic_private.slot_2_int_num, &ali_m36_cic_dev);

	if(ali_m36_cic_dev_node != NULL)
		device_del(ali_m36_cic_dev_node);

	if(ali_m36_cic_class != NULL)
		class_destroy(ali_m36_cic_class);
	cdev_del(&ali_m36_cic_dev.cdev);
	mutex_destroy(&ali_ci_mutex);
}

module_init(ali_m36_cic_dev_init);
module_exit(ali_m36_cic_dev_exit);

MODULE_DESCRIPTION("driver for the Ali M36xx CI controller device");
MODULE_AUTHOR("ALi Corp ShangHai SDK Team, Eric Li");
MODULE_LICENSE("GPL");
