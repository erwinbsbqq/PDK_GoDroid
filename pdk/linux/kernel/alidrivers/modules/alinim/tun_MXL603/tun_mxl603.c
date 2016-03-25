/*****************************************************************************************
 *
 * FILE NAME          : MxL603_OEM_Drv.c
 * 
 * AUTHOR             : Mahendra Kondur
 *
 * DATE CREATED       : 12/23/2011  
 *
 * DESCRIPTION        : This file contains I2C driver and Sleep functins that 
 *                      OEM should implement for MxL603 APIs
 *                             
 *****************************************************************************************
 *                Copyright (c) 2011, MaxLinear, Inc.
 ****************************************************************************************/
#if defined(__NIM_LINUX_PLATFORM__)
#include "tun_mxl603_linux.h"
#elif defined(__NIM_TDS_PLATFORM__)
#include "tun_mxl603_tds.h"
#endif


#include "MxL603_TunerApi.h"
#include "MxL603_TunerCfg.h"


#define MAX_TUNER_SUPPORT_NUM 		2
#define NIM_TUNER_SET_STANDBY_CMD	0xffffffff

#define MAGIC_CONST_11            11
#define MAGIC_CONST_6            6
#define MAGIC_CONST_255         255


#if  ((SYS_TUN_MODULE == MXL603) || (SYS_TUN_MODULE == ANY_TUNER))


#if 0
#define MXL603_PRINTF  nim_print
#else
#define MXL603_PRINTF(...) 
#endif


//kent,2013.6.17
typedef struct _mxl603_config_t
{
	UINT16  				c_tuner_crystal;
	UINT8  					c_tuner_base_addr;		/* Tuner BaseAddress for Write Operation: (BaseAddress + 1) for Read */	
	UINT16 					w_tuner_if_freq;
	UINT32 					i2c_type_id;

    MXL603_SIGNAL_MODE_E 	signalMode;
    MXL_BOOL 				singleSupply_3_3V;
    UINT8 					xtalCap;
    MXL_BOOL 				clkOutEnable;           //enable or disable clock out.
	
}MXL603_CONFIG;


static MXL603_CONFIG MxL603_Config[MAX_TUNER_SUPPORT_NUM]={0};
static DEM_WRITE_READ_TUNER m_ThroughMode[MAX_TUNER_SUPPORT_NUM]={0};
static BOOL bMxl_Tuner_Inited[2];
static UINT32 tuner_cnt = 0;

static MxL_ERR_MSG MxL603_Tuner_RFTune(UINT32 tuner_id, UINT32 rf_freq_hz, MXL603_BW_E bwhz);
static MxL_ERR_MSG MxL603_Tuner_Init(UINT32 tuner_id,UINT32 freq);
static BOOL run_on_through_mode(UINT32 tuner_id);
static INT32 tun_mxl603_powcontrol(UINT32 tuner_id, UINT8 stdby);

static INT32 tun_mxl603_init_common(UINT32 *tuner_id,UINT8 tunerid_input);  //, MXL603_CONFIG* ptrtuner_config

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_OEM_WriteRegister
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 7/30/2009
--|
--| DESCRIPTION   : This function does I2C write operation.
--|
--| RETURN VALUE  : True or False
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_OEM_WriteRegister(UINT32 tuner_id, UINT8 regaddr, UINT8 regdata)
{
    UINT32 status = ERR_FAILUE;
    UINT8 cmd[2] ={ 0 };

    cmd[0] = regaddr;
    cmd[1] = regdata;

    if (run_on_through_mode(tuner_id))
    {
        status = m_ThroughMode[tuner_id].dem_write_read_tuner(m_ThroughMode[tuner_id].nim_dev_priv, 
	                                                      MxL603_Config[tuner_id].c_tuner_base_addr, 
							                              cmd, 2, 0, 0);
    }
    else
    {
	//	MXL603_PRINTF("[%s]line=%d,i2c_type=%d,i2c_addr=0x%x!\n",__FUNCTION__,__LINE__,
	//		             MxL603_Config[tuner_id].i2c_type_id,
	//		             MxL603_Config[tuner_id].c_tuner_base_addr);
		
        status = nim_i2c_write(MxL603_Config[tuner_id].i2c_type_id, MxL603_Config[tuner_id].c_tuner_base_addr, &cmd, 2);
    }
  
    return (status==SUCCESS ? MXL_TRUE : MXL_FALSE);
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_OEM_ReadRegister
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

MXL_STATUS MxLWare603_OEM_ReadRegister(UINT32 tuner_id, UINT8 regaddr, UINT8 *dataptr)
{
    UINT32 status = ERR_FAILUE;
    UINT8 read_cmd[2] = {0};

    if(NULL == dataptr)
    {
		return MXL_FALSE;
    }
	
	/* read step 1. accroding to mxl301 driver API user guide. */
    read_cmd[0] = 0xFB;
    read_cmd[1] = regaddr;
    if (run_on_through_mode(tuner_id))
    {
        status = m_ThroughMode[tuner_id].dem_write_read_tuner(m_ThroughMode[tuner_id].nim_dev_priv, 
	                                                      MxL603_Config[tuner_id].c_tuner_base_addr, 
							      read_cmd, 2, 0, 0);
        if(status != SUCCESS)
		{
	            return MXL_FALSE;
		}
        status = m_ThroughMode[tuner_id].dem_write_read_tuner( m_ThroughMode[tuner_id].nim_dev_priv, 
	                                                       MxL603_Config[tuner_id].c_tuner_base_addr, 
							       0, 0, dataptr, 1);
        if(status != SUCCESS)
		{
	            return MXL_FALSE;
		}
    }
    else
    {
    	status = nim_i2c_write(MxL603_Config[tuner_id].i2c_type_id, 
	                       MxL603_Config[tuner_id].c_tuner_base_addr, 
	                       read_cmd, 2);
        if(status != SUCCESS)
		{
	            return MXL_FALSE;
		}
    	status = nim_i2c_read(MxL603_Config[tuner_id].i2c_type_id, 
	                      MxL603_Config[tuner_id].c_tuner_base_addr, 
	                      dataptr, 1);
        if(status != SUCCESS)
	    {
            return MXL_FALSE;
        }
     }
    
     return MXL_TRUE;
  
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_OEM_Sleep
--| 
--| AUTHOR        : Dong Liu
--|
--| DATE CREATED  : 01/10/2010
--|
--| DESCRIPTION   : This function complete sleep operation. WaitTime is in ms unit
--|
--| RETURN VALUE  : None
--|
--|-------------------------------------------------------------------------------------*/
void MxLWare603_OEM_Sleep(UINT16 delaytimeinms)
{
    comm_sleep(delaytimeinms);
}

static BOOL run_on_through_mode(UINT32 tuner_id)
{
    return (m_ThroughMode[tuner_id].nim_dev_priv && m_ThroughMode[tuner_id].dem_write_read_tuner);
}

static INT32 set_through_mode(UINT32 tuner_id, DEM_WRITE_READ_TUNER *ThroughMode)
{
    if(tuner_id >= tuner_cnt)
    {
	    return ERR_FAILUE;
    } 
    comm_memcpy((UINT8*)(&m_ThroughMode[tuner_id]), (UINT8*)ThroughMode, sizeof(DEM_WRITE_READ_TUNER));
    return SUCCESS;
}



/*****************************************************************************
* INT32 tun_mxl603_init(struct COFDM_TUNER_CONFIG_EXT * ptrtuner_config)
*
* Tuner mxl603 Initialization
*
* Arguments:
*  Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrtuner_config		: pointer for Tuner configuration structure 
*
* Return Value: INT32			: Result
*****************************************************************************/
INT32 tun_mxl603_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrtuner_config)
{
    if (NULL == ptrtuner_config)
    {
	  	MXL603_PRINTF("[%s]line=%d,error,back!\n",__FUNCTION__,__LINE__);
		return ERR_FAILUE;
    }	
	
    if(tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
    {
		MXL603_PRINTF("[%s]line=%d,error,back!\n",__FUNCTION__,__LINE__);
		return ERR_FAILUE;
    }	

    MxL603_Config[tuner_cnt].signalMode = MXL603_DIG_DVB_T_DTMB;
    MxL603_Config[tuner_cnt].singleSupply_3_3V = MXL_ENABLE;
    MxL603_Config[tuner_cnt].xtalCap = 20;
    MxL603_Config[tuner_cnt].clkOutEnable = MXL_DISABLE;

	
    MxL603_Config[tuner_cnt].c_tuner_base_addr=(UINT8)(ptrtuner_config->c_tuner_base_addr);
    MxL603_Config[tuner_cnt].c_tuner_crystal=(UINT16)ptrtuner_config->c_tuner_crystal;
    MxL603_Config[tuner_cnt].i2c_type_id=(UINT32)ptrtuner_config->i2c_type_id;
    MxL603_Config[tuner_cnt].w_tuner_if_freq=(UINT16)ptrtuner_config->w_tuner_if_freq;

    return tun_mxl603_init_common(tuner_id,0);  //, ptrtuner_config
}

INT32 tun_mxl603_init_DVBC(UINT32 *tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrtuner_config)
{
    UINT32 curr_tuner = 0;

    if (NULL == ptrtuner_config)
    {
	  	MXL603_PRINTF("[%s]line=%d,error,back!\n",__FUNCTION__,__LINE__);
		return ERR_FAILUE;
    }	

	
    MXL603_PRINTF("[%s]line=%d,enter,*tuner_id=%d,reopen=%d!\n",__FUNCTION__,__LINE__,*tuner_id,
                                                                ptrtuner_config->c_tuner_reopen);
	
    if(ptrtuner_config->c_tuner_reopen & 0xff)
    {
     	curr_tuner=*tuner_id;
    }
    else	
    {
    	curr_tuner=tuner_cnt;
    }
	
    if (NULL == ptrtuner_config)
    {
	    MXL603_PRINTF("[%s]line=%d,error,back!\n",__FUNCTION__,__LINE__);
	    return ERR_FAILUE;
    }	

    if(curr_tuner >= MAX_TUNER_SUPPORT_NUM)
    {
 	    MXL603_PRINTF("[%s]line=%d,error,back,curr_tuner=%d!\n",__FUNCTION__,__LINE__,curr_tuner);
	    return ERR_FAILUE;
    }	

    MxL603_Config[curr_tuner].signalMode = MXL603_DIG_DVB_C;
    MxL603_Config[curr_tuner].singleSupply_3_3V = MXL_ENABLE;
    MxL603_Config[curr_tuner].xtalCap = 12;
    MxL603_Config[curr_tuner].clkOutEnable = MXL_DISABLE;

		
    MxL603_Config[curr_tuner].c_tuner_base_addr=(UINT8)ptrtuner_config->c_tuner_base_addr;
    MxL603_Config[curr_tuner].c_tuner_crystal=ptrtuner_config->c_tuner_crystal;
		
    MxL603_Config[curr_tuner].i2c_type_id=(UINT32)ptrtuner_config->i2c_type_id;
    MxL603_Config[curr_tuner].w_tuner_if_freq=(UINT16)ptrtuner_config->w_tuner_if_freq;

    MXL603_PRINTF("[%s]line=%d,curr_tuner=%d,success!\n",__FUNCTION__,__LINE__,curr_tuner);
    return tun_mxl603_init_common(tuner_id,ptrtuner_config->c_tuner_reopen & 0xff);  //, ptrtuner_config
}


INT32 tun_mxl603_init_ISDBT(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrtuner_config)   //ISDB-T mode
{
    if (NULL == ptrtuner_config )
    {
    	return ERR_FAILUE;
    }

    if(tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
    {
    	return ERR_FAILUE;
    }

    MxL603_Config[tuner_cnt].signalMode = MXL603_DIG_ISDBT_ATSC;
    MxL603_Config[tuner_cnt].singleSupply_3_3V = MXL_ENABLE;
    MxL603_Config[tuner_cnt].xtalCap = 20;
    MxL603_Config[tuner_cnt].clkOutEnable = MXL_DISABLE;

    MxL603_Config[tuner_cnt].c_tuner_base_addr=(UINT8)ptrtuner_config->c_tuner_base_addr;
    MxL603_Config[tuner_cnt].c_tuner_crystal=(UINT16)ptrtuner_config->c_tuner_crystal;
    MxL603_Config[tuner_cnt].i2c_type_id=(UINT32)ptrtuner_config->i2c_type_id;
    MxL603_Config[tuner_cnt].w_tuner_if_freq=(UINT16)ptrtuner_config->w_tuner_if_freq;
	
    return tun_mxl603_init_common(tuner_id,0);  //, ptrtuner_config
}

INT32 tun_mxl603_init_CXD2834(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrtuner_config) 
{
    if (NULL == ptrtuner_config)
    {
     	return ERR_FAILUE;
    }

    if(tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
    {
    	return ERR_FAILUE;
    }

    MxL603_Config[tuner_cnt].signalMode = MXL603_DIG_DVB_T_DTMB;
    MxL603_Config[tuner_cnt].singleSupply_3_3V = MXL_DISABLE;
    MxL603_Config[tuner_cnt].xtalCap = 24;
    MxL603_Config[tuner_cnt].clkOutEnable = MXL_DISABLE;

    MxL603_Config[tuner_cnt].c_tuner_base_addr=(UINT8)ptrtuner_config->c_tuner_base_addr;
    MxL603_Config[tuner_cnt].c_tuner_crystal=(UINT16)ptrtuner_config->c_tuner_crystal;
    MxL603_Config[tuner_cnt].i2c_type_id=(UINT32)ptrtuner_config->i2c_type_id;
    MxL603_Config[tuner_cnt].w_tuner_if_freq=(UINT16)ptrtuner_config->w_tuner_if_freq;


    return tun_mxl603_init_common(tuner_id,0); //, ptrtuner_config
}

INT32 tun_mxl603_init_CDT_MN88472(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrtuner_config)  
{
    if (NULL == ptrtuner_config)
    {
    	return ERR_FAILUE;
    }

    if(tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
    {
        return ERR_FAILUE;
     }

    MxL603_Config[tuner_cnt].signalMode = MXL603_DIG_DVB_T_DTMB;
    MxL603_Config[tuner_cnt].singleSupply_3_3V = MXL_DISABLE;
    MxL603_Config[tuner_cnt].xtalCap = 24; //28
    MxL603_Config[tuner_cnt].clkOutEnable = MXL_ENABLE;

    MxL603_Config[tuner_cnt].c_tuner_base_addr=(UINT8)ptrtuner_config->c_tuner_base_addr;
    MxL603_Config[tuner_cnt].c_tuner_crystal=(UINT16)ptrtuner_config->c_tuner_crystal;
    MxL603_Config[tuner_cnt].i2c_type_id=(UINT32)ptrtuner_config->i2c_type_id;
    MxL603_Config[tuner_cnt].w_tuner_if_freq=(UINT16)ptrtuner_config->w_tuner_if_freq;

    return tun_mxl603_init_common(tuner_id,0);  //, ptrtuner_config
	
}
static INT32 tun_mxl603_init_common(UINT32 *tuner_id,UINT8 tunerid_input)   //, ptrtuner_config
{
    UINT16 xtal_freq = 0;
    UINT32 curr_tuner = 0;
	
    if(tunerid_input)
    {
    	curr_tuner=*tuner_id;
    } 
    else	
    {
    	curr_tuner=tuner_cnt;
    }
	
    if(curr_tuner >= MAX_TUNER_SUPPORT_NUM)
    {
		MXL603_PRINTF("[%s]line=%d,error,back!\n",__FUNCTION__,__LINE__);
		return ERR_FAILUE;
    }	

     m_ThroughMode[curr_tuner].dem_write_read_tuner = NULL;
     xtal_freq = MxL603_Config[curr_tuner].c_tuner_crystal;
     if(xtal_freq >= 1000) //If it's in Khz, then trans it into Mhz.
     {
     	xtal_freq /= 1000;
     }

    switch (xtal_freq)
    {
        case 16:
            MxL603_Config[curr_tuner].c_tuner_crystal = (MXL603_XTAL_FREQ_E)MXL603_XTAL_16MHz;
            break;
        case 24:
            MxL603_Config[curr_tuner].c_tuner_crystal = (MXL603_XTAL_FREQ_E)MXL603_XTAL_24MHz;
            break;
	    default:
     	    MXL603_PRINTF("[%s]line=%d,error,back!\n",__FUNCTION__,__LINE__);
	    return ERR_FAILUE;
	}
    
    switch (MxL603_Config[curr_tuner].w_tuner_if_freq)
    {
        case 5000:
            MxL603_Config[curr_tuner].w_tuner_if_freq = MXL603_IF_5MHz;
            break;
        case 4570:
            MxL603_Config[curr_tuner].w_tuner_if_freq = MXL603_IF_4_57MHz;
            break;
	    case 5380:
	        MxL603_Config[curr_tuner].w_tuner_if_freq = MXL603_IF_5_38MHz;
	        break;
	    default:
	        MXL603_PRINTF("[%s]line=%d,error,back!\n",__FUNCTION__,__LINE__);
	        return ERR_FAILUE;
	}

	bMxl_Tuner_Inited[curr_tuner] = FALSE;

	if(!tunerid_input)
	{
	    if(tuner_id)
        {
	        *tuner_id = tuner_cnt;
	    }
	    MXL603_PRINTF("[%s]line=%d,here,tuner_cnt=%d!\n",__FUNCTION__,__LINE__,tuner_cnt);
	    tuner_cnt++;
	}
	
	return SUCCESS;
}

/*****************************************************************************
* INT32 tun_mxl603_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock		: Phase lock status
*
* Return Value: INT32			: Result
*****************************************************************************/

INT32 tun_mxl603_status(UINT32 tuner_id, UINT8 *lock)
{
    MXL_STATUS status = 0; 
    MXL_BOOL refLockPtr = MXL_UNLOCKED;
    MXL_BOOL rfLockPtr = MXL_UNLOCKED;		

    if(tuner_id >= tuner_cnt)
    {
		return ERR_FAILUE;
    }
	
    *lock = FALSE;   
    status = MxLWare603_API_ReqTunerLockStatus(tuner_id, &rfLockPtr, &refLockPtr);
    if (status == MXL_TRUE)
    {
        if ((MXL_LOCKED == rfLockPtr) && (MXL_LOCKED == refLockPtr))
		{
	        *lock  = TRUE;
		}
    }

    return SUCCESS;
}

INT32 tun_mxl603_get_rf_level(UINT32 tuner_id, UINT16 *rf_level)// return level in dbuV.
{
	INT32 result = 0;
	MXL_STATUS status = 0; 
	UINT8 devId = 0;
	INT32 rf_info = 0;		

	devId = (UINT8)(tuner_id & 0xff);
	
	// Read back Tuner RF level.
	status = MxLWare603_API_ReqTunerRxPower(devId, &rf_info);
	if (status == MXL_TRUE)
	{
		MXL603_PRINTF("mxl603 get RF level: %d dbm !\n", rf_info);

		rf_info = rf_info/100 + 107;

		if (rf_info > MAGIC_CONST_255)
		{
			*rf_level = MAGIC_CONST_255;
		}
		else if (rf_info < 0)
		{
			*rf_level = 0;
		}
		else
		{
			*rf_level = rf_info;
		}
		
		MXL603_PRINTF("mxl603 get RF level: %d dbuV !\n", *rf_level);
			
	}
	else
	{
		*rf_level = 0;
		return ERR_FAILUE;
	}
	

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_mxl603_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
*
* Tuner write operation
*
* Arguments:
*  Parameter1: UINT32 freq		: Synthesiser programmable divider
*  Parameter2: UINT8 bandwidth		: channel bandwidth
*  Parameter3: UINT8 AGC_Time_Const	: AGC time constant
*  Parameter4: UINT8 *data		: 
*
* Return Value: INT32			: Result
*****************************************************************************/
INT32 tun_mxl603_control(UINT32 tuner_id, UINT32 freq, UINT32 bandwidth, 
                         UINT8 agc_time_const,UINT8 *data,UINT8 _i2c_cmd)	
{
	MXL603_BW_E bw = 0;
	MxL_ERR_MSG Status = 0;
	
	if(tuner_id >= tuner_cnt)
	{
		MXL603_PRINTF("Error! tun_mxl603_control,tuner_id=%d,tuner_cnt=%d\n",tuner_id,tuner_cnt);   
		return ERR_FAILUE;
	}	

	//kent,2013-8-7
	if( freq == NIM_TUNER_SET_STANDBY_CMD )
	{
		if(0 == bandwidth )
		{
			MXL603_PRINTF("[%s]line=%d,standby!!\n",__FUNCTION__,__LINE__);
			MxLWare603_API_CfgTunerLoopThrough(tuner_id, MXL_DISABLE);//MXL_DISABLE,MXL_ENABLE
			
			tun_mxl603_powcontrol(tuner_id, 1);
		}	
		else if(1 == bandwidth)
		{
			MXL603_PRINTF("[%s]line=%d,wakeup!!\n",__FUNCTION__,__LINE__);
			tun_mxl603_powcontrol(tuner_id, 0);
			MxLWare603_API_CfgTunerLoopThrough(tuner_id, MXL_ENABLE);//MXL_DISABLE,MXL_ENABLE			
		}
		else
		{
			
			return ERR_FAILED;
		}
		MXL603_PRINTF("[%s]line=%d,standby,return!!\n",__FUNCTION__,__LINE__);
		return SUCCESS;
	}

	switch(bandwidth)
	{
	    case 6:
               bw = MXL603_TERR_BW_6MHz;
               break;
	    case 7:
               bw = MXL603_TERR_BW_7MHz;
               break;
	    case 8:
               bw = MXL603_TERR_BW_8MHz;
               break;
	    default:
	       MXL603_PRINTF("bandwidth=%d\n",bandwidth);   
	       break;
	}
 
	if( bMxl_Tuner_Inited[tuner_id] != TRUE )
	{
    	if( MxL603_Tuner_Init(tuner_id,freq) != MxL_OK )
    	{
		    MXL603_PRINTF("Error! MxL603_Tuner_Init\n");   
     		return ERR_FAILUE;
    	}
		bMxl_Tuner_Inited[tuner_id] = TRUE;
	}

	if(  (Status = MxL603_Tuner_RFTune(tuner_id, freq*1000, bw)) != MxL_OK )
	{
		MXL603_PRINTF("Error! MxL603_Tuner_RFTune\n");   
		return ERR_FAILUE;
	}

	return SUCCESS;	
}


INT32 tun_mxl603_control_DVBC(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 agc_time_const, UINT8 _i2c_cmd)
{
	MXL603_PRINTF("[%s]line=%d,enter,tuner_id=%d!\n",__FUNCTION__,__LINE__,tuner_id);
	return tun_mxl603_control(tuner_id, freq, sym,0,0,0);
}

INT32 tun_mxl603_control_DVBC_X(UINT32 tuner_id, UINT32 freq, UINT32 sym)
{
	MXL603_PRINTF("[%s]line=%d,enter,tuner_id=%d!\n",__FUNCTION__,__LINE__,tuner_id);
	return tun_mxl603_control(tuner_id,freq,sym,0,0,0);
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//																		   //
//							Tuner Functions								   //
//																		   //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
static MxL_ERR_MSG MxL603_Tuner_Init(UINT32 tuner_id, UINT32 freq)
{	
    MXL_STATUS status = 0; 
    MXL_BOOL singlesupply_3_3v = MXL_DISABLE;
    MXL603_XTAL_SET_CFG_T xtalcfg;
    MXL603_IF_OUT_CFG_T ifoutcfg;
    MXL603_AGC_CFG_T agccfg;
    MXL603_TUNER_MODE_CFG_T tunermodecfg;
    MXL603_CHAN_TUNE_CFG_T chantunecfg;
    MXL603_IF_FREQ_E ifoutfreq = MXL603_IF_3_65MHz;

    //step 0. Init device.
    status = MxLWare603_API_CfgDrvInit(tuner_id, NULL);  

    //Step 1 : Soft Reset MxL603
    status = MxLWare603_API_CfgDevSoftReset(tuner_id);
    if (MXL_SUCCESS != status)
    {
        MXL603_PRINTF("Error! MxLWare603_API_CfgDevSoftReset\n");    
        return status;
    }

    //Step 2 : Overwrite Default. It should be called after MxLWare603_API_CfgDevSoftReset and 
    //MxLWare603_API_CfgDevXtal.
    // singlesupply_3_3v = MXL_DISABLE;
    singlesupply_3_3v = MxL603_Config[tuner_id].singleSupply_3_3V;
    status = MxLWare603_API_CfgDevOverwriteDefaults(tuner_id, singlesupply_3_3v);
    if (MXL_SUCCESS != status)
    {
        MXL603_PRINTF("Error! MxLWare603_API_CfgDevOverwriteDefaults\n");    
        return status;
    }

    //Step 3 : XTAL Setting
    xtalcfg.xtalFreqSel = MxL603_Config[tuner_id].c_tuner_crystal;
    xtalcfg.xtalCap = MxL603_Config[tuner_id].xtalCap; //20, 12
    xtalcfg.clkOutEnable = MxL603_Config[tuner_id].clkOutEnable;
    xtalcfg.clkOutDiv = MXL_DISABLE;
    xtalcfg.clkOutExt = MXL_DISABLE;
    xtalcfg.singleSupply_3_3V = singlesupply_3_3v;
    xtalcfg.XtalSharingMode = MXL_DISABLE;
    status = MxLWare603_API_CfgDevXtal(tuner_id, xtalcfg);
    if ( MXL_SUCCESS != status)
    {
        MXL603_PRINTF("Error! MxLWare603_API_CfgDevXtal\n");    
        return status;
    }

    //Step 4 : IF Out setting
//    ifoutcfg.ifoutfreq = MXL603_IF_4_57MHz; //MXL603_IF_4_1MHz
//    ifoutcfg.ifoutfreq = MXL603_IF_5MHz;    //For match to DEM mn88472 IF: DMD_E_IF_5000KHZ.
     if(MxL603_Config[tuner_id].signalMode==MXL603_DIG_DVB_C)
      {
          ifoutcfg.gainLevel = MAGIC_CONST_6;
      }
      else
      {
          ifoutcfg.gainLevel = MAGIC_CONST_11;
      }	

     ifoutcfg.ifOutFreq = MxL603_Config[tuner_id].w_tuner_if_freq;
     ifoutcfg.ifInversion = MXL_DISABLE;
     ifoutcfg.manualFreqSet = MXL_DISABLE;
     ifoutcfg.manualIFOutFreqInKHz = 0;
     status = MxLWare603_API_CfgTunerIFOutParam(tuner_id, ifoutcfg);
     if (MXL_SUCCESS != status)
     {
         MXL603_PRINTF("Error! MxLWare603_API_CfgTunerIFOutParam\n");    
         return status;
     }

    //Step 5 : AGC Setting
     agccfg.agcType = MXL603_AGC_EXTERNAL;
     //agccfg.agcType = MXL603_AGC_SELF;
     agccfg.setPoint = 66;
     agccfg.agcPolarityInverstion = MXL_DISABLE;
     status = MxLWare603_API_CfgTunerAGC(tuner_id, agccfg);
     if (MXL_SUCCESS != status)
     {
        MXL603_PRINTF("Error! MxLWare603_API_CfgTunerAGC\n");    
        return status;
    }

    //Step 6 : Application Mode setting
    tunermodecfg.signalMode = MxL603_Config[tuner_id].signalMode;
  
     tunermodecfg.xtalFreqSel = MxL603_Config[tuner_id].c_tuner_crystal;
     tunermodecfg.ifOutFreqinKHz = MxL603_Config[tuner_id].w_tuner_if_freq;   //4100
  
    tunermodecfg.ifOutGainLevel = 11;
    status = MxLWare603_API_CfgTunerMode(tuner_id, tunermodecfg);
    if (MXL_SUCCESS != status)
    {
        MXL603_PRINTF("Error! MxLWare603_API_CfgTunerMode\n");    
        return status;
    }

    //Step 7 : Channel frequency & bandwidth setting
	if(MxL603_Config[tuner_id].signalMode==MXL603_DIG_DVB_C)
	{
		if (MxL603_Config[tuner_id].w_tuner_if_freq < MXL603_IF_5_38MHz) // 8m mode, for 6M/8M mode tmp code.
		{
			chantunecfg.bandWidth = MXL603_CABLE_BW_8MHz;
		}
		else
		{
			chantunecfg.bandWidth = MXL603_CABLE_BW_6MHz;
		}
		chantunecfg.freqInHz = (freq * 1000);

	}
	else
	{
		chantunecfg.bandWidth = MXL603_TERR_BW_8MHz;
		chantunecfg.freqInHz = 474000000;
	}
    
    chantunecfg.xtalFreqSel = MxL603_Config[tuner_id].c_tuner_crystal;
    chantunecfg.signalMode = MxL603_Config[tuner_id].signalMode;
    
    chantunecfg.startTune = MXL_START_TUNE;
    status = MxLWare603_API_CfgTunerChanTune(tuner_id, chantunecfg);
    if (MXL_SUCCESS != status)
    {
        MXL603_PRINTF("Error! MxLWare603_API_CfgTunerChanTune\n");    
        return status;
    }
    
    // Wait 15 ms 
    MxLWare603_OEM_Sleep(15);

	if(MxL603_Config[tuner_id].signalMode!=MXL603_DIG_DVB_C)
	{
	    status = MxLWare603_API_CfgTunerLoopThrough(tuner_id, MXL_ENABLE);
	    if (MXL_SUCCESS != status)
		{
			MXL603_PRINTF("Error! MxLWare603_API_CfgTunerLoopThrough\n");    
			return status;
		}
	}
    return MxL_OK;
}


static MxL_ERR_MSG MxL603_Tuner_RFTune(UINT32 tuner_id, UINT32 rf_freq_hz, MXL603_BW_E bwhz)
{
    MXL_STATUS status = 0; 
    MXL603_CHAN_TUNE_CFG_T chantunecfg;

	if(MXL603_DIG_DVB_C == MxL603_Config[tuner_id].signalMode)
	{
		if (MxL603_Config[tuner_id].w_tuner_if_freq < MXL603_IF_5_38MHz) // 8m mode, for 6M/8M mode tmp code.
		{
			chantunecfg.bandWidth = MXL603_CABLE_BW_8MHz;
		}
		else
		{
			chantunecfg.bandWidth = MXL603_CABLE_BW_6MHz;
		}

		chantunecfg.xtalFreqSel = MXL603_XTAL_16MHz;
	}
	else
	{
		chantunecfg.bandWidth = bwhz;
		chantunecfg.xtalFreqSel = MxL603_Config[tuner_id].c_tuner_crystal;		
	}

	chantunecfg.freqInHz = rf_freq_hz;
    chantunecfg.signalMode = MxL603_Config[tuner_id].signalMode;
    chantunecfg.startTune = MXL_START_TUNE;
    status = MxLWare603_API_CfgTunerChanTune(tuner_id, chantunecfg);
    if (status != MXL_SUCCESS)
    {
        MXL603_PRINTF("Error! MxLWare603_API_CfgTunerChanTune\n");    
        return status;
    }

    // Wait 15 ms 
    MxLWare603_OEM_Sleep(15);

	return MxL_OK;
}




static INT32 tun_mxl603_powcontrol(UINT32 tuner_id, UINT8 stdby)
{
    MXL_STATUS status = 0;

    if(tuner_id >= tuner_cnt)
    {
		return ERR_FAILUE;
    }

    if (stdby)
    {
		//TUN_MXL603_PRINTF("start standby mode!\n");
		status = MxLWare603_API_CfgDevPowerMode(tuner_id, MXL603_PWR_MODE_STANDBY);
		if (MXL_SUCCESS != status)
		{
			//TUN_MXL603_PRINTF("standby mode setting fail!\n");
			return ERR_FAILUE;
		}
     }
     else
     {
	 	//TUN_MXL603_PRINTF("start wakeup mode!\n");
		status = MxLWare603_API_CfgDevPowerMode(tuner_id, MXL603_PWR_MODE_ACTIVE);
		if (MXL_SUCCESS != status)
		{
		    //TUN_MXL603_PRINTF("wakeup mode setting fail!\n");
		    return ERR_FAILUE;
		}
      }

     return SUCCESS;
}


INT32 tun_mxl603_command(UINT32 tuner_id, INT32 cmd, UINT32 param)
{
    INT32 ret = SUCCESS;
#if 0  //kent.2013-6-17 for can't compile	

    switch(cmd)
    {
        case NIM_TUNER_SET_THROUGH_MODE:
            ret = set_through_mode(tuner_id, (DEM_WRITE_READ_TUNER *)param);
            break;

        case NIM_TUNER_POWER_CONTROL:
            tun_mxl603_powcontrol(tuner_id, param);
            break;

	case NIM_TUNER_GET_AGC:
	    ret = tun_mxl603_get_rf_level(tuner_id, (UINT16 *)param);
	    break;

	case NIM_TUNER_GET_RF_POWER_LEVEL:
	    ret = MxLWare603_API_ReqTunerRxPower(tuner_id, (REAL32*)param);
	    break;

        default:
            ret = ERR_FAILUE;
            break;
    }
#endif
    return ret;
}


#endif

