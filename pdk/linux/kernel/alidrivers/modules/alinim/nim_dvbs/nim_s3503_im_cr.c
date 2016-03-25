/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_s3503_im_01.c
*
*    Description:  s3503 im layer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#include "nim_s3503.h"

#define BER_DVBS_BETA    100
#define BER_DVBS2_BETA    100
#define TDATA_SUM         4
#define TDATA_START       7
#define MAP_TYPE_2  2
#define MAP_TYPE_3  3
#define MAP_TYPE_4  4
#define MAP_TYPE_5  5
#define MER_TEMP_1681  1681
#define MER_TEMP_255  255
#define SUM_VAL_255 255
#define COMPARE_VAL_5  5
#define SNR_CNT_3 3
#define SYM_40000 40000
#define CODE_RATE_4 4
#define EST_NOISE_221 221
#define EST_NOISE_96 96

static UINT32      va_ber = 0;
static UINT32      va_ber_window = 0;
static UINT32      va_mer = 0;
static UINT32      va_mer_window = 0;
static const UINT8 index_width = 5;

static const UINT8 scale = 15;

#ifdef AGC_FILTER
static INT32       agc_j = 0;
static UINT8       s_data[15]={0};
static UINT8       s_agc = 0x00;

static UINT8       flag0 = 1;
static UINT8       flag1 = 0;

static INT32       snr_j = 0;
static UINT8       average = 0x00;
static UINT8       s_snr = 0x00;
#endif


static const UINT8 snr_tab[177] =
{
    0, 1, 3, 4, 5, 7, 8, \
    9, 10, 11, 13, 14, 15, 16, 17, \
    18, 19, 20, 21, 23, 24, 25, 26, \
    26, 27, 28, 29, 30, 31, 32, 33, \
    34, 35, 35, 36, 37, 38, 39, 39, \
    40, 41, 42, 42, 43, 44, 45, 45, \
    46, 47, 47, 48, 49, 49, 50, 51, \
    51, 52, 52, 53, 54, 54, 55, 55, \
    56, 57, 57, 58, 58, 59, 59, 60, \
    61, 61, 62, 62, 63, 63, 64, 64, \
    65, 66, 66, 67, 67, 68, 68, 69, \
    69, 70, 70, 71, 72, 72, 73, 73, \
    74, 74, 75, 76, 76, 77, 77, 78, \
    79, 79, 80, 80, 81, 82, 82, 83, \
    84, 84, 85, 86, 86, 87, 88, 89, \
    89, 90, 91, 92, 92, 93, 94, 95, \
    96, 96, 97, 98, 99, 100, 101, 102, \
    102, 103, 104, 105, 106, 107, 108, 109, \
    110, 111, 113, 114, 115, 116, 117, 118, \
    119, 120, 122, 123, 124, 125, 127, 128, \
    129, 131, 132, 133, 135, 136, 138, 139, \
    141, 142, 144, 145, 147, 148, 150, 152, \
    153, 155
};

static const UINT32 log2lut[] =
    {
        0, 290941,  573196,  847269, 1113620, 1372674, 1624818,
        1870412, 2109788, 2343253, 2571091, 2793569, 3010931,
        3223408, 3431216, 3634553, 3833610, 4028562, 4219576,
        4406807, 4590402, 4770499, 4947231, 5120719, 5291081,
        5458428, 5622864, 5784489, 5943398,  6099680, 6253421,
        6404702,  6553600
    };

static UINT32 log10times100_l( UINT32 x)
{
    UINT8  i = 0;
    UINT32 y = 0;
    UINT32 d = 0;
    UINT32 k = 0;
    UINT32 r = 0;

    if (x == 0)
    {
       return (0);
    }

    /* Scale x (normalize) */
    /* computing y in log(x/y) = log(x) - log(y) */
    if ( (x & (((UINT32)(-1)) << (scale + 1)) ) == 0 )
    {
        for (k = scale; k > 0 ; k--)
        {
            if (x & (((UINT32)1) << scale))
        {
             break;
        }
            x <<= 1;
        }
    }
    else
    {
        for (k = scale; k < 31 ; k++)
        {
            if ((x & (((UINT32)(-1)) << (scale + 1))) == 0)
        {
            break;
            }
            x >>= 1;
        }
    }
    /*
      Now x has binary point between bit[scale] and bit[scale-1]
      and 1.0 <= x < 2.0 */

    /* correction for divison: log(x) = log(x/y)+log(y) */
    y = k * ( ( ((UINT32)1) << scale ) * 200 );

    /* remove integer part */
    x &= ((((UINT32)1) << scale) - 1);
    /* get index */
    i = (UINT8) (x >> (scale - index_width));
    /* compute delta (x-a) */
    d = x & ((((UINT32)1) << (scale - index_width)) - 1);
    /* compute log, multiplication ( d* (.. )) must be within range ! */
    y += log2lut[i] + (( d * ( log2lut[i + 1] - log2lut[i] )) >> (scale - index_width));
    /* Conver to log10() */
    y /= 108853; /* (log2(10) << scale) */
    r = (y >> 1);
    /* rounding */
    if (y & ((UINT32)1))
    {
        r++;
    }

    return (r);

}
/*****************************************************************************
* INT32 nim_s3503_get_mer(struct nim_device *dev, UINT32 *mer)
* Description: Ger MER
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *mer
*
* Return Value: UINT32
*****************************************************************************/

INT32 nim_s3503_get_mer(struct nim_device *dev, UINT32 *mer)
{
    UINT8 data = 0;
    UINT8 data_in_i = 0;
    UINT8 data_in_q = 0;
    UINT8 data_out_i = 0;
    UINT8 data_out_q = 0;
    UINT8 map_type = 0;
    UINT8 rdata[4]={0};
    UINT32 i = 0;
    UINT32 err_sum = 0;
    UINT32 mer_tmp = 0;

#define NIM_MER_ACC_LEN 512
#define NIM_MER_AVG_LEN 10
#define NIM_MER_STA_LEN 4

    if((dev == NULL) || (mer == NULL))
    {
	    return RET_FAILURE;
	}	
    nim_reg_read(dev, R04_STATUS, &data, 1);
    if (data & 0x08)
    {
        // CR lock
        nim_reg_read(dev, R69_RPT_CARRIER + 2, &data, 1);
        map_type = data >> 5;
        NIM_PRINTF("map_type = %d\n", map_type);
        err_sum = 0;
        if ((map_type == MAP_TYPE_2) || (map_type == MAP_TYPE_3) || (map_type == MAP_TYPE_4) || (map_type == MAP_TYPE_5))
        {
            if(va_mer_window < NIM_MER_STA_LEN)
            {
                for (i = 0; i < NIM_MER_ACC_LEN; i++)
                {
                    data = 0x01;
                    nim_reg_write(dev, R11C_MAP_IN_I, &data, 1);
                    comm_sleep(10);
                    nim_reg_read(dev, R11C_MAP_IN_I, rdata, 4);
                    if (rdata[0] & 0x80)
                    {
                data_in_i = 256 - rdata[0];
                    }
                    else
                    {
                data_in_i = rdata[0];
                    }
                    if (rdata[1] & 0x80)
                    {
                data_in_q = 256 - rdata[1];
                    }
                    else
                    {
                data_in_q = rdata[1];
                    }
                    if (rdata[2] & 0x80)
                    {
                data_out_i = 256 - rdata[2];
                    }
                    else
                    {
                data_out_i = rdata[2];
                    }
                    if (rdata[3] & 0x80)
                    {
                data_out_q = 256 - rdata[3];
                    }
                    else
                    {
                data_out_q = rdata[3];
                     }
                    err_sum = err_sum + (data_out_i - data_in_i) * (data_out_i - data_in_i) + \
                      (data_out_q - data_in_q) * (data_out_q - data_in_q);
                }
                va_mer = va_mer + err_sum;
                va_mer_window++;
                mer_tmp = err_sum / NIM_MER_ACC_LEN;
            }
            else
            {
                for (i = 0; i < NIM_MER_AVG_LEN; i++)
                {
                    data = 0x01;
                    nim_reg_write(dev, R11C_MAP_IN_I, &data, 1);
                    comm_sleep(10);
                    nim_reg_read(dev, R11C_MAP_IN_I, rdata, 4);
                    if (rdata[0] & 0x80)
                    {
                data_in_i = 256 - rdata[0];
                     }
                    else
                    {
                data_in_i = rdata[0];
                    }
                    if (rdata[1] & 0x80)
                    {
                data_in_q = 256 - rdata[1];
                    }
                    else
                    {
                data_in_q = rdata[1];
                    }
                    if (rdata[2] & 0x80)
                    {
                data_out_i = 256 - rdata[2];
                    }
                    else
                    {
                data_out_i = rdata[2];
                     }
                    if (rdata[3] & 0x80)
                    {
                data_out_q = 256 - rdata[3];
                    }
                    else
                    {
                data_out_q = rdata[3];
                    }
                    err_sum = err_sum + (data_out_i - data_in_i) * (data_out_i - data_in_i) + \
                          (data_out_q - data_in_q) * (data_out_q - data_in_q);
                }
                va_mer = va_mer + err_sum / NIM_MER_AVG_LEN - va_mer / (NIM_MER_ACC_LEN * NIM_MER_STA_LEN);
                mer_tmp = va_mer / (NIM_MER_ACC_LEN * NIM_MER_STA_LEN);
            }
        }
        else // other modu type do not support
        {
            *mer = 0;
            NIM_PRINTF("map_type = %d, not supported\n", map_type);
            return ERR_FAILUE;
        }
    }
    else
    {
        NIM_PRINTF("cr unlock, data=%02x\n", data);
        *mer = 0;
        va_mer = 0;
        va_mer_window = 0;
        return ERR_FAILUE;
    }
    if (mer_tmp > MER_TEMP_1681)
    {
        mer_tmp = 1681;
    }
    mer_tmp = 323 - log10times100_l(mer_tmp);
    if (mer_tmp > MER_TEMP_255)
    {
        *mer = 255;
    }
    else
    {
        *mer = mer_tmp;
    }
    return SUCCESS;
}


/*****************************************************************************
* INT32 nim_s3503_get_snr(struct nim_device *dev, UINT8 *snr)
*
* This function returns an approximate estimation of the SNR from the NIM
*  The Eb No is calculated using the SNR from the NIM, using the formula:
*     Eb ~     13312- M_SNR_H
*     -- =    ----------------  dB.
*     NO           683
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/


INT32 nim_s3503_get_snr(struct nim_device *dev, UINT8 *snr)
{
    UINT8  data = 0;
    UINT16 snr_db = 0;
	
    if(dev == NULL || snr == NULL)
    {
	    return RET_FAILURE;
	}	
	

    nim_s3503_get_snr_db(dev,&snr_db);
    snr_db=snr_db/10;

    nim_reg_read(dev, R04_STATUS, &data, 1);
    if (0x3F == (data&0x3F))        // demod lock
    {
        if(snr_db>=150)
        {
            *snr=90;
        }
        else if(snr_db<120)
        {
            *snr=snr_db/4;
        }
        else
        {
            *snr=(snr_db-120)*2+30;
        }
     }
     else    // demod unlock
     {
        *snr=10;
     }
    


    return SUCCESS;

}


/*****************************************************************************
* INT32 nim_s3503_get_ber(struct nim_device *dev, UINT32 *rsubc)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_get_ber(struct nim_device *dev, UINT32 *rsubc)
{
    UINT8 data = 0;
    UINT8 ber_data[3] ={0};
    UINT32 u32ber_data[3] ={0};
    UINT32 uber_data = 0;
    struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL || rsubc == NULL)
    {
	    return RET_FAILURE;
	}	
	
    priv = (struct nim_s3501_private *) dev->priv;

    //CR78
    nim_reg_read(dev, R76_BIT_ERR + 0x02, &data, 1);
    if (0x00 == (0x80 & data))
    {
        //      NIM_PRINTF( "CR78= %x\n", data);
        //CR76
        nim_reg_read(dev, R76_BIT_ERR, &ber_data[0], 1);
        u32ber_data[0] = (UINT32) ber_data[0];
        //CR77
        nim_reg_read(dev, R76_BIT_ERR + 0x01, &ber_data[1], 1);
        u32ber_data[1] = (UINT32) ber_data[1];
        u32ber_data[1] <<= 8;
        //CR78
        nim_reg_read(dev, R76_BIT_ERR + 0x02, &ber_data[2], 1);
        u32ber_data[2] = (UINT32) ber_data[2];
        u32ber_data[2] <<= 16;

        uber_data = u32ber_data[2] + u32ber_data[1] + u32ber_data[0];

        uber_data *= 100;
        uber_data /= 1632;
        *rsubc = uber_data;
        priv->ul_status.old_ber = uber_data;
        //CR78
        data = 0x80;
        nim_reg_write(dev, R76_BIT_ERR + 0x02, &data, 1);
    }
    else
    {
        *rsubc = priv->ul_status.old_ber;
    }

    return SUCCESS;
}



/*****************************************************************************
* INT32 nim_s3503_get_per(struct nim_device *dev, UINT32 *rsubc)
* Get packet error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_get_per(struct nim_device *dev, UINT32 *rsubc)
{
    UINT8 per[2]={0};
    UINT16 percount = 0;
    UINT8 data = 0;

    if(dev == NULL || rsubc == NULL)
    {
	    return RET_FAILURE;
	}	
    *rsubc = 1010;
    nim_reg_read(dev, R04_STATUS, &data, 1);
    if (0x00 != (0x20 & data))
    {
        nim_reg_read(dev, R79_PKT_ERR + 0x01, &per[1], 1);
        per[1] = per[1] & 0x7f;
        nim_reg_read(dev, R79_PKT_ERR, &per[0], 1);
        percount = (UINT16) (per[1] * 256 + per[0]);
        *rsubc = (UINT32) percount;
        NIM_PRINTF("current PER is  %d\n", percount);

        nim_reg_read(dev, R70_CAP_REG + 0x01, &data, 1);
        data = 0x7f & data;
        nim_reg_write(dev, R70_CAP_REG + 0x01, &data, 1);
        comm_delay(10);
        nim_reg_read(dev, R70_CAP_REG + 0x01, &data, 1);
        data = 0x80 | data;
        nim_reg_write(dev, R70_CAP_REG + 0x01, &data, 1);

        return SUCCESS;
    }
    else
    {
        return ERR_PARA;
    }
}


/*****************************************************************************
* static INT32 nim_s3503_get_new_ber(struct nim_device *dev, UINT32 *ber)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_get_new_ber(struct nim_device *dev, UINT32 *ber)
{
    UINT8 data = 0;
    UINT32 t_count = 0;
    UINT32 myber = 0;

    if((dev == NULL) || (ber == NULL))
    {
	    return RET_FAILURE;
	}	
    myber = 0;
    for (t_count = 0; t_count < 200; t_count++)
    {
        nim_reg_read(dev, R76_BIT_ERR + 0x02, &data, 1);
        if ((data & 0x80) == 0)
        {
            myber = data & 0x7f;
            nim_reg_read(dev, R76_BIT_ERR + 0x01, &data, 1);
            myber <<= 8;
            myber += data;
            nim_reg_read(dev, R76_BIT_ERR, &data, 1);
            myber <<= 8;
            myber += data;
            break;
        }
    }
    *ber = myber;

    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_get_new_per(struct nim_device *dev, UINT32 *per)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_get_new_per(struct nim_device *dev, UINT32 *per)
{
    UINT8 data = 0;
    UINT32 t_count = 0;
    UINT32 myper = 0;

    if((dev == NULL) || (per == NULL))
    {
	    return RET_FAILURE;
	}	
    myper = 0;
    for (t_count = 0; t_count < 200; t_count++)
    {
        nim_reg_read(dev, R79_PKT_ERR + 0x01, &data, 1);
        if ((data & 0x80) == 0)
        {
            myper = data & 0x7f;
            nim_reg_read(dev, R79_PKT_ERR, &data, 1);
            myper <<= 8;
            myper += data;
            break;
        }
    }
    *per = myper;
    //  NIM_PRINTF("!!!!!!!! myPER cost %d time, per = %d\n",t_count,myper);
    return SUCCESS;
}


/*****************************************************************************
* INT32 nim_s3503_get_agc(struct nim_device *dev, UINT8 *agc)
*
*  This function will access the NIM to determine the AGC feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* agc
*
* Return Value: INT32
*****************************************************************************/
// get signal intensity

INT32 nim_s3503_get_agc(struct nim_device *dev, UINT8 *agc)
{
    UINT8 data = 0;
    struct nim_s3501_private *priv= NULL;
	
#ifdef AGC_FILTER	
    UINT32 s_sum = 0;
    INT32 k = 0;
    INT32 n1 = 0;
    INT32 n2 = 0;
#endif

    if((dev == NULL) || (agc == NULL))
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    if (priv->tuner_config_data.qpsk_config & M3501_SIGNAL_DISPLAY_LIN)
    {
        nim_reg_read(dev, R04_STATUS, &data, 1);  //agc1锁定
        if (data & 0x01)
        {
            // AGC1 lock
            nim_reg_read(dev, R0A_AGC1_LCK_CMD + 0x01, &data, 1);  // read AGC1 gain
            if (data > 0x7f)
            {
            *agc = data - 0x80;
            }
            else
            {
            *agc = data + 0x80;
            }

        }
        else
        {
        *agc = 0;
        }
    }
    else
    {
        //CR0B
        nim_reg_read(dev, R07_AGC1_CTRL + 0x04, &data, 1);
        data = 255 - data;

        if (0x40 <= data)
        {
        data -= 0x40;
        }
        else if ((0x20 <= data) || (0x40 > data))
        {
        data -= 0x20;
         }
        else
        {
        data -= 0;
         }

        data /= 2;
        data += 16;
        *agc = (UINT8) data;
    }
#ifdef AGC_FILTER

#define    s_filter_len  4  //even is better.
    if(*agc == 0)
        agc_j = 0;
    if (agc_j == (s_filter_len - 1))
    {
        for(k = 0; k <= (s_filter_len - 1); k++)
        {
            s_sum = s_sum + s_data[k];
        }
        *agc = s_sum / s_filter_len;
        n1 = s_sum - (*agc) * s_filter_len;
        if(n1 >= (s_filter_len / 2))
            n1 = 1;
        else
            n1 = 0;
        *agc = *agc + n1;
        s_agc = *agc;
        agc_j = agc_j + 1;
    }
    else if (agc_j == s_filter_len)
    {
        n1 = (s_agc * (s_filter_len - 1)) / s_filter_len;
        n1 = (s_agc * (s_filter_len - 1)) - n1 * s_filter_len;
        n2 = *agc / s_filter_len;
        n2 = *agc - n2 * s_filter_len;
        n2 = n1 + n2;
        if(n2 >= (3 * (s_filter_len / 2)))
            n1 = 2;
        else if((n2 < (3 * (s_filter_len / 2))) && (n2 >= (s_filter_len / 2)))
            n1 = 1;
        else
            n1 = 0;
        *agc = (s_agc * (s_filter_len - 1)) / s_filter_len + *agc / s_filter_len + n1;
        s_agc = *agc;
    }
    else
    {
        s_data[agc_j] = *agc;
        *agc = *agc;
        agc_j = agc_j + 1;
    }
#endif
    return SUCCESS;
}







static void nim_s3503_print_map_type(UINT8 work_mode,UINT8 map_type)
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

static void nim_s3503_print_code_rate(UINT8 work_mode,UINT8 code_rate)
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

static void nim_s3503_print_work_mode(UINT8 work_mode)
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

static void nim_s3503_print_roll_off(UINT8 roll_off)
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

/*****************************************************************************
* static INT32 nim_s3503_waiting_channel_lock(struct nim_device *dev, UINT32 freq, UINT32 sym)
* Description: S3501 set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
INT32 nim_s3503_waiting_channel_lock(struct nim_device *dev, UINT32 freq, UINT32 sym)
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
    struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    timeout = 0;

    //不同的符号率影响硬件超时时间
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

    locktimes *= 3;

    while (priv->ul_status.s3501_chanscan_stop_flag == 0)
    {
        timeout ++ ;
        if (locktimes < timeout)                     // hardware timeout
        {
            priv->t_param.phase_noise_detect_finish = 1;
            nim_s3503_interrupt_clear(dev);
            NIM_PRINTF("    timeout \n");
            priv->ul_status.s3501_chanscan_stop_flag = 0;
            priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_SETTING;
            return ERR_FAILED;
        }

        nim_reg_read(dev, R04_STATUS, &intdata, 1);
        data = 0x3f;
        if ((intdata & 0x3f) == data) // if lock, get and print TP information
        {
            nim_s3503_reg_get_work_mode(dev, &work_mode);
            nim_s3503_print_work_mode(work_mode);

            priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_CLEAR;
            priv->ul_status.m_cur_freq = tempfreq;
            priv->ul_status.s3501_chanscan_stop_flag = 0;

            // ------ Get information start
            tempfreq = freq;
            nim_s3503_reg_get_freq(dev, &tempfreq);
            NIM_PRINTF("            Freq is %d\n", (LNB_LOACL_FREQ - tempfreq));

            nim_s3503_reg_get_symbol_rate(dev, &rs);
            NIM_PRINTF("            rs is %d\n", rs);

            nim_s3503_reg_get_code_rate(dev, &code_rate);
            nim_s3503_print_code_rate(work_mode,code_rate);

            if(1 == work_mode)
            {
                nim_s3503_reg_get_roll_off(dev, &roll_off);
                nim_s3503_print_roll_off(roll_off);
            }
            if(1 == work_mode)
            {
                nim_s3503_reg_get_modcod(dev, &modcod);
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

            nim_s3503_reg_get_iqswap_flag(dev, &iqswap_flag);
            NIM_PRINTF("            iqswap_flag is %d\n", iqswap_flag);

            nim_s3503_reg_get_map_type(dev, &map_type);
            nim_s3503_print_map_type(work_mode,map_type);

            // ------ Get information end

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

                priv->t_param.phase_noise_detect_finish = 1;
                if ((work_mode == M3501_DVBS2_MODE) && (map_type == MAP_TYPE_3) && (priv->t_param.t_phase_noise_detected == 0))
                {
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

                if ((work_mode == M3501_DVBS2_MODE) && (map_type <= MAP_TYPE_3))     //only s2 need dynamic power
                {
                     priv->t_param.t_dynamic_power_en = 1;
                }

                /* Keep current frequency.*/
                priv->ul_status.m_cur_freq = tempfreq;
                nim_s3503_interrupt_clear(dev);
                priv->ul_status.s3501_chanscan_stop_flag = 0;
                return SUCCESS;
            }
        }
        else
        {
            if (priv->ul_status.s3501_chanscan_stop_flag>0)
            {
                priv->ul_status.s3501_chanscan_stop_flag = 0;
                priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_SETTING;
                return ERR_FAILED;
            }
           // comm_delay(200);
           comm_sleep(10);
        }
    }

    priv->ul_status.s3501_chanscan_stop_flag = 0;
    return SUCCESS;
}


/*****************************************************************************
* static INT32 nim_s3503_debug_intask(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_debug_intask(struct nim_device *dev)
{
#ifdef NEW_CR_ADPT_SNR_EST_RPT	
	UINT16  snr_db = 0;
#endif

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    NIM_PRINTF(" Enter function nim_s3503_debug_intask \n");
#ifdef HW_ADPT_CR
#ifdef HW_ADPT_CR_MONITOR
    nim_s3503_cr_adaptive_monitor(dev);
#endif
#ifdef HW_ADPT_NEW_CR_MONITOR
    nim_s3503_cr_new_adaptive_monitor(dev);
#ifdef NEW_CR_ADPT_SNR_EST_RPT
    nim_s3503_get_snr_db(dev,&snr_db);
#endif
#endif
#ifdef LLR_SHIFT_DEBUG
    nim_s3503_fec_llr_shift(dev);
#endif
#else
#ifdef SW_ADPT_CR
#ifdef SW_SNR_RPT_ONLY
    nim_s3503_cr_sw_snr_rpt(dev);
#else
    nim_s3503_cr_sw_adaptive(dev);
#endif
#endif
#endif
    return SUCCESS;
}

// begin add operation for s3501 optimize
/*****************************************************************************
* static INT32 nim_s3503_set_err(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_set_err(struct nim_device *dev)
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

/*****************************************************************************
 * INT32 nim_s3503_check_BER(struct nim_device *dev, UINT32 *rsubc)
 * Get bit error ratio(10E-5)
 *
 *  Arguments:
 *  Parameter1: struct nim_device *dev
 *  Parameter2: UINT16* rsubc
 *
 * Return Value: INT32
 *****************************************************************************/
INT32 nim_s3503_check_ber(struct nim_device *dev, UINT32 *rsubc)
{
    UINT8 data = 0;
    UINT8 work_mode = 0;
    UINT8 ber_data[2] = {0};
    UINT32 u32ber_data[2] = {0};
    UINT32 uber_data = 0;
    UINT32 i = 0;

    if((dev == NULL) || (rsubc == NULL))
    {
	    return RET_FAILURE;
	}	
    nim_reg_read(dev, R04_STATUS, &data, 1);
    if(0x3f != (data & 0x3f))
    {
        va_ber = 0;
        va_ber_window = 0;
        *rsubc = 0xffff;
        NIM_PRINTF("----NIM_WARNING: FEC unlock, can not get BER\n");
        return SUCCESS;
    }
    else
    {
        nim_s3503_reg_get_work_mode(dev, &work_mode);

        if(0 == work_mode) // DVBS
        {
            if(va_ber_window < BER_DVBS_BETA)
            {
                for (i = 0; i < BER_DVBS_BETA; i++)
                {
                    nim_reg_read(dev, R54_VITERBI_FRAME_SYNC + 1, ber_data, 2);
                    u32ber_data[0] = (UINT32) ber_data[0];
                    u32ber_data[1] = (UINT32) (ber_data[1] & 0x0f);
                    u32ber_data[1] <<= 8;
                    uber_data = u32ber_data[1] + u32ber_data[0];
                    va_ber = va_ber + uber_data;
                    va_ber_window++;
                }
                *rsubc = (va_ber * 100) / BER_DVBS_BETA;
            }
            else
            {
                nim_reg_read(dev, R54_VITERBI_FRAME_SYNC + 1, ber_data, 2);
                u32ber_data[0] = (UINT32) ber_data[0];
                u32ber_data[1] = (UINT32) (ber_data[1] & 0x0f);
                u32ber_data[1] <<= 8;
                uber_data = u32ber_data[1] + u32ber_data[0];
                va_ber = va_ber + uber_data - (va_ber / BER_DVBS_BETA);
                *rsubc = (va_ber * 100) / BER_DVBS_BETA;
            }
        }
        else // DVBS2
        {
            if(va_ber_window < BER_DVBS2_BETA)
            {
                for (i = 0; i < BER_DVBS2_BETA; i++)
                {
                    nim_reg_read(dev, RD3_BER_REG + 0x01, ber_data, 2);
                    u32ber_data[0] = (UINT32) ber_data[0];
                    u32ber_data[1] = (UINT32) ber_data[1];
                    u32ber_data[1] <<= 8;
                    uber_data = u32ber_data[1] + u32ber_data[0];
                    va_ber = va_ber + uber_data;
                    va_ber_window++;
                }
                *rsubc = (va_ber * 10000) / (BER_DVBS2_BETA * 648);
            }
            else
            {
                nim_reg_read(dev, RD3_BER_REG + 0x01, ber_data, 2);
                u32ber_data[0] = (UINT32) ber_data[0];
                u32ber_data[1] = (UINT32) ber_data[1];
                u32ber_data[1] <<= 8;
                uber_data = u32ber_data[1] + u32ber_data[0];
                va_ber = va_ber + uber_data - (va_ber / BER_DVBS2_BETA);
                *rsubc = (va_ber * 10000) / (BER_DVBS2_BETA * 648);
            }
        }

        return SUCCESS;
    }
}

/*****************************************************************************
* INT32 nim_s3503_get_ldpc(struct nim_device *dev, UINT32 *rsubc)
* Get LDPC average iteration number
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_get_ldpc(struct nim_device *dev, UINT32 *rsubc)
{
    UINT8 data = 0;

    if((dev == NULL) || (rsubc == NULL))
    {
	    return RET_FAILURE;
	}	

    // read single LDPC iteration number
    nim_reg_read(dev, RAA_S2_FEC_ITER, &data, 1);
    *rsubc = (UINT32) data;
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_get_phase_error(struct nim_device *dev, INT32 *phase_error)
* Description: S3501 set LNB votage 12V enable or not
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 flag
*
* Return Value: SUCCESS
*****************************************************************************/
INT32 nim_s3503_get_phase_error(struct nim_device *dev, INT32 *phase_error)
{
    UINT8 rdata = 0;
    UINT8 data = 0;

    if((dev == NULL) || (phase_error == NULL))
    {
	    return RET_FAILURE;
	}	
    nim_reg_read(dev, RC0_BIST_LDPC_REG + 4, &data, 1);
    if (data & 0x80)
    {
        data &= 0x7f;
        nim_reg_write(dev, RC0_BIST_LDPC_REG + 4, &data, 1);
    }
    nim_reg_read(dev, RC0_BIST_LDPC_REG, &data, 1);
    if ((data & 0x04) == 0)
    {
        nim_reg_read(dev, RC0_BIST_LDPC_REG + 5, &rdata, 1);
        data |= 0x04;
        nim_reg_write(dev, RC0_BIST_LDPC_REG, &data, 1);
        if (rdata & 0x80)
        {
        *phase_error = rdata - 256;
         }
        else
        {
        *phase_error = rdata;       // phase error is signed!!!
        }
        NIM_PRINTF("phase error is %d\n", (*phase_error));
        return SUCCESS;  // means phase error measured.
    }
    else
    {
        *phase_error = 0;
        return ERR_FAILUE;  // means that phase error is not ready
    }
}

/*****************************************************************************
* static INT32 nim_s3503_get_type(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_get_type(struct nim_device *dev)
{
    UINT8 temp[4] = { 0x00, 0x00, 0x00, 0x00 };
    UINT32 m_value = 0x00;
	struct nim_s3501_private *priv = NULL;
    
	if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    
	priv = (struct nim_s3501_private *) dev->priv;

    priv->ul_status.m_s3501_type = 0x00;
    nim_reg_read(dev, RA3_CHIP_ID, temp, 4);
    m_value = temp[1];
    m_value = (m_value << 8) | temp[0];
    m_value = (m_value << 8) | temp[3];
    m_value = (m_value << 8) | temp[2];
    priv->ul_status.m_s3501_type = m_value;
    nim_reg_read(dev, RA6_VER_SUB_ID, temp, 1); // if 1,C3503;else S3503
    if(temp[0] == 0x01)
    {
        priv->ul_status.m_s3501_sub_type = NIM_C3503_SUB_ID;
     }
    else
    {
        priv->ul_status.m_s3501_sub_type = NIM_S3503_SUB_ID;
    }
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_get_snr_db(struct nim_device *dev,UINT16 *snr_db)
* Get estimated CNR in unit 0.01dB from CR new adaptive
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: UINT16
*****************************************************************************/

INT32 nim_s3503_get_snr_db(struct nim_device *dev,UINT16 *snr_db)
{
    UINT16 linear_cn = 1 ;
    UINT16 diff_cn = 0;
    UINT16 ref_est_noise = 0 ;
    UINT16 ref_cn = 0;
    UINT8 rdata[2]={0};
    UINT16 rdata16 = 0;
    UINT16 est_noise_1 = 0; // here must be test again
    UINT8 modu = 0 ;
    UINT16 snr_cn = 0;

    if((dev == NULL) || (snr_db == NULL))
	{
        return RET_FAILURE;
	}	
    nim_reg_read(dev, r13b_est_noise, rdata, 2);
    rdata16 = rdata[1];
    est_noise_1 = (rdata[0] | (rdata16 << 8)) >> 4; // here must be test again
    ADPT_NEW_CR_PRINTF("EST_NOISE=%d--0x%x\n", est_noise_1, est_noise_1);
    nim_reg_read(dev, RF8_MODCOD_RPT, &modu, 1);
    modu = (modu & 0x7f) >> 2;
    switch(modu)
    {
    case 0:
        ADPT_NEW_CR_PRINTF("unknown constellation\n");
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11: // QPSK
        ADPT_NEW_CR_PRINTF("QPSK\n");
        if(est_noise_1 > EST_NOISE_221)    //8.0dB because when c/n>8.0dB, estimate c/n error is bigger than 0.4dB
        {
            linear_cn = 196;
            diff_cn = 5;
            ref_est_noise = 47900; // QPSK, 2.0 AWGN frequency=1000M, 5Msymbol rate; roll off=0.35;
                               //pilot off;1/2code rate
            ref_cn = 200;
        }
        else
        {
            linear_cn = 155;
            diff_cn = 5;
            ref_est_noise = 22100; // QPSK, 8.1dB AWGN frequency=1000M, 5Msymbol rate;
                               // roll off=0.35;pilot off;1/2code rate
            ref_cn = 810;
        }
        break;
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:  // 8PSK
        ADPT_NEW_CR_PRINTF("8PSK\n");
        linear_cn = 100;
        diff_cn = 5;
        ref_est_noise = 21500; // 8psk, 6.0 AWGN frequency=1000M, 5Msymbol rate; roll off=0.35;pilot off;3/5code rate
        ref_cn = 600;
        break;
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23: // 16APSK
        ADPT_NEW_CR_PRINTF("16PSK\n");
        if(est_noise_1 > EST_NOISE_96)
        {
            linear_cn = 50;
            diff_cn = 5;
            ref_est_noise = 11300; // 10.0 AWGN frequency=1000M, 5Msymbol rate; roll off=0.35;pilot off;2/3code rate
            ref_cn = 1000;
        }
        else
        {
            linear_cn = 51;
            diff_cn = 5;
            ref_est_noise = 9600; // 11.4 AWGN frequency=1000M, 5Msymbol rate; roll off=0.35;pilot off;2/3code rate
            ref_cn = 1150;
        }
        break;
    case 24:
    case 25:
    case 26:
    case 27://32APSK
        ADPT_NEW_CR_PRINTF("32PSK,24->28 \n");
        linear_cn = 25;
        diff_cn = 5;
        ref_est_noise = 5400; // 14 AWGN frequency=1000M, 5Msymbol rate; roll off=0.35;pilot off;3/4code rate
        ref_cn = 1400;
        break;
    case 28:    // 32APSK
        ADPT_NEW_CR_PRINTF("32PSK,28\n");
        linear_cn = 25;
        diff_cn = 5;
        ref_est_noise = 3700; // 17 AWGN frequency=1000M, 5Msymbol rate; roll off=0.35;pilot off;9/10code rate
        ref_cn = 1700;
        break;
    default:
        break;
    }
    snr_cn = ((ref_est_noise - est_noise_1 * 100) / linear_cn) * diff_cn + ref_cn; // the current c/n value X10
    ADPT_NEW_CR_PRINTF("c/n=%d\n", snr_cn);
     *snr_db=snr_cn;

     return SUCCESS;
}


//for SNR use
/*****************************************************************************
* static UINT8 nim_s3503_get_snr_index(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
UINT8 nim_s3503_get_snr_index(struct nim_device *dev)
{
    INT16 lpf_out16 = 0;
    INT16 agc2_ref5 = 0;
    INT32 snr_indx = 0;
    UINT8 data[2] ={0};
    UINT16 tdata[2]={0};

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

    snr_indx = (lpf_out16 * agc2_ref5 / 21) - 27;//27~0

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

/*****************************************************************************
* INT32 nim_s3503_reg_get_freqoffset(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3503_reg_get_freqoffset(struct nim_device *dev)
{
    INT32 freq_off = 0;
    UINT8 data[3] ={0};
    UINT32 tdata = 0;
    UINT32 temp = 0;
    UINT32  sample_rate = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    nim_s3503_get_dsp_clk(dev, &sample_rate);
    temp = 0;
    nim_reg_read(dev, R69_RPT_CARRIER, data, 3);
    tdata = (data[0] & 0xff) + ((data[1] & 0xff) << 8);
    if ((data[2] & 0x01) == 1)
    {
        temp = (tdata ^ 0xffff) + 1;
     }
    else
    {
        temp = tdata & 0xffff;
     }
    if ((data[2] & 0x01) == 1)
    {
        freq_off = 0 - nim_s3501_multu64div(temp, sample_rate, 92160000); // 92160000 == 90000*1024
     }
    else
    {
        freq_off = nim_s3501_multu64div(temp, sample_rate, 92160000);
     }

    return freq_off;
}


/*****************************************************************************
* static INT32 nim_s3503_reg_get_modcod(struct nim_device *dev, UINT8 *modcod)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_reg_get_modcod(struct nim_device *dev, UINT8 *modcod)
{
    UINT8 data = 0;

    if((dev == NULL) || (modcod == NULL))
    {
	    return RET_FAILURE;
	}	
    //  NIM_PRINTF("Enter Fuction nim_s3503_reg_get_modcode \n");
    nim_reg_read(dev, RF8_MODCOD_RPT, &data, 1);
    *modcod = data & 0x7f;
    return SUCCESS;
    // bit0 : pilot on/off
    // bit[6:1] : modcod,
}

/*****************************************************************************
* static INT32 nim_s3503_get_LDPC_iter_cnt(struct nim_device *dev, UINT16 *iter_cnt)
* Description: get ldpc iter counter
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *iter_cnt, it is 10 bits width
*
* Return Value: INT32
*****************************************************************************/

static INT32 nim_s3503_get_ldpc_iter_cnt(struct nim_device *dev, UINT16 *iter_cnt)
{
    UINT8 data[2] ={0};

    if((dev == NULL) || (iter_cnt == NULL))
	{
        return RET_FAILURE;
	}	
    // read LDPC iteration number, maybe once, maybe the max, according to reg_crc1[3]
    nim_reg_read(dev, RAA_S2_FEC_ITER, data, 2);
    data[1] &= 0x03;
    *iter_cnt = (UINT16)(data[1] * 256 + data[0]);
    return SUCCESS;
}


