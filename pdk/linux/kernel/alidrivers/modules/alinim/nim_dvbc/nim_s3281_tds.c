/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_s3281_tds.c
*
*    Description:  s3281 nim driver for linux api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "porting_s3281_tds.h"
#include "nim_s3281.h"



struct nim_s3281_private         *ali_m3281_nim_priv = NULL;
static OSAL_ID                     dem_s3281_task_id = OSAL_INVALID_ID;
/* Name for the tuner, the last character must be Number for index */
static char                     nim_s3281_name[HLD_MAX_NAME_SIZE] = "NIM_QAM_S3281";
//static UINT8                     nim_task_num = 0x00;



/*****************************************************************************
* static INT32 nim_s3281_task_init(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/

INT32 nim_s3281_task_init(struct nim_device *dev)
{
    UINT8 nim_device[3][3] =
    {
        'N', 'M', '0', 'N', 'M', '1', 'N', 'M', '2'
    };
    T_CTSK nim_task_praram;
    static UINT8 nim_task_num = 0x00;
    struct nim_s3281_private *priv = (struct nim_s3281_private *) dev->priv;

    if (nim_task_num > 1)
        return SUCCESS;

    nim_task_praram.task = nim_s3281_task_proc;
    nim_task_praram.name[0] = nim_device[nim_task_num][0];
    nim_task_praram.name[1] = nim_device[nim_task_num][1];
    nim_task_praram.name[2] = nim_device[nim_task_num][2];
    nim_task_praram.stksz = 0xc00 ;
    nim_task_praram.itskpri = OSAL_PRI_NORMAL;
    nim_task_praram.quantum = 10 ;
    nim_task_praram.para1 = (UINT32) dev ;
    nim_task_praram.para2 = 0 ;
    priv->thread_id = osal_task_create(&nim_task_praram);
    if (OSAL_INVALID_ID == priv->thread_id)
    {
        return OSAL_E_FAIL;
    }
    nim_task_num++;

    return SUCCESS;
}



#if 0
static INT32 nim_s3281_task_init(struct nim_device *dev)
{
    UINT8 nim_device[3][3] = {'N', 'M', '0', 'N', 'M', '1', 'N', 'M', '2'};
    T_CTSK nim_task_praram={0};

    struct nim_s3281_private *priv = (struct nim_s3281_private *) dev->priv;

    //INT32 ret = 0;

    if (nim_task_num > 1)
    {
        return SUCCESS;
    }

    nim_task_praram.task = nim_s3281_dvbc_task;//dmx_m3327_record_task ;
    nim_task_praram.name[0] = nim_device[nim_task_num][0];
    nim_task_praram.name[1] = nim_device[nim_task_num][1];
    nim_task_praram.name[2] = nim_device[nim_task_num][2];
    nim_task_praram.stksz = 0xc00 ;
    nim_task_praram.itskpri = OSAL_PRI_NORMAL;
    nim_task_praram.quantum = 10 ;
    nim_task_praram.para1 = (UINT32) dev ;
    nim_task_praram.para2 = 0 ;//Reserved for future use.
    priv->tsk_status.m_task_id = osal_task_create(&nim_task_praram);
    if (OSAL_INVALID_ID == priv->tsk_status.m_task_id)
    {
        //NIM_PRINTF("Task create error\n");
        return OSAL_E_FAIL;
    }
    nim_task_num++;

    return SUCCESS;
}

#endif

/*****************************************************************************
* INT32 nim_s3281_dvbc_open(struct nim_device *dev)
* Description: S3202 open
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/

static INT32 nim_s3281_dvbc_open(struct nim_device *dev)
{
    struct nim_s3281_private *priv = NULL; 
    
	if(dev == NULL)
    {
	    return ERR_FAILURE;
	}	
    priv = dev->priv;

    priv->i2c_mutex = osal_mutex_create();
    if (priv->i2c_mutex == OSAL_INVALID_ID)
    {
        S3281_PRINTF("nim_s3281_dvbc_open: Create mutex failed!\n");
    }



    nim_s3281_hw_init(dev);


    //nim_s3281_task_init(dev);


    return SUCCESS;
}



/*****************************************************************************
* INT32 nim_s3281_dvbc_close(struct nim_device *dev)
* Description: S3202 close
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3281_dvbc_close(struct nim_device *dev)
{
    UINT8 data = 0;
    INT32 ret = SUCCESS;
	struct nim_s3281_private *priv = NULL;
	
    if(dev == NULL)
    {
	    return ERR_FAILUE;
	}	
    priv = dev->priv;

    //soft reset.
    data = 0x80;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);

    if (RET_SUCCESS != osal_mutex_delete(priv->i2c_mutex))
    {
        S3281_PRINTF("nim_s3281_dvbc_close: Delete mutex failed!\n");
        ret = ERR_FAILUE;
    }

    priv->i2c_mutex = OSAL_INVALID_ID;

#ifndef NOT_MONITOR_NIM
    if (RET_SUCCESS != osal_task_delete(dem_s3281_task_id))
    {
        S3281_PRINTF("nim_s3281_dvbc_close: Delete minitoring task failed!\n");
        ret = ERR_FAILUE;
    }
    dem_s3281_task_id = OSAL_INVALID_ID;
#endif


    return ret;
}



/*****************************************************************************
* INT32 nim_s3281_dvbc_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
*
*  device input/output operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: INT32 cmd
*  Parameter3: UINT32 param
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3281_dvbc_ioctl(struct nim_device *dev,INT32 cmd,UINT32 param)
{
    INT32 rtn = 0;

    if(dev == NULL)
    {
	    return ERR_FAILUE;
	}	
    switch( cmd )
    {
    case NIM_DRIVER_READ_QPSK_BER:
        rtn =  nim_s3281_dvbc_get_ber(dev, (UINT32 *)param);
        break;
    case NIM_DRIVER_READ_RSUB:
        rtn =  nim_s3281_dvbc_get_per(dev, (UINT32 *)param);
        break;
    case NIM_DRIVER_SET_RESET_CALLBACK:
        S3281_PRINTF("[%s] line = %d,wakeup!\n", __FUNCTION__, __LINE__);
        /*struct nim_s3281_private *priv;
        priv = (struct nim_s3281_private *)dev->priv;
        if (priv->nim_Tuner_Control != NULL)
         {
               priv->nim_Tuner_Control(priv->tuner_id, NIM_TUNER_SET_STANDBY_CMD, 1,0,0,0);
         }*/
        break;
    case NIM_TURNER_SET_STANDBY:
        S3281_PRINTF("[%s] line = %d,standby!\n", __FUNCTION__, __LINE__);
        /*struct nim_s3281_private *priv;
        priv = (struct nim_s3281_private *)dev->priv;
        if (priv->nim_Tuner_Control != NULL)
         {
           priv->nim_Tuner_Control(priv->tuner_id, NIM_TUNER_SET_STANDBY_CMD, 0,0,0,0);
         }*/
        break;
    case NIM_DRIVER_GET_AGC:
        rtn = nim_s3281_dvbc_get_agc(dev, (UINT8 *)param);
        break;
    default:
        rtn = SUCCESS;
        break;
    }
    return rtn;
}
static INT32 nim_s3281_dvbc_ioctl_ext(struct nim_device *dev,INT32 cmd,void *param_list)
{
    INT32 rtn = 0;
    struct nim_s3281_private *priv = NULL;
    struct NIM_I2C_INFO *i2c_info = NULL;

    if(dev == NULL)
    {
	    return ERR_FAILUE;
	}	
    switch( cmd )
    {
    case NIM_DRIVER_AUTO_SCAN:            /* Do AutoScan Procedure */
        //nim_s3202_AutoScan(dev, (struct NIM_AUTO_SCAN *) (param_list));
        rtn = SUCCESS;
        break;
    case NIM_DRIVER_CHANNEL_CHANGE:        /* Do Channel Change */
        rtn = nim_s3281_dvbc_channel_change(dev, (NIM_CHANNEL_CHANGE_T *) (param_list));
        break;
    case NIM_DRIVER_QUICK_CHANNEL_CHANGE:    /* Do Quick Channel Change without waiting lock */
        rtn = nim_s3281_dvbc_quick_channel_change(dev, (NIM_CHANNEL_CHANGE_T *) (param_list));
        break;
    case NIM_DRIVER_CHANNEL_SEARCH:    /* Do Channel Search */
        rtn = SUCCESS;
        break;
    case NIM_DRIVER_GET_RF_LEVEL:
        rtn = nim_s3281_dvbc_get_rf_level(dev, (UINT16 *)param_list);
        break;
    case NIM_DRIVER_GET_CN_VALUE:
        rtn = nim_s3281_dvbc_get_cn_value(dev, (UINT16 *)param_list);
        break;
    case NIM_DRIVER_GET_BER_VALUE:
        rtn = nim_s3281_dvbc_get_ber(dev, (UINT32 *)param_list);
        break;
    case NIM_DRIVER_SET_PERF_LEVEL:
        rtn = nim_s3281_dvbc_set_perf_level(dev, (UINT32)param_list);
        break;
    case NIM_DRIVER_GET_I2C_INFO:
    {
        i2c_info = (struct NIM_I2C_INFO *)param_list;
        priv = dev->priv;

#if (QAM_WORK_MODE == QAM_ONLY)

        i2c_info->i2c_type = priv->i2c_type;
#ifndef SYS_DEM_BASE_ADDR
        i2c_info->i2c_addr = 0x40;
#else
        i2c_info->i2c_addr = SYS_DEM_BASE_ADDR;
#endif
        rtn = SUCCESS;
#else
        i2c_info->i2c_type = 0xFF;
        i2c_info->i2c_addr = 0xFF;
        rtn = ERR_FAILUE;
#endif
        break;
    }
    case NIM_DRIVER_SET_NIM_MODE:
        //kent:tunerid_input=1
        rtn = nim_s3281_dvbc_set_mode(dev, 1, *((struct DEMOD_CONFIG_ADVANCED *)param_list));
        break;

    case NIM_DRIVER_RESET_QAM_FSM:   //kent,for reset qam finite state machine
        rtn = nim_s3281_fsm_reset(dev);
        break;

    default:
        rtn = SUCCESS;
        break;
    }

    return rtn;
}



INT32 nim_s3281_dvbc_attach(struct QAM_TUNER_CONFIG_API *ptrqam_tuner)
{

    struct nim_device *dev = NULL;
    struct nim_s3281_private *priv = NULL;

    if(ptrqam_tuner == NULL)
    {
	    return RET_FAILURE;
	}	
    /* Attatch the private Tuner Configuration */
#if (QAM_FPGA_USAGE == SYS_FUNC_ON)
    nim_s3202_tuner_attatch(ptrqam_tuner);
#endif

    /* Alloc structure space of tuner devive*/
    dev = (struct nim_device *)dev_alloc(nim_s3281_name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if (dev == NULL)
    {
        S3281_PRINTF("Error: Alloc nim device error!\n");
        return ERR_NO_MEM;
    }

    /* Alloc structure space of private */
    priv = (struct nim_s3281_private *)comm_malloc(sizeof(struct nim_s3281_private));
    if ((void *)priv == NULL)
    {
        dev_free(dev);
        S3281_PRINTF("Alloc nim device prive memory error!/n");
        return ERR_NO_MEM;
    }
    comm_memset((void *)priv, 0, sizeof(struct nim_s3281_private));


    priv->i2c_mutex = OSAL_INVALID_ID;
    ali_m3281_nim_priv = priv;


    /* tuner configuration function */
    comm_memcpy(&priv->tuner_config_data,&ptrqam_tuner->tuner_config_data,sizeof(struct QAM_TUNER_CONFIG_DATA));
    comm_memcpy(&priv->tuner_config_ext,&ptrqam_tuner->tuner_config_ext,sizeof(struct QAM_TUNER_CONFIG_EXT));
    priv->nim_tuner_init = ptrqam_tuner->nim_tuner_init;
    priv->nim_tuner_control = ptrqam_tuner->nim_tuner_control;
    priv->nim_tuner_status = ptrqam_tuner->nim_tuner_status;
    priv->qam_mode = ptrqam_tuner->dem_config_advanced.qam_config_advanced;
    priv->qam_buffer_addr = ptrqam_tuner->dem_config_advanced.qam_buffer_addr;
    priv->qam_buffer_len = ptrqam_tuner->dem_config_advanced.qam_buffer_len;
    priv->i2c_type = ptrqam_tuner->tuner_config_ext.i2c_type_id;


    dev->priv = (void *)priv;
    /* Function point init */
    dev->base_addr = S3281_QAM_ONLY_I2C_BASE_ADDR;    //0x40
    dev->init = nim_s3281_dvbc_attach;
    dev->open = nim_s3281_dvbc_open;
    dev->stop = nim_s3281_dvbc_close;
    dev->do_ioctl = nim_s3281_dvbc_ioctl;
    dev->do_ioctl_ext = nim_s3281_dvbc_ioctl_ext;
    dev->get_lock = nim_s3281_dvbc_get_lock;
    dev->get_freq = nim_s3281_dvbc_get_freq;
    dev->get_fec = nim_s3281_dvbc_get_qam_order;
    dev->get_snr = nim_s3281_dvbc_get_snr;
    dev->get_sym = nim_s3281_dvbc_get_symbol_rate;
    dev->get_ber =nim_s3281_dvbc_get_ber;
    dev->get_agc = nim_s3281_dvbc_get_agc;
    dev->get_fft_result = nim_s3281_dvbc_get_fft_result;
   // dev->channel_search = nim_s3281_dvbc_channel_search;

    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        S3281_PRINTF("Error: Register nim device error!\n");
        comm_free(priv);
        dev_free(dev);
        return ERR_NO_DEV;
    }
    S3281_PRINTF("[%s]line=%d,here!\n", __FUNCTION__, __LINE__);

    return SUCCESS;
}


