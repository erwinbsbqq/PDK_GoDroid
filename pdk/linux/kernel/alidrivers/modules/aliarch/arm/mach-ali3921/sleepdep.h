/*
 * sleep dependnecy.h
 * This file contains all the sleep hardware dependencies.
 *
 */
#ifndef __ALI_M36_SLEEP_DEPENDENCY_INCLUDE____H__
#define __ALI_M36_SLEEP_DEPENDENCY_INCLUDE____H__

#include <linux/suspend.h>
#include <linux/delay.h>

#include <asm/io.h>


#define VALUE_TOUT			      24000   /* Timeout threshold, in uS */
#define VALUE_CLK_CYC		      8       /* Work clock cycle, in uS */
#define VALUE_NOISETHR		    80      /* Noise threshold, in uS */
#define IR_RLC_SIZE			      256
#define IO_BASE_ADDR          0x18000000
#define AUDIO_IO_BASE_ADDR    0x18002000
#define USB_IO_BASE_ADDR      0x1803d800
#define HDMI_PHY 	            0x1800006c
#define SYS_IC_NB_BASE_H		  0x1800
#define SYS_IC_NB_CFG_SEQ0    0x74
#define SYS_IC_NB_BIT1033     0x1033
#define SYS_IC_NB_BIT1031     0x1031


#define PM_ENABLE_DEVICE 1
#define PM_DISABLE_DEVICE 0

#define PM_ENTER_STANDBY 1
#define PM_EXIT_STANDBY 0

extern int ali_3921_enter_lowpower(unsigned int cpu, unsigned int power_state);
extern int ali_3921_finish_suspend(unsigned long cpu_state);
extern void ali_3921_cpu_resume(void);

void operate_device(int enable);
void pm_standby_prepare(int enter);
void operate_time_count(unsigned long timeout);


#endif

