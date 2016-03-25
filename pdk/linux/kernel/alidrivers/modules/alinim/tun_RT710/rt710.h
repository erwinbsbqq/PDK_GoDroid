//#include "..\stdafx.h"
#ifndef _NIM_RT710_H
#define _NIM_RT710_H
#include <types.h>
#include <hld/nim/nim_tuner.h>


#define RT710_Xtal	27000   /* 27MHZ */
/*#define RT710_Xtal	16000*/

#define UINT8 unsigned char
#define UINT16 unsigned int
#define UINT32 unsigned long

#define RT710_DEVICE_ADDRESS	0xF4
#define RT710_Reg_Num	16
#define MAX_TUNER_SUPPORT_NUM 2

#define RT710_0DBM_SETTING 	FALSE   
/*#define RT710_0DBM_SETTING 	TRUE  */

#define R710_Delay_MS	Sleep

typedef enum _RT710_Err_Type
{
	RT_Success = TRUE,
	RT_Fail    = FALSE
}RT710_Err_Type;

typedef enum _RT710_LoopThrough_Type
{
	LOOP_THROUGH = TRUE,
	SIGLE_IN     = FALSE
}RT710_LoopThrough_Type;

typedef enum _RT710_ClockOut_Type
{
	ClockOutOn = TRUE,
	ClockOutOff= FALSE
}RT710_ClockOut_Type;

typedef enum _RT710_OutputSignal_Type
{
	DifferentialOut = TRUE,
	SingleOut     = FALSE
}RT710_OutputSignal_Type;

typedef enum _RT710_AGC_Type
{
	AGC_Negative = TRUE,
	AGC_Positive = FALSE
}RT710_AGC_Type;

typedef enum _RT710_AttenVga_Type
{
	ATTENVGAON = TRUE,
	ATTENVGAOFF= FALSE
}RT710_AttenVga_Type;

typedef enum _R710_FineGain_Type
{
	FINEGAIN_3DB = 0,
	FINEGAIN_2DB,
	FINEGAIN_1DB,
	FINEGAIN_0DB
}R710_FineGain_Type;

typedef struct _RT710_RF_Gain_Info
{
	UINT8   RF_gain;

}RT710_RF_Gain_Info;

typedef struct _RT710_INFO_Type
{
	UINT32 DVBSBW;
	UINT32 RT710_Freq;
	RT710_LoopThrough_Type RT710_LoopThrough_Mode;
	RT710_ClockOut_Type RT710_ClockOut_Mode;
	RT710_OutputSignal_Type RT710_OutputSignal_Mode;
	RT710_AGC_Type RT710_AGC_Mode;
	RT710_AttenVga_Type RT710_AttenVga_Mode;
	R710_FineGain_Type R710_FineGain;
}RT710_INFO_Type;


//typedef struct QPSK_TUNER_CONFIG_EXT 
//{ 
//        UINT16 wTuner_Crystal;                        /* Tuner Used Crystal: in KHz unit */ 
//        UINT8  cTuner_Base_Addr;                /* Tuner BaseAddress for Write Operation: (BaseAddress + 1) for Read */ 
//        UINT8  cTuner_Out_S_D_Sel;                /* Tuner Output mode Select: 1 --> Single end, 0 --> Differential */ 
//        UINT32 i2c_type_id;        /*i2c type and dev id select. bit16~bit31: type, I2C_TYPE_SCB/I2C_TYPE_GPIO. bit0~bit15:dev id, 0/1.*/         
//}QPSK_TUNER_CONFIG_EXT; 

typedef enum _TUNER_NUM
{
	RT710_TUNER_1 = 0,
	RT710_TUNER_2,
	RT710_TUNER_3,
	RT710_TUNER_4,
	MAX_TUNER_NUM
}RT710_TUNER_NUM_TYPE;

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


#ifdef __cplusplus
extern "C"
{
#endif

INT32 nim_rt710_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 nim_rt710_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
INT32 nim_rt710_status(UINT32 tuner_id, UINT8 *lock);
INT32nim_rt710_Standby(UINT32 tuner_id, RT710_LoopThrough_Type RT710_LTSel );

/***********************************************************
                   RT710 Function                         
************************************************************/
#ifdef __cplusplus
}
#endif

#endif  /* _NIM_RT710_H */

