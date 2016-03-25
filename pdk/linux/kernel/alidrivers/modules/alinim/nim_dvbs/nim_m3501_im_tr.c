/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_m3501_im_02.c
*
*    Description:  m3501 im layer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#include "nim_m3501.h"


#define  TDATA_START      7
#define  SYM_3000 3000
#define  SYM_40000 40000
#define  PACKET_NUM_662 662
#define  FFT_VAL_127 127
#define  FFT_VAL_128 128
#define  CODE_RATE_4 4
#define  CRYSTAL_FREQ_15000 15000
#define  MAP_TYPE_3 3
#define  MAP_TYPE_5 5

#define    TDATA_SUM        4

#define FREQ_P_V        5750
#define FREQ_P_H        5150
#define KU_C_BAND_INTERFACE 8000
#define INTERVAL_CNT_16 16
#define TAB_ID_2 2
#define CNT_NUM_3 3
#define CUR_MAX_ITER_50 50
#define CUR_MAX_ITER_35 35
#define CR_IRS_VAL_6 6
#define CR_PRS_VAL_4 4
#define INTERVAL_CNT_10 10
#define CLK_SEL_6 6
#define SUM_VALUE_255 255


#define CNT_NUM_8 8
#define BUF_INDEX_2 2
#define BIT_RATE_98 98
#define BIT_RATE_50 50
#define CRYSTAL_FREQ_15000 15000
#define SYM_7000 7000
#define TEMP_VAL_200 200
#define TEMP_VAL_254 254
#define MAP_TYPE_2 2




static const UINT8 snr_tab[177] =
{
    0, 1, 3, 4, 5, 7, 8,
    9, 10, 11, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 23, 24, 25, 26,
    26, 27, 28, 29, 30, 31, 32, 33,
    34, 35, 35, 36, 37, 38, 39, 39,
    40, 41, 42, 42, 43, 44, 45, 45,
    46, 47, 47, 48, 49, 49, 50, 51,
    51, 52, 52, 53, 54, 54, 55, 55,
    56, 57, 57, 58, 58, 59, 59, 60,
    61, 61, 62, 62, 63, 63, 64, 64,
    65, 66, 66, 67, 67, 68, 68, 69,
    69, 70, 70, 71, 72, 72, 73, 73,
    74, 74, 75, 76, 76, 77, 77, 78,
    79, 79, 80, 80, 81, 82, 82, 83,
    84, 84, 85, 86, 86, 87, 88, 89,
    89, 90, 91, 92, 92, 93, 94, 95,
    96, 96, 97, 98, 99, 100, 101, 102,
    102, 103, 104, 105, 106, 107, 108, 109,
    110, 111, 113, 114, 115, 116, 117, 118,
    119, 120, 122, 123, 124, 125, 127, 128,
    129, 131, 132, 133, 135, 136, 138, 139,
    141, 142, 144, 145, 147, 148, 150, 152,
    153, 155
};

static INT32 nim_s3501_get_err(struct nim_device *dev)
{
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

static INT32 nim_s3501_clear_err(struct nim_device *dev)
{
    struct nim_s3501_private *priv =NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    priv->ul_status.m_err_cnts = 0x00;
    return SUCCESS;
}

static void nim_s3501_print_work_mode(UINT8 work_mode)
{
    switch(work_mode)
    {
    case 0 :
    {
        NIM_PRINTF("            work_mode is DVB-S \n");
        break;
    }
    case 1 :
    {
        NIM_PRINTF("            work_mode is DVB-S2 \n");
        break;
    }
    case 2 :
    {
        NIM_PRINTF("            work_mode is DVB-H8PSK \n");
        break;
    }
   default:
       break;
    }
}

static void nim_s3501_print_roll_off(UINT8 roll_off)
{
    switch(roll_off)
    {
    case 0 :
    {
        NIM_PRINTF("            roll_off is 0.35\n");
        break;
    }
    case 1 :
    {
        NIM_PRINTF("            roll_off is 0.25\n");
        break;
    }
    case 2 :
    {
        NIM_PRINTF("            roll_off is 0.20\n");
        break;
    }
    case 3 :
    {
        NIM_PRINTF("            roll_off is wrong\n");
        break;
    }
   default:
       break;
    }

}

static void nim_s3501_print_map_type(UINT8 work_mode,UINT8 map_type)
{
    if(1 == work_mode)
    {
        switch(map_type)
        {
        case 0 :
        {
            NIM_PRINTF("            map_type is HBCD\n");
            break;
        }
        case 1 :
        {
            NIM_PRINTF("            map_type is BPSK\n");
            break;
        }
        case 2 :
        {
            NIM_PRINTF("            map_type is QPSK\n");
            break;
        }
        case 3 :
        {
            NIM_PRINTF("            map_type is 8PSK\n");
            break;
        }
        case 4 :
        {
            NIM_PRINTF("            map_type is 16APSK\n");
            break;
        }
        case 5 :
        {
            NIM_PRINTF("            map_type is 32APSK\n");
            break;
        }
        default:
            break;
        }
    }
    else
    {
        NIM_PRINTF("            map_type is QPSK\n");
    }
}

static void nim_s3501_print_code_rate(UINT8 work_mode,UINT8 code_rate)
{
    if(0 == work_mode)
    {
        switch(code_rate)
        {
        case 0 :
        {
            NIM_PRINTF("            code_rate is 1/2\n");
            break;
        }
        case 1 :
        {
            NIM_PRINTF("            code_rate is 2/3\n");
            break;
        }
        case 2 :
        {
            NIM_PRINTF("            code_rate is 3/4\n");
            break;
        }
        case 3 :
        {
            NIM_PRINTF("            code_rate is 5/6\n");
            break;
        }
        case 4 :
        {
            NIM_PRINTF("            code_rate wrong\n");
            break;
        }
        case 5 :
        {
            NIM_PRINTF("            code_rate is 7/8\n");
            break;
        }
        default:
            break;
        }

    }
    else if(1 == work_mode)
    {
        switch(code_rate)
        {
        case 0 :
        {
            NIM_PRINTF("            code_rate is 1/4\n");
            break;
        }
        case 1 :
        {
            NIM_PRINTF("            code_rate is 1/3\n");
            break;
        }
        case 2 :
        {
            NIM_PRINTF("            code_rate is 2/5\n");
            break;
        }
        case 3 :
        {
            NIM_PRINTF("            code_rate is 1/2\n");
            break;
        }
        case 4 :
        {
            NIM_PRINTF("            code_rate is 3/5\n");
            break;
        }
        case 5 :
        {
            NIM_PRINTF("            code_rate is 2/3\n");
            break;
        }
        case 6 :
        {
            NIM_PRINTF("            code_rate is 3/4\n");
            break;
        }
        case 7 :
        {
            NIM_PRINTF("            code_rate is 4/5\n");
            break;
        }
        case 8 :
        {
            NIM_PRINTF("            code_rate is 5/6\n");
            break;
        }
        case 9 :
        {
            NIM_PRINTF("            code_rate is 8/9\n");
            break;
        }
        case 10 :
        {
            NIM_PRINTF("            code_rate is 9/10\n");
            break;
        }
        default:
             break;
        }

    }

}
UINT32 nim_s3501_set_lock_times(UINT32 sym)
{
    UINT32 locktimes = 200;

    if (sym > SYM_40000)
    {
        locktimes = 204;
    }
    else if (sym < 2000)
    {
        locktimes = 2000;
    }
    else if (sym < 4000)
    {
        locktimes = 1604 - sym / 40;
    }
    else if (sym < 6500)
    {
        locktimes = 1004 - sym / 60;
    }
    else
    {
        locktimes = 604 - sym / 100;
    }

	return locktimes;
}

static void nim_s3501_set_snr_thre(struct nim_s3501_private *priv,UINT8 code_rate)
{
    if(priv == NULL)
    {
	    return;
	}	
    // S2, 8PSK
    if (code_rate == CODE_RATE_4)
    {
        // coderate3/5
        priv->t_param.t_snr_thre1 = 30;
        priv->t_param.t_snr_thre2 = 45;
        priv->t_param.t_snr_thre3 = 85;
    }
    else if ((code_rate == 5) || (code_rate == 6))
    {
        // coderate2/3,3/4
        priv->t_param.t_snr_thre1 = 35;
        priv->t_param.t_snr_thre2 = 55;
    }
    else if (code_rate == 7)
    {
        // coderate5/6
        priv->t_param.t_snr_thre1 = 55;
        priv->t_param.t_snr_thre2 = 65;
    }
    else if (code_rate == 8)
    {
        // coderate8/9
        priv->t_param.t_snr_thre1 = 75;
    }
    else
    {
        // coderate9/10
        priv->t_param.t_snr_thre1 = 80;
    }
}


static void nim_s3501_set_phase_noise_x(struct nim_device *dev,PHASE_NOISE_PARAM *params)
{
    UINT8 data = 0;
	struct nim_s3501_private *priv =NULL;
	
    if((dev == NULL) || (params== NULL) )
    {
	    return;
	}	
    priv = (struct nim_s3501_private *) dev->priv;


    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if (params->work_mode != M3501_DVBS2_MODE)
        {
            if(CRYSTAL_FREQ > CRYSTAL_FREQ_15000)
            {
               data = 0xf7;
            }
            else
            {
               data = 0x77;
            }
            nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);
            priv->ul_status.phase_err_check_status = 1000;
        }

        if((priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501B) ||
                ((priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C) &&
        (priv->ul_status.m_tso_mode == 0)))
        {
            priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_CLEAR;
            nim_s3501_set_ts_mode(dev, params->work_mode, params->map_type, params->code_rate,
                              params->rs, params->channel_change_flag);
            nim_m3501c_fec_ts_on(dev);
            nim_m3501_ts_on(dev);

            nim_reg_read(dev, R04_STATUS, &data, 1);
            if (0x3f == (data & 0x3f))
            {
                priv->ul_status.s3501d_lock_status = NIM_LOCK_STUS_NORMAL;
                if ((params->work_mode == M3501_DVBS2_MODE) && (params->map_type == MAP_TYPE_3))
                {
                    nim_s3501_set_phase_noise(dev);
                }
            }
            else
            {
                priv->ul_status.s3501d_lock_status = NIM_LOCK_STUS_SETTING;
            }
        }
    }
    else
    {
        //M3501A
        if (params->work_mode != M3501_DVBS2_MODE)
        {
            if(CRYSTAL_FREQ > CRYSTAL_FREQ_15000)
            {
              data = 0xf7;
            }
            else
            {
               data = 0x77;
            }
            nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);
        }
        else
        {
            if (params->map_type == MAP_TYPE_3)   // S2,8PSK
            {
                nim_s3501_set_phase_noise(dev);
            }
        }
    }

}


INT32 nim_s3501_set_dsp_clk (struct nim_device *dev, UINT8 clk_sel)
{
    UINT8 data = 0;
	struct nim_s3501_private *priv =NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if(priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C)
        {
            nim_reg_read(dev, RF0_HW_TSO_CTRL, &data, 1);
            if((clk_sel << CLK_SEL_6) != (data & 0xc0))
            {
                switch (clk_sel)
                {
                case 0:
                    data = (data & 0x3f) | (0 << 6) ;
                    nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
                    data = ((CRYSTAL_FREQ * 90  + 6750) / 13500);
                    nim_reg_write(dev, R90_DISEQC_CLK_RATIO, &data, 1);
                    break;
                case 1:
                    data = (data & 0x3f) | (1 << 6) ;
                    nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
                    data = ((CRYSTAL_FREQ * 98  + 6750) / 13500);
                    nim_reg_write(dev, R90_DISEQC_CLK_RATIO, &data, 1);
                    break;
                case 2:
                    data = (data & 0x3f) | (2 << 6) ;
                    nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
                    data = ((CRYSTAL_FREQ * 108  + 6750) / 13500);
                    nim_reg_write(dev, R90_DISEQC_CLK_RATIO, &data, 1);
                    break;
                case 3:
                    data = (data & 0x3f) | (3 << 6) ;
                    nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
                    data = ((CRYSTAL_FREQ * 135  + 6750) / 13500);
                    nim_reg_write(dev, R90_DISEQC_CLK_RATIO, &data, 1);
                    break;
               default:
                break;
                }
            }
        }
        else
        {
            NIM_PRINTF("Ali_NIM_Warning : M3501B need not set DSP clock\n");
        }
    }
    return SUCCESS;
}


INT32 nim_s3501_get_snr(struct nim_device *dev, UINT8 *snr)
{
    UINT8 lock = 0;
    UINT8 data = 0;
    UINT32 tdata = 0;
    UINT32 iter_num = 0;
    UINT32 sym_rate = 0;
    INT32 i = 0;
    INT32 total_iter = 0;
    INT32 sum = 0;
	struct nim_s3501_private *priv = NULL;
	
    if((dev == NULL) || (snr == NULL))
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    if (priv->tuner_config_data.qpsk_config & M3501_SIGNAL_DISPLAY_LIN)
    {
#define    TDATA_SUM_LIN 6
        nim_reg_read(dev, R04_STATUS, &data, 1);
        if (data & 0x08)
        {
            // CR lock
            sum = 0;
            total_iter = 0;
            for (i = 0; i < TDATA_SUM_LIN; i++)
            {
                nim_reg_read(dev, R45_CR_LCK_DETECT + 0x01, &data, 1);
                tdata = data << 8;
                nim_reg_read(dev, R45_CR_LCK_DETECT, &data, 1);
                tdata |= data;

                if (tdata & 0x8000)
                {
                   tdata = 0x10000 - tdata;
                }

                nim_s3501_get_ldpc(dev, &iter_num);
                total_iter += iter_num;
                tdata >>= 5;
                sum += tdata;
            }
            sum /= TDATA_SUM_LIN;
            total_iter /= TDATA_SUM_LIN;
            sum *= 3;
            sum /= 5;
            if (sum > SUM_VALUE_255)
            {
                sum = 255;
            }

            if (priv->t_param.t_last_snr == -1)
            {
                *snr = sum;
                priv->t_param.t_last_snr = *snr;
            }
            else
            {
                if (total_iter == priv->t_param.t_last_iter)
                {
                    *snr = priv->t_param.t_last_snr;
                    if (sum > priv->t_param.t_last_snr)
                    {
                        priv->t_param.t_last_snr ++;
                    }
                    else if (sum < priv->t_param.t_last_snr)
                    {
                        priv->t_param.t_last_snr --;
                    }
                }
                else if ((total_iter > priv->t_param.t_last_iter) && (sum < priv->t_param.t_last_snr))
                {
                    *snr = sum;
                    priv->t_param.t_last_snr = sum;
                }
                else if ((total_iter < priv->t_param.t_last_iter) && (sum > priv->t_param.t_last_snr))
                {
                    *snr = sum;
                    priv->t_param.t_last_snr = sum;
                }
                else
                {
                    *snr = priv->t_param.t_last_snr;
                }
            }
            priv->t_param.t_last_iter = total_iter;
        }
        else
        {
            // CR unlock
            *snr = 0;
            priv->t_param.t_last_snr = -1;
            priv->t_param.t_last_iter = -1;
        }
    }
    else
    {
        tdata = 0;
        for (i = 0; i < TDATA_SUM; i++)
        {
            data = snr_tab[nim_s3501_get_snr_index(dev)];
            tdata += data;
        }
        tdata /= TDATA_SUM;
        //CR04
        nim_reg_read(dev, R04_STATUS, &data, 1);
        if ((0x3F == data) | (0x7f == data) | (0xaf == data))
        {
            NIM_PRINTF("locked!\n");
        }
        else
        {
           tdata=nim_s3501_get_tdata(data);
        }
        *snr = tdata / 2;
    }

    if (priv->t_param.t_aver_snr == -1)
    {
        priv->t_param.t_aver_snr = (*snr);
    }
    else
    {
        priv->t_param.t_aver_snr += (((*snr) - priv->t_param.t_aver_snr) >> 2);
    }

    nim_s3501_get_lock(dev, &lock);
    nim_s3501_get_symbol_rate(dev, &sym_rate);
    nim_s3501_set_snr_status(dev,lock,sym_rate);

    if (priv->t_param.t_dynamic_power_en)
    {
        nim_s3501_dynamic_power(dev, (*snr));
    }

    return SUCCESS;
}




INT32 nim_s3501_get_bit_rate(struct nim_device *dev, UINT8 work_mode, UINT8 map_type, 
                              UINT8 code_rate, UINT32 rs, UINT32 *bit_rate)
{
    UINT32 data = 0;
    UINT32 temp = 0;
	struct nim_s3501_private *priv =NULL;
	
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
            data = temp + 1;
        }
        *bit_rate = data;
        NIM_PRINTF("xxx dvbs bit_rate is %d \n", (int)(*bit_rate));
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

        data += 2;
        *bit_rate = data;
        NIM_PRINTF("xxx dvbs2 bit_rate is %d \n", (int)(*bit_rate));
        return SUCCESS;
    }
}

UINT8 nim_s3501_get_snr_index(struct nim_device *dev)
{
    INT16 lpf_out16 = 0;
    INT16 agc2_ref5 = 0;
    INT32 snr_indx = 0;
    UINT8 data[2] ={0};
    UINT16 tdata[2] ={0};

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    //CR45
    nim_reg_read(dev, R45_CR_LCK_DETECT, &data[0], 1);
    //CR46
    nim_reg_read(dev, R45_CR_LCK_DETECT + 0x01, &data[1], 1);

    tdata[0] = (UINT16) data[0];
    tdata[1] = (UINT16) (data[1] << 8);
    lpf_out16 = (INT16) (tdata[0] + tdata[1]);
    lpf_out16 /= (16 * 2);

    //CR07
    nim_reg_read(dev, R07_AGC1_CTRL, &data[0], 1);
    agc2_ref5 = (INT16) (data[0] & 0x1F);

    snr_indx = (lpf_out16 * agc2_ref5 / 21) - 27;

    if (snr_indx < 0)
    {
        snr_indx = 0;
    }
    else if (snr_indx > 176)
    {
        snr_indx = 176;
    }

    return snr_indx;
}


INT32 nim_s3501_waiting_channel_lock(struct nim_device *dev, UINT32 freq, UINT32 sym)
{
    UINT32 timeout = 0;
    UINT32 locktimes = 200;
    UINT32 tempfreq = 0;
    UINT32 rs = 0;
    UINT8 work_mode = 0;
    UINT8 map_type = 0;
    UINT8 iqswap_flag = 0;
    UINT8 roll_off = 0;
    UINT8 modcod = 0;

    UINT8 intdata = 0;
    UINT8 code_rate = 0;
    UINT8 data = 0;
    UINT8 channel_change_flag = 1;
    PHASE_NOISE_PARAM params;
	struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    NIM_PRINTF("Enter nim_s3501_waiting_channel_lock : freq=%d, sym=%d\n", (int)freq, (int)sym);

	locktimes=nim_s3501_set_lock_times(sym);
    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        locktimes *= 3;
    }
    else
    {
        locktimes *= 2;
    }

    while (priv->ul_status.s3501_chanscan_stop_flag == 0)
    {
        timeout ++ ;
        if (locktimes < timeout)
        {
            priv->t_param.phase_noise_detect_finish = 1;
            nim_s3501_clear_int(dev);
            NIM_PRINTF("    timeout \n");
            priv->ul_status.s3501_chanscan_stop_flag = 0;
            if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
            {
                priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_SETTING;
            }
            return ERR_FAILED;
        }

        nim_reg_read(dev, R04_STATUS, &intdata, 1);
        if (0x3f == (intdata & 0x3f))
        {
            nim_s3501_reg_get_work_mode(dev, &work_mode);
            nim_s3501_print_work_mode(work_mode);

            if((priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B) &&
                (priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C))
            {
                if((priv->ul_status.m_tso_mode == 1))
                {
                    if(work_mode == M3501_DVBS2_MODE)
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
                    priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_CLEAR;
                    priv->ul_status.m_cur_freq = tempfreq;
                    priv->ul_status.s3501_chanscan_stop_flag = 0;
                }
            }

            // ------ Get information start
            tempfreq = freq;
            nim_s3501_reg_get_freq(dev, &tempfreq);
            NIM_PRINTF("            Freq is %d\n", (int)(LNB_LOACL_FREQ - tempfreq));

            nim_s3501_reg_get_symbol_rate(dev, &rs);
            NIM_PRINTF("            rs is %d\n", (int)rs);

            nim_s3501_reg_get_code_rate(dev, &code_rate);
            nim_s3501_print_code_rate(work_mode,code_rate);

            if(1 == work_mode)
            {
                nim_s3501_reg_get_roll_off(dev, &roll_off);
                nim_s3501_print_roll_off(roll_off);
            }
            if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
            {
                if(1 == work_mode)
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
                        modcod = modcod >> 2;
                        NIM_PRINTF("            Modcod is %d\n", modcod);
                    }
                }
            }
            nim_s3501_reg_get_iqswap_flag(dev, &iqswap_flag);
            NIM_PRINTF("            iqswap_flag is %d\n", iqswap_flag);

            nim_s3501_reg_get_map_type(dev, &map_type);
            nim_s3501_print_map_type(work_mode,map_type);

            if ((priv->ul_status.m_enable_dvbs2_hbcd_mode == 0) && ((map_type == 0) || (map_type == MAP_TYPE_5)))
            {
                NIM_PRINTF("            Demod Error: wrong map_type is %d\n", map_type);
            }
            else
            {
                NIM_PRINTF("        lock chanel \n");

#ifdef NIM_S3501_ASCAN_TOOLS
                tmp_lock = 0x01;
#endif
                if (work_mode == M3501_DVBS2_MODE)
                {
                    data = 0x52;
                    nim_reg_write(dev, R33_CR_CTRL + 0x04, &data, 1);
                    data = 0xba;
                    nim_reg_write(dev, RB5_CR_PRS_TRA, &data, 1);
                }

                comm_memset(&params,0,sizeof(PHASE_NOISE_PARAM));
                params.work_mode=work_mode;
                params.map_type=map_type;
                params.code_rate=code_rate;
                params.channel_change_flag=channel_change_flag;
                params.rs=rs;
                nim_s3501_set_phase_noise_x(dev,&params);

                priv->t_param.phase_noise_detect_finish = 1;
                if ((work_mode == M3501_DVBS2_MODE) && (map_type == MAP_TYPE_3) && (priv->t_param.t_phase_noise_detected == 0))
                {
                    nim_s3501_set_snr_thre(priv,code_rate);
                }

                if ((work_mode == M3501_DVBS2_MODE) && (map_type <= MAP_TYPE_3)) //only s2 need dynamic power
                {
                    priv->t_param.t_dynamic_power_en = 1;
                }

                priv->ul_status.m_cur_freq = tempfreq;// Keep current frequency.
                nim_s3501_clear_int(dev);
                priv->ul_status.s3501_chanscan_stop_flag = 0;
                return SUCCESS;
            }
        }
        else
        {
            if (priv->ul_status.s3501_chanscan_stop_flag)
            {
                priv->ul_status.s3501_chanscan_stop_flag = 0;
                if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
                {
                    priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_SETTING;
                }
                return ERR_FAILED;
            }
           // comm_delay(200);
            comm_sleep(10);
        }
    }

    priv->ul_status.s3501_chanscan_stop_flag = 0;
    return SUCCESS;
}

INT32 nim_s3501_i2c_open(struct nim_device *dev)
{
    struct nim_s3501_private *priv =NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    nim_s3501_clear_err(dev);
    if (priv->tuner_config_data.qpsk_config & M3501_I2C_THROUGH)
    {
        UINT8 data = 0;
        UINT8 ver_data = 0;

        data = 0xd4;
        nim_reg_write(dev, RCB_I2C_CFG, &data, 1);

        nim_reg_read(dev, RB3_PIN_SHARE_CTRL, &ver_data, 1);
        data = ver_data | 0x04;
        nim_reg_write(dev, RB3_PIN_SHARE_CTRL, &data, 1);
        comm_delay(200);

        if (nim_s3501_get_err(dev))
        {
        return ERR_FAILED;
        }
        return SUCCESS;
    }

    return SUCCESS;
}

INT32 nim_s3501_i2c_close(struct nim_device *dev)
{
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    nim_s3501_clear_err(dev);
    if (priv->tuner_config_data.qpsk_config & M3501_I2C_THROUGH)
    {
        UINT8 data = 0;
        UINT8 ver_data = 0;

        comm_delay(200);
        nim_reg_read(dev, RB3_PIN_SHARE_CTRL, &ver_data, 1);
        data = ver_data & 0xfb;
        nim_reg_write(dev, RB3_PIN_SHARE_CTRL, &data, 1);
        data = 0xc4;
        nim_reg_write(dev, RCB_I2C_CFG, &data, 1);
        NIM_PRINTF("     @@@@Exit nim through\n");

        if (nim_s3501_get_err(dev))
        {
            return ERR_FAILED;
        }
        return SUCCESS;
    }
    return SUCCESS;
}




INT32 nim_s3501_fft(struct nim_device *dev, UINT32 startfreq)
{
    UINT32 freq = 0;
    UINT8 lock = 200;
    UINT32 cur_f = 0;
    UINT32 last_f = 0;
    UINT32 cur_rs = 0;
    UINT32 last_rs = 0;
    UINT32 ch_num = 0;

    INT32 i = 0;
    struct nim_s3501_private *dev_priv = NULL;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    // date_time dt;
    // get_local_time(&dt);
    // AUTOSCAN_PRINTF("\nStep into nim_s3501_FFT:Blind Scan Current 90MHZ BW time %d:%d:%d\n",dt.hour,dt.min,dt.sec);

    dev_priv = dev->priv;
    freq = startfreq;

    //clear interrupt for software search
    if (dev_priv->ul_status.s3501_autoscan_stop_flag)
    {
#if defined(__NIM_LINUX_PLATFORM__)
        if(dev_priv->yet_return == FALSE)
        {
            nim_send_as_msg(dev_priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
            dev_priv->yet_return = TRUE;
        }

#endif


        NIM_PRINTF("    leave fuction nim_s3501_FFT\n");
        return SUCCESS;
    }


    if (nim_s3501_i2c_open(dev))
    {
        return S3501_ERR_I2C_NO_ACK;
     }

    dev_priv->nim_tuner_control(dev_priv->tuner_id, freq, 0);

#if defined(__NIM_TDS_PLATFORM__)
    comm_delay(0xffff * 4);
#endif

    if (dev_priv->nim_tuner_status != NULL)
     {
        dev_priv->nim_tuner_status(dev_priv->tuner_id, &lock);
     } 

    if (nim_s3501_i2c_close(dev))
    {
        return S3501_ERR_I2C_NO_ACK;
    }  

    dev_priv->ul_status.m_crnum = 0;
    dev_priv->ul_status.m_step_freq = nim_s3501_fft_find_channel(dev, &startfreq);

    AUTOSCAN_PRINTF("\tCurrent Time Blind Scan Range: From  %dMHz to %dMHz  have find %d channels\n", 
                    (int)(startfreq - 45), (int)(startfreq + 45), dev_priv->ul_status.m_crnum);
    for (i = 0; i < dev_priv->ul_status.m_crnum; i++)
    {
        AUTOSCAN_PRINTF("\tTP -> %d. Freq = %d, rs = %d\n", (int)i, (int)dev_priv->ul_status.m_freq[i], 
	                  (int)dev_priv->ul_status.m_rs[i]);
    }

    {
        if (dev_priv->ul_status.m_crnum > 1)
        {
            ch_num = dev_priv->ul_status.m_crnum;
            for (i = 1; i < dev_priv->ul_status.m_crnum; i++)
            {
                cur_f = dev_priv->ul_status.m_freq[i];
                last_f = dev_priv->ul_status.m_freq[i - 1];

                cur_rs = dev_priv->ul_status.m_rs[i];
                last_rs = dev_priv->ul_status.m_rs[i - 1];

                if (cur_f - last_f < (cur_rs + last_rs) / 2000)
                {
                    cur_f = last_f + (cur_f - last_f) * cur_rs / (cur_rs + last_rs);
                    cur_rs = last_rs + (cur_f - last_f) * 2000;
                    dev_priv->ul_status.m_freq[ch_num] = cur_f;
                    dev_priv->ul_status.m_rs[ch_num] = cur_rs;
                    ch_num ++;
                    AUTOSCAN_PRINTF("\tError detected TP, modified to -> %d. Freq=%d, rs=%d\n", (int)ch_num, (int)cur_f, (int)cur_rs);
                }
            }
            if (dev_priv->ul_status.m_crnum < ch_num)
            {
                AUTOSCAN_PRINTF("current FFT result is:\n");
                for (i = 0; i < 1024; i = i + 1)
                {    
		     AUTOSCAN_PRINTF("%d\n", (int)fft_energy_1024[i]);
		}
            }
            dev_priv->ul_status.m_crnum = ch_num;
        }

        for (i = 0; i < dev_priv->ul_status.m_crnum; i++)
        {
            ;
        }
    }

    return SUCCESS;
}



INT32 nim_s3501_ext_lnb_config(struct nim_device *dev, struct QPSK_TUNER_CONFIG_API *ptrqpsk_tuner)
{
    struct nim_s3501_private *priv_mem =NULL;
	
    if(dev == NULL || ptrqpsk_tuner == NULL)
    {
	    return RET_FAILURE;
	}	
    /****For external lnb controller config****/
    priv_mem = (struct nim_s3501_private *) dev->priv;

    priv_mem->ext_lnb_control = NULL;
    if (ptrqpsk_tuner->ext_lnb_config.ext_lnb_control)
    {
        UINT32 check_sum = 0;

        check_sum = (UINT32) (ptrqpsk_tuner->ext_lnb_config.ext_lnb_control);
        check_sum += ptrqpsk_tuner->ext_lnb_config.i2c_base_addr;
        check_sum += ptrqpsk_tuner->ext_lnb_config.i2c_type_id;
        if (check_sum == ptrqpsk_tuner->ext_lnb_config.param_check_sum)
        {
            priv_mem->ext_lnb_control = ptrqpsk_tuner->ext_lnb_config.ext_lnb_control;
            priv_mem->ext_lnb_control(0, LNB_CMD_ALLOC_ID, (UINT32) (&priv_mem->ext_lnb_id));
            priv_mem->ext_lnb_control(priv_mem->ext_lnb_id, LNB_CMD_INIT_CHIP,
                                   (UINT32) (&ptrqpsk_tuner->ext_lnb_config));
        }
    }

    return SUCCESS;
}




INT32 nim_change_ts_gap(struct nim_device *dev, UINT8 gap)
{
    UINT8 data = 0;
    UINT8 temp = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    if (gap == 0)
    {
        temp = 0x00;
        NIM_PRINTF("Set ts gap 0....\n");
    }
    else if (gap == 4)
    {
        temp = 1 << 1;
        NIM_PRINTF("Set ts gap 4....\n");
    }
    else if (gap == 8)
    {
        temp = 2 << 1;
        NIM_PRINTF("Set ts gap 8....\n");
    }
    else if (gap == 16)
    {
        temp = 3 << 1;
        NIM_PRINTF("Set ts gap 16....\n");
    }
    else
    {
        temp = 3 << 1;
        NIM_PRINTF("Nim error: wrong ts gap setting....\n");
    }
    nim_reg_read(dev, RD8_TS_OUT_SETTING, &data, 1);
    data = (data & 0xf9) | temp;
    nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
    return SUCCESS;
}




#if 0
static INT32 nim_m3501c_dvbs_ts_bps (struct nim_device *dev)
{
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if(priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C)
        {
            NIM_PRINTF("            nim_m3501c_dvbs_ts_bps \n");
        }
    }
    return SUCCESS;
}
#endif


