#include <dp8051xp.h>
#include <stdio.h>
#include "sys.h"
#include "ir.h"
#include "pannel.h"
#include "uart.h"
#include "pmu_rtc.h"
#include <intrins.h>

//============================================================//
extern void exit_standby_status(unsigned char status0, unsigned char status1, unsigned char status2);
extern void mcu_disable_int(void);
unsigned char power_key_press_last = 0xa5;
unsigned char power_key_press_first = 0xa5;
unsigned char power_key_press_count =0;

//============================================================//
void power_key_process(pRTC_TIMER rtc)
{
	unsigned char gpio_in_status_reg;
	unsigned char power_key_press;

	WRITE_BYTE(HAL_GPIO_EN,  READ_BYTE(HAL_GPIO_EN) |(1<<6));
	WRITE_BYTE(HAL_GPIO_DIR_REG,  READ_BYTE(HAL_GPIO_DIR_REG) &(~(1<<6)));

	gpio_in_status_reg = READ_BYTE(HAL_GPIO_DI_REG);	// use PMU_CEC pin as power key

	power_key_press = (gpio_in_status_reg>>6) & 0x01;	// xpmu_gpio[6]

	if(power_key_press != power_key_press_last)
	{
		//printf("1");
		power_key_press_last = power_key_press;
		power_key_press_count = 0;
	}
	else
	{
		//printf("0");
		power_key_press_count ++;							
	}
	if(power_key_press_count > 20)
	{
		if(power_key_press_first == 0xa5)
		{
			power_key_press_first = power_key_press_last;
		}
		else if(power_key_press_first != power_key_press)
		{
			PMU_WRITE_BYTE(PRS_KEY_CFG+2,(PMU_READ_BYTE(PRS_KEY_CFG+2)|((1<<5)))); //clear interrupt
			main_cpu_get_time(rtc);
			mcu_disable_int();
			//HAL_MCU_GPIO_DISABLE_EN();
			hal_mcu_gpio_disable_en();
			exit_standby_status(0x50,0x4d,0x55);
			PMU_WRITE_BYTE(PRS_KEY_STANDBY_LED,GREEN_LED);//enter norm mode,light  green 
			while(1); //exit standby ,then hold mcu
		}
	}
	
}

void main(void)
{
	enum SHOW_TYPE show_type;
	//init for normal mode
	pmu_init();   

	// wait mailbox interrupt come
	while(g_standby_flag != ENTER_STANDBY);

	g_standby_flag = NORMAL_STATUS;
	standby_init();  

	//get show type from cpu
	show_type=(enum SHOW_TYPE)(PMU_READ_BYTE(SHOW_TYPE_PAR));

	while(1)
	{
		//it will loop,until ir,pannel_key or time comeing          
		power_key_process(&g_rtc);//In without panel case, use a GPIO as power key.
		pannel_process(&g_rtc, show_type);  
		rtc_process(&g_rtc,&g_wake_rtc);
		ir_process(&g_rtc);
	}
}
