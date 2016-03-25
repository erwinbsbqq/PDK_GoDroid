/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: nim_s3281_ic.c
*
*    Description: s3281,ic layer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "nim_s3281.h"

#define INDEX_2   2
#define INDEX_3   3
#define INDEX_7   7

#define FFT_DIVI_64   64
#define FFT_NUM_68   68
#define FFT_NUM_87   87
#define FFT_NUM_167   167
#define FFT_NUM_186   186
#define TUNER_RETRY_2  2
#define TUNER_RETRY_5  5
#define TIME_OUT_100    100
#define TIME_OUT_50    50
#define COMPARE_VALUE_10    10
#define COMPARE_VALUE_200   200
#define COMPARE_VALUE_50   50
#define COMPARE_VALUE_500   500



#define RF_GAIN_TIMES 16
#define TR_TOUT_CNT_7 7
#define CMA_TOUT_CNT_6 6
#define MODULATION_6 6


#define CUR_BER_VAL_5000  5000
#define CUR_BER_VAL_1000  1000
#define CUR_BER_VAL_5  5
#define READ_BER_CNT_4  4
#define READ_BER_CNT_6  6
#define BER_TH_CNT_3  3
#define BER_TH_CNT_4  4
#define PER_LF_CEN_1  1
#define PER_LF_CEN_2  2
#define EQ_UNLOCK_CNT_2  2
#define EQ_UNLOCK_CNT_3  3
#define TASK_CNT  20
#define DELFRQ_CAL_MAX_4095 4095

#define LOCK_TRY_TIMES_50 50
#define AGC_GAIN_VALUE_208 208
#define AGC_GAIN_VALUE_721 721
#define AGC_GAIN_VALUE_456 456

#define SYM_6500  6500

#define SYM_1200  1200
#define SYM_1300  1300
#define SYM_1500  1500
#define SYM_1800  1800




#define MAGIC_CONST_0    0
#define MAGIC_CONST_3    3
#define MAGIC_CONST_2    2
#define MAGIC_CONST_1    1
#define MAGIC_CONST_0X0F 0X0F
#define MAGIC_CONST_0X1F 0X1F

#define ACI_IMPROVED_ENABLE //for imporve 6M ACI performance about TDA18250 add by grace 



struct s3281_lock_info s3281_cur_channel_info;
UINT32       BER_COUNTS = 0;
UINT32       PER_COUNTS = 0;
BOOL       channel_change_en = FALSE;
BOOL        rf_agc_en = TRUE;
UINT8        if_def_val2 = 0;//for aci signal


//joey 20080504. add in ber&per ready count.
//static UINT32       acc_ber_cnt = 0;
//static UINT16        aci_delfreq =  0;

//joey 20080504. For ACI and max level AGC patch.
static UINT8        if_sml_val1 = 0; //for strong signal
static UINT16        rf_sml_th1 = 0;
static UINT16        rf_big_th2 = 0;

static BOOL        sw_test_thread_off = FALSE;

static UINT32      last_time = 0;
static UINT8       first_time = 1;
static BOOL        reset_once = FALSE;

static UINT8       first_cal = 1;
static UINT8       tr_tout_cnt = 0;
static UINT8       cma_tout_cnt = 0;
static UINT8       tr_tout_flg = 0;
static UINT8       cma_tout_flg = 0;
static UINT32      old_time = 0;
static UINT8       fsm_trans = 1;
static UINT8       init_act = 1;
//static UINT8       read_ber_cnt = 0;
//static UINT8       ber_th_cnt_a = 0;
//static UINT8       ber_th_cnt_c = 0;
//static UINT8       per_1f_cnt = 0;
//static UINT8       first_cal = 1;
//static UINT32      old_acc_ber_cnt = 0;
static const UINT8 scale = 15;
static const UINT8 index_width = 5;
//static UINT8       eq_st_flg = 0;
//static UINT8       eq_unlock_cnt = 0;
//static UINT8       fff_len_ch = 0;
//static UINT8       first_cal = 1;


#if (QAM_FPGA_USAGE == SYS_FUNC_ON)
static BOOL     draw_osd_en = TRUE;
//#else
//static BOOL     draw_osd_en = FALSE;
#endif

static const INT16 sin_table[65] =
{
    0, 13, 25, 38, 50, 63, 75, 87,
    100, 112, 124, 136, 148, 160, 172, 184,
    196, 207, 218, 230, 241, 252, 263, 273,
    284, 294, 304, 314, 324, 334, 343, 352,
    361, 370, 379, 387, 395, 403, 410, 418,
    425, 432, 438, 445, 451, 456, 462, 467,
    472, 477, 481, 485, 489, 492, 496, 499,
    501, 503, 505, 507, 509, 510, 510, 511,
    511
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



static void     nim_s3281_dvbc_set_rs(UINT32 sym);


static void        nim_s3281_dvbc_set_search_rs(UINT16 rs_min, UINT16 rs_max );









//=============================================================
#if (QAM_TEST_FUNC==SYS_FUNC_ON)

/*These functions come from the win_nimreg.c file   */

/* These functions is software test or patch*/
static void nim_s3201_r2fft(INT16 *FFT_I_256, INT16 *FFT_Q_256)
{
    INT16   L, k, i, j;
    INT16   wn_num, p;

    INT16   wn_imag_index, wn_real_index;
    INT16   neg_flag;
    INT16   wn_imag,  wn_real;

    INT32   real_tmp_a1, real_tmp_b1, real_tmp_3;
    INT32   imag_tmp_a1, imag_tmp_b1, imag_tmp_3;

    INT16   TR, TI;
    INT16   temp_r, temp_i;
    INT16   data_r[256];
    INT16   data_i[256];

    for (i = 0; i < 256; i++)
    {
        k = 0;
        for(j = 0; j < 8; j++)
        {
            k = k << 1;
            k += ((i >> j) & 0x1); //address change for 256 FFT
        }
        data_r[k] = *(FFT_I_256 + i);
        data_i[k] = *(FFT_Q_256 + i);
    }
    i = 0;
    j = 0;
    k = 0;

    /************** following code FFT *******************/

    for(L = 1; L <= 8; L++)
    {
        wn_num = 1;
        wn_num  = wn_num  << (L - 1); //unmber of Wn in this layer

        for(j = 0; j < wn_num; j++)
        {
            p = 1;
            p = p << (8 - L);
            p = p * j;
            if(p > FFT_DIVI_64)
                wn_imag_index = 128 - p;
            else
                wn_imag_index = p;

            wn_imag = sin_table[wn_imag_index];

            if(p > FFT_DIVI_64)
            {
                wn_real_index = p - 64;
                neg_flag = 1;
            }
            else
            {
                wn_real_index = 64 - p;
                neg_flag = 0;
            }

            if(neg_flag)
                wn_real = -sin_table[wn_real_index];
            else
                wn_real = sin_table[wn_real_index];

            for(k = j; k < 256; k = k + 2 * wn_num)  //butterfly
            {

                TR = data_r[k];
                TI = data_i[k];
                temp_r = data_r[k + wn_num];
                temp_i = data_i[k + wn_num];

                real_tmp_a1 = temp_r * wn_real;

                real_tmp_b1 = temp_i * wn_imag;
                real_tmp_3  = (real_tmp_a1 + real_tmp_b1 + 256) >> 9;

                imag_tmp_a1 = temp_r * wn_imag;
                imag_tmp_b1 = temp_i * wn_real;
                imag_tmp_3  = (imag_tmp_a1  - imag_tmp_b1 + 256) >> 9;

                data_r[k] = TR + real_tmp_3;
                data_r[k + wn_num] = TR - real_tmp_3;

                data_i[k] = TI - imag_tmp_3;
                data_i[k + wn_num] = TI + imag_tmp_3;

                *(FFT_I_256 + k) = data_r[k];
                *(FFT_Q_256 + k) = data_i[k];
                *(FFT_I_256 + k + wn_num) = data_r[k + wn_num];
                *(FFT_Q_256 + k + wn_num) = data_i[k + wn_num];
            }
        }

    }
}

static void nim_s3281_dvbc_mon_catch_ad_data(struct nim_device *dev)
{
    UINT8  data=0;
    UINT16 i=0;
    UINT8  data_reg[3]={0};
    INT16  di[256]={0};
    INT16  dq[256]={0};
    UINT32 spectrum = 0;
    UINT8  fft_buf_full = 0;
    UINT8  time_out = 0;
    UINT16 sum1 = 0;
    UINT16 sum2 = 0;
    UINT8  lock = 0;
    UINT8 tuner_retry = 0;

    UINT32 freq = s3281_cur_channel_info.frequency;
    struct nim_s3281_private *dev_priv;
    dev_priv = (struct nim_s3281_private *)dev->priv;

    //Step 1: set receiver to IDLE status, reset Interrupt indicator, and set WORK_MODE

    data = 0x80;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);
    data = 0x00;
    nim_s3281_dvbc_write(NIM_S3202_INTERRUPT_EVENTS, &data, 1);
    data = 0x0F;
    nim_s3281_dvbc_write(NIM_S3202_INTERRUPT_MASK, &data, 1);

    nim_s3281_dvbc_read(NIM_S3202_FSM1, &data, 1);
    data = (data & 0xcf) | 0x10;
    nim_s3281_dvbc_write(NIM_S3202_FSM1, &data, 1); // set to catch mode.

    data = 0x40;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);

    //Setp 2: set tuner frequency
    do
    {
        if(tuner_retry > TUNER_RETRY_2)
        {
            S3281_PRINTF("ERROR! Tuner Lock Fail\n");
            lock = 0;
            break;
        }
        tuner_retry++;
        // Fast config tuner  //, FAST_TIMECST_AGC, _1ST_I2C_CMD
        if(dev_priv->nim_tuner_control(dev_priv->tuner_id, freq, 0) == ERR_FAILUE)
        {
            S3281_PRINTF("Fast Config tuner failed !\n");
        }

        // Slow config tuner  //, SLOW_TIMECST_AGC, _1ST_I2C_CMD
        if(dev_priv->nim_tuner_control(dev_priv->tuner_id, freq, 0) == ERR_FAILUE)
        {
            S3281_PRINTF("Slow Config tuner failed!\n");
        }

        // Read status
        if(dev_priv->nim_tuner_status(dev_priv->tuner_id, &lock) == ERR_FAILUE)
        {
            S3281_PRINTF("ERROR! Tuner Read Status Fail\n");
        }

        S3281_PRINTF("catch AD Tuner Lock Times=%d, *lock=%d !!\n", tuner_retry, lock);
    }
    while(0 == lock);

    //Step 3: Wait buffer full interrupt

    // First, we should ensure that the AGC is locked
    time_out = 0;
    while(1)
    {
        comm_sleep(10);
        if(time_out > TIME_OUT_100)
        {
            //S3281_PRINTF("AGC lock time_out !!\n");
            goto out;
        }
        time_out += 1;
        nim_s3281_dvbc_read(NIM_S3202_MONITOR1, &data, 1);
        if(0x01 == (data & 0x01))
        {
            //S3281_PRINTF("AGC locked!\n");
            break;
        }
    }

    nim_s3281_dvbc_read(NIM_S3202_FSM11, &data, 1);
    data = (data & 0xE7) | 0x08;
    nim_s3281_dvbc_write(NIM_S3202_FSM11, &data, 1);

    // Second, wait for the BUF is fulled
    time_out = 0;
    while(!fft_buf_full)
    {
        if(time_out > TIME_OUT_50)
        {
            //S3281_PRINTF("FFT buffer full time out!!\n");
            goto out;
        }
        time_out += 1;
        nim_s3281_dvbc_read(NIM_S3202_MONITOR1, &data, 1);
        fft_buf_full = (data >> 6) & 0x01;
        comm_sleep( 100 );
    }

    //Step 4: Get data from registers
    for (i = 0; i < 256; i++ )
    {
        data = i;
        nim_s3281_dvbc_write(NIM_S3202_MONITOR2, &data, 1);
        nim_s3281_dvbc_read(NIM_S3202_MONITOR3, data_reg, 3);
        di[i] = (INT16)((data_reg[2] & 0x0f) << 6 | (data_reg[1] & 0xfc) >> 2);
        dq[i] = (INT16)((data_reg[1] & 0x03) << 8 | (data_reg[0]));
        //S3281_PRINTF("ram[%03d] I,Q= 0x%03x, 0x%03x \n",i,di[i],dq[i]);
        // di , dq  from 10bit extern to 16bit for sign data.
        if (0x200 == (di[i] & 0x200))    di[i]  = di[i]  | 0xfc00;
        if (0x200 == (dq[i] & 0x200))    dq[i] = dq[i] | 0xfc00;

    }

    nim_s3201_r2fft(di, dq);


#if defined(__NIM_TDS_PLATFORM__)
    sum1 = 0;
    sum2 = 0;
    if (draw_osd_en == TRUE)
    {
        draw_square(100, 270, 256, 150, 87);
        for (i = 0 ; i < 150 ; i ++)
            osd_draw_fill(228, 270 + i, 1, 1, 0xF1, NULL);
    }

    for (i = 0 ; i < 256; i++)
    {
        spectrum =    log10times100_l ((UINT32)(di[i] * di[i]) + (UINT32)(dq[i] * dq[i])) / 10;
        //S3281_PRINTF(" %03d, di = 0x%03x, dq = 0x%03x, Result = %d \n",i , di[i], dq[i],spectrum);
        if (i >= FFT_NUM_68 & i <= FFT_NUM_87)
            sum1 = sum1 + spectrum;
        if (i >= FFT_NUM_167 & i <= FFT_NUM_186)
            sum2 = sum2 + spectrum;
        if (draw_osd_en == TRUE)
            osd_draw_fill(100 + i, 245 + spectrum * 2, 1, 2, 0xF2, NULL);
    }

    sum1 = sum1 / 20;
    sum2 = sum2 / 20;

    if ((sum1 - sum2) > COMPARE_VALUE_10)
        aci_delfreq = 300;
    else
        aci_delfreq = 0;

    S3281_PRINTF(" Sum1 = %d, Sum2 = %d \n", sum1, sum2);
#endif

out:
    S3281_PRINTF("\n");
    //Step 5: After catch data, should set the mode from " catch mode" to "normal mode".
    nim_s3281_dvbc_read(NIM_S3202_FSM1, &data, 1);
    data = (data & 0xcf) | 0x00;
    nim_s3281_dvbc_write(NIM_S3202_FSM1, &data, 1);

    nim_s3281_dvbc_read(NIM_S3202_FSM11, &data, 1);
    data = (data & 0xE7) | 0x10;
    nim_s3281_dvbc_write(NIM_S3202_FSM11, &data, 1);

    data = 0x80;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);
    data = 0x40;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);


    return;
}
#endif

#if 0
static void nim_s3281_set_to_nomal_mode(void)
{
    UINT8 data = 0;

    S3281_PRINTF("\n");
    //Step 5: After catch data, should set the mode from " catch mode" to "normal mode".
    nim_s3281_dvbc_read(NIM_S3202_FSM1, &data, 1);
    data = (data & 0xcf) | 0x00;
    nim_s3281_dvbc_write(NIM_S3202_FSM1, &data, 1);

    nim_s3281_dvbc_read(NIM_S3202_FSM11, &data, 1);
    data = (data & 0xE7) | 0x10;
    nim_s3281_dvbc_write(NIM_S3202_FSM11, &data, 1);

    data = 0x80;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);
    data = 0x40;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);

}
#endif

#if 0
static void nim_s3281_dvbc_mon_catch_ad_data_loop(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT16 i = 0;
    UINT8 data_reg[3] = {0};
    INT16 di[256] = {0};
    INT16 dq[256] = {0};
    UINT8 fft_buf_full = 0;
    UINT8 time_out = 0;
    UINT32 freq = 0;

    struct nim_s3281_private *dev_priv = NULL;

    freq = s3281_cur_channel_info.frequency;
    dev_priv = (struct nim_s3281_private *)dev->priv;

    //Step 1: set receiver to IDLE status, reset Interrupt indicator, and set WORK_MODE

    data = 0x80;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);
    data = 0x00;
    nim_s3281_dvbc_write(NIM_S3202_INTERRUPT_EVENTS, &data, 1);
    data = 0x0F;
    nim_s3281_dvbc_write(NIM_S3202_INTERRUPT_MASK, &data, 1);

    nim_s3281_dvbc_read(NIM_S3202_FSM1, &data, 1);
    data = (data & 0xcf) | 0x10;
    nim_s3281_dvbc_write(NIM_S3202_FSM1, &data, 1); // set to catch mode.

    data = 0x40;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);

    //Setp 2: set tuner frequency
#if 0
    do
    {
        if(tuner_retry > TUNER_RETRY_2)
        {
            S3281_PRINTF("ERROR! Tuner Lock Fail\n");
            lock = 0;
            break;
        }
        tuner_retry++;
        // Fast config tuner
        if(dev_priv->nim_tuner_control(dev_priv->tuner_id, freq, 0, FAST_TIMECST_AGC, _1st_i2c_cmd) == ERR_FAILUE)
        {
            S3281_PRINTF("Fast Config tuner failed !\n");
        }

        // Slow config tuner
        if(dev_priv->nim_tuner_control(dev_priv->tuner_id, freq, 0, SLOW_TIMECST_AGC, _1st_i2c_cmd) == ERR_FAILUE)
        {
            S3281_PRINTF("Slow Config tuner failed!\n");
        }

        // Read status
        if(dev_priv->nim_tuner_status(dev_priv->tuner_id, &lock) == ERR_FAILUE)
        {
            S3281_PRINTF("ERROR! Tuner Read Status Fail\n");
        }

        S3281_PRINTF("catch AD Tuner Lock Times=%d, *lock=%d !!\n", tuner_retry, lock);
    }
    while(0 == lock);
#endif
    //Step 3: Wait buffer full interrupt

    // First, we should ensure that the AGC is locked
    time_out = 0;
    while(1)
    {
        comm_sleep(10);
        if(time_out > TIME_OUT_100)
        {
            //S3281_PRINTF("AGC lock time_out !!\n");
            nim_s3281_set_to_nomal_mode();
            return;
        }
        time_out += 1;
        nim_s3281_dvbc_read(NIM_S3202_MONITOR1, &data, 1);
        if(0x01 == (data & 0x01))
        {
            //S3281_PRINTF("AGC locked!\n");
            break;
        }
    }

    nim_s3281_dvbc_read(NIM_S3202_FSM11, &data, 1);
    data = (data & 0xE7) | 0x08;
    nim_s3281_dvbc_write(NIM_S3202_FSM11, &data, 1);

    // Second, wait for the BUF is fulled
    time_out = 0;
    while(!fft_buf_full)
    {
        if(time_out > TIME_OUT_50)
        {
            //S3281_PRINTF("FFT buffer full time out!!\n");
            nim_s3281_set_to_nomal_mode();
            return;
        }
        time_out += 1;
        nim_s3281_dvbc_read(NIM_S3202_MONITOR1, &data, 1);
        fft_buf_full = (data >> 6) & 0x01;
        comm_sleep( 100 );
    }


    //Step 4: Get data from registers
    for (i = 0; i < 256; i++ )
    {
        data = i;
        nim_s3281_dvbc_write(NIM_S3202_MONITOR2, &data, 1);
        nim_s3281_dvbc_read(NIM_S3202_MONITOR3, data_reg, 3);
        di[i] = (INT16)((data_reg[2] & 0x0f) << 6 | (data_reg[1] & 0xfc) >> 2);
        dq[i] = (INT16)((data_reg[1] & 0x03) << 8 | (data_reg[0]));


        S3281_PRINTF("%d, %d \n", di[i], dq[i]);

        // di , dq  from 10bit extern to 16bit for sign data.
        if (0x200 == (di[i] & 0x200))
    {
        di[i]  = di[i]  | 0xfc00;
        }
        if (0x200 == (dq[i] & 0x200))
    {
        dq[i] = dq[i] | 0xfc00;
         }

    }


    return;
}

#endif


void nim_s3281_task_proc(UINT32 param1, UINT32 param2)
{
	UINT16         rdata = 0;
	UINT8          data = 0;
	UINT8          qammod = 0;
	UINT32         the_curt_time = 0;
	UINT32         the_last_time = 0;
    BOOL           BER_VALID = FALSE;
	
	struct nim_device *dev = (struct nim_device *) param1 ;
	struct nim_s3281_private *priv = (struct nim_s3281_private *) dev->priv ;
	
  	while (1)
  	{
  		//S3281_PRINTF("[%s]line =%d,here!\n",__FUNCTION__,__LINE__);
       	the_curt_time = osal_get_tick();
		
		if (the_curt_time - the_last_time > 300)
		{
			nim_s3281_dvbc_monitor_berper(priv, &BER_VALID);
			the_last_time = osal_get_tick();
		}



		
#ifdef ACI_IMPROVED_ENABLE    //start for 6M ACI performance improve by grace
		qammod = priv->qam_mode;

		nim_s3281_dvbc_read(0x08a, &data, 1);
		rdata = (data<<8);
		nim_s3281_dvbc_read(0x08b, &data, 1);
		rdata |= data;//DAGC_GAIN
			
		nim_s3281_dvbc_read(0x13b, &data, 1);
		rdata = (data<<8);
		nim_s3281_dvbc_read(0x13a, &data, 1);
		rdata |= data;//DAGC0_GAIN
					
		nim_s3281_dvbc_read(NIM_S3202_MONITOR1, &data, 1);

		if ((rdata>2056)&&(data&0x02))
		{
			if((qammod&0x01) == NIM_DVBC_J83B_MODE)
			{ // j83b
				nim_s3281_dvbc_read(0x136, &data, 1);
				if (data & 0x01)
				{
					//S3281_PRINTF("\n\nOpen ACI improve for J83B \r\n");
				}
					data = 0x34;
					nim_s3281_dvbc_write(0x136, &data, 1);					
			}
			else
			{
				nim_s3281_dvbc_read(0x136, &data, 1);
				if (data & 0x01)
				{
					//S3281_PRINTF("\n\nOpen ACI improve for J83AC \r\n");
				}
					data = 0x04;
					nim_s3281_dvbc_write(0x136, &data, 1); 
				}
			}
			else if(rdata<2200)
			{
				data = 0x01;
				nim_s3281_dvbc_write(0x136, &data, 1); 
			}		
#endif//end for 6M ACI performance improve by grace
			comm_sleep(100);
  	}
}






INT32 nim_s3281_dvbc_monitor_agc_status(struct nim_device *dev,UINT16 *cir)
{
    UINT8 i = 0;
    UINT8 data[2] = {0};
    UINT32 cur_time = 0;
    UINT16 rf_gain_sum = 0;
    UINT16 tmp_rf = 0;

    if((NULL == dev) || (NULL == cir))
    {
        return RET_FAILURE;
    }

    if (channel_change_en == TRUE)
    {
        first_time = 1;
        reset_once = FALSE;
        return SUCCESS;
    }

    if (1 == first_time)
    {
        last_time = osal_get_tick();// the unit is 1ms.
        first_time = 0;
    }

    //check time first.
    cur_time = osal_get_tick();
    if (cur_time - last_time < COMPARE_VALUE_200)
    {
        //time is not arrive, still polling.
        return SUCCESS;
    }
    else
    {
        //time is arrive, do work. update last time.
        last_time = cur_time;
    }

    i = 0;
    rf_gain_sum = 0;
    while (i < RF_GAIN_TIMES)
    {
        if (channel_change_en == TRUE)
        {
            first_time = 1;
            reset_once = FALSE;
            return SUCCESS;

        }

        comm_sleep(10);
        nim_s3281_dvbc_read(0x15, data, 2);
        tmp_rf = ((data[0] >> 2 | (data[1] & 0x0f) << 6) + 512) & 0x3ff;
        rf_gain_sum += tmp_rf;
        i++;
    }

    rf_gain_sum = rf_gain_sum >> 6; //use this, we can reduce the complex.

    if (channel_change_en == TRUE)
    {
        first_time = 1;
        reset_once = FALSE;
        return SUCCESS;

    }

    if (rf_gain_sum < rf_sml_th1) //so-called TH1.
    {
        nim_s3281_dvbc_read(0x12, data, 1);
        if (data[0] > if_sml_val1)
        {
            data[0] = if_sml_val1;
            nim_s3281_dvbc_write(0x12, data, 1);

            reset_once = FALSE;

            S3281_PRINTF("Reach Max sensitivity once!\n ");
        }
    }
    else if (rf_gain_sum > rf_big_th2) // so-called TH2.
    {
        nim_s3281_dvbc_read(0x12, data, 1);
        if ((data[0] < if_def_val2) && (reset_once == FALSE))
        {
            data[0] = if_def_val2;
            nim_s3281_dvbc_write(0x12, data, 1);

            data[0] = 0x80;
            nim_s3281_dvbc_write(0x00, data, 1);

            data[0] = 0x40;
            nim_s3281_dvbc_write(0x00, data, 1);

            reset_once = TRUE;

            S3281_PRINTF("take over point recovey! \n");

        }
    }


    return SUCCESS;
}


static INT32 nim_s3281_agc0a(UINT8 data)
{
    UINT8 new_cr0a = 0;
    UINT8 agc_ref = 0;

    if(data >= MAGIC_CONST_0X1F)
    {
        if (1 == fsm_trans)    // flag to record and clear the time out cnt from unlock to lock
        {

            tr_tout_cnt = 0;
            tr_tout_flg = 0;
            cma_tout_cnt = 0;
            cma_tout_flg = 0;
            fsm_trans = 0;

            nim_s3281_dvbc_read(0x0a, &new_cr0a, 1);

            if (new_cr0a != agc_ref)
            {
                new_cr0a = agc_ref;
                nim_s3281_dvbc_write(0x0a, &new_cr0a, 1);
            }
        }

    }
    else if (MAGIC_CONST_0 == data)//idle state.
    {
        nim_s3281_dvbc_read(0x0a, &new_cr0a, 1);
        if (new_cr0a != agc_ref)
        {
            new_cr0a = agc_ref;
            nim_s3281_dvbc_write(0x0a, &new_cr0a, 1);
        }
        if (0 == first_cal)
        {
            first_cal = 1;
        }

        if (0 == fsm_trans)
        {
            fsm_trans = 1;
        }

        if (1 == init_act)
        {
            tr_tout_cnt = 0;
            tr_tout_flg = 0;

            cma_tout_cnt = 0;
            cma_tout_flg = 0;

            init_act = 0;

        }
    }
    else if (data < MAGIC_CONST_3) //tr unlock.
    {
        tr_tout_cnt += 1;
        if (tr_tout_cnt > TR_TOUT_CNT_7)
        {
            tr_tout_cnt = 0;
            cma_tout_cnt = 0;

            switch(tr_tout_flg)
            {
                case 0:
               {
                nim_s3281_dvbc_read(0x0a, &new_cr0a, 1);
                if (new_cr0a != 0x92)
                {
                    new_cr0a = 0x92;
                    nim_s3281_dvbc_write(0x0a, &new_cr0a, 1);
                }

                tr_tout_flg = 1; //modified by magic
               }
                break;
              case MAGIC_CONST_1:
               {
                nim_s3281_dvbc_read(0x0a, &new_cr0a, 1);
                if (new_cr0a != 0x9a)
                {
                    new_cr0a = 0x9a;
                    nim_s3281_dvbc_write(0x0a, &new_cr0a, 1);
                }
                tr_tout_flg = 2; //modified by magic
              }
              break;
              case MAGIC_CONST_2:
              {
                nim_s3281_dvbc_read(0x0a, &new_cr0a, 1);
                if (new_cr0a != 0x9a)
                {
                    new_cr0a = 0x9a;
                    nim_s3281_dvbc_write(0x0a, &new_cr0a, 1);
                }
                tr_tout_flg = 3; //modified by magic
             }
              break;
              case MAGIC_CONST_3:
              {
                nim_s3281_dvbc_read(0x0a, &new_cr0a, 1);
                if (new_cr0a != 0x88)
                {
                    new_cr0a = 0x88;
                    nim_s3281_dvbc_write(0x0a, &new_cr0a, 1);
                }
                tr_tout_flg = 0; //modified by magic
             }
              break;
              default:
                  break;
            }

        }

        if (fsm_trans == FALSE)
        {
            fsm_trans = TRUE;
        }

        if (init_act == 0)
        {
            init_act = 1;
        }
    }
    else if (data < MAGIC_CONST_0X0F) //dd unlock.
    {
        cma_tout_cnt += 1;
        if (cma_tout_cnt > CMA_TOUT_CNT_6)
        {
            tr_tout_cnt = 0;
            cma_tout_cnt = 0;

            switch(cma_tout_flg)
            {
              case 0:
                {
                    nim_s3281_dvbc_read(0x0a, &new_cr0a, 1);
                    if (new_cr0a != 0x92)
                    {
                        new_cr0a = 0x92;
                        nim_s3281_dvbc_write(0x0a, &new_cr0a, 1);
                    }
                    cma_tout_flg = 1;
                }
                 break;
               case MAGIC_CONST_1:
                {
                    nim_s3281_dvbc_read(0x0a, &new_cr0a, 1);
                    if (new_cr0a != 0x9a)
                    {
                        new_cr0a = 0x9a;
                        nim_s3281_dvbc_write(0x0a, &new_cr0a, 1);
                    }
                    cma_tout_flg = 2;
                }
               break;
               case MAGIC_CONST_2:
                {
                    nim_s3281_dvbc_read(0x0a, &new_cr0a, 1);
                    if (new_cr0a != 0x9a)
                    {
                        new_cr0a = 0x9a;
                        nim_s3281_dvbc_write(0x0a, &new_cr0a, 1);
                    }
                    cma_tout_flg = 3;
                }
               break;
               case MAGIC_CONST_3:
                {
                    nim_s3281_dvbc_read(0x0a, &new_cr0a, 1);
                    if (new_cr0a != 0x88)
                    {
                        new_cr0a = 0x88;
                        nim_s3281_dvbc_write(0x0a, &new_cr0a, 1);
                    }
                    cma_tout_flg = 0;
                 }
               break;
               default:
                   break;
            }
        }

        if (fsm_trans == FALSE)
        {
            fsm_trans = TRUE;
        }

        if (init_act == 0)
        {
            init_act = 1;
        }

    }

    return SUCCESS;
}

INT32 nim_s3281_dvbc_monitor_agc0a_loop(struct nim_device *dev,UINT16 *agc0a_loop)
{
    struct nim_s3281_private *dev_priv = NULL;
    UINT8 agc_ref = 0;
    UINT32 curt_time = 0;
    UINT32 ret=SUCCESS;
    UINT8 data = 0;

    if((NULL == dev) || (NULL == agc0a_loop))
    {
        return RET_FAILURE;
    }
    curt_time = osal_get_tick();
    if (((curt_time >= old_time) && (curt_time - old_time > COMPARE_VALUE_50)) \
            || ((curt_time < old_time) && (((0xffffffff - old_time) + curt_time) > COMPARE_VALUE_50)))
    {
        old_time = osal_get_tick();
    }
    else
    {
        return SUCCESS;
    }

    //64Qam and above 6.5Msym
    if ((s3281_cur_channel_info.modulation != MODULATION_6) || (s3281_cur_channel_info.symbol_rate < SYM_6500))
    {
        if (first_cal == 0)
        {
            first_cal = 1;
        }

        return SUCCESS;
    }

    dev_priv = (struct nim_s3281_private *)dev->priv;
    agc_ref = dev_priv->tuner_config_data.AGC_REF;
    nim_s3281_dvbc_read(0x56, &data, 1);
    data = data & 0x3f;

    ret=nim_s3281_agc0a(data);

    return ret;

}

#if 0
static INT32 nim_s3281_dvbc_monitor_cr_loop(struct nim_device *dev, UINT16 *cr_loop)
{
    UINT8 data = 0;
    UINT32 cur_ber = 0;
    UINT32 cur_per = 0;

    if((NULL == dev) || (NULL == cr_loop))
    {
        return RET_FAILURE;
    }

    nim_s3281_dvbc_read(0x56, &data, 1);
    if((data & NIM_S3281_ALL_LOCK) < NIM_S3281_ALL_LOCK)
    {

        if (0 == first_cal)
        {
            first_cal = 1;
        }

        return SUCCESS;
    }

    if (1 == first_cal)
    {
        read_ber_cnt = 0;
        ber_th_cnt_a = 0;
        ber_th_cnt_c = 0;
        per_1f_cnt = 0;

        first_cal = 0;
    }

    if (old_acc_ber_cnt == acc_ber_cnt)
    {
        return SUCCESS;
    }
    else
    {
        old_acc_ber_cnt = acc_ber_cnt;
    }

    cur_ber = BER_COUNTS;
    cur_per = PER_COUNTS;
    read_ber_cnt += 1;


    if (cur_ber > CUR_BER_VAL_5000)
    {
        ber_th_cnt_a += 1;
    }
    else if (cur_ber < CUR_BER_VAL_1000)
    {
        ber_th_cnt_c += 1;
    }

    if (cur_per > CUR_BER_VAL_5)
    {
        per_1f_cnt += 1;
    }

    if (read_ber_cnt > READ_BER_CNT_4)
    {
        if (ber_th_cnt_a > BER_TH_CNT_3)
        {
            nim_s3281_dvbc_read(0xd8, &data, 1);
            if (data != 0x0a)
            {
                data = 0x0a;
                nim_s3281_dvbc_write(0xd8, &data, 1);
            }

            first_cal = 1;

            return SUCCESS;
        }
        else if ((read_ber_cnt <= READ_BER_CNT_6) && (ber_th_cnt_c > BER_TH_CNT_3) && (per_1f_cnt > PER_LF_CEN_1))
        {  //for Anti-Frequency Jitter

            nim_s3281_dvbc_read(0xd8, &data, 1);
            if (data != 0x0b)
            {
                data = 0x0b;
                nim_s3281_dvbc_write(0xd8, &data, 1);
            }

            first_cal = 1;

            return SUCCESS;
        }
        else if (read_ber_cnt > READ_BER_CNT_6)
        {
            if ((ber_th_cnt_c > BER_TH_CNT_4) && (per_1f_cnt > PER_LF_CEN_2))
            {
                nim_s3281_dvbc_read(0xd8, &data, 1);
                if (data != 0x0b)
                {
                    data = 0x0b;
                    nim_s3281_dvbc_write(0xd8, &data, 1);
                }
            }


            first_cal = 1;

            return SUCCESS;
        }
    }

    return SUCCESS;
}
#endif

#if 0
static INT32 nim_s3281_dvbc_monitor_fff_len(struct nim_device *dev,UINT16 *fff_len)
{
    UINT8 data = 0;

    if((NULL == fff_len) || (dev == NULL))
    {
        return ERR_FAILUE;
    }

    nim_s3281_dvbc_read(0x56, &data, 1);
    if((data & NIM_S3281_ALL_LOCK) >= NIM_S3281_ALL_LOCK)
    {

        if (first_cal == 0)
        {
            first_cal = 1;
        }

        return SUCCESS;
    }

    if (1 == first_cal)
    {
        eq_st_flg = 0;
        eq_unlock_cnt = 0;
        fff_len_ch = 0;

        first_cal = 0;
    }



    if ((0x0f == (data & 0x3f)) && (0 == eq_st_flg)) //eq start and not lock yet
    {
        eq_st_flg = 1;
    }
    else
    {
        if (((data & 0x3f) < 0x0f) && (1 == eq_st_flg))
        {
            eq_st_flg = 0;
            eq_unlock_cnt += 1;

            //if (eq_unlock_cnt > 3)
            if (eq_unlock_cnt > EQ_UNLOCK_CNT_2)
            {
                nim_s3281_dvbc_read(0xd8, &data, 1);
                if (data != 0x0a)
                {
                    data = 0x0a;
                    nim_s3281_dvbc_write(0xd8, &data, 1);
                }
                else
                {
                    data = 0x0b;
                    nim_s3281_dvbc_write(0xd8, &data, 1);
                }// add for anti-unlock issue under larger frequency jitter or larger IQ imbalance issue

                // add by magic 20090813 for moniter the fff patch

                S3281_PRINTF("%s,      CRd8  =   0x%x \n", __FUNCTION__, data);

                if (eq_unlock_cnt > EQ_UNLOCK_CNT_3)
                {
                    eq_unlock_cnt = 0;   //reset this counter when (eq_unlock_cnt >3)
                    if (fff_len_ch == 0)
                    {
                        nim_s3281_dvbc_read(0x28, &data, 1);
                        data = (data & 0xf8) | 0x03;
                        nim_s3281_dvbc_write(0x28, &data, 1);

                        fff_len_ch = 1;
                    }
                    else
                    {
                        nim_s3281_dvbc_read(0x28, &data, 1);
                        data = (data & 0xf8) | 0x04;
                        nim_s3281_dvbc_write(0x28, &data, 1);

                        fff_len_ch = 0;
                    }
                    // add by magic 20090813 for moniter the fff patch

                    S3281_PRINTF("%s,      CR28  =   0x%x \n", __FUNCTION__, data);

                }
            }
        }
    }

    return SUCCESS;
}
#endif

//=============================================================

BOOL get_test_thread_off(void)
{
    return sw_test_thread_off;
}

#if 0
static void set_test_thread_off(BOOL flag)
{
    sw_test_thread_off = flag;
}
#endif

/*****************************************************************************
* INT32 nim_s3281_hw_init(struct nim_device *dev)
* Description: S3202 open
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/

INT32 nim_s3281_hw_init(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT8 data1 = 0;
    struct DEMOD_CONFIG_ADVANCED qam_config;
	struct nim_s3281_private *priv = NULL;
	struct QAM_TUNER_CONFIG_DATA tunerconfig;
	
    if(dev == NULL)
    {
	    return ERR_FAILURE;
	}	
    priv = (struct nim_s3281_private *)dev->priv;
    

    S3281_PRINTF("[%s] line=%d,here!\n", __FUNCTION__, __LINE__);
	comm_memcpy(&tunerconfig,&priv->tuner_config_data,sizeof(struct QAM_TUNER_CONFIG_DATA));
    comm_memset(&qam_config,0,sizeof(struct DEMOD_CONFIG_ADVANCED));
    //soft reset.
    data = 0x80;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);

	 S3281_PRINTF("[%s] line=%d,here!\n", __FUNCTION__, __LINE__);
#if 1
    //add more margin to TSO clock
    data1 = 0x20;
    nim_s3281_dvbc_write(NIM_S3281_CBR1_R408, &data1, 1);

    //set  INI_FREQ_OFFSET to zero
    nim_s3281_dvbc_read( NIM_S3202_FSM4, &data1, 1);
    data1 = 0;
    nim_s3281_dvbc_write( NIM_S3202_FSM4, &data1, 1);

    //ad out sign set from unsigned to signed
    nim_s3281_dvbc_read( NIM_S3281_ANA_INT_SET_R420, &data1, 1);
    data1 = data1 | 0x02;
    nim_s3281_dvbc_write( NIM_S3281_ANA_INT_SET_R420, &data1, 1);

    //Set the buffer addr for J83B deinterleave: AutoRun12=wm 0xb8028428 buffer_addr

    //TSO set:AutoRun13=wm 0xb8028404 0x0C210010
    nim_s3281_dvbc_read( NIM_S3281_TSO1_R406, &data1, 1);
    data1 = data1 & 0xFC;    //SPI select
    nim_s3281_dvbc_write( NIM_S3281_TSO1_R406, &data1, 1);

    //AutoRun14=wm 0xb8028400 0xFF1F4781
    nim_s3281_dvbc_read( NIM_S3281_TSO0_R400, &data1, 1);
    data1 =  0x81;
    nim_s3281_dvbc_write( NIM_S3281_TSO0_R400, &data1, 1);

    //ad clk phase set
    nim_s3281_dvbc_read( NIM_S3281_ANA_INT_SET_R420, &data1, 1);
    data1 = data1 | 0x05;
    data1 = data1 & 0xfd;
    nim_s3281_dvbc_write( NIM_S3281_ANA_INT_SET_R420, &data1, 1);


    //close RF-tuner and open IF-AGC: AutoRun5=wm 0xb8028010 0x83335D23
    nim_s3281_dvbc_read( NIM_S3202_AGC10, &data1, 1);
    data1 = data1 & 0xdf;
    data1 = data1 | 0x80;
    nim_s3281_dvbc_write( NIM_S3202_AGC10, &data1, 1);

    //set the AGC lock length: AutoRun8=wm 0xB802800C 0xA8C5003B
    nim_s3281_dvbc_read( NIM_S3202_AGC5, &data1, 1);
    data1 = data1 & 0xfb;
    data1 = data1 | 0x05;
    nim_s3281_dvbc_write( NIM_S3202_AGC5, &data1, 1);

    //    AutoRun12=wm 0xb8028428 0xa0600000
    //    nim_s3281_dvbc_read( NIM_S3202_AGC5, &data1, 1);
    //    data1 = 0x60;
    //    nim_s3281_dvbc_write( NIM_S3202_AGC5, &data1, 1);

    //dsp clock source select bit7bit6:  00: 60m; 01: adc 54m; 10: pll 54
    nim_s3281_dvbc_read( NIM_S3281_CLKGATE_R40F, &data1, 1);
    data1 = data1 & 0x3f;
    nim_s3281_dvbc_write( NIM_S3281_CLKGATE_R40F, &data1, 1);

    //     regd0 = 0x12 (default = 0x13)
    //     regd1 = 0x11 (default = 0x12)
    //     regf6 = 0x08 (default = 0x00)
    //     regf7 = 0x0a (default = 0x0c)
    data1 = 0x12;
    nim_s3281_dvbc_write( NIM_S3202_CR_INT_PATH_GAIN_1, &data1, 1);
    data1 = 0x11;
    nim_s3281_dvbc_write( NIM_S3202_CR_INT_PATH_GAIN_2, &data1, 1);
    data1 = 0x08;
    nim_s3281_dvbc_write( 0xf6, &data1, 1);
    data1 = 0x0a;
    nim_s3281_dvbc_write( 0xf7, &data1, 1);


    //Modify for SKW M3202C 674M can't lock issue.(2012-8-13)
    data = 0x3a; //bypass pre-echo, awgn still on. Reg[0x0c]
    nim_s3281_dvbc_write(NIM_S3202_AGC3, &data, 1);

#if 0
    //joey, 20120720, for skyworth 674M issue. modify CR register for 64Qam easy lock.
    data = 0x11; // CR_P_GAIN_4,Reg[0xcd]
    nim_s3281_dvbc_write(NIM_S3202_CR_PRO_PATH_GAIN_3, &data, 1);

    data = 0x09; // CR_I_GAIN_4,Reg[0xd6]
    nim_s3281_dvbc_write(NIM_S3202_CR_TIME_OUT_FOR_ACQ_2, &data, 1);

    data = 0x13; // CR_P_GAIN_5,Reg[0xce]
    nim_s3281_dvbc_write(NIM_S3202_CR_PRO_PATH_GAIN_4, &data, 1);

    data = 0x7a; // CR_THRED_LOCK_64_4,Reg[0xb8]
    nim_s3281_dvbc_write(NIM_S3202_CR_LOCK_THRD_26, &data, 1);

    data = 0x8e; // CR_THRED_LOCK_64_5,Reg[0xb9]
    nim_s3281_dvbc_write(NIM_S3202_CR_LOCK_THRD_27, &data, 1);

    data = 0x64; // CR_THRED_LOCK_64_6,Reg[0xba]
    nim_s3281_dvbc_write(NIM_S3202_CR_LOCK_THRD_28, &data, 1);

    //joey, 20120724. CR register update.
    data = 0x09; // CR_I_GAIN_6_0,Reg[0xd8]
    nim_s3281_dvbc_write(NIM_S3202_CR_TIME_OUT_FOR_DRT_0, &data, 1);

    data = 0x10;//Reg[0x5e]
    nim_s3281_dvbc_write(NIM_S3202_RS_BER1, &data, 1);

    data = 0x08;//Reg[0x5f]
    nim_s3281_dvbc_write(NIM_S3202_RS_BER2, &data, 1);

    data = 0x9f; // Change threshod to 0x9f. Reg[0x67]
    nim_s3281_dvbc_write(NIM_S3202_EQ_COEF5, &data, 1);
    // end of Modify for SKW M3202C 674M can't lock issue
#endif

    qam_config.qam_config_advanced = priv->qam_mode;
    qam_config.qam_buffer_addr = priv->qam_buffer_addr;
    qam_config.qam_buffer_len = priv->qam_buffer_len;
    nim_s3281_dvbc_set_mode(dev, 0, qam_config);

#endif
   //kent for S3921 (2014/3/17)
    nim_s3281_task_init(dev);

    data = tunerconfig.RF_AGC_MAX;
    nim_s3281_dvbc_write(NIM_S3202_AGC6, &data, 1);
    data = tunerconfig.RF_AGC_MIN;
    nim_s3281_dvbc_write(NIM_S3202_AGC7, &data, 1);
    data = tunerconfig.IF_AGC_MAX;
    nim_s3281_dvbc_write(NIM_S3202_AGC8, &data, 1);
    data = tunerconfig.IF_AGC_MIN;
    nim_s3281_dvbc_write(NIM_S3202_AGC9, &data, 1);
    data = tunerconfig.AGC_REF;
    nim_s3281_dvbc_write(NIM_S3202_AGC1, &data, 1);
    //20130107 modified by russell, for Kingvon issue
    nim_s3281_dvbc_read(NIM_S3202_Q32_MODE1_1, &data, 1);
    data &= 0xf7;
    nim_s3281_dvbc_write(NIM_S3202_Q32_MODE1_1, &data, 1);

    //20130107 modified by russell. for watch-dog timer (FSM reset gap) enlarge to 5.8s.
    data = 0xa0;
    nim_s3281_dvbc_write(NIM_S3202_TIMEOUT_THRESHOLD, &data, 1);

     S3281_PRINTF("[%s] line=%d,here!\n", __FUNCTION__, __LINE__);
	 
    return SUCCESS;
}


void nim_set_pro_path_gain(UINT32 sym,UINT8 fec)
{
    UINT8 data = 0;
    UINT8 data1 = 0;

    if (sys_ic_get_rev_id() == IC_REV_0)
    {
        if (((0x04 == fec) && (sym <= SYM_1200)) || ((0x05 == fec) && (sym <= SYM_1300)) || \
            ((0x06 == fec) && (sym <= SYM_1500)) || ((0x07 == fec) && (sym <= SYM_1500)) || \
            ((0x08 == fec) && (sym <= SYM_1800)) )
        {
            data = 0x26;
            data1 = 0x12;
            nim_s3281_dvbc_write(NIM_S3202_CR_PRO_PATH_GAIN_3, &data, 1);
            nim_s3281_dvbc_write(NIM_S3202_CR_PRO_PATH_GAIN_4, &data1, 1);
        }
        else
        {
            data = 0x24;
            data1 = 0x11;
            nim_s3281_dvbc_write(NIM_S3202_CR_PRO_PATH_GAIN_3, &data, 1);
            nim_s3281_dvbc_write(NIM_S3202_CR_PRO_PATH_GAIN_4, &data1, 1);
        }
    }

}

void nim_set_rs_and_range(UINT32 sym)
{
    UINT32 rs_max = 0;
    UINT32 rs_min = 0;

    //step 3: set symbol rate and symbol rate sweep ranger
    rs_max = (UINT32)((sym * 1076 + 500) / 1024);
    rs_min = (UINT32)((sym * 973 + 500) / 1024);

    nim_s3281_dvbc_set_rs(sym);  // eg. sym = 6900
    nim_s3281_dvbc_set_search_rs(rs_min, rs_max);

}




// berper monitor of s3281
INT32 nim_s3281_dvbc_monitor_berper(struct nim_device *dev, BOOL *bervalid)
{
    UINT8 data = 0;
    UINT8 data_b[3] = {0};
    UINT32 btemp = 0;

    if((NULL == bervalid) || (dev == NULL))
    {
        return ERR_FAILUE;
    }
    //CR62.
    nim_s3281_dvbc_read(NIM_S3281_BERPER_RPT6_R43E, &data, 1);
    if(0 != (data & 0x40))
    {
        data_b[2] = data & 0x3f;
        nim_s3281_dvbc_read(NIM_S3281_BERPER_RPT4_R43C, &data_b[0], 2);
        BER_COUNTS = (UINT32)((data_b[2] << 16 | data_b[1] << 8 | data_b[0]) >> 1);

        nim_s3281_dvbc_read(NIM_S3281_BERPER_RPT3_R43B, &data, 1);
        btemp = (data & 0x7f) << 8;
        nim_s3281_dvbc_read(NIM_S3281_BERPER_RPT2_R43A, &data, 1);
        btemp |= data;
        PER_COUNTS = btemp ;

        // restart the per/ber statistic
        data = 0x80;
        nim_s3281_dvbc_write( NIM_S3281_BERPER_RPT6_R43E, &data, 1);

        return SUCCESS;

    }
    else
    {
        *bervalid = FALSE;
        return ERR_FAILUE;
    }
}

INT32 nim_s3281_dvbc_set_perf_level(struct nim_device *dev, UINT32 level)
{
    UINT8 data[3] = {0};

    if(NULL == dev)
    {
        return ERR_FAILUE;
    }
    switch(level)
    {
    case NIM_PERF_RISK:
        data[0] = 0x32;
        data[1] = 0x32;
        data[2] = 0x46;
        break;
    case NIM_PERF_SAFER:
        data[0] = 0x64;
        data[1] = 0x32;
        data[2] = 0x47;
        break;
    case NIM_PERF_DEFAULT:
    default:
        data[0] = 0xb6;
        data[1] = 0x88;
        data[2] = 0x47;
        break;
    }

    nim_s3281_dvbc_write(NIM_S3202_EQ2, &data[2], 1);
    nim_s3281_dvbc_write(NIM_S3202_EQ17, &data[0], 2);

    return SUCCESS;
}


static void nim_s3281_dvbc_set_rs(UINT32 sym)    //Unit:KBaud
{
    UINT8 data = 0;
    UINT32 sym_cal = 0;

    sym_cal = (UINT32)((sym * 1024 + 500) / 1000); //0.5 for precision reasion.

    //CR39
    data = (UINT8)(sym_cal & 0xFF);
    nim_s3281_dvbc_write(NIM_S3202_FSM2, &data, 1);

    //CR3A
    nim_s3281_dvbc_read(NIM_S3202_FSM3, &data, 1);
    data &= 0xe0;
    data |= (UINT8)((sym_cal >> 8) & 0x1f) ;
    nim_s3281_dvbc_write(NIM_S3202_FSM3, &data, 1);
}

void nim_s3281_dvbc_set_search_freq(UINT16 freq_range)    //+/-KHz
{
    UINT8 data = 0;
    UINT16 fr_range_cal = 0;

    //kent
    fr_range_cal = (UINT16)((freq_range * 1024 + 500) / 1000);

    //CR44.
    data = (UINT8)(fr_range_cal & 0xff);
    nim_s3281_dvbc_write(NIM_S3202_FSM13, &data, 1);
    //CR45.
    nim_s3281_dvbc_read(NIM_S3202_FSM14, &data, 1);
    data = (UINT8)((data & 0xf0) | ((fr_range_cal >> 8) & 0x0f));
    nim_s3281_dvbc_write(NIM_S3202_FSM14, &data, 1);

    //CR42
    nim_s3281_dvbc_read(NIM_S3202_FSM11, &data, 1);
    if (fr_range_cal == 0)//"0" means no freq sweep need. disable the sweep function.
    {
        data &= 0x7f;
    }
    else//enable the tr_freq sweep function.
    {
        data |= 0x80;
    }
    nim_s3281_dvbc_write(NIM_S3202_FSM11, &data, 1);
}

INT32 nim_s3281_fsm_reset(struct nim_device *dev)
{
    UINT8 data = 0;

    if( NULL == dev)
    {
       return RET_FAILURE;
    }

    //step 1: set receiver to IDLE status, disable enable block signal
    data = 0x80;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);
    //step2: start FSM from start AGC1
    data = 0x40;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);

    S3281_PRINTF( " QAM FSM RESET!!!");
    return SUCCESS;
}

void nim_s3281_dvbc_set_search_rs(UINT16 rs_min, UINT16 rs_max)	//KBaud
{
    UINT8 data = 0;
    UINT16 rs_min_cal = 0;
    UINT16 rs_max_cal = 0;

    if((rs_min>rs_max) || ((!rs_min) && (!rs_max)))
    {
	    return;
	}	
    //kent
    rs_min_cal = (UINT16)((rs_min * 1024 + 500) / 1000);
    rs_max_cal = (UINT16)((rs_max * 1024 + 500) / 1000);

    //CR40
    data = (UINT8)(rs_min_cal & 0xff);
    nim_s3281_dvbc_write(NIM_S3202_FSM9, &data, 1);
    //CR41
    nim_s3281_dvbc_read(NIM_S3202_FSM10, &data, 1);
    data = (UINT8)((data & 0xe0) | ((rs_min_cal >> 8) & 0x1f));
    nim_s3281_dvbc_write(NIM_S3202_FSM10, &data, 1 );

    //CR3E
    data = (UINT8)(rs_max_cal & 0xff);
    nim_s3281_dvbc_write(NIM_S3202_FSM7, &data, 1);
    //CR3F
    nim_s3281_dvbc_read(NIM_S3202_FSM8, &data, 1);
    data = (UINT8)((data & 0xe0) | ((rs_max_cal >> 8) & 0x1f));
    nim_s3281_dvbc_write(NIM_S3202_FSM8, &data, 1);
}

void nim_s3281_dvbc_set_delfreq(INT16 delfreq)    //+/-KHz
{
    UINT8 data = 0;
    INT16 delfrq_cal = 0;

    delfrq_cal = (INT16)(delfreq * 1024 / 1000); //linux

    if(delfrq_cal > DELFRQ_CAL_MAX_4095) //13bit signed value, max positive is 4095.
    {
        delfrq_cal = 4095;
     }
    else if(delfrq_cal < -4096)
    {
        delfrq_cal = -4096;
     }

    //CR3B
    data = (UINT8)(delfrq_cal & 0xff);
    nim_s3281_dvbc_write(NIM_S3202_FSM4, &data, 1);
    //CR3C
    nim_s3281_dvbc_read(NIM_S3202_FSM5, &data, 1);
    data = (UINT8)((data & 0xe0) | ((delfrq_cal >> 8) & 0x1f));
    nim_s3281_dvbc_write(NIM_S3202_FSM5, &data, 1);
}



/*============================================================================*/
/**
* \fn UINT32 Log10Times100( UINT32 x)
* \brief Compute: 100*log10(x)
* \param x 32 bits
* \return 100*log10(x)
*
* 100*log10(x)
* = 100*(log2(x)/log2(10)))
* = (100*(2^15)*log2(x))/((2^15)*log2(10))
* = ((200*(2^15)*log2(x))/((2^15)*log2(10)))/2
* = ((200*(2^15)*(log2(x/y)+log2(y)))/((2^15)*log2(10)))/2
* = ((200*(2^15)*log2(x/y))+(200*(2^15)*log2(y)))/((2^15)*log2(10)))/2
*
* where y = 2^k and 1<= (x/y) < 2
*/

UINT32 log10times100_l(UINT32 x)
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




INT32 nim_s3281_dvbc_set_tuner(struct nim_device *dev,NIM_CHANNEL_CHANGE_T *pstchl_change)
{
    struct nim_s3281_private *dev_priv = NULL;
    UINT32 freq = 0;
    UINT32 sym = 0;

    if((NULL == dev) || (pstchl_change == NULL))
    {
        return ERR_FAILUE;
    }

    freq = pstchl_change->freq;
    sym = pstchl_change->sym;

    dev_priv = (struct nim_s3281_private *)dev->priv;
    dev_priv->nim_tuner_control(dev_priv->tuner_id, freq, sym) ;

    return  SUCCESS;
}


