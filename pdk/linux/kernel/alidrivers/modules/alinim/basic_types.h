/*****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2004 Copyright (C)
 *
 *  File: basic_types.h
 *
 *  Contents: 	This file define the basic data types which may be used
 *			throughout the project.
 *  History:
 *		Date		Author      		Version 	Comment
 *		==========	==================	========== 	=======
 *  1.  03/09/2004  Tom Gao     		0.1.000 	Initial
 *
 *****************************************************************************/
#ifndef __BASIC_TYPES_H__
#define __BASIC_TYPES_H__


#include <alidefinition/adf_basic.h>


#define HW_TYPE_CHIP_REV		0x00020000	/* Chip Revised */
/* CHIP Revised */
#define IC_REV_0				(HW_TYPE_CHIP_REV + 1)
#define IC_REV_1				(HW_TYPE_CHIP_REV + 2)
#define IC_REV_2				(HW_TYPE_CHIP_REV + 3)
#define IC_REV_3				(HW_TYPE_CHIP_REV + 4)
#define IC_REV_4				(HW_TYPE_CHIP_REV + 5)
#define IC_REV_5				(HW_TYPE_CHIP_REV + 6)
#define IC_REV_6				(HW_TYPE_CHIP_REV + 7)
#define IC_REV_7				(HW_TYPE_CHIP_REV + 8)
#define IC_REV_8				(HW_TYPE_CHIP_REV + 9)



//kent,this define from sys_define.h
#if 0
#define LLD_DEV_TYPE_TUN		0x02050200  /* Tuner */

#define DCT70701				(LLD_DEV_TYPE_TUN + 55)//DVBC
#define DCT7044					(LLD_DEV_TYPE_TUN + 56)//DVBC
#define ALPSTDQE				(LLD_DEV_TYPE_TUN + 57)//DVBC
#define ALPSTDAE				(LLD_DEV_TYPE_TUN + 62) //DVBC
#define TDCCG0X1F				(LLD_DEV_TYPE_TUN + 66)//DVBC
#define DBCTE702F1			    (LLD_DEV_TYPE_TUN + 71)//DVBC
#define CD1616LF				(LLD_DEV_TYPE_TUN + 73)//DVBC
#define ALPSTDAC				(LLD_DEV_TYPE_TUN + 82) //DVBC/DTMB
#define RT810                   (LLD_DEV_TYPE_TUN + 84) //DVBC
#define RT820C                  (LLD_DEV_TYPE_TUN + 85) //DVBC


#define MXL603      	    	(LLD_DEV_TYPE_TUN + 89)
#define SHARP_VZ7306  	        (LLD_DEV_TYPE_TUN + 90)
#define AV_2012 				(LLD_DEV_TYPE_TUN + 91)
#define TDA18250				(LLD_DEV_TYPE_TUN + 92)
#endif


#ifndef	NULL
#define NULL 			      ((void *)0)
#endif

#ifndef TRUE
#define TRUE             1
#endif

#ifndef FALSE
#define FALSE            0
#endif




#define SUCCESS          0       /* Success return */

#define ERR_NO_MEM      -1      /* Not enough memory error */
#define ERR_LONG_PACK   -2      /* Package too long */
#define ERR_RUNT_PACK   -3      /* Package too short */
#define ERR_TX_BUSY     -4      /* TX descriptor full */
#define ERR_DEV_ERROR   -5      /* Device work status error */
#define ERR_DEV_CLASH   -6      /* Device clash for same device in queue */
#define ERR_QUEUE_FULL  -7      /* Queue node count reached the max. val*/
#define ERR_NO_DEV      -8      /* Device not exist on PCI */
#define ERR_FAILURE	  	-9      /* Common error, operation not success */
/* Compatible with previous written error*/
#define ERR_FAILUE	  	-9

#define ERR_PARA        -20     /* Parameter is invalid */
#define ERR_ID_FULL     -21     /* No more ID available */
#define ERR_ID_FREE     -22     /* Specified ID isn't allocated yet */

#define ERR_OFF_SCRN    -30     /* Block is out off the screen */
#define ERR_V_OVRLAP    -31     /* Block is overlaped in vertical */
#define ERR_BAD_CLR     -32     /* Invalid Color Mode code */
#define ERR_OFF_BLOCK   -33     /* Bitmap is out off the block */
#define ERR_TIME_OUT    -34     /* Waiting time out */

/* add by Sen */
#define ERR_FAILED		-40
#define ERR_BUSY		-41
#define ERR_ADDRESS		-42
/* end of Sen */

#if 0

#ifndef TYPEDEF_CONFILICT
#define TYPEDEF_CONFILICT
typedef char		      	INT8;
typedef unsigned char	    UINT8;

typedef short		      	INT16;
typedef unsigned short	    UINT16;

typedef long			    INT32;
typedef unsigned long	    UINT32;

typedef unsigned long long  UINT64;
typedef long long           INT64;
#endif




#endif

#if 0
struct QPSK_TUNER_CONFIG_EXT
{
	UINT16 							w_tuner_crystal;			/* Tuner Used Crystal: in KHz unit */
	UINT8  							c_tuner_base_addr;		/* Tuner BaseAddress for Write Operation: (BaseAddress + 1) for Read */
	UINT8  							c_tuner_out_S_d_sel;		/* Tuner Output mode Select: 1 --> Single end, 0 --> Differential */
	UINT32 							i2c_type_id;	/*i2c type and dev id select. bit16~bit31: type, I2C_TYPE_SCB/I2C_TYPE_GPIO. bit0~bit15:dev id, 0/1.*/	
};

struct QPSK_TUNER_CONFIG_DATA
{
	UINT16 							recv_freq_low;
	UINT16 							recv_freq_high;
	UINT16 							ana_filter_bw;
	UINT8 							connection_config;
	UINT8 							reserved_byte;
	UINT8 							agc_threshold_1;
	UINT8 							agc_threshold_2;
	UINT16 							qpsk_config;/*bit0:QPSK_FREQ_OFFSET,bit1:EXT_ADC,bit2:IQ_AD_SWAP,bit3:I2C_THROUGH,bit4:polar revert bit5:NEW_AGC1,bit6bit7:QPSK bitmode:
	                                00:1bit,01:2bit,10:4bit,11:8bit*/
};



struct QPSK_TUNER_CONFIG_API
{
	/* struct for QPSK Configuration */
	struct   QPSK_TUNER_CONFIG_DATA config_data;

	/* Tuner Initialization Function */
	INT32 (*nim_tuner_init) (UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);

	/* Tuner Parameter Configuration Function */
	INT32 (*nim_tuner_control) (UINT32 tuner_id, UINT32 freq, UINT32 sym);

	/* Get Tuner Status Function */
	INT32 (*nim_tuner_status) (UINT32 tuner_id, UINT8 *lock);

	/* Extension struct for Tuner Configuration */
	struct QPSK_TUNER_CONFIG_EXT tuner_config;
	struct EXT_DM_CONFIG ext_dm_config;
	struct EXT_LNB_CTRL_CONFIG ext_lnb_config;
	UINT32 device_type;	//current chip type. only used for M3501A

	UINT32 tuner_id;
};



#endif

#if 0
struct EXT_DM_CONFIG
{
	UINT32             i2c_base_addr;
	UINT32             i2c_type_id;
	UINT32             dm_crystal;
	UINT32             dm_clock;
	UINT32             polar_gpio_num;
  UINT32             lock_polar_reverse;
};

struct EXT_LNB_CTRL_CONFIG
{
	UINT32             param_check_sum; //ext_lnb_control+i2c_base_addr+i2c_type_id = param_check_sum
	INT32 	           (*ext_lnb_control) (UINT32, UINT32, UINT32);
	UINT32             i2c_base_addr;
	UINT32             i2c_type_id;
	UINT8              int_gpio_en;
	UINT8              int_gpio_polar;
	UINT8              int_gpio_num;
};



struct COFDM_TUNER_CONFIG_EXT
{
	UINT16  c_tuner_crystal;
	UINT8  c_tuner_base_addr;		/* Tuner BaseAddress for Write Operation: (BaseAddress + 1) for Read */	
	UINT8  cChip;
	UINT8  cTuner_Ref_DivRatio;
	UINT16 w_tuner_if_freq;
	UINT8  cTuner_AGC_TOP;
	UINT16 cTuner_Step_Freq;
	INT32  (*Tuner_Write)(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);		/* Write Tuner Program Register */
	INT32  (*Tuner_Read)(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);		/* Read Tuner Status Register */	
	INT32  (*Tuner_Write_Read)(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT8 wlen,int len);
	UINT32 i2c_type_id;	/*i2c type and dev id select. bit16~bit31: type, I2C_TYPE_SCB/I2C_TYPE_GPIO. bit0~bit15:dev id, 0/1.*/				
        
        // copy from COFDM_TUNER_CONFIG_DATA struct in order to  let tuner knows whether the RF/IF AGC is enable or not.
	// esp for max3580, which uses this info to turn on/off internal power detection circuit. See max3580 user manual for detail.

	//bit0: IF-AGC enable <0: disable, 1: enalbe>;bit1: IF-AGC slop <0: negtive, 1: positive>
	//bit2: RF-AGC enable <0: disable, 1: enalbe>;bit3: RF-AGC slop <0: negtive, 1: positive>
	//bit4: Low-if/Zero-if.<0: Low-if, 1: Zero-if>
	//bit5: RF-RSSI enable <0: disable, 1: enalbe>;bit6: RF-RSSI slop <0: negtive, 1: positive>
	UINT16 Cofdm_Config;

	INT32  if_signal_target_intensity;
};

#endif

typedef struct
{
	unsigned long parg1;
	unsigned long parg2;
}AUTOSCAN_MSG;

typedef struct
{
	INT32 port_id;
	UINT32 flag;
	AUTOSCAN_MSG msg;
}AUTO_SCAN_PARA;



struct t_diseqc_info
{
	UINT8 							sat_or_tp;			/* 0:sat, 1:tp*/
	UINT8 							diseqc_type;
	UINT8 							diseqc_port;
	UINT8 							diseqc11_type;
	UINT8 							diseqc11_port;
	UINT8 							diseqc_k22;
	UINT8 							diseqc_polar;		/* 0: auto,1: H,2: V */
	UINT8 							diseqc_toneburst;	/* 0: off, 1: A, 2: B */	

	UINT8 							positioner_type;	/*0-no positioner 1-1.2 positioner support, 2-1.3 USALS*/
	UINT8 							position;			/*use for DiSEqC1.2 only*/	
	UINT16 							wxyz;			/*use for USALS only*/
};



enum NIM_BLSCAN_MODE
{
	NIM_SCAN_FAST = 0,
	NIM_SCAN_SLOW = 1,
};


enum nim_perf_level
{
	NIM_PERF_DEFAULT		= 0,
	NIM_PERF_SAFER		= 1,
	NIM_PERF_RISK			= 2,
};



typedef INT32 (*pfn_nim_reset_callback)(UINT32 param);



enum NIM_TYPE
{
	NIM_DVBS     =1,
	NIM_DVBC,
	NIM_DVBT
};

typedef struct _tuner_config_t
{
	UINT8							type;
	UINT16          				tuner_crystal;		// Tuner Used Crystal: in KHz unit
	UINT8  							tuner_base_addr;	// Tuner BaseAddress for Write Operation: (BaseAddress + 1) for Read
	UINT32 							i2c_type;	        //i2c type and dev id select. bit16~bit31: type, I2C_TYPE_SCB/I2C_TYPE_GPIO. bit0~bit15:dev id, 0/1.
	UINT16                          tuner_if_freq;      //
}TUNER_CONFIG_DATA;



//common struct,unified use of the interface in the future
typedef INT32 (*tuner_init_callback) (UINT32* tuner_id, TUNER_CONFIG_DATA* tuner_config);
typedef INT32 (*tuner_control_callback) (UINT32 tuner_id, UINT32 freq, UINT32 sym);
typedef INT32 (*tuner_status_callback) (UINT32 tuner_id, UINT8 *lock);
typedef INT32 (*tuner_close_callback)(void);


typedef INT32 (*dvbs_tuner_init_callback) (UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
typedef INT32 (*dvbt_tuner_init_callback) (UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
typedef INT32 (*dvbc_tuner_init_callback) (UINT32 *tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);


typedef struct _tuner_io_func_t
{
	enum NIM_TYPE 			nim_type;              //1// 1,dvbs,2,dvbc,3,dvbt
	UINT32                  tuner_id;
	tuner_init_callback     pf_init;
	tuner_control_callback  pf_control;
	tuner_status_callback   pf_status;
	tuner_close_callback    pf_close;
}TUNER_IO_FUNC;





#endif	//__BASIC_TYPES_H__

