#if 0

#ifndef __SYS_CONFIG_H
#define __SYS_CONFIG_H

#include <ali/sys_define.h>
#include <config.h>

//#ifdef _CAS9_CA_ENABLE_
#define DUAL_ENABLE
//#endif

#if defined DUAL_ENABLE || defined _M36F_SINGLE_CPU
	//#define SYS_MAIN_BOARD BOARD_DB_M3606_01V01
	#define SYS_MAIN_BOARD BOARD_DB_M3701C_01V01
	//#define SYS_MAIN_BOARD BOARD_DB_M3603_01V01
	//#define SYS_MAIN_BOARD BOARD_SB_S3602F_QFP_01V01
	//#define SYS_MAIN_BOARD BOARD_DB_M3603_02V01

	#define M36F_CHIP_MODE
	
#else
	#define SYS_MAIN_BOARD BOARD_DB_M3701C_01V01
#endif

//#define C3041 // please QT don't enable this macro

#ifndef _BUILD_OTA_E_
//#define USB_LOGO_TEST
#endif

#ifdef _CAS9_CA_ENABLE_ 
   #define ALI_AS_BOOT
#endif

#if(SYS_MAIN_BOARD == BOARD_DB_M3701C_01V01)
#define  TVE_USE_FULL_CURRENT_MODE
#endif
//#define AUTO_SYANDBY_DEFAULT_ON
//#define MULTIFEED_SUPPORT
//#define ALI_SHUTTLE_MODIFY
//#define SUPPORT_UNICABLE
//#define SUPPORT_DISEQC11
//#define UI_MEDIA_PLAYER_REPEAT_PLAY
//#define TVE_USE_FULL_CURRENT_MODE
#define WATCH_DOG_SUPPORT
#define RTC_TIME_SUPPORT 
//#define DO_DDP_CERTIFICATION
//#define SUPPORT_AUDIO_DELAY
//#define MP_PREVIEW_SWITCH_SMOOTHLY
//#define CHANCHG_VIDEOTYPE_SUPPORT
//#define AFD_SW_SUPPORT
//#define AFD_HW_SUPPORT

/* Add "RAM_TMS_TEST" for test: doing timeshift using RAM disk. 
 * If open it, MUST re-define "RAM_DISK_ADDR" and "RAM_DISK_SIZE".
 */
//#define RAM_TMS_TEST 
#define ATSC_SUBTITLE_SUPPORT

#ifdef ATSC_SUBTITLE_SUPPORT
#define __MM_DCII_SUB_BS_LEN (16*1024)
#define __MM_SUBT_ATSC_SEC_LEN (720*576)
#endif

#ifdef _SFU_TEST_SUPPORT
#define SFU_TEST_SUPPORT
#endif

#ifdef _DXXX_CERT_ENABLE
#define DXXX_CERT_ENABLE
#endif

#define BOARD_S3602F_MAX    (BOARD_SB_S3602F_QFP_01V01 + 100)

#ifdef M36F_CHIP_MODE
	#define HDMI_TV_TEST
	#define DUAL_VIDEO_OUTPUT_USE_VCAP
	#define HDMI_1080P_SUPPORT
	#define HDMI_ENABLE     //  enable HDMI in vpo driver because sabbat ui cannot disable it.
	#define DRAM_SPLIT
	#define OSD_16BIT_SUPPORT
//	#ifndef _BUILD_OTA_E_ // for ota compile
		#define USB_SUPPORT_HUB
//	#endif	
	#define VIDEO_DEBLOCKING_SUPPORT
	#if defined(MAIN_CPU) || defined(_M36F_SINGLE_CPU)
		#define GE_SIMULATE_OSD
	#endif	
	//#define HW_SECURE_ENABLE
	#define AUDIO_DESCRIPTION_SUPPORT	
#ifndef _BUILD_OTA_E_	
	//#define CEC_SUPPORT
	//#define HDMI_CERTIFICATION_UI
#endif

#ifdef _WIFI_ENABLE_
	#define WIFI_SUPPORT
#endif	
	
	/*************************/
	#ifdef _CAS9_CA_ENABLE_
		#define SUPPORT_CAS9
		#define SUPPORT_HW_CONAX
		#define MULTI_CAS
		#define DSC_SUPPORT
		#define CAS_TYPE	CAS_CONAX
		#define HW_SECURE_ENABLE
		#define CAS9_PVR_SUPPORT
        #define DISABLE_PRINTF
        #define HDCP_FROM_CE
		#define AUDIO_CHANNEL_LANG_SUPPORT
        #define STO_PROTECT_BY_MUTEX
		#ifdef _M3383_SABBAT_
			#define SD_PVR
			#define MEDIAPLAYER_SUPPORT
            #undef MP_PREVIEW_SWITCH_SMOOTHLY
            #undef CHANCHG_VIDEOTYPE_SUPPORT
		#endif
	#endif
	/**************************/

    #define AV_DELAY_SUPPORT
#else
	#define DUAL_VIDEO_OUTPUT
#endif

#ifdef OSD_16BIT_SUPPORT
//#define SUPPORT_DRAW_EFFECT
#endif

#if(SYS_MAIN_BOARD == BOARD_DB_M3606_01V01 || SYS_MAIN_BOARD == BOARD_DB_M3603_01V01|| SYS_MAIN_BOARD ==BOARD_DB_M3701C_01V01)

#ifdef WIFI_SUPPORT
	#define SYS_NETWORK_MODULE 	NET_ALIETHMAC
	#define USB_DRIVER_WIFI_SUPPORT
	#define NETWORK_SUPPORT
#else	
	#define SYS_NETWORK_MODULE 	NET_ALIETHMAC
#endif	
	//#define NETWORK_SUPPORT
	#define SHOW_TWO_TUNER_LOCK_STATUS
#elif (SYS_MAIN_BOARD == BOARD_SB_S3602F_QFP_01V01)
	#define SYS_NETWORK_MODULE 	NET_ALIETHMAC
	#define NETWORK_SUPPORT
#endif

#if(SYS_MAIN_BOARD == BOARD_DB_M3606_01V01)
	//#define SDIO_SUPPORT
	#define SYS_BOARD_IS_M3606_01V02	
    //#define SYS_BOARD_IS_M3606_01V04    
#elif( SYS_MAIN_BOARD == BOARD_DB_M3603_01V01)
	#define SDIO_SUPPORT
#elif( SYS_MAIN_BOARD == BOARD_DB_M3602_04V01)
	#define SDIO_SUPPORT
#endif

#ifdef _SHOW_ALI_DEMO_ON_SCREEN
#define SHOW_ALI_DEMO_ON_SCREEN
#endif

#ifdef _SATA_SUPPORT
#define SATA_SUPPORT
#endif

#if( defined (AFD_SW_SUPPORT)) || ( defined(AFD_HW_SUPPORT))
#define SUPPORT_AFD_PARSE
#define SUPPORT_AFD_SCALE
#define SUPPORT_AFD_WSS_OUTPUT
#endif

#define SUPPORT_MPEG4_TEST
#define MP_SUBTITLE_SUPPORT
#define FS_STDIO_SUPPORT
#define RAM_DISK_SUPPORT

#ifndef OSD_16BIT_SUPPORT
	#define HDOSD_DEO_OUTPUT
#endif

#define HDCP_IN_FLASH
//#define HDCP_FROM_CE

#define UNIFIED_CI_PATCH
#define AUTOMATIC_STANDBY
//#define NETWORK_SUPPORT

#define PVR3_SUPPORT
#define NEW_EPG_ARCH
#define ISO8859_SUPPORT
#define SYS_AGC_SNR_API
#define SYS_TUN_MODULE_UNIFIED_API 
#define MULTI_AUDIO_PID
#define M3327SW_AUTOSCAN
#define TUNER_OPTION_IX2410
#define DUP_TRANSPONDER_REMOVAL 	1
#define SYS_PSEARCH_SCAN_TP_ALL		SYS_FUNC_ON
#define SYS_PROJECT					SYS_DEFINE_NULL

#ifdef _SYSTEM_DVBC
#define SYS_PROJECT_FE				PROJECT_FE_DVBC
#if(SYS_MAIN_BOARD == BOARD_DB_M3701C_01V01)
#define SYS_TUNER_MODULE 			DCT70701//ALPSTDAE
#endif
#else
#define SYS_PROJECT_FE				PROJECT_FE_DVBS
#endif

#if(SYS_MAIN_BOARD == BOARD_DB_M3701C_01V01)
#define SYS_GPIO_MODULE				M3602F_GPIO
#else
#define SYS_GPIO_MODULE				M6303GPIO
#endif

#define SYS_PROJECT_SM				PROJECT_SM_CI
#define SYS_OS_MODULE				ALI_TDS2
#define SYS_MW_MODULE				SYS_DEFINE_NULL
#define SYS_CA_MODULE				SYS_DEFINE_NULL
#define SYS_EPG_MODULE				SYS_DEFINE_NULL
#define SYS_CHIP_MODULE				ALI_S3602
#define SYS_CPU_MODULE				CPU_MIPS24KE
#define SYS_CPU_ENDIAN				ENDIAN_LITTLE
#define SYS_CHIP_CLOCK				27000000
//#define SYS_CPU_CLOCK				//396000000
#define SDRAM_SIZE					128

#define SYS_I2C_MODULE				M6303I2C
#define SYS_I2C_SDA					3
#define SYS_I2C_SCL					4
#define GPIO_I2C_CONTROL_TUNER
#define SYS_SCI_MODULE				UART16550
#define SYS_TSI_MODULE				M3327TSI
#define SYS_FLASH_BASE_ADDR			0xafc00000
#define SYS_FLASH_SIZE				0x400000
#define SYS_DMX_MODULE				M3327DMX
#define SYS_LNB_POWER_OFF			SYS_FUNC_ON
#define SYS_TUN_BASE_ADDR			0xc0
#define SYS_DEM_MODULE				ALI_S3501
#define SYS_RFM_MODULE				MCBS373
#define SYS_RFM_BASE_ADDR			0xca
#define SYS_IRP_MOUDLE				ROCK00
#define TABLE_BLOCK_SPACE_SIZE		(64 * 1024)
#define MAX_PROG_NUM				5000
#define MAX_TP_NUM					3000
#define MAX_SAT_NUM					64
#define TTX_ON						1
#define SUBTITLE_ON					1
#define TTX_BY_OSD
#define ALL_BY_VBI
#define TTX_BY_VBI
#define TTX_COLOR_NUMBER			256
#define DECODE_LIB_MAD				1
#define DECODE_LIB					DECODE_LIB_MAD
#define VDEC_AV_SYNC
//#define AV_DELAY_SUPPORT
#define COLOR_N						256
#define LOCAL_VSCR					0
#define OSD_VSCREEN					LOCAL_VSCR
#define OSD_MAX_WIDTH				1008
#define OSD_MAX_HEIGHT				640
#define LIST_SUPPORT				1
#define MATRIXBOX_SUPPORT			1
#define USB_MP_SUPPORT
//#define CI_SUPPORT
#define ISO_5937_SUPPORT
#define DB_USE_UNICODE_STRING
#define NEW_DISEQC_LIB
#define OSD_STARTCOL				((1280 - OSD_MAX_WIDTH)>>1)
#define OSD_STARTROW_N				((720 - OSD_MAX_HEIGHT)>>1)
#define OSD_STARTROW_P				((720 - OSD_MAX_HEIGHT)>>1)
#define AC3DEC						1
#define PSI_MONITOR_SUPPORT
#define TTX_GREEK_SUPPORT
#define TTX_ARABIC_SUPPORT 
#define TTX_ARABIC_G2_SUPPORT

#define DB_VERSION					40
#define ALI_SDK_API_ENABLE

#if (SYS_MAIN_BOARD == BOARD_DB_M3603_02V01)
#define GPIO_NULL 127//63
#else
#define GPIO_NULL 63
#endif


#define TRUE_COLOR_HD_OSD
//#define SD_PVR
#define PVR_FS_SH
#ifdef _MP2_0_SUPPORT
	#define MP2_0_SUPPORT
	#define PS_TS_SUPPORT
#endif

#ifndef SD_PVR
#define HD_SUBTITLE_SUPPORT
#endif
#define HDTV_SUPPORT
#define SUPPORT_DUAL_OUTPUT_ONOFF   0
#define DISPLAY_SETTING_SUPPORT
#define GPIO_RGB_YUV_SWITCH     68

#ifdef SD_PVR
	#ifdef DUAL_VIDEO_OUTPUT
		#undef DUAL_VIDEO_OUTPUT
	#endif
	#ifdef DUAL_VIDEO_OUTPUT_USE_VCAP
		#undef DUAL_VIDEO_OUTPUT_USE_VCAP
	#endif	
#endif

#define TTX_SUB_PAGE
#define SUPPORT_PACKET_26


//not in M3327.mdf
#define LIB_TSI3_FULL
#define SYS_I2C_SDA2 64
#define SYS_I2C_SCL2 65
#define ENABLE_SERIAL_FLASH
#define SUPPORT_ERASE_UNKOWN_PACKET //for ttx

#ifdef _CAS9_CA_ENABLE_
	#undef SYS_PROJECT_SM
	#define SYS_PROJECT_SM				PROJECT_SM_CA

	#ifdef SDIO_SUPPORT
	#undef SDIO_SUPPORT
	#endif

	#ifdef NETWORK_SUPPORT
	#undef NETWORK_SUPPORT
	#endif

	#ifdef AV_DELAY_SUPPORT
	#undef AV_DELAY_SUPPORT
	#endif

	#ifdef SATA_SUPPORT
	#undef SATA_SUPPORT
	#endif

	#ifdef WIFI_SUPPORT
	#undef WIFI_SUPPORT
	#endif
#endif

#if (SYS_PROJECT_SM ==PROJECT_SM_CI)
    #if (SYS_MAIN_BOARD== BOARD_DB_M3606_01V01)
    #define CONFIG_SLOT_NUM	1
    #define PFLASH_CI_SHARE_PIN
    #else
    #define CONFIG_SLOT_NUM	2//1,2
    #endif
#define CI_SLOT_NS		CONFIG_SLOT_NUM
#define CI_SUPPORT 
//#define SYS_PIN_MUX_MODE_04  

#ifndef _BUILD_OTA_E_
//CI+ begin
//#define CI_PLUS_SUPPORT
#ifdef _MHEG5_V20_ENABLE_
// for generic browser test
#define DATABRODCAST_SERVICE_TO_TV
#endif
#endif

#ifdef CI_PLUS_SUPPORT
    /*
     *  @The following MACRO should be enabled defaultly
     */
    #define CI_PLUS_PVR_SUPPORT             /* @PVR support */
    #define DSC_SUPPORT                     /* @Crypto support */
    #define CI_PLUS_SERVICESHUNNING         /* @Service shunning functionality */
    #define NETWORK_ID_ENABLE               /* @Host tune and CAM upgrade */ 

    #define CI_PLUS_TEST_CAM_USED           /* @Can't disable */
    #define CI_PLUS_CLOSE_YPBPR_ICTMVCGMS   /* @Close YUV when ICT / APS */

    #define CI_PLUS_URI_RESPONSE_REALTIME   /* @Response when modify configuration in menu */

    /*
     *  @The following MACRO could be modified by board configuration
     */
    //CI+ need disable Serial output
    //#define DISABLE_PRINTF                /* @Could be enabled for debug */

    //GXX no Macrovision license
    //#define CI_NO_MACROVISION_LICENSE     /* @Should be enable if no macrovision license */

    //if support CGMA verify, should open this define to close RGB output
    #define SCART_RGB_UNSUPPORT             /* @Ali demo must close RGB output */

    //#define CI_PLUS_NO_YPBPR              /* @Could be enable when no YUV available */
    
    //New lib used
    #ifdef _CIPLUS_CC_ENABLE_
        #define CI_PLUS_NEW_CC_LIB
    #endif

    /*
     *  @The following MACRO is retained by history reasons
     */
    //#define CI_TEST
    //#define CI_PLUS_TEST_CASE

    //#define CI_WRITE_AES_DES_DATA
    //#define CI_PLUS_NO_SINGLE_SLOT
    //#define CI_PLUS_TEST_MV_OR_CGMS
    //#define CI_PLUS_TEST_ONLY_CGMS    
    //#define CI_PLUS_PRODUCT_CAM
    //#define CI_SEND_CAPMT_OK_TO_APP

    //for some CI+ CAM's CIS has no CI+ compatible info
    //#define CI_PLUS_NO_COMPATIBLE_INFO   

    #ifdef CI_PLUS_NO_YPBPR
        #undef CI_PLUS_CLOSE_YPBPR_ICTMVCGMS
    #endif
#endif

//CI+ end

#endif


#ifndef _BUILD_OTA_E_
#define DVR_PVR_SUPPORT 
#define USB_MP_SUPPORT
#define RECORD_SUPPORT 
#define USB_SAFELY_REMOVE_SUPPORT
#define USB_UPGRADE_SUPPORT_LONG_FILENAME
#define DVR_PVR_SUPPORT_SUBTITLE    1
#else
	#undef USB_MP_SUPPORT
#endif

#define SYS_OS_TASK_NUM			48
#define SYS_OS_SEMA_NUM			64
#define SYS_OS_FLAG_NUM			128
#define SYS_OS_MSBF_NUM			32
#define SYS_OS_MUTX_NUM 		512

#ifdef HDTV_SUPPORT

//#define H264_SUPPORT_MULTI_BANK
#define MAX_EXTRA_FB_NUM 3
#ifndef SD_PVR
    #define MAX_MB_WIDTH 120 //(1920/16)
    #define MAX_MB_HEIGHT 68 //(1088/16)
#else
    #define MAX_MB_WIDTH 46//45 //(720/16)
    #define MAX_MB_HEIGHT 36//36 //(576/16)
#endif

#ifdef H264_SUPPORT_MULTI_BANK
    #ifndef SD_PVR
        #define MAX_MB_STRIDE 120 //120 MB alignment to improve AVC performance
    #else
        #define MAX_MB_STRIDE 46//46 //120 MB alignment to improve AVC performance
    #endif
#define EXTRA_FB_SIZE 0x2000
#define ONE_FB_SIZE (((MAX_MB_STRIDE*MAX_MB_HEIGHT*256*3/2+EXTRA_FB_SIZE-1)&0xffffe000)+EXTRA_FB_SIZE)
#else
    #ifndef SD_PVR
        #define MAX_MB_STRIDE 120 //120 MB alignment to improve AVC performance
    #else
        #define MAX_MB_STRIDE 46//46 //120 MB alignment to improve AVC performance
    #endif
    
    #ifdef SD_PVR 
        #define one_frm_y_size 		(MAX_MB_STRIDE*((MAX_MB_HEIGHT+1)/2)*512)
        #define one_frm_c_size   (MAX_MB_STRIDE*((((MAX_MB_HEIGHT+1)/2)+1)/2)*512)   
        #define ONE_FB_SIZE (one_frm_y_size + one_frm_c_size)
    #else
        #define ONE_FB_SIZE (MAX_MB_STRIDE*MAX_MB_HEIGHT*256*3/2)
    #endif

#endif

#define ONE_DV_FB_SIZE ((MAX_MB_WIDTH*MAX_MB_HEIGHT*256*3/2)/4)
#define ONE_MV_SIZE 64*(MAX_MB_WIDTH*MAX_MB_HEIGHT) //522240

#ifndef SD_PVR
#define AVC_FB_LEN		ONE_FB_SIZE*(4+MAX_EXTRA_FB_NUM) //0x1700000
#define AVC_DVIEW_LEN   ONE_DV_FB_SIZE*(4+MAX_EXTRA_FB_NUM) //0xb00000
#define AVC_MV_LEN		ONE_MV_SIZE*(4+MAX_EXTRA_FB_NUM) //0x37c800//0x2FD000
#else
#define const_frm_num   5
#define AVC_FB_LEN		ONE_FB_SIZE*(const_frm_num+MAX_EXTRA_FB_NUM) //0x1700000
#define AVC_DVIEW_LEN   0//  ONE_DV_FB_SIZE*(4+MAX_EXTRA_FB_NUM) //0xb00000
#define AVC_MV_LEN		ONE_MV_SIZE*(const_frm_num+MAX_EXTRA_FB_NUM) //0x37c800//0x2FD000
#endif

#ifndef SD_PVR
#define AVC_MB_COL_LEN		0x22000 //0x11000
#endif
#define AVC_MB_NEI_LEN		0x8000
#define AVC_CMD_QUEUE_LEN 0x10000 //In allegro test stream, this length could be bigger than 128k, however, in realistic, 0x10000 should be enough
#define AVC_LAF_RW_BUF_LEN ((MAX_MB_WIDTH*MAX_MB_HEIGHT)*32*2*2)
//#define AVC_LAF_FLAG_BUF_LEN (0xc00*21)
#define AVC_LAF_FLAG_BUF_LEN (0xc00*22) //when enable dual output, we need 1 more laf buffer

#ifndef SD_PVR
    #define AVC_VBV_LEN		0x400000 // for CI+, must use 4MB. if this size is set to 0x200000, vbv buffer overflow could happen in 20061007d_0.ts
#endif
#define AVC_MEM_LEN		0x1898400
/*
#define AVC_VBV_ADDR 		(__MM_FB_TOP_ADDR-AVC_VBV_LEN)
#define AVC_FB_ADDR 		(AVC_VBV_ADDR -  AVC_FB_LEN)
#define AVC_DVIEW_ADDR 	(AVC_FB_ADDR -  AVC_DVIEW_LEN)
#define AVC_MV_ADDR 		(AVC_DVIEW_ADDR - AVC_MV_LEN)
#define AVC_MB_COL_ADDR 	(AVC_MV_ADDR - AVC_MB_COL_LEN)
#define AVC_MB_NEI_ADDR 	(AVC_MB_COL_ADDR - AVC_MB_NEI_LEN)
#define AVC_CMD_QUEUE_ADDR (AVC_MB_NEI_ADDR - AVC_CMD_QUEUE_LEN)
#define AVC_LAF_RW_BUF_ADDR   (AVC_CMD_QUEUE_ADDR - AVC_LAF_RW_BUF_LEN)
#define AVC_LAF_FLAG_BUF_ADDR   (AVC_LAF_RW_BUF_ADDR - AVC_LAF_FLAG_BUF_LEN)
*/

#endif

/************************************************************
other ui and ap use macro
**************************************************************/
#if (SYS_MAIN_BOARD != BOARD_SB_S3602F_QFP_01V01 && SYS_MAIN_BOARD != BOARD_DB_M3603_01V01 \
	&& SYS_MAIN_BOARD != BOARD_DB_M3603_02V01&& SYS_MAIN_BOARD != BOARD_DB_M3701C_01V01)
#define SUPPORT_TWO_TUNER	
#endif

#define GLOBAL_MOTOR_ANTENNA_SETTING
#ifndef SUPPORT_TWO_TUNER
	#define SELECT_SAT_ONLY
	#if (defined(CI_SUPPORT) && (SYS_MAIN_BOARD == BOARD_DB_M3602_04V01))
		#define CC_USE_TSG_PLAYER
		#define CI_STREAM_PASS_MATCH_CARD
	#endif
#endif
#define SEARCH_DEFAULT_FTAONLY 0
#define DISK_MANAGER_SUPPORT

#define EPG_MULTI_TP
#define _EPG_MULTI_SERVICE
#define GET_TOTAL_SCH_EPG
#define EPG_FAST_PARSE
#define EPG_NETWORK_PATCH
#define SUPPORT_POP_SUBT
//#define TRANSFER_FORMAT2_SUPPORT  // consistent with bootloader for P2P upgrade
//#define MULTI_CAS

/* OTA data backup */
#define OTA_POWEROFF_SAFE
#define OTA_START_BACKUP_FLAG	0xFF
#define OTA_START_END_FLAG	0xFE
#define BACKUP_START_FLAG	0x55aa5a5a



#define SUPPORT_DUAL_OUTPUT_ONOFF   0
#define DISPLAY_SETTING_SUPPORT

#define AUDIO_DEF_CH_L				0x00
#define AUDIO_DEF_CH_R				0x01	
#define AUDIO_DEF_CH_STEREO			0x02
#define AUDIO_DEF_CH_MONO			0x03

#define DEFAULT_AUDIO_CHANNEL		AUDIO_DEF_CH_STEREO

#define GPIO_MUTE					0x00
#define SCART_MUTE					0x01

#define SYS_MUTE_MODE				GPIO_MUTE
#define MEDIA_PLAYER_VIDEO_SUPPORT

// for video decoder
#define __MM_MPG_DEC_START_ADDR     (__MM_OSD_START_ADDR + __MM_OSD_LEN)
#define __MM_MPG_DEC_LEN            (__MM_VBV_START_ADDR - __MM_MPG_DEC_START_ADDR)
#define SYSTEM_BUF_BLOCK_SIZE		2324
#define SYSTEM_BUF_MAX_BLOCK		4

#define DB_RAM_BACKUP

#define NEW_DEMO_FRAME

#ifdef GLASS_HD_FRAME
#undef NEW_DEMO_FRAME
#endif

//#ifdef _BUILD_OTA_E_
//#undef NEW_DEMO_FRAME
//#endif

#ifdef NEW_DEMO_FRAME
  #ifdef _SYSTEM_DVBC
    #define FE_DVBC_SUPPORT
  #else
    #define FE_DVBS_SUPPORT
  #endif
  #define DB_PIP_SPE_USE
#else
  // for ChChg TSG player
  #define CC_USE_TSG_PLAYER
  // define these 2 macro just for pass compile under NEW_DEMO_FRAME
  #define CI_SEND_CAPMT_OK_TO_APP
//#define CI_SLOT_DYNAMIC_DETECT
#endif

#ifdef NETWORK_SUPPORT
	#ifndef SYS_NETWORK_MODULE
		#define SYS_NETWORK_MODULE		SMSC9220	//NET_ENC28J60
	#endif
	
	#if (SYS_NETWORK_MODULE == NET_ENC28J60)
		#define STO_PROTECT_BY_MUTEX
	#endif
#endif

#if ((SYS_MAIN_BOARD == BOARD_DB_M3602_04V01) && defined(SDIO_SUPPORT) && \
	defined(NETWORK_SUPPORT) && (SYS_NETWORK_MODULE == SMSC9220))
	#error "Network chip SMSC9220 and SD card are share pins, cannot use both of them."
#endif

//#define IDE_SUPPORT

#define MULTI_PARTITION_SUPPORT
#define MULTI_VOLUME_SUPPORT

#define MP_SPECTRUM			//support MP3 spectrum
#define OGG_36
#define OSDLAYER_CONFIG

//#define SUPPORT_DEO_HINT
#define DTG_CHAR  //for spec char display
#define GB2312_SUPPORT //to show CCTV5...

#define SYS_SDRAM_SIZE  128

// Some logic blocks
#if ( SYS_SDRAM_SIZE == 2 )

#elif ( SYS_SDRAM_SIZE == 4 )

#elif (SYS_SDRAM_SIZE == 16)

#elif (SYS_SDRAM_SIZE == 32)

#elif (SYS_SDRAM_SIZE == 64)

#elif (SYS_SDRAM_SIZE == 128)
#ifndef DRAM_SPLIT

// Buffer length definition
#define __MM_FB_LEN_DEVIDED	    0X32A000	//(HD:1920X1152X1.5)
#define __MM_FB_LEN			    0xdd2200	//0XCA8000//0X9B4000//0X26D000//(16*SD>3*HD)
#define __MM_MAF_LEN			0X8C000		//0X3000//((FLAG==4*4*3K) + VALUE ==120*72*32 *2(Y+C)>46*36*32 *2(Y+C) *4 )
#define __MM_DVW_LEN			0

#define __MM_GMA1_LEN			(0x1FA400*4) 	// 1920*1080*4 osd layer1  		
#define __MM_GMA2_LEN			0x1FA400 		//1920*1080  osd layer2
#define __MM_TEMP_BUF_LEN		(1920*1080*4 + 0x100)//0x100 			//1920*1080*4 temp resize buffer
#define __MM_CMD_LEN			0x6DB0 			// command buffer
#define __MM_GE_LEN			    (__MM_GMA1_LEN+__MM_GMA2_LEN+__MM_TEMP_BUF_LEN+__MM_CMD_LEN) //0xBE45B0
#define __MM_OSD1_LEN           (1280*720*4)
#define __MM_OSD2_LEN			(1280*720) 

#ifdef HDOSD_DEO_OUTPUT
#define __MM_OSD_DEO_LEN        0x65400//720*576
#else
#define __MM_OSD_DEO_LEN        0//720*576
#endif

// Oncer Yu 20110728: For Sabbat_dual UI 1280x720 size, so it takes less than 2M!
#define OSD_VSRC_MEM_MAX_SIZE 	0x200000//0x400000	//300*1024

#define __MM_VBV_LEN			0x12C000//(HD = 8*SD > 4*SD)
#define __MM_DMX_SI_LEN			(32*188)//(16*188)

#ifdef SUPPORT_MULTI_SD_VIDEO
	#define EXTRA_VIDEO_NUM 3
	#define __MM_DMX_SI_TOTAL_LEN	(__MM_DMX_SI_LEN*(44-EXTRA_VIDEO_NUM))
#else
	#define __MM_DMX_SI_TOTAL_LEN	(__MM_DMX_SI_LEN*44)
#endif

#define __MM_SI_VBV_OFFSET		__MM_DMX_SI_TOTAL_LEN
#define __MM_DMX_DATA_LEN		(30*188)
#define __MM_DMX_PCR_LEN		(10*188)
#define __MM_DMX_AUDIO_LEN		(256*188)//(32*188)
#define __MM_DMX_VIDEO_LEN		(12*512*188)//(8*512*188)
#ifdef SUPPORT_MULTI_SD_VIDEO
	#define __MM_DMX_EXTRA_VIDEO_LEN (960*188)
	#define __MM_DMX_AVP_LEN		(__MM_DMX_VIDEO_LEN+__MM_DMX_EXTRA_VIDEO_LEN*EXTRA_VIDEO_NUM+__MM_DMX_AUDIO_LEN+__MM_DMX_PCR_LEN)
#else
	#define __MM_DMX_AVP_LEN		(2*(__MM_DMX_VIDEO_LEN+__MM_DMX_AUDIO_LEN+__MM_DMX_PCR_LEN))//(__MM_DMX_VIDEO_LEN+__MM_DMX_AUDIO_LEN+__MM_DMX_PCR_LEN)
#endif
#define __MM_SUB_BS_LEN			0xA000 //0X2800
#ifdef HD_SUBTITLE_SUPPORT
    #define __MM_SUB_PB_LEN			0xA0000 //0x50000//0X19000(1920X340 for hd subt)
#else
    #define __MM_SUB_PB_LEN			0X19000
#endif

// TTX
#define __MM_TTX_BS_LEN			0x5000//0X2800
#if 1//def TTX_SUB_PAGE
#define __MM_TTX_PB_LEN	        0xCB200 //+80*1040 //0XCA800
#define __MM_TTX_SUB_PAGE_LEN   0x14500 //80*1040
#else
#define __MM_TTX_PB_LEN		    0xCA800 //+80*1040 //
#define __MM_TTX_SUB_PAGE_LEN   0 //80*1040
#endif
#if 1//def SUPPORT_PACKET_26
#define __MM_TTX_P26_NATION_LEN 0x61A80 //25*40*400
#define __MM_TTX_P26_DATA_LEN   0x3E8 //25*40
#else
#define __MM_TTX_P26_NATION_LEN 0
#define __MM_TTX_P26_DATA_LEN   0
#endif

#ifdef _MHEG5_V20_ENABLE_ //MHEG5 enable, reserve 8MB
#define __MM_PVR_VOB_BUFFER_LEN	    ((47*1024)*(190*3)+0x1000)
#define MHEG5_MEMORY_ALLOC_REGION_SIZE    0X800000
#else
#define __MM_PVR_VOB_BUFFER_LEN	    ((47*1024)*(250*3)+0x1000)
#endif
//#define __MM_DMX_REC_LEN		    (__MM_DMX_VIDEO_LEN)
#define __MM_DMX_REC_LEN		(__MM_SI_VBV_OFFSET + __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN)


#define __MM_USB_DMA_LEN                0	// 0x10FFFF currently not use
#define __MM_EPG_BUFFER_LEN             0x100000
#define __MM_AUTOSCAN_DB_BUFFER_LEN     0x100000

#ifdef NETWORK_SUPPORT
#define __MM_LWIP_MEM_LEN               0x8000
#define __MM_LWIP_MEMP_LEN              0x5FC00
#else
#define __MM_LWIP_MEM_LEN               0
#define __MM_LWIP_MEMP_LEN              0
#endif


//=========================================================
// Memory mapping scope 128M - 64M
// 1. Network buffer
// 2. VOB cache buffer.
// 3. GE buffer.
// 4. OSD layer1 buffer
// 5. OSD layer2 buffer.
// 6. OSD Virtual screen buffer
//=========================================================
#define __MM_HIGHEST_ADDR  				0xa8000000
#define __MM_VOID_BUFFER_ADDR			0xa7f00000
#define __MM_LWIP_MEM_ADDR              (__MM_VOID_BUFFER_ADDR - __MM_LWIP_MEM_LEN)
#define __MM_LWIP_MEMP_ADDR             (__MM_LWIP_MEM_ADDR - __MM_LWIP_MEMP_LEN)
#define __MM_PVR_VOB_BUFFER_ADDR		(__MM_LWIP_MEMP_ADDR - __MM_PVR_VOB_BUFFER_LEN)

#ifdef _MHEG5_V20_ENABLE_
#define __MM_MHEG5_BUFFER_ADDR      (__MM_PVR_VOB_BUFFER_ADDR - MHEG5_MEMORY_ALLOC_REGION_SIZE)
#define __MM_GE_START_ADDR			    ((__MM_MHEG5_BUFFER_ADDR - __MM_GE_LEN -0x1000)&0XFFFFF000)
#else
#define __MM_GE_START_ADDR			    ((__MM_PVR_VOB_BUFFER_ADDR - __MM_GE_LEN -0x1000)&0XFFFFF000)
#endif

//#define __MM_GE_START_ADDR			    ((__MM_PVR_VOB_BUFFER_ADDR - __MM_GE_LEN -0x1000)&0XFFFFF000)
#define __MM_OSD_BK_ADDR1  				(__MM_GE_START_ADDR -  __MM_OSD1_LEN)
#define __MM_OSD_BK_ADDR2  				(__MM_OSD_BK_ADDR1 - __MM_OSD2_LEN)
#define __MM_OSD_VSRC_MEM_ADDR			(__MM_OSD_BK_ADDR2 - OSD_VSRC_MEM_MAX_SIZE)

#if(__MM_OSD_VSRC_MEM_ADDR < 0xa4000000)
#error "__MM_OSD_VSRC_MEM_ADDR  too small"
#endif
//=========================================================
// Memory mapping scope 64M - 0M
// 1. MPEG frame buffer, shared with AVC frame buffer.
// 2. AVC frame buffer and temp buffer.
// 3. VBV buffer.
// 4. DMX REC buffer.
// 5. TTX SUBT buffer.
// 6. EPG buffer.
// 7. VBV buffer
// 8. Autoscan database buffer.
// 9. Heap.
// 10.Code.
//=========================================================
#define __MM_FB_TOP_ADDR				0xa4000000

#define __MM_FB_START_ADDR				((__MM_FB_TOP_ADDR - __MM_FB_LEN)&0XFFFFFF00)
#define __MM_MAF_START_ADDR				((__MM_FB_START_ADDR - __MM_MAF_LEN)&0XFFFFFC00)
#define AVC_VBV_ADDR 					(__MM_FB_TOP_ADDR-AVC_VBV_LEN)
#define AVC_FB_ADDR 					(AVC_VBV_ADDR -  AVC_FB_LEN)
#define AVC_DVIEW_ADDR 					(AVC_FB_ADDR -  AVC_DVIEW_LEN)
#define AVC_MV_ADDR 					(AVC_DVIEW_ADDR - AVC_MV_LEN)
#define AVC_MB_COL_ADDR 				(AVC_MV_ADDR - AVC_MB_COL_LEN)
#define AVC_MB_NEI_ADDR 				(AVC_MB_COL_ADDR - AVC_MB_NEI_LEN)
#define AVC_CMD_QUEUE_ADDR 				(AVC_MB_NEI_ADDR - AVC_CMD_QUEUE_LEN)
#define AVC_LAF_RW_BUF_ADDR   			(AVC_CMD_QUEUE_ADDR - AVC_LAF_RW_BUF_LEN)
#define AVC_LAF_FLAG_BUF_ADDR   		(AVC_LAF_RW_BUF_ADDR - AVC_LAF_FLAG_BUF_LEN)

#define __MM_FB_BOTTOM_ADDR             AVC_LAF_FLAG_BUF_ADDR

#define __MM_DMX_REC_START_ADDR			(__MM_FB_BOTTOM_ADDR - __MM_DMX_REC_LEN)
#define __MM_TTX_BS_START_ADDR			((__MM_DMX_REC_START_ADDR - __MM_TTX_BS_LEN)&0XFFFFFFFC)
#define __MM_TTX_PB_START_ADDR			((__MM_TTX_BS_START_ADDR - __MM_TTX_PB_LEN)&0XFFFFFFFC)
#define __MM_TTX_SUB_PAGE_BUF_ADDR  	(__MM_TTX_PB_START_ADDR - __MM_TTX_SUB_PAGE_LEN)
#define __MM_TTX_P26_NATION_BUF_ADDR 	(__MM_TTX_SUB_PAGE_BUF_ADDR - __MM_TTX_P26_NATION_LEN)
#define __MM_TTX_P26_DATA_BUF_ADDR  	(__MM_TTX_P26_NATION_BUF_ADDR -  __MM_TTX_P26_DATA_LEN)

#define __MM_SUB_BS_START_ADDR			((__MM_TTX_P26_DATA_BUF_ADDR  - __MM_SUB_BS_LEN)&0XFFFFFFFC)
#define __MM_SUB_PB_START_ADDR			((__MM_SUB_BS_START_ADDR - __MM_SUB_PB_LEN)&0XFFFFFFFC)
#define __MM_USB_START_ADDR				((__MM_SUB_PB_START_ADDR - __MM_USB_DMA_LEN)&0XFFFFFFE0)
#define __MM_EPG_BUFFER_START   		(__MM_USB_START_ADDR-__MM_EPG_BUFFER_LEN)

#define __MM_VBV_START_ADDR				((__MM_EPG_BUFFER_START - __MM_VBV_LEN)&0XFFFFFF00)
#define __MM_DMX_FFT_START_BUFFER        __MM_VBV_START_ADDR
#define __MM_DMX_AVP_START_ADDR			((__MM_VBV_START_ADDR - __MM_SI_VBV_OFFSET - __MM_DMX_DATA_LEN - __MM_DMX_AVP_LEN)&0XFFFFFFFC)
#define __MM_AUTOSCAN_DB_BUFFER_ADDR    ((__MM_DMX_AVP_START_ADDR - __MM_AUTOSCAN_DB_BUFFER_LEN)&0XFFFFFFF0)//((__MM_OSD_BK_ADDR2 - __MM_AUTOSCAN_DB_BUFFER_LEN)&0XFFFFFFF0)

#define __MM_OSD_DEO_ADDR               ((__MM_AUTOSCAN_DB_BUFFER_ADDR - __MM_OSD_DEO_LEN)&0XFFFFFFF0)    

#define __MM_HEAP_TOP_ADDR             	__MM_OSD_DEO_ADDR //0xA7000000 // // //

 
// for jpeg decoder memmap
#define __MM_FB0_Y_LEN			(1920*1088+1024)//(736*576+512)	//for high definition jpg decode
#define __MM_FB1_Y_LEN			__MM_FB0_Y_LEN
#define __MM_FB2_Y_LEN			__MM_FB0_Y_LEN

#define __MM_FB0_C_LEN			(__MM_FB0_Y_LEN/2)
#define __MM_FB1_C_LEN			__MM_FB0_C_LEN
#define __MM_FB2_C_LEN			__MM_FB0_C_LEN

#define __MM_FB3_Y_LEN			(736*576+1024)
#define __MM_FB3_C_LEN			(__MM_FB3_Y_LEN/2)
#define __MM_FB4_Y_LEN			__MM_FB3_Y_LEN
#define __MM_FB4_C_LEN			__MM_FB3_C_LEN
#define __MM_FB5_Y_LEN          __MM_FB3_Y_LEN
#define __MM_FB5_C_LEN          __MM_FB3_C_LEN
#define __MM_FB6_Y_LEN          __MM_FB3_Y_LEN
#define __MM_FB6_C_LEN          __MM_FB3_C_LEN

#define __MM_FB0_Y_START_ADDR   (__MM_FB_START_ADDR)
#define __MM_FB0_C_START_ADDR   (__MM_FB0_Y_START_ADDR+__MM_FB0_Y_LEN)

#define __MM_FB1_Y_START_ADDR   ((__MM_FB0_C_START_ADDR+__MM_FB0_C_LEN)&0XFFFFFE00)
#define __MM_FB1_C_START_ADDR   ((__MM_FB1_Y_START_ADDR+__MM_FB1_Y_LEN)&0XFFFFFE00)

#define __MM_FB2_Y_START_ADDR   ((__MM_FB1_C_START_ADDR+__MM_FB1_C_LEN)&0XFFFFFE00)
#define __MM_FB2_C_START_ADDR   ((__MM_FB2_Y_START_ADDR+__MM_FB2_Y_LEN)&0XFFFFFE00)

#define __MM_FB3_Y_START_ADDR   ((__MM_FB2_C_START_ADDR+__MM_FB2_C_LEN)&0XFFFFFE00)
#define __MM_FB3_C_START_ADDR   ((__MM_FB3_Y_START_ADDR+__MM_FB3_Y_LEN)&0XFFFFFE00)
#define __MM_FB4_Y_START_ADDR	((__MM_FB3_C_START_ADDR+__MM_FB3_C_LEN)&0XFFFFFE00)
#define __MM_FB4_C_START_ADDR   ((__MM_FB4_Y_START_ADDR+__MM_FB4_Y_LEN)&0XFFFFFE00)
#define __MM_FB5_Y_START_ADDR	((__MM_FB4_C_START_ADDR+__MM_FB4_C_LEN)&0XFFFFFE00)
#define __MM_FB5_C_START_ADDR   ((__MM_FB5_Y_START_ADDR+__MM_FB5_Y_LEN)&0XFFFFFE00)
#define __MM_FB6_Y_START_ADDR	((__MM_FB5_C_START_ADDR+__MM_FB5_C_LEN)&0XFFFFFE00)
#define __MM_FB6_C_START_ADDR   ((__MM_FB6_Y_START_ADDR+__MM_FB6_Y_LEN)&0XFFFFFE00)

// for satcodx memmap
#define __MM_DVW_START_ADDR			((__MM_FB2_C_START_ADDR - 0X25FA0)&0XFFFFFFF0)
//#endif

#ifdef USB_MP_SUPPORT
#define __MM_MP_BUFFER_LEN			__MM_PVR_VOB_BUFFER_LEN
#define __MM_MP_BUFFER_ADDR			__MM_PVR_VOB_BUFFER_ADDR
#endif

#else //#ifndef DRAM_SPLIT

/* memory mapping

== limitations
1. AVC and MPEG2 decoder both use about 30MB memory, MPEG2 requires one 32MB segment,
	so we put decoder buffer between [64MB, 96MB], start from 96MB and go down.
2. DMX dma buffer should under same 8M segment.
	__MM_DMX_AVP_START_ADDR, and __MM_DMX_REC_START_ADDR, must under same 8M segment, 

	[126,128]  2MB -- cpu memory -- void memory, left to customer
    [96, 126] 30MB -- see memory -- private memory
	[64 , 96] 32MB -- cpu memory -- Video Decoder buffer
	[30 , 64] 34MB -- cpu memory -- PVR VOB buffer.
	[15 , 30] 15MB -- cpu memory -- Device Buffer: DMX, GE, OSD, VCAP, USB, EPG, DB, LWIP, etc.
	[0	, 15] 15MB -- cpu memory -- main code + heap

*/
#ifdef DVR_PVR_SUPPORT
#ifdef _MHEG5_V20_ENABLE_ //MHEG5 enable, reserve 8MB
	#define __MM_PVR_VOB_BUFFER_LEN	    ((47*1024)*(180*3)+0x1000)
    #define MHEG5_MEMORY_ALLOC_REGION_SIZE	    0X800000
#else
	#ifdef SD_PVR
		#define __MM_PVR_VOB_BUFFER_LEN	    ((47*1024)*(93*2)+0x1000)//0x88a800//0x6fb000
	#else
    	#define __MM_PVR_VOB_BUFFER_LEN	     		((47*1024)*(240*3)+0x1000)
	#endif
#endif
#else
    #define __MM_PVR_VOB_BUFFER_LEN	    ((47*1024)*(80+75 * 2)+0x1000)
	#define MHEG5_MEMORY_ALLOC_REGION_SIZE 0
#endif    

#define __MM_SHARED_MEM_LEN  	256
#ifdef HW_SECURE_ENABLE
	#define __MM_SEE_MP_BUF_LEN  	0x00900000	//9M
	#ifdef SD_PVR
		#define __MM_PRIVATE_SHARE_LEN	0x01A00000//0x01e00000	//26M
	#else
		#define __MM_PRIVATE_SHARE_LEN	0x01e00000	//30M
	#endif
	#define __MM_VOID_BUFFER_LEN	0x00200000	//2M
#else
	#define __MM_SEE_MP_BUF_LEN  	0
	#define __MM_PRIVATE_SHARE_LEN	0x01e00000	//30M
	#define __MM_VOID_BUFFER_LEN	0x00200000	//2M
#endif
#define __MM_PRIVATE_LEN		(__MM_PRIVATE_SHARE_LEN-__MM_SHARED_MEM_LEN)

#ifdef SD_PVR
	#define __MM_HIGHEST_ADDR   0xa4000000		//64M
#else
	#define __MM_HIGHEST_ADDR   0xa8000000		//128M
#endif
#define __MM_VOID_BUFFER_ADDR	(__MM_HIGHEST_ADDR - __MM_VOID_BUFFER_LEN)	//126M or 114M
#define __MM_SHARE_BASE_ADDR 	(__MM_VOID_BUFFER_ADDR-__MM_SHARED_MEM_LEN)
#define __MM_PRIVATE_TOP_ADDR 	(__MM_SHARE_BASE_ADDR)
#define __MM_PRIVATE_ADDR		(__MM_PRIVATE_TOP_ADDR-__MM_PRIVATE_LEN) 	//96M
#ifdef SD_PVR
	#define __MM_MAIN_TOP_ADDR		0xa2400000//0xa2000000		//36MB
	
	#ifdef _M3383_SABBAT_
	#define __MM_GE_START_ADDR			((__MM_MAIN_TOP_ADDR - __MM_GE_LEN)&0XFFFFFFE0)
	#define __MM_OSD_BK_ADDR1  			((__MM_GE_START_ADDR - __MM_OSD1_LEN)&0XFFFFFFF0)
	#define __MM_VCAP_FB_ADDR           ((__MM_OSD_BK_ADDR1 - __MM_VCAP_FB_SIZE)&0XFFFFFF00) // 256 bytes alignment
	#endif
	
	#define __MM_OSD_VSRC_MEM_ADDR	(0xa2200000 - OSD_VSRC_MEM_MAX_SIZE)
    #define __MM_FB_TOP_ADDR		__MM_OSD_VSRC_MEM_ADDR		//32MB
#else
	#define __MM_FB_TOP_ADDR		0xa6000000		//96MB
#endif

#ifdef SD_PVR
	#if(__MM_MAIN_TOP_ADDR != __MM_PRIVATE_ADDR)
	#error "__MM_PRIVATE_ADDR address error"
	#endif
#else
	#if(__MM_FB_TOP_ADDR != __MM_PRIVATE_ADDR)
	#error "__MM_PRIVATE_ADDR address error"
	#endif
#endif

#define __MM_FB_LEN_DEVIDED	    0X32A000//(HD:1920X1152X1.5)
#ifndef SD_PVR
    #ifdef VIDEO_DEBLOCKING_SUPPORT
    #define __MM_FB_LEN			    0x10CF200//0x19c6200
    #else
    #define __MM_FB_LEN			    0xdd2200//0XCA8000//0X9B4000//0X26D000//(16*SD>3*HD)
    #endif
    #define __MM_MAF_LEN			0x198C00//0xd0000//0X3000//((FLAG==4*4*3K) + VALUE ==120*72*32 *2(Y+C)>46*36*32 *2(Y+C) *4 )
#else
    #ifdef VIDEO_DEBLOCKING_SUPPORT
    #define __MM_FB_LEN			    0x61BC00//0x10CF200//0x19c6200
    #else
    #define __MM_FB_LEN			    0x308400//0xdd2200//0XCA8000//0X9B4000//0X26D000//(16*SD>3*HD)
    #endif
    #define __MM_MAF_LEN			0X8C000//0x198C00//0xd0000//0X3000//((FLAG==4*4*3K) + VALUE ==120*72*32 *2(Y+C)>46*36*32 *2(Y+C) *4 )
#endif
#define __MM_DVW_LEN			0

//#define SD_PVR //to support 64M with SD pvr


//#define __MM_FB_START_ADDR		((AVC_VBV_ADDR - __MM_FB_LEN)&0XFFFFFF00)
#define __MM_FB_START_ADDR		((__MM_FB_TOP_ADDR - __MM_FB_LEN)&0XFFFFFF00)

// for satcodx memmap
#define __MM_DVW_START_ADDR			((__MM_FB2_C_START_ADDR - 0X25FA0)&0XFFFFFFF0)

#define MAX_EXTRA_FB_NUM 3
#ifndef SD_PVR
    #define MAX_MB_WIDTH 120 //(1920/16)
    #define MAX_MB_HEIGHT 68 //(1088/16)
#else
    #define MAX_MB_WIDTH 46//45 //(720/16)
    #define MAX_MB_HEIGHT 36//36 //(576/16)
#endif

#ifdef H264_SUPPORT_MULTI_BANK
    #ifndef SD_PVR
        #define MAX_MB_STRIDE 120 //120 MB alignment to improve AVC performance
    #else
        #define MAX_MB_STRIDE 46//46 //120 MB alignment to improve AVC performance
    #endif
#define EXTRA_FB_SIZE 0x2000
#define ONE_FB_SIZE (((MAX_MB_STRIDE*MAX_MB_HEIGHT*256*3/2+EXTRA_FB_SIZE-1)&0xffffe000)+EXTRA_FB_SIZE)
#else
    #ifndef SD_PVR
        #define MAX_MB_STRIDE 120 //120 MB alignment to improve AVC performance
    #else
        #define MAX_MB_STRIDE 46//46 //120 MB alignment to improve AVC performance
    #endif
    
    #ifdef SD_PVR 
        #define one_frm_y_size 		(MAX_MB_STRIDE*((MAX_MB_HEIGHT+1)/2)*512)
        #define one_frm_c_size   (MAX_MB_STRIDE*((((MAX_MB_HEIGHT+1)/2)+1)/2)*512)   
        #define ONE_FB_SIZE (one_frm_y_size + one_frm_c_size)
    #else
        #define ONE_FB_SIZE (MAX_MB_STRIDE*MAX_MB_HEIGHT*256*3/2)
    #endif

#endif

#define ONE_DV_FB_SIZE ((MAX_MB_WIDTH*MAX_MB_HEIGHT*256*3/2)/4)
#define ONE_MV_SIZE 64*(MAX_MB_WIDTH*MAX_MB_HEIGHT) //522240

#ifndef SD_PVR
#define AVC_FB_LEN		ONE_FB_SIZE*(4+MAX_EXTRA_FB_NUM) //0x1700000
#define AVC_DVIEW_LEN   ONE_DV_FB_SIZE*(4+MAX_EXTRA_FB_NUM) //0xb00000
#define AVC_MV_LEN		ONE_MV_SIZE*(4+MAX_EXTRA_FB_NUM) //0x37c800//0x2FD000
#define AVC_MB_COL_LEN  0x22000
#else
#define const_frm_num   5
#define AVC_FB_LEN		ONE_FB_SIZE*(const_frm_num+MAX_EXTRA_FB_NUM) //0x1700000
#define AVC_DVIEW_LEN   0//  ONE_DV_FB_SIZE*(4+MAX_EXTRA_FB_NUM) //0xb00000
#define AVC_MV_LEN		ONE_MV_SIZE*(const_frm_num+MAX_EXTRA_FB_NUM) //0x37c800//0x2FD000
#define AVC_MB_COL_LEN	0x11000
#endif

#define AVC_MB_NEI_LEN		0x8000
#define AVC_CMD_QUEUE_LEN   0x10000 //In allegro test stream, this length could be bigger than 128k, however, in realistic, 0x10000 should be enough
#undef  AVC_LAF_RW_BUF_LEN
//#define AVC_LAF_RW_BUF_LEN ((MAX_MB_WIDTH*MAX_MB_HEIGHT)*32*2*2)
#define AVC_LAF_RW_BUF_LEN (((((MAX_MB_WIDTH*MAX_MB_HEIGHT)*48*2)+1023)&0x0ffffc00)*2)

//#define AVC_LAF_FLAG_BUF_LEN (0xc00*21)
#define AVC_LAF_FLAG_BUF_LEN (0xc00*22) //when enable dual output, we need 1 more laf buffer

#ifndef SD_PVR
    #define AVC_VBV_LEN		0x400000 //for CI+, must use 4MB. if this size is set to 0x200000, vbv buffer overflow could happen in 20061007d_0.ts
#else
    #define AVC_VBV_LEN		0x180000 
#endif
#define AVC_FB_ADDR 		((__MM_FB_TOP_ADDR -  AVC_FB_LEN )&0xfffffe00)   		//512 bytes alignment
#define AVC_DVIEW_ADDR 		((AVC_FB_ADDR -  AVC_DVIEW_LEN)&0xfffffe00) 	//512 bytes alignment
#define AVC_MV_ADDR 		((AVC_DVIEW_ADDR - AVC_MV_LEN)&0xffffff00)  		//256 bytes alignment
#define AVC_MB_COL_ADDR 	((AVC_MV_ADDR - AVC_MB_COL_LEN)&0xffffff00) 		//256 bytes alignment
#define AVC_MB_NEI_ADDR 	((AVC_MB_COL_ADDR - AVC_MB_NEI_LEN)&0xffffff00) 	//256 bytes alignment
#define AVC_LAF_RW_BUF_ADDR ((AVC_MB_NEI_ADDR - AVC_LAF_RW_BUF_LEN)&0xfffffc00)  //1024 bytes alignment

#if 1 //ifdef FPGA_TEST
#define __MM_GMA1_LEN			0 // 0x1FA400 // 1920*1080 osd layer1  		
#define __MM_GMA2_LEN			0 // 0x1FA400//1920*1080  osd layer2
#define __MM_TEMP_BUF_LEN		0 // 0x100 //1920*1080*4 temp resize buffer
#define __MM_CMD_LEN			0 // 0x6DB0 // command buffer
#else
#define __MM_GMA1_LEN			0x1FA400 // 1920*1080 osd layer1  		
#define __MM_GMA2_LEN			0x1FA400//1920*1080  osd layer2
#define __MM_TEMP_BUF_LEN		0x100 //1920*1080*4 temp resize buffer
#define __MM_CMD_LEN			0x6DB0 // command buffer
#endif
#define __MM_GE_LEN			    (__MM_GMA1_LEN+__MM_GMA2_LEN+__MM_TEMP_BUF_LEN+__MM_CMD_LEN) //0xBE45B0
#define __MM_OSD_LEN			0x65400 // 720*576
#ifndef SD_PVR
#define __MM_OSD1_LEN           (1280*720*4)
#define __MM_OSD2_LEN			(1920*1080) 
#else
#define __MM_OSD1_LEN           (720*576*4)
#define __MM_OSD2_LEN			(720*576) 
#endif

#define OSD_VSRC_MEM_MAX_SIZE 	0x400000	//300*1024

#ifndef SD_PVR
#define __MM_VBV_LEN			0x200000
#else
#define __MM_VBV_LEN			0x12C000//(HD = 8*SD > 4*SD)
#endif

#define __MM_DMX_SI_LEN			(32*188)//(16*188)
#ifdef SUPPORT_MULTI_SD_VIDEO
	#define EXTRA_VIDEO_NUM 3
	#define __MM_DMX_SI_TOTAL_LEN	(__MM_DMX_SI_LEN*(44-EXTRA_VIDEO_NUM))
#else
	#define __MM_DMX_SI_TOTAL_LEN	(__MM_DMX_SI_LEN*44)
#endif

#define __MM_SI_VBV_OFFSET		__MM_DMX_SI_TOTAL_LEN
#define __MM_DMX_DATA_LEN		(30*188)
#define __MM_DMX_PCR_LEN		(10*188)
#define __MM_DMX_AUDIO_LEN		(256*188)//(32*188)
#define __MM_DMX_VIDEO_LEN		(12*512*188)//(8*512*188)
#ifdef SUPPORT_MULTI_SD_VIDEO
	#define __MM_DMX_EXTRA_VIDEO_LEN (960*188)
	#define __MM_DMX_AVP_LEN		(__MM_DMX_VIDEO_LEN+__MM_DMX_EXTRA_VIDEO_LEN*EXTRA_VIDEO_NUM+__MM_DMX_AUDIO_LEN+__MM_DMX_PCR_LEN)
#else
	#define __MM_DMX_AVP_LEN		(__MM_DMX_VIDEO_LEN+__MM_DMX_AUDIO_LEN+__MM_DMX_PCR_LEN)
#endif
#define __MM_DMX_BLK_BUF_LEN    0xbc000

#define __MM_SUB_BS_LEN			0xA000 //0X2800

#ifdef HD_SUBTITLE_SUPPORT
    #define __MM_SUB_PB_LEN			0xA0000 //0x50000//0X19000(1920X340 for hd subt)
#else
    #define __MM_SUB_PB_LEN			0X19000
#endif

#ifdef SUPPORT_HW_SUBT_DECODE
#define __MM_SUB_HW_DATA_LEN 0xC000
#else
#define __MM_SUB_HW_DATA_LEN 0
#endif

// TTX
#define __MM_TTX_BS_LEN			0x5000//0X2800
#ifdef TTX_SUB_PAGE
#define __MM_TTX_PB_LEN	            0xCB200 //+80*1040 //0XCA800
#define __MM_TTX_SUB_PAGE_LEN       0x14500 //80*1040
#else
#define __MM_TTX_PB_LEN		        0xCA800 //+80*1040 //
#define __MM_TTX_SUB_PAGE_LEN       0 //80*1040
#endif
#ifdef SUPPORT_PACKET_26
#define __MM_TTX_P26_NATION_LEN     0x61A80 //25*40*400
#define __MM_TTX_P26_DATA_LEN       0x3E8 //25*40
#else
#define __MM_TTX_P26_NATION_LEN     0
#define __MM_TTX_P26_DATA_LEN       0
#endif


#define __MM_DMX_REC_LEN		    (__MM_DMX_VIDEO_LEN)
    
#define __MM_USB_DMA_LEN                0	// 0x10FFFF currently not use
#define __MM_EPG_BUFFER_LEN             0x100000
#define __MM_AUTOSCAN_DB_BUFFER_LEN     0x100000

#ifdef NETWORK_SUPPORT
#define STO_PROTECT_BY_MUTEX
#define __MM_LWIP_MEM_LEN               0x8000
#define __MM_LWIP_MEMP_LEN              0x5FC00
#define __MM_XML_MEMP_LEN				0x500000 // 5M
#else
#define __MM_LWIP_MEM_LEN               0
#define __MM_LWIP_MEMP_LEN              0
#define __MM_XML_MEMP_LEN				0
#endif

#ifdef DUAL_VIDEO_OUTPUT_USE_VCAP
#define __MM_VCAP_FB_SIZE               (736*576*2*4)
#else
#define __MM_VCAP_FB_SIZE               0
#endif
#define __MM_DBG_MEM_LEN   0x4000

#if (__MM_FB_START_ADDR < AVC_LAF_RW_BUF_ADDR)
#define __MM_FB_BOTTOM_ADDR         __MM_FB_START_ADDR
#else
#define __MM_FB_BOTTOM_ADDR         AVC_LAF_RW_BUF_ADDR
#endif

#ifdef _M3383_SABBAT_

#define __MM_DMX_REC_START_ADDR		(__MM_FB_BOTTOM_ADDR - __MM_DMX_REC_LEN)
#define __MM_DMX_CPU_BLK_ADDR		((__MM_DMX_REC_START_ADDR - __MM_DMX_BLK_BUF_LEN)&0XFFFFFFE0)
#define __MM_BUF_PVR_TOP_ADDR       __MM_DMX_CPU_BLK_ADDR
#define __MM_PVR_VOB_BUFFER_ADDR	(__MM_BUF_PVR_TOP_ADDR - __MM_PVR_VOB_BUFFER_LEN)

#ifdef _MHEG5_V20_ENABLE_
#define __MM_MHEG5_BUFFER_ADDR      (__MM_PVR_VOB_BUFFER_ADDR - MHEG5_MEMORY_ALLOC_REGION_SIZE)
#define __MM_DMX_AVP_START_ADDR		((__MM_MHEG5_BUFFER_ADDR - __MM_SI_VBV_OFFSET - __MM_DMX_DATA_LEN - __MM_DMX_AVP_LEN)&0XFFFFFFFC)
#else
#define __MM_DMX_AVP_START_ADDR		((__MM_PVR_VOB_BUFFER_ADDR - __MM_SI_VBV_OFFSET - __MM_DMX_DATA_LEN - __MM_DMX_AVP_LEN)&0XFFFFFFFC)
#endif

#define __MM_EPG_BUFFER_START   	(__MM_DMX_AVP_START_ADDR - __MM_EPG_BUFFER_LEN)
#define __MM_LWIP_MEM_ADDR          (__MM_EPG_BUFFER_START - __MM_LWIP_MEM_LEN)

#else /* #ifdef _M3383_SABBAT*/

#define __MM_BUF_PVR_TOP_ADDR       __MM_FB_BOTTOM_ADDR
#define __MM_PVR_VOB_BUFFER_ADDR	(__MM_BUF_PVR_TOP_ADDR - __MM_PVR_VOB_BUFFER_LEN)

#ifdef _MHEG5_V20_ENABLE_
#define __MM_MHEG5_BUFFER_ADDR      (__MM_PVR_VOB_BUFFER_ADDR - MHEG5_MEMORY_ALLOC_REGION_SIZE)
#define __MM_DMX_AVP_START_ADDR		((__MM_MHEG5_BUFFER_ADDR - __MM_SI_VBV_OFFSET - __MM_DMX_DATA_LEN - __MM_DMX_AVP_LEN)&0XFFFFFFFC)
#else
#define __MM_DMX_AVP_START_ADDR		((__MM_PVR_VOB_BUFFER_ADDR - __MM_SI_VBV_OFFSET - __MM_DMX_DATA_LEN - __MM_DMX_AVP_LEN)&0XFFFFFFFC)
#endif

#define __MM_DMX_REC_START_ADDR		(__MM_DMX_AVP_START_ADDR - __MM_DMX_REC_LEN)
#define __MM_DMX_CPU_BLK_ADDR		((__MM_DMX_REC_START_ADDR - __MM_DMX_BLK_BUF_LEN)&0XFFFFFFE0)
#define __MM_EPG_BUFFER_START   	(__MM_DMX_CPU_BLK_ADDR - __MM_EPG_BUFFER_LEN)

#define __MM_GE_START_ADDR			((__MM_EPG_BUFFER_START - __MM_GE_LEN)&0XFFFFFFE0)
#ifdef _M36F_SINGLE_CPU
// for m36f single cpu mode(only test for ali internal)
// move osd, vsrc, scap buffer to private buffer base.
#define __MM_LWIP_MEM_ADDR          (__MM_GE_START_ADDR - __MM_LWIP_MEM_LEN)
#else
#define __MM_OSD_BK_ADDR1  			((__MM_GE_START_ADDR - __MM_OSD1_LEN)&0XFFFFFFF0)
#define __MM_VCAP_FB_ADDR           ((__MM_OSD_BK_ADDR1 - __MM_VCAP_FB_SIZE)&0XFFFFFF00) // 256 bytes alignment
    #ifndef SD_PVR
    #define __MM_OSD_VSRC_MEM_ADDR		(__MM_VCAP_FB_ADDR - OSD_VSRC_MEM_MAX_SIZE)
    #define __MM_LWIP_MEM_ADDR          (__MM_OSD_VSRC_MEM_ADDR - __MM_LWIP_MEM_LEN)
    #else
    #define __MM_LWIP_MEM_ADDR          (__MM_VCAP_FB_ADDR - __MM_LWIP_MEM_LEN)
    #endif
#endif

#endif /* #ifdef _M3383_SABBAT*/

#define __MM_LWIP_MEMP_ADDR         (__MM_LWIP_MEM_ADDR - __MM_LWIP_MEMP_LEN)
#define __MM_USB_START_ADDR			((__MM_LWIP_MEMP_ADDR - __MM_USB_DMA_LEN)&0XFFFFFFE0)
#define __MM_DCII_SUB_BS_START_ADDR (__MM_USB_START_ADDR - __MM_DCII_SUB_BS_LEN)
#define __MM_CPU_DBG_MEM_ADDR      	(__MM_DCII_SUB_BS_START_ADDR - __MM_DBG_MEM_LEN)

#define __MM_AUTOSCAN_DB_BUFFER_ADDR    __MM_PVR_VOB_BUFFER_ADDR

//end of main mem map

//Private mem map
/* 
    In HW security mode, some see buffer must be shared by media player, 
    so put them together on the top:
    VBV, DMX, TTX
*/
#define __MM_VBV_START_ADDR			((__MM_PRIVATE_TOP_ADDR - __MM_VBV_LEN)&0XFFFFFF00)
#define __MM_MAF_START_ADDR			((__MM_VBV_START_ADDR - __MM_MAF_LEN)&0XFFFFFC00)
#define AVC_VBV_ADDR 		        (__MM_PRIVATE_TOP_ADDR - AVC_VBV_LEN) 	//256 bytes alignment

#if (AVC_VBV_ADDR <= __MM_MAF_START_ADDR)
#define __MM_DMX_SEE_BLK_ADDR	((AVC_VBV_ADDR - __MM_DMX_BLK_BUF_LEN)&0XFFFFFFE0)
#else
#define __MM_DMX_SEE_BLK_ADDR	((__MM_MAF_START_ADDR - __MM_DMX_BLK_BUF_LEN)&0XFFFFFFE0)
#endif

#define __MM_TTX_SUB_PAGE_BUF_ADDR  (__MM_DMX_SEE_BLK_ADDR - __MM_TTX_SUB_PAGE_LEN)
#define __MM_TTX_P26_NATION_BUF_ADDR (__MM_TTX_SUB_PAGE_BUF_ADDR - __MM_TTX_P26_NATION_LEN)
#define __MM_TTX_P26_DATA_BUF_ADDR  (__MM_TTX_P26_NATION_BUF_ADDR -  __MM_TTX_P26_DATA_LEN)
#define __MM_TTX_BS_START_ADDR	((__MM_TTX_P26_DATA_BUF_ADDR - __MM_TTX_BS_LEN)&0XFFFFFFFC)
#define __MM_TTX_PB_START_ADDR	((__MM_TTX_BS_START_ADDR - __MM_TTX_PB_LEN)&0XFFFFFFFC)
#define __MM_SEE_MP_BUF_ADDR    (__MM_TTX_PB_START_ADDR -  __MM_SEE_MP_BUF_LEN)

#define AVC_CMD_QUEUE_ADDR  ((__MM_SEE_MP_BUF_ADDR - AVC_CMD_QUEUE_LEN)&0xffffff00)  	//256 bytes alignment
#define AVC_LAF_FLAG_BUF_ADDR   ((AVC_CMD_QUEUE_ADDR - AVC_LAF_FLAG_BUF_LEN)&0xfffffc00)  //1024 bytes alignment
#define __MM_OSD_BK_ADDR2       ((AVC_LAF_FLAG_BUF_ADDR - __MM_OSD2_LEN)&0XFFFFFFF0)

#define __MM_SUB_BS_START_ADDR	((__MM_OSD_BK_ADDR2  - __MM_SUB_BS_LEN)&0XFFFFFFFC)
#define __MM_SUB_HW_DATA_ADDR ((__MM_SUB_BS_START_ADDR - __MM_SUB_HW_DATA_LEN)&0XFFFFFFF0)
#define __MM_SUB_PB_START_ADDR	((__MM_SUB_HW_DATA_ADDR - __MM_SUB_PB_LEN)&0XFFFFFFFC)

#define __MM_SUBT_ATSC_SEC_ADDR (__MM_SUB_PB_START_ADDR - __MM_SUBT_ATSC_SEC_LEN)
#ifdef _M36F_SINGLE_CPU
// for m3f single cpu(only test for ali internal)
// more osd, vsrc, scap buffer to private buffer base.
#define __MM_OSD_BK_ADDR1  			((__MM_SUB_PB_START_ADDR - __MM_OSD1_LEN)&0XFFFFFFF0)
#define __MM_VCAP_FB_ADDR           ((__MM_OSD_BK_ADDR1 - __MM_VCAP_FB_SIZE)&0XFFFFFF00) // 256 bytes alignment
#define __MM_OSD_VSRC_MEM_ADDR		(__MM_VCAP_FB_ADDR - OSD_VSRC_MEM_MAX_SIZE)
#define __MM_SEE_DBG_MEM_ADDR      (__MM_OSD_VSRC_MEM_ADDR - __MM_DBG_MEM_LEN)
	#if(__MM_SEE_DBG_MEM_ADDR < __MM_PRIVATE_ADDR)
		#error "__MM_SEE_DBG_MEM_ADDR address error"
	#endif
#else
#define __MM_SEE_DBG_MEM_ADDR      ( __MM_SUBT_ATSC_SEC_ADDR - __MM_DBG_MEM_LEN)
#endif

// for jpeg decoder memmap
#ifdef SD_PVR
#define __MM_FB0_Y_LEN			(736*576 + 1024)
#else
#define __MM_FB0_Y_LEN			(1920*1088+1024)//(736*576+512)	//for high definition jpg decode
#endif
#define __MM_FB1_Y_LEN			__MM_FB0_Y_LEN
#define __MM_FB2_Y_LEN			__MM_FB0_Y_LEN

#define __MM_FB0_C_LEN			(__MM_FB0_Y_LEN/2)
#define __MM_FB1_C_LEN			__MM_FB0_C_LEN
#define __MM_FB2_C_LEN			__MM_FB0_C_LEN

#define __MM_FB3_Y_LEN			(736*576+1024)
#define __MM_FB3_C_LEN			(__MM_FB3_Y_LEN/2)
#define __MM_FB4_Y_LEN			__MM_FB3_Y_LEN
#define __MM_FB4_C_LEN			__MM_FB3_C_LEN
#define __MM_FB5_Y_LEN          __MM_FB3_Y_LEN
#define __MM_FB5_C_LEN          __MM_FB3_C_LEN
#define __MM_FB6_Y_LEN          __MM_FB3_Y_LEN
#define __MM_FB6_C_LEN          __MM_FB3_C_LEN

#ifdef HW_SECURE_ENABLE
	#define __MM_FB0_Y_START_ADDR	(__MM_SEE_MP_BUF_ADDR & 0xFFFFFE00)
	#define __MM_FB0_C_START_ADDR	(__MM_FB0_Y_START_ADDR+__MM_FB0_Y_LEN)
	
	#define __MM_FB1_Y_START_ADDR	((__MM_FB0_C_START_ADDR+__MM_FB0_C_LEN)&0XFFFFFE00)
	#define __MM_FB1_C_START_ADDR	((__MM_FB1_Y_START_ADDR+__MM_FB1_Y_LEN)&0XFFFFFE00)
	
	#define __MM_FB2_Y_START_ADDR	((__MM_FB1_C_START_ADDR+__MM_FB1_C_LEN)&0XFFFFFE00)
	#define __MM_FB2_C_START_ADDR	((__MM_FB2_Y_START_ADDR+__MM_FB2_Y_LEN)&0XFFFFFE00)
	
	#define __MM_FB3_Y_START_ADDR	0 //((__MM_FB2_C_START_ADDR+__MM_FB2_C_LEN)&0XFFFFFE00)
	#define __MM_FB3_C_START_ADDR	0 //((__MM_FB3_Y_START_ADDR+__MM_FB3_Y_LEN)&0XFFFFFE00)
	#define __MM_FB4_Y_START_ADDR	0 //((__MM_FB3_C_START_ADDR+__MM_FB3_C_LEN)&0XFFFFFE00)
	#define __MM_FB4_C_START_ADDR	0 //((__MM_FB4_Y_START_ADDR+__MM_FB4_Y_LEN)&0XFFFFFE00)
	#define __MM_FB5_Y_START_ADDR	0 //((__MM_FB4_C_START_ADDR+__MM_FB4_C_LEN)&0XFFFFFE00)
	#define __MM_FB5_C_START_ADDR	0 //((__MM_FB5_Y_START_ADDR+__MM_FB5_Y_LEN)&0XFFFFFE00)
	#define __MM_FB6_Y_START_ADDR	0 //((__MM_FB5_C_START_ADDR+__MM_FB5_C_LEN)&0XFFFFFE00)
	#define __MM_FB6_C_START_ADDR	0 //((__MM_FB6_Y_START_ADDR+__MM_FB6_Y_LEN)&0XFFFFFE00)
	#define __MM_MP_BUFFER_LEN		(__MM_PRIVATE_TOP_ADDR - __MM_TTX_P26_DATA_BUF_ADDR)//(__MM_SEE_MP_BUF_LEN - __MM_FB0_Y_LEN * 3 - __MM_FB0_C_LEN * 3)
	#define __MM_MP_BUFFER_ADDR		__MM_TTX_P26_DATA_BUF_ADDR//(__MM_FB2_C_START_ADDR + __MM_FB2_C_LEN)
#else
	
	#define __MM_FB0_Y_START_ADDR   (__MM_FB_START_ADDR)
	#define __MM_FB0_C_START_ADDR   (__MM_FB0_Y_START_ADDR+__MM_FB0_Y_LEN)

	#define __MM_FB1_Y_START_ADDR   ((__MM_FB0_C_START_ADDR+__MM_FB0_C_LEN)&0XFFFFFE00)
	#define __MM_FB1_C_START_ADDR   ((__MM_FB1_Y_START_ADDR+__MM_FB1_Y_LEN)&0XFFFFFE00)

	#define __MM_FB2_Y_START_ADDR   ((__MM_FB1_C_START_ADDR+__MM_FB1_C_LEN)&0XFFFFFE00)
	#define __MM_FB2_C_START_ADDR   ((__MM_FB2_Y_START_ADDR+__MM_FB2_Y_LEN)&0XFFFFFE00)

	#define __MM_FB3_Y_START_ADDR   ((__MM_FB2_C_START_ADDR+__MM_FB2_C_LEN)&0XFFFFFE00)
	#define __MM_FB3_C_START_ADDR   ((__MM_FB3_Y_START_ADDR+__MM_FB3_Y_LEN)&0XFFFFFE00)
	#define __MM_FB4_Y_START_ADDR	((__MM_FB3_C_START_ADDR+__MM_FB3_C_LEN)&0XFFFFFE00)
	#define __MM_FB4_C_START_ADDR   ((__MM_FB4_Y_START_ADDR+__MM_FB4_Y_LEN)&0XFFFFFE00)
	#define __MM_FB5_Y_START_ADDR	((__MM_FB4_C_START_ADDR+__MM_FB4_C_LEN)&0XFFFFFE00)
	#define __MM_FB5_C_START_ADDR   ((__MM_FB5_Y_START_ADDR+__MM_FB5_Y_LEN)&0XFFFFFE00)
	#define __MM_FB6_Y_START_ADDR	((__MM_FB5_C_START_ADDR+__MM_FB5_C_LEN)&0XFFFFFE00)
	#define __MM_FB6_C_START_ADDR   ((__MM_FB6_Y_START_ADDR+__MM_FB6_Y_LEN)&0XFFFFFE00)
#ifdef DVR_PVR_SUPPORT
    #define __MM_MP_BUFFER_LEN			__MM_PVR_VOB_BUFFER_LEN
    #define __MM_MP_BUFFER_ADDR			__MM_PVR_VOB_BUFFER_ADDR
#else
    #define __MM_MP_BUFFER_LEN			0x1000000
    #define __MM_MP_BUFFER_ADDR			((__MM_FB2_C_START_ADDR - __MM_MP_BUFFER_LEN)&0XFFFFFFF0)
#endif
#endif

//end of Private mem map

#ifdef SEE_CPU
#define __MM_DBG_MEM_ADDR __MM_SEE_DBG_MEM_ADDR
#else
#define __MM_DBG_MEM_ADDR __MM_CPU_DBG_MEM_ADDR
#endif

#define __MM_HEAP_TOP_ADDR      __MM_DBG_MEM_ADDR

#endif //#ifndef DRAM_SPLIT

#endif //#elif (SYS_SDRAM_SIZE == 128)

#ifdef HW_SECURE_ENABLE
#define __MM_VIDEO_FILE_BUF_ADDR    __MM_SEE_MP_BUF_ADDR
#define __MM_VIDEO_FILE_BUF_LEN     (__MM_PRIVATE_TOP_ADDR - __MM_SEE_MP_BUF_ADDR)
#else
#define __MM_VIDEO_FILE_BUF_ADDR    __MM_MP_BUFFER_ADDR
#define __MM_VIDEO_FILE_BUF_LEN     __MM_MP_BUFFER_LEN
#endif

#if (256 == COLOR_N)	
#define	BIT_PER_PIXEL		8
#define 	OSD_TRANSPARENT_COLOR		0xFF
#define 	OSD_TRANSPARENT_COLOR_BYTE 	0xFF
#define	IF_GLOBAL_ALPHA	FALSE
#elif (16 == COLOR_N)	
#define	BIT_PER_PIXEL		4
#define 	OSD_TRANSPARENT_COLOR		15
#define 	OSD_TRANSPARENT_COLOR_BYTE 	0xFF
#define	IF_GLOBAL_ALPHA	FALSE
#elif(4 == COLOR_N)	
#define	BIT_PER_PIXEL		2
#define 	OSD_TRANSPARENT_COLOR		3
#define 	OSD_TRANSPARENT_COLOR_BYTE 0xFF
#define	IF_GLOBAL_ALPHA	TRUE
#endif

#if		(BIT_PER_PIXEL	== 2)
#define	FACTOR					2					
#elif	(BIT_PER_PIXEL    == 4)
#define	FACTOR					1					
#elif	(BIT_PER_PIXEL     == 8)
#define	FACTOR					0					
#endif


/***********************************************************************
 AP feature support  maro define
*************************************************************************/
#define	LOGO_ID_TYPE 0x02FD0000
#define	LOGO_ID			(LOGO_ID_TYPE | 0x0100)
#define	MENU_LOGO_ID	(LOGO_ID_TYPE | 0x0100)
#define	RADIO_LOGO_ID	(LOGO_ID_TYPE | 0x0100)
#define	MEDIA_LOGO_ID	(LOGO_ID_TYPE | 0x0100)
#define	BOOT_LOGO_ID	(LOGO_ID_TYPE | 0x0100)
#define	MAINCODE_ID		0x01FE0101
#define KEY_STORE_ID		0x21DE0100
#define OTA_LOADER_ID   0x00FF0100
#define SEECODE_ID       0x06F90101
#define DECRYPT_KEY_ID  0x22DD0100
#define DXXX_CHUCK_ID   0x0AF50100
#define HDCPKEY_CHUNK_ID (HDCPKEY_CHUNID_TYPE |0x0100)
#define STBINFO_ID       0x20DF0100
#define	 CADATA_ID		0x05FA0100
#define	 DEFAULT_DB_ID	0x03FC0100
#define USER_DB_ID       0x04FB0100
#define CIPLUSKEY_CHUNK_ID 0x09F60101

/* OTA Setting */
#define OTA_CONTROL_FLOW_ENHANCE
#define SYS_OUI					0x00000
#define SYS_HW_MODEL           0x0000
#define SYS_HW_VERSION	        0x0000
#define SYS_SW_MODEL          0x0000
#define SYS_SW_VERSION        0x0000

#ifdef _CAS9_CA_ENABLE_
#undef SYS_HW_VERSION
#undef SYS_SW_VERSION
#define SYS_HW_VERSION		0x0001
#define SYS_SW_VERSION		0x0001
#endif

#define BINCODE_COMPRESSED
#define OTA_STATIC_SWAP_MEM
//----------------------------------------------------------

#define VIDEO_SUPPORT_EXTRA_DVIEW_WINDOW
#define VDEC27_SUPPORT_COLORBAR
//#define VIDEO_OVERSHOOT_SOLUTION
#define VIDEO_SYNC_ISSUE

#define SYS_TUN_MODULE IX2410 


//#define ANTENNA_INSTALL_BEEP	2

/* If define ANTENNA_INSTALL_BEEP ==1,
	then DVBT_BEE_TONE have effect,
   else if (ANTENNA_INSTALL_BEEP == 2)
   	then AUDIO_SPECIAL_EFFECT have effect */
#define AUDIO_SPECIAL_EFFECT	/* play mpeg audio file*/
#define ASE_EXT
#define DVBT_BEE_TONE	/* continues beep*/
//#define ALI_SDK_API_ENABLE


#define CODEC_I2S (0x0<<1)
#define CODEC_LEFT (0x1<<1)
#define CODEC_RIGHT (0x2<<1)


#define EASY_DM_SWITCH
#define PSI_MONITOR_PAT_SUPPORT

#define KEYBOARD_SUPPORT

/* If "RAM_TMS_TEST" is defined, MUST re-define "RAM_DISK_ADDR" and "RAM_DISK_SIZE".
 *	1. RAM_DISK_ADDR: MUST NOT share with other module.
 *	2. RAM_DISK_SIZE: MUST (>= 14MByte) for SD and (>= 32MByte) for HD.
 */
#ifdef RAM_TMS_TEST
	#define RAM_DISK_ADDR	__MM_PVR_VOB_BUFFER_ADDR
	#define RAM_DISK_SIZE	((__MM_PVR_VOB_BUFFER_LEN / (47*1024) / 3) * (47*1024))
#endif

/***********************************************************
AP and UI  use GPIO configure
************************************************************/

//#define  SYS_MAIN_BOARD	BOARD_DB_M3602_04V01    //BOARD_DB_M3602_02V01    //BOARD_S3602_DEMO BOARD_S3602_SORT BOARD_DB_M3602_02V01

// I2C config same for all M3602 board now
#define I2C_TYPE_NUM            2		/* Use SCB I2C bus only */
#define I2C_FOR_TUNER           I2C_TYPE_SCB0
#define I2C_FOR_HZ6303          I2C_TYPE_SCB0
#define I2C_FOR_S3501           I2C_TYPE_SCB0
#define I2C_FOR_SCART           I2C_TYPE_SCB1
#define I2C_FOR_RFM             I2C_TYPE_GPIO0
#define I2C_FOR_EDDC            I2C_TYPE_GPIO1
#define I2C_FOR_HDCP             I2C_TYPE_GPIO2

#define EXTERNAL_PULL_HIGH		TRUE    // for I2C_gpio.c

// comon function
#define SYS_12V_SWITCH		SYS_FUNC_OFF
//#define SYS_12V_GPIO				60		// ??
#define SYS_LNB_SHORT_DET SYS_FUNC_OFF
#define SYS_12V_SHORT_DET 	SYS_FUNC_OFF
//#define SYS_12V_SHORT_DET_GPIO

#if(SYS_MAIN_BOARD == BOARD_S3602_DEMO || SYS_MAIN_BOARD == BOARD_S3602_SORT)

#elif (SYS_MAIN_BOARD == BOARD_DB_M3602_02V01 || SYS_MAIN_BOARD == BOARD_DB_M3602_04V01)

#define SYS_12V_SWITCH		SYS_FUNC_OFF
//#define SYS_12V_GPIO
#define SYS_LNB_POWER_OFF SYS_FUNC_ON
#define SYS_LNB_POWER_CUT			73

#define SYS_LNB_SHORT_DET SYS_FUNC_OFF
//#define SYS_LNB_SHORT_DET_GPIO	67
#define SYS_12V_SHORT_DET 	SYS_FUNC_OFF
//#define SYS_12V_SHORT_DET_GPIO

#define	FRONT_PANEL_TYPE		1

//add for set DacType
#define	VDAC_USE_CVBS_TYPE		CVBS_1 // CVBS_2
#define	VDAC_USE_RGB_TYPE		RGB_1
#define	VDAC_USE_YUV_TYPE		YUV_1

#define		CVBS_DAC  				DAC3

//#define		SVIDEO_DAC_Y			DAC1
//#define		SVIDEO_DAC_C			DAC0

#define		RGB_DAC_R				DAC0
#define		RGB_DAC_G				DAC2
#define		RGB_DAC_B				DAC1

#define		YUV_DAC_Y				DAC2
#define		YUV_DAC_U				DAC1
#define		YUV_DAC_V				DAC0

// GPIO for SCART
#define     POS_SCART_POWER         70
#define     GPIO_RGB_YUV_SWITCH     68
//#define     POS_SCART_VCR_DETECT	5

#if(SYS_MAIN_BOARD == BOARD_DB_M3602_02V01)

#define FP_STANDBY_GPIO_NUM 25
#define LOCK_LED_POS 	6

#elif(SYS_MAIN_BOARD == BOARD_DB_M3602_04V01)

#define FP_STANDBY_GPIO_NUM 24
#define LOCK_LED_POS 	23

#endif

#define SDIO_GPIO_CS				25
#define SDIO_GPIO_CS_POLOR			0
#define SDIO_GPIO_DETECT			6
#define SDIO_GPIO_LOCK				7
#define ATA2LAN_CS_GPIO_NUM			127
#define GPIO_USB_POWER            	127

#elif(SYS_MAIN_BOARD == BOARD_DB_M3701C_01V01)

/*==========GPIO Configuration==========*/

#undef  SYS_RFM_MODULE
#define SYS_RFM_MODULE  SYS_DEFINE_NULL

#undef  GPIO_RGB_YUV_SWITCH

#undef SYS_I2C_SDA
#undef SYS_I2C_SCL
#define SYS_I2C_SDA		10
#define SYS_I2C_SCL		11
#define SYS_I2C_SDA1		GPIO_NULL//10
#define SYS_I2C_SCL1		GPIO_NULL//11
#define SYS_I2C_SDA2		GPIO_NULL
#define SYS_I2C_SCL2		GPIO_NULL

#ifndef USB_MP_SUPPORT
#undef  MULTI_VOLUME_SUPPORT
#endif

//#undef  DUAL_VIDEO_OUTPUT_USE_VCAP

#define SYS_LNB_POWER_CUT		GPIO_NULL
#define DEMO_RST_GPIO_NUM		42
#define DEMO_RST_GPIO_NUM2       127  // no GPIO2 for nim2 reset

#define SYS_POWER_GPIO_NUM	24 
//#define MUTE_GPIO_NUM			37
#define SYS_MUTE_GPIO_NUM 		37

#define QAM_INT_GPIO_NUM		GPIO_NULL//6 

#define FP_LOCK_GPIO_NUM		89
#define FP_LOCK_GPIO_NUM2		88
#define FP_STANDBY_GPIO_NUM	74
#define FP_CLOCK_GPIO_NUM		60
#define FP_DATA_GPIO_NUM		61
#define FP_CS_GPIO_NUM            	 GPIO_NULL
#define FP_KEY1_GPIO_NUM		57
#define FP_KEY2_GPIO_NUM		GPIO_NULL
#define FP_COM1_GPIO_NUM		59
#define FP_COM2_GPIO_NUM		58
#define FP_COM3_GPIO_NUM		63
#define FP_COM4_GPIO_NUM		62

// GPIO for SCART
//#define POS_SCART_POWER             127
#define POS_SCART_POWER		GPIO_NULL
#define ATA2LAN_CS_GPIO_NUM	GPIO_NULL

#define SDIO_GPIO_CS				127 // not used for 3606
#define SDIO_GPIO_CS_POLOR			127 // not used for 3606
#define SDIO_GPIO_DETECT			100
#define SDIO_GPIO_LOCK				101
#define GPIO_USB_POWER            		127


//=======
#define MUTE_GPIO_POL			0
#define MUTE_GPIO_EXIST			1
#define LAN_PHY_RST_GPIO			7

/*Front panel*/
#define SLOT_SCAN				0
#define SHADOW_SCAN			1
#define FP_SCAN_TYPE			SHADOW_SCAN
//#define STANDBY_LED_POS			74//86
#define  FP_STANDBY_GPIO_NUM	74//86


#ifdef C3041
#undef SYS_LNB_POWER_CUT
#define SYS_LNB_POWER_CUT           	99
#undef DEMO_RST_GPIO_NUM
#define DEMO_RST_GPIO_NUM           	100


#define SYS_BOARD_IS_M3606_01V02
#define POS_SCART_TVSAT_SWITCH 	101
#define POS_SCART_ASPECT			102
#define POS_SCART_TV_FB			103

#undef FP_CS_GPIO_NUM
#define FP_CS_GPIO_NUM           		8
#undef FP_DATA_GPIO_NUM
#define FP_DATA_GPIO_NUM           	9
#undef FP_CLOCK_GPIO_NUM
#define FP_CLOCK_GPIO_NUM           	37
#undef GPIO_USB_POWER
#define GPIO_USB_POWER            		43

#undef SYS_MUTE_GPIO_NUM
#define SYS_MUTE_GPIO_NUM		127
#undef POS_SCART_POWER
#undef GPIO_RGB_YUV_SWITCH
#endif

//#define DDR_POWER_CONTROL_ENABLE // use a GPIO to control the DDR power on/off
#ifdef DDR_POWER_CONTROL_ENABLE
	#define DDR_POWER_CTL_GPIO_POS		37		/* depends on board design */
	#define DDR_POWER_CTL_GPIO_POLAR	1		/* depends on board design */
#else
	#define DDR_POWER_CTL_GPIO_POS		0xffff	/* invalid GPIO */
	#define DDR_POWER_CTL_GPIO_POLAR	0
#endif

#elif (SYS_MAIN_BOARD == BOARD_DB_M3606_01V01)

#undef  SYS_RFM_MODULE
#define SYS_RFM_MODULE  SYS_DEFINE_NULL

#undef  GPIO_RGB_YUV_SWITCH

#undef SYS_I2C_SDA
#undef SYS_I2C_SCL
#define SYS_I2C_SDA					10
#define SYS_I2C_SCL					11


#ifndef USB_MP_SUPPORT
#undef  MULTI_VOLUME_SUPPORT
#endif

//#undef  DUAL_VIDEO_OUTPUT_USE_VCAP

#define SYS_LNB_POWER_CUT           127
#define DEMO_RST_GPIO_NUM           127  // no GPIO for nim reset
#define DEMO_RST_GPIO_NUM2          127  // no GPIO2 for nim2 reset

#ifdef SYS_BOARD_IS_M3606_01V02
	#define SYS_POWER_GPIO_NUM      12
	#define SYS_MUTE_GPIO_NUM       (96+6) // MGPIO6
#else
	#define SYS_POWER_GPIO_NUM      (96+6) // MGPIO6
	#define SYS_MUTE_GPIO_NUM       12
#endif

#define QAM_INT_GPIO_NUM			  6

#define FP_LOCK_GPIO_NUM            127
#define FP_STANDBY_GPIO_NUM         127
#define FP_CLOCK_GPIO_NUM           127
#define FP_DATA_GPIO_NUM            127
#define FP_CS_GPIO_NUM            	127
#define FP_KEY1_GPIO_NUM            127
#define FP_COM1_GPIO_NUM            127
#define FP_COM2_GPIO_NUM            127
#define FP_COM3_GPIO_NUM            127
#define FP_COM4_GPIO_NUM            127
// GPIO for SCART
//#define POS_SCART_POWER             127
#define ATA2LAN_CS_GPIO_NUM			127
#define SDIO_GPIO_CS				127 // not used for 3606
#define SDIO_GPIO_CS_POLOR			127 // not used for 3606
#define SDIO_GPIO_DETECT			100
#define SDIO_GPIO_LOCK				101
#define GPIO_USB_POWER            	127

#ifdef C3041
#undef SYS_LNB_POWER_CUT
#define SYS_LNB_POWER_CUT           	99
#undef DEMO_RST_GPIO_NUM
#define DEMO_RST_GPIO_NUM           	100


#define SYS_BOARD_IS_M3606_01V02
#define POS_SCART_TVSAT_SWITCH 	101
#define POS_SCART_ASPECT			102
#define POS_SCART_TV_FB			103

#undef FP_CS_GPIO_NUM
#define FP_CS_GPIO_NUM           		8
#undef FP_DATA_GPIO_NUM
#define FP_DATA_GPIO_NUM           	9
#undef FP_CLOCK_GPIO_NUM
#define FP_CLOCK_GPIO_NUM           	37
#undef GPIO_USB_POWER
#define GPIO_USB_POWER            		43

#undef SYS_MUTE_GPIO_NUM
#define SYS_MUTE_GPIO_NUM		127
#undef POS_SCART_POWER
#undef GPIO_RGB_YUV_SWITCH
#endif

//#define DDR_POWER_CONTROL_ENABLE // use a GPIO to control the DDR power on/off
#ifdef DDR_POWER_CONTROL_ENABLE
	#define DDR_POWER_CTL_GPIO_POS		37		/* depends on board design */
	#define DDR_POWER_CTL_GPIO_POLAR	1		/* depends on board design */
#else
	#define DDR_POWER_CTL_GPIO_POS		0xffff	/* invalid GPIO */
	#define DDR_POWER_CTL_GPIO_POLAR	0
#endif

#elif (SYS_MAIN_BOARD == BOARD_DB_M3603_01V01)

#undef  SYS_RFM_MODULE
#define SYS_RFM_MODULE  SYS_DEFINE_NULL

#undef GPIO_RGB_YUV_SWITCH
#ifndef USB_MP_SUPPORT
#undef  MULTI_VOLUME_SUPPORT
#endif

#undef SYS_I2C_SDA
#undef SYS_I2C_SCL
#define SYS_I2C_SDA					10
#define SYS_I2C_SCL					11

#define SYS_LNB_POWER_CUT           12

#define QAM_INT_GPIO_NUM			  6
#define SYS_MUTE_GPIO_NUM             6

#define DEMO_RST_GPIO_NUM           1   // GPIO1 for nim1 reset
#define DEMO_RST_GPIO_NUM2          2   // GPIO2 for nim2 reset
#define SYS_POWER_GPIO_NUM          0

#define FP_LOCK_GPIO_NUM            127

#define FP_STANDBY_GPIO_NUM         105
#define FP_CLOCK_GPIO_NUM           127
#define FP_DATA_GPIO_NUM            127
#define FP_CS_GPIO_NUM            	127
#define FP_KEY1_GPIO_NUM            127
#define FP_COM1_GPIO_NUM            127
#define FP_COM2_GPIO_NUM            127
#define FP_COM3_GPIO_NUM            127
#define FP_COM4_GPIO_NUM            127
#define GPIO_USB_POWER            	127

// GPIO for SCART
//#define POS_SCART_POWER             127

#define ATA2LAN_CS_GPIO_NUM			127
#define SDIO_GPIO_CS				25
#define SDIO_GPIO_CS_POLOR			0
#define SDIO_GPIO_DETECT			46
#define SDIO_GPIO_LOCK				103

#elif (SYS_MAIN_BOARD == BOARD_DB_M3603_02V01)

#undef CI_SUPPORT
#undef SDIO_SUPPORT

#undef  SYS_RFM_MODULE
#define SYS_RFM_MODULE  SYS_DEFINE_NULL

#undef GPIO_RGB_YUV_SWITCH
#ifndef USB_MP_SUPPORT
#undef  MULTI_VOLUME_SUPPORT
#endif

#undef SYS_I2C_SDA
#undef SYS_I2C_SCL
#define SYS_I2C_SDA					10
#define SYS_I2C_SCL					11

#define SYS_LNB_POWER_CUT           7//12

#define DEMO_RST_GPIO_NUM           5//1   // GPIO1 for nim1 reset
#define DEMO_RST_GPIO_NUM2          9//2   // GPIO2 for nim2 reset
#define SYS_POWER_GPIO_NUM          0

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

// GPIO for SCART
//#define POS_SCART_POWER             127

#define ATA2LAN_CS_GPIO_NUM			127
#define SDIO_GPIO_CS				127//25
#define SDIO_GPIO_CS_POLOR			127//0
#define SDIO_GPIO_DETECT			127//46
#define SDIO_GPIO_LOCK				127//103

#define SYS_LNB2_POWER_CUT			75
#define MUTE_GPIO_NUM				6
#define NIM1_INIT_GPIO_NUM			8
#define NIM1_RST_GPIO_NUM			9
#define RMII_RST_GPIO_NUM			37
#define XNF_WPJ_GPIO_NUM			64
#define USB0_PWR_INT_GPIO_NUM		73
#define SCART_16_9_GPIO_NUM			83
#define TV_SAT_GPIO_NUM				84
#define CA_SW5_3_GPIO_NUM			94
#define TV_MODE_GPIO_NUM			105
#define UART1_TX_GPIO_NUM			121
#define UART1_RX_GPIO_NUM			122

#define SMC_GPIO_5V                 94

#elif (SYS_MAIN_BOARD == BOARD_SB_S3602F_QFP_01V01)


#undef  SYS_RFM_MODULE
#define SYS_RFM_MODULE  SYS_DEFINE_NULL

#undef GPIO_RGB_YUV_SWITCH
#ifndef USB_MP_SUPPORT
#undef  MULTI_VOLUME_SUPPORT
#endif

#undef SYS_I2C_SDA
#undef SYS_I2C_SCL
#define SYS_I2C_SDA					10
#define SYS_I2C_SCL					11

#define SYS_LNB_POWER_CUT           127

#define DEMO_RST_GPIO_NUM           1   // GPIO1 for nim1 reset
#define DEMO_RST_GPIO_NUM2          2   // GPIO2 for nim2 reset
#define SYS_POWER_GPIO_NUM          127

#define FP_LOCK_GPIO_NUM            127

#define FP_STANDBY_GPIO_NUM         127
#define FP_CLOCK_GPIO_NUM           127
#define FP_DATA_GPIO_NUM            127
#define FP_CS_GPIO_NUM            	127
#define FP_KEY1_GPIO_NUM            127
#define FP_COM1_GPIO_NUM            127
#define FP_COM2_GPIO_NUM            127
#define FP_COM3_GPIO_NUM            127
#define FP_COM4_GPIO_NUM            127
#define GPIO_USB_POWER            	127

// GPIO for SCART
//#define POS_SCART_POWER             127

#define ATA2LAN_CS_GPIO_NUM			127

#endif

#if((SYS_MAIN_BOARD >= BOARD_SB_S3602F_QFP_01V01 && SYS_MAIN_BOARD < BOARD_S3602F_MAX)||(SYS_MAIN_BOARD==BOARD_DB_M3701C_01V01))


#define SYS_LNB_POWER_OFF   SYS_FUNC_ON
//add for set DacType
#define VDAC_USE_CVBS_TYPE      CVBS_1
#define VDAC_USE_RGB_TYPE       RGB_1
#define VDAC_USE_YUV_TYPE       YUV_1

#define CVBS_DAC                DAC3

//#define SVIDEO_DAC_Y            DAC1
//#define SVIDEO_DAC_C            DAC0

#if (SYS_MAIN_BOARD == BOARD_SB_S3602F_QFP_01V01)
#define RGB_DAC_R               DAC2
#define RGB_DAC_G               DAC0
#define RGB_DAC_B               DAC1

#define YUV_DAC_Y               DAC0
#define YUV_DAC_U               DAC1
#define YUV_DAC_V               DAC2
#else
#define YUV_DAC_Y               DAC2
#define YUV_DAC_U               DAC1
#define YUV_DAC_V               DAC0

#ifdef C3041
#define RGB_DAC_R               DAC0
#define RGB_DAC_G               DAC1
#define RGB_DAC_B               DAC2
#else
#define RGB_DAC_R               DAC0
#define RGB_DAC_G               DAC2
#define RGB_DAC_B               DAC1
#endif

#endif

#endif

#ifdef _BUILD_OTA_E_
	#undef CI_SLOT_DYNAMIC_DETECT
	#define DMX_XFER_V_ES_BY_DMA 	//added for s3601. transfer video ES by DMA, others by CPU copy	
	#undef CI_SUPPORT
	#undef CC_USE_TSG_PLAYER
	#undef CI_STREAM_PASS_MATCH_CARD
	#undef MP2_0_SUPPORT
	#undef MULTI_VOLUME_SUPPORT
	#undef PS_TS_SUPPORT
	#undef DYNAMIC_PID_SUPPORT
    #undef NETWORK_SUPPORT
    #undef SUPPORT_DEO_HINT
    #undef AUTOMATIC_STANDBY
	#undef MP_SUBTITLE_SUPPORT
	#undef SUPPORT_MPEG4_TEST
	#undef SHOW_ALI_DEMO_ON_SCREEN
	#undef SUPPORT_DRAW_EFFECT
	#undef AUDIO_DESCRIPTION_SUPPORT
	#undef RAM_TMS_TEST
	#undef MULTI_CAS
	#undef CAS_TYPE
	#undef SUPPORT_HW_CONAX
	#undef SUPPORT_CAS9

	#undef CAS9_PVR_SUPPORT
	#define DSC_SUPPORT
#endif

#ifdef _DXXX_CERT_ENABLE
    #ifndef M36F_CHIP_MODE
    #error "Only M3606 support dxxx, please modify compile_3602.def"
    #endif

    #define SUPPORT_SUBTITLE
    #define SUPPORT_DXXX_CERT
    #ifdef _M36F_SINGLE_CPU
        #define SUPPORT_DRM
    #endif

    #undef VDAC_USE_RGB_TYPE
    #undef VDAC_USE_YUV_TYPE
#endif

#ifdef _SYSTEM_DVBC
    #define DVBC_COUNTRY_BAND_SUPPORT
    #define SHOW_WELCOME_SCREEN  ////need  define DVBC_COUNTRY_BAND_SUPPORT
    #define QAM_ONLY_USAGE   	SYS_FUNC_ON
#if(SYS_MAIN_BOARD == BOARD_DB_M3701C_01V01)
     #define SYS_DEM_BASE_ADDR  	0x40
#else
    #define SYS_DEM_BASE_ADDR  	0x42	//0x40
#endif
    #define TUNER_I2C_BYPASS
#endif



//#define SECURITY_MP_UPG_SUPPORT 	//just for MP, not to open for normal project

#endif	// __SYS_CONFIG_H

#endif
