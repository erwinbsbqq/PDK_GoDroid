/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2014.09.21			Chuhua		Creation
 ****************************************************************************/

#ifndef __INCLUDE_KERNEL_ALI_PMU_H____
#define __INCLUDE_KERNEL_ALI_PMU_H____

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <ali_pmu_common.h>
#include <asm/uaccess.h>
#include <asm/mach-ali/m36_gpio.h>
#include "ali_pan_hwscan.h"

#if defined(CONFIG_ALI_CHIP_M3921)
#include "ali_pmu_bin_3921.h"
#else
	#if defined(CONFIG_ALI_CHIP_M3515)
	#include "ali_pmu_bin_3503.h"
	#else
	#include "ali_pmu_bin_3821.h"
	#endif
#endif

#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <asm/mach-ali/m36_irq.h>
#include <linux/kthread.h>
#include <linux/types.h>
#include <linux/version.h>
#include <ali_soc_common.h>
#include <ali_reg.h>
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/init.h>
#include <linux/linux_logo.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/console.h>
#include <linux/kmod.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/efi.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/suspend.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/crc32.h>
#include <linux/ioport.h>
#include <linux/serial_core.h>
#include <linux/io.h>
#include <ali_pmu_common.h>

//=======================================================================================//
//#define DEBUG_PMU_PRINTF
#ifdef  DEBUG_PMU_PRINTF
#define PMU_PRINTF		                                                                                             printk
#else
#define PMU_PRINTF(...)	                                                                                             do{}while(0)
#endif

#if defined(CONFIG_ARM)
#define PMU_WRITE_BYTE(address,value)                                                                    (__REG32ALI(address) = value)
#define PMU_READ_BYTE(address)                                                                                 __REG32ALI(address)
#define PMU_PRS_KEY_CFG                                                                                            0x1805e010
#define KEY_DISABLE_VAL_LOW0                                                                                  (0x18+0x1805e000)
#define KEY_DISABLE_VAL_LOW1                                                                                  (0x19+0x1805e000)
#define KEY_DISABLE_VAL_LOW2                                                                                  (0x1a+0x1805e000)
#define KEY_DISABLE_EN                                                                                               (0x1b+0x1805e000)
#define PMU_CFG_SET                                                                                                    0x1805e000
#define MCU_SYS_IPR                                                                                                     0x1805c021
#define MCU_SYS_IER                                                                                                     0x1805c022
#else
#define PMU_WRITE_BYTE(address,value)                                                                     (*(volatile unsigned char *)(address)=value)
#define PMU_READ_BYTE(address)                                                                                 (*(volatile unsigned char*)(address))
#define PMU_PRS_KEY_CFG                                                                                             0xB805e010
#define KEY_DISABLE_VAL_LOW0                                                                                   (0x18+0xB805e000)
#define KEY_DISABLE_VAL_LOW1                                                                                   (0x19+0xB805e000)
#define KEY_DISABLE_VAL_LOW2                                                                                   (0x1a+0xB805e000)
#define KEY_DISABLE_EN                                                                                                (0x1b+0xB805e000)
#define PMU_CFG_SET                                                                                                     0xB805e000
#define MCU_SYS_IPR                                                                                                      0xB805c021
#define MCU_SYS_IER                                                                                                      0xB805c022
#endif

#define MAILBOX_GET_EXIT_STANDBY_STATUS2_M35X                                                0xb8000092
#define MAILBOX_GET_EXIT_STANDBY_STATUS1_M35X                                                0xb8000091
#define MAILBOX_GET_EXIT_STANDBY_STATUS0_M35X                                                0xb8000090
#define MAILBOX_GET_EXIT_STANDBY_STATUS2_M382X                                              0xb8000592
#define MAILBOX_GET_EXIT_STANDBY_STATUS1_M382X                                              0xb8000591
#define MAILBOX_GET_EXIT_STANDBY_STATUS0_M382X                                              0xb8000590
#define MAILBOX_GET_EXIT_STANDBY_STATUS2_M39X                                                0x18000592
#define MAILBOX_GET_EXIT_STANDBY_STATUS1_M39X                                                0x18000591
#define MAILBOX_GET_EXIT_STANDBY_STATUS0_M39X                                                0x18000590

#if defined(CONFIG_ARM)
/*S3921C PMU  from pmu mcu => main cpu*/
#define MAILBOX_GET_YEAR_H                                                                                       0x18053fe6
#define MAILBOX_GET_YEAR_L                                                                                        0x18053fe5
#define MAILBOX_GET_MONTH                                                                                         0x18053fe4
#define MAILBOX_GET_DAY                                                                                              0x18053fe3
#define MAILBOX_GET_HOUR                                                                                            0x18053fe2
#define MAILBOX_GET_MIN                                                                                               0x18053fe1
#define MAILBOX_GET_SEC                                                                                               0x18053fe0

/*main cpu  => pmu mcu*/
#define MAILBOX_WAKE_SECOND                                                                                      0x1805d20F
#define MAILBOX_WAKE_MONTH                                                                                        0x1805d20E
#define MAILBOX_WAKE_DAY                                                                                             0x1805d20D
#define MAILBOX_WAKE_HOUR                                                                                           0x1805d20C
#define MAILBOX_WAKE_MIN                                                                                             0x1805d20B
#define MAILBOX_SET_YEAR_H                                                                                          0x1805d20A
#define MAILBOX_SET_YEAR_L                                                                                           0x1805d209
#define MAILBOX_SET_MONTH                                                                                            0x1805d208
#define MAILBOX_SET_DAY                                                                                                0x1805d207
#define MAILBOX_SET_HOUR                                                                                              0x1805d206
#define MAILBOX_SET_MIN                                                                                                 0x1805d205
#define MAILBOX_SET_SEC                                                                                                 0x1805d204

#define NEC_IR_KEY_SARM_LOW3                                                                                      0x1805d203   
#define NEC_IR_KEY_SARM_LOW2                                                                                      0x1805d202   
#define NEC_IR_KEY_SARM_LOW1                                                                                      0x1805d201   
#define NEC_IR_KEY_SARM_LOW0                                                                                      0x1805d200
#define CPU_TO_MCU_INTERRUPT_ENABLE_ADDR                                                             0x1805d211
#define CPU_TO_MCU_INTERRUPT_STATUS_ADDR                                                             0x1805d210
#define STANDBY_SHOW_TIMR_SARM                                                                                 0x18053fff
#define SHOW_TYPE_SRAM                                                                                                  0x18053ffe
#define EXIT_STANDBY_TYPE_REG                                                                                      0x18053ffd
#define EXIT_STANDBY_TYPE_PANEL                                                                                  0x1
#define EXIT_STANDBY_TYPE_IR                                                                                         0x2
#define EXIT_STANDBY_TYPE_RTC                                                                                       0x3
#else
#define MAILBOX_GET_YEAR_H                                                                                            0xb8053fe6
#define MAILBOX_GET_YEAR_L                                                                                             0xb8053fe5
#define MAILBOX_GET_MONTH                                                                                              0xb8053fe4
#define MAILBOX_GET_DAY                                                                                                   0xb8053fe3
#define MAILBOX_GET_HOUR                                                                                                 0xb8053fe2
#define MAILBOX_GET_MIN                                                                                                    0xb8053fe1
#define MAILBOX_GET_SEC                                                                                                    0xb8053fe0

/*main cpu  => pmu mcu*/
#define MAILBOX_WAKE_SECOND                                                                                          0xb805d20F
#define MAILBOX_WAKE_MONTH                                                                                            0xb805d20E
#define MAILBOX_WAKE_DAY                                                                                                0xb805d20D
#define MAILBOX_WAKE_HOUR                                                                                              0xb805d20C
#define MAILBOX_WAKE_MIN                                                                                                 0xb805d20B

/*main cpu =>pmu mcu*/
#define MAILBOX_SET_YEAR_H                                                                                              0xb805d20A
#define MAILBOX_SET_YEAR_L                                                                                               0xb805d209
#define MAILBOX_SET_MONTH                                                                                                0xb805d208
#define MAILBOX_SET_DAY                                                                                                    0xb805d207
#define MAILBOX_SET_HOUR                                                                                                  0xb805d206
#define MAILBOX_SET_MIN                                                                                                     0xb805d205
#define MAILBOX_SET_SEC                                                                                                     0xb805d204
#define MAILBOX_SET_POWERLOW3                                                                                       0xb805d203
#define MAILBOX_SET_POWERLOW2                                                                                       0xb805d202
#define MAILBOX_SET_POWERLOW1                                                                                       0xb805d201
#define MAILBOX_SET_POWERLOW0                                                                                       0xb805d200

#define NEC_IR_KEY_SARM_LOW3                                                                                          0xb805d203
#define NEC_IR_KEY_SARM_LOW2                                                                                          0xb805d202
#define NEC_IR_KEY_SARM_LOW1                                                                                          0xb805d201
#define NEC_IR_KEY_SARM_LOW0                                                                                          0xb805d200
#define CPU_TO_MCU_INTERRUPT_ENABLE_ADDR                                                                  0xb805d211
#define CPU_TO_MCU_INTERRUPT_STATUS_ADDR                                                                  0xb805d210

// pmu sdrm maping
#define STANDBY_SHOW_TIMR_SARM                                                                                     0xb8053fff
#define LOGIC_ADDR_REG                                                                                                       0xb805e272
#define SHOW_TYPE_SRAM                                                                                                      0xb8053ffe
#define EXIT_STANDBY_TYPE_REG                                                                                          0xb8053ffd
#define EXIT_STANDBY_TYPE_PANEL                                                                                      0x1
#define EXIT_STANDBY_TYPE_IR                                                                                             0x2
#define EXIT_STANDBY_TYPE_RTC                                                                                           0x3
#endif

#if defined(CONFIG_ARM)
#define PMU_BASE                                                                                                                    0x18050000
#else
#define PMU_BASE                                                                                                                    0xb8050000
#endif

#define PMU_IRQ                                                                                                                       (20+8+32)
#define C3921_PMU_RTC0_IRQ                                                                                                 (115)
#define INT_ALI_IRQPMU                                                                                                          (108)
#define SB_CLK                                                                                                                          12000000L  
#define RC_CLK                                                                                                                          2000000L
#define SB_CNT                                                                                                                         8192L

/*PMU IR_Decode only can operate by byte */
#define PMU_CFG                                                                                                                       0x00
#define IR_RC_ADJUST                                                                                                              0x01
#define IR_RC_ADJUST1                                                                                                            0x02
#define TIMETHR                                                                                                                        0x03
#define NOISETHR                                                                                                                      0x04

/*IR1 config*/
#define IR1_THR0                                                                                                                       0x08
#define IR1_THR_NUM                                                                                                                0x18
#define IR1_DECODE_NUM                                                                                                          0x19
#define IR1_POL                                                                                                                          0x1c
#define IR1_DONCA                                                                                                                     0x28
#define IR1_THR_SEL                                                                                                                  0x34

/* IR2 config */
#define IR2_THR_NUM                                                                                                                 0x64 
#define IR2_DECODE_NUM                                                                                                          0x65
#define IR2_POL                                                                                                                          0x68
#define IR2_DONCA                                                                                                                     0x74
#define IR2_THR_SEL                                                                                                                  0x80
#define IR2_THR0                                                                                                                        0xb8    
#define PRS_KEY_CFG                                                                                                                 0xb0
#define ANALOG_SEL                                                                                                                   0xb4 
#define PMU_DEBUG_REG                                                                                                            0xb6

#if defined(CONFIG_ARM)
#define BASE_ADD	                                                                                                                 PMU_BASE
#define RTC_BASE                                                                                                                        0x18018a00
#else
#define BASE_ADD	                                                                                                                 PMU_BASE
#define RTC_BASE                                                                                                                        0xb8018a00
#endif

#define WR_RTC		                                                                                                                 RTC_BASE+0x00//write this to updata new rtc value 
#define RD_RTC		                                                                                                                 RTC_BASE+0x04
#define RD_RTC_MS	                                                                                                                 RTC_BASE+0x08
#define RD_EXIT_STANDBY                                                                                                          RTC_BASE+0x06
#define TICK_MSK	                                                                                                                  RTC_BASE+0x0a
#define IIR			                                                                                                                  RTC_BASE+0x0c//interrupt identify register
#define CONFIG0		                                                                                                                  RTC_BASE+0x10
#define PMU_RTC_EPRTC                                                                                                              RTC_BASE+0x40

/*  RTC  */
#define WR_RTC_L32                                                                                                                    RTC_BASE+0x00
#define WR_RTC_H8                                                                                                                     RTC_BASE+0x04
#define RTC_CTL_REG                                                                                                                   RTC_BASE+0x08

#define SB_TIMER0_CTL_REG                                                                                                       RTC_BASE+0x08
#define SB_TIMER1_CTL_REG                                                                                                       RTC_BASE+0x18
#define SB_TIMER2_CTL_REG                                                                                                       RTC_BASE+0x28
#define SB_TIMER3_CTL_REG                                                                                                       RTC_BASE+0x38
#define SB_TIMER4_CTL_REG                                                                                                       RTC_BASE+0x48
#define SB_TIMER5_CTL_REG                                                                                                       RTC_BASE+0x58
#define SB_TIMER6_CTL_REG                                                                                                       RTC_BASE+0x68
#define SB_TIMER7_CTL_REG                                                                                                       RTC_BASE+0x78
#define SB_TIMER3_CNT                                                                                                               RTC_BASE+0x30
#define SB_CLK_RTC                                                                                                                      27//27MHz

#define ALARM_NUM	                                                                                                                   0x0a
#define PMU_RTC_IRQ                                                                                                                   (21+8+32 )
#define CUR_MONTH	                                                                                                                   0x1e000000
#define CUR_DATE	                                                                                                                   0x01f00000
#define CUR_DAY		                                                                                                                   0x000e0000
#define CUR_HOUR	                                                                                                                   0x0001f000
#define CUR_MIN		                                                                                                                   0x00000fc0
#define CUR_SEC		                                                                                                                   0x0000003f

/*define some operations*/
#define WRITE_BYTE(address,value)                                                                                             __REG8ALI(address) = (value)
#define READ_BYTE(address)                                                                                                         __REG8ALI(address)
#define WRITE_WORD(address,value)                                                                                           __REG16ALI(address) = (value)
#define READ_WORD(address)                                                                                                       __REG16ALI(address)
#define WRITE_DWORD(address,value)                                                                                         __REG32ALI(address) = (value)
#define READ_DWORD(address)                                                                                                     __REG32ALI(address)
#define SET_BIT(address,bit)                                                                                                        __REG8ALI(address) |= (0x1<<(bit))
#define READ_BIT(address,bit)                                                                                                     (__REG8ALI(address)) & (0x1<<(bit))

#define PMU_SRAM_SIZE                                                                                                               0x4000
#ifdef CONFIG_ARM
#define WATCHDOG_REG_ADDR                                                                                                     0x18018504
#define INTERRUPT_ENABLE_REG1                                                                                                0x18000038
#define INTERRUPT_ENABLE_REG2                                                                                                0x1800003c
#define PMU_IP_RESET_REG                                                                                                          0x18000320
#define PMU_RAM_SWITCH_REG                                                                                                    0x1805d100
#define CPU_TO_MCU_IE_REG1                                                                                                      0x1805d211
#define CPU_TO_MCU_IE_REG2                                                                                                      0x1805d210
#define PMU_SRAM_BASE_ADDR                                                                                                     0x18050000
#define UART_OUTPUT_REG                                                                                                            0x18018300
#else
#define WATCHDOG_REG_ADDR                                                                                                      0xb8018504
#define INTERRUPT_ENABLE_REG1                                                                                                 0xb8000038
#define INTERRUPT_ENABLE_REG2                                                                                                 0xb800003c
#define PMU_IP_RESET_REG                                                                                                           0xb8000084
#define PMU_RAM_SWITCH_REG                                                                                                     0xb805d100
#define CPU_TO_MCU_IE_REG1                                                                                                      0xb805d211
#define CPU_TO_MCU_IE_REG2                                                                                                      0xb805d210
#define PMU_SRAM_BASE_ADDR                                                                                                     0xb8050000
#define UART_OUTPUT_REG                                                                                                            0x18018300
#endif
#define M3821_POWER_OFF_ENABLE_REG                                                                                     0xb805c05f

//=======================================================================================//
#if defined(CONFIG_ALI_CHIP_M3701)
static unsigned  char g_panel_key_en = 0;
#endif

/*    pm_key.ir_power[0] = 0x10effb04;
        pm_key.ir_power[1] = 0x60df708f;
        pm_key.ir_power[2] = 0x10efeb14;
        pm_key.ir_power[3] = 0x00ff00ff;
        pm_key.ir_power[4] = 0x00ff807f;
        pm_key.ir_power[5] = 0x00ff40bf;
        pm_key.ir_power[6] = 0x10ef9b64;
        pm_key.ir_power[7] = 0x10efbb44;
*/		
//static unsigned int wakeup_power_key = 0x60df708f;
static unsigned long  wakeup_power_key[8] = {0x60df708f, 0x10effb04, 0x10efeb14, 0x00ff00ff, 0x00ff807f, 0x00ff40bf, 0x10ef9b64, 0x10efbb44};
static spinlock_t rtc_lock;
static unsigned int RTC_DIV[4] = {32, 64, 128, 256};
static unsigned int chip_id = 0;
static unsigned int rev_id = 0;
static unsigned long long rtc_upper = 0xFFFFFFFFFF;
static unsigned long long rtc_onesec = 0xFFFFF32019;
struct rtc_base_time base_time_0 = {30, 30, 11, 16, 8, 0, 1, 1, 1, 1, 1, 1, 1, 1};
struct rtc_base_time base_time_3 = {30, 30, 11, 16, 8, 0, 1, 1, 1, 1, 1, 1, 1, 1};
unsigned char rtc_sec = 30;
unsigned char rtc_min = 30;
unsigned char rtc_hour = 11;
unsigned char rtc_date = 16;
unsigned char rtc_month = 8;
unsigned int  rtc_year = 0; 
unsigned char g_year_h=1;
unsigned char g_year_l=1;
unsigned int  g_year=1;
unsigned char g_month=1;
unsigned char g_day=1;
unsigned char g_hour=1 ;
unsigned char g_min=1;
unsigned char g_sec=1;
unsigned char g_exit_standby_sts=0;
static unsigned short key=KEY_POWER;
static struct input_dev *pmu_input;
extern void ali_suspend_register_ops(void);

struct pmu_private
{
	unsigned long reserved;
};

struct pmu_device
{
	/* Common */
	char *dev_name;

	/* Hardware privative structure */
	struct pmu_private *priv;	
	struct device *pmu_device_node;
	struct cdev pmu_cdev;
	struct class *pmu_class;
	dev_t dev_no;
};

enum RTC_CTL_REG_CONTENT
{
	RTC_TOV = 1 << 3,
	RTC_DIV_32 = 0,
	RTC_DIV_64 = 1,
	RTC_DIV_128 = 2,
	RTC_DIV_256 = 3,
}; 

enum SB_TIMER
{
	SB_TIMER_0 = 0,
	SB_TIMER_1,
	SB_TIMER_2,
	SB_TIMER_3,
	SB_TIMER_4,
	SB_TIMER_5,
	SB_TIMER_6,
	SB_TIMER_7,
};

typedef struct pmu_ir_key
{
	unsigned short ir_thr[8];
	unsigned char  ir_thr_cnt ; 
	unsigned char  ir_decode_cnt;
	unsigned char  ir_pol[12];
	unsigned char  ir_pol_cnt;
	unsigned char  ir_donca[12];
	unsigned char  ir_donca_cnt;
	unsigned char  ir_thr_sel[48];
	unsigned char  ir_thr_sel_cnt;
	unsigned char  flg;
	unsigned char  type;
} PMU_IR;

enum irp_type
{
	IR_TYPE_NEC = 0,
	IR_TYPE_LAB,
	IR_TYPE_50560,
	IR_TYPE_KF,
	IR_TYPE_LOGIC,
	IR_TYPE_SRC,
	IR_TYPE_NSE,
	IR_TYPE_RC5,
	IR_TYPE_RC5_X,
	IR_TYPE_RC6,
};

enum value_key_press
{
	KEY_RELEASE		= 0,
	KEY_PRESSED		= 1,
	/*	KEY_REPEAT			= 2 */
};
#endif
