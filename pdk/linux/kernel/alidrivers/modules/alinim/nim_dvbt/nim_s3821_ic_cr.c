/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File:  nim_s3821_ic_01.c
*   
*    Description:  s3821 ic layer
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

#include "nim_s3821.h"

#define MAGIC_CONST_0      0
#define MAGIC_CONST_1      1
#define MAGIC_CONST_2      2
#define MAGIC_CONST_3      3
#define MAGIC_CONST_4      4
#define MAGIC_CONST_5      5
#define MAGIC_CONST_6      6
#define MAGIC_CONST_7      7
#define MAGIC_CONST_100    100
#define MAGIC_CONST_300    300 




#define MAGIC_CONST_256    256
#define MAGIC_CONST_512    512

#define MAGIC_CONST_1024   1024
#define MAGIC_CONST_2048   2048
#define MAGIC_CONST_4096   4096
#define MAGIC_CONST_8192   8192

#define MAGIC_CONST_3520   3520
#define MAGIC_CONST_3620   3620

#define MAGIC_CONST_3950   3950
#define MAGIC_CONST_4050   4050

#define MAGIC_CONST_4950   4950
#define MAGIC_CONST_5050   5050

#define MAGIC_CONST_4520   4520
#define MAGIC_CONST_4620   4620

#define MAGIC_CONST_43500  43500
#define MAGIC_CONST_44500  44500
#define MAGIC_CONST_35500  35500
#define MAGIC_CONST_36500  36500









UINT32 g_var_adc2dma_addr = NIM_S3821_ADC2DMA_START_ADDR;
UINT32 g_var_adc2dma_len = (NIM_S3821_ADC2DMA_MEM_LEN - 2 * 8192); // to avoid the buffer excess the bound limit.

//joey, 20120820, for auto-test, disable un-related print.

UINT8 dem_print = 1;

//joey, 20120831, for CCI print.

UINT8 cci_info_print = 0;

//joey, 20120912, for T2 function print on/off switch.

BOOL mon_ifft_en = FALSE;
UINT8 mon_status_print = 0;



static UINT16 ifft_result[8192] ={0};


INT32 nim_s3821_read_external_cofdm_mode(struct nim_device *dev, UINT16 reg_add, UINT8 *data, UINT8 len)
{
    INT32 err = 0;
    struct nim_s3821_private *priv = (struct nim_s3821_private *)(dev->priv);
    UINT32 i2c_type_id = 0;
    UINT8 reg_page = ((UINT16)reg_add) >> 8;
    UINT8 buffer[2] ={0};

#ifdef T2_EXTERNAL_EVKIT

    //joey, 20120807, for external sony box support.
    return SUCCESS;
#endif

    i2c_type_id = priv->tuner_control.ext_dm_config.i2c_type_id;
    osal_mutex_lock(priv->nim_s3821_i2c_mutex, OSAL_WAIT_FOREVER_TIME);
    // change register page if nessary
    if ( priv->m_reg_page != reg_page )
    {
        priv->m_reg_page = reg_page;
        buffer[0] = 0xFF;
        buffer[1] = reg_page;
        err = nim_i2c_write(i2c_type_id, dev->base_addr, buffer, 2);
    }

    data[0] = reg_add & 0xFF;
    err = nim_i2c_write(i2c_type_id, dev->base_addr, data, 1);
    if (err == SUCCESS)
    {
        err = nim_i2c_read(i2c_type_id, dev->base_addr, data, len);
    }

    osal_mutex_unlock(priv->nim_s3821_i2c_mutex);
    return err;

}



INT32 nim_s3821_write_external_cofdm_mode(struct nim_device *dev, UINT16 reg_add, UINT8 *data, UINT8 len)
{

    INT32  err = 0;

    UINT8  i = 0;
	UINT8  buffer[8] ={0};
    struct nim_s3821_private *priv = (struct nim_s3821_private *)(dev->priv);
    UINT32 i2c_type_id = 0;
    UINT8 reg_page = ((UINT16)reg_add) >> 8;

#ifdef T2_EXTERNAL_EVKIT
    //joey, 20120807, for external sony box support.
    return SUCCESS;
#endif

    if (len > MAGIC_CONST_7)
    {
        return ERR_FAILUE;
    }

    i2c_type_id = priv->tuner_control.ext_dm_config.i2c_type_id;
    osal_mutex_lock(priv->nim_s3821_i2c_mutex, OSAL_WAIT_FOREVER_TIME);
    // change register page if nessary

    if ( priv->m_reg_page != reg_page )
    {
        priv->m_reg_page = reg_page;
        buffer[0] = 0xFF;
        buffer[1] = reg_page;
        err = nim_i2c_write(i2c_type_id, dev->base_addr, buffer, 2);
    }

    buffer[0] = reg_add & 0xFF;
    for (i = 0; i < len; i++)
    {
        buffer[i + 1] = data[i];
    }

    err = nim_i2c_write(i2c_type_id, dev->base_addr, buffer, len + 1);

    osal_mutex_unlock(priv->nim_s3821_i2c_mutex);

    return err;

}



/*****************************************************************************
* INT32 nim_s3821_mon_internal_status(struct nim_device *dev, UINT8 *lock)
*
*  Read FEC lock status
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: BOOL *fec_lock
*
*Return Value: INT32
*****************************************************************************/
static INT32 nim_s3821_mon_internal_status(struct nim_device *dev, UINT32 *status)
{
    UINT8 tmp_data[3] = {0};

	do
    {
        NIM_S3821_DEBUG("\n");

        nim_s3821_read(dev, 0x84, tmp_data, 2);
        NIM_S3821_DEBUG("CR84: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR85: 0x%2x \n", tmp_data[1]);

        nim_s3821_read(dev, 0x86, tmp_data, 2);
        NIM_S3821_DEBUG("CR86: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR87: 0x%2x \n", tmp_data[1]);

        nim_s3821_read(dev, 0x88, tmp_data, 2);
        NIM_S3821_DEBUG("CR88: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR89: 0x%2x \n", tmp_data[1]);

        nim_s3821_read(dev, 0x127, tmp_data, 3);
        NIM_S3821_DEBUG("CR127: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR128: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR129: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x12a, tmp_data, 3);
        NIM_S3821_DEBUG("CR12a: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR12b: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR12c: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x12d, tmp_data, 3);
        NIM_S3821_DEBUG("CR12d: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR12e: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR12f: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x13f, tmp_data, 2);
        NIM_S3821_DEBUG("CR13f: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR140: 0x%2x \n", tmp_data[1]);

        nim_s3821_read(dev, 0x141, tmp_data, 2);
        NIM_S3821_DEBUG("CR141: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR142: 0x%2x \n", tmp_data[1]);

        nim_s3821_read(dev, 0x145, tmp_data, 1);
        NIM_S3821_DEBUG("CR145: 0x%2x \n", tmp_data[0]);

        nim_s3821_read(dev, 0x1b9, tmp_data, 2);
        NIM_S3821_DEBUG("CR1ba: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR1b9: 0x%2x \n", tmp_data[0]);

        nim_s3821_read(dev, 0x0f, tmp_data, 1);
        NIM_S3821_DEBUG("CR0f: 0x%2x \n", tmp_data[0]);

        nim_s3821_read(dev, 0x11, tmp_data, 3);

        NIM_S3821_DEBUG("CR11: 0x%2x \n", tmp_data[0]);

        NIM_S3821_DEBUG("CR12: 0x%2x \n", tmp_data[1]);

        NIM_S3821_DEBUG("CR13: 0x%2x \n", tmp_data[2]);



        nim_s3821_read(dev, 0x22, tmp_data, 2);

        NIM_S3821_DEBUG("CR22: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR23: 0x%2x \n", tmp_data[1]);

        nim_s3821_read(dev, 0x24, tmp_data, 3);
        NIM_S3821_DEBUG("CR24: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR25: 0x%2x \n", tmp_data[1]);

        nim_s3821_read(dev, 0x26, tmp_data, 3);
        NIM_S3821_DEBUG("CR26: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR27: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR28: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x43, tmp_data, 3);
        NIM_S3821_DEBUG("CR43: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR44: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR45: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x46, tmp_data, 3);
        NIM_S3821_DEBUG("CR46: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR47: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR48: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x49, tmp_data, 3);
        NIM_S3821_DEBUG("CR49: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR4a: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR4b: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x4c, tmp_data, 2);
        NIM_S3821_DEBUG("CR4c: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR4d: 0x%2x \n", tmp_data[1]);

        nim_s3821_read(dev, 0x5d, tmp_data, 2);
        NIM_S3821_DEBUG("CR5d: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR5e: 0x%2x \n", tmp_data[1]);

        nim_s3821_read(dev, 0x11e, tmp_data, 3);
        NIM_S3821_DEBUG("CR11e: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR120: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x65, tmp_data, 3);
        NIM_S3821_DEBUG("CR65: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR67: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x96, tmp_data, 1);
        NIM_S3821_DEBUG("CR96: 0x%2x \n", tmp_data[0]);

        nim_s3821_read(dev, 0x6d, tmp_data, 2);
        NIM_S3821_DEBUG("CR6d: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR6e: 0x%2x \n", tmp_data[1]);

        nim_s3821_read(dev, 0x75, tmp_data, 2);
        NIM_S3821_DEBUG("CR75: 0x%2x \n", tmp_data[0]);

        nim_s3821_read(dev, 0x19e, tmp_data, 3);
        NIM_S3821_DEBUG("CR19e: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR19f: 0x%2x \n", tmp_data[1]);

        NIM_S3821_DEBUG("CR1a0: 0x%2x \n", tmp_data[2]);
 
        nim_s3821_read(dev, 0x1a1, tmp_data, 3);
        NIM_S3821_DEBUG("CR1a1: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR1a2: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR1a3: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x1a4, tmp_data, 3);
        NIM_S3821_DEBUG("CR1a4: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR1a5: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR1a6: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x1a7, tmp_data, 3);
        NIM_S3821_DEBUG("CR1a7: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR1a8: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR1a9: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x1aa, tmp_data, 3);
        NIM_S3821_DEBUG("CR1aa: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR1ab: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR1ac: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x1ad, tmp_data, 3);
        NIM_S3821_DEBUG("CR1ad: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR1ae: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR1af: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x165, tmp_data, 3);
        NIM_S3821_DEBUG("CR165: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR166: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR167: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x16f, tmp_data, 3);
        NIM_S3821_DEBUG("CR16f: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR170: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR171: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x174, tmp_data, 1);
        NIM_S3821_DEBUG("CR174: 0x%2x \n", tmp_data[0]);

        nim_s3821_read(dev, 0x177, tmp_data, 2);
        NIM_S3821_DEBUG("CR177: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR178: 0x%2x \n", tmp_data[1]);

        nim_s3821_read(dev, 0x179, tmp_data, 2);
        NIM_S3821_DEBUG("CR179: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR17a: 0x%2x \n", tmp_data[1]);

        nim_s3821_read(dev, 0x17e, tmp_data, 3);
        NIM_S3821_DEBUG("CR17e: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR17f: 0x%2x \n", tmp_data[1]);
        NIM_S3821_DEBUG("CR180: 0x%2x \n", tmp_data[2]);

        nim_s3821_read(dev, 0x183, tmp_data, 3);
        NIM_S3821_DEBUG("CR183: 0x%2x \n", tmp_data[0]);
        NIM_S3821_DEBUG("CR185: 0x%2x \n", tmp_data[2]);
    }
    while (0);

    return SUCCESS;

}

static INT32 nim_s3821_get_dvbt2_lock(struct nim_device *dev, UINT8 *lock)
{
    struct nim_s3821_private *dev_priv = NULL;
    UINT8 data = 0;
    OSAL_ER	result = 0;
    UINT32 flgptn = 0;

    dev_priv = (struct nim_s3821_private *)dev->priv;
	result = osal_flag_wait(&flgptn, dev_priv->nim_flag, NIM_3821_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK == result)
    {
        nim_s3821_read(dev, 0x67, &data, 1);
        if((data & 0x0f) >= 0x0a) // FSM status.
        {
            *lock = 1;
        }
        else
        {
            *lock = 0;
        }
    }
    else
    {
        *lock = 0xff;
    }

    return SUCCESS;
}

static INT32 nim_s3811_get_dvbt_isdbt_lock(struct nim_device *dev, UINT8 *lock)
{
    struct nim_s3821_private *dev_priv = NULL;
    UINT8 data = 0;
    OSAL_ER	result = 0;
    UINT32 flgptn = 0;

    dev_priv = (struct nim_s3821_private *)dev->priv;
    result = osal_flag_wait(&flgptn, dev_priv->nim_flag, NIM_3821_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK == result)
    {
        nim_s3821_read(dev, 0x1d, &data, 1);
        if((data & 0x20) == 0x20)
        {
            *lock = 1;
        }
        else
        {
            *lock = 0;
        }
    }
    else
    {
        *lock = 0xff;
    }

    return SUCCESS;

}

INT32 nim_s3821_get_lock(struct nim_device *dev, UINT8 *lock)
{
    UINT8 cur_mode = 0;

    //NIM_S3821_DEBUG("[%s]line=%d,here!\n",__FUNCTION__,__LINE__);
   
    nim_s3821_get_cur_mode(dev, &cur_mode);
    if (MODE_DVBT2 == cur_mode)
    {
        return nim_s3821_get_dvbt2_lock(dev, lock);
    }
    else if ((MODE_DVBT == cur_mode) || (MODE_ISDBT == cur_mode))
    {
        return nim_s3811_get_dvbt_isdbt_lock(dev, lock);
    }
    else
    {
        *lock = 0;
    }

    return SUCCESS;
}

INT32 nim_s3821_dvbt2_getinfo(struct nim_device *dev, UINT8 *guard_interval, 
              UINT8 *fft_mode, UINT8 *modulation, UINT8 *code_rate)
{
    UINT8 data = 0;

    //for DVBT2 get modulation parameters.
    //part for gi.
    nim_s3821_read(dev, 0x174, &data, 1);
    switch ((data & 0x38) >> 3)
    {
    case 0:
        *guard_interval = GUARD_1_32;
        break;
    case 1:
        *guard_interval = GUARD_1_16;
       break;
    case 2:
        *guard_interval = GUARD_1_8;
        break;
    case 3:
        *guard_interval = GUARD_1_4;
       break;

    case 4: // 1/128.
        *guard_interval = 128;
        break;
    case 5: // 19/128.
        *guard_interval = (19 + 128);
        break;
    case 6: // 19/256.
        *guard_interval = 19;
        break;
    default:
        *guard_interval = 0xff; /* error */
        break;
    }

    //part for fft_mode.
    nim_s3821_read(dev, 0x65, &data, 1);
    switch ((data & 0x70) >> 4)
    {
    case 0:
        *fft_mode = MODE_2K;
        break;
    case 1:
    case 6:
        *fft_mode = MODE_8K;
        break;
    case 2:
        *fft_mode = MODE_4K;
        break;
    case 3:
        *fft_mode = 1;
        break;
    case 4:
        *fft_mode = 16;
        break;
    case 5:
    case 7:
        *fft_mode = 32;
        break;
    default:
        *fft_mode = 0xff; /* error */
        break;
    }

    //part for modulation.
    nim_s3821_read(dev, 0x183, &data, 1);
    switch ((data & 0x18) >> 3)
    {
    case 0:
        * modulation = TPS_CONST_QPSK;
        break;
    case 1:
        * modulation = TPS_CONST_16QAM;
        break;
    case 2:
        * modulation = TPS_CONST_64QAM;
        break;
    case 3: // 256Qam.
        * modulation = (64 + 1);
        break;
    default:
        * modulation = 0xff; /* error */
        break;
    }

    //part for code_rate.
    switch (data & 0x07)
    {
    case 0:
        *code_rate = FEC_1_2;
        break;
    case 1: // 3/5.
        *code_rate = 5 ;
        break;
    case 2:
        *code_rate = FEC_2_3;
        break;
    case 3:
        *code_rate = FEC_3_4;
        break;
    case 4: // 4/5.
        *code_rate = 6;
        break;
    case 5:
        *code_rate = FEC_5_6;
        break;
    default:
        *code_rate = 0xf; /* error */
        break;
    }

    return SUCCESS;
}


static INT32 nim_s3821_monitor_ifft_result(struct nim_device *dev)
{
    UINT16 ifft_idx  = 0;
    UINT16 i = 0;
    UINT16 rd_addr = 0;
    UINT8 rd_arr[2] = {0};
    UINT8 rd_data[3]= {0};
    UINT8 cnt = 0;
    UINT8 data = 0;
    UINT8 tmp_dat = 0;
    INT16 ifft_i = 0;
	INT16 ifft_q = 0;
    UINT16 abs_ifft_i = 0;
	UINT16 abs_ifft_q = 0;
	UINT16 iq_tmp = 0;
    UINT8 timeout_flg = 0; // 0: means OK, if 1, means abnormal.

    nim_s3821_read(dev, 0x67, &data, 1);
    if ((data & 0x0f) < 0x0a)		// if s3110 OFDM unlock, then call back
    {
       return ERR_FAILUE;
    }

    NIM_S3821_DEBUG("\nlog Ifft result once a time!\n");
    nim_s3821_read(dev, 0x12, &data, 1);
    tmp_dat = (data & 0x1c) >> 2;
    if (0 == (data & 0x02)) // p2_type == 0.
    {
        if (MAGIC_CONST_0 == tmp_dat) // 1k.
        {
            ifft_idx = MAGIC_CONST_1024;
            NIM_S3821_DEBUG("1k mode\n");
        }
        else if ((MAGIC_CONST_1 == tmp_dat) || (0x06 == tmp_dat))// 4k.
        {
            ifft_idx = MAGIC_CONST_4096;
            NIM_S3821_DEBUG("4k mode\n");
        }
        else if (MAGIC_CONST_2 == tmp_dat) // 2k.
        {
            ifft_idx = MAGIC_CONST_2048;
            NIM_S3821_DEBUG("2k mode\n");
        }
        else if (MAGIC_CONST_3 == tmp_dat) // 512.
        {
            ifft_idx = MAGIC_CONST_512;
            NIM_S3821_DEBUG("512 mode\n");
        }
        else if ((MAGIC_CONST_4 == tmp_dat) || (MAGIC_CONST_5 == tmp_dat) || (MAGIC_CONST_7 == tmp_dat))// 8k.
        {
            ifft_idx = MAGIC_CONST_8192;
            NIM_S3821_DEBUG("8k mode\n");
        }
        else
        {
            return ERR_FAILUE;	// unsupported mode
        }

    }
    else
    {
        if (MAGIC_CONST_0 == tmp_dat) // 512.
        {
            ifft_idx = MAGIC_CONST_512;
            NIM_S3821_DEBUG("512 mode\n");
        }
        else if ((MAGIC_CONST_1 == tmp_dat) || (0x06 == tmp_dat))// 2k.
        {
            ifft_idx = MAGIC_CONST_2048;
            NIM_S3821_DEBUG("2k mode\n");
        }
        else if (MAGIC_CONST_2 == tmp_dat) // 1k.
        {
            ifft_idx = MAGIC_CONST_1024;
            NIM_S3821_DEBUG("1k mode\n");
        }
        else if (MAGIC_CONST_3 == tmp_dat) // 256.
        {
            ifft_idx =MAGIC_CONST_256;
            NIM_S3821_DEBUG("256 mode\n");
        }
        else if (MAGIC_CONST_4 == tmp_dat) // 4096.
        {
            ifft_idx = MAGIC_CONST_4096;
            NIM_S3821_DEBUG("4k mode\n");
        }
        else if ((MAGIC_CONST_5 == tmp_dat) || (0x07 == tmp_dat))// 8k.
        {
            ifft_idx = MAGIC_CONST_8192;
            NIM_S3821_DEBUG("8k mode\n");
        }
        else
        {
            return ERR_FAILUE;	// unsupported mode
        }
    }

    nim_s3821_read(dev, 0x68, &data, 1);	// freeze the cci pre in tracking mode
    data = data & 0x7f;
    nim_s3821_write(dev, 0x68, &data, 1);

    //joey, 20111031. for ifft function debug.
    osal_task_sleep(200);
    for (i = 0; i < ifft_idx; i++)
    {
        rd_addr = i;
        rd_arr[0] = (UINT8)(rd_addr & 0xff);
        rd_arr[1] = (UINT8)((rd_addr >> 8) & 0x1f);
        nim_s3821_write(dev, 0x90, rd_arr, 2);	 // update IFFT_RD_ADDR
        
        //joey, 20111031. for ifft function debug.
        //osal_delay(1000);
        cnt = 0;
        while (1)
        {
            nim_s3821_read(dev, 0x95, rd_data, 1);
            //joey, 20111031. for ifft function debug.
            //osal_delay(1000);
            if ((rd_data[0] & 0x80) == 0x80)		// IFFT_RD_RDY ==1
            {
                break;
            }

            cnt += 1;
            if (cnt > 20 * 2)
            {
                timeout_flg = 1;
                break;
            }
            osal_task_sleep(2);
        }

        //joey, 20111101, for ifft function abnormal dealing.
        if (1 == timeout_flg)
        {
          //abnormal, quit read action.
            break;
        }

        nim_s3821_read(dev, 0x92, rd_data, 3); // 24-bit all.

        abs_ifft_i = (UINT16)((rd_data[2] << 4) | ((rd_data[1] & 0xf0) >> 4));
        abs_ifft_q = (UINT16)(((rd_data[1] & 0x0f) << 8) | rd_data[0]);

        if (0x800 == (abs_ifft_i & 0x800)) // means it's should be a negative value.
        {

            ifft_i = (INT16)(abs_ifft_i | 0xf000);
            abs_ifft_i = (UINT16)(-ifft_i);
        }

        if (0x800 == (abs_ifft_q & 0x800)) // means it's should be a negative value.
        {
            ifft_q = (INT16)(abs_ifft_q | 0xf000);
            abs_ifft_q = (UINT16)(-ifft_q);

        }

        if (abs_ifft_i >= abs_ifft_q)
        {
            iq_tmp = 	abs_ifft_i + (abs_ifft_q >> 2);
        }
        else
        {
            iq_tmp = 	abs_ifft_q + (abs_ifft_i >> 2);
        }
        ifft_result[i]  = iq_tmp;
    }

    if (0 == timeout_flg) // no time out case happen.
    {
        NIM_S3821_DEBUG("\nstart!\n");
        for (i = 0; i < ifft_idx; i++)
        {
            NIM_S3821_DEBUG("%4d\n", ifft_result[i]);
            //NIM_S3821_DEBUG("0x%2x, 0x%2x, 0x%2x, %4d\n", reg_arr[i*3+0], reg_arr[i*3+1], 
	    //               reg_arr[i*3+2], ifft_result[i]);
        }
        NIM_S3821_DEBUG("\nend!\n");
    }
    else
    {
        NIM_S3821_DEBUG("\nread ifft result fail!!!!\n");
    }

    nim_s3821_read(dev, 0x68, &data, 1);	// release the ccipre freeze
    data = data | 0x80;
    nim_s3821_write(dev, 0x68, &data, 1);
    return SUCCESS;

}


static UINT8 nim_s3821_get_ad_gain_level(nim_ad_gain_table_t *agt, UINT16 ad_val, UINT8 slp)
{
    UINT8 i = 0;
	
    for(i = 0; i < agt->count; i++)
    {
        if(1 == slp)
        {
            if(ad_val >= agt->p_table[i])
            {
				break;
            } 	
        }
        else
        {
            if(ad_val <= agt->p_table[i])
            {
                break;
            }
        }
    }
    if(i >= agt->count)
    {
        i = agt->count;
    }

    return i;

}


static UINT32 nim_s3821_monitor_cci_last_read_mem(struct nim_device *dev)
{
    UINT8 cnt = 0;
	UINT8 data = 0;
    UINT8 demod_type = 0;
    UINT16 i = 0;
    UINT16 chan_diff_idx = 0;
    UINT16 rd_addr = 0;
    UINT8 rd_arr[2] = {0};
    UINT8 rd_data[3] = {0};
    UINT16 tmp_data = 0;
    INT16 tmp_data_i = 0;
	INT16 tmp_data_q = 0;

    nim_s3821_read(dev, 0x1d, &data, 1);
    if ((data & 0x40) == 0x00)		// if s3811 OFDM unlock, then call back
    {
        return ERR_FAILUE;
    }

    NIM_S3821_DEBUG("\nlog chan_diff result once a time!\n");

    nim_s3821_read(dev, 0x12, &demod_type, 1);
    demod_type = demod_type >> 7;
    nim_s3821_read(dev, 0x1e, &data, 1);
    if ((data & 0x30) == 0x00) // 2k.
    {
        if(demod_type == DVBT_TYPE)
        {
            chan_diff_idx = 569;
        }
        else
        {
            chan_diff_idx = 469;
        }

        NIM_S3821_DEBUG("2k mode\n");
    }
    else if ((data & 0x30) == 0x10) // 8k.
    {
        if(demod_type == DVBT_TYPE)
        {
            chan_diff_idx = 2273;
        }
        else
        {
            chan_diff_idx = 1873;
        }

        NIM_S3821_DEBUG("8k mode\n");

    }
    else if((data & 0x30) == 0x20) // 4k
    {
        if(demod_type == DVBT_TYPE)
        {
            chan_diff_idx = 1137;
        }
        else
        {
            chan_diff_idx = 937;
        }
        NIM_S3821_DEBUG("4k mode\n");
    }
    else
    {
        return ERR_FAILUE;	// unsupported mode
    }
    NIM_S3821_DEBUG("length=%d\n", chan_diff_idx);
    NIM_S3821_DEBUG("chan_diff_start!\n");
    for (i = 0; i < chan_diff_idx; i++)
    {
        rd_addr = i;
        rd_arr[0] = (UINT8)(rd_addr & 0xff);
        rd_arr[1] = (UINT8)((rd_addr >> 8) & 0x0f);
        nim_s3821_write(dev, 0xbf, rd_arr, 2);	 // update chan_diff_addr
        cnt = 0;
        while (1)
        {
            nim_s3821_read(dev, 0xc3, rd_data, 1);
            if ((rd_data[0] & 0x80) == 0x80)		// DIFF_CHAN_RD_RDY ==1
            {
                rd_data[2] = rd_data[0];
                break;
            }
            cnt += 1;
            if (cnt > MAGIC_CONST_100)
            {
                return ERR_FAILUE;
            }
            osal_delay(20);
        }

        nim_s3821_read(dev, 0xc1, rd_data, 2);
        tmp_data = (UINT16)(((rd_data[2] & 0x0f) << 6) | ((rd_data[1] >> 2) & 0x3f));
        if(tmp_data > MAGIC_CONST_512)
        {
            tmp_data_i = tmp_data - 1024;
        }
        else
        {
            tmp_data_i = tmp_data;
        }

        tmp_data = (UINT16)(((rd_data[1] & 0x03) << 8) | rd_data[0]);
        if(tmp_data > MAGIC_CONST_512)
        {
            tmp_data_q = tmp_data - 1024;
        }
        else
        {
            tmp_data_q = tmp_data;
        }
        NIM_S3821_DEBUG("%4d	%4d\n", tmp_data_i, tmp_data_q);
    }

    NIM_S3821_DEBUG("chan_diff_end!\n");
    return SUCCESS;
}

//joey, 20130715. for adc2dma capture function.
INT32 nim_s3821_init_adc2dma(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT32 tmp_1 = 0;
    UINT16 tmp_len = 0;
    UINT8 tmp_arr[4];

    // step 0: Change to ADC2DMA memory space directly.
    nim_s3821_read(dev, 0x2ff, &data, 1);
    data = data | 0x80;
    nim_s3821_write(dev, 0x2ff, &data, 1);

    // step 1: get memory address and len.
    //because the below is from CPU address, get rid of the highest 4bit when set to adc2dma register.

    //	tmp_1 = (NIM_S3821_ADC2DMA_START_ADDR & 0x0fffffff) >> 2; // base_address/4.
    //	tmp_1 = (((UINT32)(dev_priv->Tuner_Control.config_data.memory))  & 0x0fffffff) >> 2; // base_address/4.
    // base_address/4. //plus 3, avoid 4-byte align issue. the upper 2-bit is unseen for this IP.
     // and with CPU limit, equal 3-bit unseen.
    tmp_1 = ((g_var_adc2dma_addr + 3) & 0x1fffffff) >> 2; 

    //tmp_len = (NIM_S3821_ADC2DMA_MEM_LEN) >> 13; // 8K uinit.
    //tmp_len = dev_priv->Tuner_Control.config_data.memory_size >> 13; // 8K uinit.
    //tmp_len = (70*1024*1024) >> 13; // 8K uinit.
    tmp_len = (g_var_adc2dma_len) >> 13; // 8K uinit.
    if (tmp_len <= 0) // no real buffer, return.
    {
        return ERR_FAILURE;
    }

    // step 2: set memory address to buffer address.
    tmp_arr[0] = (UINT8)(tmp_1 & 0xff);
    tmp_arr[1] = (UINT8)((tmp_1 >> 8) & 0xff);
    tmp_arr[2] = (UINT8)((tmp_1 >> 16) & 0xff);
    tmp_arr[3] = (UINT8)((tmp_1 >> 24) & 0x0f);
    nim_s3821_write(dev, 0x04, tmp_arr, 4);
	
    tmp_arr[0] = (UINT8)(tmp_len & 0xff);
    tmp_arr[1] = (UINT8)((tmp_len >> 8) & 0x7f);
    nim_s3821_write(dev, 0x02, tmp_arr, 2);

    // step 3: Change out of ADC2DMA memory space to original setting.
    nim_s3821_read(dev, 0x2ff, &data, 1);
    data = data & 0x7f;
    nim_s3821_write(dev, 0x2ff, &data, 1);

    return SUCCESS;
}

static INT32 nim_s3821_init_27madc(struct nim_device *dev,struct COFDM_TUNER_CONFIG_API *config_info)
{
	UINT8 data = 0;
    UINT8 data_arr[4] = {0};
	INT32 ret=RET_CONTINUE;
	
    //The below is just for 27M now.
    // check zero IF system
    if ((config_info->config_data.cofdm_config & 0x10) == 0x00) // 0: low-if, shutdown Q path, 1: zero-if, still on.
    {
        if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_4520) && 
          (config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_4620)) // for mxl5007, 4.57M., low low-IF.
        {
            data_arr[0] = 0xb5;
            data_arr[1] = 0x49;
            data_arr[2] = 0x32;
            nim_s3821_write(dev, 0x14, data_arr, 3);

            nim_s3821_read(dev, 0x13, &data, 1);
            data = (UINT8)((data & 0xf0) | 0x02);
            nim_s3821_write(dev, 0x13, &data, 1);
        }
        else if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_3950) && 
               (config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_4050)) // for mxl136, 4.00M., low low-IF.
        {
            data_arr[0] = 0x5e;
            data_arr[1] = 0xd0;
            data_arr[2] = 0x97;
            nim_s3821_write(dev, 0x14, data_arr, 3);

            nim_s3821_read(dev, 0x13, &data, 1);
            data = (UINT8)((data & 0xf0) | 0x02);
            nim_s3821_write(dev, 0x13, &data, 1);
        }
        else if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_3520) && 
              (config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_3620)) // for R820T 6M, 3.57M, low low-IF.
        {
            data_arr[0] = 0x1d;
            data_arr[1] = 0x95;
            data_arr[2] = 0x0c;
            nim_s3821_write(dev, 0x14, data_arr, 3);

            nim_s3821_read(dev, 0x13, &data, 1);
            data = (UINT8)((data & 0xf0) | 0x02);
            nim_s3821_write(dev, 0x13, &data, 1);
        }
        else if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_35500) && 
                (config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_36500)) // for TD1611, 36M low-IF.
        {
            //use (36 -27) = 9M as the frequency been sampled.
            data_arr[0] = 0x55;
            data_arr[1] = 0x55;
            data_arr[2] = 0x55;
            nim_s3821_write(dev, 0x14, data_arr, 3);

            nim_s3821_read(dev, 0x13, &data, 1);
            data = (UINT8)((data & 0xf0) | 0x05);
            nim_s3821_write(dev, 0x13, &data, 1);
        }
        else if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_43500) && 
             (config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_44500)) // for sharp6401, 44M low-IF.
        {
            //use (44-27) = 17M as the frequency been sampled.
            data_arr[0] = 0x12;
            data_arr[1] = 0xf6;
            data_arr[2] = 0x84;
            nim_s3821_write(dev, 0x14, data_arr, 3);

            nim_s3821_read(dev, 0x13, &data, 1);
            data = (UINT8)((data & 0xf0) | 0x0a);
            nim_s3821_write(dev, 0x13, &data, 1);
        }
        else
        {
            NIM_S3821_DEBUG("Error, un-support tuner type!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
            return ERR_FAILUE;
        }
    }
    else //zero-if, setting it.
    {
        nim_s3821_read(dev, 0x13, &data, 1);
        data = (UINT8)((data & 0xbf) | 0x40);			// set zero-IF
        nim_s3821_write(dev, 0x13, &data, 1);

        // set initial frequency offset to 0
        data_arr[0] = 0x0;
        data_arr[1] = 0x0;
        data_arr[2] = 0x0;
        nim_s3821_write(dev, 0x14, data_arr, 3);

        nim_s3821_read(dev, 0x13, &data, 1);
        data = (UINT8)(data & 0xf0);
        nim_s3821_write(dev, 0x13, &data, 1);

        NIM_S3821_DEBUG("zero if setting (1) \n");
    }	

	return ret;
	
}


static INT32 nim_s3821_init_54madc(struct nim_device *dev,struct COFDM_TUNER_CONFIG_API *config_info)
{
	UINT8 data = 0;
    UINT8 data_arr[4] = {0};
	INT32 ret=RET_CONTINUE;
	UINT32 data_tmp = 0;

    //The below is just for 54M now.
    // check zero IF system
    if ((config_info->config_data.cofdm_config & 0x10) == 0x00) // 0: low-if, shutdown Q path, 1: zero-if, still on.
    {
        if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_4520) && 
          (config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_4620)) // for mxl5007, 4.57M., low low-IF.
        {
            data_arr[0] = 0x5a;
            data_arr[1] = 0xa4;
            data_arr[2] = 0x99;
            nim_s3821_write(dev, 0x14, data_arr, 3);

            nim_s3821_read(dev, 0x13, &data, 1);
            data = (UINT8)((data & 0xf0) | 0x01);
            nim_s3821_write(dev, 0x13, &data, 1);
        }
        else if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_3950) && 
              (config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_4050)) // for mxl136, 4.00M., low low-IF.
        {
            data_arr[0] = 0x2f;
            data_arr[1] = 0x68;
            data_arr[2] = 0x4b;
            nim_s3821_write(dev, 0x14, data_arr, 3);

            nim_s3821_read(dev, 0x13, &data, 1);
            data = (UINT8)((data & 0xf0) | 0x01);
            nim_s3821_write(dev, 0x13, &data, 1);
        }
        else if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_35500) && 
            (config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_36500)) // for TD1611, 36M low-IF.
        {
            //use 36M as the frequency been sampled.
            data_arr[0] = 0xaa;
            data_arr[1] = 0xaa;
            data_arr[2] = 0xaa;
            nim_s3821_write(dev, 0x14, data_arr, 3);

            nim_s3821_read(dev, 0x13, &data, 1);
            data = (UINT8)((data & 0xf0) | 0x0a);
            nim_s3821_write(dev, 0x13, &data, 1);
        }
        else if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_43500) && 
            (config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_44500)) // for sharp6401, 44M low-IF.
        {
            data_arr[0] = 0x09;
            data_arr[1] = 0x7b;
            data_arr[2] = 0x42;
            nim_s3821_write(dev, 0x14, data_arr, 3);

            nim_s3821_read(dev, 0x13, &data, 1);
            data = (UINT8)((data & 0xf0) | 0x0d);
            nim_s3821_write(dev, 0x13, &data, 1);
        }
        else
        {
            NIM_S3821_DEBUG("Error, un-support tuner type!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
            return ERR_FAILUE;
        }
    }
    else //zero-if, setting it.
    {
        nim_s3821_read(dev, 0x13, &data, 1);
        data = (UINT8)((data & 0xbf) | 0x40);			// set zero-IF
        nim_s3821_write(dev, 0x13, &data, 1);

        // set initial frequency offset to 0
        data_arr[0] = 0x0;
        data_arr[1] = 0x0;
        data_arr[2] = 0x0;
        nim_s3821_write(dev, 0x14, data_arr, 3);

        nim_s3821_read(dev, 0x13, &data, 1);
        data = (UINT8)(data & 0xf0);
        nim_s3821_write(dev, 0x13, &data, 1);

        NIM_S3821_DEBUG("zero if setting (1) \n");
    }

    //joey, 20111101, Set COFDM ADC to 54M sampling clock in ADC format controller register.
    nim_s3821_read(dev, 0x01, &data, 1);
    data  = data | 0x01;
    nim_s3821_write(dev, 0x01, &data, 1);

    //In system ADC PLL mode selection.  RXPLL_PD.
    
    data_tmp = *(volatile UINT32 *)0xb80000b4;
    data_tmp = data_tmp & 0xffffefff;
    *(volatile UINT32 *)0xb80000b4 = data_tmp;
    osal_delay(10);

	return ret;
	
}



INT32 nim_s3821_init_dvbt_isdbt(struct nim_device *dev)
{
    struct nim_s3821_private *priv_mem = (struct nim_s3821_private *)(dev->priv);
    struct COFDM_TUNER_CONFIG_API *config_info = &(priv_mem->tuner_control);

    UINT8 data = 0;
	INT32 ret=0;
	
    if (priv_mem->cofdm_type == ISDBT_TYPE && (priv_mem->tuner_control.config_data.memory != NULL))
    {
        UINT8 tf_addr[4]= {0};

        tf_addr[0] = ((UINT32)(priv_mem->tuner_control.config_data.memory)) & 0xff;
        tf_addr[1] = (((UINT32)(priv_mem->tuner_control.config_data.memory)) >> 8) & 0xff;
        tf_addr[2] = (((UINT32)(priv_mem->tuner_control.config_data.memory)) >> 16) & 0xff;
        tf_addr[3] = (((UINT32)(priv_mem->tuner_control.config_data.memory)) >> 24) & 0x1f;

        nim_s3821_write(dev, 0x110, tf_addr, 4);
    }

    //check the demod type
    if ((DVBT_TYPE == priv_mem->cofdm_type) || (DVBT2_COMBO == priv_mem->cofdm_type))
    {
        nim_s3821_read(dev, 0x12, &data, 1);
        data = (UINT8)((data & 0x7f) | 0x80);			// set the demod type to DVB type
        nim_s3821_write(dev, 0x12, &data, 1);
    }
    else // isdbt.
    {
        nim_s3821_read(dev, 0x12, &data, 1);
        data = (UINT8)((data & 0x7f) | 0x00);			// set the demod type to ISDBT type
        nim_s3821_write(dev, 0x12, &data, 1);
    }

    //enable ts_out.
    //	nim_s3821_read(dev, 0xf8, &data, 1);
    //	data = (UINT8)(data&0x7f);			//
    //	nim_s3821_write(dev, 0xf8, &data, 1);

    //Please care about 27M and 54M ADC sampling.
    //ADC set to 54M by 36M/44M IF or function mode set.
    if ((config_info->config_data.cofdm_config & 0x100) == 0x000)
    {
       ret=nim_s3821_init_27madc(dev,config_info);
	   if(ret!=RET_CONTINUE)
	   {
	   	   return ret;
	   }
	   
    }
    else // 54M ADC.
    {
       ret=nim_s3821_init_54madc(dev,config_info);
	   if(ret!=RET_CONTINUE)
	   {
	   	   return ret;
	   }
    }


    //RF-AGC setting.
    if ((config_info->config_data.cofdm_config & 0x04) == 0x04) //rf-agc enabled.
    {
        nim_s3821_read(dev, 0x2a, &data, 1);
        data = (UINT8)((data & 0xde) | 0x20);//enable rf-agc.
        if ((config_info->config_data.cofdm_config & 0x08) == 0x08) //direct portion.
        {
            data = (UINT8)(data | 0x01);
        }
        else //negative portion.
        {
            data = (UINT8)(data | 0x00);
        }
        nim_s3821_write(dev, 0x2a, &data, 1);

        data = config_info->config_data.RF_AGC_MAX;//RF AGC_MAX
        nim_s3821_write(dev, 0x26, &data, 1);

        data = config_info->config_data.RF_AGC_MIN;//RF AGC Min
        nim_s3821_write(dev, 0x27, &data, 1);
        NIM_S3821_DEBUG("zero if setting (2) \n");
    }
    else//rf-agc disable.
    {
        nim_s3821_read(dev, 0x2a, &data, 1);
        data = (UINT8)(data & 0xdf);//disable rf-agc.
        nim_s3821_write(dev, 0x2a, &data, 1);
    }

    // enable RF RSSI ADC.
    if ((config_info->config_data.cofdm_config & 0x20) == 0x20)
    {
        nim_s3821_read(dev, 0x2b, &data, 1);
        data |= 0x40;			// enable RSSI
        nim_s3821_write(dev, 0x2b, &data, 1);
    }

    //IF-AGC setting.
    nim_s3821_read(dev, 0x2a, &data, 1);
    data = (UINT8)((data & 0xfd) | 0x02);//enable if-agc.
    nim_s3821_write(dev, 0x2a, &data, 1);

    data = config_info->config_data.IF_AGC_MAX;//IF AGC_MAX
    nim_s3821_write(dev, 0x28, &data, 1);

    data = config_info->config_data.IF_AGC_MIN;//IF AGC Min
    nim_s3821_write(dev, 0x29, &data, 1);

    //ADC format setting. For Soc, internal AC, unsigned mode. And for Low-IF, use Q-path.
    // check zero IF system
    nim_s3821_read(dev, 0x01, &data, 1);
    if ((config_info->config_data.cofdm_config & 0x10) == 0x00) // 0: low-if, 1: zero-if..
    {
        data = data | 0x08;
    }
    else
    {
        data = data | 0x08;
    }
    nim_s3821_write(dev, 0x01, &data, 1);

    //joey, 20111018, clear all INT_MASK.
    data = 0x00;
    nim_s3821_write(dev, 0x03, &data, 1);

    //20120426, joey. for s3821, C3811, distinguish. for C3811, all use default setting.
    nim_s3821_read(dev, 0x3b, &data, 1);
    if (MAGIC_CONST_1 == data) // check if s3821/C3811.
    {
        nim_s3821_read(dev, 0x147, &data, 1);
        if (MAGIC_CONST_0 == data) // denote for s3811.
        {
           NIM_S3821_DEBUG("[%s]line=%d,here!\n",__FUNCTION__,__LINE__);
        }
        else // denote for C3811, re-fine some register setting after ES verification.
        {
            //20120524, joey, for andy/linda shorten the TMCC/TPS fail counter.
            data = 0x00; // the shortest value.
            nim_s3821_write(dev, 0x17, &data, 1);

            //20120503, joey, for linda doppler detect function.
            data = 0x60; // fine tune the threshold.
            nim_s3821_write(dev, 0xc4, &data, 1);
        }
    }

    //joey, 20130710, for doppler detection threshold modification according to Algorithm.
    //Doppler will be detected under S3821 DVB-T and ISDB-T long echo when RF ON/OFF, 
    //which will degrade the performance.
    data = 0x75;
    nim_s3821_write(dev, 0xc4, &data, 1);

    //joey, 20130723, for TF memory latency value setting.
    nim_s3821_read(dev, 0x11a, &data, 1);
    data |= 0x10;
    nim_s3821_write(dev, 0x11a, &data, 1);

    //joey, 20130828, for TF memory latency value setting. change from 0x90 to 0x50.
    data = 0x50;
    nim_s3821_write(dev, 0x11c, &data, 1);

    //check the demod type
    if ((DVBT_TYPE == priv_mem->cofdm_type) || (DVBT2_COMBO == priv_mem->cofdm_type))
    {
        //20130731, joey. change cci post value for Nordig SQI requirement.
        data = 0xa5;
        nim_s3821_write(dev, 0xcd, &data, 1);

        nim_s3821_read(dev, 0xc6, &data, 1);
        data = ((data & 0x8f) | 0x30);
        nim_s3821_write(dev, 0xc6, &data, 1);
    }

    return SUCCESS;

}

INT32 nim_s3821_init_dvbt2(struct nim_device *dev)
{
    struct nim_s3821_private *priv_mem = (struct nim_s3821_private *)(dev->priv);
    struct COFDM_TUNER_CONFIG_API *config_info = &(priv_mem->tuner_control);

    UINT8 data = 0;
    UINT8 data_arr[4]= {0};

    if (priv_mem->tuner_control.config_data.memory != NULL)
    {
        UINT8 tf_addr[4]= {0};

        tf_addr[0] = ((UINT32)(priv_mem->tuner_control.config_data.memory)) & 0xff;
        tf_addr[1] = (((UINT32)(priv_mem->tuner_control.config_data.memory)) >> 8) & 0xff;
        tf_addr[2] = (((UINT32)(priv_mem->tuner_control.config_data.memory)) >> 16) & 0xff;
        tf_addr[3] = (((UINT32)(priv_mem->tuner_control.config_data.memory)) >> 24) & 0x1f;

        nim_s3821_write(dev, 0x102, tf_addr, 4);
    }

    //Only support 27M ADC. no zero-if.
    if ((config_info->config_data.cofdm_config & 0x100) == 0x000)
    {
        //The below is just for 27M now.
        // check zero IF system
        if ((config_info->config_data.cofdm_config & 0x10) == 0x00) // 0: low-if, shutdown Q path, 1: zero-if, still on.
        {
            if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_4520) && 
	        (config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_4620)) // for mxl5007, 4.57M., low low-IF.
            {
                data_arr[0] = 0x32;
                data_arr[1] = 0x49;
                data_arr[2] = 0xb5;

                nim_s3821_write(dev, 0x61, data_arr, 3);
                nim_s3821_read(dev, 0x64, &data, 1);
                data = (UINT8)((data & 0xf0) | 0x02);
                nim_s3821_write(dev, 0x64, &data, 1);
            }
			else if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_4950) && 
			(config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_5050)) // for tmp 5.00M., low low-IF.
			{
				data_arr[0] = 0xbe;
				data_arr[1] = 0x84;
				data_arr[2] = 0xf6;
			    nim_s3821_write(dev, 0x61, data_arr, 3);
				nim_s3821_read(dev, 0x64, &data, 1);
				data = (UINT8)((data&0xf0)|0x02);
				nim_s3821_write(dev, 0x64, &data, 1);
			}
            else if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_3950) && 
	           (config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_4050)) // for mxl136, 4.00M., low low-IF.
            {
                NIM_S3821_DEBUG("[%s]line=%d,here!\n",__FUNCTION__,__LINE__);
            }
            else if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_3520) && 
	          (config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_3620)) // for R820T 6M, 3.57M, low low-IF.
            {
                NIM_S3821_DEBUG("[%s]line=%d,here!\n",__FUNCTION__,__LINE__);
            }
            else if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_35500) && 
	         (config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_36500)) // for TD1611, 36M low-IF.
            {
                NIM_S3821_DEBUG("[%s]line=%d,here!\n",__FUNCTION__,__LINE__);
                //use (36 -27) = 9M as the frequency been sampled.
            }
            else if ((config_info->tuner_config.w_tuner_if_freq >= MAGIC_CONST_43500) && 
	               (config_info->tuner_config.w_tuner_if_freq <= MAGIC_CONST_44500)) // for sharp6401, 44M low-IF.
            {
                 NIM_S3821_DEBUG("[%s]line=%d,here!\n",__FUNCTION__,__LINE__);
                //use (44-27) = 17M as the frequency been sampled.

            }
            else
            {
                NIM_S3821_DEBUG("Error, un-support tuner type!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
                return ERR_FAILUE;
            }

        }
        else //zero-if, not support.
        {
            NIM_S3821_DEBUG("Error, not support zero if setting!\n");
        }

    }
    else // 54M ADC.
    {
        NIM_S3821_DEBUG("Error, un-support 54M  ADC!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
    }


    //RF-AGC setting.
    if ((config_info->config_data.cofdm_config & 0x04) == 0x04) //rf-agc enabled.
    {
        nim_s3821_read(dev, 0x82, &data, 1);
        data = (UINT8)((data & 0xfb) | 0x40);//enable rf-agc.
        nim_s3821_write(dev, 0x82, &data, 1);
        nim_s3821_read(dev, 0x7d, &data, 1);
        if ((config_info->config_data.cofdm_config & 0x08) == 0x08) //direct portion.
        {
            data = (UINT8)((data & 0xfd) | 0x02);
        }
        else //negative portion.
        {
            data = (UINT8)((data & 0xfd) | 0x00);
        }
        nim_s3821_write(dev, 0x7d, &data, 1);
        data = config_info->config_data.RF_AGC_MAX;//RF AGC_MAX
        nim_s3821_write(dev, 0x7e, &data, 1);

        data = config_info->config_data.RF_AGC_MIN;//RF AGC Min
        nim_s3821_write(dev, 0x7f, &data, 1);
        NIM_S3821_DEBUG("zero if setting (2) \n");

    }
    else//rf-agc disable.
    {
        nim_s3821_read(dev, 0x82, &data, 1);
        data = (UINT8)(data & 0xfb);//disable rf-agc.
        nim_s3821_write(dev, 0x82, &data, 1);
    }

    // enable RF RSSI ADC.
    //IF-AGC setting.
    nim_s3821_read(dev, 0x82, &data, 1);
    data = (UINT8)((data & 0xfe) | 0x01);//enable if-agc.
    nim_s3821_write(dev, 0x82, &data, 1);
    data = config_info->config_data.IF_AGC_MAX;//IF AGC_MAX
    nim_s3821_write(dev, 0x80, &data, 1);
    data = config_info->config_data.IF_AGC_MIN;//IF AGC Min
    nim_s3821_write(dev, 0x81, &data, 1);

    //ADC format setting. For Soc, internal AC, unsigned mode. And for Low-IF, use Q-path.
    // check zero IF system
    nim_s3821_read(dev, 0x02, &data, 1);
    if ((config_info->config_data.cofdm_config & 0x10) == 0x00) // 0: low-if, 1: zero-if..
    {
        data = data | 0x14;
    }
    else
    {
        data = data | 0x10;
    }
    nim_s3821_write(dev, 0x02, &data, 1);

    //joey, 20130625, clear all INT_MASK.
    //data = 0x00;
    //nim_s3821_write(dev, 0x03, &data, 1);

    //joey, 20130705, for TIC control register, by Herbie.
    data = 0x28;
    nim_s3821_write(dev, 0x1bc, &data, 1);

    data = 0x03;
    nim_s3821_write(dev, 0x1bd, &data, 1);

    //joey, 20130729, enable T2 watch-dog.
    nim_s3821_read(dev, 0x02, &data, 1);
    data = data | 0x01;
    nim_s3821_write(dev, 0x02, &data, 1);

    return SUCCESS;

}


void nim_s3821_main_thread(UINT32 param1, UINT32 param2)
{
    //-------added by randy for demux error monitor-----

    UINT32 cur_time = 0;
	UINT32 dd_time = 0;
    UINT8 ber_vld = 0;
    UINT32 m_vbber = 0;
	UINT32 m_per  = 0;
    UINT8 iflock = 0;
    UINT8 tmp_data[4] ={0};
    UINT8 tmpdata = 0;
    UINT8 unlock_cnt = 0;
	UINT32 cci_post_end_time = 0;
    UINT32 tmp_cci_info = 0;
    UINT8 tmp_cci_cnt = 0;
    UINT8 cur_mode = 0;
	
    struct nim_device *dev = (struct nim_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_NIM);
    PNIM_S3821_PRIVATE priv = (PNIM_S3821_PRIVATE)dev->priv;

    dd_time = osal_get_tick();
    cci_post_end_time = osal_get_tick();

    while (1) //timer , message, interrupt, flag.
    {
        nim_s3821_get_cur_mode(dev, &cur_mode);
        if (MODE_DVBT2 == cur_mode)
        {
            if (mon_ifft_en == TRUE)
            {
                nim_s3821_monitor_ifft_result(dev);
                mon_ifft_en = FALSE;
            }

            cur_time = osal_get_tick();
            if (cur_time - dd_time > MAGIC_CONST_300)
            {
                nim_s3821_monitor_ber(dev, &ber_vld, &m_vbber, &m_per);
                nim_s3821_dvbt2_monitor_cnr(dev, NULL);
                if (1 == mon_status_print)
                {
                    nim_s3821_mon_internal_status(dev, NULL);
                }

                //joey, 20130704, for DVBT2 TIC interrupt print.
                nim_s3821_read(dev, 0x96, &tmpdata, 1);
                nim_s3821_read(dev, 0x67, &iflock, 1);
                //NIM_S3821_DEBUG("CR1d=%x-- FSM state\n",iflock);

                if ((iflock & 0x0f) < 0x0a)
                {
#if (CHIP_VERIFY_FUNC == SYS_FUNC_ON) //for S3811 chip test support.
                    unlock_cnt += 1;
                    //joey, 20111019. for FSM print.
                    if (unlock_cnt > MAGIC_CONST_4)
                    {
                        if (1 == dem_print)
                        {
                            //libc_printf("BER= 0,PER= 0,Unlock, FSM = 0x%2x;\n", iflock);
                            libc_printf("BER= 0,PER= 0,Unlock, FSM = 0x%2x; CR96 = 0x%2x;\n", iflock, tmpdata);
                        }

                        //sprintf(osd_buffer, "Unlock!\n"); 
                        //osd_printf(osd_buffer);	
                        unlock_cnt = 0;
                    }

#endif
                }
                else // (iflock & S3811_COFDM_STATUS_LOCK) == TRUE
                {
                    if (ber_vld == 1)
                    {
#if (CHIP_VERIFY_FUNC == SYS_FUNC_ON) //for S3811 chip test support.
                        if (1 == dem_print)
                        {
                            //libc_printf("BER=%6d,PER=%d,Locked;\n",m_vbber,m_per);
                            libc_printf("BER=%6d,PER=%d,Locked; CR96 = 0x%2x;\n", m_vbber, m_per, tmpdata);
                        }
                        //sprintf(osd_buffer, "B=%6d,  P=%d\n", m_vbber,m_per); 
                        //osd_printf(osd_buffer);// Note: this and above code indicate print BerPer On TV screen

#endif

                    }
                    //joey, 20120831, add for CCI info print.
                    if (1 == cci_info_print)
                    {
                        tmp_cci_cnt += 1;
                        if (tmp_cci_cnt > MAGIC_CONST_6)
                        {
                            nim_s3821_read(dev, 0x19e, tmp_data, 4);
                            tmp_cci_info = (tmp_data[3] & 0x3f) << 24 | (tmp_data[2]) << 16 | 
			                       (tmp_data[1]) << 8 | tmp_data[0];
                            libc_printf("CCI_ABNORM_NUM_1: %d !\n", tmp_cci_info);

                            nim_s3821_read(dev, 0x1a2, tmp_data, 4);
                            tmp_cci_info = (tmp_data[3] & 0x3f) << 24 | (tmp_data[2]) << 16 | 
			                    (tmp_data[1]) << 8 | tmp_data[0];
                            libc_printf("CCI_ABNORM_NUM_2: %d !\n", tmp_cci_info);

                            nim_s3821_read(dev, 0x1a6, tmp_data, 4);
                            tmp_cci_info = (tmp_data[3] & 0x3f) << 24 | (tmp_data[2]) << 16 | 
			                  (tmp_data[1]) << 8 | tmp_data[0];
                            libc_printf("CCI_ABNORM_NUM_3: %d !\n", tmp_cci_info);

                            nim_s3821_read(dev, 0x1aa, tmp_data, 2);
                            tmp_cci_info = (tmp_data[1] & 0x07) << 8 | tmp_data[0];
                            libc_printf("CCI_ABNORM_MAX: %d !\n", tmp_cci_info);

                            nim_s3821_read(dev, 0x1ac, tmp_data, 2);
                            tmp_cci_info = (tmp_data[1] & 0x07) << 8 | tmp_data[0];
                            libc_printf("CCI_ABNORM_MEAN: %d !\n", tmp_cci_info);

                            libc_printf("CCI_POST_FIND_FLG: %d !\n", ((tmp_data[1] >> 3) & 0x01));
                            libc_printf("CCI_POST_FIND_FLG_1: %d !\n", ((tmp_data[1] >> 4) & 0x01));
                            libc_printf("CCI_POST_FIND_FLG_2: %d !\n", ((tmp_data[1] >> 5) & 0x01));
                            libc_printf("CCI_POST_FIND_FLG_3: %d !\n", ((tmp_data[1] >> 6) & 0x01));

                            nim_s3821_read(dev, 0x1ae, tmp_data, 2);
                            tmp_cci_info = (tmp_data[1] & 0x7f) << 8 | tmp_data[0];
                            libc_printf("CCI_ABNORM_CARR_NUM: %d !\n", tmp_cci_info);

                            tmp_cci_cnt = 0;
                        }
                    }
                }
                dd_time = cur_time;
            }
            //joey, 20130719, for T2 256Qam, 1/2CR CNR issue, change CR119 proc.
            nim_s3821_ldpc_proc(dev);
        }
        else if ((MODE_DVBT == cur_mode) || (MODE_ISDBT == cur_mode))
        {
             nim_s3821_dvbt_isdbt_proc(dev,priv,&unlock_cnt);

        }
        osal_task_sleep(1);
    }

}


