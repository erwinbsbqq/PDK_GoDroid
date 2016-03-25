/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_m3501_tds.c
*
*    Description:  m3501 nim driver for tds api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/



#include "nim_m3501.h"


#define NIM_DEV_NUM_2 2


/* Name for the tuner, the last character must be Number for index */
static char nim_s3501_name[3][HLD_MAX_NAME_SIZE] =
{
    "NIM_S3501_0", "NIM_S3501_1", "NIM_S3501_2"
};


static UINT8 nim_task_num = 0x00;
static unsigned char nim_dev_num = 0;



static INT32 nim_s3501_close(struct nim_device *dev);

static INT32 nim_s3501_task_init(struct nim_device *dev);
static INT32 nim_s3501_ioctl_ext(struct nim_device *dev, INT32 cmd, void *param_list);


void nim_comm_delay(UINT32 us)
{
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 k = 0;

    i = us / 0xFFFF;
    j = us % 0xFFFF;
    for (k = 0; k < i; k++)
    {
        osal_delay(0xffff);
    }
    osal_delay(j);
    return;
}


INT32 nim_callback(NIM_AUTO_SCAN_T *pstauto_scan, void *pfun, UINT8 status, UINT8 polar, UINT32 freq,
                    UINT32 sym, UINT8 fec, UINT8 stop)
{
    if(pstauto_scan == NULL)
    {
	    return RET_FAILURE;
	}	
    return pstauto_scan->callback(status, polar, freq, sym, fec);
}


UINT32 nim_flag_read(struct nim_s3501_private *priv, UINT32 t1, UINT32 t2, UINT32 t3)
{
    UINT32 flag_ptn = 0;

    if(priv == NULL)
    {
	    return RET_FAILURE;
	}	
    osal_flag_wait(&flag_ptn, priv->flag_id, t1, t2, t3);

    return flag_ptn;
}

UINT32 nim_flag_create(struct nim_s3501_private *priv)
{
    if(priv == NULL)
    {
	    return RET_FAILURE;
	}	
    if (priv->flag_id == OSAL_INVALID_ID)
    {
        priv->flag_id = osal_flag_create(0);
    }

    return 0;
}

UINT32 nim_flag_set(struct nim_s3501_private *priv, UINT32 value)
{
    if(priv == NULL)
    {
	    return RET_FAILURE;
	}	
    return osal_flag_set(priv->flag_id, value);

}

UINT32 nim_flag_clear(struct nim_s3501_private *priv, UINT32 value)
{
    if(priv == NULL)
    {
	    return RET_FAILURE;
	}	
    return osal_flag_clear(priv->flag_id, value);

}

UINT32 nim_flag_del(struct nim_s3501_private *priv)
{
    if(priv == NULL)
    {
	    return RET_FAILURE;
	}	
    return osal_flag_delete(priv->flag_id);

}



INT32 nim_s3501_autoscan_signal_input(struct nim_device *dev, UINT8 s_cse)
{
    struct dmx_device *l_nim_dmx_dev=NULL;
	struct nim_s3501_private *priv =NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    l_nim_dmx_dev = (struct dmx_device *) dev_get_by_name("DMX_S3601_0");
    switch (s_cse)
    {
    case NIM_SIGNAL_INPUT_OPEN:
        dmx_io_control(l_nim_dmx_dev, IO_SET_BYPASS_MODE, (UINT32) (priv->ul_status.adcdata));
        break;
    case NIM_SIGNAL_INPUT_CLOSE:
        dmx_io_control(l_nim_dmx_dev, IO_CLS_BYPASS_MODE, (UINT32) NULL);
        break;
     default:
        break;
    }

    return SUCCESS;
}




/*****************************************************************************
* INT32 nim_s3501_attach (struct QPSK_TUNER_CONFIG_API * ptrqpsk_tuner)
* Description: S3501 initialization
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3501_attach(struct QPSK_TUNER_CONFIG_API *ptrqpsk_tuner)
{
    struct nim_device *dev = NULL;
    struct nim_s3501_private *priv_mem = NULL;


    NIM_PRINTF("Enter nim_s3501_attach:\n");

    if (ptrqpsk_tuner == NULL)
    {
        NIM_PRINTF("Tuner Configuration API structure is NULL!/n");
        return ERR_NO_DEV;
    }
    if (nim_dev_num > NIM_DEV_NUM_2)
    {
        NIM_PRINTF("Can not support three or more S3501 !/n");
        return ERR_NO_DEV;
    }

    dev = (struct nim_device *) dev_alloc(nim_s3501_name[nim_dev_num], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if (dev == NULL)
    {
        NIM_PRINTF("Error: Alloc nim device error!\n");
        return ERR_NO_MEM;
    }

    priv_mem = (struct nim_s3501_private *) comm_malloc(sizeof(struct nim_s3501_private));
    if (priv_mem == NULL)
    {
        dev_free(dev);
        NIM_PRINTF("Alloc nim device prive memory error!/n");
        return ERR_NO_MEM;
    }
    comm_memset(priv_mem, 0, sizeof(struct nim_s3501_private));
    dev->priv = (void *) priv_mem;
    //diseqc state init
    dev->diseqc_info.diseqc_type = 0;
    dev->diseqc_info.diseqc_port = 0;
    dev->diseqc_info.diseqc_k22 = 0;

    if ((ptrqpsk_tuner->config_data.qpsk_config & M3501_POLAR_REVERT) == M3501_POLAR_REVERT) //bit4: polarity revert.
    {
        dev->diseqc_info.diseqc_polar = LNB_POL_V;
    }
    else //default usage, not revert.
    {
        dev->diseqc_info.diseqc_polar = LNB_POL_H;
    }

    dev->diseqc_typex = 0;
    dev->diseqc_portx = 0;

    /* Function point init */
    dev->base_addr = ptrqpsk_tuner->ext_dm_config.i2c_base_addr;
    dev->init = nim_s3501_attach;
    dev->open = nim_s3501_open;
    dev->stop = nim_s3501_close;
    dev->do_ioctl = nim_s3501_ioctl;
    dev->do_ioctl_ext = nim_s3501_ioctl_ext;
    dev->get_lock = nim_s3501_get_lock;
    dev->get_freq = nim_s3501_get_freq;
    dev->get_fec = nim_s3501_get_code_rate;
    dev->get_snr = nim_s3501_get_snr;

#if ( SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)
    dev->set_polar = nim_s3501_set_polar;
    dev->set_12v = nim_s3501_set_12v;
    dev->channel_search = nim_s3501_channel_search;
    dev->di_seq_c_operate = nim_s3501_di_seq_c_operate;
    dev->di_seq_c2x_operate = nim_s3501_di_seq_c2x_operate;
    dev->get_sym = nim_s3501_get_symbol_rate;
    dev->get_agc = nim_s3501_get_agc;
    dev->get_ber = nim_s3501_get_ber;
    dev->get_fft_result = nim_s3501_get_fft_result;
    dev->get_ver_infor = NULL;
#endif
    /* tuner configuration function */
    priv_mem->nim_tuner_init = ptrqpsk_tuner->nim_tuner_init;
    priv_mem->nim_tuner_control = ptrqpsk_tuner->nim_tuner_control;
    priv_mem->nim_tuner_status = ptrqpsk_tuner->nim_tuner_status;
    priv_mem->i2c_type_id = ptrqpsk_tuner->tuner_config.i2c_type_id;

    priv_mem->tuner_config_data.qpsk_config = ptrqpsk_tuner->config_data.qpsk_config;
    priv_mem->ext_dm_config.i2c_type_id = ptrqpsk_tuner->ext_dm_config.i2c_type_id;
    priv_mem->ext_dm_config.i2c_base_addr = ptrqpsk_tuner->ext_dm_config.i2c_base_addr;

    priv_mem->ul_status.m_enable_dvbs2_hbcd_mode = 0;
    priv_mem->ul_status.m_dvbs2_hbcd_enable_value = 0x7f;
    priv_mem->ul_status.nim_s3501_sema = OSAL_INVALID_ID;
    priv_mem->ul_status.s3501_autoscan_stop_flag = 0;
    priv_mem->ul_status.s3501_chanscan_stop_flag = 0;
    priv_mem->ul_status.old_ber = 0;
    priv_mem->ul_status.old_per = 0;
    priv_mem->ul_status.m_hw_timeout_thr = 0;
    priv_mem->ul_status.old_ldpc_ite_num = 0;
    priv_mem->ul_status.c_rs = 0;
    priv_mem->ul_status.phase_err_check_status = 0;
    priv_mem->ul_status.s3501d_lock_status = NIM_LOCK_STUS_NORMAL;
    priv_mem->ul_status.m_tso_status = NIM_TSO_STUS_UNLOCK;
    priv_mem->ul_status.m_s3501_type = 0x00;
    priv_mem->ul_status.m_setting_freq = 123;
    priv_mem->ul_status.m_err_cnts = 0x00;
    priv_mem->tsk_status.m_lock_flag = NIM_LOCK_STUS_NORMAL;
    priv_mem->tsk_status.m_task_id = 0x00;
    priv_mem->t_param.t_aver_snr = -1;
    priv_mem->t_param.t_last_iter = -1;
    priv_mem->t_param.t_last_snr = -1;
    priv_mem->t_param.t_snr_state = 0;
    priv_mem->t_param.t_snr_thre1 = 256;
    priv_mem->t_param.t_snr_thre2 = 256;
    priv_mem->t_param.t_snr_thre3 = 256;
    priv_mem->t_param.t_dynamic_power_en = 0;
    priv_mem->t_param.t_phase_noise_detected = 0;
    priv_mem->t_param.t_reg_setting_switch = 0x0f;
    priv_mem->t_param.t_i2c_err_flag = 0x00;
    priv_mem->flag_id = OSAL_INVALID_ID;

    priv_mem->blscan_mode = NIM_SCAN_SLOW;

    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        NIM_PRINTF("Error: Register nim device error!\n");
        comm_free(priv_mem);
        dev_free(dev);
        return ERR_NO_DEV;
    }
    nim_dev_num++;
    priv_mem->ul_status.nim_s3501_sema = NIM_MUTEX_CREATE(1);

    if (nim_s3501_i2c_open(dev))
    {
        return S3501_ERR_I2C_NO_ACK;
    }

    // Initial the QPSK Tuner
    if (priv_mem->nim_tuner_init != NULL)
    {
        NIM_PRINTF(" Initial the Tuner \n");
        if (((struct nim_s3501_private *) dev->priv)->nim_tuner_init(&priv_mem->tuner_id,
                                                  &(ptrqpsk_tuner->tuner_config)) != SUCCESS)
        {
            NIM_PRINTF("Error: Init Tuner Failure!\n");

            if (nim_s3501_i2c_close(dev))
            {
                return S3501_ERR_I2C_NO_ACK;
            }

            return ERR_NO_DEV;
        }
    }

    if (nim_s3501_i2c_close(dev))
    {
        return S3501_ERR_I2C_NO_ACK;
    }

    nim_s3501_ext_lnb_config(dev, ptrqpsk_tuner);

    nim_s3501_get_type(dev);

    if (priv_mem->ul_status.m_s3501_type == NIM_CHIP_ID_M3501A &&
            (priv_mem->tuner_config_data.qpsk_config & 0xc0) == M3501_2BIT_MODE)
    {
        if(SYS_MAIN_BOARD == BOARD_DB_M3606_01V01)
        {
            priv_mem->tuner_config_data.qpsk_config &= 0x3f;
            NIM_PRINTF("M3501A SSI 2bit mode, auto change to 1bit mode\n");
        }
    }

    ptrqpsk_tuner->device_type = priv_mem->ul_status.m_s3501_type;

    NIM_PRINTF("Leave nim_s3501_attach\n");

    return SUCCESS;
}


/*****************************************************************************
* INT32 nim_s3501_open(struct nim_device *dev)
* Description: S3501 open
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3501_open(struct nim_device *dev)
{
    INT32 ret = 0;
    UINT8 data = 0;
	struct nim_s3501_private *priv =NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    NIM_PRINTF("Enter nim_s3501_open:\n");
    nim_s3501_set_acq_workmode(dev, NIM_OPTR_HW_OPEN);

    if(priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B && priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C)
    {
        // open clock
        nim_reg_read(dev, RCE_TS_FMT_CLK + 1, &data, 1);
        data = data | 0x01;
        nim_reg_write(dev, RCE_TS_FMT_CLK + 1, &data, 1);
        nim_reg_read(dev, R5B_ACQ_WORK_MODE, &data, 1);
        data = data | 0x40;
        nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);

        // open rxadc
        nim_reg_read(dev, RA0_RXADC_REG + 2, &data, 1);
        data = data & 0xf8;
        nim_reg_write(dev, RA0_RXADC_REG + 2, &data, 1);
    }

    ret = nim_s3501_hw_check(dev);
    if (ret != SUCCESS)
    {
        return ret;
    }
    ret = nim_s3501_hw_init(dev);

    nim_s3501_after_reset_set_param(dev);

    nim_s3501_hbcd_timeout(dev, NIM_OPTR_HW_OPEN);

    nim_s3501_task_init(dev);

#ifdef CHANNEL_CHANGE_ASYNC
    nim_flag_create(priv);

#endif
    NIM_PRINTF("Leave nim_s3501_open\n");
    return SUCCESS;
}



/*****************************************************************************
* INT32 nim_s3501_close(struct nim_device *dev)
* Description: S3501 close
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3501_close(struct nim_device *dev)
{
    UINT8 data = 0;
	struct nim_s3501_private *priv =NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;
	
    priv->work_alive = 0;
    nim_s3501_demod_ctrl(dev, NIM_DEMOD_CTRL_0X90);

    nim_s3501_set_acq_workmode(dev, NIM_OPTR_HW_CLOSE);

    if(priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B && priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C)
    {
        // close clock
        nim_reg_read(dev, RCE_TS_FMT_CLK + 1, &data, 1);
        data = data & 0xfe;
        nim_reg_write(dev, RCE_TS_FMT_CLK + 1, &data, 1);
        nim_reg_read(dev, R5B_ACQ_WORK_MODE, &data, 1);
        data = data & 0xbf;
        nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);

        // close rxadc
        nim_reg_read(dev, RA0_RXADC_REG + 2, &data, 1);
        data = data | 0x07;
        nim_reg_write(dev, RA0_RXADC_REG + 2, &data, 1);
    }
    NIM_MUTEX_DELETE(priv->ul_status.nim_s3501_sema);

#ifdef CHANNEL_CHANGE_ASYNC

    nim_flag_del(priv);
#endif

    if(OSAL_E_OK != osal_task_delete(priv->tsk_status.m_task_id))
    {
        AUTOSCAN_PRINTF("M3501 Task Delete Error Happenened!\n");
    }
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3501_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
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
INT32 nim_s3501_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{
    INT32 crnum = 0;
    INT32 curfreq = 0;
    INT32 step = 0;
    struct nim_s3501_private *priv =NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    switch (cmd)
    {
    case NIM_DRIVER_READ_TUNER_STATUS:
        return nim_s3501_tuner_lock(dev, (UINT8 *) param);
    case NIM_DRIVER_READ_QPSK_STATUS:
        return nim_s3501_get_lock(dev, (UINT8 *) param);
    case NIM_DRIVER_READ_FEC_STATUS:
        break;
    case NIM_DRIVER_READ_QPSK_BER:
        return nim_s3501_get_ber(dev, (UINT32 *) param);
    case NIM_DRIVER_READ_VIT_BER:
        break;
    case NIM_DRIVER_READ_RSUB:
        return nim_s3501_get_per(dev, (UINT32 *) param);
    case NIM_DRIVER_STOP_ATUOSCAN:
        priv->ul_status.s3501_autoscan_stop_flag = param;
        break;
    case NIM_DRIVER_GET_CR_NUM:
        crnum = (INT32) nim_s3501_get_crnum(dev);
        return crnum;
    case NIM_DRIVER_GET_CUR_FREQ:
        switch (param)
        {
        case NIM_FREQ_RETURN_SET:
            return priv->ul_status.m_setting_freq;
        case NIM_FREQ_RETURN_REAL:
        default:
            curfreq = (INT32) nim_s3501_get_curfreq(dev);
            return curfreq;
        }
    break;
    case NIM_DRIVER_FFT_PARA:
        nim_s3501_set_fft_para(dev);
        break;
    case NIM_DRIVER_FFT:
        return nim_s3501_fft(dev, param);
    case NIM_FFT_JUMP_STEP:
        step = 0;
        nim_s3501_get_tune_freq(dev, &step);
        return step;
    case NIM_DRIVER_SET_RESET_CALLBACK:
        priv->ul_status.m_pfn_reset_s3501 = (pfn_nim_reset_callback) param;
        break;
    case NIM_DRIVER_RESET_PRE_CHCHG:
        if (priv->ul_status.m_pfn_reset_s3501)
        {
            priv->ul_status.m_pfn_reset_s3501((priv->tuner_id + 1) << 16);
        }
        break;
    case NIM_DRIVER_ENABLE_DVBS2_HBCD:
        priv->ul_status.m_enable_dvbs2_hbcd_mode = param;
        nim_s3501_hbcd_timeout(dev, NIM_OPTR_IOCTL);
        break;
    case NIM_DRIVER_STOP_CHANSCAN:
        priv->ul_status.s3501_chanscan_stop_flag = param;
        break;
    case NIM_DRIVER_CHANGE_TS_GAP:
        return nim_change_ts_gap(dev, (UINT8)param);
    case NIM_DRIVER_SET_SSI_CLK:
        return nim_s3501_set_ssi_clk(dev, (UINT8)param);

    case NIM_DRIVER_SET_BLSCAN_MODE:
        if(0 == param)
        {
        priv->blscan_mode = NIM_SCAN_FAST;
        }
        else if(1 == param)
        {
        priv->blscan_mode = NIM_SCAN_SLOW;
        }
        break;

    case NIM_DRIVER_SET_POLAR:
        return nim_s3501_set_polar(dev, (UINT8)param);

    case NIM_DRIVER_SET_12V:
        return nim_s3501_set_12v(dev, (UINT8)param);

    case NIM_DRIVER_GET_SYM:
        return nim_s3501_get_symbol_rate(dev, (UINT32 *)param);

    case NIM_DRIVER_GET_BER:
        return nim_s3501_get_ber(dev, (UINT32 *)param);

    case NIM_DRIVER_GET_AGC:
        return nim_s3501_get_agc(dev, (UINT8 *)param);

    case NIM_TURNER_SET_STANDBY:
        if (nim_s3501_i2c_open(dev))
        {
        return S3501_ERR_I2C_NO_ACK;
        }
        if (priv->nim_tuner_control != NULL)
        {
            priv->nim_tuner_control(priv->tuner_id, NIM_TUNER_SET_STANDBY_CMD, 0);
        }
        if (nim_s3501_i2c_close(dev))
        {
        return S3501_ERR_I2C_NO_ACK;
        }
        break;
    default:
        break;
    }
    return SUCCESS;
}



INT32 nim_s3501_ioctl_ext(struct nim_device *dev, INT32 cmd, void *param_list)
{
    nim_get_fft_result_t *fft_para=NULL;
    nim_diseqc_operate_para_t *diseqc_para=NULL;
	struct nim_s3501_private *priv = NULL;
    INT32 result = SUCCESS;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    
    priv = (struct nim_s3501_private *) dev->priv;
    NIM_PRINTF("    Enter fuction nim_s3501_event_control\n");
    switch (cmd)
    {
    case NIM_DRIVER_AUTO_SCAN:
        /* Do AutoScan Procedure */
#ifdef NIM_S3501_ASCAN_TOOLS
#endif
#ifdef NIM_S3501_ASCAN_TOOLS
        ascan_stop_flag = 0x00;
#endif
        NIM_PRINTF(">>>[JIE] NIM_DRIVER_AUTO_SCAN\n");
        result = nim_s3501_autoscan(dev, (NIM_AUTO_SCAN_T *) (param_list));
#ifdef NIM_S3501_ASCAN_TOOLS
        if(priv->ul_status.s3501_autoscan_stop_flag)
            ascan_stop_flag = 0x01;
        else
            nim_s3501_ascan_tps_lock_check();
#endif
#ifdef NIM_S3501_ASCAN_TOOLS
        //nim_s3501_ascan_end_process();
#endif
        return result;
    case NIM_DRIVER_CHANNEL_CHANGE:
        /* Do Channel Change */
        NIM_PRINTF(">>>[JIE] NIM_DRIVER_CHANNEL_CHANGE\n");
        {
            struct NIM_CHANNEL_CHANGE *nim_param = (struct NIM_CHANNEL_CHANGE *) (param_list);

            return nim_s3501_channel_change(dev, nim_param->freq, nim_param->sym, nim_param->fec);
        }
    case NIM_DRIVER_CHANNEL_SEARCH:
        /* Do Channel Search */
        break;
    case NIM_DRIVER_GET_ID:
        *((UINT32 *) param_list) = priv->ul_status.m_s3501_type;
        break;
    case NIM_DRIVER_GET_FFT_RESULT:
        fft_para = (nim_get_fft_result_t *)(param_list);
        return nim_s3501_get_fft_result(dev, fft_para->freq, fft_para->start_addr);
    case NIM_DRIVER_DISEQC_OPERATION:
        diseqc_para = (nim_diseqc_operate_para_t *)(param_list);
        return nim_s3501_di_seq_c_operate(dev, diseqc_para->mode, diseqc_para->cmd, diseqc_para->cnt);
  
    case NIM_DRIVER_DISEQC2X_OPERATION:
        diseqc_para = (nim_diseqc_operate_para_t *)(param_list);
        return nim_s3501_di_seq_c2x_operate(dev, diseqc_para->mode, diseqc_para->cmd, diseqc_para->cnt, \
                                          diseqc_para->rt_value, diseqc_para->rt_cnt);

    default:
        break;
    }

    return SUCCESS;
}


static INT32 nim_s3501_task_init(struct nim_device *dev)
{
    UINT8 nim_device[3][3] =
    {
        {'N', 'M', '0'},
        {'N', 'M', '1'},
        { 'N', 'M', '2'}
    };
    T_CTSK nim_task_praram;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    if (nim_task_num > 1)
    {
        return SUCCESS;
    }   
	
	MEMSET(&nim_task_praram, 0 , sizeof(nim_task_praram));
	
    priv->work_alive = 1;
	
    nim_task_praram.task = nim_s3501_task;
    nim_task_praram.name[0] = nim_device[nim_task_num][0];
    nim_task_praram.name[1] = nim_device[nim_task_num][1];
    nim_task_praram.name[2] = nim_device[nim_task_num][2];
    nim_task_praram.stksz = 0xc00 ;
    nim_task_praram.itskpri = OSAL_PRI_NORMAL;
    nim_task_praram.quantum = 10 ;
    nim_task_praram.para1 = (UINT32) dev ;
    nim_task_praram.para2 = 0 ;
    priv->tsk_status.m_task_id = osal_task_create(&nim_task_praram);
    if (OSAL_INVALID_ID == priv->tsk_status.m_task_id)
    {
        return OSAL_E_FAIL;
    }
    nim_task_num++;

    return SUCCESS;
}


