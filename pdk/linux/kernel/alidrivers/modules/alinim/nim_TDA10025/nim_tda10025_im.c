#if defined(__NIM_LINUX_PLATFORM__)
#include "porting_tda10025_linux.h"
#elif defined(__NIM_TDS_PLATFORM__)
#include "porting_tda10025_tds.h"
#endif



#include "tmNxTypes.h"
#include "tmCompId.h"
#include "tmFrontEnd.h"
#include "tmbslFrontEndTypes.h"
#include "tmFrontEnd.h"
#include "tmbslTDA10025.h"
#include "tmbslTDA10025_Cfg.h"
#include "tmbslHCDP.h"
#include "tmbslHCDP_Advanced.h"
#include "tmbslTDA10025local.h"
#include "tmbslTDA10025Instance.h"


#define MAGIC_CONFIG_20      20

	 


extern struct nim_tda10025_private 		*ali_tda10025_nim_priv[MAX_TUNER_SUPPORT_NUM];
/*----------------------------------------------*
 * 数据类型定义                                 *
 *----------------------------------------------*/
typedef enum
{
	MOD_QAM_16 = 4,  //the  (= 4)  add by michael chen 2013620 : as  up_layer send MOD_QAM_64 = 6  
	MOD_QAM_32 = 5,
	MOD_QAM_64 = 6,
	MOD_QAM_128 = 7,
	MOD_QAM_256 = 8
};

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_I2C_RW_SIZE			12
#define DEMOD_I2C_ADDRESS 		0x18


static tmErrorCode_t 	demod_tda10025_read( tmUnitSelect_t tunit, UInt32 addrsize, UInt8* paddr, 
                                            UInt32 readlen, UInt8* pdata );
static tmErrorCode_t 	demod_tda10025_write( tmUnitSelect_t tunit, UInt32 addrsize, UInt8* paddr,
                                            UInt32 writelen, UInt8* pdata );
static tmErrorCode_t 	demod_tda10025_wait( tmUnitSelect_t tunit, UInt32 tms );
static void 		    demd_tda10025_print( UInt32 level, const char* format, ... );



static UINT32	        g_curfreq=0;
static UINT32           g_nim_status[MAX_TUNER_SUPPORT_NUM]={0};




tmErrorCode_t demod_tda10025_read( tmUnitSelect_t tunit, UInt32 addrsize, UInt8* paddr, UInt32 readlen, UInt8* pdata )
{
	INT32			errcode = SUCCESS;
	UInt8			tmpdata[MAX_I2C_RW_SIZE+2] = {0 };
	INT32 			remainlen = 0;
	INT32           burstnum = 0;
	UINT16			u16addr = 0;
	INT32 			i = 0;
	INT32           j = 0;
    INT32           index= 0;
	
	
	if (( NULL == paddr )||( NULL == pdata ))
	{
		TDA10025_PRINTF("[%s] line=%d,error,back!\n", __FUNCTION__,__LINE__);
		return TM_ERR_BAD_PARAMETER;
	}

	remainlen = readlen % MAX_I2C_RW_SIZE; 
	if (remainlen)
	{
		burstnum = readlen / MAX_I2C_RW_SIZE; 
	}
	else
	{
		burstnum = (readlen / MAX_I2C_RW_SIZE) - 1;
		remainlen = MAX_I2C_RW_SIZE;
	}

	index = (INT32)tunit;

#if 0
	TDA10025_PRINTF("[%s] line=%d,index=%d,i2c_type=%d,i2c_addr=0x%x!\n", __FUNCTION__,__LINE__,
		    index,
			ali_tda10025_nim_priv[index]->ext_dem_config.i2c_type_id,
			ali_tda10025_nim_priv[index]->ext_dem_config.i2c_base_addr);
#endif

	u16addr = (paddr[1]<<8)|paddr[0];
   
	for ( i = 0 ; i < burstnum; i ++ )
	{
		tmpdata[0] = (UInt8)((u16addr>>8)&0xff);
		tmpdata[1] = (UInt8)(u16addr&0xff);

	   	NIM_MUTEX_ENTER(ali_tda10025_nim_priv[index]);
	   	
        errcode |= nim_i2c_write(ali_tda10025_nim_priv[index]->ext_dem_config.i2c_type_id, 
	    ali_tda10025_nim_priv[index]->ext_dem_config.i2c_base_addr, tmpdata, 2);     
		errcode |= nim_i2c_read(ali_tda10025_nim_priv[index]->ext_dem_config.i2c_type_id,
		ali_tda10025_nim_priv[index]->ext_dem_config.i2c_base_addr, tmpdata, MAX_I2C_RW_SIZE);
  
		NIM_MUTEX_LEAVE(ali_tda10025_nim_priv[index]);
		
        
		for ( j = 0 ; j < (MAX_I2C_RW_SIZE/2) ; j++  )
		{
			 pdata[i * MAX_I2C_RW_SIZE + j*2] = tmpdata[j*2+1];
			 pdata[i * MAX_I2C_RW_SIZE + j*2 + 1] = tmpdata[j*2];
		}
		u16addr += MAX_I2C_RW_SIZE;
	}

	tmpdata[0] = (UInt8)((u16addr>>8)&0xff);
	tmpdata[1] = (UInt8)(u16addr&0xff);

    NIM_MUTEX_ENTER(ali_tda10025_nim_priv[index]);
    
	errcode |= nim_i2c_write(ali_tda10025_nim_priv[index]->ext_dem_config.i2c_type_id, 
	                 ali_tda10025_nim_priv[index]->ext_dem_config.i2c_base_addr, tmpdata, 2);
	errcode |= nim_i2c_read(ali_tda10025_nim_priv[index]->ext_dem_config.i2c_type_id,
	                ali_tda10025_nim_priv[index]->ext_dem_config.i2c_base_addr, tmpdata, remainlen);
    
    NIM_MUTEX_LEAVE(ali_tda10025_nim_priv[index]);

	for ( i = 0 ; i < (remainlen/2) ; i++ )
	{
		pdata[burstnum * MAX_I2C_RW_SIZE + i *2] = tmpdata[i*2+1];
		pdata[burstnum * MAX_I2C_RW_SIZE + i *2 + 1] = tmpdata[i*2];

	}


	if ( errcode != SUCCESS )
	{
		TDA10025_PRINTF("[%s] line=%d,error,back,i2c_type=%d,i2c_addr=0x%x!\n", __FUNCTION__,__LINE__,
			ali_tda10025_nim_priv[index]->ext_dem_config.i2c_type_id,
			ali_tda10025_nim_priv[index]->ext_dem_config.i2c_base_addr);
		return TM_ERR_READ;
	}
	//PRINTK_INFO("[%s] line=%d,success!\n", __FUNCTION__,__LINE__);
	return TM_OK;
}


tmErrorCode_t demod_tda10025_write( tmUnitSelect_t tunit, UInt32 addrsize, UInt8* paddr, UInt32 writelen, UInt8* pdata )
{
	INT32			errcode = SUCCESS;
	UInt8			tmpdata[MAX_I2C_RW_SIZE+2] = {0};
	INT32 			remainlen = 0 ;
	INT32           burstnum = 0;
	UINT16			u16addr = 0;
	INT32 			i = 0 ;
	INT32           j = 0;
	INT32           index= 0;
	
	if (( NULL == paddr )||( NULL == pdata ))
	{
		TDA10025_PRINTF("[%s] line=%d,error,back!\n", __FUNCTION__,__LINE__);
		return TM_ERR_BAD_PARAMETER;
	}

	remainlen = writelen % MAX_I2C_RW_SIZE; 
	if (remainlen)
	{
		burstnum = writelen / MAX_I2C_RW_SIZE; 
	}
	else
	{
		burstnum = (writelen / MAX_I2C_RW_SIZE) - 1;
		remainlen = MAX_I2C_RW_SIZE;
	}

	index = (INT32)tunit;
#if 0
	TDA10025_PRINTF("[%s] line=%d,index=%d,i2c_type=%d,i2c_addr=0x%x!\n", __FUNCTION__,__LINE__,
		    index,
			ali_tda10025_nim_priv[index]->ext_dem_config.i2c_type_id,
			ali_tda10025_nim_priv[index]->ext_dem_config.i2c_base_addr);
#endif	
	u16addr = (paddr[1]<<8)|paddr[0];

	for ( i = 0 ; i < burstnum; i ++ )
	{
		for ( j = 0 ; j < MAX_I2C_RW_SIZE/2 ; j++  )
		{
			 tmpdata[j*2 + 2] = pdata[i * MAX_I2C_RW_SIZE + j*2 + 1];
			 tmpdata[j*2 + 1 + 2] = pdata[i * MAX_I2C_RW_SIZE + j*2];
		}
		tmpdata[0] = (UInt8)((u16addr>>8)&0xff);
		tmpdata[1] = (UInt8)(u16addr&0xff);
        NIM_MUTEX_ENTER(ali_tda10025_nim_priv[index]);
		
        errcode |= nim_i2c_write(ali_tda10025_nim_priv[index]->ext_dem_config.i2c_type_id,  
	              ali_tda10025_nim_priv[index]->ext_dem_config.i2c_base_addr, tmpdata,MAX_I2C_RW_SIZE+2);
		
        NIM_MUTEX_LEAVE(ali_tda10025_nim_priv[index]);
    
		u16addr += MAX_I2C_RW_SIZE;
	}

	for ( i = 0 ; i < remainlen/2 ; i++ )
	{
		tmpdata[i*2 + 2] = pdata[burstnum* MAX_I2C_RW_SIZE + i*2 + 1];
		tmpdata[i*2 + 1 + 2] = pdata[burstnum* MAX_I2C_RW_SIZE + i*2];
	}
	tmpdata[0] = (UInt8)((u16addr>>8)&0xff);
	tmpdata[1] = (UInt8)(u16addr&0xff);
    NIM_MUTEX_ENTER(ali_tda10025_nim_priv[index]);

	errcode |= nim_i2c_write(ali_tda10025_nim_priv[index]->ext_dem_config.i2c_type_id, 
	                  ali_tda10025_nim_priv[index]->ext_dem_config.i2c_base_addr, tmpdata, remainlen+2);
    
    NIM_MUTEX_LEAVE(ali_tda10025_nim_priv[index]);

	if ( errcode != SUCCESS )
	{
		TDA10025_PRINTF("[%s] line=%d,error,back,i2c_type=%d,i2c_add=0x%x!\n", __FUNCTION__,__LINE__,
			                       ali_tda10025_nim_priv[index]->ext_dem_config.i2c_type_id,
			                       ali_tda10025_nim_priv[index]->ext_dem_config.i2c_base_addr);
		return TM_ERR_WRITE;
	}

	return TM_OK;
}


tmErrorCode_t demod_tda10025_wait( tmUnitSelect_t tunit, UInt32 tms )
{
	comm_sleep(tms);

	return TM_OK;
}


void demd_tda10025_print( UInt32 level, const char* format, ... )
{
	va_list args;
	
	va_start(args,format);
//	vprintf(format, args);    //comment out firstly
	va_end(args);
}


INT32 nim_tda10025_get_lock(struct nim_device *dev, UINT8 *lock )
{
	tmbslFrontEndState_t 	lockinfo = 0;
	tmErrorCode_t		    errcode = TM_OK;
    struct nim_tda10025_private *priv = dev->priv;
	
	errcode = tmbslTDA10025_GetLockStatus(priv->dev_idx,&lockinfo);
	if ( errcode != TM_OK )
	{
		return ERR_FAILED;
	}

	if(lockinfo == tmbslFrontEndStateLocked)
	{
		*lock=1;
	}
	else
	{
		*lock=0;	 
	}
	
	return SUCCESS;
}


INT32 nim_tda10025_get_freq(struct nim_device *dev, UINT32 *freq )
{

	*freq = g_curfreq;
	return SUCCESS;
}


INT32 nim_tda10025_get_qam(struct nim_device *dev, UINT8 *qam_order )
{
	tmErrorCode_t				errcode = TM_OK;
	tmFrontEndModulation_t		emod = 0;
	struct nim_tda10025_private *priv = dev->priv;
	

	errcode =  tmbslTDA10025_GetMod(priv->dev_idx,&emod);
	if( TM_OK != errcode )
	{
		return ERR_DEV_ERROR;
	}

	switch ( emod )
	{
		case tmFrontEndModulationAuto:
		case tmFrontEndModulationBpsk:
		case tmFrontEndModulationQpsk:
		case tmFrontEndModulationQam4:
		case tmFrontEndModulationPsk8:
		case tmFrontEndModulationQam512:
		case tmFrontEndModulationQam1024:
		case tmFrontEndModulation8VSB:
		case tmFrontEndModulation16VSB:
		case tmFrontEndModulationQam:
			errcode = !TM_OK;
			break;
			
		case tmFrontEndModulationQam16:
			*qam_order =MOD_QAM_16;
			break;
			
		case tmFrontEndModulationQam32:
			*qam_order =MOD_QAM_32;
			break;
			
		case tmFrontEndModulationQam64:
			*qam_order =MOD_QAM_64;
			break;
			
		case tmFrontEndModulationQam128:
			*qam_order =MOD_QAM_128;
			break;
			
		case tmFrontEndModulationQam256:
			*qam_order =MOD_QAM_256;
			break;
			
		default:
			errcode = !TM_OK;
			break;
	}

	if(errcode != TM_OK)
	{
		return ERR_DEV_ERROR;
	}
	return SUCCESS;
}


INT32 nim_tda10025_get_snr(struct nim_device *dev, UINT8 *snr )
{
	tmErrorCode_t		errcode = TM_OK;
	UInt32			    quality = 0;
	struct nim_tda10025_private *priv = dev->priv;
	

	errcode = tmbslTDA10025_GetSignalQuality(priv->dev_idx,&quality);
	if ( errcode != TM_OK )
	{
		return ERR_DEV_ERROR;
	}

	*snr = (UINT8)quality;
	
	return SUCCESS;
}

INT32 nim_tda10025_get_symbolrate(struct nim_device *dev, UINT32 *sym_rate )
{
	tmErrorCode_t		errcode = TM_OK;
	UINT32			    u32sym = 0;
	struct nim_tda10025_private *priv = dev->priv;
    
	errcode = tmbslTDA10025_GetSR(priv->dev_idx,&u32sym);
	if ( TM_OK != errcode )
	{
		return ERR_DEV_ERROR;
	}
	*sym_rate = u32sym/1000;
	return SUCCESS;
}


INT32 nim_tda10025_get_ber(struct nim_device *dev, UINT32 *err_count )
{
	tmErrorCode_t		    errcode = TM_OK;
	tmFrontEndFracNb32_t 	stfrac;
	UInt32 			        uncors = 0;
	struct nim_tda10025_private *priv = dev->priv;
	

	errcode = tmbslTDA10025_GetBER(priv->dev_idx,&stfrac,&uncors);
	if ( errcode != TM_OK )
	{
		return ERR_DEV_ERROR;
	}

	*err_count = uncors;
	
	return SUCCESS;
}


INT32 nim_tda10025_get_agc(struct nim_device *dev, UINT8 *agc )
{
	tmErrorCode_t	errcode = TM_OK;
	UInt16		    signallevel=0;
    struct nim_tda10025_private *priv = dev->priv;
	
	errcode = tmbslHCDP_GetSignalLevel(priv->dev_idx,&signallevel);
	if( TM_OK != errcode )
	{
		return ERR_DEV_ERROR;
	}

	*agc = (UINT8)signallevel;
	return SUCCESS;
}


INT32 nim_tda10025_channel_change(struct nim_device *dev, NIM_CHANNEL_CHANGE_T *pstChl_Change )
{
	tmErrorCode_t			errcode = TM_OK;
	tmbslFrontEndState_t	efrontendlock = tmbslFrontEndStateUnknown;
	UInt32					loopcount = 0;
	tmFrontEndModulation_t	emod =tmFrontEndModulationAuto;
	struct nim_tda10025_private *priv = (struct nim_tda10025_private *)(dev->priv);
	UInt32 					uif = 0;
    
	if(NULL == priv)
	{
		return ERR_FAILED;
	}	
	TDA10025_PRINTF("[%s] line =%d, enter,index=%d!\n ", __FUNCTION__, __LINE__,priv->dev_idx);
	
	uif = priv->tuner_config_ext.w_tuner_if_freq*1000;
	TDA10025_PRINTF("[%s] line=%d,freq: %d,modulation=%d,sym=%d\n", __FUNCTION__,__LINE__,
	               pstChl_Change->freq,pstChl_Change->modulation,pstChl_Change->sym);
	g_curfreq = pstChl_Change->freq;

 	pstChl_Change->freq = (pstChl_Change->freq)*10;
      
	if (priv->nim_tuner_control != NULL )
	{
		if (priv->nim_tuner_control(priv->tuner_id,pstChl_Change->freq/*514000*/,pstChl_Change->sym/*6875*/,0,0) != SUCCESS)
		{
			TDA10025_PRINTF("[%s] line=%d,nim_tuner_control error,back!\n", __FUNCTION__,__LINE__);
			return ERR_FAILED;
		}
	}
	
	/* Set channel FEC mode with IF from Tuner */
    if (NIM_DVBC_J83B_MODE == (priv->qam_mode & 0x01) )
    {
		errcode = tmbslTDA10025_SetFECMode(priv->dev_idx, tmFrontEndFECModeAnnexB);  
    }
	else
	{
		errcode = tmbslTDA10025_SetFECMode(priv->dev_idx, tmFrontEndFECModeAnnexA); 
	}
	
	if(TM_OK != errcode)
	{
		TDA10025_PRINTF("[%s] line=%d,tmbslTDA10025_SetFECMode error,back!\n", __FUNCTION__,__LINE__);
		return ERR_FAILUE;
	}	
	/* Set channel decoder IF with IF from Tuner */
	errcode = tmbslTDA10025_SetIF(priv->dev_idx, uif);
	if(TM_OK != errcode)
	{
		TDA10025_PRINTF("[%s] line=%d,tmbslTDA10025_SetIF error,back!\n", __FUNCTION__,__LINE__);
		return ERR_FAILUE;
	}	
	/* Set channel decoder Spectrum Inversion */
	errcode = tmbslTDA10025_SetSI(priv->dev_idx, tmFrontEndSpecInvAuto);
	if(TM_OK != errcode)
	{
		TDA10025_PRINTF("[%s] line=%d,tmbslTDA10025_SetSI error,back!\n", __FUNCTION__,__LINE__);
		return ERR_FAILUE;
	}	
	/* Set channel decoder Modulation */
	switch ( pstChl_Change->modulation )
	{
		case MOD_QAM_16:
			emod = tmFrontEndModulationQam16;
			break;
		
		case MOD_QAM_32:
			emod = tmFrontEndModulationQam32;
			break;
		
		case MOD_QAM_64:
			emod = tmFrontEndModulationQam64;
			break;
		
		case MOD_QAM_128:
			emod = tmFrontEndModulationQam128;
			break;
		
		case MOD_QAM_256:
			emod = tmFrontEndModulationQam256;
			break;
		
		default:
			emod = tmFrontEndModulationQam64;
			break;
	}
	
	errcode = tmbslTDA10025_SetMod(priv->dev_idx, emod/*tmFrontEndModulationQam64*/);
	if(TM_OK != errcode)
	{
		TDA10025_PRINTF("[%s] line=%d,tmbslTDA10025_SetMod error,back!\n", __FUNCTION__,__LINE__);
		return ERR_FAILUE;
	}	
	/* Set channel decoder Symbol Rate */
	errcode = tmbslTDA10025_SetSR(priv->dev_idx, pstChl_Change->sym*1000/*6875000*/);
	if(TM_OK != errcode)
	{
		TDA10025_PRINTF("[%s] line=%d,tmbslTDA10025_SetSR error,back!\n", __FUNCTION__,__LINE__);
		return ERR_FAILUE;
	}	
	/* Start Lock acquisition */
	errcode = tmbslTDA10025_StartLock(priv->dev_idx);
	if(TM_OK != errcode)
	{
		TDA10025_PRINTF("[%s] line=%d,tmbslTDA10025_StartLock error,back!\n", __FUNCTION__,__LINE__);
		return ERR_FAILUE;
	}	
	/* Wait for channel decoder lock procedure to complete or timeout after 5s */
	loopcount = 0;
	do
	{
		errcode = tmbslTDA10025_GetLockStatus(priv->dev_idx, &efrontendlock);
		
       // TDA10025_PRINTF("[%s] line=%d,loopcount=%d!\n", __FUNCTION__,__LINE__,loopcount);
		
        comm_sleep(100);
        
		loopcount++;
	} while ((loopcount <MAGIC_CONFIG_20) && (efrontendlock != tmbslFrontEndStateLocked));
   // MAGIC_CONFIG_20
    if(efrontendlock == tmbslFrontEndStateLocked)
    {
    	TDA10025_PRINTF("[%s] line=%d,success,loopcount=%d!\n", __FUNCTION__,__LINE__,loopcount);
    	return SUCCESS;
    }
    else
    {
    	TDA10025_PRINTF("[%s] line=%d,fail,loop=%d,lock=%d!\n", __FUNCTION__,__LINE__,loopcount,efrontendlock);
    	return ERR_FAILED;
    }
}



INT32 nim_tda10025_enter_standby(struct nim_device *dev )
{
	tmErrorCode_t				errcode = TM_OK;
	struct nim_tda10025_private *priv = (struct nim_tda10025_private *)(dev->priv);
	
	errcode = tmbslTDA10025_SetPowerState(priv->dev_idx,tmPowerStandby);
	if ( TM_OK != errcode )
	{
		TDA10025_PRINTF("Set TDA10025 Standby Failed!\n");
		return ERR_DEV_ERROR;
	}
	if (priv->nim_tuner_close != NULL )
	{
		if (priv->nim_tuner_close(priv->tuner_id) != SUCCESS)
		{
			return ERR_FAILED;
		}
	}
	
	return SUCCESS;
}


INT32 nim_tda10025_dev_init(struct nim_device *dev)
{
	struct nim_tda10025_private *priv = (struct nim_tda10025_private *)(dev->priv);
	tmErrorCode_t errcode                                = TM_OK;
	tmbslFrontEndDependency_t stSrvFunc;
	TDA10025_AdditionnalData_t stTDA10025AdditionnalData ;
 	
	
    if(!g_nim_status[priv->dev_idx])
    {
		TDA10025_PRINTF("[%s] line =%d, enter,index=%d!\n ", __FUNCTION__, __LINE__,priv->dev_idx);
		stSrvFunc.sIo.Write 			= demod_tda10025_write;
		stSrvFunc.sIo.Read 				= demod_tda10025_read;
		stSrvFunc.sTime.Get 			= NULL;
		stSrvFunc.sTime.Wait 			= demod_tda10025_wait;
		stSrvFunc.sDebug.Print 			= demod_tda10025_wait;
		stSrvFunc.sMutex.Init 			= NULL;
		stSrvFunc.sMutex.DeInit 		= NULL;
		stSrvFunc.sMutex.Acquire 		= NULL;
		stSrvFunc.sMutex.Release 		= NULL;
		stSrvFunc.dwAdditionalDataSize 	= sizeof(TDA10025_AdditionnalData_t);
		stSrvFunc.pAdditionalData 		= (void*)&stTDA10025AdditionnalData;
		
		stTDA10025AdditionnalData.tUnitCommon 	= priv->dev_idx;
		stTDA10025AdditionnalData.eChannelSel 	=TDA10025_ChannelSelectionIpA;

		errcode = tmbslTDA10025_Open(priv->dev_idx, &stSrvFunc);
		if(TM_OK != errcode)
		{
			TDA10025_PRINTF("[%s] line =%d, errcode =%d \n ", __FUNCTION__, __LINE__, errcode);
			return ERR_FAILUE;
		}
	     
		errcode = tmbslTDA10025_Reset(priv->dev_idx);
	        if(TM_OK != errcode)
		{
			TDA10025_PRINTF("[%s] line =%d, errcode =%d \n ", __FUNCTION__, __LINE__, errcode);
			return ERR_FAILUE;
	         }	
	    
		#if 0
		// TDA10025 Hardware initialization
		errcode = tmbslTDA10025_HwInit(0);
		if(errcode != TM_OK)
		{
			return ERR_FAILUE;
	        } 
		#endif

	       
		// Set TDA10025 power state to On 
		errcode = tmbslTDA10025_SetPowerState(priv->dev_idx, tmPowerOn);
		if(TM_OK != errcode)
		{
			return ERR_FAILUE;
	    }
		g_nim_status[priv->dev_idx]=1;
    }
	
	return RET_SUCCESS;
}

INT32 nim_tda10025_dev_close(struct nim_device *dev)
{
#if 0	
	struct nim_tda10025_private *priv = (struct nim_tda10025_private *)(dev->priv);

	TDA10025_PRINTF("[%s] line =%d, enter,index=%d!\n ", __FUNCTION__, __LINE__,priv->dev_idx);
	
    g_nim_status[priv->dev_idx]  =0;
    return tmbslTDA10025_Close(priv->dev_idx);
#else
    return 0;
#endif
}

