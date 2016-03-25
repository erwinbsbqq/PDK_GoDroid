/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    ALi_nim_mxl241.c
*
*    Description:    Source file in alidriver,mxl241 dirver for linux.
*    History:
*           Date            	Athor        	Version          				Reason
*	    ============	=============	=========	=================
*	1.20121018	    	coeus		Ver linux1.0	Create for ali linux system
*****************************************************************************/

#include "ali_nim_mxl241.h"



#if (1)
	#define MXL241_PRINTF		printk
#else
	#define MXL241_PRINTF(...)		do{}while(0)
#endif


/* Need to modify here for different perpose */
//===============
#ifdef TUNER_I2C_BYPASS
#define	I2C_BYPASS
#endif
//===============

#define	NIM_mxl241_FLAG_ENABLE			0x00000100
#define NIM_mxl241_SCAN_END             0x00000001
#define NIM_mxl241_CHECKING			    0x00000002
#define OSAL_INVALID_ID		            0
#define MXL241_CHIP_ADRRESS             0xC0//0xC6//99//0x63



struct ali_nim_device ali_mxl241_nim_dev;
struct ali_nim_mxl241_private *ali_mxl241_nim_priv=NULL;
struct class *ali_mxl241_nim_class = NULL;
struct device *ali_mxl241_nim_dev_node = NULL;

static struct mxl241_Lock_Info mxl241_CurChannelInfo;

static UINT32  BER_COUNTS=0;
static UINT32  PER_COUNTS=0;
static BOOL    BER_VALID=FALSE;

static UINT8   usage_QAM_type = 0;	


static INT32 nim_mxl241_channel_change(struct ali_nim_mxl241_private *priv, struct NIM_CHANNEL_CHANGE* pstChl_Change);
static INT32 nim_mxl241_channel_search(struct ali_nim_mxl241_private *priv, UINT32 freq);

static INT32 nim_mxl241_get_BER(struct ali_nim_mxl241_private *priv, UINT32 *err_count);
static INT32 nim_mxl241_get_lock(struct ali_nim_mxl241_private *priv, UINT8 *lock);
static INT32 nim_mxl241_get_freq(struct ali_nim_mxl241_private *priv, UINT32 *freq);
static INT32 nim_mxl241_get_symbol_rate(struct ali_nim_mxl241_private *priv, UINT32 *sym_rate);
static INT32 nim_mxl241_get_qam_order(struct ali_nim_mxl241_private *priv, UINT8 *QAM_order);
static INT32 nim_mxl241_get_agc(struct ali_nim_mxl241_private *priv, UINT8 *agc);
static INT32 nim_mxl241_get_snr(struct ali_nim_mxl241_private *priv, UINT8 *snr);
static INT32 nim_mxl241_get_per(struct ali_nim_mxl241_private *priv, UINT32 *RsUbc);

static INT32 nim_mxl241_get_rf_level(struct ali_nim_mxl241_private *priv, UINT16 *RfLevel);
static INT32 nim_mxl241_get_cn_value(struct ali_nim_mxl241_private *priv, UINT16 *CNValue);
static void nim_mxl241_set_qam_type(struct ali_nim_mxl241_private *priv, UINT32 mode);
static UINT32 Log10Times100_L( UINT32 x);

long     ali_mxl241_nim_ioctl(struct file *file, unsigned int cmd, void *parg);
RET_CODE ali_mxl241_nim_open(struct inode *inode, struct file *file);
RET_CODE ali_mxl241_nim_release(struct inode *inode, struct file *file);


static struct file_operations ali_mxl241_nim_fops = {
	.owner		= THIS_MODULE,
	.write		= NULL, 
	.unlocked_ioctl	= ali_mxl241_nim_ioctl,
	.open		= ali_mxl241_nim_open,
	.release		=  ali_mxl241_nim_release,
};



/*
Add for debug by Coeus 2012/10/27 
You can read or set any register by
*/

UINT16 address[64] =
{	
	AIC_RESET_REG,
	SLEEP_MODE_CTRL_REG,
	XTAL_CLK_OUT_CTRL_REG,
	DIG_XTAL_FREQ_CTRL_REG,
	DIG_XTAL_FREQ_CTRL_REG,
	DIG_XTAL_BIAS_CTRL_REG,
	TOP_MASTER_CONTROL_REG,
	SYMBOL_RATE_RESAMP_BANK_REG,
	SYMBOL_RATE_RESAMP_RATE_LO_REG,
	SYMBOL_RATE_RESAMP_RATE_MID_LO_REG,

	SYMBOL_RATE_RESAMP_RATE_MID_HI_REG,
	SYMBOL_RATE_RESAMP_RATE_HI_REG,
	DIG_HALT_GAIN_CTRL_REG,
	DIG_AGC_SETPT_CTRL_LO_REG,
	DIG_AGC_SETPT_CTRL_HI_REG,
	STAMP_REG,
	MCNSSD_SEL_REG,
	MCNSSD_REG,
	FRCNT_REG,
	MEF_REG,

	QAM_LOCK_STATUS_REG,
	FEC_MPEG_LOCK_REG,
	DIG_ADCIQ_FLIP_REG,
	QAM_ANNEX_TYPE_REG,
	QAM_TYPE_AUTO_DETECT_REG,
	SNR_MSE_AVG_COEF_REG,
	SNR_EXT_SPACE_ADDR_REG,
	SNR_EXT_SPACE_DATA_REG,
	MPEGOUT_PAR_CLK_CTRL_REG,
	MPEGOUT_EXT_MPEGCLK_INV_REG,

	MPEGOUT_SER_CTRL_REG,
	EQ_SPUR_BYPASS_REG,
	CARRIER_OFFSET_REG,
	RF_CHAN_BW_REG,
	RF_LOW_REG,
	RF_HIGH_REG,
	RF_SEQUENCER_REG,
	RF_LOCK_STATUS_REG,
	AGC_LOCK_STATUS_REG,
	RF_PIN_RB_EN_REG,

	DIG_RF_PIN_RB_LO_REG,
	DIG_RF_PIN_RB_HI_REG,
	INTERLEAVER_DEPTH_REG,
	CUSTOM_COEF_EN_REG,
	EQU_FREQ_SWEEP_LIMIT_REG,
	EQUALIZER_FILTER_REG,
	EQUALIZER_FILTER_DATA_RB_REG,
	CARRIER_OFFSET_RB_REG,
	FREEZE_AGC_CFG_REG,
	INTR_MASK_REG,

	INTR_STATUS_REG,
	INTR_CLEAR_REG,
	RESAMP_BANK_REG,
	RATE_RESAMP_RATE_MID_LO_REG,
	RATE_RESAMP_RATE_MID_HI_REG,
	GODARD_ACC_REG,
	PHY_EQUALIZER_FFE_FILTER_REGISTER,
	PHY_EQUALIZER_DFE_FILTER_REGISTER,
	PHY_EQUALIZER_LLP_CONTROL_1_DEBUG_MSE_REGISTER,
	OVERWRITE_DEFAULT_REG,

	AGC_CTRL_SPEED_REG,
	DN_LNA_BIAS_CTRL_REG,
	DIG_PIN_2XDRV_REG,
	RF_SPARE_REG,
};




/*------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_WriteRegister
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 7/30/2009
--|
--| DESCRIPTION   : This function does I2C write operation.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/
MXL_STATUS Ctrl_WriteRegister(unsigned char I2cSlaveAddr, unsigned short RegAddr, unsigned short RegData)
{
	MXL_STATUS status = MXL_TRUE;
	
#if 1
	struct ali_nim_mxl241_private *priv=ali_mxl241_nim_priv;
	unsigned char Write_Cmd[4] = {0}; 
	
	NIM_MUTEX_ENTER(priv);
	Write_Cmd[0] = (RegAddr>>8)&0xFF;
	Write_Cmd[1] = (RegAddr)&0xFF;
	Write_Cmd[2] = (RegData>>8)&0xFF;
	Write_Cmd[3] = (RegData)&0xFF;
	
	// User should implememnt his own I2C write register routine corresponding to
	// his hardaware.

	status=ali_i2c_write(priv->tuner_config_ext.i2c_type_id, priv->tuner_config_ext.c_tuner_base_addr, Write_Cmd, 4);
	if(status!=0)
	{
		printk("[%s]line=%d,status %d : Addr[ 0x%x]=  0x%x\n",__FUNCTION__,__LINE__, status,RegAddr, RegData); 
	}
	//printk("[%s]line=%d,end!\n",__FUNCTION__,__LINE__);
	NIM_MUTEX_LEAVE(priv);
#endif
	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_ReadRegister
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 7/30/2009
--|
--| DESCRIPTION   : This function does I2C read operation.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS Ctrl_ReadRegister(unsigned char I2cSlaveAddr, UINT16 RegAddr, UINT16 *DataPtr)
{
	MXL_STATUS status = MXL_TRUE;
#if 1	
	unsigned char Read_Cmd[4] = {0};
	unsigned char	Read_data[2] = {0};
	struct ali_nim_mxl241_private *priv=ali_mxl241_nim_priv;

	NIM_MUTEX_ENTER(priv);
	
	/* read step 1. accroding to mxl5007 driver API user guide. */
	Read_Cmd[0] = 0xFF;
	Read_Cmd[1] = 0xFB;
	Read_Cmd[2] = (RegAddr>>8)&0xFF;//ADD_HIGH
	Read_Cmd[3] = RegAddr&0xFF;//ADD_LOW
	
	status=ali_i2c_write(priv->tuner_config_ext.i2c_type_id, priv->tuner_config_ext.c_tuner_base_addr, Read_Cmd, 4);
	status=ali_i2c_read(priv->tuner_config_ext.i2c_type_id, priv->tuner_config_ext.c_tuner_base_addr, Read_data, 2);
	*DataPtr=(Read_data[0]<<8)|Read_data[1];
	if(status!=0)
	{	
		printk("[%s]line=%d,status  %d  : Addr [ 0x%x]= 0x%x\n",__FUNCTION__,__LINE__,status, RegAddr, *DataPtr); 
	}
	//printk("[%s]line=%d,end!\n",__FUNCTION__,__LINE__);
	NIM_MUTEX_LEAVE(priv);
	
#endif
	return status;
}


void MxlReadAllRegister()
{	
	//return ;
	//printk("==========================\n");
	UINT8 i = 0;
	UINT16 temp_value = 0;
	
	while(0)//(i<64)
	{
		Ctrl_ReadRegister(0, address[i], &temp_value);
		MXL241_PRINTF("reg:0x%04xH,value:%04x\n",address[i],temp_value);
		i++;
	}
	
	/*
	Ctrl_ReadRegister(0, address[0], &temp_value);
	printk("add:0x80a3,value:%04x\n",temp_value);
	Ctrl_ReadRegister(0, address[1], &temp_value);
	printk("add:0x00f8,value:%04x\n",temp_value);
	Ctrl_ReadRegister(0, address[2], &temp_value);
	printk("add:0x800E,value:%04x\n",temp_value);

	*/
	//printk("==========================\n");
	
}

void SetMxlRegister(void)
{
	UINT16 Data = 0;
	
	//printk("bef:add:0x000a,value:%04x\n",Data);
	Data=0xffff;
	Ctrl_WriteRegister(0, 0x0002, Data);
	Data=0;
	Ctrl_ReadRegister(0, 0x0002, &Data);
	//printk("aft:add:0x0002,value:%04x\n",Data);

	//printk("bef:add:0x0009,value:%04x\n",Data);
	Data=0xffff;
	Ctrl_WriteRegister(0, 0x0009, Data);
	Data=0;
	Ctrl_ReadRegister(0, 0x0009, &Data);
	//printk("aft:add:0x0009,value:%04x\n",Data);
	
}
void nim_mxl241_reg_dump(void)
{
	UINT16 Data,i=0;

#if 1
	Data=0;
	Ctrl_ReadRegister(0, 0x0017, &Data);
	msleep(1);
	MXL241_PRINTF("Reg[ID]=%x \n",Data);

	Data=0x0014;
	Ctrl_WriteRegister(0, 0x0006, Data);
	Data=0;
	Ctrl_ReadRegister(0, 0x0006, &Data);
	msleep(100);
	MXL241_PRINTF("Reg[LT]=%x \n",Data);
	Data=0;
	Ctrl_ReadRegister(0, 0x0027, &Data);
	msleep(1);
	MXL241_PRINTF("Reg[IQ]=%x \n",Data);
#else	
	for(i=0x0000;i<0xffff;i++)
	{
		Data=0;
		Ctrl_ReadRegister(0, i, &Data);
		msleep(1);
		printk("Reg[%x]=%x \n",i,Data);
	}	
#endif

}

/***************debug end******************/

void nim_mxl241_dump_all(void)
{
	UINT16 Data,i=0;

	for(i=0x0000;i<0xffff;i++)
	{
		Data=0;
		Ctrl_ReadRegister(0, i, &Data);
		msleep(1);
		MXL241_PRINTF("Reg[%x]=%x \n",i,Data);
	}	
}

/*****************************************************************************
* INT32 nim_mxl241_open(struct nim_device *dev)
* Description: mxl241 open
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/


RET_CODE  ali_mxl241_nim_open(struct inode *inode, struct file *file)
{
	struct ali_nim_mxl241_private *priv = ali_mxl241_nim_priv; //priv->priv;

	MXL241_PRINTF("%s\n", __FUNCTION__);
	ali_mxl241_nim_dev.priv = (void *)priv;
	file->private_data=(void*)&ali_mxl241_nim_dev;
	return RET_SUCCESS;
}

static INT32 nim_mxl241_channel_search(struct ali_nim_mxl241_private *priv, UINT32 freq)
{
	return SUCCESS;
}

static INT32 nim_mxl241_get_lock(struct ali_nim_mxl241_private *priv, UINT8 *lock)
{
	MXL_STATUS status = MXL_TRUE;
	MXL_DEMOD_LOCK_STATUS_T       mxL241sf_lockstatus;

	mxL241sf_lockstatus.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;
	status = MxLWare_API_GetDemodStatus(MXL_DEMOD_MPEG_LOCK_REQ, (void*)&mxL241sf_lockstatus);
	if (mxL241sf_lockstatus.Status == 1)
	{		 
		*lock = 1;
	}
	else
	{
		*lock = 0;
	}
	return SUCCESS;
}

static INT32 nim_mxl241_get_freq(struct ali_nim_mxl241_private *priv, UINT32 *freq)
{
	*freq = mxl241_CurChannelInfo.Frequency/10;
	MXL241_PRINTF("*freq is %d !\n", *freq);
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_mxl241_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
* Read mxl241 symbol rate
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *sym_rate			: Symbol rate in kHz
*
* Return Value: void
*****************************************************************************/
static INT32 nim_mxl241_get_symbol_rate(struct ali_nim_mxl241_private *priv, UINT32 *sym_rate)
{
	*sym_rate =mxl241_CurChannelInfo.SymbolRate;
	return SUCCESS;
}

static INT32 nim_mxl241_get_qam_order(struct ali_nim_mxl241_private *priv, UINT8 *qam_order)//fec
{
	*qam_order =mxl241_CurChannelInfo.Modulation;
	
	return SUCCESS;
}

static INT32 nim_mxl241_get_fft_result(struct ali_nim_mxl241_private *priv, UINT32 freq, UINT32* start_adr )
{
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_mxl241_get_AGC(struct nim_device *dev, UINT8 *agc)
*
*  This function will access the NIM to determine the AGC feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* agc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_mxl241_get_agc(struct ali_nim_mxl241_private *priv, UINT8 *agc)
{
	UINT8 data[4] = {0};
	UINT8	RF_AGC_MAX_priv=0;
	UINT8	RF_AGC_MIN_priv=0 ;
	UINT8	IF_AGC_MAX_priv=0;
	UINT8	IF_AGC_MIN_priv=0;


	RF_AGC_MAX_priv=data[0];
	RF_AGC_MIN_priv=data[1] ;
	IF_AGC_MAX_priv=data[2];
	IF_AGC_MIN_priv=data[3];
	
	*agc = 100;	
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_mxl241_get_SNR(struct nim_device *dev, UINT8 *snr)
*
* This function returns an approximate estimation of the SNR from the NIM
*  The Eb No is calculated using the SNR from the NIM, using the formula:
*     Eb ~     13312- M_SNR_H
*     -- =    ----------------  dB.
*     NO           683
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_mxl241_get_snr(struct ali_nim_mxl241_private *priv, UINT8 *snr)
{
	UINT8 data[2] = {0};
	UINT32 rpt_power = 0;
	
	*snr = 100;
	
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_mxl241_get_RF_Level(struct nim_device *dev, UINT16 *RfLevel)
*
*  This function will access the NIM to determine the RF level feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *RfLevel
*  Real_RF_level and RfLevel relation is : RfLevel = -(Real_RF_level * 10)
*  eg.  if Real_RF_level = -30.2dBm then , RfLevel = -(Real_RF_level * 10) = 302
* Return Value: INT32
*****************************************************************************/
static INT32 nim_mxl241_get_rf_level(struct ali_nim_mxl241_private *priv, UINT16 *rflevel)
{
	UINT8 data[3]={0};
	UINT16 if_agc_gain = 0;
	UINT16 rf_agc_gain = 0;
	UINT32 temp32=0;
	
	temp32 = 460;
	*rflevel = (UINT16)temp32;
	
	return SUCCESS;	
}

/*****************************************************************************
* INT32 nim_mxl241_get_CN_value(struct nim_device *dev, UINT16 *CNValue)
*
*  This function will access the NIM to determine the C/N  feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *CNValue
*  Real_CN_value and CNValue relation is : CNValue = Real_CN_value * 10
*  eg.  if Real_CN_value = 28.3dB then , CNValue = Real_RF_level * 10 = 283
* Return Value: INT32
*****************************************************************************/

static INT32 nim_mxl241_get_cn_value(struct ali_nim_mxl241_private *priv, UINT16 *cnvalue)
{
	*cnvalue = 30;//cnr;
	return SUCCESS;	
}

/*****************************************************************************
* INT32 nim_mxl241_get_PER(struct nim_device *dev, UINT32 *RsUbc)
* Reed Solomon Uncorrected block count
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_mxl241_get_per(struct ali_nim_mxl241_private *priv, UINT32 *rsubc)
{
	*rsubc = PER_COUNTS;
	return SUCCESS;
}

static INT32 nim_mxl241_get_ber(struct ali_nim_mxl241_private *priv, UINT32 *err_count)
{
	MXL_DEMOD_BER_INFO_T berinfoptr;
	MXL_STATUS status = MXL_TRUE;
	
	*err_count = BER_COUNTS;

	berinfoptr.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;//99; 
	status=MxLWare_API_GetDemodStatus(MXL_DEMOD_BER_UNCODED_BER_CER_REQ, (void*)&berinfoptr);
	//printk("ber=%d \n",BerInfoPtr.BER);

	return SUCCESS;
}

static void nim_mxl241_set_qam_type(struct ali_nim_mxl241_private *priv, UINT32 mode)
{
	MXL_SYMBOL_RATE_T	mxl241sf_symbolrate;
	MXL_ANNEX_CFG_T 	mxl241sf_annextype;
	MXL_RESET_CFG_T 	mxl241sf_reset;

    MXL241_PRINTF("[%s] line=%d,mode=%d\n", __FUNCTION__,__LINE__,mode);
	
	usage_QAM_type = mode  & 0x01;  
	
	mxl241sf_symbolrate.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;
	
	if(NIM_DVBC_J83B_MODE == usage_QAM_type)
	{
		mxl241sf_symbolrate.SymbolType = SYM_TYPE_USER_DEFINED_J83B;
	}	
	else
	{
		mxl241sf_symbolrate.SymbolType = SYM_TYPE_USER_DEFINED_J83A;
	}	

	MxLWare_API_ConfigDemod(MXL_DEMOD_SYMBOL_RATE_CFG, (void*)&mxl241sf_symbolrate);

	msleep(1);
	
	mxl241sf_annextype.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;
	if(NIM_DVBC_J83B_MODE == usage_QAM_type)
	{
		mxl241sf_annextype.AnnexType = ANNEX_B;
	}	
	else
	{
		mxl241sf_annextype.AnnexType = ANNEX_A;
	}	

	MxLWare_API_ConfigDemod(MXL_DEMOD_ANNEX_QAM_TYPE_CFG, (void*)&mxl241sf_annextype);

	msleep(1);

	mxl241sf_reset.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;
	MxLWare_API_ConfigDevice(MXL_DEV_SOFT_RESET_CFG, (void*)&mxl241sf_reset);

	msleep(10);
}

/*****************************************************************************
* INT32 nim_mxl241_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);
* Description: mxl241 channel change operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq				: Frequence
*  Parameter3: UINT32 sym				: Symbol rate
*  Parameter4: UINT8 fec				: Code rate
*
* Return Value: INT32
*****************************************************************************/

static INT32 nim_mxl241_channel_change(struct ali_nim_mxl241_private *priv, struct NIM_CHANNEL_CHANGE* pstChl_Change)
{
	UINT32 freq = 0;// = 54600;
	UINT32 sym = 0;// = 6875;
	UINT8 fec = 0;// = 8;
	UINT16 data = 0;
	UINT8 lockget = 0;
	UINT32 loop = 0;
	
	MXL_STATUS status;
  	MXL_RF_TUNE_CFG_T mxl241sf_chantune;
  	MXL_DEMOD_LOCK_STATUS_T mxL241sf_lockstatus;
  	MXL_SYMBOL_RATE_T mxL241sf_symbolrate;
  	MXL_AGC_T mxl241sf_agc;
  	MXL_TOP_MASTER_CFG_T mxl241sf_poweruptuner;
 	MXL_ANNEX_CFG_T mxl241sf_annextype;
  	MXL_PWR_MODE_CFG_T mxl241sf_pwrmode;
  	MXL_MPEG_OUT_CFG_T mxl241sf_mpegout;
  	MXL_ADCIQFLIP_CFG_T mxl241sf_adcIpflip;

  	MXL_CHAN_DEPENDENT_T mxl241sf_chandependent;
	MXL_RESET_CFG_T mxl241sf_reset;
  	MXL_OVERWRITE_DEFAULT_CFG_T mxl241sf_overwritedefault;
  	MXL_DEV_INFO_T mxl241sf_deviceinfo;
 	MXL_XTAL_CFG_T mxl241sf_xtalsetting;



	freq = pstChl_Change->freq;
	sym = pstChl_Change->sym;
	fec = pstChl_Change->modulation;
	
	nim_mxl241_get_lock(priv, &lockget);
	
	// Add a comparametion, when the the new parameter and the existed parameter is the same ,
	// then return success directly without set it to the tuner and demod.
	// For the request of Mark_Li 2007/11/07
	if ((((freq - mxl241_CurChannelInfo.Frequency/10)<= 40)||\
		((mxl241_CurChannelInfo.Frequency/10 - freq)<= 40))&&\
	   	((sym - mxl241_CurChannelInfo.SymbolRate <=40)||\
	    	(mxl241_CurChannelInfo.SymbolRate - sym <=40))&&\
	    	(mxl241_CurChannelInfo.Modulation == fec)&&\
	    	(lockget ==1))
	{

		mxl241_CurChannelInfo.Frequency = freq/10;
		mxl241_CurChannelInfo.SymbolRate = sym;
		mxl241_CurChannelInfo.Modulation = fec;
		MXL241_PRINTF("MpegClkFreq=%d\n",mxl241sf_mpegout.MpegClkFreq);("return CG : lockget=%d ,freq=%d sym=%d ,fec=%d \n",lockget,freq,sym,fec);
	   	return SUCCESS;
	}	

	
	// 1. Do SW Reset
	mxl241sf_reset.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;//99;
  	status = MxLWare_API_ConfigDevice(MXL_DEV_SOFT_RESET_CFG, (void*)&mxl241sf_reset);
	if(status!=MXL_TRUE)
	{
		MXL241_PRINTF("[%s] line=%d,API return FALSE!\n", __FUNCTION__,__LINE__);
	}
	//printk("channel change step 2..\n");

	// 2. Overwrite default
  	mxl241sf_overwritedefault.I2cSlaveAddr =priv->tuner_config_ext.c_tuner_base_addr;// 99;
  	status = MxLWare_API_ConfigDevice(MXL_DEV_OVERWRITE_DEFAULT_CFG, (void*)&mxl241sf_overwritedefault);
	if(status!=MXL_TRUE)
	{
		MXL241_PRINTF("[%s] line=%d,API return FALSE!\n", __FUNCTION__,__LINE__);
	}
  	// 3. Read Back Device id and version
  	status = MxLWare_API_GetDeviceStatus(MXL_DEV_ID_VERSION_REQ, (void*)&mxl241sf_deviceinfo);
  	if (status == MXL_TRUE) 
  	{
    		MXL241_PRINTF("MxL241SF : DevId = 0x%x, Version = 0x%x \n\n", mxl241sf_deviceinfo.DevId, mxl241sf_deviceinfo.DevVer);
  	}

  	// 4. XTAL and Clock out setting
  	mxl241sf_xtalsetting.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;//99;
  	mxl241sf_xtalsetting.XtalEnable = MXL_ENABLE;
  	mxl241sf_xtalsetting.DigXtalFreq = XTAL_24MHz;
  	mxl241sf_xtalsetting.XtalBiasCurrent = 1;
  	mxl241sf_xtalsetting.XtalCap = 10;//15;//10; // 10pF
  	mxl241sf_xtalsetting.XtalClkOutEnable = MXL_ENABLE;
  	mxl241sf_xtalsetting.XtalClkOutGain =  0xa; 
  	mxl241sf_xtalsetting.LoopThruEnable = MXL_ENABLE;
  	status=MxLWare_API_ConfigDevice(MXL_DEV_XTAL_SETTINGS_CFG, (void*)&mxl241sf_xtalsetting);
	if(status!=MXL_TRUE)
	{
		MXL241_PRINTF("[%s] line=%d,API return FALSE!\n", __FUNCTION__,__LINE__);
	}
	
  	// 5. AGC configuration
  	mxl241sf_agc.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;//99;
  	mxl241sf_agc.FreezeAgcGainWord = MXL_NO_FREEZE;
  	status=MxLWare_API_ConfigTuner(MXL_TUNER_AGC_SETTINGS_CFG, (void*)&mxl241sf_agc);
	if(status!=MXL_TRUE)
	{
		MXL241_PRINTF("[%s] line=%d,API return FALSE!\n", __FUNCTION__,__LINE__);
	}
	
  	// 6. Power Up Tuner
  	mxl241sf_poweruptuner.I2cSlaveAddr =priv->tuner_config_ext.c_tuner_base_addr;// 99;
  	mxl241sf_poweruptuner.TopMasterEnable = MXL_ENABLE;

    msleep(1);

  	status=MxLWare_API_ConfigTuner(MXL_TUNER_TOP_MASTER_CFG, (void*)&mxl241sf_poweruptuner);
	if(status!=MXL_TRUE)
	{
		MXL241_PRINTF("[%s] line=%d,API return FALSE!\n", __FUNCTION__,__LINE__);
	}
		
  	//printk("channel change step 3..\n");

	// 7. MPEG out setting
	if( priv->tuner_config_ext.c_tuner_special_config)
	{
		mxl241sf_mpegout.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;//99;
		mxl241sf_mpegout.SerialOrPar = MPEG_DATA_SERIAL;
		mxl241sf_mpegout.MpegValidPol = MPEG_ACTIVE_HIGH;//MPEG_CLK_IN_PHASE;//;
		mxl241sf_mpegout.MpegClkPol = MPEG_CLK_POSITIVE;//MPEG_CLK_POSITIVE;//MPEG_CLK_NEGATIVE;//
		mxl241sf_mpegout.MpegSyncPol = MPEG_ACTIVE_HIGH;//MPEG_CLK_IN_PHASE;//;
		mxl241sf_mpegout.MpegClkFreq = MPEG_CLK_57MHz;//MPEG_CLK_4_75MHz;
		mxl241sf_mpegout.MpegClkSource = MPEG_CLK_INTERNAL;	
		mxl241sf_mpegout.LsbOrMsbFirst = MPEG_SERIAL_MSB_1ST;
		mxl241sf_mpegout.MpegSyncPulseWidth=MPEG_SYNC_WIDTH_BYTE;

		MXL241_PRINTF("MPEG_DATA_SERIAL: %d\n",priv->tuner_config_ext.c_tuner_special_config);
	}
	else
	{
	  	mxl241sf_mpegout.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;//99;
	  	mxl241sf_mpegout.SerialOrPar = MPEG_DATA_PARALLEL;
	  	mxl241sf_mpegout.MpegValidPol = MPEG_ACTIVE_HIGH;
	  	mxl241sf_mpegout.MpegClkPol = MPEG_CLK_POSITIVE;//MPEG_CLK_NEGATIVE;
	  	mxl241sf_mpegout.MpegSyncPol = MPEG_ACTIVE_HIGH;
	  	mxl241sf_mpegout.MpegClkFreq = MPEG_CLK_19MHz;//MPEG_CLK_4_75MHz;
		mxl241sf_mpegout.MpegClkSource = MPEG_CLK_INTERNAL;
		
		MXL241_PRINTF("MPEG_DATA_PARALLEL: %d\n",priv->tuner_config_ext.c_tuner_special_config);
	}
  	status=MxLWare_API_ConfigDemod(MXL_DEMOD_MPEG_OUT_CFG, (void*)&mxl241sf_mpegout);
	if(status!=MXL_TRUE)
	{
		MXL241_PRINTF("[%s] line=%d,API return FALSE!\n", __FUNCTION__,__LINE__);
	}
		
	//Up_layer send freq para is not "KHz", should be multipier 10 time to "KHz".
	freq = freq*10; //KHZ
	
	//adjust freq offset
	//kent,2013.5.13
	/*
	if(freq >= 150000 && freq < 474000)
		freq -= 50;
	else if(freq >= 474000 && freq < 666000)	
		freq -= 100;
	else if(freq >= 666000)	
		freq -= 150;
	*/	
	// To watch the input parameter	
	char  *char_qam;	
	switch(fec)
	{
		case 4: char_qam = "16QAM";break;
		case 5: char_qam = "32QAM";break;
		case 6: char_qam = "64QAM";break;
		case 7: char_qam = "128QAM";break;
		case 8: char_qam = "256QAM";break;
		default: char_qam = "NONE"; break;
	}

	MXL241_PRINTF("%s,freq = %d, sym = %d, fec = %s QAM_type=%d\n",__FUNCTION__, freq,sym,char_qam,usage_QAM_type);

	mxl241_CurChannelInfo.Frequency = freq;
	mxl241_CurChannelInfo.SymbolRate = sym;
	mxl241_CurChannelInfo.Modulation = fec;

 	// 8. Symbol rate
  	mxL241sf_symbolrate.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;

	if(usage_QAM_type)
	{
		mxL241sf_symbolrate.SymbolType = SYM_TYPE_USER_DEFINED_J83B;
		
		if((fec==6)||(fec==8))
		{	
			mxL241sf_symbolrate.SymbolRate =((REAL32)(sym)/1000);
			mxL241sf_symbolrate.SymbolRate256=((REAL32)(sym)/1000);	
		}
		else
		{
			mxL241sf_symbolrate.SymbolRate =0;
		}	
	}
	else
	{
		mxL241sf_symbolrate.SymbolType = SYM_TYPE_USER_DEFINED_J83A;
 		// sym=sym/10;
  		mxL241sf_symbolrate.SymbolRate =sym;//((REAL32)(sym)/1000);//6.89;//(REAL32)(((REAL32)(sym))/100.0);//(REAL32)6.952;
	}
	
   	
  	status=MxLWare_API_ConfigDemod(MXL_DEMOD_SYMBOL_RATE_CFG, (void*)&mxL241sf_symbolrate);
	if(status!=MXL_TRUE)
	{
		MXL241_PRINTF("[%s] line=%d,API return FALSE!\n", __FUNCTION__,__LINE__);
	}
		
	
  	msleep(1);

  	// 9. Config Annex Type
  	mxl241sf_annextype.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;//99;

	if(usage_QAM_type)
	{
		mxl241sf_annextype.AnnexType = ANNEX_B;
		
		mxl241sf_annextype.AutoDetectMode = MXL_ENABLE;//ANNEX detect

#if 0//AutoDetectQam
		mxl241sf_annextype.AutoDetectQamType = MXL_ENABLE;//MXL_ENABLE;
#else
		mxl241sf_annextype.AutoDetectQamType = MXL_DISABLE;//MXL_ENABLE;

		switch(fec)
		{
			case 6: mxl241sf_annextype.QamType=MXL_QAM64;break;		
			case 8: mxl241sf_annextype.QamType=MXL_QAM256;break;
			default: mxl241sf_annextype.QamType=MXL_QAM64; break;
		}
#endif
		status=MxLWare_API_ConfigDemod(MXL_DEMOD_ANNEX_QAM_TYPE_CFG, (void*)&mxl241sf_annextype);
		if(status!=MXL_TRUE)
		{
			MXL241_PRINTF("[%s] line=%d,API return FALSE!\n", __FUNCTION__,__LINE__);
		}
	
	}
  	else
	{

		mxl241sf_annextype.AnnexType = ANNEX_A;
	  	mxl241sf_annextype.AutoDetectQamType = MXL_DISABLE;//MXL_ENABLE;
	  	mxl241sf_annextype.AutoDetectMode = MXL_ENABLE;
	  	//mxl241sf_annextype.QamType=MXL_QAM256;
		switch(fec)
		{
			case 4: mxl241sf_annextype.QamType=MXL_QAM16;break;
			case 5: mxl241sf_annextype.QamType=MXL_QAM32;break;
			case 6: mxl241sf_annextype.QamType=MXL_QAM64;break;
			case 7: mxl241sf_annextype.QamType=MXL_QAM128;break;
			case 8: mxl241sf_annextype.QamType=MXL_QAM256;break;
			default: mxl241sf_annextype.QamType=MXL_QAM64; break;
		}
		status=MxLWare_API_ConfigDemod(MXL_DEMOD_ANNEX_QAM_TYPE_CFG, (void*)&mxl241sf_annextype);
		if(status!=MXL_TRUE)
		{
			MXL241_PRINTF("[%s] line=%d,API return FALSE!\n", __FUNCTION__,__LINE__);
		}
	  }
  	// 10. Do MiscSettings if needed
  	// 11. Tune RF with channel frequency and bandwidth
  	mxl241sf_chantune.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;
  	mxl241sf_chantune.BandWidth = 8;                  // 8MHz
  	mxl241sf_chantune.Frequency = freq*1000;;         // UINT:HZ
  	status=MxLWare_API_ConfigTuner(MXL_TUNER_CHAN_TUNE_CFG, (void*)&mxl241sf_chantune );
	if(status!=MXL_TRUE)
	{
		MXL241_PRINTF("[%s] line=%d,API return FALSE!\n", __FUNCTION__,__LINE__);
	}
		
	//printk("channel change step 6..\n");
    msleep(30);

	if(usage_QAM_type==0)  //ANNEX_A :nomal ANNEX_B :inverse
	{
	  	// 12. Enable I/Q path flip
	    mxl241sf_adcIpflip.I2cSlaveAddr =priv->tuner_config_ext.c_tuner_base_addr;
	    mxl241sf_adcIpflip.AdcIqFlip = MXL_ENABLE;
	    status=MxLWare_API_ConfigDemod(MXL_DEMOD_ADC_IQ_FLIP_CFG, (void*)&mxl241sf_adcIpflip);  
		if(status!=MXL_TRUE)
		{
			MXL241_PRINTF("[%s] line=%d,API return FALSE!\n", __FUNCTION__,__LINE__);
		}
			
		msleep(25);
	}
	else
	{
		// 12. Enable I/Q path flip
	    	mxl241sf_adcIpflip.I2cSlaveAddr =priv->tuner_config_ext.c_tuner_base_addr;
	    	mxl241sf_adcIpflip.AdcIqFlip = MXL_DISABLE;
	    	status=MxLWare_API_ConfigDemod(MXL_DEMOD_ADC_IQ_FLIP_CFG, (void*)&mxl241sf_adcIpflip);  
			if(status!=MXL_TRUE)
			{
				MXL241_PRINTF("[%s] line=%d,API return FALSE!\n", __FUNCTION__,__LINE__);
			}
			msleep(25);		 
	}

#if (defined(MXL241_DBG))
	Ctrl_ReadRegister(0, 0x0027, &data);		
	MXL241_PRINTF("[%s] line=%d,Reg[I/Q flip]=%x \n", __FUNCTION__,__LINE__,data);
	
	Ctrl_ReadRegister(0, 0x00BD, &data);		
	MXL241_PRINTF("[%s] line=%d,Reg[VCO_BIOS]=%x \n", __FUNCTION__,__LINE__,data);
#endif

  	/* Please do not forget to put time delay at least 25ms */
  	// Wait 25ms
   	// osal_task_sleep(25);

  	// 13. Channel dependent setting
  	mxl241sf_chandependent.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;//99;
 	mxl241sf_chandependent.ChanDependentCfg = MXL_ENABLE;
  	status=MxLWare_API_ConfigTuner(MXL_TUNER_CHAN_DEPENDENT_TUNE_CFG, (void*)&mxl241sf_chandependent);
	if(status!=MXL_TRUE)
	{
		MXL241_PRINTF("[%s] line=%d,API return FALSE!\n", __FUNCTION__,__LINE__);
	}
  	/* Please do not forget to put time delay at least 300ms */
  	// Wait 300ms
    	//osal_task_sleep(300); 

	  // 14. Wait QAM_LOCK
	do
	{
		
	  	loop = 0; 
	 	while (loop < 10) //  This number is experimental to monitor statistics.
	  	{
	    	mxL241sf_lockstatus.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;
	    	status = MxLWare_API_GetDemodStatus(MXL_DEMOD_QAM_LOCK_REQ, (void*)&mxL241sf_lockstatus);
		   	if (mxL241sf_lockstatus.Status== 1)
		    {
		    	MXL241_PRINTF("QAM_LOCK : QAM_LOCK status = 0x%x\n", mxL241sf_lockstatus.Status); 
				break;
		    }
	  		msleep(10);
	    	loop++;
	 	}

		MXL241_PRINTF("[%s] line=%d,QAM_LOCK status = 0x%x\n", __FUNCTION__,__LINE__,mxL241sf_lockstatus.Status);
		
		//printk("channel change step 7..\n");
		 // 15. Wait FEC Lock
	  	loop = 0; 
	  	while (loop < 10) //  This number is experimental to monitor statistics.
	  	{
	    	mxL241sf_lockstatus.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;//99;   
	    	status = MxLWare_API_GetDemodStatus(MXL_DEMOD_FEC_LOCK_REQ, (void*)&mxL241sf_lockstatus);
	    	if (mxL241sf_lockstatus.Status== 1)
	    	{
	       		MXL241_PRINTF("FEC Lock  : FEC Lock status = 0x%x\n", mxL241sf_lockstatus.Status); 
		   		break;
	    	}
	  		msleep(10);
	    	loop++;
	 	 }
		
		MXL241_PRINTF("[%s] line=%d,FEC_LOCK status = 0x%x\n", __FUNCTION__,__LINE__,mxL241sf_lockstatus.Status);
		
		//printk("channel change step 8...\n");
		//16. MXL_DEMOD_MPEG_LOCK
	  	loop = 0; 
	  	while (loop < 10) //  This number is experimental to monitor statistics.
	  	{
	    	mxL241sf_lockstatus.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;//99;   
	    	status = MxLWare_API_GetDemodStatus(MXL_DEMOD_MPEG_LOCK_REQ, (void*)&mxL241sf_lockstatus);
	    	if (mxL241sf_lockstatus.Status== 1)
	    	{
				MXL241_PRINTF("MPEG_LOCK : MPEG_LOCK status = 0x%x\n", mxL241sf_lockstatus.Status); 
		     	break;
	    	}
		  	msleep(10);
		    loop++;
	  	}
	}while(0);
	MXL241_PRINTF("[%s] line=%d,MPEG_LOCK status = 0x%x\n", __FUNCTION__,__LINE__,mxL241sf_lockstatus.Status);
	if(mxL241sf_lockstatus.Status>0)
    {
		return SUCCESS;
	}
	
	return -1;
}

/*============================================================================*/
/**
* \fn UINT32 Log10Times100( UINT32 x)
* \brief Compute: 100*log10(x)
* \param x 32 bits
* \return 100*log10(x)
*
* 100*log10(x)
* = 100*(log2(x)/log2(10)))
* = (100*(2^15)*log2(x))/((2^15)*log2(10))
* = ((200*(2^15)*log2(x))/((2^15)*log2(10)))/2
* = ((200*(2^15)*(log2(x/y)+log2(y)))/((2^15)*log2(10)))/2
* = ((200*(2^15)*log2(x/y))+(200*(2^15)*log2(y)))/((2^15)*log2(10)))/2
*
* where y = 2^k and 1<= (x/y) < 2
*/

UINT32 Log10Times100_L( UINT32 x)
{
	static const UINT8 scale=15;
	static const UINT8 indexWidth=5;
	/*
	log2lut[n] = (1<<scale) * 200 * log2( 1.0 + ( (1.0/(1<<INDEXWIDTH)) * n ))
	0 <= n < ((1<<INDEXWIDTH)+1)
	*/

	static const UINT32 log2lut[] = {
	0, 290941,  573196,  847269,1113620, 1372674, 1624818, 
	1870412, 2109788, 2343253, 2571091, 2793569,3010931, 
	3223408, 3431216, 3634553, 3833610, 4028562, 4219576, 
	4406807, 4590402, 4770499, 4947231, 5120719, 5291081, 
	5458428, 5622864, 5784489, 5943398,  6099680, 6253421, 
	6404702,  6553600  };

	UINT8  i = 0;
	UINT32 y = 0;
	UINT32 d = 0;
	UINT32 k = 0;
	UINT32 r = 0;

	if (x==0) return (0);
	/* Scale x (normalize) */
	/* computing y in log(x/y) = log(x) - log(y) */
	if ( (x & (((UINT32)(-1))<<(scale+1)) ) == 0 )
	{
		for (k = scale; k>0 ; k--)
		{
			if (x & (((UINT32)1)<<scale)) break;
			x <<= 1;
		}
	}
	else
	{
		for (k = scale; k<31 ; k++)
		{
			if ((x & (((UINT32)(-1))<<(scale+1)))==0) break;
			x >>= 1;
		}
	}
	/*
	Now x has binary point between bit[scale] and bit[scale-1]
	and 1.0 <= x < 2.0 */

	/* correction for divison: log(x) = log(x/y)+log(y) */
	y = k * ( ( ((UINT32)1) << scale ) * 200 );

	/* remove integer part */
	x &= ((((UINT32)1) << scale)-1);
	/* get index */
	i = (UINT8) (x >> (scale -indexWidth));
	/* compute delta (x-a) */
	d = x & ((((UINT32)1) << (scale-indexWidth))-1);
	/* compute log, multiplication ( d* (.. )) must be within range ! */
	y += log2lut[i] + (( d*( log2lut[i+1]-log2lut[i] ))>>(scale-indexWidth));
	/* Conver to log10() */
	y /= 108853; /* (log2(10) << scale) */
	r = (y>>1);
	/* rounding */
	if (y&((UINT32)1)) r++;
	return (r);

}



static INT32 ali_mxl241_nim_hw_initialize(struct ali_nim_mxl241_private *priv, struct ali_nim_m3200_cfg *nim_cfg)
{
	MXL_STATUS status;
	MXL_RESET_CFG_T               mxl241sf_reset;
	MXL_OVERWRITE_DEFAULT_CFG_T   mxl241sf_overwritedefault;
	MXL_DEV_INFO_T                mxl241sf_deviceinfo;
	MXL_XTAL_CFG_T                mxl241sf_xtalsetting;
	MXL_AGC_T                     mxl241sf_agc;
	MXL_TOP_MASTER_CFG_T          mxl241sf_poweruptuner;
	MXL_MPEG_OUT_CFG_T            mxl241sf_mpegout;
	
	
	memcpy((void*)&(priv->tuner_config_data), (void*)&(nim_cfg->tuner_config_data), sizeof(struct QAM_TUNER_CONFIG_DATA));
	memcpy((void*)&(priv->tuner_config_ext), (void*)&(nim_cfg->tuner_config_ext), sizeof(struct QAM_TUNER_CONFIG_EXT));
	
	priv->i2c_type_id = nim_cfg->tuner_config_ext.i2c_type_id;

    usage_QAM_type = nim_cfg->qam_mode  & 0x01;  
	//nim_mxl241_set_qam_type(priv, nim_cfg->qam_mode);
	

	mxl241sf_reset.I2cSlaveAddr =  priv->tuner_config_ext.c_tuner_base_addr;//99;
	status = MxLWare_API_ConfigDevice(MXL_DEV_SOFT_RESET_CFG, (void*)&mxl241sf_reset);

	// 2. Overwrite default
	mxl241sf_overwritedefault.I2cSlaveAddr =priv->tuner_config_ext.c_tuner_base_addr;// 99;
	status = MxLWare_API_ConfigDevice(MXL_DEV_OVERWRITE_DEFAULT_CFG, (void*)&mxl241sf_overwritedefault);

	// 3. Read Back Device id and version
	status = MxLWare_API_GetDeviceStatus(MXL_DEV_ID_VERSION_REQ, (void*)&mxl241sf_deviceinfo);
	if (status == MXL_TRUE) 
	{
		MXL241_PRINTF("MxL241SF : DevId = 0x%x, Version = 0x%x \n\n", mxl241sf_deviceinfo.DevId, mxl241sf_deviceinfo. DevVer);
	}

	// 4. XTAL and Clock out setting
	mxl241sf_xtalsetting.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;//99;
	mxl241sf_xtalsetting.XtalEnable = MXL_ENABLE;
	mxl241sf_xtalsetting.DigXtalFreq = XTAL_24MHz;//XTAL_48MHz;
	mxl241sf_xtalsetting.XtalBiasCurrent = 1;
	mxl241sf_xtalsetting.XtalCap = 10; // 10pF
	mxl241sf_xtalsetting.XtalClkOutEnable = MXL_ENABLE;
	mxl241sf_xtalsetting.XtalClkOutGain =  0xa; 
	mxl241sf_xtalsetting.LoopThruEnable = MXL_ENABLE;//MXL_DISABLE;

	MxLWare_API_ConfigDevice(MXL_DEV_XTAL_SETTINGS_CFG, (void*)&mxl241sf_xtalsetting);

	// 5. AGC configuration
	mxl241sf_agc.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;//99;
	mxl241sf_agc.FreezeAgcGainWord = MXL_NO_FREEZE;

	MxLWare_API_ConfigTuner(MXL_TUNER_AGC_SETTINGS_CFG, (void*)&mxl241sf_agc);

	// 7. MPEG out setting
	if( priv->tuner_config_ext.c_tuner_special_config)
	{
		mxl241sf_mpegout.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;//99;
		mxl241sf_mpegout.SerialOrPar = MPEG_DATA_SERIAL;
		mxl241sf_mpegout.MpegValidPol = MPEG_ACTIVE_HIGH;
		mxl241sf_mpegout.MpegClkPol = MPEG_CLK_POSITIVE;
		mxl241sf_mpegout.MpegSyncPol = MPEG_ACTIVE_HIGH;
		mxl241sf_mpegout.MpegClkFreq = MPEG_CLK_57MHz;
		mxl241sf_mpegout.MpegClkSource = MPEG_CLK_INTERNAL;	
		mxl241sf_mpegout.LsbOrMsbFirst = MPEG_SERIAL_MSB_1ST;
		mxl241sf_mpegout.MpegSyncPulseWidth=MPEG_SYNC_WIDTH_BYTE;
	}
	else
	{
		mxl241sf_mpegout.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;//99;
		mxl241sf_mpegout.SerialOrPar = MPEG_DATA_PARALLEL;
		mxl241sf_mpegout.MpegValidPol = MPEG_ACTIVE_HIGH;
		mxl241sf_mpegout.MpegClkPol = MPEG_CLK_POSITIVE;
		mxl241sf_mpegout.MpegSyncPol = MPEG_ACTIVE_HIGH;
		mxl241sf_mpegout.MpegClkFreq = MPEG_CLK_19MHz;
		mxl241sf_mpegout.MpegClkSource = MPEG_CLK_INTERNAL;
		
	}
	MxLWare_API_ConfigDemod(MXL_DEMOD_MPEG_OUT_CFG, (void*)&mxl241sf_mpegout);
	// 6. Power Up Tuner
	mxl241sf_poweruptuner.I2cSlaveAddr =priv->tuner_config_ext.c_tuner_base_addr;// 99;
	mxl241sf_poweruptuner.TopMasterEnable = MXL_ENABLE;
	MxLWare_API_ConfigTuner(MXL_TUNER_TOP_MASTER_CFG, (void*)&mxl241sf_poweruptuner);


	msleep(1);
	return SUCCESS;
}
long ali_mxl241_nim_ioctl(struct file *file, unsigned int cmd, void *parg)
{
	struct ali_nim_device *dev = file->private_data;
	struct ali_nim_mxl241_private *priv = dev->priv;
	unsigned long arg = (unsigned long) parg;
	long ret = 0;

	switch (cmd) 
	{
		case ALI_NIM_HARDWARE_INIT_C:
		{	
			struct ali_nim_m3200_cfg nim_param;

			
			copy_from_user(&nim_param, parg, sizeof(struct ali_nim_m3200_cfg));
			ret=ali_mxl241_nim_hw_initialize(priv, &nim_param);
			break;
		}
		case ALI_NIM_CHANNEL_CHANGE:
		{
			struct NIM_CHANNEL_CHANGE nim_param;
			
			copy_from_user(&nim_param, parg, sizeof(struct NIM_CHANNEL_CHANGE));
			if(0==nim_param.fec)
			{
				return nim_mxl241_channel_change(priv, &nim_param);
			}	
			else
			{
				return nim_mxl241_channel_change(priv, &nim_param);
			}
			
		}
		case ALI_NIM_GET_LOCK_STATUS:
		{
			UINT8 lock = 0;
			
			nim_mxl241_get_lock(priv, &lock);
			ret=lock;
			break;
		}
		case ALI_NIM_READ_QPSK_BER:
		{
			UINT32 ber = 0;
			
			nim_mxl241_get_ber(priv, &ber);
			ret=ber;
			break;
		}
		case ALI_NIM_READ_RSUB:
		{
			UINT32 per = 0;
			
			nim_mxl241_get_per(priv, &per);
			ret=per;
			break;
		}
		case ALI_NIM_GET_RF_LEVEL:
		{
			UINT16 rf_level = 0;
			
			nim_mxl241_get_rf_level(priv, &rf_level);
			ret=rf_level;
			break;
		}
		case ALI_NIM_GET_CN_VALUE:
		{
			UINT16 cn_value = 0;
			
			
			nim_mxl241_get_cn_value(priv, &cn_value);
			ret=cn_value;
			break;
		}
		case ALI_NIM_SET_PERF_LEVEL:
			
			ret = SUCCESS;
			break;
		case ALI_NIM_READ_AGC:
		{
			UINT8 agc = 0;
			
			nim_mxl241_get_agc(priv, &agc);
			ret=agc;
			break;
		}
		case ALI_NIM_READ_SNR:
		{
			UINT8 snr = 0;
			
			nim_mxl241_get_snr(priv, &snr);
			ret=snr;
			break;
		}
		case ALI_NIM_READ_SYMBOL_RATE:
		{
			UINT32 sym = 0;
			
			nim_mxl241_get_symbol_rate(priv, &sym);
			ret=sym;
			break;
		}
		case ALI_NIM_READ_FREQ:
		{
			UINT32 freq = 0;
			
			nim_mxl241_get_freq(priv, &freq);
			ret=freq;
			break;
		}
		case ALI_NIM_REG_RW:
		{
			
			ret = SUCCESS;
			break;
		}
		default:
			
			ret = -ENOIOCTLCMD;
			break;
	}
	return ret;
}

RET_CODE ali_mxl241_nim_release(struct inode *inode, struct file *file)
{
	UINT8 data = 0;
	INT32 ret = SUCCESS;
	struct ali_nim_device *dev = file->private_data;
	struct ali_nim_mxl241_private *priv = dev->priv;
	MXL_PWR_MODE_CFG_T            mxl241sf_pwrmode;
	
 	mxl241sf_pwrmode.I2cSlaveAddr = priv->tuner_config_ext.c_tuner_base_addr;
  	mxl241sf_pwrmode.PowerMode = STANDBY_ON;
	

	return ret;
}

static int __devinit ali_mxl241_nim_init(void)
{
	INT32 ret = 0;
	dev_t devno;
	INT32   result = 0;
	
	ali_mxl241_nim_priv=kmalloc(sizeof(struct ali_nim_mxl241_private), GFP_KERNEL);
	if (!ali_mxl241_nim_priv)
	{
		return -ENOMEM;
	}	
	memset(ali_mxl241_nim_priv, 0, sizeof(struct ali_nim_mxl241_private));
	mutex_init(&ali_mxl241_nim_priv->i2c_mutex);
	ret=alloc_chrdev_region(&devno, 0, 1, ALI_NIM_DEVICE_NAME);
	if(ret<0)
	{
		printk("Alloc device region failed, err: %d.\n",ret);
		return ret;
	}

	cdev_init(&ali_mxl241_nim_dev.cdev, &ali_mxl241_nim_fops);
	ali_mxl241_nim_dev.cdev.owner=THIS_MODULE;
	ali_mxl241_nim_dev.cdev.ops=&ali_mxl241_nim_fops;
	ret=cdev_add(&ali_mxl241_nim_dev.cdev, devno, 1);
	if(ret)
	{
		printk("Alloc NIM device failed, err: %d.\n", ret);
		goto error1;
	}
	
	MXL241_PRINTF("register NIM device end.\n");
	ali_mxl241_nim_class = class_create(THIS_MODULE, "ali_mxl241_nim_class");
	if (IS_ERR(ali_mxl241_nim_class))
	{
		ret = PTR_ERR(ali_mxl241_nim_class);
		goto error2;
	}
	ali_mxl241_nim_dev_node = device_create(ali_mxl241_nim_class, NULL, devno, &ali_mxl241_nim_dev, 
                           "ali_mxl241_nim0");
	if (IS_ERR(ali_mxl241_nim_dev_node))
	{
		printk(KERN_ERR "device_create() failed!\n");
		ret = PTR_ERR(ali_mxl241_nim_dev_node);
		goto error3;
	}
	return ret;


error3:
	class_destroy(ali_mxl241_nim_class);
error2:
	cdev_del(&ali_mxl241_nim_dev.cdev);
error1:
	mutex_destroy(&ali_mxl241_nim_priv->i2c_mutex);
	kfree(ali_mxl241_nim_priv);
	return ret;
}

static void __exit ali_mxl241_nim_exit(void)
{
	if(ali_mxl241_nim_dev_node != NULL)
	{
		device_del(ali_mxl241_nim_dev_node);
		ali_mxl241_nim_dev_node= NULL;
	}	
	if(ali_mxl241_nim_class != NULL)
	{
		class_destroy(ali_mxl241_nim_class);
		ali_mxl241_nim_class = NULL;
	}	
	cdev_del(&ali_mxl241_nim_dev.cdev);
	mutex_destroy(&ali_mxl241_nim_priv->i2c_mutex);
	kfree(ali_mxl241_nim_priv);
}

module_init(ali_mxl241_nim_init);
module_exit(ali_mxl241_nim_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Coeus Zhang");
MODULE_DESCRIPTION("Ali mxl241 NIM driver");


