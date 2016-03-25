#if defined(__NIM_LINUX_PLATFORM__)
#include "../porting_linux_header.h"
#include "porting_m3501_linux.h"
#elif defined(__NIM_TDS_PLATFORM__)
#include "../m3501/porting_tds_header.h"
#include "../m3501/porting_m3501_tds.h"

#endif

#include "nim_s3501_autosearch.h"


#define CNT_VAL_2 2
#define CNT_VAL_20 20
#define CNT_VAL_199 199
#define SAMPLE_FREQ_92 92
#define FFT_VAL_700 700
#define FFT_VAL_10 10
#define FFT_POINT_9 9
#define MAX_VAL_25BIT 33554431

typedef struct _s3501_find_tp_params
{
	INT32 *addrstart;
	INT32 *addrend;
	INT32 chlspec_num;
	INT32 *channel_spectrum;
	UINT32 adc_sample_freq;
	UINT8 *addrflag;
	INT32 *addr;	
	INT32 freq_end_addr;
	INT32 comparecnt;
	INT32 number_tp;
	INT32 addrcnt;
	INT32 maxvalue;
	INT32 average;
	INT32 subaverage;
	INT32 compare1;
	INT32 compare2;
	INT32 last_a ;	
}NIM_S3501_FIND_TP_PARAMS;



static INT32 nim_s3501_find_tp_condition1(NIM_S3501_FIND_TP_PARAMS *params, INT32 i)
{
    INT32 left_addr = 0;
    INT32 right_addr = 0;
	INT32 tp_bandwidth = 0;
	INT32 k = 0;
	
    //Add this TP
    nim_s3501_add_tp(params->channel_spectrum, params->addr[params->addrcnt], i, &left_addr, &right_addr);
    tp_bandwidth = ((right_addr - left_addr + 1) * params->adc_sample_freq + FFT_LENGTH / 2) / FFT_LENGTH;
    if((tp_bandwidth > MIN_BANDWIDTH) && (tp_bandwidth < MAX_BANDWIDTH))
    {
        if(1 == nim_s3501_repeat_detect(params->addrstart, params->addrend, params->number_tp - 1, left_addr, 
                           right_addr))
        {
            params->addrstart[params->number_tp] = left_addr;
            params->addrend[params->number_tp] = right_addr;
            AUTOSEARCH_PRINTF("Found_TP %d -> [ %d , %d ] ,path1\n", params->number_tp,
                      params->addrstart[params->number_tp], params->addrend[params->number_tp]);
            params->number_tp = params->number_tp + 1;
        }
    }

    //Add other's TPs
    if((params->addrflag[params->addrcnt] == 1) || (params->addrflag[params->addrcnt] == CNT_VAL_2))
    {
        for(k = params->addrcnt - 1; k >= 0; k--)
        {
            if(params->addrflag[k] == 1)
            {
                nim_s3501_add_tp(params->channel_spectrum, params->addr[k], i, &left_addr, &right_addr);
                tp_bandwidth = ((right_addr - left_addr + 1) * params->adc_sample_freq + \
                 FFT_LENGTH / 2) / FFT_LENGTH;
                if((tp_bandwidth > MIN_BANDWIDTH) && (tp_bandwidth < MAX_BANDWIDTH))
                {
                    if(1 == nim_s3501_repeat_detect(params->addrstart, params->addrend, params->number_tp - 1, 
                                    left_addr, right_addr))
                    {
                        params->addrstart[params->number_tp] = left_addr;
                        params->addrend[params->number_tp] = right_addr;
                        AUTOSEARCH_PRINTF("Found_TP %d -> [ %d , %d ] ,path2\n", 
	                      params->number_tp, params->addrstart[params->number_tp], params->addrend[params->number_tp]);
                        params->number_tp = params->number_tp + 1;
                    }
                }
            }
            else if(params->addrflag[k] == 0)
            {
                nim_s3501_add_tp(params->channel_spectrum, params->addr[k], i, &left_addr, &right_addr);
                tp_bandwidth = ((right_addr - left_addr + 1) * params->adc_sample_freq + \
                 FFT_LENGTH / 2) / FFT_LENGTH;
                if((tp_bandwidth > MIN_BANDWIDTH) && (tp_bandwidth < MAX_BANDWIDTH))
                {
                    if(1 == nim_s3501_repeat_detect(params->addrstart, params->addrend, params->number_tp - 1, 
                                    left_addr, right_addr))
                    {
                        params->addrstart[params->number_tp] = left_addr;
                        params->addrend[params->number_tp] = right_addr;
                        AUTOSEARCH_PRINTF("Found_TP %d -> [ %d , %d ] ,path3\n", 
	                 params->number_tp, params->addrstart[params->number_tp], params->addrend[params->number_tp]);
                        params->number_tp = params->number_tp + 1;
                    }
                }
                break;
            }
        }
    }

    //Add this point
    params->addrcnt++;
    params->addr[params->addrcnt] = i;
    if((params->addrflag[params->addrcnt - 1] == 1) || (params->addrflag[params->addrcnt - 1] == 2))
    {
        AUTOSEARCH_PRINTF("condition_1\n");
        if(params->last_a == 0)
        {
           params->last_a = params->compare1;
        }

        AUTOSEARCH_PRINTF("compare[%d.%d]\n", params->last_a, params->compare2);
        if(params->last_a >= params->compare2)
        {
            params->addrflag[params->addrcnt] = 0;
        }
        else
        {
            if(((params->compare2 - params->last_a) > (params->last_a * 2 / 3)) && 
	                             (params->average <= (params->compare2 * 5)))
            {
                params->addrflag[params->addrcnt] = 1;
            }
            else
            {
                params->addrflag[params->addrcnt] = 0;
            }
        }
    }
    else if(((params->compare1 * 2) <= params->compare2) && (params->average <= (params->compare2 * 5)))
    {
        params->addrflag[params->addrcnt] = 1;
        if (params->addrflag[params->addrcnt - 1] == 0)
        {
            params->last_a = params->compare1;
        }
    }
    else
    {
       params->addrflag[params->addrcnt] = 0;
    }
	return SUCCESS;
}

static INT32 nim_s3501_find_tp_condition2(NIM_S3501_FIND_TP_PARAMS *params, INT32 i)
{

    if((params->addrflag[params->addrcnt] == 1) || (params->addrflag[params->addrcnt] == CNT_VAL_2))
    {
        AUTOSEARCH_PRINTF("condition_4\n");
        if(params->last_a == 0)
        {
              params->last_a = params->compare1;
        }

        AUTOSEARCH_PRINTF("compare[ %d, %d ]\n", params->last_a, params->compare2);
        if(params->last_a >= params->compare2)
        {
            params->addrcnt++;
            params->addr[params->addrcnt] = i;
            params->addrflag[params->addrcnt] = 0;
        }
        else
        {
            if((params->compare2 - params->last_a) < (params->last_a * 2 / 3))
            {
                params->addrcnt++;
                params->addr[params->addrcnt] = i;
                params->addrflag[params->addrcnt] = 0;
            }
            else
            {
                if(params->channel_spectrum[params->addr[params->addrcnt]] == params->channel_spectrum[i])
                {
                    params->addr[params->addrcnt] = i;
                }
                else
                {
                    params->addrcnt++;
                    params->addr[params->addrcnt] = i;
                    params->addrflag[params->addrcnt] = 2;
                }
            }
        }
    }
    else
    {
        params->addrcnt++;
        params->addr[params->addrcnt] = i;
        params->addrflag[params->addrcnt] = 0;
    }
	return SUCCESS;				
}

static INT32 nim_s3501_find_tp_condition3(NIM_S3501_FIND_TP_PARAMS *params, INT32 i)
{
    INT32 left_addr = 0;
    INT32 right_addr = 0;
	INT32 tp_bandwidth = 0;

	
    if((params->compare2 - params->subaverage) <= params->subaverage / 3)
    {
        nim_s3501_add_tp(params->channel_spectrum, params->addr[params->addrcnt], i, &left_addr, &right_addr);
        tp_bandwidth = ((right_addr - left_addr + 1) * params->adc_sample_freq + FFT_LENGTH / 2) / FFT_LENGTH;
        if((tp_bandwidth > MIN_BANDWIDTH) && (tp_bandwidth < MAX_BANDWIDTH))
        {
            if(1 == nim_s3501_repeat_detect(params->addrstart, params->addrend, params->number_tp - 1, 
	                                      left_addr, right_addr))
            {
                params->addrstart[params->number_tp] = left_addr;
                params->addrend[params->number_tp] = right_addr;
                AUTOSEARCH_PRINTF("Found_TP %d -> [ %d , %d ] ,path4\n", params->number_tp, 
                  params->addrstart[params->number_tp], params->addrend[params->number_tp]);
                params->number_tp = params->number_tp + 1;
            }
        }

        params->addrcnt++;
        params->addr[params->addrcnt] = i;
        params->addrflag[params->addrcnt] = 1;

        if(params->addrflag[params->addrcnt - 1] == 0)
        {
            params->last_a = params->compare1;
        }
        else if((params->addrflag[params->addrcnt - 1] == 2) && (params->compare1 * 2 <= params->compare2))
        {
            params->addrflag[params->addrcnt - 1] = 1;
        }
    }
    else if((params->addrflag[params->addrcnt] == 1) || (params->addrflag[params->addrcnt] == 2))
    {
        if(params->channel_spectrum[params->addr[params->addrcnt]] == params->channel_spectrum[i])
        {
            params->addr[params->addrcnt] = i;
        }
        else if((params->addrflag[params->addrcnt] == 2) && (params->compare1 * 2 <= params->compare2))
        {
            params->addrflag[params->addrcnt] = 1;
        }
        else
        {
            params->addrcnt++;
            params->addr[params->addrcnt] = i;
            params->addrflag[params->addrcnt] = 2;
        }
    }
	return SUCCESS;
}


static INT32 nim_s3501_find_tp_condition4(NIM_S3501_FIND_TP_PARAMS *params, INT32 i)
{
    INT32 left_addr = 0;
    INT32 right_addr = 0;
	INT32 tp_bandwidth = 0;
	INT32 k = 0;
	
    if(1)
    {
        nim_s3501_add_tp(params->channel_spectrum, params->addr[params->addrcnt], i, &left_addr, &right_addr);
        tp_bandwidth = ((right_addr - left_addr + 1) * params->adc_sample_freq + FFT_LENGTH / 2) / FFT_LENGTH;
        if((tp_bandwidth > MIN_BANDWIDTH) && (tp_bandwidth < MAX_BANDWIDTH))
        {
            if(1 == nim_s3501_repeat_detect(params->addrstart, params->addrend, 
	                                 params->number_tp - 1, left_addr, right_addr))
            {
                params->addrstart[params->number_tp] = left_addr;
                params->addrend[params->number_tp] = right_addr;
                AUTOSEARCH_PRINTF("Found_TP %d -> [ %d , %d ] ,path5\n", params->number_tp, 
                    params->addrstart[params->number_tp], params->addrend[params->number_tp]);
                params->number_tp = params->number_tp + 1;
            }
        }
    }

    //////Est_TP before addr[addrcnt]
    if((params->addrflag[params->addrcnt] == 1) || (params->addrflag[params->addrcnt] == CNT_VAL_2))
    {
        for(k = params->addrcnt - 1; k >= 0; k--)
        {
            if(params->addrflag[k] == 1)
            {
                nim_s3501_add_tp(params->channel_spectrum, params->addr[k], i, &left_addr, &right_addr);
                tp_bandwidth = ((right_addr - left_addr + 1) * params->adc_sample_freq + \
                 FFT_LENGTH / 2) / FFT_LENGTH;
                if((tp_bandwidth > MIN_BANDWIDTH) && (tp_bandwidth < MAX_BANDWIDTH))
                {
                    if(1 == nim_s3501_repeat_detect(params->addrstart, params->addrend, params->number_tp - 1, 
                                     left_addr, right_addr))
                    {
                        params->addrstart[params->number_tp] = left_addr;
                        params->addrend[params->number_tp] = right_addr;
                        AUTOSEARCH_PRINTF("Found_TP %d -> [ %d , %d ] ,path6\n", params->number_tp,
	                    params->addrstart[params->number_tp], params->addrend[params->number_tp]);
                        params->number_tp = params->number_tp + 1;
                    }
                }
            }
            else if(params->addrflag[k] == 0)
            {
                nim_s3501_add_tp(params->channel_spectrum, params->addr[k], i, &left_addr, &right_addr);
                tp_bandwidth = ((right_addr - left_addr + 1) * params->adc_sample_freq +\
                FFT_LENGTH / 2) / FFT_LENGTH;
                if((tp_bandwidth > MIN_BANDWIDTH) && (tp_bandwidth < MAX_BANDWIDTH))
                {
                    if(1 == nim_s3501_repeat_detect(params->addrstart, params->addrend, params->number_tp - 1, 
                                    left_addr, right_addr))
                    {
                        params->addrstart[params->number_tp] = left_addr;
                        params->addrend[params->number_tp] = right_addr;
                        AUTOSEARCH_PRINTF("Found_TP %d -> [ %d , %d ] ,path7\n", 
	                 params->number_tp, params->addrstart[params->number_tp], params->addrend[params->number_tp]);
                        params->number_tp = params->number_tp + 1;
                    }
                }
                break;
            }
        }

        params->addrcnt++;
        params->addr[params->addrcnt] = i;

        AUTOSEARCH_PRINTF("condition_3\n");
        if(params->last_a == 0)
        {
            params->last_a = params->compare1;
        }  

        AUTOSEARCH_PRINTF("compare[%d,%d]\n", params->last_a, params->compare2);
        if(params->last_a >= params->compare2)
        {
            params->addrflag[params->addrcnt] = 0;
        }
        else
        {
            if(((params->compare2 - params->last_a) < (params->last_a * 2 / 3)) || 
	                (params->compare1 >= (params->compare2 * 3)))
            {
                 params->addrflag[params->addrcnt] = 0;
            }
            else
            {
                 params->addrflag[params->addrcnt] = 1;
            } 
        }
    }
    else if(params->compare1 >= (params->compare2 * 2))
    {
        for(k = params->addrcnt - 1; k >= 0; k--)
        {
            if(params->addrflag[k] == 1)
            {
                nim_s3501_add_tp(params->channel_spectrum, params->addr[k], i, &left_addr, &right_addr);
                tp_bandwidth = ((right_addr - left_addr + 1) * params->adc_sample_freq + \
                FFT_LENGTH / 2) / FFT_LENGTH;
                if((tp_bandwidth > MIN_BANDWIDTH) && (tp_bandwidth < MAX_BANDWIDTH))
                {
                    if(1 == nim_s3501_repeat_detect(params->addrstart, params->addrend, params->number_tp - 1, 
                                    left_addr, right_addr))
                    {
                        params->addrstart[params->number_tp] = left_addr;
                        params->addrend[params->number_tp] = right_addr;
                        AUTOSEARCH_PRINTF("Found_TP %d -> [ %d , %d ] ,path8\n", 
	                   params->number_tp, params->addrstart[params->number_tp], params->addrend[params->number_tp]);
                        params->number_tp = params->number_tp + 1;
                    }
                }
            }
            else if(params->addrflag[k] == 0)
            {
                nim_s3501_add_tp(params->channel_spectrum, params->addr[k], i, &left_addr, &right_addr);
                tp_bandwidth = ((right_addr - left_addr + 1) * params->adc_sample_freq + \
                 FFT_LENGTH / 2) / FFT_LENGTH;
                if((tp_bandwidth > MIN_BANDWIDTH) && (tp_bandwidth < MAX_BANDWIDTH))
                {
                    if(1 == nim_s3501_repeat_detect(params->addrstart, params->addrend, params->number_tp - 1, 
                                    left_addr, right_addr))
                    {
                        params->addrstart[params->number_tp] = left_addr;
                        params->addrend[params->number_tp] = right_addr;
                        AUTOSEARCH_PRINTF("Found_TP %d -> [ %d , %d ] ,path9\n", params->number_tp, 
	                  params->addrstart[params->number_tp], params->addrend[params->number_tp]);
                        params->number_tp = params->number_tp + 1;
                    }
                }
                break;
            }
        }

        params->addrcnt++;
        params->addr[params->addrcnt] = i;
        params->addrflag[params->addrcnt] = 0;
    }
    else
    {
        params->addrcnt++;
        params->addr[params->addrcnt] = i;
        params->addrflag[params->addrcnt] = 0;
    }
    return SUCCESS;
	
}


INT32 nim_s3501_find_tp(INT32 *addrstart, INT32 *addrend, INT32 *tp_count, INT32 chlspec_num, 
                               INT32 *channel_spectrum, UINT32 adc_sample_freq)
{
	NIM_S3501_FIND_TP_PARAMS params;
    INT32 i = 0;
    INT32 j = 0;
    INT32 m = 0;
    INT32 comparecnt = 10;
    INT32 risecnt = 0;
    INT32 fallcnt = 0;
    UINT8 flag = 1;
    INT32 lastmaxvalue = 0;
    
    INT32 maxvalueaddr = 0;
    INT32 subsum = 0;
    INT32 subcnt = 0;
    INT32 freq_start_addr = 0;
    INT32 freq_end_addr = 0;

    if(adc_sample_freq <= SAMPLE_FREQ_92)
    {
        comparecnt = 10;
    }
    else
    {
        comparecnt = 8;
    }

	memset(&params,0,sizeof(NIM_S3501_FIND_TP_PARAMS));
	
    params.addrflag = (UINT8 *)comm_malloc(200 * sizeof(UINT8));
    params.addr = (INT32 *)comm_malloc(200 * sizeof(INT32));	
    if( (NULL == params.addrflag) || (NULL == params.addr))
    {
        AUTOSEARCH_PRINTF("addrflag[] or addr[] malloc failed!\n");
        return 1;
    }
    comm_memset(params.addrflag, 0, 200 * sizeof(UINT8));
    comm_memset(params.addr, 0, 200 * sizeof(INT32));

    freq_start_addr = FREQ_SEARCH_START_INDEX + comparecnt;
    freq_end_addr = chlspec_num - FREQ_SEARCH_START_INDEX - comparecnt;

	params.addrstart = addrstart;
	params.addrend = addrend;
	params.chlspec_num = chlspec_num;
	params.channel_spectrum = channel_spectrum;
	params.adc_sample_freq = adc_sample_freq;
	params.freq_end_addr = freq_end_addr;
	params.comparecnt = comparecnt;

    for(i = freq_start_addr; i <= freq_end_addr; i++)
    {
        if(params.addrcnt == CNT_VAL_199)
        {
            for(m = 0; m < 100; m++)
            {
                AUTOSEARCH_PRINTF("Find_TP addr-->%d : %d\n", m, params.addr[m]);
                AUTOSEARCH_PRINTF("Find_TP addrflag-->%d : %d\n", m, params.addrflag[m]);
            }

            for(m = 100; m < 200; m++)
            {
                params.addr[m - 100] = params.addr[m];
                params.addrflag[m - 100] = params.addrflag[m];
            }

            params.addrcnt = 99;
        }

        risecnt = 0;
        for(j = i - comparecnt; j < i; j++)
        {
            if(channel_spectrum[j] >= channel_spectrum[i])
            {
	        risecnt = risecnt + 1;
            }   
            else
            {
	        risecnt = 0;
             }
        }

        fallcnt = 0;
        for(j = i + 1; j <= i + comparecnt; j++)
        {
            if(channel_spectrum[j] >= channel_spectrum[i])
            {
	        fallcnt = fallcnt + 1;
            } 
            else
            {
	        fallcnt = 0;
            } 
        }

        if(((risecnt == comparecnt) && (fallcnt == comparecnt)) || (i == freq_end_addr))
        {
            if(flag == 1)
            {
                params.addr[params.addrcnt] = i;
                flag = 0;
            }
            else
            {
                if(((i - params.addr[params.addrcnt]) == 1) && (params.addrflag[params.addrcnt] == 0))
                {
                    params.addr[params.addrcnt] = i;
                    continue;
                }

                params.maxvalue = 0;
                maxvalueaddr = 0;
                for(j = params.addr[params.addrcnt]; j <= i; j++)
                {
                    if(channel_spectrum[j] >= params.maxvalue)
                    {
                        params.maxvalue = channel_spectrum[j];
                        maxvalueaddr = j;
                    }
                }

                if(params.maxvalue <= 0)  
		        {
		          continue;
                } 

                subsum = 0;
                subcnt = 0;
                for(j = maxvalueaddr - 2; j <= maxvalueaddr + 2; j++)
                {
                    subsum = subsum + channel_spectrum[j];
                    subcnt = subcnt + 1;
                }
                params.average = (subsum + 2) / subcnt;
                params.subaverage = (2 * params.average + 2) / 3;

                params.compare1 = channel_spectrum[params.addr[params.addrcnt]];
                params.compare2 = channel_spectrum[i];

                AUTOSEARCH_PRINTF("Est_Going [%d,%d],Compare{%d,%d,%d}\n", params.addr[params.addrcnt], i, 
					      params.compare1, params.subaverage, params.compare2);

                if((params.subaverage >= params.compare1) && (params.subaverage >= params.compare2))
                {
     				nim_s3501_find_tp_condition1(&params, i);
                }
                else if((params.subaverage < params.compare1) && (params.subaverage < params.compare2))
                {
                   nim_s3501_find_tp_condition2(&params, i);
                }
                else if((params.subaverage >= params.compare1) && (params.subaverage < params.compare2))
                {
                  nim_s3501_find_tp_condition3(&params, i);
                }
                else if((params.subaverage < params.compare1) && (params.subaverage >= params.compare2))
                {
                   nim_s3501_find_tp_condition4(&params,i);
                }

                if(params.addrflag[params.addrcnt] == 0)
                {
                    params.last_a = 0;
                }
                lastmaxvalue = params.maxvalue;
            }
        }
    }

    *tp_count =params.number_tp;

    for(m = 0; m < params.addrcnt + 1; m++)
    {
        AUTOSEARCH_PRINTF("Find_TP addr-->%d : %d\n", m, params.addr[m]);
        AUTOSEARCH_PRINTF("Find_TP addrflag-->%d : %d\n", m, params.addrflag[m]);
    }

    for(i = 0; i < params.number_tp; i++)
    {
        AUTOSEARCH_PRINTF("Find_TP addrstart-->%d : %d\n", i, params.addrstart[i]);
        AUTOSEARCH_PRINTF("Find_TP addrend-->%d : %d\n", i, params.addrend[i]);
    }

    comm_free(params.addr);
    comm_free(params.addrflag);
    params.addr = NULL;
    params.addrflag = NULL;

    AUTOSEARCH_PRINTF("number_tp = %d\n", params.number_tp);
    return 0;
}


static void nim_s3501_smoothfilter3(INT32 number, INT32 *channel_spectrum)
{
    INT32 i = 0;
    INT32 j = 0;
    INT32 min_index = 0;
    INT32 max_index = 0;
    INT32 data_tmp = 0;
    INT32 t_data_tmp = 0;
    INT32 data_tmp2[5]={0};

    INT32 average_tmp = 0;
    INT32 min_num = 0;
    INT32 max_num = 0;
    INT32  filter_taps = 4;

    for(i = filter_taps - 1; i < number - 4; i++)
    {
        t_data_tmp = 0;
        if(channel_spectrum[i] > FFT_VAL_700 )
        {
            //first change the biggest and smallest num to the average, and then filter
            min_num = channel_spectrum[i];
            max_num = channel_spectrum[i];
            for(j = 0; j < filter_taps; j++)
            {
                t_data_tmp += channel_spectrum[i - j];
                if(min_num > channel_spectrum[i - j] )
                {
                    min_num = channel_spectrum[i - j];
                    min_index = j;
                }
                if(max_num < channel_spectrum[i - j] )
                {
                    max_num = channel_spectrum[i - j];
                    max_index = j;
                }
            }
            t_data_tmp /= filter_taps;
            average_tmp = t_data_tmp;
            channel_spectrum[i - min_index] = average_tmp;
            channel_spectrum[i - max_index] = average_tmp;

            for(j = 0; j < filter_taps; j++)
            {
                t_data_tmp += channel_spectrum[i - j];
            }
            t_data_tmp /= filter_taps;
            channel_spectrum[i - filter_taps / 2] = t_data_tmp;
        }
    }

    for (i = 5; i < number - 5; i++)
    {
        //filter = [1/64 1/32 1/16 1/8 1/4 1/2 1/4 1/8 1/16 1/32 1/64];
        data_tmp = *(channel_spectrum + i - 5)    + *(channel_spectrum + i + 5);
        data_tmp += (*(channel_spectrum + i - 4) << 1) + (*(channel_spectrum + i + 4) << 1);
        data_tmp += (*(channel_spectrum + i - 3) << 2) + (*(channel_spectrum + i + 3) << 2);
        data_tmp += ((*(channel_spectrum + i - 2) << 3) + (*(channel_spectrum + i + 2) << 3)) * 2 / 3;
        data_tmp += ((*(channel_spectrum + i - 1) << 4) + (*(channel_spectrum + i + 1) << 4)) * 2 / 3;
        data_tmp += (*(channel_spectrum + i  ) << 5) * 3 / 4;
        data_tmp >>= 6;

        if (i > FFT_POINT_9)
        {
            *(channel_spectrum + i - 5) = data_tmp2[0];
        }

        for (j = 0; j < 4; j++)
        {
            data_tmp2[j] = data_tmp2[j + 1];
        }

        if (data_tmp > MAX_VAL_25BIT)	// max 25bit
        {
            data_tmp2[4] = 33554431;
        }
        else
        {
            data_tmp2[4] = data_tmp;
        }
    }

    for (i = 0; i < 5; i++)
    {
        *(channel_spectrum + number - 1 + i - 9) = data_tmp2[i];
    }
}



INT32 nim_s3501_search_tp(INT32 chlspec_num,
                         INT32 *channel_spectrum,
                         UINT32 sfreq,
                         UINT32 adc_sample_freq,
                         INT32 loop)
{
    INT32 i = 0;
    INT32 j = 0;
    INT32 *addrstart = NULL;
    INT32 *addrend = NULL;
    INT32 tp_count = 0;

    addrstart = (INT32 *)comm_malloc(TP_MAXNUM * sizeof(INT32));
    addrend = (INT32 *)comm_malloc(TP_MAXNUM * sizeof(INT32));
    if ((NULL == addrstart) || (NULL == addrend))
    {
        AUTOSEARCH_PRINTF("addrstart[] or addrend[]  malloc failed!\n");
        return 1;
    }
    comm_memset(addrstart, 0, TP_MAXNUM * sizeof(INT32));
    comm_memset(addrend, 0, TP_MAXNUM * sizeof(INT32));

    for(i = 0; i < chlspec_num; i++)
    {
        if((*(channel_spectrum + i) < FFT_VAL_10) && (*(channel_spectrum + i) > 0))
        {
            *(channel_spectrum + i) = 10;
        }
        else if(*(channel_spectrum + i) <= 0)
        {
            INT32 tmp0 = 0;
	    INT32 tmp1 = 0;

            for(j = i; j >= 0; j--)
            {
                if(*(channel_spectrum + j) > 0)
                {
                    tmp0 = j;
                    break;
                }
            }
            for(j = i; j < chlspec_num; j++)
            {
                if(*(channel_spectrum + j) > 0)
                {
                    tmp1 = j;
                    break;
                }
            }
            *(channel_spectrum + i) = (tmp0 + tmp1 + 1) / 2;
        }
    }

    //filter
    nim_s3501_smoothfilter3(chlspec_num, channel_spectrum);

    for(i = 0; i < chlspec_num; i++)
    {
        AUTOSEARCH_PRINTF("WBSFFT_F->%d : %d\n", i, channel_spectrum[i]);
    }   

    //find_TP
    if(0 == nim_s3501_find_tp(addrstart, addrend, &tp_count, chlspec_num, channel_spectrum, adc_sample_freq))
    {
        AUTOSEARCH_PRINTF("tp_count=%d\n", tp_count);
        nim_s3501_fs_estimate(addrstart, addrend, tp_count, channel_spectrum, sfreq, adc_sample_freq);
    }

    comm_free(addrstart);
    comm_free(addrend);
    addrstart = NULL;
    addrend = NULL;

    return 0;
}


void nim_s3501_adjust_window_wide(INT32 flength, INT32 *fdata, INT32 *fdata_new)
{
    INT32 i = 0;
    INT32 j = 0;
    INT32 risecnt = 0;
    INT32 fallcnt = 0;
    INT32 comparecnt = 5;
    INT32 compareth = 20;
    INT32 count0 = 0;
    INT32 count1 = 0;
    INT32 last_left = 0;
    INT32 last_right = 0;
    INT32 last_addr = 0;
    INT32 delta_cnt = 0;
    UINT8 flag0 = 0;


    for(i = 0 + 100; i < flength - 100; i++)
    {
        risecnt = 0;
        for(j = i - 1; j >= i - comparecnt; j--)
        {
            if(fdata[j] > fdata[i])
            {
	        risecnt = risecnt + 1;
            } 
            else
            {
                risecnt = 0;
                break;
            }
        }

        fallcnt = 0;
        for(j = i + 1; j <= i + comparecnt; j++)
        {
            if(fdata[j] > fdata[i])
            {
	        fallcnt = fallcnt + 1;
            } 
            else if((fdata[j] == fdata[i]) && (fdata[j] == fdata[j - 1]))
            {
                fallcnt = fallcnt + 1;
            }
            else
            {
                fallcnt = 0;
                break;
            }
        }

        if((fallcnt == comparecnt) && (risecnt == comparecnt))
        {
            compareth = fdata[i] * 3 / 2;

            flag0 = 0;
            count0 = 0;
            for(j = i - 1; j >= 0; j--)
            {
                if(flag0 == 0)
                {
                    if(fdata[j] > compareth)
                    {
                        count0++;
                        flag0 = 1;
                        last_left = j;
                    }
                    else if((i - j) > 10)
                    {
                        break;
                    }
                }
                else
                {
                    if(fdata[j] < compareth)
                    {
                        break;
                    }
                    else
                    {
                        count0++;
                    }
                }
            }

            flag0 = 0;
            count1 = 0;
            for(j = i + 1; j < flength; j++)
            {
                if(flag0 == 0)
                {
                    if(fdata[j] > compareth)
                    {
                        count1++;
                        flag0 = 1;
                        last_right = j;
                    }
                    else if((j - i) > 10)
                    {
                        break;
                    }
                }
                else
                {
                    if(fdata[j] < compareth)
                    {
                        break;
                    }
                    else
                    {
                        count1++;
                    }
                }
            }

            delta_cnt = last_right - last_left;

           nim_s3501_adjust_window_wide_x(count0,count1,last_left,last_right,delta_cnt,i,fdata);

            last_addr = i;
        }
    }
}



INT32 nim_s3501_add_tp(INT32 *channel_spectrum, INT32 start_addr, INT32 end_addr, 
                                INT32 *left_addr, INT32 *right_addr)
{
    INT32 compare1 = 0;
    INT32 compare2 = 0;
    INT32 j = 0;
    INT32 maxvalue = 0;
    INT32 maxvalueaddr = 0;
    INT32 sedmaxvalue = 0;
    INT32 compareth = 0;
    INT32 tp_valid = 0;
    INT32 cnt = 0;

    *left_addr = 0;
    *right_addr = 0;

    compare1 = channel_spectrum[start_addr];
    compare2 = channel_spectrum[end_addr];

    tp_valid = 1;

    compareth = (compare1 >= compare2) ? compare1 : compare2;
    for(j = start_addr; j <= end_addr; j++)
    {
        if(channel_spectrum[j] > compareth)
        {
            *left_addr = j;
            break;
        }
    }

    for(j = end_addr; j >= start_addr; j--)
    {
        if(channel_spectrum[j] > compareth)
        {
            *right_addr = j;
            break;
        }
    }

    for(j = *left_addr + 1; j < *right_addr; j++)
    {
        if(channel_spectrum[j] < compareth)
        {
            *left_addr = 0;
            *right_addr = 0;
            tp_valid = 0;
            break;
        }
    }

    if(tp_valid == 1)
    {
        tp_valid = 0;
        for(j = *left_addr + 1; j < *right_addr; j++)
        {
            if(channel_spectrum[j] > compareth)
            {
                tp_valid = 1;
                break;
            }
        }
    }

    if(tp_valid == 1)
    {
        //find Max_Value
        maxvalue = 0;
        maxvalueaddr = 0;
        for(j = start_addr; j <= end_addr; j++)
        {
            if(channel_spectrum[j] >= maxvalue)
            {
                maxvalue = channel_spectrum[j];
                maxvalueaddr = j;
            }
        }

        //find right_Threshold
        compareth = (compare1 >= compare2) ? compare1 : compare2;
        cnt = 0;
        for(j = start_addr; j <= end_addr; j++)
        {
            if(cnt == 0)
            {
                if(channel_spectrum[j] > compareth)
                {
                    cnt++;
                }
            }
            else
            {
                if(channel_spectrum[j] <= compareth)
                {
                    break;
                }
                else
                {
                    cnt++;
                }
            }
        }

        if(cnt >= CNT_VAL_20)
        {
            sedmaxvalue = maxvalue * 3 / 4; /////Taixing5:3719/V/1600
        }
        else
        {
            sedmaxvalue = maxvalue * 1 / 2; //////Yatai5:12443/V/2488
        }

        //sedmaxvalue=maxvalue*1/2;

        maxvalue = maxvalue / 100;
        if((maxvalue > compare1) && (maxvalue > compare2))
        {
            compareth = maxvalue;
        }
        else if((compare2 > sedmaxvalue) || (compare1 > sedmaxvalue))
        {
            compareth = sedmaxvalue;
        }
        else if(compare1 >= compare2)
        {
            compareth = compare1;
        }
        else
        {
            compareth = compare2;
        }
        for(j = start_addr; j <= end_addr; j++)
        {
            if(channel_spectrum[j] > compareth)
            {
                *left_addr = j;
                break;
            }
        }

        for(j = end_addr; j >= start_addr; j--)
        {
            if(channel_spectrum[j] > compareth)
            {
                *right_addr = j;
                break;
            }
        }
    }
    return 1;
}


void nim_s3501_reorder(INT32 index, INT32 *fdata, INT32 mf_window_len)
{
    INT32 i = 0;
    INT32 j = 0;
    INT32 k = 0;
    INT32 max_addr = 0;
    INT32 min_addr = 0;
    INT32 data[2 * WINDOW_LEN + 1]={0};
    INT32 data_flag[2 * WINDOW_LEN + 1]={0};
    INT32 average = 0;
    INT32 count = 0;

    INT32 se_l = 0;

    comm_memset(data, 0, sizeof(data));
    comm_memset(data_flag, 0, sizeof(data_flag));

    k = 0;
    for(i = index - mf_window_len; i <= index + mf_window_len; i++)
    {
        data[k] = fdata[i];
        k++;
    }

    se_l = k;
    for(j = 0; j < mf_window_len; j++)
    {
        for(k = 0; k < se_l; k++)
        {
            if(0 == data_flag[k])
            {
                max_addr = k;
                min_addr = k;
                break;
            }
        }
        for(k = 0; k < se_l; k++)
        {
            if(0 == data_flag[k])
            {
                if(data[k] >= data[max_addr])
                {
                    max_addr = k;
                }
                else if(data[k] <= data[min_addr])
                {
                    min_addr = k;
                }
            }
        }
        data_flag[max_addr] = 1;
        data_flag[min_addr] = 1;
    }

    count = 0;
    for(k = 0; k < se_l; k++)
    {
        if(0 == data_flag[k])
        {
            average += data[k];
            count++;
        }
    }
	if(count>0)
	{
		average = (average + count / 2) / count;
        fdata[index] = average;
	}	
}

void nim_s3501_find_max_value(INT32 start_addr, INT32 end_addr, INT32 *fdata, INT32 *flag, 
                                     INT32 factor, INT32 sel)
{
    INT32 i = 0;
    INT32 submaxvalue = 0;

    ////find max-value
    submaxvalue = 0;
    for(i = start_addr + 1; i < end_addr; i++)
    {
        if(fdata[i] > submaxvalue)
        {
            submaxvalue = fdata[i];
        }
    }
    submaxvalue = submaxvalue / factor;

    //////compare
    if(sel == 0)
    {
        /////select start_addr to compare
        if(submaxvalue >= fdata[start_addr])
        {
	    *flag = 1;
        } 
    }
    else if(sel == 1)
    {
        /////all selected
        if((submaxvalue >= fdata[start_addr]) && (submaxvalue >= fdata[end_addr]))
        {
	    *flag = 1;
        } 
    }
    else if(sel == 2)
    {
        /////select start_addr to compare
        if(submaxvalue >= fdata[end_addr])
        {
	    *flag = 1;
        }  
    }

}


