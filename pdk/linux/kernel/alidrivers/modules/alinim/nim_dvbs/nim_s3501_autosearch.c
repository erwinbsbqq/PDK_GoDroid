/*****************************************************************************
*    Copyright (C)2003 Ali Corporation. All Rights Reserved.
*
*    File:    nim_s3501_autosearch.c
*
*    Description:    Software blind search source file
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	2005/06/10        Berg Xing      Ver 0.1       Create file.
*	2.  2005/06/16		  Berg Xing      Ver 0.2	   Remove short time hollow impulse.
*	3.  2005/06/21		  Berg Xing		 Ver 0.3       Exclude channels whose symbol rate are
*													   less than 2M.
*	4.  2005/06/27        Berg Xing      Ver 0.4       Code size optimize
*	5.  2005/06/28        Berg Xing      Ver 0.5       Remove warning information when compiling
*	6.  2005/07/15        Berg Xing      Ver 0.6       Modify next frequency offset method
*   7.  2005/08/12        Berg Xing      Ver 0.7       Kill bug for noise_level& signal level estimation
*	8.  2005/08/30        Berg Xing      Ver 0.8       Tune noise_level
*   9.  2005/10/11        Berg Xing      Ver 0.9       Slove lost TP (12460/H/2220 on 166 PANAMSAT-8)
*
*	 Notes:
*****************************************************************************/

#if defined(__NIM_LINUX_PLATFORM__)
#include "../porting_linux_header.h"
#include "porting_m3501_linux.h"
#elif defined(__NIM_TDS_PLATFORM__)
#include "../m3501/porting_tds_header.h"
#include "../m3501/porting_m3501_tds.h"

#endif

#include "nim_s3501_autosearch.h"


#define RET_FOR_CONTINUE 0xFF
#define FS_MAXNUM 15000
#define FFT_POINT_9 9
#define FFT_POINT_1023 1023
#define FFT_POINT_1024 1024
#define FFT_VAL_1200 1200
#define FFT_VAL_700 700
#define FFT_VAL_800 800
#define SIGNAL_LEVEL_2000 2000
#define SIGNAL_LEVEL_5500 5500
#define SIGNAL_CNT_8 8
#define THRESHOLD_4000 4000
#define TEMP_4 4
#define TEMP_70 70
#define TEMP_1020 1020
#define FALL_EDGE_1023 1023
#define START_INDEX_5 5
#define NOISE_NUM_16 16
#define NOISE_LEVEL_750 750
#define NOISE_LEVEL_700 700
#define STEP_NUM_4 4
#define EST_SYM_25000 25000
#define DEL_FREQ_3 3
#define MAX_VAL_27BIT 251658239
#define CNT_VAL_4 4
#define CNT_VAL_15 15
#define CNT_VAL_40 40
#define CNT_VAL_750 750
#define DELTA_VAL_3 3
#define DELTA_VAL_10 10
#define DELTA_VAL_12 12
#define SUM_VAL_2000000 2000000
#define FACTOR_1000000 1000000
#define FFT_FLAG_5 5
#define FALL_EDGE_1023 1023




typedef struct _s3501_autosearch_params
{
	UINT32 start_index;
	UINT32 end_index;
	UINT32 noise_level;
	UINT32 signal_level;
	INT32 ch_number;
	INT32 found_channel;
	UINT32 rise_edge_tmp;
	UINT32 *fall_edge;
	UINT32 *rise_edge;
}NIM_S3501_AUTOSEARCH_PARAMS;


typedef struct _s3501_find_channel_params_t
{
	UINT32 signallevel;
	UINT32 noiselevel;
	UINT32 start;
	UINT32 end;
	UINT32 *zero_position;
}NIM_S3501_FIND_CHANNEL_PARAMS;

INT32	fft_energy_1024[1024] ={0};	
INT32	fft_energy_1024_tmp[1024] ={0};
INT32 	frequency_est[TP_MAXNUM] ={0};
INT32 	symbol_rate_est[TP_MAXNUM] ={0};
INT32 	tp_number = 0;
INT32   *channel_spectrum = NULL;
INT32   *channel_spectrum_tmp = NULL;
INT32   last_tuner_if = 0;
INT32   chlspec_num = 0;
INT32   called_num = 0;
INT32	final_est_freq = 0;
INT32	final_est_sym_rate = 0;
INT32   max_fft_energy = 0;

static UINT8	fft_wave_1024[1024] ={0};
static INT32 fft_energy_1024_value[1024] ={0};
static UINT8 fft_energy_1024_flag[1024] ={0};

static void nim_s3501_smoothfilter_old(void)
{
    INT32 i = 0;
    INT32 j = 0;
    INT32 data_tmp = 0;
    INT32 data_tmp2[5]={0};

    //5 points of start and end of data is excluded out of filter
    //because filter has 11 orders
    for (i = 5; i < 1024 - 5; i++)
    {
        data_tmp = *(fft_energy_1024 + i - 5)    + *(fft_energy_1024 + i + 5)	 ;
        data_tmp += (*(fft_energy_1024 + i - 4) << 1) + (*(fft_energy_1024 + i + 4) << 1);
        data_tmp += (*(fft_energy_1024 + i - 3) << 2) + (*(fft_energy_1024 + i + 3) << 2);
        data_tmp += (*(fft_energy_1024 + i - 2) << 3) + (*(fft_energy_1024 + i + 2) << 3);
        data_tmp += (*(fft_energy_1024 + i - 1) << 4) + (*(fft_energy_1024 + i + 1) << 4);
        data_tmp += (*(fft_energy_1024 + i  ) << 5)							 ;
        data_tmp >>= 6;
        //filter = [1/64 1/32 1/16 1/8 1/4 1/2 1/4 1/8 1/16 1/32 1/64];
        if (i > FFT_POINT_9)
        {
            *(fft_energy_1024 + i - 5) = data_tmp2[0];
        }
        for (j = 0; j < 4; j++)
        {
            data_tmp2[j] = data_tmp2[j + 1];
        }
        if ((data_tmp >> 20) & 1)
        {
            data_tmp2[4] = 1048575; //maximum unsigned value of 20 bits
        }
        else
        {
            data_tmp2[4] = data_tmp;
        }
    }
    for (i = 0; i < 5; i++)
    {
        *(fft_energy_1024 + 1023 + i - 9) = data_tmp2[i];
    }
}

#if 0
static void nim_s3501_smoothfilter1(void)
{
    UINT32 i = 0;
    UINT32 j = 0;
    INT32  data_tmp = 0;
    UINT32  filter_taps = 4;

    for (i = filter_taps - 1; i < 1024; i++)
    {
        data_tmp = 0;
        for (j = 0; j < filter_taps; j++)
        {
            data_tmp += fft_energy_1024[i - j];
            //		data_tmp += fft_tmp[i-j];	// initial cic has no fft_tmp, here use FFT_energy_1024, it is an error
        }
        data_tmp /= filter_taps;
        fft_energy_1024[i] = data_tmp;
    }
    for (i = 0; i < filter_taps - 1; i++)
    {
        fft_energy_1024[i] = fft_energy_1024[filter_taps - 1] ;
    }   

}
#endif

static void nim_s3501_smoothfilter2(void)
{
    UINT32 i = 0;
    UINT32 j = 0;
    INT32  data_tmp = 0;
    UINT32  filter_taps = 4;

    for (i = filter_taps - 1; i < 1024; i++)
    {
        data_tmp = 0;
        if(fft_energy_1024[i] > FFT_VAL_1200 && fft_energy_1024[i + 1] > FFT_VAL_1200 && fft_energy_1024[i - 1] > FFT_VAL_1200)
        {
            for(j = 0; j < filter_taps; j++)
            {
                data_tmp += fft_energy_1024[i - j];
                //		data_tmp += fft_tmp[i-j];	// initial cic has no fft_tmp, here use FFT_energy_1024, it is an error
            }
            data_tmp /= filter_taps;
            fft_energy_1024[i] = data_tmp;
        }
    }
    //	for (i=0;i<filter_taps-1;i++)
    //		FFT_energy_1024[i] = FFT_energy_1024[filter_taps-1] ;

}

static void nim_s3501_smooth_backup(void)
{
    INT32 i = 0;

    for(i = 0; i < 1024; i++)
    {
        fft_energy_1024_tmp[i] = fft_energy_1024[i];
     }  
}
#if 0
static void nim_s3501_smooth_restore(void)
{
    INT32 i = 0;

    for(i = 0; i < 1024; i++)
    {
        fft_energy_1024[i] = fft_energy_1024_tmp[i];
    } 
}
#endif


static void nim_s3501_smoothfilter_new(void)
{
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 min_index = 0;
    UINT32 max_index = 0;
    INT32  data_tmp = 0;
    INT32  t_data_tmp = 0;
    INT32  data_tmp2[5]={0};
    INT32  average_tmp = 0;
    INT32  min_num = 0;
    INT32  max_num = 0;

    UINT32  filter_taps = 4;


    for(i = filter_taps - 1; i < 1024 - 4; i++)
    {
        t_data_tmp = 0;
        if(fft_energy_1024[i] > FFT_VAL_700 ) //&& FFT_energy_1024[i-2] >1900  && FFT_energy_1024[i+2] >1900
        {
            //first change the bigest and smallest num to the average, and then filter
            min_num = fft_energy_1024[i];
            max_num = fft_energy_1024[i];
            for(j = 0; j < filter_taps; j++)
            {
                t_data_tmp += fft_energy_1024[i - j];
                if(min_num > fft_energy_1024[i - j] )
                {
                    min_num = fft_energy_1024[i - j];
                    min_index = j;
                }
                if(max_num < fft_energy_1024[i - j] )
                {
                    max_num = fft_energy_1024[i - j];
                    max_index = j;
                }
                //		data_tmp += fft_tmp[i-j];	// initial cic has no fft_tmp, here use FFT_energy_1024, it is an error
            }
            t_data_tmp /= filter_taps;
            average_tmp = t_data_tmp;
            fft_energy_1024[i - min_index] = average_tmp;
            fft_energy_1024[i - max_index] = average_tmp;

            for(j = 0; j < filter_taps; j++)
            {
                t_data_tmp += fft_energy_1024[i - j];
            }
            t_data_tmp /= filter_taps;
            fft_energy_1024[i - filter_taps / 2] = t_data_tmp;
        }
    }

    for (i = 5; i < 1024 - 5; i++)
    {
        data_tmp = *(fft_energy_1024 + i - 5)    + *(fft_energy_1024 + i + 5)	 ;
        data_tmp += (*(fft_energy_1024 + i - 4) << 1) + (*(fft_energy_1024 + i + 4) << 1);
        data_tmp += (*(fft_energy_1024 + i - 3) << 2) + (*(fft_energy_1024 + i + 3) << 2);
        data_tmp += ((*(fft_energy_1024 + i - 2) << 3) + (*(fft_energy_1024 + i + 2) << 3)) * 2 / 3;
        data_tmp += ((*(fft_energy_1024 + i - 1) << 4) + (*(fft_energy_1024 + i + 1) << 4)) * 2 / 3;
        data_tmp += (*(fft_energy_1024 + i  ) << 5) * 3 / 4;							 ;
        data_tmp >>= 6;
        //filter = [1/64 1/32 1/16 1/8 1/4 1/2 1/4 1/8 1/16 1/32 1/64];
        if (i > FFT_POINT_9)
        {
            //if(FFT_energy_1024[i-5] >1000 && FFT_energy_1024[i-6] >1000 && FFT_energy_1024[i-4]>1000)
            *(fft_energy_1024 + i - 5) = data_tmp2[0];
        }
        for (j = 0; j < 4; j++)
        {
            data_tmp2[j] = data_tmp2[j + 1];
        }
        if ((data_tmp >> 20) & 1)
        {
            data_tmp2[4] = 1048575; //maximum unsigned value of 20 bits
        }
        else
        {
            data_tmp2[4] = data_tmp;
        }
    }
    for (i = 0; i < 5; i++)
    {
        *(fft_energy_1024 + 1023 + i - 9) = data_tmp2[i];
    }
}

void nim_s3501_smoothfilter(void)
{
    nim_s3501_smooth_backup();
    nim_s3501_smoothfilter2();
    nim_s3501_smoothfilter_old();
    //	nim_s3501_smoothfilter2();
}


static UINT32 nim_s3501_find_channel_x(NIM_S3501_FIND_CHANNEL_PARAMS *params)
{
    UINT32 i = 0;
	UINT32 j = 0;
	UINT32 zero_num =0;
	UINT32 compare_thre = 0;
	UINT32 find_zero = 0;
	UINT32 noiselevel_x5 = 0;
	UINT32 temp = 0;
	INT32 diff = 0;
	UINT32 zero_counter = 0;
	UINT32 first_signal_pos = 0;
    UINT32 first_zero_pos = 0;
    UINT32 second_zero_pos = 0;
    UINT32 second_signal_pos = 0;
	UINT32 signal_counter = 0;
	INT32 signal_value = 0; 

    if(params->signallevel > SIGNAL_LEVEL_2000)
    {
        compare_thre = 800;
    }
    else
    {
        compare_thre = params->noiselevel;
    }
    compare_thre = compare_thre * 5; //compare_thre = compare_thre*5;
    noiselevel_x5 = params->noiselevel * 3;
    for(i = 0; i < 4; i++)
    {
        fft_wave_1024[i] = 0;
    }  
    for(i = 1020; i < 1024; i++)
    {
        fft_wave_1024[i] = 0;
    }

    temp = fft_energy_1024[3] + fft_energy_1024[3 + 1] + fft_energy_1024[3 - 1] + fft_energy_1024[3 + 2] \
                + fft_energy_1024[3 - 2];
    for(i = 4; i < 1020; i++)
    {
        
        temp = temp + fft_energy_1024[i + 2] - fft_energy_1024[i - 3];
        //		diff = diff/5;
        //		diff = diff - noiselevel;
        diff = temp - noiselevel_x5;
        if(diff < 0)
        {
	    diff = -diff;
        }
        /*
        if(diff<compare_thre)
        FFT_wave_1024[i] = 0;
        else
        FFT_wave_1024[i] = 255;
        */
        fft_wave_1024[i] = (UINT32)diff < compare_thre ? 0 : 255;
    }

#ifdef NIM_AS_DEBUG
    //	for(i=0; i<1024; i++)
    //		fprintf(fp_debug, "%d\n", FFT_wave_1024[i]);
    //		fprintf(fp_debug, "%d\n", FFT_energy_1024[i]);
#endif

    // find how many channels in current bandwidth
    zero_num = 0;
    find_zero = 0;
    zero_counter = 0;
    for(i = 0; i < 1024; i++)
    {
        if(fft_wave_1024[i] == 0)
        {
            if(find_zero == 0)
            {
                find_zero = 1;
                zero_counter = 1;
            }
            else
            {
	        zero_counter++;
            } 
        }
        if((fft_wave_1024[i] != 0 || i == FFT_POINT_1023) && find_zero == 1)
        {
            params->zero_position[zero_num] = i - zero_counter / 2 - 1;
            zero_counter = 0;
            find_zero = 0;
            zero_num++;
        }
    }
    // clear short time impule
    if(zero_num == 0)
    {
        zero_num = 1;
    } 

	
    for(i = 0; i < (zero_num - 1); i++)
    {
        first_zero_pos = params->zero_position[i];
        second_zero_pos = params->zero_position[i + 1];
        first_signal_pos = 0;
        signal_counter = 0;
        for(j = first_zero_pos; j <= second_zero_pos; j++)
        {
            if(fft_wave_1024[j] != 0)
            {
	        signal_counter++;
            }  
            if(signal_counter == 1 && first_signal_pos == 0)
            {
	        first_signal_pos = j;
            }  
        }
        second_signal_pos = first_signal_pos + signal_counter - 1;
        if(signal_counter <= SIGNAL_CNT_8)
        {
            signal_value = fft_energy_1024[first_signal_pos - 1]	+ fft_energy_1024[second_signal_pos + 1];
            signal_value = signal_value / 2;
            for(j = first_signal_pos; j <= second_signal_pos; j++)
            {
                fft_wave_1024[j] = 0;
                fft_energy_1024[j] = signal_value;
            }
        }
    }
    // clear short time hollow
    for(i = 1; i < (zero_num - 1); i++)
    {
        if(fft_wave_1024[params->zero_position[i] + 2] != 0 || fft_wave_1024[params->zero_position[i] - 2] != 0)
        {
            for(j = -2; j <= 2; j++)
            {
	        fft_wave_1024[params->zero_position[i] + j] = 255;
            } 
            params->zero_position[i] = 0;
        }
    }
    temp = 0;
    for(i = 0; i < zero_num; i++)
    {
        if(params->zero_position[i] == 0)
        {
            for(j = i + 1; j < zero_num; j++)
            {
                params->zero_position[j - 1] = params->zero_position[j];
            }
            temp++;
        }
    }
    zero_num = zero_num - temp;
#ifdef NIM_AS_DEBUG
    for(i = 0; i < 1024; i++)
    {
        s3501_debug(fp_debug, "%d\n", fft_wave_1024[i]);
    }  
    //		s3501_DEBUG(fp_debug, "%d\n", FFT_energy_1024[i]);
#endif

    // find how many channels in current bandwidth
    zero_num = 0;
    find_zero = 0;
    zero_counter = 0;
    for(i = 0; i < 1024; i++)
    {
        if(fft_wave_1024[i] == 0)
        {
            if(find_zero == 0)
            {
                find_zero = 1;
                zero_counter = 1;
            }
            else
            {
	        zero_counter++;
            } 
        }
        if((fft_wave_1024[i] != 0 || i == FFT_POINT_1023) && find_zero == 1)
        {
            params->zero_position[zero_num] = i - zero_counter / 2 - 1;
            zero_counter = 0;
            find_zero = 0;
            zero_num++;
        }
    }
    return SUCCESS;

}

static INT32 get_threshold_value(NIM_S3501_FIND_CHANNEL_PARAMS *params,int i)
{
	UINT32 signal_counter = 0;
    UINT32 index = 0;
    INT32  signal_value = 0;
    UINT32 threshold =0;

	//estimate threshold
#if(0)
    for(index = params->zero_position[i]; index <= params->zero_position[i + 1]; index++)
    {
        if(fft_energy_1024[index] > FFT_VAL_800)
        {
            threshold += fft_energy_1024[index];
            signal_counter++;
        }
    }
    if(signal_counter != 0)
    {
    threshold = threshold / signal_counter;
    }  
    threshold = (threshold + params->noiselevel) / 2;

    if(threshold > THRESHOLD_4000)
    {
    threshold = 4000;
    } 
#else
    signal_value = 0;
    for(index = params->zero_position[i]; index <= params->zero_position[i + 1]; index++)
    {
        if(fft_energy_1024[index] > signal_value)
        {
        signal_value = fft_energy_1024[index];
        } 
    }
    signal_value >>= 1;
    for(index = params->zero_position[i]; index <= params->zero_position[i + 1]; index++)
    {
        if(fft_energy_1024[index] > signal_value)
        {
            threshold += fft_energy_1024[index];
            signal_counter++;
        }
    }
    if(signal_counter != 0)
    {
    threshold = threshold / signal_counter;
    }  
    threshold >>= 1;
    if(threshold > SIGNAL_LEVEL_TRH * 4)
    {
       threshold = SIGNAL_LEVEL_TRH * 4;
    } 
#endif

   return threshold;
}

static UINT32 nim_s3501_find_channel(UINT32 signallevel, UINT32 noiselevel, UINT32 start, UINT32 end, 
                                     UINT32 *chl_num, UINT32 *rise_edge, UINT32 *fall_edge)
{
	NIM_S3501_FIND_CHANNEL_PARAMS params;
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 index = 0;
    UINT32 judge_state = 0;
    UINT32 noise_point = 0;
    UINT32 signal_point = 0;
    UINT32 rise_edge_tmp = 0;
    UINT32 fall_edge_buf[MAX_CH_NUMBER]={0};
    UINT32 rise_edge_buf[MAX_CH_NUMBER]={0};
    UINT32 first_fall_edge = 0;
    UINT32 est_chl_num = 0;
    UINT32 threshold = 0;
    UINT32 last_rise_edge = 0;
    
    UINT32 zero_position[MAX_CH_NUMBER]={0};
    UINT32 edge_buf[64][2]={{0}};
    UINT32 channel_num = 0;
    UINT32 channel_start = 0;
    UINT32 channel_end = 0;
    
    INT32 temp_int = 0;
    UINT32 zero_num =0;
	UINT32 temp = 0;
	INT32 redge = 0;
	INT32 fedge = 0;

    comm_memset(&params,0,sizeof(NIM_S3501_FIND_CHANNEL_PARAMS));	
    params.signallevel = signallevel;
    params.noiselevel = noiselevel;
    params.start = start;
    params.end = end;
    params.zero_position = zero_position;

    zero_num=nim_s3501_find_channel_x(&params);

    // rough estimate channels' rise and fall edge
    channel_num = 0;
    for(i = 0; i < (zero_num - 1); i++)
    {

        threshold=get_threshold_value(&params,i);

        //varies intitialize
        judge_state = 0; //judge_state:0,searching noise;1,searching signal
        noise_point = 0;
        signal_point = 0;
        est_chl_num = 0;
        //		channel_start = zero_position[i] - (zero_position[i+1] -zero_position[i])/16;
        //		channel_end = zero_position[i+1] + (zero_position[i+1] -zero_position[i])/16;
        temp_int = zero_position[i] - (zero_position[i + 1] - zero_position[i]) / 16;
        if(temp_int < TEMP_4)
        {
	    temp_int = 4;
        } 
        channel_start = (UINT32)temp_int;
        temp_int = zero_position[i + 1] + (zero_position[i + 1] - zero_position[i]) / 16;
        if(temp_int > TEMP_1020)
        {
	    temp_int = 1020;
        }
        channel_end = (UINT32)temp_int;
        rise_edge_buf[est_chl_num] = channel_end;
        fall_edge_buf[est_chl_num] = channel_start;
        rise_edge_tmp = channel_end;

        //find channels
        for(index = channel_start; index <= channel_end; index++)
        {
            if(judge_state == 0)
            {
                if((UINT32)fft_energy_1024[index] < threshold)
                {
                    noise_point++;
                }
                else
                {
                    noise_point = 0;
                }
                if(noise_point > MIN_NOISE_LENGTH)
                {
                    judge_state = 1;
                    noise_point = 0;
                    fall_edge_buf[est_chl_num] = index - MIN_NOISE_LENGTH;
                    if (fall_edge_buf[est_chl_num] > rise_edge_buf[est_chl_num])
                    {
                        est_chl_num++;
                    }
                    else
                    {
                        first_fall_edge = fall_edge_buf[est_chl_num];
                    }
                    rise_edge_tmp = zero_position[i + 1];
                }
            }
            else
            {
                if((UINT32)fft_energy_1024[index] >= threshold)
                {
                    signal_point++;
                }
                else
                {
                    signal_point = 0;
                }
                if(signal_point > MIN_SIGNAL_LENGTH)
                {
                    judge_state = 0;
                    signal_point = 0;
                    rise_edge_tmp = index - MIN_SIGNAL_LENGTH;
                    if (est_chl_num == MAX_CH_NUMBER)
                    {
                        break;//when rise edge of channel which cant be stored found, stop search
                    }
                    else
                    {
                        rise_edge_buf[est_chl_num] = rise_edge_tmp;
                    }
                }
            }
        }
        // store channel's rise and fall edge
        for(j = 0; j < est_chl_num; j++)
        {
            edge_buf[channel_num][0] = rise_edge_buf[j];
            edge_buf[channel_num][1] = fall_edge_buf[j];
            channel_num++;
        }
    }

   //estimate last rise edge for frequency jump of next step
    last_rise_edge = end - 69;
	
    for(i = 0; i < channel_num; i++)
    {
        temp = edge_buf[i][1] - edge_buf[i][0];
        temp = temp * 7 / 40;
        redge = edge_buf[i][0] - temp;
        if(redge < 0)
        {
	    redge = 0;
        }
        fedge = edge_buf[i][1] + temp;
        if(fedge > FALL_EDGE_1023)
        {
	    fedge = 1023;
        }   

        if(edge_buf[i][0] >= start && edge_buf[i][1] <= end)
        {
            *(rise_edge + j) = edge_buf[i][0];
            *(fall_edge + j) = edge_buf[i][1];

            last_rise_edge = fedge;		//???
            j++;
        }
        if(edge_buf[i][0] >= (start + 23) && edge_buf[i][0] <= end && edge_buf[i][1] > end)
        {
            last_rise_edge = redge - 23;		//???
            if(last_rise_edge < (start + 12))
            {
	        last_rise_edge = start + 12;			// 1M offset
            }
        }

    }

    *chl_num = j;

    return last_rise_edge;
}




static void  nim_s3501_autosearch_x(NIM_S3501_AUTOSEARCH_PARAMS *params)
{
	UINT32 index =0;
    INT32 energy_step = 0;
	UINT32 noise_num = 0;
	UINT32 signal_num = 0;
	INT32 i = 0;
	
	//frequency smooth
	nim_s3501_smoothfilter_new();
	
	//remove DC component parts
	energy_step = fft_energy_1024[511+4] - fft_energy_1024[511-4];
	energy_step >>=3;
	for(index=(511-3); index<=(511+3); index++)
    { 
		fft_energy_1024[index] = fft_energy_1024[index-1] + energy_step;
    }
#ifdef NIM_AS_DEBUG
	//	for(i=0; i<1024; i++)
	//		s3501_DEBUG(fp_debug, "%d\n", FFT_energy_1024[i]);
#endif
	
	//get search window according to setting from analog filter bandwidth
	if (FREQ_SEARCH_START_INDEX<START_INDEX_5)
	{
		params->start_index=5;
	}
	else
	{
		params->start_index=FREQ_SEARCH_START_INDEX;
	}
	params->end_index=1022-params->start_index;//DC component is at index 511
	
	//get nosie level and signal level
	params->noise_level = 0;
	params->signal_level = 0;

	for (index=params->start_index;index<=params->end_index;index++)
	{
		if(fft_energy_1024[index]<NOISE_LEVEL_TRH)
		{
			noise_num++;
			params->noise_level += fft_energy_1024[index];
		}
		else if(fft_energy_1024[index]>SIGNAL_LEVEL_TRH)
		{
			signal_num++;
			params->signal_level += fft_energy_1024[index];
		}
	}
	
	if(noise_num >NOISE_NUM_16)
        { 
		params->noise_level = params->noise_level/noise_num;
        }
	else
        {
		params->noise_level = NOISE_LEVEL_TRH + 200;
        }
	if(signal_num >NOISE_NUM_16)
	{
		params->signal_level = params->signal_level/signal_num;
        } 
	else
	{
		params->signal_level = SIGNAL_LEVEL_TRH;
        } 
	
	if(signal_num <NOISE_NUM_16 || params->noise_level > NOISE_LEVEL_750)
       {
		params->found_channel = 0;
       }   
	else if(params->signal_level < SNR_TRH*params->noise_level)
       {
		params->found_channel = 0;
        } 
	else
	{
		params->found_channel = 1;
        }
	
	if(noise_num<1 && params->noise_level>=NOISE_LEVEL_700 && params->signal_level >=SIGNAL_LEVEL_5500)
	{
		//nim_s3501_smooth_restore();
		//nim_s3501_smoothfilter_old();
	}
	
#ifdef NIM_AS_DEBUG
	s3501_debug("found_channel=%d, noise_level=%d, signal_level=%d\n", params->found_channel, 
	                        params->noise_level, params->signal_level);
#endif

	params->ch_number = 0;

	if(params->found_channel)
        { 
		params->rise_edge_tmp = nim_s3501_find_channel(params->signal_level, params->noise_level,
		          params->start_index, params->end_index, (UINT32*)&params->ch_number, params->rise_edge, params->fall_edge);
        } 
	else
	{
		params->rise_edge_tmp = params->end_index;
        }
	if(params->ch_number > 0)
       {
		params->found_channel = 1;
        }  
	else
       {  
		params->found_channel = 0;
       }
#ifdef NIM_AS_DEBUG
	s3501_debug("rough find channel number=%d\n", params->ch_number);
	for(i=0; i<params->ch_number; i++)
    {
		s3501_debug("channel %d: rise_edge=%d, fall_edge=%d\n", i, params->rise_edge[i], params->fall_edge[i]);
	}	
#endif

	
}



INT32 nim_s3501_autosearch(INT32 *success, INT32 *delta_fc_est, INT32 *symbol_rate_est, 
                            INT32 * m_if_freq,
                            INT32* m_ch_number)
{
	NIM_S3501_AUTOSEARCH_PARAMS params;
	INT32 i = 0;
	INT32 j = 0;
	INT32 if_freq = 0;
	
	UINT32 fall_edge[MAX_CH_NUMBER] ={0};
	UINT32 rise_edge[MAX_CH_NUMBER] ={0};

	INT32  last_ch_fall2zero = 0;
	INT32  last_ch_rise = 0;
	INT32  shift_frequency = 0;
	INT32 signal_value = 0;
	
	INT32 rise_step = 0;
	INT32 fall_step = 0;
	INT32 step_num = 0;
	
#ifdef NIM_AS_DEBUG
	UINT32 est_delta_fc[MAX_CH_NUMBER] ={0};
	UINT32 est_symbol_rate[MAX_CH_NUMBER] ={0};

#endif

   memset(&params,0,sizeof(NIM_S3501_AUTOSEARCH_PARAMS));
   params.fall_edge=fall_edge;
   params.rise_edge=rise_edge;

   nim_s3501_autosearch_x(&params);
	
	//caculate channels symbol rate and carrier offset
	if(params.found_channel)
	{
		for(i=0; i<params.ch_number; i++)
		{
			//estimate rise lines
			step_num = fall_edge[i] - rise_edge[i] + 1;
			step_num = (step_num*7 + 20)/40;
			if(step_num<STEP_NUM_4)
            { 
				step_num = 4;
            }
			rise_step = 0;
			for(j=0; j<step_num; j++)
			{
				rise_step +=fft_energy_1024[rise_edge[i] + step_num -j] - fft_energy_1024[rise_edge[i] -j];
			}
			rise_step =rise_step/(step_num*step_num) ;
			
			//estimat fall lines
			fall_step = 0;
			for(j=0; j<step_num; j++)
			{
				fall_step +=fft_energy_1024[fall_edge[i] - step_num +j] - fft_energy_1024[fall_edge[i] +j];
			}
			fall_step = fall_step/(step_num*step_num);
			
			if(rise_step>0)		//???
			{
				signal_value = fft_energy_1024[rise_edge[i]];
				do
				{
					signal_value -=rise_step;
					rise_edge[i]--;
					if(rise_edge[i]<=params.start_index)
                    {   
						break;
                    }
				}
				while(((UINT32)signal_value >params.noise_level)&&(signal_value >0));
			}
			if(fall_step>0)		//???
			{
				signal_value = fft_energy_1024[fall_edge[i]];
				do
				{
					signal_value -=fall_step;
					fall_edge[i]++;
					if(fall_edge[i]>=params.end_index)
                    {
						break;
                    }
                }  
                while(((UINT32)signal_value >params.noise_level)&&(signal_value >0));
			}
#ifdef NIM_AS_DEBUG			
			est_delta_fc[i]=((fall_edge[i]+rise_edge[i])>>1);
			est_symbol_rate[i] = fall_edge[i] - rise_edge[i];
			est_symbol_rate[i] *=20;
			est_symbol_rate[i] = (est_symbol_rate[i] + 14)/27;
#endif
			*(delta_fc_est+i)=(fall_edge[i] + rise_edge[i])/2 - 511;
			*(delta_fc_est+i)= *(delta_fc_est+i)*90;
			
			if (*(delta_fc_est+i)>(ANALOG_FILTER_BW*1024 - 1024))
			{
				*(delta_fc_est+i)=(ANALOG_FILTER_BW*1024 - 1024);
			}
			else if (*(delta_fc_est+i)<-(ANALOG_FILTER_BW*1024 - 1024)) 
			{
				*(delta_fc_est+i)=-(ANALOG_FILTER_BW*1024 - 1024);
			}
			
			*(symbol_rate_est+i)=((fall_edge[i]-rise_edge[i])*900*2 + 14)/27;
			if (*(symbol_rate_est+i)>45*1024)
			{
				*(symbol_rate_est+i)=45*1024;
			}
		}
	}
#ifdef NIM_AS_DEBUG
	for(i=0; i<params.ch_number; i++)
	{
		s3501_debug("channel %d rise_edge=%d, fall_edge=%d\n", i, rise_edge[i], fall_edge[i]);
		s3501_debug("channle %d estimatied symbol rate: %d, %d\n", i, delta_fc_est[i], symbol_rate_est[i]);
	}
#endif
	//estimate turer shift frequency step
	shift_frequency = ANALOG_FILTER_BW*1024 - (params.end_index - params.rise_edge_tmp)*90;
	if(params.found_channel)
	{
		last_ch_fall2zero = *(delta_fc_est+params.ch_number-1)+((*(symbol_rate_est+params.ch_number-1)*27)/40);
		
		if(last_ch_fall2zero>(64*1024-1))
		{
			last_ch_fall2zero=64*1024-1;
		}
		if(last_ch_fall2zero>shift_frequency)
		{
			shift_frequency=last_ch_fall2zero;
		}
		if(last_ch_fall2zero > (INT32)((params.end_index - 511)*90 - 2*1024))
		{
			last_ch_rise = *(delta_fc_est+params.ch_number-1)-((*(symbol_rate_est+params.ch_number-1)*27)/40);
			
			if(last_ch_rise > (INT32)((params.start_index - 511)*90))
			{
				shift_frequency = last_ch_rise;
				shift_frequency = shift_frequency - 2*1024;  //2M protection bandwidth			
			}
			params.ch_number--;
		}				
	}
#ifdef NIM_AS_DEBUG				
	s3501_debug("shift_frequency=%d\n", shift_frequency);
#endif
	if_freq=ANALOG_FILTER_BW*1024 + shift_frequency;
	if(if_freq <= 2*1024)
    {
		if_freq = 2*1024;
    } 
	else if(if_freq > 2*ANALOG_FILTER_BW*1024)
    {
		if_freq = 2*ANALOG_FILTER_BW*1024;
	}
	if(params.ch_number==0)
    {
		*success = 0;
    } 
	else
    {
		*success = 1;
	}
	//exclude those channels whose symbol rate are not in search range designated
	if(*success)
	{
		i=0;
		while(i<params.ch_number)
		{
			if(*(symbol_rate_est+i)>(45*1024))
            {  
				*(symbol_rate_est+i) = 45*1024;
            } 
			if(*(symbol_rate_est+i)<FFT_POINT_1024)
			{
				params.ch_number--;
				for (j=i;j<params.ch_number;j++)
				{
					*(symbol_rate_est+j) = *(symbol_rate_est+j+1);
					*(delta_fc_est+j) = *(delta_fc_est+j+1);
				}
			}
			else
			{
				i++;
			}	
		}
	}
	if(params.ch_number==0)
	{
		*success = 0;
	}	
	
	*m_if_freq = if_freq;
	*m_ch_number = params.ch_number;
	
	return 0;
}



INT32 nim_s3501_repeat_detect(INT32 *addrstart, INT32 *addrend, INT32 tp_count, INT32 left_addr, 
                                     INT32 right_addr)
{
    INT32 i = 0;

    for(i = tp_count; i >= 0; i--)
    {
        if((addrstart[i] == left_addr) && (addrend[i] == right_addr))
        {
            return 0;
        }
    }
    return 1;
}



static void nim_s3501_fs_insert(INT32 *tp_flag,INT32 *tp_frequency,INT32 *tp_symbolrate)
{
	INT32 k = 0;
    INT32 j = 0;
    INT32 est_frequency = 0;
    INT32 est_symbolrate = 0;
    INT32 compare0 = 0;
    INT32 compare1 = 0;
	UINT8 addflag = 0;
	INT32 insert_index = 0;
	
    for(k = 0; k < 5; k++)
    {
        if(tp_flag[k] == 1)
        {
            est_symbolrate = tp_symbolrate[k];
            est_frequency = tp_frequency[k];

            compare1 = 1024;
            if(est_symbolrate > EST_SYM_25000)
            {
                compare0 = 1024 * 2;
            }
            else if(est_symbolrate > 7000)
            {
                compare0 = 1024;
            }
            else if(est_symbolrate > 5000)
            {
                compare0 = 512;
            }
            else
            {
                compare0 = 256;
            }

            if(((est_symbolrate / 1024) <= TEMP_70) && ((est_symbolrate / 1024) >= 1))
            {
                addflag = 1;
                insert_index = tp_number;
                for(j = tp_number - 1; j >= 0; j--)
                {
                    if(est_frequency - frequency_est[j] > compare1)
                    {
	                    break;
                    }
                    if((abs(est_frequency - frequency_est[j]) <= compare1) && 
	                     (abs(est_symbolrate - symbol_rate_est[j]) < compare0))
                    {
                        addflag = 0;
                        break;
                    }
                    if(frequency_est[j] > est_frequency)
                    {
	                    insert_index = j;
                    }
                    else if(frequency_est[j] == est_frequency)
                    {
                        if(symbol_rate_est[j] > est_symbolrate)
                        {
		                    insert_index = j;
                        }
                    }
                }

                if(addflag == 1)
                {
                    if(insert_index < tp_number)
                    {
                        for(j = tp_number; j >= insert_index; j--)
                        {
                            symbol_rate_est[j] = symbol_rate_est[j - 1];
                            frequency_est[j] = frequency_est[j - 1];
                        }
                        symbol_rate_est[insert_index] = est_symbolrate;
                        frequency_est[insert_index] = est_frequency;
                    }
                    else
                    {
                        symbol_rate_est[tp_number] = est_symbolrate;
                        frequency_est[tp_number] = est_frequency;
                    }
                    tp_number = tp_number + 1;
                    AUTOSEARCH_PRINTF("Est frequency and SymbolRate --> [%d,%d]  \n", 
	                       est_frequency / 1024, est_symbolrate);
                }
            }
        }
    }


	
}

static void nim_s3501_fs_get_tp_flag(INT32 delta0,INT32 delta1,INT32 *tp_flag)
{

    if((delta0 < DEL_FREQ_3) && (delta1 < DEL_FREQ_3))
    {
        tp_flag[3] = 1;
    }
    else if((delta0 < 6) && (delta1 < 6))
    {
        tp_flag[2] = 1;
    }  
    else if((delta0 >= 20) && (delta1 >= 20)) ///modify to >= for China6B 3971/H/10000
    {
        tp_flag[2] = 1;
        tp_flag[3] = 1;
    }
    else if((delta0 >= 20) || (delta1 >= 20)) ///modify to >= for China6B 3971/H/10000
    {
        tp_flag[1] = 1;
        tp_flag[2] = 1;
        tp_flag[3] = 1;
    }
    else if((delta0 >= ((3 * delta1 + 1) / 2)) || (((3 * delta0 + 1) / 2) <= delta1))
    {
        tp_flag[2] = 1;
        tp_flag[3] = 1;
    }
    else
    {
       tp_flag[2] = 1;
    } 
	
}

typedef struct _s3501_fs_params_t
{
	
    INT32 mix_tp_flag;
    INT32 min_tp_width;
    INT32 tp_num;
	UINT8 *wave;
	INT32 wave_len;
	INT32 *channel_spectrum;
	INT32 *addrstart;
	INT32 *addrend;
	INT32 *tp_riseedge;
	INT32 *tp_falledge;
	
}NIM_S3501_FS_PARAMS;


static INT32 nim_s3501_fs_estimate_x(UINT8 *tp_threshold_flag,INT32 i,NIM_S3501_FS_PARAMS *params)
{
    INT32 k = 0;
    INT32 j = 0;
	INT32 n = 0;
    INT32 counter0 = 0;
    INT32 counter1 = 0;
    INT32 counter = 0;
	INT32 last_counter = 0;
    INT32 subsum = 0;
    INT32 subcnt = 0;
	
    INT32 submaxvalue = 0;
	INT32 submaxvalueaddr = 0;
    INT32 submaxvalue_min = 0;
    INT32 submaxvalueaddr_min = 0;
	INT32 ret=RET_FOR_CONTINUE;

    INT32 tp_threshold[TP_EST_MAX_CNT]={0};

	comm_memset(tp_threshold, 0, sizeof(tp_threshold));

    //calculate compare_threshold
    submaxvalue = 0;
    submaxvalueaddr = 0;
    for(j = params->addrstart[i]; j <= params->addrend[i]; j++)
    {
        if(params->channel_spectrum[j] > submaxvalue)
        {
            submaxvalue = params->channel_spectrum[j];
            submaxvalueaddr = j;
        }
    }
    submaxvalue_min = submaxvalue;
    submaxvalueaddr_min = submaxvalueaddr;

     /////////////modify to >= for China6B 4116/H/21374
    if((i > 0) && ((params->addrstart[i] <= params->addrstart[i - 1]) && 
                   (params->addrend[i] >= params->addrend[i - 1]))) 
    {
        params->mix_tp_flag = 1;
        AUTOSEARCH_PRINTF("[%d : %d] : [%d : %d] \n", params->addrstart[i], params->addrend[i], 
	                   params->addrstart[i - 1], params->addrend[i - 1]);
        for(j = 0; j < params->tp_num; j++)
        {
            if((params->addrstart[i] <= params->addrstart[j]) && (params->addrend[i] >= params->addrend[j]))
            {
                submaxvalue = 0;
                submaxvalueaddr = 0;
                for(n = params->addrstart[j]; n <= params->addrend[j]; n++)
                {
                    if(params->channel_spectrum[n] > submaxvalue)
                    {
                        submaxvalue = params->channel_spectrum[n];
                        submaxvalueaddr = n;
                    }
                }
                if(submaxvalue < submaxvalue_min)
                {
                    submaxvalue_min = submaxvalue;
                    submaxvalueaddr_min = submaxvalueaddr;
                }
            }
        }
    }

	submaxvalue = submaxvalue_min;
    submaxvalueaddr = submaxvalueaddr_min;
    AUTOSEARCH_PRINTF("submaxvalue=%d, submaxvalueaddr=%d\n", submaxvalue, submaxvalueaddr);

    *tp_threshold_flag = 0;
    for(j = params->addrstart[i]; j <= params->addrend[i]; j++)
    {
        if(params->channel_spectrum[j] >= (submaxvalue * 3 + 2) / 4)
        {
            subsum = subsum + params->channel_spectrum[j];
            subcnt = subcnt + 1;
            if(subsum >= MAX_VAL_27BIT) //27bit=0xEFFFFFF
            {
                *tp_threshold_flag = 1;
                break;
            }
        }
    }

    AUTOSEARCH_PRINTF("tp_threshold_flag=%d\n", *tp_threshold_flag);

    if(*tp_threshold_flag == 1)
    {
        subsum = submaxvalue;
        subcnt = 1;
        tp_threshold[0] = (subsum * 4 + (5 * subcnt) / 2) / (5 * subcnt);
        tp_threshold[4] = (subsum * 4 + (5 * subcnt) / 2) / (5 * subcnt);

        tp_threshold[1] = (subsum * 3 + (5 * subcnt) / 2) / (5 * subcnt);
        tp_threshold[2] = (subsum + (2 * subcnt) / 2) / (2 * subcnt);
        tp_threshold[3] = (subsum + (3 * subcnt) / 2) / (3 * subcnt);
        AUTOSEARCH_PRINTF("Thredshold_value --> %d : %d : %d : %d : %d \n", tp_threshold[0], 
                      tp_threshold[4], tp_threshold[1], tp_threshold[2], tp_threshold[3]);
    }
    else
    {
        if(0 == subcnt)
        {
            AUTOSEARCH_PRINTF("ERROR: subcnt=0 \n");
            return ret;
        }
        tp_threshold[0] = (submaxvalue * 8) / 10;
        tp_threshold[4] = (submaxvalue * 17) / 20;


        tp_threshold[1] = (subsum * 3 + (5 * subcnt) / 2) / (5 * subcnt);
        tp_threshold[2] = (subsum + (2 * subcnt) / 2) / (2 * subcnt);
        tp_threshold[3] = (subsum + (3 * subcnt) / 2) / (3 * subcnt);
        AUTOSEARCH_PRINTF("subsum=%d, subcnt=%d \n", subsum, subcnt);
        AUTOSEARCH_PRINTF("Thredshold_value --> %d : %d : %d : %d : %d \n", tp_threshold[0], 
                        tp_threshold[4], tp_threshold[1], tp_threshold[2], tp_threshold[3]);
    }
	
    for(j = 0; j < 5; j++)
    {
        counter = params->addrstart[i];
        counter0 = 0;
        counter1 = 0;
        for(k = 0; k <= params->wave_len; k++)
        {
            if (params->channel_spectrum[counter] >= tp_threshold[j])
            {
                params->wave[k] = 1;
                counter1 ++;
                if ((counter0 > 0) && (counter0 < params->min_tp_width))
                {
                    //	printf("find short hollow, cleared!\n");
                    for(; counter0 > 0; counter0--)
                    {
		                params->wave[k - counter0] = 1;
                    }
                    counter1 = 1;
                }
                counter0 = 0;
            }
            else
            {
                params->wave[k] = 0;
                if(counter1 > 0)
                {
	               counter0++;
                }
            }
            counter ++;
        }

        counter = 0;
        last_counter = 0;
        for(k = 0; k < params->wave_len; k++)
        {
            if(params->wave[k] > 0)
            {
                counter++;
                if (counter == 1)
                {
                    params->tp_riseedge[j] = k + params->addrstart[i];
                    if (last_counter == 0)
                    {
		               params->tp_falledge[j] = params->tp_riseedge[j];
                    }
                }
            }
            else
            {
                if(counter > params->min_tp_width)
                {
                    if(counter > last_counter)
                    {
                        params->tp_falledge[j] = params->tp_riseedge[j] + counter;
                        last_counter = counter;
                    }
                }
                counter = 0;
            }
        }
        if (last_counter == 0)
        {
            params->tp_falledge[j] = params->tp_riseedge[j] + counter;
        }
        else
        {
            params->tp_riseedge[j] = params->tp_falledge[j] - last_counter;
        }

        AUTOSEARCH_PRINTF("Est [%d] riseedge and falledge is [%d,%d]\n", j, params->tp_riseedge[j], 
	        params->tp_falledge[j]);
    }

   return SUCCESS;


}

INT32 nim_s3501_fs_estimate(INT32 *addrstart,
                            INT32 *addrend,
                            INT32 tp_num,
                            INT32 *channel_spectrum,
                            UINT32 sfreq,
                            UINT32 adc_sample_freq)
{
	NIM_S3501_FS_PARAMS params;
	UINT8 tp_threshold_flag=0;
	
    INT32 i = 0;
    INT32 j = 0;

    INT32 tp_riseedge[TP_EST_MAX_CNT]={0};
    INT32 tp_falledge[TP_EST_MAX_CNT]={0};
    INT32 tp_flag[TP_EST_MAX_CNT]={0};
    INT32 tp_symbolrate[TP_EST_MAX_CNT]={0};
    INT32 tp_frequency[TP_EST_MAX_CNT]={0};
    UINT8 tp_valid = 0;
    
    INT32 min_tp_width = 13;

    INT32 delta0 = 0;
    INT32 delta1 = 0;
    INT32 delta11 = 0;
    INT32 mix_tp_flag = 0;

    UINT8 *wave = NULL;
    INT32 wave_len = 0;
    INT32 ret =0;
		
    wave = (UINT8 *)comm_malloc(MAX_TP_WIDTH * sizeof(UINT8));
    if (NULL == wave)
    {
        AUTOSEARCH_PRINTF("wave[] malloc failed!\n");
        return 1;
    }
    comm_memset(wave, 0, MAX_TP_WIDTH * sizeof(UINT8));

    comm_memset(tp_riseedge, 0, sizeof(tp_riseedge));
    comm_memset(tp_falledge, 0, sizeof(tp_falledge));
    
    comm_memset(tp_flag, 0, sizeof(tp_flag));
    comm_memset(tp_symbolrate, 0, sizeof(tp_symbolrate));
    comm_memset(tp_frequency, 0, sizeof(tp_frequency));

    min_tp_width = 1024 / adc_sample_freq;


    for(i = 0; i < tp_num; i++) //TP_number is TP-found number in function "nim_s3501_find_TP"
    {
        AUTOSEARCH_PRINTF("\nBegin Est %d-th TP[%d , %d]--> \n", i, addrstart[i], addrend[i]);

        mix_tp_flag = 0;

        if(addrstart[i] > addrend[i])
        {
            AUTOSEARCH_PRINTF("ERROR: addrstart>addrend\n");
            continue;
        }

        if (tp_number > TP_MAXNUM - 3)
        {
            AUTOSEARCH_PRINTF("FATAL ERROR: TPcnt overflow! TPcnt= %d \n", tp_number);
            break;
        }
        wave_len = addrend[i] - addrstart[i];
        if (wave_len >= MAX_TP_WIDTH) //MAX_TP_WIDTH=1000
        {
            AUTOSEARCH_PRINTF("Warning: large wave [%d %d]!\n", addrstart[i], addrend[i]);
            continue;
        }

	    params.mix_tp_flag =mix_tp_flag;
        params.min_tp_width=min_tp_width;
        params.tp_num=tp_num;
	    params.wave=wave;
		params.wave_len=wave_len;
	    params.channel_spectrum=channel_spectrum;
	    params.addrstart=addrstart;
	    params.addrend=addrend;
	    params.tp_riseedge=tp_riseedge;
	    params.tp_falledge=tp_falledge;
        ret=nim_s3501_fs_estimate_x(&tp_threshold_flag,i,&params);
        if(ret==RET_FOR_CONTINUE)
        {
			continue;
        }
		
        comm_memset(tp_symbolrate, 0, sizeof(tp_symbolrate));
        comm_memset(tp_frequency, 0, sizeof(tp_frequency));
        tp_valid = 1;
        for(j = 0; j < 5; j++)
        {
            if((j == 0) || (j == CNT_VAL_4))
            {
                if((tp_falledge[j] - tp_riseedge[j]) > (min_tp_width - 3))
                {
                    tp_symbolrate[j] = (tp_falledge[j] - tp_riseedge[j]) * adc_sample_freq * 125 / 128;
                    tp_symbolrate[j] = tp_symbolrate[j] * 26 / 20;
                    tp_frequency[j] = ((tp_falledge[j] + tp_riseedge[j]) * adc_sample_freq / 2) + \
		                      (sfreq - (adc_sample_freq / 2)) * 1024;
                    AUTOSEARCH_PRINTF("Est [%d] tp_frequency and tp_symbolrate is [%d,%d]\n", j, 
		                       tp_frequency[j] / 1024, tp_symbolrate[j]);
                }
            }
            else
            {
                if(tp_falledge[j] == tp_riseedge[j])
                {
                    tp_valid = 0;
                }
                if((tp_falledge[j] - tp_riseedge[j]) > min_tp_width)
                {
                    tp_symbolrate[j] = (tp_falledge[j] - tp_riseedge[j]) * adc_sample_freq * 125 / 128;
                    tp_frequency[j] = ((tp_falledge[j] + tp_riseedge[j]) * adc_sample_freq / 2) + \
		                      (sfreq - (adc_sample_freq / 2)) * 1024;
                    AUTOSEARCH_PRINTF("Est [%d] tp_frequency and tp_symbolrate is [%d,%d]\n", j, 
		                          tp_frequency[j] / 1024, tp_symbolrate[j]);
                }
            }
        }

        AUTOSEARCH_PRINTF("tp_valid=%d \n", tp_valid);

        if(tp_valid == 1)
        {
            comm_memset(tp_flag, 0, sizeof(tp_flag));

            if(tp_threshold_flag == 1)
            {
                tp_flag[0] = 1;
                tp_flag[1] = 1;
                tp_flag[2] = 1;
                tp_flag[3] = 1;
            }
            else
            {
#if 1                 ///modify for Maxing3  3722/V/2169
                if(((tp_frequency[4] / 1024 >= 1427) && (tp_frequency[4] / 1024 <= 1430)) || 
		        ((tp_frequency[0] / 1024 >= 1427) && (tp_frequency[0] / 1024 <= 1430)))
                {
                    if(tp_symbolrate[4] > 0)
                    {
                        delta11 = tp_falledge[0] - tp_falledge[4];
                        if(delta11 > DELTA_VAL_10)
                        {
                            delta11 = tp_falledge[1] - tp_falledge[4];
                            if((delta11 > DELTA_VAL_10) && (mix_tp_flag == 0))
                            {
                                tp_flag[4] = 1;
                            }
                        }
                        else
                        {
                            delta11 = tp_falledge[1] - tp_falledge[0];
                            if((delta11 > DELTA_VAL_10) && (mix_tp_flag == 0))
                            {
                                tp_flag[0] = 1;
                            }
                        }
                    }
                    else if(tp_symbolrate[0] > 0)
                    {
                        delta11 = tp_falledge[1] - tp_falledge[0];
                        if((delta11 > DELTA_VAL_10) && (mix_tp_flag == 0))
                        {
                            tp_flag[0] = 1;
                        }
                    }
                }
#endif

                delta0 = tp_riseedge[1] - tp_riseedge[3];
                delta1 = tp_falledge[3] - tp_falledge[1];
                nim_s3501_fs_get_tp_flag(delta0,delta1,tp_flag);

            }

            nim_s3501_fs_insert(tp_flag,tp_frequency,tp_symbolrate);

        }
    }
    comm_free(wave);
    wave = NULL;

    return 0;
}
INT32 nim_s3501_fft_wideband_scan(UINT32 tuner_if, UINT32 adc_sample_freq)
{
    INT32 i = 0;
    INT32 j = 0;
    INT32 delta_fre = 0;
    UINT32 cover_num = 0;
    UINT32 uncover_num = 0;
    INT32 factor = 0;

    UINT32 sum1 = 0;
    UINT32 sum2 = 0;
    UINT32 tmp_addr = 0;
    INT32 energy_step = 0;
    INT32 factor_precision = 1000;
    INT32 max_int32 = 2147483647;
    INT32 central_addr0 = 0;
    INT32 central_addr1 = 0;
    UINT8 overflow = 0;

    for(i = 0; i < 1024; i++) /////FFT_energy_1024[i] //// 25bit
    {
        if(fft_energy_1024[i] == 0)
        {
	    fft_energy_1024[i] = 1;
        }
    }

    //remove DC component parts
    energy_step = fft_energy_1024[511 + 2] - fft_energy_1024[511 - 2];
    energy_step >>= 2;
    for(i = (511 - 2); i < (511 + 2); i++)
    {
        fft_energy_1024[i + 1] = fft_energy_1024[i] + energy_step;
    }

    if(chlspec_num == 0)
    {
        for(i = 0; i < 1024; i++)
        {
            channel_spectrum[chlspec_num] = fft_energy_1024[i];
            if(max_fft_energy < channel_spectrum[chlspec_num])
            {
	        max_fft_energy = channel_spectrum[chlspec_num];
            } 
            chlspec_num++;
        }
    }
    else
    {
        delta_fre = tuner_if - last_tuner_if;
        uncover_num = (delta_fre * 1024 + (adc_sample_freq / 2)) / adc_sample_freq; //un-cover number
        cover_num = 1024 - uncover_num; //cover number
        central_addr0 = cover_num / 2;
        central_addr1 = 1024 - uncover_num - central_addr0;
        WBSCAN_PRINTF("delta_fre=%d, uncover_num=%d, cover_num=%d \n", delta_fre, uncover_num, cover_num);

        if(delta_fre > 0)
        {
            sum1 = 0;
            sum2 = 1;
            factor = 0;
            for(i = central_addr0 - 20; i < central_addr0 + 20; i++)
            {
                tmp_addr = chlspec_num - cover_num + i;
                sum1 += channel_spectrum[tmp_addr];
                sum2 += fft_energy_1024[i];
            }

            while(sum1 > SUM_VAL_2000000)
            {
                sum1 = (sum1 + 1) / 2;
                sum2 = (sum2 + 1) / 2;
            }

            factor = (sum1 * factor_precision + sum2 / 2) / sum2;
            WBSCAN_PRINTF("sum1/sum2 = %d / %d ,factor = %d\n", sum1, sum2, factor);

            if(factor >= FACTOR_1000000)
            {
                factor_precision = 10;
                factor = (sum1 * factor_precision + sum2 / 2) / sum2;
            }
            else if(factor >= 100000)
            {
                factor_precision = 100;
                factor = (sum1 * factor_precision + sum2 / 2) / sum2;
            }
            else if(factor == 0)
            {
                factor_precision = 10000;
                factor = (sum1 * factor_precision + sum2 / 2) / sum2;
                WBSCAN_PRINTF("Error factor = 0, new factor = %d \n ", factor);
            }
         

            for(i = (chlspec_num - cover_num + central_addr0); i < chlspec_num; i++)
            {
                tmp_addr = i - (chlspec_num - cover_num);
                overflow = 0;
                while(max_int32 / fft_energy_1024[tmp_addr] < factor)
                {
                    if(factor_precision == 1)
                    {
                        overflow = 1;
                        WBSCAN_PRINTF("**********Warning! FFT_WBS path1 OverFlow!*************");
                        break;
                    }
                    factor_precision = factor_precision / 10;
                    factor = (sum1 * factor_precision + sum2 / 2) / sum2;
                }
                if(overflow == 1)
                {
		    channel_spectrum[i] = max_int32;
                }
                else
                {
		    channel_spectrum[i] = (fft_energy_1024[tmp_addr] * factor + factor_precision / 2) / factor_precision;
                } 

                if(channel_spectrum[i] == 0)
                {
		    channel_spectrum[i] = 1;
                }

                if(max_fft_energy < channel_spectrum[i])
                {
		    max_fft_energy = channel_spectrum[i];
                }
            }

            tmp_addr = i - (chlspec_num - cover_num);
            for(j = tmp_addr; j < 1024; j++)
            {
                overflow = 0;
                while(max_int32 / fft_energy_1024[j] < factor)
                {
                    if(factor_precision == 1)
                    {
                        overflow = 1;
                        WBSCAN_PRINTF("**********Warning! FFT_WBS path2 OverFlow!*************");
                        break;
                    }
                    factor_precision = factor_precision / 10;
                    factor = (sum1 * factor_precision + sum2 / 2) / sum2;
                }
                if(overflow == 1)
                {
		    channel_spectrum[chlspec_num] = max_int32;
                }
                else
                {
		    channel_spectrum[chlspec_num] = (fft_energy_1024[j] * factor + factor_precision / 2) / factor_precision;
                }

                if(channel_spectrum[chlspec_num] == 0)
                {
		    channel_spectrum[chlspec_num] = 1;
                }

                if(max_fft_energy < channel_spectrum[chlspec_num])
                {
		    max_fft_energy = channel_spectrum[chlspec_num];
                 }

                chlspec_num++;
            }
        }
    }
    last_tuner_if = tuner_if;

    return SUCCESS;
}



void nim_s3501_adjust_window_wide_x
(
	INT32 count0,
    INT32 count1,
	INT32 last_left,
    INT32 last_right,
	INT32 delta_cnt,
	INT32 i,
	INT32 *fdata
	)
{
	INT32 j = 0;
    UINT8 flag0 = 0;


    if(((count1 >= CNT_VAL_15) && (count1 <= CNT_VAL_750)) && ((count0 >= CNT_VAL_15) && (count0 <= CNT_VAL_750)))
    {
        if((delta_cnt <= DELTA_VAL_12) && (delta_cnt >= DELTA_VAL_3))
        {
            for(j = last_left; j <= last_right; j++)
            {
                fft_energy_1024_flag[j] = 1;
            }

            if(count0 <= CNT_VAL_40)
            {
                for(j = last_left - count0; j < last_left; j++)
                {
                    if(fft_energy_1024_flag[j] == FFT_FLAG_5)
                    {
                        fft_energy_1024_flag[j] = 3;
                    }
                }
            }

            if(count1 <= CNT_VAL_40)
            {
                for(j = last_right; j < last_right + count1; j++)
                {
                    if(fft_energy_1024_flag[j] == FFT_FLAG_5)
                    {
                        fft_energy_1024_flag[j] = 3;
                    }
                }
            }
        }
    }
    else if(((count1 >= 15) && (count1 <= 50)) && (count0 >= 3))
    {
        if((delta_cnt <= DELTA_VAL_12) && (delta_cnt >= DELTA_VAL_3))
        {
            flag0 = 0;
            nim_s3501_find_max_value(i, (last_right + count1), fdata, (INT32*)&flag0, 2, 0);
            if(flag0 == 1)
            {
                if(count1 <= CNT_VAL_40)
                {
                    for(j = last_right; j < last_right + count1; j++)
                    {
                        if(fft_energy_1024_flag[j] == FFT_FLAG_5)
                        {
                            fft_energy_1024_flag[j] = 3;
                        }
                    }
                }

                for(j = i; j <= last_right; j++)
                {
                    fft_energy_1024_flag[j] = 1;
                }

                for(j = (last_left - count0); j < i; j++)
                {
                    if(fft_energy_1024_flag[j] == FFT_FLAG_5)
                    {
                        fft_energy_1024_flag[j] = 1;
                    }
                }
            }
        }
    }
    else if(((count0 >= 15) && (count0 <= 50)) && (count1 >= 3))
    {
        if((delta_cnt <= DELTA_VAL_12) && (delta_cnt >= DELTA_VAL_3))
        {
            flag0 = 0;
            nim_s3501_find_max_value((last_left - count0), i, fdata,(INT32*)&flag0, 2, 2);
            if(flag0 == 1)
            {
                if(count0 <= CNT_VAL_40)
                {
                    for(j = last_left - count0; j < last_left; j++)
                    {
                        if(fft_energy_1024_flag[j] == FFT_FLAG_5)
                        {
                            fft_energy_1024_flag[j] = 3;
                        }
                    }
                }

                for(j = last_left; j <= i; j++)
                {
                    fft_energy_1024_flag[j] = 1;
                }

                for(j = i + 1; j < last_right + count1; j++)
                {
                    if(fft_energy_1024_flag[j] == FFT_FLAG_5)
                    {
                        fft_energy_1024_flag[j] = 1;
                    }
                }
            }
        }
    }

	
}
void nim_s3501_median_filter(INT32 flength, INT32 *fdata, INT32 scan_mode)
{
    INT32 i = 0;
    INT32 mf_window_len = 5;

    mf_window_len = 5;

    comm_memset(fft_energy_1024_value, 0, sizeof(fft_energy_1024_value));
    for(i = 0; i < flength; i++)
    {
        fft_energy_1024_value[i] = fdata[i];
    }
    for(i = 0 + 50; i < flength - 50; i++)
    {
        nim_s3501_reorder(i, fft_energy_1024_value, 1);
    }

    comm_memset(fft_energy_1024_flag, 5, sizeof(fft_energy_1024_flag));

    if(scan_mode == 1)
    {
        nim_s3501_adjust_window_wide(flength, fft_energy_1024_value, fdata);
    }

    for(i = 0 + 50; i < flength - 50; i++)
    {
        nim_s3501_reorder(i, fdata, fft_energy_1024_flag[i]);
    }
}
