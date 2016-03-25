#ifndef _ALI_PMU_H_#define _ALI_PMU_H_#ifdef __cplusplusextern "C" {#endif#define ALI_PMU_IRX_POWER_KEY	0x01#define ALI_PMU_CEC_POWER_KEY	0x02 
struct rtc_time_pmu
{
     unsigned int  year;  
    	unsigned char month; 	unsigned char date;
	unsigned char day; 
	unsigned char hour;
	unsigned char min; 
	unsigned char sec; 
};

/*!@brief setup pmu standby ir wakeup key@param[in] ip_power_key : pmu standby ir wakeup key@return void*/void set_ali_pmu_standby_ir_power_key(unsigned long * ir_power_key);/*!@brief setup pmu standby panel wakeup key@param[in] panel_power_key : pmu standby panel wakeup key@return void*/void set_ali_pmu_standby_panel_power_key(unsigned long panel_power_key);/*!@brief setup pmu standby wakeup timer@param[in] current_time : the current time @param[in] wakeup_time : the time standby will wakeup@return void*/void set_ali_pmu_standby_timeout(struct rtc_time_pmu *current_time,struct rtc_time_pmu *wakeup_time);/*!@brief enter pmu standby*/void enter_ali_pmu_standby(void);/*!@brief get pmu wakeup status@return wakeup status@retval  wakeup from ir power key@retval  wakeup from panel power key@retval  wakeup from timer*/unsigned char  get_ali_pmu_wakeup_status(void);
#ifdef __cplusplus}#endif
#endif /*_ALI_PMU_H_*/
