#ifndef _ALI_PMU_COMMON_H_
#define _ALI_PMU_COMMON_H_

/*! @addtogroup  Devicedriver
 *  @{
    */

 
/*! @addtogroup ALiPMU
 *  @{
    */
    
#define ALI_PMU_IO_COMMAND_BASE		                  (0x60<<8)//!<PMU io-ctrl call base addr.
#define ALI_PMU_EN                                                      ALI_PMU_IO_COMMAND_BASE//!<PMU enable io-ctrl call.
#define ALI_PMU_IR_PROTOL_NEC			                  (ALI_PMU_IO_COMMAND_BASE+1)//!<PMU NEC IR standby io-ctrl call.
#define ALI_PMU_IR_PROTOL_RC5_X                            (ALI_PMU_IO_COMMAND_BASE+2)//!<PMU RC5 IR standby io-ctrl call.
#define ALI_PMU_RTC_SET_VAL                                    (ALI_PMU_IO_COMMAND_BASE+10)//!<PMU set RTC value io-ctrl call.
#define ALI_PMU_RTC_SET_MIN_ALARM                        (ALI_PMU_IO_COMMAND_BASE+11)//!<PMU set RTC wakeup time io-ctrl call(the smallest unit is minutes.)
#define ALI_PMU_RTC_SET_MS_ALARM                         (ALI_PMU_IO_COMMAND_BASE+12)//!<PMU set RTC wakeup time io-ctrl call(the smallest unit is ms.)
#define ALI_PMU_RTC_EN_ALARM                                 (ALI_PMU_IO_COMMAND_BASE+13)//!<PMU enable RTC io-ctrl call.
#define ALI_PMU_RTC_RD_VAL                                     (ALI_PMU_IO_COMMAND_BASE+14)//!<PMU read RTC value io-ctrl call.(the smallest unit is minutes.)
#define ALI_PMU_RTC_RD_MS_VAL                               (ALI_PMU_IO_COMMAND_BASE+15)//!<PMU read RTC value io-ctrl call.(the smallest unit is ms.)
#define ALI_PMU_EXIT_STANDBY_STATUS                    (ALI_PMU_IO_COMMAND_BASE+16)//!<PMU exit standby io-ctrl call.
#define ALI_PMU_MCU_RST                                          (ALI_PMU_IO_COMMAND_BASE+17)//!<PMU MCU reset io-ctrl call.
#define ALI_PMU_MCU_ENTER_STANDBY                      (ALI_PMU_IO_COMMAND_BASE+18)//!<PMU MCU enter standby io-ctrl call.
#define ALI_PMU_MCU_SET_TIME                                 (ALI_PMU_IO_COMMAND_BASE+19)//!<PMU set MCU time io-ctrl call.
#define ALI_PMU_MCU_READ_TIME                               (ALI_PMU_IO_COMMAND_BASE+20)//!<PMU read MCU time io-ctrl call.
#define ALI_PMU_MCU_WAKEUP_TIME                          ALI_PMU_RTC_SET_MIN_ALARM//!<PMU set MCU wakeup time io-ctrl call.
#define ALI_PMU_PANEL_I2C_EN                                  (ALI_PMU_IO_COMMAND_BASE+22)//!<Do not use it now.
#define ALI_PMU_SHOW_TIME_EN                                (ALI_PMU_IO_COMMAND_BASE+23)//!<PMU show time enable io-ctrl call.
#define ALI_PMU_RTC_TIMER3_SET_VAL                      (ALI_PMU_IO_COMMAND_BASE+24)//!<PMU set timer3 value io-ctrl call.
#define ALI_PMU_RTC_TIMER3_GET_VAL                      (ALI_PMU_IO_COMMAND_BASE+25)//!<PMU get timer3 value io-ctrl call.
#define ALI_PMU_GET_SET_LOGADDR                           (ALI_PMU_IO_COMMAND_BASE+26)//!<Do not use it now.
#define ALI_PMU_RESET_MAILBOX                               (ALI_PMU_IO_COMMAND_BASE+27)//!<PMU reset mailbox io-ctrl call----not use it now.
#define ALI_PMU_PANEL_POWKEY_EN                          (ALI_PMU_IO_COMMAND_BASE+22)//!<PMU enable panel power key io-ctrl call.
#define ALI_PMU_REPORT_EXIT_TYPE                          (ALI_PMU_IO_COMMAND_BASE+28)////!<PMU report exit standby type io-ctrl call
#define ALI_PMU_IRX_POWER_KEY	                        0x01//!<PMU IR power key value.
#define ALI_PMU_CEC_POWER_KEY	                        0x02//!<PMU CEC power key value.

/*! @struct rtc_time_pmu
@brief It indicates PMU RTC time.
*/
struct rtc_time_pmu
{
	unsigned int year;//!<PMU RTC time----year
	unsigned char month;//!<PMU RTC time----month
	unsigned char date;//!<PMU RTC time----date
	unsigned char day;//!<PMU RTC time----day
	unsigned char hour;//!<PMU RTC time----hour
	unsigned char min;//!<PMU RTC time----minute
	unsigned char sec;//!<PMU RTC time----second
};

/*! @struct min_alarm
@brief PMU MCU wakeup time(the smallest unit is minutes).
*/
struct min_alarm
{
	unsigned char en_month;//!<PMU MCU wakeup time----month(only for C3701)
	unsigned char en_date;//!<PMU MCU wakeup time----date(only for C3701)
	unsigned char en_sun;//!<PMU MCU wakeup time----sun(only for C3701)
	unsigned char en_mon;//!<PMU MCU wakeup time----mon(only for C3701)
	unsigned char en_tue;//!<PMU MCU wakeup time----tue(only for C3701)
	unsigned char en_wed;//!<PMU MCU wakeup time----wed(only for C3701)
	unsigned char en_thr;//!<PMU MCU wakeup time----thr(only for C3701)
	unsigned char en_fri;//!<PMU MCU wakeup time----fri(only for C3701)
	unsigned char en_sat;//!<PMU MCU wakeup time----sat(only for C3701)

	unsigned char month;//!<PMU MCU wakeup time----month
	unsigned char date;//!<PMU MCU wakeup time----date
	unsigned char hour;//!<PMU MCU wakeup time----hour
	unsigned char min;//!<PMU MCU wakeup time----min
};

/*! @struct ms_alarm
@brief PMU MCU wakeup time(the smallest unit is ms.)
*/
struct ms_alarm
{
	unsigned char en_hour;//!<PMU MCU wakeup time----hour(only for C3701)
	unsigned char en_min;//!<PMU MCU wakeup time----min(only for C3701)
	unsigned char en_sec;//!<PMU MCU wakeup time----sec(only for C3701)
	unsigned char en_ms;//!<PMU MCU wakeup time----ms(only for C3701)

	unsigned char hour;//!<PMU MCU wakeup time----hour
	unsigned char min;//!<PMU MCU wakeup time----min
	unsigned char sec;//!<PMU MCU wakeup time----sec
	unsigned char ms;//!<PMU MCU wakeup time----m
};

/*! @struct min_alarm_num
@brief It defines multiple min alarm. 
*/
struct min_alarm_num
{
	struct min_alarm min_alm;//!<Define a min alarm.
	unsigned char num;//!<RTC alarm number,from 0~7
};

/*! @struct ms_alarm_num
@brief It defines multiple ms alarm.
*/
struct ms_alarm_num
{
	struct ms_alarm ms_alm;//!<Define a ms alarm.
	unsigned char num;//!<RTC alarm number,from 8~9
};

/*! @struct rtc_base_time
@brief It definese system time.
*/
struct rtc_base_time
{
	unsigned char rtc_sec;//!<RTC time----sec
	unsigned char rtc_min;//!<RTC time----min
	unsigned char rtc_hour;//!<RTC time----hour
	unsigned char rtc_date;//!<RTC time----date
	unsigned char rtc_month;//!<RTC time----month
	unsigned int rtc_year;//!<RTC time----yaer
	unsigned char g_year_h;//!<Global time----year(high byte)
	unsigned char g_year_l;//!<Global time----year(low byte)
	unsigned int g_year;//!<Global time----year
	unsigned char g_month;//!<Global time----month
	unsigned char g_day;//!<Global time----day
	unsigned char g_hour;//!<Global time----hour
	unsigned char g_min;//!<Global time----min
	unsigned char g_sec;//!<global time----sec
};

/*! @enum MCU_SHOW_PANNEL
@brief It defines MCU show panel type.
*/
enum MCU_SHOW_PANNEL
{
	MCU_SHOW_NOTHING = 0,//!<Show nothing
	MCU_SHOW_OFF,//!<Sshow "OFF"
	MCU_SHOW_TIME//!<Show time
};

/*! @enum PMU_EXIT_FLAG
@brief It defines pmu standby exit type.
*/
typedef enum
{
	E_PMU_KEY_EXIT=0,//!<Panel key wakeup
	E_PMU_IR_EXIT,//!<IR key wakeup
	E_PMU_RTC_EXIT,//!<RTC wakeup
	E_PMU_COLD_BOOT//!<COLD_BOOT
}PMU_EXIT_FLAG;

/*!
@}
*/

/*!
@}
*/

#endif /*! _ALI_PMU_H_*/
