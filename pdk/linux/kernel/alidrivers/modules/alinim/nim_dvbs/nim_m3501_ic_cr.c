/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_m3501_ic_02.c
*
*    Description:  m3501 ic layer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "nim_m3501.h"

#define CNT_NUM_8          8
#define BUF_INDEX_2        2
#define BIT_RATE_98        98
#define BIT_RATE_50        50
#define CRYSTAL_FREQ_15000 15000
#define SYM_7000           7000
#define TEMP_VAL_200       200
#define TEMP_VAL_254       254
#define MAP_TYPE_2         2
#define DISEQC_DELAY       60      //adjust timing for DiSEqC circuit


static UINT8       va_22k = 0x00;

INT32 nim_s3501_open_ci_plus(struct nim_device *dev, UINT8 *ci_plus_flag)
{
    UINT8 data = 0;
	struct nim_s3501_private *priv =NULL;
	
    if(dev == NULL || ci_plus_flag == 0)
    {
	   return ERR_FAILURE;
	}  
    
	priv = (struct nim_s3501_private *) dev->priv;

    // For CI plus test.
    if (priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_S3501D)
    {
        data = 0x06;
     }
    else
    {
        data = 0x02;    // symbol period from reg, 2 cycle
    }
    nim_reg_write(dev, RAD_TSOUT_SYMB, &data, 1);
    NIM_PRINTF("open ci plus enable REG_ad = %02x \n", data);

    nim_reg_read(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
    data = data | 0x80;    // enable symbol period from reg
    nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);

    nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
    data = data | 0xe0;
    nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);

    nim_reg_read(dev, RDF_TS_OUT_DVBS2, &data, 1);
    data = (data & 0xfc) | 0x01;
    nim_reg_write(dev, RDF_TS_OUT_DVBS2, &data, 1);

    *ci_plus_flag = 1;

    return SUCCESS;
}


/*****************************************************************************
* INT32 nim_s3501_DiSEqC_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt)
*
*  defines DiSEqC operations
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 mode
*  Parameter3: UINT8* cmd
*  Parameter4: UINT8 cnt
*
* Return Value: void
*****************************************************************************/
INT32 nim_s3501_di_seq_c_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt)
{
    UINT8 data = 0;
    UINT8 temp = 0;
    UINT16 timeout = 0;
    UINT16 timer = 0;
    UINT8 i = 0;
  
    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}  
    NIM_PRINTF("[%s] mode = %d\n",__FUNCTION__, (int)mode);
    switch (mode)
    {
    case NIM_DISEQC_MODE_22KOFF:
    case NIM_DISEQC_MODE_22KON:
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        data = ((data & 0xF8) | mode);
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
        break;
    case NIM_DISEQC_MODE_BURST0:
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        //tone burst 0
        temp = 0x02;
        data = ((data & 0xF8) | temp);
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
        comm_sleep(16);
        break;
    case NIM_DISEQC_MODE_BURST1:
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        //tone bust 1
        temp = 0x03;
        data = ((data & 0xF8) | temp);
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
        comm_sleep(16);
        break;
    case NIM_DISEQC_MODE_BYTES:
		comm_sleep(DISEQC_DELAY);
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        va_22k = (data & 0x41);    // get DISEQC_22k origianl value
        if(cnt > CNT_NUM_8)
        {
            NIM_PRINTF("\t\t NIM_ERROR : Diseqc cnt larger than 8: cnt = %d\n", cnt);
            return ERR_FAILED;
        }
        else
        {
            // close 22K and set TX byte number
            data = ((data & 0xc0) | ((cnt - 1) << 3));
            nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
        }
        //write the tx data, max 8 byte
        for (i = 0; i < cnt; i++)
        {
            nim_reg_write(dev, (i + 0x7E), cmd + i, 1);
        }
        // remove clean interrupt, since reg7d is read only
        //write the control bits, start TX
        temp = 0x04;
        data = ((data & 0xF8) | temp);
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);

        //wait for DISEQC_EVENT register cleared before every transmission
        comm_sleep(1);

        //waiting for the send over
        timer = 0;
        timeout = 75 + 13 * cnt;
        while (timer < timeout)
        {
            nim_reg_read(dev, R7C_DISEQC_CTRL + 0x01, &data, 1);
             if ((0 != (data & 0x07))&&(timer>50))
            {
                break;
            }
            comm_sleep(10);
            timer += 10;
        }
        if (1 == (data & 0x07))
        {
            //resume DISEQC_22k origianl value
            nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
            data = ((data & 0xB8) | (va_22k));
            nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
            return  SUCCESS;
        }
        else
        {
            //resume DISEQC_22k origianl value
            nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
            data = ((data & 0xB8) | (va_22k));
            nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
            return ERR_FAILED;
        }
        break;
    case NIM_DISEQC_MODE_BYTES_EXT_STEP1:
		comm_sleep(DISEQC_DELAY);
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        va_22k = (data & 0x41);    // get DISEQC_22k origianl value
        if(cnt > CNT_NUM_8)
        {
            NIM_PRINTF("\t\t NIM_ERROR : Diseqc cnt larger than 8: cnt = %d\n", cnt);
            return ERR_FAILED;
        }
        else
        {
            // close 22K and set TX byte number
            data = ((data & 0xc0) | ((cnt - 1) << 3));
            nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
        }

        //write the data
        for (i = 0; i < cnt; i++)
        {
            nim_reg_write(dev, (i + 0x7E), cmd + i, 1);
        }
        // remove clean interrupt, since reg7d is read only
        break;
    case NIM_DISEQC_MODE_BYTES_EXT_STEP2:
        //TX start : Send byte
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        temp = 0x04;
        data = ((data & 0xF8) | temp);
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);

        //wait for DISEQC_EVENT register cleared before every transmission
        comm_sleep(1);

        //waiting for the send over
        timer = 0;
        timeout = 75 + 13 * cnt;
        while (timer < timeout)
        {
            nim_reg_read(dev, R7C_DISEQC_CTRL + 0x01, &data, 1);
            if ((0 != (data & 0x07))&&(timer>50))
            {
                break;
            }
            comm_sleep(10);
            timer += 10;
        }
        if (1 == (data & 0x07))
        {
            //resume DISEQC_22k origianl value
            nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
            data = ((data & 0xB8) | (va_22k));
            nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);

            return SUCCESS;
        }
        else
        {
            //resume DISEQC_22k origianl value
            nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
            data = ((data & 0xB8) | (va_22k));
            nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);

            return ERR_FAILED;
        }
        break;
    case NIM_DISEQC_MODE_ENVELOP_ON:
    {
        nim_reg_read(dev, R24_MATCH_FILTER, &data, 1);
        data |= 0x01;
        nim_reg_write(dev, R24_MATCH_FILTER, &data, 1);
    }
    break;
    case NIM_DISEQC_MODE_ENVELOP_OFF:
    {
        nim_reg_read(dev, R24_MATCH_FILTER, &data, 1);
        data &= 0xFE;
        nim_reg_write(dev, R24_MATCH_FILTER, &data, 1);
    }
    break;
    default :
        break;
    }
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3501_DiSEqC2X_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt, \
*                               UINT8 *rt_value, UINT8 *rt_cnt)
*
*  defines DiSEqC 2.X operations
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 mode
*  Parameter3: UINT8* cmd
*  Parameter4: UINT8 cnt
*  Parameter5: UINT8 *rt_value
*  Parameter6: UINT8 *rt_cnt
*
* Return Value: Operation result.
*****************************************************************************/
INT32 nim_s3501_di_seq_c2x_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt, 
                         UINT8 *rt_value, UINT8 *rt_cnt)
{
    INT32 result = 0;
    UINT8 data = 0;
    UINT8 temp = 0;
    UINT16 timeout = 0;
    UINT16 timer = 0;
    UINT8 i = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    switch (mode)
    {
    case NIM_DISEQC_MODE_BYTES:
        if((rt_value == NULL) || (rt_cnt == NULL))
        {
		    return ERR_FAILURE;
		}	
        if(cnt && (cmd == 0))
        {
		    return ERR_FAILURE;
		}	
        //write the data to send buffer
        if(cnt > CNT_NUM_8)
        {
            NIM_PRINTF("\t\t NIM_ERROR : Diseqc cnt larger than 8: cnt = %d\n", cnt);
            return ERR_FAILED;
        }
        for (i = 0; i < cnt; i++)
        {
            data = cmd[i];
            nim_reg_write(dev, (i + R7C_DISEQC_CTRL + 0x02), &data, 1);
        }

        //set diseqc data counter
        temp = cnt - 1;
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        va_22k = (data & 0x41);    // get DISEQC_22k origianl value
        data = ((data & 0x47) | (temp << 3));
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);

        //enable diseqc interrupt mask event bit.
        nim_reg_read(dev, R03_IMASK, &data, 1);
        data |= 0x80;
        nim_reg_write(dev, R03_IMASK, &data, 1);

        //clear co-responding diseqc interrupt event bit.
        nim_reg_read(dev, R02_IERR, &data, 1);
        data &= 0x7f;
        nim_reg_write(dev, R02_IERR, &data, 1);

        //write the control bits, need reply
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        temp = 0x84;
        data = ((data & 0x78) | temp);
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);

        //wait for DISEQC_EVENT register cleared before every transmission
        comm_sleep(1);

        //waiting for the send over
        timer = 0;
        timeout = 75 + 13 * cnt + 200;
        data = 0;

        //check diseqc interrupt state.
        while (timer < timeout)
        {
            nim_reg_read(dev, R02_IERR, &data, 1);
            if (0x80 == (data & 0x80))
            {
                break;
            }
            comm_sleep(10);
            timer += 1;
        }

        //init value for error happens.
        result = ERR_FAILUE;
        rt_value[0] = DISEQC2X_ERR_NO_REPLY;
        *rt_cnt = 0;
        if (0x80 == (data & 0x80))
        {
            nim_reg_read(dev, R7C_DISEQC_CTRL + 0x01, &data, 1);

            switch (data & 0x07)
            {
            case 1:
                *rt_cnt = (UINT8) ((data >> 4) & 0x0f);
                if (*rt_cnt > 0)
                {
                    for (i = 0; i < *rt_cnt; i++)
                    {
                        nim_reg_read(dev, (i + R86_DISEQC_RDATA), (rt_value + i), 1);
                    }
                    result = SUCCESS;
                }

                break;

            case 2:
                rt_value[0] = DISEQC2X_ERR_NO_REPLY;
                break;
            case 3:
                rt_value[0] = DISEQC2X_ERR_REPLY_PARITY;
                break;
            case 4:
                rt_value[0] = DISEQC2X_ERR_REPLY_UNKNOWN;
                break;
            case 5:
                rt_value[0] = DISEQC2X_ERR_REPLY_BUF_FUL;
                break;
            default:
                rt_value[0] = DISEQC2X_ERR_NO_REPLY;
                break;
            }
        }

        //set 22k and polarity by origianl value; other-values are not care.
        nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);
        data = (data & 0xb8) | va_22k;
        nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);

        return result;

    default :
        break;
    }

    comm_sleep(1000);

    return SUCCESS;
}

INT32 nim_s3501_interrupt_mask_clean(struct nim_device *dev)
{
    UINT8 data = 0x00;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    nim_reg_write(dev, R02_IERR, &data, 1);
    data = 0xff;
    nim_reg_write(dev, R03_IMASK, &data, 1);
    return SUCCESS;
}


INT32 nim_s3501_adc_setting(struct nim_device *dev)
{
    struct nim_s3501_private *priv;
    UINT8 data = 0;
    UINT8 ver_data = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}  

    priv = (struct nim_s3501_private *)dev->priv;

    // ADC setting
    if (priv->tuner_config_data.qpsk_config & M3501_IQ_AD_SWAP)
    {
        data = 0x4a;
     }
    else
    {
        data = 0x0a;
     }

    if (priv->tuner_config_data.qpsk_config & M3501_EXT_ADC)
    {
        data |= 0x80;
    }
    else
    {
        data &= 0x7f;
     }
    nim_reg_write(dev, R01_ADC, &data, 1);

    nim_reg_read(dev, R01_ADC, &ver_data, 1);
    if (data != ver_data)
    {
        NIM_PRINTF(" wrong 0x8 reg write\n");
        return ERR_FAILED;
    }


    return SUCCESS;
}

INT32 nim_s3501_cr_setting(struct nim_device *dev, UINT8 s_case)
{
    struct nim_s3501_private *priv = NULL;
    UINT8 data = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}
	priv = (struct nim_s3501_private *) dev->priv;
    switch (s_case)
    {
    case NIM_OPTR_SOFT_SEARCH:
        // set CR parameter
        data = 0xaa;
        nim_reg_write(dev, R33_CR_CTRL + 0x03, &data, 1);
        data = 0x45;
        nim_reg_write(dev, R33_CR_CTRL + 0x04, &data, 1);
        data = 0x97;
        nim_reg_write(dev, R33_CR_CTRL + 0x05, &data, 1);

        if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
        {
            data = 0xab;
         }
        else
        {
            data = 0xaa; // S2 CR parameter
        }

        nim_reg_write(dev, RB5_CR_PRS_TRA, &data, 1);
        break;
    case NIM_OPTR_CHL_CHANGE:
        // set CR parameter
        data = 0xaa;
        nim_reg_write(dev, R33_CR_CTRL + 0x03, &data, 1);
        data = 0x45;
        nim_reg_write(dev, R33_CR_CTRL + 0x04, &data, 1);
        data = 0x87;
        nim_reg_write(dev, R33_CR_CTRL + 0x05, &data, 1);

        if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
        {
           data = 0xaa;
        }
        else
        {
           data = 0xaa; // S2 CR parameter
         }

        nim_reg_write(dev, RB5_CR_PRS_TRA, &data, 1);

        break;
     default:
        break;
    }

    return SUCCESS;
}


INT32 nim_s3501_ldpc_setting(struct nim_device *dev, UINT8 s_case, UINT8 c_ldpc, UINT8 c_fec)
{
    struct nim_s3501_private *priv = NULL;
    UINT8 data = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}
	
    priv = (struct nim_s3501_private *) dev->priv; 	  
    // LDPC parameter
    switch (s_case)
    {
    case NIM_OPTR_CHL_CHANGE:
        if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
        {
            UINT8 temp[3] ={0x32, 0xc8, 0x08};

            nim_reg_write(dev, R57_LDPC_CTRL, temp, 3);
        }
        else
        {
            UINT8 temp[3] ={0x32, 0x44, 0x04};

            temp[0] = 0x1e - priv->ul_status.c_rs;
            nim_reg_write(dev, R57_LDPC_CTRL, temp, 3);
        }
        data = 0x01;
        nim_reg_write(dev, RC1_DVBS2_FEC_LDPC, &data, 1);


        break;
    case NIM_OPTR_SOFT_SEARCH:
        if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
        {
            UINT8 temp[3] ={0x32, 0x48, 0x08};

            nim_reg_write(dev, R57_LDPC_CTRL, temp, 3);
        }
        else
        {
            UINT8 temp[3] ={0x1e, 0x46, 0x06};

            nim_reg_write(dev, R57_LDPC_CTRL, temp, 3);
        }
        data = 0x01;
        nim_reg_write(dev, RC1_DVBS2_FEC_LDPC, &data, 1);

        break;
    case NIM_OPTR_DYNAMIC_POW:
        data = c_ldpc - priv->ul_status.c_rs;
        nim_reg_write(dev, R57_LDPC_CTRL, &data, 1);

        break;
    default:
        break;
    }
    data = c_fec;
    nim_reg_write(dev, RC1_DVBS2_FEC_LDPC, &data, 1);

    return SUCCESS;
}


INT32 nim_s3501_hw_init(struct nim_device *dev)
{
    struct nim_s3501_private *priv = NULL;
    UINT8 data = 0xc0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}
	priv = (struct nim_s3501_private *) dev->priv;
	 
    nim_reg_write(dev, RA7_I2C_ENHANCE, &data, 1);

    nim_reg_read(dev, RCC_STRAP_PIN_CLOCK, &data, 1);
    data = data & 0xfa;
    data = data | 0x20;
    nim_reg_write(dev, RCC_STRAP_PIN_CLOCK, &data, 1);

    data = 0x10;
    nim_reg_write(dev, RB3_PIN_SHARE_CTRL, &data, 1);
    data = 0x1f;
    nim_reg_write(dev, R1B_TR_TIMEOUT_BAND, &data, 1);

    data = 0x84;
    nim_reg_write(dev, R28_PL_TIMEOUT_BND + 0x01, &data, 1);

    // Set Hardware time out
    nim_s3501_set_hw_timeout(dev, 0xff);

    //----eq demod setting
    data = 0x04;
    nim_reg_write(dev, R21_BEQ_CTRL, &data, 1);

    data = 0x24;
    nim_reg_write(dev, R25_BEQ_MASK, &data, 1);

    if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_1BIT_MODE)
    {
        data = 0x08;
     }
    else
    {
        data = 0x00;
    }
    nim_reg_write(dev, RAF_TSOUT_PAD, &data, 1);

    data = 0x00;
    nim_reg_write(dev, RB1_TSOUT_SMT, &data, 1);

    data = 0x6c;
    nim_reg_write(dev, R2A_PL_BND_CTRL + 0x02, &data, 1);
    //----eq demod setting end

    nim_s3501_adc_setting(dev);

#ifdef SW_ADPT_CR
    if(priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B && priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C)
    {
        data = 0xc3;
        nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);
        data = 0x9e;
        nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);
        nim_reg_read(dev, RE0_PPLL_CTRL, &data, 1);
        data |= 0x08;
        nim_reg_write(dev, RE0_PPLL_CTRL, &data, 1);
    }
#endif

#ifdef HW_ADPT_CR
    if(priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B && priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C)
    {
        nim_cr_tab_init(dev);
        data = 0x81;
        nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);

#ifdef HW_ADPT_CR_MONITOR
        data = 0xe6;
        nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);
#endif
    }
#endif


    // config EQ
#ifdef DFE_EQ
    data = 0xf0;
    nim_reg_write(dev, RD7_EQ_REG, &data, 1);
    nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
    data = data | 0x81;
    nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
#else
    data = 0x00;
    nim_reg_write(dev, RD7_EQ_REG, &data, 1);
    nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
    data = data | 0x81;
    nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
#endif

    return SUCCESS;
}

INT32 nim_s3501_set_phase_noise(struct nim_device *dev)
{
    UINT32 debug_time = 0;
    UINT32 debug_time_thre = 0;
    UINT32 i = 0;
    UINT8 snr = 0;
    UINT8 data = 0;
    UINT8 verdata = 0;
    UINT8 sdat = 0;
    UINT32 ber = 0;
    UINT32 per = 0;
    UINT32 min_per = 0;
    UINT32 max_per = 0;
    UINT32 per_buf[4] ={0, 0, 0, 0};
    UINT32 buf_index = 0;
    struct nim_s3501_private *priv = NULL;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}  
    priv = (struct nim_s3501_private *) dev->priv;
	
    NIM_PRINTF("            Eenter function set_phase_noise\n");
    sdat = 0xba;
    nim_reg_write(dev, RB5_CR_PRS_TRA, &sdat, 1);
    debug_time = 0;
    debug_time_thre = 4;
    sdat = 0xba;

    data = 0x00;
    nim_reg_write(dev, R74_PKT_STA_NUM, &data, 1);
    data = 0x10;
    nim_reg_write(dev, R74_PKT_STA_NUM + 0x01, &data, 1);
    for (debug_time = 0; debug_time < debug_time_thre; debug_time++)
    {
        nim_reg_read(dev, R04_STATUS, &data, 1);
        if ((data & 0x3f) == 0x3f)
        {
            break;
        }
        data = 0x80;
        nim_reg_write(dev, R76_BIT_ERR + 0x02, &data, 1);
        nim_reg_read(dev, R76_BIT_ERR + 0x02, &verdata, 1);
        if (verdata != data)
        {
            NIM_PRINTF("!!! RESET BER ERROR!\n");
        }
        data = 0x80;
        nim_reg_write(dev, R79_PKT_ERR + 0x01, &data, 1);
        nim_reg_read(dev, R79_PKT_ERR + 0x01, &verdata, 1);
        if (verdata != data)
        {
            NIM_PRINTF("!!! RESET PER ERROR!\n");
        }
        comm_delay(100);

        nim_s3501_get_snr(dev, &snr);
        nim_s3501_get_new_ber(dev, &ber);
        nim_s3501_get_new_per(dev, &per);
        NIM_PRINTF("--- snr/ber/per = %x/%x/%x\n", snr, (unsigned int)ber, (unsigned int)per);

        per_buf[buf_index] = per;
        sdat = sdat - 0x10;
        buf_index ++;
        nim_reg_write(dev, RB5_CR_PRS_TRA, &sdat, 1);
		if(buf_index >= BUF_INDEX_2)
		{
        	if (per_buf[buf_index - 2] == 0) 
            {
                break;
            }
        }
		
    }



    min_per = 0;
    max_per = 0;
    for (i = 0; i < buf_index; i++)
    {
        NIM_PRINTF("per_buf[%d] = 0x%x\n", (int)i, (unsigned int)per_buf[i]);
        per_buf[i] >>= 4;
        if (per_buf[i] < per_buf[min_per])
        {
            min_per = i;
        }
        if (per_buf[i] > per_buf[max_per])
        {
            max_per = i;
            if (i > 1)
            {
                break;
            }
        }
    }

    if (min_per <= max_per)
    {
        priv->t_param.t_phase_noise_detected = 0;
        NIM_PRINTF("No phase noise detected!\n");
    }
    else
    {
        priv->t_param.t_phase_noise_detected = 1;
        NIM_PRINTF("Phase noise detected!\n");
        data = 0x42;
        nim_reg_write(dev, R33_CR_CTRL + 0x04, &data, 1);
    }

    NIM_PRINTF("min_per is %d, max_per is %d\n", (int)min_per, (int)max_per);
    if ((min_per < buf_index - 1) && (per_buf[min_per] == per_buf[min_per + 1]))
    {
        if ((per_buf[min_per] > 0) || (snr < 0x10) || (priv->t_param.t_phase_noise_detected == 0))
        {
            sdat = 0xba - min_per * 0x10;
         }
        else
        {
            sdat = 0xaa - min_per * 0x10;
        }
        nim_reg_write(dev, RB5_CR_PRS_TRA, &sdat, 1);
    }
    else
    {
        sdat = 0xba - min_per * 0x10;
        nim_reg_write(dev, RB5_CR_PRS_TRA, &sdat, 1);
    }

    NIM_PRINTF("--------------------EXIT set_phase_noise, REG_b5 = 0x%x\n", sdat);

    data = 0x10;
    nim_reg_write(dev, R74_PKT_STA_NUM, &data, 1);
    data = 0x27;
    nim_reg_write(dev, R74_PKT_STA_NUM + 0x01, &data, 1);
    return SUCCESS;
}



INT32 nim_m3501c_fec_ts_off (struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    // close fec ts output
    nim_reg_read(dev, RAF_TSOUT_PAD, &data, 1);
    data = data | 0x10;
    nim_reg_write(dev, RAF_TSOUT_PAD, &data, 1);
    NIM_PRINTF("            fec ts off\n");
    return SUCCESS;
}

INT32 nim_m3501c_fec_ts_on (struct nim_device *dev)
{
    UINT8 data = 0;
	
    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   

    // Open fec ts output
    nim_reg_read(dev, RAF_TSOUT_PAD, &data, 1);
    data = data & 0xef;
    nim_reg_write(dev, RAF_TSOUT_PAD, &data, 1);
    NIM_PRINTF("            fec ts on\n");
    return SUCCESS;
}


INT32 nim_m3501c_reset_tso (struct nim_device *dev)
{
    UINT8 data = 0;
	
    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   

    // Enable soft reset
    data = 0xfe;
    nim_reg_write(dev, RFA_RESET_CTRL, &data, 1);

    // Enable tso soft reset
    data = 0xbf;
    nim_reg_write(dev, RFA_RESET_CTRL + 4, &data, 1);
    data = 0xff;
    nim_reg_write(dev, RFA_RESET_CTRL + 4, &data, 1);

    // Disable soft reset
    data = 0xff;
    nim_reg_write(dev, RFA_RESET_CTRL, &data, 1);

    NIM_PRINTF("            Reset TSO\n");
    return SUCCESS;
}

INT32 nim_m3501c_get_int(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT8 rdata = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    // Enable soft reset
    nim_reg_read(dev, R02_IERR, &rdata, 1);
    if((rdata & 0x02) != 0)
    {
        nim_m3501c_fec_ts_off(dev);
        nim_m3501c_reset_tso(dev);
        nim_m3501c_fec_ts_on(dev);
        data = rdata & 0xfd;
        nim_reg_write(dev, R02_IERR, &data, 1);
#ifdef C3501C_ERRJ_LOCK
        nim_m3501c_recover_moerrj(dev);
#endif
        NIM_PRINTF("            Found plug_in_out and reset tso \n");
    }
    return SUCCESS;
}


INT32 nim_set_ts_rs(struct nim_device *dev, UINT32 rs_input)
{
    UINT8 data = 0;
    UINT32 temp = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    rs_input = ((rs_input << 10) + 500) / 1000;
    temp = (rs_input * 204 + 94) / 188;
    temp = temp + 1024;

    if(temp > (45 * 1024))
    {
        temp =  (45 * 1024);
    }
    data = temp & 0xff;
    nim_reg_write(dev, RDD_TS_OUT_DVBS, &data, 1);
    data = (temp >> 8) & 0xff;
    nim_reg_write(dev, RDD_TS_OUT_DVBS + 0x01, &data, 1);

    nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
    data = data | 0x10;
    nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
    NIM_PRINTF("xxx set rs from register file........... \n");
    return SUCCESS;
}

static UINT8 nim_s3501_set_8bit_mode(struct nim_device *dev,UINT8 work_mode,UINT8 rdata,UINT32 bit_rate,
                                     UINT8 channel_change_flag)
{
    UINT8 ci_plus_flag = 0;
    UINT8 data = 0;
	struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    
	priv = (struct nim_s3501_private *) dev->priv;

    NIM_PRINTF("xxx bit rate kkk is %d \n", (int)bit_rate);
    if (work_mode != M3501_DVBS2_MODE)// DVBS mode
    {
        if (((bit_rate >= BIT_RATE_98) || (bit_rate <= ssi_clock_tab[8])) && channel_change_flag)
        {
            // USE SPI dummy, no SSI debug mode.
            if ((priv->tuner_config_data.qpsk_config & M3501_USE_188_MODE) == M3501_USE_188_MODE)
            {
        data = (rdata & 0x08) | 0x10;
            }
            else
            {
        data = (rdata & 0x08) | 0x16;
            }
            nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
            NIM_PRINTF("DVBS USE SPI normal dummy, no SSI debug mode \n");
        }
        else
        {
            // USE SSI debug mode with dummy enable.
            nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
            data = data | 0x20;
            nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
            nim_s3501_set_ssi_clk(dev, (UINT8)bit_rate);
            if ((priv->tuner_config_data.qpsk_config & M3501_USE_188_MODE) == M3501_USE_188_MODE)
            {
               data = (rdata & 0x08) | 0x21;
            }
            else
            {
               data = (rdata & 0x08) | 0x27;
            }
            nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
            NIM_PRINTF("DVBS  USE SSI debug mode \n");
        }
    }
    else    // DVBS2 mode
    {
        if (((bit_rate >= BIT_RATE_98) || (bit_rate <= ssi_clock_tab[8])) && channel_change_flag)
        {
            // USE normal SPI
            if ((priv->tuner_config_data.qpsk_config & M3501_USE_188_MODE) == M3501_USE_188_MODE)
            {
               data = (rdata & 0x08) | 0x10;
            }
            else
            {
               data = (rdata & 0x08) | 0x16;
            }
            nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);

            NIM_PRINTF("DVBS2 Enter normal SPI mode, not use SSI debug..\n");
        }
        else
        {
            //use ssi debug to output 8bit spi mode
            nim_s3501_open_ci_plus(dev, &ci_plus_flag);
            nim_s3501_set_ssi_clk(dev, (UINT8)bit_rate);
            if ((priv->tuner_config_data.qpsk_config & M3501_USE_188_MODE) == M3501_USE_188_MODE)
            {
              data = (rdata & 0x08) | 0x21;
             }
            else
            {
              data = (rdata & 0x08) | 0x27;
            }
            nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
            NIM_PRINTF("DVBS2 Enter SSI debug..\n");
        }
    }
    return ci_plus_flag;

}



static UINT8 nim_s3501_set_4bit_mode(struct nim_device *dev,UINT8 work_mode,UINT8 rdata,UINT32 bit_rate,
                                     UINT8 channel_change_flag)
{
    UINT8 ci_plus_flag = 0;
    UINT8 data = 0;
	struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}
	   
    priv = (struct nim_s3501_private *) dev->priv;

    //TS 4bit mode
    if (priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_S3501D)
    {
        if ((priv->tuner_config_data.qpsk_config & M3501_USE_188_MODE) == M3501_USE_188_MODE)
        {
    data = (rdata & 0x08) | 0x10;
        }
        else
        {
    data = (rdata & 0x08) | 0x16;    //moclk interv for 4bit.
         }
        nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
        data = 0x20;
        nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
        NIM_PRINTF("S3501D Enter 4bit Mode============= \n");
    }
    else
    {
        //TS 4bit mode
        //ECO_SSI_2B_EN = cr9f[3]
        nim_reg_read(dev, R9C_DEMAP_BETA + 0x03, &data, 1);
        data = data | 0x08;
        nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);

        //ECO_SSI_SEL_2B = crc0[3]
        nim_reg_read(dev, RC0_BIST_LDPC_REG, &data, 1);
        data = data & 0xf7; //for 4bit mode
        nim_reg_write(dev, RC0_BIST_LDPC_REG, &data, 1);

        nim_s3501_set_ssi_clk(dev, (UINT8)bit_rate);

        if (work_mode != M3501_DVBS2_MODE)// DVBS mode
        {
            nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
            data = data | 0x20;                                                     //add for ssi_clk change point
            nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
            NIM_PRINTF("M3501B Enter DVBS 4bit mode \n");
        }
        else    // DVBS2 mode
        {
            // For CI plus test.
            if (bit_rate < BIT_RATE_50)
            {
                NIM_PRINTF("Low bit rate, Close CI plus......................... \n");
            }
            else
            {
                nim_s3501_open_ci_plus(dev, &ci_plus_flag);
            }
            NIM_PRINTF("M3501B Enter DVBS2 4bit mode..\n");
        }

        //no matter bit_rate all use ssi_debug mode
        if ((priv->tuner_config_data.qpsk_config & M3501_USE_188_MODE) == M3501_USE_188_MODE)
        {
              data = (rdata & 0x08) | 0x21;
        }
        else
        {
             data = (rdata & 0x08) | 0x27;    // enable SSI debug
        }
        nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
    }    //end if M3501B  4bit

    return ci_plus_flag;
}


static UINT8 nim_s3501_set_2bit_mode(struct nim_device *dev,UINT8 work_mode,UINT8 rdata,UINT32 bit_rate,
                                     UINT8 channel_change_flag)
{
    UINT8 ci_plus_flag = 0;
    UINT8 data = 0;
	struct nim_s3501_private *priv =NULL;
	
    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    priv = (struct nim_s3501_private *) dev->priv;

    //TS 2bit mode
    if (priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_S3501D)
    {
        if ((priv->tuner_config_data.qpsk_config & M3501_USE_188_MODE) == M3501_USE_188_MODE)
        {
    data = (rdata & 0x08) | 0x10;
        }
        else
        {
    data = (rdata & 0x08) | 0x16;
        }
        nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
        data = 0x00;
        nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
        NIM_PRINTF("S3501D Enter 2bit Mode============= \n");
    }
    else
    {
        //TS 2bit mode
        //ECO_SSI_2B_EN = cr9f[3]
        nim_reg_read(dev, R9C_DEMAP_BETA + 0x03, &data, 1);
        data = data | 0x08;
        nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);

        //ECO_SSI_SEL_2B = crc0[3]
        nim_reg_read(dev, RC0_BIST_LDPC_REG, &data, 1);
        data = data | 0x08; //for 2bit mode
        nim_reg_write(dev, RC0_BIST_LDPC_REG, &data, 1);

        nim_s3501_set_ssi_clk(dev, (UINT8)bit_rate);

        if (work_mode != M3501_DVBS2_MODE)// DVBS mode
        {
            nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
            data = data | 0x20;                              //add for ssi_clk change point
            nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
            NIM_PRINTF("Enter DVBS 2bit mode \n");
        }
        else    // DVBS2 mode
        {
            // For CI plus test.
            if (bit_rate < BIT_RATE_50)
            {
                NIM_PRINTF("Low bit rate, Close CI plus......................... \n");
            }
            else
            {
                nim_s3501_open_ci_plus(dev, &ci_plus_flag);
            }
            NIM_PRINTF("Enter DVBS2 2bit mode..\n");
        }

        //no matter bit_rate all use ssi_debug mode
        if ((priv->tuner_config_data.qpsk_config & M3501_USE_188_MODE) == M3501_USE_188_MODE)
        {
             data = (rdata & 0x08) | 0x21;
        }
        else
        {
             data = (rdata & 0x08) | 0x27;    // enable SSI debug
        }
        nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
    }//end if M3501B 2bit

	return ci_plus_flag;	

}

static UINT8 nim_s3501_set_1bit_mode(struct nim_device *dev,UINT8 work_mode,UINT8 rdata,UINT32 bit_rate,
                                     UINT8 channel_change_flag)
{
    UINT8 ci_plus_flag = 0;
    UINT8 data = 0;
	struct nim_s3501_private *priv =NULL;
	
    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}  
    priv = (struct nim_s3501_private *) dev->priv;

    //SSI mode
    if (priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_S3501D)
    {
        data = 0x60;
        nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
    }
    nim_s3501_set_ssi_clk(dev, (UINT8)bit_rate);
    if (work_mode != M3501_DVBS2_MODE)
    {
        //DVBS mode
        nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
        data = data | 0x20;                             //add for ssi_clk change point
        nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
        NIM_PRINTF("DVBS USE SSI mode \n");
    }
    else
    {
        //  DVBS2 mode
        if (bit_rate < BIT_RATE_50)
        {
            NIM_PRINTF("Low bit rate, Close CI plus......................... \n");
        }
        else
        {
            nim_s3501_open_ci_plus(dev, &ci_plus_flag);
        }
        NIM_PRINTF("DVBS2 USE SSI mode \n");
    }

    if ((priv->tuner_config_data.qpsk_config & M3501_USE_188_MODE) == M3501_USE_188_MODE)
    {
         data = (rdata & 0x08) | 0x01;
     }
    else
    {
         data = (rdata & 0x08) | 0x07;    // enable SSI debug
    }
    nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);

    return ci_plus_flag;
}


INT32 nim_s3501_set_ts_mode(struct nim_device *dev, UINT8 work_mode, UINT8 map_type, UINT8 code_rate,
                            UINT32 rs_input, UINT8 channel_change_flag)
{
    UINT8 data = 0;
    UINT8 rdata = 0;
    UINT32 bit_rate = 0;
	struct nim_s3501_private *priv =NULL;
	UINT8 ci_plus_flag = 0;
	
    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    priv = (struct nim_s3501_private *) dev->priv;
    

    NIM_PRINTF("Enter nim_s3501_set_ts_mode:\n");

    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if((priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C))
        {
            if(priv->ul_status.m_tso_mode == 1)
            {
                NIM_PRINTF("C3501C use NEW TSO, but enter wrong function, nim_s3501_set_ts_mode \n");
                return SUCCESS;
            }
            else
            {
                NIM_PRINTF("C3501C use OLD TSO, nim_s3501_set_ts_mode \n");
            }
        }
        nim_reg_read(dev, RC0_BIST_LDPC_REG, &data, 1);
        data = data | 0xc0;
        nim_reg_write(dev, RC0_BIST_LDPC_REG, &data, 1);

        bit_rate = 0x000000ff;
        nim_s3501_get_bit_rate(dev, work_mode, map_type, code_rate, rs_input, &bit_rate);

        // bit rate re-caculate since all clock is over-frequency.
        bit_rate = (bit_rate * 13500 + CRYSTAL_FREQ / 2) / CRYSTAL_FREQ ;

        nim_reg_read(dev, RD8_TS_OUT_SETTING, &rdata, 1);
        /****************TS output config 8bit mode begin*****************************************/
        if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_8BIT_MODE) //8bit mode
        {
             ci_plus_flag=nim_s3501_set_8bit_mode(dev,work_mode,rdata,bit_rate,channel_change_flag);
        }
        /****************TS output config 8bit mode end*****************************************/
        /****************TS output config 1bit mode begin*****************************************/
        else if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_1BIT_MODE)    //SSI mode
        {
             ci_plus_flag=nim_s3501_set_1bit_mode(dev,work_mode,rdata,bit_rate,channel_change_flag);
        }
        /****************TS output config 1bit mode end*****************************************/
        /****************TS output config 2bit mode begin*****************************************/
        else if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_2BIT_MODE)    //TS 2bit mode
        {
            ci_plus_flag=nim_s3501_set_2bit_mode(dev,work_mode,rdata,bit_rate,channel_change_flag);
        }
        /****************TS output config 2bit mode end*****************************************/
        /****************TS output config 4bit mode begin*****************************************/
        else if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_4BIT_MODE)    //4//4bit mode
        {
            ci_plus_flag=nim_s3501_set_4bit_mode(dev,work_mode,rdata,bit_rate,channel_change_flag);
        }
        if (ci_plus_flag)
        {
            //RST fsm
            data = 0x91;
            nim_reg_write(dev, R00_CTRL, &data, 1);
            data = 0x51;
            nim_reg_write(dev, R00_CTRL, &data, 1);
        }
        /****************TS output config 4bit mode end*****************************************/
        //end M3501B config.
    }
    else
    {
        //M3501A
        if ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_1BIT_MODE)
        {
           data = 0x60;
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

        nim_reg_read(dev, RAD_TSOUT_SYMB + 0x01, &rdata, 1);
        if(rdata != data)
        {
            NIM_PRINTF("M3501A set TS mode faily !(%0x , %0x )\n", data, rdata);
        }
        else
        {
            NIM_PRINTF("M3501A set TS mode successfully !( %0x )\n", data);
        }

    }

    NIM_PRINTF("Leave nim_s3501_set_ts_mode\n");

    return SUCCESS;
}


/*****************************************************************************
* INT32 nim_s3501_set_polar(struct nim_device *dev, UINT8 polar)
* Description: S3501 set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
//#define REVERT_POLAR
INT32 nim_s3501_set_polar(struct nim_device *dev, UINT8 polar)
{
    UINT8 data = 0;
	struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    priv = (struct nim_s3501_private *) dev->priv;

	NIM_PRINTF("[%s]line=%d,enter,polar=%d!\n",__FUNCTION__,__LINE__,polar);
	
    if (priv->ext_lnb_control)
    {
        return priv->ext_lnb_control(priv->ext_lnb_id, LNB_CMD_SET_POLAR, (UINT32) polar);
    }
    nim_reg_read(dev, R7C_DISEQC_CTRL, &data, 1);

    if ((priv->tuner_config_data.qpsk_config & M3501_POLAR_REVERT) == 0x00) //not exist H/V polarity revert.
    {
        if (NIM_PORLAR_HORIZONTAL == polar)
        {
            data &= 0xBF;
			NIM_PRINTF("[%s]line=%d,CR5C is 00!\n",__FUNCTION__,__LINE__);
        }
        else if (NIM_PORLAR_VERTICAL == polar)
        {
            data |= 0x40;
			NIM_PRINTF("[%s]line=%d,CR5C is 40!\n",__FUNCTION__,__LINE__);
        }
        else
        {
            NIM_PRINTF("nim_s3501_set_polar error\n");
            return ERR_FAILUE;
        }
    }
    else//exist H/V polarity revert.
    {
        if (NIM_PORLAR_HORIZONTAL == polar)
        {
            data |= 0x40;
            NIM_PRINTF("[%s]line=%d,CR5C is 40!\n",__FUNCTION__,__LINE__);
        }
        else if (NIM_PORLAR_VERTICAL == polar)
        {
            data &= 0xBF;
            NIM_PRINTF("[%s]line=%d,CR5C is 00!\n",__FUNCTION__,__LINE__);
        }
        else
        {
            NIM_PRINTF("nim_s3501_set_polar error\n");
            return ERR_FAILUE;
        }
    }

    nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
    return SUCCESS;
}



INT32 nim_close_ts_dummy(struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    nim_reg_read(dev, RD8_TS_OUT_SETTING, &data, 1);
    data = data & 0xee;
    nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
    data = 0x40;
    nim_reg_write(dev, RAD_TSOUT_SYMB + 0x01, &data, 1);
    nim_reg_read(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
    data = data & 0x9f;
    nim_reg_write(dev, RDC_EQ_DBG_TS_CFG, &data, 1);
    data = 0xff;    // ssi tx debug close
    nim_reg_write(dev, RDF_TS_OUT_DVBS2, &data, 1);

    return SUCCESS;
}

INT32 nim_m3501c_close_dummy (struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    nim_m3501c_fec_ts_off(dev);

    nim_reg_read(dev, RF0_HW_TSO_CTRL, &data, 1);
    data = data & 0xfd;
    nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
    NIM_PRINTF("            nim m3501c close dummy for dvbs \n");

    nim_m3501c_reset_tso(dev);
    nim_m3501c_fec_ts_on(dev);
    return SUCCESS;
}

INT32 nim_m3501c_open_dummy (struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    nim_m3501c_fec_ts_off(dev);

    nim_reg_read(dev, RF0_HW_TSO_CTRL, &data, 1);
    data = data | 0x02;
    nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
    NIM_PRINTF("            nim m3501c insert dummy\n");

    nim_m3501c_reset_tso(dev);
    nim_m3501c_fec_ts_on(dev);
    return SUCCESS;
}


INT32 nim_m3501_ts_off (struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    // Open ts output
    nim_reg_read(dev, R9D_RPT_DEMAP_BETA + 1, &data, 1);
    data = data & 0x7f;
    nim_reg_write(dev, R9D_RPT_DEMAP_BETA + 1, &data, 1);
    NIM_PRINTF("            Final ts on\n");
    return SUCCESS;
}


INT32 nim_m3501_ts_on (struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    // close ts output
    nim_reg_read(dev, R9D_RPT_DEMAP_BETA + 1, &data, 1);
    data = data | 0x80;
    nim_reg_write(dev, R9D_RPT_DEMAP_BETA + 1, &data, 1);
    NIM_PRINTF("            Final ts off\n");
    return SUCCESS;
}

INT32 nim_m3501c_invert_moerrj (struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
    nim_reg_read(dev, RF0_HW_TSO_CTRL + 6, &data, 1);
    data = data | 0x08;
    nim_reg_write(dev, RF0_HW_TSO_CTRL + 6, &data, 1);
    NIM_PRINTF("            nim m3501c invert_moerrj \n");
    return SUCCESS;
}

INT32 nim_m3501c_recover_moerrj (struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	 }  
    nim_reg_read(dev, RF0_HW_TSO_CTRL + 6, &data, 1);
    data = data & 0xf7;
    nim_reg_write(dev, RF0_HW_TSO_CTRL + 6, &data, 1);
    NIM_PRINTF("            nim m3501c recover_moerrj\n");
    return SUCCESS;
}


void nim_s3501_clear_int(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT8 rdata = 0;

    if(dev == NULL)
    {
	   return;
	}   
    data = 0x00;
    nim_reg_write(dev, R02_IERR, &data, 1);

    nim_reg_read(dev, R00_CTRL, &rdata, 1);
    data = (rdata | 0x10);
    nim_s3501_demod_ctrl(dev, data);
}

void nim_s3501_set_freq_offset(struct nim_device *dev, INT32 delfreq)
{
    UINT8 data[3]={0};
    UINT32 temp = 0;
    UINT32 delfreq_abs = 0;
    UINT32 sample_rate = 0;

    if(dev == NULL)
    {
	   return;
	}   
     //delfreq unit is KHz, sample_rate unit is KHz
    if(delfreq < 0)
    {
        delfreq_abs = 0 - delfreq;
    }
    else
    {
        delfreq_abs = delfreq;
     }
    nim_s3501_get_dsp_clk(dev, &sample_rate);
    temp = nim_s3501_multu64div(delfreq_abs, 92160, sample_rate); // 92160 == 90000 * 1.024
    if(temp > 0xffff)
    {
        NIM_PRINTF("\t\t NIM_Error: Symbol rate set error %d\n", (int)temp);
        temp = 0xffff;
    }

    if(delfreq < 0)
    {
        temp = (temp ^ 0xffff) | 0x10000;
    }
    else
    {
        temp = temp & 0xffff ;
    }
    //CR5C
    data[0] = (UINT8) (temp & 0xFF);
    data[1] = (UINT8) ((temp & 0xFF00) >> 8);
    data[2] = (UINT8) ((temp & 0x10000) >> 16);
    nim_reg_write(dev, R5C_ACQ_CARRIER, data, 3);
}


INT32 nim_s3501_tr_cr_setting(struct nim_device *dev, UINT8 s_case)
{
    UINT8 data = 0;
    struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
	priv = (struct nim_s3501_private *) dev->priv;  
    switch (s_case)
    {
    case NIM_OPTR_SOFT_SEARCH:
    {
        data = 0x49;
        nim_reg_write(dev, R66_TR_SEARCH, &data, 1);
        data = 0x31;
        nim_reg_write(dev, R67_VB_CR_RETRY, &data, 1);
    }
    break;
    case NIM_OPTR_CHL_CHANGE:
        if (priv->t_param.t_reg_setting_switch & NIM_SWITCH_TR_CR)
        {
            data = 0x59;
            nim_reg_write(dev, R66_TR_SEARCH, &data, 1);
            data = 0x33;
            nim_reg_write(dev, R67_VB_CR_RETRY, &data, 1);
            priv->t_param.t_reg_setting_switch &= ~NIM_SWITCH_TR_CR;
        }
        break;
    default:
        break;
    }
    return SUCCESS;
}


INT32 nim_s3501_agc1_ctrl(struct nim_device *dev, UINT8 low_sym, UINT8 s_case)
{
    struct nim_s3501_private *priv = NULL;
	UINT8 data = 0;
	
    if(dev == NULL)
    {
	   return ERR_FAILURE;
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
            data = 0xb3;
            }
            else
            {
            data = 0xba;
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
    if ((priv->tuner_config_data.qpsk_config & M3501_AGC_INVERT) == 0x0)
    {
        data = data ^ 0x80;
     }

    nim_reg_write(dev, R07_AGC1_CTRL, &data, 1);

    data = 0x58;
    nim_reg_write(dev, R07_AGC1_CTRL + 0x01, &data, 1);

    return SUCCESS;
}


INT32 nim_s3501_hbcd_timeout(struct nim_device *dev, UINT8 s_case)
{
    struct nim_s3501_private *priv = NULL;
    UINT8 data = 0;

    if(dev == NULL)
	{
       return ERR_FAILURE;
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
        break;
    }
    return SUCCESS;
}

INT32 nim_s3501_set_acq_workmode(struct nim_device *dev, UINT8 s_case)
{
    struct nim_s3501_private *priv = NULL;
    UINT8 data = 0;
    UINT8 work_mode = 0x00;

    if(dev == NULL)
    {
	   return ERR_FAILURE;
	}   
	priv = (struct nim_s3501_private *) dev->priv;
    switch (s_case)
    {
    case NIM_OPTR_CHL_CHANGE0:
    case NIM_OPTR_DYNAMIC_POW0:
        if(CRYSTAL_FREQ > CRYSTAL_FREQ_15000)
        {
            data = 0xf3;
         }
        else
        {
            data = 0x73;
         }
        nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);
        break;
    case NIM_OPTR_CHL_CHANGE:
        nim_s3501_reg_get_work_mode(dev, &work_mode);
        if (work_mode != M3501_DVBS2_MODE)
        {
            if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
            {
                priv->ul_status.phase_err_check_status = 1000;
            }
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
        break;
    case NIM_OPTR_SOFT_SEARCH:
    case NIM_OPTR_DYNAMIC_POW:
        if(CRYSTAL_FREQ > CRYSTAL_FREQ_15000)
        {
            data = 0xf7;
        }
        else
        {
            data = 0x77;
         }
        nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);
        break;
    case NIM_OPTR_HW_OPEN:
        if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
        {
            // enable ADC
            data = 0x00;
            nim_reg_write(dev, RA0_RXADC_REG + 0x02, &data, 1);
            // enable S2 clock
            if(CRYSTAL_FREQ > CRYSTAL_FREQ_15000)
            {
                data = 0xf3;
             }
            else
            {
                data = 0x73;
             }
            nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);
        }
        break;
    case NIM_OPTR_HW_CLOSE:
        if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
        {
            // close ADC
            data = 0x07;
            nim_reg_write(dev, RA0_RXADC_REG + 0x02, &data, 1);
            // close S2 clock
            data = 0x3f;
        }
        else
        {
            data = 0x7f;
        }
        nim_reg_write(dev, R5B_ACQ_WORK_MODE, &data, 1);
        break;
    default:
        break;
    }
    return SUCCESS;
}

void nim_s3501_set_rs(struct nim_device *dev, UINT32 rs)
{
    UINT8 data = 0;
    UINT8 ver_data[3]={0};
    UINT32 sample_rate = 0;
    UINT32 temp = 0;
	struct nim_s3501_private *priv =NULL;
	
    if(dev == NULL)
    {
	   return;
	}   
    
	priv = (struct nim_s3501_private *) dev->priv;

    if (priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3501B)
    {
        if((priv->ul_status.m_s3501_sub_type == NIM_CHIP_SUB_ID_M3501C) && (priv->ul_status.m_tso_mode == 1))
        {
            if(rs < SYM_7000)
            {
            data = 0x10;
            }
            else if(rs < 15000)
            {
            data = 0x20;
            }
            else if(rs < 23000)
            {
            data = 0x30;
            }
            else if(rs < 31000)
            {
            data = 0x40;
            }
            else if(rs < 39000)
            {
            data = 0x50;
            }
            else if(rs < 47000)
            {
            data = 0x60;
            }
            else if(rs < 54000)
            {
            data = 0x70;
            }
            else if(rs < 59000)
            {
            data = 0x90;
            }
            else if(rs < 62000)
            {
            data = 0xa0;
            }
            else if(rs < 65000)
            {
            data = 0xb0;
            }
            else
            {
            data = 0xc0;
             }
            nim_reg_write(dev, RF1_DSP_CLK_CTRL, &data, 1);
        }
    }

    temp = 0;
    nim_s3501_get_dsp_clk(dev, &sample_rate);

    NIM_PRINTF("\t\t set ts %d with dsp clock %d\n", (int)rs, (int)(sample_rate / 1000));
    temp = nim_s3501_multu64div(rs, 184320, sample_rate); // 184320 == 90000 * 2.048

    if(temp > 0x16800)
    {
        NIM_PRINTF("\t\t NIM_Error: Symbol rate set error %d\n", (int)temp);
        temp = 0x16800;
    }
    //CR3F
    ver_data[0] = (UINT8) (temp & 0xFF);
    ver_data[1] = (UINT8) ((temp & 0xFF00) >> 8);
    ver_data[2] = (UINT8) ((temp & 0x10000) >> 16);
    nim_reg_write(dev, R5F_ACQ_SYM_RATE, ver_data, 3);
}



