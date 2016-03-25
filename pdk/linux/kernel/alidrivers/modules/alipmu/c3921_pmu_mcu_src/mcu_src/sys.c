#include "sys.h"
#include <dp8051xp.h>
#include <stdio.h>
#include "ir.h"
#include "pannel.h"
#include "uart.h"
#include "pmu_rtc.h"
#include <intrins.h>

//============================================================//
static UINT8 g_power_key = 0;
void power_key_mode(enum KEY_MODE mode);
void exit_standby_status(unsigned char status0, unsigned char status1, unsigned char status2);
volatile unsigned long g_ir_power_key[8] _at_ 0x3fc0;
extern void IR_Init(void);
extern void IR_ISR(void);
//main cpu enter standby
UINT8  g_standby_flag = NORMAL;

// exit standby flag ,key,ir or time
UINT8  g_exit_standby_sts=0;

//=====================================================================================//
static void hal_gpio_write(UINT8 val)
{
	*((volatile UINT8 xdata*)(HAL_GPIO_DO_REG)) = val;
}

static UINT8 hal_gpio_read(void)
{
	UINT8 ret = 0;

	ret =*((volatile UINT8 xdata*)(HAL_GPIO_DI_REG));
	return ret;
}

static UINT8 hal_gpio_dir_get(void)
{
	UINT8 ret = 0;

	ret =*((volatile UINT8 xdata*)(HAL_GPIO_DIR_REG));
	return ret;
}

static void hal_gpio_dir_set(UINT8 val)
{
	*((volatile UINT8 xdata*)(HAL_GPIO_DIR_REG)) = val;
}

static void hal_gpio_en(UINT8 val)
{
	*((volatile UINT8 xdata*)(HAL_GPIO_EN)) = val;
}
    
static void hal_gpio1_write(UINT8 val)
{
	*((volatile UINT8 xdata*)(HAL_GPIO1_DO_REG)) = val;
}

static UINT8 hal_gpio1_read(void)
{
	UINT8 ret = 0;

	ret =*((volatile UINT8 xdata*)(HAL_GPIO1_DI_REG));
	return ret;
}

static UINT8 hal_gpio1_dir_get(void)
{
	UINT8 ret = 0;

	ret =*((volatile UINT8 xdata*)(HAL_GPIO1_DIR_REG));
	return ret;
}

static void hal_gpio1_dir_set(UINT8 val)
{
	*((volatile UINT8 xdata*)(HAL_GPIO1_DIR_REG)) = val;
}

static void hal_gpio1_en(UINT8 val)
{
	*((volatile UINT8 xdata*)(HAL_GPIO1_EN)) = val;
}

 void hal_mcu_gpio_en(UINT8 pos)
{
	if(pos<8)
	{
		hal_gpio_en((*((volatile UINT8 xdata*)(HAL_GPIO_EN)))|(1<<pos));
	}
	else
	{
		hal_gpio1_en((*((volatile UINT8 xdata*)(HAL_GPIO1_EN)))|(1<<(pos-8)));
	}
}

void hal_mcu_gpio_disable_en(void)
{	        
#ifdef PMU_MCU_M3503
	{
		hal_gpio_en(0);
	}
#else
	{
		hal_gpio_en(0);
		hal_gpio1_en(0);
	}
#endif                       
 }

UINT8 hal_gpio_bit_get(UINT8 pos)
{
	UINT8 ret = 0;

	if(pos<8)
	{
		ret = (( hal_gpio_read() >> pos) & 1);
	}
	else
	{
		ret = ((hal_gpio1_read() >> (pos-8)) & 1);
	}
	
	return ret;    
}

void hal_gpio_bit_set(UINT8 pos, UINT8 val)
{
	if(pos < 8)
	{
		hal_gpio_write((( (*(volatile UINT8 xdata*)(HAL_GPIO_DO_REG)) &~(1 << pos)  ) | (val << pos)));
	}
	else
	{
		hal_gpio1_write( ( (*(volatile UINT8 xdata*)(HAL_GPIO1_DO_REG)) & (~(1 << (pos-8)))) | (val << (pos-8)));
	}
}

void hal_gpio_bit_dir_set(UINT8 pos, UINT8 val)
{
	if(pos<8)
	{
		hal_gpio_dir_set((hal_gpio_dir_get() & (~(1 << pos))) | (val << pos));
	}
	else
	{
		hal_gpio1_dir_set((hal_gpio1_dir_get() & (~(1 << (pos-8)))) | (val << (pos-8)));
	}
}

void mcu_disable_int(void)
{
	PMU_WRITE_BYTE(0xc022,0);
}

void pmu_init(void)
{         
	InitUart(); 
	init_externint2();/*extern int2 */
	init_externint3();
	init_externint4();
	init_externint5();     
	//PMU_WRITE_BYTE(PMU_TM0_CTRL, 0x0);
	PMU_WRITE_BYTE(PMU_RTC1_CTRL,0x0);
	//PMU_WRITE_BYTE(PMU_TM2_CTRL, 0x0 );
	WRITE_BYTE(SYS_REG_IER,  READ_BYTE(SYS_REG_IER)&0xfe);//dis sys ir interrupt
	WRITE_BYTE(IR_REG_IER, 0x00);//disable ir ip interrupt 
	power_key_mode(NORMAL);
 }

void standby_init(void)
{
	PMU_WRITE_BYTE(SYS_REG_ISR, PMU_READ_BYTE(SYS_REG_ISR));// clear interrupt
	IR_Init();
	pannel_init();
	power_key_mode(STABDY);
	pmu_timer1_set_time(1);
}

void pannel_process(pRTC_TIMER rtc, enum SHOW_TYPE type)
{
	INT8 ret = 0;

	show_pannel(type,rtc);
	if(SUCCESS == pannel_scan())
	{
		g_exit_standby_sts = EXIT_STANDBY_TYPE_PANEL;
		PMU_WRITE_BYTE(EXIT_STANDBY_TYPE, g_exit_standby_sts);
		exit_standby_status(0x50,0x4d,0x55);
		main_cpu_get_time(rtc);
		mcu_disable_int();
		hal_mcu_gpio_disable_en();
		PMU_WRITE_BYTE(PRS_KEY_STANDBY_LED, GREEN_LED);//enter norm mode,light  green Led
	#ifdef PMU_MCU_DEBUG
		//printf("pannel exit standby\n");
	#endif
		while(1);//exit standby ,then hold mcu
	}
}

void ir_process(pRTC_TIMER rtc)
{
	INT8 ret = 0;

	if(SUCCESS == get_ir())
	{
		g_exit_standby_sts= EXIT_STANDBY_TYPE_IR;
		PMU_WRITE_BYTE(EXIT_STANDBY_TYPE, g_exit_standby_sts);
		exit_standby_status(0x50,0x4d,0x55);
		main_cpu_get_time(rtc);
		mcu_disable_int();
		hal_mcu_gpio_disable_en();
		PMU_WRITE_BYTE(PRS_KEY_STANDBY_LED,GREEN_LED);//enter norm mode,light  green Led
	#ifdef PMU_MCU_DEBUG
		printf("ir exit standby\n");
	#endif
		while(1);  //exit standby ,then hold mcu
	} 
}

void rtc_process(pRTC_TIMER rtc, pRTC_TIMER wakeup_rtc)
{
	if((rtc->day == wakeup_rtc->day) && (rtc->hour == wakeup_rtc->hour) && \
		(rtc->min == wakeup_rtc->min) && (rtc->month == wakeup_rtc->month) && \
		rtc->sec == wakeup_rtc->sec)
	{ 
		if((rtc->day>0&&rtc->day<32)&&(rtc->month>0&&rtc->month<13))
		{
			g_exit_standby_sts=EXIT_STANDBY_TYPE_RTC;
			PMU_WRITE_BYTE(EXIT_STANDBY_TYPE, g_exit_standby_sts);
			mcu_disable_int();
			exit_standby_status(0x50,0x4d,0x55);
			main_cpu_get_time(rtc);        
			//HAL_MCU_GPIO_DISABLE_EN(); 
			hal_mcu_gpio_disable_en();
			PMU_WRITE_BYTE(PRS_KEY_STANDBY_LED,GREEN_LED);//enter norm mode,light  green Led
		#ifdef PMU_MCU_DEBUG
			printf("rtc exit standby\n");
		#endif
			while(1);//exit standby ,then hold mcu
		}
	}	
}

void power_key_cfg(void)
{
	UINT8 reg = 0;

	/*about 0.2s==>>>0x3e80 (UINT8)(4000*4/256)*/
	PMU_WRITE_BYTE(PRS_KEY_CFG, 0x80);/*about 0.1s==>>>0x1f40 (UINT8)(4000*2/256)*/
	PMU_WRITE_BYTE(PRS_KEY_CFG+1,0x3e);
	PMU_WRITE_BYTE(PRS_KEY_CFG+2,0x00);
	PMU_WRITE_BYTE(PRS_KEY_CFG+3,PMU_READ_BYTE(PRS_KEY_CFG+3)&0xf0);

	/*
	*  KEY_DISABLE_VAL=n us*1.5/(4*(5+1))
	*/
	PMU_WRITE_BYTE(KEY_DISABLE_VAL_LOW0,0x6a);
	PMU_WRITE_BYTE(KEY_DISABLE_VAL_LOW1,0x18); //0.5 second 
	PMU_WRITE_BYTE(KEY_DISABLE_VAL_LOW2,0x00);
	PMU_WRITE_BYTE(KEY_DISABLE_EN,0x80);
	reg = PMU_READ_BYTE(MCU_SYS_IPR);//MCU_Polar
	// reg|=(1<<3)|(1<<6);  /*KEY_INT_STANDBY and KEY_INT_NORM*/
	reg|=(1<<3);
	PMU_WRITE_BYTE(MCU_SYS_IPR, reg);
	reg = PMU_READ_BYTE(MCU_SYS_IER); //MCU_INT_EN
	//reg|=(1<<3)|(1<<6);  /*KEY_INT_STANDBY and KEY_INT_NORM */
	reg|=(1<<3);
	PMU_WRITE_BYTE(MCU_SYS_IER, reg);
	IT0=1;
	EX0=1;
	*(volatile UINT8 xdata *)( PRS_KEY_CFG+3) |=(0x1<<7);
	EA = 1;
}

void power_key_mode(enum KEY_MODE mode)
{
	if(mode == STABDY)
	{
		PMU_WRITE_BYTE(PMU_CFG, PMU_READ_BYTE(PMU_CFG) | 1<<7) ;// standby modde
	}
	else
	{
		PMU_WRITE_BYTE(PMU_CFG, PMU_READ_BYTE(PMU_CFG)&(~(1<<7))) ;// normal mode
	}

	power_key_cfg();
}

/*
*(1) PMU->MAIN CPU,exit standby ,get status
*/
void exit_standby_status(unsigned char status0, unsigned char status1, unsigned char status2)
{
	PMU_WRITE_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS0,status0);	
	PMU_WRITE_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS1,status1);
	PMU_WRITE_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS2,status2);
}

/*
interrupt process
1: time 0 interrupt
2: ir interrupt
*/
void extern_int0() interrupt 0   
{
	UINT8 reg = 0;

	EA=0;
	reg = PMU_READ_BYTE(SYS_REG_ISR);
    
	if(reg&(1<<7)) // time0 3503 do not use it
	{
		PMU_WRITE_BYTE(PMU_TM0_CTRL,PMU_READ_BYTE(PMU_TM0_CTRL)|(1<<3)); //write 1 to clear interrupt
	}
  		  
	if(reg&(1<<IR_BIT)) // ir  interrupt
	{
		IR_ISR();
	}
         
	if(reg&(1<<3))
	{        
		if( PMU_READ_BYTE(PRS_KEY_CFG+2)&(1<<5))
		{
			g_power_key =1;
		}
	}

	EA=1;
}

/*
mailbox interrupt
* MAIN CPU => PMU MCU
*(1)	set ir wakeup power key
*(2)	set timer wake up time 
*(3)	set current time
*/
void extern_int2() interrupt   7             
{
	EA=0;
	PMU_WRITE_BYTE(0xc220,0x01);	/*CPU TO MCU int clear*/
 
	/*get wakeup power key*/
	g_set_ir_key.ir_key_low3   =   PMU_READ_BYTE(MAILBOX_SET_POWERLOW3); 
	g_set_ir_key.ir_key_low2   =   PMU_READ_BYTE(MAILBOX_SET_POWERLOW2);
	g_set_ir_key.ir_key_low1   =   PMU_READ_BYTE(MAILBOX_SET_POWERLOW1);
	g_set_ir_key.ir_key_low0   =   PMU_READ_BYTE(MAILBOX_SET_POWERLOW0);
  
	/*get wakeup time*/
	g_wake_rtc.month = PMU_READ_BYTE(MAILBOX_WAKE_MONTH);
	g_wake_rtc.day  = PMU_READ_BYTE(MAILBOX_WAKE_DAY);
	g_wake_rtc.hour = PMU_READ_BYTE(MAILBOX_WAKE_HOUR);
	g_wake_rtc.min  = PMU_READ_BYTE(MAILBOX_WAKE_MIN);
	g_wake_rtc.sec  = 0;//PMU_READ_BYTE(MAILBOX_WAKE_SECOND);
	
	/*get current time*/
	g_rtc.year_h =  PMU_READ_BYTE(MAILBOX_SET_YEAR_H);
	g_rtc.year_l =  PMU_READ_BYTE(MAILBOX_SET_YEAR_L);
	g_rtc.month  =  PMU_READ_BYTE(MAILBOX_SET_MONTH);
	g_rtc.day    =  PMU_READ_BYTE(MAILBOX_SET_DAY );
	g_rtc.hour   =  PMU_READ_BYTE(MAILBOX_SET_HOUR);
	g_rtc.min    =  PMU_READ_BYTE(MAILBOX_SET_MIN );
	g_rtc.sec    =  PMU_READ_BYTE(MAILBOX_SET_SEC);
	g_rtc.year   = (g_rtc.year_h*100)+g_rtc.year_l;

#ifndef PMU_MCU_DEBUG
	do{
		// enter real standby mode
		PMU_WRITE_BYTE(0xc101, PMU_READ_BYTE(0xc101) | 0x1);
		PMU_WRITE_BYTE(0xc101, PMU_READ_BYTE(0xc101) | 0x1);
	}while(RED_LED != PMU_READ_BYTE(PRS_KEY_STANDBY_LED));
#else 
	printf("enter stdby\n");
#endif
    
	g_standby_flag = ENTER_STANDBY;  

	EA=1;
}

void extern_int3() interrupt   8
{
	EA=0;
	PMU_WRITE_BYTE(0xc220,0x02);	/*CPU TO MCU int clear*/
	EA=1;
}

void extern_int4() interrupt   9
{
	EA=0;
	PMU_WRITE_BYTE(0xc220,0x04);	/*CPU TO MCU int clear*/
	EA=1;
}

void extern_int5() interrupt   10
{
	EA=0;
	EIF=0;
	PMU_WRITE_BYTE(0xc220,0x08);	/*CPU TO MCU int clear*/
	EA=1;
}

void init_externint2(void)
{
	EX2 = 1;
	EA = 1;
}

void init_externint3(void)
{
	EX3 = 1;
	EA = 1;
}

void init_externint4(void)
{
	EX4 = 1;
	EA = 1;
}

void init_externint5(void)
{
	EX5 = 1;
	EA = 1;
}
