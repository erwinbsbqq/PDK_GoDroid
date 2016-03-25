/*
 * (C) Copyright 2008
 * Texas Instruments
 *
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Moahmmed Khasim <khasim@ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <boot_common.h>

#ifdef ALI_ARM_STB
//==============================================add for timer
#define TWD_BASE_ADDR     0x1bf00600//0x1bf00600  //0x1bf00600
#define TWD_TIMER_LOAD_VAL		0x00
#define TWD_TIMER_COUNTER		0x04
#define TWD_TIMER_CONTROL		0x08
#define TWD_TIMER_INTSTAT		       0x0C

#define TWD_TIMER_CONTROL_ENABLE	(1 << 0)
#define TWD_TIMER_CONTROL_ONESHOT	(0 << 1)
#define TWD_TIMER_CONTROL_PERIODIC	(1 << 1)
#define TWD_TIMER_CONTROL_IT_ENABLE	(1 << 2)

#define TIMER_LOAD_VAL		0xffffffff
#else

DECLARE_GLOBAL_DATA_PTR;

static struct gptimer *timer_base = (struct gptimer *)CONFIG_SYS_TIMERBASE;

/*
 * Nothing really to do with interrupts, just starts up a counter.
 */

#define TIMER_CLOCK		(V_SCLK / (2 << CONFIG_SYS_PTV))
#define TIMER_LOAD_VAL		0
#endif
#define TIMER_OVERFLOW_VAL	0xffffffff

int timer_init(void)
{

#ifndef ALI_ARM_STB
	/* start the counter ticking up, reload value on overflow */
	writel(TIMER_LOAD_VAL, &timer_base->tldr);
	/* enable timer */
	writel((CONFIG_SYS_PTV << 2) | TCLR_PRE | TCLR_AR | TCLR_ST,
	&timer_base->tclr);

	/* reset time, capture current incrementer value time */
	gd->lastinc = readl(&timer_base->tcrr) / (TIMER_CLOCK / CONFIG_SYS_HZ);
	gd->tbl = 0;		/* start "advancing" time stamp from 0 */
#else
	unsigned long cpu_reg_clk;
	unsigned long ctrl = GET_WORD(TWD_BASE_ADDR + TWD_TIMER_CONTROL);
	ctrl |= TWD_TIMER_CONTROL_ENABLE|TWD_TIMER_CONTROL_PERIODIC;
	ctrl &= ~(0xff<<8);	// bit[8:15]:perscaler
	ctrl |= (49<<8);		//timer clk = cpu_clk/((perscaler+1)*periph_clk_ratio) = cpu_clk/100

	SET_WORD(TWD_BASE_ADDR + TWD_TIMER_COUNTER, TIMER_LOAD_VAL);
	SET_WORD(TWD_BASE_ADDR + TWD_TIMER_LOAD_VAL, TIMER_LOAD_VAL);
	SET_WORD(TWD_BASE_ADDR + TWD_TIMER_CONTROL, ctrl);
#endif
	return 0;
}

/*
 * timer without interrupts
 */
ulong get_timer(ulong base)
{
#ifndef ALI_ARM_STB
	return get_timer_masked() - base;
#else

	unsigned long last_counter,cur_counter,diff;

	cur_counter = GET_WORD(TWD_BASE_ADDR + TWD_TIMER_COUNTER);

#if 0 //just for debug
	static unsigned long last_cnt = 0;
	static int auto_load = 0;
	if (last_cnt != 0 && last_cnt < cur_counter)
		auto_load = 1;
	if (base)
	printf("---------cur_counter=0x%08x,last_cnt=0x%08x,auto_load=0x%x######\n",cur_counter,last_cnt,auto_load);
	last_cnt = cur_counter;
#endif

	if (0 == base){
		diff = (0xffffffff - cur_counter);	// count down timer
		diff = diff/(DELAY_1US*1000);	// return ms after base
		return diff;	// return ms after base
	}
	else
	{
		last_counter = base*(DELAY_1US*1000);
		if (last_counter >= cur_counter) // count down timer 
		{
			diff = last_counter - cur_counter;
			diff = diff/(DELAY_1US*1000);	// return ms after base
			return diff;
		}else{	// timer count down to 0 and reload to 0xFFFF_FFFFF
			diff = (last_counter -0) + (TIMER_OVERFLOW_VAL-cur_counter);
			diff = diff/(DELAY_1US*1000);	// return ms after base
			return diff;
		}
	}
#endif
}

static void delay_us(unsigned long usec)
{
	long tmo = DELAY_1US*usec;	// 1G:10 900MHz:9 800MHz:8 ...
	unsigned long now, last ;

    last = GET_WORD(TWD_BASE_ADDR + TWD_TIMER_COUNTER);

	while (tmo > 0)
      {
		now = GET_WORD(TWD_BASE_ADDR + TWD_TIMER_COUNTER);
		if (last >= now){ // count down timer 
			tmo -=  last - now; 
		}else{		// timer count down to 0 and reload to 0xFFFF_FFFFF
			tmo -=  (last -0)+(TIMER_OVERFLOW_VAL- now) ;
		}
		last = now;
	}
}

/* delay x useconds */
void __udelay(unsigned long usec)
{
#ifndef ALI_ARM_STB
	long tmo = usec * (TIMER_CLOCK / 1000) / 1000;
	unsigned long now, last = readl(&timer_base->tcrr);

	while (tmo > 0) {
		now = readl(&timer_base->tcrr);
		if (last > now) /* count up timer overflow */
			tmo -= TIMER_OVERFLOW_VAL - last + now + 1;
		else
			tmo -= now - last;
		last = now;
	}
#else
	//unsigned long tick = get_timer(0);
	//while((get_timer(0) - tick) < usec*DELAY_1US);

	delay_us(usec);
#endif
}

void __mdelay(unsigned long msec)
{
	while (msec--)
		__udelay(1000);
}

#ifndef ALI_ARM_STB
ulong get_timer_masked(void)
{
	/* current tick value */
	ulong now = readl(&timer_base->tcrr) / (TIMER_CLOCK / CONFIG_SYS_HZ);

	if (now >= gd->lastinc)	/* normal mode (non roll) */
		/* move stamp fordward with absoulte diff ticks */
		gd->tbl += (now - gd->lastinc);
	else	/* we have rollover of incrementer */
		gd->tbl += ((TIMER_LOAD_VAL / (TIMER_CLOCK / CONFIG_SYS_HZ))
			     - gd->lastinc) + now;
	gd->lastinc = now;
	return gd->tbl;
}
#endif

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
