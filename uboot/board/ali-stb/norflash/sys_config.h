/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    sys_config.h
*
*    Description:    This file contains all system configuration switches and
*						parameter definations.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Jan.18.2003       Justin Wu       Ver 0.1    Create file.
*	2.	Feb.10.2003		  Justin Wu       Ver 0.2    Update
*	3.  2003.2.13         Liu Lan         Ver 0.3    M3325 config
*****************************************************************************/

#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_
#include "sys_define.h"
#define HW_SECURE_ENABLE    //security enable						
#define DUAL_CPU
#define HDMI_CEC_ENABLE
#define PAN_INFO_ALIGN
#define FIRMWARE_SIGN

#if (defined _BOOT_LOGO_)
#include "main_config_logo.h"
#endif
#define DB_VERSION			40


#define SYS_PROJECT				SYS_DEFINE_NULL	/* Target project */

#define SYS_VERSION_BL			"01.000.00"		/* Boot loader: %02d.%03d.%02d */
#define SYS_VERSION_SW			"00.000.00"		/* Software:    %02d.%03d.%02d */
#define SYS_VERSION_PL			"00.000.00"		/* Program list:%02d.%03d.%02d */

//#define SYS_OS_MODULE			ALI_TDS2			/* OS configuration */

#define SYS_MW_MODULE			SYS_DEFINE_NULL	/* MW configuration */

#define SYS_CA_MODULE			SYS_DEFINE_NULL	/* CA configuration */

#define SYS_EPG_MODULE			SYS_DEFINE_NULL	/* EPG configuration */

#define SYS_PROJECT_FE	PROJECT_FE_DVBT
/****************************************************************************
 * Section for HW configuration, include bus configuration.
 ****************************************************************************/
/* customer loader build related */
//#define ENABLE_4M_FLASH

//#define BL_DISABLE_PRINTF
#if((defined _M3603_01V01_) || (defined _M3383_SABBAT_))
#define SYS_MAIN_BOARD          BOARD_DB_M3603_01V01
#endif

#ifdef _M3383_01V01_
#define SYS_MAIN_BOARD          BOARD_DB_M3383_01V01
#endif

#ifdef _M3603_02V01_
#define SYS_MAIN_BOARD          BOARD_DB_M3603_02V01
#endif

#ifndef M3606_SECURE_ENABLE
#define CA_NO_CRC_ENABLE
#else
#define M3603_NORMAL_LINUX
#endif 

//#define SUPPORT_EJTAG_VERIFY
//#define SUPPORT_ENHANCED_EJTAG_VERIFY
//#define USE_SMC1

/* makefile.cmd for LLD_PAN_XXX */
#define SYS_CHIP_MODULE			ALI_S3602		/* Chip configuration */

#define SYS_CPU_MODULE			CPU_MIPS24KE		/* CPU configuration */
#define SYS_CPU_ENDIAN			ENDIAN_LITTLE	/* CPU endian */

#define SYS_CHIP_CLOCK			27000000		/* Extarnal clock */
#define SYS_CPU_CLOCK			298000000		/* CPU clock */


#if (SYS_MAIN_BOARD == BOARD_DB_M3383_01V01)
#define SYS_SDRAM_SIZE			64
#define SDRAM_BIT_MODE			16
#elif (SYS_MAIN_BOARD == BOARD_DB_M3603_01V01||SYS_MAIN_BOARD == BOARD_DB_M3603_02V01)
#define SYS_SDRAM_SIZE			128
#define SDRAM_BIT_MODE			32
#endif	

#define SYS_GPIO_MODULE			M3602F_GPIO		/* GPIO configuration */

#define SYS_I2C_MODULE			M6303I2C		/* I2C configuration */
#define SYS_I2C_SDA				SYS_DEFINE_NULL	/* I2C SDA GPIO pin number */
#define SYS_I2C_SCL				SYS_DEFINE_NULL	/* I2C SDL GPIO pin number */

#define SYS_PCI_MODULE			SYS_DEFINE_NULL	/* PCI configuration */

#define SYS_SCI_MODULE			UART16550		/* SCI configuration */

//#define DDR_POWER_CONTROL_ENABLE // use a GPIO to control the DDR power on/off
#ifdef DDR_POWER_CONTROL_ENABLE
	#define DDR_POWER_CTL_GPIO_POS		37		/* depends on board design */
	#define DDR_POWER_CTL_GPIO_POLAR	1		/* depends on board design */
#else
	#define DDR_POWER_CTL_GPIO_POS		0xffff	/* invalid GPIO */
	#define DDR_POWER_CTL_GPIO_POLAR	0
#endif

/************** common data define ****************/
#ifndef DISABLE_PRESET_CLOCK
#if (SYS_CHIP_MODULE == ALI_M3327C && SYS_SDRAM_SIZE == 2)
#define SYS_COMMON_DATA_BASE_ADDR               0xbfc0fe80
#define SYS_COMMON_DATA_BASE_ADDR               0xbfc0fe80
#define SYS_DEFAULT_CPU_CLOCK_OFFSET                    0x00
#define SYS_DEFAULT_MEM_CLOCK_OFFSET                    0x01
#define SYS_DEFAULT_MEM_READ_CLOCK_DELAY_CHAIN_OFFSET   0x02
#define SYS_DEFAULT_MEM_CLOCK_TREE_DELAY_CHAIN_OFFSET   0x03

#define SYS_CPU_CLOCK_OFFSET                    		0x60
#define SYS_MEM_CLOCK_OFFSET                  			0x61
#define SYS_MEM_READ_CLOCK_DELAY_CHAIN_OFFSET   		0x62
#define SYS_MEM_CLOCK_TREE_DELAY_CHAIN_OFFSET   		0x63
#endif
#endif

/****************************************************************************
 * Section for LLD configuration.
 ****************************************************************************/
//#define SYS_FLASH_MODULE        AM29800B		/* Flash configuration */
//#define SYS_FLASH_BASE_ADDR		0xafc00000		/* Flash base address */
//#define SYS_FLASH_SIZE			0x200000		/* Flash size, in byte */
#include "../../../include/configs/ali-stb.h"
#define SYS_FLASH_BASE_ADDR		CONFIG_SYS_FLASH_BASE		/* Flash base address */

#define SYS_EEPROM_MODULE		SYS_DEFINE_NULL	/* EEPROM configuration */
#define SYS_EEPROM_BASE_ADDR	0xA0			/* EEPROM base address */
#define SYS_EEPROM_SIZE			1024			/* EEPROM size, in byte */

#define SYS_NET_MOUDLE			SYS_DEFINE_NULL	/* Net configuration */

#define SYS_DMX_MODULE			SYS_DEFINE_NULL	/* Demux configuration */

//#define SYS_TUN_MODULE			SYS_DEFINE_NULL	/* Tuner configuration */
//#define SYS_TUN_BASE_ADDR		SYS_DEFINE_NULL	/* Tuner device address */

//#define SYS_DEM_MODULE			SYS_DEFINE_NULL	/* Demodulator configuration */
//#define SYS_DEM_BASE_ADDR		SYS_DEFINE_NULL	/* Demodulator device address */

//#define SYS_RFM_MODULE			SYS_DEFINE_NULL	/* RF modulator configuration */
//#define SYS_RFM_BASE_ADDR		SYS_DEFINE_NULL	/* RF modulator device address */

#define SYS_IRP_MOUDLE			ROCK00 //GMI_00			/* IR Pad configuration */
//#define SYS_PAN_MOUDLE			GMI_PAN_SL65
#define SYS_PAN_MOUDLE			GMI_PAN_SL35

#define PANEL_DISPLAY

//#define LOGO_ID					0x02FD0100

#define STANDBY_SUSPEND				0
#define STANDBY_SHOW_PANEL			1
#define STANDBY_PANEL_SHOW_TIMER	2
#define STANDBY_PANEL_SHOW_OFF		3
#define STANDBY_PANEL_SHOW_BLANK	4
#define STANDBY_ACTION				STANDBY_SHOW_PANEL

#define	STANDBY_PANEL_SHOW_WAHT	STANDBY_PANEL_SHOW_TIMER//STANDBY_PANEL_SHOW_TIMER
#define IRP_KEY_STANDBY			0x807fc03f
#define IRP_KEY_STANDBY2		0x60df708f

#if (SYS_MAIN_BOARD == BOARD_DB_M3603_01V01||SYS_MAIN_BOARD == BOARD_DB_M3383_01V01)
#ifdef PANEL_16515_VFD
#define PAN_KEY_STANDBY			0xFFFF0004
#else
#define PAN_KEY_STANDBY			0xFFFF0008
#endif
#else
#define PAN_KEY_STANDBY			0xffff0001
#endif

#ifdef PANEL_DISPLAY
#if(SYS_MAIN_BOARD == BOARD_DB_M3603_02V01) 
#define MAX_GPIO_NUM                   127
#define FP_LOCK_GPIO_NUM            4//127
#define FP_STANDBY_GPIO_NUM         127//105
#define FP_CLOCK_GPIO_NUM           46//127
#define FP_DATA_GPIO_NUM            24
#define FP_CS_GPIO_NUM            	127
#define FP_KEY1_GPIO_NUM            63//127
#define FP_COM1_GPIO_NUM            59//127
#define FP_COM2_GPIO_NUM            60//127
#define FP_COM3_GPIO_NUM            61//127
#define FP_COM4_GPIO_NUM            62//127
#define GPIO_USB_POWER            	127
#elif(SYS_MAIN_BOARD == BOARD_DB_M3383_01V01) 
#define MAX_GPIO_NUM                   95
#define FP_LOCK_GPIO_NUM            55
#define FP_STANDBY_GPIO_NUM      95
#define FP_CLOCK_GPIO_NUM           46
#define FP_DATA_GPIO_NUM            60
#define FP_CS_GPIO_NUM            	95
#define FP_KEY1_GPIO_NUM            53
#define FP_KEY2_GPIO_NUM            52
#define FP_COM1_GPIO_NUM            59
#define FP_COM2_GPIO_NUM            63
#define FP_COM3_GPIO_NUM            62
#define FP_COM4_GPIO_NUM            61
#define GPIO_USB_POWER            	95
#else
#define MAX_GPIO_NUM                   127
#define FP_LOCK_GPIO_NUM            127
#define FP_STANDBY_GPIO_NUM         127
#define FP_CLOCK_GPIO_NUM           127
#define FP_DATA_GPIO_NUM            127
#define FP_KEY1_GPIO_NUM            127
#define FP_COM1_GPIO_NUM            127
#define FP_COM2_GPIO_NUM            127
#define FP_COM3_GPIO_NUM            127
#define FP_COM4_GPIO_NUM            127
#endif
#endif
//#define GUNZIP_SUPPORT
//#define TRANSFER_FORMAT2_SUPPORT

#ifdef _EROM_E_
#define ENABLE_EROM
#endif
#ifdef _MULTI_SECTION_E
#define SUPPORT_MULTI_SECTION
#endif

#define ENABLE_SERIAL_FLASH


#define GPIO_MUTE	0
#define SCART_MUTE	1

#if (SYS_MAIN_BOARD == BOARD_S3602_DEMO || SYS_MAIN_BOARD == BOARD_ATSC_DEMO_00)
#define SYS_MUTE_MODE				GPIO_MUTE
#define MUTE_CIRCUIT_GPIO_NUM		67

#elif (SYS_MAIN_BOARD == BOARD_DB_M3603_01V01||SYS_MAIN_BOARD == BOARD_DB_M3603_02V01||SYS_MAIN_BOARD == BOARD_DB_M3383_01V01)
#define SYS_MUTE_MODE				GPIO_MUTE
#if(SYS_MUTE_MODE == GPIO_MUTE)
  #define MUTE_CIRCUIT_GPIO_NUM		76
#else
  #define SCART_POWER_DOWN_GPIO_NUM	70
#endif

#endif



#define __MM_VOID_BUFFER_LEN	0x00200000	//2M
#define __MM_SHARED_MEM_LEN  	256

#if(SYS_MAIN_BOARD == BOARD_DB_M3383_01V01)
#define ENTER_SW_UPGRADE_BY_KEY
#define SUPPORT_UPDATE_UK
#define LZO_COMPRESS_ENABLE
#define __MM_PRIVATE_SHARE_LEN	0x00e00000	//14M
#define __MM_HIGHEST_ADDR  		0xa4000000		//64M
#elif(SYS_MAIN_BOARD == BOARD_DB_M3603_01V01||SYS_MAIN_BOARD == BOARD_DB_M3603_02V01)
#ifdef _M3383_SABBAT_
#define __MM_PRIVATE_SHARE_LEN	0x01a00000	//26M
#define __MM_HIGHEST_ADDR  		0xa4000000		//64M
#else
#define __MM_PRIVATE_SHARE_LEN	0x01e00000	//30M
#define __MM_HIGHEST_ADDR  		0xa8000000		//128M
#endif
#define  GEN_ROOT_KEY_FOR_PVR_EN
#define OTP_ROOT_PVR_KEY_ADDR  0x70
#else
    #define __MM_PRIVATE_SHARE_LEN	0x01e00000	//30M
    #define __MM_HIGHEST_ADDR  		0xa8000000		//128M    
#endif

#define __MM_PRIVATE_LEN		(__MM_PRIVATE_SHARE_LEN-__MM_SHARED_MEM_LEN)
#define __MM_VOID_BUFFER_ADDR	(__MM_HIGHEST_ADDR - __MM_VOID_BUFFER_LEN)//
#define __MM_SHARE_BASE_ADDR 	(__MM_VOID_BUFFER_ADDR-__MM_SHARED_MEM_LEN)
#define __MM_PRIVATE_TOP_ADDR 	(__MM_SHARE_BASE_ADDR)
#define __MM_PRIVATE_ADDR		(__MM_PRIVATE_TOP_ADDR-__MM_PRIVATE_LEN)//
#define __MM_SEE_FW_BOOT_ADDR  	(__MM_PRIVATE_ADDR|0x200)


#define __MM_HEAP_TOP_ADDR		__MM_PRIVATE_ADDR	//


#define SEE_ENTRY (__MM_PRIVATE_ADDR + 0x200)
#define HOOK_INT_HANDLER_BY_CP0

#endif	/* _SYS_CONFIG_H_ */
