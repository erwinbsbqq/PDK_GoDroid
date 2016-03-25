/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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

#include <common.h>
#include <asm/proc-armv/ptrace.h>
#include <errno.h>

///#define S3921_TIMER_TEST

//#ifdef CONFIG_USE_IRQ

#include "ali_interrupt.h"
#include "ali_gic.h"

static struct irq_desc g_irq_desc[NR_IRQS];

struct irq_desc *irq_to_desc(unsigned int irq)
{
  if(irq>NR_IRQS)
  {
     printf("[irq_to_desc] error!\n");
	 return NULL;
  }
  return &g_irq_desc[irq];
}


/*
* default isr func
*/
static void default_isr( void *data) 
{
	printf ("default_isr():  called for IRQ %d\n", (int)data);
}


#ifdef S3921_TIMER_TEST

#define A9_MPCORE_GIT		(0x1bf00000 + 0x0200)

static int g_mytimercnt = 0;
static irqreturn_t ali_s3921_timer_interrupt(int irq, void *dev_id)
{
    writel(0x1, (A9_MPCORE_GIT)+ 0x0C);
    g_mytimercnt++;	
	printf("\nTimer:%d\n",g_mytimercnt);
	return IRQ_HANDLED;
}

static void ali_s3921_timer_init(void)
{
	unsigned int timer = 0x2468;//0x5F5E10;//0xF424;//0x5F5E10;
	int err;
	unsigned int cycle = 500000000/1; //1s

	writel(0, (A9_MPCORE_GIT) + 0x00);
	
	writel(0, (A9_MPCORE_GIT) + 0x04);

	/////////
	writel(0x00, (A9_MPCORE_GIT) + 0x08);
	
	writel(cycle, (A9_MPCORE_GIT) + 0x10);
	writel(cycle, (A9_MPCORE_GIT) + 0x18);
	writel(0x0F,  (A9_MPCORE_GIT) + 0x08);
	/////////

	err = request_irq(27, ali_s3921_timer_interrupt, 0 ,"ALI s3921 Timer Tick",0);
	if(err)
	{
		printf("setup_percpu_irq failed with err = %d\n", err);
	}
   	
}
#endif

/*
*S3921 Interrupt Init
*/
int arch_interrupt_init (void)
{	
	gic_init(16,A9_MPCORE_GIC_DIST, A9_MPCORE_GIC_CPU);
	//gic_init(90,0x1BF01000, 0x1BF00100);
	//gic_init(89,0x1BF01000, 0x1BF00100);
	//gic_init(88,0x1BF01000, 0x1BF00100);
	//gic_init(27,0x1BF01000, 0x1BF00100);

#ifdef S3921_TIMER_TEST
    ali_s3921_timer_init();
#endif

	return 0;
}

/*
* register interrupt func for user
*/
int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
	    const char *name, void *dev)
{
	struct irqaction *action;
	struct irq_desc *desc;
	int retval = 0;

	desc = irq_to_desc(irq);
	if (!desc)
		return -EINVAL;

	if (!handler) {
			return -EINVAL;
	}
	action = malloc(sizeof(struct irqaction));
	if (!action)
		return -ENOMEM;

	action->handler = handler;
	action->flags = flags;
	action->name = name;
	action->dev_id = dev;
	action->irq = irq;

	desc->action = action;

	if (desc->irq_data.chip->irq_enable){
		desc->irq_data.chip->irq_enable(&desc->irq_data); 
		}    /*enable int*/
	else{
		desc->irq_data.chip->irq_unmask(&desc->irq_data); 
		}/*unmask int*/

	return retval;
}


/*
* free interrupt func for user
*/
static int free_irq(unsigned int irq, void *dev_id)
{
	struct irq_desc *desc = irq_to_desc(irq);
	struct irqaction *action, **action_ptr;
	unsigned long flags;

	if(desc->action->dev_id == dev_id)
	{
	   desc->action = NULL;
	}
	else
		return -1;
	
	if (desc->irq_data.chip->irq_disable)
		desc->irq_data.chip->irq_disable(&desc->irq_data); /*disable int*/
	else
		desc->irq_data.chip->irq_mask(&desc->irq_data);    /*maks int*/

	return 0;
}

static void generic_handle_irq_desc(unsigned int irq, struct irq_desc *desc)
{
	desc->handle_irq(irq, desc);
}

int generic_handle_irq(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);

	if (!desc)
		return -EINVAL;
	generic_handle_irq_desc(irq, desc);
	return 0;
}

void handle_IRQ(unsigned int irq, struct pt_regs *regs)
{
	//struct pt_regs *old_regs = set_irq_regs(regs);

	if(irq >= NR_IRQS) 
	{
			printf( "Bad IRQ %u\n", irq);
			return;
	} else 
	{
		generic_handle_irq(irq);
	}

	/* AT91 specific workaround */
	//irq_finish(irq);

	//irq_exit();
	//set_irq_regs(old_regs);
}


/*
* Entry of irq process
*/
void ali_do_irq (struct pt_regs *pt_regs)
{
   /*process irq*/
   int irq = gic_read_irq();
   handle_IRQ(irq, pt_regs);
}


//#endif


