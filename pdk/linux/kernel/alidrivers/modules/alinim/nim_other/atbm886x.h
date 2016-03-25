/***************************************************************************** 
Copyright 2012-2020, AltoBeam Inc. All rights reserved.
     
File Name: atbm886x.h
******************************************************************************/
#ifndef __ATBM886X_H__
#define __ATBM886X_H__

#include "../porting_linux_header.h"

#include "../tun_other/tdac_7_tuner.h"
/*Common data type redefine for atbm886x.h/c*/

typedef unsigned char   	uint8;
typedef unsigned short  	uint16;
typedef unsigned int     	uint32;
typedef unsigned char  	UINT8;
typedef unsigned short 	UINT16;
typedef unsigned int   		UINT32;
typedef int 				INT32;

#define _1st_i2c_cmd		0
#define _2nd_i2c_cmd		1


#define ATMB_DBG_OUTPUT   0

/********************************************************************************************************************************** 
*struct MPEG_TS_mode_t 
*@ui8TSTransferType: TS stream transfer type, can be set to parallel(8 bit data bus) or serial(1 bit data bus) mode
*@Demod output edge: demod will output TS data on this edge of TS stream clock
*@ui8SPIClockConstantOutput: TS stream clock can be set outputting all the time or only during TS data valid (188 bytes) 
**********************************************************************************************************************************/
/*****************ui8TSTransferType Option Value***************************/
#define TS_PARALLEL_MODE             1
#define TS_SERIAL_MODE               0
/**********************ui8OutputEdge Option Value***************************/
#define TS_OUTPUT_FALLING_EDGE       1
#define TS_OUTPUT_RISING_EDGE        0
/**********************ui8TSSPIMSBSelection Option Value******************/
#define TS_SPI_MSB_ON_DATA_BIT7      1
#define TS_SPI_MSB_ON_DATA_BIT0      0
/**********************ui8TSSSIOutputSelection Option Value***************/
#define TS_SSI_OUTPUT_ON_DATA_BIT7   1
#define TS_SSI_OUTPUT_ON_DATA_BIT0   0
/**********************ui8SPIClockConstantOutput Option Value*************/
#define TS_CLOCK_CONST_OUTPUT        1
#define TS_CLOCK_VALID_OUTPUT        0	
typedef struct MPEG_TS_mode_t 
{
	uint8 ui8TSTransferType;
	uint8 ui8OutputEdge; 
	uint8 ui8TSSPIMSBSelection;
	uint8 ui8TSSSIOutputSelection;
	uint8 ui8SPIClockConstantOutput; 		
}MPEG_TS_mode_t;

/********************************************************************************************************************************** 
*struct DVBC_Params_t 
*i32SymbolRate: typically use 6875K
*ui8InputMode : for DVBC parameter config 
**********************************************************************************************************************************/
/**********************ui8InputMode Option Value****************************/
#define DVBC_IF_INPUT                0
#define DVBC_IQ_INPUT                1
typedef struct DVBC_Params_t 
{   
	uint8 ui8InputMode;	
	int	  i32SymbolRate; 
}DVBC_Params_t;

/********************************************************************************************************************************** 
*struct tuner_config_t
*@dbIFFrequency: tuner IF frequency output in MHz.  Most CAN Tuners' are 36M, 36.166M
*@or 36.125 MHz (Typical IF used by DVB-C tuners)
*@ui8IQmode: demod needs to know if IQ is swapped or not on hardware board
**********************************************************************************************************************************/
/**********************ui8IQmode Option Value*******************************/
#define SWAP_IQ                      0
#define NO_SWAP_IQ                   1
typedef struct tuner_config_t 
{
	uint8  ui8IQmode;	
	uint8  ui8DTMBBandwithMHz;/**/
	uint32 ui32IFFrequency;
}tuner_config_t;

/********************************************************************************************************************************** 
* struct     custom_config_t 
*@tuner_config: struct of tuner configuration
*@stTsMode: struct of TS mode
*@ui8CrystalOrOscillator: demod can use crystal or oscillator 
*@dbSampleClkFrequency: crystal or oscillator frequency on hardware board for demod
*@ui8DtmbDvbcMode:select receiving mode DTMB or DVB-C for ATBM886x
*@stDvbcParams: DVB-C parameters
**********************************************************************************************************************************/
/**********************ui8CrystalOrOscillator Option Value*****************/
#define CRYSTAL                      0   
#define OSCILLATOR                   1   
/**********************ui8DtmbDvbcMode Option Value************************/
#define ATBM_DTMB_MODE               1
#define ATBM_DVBC_MODE               0    
typedef struct custom_config_t
{
	uint8          ui8CrystalOrOscillator;
	uint8          ui8DtmbDvbcMode;	
	tuner_config_t stTunerConfig;
	MPEG_TS_mode_t stTsMode;	
	uint32         ui32SampleClkFrequency;
	DVBC_Params_t  stDvbcParams;
}custom_config_t;

typedef enum _ATBM_I2CREADWRITE_STATUS
{
	ATBM_I2CREADWRITE_OK    =0,
	ATBM_I2CREADWRITE_ERROR
}ATBM_I2CREADWRITE_STATUS;



typedef enum _DTMB_QAM_INDEX
{
	DTMB_QAM_UNKNOWN = 0,
	DTMB_QAM_4QAM_NR,
	DTMB_QAM_4QAM,
	DTMB_QAM_16QAM,
	DTMB_QAM_32QAM,
	DTMB_QAM_64QAM
}DTMB_QAM_INDEX;

// Code rate
typedef enum _DTMB_CODE_RATE
{
	DTMB_CODE_RATE_UNKNOWN = 0,
	DTMB_CODE_RATE_0_DOT_4,
	DTMB_CODE_RATE_0_DOT_6,
	DTMB_CODE_RATE_0_DOT_8
}DTMB_CODE_RATE;

// Time interleaving
typedef enum _DTMB_TIME_INTERLEAVE
{
	DTMB_TIME_INTERLEAVER_UNKNOWN = 0,
	DTMB_TIME_INTERLEAVER_240,
	DTMB_TIME_INTERLEAVER_720
}DTMB_TIME_INTERLEAVE;

//Single carrier or Multi-Carrier
typedef enum _DTMB_CARRIER_MODE
{   
	DTMB_CARRIER_UNKNOWN = 0,
	DTMB_SINGLE_CARRIER,
	DTMB_MULTI_CARRIER
}DTMB_CARRIER_MODE;

typedef enum _DTMB_GUARD_INTERVAL
{
	GI_UNKNOWN = 0,
	GI_420,
	GI_595,
	GI_945
}DTMB_GUARD_INTERVAL;

typedef struct STRU_DTMB_SIGNAL_PARAMS
{
	DTMB_CARRIER_MODE dtmb_carrier_mode;
	DTMB_QAM_INDEX dtmb_qam_index;
	DTMB_CODE_RATE dtmb_code_rate;
	DTMB_TIME_INTERLEAVE dtmb_time_interleave;
	DTMB_GUARD_INTERVAL dtmb_guard_interval;		
}DTMB_SIGNAL_PARAMS;


/********DTMB and DVB-C common API functions******************************/
int ATBMPowerOnInit(void);
uint8  ATBMChipID(void);
void   ATBMI2CByPassOn(void);
void   ATBMI2CByPassOff(void);
int    ATBMLockedFlag(void);
int    ATBMChannelLockCheck(void);
int    ATBMChannelLockCheckForManual(void);
void   ATBMHoldDSP(void);
void   ATBMStartDSP(void);
void   ATBMStandby(void);
void   ATBMStandbyWakeUp(void);
void   ATBMSuspend(void);

/****DTMB API Functions***************************************************/
void   ATBMSetDTMBMode(void);
int    ATBMSignalStrength(void);	
uint32 ATBMSignalNoiseRatio(void);
int    ATBMSignalQuality(void);
uint32 ATBMFrameErrorRatio(void);
uint32 ATBMPreBCHBlockErrorRatio(void);
uint32 ATBMBER_Calc(void);
int    ATBM_PPM_Test(void);
int ATBMCarrierOffset(void);
int  ATBMGetDTMBBitRate(void);

/****DVBC API Macro define************************************************/
#define ATBM_DEBUG_DVBC                           0      /*default no debug output*/
// #define CMS0022_COARSE_CARRIER_ACQ_SWEEP_STEP    5/100 
#define DVBC_SAMPLE_RATE_ADDR                     0x210
#define DVBC_SAMPLE_RATE_RECIP_ADDR               0x214
#define DVBC_OUTPUT_SHIFT_ADDR                    0x128
#define DVBC_DECIMATION_FACTOR_ADDR               0x124
#define DVBC_SLOW_CONTROL_TC_ADDR                 0x3BC
#define DVBC_CARRIER_LOCK_ACQUIRE_TIMEOUT_ADDR    0x348
#define DVBC_PL_CARRIER_FREQUENCY_RANGE_ADDR      0x38C
#define DVBC_PL_CARRIER_STEP_FREQUENCY_ADDR       0x388
#define DVBC_COARSE_FREQUENCY_OFFSET_ADDR         0x118
#define DVBC_SEARCH_STEP_ADDR                     0x3B0 
#define DVBC_SEARCH_RANGE_ADDR                    0x3B4  
#define DVBC_BITSYNC_DETECT_TIMEOUT_ADDR          0x364 
#define DVBC_AUTO_EQU_SEARCH_ADDR                 0x3CC
/****DVB-C API Functions*************************************************/
void   ATBMSetDVBCMode(void);// this function may be changed later
int ATBMDVBCSNR(void);
uint32 ATBMDVBCBER(int *i32pBerExponent);
uint32 ATBMDVBCUncorrectablePER(int *i32pPktsExponent);
uint8  ATBMDVBCGetQAM(void);
int    ATBMDVBCSignalStrength(void);
uint32 ATBMDVBCGetSymbolRate(void);
int ATBMDVBCCarrierOffset(void);

/*************DVB-C internal functions************************/
void   ATBMDVBCInit( custom_config_t stCustomConfig);
void   ATBMDVBCSetSymbolRate(uint32 ui32OSCFreq, uint32 ui32SymbolRateM);
void   ATBMDVBCSetCarrier(uint32 ui32OSCFreq,uint32 ui32SymbolRateM);
void   ATBMDVBCSetQAM(void);

/******************Demodulator Internal functions***********************/
void   ATBMSetConfigParas(custom_config_t stCustomConfigp);
void   ATBMInit(void);
void   ATBMConfig( custom_config_t stCustomConfig);
void   ATBMSetTSMode( MPEG_TS_mode_t stTSMode);   /*Default SPI , it can be configured to Serial mode*/
int    ATBMSetOSC( tuner_config_t stTunerConfig, uint32 ui32SampleClkFrequency); 
uint8  ATBMGetTPS(void);
void   ATBMDebugRegister(void);
uint8  ATBMCheckDemodStatus(void);
int    ATBMReset(uint8 ui8CryOrOsc);
uint8  ATBMCheckPLLStatus(void);
/****DTMB I2C interface functions****************************************/
void   ATBMWriteRegArray(uint8 *ui8ARegTable, int i32TableLen);
void   ATBMDebugRegArray(uint8 *ui8ARegTable, int i32TableLen);
ATBM_I2CREADWRITE_STATUS  ATBMRead(uint8 ui8BaseAddr, uint8 ui8RegisterAddr,uint8 *ui8pValue);
ATBM_I2CREADWRITE_STATUS  ATBMWrite(uint8 ui8BaseAddr, uint8 ui8RegisterAddr, uint8 ui8Data);
/****DVB-C I2C interface functions***************************************/
ATBM_I2CREADWRITE_STATUS  ATBMTransRead(uint8 ui8BaseAddr, uint8 ui8RegisterAddr,uint8 *ui8pValue);
ATBM_I2CREADWRITE_STATUS  ATBMTransWrite(uint8 ui8BaseAddr, uint8 ui8RegisterAddr, uint8 ui8Data);
ATBM_I2CREADWRITE_STATUS  ATBMDVBCWrite(uint32 ui32AAddress,uint32 ui32Data);
ATBM_I2CREADWRITE_STATUS  ATBMDVBCRead(uint32 ui32AAddress,uint32 *ui32pValue);

/****General interface functions*****************************************/
uint32 ATBMPreBCHBlockErrorRatio(void);
void   ATBM_GPO_I2CINT_Output(uint8 ui8Level);
void   ATBM_GPO_PWM1_Output(uint8 ui8Level);
void   ATBM_GPO_TestIO23_Output(uint8 ui8Level);
void   ATBM_TestIO23_Indicate_TS_Lock(void);
void   ATBM_TestIO23_Indicate_FEC_No_Error(void);
void   ATBM_GPO_TestIO20_Output(uint8 ui8Level);
void   ATBM_TestIO20_Indicate_FEC_Error(void);
void   ATBM_TestIO20_Indicate_TS_Unlock(void);
void   ATBMErrorOnDurationMillisecond(int i32MS);
void   ATBMLockOffDurationMillisecond(int i32MS);


//API of Getting DTMB signal parameters
DTMB_GUARD_INTERVAL   ATBM_GetGuradInterval(void);
DTMB_QAM_INDEX        ATBM_GetQamIndex(void);
DTMB_CODE_RATE        ATBM_GetCodeRate(void);
DTMB_TIME_INTERLEAVE  ATBM_GetInterleavingMode(void);
DTMB_CARRIER_MODE     ATBM_GetCarrierMode(void);
void                  ATBMGetSignalParameters(DTMB_SIGNAL_PARAMS *singal_params);


/****extern interface functions******************************************/
void Delayms (int i32MS);
void DemodHardwareReset(void);
ATBM_I2CREADWRITE_STATUS I2CReadOneStep(uint8 ui8I2CSlaveAddr, uint16 addr_length, uint8 *addr_dat,  uint16 data_length, uint8 *reg_dat);
ATBM_I2CREADWRITE_STATUS I2CWriteWithRestart(uint8 ui8I2CSlaveAddr, uint8 addr_length, uint8 *addr_dat,  uint8 data_length, uint8 *reg_dat);


#define DVBCWriteValue(BaseAddr,RegAddr,WriteValue) {\
   ATBM_I2CREADWRITE_STATUS enumStatus;\
   enumStatus =ATBMTransWrite(BaseAddr,RegAddr,WriteValue);\
   if(ATBM_I2CREADWRITE_OK != enumStatus)\
   {\
       return enumStatus;\
   }\
}

#define DVBCReadValue(BaseAddr,RegAddr,ReadValue) {\
	ATBM_I2CREADWRITE_STATUS enumStatus;\
	enumStatus =ATBMTransRead(BaseAddr,RegAddr,ReadValue);\
	if(ATBM_I2CREADWRITE_OK != enumStatus)\
{\
	return enumStatus;\
}\
}

/////////ali device define added /////////
#define ATMB_DEBUG_FLAG	0
#if(ATMB_DEBUG_FLAG)
#define QAM_DEBUG			1
#define TUNER_DEBUG		1
#else
#define QAM_DEBUG			0
#define TUNER_DEBUG		0
#endif
#if(QAM_DEBUG)
	#define ATBM_PRINTF	printk
#else
	#define ATBM_PRINTF(...)		do{}while(0)
#endif
#if(TUNER_DEBUG)
	#define TUNER_PRINTF	printk
#else
	#define TUNER_PRINTF(...)	do{}while(0)
#endif

#define ATMB_LOG_ERROR		1		// add for printf ERROR
#if(ATMB_LOG_ERROR)
	#define NIM_LOG_ERROR(format, args...)		printk( KERN_ERR "func: %s, line: %d "format,  __FUNCTION__, __LINE__, ##args)
#else
	#define NIM_LOG_ERROR(...)					do{}while(0)
#endif

#define NIM_MUTEX_ENTER(priv)  \
	do \
	{ \
		mutex_lock(&priv->i2c_mutex); \
	}while(0)

#define NIM_MUTEX_LEAVE(priv) \
	do\
	{ \
		mutex_unlock(&priv->i2c_mutex);\
	}while(0)


#define SUCCESS         0       		/* Success return */
#define ERR_NO_MEM      -1      	/* Not enough memory error */
#define ERR_LONG_PACK   -2      	/* Package too long */
#define ERR_RUNT_PACK   -3      	/* Package too short */
#define ERR_TX_BUSY     -4      	/* TX descriptor full */
#define ERR_DEV_ERROR   -5      	/* Device work status error */
#define ERR_DEV_CLASH   -6      	/* Device clash for same device in queue */
#define ERR_QUEUE_FULL  -7      	/* Queue node count reached the max. val*/
#define ERR_NO_DEV      -8      	/* Device not exist on PCI */
#define ERR_FAILURE		-9      	/* Common error, operation not success */
/* Compatible with previous written error*/
#define ERR_FAILUE		-9

#define ERR_PARA        -20     		/* Parameter is invalid */
#define ERR_ID_FULL     -21     	/* No more ID available */
#define ERR_ID_FREE     -22     	/* Specified ID isn't allocated yet */

#define ERR_OFF_SCRN    -30     	/* Block is out off the screen */
#define ERR_V_OVRLAP    -31     	/* Block is overlaped in vertical */
#define ERR_BAD_CLR     -32     	/* Invalid Color Mode code */
#define ERR_OFF_BLOCK   -33     	/* Bitmap is out off the block */
#define ERR_TIME_OUT    -34     	/* Waiting time out */

/* add by Sen */
#define ERR_FAILED		-40
#define ERR_BUSY		-41
#define ERR_ADDRESS		-42
/* end of Sen */

#define I2C_ERROR_BASE		-200

#define ERR_I2C_SCL_LOCK	(I2C_ERROR_BASE - 1)	/* I2C SCL be locked */
#define ERR_I2C_SDA_LOCK	(I2C_ERROR_BASE - 2)	/* I2C SDA be locked */
#define ERR_I2C_NO_ACK		(I2C_ERROR_BASE - 3)	/* I2C slave no ack */
#define S3501_ERR_I2C_NO_ACK	ERR_I2C_NO_ACK

//#define ALI_TRACE_MABEL_TUN
#ifdef ALI_TRACE_MABEL_TUN
#define PRINTK_INFO(fmt,args...)	printk("\033[1m\033[40;31m [%s]@[%d]:  \033[0m\n"fmt, __FUNCTION__, __LINE__,##args);
#else
#define PRINTK_INFO(fmt,args...)
#endif
#define ALI_NIM_DEVICE_NAME 	"ali_nim_m3200"

struct tuner_model
{
	uint8 dev_ids;
	uint8 dev_name[16];
};
struct tuner_handle
{
	uint8 seq_num;
	uint8 tun_name[16];
	int (*tuner_init)(uint32 *ptrTun_id, struct QAM_TUNER_CONFIG_EXT *ptrTuner_Config);
    	int (*tuner_control)(uint32 Tun_id, uint32 freq, uint32 sym, uint8 AGC_Time_Const, uint8 _i2c_cmd);
    	int (*tuner_status)(uint32 Tun_id, uint8 *lock);
    	int (*tuner_rflevel)(uint32 Tun_id, uint32 *type, uint32 *rf_level);
	int (*tuner_release)(void);

	struct list_head list;
};
struct ali_nim_m3200_private
{
	/* struct for QAM Configuration */
	struct QAM_TUNER_CONFIG_DATA tuner_config_data;
	
	/* Tuner Initialization Function */
	int (*nim_tuner_init)(uint32 *ptrTun_id, struct QAM_TUNER_CONFIG_EXT *ptrTuner_Config);
	/* Tuner Parameter Configuration Function */
	int (*nim_tuner_control)(uint32 Tun_id, uint32 freq, uint32 sym, uint8 AGC_Time_Const, uint8 _i2c_cmd);
	/* Get Tuner Status Function */
	int (*nim_tuner_status)(uint32 Tun_id, uint8 *lock);
	/* Get Tuner RF level Function: type = 0, rf_level dbuV unit, type = 1, rf_level AGC unit. */
	int (*nim_Tuner_Get_rf_level)(uint32 Tun_id, uint32 *type, uint32 *rf_level);
	/* Clear tuner init flag Function */
	int (*nim_Tuner_Release)(void);
	/* Extension struct for Tuner Configuration */
	struct QAM_TUNER_CONFIG_EXT tuner_config_ext;
	
	//struct QAM_TUNER_CONFIG_API TUNER_PRIV;	
	uint32 tuner_id;
	uint32 dem_i2c_id;
	uint32 qam_mode;
	struct mutex i2c_mutex;
	struct workqueue_struct *workqueue;
	struct work_struct work;
};
struct ali_nim_device
{
	struct cdev cdev;
	void *priv;
};

int demo_I2c_read(uint8 I2CSlaveAddr, uint8 *data, int length);
int demo_I2c_write(uint8 I2CSlaveAddr, uint8 *data, int length);
//int ali_i2c_scb_write(uint32 id, uint8 slv_addr, uint8 *data, int len);
//int ali_i2c_scb_read(uint32 id, uint8 slv_addr, uint8 *data, int len);
INT32 ali_nim_tuner_register(struct tuner_handle *model);
INT32 ali_nim_tuner_unregister(const UINT8 *model_name);


#endif
