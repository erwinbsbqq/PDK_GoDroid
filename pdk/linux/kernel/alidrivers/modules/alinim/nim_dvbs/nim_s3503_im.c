/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_s3503_im.c
*
*    Description:  s3503 im layer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#include "nim_s3503.h"


#define	TEMP_VAL_254         254
#define	TEMP_VAL_200         200
#define MAP_TYPE_2           2
#define CNT_VAL_30           30
#define SYM_44000            44000
#define FFT_POINT_1024       1024
#define MAX_FFT_VAL_8388607  8388607
#define LOOP_NUM_2           2
#define EST_RREQ_945         945
#define EST_RREQ_2155        2155
#define	RET_VALUE_2          2

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


UINT8 	           s3503_debug_flag = 0;
static UINT8 	   autoscan_or_channelchange = 1; ///default:1:channelchange_mode


static UINT32 	   last_ch_fc = 0;
static UINT32      last_tp_freq = 0;
static UINT16 	   config_data = 0;

	
//--------Function get /check information---------

/*****************************************************************************
* static INT32 nim_s3503_get_bit_rate(struct nim_device *dev, UINT8 work_mode, UINT8 map_type, 
*                                     UINT8 code_rate, UINT32 rs, UINT32 *bit_rate)
* Description: S3501 set LNB votage 12V enable or not
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 flag
*
* Return Value: SUCCESS
*****************************************************************************/
INT32 nim_s3503_get_bit_rate(struct nim_device *dev, UINT8 work_mode, UINT8 map_type, UINT8 code_rate,
                             UINT32 rs, UINT32 *bit_rate)
{
    UINT32 data = 0;
    UINT32 temp = 0;
	struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL || bit_rate == NULL)
    {
	    return RET_FAILURE;
	}	
	
    priv = (struct nim_s3501_private *) dev->priv;

    if (work_mode != M3501_DVBS2_MODE) // DVBS mode
    {
        if (code_rate == 0)
        {
        temp = (rs * 2 * 1 + 1000) / 2000;
        }
        else if (code_rate == 1)
        {
        temp = (rs * 2 * 2 + 1500) / 3000;
        }
        else if (code_rate == 2)
        {
        temp = (rs * 2 * 3 + 2000) / 4000;
        }
        else if (code_rate == 3)
        {
        temp = (rs * 2 * 5 + 3000) / 6000;
         }
        else
        {
        temp = (rs * 2 * 7 + 4000) / 8000;
        }

        if (temp > TEMP_VAL_254)
        {
        data = 255;
         }
        else
        {
        data = temp + 1;    // add 1 M for margin
        }
        *bit_rate = data;
        NIM_PRINTF("xxx dvbs bit_rate is %d \n", *bit_rate);
        return SUCCESS;
    }
    else    //DVBS2 mode
    {
        if (code_rate == 0)
        {
        temp = (rs * 1 + 2000) / 4000;
        }
        else if (code_rate == 1)
        {
        temp = (rs * 1 + 1500) / 3000;
        }
        else if (code_rate == 2)
        {
        temp = (rs * 2 + 2500) / 5000;
        }
        else if (code_rate == 3)
        {
        temp = (rs * 1 + 1000) / 2000;
         }
        else if (code_rate == 4)
        {
        temp = (rs * 3 + 2500) / 5000;
        }
        else if (code_rate == 5)
        {
        temp = (rs * 2 + 1500) / 3000;
        }
        else if (code_rate == 6)
        {
        temp = (rs * 3 + 2000) / 4000;
         }
        else if (code_rate == 7)
        {
        temp = (rs * 4 + 2500) / 5000;
         }
        else if (code_rate == 8)
        {
        temp = (rs * 5 + 3000) / 6000;
         }
        else if (code_rate == 9)
        {
        temp = (rs * 8 + 4500) / 9000;
         }
        else
        {
        temp = (rs * 9 + 5000) / 10000;
         }

        if (map_type == MAP_TYPE_2)
        {
        temp = temp * 2;
        }
        else if (map_type == 3)
        {
        temp = temp * 3;
        }
        else if (map_type == 4)
        {
        temp = temp * 4;
         }
        else
        {
            NIM_PRINTF("Map type error: %02x \n", map_type);
        }

        if ((priv->tuner_config_data.qpsk_config & M3501_USE_188_MODE) == M3501_USE_188_MODE)
        {
            temp = temp;
        }
        else
        {
            temp = (temp * 204 + 94) / 188;
        }

        if (temp > TEMP_VAL_200)
        {
        data = 200;
         }
        else
        {
        data = temp;
         }
        NIM_PRINTF("Code rate is: %02x \n", code_rate);
        NIM_PRINTF("Map type is: %02x \n", map_type);

        data += 1; // Add 1M
        *bit_rate = data;
        NIM_PRINTF("xxx dvbs2 bit_rate is %d \n", *bit_rate);
        return SUCCESS;
    }
}

/*****************************************************************************
* static INT32 nim_s3503_get_bypass_buffer(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_get_bypass_buffer(struct nim_device *dev)
{
    // According to DMX IC spec, bypass buffer must locate in 8MB memory segment.
    // Bypass buffer size is (BYPASS_BUFFER_REG_SIZE * 188)
#define BYPASS_BUF_SIZE 0x20000     // 128 KB
#define BYPASS_BUF_MASK (BYPASS_BUF_SIZE - 1)
    UINT32 tmp = 0;
	struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
	{
        return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    priv->ul_status.adcdata_malloc_addr = (UINT8 *) comm_malloc(BYPASS_BUF_SIZE * 2);
    if (priv->ul_status.adcdata_malloc_addr == NULL)
    {
        return ERR_NO_MEM;
     }

    NIM_PRINTF("ADCdata_malloc_addr=0x%08x\n", priv->ul_status.adcdata_malloc_addr);

    comm_memset((int *)priv->ul_status.adcdata_malloc_addr, 0, BYPASS_BUF_SIZE * 2);
    tmp = ((UINT32) (priv->ul_status.adcdata_malloc_addr)) & BYPASS_BUF_MASK;
    if (tmp)
    {
        priv->ul_status.adcdata = priv->ul_status.adcdata_malloc_addr + BYPASS_BUF_SIZE - tmp;
     }
    else
    {
        priv->ul_status.adcdata = priv->ul_status.adcdata_malloc_addr;
    }

    NIM_PRINTF("ADCdata=0x%08x\n", priv->ul_status.adcdata);

    return SUCCESS;
}


/*****************************************************************************
* static INT32 nim_s3503_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32 *start_adr)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32 *start_adr)
{
    INT32 i = 0;
    INT32 index = 0;
    INT32 ave_energy = 0;
    INT32 max_est = 0;
    INT32 maxvalue = 0;
    UINT8 data = 0x10;
    UINT8 lock = 200;
    struct nim_s3501_private *dev_priv = NULL;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    dev_priv = (struct nim_s3501_private *)dev->priv;

    if (NULL == start_adr)
    {
        return ERR_ADDRESS;
    }
    //reset first
    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X90);

    data = 0x35;////0x2B;
    nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);

    dev_priv->nim_tuner_control(dev_priv->tuner_id, freq, 0);

    comm_sleep(10);
    dev_priv->nim_tuner_status(dev_priv->tuner_id, &lock);

    nim_s3503_cap_calculate_energy(dev);
    nim_s3501_smoothfilter();

    comm_memcpy((int *)start_adr, fft_energy_1024, sizeof(fft_energy_1024));
    //just deal with middle part of the data.
    //start from 256 to 767.
    //if needed, the range should be 0~1023, the whole array.

    start_adr[511] = (start_adr[510] + start_adr[512]) / 2;
    i = 0;
    ave_energy = 0;
    max_est = 0;
    for (index = 256; index <= 767; index++)
    {
        ave_energy += start_adr[index];
        if (i == (1 << (STATISTIC_LENGTH + 1)) - 1)
        {
            if (ave_energy > max_est)
            {
            max_est = ave_energy;
             }
            i = 0;
            ave_energy = 0;
        }
        else
        {
            i++;
        }
    }
    max_est = max_est / 8;
    maxvalue = max_est;

    for (index = 256; index <= 767; index++)
    {
        if ((start_adr[index] > (UINT32) maxvalue) && (start_adr[index] < (UINT32) (max_est * 2)))
        {
        maxvalue = start_adr[index];
        }
    }

    for (index = 256; index <= 767; index++)
    {
        if (start_adr[index] > (UINT32) maxvalue)
        {
        start_adr[index] = maxvalue;
         }
        //devider = maxvalue/250;
        start_adr[index] = start_adr[index] * 250 / maxvalue;
    }
    return SUCCESS;
}

/*****************************************************************************
* UINT8 nim_s3503_get_CRNum(struct nim_device *dev)
* Description: S3501 set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
UINT8 nim_s3503_get_crnum(struct nim_device *dev)
{
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    return ((struct nim_s3501_private *) (dev->priv))->ul_status.m_crnum;
}

/*****************************************************************************
* static INT32 nim_s3503_tuner_lock(struct nim_device *dev, UINT8 *tun_lock)
* Description: Get tuner lock status
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_tuner_lock(struct nim_device *dev, UINT8 *tun_lock)
{
    if(dev == NULL || tun_lock == NULL)
    {
	    return RET_FAILURE;
	}	
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    //UINT8 data;

    /* Setup tuner */
    NIM_MUTEX_ENTER(priv);
    priv->nim_tuner_status(priv->tuner_id, tun_lock);
    NIM_MUTEX_LEAVE(priv);

    return SUCCESS;
}


/*****************************************************************************
* INT32 nim_s3503_get_freq(struct nim_device *dev, UINT32 *freq)
* Read S3501 frequence
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *sym_rate         : Symbol rate in kHz
*
* Return Value: void
*****************************************************************************/
INT32 nim_s3503_get_freq(struct nim_device *dev, UINT32 *freq)
{
    if(dev == NULL || freq == NULL)
    {
	    return RET_FAILURE;
	}	
    nim_s3503_reg_get_freq(dev, freq);
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3503_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
* Read S3501 symbol rate
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *sym_rate 		: Symbol rate in kHz
*
* Return Value: void
*****************************************************************************/
INT32 nim_s3503_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
{
    if(dev == NULL || sym_rate == NULL)
    {
	    return RET_FAILURE;
	}	
    nim_s3503_reg_get_symbol_rate(dev, sym_rate);
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3503_get_code_rate(struct nim_device *dev, UINT8* code_rate)
* Description: Read S3501 code rate
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* code_rate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_get_code_rate(struct nim_device *dev, UINT8 *code_rate)
{
    if(dev == NULL || code_rate == NULL)
    {
	    return RET_FAILURE;
	}	
    nim_s3503_reg_get_code_rate(dev, code_rate);
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_get_tune_freq(struct nim_device *dev, INT32 *freq)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_get_tune_freq(struct nim_device *dev, INT32 *freq)
{
    if(dev == NULL || freq == NULL)
    {
	    return RET_FAILURE;
	}	
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    *freq += priv->ul_status.m_step_freq;
    if (*freq <= 0)
    {
        *freq = 30;
     }
    else if (*freq > 70)
     {
        *freq = 70;
     }  

    //NIM_PRINTF("  tune *freq step is %d\n", *freq );
    return SUCCESS;
}

#if 0
/*****************************************************************************
* static INT32 nim_s3503_get_err(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3503_get_err(struct nim_device *dev)
{
    if(dev == NULL)
        return RET_FAILURE;
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    if (priv->ul_status.m_err_cnts > 0x00)
    {
        return ERR_FAILED;
     }
    else
    {
        return SUCCESS;
    }
}
#endif


/*****************************************************************************
* INT32 nim_s3503_get_lock(struct nim_device *dev, UINT8 *lock)
*
*  Read FEC lock status
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: BOOL *fec_lock
*
*Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_get_lock(struct nim_device *dev, UINT8 *lock)
{
    UINT8 data = 0;
    UINT8 h8psk_lock = 0;

    if(dev == NULL || lock == NULL)
    {
	    return RET_FAILURE;
	}	
    comm_delay(150);
    nim_reg_read(dev, R04_STATUS, &data, 1);

    if ((data & 0x80) == 0x80)
    {
        h8psk_lock = 1;
    }
    else
    {
        h8psk_lock = 0;
    }

    if (h8psk_lock & ((data & 0x2f) == 0x2f))
    {
        *lock = 1;
    }
    else if ((data & 0x3f) == 0x3f)
    {
        *lock = 1;
    }
    else
    {
        *lock = 0;
    }
    comm_delay(150);

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3503_reg_get_code_rate(struct nim_device *dev, UINT8 *code_rate)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_reg_get_code_rate(struct nim_device *dev, UINT8 *code_rate)
{
    UINT8 data = 0;

    if(dev == NULL || code_rate == NULL)
    {
	    return RET_FAILURE;
	}	
    //  NIM_PRINTF("Enter Fuction nim_s3503_reg_get_code_rate \n");
    nim_reg_read(dev, R69_RPT_CARRIER + 0x02, &data, 1);
    *code_rate = ((data >> 1) & 0x0f);
    return SUCCESS;
}

//  Carcy add ldpc_code information .

/*****************************************************************************
* UINT32 nim_s3503_get_CURFreq(struct nim_device *dev)
* Description: S3501 set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
UINT32 nim_s3503_get_curfreq(struct nim_device *dev)
{
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    return ((struct nim_s3501_private *) dev->priv)->ul_status.m_cur_freq;
}

/*****************************************************************************
* INT32 nim_s3503_get_bitmode(struct nim_device *dev, UINT8 *bitmode)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_get_bitmode(struct nim_device *dev, UINT8 *bitmode)
{
    if(dev == NULL || bitmode == NULL)
    {
	    return RET_FAILURE;
	}	
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_1BIT_MODE)
    {
        *bitmode = 0x60;
     }
    else if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_2BIT_MODE)
    {
        *bitmode = 0x00;
     }
    else if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_4BIT_MODE)
    {
        *bitmode = 0x20;
     }
    else if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_8BIT_MODE)
    {
        *bitmode = 0x40;
     }
    else
    {
        *bitmode = 0x40;
     }
    return SUCCESS;
}


/*****************************************************************************
* static INT32 nim_s3503_get_dsp_clk(struct nim_device *dev, UINT8 *sample_rate)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_get_dsp_clk(struct nim_device *dev, UINT32 *sample_rate)
{
    UINT8 data = 0;

    if(dev == NULL || sample_rate == NULL)
    {
	    return RET_FAILURE;
	}	
    //  NIM_PRINTF("Enter Fuction nim_s3503_reg_get_modcode \n");

    nim_reg_read(dev, RF1_DSP_CLK_CTRL, &data, 1);
    data = data & 0x03;
    // bit[1:0] dsp clk sel: // 00: 135m, 01:96m, 10:112m
    if(0 == data)
    {
        *sample_rate = 135000; ////uint is KHz
    }
    else if(1 == data)
    {
        *sample_rate = 96430; ////uint is KHz
    }
    else if(2 == data)
    {
        *sample_rate = 112500; ////uint is KHz
    }

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3503_hw_check(struct nim_device *dev)
* Description: S3501 set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
INT32 nim_s3503_hw_check(struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    NIM_PRINTF("    Enter fuction nim_s3503_hw_check\n");

    nim_reg_read(dev, RA3_CHIP_ID + 0x01, &data, 1);
    if (data != 0x35)
    {
        return ERR_FAILED;
    }
    else
    {
        return SUCCESS;
    }
}
/*****************************************************************************
 * INT32 nim_S3501_adc2mem_cap(struct nim_device *dev, UINT32 startfreq, UINT32 sym, UINT32 *cap_buffer,
 *                             UINT32 dram_len)
 * capture dram_len bytes RX_ADC data to DRAM, and calculate FFT result for spectrum.
 *
 * Arguments:
 *  Parameter1: struct nim_device *dev
 *  Parameter2: UINT32 startfreq
 *  Parameter3: UINT32 *cap_buffer
 *       The base addr for DRAM buffer
 *  Parameter4: UINT32 sym
 *       if(sym == 0)    tuner_lpf_bandwidth = tuner's top bandwidth
 *       else            tuner_lpf_bandwidth = sym
 *  Parameter5: UINT32 dram_len
 *
 * Usage:
 *      cap_buffer=(UINT8 *) MALLOC(dram_len );
 *      nim_S3501_adc2mem_cap(dev,startfreq,sym,cap_buffer,dram_len);
 * Return Value: INT32
 *****************************************************************************/
INT32 nim_s3503_adc2mem_cap(struct nim_device *dev, UINT32 startfreq, UINT32 sym, UINT32 *cap_buffer, UINT32 dram_len)
{
    INT32  i = 0;
    UINT8  data = 0x10;
    UINT8  low_sym = 0;
    INT32  result = 200;
    UINT8  lock = 200;
    struct nim_s3501_private *dev_priv = NULL;
    UINT8  *dram_base_t = NULL;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    dev_priv = (struct nim_s3501_private *)dev->priv;

    dram_base_t = (UINT8 *)cap_buffer;
    if(NULL == dram_base_t)
    {
        NIM_PRINTF("S3503 adc2mem memory allocation error!\n");
        return ERR_FAILUE;
    }
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
    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
    nim_s3503_set_dsp_clk (dev, 96);
    nim_s3503_interrupt_mask_clean(dev);

    // Config AGC1 register CR07 to adjuct AGC1 target
      nim_s3503_set_agc1(dev, low_sym, NIM_OPTR_CHL_CHANGE); // AGC1 target is channel_change mode

    // a difference btw adc2dram and capture, in adc2dram agc1 must be set to unlock to make sure FSM at AGC1 state
    data = 0x55; ////0x2B; let AGC can not lock, try to modify tuner's gain
    nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);
    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X52);

    comm_delay(10000);  //wait 5ms for agc1 stable
    nim_s3503_adc2mem_calculate_energy(dev, dram_base_t, dram_len);

    data = 0x50; ////0x2B; let AGC  lock, try to modify tuner's gain
    nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);

    return SUCCESS;

}


/*****************************************************************************
* void nim_s3503_adc2mem_calculate_energy(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_adc2mem_calculate_energy(struct nim_device *dev, UINT8 *cap_buffer, UINT32 dram_len)
{
    INT32 i = 0;
    INT32 j = 0;
    INT32 k = 0;
    INT32 m = 0;
    INT32 n = 0;
    INT32 energy_real = 0;
    INT32 energy_imag = 0;
    INT32 energy = 0;
    INT32 energy_tmp = 0;
    INT32 fft_i = 0;
    INT32 fft_q = 0;
    UINT8 *data_ptr=NULL;
    struct nim_s3501_private *priv = NULL;
	
    if((dev == NULL) || (cap_buffer == NULL))
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    comm_memset(fft_energy_1024, 0, sizeof(fft_energy_1024));

    // capture 64Kbyte ADC2DRAM data
    nim_s3503_adc2mem_entity( dev,  cap_buffer, dram_len);
    //libc_printf("adc2dram begin\n");

    for(k = 0; k < 20; k++)
    {
        data_ptr = cap_buffer + k * 2048;

        for (i = 0; i < 1024; i++)
        {

            fft_i = data_ptr[2 * i];
            fft_q = data_ptr[2 * i + 1];

            if (fft_i & 0x80)
            {
            fft_i |= 0xffffff00;
             }
            if (fft_q & 0x80)
            {
            fft_q |= 0xffffff00;
            }

            m = 0;
            for (j = 0; j < 10; j++)
            {
                m <<= 1;
                m += ((i >> j) & 0x1);
            }//address change for 1024 FFT
            fft_i = fft_i << (FFT_BITWIDTH - 8);
            fft_q = fft_q << (FFT_BITWIDTH - 8);
            priv->ul_status.FFT_I_1024[m] = fft_i;
            priv->ul_status.FFT_Q_1024[m] = fft_q;
        }
        // calculate FFT
        R2FFT(priv->ul_status.FFT_I_1024, priv->ul_status.FFT_Q_1024);

        //accumulation
        for (i = 0; i < 1024; i++)
        {
            fft_i = priv->ul_status.FFT_I_1024[i];
            fft_q = priv->ul_status.FFT_Q_1024[i];

            energy_real = fft_i * fft_i;
            energy_imag = fft_q * fft_q;
            energy = energy_real + energy_imag;
            //because 20 times of accumulation needed, result will extend 5 bits
            //nevertherless maximum energy of possible signal(eg. 2Msps) is 5 times
            //lessthan theoretic max energy, ie. 2 bits less
            //if x is maximum FFT level, x^2/20 is maximum signal(2Msps) energy level(psd)
            //if we clip the MSB in FFT module(when FFT layer is 5), the maximum energy is
            //x^2/4
            //so we can get above conclusion
            //so we only need to reduce 3 bits LSB of accumulation result
            energy >>= 3;
            j = (i + 511) & 1023;//fold FFT sequence to natural sequence

            energy_tmp =  energy;

            if ((energy_tmp >> 20) & 1)
            {
                fft_energy_1024_tmp[j] = 1048575;//maximum unsigned value of 20 bits
            }
            else
            {
                fft_energy_1024_tmp[j] = energy_tmp;
            }
        }
        for(n = 0; n < 1024; n++)
        {
            fft_energy_1024[n] += fft_energy_1024_tmp[n];
        }
    }

    //  libc_printf("adc2dram end\n");
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3503_adc2mem_start(struct nim_device *dev, UINT32 startfreq, UINT32 sym, UINT32 *cap_buffer)
*    capture dram_len bytes RX_ADC data to DRAM, and calculate FFT result for spectrum for autoscan.
*
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 startfreq
*  Parameter3: UINT32 *cap_buffer
*       Not used, because the DRAM buffer is allocated and free in this function
*  Parameter4: UINT32 sym
*       always '0', set tuner_lpf_bandwidth = tuner's top bandwidth for autoscan
*  Parameter5: UINT32 dram_len
*
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_adc2mem_start(struct nim_device *dev, UINT32 startfreq, UINT32 sym, UINT32 *cap_buffer, UINT32 dram_len)
{
    INT32 i = 0;
    INT32   result = 200;
    UINT8   lock = 200;
    struct nim_s3501_private *dev_priv = NULL;
    UINT8  *dram_base_t = 0;

	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    
    dev_priv = (struct nim_s3501_private *)dev->priv;
    dram_base_t = (UINT8 *) comm_malloc(dram_len );
    if(NULL == dram_base_t)
    {
        NIM_PRINTF("S3503 adc2mem memory allocation error!\n");
        return ERR_FAILUE;
    }
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

    comm_delay(10000);  //wait 5ms for agc1 stable
    nim_s3503_adc2mem_calculate_energy(dev, dram_base_t, dram_len);

    comm_free(dram_base_t);

    return SUCCESS;
}

/*****************************************************************************
* void nim_s3503_cap_calculate_energy(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
void nim_s3503_cap_calculate_energy(struct nim_device *dev)
{
    INT32 i = 0;
    INT32 j = 0;
    INT32 k = 0;
    INT32 n = 0;
    INT32 energy_real = 0;
    INT32 energy_imag = 0;
    INT32 energy = 0;
    INT32 energy_tmp = 0;
    INT32 fft_i = 0;
    INT32 fft_q = 0;
    UINT32 oldidpos = 0;
    UINT32 packetnum = 0;
    struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	    return;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    comm_memset(fft_energy_1024, 0, sizeof(fft_energy_1024));

    oldidpos = 0;
    packetnum = 0;

    for (k = 0; k < 20; k++)
    {
        if (priv->ul_status.s3501_autoscan_stop_flag)
        {
            return;
        }
        //get FFT result
        nim_s3503_cap_fft_result_read(dev);
        //accumulation
        for (i = 0; i < 1024; i++)
        {
            fft_i = priv->ul_status.FFT_I_1024[i];
            fft_q = priv->ul_status.FFT_Q_1024[i];

            energy_real = fft_i * fft_i;
            energy_imag = fft_q * fft_q;
            energy = energy_real + energy_imag;
            //because 20 times of accumulation needed, result will extend 5 bits
            //nevertherless maximum energy of possible signal(eg. 2Msps) is 5 times
           //lessthan theoretic max energy, ie. 2 bits less
            //if x is maximum FFT level, x^2/20 is maximum signal(2Msps) energy level(psd)
            //if we clip the MSB in FFT module(when FFT layer is 5), the maximum energy is
            //x^2/4
            //so we can get above conclusion
            //so we only need to reduce 3 bits LSB of accumulation result
            energy >>= 3;
            j = (i + 511) & 1023;//fold FFT sequence to natural sequence
            energy_tmp =  energy;

            if ((energy_tmp >> 20) & 1)
            {
                fft_energy_1024_tmp[j] = 1048575;//maximum unsigned value of 20 bits
            }
            else
            {
                fft_energy_1024_tmp[j] = energy_tmp;
            }
        }
        for(n = 0; n < 1024; n++)
        {
            fft_energy_1024[n] += fft_energy_1024_tmp[n];
        }

    }
}



/*****************************************************************************
* static void nim_s3503_task(UINT32 param1, UINT32 param2)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
void nim_s3503_task(UINT32 param1, UINT32 param2)
{
    	
    struct nim_device *dev = NULL;
    struct nim_s3501_private *priv = NULL ;
    UINT32 v_cnt_val = 0x00;
    UINT32 sym_rate=0;
	UINT8  data_temp = 0;
	UINT8  data = 0;
	UINT8  modcod_temp=0;
	
#ifdef CHANNEL_CHANGE_ASYNC
    UINT32 flag_ptn = 0;

#endif
	
	if(param1 == 0)
    {
	    return ;
	}
	
	dev = (struct nim_device *) param1 ;
	priv = (struct nim_s3501_private *) dev->priv ;
    priv->tsk_status.m_sym_rate = 0x00;
    priv->tsk_status.m_code_rate = 0x00;
    priv->tsk_status.m_map_type = 0x00;
    priv->tsk_status.m_work_mode = 0x00;
    priv->tsk_status.m_info_data = 0x00;

    NIM_PRINTF("            Enter nim_s3503_task:\n");

    while (priv->work_alive)
    {
#ifdef CHANNEL_CHANGE_ASYNC

        flag_ptn = nim_flag_read(priv, NIM_FLAG_CHN_CHG_START, OSAL_TWF_ANDW | OSAL_TWF_CLR, 0);


        if((flag_ptn & NIM_FLAG_CHN_CHG_START) && (flag_ptn != OSAL_INVALID_ID))
        {
            nim_flag_clear(priv, NIM_FLAG_CHN_CHG_START);
            nim_flag_set(priv, NIM_FLAG_CHN_CHANGING);
            nim_s3503_waiting_channel_lock(dev, priv->cur_freq, priv->cur_sym);

            nim_flag_clear(priv, NIM_FLAG_CHN_CHANGING);
            NIM_PRINTF("\t\t Here is the task for S3503 wait channel lock \n");
        }
#endif

        if(priv->tsk_status.m_lock_flag == NIM_LOCK_STUS_CLEAR)
        {
            if(v_cnt_val  == CNT_VAL_30)
            {
                v_cnt_val = 0;
                NIM_PRINTF("\t\t Here is the task for M3501C plug_in_out \n");
                nim_s3503_debug_intask(dev);
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
                nim_s3503_get_lock(dev, &(priv->tsk_status.m_info_data));
                if (priv->tsk_status.m_info_data && (priv->t_param.t_i2c_err_flag == 0x00))
                {
                    nim_s3503_reg_get_symbol_rate(dev, &(priv->tsk_status.m_sym_rate));
                    nim_s3503_reg_get_code_rate(dev, &(priv->tsk_status.m_code_rate));
                    nim_s3503_reg_get_work_mode(dev, &(priv->tsk_status.m_work_mode));
                    nim_s3503_reg_get_map_type(dev, &(priv->tsk_status.m_map_type));
                    if ((priv->ul_status.m_enable_dvbs2_hbcd_mode == 0) &&
                            ((priv->tsk_status.m_map_type == 0) || (priv->tsk_status.m_map_type == 5)))
                    {
                        NIM_PRINTF("            Demod Error: wrong map_type is %d\n", priv->tsk_status.m_map_type);
                    }
                    else
                    {
                        nim_s3503_interrupt_clear(dev);
                        priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_CLEAR;
                    }
                }
            }
        }

        nim_reg_read(dev, RF8_MODCOD_RPT, &data, 1);
        data = data & 0x7f;
        nim_reg_read(dev, R04_STATUS, &data_temp, 1);
        if( (modcod_temp!=data)&&(data_temp==0x7f))
        {
            modcod_temp=data;
            nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
            nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);    
        }
			
        // For C3503 only
        if((priv->ul_status.m_s3501_sub_type == NIM_C3503_SUB_ID) && (autoscan_or_channelchange == 1))
        {
            nim_s3503_cr_new_adaptive_unlock_monitor(dev);
			sym_rate=100;
            nim_s3503_nframe_step_tso_setting(dev, &sym_rate, 0x00);
            
        }
        if(priv->tsk_status.m_lock_flag == NIM_LOCK_STUS_CLEAR)
        {
	       comm_sleep(100);
         }
        else
        {
	      comm_sleep(5);
         }
    }


}


//static  test_case = 1;
/*****************************************************************************
* static INT32 nim_s3503_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec)
* Description: S3501 set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
INT32 nim_s3503_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec)
{
    struct nim_s3501_private *priv = NULL;
    UINT8 lock = 200;
    UINT8 low_sym = 0;

#ifdef CHANNEL_CHANGE_ASYNC
    UINT32 flag_ptn = 0;
#endif
  
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}
	priv = (struct nim_s3501_private *) dev->priv;	
    //starttime = 0;
    NIM_PRINTF("    Enter Fuction nim_s3503_channel_change \n");
    NIM_PRINTF("    freq is %d\n", freq);
    NIM_PRINTF("    sym is %d\n", sym);
    NIM_PRINTF("    fec is %d\n", fec);

    autoscan_or_channelchange = 1;

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

#ifdef CHANNEL_CHANGE_ASYNC
    flag_ptn = nim_flag_read(priv, NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING, OSAL_TWF_ORW, 0);
    if((flag_ptn & (NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING)) && (flag_ptn != OSAL_INVALID_ID))
    {
        priv->ul_status.s3501_chanscan_stop_flag = 1;
        comm_sleep(2); //sleep 2ms
    }
#endif
    priv->ul_status.phase_err_check_status = 0;
    priv->ul_status.s3501d_lock_status = NIM_LOCK_STUS_NORMAL;
    priv->ul_status.m_setting_freq = freq;

    //reset first
    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);

    if(sym < SYM_44000)
    {
        nim_s3503_set_dsp_clk (dev, 96);
    }
    else
    {
        nim_s3503_set_dsp_clk (dev, 135);
     }

    if ((0 == freq) || (0 == sym))
    {
        return SUCCESS;
     }

    nim_s3503_sym_config(dev, sym);
    // time for channel change and sof search.
    nim_s3503_tr_cr_setting(dev, NIM_OPTR_CHL_CHANGE);

    low_sym = sym < 6500 ? 1 : 0;   /* Symbol rate is less than 10M, low symbol rate */

    nim_s3503_freq_offset_set(dev, low_sym, &freq);

    //result = 200;
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

    nim_s3503_set_adc(dev);

    nim_s3503_interrupt_mask_clean(dev);

    // hardware timeout setting
    nim_s3503_set_hw_timeout(dev, 0xff);

    // AGC1 setting
    nim_s3503_set_agc1(dev, low_sym, NIM_OPTR_CHL_CHANGE);

    // Set symbol rate
    nim_s3503_set_rs(dev, sym);

    // Set carry offset
    nim_s3503_freq_offset_reset(dev, low_sym);

    nim_s3503_cr_setting(dev, NIM_OPTR_CHL_CHANGE);

    nim_s3503_set_acq_workmode(dev, NIM_OPTR_CHL_CHANGE0);

    // set sweep range
    nim_s3503_set_fc_search_range(dev, NIM_OPTR_CHL_CHANGE, sym);
    nim_s3503_set_rs_search_range(dev, NIM_OPTR_CHL_CHANGE, sym);
    // LDPC parameter
    nim_s3503_fec_set_ldpc(dev, NIM_OPTR_CHL_CHANGE, 0x00, 0x01);
    // Carcy disable HBCD check, let time out. 2008-03-12
    nim_s3503_set_hbcd_timeout(dev, NIM_OPTR_CHL_CHANGE);

    nim_s3503_tso_dummy_on(dev);
    nim_s3503_tso_soft_cbr_off(dev);

    //    comm_delay(10);

    nim_s3503_cr_adaptive_configure (dev, sym);

    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);

    /// re-initial NEW CR TAB
    nim_s3503_cr_new_modcod_table_init(dev, sym);

    nim_s3503_cr_new_adaptive_unlock_monitor(dev);

    nim_s3503_nframe_step_tso_setting(dev, &sym, 0x01);
    //nim_s3503_cr_ld_setting(dev);
#if defined CHANNEL_CHANGE_ASYNC && !defined NIM_S3501_ASCAN_TOOLS
    priv->cur_freq = freq;
    priv->cur_sym = sym;

    nim_flag_set(priv, NIM_FLAG_CHN_CHG_START);
#else
    nim_s3503_waiting_channel_lock(dev, freq, sym);
#endif

    NIM_PRINTF("    Leave Fuction nim_s3503_channel_change \n");
    priv->ul_status.s3501_chanscan_stop_flag = 0;

    return SUCCESS;
}



static INT32 nim_s3503_autoscan_init(struct nim_device *dev,NIM_AUTOSCAN_PARAMS *params)
{
	UINT8 data = 0;
	INT32 ret=RET_CONTINUE;

#ifdef CHANNEL_CHANGE_ASYNC
    UINT32 flag_ptn = 0;
#endif	

    if((dev == NULL) || (params == NULL))
	{
        return RET_FAILURE;
	}	
    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);
	 
    params->start_t = os_get_tick_count();
    autoscan_or_channelchange = 0;
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
    params->priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_NORMAL;
    if ((params->pstauto_scan->sfreq < AS_FREQ_MIN) || (params->pstauto_scan->efreq > AS_FREQ_MAX) ||
         (params->pstauto_scan->sfreq > params->pstauto_scan->efreq))
    {
        return ERR_FAILUE;
    }
    // reset HW FSM
    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X90);
#ifdef __NIM_TDS_PLATFORM__
    nim_s3503_ioctl(dev, NIM_DRIVER_STOP_ATUOSCAN, 0);
#endif
#ifdef __NIM_LINUX_PLATFORM__
    if (params->priv->ul_status.s3501_autoscan_stop_flag)
    {
        nim_send_as_msg(params->priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
        params->priv->ul_status.s3501_autoscan_stop_flag = 0;
        return SUCCESS;
    }
#endif

    data = 0x1f; // setting for soft search function
    nim_reg_write(dev, R1B_TR_TIMEOUT_BAND, &data, 1);
    nim_s3503_cr_setting(dev, NIM_OPTR_SOFT_SEARCH);
    nim_s3503_tr_cr_setting(dev, NIM_OPTR_SOFT_SEARCH);
    nim_s3503_fec_set_ldpc(dev, NIM_OPTR_SOFT_SEARCH, 0x00, 0x01);
    nim_s3503_set_hbcd_timeout(dev, NIM_OPTR_SOFT_SEARCH);
    nim_s3503_autoscan_initial( dev);
    nim_s3503_tso_on(dev);
#ifdef ADC2DRAM_SUPPORT
    nim_s3503_set_adc(dev);
    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
    nim_s3503_set_dsp_clk (dev, 96);
    nim_s3503_set_adc(dev);
    nim_s3503_interrupt_mask_clean(dev);
    nim_s3503_set_agc1(dev, 0x00, NIM_OPTR_FFT_RESULT);
    // a difference btw adc2dram and capture, in adc2dram agc1 must be set to unlock to make sure FSM at AGC1 state
    data = 0x55; ////0x2B; let AGC can not lock, try to modify tuner's gain
    nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);
    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X52);
#endif
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

    if (nim_s3503_get_bypass_buffer(dev))
    {
        AUTOSCAN_PRINTF("\n ADCdata--> no enough memory!\n");
        comm_free(channel_spectrum);
        comm_free(channel_spectrum_tmp);
        return ERR_NO_MEM;
    }


    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;

}

static INT32 nim_s3503_fft_scan(struct nim_device *dev,NIM_AUTOSCAN_PARAMS *params)
{
    INT32 i = 0;
    UINT8 data = 0;
    INT32 ret=RET_CONTINUE;

    UINT32 step_freq = 0;
    INT32 scan_mode = 0;
    UINT32 temp_t = 0;
	
    if(dev == NULL || params == NULL)
    {
	    return RET_FAILURE;
	}	

    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);
    params->adc_sample_freq = 96;
    step_freq = params->adc_sample_freq / 2;
    for (params->fft_freq = params->pstauto_scan->sfreq; params->fft_freq < params->pstauto_scan->efreq;
                        params->fft_freq += step_freq)
    {
        comm_memset(fft_energy_1024, 0, sizeof(fft_energy_1024));
        while(1)
        {
#ifdef ADC2DRAM_SUPPORT
            nim_s3503_adc2mem_start(dev, params->fft_freq, 0, fft_energy_1024, BYPASS_BUF_SIZE_DMA);
#else
            nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
            nim_s3503_cap_start(dev, params->fft_freq, 0, fft_energy_1024);
#endif
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
                comm_free((int *)params->priv->ul_status.adcdata_malloc_addr);

#if defined(__NIM_LINUX_PLATFORM__)
                nim_send_as_msg(params->priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
                params->priv->ul_status.s3501_autoscan_stop_flag = 0;
#endif
                AUTOSCAN_PRINTF("\tleave fuction nim_s3503_autoscan\n");
                return SUCCESS;
            }
        }
        called_num++;
        if(1 == ((s3503_debug_flag >> 1) & 0x01))
        {
            AUTOSCAN_PRINTF("call_num=%d,Tuner_IF=%d \n", called_num, params->fft_freq);
            AUTOSCAN_PRINTF("FFT1024-> 0000 : 1000000 \n");
            for(i = 0; i < 1024; i++)
            {
            AUTOSCAN_PRINTF("FFT1024-> %4d : %d \n", i, fft_energy_1024[i]);
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
            comm_free((int *)params->priv->ul_status.adcdata_malloc_addr);

#if defined(__NIM_LINUX_PLATFORM__)
            nim_send_as_msg(params->priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
            params->priv->ul_status.s3501_autoscan_stop_flag = 0;
#endif

            AUTOSCAN_PRINTF("\tleave fuction nim_s3503_autoscan\n");
            return SUCCESS;
        }
    }
#ifdef ADC2DRAM_SUPPORT
    data = 0x50; ////0x2B; let AGC  lock, try to modify tuner's gain
    nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);
#endif
    comm_free((int *)params->priv->ul_status.adcdata_malloc_addr);
    params->end_t = os_get_tick_count();
    temp_t = params->end_t;
    AUTOSCAN_PRINTF("\tWideband FFT cost time %dms\n", params->end_t - params->start_t);
    if(1 == ((s3503_debug_flag >> 2) & 0x01))
    {
        for(i = 0; i < chlspec_num; i++)
        {
              AUTOSCAN_PRINTF("FFT_WBS-->%d : %d\n", i, channel_spectrum[i]);
        }
    }
    params->end_t = os_get_tick_count();
    AUTOSCAN_PRINTF("\tPrint FFT result cost time %dms\n", params->end_t - temp_t);

    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;

}


static INT32 nim_s3503_find_tp(struct nim_device *dev,NIM_AUTOSCAN_PARAMS *params)
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

    if(params == NULL)
    {
	    return RET_FAILURE;
	}	

    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);

    temp_t = params->end_t;

    AUTOSCAN_PRINTF("Max_FFT_energy=%d\n", max_fft_energy);
    loop = 1;
    if(max_fft_energy > MAX_FFT_VAL_8388607)
    {
        loop = 2;
    }
    AUTOSCAN_PRINTF("************loop=%d****************\n", loop);
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

    ////******find TP***
    tp_number = 0;
    comm_memset(frequency_est, 0, sizeof(frequency_est));
    comm_memset(symbol_rate_est, 0, sizeof(symbol_rate_est));

    for(loop_index = 0; loop_index < loop; loop_index++)
    {
        if(loop_index == 0)
        {
            nim_s3501_search_tp(chlspec_num, channel_spectrum, params->pstauto_scan->sfreq,
                            params->adc_sample_freq, loop_index);
            AUTOSCAN_PRINTF("Time %d : Find TP number is %d\n", loop_index, tp_number);
        }
        else if(loop_index == 1)
        {
            nim_s3501_search_tp(chlspec_num, channel_spectrum_tmp, params->pstauto_scan->sfreq,
                          params->adc_sample_freq, loop_index);
            AUTOSCAN_PRINTF("time %d : Find TP number is %d\n", loop_index, tp_number);
        }
    }
    comm_free(channel_spectrum);
    comm_free(channel_spectrum_tmp);
    params->end_t = os_get_tick_count();
    AUTOSCAN_PRINTF("\tSearch TP cost time %dms\n", params->end_t - temp_t);

#ifdef DEBUG_SOFT_SEARCH
    params->success = 0;
    tp_number = 1000;
    bug_freq[0] = 1889824; ///12446/V/1537
    bug_sym[0] = 2062;
    bug_freq[1] = 1762000; ////12321/V/1500
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

    AUTOSCAN_PRINTF("\\success = %d , TP_number = %d  \n",  params->success, tp_number);

    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;

}

static INT32 nim_s3503_get_tp(struct nim_device *dev,NIM_AUTOSCAN_PARAMS *params)
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

    if((dev == NULL) || (params == NULL))
    {
	    return RET_FAILURE;
	}	
    PRINTK_INFO("[%s]line=%d enter!\n",__FUNCTION__ , __LINE__);

     if (params->success == 0)
    {
        last_lock_freq = 0;
        last_lock_rs = 0;
        last_tp_freq = 0;
        final_est_freq = 0;
        final_est_sym_rate = 0;
        for (i = 0; i < tp_number; i++)
        {
            AUTOSCAN_PRINTF("\n\n----> Try %d-th TP [freq,Sym]=[%d, %d]\n", i, frequency_est[i], symbol_rate_est[i]);
            if (params->priv->ul_status.s3501_autoscan_stop_flag>0)
            {
                AUTOSCAN_PRINTF("\tleave fuction nim_s3503_autoscan\n");
#if defined(__NIM_LINUX_PLATFORM__)
                nim_send_as_msg(params->priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
                params->priv->ul_status.s3501_autoscan_stop_flag = 0;
#endif

                return SUCCESS;
            }
#ifdef DEBUG_SOFT_SEARCH
            params->priv->ul_status.m_crnum = 1;
            params->priv->ul_status.m_freq[0] = frequency_est[i];
            params->priv->ul_status.m_rs[0] = symbol_rate_est[i];
#else
            if((frequency_est[i] / 1024) < EST_RREQ_945)
            {
	             continue;
             }
            if((frequency_est[i] / 1024) > EST_RREQ_2155)
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
            if (SUCCESS == nim_s3503_channel_search(dev, 0))
            {
                AUTOSCAN_PRINTF("Find %d-th TP ---> SUCCESS!\n", i);
                nim_s3503_get_code_rate(dev, &cur_fec);
                cur_freq = final_est_freq;
                cur_sym =    final_est_sym_rate;
                last_lock_freq = cur_freq;
                last_lock_rs = cur_sym;
                if ((cur_freq >= params->pstauto_scan->sfreq) && (cur_freq <= params->pstauto_scan->efreq))
                {
                    if (params->pstauto_scan->unicable) //cur_freq -> [950, 2150]
                    {
                        AUTOSCAN_PRINTF("\tUnicable fft_freq: %d, cur_freq: %d, Fub: %d\n",
                             params->fft_freq, cur_freq, params->pstauto_scan->fub);

                        ret_value = nim_callback(params->pstauto_scan, params->priv, 1, 0,
                                   params->fft_freq+ cur_freq - params->pstauto_scan->fub, cur_sym, cur_fec, 0);
                    }
                    else
                    {
                        if((cur_freq > last_tp_freq - 2) && (cur_freq < last_tp_freq + 2))
                        {
                            //ret = pstauto_scan->callback(0, 0, Frequency_Est[i]/1024, 0, 0);
                            ret_value = nim_callback(params->pstauto_scan, params->priv, 0, 0,
                                           frequency_est[i] / 1024, 0, 0, 0);
                            AUTOSCAN_PRINTF("\tRescan a tp: fft_freq: %d, last_tp_freq: %d\n",
                                frequency_est[i] / 1024, last_tp_freq);
                        }
                        else
                        {
                            //ret = pstauto_scan->callback(1, 0, cur_freq, cur_sym, cur_fec);
                            ret_value = nim_callback(params->pstauto_scan, params->priv, 1, 0,
                                            cur_freq, cur_sym, cur_fec, 0);
                            last_tp_freq = cur_freq;
                        }
                    }
                }
                else
                {
                    if (params->priv->ul_status.s3501_autoscan_stop_flag>0)
                    {
                        AUTOSCAN_PRINTF("\tleave fuction nim_s3503_autoscan\n");
#if defined(__NIM_LINUX_PLATFORM__)
                        nim_send_as_msg(params->priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
                        params->priv->ul_status.s3501_autoscan_stop_flag = 0;
#endif

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
                //ret = pstauto_scan->callback(0, 0, Frequency_Est[i]/1024, 0, 0);
                ret_value = nim_callback(params->pstauto_scan, params->priv, 0, 0, frequency_est[i] / 1024, 0, 0, 0);

            }
            if (ret_value == RET_VALUE_2)
            {
                return SUCCESS;
            }
            else if (ret_value == 1)
            {
                nim_callback(params->pstauto_scan, params->priv, 2, 0, 0, 0, 0, 0);

#if defined(__NIM_LINUX_PLATFORM__)
                nim_send_as_msg(params->priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
                params->priv->ul_status.s3501_autoscan_stop_flag = 0;


#endif
                nim_s3501_autoscan_signal_input(dev, NIM_SIGNAL_INPUT_CLOSE);

            }
            params->end_t = os_get_tick_count();
            AUTOSCAN_PRINTF("\tTP %d cost time %dms\n", i, params->end_t - temp_t);
        }
    }

    PRINTK_INFO("[%s]line=%d end!\n",__FUNCTION__ , __LINE__);
    return ret;

}
/*****************************************************************************
* static INT32 nim_s3503_autoscan(struct nim_device *dev, struct NIM_Auto_Scan *pstauto_scan)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_autoscan(struct nim_device *dev, NIM_AUTO_SCAN_T *pstauto_scan)
{
    INT32 ret=RET_CONTINUE;
    NIM_AUTOSCAN_PARAMS params;
	struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL || pstauto_scan == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;


    NIM_PRINTF("\t Start autoscan \n");
    AUTOSCAN_PRINTF("Enter nim_s3503_autoscan : sfreq = %d, efreq = %d\n", pstauto_scan->sfreq, pstauto_scan->efreq);

    comm_memset(&params,0,sizeof(NIM_AUTOSCAN_PARAMS));
    params.priv=priv;
    params.pstauto_scan=pstauto_scan;

    ret=nim_s3503_autoscan_init(dev,&params);
    if(ret!=RET_CONTINUE)
    {
        return ret;
    }

    ret=nim_s3503_fft_scan(dev,&params);
    if(ret!=RET_CONTINUE)
    {
        return ret;
    }

    ret=nim_s3503_find_tp(dev,&params);
    if(ret!=RET_CONTINUE)
    {
        return ret;
    }

    ret=nim_s3503_get_tp(dev,&params);
    if(ret!=RET_CONTINUE)
    {
        return ret;
    }

#ifdef AUTOSCAN_DEBUG
    priv->tuner_config_data.qpsk_config = config_data;
#endif
    params.end_t = os_get_tick_count();

    AUTOSCAN_PRINTF("\tautoscan cost time   %d:%d   \n", (((params.end_t - params.start_t) + 500) / 1000 / 60),
   (((params.end_t - params.start_t) + 500) / 1000 - (((params.end_t - params.start_t) + 500) / 1000 / 60) * 60));
    NIM_PRINTF("\tfinish autoscan\n");

#if defined(__NIM_LINUX_PLATFORM__)
    nim_send_as_msg(priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
    priv->ul_status.s3501_autoscan_stop_flag = 0;
#endif

    return SUCCESS;

}


