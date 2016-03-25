/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_m3501_im.c
*
*    Description:  m3501 im layer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#include "nim_m3501.h"


#define LEN_VAL_7           7
#define CNT_VAL_10          10
#define CRYSTAL_FREQ_15000  15000
#define SYM_1800            1800
#define SYM_2000            2000
#define SYM_3000            3000
#define SYM_5000            5000
#define SYM_10000           10000
#define SYM_20000           20000
#define FFT_POINT_1024      1024
#define MAX_FFT_VAL_8388607 8388607
#define LOOP_NUM_2          2
#define EST_RREQ_945        945
#define EST_RREQ_2155       2155
#define RET_VALUE_2         2
#define TEMP_VAL_20         20
#define TEMP_VAL_30         30
#define STEP_FREQ_5         5
#define STEP_FREQ_30        30
#define STEP_FREQ_24        24
#define STEP_FREQ_40        40

typedef struct nim_autoscan_params_t
{
    struct nim_s3501_private *priv;
    NIM_AUTO_SCAN_T *pstauto_scan;
    UINT32 start_t;
    UINT32 end_t;
    INT32 adc_sample_freq;
    INT32 success;
    UINT32 fft_freq;
}NIM_AUTOSCAN_PARAMS;


UINT8 m3501_debug_flag = 0;



static UINT32 last_ch_fc = 0;
static UINT32 last_tp_freq = 0;
static UINT16  config_data = 0;




//--------Function get /check information---------
static INT32 nim_s3501_set_err(struct nim_device *dev);


INT32 nim_s3501_read(struct nim_device *dev, UINT8 bmemadr, UINT8 *pdata, UINT8 blen)
{
    INT32 err = 0;
    UINT8 chip_adr = 0 ;
    UINT32 i2c_type_id = 0;
	
    if((dev == NULL) || (pdata == NULL) || (blen == 0))
    {
	    return RET_FAILURE;
	}	
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    chip_adr = priv->ext_dm_config.i2c_base_addr;
    i2c_type_id = priv->ext_dm_config.i2c_type_id;

    pdata[0] = bmemadr;
    NIM_MUTEX_ENTER(priv);
    err = nim_i2c_write_read(i2c_type_id, chip_adr, pdata, 1, blen);
    NIM_MUTEX_LEAVE(priv);
    if (err)
    {
        if (priv->ul_status.m_pfn_reset_s3501)
        {
            priv->ul_status.m_pfn_reset_s3501((priv->tuner_id + 1) << 16);
            comm_delay(100);
            priv->t_param.t_i2c_err_flag = 0x01;
        }
        nim_s3501_set_err(dev);
        NIM_PRINTF("s3501 i2c read error = %d,chip_adr=0x%x,bmemadr=0x%x,I2C_FOR_S3501 = %d\n", (int)(-err), 
	             chip_adr, pdata[0], (int)i2c_type_id);
    }
    else
    {
        if (priv->t_param.t_i2c_err_flag)
        {
            NIM_PRINTF("t_i2c_err_flag==1\n");
            priv->t_param.t_i2c_err_flag = 0x00;
#if defined(__NIM_TDS_PLATFORM__)
            nim_s3501_open(dev);
#endif
        }
    }
    return err;
}


INT32 nim_s3501_write(struct nim_device *dev, UINT8 bmemadr, UINT8 *pdata, UINT8 blen)
{
    INT32 err = 0;
    UINT8 i = 0;
    UINT8 buffer[8]={0};
    UINT8 chip_adr  = 0;
    UINT32 i2c_type_id = 0;
	struct nim_s3501_private *priv =NULL;
	
    if((dev == NULL) || (pdata == NULL) || (blen == 0))
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    chip_adr = priv->ext_dm_config.i2c_base_addr;
    i2c_type_id = priv->ext_dm_config.i2c_type_id;

    if (blen > LEN_VAL_7)
    {
        nim_s3501_set_err(dev);
        return ERR_FAILUE;
    }
    buffer[0] = bmemadr;
    for (i = 0; i < blen; i++)
    {
        buffer[i + 1] = pdata[i];
    }
    NIM_MUTEX_ENTER(priv);
    err = nim_i2c_write(i2c_type_id, chip_adr, buffer, blen + 1);
    NIM_MUTEX_LEAVE(priv);
    if (err != 0)
    {
        if (priv->ul_status.m_pfn_reset_s3501)
        {
            priv->ul_status.m_pfn_reset_s3501((priv->tuner_id + 1) << 16);
            comm_delay(100);
            priv->t_param.t_i2c_err_flag = 0x01;
        }
        nim_s3501_set_err(dev);
        NIM_PRINTF("s3501 i2c write error = %d,chip_adr=0x%x,bmemadr=0x%x,I2C_FOR_S3501 = %d\n", 
			     (int)(-err), chip_adr, pdata[0], (int)i2c_type_id);
    }
    else
    {
        if (priv->t_param.t_i2c_err_flag)
        {
            NIM_PRINTF("t_i2c_err_flag==1\n");
            priv->t_param.t_i2c_err_flag = 0x00;
#if defined(__NIM_TDS_PLATFORM__)
            nim_s3501_open(dev);
#endif
        }
    }
    return err;
}




void nim_s3501_task(UINT32 param1, UINT32 param2)
{
    struct nim_device *dev =NULL;
	struct nim_s3501_private *priv =NULL;
	UINT32 v_cnt_val = 0x00;

#ifdef CHANNEL_CHANGE_ASYNC
    UINT32 flag_ptn = 0;

#endif	

	
    if(param1 == 0)
    {
	    return;
	}	
    dev = (struct nim_device *) param1 ;
    priv = (struct nim_s3501_private *) dev->priv ;
    
	
    
	NIM_PRINTF("Enter nim_s3501_task:\n");

    priv->tsk_status.m_sym_rate = 0x00;
    priv->tsk_status.m_code_rate = 0x00;
    priv->tsk_status.m_map_type = 0x00;
    priv->tsk_status.m_work_mode = 0x00;
    priv->tsk_status.m_info_data = 0x00;
    
    while (priv->work_alive)
    {
#ifdef CHANNEL_CHANGE_ASYNC

        flag_ptn = nim_flag_read(priv, NIM_FLAG_CHN_CHG_START, OSAL_TWF_ANDW | OSAL_TWF_CLR, 0);
        //NIM_PRINTF("Enter nim_s3501_task:flag_ptn=0x%x\n",flag_ptn);

        if((flag_ptn & NIM_FLAG_CHN_CHG_START) && (flag_ptn != OSAL_INVALID_ID))
        {
            // NIM_FLAG_SET(priv->flag_id, NIM_FLAG_CHN_CHANGING);
            //osal_mutex_lock(priv->m3501_mutex,OSAL_WAIT_FOREVER_TIME);

            nim_flag_clear(priv, NIM_FLAG_CHN_CHG_START);
            nim_flag_set(priv, NIM_FLAG_CHN_CHANGING);


            nim_s3501_waiting_channel_lock(dev, priv->cur_freq, priv->cur_sym);
            nim_flag_clear(priv, NIM_FLAG_CHN_CHANGING);

            //osal_mutex_unlock(priv->m3501_mutex);
            //NIM_FLAG_CLEAR(priv->flag_id, NIM_FLAG_CHN_CHANGING);
            NIM_PRINTF("\t\t Here is the task for M3501C wait channel lock \n");
        }
#endif
        if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
        {
            if((priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C) &&
                 (priv->tsk_status.m_lock_flag == NIM_LOCK_STUS_CLEAR))
            {
                if(v_cnt_val  == CNT_VAL_10)
                {
                    if(priv->ul_status.m_tso_mode == 1)
                    {
                        nim_m3501c_get_int(dev);
                    }
                    v_cnt_val = 0;
                    NIM_PRINTF("\t\t Here is the task for M3501C plug_in_out \n");
#ifdef HW_ADPT_CR
#ifdef HW_ADPT_CR_MONITOR
                    nim_hw_adaptive_cr_monitor(dev);
#endif
#else
#ifdef SW_ADPT_CR
#ifdef SW_SNR_RPT_ONLY
                    nim_sw_snr_rpt(dev);
#else
                    nim_sw_adaptive_cr(dev);
#endif
#endif
#endif

                }
                else
                {
                   v_cnt_val ++;
                }
            }
            else
            {
                if ((priv->tsk_status.m_lock_flag == NIM_LOCK_STUS_SETTING) && (priv->t_param.t_i2c_err_flag == 0x00))
                {
                    nim_s3501_get_lock(dev, &(priv->tsk_status.m_info_data));
                    if (priv->tsk_status.m_info_data && (priv->t_param.t_i2c_err_flag == 0x00))
                    {
                        nim_s3501_reg_get_symbol_rate(dev, &(priv->tsk_status.m_sym_rate));
                        nim_s3501_reg_get_code_rate(dev, &(priv->tsk_status.m_code_rate));
                        nim_s3501_reg_get_work_mode(dev, &(priv->tsk_status.m_work_mode));
                        nim_s3501_reg_get_map_type(dev, &(priv->tsk_status.m_map_type));
                        if ((priv->ul_status.m_enable_dvbs2_hbcd_mode == 0) &&
                                ((priv->tsk_status.m_map_type == 0) || (priv->tsk_status.m_map_type == 5)))
                        {
                            NIM_PRINTF("            Demod Error: wrong map_type is %d\n", priv->tsk_status.m_map_type);
                        }
                        else
                        {
                            if((priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C) &&
                                   (priv->ul_status.m_tso_mode == 1))
                            {
                                if((priv->tsk_status.m_work_mode) == M3501_DVBS2_MODE)
                                {
                                    nim_m3501c_open_dummy(dev);
                                }
                                else
                                {
                                    nim_m3501c_close_dummy(dev);
                                }
#ifdef C3501C_ERRJ_LOCK
                                nim_m3501c_recover_moerrj(dev);
#endif
                                nim_s3501_clear_int(dev);
                            }
                            else
                            {
                                nim_s3501_set_ts_mode(dev, priv->tsk_status.m_work_mode, priv->tsk_status.m_map_type,
                                      priv->tsk_status.m_code_rate,
                                                      priv->tsk_status.m_sym_rate, 0x1);
                                // open TS
                                nim_m3501c_fec_ts_on(dev);
                                nim_m3501_ts_on(dev);
                                priv->tsk_status.m_info_data = priv->tsk_status.m_info_data | 0x80;    // ts open
                                nim_s3501_clear_int(dev);
                            }
                            priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_CLEAR;
                        }
                    }
                }
            }
        }
        else
        {
            break;
        }
        comm_sleep(100);
    }
}


INT32 nim_s3501_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec)
{
    struct nim_s3501_private *priv =NULL;
    
    UINT8 data = 0x10;
    UINT8 lock = 200;
    UINT8 low_sym = 0;

    UINT8 tabid = 0;
    UINT16 tabval = 0;
    UINT16 tabvaltemp = 0;
    UINT8 datarray[2]={0};

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;
#ifdef CHANNEL_CHANGE_ASYNC
    UINT32 flag_ptn = 0;
#endif

    NIM_PRINTF("Enter nim_s3501_channel_change : freq=%d, sym=%d, fec=%d \n", (int)freq, (int)sym, fec);

    priv->t_param.t_phase_noise_detected = 0;
    priv->t_param.t_dynamic_power_en = 0;
    priv->t_param.t_last_snr = -1;
    priv->t_param.t_last_iter = -1;
    priv->t_param.t_aver_snr = -1;
    priv->t_param.t_snr_state = 0;
    priv->t_param.t_snr_thre1 = 256;
    priv->t_param.t_snr_thre2 = 256;
    priv->t_param.t_snr_thre3 = 256;
    priv->t_param.phase_noise_detect_finish = 0x00;
    priv->ul_status.m_tso_status = NIM_TSO_STUS_UNLOCK;

#ifdef CHANNEL_CHANGE_ASYNC
    flag_ptn = nim_flag_read(priv, NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING, OSAL_TWF_ORW, 0);
    if((flag_ptn & (NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING)) && (flag_ptn != OSAL_INVALID_ID))
        
    {
        // channel chaning, stop the old changing first.
        priv->ul_status.s3501_chanscan_stop_flag = 1;
        comm_sleep(2);
    }
#endif

    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        priv->ul_status.phase_err_check_status = 0;
        priv->ul_status.s3501d_lock_status = NIM_LOCK_STUS_NORMAL;
    }

    priv->ul_status.m_setting_freq = freq;

    //reset
    nim_s3501_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);

    if(CRYSTAL_FREQ_15000 >= CRYSTAL_FREQ)
    {
        //if(sym<44000)
        if(sym < (CRYSTAL_FREQ * 440 / 135))
        {
           nim_s3501_set_dsp_clk (dev, 0);
        }
        else
        {
           nim_s3501_set_dsp_clk (dev, 3);
        }
    }
    else
    {
        nim_s3501_set_dsp_clk (dev, 0);
    }

    if ((0 == freq) || (0 == sym))
    {
        return SUCCESS;
    }

    nim_s3501_sym_config(dev, sym);

#if 0
    if (priv->ul_status.s3501_chanscan_stop_flag)
    {
        priv->ul_status.s3501_chanscan_stop_flag = 0;
        return SUCCESS;
    }
#endif

    nim_s3501_tr_cr_setting(dev, NIM_OPTR_CHL_CHANGE);

    low_sym = sym < 6500 ? 1 : 0;   /* Symbol rate is less than 10M, low symbol rate */

    nim_s3501_freq_offset_set(dev, low_sym, &freq);

    if (nim_s3501_i2c_open(dev))
    {
        return S3501_ERR_I2C_NO_ACK;
    }

    if (priv->nim_tuner_control != NULL)
    {
        priv->nim_tuner_control(priv->tuner_id, freq, sym);
    }

    lock = 200;
    comm_sleep(1);
    if (priv->nim_tuner_status != NULL)
    {
        priv->nim_tuner_status(priv->tuner_id, &lock);
    }

    if (nim_s3501_i2c_close(dev))
    {
        return S3501_ERR_I2C_NO_ACK;
    }


    data = 0x10;
    nim_reg_write(dev, RB3_PIN_SHARE_CTRL, &data, 1);
    nim_s3501_adc_setting(dev);

    nim_s3501_interrupt_mask_clean(dev);

    nim_s3501_set_hw_timeout(dev, 0xff);

    nim_s3501_agc1_ctrl(dev, low_sym, NIM_OPTR_CHL_CHANGE);

    nim_s3501_set_rs(dev, sym);

    nim_s3501_freq_offset_reset(dev, low_sym);

    nim_s3501_cr_setting(dev, NIM_OPTR_CHL_CHANGE);

    nim_s3501_set_acq_workmode(dev, NIM_OPTR_CHL_CHANGE0);

    nim_s3501_set_fc_search_range(dev, NIM_OPTR_CHL_CHANGE, sym);
    nim_s3501_rs_search_range(dev, NIM_OPTR_CHL_CHANGE, sym);

    nim_s3501_ldpc_setting(dev, NIM_OPTR_CHL_CHANGE, 0x00, 0x01);

    nim_s3501_hbcd_timeout(dev, NIM_OPTR_CHL_CHANGE);

    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if((priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C) && (priv->ul_status.m_tso_mode == 1))
        {
#ifdef C3501C_ERRJ_LOCK
            nim_m3501c_invert_moerrj(dev);
#endif
            nim_m3501c_fec_ts_off(dev);
            nim_m3501c_reset_tso(dev);
        }
        else
        {
            nim_m3501c_fec_ts_off(dev);
            nim_set_ts_rs(dev, sym);
            //when enter channel change, first close ts dummy.
            nim_close_ts_dummy(dev);
            //ECO_TS_EN = reg_cr9e[7], disable before dmy config successfully.
            nim_m3501_ts_off(dev);
        }
    }
    else if(priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501A)
    {
        //M3501A config.
        nim_s3501_set_ts_mode(dev, 0x0, 0x0, 0x0, 0x0, 0X1);
    }
    else
    {
        NIM_PRINTF("ERROR: CHIP_ID_type=0x%0x \n", (unsigned int)priv->ul_status.m_s3501_type);
    }

    comm_delay(10);

    if (sym < SYM_3000)
    {
        if (sym < SYM_2000)
        {
           data = 0x08;
        }
        else
        {
           data = 0x0a;
        }
        nim_reg_write(dev, R1B_TR_TIMEOUT_BAND, &data, 1);
    }

#ifdef SW_ADPT_CR
    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B &&
         priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C)
    {
        s3501_snr_initial_en = 1;
    }
#endif

#ifdef HW_ADPT_CR
    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B &&
           priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C)
    {
        data = 0xe1;
        nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);

        data = 0xf0 | 2;
        nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
        if((sym > SYM_3000) && (sym <= SYM_10000)) //add(sym<=3000)for 1M QPSK AWGN performance issue
        {
           datarray[0] = 0x02;
        }
        else
        {
           datarray[0] = 0x00;
        }
        datarray[1] = 0x10;
        nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

        data = 0xf0 | 3;
        nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
        if(sym <= SYM_1800)
        {
           tabval = 0x3F8;
        }
        else
        {
           tabval = 0x1F8;
        }
        datarray[0] = (UINT8)(tabval & 0x0ff);
        datarray[1] = (UINT8)(((tabval >> 8) & 0x0ff) | 0x10);
        nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

        if(sym <= SYM_5000)
        {
           tabval = 0x1280;
        }
        else
        {
           tabval = 0x1290;
        }

        tabvaltemp = (tabval & 3) << 9;
        tabval = (tabval >> 4) | tabvaltemp; // to HW format
        datarray[0] = (UINT8)(tabval & 0x0ff);
        datarray[1] = (UINT8)(((tabval >> 8) & 0x0ff) | 0x10);

        for(tabid = 2; tabid <= 7; tabid++)
        {
            data = (tabid << 4) | (TAB_SIZE - 1);
            nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
            nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

            data = (tabid << 4) | (TAB_SIZE - 2);
            nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
            nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

            data = (tabid << 4) | (TAB_SIZE - 3);
            nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
            nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);
        }

#ifdef HW_ADPT_CR_MONITOR
        data = 0xe6; // enable hw adpt CR report, also enable always update to regfile, latter need review
#else
        data = 0xe0; // CR Tab init off
#endif

        nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);
    }
#endif


    nim_s3501_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);

#if defined CHANNEL_CHANGE_ASYNC && !defined NIM_S3501_ASCAN_TOOLS
    priv->cur_freq = freq;
    priv->cur_sym = sym;
    // NIM_FLAG_SET(priv->flag_id, NIM_FLAG_CHN_CHG_START);
    nim_flag_set(priv, NIM_FLAG_CHN_CHG_START);
#else
    nim_s3501_waiting_channel_lock(dev, freq, sym);
#endif

    priv->ul_status.s3501_chanscan_stop_flag = 0;
    NIM_PRINTF("Leave nim_s3501_channel_change\n");

    return SUCCESS;
}




#ifdef AUTOSCAN_FULL_SPECTRUM

static INT32 nim_s3501_autoscan_ts_set(struct nim_device *dev)
{
	UINT8 data = 0;
	struct nim_s3501_private *priv = NULL;
	
    if(dev == 0)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;
    nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
    data = data & 0xef;
    nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);

    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if((priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C) && (priv->ul_status.m_tso_mode == 1))
        {
#ifdef C3501C_ERRJ_LOCK
            nim_m3501c_recover_moerrj(dev);
#endif
            nim_m3501c_close_dummy(dev);
        }
        else if ((priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_S3501D))
        {
            //S3501D
            if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_1BIT_MODE)
            {
                data = 0x06; // symbol period from reg, 2 cycle
                nim_reg_write(dev, RAD_TSOUT_SYMB, &data, 1);
                NIM_PRINTF("open ci plus enable REG_ad = %02x \n", data);

                nim_reg_read(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
                data = data | 0x80;    // enable symbol period from reg
                nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);

                nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
                data = data | 0xe0;
                nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);

                data = 0x1d;  //use 98M clk and enable ci+
                nim_reg_write(dev, RDF_TS_OUT_DVBS2, &data, 1);

                data = 0x0f;
                nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
            }
            else
            {
                //for S3501D 2bit/4bit/8bit auto search, close dummy.
                nim_close_ts_dummy(dev);
            }
        }
        else
        {
            if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_8BIT_MODE)
            {
                nim_close_ts_dummy(dev);
            }
            else
            {
                //open CI+ first.
                data = 0x02;    // symbol period from reg, 2 cycle
                nim_reg_write(dev, RAD_TSOUT_SYMB, &data, 1);
                NIM_PRINTF("open ci plus enable REG_ad = %02x \n", data);

                nim_reg_read(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
                data = data | 0x80;    // enable symbol period from reg
                nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);

                nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
                data = data | 0xe0;
                nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);

                data = 0x1d;  //use 98M clk and enable ci+
                nim_reg_write(dev, RDF_TS_OUT_DVBS2, &data, 1);

                nim_reg_read(dev, RD8_TS_OUT_SETTING, &data, 1);
                data = (data & 0x08) | 0x07;
                nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);

                if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_2BIT_MODE)
                {
                    //ECO_SSI_2B_EN = cr9f[3]
                    nim_reg_read(dev, R9C_DEMAP_BETA + 0x03, &data, 1);
                    data = data | 0x08;
                    nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);

                    //ECO_SSI_SEL_2B = crc0[3]
                    nim_reg_read(dev, RC0_BIST_LDPC_REG, &data, 1);
                    data = data | 0x08;
                    nim_reg_write(dev, RC0_BIST_LDPC_REG, &data, 1);
                }
                else if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_4BIT_MODE)
                {
                    //ECO_SSI_2B_EN = cr9f[3]
                    nim_reg_read(dev, R9C_DEMAP_BETA + 0x03, &data, 1);
                    data = data | 0x08;
                    nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);

                    //ECO_SSI_SEL_2B = crc0[3]
                    nim_reg_read(dev, RC0_BIST_LDPC_REG, &data, 1);
                    data = data & 0xf7;
                    nim_reg_write(dev, RC0_BIST_LDPC_REG, &data, 1);
                }
            }
            //open eco_ts_en
            nim_m3501c_fec_ts_on(dev);
            nim_m3501_ts_on(dev);
        }//end M3501B
    }

    return SUCCESS;
}

static INT32 nim_s3501_autoscan_init(struct nim_device *dev,NIM_AUTOSCAN_PARAMS *params)
{
	UINT8 data = 0;
	INT32 ret=RET_CONTINUE;

#ifdef CHANNEL_CHANGE_ASYNC
    UINT32 flag_ptn = 0;
#endif

    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);

    if((dev == 0) || (params == 0))
    {
	    return RET_FAILURE;
	}	

    last_tp_freq = 0;
    params->start_t = os_get_tick_count();

#ifdef AUTOSCAN_DEBUG
    config_data = params->priv->tuner_config_data.qpsk_config;
    params->priv->tuner_config_data.qpsk_config |= M3501_SIGNAL_DISPLAY_LIN;
#endif

#ifdef CHANNEL_CHANGE_ASYNC
    flag_ptn = nim_flag_read(params->priv, NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING, OSAL_TWF_ORW, 0);
    if((flag_ptn & (NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING)) && (flag_ptn != OSAL_INVALID_ID))
    {
        params->priv->ul_status.s3501_chanscan_stop_flag = 1;
        comm_sleep(10);
    }
#endif

    if (params->priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        params->priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_NORMAL;
    }

    if (params->pstauto_scan->sfreq < AS_FREQ_MIN || params->pstauto_scan->efreq > AS_FREQ_MAX ||
                params->pstauto_scan->sfreq > params->pstauto_scan->efreq)
    {
        return ERR_FAILUE;
    }

#ifdef __NIM_TDS_PLATFORM__
    nim_s3501_ioctl(dev, NIM_DRIVER_STOP_ATUOSCAN, 0);
#endif

#ifdef __NIM_LINUX_PLATFORM__
    if (params->priv->ul_status.s3501_autoscan_stop_flag)
    {
        nim_send_as_msg(params->priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
        params->priv->ul_status.s3501_autoscan_stop_flag = 0;
        return SUCCESS;
    }
#endif
    //-------- soft search configure
    data = 0x1f;
    nim_reg_write(dev, R1B_TR_TIMEOUT_BAND, &data, 1);
    nim_s3501_cr_setting(dev, NIM_OPTR_SOFT_SEARCH);

    if(CRYSTAL_FREQ > CRYSTAL_FREQ_15000)
    {
        data = 0xf3;
    }
    else
    {
        data = 0x73;
    }
    nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);

    nim_s3501_tr_cr_setting(dev, NIM_OPTR_SOFT_SEARCH);
    nim_s3501_ldpc_setting(dev, NIM_OPTR_SOFT_SEARCH, 0x00, 0x01);

    nim_s3501_hbcd_timeout(dev, NIM_OPTR_SOFT_SEARCH);

    nim_s3501_autoscan_ts_set(dev);

    last_tuner_if = 0;
    chlspec_num = 0;
    called_num = 0;
    max_fft_energy = 0;

    channel_spectrum = (INT32 *) comm_malloc(FS_MAXNUM * 4);
    if(channel_spectrum == NULL)
    {
        AUTOSCAN_PRINTF("\n channel_spectrum--> no enough memory!\n");
        return ERR_NO_MEM;
    }

    channel_spectrum_tmp = (INT32 *) comm_malloc(FS_MAXNUM * 4);
    if(channel_spectrum_tmp == NULL)
    {
        AUTOSCAN_PRINTF("\n channel_spectrum_tmp--> no enough memory!\n");
        comm_free(channel_spectrum);
        return ERR_NO_MEM;
    }

    if (nim_s3501_get_bypass_buffer(dev))
    {
        AUTOSCAN_PRINTF("\n ADCdata--> no enough memory!\n");
        comm_free(channel_spectrum);
        comm_free(channel_spectrum_tmp);
        return ERR_NO_MEM;
    }


    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;

}


static INT32 nim_s3501_fft_scan(struct nim_device *dev,NIM_AUTOSCAN_PARAMS *params)
{
    INT32 i = 0;
    INT32 ret=RET_CONTINUE;
    UINT32 step_freq = 0;
    INT32 scan_mode = 0;
    UINT32 temp_t = 0;

    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);

    if(dev == 0 || params == 0)
    {
	    return RET_FAILURE;
	}	

    if(params->priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501A)
    {
        params->adc_sample_freq = 98;
    }
    else
    {
        params->adc_sample_freq = ((CRYSTAL_FREQ * 90 + 6750) / 13500);
    }
    params->adc_sample_freq = params->adc_sample_freq - (params->adc_sample_freq % 2);
    AUTOSCAN_PRINTF("=========>>adc_sample_freq=%d\n", (int)params->adc_sample_freq);
    step_freq = params->adc_sample_freq / 2;
    //    step_freq=(adc_sample_freq+2)/4;
    for (params->fft_freq = params->pstauto_scan->sfreq; params->fft_freq < params->pstauto_scan->efreq; \
           params->fft_freq += step_freq)
    {
        nim_s3501_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
        comm_memset(fft_energy_1024, 0, sizeof(fft_energy_1024));
        while(1)
        {
            nim_s3501_cap_start(dev, params->fft_freq, 0, fft_energy_1024);

            for(i = 0; i < 1024; i++)
            {
                if(fft_energy_1024[i] > 0)
                {
                    break;
                }
            }
            if(i < FFT_POINT_1024)
            {
               break;
            }

            if(params->priv->ul_status.s3501_autoscan_stop_flag)
            {
                comm_free(channel_spectrum);
                comm_free(channel_spectrum_tmp);
#if defined(__NIM_TDS_PLATFORM__)
                comm_free(params->priv->ul_status.adcdata_malloc_addr);
#endif

#if defined(__NIM_LINUX_PLATFORM__)
                nim_send_as_msg(params->priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
                params->priv->ul_status.s3501_autoscan_stop_flag = 0;
#endif

                AUTOSCAN_PRINTF("nim_s3501_autoscan:stop_flag=1\n");
                return SUCCESS;
            }
        }
        called_num++;
        if(1 == ((m3501_debug_flag >> 1) & 0x01))
        {
            AUTOSCAN_PRINTF("call_num=%d,Tuner_IF=%d \n", (int)called_num, (int)params->fft_freq);
            AUTOSCAN_PRINTF("FFT1024-> 0000 : 1000000 \n");
            for(i = 0; i < 1024; i++)
            {
	        AUTOSCAN_PRINTF("FFT1024-> %4d : %d \n", (int)i, (int)fft_energy_1024[i]);
             }
        }

        if(NIM_SCAN_SLOW == params->priv->blscan_mode)
        {
            scan_mode = 1;
        }
        else
        {
            scan_mode = 0;
        }
        nim_s3501_median_filter(1024, fft_energy_1024, scan_mode); // Median filter for restrain single tone noise
        nim_s3501_fft_wideband_scan(params->fft_freq, params->adc_sample_freq);

        if(params->priv->ul_status.s3501_autoscan_stop_flag)
        {
            comm_free(channel_spectrum);
            comm_free(channel_spectrum_tmp);
#if defined(__NIM_TDS_PLATFORM__)
            comm_free(params->priv->ul_status.adcdata_malloc_addr);
#endif

#if defined(__NIM_LINUX_PLATFORM__)
            nim_send_as_msg(params->priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
            params->priv->ul_status.s3501_autoscan_stop_flag = 0;
#endif

            AUTOSCAN_PRINTF("nim_s3501_autoscan:stop_flag=1\n");
            return SUCCESS;
        }
    }
#if defined(__NIM_TDS_PLATFORM__)
    comm_free(params->priv->ul_status.adcdata_malloc_addr);
#endif



    params->end_t = os_get_tick_count();
    temp_t = params->end_t;
    AUTOSCAN_PRINTF("\tWideband FFT cost time %dms\n", (int)(params->end_t - params->start_t));
	
    if(1 == ((m3501_debug_flag >> 2) & 0x01))
    {
        for(i = 0; i < chlspec_num; i++)
        {
	    AUTOSCAN_PRINTF("FFT_WBS-->%d : %d\n", (int)i, (int)channel_spectrum[i]);
        }  
    }
    params->end_t = os_get_tick_count();
    AUTOSCAN_PRINTF("\tPrint FFT result cost time %dms\n", (int)(params->end_t - temp_t));


    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;
}



static INT32 nim_s3501_find_tp(struct nim_device *dev,NIM_AUTOSCAN_PARAMS *params)	
{
    INT32 loop = 1;
    UINT32 temp_t = 0;
    INT32 i = 0;
    INT32 loop_index = 0;
    INT32 ret=RET_CONTINUE;
#ifdef DEBUG_SOFT_SEARCH    
    INT32 bug_freq[2]={0};
    INT32 bug_sym[2]={0};
#endif

    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);

    if(dev == 0 || params == 0)
    {
	    return RET_FAILURE;
	}	

    temp_t = params->end_t;
    AUTOSCAN_PRINTF("Max_FFT_energy=%d\n", (int)max_fft_energy);
    loop = 1;
    if(max_fft_energy > MAX_FFT_VAL_8388607)
    {
        loop = 2;
    }
    AUTOSCAN_PRINTF("************loop=%d****************\n", (int)loop);
    if(loop == LOOP_NUM_2)
    {
        for(i = 0; i < chlspec_num; i++)
        {
            channel_spectrum_tmp[i] = channel_spectrum[i] / (8388607 >> 5);
            if(channel_spectrum_tmp[i] <= 0)
            {
            channel_spectrum_tmp[i] = 1;
            }
            else if(channel_spectrum_tmp[i] > 8388607)
            {
            channel_spectrum_tmp[i] = 8388607;
            }

            if (channel_spectrum[i] > MAX_FFT_VAL_8388607)  // max 23bit = 0x7fffff
            {
            channel_spectrum[i] = 8388607;
            }
            else if(channel_spectrum[i] == 0)
            {
            channel_spectrum[i] = 1;
            }
        }
    }

    //autosearch
    tp_number = 0;
    comm_memset(frequency_est, 0, sizeof(frequency_est));
    comm_memset(symbol_rate_est, 0, sizeof(symbol_rate_est));

    for(loop_index = 0; loop_index < loop; loop_index++)
    {
        if(loop_index == 0)
        {
            nim_s3501_search_tp(chlspec_num, channel_spectrum, params->pstauto_scan->sfreq, 
				               params->adc_sample_freq, loop_index);
            AUTOSCAN_PRINTF("Time %d : Find TP number is %d\n", (int)loop_index, (int)tp_number);
        }
        else if(loop_index == 1)
        {
            nim_s3501_search_tp(chlspec_num, channel_spectrum_tmp, params->pstauto_scan->sfreq, 
				               params->adc_sample_freq, loop_index);
            AUTOSCAN_PRINTF("time %d : Find TP number is %d\n", (int)loop_index, (int)tp_number);
        }
    }
    comm_free(channel_spectrum);
    comm_free(channel_spectrum_tmp);
    params->end_t = os_get_tick_count();
    AUTOSCAN_PRINTF("\tSearch TP cost time %dms\n", (int)(params->end_t - temp_t));

#ifdef DEBUG_SOFT_SEARCH
    params->success = 0;
    tp_number = 1000;
    bug_freq[0] = 1887904;
    bug_sym[0] = 1875;
    bug_freq[1] = 1762288;
    bug_sym[1] = 1968;
    comm_memset(frequency_est, 0, sizeof(frequency_est));
    comm_memset(symbol_rate_est, 0, sizeof(symbol_rate_est));
    for(i = 0; i < tp_number; i++)
    {
        frequency_est[i] = bug_freq[i % 2];
        symbol_rate_est[i] = bug_sym[i % 2];
    }
#endif

    if(tp_number > 0)
    {
        params->success = 0;
    }
    AUTOSCAN_PRINTF("success = %d , TP_number = %d  \n", (int)params->success, (int)tp_number);


    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;
}


static INT32 nim_s3501_get_tp(struct nim_device *dev,NIM_AUTOSCAN_PARAMS *params)	
{
    UINT32 last_lock_rs = 0;
    INT32 last_lock_freq = 0;
    UINT32 temp_t = 0;
    INT32 i = 0;
    INT32 temp_freq = 0;
    UINT8 cur_fec = 0;
    UINT32 cur_freq = 0;
    UINT32 cur_sym = 0;
    INT32  ret_value = SUCCESS;
    INT32 ret=RET_CONTINUE;

    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);

    if(dev == 0 || params == 0)
    {
	    return RET_FAILURE;
	}	

   if (params->success == 0)
    {
        last_lock_freq = 0;
        last_lock_rs = 0;
        final_est_freq = 0;
        final_est_sym_rate = 0;
        for (i = 0; i < tp_number; i++)
        {
            AUTOSCAN_PRINTF("\n\n----> Try TP [freq,Sym]=[%d, %d]\n", (int)frequency_est[i], (int)symbol_rate_est[i]);
            if (params->priv->ul_status.s3501_autoscan_stop_flag)
            {
#if defined(__NIM_LINUX_PLATFORM__)
                nim_send_as_msg(params->priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
                params->priv->ul_status.s3501_autoscan_stop_flag = 0;
#endif
                AUTOSCAN_PRINTF("nim_s3501_autoscan:stop_flag=1\n");
                return SUCCESS;
            }

#ifdef DEBUG_SOFT_SEARCH
            params->priv->ul_status.m_crnum = 1;
            params->priv->ul_status.m_freq[0] = frequency_est[i];
            params->priv->ul_status.m_rs[0] = symbol_rate_est[i];
#else
            if(frequency_est[i] / 1024 < EST_RREQ_945)
            {
               continue;
            }
            if(frequency_est[i] / 1024 > EST_RREQ_2155)
            {
               continue;
            }
            temp_t = os_get_tick_count();
            temp_freq = (frequency_est[i] + 512) / 1024 - last_lock_freq;
            if(temp_freq == 0)
            {
               continue;
            }
            if (temp_freq < 0)
            {
            temp_freq = -temp_freq;
            }
            if ((symbol_rate_est[i] + 1000) / 2000 > temp_freq)
            {
               continue;
            }
            if ((last_lock_rs + 1000) / 2000 > (UINT32)temp_freq)
            {
               continue;
            }
            params->priv->ul_status.m_crnum = 1;
            params->priv->ul_status.m_freq[0] = frequency_est[i];
            params->priv->ul_status.m_rs[0] = symbol_rate_est[i];
#endif

            if (SUCCESS == nim_s3501_channel_search(dev, 0))
            {
                nim_s3501_get_code_rate(dev, &cur_fec);
                cur_freq = final_est_freq;
                cur_sym = final_est_sym_rate;
                last_lock_freq = cur_freq;
                last_lock_rs = cur_sym;
                if ((cur_freq >= params->pstauto_scan->sfreq) && (cur_freq <= params->pstauto_scan->efreq))
                {
                    //>>> Unicable begin
                    if (params->pstauto_scan->unicable) //cur_freq -> [950, 2150]
                    {
                        AUTOSCAN_PRINTF("\tUnicable fft_freq: %d, cur_freq: %d, Fub: %d\n", (int)params->fft_freq, 
			                  (int)cur_freq, params->pstauto_scan->fub);
                        ret_value = nim_callback(params->pstauto_scan, params->priv, 1, 0, 
							        params->fft_freq + cur_freq - params->pstauto_scan->fub, cur_sym, cur_fec, 0);
                    }
                    else    //<<< Unicable end
                    {
                        if((cur_freq > last_tp_freq - 2) && (cur_freq < last_tp_freq + 2))
                        {
                            ret_value = nim_callback(params->pstauto_scan, params->priv, 0, 0, 
								               frequency_est[i] / 1024, 0, 0, 0);
                            AUTOSCAN_PRINTF("\tRescan a tp: fft_freq: %d, last_tp_freq: %d\n", 
			                  (int)(frequency_est[i] / 1024), (int)last_tp_freq);
                        }
                        else
                        {
                            ret_value = nim_callback(params->pstauto_scan, params->priv, 1, 0,
                                                 cur_freq, cur_sym, cur_fec, 0);
                            last_tp_freq = cur_freq;
                        }

                    }
                }
                else
                {
                    if (params->priv->ul_status.s3501_autoscan_stop_flag)
                    {
#if defined(__NIM_LINUX_PLATFORM__)
                        nim_send_as_msg(params->priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
                        params->priv->ul_status.s3501_autoscan_stop_flag = 0;
#endif
                        AUTOSCAN_PRINTF("nim_s3501_autoscan:stop_flag=1\n");
                        return SUCCESS;
                    }
                    else
                    {
                        continue ;
                    }
                }
            }
            else
            {
                ret_value = nim_callback(params->pstauto_scan, params->priv, 0, 0,
                                     frequency_est[i] / 1024, 0, 0, 0);
            }
            if (ret_value == RET_VALUE_2)
            {
                return SUCCESS;
            }
            else if (ret_value == 1)
            {
                nim_callback(params->pstauto_scan, params->priv, 2, 0, 0, 0, 0, 0); /* Tell callback search finished */

#if defined(__NIM_LINUX_PLATFORM__)
                nim_send_as_msg(params->priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
                params->priv->ul_status.s3501_autoscan_stop_flag = 0;
#endif
			    nim_s3501_autoscan_signal_input(dev, NIM_SIGNAL_INPUT_CLOSE);

            }
            params->end_t = os_get_tick_count();
            AUTOSCAN_PRINTF("\tTP %d cost time %dms\n", (int)i, (int)(params->end_t - temp_t));
        }
    }



    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;

}

INT32 nim_s3501_autoscan(struct nim_device *dev, NIM_AUTO_SCAN_T *pstauto_scan)
{
    INT32 ret=RET_CONTINUE;
    NIM_AUTOSCAN_PARAMS params;
	struct nim_s3501_private *priv = NULL;
	
    if(dev == 0 || pstauto_scan == 0)
    {
	    return RET_FAILURE;
	}	
	
    priv = (struct nim_s3501_private *) dev->priv;


    NIM_PRINTF("Enter nim_s3501_autoscan:\n");
    AUTOSCAN_PRINTF("Enter nim_s3501_autoscan:sfreq = %d, efreq = %d\n", pstauto_scan->sfreq,
                                                                        pstauto_scan->efreq);
    comm_memset(&params,0,sizeof(NIM_AUTOSCAN_PARAMS));
    params.priv=priv;
    params.pstauto_scan=pstauto_scan;

    ret=nim_s3501_autoscan_init(dev,&params);
    if(ret!=RET_CONTINUE)
    {
        return ret;
    }

    ret=nim_s3501_fft_scan(dev,&params);
    if(ret!=RET_CONTINUE)
    {
        return ret;
    }

    ret=nim_s3501_find_tp(dev,&params);
    if(ret!=RET_CONTINUE)
    {
        return ret;
    }

    ret=nim_s3501_get_tp(dev,&params);
    if(ret!=RET_CONTINUE)
    {
        return ret;
    }




#ifdef AUTOSCAN_DEBUG
    priv->tuner_config_data.qpsk_config = config_data;
#endif

    params.end_t = os_get_tick_count();
    AUTOSCAN_PRINTF("\tautoscan cost time   %d:%d   \n", (int)(((params.end_t - params.start_t) + 500) / 1000 / 60), 
   (int)(((params.end_t - params.start_t) + 500) / 1000 - (((params.end_t - params.start_t) + 500) / 1000 / 60) * 60));
    NIM_PRINTF("Leave nim_s3501_autoscan\n");

#if defined(__NIM_LINUX_PLATFORM__)
    nim_send_as_msg(priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
    priv->ul_status.s3501_autoscan_stop_flag = 0;
#endif

    return SUCCESS;

}
#else

INT32 nim_s3501_autoscan(struct nim_device *dev, NIM_AUTO_SCAN_T *pstauto_scan)
{
    UINT8 cur_fec = 0;
    UINT32 cur_freq = 0;
	UINT32 cur_sym = 0;
    INT32 i = 0;
	INT32 ret = SUCCESS;
    INT32 crnum = 0;
    UINT32 fft_freq = 0; 
	UINT32 step_freq = 0;
    UINT32 fft_result = 0;
    UINT8  data = 0;
    UINT8 chcnt = 0;
	UINT8 double_i = 0;
    UINT32 temp = 0;
	UINT32 temp_step = 0;
    UINT32 sym = 0;
	UINT32 freq = 0;
	struct nim_s3501_private *priv =NULL;
    struct nim_s3501_private *dev_priv = NULL;

    if((dev == NULL) || (pstauto_scan == NULL))
    {
	    return RET_FAILURE;
	}	
        
	priv = (struct nim_s3501_private *) dev->priv;
	dev_priv = dev->priv;

    last_ch_fc = 0;
    last_tp_freq = 0;

    AUTOSCAN_PRINTF("\napi_nim_autoscan: Begin autoscan\n");

#ifdef NIM_S3501_ASCAN_TOOLS
    //pstauto_scan->sfreq=vs_s3501_ascan.va_ascan_g6_scan_f_start;
    //pstauto_scan->efreq=vs_s3501_ascan.va_ascan_g6_scan_f_end;
#endif
    AUTOSCAN_PRINTF("\tsfreq = %d, efreq = %d\n", pstauto_scan->sfreq, pstauto_scan->efreq);
#ifdef AUTOSCAN_DEBUG
    config_data = priv->tuner_config_data.qpsk_config;
    priv->tuner_config_data.qpsk_config |= M3501_SIGNAL_DISPLAY_LIN;
#endif

#ifdef CHANNEL_CHANGE_ASYNC
    UINT32 flag_ptn = 0;
    flag_ptn = nim_flag_read(priv, NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING, OSAL_TWF_ORW, 0);
    if((flag_ptn & (NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING)) && (flag_ptn != OSAL_INVALID_ID))
    {
        priv->ul_status.s3501_chanscan_stop_flag = 1;
        comm_sleep(10);
    }
#endif

    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
        priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_NORMAL;

    if (pstauto_scan->sfreq < AS_FREQ_MIN || pstauto_scan->efreq > AS_FREQ_MAX ||
                               pstauto_scan->sfreq > pstauto_scan->efreq)
    {
        return ERR_FAILUE;
    }

#ifdef __NIM_TDS_PLATFORM__
    nim_s3501_ioctl(dev, NIM_DRIVER_STOP_ATUOSCAN, 0);
#endif
#ifdef __NIM_LINUX_PLATFORM__
    if (priv->ul_status.s3501_autoscan_stop_flag)
    {
        nim_send_as_msg(priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
        priv->ul_status.s3501_autoscan_stop_flag = 0;
        return SUCCESS;
    }
#endif
    if (nim_s3501_get_bypass_buffer(dev))
        return ERR_NO_MEM;

    //-------- soft search configure
    data = 0x1f;
    nim_reg_write(dev, R1B_TR_TIMEOUT_BAND, &data, 1);
    nim_s3501_cr_setting(dev, NIM_OPTR_SOFT_SEARCH);

    if(CRYSTAL_FREQ > CRYSTAL_FREQ_15000)
        data = 0xf3;
    else
        data = 0x73;
    nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);

    nim_s3501_tr_cr_setting(dev, NIM_OPTR_SOFT_SEARCH);
    nim_s3501_ldpc_setting(dev, NIM_OPTR_SOFT_SEARCH, 0x00, 0x01);

    nim_s3501_hbcd_timeout(dev, NIM_OPTR_SOFT_SEARCH);

    nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
    data = data & 0xef;
    nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);

    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if((priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C) && (priv->ul_status.m_tso_mode == 1))
        {
#ifdef C3501C_ERRJ_LOCK
            nim_m3501c_recover_moerrj(dev);
#endif
            nim_m3501c_close_dummy(dev);
        }
        else if ((priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_S3501D))
        {
            //S3501D
            if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_1BIT_MODE)
            {
                data = 0x06; // symbol period from reg, 2 cycle
                nim_reg_write(dev, RAD_TSOUT_SYMB, &data, 1);
                NIM_PRINTF("open ci plus enable REG_ad = %02x \n", data);

                nim_reg_read(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
                data = data | 0x80;    // enable symbol period from reg
                nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);

                nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
                data = data | 0xe0;
                nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);

                data = 0x1d;  //use 98M clk and enable ci+
                nim_reg_write(dev, RDF_TS_OUT_DVBS2, &data, 1);

                data = 0x0f;
                nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
            }
            else
            {
                //for S3501D 2bit/4bit/8bit auto search, close dummy.
                nim_close_ts_dummy(dev);
            }
        }
        else
        {
            if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_8BIT_MODE)
            {
                nim_close_ts_dummy(dev);
            }
            else
            {
                //open CI+ first.
                data = 0x02;    // symbol period from reg, 2 cycle
                nim_reg_write(dev, RAD_TSOUT_SYMB, &data, 1);
                NIM_PRINTF("open ci plus enable REG_ad = %02x \n", data);

                nim_reg_read(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
                data = data | 0x80;    // enable symbol period from reg
                nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);

                nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
                data = data | 0xe0;
                nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);

                data = 0x1d;  //use 98M clk and enable ci+
                nim_reg_write(dev, RDF_TS_OUT_DVBS2, &data, 1);
                nim_reg_read(dev, RD8_TS_OUT_SETTING, &data, 1);
                data = data & 0x08 | 0x07;
                nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);

                if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_2BIT_MODE)
                {
                    //ECO_SSI_2B_EN = cr9f[3]
                    nim_reg_read(dev, R9C_DEMAP_BETA + 0x03, &data, 1);
                    data = data | 0x08;
                    nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);

                    //ECO_SSI_SEL_2B = crc0[3]
                    nim_reg_read(dev, RC0_BIST_LDPC_REG, &data, 1);
                    data = data | 0x08;
                    nim_reg_write(dev, RC0_BIST_LDPC_REG, &data, 1);
                }
                else if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_4BIT_MODE)
                {
                    //ECO_SSI_2B_EN = cr9f[3]
                    nim_reg_read(dev, R9C_DEMAP_BETA + 0x03, &data, 1);
                    data = data | 0x08;
                    nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);

                    //ECO_SSI_SEL_2B = crc0[3]
                    nim_reg_read(dev, RC0_BIST_LDPC_REG, &data, 1);
                    data = data & 0xf7;
                    nim_reg_write(dev, RC0_BIST_LDPC_REG, &data, 1);
                }
            }
            //open eco_ts_en
            nim_m3501c_fec_ts_on(dev);
            nim_m3501_ts_on(dev);
        }//end M3501B
    }

    for (fft_freq = pstauto_scan->sfreq; fft_freq < pstauto_scan->efreq; fft_freq += step_freq)
    {
        nim_s3501_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
        nim_s3501_autoscan_signal_input(dev, NIM_SIGNAL_INPUT_OPEN);
        //>>> Unicable begin
        //only unicable need set here!
        if (pstauto_scan->unicable)
        {
            ret = nim_callback(pstauto_scan, priv, 5, 0, fft_freq, 0, 0, 0);
            if (1 == ret)
            {
                return 1;
            }
            else if (ret == 2)
            {
                goto nim_as_break;
            }
            fft_result = nim_s3501_fft(dev, pstauto_scan->fub);
        }
        else
        {
            //<<< Unicable end
            fft_result = nim_s3501_fft(dev, fft_freq);
        }
        nim_s3501_autoscan_signal_input(dev, NIM_SIGNAL_INPUT_CLOSE);

        if(dev_priv->ul_status.s3501_autoscan_stop_flag)
        {

#if defined(__NIM_LINUX_PLATFORM__)
            nim_send_as_msg(priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
            priv->ul_status.s3501_autoscan_stop_flag = 0;
#endif
            AUTOSCAN_PRINTF("\tleave fuction nim_s3501_autoscan\n");
            return SUCCESS;
        }

        if (SUCCESS == fft_result)

        {
            step_freq = nim_s3501_ioctl(dev, NIM_FFT_JUMP_STEP, 0);
            crnum = nim_s3501_ioctl(dev, NIM_DRIVER_GET_CR_NUM, 0);
            for (i = 0; i < crnum; i++)
            {
                if (SUCCESS == nim_s3501_channel_search(dev, (UINT32) i))
                {
                    cur_freq = (UINT32) nim_s3501_ioctl(dev, NIM_DRIVER_GET_CUR_FREQ, 0);
                    nim_s3501_get_symbol_rate(dev, &cur_sym);
                    nim_s3501_get_code_rate(dev, &cur_fec);
                    if (i == crnum - 1)
                    {
                        temp = cur_freq + cur_sym / 2000 - fft_freq + 45;
                        if (temp < TEMP_VAL_20)
                        {
                            temp = 5;
                        }
                        else
                        {
                            temp = temp - 15;
                        }
                        AUTOSCAN_PRINTF("least step_freq is %d, step_freq is %d\n", temp, step_freq);
                        if (temp < step_freq)
                        {
                            AUTOSCAN_PRINTF("@@@@@ amy ajust step_freq from %d to %d @@@@@\n", step_freq, temp);
                            step_freq = temp;
                        }
                    }
                    if(NIM_SCAN_SLOW == priv->blscan_mode)
                    {
                        if((priv->ul_status.m_freq[i] / 1024) > last_ch_fc + 10)
                        {
                            double_i = 0;
                        }
                        if (i == crnum - 1)
                        {
                            temp_step = (priv->ul_status.m_freq[i] / 1024) + priv->ul_status.m_rs[i] / 2000 * 1.2 - \
                                fft_freq + 45;
                            temp_step = temp_step - 18;

                            AUTOSCAN_PRINTF("\tleast step_freq is %d, step_freq is %d\n", temp_step, step_freq);
                            if ( temp_step < step_freq )
                            {
                                step_freq = temp_step;
                            }

                            if (step_freq < STEP_FREQ_5)
                            {
                                step_freq = 5;
                            }
                        }
                    }


                    if ((cur_freq >= pstauto_scan->sfreq) && (cur_freq <= pstauto_scan->efreq))
                    {
                        //>>> Unicable begin
                        if (pstauto_scan->unicable) //cur_freq -> [950, 2150]
                        {
                            AUTOSCAN_PRINTF("\tUnicable fft_freq: %d, cur_freq: %d, Fub: %d\n", fft_freq,
                               cur_freq, pstauto_scan->fub);
                            ret = nim_callback(pstauto_scan, priv, 1, 0, fft_freq + cur_freq - pstauto_scan->fub,
                                cur_sym, cur_fec, 0);
                        }
                        else    //<<< Unicable end
                        {
                            if((cur_freq > last_tp_freq - 3) && (cur_freq < last_tp_freq + 3))
                            {
                                ret = nim_callback(pstauto_scan, priv, 0, 0, fft_freq, 0, 0, 0);
                                AUTOSCAN_PRINTF("\tRescan a tp: fft_freq: %d, last_tp_freq: %d\n",
                                fft_freq, last_tp_freq);
                            }
                            else
                            {
                                ret = nim_callback(pstauto_scan, priv, 1, 0, cur_freq, cur_sym, cur_fec, 0);
                                last_tp_freq = cur_freq;
                            }

                        }
                    }
                    else
                    {
                        if (dev_priv->ul_status.s3501_autoscan_stop_flag)
                        {

#if defined(__NIM_LINUX_PLATFORM__)
                            nim_send_as_msg(priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
                            priv->ul_status.s3501_autoscan_stop_flag = 0;
#endif
                            AUTOSCAN_PRINTF("\tleave fuction nim_s3501_autoscan\n");
                            return SUCCESS;
                        }
                        else
                        {
                            continue ;
                        }
                    }
                }
                else
                {
                    ret = nim_callback(pstauto_scan, priv, 0, 0, fft_freq, 0, 0, 0);
                    if (i == crnum - 1)
                    {
                        if (i > 0)
                        {
                            temp = (dev_priv->ul_status.m_freq[i - 1] >> 10) + \
                               dev_priv->ul_status.m_rs[i - 1] / 2000 - fft_freq + 45;
                            if (temp < TEMP_VAL_20)
                            {
                                temp = 5;
                            }
                            else
                            {
                                temp = temp - 15;
                            }
                            AUTOSCAN_PRINTF("least step_freq is %d, step_freq is %d\n", temp, step_freq);
                            if (temp < step_freq)
                            {
                                if (temp > TEMP_VAL_30)
                                {
                                    AUTOSCAN_PRINTF("@@@@@ amy ajust step_freq from %d to %d @@@@@\n",
                                      step_freq, temp);
                                    step_freq = temp;
                                }
                                else
                                {
                                    AUTOSCAN_PRINTF("@@@@@ amy ajust step_freq from %d to %d @@@@@\n",
                                   step_freq, step_freq - 5);
                                    step_freq = step_freq - 5;
                                }
                            }
                        }
                        else
                        {
                            AUTOSCAN_PRINTF("least step_freq is 30, step_freq is %d\n", step_freq);
                            if (step_freq > STEP_FREQ_30)
                            {
                                AUTOSCAN_PRINTF("@@@@@ amy ajust step_freq from %d to 30 @@@@@\n", step_freq);
                                step_freq = 30;
                            }
                        }
                    }

                    if(NIM_SCAN_SLOW == priv->blscan_mode)
                    {
                        if((priv->ul_status.m_freq[i] / 1024) > last_ch_fc + 10)
                        {
                            double_i = 0;
                        }

                        if(double_i == 0)
                        {
                            if(priv->ul_status.m_rs[i] > SYM_20000)
                            {
                                step_freq = (priv->ul_status.m_freq[i] / 1024) - \
                                    priv->ul_status.m_rs[i] / 2000 * 1.2 - fft_freq + 45 - 18;
                                last_ch_fc = (priv->ul_status.m_freq[i] / 1024);
                                double_i = 1;
                                if(step_freq < STEP_FREQ_5)
                                {
                                    step_freq = 5;
                                }
                                AUTOSCAN_PRINTF("\tdouble scan step freq: %d\n", step_freq);
                                break;
                            }
                        }


                        if (i == crnum - 1)
                        {
                            if (i > 0)
                            {
                                temp_step = (priv->ul_status.m_freq[i] / 1024) - \
                                   priv->ul_status.m_rs[i] / 2000 * 1.2 - fft_freq + 45;
                                temp_step = temp_step - 18;

                                AUTOSCAN_PRINTF("temp_freq is %d\n", temp_step);
                                if (temp_step < step_freq)
                                {
                                    step_freq = temp_step;
                                    AUTOSCAN_PRINTF("\tamy ajust step_freq from %d to %d @@@@@\n",
                                step_freq, temp_step);
                                }
                                if (step_freq < STEP_FREQ_5)
                                {
                                    step_freq = 5;
                                    AUTOSCAN_PRINTF("\tamy ajust step_freq from %d to 5 @@@@@\n",
                                  step_freq);
                                }

                            }
                            else
                            {
                                AUTOSCAN_PRINTF("\tonly 1 channel founed, least step_freq is 24, \
				                step_freq is %d\n", step_freq);
                                if (step_freq > STEP_FREQ_24)
                                {
                                    AUTOSCAN_PRINTF("\tamy ajust step_freq from %d to 24 @@@@@\n", step_freq);
                                    step_freq = 24;
                                }

                            }
                        }
                    }
                }
                if (ret == RET_VALUE_2)
                {
                    return SUCCESS;
                }
                else if (ret == 1)
                {
                    goto nim_as_break;
                }
            }
            if (crnum == 0)
            {
                if(NIM_SCAN_SLOW == priv->blscan_mode)
                {
                    if(step_freq >= STEP_FREQ_40)
                    {
                        step_freq = 40;
                    }
                }

                if (nim_callback(pstauto_scan, priv, 0, 0, fft_freq, 0, 0, 0) == 1)
                {
                    goto nim_as_break;
                }
            }
        }
        else if(NIM_SCAN_SLOW == priv->blscan_mode)
        {
            step_freq = 5;
        }
    }

#ifdef AUTOSCAN_DEBUG
    priv->tuner_config_data.qpsk_config = config_data;
#endif


#if defined(__NIM_LINUX_PLATFORM__)
    nim_send_as_msg(priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
    priv->ul_status.s3501_autoscan_stop_flag = 0;
#endif

    AUTOSCAN_PRINTF("\tfinish autoscan\n");

    return SUCCESS;

nim_as_break:
    nim_callback(pstauto_scan, priv, 2, 0, 0, 0, 0, 0); /* Tell callback search finished */

#if defined(__NIM_LINUX_PLATFORM__)
    nim_send_as_msg(priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
    priv->ul_status.s3501_autoscan_stop_flag = 0;


#endif
    nim_s3501_autoscan_signal_input(dev, NIM_SIGNAL_INPUT_CLOSE);
    return ret;
}
#endif




// begin add operation for s3501 optimize

static INT32 nim_s3501_set_err(struct nim_device *dev)
{
    struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
	priv = (struct nim_s3501_private *) dev->priv;
    priv->ul_status.m_err_cnts++;
    return SUCCESS;
}


