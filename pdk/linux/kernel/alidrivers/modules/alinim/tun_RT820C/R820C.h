//#include "..\stdafx.h"
#ifndef  _R828_H_ 
#define _R828_H_

//***************************************************************
//*                       INCLUDES.H
//***************************************************************
#include "../basic_types.h"

#define VERSION   "R820C_v1.50C"
#define VER_NUM  50
#define USE_16M_XTAL     TRUE
#define R828_Xtal	  16000

#define USE_DIPLEXER      FALSE
#define TUNER_CLK_OUT  FALSE

//----------------------------------------------------------//
//                   Type Define                            //
//----------------------------------------------------------//
//#define UINT8  unsigned char
//#define UINT16 unsigned short
//#define UINT32 unsigned long

typedef enum _R828_ErrCode
{
	RT_Success = 0,
	RT_Fail    = 1
}R828_ErrCode;

typedef enum _Rafael_Chip_Type  //Don't modify chip list
{
	R828 = 0,
	R828D,
	R828S,
	R820T,
	R820C,
	R620D,
	R620S
}Rafael_Chip_Type;
//----------------------------------------------------------//
//                   R828 Parameter                        //
//----------------------------------------------------------//

extern UINT8 R828_ADDRESS;

#define DIP_FREQ  	  320000
#define IMR_TRIAL    9
#define VCO_pwr_ref   0x01

extern UINT32 R828_IF_khz;
extern UINT32 R828_CAL_LO_khz;
extern UINT8  R828_IMR_point_num;
extern UINT8  R828_IMR_done_flag;
extern UINT8  Rafael_Chip;

typedef enum _R828_Standard_Type  //Don't remove standand list!!
{

	NTSC_MN = 0,
	PAL_I,
	PAL_DK,
	PAL_B_7M,       //no use
	PAL_BGH_8M,     //for PAL B/G, PAL G/H
	SECAM_L,
	SECAM_L1_INV,   //for SECAM L'
	SECAM_L1,       //no use
	ATV_SIZE,
	DVB_T_6M = ATV_SIZE,
	DVB_T_7M,
	DVB_T_7M_2,
	DVB_T_8M,
    DVB_T2_6M,
	DVB_T2_7M,
	DVB_T2_7M_2,
	DVB_T2_8M,
	DVB_T2_1_7M,
	DVB_T2_10M,
	DVB_C_8M,
	DVB_C_6M,
	ISDB_T,
	DTMB,
	ATSC,
	FM,
	STD_SIZE
}R828_Standard_Type;

extern UINT8  R828_Fil_Cal_flag[STD_SIZE];

typedef enum _R828_SetFreq_Type
{
	FAST_MODE = 0,
	NORMAL_MODE = 1
}R828_SetFreq_Type;

typedef enum _R828_LoopThrough_Type
{
	LOOP_THROUGH = 0,
	SIGLE_IN     = 1
}R828_LoopThrough_Type;


typedef enum _R828_InputMode_Type
{
	AIR_IN = 0,
	CABLE_IN_1,
	CABLE_IN_2
}R828_InputMode_Type;

typedef enum _R828_IfAgc_Type
{
	IF_AGC1 = 0,
	IF_AGC2
}R828_IfAgc_Type;

typedef enum _R828_GPIO_Type
{
	HI_SIG = 0,
	LO_SIG = 1
}R828_GPIO_Type;

typedef struct _R828_Set_Info
{
	UINT32        RF_KHz;
	R828_Standard_Type R828_Standard;
	R828_LoopThrough_Type RT_Input;
	R828_InputMode_Type   RT_InputMode;
	R828_IfAgc_Type R828_IfAgc_Select; 
}R828_Set_Info;

typedef struct _R828_RF_Gain_Info
{
	UINT8   RF_gain1;
	UINT8   RF_gain2;
	UINT8   RF_gain_comb;
}R828_RF_Gain_Info;

typedef enum _R828_RF_Gain_TYPE
{
	RF_AUTO = 0,
	RF_MANUAL
}R828_RF_Gain_TYPE;

//----------------------------------------------------------//
//                   R828 Function                         //
//----------------------------------------------------------//

//#define R828_Delay_MS	Sleep
void R828_Delay_MS(int ms);
typedef struct _I2C_LEN_TYPE
{
	UINT8 RegAddr;
	UINT8 Data[50];
	UINT8 Len;
}I2C_LEN_TYPE;

typedef struct _I2C_TYPE
{
	UINT8 RegAddr;
	UINT8 Data;
}I2C_TYPE;

R828_ErrCode R828_Init(void);
R828_ErrCode R828_Standby(R828_LoopThrough_Type R828_LoopSwitch);
R828_ErrCode R828_GPIO(R828_GPIO_Type R828_GPIO_Conrl);
R828_ErrCode R828_SetStandard(R828_Standard_Type RT_Standard);
R828_ErrCode R828_SetFrequency(R828_Set_Info R828_INFO, R828_SetFreq_Type R828_SetFreqMode);
R828_ErrCode R828_GetRfGain(R828_RF_Gain_Info *pR828_rf_gain);
R828_ErrCode R828_RfGainMode(R828_RF_Gain_TYPE R828_RfGainType);

extern UINT8 R828_IMR_XT[6];
extern UINT8 R828_IMR_YT[6];
R828_ErrCode R828_ReadIMR(void);

R828_ErrCode I2C_Write_Len(I2C_LEN_TYPE *I2C_Info);
R828_ErrCode I2C_Write(I2C_TYPE *I2C_Info);
R828_ErrCode I2C_Read_Len(I2C_LEN_TYPE *I2C_Info);

#endif
