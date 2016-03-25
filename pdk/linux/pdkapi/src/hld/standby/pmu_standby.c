
#include <fcntl.h>
#include <sys/ioctl.h>
// #include <hld/pmu/adr_pmu.h>  // mask it for redefinition of 'struct rtc_time_pmu',  adr_pmu.h for app call lib_hld.o, pmu_standby.c do not need it
#include <hld_cfg.h>

#include <ali_pmu_common.h>

#define ALI_PMU_PRINTF(fmt, args...) ADR_DBG_PRINT(PMU, fmt, ##args)

struct rtc_time_pmu base_time={0,0,0,0,0,0,0};
struct min_alarm_num ali_pmu_alarm ={0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned long ali_pmu_ir_power_key[8]={0x60df708f, 0x10efeb14,0xff50af,0x00ff807f,0,0,0,0};
unsigned long ali_pmu_panel_power_key = 0;
	
void set_ali_pmu_standby_ir_power_key(unsigned long * ir_power_key)
{
	int pmu_handle;
	
	pmu_handle = open("/dev/ali_pmu", O_RDWR|O_NONBLOCK| O_CLOEXEC);
	if (pmu_handle < 0)
	{
		ALI_PMU_PRINTF("open /dev/ali_pmu fail\n");
		return;
	}
	
	ali_pmu_ir_power_key[0] = ir_power_key[0];
	ali_pmu_ir_power_key[1] = ir_power_key[1];
	ali_pmu_ir_power_key[2] = ir_power_key[2];
	ali_pmu_ir_power_key[3] = ir_power_key[3];
	ali_pmu_ir_power_key[4] = ir_power_key[4];
	ali_pmu_ir_power_key[5] = ir_power_key[5];
	ali_pmu_ir_power_key[6] = ir_power_key[6];
	ali_pmu_ir_power_key[7] = ir_power_key[7];
	
	ALI_PMU_PRINTF("set_ali_pmu_standby_ir_power_key:0x%lx ,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx\n", \
			ali_pmu_ir_power_key[0],ali_pmu_ir_power_key[1],ali_pmu_ir_power_key[2],ali_pmu_ir_power_key[3],ali_pmu_ir_power_key[4],ali_pmu_ir_power_key[5],ali_pmu_ir_power_key[6],ali_pmu_ir_power_key[7]);
	
	ioctl(pmu_handle, ALI_PMU_IR_PROTOL_NEC, ali_pmu_ir_power_key);
	
	close(pmu_handle);
	
	return;
}

 
 void set_ali_pmu_standby_panel_power_key(unsigned long panel_power_key)
{
	int pmu_handle;
	
	pmu_handle = open("/dev/ali_pmu", O_RDWR|O_NONBLOCK| O_CLOEXEC);
	if (pmu_handle < 0)
	{
		ALI_PMU_PRINTF("open /dev/ali_pmu fail\n");
		return;
	}
	
	ali_pmu_panel_power_key = panel_power_key;
	
	ALI_PMU_PRINTF("set_ali_pmu_standby_panel_power_key:0x%lx \n", ali_pmu_panel_power_key);
	ioctl(pmu_handle, ALI_PMU_PANEL_POWKEY_EN, &ali_pmu_panel_power_key);
		
	close(pmu_handle);
	return;
}


void set_ali_pmu_standby_timeout(struct rtc_time_pmu *current_time,struct rtc_time_pmu *wakeup_time)
{
	int pmu_handle;
	
	pmu_handle = open("/dev/ali_pmu", O_RDWR|O_NONBLOCK| O_CLOEXEC);
	if (pmu_handle < 0)
	{
		ALI_PMU_PRINTF("open /dev/ali_pmu fail\n");
		return;
	}
	
	base_time.year = current_time->year;
	base_time.month = current_time->month;
	base_time.date =current_time->day;
	base_time.hour  = current_time->hour;
	base_time.min = current_time->min;
	base_time.sec =current_time->sec ; 
	ALI_PMU_PRINTF("pmu_enable base_time year=%d, month=%d, date=%d, hour=%d, min=%d, sec=%d\n", \
			base_time.year, base_time.month, base_time.date, base_time.hour, base_time.min, base_time.sec);
	ioctl(pmu_handle, ALI_PMU_MCU_SET_TIME, &base_time); 
	
	ali_pmu_alarm.min_alm.en_month = 1;
	ali_pmu_alarm.min_alm.en_date = 1;
//	ali_pmu_alarm.min_alm.year = wakeup_time->year;
	ali_pmu_alarm.min_alm.month = wakeup_time->month;
	ali_pmu_alarm.min_alm.date = wakeup_time->day;
	ali_pmu_alarm.min_alm.hour = wakeup_time->hour;
	ali_pmu_alarm.min_alm.min = wakeup_time->min;
//	ali_pmu_alarm.min_alm.sec = wakeup_time->sec;
	ali_pmu_alarm.num = 0;  
	if((ali_pmu_alarm.min_alm.month!=0)&&(ali_pmu_alarm.min_alm.date!=0)&&(ali_pmu_alarm.min_alm.hour!=0)&&(ali_pmu_alarm.min_alm.min!=0))
	{
		ALI_PMU_PRINTF("@@@@@ pmu_enable alarm en_mouth=%d, en_date=%d, month=%d, date=%d, hour=%d, min=%d\n", \
				ali_pmu_alarm.min_alm.en_month, ali_pmu_alarm.min_alm.en_date, ali_pmu_alarm.min_alm.month, ali_pmu_alarm.min_alm.date, ali_pmu_alarm.min_alm.hour, ali_pmu_alarm.min_alm.min);
		ioctl(pmu_handle, ALI_PMU_MCU_WAKEUP_TIME, &ali_pmu_alarm);
	}
	close(pmu_handle);
	return;
}

 
void enter_ali_pmu_standby(void)
{	
	unsigned long power_key[8];
	int pmu_handle;
	
	pmu_handle = open("/dev/ali_pmu", O_RDWR|O_NONBLOCK| O_CLOEXEC);
	if (pmu_handle < 0)
	{
		ALI_PMU_PRINTF("open /dev/ali_pmu fail\n");
		return;
	}

	ioctl(pmu_handle, ALI_PMU_MCU_ENTER_STANDBY, 0);
	
	close(pmu_handle);
	return;
}


unsigned char  get_ali_pmu_wakeup_status(void)
{
	int pmu_handle;
	unsigned char read_status=0;
	
	pmu_handle = open("/dev/ali_pmu", O_RDWR|O_NONBLOCK|O_CLOEXEC);
	if (pmu_handle < 0)
	{
		ALI_PMU_PRINTF("open /dev/ali_pmu fail\n");
		return;
	}
	
	ioctl(pmu_handle,ALI_PMU_EXIT_STANDBY_STATUS, &read_status);
	ALI_PMU_PRINTF("wakeup status 0x%lx\n", read_status);
	if ( 0x80 == read_status) //timer wakeup
	{
		ALI_PMU_PRINTF("coming here in timer wakeup bootup!\n");
	}
	close(pmu_handle);

	/********************************
	0x1-IR_EXIT_STANDBY
	0x2-KEY_EXIT_STANDBY
	0x4-CEC_EXIT_STANDBY
	0x8-RTC_EXIT_STANDBY
	*********************************/
	return read_status;
}	
