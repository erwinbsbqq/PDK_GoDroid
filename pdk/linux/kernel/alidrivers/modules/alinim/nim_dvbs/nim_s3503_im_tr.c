/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_s3503_im_02.c
*
*    Description:  s3503 im layer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#include "nim_s3503.h"

#define LEN_VAL_8       8
#define CNT_VAL_3       3
#define CUR_MAX_ITER_50 50
#define CUR_MAX_ITER_35 35
#define INTERVAL_CNT_10 10
#define INTERVAL_CNT_16 16

static UINT16 snr_max = 0;
static UINT16 snr_min = 0;
static UINT32 ber_sum = 0;  // store the continuous ber
static UINT32 last_ber_sum = 0;
static UINT32 cur_ber_sum = 0;
static UINT32 ber_thres = 0x180;
static UINT8 cur_max_iter = 50; // static variable can not auto reset at channel change???
static UINT8 snr_bak = 0;
static UINT8 last_max_iter = 50;
static INT32 cnt3 = 0;

static INT32  curr_snr_idx = 0;
static UINT16 snr_lpf = 0;
static INT32 interval_cnt = 0;


#ifdef HW_ADPT_CR_MONITOR
static UINT8 snr_dbx10_tab[SNR_TAB_SIZE] = 
{
     30, 40, 50,60, 65, 70, 75, 80, 85, 90, \
     95, 100, 105, 110, 115, 120, 140, 160, 180
};
#endif

// 1st column : >=thr, switch to lower snr, also use for initial search
// 2nd column: <=thr, switch to high snr
static const UINT16 snr_thr[SNR_TAB_SIZE * 2] =
{
    956, 1060, // 3
    837, 893, // 4
    713, 775, // 5
    690, 725, // 6,  8SPK start**
    630, 677, // 6.5
    573, 615, // 7
    520, 559, // 7.5
    470, 508, // 8
    426, 460, // 8.5
    386, 415, // 9    // 16 APSK start
    347, 376, // 9.5
    313, 339, // 10
    281, 303, // 10.5 //
    253, 275,  // 11
    228, 246,  // 11.5
    205, 222, // 12   **
    113, 144, // 14
    74, 91, // 16
    0, 60 // 18
};

// head gain have 3 bit, in SW Tab, using 4bit for better understanding
//{IRS[4:0],  PRS[3:0], HEAD_GAIN[3:0],FRAC[3:0],}
// for 8PSK 3/5  add by grace
static const UINT32 cr_para_8psk_3f5[PSK8_TAB_SIZE] =
{
    0x12930,  //6
    0x12930,  //6.5
    0x12900,  //7
    0x11800,  //7.5
    0x1070a,  //8
    0x10705,  //8.5
    0x10705,  //9
    0x10705,  //9.5
    0x10705,  //10
    0x10705,  // 10.5
    0x10705,  // 11
    0x10705,  // 11.5
    0x10700,  // 12
    0x10700   // 14
};

// for 8PSK other coderate 9/10,8/9,5/6,3/4,2/3  add by grace
static const UINT32 cr_para_8psk_others[PSK8_TAB_SIZE] =
{
    0x12930, // 6
    0x12910, // 6.5
    0x12900, // 7
    0x11900, // 7.5
    0x11800, // 8
    0x11800, // 8.5
    0x1070a, // 9
    0x1070a, // 9.5
    0x10705, // 10
    0x10705,  // 10.5
    0x1070a, // 11
    0x10705, // 11.5
    0x10700, // 12
    0x10700  // 14
};


/*****************************************************************************
* INT32 nim_reg_read(struct nim_device *dev, UINT8 bmemadr, UINT8 *pdata, UINT8 blen)
* Description: S3501 register read function
*                   Can read 8-byte one time and blen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
INT32 nim_s3503_read(struct nim_device *dev, UINT16 bmemadr, UINT8 *pdata, UINT8 blen)
{
    UINT8 i = 0;

    if(dev == NULL || pdata == NULL)
    {
	    return RET_FAILURE;
	}	
    pdata[0] = bmemadr;
    if (blen > LEN_VAL_8)
    {
        nim_s3503_set_err(dev);
        return ERR_FAILUE;
    }

    for (i = 0; i < blen; i++)
    {
        pdata[i ] = NIM_GET_BYTE(NIM_S3503_BASE_ADDR + bmemadr + i);
    }

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_reg_write(struct nim_device *dev, UINT8 bmemadr, UINT8 *pdata, UINT8 blen)
* Description: S3501 register write function
*                   Can write 8-byte one time and blen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
INT32 nim_s3503_write(struct nim_device *dev, UINT16 bmemadr, UINT8 *pdata, UINT8 blen)
{
    UINT8 i = 0;

    if(dev == NULL || pdata == NULL)
    {
	    return RET_FAILURE;
	}	
    if (blen > LEN_VAL_8)
    {
        nim_s3503_set_err(dev);
        return ERR_FAILUE;
    }

    for (i = 0; i < blen; i++)
    {
        NIM_SET_BYTE(NIM_S3503_BASE_ADDR + bmemadr + i, pdata[i]);
    }

    return SUCCESS;
}




void nim_s3503_set_cr_new_value(struct nim_device *dev,UINT8 tabid,UINT8 cellid,UINT32 value)
{
    UINT8 data = 0;
    UINT16 tabval_14_0 = 0;
    UINT16 tabval_29_15 = 0;
    UINT8 datarray[2]={0};

    if(dev == NULL)
    {
	    return ;
	}	
    //step 1:
    data = (tabid << 5) | (cellid & 0x1f);
    nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1 );

    //step 2:
    tabval_14_0 = (UINT16)((value  & 0x7fff));    // and disable the CR_PARA_WE to avoid disturb with seen's adpt
    datarray[0] = (UINT8)tabval_14_0;
    datarray[1] = ((UINT8)(tabval_14_0 >> 8)) & 0x7f;
    nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

    //step 3:
    tabval_29_15 = (UINT16)(value >> 15);
    datarray[0] = (UINT8)(tabval_29_15);
    datarray[1] = (UINT8)(tabval_29_15 >> 8) | 0x80; // enable CR_PARA_WE_2
    nim_reg_write(dev, R130_CR_PARA_DIN, datarray, 2);

}

/*****************************************************************************
* INT32 nim_s3503_set_dynamic_power(struct nim_device *dev, UINT32 *RsUbc)
* Get bit error ratio
*
* Arguments:
* Parameter1:
* Key word: power_ctrl
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_set_dynamic_power(struct nim_device *dev, UINT8 snr)
{
    UINT8 coderate = 0;
    UINT32 ber = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	

    if (cnt3 >= CNT_VAL_3)
    {
        last_ber_sum = cur_ber_sum;
        cur_ber_sum = ber_sum;
        cnt3 = 0;
        ber_sum = 0;
    }
    nim_s3503_get_ber(dev, &ber);
    nim_s3503_reg_get_code_rate(dev, &coderate);
    ber_sum += ber;
    cnt3 ++;
    if (coderate < 0x04)      // 1/4 rate
    {
        ber_thres = 0x120;
    }
    else
    {
        ber_thres = 0x70;
    }
    if (cur_max_iter == CUR_MAX_ITER_50)
    {
        if (ber_sum >= ber_thres)
        {
            if (snr > snr_bak)
            {
            snr_bak = snr;
            }
            cur_max_iter -= 15;
        }
    }
    else if (cur_max_iter < 50)
    {
        if (((cur_ber_sum + 0x80) < last_ber_sum) || (snr > (snr_bak + 2)))
        {
            cur_max_iter += 15;
            snr_bak = 0;
            cnt3 = 0;
            ber_sum = 0;
            last_ber_sum = 0;
            cur_ber_sum = 0;
        }
        else if (ber_sum > 3 * ber_thres)
        {
            cur_max_iter -= 15;
            if ((coderate < 0x04) && (cur_max_iter < CUR_MAX_ITER_35))
            {
            cur_max_iter = 35;
            }
            else if (cur_max_iter < 20)
            {
            cur_max_iter = 20;
            }
        }
    }

    if (cur_max_iter != last_max_iter)
    {
        NIM_PRINTF("----change cur_max_iter to %d----\n\n", cur_max_iter);
        nim_reg_write(dev, R57_LDPC_CTRL, &cur_max_iter, 1);
        last_max_iter = cur_max_iter;
    }
    return SUCCESS;
}




/*****************************************************************************
* static INT32 nim_sw_snr_rpt(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_cr_sw_snr_rpt(struct nim_device *dev)
{
    UINT8 data = 0;

    UINT16 var = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	

    interval_cnt++;
    if(interval_cnt == INTERVAL_CNT_10)
    {
        nim_reg_read(dev, RBC_SNR_RPT2, &data, 1);
        if (data & 0x80) // SNR estimation data ready
        {
            var = (data & 0x1f) << 6;
            nim_reg_read(dev, RBB_SNR_RPT1, &data, 1); // read SNR LSB
            var += (data >> 2) & 0x03f;

            if(s3503_snr_initial_en)
            {
                snr_max = var;
                snr_min = var;
                s3503_snr_initial_en = 0;
            }

            if(var > snr_max)
            {
            snr_max = var;
            }
            if(var < snr_min)
            {
            snr_min = var;
            }

            data = 0x00;
            nim_reg_write(dev, RBC_SNR_RPT2, &data, 1); // clear SNR ready flag

            NIM_PRINTF("*** SNR = %u min=%u max=%u\n", var, snr_min, snr_max);
        }

        interval_cnt = 0;
    }

    return SUCCESS;
}



/*****************************************************************************
* static INT32 nim_sw_adaptive_cr(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_cr_sw_adaptive(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT16 var = 0;
    INT8 snr_chg = 0;
    INT16 i = 0;
    UINT16 cr_data = 0;
    UINT8 frac = 0;
    UINT8 prs = 0;
    UINT8 irs = 0;
    UINT32 *cr_para=NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	

    //ByteDat = ReadMemB(0x180030bc);
    nim_reg_read(dev, RBC_SNR_RPT2, &data, 1);
    if (data & 0x80) // SNR estimation data ready
    {
        var = (data & 0x0f) << 6;
        //ByteDat = ReadMemB(0x180030bb);
        nim_reg_read(dev, RBB_SNR_RPT1, &data, 1); // read SNR LSB
        var += (data >> 2) & 0x03f;

        // slide filter
        if(s3503_snr_initial_en)
        {
        snr_lpf = var;
        }
        else
        {
            snr_lpf = (snr_lpf * 7 + var) >> 2;
            snr_lpf += 1;
            snr_lpf >>= 1;
        }
        interval_cnt++;
        if(interval_cnt == INTERVAL_CNT_16)
        {
            interval_cnt = 0;
            //NIM_PRINTF("*** SNR = %u \r\n", snr_lpf);

        }

        if(s3503_snr_initial_en) // for first SNR data
        {
            curr_snr_idx = SNR_TAB_SIZE - 1;
            for(i = SNR_TAB_SIZE - 1; i >= 0; i--)
            {
                if(var >= snr_thr[i * 2])
                {
            curr_snr_idx = i;
                }
                else
                {
            break;
                }
            }

            s3503_snr_initial_en = 0;
            snr_chg = 1;
            snr_lpf = var;
        }
        else if(curr_snr_idx > 0 && snr_lpf >= snr_thr[2 * (curr_snr_idx - 1)] && (interval_cnt == 0))
        {   // switch to lower snr
            curr_snr_idx--;
            snr_chg = 1;
        }
        else if(curr_snr_idx < SNR_TAB_SIZE - 1 && snr_lpf <= snr_thr[2 * curr_snr_idx + 3] && \
            (interval_cnt == 0)) // to higher snr
        {
            curr_snr_idx++;
            snr_chg = 1;
        }

        if(snr_chg)
        {
            cr_para = (UINT32 *)cr_para_8psk_3f5;
            //if((LastModCode>>2)==12) // 8PSK 3/5
            //    cr_para = CR_PARA_TAB[0];
            //else
            //    cr_para = CR_PARA_TAB[1];

            cr_data = cr_para[curr_snr_idx];
            frac = cr_data & 0x0f;
            prs = (cr_data >> 4) & 0x0f;
            irs = (cr_data >> 8) & 0x1f;
            data = (((irs & 0x03) << 4) | frac) << 2;
            //WriteMemB(0x180030bb, data);
            nim_reg_write(dev, RBB_SNR_RPT1, &data, 1);

            data = ((irs >> 2) << 4) | prs | 0x80;
            //WriteMemB(0x180030bc, data);
            nim_reg_write(dev, RBC_SNR_RPT2, &data, 1);

            data = 0xe8;
            //WriteMemB(0x1800300e, 0xe8); // enable cr para update
            nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);


            data = (curr_snr_idx & 0x01) ? 5 : 0;
            NIM_PRINTF("--->switch to %d.%d dB Setting \n", 6 + curr_snr_idx / 2, data);
        }

        data = 0x00;
        nim_reg_write(dev, RBC_SNR_RPT2, &data, 1); // clear SNR ready flag

    }

    return SUCCESS;
}



/*****************************************************************************
* static INT32 nim_s3503_cr_tab_init(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_cr_tab_init(struct nim_device *dev)
{
    UINT8 tabid = 0;
    UINT8 cellid = 0;
    UINT32 tabval = 0;
    UINT32 tabvaltemp = 0;
    UINT8 data = 0;
    UINT8 datarray[2]={0};

    const unsigned others_tab[6] = {0x09,    0x00,    0x002,   0x5fa,    0x000,   0x00};
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	

#ifdef HW_ADPT_CR_MONITOR
    ADPT_CR_PRINTF("CR TAB Initialization Begin \n");
#endif
    data = 0xe1; // CR Tab init en
    nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);

    // Write SNR threshold low, tabid=0~1;
    for(cellid = 0; cellid < SNR_TAB_SIZE; cellid++)
    {
        tabid =  (cellid >= 16) ? 1 : 0;
        data = (tabid << 4) | (cellid & 0x0f);
        nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
        tabval = snr_thr[2 * cellid]  | 0x8000;
        datarray[0] = (UINT8)(tabval & 0x0ff);
        datarray[1] = (UINT8)((tabval >> 8) & 0x0ff);
        nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);
    }

    // Write SNR threshold high, tabid=2~3;
    for(cellid = 0; cellid < SNR_TAB_SIZE; cellid++)
    {

        tabid =  (cellid >= 16) ? 3 : 2;
        data = (tabid << 4) | (cellid & 0x0f);
        nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
        tabval = snr_thr[2 * cellid + 1] | 0x8000;
        datarray[0] = (UINT8)(tabval & 0x0ff);
        datarray[1] = (UINT8)((tabval >> 8) & 0x0ff);
        nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);
    }

    for(cellid = 0; cellid < PSK8_TAB_SIZE; cellid++)
    {
        data = (4 << 4) | cellid;
        nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
        tabval = cr_para_8psk_3f5[cellid];
        tabvaltemp = (tabval & 0x7c) << 7;
        tabval = (tabval >> 8) | tabvaltemp | 0x8000; // to HW format;
        datarray[0] = (UINT8)(tabval & 0x0ff);
        datarray[1] = (UINT8)((tabval >> 8) & 0x0ff);
        nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);
    }

    // 8PSK 2/3 ~ 9/10,  3/4~9/10 share one group
    for(tabid = 5; tabid <= 6; tabid++)
    {
        for(cellid = 0; cellid < PSK8_TAB_SIZE; cellid++)
        {
            data = (tabid << 4) | cellid;
            nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
            tabval = cr_para_8psk_others[cellid];
            tabvaltemp = (tabval & 0x7c) << 7;
            tabval = (tabval >> 8) | tabvaltemp | 0x8000; // to HW format;
            datarray[0] = (UINT8)(tabval & 0x0ff);
            datarray[1] = (UINT8)((tabval >> 8) & 0x0ff);
            nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);
        }
    }

    // QPSK: tabid=7~9, need parameter table later
    for(tabid = 7; tabid <= 9; tabid++)
    {
        for(cellid = 0; cellid < QPSK_TAB_SIZE; cellid++)
        {
            data = (tabid << 4) | cellid;
            nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
            tabval = 0x129 | 0x8000;
            datarray[0] = (UINT8)(tabval & 0x0ff);
            datarray[1] = (UINT8)((tabval >> 8) & 0x0ff);
            nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);
        }
    }

    // 16APSK: tabid=10~12
    for(tabid = 10; tabid <= 12; tabid++)
    {
        for(cellid = 0; cellid < APSK16_TAB_SIZE; cellid++)
        {
            data = (tabid << 4) | cellid;
            nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
            tabval = 0x129 | 0x8000;
            datarray[0] = (UINT8)(tabval & 0x0ff);
            datarray[1] = (UINT8)((tabval >> 8) & 0x0ff);
            nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);
        }
    }

    // ON/OFF Table
    for(cellid = 0; cellid < 6; cellid++)
    {
        data = 0xf0 | cellid;
        nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
        tabval = others_tab[cellid] | 0x8000;
        datarray[0] = (UINT8)(tabval & 0x0ff);
        datarray[1] = (UINT8)((tabval >> 8) & 0x0ff);
        nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);
    }

    data = 0xe0; // CR Tab init off
    nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);

#ifdef HW_ADPT_CR_MONITOR
    ADPT_CR_PRINTF("CR TAB Initialization Done \n");
#endif
    return SUCCESS;
}

#ifdef HW_ADPT_CR_MONITOR
/*****************************************************************************
* static void nim_s3503_cr_adaptive_monitor(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
void nim_s3503_cr_adaptive_monitor(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT8 cr_prs = 0;
    UINT8 snr_idx = 0;
    UINT8 cr_irs = 0;
    UINT8 cr_frac = 0;
    UINT8 head_gain = 0;
    UINT32 rdata = 0;
	
    if(dev == NULL)
    {
	    return;
	}	

    if(interval_cnt == 0)
    {
        UINT8 data1 = 0xe6;

        nim_reg_write(dev, 0x0e, &data1, 1);
        //ByteDat = ReadMemB(0x180030bc);
        nim_reg_read(dev, RBB_SNR_RPT1, &data, 1);
        rdata = data;
        nim_reg_read(dev, RBC_SNR_RPT2, &data, 1);
        rdata |= data << 8;
        nim_reg_read(dev, R26_TR_LD_LPF_OPT, &data, 1);
        rdata |= data << 16;
        //if(rdata != old_cr_para)
        //{
        //    old_cr_para = rdata;
        cr_prs = (UINT8)((rdata >> 9) & 0xf);
        cr_irs = (UINT8)((rdata >> 4) & 0x1f);
        cr_frac = (UINT8)((rdata >> 2) & 3);
        head_gain =  (UINT8)((rdata >> 13) & 7);

        //if(cr_irs<=6)
        //    cr_irs += 16;

        //if(cr_prs<4)
        //    cr_prs += 8;

        snr_idx = (UINT8) ((rdata >> 16) & 0x1f);

        if((rdata & 1))
        {
            ADPT_CR_PRINTF("OLDCR \n");
        }
        ADPT_CR_PRINTF("SNR is:%d/10 dB, ", snr_dbx10_tab[snr_idx]);
        ADPT_CR_PRINTF("head_gain=%d, cr_prs=0x%x, cr_irs=0x%x, cr_frac=%d, wider_loop=%d\n",
                    head_gain, cr_prs, cr_irs, cr_frac, (rdata >> 1 & 1));
        interval_cnt = 6;
    }
    interval_cnt--;

#ifdef RPT_CR_SNR_EST
    UINT8 byte_dat;
    UINT16 SNR_EST;
    //nim_reg_read(dev,0xe7,ByteDat,1);
    //ByteDat |= 0x80;
    //nim_reg_write(dev,0xe7, &ByteDat,1);
    ADPT_CR_PRINTF("\t\tenter RPT_CR_SNR_EST block\n");
    byte_dat = 0xe4; // ADPT_CR_DEBUG=0
    nim_reg_write(dev, 0x0e, &byte_dat, 1);

    nim_reg_read(dev, 0xbc, &byte_dat, 1);
    byte_dat &= 0x7f; // reg_crbc[7]=0
    nim_reg_write(dev, 0xbc, &byte_dat, 1);
    //if (ByteDat & 0x80)
    {
        //NIM_PRINTF("\tenter the if block");
        nim_reg_read(dev, 0xbc, &byte_dat, 1);
        SNR_EST = (byte_dat & 0x1f) << 6;
        nim_reg_read(dev, 0xbb, &byte_dat, 1);

        SNR_EST += (byte_dat >> 2) & 0x03f;
        byte_dat = 0x00;
        nim_reg_write(dev, 0xbc, &byte_dat, 1);

        //if(var>var_max)
        //var_max=var;
        //    if(var<var_min)
        //    var_min = var;
        ADPT_CR_PRINTF("SNR_EST=%d\n", SNR_EST);
    }
    //nim_reg_read(dev,0xe7,ByteDat,1);
    //ByteDat &= 0x7f;
    //nim_reg_write(dev,0xe7, &ByteDat,1);
#endif
};

#endif

/*****************************************************************************
* INT32 nim_s3503_fft(struct nim_device *dev, UINT32 startfreq)
* Description: S3501 set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
INT32 nim_s3503_fft(struct nim_device *dev, UINT32 startfreq)
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
    dev_priv = (struct nim_s3501_private *)dev->priv;
    freq = startfreq;

    if (dev_priv->ul_status.s3501_autoscan_stop_flag)
    {
#if defined(__NIM_LINUX_PLATFORM__)
        if(dev_priv->yet_return == FALSE)
        {
            nim_send_as_msg(dev_priv->blind_msg.port_id, 0, 0, 0, 0, 0, 1);
            dev_priv->yet_return = TRUE;
        }

#endif
        NIM_PRINTF("    leave fuction nim_s3503_FFT\n");
        return SUCCESS;
    }

    dev_priv->nim_tuner_control(dev_priv->tuner_id, freq, 0);

    comm_delay(0xffff * 4);
    if (dev_priv->nim_tuner_status != NULL)
    {
        dev_priv->nim_tuner_status(dev_priv->tuner_id, &lock);
     }

    dev_priv->ul_status.m_crnum = 0;
    dev_priv->ul_status.m_step_freq = nim_s3503_cap_fft_find_channel(dev, &startfreq);

    AUTOSCAN_PRINTF("\tCurrent Time Blind Scan Range: From  %dMHz to %dMHz  have find %d channels\n",
                    startfreq - 45, startfreq + 45, dev_priv->ul_status.m_crnum);

    for (i = 0; i < dev_priv->ul_status.m_crnum; i++)
    {
        AUTOSCAN_PRINTF("\tTP -> %d. Freq = %d, rs = %d\n", i, dev_priv->ul_status.m_freq[i],
                    dev_priv->ul_status.m_rs[i]);
    }

    {
        // amy add for double check, avoid one TP detected as two
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
                    AUTOSCAN_PRINTF("\tError detected TP, modified to -> %d. Freq=%d, rs=%d\n", ch_num, cur_f, cur_rs);
                }
            }
            if (dev_priv->ul_status.m_crnum < ch_num)
            {
                AUTOSCAN_PRINTF("current FFT result is:\n");
                for (i = 0; i < 1024; i = i + 1)
                {
            AUTOSCAN_PRINTF("%d\n", fft_energy_1024[i]);
                 }
            }
            dev_priv->ul_status.m_crnum = ch_num;

        }

        //for (i = 0; i < dev_priv->ul_status.m_CRNum; i++)
        //{
        //AUTOSCAN_PRINTF("\tAfter raw TP Modified,TP Distribution:TP -> %d. Freq = %d, rs = %d\n", i,
    //                 dev_priv->ul_status.m_Freq[i], dev_priv->ul_status.m_Rs[i]);
        //}
    }

    return SUCCESS;
}


/*****************************************************************************
* static INT32 nim_s3503_set_ext_lnb(struct nim_device *dev, struct QPSK_TUNER_CONFIG_API *ptrqpsk_tuner)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_set_ext_lnb(struct nim_device *dev, struct QPSK_TUNER_CONFIG_API *ptrqpsk_tuner)
{
     struct nim_s3501_private *priv_mem = NULL;
	 
    if((dev == NULL) || (ptrqpsk_tuner == NULL))
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




/*****************************************************************************
* static INT32 nim_s3503_autoscan_initial (struct nim_device *dev)
*
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_autoscan_initial (struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    // Set Cap speed
    data = 0x3c;    //[7:6]: speed, [5:0] packet num.
    nim_reg_write(dev, RBD_CAP_PRM + 1, &data, 1);

    // cap from AGC1
    nim_reg_read(dev, R70_CAP_REG + 1, &data, 1);
    data = data & 0x40;
    nim_reg_write(dev, R70_CAP_REG + 1, &data, 1);

    // cap from REG
    data = 0x00;
    nim_reg_write(dev, RBD_CAP_PRM, &data, 1);

    return SUCCESS;
}


/*****************************************************************************
* static INT32 nim_s3503_reg_get_map_type(struct nim_device *dev, UINT8 *map_type)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_reg_get_map_type(struct nim_device *dev, UINT8 *map_type)
{
    UINT8 data = 0;

    if(dev == NULL || map_type == NULL)
    {
	    return RET_FAILURE;
	}	
    //  NIM_PRINTF("Enter Fuction nim_s3503_reg_get_map_type \n");
    nim_reg_read(dev, R69_RPT_CARRIER + 0x02, &data, 1);
    *map_type = ((data >> 5) & 0x07);
    return SUCCESS;

    //  Map type:
    //      0x0:    HBCD.
    //      0x1:    BPSK
    //      0x2:    QPSK
    //      0x3:    8PSK
    //      0x4:    16APSK
    //      0x5:    32APSK
}



/*****************************************************************************
* static INT32 nim_s3503_reg_get_iqswap_flag(struct nim_device *dev, UINT8 *iqswap_flag)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_reg_get_iqswap_flag(struct nim_device *dev, UINT8 *iqswap_flag)
{
    UINT8 data = 0;
	
    if((dev == NULL) || (iqswap_flag == NULL))
    {
	    return RET_FAILURE;
	}	

    //  NIM_PRINTF("Enter Fuction nim_s3503_reg_get_iqswap_flag \n");
    nim_reg_read(dev, R6C_RPT_SYM_RATE + 0x02, &data, 1);
    *iqswap_flag = ((data >> 4) & 0x01);
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_reg_get_roll_off(struct nim_device *dev, UINT8 *roll_off)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_reg_get_roll_off(struct nim_device *dev, UINT8 *roll_off)
{
    UINT8 data = 0;

    if((dev == NULL) || (roll_off == NULL))
	{
        return RET_FAILURE;
	}	
    //  NIM_PRINTF("Enter Fuction nim_s3503_reg_get_roll_off \n");
    nim_reg_read(dev, R6C_RPT_SYM_RATE + 0x02, &data, 1);
    *roll_off = ((data >> 5) & 0x03);
    return SUCCESS;

    //  DVBS2 Roll off report
    //      0x0:    0.35
    //      0x1:    0.25
    //      0x2:    0.20
    //      0x3:    Reserved
}


/*****************************************************************************
* static INT32 nim_s3503_reg_get_work_mode(struct nim_device *dev, UINT8 *work_mode)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_reg_get_work_mode(struct nim_device *dev, UINT8 *work_mode)
{
    UINT8 data = 0;

    if((dev == NULL) || (work_mode == NULL))
	{
        return RET_FAILURE;
	}	
    //  NIM_PRINTF("Enter Fuction nim_s3503_reg_get_work_mode \n");
    nim_reg_read(dev, R68_WORK_MODE, &data, 1);
    *work_mode = data & 0x03;
    return SUCCESS;

    //  Work Mode
    //      0x0:    DVB-S
    //      0x1:    DVB-S2
    //      0x2:    DVB-S2 HBC
}

/*****************************************************************************
* INT32 nim_s3503_reg_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* rsubc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_reg_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
{
    UINT8 data[3] = {0};//, i;
    UINT32 sample_rate = 0;
    UINT32 temp = 0;
    UINT32 symrate = 0;
	
    if((dev == NULL) || (sym_rate == NULL))
	{
        return RET_FAILURE;
    }
    temp = 0;
    nim_reg_read(dev, R6C_RPT_SYM_RATE, data, 3);
    //        symrate = (data[0]>>1) + (data[1] <<7)+((data[2]&0x01) <<15); // K S/s
    symrate = data[0] + (data[1] << 8) + ((data[2] & 0x01) << 16);
    nim_s3503_get_dsp_clk(dev, &sample_rate);
    sample_rate = sample_rate / 2;
    *sym_rate = nim_s3501_multu64div(symrate, sample_rate, 92160);
    return SUCCESS;
}

INT32 nim_s3503_reg_get_freq(struct nim_device *dev, UINT32 *freq)
{
    INT32 freq_off = 0;
    UINT8 data[3] ={0};
    UINT32 sample_rate = 0;
    UINT32 tdata = 0;
    UINT32 temp = 0;
	
    if((dev == NULL) || (freq == NULL))
	{
        return RET_FAILURE;
    }
	
    temp = 0;
    nim_reg_read(dev, R69_RPT_CARRIER, data, 3);
    tdata = (data[0] & 0xff)  + ((data[1] & 0xff) << 8);
    if ((data[2] & 0x01) == 1)
    {
        temp = (tdata ^ 0xffff) + 1;
     }
    else
    {
        temp = tdata & 0xffff;
     }
    nim_s3503_get_dsp_clk(dev, &sample_rate);
    if ((data[2] & 0x01) == 1)
    {
        freq_off = 0 - nim_s3501_multu64div(temp, sample_rate, 92160000);
     }
    else
    {
        freq_off = nim_s3501_multu64div(temp, sample_rate, 92160000);
     }
    *freq += freq_off;
    return SUCCESS;
}


