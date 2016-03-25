/***************************************************************************** 
Copyright 2012, AltoBeam Inc. All rights reserved.
     
File Name: atbm886x.c
Description: Software development source codes for ATBM886x DTMB/DVBC demodulator
(for SoC only supporting the fixed point calculation)
Version: 1.0.3
Released 2012-10-18

===============================================================================
Brief introduction for software porting

--------------------------------------------------------------------------------
[Basic functions for porting]

There are four basic functions which are called by other predefined APIs. 
Developer needs to implement them according to certain SoC/OS platform.
All functions for porting are listed at below:

SemodHardwareReset()
	This function is to control the output of SoC GPIO which is connected to the RESETN pin of demodulator.
DelayMS()
	This function is to generate time delay as required.
I2CRead()
	This function is to conduct a single read transaction on I2C bus.
I2CWrite()
	This function is to conduct a single write transaction on I2C bus.
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
[Configuration according to hardware environment]

Some environment variables need to be specified according to a certain hardware design.
The configuration for these varibles is included in function ATBMPowerOnInit().
All items for customization and the allowed values for each are listed at below:

MPEG_TS_mode_t
	ui8TSTransferType: TS_PARALLEL_MODE / TS_SERIAL_MODE	
	ui8OutputEdge: TS_OUTPUT_FALLING_EDGE / TS_OUTPUT_RISING_EDGE	
	ui8TSSPIMSBSelection: TS_SPI_MSB_ON_DATA_BIT7 / TS_SPI_MSB_ON_DATA_BIT0	
	ui8TSSSIOutputSelection: TS_SSI_OUTPUT_ON_DATA_BIT7 / TS_SSI_OUTPUT_ON_DATA_BIT0	
	ui8SPIClockConstantOutput: TS_CLOCK_CONST_OUTPUT / TS_CLOCK_VALID_OUTPUT	
DVBC_Params_t
	ui8InputMode: DVBC_IF_INPUT / DVBC_IQ_INPUT	
	i32SymbolRate: (1000k ~ 8000k symbol/sec)
tuner_config_t
	ui8IQmode: SWAP_IQ / NO_SWAP_IQ	
	ui8DTMBBandwithMHz: (6MHz, 7MHz and 8MHz bandwidth)
	dbIFFrequency: (4MHz~11MHz, 36MHz~37MHz)
custom_config_t
	ui8ATBMSlaveAddr: 0x80 / 0x88
	ui8CrystalOrOscillator:CRYSTAL / OSCILLATOR	
	ui8DtmbDvbcMode: ATBM_DTMB_MODE / ATBM_DVBC_MODE 
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
[Predefined API functions to be called]
There are a set of predefined functions provided in the SDK source codes.
Simply call some top-level functions to make a basic DTV application, including:

ATBMPowerOnInit()
       This function is to initialize demodulator after power up, this is the first function 
       must be called before any other demodulator functions.    
ATBMSetConfigParas(custom_config_t stCustomConfigp)
	This function config the parameter to the global variable ,it should be called 
	before ATBMSetDTMBMode() or ATBMSetDVBCMode()
ATBMSetDTMBMode()
	This function sets the demodulator working in DTMB mode. 
ATBMSetDVBCMode()
        This function sets the demodulator working in DVB-C mode. 
--------------------------------------------------------------------------------

For more information, please refer to the complete SDK user manual.
===============================================================================
******************************************************************************/
#include "atbm886x.h"

uint8            	ui8ATBMSlaveAddr         = 0x80;   /*either 0x80 or 0x88 */
uint8            	ui8ATBMSoftwareVersion = 10;     /*SW guide version = ui8ATBMSoftwareVersion/10 */
uint32		ui32DvbcOscFreq;
uint32          	ui32DvbcSymbolRate;
custom_config_t stCustomConfig;  /*global config parameter*/

struct ali_nim_device ali_m3200_nim_dev;
struct ali_nim_m3200_private *ali_m3200_nim_priv = NULL;
struct class *ali_m3200_nim_class;
struct device *ali_m3200_nim_dev_node;

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
int ali_m3200_nim_ioctl(struct file *file, unsigned int cmd, unsigned long parg);
#else
int ali_m3200_nim_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long parg);
#endif

RET_CODE ali_m3200_nim_open(struct inode *inode, struct file *file);
RET_CODE ali_m3200_nim_release(struct inode *inode, struct file *file);

static struct file_operations ali_m3200_nim_fops =
{
	.owner		= THIS_MODULE,
	.write		= NULL,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	.unlocked_ioctl = ali_m3200_nim_ioctl,
#else
	.ioctl		= ali_m3200_nim_ioctl,
#endif	
	.open		= ali_m3200_nim_open,
	.release		= ali_m3200_nim_release,
};

/****************************************************************************

   The following are APIs users need to implement based on their platform.
   If the enum define is not the same as your i2c return value, 
   You should change the header file to modify the enum ATBM_I2CREADWRITE_STATUS
   which value is success for i2c write.
                
****************************************************************************/
int demo_I2c_write(uint8 I2CSlaveAddr, uint8 *data, int length)
{
	struct ali_nim_m3200_private *priv = ali_m3200_nim_priv;
	INT32 bRet;

   	if(data == NULL||length == 0)
   		return -1;
	priv->dem_i2c_id =1;
	NIM_MUTEX_ENTER(priv);
	bRet = ali_i2c_write(priv->dem_i2c_id, I2CSlaveAddr, data, length); //
	NIM_MUTEX_LEAVE(priv);
	if (bRet)
	{
		NIM_LOG_ERROR("atmb8869 i2c write error = %d,chip_adr=0x%x,bMemAdr=0x%x,TaskID=0x%x\n", bRet, I2CSlaveAddr, priv->dem_i2c_id, (UINT32)current);
	}
	return bRet;
}

int demo_I2c_read(uint8 I2CSlaveAddr, uint8 *data, int length)
{
	struct ali_nim_m3200_private *priv = ali_m3200_nim_priv;
	int ret;
	
	if(data == NULL)
		return -1;
	priv->dem_i2c_id =1;
	NIM_MUTEX_ENTER(priv);

	ret = ali_i2c_read(priv->dem_i2c_id, I2CSlaveAddr, data, length);
	NIM_MUTEX_LEAVE(priv);
	if(ret)
	{
		NIM_LOG_ERROR("atmb8869 i2c read error \n");
	}
	return ret;
}

/******************************************************************************  
I2CRead:  Read one register by one step I2C format, parameters: 
   I2CSlaveAddr:  I2C slave address;   
   addr_length:   address length, here is 2 bytes; 
   addr_dat:      pointer to 2 bytes register address;  
   data_length:   Data length (in byte) to read, here is 1 byte;
   reg_dat:       pointer to retuned 1 byte register value;
   
   Reading register 0x0004 the I2C sequences is:   
   S_0x80_A_0x00_A_0x04_A_S_0x81_A_0x01_N_P   
   
   S:I2C start;
   A:ACK;
   N:NACK;
   P:stop        
****************************************************************************/
ATBM_I2CREADWRITE_STATUS I2CReadOneStep(uint8 ui8I2CSlaveAddr, uint16 addr_length, uint8 *addr_dat,  uint16 data_length, uint8 *reg_dat)
{
	 /*I2C read in one step format, please port this function if needed*/	
 	int bRet;
 	ATBM_I2CREADWRITE_STATUS status = ATBM_I2CREADWRITE_ERROR;
 	uint8 data[3];
	data[0] = addr_dat[0];
	data[1] = addr_dat[1];

	//demo_get_lock();
	bRet = demo_I2c_write(ui8I2CSlaveAddr,data,2);
	bRet |= demo_I2c_read(ui8I2CSlaveAddr,reg_dat,1);
	if(bRet == 0)
		status = ATBM_I2CREADWRITE_OK;
	else
		NIM_LOG_ERROR("status=%d\n",status);
    return status;
}

/******************************************************************************  
I2CWriteWithRestart: Write one register by one step I2C format, parameters: 
   I2CSlaveAddr:  I2C slave address;   
   addr_length:   address length number, here is 2 bytes; 
   addr_dat:      pointer to 2 bytes register address;  
   data_length:   Data length (in byte) to write, here is 1 byte;
   reg_dat:       pointer to 1 byte register value;

   Writing register 0x0004 with 0x01 the I2C sequences is:   
   S_0x80_A_0x00_A_0x04_A_S_0x80_A_0x01_A_P
      
   S:I2C start; A:ACK; N:NACK; P:stop    
   ******Attention a start is needed after sending the register's address.     
****************************************************************************/
ATBM_I2CREADWRITE_STATUS I2CWriteWithRestart(uint8 ui8I2CSlaveAddr, uint8 addr_length, uint8 *addr_dat,  uint8 data_length, uint8 *reg_dat)
{
 /*I2C write in one step format, please port this function if needed*/	
  	int bRet;
 	ATBM_I2CREADWRITE_STATUS status = ATBM_I2CREADWRITE_ERROR;
 	uint8 data[3];
	data[0] = addr_dat[0];
	data[1] = addr_dat[1];

	//demo_get_lock();
	bRet = demo_I2c_write(ui8I2CSlaveAddr,data,2);
	bRet |= demo_I2c_write(ui8I2CSlaveAddr,reg_dat,1);
	if(bRet == 0)
		status = ATBM_I2CREADWRITE_OK;
	else
		NIM_LOG_ERROR("status=%d\n",status);
    return status;
}

void DemodHardwareReset(void) 
{
	// <to be implemented by user>
	ATBM_PRINTF("Reset demod hardware!\n");
	Delayms(20);
}

void Delayms (int ms) 
{
	msleep(ms);
}



/****************************************************************************

   The following are APIs predefined by AltoBeam.
   You should change the values of the variables for configuration in function
   ATBMPowerOnInit() according to hardware design.
                
****************************************************************************/
/****************************************************************************
Next 3 functions is used for fix edition SDK calculation
******************************************************************************/
int SNR_log_table_int[]=
{
		0, 100,
		2, 104,
		4, 109,
		6, 114,
		8, 120,
		10, 125,
		12, 131,
		14, 138,
		16, 144,
		18, 151,
		20, 158,
		22, 166,
		24, 173,
		26, 182,
		28, 190,
		30, 199,
		32, 208	
};

int LeftTable(int InValue)
{
	int OutValue = 0;
	int TableSize = sizeof(SNR_log_table_int)/sizeof(int);
	int i;	
	for(i=0; i<TableSize; i+=2)
	{	
		if(InValue<=SNR_log_table_int[i+1])
		{		
			OutValue = SNR_log_table_int[i];	
			return OutValue;
		}	
	}
	OutValue = 32;
	return OutValue;
}

uint32 Log10Convert(uint32 InValue)
{
	uint8       		indx = 0;
	uint32		        u32_temp = 1;
	uint32              InValue_left;
	uint32 log_ret;    

	do {	
		u32_temp = u32_temp << 1;
		if (InValue < u32_temp)
			break;		
	}while(++indx < 32);
	
	log_ret = 301*indx/100;

	InValue_left = InValue*100/(1<<indx);
	
	log_ret = log_ret + LeftTable(InValue_left)/10;
	
	return log_ret;	
}

int  FixPow(int x,uint32 y)
{
	int i32Rst = x;
	uint32 i;
	if (y == 0)
	{
		return 1;
	}

	for (i = 0;i<y;i++)
	{
		i32Rst*= x;
	}
	return i32Rst;
}
/****************************************************************************
Function:     ATBMPowerOnInit
Parameters:   none
Return:       power_on_status  
Description:           
          This function close the reference clock and set  I2C gate off from demod to tuner.     
******************************************************************************/
int ATBMPowerOnInit(void)
{
	int power_on_status = 0;
	uint8 ui8pll, reset_status;
	uint8 ui8ChipID;

	stCustomConfig.stTunerConfig.ui8IQmode              		= NO_SWAP_IQ;        					     
	stCustomConfig.stTunerConfig.ui8DTMBBandwithMHz     	= 8; 				//unit: MHz
	stCustomConfig.stTunerConfig.ui32IFFrequency        	= 36000; 			//unit: KHz

	//demod config
	stCustomConfig.stTsMode.ui8TSTransferType         		= TS_SERIAL_MODE;	  
	stCustomConfig.stTsMode.ui8OutputEdge               	= TS_OUTPUT_FALLING_EDGE; //TS_OUTPUT_RISING_EDGE;    
	stCustomConfig.stTsMode.ui8SPIClockConstantOutput   	= TS_CLOCK_CONST_OUTPUT;
	stCustomConfig.stTsMode.ui8TSSPIMSBSelection        	= TS_SPI_MSB_ON_DATA_BIT7;
	stCustomConfig.stTsMode.ui8TSSSIOutputSelection     	= TS_SSI_OUTPUT_ON_DATA_BIT0;
	stCustomConfig.ui8CrystalOrOscillator               		= CRYSTAL;  			// when dbSampleClkFrequency == 30.4 then use CRYSTAL macro   and dbSampleClkFrequency == 24.0 then use OSCILLATOR
	stCustomConfig.ui32SampleClkFrequency               		= 30400; 			//unit: KHz

	//dvbc config
	stCustomConfig.stDvbcParams.i32SymbolRate		= 6875;					//unit: KSymbol/sec, the default value is 6875K in China DVBC broadcasting
	stCustomConfig.stDvbcParams.ui8InputMode		= DVBC_IF_INPUT;

	ui8ChipID = ATBMChipID();	
	ATBM_PRINTF("%s, demod id =%d \n",__FUNCTION__,ui8ChipID);
	if(ui8ChipID == 0x40)
	{
		ATBMWrite(0x01, 0x0c, 0x00);
		ATBMWrite(0x06, 0x06, 0x00);
		ATBMWrite(0x01, 0x03, 0x00); 
		ATBMRead(0x06, 0x0e, &ui8pll);
		ATBMWrite(0x06, 0x04, 0x01); 
		ui8pll |= 0x01;
		ATBMWrite(0x06, 0x0e,ui8pll);	
		ui8pll &= 0xfe;
		ATBMWrite(0x06, 0x0e,ui8pll);
		ATBMWrite(0x06, 0x04, 0x00);
	}
	else if(ui8ChipID == 0x30)
	{
		reset_status = ATBMReset(stCustomConfig.ui8CrystalOrOscillator); //Check hardware state
		if(reset_status ==0)
		{
			ATBM_PRINTF("please check Crystal or oscillator setting\n");			
		}
		ATBMWrite(0x06, 0x06, 0x00); 
		ATBMWrite(0x01, 0x03, 0x00);
	}
	else
	{
		NIM_LOG_ERROR("power on init failed, chip not correctly detected\n");
	}
	Delayms(1);/*1 ms*/ 
	return power_on_status;
}

/****************************************************************************
Function:     ATBMChipID
Parameters:   none
Return:       ChipID  
Description: This function checks ATBM886x chip ID,  The chip ID of ATBM886x is 0x40.     
******************************************************************************/
uint8 ATBMChipID(void)
{
	uint8 ui8ChipID;
	if(ATBM_I2CREADWRITE_OK == ATBMRead(0x00, 0x00,&ui8ChipID))
	{
		return ui8ChipID;
	}
	return 0xff;
}

/****************************************************************************
Function:    ATBMCheckDemodStatus
Parameters:  none
Return:      PLL lock status  

Description:  
     Call this function when crystal is used. if PLL locked, hardware reset is not needed.  
     This function is called before configuring demodulator registers.
	 This function check demodulator status after power on or hardware reset 
     If 1 is returned, demodulator in ready status.
     If 0 is returned, demodulator need hardware reset or power on.
******************************************************************************/
uint8 ATBMCheckDemodStatus(void)
{	
	uint8 ui8CheckTimes = 0;
	uint8 ui8Analog, ui8PLLState, ui8Tmp, ui8HardwareState, ui8ChipID;
	uint8 ui8Step = 0x20; 

	ATBMRead(0x06, 0x07, &ui8HardwareState);
	if(ui8HardwareState!=5)
	{
		return 0;
	}
	ATBMRead(0x00, 0x00, &ui8ChipID);
	if(ui8ChipID == 0x20) 
	{
		return 1; 
	}
	ui8PLLState = ATBMCheckPLLStatus();  //check state 0, no register change
	if(ui8PLLState == 1)
	{         
		return 1; 
	}	

	ATBMRead(0x02, 0x14, &ui8Tmp);	
	while(ui8CheckTimes<5)
	{
		if((ui8Tmp&0x02) == 0x02) 
		{		
			ATBMWrite(0x06, 0x06, 0x02); //open		
		}else
		{
			ATBMWrite(0x06, 0x06, 0x00); 
		}
		ui8Analog = ui8CheckTimes*ui8Step;	
		if(ui8CheckTimes == 4)
		{
			ui8Analog = 0x60;	
		}
		ATBMWrite(0x06, 0x0d, ui8Analog);
		if(ui8CheckTimes == 4)
		{
			ATBMWrite(0x06, 0x05, 0x00);
		}
		Delayms(10);			
		ui8PLLState = ATBMCheckPLLStatus();  
		if(ui8PLLState == 1)
		{         
			return 1; 
		}	
		ui8CheckTimes = ui8CheckTimes + 1;
	}	
	return 0;
} 

/****************************************************************************
Function:    PLLLock_Flag
Parameters:  none
Return:      demodulator PLL lock status  

Description: 
	 when internal PLL is locked, the returned value is 1.
******************************************************************************/
 uint8 PLLLockFlag(void)
 {
	 uint8 ui8Flag;
	 int i32Iter = 0;
	 while(i32Iter<20)
	 {
		 ATBMRead(0x06, 0x11, &ui8Flag);
		 if((ui8Flag&0x01) != 0x01)
		 {
			 return 0;
		 }
		 i32Iter++;
	 }
	 return 1;
 }

 /****************************************************************************
 Function:    Digtal_PLL_Lock_Flag
 Parameters:  none
 Return:      demodulator PLL lock status  

 Description: 
 when DVB-C register set a value different from the default value,
 if PLL unlocked, the register will be cleared to default value.
 If PLL aways locked, the register will not be changed.
 ******************************************************************************/
 unsigned char Digtal_PLL_Lock_Flag(void)
 {
	 int check_times = 0;
	 uint8 locked_flag = 0;
	 uint32 cla_in = 0, cla_out = 0x04;

	 ATBMWrite(0x00, 0x13, 0x01);
	 ATBMDVBCWrite(0x348, cla_out);
	 while(check_times<3)
	 {
		 ATBMDVBCRead(0x348, &cla_in);
		 if(cla_in != cla_out)
		 {		

			 locked_flag = 0;
			 break;
		 }
		 else
		 {
			 locked_flag = 1;
		 }
		 Delayms(10);
		 check_times = check_times + 1;
	 }	
	 ATBMWrite(0x00, 0x13, 0x00);
	 return locked_flag;
 }

/****************************************************************************
Function:    ATBMCheckPLLStatus
Parameters:  none
Return:      PLL status  
Description: 
	 When demodulator PLL locked and stable, the returned value is 1.
******************************************************************************/
 uint8 ATBMCheckPLLStatus(void)
 {
	 uint8 ui8PllState,pll_digtal_state, ui8Tmp;	

	 ui8PllState = PLLLockFlag(); 
	 if(ui8PllState == 1)
	 {
		 ATBMRead(0x02, 0x14, &ui8Tmp);
		 if((ui8Tmp&0x02) != 0x02)
		 {		
			 ATBMWrite(0x06, 0x06, 0x02); 
		 }
		 else
		 {
			 ATBMWrite(0x06, 0x06, 0x00); 
		 }
	 }
	 else
	 {
		 return 0;
	 }
	 Delayms(10);
	 ui8PllState = PLLLockFlag();
	 if(ui8PllState == 1)
	 {
		 pll_digtal_state = Digtal_PLL_Lock_Flag();
		 if(pll_digtal_state == 1)	 
		 {	 
			 return 1;
		 }

	 }
	 return 0;
 }

 /****************************************************************************
 Function:    ATBMReset
 Parameters:  ui8CryOrOsc
 Return:      1, succed. 0:  failed  
 Description:
 This function resets ATBM8869T demodulator if PLL is not locked. this is a
 compatible function used in ATBMInit or be called after suspend.
 for example, transfer from  suspend mode to standby mode.
 ******************************************************************************/
 int ATBMReset(uint8 ui8CryOrOsc)
 {

	 uint8 ui8HardwareState, ui8CheckResult = 0;
	 uint8 ui8Loop, ui8Tmp;	
	 /* After hardware power on properly or reset correctly,	  
	 * ui8HardwareState value should be 0x05 when using crystal, 3.3V PLL 
	 * ui8HardwareState value should be 0x07 when using oscillator, 3.3V PLL */	

	 if(ui8CryOrOsc==CRYSTAL)
	 {
		 //Software checking, when power on failed, it will return 0 because I2C error or PLL error
		 ui8CheckResult = ATBMCheckDemodStatus();
		 if(ui8CheckResult == 1)
		 {
			 return 1;
		 }	
		 //Hardware checking, usually used after demodulator resume from suspend mode
		 for(ui8Loop=0; ui8Loop<2; ui8Loop++)
		 {
			 if(ui8CheckResult != 1)
			 {
				 // ATBM_PRINTF("I2C Error or it is not properly power up, need hardware reset\n");
				 DemodHardwareReset();    //hardware check
				 Delayms(50);	
				 ui8CheckResult = ATBMCheckDemodStatus(); 		
			 }
			 else
			 {
				 return 1;
			 }	 
		 }
		 return 0;
	 }	

	 if(ui8CryOrOsc==OSCILLATOR)
	 {
		 ATBMRead(0x06, 0x07, &ui8HardwareState);
		 for(ui8Loop=0; ui8Loop<10; ui8Loop++)
		 {
			 if(ui8HardwareState!=0x07)
			 {		            
				 // ATBM_PRINTF("I2C Error or it is not properly power up, need hardware reset\n");
				 DemodHardwareReset();
				 Delayms(100);					
				 ATBMRead(0x06, 0x07, &ui8HardwareState);
			 }
			 else
			 {
				 ATBMRead(0x02, 0x14, &ui8Tmp);
				 if((ui8Tmp&0x02) != 0x02)
				 {		
					 ATBMWrite(0x06, 0x06, 0x02); 
				 }
				 else
				 {
					 ATBMWrite(0x06, 0x06, 0x00); 
				 }
				 break;
			 }	 
		 }

		 if( ui8Loop == 10)
		 {
			 return 0;
		 }else
		 {
			 return 1;
		 }
	 }
	 return 0;
 }

/****************************************************************************
Function:    ATBM_I2CByPassOn
Parameters:  none
Return:      none 
Description: Enable demod to pass I2C commands between tuner and host.  
 ****************************************************************************/
void ATBMI2CByPassOn(void)
{
	ATBMWrite(0x01,0x03,1); /* the 3st parameter:1-> Enable demod to pass I2C commands between tuner and host. */
}

/****************************************************************************
Function:    ATBM_I2CByPassOff
Parameters:  none
Return:      none 
Description: Disable the I2C pass-through. Tuner is disconnected from the I2C BUS after this function is called. 
****************************************************************************/
void ATBMI2CByPassOff(void)
{
	ATBMWrite(0x01,0x03,0); /*the 3st parameter:0-> Disable demod to pass I2C commands between tuner and host*/
}

/****************************************************************************
Function:    ATBMLockedFlag
Parameters:  none
Return:      1: locked, 0:unlocked  
Description: This function checks ATBM886x Demod locking status   		 
****************************************************************************/
int ATBMLockedFlag(void)
{
	 uint8 ui8DTMBFEC           = 0;
	 uint8 ui8CurReceivingMode;
	 uint32   ui32DvbcStatus         = 0; 	 

	 ATBMRead(0x00, 0x13, &ui8CurReceivingMode); /*0:DTMB, 1:DVB-C*/
	 if(ui8CurReceivingMode == 0x00) /*DTMB*/
	 {
	     	 ATBMRead(0x0b, 0x08, &ui8DTMBFEC);/*read DTMB internal register*/
		 if((ui8DTMBFEC&0x01) == 0x01) /*FER locked:bit0*/
		 {
			 return 1;
		 }
	 }
	 if(ui8CurReceivingMode == 0x01)
	 {
		 ATBMDVBCRead(0x08, &ui32DvbcStatus);/*read DVB-C internal register*/        
		 if(ui32DvbcStatus&0x20) /*bit 5 is locked flag*/
		 {                           
			 return 1;   /*DVB-C FEC locked*/
		 } 	 
	}
	return 0;
}


/****************************************************************************
Function:    ATBM_sync_state
Parameters:  none
Return:      2:lock, 1: sync, 0:unlocked 
Description: This function checks ATBM886x signal state, this is an internal 
             function used by ATBMChannelLockCheck.
****************************************************************************/
int ATBM_sync_state(void)
{
	uint8 i, dtmb_fec, signal_sync, sync_times = 0;
	for(i=0; i<10; i++)  
	{
		ATBMRead(0x0b, 0x08, &dtmb_fec);
		if(dtmb_fec&0x01) //bit 0
		{                           
			return 2;           //DTMB FEC locked
		}      	
		ATBMRead(0x08, 0x2B, &signal_sync);
		if(signal_sync == 6)
		{
			sync_times++;
		}
		Delayms(30);
	}   
	if(sync_times >= 3)
	{
		return 1;
	}else
	{
		return 0;
	}
}

/****************************************************************************
Function:    ATBMChannelLockCheck
Parameters:  none
Return:      1: locked, 0:unlocked 
Description: This function keeps checking ATBM886x signal locking status until it's Locked or time out.
             This function can be used for Auto channel scanning.	 
****************************************************************************/
int ATBMChannelLockCheck(void)
{
	uint8 dtmb_fec = 0;
	int check_result;
	uint32 dvbc_status = 0; 	
	uint32 i32Iter;
	uint8 cur_receiving_mode;
	uint32 check_times = 6; //default DTMB: 100ms + 6*300ms timeout 
	uint32 dvbc_time_out = 600; //milliseconds
	ATBMRead(0x00, 0x13, &cur_receiving_mode); //0:DTMB, 1:DVB-C
	if(cur_receiving_mode == 0) //DTMB
	{		
		for(i32Iter =0; i32Iter<2; i32Iter++)
		{
			ATBMRead(0x0b, 0x08, &dtmb_fec);
			if(dtmb_fec&0x01) //bit 0
			{                           
				return 1;           //DTMB FEC locked
			}     
			Delayms(50);
		}		
		for(i32Iter=0; i32Iter<check_times; i32Iter++)  
		{
			check_result = ATBM_sync_state();
			if(check_result == 2)
			{
				return 1; //FEC locked
			}
			if(check_result ==0)
			{
				return 0; //signal unsync
			}
		}   
	}
	if(cur_receiving_mode == 1) //DVB-C
	{
		check_times = dvbc_time_out/20;
		for(i32Iter=0; i32Iter<check_times; i32Iter++)
		{
			ATBMDVBCRead(0x08, &dvbc_status);       //read DVB-C internal register        
			if(dvbc_status&0x20) //bit 5
			{                           
				return 1;           //DVB-C FEC locked
			}                 
			Delayms(20);
		}
	}
	return 0; //FEC not locked
}


/****************************************************************************
Function:    ATBMChannelLockCheckForManual
Parameters:  none
Return:      1: locked, 0:unlocked 
Description: This function keeps checking ATBM886x signal locking status until it's Locked or time out.
             This function can be used for Manual channel scanning.	 
****************************************************************************/
int ATBMChannelLockCheckForManual(void)
{   
	uint8  ui8DTMBFEC          = 0;
	uint32  ui32DVBCStatus       = 0; 	
	uint8  ui8CurReceivingMode;	 
	uint32 ui32DTMBTimeOut     = 5000;  /*5000 millisecond*/
	uint32 ui32DVBCTimeOut     = 600;  //milliseconds
	int    i32CheckTimes;
	int    i32Iter;

    	ATBMRead(0x00, 0x13, &ui8CurReceivingMode); /*0:DTMB, 1:DVB-C*/
	if(ui8CurReceivingMode == 0) /*DTMB*/
	{
		i32CheckTimes = ui32DTMBTimeOut/20;
		for(i32Iter=0; i32Iter<i32CheckTimes; i32Iter++)
		{
			ATBMRead(0x0b, 0x08, &ui8DTMBFEC);			
			if(ui8DTMBFEC&0x01) /*bit 0*/
			{                           
				return 1;           /*DTMB FEC locked*/
			}                 
			Delayms(20);
		}
	}
	if(ui8CurReceivingMode == 1) /*DVB-C*/
	{
		i32CheckTimes = ui32DVBCTimeOut/20;
		for(i32Iter=0; i32Iter<i32CheckTimes; i32Iter++)
		{
			ATBMDVBCRead(0x08, &ui32DVBCStatus);       /*read DVB-C internal register*/        
			if(ui32DVBCStatus&0x20) /*bit 5*/
			{                           
				return 1;           /*DVB-C FEC locked*/
			}                 
			Delayms(20);
		}
	}
	return 0;
}

/****************************************************************************
Function:    ATBMHoldDSP
Parameters:  none
Return:      none  
Description: This function should be called before set tuner frequency.
 ****************************************************************************/
 void ATBMHoldDSP(void)
 {	 
	 ATBMWrite(0x00, 0x05, 0x01); 
 }

 /****************************************************************************
Function:    ATBMStartDSP
Parameters:  none
Return:      none 
Description: This function should be called after set tuner frequency.Attention: ATBMHoldDSP and ATBMStartDSP should be used together.
 ****************************************************************************/
 void ATBMStartDSP(void)
 {
	 ATBMWrite(0x00, 0x05, 0x00); 
 }
/****************************************************************************
Function:    ATBM_Standby
Parameters:  none
Return:      none
Description:
      This function put demodulator in standby mode, it can cut off part power,
	  the demodulator can wake up at once by calling ATBM_Standby_WakeUp.
	  two goals:
	  1, cut off part power when demodulator put in standby mode
	  2, put TS output pin in high-z state.
 ****************************************************************************/
void ATBMStandby(void)
{
	uint8 ui8ADCPD,ui8TSEdge,ui8RefStandby, ui8ADCRfv,ui8GPORge;

	ATBMRead(0x06, 0x00,&ui8RefStandby); 
	ATBMRead(0x06, 0x02,&ui8ADCPD);
	ATBMRead(0x03, 0x01,&ui8TSEdge);
	ATBMRead(0x06, 0x0d,&ui8ADCRfv);
	ATBMRead(0x10, 0xf7,&ui8GPORge);

	ui8RefStandby |= 0x02;     
	ui8ADCPD      |= 0x01;     
	ui8TSEdge     |= 0x02;     
	ui8ADCRfv     |=0x08;    
	ui8GPORge     |=0x01;      

	ATBMWrite(0x00, 0x05, 0x01); 
	ATBMWrite(0x06, 0x0d, ui8ADCRfv);
	ATBMWrite(0x06, 0x00, ui8RefStandby);
	ATBMWrite(0x06, 0x02, ui8ADCPD); 
	ATBMWrite(0x15, 0x00, 0x00); 
	ATBMWrite(0x03, 0x01, ui8TSEdge); /*all TS output PINs will be high-z*/
	ATBMWrite(0x10, 0xf7, ui8GPORge); 
	ATBMWrite(0x00, 0x19, 0x00); 
}

/****************************************************************************
Function:    ATBM_Standby_WakeUp
Parameters:  none
Return:      none
Description: Put demodulator in previous normal working mode from standby mode 		 
 ****************************************************************************/
void ATBMStandbyWakeUp(void)
{
	uint8 ui8ADCPD,ui8TSEdge,ui8RefStandby,ui8ADCRfv,ui8GPORge;

	ATBMRead(0x06, 0x00,&ui8RefStandby);
	ATBMRead(0x06, 0x02,&ui8ADCPD);
	ATBMRead(0x03, 0x01,&ui8TSEdge);
	ATBMRead(0x06, 0x0d,&ui8ADCRfv);
	ATBMRead(0x10, 0xf7,&ui8GPORge);
	ui8RefStandby  &= 0xfd;  //bit1 set 0
	ui8ADCPD       &= 0xfe;     //bit0 set 0
	ui8TSEdge      &= 0xfd;    //bit1 set 0     
	ui8ADCRfv      &= 0xf7;
	ui8GPORge      &= 0xfe; //bit 0 set 0

	ATBMWrite(0x06, 0x00, ui8RefStandby);
	ATBMWrite(0x06, 0x02, ui8ADCPD); 
	ATBMWrite(0x15, 0x00, 0x01); 
	ATBMWrite(0x03, 0x01, ui8TSEdge); //all TS output PINs will be in normal mode     
	ATBMWrite(0x10, 0xf7, ui8GPORge); 
	ATBMWrite(0x06, 0x0d, ui8ADCRfv);
	ATBMWrite(0x00, 0x19, 0x01); 
	ATBMWrite(0x00, 0x05, 0x00); 

}

/****************************************************************************
Function:    ATBM_Suspend
Parameters:  none
Return:      none
Description:
    Put ATBM886x demodulator in suspend mode, Hardware reset is needed to wake up
    the demodulator. if external oscillator is used Hardware pull down is used 
	to enter suspend mode.
****************************************************************************/
void ATBMSuspend(void)
{	
	uint8 ui8ChipID;
	ui8ChipID = ATBMChipID();	
	if(ui8ChipID == 0x40)
	{
       	ATBMWrite(0x06, 0x06, 0x2); 
	}
	ATBMStandby();	
	ATBMWrite(0x01, 0x0c, 0x00); //I2C clock switch 
	ATBMWrite(0x06, 0x04, 0x01); //shutdown PLL     
	ATBMWrite(0x06, 0x0e, 0x02); //shut down crystal clock
}

/****************************************************************************
Function:       ATBMSignalStrength
Parameters:     none
Return:         unsigned short PWM value, in the range of 0--1023
Description:
     This function checks ATBM886x signal strength: when signal is weak the number 
     will be big; The smaller the PWM is, the stronger the signal strength detected.	 
 ****************************************************************************/
int ATBMSignalStrength(void)	
{
    uint8 ui8PWM0, ui8PWM1;

    ATBMWrite(0x08,0x4d, 1); /*latch read only registers*/	
    ATBMRead(0x10,0x28,&ui8PWM0);  
    ATBMRead(0x10,0x29,&ui8PWM1);	
    ATBMWrite(0x08,0x4d, 0);
    return  (((ui8PWM1&0x03)<<8)|ui8PWM0);
}

 /****************************************************************************
 Function:    ATBMSetDTMBMode
 Parameters:  none
 Return:      none   
 Description:
	This function initializes ATBM886x in DTMB mode, selects DTMB receiving mode,
	then calls ATBMInit function.	  
	ATBMSetConfigParas function should be called before this function.
 ******************************************************************************/
 void ATBMSetDTMBMode(void)
 {
	 stCustomConfig.ui8DtmbDvbcMode = ATBM_DTMB_MODE;
	 ATBMInit();
 }

/****************************************************************************
Function:        ATBMSignalNoiseRatio
Parameters:      none
Return:          uint32 (dB) 
Description:
     This function checks ATBM886x signal-to-noise ratio.
     Can be used for signal quality evaluation.   		 
 ****************************************************************************/
 uint32 ATBMSignalNoiseRatio(void)
 {
	 uint8  ui8PNValue,ui8PN, ui8TmpNoise,ui8IsSc,ui8SignalH, ui8SignalM, ui8SignalL,ui8NoiseH,ui8NoiseM, ui8NoiseL;   
	 int    i32SignalPower,i32NoisePower;
	 uint32 ui32SNR = 0;   

	 ATBMWrite(0x08, 0x4d, 1);/*lock register value*/	

	 ATBMRead(0x08, 0x2d,&ui8PNValue);   
	 ui8PN       = (ui8PNValue&0x03);	 
	 ATBMRead(0x14, 0xAF,&ui8TmpNoise); 	
	 ATBMRead(0x0d, 0x0f,&ui8IsSc); 

	 if(ui8IsSc !=1 )
	 {
		 ATBMRead(0x0f, 0x1d,&ui8SignalL);
		 ATBMRead(0x0f, 0x1e,&ui8SignalM);
		 ATBMRead(0x0f, 0x1f,&ui8SignalH); 
		 ATBMRead(0x0f, 0x1a,&ui8NoiseL);
		 ATBMRead(0x0f, 0x1b,&ui8NoiseM);   
		 ATBMRead(0x0f, 0x1c,&ui8NoiseH);
		 i32SignalPower = ((ui8SignalH&0x0f)<<16)+(ui8SignalM<<8)+ui8SignalL;
		 i32NoisePower  = ((ui8NoiseH&0x07)<<16)+(ui8NoiseM<<8)+ui8NoiseL; 				 
		 if(i32SignalPower == 0)
		 {
			 i32SignalPower = 1; //signal_power should >0 in log calculation;
		 }		
		 // 		 dbSNR = (double) (10*(log10(i32SignalPower*1.0/(i32NoisePower+1))));	    
		 ui32SNR = Log10Convert(i32SignalPower)-Log10Convert(i32NoisePower+1);
	 }	 
	 if(ui8IsSc ==1 )
	 {
		 if(ui8PN == 2) //PN595
		 {   	 
			 ATBMRead(0x14, 0xdc,&ui8SignalL);
			 ATBMRead(0x14, 0xdd,&ui8SignalM);
			 ATBMRead(0x14, 0xde,&ui8SignalH);  
			 i32SignalPower = ((ui8SignalH&0x0f)<<16)+(ui8SignalM<<8)+ui8SignalL;
			 i32SignalPower = i32SignalPower/16;
			 if(ui8TmpNoise != 3)
			 {
				 ATBMRead(0x14, 0xf8,&ui8NoiseL);		
				 ATBMRead(0x14, 0xf9,&ui8NoiseH);
				 i32NoisePower = ((ui8NoiseH&0x3f)<<8) + ui8NoiseL;
			 }
			 else
			 {                    
				 ATBMRead(0x13, 0x40,&ui8NoiseL);		
				 ATBMRead(0x13, 0x41,&ui8NoiseH);                       
				 i32NoisePower = ((ui8NoiseH&0x3f)<<8) + ui8NoiseL;
			 }	
			 if(i32SignalPower == 0)
			 {
				 i32SignalPower = 1; //signal_power should >0 in log calculation;
			 }		
			 // 			 dbSNR = (double) (10*(log10(i32SignalPower*1.0/(i32NoisePower+1))));	    
			 ui32SNR = Log10Convert(i32SignalPower)-Log10Convert(i32NoisePower+1);
		 }

		 if((ui8PN == 1)||(ui8PN == 3)) //PN420 & PN945
		 {   	   	
			 ATBMRead(0x09, 0xcc,&ui8SignalL);	
			 ATBMRead(0x09, 0xcd,&ui8SignalH);  
			 i32SignalPower = (ui8SignalH<<8) + ui8SignalL;
			 ATBMRead(0x09, 0xed,&ui8NoiseL);
			 ATBMRead(0x09, 0xee,&ui8NoiseH);
			 i32NoisePower  = ((ui8NoiseH&0x3f)<<8) + ui8NoiseL;						

			 if(i32SignalPower == 0)
			 {
				 i32SignalPower = 1; //signal_power >0;
			 }			 
			 if(i32NoisePower == 0)
			 {
				 // 				 dbSNR = (double)(10*(log10(i32SignalPower*1.0/(i32NoisePower + 1))));
				 ui32SNR = Log10Convert(i32SignalPower)-Log10Convert(i32NoisePower+1);
			 }
			 else
			 {	
				 // 				 dbSNR = (double)(10*(log10(i32SignalPower*1.0/i32NoisePower)));	
				 ui32SNR = Log10Convert(i32SignalPower)-Log10Convert(i32NoisePower);
			 }    
		 }
	 }

	 ATBMWrite(0x08,0x4d, 0);

	 return ui32SNR;
 }
 /****************************************************************************
 Function:           ATBMSignalDivideNoise
 Parameters:      none
 Return:             SNR  

 Description:
 This function is an internal function called by ATBMSignalQuality()
 ****************************************************************************/
 uint32 ATBMSignalDivideNoise(void)	
 {
	 uint8 LockValue;

	 uint8 pn_value,PN;   
	 uint8 tmp_noise; 
	 uint8 is_sc;
	 uint32 SNR = 0;   

	 uint8 signal_h, signal_m, signal_l;
	 uint8 noise_h,noise_m, noise_l;   
	 uint32 signal_power = 0;
	 uint32 noise_power = 0;

	 LockValue =1; /*lock register value*/
	 ATBMWrite(0x08, 0x4d, LockValue);

	 ATBMRead(0x08, 0x2d, &pn_value);   
	 PN = (uint8)(pn_value&0x03);

	 ATBMRead(0x14, 0xAF, &tmp_noise); 	
	 ATBMRead(0x0d, 0x0f, &is_sc); 
	 if(is_sc!=1)
	 {
		 ATBMRead(0x0f, 0x1d, &signal_l);
		 ATBMRead(0x0f, 0x1e, &signal_m);
		 ATBMRead(0x0f, 0x1f, &signal_h); 
		 ATBMRead(0x0f, 0x1a, &noise_l);
		 ATBMRead(0x0f, 0x1b, &noise_m);   
		 ATBMRead(0x0f, 0x1c, &noise_h);
		 signal_power = ((signal_h&0x0f)<<16)+(signal_m<<8)+signal_l;
		 noise_power  = ((noise_h&0x07)<<16)+(noise_m<<8)+noise_l; 				 
		 if(signal_power ==0)
		 {
			 signal_power =1; //signal_power should >0 in log calculation;
		 }		
		 SNR = (uint32) (signal_power/(noise_power+1));	    
	 }	 
	 if(is_sc==1)
	 {
		 if(PN==2) //PN595
		 {   	 
			 ATBMRead(0x14, 0xdc, &signal_l);
			 ATBMRead(0x14, 0xdd, &signal_m);
			 ATBMRead(0x14, 0xde, &signal_h);  
			 signal_power = ((signal_h&0x0f)<<16)+(signal_m<<8)+signal_l;
			 signal_power = signal_power/16;
			 if(tmp_noise!=3)
			 {
				 ATBMRead(0x14, 0xf8, &noise_l);		
				 ATBMRead(0x14, 0xf9, &noise_h);
				 noise_power = ((noise_h&0x3f)<<8) + noise_l;
			 }
			 else
			 {                    
				 ATBMRead(0x13, 0x40, &noise_l);		
				 ATBMRead(0x13, 0x41, &noise_h);                       
				 noise_power = ((noise_h&0x3f)<<8) + noise_l;
			 }	
			 if(signal_power ==0)
			 {
				 signal_power =1; //signal_power should >0 in log calculation;
			 }		
			 SNR = (uint32)(signal_power/(noise_power+1));	    

		 }

		 if( (PN==1)||(PN==3) ) //PN420 & PN945
		 {   	   	

			 ATBMRead(0x09, 0xcc, &signal_l);	
			 ATBMRead(0x09, 0xcd, &signal_h);  
			 signal_power = (signal_h<<8) + signal_l;
			 ATBMRead(0x09, 0xed, &noise_l);
			 ATBMRead(0x09, 0xee, &noise_h);
			 noise_power = ((noise_h&0x3f)<<8) + noise_l;						

			 if(signal_power ==0)
			 {
				 signal_power = 1; //signal_power >0;
			 }			 
			 if(noise_power==0)
			 {
				 // ATBM_PRINTF("noise_power is 0,  signal error!");
				 SNR = (uint32)(signal_power/(noise_power + 1));
			 }
			 else
			 {	
				 SNR = (uint32)(signal_power/noise_power);	    
			 }    
		 }
	 }
	 LockValue = 0; /*cancel the lock*/
	 ATBMWrite(0x08,0x4d, LockValue);
	 return SNR;
 }
/****************************************************************************
Function:    ATBMSignalQuality
Parameters:  none
Return:      i32SNR100per, the signal quality evaluated value.
Description:
     This function checks ATBM886x signal-to-noise ratio.
     Can be used for signal quality evaluation:
	 max value:99, min:20
 ****************************************************************************/
int ATBMSignalQuality(void)
{   
	uint8  ui8TPSCode, ui8TSSync;  
	int    i32SNR = 0;	
	int    i32SNR100per;   
//	uint32 ui32SNRdB = 0;    
	uint32 ui32FrameError;

//	ui32SNRdB    = ATBMSignalNoiseRatio(); //ATBM886x
	i32SNR = ATBMSignalDivideNoise(); 
	ui8TPSCode = ATBMGetTPS();
	ATBMRead(0x03, 0x0d,&ui8TSSync);  
	if(ui8TSSync != 1)
	{
		ATBMRead(0x03, 0x0d,&ui8TSSync);  
	}
	ui32FrameError = ATBMFrameErrorRatio();

	i32SNR100per = 20;  
	if((ui8TPSCode <11)&&(ui8TPSCode>2))/*4QAM FEC:0.6 0.8*/
	{
		if(i32SNR>100)
		{
			i32SNR100per = 90+ i32SNR/100;
		}
		else if(i32SNR>60)
		{
			i32SNR100per = 80 + (i32SNR-60)/4 ;
		}   	 
		else if(i32SNR>20)
		{
			i32SNR100per = 60 + (i32SNR-20)/2;
		}
		else if(i32SNR>5)
		{	  	
			i32SNR100per = 40 + i32SNR;
		}
		else
		{
			i32SNR100per = 40;
		}
	}

	if((ui8TPSCode <17)&&(ui8TPSCode>10)) /*For 16QAM FEC:0.6 0.8*/
	{
		if(i32SNR>100)
		{
			i32SNR100per = 90+ i32SNR/100;
		}
		else if(i32SNR>70)
		{
			i32SNR100per = 80 + (i32SNR-70)/3 ;
		}   	 
		else if(i32SNR>30)
		{
			i32SNR100per = 60 + (i32SNR-30)/2;
		}
		else if(i32SNR>10)
		{	  	
			i32SNR100per = 50 + (i32SNR-10)/2;
		}
		else
		{
			i32SNR100per = 40;
		}
	}   
	if((ui8TPSCode <25)&&(ui8TPSCode>16))/*64QAM FEC:0.6; 32QAM 0.8*/
	{

		if(i32SNR>100)
		{
			i32SNR100per = 90+ i32SNR/100;
		}
		else if(i32SNR>70)
		{
			i32SNR100per = 80 + (i32SNR-70)/3 ;
		}   	 
		else if(i32SNR>30)
		{
			i32SNR100per = 60 + (i32SNR-30)/2;
		}
		else if(i32SNR>20)
		{	  	
			i32SNR100per = 30+ i32SNR;
		}
		else
		{
			i32SNR100per = 40;
		}
	}

	//re-evaluation   
	if((ui8TSSync ==1)&&(ui32FrameError == 0)) //no frame error
	{
		//default:5, you can change this value form 0--10, for evaluation	
		i32SNR100per +=5; 	
	}
	else if((ui8TSSync ==1)&&(ui32FrameError!=0))
	{
		i32SNR100per = i32SNR100per - 10; 
	}
	else
	{
		i32SNR100per = 30;
	}
	if(i32SNR100per>99)
	{
		i32SNR100per = 99;
	}  

	return i32SNR100per;
}

/****************************************************************************
Function:    ATBMSignalStrength_reference
Parameters:  none
Return:      signal_percent 
Description:    
	This function is a sample of signal strength evaluation. 
	the value AGC_A, AGC_B, AGC_C should be calibrated according to can tuner used.	
****************************************************************************/
uint16 ATBMSignalStrength_reference(void)	
{
	uint8 agc0, agc1,LockValue;
	uint16 agc_value;
	uint16 signal_percent;

	//agc_value can be read from register 0x1028, 0x1029,  the AGC Value is related with AGC voltage: about (1024/3.3)*V_agc,
	//Refer to SDK document. the following value should be changed according to Tuner used.

	uint16 AGC_A = 186; //about AGC 0.6V, Signal strength:80
	uint16 AGC_B = 496; //about AGC 1.6V, Signal strength:60
	uint16 AGC_C = 930; //about AGC 3.0V, Signal strength:20

	LockValue =1; /*lock register value*/
	ATBMWrite(0x08,0x4d, LockValue);

	ATBMRead(0x10, 0x28, &agc0);  
	ATBMRead(0x10, 0x29, &agc1);

	LockValue = 0;
	ATBMWrite(0x08,0x4d, LockValue);
	agc_value = (uint16)(((agc1&0x03)<<8)|agc0);

	/*Tuner Reference evaluation*/
	if(agc_value <=AGC_A)
	{
		signal_percent = 80;	
	}                    
	else if(agc_value <=AGC_B)
	{   	
		signal_percent = (uint16)((AGC_B - agc_value)*20/(AGC_B - AGC_A) + 60);	
	}
	else if(agc_value <=AGC_C)
	{   	
		signal_percent = (uint16)((AGC_C - agc_value)*40/(AGC_C - AGC_B) + 20);	
	}
	else
	{
		signal_percent = 20;	
	}

	return signal_percent;
}

/****************************************************************************
Function:    ATBMFrameErrorRatio
Parameters:  none
Return:      ui32FrameErrorRatio 
Description:    
	This function counts pre-BCH error frames in specified frame window size
	(Default 2048 frames), then print and return pre-BCH frames error ratio.   
****************************************************************************/
uint32 ATBMFrameErrorRatio(void)
{

	uint8  ui8FrameError0, ui8FrameError1;
	uint8  ui8ArrFrameCountMax[2];
	uint32 ui32FrameErrorRatio;

	ATBMRead(0x0B, 0x09,&ui8ArrFrameCountMax[0]); 
	ATBMRead(0x0B, 0x0A,&ui8ArrFrameCountMax[1]);	
	ATBMWrite(0x08, 0x4d, 1);  /*latch read only registers*/
	ATBMRead(0x0B, 0x14,&ui8FrameError0);  
	ATBMRead(0x0b, 0x15,&ui8FrameError1);
	ATBMWrite(0x08, 0x4d, 0);
	if((ui8ArrFrameCountMax[1] == 0)&&(ui8ArrFrameCountMax[0] == 0))
	{
		NIM_LOG_ERROR("I2C error\n");
		ui32FrameErrorRatio = 0;
		return ui32FrameErrorRatio;
	}
	ui32FrameErrorRatio = ((ui8FrameError1&0x7f)*256 + ui8FrameError0)/((ui8ArrFrameCountMax[1]&0x7f)*256 + ui8ArrFrameCountMax[0]);	

	return  ui32FrameErrorRatio; 
}

/****************************************************************************
Function:    ATBMPreBCHBlockErrorRatio
Parameters:  none
Return:      ui32BlockErrorRatio

Description:
    This function counts blocks with error in specified block window size,
    Then calculate and return pre-BCH block error ratio.    		 
****************************************************************************/
uint32 ATBMPreBCHBlockErrorRatio(void)
{	

	uint8 ui8BlockCountMax[2];
	uint8 ui8PreBCHBerCount[2];            
	uint8 ui8LockValue; 	
	uint32 ui32BlockErrorRatio;

	ATBMRead(0x0B, 0x10,&ui8BlockCountMax[0]);	
	ATBMRead(0x0B, 0x11,&ui8BlockCountMax[1]);		

	ui8LockValue =1; 
	ATBMWrite(0x08, 0x4d, ui8LockValue);  /*latch read only registers*/	
	ATBMRead(0x0B, 0x12,&ui8PreBCHBerCount[0]); 
	ATBMRead(0x0B, 0x13,&ui8PreBCHBerCount[1]);  	
	ui8LockValue = 0;
	ATBMWrite(0x08, 0x4d, ui8LockValue);
	if((ui8BlockCountMax[0] == 0)&&(ui8BlockCountMax[1]==0))
	{
		NIM_LOG_ERROR("I2C error\n");
		ui32BlockErrorRatio = 0;
		return ui32BlockErrorRatio;
	}
	ui32BlockErrorRatio  = (ui8PreBCHBerCount[1]*256 + ui8PreBCHBerCount[0])/(ui8BlockCountMax[1]*256 + ui8BlockCountMax[0]);
	ATBM_PRINTF(" DTMB Pre-BCH Block error ratio = %f\n", ui32BlockErrorRatio);
	return ui32BlockErrorRatio;
}

/****************************************************************************
Function:    ATBMBER_Calc
Parameters:  none
Return:      BER

Description:
    This function returns calculated BER.    		 
****************************************************************************/
uint32 ATBMBER_Calc(void)
{	
	uint8 ber_count[3];            
	uint32 ber_calc = 0;
	uint8 LockValue; 	

	LockValue =1; 
	ATBMWrite(0x08, 0x4d, LockValue);  /*latch read only registers*/	
	ATBMRead(0x0B, 0x21, &ber_count[0]);	
	ATBMRead(0x0B, 0x22, &ber_count[1]);
	ATBMRead(0x0B, 0x23, &ber_count[2]); 
	LockValue = 0;
	ATBMWrite(0x08, 0x4d, LockValue);	

	ber_calc  = ((ber_count[2]&0x07)*256*256 + ber_count[1]*256 + ber_count[0])/FixPow(2,27) ; //134217728
	return ber_calc;
}


/****************************DVBC Parameters************************************************/
uint8  ui8DVBCI2CBurstMode = 0;
// uint32 dbCMS0022MaxSymbolRateRatio         = (1.0/4);         
// uint32 dbCMS0022CIC2DELAY                  = (3.0);
// uint32 dbCMS0022CIC3DELAY                  = (0.0);
uint8  ui8ADvbcCciAci[]=
{		
	0x0a, 0x3b, 0x28,
	0x0a, 0x42, 0x10,
	0x0a, 0x43, 0x05,
	0x0a, 0x46, 0x00,
	0x0a, 0x47, 0x01,
	0x0a, 0x48, 0x14,
	0x0a, 0x49, 0x00,
	0x0a, 0x4a, 0x76,
	0x0a, 0x52, 0x10,
	0x0a, 0x53, 0x52,
	0x0a, 0x54, 0xb1,
	0x0a, 0x58, 0x00,
	0x0a, 0x59, 0x40,
	0x0a, 0x5b, 0x00,
	0x0a, 0x5c, 0x1f,
	0x0a, 0x5d, 0x05,
	0x0a, 0x5e, 0x20,
	0x0a, 0x5f, 0x00,
	0x0a, 0x60, 0x04,
	0x0a, 0x62, 0x1e,	
	0x0a, 0x91, 0x0f,
	0x0a, 0x95, 0x38,
	0x0a, 0xa0, 0xff,
	0x0a, 0xa1, 0xfa,
	0x0a, 0xa2, 0x00,
	0x0a, 0xac, 0xff,
	0x0a, 0xb2, 0x00,
	0x0a, 0xb3, 0xff,
	0x0a, 0xb4, 0x0f,
	0x0a, 0xb5, 0x68,
	0x0a, 0xb7, 0x5c,
	0x0a, 0xb9, 0xff,
	0x0a, 0xba, 0x0f,
	0x0a, 0xbe, 0x9d,
	0x0a, 0xc0, 0x14,
	0x0a, 0xd2, 0x30,
	0x0a, 0xd3, 0x00,
	0x0a, 0xd4, 0x05,
	0x0a, 0xd5, 0x40,
	0x0a, 0xd9, 0x0a,
	0x10, 0x81, 0xfa,
	0x10, 0x82, 0x00,
	0x10, 0x9d, 0x00,
	0x10, 0x9f, 0x00,
	0x10, 0xa1, 0x00,	
	0x10, 0xa2, 0x00,
	0x10, 0xa7, 0x0b,
	0x10, 0xac, 0x4d	
};

static uint8 ui8ADtmbInternal[]=
{	
	0x00, 0x16, 0x1d, //DTMB mode   
	0x09, 0x88, 0x08,
	0x09, 0x89, 0x0c,    
	0x09, 0x9a, 0x40,	
	0x09, 0x35,0x14,
	0x0a, 0xf9, 0x00,
	0x0e, 0x01, 0x08,
	0x08, 0x52, 0x28,
	0x0c, 0x24, 0x0a,
	0x0c, 0x26, 0x0a
};
static uint8 uiA8DvbcInternal[]=
{		
	0x00, 0x0a, 0x00,
	0x00, 0x09, 0x00,
	0x00, 0x13, 0x01,
	0x15, 0x18, 0x01,
	0x15, 0x15, 0x01,
	0x15, 0x20, 0x00,
	0x15, 0x21, 0x80,
	0x15, 0x2e, 0x01
};

uint8 ui8AATBM886xCommonReg[]=
{
	0x02, 0x45, 0x33,
	0x02, 0x4a, 0x96,
	0x02, 0xc6, 0x00,
	0x02, 0xc7, 0x01 
};
uint8 ui8AATBM886xDtmbSet[]=
{
	0x02, 0x28, 0x01, 
	0x02, 0xa6, 0x10, 
	0x02, 0xa9, 0x60, 
	0x02, 0xcb, 0x68, 
	0x02, 0xcc, 0x70, 
	0x02, 0x49, 0x30  
};
uint8 ui8AATBM886xDvbcSet[]=
{
	0x02, 0x28, 0x00, 
	0x02, 0x3c, 0x0c, 
	0x02, 0x49, 0x29, 
	0x02, 0xa6, 0x14, 
	0x02, 0xa9, 0x4a,
	0x02, 0xcb, 0x40, 
	0x02, 0xcc, 0x43, 
	0x15, 0x12, 0x00
};

uint8 ui8AATBM886xDtmb30Dot4MSet[]=
{	
	0x02, 0xC5, 0x05,
	0x02, 0x4E, 0x08,
	0x02, 0x3E, 0x1C,
	0x02, 0x3B, 0x1C,
	0x0A, 0xFB, 0x00,
	0x02, 0x3C, 0x16
};
uint8 ui8AATBM886xDtmb24MSet[]=
{
	0x02, 0xC5, 0x15,            
	0x02, 0x4E, 0x10,            
	0x02, 0x3E, 0x28,            
	0x02, 0x3B, 0x28,            
	0x0A, 0xFB, 0x01,            
	0x02, 0x3C, 0x1C
};
uint8 ui8ATBM886xDvbc30Dot4MSet[]=
{
	0x02, 0xC5, 0x05,
	0x02, 0x4E, 0x08,
	0x02, 0x3E, 0x1C,
	0x02, 0x3B, 0x1C
};
uint8 ui8AATBM886xDvbc24MSet[]=
{
	0x02, 0xC5, 0x15,            
	0x02, 0x4E, 0x10,            
	0x02, 0x3E, 0x28,            
	0x02, 0x3B, 0x28 
};



 /****************************************************************************
 Function:    ATBMSetDVBCMode
 Parameters:  none
 Return:      none   
 Description:
	  This function initializes ATBM886x in DVB-C mode, selects DVB-C receiving mode,
	  then calls ATBMInit function	
	  ATBMSetConfigParas function should be called before this function.
 ******************************************************************************/
void ATBMSetDVBCMode(void)
 {	  
	 stCustomConfig.ui8DtmbDvbcMode = ATBM_DVBC_MODE;
	
	 ATBMInit();  
 }

/****************************************************************************
Function:   ATBMDVBCSNR
Parameters:   None                    
Return:    DVBC signal-to-noise ratio in dB format

Description:
	Calculate  DVB-C SNR:			 
******************************************************************************/
int ATBMDVBCSNR(void)
{
	int ui32DVBCSNR;
	int    i32Tmp,i32Tmp1; 				

	ui32DVBCSNR = 0;
	ATBMDVBCRead(0x3C4,&i32Tmp);
	ATBMDVBCRead(0x3d0,&i32Tmp1);
	i32Tmp    = i32Tmp&0xffff;
	i32Tmp1   = (i32Tmp1&0x7)+1; //pow(2,i32Tmp1) means QAM		
    
	if(i32Tmp ==0)
	{    	
		i32Tmp = 1;
	}

	if((i32Tmp1==4) ||(i32Tmp1==6) ||(i32Tmp1==8))
	{
		if(i32Tmp!=0)
		{
// 			dbDVBCSNR = -20*log10(i32Tmp/65536.0)-0.8;//QAM=16,64,256
			ui32DVBCSNR = -2*(Log10Convert(i32Tmp)-Log10Convert(65536))-1;
		}
	}
	else if( (i32Tmp1==5) ||(i32Tmp1==7))
	{
		if(i32Tmp !=0)
		{
// 			dbDVBCSNR = -20*log10(i32Tmp/65536.0)-4.0;//QAM=32,128
			ui32DVBCSNR = -2*(Log10Convert(i32Tmp)-Log10Convert(65536))-4;

		}
	}
	else
	{
		ui32DVBCSNR=0;

	}	
	//  ATBM_PRINTF("DVBC SNR = %f dB\n",dbDVBCSNR);	
	return ui32DVBCSNR;

}

/****************************************************************************
Function:   ATBMDVBCBER
Parameters: BerExponent: scientific notation exponent                    
Return:     ui32BerCoeffient: RS corrected Bit error rate real number.
Description:
	Calculate DVB-C RS bit error rate. 
******************************************************************************/
uint32   ATBMDVBCBER(int *pi32BerExponent)
{
	uint8  ui8Adata[2];
	uint8  ui8Iter;	
	int    i32Tmp;
	int    i32TotalBits = 13369344; //SampleSize: 8192*204*8
	int    i32EPara = 0;
	int    i32CorrectedBerBits = 0;	
	uint32 ui32BerCoeffient =0;

	ATBMRead(0x15, 0x23,&ui8Adata[1]);
	ATBMRead(0x15, 0x22,&ui8Adata[0]);
	i32CorrectedBerBits = ui8Adata[1]*256 + ui8Adata[0];
	ATBMRead(0x15, 0x21,&ui8Adata[1]);
	ATBMRead(0x15, 0x20,&ui8Adata[0]);
	i32TotalBits        = (ui8Adata[1]*256 + ui8Adata[0])*204*8;		
	if((i32CorrectedBerBits ==0)||(i32TotalBits == 0))
	{
		ui32BerCoeffient = 0;
		*pi32BerExponent = 0;			
		//ATBM_PRINTF("PreRsBer = %f E- %d\n",dbBerCoeffient, *BerExponent);			
		return ui32BerCoeffient;
	}	
	for (ui8Iter=0;ui8Iter<12;ui8Iter++)
	{
		i32Tmp = i32CorrectedBerBits/i32TotalBits;
		if (i32Tmp)
		{
			ui32BerCoeffient = i32CorrectedBerBits/i32TotalBits;
			break;
		}
		else
		{
			i32EPara +=1;
			i32CorrectedBerBits *=10;
		}
	}	
	*pi32BerExponent  = i32EPara;
	//ATBM_PRINTF("PreRsBer = %f E- %d\n",dbBerCoeffient, *BerExponent);		
	return ui32BerCoeffient;
}

/****************************************************************************
Function:   ATBMDVBCUncorrectablePER
Parameters: i32pPktsExponent: scientific notation exponent                   
Return:   ui32PktsCoeffient: error packets rate real number.
Description:
	This function counts DVB-C ui8AData packet which contains uncorrected error and calculate packet error rates.
******************************************************************************/
uint32  ATBMDVBCUncorrectablePER(int *i32pPktsExponent )
{
	uint8  ui8AData[2],ui8Iter;
	int    i32Tmp;
	int    i32TotalPkts = 8192; //SampleSize: 8192 packets
	int    i32EPara = 0;
	int    i32UncorrectedPackets = 0;	
	uint32 ui32PktsCoeffient = 0;

	ATBMRead(0x15, 0x27,&ui8AData[1]);
	ATBMRead(0x15, 0x26,&ui8AData[0]);
	i32UncorrectedPackets = ui8AData[1]*256 + ui8AData[0];
	ATBMRead(0x15, 0x21,&ui8AData[1]);
	ATBMRead(0x15, 0x20,&ui8AData[0]);
	i32TotalPkts          = ui8AData[1]*256 + ui8AData[0];		

	if((i32UncorrectedPackets ==0)||(i32TotalPkts == 0))
	{
		ui32PktsCoeffient   = 0;
		*i32pPktsExponent = 0;			
		//ATBM_PRINTF("Pks Error = %f E- %d\n",dbPktsCoeffient, *i32pPktsExponent);			
		return ui32PktsCoeffient;
	}	
	for (ui8Iter=0;ui8Iter<12;ui8Iter++)
	{
		i32Tmp = i32UncorrectedPackets/i32TotalPkts;
		if (i32Tmp)
		{
			ui32PktsCoeffient = (i32UncorrectedPackets)/i32TotalPkts;
			break;
		}
		else
		{
			i32EPara += 1;
			i32UncorrectedPackets *=10;
		}
	}	
	*i32pPktsExponent  = i32EPara;
	//ATBM_PRINTF("Pks Error = %f E- %d\n",dbPktsCoeffient, *i32pPktsExponent);	
	return ui32PktsCoeffient;

}

/****************************************************************************
Function:     ATBMDVBCGetQAM
Parameters:   None                    
Return:       QAM_index
Description:
	Get DVB-C QAM index			
	4: 16QAM
	5: 32QAM
	6: 64QAM
	7: 128QAM
	8: 256QAM
******************************************************************************/
uint8   ATBMDVBCGetQAM(void)
{
	uint8 ui8QAMIndex; 				
	int   i32Tmp;

	ATBMDVBCRead(0x3d0,&i32Tmp);
	ui8QAMIndex = (uint8)((i32Tmp&0x7)+1); //pow(2,tmp) means QAM		
	//ATBM_PRINTF("DVB-C signal QAM index = %d \n", QAM_index);       
	return ui8QAMIndex;
}

/*****************************************************************************
Function:    ATBMDVBCSignalStrength
Parameters:  none
Return:      XgcGain
Description:
    This function checks DVB-C external IF gain value used by the XGC. The function 
    can be used for DVB-C signal strength evaluation.
    Different Tuner have different XgcGain Range. It is needed to test XgcGain value
    on different signal strength level to make a Tuner's private signal strength evaluation function.    
*****************************************************************************/
int ATBMDVBCSignalStrength(void)
{
	int i32XgcGain;
	int i32XgcGain1; 

	ATBMDVBCRead(0x114,&i32XgcGain1);	
	i32XgcGain = (int)(i32XgcGain1&0xfff);
//	ATBM_PRINTF(" XgcGain = 0x%x\n", XgcGain);
	return i32XgcGain;
}

/****************************************************************************
Function:		ATBMDVBCInit
Parameters:   stCustomConfig              
Return: 		None
Description:
	Call this function to configure all DVBC related registers.		 
******************************************************************************/
void ATBMDVBCInit( custom_config_t stCustomConfig)
{

	uint8  ui8RegH, ui8RegL , ui8Rst, ui8TmpData, ui8pll;
	uint8 ui8ATBMChipID;
	uint16 ui16ParamsDvbcRatio;
	int    i32TmpRst;
	//int    i32Length = 0;
	int    i32RifControl;
	//int    i32DebugData = 0;
	int	   i32ItbCoarseFrequencyOffset;
	int ui32OSCFreq = 0;
	int ui32IFFreq = 0;
	uint32 ui32SymbolRateM, ui32SysClk,ui32ItbCoarseFrequencyOffset;
	ui8ATBMChipID = ATBMChipID();
	ATBMDVBCWrite(0x04, 0x1);  //DVBC Module Reset
	if(stCustomConfig.stDvbcParams.ui8InputMode == DVBC_IF_INPUT)
	{
		i32RifControl = 0x01; //IF mode
		ATBMDVBCWrite(0x100, i32RifControl); 

		//886x special register
		if(ui8ATBMChipID == 0x40)
		{
			ATBMDVBCWrite(0x104,0x01);
			ATBMDVBCWrite(0x304,0x05);
		}	
		
	}else
	{
		i32RifControl = 0x00; //IQmode
		ATBMDVBCWrite(0x100, i32RifControl); 
		ATBMDVBCWrite(0x104,0x01);  //gain control   
		ATBMDVBCWrite(0x118, 0x00);
		ATBMWriteRegArray(ui8ADvbcCciAci,sizeof(ui8ADvbcCciAci)/sizeof(uint8)); 
	}

	ui32SymbolRateM = (stCustomConfig.stDvbcParams.i32SymbolRate); 

	//886x special register setting 
	if(ui8ATBMChipID == 0x40)
	{
		ui16ParamsDvbcRatio = (uint16)(ui32SymbolRateM*1032/1000);
		ui8RegH = ui16ParamsDvbcRatio/256;
		ui8RegL = ui16ParamsDvbcRatio&0xff;
		ATBMWrite(0x02, 0x53, ui8RegL); 
		ATBMWrite(0x02, 0x54, ui8RegH); 
		if(stCustomConfig.ui32SampleClkFrequency == 30400) //30.4MHz crystal
		{
			ATBMRead(0x06, 0x0e,&ui8TmpData);
			if(ui8TmpData&0x08)
			{
				ui8Rst =(uint8)(4778125/ui32SymbolRateM/40); 
			}else
			{
				ui8Rst =(uint8)(4180000/ui32SymbolRateM/40); 
			}
			ATBMWrite(0x15, 0x37, ui8Rst); 
		}
		if(stCustomConfig.ui32SampleClkFrequency == 24000) //24MHz crystal
		{
			ATBMRead(0x06, 0x0e,&ui8TmpData);
			if(ui8TmpData&0x08)
			{
				ui32SysClk = 64;
			}else
			{
				ui32SysClk = 72;
			}
			ui8Rst =(uint8)(6875*ui32SysClk/ui32SymbolRateM/4); 
			ATBMWrite(0x15, 0x37, ui8Rst); 
		}
	}
	//end for 886x

	ui32DvbcSymbolRate = stCustomConfig.stDvbcParams.i32SymbolRate;
	if(stCustomConfig.ui32SampleClkFrequency == 24000) //24MHz crystal
	{
		ui32OSCFreq = stCustomConfig.ui32SampleClkFrequency*2;	
		if(ui8ATBMChipID == 0x30)
		{
			ATBMDVBCWrite(0x104,0x14);
		}
	}
	else
	{
		ui32OSCFreq = stCustomConfig.ui32SampleClkFrequency;	
		if(ui8ATBMChipID == 0x30)
		{
			ATBMDVBCWrite(0x104,0x34);
		}
	}
	ui32DvbcOscFreq= ui32OSCFreq;	
	if(stCustomConfig.stDvbcParams.ui8InputMode == DVBC_IF_INPUT)
	{
		ui32IFFreq = stCustomConfig.stTunerConfig.ui32IFFrequency;	
		i32TmpRst = (ui32OSCFreq>ui32IFFreq?(ui32OSCFreq - ui32IFFreq):(ui32IFFreq -ui32OSCFreq)*-1);
		if (ui32OSCFreq>ui32IFFreq)
		{
			ui32ItbCoarseFrequencyOffset = i32TmpRst/100;
			ui32ItbCoarseFrequencyOffset = (8388608*ui32ItbCoarseFrequencyOffset/(ui32OSCFreq/100)*16);//no problem
			ATBMDVBCWrite(DVBC_COARSE_FREQUENCY_OFFSET_ADDR,ui32ItbCoarseFrequencyOffset);
		}
		else
		{
			i32ItbCoarseFrequencyOffset= (8388608*(i32TmpRst/100)/(ui32OSCFreq/100)*16);//no problem
			ATBMDVBCWrite(DVBC_COARSE_FREQUENCY_OFFSET_ADDR,i32ItbCoarseFrequencyOffset);
		}
	}
	ATBMDVBCSetCarrier(ui32OSCFreq,	ui32SymbolRateM);	 	
	ATBMDVBCSetSymbolRate(ui32OSCFreq, ui32SymbolRateM);		 
	ATBMDVBCSetQAM();	 

	ATBMDVBCWrite(0x20c,0x10); 
	ATBMDVBCWrite(0x204,0x400); //DVB-C symbol rate offset related	
	ATBMDVBCWrite(0x04, 0x00);  //DVBC Module start processing
	if(ui8ATBMChipID == 0x40)
	{
		if(stCustomConfig.ui32SampleClkFrequency == 24000) //24MHz crystal
		{
			ATBMWrite(0x01, 0x0c, 0x00); 
			Delayms(1);/*1 ms*/ 
			ATBMRead(0x06, 0x0e, &ui8pll);
			ATBMWrite(0x06, 0x04, 0x01); 
			ui8pll |= 0x01;
			ATBMWrite(0x06, 0x0e,ui8pll);	
			ui8pll &= 0xfe;
			ATBMWrite(0x06, 0x0e,ui8pll);
			ATBMWrite(0x06, 0x04, 0x00);
			Delayms(1);/*1 ms*/ 
			ATBMWrite(0x01, 0x0c, 0x01); 
		}
	}
}

/****************************************************************************
Function:		ATBMDVBCSetSymbolRate
Parameters:     ui32OscFreq: OSC sampling frequency, in KHz   
                ui32SymbolRateM, symbol rate, in Ksbps
Return: 		None
Description:
	Call this function to configure all DVB-C  symbol rate related registers.			 
******************************************************************************/
void ATBMDVBCSetSymbolRate(uint32 ui32OscFreq, uint32 ui32SymbolRateM)
{
	uint32  ui32Log2DecFactor;		
	uint32	ui32DecCicOutputShift; 
	uint32	ui32AfcSlowControlTC;	
	uint32	ui32TrlSampleRate;
	uint32	ui32TrlSampleRateRecip;
// 	uint32  ui32CicDcGain;		

	// Calculate factor by which decimation filter must reduce rate
// 	ui32Log2DecFactor = (uint32)(floor(log10(((dbCMS0022MaxSymbolRateRatio*dbOscFreq)/dbSymbolRateM))/log10(2.0)));
	ui32Log2DecFactor = (Log10Convert(ui32OscFreq)-Log10Convert(4)-Log10Convert(ui32SymbolRateM))/Log10Convert(2);
	// Samples per symbol
	if(ui32Log2DecFactor > 3)
	{
		ui32Log2DecFactor=3;
	}
//  	ui32TrlSampleRate = ((1<<16)*2/(dbSmpsPerSymb/(1<<ui32Log2DecFactor)));
	ui32TrlSampleRate = 131072*FixPow(2,ui32Log2DecFactor)*ui32SymbolRateM/ui32OscFreq;
	ATBMDVBCWrite(DVBC_SAMPLE_RATE_ADDR, ui32TrlSampleRate);

//  ui32TrlSampleRateRecip = (int)(((1<<11)*(dbSmpsPerSymb/(1<<ui32Log2DecFactor))/2));
	ui32TrlSampleRateRecip = 1024*FixPow(2,ui32Log2DecFactor)*ui32OscFreq/ui32SymbolRateM;
	                            

	if(ui32TrlSampleRateRecip > 0x3fff)
	{
		ui32TrlSampleRateRecip = 0x3fff;
	}
	ATBMDVBCWrite(DVBC_SAMPLE_RATE_RECIP_ADDR, ui32TrlSampleRateRecip);
	// Calculate CIC gain
// 	dbCicDcGain  = pow(2*(1<<(ui32Log2DecFactor-1)),(int)dbCMS0022CIC2DELAY);
// 	//Calculate CIC gain *=
// 	dbCicDcGain *= pow(3*(1<<(ui32Log2DecFactor-1)),(int)dbCMS0022CIC3DELAY);
	ui32DecCicOutputShift = 1;

	if(ui32Log2DecFactor ==1)
	{
		ui32DecCicOutputShift = 4;
	}
	if(ui32Log2DecFactor == 2)
	{
		ui32DecCicOutputShift = 7;
	}
	if(ui32Log2DecFactor == 3)
	{
		ui32DecCicOutputShift = 10;
	}

// 	ui32CicDcGain = 1;

// 	ui32DecCicOutputShift = (ui32Log2DecFactor > 0) ? ((uint32)((log10(dbCicDcGain)/log10(2.0))+0.5)+1) : 1;
// 	ui32DecCicOutputShift = (ui32Log2DecFactor > 0) ? (((Log10Convert(ui32CicDcGain)/Log10Convert(2))+1)) : 1;
	ATBMDVBCWrite(DVBC_OUTPUT_SHIFT_ADDR, ui32DecCicOutputShift);
	ATBMDVBCWrite(DVBC_DECIMATION_FACTOR_ADDR,ui32Log2DecFactor);
	// Program AFC tracking time constant as function of symbol rate
	ui32AfcSlowControlTC = (1<<(ui32Log2DecFactor+2));
	ATBMDVBCWrite(DVBC_SLOW_CONTROL_TC_ADDR,ui32AfcSlowControlTC);
}

/****************************************************************************
Function:	ATBMDVBCSetCarrier
Parameters: ui32OscFreq: OSC sampling frequency, 30400, in kHz   
            ui32SymbolRateM, symbol rate. 
Return: 	None
Description:
	Call this function to configure DVBC registers according to OSC frequency  
	and symbol rate. 
******************************************************************************/
void   ATBMDVBCSetCarrier(uint32 ui32OscFreq,uint32 ui32SymbolRateM)
{
	int	    i32IfSrchStartFreq;
	int	    i32IfSrchStepFreq;
	uint32	ui32TimSearchLockTimeout;
	uint32	ui32PlCarrierFrequencyRange;
	uint32	ui32CarrierLockAcquireTimeout;
	uint32  ui32IfSearchStartFoOvrFsamp; 
	uint32  ui32IfSearchStepFstpOvrFsamp;  
// 	double  dbWinEndFreqMHz = 3*dbSymbolRateM/20;

// 	ui32IfSearchStartFoOvrFsamp = /*dbWinEndFreqMHz*/3*ui32SymbolRateM/ui32OscFreq/20;
	ui32IfSearchStartFoOvrFsamp = /*dbWinEndFreqMHz*/393216*ui32SymbolRateM/(ui32OscFreq/5);
	if(ui32SymbolRateM != 0)
	{
// 		ui32IfSearchStepFstpOvrFsamp = ui32SymbolRateM*5/ui32OscFreq/100;
		ui32IfSearchStepFstpOvrFsamp = 131072*ui32SymbolRateM/(ui32OscFreq/5);
	}
	else
	{
		ui32IfSearchStepFstpOvrFsamp = 0;
	}
// 	i32IfSrchStartFreq = (int)(((dbIfSearchStartFoOvrFsamp)*(1<<27))+0.5); 
	i32IfSrchStartFreq = ui32IfSearchStartFoOvrFsamp*1024/100; 

	ATBMDVBCWrite(DVBC_SEARCH_RANGE_ADDR,i32IfSrchStartFreq);

	i32IfSrchStepFreq = ui32IfSearchStepFstpOvrFsamp*1024/100;
	ATBMDVBCWrite(DVBC_SEARCH_STEP_ADDR,i32IfSrchStepFreq);	
	// Program parameters for fine stepped carrier search
	// Step size
	ui32TimSearchLockTimeout = 20;	// 20 for 1% step;
	ATBMDVBCWrite(DVBC_PL_CARRIER_STEP_FREQUENCY_ADDR,ui32TimSearchLockTimeout);
	// Search range
	ui32PlCarrierFrequencyRange = 39;// 39 for +/-15% limits
	ATBMDVBCWrite(DVBC_PL_CARRIER_FREQUENCY_RANGE_ADDR,ui32PlCarrierFrequencyRange);
	// Standard times and thresholds for all other QAM levels
	ui32CarrierLockAcquireTimeout = 4;   	
	ATBMDVBCWrite(DVBC_CARRIER_LOCK_ACQUIRE_TIMEOUT_ADDR, ui32CarrierLockAcquireTimeout);

}

/****************************************************************************
Function:	ATBMDVBCSetQAM
Parameters: none                
Return: 	None
Description:Call this function to configure DVBC QAM related registers:			 
******************************************************************************/
void ATBMDVBCSetQAM(void)
{
	int	i32BitSyncDetectTimeout;   
	ATBMDVBCWrite(DVBC_AUTO_EQU_SEARCH_ADDR,0x3f8); //Auto QAM detect
	i32BitSyncDetectTimeout = 60000/(256*16); 
	ATBMDVBCWrite(DVBC_BITSYNC_DETECT_TIMEOUT_ADDR,i32BitSyncDetectTimeout);
}

void   ATBMSetConfigParas(custom_config_t stCustomConfigp)
{
	stCustomConfig = stCustomConfigp;
}

/****************************************************************************
Function:     ATBMInit
Parameters:   none
Return:       none  
Description:      
    Initializes ATBM886x demodulator and configures all necessary registers. 
    The demodulator will start processing incoming data after this function is called.                       
******************************************************************************/
void ATBMInit(void)
{    
	uint8 ui8ATBMChipID;
	Delayms(1);/*1 ms*/ 
	ATBMWrite(0x01, 0x0c, 0x01);//i2c clock using PLL, 1:PLL, 0:Crystal.
	//Delayms(10); /*10 ms*/   
	ATBMWrite(0x00, 0x04, 0x00);	
	ATBMWrite(0x10, 0xf7, 0xe8);	
	ATBMWrite(0x10, 0xfb, 0x07);	
	ui8ATBMChipID = ATBMChipID();
	if(ui8ATBMChipID == 0x40)
	{
		ATBMWriteRegArray(ui8AATBM886xCommonReg,sizeof(ui8AATBM886xCommonReg)/sizeof(unsigned char)); 	
	}
	ATBMConfig( stCustomConfig);
	if(stCustomConfig.ui8DtmbDvbcMode == ATBM_DVBC_MODE) 
	{
		ATBMWriteRegArray(uiA8DvbcInternal,sizeof(uiA8DvbcInternal)/sizeof(unsigned char)); 

		if(stCustomConfig.stDvbcParams.ui8InputMode == DVBC_IF_INPUT)
		{
			ATBMWrite(0x15, 0x11, 0x01); 
			if(ui8ATBMChipID == 0x40)
			{
				ATBMWrite(0x15, 0x12, 0x00); 
			}
			if(ui8ATBMChipID == 0x30)
			{
				ATBMWrite(0x15, 0x12, 0x01); 
			}
			ATBMWrite(0x00, 0x16, 0x00); 
		}
		if(stCustomConfig.stDvbcParams.ui8InputMode == DVBC_IQ_INPUT)
		{
			ATBMWrite(0x00, 0x16, 0x03); 
		}
		if(ui8ATBMChipID == 0x40)
		{
			ATBMWrite(0x02, 0x52, 0x01);
			ATBMWriteRegArray(ui8AATBM886xDvbcSet,sizeof(ui8AATBM886xDvbcSet)/sizeof(uint8)); 
			if(stCustomConfig.ui32SampleClkFrequency == 24000) 
			{
				ATBMWriteRegArray(ui8AATBM886xDvbc24MSet,sizeof(ui8AATBM886xDvbc24MSet)/sizeof(uint8));
			}
			if(stCustomConfig.ui32SampleClkFrequency == 30400)
			{
				ATBMWriteRegArray(ui8ATBM886xDvbc30Dot4MSet,sizeof(ui8ATBM886xDvbc30Dot4MSet)/sizeof(uint8));
			}
		}
		ATBMDVBCInit(stCustomConfig);                     
	}
	else //DTMB mode
	{  
		ATBMWrite(0x00, 0x0a, 0x01); 
		ATBMWrite(0x00, 0x09, 0x01);  	     
		ATBMWrite(0x00, 0x13, 0x00); //DTMB mode
		ATBMWrite(0x15, 0x18, 0x00);			
		ATBMWrite(0x15, 0x15, 0x00); 	
		ATBMWrite(0x15, 0x11, 0x00); 
		ATBMWrite(0x15, 0x12, 0x00);
		if(ui8ATBMChipID == 0x40)
		{
			ATBMWrite(0x02, 0x52, 0x00);
			ATBMWriteRegArray(ui8AATBM886xDtmbSet,sizeof(ui8AATBM886xDtmbSet)/sizeof(unsigned char)); 	
			if(stCustomConfig.ui32SampleClkFrequency == 24000) 
			{
				ATBMWriteRegArray(ui8AATBM886xDtmb24MSet,sizeof(ui8AATBM886xDtmb24MSet)/sizeof(unsigned char));
			}
			if(stCustomConfig.ui32SampleClkFrequency == 30400)
			{
				ATBMWriteRegArray(ui8AATBM886xDtmb30Dot4MSet,sizeof(ui8AATBM886xDtmb30Dot4MSet)/sizeof(unsigned char));
			}
		}	 
		ATBMWriteRegArray(ui8ADtmbInternal,sizeof(ui8ADtmbInternal)/sizeof(unsigned char)); 			
	}

	ATBMWrite(0x08,0x00,0xa0);
	ATBMWrite(0x08,0x01,0x16);
	ATBMWrite(0x00, 0x04, 1);	// write 1 config done

#if ATMB_DBG_OUTPUT
	ATBMDebugRegister();
#endif  
}

/****************************************************************************
Function:    ATBMConfig
Parameters:  custom_config
Return:      none  

Description:  
     Set registers according to customer's configuration, 
     This function configures ATBM886x. 
 ******************************************************************************/
 void ATBMConfig( custom_config_t stCustomConfigp)
 {
	 /*Demodulator default clock 30.4MHz; Tuner IF 36.166Mhz*/
	 ATBMSetOSC(stCustomConfigp.stTunerConfig,stCustomConfigp.ui32SampleClkFrequency);	
	 /*Configures  MPEG TS output  mode*/		 	
	 ATBMSetTSMode(stCustomConfigp.stTsMode);   	 
	 ATBMWrite(0x01, 0x02, 1);	 //I2C related
	 ATBMWrite(0x02, 0x0c, ui8ATBMSoftwareVersion);
 }

 /****************************************************************************
Function:    ATBMSetTSMode
Parameters:  stTSMode
Return:      none  
Description:
      This function configures Transport stream interface mode.   		 
******************************************************************************/
void ATBMSetTSMode( MPEG_TS_mode_t stTSMode)
{
	uint8 ui8ChipID;
	uint8 ui8SerialEnabled =0x0;/*parallel clock out */
	uint8 ui8TSOutputEdegeMode = stTSMode.ui8OutputEdge;  
	uint8 ui8TsClockMode = stTSMode.ui8SPIClockConstantOutput;  

	/*0x00: SPI data align with rising edge of TSBCLK; 0x01: SPI data align with falling edge of TSBCLK*/
	ATBMWrite(0x03,0x01, ui8TSOutputEdegeMode); 	
	ATBMWrite(0x03,0x06, ui8TsClockMode);  /*either free running or only active when data are valid*/
	ATBMWrite(0x03,0x11, 0);  
	ATBMRead(0x00,0x00, &ui8ChipID);
	if(ui8ChipID == 0x40)
	{
		ATBMWrite(0x03,0x12, 1);//886x special
		ATBMWrite(0x0b,0x24, 0x01);
	}
	if(stTSMode.ui8TSTransferType==TS_SERIAL_MODE)
	{    
		ui8SerialEnabled = 0x01;    /*enable serial TS mode */   

		//SSI_sync_hold_cycle = 0x00; //0x00:1 bit sync;0x01: 8 bit sync
		/*0: serial sync  holds 1 cycle; 1: serial sync holds 8 cycles*/
		//ATBMWrite(0x03,0x0f, SSI_sync_hold_cycle);  

		/*0:disable serial mode; 1: enable serial mode */  
		ATBMWrite(0x03,0x05, ui8SerialEnabled);      
		if(stTSMode.ui8TSSSIOutputSelection == TS_SSI_OUTPUT_ON_DATA_BIT0)
		{
			ATBMWrite(0x03, 0x1f, 0x00); 
		}else
		{
			ATBMWrite(0x03, 0x1f, 0x01); 
		}
	}
	else	
	{	  
		ui8SerialEnabled =0x0; /*parallel clock output */
		/*0: disable serial mode; 1:enable serial mode */	
		ATBMWrite(0x03,0x05, ui8SerialEnabled);  		
		if(stTSMode.ui8TSSPIMSBSelection == TS_SPI_MSB_ON_DATA_BIT7)
		{
			ATBMWrite(0x03, 0x1f, 0x00); 
		}else
		{
			ATBMWrite(0x03, 0x1f, 0x01); 
		}
	}
}

/****************************************************************************
Function:    ATBMSetOSC
Parameters:  stTunerConfig, ui32SampleClkFrequency
Return:      1 for success,0 for error  
Description:
     This function configures ATBM886x according to external Clock source 
     and  Tuner IF frequency.
 ******************************************************************************/
int ATBMSetOSC( tuner_config_t stTunerConfig, uint32 ui32SampleClkFrequency) 
{
	/*Demodulator default registers are for 30.4 MHz crystal, no need to list here.*/	
	uint8  ui8ARXFLT[6]; 
	uint8  ui8BandWidth;
	uint8  ui8SwapIQ;	
	uint8  ui8ARge[3], ui8ChipID;	
	uint8 ui8Tmp;
	uint16 ui16ParamsFSADC, ui16ParamsIFFreq;
	uint32 ui32ParamsFSADCInverse;
	int    i32FreqLifTmp;
	int    i32RateRatioTmp;
	uint32 ui32TunerIFFrequency = stTunerConfig.ui32IFFrequency/100;	   
	uint32 ui32CLKFreq = ui32SampleClkFrequency/100;//0.1MHz origin is MHz
	ATBMRead(0x00,0x00, &ui8ChipID);
	if(stTunerConfig.ui8IQmode == SWAP_IQ)
	{	 	
		ui8SwapIQ = 0x03;   
		ATBMWrite(0x0c,0x01, ui8SwapIQ); 
	}
	else
	{
		ATBMWrite(0x0c,0x01, 0x01); 
	}
	/* 0x0A03~0x0A05 are only related to sampling frequency; 
	* 0x0A00~0x0A02 are related to sampling frequency and tuner IF frequency.*/			
	if(ui32TunerIFFrequency>ui32CLKFreq) 
	{
		i32FreqLifTmp=(26353589*(ui32TunerIFFrequency-ui32CLKFreq)/ui32CLKFreq);
	}
	else //low IF
	{
		i32FreqLifTmp=(26353589*ui32TunerIFFrequency/ui32CLKFreq);
	}
	ui8ARXFLT[0] = (uint8)(i32FreqLifTmp&0xff);
	ui8ARXFLT[1] = (uint8)((i32FreqLifTmp>>8)&0xff);
	ui8ARXFLT[2] = (uint8)((i32FreqLifTmp>>16)&0xff);

	ATBMWrite(0x0A,0x00, ui8ARXFLT[0]);
	ATBMWrite(0x0A,0x01, ui8ARXFLT[1]);
	ATBMWrite(0x0A,0x02, ui8ARXFLT[2]);   
	//ATBM_PRINTF("IF freq_lif: 0x%x 0x%x  0x%x\n", ui8ARXFLT[0], ui8ARXFLT[1], ui8ARXFLT[2]);

	ui8BandWidth = stTunerConfig.ui8DTMBBandwithMHz;
	i32RateRatioTmp = (4194304*ui32CLKFreq/(ui8BandWidth*152));

	//i32RateRatioTmp=(int)floor(2*dbCLKFreq*(1<<19)/30.4 + 0.5);
	ui8ARXFLT[3] = (uint8)(i32RateRatioTmp&0xff);
	ui8ARXFLT[4] = (uint8)((i32RateRatioTmp>>8)&0xff);
	ui8ARXFLT[5] = (uint8)((i32RateRatioTmp>>16)&0x1f);
	ATBMWrite(0x0A,0x03, ui8ARXFLT[3]);
	ATBMWrite(0x0A,0x04, ui8ARXFLT[4]);
	ATBMWrite(0x0A,0x05, ui8ARXFLT[5]);   	
	//ATBM_PRINTF("ADC rate ratio: 0x%x 0x%x  0x%x\n", ui8ARXFLT[3], ui8ARXFLT[4], ui8ARXFLT[5]);	
	if(ui32CLKFreq == 240) //24MHz crystal, register should set here.
	{
		ATBMRead(0x06,0x0e,&ui8Tmp);
		ATBMWrite(0x06,0x0e, ui8Tmp|0x4);
		if(ui8ChipID == 0x30)
		{
			ATBMWrite(0x0a,0xfb,0x02);
		}
		if(ui8ChipID == 0x40)
		{
			ATBMWrite(0x0a,0xfb,0x01);
		}
	}
	ATBMRead(0x00,0x00, &ui8ChipID);
	if(ui8ChipID == 0x40)
	{
		ui16ParamsFSADC = (uint16)(ui32CLKFreq * 2048/10);
		ui8ARge[1] = ui16ParamsFSADC/256;
		ui8ARge[0] = ui16ParamsFSADC&0xff;
		ATBMWrite(0x02, 0x33, ui8ARge[0]);
		ATBMWrite(0x02, 0x34, ui8ARge[1]);
		ui32ParamsFSADCInverse = 20971520/ui32CLKFreq;
		ui8ARge[0] = (uint8)(ui32ParamsFSADCInverse&0xff);
		ui8ARge[1] = (uint8)((ui32ParamsFSADCInverse>>8)&0xff);
		ui8ARge[2] = (uint8)((ui32ParamsFSADCInverse>>16)&0x01);
		ATBMWrite(0x02, 0x9d, ui8ARge[0]);
		ATBMWrite(0x02, 0x9e, ui8ARge[1]);
		//ATBMWrite(0x02, 0x9f, ui8ARge[2]);
		if(ui32CLKFreq == 240) //24MHz crystal, register should set here.
		{
			ui8ARge[2] = ui8ARge[2]|0x02;
			ATBMWrite(0x02, 0x9f, ui8ARge[2]);
		}
		if(ui32CLKFreq == 304) //30.4MHz crystal, register should set here.
		{					
			ATBMWrite(0x02, 0x9f, ui8ARge[2]);
		}
		if(ui32TunerIFFrequency > ui32CLKFreq) 
		{
			ui16ParamsIFFreq = (ui32TunerIFFrequency-ui32CLKFreq)*2048/10;//to mhz
		}
		else //low IF
		{
			ui16ParamsIFFreq = ui32TunerIFFrequency*2048/10;
		}
		ui8ARge[1] = ui16ParamsIFFreq/256; 
		ui8ARge[0] = ui16ParamsIFFreq&0xff;
		ATBMWrite(0x02, 0x31,  ui8ARge[0]);
		ATBMWrite(0x02, 0x32,  ui8ARge[1]);
		if(ui8BandWidth == 8)
		{	
			ATBMWrite(0x02, 0x51, 0x00);
		}
		else if(ui8BandWidth == 7)
		{
			ATBMWrite(0x02, 0x51, 0x01);
		}
		else if(ui8BandWidth == 6)
		{
			ATBMWrite(0x08, 0x44, 0x00); 
			ATBMWrite(0x14, 0x8a, 0x0f);
			ATBMWrite(0x02, 0x51, 0x02);
		}else
		{
			ATBMWrite(0x02, 0x51, 0x00);
		}
		if(ui32CLKFreq == 240) 
		{
			ATBMWrite(0x02, 0x90, 0xd0); 
			ATBMWrite(0x02, 0x91, 0x93); 
			ATBMWrite(0x02, 0x92, 0x01); 
		}
		if(ui32CLKFreq == 304) 
		{
			ATBMWrite(0x02, 0x90, 0x00); 
			ATBMWrite(0x02, 0x91, 0xe0); 
			ATBMWrite(0x02, 0x92, 0x01); 
		}
	}	
	return 1;			
}

/****************************************************************************
Function:    ATBMGetTPS
Parameters:  none
Return:      TPS index
Description:
      This function checks signal TPS. The return value is TPS index in GB20600-2006 		 
 ****************************************************************************/
uint8 ATBMGetTPS(void)
{  
	uint8 ui8TPS;
    	if(ATBM_I2CREADWRITE_OK == ATBMRead(0x0d,0x04,&ui8TPS))
		return ui8TPS;
	else
		return 0xff;
}


/****************************************************************************
Function:    ATBMDebugRegister
Parameters:  none
Return:      none
Description:
      This function checks Demodulator registers,  used for debug purpose 
 ****************************************************************************/
void ATBMDebugRegister(void)
{
   /*common register*/
	uint8 ui8SoftwareVersion;		
	uint8  ui8dDmodState;	
	uint8  ui8ConfigDone;
 	uint8  ui8FreqOffset; 	
 	uint8  ui8SDPReset;	
	uint8  ui8I2CByPassState;
	uint8  ui8CANTuner;
	uint8  ui8ARXFLT[6];
	/*tuner related config*/
	uint8  ui8ADCConfig;	
	uint8  ui8LNAOnOff;
	uint8  ui8FatalRst, ui8WeakRst;
	/*TS interface configuration*/
	uint8 ui8SamplingEdegeMode;   
	uint8 ui8SerialEnabled;
	uint8 ui8ClockOutSelect;
	
	ATBMRead(0x02, 0x0c,&ui8SoftwareVersion);	
	ATBMRead(0x06, 0x07,&ui8dDmodState);	
	ATBMRead(0x00, 0x04,&ui8ConfigDone);	
	ATBMRead(0x00, 0x0A,&ui8SDPReset);	
	printk("Current software version: %d\n", ui8SoftwareVersion);	
	printk("ui8dDmodState:0x%x configure done:0x%x ui8SDPReset:0x%x\n",ui8dDmodState, ui8ConfigDone,ui8SDPReset);
	
	/*Demod passing I2C commands between host and tuner. 0:disable 1:enable.*/	
	ATBMRead(0x01, 0x03,&ui8I2CByPassState); 	
   	ATBMRead(0x00, 0x01,&ui8CANTuner);	
	printk("bypass state:0x%x; Tuner used(silicon tuner=0):0x%x\n", ui8I2CByPassState, ui8CANTuner);
	   	
	ATBMRead(0x06, 0x02,&ui8ADCConfig);	
	ATBMRead(0x10, 0x30,&ui8LNAOnOff);
	printk("tuner ui8ADCConfig: 0x%x; ui8LNAOnOff: 0x%x\n", ui8ADCConfig, ui8LNAOnOff);
	
   	ATBMRead(0x08, 0x00,&ui8FreqOffset);	
	printk("ui8FreqOffset 0x08_0x00: 0x%x\n", ui8FreqOffset);
	ATBMRead(0x08, 0x01,&ui8FreqOffset);	
	printk("ui8FreqOffset 0x08_0x01: 0x%x\n", ui8FreqOffset);
    
	ATBMRead(0x0A, 0x00,&ui8ARXFLT[0]);
	ATBMRead(0x0A, 0x01,&ui8ARXFLT[1]);
	ATBMRead(0x0A, 0x02,&ui8ARXFLT[2]);
	ATBMRead(0x0A, 0x03,&ui8ARXFLT[3]);
	ATBMRead(0x0A, 0x04,&ui8ARXFLT[4]);
	ATBMRead(0x0A, 0x05,&ui8ARXFLT[5]);
	printk("IF freq_lif: 0x%x 0x%x  0x%x\n", ui8ARXFLT[0], ui8ARXFLT[1], ui8ARXFLT[2]);
	printk("ADC rate ratio: 0x%x 0x%x  0x%x\n", ui8ARXFLT[3], ui8ARXFLT[4], ui8ARXFLT[5]);    	
	
	ATBMRead(0x00, 0x09,&ui8FatalRst);	
	ATBMRead(0x00, 0x0a,&ui8WeakRst);
	printk("DSP reset options ui8FatalRst = 0x%x, ui8WeakRst =  0x%x\n", ui8FatalRst, ui8WeakRst);

	ATBMRead(0x03, 0x01,&ui8SamplingEdegeMode);/*0x00: rising edge TS output; 0x01: falling edge TS output*/	  	                                                                                                                          
	ATBMRead(0x03, 0x05,&ui8SerialEnabled);    /*0: disable serial mode; 1: enable serial mode */
	ATBMRead(0x03, 0x0b,&ui8ClockOutSelect);   /*0: SPI clock output; 1: TS serial clock output */  	

	printk("TS interface ui8SerialEnabled= %d ui8SamplingEdegeMode =%d, clock select ui8ClockOutSelect%d\n",\
		ui8SerialEnabled,ui8SamplingEdegeMode, ui8ClockOutSelect);	  
}

/****************************************************************************
Function:    ATBMWriteRegArray
Parameters:  *ui8ARegTable, i32TableLen
Return:      none
Description:
      This function writes a group of registers to demodulator	 	 
 ****************************************************************************/
void ATBMWriteRegArray(uint8 *ui8ARegTable, int i32TableLen)
{
   int i32Iter;
   for(i32Iter=0;i32Iter<i32TableLen;i32Iter+=3)
   {
         ATBMWrite(ui8ARegTable[i32Iter],ui8ARegTable[i32Iter+1],ui8ARegTable[i32Iter+2]);
   }
}

/****************************************************************************
Function:    ATBMDebugRegArray
Parameters:  *ui8ARegTable, i32TableLen
Return:      none
Description:
     This function reads a group of registers from demodulator and prints them
     for debug purposes.	 
 ****************************************************************************/
void ATBMDebugRegArray(uint8 *ui8ARegTable, int i32TableLen)
{
   int i32Iter;
   uint8 ui8RegData;
   for(i32Iter=0;i32Iter<i32TableLen;i32Iter+=3)
   {
        ATBMRead(ui8ARegTable[i32Iter],ui8ARegTable[i32Iter+1],&ui8RegData);		
   }
}

/****************************************************************************
Function:   ATBMDVBCWrite
Parameters: ui32AAddress: Register physical address ;ui32Data:data need to write to the register
ui32Data - 32bit data 
Return:    I2C write status:ATBM_I2CREADWRITE_OK is OK  

Description:
	For DVB-C register write, It is an encapsulated function by calling ATBMTransWrite function.			 
******************************************************************************/
ATBM_I2CREADWRITE_STATUS ATBMDVBCWrite(uint32 ui32AAddress, uint32 ui32Data)
{

	uint8 ui8AWData[4];
	uint8 ui8AddrL, ui8AddrH;


	ui8AddrL = ui32AAddress&0xff;
	ui8AddrH = (ui32AAddress>>8)&0xff;
        
	ui8AWData[0]=ui32Data&0xff,ui8AWData[1]=(ui32Data>>8)&0xff,ui8AWData[2]=(ui32Data>>16)&0xff,ui8AWData[3]=(ui32Data>>24)&0xff;

	//Add mutex lock here if there is multi-thread operation of demodulator's I2C
	//the following I2C operations will not be interrupted. 
	DVBCWriteValue(0x15, 0x03, 0x0);		
	DVBCWriteValue(0x15, 0x01, ui8AddrL);
	DVBCWriteValue(0x15, 0x02, ui8AddrH);

	DVBCWriteValue(0x15, 0x04, ui8AWData[0]);
	DVBCWriteValue(0x15, 0x05, ui8AWData[1]);
	DVBCWriteValue(0x15, 0x06, ui8AWData[2]);	
	DVBCWriteValue(0x15, 0x07, ui8AWData[3]);

	DVBCWriteValue(0x15, 0x03, 0x01);	

	DVBCWriteValue(0x15, 0x03, 0x0);
	//Release mutex lock.
	ATBM_PRINTF(" %x %x\n", ui32AAddress, ui32Data);
	return ATBM_I2CREADWRITE_OK;
}


/****************************************************************************
Function:      ATBMDVBCRead
Parameters:    ui32AAddress: Register physical address ;ui32pValue:32-bit data read from target register 
Return:        I2C write status:ATBM_I2CREADWRITE_OK is OK    

Description:
	For DVB-C register read, It is an encapsulated function by calling
	ATBMTransWrite and ATBMTransRead functions.
******************************************************************************/
ATBM_I2CREADWRITE_STATUS ATBMDVBCRead(uint32  ui32AAddress,uint32 *ui32pValue)
{

	uint8 ui8RoLatchOn=1,ui8RoLatchOff=0;
	uint8 ui8Data[4];
	uint8 ui8AddrL, ui8AddrH;
	uint32 ui32Ddata;
	ui8AddrL = ui32AAddress&0xff;
	ui8AddrH = (ui32AAddress>>8)&0xff;

	//Add mutex lock here if there is multi-thread operation of demodulator's I2C
	//the following I2C operations will not be interrupted. 

	DVBCWriteValue(0x08, 0x4d, ui8RoLatchOff);
	DVBCWriteValue(0x15, 0x03, 0x0);		
	DVBCWriteValue(0x15, 0x01, ui8AddrL);
	DVBCWriteValue(0x15, 0x02, ui8AddrH);

	DVBCWriteValue(0x08, 0x4d, ui8RoLatchOn);
	DVBCReadValue(0x15, 0x08,&ui8Data[0]);
	DVBCReadValue(0x15, 0x09,&ui8Data[1]);
	DVBCReadValue(0x15, 0x0a,&ui8Data[2]);
	DVBCReadValue(0x15, 0x0b,&ui8Data[3]);
	DVBCWriteValue(0x08, 0x4d, ui8RoLatchOff);

        //Release mutex lock.

	ui32Ddata = (ui8Data[3]<<24) + (ui8Data[2]<<16) + (ui8Data[1]<<8) +ui8Data[0];	
	*ui32pValue = ui32Ddata;
	return ATBM_I2CREADWRITE_OK;
}

/****************************************************************************
Function:      ATBMCarrierOffset
Parameters:    none
Return:        i32CarrierOffset1:(KHz)

Description:
      This function checks DTMB signal carrier offset on field testing environment.
      This function is not meant to be used by customers. It's a debug utility to be used by FAEs.
 ****************************************************************************/
int ATBMCarrierOffset(void)
{
	uint32 mCarrierOffset;
	int CarrierOffset;
	uint8 pbuff[4];
	uint8 LockValue;

	LockValue =1; /*lock register value*/
	ATBMWrite(0x08,0x4d, LockValue);

	ATBMRead(0x08, 0x27, &pbuff[0]);//carrier_offset_est[7:0]
	ATBMRead(0x08, 0x28, &pbuff[1]);//carrier_offset_est[15:8]
	ATBMRead(0x08, 0x29, &pbuff[2]);//carrier_offset_est[21:16]

	LockValue = 0; /*lock register value*/
	ATBMWrite(0x08,0x4d, LockValue);

	/*=  carrier offset  =*/
	if(pbuff[2]&0x80)
	{
		mCarrierOffset = (0xffffff+1- (((pbuff[2]&0xff)<<16)+(pbuff[1]<<8)+pbuff[0]))*143/1000;
		mCarrierOffset = (mCarrierOffset/1000);
		CarrierOffset = 0 - mCarrierOffset;
		ATBM_PRINTF("DTMB signal  Carrier Offset = -%d KHz\n", mCarrierOffset);	
	}
	else
	{
		mCarrierOffset= ((((pbuff[2]&0xff)<<16)+(pbuff[1]<<8)+pbuff[0]))*143/1000;
		mCarrierOffset = mCarrierOffset/1000;
		CarrierOffset = mCarrierOffset;			
		ATBM_PRINTF("DTMB signal Carrier Offset = %d KHz\n", mCarrierOffset);                
	}
	return CarrierOffset;
}


/****************************************************************************
Function:      ATBM_DVBC_CarrierOffset
Parameters:    none
Return:        DVBC_CarrierOffset(KHz)

Description:
	This function checks DVB-C signal carrier offset on field testing environment.
	This function is not meant to be used by customers. It's a debug utility to be used by FAEs.
****************************************************************************/
int    ATBMDVBCCarrierOffset(void)
{  
	int tmp;
	int dtmp;
	uint32 acq_status;
	int ItbCarFreqOffset_stat,ItbCoarseFrequencyOffset,ChannelCentreFrequency;
	int DVBC_CarrierOffset;
	//-------Carrier frequency offset, reg ItbCarFreqOffset_stat---------------
	ATBMDVBCRead(0x3C0, &tmp); //32bit signed
	//ATBM_PRINTF("DVB-C carrier offset 0x3C0 = %d\n", tmp);
	dtmp = tmp;
	if(tmp<0)
		dtmp = -tmp; 
	ItbCarFreqOffset_stat = (dtmp/1000)*(ui32DvbcSymbolRate/10)/53687;//khz
	if(tmp<0)
	ItbCarFreqOffset_stat = - ItbCarFreqOffset_stat;
	//-----ChannelCentreFrequency , reg ItbFrequencyStatus------------------------
	ATBMDVBCRead(0x11c, &tmp); //27bit signed  
	//ATBM_PRINTF("DVB-C carrier offset 0x11C= %d\n",tmp);    

	tmp = tmp&0x7ffffff;
	dtmp = tmp;
	if(tmp&(0x1<<26))
	{
		dtmp=-(0x7ffffff-dtmp+1);
	}
	if(dtmp<0)
	{
		dtmp = -dtmp;
		ChannelCentreFrequency = (dtmp/100)*(ui32DvbcOscFreq/100)/13422; //khz
		ChannelCentreFrequency = -ChannelCentreFrequency;
	}else
	{
		ChannelCentreFrequency = (dtmp/100)*(ui32DvbcOscFreq/100)/13422; //khz
	}
	//------ItbCoarseFrequencyOffset--------------------------------------------
	ATBMDVBCRead(0x118, &tmp); //32bit signed      
	dtmp = tmp;
	if (tmp<0)
	{
		dtmp = -tmp;
	}
	//ATBM_PRINTF("DVB-C carrier offset 0x118  = %d\n", tmp);
	ItbCoarseFrequencyOffset = (dtmp/100)*(ui32DvbcOscFreq/100)/13422; //khz
	//-----------  calculate overall_CFO -------------
	if(ChannelCentreFrequency<0)
	{
		DVBC_CarrierOffset =  ChannelCentreFrequency  + ItbCoarseFrequencyOffset+ ItbCarFreqOffset_stat;
	}
	else
	{
		DVBC_CarrierOffset =  ChannelCentreFrequency  - ItbCoarseFrequencyOffset+ ItbCarFreqOffset_stat;
	}
	ATBMDVBCRead(0x3d0, &acq_status);
	if(acq_status&(0x20))
	{
		DVBC_CarrierOffset = DVBC_CarrierOffset;
	}
	else
	{
		DVBC_CarrierOffset = -DVBC_CarrierOffset;
	}
	ATBM_PRINTF("DVB-C signal Carrier Offset = %d KHz\n", DVBC_CarrierOffset);     
	return DVBC_CarrierOffset;	
}

/****************************************************************************
Function:	ATBM_DVBC_GetSymbolRate
Parameters:   none                
Return: 	symbol_rate(K symbol/s)

Description:
    Call this function to Check DVB-C Symbol Rate, valid for DVB-C mode.			 
******************************************************************************/
uint32 ATBMDVBCGetSymbolRate(void)
{
	uint8 mode;
	uint32 uTrlSampleRate;
	int uTrlSampleRateError;
	uint32 uDecLog2DecFactor;
	int dProgrammedSymbRate;
	int dOffset;
	uint32 symbol_rate; 
	ATBMDVBCRead(0x218, &uTrlSampleRateError);
	ATBMDVBCRead(0x210, &uTrlSampleRate);
	ATBMDVBCRead(0x124, &uDecLog2DecFactor);
	ATBMRead(0x00, 0x13,&mode);
	if((mode != 0x01)||(uTrlSampleRate == 0)) //Not in DVB-C mode or I2C Error
	{
		return 0;
	}
	dProgrammedSymbRate = (ui32DvbcOscFreq*uTrlSampleRate+ ((1<<(uDecLog2DecFactor+1))*(1<<15)))/((1<<(uDecLog2DecFactor+1))*(1<<16));
	if(dProgrammedSymbRate == 0)
	{
		dProgrammedSymbRate = dProgrammedSymbRate +1; 
	}
	dOffset = ((uTrlSampleRateError/137439)*ui32DvbcOscFreq)/(dProgrammedSymbRate *(1<<uDecLog2DecFactor));	
	symbol_rate = dProgrammedSymbRate;
	if(dOffset>=0)
	{
		symbol_rate = symbol_rate + (symbol_rate*dOffset + 500000) /1000000;
	}
	else
	{
		symbol_rate = symbol_rate + (symbol_rate*dOffset - 500000) /1000000;
	}
	ATBM_PRINTF("dProgrammedSymbRate = %d, symbol rate = %d K\n", dProgrammedSymbRate, symbol_rate);
	return symbol_rate;
}

int ATBM_PPM_Test(void)
{
	uint8  ui8APhiAdj[2];  
	uint8  ui8ASCPPM[2];
	uint8  ui8ALCPPM[2];  
	uint8  ui8pABuffTmp[4];
	uint8  ui8LockValue;
	uint8  ui8HeaderMode;
	uint32 ui32Bias = 0;
	uint32 ui32AcqRateFrac, ui32AcqRateFracCurr;
	int    i32OffsetPPM;

	ui8LockValue =1; /*lock register value*/
	ATBMWrite(0x08, 0x4d, ui8LockValue);
	ATBMRead(0x08, 0x39,&ui8APhiAdj[0]);//ui8APhiAdj[7:0]        
	ATBMRead(0x08, 0x3a,&ui8APhiAdj[1]);//ui8APhiAdj[13:8]
	ATBMRead(0x08, 0x5B,&ui8ASCPPM[0]);     
	ATBMRead(0x08, 0x5C,&ui8ASCPPM[1]);
	ATBMRead(0x08, 0x85,&ui8ALCPPM[0]);      
	ATBMRead(0x08, 0x86,&ui8ALCPPM[1]);
	ATBMRead(0x08, 0x00,&ui8pABuffTmp[0]);//ui32AcqRateFrac[7:0]
	ATBMRead(0x08, 0x01,&ui8pABuffTmp[1]);//ui32AcqRateFrac[12:8]      

	ui8LockValue = 0; /*cancel the lock*/
	ATBMWrite(0x08,0x4d, ui8LockValue);
	ui32AcqRateFrac = (ui8pABuffTmp[1]*256)+ui8pABuffTmp[0];
	ui32AcqRateFracCurr = ui32AcqRateFrac;              
	ATBMRead(0x08,0x2d,&ui8HeaderMode); 
	if(ui8APhiAdj[1]&(0x20)) // sign bit, negative
	{    
		//ui32Bias= (0xfff+1- (((ui8APhiAdj[1]&0x3f)*256)+ ui8APhiAdj[0]))*1000000/8388608;//power(2,23)          
		ui32Bias= (0x3fff+1- (((ui8APhiAdj[1]&0x3f)*256)+ ui8APhiAdj[0]))*1192/10000;//power(2,23)   
		ui32AcqRateFrac = ui32AcqRateFrac -ui32Bias;
	}else     
	{  
		ui32Bias = ((((ui8APhiAdj[1]&0x3f)*256)+ ui8APhiAdj[0]))*1192/10000;           
		ui32AcqRateFrac = ui32AcqRateFrac + ui32Bias;
	}
	if((ui8HeaderMode&0x3)==2) //single-carrier mode
	{
		if(ui8ASCPPM[1]&(0x01))//sign bit, negative
		{
			//      ui32Bias =(0x1ff+1-(((ui8ASCPPM[1]&0x01)*256)+ ui8ASCPPM[0]))*1000000/262144;//power(2,18)              
			ui32Bias =(0x1ff+1-(((ui8ASCPPM[1]&0x01)*256)+ ui8ASCPPM[0]))*38/10;//power(2,18)
			ui32AcqRateFrac = ui32AcqRateFrac - ui32Bias;
			ui8pABuffTmp[3]= (ui32AcqRateFrac>>8)&0xff;
			ui8pABuffTmp[2]= ui32AcqRateFrac&0xff;
		}
		else{
			ui32Bias =((( ui8ASCPPM[1]&0x01)*256)+ ui8ASCPPM[0])*38/10;                      
			ui32AcqRateFrac = ui32AcqRateFrac + ui32Bias;
			ui8pABuffTmp[3]= (ui32AcqRateFrac>>8)&0xff;
			ui8pABuffTmp[2]= ui32AcqRateFrac&0xff;
		}
	}
	//multi-carrier mode: ui8APhiAdj + ui8ALCPPM
	else if( ((ui8HeaderMode&0x3)==1)||(( ui8HeaderMode&0x3)==3)  )//multi-carrier mode
	{
		if(ui8ALCPPM[1]&(0x80))//sign bit
		{
			ui32Bias=(0xffff+1- (((ui8ALCPPM[1])*256)+ ui8ALCPPM[0]))*1192/10000;//power(2,23)
			ui32AcqRateFrac = ui32AcqRateFrac - ui32Bias;
			ui8pABuffTmp[3]= (ui32AcqRateFrac>>8)&0xff;
			ui8pABuffTmp[2]= ui32AcqRateFrac&0xff;
		}
		else{
			ui32Bias = ((( ui8ALCPPM[1])*256)+ ui8ALCPPM[0])*1192/10000;
			ui32AcqRateFrac = ui32AcqRateFrac + ui32Bias;
			ui8pABuffTmp[3]= (ui32AcqRateFrac>>8)&0xff;
			ui8pABuffTmp[2]= ui32AcqRateFrac&0xff;
		}

	}        
	i32OffsetPPM = ui32AcqRateFrac - ui32AcqRateFracCurr;
	ATBM_PRINTF("The current sampling frequency offset is about %d ppm\n", i32OffsetPPM);
	ATBM_PRINTF("The current values are: [0x0801] = 0x%x [0x0800] = 0x%x\n",ui8pABuffTmp[1],ui8pABuffTmp[0]);
	ATBM_PRINTF("The ideal values for minimal offset are: [0x0801] = 0x%x [0x0800]=0x%x\n", ui8pABuffTmp[3], ui8pABuffTmp[2]);
	return i32OffsetPPM;
}

void ATBM_GPO_I2CINT_Output(uint8 ui8Level)
{
	uint8 ui8GPIOReg;   	
	ATBMRead(0x10, 0xf7,&ui8GPIOReg); 
	ui8GPIOReg |= 0x20; //Set bit5, Enable I2CINT output
	if(ui8Level == 0)
		ui8GPIOReg &= 0xef; //Clear bit4, I2CINT pin output 0
	else
		ui8GPIOReg |= 0x10; //Set bit4, I2CINT pin output 1
	ATBMWrite(0x10, 0xf7, ui8GPIOReg);	
}

void ATBM_GPO_PWM1_Output(uint8 ui8Level)
{
	uint8 ui8GPIOReg;    	
	ATBMRead(0x10, 0xf7,&ui8GPIOReg); 
	ui8GPIOReg |= 0x08; //Set bit3,Enable PWM1 output
	if(ui8Level == 0)
		ui8GPIOReg &= 0xfb; //clear bit2,PWM1 pin output 0
	else 
		ui8GPIOReg |= 0x04; //set bit2,PWM1 pin output 1
	ATBMWrite(0x10, 0xf7, ui8GPIOReg);	
}

void ATBM_GPO_TestIO23_Output(uint8 ui8Level)
{
	uint8 ui8GPOReg; 
	uint8 ui8Data;
	ATBMRead(0x10, 0xf7,&ui8GPOReg); 
	ATBMRead(0x10, 0xfb,&ui8Data); 
	ui8GPOReg |= 0x80; //Set bit7,Enable TestIO23 output
	ui8Data|= 0x04; //set bit2
	if(ui8Level == 0)
	{
		ui8Data&=0xef; //clear bit4, this bit is output level
	}
	else
	{
		ui8Data|= 0x10; //set bit4 as High level
	}
	ATBMWrite(0x10, 0xfb, ui8Data);
	ATBMWrite(0x10, 0xf7, ui8GPOReg);  
}

/****************************************************************************
Function:    ATBM_TestIO23_Indicate_TS_Lock
Parameters: none
Return:      none
Description:  
    Enable lock indicator output on testIO23 when TS lock detected.
****************************************************************************/
void ATBM_TestIO23_Indicate_TS_Lock(void)
{
     uint8 ui8GPOReg; 
     uint8 ui8Data;
     ATBMRead(0x10, 0xf7,&ui8GPOReg); 
     ATBMRead(0x10, 0xfb,&ui8Data); 
     ui8GPOReg|= 0x80; //set bit7
     ui8Data&=0xfb; //clear bit2 
     ATBMWrite(0x10, 0xfb, ui8Data);
     ATBMWrite(0x10, 0xf7, ui8GPOReg);  
     ATBMWrite(0x00, 0x1a, 0x00); //TestIO23 using TS lock.
}

/****************************************************************************
Function:    ATBM_TestIO23_Indicate_FEC_No_Error
Parameters:  none
Return:      none
Description:  
    TestIO23 auto output high only when FEC no Error occurred.
    Strict on lock indicator, if FEC error occurred, lock indicator will be off.
    Lock indicator will be on only if not FEC error occurred
****************************************************************************/
void ATBM_TestIO23_Indicate_FEC_No_Error(void)
{
     uint8 ui8LockUseErrorInv;       
     ui8LockUseErrorInv = 0x01;
     ATBM_TestIO23_Indicate_TS_Lock();          //enable TestIO23 normal output
     ATBMWrite(0x00, 0x1a, ui8LockUseErrorInv); //TestIO23 using FEC no error
}

void ATBM_GPO_TestIO20_Output(uint8 ui8Level)
{
	uint8 ui8GPOReg; 
	uint8 ui8Data;
	ATBMRead(0x10, 0xf7,&ui8GPOReg); 
	ATBMRead(0x10, 0xfb,&ui8Data); 
	ui8GPOReg |= 0x40; //Set bit6,Enable TestIO20 output
	ui8Data|=0x02; //set bit1 enable
	if(ui8Level == 0)
	{
		ui8Data&=0xf7; //clear bit3
	}
	else
	{   
		ui8Data|=0x08; //set bit3 output 1    
	}
	ATBMWrite(0x10, 0xfb, ui8Data);
	ATBMWrite(0x10, 0xf7, ui8GPOReg);  
}

/****************************************************************************
Function:    ATBM_TestIO20_Indicate_FEC_Error
Parameters: none
Return:      none
Description: 
     Enable error indicator output on testIO20 when FEC error detected.
****************************************************************************/
void ATBM_TestIO20_Indicate_FEC_Error(void)
{
     uint8 ui8GPOReg; 
     uint8 ui8Data;
     ATBMRead(0x10, 0xf7,&ui8GPOReg); 
     ATBMRead(0x10, 0xfb,&ui8Data); 
     ui8GPOReg|= 0x40; //set bit6 
     ui8Data&=0xfd;  //clear bit1
     ATBMWrite(0x10, 0xfb, ui8Data);
     ATBMWrite(0x10, 0xf7, ui8GPOReg);  
     ATBMWrite(0x00, 0x1b, 0x00 );    //TestIO20 using FEC error
}

/****************************************************************************
Function:    ATBM_TestIO20_Indicate_TS_Unlock
Parameters:  none
Return:      none
Description:   
    Strict on Error indicator, if lock occurred (TS sync is true),
    error indicator will be off. 
    error indicator will be on when TS sync is not checked.
****************************************************************************/
void ATBM_TestIO20_Indicate_TS_Unlock(void)
{
     uint8 ui8ErrorUseLockInv;          
     ui8ErrorUseLockInv = 0x01;
     ATBM_TestIO20_Indicate_FEC_Error(); //enable TestIO20 normal output error
     ATBMWrite(0x00, 0x1b, ui8ErrorUseLockInv); //using TS unlock
}

/****************************************************************************
Function:    ATBMErrorDurationMillisecond
Parameters:  ms (0--275 milliseconds)
Return:      none
Description:   
  Set error indicator duration time.
  The default time is 8.6MS (0x800*4.2us), register value is 0x800.
  The maximum value is 275 milliseconds
****************************************************************************/
void ATBMErrorOnDurationMillisecond(int i32MS)
{
     int   i32SetValue;
     uint8 ui8RegHigh, ui8RegLow;
     i32SetValue = (int)(i32MS*10000/42);
     ui8RegLow = (uint8)(i32SetValue&0xff);
     ui8RegHigh = (uint8)(i32SetValue/256);
     ATBMWrite(0x15, 0x2b, ui8RegLow);
     ATBMWrite(0x15, 0x2c, ui8RegHigh);
}
/****************************************************************************
Function:    ATBMLockOffDurationMillisecond
Parameters:  ms (0--1100 milliseconds)
Return:      none
Description:   
  Set lock indicator off Duration time after unlocked. 
  The default time is 34.4ms:(0x800*16.8us), register value is 0x800.
  The maximum value is 1100 milliseconds
****************************************************************************/
void ATBMLockOffDurationMillisecond(int i32MS)
{
     int i32SetValue;
     uint8 ui8RegHigh, ui8RegLow;
     i32SetValue = (int)(i32MS*10000/168);
     ui8RegLow = (uint8)(i32SetValue&0xff);
     ui8RegHigh = (uint8)(i32SetValue/256);
     ATBMWrite(0x03, 0x19, ui8RegLow);
     ATBMWrite(0x03, 0x1a, ui8RegHigh);
}

/****************************************************************************
Function:    ATBM_GetGuradInterval
Parameters:  none
Return:      gi_interval

Description: 
   Function to get guard interval from TPS value: 420 595, 945
****************************************************************************/
DTMB_GUARD_INTERVAL  ATBM_GetGuradInterval(void)
{
	uint8 pn_value,PN;  
	DTMB_GUARD_INTERVAL gi_interval;
	ATBMRead(0x08, 0x2d,&pn_value);   
	PN = (uint8)(pn_value&0x03);
	// check Guard interval
	if(PN == 2)
	{
		gi_interval = GI_595;
	}
	else if(PN == 1)
	{

		gi_interval = GI_420;
	}
	else if(PN == 3)
	{

		gi_interval= GI_945;
	}
	else
	{
		gi_interval = GI_UNKNOWN;		
	}
	return gi_interval;
}

/****************************************************************************
Function:    ATBM_GetQamIndex
Parameters:  none
Return:      code_rate

Description: 
   Function to get QAM from TPS value
   4QAM-NR, 4QAM, 16QAM, 32QAM, 64QAM
****************************************************************************/
DTMB_QAM_INDEX  ATBM_GetQamIndex(void)
{
	uint8 tps_code;
	DTMB_QAM_INDEX qam_index;
	ATBMRead(0x0d, 0x04,&tps_code);
	if((tps_code>24)||(tps_code<3))
	{
		qam_index =DTMB_QAM_UNKNOWN;
		return qam_index;
	}
	if(tps_code<=4)
	{
		qam_index = DTMB_QAM_4QAM_NR;
	}
	else if(tps_code<=10)
	{
		qam_index = DTMB_QAM_4QAM;;
	}
	else if(tps_code<=16)
	{
		qam_index = DTMB_QAM_16QAM;
	}
	else if(tps_code<=18)
	{
		qam_index = DTMB_QAM_32QAM;
	}
	else if(tps_code<=24)
	{
		qam_index = DTMB_QAM_64QAM;
	}
	else
	{
		qam_index = DTMB_QAM_UNKNOWN;
	}
	return qam_index;
}

/****************************************************************************
Function:    ATBM_GetCodeRate
Parameters:  none
Return:      code_rate

Description: 
   Function to get FEC code rate from TPS value: 0.4, 0.6, 0.8
****************************************************************************/
DTMB_CODE_RATE  ATBM_GetCodeRate(void)
{
	uint8 tps_code;
	DTMB_CODE_RATE code_rate;
	ATBMRead(0x0d, 0x04,&tps_code);
	if((tps_code>24)||(tps_code<3))
	{
		code_rate = DTMB_CODE_RATE_UNKNOWN;
		return code_rate;
	}
	if((tps_code == 5)||(tps_code == 6))
	{
		code_rate = DTMB_CODE_RATE_0_DOT_4;
	}
	else if ((tps_code == 11)||(tps_code == 12))
	{
		code_rate = DTMB_CODE_RATE_0_DOT_4;
	}
	else if((tps_code == 19)||(tps_code == 20))
	{
		code_rate = DTMB_CODE_RATE_0_DOT_4;
	}
	else if((tps_code == 7)||(tps_code == 8))
	{
		code_rate = DTMB_CODE_RATE_0_DOT_6;
	}
	else if((tps_code == 13)||(tps_code == 14))
	{
		code_rate = DTMB_CODE_RATE_0_DOT_6;
	}
	else if((tps_code == 21)||(tps_code == 22))
	{
		code_rate = DTMB_CODE_RATE_0_DOT_6;
	}
	else
	{
		code_rate = DTMB_CODE_RATE_0_DOT_8;
	}
	return code_rate;
}

/****************************************************************************
Function:    ATBM_GetInterleavingMode
Parameters:  none
Return:      time_interleave

Description: 
Function to get Time interleaving mode from TPS value: 240 or 720
****************************************************************************/
DTMB_TIME_INTERLEAVE  ATBM_GetInterleavingMode(void)
{
	uint8 tps_code;
	DTMB_TIME_INTERLEAVE time_interleave = DTMB_TIME_INTERLEAVER_UNKNOWN;
	ATBMRead(0x0d, 0x04,&tps_code);

	if((tps_code>24)||(tps_code<3))
	{
		time_interleave = DTMB_TIME_INTERLEAVER_UNKNOWN;
		return time_interleave;
	}
	if((tps_code%2) == 0)
	{
		time_interleave = DTMB_TIME_INTERLEAVER_720;
	}
	if((tps_code%2) != 0)
	{
		time_interleave = DTMB_TIME_INTERLEAVER_240;
	}
	return  time_interleave;
}

/****************************************************************************
Function:    ATBM_GetCarrierMode
Parameters:  none
Return:      carrier_mode

Description: 
Function to get carrier mode: single carrier or Multi-carrier
****************************************************************************/
DTMB_CARRIER_MODE   ATBM_GetCarrierMode(void)
{
	uint8 is_sc;
	DTMB_CARRIER_MODE carrier_mode;
	ATBMRead(0x0d, 0x0f,&is_sc); 
	//check carrier mode
	if(is_sc == 1)
	{
		carrier_mode = DTMB_SINGLE_CARRIER;
	}
	else if(is_sc == 0)
	{

		carrier_mode = DTMB_MULTI_CARRIER;
	}
	else
	{
		carrier_mode = DTMB_CARRIER_UNKNOWN;		
	}	
	return carrier_mode;
}

/****************************************************************************
Function:    ATBMGetSignalParameters
Parameters:  singal_params
Return:       none

Description: 
API to get all DTMB signal parameters:
Carrier mode, Constellation QAM index,  FEC code rate, Guard interval
and time interleaving mode.
****************************************************************************/
void ATBMGetSignalParameters(DTMB_SIGNAL_PARAMS *singal_params)
{
	singal_params->dtmb_carrier_mode = ATBM_GetCarrierMode();

	singal_params->dtmb_qam_index =  ATBM_GetQamIndex();

	singal_params->dtmb_code_rate = ATBM_GetCodeRate();

	singal_params->dtmb_guard_interval =   ATBM_GetGuradInterval();

	singal_params->dtmb_time_interleave = ATBM_GetInterleavingMode();
}

/****************************************************************************
Function:    ATBMGetDTMBBitRate
Parameters:  none
Return:       payload

Description: 
API to get DTMB MPEG2 TS payload in unit of Kbps
****************************************************************************/
int  ATBMGetDTMBBitRate(void)
{
	DTMB_CODE_RATE code_rate;
	DTMB_QAM_INDEX qam_index;
	DTMB_GUARD_INTERVAL gi_interval;
	
	int payload = 0;
	
	qam_index =  ATBM_GetQamIndex();
	code_rate = ATBM_GetCodeRate();
	gi_interval =   ATBM_GetGuradInterval();	

	if(qam_index == DTMB_QAM_4QAM) 
	{	
		if (code_rate == DTMB_CODE_RATE_0_DOT_4) 
		{
			if(gi_interval == GI_420)
			{
				payload = 5414;
			}
			else if(gi_interval == GI_595)
			{
				payload = 5198;
			}
			else if (gi_interval == GI_945) 
			{
				payload = 4813;
			}
			else 
			{
				payload = 0;
			}
		}
		else if (code_rate == DTMB_CODE_RATE_0_DOT_6) 
		{
			if(gi_interval == GI_420)
			{
				payload = 8122;
			}
			else if(gi_interval == GI_595)
			{
				payload = 7797;
			}
			else if (gi_interval == GI_945) 
			{
				payload = 7219;
			}
			else 
			{
				payload = 0;
			}
		}
		else if (code_rate == DTMB_CODE_RATE_0_DOT_8) 
		{
			if(gi_interval == GI_420)
			{
				payload = 10829;
			}
			else if(gi_interval == GI_595)
			{
				payload = 10396;
			}
			else if (gi_interval == GI_945) 
			{
				payload = 9626;
			}
			else 
			{
				payload = 0;
			}
		}
		else
		{
			payload = 0;
		}
	}
	else if (qam_index == DTMB_QAM_16QAM) 
	{
		if (code_rate == DTMB_CODE_RATE_0_DOT_4) 
		{
			if(gi_interval == GI_420)
			{
				payload = 10829;
			}
			else if(gi_interval == GI_595)
			{
				payload = 10396;
			}
			else if (gi_interval == GI_945) 
			{
				payload = 9626;
			}
			else 
			{
				payload = 0;
			}
		}
		else if (code_rate == DTMB_CODE_RATE_0_DOT_6) 
		{
			if(gi_interval == GI_420)
			{
				payload = 16243;
			}
			else if(gi_interval == GI_595)
			{
				payload = 15593;
			}
			else if (gi_interval == GI_945) 
			{
				payload = 14438;
			}
			else 
			{
				payload = 0;
			}
		}
		else if (code_rate == DTMB_CODE_RATE_0_DOT_8) 
		{
			if(gi_interval == GI_420)
			{
				payload = 21658;
			}
			else if(gi_interval == GI_595)
			{
				payload = 20791;
			}
			else if (gi_interval == GI_945) 
			{
				payload = 19251;
			}
			else 
			{
				payload = 0;
			}
		}
		else
		{
			payload = 0;
		}
	}
	else if (qam_index == DTMB_QAM_64QAM)
	{ 
		if (code_rate == DTMB_CODE_RATE_0_DOT_4) 
		{
			if(gi_interval == GI_420)
			{
				payload = 16243;
			}
			else if(gi_interval == GI_595)
			{
				payload = 15593;
			}
			else if (gi_interval == GI_945) 
			{
				payload = 14438;
			}
			else 
			{
				payload = 0;
			}
		}
		else if (code_rate == DTMB_CODE_RATE_0_DOT_6) 
		{
			if(gi_interval == GI_420)
			{
				payload = 24365;
			}
			else if(gi_interval == GI_595)
			{
				payload = 23390;
			}
			else if (gi_interval == GI_945) 
			{
				payload = 21658;
			}
			else 
			{
				payload = 0;
			}
		}
		else if (code_rate == DTMB_CODE_RATE_0_DOT_8) 
		{
			if(gi_interval == GI_420)
			{
				payload = 32486;
			}
			else if(gi_interval == GI_595)
			{
				payload = 31187;
			}
			else if (gi_interval == GI_945) 
			{
				payload = 28877;
			}
			else 
			{
				payload = 0;
			}
		}
		else
		{
			payload = 0;
		}
	}
	else if((qam_index == DTMB_QAM_32QAM))
	{
		if (code_rate == DTMB_CODE_RATE_0_DOT_8) 
		{
			if(gi_interval == GI_420)
			{
				payload = 27072;
			}
			else if(gi_interval == GI_595)
			{
				payload = 25989;
			}
			else if (gi_interval == GI_945) 
			{
				payload = 24064;
			}
			else 
			{
				payload = 0;
			}
		}else
		{
			payload = 0;
		}
	}
	else if((qam_index == DTMB_QAM_4QAM_NR))
	{
		if (code_rate == DTMB_CODE_RATE_0_DOT_8) 
		{
			if(gi_interval == GI_420)
			{
				payload = 5414;
			}
			else if(gi_interval == GI_595)
			{
				payload = 5198;
			}
			else if (gi_interval == GI_945) 
			{
				payload = 4813;
			}
			else 
			{
				payload = 0;
			}
		}else
		{
			payload = 0;
		}
	}
	else
	{
		payload = 0;
	}	
	return payload;
}

/*Internal function used by ATBMSetClock64M*/
void ATBMSetTS_SPIClock64M(void)
{
	unsigned char tmp_data;
	ATBMRead(0x06, 0x0e, &tmp_data);
	tmp_data&=0x8f; //clear bit[14:12]
	tmp_data|=0x60; //set bit[14:12]:110
	ATBMWrite(0x06, 0x0e, tmp_data);
}

/*Internal function used by ATBMSetClock64M*/
void ATBMSetSYS_Clock64M(void)
{
	unsigned char adc_rfv0, adc_rfv1;
	ATBMRead(0x06, 0x0d, &adc_rfv0);
	adc_rfv0&=0x7f; //clear bit7
	ATBMRead(0x06, 0x0e, &adc_rfv1);
	adc_rfv1&=0xf7; //clear bit[11] 

	adc_rfv0|=0x80; //set bit[7]:1
	ATBMWrite(0x06, 0x0d, adc_rfv0);
	adc_rfv1|=0x08; //set bit[11]:1
	ATBMWrite(0x06, 0x0e, adc_rfv1);	
}

/*Internal function used by ATBMSetClock72M*/
void ATBMSetTS_SPIClock72M(void)
{
	unsigned char tmp_data;
	ATBMRead(0x06, 0x0e, &tmp_data);
	tmp_data&=0x8f; //clear bit[14:12], default is 72M for 24MHz Crystal
	tmp_data|=0x00;//set bit[14:12]:000
	ATBMWrite(0x06, 0x0e, tmp_data);
}

/*Internal function used by ATBMSetClock72M*/
void ATBMSetSYS_Clock72M(void)
{
	unsigned char adc_rfv0,adc_rfv1;
	ATBMRead(0x06, 0x0e, &adc_rfv1);
	adc_rfv1&=0xf7; //clear bit[11] 
	ATBMRead(0x06, 0x0d, &adc_rfv0);
	adc_rfv0&=0x7f; //clear bit7

	adc_rfv0|=0x00;//set bit[7]:0
	ATBMWrite(0x06, 0x0d, adc_rfv0); 
	adc_rfv1|=0x00;//set bit[11]:0
	ATBMWrite(0x06, 0x0e, adc_rfv1); 			
}

/****************************************************************************
Function:    ATBMSetClock64M
Parameters:  none
Return:      none  

Description:
      Call this function to change TS SPI clock,and sys clock to 64MHz, 
      the default used clock is 72MHz with 24MHz crystal  or oscillator.

	  Valid only for 24MHz crystal or oscillator.
******************************************************************************/
void ATBMSetClock64M(void)
{
	unsigned char ui8Rst;
	ATBMWrite(0x01, 0x0c, 0); 
	Delayms(2);
	ATBMSetTS_SPIClock64M();
	ATBMSetSYS_Clock64M();    
	if((ui32DvbcSymbolRate > 7500)||(ui32DvbcSymbolRate < 1000))
	{
		ui32DvbcSymbolRate = 6875;
	}
	ui8Rst =(uint8)(6875*16/ui32DvbcSymbolRate); 
	ATBMWrite(0x15, 0x37, ui8Rst);
	Delayms(2);
	ATBMWrite(0x01, 0x0c, 1); 
}

/****************************************************************************
Function:    ATBMSetClock72M
Parameters:  none
Return:      none  

Description:
      Call this function to change TS SPI clock,and sys clock to 72MHz, 
      the default used clock is 72MHz with 24MHz crystal or oscillator.

	  Valid only for 24MHz crystal or oscillator.
******************************************************************************/
void ATBMSetClock72M(void)
{
	unsigned char ui8Rst;
	ATBMWrite(0x01, 0x0c, 0); 
	Delayms(2);
	ATBMSetTS_SPIClock72M();
	ATBMSetSYS_Clock72M();
	if((ui32DvbcSymbolRate > 7500)||(ui32DvbcSymbolRate < 1000))
	{
		ui32DvbcSymbolRate = 6875;
	}
	ui8Rst =(uint8)(6875*18/ui32DvbcSymbolRate); 
	ATBMWrite(0x15, 0x37, ui8Rst);
	Delayms(2);
	ATBMWrite(0x01, 0x0c, 1); 
}

/****************************************************************************
Function:    ATBMRead
Parameters:  ui8BaseAddr, ui8RegisterAddr, *ui8pValue
Return:      I2C read status:ATBM_I2CREADWRITE_OK is OK
Description: 
     ATBM886x demodulator register Reading API, it calls I2CRead. 	
****************************************************************************/
ATBM_I2CREADWRITE_STATUS  ATBMRead(uint8 ui8BaseAddr, uint8 ui8RegisterAddr,uint8 *ui8pValue)
{ 
	uint8 ui8AddrData[2];

	ATBM_I2CREADWRITE_STATUS enumRetval = ATBM_I2CREADWRITE_OK; 
	ui8AddrData[0] = ui8BaseAddr;
	ui8AddrData[1] = ui8RegisterAddr;

	//Option 1:One step I2C format, please port this function according to I2C format of this function..
	enumRetval = I2CReadOneStep(ui8ATBMSlaveAddr, 2, ui8AddrData, 1, ui8pValue);

	//Option 2: two step format, returned status from low level I2C interface 
                    //shoud be processsed here if needed.       
	//enumRetval = I2CWrite(ui8ATBMSlaveAddr, i2c_data, 2);	
	//enumRetval = I2CRead(ui8ATBMSlaveAddr, ui8pValue, 1);

	//Release mutex lock
   return enumRetval;
}

/****************************************************************************
Function:    ATBMWrite
Parameters:  ui8BaseAddr, ui8RegisterAddr, ui8Data
Return:      I2C write status:ATBM_I2CREADWRITE_OK is OK  

Description: 
     ATBM886x demodulator register Writing API, It calls I2C low level interface. 
    ****************************************************************************/
ATBM_I2CREADWRITE_STATUS  ATBMWrite(uint8 ui8BaseAddr, uint8 ui8RegisterAddr, uint8 ui8Data)
{ 

	ATBM_I2CREADWRITE_STATUS enumRetval =  ATBM_I2CREADWRITE_OK ; 
	uint8 i2c_data[2];
    uint8 reg_data[1];
	

	i2c_data[0] = ui8BaseAddr;
	i2c_data[1] = ui8RegisterAddr;
	reg_data[0] = ui8Data;
	
        //Option l: One step I2C format
        enumRetval = I2CWriteWithRestart(ui8ATBMSlaveAddr, 2, i2c_data, 1, reg_data);

	//Option 2: two step format, returned status from low level I2C interface 
                    //should be processed here if needed.  
		//Add mutex lock here if there is multi-thread operation of demodulator's I2C
	//the following I2C operations will not be interrupted. 
	//enumRetval = I2CWrite(ui8ATBMSlaveAddr, i2c_data, 2);	
	//enumRetval = I2CWrite(ui8ATBMSlaveAddr, reg_data, 1);

	//Release mutex lock
#if ATMB_DBG_OUTPUT
	ATBM_PRINTF("%x %x %x\n", ui8BaseAddr, ui8RegisterAddr, ui8Data);	
#endif	
	return enumRetval;
}

/****************************************************************************
Function:    ATBMTransRead
Parameters:  ui8BaseAddr, ui8RegisterAddr,ui8pValue
Return:      I2C read status:ATBM_I2CREADWRITE_OK is OK

Description:   
     ATBM886x demodulator register Reading API, This function is used for DVB-C transfer. 	   
****************************************************************************/
ATBM_I2CREADWRITE_STATUS  ATBMTransRead(uint8 ui8BaseAddr, uint8 ui8RegisterAddr,uint8 *ui8pValue)
{ 
	ATBM_I2CREADWRITE_STATUS enumRetval =  ATBM_I2CREADWRITE_OK ; 
	uint8 ui8AddrData[2];
	ui8AddrData[0] = ui8BaseAddr;
	ui8AddrData[1] = ui8RegisterAddr;
	
		//Option 1:One step I2C format, please port this function according to I2C format of this function..
	enumRetval = I2CReadOneStep(ui8ATBMSlaveAddr, 2, ui8AddrData, 1, ui8pValue);

	//Option 2: two step format, returned status from low level I2C interface 
                    //should be processed here if needed.       
	//enumRetval = I2CWrite(ui8ATBMSlaveAddr, ui8AddrData, 2);	
	//enumRetval = I2CRead(ui8ATBMSlaveAddr, ui8pValue, 1);

	//Release mutex lock
   return enumRetval;   
	
}


/****************************************************************************
Function:        ATBMTransWrite
Parameters:  ui8BaseAddr, ui8RegisterAddr, ui8Data
Return:      I2C write status:ATBM_I2CREADWRITE_OK is OK 

Description: 
     ATBM886x demodulator register Writing API, It calls low level I2C interface    
****************************************************************************/
ATBM_I2CREADWRITE_STATUS  ATBMTransWrite(uint8 ui8BaseAddr, uint8 ui8RegisterAddr, uint8 ui8Data)
{
	ATBM_I2CREADWRITE_STATUS enumRetval =  ATBM_I2CREADWRITE_OK ; 
	uint8 i2c_data[2];
	uint8 reg_data[1];
	

	i2c_data[0] = ui8BaseAddr;
	i2c_data[1] = ui8RegisterAddr;
	reg_data[0] = ui8Data;
	
        //Option l: One step I2C format
        enumRetval = I2CWriteWithRestart(ui8ATBMSlaveAddr, 2, i2c_data, 1, reg_data);

	//Option 2: two step format, returned status from low level I2C interface 
                    //should be processed here if needed.  
		//Add mutex lock here if there is multi-thread operation of demodulator's I2C
	//the following I2C operations will not be interrupted. 
	//enumRetval = I2CWrite(ui8ATBMSlaveAddr, i2c_data, 2);	
	//enumRetval = I2CWrite(ui8ATBMSlaveAddr, reg_data, 1);

	//Release mutex lock
#if ATMB_DBG_OUTPUT
	ATBM_PRINTF("%x %x %x\n", ui8BaseAddr, ui8RegisterAddr, ui8Data);	
#endif	
	return enumRetval;
}


/**************************************************************************************************

====================ALI linux demod+tuner devices driver added by mabel=====================

***************************************************************************************************/

static UINT8 g_tuner_model_alive  = 0;   //support tuner amount
static UINT8 g_tuner_index_online = 0;   //valid start form 1
static UINT8 g_tuner_name_online[16];
static UINT32 	Current_freq 	= 0;
static UINT32 	Current_Sym 	= 0;
static UINT8	Current_Qam 	= 0;
//static UINT8 	nim_sw_test_tread_status = 0;

struct tuner_handle TUNER_ALIVE_HEAD;


static void check_ali_tuner_alive_list(void)
{
	struct tuner_handle *tmp;
    	struct list_head *pos;
	UINT8 cnt = 0;
	ATBM_PRINTF("[%s]========= print the tuner list ============\n",__FUNCTION__);
	list_for_each(pos, &(TUNER_ALIVE_HEAD.list))
	{
		cnt++;
		tmp = list_entry(pos, struct tuner_handle, list);
		tmp->seq_num = cnt;
		ATBM_PRINTF("[INDEX]  name: %s, ids: %d\n", tmp->tun_name, tmp->seq_num);
	}
	g_tuner_model_alive = cnt;
	ATBM_PRINTF("[NOTE] tuner list amount = %d\n\n", g_tuner_model_alive);
}

INT32 ali_nim_tuner_register(struct tuner_handle *model)
{
	struct ali_nim_m3200_private *priv = ali_m3200_nim_priv;
	struct tuner_handle *add_model;
    	if (NULL == priv)
	{
		NIM_LOG_ERROR("[ERR] No Ali M3200 NIM device available!\n");
		return -1;
	}
	add_model = model;
	g_tuner_model_alive++;
	add_model->seq_num = g_tuner_model_alive;
	list_add_tail(&(add_model->list), &(TUNER_ALIVE_HEAD.list));
	ATBM_PRINTF("[%s] tuner module has been added!\n",__FUNCTION__);
	check_ali_tuner_alive_list();
	return 0;
}
EXPORT_SYMBOL(ali_nim_tuner_register);
INT32 ali_nim_tuner_unregister(const UINT8 *model_name)
{
	struct ali_nim_m3200_private *priv = ali_m3200_nim_priv;
	struct tuner_handle *del_model;
	struct list_head *pos, *n;
	if (NULL == priv)
	{   
		NIM_LOG_ERROR("[ERR] No Ali M3200 NIM device available!\n");
		return -1;
	}

	if (list_empty(&(TUNER_ALIVE_HEAD.list)))
	{
		ATBM_PRINTF("[WARN] tuner alive list is empty already!\n");
		g_tuner_model_alive          = 0;
		g_tuner_index_online         = 0;
		priv->nim_tuner_init         = NULL;
		priv->nim_tuner_control      = NULL;
		priv->nim_tuner_status       = NULL;
		priv->nim_Tuner_Get_rf_level = NULL;
		priv->nim_Tuner_Release      = NULL;
		memset(g_tuner_name_online, 0, sizeof(g_tuner_name_online));
		return -1;
	}
	
	list_for_each_safe(pos, n, &(TUNER_ALIVE_HEAD.list))
	{
		del_model = list_entry(pos, struct tuner_handle, list);
		if (0 == strcmp(del_model->tun_name, model_name))
		{
			list_del_init(pos);
			ATBM_PRINTF("[NOTE] tuner module has been removed!\n");
			//kfree(del_model);  will release in relevant tuner module
			
			g_tuner_model_alive--;
			if (0 == strcmp(model_name, g_tuner_name_online))
			{
				/*make current config invalid*/
				priv->nim_tuner_control(priv->tuner_id, 0, 0, FAST_TIMECST_AGC, _1st_i2c_cmd);
				ATBM_PRINTF("[NOTE] tuner %s online has been removed!\n", g_tuner_name_online);
				
				g_tuner_index_online         = 0;
				priv->nim_tuner_init         = NULL;
				priv->nim_tuner_control      = NULL;
				priv->nim_tuner_status       = NULL;
				priv->nim_Tuner_Get_rf_level = NULL;
				priv->nim_Tuner_Release      = NULL;
				memset(g_tuner_name_online, 0, sizeof(g_tuner_name_online));
			}
			check_ali_tuner_alive_list();
			return 0;
		}
	}
	ATBM_PRINTF("[WARN] no matched tuner is removed!\n");
	//check_ali_tuner_alive_list();
	return -1;
}
EXPORT_SYMBOL(ali_nim_tuner_unregister);

static INT32 tunflag=0;

static INT32 nim_s3202_tunerself_adaption(struct ali_nim_m3200_private *priv, struct tuner_model *addr_list)
{	

//	UINT8 find_addr = 0;
	UINT8 user_cnt, i;
	struct tuner_model *user_model;
	struct tuner_handle *ker_model;
	struct list_head *pos;

	user_cnt = (addr_list[0].dev_ids) >> 4;
	ATBM_PRINTF("[%s][NOTE] %d times of trying to communicate from user space \n", __FUNCTION__,user_cnt);

	for (i = 1; i < user_cnt; i++)
	{
		user_model = &(addr_list[i]);		
	    	ATBM_PRINTF("[NOTE] i2c id is %d, current test addr is %02x\n", addr_list[0].dev_ids&0x0F, user_model->dev_ids);
		list_for_each(pos, &(TUNER_ALIVE_HEAD.list))
		{
			ker_model = list_entry(pos, struct tuner_handle, list);
			ATBM_PRINTF("[NOTE] test to CMP tuner name is %s vs %s\n", ker_model->tun_name, user_model->dev_name);
			if (0 == strcmp(ker_model->tun_name, user_model->dev_name))
			{
			    	ATBM_PRINTF("[NOTE] find tuner name is %s\n", ker_model->tun_name);
			    	g_tuner_index_online         = ker_model->seq_num;
				priv->nim_tuner_init         = ker_model->tuner_init;
				priv->nim_tuner_control      = ker_model->tuner_control;
				priv->nim_tuner_status       = ker_model->tuner_status;
				priv->nim_Tuner_Get_rf_level = ker_model->tuner_rflevel;
				priv->nim_Tuner_Release      = ker_model->tuner_release;
				strcpy(g_tuner_name_online, ker_model->tun_name);
				if(i==6)
					tunflag = 1;
				else
					tunflag =0;
			    	return i;
			}
		}

	}
	NIM_LOG_ERROR("[ERROR] \n");
	return -1;

}
static INT32 ali_m3200_nim_hw_initialize(struct ali_nim_m3200_private *priv, struct ali_nim_m3200_cfg *nim_cfg)
{
	ATBM_PRINTF("%s: QAM base addr: %08x\n", __FUNCTION__, 0xb8028000);        //S3202_QAM_SOC_BASE_ADDR);

	/* tuner configuration function */
	memcpy((void*)&(priv->tuner_config_data), (void*)&(nim_cfg->tuner_config_data), sizeof(struct QAM_TUNER_CONFIG_DATA));
	memcpy((void*)&(priv->tuner_config_ext), (void*)&(nim_cfg->tuner_config_ext), sizeof(struct QAM_TUNER_CONFIG_EXT));
    	priv->qam_mode = nim_cfg->qam_mode;
	priv->dem_i2c_id = 1;
#ifdef I2C_BYPASS
	priv->dem_i2c_id = nim_cfg->tuner_config_ext.i2c_type_id;
#endif
	ATBM_PRINTF("first priv->dem_i2c_id : %d \n",priv->dem_i2c_id);

      //nim_s3202_mode_regset(priv);
	/* Tuner Initial */
	if (NULL != priv->nim_tuner_init)
	{
 	    if (priv->nim_tuner_init(&(priv->tuner_id), &(priv->tuner_config_ext)) != SUCCESS)
	    {
	 	    NIM_LOG_ERROR("[ERR] Error: Init Tuner Failure!\n");
		    return ERR_NO_DEV;
	    }
	}
	ATBM_PRINTF("[NIMTRACE]: tuner i2c_type_id: %08x\n", nim_cfg->tuner_config_ext.i2c_type_id);	
	return RET_SUCCESS;
}

static UINT32 curflag = 0; 
static INT32 nim_s3202_get_lock(struct ali_nim_m3200_private *priv, UINT8 *lock)
{
	UINT8 ret=0;
	ret = ATBMLockedFlag();

	ATBM_PRINTF("LOCK data is 0x%x %d\n", ret,tunflag);
	*lock = ret;
	if(ret == 0)
	{
		ATBM_PRINTF("TUNER LOCK FAIL TIME TICK :%d \n",osal_get_tick());
		curflag = osal_get_tick();
	}
	else
		ATBM_PRINTF("TUNER LOCK SUC SPENT TIME TICK :%d \n",osal_get_tick()-curflag);
		
	return 0;
}

static INT32 nim_s3202_channel_change(struct ali_nim_m3200_private *priv, struct Ali_NIM_Channel_Change* pstChl_Change)
{
	UINT8	lock;
	UINT32 	freq,sym;
	UINT8 	qam;

	freq = pstChl_Change->freq;
	sym = 6875;
	qam = QAM64;
	ATBM_PRINTF("Enter Function nim_ATBM_channel_change \n");
	ATBM_PRINTF("freq is %d\n",freq);
	ATBM_PRINTF("sym is %d\n", sym);
	ATBM_PRINTF("qam is %d\n", qam);
	if(( qam != QAM16 )&& ( qam != QAM32 )&&( qam != QAM64 )&&( qam != QAM128 ) &&( qam != QAM256 ) ){
		NIM_LOG_ERROR("	QAM is not in [16,32,64,128,256]!\n");
		return ERR_FAILUE;
	} else if( (sym < 1000 ) || ( sym  > 7000) ){
		NIM_LOG_ERROR("	SYM is not in 1MBd ~ 7MBd!\n");
        	freq = 47400;
		qam = QAM64;
        	sym = 6875;
	} else if((freq < 4700 ) || ( freq > 86200)) {
		NIM_LOG_ERROR("	FREQ is not in 47MHz ~ 862MHz!\n");
		freq = 47400;
		qam = QAM64;
        	sym = 6875;
	}
	Current_freq = freq;
	Current_Sym = sym;
	Current_Qam = qam;
	//The following procedures are to set the tuner's center frequency to switch channel
	ATBMHoldDSP();
	ATBMI2CByPassOn();
	Delayms(10);
	priv = ali_m3200_nim_priv;

	//NIM_MUTEX_ENTER(priv);
	//Set_Tuner_TDAC2_DTMB(freq*10);
	priv->nim_tuner_control(0,freq*10,0,0,0);
	//NIM_MUTEX_LEAVE(priv);
	Delayms(50);
	ATBMI2CByPassOff();
	Delayms(10);
	ATBMStartDSP();	
    	Delayms(50);
    	nim_s3202_get_lock(priv,&lock);
	if(lock)
		return SUCCESS;
	else
		return !SUCCESS;
}
static INT32 nim_s3202_get_BER(struct ali_nim_m3200_private *priv, UINT32 *err_count)
{
	*err_count = ATBMFrameErrorRatio();
	return 0;
}
static INT32 nim_s3202_get_PER(struct ali_nim_m3200_private *priv, UINT32 *RsUbc)
{
	return 0;
}
static INT32 nim_s3202_get_PER_sum(struct ali_nim_m3200_private *priv, UINT32 *RsUbc)
{
	return 0;
}
static INT32 nim_s3202_get_RF_Level(struct ali_nim_m3200_private *priv, UINT16 *RfLevel)
{
	 UINT16 level=  ATBMSignalStrength();  
	*RfLevel = level;
	return 0;
}
static INT32 nim_s3202_get_CN_value(struct ali_nim_m3200_private *priv, UINT16 *CNValue)
{
	UINT16 tmp;

	tmp = ATBMSignalQuality();
	*CNValue =tmp*10;
	ATBM_PRINTF("get snr: %d \n",*CNValue);
	return 0;
}
static INT32 nim_s3202_set_perf_level(struct ali_nim_m3200_private *priv, UINT32 level)
{
	return 0;
}
static INT32 nim_s3202_get_AGC(struct ali_nim_m3200_private *priv, UINT8 *agc)
{
	 UINT16 level=  ATBMSignalStrength();  

	 ATBM_PRINTF("----- level = %d\n",level);
	*agc = level;
	return 0;
}
static INT32 nim_s3202_get_SNR(struct ali_nim_m3200_private *priv, UINT8 *snr)
{
	*snr =  (UINT8)ATBMSignalNoiseRatio();
	return 0;
}
static INT32 nim_s3202_get_symbol_rate(struct ali_nim_m3200_private *priv, UINT32 *sym_rate)
{
	*sym_rate = Current_Sym;
	return 0;
}
static INT32 nim_s3202_get_freq(struct ali_nim_m3200_private *priv, UINT32 *freq)
{
	*freq = Current_freq;
	return 0;
}
static INT32 nim_s3202_get_qam_order(struct ali_nim_m3200_private *priv, UINT8 *qam_order)
{
	*qam_order = Current_Qam;
	return 0;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
int ali_m3200_nim_ioctl(struct file *file, unsigned int cmd, unsigned long parg)
#else
int ali_m3200_nim_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long parg)
#endif
{
	struct ali_nim_device *dev = file->private_data;
	struct ali_nim_m3200_private *priv = dev->priv;
	int ret = 0;	
	switch(cmd)
	{
		case ALI_NIM_READ_TUNTYPE:
		{
			ATBM_PRINTF("FUCK CALL READ TUNTYPE \n");
			if(tunflag == 1)
				ret =1;
			break;			
		}
		case ALI_NIM_TUNERSELT_ADAPTION: //0x80186f5e: //
		{
			ret = nim_s3202_tunerself_adaption(priv, (struct tuner_model *)parg);
			//struct tuner_model nim_param;
			//copy_from_user(&nim_param, (struct tuner_model *)parg, sizeof(struct tuner_model));
			//ret = nim_s3202_tunerself_adaption(priv, &nim_param);
			break;
		}
		case ALI_NIM_HARDWARE_INIT_C:
		{
			struct ali_nim_m3200_cfg nim_param;
			if(copy_from_user(&nim_param, (struct ali_nim_m3200_cfg *)parg, sizeof(struct ali_nim_m3200_cfg))>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}
			ret = ali_m3200_nim_hw_initialize(priv, &nim_param);
			break;
		}
		case ALI_NIM_CHANNEL_CHANGE:
		{
			struct Ali_NIM_Channel_Change nim_param;
			if(copy_from_user(&nim_param, (struct Ali_NIM_Channel_Change *)parg, sizeof(struct Ali_NIM_Channel_Change))>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}
			if (0 == nim_param.fec)
				return nim_s3202_channel_change(priv, &nim_param);
			else
				return nim_s3202_channel_change(priv, &nim_param);
			break;
		}
		case ALI_NIM_GET_LOCK_STATUS:
		{
			UINT8 lock;
			nim_s3202_get_lock(priv, &lock);
			ret = lock;
			break;
		}
		case ALI_NIM_READ_QPSK_BER:
		{
			UINT32 ber;
			nim_s3202_get_BER(priv, &ber);
			ret = ber;
			break;
		}
		case ALI_NIM_READ_RSUB:
		{
			UINT32 per=0;

			nim_s3202_get_PER(priv, &per);
			ret = per;
			break;
		}
		case ALI_NIM_DRIVER_READ_SUMPER:
		{
			UINT32 per_sum;
			if(copy_from_user(&per_sum, (UINT32 *)parg, sizeof(UINT32))>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}
				
			nim_s3202_get_PER_sum(priv, &per_sum);
			ret = per_sum;
			break;
		}
		case ALI_NIM_GET_RF_LEVEL:
		{
			UINT16 rf_level;
			nim_s3202_get_RF_Level(priv, &rf_level);
			ret = rf_level;
			break;
		}
		case ALI_NIM_GET_CN_VALUE:
		{
			UINT16 cn_value;
			nim_s3202_get_CN_value(priv, &cn_value);
			ret = cn_value;
			break;
		}
		case ALI_NIM_SET_PERF_LEVEL:
		{
			ret = nim_s3202_set_perf_level(priv, parg);
			break;
		}
		case ALI_NIM_READ_AGC:
		{
			UINT8 agc;
			nim_s3202_get_AGC(priv, &agc);
			ret = agc;
			break;
		}
		case ALI_NIM_READ_SNR:
		{
			UINT8 snr;
			nim_s3202_get_SNR(priv, &snr);
			ret = snr;
			break;
		}
		case ALI_NIM_READ_SYMBOL_RATE:
		{
			UINT32 sym;
			nim_s3202_get_symbol_rate(priv, &sym);
			ret = sym;
			break;
		}
		case ALI_NIM_READ_FREQ:
		{
			UINT32 freq;
			nim_s3202_get_freq(priv, &freq);
			ret = freq;
			break;
		}
		case ALI_NIM_READ_CODE_RATE:
		{
			UINT8 fec;
			nim_s3202_get_qam_order(priv, &fec);
			ret = fec;
			break;
		}
		case ALI_NIM_REG_RW:
		{
			break;
		}
		case ALI_NIM_REG_RW_EXT:
		{
			break;
		}
		case ALI_SYS_REG_RW:
		{
			break;
		}
   	    	case ALI_NIM_DRIVER_SET_MODE:
		{
			break;
		}
#ifdef ADC2DRAM_KENABLE
		case ALI_NIM_ADC2MEM_START:
		{
			adc_dma_is_test_mode[0] = ((UINT8 *)parg)[0];    //0x100
			adc_dma_is_test_mode[1] = ((UINT8 *)parg)[1];    //0x101
			adc_dma_is_test_mode[2] = ((UINT8 *)parg)[2];    //0x102
			adc_dma_is_test_mode[3] = ((UINT8 *)parg)[10];   //test mode?
			user_start_adc_dma = 1;
			g_rx_DMA_buffer = __G_MM_VIDEO_START_ADDR;
			g_rx_buffer_len = 32*1024*1024;
			ret = SUCCESS;
			break;
		}
		case ALI_NIM_ADC2MEM_STOP:
		{
			user_force_stop_adc_dma = 1;
			ret = SUCCESS;
			break;
		}
		case ALI_NIM_ADC2MEM_SEEK_SET:
		{
			g_rx_buf_start_addr = (UINT8 *)(((UINT32)g_rx_DMA_buffer)&0xfffffffc);
			g_rx_buf_cur = (UINT32)parg;
			ATBM_PRINTF("adc data buf: %p, cur: 0x%08x\n", g_rx_buf_start_addr, g_rx_buf_cur);
			break;
		}
		case ALI_NIM_ADC2MEM_READ_8K:
		{
			break;
		}
#endif
		default:
		{
			ret = -ENOIOCTLCMD;
			break;
		}
	}
	return ret;
}
RET_CODE ali_m3200_nim_open(struct inode *inode, struct file *file)
{
	struct ali_nim_m3200_private *priv = ali_m3200_nim_priv; //priv->priv;

	ali_m3200_nim_dev.priv = (void *)priv;
	file->private_data = (void*) & ali_m3200_nim_dev;
	
	//The next function is to initialize demodulator after STB power up. 
	//It MUST BE the first function to call before any other operation on the demodulator.
	ATBMPowerOnInit();  
	//The next function is to configure demodulator working for DTMB or DVBC demodulation
	ATBMSetDTMBMode();  	//or	ATBMSetDVBCMode(); 
	return RET_SUCCESS;
}

#if 0
static void nim_s3202_sw_test_thread(struct work_struct *work)
{
	UINT8 i;
	static UINT32 curt_time, last_time[10];

	for (i = 0; i < 10; i++)
		last_time[i] = 0;

	while(1)
	{
		if(1!=nim_sw_test_tread_status)
		{
			nim_sw_test_tread_status=0;
			break;
		}

		curt_time = osal_get_tick();

#ifdef ADC2DRAM_KENABLE
		if(user_start_adc_dma)
		{
			user_start_adc_dma = 0;
            		//g_rx_DMA_buffer = __G_MM_VIDEO_START_ADDR;
			nim_start_adc2mem(g_rx_DMA_buffer, g_rx_buffer_len/*32*1024*1024*/, adc_dma_is_test_mode);
			last_time[0] = osal_get_tick();
			ATBM_PRINTF("ADC_DMA -> Memory finished. tick: %d ms, writing to flash, pls wait...\n", last_time[0]-curt_time);
		}
#endif
		if (curt_time - last_time[0] > 300)
		{
		//	nim_s3202_monitor_berper(priv, &BER_VALID);
			last_time[0] = osal_get_tick();
		}

		msleep(1000);
	}
}
#endif

RET_CODE ali_m3200_nim_release(struct inode *inode, struct file *file)
{
	INT32 ret = SUCCESS;
	struct ali_nim_device *dev = file->private_data;
	struct ali_nim_m3200_private *priv = dev->priv;

	//soft reset.
	//Data = 0x80;
	//nim_s3202_write(NIM_S3202_CONTROL1, &Data, 1);

	if (NULL != priv->nim_Tuner_Release)
	{
	    priv->nim_Tuner_Release();
	}

	ATBM_PRINTF("[NOTE] RELEASE FUNCTION \n");
	return ret;
}

static int __devinit ali_m3200_nim_init(void)
{
	INT32 ret;
	dev_t devno;

	ali_m3200_nim_priv = kmalloc(sizeof(struct ali_nim_m3200_private), GFP_KERNEL);
	if (!ali_m3200_nim_priv)
		return -ENOMEM;
	memset(ali_m3200_nim_priv, 0, sizeof(struct ali_nim_m3200_private));
	mutex_init(&ali_m3200_nim_priv->i2c_mutex);
	ret = alloc_chrdev_region(&devno, 0, 1, ALI_NIM_DEVICE_NAME);
	if (ret < 0)
	{
		NIM_LOG_ERROR("[ERR] Alloc device region failed, err: %d.\n", ret);
		return ret;
	}

	cdev_init(&ali_m3200_nim_dev.cdev, &ali_m3200_nim_fops);
	ali_m3200_nim_dev.cdev.owner = THIS_MODULE;
	ali_m3200_nim_dev.cdev.ops = &ali_m3200_nim_fops;
	ret = cdev_add(&ali_m3200_nim_dev.cdev, devno, 1);
	if (ret)
	{
		//use error print
		NIM_LOG_ERROR("[ERR] Alloc NIM device failed, err: %d.\n", ret);
		//PRINTK_INFO("[ERR] Alloc NIM device failed, err: %d.\n", ret);
		goto error1;
	}

	ATBM_PRINTF("register NIM device end.\n");

	ali_m3200_nim_class = class_create(THIS_MODULE, "ali_m3200_nim_class");

	if (IS_ERR(ali_m3200_nim_class))
	{
		ret = PTR_ERR(ali_m3200_nim_class);

		goto error2;
	}

	ali_m3200_nim_dev_node = device_create(ali_m3200_nim_class, NULL, devno, &ali_m3200_nim_dev,
										   "ali_m3200_nim0");
	if (IS_ERR(ali_m3200_nim_dev_node))
	{
		NIM_LOG_ERROR(KERN_ERR "device_create() failed!\n");
		//PRINTK_INFO(KERN_ERR "device_create() failed!\n");

		ret = PTR_ERR(ali_m3200_nim_dev_node);

		goto error3;
	}

	//init tuner module list
	INIT_LIST_HEAD(&(TUNER_ALIVE_HEAD.list));
	return ret;

error3:
	class_destroy(ali_m3200_nim_class);
error2:
	cdev_del(&ali_m3200_nim_dev.cdev);
error1:
	mutex_destroy(&ali_m3200_nim_priv->i2c_mutex);
	kfree(ali_m3200_nim_priv);

	return ret;
}

static void __exit ali_m3200_nim_exit(void)
{
	struct tuner_handle *del_model;
    	struct list_head *pos, *n;

	if (ali_m3200_nim_dev_node != NULL)
	{
		device_del(ali_m3200_nim_dev_node);
	}
	if (ali_m3200_nim_class != NULL)
	{
		class_destroy(ali_m3200_nim_class);
	}
	cdev_del(&ali_m3200_nim_dev.cdev);
	mutex_destroy(&ali_m3200_nim_priv->i2c_mutex);
	kfree(ali_m3200_nim_priv);
    
	//destroy tuner module list
	list_for_each_safe(pos, n, &(TUNER_ALIVE_HEAD.list))
	{
		del_model = list_entry(pos, struct tuner_handle, list);
		list_del_init(pos);
		kfree(del_model);
	}
	g_tuner_model_alive = 0;
	g_tuner_index_online = 0;
	memset(g_tuner_name_online, 0, sizeof(g_tuner_name_online));	
}

module_init(ali_m3200_nim_init);
module_exit(ali_m3200_nim_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Corey");
MODULE_DESCRIPTION("Ali M3200 full NIM driver");
	
