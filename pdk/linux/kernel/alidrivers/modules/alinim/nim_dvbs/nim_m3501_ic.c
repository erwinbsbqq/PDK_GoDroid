/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_m3501_ic.c
*
*    Description:  m3501 ic layer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "nim_m3501.h"


#define SYM_40000 40000
#define SAMPLE_FREQ_90000 90000
#define SYM_1000 1000
#define SYM_2500 2500
#define SYM_3000 3000
#define SYM_5000 5000
#define SYM_16000 16000
#define SYM_10000 10000
#define SYM_18000 18000
#define TEMP_VALUE_3 3
#define FFT_POINT_1024 1024
#define CAP_NUM_100 100
#define DEL_FREQ_512 512
#define DEL_FREQ_2 2
#define DEL_SYM_100 100
#define LOCK_CNT_4 4
#define TR_LOCK_CNT_5 5
#define CR_LOCK_CNT_5 5
#define CRYSTAL_FREQ_15000 15000
#define TIMEOUT_NUM_100 100



typedef struct nim_search_params_t
{
    struct nim_s3501_private *priv;
    UINT8 intindex;
    UINT8 lock_monitor;
    UINT32 freq;
    UINT32 rs_input;
    UINT32 rs;
    UINT32 rs_rev;
    UINT8 lock;
    UINT32 hwtmout_cnt;
    UINT32 tr_lock_flag;
    UINT32 cr_lock_flag;
    UINT32 last_freq;
    UINT32 last_rs;
    UINT32 s2_lock_cnt;
    UINT8 low_sym;
    UINT32 timeout;
    UINT32 tr_lock_thr;
    UINT32 cr_lock_thr;
    UINT32 fs_lock_thr;
    UINT32 ck_lock_num;
    UINT32 tr_lock_num;
    UINT32 cr_lock_num;
    INT32 delfreq;
}NIM_SEARCH_PARAMS;




static INT32     fft_energy_store[1024] ={0};


const UINT8 ssi_clock_tab[] =
{
    98, 90, 83, 77, 72, 67, 60, 54, 50
};



static INT32 nim_s3501_cap_iq_enerage(struct nim_device *dev);
static INT32         nim_s3501_soft_search(struct nim_device *dev, UINT32 *rs, UINT32 *freq, INT32 delfreq);

/*****************************************************************************
* INT32 nim_s3501_set_12v(struct nim_device *dev, UINT8 flag)
* Description: S3501 set LNB votage 12V enable or not
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 flag
*
* Return Value: SUCCESS
*****************************************************************************/
INT32 nim_s3501_set_12v(struct nim_device *dev, UINT8 flag)
{
    return SUCCESS;
}


INT32 nim_s3501_sym_config(struct nim_device *dev, UINT32 sym)
{
    struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	    return ERR_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    if (sym > SYM_40000)
    {
        priv->ul_status.c_rs = 8;
    }
    else if (sym > 30000)
    {
        priv->ul_status.c_rs = 4;
     }
    else if (sym > 20000)
    {
        priv->ul_status.c_rs = 2;
    }
    else if (sym > 10000)
    {
        priv->ul_status.c_rs = 1;
     }
    else
    {
        priv->ul_status.c_rs = 0;
    }
    return SUCCESS;
}


INT32 nim_s3501_fft_find_channel(struct nim_device *dev, UINT32 *tune_freq)
{
    INT32 success = 0;
    INT32 delta_fc_est[MAX_CH_NUMBER]={0};
    INT32 symbolrate_est[MAX_CH_NUMBER]={0};
    INT32 if_freq = 0;
    INT32 ch_number = 0;
    INT32 i = 0;
    INT32 tune_jump_freq = 0;
    INT32 tempfreq = 0;
    INT32 temp = 0;
    INT32 temp1 = 0;
    UINT8 data = 0;
    UINT32 sample_rate = 0;

    if((NULL == dev) || (NULL == tune_freq))
    {
	    return ERR_FAILURE;
	}	

    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;


    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if ((priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C) && (priv->ul_status.m_tso_mode == 1))
        {
          NIM_PRINTF("Need not configure TSO speed.\n");

        }
        else if (priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_S3501D)
        {
            data = 0x40;    //capture fix to 8bit mode
            nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
        }
        else
        {
            nim_reg_read(dev, RC0_BIST_LDPC_REG, &data, 1);
            data = data & 0x3f;
            data = data | 0x80; //8bit mode
            nim_reg_write(dev, RC0_BIST_LDPC_REG, &data, 1);
        }
    }
    else
    {
        data = 0x40;
        nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
    }

    nim_s3501_calculate_energy(dev);
    NIM_PRINTF("tune_freq = %d\n", (int)(*tune_freq));

    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if ((priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C) && (priv->ul_status.m_tso_mode == 1))
        {
             NIM_PRINTF("Need not configure TSO speed.\n");
        }
        else if (priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_S3501D)
        {
            if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_1BIT_MODE)
            {
                data = 0xe0;
            }
            else if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_2BIT_MODE)
            {
                data = 0x00;
            }
            else if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_4BIT_MODE)
            {
                data = 0x20;
             }
            else if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_8BIT_MODE)
            {
                data = 0x40;
            }
            else
            {
                data = 0x40;
            }
            nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
        }
        else
        {
            nim_reg_read(dev, RC0_BIST_LDPC_REG, &data, 1);
            data = data & 0x3f;
            data = data | 0xc0; //1//enable 1bit mode
            nim_reg_write(dev, RC0_BIST_LDPC_REG, &data, 1);
        }
    }
    else
    {
        nim_s3501_set_ts_mode(dev, 0x0, 0x0, 0x0, 0x0, 0x1);
    }

    if(NIM_SCAN_SLOW == priv->blscan_mode)
    {
        comm_memcpy( fft_energy_store, fft_energy_1024, sizeof(fft_energy_1024));
        temp1 = fft_energy_store[511];
        fft_energy_store[511] = (fft_energy_store[510] + fft_energy_store[512]) / 2;
        for (i = 6; i < 1024 - 6; i++)
        {
            temp = 0;
            for (data = 0; data < 12; data++)
            {
                temp = temp + fft_energy_store[i - 6 + data];
            }
            temp = temp / 12;
            fft_energy_1024[i] = temp;
        }
        fft_energy_store[511] = temp1;
    }

    nim_s3501_autosearch((INT32 *) &success, (INT32 *) delta_fc_est, (INT32 *) symbolrate_est,
                              (INT32 *) &if_freq, (INT32 *) &ch_number);

    nim_s3501_get_dsp_clk(dev, &sample_rate);
    if (1 == success)
    {
        for (i = 0; i < ch_number; i++)
        {
            temp = nim_s3501_multu64div(delta_fc_est[i], sample_rate, 90000); //
            tempfreq = (*tune_freq) * 1024 + temp;
            priv->ul_status.m_freq[priv->ul_status.m_crnum] = tempfreq;
            NIM_PRINTF("    m_Freq[%d] is %d\n", priv->ul_status.m_crnum,
                      (int)priv->ul_status.m_freq[priv->ul_status.m_crnum]);
            priv->ul_status.m_rs[priv->ul_status.m_crnum] = nim_s3501_multu64div(symbolrate_est[i], sample_rate, 90000);
            priv->ul_status.m_crnum++;
        }
    }
    else
    {
        NIM_PRINTF("    Base band width is %d, ch_number is%d\n", (int)(if_freq / 1024), (int)ch_number);
    }
    if_freq = nim_s3501_multu64div(if_freq, sample_rate, SAMPLE_FREQ_90000); //
    tune_jump_freq = if_freq / 1024;
    return tune_jump_freq;
}


void nim_s3501_fft_result_read(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT8 read_data = 0;
    INT32 m = 0;

    if(NULL == dev)
    {
       return;
    }	
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    nim_s3501_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);

    nim_s3501_set_dsp_clk (dev, 0);

    nim_s3501_adc_setting(dev);

    nim_s3501_interrupt_mask_clean(dev);

    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if((priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C) && (priv->ul_status.m_tso_mode == 1))
        {
            data = 0x11;
            nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
        }
    }
    //CR07
    nim_s3501_agc1_ctrl(dev, 0x00, NIM_OPTR_FFT_RESULT);

#ifdef NIM_TS_PORT_CAP

    nim_s3501_demod_ctrl(dev, NIM_DEMOD_CTRL_0X02);

    data = 0x3c;    //0xfc lead lots of tps.
    nim_reg_write(dev, RBD_CAP_PRM + 0x01, &data, 1);

#else

    //CRBE
    data = 0x01;
    nim_reg_write(dev, RBD_CAP_PRM + 0x01, &data, 1);
#endif
    //CR00

    data = 0x55;
    nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);

    nim_s3501_demod_ctrl(dev, NIM_DEMOD_CTRL_0X52);

    comm_delay(1000);
    comm_memset(priv->ul_status.adcdata_malloc_addr, 0, 0x20000 * 2);

#ifdef __NIM_TDS_PLATFORM__
    dmx_pause((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
#endif

    nim_s3501_autoscan_signal_input(dev, NIM_SIGNAL_INPUT_OPEN);

    data = 0x50;
    nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);

    for (m = 0; m < 1000; m++)
    {
        comm_delay(1000);
        nim_reg_read(dev, R02_IERR, &read_data, 1);
        if (0x01 == (read_data & 0x01))
        {
#ifdef NIM_TS_PORT_CAP
            NIM_PRINTF(" ts data transfer finish\n");
            break;
#else

            nim_s3501_cap_iq_enerage(dev);

            //R2FFT
            R2FFT(priv->ul_status.FFT_I_1024, priv->ul_status.FFT_Q_1024);
            NIM_PRINTF(" ADC DATA  transfer finish\n");
            break;

#endif
        }
    }



    nim_s3501_autoscan_signal_input(dev, NIM_SIGNAL_INPUT_CLOSE);
#ifdef __NIM_TDS_PLATFORM__
    dmx_start((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
#endif

}


#ifdef NIM_CAPTURE_SUPPORT

INT32 nim_s3501_cap_start(struct nim_device *dev, UINT32 startfreq, UINT32 sym, UINT32 *cap_buffer)
{
    INT32 i = 0;
    UINT32  freq = 0;
    UINT8   data = 0x10;
    UINT8   low_sym = 0;
    INT32   result = 200;
    UINT8   lock = 200;
    UINT8 ver_data = 0;
    struct nim_s3501_private *dev_priv = NULL;

    if(NULL == dev)
    {
	    return ERR_FAILURE;
	}	
    dev_priv = dev->priv;
    nim_s3501_agc1_ctrl(dev, low_sym, NIM_OPTR_CHL_CHANGE);

    NIM_PRINTF("Enter nim_S3501_cap_start:\n");

    freq = startfreq;

    if(nim_s3501_i2c_open(dev))
    {
        return ERR_I2C_NO_ACK;
     }

    freq = startfreq;
    result = dev_priv->nim_tuner_control(dev_priv->tuner_id, freq, sym);
    if (result == SUCCESS)
    {
        NIM_PRINTF("Config tuner %dMHz success\n", (int)freq);
    } 
    else
    {
        NIM_PRINTF("Config tuner %dMHz failed\n", (int)freq);
    }

    lock = 0;
    for( i = 0; i < 1000; i++ )
    {
        comm_delay(1000);
        result = dev_priv->nim_tuner_status(dev_priv->tuner_id, &lock);
        if (lock)
        {
            NIM_PRINTF("Tuner lock after %d times delay\n", (int)(i + 1));
            break;
        }
    }
    if(lock == 0)
    {
        NIM_PRINTF("Tuner unlock at %dMHz\n", (int)freq);
    }

    if(nim_s3501_i2c_close(dev))
    {
        return ERR_I2C_NO_ACK;
    }

    nim_s3501_adc_setting(dev);
    //CR07
    nim_s3501_agc1_ctrl(dev, low_sym, NIM_OPTR_CHL_CHANGE);

    nim_reg_read(dev, R07_AGC1_CTRL, &ver_data, 1);
    if(dev_priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if ((dev_priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C) && (dev_priv->ul_status.m_tso_mode == 1))
        {
             NIM_PRINTF("Need not configure TSO speed.\n");
        }
        else if(dev_priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_S3501D)
        {
            data = 0x40;    //capture fix to 8bit mode
            nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
        }
        else
        {
            nim_reg_read(dev, RC0_BIST_LDPC_REG, &data, 1);
            data = data & 0x3f;
            data = data | 0x80; //8bit mode
            nim_reg_write(dev, RC0_BIST_LDPC_REG, &data, 1);
            nim_m3501c_fec_ts_on(dev);
            nim_m3501_ts_on(dev);
        }
    }
    else
    {
        data = 0x40; //M3501A need config this for SPI_CLK output
        nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
    }

    nim_s3501_calculate_energy(dev);

    if(dev_priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if ((dev_priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C) && (dev_priv->ul_status.m_tso_mode == 1))
        {
             NIM_PRINTF("Need not configure TSO speed.\n");
        }
        else if(dev_priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_S3501D)
        {
            if((dev_priv->tuner_config_data.qpsk_config & 0xc0) == M3501_1BIT_MODE)
            {
                data = 0xe0;
            }
            else if((dev_priv->tuner_config_data.qpsk_config & 0xc0) == M3501_2BIT_MODE)
            {
                data = 0x00;
            }
            else if((dev_priv->tuner_config_data.qpsk_config & 0xc0) == M3501_4BIT_MODE)
            {
                data = 0x20;
             }
            else if((dev_priv->tuner_config_data.qpsk_config & 0xc0) == M3501_8BIT_MODE)
            {
                data = 0x40;
            }
            else
            {
                data = 0x40;
            }
            nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
        }
        else
        {
            nim_reg_read(dev, RC0_BIST_LDPC_REG, &data, 1);
            data = data & 0x3f;
            data = data | 0xc0; //1//enable 1bit mode
            nim_reg_write(dev, RC0_BIST_LDPC_REG, &data, 1);
        }
    }
    else
    {
        nim_s3501_set_ts_mode(dev, 0x0, 0x0, 0x0, 0x0, 0x1);
    }

    return SUCCESS;
}

INT32 nim_s3501_cap(struct nim_device *dev, UINT32 startfreq, UINT32 *cap_buffer, UINT32 sym)
{
#if defined(__NIM_TDS_PLATFORM__)	
    struct nim_s3501_private *priv = NULL;
#endif

    if((NULL == dev)|| (NULL == cap_buffer))
    {
	    return ERR_FAILURE;
	}	

    NIM_PRINTF("Enter nim_S3501_cap:\n");
    if (nim_s3501_get_bypass_buffer(dev))
    {
        return ERR_NO_MEM;
     }
    nim_s3501_autoscan_signal_input(dev, NIM_SIGNAL_INPUT_OPEN);
    nim_s3501_cap_start(dev, startfreq, sym, cap_buffer);
    nim_s3501_autoscan_signal_input(dev, NIM_SIGNAL_INPUT_CLOSE);
#if defined(__NIM_TDS_PLATFORM__)
    priv = (struct nim_s3501_private *) dev->priv;
    comm_free(priv->ul_status.adcdata_malloc_addr);
#endif
    return SUCCESS;
}

#endif



INT32 nim_s3501_set_hw_timeout(struct nim_device *dev, UINT8 time_thr)
{
    if(NULL == dev)
    {
	    return ERR_FAILURE;
	}	
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    // AGC1 setting
    if (time_thr != priv->ul_status.m_hw_timeout_thr)
    {
        nim_reg_write(dev, R05_TIMEOUT_TRH, &time_thr, 1);
        priv->ul_status.m_hw_timeout_thr = time_thr;
    }

    return SUCCESS;
}




INT32 nim_s3501_freq_offset_set(struct nim_device *dev, UINT8 low_sym, UINT32 *s_freq)
{
    struct nim_s3501_private *priv = NULL;
	
    if((dev == NULL) || (s_freq == NULL))
	{
        return ERR_FAILURE;
	}	
    
	priv = (struct nim_s3501_private *) dev->priv;

    if (priv->tuner_config_data.qpsk_config & M3501_QPSK_FREQ_OFFSET)
    {
        if (1 == low_sym)
        {
           *s_freq += 3;
         }
    }
    return SUCCESS;
}




INT32 nim_s3501_freq_offset_reset(struct nim_device *dev, UINT8 low_sym)
{
    struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	    return ERR_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    if (priv->tuner_config_data.qpsk_config & M3501_QPSK_FREQ_OFFSET)
    {
        if (1 == low_sym)
        {
        nim_s3501_set_freq_offset(dev, -3000);
         }
        else
        {
        nim_s3501_set_freq_offset(dev, 0);
         }
    }
    else
    {
         nim_s3501_set_freq_offset(dev, 0);
     }

    return SUCCESS;
}



INT32 nim_s3501_freq_offset_reset1(struct nim_device *dev, UINT8 low_sym, INT32 delfreq)
{
    struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	    return ERR_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    if (priv->tuner_config_data.qpsk_config & M3501_QPSK_FREQ_OFFSET)
    {
        if (1 == low_sym)
        {
            nim_s3501_set_freq_offset(dev, -3000 + delfreq);
        }
        else
        {
            nim_s3501_set_freq_offset(dev, delfreq);
         }
    }
    else
    {
        nim_s3501_set_freq_offset(dev, delfreq);
     }

    return SUCCESS;
}



INT32 nim_s3501_demod_ctrl(struct nim_device *dev, UINT8 c_value)
{
    UINT8 data = c_value;

    if(dev == NULL)
    {
	    return ERR_FAILURE;
	}	

    nim_reg_write(dev, R00_CTRL, &data, 1);
    return SUCCESS;
}

static INT32 nim_s3501_cap_iq_enerage(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT8 buffer[8]={0};
    INT32 i = 0;
    INT32 j = 0;
    INT32 k = 0;
    UINT8 fft_data[3]={0};
    INT32 fft_i = 0;
    INT32 fft_q = 0;
	struct nim_s3501_private *priv =NULL;
	
    if(dev == NULL)
    {
	    return ERR_FAILURE;
	}	
    
	priv = (struct nim_s3501_private *) dev->priv;

    for (i = 0; i < 1024; i++)
    {
        data = (UINT8) ((i) & 0xff);
        buffer[0] = data;
        data = (UINT8) (i >> 8);
        data &= 0x3;
        buffer[1] = data;
        nim_reg_write(dev, R70_CAP_REG, buffer, 2);
        nim_reg_read(dev, R70_CAP_REG + 0x02, fft_data, 2);

        fft_i = fft_data[0];
        fft_q = fft_data[1];

        if (fft_i & 0x80)
        {
        fft_i |= 0xffffff00;
         }
        if (fft_q & 0x80)
        {
        fft_q |= 0xffffff00;
        }

        k = 0;
        for (j = 0; j < 10; j++)
        {
            k <<= 1;
            k += ((i >> j) & 0x1);
        }

        fft_i = fft_i << (FFT_BITWIDTH - 8);
        fft_q = fft_q << (FFT_BITWIDTH - 8);

        priv->ul_status.FFT_I_1024[k] = fft_i;
        priv->ul_status.FFT_Q_1024[k] = fft_q;
    }
#if 0
    if(1 == m3501_debug_flag & (0x01))
    {
        for(i = 0; i < 1024; i++)
        {
            AUTOSCAN_PRINTF("ADCdata%d:[%d,%d]\n", i, priv->ul_status.FFT_I_1024[i] , priv->ul_status.FFT_Q_1024[i]);
        }
    }
#endif

    return SUCCESS;
}




INT32 nim_s3501_set_fc_search_range(struct nim_device *dev, UINT8 s_case, UINT32 rs)
{
  struct nim_s3501_private *priv = NULL;
    
    UINT8 data = 0;

    UINT32 sample_rate = 0;
    UINT32 temp = 0;

    if(dev == NULL)
	{
        return ERR_FAILURE;
	}
	priv = (struct nim_s3501_private *) dev->priv;
		
    nim_s3501_get_dsp_clk (dev, &sample_rate);
    sample_rate = ((sample_rate + 500) / 1000);
    switch (s_case)
    {
    case NIM_OPTR_SOFT_SEARCH:
    {
        if (rs > SYM_16000)
         {
        temp = 5 * 16;
         }
        else if (rs > 5000)
         {
        temp = 4 * 16;
         }
        else
        {
        temp = 3 * 16;
         }
        data = temp & 0xff;
        nim_reg_write(dev, R62_FC_SEARCH, &data, 1);
        data = (temp >> 8) & 0x3;
        if (rs > SYM_10000)
        {
        data |= 0xa0;
        }
        else if (rs > 3000)
        {
        data |= 0xc0;
        }
        else
        {
        data |= 0xb0;
        }
        nim_reg_write(dev, R62_FC_SEARCH + 0x01, &data, 1);
        priv->t_param.t_reg_setting_switch |= NIM_SWITCH_FC;
    }
    break;
    case NIM_OPTR_CHL_CHANGE:
        //if (priv->t_Param.t_reg_setting_switch & NIM_SWITCH_FC)
    {
        //temp = ((3*90*16) + (sample_rate/2)) / sample_rate;
        if (rs > SYM_18000)
        {
        temp = 5 * 16;
        }
        else if (rs > 15000)
        {
        temp = 4 * 16;
        }
        else if (rs > 4000)
        {
        temp = 3 * 16;
        }
        else
        {
        temp = 2 * 16;
        }

        data = temp & 0xff;
        nim_reg_write(dev, R62_FC_SEARCH, &data, 1);

        data = (temp >> 8) & 0x3;
        data |= 0xb0;
        nim_reg_write(dev, R62_FC_SEARCH + 0x01, &data, 1);
        priv->t_param.t_reg_setting_switch &= ~NIM_SWITCH_FC;
    }

    break;
    default:
    break;
    }
    return SUCCESS;
}

INT32 nim_s3501_rs_search_range(struct nim_device *dev, UINT8 s_case, UINT32 rs)
{
    struct nim_s3501_private *priv = NULL;
    UINT8 data = 0;
    UINT32 sample_rate = 0;
    UINT32 temp = 0;

    if(dev == NULL)
	{
        return ERR_FAILURE;
	}	
	priv = (struct nim_s3501_private *) dev->priv;
	
	sample_rate = (CRYSTAL_FREQ * 99 * 10 + 67) / 135;
    nim_s3501_get_dsp_clk (dev, &sample_rate);
    sample_rate = ((sample_rate + 500) / 1000);
    switch (s_case)
    {
    case NIM_OPTR_SOFT_SEARCH:
    {
        //CR64
        temp = rs / 2000;

        if (temp < TEMP_VALUE_3)
        {
           temp = 3;
        }
        else if (temp > 11)
        {
           temp = 11;
        }
        temp = temp << 4;
        data = temp & 0xff;
        nim_reg_write(dev, R64_RS_SEARCH, &data, 1);
        data = (temp >> 8) & 0x3;
        data |= 0x90;
        nim_reg_write(dev, R64_RS_SEARCH + 0x01, &data, 1);
        priv->t_param.t_reg_setting_switch |= NIM_SWITCH_RS;
    }

    break;
    case NIM_OPTR_CHL_CHANGE:
        if (priv->t_param.t_reg_setting_switch & NIM_SWITCH_RS)
        {
            temp = ((3 * 90 * 16) + sample_rate / 2) / sample_rate;
            data = temp & 0xff;
            nim_reg_write(dev, R64_RS_SEARCH, &data, 1);

            data = (temp >> 8) & 0x3;
            data |= 0x30;
            nim_reg_write(dev, R64_RS_SEARCH + 0x01, &data, 1);
            priv->t_param.t_reg_setting_switch &= ~NIM_SWITCH_RS;
        }

        break;
    default:
        break;
    }
    return SUCCESS;
}



INT32 nim_s3501_wideband_scan(struct nim_device *dev, UINT32 start_freq, UINT32 end_freq)
{
    UINT32 fft_freq = 0;
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 adc_sample_freq = 0;
    UINT32  step_freq = 0;
    struct nim_s3501_private *priv = NULL;

    if(dev == NULL)
    {
	    return ERR_FAILURE;
	}
		
    NIM_PRINTF("Enter function nim_s3501_WidebandScan:\n");
    priv = (struct nim_s3501_private *) dev->priv;
    if(priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501A)
    {
        adc_sample_freq = 98;
    }
    else
    {
        adc_sample_freq = ((CRYSTAL_FREQ * 90 + 6750) / 13500);
    }
    step_freq = adc_sample_freq / 2;

    if(SUCCESS == nim_s3501_wideband_scan_open(dev, start_freq, end_freq, step_freq))
    {
        for (fft_freq = start_freq; fft_freq < end_freq; fft_freq += step_freq)
        {
            nim_s3501_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
            comm_memset(fft_energy_1024, 0, sizeof(fft_energy_1024));
            for(j = 0; j < 100; j++)
            {
                nim_s3501_cap_start(dev, fft_freq, 0, fft_energy_1024);
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
            }

            if(CAP_NUM_100 == j)
            {
#if defined(__NIM_TDS_PLATFORM__)
                comm_free(priv->ul_status.adcdata_malloc_addr);
#endif
                NIM_PRINTF("the nim_s3501_WidebandScan function  works wrong !\n");
                NIM_PRINTF("ERR_FAILURE, Leave the nim_s3501_WidebandScan function !\n");
                return ERR_FAILURE;
            }
            nim_s3501_fft_wideband_scan(fft_freq, adc_sample_freq);
        }
#if defined(__NIM_TDS_PLATFORM__)
        comm_free(priv->ul_status.adcdata_malloc_addr);
#endif
        NIM_PRINTF("SUCCESS, Leave the nim_s3501_WidebandScan function !\n");
        return SUCCESS;
    }
    else
    {
        NIM_PRINTF("ERR_NO_MEM, Leave the nim_s3501_WidebandScan function !\n");
        return ERR_NO_MEM;
    }
}



INT32 nim_s3501_wideband_scan_open(struct nim_device *dev, UINT32 start_freq, UINT32 end_freq, UINT32 step_freq)
{
    UINT32 full_size = 0;
    last_tuner_if = 0;
    chlspec_num = 0;
    called_num = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	 }  
    full_size = (end_freq - start_freq) / step_freq;
    full_size = (full_size + 2) * 512;
    channel_spectrum = (INT32 *) comm_malloc(full_size * 4);
    if(channel_spectrum == NULL)
    {
        NIM_PRINTF("\n channel_spectrum--> no enough memory!\n");
        return ERR_NO_MEM;
    }

    if (nim_s3501_get_bypass_buffer(dev))
    {
        NIM_PRINTF("\n ADCdata--> no enough memory!\n");
        comm_free(channel_spectrum);
        channel_spectrum = NULL;
        return ERR_NO_MEM;
    }

    return SUCCESS;

}


INT32 nim_s3501_wideband_scan_close(void)
{
    comm_free(channel_spectrum);
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3501_channel_search(struct nim_device *dev, UINT32 crnum );
* Description: S3501 channel blind searching operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq              : Frequence
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3501_channel_search(struct nim_device *dev, UINT32 crnum)
{
    UINT32 freq = 0;
    UINT32 freq0 = 0;
    INT32 delfreq = 0;
    INT32 delfreq0 = 0;
    UINT32 sym = 0;
    struct nim_s3501_private *dev_priv= NULL;
	
    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}  

    dev_priv = (struct nim_s3501_private *)dev->priv;

    NIM_PRINTF("Enter nim_s3501_channel_search:\n");

    freq = dev_priv->ul_status.m_freq[crnum];
    sym = dev_priv->ul_status.m_rs[crnum];

    delfreq = freq % 1024;
    freq = freq >> 10;
    if(delfreq > DEL_FREQ_512)
    {
        delfreq = delfreq - 1024;
        freq = freq + 1;
    }
    dev_priv->ul_status.m_cur_freq = freq;

    AUTOSCAN_PRINTF("Enter nim_s3501_channel_search:TP -> %d. Freq=%d, rs_input=%d\n", (int)crnum, (int)freq, (int)sym);

    if (SUCCESS == nim_s3501_soft_search(dev, &dev_priv->ul_status.m_rs[crnum], &freq, delfreq))
    {
        return SUCCESS;
    }
    else if((freq <= 1846) && (freq >= 1845) && (sym <= 3000) && (sym >= 1500)) /////Yatai5:12446/V/1537
    {
        AUTOSCAN_PRINTF("*************Try_again*************\n");
        if(delfreq < 0)
        {
            freq0 = freq;
            delfreq0 = 0;
        }
        else if(delfreq < 400)
        {
            freq0 = freq;
            delfreq0 = 0;
        }
        else
        {
            freq0 = freq + 1;
            delfreq0 = 0;
        }
        if (SUCCESS == nim_s3501_soft_search(dev, &dev_priv->ul_status.m_rs[crnum], &freq, delfreq))
        {
            return SUCCESS;
        }
        else if (SUCCESS == nim_s3501_soft_search(dev, &dev_priv->ul_status.m_rs[crnum], &freq0, delfreq0))
        {
            return SUCCESS;
        }
    }

#if 0
    if(NIM_SCAN_SLOW == dev_priv->blscan_mode)
    {
        if ((sym < SYM_5000) && (sym > SYM_3000))
        {
            for (k = 0; k < 2; k++)
            {
                switch(k)
                {
                case 0:
                    freq = dev_priv->ul_status.m_freq[crnum] - 1024;
                    delfreq = freq % 1024;
                    freq = freq >> 10;
                    break;
                case 1:
                    freq = dev_priv->ul_status.m_freq[crnum] + 1024;
                    delfreq = freq % 1024;
                    freq = freq >> 10;
                    break;
                }
                if (SUCCESS == nim_s3501_soft_search(dev, &dev_priv->ul_status.m_rs[crnum], &freq, delfreq))
                    return SUCCESS;
            }
        }
    }
#endif

    return ERR_FAILUE;
}


static INT32 nim_s3501_locked_channel(struct nim_device *dev,NIM_SEARCH_PARAMS *params)
{
    UINT32 tempfreq = 0;
    UINT32 rs_input = 0;
    INT32  del_rs = 0;
    UINT8 code_rate = 0;
    UINT8 work_mode = 0;
    UINT8 map_type = 0;
    UINT8 modcod = 0;
	struct nim_s3501_private *priv = NULL;
	
    if((dev == NULL) || (params == NULL))
	{
       return ERR_FAILURE;
	}  
    priv = (struct nim_s3501_private *) dev->priv;

    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);

    NIM_PRINTF("        lock chanel \n");
    tempfreq = params->freq;
    nim_s3501_reg_get_freq(dev, &tempfreq);
    NIM_PRINTF("            Freq is %d\n", (int)(LNB_LOACL_FREQ - tempfreq));
    priv->ul_status.m_cur_freq = tempfreq ;

    nim_s3501_reg_get_symbol_rate(dev, &rs_input);
    NIM_PRINTF("            rs_input is %d\n", (int)rs_input);
    del_rs = params->rs_rev - rs_input;
    if(del_rs < 0)
    {
        del_rs = -del_rs;
    }
    if((rs_input >= SYM_1000) && (del_rs >= DEL_SYM_100))
    {
        params->rs_rev = rs_input;
    }
    rs_input = params->rs_rev;

    nim_s3501_reg_get_code_rate(dev, &code_rate);
    NIM_PRINTF("            code_rate is %d\n", code_rate);

    nim_s3501_reg_get_work_mode(dev, &work_mode);
    NIM_PRINTF("            work_mode is %d\n", work_mode);

    nim_s3501_reg_get_map_type(dev, &map_type);
    NIM_PRINTF("            map_type is %d\n", map_type);

    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if(priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C)
        {
            nim_s3501_reg_get_modcod(dev, &modcod);
            if(modcod & 0x01)
            {
                NIM_PRINTF("            Pilot on \n");
            }
            else
            {
                NIM_PRINTF("            Pilot off \n");
            }
            modcod = modcod >> 1;
            NIM_PRINTF("            Modcod is %x\n", modcod);
        }
    }

    final_est_freq = tempfreq;
    final_est_sym_rate = rs_input;
    //libc_printf("Pre_Add_TP_Para_path2:[%d,%d]\n",tempfreq,rs_input);

#ifdef NIM_S3501_ASCAN_TOOLS
    nim_s3501_ascan_add_tp(ASCAN_ADD_REAL_TP, 0x00, tempfreq, rs_input, tempfreq > 1550 ? 1 : 0);
#endif

    AUTOSCAN_PRINTF("\tLock freq %d rs_input %d with del_Rs %d del_f %d \n", (int)(LNB_LOACL_FREQ - tempfreq), 
                            (int)rs_input, (int)(rs_input - params->rs), (int)(tempfreq - params->freq));
    AUTOSCAN_PRINTF("\t timeout = %d\n", (int)params->timeout);

    nim_s3501_clear_int(dev);

    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return SUCCESS;


}

static INT32 nim_s3501_r04_0x18(struct nim_device *dev,NIM_SEARCH_PARAMS *params)
{
    UINT32 rs_input = 0;
    UINT8 work_mode = 0;
    UINT32 tempfreq = 0;
    INT32 del_freq = 0;
    INT32 del_rs = 0;
    UINT8 code_rate = 0;
    UINT8 map_type = 0;
    INT32 ret=RET_CONTINUE;

    if(dev == NULL || params == NULL)
    {
	   return ERR_FAILURE;
	}  
    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);
	
    params->lock_monitor = params->lock_monitor | params->intindex;
    nim_s3501_reg_get_symbol_rate(dev, &rs_input);
    tempfreq = params->freq;
    nim_s3501_reg_get_freq(dev, &tempfreq);
    nim_s3501_reg_get_work_mode(dev, &work_mode);

    del_freq = tempfreq - params->last_freq;
    if (del_freq < 0)
     {
        del_freq = -del_freq;
     }

    del_rs = rs_input - params->last_rs;
    if (del_rs < 0)
    {
        del_rs = -del_rs;
    }

    if(del_freq <= DEL_FREQ_2)
    {
        params->cr_lock_num++;
    }
    else
    {
        params->cr_lock_num = 0;
        params->last_freq = tempfreq;
        params->last_rs = rs_input;
    }
    AUTOSCAN_PRINTF("       current lock rs_input is %d,  Freq is %d at time %d\n", (int)rs_input, 
               (int)tempfreq, (int)params->timeout);

    del_rs = params->rs_rev - rs_input;
    if(del_rs < 0)
    {
        del_rs = -del_rs;
    }
    if((rs_input >= SYM_1000) && (del_rs >= DEL_SYM_100))
    {
        params->rs_rev = rs_input;
    }
    rs_input = params->rs_rev;
    if ((work_mode == 0) || (work_mode == 1) || (params->s2_lock_cnt > LOCK_CNT_4))
    {
        NIM_PRINTF("        lock chanel \n");
        NIM_PRINTF("            Freq is %d\n", (int)(LNB_LOACL_FREQ - tempfreq));
        params->priv->ul_status.m_cur_freq = tempfreq ;
        NIM_PRINTF("            rs_input is %d\n", (int)rs_input);
        nim_s3501_reg_get_code_rate(dev, &code_rate);
        NIM_PRINTF("            code_rate is %d\n", (int)code_rate);
        NIM_PRINTF("            work_mode is %d\n", (int)work_mode);
        nim_s3501_reg_get_map_type(dev, &map_type);
        NIM_PRINTF("            map_type is %d\n", (int)map_type);
        AUTOSCAN_PRINTF("\tLock freq %d rs_input %d with del_Rs %d del_f %d \n",
			       (int)(LNB_LOACL_FREQ - tempfreq), 
                   (int)rs_input, (int)(rs_input - params->rs), (int)(tempfreq - params->freq));
        AUTOSCAN_PRINTF("work_mode= %d, s2_lock_cnt= %d \n", (int)work_mode, (int)params->s2_lock_cnt);
        final_est_freq = tempfreq;
        final_est_sym_rate = rs_input;
#ifdef NIM_S3501_ASCAN_TOOLS
        nim_s3501_ascan_add_tp(ASCAN_ADD_REAL_TP, 0x00, tempfreq, rs_input, tempfreq > 1550 ? 1 : 0);
#endif
        AUTOSCAN_PRINTF("\t timeout = %d\n", (int)params->timeout);
        nim_s3501_clear_int(dev);
        return SUCCESS;
    }

    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;

}



static INT32 nim_s3501_r04_0x04(struct nim_device *dev,NIM_SEARCH_PARAMS *params)
{
    UINT32 rs_input = 0;
    UINT8 data = 0;
    INT32 ret=RET_CONTINUE;

    if(dev == NULL || params == NULL)
    {
	   return ERR_FAILURE;
	}   
    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);

    params->lock_monitor = params->lock_monitor | 0x04;
    params->tr_lock_num++;
    if (params->tr_lock_flag == 0)
    {
        nim_s3501_reg_get_symbol_rate(dev, &rs_input);
        NIM_PRINTF("        RS=%d \n", (int)rs_input);
        if(rs_input <= SYM_1000)
        {
            params->tr_lock_num--;
        }
        else
        {
            params->tr_lock_flag = 1;
            if(rs_input <= SYM_3000)
            {
                params->cr_lock_thr = 30;
            }
            else if(rs_input < SYM_10000)
            {
                params->cr_lock_thr = 20;
            }
            else
            {
                params->cr_lock_thr = 15;
            }
            params->fs_lock_thr = params->cr_lock_thr + 10;
            nim_s3501_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
            if (nim_s3501_i2c_open(dev))
            {
                return S3501_ERR_I2C_NO_ACK;
            }
            comm_delay(500);
            if (params->priv->nim_tuner_control != NULL)
            {
                 params->priv->nim_tuner_control(params->priv->tuner_id, params->freq, rs_input);
            }
            if (params->priv->nim_tuner_status != NULL)
            {
                params->priv->nim_tuner_status(params->priv->tuner_id, &params->lock);
            }
            comm_delay(500);
            if (nim_s3501_i2c_close(dev))
            {
                 return S3501_ERR_I2C_NO_ACK;
            }
            params->lock = 200;
            nim_s3501_agc1_ctrl(dev, params->low_sym, NIM_OPTR_CHL_CHANGE);
            nim_s3501_set_rs(dev, rs_input);
            nim_s3501_set_fc_search_range(dev, NIM_OPTR_SOFT_SEARCH, rs_input);
            nim_s3501_clear_int(dev);
            data = 0x00;
            nim_reg_write(dev, R28_PL_TIMEOUT_BND, &data, 1);
            nim_reg_read(dev, R28_PL_TIMEOUT_BND + 1, &data, 1);
            data &= 0xe0;
            data |= 0x01;
            nim_reg_write(dev, R28_PL_TIMEOUT_BND + 1, &data, 1);
            if (params->priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
            {
                if((params->priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C) &&
              (params->priv->ul_status.m_tso_mode == 1))
                {
                    nim_m3501c_close_dummy(dev); // include reset TSO
                }
            }
            nim_s3501_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);
            params->tr_lock_thr = params->tr_lock_thr - 2;
            params->lock_monitor = 0;
            params->hwtmout_cnt = 0;
            params->ck_lock_num = 0;
            params->tr_lock_num = 0;
            params->cr_lock_num = 0;
            params->rs_rev = rs_input;
            params->timeout = 0;

            AUTOSCAN_PRINTF("Reset: freq = %d, rs_input = %d at time %d\n", 
                    (int)params->freq, (int)rs_input, (int)params->timeout);
        }
    }

    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;
}

static INT32 nim_s3501_get_lock_status(struct nim_device *dev,NIM_SEARCH_PARAMS *params)
{
    UINT8 intindex = 0;
    UINT8 intdata = 0;
    INT32 ret=RET_CONTINUE;

    if(dev == NULL || params == NULL)
    {
	   return ERR_FAILURE;
	}  
    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);

    if(params->tr_lock_thr == params->timeout)
    {
        if((params->lock_monitor & 0xfc) == 0x00)
        {
            AUTOSCAN_PRINTF("        Fault TP, exit without TR lock \n");
            return ERR_FAILED;
        }
    }
    else if(params->cr_lock_thr == params->timeout)
    {
        if(((params->lock_monitor & 0xf8) == 0x00) &  (params->tr_lock_num < TR_LOCK_CNT_5))
        {
            AUTOSCAN_PRINTF("        Fault TP, exit without CR lock,  tr_lock_num %d\n", (int)params->tr_lock_num);
            return ERR_FAILED;
        }
    }
    else if(params->fs_lock_thr == params->timeout)
    {
        if(((params->lock_monitor & 0xf0) == 0x00) &  (params->cr_lock_num < CR_LOCK_CNT_5))
        {
            AUTOSCAN_PRINTF("        Fault TP, exit without Frame lock, cr_lock_num %d\n", (int)params->cr_lock_num);
            return ERR_FAILED;
        }
    }

    nim_reg_read(dev, R00_CTRL, &intindex, 1);
    if (0 == (intindex & 0x10))
    {
        nim_reg_read(dev, R02_IERR, &intdata, 1);
        AUTOSCAN_PRINTF("        Interrupt register is 0x%02x\n", intdata);
        if (0x04 == (intdata & 0x04))
        {
            AUTOSCAN_PRINTF("        Can not lock chanel \n");
            return ERR_FAILED;
        }
        if (0x20 == (intdata & 0x20))
        {
            AUTOSCAN_PRINTF("        CR loss lock  \n");
        }
        if (0 != (intdata & 0x02))
        {
            return nim_s3501_locked_channel(dev,params);
        }
        nim_s3501_clear_int(dev);
    }

    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;
}




static INT32 nim_s3501_init_search(struct nim_device *dev,NIM_SEARCH_PARAMS *params)
{
    UINT8 data = 0;
    INT32 ret=RET_CONTINUE;

    if(dev == NULL || params == NULL)
    {
	   return ERR_FAILURE;
	}   
    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);

    params->intindex = 0;
    params->lock_monitor = 0;
    params->rs_input = 0;
    params->rs_rev = 0;
    params->lock = 200;
    params->hwtmout_cnt = 0;
    params->tr_lock_flag = 0;
    params->cr_lock_flag = 0;
    params->last_freq = 0;
    params->last_rs = 0;
    params->s2_lock_cnt = 0;
    params->low_sym = 0;
    params->timeout = 0;
    params->tr_lock_thr = 0;
    params->cr_lock_thr = 0;
    params->fs_lock_thr = 0;
    params->ck_lock_num = 0;
    params->tr_lock_num = 0;
    params->cr_lock_num = 0;


    //reset
    nim_s3501_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);

    params->low_sym = 0;
    nim_s3501_freq_offset_set(dev, params->low_sym, &params->freq);

    if (nim_s3501_i2c_open(dev))
    {
        return S3501_ERR_I2C_NO_ACK;
    }

    comm_delay(500);

    params->rs_input = params->rs;

    if(params->rs < SYM_2500)
    {
        params->tr_lock_thr = 10;
    }
    else
    {
        params->tr_lock_thr = 6;
    }

    if(params->rs <= SYM_3000)
    {
        params->cr_lock_thr = 30;
     }
    else if(params->rs < 10000)
    {
        params->cr_lock_thr = 20;
    }
    else
    {
        params->cr_lock_thr = 15;
     }

    params->fs_lock_thr = params->cr_lock_thr + 10;

    if(CRYSTAL_FREQ_15000 >= CRYSTAL_FREQ)
    {
        // if(rs_input<44000)
        if(params->rs_input < (CRYSTAL_FREQ * 440 / 135))
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

    if (params->priv->nim_tuner_control != NULL)
    {
        params->priv->nim_tuner_control(params->priv->tuner_id, params->freq, params->rs_input);
     }

    if (params->priv->nim_tuner_status != NULL)
    {
        params->priv->nim_tuner_status(params->priv->tuner_id, &params->lock);
    }
    //NIM_PRINTF("Tuner lock = %d\n",lock);

    comm_delay(500);
    params->lock = 200;

    if (nim_s3501_i2c_close(dev))
    {
        return S3501_ERR_I2C_NO_ACK;
    }

    nim_s3501_interrupt_mask_clean(dev);

    data = 0xff;
    nim_s3501_set_hw_timeout(dev, data);

    nim_s3501_agc1_ctrl(dev, params->low_sym, NIM_OPTR_SOFT_SEARCH);

    nim_s3501_set_rs(dev, params->rs);

    nim_s3501_freq_offset_reset1(dev, params->low_sym, params->delfreq);

    nim_s3501_set_fc_search_range(dev, NIM_OPTR_SOFT_SEARCH, params->rs);

    nim_s3501_rs_search_range(dev, NIM_OPTR_SOFT_SEARCH, params->rs);

    if (params->priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if((params->priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C) &&
            (params->priv->ul_status.m_tso_mode == 1))
        {
            nim_m3501c_close_dummy(dev); // include reset TSO
        }
    }

#ifdef HW_ADPT_CR
    if (params->priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B &&
          params->priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C)
    {
        data = 0xe1;
        nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);

        data = 0xf0 | 2;
        nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
        if(params->rs <= SYM_10000)
        {
        data = 0x02;
        }
        else
        {
        data = 0x00;
        }
        nim_reg_write(dev, R11_DCC_OF_I, &data, 1);
        data = 0x10;
        nim_reg_write(dev, R12_DCC_OF_Q, &data, 1);

#ifdef HW_ADPT_CR_MONITOR
        data = 0xe6;
#else
        data = 0xe0;
#endif

        nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);
    }
#endif

    nim_s3501_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);

    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;

}

/****************************************************************************************************
   For each evaluation TP,soft search HW Time out cnt can never great than 2 times.
****************************************************************************************************/
static INT32 nim_s3501_soft_search(struct nim_device *dev, UINT32 *est_rs, UINT32 *est_freq, INT32 delfreq)
{
	INT32 ret=RET_CONTINUE;
    NIM_SEARCH_PARAMS params;
	struct nim_s3501_private *priv = NULL;
	
    if((dev == NULL) || (est_rs == 0) || (est_freq == 0))
	{
       return ERR_FAILURE;
	}  
    priv = (struct nim_s3501_private *) dev->priv;

	
    NIM_PRINTF("Enter nim_s3501_soft_search:\n");

    if ((0 == *est_freq) || (0 == *est_rs))
    {
        return ERR_FAILED;
    } 
	
    AUTOSCAN_PRINTF("       Try TP : Freq = %4d, RS = %5d, delfc = %6d:\n ", (int)(*est_freq), (int)(*est_rs), (int)delfreq);
    
    comm_memset(&params, 0, sizeof(params));
    params.rs = *est_rs;
    params.freq = *est_freq;
    params.delfreq=delfreq;
    params.priv=priv;

    ret=nim_s3501_init_search(dev,&params);
    if(ret!=RET_CONTINUE)
    {
        return ret;
    }

    while (1)
    {
        if (params.lock == 0)
        {
            priv->nim_tuner_status(priv->tuner_id, &params.lock);
            if (params.lock == 1)
            {
                NIM_PRINTF("Tuner lock = %d at timeout = %d\n", (int)params.lock, (int)params.timeout);
            }
        }
        params.timeout ++ ;
        if (TIMEOUT_NUM_100 < params.timeout)
        {
            nim_s3501_clear_int(dev);
            AUTOSCAN_PRINTF("\tTimeout \n");
            return ERR_FAILED;
        }
        if (priv->ul_status.s3501_autoscan_stop_flag)
        {
            AUTOSCAN_PRINTF("\tleave fuction nim_s3501_soft_search\n");
            return SUCCESS;
        }
        comm_sleep(50);

        params.intindex = 0;
        nim_reg_read(dev, R04_STATUS, &params.intindex, 1);
        NIM_PRINTF("        Lock status is 0x%x\n", params.intindex);

        if (params.intindex & 0x40)
        {
             params.s2_lock_cnt ++;
        }
        else
        {
             params.s2_lock_cnt = 0;
        }

        if(params.intindex & 0x18)
        {
             ret=nim_s3501_r04_0x18(dev,&params);
             if(ret!=RET_CONTINUE)
             {
                 return ret;
             }
        }

        if((params.intindex & 0x04) == 0x04)
        {
             ret=nim_s3501_r04_0x04(dev,&params);
             if(ret!=RET_CONTINUE)
             {
                 return ret;
             }

        }

        ret=nim_s3501_get_lock_status(dev,&params);
        if(ret!=RET_CONTINUE)
        {
            return ret;
        }
    }
}
















