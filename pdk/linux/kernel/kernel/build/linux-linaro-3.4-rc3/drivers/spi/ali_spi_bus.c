/*
 * Driver for Atmel AT32 and AT91 SPI Controllers
 *
 * Copyright (C) 2006 Atmel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>

#include <asm/io.h>   // cari chen

#include <../../../alidrivers/include/ali_reg.h>  


//#include <asm/io.h>
//#include <mach/board.h>
//#include <mach/gpio.h>
//#include <mach/cpu.h>

//#include "atmel_spi.h"

/*
 * The core SPI transfer engine just talks to a register bank to set up
 * DMA transfers; transfer queue progress is driven by IRQs.  The clock
 * framework provides the base clock, subdivided for each spi_device.
 *
 * Newer controllers, marked with "new_1" flag, have:
 *  - CR.LASTXFER
 *  - SPI_MR.DIV32 may become FDIV or must-be-zero (here: always zero)
 *  - SPI_SR.TXEMPTY, SPI_SR.NSSR (and corresponding irqs)
 *  - SPI_CSRx.CSAAT
 *  - SPI_CSRx.SBCR allows faster clocking
 */
 

typedef char INT8;
typedef unsigned char UINT8;
typedef short INT16;
typedef unsigned short UINT16;
typedef int INT32;
typedef unsigned int UINT32;

static UINT32 reg_data_save_8c;  // cari chen
static UINT32 reg_data_save_88;

#if 0
#define PRINTK_INFO(x...) printk(KERN_INFO x)
#else
#define PRINTK_INFO(x...)  do{}while(0)
#endif

#if 0
#define PRINTK_BUS(x...) printk(KERN_INFO x)
#else
#define PRINTK_BUS(x...)  do{}while(0)
#endif


#define ALI_FLASH_BASE_ADDR_PHYS		0x0fc00000
#define ALI_SFLASH_REG_BASE_ADDR_PHYS   0x1802e000
#define ALI_SOC_REG_BASE_ADDR_PHYS   	0x18000000


void __iomem * ali_sflash_virt_addr ;
//void __iomem * ali_soc_ioremap_reg ;
//void __iomem * ali_sflash_ioremap_reg ;


#define	ALI_SPI_INS		(ALI_SFLASH_REG_BASE_ADDR_PHYS + 0x98)
#define	ALI_SPI_FMT		(ALI_SFLASH_REG_BASE_ADDR_PHYS + 0x99)
#define	ALI_SPI_DUM		(ALI_SFLASH_REG_BASE_ADDR_PHYS + 0x9A)
#define	ALI_SPI_CFG		(ALI_SFLASH_REG_BASE_ADDR_PHYS + 0x9B)

#define SPI_HIT_DATA    0x01
#define SPI_HIT_DUMM    0x02
#define SPI_HIT_ADDR    0x04
#define SPI_HIT_CODE    0x08
#define SPI_CONT_RD    	0x40
#define SPI_CONT_WR	    0x80

#define STD_READ	(0x03|((SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA)<<8))
#define FST_READ	(0x0b|((SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA|SF_HIT_DUMM)<<8))
#define DIO_READ	(0xbb|((SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA|SF_HIT_DUMM)<<8))
#define QIO_READ	(0xeb|((SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA|SF_HIT_DUMM|0x20)<<8))

struct ali_spi {
	spinlock_t		       lock;
	struct platform_device *pdev;
	struct spi_device	   *stay;

	u8			           stopping;
	struct list_head	   queue;
	struct spi_transfer	   *current_transfer;
	unsigned long		   current_remaining_bytes;
	struct spi_transfer	   *next_transfer;
	unsigned long		   next_remaining_bytes;

	UINT8 spi_ins;         //SPI instruction.
	UINT8 ins_fmt;         //Ali SPI instruction format.
	UINT8 r_or_w;          //0: Read; 1: Write
	UINT8 spi_active;      //0: idle; 1: SPI instruction ready.
	UINT32 offset;         //SPI access offset.
	UINT32 xfer_len;       //Transfer length.
};


#include <mtd/mtd-abi.h>

#ifdef CONFIG_ALI_SFLASH
extern int check_sflash_cmdext_group_by_bitoffset(unsigned int);
#define CHECK_SFLASH_EXT(bo) check_sflash_cmdext_group_by_bitoffset(bo)
#else
#define CHECK_SFLASH_EXT(bo) 0
#endif

void cari_delay_us(int t)
{
	int i,j,k;
	
	for(i= 0;i<t;i++)
	{
		for(j=0;j<t;j++)
		{
			for(k=0;k<t;k++)
			{
				
			}
		}
	}
}

/*Reserved for flash/ethernet DMA*/
/*static irqreturn_t
ali_spi_interrupt(int irq, void *dev_id)
{
	return 0;
}*/

/* the spi->mode bits understood by this driver: */
//#define MODEBITS (SPI_CPOL | SPI_CPHA | SPI_CS_HIGH)

static int ali_spi_setup(struct spi_device *spi)
{
	PRINTK_INFO("%s.\n", __FUNCTION__);
	
	__REG8ALI(ALI_SPI_CFG) = 0xc2 ;
	__REG8ALI(ALI_SPI_DUM) = 0x00 ;
	__REG8ALI(ALI_SPI_FMT) = 0x0d ;
	__REG8ALI(ALI_SPI_INS) = 0x03 ;

	//PRINTK_BUS("ALI_SPI_CFG = 0x%08x\n",__REG8ALI(ALI_SPI_CFG));
	//PRINTK_BUS("ALI_SPI_DUM = 0x%08x\n",__REG8ALI(ALI_SPI_DUM));
	//PRINTK_BUS("ALI_SPI_FMT = 0x%08x\n",__REG8ALI(ALI_SPI_FMT));
	//PRINTK_BUS("ALI_SPI_INS = 0x%08x\n",__REG8ALI(ALI_SPI_INS));
	
	//*((volatile UINT8 *)0x1800008d) &= 0xfe;
	
#if 0
	struct ali_spi	*as;
	u32			scbr, csr;
	unsigned int		bits = spi->bits_per_word;
	unsigned long		bus_hz;
	unsigned int		npcs_pin;
	int			ret;

	as = spi_master_get_devdata(spi->master);

	if (as->stopping)
		return -ESHUTDOWN;

	if (spi->chip_select > spi->master->num_chipselect) {
		dev_dbg(&spi->dev,
				"setup: invalid chipselect %u (%u defined)\n",
				spi->chip_select, spi->master->num_chipselect);
		return -EINVAL;
	}

	if (bits == 0)
		bits = 8;
	if (bits < 8 || bits > 16) {
		dev_dbg(&spi->dev,
				"setup: invalid bits_per_word %u (8 to 16)\n",
				bits);
		return -EINVAL;
	}

	if (spi->mode & ~MODEBITS) {
		dev_dbg(&spi->dev, "setup: unsupported mode bits %x\n",
			spi->mode & ~MODEBITS);
		return -EINVAL;
	}

	/* see notes above re chipselect */
	if (cpu_is_at91rm9200()
			&& spi->chip_select == 0
			&& (spi->mode & SPI_CS_HIGH)) {
		dev_dbg(&spi->dev, "setup: can't be active-high\n");
		return -EINVAL;
	}

	/*
	 * Pre-new_1 chips start out at half the peripheral
	 * bus speed.
	 */
	bus_hz = clk_get_rate(as->clk);
	if (!as->new_1)
		bus_hz /= 2;

	if (spi->max_speed_hz) {
		/*
		 * Calculate the lowest divider that satisfies the
		 * constraint, assuming div32/fdiv/mbz == 0.
		 */
		scbr = DIV_ROUND_UP(bus_hz, spi->max_speed_hz);

		/*
		 * If the resulting divider doesn't fit into the
		 * register bitfield, we can't satisfy the constraint.
		 */
		if (scbr >= (1 << SPI_SCBR_SIZE)) {
			dev_dbg(&spi->dev,
				"setup: %d Hz too slow, scbr %u; min %ld Hz\n",
				spi->max_speed_hz, scbr, bus_hz/255);
			return -EINVAL;
		}
	} else
		/* speed zero means "as slow as possible" */
		scbr = 0xff;

	csr = SPI_BF(SCBR, scbr) | SPI_BF(BITS, bits - 8);
	if (spi->mode & SPI_CPOL)
		csr |= SPI_BIT(CPOL);
	if (!(spi->mode & SPI_CPHA))
		csr |= SPI_BIT(NCPHA);

	/* DLYBS is mostly irrelevant since we manage chipselect using GPIOs.
	 *
	 * DLYBCT would add delays between words, slowing down transfers.
	 * It could potentially be useful to cope with DMA bottlenecks, but
	 * in those cases it's probably best to just use a lower bitrate.
	 */
	csr |= SPI_BF(DLYBS, 0);
	csr |= SPI_BF(DLYBCT, 0);

	/* chipselect must have been muxed as GPIO (e.g. in board setup) */
	npcs_pin = (unsigned int)spi->controller_data;
	if (!spi->controller_state) {
		ret = gpio_request(npcs_pin, spi->dev.bus_id);
		if (ret)
			return ret;
		spi->controller_state = (void *)npcs_pin;
		gpio_direction_output(npcs_pin, !(spi->mode & SPI_CS_HIGH));
	} else {
		unsigned long		flags;

		//spin_lock_irqsave(&as->lock, flags);
		if (as->stay == spi)
			as->stay = NULL;
		cs_deactivate(as, spi);
		//spin_unlock_irqrestore(&as->lock, flags);
	}

	dev_dbg(&spi->dev,
		"setup: %lu Hz bpw %u mode 0x%x -> csr%d %08x\n",
		bus_hz / scbr, bits, spi->mode, spi->chip_select, csr);

	spi_writel(as, CSR0 + 4 * spi->chip_select, csr);

#endif
	return 0;
}

static inline void ali_spi_config_instruction(struct ali_spi *as)
{

	__REG8ALI(ALI_SPI_FMT) = as->ins_fmt;
	__REG8ALI(ALI_SPI_INS) = as->spi_ins;
	
	as->spi_active=0;
}


	
//static int ali_spi_execute_xfer(struct ali_spi *as, struct spi_transfer *xfer)
static int ali_spi_execute_xfer(UINT8 r_or_w, UINT32 offset, UINT8 *buf, UINT32 len)
{
	//void __iomem * 
	UINT32 ali_flash_base_addr;
	UINT32 cur_seg; // = (flash_offset&0xc00000)>>22;
	UINT32 tge_seg; // = ((flash_offset+len-1)&0xc00000)>>22;
	UINT32 ret_len;
	UINT32 tmp;
	UINT32 i;
				 
	ret_len=0;
	cur_seg=(offset&0xc00000)>>22;
	tge_seg=((offset+len-1)&0xc00000)>>22;

	if(cur_seg!=tge_seg)
	{
		UINT32 inter_seg_len;
	
		inter_seg_len = ((cur_seg+1)<<22) - offset;
		ret_len=ali_spi_execute_xfer(r_or_w, offset, buf, inter_seg_len);
		offset += inter_seg_len;
		buf = (void *)((UINT32)buf+inter_seg_len);
		len -= inter_seg_len;
		cur_seg++;
		
		while(tge_seg != cur_seg)
		{
			ret_len+=ali_spi_execute_xfer(r_or_w, offset, buf, 0x400000);
			offset+=0x400000;
			buf = (void *)((UINT32)buf+0x400000);
			len -= 0x400000;
			cur_seg++;
		}

		ret_len+=ali_spi_execute_xfer(r_or_w, offset, buf, len);
		return ret_len;
	}
	else
	{
		ali_flash_base_addr = ali_sflash_virt_addr - (offset&0xc00000);
		
		offset &= 0x3fffff;
	}	
	
	if(r_or_w)
	{
		//SPI write;
		if(NULL==buf)
		{			
			((volatile UINT8 *)ali_flash_base_addr)[offset] = 0;   // write enable
			return 1;
		}
		else
		{	
			/*
			if (len>4)  // test driver data whether equal app data
			{
				for (i = 0; i < (64*4); i+=4)    // printf write data
				{
					
					PRINTK_BUS("driver,before write,[buf] =0x%08x\n",*(UINT32 *)((UINT32)buf+i));
				}
			}
			*/
			for (i = 0; i < len; i++)    // write data
			{
				((volatile UINT8 *)ali_flash_base_addr)[offset+i] = *buf++;

			}
			
			return len;
			
		}
	}
	else
	{
		
		//SPI read;
		UINT32 len_tmp;
		len_tmp=len;
		
		if (((UINT32)buf & 3) == 0)  // align
		{	
			
			for (i = 0; i < (len&(~0x3)); i+=4)
			{
				
				
				*(UINT32 *)((UINT32)buf + i) = *(UINT32 *)(ali_flash_base_addr+offset + i);  
				
			}
			/*
			if((0 == offset)&&(1 == len)&&(buf)) //cari chen , wait free
			{
				while (1) 
				{
				    if(((*((UINT8 *)buf)) & 0x01) == 0)
						break;
					udelay(10);
				}
			}
			*/
			/*
			if(len>4)  // cari chen , test driver data whether equal app data
			{
				for (i = 0; i < (64*4); i+=4)
				{
					PRINTK_BUS("driver,after read,[buf] =0x%08x\n",*(UINT32 *)((UINT32)buf + i));
				}
			}
			*/
		}
		else  // unalign
		{
			
			for (i = 0; i < (len&(~0x3)); i+=4)
			{
				tmp = *(UINT32 *)(ali_flash_base_addr+offset + i);
				*(UINT8 *)((UINT32)buf + i) = tmp & 0xff;
				*(UINT8 *)((UINT32)buf + i + 1) = ((tmp >> 8) & 0xff);
				*(UINT8 *)((UINT32)buf + i + 2) = ((tmp >> 16)& 0xff);
				*(UINT8 *)((UINT32)buf + i + 3) = ((tmp >> 24) & 0xff);
			}
			/*
			if ((0 == offset)&&(1 == len)&&(buf)) // cari chen wait free
			{
				while (1)  
				{
				    if(((*((UINT8 *)buf)) & 0x01) == 0)
						break;
					udelay(10);
				}
			}
			*/
			/*
			if(len>4) // cari chen , test driver data whether equal app data
			{
				for (i = 0; i < (64*4); i+=4)
				{
					PRINTK_BUS("driver,after read,[buf] =0x%08x\n",*(UINT32 *)((UINT32)buf + i));
				}
			}
			*/
		}
		offset = offset + (len&(~0x3));
		buf = buf + (len&(~0x3));
		len -= (len&(~0x3));
		memcpy(buf, (UINT8*)(ali_flash_base_addr+offset), len);
				
		return len_tmp;
	}
}

static int ali_spi_transfer(struct spi_device *spi, struct spi_message *msg)
{
	struct ali_spi	*as;
	struct spi_transfer	*xfer;
	int status = 0;
	
	PRINTK_INFO("%s.\n", __FUNCTION__);

	as = spi_master_get_devdata(spi->master);

	msg->actual_length=0;
	
	int buff_addr1 = 0,buff_addr2 = 0;

	int flag = 1 ;
	
	list_for_each_entry(xfer, &msg->transfers, transfer_list)
	{
	
		//if (!(xfer->tx_buf || xfer->rx_buf) && xfer->len)
		if (!(xfer->len))
		{
			PRINTK_INFO("missing rx or tx buf\n");
			goto ret_failure;
		}
		
		// case 1
		//if((xfer->tx_buf)&&(xfer->rx_buf)&&(xfer->rx_buf==xfer->tx_buf))
		if(xfer->rx_buf==xfer->tx_buf)
		{
			as->spi_ins=((UINT8*)xfer->tx_buf)[0];
			
			PRINTK_BUS("\n1.Write then Read (Read special). ins:0x%02x\n\n", as->spi_ins);  // obtain ID
			
			as->ins_fmt=(SPI_HIT_CODE|SPI_HIT_DATA);
			as->spi_active = 1;
			as->offset=0;
			as->r_or_w=0; //
			as->xfer_len = xfer->len-1;
			ali_spi_config_instruction(as);
			//spin_lock_irq(&as->lock);
			msg->actual_length+=ali_spi_execute_xfer(as->r_or_w, as->offset, &(((UINT8*)xfer->tx_buf)[1]), as->xfer_len);
			//spin_unlock_irq(&as->lock);
			msg->status = status;
			msg->complete(msg->context);	
			if(0x05==as->spi_ins)         // wait free ,Reset sflash to common read mode 
			{
				as->ins_fmt=(SPI_HIT_CODE | SPI_HIT_DATA | SPI_HIT_ADDR);
				as->spi_ins=0x03;
				ali_spi_config_instruction(as);   //Reset sflash to common read mode
			}
			continue;
		}
		
		// case 2
		//if((xfer->tx_buf)&&(!xfer->rx_buf))
		if((*(UINT8*)(xfer->tx_buf))&&(!(*(UINT8*)(xfer->rx_buf))))
		{	
			//case 2.1
			if((0==as->spi_active)&&(1==xfer->len))
			{
				as->spi_ins=((UINT8*)xfer->tx_buf)[0];  // 
				
				PRINTK_BUS("\n2.Simple command. ins:0x%02x\n\n", as->spi_ins);   // write enable & erase chip  
				
				as->ins_fmt=(SPI_HIT_CODE );  // 
				as->spi_active = 1;
				as->offset = 0;
				as->r_or_w = 1;
				//as->xfer_len = 1;
				ali_spi_config_instruction(as);

				//spin_lock_irq(&as->lock);
				msg->actual_length+=ali_spi_execute_xfer(as->r_or_w, as->offset, NULL, as->xfer_len);
								
				//spin_unlock_irq(&as->lock);
				msg->status = status;
				msg->complete(msg->context);
				
				continue;
			}
			//case 2.2
			if(0==as->spi_active)    // sflash read  & sflash write
			{
				as->spi_ins=((UINT8*)xfer->tx_buf)[0];   // read command  0x03 & write command  0x02
				as->offset=(((UINT8*)xfer->tx_buf)[1]<<16)|(((UINT8*)xfer->tx_buf)[2]<<8)|((UINT8*)xfer->tx_buf)[3];
														 // read addr & write addr
				PRINTK_BUS("\n3.Cfg ins and addr, ins:0x%02x, addr:0x%08x\n\n", as->spi_ins, as->offset);
					
				if (4==xfer->len)       // erase sector
				{
					as->ins_fmt=(SPI_HIT_CODE|SPI_HIT_ADDR);  // 0x0c
					as->spi_active=1;
					msg->actual_length+=4;
				}
				else if (5==xfer->len)   //  why
				{
					as->ins_fmt=(SPI_HIT_CODE|SPI_HIT_ADDR|SPI_HIT_DUMM);
					as->spi_active=1;
					msg->actual_length+=5;
				}
				continue;
			}
			//case 2.3
			if((1==as->spi_active)&&(0!=xfer->len))   // sflash write
			{
				PRINTK_BUS("\n4.SPI write. len:%d\n", xfer->len);
				if((xfer->len>1)&&(0x02==as->spi_ins))
				{
					as->ins_fmt|=(SPI_HIT_DATA|SPI_CONT_WR);  // 0x8d   // continue write
					//as->ins_fmt|=(SPI_HIT_DATA);                // 0x08   // normal write
				}
				else if (0x42==as->spi_ins)
				{
					if (CHECK_SFLASH_EXT(FLASHCMD_EXT_OFFSETBIT_FOR_GIGA))
					{
					    as->ins_fmt|=(SPI_HIT_DATA|SPI_CONT_WR);
					}
				}
				else
				{
					as->ins_fmt|=SPI_HIT_DATA;
				}
				as->r_or_w=1;
				as->xfer_len=xfer->len;
				ali_spi_config_instruction(as);
				//spin_lock_irq(&as->lock);
				msg->actual_length+=ali_spi_execute_xfer(as->r_or_w, as->offset, xfer->tx_buf, as->xfer_len);
			    //spin_unlock_irq(&as->lock);	
				msg->status = status;
				msg->complete(msg->context);	
				
				if(0x02==as->spi_ins)   // cari chen
				{
					as->ins_fmt=(SPI_HIT_CODE | SPI_HIT_DATA | SPI_HIT_ADDR);
					__REG8ALI(ALI_SPI_FMT) = as->ins_fmt;
				}
				continue;
			}
		}
		// case 3
		//if((xfer->rx_buf)&&(!xfer->tx_buf)&&(1==as->spi_active)&&(0!=xfer->len))
		if(((UINT8*)(xfer->rx_buf))&&(!(*(UINT8*)(xfer->tx_buf)))&&(1==as->spi_active)&&(0!=xfer->len))
		{                                                     // sflash read
			PRINTK_BUS("\n5.SPI read. len:%d\n", xfer->len);
			as->ins_fmt |= SPI_HIT_DATA;
			as->ins_fmt |= 0x40;   // cari chen
			as->r_or_w=0;
			as->xfer_len=xfer->len;
			ali_spi_config_instruction(as);
			//spin_lock_irq(&as->lock);
			msg->actual_length+=ali_spi_execute_xfer(as->r_or_w, as->offset, xfer->rx_buf, as->xfer_len);

			//spin_unlock_irq(&as->lock);
			msg->status = status;
			msg->complete(msg->context);	

			if(0x03==as->spi_ins)  // cari chen
			{
				as->ins_fmt=(SPI_HIT_CODE | SPI_HIT_DATA | SPI_HIT_ADDR);
				ali_spi_config_instruction(as);
			}
			continue;
		}
	}

	
	if(1==as->spi_active)    // erase sector
	{
		PRINTK_BUS("\n6.SPI command with addr:0x%08x, ins fmt:0x%02x, ins:0x%02x\n", as->offset, as->ins_fmt, as->spi_ins);
		as->r_or_w=1;
		as->xfer_len=1;
		ali_spi_config_instruction(as);
	    //spin_lock_irq(&as->lock);
		msg->actual_length+=ali_spi_execute_xfer(as->r_or_w, as->offset, NULL, as->xfer_len);
		//spin_unlock_irq(&as->lock);
		msg->status = status;
		msg->complete(msg->context);	
		/*
		if(0xd8==as->spi_ins)
		{
			as->ins_fmt=(SPI_HIT_CODE | SPI_HIT_DATA | SPI_HIT_ADDR);
			as->spi_ins=0x03;
			ali_spi_config_instruction(as);
		}*/
	}

//ret_success:

	return 0;
	
ret_failure:
	return -EINVAL;
	
}

static void ali_spi_cleanup(struct spi_device *spi)
{
	PRINTK_INFO("%s.\n", __FUNCTION__);
/*
	struct ali_spi	*as = spi_master_get_devdata(spi->master);
	unsigned		gpio = (unsigned) spi->controller_data;
	unsigned long		flags;

	if (!spi->controller_state)
		return;

	//spin_lock_irqsave(&as->lock, flags);
	if (as->stay == spi) {
		as->stay = NULL;
		cs_deactivate(as, spi);
	}
	//spin_unlock_irqrestore(&as->lock, flags);

	gpio_free(gpio);
*/
}

/*-------------------------------------------------------------------------*/

static int __init ali_spi_probe(struct platform_device *pdev)
{
	int			ret;
	struct spi_master	*master;
	struct ali_spi	*as;
	
	PRINTK_BUS("\n%s.\n\n", __FUNCTION__);

	UINT32 bonding;
	UINT32 chipid;
	

	/* setup spi core then atmel-specific driver state */
	ret = -ENOMEM;
	master = spi_alloc_master(&pdev->dev, sizeof *as);
	if (!master)
		goto out_free;

	master->bus_num = pdev->id;
	master->num_chipselect = 4;
	master->setup = ali_spi_setup;
	master->transfer = ali_spi_transfer;
	master->cleanup = ali_spi_cleanup;
	platform_set_drvdata(pdev, master);

	as = spi_master_get_devdata(master);

	/*
	 * Scratch buffer is used for throwaway rx and tx data.
	 * It's coherent to minimize dcache pollution.
	 */
//	as->buffer = dma_alloc_coherent(&pdev->dev, BUFFER_SIZE,
//					&as->buffer_dma, GFP_KERNEL);
//	if (!as->buffer)
//		goto out_free;

	//spin_lock_init(&as->lock);
	INIT_LIST_HEAD(&as->queue);
	as->pdev = pdev;

	/* Initialize the hardware */

	/* go! 
	dev_info(&pdev->dev, "Atmel SPI Controller at 0x%08lx (irq %d)\n",
			(unsigned long)regs->start, irq)*/;

	ret = spi_register_master(master);
	if (ret)
		goto out_free_buffer;

	return 0;

out_free_buffer:
//	dma_free_coherent(&pdev->dev, BUFFER_SIZE, as->buffer,
//			as->buffer_dma);
out_free:
	spi_master_put(master);
	return ret;
}

static void pinmux_to_strapin()
{
	UINT32 value1,value2 ;

	value1 = __REG32ALI(ALI_SOC_REG_BASE_ADDR_PHYS+0x8c);
	value1 = (value1 |(0x01<<31));
	__REG32ALI(ALI_SOC_REG_BASE_ADDR_PHYS+0x8c) = value1;
	
}

static void strapin_to_pinmux()
{
	UINT32 value1,value2 ;

	reg_data_save_8c = __REG32ALI(ALI_SOC_REG_BASE_ADDR_PHYS+0x8c);
	reg_data_save_88 = __REG32ALI(ALI_SOC_REG_BASE_ADDR_PHYS+0x88);
	
	value1 = __REG32ALI(ALI_SOC_REG_BASE_ADDR_PHYS+0x8c);
	value1 = (value1 & (~(0x01<<31)));
	__REG32ALI(ALI_SOC_REG_BASE_ADDR_PHYS+0x8c) = value1;
}


static void pinmux88_to_default()
{
	UINT32 value1;
	
	value1 = __REG32ALI(ALI_SOC_REG_BASE_ADDR_PHYS+0x88);
	value1 = reg_data_save_88;
	__REG32ALI(ALI_SOC_REG_BASE_ADDR_PHYS+0x88) = value1;
		
}

static void pinmux8c_to_default()
{
	UINT32 value1;
		
	value1 = __REG32ALI(ALI_SOC_REG_BASE_ADDR_PHYS+0x8c);
	
	value1 = reg_data_save_8c;
	__REG32ALI(ALI_SOC_REG_BASE_ADDR_PHYS+0x8c) = value1;

}

static void pinmux_to_spi()
{

	UINT32 value1,value2 ;
	
	strapin_to_pinmux();
	
	value2 = __REG32ALI(ALI_SOC_REG_BASE_ADDR_PHYS+0x88);
	value2 = (value2 & (~(0x01<<3)));
	value2 = (value2 |(0x01<<19));
	__REG32ALI(ALI_SOC_REG_BASE_ADDR_PHYS+0x88) = value2;
}

static void pinmux_to_nand()
{
	pinmux88_to_default();
	pinmux8c_to_default();
}

static int __exit ali_spi_remove(struct platform_device *pdev)
{
	struct spi_master	*master = platform_get_drvdata(pdev);
	struct ali_spi	*as = spi_master_get_devdata(master);
	struct spi_message	*msg;

	/* reset the hardware and block queue progress */
	//spin_lock_irq(&as->lock);
	as->stopping = 1;
	//spin_unlock_irq(&as->lock);

	/* Terminate remaining queued transfers */
	list_for_each_entry(msg, &as->queue, queue) {
		/* REVISIT unmapping the dma is a NOP on ARM and AVR32
		 * but we shouldn't depend on that...
		 */
		msg->status = -ESHUTDOWN;
		msg->complete(msg->context);
	}

//	dma_free_coherent(&pdev->dev, BUFFER_SIZE, as->buffer,
//			as->buffer_dma);

	spi_unregister_master(master);
	platform_device_unregister(pdev);

	return 0;
}

static struct platform_driver ali_spi_driver = {
	.driver		= {
		.name	= "ali_spi_bus",
		.owner	= THIS_MODULE,
	},
	.suspend	= NULL, //ali_spi_suspend,
	.resume		= NULL, //ali_spi_resume,
	.remove		= __exit_p(ali_spi_remove),
};

static int __init ali_spi_init(void)
{
	struct platform_device *pd;

	UINT32 bonding;
	UINT32 chipid;
	
	PRINTK_BUS("\n%s.\n\\n", __FUNCTION__);

	
	ali_sflash_virt_addr = ioremap(ALI_FLASH_BASE_ADDR_PHYS,0x40000);
		
	/*Check IC bonding*/
	/*
	bonding = __REG32ALI(ALI_SOC_REG_BASE_ADDR_PHYS);
	bonding = bonding>>16;

	PRINTK_BUS("\n\n BOARD = 0x%08x,%s.\n\n\n\n",bonding,__FUNCTION__);

	PRINTK_BUS("\n\n FLASH_BASE_ADDR = 0x%08x,%s.\n\n\n\n",ali_sflash_virt_addr,__FUNCTION__);

	pinmux_to_spi();
	
	__REG8ALI(ALI_SPI_INS) = 0x9f;
	__REG8ALI(ALI_SPI_FMT) = 0x09;
	
  	chipid = *((volatile UINT32 *)ali_sflash_virt_addr); // get ID 
  	PRINTK_BUS("\n\n CHIP_ID = 0x%08x,%s.\n\n\n\n",chipid,__FUNCTION__);  //c21620c2
  	chipid = chipid & 0xffff ;
  	PRINTK_BUS("\n\n CHIP_ID = 0x%08x,%s.\n\n\n\n",chipid,__FUNCTION__);  //20c2

	pinmux_to_nand();
	*/
	
	pd = platform_device_alloc("ali_spi_bus", 1);
	if (pd == NULL) {
		printk(KERN_ERR "ali_spi_bus: failed to allocate device id %d\n",1);
		return -ENODEV;
	}

	pd->dev.platform_data = NULL;
	platform_device_add(pd);

	return platform_driver_probe(&ali_spi_driver, ali_spi_probe);
}

static void __exit ali_spi_exit(void)
{
	platform_driver_unregister(&ali_spi_driver);
}


module_init(ali_spi_init);
module_exit(ali_spi_exit);

MODULE_DESCRIPTION("Ali SPI Controller driver");
MODULE_AUTHOR("Eric Li");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ali_spi_bus");

