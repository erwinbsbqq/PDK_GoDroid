#ifndef __SYS_H__
#define __SYS_H__
#include "pmu_rtc.h"
#include "pannel.h"

//=====================================================================================//
//#define PMU_MCU_M3503                                                                                    1
//#define PMU_MCU_M3821                                                                                    1
#define PMU_MCU_M3921                                                                                     1
//#define PMU_MCU_DEBUG                                                                                   1

#define PMUSRAM_GET_YEAR_H                                                                           0x3fe6
#define PMUSRAM_GET_YEAR_L                                                                           0x3fe5
#define PMUSRAM_GET_MONTH                                                                            0x3fe4
#define PMUSRAM_GET_DAY                                                                                 0x3fe3
#define PMUSRAM_GET_HOUR                                                                               0x3fe2
#define PMUSRAM_GET_MIN                                                                                 0x3fe1
#define PMUSRAM_GET_SEC                                                                                  0x3fe0

#define PMU_WORK_STATUS                                                                                 0xc20F
#define MAILBOX_GET_EXIT_STANDBY_STATUS2                                                0xc202
#define MAILBOX_GET_EXIT_STANDBY_STATUS1                                                0xc201
#define MAILBOX_GET_EXIT_STANDBY_STATUS0                                                0xc200

#define MAILBOX_WAKE_SECOND                                                                        0xc23F
#define MAILBOX_WAKE_MONTH                                                                          0xc23E
#define MAILBOX_WAKE_DAY                                                                               0xc23D  
#define MAILBOX_WAKE_HOUR                                                                            0xc23C   
#define MAILBOX_WAKE_MIN                                                                               0xc23B  

#define MAILBOX_SET_YEAR_H                                                                            0xc23A   
#define MAILBOX_SET_YEAR_L                                                                             0xc239
#define MAILBOX_SET_MONTH                                                                              0xc238
#define MAILBOX_SET_DAY                                                                                   0xc237
#define MAILBOX_SET_HOUR                                                                                 0xc236
#define MAILBOX_SET_MIN                                                                                   0xc235
#define MAILBOX_SET_SEC                                                                                   0xc234
#define MAILBOX_SET_POWERLOW3                                                                     0xc233
#define MAILBOX_SET_POWERLOW2                                                                     0xc232
#define MAILBOX_SET_POWERLOW1                                                                     0xc231
#define MAILBOX_SET_POWERLOW0                                                                     0xc230
#define SYS_IOBASE		                                                                                   0xC000
#define SYS_REG_ISR                                                                                            0xC020
#define SYS_REG_IPR                                                                                            0xC021
#define SYS_REG_IER                                                                                            0xC022
#define SYS_REG_RST			                                                                             0xC090

/*for gpio*/
#define HAL_GPIO1_DO_REG                                                                                 0xc05d//output register 
#define HAL_GPIO1_DI_REG                                                                                  0xc05c//Input Status Register
#define HAL_GPIO1_DIR_REG                                                                                0xc05e//Output control register
#define HAL_GPIO1_EN                                                                                          0xc05f//Output control register
#define HAL_GPIO_DIR_REG	                                                                             0xc056//Output control register
#define HAL_GPIO_DI_REG                                                                                   0xc054//Input Status Register
#define HAL_GPIO_DO_REG		                                                                      0xc055//output register 
#define HAL_GPIO_EN                                                                                            0xc057//GPIO ENABLE
#define HAL_GPIO_O_DIR	    	                                                                             1
#define HAL_GPIO_I_DIR                                                                                       0
#define HAL_GPIO_SET_HI	                                                                              1
#define HAL_GPIO_SER_LOW                                                                                  0

/* 3503 has 3 pin can use dor gpio*/
#if  PMU_MCU_M3503
#define XPMU_GPIO_0                                                                                             0
#define XPMU_GPIO_1                                                                                             1
#define XPMU_CEC                                                                                                   3
#else
//for gpio 0
#define XPMU_GPIO_0                                                                                              0
#define XPMU_GPIO_1                                                                                              1
#define XPMU_GPIO_2                                                                                              2
#define XPMU_GPIO_3                                                                                              3
#define XPMU_GPIO_4                                                                                              4

//for gpio1 
#define XPMU_GPIO1_0                                                                                            8
#define XPMU1_CEC                                                                                                  10
#endif

#define WRITE_BYTE(address,value)                                                                       (*(volatile unsigned char xdata *)(address)=value)
#define READ_BYTE(address)                                                                                   (*(volatile unsigned char xdata *)(address))
#define PMU_WRITE_BYTE(address,value)                                                               (*(volatile unsigned char xdata *)(address)=value)
#define PMU_READ_BYTE(address)                                                                           (*(volatile unsigned char xdata *)(address))
#define NORMAL_STATUS                                                                                          0
#define ENTER_STANDBY                                                                                           1
#define SUCCESS                                                                                                        0
#define ERROR                                                                                                            -1
#define SHOW_TYPE_PAR                                                                                          0x3fff
#define PANNEL_POWER_STATUS                                                                             0x3ffe
#define EXIT_STANDBY_TYPE                                                                                   0x3ffd

//define wakeup reason
#define EXIT_STANDBY_TYPE_PANEL                                                                       0x1
#define EXIT_STANDBY_TYPE_IR                                                                              0x2
#define EXIT_STANDBY_TYPE_RTC                                                                           0x3
#define EXIT_STANDBY_TYPE_CEC                                                                            0x4
//=====================================================================================//
typedef unsigned char                                                                                          UINT8;
typedef unsigned short                                                                                         UINT16;
typedef unsigned long                                                                                          UINT32;
typedef char                                                                                                        INT8;
typedef short                                                                                                       INT16;
typedef long                                                                                                        INT32;

//=====================================================================================//
enum KEY_MODE
{
	NORMAL=0,
	STABDY
};

void init_externint2(void);
void init_externint3(void);
void init_externint4(void);
void init_externint5(void);
extern UINT8 g_standby_flag;
extern UINT8 g_exit_standby_sts;
extern void pmu_init(void);
extern void standby_init(void);
extern void pannel_process(pRTC_TIMER rtc, enum SHOW_TYPE type);
extern void ir_process(pRTC_TIMER rtc);
extern void rtc_process(pRTC_TIMER rtc, pRTC_TIMER wakeup_rtc);
extern void hal_mcu_gpio_en(UINT8 pos);
extern void hal_mcu_gpio_disable_en(void);
extern UINT8 hal_gpio_bit_get(UINT8 pos);
extern void hal_gpio_bit_set(UINT8 pos, UINT8 val);
extern void hal_gpio_bit_dir_set(UINT8 pos, UINT8 val);
#endif
