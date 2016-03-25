/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation. 2010 Copyright (C)
 *  (C)
 *  File: m36_irq.c
 *  (I)
 *  Description:
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.06.03			Sam			Create

*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version
* 2 of the License, or (at your option) any later version.
*

 ****************************************************************************/
 
#include <linux/bitops.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/kernel.h>
#include <linux/random.h>

//#include <asm/i8259.h>
#include <asm/irq_cpu.h>
#include <asm/io.h>
#include <asm/irq_regs.h>

#include <asm/mach-ali/m6303.h>


/* ---------------------  IRQ init stuff ---------------------- */

static int sys_irq_base;

int sys_rpc_addr;
static unsigned int sys_rpc_mask;
static unsigned int sys_rpc_irq1_mask;
static unsigned int sys_rpc_irq2_mask;

/* ---------------------- sys irq ------------------------ */
static void sys_irq_enable(struct irq_data *d)
{
	unsigned int irq = d->irq;
	irq -= sys_irq_base;

	if (irq >= 32) {
		*(u32 *)(0xB800003C) |= (u32)(1<<(irq - 32));
#if 1
	if (irq >= 60) 
	{
	    //printk("irqe:%d,j:%x\n", irq, (unsigned int)jiffies);
	    return;
	}
#endif

	}
	else {
		*(u32 *)(0xB8000038) |= (u32)(1<<irq);
	}
}

static void sys_irq_disable(struct irq_data *d)
{
	unsigned int irq = d->irq;
	irq -= sys_irq_base;

	if (irq >= 32) {
		*(u32 *)(0xB800003C) &= ~((u32)(1<<(irq-32)));

#if 1
	if (irq >= 60) 
	{
       //printk("irqd:%d,j:%x\n", irq, (unsigned int)jiffies);
	   return;
	}
#endif

	}
	else {
		*(u32 *)(0xB8000038) &= ~((u32)(1<<irq));
	}
}

static unsigned int sys_irq_startup(struct irq_data *d)
{  
// 	unsigned long chip_id;
//	unsigned long tmp_u32;
	
	unsigned int irq = d->irq;
	printk("in sys_irq_startup (irq: %d).\n",irq);

#if 0	
	tmp_u32 = *(volatile unsigned long*)0xB8000000;
	chip_id = tmp_u32 >> 16;	
	if (chip_id == 0x3602)	// 3603 interrupt polarity is ok, do nothing
	{
		if (irq >= 40) {
			*(u32 *)(0xB800002C) |= (u32)(1<<(irq - 32 - sys_irq_base));	/* Set polarity */
		}
		else {
			*(u32 *)(0xB8000028) |= (u32)(1<<(irq - sys_irq_base));	/* Set polarity */
		}
	}
#endif

	sys_irq_enable(d);

	return 0;
}

/*
static inline void sys_unmask_irq(unsigned int irq)
{
	set_c0_status(0x100 << (irq - MIPS_CPU_IRQ_BASE));
	irq_enable_hazard();
}

static inline void sys_mask_irq(unsigned int irq)
{
	clear_c0_status(0x100 << (irq - MIPS_CPU_IRQ_BASE));
	irq_disable_hazard();
}
*/

#define sys_irq_shutdown	sys_irq_disable
#define sys_irq_ack		sys_irq_disable

static void sys_irq_end(struct irq_data *d)
{
	//printk("in sys_irq_end (irq: %d).\n",irq);
	
	if(!(d->state_use_accessors & (IRQD_IRQ_DISABLED | IRQD_IRQ_INPROGRESS)))
	    sys_irq_enable(d);
}


static void sys_irqdispatch(void)
{
	unsigned long intc0_req1, intc0_req2;
	unsigned long intc0_msk1, intc0_msk2;
	unsigned int bit;
	int i = 0;

	intc0_req1 = *((volatile unsigned long *)(0xB8000030));	/* IRQ status */
	intc0_req2 = *((volatile unsigned long *)(0xB8000034));	/* IRQ status */

	intc0_msk1 = *((volatile unsigned long *)(0xB8000038));
	intc0_msk2 = *((volatile unsigned long *)(0xB800003c));

	intc0_req1 &= intc0_msk1;
	intc0_req2 &= intc0_msk2;

	// mask rpc status;
	intc0_req2 &= 0x00FFFFFF;
	
#ifdef CONFIG_ENABLE_RPC
	{
		unsigned long rpc_status;

		rpc_status = *(volatile unsigned char *)(sys_rpc_addr);
		rpc_status &= sys_rpc_mask;
		
		if(rpc_status & sys_rpc_irq1_mask)
			do_IRQ(71);
		
		if(rpc_status & sys_rpc_irq2_mask)
			do_IRQ(70);
	}
#endif
	
	if (intc0_req1 > 0)
	{
		bit = M36_SYS_IRQ_BASE;
		
		for(i = 0; i < 32;i++, bit++)
		{
			if(intc0_req1 & (1<<i))
			{
				do_IRQ(bit);
			}
		}		
	}
	
	if(intc0_req2 > 0)
	{
		bit = M36_SYS_IRQ_BASE + 32;
		for(i = 0; i < 32;i++, bit++)
		{
			if(intc0_req2 & (1<<i))
			{
				do_IRQ(bit);
			}
		}	
	}
}

static struct irq_chip sys_irq_controller = {
	.name		= "M36_sys_irq",
	.irq_ack			= sys_irq_ack,
	.irq_startup	= sys_irq_startup,
	.irq_shutdown	= sys_irq_shutdown,
	.irq_enable	= sys_irq_enable,
	.irq_disable	= sys_irq_disable,
	.irq_eoi		= sys_irq_end,
/*
	.mask		= sys_mask_irq,
	.mask_ack	= sys_mask_irq,
	.unmask	= sys_unmask_irq,
	.eoi		= sys_unmask_irq,
*/
};

#if 0
asmlinkage void plat_irq_dispatch(void)
{
	unsigned int pending = read_c0_status() & read_c0_cause();

	if (pending & CAUSEF_IP7)	
	{
		do_IRQ(M36_IRQ_TIMER);
	}
	else if ((pending & CAUSEF_IP3) || (pending & CAUSEF_IP2))
	{
		sys_irqdispatch();
	}
	else
	{
#ifdef CONFIG_ENABLE_RPC
		{
			unsigned long rpc_status;

			rpc_status = *(volatile unsigned char *)(sys_rpc_addr);
			rpc_status &= sys_rpc_mask;
			
			if(rpc_status & sys_rpc_irq1_mask)
				do_IRQ(71);
			
			if(rpc_status & sys_rpc_irq2_mask)
				do_IRQ(70);	
		}
#endif
		spurious_interrupt();
	}
}


#else

unsigned int spurious_rpc_71_cnt = 0;

unsigned int spurious_rpc_70_cnt = 0;


unsigned int rpc_71_cnt = 0;

unsigned int rpc_70_cnt = 0;


#if 0
asmlinkage void plat_irq_dispatch(void)
{
    unsigned int status = read_c0_status();
    unsigned int cause = read_c0_cause();

	
	unsigned int pending = read_c0_status() & read_c0_cause();

    {
    	unsigned long rpc_status;
    
    	rpc_status = *(volatile unsigned char *)(sys_rpc_addr);
    	rpc_status &= sys_rpc_mask;
    	
    	if(rpc_status & sys_rpc_irq1_mask)
    	{
    	    rpc_71_cnt++;
    		do_IRQ(71);
		}
    	
    	if(rpc_status & sys_rpc_irq2_mask)
    	{
    	    rpc_70_cnt++;
    		do_IRQ(70);
		}
    }
	
	if (pending & CAUSEF_IP7)	
	{
		do_IRQ(M36_IRQ_TIMER);
	}
	else if ((pending & CAUSEF_IP3) || (pending & CAUSEF_IP2))
	{
		sys_irqdispatch();
	}
	else
	{
#ifdef CONFIG_ENABLE_RPC
		{
			unsigned long rpc_status;

			rpc_status = *(volatile unsigned char *)(sys_rpc_addr);
			rpc_status &= sys_rpc_mask;
			
			if(rpc_status & sys_rpc_irq1_mask)
			{
                spurious_rpc_71_cnt++;
				panic("%s,%d,status:%x,cause:%x,j:%x,sys_rpc_addr:%x,sys_rpc_mask:%x,sys_rpc_irq1_mask:%x,spurious_rpc_71_cnt:%d\n",
					   __FUNCTION__, __LINE__, status, cause, (unsigned int)jiffies, sys_rpc_addr, sys_rpc_mask, sys_rpc_irq1_mask, spurious_rpc_71_cnt);

    			//do_IRQ(71);

			}
			
			if(rpc_status & sys_rpc_irq2_mask)
			{
			
    			spurious_rpc_70_cnt++;
				//do_IRQ(70);	

				
				panic("%s,%d,status:%x,cause:%x,j:%x,sys_rpc_addr:%x,sys_rpc_mask:%x,sys_rpc_irq1_mask:%x,spurious_rpc_70_cnt:%d\n",
					   __FUNCTION__, __LINE__, status, cause, (unsigned int)jiffies, sys_rpc_addr, sys_rpc_mask, sys_rpc_irq1_mask, spurious_rpc_70_cnt);

			}
		}
#endif   		


        spurious_interrupt();
	}
}
#else

static unsigned long flags;
asmlinkage void plat_irq_dispatch(void)
{
	unsigned int pending = 0;
	
	local_irq_save(flags);

	pending = read_c0_status() & read_c0_cause();
	if (pending & CAUSEF_IP7)	
	{
		do_IRQ(M36_IRQ_TIMER);
	}
	else if ((pending & CAUSEF_IP3) || (pending & CAUSEF_IP2))
	{
		sys_irqdispatch();
	}
	else
	{
#if 0
#ifdef CONFIG_ENABLE_RPC
		{
			unsigned long rpc_status;

			rpc_status = *(volatile unsigned char *)(sys_rpc_addr);
			rpc_status &= sys_rpc_mask;
			
			if(rpc_status & sys_rpc_irq1_mask)
				do_IRQ(71);
			
			if(rpc_status & sys_rpc_irq2_mask)
				do_IRQ(70);	
		}
#endif
#endif
		spurious_interrupt();
	}	

	local_irq_restore(flags);	
}

#endif
#endif



void __init arch_init_irq(void)
{
	int i;
	//extern irq_desc_t irq_desc[];

	/* init CPU irqs */
	mips_cpu_irq_init();

	/* init sys irqs */
	sys_irq_base = M36_SYS_IRQ_BASE;
	for (i=sys_irq_base; i < sys_irq_base + M36_NUM_SYS_IRQ; i++)
		irq_set_chip_and_handler(i, &sys_irq_controller,handle_percpu_irq);

	/* Default all ICU IRQs to off and enable IM bit(IP3) of CP0 status for sys IRQ */
#if 	1
	*((unsigned long *)(0xB8000038)) = 0;
	*((unsigned long *)(0xB800003C)) = 0;
	*((unsigned long *)(0xB80000EC)) = 0;
	write_c0_status(read_c0_status() | STATUSF_IP3);
#else
	*M6303_MSYSINT1REG = 0;
	*M6303_MSYSINT2REG = 0;
#endif 

#ifdef CONFIG_REMOTE_DEBUG
	printk("Setting debug traps - please connect the remote debugger.\n");

	set_debug_traps();

	// you may move this line to whereever you want
	breakpoint();
#endif

	if((*(unsigned short *)0xB8000002 == 0x3901)  \
      || (*(unsigned short *)0xB8000002 == 0x3701) \
	  || (*(unsigned short *)0xB8000002 == 0x3503))
	{
		sys_rpc_addr = 0xB8040037;
		sys_rpc_mask = 0x0C;
		sys_rpc_irq1_mask = 0x08;
		sys_rpc_irq2_mask = 0x04;
	}
	else
	{
		sys_rpc_addr = 0xB8040036;
		sys_rpc_mask = 0xC0;
		sys_rpc_irq1_mask = 0x80;
		sys_rpc_irq2_mask = 0x40;		
	}
}

