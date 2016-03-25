/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_s3503_ic.c
*
*    Description:  s3503 ic layer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "nim_s3503.h"


#define SYM_40000 40000
#define SYM_18000 18000
#define SYM_10000 10000
#define SYM_1000 1000
#define SYM_2000 2000
#define SYM_2500 2500
#define SYM_3000 3000
#define SYM_44000 44000
#define SAMPLE_FREQ_90000 90000
#define TEST_MODE_2 2
#define TEMP_VAL_2 2
#define FFT_POINT_1024 1024
#define CAP_NUM_100 100
#define DEL_FREQ_512 512
#define DEL_FREQ_2 2
#define DEL_SYM_100 100
#define LOCK_CNT_4 4
#define TR_LOCK_CNT_5 5
#define CR_LOCK_CNT_5 5
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




static INT32  fft_energy_store[1024]={0};
INT32         s3503_snr_initial_en = 0; // Seen, 1 means first snr estimation done, set 1 in channel change


//-------function for capture by register-------------
static INT32  nim_s3503_cap_iq_enerage(struct nim_device *dev);

//--------Function essential progess --------
static INT32  nim_s3503_soft_search(struct nim_device *dev, UINT32 *rs, UINT32 *freq, INT32 delfreq);



/*****************************************************************************
* static INT32 nim_s3503_sym_config(struct nim_device *dev, UINT32 sym)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_sym_config(struct nim_device *dev, UINT32 sym)
{
    struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
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

/*****************************************************************************
* INT32 nim_s3503_cap_fft_find_channel(struct nim_device *dev, UINT32 *tune_freq)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_cap_fft_find_channel(struct nim_device *dev, UINT32 *tune_freq)
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
    UINT32 temp1 = 0;
    UINT8 data = 0;
    UINT32 sample_rate = 0;
    struct nim_s3501_private *priv = NULL;
	
    if((dev == NULL) || (tune_freq == NULL))
    {
	    return RET_FAILURE;
	}	
    
	priv = (struct nim_s3501_private *) dev->priv;

    nim_s3503_cap_calculate_energy(dev);
    NIM_PRINTF("tune_freq = %d\n", *tune_freq);

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

    nim_s3503_get_dsp_clk(dev, &sample_rate);

    if (1 == success)
    {
        for (i = 0; i < ch_number; i++)
        {
            temp = nim_s3501_multu64div(delta_fc_est[i], sample_rate, 90000); //
            tempfreq = (*tune_freq) * 1024 + temp;
            priv->ul_status.m_freq[priv->ul_status.m_crnum] = tempfreq;
            NIM_PRINTF("    m_Freq[%d] is %d\n", priv->ul_status.m_crnum,
                        priv->ul_status.m_freq[priv->ul_status.m_crnum]);
            priv->ul_status.m_rs[priv->ul_status.m_crnum] = nim_s3501_multu64div(symbolrate_est[i], sample_rate, 90000);
            priv->ul_status.m_crnum++;
        }
    }
    else
    {
        //NIM_PRINTF("err\n");
        NIM_PRINTF("    Base band width is %d, ch_number is%d\n", if_freq / 1024, ch_number);
    }
    if_freq = nim_s3501_multu64div(if_freq, sample_rate, SAMPLE_FREQ_90000); //
    tune_jump_freq = if_freq / 1024;
    return tune_jump_freq;
}

/*****************************************************************************
* void nim_s3503_cap_fft_result_read(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
void nim_s3503_cap_fft_result_read(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT8 read_data = 0;
    INT32 m = 0;
    struct nim_s3501_private *priv = NULL;
	 
    if(dev == NULL)
    {
	    return;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);

    nim_s3503_set_dsp_clk (dev, 96);

    nim_s3503_set_adc(dev);

    nim_s3503_interrupt_mask_clean(dev);

    // tso dummy off
    nim_s3503_tso_dummy_off(dev);
    //CR07
    nim_s3503_set_agc1(dev, 0x00, NIM_OPTR_FFT_RESULT);

    //CRBE
    data = 0x01;
    nim_reg_write(dev, RBD_CAP_PRM + 1, &data, 1);

    data = 0x55; ////0x2B; let AGC can not lock, try to modify tuner's gain
    nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);

    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X52);

    comm_delay(1000);
    comm_memset(priv->ul_status.adcdata_malloc_addr, 0, 0x20000 * 2);
#ifdef __NIM_TDS_PLATFORM__
    dmx_pause((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
#endif

    nim_s3501_autoscan_signal_input(dev, NIM_SIGNAL_INPUT_OPEN);

    data = 0x50; ////0x2B; let AGC  lock, try to modify tuner's gain
    nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);

    for (m = 0; m < 1000; m++)
    {
        comm_delay(1000);
        nim_reg_read(dev, R02_IERR, &read_data, 1);
        if (0x01 == (read_data & 0x01))
        {
            nim_s3503_cap_iq_enerage(dev);

            //do software 1024 points R2 FFT
            R2FFT(priv->ul_status.FFT_I_1024, priv->ul_status.FFT_Q_1024);
            NIM_PRINTF(" ADC DATA  transfer finish\n");
            break;
        }
    }
    nim_s3501_autoscan_signal_input(dev, NIM_SIGNAL_INPUT_CLOSE);
#ifdef __NIM_TDS_PLATFORM__
    dmx_start((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));
#endif
    //NIM_PRINTF("  leave Fuction nim_s3503_cap_fft_result_read \n");
}

#ifdef NIM_CAPTURE_SUPPORT
/*****************************************************************************
* INT32 nim_s3503_cap_start(struct nim_device *dev, UINT32 startfreq, UINT32 sym, UINT32 *cap_buffer)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_cap_start(struct nim_device *dev, UINT32 startfreq, UINT32 sym, UINT32 *cap_buffer)
{
    INT32 i = 0;
    UINT8   low_sym = 0;
    INT32   result = 200;
    UINT8   lock = 200;
    struct nim_s3501_private *dev_priv = NULL;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    dev_priv = (struct nim_s3501_private *)dev->priv;

    NIM_PRINTF("Enter nim_s3503_cap_start : Tuner_IF=%d\n", startfreq);

    nim_s3503_set_agc1(dev, low_sym, NIM_OPTR_CHL_CHANGE);

    result = dev_priv->nim_tuner_control(dev_priv->tuner_id, startfreq, sym);
    if (result == SUCCESS)
    {
        NIM_PRINTF("nim_tuner_control OK\n");
    }
    else
    {
        NIM_PRINTF("nim_tuner_control failed\n");
    }

    lock = 0;
    for( i = 0; i < 1000; i++ )
    {
        comm_delay(1000);
        result = dev_priv->nim_tuner_status(dev_priv->tuner_id, &lock);
        if (lock)
        {
            NIM_PRINTF("Tuner lock after %d times delay\n", i + 1);
            break;
        }
    }
    if(lock == 0)
    {
        NIM_PRINTF("Tuner can not lock\n");
    }

    nim_s3503_set_adc(dev);
    nim_s3503_set_agc1(dev, low_sym, NIM_OPTR_CHL_CHANGE);
    nim_s3503_cap_calculate_energy(dev);

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3503_cap(struct nim_device *dev, UINT32 startfreq, UINT32 *cap_buffer, UINT32 sym)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_cap(struct nim_device *dev, UINT32 startfreq, UINT32 *cap_buffer, UINT32 sym)
{
     struct nim_s3501_private *priv = NULL;
	 
    if((dev == NULL) || (cap_buffer == NULL))
    {
	    return RET_FAILURE;
	}	
    
	priv = (struct nim_s3501_private *) dev->priv;
    NIM_PRINTF("Enter nim_s3503_cap : Tuner_IF=%d\n", startfreq);
    if (nim_s3503_get_bypass_buffer(dev))
    {
        return ERR_NO_MEM;
    }
    nim_s3503_cap_start(dev, startfreq, sym, cap_buffer);
    comm_free((int *)priv->ul_status.adcdata_malloc_addr);
    return SUCCESS;
}

#endif


/*****************************************************************************
* static INT32 nim_S3501_adc2mem_entity(struct nim_device *dev, UINT32 startfreq, UINT32 *cap_buffer, UINT32 sym)
* capture RX_ADC data to DRAM
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 startfreq
*  Parameter3: UINT32 *cap_buffer
*       The base addr for DRAM buffer
*  Parameter4: UINT32 sym
*  Parameter5: UINT32 dram_len
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_adc2mem_entity(struct nim_device *dev,  UINT8 *cap_buffer, UINT32 dram_len)
{
    UINT8 data = 0;
    UINT8 is_test_mode = 0;
    UINT8 user_force_stop_adc_dma = 0;
    UINT32 dram_base = 0;

    if((dev == NULL) || (cap_buffer == NULL))
    {
	    return RET_FAILURE;
	}	
    dram_base = (UINT32)cap_buffer;
    //Reset ADCDMA.
    nim_reg_read(dev, 0x100, &data, 1);
    data |= 0x40;
    nim_reg_write(dev, 0x100, &data, 1);
    //check reset is done
    nim_reg_read(dev, 0x100, &data, 1);
    while(1)
    {
        if(data & 0x01)
        {
            NIM_PRINTF("Reset ADCDMA done\n");
            break;
        }
        nim_reg_read(dev, 0x100, &data, 1);
    }

    //Configure DRAM base address, unit is 32bytes
    dram_base &=  0x7fffffe0;
    NIM_PRINTF("Enter function %s with DRAM base address: 0x%08x\n", __FUNCTION__, dram_base);
    dram_base = (dram_base & 0xffffffff) >> 5;
    data = (UINT8)(dram_base & 0xff);
    nim_reg_write(dev, 0x104, &data, 1);
    data = (UINT8)((dram_base >> 8) & 0xff);
    nim_reg_write(dev, 0x105, &data, 1);
    data = (UINT8)((dram_base >> 16) & 0xff);
    nim_reg_write(dev, 0x106, &data, 1);
    data = (UINT8)((dram_base >> 24) & 0xff);
    nim_reg_write(dev, 0x107, &data, 1);

    //Configure DRAM length, can support 1G byte, but should consider with base address
    //and make sure that it is not beyond availbe sdram range.
    if (dram_len > 0x10000000)
    {
        dram_len = 0x10000000;
    }
    dram_len = (dram_len >> 15);    // unit is 32K byte

    NIM_PRINTF("\t Configure DRAM length: 0x%08x.\n", dram_len << 15);
    data = (UINT8)(dram_len & 0xff);
    nim_reg_write(dev, 0x108, &data, 1);
    data = (UINT8)((dram_len >> 8) & 0xff);
    nim_reg_write(dev, 0x109, &data, 1);

    //Configure capture points, reg102[3:0]
    //0:ADC_OUT 1:DCC_OUT 2:IQB_OUT 3:DC_OUT
    //4:FLT_OUT 5:AGC2_OUT 6:TR_OUT 7:EQ_OUT
    //8:PLDS_OUT 9:AGC3_OUT 10:DELAY_PLL_OUT 11:CR_OUT
    nim_reg_read(dev, 0x102, &data, 1);
    data |= 0x00;
    nim_reg_write(dev, 0x102, &data, 1);
    // select sram shared
    nim_reg_read(dev, 0x111, &data, 1);
    data |= 0x20;
    nim_reg_write(dev, 0x111, &data, 1);

    //Configure test pattern mode if in test mode.
    if (is_test_mode)
    {
        NIM_PRINTF("\t Configure test pattern mode and test mode.\n");
        nim_reg_read(dev, 0x101, &data, 1);    // enable test mode
        data |= 0x01;
        nim_reg_write(dev, 0x101, &data, 1);

        if (TEST_MODE_2 == is_test_mode)
        {
            nim_reg_read(dev, 0x100, &data, 1);    // test mode 1: pn seq, 0: counter by 1
            data |= 0x80;
            nim_reg_write(dev, 0x100, &data, 1);
        }
    }

    //Reset ADCDMA.
    nim_reg_read(dev, 0x100, &data, 1);
    data |= 0x40;
    nim_reg_write(dev, 0x100, &data, 1);

    //Start ADC DMA
    NIM_PRINTF("\t Start ADC DMA.\n");


    nim_reg_read(dev, 0x100, &data, 1);
    data |= 0x20;
    nim_reg_write(dev, 0x100, &data, 1);

    //Wait ADC DMA finish.
    NIM_PRINTF("\t Wait ADC DMA finish.\n");
    while(1)
    {
        if (user_force_stop_adc_dma)
        {
            user_force_stop_adc_dma = 0;
            nim_reg_read(dev, 0x100, &data, 1);
            data |= 0x40;
            nim_reg_write(dev, 0x100, &data, 1);
            NIM_PRINTF("ADC_DMA force stopped by user.\n");
            break;
        }
        nim_reg_read(dev, 0x100, &data, 1);
        NIM_PRINTF("\t Waiting: ... CR 0x100: 0x%02x.\n", data);

        if (data & 0x10)
        {
            NIM_PRINTF("ADC_DMA overflowed.\n");
            break;
        }

        if (data & 0x08)
        {
            NIM_PRINTF("ADC_DMA finished.\n");
            break;
        }
        comm_sleep(50);
    }

    NIM_PRINTF("%s Exit.\n", __FUNCTION__);

    //Reset ADCDMA.
    nim_reg_read(dev, 0x111, &data, 1);
    data &= 0xdf;
    nim_reg_write(dev, 0x111, &data, 1);

    nim_reg_read(dev, 0x100, &data, 1);
    data |= 0x40;
    nim_reg_write(dev, 0x100, &data, 1);

    return SUCCESS;
}
/*****************************************************************************
* static INT32 nim_s3503_set_agc1(struct nim_device *dev, UINT8 low_sym, UINT8 s_case)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_set_agc1(struct nim_device *dev, UINT8 low_sym, UINT8 s_case)
{
    struct nim_s3501_private *priv = NULL;
    UINT8 data = 0;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    // AGC1 setting
    if (priv->tuner_config_data.qpsk_config & M3501_NEW_AGC1)
    {
        switch (s_case)
        {
        case NIM_OPTR_CHL_CHANGE:
            if (1 == low_sym)
            {
               data = 0xaf;
            }
            else
            {
               data = 0xb5;
             }
            break;
        case NIM_OPTR_SOFT_SEARCH:
            if (1 == low_sym)
            {
                data = 0xb3;//0xb1;
            }
            else
            {
               data = 0xba;//0xb9;
            }
            break;
        case NIM_OPTR_FFT_RESULT:
                data = 0xb9;
            break;
        default:
         break;
        }
    }
    else
    {
        switch (s_case)
        {
        case NIM_OPTR_CHL_CHANGE:
        case NIM_OPTR_SOFT_SEARCH:
            if (1 == low_sym)
            {
            data = 0x3c;
            }
            else
            {
            data = 0x54;
            }
            break;
        case NIM_OPTR_FFT_RESULT:
            data = 0x54;
            break;
        default:
        break;
        }
    }
    if ((priv->tuner_config_data.qpsk_config & M3501_AGC_INVERT) == 0x0)  // STV6110's AGC be invert by QinHe
    {
        data = data ^ 0x80;
    }

    nim_reg_write(dev, R07_AGC1_CTRL, &data, 1);

    data = 0x58;
    nim_reg_write(dev, R07_AGC1_CTRL + 0x01, &data, 1);

    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_freq_offset_set(struct nim_device *dev, UINT8 low_sym, UINT32 *s_freq)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_freq_offset_set(struct nim_device *dev, UINT8 low_sym, UINT32 *s_freq)
{
    struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL || s_freq == NULL)
    {
	    return RET_FAILURE;
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

/*****************************************************************************
* static INT32 nim_s3503_freq_offset_reset(struct nim_device *dev, UINT8 low_sym)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_freq_offset_reset(struct nim_device *dev, UINT8 low_sym)
{
    struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    if (priv->tuner_config_data.qpsk_config & M3501_QPSK_FREQ_OFFSET)
    {
        if (1 == low_sym)
        {
        nim_s3503_set_freq_offset(dev, -3000);
        }
        else
        {
        nim_s3503_set_freq_offset(dev, 0);
        }
    }
    else
    {
        nim_s3503_set_freq_offset(dev, 0);
    }

    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_freq_offset_reset1(struct nim_device *dev, UINT8 low_sym, INT32 delfreq)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_freq_offset_reset1(struct nim_device *dev, UINT8 low_sym, INT32 delfreq)
{
    struct nim_s3501_private *priv = NULL;
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    if (priv->tuner_config_data.qpsk_config & M3501_QPSK_FREQ_OFFSET)
    {
        if (1 == low_sym)
        {
        nim_s3503_set_freq_offset(dev, -3000 + delfreq);
        }
        else
        {
        nim_s3503_set_freq_offset(dev, delfreq);
        }
    }
    else
    {
        nim_s3503_set_freq_offset(dev, delfreq);
    }

    return SUCCESS;
}


/*****************************************************************************
* static INT32 nim_s3503_hw_init(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_hw_init(struct nim_device *dev)
{
    struct nim_s3501_private *priv = NULL;
    UINT8 data = 0 ;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    
    priv = (struct nim_s3501_private *) dev->priv;

    // set TR lock symbol number thr, k unit.
    data = 0x1f; // setting for soft search function
    nim_reg_write(dev, R1B_TR_TIMEOUT_BAND, &data, 1);

    // Carcy change PL time out, for low symbol rate. 2008-03-12
    data = 0x84;
    nim_reg_write(dev, R28_PL_TIMEOUT_BND + 0x01, &data, 1);

    // Set Hardware time out
    //data = 0xff;
    //nim_reg_write(dev,R05_TIMEOUT_TRH, &data, 1);
    nim_s3503_set_hw_timeout(dev, 0xff);

    //----eq demod setting
    // Open EQ controll for QPSK and 8PSK
    data = 0x04;        //  set EQ control
    nim_reg_write(dev, R21_BEQ_CTRL, &data, 1);

    data = 0x24;        //  set EQ mask mode, mask EQ for 1/4,1/3,2/5 code rate
    nim_reg_write(dev, R25_BEQ_MASK, &data, 1);

    // Carcy add for 16APSK
    data = 0x6c;
    nim_reg_write(dev, R2A_PL_BND_CTRL + 0x02, &data, 1);
    //----eq demod setting end

    data = 0x81;
    nim_reg_write(dev, RFA_RESET_CTRL, &data, 1);

    nim_s3503_set_adc(dev);

    if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_8BIT_MODE)
    {
        nim_s3503_tso_initial (dev, 1, 1);
    }
    else if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_1BIT_MODE)
    {
        nim_s3503_tso_initial (dev, 1, 0);
     }

    nim_s3503_di_seq_c_initial(dev);
    nim_s3503_cr_adaptive_initial(dev);

    data = 0xe1;
    nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
    // For C3503 only
    if(priv->ul_status.m_s3501_sub_type == NIM_C3503_SUB_ID)
    {
        nim_reg_read (dev, RF1_DSP_CLK_CTRL, &data, 1);
        data = data | 0x08; // enable DVBS/S2 FEC clock gate.
        data = data | 0x10; // enable mdo_cnt_clean with sync
        nim_reg_write(dev, RF1_DSP_CLK_CTRL, &data, 1);
    }

    // For C3503 new CR only
    if(priv->ul_status.m_s3501_sub_type == NIM_C3503_SUB_ID)
    {
        nim_s3503_cr_new_tab_init(dev);
        nim_s3503_cr_adaptive_method_choice(dev, 0); //0:new;1:old;2:both;3:register
        // enable MERGE_ROUND
        nim_reg_read(dev, 0x137, &data, 1);
        data |= 0x20;
        nim_reg_write(dev, 0x137, &data, 1);
    }


    // For C3503 FEC only
    if(priv->ul_status.m_s3501_sub_type == NIM_C3503_SUB_ID)
    {
#ifdef INIT_32APSK_TARGET
        nim_s3503_set_32apsk_target(dev);
#endif
        nim_reg_read(dev, 0xb4, &data, 1);
        data &= 0x8f;
        data |= 0x10; // CENTRAL_UPDATE_STEP =1
        nim_reg_write(dev, 0xb4, &data, 1);

        nim_reg_read(dev, 0x21, &data, 1);
        data &= 0x1f;
        data |= 0x20; // OTHER_UPDATE_STEP =1
        nim_reg_write(dev, 0x21, &data, 1);

        // add by hongyu for ldpc maximum iteration
        data = 0xff;
        nim_reg_write(dev, R57_LDPC_CTRL, &data, 1);
        nim_reg_read(dev, 0xc1, &data, 1);
        data &= 0xfe; //disable LDPC_AVG_ITER_ACT
        data |= 0x30; // ldpc_max_iter_num[9:8]=0x3
        nim_reg_write(dev, 0xc1, &data, 1);
    }

    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_set_demod_ctrl(struct nim_device *dev, UINT8 c_value)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_set_demod_ctrl(struct nim_device *dev, UINT8 c_value)
{
    UINT8 data = c_value;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	

    nim_reg_write(dev, R00_CTRL, &data, 1);
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_set_hbcd_timeout(struct nim_device *dev, UINT8 s_case)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_set_hbcd_timeout(struct nim_device *dev, UINT8 s_case)
{
     struct nim_s3501_private *priv = NULL;
    UINT8 data = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;
    switch (s_case)
    {
    case NIM_OPTR_CHL_CHANGE:
    case NIM_OPTR_IOCTL:
        if (priv->t_param.t_reg_setting_switch & NIM_SWITCH_HBCD)
        {
            if (priv->ul_status.m_enable_dvbs2_hbcd_mode)
            {
            data = priv->ul_status.m_dvbs2_hbcd_enable_value;
             }
            else
            {
            data = 0x00;
            }
            nim_reg_write(dev, R47_HBCD_TIMEOUT, &data, 1);
            priv->t_param.t_reg_setting_switch &= ~NIM_SWITCH_HBCD;
        }
        break;
    case NIM_OPTR_SOFT_SEARCH:
        if (!(priv->t_param.t_reg_setting_switch & NIM_SWITCH_HBCD))
        {
            data = 0x00;
            nim_reg_write(dev, R47_HBCD_TIMEOUT, &data, 1);
            priv->t_param.t_reg_setting_switch |= NIM_SWITCH_HBCD;
        }
        break;
    case NIM_OPTR_HW_OPEN:
        nim_reg_read(dev, R47_HBCD_TIMEOUT, &(priv->ul_status.m_dvbs2_hbcd_enable_value), 1);
        break;
    default:
        NIM_PRINTF(" CMD for nim_s3503_set_hbcd_timeout ERROR!!!");
        break;
    }
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_set_acq_workmode(struct nim_device *dev, UINT8 s_case)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_set_acq_workmode(struct nim_device *dev, UINT8 s_case)
{
    struct nim_s3501_private *priv = NULL;
    UINT8 data = 0;
    UINT8 work_mode = 0x00;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
	priv = (struct nim_s3501_private *) dev->priv;
    switch (s_case)
    {
    case NIM_OPTR_CHL_CHANGE0:
    case NIM_OPTR_DYNAMIC_POW0:
        data = 0x73;
        nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);
        break;
    case NIM_OPTR_CHL_CHANGE:
        nim_s3503_reg_get_work_mode(dev, &work_mode);
        if (work_mode != M3501_DVBS2_MODE)// not in DVBS2 mode, key word: power_ctrl
        {
            priv->ul_status.phase_err_check_status = 1000;
            // slow down S2 clock
            data = 0x77;
            nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);
        }
        break;
    case NIM_OPTR_SOFT_SEARCH:
        data = 0x77;
        nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);
        break;
    case NIM_OPTR_HW_OPEN:
        // enable ADC
        data = 0x00;
        nim_reg_write(dev, RA0_RXADC_REG + 0x02, &data, 1);
        // enable S2 clock
        data = 0x73;
        nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);
        break;
    case NIM_OPTR_HW_CLOSE:
        // close ADC
        data = 0x07;
        nim_reg_write(dev, RA0_RXADC_REG + 0x02, &data, 1);
        // close S2 clock
        data = 0x3f;
        nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);
        break;
    default:
        NIM_PRINTF(" CMD for nim_s3503_set_acq_workmode ERROR!!!");
        break;
    }
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_cap_iq_enerage(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3503_cap_iq_enerage(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT8 buffer[8]={0};
    INT32 i = 0;
    INT32 j = 0;
    INT32 k = 0;
    UINT8 fft_data[3]={0};
    INT32 fft_i = 0;
    INT32 fft_q = 0;
    struct nim_s3501_private *priv = NULL;

    if(dev == NULL)
	{
        return RET_FAILURE;
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
        }//address change for 1024 FFT

        fft_i = fft_i << (FFT_BITWIDTH - 8);
        fft_q = fft_q << (FFT_BITWIDTH - 8);

        priv->ul_status.FFT_I_1024[k] = fft_i;
        priv->ul_status.FFT_Q_1024[k] = fft_q;
    }
#if 0
    if(1 == s3503_debug_flag && (0x01))
    {
        for(i = 0; i < 1024; i++)
        {
            AUTOSCAN_PRINTF("ADCdata%d:[%d,%d]\n", i, priv->ul_status.FFT_I_1024[i] , priv->ul_status.FFT_Q_1024[i]);
        }
    }
#endif
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_set_fc_search_range(struct nim_device *dev, UINT8 s_case, UINT32 rs)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_set_fc_search_range(struct nim_device *dev, UINT8 s_case, UINT32 rs)
{
    struct nim_s3501_private *priv = NULL;
    UINT8 data = 0;
    UINT32 temp = 0;
    UINT32 sample_rate = 0;

    if(dev == NULL)
	{
        return RET_FAILURE;
	}	
	priv = (struct nim_s3501_private *) dev->priv;
    nim_s3503_get_dsp_clk (dev, &sample_rate);
    sample_rate = ((sample_rate + 500) / 1000);
    switch (s_case)
    {
    case NIM_OPTR_SOFT_SEARCH:
    {
        if (rs > SYM_18000)
        {
        temp = 5 * 16;
        }
        else if (rs > 15000)
        {
        temp = 4 * 16;
        }
        else if (rs > 10000)
        {
        temp = 3 * 16;
         }
        else if (rs > 5000)
        {
        temp = 2 * 16;
        }
        else
        {
        temp = 1 * 16;
        }
        data = temp & 0xff;
        nim_reg_write(dev, R62_FC_SEARCH, &data, 1);
        data = (temp >> 8) & 0x3;
        if (rs > SYM_10000)
        {
        data |= 0xa0;    // amy change for 138E 12354V43000 est 1756/37333
        }
        else if (rs > 5000)
        {
        data |= 0xc0;       //amy change for 91.5E 3814/V/6666
        }
        else
        {
        data |= 0xb0;       //amy change for 91.5E 3629/V/2200
        }
        nim_reg_write(dev, R62_FC_SEARCH + 0x01, &data, 1);
        priv->t_param.t_reg_setting_switch |= NIM_SWITCH_FC;
    }
    break;
    case NIM_OPTR_CHL_CHANGE:
        //if (priv->t_Param.t_reg_setting_switch & NIM_SWITCH_FC)
    {
        // set sweep range
        //temp = (3 * 90 * 16+sample_rate/2) / sample_rate;
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
        NIM_PRINTF(" CMD for nim_s3503_set_FC_Search_Range ERROR!!!");
        break;
    }
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_set_rs_search_range(struct nim_device *dev, UINT8 s_case, UINT32 rs)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_set_rs_search_range(struct nim_device *dev, UINT8 s_case, UINT32 rs)
{
    struct nim_s3501_private *priv = NULL;
    UINT8 data = 0;
    UINT32 sample_rate = 0;
    UINT32 temp = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
	priv = (struct nim_s3501_private *) dev->priv;
		
    nim_s3503_get_dsp_clk (dev, &sample_rate);
    sample_rate = ((sample_rate + 500) / 1000);

    switch (s_case)
    {
    case NIM_OPTR_SOFT_SEARCH:
    {
        temp = (rs + 1000) / 2000;
        if (temp < TEMP_VAL_2)
        {
        temp = 2;
        }
        else if (temp > 11)
        {
        temp = 11;
        }
        temp = temp << 4;
        data = temp & 0xff;
        nim_reg_write(dev, R64_RS_SEARCH, &data, 1); ////CR64
        data = (temp >> 8) & 0x3;
        data |= 0xa0;        ////rs_search_step=2
        nim_reg_write(dev, R64_RS_SEARCH + 0x01, &data, 1);////CR65
        priv->t_param.t_reg_setting_switch |= NIM_SWITCH_RS;
    }
    break;
    case NIM_OPTR_CHL_CHANGE:
        if (priv->t_param.t_reg_setting_switch & NIM_SWITCH_RS)
        {
            if(sample_rate)
            {
			   temp = (3 * 90 * 16 + sample_rate / 2) / sample_rate;
			}
            else
            {
			    temp = 3 * 90 * 16;
			}	
            data = temp & 0xff;
            nim_reg_write(dev, R64_RS_SEARCH, &data, 1);
            data = (temp >> 8) & 0x3;
            data |= 0x30;
            nim_reg_write(dev, R64_RS_SEARCH + 0x01, &data, 1);
            priv->t_param.t_reg_setting_switch &= ~NIM_SWITCH_RS;
        }
        break;
    default:
        NIM_PRINTF(" CMD for nim_s3503_set_RS_Search_Range ERROR!!!");
        break;
    }
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_wideband_scan(struct nim_device *dev,UINT32 start_freq, UINT32 end_freq)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_wideband_scan(struct nim_device *dev, UINT32 start_freq, UINT32 end_freq)
{
    UINT32 fft_freq = 0;
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 adc_sample_freq = 0;
    UINT32 step_freq = 0;
    struct nim_s3501_private *priv = NULL;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
	priv = (struct nim_s3501_private *) dev->priv;
    NIM_PRINTF("Enter nim_s3503_WidebandScan\n");

    adc_sample_freq = 96;
    step_freq = adc_sample_freq / 2;

    if(SUCCESS == nim_s3503_wideband_scan_open(dev, start_freq, end_freq, step_freq))
    {
        for (fft_freq = start_freq; fft_freq < end_freq; fft_freq += step_freq)
        {
            nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
            comm_memset(fft_energy_1024, 0, sizeof(fft_energy_1024));
            for(j = 0; j < 100; j++)
            {
                nim_s3503_cap_start(dev, fft_freq, 0, fft_energy_1024);
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
                comm_free(priv->ul_status.adcdata_malloc_addr);
                NIM_PRINTF("nim_s3503_WidebandScan works wrong !\n");
                NIM_PRINTF("ERR_FAILURE, Leave nim_s3503_WidebandScan!\n");
                return ERR_FAILURE;
            }
            nim_s3501_fft_wideband_scan(fft_freq, adc_sample_freq);
        }
        comm_free(priv->ul_status.adcdata_malloc_addr);
        NIM_PRINTF("SUCCESS, Leave nim_s3503_WidebandScan!\n");
        return SUCCESS;
    }
    else
    {
        NIM_PRINTF("ERR_NO_MEM, Leave nim_s3503_WidebandScan!\n");
        return ERR_NO_MEM;
    }
}

/*****************************************************************************
* static INT32 nim_s3503_wideband_scan_open(struct nim_device *dev,UINT32 start_freq, UINT32 end_freq,UINT32 step_freq)
* Open
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_wideband_scan_open(struct nim_device *dev, UINT32 start_freq, UINT32 end_freq, UINT32 step_freq)
{
    UINT32 full_size = 0;

    NIM_PRINTF("Enter nim_s3503_WidebandScan_open\n");

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    last_tuner_if = 0;
    chlspec_num = 0;
    called_num = 0;

    full_size = (end_freq - start_freq) / step_freq;
    full_size = (full_size + 2) * 512;
    channel_spectrum = (INT32 *) comm_malloc(full_size * 4);
    if(channel_spectrum == NULL)
    {
        NIM_PRINTF("\n channel_spectrum--> no enough memory!\n");
        return ERR_NO_MEM;
    }

    if (nim_s3503_get_bypass_buffer(dev))
    {
        NIM_PRINTF("\n ADCdata--> no enough memory!\n");
        comm_free(channel_spectrum);
        channel_spectrum = NULL;
        return ERR_NO_MEM;
    }

    return SUCCESS;

}

/*****************************************************************************
* static INT32 nim_s3503_wideband_scan_close()
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_wideband_scan_close(void)
{
    comm_free(channel_spectrum);
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3503_channel_search(struct nim_device *dev, UINT32 crnum );
* Description: S3501 channel blind searching operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq              : Frequence
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_channel_search(struct nim_device *dev, UINT32 crnum)
{
    UINT32 freq = 0;
    UINT32 freq0 = 0;
    INT32 delfreq = 0;
    INT32 delfreq0 = 0;
    UINT32 sym = 0;
    struct nim_s3501_private *dev_priv = NULL;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    dev_priv = (struct nim_s3501_private *)dev->priv;
    NIM_PRINTF("ENTER nim_s3503_channel_search: TP is %d\n", crnum);

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
    AUTOSCAN_PRINTF("ENTER nim_s3503_channel_search: TP -> %d. Freq=%d, rs=%d\n", crnum, freq, sym);
    if (SUCCESS == nim_s3503_soft_search(dev, &dev_priv->ul_status.m_rs[crnum], &freq, delfreq))
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
        if (SUCCESS == nim_s3503_soft_search(dev, &dev_priv->ul_status.m_rs[crnum], &freq, delfreq))
        {
        return SUCCESS;
         }
        else if (SUCCESS == nim_s3503_soft_search(dev, &dev_priv->ul_status.m_rs[crnum], &freq0, delfreq0))
        {
        return SUCCESS;
        }
    }


    return ERR_FAILUE;
}

static INT32 nim_s3503_r04_0x18(struct nim_device *dev,NIM_SEARCH_PARAMS *params)
{
    UINT32 rs_input = 0;
    UINT8 work_mode = 0;
    UINT32 tempfreq = 0;
    INT32 del_freq = 0;
    INT32 del_rs = 0;
    UINT8 code_rate = 0;
    UINT8 map_type = 0;
    INT32 ret=RET_CONTINUE;

    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);

    if((dev == NULL) || (params == NULL))
    {
	    return RET_FAILURE;
	}	
    params->lock_monitor = params->lock_monitor | params->intindex;
    nim_s3503_reg_get_symbol_rate(dev, &rs_input);
    tempfreq = params->freq;
    nim_s3503_reg_get_freq(dev, &tempfreq);
    nim_s3503_reg_get_work_mode(dev, &work_mode);
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
    AUTOSCAN_PRINTF("       current lock rs_input is %d,  Freq is %d at time %d\n", rs_input,
                    tempfreq, params->timeout);

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
        NIM_PRINTF("    path2:lock chanel \n");
        NIM_PRINTF("            Freq is %d\n", (LNB_LOACL_FREQ - tempfreq));
        params->priv->ul_status.m_cur_freq = tempfreq ;
        NIM_PRINTF("            rs_input is %d\n", rs_input);
        nim_s3503_reg_get_code_rate(dev, &code_rate);
        NIM_PRINTF("            code_rate is %d\n", code_rate);
        NIM_PRINTF("            work_mode is %d\n", work_mode);
        nim_s3503_reg_get_map_type(dev, &map_type);
        NIM_PRINTF("            map_type is %d\n", map_type);
        AUTOSCAN_PRINTF("\tLock freq %d rs_input %d with del_Rs %d del_f %d \n",
               (LNB_LOACL_FREQ - tempfreq), rs_input, rs_input - params->rs, tempfreq - params->freq);
        AUTOSCAN_PRINTF("work_mode= %d, s2_lock_cnt= %d \n", work_mode, params->s2_lock_cnt);
        final_est_freq = tempfreq;
        final_est_sym_rate = rs_input;
#ifdef NIM_S3501_ASCAN_TOOLS
        nim_s3501_ascan_add_tp(ASCAN_ADD_REAL_TP, 0x00, tempfreq, rs_input, tempfreq > 1550 ? 1 : 0);
#endif
        AUTOSCAN_PRINTF("\t timeout = %d\n", params->timeout);
        nim_s3503_interrupt_clear(dev);
        return SUCCESS;
    }

    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;

}

static INT32 nim_s3503_r04_0x04(struct nim_device *dev,NIM_SEARCH_PARAMS *params)
{
    UINT32 rs_input = 0;
    UINT8 data = 0;
    INT32 ret=RET_CONTINUE;

    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);

    if((dev == NULL) || (params == NULL))
    {
	    return RET_FAILURE;
	}	
    params->lock_monitor = params->lock_monitor | 0x04;
    params->tr_lock_num++;
    if (params->tr_lock_flag == 0)
    {
        nim_s3503_reg_get_symbol_rate(dev, &rs_input);
        NIM_PRINTF("        RS=%d \n", rs_input);
        if(rs_input <= SYM_1000)
        {
            params->tr_lock_num--;
        }
        else
        {
            params->tr_lock_flag = 1;
            if(rs_input <= SYM_2000)
            {
               params->cr_lock_thr = 35;////Yatai5: 12321/V/1500
            }
            else if(rs_input <= 3000)
            {
               params->cr_lock_thr = 30;
            }
            else if(rs_input < 10000)
            {
               params->cr_lock_thr = 20;
            }
            else
            {
               params->cr_lock_thr = 15;
            }
            params->fs_lock_thr = params->cr_lock_thr + 10;
            nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
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
            params->lock = 200;
            nim_s3503_set_agc1(dev, params->low_sym, NIM_OPTR_CHL_CHANGE);
            nim_s3503_set_rs(dev, rs_input);
            nim_s3503_set_fc_search_range(dev, NIM_OPTR_SOFT_SEARCH, rs_input);
            nim_s3503_interrupt_clear(dev);
            data = 0x00;
            nim_reg_write(dev, R28_PL_TIMEOUT_BND, &data, 1);
            nim_reg_read(dev, R28_PL_TIMEOUT_BND + 1, &data, 1);
            data &= 0xe0;
            data |= 0x01;
            nim_reg_write(dev, R28_PL_TIMEOUT_BND + 1, &data, 1);
            nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);
            params->tr_lock_thr = params->tr_lock_thr - 2;
            params->lock_monitor = 0;
            params->hwtmout_cnt = 0;
            params->ck_lock_num = 0;
            params->tr_lock_num = 0;
            params->cr_lock_num = 0;
            params->rs_rev = rs_input;
            AUTOSCAN_PRINTF("Reset: freq = %d, rs_input = %d at time %d\n",
                             params->freq, rs_input, params->timeout);
            params->timeout = 0;
        }
    }

    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;

}




static INT32 nim_s3503_init_search(struct nim_device *dev,NIM_SEARCH_PARAMS *params)
{
    UINT8 data = 0;
    INT32 ret=RET_CONTINUE;

    if((dev == NULL) || (params == NULL))
    {
	    return RET_FAILURE;
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


    //reset first
    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
    nim_reg_read(dev, R07_AGC1_CTRL + 1, &data, 1);
    data = data & 0x7f;
    nim_reg_write(dev, R07_AGC1_CTRL + 1, &data, 1);


    params->low_sym = 0;                    // always use centre frequency
    nim_s3503_freq_offset_set(dev, params->low_sym, &params->freq);
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
    if(params->rs_input < SYM_44000)
    {
        nim_s3503_set_dsp_clk (dev, 96);
    }
    else
    {
        nim_s3503_set_dsp_clk (dev, 135);
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
    nim_s3503_interrupt_mask_clean(dev);
    data = 0xff;
    nim_s3503_set_hw_timeout(dev, data);
    nim_s3503_set_agc1(dev, params->low_sym, NIM_OPTR_SOFT_SEARCH);////CR07  AGC setting
    nim_s3503_set_rs(dev, params->rs);
    nim_s3503_freq_offset_reset1(dev, params->low_sym, params->delfreq);
    nim_s3503_set_fc_search_range(dev, NIM_OPTR_SOFT_SEARCH, params->rs);
    nim_s3503_set_rs_search_range(dev, NIM_OPTR_SOFT_SEARCH, params->rs);
    //nim_s3503_cr_adaptive_configure(dev, rs);  //// Seen add begin, copy from channel change
    nim_s3503_tso_soft_cbr_off(dev);
    nim_s3503_tso_dummy_on ( dev);
    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);


    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;

}


static INT32 nim_s3503_get_lock_status(struct nim_device *dev,NIM_SEARCH_PARAMS *params)
{
    UINT8  intindex = 0;
    UINT8  intdata = 0;
    UINT32 tempfreq = 0;
    UINT32 rs_input = 0;
    INT32  del_rs = 0;
    UINT8  code_rate = 0;
    UINT8  work_mode = 0;
    UINT8  map_type = 0;
    UINT8  modcod = 0;

    INT32  ret=RET_CONTINUE;


    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);

    if(dev == NULL || params == NULL)
    {
	    return RET_FAILURE;
	}	

    if(params->tr_lock_thr == params->timeout)
    {
        if((params->lock_monitor & 0xfc) == 0x00) // TR never lock
        {
            AUTOSCAN_PRINTF("        Fault TP, exit without TR lock \n");
            return ERR_FAILED;
        }
    }
    else if(params->cr_lock_thr == params->timeout)
    {
        if(((params->lock_monitor & 0xf8) == 0x00) &  (params->tr_lock_num < TR_LOCK_CNT_5)) // TR  lock
        {
            AUTOSCAN_PRINTF("        Fault TP, exit without CR lock,  tr_lock_num %d\n",
                          params->tr_lock_num);
            return ERR_FAILED;
        }
    }
    else if(params->fs_lock_thr == params->timeout)
    {
        if(((params->lock_monitor & 0xf0) == 0x00) &  (params->cr_lock_num < CR_LOCK_CNT_5)) // CR  lock
        {
            AUTOSCAN_PRINTF("        Fault TP, exit without Frame lock, cr_lock_num %d\n",
                           params->cr_lock_num);
            return ERR_FAILED;
        }
    }
    nim_reg_read(dev, R00_CTRL, &intindex, 1);  //捕获模式
    if (0 == (intindex & 0x10))
    {
        nim_reg_read(dev, R02_IERR, &intdata, 1);
        AUTOSCAN_PRINTF("       Interrupt register is 0x%02x\n", intdata);
        if (0x04 == (intdata & 0x04))  //不能锁定
        {
            AUTOSCAN_PRINTF("        Can not lock chanel \n");
            return ERR_FAILED;
        }
        if (0x20 == (intdata & 0x20))  //CR失锁
        {
            AUTOSCAN_PRINTF("        CR loss lock  \n");
        }
        if (0 != (intdata & 0x02))   //当无法锁定，打印当前的信息，包括传入的参数等
        {
            NIM_PRINTF("    path1:lock chanel \n");
            tempfreq = params->freq;
            nim_s3503_reg_get_freq(dev, &tempfreq);
            NIM_PRINTF("            Freq is %d\n", (LNB_LOACL_FREQ - tempfreq));
            params->priv->ul_status.m_cur_freq = tempfreq ;
            nim_s3503_reg_get_symbol_rate(dev, &rs_input);
            NIM_PRINTF("            rs_input is %d\n", rs_input);
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
            nim_s3503_reg_get_code_rate(dev, &code_rate);
            NIM_PRINTF("            code_rate is %d\n", code_rate);
            nim_s3503_reg_get_work_mode(dev, &work_mode);
            NIM_PRINTF("            work_mode is %d\n", work_mode);
            nim_s3503_reg_get_map_type(dev, &map_type);
            NIM_PRINTF("            map_type is %d\n", map_type);
            nim_s3503_reg_get_modcod(dev, &modcod);
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
            final_est_freq = tempfreq;
            final_est_sym_rate = rs_input;
#ifdef NIM_S3501_ASCAN_TOOLS
            nim_s3501_ascan_add_tp(ASCAN_ADD_REAL_TP, 0x00, tempfreq, rs_input, tempfreq > 1550 ? 1 : 0);
#endif
            AUTOSCAN_PRINTF("\tLock freq %d rs_input %d with del_Rs %d del_f %d \n",
                    (LNB_LOACL_FREQ - tempfreq), rs_input, rs_input - params->rs, tempfreq - params->freq);
            AUTOSCAN_PRINTF("\t timeout = %d\n", params->timeout);
            nim_s3503_interrupt_clear(dev);
            return SUCCESS;
        }
        nim_s3503_interrupt_clear(dev);
    }

    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;

}
/*****************************************************************************
* INT32 nim_s3503_soft_search(struct nim_device *dev, UINT32 *est_rs, UINT32 *est_freq, INT32 delfreq)
* Description: S3501 set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
static  INT32 nim_s3503_soft_search(struct nim_device *dev, UINT32 *est_rs, UINT32 *est_freq, INT32 delfreq)
{
    INT32 ret=RET_CONTINUE;
    NIM_SEARCH_PARAMS params;
	struct nim_s3501_private *priv = NULL;
	
    if((dev == NULL) || (est_rs == NULL) || (est_freq == NULL))
    {
	    return RET_FAILURE;
	}	
    
	priv = (struct nim_s3501_private *) dev->priv;

    if ((0 == *est_rs) || (0 == *est_freq))
    {
        return ERR_FAILED;
    }

    AUTOSCAN_PRINTF("       Try TP : Freq = %4d, RS = %5d, delfc = %6d:\n ", *est_freq, *est_rs, delfreq);
    comm_memset(&params, 0, sizeof(params));
    params.rs = *est_rs;
    params.freq = *est_freq;
    params.delfreq=delfreq;
    params.priv=priv;

    ret=nim_s3503_init_search(dev,&params);
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
                NIM_PRINTF("Tuner lock = %d at timeout = %d\n", params.lock, params.timeout);
            }
        }
        params.timeout ++ ;
        if (TIMEOUT_NUM_100 < params.timeout)
        {
            nim_s3503_interrupt_clear(dev);
            AUTOSCAN_PRINTF("\tTimeout \n");
            return ERR_FAILED;
        }
        if (priv->ul_status.s3501_autoscan_stop_flag)
        {
            AUTOSCAN_PRINTF("\tleave fuction nim_s3503_soft_search\n");
            return SUCCESS;
        }
        comm_sleep(50);

        params.intindex = 0;
        nim_reg_read(dev, R04_STATUS, &params.intindex, 1);
        if (params.intindex & 0x40)
        {
            params.s2_lock_cnt ++;
        }
        else
        {
            params.s2_lock_cnt = 0;
        }
        NIM_PRINTF("        Lock status is 0x%x\n", params.intindex);

        if(params.intindex & 0x18) //FS_LOCK or CR_lock
        {
             ret=nim_s3503_r04_0x18(dev,&params);
             if(ret!=RET_CONTINUE)
             {
                 return ret;
             }

        }
        if((params.intindex & 0x04) == 0x04) //TR_lock???
        {
             ret=nim_s3503_r04_0x04(dev,&params);
             if(ret!=RET_CONTINUE)
             {
                 return ret;
             }
        }
        //AUTOSCAN_PRINTF("     lock_monitor is 0x%x\n", lock_monitor );

        ret=nim_s3503_get_lock_status(dev,&params);
        if(ret!=RET_CONTINUE)
        {
            return ret;
        }

    }
}

/*****************************************************************************
* void nim_s3503_interrupt_clear(struct nim_device *dev)
* Description: S3501 set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
void nim_s3503_interrupt_clear(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT8 rdata = 0;

    if(dev == NULL)
    {
	    return;
	}	
    //clear the int
    //CR02
    data = 0x00;
    nim_reg_write(dev, R02_IERR, &data, 1);
    nim_reg_write(dev, R04_STATUS, &data, 1);

    nim_reg_read(dev, R00_CTRL, &rdata, 1);
    data = (rdata | 0x10);
    nim_s3503_set_demod_ctrl(dev, data);

    //NIM_PRINTF("    enter nim_s3503_interrupt_clear\n");
}

/*****************************************************************************
* void nim_s3503_set_rs(struct nim_device *dev, UINT8 coderate)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
void nim_s3503_set_rs(struct nim_device *dev, UINT32 rs)
{
    UINT8 data = 0;
    UINT8 ver_data[3] ={0};
    UINT32 sample_rate = 0;
    UINT32 temp = 0;
	
    if(dev == NULL)
    {
	    return;
	}	

    nim_s3503_get_dsp_clk(dev, &sample_rate);
    //NIM_PRINTF("\t\t dsp clock is %d\n", sample_rate);
    NIM_PRINTF("\t\t set ts %d with dsp clock %d\n", rs, sample_rate / 1000);
    temp = nim_s3501_multu64div(rs, 184320, sample_rate); // 184320 == 90000 * 2.048
    if(temp > 0x16800)
    {
        NIM_PRINTF("\t\t NIM_Error: Symbol rate set error %d\n", temp);
        temp = 0x16800;
    }
    //CR3F
    ver_data[0] = (UINT8) (temp & 0xFF);
    ver_data[1] = (UINT8) ((temp & 0xFF00) >> 8);
    ver_data[2] = (UINT8) ((temp & 0x10000) >> 16);
    nim_reg_write(dev, R5F_ACQ_SYM_RATE, ver_data, 3);
    if (rs < SYM_3000)
    {
        if (rs < SYM_2000)
        {
        data = 0x08;
        }
        else
        {
        data = 0x0a;
         }
        nim_reg_write(dev, R1B_TR_TIMEOUT_BAND, &data, 1);
    }
    return ;
}

/*****************************************************************************
* static INT32 nim_s3503_clear_err(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3503_clear_err(struct nim_device *dev)
{
    struct nim_s3501_private *priv = NULL;
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    
	priv = (struct nim_s3501_private *) dev->priv;

    priv->ul_status.m_err_cnts = 0x00;
    return SUCCESS;
}


