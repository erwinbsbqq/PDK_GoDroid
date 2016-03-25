/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: nim_s3281_im.c
*
*    Description: s3281,im layer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "nim_s3281.h"

#define COMPARE_VALUE_307   307
#define COMPARE_VALUE_204   204
#define COMPARE_VALUE_88   88
#define COMPARE_VALUE_40   40

#define TUNER_RETRY_2  2
#define SYM_1000  1000
#define SYM_7000  7000

#define FREQ_48000  48000
#define FREQ_859000  900000
#define FREQ_10000  10000
#define FREQ_36000  36000

#define FEC_VALUE_4 4
#define FEC_VALUE_8 8
#define DIVI_10   10

#define MAGIC_CONST_44000  44000
#define MAGIC_CONST_0      0
#define MAGIC_CONST_341    341
#define MAGIC_CONST_350    350
#define MAGIC_CONST_373    373
#define MAGIC_CONST_407    407
#define MAGIC_CONST_430    430
#define MAGIC_CONST_583    583
#define MAGIC_CONST_608    608
#define MAGIC_CONST_628    628
#define MAGIC_CONST_650    650
#define MAGIC_CONST_667    667
#define MAGIC_CONST_685    685
#define MAGIC_CONST_704    704
#define MAGIC_CONST_155    155
#define MAGIC_CONST_201    201
#define MAGIC_CONST_245    245
#define MAGIC_CONST_315    315
#define MAGIC_CONST_387    387
#define MAGIC_CONST_532    532
#define MAGIC_CONST_694    694
#define MAGIC_CONST_767    767
#define MAGIC_CONST_806    806
#define MAGIC_CONST_878    878
#define MAGIC_CONST_914    914
#define MAGIC_CONST_940    940
#define MAGIC_CONST_967    967
#define MAGIC_CONST_91     91
#define MAGIC_CONST_114    114
#define MAGIC_CONST_142    142

#define PHASENOISE_DEF_J83AC
#define PHASENOISE_DEF_J83B

#define  SYS_TUNER_MODULE 0



static INT16   init_foffset = 0;
static UINT8   if_agc_min_ch = 0;


static INT32  nim_s3281_j83ac_channel_change(struct nim_device *dev,NIM_CHANNEL_CHANGE_T *pstchl_change);
static INT32  nim_s3281_j83b_channel_change(struct nim_device *dev,NIM_CHANNEL_CHANGE_T *pstchl_change);
static void   nim_s3281_dvbc_set_qam_order(UINT8 known, UINT8 qam_order );

//=============================================================
#if (QAM_WORK_MODE == QAM_ONLY)

INT32 nim_s3281_dvbc_read(UINT16 bmemadr, UINT8 *pdata,UINT8 blen)
{
    UINT8 chip_adr = 0;
    INT32 b_ret = 0;

#ifndef SYS_DEM_BASE_ADDR
    chip_adr = 0x40;
#else
    chip_adr = SYS_DEM_BASE_ADDR;
#endif

    NIM_MUTEX_ENTER(ali_m3281_nim_priv);

    b_ret = nim_i2c_write(ali_m3281_nim_priv->i2c_type, chip_adr, &bmemadr, 1);
    if(b_ret == SUCCESS)
    {
        b_ret = nim_i2c_read(ali_m3281_nim_priv->i2c_type, chip_adr, pdata, blen);
    }

    NIM_MUTEX_LEAVE(ali_m3281_nim_priv);

    return b_ret;
}
INT32 nim_s3281_dvbc_write(UINT16 bmemadr,UINT8 *pdata,UINT8 blen)
{
    UINT8 chip_adr = 0;
    INT32 i = 0 ;
    INT32 b_ret = 0;

#ifndef SYS_DEM_BASE_ADDR
    chip_adr = 0x40;
#else
    chip_adr = SYS_DEM_BASE_ADDR;
#endif
    UINT8 btemp[blen + 1];

    for(i = 1; i < blen + 1; i++)
    {
        btemp[i] = pdata[i - 1];
     }
    btemp[0] = bmemadr;

    NIM_MUTEX_ENTER(ali_m3281_nim_priv);

    b_ret = nim_i2c_write(ali_m3281_nim_priv->i2c_type, chip_adr, btemp, blen + 1);
    NIM_MUTEX_LEAVE(ali_m3281_nim_priv);

    return b_ret;
}

#elif (QAM_WORK_MODE == QAM_SOC)
INT32 nim_s3281_dvbc_read(UINT16 bmemadr, UINT8 *pdata, UINT8 blen)
{
    INT32 i = 0;

    if((pdata == NULL) || (blen == 0))
    {
	    return ERR_FAILURE;
	}	
    for (i = 0; i < blen; i++)
    {
        *(pdata + i) = NIM_S3281_GET_BYTE((S3281_QAM_SOC_BASE_ADDR + bmemadr + i));
    }
    return SUCCESS;
}
INT32 nim_s3281_dvbc_write(UINT16 bmemadr, UINT8 *pdata, UINT8 blen)
{
    INT32 i = 0;
	
    if((pdata == NULL) || (blen == 0))
    {
	    return ERR_FAILURE;
	}	

    for (i = 0; i < blen; i++)
    {
        NIM_S3281_SET_BYTE((S3281_QAM_SOC_BASE_ADDR + bmemadr + i),  *(pdata + i));
		
    }
    return SUCCESS;
}
#endif


INT32 system_reg_read(UINT32 bmemadr, UINT8 *pdata, UINT8 blen)
{
	INT32 i;
	for (i = 0; i < blen; i++)
	{
		*(pdata + i) = NIM_S3281_GET_BYTE((S3281_SOC_BASE_ADDR + bmemadr + i));
	}
	return SUCCESS;
}

INT32 system_reg_write(UINT32 bmemadr, UINT8 *pdata, UINT8 blen)
{
	INT32 i;
	for (i = 0; i < blen; i++)
	{
		NIM_S3281_SET_BYTE((S3281_SOC_BASE_ADDR + bmemadr + i), *(pdata + i));
	}
	return SUCCESS;
}


static void nim_s3281_dvbc_set_qam_order(UINT8 known, UINT8 qam_order)
{
    UINT8 data = 0;

    //CR38.
    //since there is a "work_mode" bit, in this function, should be normal mode.
    nim_s3281_dvbc_read(NIM_S3202_FSM1, &data, 1);
    if(NIM_S3202_QAM_ORDER_KNOWN == known)
    {
        data = (UINT8)((data & 0x80) | 0x40 | (qam_order & 0x0f));
    }
    else
    {
        data = (UINT8)((data & 0x80) | 0x00 | (qam_order & 0x0f));
        data &= 0xbf;
    }
    nim_s3281_dvbc_write(NIM_S3202_FSM1, &data, 1);

    return;
}

INT32 nim_s3281_dvbc_channel_search(struct nim_device *dev,UINT32 freq)
{
    if(NULL == dev)
    {
        return ERR_FAILUE;
    }

    return SUCCESS;
}

INT32 nim_s3281_dvbc_get_lock(struct nim_device *dev, UINT8 *lock)
{
    UINT8 data = 0;

    if((NULL == dev) || (lock == NULL))
    {
        return ERR_FAILUE;
    }

    //CR56.
    nim_s3281_dvbc_read(NIM_S3202_MONITOR1, &data, 1);
    data &= NIM_S3281_ALL_LOCK;
    if(NIM_S3281_ALL_LOCK == data)
    {
        *lock = 1;
    }
    else
    {
        *lock = 0;
    }

    return SUCCESS;
}

INT32 nim_s3281_dvbc_get_freq(struct nim_device *dev, UINT32 *freq)
{
    INT32 freq_off = 0;
    UINT8 data[2] = {0};

    if((NULL == dev) || (freq == NULL))
    {
        return ERR_FAILUE;
    }

    //CR48.CR49
    nim_s3281_dvbc_read(NIM_S3202_FSM17, data, 2);

    if(0x00 == (data[1] & 0x10)) //positive value.
    {
        freq_off = (INT32)(data[0] | ((data[1] & 0x1f) << 8)) ;
    }
    else//negtive.
    {
        freq_off = (INT32)(data[0] | ((data[1] & 0x1f) << 8) | 0xFFFFE000);
    }

    S3281_PRINTF("data is 0x%x 0x%x !\n", data[0], data[1]);

    //actually is /1024*1000/10. unit is KHz*10.
    freq_off = (INT32)(freq_off * 1000 / 1024);

    //here I think it should be refer the I/Q status.
    //*freq -= freq_off;

    S3281_PRINTF("offset is %d !\n", (int)freq_off);

    //joey 20080504. Use persudo "freq offset" for auto-scan xxx.000 display. not xxx.150/130 display.
    *freq = (s3281_cur_channel_info.frequency - init_foffset) / 10;

    S3281_PRINTF("*freq is %d !\n", (int)(*freq));

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3281_dvbc_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
* Read S3202 symbol rate
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *sym_rate            : Symbol rate in kHz
*
* Return Value: void
*****************************************************************************/
INT32 nim_s3281_dvbc_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
{
    UINT8 data[2] = {0};
    UINT32 rtp_sym = 0;

    if((NULL == dev) || (sym_rate == NULL))
    {
        return ERR_FAILUE;
    }

    nim_s3281_dvbc_read(NIM_S3202_FSM15, data, 2);

    rtp_sym = (UINT32)(((0x1f & data[1]) << 8) | data[0]);

    *sym_rate = (UINT32)(rtp_sym*1000 / 1024);

    return SUCCESS;
}

INT32 nim_s3281_dvbc_get_qam_order(struct nim_device *dev, UINT8 *qam_order)
{
    UINT8 data = 0;

    if((NULL == dev) || (qam_order == NULL))
    {
        return ERR_FAILUE;
    }

    nim_s3281_dvbc_read(NIM_S3202_FSM19, &data, 1);

    *qam_order = (UINT8)(data & 0x0f);

    return SUCCESS;
}

INT32 nim_s3281_dvbc_get_fft_result(struct nim_device *dev, UINT32 freq,UINT32 *start_adr )
{
    if((NULL == dev) || (start_adr == NULL))
    {
        return ERR_FAILUE;
    }


    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3281_dvbc_get_agc(struct nim_device *dev, UINT8 *agc)
*
*  This function will access the NIM to determine the AGC feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* agc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3281_dvbc_get_agc(struct nim_device *dev, UINT8 *agc)
{
    UINT8  data[4] = {0};
    UINT16 temp = 0;
    UINT8  if_agc_gain = 0;
    UINT8  tun_agc_gain = 0;
    UINT8  rf_en = 0;
    UINT8  if_en = 0;
    UINT8  rf_agc_max_priv = 0;
    UINT8  rf_agc_min_priv = 0;
    UINT8  if_agc_max_priv = 0;
    UINT8  if_agc_min_priv = 0;

    if((NULL == dev) || (agc == NULL))
    {
        return ERR_FAILUE;
    }

    nim_s3281_dvbc_read(NIM_S3202_AGC6, data, 4);
    rf_agc_max_priv = data[0];
    rf_agc_min_priv = data[1] ;
    if_agc_max_priv = data[2];
    if_agc_min_priv = data[3];

    //CR56.
    nim_s3281_dvbc_read(NIM_S3202_MONITOR1, data, 1);
    if(0x01 == (data[0] & 0x01))    //AGC is locked
    {
        nim_s3281_dvbc_read(NIM_S3202_AGC10, data, 4);
        rf_en = (data[0] & 0x20) >> 5;
        if_en = (data[0] & 0x02) >> 1;

        //IF_AGC_GAIN operation.
        temp = (UINT16)(((data[2] & 0x03) << 8) | data[1]);
        if_agc_gain = (UINT8)((temp >> 2) + 0x80);

        //TUN_AGC_GAIN operation.
        temp = (UINT16)(((data[3] & 0x0f) << 6) | ((data[2] >> 2) & 0x3f));
        tun_agc_gain = (UINT8)((temp >> 2) + 0x80);

        if (rf_en && if_en)  // RF agc & IF agc are all opened
        {
            if (tun_agc_gain < rf_agc_min_priv)// we not use 100 and 0 for indicator.
            {
               *agc = 99;
            }
            else if (tun_agc_gain <= rf_agc_max_priv)
            {
                if(0 != (rf_agc_max_priv - rf_agc_min_priv))
                {
                    *agc = (99 - (tun_agc_gain - rf_agc_min_priv)*40 / (rf_agc_max_priv - rf_agc_min_priv));
                }
            }
            else if (if_agc_gain <= if_agc_max_priv)
            {
                if(0 != (if_agc_max_priv - if_agc_min_priv))
                {
                    *agc = (60 - (if_agc_gain - if_agc_min_priv)*57 / (if_agc_max_priv - if_agc_min_priv));
                }
            }
            else
            {
               *agc = 1;
            }
        }
        else if (rf_en && !if_en) // RF agc opened & IF agc closed
        {
            if (tun_agc_gain < rf_agc_min_priv)
            {
                *agc = 99;
            }
            else if(tun_agc_gain < rf_agc_max_priv)
            {
                if(0 != (rf_agc_max_priv - rf_agc_min_priv))
                {
                    *agc = 99 - 99*(tun_agc_gain - rf_agc_min_priv) / (rf_agc_max_priv - rf_agc_min_priv);
                }
            }
            else
            {
               *agc = 1;
            }
        }
        else if (!rf_en && if_en) // RF agc closed & IF agc opened
        {
            if (if_agc_gain < if_agc_min_priv)
            {
            *agc = 99;
            }
            else if (if_agc_gain < if_agc_max_priv)
            {
                if(0 != (if_agc_max_priv - if_agc_min_priv))
                {
                    *agc = 99 - 99*(if_agc_gain - if_agc_min_priv) / (if_agc_max_priv - if_agc_min_priv);
                }
            }
            else
            {
               *agc = 1;
            }
        }
        else  // RF agc & IF agc are all closed
        {
           *agc = 1;
        }

    }
    else
    {
        *agc = 0x00;
     }

    //S3281_PRINTF("[%s]line=%d,agc=%d!\n", __FUNCTION__, __LINE__, *agc);
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3281_dvbc_get_snr(struct nim_device *dev, UINT8 *snr)
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
INT32 nim_s3281_dvbc_get_snr(struct nim_device *dev, UINT8 *snr)
{
    UINT8 data[2] = {0};
    UINT32 rpt_power = 0;


    if((NULL == dev) || (snr == NULL))
    {
        return ERR_FAILUE;
    }

    //CR56.
    nim_s3281_dvbc_read(NIM_S3202_MONITOR1, data, 1 );
    if((data[0]&NIM_S3281_ALL_LOCK) == NIM_S3281_ALL_LOCK)    //dem is all locked.
    {
        //CR6C,CR6D.
        nim_s3281_dvbc_read(NIM_S3202_SNR_MONI1, data, 2);

        rpt_power = (UINT32)(data[1] << 8 | data[0]);

        if(rpt_power > COMPARE_VALUE_307)
        {
            *snr = 5;
        }
        else if(rpt_power > COMPARE_VALUE_204)
        {
            *snr = 5 + (307 - rpt_power)*55 / 103;
         }
        else  if(rpt_power > COMPARE_VALUE_88)
        {
             *snr = 60 + ((204 - rpt_power)*35) / 116;
        }
        else
        {
           *snr = 95;
        }
    }
    else
    {
        *snr = 0x00;
    }

    //S3281_PRINTF("[%s]line=%d,snr=%d!\n", __FUNCTION__, __LINE__, *snr);
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3281_dvbc_get_rf_level(struct nim_device *dev, UINT16 *rflevel)
*
*  This function will access the NIM to determine the RF level feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *rflevel
*  Real_RF_level and rflevel relation is : rflevel = -(Real_RF_level * 10)
*  eg.  if Real_RF_level = -30.2dBm then , rflevel = -(Real_RF_level * 10) = 302
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3281_dvbc_get_rf_level(struct nim_device *dev,UINT16 *rflevel)
{
    UINT8  data[3] ={0};
    UINT16 temp16 = 0;
    UINT16 if_agc_gain = 0;
    UINT16 rf_agc_gain = 0;
    UINT32 temp32 = 0;

    if((NULL == dev) || (rflevel == NULL))
    {
        return ERR_FAILUE;
    }

    nim_s3281_dvbc_read(NIM_S3202_AGC11, data, 3);
    temp16 = (UINT16)(((data[1] & 0x03) << 8) | data[0]);
    if_agc_gain = (UINT16)((temp16 + 0x200) & 0x3FF);
    temp16 = (UINT16)(((data[2] & 0x0f) << 6) | ((data[1] >> 2) & 0x3f));
    rf_agc_gain = (UINT16)((temp16 + 0x200) & 0x3FF);

#if (SYS_TUNER_MODULE ==DCT7044)
    if (if_agc_gain > AGC_GAIN_VALUE_208)
        temp32 = (UINT32)((179 * if_agc_gain + 30205 + 50 ) / 100) ;
    else if (if_agc_gain > MAGIC_CONST_142)
        temp32 = (UINT32)((159 * if_agc_gain + 34309 + 50) / 100);
    else if (if_agc_gain > MAGIC_CONST_114)
        temp32 = (UINT32) ((114 * if_agc_gain + 40671 + 50) / 100);
    else if (if_agc_gain > MAGIC_CONST_91 )
        temp32 = (UINT32 )((52 * if_agc_gain + 47752 + 50) / 100);
    else if ((if_agc_gain <= MAGIC_CONST_91) && (rf_agc_gain > MAGIC_CONST_967 ))
        temp32 = (UINT32) 510;
    else if (rf_agc_gain > MAGIC_CONST_967 )
        temp32 = (UINT32)((13 * rf_agc_gain + 38793 + 50) / 100);
    else if (rf_agc_gain > MAGIC_CONST_940)
        temp32 = (UINT32 )((26 * rf_agc_gain + 26030 + 50) / 100);
    else if (rf_agc_gain > MAGIC_CONST_914 )
        temp32 = (UINT32 )((58 * rf_agc_gain - 3831 + 50) / 100);
    else if (rf_agc_gain > MAGIC_CONST_878 )
        temp32 = (UINT32)(( 75 * rf_agc_gain - 19650 + 50) / 100);
    else if (rf_agc_gain > MAGIC_CONST_806)
        temp32 = (UINT32)(( 83 * rf_agc_gain - 26967 + 50) / 100);
    else if (rf_agc_gain > MAGIC_CONST_767)
        temp32 = (UINT32)((67 * rf_agc_gain - 13533 + 50) / 100);
    else if (rf_agc_gain > MAGIC_CONST_694)
        temp32 = (UINT32)((49 * rf_agc_gain - 225 + 50) / 100);
    else if (rf_agc_gain > MAGIC_CONST_532)
        temp32 = (UINT32) ((28 * rf_agc_gain + 14294 + 50) / 100);
    else if(rf_agc_gain > MAGIC_CONST_387 )
        temp32 = (UINT32)(( 21 * rf_agc_gain + 18026 + 50) / 100);
    else if (rf_agc_gain > MAGIC_CONST_315)
        temp32 = (UINT32)(( 33 * rf_agc_gain + 13400 + 50) / 100);
    else if (rf_agc_gain > MAGIC_CONST_245)
        temp32 = (UINT32)((64 * rf_agc_gain + 3650 + 50) / 100);
    else if (rf_agc_gain > MAGIC_CONST_201)
        temp32 = (UINT32)((141 * rf_agc_gain - 15123 + 50) / 100);
    else if (rf_agc_gain > MAGIC_CONST_155)
        temp32 = (UINT32)((570 * rf_agc_gain - 101283 + 50) / 100);
    else
        temp32 = 0;
#elif (SYS_TUNER_MODULE ==DCT70701)
    if (if_agc_gain > AGC_GAIN_VALUE_721)
        temp32 = 770;
    else if (if_agc_gain > MAGIC_CONST_704)
        temp32 = (UINT32)((176 * if_agc_gain - 49896) / 100);
    else if (if_agc_gain > MAGIC_CONST_685)
        temp32 = (UINT32)((211 * if_agc_gain - 74544) / 100);
    else if (if_agc_gain > MAGIC_CONST_667)
        temp32 = (UINT32)((167 * if_agc_gain - 44395) / 100);
    else if (if_agc_gain > MAGIC_CONST_650)
        temp32 = (UINT32)((294 * if_agc_gain - 129098) / 100);
    else if (if_agc_gain > MAGIC_CONST_628)
        temp32 = (UINT32)((227 * if_agc_gain - 85550) / 100);
    else if (if_agc_gain > MAGIC_CONST_608)
        temp32 = (UINT32)((250 * if_agc_gain - 100000) / 100);
    else if (if_agc_gain > MAGIC_CONST_583)
        temp32 = (UINT32)((200 * if_agc_gain - 69600) / 100);
    else
        temp32 = 470;
#elif (SYS_TUNER_MODULE ==TDCCG0X1F)
    if (if_agc_gain > AGC_GAIN_VALUE_456)
        temp32 = 850;
    else if (if_agc_gain > MAGIC_CONST_430)
        temp32 = (UINT32)((222 * if_agc_gain - 16460) / 100);
    else if (if_agc_gain > MAGIC_CONST_407)
        temp32 = (UINT32)((217 * if_agc_gain - 14319) / 100);
    else if (if_agc_gain > MAGIC_CONST_373)
        temp32 = (UINT32)((205 * if_agc_gain - 9465) / 100);
    else if (if_agc_gain > MAGIC_CONST_350)
        temp32 = (UINT32)((434 * if_agc_gain - 94878) / 100);
    else if (if_agc_gain > MAGIC_CONST_341)
        temp32 = (UINT32)((1111 * if_agc_gain - 331851) / 100);
    else
        temp32 = 460;
#endif

    *rflevel = (UINT16)temp32;
    S3281_PRINTF("[%s]line=%d,rflevel=%d!\n", __FUNCTION__, __LINE__, *rflevel);
    return SUCCESS;

}

/*****************************************************************************
* INT32 nim_s3281_dvbc_get_cn_value(struct nim_device *dev, UINT16 *cnvalue)
*
*  This function will access the NIM to determine the C/N  feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *cnvalue
*  Real_CN_value and cnvalue relation is : cnvalue = Real_CN_value * 10
*  eg.  if Real_CN_value = 28.3dB then , cnvalue = Real_RF_level * 10 = 283
* Return Value: INT32
*****************************************************************************/

INT32 nim_s3281_dvbc_get_cn_value(struct nim_device *dev, UINT16 *cnvalue)
{
    UINT16 eq_mse = 0;
    UINT8 data[2] ={0};
    UINT8 qam_order = 0;
    UINT16 const_modules = 0;
    UINT16 log_cstmod_x100_add_100 = 0;//  100 * log10(const_modules) + 100
    UINT16 snr_offset = 25;
    UINT16 cnr = 0;

    if((NULL == dev) || (cnvalue == NULL))
    {
        return ERR_FAILUE;
    }
    nim_s3281_dvbc_read(NIM_S3202_SNR_MONI1, data, 2);
    eq_mse = (UINT16)(data[1] << 8 | data[0]);

    qam_order = s3281_cur_channel_info.modulation;
    switch (qam_order)
    {
    case 4:
        const_modules = 40960;
        log_cstmod_x100_add_100 = 561;
        break;
    case 5:
        const_modules = 20480;
        log_cstmod_x100_add_100 = 531;
        break;
    case 6:
        const_modules = 43008;
        log_cstmod_x100_add_100 = 563;
        break;
    case 7:
        const_modules = 20992;
        log_cstmod_x100_add_100 = 532;
        break;
    case 8:
        const_modules = 43520;
        log_cstmod_x100_add_100 = 564;
        break;
    default:
        const_modules = 43008;
        log_cstmod_x100_add_100 = 563;
        break;
    }

    cnr = log_cstmod_x100_add_100 - log10times100_l(eq_mse * 10) - snr_offset;

    *cnvalue = cnr;

    S3281_PRINTF("[%s]line=%d,cnvalue=%d!\n", __FUNCTION__, __LINE__, *cnvalue);

    return SUCCESS;

}

/*****************************************************************************
* INT32 nim_s3281_dvbc_get_per(struct nim_device *dev, UINT32 *rsubc)
* Reed Solomon Uncorrected block count
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3281_dvbc_get_per(struct nim_device *dev,UINT32 *rsubc)
{
    BOOL valid = FALSE;

    if((dev == NULL) || (rsubc == NULL))
    {
	    return ERR_FAILURE;
	}	
    nim_s3281_dvbc_monitor_berper(dev, &valid);

    *rsubc = PER_COUNTS;
    S3281_PRINTF("[%s]line=%d,PERValue=%d!\n", __FUNCTION__, __LINE__, (int)(*rsubc));
    return SUCCESS;
}

INT32 nim_s3281_dvbc_get_ber(struct nim_device *dev, UINT32 *err_count)
{
    BOOL valid = FALSE;

    if((dev == NULL) || (err_count == NULL))
	{
        return ERR_FAILURE;
	}	
    nim_s3281_dvbc_monitor_berper(dev, &valid);
    *err_count = BER_COUNTS;

    S3281_PRINTF("[%s]line=%d,BERValue=%d!\n", __FUNCTION__, __LINE__, (int)(*err_count));

    return SUCCESS;
}

static INT32 nim_get_qam_chr(UINT8 fec,UINT8 *char_qam,UINT32 len)
{
	if((char_qam == NULL) || (len<1))
    {
        S3281_PRINTF("[%s]line=%d,param error!\n", __FUNCTION__, __LINE__);
        return ERR_FAILUE;
    }

    switch(fec)
    {
    case 4:
        strncpy(char_qam, "16QAM", len-1);
        break;
    case 5:
        strncpy(char_qam, "32QAM", len-1);
        break;
    case 6:
        strncpy(char_qam , "64QAM", len-1);
        break;
    case 7:
        strncpy(char_qam, "128QAM", len-1);
        break;
    case 8:
        strncpy(char_qam , "256QAM", len-1);
        break;
    default:
        strncpy(char_qam, "NONE", len-1);
        break;
    }

    return SUCCESS;
}

static INT32 nim_get_lock_status(struct nim_device *dev,UINT8 fec)
{
    INT32  rtn = 0;
    UINT8  try_time = 0;
    UINT16 cons_time = 0;
    UINT8  data = 0;
    UINT8 qam = 6;

    if(dev == NULL)
    {
	    return ERR_FAILED;
	}	
    //step 6: check dem lock or not.
    try_time = 0;
    //Joey 20080504. According to Program guide 20080430. for the worst case lock time.
    cons_time = 40;

    while(1)
    {
        nim_s3281_dvbc_read(NIM_S3202_MONITOR1, &data, 1);
        if(NIM_S3281_ALL_LOCK == (data & NIM_S3281_ALL_LOCK))
        {
            // Add for unknown qam order. Trueve 090415
            if (fec == 0)
            {
                qam = 6;

                nim_s3281_dvbc_get_qam_order(dev, &qam);
                s3281_cur_channel_info.modulation = qam;

            }
            S3281_PRINTF("Demod Locked! Try times = %d!\n", try_time);
            rtn = SUCCESS;
            break;
        }

        if(try_time > cons_time) // If 200ms no lock , then retrun failed.
        {
            S3281_PRINTF("Demod Unlock! Try times = %d!\n", try_time);
            rtn = ERR_FAILED;
            break;
        }

        comm_sleep(10);
        try_time++;

    }

    return rtn;

}


static INT32 nim_get_tuner_lock_status(struct nim_s3281_private *dev_priv,UINT32 freq,UINT32 sym)
{
    UINT8 tuner_retry = 0;
    UINT8 lock = 0;

    if(dev_priv == NULL)
    {
	    return 0;
	}	
    do
    {
        if(tuner_retry > TUNER_RETRY_2)
        {
            TUNER_PRINTF("ERROR! Tuner Lock Fail\n");
            lock = 0;
            break;
        }
        tuner_retry++;
        // Fast config tuner
        if(dev_priv->nim_tuner_control(dev_priv->tuner_id, freq, sym) != SUCCESS)
        {
            TUNER_PRINTF("[%s]line=%d,Fast Config tuner failed !\n",__FUNCTION__,__LINE__);
        }

        // Read status
        if(dev_priv->nim_tuner_status(dev_priv->tuner_id, &lock) != SUCCESS)
        {
            lock = 0;
            TUNER_PRINTF("[%s]line=%d,ERROR! Tuner Read Status Fail\n",__FUNCTION__,__LINE__);
        }

        S3281_PRINTF("Tuner Lock Times=%d, *lock=%d !!\n", tuner_retry, lock);
    }
    while(0 == lock);

    return lock;
}

static INT32 nim_get_has_locked(struct nim_device *dev,UINT32 freq,UINT32 sym,UINT8 fec)
{
    UINT8 lockget = 0;

    if(dev == NULL)
    {
	    return ERR_FAILURE;
	}	
    nim_s3281_dvbc_get_lock(dev, &lockget);

    // Add a comparametion, when the the new parameter and the existed parameter is the same ,
    // then return success directly without set it to the tuner and demod.
    // For the request of Mark_Li 2007/11/07
    if ((((freq - s3281_cur_channel_info.frequency / DIVI_10) <= COMPARE_VALUE_40) || \
         ((s3281_cur_channel_info.frequency / DIVI_10 - freq) <= COMPARE_VALUE_40)) && \
         ((sym - s3281_cur_channel_info.symbol_rate <= COMPARE_VALUE_40) || \
     (s3281_cur_channel_info.symbol_rate - sym <= COMPARE_VALUE_40)) && \
         (s3281_cur_channel_info.modulation == fec) && (lockget == 1))
    {
        return SUCCESS;
    }
    return ERR_FAILED;
}

static void nim_amend_params(UINT32 *freq, UINT32 *sym, UINT8 *fec)
{
	// Just the input parameter's correction
    // If given an out bound data, then set it to an imposible state, to make it unlock. instead of return to ERR_FAILED
    // Because, if return a ERR_FAILED, the original set is still valid, and the demod is still 
    // in original state(eg. LOCKED)
    // even the new parameter will make it UNLOCK. so give it an imposible data to set it to realy UNLOCK is reasonable.
    // For the request of Robbin_Han 2007/12/19
    // fec == 0 means unknown qam order. Trueve 090415
#ifdef UNKNOWN_QAM_SUPPORT
    if(((*fec != 0) && ( *fec != QAM16 ) && ( *fec != QAM32 ) && ( *fec != QAM64 ) \
        && ( *fec != QAM128 ) && ( *fec != QAM256 ) ) \
        || ( (*sym < SYM_1000 ) || ( *sym  > SYM_7000) ) || ((*freq < FREQ_48000 ) || ( *freq > FREQ_859000)) )
#else
    if((( *fec != QAM16 ) && ( *fec != QAM32 ) && ( *fec != QAM64 ) && ( *fec != QAM128 ) \
        && ( *fec != QAM256 ) ) \
        || ( (*sym < SYM_1000 ) || ( *sym  > SYM_7000) ) || ((*freq < FREQ_48000 ) || ( *freq > FREQ_859000)) )
#endif
    {
        *fec = QAM16;
        *sym = 1000;
        *freq = 10000;
    }

}

/*****************************************************************************
* INT32 nim_s3281_dvbc_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);
* Description: S3202 channel change operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq                : Frequence
*  Parameter3: UINT32 sym                : Symbol rate
*  Parameter4: UINT8 fec                : Code rate
*
* Return Value: INT32
*****************************************************************************/
#if (QAM_FPGA_USAGE == SYS_FUNC_ON)
INT32 nim_s3281_dvbc_channel_change(struct nim_device *dev, NIM_CHANNEL_CHANGE_T *pstchl_change)
{
    UINT8 lock = 0;
    UINT8 tuner_retry = 0;
    UINT8 data = 0;
    UINT8 data1 = 0;
    UINT8 try_time = 0;
    UINT8 try_256qam = 0;
    struct nim_s3281_private *dev_priv = NULL;
    UINT32    freq = pstchl_change->freq;
    UINT32    sym = pstchl_change->sym;
    UINT8    fec   = pstchl_change->modulation;

    dev_priv = (struct nim_s3281_private *)dev->priv;
    freq *= 10;

    //joey 20080417. add in "+150Khz" according to register config by xian_chen.
    freq = freq + 150;

    // Just whther it is Catching data Mode
    nim_s3281_dvbc_read(NIM_S3202_FSM1, &data, 1);
    nim_s3281_dvbc_read(NIM_S3202_FSM11, &data1, 1);

    if((0x10 == (data & 0x10)) || (0x08 ==( data1 & 0x08)))
    {
        return SUCCESS;
    }
    else
    {

        //step 1: set receiver to IDLE status, reset Interrupt indicator
        S3281_PRINTF("%s,freq = %d, sym = %d, fec = %d \n", __FUNCTION__, freq, sym, fec);
        data = 0x80;
        nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);
        data = 0x00;
        nim_s3281_dvbc_write(NIM_S3202_INTERRUPT_EVENTS, &data, 1);

        // step3: set QAM_ORDER_KNOWN = Unknow
        do
        {
            if(tuner_retry > TUNER_RETRY_5)
            {
                S3281_PRINTF("ERROR! Tuner Lock Fail\n");
                lock = 0;
                return ERR_FAILUE;
            }

            tuner_retry++;
            // Fast config tuner  //, FAST_TIMECST_AGC, _1ST_I2C_CMD
            if(dev_priv->nim_tuner_control(dev_priv->tuner_id, freq, sym) == ERR_FAILUE)
            {
                S3281_PRINTF("Fast Config tuner failed step 1!\n");
            }

            if((dev_priv->tuner_config_ext.c_chip) == tuner_chip_infineon)
            {
				//, FAST_TIMECST_AGC, _2nd_i2c_cmd
                if(dev_priv->nim_tuner_control(dev_priv->tuner_id, freq, sym) == ERR_FAILUE)
                {
                    S3281_PRINTF("Fast Config tuner failed step 2!\n");
                }
            }
            // Slow config tuner  //, SLOW_TIMECST_AGC, _1ST_I2C_CMD
            if(dev_priv->nim_tuner_control(dev_priv->tuner_id, freq, sym) == ERR_FAILUE)
            {
                S3281_PRINTF("Slow Config tuner failed step 1!\n");
            }

            if((dev_priv->tuner_config_ext.c_chip) == tuner_chip_infineon)
            {
				//, SLOW_TIMECST_AGC, _2nd_i2c_cmd  
                if(dev_priv->nim_tuner_control(dev_priv->tuner_id, freq, sym) == ERR_FAILUE)
                {
                    S3281_PRINTF("Slow Config tuner failed step 2!\n");
                }
            }
            // Read status
            if(dev_priv->nim_tuner_status(dev_priv->tuner_id, &lock) == ERR_FAILUE)
            {
                S3281_PRINTF("ERROR! Tuner Read Status Fail\n");
            }

            S3281_PRINTF("Tuner Lock Times=0x%d,*lock=0x%d !!\n", tuner_retry, lock);

        }
        while(0 == lock);

        //joey 20080417 add in rs swap according to register update by xian_chen.
        //step 3: set symbol rate and symbol rate sweep ranger
        nim_set_rs_and_range(sym);

        //step 5:start capture.
        data = 0x40;
        nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);

        s3281_cur_channel_info.frequency = freq;
        s3281_cur_channel_info.symbol_rate = sym;
        s3281_cur_channel_info.modulation = fec;

        //step 6: check dem lock or not.
        try_time = 0;
        while(1)
        {
            nim_s3281_dvbc_read(NIM_S3202_MONITOR1, &data, 1);
            if(NIM_S3281_ALL_LOCK == (data & NIM_S3281_ALL_LOCK))
            {
                return SUCCESS;
            }

            comm_sleep(20);

            try_time++;
            if(try_time > LOCK_TRY_TIMES_50)
            {
                return ERR_FAILED;
            }
        }
    }
}

#else
INT32 nim_s3281_dvbc_channel_change(struct nim_device *dev,NIM_CHANNEL_CHANGE_T *pstchl_change)
{
    UINT8 qam_mod = 0;

    if((dev == NULL) || (pstchl_change == NULL))
    {
	    return ERR_FAILURE;
	}	
    qam_mod = ((struct nim_s3281_private *)dev->priv)->qam_mode;
    S3281_PRINTF( " Enter channel change and QAM_mode is %d \n", qam_mod);

    if (NIM_DVBC_J83B_MODE == (qam_mod & 0x01))
    {
        nim_s3281_j83b_channel_change( dev,  pstchl_change);
    }
    else
    {
        nim_s3281_j83ac_channel_change( dev,  pstchl_change);
    }
    return SUCCESS;
}

static INT32 nim_s3281_j83ac_channel_change(struct nim_device *dev,NIM_CHANNEL_CHANGE_T *pstchl_change)
{
    struct nim_s3281_private *dev_priv = NULL;
    INT32 rtn = 0;
    UINT8 lock = 0;
    UINT8 data = 0;
    UINT8 data1 = 0;
    UINT32 freq = 0;
	UINT32 sym = 0;
	UINT8  fec = 0;
    UINT8  char_qam[10]={0};

    if((dev == NULL) || (pstchl_change == NULL))
	{
        return ERR_FAILURE;
	}	
    freq = pstchl_change->freq;
    sym = pstchl_change->sym;
    fec = pstchl_change->modulation;

    lock=nim_get_has_locked(dev,freq,sym,fec);
    if(lock==SUCCESS)
    {
        return SUCCESS;
    }

    //Up_layer send freq para is not "KHz", should be multipier 10 time to "KHz".
    freq = freq * 10;

    //Patch for S3202
    nim_set_pro_path_gain(sym,fec);

    // Just whther it is Catching data Mode
    nim_s3281_dvbc_read(NIM_S3202_FSM1, &data, 1);
    nim_s3281_dvbc_read(NIM_S3202_FSM11, &data1, 1);
    if((0x10 == (data & 0x10)) || (0x08 == ( data1 & 0x08)))
    {
        return SUCCESS;
    }

    channel_change_en = TRUE;

	nim_get_qam_chr(fec,char_qam,sizeof(char_qam));
    S3281_PRINTF("%s,freq = %d, sym = %d, fec = %s \n", __FUNCTION__, (int)freq, (int)sym, char_qam);

    nim_amend_params( &freq, &sym, &fec);

    dev_priv = (struct nim_s3281_private *)dev->priv;

    //step 1: set receiver to IDLE status, reset Interrupt indicator, and set WORK_MODE.
    data = 0x80;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);

    data = 0x00;
    nim_s3281_dvbc_write(NIM_S3202_INTERRUPT_EVENTS, &data, 1);
    data = 0x0f;
    nim_s3281_dvbc_write(NIM_S3202_INTERRUPT_MASK, &data, 1);

    // modified by magic ( move from 2590 to here , to protect the agc0a patch)
    s3281_cur_channel_info.frequency = freq;
    s3281_cur_channel_info.symbol_rate = sym;
    s3281_cur_channel_info.modulation = fec;

#if 1
    // step 2: set tuner frequency
    //because I2c for  confige tuner use by-pass mode of demodulator. enable the function first.
#ifdef    I2C_BYPASS
    nim_s3281_dvbc_read(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
    data |= 0x01;
    nim_s3281_dvbc_write(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
#endif
    lock=nim_get_tuner_lock_status(dev_priv,freq,sym);

    //because I2c for  confige tuner use by-pass mode of dimodulator. disable the function after config.
#ifdef    I2C_BYPASS
    nim_s3281_dvbc_read(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
    data &= 0xfe;
    nim_s3281_dvbc_write(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
#endif
    //if (lock == 0)        return ERR_FAILUE;
    if (lock == 0)
    {
        TUNER_PRINTF("tuner unlock...\n");// sometime TCL tuner will lock failure, I don't know why ^_^
     }

#endif // do not control tuner

    //step 3: set symbol rate and symbol rate sweep ranger
    nim_set_rs_and_range(sym);

    //step 4: set delat frequency and delata frequency sweep range
    //set delta freq and sweep range.//unit is KHz.( range from  -4096 to  +4095 KHz)
    //joey 20080418. update according to register config xian_chen.
    //joey 20080422. add init_foffset;
    nim_s3281_dvbc_set_delfreq(init_foffset);
    //sweep range is "0" means disable sweep.//unit is KHz. +/- range is include.
    nim_s3281_dvbc_set_search_freq(1000);

#if 0  //20130107 russell modified for kvn dvb-mc issue.
    if(FEC_VALUE_4 == fec)
    {
        data = 0x06;
        nim_s3281_dvbc_write(NIM_S3202_CR_TIME_OUT_FOR_DRT_0, &data, 1);
        data = 0x0f;
        nim_s3281_dvbc_write(NIM_S3202_CR_INT_PATH_GAIN_0, &data, 1);
    }
    else
    {
        data = 0x09;
        nim_s3281_dvbc_write(NIM_S3202_CR_TIME_OUT_FOR_DRT_0, &data, 1);
        data = 0x11;
        nim_s3281_dvbc_write(NIM_S3202_CR_INT_PATH_GAIN_0, &data, 1);
    }
#endif

    //step 5: set QAM_OREDER
#ifdef UNKNOWN_QAM_SUPPORT
    // Add for unknown qam order, set default 64-QAM. Trueve 090415
    if (fec == 0)
    {
        nim_s3281_dvbc_set_qam_order(NIM_S3202_QAM_ORDER_UNKNOWN, 6);
    }
    else
#endif
     {
        nim_s3281_dvbc_set_qam_order(NIM_S3202_QAM_ORDER_KNOWN, fec);
     }


#ifdef PHASENOISE_DEF_J83AC
//start for QAM3.3 phasenoise improve by grace
	data = 0x12; // CR_P_GAIN_6,Reg[0xcf]
	nim_s3281_dvbc_write(NIM_S3202_CR_INT_PATH_GAIN_0,&data,1); 

	data = 0x07; // CR_I_GAIN_6,Reg[0xd8]
	nim_s3281_dvbc_write(NIM_S3202_CR_TIME_OUT_FOR_DRT_0,&data,1); 
	
	data = 0x56; // CR_THRED_LOCK_64_4,Reg[0xb8]
	nim_s3281_dvbc_write(NIM_S3202_CR_LOCK_THRD_26,&data,1);	

	data = 0x56; // CR_THRED_LOCK_256_4,Reg[0xc6]
	nim_s3281_dvbc_write(NIM_S3202_CR_LOCK_THRD_40,&data,1);

	nim_s3281_dvbc_read( NIM_S3202_EQ1, &data, 1);//FFF order:3  FBF order:3	
	data = data | 0x33;	
	nim_s3281_dvbc_write( NIM_S3202_EQ1, &data, 1);

	nim_s3281_dvbc_read( NIM_S3202_EQ_COEF3, &data, 1);//FFF_INI_POS_AWGN:5, reg_13c[5:4]reg_65[3:2]	
	data = data & 0xf3;	
	data = data | 0x04;	
	nim_s3281_dvbc_write( NIM_S3202_EQ_COEF3, &data, 1);

	nim_s3281_dvbc_read( 0x13c, &data, 1);
	data = data & 0xcf;	
	data = data | 0x10;	
	nim_s3281_dvbc_write( 0x13c, &data, 1);	
//end for QAM3.3 phasenoise improve by grace
#endif








    //clear all interupt.
    data = 0x00;
    nim_s3281_dvbc_write(NIM_S3202_INTERRUPT_EVENTS, &data, 1);

    //step 6: start capture.
    if ((sys_ic_get_rev_id() == IC_REV_1) && (rf_agc_en == TRUE))
    {
        data = if_def_val2;
        nim_s3281_dvbc_write(0x12, &data, 1);
    }

    // pretect for agc0a patch
    nim_s3281_dvbc_read(NIM_S3202_AGC1, &data, 1);
    if ((dev_priv->tuner_config_data.AGC_REF) != data)
    {
        data = (dev_priv->tuner_config_data.AGC_REF);
        nim_s3281_dvbc_write(NIM_S3202_AGC1, &data, 1);
    }

    //patch for 64/256 diffirent IF_min patch.
    if (dev_priv->tuner_config_ext.c_chip == TUNER_CHIP_CD1616LF_GIH)
    {
        if ((QAM256 == fec) && ( 0 == if_agc_min_ch))
        {
            data = (dev_priv->tuner_config_data.IF_AGC_MIN) - 0x0c;//0x7a=0x86-0x0c.
            nim_s3281_dvbc_write(0x12, &data, 1);
            if_agc_min_ch = 1;
        }
        else if ((QAM256 != fec) && (1 ==if_agc_min_ch))
        {
            data = (dev_priv->tuner_config_data.IF_AGC_MIN);
            nim_s3281_dvbc_write(0x12, &data, 1);
            if_agc_min_ch = 0;
        }
    }

    data = 0x40;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);

    // Seen add to start PER/BER sta
    data = 0x80;
    nim_s3281_dvbc_write( NIM_S3281_BERPER_RPT6_R43E, &data, 1);

    /* Modify for meeting minutes @ 2010-02-09 for fast cc */
#ifdef UNKNOWN_QAM_SUPPORT
    rtn=nim_get_lock_status(dev,fec);
#endif

    channel_change_en = FALSE;
    S3281_PRINTF("[%s] line = %d end!\n", __FUNCTION__, __LINE__);

    return rtn;
}


static INT32 nim_s3281_j83b_channel_change(struct nim_device *dev,NIM_CHANNEL_CHANGE_T *pstchl_change)
{
    struct nim_s3281_private *dev_priv =NULL;
    INT32 rtn = 0;
    UINT8 lock = 0;
    UINT8 data = 0;
    UINT8 data1 = 0;
    UINT8  char_qam[10]={0};
    UINT32 freq = 0;
    UINT32 sym = 0;
    UINT8  fec   = 0;
	
    if((dev == NULL) || (pstchl_change == NULL))
	{
        return ERR_FAILURE;
	}	
    freq = pstchl_change->freq;
    sym = pstchl_change->sym;
    fec   = pstchl_change->modulation;

    lock=nim_get_has_locked(dev,freq,sym,fec);
    if(lock==SUCCESS)
    {
        return SUCCESS;
    }

    //Up_layer send freq para is not "KHz", should be multipier 10 time to "KHz".
    freq = freq * 10;

    //Patch for S3202
    nim_set_pro_path_gain(sym,fec);

    // Just whther it is Catching data Mode
    nim_s3281_dvbc_read(NIM_S3202_FSM1, &data, 1);
    nim_s3281_dvbc_read(NIM_S3202_FSM11, &data1, 1);
    if((0x10 == (data & 0x10)) || (0x08 == ( data1 & 0x08)))
    {
        return SUCCESS;
    }

    channel_change_en = TRUE;

    nim_get_qam_chr(fec,char_qam,sizeof(char_qam));
    S3281_PRINTF("%s,freq = %d, sym = %d, fec = %s \n", __FUNCTION__, (int)freq, (int)sym, char_qam);

	nim_amend_params( &freq, &sym, &fec);
	
    dev_priv = (struct nim_s3281_private *)dev->priv;

    //step 1: set receiver to IDLE status, reset Interrupt indicator, and set WORK_MODE.
    data = 0x80;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);
    data = 0x00;
    nim_s3281_dvbc_write(NIM_S3202_INTERRUPT_EVENTS, &data, 1);
    data = 0x0f;
    nim_s3281_dvbc_write(NIM_S3202_INTERRUPT_MASK, &data, 1);

    // modified by magic ( move from 2590 to here , to protect the agc0a patch)
    s3281_cur_channel_info.frequency = freq;
    s3281_cur_channel_info.symbol_rate = sym;
    s3281_cur_channel_info.modulation = fec;

    // step 2: set tuner frequency
    //because I2c for  confige tuner use by-pass mode of demodulator. enable the function first.
#ifdef    I2C_BYPASS
    nim_s3281_dvbc_read(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
    data |= 0x01;
    nim_s3281_dvbc_write(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
#endif

    lock=nim_get_tuner_lock_status(dev_priv,freq,sym);

    //because I2c for  confige tuner use by-pass mode of dimodulator. disable the function after config.
#ifdef    I2C_BYPASS
    nim_s3281_dvbc_read(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
    data &= 0xfe;
    nim_s3281_dvbc_write(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
#endif
    if (lock == 0)
    {
        TUNER_PRINTF("tuner unlock...\n");// sometime TCL tuner will lock failure, I don't know why ^_^
    }

    //step 3: set symbol rate and symbol rate sweep ranger
    nim_set_rs_and_range(sym);

    //step 4: set delat frequency and delata frequency sweep range
    //set delta freq and sweep range.//unit is KHz.( range from  -4096 to  +4095 KHz)
    nim_s3281_dvbc_set_delfreq(init_foffset);
    //sweep range is "0" means disable sweep.//unit is KHz. +/- range is include.
    nim_s3281_dvbc_set_search_freq(1000);

    //step 5: set QAM_OREDER
#ifdef UNKNOWN_QAM_SUPPORT
    // Add for unknown qam order, set default 64-QAM. Trueve 090415
    if (fec == 0)
    {
        nim_s3281_dvbc_set_qam_order(NIM_S3202_QAM_ORDER_UNKNOWN, 6);
    }
    else
#endif
    {
        nim_s3281_dvbc_set_qam_order(NIM_S3202_QAM_ORDER_KNOWN, fec);
    }
    if(FEC_VALUE_8 == fec)
    {
        //set the register for new CR in case J83B 256QAM
        data1 = 0xfa;    //    WriteMemB(0x18003113,0xfa);  // enable new CR
        nim_s3281_dvbc_write( 0x113, &data1, 1);

        data1 = 0x0e;    //    WriteMemB(0x1800311e,0x0e); // EQ slow thr
        nim_s3281_dvbc_write( 0x11e, &data1, 1);

        data1 = 0x00;//    WriteMemB(0x180030d8,0x00); // CR track I GAIN
        nim_s3281_dvbc_write( NIM_S3202_CR_TIME_OUT_FOR_DRT_0, &data1, 1);

        nim_s3281_dvbc_read( 0x11f, &data1, 1); //rd_byte = ReadMemB(0x1800311f); // disable PN_SLOW in all cases
        data1 &= 0xcf;
        nim_s3281_dvbc_write( 0x11f, &data1, 1); //    WriteMemB(0x1800311f, rd_byte);

        //    reg115[5:4] = 1; // CENTERAL_CONSTE_STEP
        nim_s3281_dvbc_read( 0x115, &data1, 1);
        data1 &= 0xdf;
        data1 |=    0x10;
        nim_s3281_dvbc_write( 0x115, &data1, 1);
    }
    else
    {
        //set the register for new CR
        data1 = 0x00;    //    WriteMemB(0x18003113,0x00);  disable new CR
        nim_s3281_dvbc_write( 0x113, &data1, 1);

        data1 = 0x08;//    WriteMemB(0x180030d8,0x08);  CR track I GAIN(default value)
        nim_s3281_dvbc_write( NIM_S3202_CR_TIME_OUT_FOR_DRT_0, &data1, 1);

        nim_s3281_dvbc_read( 0x11f, &data1, 1); //rd_byte = ReadMemB(0x1800311f); // disable PN_SLOW in all cases
        data1 &= 0xcf;
        nim_s3281_dvbc_write( 0x11f, &data1, 1); //    WriteMemB(0x1800311f, rd_byte);

        //reg115[5:4] = 0; // CENTERAL_CONSTE_STEP
        nim_s3281_dvbc_read( 0x115, &data1, 1);
        data1 &= 0xcf;
        nim_s3281_dvbc_write( 0x115, &data1, 1);
    }

#ifdef PHASENOISE_DEF_J83B
//start for QAM3.3 phasenoise improve by grace
	data = 0x12; // CR_P_GAIN_6,Reg[0xcf]
	nim_s3281_dvbc_write(NIM_S3202_CR_INT_PATH_GAIN_0,&data,1); 
	
	if(fec == 6)
	{
		data = 0x07; // CR_I_GAIN_6,Reg[0xd8]
		nim_s3281_dvbc_write(NIM_S3202_CR_TIME_OUT_FOR_DRT_0,&data,1); 
	}
		
	data = 0x56; // CR_THRED_LOCK_64_4,Reg[0xb8]
	nim_s3281_dvbc_write(NIM_S3202_CR_LOCK_THRD_26,&data,1);	
	
	data = 0x56; // CR_THRED_LOCK_256_4,Reg[0xc6]
	nim_s3281_dvbc_write(NIM_S3202_CR_LOCK_THRD_40,&data,1);
	
	nim_s3281_dvbc_read( NIM_S3202_EQ1, &data, 1);//FFF order:3  FBF order:3	
	data = data | 0x33; 
	nim_s3281_dvbc_write( NIM_S3202_EQ1, &data, 1);
	
	nim_s3281_dvbc_read( NIM_S3202_EQ_COEF3, &data, 1);//FFF_INI_POS_AWGN:5, reg_13c[5:4]reg_65[3:2]	
	data = data & 0xf3; 
	data = data | 0x04; 
	nim_s3281_dvbc_write( NIM_S3202_EQ_COEF3, &data, 1);
	
	nim_s3281_dvbc_read( 0x13c, &data, 1);
	data = data & 0xcf; 
	data = data | 0x10; 
	nim_s3281_dvbc_write( 0x13c, &data, 1); 
//end for QAM3.3 phasenoise improve by grace
#endif


    //clear all interupt.
    data = 0x00;
    nim_s3281_dvbc_write(NIM_S3202_INTERRUPT_EVENTS, &data, 1);

    //step 6: start capture.
    if ((sys_ic_get_rev_id() == IC_REV_1) && (rf_agc_en == TRUE))
    {
        data = if_def_val2;
        nim_s3281_dvbc_write(0x12, &data, 1);
    }

    // pretect for agc0a patch
    nim_s3281_dvbc_read(NIM_S3202_AGC1, &data, 1);
    if ((dev_priv->tuner_config_data.AGC_REF) != data)
    {
        data = (dev_priv->tuner_config_data.AGC_REF);
        nim_s3281_dvbc_write(NIM_S3202_AGC1, &data, 1);
    }

    //patch for 64/256 diffirent IF_min patch.
    if (dev_priv->tuner_config_ext.c_chip == TUNER_CHIP_CD1616LF_GIH)
    {
        if ((QAM256 == fec) && (0 == if_agc_min_ch))
        {
            data = (dev_priv->tuner_config_data.IF_AGC_MIN) - 0x0c;//0x7a=0x86-0x0c.
            nim_s3281_dvbc_write(0x12, &data, 1);
            if_agc_min_ch = 1;
        }
        else if ((QAM256 != fec) && ( 1 == if_agc_min_ch))
        {
            data = (dev_priv->tuner_config_data.IF_AGC_MIN);
            nim_s3281_dvbc_write(0x12, &data, 1);
            if_agc_min_ch = 0;
        }
    }
    data = 0x40;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);

    // Seen add to start PER/BER sta
    data = 0x80;
    nim_s3281_dvbc_write( NIM_S3281_BERPER_RPT6_R43E, &data, 1);
    //
    /* Modify for meeting minutes @ 2010-02-09 for fast cc */
#ifdef UNKNOWN_QAM_SUPPORT
    rtn=nim_get_lock_status(dev,fec);
#endif
    channel_change_en = FALSE;

    return rtn;
}

#endif

INT32 nim_s3281_dvbc_quick_channel_change(struct nim_device *dev,NIM_CHANNEL_CHANGE_T *pstchl_change)
{
    struct nim_s3281_private *dev_priv =NULL;
    INT32 rtn = SUCCESS;
    UINT8 lock = 0;
    UINT8 data = 0;
    UINT8 data1 = 0;
    UINT32 freq = 0;
    UINT32 sym = 0;
    UINT8  fec = 0;

    UINT8  char_qam[10]={0};
	
    if((dev == NULL) || (pstchl_change == NULL))
    {
	    return ERR_FAILURE;
	}	
    freq = pstchl_change->freq;
    sym = pstchl_change->sym;
    fec   = pstchl_change->modulation;

    lock=nim_get_has_locked(dev,freq,sym,fec);
    if(lock==SUCCESS)
    {
        return SUCCESS;
    }

    //Up_layer send freq para is not "KHz", should be multipier 10 time to "KHz".
    freq = freq * 10;
    freq = freq + init_foffset;

    //Patch for S3202
    nim_set_pro_path_gain(sym,fec);

    // Just whther it is Catching data Mode
    nim_s3281_dvbc_read(NIM_S3202_FSM1, &data, 1);
    nim_s3281_dvbc_read(NIM_S3202_FSM11, &data1, 1);

    if((0x10 ==(data & 0x10)) || (0x08 == ( data1 & 0x08)))
    {
        return SUCCESS;
    }

    channel_change_en = TRUE;

    // To watch the input parameter
    nim_get_qam_chr(fec,char_qam,sizeof(char_qam));
    S3281_PRINTF("%s,freq = %d, sym = %d, fec = %s \n", __FUNCTION__, (int)freq, (int)sym, char_qam);

	nim_amend_params( &freq, &sym, &fec);

    dev_priv = (struct nim_s3281_private *)dev->priv;

    //step 1: set receiver to IDLE status, reset Interrupt indicator, and set WORK_MODE.
    data = 0x80;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);

    data = 0x00;
    nim_s3281_dvbc_write(NIM_S3202_INTERRUPT_EVENTS, &data, 1);
    data = 0x0f;
    nim_s3281_dvbc_write(NIM_S3202_INTERRUPT_MASK, &data, 1);

    // modified by magic ( move from 2590 to here , to protect the agc0a patch)
    s3281_cur_channel_info.frequency = freq;
    s3281_cur_channel_info.symbol_rate = sym;
    s3281_cur_channel_info.modulation = fec;

#if 1
    // step 2: set tuner frequency
    //because I2c for  confige tuner use by-pass mode of demodulator. enable the function first.
#ifdef    I2C_BYPASS
    nim_s3281_dvbc_read(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
    data |= 0x01;
    nim_s3281_dvbc_write(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
#endif

    lock=nim_get_tuner_lock_status(dev_priv,freq,sym);

    //because I2c for  confige tuner use by-pass mode of dimodulator. disable the function after config.
#ifdef    I2C_BYPASS
    nim_s3281_dvbc_read(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
    data &= 0xfe;
    nim_s3281_dvbc_write(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
#endif
    //if (lock == 0)        return ERR_FAILUE;
    if (lock == 0)
    {
        TUNER_PRINTF("tuner unlock...\n");// sometime TCL tuner will lock failure, I don't know why ^_^
    }

#endif // do not control tuner

    //step 3: set symbol rate and symbol rate sweep ranger
    nim_set_rs_and_range(sym);

    //step 4: set delat frequency and delata frequency sweep range
    //set delta freq and sweep range.//unit is KHz.( range from  -4096 to  +4095 KHz)
    nim_s3281_dvbc_set_delfreq(init_foffset);
    //sweep range is "0" means disable sweep.//unit is KHz. +/- range is include.
    nim_s3281_dvbc_set_search_freq(1000);

    //step 5: set QAM_OREDER
#ifdef UNKNOWN_QAM_SUPPORT
    // Add for unknown qam order, set default 64-QAM. Trueve 090415
    if (fec == 0)
    {
        nim_s3281_dvbc_set_qam_order(NIM_S3202_QAM_ORDER_UNKNOWN, 6);
    }
    else
#endif
    {
        nim_s3281_dvbc_set_qam_order(NIM_S3202_QAM_ORDER_KNOWN, fec);
     }
    //clear all interupt.
    data = 0x00;
    nim_s3281_dvbc_write(NIM_S3202_INTERRUPT_EVENTS, &data, 1);

    //step 6: start capture.
    // For agc patch
    if ((sys_ic_get_rev_id() == IC_REV_1) && (rf_agc_en == TRUE))
    {
        data = if_def_val2;
        nim_s3281_dvbc_write(0x12, &data, 1);
    }

    // pretect for agc0a patch
    nim_s3281_dvbc_read(NIM_S3202_AGC1, &data, 1);
    if ((dev_priv->tuner_config_data.AGC_REF) != data)
    {
        data = (dev_priv->tuner_config_data.AGC_REF);
        nim_s3281_dvbc_write(NIM_S3202_AGC1, &data, 1);
    }

    //patch for 64/256 diffirent IF_min patch.
    if (dev_priv->tuner_config_ext.c_chip == TUNER_CHIP_CD1616LF_GIH)
    {
        if ((QAM256 == fec) && ( 0 == if_agc_min_ch ))
        {
            data = (dev_priv->tuner_config_data.IF_AGC_MIN) - 0x0c;//0x7a=0x86-0x0c.
            nim_s3281_dvbc_write(0x12, &data, 1);
            if_agc_min_ch = 1;
        }
        else if ((QAM256 != fec) && (1 == if_agc_min_ch ))
        {
            data = (dev_priv->tuner_config_data.IF_AGC_MIN);
            nim_s3281_dvbc_write(0x12, &data, 1);
            if_agc_min_ch = 0;
        }
    }

    data = 0x40;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);

    channel_change_en = FALSE;

    return rtn;
}


INT32 nim_s3281_dvbc_set_mode(struct nim_device *dev,UINT8 tunerid_input,
                              struct DEMOD_CONFIG_ADVANCED qam_config)
{
    struct nim_s3281_private *priv_mem = NULL;
    struct QAM_TUNER_CONFIG_EXT tunerconfigext;
    UINT32 dword = 0x00;
    UINT8 data = 0x00;
    UINT32 data_clock = 0;

#if defined(__NIM_LINUX_PLATFORM__)
	UINT32 data_long = 0x00;
    extern unsigned long __G_ALI_MM_NIM_J83B_MEM_START_ADDR;
    extern unsigned long __G_ALI_MM_NIM_J83B_MEM_SIZE;
	
	qam_config.qam_buffer_addr = __G_ALI_MM_NIM_J83B_MEM_START_ADDR;
	qam_config.qam_buffer_len = __G_ALI_MM_NIM_J83B_MEM_SIZE;
#endif
	
    S3281_PRINTF("[%s]line=%d,enter!\n", __FUNCTION__, __LINE__);

    if(dev == NULL)
    {
	    return ERR_FAILURE;
	}	
    priv_mem = (struct nim_s3281_private *)dev->priv;
	
    comm_memset(&tunerconfigext,0,sizeof(struct QAM_TUNER_CONFIG_EXT));
    //set config data
    priv_mem->qam_mode = qam_config.qam_config_advanced;
	
    priv_mem->qam_buffer_addr = qam_config.qam_buffer_addr;
    priv_mem->qam_buffer_len = qam_config.qam_buffer_len;

    //kent,for debug
   // priv_mem->tuner_config_ext.w_tuner_if_freq_j83a = 36000;
	//priv_mem->qam_mode = 1;
		
    if (NIM_DVBC_J83B_MODE == (priv_mem->qam_mode & 0x01) )
    {
        
        priv_mem->tuner_config_ext.c_tuner_reopen = 0x0C << 8 | 0x01;
        //set the buffer addr for deinterleave
        if(priv_mem->qam_buffer_len > MAGIC_CONST_0)
        {
	    dword = ((priv_mem->qam_buffer_addr) & 0xFFFFFFF) >> 2; //phisical addr
        }
        else
        {
	    dword = 0x00;
        }

        nim_s3281_dvbc_write(NIM_S3281_DEINT_BASEADDR0_R428, (UINT8 *)(&dword), 4);
        S3281_PRINTF("[%s]line=%d,J83B,w_tuner_if_freq=%d!\n", __FUNCTION__, __LINE__, 
	             priv_mem->tuner_config_ext.w_tuner_if_freq);

#if defined(__NIM_LINUX_PLATFORM__)
        data_long = NIM_S3281_GET_DWORD(S3281_SOC_BASE_ADDR+0x6c);
        data_long = data_long & 0xFFE7FFFF;
        NIM_S3281_SET_DWORD((S3281_SOC_BASE_ADDR+0x6c),data_long);
#endif

    }
    else
    {

        priv_mem->tuner_config_ext.c_tuner_reopen = 0x09 << 8 | 0x01;
        if(0x01 == priv_mem->tuner_config_ext.w_tuner_if_j83ac_type )
        {
            priv_mem->tuner_config_ext.w_tuner_if_freq = priv_mem->tuner_config_ext.w_tuner_if_freq_j83c;
            priv_mem->tuner_config_ext.c_tuner_reopen = 0x0C << 8 | 0x01;
        }
        dword = 0x00;
        nim_s3281_dvbc_write(NIM_S3281_DEINT_BASEADDR0_R428, (UINT8 *)(&dword), 4);
        S3281_PRINTF("[%s]line=%d,J83AC,w_tuner_if_freq=%d!\n", __FUNCTION__, __LINE__, 
	              priv_mem->tuner_config_ext.w_tuner_if_freq);

#if defined(__NIM_LINUX_PLATFORM__)
        data_long =  NIM_S3281_GET_DWORD(S3281_SOC_BASE_ADDR+0x6c); 
        data_long = data_long & 0xFFE7FFFF;
        NIM_S3281_SET_DWORD((S3281_SOC_BASE_ADDR+0x6c),data_long | 0x00080000);
#endif

    }

    if(tunerid_input == 0)
    {
        priv_mem->tuner_config_ext.c_tuner_reopen = 0x0;
    }
    else
    {
        priv_mem->tuner_config_ext.c_tuner_reopen = 0x1;
    }

    if(priv_mem->nim_tuner_init != NULL)
    {
        if(priv_mem->nim_tuner_init(&(priv_mem->tuner_id), &(priv_mem->tuner_config_ext)) != SUCCESS)
        {
            S3281_PRINTF("Error: Init Tuner Failure!\n");
            return ERR_NO_DEV;
        }
    }

    tunerconfigext = priv_mem->tuner_config_ext;
    // ini freq offset for downmixer set
    if (tunerconfigext.w_tuner_if_freq <= FREQ_10000)
    {
#if defined(__NIM_LINUX_PLATFORM__)
        if (NIM_SAMPLE_CLK_27M == (priv_mem->qam_mode & 0x02))
        {
            dword = NIM_S3281_GET_DWORD(S3281_SOC_BASE_ADDR+0xB4);
            dword |= (1 << 12); //27MHz CLK
            NIM_S3281_SET_DWORD((S3281_SOC_BASE_ADDR+0xB4),dword);
			S3281_PRINTF("[%s]line=%d,here!\n", __FUNCTION__, __LINE__);
        }
        else if (NIM_SAMPLE_CLK_54M == (priv_mem->qam_mode & 0x02))
        {
            dword = NIM_S3281_GET_DWORD(S3281_SOC_BASE_ADDR+0xB4);
            dword &= (~(1 << 12));
            NIM_S3281_SET_DWORD((S3281_SOC_BASE_ADDR+0xB4),dword);
			S3281_PRINTF("[%s]line=%d,here!\n", __FUNCTION__, __LINE__);
        }
#endif

#if(SYS_TUNER_MODULE == nt220x)
        tunerconfigext.w_tuner_if_freq = 4500; //IF 4.5MHz
#endif
        data = (UINT8)(0xff & tunerconfigext.w_tuner_if_freq);
        nim_s3281_dvbc_write(INI_DM_FREQ_OFFSET_0, &data, 1);
        data = (UINT8)(0xff & ((tunerconfigext.w_tuner_if_freq) >> 8));
        nim_s3281_dvbc_write(INI_DM_FREQ_OFFSET_1, &data, 1);
        S3281_PRINTF("[%s]line=%d,w_tuner_if_freq=%d!\n", __FUNCTION__, __LINE__, tunerconfigext.w_tuner_if_freq);
        init_foffset = 0;
    }
	else
	{
		data_clock = priv_mem->qam_mode & 0x02;
		switch(data_clock)
		{
            case NIM_SAMPLE_CLK_27M:
			{

#if defined(__NIM_LINUX_PLATFORM__)
		        dword = NIM_S3281_GET_DWORD(S3281_SOC_BASE_ADDR+0xB4);
		        dword |= (1 << 12);
		        NIM_S3281_SET_DWORD((S3281_SOC_BASE_ADDR+0xB4),dword);
#endif
		        S3281_PRINTF("[%s]line=%d,w_tuner_if_freq=%d!\n", __FUNCTION__, __LINE__, tunerconfigext.w_tuner_if_freq);
		        if(FREQ_36000 == tunerconfigext.w_tuner_if_freq )
		        {
		            data = (UINT8)(0xff & (INT32)9000 );
		            nim_s3281_dvbc_write(INI_DM_FREQ_OFFSET_0, &data, 1);
		            data = (UINT8)(0xff & ((INT32)9000 >> 8));
		            nim_s3281_dvbc_write(INI_DM_FREQ_OFFSET_1, &data, 1);
		        }
		        else if(MAGIC_CONST_44000 == tunerconfigext.w_tuner_if_freq )
		        {
		            data = (UINT8)(0xff & (INT32)(-10000));
		            nim_s3281_dvbc_write(INI_DM_FREQ_OFFSET_0, &data, 1);
		            data = (UINT8)(0xff & ((INT32)(-10000) >> 8));
		            nim_s3281_dvbc_write(INI_DM_FREQ_OFFSET_1, &data, 1);
		        }
		        init_foffset = 150;
      		}
			break;
            case NIM_SAMPLE_CLK_54M:
            {

#if defined(__NIM_LINUX_PLATFORM__)
		        dword = NIM_S3281_GET_DWORD(S3281_SOC_BASE_ADDR+0xB4);
		        dword &= (~(1 << 12));
		        NIM_S3281_SET_DWORD((S3281_SOC_BASE_ADDR+0xB4),dword);
#endif
		        S3281_PRINTF("[%s]line=%d,w_tuner_if_freq=%d!\n", __FUNCTION__, __LINE__, tunerconfigext.w_tuner_if_freq);
		        if( FREQ_36000 ==tunerconfigext.w_tuner_if_freq )
		        {
		            data = (UINT8)(0xff & (INT32)(-18000));
		            nim_s3281_dvbc_write(INI_DM_FREQ_OFFSET_0, &data, 1);
		            data = (UINT8)(0xff & ((INT32)(-18000) >> 8));
		            nim_s3281_dvbc_write(INI_DM_FREQ_OFFSET_1, &data, 1);
		        }
		        else if( tunerconfigext.w_tuner_if_freq == MAGIC_CONST_44000 )
		        {
		            data = (UINT8)(0xff & (INT32)(-10000));
		            nim_s3281_dvbc_write(INI_DM_FREQ_OFFSET_0, &data, 1);
		            data = (UINT8)(0xff & ((INT32)(-10000) >> 8));
		            nim_s3281_dvbc_write(INI_DM_FREQ_OFFSET_1, &data, 1);
		        }

		        init_foffset = 150;
    		}
			break;
    		default:
        		S3281_PRINTF(" Sample clock or IF freq out of band! \n");
				break;
		}		
	}
    return SUCCESS;

}


void nim_s3281_dvbc_task(UINT32 param1, UINT32 param2)
{
    UINT8  i = 0;
    UINT32 curt_time = 0;
    UINT32 last_time[10]={0};
    struct nim_device *dev = (struct nim_device *) param1 ;
	BOOL   BER_VALID = FALSE;
	
#if (1 == M3281_LOG_FUNC)
    UINT8 data[4]={0};
    UINT32 sym_rate = 0;
    UINT32 rtp_sym = 0;
    UINT32 freq = 0;
    UINT8  qam_order = 0;
    UINT16 time_cons = 200;
    UINT8  lock = 0;
    UINT8  cnt = 0;
#endif

    if(dev == NULL)
    {
        return;
    }
    param2 = 0;

    for(i = 0; i < 10; i++)
    {
        last_time[i] = 0;
    }

#if (QAM_TEST_FUNC==SYS_FUNC_ON)
    for (i = 0; i < 8 ; i++)
        nim_reg_func_flag(i, FALSE, TRUE);
#endif

    while(1)
    {
        if(get_test_thread_off())
        {
            comm_sleep(100);
            continue;
        }

        curt_time = osal_get_tick();
        if (curt_time - last_time[0] > 300)
        {
            nim_s3281_dvbc_monitor_berper(dev, &BER_VALID);
            last_time[0] = osal_get_tick();

#if (1 == M3281_LOG_FUNC)
            {
                data[1] = 0x56;
                nim_s3281_dvbc_read(data[1], data, 1);
                S3281_PRINTF("CR%2x   =   0x%x   ", data[1], data[0]);

                data[1] = 0x0a;
                nim_s3281_dvbc_read(data[1], data, 1);
                S3281_PRINTF(" CR%2x   =   0x%x   ", data[1], data[0]);

                data[2] = 0x14;
                nim_s3281_dvbc_read(data[2], data, 2);
                S3281_PRINTF(" CR%2x   =   0x%x   ", data[2], (((data[1] & 0x03) << 8) | data[0]) );

                data[1] = 0x28;
                nim_s3281_dvbc_read(data[1], data, 1);
                S3281_PRINTF("CR%2x   =   0x%x   ", data[1], data[0]);

                data[1] = 0x30;
                nim_s3281_dvbc_read(data[1], data, 1);
                S3281_PRINTF("CR%2x   =   0x%x   ", data[1], data[0]);

                data[1] = 0xd8;
                nim_s3281_dvbc_read(data[1], data, 1);
                S3281_PRINTF("CR%2x   =   0x%x   ", data[1], data[0]);

                data[2] = 0x6c;
                nim_s3281_dvbc_read(data[2], data, 2);
                S3281_PRINTF("CR%2x   =   0x%x   ", data[2], ((data[1] << 8) | data[0]) );

                if(BER_VALID == TRUE)
                {
                    S3281_PRINTF("BER = %6d, PER = %d ", BER_COUNTS, PER_COUNTS);
                    BER_VALID = FALSE;
                }
                S3281_PRINTF(";\n");

                cnt ++;
                if (TASK_CNT == cnt)
                {
                    freq = s3281_cur_channel_info.frequency;
                    sym_rate = s3281_cur_channel_info.symbol_rate;
                    data[0] = s3281_cur_channel_info.modulation;

                    S3281_PRINTF("%s,freq = %d, sym = %d, fec = %d\n", __FUNCTION__, freq, sym_rate, data[0]);
                    cnt = 0;
                }

            }
#endif
        }

        // This Part is For Test Perpose----------
#if (QAM_TEST_FUNC==SYS_FUNC_ON)
        if (BER_VALID == TRUE)
        {
            //joey. 20090224. To make the driver more reasonable and readable.
            nim_s3281_dvbc_get_lock(dev, &lock);
            if (1 == lock )
                S3281_PRINTF("Locked, BER = %6d, PER = %d ;\n", BER_COUNTS, PER_COUNTS);
            else
                S3281_PRINTF("Unlock, BER = %6d, PER = %d ;\n", BER_COUNTS, PER_COUNTS);

            if (nim_reg_func_flag(1, FALSE, FALSE) == TRUE)
                nimreg_ber_refresh (BER_COUNTS, PER_COUNTS);
            BER_VALID = FALSE;
        }

        if (nim_reg_func_flag(0, FALSE, FALSE) == TRUE)
        {
            data[0] = 0x80;
            nim_s3281_dvbc_write(0x00, data, 1);
            data[0] = 0x40;
            nim_s3281_dvbc_write(0x00, data, 1);
            nim_reg_func_flag(0, FALSE, TRUE) ;
        }
        if (nim_reg_func_flag(INDEX_2, FALSE, FALSE) == TRUE)
        {
            if (curt_time - last_time[1] > COMPARE_VALUE_500)
            {
                display_dynamic_vision();
                last_time[1] = osal_get_tick();
            }
        }
        if (nim_reg_func_flag(INDEX_3, FALSE, FALSE) == TRUE)
        {
            nim_s3281_dvbc_mon_catch_ad_data(dev);
            nim_reg_func_flag(INDEX_3, FALSE, TRUE) ;
        }

        if (nim_reg_func_flag(INDEX_7, FALSE, FALSE) == TRUE)
            time_cons = 5;
        else
            time_cons = 300;

        if (curt_time - last_time[2] > time_cons)
        {
            nim_reg_print();
            last_time[2] = osal_get_tick();
        }
#else
        if (BER_VALID == TRUE)
        {
#ifndef SFU_AUTO_TEST
            //libc_printf("BER = %6d, PER = %d ;\n",BER_COUNTS,PER_COUNTS);
#endif
            BER_VALID = FALSE;
        }
#endif

        // This Part is For Patch Perpose----------
#if (QAM_FPGA_USAGE == SYS_FUNC_OFF)
        //joey 20080504. update according to program guide 20080430 by Dan.
        if ((sys_ic_get_rev_id() == IC_REV_1) && (rf_agc_en == TRUE))
        {
            nim_s3281_dvbc_monitor_agc_status(dev, NULL);
        }

        if (sys_ic_get_rev_id() > IC_REV_0)
        {
            //joey 20090225. for CCI pK with stv0297e.
            nim_s3281_dvbc_monitor_agc0a_loop(dev, NULL);

#if (1 == M3281_LOG_FUNC)
            {
                nim_s3281_dvbc_read(0x05, data, 1);
                if ( 0x80 == (data[0] & 0x80) )
                {
                    S3281_PRINTF("Capture data start \n");
                    for(i = 0; i < 10; i++)
                    {
                        nim_s3281_dvbc_mon_catch_ad_data_loop(dev);
                    }
                    S3281_PRINTF("Capture data end \n");
                    data[0] = 0x00;
                    nim_s3281_dvbc_write(0x05, data, 1);
                }
            }
#endif
        }
#endif
    }
}




