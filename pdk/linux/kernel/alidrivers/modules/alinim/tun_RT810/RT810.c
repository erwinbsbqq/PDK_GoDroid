/****************************************************************

                           R T 8 1 0 . C P P


This module deals with the RT810P2 share Xtal version.

Copyright 2011 by Rafaelmicro., Inc.


                 C H A N G E   R E C O R D

  Date         Who            Version      Description
--------   --------------    --------   ----------------------
10/28/100  Cloudio & Wig        124.0        Module released
10/31/100       Wig             124.1    Increase DVB-C 8M system
****************************************************************/


//#include "stdafx.h"
#include "RT810.h"



//#include "I2C_Sys.h"

//----------------------------------------------------------//
//                   RT810 Protct Parameter                 //
//----------------------------------------------------------//

#define RingCal	TRUE
#define LNACal	FALSE

#define BOOL UINT8
#define bool UINT8

#define true TRUE
#define false FALSE

UINT8 RT810_iniArry[50] = {0xDB, 0x04, 0x92, 0x05, 0x60, 0x06, 0x1C, 0x07, 0x34, 0x08,
	                     //0x03        0x04        0x05        0x06        0x07      
	                      0x40, 0x09, 0x40, 0x0A, 0x4F, 0x0B, 0x02, 0x0C, 0x90, 0x0D,
	                      //0x08      0x09        0x0A        0x0B        0x0C       
	                      0x82, 0x0E, 0x63, 0x0F, 0x88, 0x10, 0x20, 0x11, 0x13, 0x12,
	                      //0x0D      0x0E        0x0F        0x10        0x11      
	                      0x40, 0x13, 0x00, 0x14, 0xC8, 0x15, 0x5B, 0x16, 0x8E, 0x17,
	                      //0x12      0x13        0x14        0x15        0x16      
	                      0x0F, 0x18, 0xC1, 0x19, 0x40, 0x1A, 0x10, 0x1B, 0x00};
	                      //0x17      0x18        0x19        0x1A        0x1B 


//----------------------------------------------------------//
//                   Internal Structs                       //
//----------------------------------------------------------//
typedef struct _RT810_SectType
{
	UINT8 Phase_Y;
	UINT8 Gain_X;
	UINT16 Value;
}RT810_SectType;

typedef enum _BW_Type
{
	BW_6M = 0,
	BW_7M,
	BW_8M
}BW_Type;

typedef struct _Sys_Info_Type
{
	UINT16		IF_KHz;
	BW_Type		BW;
	UINT32		FILT_CAL_LO;
	UINT8		FILT_GAIN;
	UINT8		HP_COR;
	UINT8		FILT_I;
}Sys_Info_Type;

typedef struct _Freq_Info_Type
{
	UINT8		RF_MUX;
	UINT8		TF_BAND;
	UINT8		RFPOLY;
	UINT8		IMR_MEM;
}Freq_Info_Type;


typedef struct _SysFreq_Info_Type
{
	UINT8		LNA_TOP;
	UINT8		LNA_VTH_L;
	UINT8		MIXER_TOP;
	UINT8		MIXER_VTH_L;
}SysFreq_Info_Type;

//----------------------------------------------------------//
//                   Internal Parameters                    //
//----------------------------------------------------------//
UINT8 RT810_Arry[50];
RT810_SectType IMR_Data_rt810[6] = {0, 0, 0, 0, 0, 0};//Please keep this array data for standby mode.
I2C_TYPE  RT810_I2C;
I2C_LEN_TYPE RT810_I2C_Len;

//----------------------------------------------------------//
//                   Internal static struct                 //
//----------------------------------------------------------//
static SysFreq_Info_Type SysFreq_Info1;
static Sys_Info_Type Sys_Info1;
static Freq_Info_Type RT810_Freq_Info;
//----------------------------------------------------------//
//                   Internal Functions                     //
//----------------------------------------------------------//
RT810_ErrCode RT810_InitReg(void);
RT810_ErrCode RT810_IMR_Prepare(void);
RT810_ErrCode RT810_IMR(UINT8 IMR_MEM, BOOL IM_Flag);
RT810_ErrCode RT810_PLL(UINT32 LO_Freq);
RT810_ErrCode RT810_MUX(UINT32 RF_KHz);
RT810_ErrCode RT810_IQ(RT810_SectType* IQ_Pont);
RT810_ErrCode RT810_IQ_Tree(UINT8 FixPot, UINT8 FlucPot, UINT8 PotReg, RT810_SectType* CompareTree);
RT810_ErrCode RT810_CompreCor(RT810_SectType* CorArry);
RT810_ErrCode RT810_CompreStep(RT810_SectType* StepArry, UINT8 Pace);
RT810_ErrCode RT810_Muti_Read(UINT8 IMR_Reg, UINT16* IMR_Data_rt810);
RT810_ErrCode RT810_Section(RT810_SectType* SectionArry);
RT810_ErrCode RT810_F_IMR(RT810_SectType* IQ_Pont);
RT810_ErrCode RT810_Filt_Cal(UINT32 Cal_Freq);

Sys_Info_Type RT810_Sys_Sel(RT810_Standard_Type RT810_Standard);
Freq_Info_Type RT810_Freq_Sel(UINT32 RF_freq);
SysFreq_Info_Type RT810_SysFreq_Sel(RT810_Standard_Type RT810_Standard,UINT32 RF_freq);

RT810_ErrCode RT810_Filt_Cal(UINT32 Cal_Freq);
RT810_ErrCode RT810_SetFrequency(RT810_Set_Info RT810_INFO);



Sys_Info_Type RT810_Sys_Sel(RT810_Standard_Type RT810_Standard)
{
	Sys_Info_Type RT810_Sys_Info;

	switch (RT810_Standard)
	{
	case 0:  //NTSC_MN
		RT810_Sys_Info.IF_KHz=5320;
		RT810_Sys_Info.BW=BW_6M;
		RT810_Sys_Info.FILT_CAL_LO=68000;
		RT810_Sys_Info.FILT_GAIN=0x00;  //0dB
		RT810_Sys_Info.HP_COR=0x10;		//1.2
		RT810_Sys_Info.FILT_I=0x60;		//lowest
		break;
	case 1:  //PAL_I
		RT810_Sys_Info.IF_KHz=6820;
		RT810_Sys_Info.BW=BW_8M;
		RT810_Sys_Info.FILT_CAL_LO=72000;
		RT810_Sys_Info.FILT_GAIN=0x00;  //0dB
		RT810_Sys_Info.HP_COR=0x10;		//1.2
		RT810_Sys_Info.FILT_I=0x60;		//lowest
		break;
	case 2:  //PAL_DK
		RT810_Sys_Info.IF_KHz=7320;
		RT810_Sys_Info.BW=BW_8M;
		RT810_Sys_Info.FILT_CAL_LO=68000;
		RT810_Sys_Info.FILT_GAIN=0x00;  //0dB
		RT810_Sys_Info.HP_COR=0x10;		//1.2
		RT810_Sys_Info.FILT_I=0x60;		//lowest
		break;
	case 3:  //PAL_BGH
		RT810_Sys_Info.IF_KHz=6820;
		RT810_Sys_Info.BW=BW_7M;
		RT810_Sys_Info.FILT_CAL_LO=74000;
		RT810_Sys_Info.FILT_GAIN=0x00;  //0dB
		RT810_Sys_Info.HP_COR=0x10;		//1.2
		RT810_Sys_Info.FILT_I=0x60;		//lowest
		break;
	case 4:  //SECAM_L
		RT810_Sys_Info.IF_KHz=7320;
		RT810_Sys_Info.BW=BW_8M;
		RT810_Sys_Info.FILT_CAL_LO=68000;
		RT810_Sys_Info.FILT_GAIN=0x00;  //0dB
		RT810_Sys_Info.HP_COR=0x10;		//1.2
		RT810_Sys_Info.FILT_I=0x60;		//lowest
		break;
	case 5:  //DVB_T_6M
		RT810_Sys_Info.IF_KHz=3570;
		RT810_Sys_Info.BW=BW_6M;
		RT810_Sys_Info.FILT_CAL_LO=68000;
		RT810_Sys_Info.FILT_GAIN=0x40;  //3dB
		RT810_Sys_Info.HP_COR=0x18;		//1M
		RT810_Sys_Info.FILT_I=0x40;		//low
		break;
	case 6:  //DVB_T_7M
		RT810_Sys_Info.IF_KHz=4070;
		RT810_Sys_Info.BW=BW_7M;
		RT810_Sys_Info.FILT_CAL_LO=68000;
		RT810_Sys_Info.FILT_GAIN=0x40;  //3dB
		RT810_Sys_Info.HP_COR=0x18;		//1M
		RT810_Sys_Info.FILT_I=0x40;		//low
		break;
	case 7:  //DVB_T_7M_2
		RT810_Sys_Info.IF_KHz=4570;
		RT810_Sys_Info.BW=BW_7M;
		RT810_Sys_Info.FILT_CAL_LO=72000;
		RT810_Sys_Info.FILT_GAIN=0x40;  //3dB
		RT810_Sys_Info.HP_COR=0x10;		//1.2M
		RT810_Sys_Info.FILT_I=0x40;		//low
		break;
	case 8:  //DVB_T_8M
		RT810_Sys_Info.IF_KHz=4570;
		RT810_Sys_Info.BW=BW_8M;
		RT810_Sys_Info.FILT_CAL_LO=68000;
		RT810_Sys_Info.FILT_GAIN=0x40;  //3dB
		RT810_Sys_Info.HP_COR=0x18;		//1M
		RT810_Sys_Info.FILT_I=0x40;		//low
		break;
	case 9:  //DVB_C
		RT810_Sys_Info.IF_KHz=5070;
		RT810_Sys_Info.BW=BW_8M;
		RT810_Sys_Info.FILT_CAL_LO=74000;
		RT810_Sys_Info.FILT_GAIN=0x80;  //6dB
		RT810_Sys_Info.HP_COR=0x18;		//1M
		RT810_Sys_Info.FILT_I=0x40;		//low
		break;
	case 10:  //ISDB_T
		RT810_Sys_Info.IF_KHz=4067;
		RT810_Sys_Info.BW=BW_6M;
		RT810_Sys_Info.FILT_CAL_LO=70000;
		RT810_Sys_Info.FILT_GAIN=0x00;  //0dB
		RT810_Sys_Info.HP_COR=0x10;		//1.2M
		RT810_Sys_Info.FILT_I=0x40;		//low
		break;
	case 11:  //DTMB
		RT810_Sys_Info.IF_KHz=4570;
		RT810_Sys_Info.BW=BW_8M;
		RT810_Sys_Info.FILT_CAL_LO=68000;
		RT810_Sys_Info.FILT_GAIN=0x40;  //3dB
		RT810_Sys_Info.HP_COR=0x18;		//1M
		RT810_Sys_Info.FILT_I=0x40;		//low
		break;
	case 12:  //DVB-C 6M
		RT810_Sys_Info.IF_KHz=4063;
		RT810_Sys_Info.BW=BW_6M;
		RT810_Sys_Info.FILT_CAL_LO=72000;
		RT810_Sys_Info.FILT_GAIN=0x40;  //3dB
		RT810_Sys_Info.HP_COR=0x10;		//1.2M
		RT810_Sys_Info.FILT_I=0x40;		//low
		break;
	case 13:  //FM
		RT810_Sys_Info.IF_KHz=6600;
		RT810_Sys_Info.BW=BW_7M;
		RT810_Sys_Info.FILT_CAL_LO=60000;
		RT810_Sys_Info.FILT_GAIN=0xC0;  //9dB
		RT810_Sys_Info.HP_COR=0x00;		//5M
		RT810_Sys_Info.FILT_I=0x00;		//low
		break;
	default:  //DVB_T_8M
		RT810_Sys_Info.IF_KHz=4570;
		RT810_Sys_Info.BW=BW_8M;
		RT810_Sys_Info.FILT_CAL_LO=68000;
		RT810_Sys_Info.FILT_GAIN=0x40;  //3dB
		RT810_Sys_Info.HP_COR=0x18;		//1M
		RT810_Sys_Info.FILT_I=0x40;		//low
		break;
	}
	return RT810_Sys_Info;
}



Freq_Info_Type RT810_Freq_Sel(UINT32 RF_freq)
{


	if(RF_freq>=0 && RF_freq<75000)
	{
		RT810_Freq_Info.RF_MUX = 0x00;  //R5[6]=0		  :RF_MUX
		RT810_Freq_Info.TF_BAND = 0x07;	 //R5[4:0]=00111  :TF_BAND
		RT810_Freq_Info.RFPOLY = 0x08;  //R24[3:2]=10     :POLY_MUX
		RT810_Freq_Info.IMR_MEM = 0;
	}
		
	else if(RF_freq>=75000 && RF_freq<100000)
	{
		RT810_Freq_Info.RF_MUX = 0x00;  //R5[6]=0		  :RF_MUX
		RT810_Freq_Info.TF_BAND = 0x04;	 //R5[4:0]=00100  :TF_BAND
		RT810_Freq_Info.RFPOLY = 0x08;  //R24[3:2]=10	  :POLY_MUX
		RT810_Freq_Info.IMR_MEM = 1;
	}
	else if( RF_freq>=100000 && RF_freq<128000)
	{
		RT810_Freq_Info.RF_MUX = 0x00;  //R5[6]=0		  :RF_MUX
		RT810_Freq_Info.TF_BAND = 0x02;	 //R5[4:0]=00010  :TF_BAND
		RT810_Freq_Info.RFPOLY = 0x08;  //R24[3:2]=10     :POLY_MUX
		RT810_Freq_Info.IMR_MEM = 1;
	}	
	else if( RF_freq>=128000 && RF_freq<177000)
	{
		RT810_Freq_Info.RF_MUX = 0x00;  //R5[6]=0        :RF_MUX
		RT810_Freq_Info.TF_BAND = 0x00;	 //R5[4:0]=00000  :TF_BAND
		RT810_Freq_Info.RFPOLY = 0x04;  //R24[3:2]=01     :POLY_MUX
		RT810_Freq_Info.IMR_MEM = 2;
	}
	else if( RF_freq>=177000 && RF_freq<280000)
	{
		RT810_Freq_Info.RF_MUX = 0x00;  //R5[6]=0		  :RF_MUX
		RT810_Freq_Info.TF_BAND = 0x00;	 //R5[4:0]=0	  :TF_BAND
		RT810_Freq_Info.RFPOLY = 0x04;  //R24[3:2]=01	  :POLY_MUX
		RT810_Freq_Info.IMR_MEM = 2;
	}	
	else if( RF_freq>=280000 && RF_freq<354000)
	{
		RT810_Freq_Info.RF_MUX = 0x40;  //R5[6]=1			:RF_MUX
		RT810_Freq_Info.TF_BAND = 0x00;	 //R5[4:0]=00000	:TF_BAND
		RT810_Freq_Info.RFPOLY = 0x04;  //R24[3:2]=01		:POLY_MUX
		RT810_Freq_Info.IMR_MEM = 2;
	}
	else if( RF_freq>=354000 && RF_freq<576000)
	{
		RT810_Freq_Info.RF_MUX = 0x40;  //R5[6]=1			:RF_MUX
		RT810_Freq_Info.TF_BAND = 0x00;	 //R5[4:0]=00000	:TF_BAND
		RT810_Freq_Info.RFPOLY = 0x04;  //R24[3:2]=01		:POLY_MUX
		RT810_Freq_Info.IMR_MEM = 3;
	}
	else if( RF_freq>=576000 && RF_freq<680000)
	{
		RT810_Freq_Info.RF_MUX = 0x40;  //R5[6]=1		 :RF_MUX
		RT810_Freq_Info.TF_BAND = 0x00;	 //R5[4:0]=00000 :TF_BAND
		RT810_Freq_Info.RFPOLY = 0x00;  //R24[3:2]=00	 :POLY_MUX
		RT810_Freq_Info.IMR_MEM = 4;
	}
	else
	{
		RT810_Freq_Info.RF_MUX = 0x40;  //R5[6]=1		:RF_MUX
		RT810_Freq_Info.TF_BAND = 0x00;	 //R5[4:0]=0	:TF_BAND
		RT810_Freq_Info.RFPOLY = 0x00;  //R24[3:2]=0	:POLY_MUX
		RT810_Freq_Info.IMR_MEM = 5;
	}
	return RT810_Freq_Info;
}



SysFreq_Info_Type RT810_SysFreq_Sel(RT810_Standard_Type RT810_Standard,UINT32 RF_freq)
{
	SysFreq_Info_Type RT810_SysFreq_Info;
	
	switch(RT810_Standard)
	{
	case 0:
		if(RF_freq>0 && RF_freq<280000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x82;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL

		}
		else if(RF_freq>280000 && RF_freq<700000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL
		}
		else if(RF_freq>700000 && RF_freq<800000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x01;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL
		}
		else
		{
			RT810_SysFreq_Info.LNA_TOP=0x00;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL
		
		}
		break;
	case 1:
		if(RF_freq>0 && RF_freq<280000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x82;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL

		}
		else if(RF_freq>280000 && RF_freq<700000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL
		}
		else if(RF_freq>700000 && RF_freq<800000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x01;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL
		}
		else
		{
			RT810_SysFreq_Info.LNA_TOP=0x00;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL
		
		}
		break;
	case 2:
		if(RF_freq>0 && RF_freq<280000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x82;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL

		}
		else if(RF_freq>280000 && RF_freq<700000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL
		}
		else if(RF_freq>700000 && RF_freq<800000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x01;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL
		}
		else
		{
			RT810_SysFreq_Info.LNA_TOP=0x00;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL
		
		}
		break;
	case 3:
		if(RF_freq>0 && RF_freq<280000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x82;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL

		}
		else if(RF_freq>280000 && RF_freq<700000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;		// MIXER VTH   , VTL
		}
		else if(RF_freq>700000 && RF_freq<800000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x01;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL
		}
		else
		{
			RT810_SysFreq_Info.LNA_TOP=0x00;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL
		
		}
		break;
	case 4:
		if(RF_freq>0 && RF_freq<280000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x82;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL

		}
		else if(RF_freq>280000 && RF_freq<700000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL
		}
		else if(RF_freq>700000 && RF_freq<800000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x01;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL
		}
		else
		{
			RT810_SysFreq_Info.LNA_TOP=0x00;		// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x41;		// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;		// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x73;	// MIXER VTH   , VTL
		
		}
		break;
	case 5:
		if(RF_freq>0 && RF_freq<400000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL

		}
		else if(RF_freq>400000 && RF_freq<700000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x01;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		}
		else
		{
			RT810_SysFreq_Info.LNA_TOP=0x00;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		
		}
		break;
	case 6:
		if(RF_freq>0 && RF_freq<400000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL

		}
		else if(RF_freq>400000 && RF_freq<700000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x01;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		}
		else
		{
			RT810_SysFreq_Info.LNA_TOP=0x00;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		
		}
		break;
	case 7:
		if(RF_freq>0 && RF_freq<400000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL

		}
		else if(RF_freq>400000 && RF_freq<700000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x01;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		}
		else
		{
			RT810_SysFreq_Info.LNA_TOP=0x00;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		
		}
		break;
	case 8:
		if(RF_freq>0 && RF_freq<400000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL

		}
		else if(RF_freq>400000 && RF_freq<700000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x01;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		}
		else
		{
			RT810_SysFreq_Info.LNA_TOP=0x00;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		}
		break;
	case 9:
		if(RF_freq>0 && RF_freq<800000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL

		}
		else if(RF_freq>800000 && RF_freq<900000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x01;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		}
		else
		{
			RT810_SysFreq_Info.LNA_TOP=0x00;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		
		}
		break;
	case 10:
		if(RF_freq>0 && RF_freq<500000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x72;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x06;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x85;// MIXER VTH   , VTL

		}
		else 
		{
			RT810_SysFreq_Info.LNA_TOP=0x01;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x72;// LNA VTH  0x72? 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x06;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x85;// MIXER VTH   , VTL
		}

		break;
	case 11:
	if(RF_freq>0 && RF_freq<400000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL

		}
		else if(RF_freq>400000 && RF_freq<700000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x01;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		}
		else
		{
			RT810_SysFreq_Info.LNA_TOP=0x00;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		
		}
		break;
	case 12:
		if(RF_freq>0 && RF_freq<800000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL

		}
		else if(RF_freq>800000 && RF_freq<900000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x01;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		}
		else
		{
			RT810_SysFreq_Info.LNA_TOP=0x00;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x04;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		
		}
		break;
	default:
		if(RF_freq>0 && RF_freq<400000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x02;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL

		}
		else if(RF_freq>400000 && RF_freq<700000)
		{
			RT810_SysFreq_Info.LNA_TOP=0x01;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		}
		else
		{
			RT810_SysFreq_Info.LNA_TOP=0x00;// LNA TOP
			RT810_SysFreq_Info.LNA_VTH_L=0x83;// LNA VTH 	,   VTL
			RT810_SysFreq_Info.MIXER_TOP=0x03;// MIXER TOP
			RT810_SysFreq_Info.MIXER_VTH_L=0x63;// MIXER VTH   , VTL
		
		}
		

	
	}
	
	return RT810_SysFreq_Info;
	
	}



RT810_ErrCode RT810_Init(void)
{



	if(RT810_InitReg() != RT_Success)         //write full register table & write depend on RT810_MUX & write RT810_PLL 
		return RT_Fail;

	if(RT810_IMR_Prepare() != RT_Success)         //write full register table & write depend on RT810_MUX & write RT810_PLL 
		return RT_Fail;
	if(RT810_IMR(3, TRUE) != RT_Success)       //Full K node 3
		return RT_Fail;

	if(RT810_IMR(1, FALSE) != RT_Success)
		return RT_Fail;

	if(RT810_IMR(0, FALSE) != RT_Success)
		return RT_Fail;

	if(RT810_IMR(2, FALSE) != RT_Success)
		return RT_Fail;

	if(RT810_IMR(4, FALSE) != RT_Success)
		return RT_Fail;

	if(RT810_IMR(5, FALSE) != RT_Success)
		return RT_Fail;


	return RT_Success;
}



RT810_ErrCode RT810_InitReg(void)
{
	UINT8 InitArryCunt = 0;
	UINT8 InitArryNum  = 49;
	//UINT32 LO_KHz      = 0;
	
	//Write Full Table
	RT810_I2C_Len.RegAddr = 0x03;
	RT810_I2C_Len.Len     = InitArryNum;
	for(InitArryCunt = 0;InitArryCunt <= InitArryNum;InitArryCunt ++)
	{
		RT810_I2C_Len.Data[InitArryCunt] = RT810_iniArry[InitArryCunt];
	}
	if(I2C_Write_Len_rt810(&RT810_I2C_Len) != RT_Success)
		return RT_Fail;

	return RT_Success;
}


RT810_ErrCode RT810_IMR_Prepare(void)
{
	UINT8 ArrayNum=50;
	for(ArrayNum=0;ArrayNum<50;ArrayNum++)
	{
		RT810_Arry[ArrayNum] = RT810_iniArry[ArrayNum];
	}

	//IMR Preparation	
	//ring pll on, lna off
	RT810_I2C.RegAddr = 0x04;
	RT810_Arry[2]     = (RT810_Arry[2] & 0x3F) | 0x40;
	RT810_I2C.Data    = RT810_Arry[2];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;	
	//ring_mux= from ring
	RT810_I2C.RegAddr = 0x05;
	RT810_Arry[4]     = (RT810_Arry[4] & 0xDF);
	RT810_I2C.Data    = RT810_Arry[4];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;
	//mixer agc off, gain = 12dB
	RT810_I2C.RegAddr = 0x06;
	RT810_Arry[6]     = (RT810_Arry[6] & 0xE0) | 0x0C;
	RT810_I2C.Data    = RT810_Arry[6];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;
	//filter bw=manual, code=1111
	RT810_I2C.RegAddr = 0x0A;
	RT810_Arry[14]    = (RT810_Arry[14] & 0xE0) | 0x1F;
	RT810_I2C.Data    = RT810_Arry[14];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;
	//filter bw=6M, adc=on
	RT810_I2C.RegAddr = 0x0B;
	RT810_Arry[16]    = (RT810_Arry[16] & 0xE5) | 0x18;
	RT810_I2C.Data    = RT810_Arry[16];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;
	//vga code mode, gain=26.5dB
	RT810_I2C.RegAddr = 0x0C;
	RT810_Arry[18]    = (RT810_Arry[18] & 0x40) | 0x2C;
	RT810_I2C.Data    = RT810_Arry[18];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;
	//ring clk = on
	RT810_I2C.RegAddr = 0x0F;
	RT810_Arry[24]   &= 0xF7;
	RT810_I2C.Data    = RT810_Arry[24];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;
	//filter gain = 9dB HP=5M
	RT810_I2C.RegAddr = 0x1A;
	RT810_Arry[46]    = (RT810_Arry[46] & 0x07) | 0xC0;
	RT810_I2C.Data    = RT810_Arry[46];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;
	

	return RT_Success;
}





RT810_ErrCode RT810_IMR(UINT8 IMR_MEM, BOOL IM_Flag)
{
	UINT32 RingFreq = 0;
	UINT32 RingRef = 0;
	UINT8 n_ring = 0;
	UINT8 n;
	
	RT810_SectType IMR_POINT;

	switch(IMR_MEM)
	{
	case 0:
		RingRef = 64000;
		break;
	case 1:
		RingRef = 128000;
		break;
	case 2:
		RingRef = 224000;
		break;
	case 3:
		RingRef = 384000;
		break;
	case 4:
		RingRef = 608000;
		break;
	default:
		RingRef = 672000;
		break;

	}
	for(n=0;n<16;n++)
	{
		if(2*(2+n)*RT810_Xtal>=RingRef)
		{
		n_ring=n;
		break;
		}
	
	}
	RingFreq = (2 * (n_ring+2) * RT810_Xtal);

	RT810_Arry[44] &= 0xF7;      //set ringdiv2
	if(n_ring==0)
	RT810_Arry[44] |= 0x08;



	RT810_Arry[26] &= 0xE0;      //set ring[4:0]
	RT810_Arry[26] |= (UINT8) n_ring;

	RT810_Arry[6]  &= 0x3F;    //R6  bit[7:6] clear (pw_ring[1:0])
	RT810_Arry[42] &= 0x3F;    //R24 bit[7:6] clear (ring_lf[1:0])

	if(RingRef < 96000)
	{
	RT810_Arry[6]  |= 0x00;    //pw_ring[1:0]=00
	RT810_Arry[42] |= 0x00;    //ring_lf[1:0]=00
	}
	else if((RingRef >= 96000) && (RingRef < 224000))
	{
	RT810_Arry[6]  |= 0x00;    //pw_ring[1:0]=00
	RT810_Arry[42] |= 0x40;    //ring_lf[1:0]=01
	}
	else if((RingRef >= 224000) && (RingRef < 480000))
	{
	RT810_Arry[6]  |= 0x00;    //pw_ring[1:0]=00
	RT810_Arry[42] |= 0x80;    //ring_lf[1:0]=10
	}
	else
	{
	RT810_Arry[6]  |= 0x40;    //pw_ring[1:0]=01
	RT810_Arry[42] |= 0xC0;    //ring_lf[1:0]=11
	}

	//write pw_ring,ring_lf,n_ring,ringdiv2 to I2C
	RT810_I2C.RegAddr = 0x06;
	RT810_I2C.Data    = RT810_Arry[6];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x10;
	RT810_I2C.Data    = RT810_Arry[26];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;	

	RT810_I2C.RegAddr = 0x18;
	RT810_I2C.Data    = RT810_Arry[42];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x19;
	RT810_I2C.Data    = RT810_Arry[44];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;




	
	if(RT810_PLL((RingFreq - 5300)) != RT_Success)                  //set pll freq = ring freq - 6M
	return RT_Fail;

	RT810_Delay_MS(2);  //12 ms for ring pll locking

	if(RT810_MUX(RingFreq-5300-4500) != RT_Success)				//MUX input freq ~ RF_in Freq
		return RT_Fail;

	if(IM_Flag == TRUE)
	{
	if(RT810_IQ(&IMR_POINT) != RT_Success)
		return RT_Fail;
	}
	else
	{
		IMR_POINT.Gain_X = IMR_Data_rt810[3].Gain_X;
		IMR_POINT.Phase_Y = IMR_Data_rt810[3].Phase_Y;
		IMR_POINT.Value = IMR_Data_rt810[3].Value;
		
		if(RT810_F_IMR(&IMR_POINT) != RT_Success)
			return RT_Fail;
	}

	//Save IMR Value
	switch(IMR_MEM)
	{
	case 0:
		IMR_Data_rt810[0].Gain_X  = IMR_POINT.Gain_X;
		IMR_Data_rt810[0].Phase_Y = IMR_POINT.Phase_Y;
		IMR_Data_rt810[0].Value   = IMR_POINT.Value;
		break;
	case 1:
		IMR_Data_rt810[1].Gain_X  = IMR_POINT.Gain_X;
		IMR_Data_rt810[1].Phase_Y = IMR_POINT.Phase_Y;
		IMR_Data_rt810[1].Value   = IMR_POINT.Value;
		break;
	case 2:
		IMR_Data_rt810[2].Gain_X  = IMR_POINT.Gain_X;
		IMR_Data_rt810[2].Phase_Y = IMR_POINT.Phase_Y;
		IMR_Data_rt810[2].Value   = IMR_POINT.Value;
		break;
	case 3:
		IMR_Data_rt810[3].Gain_X  = IMR_POINT.Gain_X;
		IMR_Data_rt810[3].Phase_Y = IMR_POINT.Phase_Y;
		IMR_Data_rt810[3].Value   = IMR_POINT.Value;
		break;
	case 4:
		IMR_Data_rt810[4].Gain_X  = IMR_POINT.Gain_X;
		IMR_Data_rt810[4].Phase_Y = IMR_POINT.Phase_Y;
		IMR_Data_rt810[4].Value   = IMR_POINT.Value;
		break;
	default:
		IMR_Data_rt810[5].Gain_X  = IMR_POINT.Gain_X;
		IMR_Data_rt810[5].Phase_Y = IMR_POINT.Phase_Y;
		IMR_Data_rt810[5].Value   = IMR_POINT.Value;
		break;
	}

	return RT_Success;
}



RT810_ErrCode RT810_PLL(UINT32 LO_Freq)
{
	UINT8  MixDiv   = 2;
	UINT8  DivBuf   = 0;
	UINT8  Ni       = 0;
	UINT8  SDM_Cunt = 0;
	UINT8  SDM_B    = 0;
	UINT8  SDM      = 0;
	UINT8  DivNum   = 0;
	UINT8  Judge    = 0;
	UINT32 N        = 0;
	UINT32 Div_1    = 0;
	UINT32 Div_2    = 0;//NF_1
	UINT32 VCO_Min  = 1416000;
	UINT32 VCO_Freq = 0;
	UINT32 Bry_M    = 1;
	UINT32 Bry_L    = 0;
	UINT32 Si       = 0;
	

	UINT32 PLL_Ref;
	if(RT810_Xtal > 24000)
	{
		PLL_Ref = RT810_Xtal /2;
		RT810_Arry[42] |= 0x20;
	}
	else
	{
		PLL_Ref = RT810_Xtal;
	}

	RT810_I2C.RegAddr = 0x18;
    RT810_I2C.Data = RT810_Arry[42];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
	return RT_Fail;



	//Divider
	while(MixDiv <= 32)
	{
		if(((LO_Freq * MixDiv) >= VCO_Min) && ((LO_Freq * MixDiv) < (VCO_Min * 2)))
		{
			DivBuf = MixDiv;
			while(DivBuf > 1)
			{
				DivBuf /= 2;
				DivNum ++;
			}
			DivNum --;
			
			RT810_I2C.RegAddr = 0x10;
			RT810_Arry[26] &= 0x1F;
			RT810_Arry[26] |= (DivNum << 5);
			RT810_I2C.Data = RT810_Arry[26];
			if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
				return RT_Fail;
			
			break;
		}
		MixDiv *= 2;
	}

	VCO_Freq = LO_Freq * MixDiv;
	Div_1    = VCO_Freq / PLL_Ref;
	Div_2    = VCO_Freq - (Div_1 * PLL_Ref);
	Div_2    = (Div_2 * 100000) / PLL_Ref;
	N        = Div_1 / 2;

	if((Div_1 - (N * 2)) != 0)
		Div_2 += 100000;
	Div_2    /= 2;

	if(Div_2 < 781)
		Div_2 = 0;
	else if((Div_2 > 49609) && (Div_2 < 50000))
		Div_2 = 49609;
	else if((Div_2 >= 50000) && (Div_2 < 50390))
		Div_2 = 50390;
	else if((99218 < Div_2) && (100000))
	{
		Div_2 = 0;
		N ++;
	}

	Ni       = (UINT8)(N - 13) / 4;
	Si       = ((N * 1000 - 13000) / 4) - (Ni * 1000);
	Si       = (Si * 4) / 1000;
	RT810_I2C.RegAddr = 0x14;
	RT810_Arry[34]  = 0x00;
	RT810_Arry[34] |= (Ni + (Si << 6));
	RT810_I2C.Data = RT810_Arry[34];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;
	
	RT810_I2C.RegAddr = 0x12;
	RT810_Arry[30] &= 0xE7;
	if(Div_2 == 0)
		RT810_Arry[30] |= 0x08;

	RT810_I2C.Data = RT810_Arry[30];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	SDM    = 0;
	SDM_B  = 0x80;
	Bry_M  = 50000;
	Bry_L  = 0;

	if(Div_2 >= Bry_M)
	{
		SDM = SDM_B;
		Div_2 -= Bry_M;
	}
	SDM_B /= 2;
	Bry_M /= 2;

	for(SDM_Cunt = 0;SDM_Cunt < 14;SDM_Cunt ++)
	{
		if(Div_2 >= Bry_M)
		{
			SDM += SDM_B;
			Div_2 -= Bry_M;
		}
		
		if(SDM_Cunt == 6)
		{
			RT810_I2C.RegAddr = 0x16;
			RT810_Arry[38]    = SDM;
			RT810_I2C.Data    = RT810_Arry[38];
			if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
				return RT_Fail;
			
			SDM_B = 0x80;
			SDM   = 0x00;
			Bry_M /= 2;
			if(Div_2 > Bry_M)
			{
				SDM += SDM_B;
				Div_2 -= Bry_M;
			}
		}

		SDM_B /= 2;
		Bry_M /= 2;
	}

	RT810_I2C.RegAddr = 0x15;
	RT810_Arry[36]    = SDM;
	RT810_I2C.Data    = RT810_Arry[36];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	//PLL Lock Judgement
	Judge = 0;
	RT810_Delay_MS(12);

	RT810_I2C_Len.RegAddr = 0x00;
	RT810_I2C_Len.Len     = 3;
	if(I2C_Read_Len_rt810(&RT810_I2C_Len) != RT_Success)
		return RT_Fail;

	Judge = RT810_I2C_Len.Data[2];

	RT810_I2C_Len.RegAddr = 0x00;
	RT810_I2C_Len.Len     = 3;
	if(I2C_Read_Len_rt810(&RT810_I2C_Len) != RT_Success)
		return RT_Fail;

	if((RT810_I2C_Len.Data[2] == 0x00) || (RT810_I2C_Len.Data[2] == 0x2F)
		|| (RT810_I2C_Len.Data[2] != Judge))
	{
		RT810_I2C.RegAddr = 0x12;
		
		RT810_Arry[30] |= 0x02;
		RT810_I2C.Data = RT810_Arry[30];
		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;

		RT810_Arry[30] &= 0xFD;
		RT810_I2C.Data = RT810_Arry[30];
		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;

		RT810_Delay_MS(12);
	}
	else
		return RT_Success;

	Judge = 0;

	RT810_I2C_Len.RegAddr = 0x00;
	RT810_I2C_Len.Len     = 3;
	if(I2C_Read_Len_rt810(&RT810_I2C_Len) != RT_Success)
		return RT_Fail;

	Judge = RT810_I2C_Len.Data[2];

	RT810_I2C_Len.RegAddr = 0x00;
	RT810_I2C_Len.Len     = 3;
	if(I2C_Read_Len_rt810(&RT810_I2C_Len) != RT_Success)
		return RT_Fail;

	if((RT810_I2C_Len.Data[2] == 0x00) || (RT810_I2C_Len.Data[2] == 0x2F)
		|| (RT810_I2C_Len.Data[2] != Judge))
	{
		RT810_I2C.RegAddr = 0x11;
		RT810_Arry[27] &= 0xFC;
		RT810_I2C.Data = RT810_Arry[27];
		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;

		RT810_Delay_MS(12);
	}
	else
		return RT_Success;

	Judge = 0;

	RT810_I2C_Len.RegAddr = 0x00;
	RT810_I2C_Len.Len     = 3;
	if(I2C_Read_Len_rt810(&RT810_I2C_Len) != RT_Success)
		return RT_Fail;

	Judge = RT810_I2C_Len.Data[2];

	RT810_I2C_Len.RegAddr = 0x00;
	RT810_I2C_Len.Len     = 3;
	if(I2C_Read_Len_rt810(&RT810_I2C_Len) != RT_Success)
		return RT_Fail;

	if((RT810_I2C_Len.Data[2] == 0x00) || (RT810_I2C_Len.Data[2] == 0x2F)
		|| (RT810_I2C_Len.Data[2] != Judge))
	{
		RT810_Arry[28] &= 0x1F;
		RT810_Arry[28] |= 0x20;
		if(I2C_Read_Len_rt810(&RT810_I2C_Len) != RT_Success)
			return RT_Fail;
	}
	else
		return RT_Success;

	Judge = 0;

	RT810_I2C_Len.RegAddr = 0x00;
	RT810_I2C_Len.Len     = 3;
	if(I2C_Read_Len_rt810(&RT810_I2C_Len) != RT_Success)
		return RT_Fail;

	Judge = RT810_I2C_Len.Data[2];

	RT810_I2C_Len.RegAddr = 0x00;
	RT810_I2C_Len.Len     = 3;
	if(I2C_Read_Len_rt810(&RT810_I2C_Len) != RT_Success)
		return RT_Fail;

	if((RT810_I2C_Len.Data[2] == 0x00) || (RT810_I2C_Len.Data[2] == 0x2F)
		|| (RT810_I2C_Len.Data[2] != Judge))
		return RT_Fail;

	return RT_Success;
}



RT810_ErrCode RT810_MUX(UINT32 RF_KHz)
{	
	Freq_Info_Type Freq_Info1;
	UINT8 RT_Reg08   = 0;
	UINT8 RT_Reg09   = 0;

    Freq_Info1 = RT810_Freq_Sel(RF_KHz);
	//TF Band , RF_MUX
	RT810_Arry[4] = (RT810_Arry[4] & 0xA0) | Freq_Info1.RF_MUX | Freq_Info1.TF_BAND;
	RT810_I2C.RegAddr = 0x05;
	RT810_I2C.Data = RT810_Arry[4];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	//Polymux
	RT810_I2C.RegAddr = 0x18;
	RT810_Arry[42] &= 0xF3;
	RT810_Arry[42] |= Freq_Info1.RFPOLY;	
	RT810_I2C.Data = RT810_Arry[42];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;
	
	//Set_IMR
	RT_Reg08 = IMR_Data_rt810[Freq_Info1.IMR_MEM].Gain_X;
	RT_Reg09 = IMR_Data_rt810[Freq_Info1.IMR_MEM].Phase_Y;

	RT810_I2C.RegAddr = 0x08;
	RT810_Arry[10] &= 0xC0;
	RT810_Arry[10] = RT_Reg08 | RT810_Arry[10];
	RT810_I2C.Data = RT810_Arry[10];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x09;
	RT810_Arry[12] &= 0xC0;
	RT810_Arry[12] = RT_Reg09 | RT810_Arry[12];
	RT810_I2C.Data =RT810_Arry[12]  ;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	return RT_Success;
}




RT810_ErrCode RT810_IQ(RT810_SectType* IQ_Pont)
{
	RT810_SectType Compare_IQ[3];
	RT810_SectType CompareTemp;
	UINT8 IQ_Cunt  = 0;
	UINT8 VGA_Cunt = 0;
	UINT16 VGA_Read = 0;
		
	//VGA
	for(VGA_Cunt = 12;VGA_Cunt < 16;VGA_Cunt ++)
	{
		RT810_I2C.RegAddr = 0x0C;
		RT810_I2C.Data    = VGA_Cunt * 4;
		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;

		RT810_Delay_MS(10);
		
		if(RT810_Muti_Read(0x01, &VGA_Read) != RT_Success)
			return RT_Fail;
		if(VGA_Read > 0xA0)
			break;
	}

	Compare_IQ[0].Gain_X  = 0x40;
	Compare_IQ[0].Phase_Y = 0x40;

	//while(IQ_Cunt < 3)
	//{
		//X
		if(RT810_IQ_Tree(Compare_IQ[IQ_Cunt].Phase_Y, Compare_IQ[IQ_Cunt].Gain_X, 0x09, &Compare_IQ[0]) != RT_Success)
			return RT_Fail;

		if(RT810_CompreCor(&Compare_IQ[0]) != RT_Success)
			return RT_Fail;

		if(RT810_CompreStep(&Compare_IQ[0], 0x08) != RT_Success)
			return RT_Fail;
		
		//Y
		if(RT810_IQ_Tree(Compare_IQ[IQ_Cunt].Gain_X, Compare_IQ[IQ_Cunt].Phase_Y, 0x08, &Compare_IQ[0]) != RT_Success)
			return RT_Fail;

		if(RT810_CompreCor(&Compare_IQ[0]) != RT_Success)
			return RT_Fail;

		if(RT810_CompreStep(&Compare_IQ[0], 0x09) != RT_Success)
			return RT_Fail;

		//CompareTemp = Compare_IQ[0];

		//Check X
		if(RT810_IQ_Tree(Compare_IQ[IQ_Cunt].Phase_Y, Compare_IQ[IQ_Cunt].Gain_X, 0x09, &Compare_IQ[0]) != RT_Success)
			return RT_Fail;

		if(RT810_CompreCor(&Compare_IQ[0]) != RT_Success)
			return RT_Fail;

		//if((CompareTemp.Gain_X == Compare_IQ[0].Gain_X) && (CompareTemp.Phase_Y == Compare_IQ[0].Phase_Y))//Ben Check
		//	break;
		
		//IQ_Cunt ++;
	//}
	//if(IQ_Cunt ==  3)
	//	return RT_Fail;

	//Section Check
	CompareTemp = Compare_IQ[0];
	for(IQ_Cunt = 0;IQ_Cunt < 5;IQ_Cunt ++)
	{
		if(RT810_Section(&Compare_IQ[0]) != RT_Success)
			return RT_Fail;

		if((CompareTemp.Gain_X == Compare_IQ[0].Gain_X) && (CompareTemp.Phase_Y == Compare_IQ[0].Phase_Y))
			break;
	}
	*IQ_Pont = Compare_IQ[0];

	RT810_I2C.RegAddr = 0x08;
	RT810_I2C.Data    = 0x40;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x09;
	RT810_I2C.Data    = 0x40;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	return RT_Success;
}


RT810_ErrCode RT810_IQ_Tree(UINT8 FixPot, UINT8 FlucPot, UINT8 PotReg, RT810_SectType* CompareTree)
{
	UINT8 TreeCunt  = 0;
	UINT8 TreeTimes = 3;
	UINT8 TempPot   = 0;
	UINT8 PntReg    = 0;
		
	if(PotReg == 0x08)
		PntReg = 0x09;
	else
		PntReg = 0x08;

	for(TreeCunt = 0;TreeCunt < TreeTimes;TreeCunt ++)
	{
		RT810_I2C.RegAddr = PotReg;
		RT810_I2C.Data    = FixPot;
		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;

		RT810_I2C.RegAddr = PntReg;
		RT810_I2C.Data    = FlucPot;
		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;
	
		if(RT810_Muti_Read(0x01, &CompareTree[TreeCunt].Value) != RT_Success)
			return RT_Fail;
		
		if(PotReg == 0x08)
		{
			CompareTree[TreeCunt].Gain_X  = FixPot;
			CompareTree[TreeCunt].Phase_Y = FlucPot;
		}
		else
		{
			CompareTree[TreeCunt].Phase_Y  = FixPot;
			CompareTree[TreeCunt].Gain_X = FlucPot;
		}
		
		if(TreeCunt == 0)
			FlucPot ++;
		else if(TreeCunt == 1)
		{
			if((FlucPot & 0x0F) < 0x02)
			{
				TempPot = 2 - (FlucPot & 0x0F);
				if(FlucPot & 0x20)
				{
					FlucPot &= 0xC0;
					FlucPot |= TempPot;
				}
				else
				{
					FlucPot |= 0x20 | TempPot;
				}
			}
			else
				FlucPot -= 2;
		}
	}

	return RT_Success;
}


RT810_ErrCode RT810_CompreCor(RT810_SectType* CorArry)
{
	UINT8 CompCunt = 0;
	RT810_SectType CorTemp;

	for(CompCunt = 3;CompCunt > 0;CompCunt --)
	{
		if(CorArry[0].Value > CorArry[CompCunt - 1].Value)
		{
			CorTemp = CorArry[0];
			CorArry[0] = CorArry[CompCunt - 1];
			CorArry[CompCunt - 1] = CorTemp;
		}
	}

	return RT_Success;
}



RT810_ErrCode RT810_CompreStep(RT810_SectType* StepArry, UINT8 Pace)
{
	//UINT8 StepCunt = 0;
	RT810_SectType StepTemp;
	
	StepTemp.Phase_Y = StepArry[0].Phase_Y;
	StepTemp.Gain_X  = StepArry[0].Gain_X;

	while(((StepTemp.Gain_X & 0x0F) < 5) || ((StepTemp.Phase_Y & 0x0F) < 5))
	{
		if(Pace == 0x08)
			StepTemp.Gain_X ++;
		else
			StepTemp.Phase_Y ++;

		if(((StepTemp.Gain_X & 0x1F) > 10) || ((StepTemp.Phase_Y & 0x1F) > 10))
		{
			StepTemp.Gain_X  = 0x40;
			StepTemp.Phase_Y = 0x40;
			
			RT810_I2C.RegAddr = 0x08;
			RT810_I2C.Data    = StepTemp.Gain_X;
			if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
				return RT_Fail;

			RT810_I2C.RegAddr = 0x09;
			RT810_I2C.Data    = StepTemp.Phase_Y;
			if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
				return RT_Fail;

			if(RT810_Muti_Read(0x01, &StepTemp.Value) != RT_Success)
				return RT_Fail;

			StepArry[0].Gain_X  = StepTemp.Gain_X;
			StepArry[0].Phase_Y = StepTemp.Phase_Y;
			StepArry[0].Value   = StepTemp.Value;
			break;
		}
		
		RT810_I2C.RegAddr = 0x08;
		RT810_I2C.Data    = StepTemp.Gain_X ;
		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;

		RT810_I2C.RegAddr = 0x09;
		RT810_I2C.Data    = StepTemp.Phase_Y;
		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;

		if(RT810_Muti_Read(0x01, &StepTemp.Value) != RT_Success)
			return RT_Fail;

		if(StepTemp.Value <= StepArry[0].Value)
		{
			StepArry[0].Gain_X  = StepTemp.Gain_X;
			StepArry[0].Phase_Y = StepTemp.Phase_Y;
			StepArry[0].Value   = StepTemp.Value;
		}
		else
			break;		
	}
	
	return RT_Success;
}


RT810_ErrCode RT810_Muti_Read(UINT8 IMR_Reg, UINT16* IMR_Data_rt810)
{
	UINT8 ReadCunt     = 0;
	UINT16 ReadAmount  = 0;
	
	for(ReadCunt = 0;ReadCunt < 6;ReadCunt ++)
	{
		RT810_I2C_Len.RegAddr = 0x00;
		RT810_I2C_Len.Len     = IMR_Reg + 1;
		if(I2C_Read_Len_rt810(&RT810_I2C_Len) != RT_Success)
			return RT_Fail;

		if((ReadCunt != 0) && (ReadCunt != 6))
		{
			ReadAmount += RT810_I2C_Len.Data[1];
		}
	}
	*IMR_Data_rt810 = ReadAmount;

	return RT_Success;
}




RT810_ErrCode RT810_Section(RT810_SectType* SectionArry)
{
	UINT8 SectionCunt  = 0;
	UINT8 SectionTimes = 4;
	RT810_SectType SectionTemp;

	SectionTemp.Gain_X  = SectionArry[0].Gain_X;
	SectionTemp.Phase_Y = SectionArry[0].Phase_Y;

	for(SectionCunt = 0;SectionCunt < SectionTimes;SectionCunt ++)
	{
		if(SectionCunt < 2)
		{
			if((SectionArry[0].Gain_X & 0x0F) < 1)
			{
				if(SectionArry[0].Gain_X & 0x20)
					SectionTemp.Gain_X &= 0xDF;
				else
					SectionTemp.Gain_X |= 0x20;

				SectionTemp.Gain_X = (SectionTemp.Gain_X & 0xF0) + 1;
			}
			else
			SectionTemp.Gain_X = SectionArry[0].Gain_X - 1;
		}
		else
			SectionTemp.Gain_X = SectionArry[0].Gain_X + 1;

		if((SectionCunt - ((SectionCunt / 2) * 2)) != 0)
		{
			if((SectionArry[0].Phase_Y & 0x0F) >= 1)
				SectionTemp.Phase_Y = SectionArry[0].Phase_Y - 1;
			else
			{
				SectionTemp.Phase_Y = SectionArry[0].Phase_Y;
				if(SectionArry[0].Phase_Y & 0x20)
					SectionTemp.Phase_Y &= 0xDF;
				else
					SectionTemp.Phase_Y |= 0x20;
				SectionTemp.Phase_Y ++;
			}
		}
		else
			SectionTemp.Phase_Y = SectionArry[0].Phase_Y + 1;

		RT810_I2C.RegAddr = 0x08;
		RT810_I2C.Data    = SectionTemp.Gain_X ;
		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;

		RT810_I2C.RegAddr = 0x09;
		RT810_I2C.Data    = SectionTemp.Phase_Y;
		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;

		if(RT810_Muti_Read(0x01, &SectionTemp.Value) != RT_Success)
			return RT_Fail;

		if(SectionTemp.Value < SectionArry[0].Value)
		{
			SectionArry[0].Gain_X  = SectionTemp.Gain_X;
			SectionArry[0].Phase_Y = SectionTemp.Phase_Y;
			SectionArry[0].Value   = SectionTemp.Value;
		}
	}

	return RT_Success;
}



RT810_ErrCode RT810_F_IMR(RT810_SectType* IQ_Pont)
{
	RT810_SectType Compare_IQ[3];
	RT810_SectType Compare_Bet[3];
	UINT8 VGA_Cunt = 0;
	UINT16 VGA_Read = 0;

	//VGA
	for(VGA_Cunt = 12;VGA_Cunt < 16;VGA_Cunt ++)
	{
		RT810_I2C.RegAddr = 0x0C;
		RT810_I2C.Data    = VGA_Cunt * 4;
		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;

		RT810_Delay_MS(10);
		
		if(RT810_Muti_Read(0x01, &VGA_Read) != RT_Success)
			return RT_Fail;
		if(VGA_Read > 32*5)
		break;
	}

	if((IQ_Pont->Gain_X & 0x1F) == 0x00)
	{
		if((IQ_Pont->Gain_X & 0xE0) == 0x40)
			Compare_IQ[0].Gain_X = 0x61;
		else
			Compare_IQ[0].Gain_X = 0x41;
	}
	else
		Compare_IQ[0].Gain_X  = IQ_Pont->Gain_X - 1;
		Compare_IQ[0].Phase_Y = IQ_Pont->Phase_Y;

	//Y
	if(RT810_IQ_Tree(Compare_IQ[0].Gain_X, Compare_IQ[0].Phase_Y, 0x08, &Compare_IQ[0]) != RT_Success)
		return RT_Fail;

	if(RT810_CompreCor(&Compare_IQ[0]) != RT_Success)
		return RT_Fail;

	Compare_Bet[0].Gain_X = Compare_IQ[0].Gain_X;
	Compare_Bet[0].Phase_Y = Compare_IQ[0].Phase_Y;
	Compare_Bet[0].Value = Compare_IQ[0].Value;

	Compare_IQ[0].Gain_X = IQ_Pont->Gain_X;
	Compare_IQ[0].Phase_Y = IQ_Pont->Phase_Y;

	if(RT810_IQ_Tree(Compare_IQ[0].Gain_X, Compare_IQ[0].Phase_Y, 0x08, &Compare_IQ[0]) != RT_Success)
		return RT_Fail;

	if(RT810_CompreCor(&Compare_IQ[0]) != RT_Success)
		return RT_Fail;

	Compare_Bet[1].Gain_X = Compare_IQ[0].Gain_X;
	Compare_Bet[1].Phase_Y = Compare_IQ[0].Phase_Y;
	Compare_Bet[1].Value = Compare_IQ[0].Value;

	Compare_IQ[0].Gain_X = IQ_Pont->Gain_X + 1;
	Compare_IQ[0].Phase_Y = IQ_Pont->Phase_Y;

	if(RT810_IQ_Tree(Compare_IQ[0].Gain_X, Compare_IQ[0].Phase_Y, 0x08, &Compare_IQ[0]) != RT_Success)
		return RT_Fail;

	if(RT810_CompreCor(&Compare_IQ[0]) != RT_Success)
		return RT_Fail;

	Compare_Bet[2].Gain_X = Compare_IQ[0].Gain_X;
	Compare_Bet[2].Phase_Y = Compare_IQ[0].Phase_Y;
	Compare_Bet[2].Value = Compare_IQ[0].Value;

	if(RT810_CompreCor(&Compare_Bet[0]) != RT_Success)
		return RT_Fail;

	*IQ_Pont = Compare_Bet[0];
	
	return RT_Success;
}



RT810_ErrCode RT810_GPIO(RT810_GPIO_Type RT810_GPIO_Conrl)
{
	if(RT810_GPIO_Conrl == HI_SIG)
		RT810_Arry[24] |= 0x01;
	else
		RT810_Arry[24] &= 0xFE;

	RT810_I2C.RegAddr = 0x0F;
	RT810_I2C.Data    = RT810_Arry[24];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	return RT_Success;
}





RT810_ErrCode RT810_Standard(RT810_Standard_Type RT_Standard)
{
	UINT8 ArrayNum=50;
	
	if(RT810_InitReg() != RT_Success)
	{
		return RT_Fail;	 
	}	

	// Used Normal Arry to Modify
	
	for(ArrayNum=0; ArrayNum<50; ArrayNum++)
	{
		RT810_Arry[ArrayNum] = RT810_iniArry[ArrayNum];
	}

	// Look Up System Dependent Table

	Sys_Info1 = RT810_Sys_Sel(RT_Standard);

	// Filter Calibration
	if(RT810_Filt_Cal(Sys_Info1.FILT_CAL_LO) != RT_Success)
	return RT_Fail;

	// Set BW,
	switch (Sys_Info1.BW)
	{
	case 0:  //6M
		RT810_Arry[16]    &= 0xE7;
		RT810_Arry[16]    |= 0x18;  //v7m=1,v6m=1
		break;
	case 1:  //7M
		RT810_Arry[16]    &= 0xE7;
		RT810_Arry[16]    |= 0x10;  //v7m=1,v6m=0, 
		break;
	case 2:  //8M
		RT810_Arry[16]    &= 0xE7;
		RT810_Arry[16]    |= 0x00;  //v7m=0,v6m=0,
		break;
	default:  //8M
		RT810_Arry[16]    &= 0xE7;
		RT810_Arry[16]    |= 0x00;  //v7m=0,v6m=0, 
		break;
	}
	RT810_I2C.RegAddr  = 0x0B;
	RT810_I2C.Data     = RT810_Arry[16];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
	{
		return RT_Fail;
	}	

	// Set Filter_gain, & HP corner
	RT810_Arry[46]  = (RT810_Arry[46] & 0x03) | Sys_Info1.FILT_GAIN | Sys_Info1.HP_COR;  
	RT810_I2C.RegAddr  = 0x1A;
	RT810_I2C.Data     = RT810_Arry[46];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
	{
		return RT_Fail;
	}	
	
	// Set Filter Current
	RT810_Arry[14]  = (RT810_Arry[14] & 0x9F) | Sys_Info1.FILT_I;  
	RT810_I2C.RegAddr  = 0x0A;
	RT810_I2C.Data     = RT810_Arry[14];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
	{
		return RT_Fail;
	}	
	
	return RT_Success;
}


RT810_ErrCode RT810_Filt_Cal(UINT32 Cal_Freq)
{
	//channel filter gain mode = manual (initial already done)
	//filter =on, filter power= low, enb_tune= auto
	RT810_I2C.RegAddr = 0x0A;
	RT810_Arry[14]    = (RT810_Arry[14] & 0x0F) | 0x40;
	RT810_I2C.Data    = RT810_Arry[14];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;
	//v7M,v8M = 8M, vstart=off, pw_adc=off (initial already done)
	//set cali clk =on
	RT810_I2C.RegAddr = 0x0F;  //reg15
	RT810_Arry[24]   |= 0x04;  //calibration clk=on
	RT810_I2C.Data    = RT810_Arry[24];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;
	//Set PLL Freq = Filter Cali Freq
	if(RT810_PLL(Cal_Freq) != RT_Success)
			return RT_Fail;

	//Start Trigger
	RT810_I2C.RegAddr = 0x0B;  //reg11
	RT810_Arry[16]   |= 0x04;	//vstart=1	
	RT810_I2C.Data    = RT810_Arry[16];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;
	//delay 0.5ms
	RT810_Delay_MS(1);
	//Stop Trigger
	RT810_I2C.RegAddr = 0x0B;
	RT810_Arry[16]   &= 0xFB;
	RT810_I2C.Data    = RT810_Arry[16];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;
	//set cali clk =off
	RT810_I2C.RegAddr  = 0x0F;
	RT810_Arry[24]    &= 0xFB;
	RT810_I2C.Data     = RT810_Arry[24];
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	return RT_Success;
}



RT810_ErrCode RT810_SetFrequency(RT810_Set_Info RT810_INFO)

{

	 UINT32	LO_KHz;
     //Set Freq Depend Variable

     if(RT810_MUX(RT810_INFO.RF_KHz) != RT_Success)

     return RT_Fail;

     //Set TOP,VTH,VTL

     SysFreq_Info1 = RT810_SysFreq_Sel(RT810_INFO.RT810_Standard, RT810_INFO.RF_KHz);



     Sys_Info1 = RT810_Sys_Sel(RT810_INFO.RT810_Standard);

     // write LNA TOP

     RT810_Arry[2] = (RT810_Arry[2] & 0xF8) | SysFreq_Info1.LNA_TOP;

     RT810_I2C.RegAddr     = 0x04;

     RT810_I2C.Data = RT810_Arry[2];

     if(I2C_Write_rt810(&RT810_I2C) != RT_Success)

           return RT_Fail;

     // write MIXER TOP

     RT810_Arry[8] = (RT810_Arry[8] & 0xF0) | SysFreq_Info1.MIXER_TOP;

     RT810_I2C.RegAddr     = 0x07;

     RT810_I2C.Data = RT810_Arry[8];

     if(I2C_Write_rt810(&RT810_I2C) != RT_Success)

           return RT_Fail;

     // write LNA VTHL

     RT810_Arry[20] = (RT810_Arry[20] & 0x00) | SysFreq_Info1.LNA_VTH_L;

     RT810_I2C.RegAddr     = 0x0D;

     RT810_I2C.Data = RT810_Arry[20];

     if(I2C_Write_rt810(&RT810_I2C) != RT_Success)

           return RT_Fail;

     // wrute MIXER VTHL

     RT810_Arry[22] = (RT810_Arry[22] & 0x00) | SysFreq_Info1.MIXER_VTH_L;

     RT810_I2C.RegAddr     = 0x0E;

     RT810_I2C.Data = RT810_Arry[22];

     if(I2C_Write_rt810(&RT810_I2C) != RT_Success)

           return RT_Fail;

 

     //RT810 Cable In or Air In

     if(RT810_INFO.InputMode == AIR_IN)

           RT810_Arry[0] |= 0x20;

     else

           RT810_Arry[0] &= 0xDF;

     //Loop Through

     if(RT810_INFO.RT_Input == LOOP_THROUGH)

           RT810_Arry[0] &= 0x3F;

     else

           RT810_Arry[0] |= 0xC0;

	 RT810_I2C.RegAddr     = 0x03;

     RT810_I2C.Data = RT810_Arry[0];

     if(I2C_Write_rt810(&RT810_I2C) != RT_Success)

           return RT_Fail;

 

	 LO_KHz = RT810_INFO.RF_KHz + Sys_Info1.IF_KHz;

     if(RT810_PLL(LO_KHz) != RT_Success)

     return RT_Fail;

     

     return RT_Success;

}




RT810_ErrCode RT810_Standby(RT810_LoopThrough_Type RT810_LoopSwitch, RT810_Input_Type RT810_LoopIN)
{
	if(RT810_LoopSwitch == LOOP_THROUGH)
	{
		RT810_I2C.RegAddr = 0x03;

		if(RT810_LoopIN == CABLE_IN)
			RT810_I2C.Data = 0x5B;
		else
			RT810_I2C.Data = 0x7B;

		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;

		RT810_I2C.RegAddr = 0x04;
		RT810_I2C.Data    = 0x92;
		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;
	}
	else
	{
		RT810_I2C.RegAddr = 0x03;
		RT810_I2C.Data    = 0xDB;
		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;

		RT810_I2C.RegAddr = 0x04;
		RT810_I2C.Data    = 0xD2;
		if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
			return RT_Fail;
	}

	RT810_I2C.RegAddr = 0x05;
	RT810_I2C.Data    = 0x3F;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x07;
	RT810_I2C.Data    = 0xB4;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x08;
	RT810_I2C.Data    = 0xC0;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x09;
	RT810_I2C.Data    = 0xC0;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x0A;
	RT810_I2C.Data    = 0xCE;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x0B;
	RT810_I2C.Data    = 0x13;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x0C;
	RT810_I2C.Data    = 0xCC;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x11;
	RT810_I2C.Data    = 0x14;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x12;
	RT810_I2C.Data    = 0xE2;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x17;
	RT810_I2C.Data    = 0x03;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x18;
	RT810_I2C.Data    = 0xCD;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x19;
	RT810_I2C.Data    = 0x73;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;

	RT810_I2C.RegAddr = 0x1A;
	RT810_I2C.Data    = 0x54;
	if(I2C_Write_rt810(&RT810_I2C) != RT_Success)
		return RT_Fail;
	
	return RT_Success;
}





//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                      Smart GUI                               //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
UINT8 RT810_IMR_XT[6];
UINT8 RT810_IMR_YT[6];
RT810_ErrCode SmartGUIFunction(void)
{
	UINT8 IMR_C = 0;
	UINT8 IMR_X[6] = {0, 0, 0, 0, 0, 0};
	UINT8 IMR_Y[6] = {0, 0, 0, 0, 0, 0};


	for(IMR_C = 0;IMR_C < 6;IMR_C ++)
	{
		IMR_X[IMR_C] = IMR_Data_rt810[IMR_C].Gain_X;
		IMR_Y[IMR_C] = IMR_Data_rt810[IMR_C].Phase_Y;

		RT810_IMR_XT[IMR_C] = IMR_X[IMR_C];
		RT810_IMR_YT[IMR_C] = IMR_Y[IMR_C];
	}
	
	return RT_Success;
}

