/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: rtc.c
 *
 *  Description: Hld video processer and tv encoder driver
 *
 *  History:
 *      Date        		Author         Version   Comment
 *      ====        	======         =======   =======
 *  1.  2012.08.20  xuhua huang     0.1.000   Initial
 ****************************************************************************/

#include <hld_cfg.h>
#include <adr_retcode.h>
#include <hld/adr_hld_dev.h>
#include <hld/rtc/adr_rtc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <linux/rtc.h>

#include <ali_pmu_common.h>
#include <ali_pm_common.h>

#include <sys/ioctl.h>


#if 0
#define RTC_PRINTF(fmt, args..._)	ADR_DBG_PRINT(RTC, fmt, ##arg)
#else
#define RTC_PRINTF(...)             do{}while(0)
#endif
enum
{
	RTC_INVALID,
	RTC_RAW,
	RTC_PMU,
};
static int rtc_type=RTC_INVALID;
enum
{
	STDBY_INVALID,
	STDBY_PM,
	STDBY_PMU,
};
static int stdby_type=STDBY_INVALID;

static inline void hld_to_rtc_pmu(struct rtc_time_pmu *time, struct rtc_time_hld *time_hld)
{
	time->sec = time_hld->second;
	time->min = time_hld->minute;
	time->hour = time_hld->hour;
	time->date = time_hld->date;
	time->month = time_hld->month;
	time->year = time_hld->year;
}

static inline void rtc_to_hld_pmu(struct rtc_time_hld *time_hld, struct rtc_time_pmu *time)
{
	time_hld->second = time->sec;
	time_hld->minute = time->min;
	time_hld->hour = time->hour;
	time_hld->date = time->date;
	time_hld->month = time->month;
	time_hld->year = time->year;
}

static inline void hld_to_rtc(struct rtc_time *time, struct rtc_time_hld *time_hld)
{
	time->tm_sec = time_hld->second;
	time->tm_min = time_hld->minute;
	time->tm_hour = time_hld->hour;
	time->tm_mday = time_hld->date;
	time->tm_mon = time_hld->month;
	time->tm_year = time_hld->year;
}

static inline void rtc_to_hld(struct rtc_time_hld *time_hld, struct rtc_time *time)
{
	time_hld->second = time->tm_sec;
	time_hld->minute = time->tm_min;
	time_hld->hour = time->tm_hour;
	time_hld->date = time->tm_mday;
	time_hld->month = time->tm_mon;
	time_hld->year = time->tm_year;
}

RET_CODE rtc_attach(void)
{
	struct rtc_device*dev ;
	dev=dev_alloc("rtc_S3701", HLD_DEV_TYPE_RTC, sizeof(struct rtc_device));
	if(dev==NULL)
	{
		RTC_PRINTF("Alloc rtc device error!\n");
		return !RET_SUCCESS;
	}

       /* Add this device to queue */
	if(dev_register(dev)!=SUCCESS)
	{
		dev_free(dev);
		RTC_PRINTF("Register rtc device failed!!!!!!!\n");
		return !RET_SUCCESS;
	}
	dev->flags = RTC_STATE_ATTACH;
	dev->handle = 0;
	return RET_SUCCESS;
}

RET_CODE rtc_dettach(struct rtc_device *dev)
{
	if(RTC_STATE_ATTACH!=dev->flags)
	{
		RTC_PRINTF("%s error. Device status error.\n",__FUNCTION__);
		return !RET_SUCCESS;
	}
	dev_free(dev);
	return RET_SUCCESS;
}

RET_CODE rtc_open(struct rtc_device * dev)
{
	unsigned int fd_cfg;
	int rtc_hdl=0;
	if(NULL==dev)return !RET_SUCCESS;
	RTC_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
	if(stdby_type == STDBY_PMU)
	{
		struct pmu_device *pd;
		pd = (struct pmu_device *)dev_get_by_id(HLD_DEV_TYPE_PMU, 0);
		if(pd)
		{
			rtc_type = RTC_PMU;
			rtc_hdl = pd->handle;
			RTC_PRINTF("share with pmu success\n");
		}
		else
		{
			RTC_PRINTF("process error, pls check\n");
			return !RET_SUCCESS;
		}
	}
	else if((rtc_hdl = open("/dev/ali_pmu", O_RDWR|O_NONBLOCK| O_CLOEXEC))>0)
	{		
		rtc_type = RTC_PMU;
		RTC_PRINTF("open pmu success\n");
	}
	else if((rtc_hdl = open("/dev/rtc0", O_RDWR|O_NONBLOCK| O_CLOEXEC))>0)
	{		
		rtc_type = RTC_RAW;
		RTC_PRINTF("open rtc0 success\n");
	}
	else
	{
		RTC_PRINTF("open rtc failed\n");
		return !RET_SUCCESS;
	}
	dev->handle = rtc_hdl;
	return RET_SUCCESS;
}

RET_CODE rtc_close(struct rtc_device * dev)
{
	int ret;
	if(dev->handle && stdby_type != STDBY_PMU)
	{
		ret=close(dev->handle);
		if(ret<0)
			return !RET_SUCCESS;
	}
	dev->handle = 0;
	rtc_type = RTC_INVALID;
	return RET_SUCCESS;
}

RET_CODE rtc_io_control(struct rtc_device * dev, UINT32 dwCmd, UINT32 dwParam)
{
	RET_CODE ret = !RET_SUCCESS;
	UINT32 licmd;
	
	if(!dev->handle)
	{
		RTC_PRINTF("%s handle not open error!\n", __FUNCTION__);
		return !RET_SUCCESS;
	}
	switch(dwCmd)
	{
		case RTC_CMD_SET_TIME:
		{
			struct rtc_time_hld *time=(struct rtc_time_hld *)dwParam;
			struct rtc_time_pmu ptime;
			struct rtc_time wtime;
			void *param;
			if(time)
			{
				if(rtc_type == RTC_PMU)
				{
					licmd = ALI_PMU_RTC_SET_VAL;
					hld_to_rtc_pmu(&ptime, time);
					param = &ptime;
				}
				else
				{
					licmd = RTC_SET_TIME;
					hld_to_rtc(&wtime, time);
					param = &wtime;
				}
				if(ioctl(dev->handle, licmd, (UINT32)param)==0)
					ret = RET_SUCCESS;
			}
			break;
		}
		case RTC_CMD_GET_TIME:
		{
			struct rtc_time_hld *time=(struct rtc_time_hld *)dwParam;
			struct rtc_time_pmu ptime;
			struct rtc_time wtime;
			void *param;
			if(time)
			{
				if(rtc_type == RTC_PMU)
				{
					licmd = ALI_PMU_RTC_RD_VAL;
					param = &ptime;
				}
				else
				{
					licmd = RTC_RD_TIME;
					param = &wtime;
				}
				if(ioctl(dev->handle, licmd, (UINT32)param)==0)
				{
					if(rtc_type == RTC_PMU)
						rtc_to_hld_pmu(time, &ptime);
					else
						rtc_to_hld(time, &wtime);
					ret = RET_SUCCESS;
				}
			}
			break;
		}
		case RTC_CMD_SET_ALARM:
		{
			struct rtc_wkalrm_hld *alarm=(struct rtc_wkalrm_hld *)dwParam;
			struct rtc_wkalrm walarm;
			if(rtc_type == RTC_PMU)
			{
				ret = !RET_SUCCESS;
				break;
			}
			if(NULL != time)
			{
				hld_to_rtc(&walarm.time, &alarm->time);
				walarm.enabled=alarm->enabled;
				licmd = RTC_WKALM_SET;
				if(ioctl(dev->handle, licmd, (UINT32)&walarm)==0)
					ret = RET_SUCCESS;
			}
			break;
		}
			break;
		case RTC_CMD_GET_ALARM:
		{
			struct rtc_wkalrm_hld *alarm=(struct rtc_wkalrm_hld *)dwParam;
			struct rtc_wkalrm walarm;
			if(rtc_type == RTC_PMU)
			{
				ret = !RET_SUCCESS;
				break;
			}
			if(alarm)
			{
				licmd = RTC_WKALM_RD;
				if(ioctl(dev->handle, licmd, (UINT32)&walarm)==0)
				{
					rtc_to_hld(&alarm->time, &walarm.time);
					alarm->enabled = walarm.enabled;
					ret = RET_SUCCESS;
				}
			}
			break;
		}
		case RTC_CMD_AIE_OFF:
			if(rtc_type == RTC_PMU)
			{
				ret = !RET_SUCCESS;
				break;
			}
			licmd = RTC_AIE_OFF;
			if(ioctl(dev->handle, licmd, dwParam)==0)
				ret = RET_SUCCESS;
			break;
		case RTC_CMD_AIE_ON:
			if(rtc_type == RTC_PMU)
			{
				ret = !RET_SUCCESS;
				break;
			}
			licmd = RTC_AIE_ON;
			if(ioctl(dev->handle, licmd, dwParam)==0)
				ret = RET_SUCCESS;
			break;
		default:
			RTC_PRINTF("%s cmd not supported error!\n", __FUNCTION__);
			return !RET_SUCCESS;
	}
	if(ret != RET_SUCCESS)
		RTC_PRINTF("%s cmd %d ioctl error!\n", __FUNCTION__, cmd);
	return ret;
}

RET_CODE pmu_attach(void)
{
	struct pmu_device*dev ;
	dev=dev_alloc("pmu_S3701", HLD_DEV_TYPE_PMU, sizeof(struct pmu_device));
	if(dev==NULL)
	{
		RTC_PRINTF("Alloc pmu device error!\n");
		return !RET_SUCCESS;
	}

       /* Add this device to queue */
	if(dev_register(dev)!=SUCCESS)
	{
		dev_free(dev);
		RTC_PRINTF("Register pmu device failed!!!!!!!\n");
		return !RET_SUCCESS;
	}
	dev->flags = RTC_STATE_ATTACH;
	dev->handle = 0;
	return RET_SUCCESS;
}

RET_CODE pmu_dettach(struct pmu_device *dev)
{
	if(RTC_STATE_ATTACH!=dev->flags)
	{
		RTC_PRINTF("%s error. Device status error.\n",__FUNCTION__);
		return !RET_SUCCESS;
	}
	dev_free(dev);
	return RET_SUCCESS;
}

RET_CODE pmu_open(struct pmu_device * dev)
{
	unsigned int fd_cfg;
	int pmu_hdl=0;
	if(NULL==dev)return !RET_SUCCESS;
	RTC_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
	if(rtc_type == RTC_PMU)
	{
		struct rtc_device *rd;
		rd = (struct rtc_device *)dev_get_by_id(HLD_DEV_TYPE_RTC, 0);
		if(rd)
			pmu_hdl = rd->handle;
		else
		{
			RTC_PRINTF("process error, pls check\n");
			return !RET_SUCCESS;
		}
		stdby_type = STDBY_PMU;
	}
	else if((pmu_hdl = open("/dev/ali_pmu", O_RDWR|O_NONBLOCK| O_CLOEXEC))>0)
	{		
		stdby_type = STDBY_PMU;
		RTC_PRINTF("open pmu success!\n");
	}
	else if((pmu_hdl = open("/dev/ali_pm", O_RDWR|O_NONBLOCK| O_CLOEXEC))>0)
	{		
		stdby_type = STDBY_PM;
		RTC_PRINTF("open pm success!\n");
	}
	else
	{
		RTC_PRINTF("open stdby failed\n");
		return !RET_SUCCESS;
	}
	dev->handle = pmu_hdl;
	dev->duration = 0;
	return RET_SUCCESS;
}

RET_CODE pmu_close(struct pmu_device * dev)
{
	int ret;
	if(dev->handle && rtc_type != RTC_PMU)
	{
		ret=close(dev->handle);
		if(ret<0)
			return !RET_SUCCESS;
	}
	dev->handle = 0;
	stdby_type = STDBY_INVALID;
	return RET_SUCCESS;
}

void _add_second(struct rtc_time_hld *ali_time, UINT32 sec)
{
	while(sec--)
		{
			ali_time->second++;
	        if(ali_time->second> 59){
                ali_time->minute += 1;
                ali_time->second = 0;
            }

            if(ali_time->minute > 59){
                ali_time->hour += 1;
                ali_time->minute = 0;
            }

            if(ali_time->hour > 23){
                ali_time->date += 1;
                ali_time->hour = 0;
            }

            if(ali_time->month == 1 || ali_time->month == 3 || ali_time->month == 5 || ali_time->month == 7 
							|| ali_time->month == 8 || ali_time->month == 10 || ali_time->month == 12){
                if(ali_time->date > 31){
                    ali_time->month += 1;
                    ali_time->date = 1;
                }
            }else{
                if(ali_time->month == 2){ //month 2
                    if(((ali_time->year%4==0)&&(ali_time->year%100!=0)) || ali_time->year%400){ //leap year
                       if(ali_time->date > 29){
                           ali_time->month += 1;
                           ali_time->date = 1;
                        }        
                    }else{//not a leap year
                     if(ali_time->date > 28){
                           ali_time->month += 1;
                           ali_time->date = 1;
                        }
                    }
                }else{
                    if(ali_time->date > 30){
                        ali_time->month += 1;
                        ali_time->date = 1;
                    }
                }
            }
            if(ali_time->month > 12){ // a new year begins         
                ali_time->year += 1; 
                ali_time->month = 1;
            }
		}
}

RET_CODE pmu_io_control(struct pmu_device * dev, UINT32 dwCmd, UINT32 dwParam)
{
	RET_CODE ret = !RET_SUCCESS;
	if(!dev->handle)
	{
		RTC_PRINTF("%s handle not open error!\n", __FUNCTION__);
		return !RET_SUCCESS;
	}
	switch(dwCmd)
	{
		case PMU_CMD_ENTER_STANDBY:
			if(stdby_type == STDBY_PMU)
			{
				if(dev->duration)
				{
					struct min_alarm_num min_alarm;
					memset(&min_alarm, 0, sizeof(min_alarm));
					_add_second(&dev->time, dev->duration);
					min_alarm.min_alm.month = dev->time.month;
					min_alarm.min_alm.date = dev->time.date;
					min_alarm.min_alm.hour = dev->time.hour;
					min_alarm.min_alm.min = dev->time.minute;
					if(ioctl(dev->handle, ALI_PMU_MCU_WAKEUP_TIME, (UINT32)&min_alarm)!=0)
					{
						RTC_PRINTF("ALI_PMU_MCU_WAKEUP_TIME error!\n");
						break;
					}
					ret = RET_SUCCESS;
				}
									
				if(ioctl(dev->handle, ALI_PMU_MCU_ENTER_STANDBY, dwParam)!=0)
				{
					RTC_PRINTF("ALI_PMU_MCU_ENTER_STANDBY error!\n");
					break;
				}
			}
			else
			{
				pm_param_t par;
				par.timeout = dev->duration;
				par.reboot = 0;
				if(ioctl(dev->handle, PM_CMD_SET_STANDBY_PARAM, &par)!=0)
				{
					RTC_PRINTF("PM_CMD_SET_STANDBY_PARAM error!\n");
					break;
				}
				system("echo mem > /sys/power/state");
			}
			ret = RET_SUCCESS;
			break;
		case PMU_CMD_SET_TIME:
		{
			struct rtc_time_hld *time_hld=(struct rtc_time_hld *)dwParam;
			
			if(time_hld)
				dev->time = *time_hld;
			else
				break;
			
			if(stdby_type == STDBY_PMU)
			{
				struct rtc_time_pmu curtime;
				
				curtime.year = time_hld->year;
				curtime.month = time_hld->month;
				curtime.date = time_hld->date;
				curtime.hour = time_hld->hour;
				curtime.min = time_hld->minute;
				curtime.sec = time_hld->second;
				if(ioctl(dev->handle, ALI_PMU_MCU_SET_TIME, (UINT32)&curtime)!=0)
				{
					RTC_PRINTF("ALI_PMU_MCU_SET_TIME error!\n");
					break;
				}
			}
			else
			{
				//to do:set time
			}
			ret = RET_SUCCESS;
			break;
		}
		case PMU_CMD_SET_WAKETIME:
			if((INT32)dwParam<=0)
				dev->duration = 0;
			else if(dwParam<60)
				dev->duration = 60;
			else
				dev->duration = dwParam;
			ret = RET_SUCCESS;
			break;
		case PMU_CMD_SET_POWER_KEY:
			if(!dwParam)
			{
				RTC_PRINTF("PMU_CMD_SET_POWER_KEY param input error!\n");
				break;
			}
			if(stdby_type == STDBY_PMU)
			{	
				UINT32 pmu_power_key[POWER_KEY_NUM];
				UINT32 *input_key=(UINT32 *)dwParam;
				memcpy(pmu_power_key, input_key, POWER_KEY_NUM*sizeof(UINT32));	
				if(ioctl(dev->handle, ALI_PMU_IR_PROTOL_NEC, pmu_power_key)!=0)
				{
					RTC_PRINTF("ALI_PMU_IR_PROTOL_NEC error!\n");
					break;
				}
			}
			else
			{
				pm_key_t key;
				UINT32 *input_key=(UINT32 *)dwParam;
				memcpy(key.ir_power, input_key, POWER_KEY_NUM*sizeof(unsigned long));
				key.standby_key = 0;
				if(ioctl(dev->handle, PM_CMD_SET_RESUME_KEY, &key)!=0)
				{
					RTC_PRINTF("PM_CMD_SET_RESUME_KEY error!\n");
					break;
				}			
			}
			ret = RET_SUCCESS;
			break;
		default:
			RTC_PRINTF("%s cmd not supported error!\n", __FUNCTION__);
			return !RET_SUCCESS;
	}
	if(ret != RET_SUCCESS)
		RTC_PRINTF("%s cmd %d ioctl error!\n", __FUNCTION__, cmd);
	return ret;
}

