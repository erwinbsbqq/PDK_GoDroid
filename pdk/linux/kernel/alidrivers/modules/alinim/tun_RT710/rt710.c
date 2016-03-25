//#include "stdafx.h"
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "rt710.h"

//UINT8 RT710_ADDRESS=0xF4;
//I2C_TYPE RT710_Write_Byte;
//I2C_LEN_TYPE RT710_Write_Len;
//I2C_LEN_TYPE RT710_Read_Len;

//#ifndef I2C_FOR_VZ7306
//#define I2C_FOR_VZ7306 	I2C_TYPE_SCB0
//#endif
//#define RT710_PRINTF(...)

/*#define I2cWriteRegs(Addr, rAddr, lpbData, bLen) i2c_write(I2C_FOR_VZ7306, Addr, lpbData, bLen)
#define I2cReadReg(Addr, rAddr, lpbData) i2c_read(I2C_FOR_VZ7306, Addr, lpbData, 16)*/


struct QPSK_TUNER_CONFIG_EXT * rt710_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 rt710_tuner_cnt = 0;

int Tuner_I2C_write(UINT32 tuner_id, unsigned char reg_start, unsigned char* buff, unsigned char length);
int Tuner_I2C_read(UINT32 tuner_id, unsigned char reg_start, unsigned char* buff, unsigned char length);



#if(RT710_0DBM_SETTING == TRUE)
	//0dBm ; LT:lna output ; LT:HPF off  ; LT Current High
	static unsigned char RT710_Reg_Arry_Init[RT710_Reg_Num] ={0x40, 0x1C, 0xA0, 0x90, 0x41, 0x50, 0xED, 0x25, 0x47, 0x58, 0x38, 0x60, 0x38, 0xE7, 0x90, 0x35};
#else
	//-10~-30dBm ; LT:lna center ; LT:HPF on  ; LT Current Low
	static unsigned char RT710_Reg_Arry_Init[RT710_Reg_Num] ={0x40, 0x1D, 0xA0, 0x10, 0x41, 0x50, 0xED, 0x25, 0x07, 0x58, 0x38, 0x64, 0x38, 0xE7, 0x90, 0x35};
#endif

static unsigned char RT710_Reg_Arry[RT710_Reg_Num];

INT32 nim_rt710_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	INT32 result;
       RT710_INFO_Type RT710_Set_Info;
	struct QPSK_TUNER_CONFIG_EXT * rt710_ptr = NULL;
	if (ptrTuner_Config == NULL||rt710_tuner_cnt>=MAX_TUNER_SUPPORT_NUM)
		return ERR_FAILUE;
	rt710_ptr = (struct QPSK_TUNER_CONFIG_EXT *)malloc(sizeof(struct QPSK_TUNER_CONFIG_EXT));
	if(!rt710_ptr)
	      return ERR_FAILUE;
	memcpy(rt710_ptr, ptrTuner_Config, sizeof(struct QPSK_TUNER_CONFIG_EXT));
	rt710_dev_id[rt710_tuner_cnt] = rt710_ptr;
	*tuner_id = rt710_tuner_cnt;
    
        int icount=0;
       for(icount=0;icount<RT710_Reg_Num;icount++)
	{
		RT710_Reg_Arry[icount]=RT710_Reg_Arry_Init[icount];
	}

	/*****************Get User Parameter************************/

	RT710_Set_Info.RT710_LoopThrough_Mode = SIGLE_IN;
	RT710_Set_Info.RT710_ClockOut_Mode = ClockOutOff;
	RT710_Set_Info.RT710_AGC_Mode = AGC_Negative;
	RT710_Set_Info.RT710_AttenVga_Mode=ATTENVGAOFF;
	RT710_Set_Info.R710_FineGain=FINEGAIN_2DB;

	/* LOOP_THROUGH(0=on ; 1=off)*/
	if(RT710_Set_Info.RT710_LoopThrough_Mode != LOOP_THROUGH)
	{
		RT710_Reg_Arry[1] |=0x04;
	}
	else
	{
		RT710_Reg_Arry[1] &=0xFB;
	}

	/*Clock out(1=off ; 0=on) */    
	if(RT710_Set_Info.RT710_ClockOut_Mode != ClockOutOn)
	{
		RT710_Reg_Arry[3] |=0x10;
	}
	else
	{
		RT710_Reg_Arry[3] &=0xEF;
	}

	/*AGC Type R13[4] Negative=0 ; Positive=1;*/
	if(RT710_Set_Info.RT710_AGC_Mode != AGC_Positive)
	{
		RT710_Reg_Arry[13] &=0xEF;
	}
	else
	{
		RT710_Reg_Arry[13] |=0x10;
	}

	/*RT710_Vga_Sttenuator_Type*/
	if(RT710_Set_Info.RT710_AttenVga_Mode != ATTENVGAON)
	{
		RT710_Reg_Arry[11] &= 0xF7;
	}
	else
	{
		RT710_Reg_Arry[11] |= 0x08;
	}

	/*R710_Fine_Gain_Type*/
	switch(RT710_Set_Info.R710_FineGain)
	{
		case FINEGAIN_3DB:  
		    RT710_Reg_Arry[14] = (RT710_Reg_Arry[14] & 0xFC) | 0x00;
		break;
		case FINEGAIN_2DB:  
		    RT710_Reg_Arry[14] = (RT710_Reg_Arry[14] & 0xFC) | 0x01;
		break;
		case FINEGAIN_1DB:  
		    RT710_Reg_Arry[14] = (RT710_Reg_Arry[14] & 0xFC) | 0x02;
		break;
		case FINEGAIN_0DB:  
		    RT710_Reg_Arry[14] = (RT710_Reg_Arry[14] & 0xFC) | 0x03;
		break;
		default:		
			 RT710_Reg_Arry[14] = (RT710_Reg_Arry[14] & 0xFC) | 0x00;
		break;
	}

	/*Output Signal Mode*/  
	if(rt710_ptr->cTuner_Out_S_D_Sel != 0)  // 1--> Single end, 0--> Differential
	{
		RT710_Reg_Arry[11] |=0x10;
	}
	else
	{
		RT710_Reg_Arry[11] &=0xEF;
	}
	 Tuner_I2C_write(rt710_tuner_cnt, 0x00, RT710_Reg_Arry_Init, 16); 

	rt710_tuner_cnt++;

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_rt710_control(UINT32 freq, UINT8 sym, UINT8 cp)
*
* Tuner write operation
*
* Arguments:
*  Parameter1: UINT32 freq		: Synthesiser programmable divider
*  Parameter2: UINT8 sym		: symbol rate
*
* Return Value: INT32			: Result
*****************************************************************************/
INT32 nim_rt710_control(UINT32 tuner_id,UINT32 freq, UINT32 sym)
{
	RT710_INFO_Type RT710_Set_Info;
       UINT32 Rs, BW;
	INT32 result;
	UINT8 fine_tune,Coarse_tune;
	UINT8 test_coar=0x0D;
	UINT32 Coarse;


	/******************For PLL Calculate********************/
	UINT8  MixDiv   = 2;
	UINT8  DivBuf   = 0;
	UINT8  Ni       = 0;
	UINT8  Si       = 0;
	UINT8  DivNum   = 0;
	UINT8  Nint     = 0;
	UINT32 VCO_Min  = 2350000;
	if ((freq*1000)>=2350000)
	{
		VCO_Min  = freq*1000;
	}
	UINT32 VCO_Max  = VCO_Min*2;
	UINT32 VCO_Freq = 0;
	UINT32 PLL_Ref	= RT710_Xtal;	
	UINT32 VCO_Fra	= 0;		
	UINT16 Nsdm		= 2;
	UINT16 SDM		= 0;
	UINT16 SDM16to9	= 0;
	UINT16 SDM8to1  = 0;
	UINT8  VCO_fine_tune = 0;

	struct QPSK_TUNER_CONFIG_EXT * rt710_ptr = NULL;
	if(tuner_id>=rt710_tuner_cnt||tuner_id>=MAX_TUNER_SUPPORT_NUM)
		return ERR_FAILUE;
	rt710_ptr = rt710_dev_id[tuner_id];

	//RT710_Set_Info
	RT710_Set_Info.RT710_Freq=freq*1000;
	#if 1
		UINT32 ratio = 125; //135;	
	#endif		
		Rs = sym;
		if (Rs == 0)
			Rs = 45000;
	#if 0
		RT710_Set_Info.DVBSBW = Rs*135/200;                
		RT710_Set_Info.DVBSBW = RT710_Set_Info.DVBSBW*130/100;                
		if (Rs < 6500)
			BW = BW + 3000;				
		RT710_Set_Info.DVBSBW = RT710_Set_Info.DVBSBW + 2000;                 
		RT710_Set_Info.DVBSBW = RT710_Set_Info.DVBSBW*108/100;                
	#else
		if (ratio == 0)
			RT710_Set_Info.DVBSBW = 34000;
		else
			RT710_Set_Info.DVBSBW = Rs * ratio / 100;	
	#endif
	if (Rs < 6500)
		RT710_Set_Info.DVBSBW += 6000;
    //   RT710_Set_Info.DVBSBW = RT710_Set_Info.DVBSBW + 2000;
    
	RT710_Reg_Arry[14] &= 0xF3;
	if(RT710_Set_Info.RT710_Freq >= 2000000)
	{
		RT710_Reg_Arry[14]=( RT710_Reg_Arry[14]& 0xF3)|0x08;
	}
	Tuner_I2C_write(tuner_id, 0x0E, &RT710_Reg_Arry[14], 1); 

    
	RT710_Reg_Arry[10] =  RT710_Reg_Arry_Init[10];
	if((RT710_Set_Info.RT710_Freq >= 1600000) && (RT710_Set_Info.RT710_Freq <= 1950000))
	{
		RT710_Reg_Arry[2] |= 0x40; 
		RT710_Reg_Arry[8] |= 0x80; 
	}
	else
	{
		RT710_Reg_Arry[2] &= 0xBF; 
		RT710_Reg_Arry[8] &= 0x7F; 
		if(RT710_Set_Info.RT710_Freq >= 1950000)
		{
			RT710_Reg_Arry[10] = ((RT710_Reg_Arry[10] & 0xF0) | 0x07);
			RT710_Reg_Arry[10] = ((RT710_Reg_Arry[10] & 0x0F) | 0x30);
		}
	}

	Tuner_I2C_write(tuner_id, 0x02, &RT710_Reg_Arry[2], 1); 
	Tuner_I2C_write(tuner_id, 0x08, &RT710_Reg_Arry[8], 1); 
	Tuner_I2C_write(tuner_id, 0x0A, &RT710_Reg_Arry[10], 1); 

	/*************PLL Calculate********************/
	while(MixDiv <= 4)  
	{
		if(((RT710_Set_Info.RT710_Freq * MixDiv) >= VCO_Min) && ((RT710_Set_Info.RT710_Freq * MixDiv) <= VCO_Max))
		{
			DivBuf = MixDiv;
			while(DivBuf <4) 
			{
				DivBuf = DivBuf << 1;
				DivNum ++;
			}			
			break;
		}
		MixDiv = MixDiv << 1;
	}

	/*Divider*/
	RT710_Reg_Arry[4] &= 0xFE;
	RT710_Reg_Arry[4] |= DivNum ;
	Tuner_I2C_write(tuner_id, 0x04, &RT710_Reg_Arry[4], 1); 


	VCO_Freq = RT710_Set_Info.RT710_Freq * MixDiv;		// Lo_Freq->Freq
	Nint     = (UINT8) (VCO_Freq / 2 / PLL_Ref);
	VCO_Fra  = (UINT16) (VCO_Freq - 2 * PLL_Ref * Nint);

	/*boundary spur prevention*/
	if (VCO_Fra < PLL_Ref/64)             // 2*PLL_Ref/128
		VCO_Fra = 0;
	else if (VCO_Fra > PLL_Ref*127/64)    // 2*PLL_Ref*127/128
	{
		VCO_Fra = 0;
		Nint ++;
	}
	else if((VCO_Fra > PLL_Ref*127/128) && (VCO_Fra < PLL_Ref)) 
		VCO_Fra = PLL_Ref*127/128;      
	else if((VCO_Fra > PLL_Ref) && (VCO_Fra < PLL_Ref*129/128)) 
		VCO_Fra = PLL_Ref*129/128;     
	else
		VCO_Fra = VCO_Fra;

	/*N & S	*/
	Ni       = (Nint - 13) / 4;
	Si       = Nint - 4 *Ni - 13;
	RT710_Reg_Arry[5]  = 0x00;
	RT710_Reg_Arry[5] |= (Ni + (Si << 6));
	Tuner_I2C_write(tuner_id, 0x05, &RT710_Reg_Arry[5], 1); 

	/*pw_sdm */
	RT710_Reg_Arry[4] &= 0xFD;
	if(VCO_Fra == 0)
		RT710_Reg_Arry[4] |= 0x02;
	Tuner_I2C_write(tuner_id, 0x04, &RT710_Reg_Arry[4], 1); 

	/*SDM calculator*/
	while(VCO_Fra > 1)
	{			
		if (VCO_Fra > (2*PLL_Ref / Nsdm))
		{		
			SDM = SDM + 32768 / (Nsdm/2);
			VCO_Fra = VCO_Fra - 2*PLL_Ref / Nsdm;
			if (Nsdm >= 0x8000)
				break;
		}
		Nsdm = Nsdm << 1;
	}

	SDM16to9 = SDM >> 8;
	SDM8to1 =  SDM - (SDM16to9 << 8);

	RT710_Reg_Arry[7]   = (UINT8) SDM16to9;
	Tuner_I2C_write(tuner_id, 0x07, &RT710_Reg_Arry[7], 1); 

	RT710_Reg_Arry[6]   = (UINT8) SDM8to1;
	Tuner_I2C_write(tuner_id, 0x06, &RT710_Reg_Arry[6], 1); 

	osal_task_sleep(10);
	//sleep(100);

	if(RT710_Set_Info.DVBSBW >=38000)
	{
		fine_tune=1;	
		Coarse =(( RT710_Set_Info.DVBSBW -38000) /1740)+16;
		if((( RT710_Set_Info.DVBSBW -38000) % 1740) > 0)
		Coarse+=1;
			
	}
	else if(RT710_Set_Info.DVBSBW<=5000)
	{
		Coarse=0;
		fine_tune=0;
	}
	else if((RT710_Set_Info.DVBSBW>5000) && (RT710_Set_Info.DVBSBW<=7300))
	{
		Coarse=0;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>7300) && (RT710_Set_Info.DVBSBW<=9600))
	{
		Coarse=1;
		fine_tune=0;
	}
	else if((RT710_Set_Info.DVBSBW>9600) && (RT710_Set_Info.DVBSBW<=10400))
	{
		Coarse=1;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>10400) && (RT710_Set_Info.DVBSBW<=11600))
	{
		Coarse=2;
		fine_tune=0;
	}
	else if((RT710_Set_Info.DVBSBW>11600) && (RT710_Set_Info.DVBSBW<=12600))
	{
		Coarse=2;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>12600) && (RT710_Set_Info.DVBSBW<=13400))
	{
		Coarse=3;
		fine_tune=0;
	}
	else if((RT710_Set_Info.DVBSBW>13400) && (RT710_Set_Info.DVBSBW<=14600))
	{
		Coarse=3;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>14600) && (RT710_Set_Info.DVBSBW<=15800))
	{
		Coarse=4;
		fine_tune=0;
	}
	else if((RT710_Set_Info.DVBSBW>15800) && (RT710_Set_Info.DVBSBW<=17000))
	{
		Coarse=4;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>17000) && (RT710_Set_Info.DVBSBW<=17800))
	{
		Coarse=5;
		fine_tune=0;
	}
	else if((RT710_Set_Info.DVBSBW>17800) && (RT710_Set_Info.DVBSBW<=19000))
	{
		Coarse=5;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>19000) && (RT710_Set_Info.DVBSBW<=20200))
	{
		Coarse=6;
		fine_tune=0;
	}
	else if((RT710_Set_Info.DVBSBW>20200) && (RT710_Set_Info.DVBSBW<=21200))
	{
		Coarse=6;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>21200) && (RT710_Set_Info.DVBSBW<=21800))
	{
		Coarse=7;
		fine_tune=0;
	}
	else if((RT710_Set_Info.DVBSBW>21800) && (RT710_Set_Info.DVBSBW<=23400))
	{
		Coarse=7;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>23400) && (RT710_Set_Info.DVBSBW<=24400))
	{
		Coarse=9;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>24400) && (RT710_Set_Info.DVBSBW<=24600))
	{
		Coarse=10;
		fine_tune=0;
	}
	else if((RT710_Set_Info.DVBSBW>24600) && (RT710_Set_Info.DVBSBW<=26200))
	{
		Coarse=10;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>26200) && (RT710_Set_Info.DVBSBW<=26600))
	{
		Coarse=11;
		fine_tune=0;
	}
	else if((RT710_Set_Info.DVBSBW>26600) && (RT710_Set_Info.DVBSBW<=28200))
	{
		Coarse=11;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>28200) && (RT710_Set_Info.DVBSBW<=29800))
	{
		Coarse=12;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>29800) && (RT710_Set_Info.DVBSBW<=31800))
	{
		Coarse=13;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>31800) && (RT710_Set_Info.DVBSBW<=34000))
	{
		Coarse=14;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>34000) && (RT710_Set_Info.DVBSBW<=35800))
	{
		Coarse=15;
		fine_tune=1;
	}
	else if((RT710_Set_Info.DVBSBW>35800) && (RT710_Set_Info.DVBSBW<=38000))
	{
		Coarse=16;
		fine_tune=1;
	}
	Coarse_tune = (unsigned char) Coarse; /*coras filter value*/
	/*fine tune and coras filter write*/
	RT710_Reg_Arry[15] = (((RT710_Reg_Arry[15] & 0x00) | ( fine_tune  )) | ( Coarse_tune<<2));

	Tuner_I2C_write(tuner_id, 0x0F, &RT710_Reg_Arry[15], 1); 
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_rt710_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock		: Phase lock status
*
* Return Value: INT32			: Result
*****************************************************************************/
INT32 nim_rt710_status(UINT32 tuner_id,UINT8 *lock)
{
	INT32 result;
	UINT8 data[16];
	UINT8 data1[15];
    
	struct QPSK_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	if(tuner_id>=rt710_tuner_cnt||tuner_id>=MAX_TUNER_SUPPORT_NUM)
	{
		*lock = 0;
		return ERR_FAILUE;
	}
	tuner_dev_ptr = rt710_dev_id[tuner_id];
	//Tuner_I2C_read(tuner_id, 0x00, &data, 16); /* Reg0~Reg15 */
	//Tuner_I2C_read(tuner_id, 0x00, &data1, 15);
	//*lock = ((data[2] & 0x80) >> 7);  /* Reg2[7] */
	//return result;

    	*lock = 1;
	return SUCCESS;
}

static int Tuner_I2C_write(UINT32 tuner_id, unsigned char reg_start, unsigned char* buff, unsigned char length) 
{ 
        UINT8 data[16]; 
        UINT32 rd = 0; 
        int i2c_result; 
        struct QPSK_TUNER_CONFIG_EXT * rt710_ptr = NULL; 
        rt710_ptr = rt710_dev_id[tuner_id];         
        data[0] = reg_start; 

        while((rd+15)<length) 
        { 
                memcpy(&data[1], &buff[rd], 15); 
                i2c_result = i2c_write(rt710_ptr->i2c_type_id, rt710_ptr->cTuner_Base_Addr, data, 16); 
                rd+=15; 
                data[0] += 15; 
                if(SUCCESS != i2c_result) 
                        return i2c_result; 
        } 
        memcpy(&data[1], &buff[rd], length-rd); 
        i2c_result = i2c_write(rt710_ptr->i2c_type_id, rt710_ptr->cTuner_Base_Addr, data, length-rd+1); 
        return i2c_result; 
} 

static int Tuner_I2C_read(UINT32 tuner_id, unsigned char reg_start, unsigned char* buff, unsigned char length) 
{ 
        UINT8 data[16]; 
        UINT32 rd = 0; 
        int i2c_result; 
        struct QPSK_TUNER_CONFIG_EXT * rt710_ptr = NULL; 
        rt710_ptr = rt710_dev_id[tuner_id];         
        data[0] = reg_start; 

        while((rd+15)<length) 
        { 
                i2c_result = i2c_write_read(rt710_ptr->i2c_type_id, rt710_ptr->cTuner_Base_Addr, data, 1, 15); 
                memcpy(&buff[rd], &data[0], 15); 
                rd+=15; 
                data[0] += 15; 
                if(SUCCESS != i2c_result) 
                        return i2c_result; 
        } 
        i2c_result = i2c_write_read(rt710_ptr->i2c_type_id, rt710_ptr->cTuner_Base_Addr, data, 1, length-rd); 
        memcpy(&buff[rd], &data[0], 15); 
		return i2c_result; 
} 

INT32 nim_rt710_Standby(UINT32 tuner_id, RT710_LoopThrough_Type RT710_LTSel )
{

	UINT8 RT710_Standby_Reg_Arry[RT710_Reg_Num]={0xFF, 0x58, 0x88, 0x10, 0x41, 0xC8, 0xED, 0x25, 0x07, 0xFC, 0x48, 0xA2, 0x08, 0x0F, 0xF3, 0x59};
	INT32 result;
	struct QPSK_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	if(tuner_id>=rt710_tuner_cnt||tuner_id>=MAX_TUNER_SUPPORT_NUM)
	{
		return ERR_FAILUE;
	}
	tuner_dev_ptr = rt710_dev_id[tuner_id];
	if(RT710_LTSel == LOOP_THROUGH)
	{
		RT710_Standby_Reg_Arry[1] &=0xFB;
		RT710_Standby_Reg_Arry[1] &=0xFD;
	}
	else
	{
		RT710_Standby_Reg_Arry[1] |=0x04;
		RT710_Standby_Reg_Arry[1] |=0x02;
	}

	Tuner_I2C_write(tuner_id, 0x00, RT710_Standby_Reg_Arry, 16); 

	return RT_Success;
}

