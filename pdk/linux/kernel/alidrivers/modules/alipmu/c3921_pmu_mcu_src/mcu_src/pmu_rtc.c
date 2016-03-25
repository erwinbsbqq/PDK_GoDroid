#include <dp8051xp.h>
#include <stdio.h>
#include "sys.h"
#include "pmu_rtc.h"

//=====================================================================================//
RTC_TIMER g_rtc;
RTC_TIMER g_wake_rtc;
static void pmu_timer1_int_en(UINT8 en);
static UINT8 is_leap_year(pRTC_TIMER rtc);

//=====================================================================================//
void pmu_timer1_set_time(unsigned char autoload)
{
	UINT8 div = 0x1;
	UINT8 reg = 0;
	UINT8 reg1 = 0;

	if(autoload) 
	{ 
		PMU_WRITE_BYTE(PMU_RTC1_CNT_LOW0,0x0c);//init value=0xffffffff-(x sec*1000*1000*27/64)
		PMU_WRITE_BYTE(PMU_RTC1_CNT_LOW1,0x90);// 1 second
		PMU_WRITE_BYTE(PMU_RTC1_CNT_LOW2,0xf9);
		PMU_WRITE_BYTE(PMU_RTC1_CNT_LOW3,0xff);

		PMU_WRITE_BYTE(PMU_RTC1_AUTO_LOAD,0x0c);//only by byte operate for pmu
		PMU_WRITE_BYTE(PMU_RTC1_AUTO_LOAD+1,0x90);
		PMU_WRITE_BYTE(PMU_RTC1_AUTO_LOAD+2,0xf9);
		PMU_WRITE_BYTE(PMU_RTC1_AUTO_LOAD+3,0xff);    
		PMU_WRITE_BYTE(PMU_RTC1_CTRL,PMU_READ_BYTE(PMU_RTC1_CTRL)|(1<<7));  //AUTO LOAD EN  
	}

	EX1 = 1;
	EA = 1;
	reg = PMU_READ_BYTE(MCU_SYS_IPR1);  
	reg |= (1<<0);
	PMU_WRITE_BYTE(MCU_SYS_IPR1, reg);//MCU  porlarity
	reg = PMU_READ_BYTE(MCU_SYS_IER1);//MCU IE 
	reg |= (1<<0);
	PMU_WRITE_BYTE(MCU_SYS_IER1, reg);

	if(autoload)
	{
		PMU_WRITE_BYTE(PMU_RTC1_CTRL, 0x14 | div|(1<<7));// //AUTO LOAD EN  
	}
	else
	{
		PMU_WRITE_BYTE(PMU_RTC1_CTRL, 0x14 | div);
	}

	pmu_timer1_int_en(1);
 }

static void pmu_timer1_int_en(UINT8 en)
{
	if(en)
	{
		PMU_WRITE_BYTE(PMU_RTC1_CTRL,PMU_READ_BYTE(PMU_RTC1_CTRL)|(0x1<<4));
	}
	else
	{
		PMU_WRITE_BYTE(PMU_RTC1_CTRL,PMU_READ_BYTE(PMU_RTC1_CTRL)&(~(0x1<<4)));
	}
}

void update_current_time(pRTC_TIMER rtc)
{
	if(rtc->sec > 59)
	{
		rtc->sec = 0;
		rtc->min++;
		if(rtc->min > 59)
		{
			rtc->min = 0;
			rtc->hour++;
			if(rtc->hour>23)
			{
				rtc->hour = 0;
				rtc->day++;
			
				if((rtc->month == 1)||(rtc->month == 3)||(rtc->month == 5)\
					||(rtc->month == 7)||(rtc->month == 8)||(rtc->month == 10)||(rtc->month == 12))
				{
					if(rtc->day > 31)
					{
						rtc->day = 1;
						rtc->month++;
						if(rtc->month > 12)
						{
							rtc->month = 1;
							rtc->year++;
						}
					}
				}
				
				if(rtc->month == 2)
				{
					if((rtc->day > 29) && (is_leap_year(rtc)))
					{
						rtc->day=1;
						rtc->month ++;
					}
					else if((rtc->day > 28) && (is_leap_year(rtc)==0))
					{
						rtc->day = 1;
                				rtc->month++;
					}
				}
				
				if((rtc->month == 4)||(rtc->month == 6)||(rtc->month == 9)||(rtc->month == 11))
				{
					if(rtc->day>30)
					{
						rtc->day = 1;
						rtc->month++;
					}
				}
			
			}
		}
	}
}

void main_cpu_get_time(pRTC_TIMER rtc)
{
	rtc->year_h = (rtc->year)/100;
	rtc->year_l = (rtc->year)%100;

	PMU_WRITE_BYTE(PMUSRAM_GET_YEAR_H, rtc->year_h);
	PMU_WRITE_BYTE(PMUSRAM_GET_YEAR_L, rtc->year_l);
	PMU_WRITE_BYTE(PMUSRAM_GET_MONTH, rtc->month);
	PMU_WRITE_BYTE(PMUSRAM_GET_DAY, rtc->day);
	PMU_WRITE_BYTE(PMUSRAM_GET_HOUR, rtc->hour);
	PMU_WRITE_BYTE(PMUSRAM_GET_MIN, rtc->min);
	PMU_WRITE_BYTE(PMUSRAM_GET_SEC, rtc->sec);
}

static UINT8 is_leap_year(pRTC_TIMER rtc)
{
	if((rtc->year%4 == 0)&&(rtc->year%100 != 0))
	{
		return 1;
	}
	else if(rtc->year%400 == 0)
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}

void extern_int1() interrupt 2
{
	UINT8  reg =0;
	UINT8  div=0x01;

	EA=0;
	reg = PMU_READ_BYTE(MCU_SYS_INT_STS);
	PMU_WRITE_BYTE(PMU_RTC1_CTRL,PMU_READ_BYTE(PMU_RTC1_CTRL)|div|(1<<3));

	if(reg&0x1)
	{
		g_rtc.sec++;
		update_current_time(&g_rtc);
	}
	EA=1;
}