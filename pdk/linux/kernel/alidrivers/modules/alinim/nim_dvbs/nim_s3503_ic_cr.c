/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_s3503_ic_02.c
*
*    Description:  s3503 ic layer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "nim_s3503.h"


static UINT8   	va_22k = 0x00;
static UINT8 last_fec_lock_st = 0x00;


#define SYM_10000       10000

#define SYM_5000        5000

#define SYM_17000       17000

#define SYM_1500        1500
#define CNT_NUM_8       8
#define BIT_RATE_512    512
#define CLK_SEL_96      96
#define DISEQC_DELAY    60   //adjust timing for DiSEqC circuit


static UINT8 cr_para_en_flag = 0;
static UINT8 last_lock_st = 0x00;





INT32 nim_s3503_cr_new_adaptive_unlock_monitor(struct nim_device *dev)
{
    // this function is to lock when strong noise
    UINT8 rdata[2]={0};
    UINT8 data = 0;
    UINT16 est_noise = 0 ; // here must be test again
    UINT16 rdata16 = 0;
    UINT8 fec_lock_st = 0x00;

    UINT8 cr_adpt_choice = 0;
    UINT8 cr_para_en = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	

    nim_reg_read(dev, r13b_est_noise, rdata, 2);
    rdata16 = rdata[1];
    est_noise = (rdata[0] | (rdata16 << 8)) >> 4; // here must be test again

    nim_reg_read(dev, R04_STATUS, &fec_lock_st, 1);
    fec_lock_st &= 0x3f; // complicable with DVBS

    nim_reg_read(dev, 0x13a, &cr_adpt_choice, 1);
    cr_adpt_choice = cr_adpt_choice >> 4;
    cr_adpt_choice &= 0x03;// when using OldAdpt, dont need to change coefficients
    nim_reg_read(dev, 0x113, &cr_para_en, 1);
    cr_para_en &= 0x02; // Only for new adpt

    if((est_noise == 0x1ff) && (fec_lock_st != 0x3f) && (cr_adpt_choice != 0x02) &&
        ((cr_para_en == 0x02) | cr_para_en_flag))
    {
        // add by hongyu for CR LOCK Based on NEW ADPT Begin
        nim_reg_read(dev, RB5_CR_PRS_TRA, &data, 1); //prs
        data &= 0x0f;
        data |= 0x70;
        nim_reg_write(dev, RB5_CR_PRS_TRA, &data, 1);
        nim_reg_read(dev, 0x37, &data, 1); //irs
        data &= 0x83;
        data |= 0x48;
        nim_reg_write(dev, 0x37, &data, 1);
        nim_reg_read(dev, RE0_PPLL_CTRL, &data, 1); //sbh
        data |= 0x20;// enable symbol before head
        data &= 0xf7;// disable force_old_cr
        nim_reg_write(dev, RE0_PPLL_CTRL, &data, 1);
        nim_reg_read(dev, 0x127, &data, 1); //hpg
        data &= 0x8f;
        nim_reg_write(dev, 0x127, &data, 1);
        nim_reg_read(dev, 0x137, &data, 1); //clip_ped
        data &= 0xee;
        nim_reg_write(dev, 0x137, &data, 1);
        // add by hongyu for CR LOCK Based on NEW ADPT end

        //data = 0x20;
        //nim_reg_write(dev,0x13a,&data,1);

        nim_reg_read(dev, 0x113, &data, 1);
        data &= 0xfd;
        nim_reg_write(dev, 0x113, &data, 1);
        cr_para_en_flag = 1;
        //comm_sleep(1000); // waiting for CR LOCK


        ADPT_NEW_CR_PRINTF("strong phase noise happened and try register parameters\n");
    }
    // ADPT_NEW_CR_PRINTF("last_fec_lock_st is %x\nfec_lock_st is %x\ncr_adpt_choice is %x\ncr_para_en_flag is %x\n ");
    if((last_fec_lock_st != 0x3f) && (fec_lock_st == 0x3f)  && cr_para_en_flag)
    {
        nim_reg_read(dev, 0x113, &data, 1);
        data |= 0x02;
        nim_reg_write(dev, 0x113, &data, 1);
        ADPT_NEW_CR_PRINTF("change to NEW ADPT\n");
        cr_para_en_flag = 0;
    }
    last_fec_lock_st = fec_lock_st;
    return  SUCCESS;
}



/*****************************************************************************
* static INT32 nim_s3503_cr_set_phase_noise(struct nim_device *dev)
* Description: S3501 set phase noise for OLD CR mode
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 flag
*
* Return Value: SUCCESS
*****************************************************************************/
INT32 nim_s3503_cr_set_phase_noise(struct nim_device *dev)
{
    return SUCCESS;
}

INT32 nim_s3503_set_32apsk_target(struct nim_device *dev)
{
    /******************************************************************************
    * fuction describle:
    *	this fuction is used to initial the value of the S2_32APSK_TARGET
    *	because of test results show that perfomance of c3503 fpga is better when disabled then enabled,
    *	this function dont need to be called.
    *
    *
    *	          	 FORCE_OLD_DEMAP_AGC3 = reg_cre0[6];		init:	0
    *               	 DIRECT_POWER_EN= reg_creb[3],			init:	1
    *
    *                 	 S2_Q8PSK_SKIP_DATA,
    *                  	 S2_16APSK_SKIP_DATA_PON=reg_creb[1],	init:	0
    *                 	 S2_16APSK_SKIP_DATA_POFF= reg_creb[2],	init:	0
    *                  	 S2_32APSK_SKIP_DATA_PON= reg_cre9[4],	init:	0
    *                    S2_32APSK_SKIP_DATA_POFF= reg_cre9[5], 	init:	0
    *          [6:0]   S2_16APSK_TARGET= reg_crec[6:0],		init:	0x1d
    *          [6:0]   S2_32APSK_TARGET=reg_crea[6:0],		init:	0x1d
    *          [6:0]   S_TARGET= reg_cre7[6:0],				init:	0x30
    *          [6:0]   S2_TARGET= reg_cre8[6:0],			init:	0x29
    ******************************************************************************/
    UINT8 rdata = 0;
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	

    //FORCE_OLD_DEMAP_AGC3
    nim_reg_read(dev, 0xe0, &rdata, 1);
    rdata &= 0xbf;
    nim_reg_write(dev, 0xe0, &rdata, 1);

    // DIRECT_POWER_EN S2_16APSK_SKIP_DATA_PON S2_16APSK_SKIP_DATA_POFF
    nim_reg_read(dev, 0xeb, &rdata, 1);
    rdata &= 0xf6;
    rdata |= 0x08;
    nim_reg_write(dev, 0xeb, &rdata, 1);

    //S2_32APSK_SKIP_DATA_PON  S2_32APSK_SKIP_DATA_POFF
    nim_reg_read(dev, 0xe9, &rdata, 1);
    rdata &= 0xcf;
    nim_reg_write(dev, 0xe9, &rdata, 1);

    //S2_16APSK_TARGET
    nim_reg_read(dev, 0xec, &rdata, 1);
    rdata |= 0x1d;
    nim_reg_write(dev, 0xec, &rdata, 1);

    //S2_32APSK_TARGET
    nim_reg_read(dev, 0xea, &rdata, 1);
    rdata |= 0x1d;
    nim_reg_write(dev, 0xea, &rdata, 1);

    // S_TARGET
    nim_reg_read(dev, 0xe7, &rdata, 1);
    rdata |= 0x30;
    nim_reg_write(dev, 0xe7, &rdata, 1);

    // S2_TARGET
    nim_reg_read(dev, 0xe8, &rdata, 1);
    rdata |= 0x29;
    nim_reg_write(dev, 0xe8, &rdata, 1);

    return SUCCESS;
}


INT32 nim_s3503_cr_adaptive_method_choice(struct nim_device *dev, UINT8 choice_type)
{
    /******************************************************************************
    	* choice_type = 0: only choice NEW CR ADPT method
    	* choice_type = 1: only choice OLD CR ADPT method
    	* choice_type = 2: part choice NEW CR ADPT and part choice OLD ADPT method
    	* choice_type = 3: using the value of regfile
    ******************************************************************************/
    /*
    			ADPT_CR_METHOD_CHOICE_2_ALL=0	reg_cr13a[4]=0
    			ADPT_CR_METHOD_CHOICE_1_ALL=0	reg_cr13a[5]=0
    			ADPT_CR_PARA_EN1 = 1		reg_cr113[1]=1
    */
    UINT8 rdata = 0;
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	

    switch(choice_type)// only choice NEW CR ADPT method
    {
    case 0:
        nim_reg_read(dev, 0x13a, &rdata, 1);
        rdata |= 0x10;
        nim_reg_write(dev, 0x13a, &rdata, 1);
        nim_reg_read(dev, 0x113, &rdata, 1);
        rdata |= 0x02;
        nim_reg_write(dev, 0x113, &rdata, 1);
        break;
    case 1: //only choice OLD CR ADPT method
        nim_reg_read(dev, 0x13a, &rdata, 1);
        rdata |= 0x20;
        rdata &= 0xef;
        nim_reg_write(dev, 0x13a, &rdata, 1);
        nim_reg_read(dev, 0x12e, &rdata, 1); // ap1e=0 ap2e=0
        rdata &= 0xcf;
        nim_reg_write(dev, 0x12e, &rdata, 1);
        nim_reg_read(dev, 0x137, &rdata, 1); //cp1e=0 cp2e=0
        rdata &= 0xee;
        nim_reg_write(dev, 0x137, &rdata, 1);
        nim_reg_read(dev, 0x113, &rdata, 1);
        rdata |= 0x02;
        nim_reg_write(dev, 0x113, &rdata, 1);
        break;
    case 2:// if you want to use this circumstance, you need to edit again
        nim_reg_read(dev, 0x13a, &rdata, 1);
        rdata &= 0xcf;
        nim_reg_write(dev, 0x13a, &rdata, 1);
        nim_reg_read(dev, 0x113, &rdata, 1);
        rdata |= 0x02;
        nim_reg_write(dev, 0x113, &rdata, 1);
        // you need edit begin here
        //
        //ADPT_CR_METHOD_CHOICE[i] = 1 or 0
        //
        break;
    case 3://using the value of regfile
        nim_reg_read(dev, 0x13a, &rdata, 1);
        rdata |= 0x10;
        nim_reg_write(dev, 0x13a, &rdata, 1);
        nim_reg_read(dev, 0x113, &rdata, 1);
        rdata &= 0xfd;
        nim_reg_write(dev, 0x113, &rdata, 1);
        break;
    default:// choice NEW CR ADPT method
        nim_reg_read(dev, 0x13a, &rdata, 1);
        rdata |= 0x10;
        nim_reg_write(dev, 0x13a, &rdata, 1);
        nim_reg_read(dev, 0x113, &rdata, 1);
        rdata |= 0x02;
        nim_reg_write(dev, 0x113, &rdata, 1);
        break;
    }
    return SUCCESS;
}
/*****************************************************************************
* static void nim_c3503_cr_new_adaptive_monitor(struct nim_device *dev)
* Get main parameters of CR and give snr_est value to estimate c/n
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static void nim_s3503_cr_new_adaptive_monitor(struct nim_device *dev)
{
    UINT8 rdata[4] = {0};
    UINT8 adpt_llr_shift = 0;
    UINT8 adpt_symbol_before_head = 0;
    UINT8 adpt_wider_loop_en = 0;
    UINT8 adpt_wider_irs_delta = 0;
    UINT8 adpt_avg_head_2half_en = 0;
    UINT8 adpt_wider_prs_delta = 0;
    UINT8 adpt_force_old_cr = 0;
    UINT8 adpt_irs_tra_frac = 0;
    UINT8 adpt_prs_tra_frac = 0;
    UINT8 adpt_cr_prs_tra_s2 = 0;
    UINT8 adpt_cr_head_ped_gain = 0;
    UINT8 adpt_cr_irs_tra_s2 = 0;
    UINT8 adpt_avg_ped1_en = 0;
    UINT8 adpt_avg_ped2_en = 0;
    UINT8 adpt_clip_ped1_en = 0;
    UINT8 adpt_clip_ped2_en = 0;
    UINT8 adpt_avg_ped1 = 0;
    UINT8 adpt_avg_ped2 = 0;
    UINT8 adpt_clip_ped1 = 0;
    UINT8 adpt_clip_ped2 = 0;

    if(dev == NULL)
    {
	    return;
	}	
    nim_reg_read(dev, r13d_adpt_cr_para_0, rdata, 3);
    // reg_cr13d
    adpt_llr_shift = rdata[0] & 0x01; //ls .
    adpt_symbol_before_head = (rdata[0] >> 1) & 0x01; //sbh .
    adpt_wider_loop_en = (rdata[0] >> 2) & 0x01; //wle .
    // reg_cr13e
    adpt_wider_irs_delta = rdata[1] & 0x07; //wid .
    adpt_avg_head_2half_en = (rdata[1] >> 3) & 0x01; //ah2e .
    adpt_wider_prs_delta = (rdata[1] >> 4) & 0x07; //wpd .
    adpt_force_old_cr = (rdata[1] >> 7) & 0x01; //old .
    // reg_cr13f
    adpt_irs_tra_frac = rdata[2] & 0x03; //itf .
    adpt_prs_tra_frac = (rdata[2] >> 2) & 0x03; //ptf .
    adpt_cr_prs_tra_s2 = (rdata[2] >> 4) & 0x0f; //p .

    nim_reg_read(dev, R140_ADPT_CR_PARA_1, rdata, 4);
    // reg_cr140
    adpt_cr_head_ped_gain = rdata[0] & 0x07; //hpg .
    adpt_cr_irs_tra_s2 = (rdata[0] >> 3) & 0x1f;
    //// reg_cr141
    adpt_avg_ped1_en = rdata[1] & 0x01; //ap1e .
    adpt_avg_ped2_en = (rdata[1] >> 1) & 0x01;//ap2e .
    adpt_clip_ped1_en    = (rdata[1] >> 2) & 0x01;//cp1e .
    adpt_clip_ped2_en    = (rdata[1] >> 3) & 0x01; //cp2e .
    // reg_cr142 & reg_cr143
    adpt_avg_ped1 = rdata[2]; //ap1 .
    adpt_avg_ped2 = rdata[3]; //ap2 .

    nim_reg_read(dev, R144_ADPT_CR_PARA_2, rdata, 2);
    // reg_cr144 & reg_cr145
    adpt_clip_ped1 = rdata[0]; //cp1.
    adpt_clip_ped2 = rdata[1]; //cp2 .


    ADPT_NEW_CR_PRINTF("p=%x i=%x ptf=%x itf=%x ap1e=%x ap1=%x ap2e=%x ap2=%x cp1e=%x \
                       cp1=%x cp2e=%x cp2=%x hpg=%x sbh=%x ah2e=%x ls=%x old=%x wpd=%x wid=%x wle=%x\n",
                       adpt_cr_prs_tra_s2,
                       adpt_cr_irs_tra_s2,
                       adpt_prs_tra_frac,
                       adpt_irs_tra_frac,
                       adpt_avg_ped1_en,
                       adpt_avg_ped1,
                       adpt_avg_ped2_en,
                       adpt_avg_ped2,
                       adpt_clip_ped1_en,
                       adpt_clip_ped1,
                       adpt_clip_ped2_en,
                       adpt_clip_ped2,
                       adpt_cr_head_ped_gain,
                       adpt_symbol_before_head,
                       adpt_avg_head_2half_en,
                       adpt_llr_shift,
                       adpt_force_old_cr,
                       adpt_wider_prs_delta,
                       adpt_wider_irs_delta,
                       adpt_wider_loop_en
                      );
#ifdef NEW_CR_ADPT_SNR_EST_RPT
    //nim_c3503_get_snr_cn(dev);
#endif

}


/*****************************************************************************
* static INT32 nim_s3503_set_12v(struct nim_device *dev, UINT8 flag)
* Description: S3501 set LNB votage 12V enable or not
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 flag
*
* Return Value: SUCCESS
*****************************************************************************/
INT32 nim_s3503_set_12v(struct nim_device *dev, UINT8 flag)
{
    return SUCCESS;
}


/*****************************************************************************
* void nim_s3503_FFT_set_para(struct nim_device *dev)
* Description: S3501 set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
void nim_s3503_fft_set_para(struct nim_device *dev)
{
    return;
}




/*****************************************************************************
* INT32 nim_s3503_DiSEqC_initial (struct nim_device *dev)
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

INT32 nim_s3503_di_seq_c_initial(struct nim_device *dev)
{
    UINT8 data = 0;
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    NIM_PRINTF("Diseqc initial \n");

    if(priv->ul_status.m_s3501_sub_type == NIM_S3503_SUB_ID)
    {
        // Set 22K clock ratio default 96MHz
        data = 0x60;
        nim_reg_write(dev, R90_DISEQC_CLK_RATIO, &data, 1);
    }

    // Diseqc time setting
    //4   DISEQC_HV_OEJ
    //3   DISEQC_OUT_OEJ
    //2   DISEQC_COMP_DIN_EN
    //1   DISEQC_ALIGN_EN
    //0   DISEQC_COMP_EN
    data = 0x02;
    nim_reg_write(dev, R114_DISEQC_TIME_SET, &data, 1);
    // For C3503 only
    if(priv->ul_status.m_s3501_sub_type == NIM_C3503_SUB_ID)
    {
        // enable Diseqc auto select clock ratio.
        nim_reg_read (dev, RF1_DSP_CLK_CTRL, &data, 1);
        data = data | 0x80;
        nim_reg_write(dev, RF1_DSP_CLK_CTRL, &data, 1);
    }
    data = 0xff;
    nim_reg_write(dev, R8E_DISEQC_TIME + 0x01, &data, 1);
#ifdef DISEQC_OUT_INVERT
    // invert diseqc out.
    nim_reg_read(dev, R90_DISEQC_CLK_RATIO + 6, &data, 1);
    data = (data | 0x80);
    nim_reg_write(dev, R90_DISEQC_CLK_RATIO + 6, &data, 1);
#endif

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3503_DiSEqC_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt)
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
INT32 nim_s3503_di_seq_c_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt)
{
    UINT8 data = 0;
    UINT8 temp = 0;
    UINT16 timeout = 0;
    UINT16 timer = 0;
    UINT8 i = 0;

    NIM_PRINTF("mode = 0x%d\n", mode);

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
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
* INT32 nim_s3503_DiSEqC2X_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt, \
*   							UINT8 *rt_value, UINT8 *rt_cnt)
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
INT32 nim_s3503_di_seq_c2x_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt,
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
	    return RET_FAILURE;
	}	
    switch (mode)
    {
    case NIM_DISEQC_MODE_BYTES:
        if((cmd == NULL) || (rt_value == NULL) || (rt_cnt == NULL))
        {
		    return RET_FAILURE;
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
        nim_reg_read(dev, R02_IERR, &data, 1);   //设置diseqc中断监控
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
        while (timer < timeout)    //判断是否超时
        {
            nim_reg_read(dev, R02_IERR, &data, 1);
            if (0x80 == (data & 0x80))   //diseqc传输是否完成
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

/*****************************************************************************
* INT32 nim_s3503_set_polar(struct nim_device *dev, UINT8 polar)
* Description: S3501 set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
//#define REVERT_POLAR
INT32 nim_s3503_set_polar(struct nim_device *dev, UINT8 polar)
{
    UINT8 data = 0;
	struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

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
            NIM_PRINTF("nim_s3503_set_polar CR5C is 00\n");
        }
        else if (NIM_PORLAR_VERTICAL == polar)
        {
            data |= 0x40;
            NIM_PRINTF("nim_s3503_set_polar CR5C is 40\n");
        }
        else
        {
            NIM_PRINTF("nim_s3503_set_polar error\n");
            return ERR_FAILUE;
        }
    }
    else//exist H/V polarity revert.
    {
        if (NIM_PORLAR_HORIZONTAL == polar)
        {
            data |= 0x40;
            NIM_PRINTF("nim_s3503_set_polar CR5C is 40\n");
        }
        else if (NIM_PORLAR_VERTICAL == polar)
        {
            data &= 0xBF;
            NIM_PRINTF("nim_s3503_set_polar CR5C is 00\n");
        }
        else
        {
            NIM_PRINTF("nim_s3503_set_polar error\n");
            return ERR_FAILUE;
        }
    }

    nim_reg_write(dev, R7C_DISEQC_CTRL, &data, 1);
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_cr_adaptive_initial (struct nim_device *dev)
*
*
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_cr_adaptive_initial (struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
#ifdef SW_ADPT_CR
    // Just for debug now, should depends on MODCOD
    data = 0xc3;
    nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1); // enable SNR estimation
    data = 0x9e;
    nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1); // Bypass Adpt ForceOldCR
    nim_reg_read(dev, RE0_PPLL_CTRL, &data, 1);
    data |= 0x08;
    nim_reg_write(dev, RE0_PPLL_CTRL, &data, 1); // force old CR, just for debug now
#endif

#ifdef HW_ADPT_CR

    nim_s3503_cr_tab_init(dev);
    //Bypass Setting: All use CR adaptive table
    data = 0x81;
    nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);

#ifdef HW_ADPT_CR_MONITOR
    data = 0xe6; // enable hw adpt CR report, also enable always update to regfile, latter need review
    nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);
#endif
#endif

    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_cr_adaptive_configure (struct nim_device *dev)
*
*
*  Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_cr_adaptive_configure (struct nim_device *dev, UINT32 sym)
{
#ifdef HW_ADPT_CR

    UINT8 data = 0;
    UINT8 tabid = 0;
    UINT32 tabval = 0;
    UINT32 tabvaltemp = 0;
    UINT8 datarray[2]={0};

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
#ifdef SW_ADPT_CR
    s3503_snr_initial_en = 1;
#endif // SW_ADPT_CR


    NIM_PRINTF("            nim m3501c close dummy for dvbs \n");

    // To circumvent the symbol rate 13M problem
    /*  TAB_ID=15, cellid = 2
        WIDE_2 |  Reserved  |<------8PSK POFF-------->|  S2_QPSK |        |
                            |9/10 8/9 5/6 3/4 2/3 3/5 |PON POFF  | DVBS
        0x002 :             |0    0   0   0   0   0   |0   1     | 0        */

    // config ON/OFF Table of WIDE_2
    data = 0xe1; // CR Tab init en
    nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);

    data = 0xf0 | 2; // TAB_ID | cellid
    nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
    if(sym <= SYM_10000) // enable wider CR when S2 QPSK rs<=10M
        datarray[0] = 0x02;
    else
        datarray[0] = 0x00;
    datarray[1] = 0x80;
    nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);
    // End of to circumvent the symbol rate 13M problem

    // PRS=8,IRS=0x12 for 8PSK pilot off rs<3M
    if(sym <= SYM_5000)
        tabval = 0x10600;
    else if((sym > 5000) && (sym < 17000)) //for pn_astraf.fcf 20M symbol rate performance worse than C3501C 20121204
        //    tabval = cr_para_8psk_others[TAB_SIZE-1];
        tabval = 0x10700;
    else
        tabval = 0x10800;
    tabvaltemp = (tabval & 0x7c) << 7;
    tabval = (tabval >> 8) | tabvaltemp | 0x8000; // to HW format;
    datarray[0] = (UINT8)(tabval & 0x0ff);
    datarray[1] = (UINT8)((tabval >> 8) & 0x0ff);

    for(tabid = 4; tabid <= 6; tabid++)
    {
        data = (tabid << 4) | (PSK8_TAB_SIZE - 1); //12.5
        nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
        nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

        data = (tabid << 4) | (PSK8_TAB_SIZE - 2); //12.0
        nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
        nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

        data = (tabid << 4) | (PSK8_TAB_SIZE - 3); //11.5
        nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
        nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);
    }
    if(sym >= SYM_17000)
    {
        for(tabid = 4; tabid <= 6; tabid++)
        {
            data = (tabid << 4) | (PSK8_TAB_SIZE - 4); //11.0
            nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
            nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);
            data = (tabid << 4) | (PSK8_TAB_SIZE - 5); //10.5
            nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
            nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

            data = (tabid << 4) | (PSK8_TAB_SIZE - 6); //10.0
            nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
            nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);
            data = (tabid << 4) | (PSK8_TAB_SIZE - 7); //9.5
            nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
            nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);
        }
    }
    //end of rs<3M issue
    if(sym <= SYM_1500)
    {
        data = 0xf0 | 0;        //force old cr
        nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
        tabval = 0x19 | 0x8000;
        datarray[0] = (UINT8)(tabval & 0x0ff);
        datarray[1] = (UINT8)((tabval >> 8) & 0x0ff);
        nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

        data = 0xf0 | 3;    //enable snr est
        nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1);
        tabval = 0x7fa | 0x8000;
        datarray[0] = (UINT8)(tabval & 0x0ff);
        datarray[1] = (UINT8)((tabval >> 8) & 0x0ff);
        nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

        //pilot_on_gain=0,head_ped_gain=0 is same as M3501B for 1M pilot on lock slow issue
        nim_reg_read(dev, 0x127, &data, 1);
        data = data & 0x83;
        nim_reg_write(dev, 0x127, &data, 1);
    }

#ifdef HW_ADPT_CR_MONITOR
    data = 0xe6; // enable hw adpt CR report, also enable always update to regfile, latter need review
#else
    data = 0xe0; // CR Tab init off
#endif
    nim_reg_write(dev, R0E_ADPT_CR_CTRL, &data, 1);
#endif // HW_ADPT_CR
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_tso_initial (struct nim_device *dev)
*
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_tso_initial (struct nim_device *dev, UINT8 insert_dummy, UINT8 tso_mode)
{
    UINT8 data = 0;
	struct nim_s3501_private *priv = NULL;
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    
	priv = (struct nim_s3501_private *) dev->priv;

    data = 0x12;
    nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
    NIM_PRINTF("            nim m3501c close dummy for dvbs \n");
    switch(tso_mode)
    {
    case 0 :
    {
        data = 0x37;
        // For C3503 only
        if(priv->ul_status.m_s3501_sub_type == NIM_C3503_SUB_ID)
        {
            data = data ^ 0x02;
        }
        nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
        NIM_PRINTF("            tso_mode is SSI \n");
        break;
    }
    case 1 :
    {
        data = 0x27;
        // For C3503 only
        if(priv->ul_status.m_s3501_sub_type == NIM_C3503_SUB_ID)
        {
            data = data ^ 0x02;
        }
        nim_reg_write(dev, RD8_TS_OUT_SETTING, &data, 1);
        NIM_PRINTF("            tso_mode is SPI \n");
        break;
    }
    default:
        break;
    }
    /*
    // For C3503 only
        if(priv->ul_status.m_s3501_sub_type == NIM_C3503_SUB_ID)
        {
            // Default ERRJ should be 0
            // Change ERRJ to 1 with bellow setting.
            nim_reg_read (dev, RF1_DSP_CLK_CTRL+4, &data, 1);
            data = data | 0x01;
            nim_reg_write(dev, RF1_DSP_CLK_CTRL+4, &data, 1);
        }
    */
    return SUCCESS;
}



/*****************************************************************************
* static INT32 nim_s3503_tso_dummy_off (struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_tso_dummy_off (struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    nim_reg_read(dev, RF0_HW_TSO_CTRL, &data, 1);
    data = data & 0xfd;
    nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
    NIM_PRINTF("            nim m3503 close dummy for dvbs \n");
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_tso_dummy_on (struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_tso_dummy_on (struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    nim_reg_read(dev, RF0_HW_TSO_CTRL, &data, 1);
    data = data | 0x02;
    nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
    //clear full then send mode of TSO
    nim_reg_read(dev, RF0_HW_TSO_CTRL, &data, 1);
    data = data & 0xbf;
    nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);
    NIM_PRINTF("            nim m3503 insert dummy\n");
    return SUCCESS;
}




/*****************************************************************************
* static INT32 nim_s3503_tso_soft_cbr_off (struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_tso_soft_cbr_off (struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    nim_reg_read(dev, RF0_HW_TSO_CTRL + 4, &data, 1);
    data = data & 0x7f;
    nim_reg_write(dev, RF0_HW_TSO_CTRL + 4, &data, 1);

    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_tso_off (struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_tso_off (struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    // close fec ts output
    nim_reg_read(dev, RAF_TSOUT_PAD, &data, 1);
    data = data | 0x10;
    nim_reg_write(dev, RAF_TSOUT_PAD, &data, 1);
    //NIM_PRINTF("            fec ts off\n");
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_tso_on (struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_tso_on (struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    // Open fec ts output
    nim_reg_read(dev, RAF_TSOUT_PAD, &data, 1);
    data = data & 0xef;
    nim_reg_write(dev, RAF_TSOUT_PAD, &data, 1);
    NIM_PRINTF("            fec ts on\n");
    return SUCCESS;
}

/*****************************************************************************
* void nim_s3503_set_freq_offset(struct nim_device *dev, INT32 delfreq)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
void nim_s3503_set_freq_offset(struct nim_device *dev, INT32 delfreq)
{
    UINT8 data[3]={0};
    UINT32 temp = 0;
    UINT32 delfreq_abs = 0;
    UINT32 sample_rate = 0;

    if(dev == NULL)
    {
	    return;
	}	
    // struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;
    //delfreq unit is KHz, sample_rate unit is KHz
    if(delfreq < 0)
    {
        delfreq_abs = 0 - delfreq;
    }
    else
    {
        delfreq_abs = delfreq;
     }
    nim_s3503_get_dsp_clk(dev, &sample_rate);
    temp = nim_s3501_multu64div(delfreq_abs, 92160, sample_rate); // 92160 == 90000 * 1.024
    if(temp > 0xffff)
    {
        NIM_PRINTF("\t\t NIM_Error: Symbol rate set error %d\n", temp);
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
    return ;
}
/*****************************************************************************
* static INT32 nim_s3503_tr_setting(struct nim_device *dev, UINT8 s_case)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_tr_setting(struct nim_device *dev, UINT8 s_case)
{
    return SUCCESS;
}


/*****************************************************************************
* static INT32 nim_s3503_cr_setting(struct nim_device *dev, UINT8 s_case)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_cr_setting(struct nim_device *dev, UINT8 s_case)
{
    UINT8 data = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    switch (s_case)
    {
    case NIM_OPTR_SOFT_SEARCH:
        // set CR parameter
        nim_reg_read(dev, RE0_PPLL_CTRL, &data, 1);
        data = data | 0x08;   ///force old CR
        nim_reg_write(dev, RE0_PPLL_CTRL, &data, 1);

        data = 0xaa;
        nim_reg_write(dev, R33_CR_CTRL + 0x03, &data, 1);
        data = 0x45;
        nim_reg_write(dev, R33_CR_CTRL + 0x04, &data, 1);
        data = 0x97;
        nim_reg_write(dev, R33_CR_CTRL + 0x05, &data, 1);
        data = 0x87;
        nim_reg_write(dev, RB5_CR_PRS_TRA, &data, 1);
        break;
    case NIM_OPTR_CHL_CHANGE:
        // set CR parameter
        nim_reg_read(dev, RE0_PPLL_CTRL, &data, 1);
        data = data & 0xf7;  ///disable old CR
        nim_reg_write(dev, RE0_PPLL_CTRL, &data, 1);

        data = 0xaa;
        nim_reg_write(dev, R33_CR_CTRL + 0x03, &data, 1);
        data = 0x4a;
        nim_reg_write(dev, R33_CR_CTRL + 0x04, &data, 1);
        data = 0x86;
        nim_reg_write(dev, R33_CR_CTRL + 0x05, &data, 1);
        data = 0x88;
        nim_reg_write(dev, RB5_CR_PRS_TRA, &data, 1);
        break;
    default:
        NIM_PRINTF(" CMD for nim_s3503_cr_setting ERROR!!!");
        break;
    }

    return SUCCESS;
}

INT32 nim_s3503_hbcd_timeout(struct nim_device *dev, UINT8 s_case)
{
    UINT8 data = 0;
	struct nim_s3501_private *priv =NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;


    PRINTK_INFO("[kangzh]line=%d,%s enter!\n", __LINE__, __FUNCTION__);

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

#if 0
/*****************************************************************************
* static INT32 nim_s3503_ldpc_keep_working_enable(struct nim_device *dev)
* Description: set ldpc to work until a new frame start
*  Arguments:
*  Parameter1: struct nim_device *dev
* Return Value: INT32
*****************************************************************************/

static INT32 nim_s3503_ldpc_keep_working_enable(struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
        return RET_FAILURE;
    nim_reg_read(dev, r12d_ldpc_sel, &data, 1);
    data |= 0x40;
    nim_reg_write(dev, r12d_ldpc_sel, &data, 1);

    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_ldpc_keep_working_disable(struct nim_device *dev)
* Description: set ldpc back to the normal mode
*  Arguments:
*  Parameter1: struct nim_device *dev
* Return Value: INT32
*****************************************************************************/

static INT32 nim_s3503_ldpc_keep_working_disable(struct nim_device *dev)
{
    UINT8 data = 0;

    if(dev == NULL)
        return RET_FAILURE;
    nim_reg_read(dev, r12d_ldpc_sel, &data, 1);
    data &= 0xbf;
    nim_reg_write(dev, r12d_ldpc_sel, &data, 1);

    return SUCCESS;
}



/*****************************************************************************
* static INT32 nim_s3503_fec_set_ldpc(struct nim_device *dev, UINT8 s_case, UINT8 c_ldpc, UINT8 c_fec)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3503_set_ldpc_iter_para(struct nim_device *dev, UINT16 most_cnt, UINT8 least_cnt, UINT8 exit_cnt_thld)
{
    UINT8 data = 0;
    UINT8 temp = 0;

    if(dev == NULL)
        return RET_FAILURE;
    data = (UINT8) most_cnt & 0xff;
    nim_reg_write(dev, R57_LDPC_CTRL, &data, 1);

    temp = (UINT8) ( (most_cnt >> 8) & 0x03 );
    temp <<= 4;
    nim_reg_read(dev, RC1_DVBS2_FEC_LDPC, &data, 1);
    data |= temp;
    nim_reg_write(dev, RC1_DVBS2_FEC_LDPC, &data, 1);

    nim_reg_read(dev, R57_LDPC_CTRL + 1, &data, 1);
    temp = least_cnt & 0x3f;
    data |= temp;
    nim_reg_write(dev, R57_LDPC_CTRL + 1, &data, 1);

    nim_reg_read(dev, R57_LDPC_CTRL + 2, &data, 1);
    temp = exit_cnt_thld & 0x3f;
    data |= temp;
    nim_reg_write(dev, R57_LDPC_CTRL + 2, &data, 1);

    return SUCCESS;
}
#endif

/*****************************************************************************
* static INT32 nim_s3503_fec_set_ldpc(struct nim_device *dev, UINT8 s_case, UINT8 c_ldpc, UINT8 c_fec)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_fec_set_ldpc(struct nim_device *dev, UINT8 s_case, UINT8 c_ldpc, UINT8 c_fec)
{
    UINT8 data = 0;
    UINT8 temp[3] = {0xff, 0xc8, 0x08};

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    // LDPC parameter
    switch (s_case)
    {
    case NIM_OPTR_CHL_CHANGE:
        nim_reg_write(dev, R57_LDPC_CTRL, temp, 3);
        data = 0x30; // disactive ldpc avg iter and ldpc_max_iter_num[9:8]=0x3
        nim_reg_write(dev, RC1_DVBS2_FEC_LDPC, &data, 1);
        break;
    case NIM_OPTR_SOFT_SEARCH:
        temp[1] =  0x48;
        nim_reg_write(dev, R57_LDPC_CTRL, temp, 3);
        data = 0x30;
        nim_reg_write(dev, RC1_DVBS2_FEC_LDPC, &data, 1);
        break;
    default:
        NIM_PRINTF(" CMD for nim_s3503_fec_set_ldpc ERROR!!!");
        break;
    }
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_TR_CR_Setting(struct nim_device *dev, UINT8 s_case)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_tr_cr_setting(struct nim_device *dev, UINT8 s_case)
{
    UINT8 data = 0;
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;

    switch (s_case)
    {
    case NIM_OPTR_SOFT_SEARCH:
        //    if (!(priv->t_Param.t_reg_setting_switch & NIM_SWITCH_TR_CR))
    {
        data = 0x49;//0x4d;
        nim_reg_write(dev, R66_TR_SEARCH, &data, 1);
        data = 0x31;
        nim_reg_write(dev, R67_VB_CR_RETRY, &data, 1);
        //        priv->t_Param.t_reg_setting_switch |= NIM_SWITCH_TR_CR;
    }
    break;
    case NIM_OPTR_CHL_CHANGE:
        if (priv->t_param.t_reg_setting_switch & NIM_SWITCH_TR_CR)
        {
            // set reg to default value
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



INT32 nim_s3503_nframe_step_tso_setting(struct nim_device *dev, UINT32 *sym_rate, UINT8 s_case)
{
    UINT8 rdata[2]={0};

    UINT8 current_lock_st = 0x00;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    nim_reg_read(dev, R04_STATUS, &current_lock_st, 1);
    current_lock_st &= 0x3f;

    if(((current_lock_st != 0x3f) && (s_case == 0x00)) | (s_case == 0x01))
    {
        // s_case==0x00 called by task function
        // s_case==0x01 called by channnel change function
        nim_reg_read(dev, R124_HEAD_DIFF_NFRAME, rdata, 2);
        rdata[0] &= 0x88;
        rdata[0] |= 0x21;
        rdata[1] &= 0x88;
        rdata[1] |= 0x32;
        nim_reg_write(dev, R124_HEAD_DIFF_NFRAME, rdata, 2);
        nim_reg_read(dev, R124_HEAD_DIFF_NFRAME, rdata, 2);
        ADPT_NEW_CR_PRINTF("channel change:\n\tnframe is %x\n\tstep is %x\nTSO is OFF\n", rdata[0], rdata[1]);
        nim_s3503_tso_off(dev);
    }
    if((last_lock_st != 0x3f) && (current_lock_st == 0x3f))
    {
        nim_reg_read(dev, R124_HEAD_DIFF_NFRAME, rdata, 2);
        //ADPT_NEW_CR_PRINTF("all state lock and change nframe/step:\n\tnframe is %x\n\t\
    //                      step is %x\n",rdata[0],rdata[1]);
        rdata[0] &= 0x88;
        rdata[0] |= 0x74;
        rdata[1] &= 0x88;
        rdata[1] |= 0x70;
        nim_reg_write(dev, R124_HEAD_DIFF_NFRAME, rdata, 2);
        nim_reg_read(dev, R124_HEAD_DIFF_NFRAME, rdata, 2);
        ADPT_NEW_CR_PRINTF("normal mode:\n\tnframe is %x\n\tstep is %x\nTSO is ON\n", rdata[0], rdata[1]);
        nim_s3503_tso_on(dev);
    }
    last_lock_st = current_lock_st;


    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_set_dsp_clk (struct nim_device *dev, UINT8 clk_sel)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_set_dsp_clk (struct nim_device *dev, UINT8 clk_sel)
{
    UINT8 data = 0;
	struct nim_s3501_private *priv =NULL;
	
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    //  NIM_PRINTF("Enter Fuction nim_s3503_reg_get_modcode \n");

    if(priv->ul_status.m_s3501_sub_type == NIM_S3503_SUB_ID)
    {
        data = clk_sel & 0xff;
        nim_reg_write(dev, R90_DISEQC_CLK_RATIO, &data, 1);
    }
    nim_reg_read(dev, RF1_DSP_CLK_CTRL, &data, 1);
    data = data & 0xfc;
    // bit[1:0] dsp clk sel: // 00: 135m, 01:96m
    if(CLK_SEL_96 == clk_sel)
    {
        data = data | (UINT8)(1) ;
    }
    else if(135 == clk_sel)
    {
        data = data | (UINT8)(0) ;
    }
    else
    {
        NIM_PRINTF(" set dsp clock error!");
    }

    nim_reg_write(dev, RF1_DSP_CLK_CTRL, &data, 1);

    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_interrupt_mask_clean(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_interrupt_mask_clean(struct nim_device *dev)
{
    UINT8 data = 0x00;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    nim_reg_write(dev, R02_IERR, &data, 1);
    data = 0xff;
    nim_reg_write(dev, R03_IMASK, &data, 1);
    return SUCCESS;
}

/*****************************************************************************
* static INT32 nim_s3503_set_adc(struct nim_device *dev)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_set_adc(struct nim_device *dev)
{
    struct nim_s3501_private *priv = NULL;
    UINT8 data = 0;
    UINT8 ver_data = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    
    priv = (struct nim_s3501_private *) dev->priv;
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

/*****************************************************************************
* static INT32 nim_s3503_set_hw_timeout(struct nim_device *dev, UINT8 time_thr)
* Get bit error ratio
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_s3503_set_hw_timeout(struct nim_device *dev, UINT8 time_thr)
{
    struct nim_s3501_private *priv = NULL;
    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    // AGC1 setting
    if (time_thr != priv->ul_status.m_hw_timeout_thr)
    {
        nim_reg_write(dev, R05_TIMEOUT_TRH, &time_thr, 1);
        priv->ul_status.m_hw_timeout_thr = time_thr;
    }

    return SUCCESS;
}












