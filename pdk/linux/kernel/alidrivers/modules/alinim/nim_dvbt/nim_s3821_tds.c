/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File:  nim_s3821_tds.c
*   
*    Description:  s3821 nim driver for tds api
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	6.14.2013        Joey.Gao	     Ver 1.0       Create file.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#include "porting_s3821_tds.h"
#include "nim_s3821.h"

struct nim_s3821_private 		*ali_m3821_nim_priv = NULL;
static OSAL_ID 					dem_s3821_task_id = OSAL_INVALID_ID;
/* Name for the tuner, the last character must be Number for index */
static char 					nim_s3821_name[HLD_MAX_NAME_SIZE] = "NIM_QAM_S3821";
static UINT8 					nim_task_num = 0x00;

static UINT8                    *g_s3821_adc2dma_st_addr = NULL;
static UINT32                   g_s3821_adc2dma_read_offset = 0;


//joey, 20130716. temp for large adc2dma requirement.
//static UINT32 g_var_adc2dma_addr = 0xa8000000; // start as the 128M length position.
//static UINT32 g_var_adc2dma_len = (0x10000000-2*8192); // The maxium is 256M, for safty, reduce 2 block(2*8192).

static INT32 nim_s3821_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{
    struct nim_s3821_private *dev_priv = NULL;
    nim_rec_performance_t *p_nim_rec_performance = NULL;

    INT32 freq_offset = 0;
    INT32 ret_val = SUCCESS;

    dev_priv = (struct nim_s3821_private *)dev->priv;

    //joey, 20111031. for adc2dma function.
    if(cmd & NIM_TUNER_COMMAND)
    {
        if(dev_priv->tuner_control.nim_tuner_command != NULL)
        {
            //            set_bypass_mode(dev, TRUE);
            ret_val = dev_priv->tuner_control.nim_tuner_command(dev_priv->tuner_id, cmd, param);
            //            set_bypass_mode(dev, FALSE);
        }
        else
        {
            ret_val = ERR_FAILUE;
        }
        return ret_val;
    }
    switch(cmd)
    {
    case NIM_DRIVER_STOP_ATUOSCAN:
        dev_priv->autoscan_stop_flag = param;
        break;
    case NIM_DRIVER_GET_OSD_FREQ_OFFSET:
        ret_val = nim_s3821_get_osd_int_freqoffset(dev, &freq_offset);
        *(INT32 *)param = freq_offset;
        break;
    case NIM_DRIVER_LOG_IFFT_RESULT:		// need to control in
        break;
    case NIM_DRIVER_GET_AGC:
        return nim_s3821_get_agc(dev, (UINT8 *)param);
        
	case NIM_DRIVER_SEARCH_T2_SIGNAL_ONLY:
		if(param)
		{
			dev_priv->cofdm_type = DVBT2_TYPE;	
		}
		else
		{
			dev_priv->cofdm_type = DVBT2_COMBO;	
		}
       break;
    case NIM_DRIVER_GET_REC_PERFORMANCE_INFO:
        p_nim_rec_performance = (nim_rec_performance_t *)param;
        nim_s3821_get_lock(dev, &(p_nim_rec_performance->lock));
        if(p_nim_rec_performance->lock == 1)
        {
            if(dev_priv->rec_ber_cnt != dev_priv->per_tot_cnt)
            {
                dev_priv->rec_ber_cnt = dev_priv->per_tot_cnt;
                p_nim_rec_performance->ber = dev_priv->snr_ber;
                p_nim_rec_performance->per = dev_priv->snr_per;
                p_nim_rec_performance->valid = TRUE;
            }
            else
            {
                p_nim_rec_performance->valid = FALSE;
            }
        }
        break;
        //joey, 20130711. for ADC2MEM function development.
    case NIM_DRIVER_ADC2MEM_START:
    {
        ret_val = nim_s3821_adc2dma_func_start(dev, param);
        break;
    }
    case NIM_DRIVER_ADC2MEM_STOP:
    {
        ret_val = nim_s3821_adc2dma_func_stop(dev, param);
        break;
    }

    case NIM_DRIVER_ADC2MEM_SEEK_SET:
        //			g_s3821_adc2dma_st_addr = (UINT8 *)((UINT32)((NIM_S3821_ADC2DMA_START_ADDR) & 0xfffffffc));
        //			(*((UINT32*)param)) = (((NIM_S3821_ADC2DMA_MEM_LEN) >> 13) * 8192);
	//plus 3, avoid 4-byte align issue.
        g_s3821_adc2dma_st_addr = (UINT8 *)((UINT32)((g_var_adc2dma_addr + 3) & 0xfffffffc)); 
        (*((UINT32 *)param)) = (((g_var_adc2dma_len) >> 13) * 8192);
        g_s3821_adc2dma_read_offset = 0;
        ret_val = SUCCESS;
        break;
    case NIM_DRIVER_ADC2MEM_READ_8K:
        memcpy((void *)(param), (void *)((UINT32)g_s3821_adc2dma_st_addr + g_s3821_adc2dma_read_offset), 8192);
        g_s3821_adc2dma_read_offset += 8192;
        ret_val = SUCCESS;
        break;
    case NIM_DRIVER_SET_RESET_CALLBACK:
        NIM_S3821_DEBUG("[%s] line = %d,wakeup!\n", __FUNCTION__, __LINE__);
        if (dev_priv->tuner_control.nim_tuner_control != NULL)
        {
            dev_priv->tuner_control.nim_tuner_control(dev_priv->tuner_id, NIM_TUNER_SET_STANDBY_CMD, 1, 0, 0, 0);
        }
        break;
    case NIM_TURNER_SET_STANDBY:
        NIM_S3821_DEBUG("[%s] line = %d,standby!\n", __FUNCTION__, __LINE__);
        if (dev_priv->tuner_control.nim_tuner_control != NULL)
        {
            dev_priv->tuner_control.nim_tuner_control(dev_priv->tuner_id, NIM_TUNER_SET_STANDBY_CMD, 0, 0, 0, 0);
        }
        break;
    case NIM_DRIVER_GET_DEMOD_LOCK_MODE:
        *(UINT8 *)param = TRUE;
        break;
    default:
        ret_val = ERR_FAILUE;
        break;

    }

    return ret_val;

}


static INT32 nim_s3821_ioctl_ext(struct nim_device *dev, INT32 cmd, void *param_list)
{
    NIM_CHANNEL_CHANGE_T *change_para = NULL;

    switch (cmd)
    {
        //joey, 20121021, for T2 mode support.
    case NIM_DRIVER_CHANNEL_CHANGE:
        change_para = (NIM_CHANNEL_CHANGE_T *)param_list;
        /*            return nim_s3811_channel_change(dev, change_para->freq, change_para->bandwidth, \
                        change_para->guard_interval, change_para->fft_mode, change_para->modulation, \
                        change_para->fec, change_para->usage_type, change_para->inverse, change_para->priority);
        */
        struct nim_s3821_private *priv_mem = (struct nim_s3821_private *)(dev->priv);

        if (DVBT2_TYPE == priv_mem->cofdm_type)
        {
            return nim_s3821_dvbt2_channel_change(dev, change_para);
        }
        else if (DVBT2_COMBO == priv_mem->cofdm_type)
        {
            return nim_s3821_combo_channel_change(dev, change_para);
        }
        else if (DVBT_TYPE == priv_mem->cofdm_type)
        {
            return nim_s3821_dvbt_isdbt_channel_change(dev, change_para);
        }
        else if (ISDBT_TYPE == priv_mem->cofdm_type)
        {
            return nim_s3821_dvbt_isdbt_channel_change(dev, change_para);
        }
        else
        {
            //cur_mode = MODE_DVBT2;
            //nim_s3821_set_cur_mode(dev, &cur_mode);
            //return nim_s3821_dvbt2_channel_change(dev, change_para);

            //abnormal case. directly return.
            return ERR_FAILUE;
        }
        break;
        //joey, 20121021, for T2 mode support, do like sharp6158, no need channel_search branch.
        /*

                case NIM_DRIVER_CHANNEL_SEARCH:

                    search_para = (struct NIM_CHANNEL_SEARCH *)param_list;

                    return nim_s3811_channel_search(dev, search_para->freq, search_para->bandwidth, \

                        search_para->guard_interval, search_para->fft_mode, search_para->modulation, \

                        search_para->fec, search_para->usage_type, search_para->inverse, \

                        search_para->freq_offset, search_para->priority);

                    break;

        */

    default:
        break;
    }
    return SUCCESS;

}


/*****************************************************************************
* static INT32 nim_s3821_task_init(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3821_task_init(const struct nim_device *dev)
{
    UINT8 nim_device[3][3] = {'N', 'M', '0', 'N', 'M', '1', 'N', 'M', '2'};
    T_CTSK nim_task_praram={0};
    
    struct nim_s3821_private *priv_mem = (struct nim_s3821_private *)(dev->priv);

    nim_task_praram.task = nim_s3821_main_thread;//dmx_m3327_record_task ;
    //nim_task_praram.name[0] = nim_device[nim_task_num][0];
    //nim_task_praram.name[1] = nim_device[nim_task_num][1];
    //nim_task_praram.name[2] = nim_device[nim_task_num][2];
    nim_task_praram.stksz = 0x1000 ;
    nim_task_praram.itskpri = OSAL_PRI_NORMAL;
    nim_task_praram.quantum = 5 ;
    nim_task_praram.para1 = (UINT32) dev ;
    nim_task_praram.para2 = 0 ;//Reserved for future use.
    priv_mem->thread_id = osal_task_create(&nim_task_praram);
    	
    if (OSAL_INVALID_ID == priv_mem->thread_id)
    {
        //NIM_PRINTF("Task create error\n");
        return OSAL_E_FAIL;
    }
 
    return SUCCESS;
}







static INT32 nim_s3821_open(struct nim_device *dev)
{
    struct nim_s3821_private *priv_mem = (struct nim_s3821_private *)(dev->priv);

	//NIM_S3821_DEBUG("nim_s3811_open\n");
    priv_mem->nim_flag = osal_flag_create(NIM_3821_FLAG_ENABLE);
    priv_mem->nim_s3821_i2c_mutex = osal_mutex_create();
    if(priv_mem->nim_s3821_i2c_mutex == OSAL_INVALID_ID)
    {
        if(priv_mem->log_en)
        {
            NIM_S3821_DEBUG("i2c mutex error\n");
        } 
    }

    if (priv_mem->nim_flag == OSAL_INVALID_ID)
    {
       //NIM_S3821_DEBUG("nim_s3811_open failed\n");
        return ERR_FAILUE;
    }

    // tuner power on
    nim_s3821_ioctl(dev, NIM_TUNER_POWER_CONTROL, FALSE);
    //joey, 20130624, for the init code as a function.
    nim_s3821_init_config(dev);
    osal_flag_set(priv_mem->nim_flag, NIM_3821_SCAN_END);
    //this part is for software patch for S3811 issue.

    nim_s3821_task_init(dev);

    return SUCCESS;

}

/*****************************************************************************
* INT32 nim_s3821_close(struct nim_device *dev)
* Description: m3101 close
* Arguments:
*  Parameter1: struct nim_device *dev
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3821_close(struct nim_device *dev)
{
	UINT8 data = 0; 
	struct nim_s3821_private *priv_mem = (struct nim_s3821_private *)(dev->priv);
	
   // tuner power off
    nim_s3821_ioctl(dev, NIM_TUNER_POWER_CONTROL, TRUE);

    nim_s3821_read(dev, 0x00, &data, 1);
    data |= 0x80;
    nim_s3821_write(dev, 0x00, &data, 1);

     if(OSAL_E_OK!=osal_task_delete(priv_mem->thread_id))  
    {
        NIM_S3821_DEBUG("M3501 Task Delete Error Happenened!\n");
    }

    priv_mem->thread_id = OSAL_INVALID_ID;
    return SUCCESS;
}




/*****************************************************************************

* INT32  nim_s3821_dvbt_attach()

* Description: s3821 initialization

* Arguments:

*  none

* Return Value: INT32

*****************************************************************************/

INT32 nim_s3821_dvbt_attach(char *name, PCOFDM_TUNER_CONFIG_API ptrcofdm_tuner)
{
    struct nim_device *dev = NULL;
    struct nim_s3821_private *priv_mem = NULL;

    if ((ptrcofdm_tuner == NULL))
    {
        //NIM_S3821_DEBUG("Tuner Configuration API structure is NULL!/n");
        return ERR_NO_DEV;
    }

    dev = (struct nim_device *)dev_alloc(name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if (dev == NULL)
    {
        //NIM_S3821_DEBUG("Error: Alloc nim device error!\n");
        return ERR_NO_MEM;
    }

    /* Alloc structure space of private */
    priv_mem = (struct nim_s3821_private *)MALLOC(sizeof(struct nim_s3821_private));
    if ((void *)priv_mem == NULL)
    {
        dev_free(dev);
        //NIM_S3821_DEBUG("Alloc nim device prive memory error!/n");
        return ERR_NO_MEM;
    }

    MEMSET(priv_mem, 0, sizeof(struct nim_s3821_private));
    MEMCPY((void *) & (priv_mem->tuner_control), (void *)ptrcofdm_tuner, sizeof(struct COFDM_TUNER_CONFIG_API));
    priv_mem->nim_flag = OSAL_INVALID_ID;
    priv_mem->nim_s3821_i2c_mutex = OSAL_INVALID_ID;
    //get tuner IF freq.  added by David.Deng @ 2007.12.12
    priv_mem->g_tuner_if_freq = ptrcofdm_tuner->tuner_config.w_tuner_if_freq;   
    //0:ISDBT_TYPE, 1:DVBT_TYPE, 2:DVBT2_TYPE, 3:DVBT2-COMBO, 4...
    priv_mem->cofdm_type = ptrcofdm_tuner->config_data.flag & 0x00000007 ; 
    dev->priv = (void *)priv_mem;

    /* Function point init */
    if (priv_mem->tuner_control.work_mode == NIM_COFDM_SOC_MODE)
    {
        dev->base_addr = S3821_COFDM_SOC_BASE_ADDR;
    }
    else
    {
        dev->base_addr = priv_mem->tuner_control.ext_dm_config.i2c_base_addr;
    }

    dev->init = nim_s3821_dvbt_attach;
    dev->open = nim_s3821_open;
    dev->stop = nim_s3821_close;
    dev->disable = nim_s3821_disable;
    dev->do_ioctl = nim_s3821_ioctl;
    dev->do_ioctl_ext = nim_s3821_ioctl_ext;
    //dev->channel_change = nim_s3821_channel_change;
    //dev->channel_search = nim_s3821_channel_search;
    dev->get_lock = nim_s3821_get_lock;
    dev->get_freq = nim_s3821_get_freq;
    dev->get_fec = nim_s3821_get_code_rate;
    dev->get_agc = nim_s3821_get_agc;
    dev->get_snr = nim_s3821_get_snr;
    dev->get_ber = nim_s3821_get_ber;

    //added for DVB-T additional elements
    dev->get_guard_interval = nim_s3821_get_gi;
    dev->get_fftmode = nim_s3821_get_fftmode;
    dev->get_modulation = nim_s3821_get_modulation;
    dev->get_spectrum_inv = nim_s3821_get_specinv;
    dev->get_hier = nim_s3821_get_hier_mode;
    dev->get_freq_offset =	nim_s3821_get_freq_offset;

    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        //NIM_S3821_DEBUG("Error: Register nim device error!\n");
        FREE(priv_mem);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    if (priv_mem->tuner_control.nim_tuner_init != NULL)
    {
        if (priv_mem->tuner_control.nim_tuner_init(&(priv_mem->tuner_id), &
	           (priv_mem->tuner_control.tuner_config)) != SUCCESS)
        {
            NIM_S3821_DEBUG("Error: Init Tuner Failure!\n");
            return ERR_NO_DEV;
        }
    }

    return SUCCESS;
}






























