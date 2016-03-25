/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
 ///#include "asm/io.h"
 #include "errno.h"
 #include "ali_interrupt.h"
 #include "ali_gic.h"
 #define u32 unsigned long

 static struct gic_chip_data gic_data;
 
 static unsigned int gic_irq(struct irq_data *d)
 {
	 return d->irq;
 }

static void *irq_data_get_irq_chip_data(struct irq_data *d)
{
	return d->chip_data;
}

static void *gic_dist_base(struct irq_data *d)
{
	struct gic_chip_data *gic_data = irq_data_get_irq_chip_data(d);
	return gic_data_dist_base(gic_data);
}

static void *gic_cpu_base(struct irq_data *d)
{
	struct gic_chip_data *gic_data = irq_data_get_irq_chip_data(d);
	return gic_data_cpu_base(gic_data);
}

static void ali_s3921_irq_enable(struct irq_data *d)
{   
unsigned int  irq_no, irq_reg;

    if (d->irq > (96 + ALI_SYS_IRQ_BASE) || d->irq < (ALI_SYS_IRQ_BASE) )
        return; 

    irq_no = (d->irq - ALI_SYS_IRQ_BASE) % ALI_SYS_IRQ_BASE;
    irq_reg = (d->irq - ALI_SYS_IRQ_BASE) / ALI_SYS_IRQ_BASE;

    switch(irq_reg)
    {
        case 0:
            *(u32 *)(SYS_INT_ENABLE1) |= (u32) (1 << irq_no);
            break;
        case 1:
            *(u32 *)(SYS_INT_ENABLE2) |= (u32) (1 << irq_no);
            break;
        case 2:         
            *(u32 *)(SYS_INT_ENABLE3) |= (u32) (1 << irq_no);
            break;  
        default:
            break;
    }   
}

static void ali_s3921_irq_disable(struct irq_data *d)
{   
    unsigned int  irq_no, irq_reg;

    if (d->irq > (96 + ALI_SYS_IRQ_BASE) || d->irq < (ALI_SYS_IRQ_BASE) )
        return; 

    irq_no = (d->irq -ALI_SYS_IRQ_BASE) %ALI_SYS_IRQ_BASE;
    irq_reg = (d->irq -ALI_SYS_IRQ_BASE) /ALI_SYS_IRQ_BASE;

    switch(irq_reg)
    {
        case 0:
            *(u32 *)(SYS_INT_ENABLE1) &= ~(u32) (1 << irq_no);
            break;
        case 1:
            *(u32 *)(SYS_INT_ENABLE2) &= ~(u32) (1 << irq_no);
            break;
        case 2:         
            *(u32 *)(SYS_INT_ENABLE3) &= ~(u32) (1 << irq_no);
            break;  
        default:
            break;
    }
}




 struct irq_chip gic_arch_extn = {
    .irq_ack = ali_s3921_irq_disable,
	.irq_eoi	= ali_s3921_irq_enable,
	.irq_mask	= ali_s3921_irq_disable,
	.irq_unmask	= ali_s3921_irq_enable,
	.irq_set_type	= 0,
	.irq_enable = 0,
	.irq_disable = 0,
};

/*
 * Generic no controller implementation
 */
struct irq_chip no_irq_chip = {
	.name		= "none",
	.irq_enable	= 0,
	.irq_disable	= 0,
	.irq_ack	= 0,
};

int irq_set_chip(unsigned int irq_start, unsigned int gic_max_irqs, struct irq_chip *chip)
{
	struct irq_desc *desc = NULL;
	unsigned int irq_num=0;
	
	if (!chip)
		chip = &no_irq_chip;

	for(irq_num=irq_start; irq_num<gic_max_irqs; irq_num++)
	{
		desc = irq_to_desc(irq_num);

		if (!desc)
			return -EINVAL;

		desc->irq_data.chip = chip;
		desc->irq_data.irq = irq_num;
	}

	return 0;
}
void handle_bad_irq(unsigned int irq, struct irq_desc *desc)
{
	printf("bad irq:%d\n",irq);
}

void irq_set_handler(unsigned int irq_start, unsigned int gic_max_irqs, irq_flow_handler_t handle, const char *name)
{
	struct irq_desc *desc = NULL;
    unsigned int irq_num=0;

    for(irq_num=irq_start; irq_num<gic_max_irqs; irq_num++)
	{
		desc = irq_to_desc(irq_num);

		if (!desc)
		{
			return;
		}

		if (!handle) 
		{
			handle = handle_bad_irq;
		} 
		else 
	    {
			if(desc->irq_data.chip == &no_irq_chip)
			{
				return;
			}
		}

		/* Uninstall? */
		if (handle == handle_bad_irq) 
		{
			if (desc->irq_data.chip != &no_irq_chip)
			{
				//if (desc->irq_data.chip->irq_mask_ack)
			       //desc->irq_data.chip->irq_mask_ack(&desc->irq_data);
		        //else 
				{
			       desc->irq_data.chip->irq_mask(&desc->irq_data);
			       if (desc->irq_data.chip->irq_ack)
				   desc->irq_data.chip->irq_ack(&desc->irq_data);
		        }
			}
		}
		
		desc->handle_irq = handle;
		desc->name = name;
    }

	return;
}

int irq_set_chip_data(unsigned int irq_start, unsigned int gic_max_irqs, void *data)
{	
	struct irq_desc *desc = NULL;
	unsigned int irq_num=0;

    for(irq_num=irq_start; irq_num<gic_max_irqs; irq_num++)
	{
		desc = irq_to_desc(irq_num);
		if (!desc)
			return -EINVAL;
		desc->irq_data.chip_data = data;
    }
	return 0;
}

void set_irq_flags(unsigned int irq, unsigned int iflags)
{
	unsigned long clr = 0, set = IRQ_NOREQUEST | IRQ_NOPROBE | IRQ_NOAUTOEN;

	if (irq >= 128) {
		printf( "Trying to set irq flags for IRQ%d\n", irq);
		return;
	}
	if (iflags & IRQF_VALID)
		clr |= IRQ_NOREQUEST;
	if (iflags & IRQF_PROBE)
		clr |= IRQ_NOPROBE;
	if (!(iflags & IRQF_NOAUTOEN))
		clr |= IRQ_NOAUTOEN;
	/* Order is clear bits in "clr" then set bits in "set" */
}

/*
 * Routines to acknowledge, disable and enable interrupts
 */
static void gic_mask_irq(struct irq_data *d)
{
	u32 mask = 1 << (gic_irq(d) % 32);
	writel(mask, gic_dist_base(d) + GIC_DIST_ENABLE_CLEAR + (gic_irq(d) / 32) * 4);
	if (gic_arch_extn.irq_mask)
		gic_arch_extn.irq_mask(d);
}



static void gic_unmask_irq(struct irq_data *d)
{
	u32 mask = 1 << (gic_irq(d) % 32);
	if (gic_arch_extn.irq_unmask)
		gic_arch_extn.irq_unmask(d);
	writel(mask, gic_dist_base(d) + GIC_DIST_ENABLE_SET + (gic_irq(d) / 32) * 4);
}

static void gic_eoi_irq(struct irq_data *d)
{
	if (gic_arch_extn.irq_eoi) {
		gic_arch_extn.irq_eoi(d);
	}
	writel(gic_irq(d), gic_cpu_base(d) + GIC_CPU_EOI);
}


static int gic_set_type(struct irq_data *d, unsigned int type)
{
	void *base = gic_dist_base(d);
	unsigned int gicirq = gic_irq(d);
	u32 enablemask = 1 << (gicirq % 32);
	u32 enableoff = (gicirq / 32) * 4;
	u32 confmask = 0x2 << ((gicirq % 16) * 2);
	u32 confoff = (gicirq / 16) * 4;
	unsigned long enabled = 0;
	u32 val;

	/* Interrupt configuration for SGIs can't be changed */
	if (gicirq < 16)
		return -EINVAL;

	if (type != IRQ_TYPE_LEVEL_HIGH && type != IRQ_TYPE_EDGE_RISING)
		return -EINVAL;

	if (gic_arch_extn.irq_set_type)
		gic_arch_extn.irq_set_type(d, type);

	val = readl(base + GIC_DIST_CONFIG + confoff);
	if (type == IRQ_TYPE_LEVEL_HIGH)
		val &= ~confmask;
	else if (type == IRQ_TYPE_EDGE_RISING)
		val |= confmask;

	/*
	 * As recommended by the spec, disable the interrupt before changing
	 * the configuration
	 */
	if (readl(base + GIC_DIST_ENABLE_SET + enableoff) & enablemask) {
		writel(enablemask, base + GIC_DIST_ENABLE_CLEAR + enableoff);
		enabled = 1;
	}

	writel(val, base + GIC_DIST_CONFIG + confoff);

	if (enabled)
		writel(enablemask, base + GIC_DIST_ENABLE_SET + enableoff);

	return 0;
}


static struct irq_chip gic_chip = {
	.name			= "GIC",
	.irq_mask		= gic_mask_irq,
	.irq_unmask		= gic_unmask_irq,
	.irq_eoi		= gic_eoi_irq,
	.irq_set_type	= gic_set_type,
	.irq_ack        = 0,
	.irq_enable     = 0,
	.irq_disable    = 0,
};


/*
* handle fasteoi irq
*/
void handle_fasteoi_irq(unsigned int irq, struct irq_desc *desc)
{
	if (!desc->action) 
	{
	    printf("no action\n");
		//mask_irq(desc);
		return;
	}

	/*mask*/
    if (desc->irq_data.chip->irq_mask) 
	{
	    desc->irq_data.chip->irq_mask(&desc->irq_data);
    }

	/*action->handle*/
	if (desc->action->handler) 
	{
	    desc->action->handler(irq, desc->action->dev_id);
    }

	/*unmask*/
	if (desc->irq_data.chip->irq_unmask) 
	{
	    desc->irq_data.chip->irq_unmask(&desc->irq_data);
    }

	/*eoi*/
	if (desc->irq_data.chip->irq_eoi) 
	{
	    desc->irq_data.chip->irq_eoi(&desc->irq_data);
    }
	
	return;
}

/*
* read gic number
*/
int gic_read_irq(void)
{
   unsigned long irqstate = 0;
   unsigned long irqnr = 0;
   irqstate = readl(A9_MPCORE_GIC_CPU + GIC_CPU_INTACK);
   irqnr = irqstate & ~0x1c00;
   return irqnr;
}


/*
*  Init the GIC Dist Register
*/
static void gic_dist_init(struct gic_chip_data *gic)
{
	unsigned int i = 0;
	unsigned char *base = (unsigned char*)gic->dist_base;
	unsigned long val = 0, shiftbit = 0;
	u32 cpu = 0; //cpu_logical_map(smp_processor_id());
	u32 cpumask = 0;
	unsigned int gic_irqs = gic->gic_irqs;

	cpumask = 1 << cpu;
	cpumask |= cpumask << 8;
	cpumask |= cpumask << 16;

	writel(0, base + GIC_DIST_CTRL);

	//*(volatile unsigned long*)(0x1bf01000)=0x0;

	/*
	 * Set all global interrupts to be level triggered, active low.
	 */
	for (i = 32; i < gic_irqs; i += 16)
	//writel(0, base + GIC_DIST_CONFIG + i * 4 / 16);
	writel(0, base + GIC_DIST_CONFIG + i * 4 / 16);

#if 0	
	debug("i:%d reg:0x%08x 001\n",i,base + GIC_DIST_CONFIG + (i/16)*4);

	val = readl(base + GIC_DIST_CONFIG + (i/16)*4);
	shiftbit = i%16;
	val &=~(0x3<<(shiftbit*2));
	writel(val, base + GIC_DIST_CONFIG + (i/16)*4);
#endif
	/*
	 * Set all global interrupts to this CPU only.
	 */

	/*//john
	printk("enter gic_dist_init:%x\n",readb_relaxed(base + GIC_DIST_TARGET));
	for (i = 0; i < 32; i += 4)
	{
		writeb_relaxed(0x3, base + GIC_DIST_TARGET + i + 0);
		writeb_relaxed(0x3, base + GIC_DIST_TARGET + i + 1);
		writeb_relaxed(0x3, base + GIC_DIST_TARGET + i + 2);
		writeb_relaxed(0x3, base + GIC_DIST_TARGET + i + 3);
	}
	//while(1);
	*/
	for (i = 32; i < gic_irqs; i += 4)
		writel(cpumask, base + GIC_DIST_TARGET + i * 4 / 4);
#if 0
	val = readl(base + GIC_DIST_TARGET + (i/4)*4);
	shiftbit = i%4;
	val&=~(0xff<<(shiftbit*8));
	val|=(0x1<<(shiftbit*8));
	writel(val, base + GIC_DIST_TARGET + (i/4)*4);
#endif
	/*
	 * Set priority on all global interrupts.
	 */
	for (i = 32; i < gic_irqs; i += 4)
		writel(0xa0a0a0a0, base + GIC_DIST_PRI + i * 4 / 4);
#if 0
	val = readl(base + GIC_DIST_PRI + (i/4)*4);
	shiftbit = i%4;
	val&=~(0xff<<(shiftbit*8));
	val|=(0xa0<<(shiftbit*8));
	writel(val, base + GIC_DIST_PRI + (i/4)*4);
#endif

	/*
	 * Disable all interrupts.  Leave the PPI and SGIs alone
	 * as these enables are banked registers.
	 */
	for (i = 32; i < gic_irqs; i += 32)
		writel(0xffffffff, base + GIC_DIST_ENABLE_CLEAR + i * 4 / 32);
#if 0
	val = readl(base + GIC_DIST_ENABLE_CLEAR + (i/32)*4);
	shiftbit = i%32;
	val|=(0x1<<shiftbit);
	writel(val, base + GIC_DIST_ENABLE_CLEAR + (i/32)*4);
#endif

	writel(1, base + GIC_DIST_CTRL);

}


/*
*  Init the GIC CPU Register
*/
static void gic_cpu_init(struct gic_chip_data *gic)
{
	void *dist_base = gic_data_dist_base(gic);
	void *base = gic_data_cpu_base(gic);
	int i;
	
	/*
	 * Deal with the banked PPI and SGI interrupts - disable all
	 * PPI interrupts, ensure all SGI interrupts are enabled.
	 */
	writel(0xffff0000, dist_base + GIC_DIST_ENABLE_CLEAR);
	writel(0x0000ffff, dist_base + GIC_DIST_ENABLE_SET);

	/*
	 * Set priority on PPI and SGI interrupts
	 */
	for (i = 0; i < 32; i += 4)
		writel(0xa0a0a0a0, dist_base + GIC_DIST_PRI + i * 4 / 4);

	writel(0xf0, base + GIC_CPU_PRIMASK);
	
	writel(1, base + GIC_CPU_CTRL);
}



/*
* ALI-3921 Init GIC Register
*/
void gic_init(unsigned int irq_start, void *dist_addr, void *cpu_addr)
{
	struct gic_chip_data *gic = NULL;
	unsigned int gic_max_irqs = 0;
	
	gic = &gic_data;
	gic->cpu_base = cpu_addr;
	gic->dist_base = dist_addr;

    //get the max number of the irqs
	gic_max_irqs = *(volatile unsigned int *)((unsigned char*)gic->dist_base + GIC_DIST_CTR) & 0x1f;	
 	gic_max_irqs = (gic_max_irqs + 1) * 32;
	if(gic_max_irqs > 1020) 
	{
	   gic_max_irqs = 1020;
	}

	gic->gic_irqs = gic_max_irqs;

	if(irq_start>=gic_max_irqs)
	{
	   printf("[gic_init] irq_start error!\n");
	   return;
	}

    irq_set_chip(irq_start, gic_max_irqs, &gic_chip);
	
	irq_set_handler(irq_start, gic_max_irqs, handle_fasteoi_irq, "int");
	
	//set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
	
	irq_set_chip_data(irq_start, gic_max_irqs, gic);

	//GIC_DIST init
	gic_dist_init(gic);

	//GIC_CPU init
	gic_cpu_init(gic);

	printf("GIC init ok!\n");

	return;

	
}

