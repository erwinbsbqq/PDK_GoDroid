/*****************************************************************************
*    Copyright (C)2003 Ali Corporation. All Rights Reserved.
*
*    File:     nim_m3327_autosearch.h
*
*    Description:    Header file for blind search.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	2005/06/10        Berg Xing       Ver 0.1       Create file.
*	2.  2005/06/16		  Berg Xing 	  Ver 0.2       Enlarge search bandwidth
*	3.  2005/06/27        Berg Xing       Ver 0.3       Code size optimize
*	4.  2005/06/28        Berg Xing       Ver 0.4       Remove warning information when compiling
*
*    Notes:
*****************************************************************************/
#ifndef __NIM_S3501_AUTOSEARCH_H__
#define __NIM_S3501_AUTOSEARCH_H__



#define NIM_AS_DEBUG		1

#if(NIM_S3501_AUTOSEARCH_DEBUG)
#define s3501_debug		PRINTF
#else
#define s3501_debug(...)
#endif


//#define AUTOSEARCH_DEBUG
#ifdef AUTOSEARCH_DEBUG
#define AUTOSEARCH_PRINTF nim_print
#else
#define AUTOSEARCH_PRINTF(...)
#endif

//#define WBSCAN_DEBUG
#ifdef WBSCAN_DEBUG
#define WBSCAN_PRINTF nim_print
#else
#define WBSCAN_PRINTF(...)
#endif






#define NOISE_LEVEL_TRH		500
#define SIGNAL_LEVEL_TRH	1000
#define SNR_TRH				2

#define TP_MAXNUM 					2000

#define MAX_BANDWIDTH 90
#define MIN_BANDWIDTH 1
#define FFT_LENGTH 1024
#define NEW_DOUBT
#define NEW_CASE
#define MAX_TP_WIDTH 1000

#define TP_EST_MAX_CNT  5
#define WINDOW_LEN 5
// notice:
// please also change the define in file nim_s3501.h.
#define		MAX_CH_NUMBER			32//maximum number of channels that can be stored


//#define		FREQ_SEARCH_START_INDEX		237
#define		FREQ_SEARCH_START_INDEX		170
//DC component corresponds to index 511
//frequency resolution is 90MHz/1024=0.08789MHz for asic version
//and 60MHz/1024=0.05859375MHz for FPGA version
//so FREQ_SEARCH_START_INDEX=max(round(511 - (analog filter bandwidth/0.08789)), 0)
//or FREQ_SEARCH_START_INDEX=max(round(511 - (analog filter bandwidth/0.05859375)), 0)
//analog filter bandwidth should be non negative and less than 45MHz

//#define		ANALOG_FILTER_BW			24
#define		ANALOG_FILTER_BW			30
//bandwidth of analog filter, at unit of MHz
//too small value will result in negative tuner shift frequency

#define		DELTA_FREQUENCY_SHIFT		7
//leave some interval from FFT window left side to possible channel rising edge
//when tune tuner mixer frequency, at unit of MHz
//7MHz is maximum roll off bandwidth of DVB-S signal, ie., when symbol rate is 45Msps

#define		MIN_NOISE_LENGTH			4
//0.05859375MHz*MIN_NOISE_LENGTH = minimum bandwidth of data to be taken as noise in FPGA version
//0.08789MHz*MIN_NOISE_LENGTH = minimum bandwidth of data to be taken as noise in asic version

#define		MIN_SIGNAL_LENGTH			13//18
//0.05859375MHz*MIN_SIGNAL_LENGTH = minimum bandwidth of data to be taken as signal in FPGA version
//0.08789MHz*MIN_SIGNAL_LENGTH = minimum bandwidth of data to be taken as signal in asic version

INT32 nim_s3501_find_tp(INT32 *addrstart, INT32 *addrend, INT32 *tp_count, INT32 chlspec_num, 
                               INT32 *channel_spectrum, UINT32 adc_sample_freq);
void nim_s3501_adjust_window_wide(INT32 flength, INT32 *fdata, INT32 *fdata_new);
INT32 nim_s3501_add_tp(INT32 *channel_spectrum, INT32 start_addr, INT32 end_addr, 
                                INT32 *left_addr, INT32 *right_addr);
void nim_s3501_reorder(INT32 index, INT32 *fdata, INT32 mf_window_len);
void nim_s3501_find_max_value(INT32 start_addr, INT32 end_addr, INT32 *fdata, INT32 *flag, 
                                     INT32 factor, INT32 sel);
void nim_s3501_adjust_window_wide_x
(
	INT32 count0,
    INT32 count1,
	INT32 last_left,
    INT32 last_right,
	INT32 delta_cnt,
	INT32 i,
	INT32 *fdata
	);
INT32 nim_s3501_fs_estimate(INT32 *addrstart,
                            INT32 *addrend,
                            INT32 tp_num,
                            INT32 *channel_spectrum,
                            UINT32 sfreq,
                            UINT32 adc_sample_freq);

INT32 nim_s3501_repeat_detect(INT32 *addrstart, INT32 *addrend, INT32 tp_count, INT32 left_addr, 
                                     INT32 right_addr);



INT32 nim_s3501_autosearch(INT32 *success, INT32 *delta_fc_est, INT32 *symbol_rate_est, INT32 *m_if_freq, 
                            INT32 *m_ch_number);
void nim_s3501_smoothfilter(void);
void nim_s3501_median_filter(INT32 flength, INT32 *fdata, INT32 scan_mode);
INT32 nim_s3501_search_tp(	INT32 chlspec_num,
                            INT32 *channel_spectrum,
                            UINT32 sfreq,
                            UINT32 adc_sample_freq,
                            INT32 loop);
INT32 nim_s3501_fft_wideband_scan(UINT32 tuner_if, UINT32 adc_sample_freq);

#endif

