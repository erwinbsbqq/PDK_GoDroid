//#include "stdafx.h"
#ifndef  _RT810_H_ 
#define _RT810_H_

#include "../basic_types.h"
//***************************************************************
//*                       INCLUDES.H
//***************************************************************
 //#include "../ali_qam.h"
//Version cloudio_wig_1.0
//modify RT810_Standard Announce
//#pragma once

//----------------------------------------------------------//
//                   Type Define                            //
//----------------------------------------------------------//

//#define UINT8  unsigned char
//#define UINT16 unsigned short
//#define UINT32 unsigned long

typedef enum _RT810_ErrCode
{
    RT_Success = 0,
    RT_Fail    = 1
}RT810_ErrCode;

//----------------------------------------------------------//
//                   RT810 Parameter                        //
//----------------------------------------------------------//

#define RT810_ADDRESS 0x34
//#define RT810_Xtal	  28800
#define RT810_Xtal	  16005

typedef enum _RT810_Standard_Type
{
    NTSC_MN = 0,
    PAL_I,
    PAL_DK,
    PAL_BGH,
    SECAM_L,
    DVB_T_6M,
    DVB_T_7M,
    DVB_T_7M_2,
    DVB_T_8M,
    DVB_C,
    ISDB_T,
    DTMB,
    DVB_C_6M,
    FM,
    ATV = 0xFF
}RT810_Standard_Type;

typedef enum _RT810_Input_Type
{
    AIR_IN   = 0,
    CABLE_IN = 1
}RT810_Input_Type;

typedef enum _RT810_LoopThrough_Type
{
    LOOP_THROUGH = 0,
    SIGLE_IN     = 1
}RT810_LoopThrough_Type;

typedef enum _RT810_GPIO_Type
{
    HI_SIG = 0,
    LO_SIG = 1
}RT810_GPIO_Type;

typedef struct _RT810_Set_Info
{
    UINT32        RF_KHz;
    RT810_Standard_Type RT810_Standard;
    RT810_Input_Type InputMode;
    RT810_LoopThrough_Type RT_Input;
}RT810_Set_Info;

//----------------------------------------------------------//
//                   RT810 Function                         //
//----------------------------------------------------------//
//#define RT810_Delay_MS	Sleep
void RT810_Delay_MS(int ms);

typedef struct _I2C_LEN_TYPE
{
    UINT8 DevAddr;
    UINT8 RegAddr;
    UINT8 Data[50];
    UINT8 Len;
}I2C_LEN_TYPE;

typedef struct _I2C_TYPE
{
    UINT8 DevAddr;
    UINT8 RegAddr;
    UINT8 Data;
    UINT8 Len;
}I2C_TYPE;

RT810_ErrCode RT810_Init(void);
RT810_ErrCode RT810_Standby(RT810_LoopThrough_Type RT810_LoopSwitch, RT810_Input_Type RT810_LoopIN);
RT810_ErrCode RT810_GPIO(RT810_GPIO_Type RT810_GPIO_Conrl);
RT810_ErrCode RT810_Standard(RT810_Standard_Type RT_Standard);
RT810_ErrCode RT810_SetFrequency(RT810_Set_Info RT810_INFO);

RT810_ErrCode I2C_Write_Len_rt810(I2C_LEN_TYPE *I2C_Info);
RT810_ErrCode I2C_Write_rt810(I2C_TYPE *I2C_Info);
RT810_ErrCode I2C_Read_Len_rt810(I2C_LEN_TYPE *I2C_Info);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                      Smart GUI                               //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
extern UINT8 RT810_IMR_XT[6];
extern UINT8 RT810_IMR_YT[6];
RT810_ErrCode SmartGUIFunction(void);
#endif
		
