/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: xx_xxxx.c/xx_xxxx.h
*
*    Description: xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __LLD_NIM_S3281_H__
#define __LLD_NIM_S3281_H__


#if defined(__NIM_LINUX_PLATFORM__)
#include "porting_s3281_linux.h"
#elif defined(__NIM_TDS_PLATFORM__)
#include "porting_s3281_tds.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define S3281_DEBUG_FLAG    0





#define UNUSED_VARABLE(x)  do{}while(0) 


#if(S3281_DEBUG_FLAG)
#define QAM_DEBUG			1
#define TUNER_DEBUG			1
#else
#define QAM_DEBUG			0
#define TUNER_DEBUG			0
#endif
#if(QAM_DEBUG)
#define S3281_PRINTF		nim_print
#else
#define S3281_PRINTF(...)	do{}while(0)
#endif
#if(TUNER_DEBUG)
#define TUNER_PRINTF		nim_print
#else
#define TUNER_PRINTF(...)	do{}while(0)
#endif






/* Need to modify here for different perpose */
#define QAM_FPGA_USAGE	SYS_FUNC_OFF
#define QAM_ONLY_USAGE	SYS_FUNC_OFF

#ifndef QAM_FPGA_USAGE
#define	QAM_FPGA_USAGE	SYS_FUNC_OFF
#endif

#ifndef QAM_ONLY_USAGE
#define QAM_ONLY_USAGE  SYS_FUNC_OFF
#endif

#if (QAM_FPGA_USAGE == SYS_FUNC_ON)
#define	QAM_WORK_MODE	QAM_ONLY
#define	QAM_TEST_FUNC	SYS_FUNC_ON
#elif (QAM_ONLY_USAGE == SYS_FUNC_ON)
#define	QAM_WORK_MODE	QAM_ONLY
#define	QAM_TEST_FUNC	SYS_FUNC_OFF
#else
#define	QAM_WORK_MODE	QAM_SOC
#define	QAM_TEST_FUNC	SYS_FUNC_OFF
#endif

//joey 20080414 for 0x19/0x1a register debug.
#if (QAM_FPGA_USAGE == SYS_FUNC_ON)
#define sys_ic_get_rev_id()	(IC_REV_0 + 1)
#else
#define sys_ic_get_rev_id()	(IC_REV_0 + 2)
#endif


#define M3281_LOG_FUNC 0     // add for printf berper and CR

//add mode control for S3281
//#define S3281_J83B_SOC_BASE_ADDR  			0xb8028000
//#define S3281_SYS_MODE_SEL 					0xB80000F8
//#define S3281_SYS_CLK_SEL 					0xB80000FC
//#define S3281_SYS_AGC_SEL 					0xB80000F0


#ifdef TUNER_I2C_BYPASS
#define	I2C_BYPASS
#endif
//===============


#define S3281_DSP_CLK_54M_ADCLK         1
#define S3281_DSP_CLK_54M_PLL			0
#define S3281_DSP_CLK_60M				0
#define UNKNOWN_QAM_SUPPORT


#define QAM_ONLY   						0
#define QAM_SOC							1





#ifdef CONFIG_ARM
#define S3281_QAM_SOC_BASE_ADDR             0x18028000
#define S3281_SOC_BASE_ADDR      	        0x18000000

#define NIM_S3281_GET_BYTE(i)            __REG8ALI(i)        //(*(volatile UINT8 *)(i))
#define NIM_S3281_SET_BYTE(i,d)          __REG8ALI(i) = (d)  //(*(volatile UINT8 *)(i)) = (d)
#define NIM_S3281_GET_DWORD(i)            __REG32ALI(i)
#define NIM_S3281_SET_DWORD(i,d)          __REG32ALI(i) = (d)



#else
#define S3281_QAM_SOC_BASE_ADDR  			0xb8028000
#define S3281_SOC_BASE_ADDR      	        0xb8000000

#define NIM_S3281_GET_BYTE(i)             	(*(volatile UINT8 *)(i))
#define NIM_S3281_SET_BYTE(i,d)          	(*(volatile UINT8 *)(i)) = (d)
#define NIM_S3281_GET_DWORD(i)             	(*(volatile UINT32 *)(i))
#define NIM_S3281_SET_DWORD(i,d)          	(*(volatile UINT32 *)(i)) = (d)


#endif


#define S3281_QAM_ONLY_I2C_BASE_ADDR  		0x40



/*Register Name*/

//EXT_regfile of S3281
#define NIM_S3281_TSO0_R400                     0x400
#define nim_s3281_tsdummy_header0_r401          0x401
#define nim_s3281_tsdummy_header1_r402          0x402
#define nim_s3281_tsdummy_header2_r403          0x403
#define nim_s3281_tsdummy_header3_r404          0x404
#define nim_s3281_tsdummy_header4_r405          0x405
#define NIM_S3281_TSO1_R406                     0x406
#define NIM_S3281_CBR0_R407                     0x407
#define NIM_S3281_CBR1_R408                     0x408
#define NIM_S3281_CBR2_R409                     0x409
#define NIM_S3281_CBR3_R40A                     0x40a
#define NIM_S3281_CBR4_R40B                     0x40b
#define NIM_S3281_CBR5_R40C                     0x40c
#define NIM_S3281_CLKGATE_R40F                  0x40f
#define NIM_S3281_ECR_RPT0_R410                 0x410
#define NIM_S3281_ECR_RPT1_R411                 0x411
#define NIM_S3281_ECR_STEP_R412                 0x412
#define NIM_S3281_ECR_SET0_R413                 0x413
#define NIM_S3281_ECR_SET1_R414                 0x414
#define NIM_S3281_ECR_SET2_R415                 0x415
#define NIM_S3281_FRMSYNC_SET0_R416             0x416
#define NIM_S3281_FRMSYNC_SET1_R417             0x417
#define NIM_S3281_RS_SET0_R419                  0x419
#define NIM_S3281_RS_SET1_R41A                  0x41a
#define NIM_S3281_BYPASS_SET_R41B               0x41b
#define NIM_S3281_MEPG_SYNC0_R41C               0x41c
#define NIM_S3281_MEPG_SYNC1_R41D               0x41d
#define NIM_S3281_MEPG_SYNC2_R41E               0x41e
#define NIM_S3281_MEPG_SYNC3_R41F               0x41f
#define NIM_S3281_ANA_INT_SET_R420              0x420
#define NIM_S3281_ADC_SET0_R421                 0x421
#define NIM_S3281_ADC_SET1_R422                 0x422
#define NIM_S3281_ADC_DCOFFSET_R423             0x423
#define NIM_S3281_EVENT_LOG_R424                0x424
#define NIM_S3281_STATUS_RPT_R425               0x425
#define NIM_S3281_MPEGSYNC_SET0_R426            0x426
#define NIM_S3281_MPEGSYNC_SET1_R427            0x427
#define NIM_S3281_DEINT_BASEADDR0_R428          0x428
#define NIM_S3281_DEINT_BASEADDR1_R429          0x429
#define NIM_S3281_DEINT_BASEADDR2_R42A          0x42a
#define NIM_S3281_DEINT_BASEADDR3_R42B          0x42b
#define NIM_S3281_NULLPKT_PAD0_R42C             0x42c
#define NIM_S3281_NULLPKT_PAD1_R42D             0x42d
#define NIM_S3281_NULLPKT_PAD2_R42E             0x42e
#define NIM_S3281_NULLPKT_PAD3_R42F             0x42f
#define NIM_S3281_NULLPKT_PAD4_R430             0x430
#define NIM_S3281_NULLPKT_PAD5_R431             0x431
#define NIM_S3281_NULLPKT_PAD6_R432             0x432
#define NIM_S3281_NULLPKT_PAD7_R433             0x433
#define NIM_S3281_BERPER_RPT0_R438              0x438
#define NIM_S3281_BERPER_RPT1_R439              0x439
#define NIM_S3281_BERPER_RPT2_R43A              0x43a
#define NIM_S3281_BERPER_RPT3_R43B              0x43b
#define NIM_S3281_BERPER_RPT4_R43C              0x43c
#define NIM_S3281_BERPER_RPT5_R43D              0x43d
#define NIM_S3281_BERPER_RPT6_R43E              0x43e
#define NIM_S3281_DEINT_TEST_R440               0x440


//common Register
#define NIM_S3202_CONTROL1                      0x00
#define NIM_S3202_ADC_CONFIG                    0x01
#define NIM_S3202_INTERRUPT_EVENTS              0x02
#define NIM_S3202_INTERRUPT_MASK                0x03
#define NIM_S3202_CONTROL2                      0x04
#define NIM_S3202_I2C_CONTROL_GPIO              0x05

#define NIM_S3202_TIMEOUT_THRESHOLD             0x06
#define NIM_S3202_TS_OUT_FORMAT                 0x07
#define NIM_S3202_PER_REGISTER1                 0x08
#define NIM_S3202_PER_REGISTER2                 0x09


//AGC Register
#define NIM_S3202_AGC1                          0x0a
#define NIM_S3202_AGC2                          0x0b
#define NIM_S3202_AGC3                          0x0c
#define NIM_S3202_AGC4                          0x0d
#define NIM_S3202_AGC5                          0x0e
#define NIM_S3202_AGC6                          0x0f
#define NIM_S3202_AGC7                          0x10
#define NIM_S3202_AGC8                          0x11
#define NIM_S3202_AGC9                          0x12
#define NIM_S3202_AGC10                         0x13
#define NIM_S3202_AGC11                         0x14
#define NIM_S3202_AGC12                         0x15
#define NIM_S3202_AGC13                         0x16


//DCC Register
#define NIM_S3202_DCC                           0x17

//Filter Bank
#define NIM_S3202_FILTER_BANK                   0x18

//TR Loop register
#define NIM_S3202_TR_LOOP1                      0x19
#define NIM_S3202_TR_LOOP2                      0x1a
#define NIM_S3202_TR_LOOP3                      0x1b
#define NIM_S3202_TR_LOOP4                      0x1c
#define NIM_S3202_TR_LOOP5                      0x1d
#define NIM_S3202_TR_LOOP6                      0x1e
#define NIM_S3202_TR_LOOP7                      0x1f
#define NIM_S3202_TR_LOOP8                      0x20
#define NIM_S3202_TR_LOOP9                      0x21
#define NIM_S3202_TR_LOOP10                     0x22
#define NIM_S3202_TR_LOOP11                     0x23
#define NIM_S3202_TR_LOOP12                     0x24
#define NIM_S3202_TR_LOOP13                     0x25
#define NIM_S3202_TR_LOOP14                     0x26

//EQ Register
#define NIM_S3202_EQ1                           0x28
#define NIM_S3202_EQ2                           0x29
#define NIM_S3202_EQ3                           0x2a
#define NIM_S3202_EQ4                           0x2b
#define NIM_S3202_EQ5                           0x2c
#define NIM_S3202_EQ6                           0x2d
#define NIM_S3202_EQ7                           0x2e
#define NIM_S3202_EQ8                                0x2f
#define NIM_S3202_EQ9                                0x30
#define NIM_S3202_EQ10                                0x31
#define NIM_S3202_EQ11                                0x32
#define NIM_S3202_EQ12                                0x33
#define NIM_S3202_EQ13                                0x34
#define NIM_S3202_EQ14                                0x35

//Frame Sync
#define NIM_S3202_FRAME_SYNC1                        0x36
#define NIM_S3202_FRAME_SYNC2                        0x37

//FSM register
#define NIM_S3202_FSM1                                0x38
#define NIM_S3202_FSM2                                0x39
#define NIM_S3202_FSM3                                0x3a
#define NIM_S3202_FSM4                                0x3b
#define NIM_S3202_FSM5                                0x3c
#define NIM_S3202_FSM6                                0x3d
#define NIM_S3202_FSM7                                0x3e
#define NIM_S3202_FSM8                                0x3f
#define NIM_S3202_FSM9                                0x40
#define NIM_S3202_FSM10                            0x41
#define NIM_S3202_FSM11                            0x42
#define NIM_S3202_FSM12                            0x43
#define NIM_S3202_FSM13                            0x44
#define NIM_S3202_FSM14                            0x45
#define NIM_S3202_FSM15                            0x46
#define NIM_S3202_FSM16                            0x47
#define NIM_S3202_FSM17                            0x48
#define NIM_S3202_FSM18                            0x49
#define NIM_S3202_FSM19                            0x4a

//ESTM register
#define NIM_S3202_ESTM1                            0x4b
#define NIM_S3202_ESTM2                            0x4c
#define NIM_S3202_ESTM3                            0x4d
#define NIM_S3202_ESTM4                            0x4e
#define NIM_S3202_ESTM5                            0x4f

//PN
#define NIM_S3202_PN1                                0x50
#define NIM_S3202_PN2                                0x51


//TR Loop register
#define NIM_S3202_TR_LOOP15                        0x52
#define NIM_S3202_TR_LOOP16                        0x53
#define NIM_S3202_TR_LOOP17                        0x54
#define NIM_S3202_TR_LOOP18                        0x55

//Monitor Register
#define NIM_S3202_MONITOR1                        0x56
#define NIM_S3202_MONITOR2                        0x57
#define NIM_S3202_MONITOR3                        0x58
#define NIM_S3202_MONITOR4                        0x59
#define NIM_S3202_MONITOR5                        0x5a
#define NIM_S3202_MONITOR6                        0x5b


#define NIM_S3202_AGC15                            0x5c
#define NIM_S3202_AGC16                            0x5d


#define NIM_S3202_RS_BER1                            0x5e
#define NIM_S3202_RS_BER2                            0x5f
#define NIM_S3202_RS_BER3                            0x60
#define NIM_S3202_RS_BER4                            0x61
#define NIM_S3202_RS_BER5                            0x62

#define NIM_S3202_EQ_COEF1                            0x63
#define NIM_S3202_EQ_COEF2                            0x64
#define NIM_S3202_EQ_COEF3                            0x65
#define NIM_S3202_EQ_COEF4                            0x66
#define NIM_S3202_EQ_COEF5                            0x67

#define NIM_S3202_PLL_CFG                            0x68
#define NIM_S3202_EADC_CFG1                        0x69
#define NIM_S3202_EADC_CFG2                        0x6a

#define NIM_S3202_QAM_DIAGNOSE                    0x6b

#define NIM_S3202_SNR_MONI1                        0x6c
#define NIM_S3202_SNR_MONI2                        0x6d

#define NIM_S3202_I2C_REP                            0x6e

#define NIM_S3202_FSM_REG20                        0x73
#define NIM_S3202_FSM_REG21                        0x74
#define NIM_S3202_FSM_REG22                        0x75
#define NIM_S3202_FSM_REG23                        0x76

#define NIM_S3202_AGC17                            0x77
#define NIM_S3202_AGC18                            0x78

#define NIM_S3202_EQ15                                0x79
#define NIM_S3202_EQ16                                0x7a
#define NIM_S3202_EQ17                                0x7b
#define NIM_S3202_EQ18                                0x7c

#define NIM_S3202_MONITOR7                        0x7d

#define NIM_S3202_Q16_1                            0x80
#define NIM_S3202_Q16_2                            0x81
#define NIM_S3202_Q32_MODE0_1                        0x82
#define NIM_S3202_Q32_MODE0_2                        0x83
#define NIM_S3202_Q32_MODE1_1                        0x84
#define NIM_S3202_Q32_MODE1_2                        0x85
#define NIM_S3202_Q32_MODE2_1                        0x86
#define NIM_S3202_Q32_MODE2_2                        0x87

#define NIM_S3202_Q64_MODE0_1                        0x88
#define NIM_S3202_Q64_MODE0_2                        0x89
#define NIM_S3202_Q64_MODE1_1                        0x8a
#define NIM_S3202_Q64_MODE1_2                        0x8b
#define NIM_S3202_Q64_MODE2_1                        0x8c
#define NIM_S3202_Q64_MODE2_2                        0x8d

#define NIM_S3202_Q128_MODE0_1                    0x8e
#define NIM_S3202_Q128_MODE0_2                    0x8f
#define NIM_S3202_Q128_MODE1_1                    0x90
#define NIM_S3202_Q128_MODE1_2                    0x91
#define NIM_S3202_Q128_MODE2_1                    0x92
#define NIM_S3202_Q128_MODE2_2                    0x93

#define NIM_S3202_Q256_MODE0_1                    0x94
#define NIM_S3202_Q256_MODE0_2                    0x95
#define NIM_S3202_Q256_MODE1_1                    0x96
#define NIM_S3202_Q256_MODE1_2                    0x97
#define NIM_S3202_Q256_MODE2_1                    0x98
#define NIM_S3202_Q256_MODE2_2                    0x99
#define NIM_S3202_Q256_MODE3_1                    0x9a
#define NIM_S3202_Q256_MODE3_2                    0x9b

#define NIM_S3202_CR_THREAD_POWER0                0x9c
#define NIM_S3202_CR_THREAD_POWER1                0x9d

#define NIM_S3202_CR_LOCK_THRD_0                    0x9e
#define NIM_S3202_CR_LOCK_THRD_1                    0x9f
#define NIM_S3202_CR_LOCK_THRD_2                    0xa0
#define NIM_S3202_CR_LOCK_THRD_3                    0xa1
#define NIM_S3202_CR_LOCK_THRD_4                    0xa2
#define NIM_S3202_CR_LOCK_THRD_5                    0xa3
#define NIM_S3202_CR_LOCK_THRD_6                    0xa4
#define NIM_S3202_CR_LOCK_THRD_7                    0xa5
#define NIM_S3202_CR_LOCK_THRD_8                    0xa6
#define NIM_S3202_CR_LOCK_THRD_9                    0xa7
#define NIM_S3202_CR_LOCK_THRD_10                    0xa8
#define NIM_S3202_CR_LOCK_THRD_11                    0xa9
#define NIM_S3202_CR_LOCK_THRD_12                    0xaa
#define NIM_S3202_CR_LOCK_THRD_13                    0xab
#define NIM_S3202_CR_LOCK_THRD_14                    0xac
#define NIM_S3202_CR_LOCK_THRD_15                    0xad
#define NIM_S3202_CR_LOCK_THRD_16                    0xae
#define NIM_S3202_CR_LOCK_THRD_17                    0xaf
#define NIM_S3202_CR_LOCK_THRD_18                    0xb0
#define NIM_S3202_CR_LOCK_THRD_19                    0xb1
#define NIM_S3202_CR_LOCK_THRD_20                    0xb2
#define NIM_S3202_CR_LOCK_THRD_21                    0xb3
#define NIM_S3202_CR_LOCK_THRD_22                    0xb4

#define NIM_S3202_CR_LOCK_THRD_23                    0xb5
#define NIM_S3202_CR_LOCK_THRD_24                    0xb6
#define NIM_S3202_CR_LOCK_THRD_25                    0xb7
#define NIM_S3202_CR_LOCK_THRD_26                    0xb8
#define NIM_S3202_CR_LOCK_THRD_27                    0xb9
#define NIM_S3202_CR_LOCK_THRD_28                    0xba
#define NIM_S3202_CR_LOCK_THRD_29                    0xbb
#define NIM_S3202_CR_LOCK_THRD_30                    0xbc
#define NIM_S3202_CR_LOCK_THRD_31                    0xbd
#define NIM_S3202_CR_LOCK_THRD_32                    0xbe
#define NIM_S3202_CR_LOCK_THRD_33                    0xbf
#define NIM_S3202_CR_LOCK_THRD_34                    0xc0
#define NIM_S3202_CR_LOCK_THRD_35                    0xc1
#define NIM_S3202_CR_LOCK_THRD_36                    0xc2
#define NIM_S3202_CR_LOCK_THRD_37                    0xc3
#define NIM_S3202_CR_LOCK_THRD_38                    0xc4
#define NIM_S3202_CR_LOCK_THRD_39                    0xc5
#define NIM_S3202_CR_LOCK_THRD_40                    0xc6
#define NIM_S3202_CR_LOCK_THRD_41                    0xc7
#define NIM_S3202_CR_LOCK_THRD_42                    0xc8
#define NIM_S3202_CR_LOCK_THRD_43                    0xc9

#define NIM_S3202_CR_PRO_PATH_GAIN_0                0xca
#define NIM_S3202_CR_PRO_PATH_GAIN_1                0xCb
#define NIM_S3202_CR_PRO_PATH_GAIN_2                0xCc
#define NIM_S3202_CR_PRO_PATH_GAIN_3                0xCD
#define NIM_S3202_CR_PRO_PATH_GAIN_4                0xCE

#define NIM_S3202_CR_INT_PATH_GAIN_0                0xcf
#define NIM_S3202_CR_INT_PATH_GAIN_1                0xd0
#define NIM_S3202_CR_INT_PATH_GAIN_2                0xd1
#define NIM_S3202_CR_INT_PATH_GAIN_3                0xd2
#define NIM_S3202_CR_INT_PATH_GAIN_4                0xd3

#define NIM_S3202_CR_TIME_OUT_FOR_ACQ_0            0xd4
#define NIM_S3202_CR_TIME_OUT_FOR_ACQ_1            0xd5
#define NIM_S3202_CR_TIME_OUT_FOR_ACQ_2            0xd6
#define NIM_S3202_CR_TIME_OUT_FOR_ACQ_3            0xd7

#define NIM_S3202_CR_TIME_OUT_FOR_DRT_0            0xd8
#define NIM_S3202_CR_TIME_OUT_FOR_DRT_1            0xd9
#define NIM_S3202_CR_TIME_OUT_FOR_DRT_2            0xda
#define NIM_S3202_CR_TIME_OUT_FOR_DRT_3            0xdb

#define NIM_S3202_CR_WORK_MODE_CTL                0xdc

//magic add for zero IF config 20090224.
#define INI_DM_FREQ_OFFSET_0                        0xeb
#define INI_DM_FREQ_OFFSET_1                        0xec

#define NIM_S3202_QAM_ORDER_KNOWN        1
#define NIM_S3202_QAM_ORDER_UNKNOWN        0


#define SWITCH_NIM_S3202_DEBUG    0


#define NIM_S3281_ALL_LOCK                            0x2F
#define NIM_S3281_DSP_LOCK                            0x1F
#define NIM_S3281_FEC_LOCK                            0x0F










struct s3281_lock_info
{
    UINT32    frequency;
    UINT32    symbol_rate;
    UINT8     modulation;
};


INT32 nim_s3281_dvbc_read(UINT16 b_mem_adr, UINT8 *p_data,UINT8 b_len);
INT32 nim_s3281_dvbc_write(UINT16 b_mem_adr,UINT8 *p_data,UINT8 b_len);
INT32 system_reg_read(UINT32 bmemadr, UINT8 *pdata, UINT8 blen);
INT32 system_reg_write(UINT32 bmemadr, UINT8 *pdata, UINT8 blen);



INT32 nim_s3281_dvbc_channel_change(struct nim_device *dev,NIM_CHANNEL_CHANGE_T *pst_chl_change);


INT32 nim_s3281_dvbc_quick_channel_change(struct nim_device *dev,NIM_CHANNEL_CHANGE_T *pst_chl_change);
INT32 nim_s3281_dvbc_channel_search(struct nim_device *dev,UINT32 freq);

INT32 nim_s3281_dvbc_get_ber(struct nim_device *dev, UINT32 *err_count);
INT32 nim_s3281_dvbc_get_lock(struct nim_device *dev, UINT8 *lock);
INT32 nim_s3281_dvbc_get_freq(struct nim_device *dev, UINT32 *freq);
INT32 nim_s3281_dvbc_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate);
INT32 nim_s3281_dvbc_get_qam_order(struct nim_device *dev, UINT8 *qam_order);
INT32 nim_s3281_dvbc_get_agc(struct nim_device *dev,  UINT8 *agc);
INT32 nim_s3281_dvbc_get_snr(struct nim_device *dev, UINT8 *snr);
INT32 nim_s3281_dvbc_get_per(struct nim_device *dev,  UINT32 *rs_ubc);

INT32 nim_s3281_dvbc_set_perf_level(struct nim_device *dev,UINT32 level);
void nim_s3281_dvbc_task(UINT32 param1, UINT32 param2);
INT32 nim_s3281_dvbc_get_rf_level(struct nim_device *dev, UINT16 *rf_level);
INT32 nim_s3281_dvbc_get_cn_value(struct nim_device *dev, UINT16 *cnvalue);
INT32 nim_s3281_dvbc_set_tuner(struct nim_device *dev,NIM_CHANNEL_CHANGE_T *pst_chl_change);
INT32 nim_s3281_dvbc_set_mode(struct nim_device *dev,UINT8 tunerid_input,
                              struct DEMOD_CONFIG_ADVANCED qam_config);

INT32 nim_s3281_fsm_reset(struct nim_device *dev);
INT32 nim_s3281_hw_init(struct nim_device *dev);
INT32 nim_s3281_dvbc_get_fft_result(struct nim_device *dev, UINT32 freq,UINT32 *start_adr );



UINT32     log10times100_l( UINT32 x);
INT32     nim_s3281_dvbc_monitor_berper(struct nim_device *dev,BOOL *bervalid);
void    nim_set_pro_path_gain(UINT32 sym,UINT8 fec);
void    nim_set_rs_and_range(UINT32 sym);
void     nim_s3281_dvbc_set_search_freq(UINT16 freq_range);
BOOL    get_test_thread_off(void);
INT32   nim_s3281_dvbc_monitor_agc_status(struct nim_device *dev, UINT16 *cir);
INT32   nim_s3281_dvbc_monitor_agc0a_loop(struct nim_device *dev, UINT16 *agc0a_loop);
void 	nim_s3281_dvbc_set_delfreq(INT16 delfreq );
void    nim_s3281_task_proc(UINT32 param1, UINT32 param2);
INT32   nim_s3281_task_init(struct nim_device *dev);

extern struct nim_s3281_private *ali_m3281_nim_priv;        //comment
extern struct s3281_lock_info s3281_cur_channel_info;       //comment
extern UINT32 BER_COUNTS;                                   //comment
extern UINT32 PER_COUNTS;                                   //comment
extern BOOL   channel_change_en;                            //comment
extern BOOL   rf_agc_en;                                    //comment
extern UINT8  if_def_val2;                                  //for aci signal


extern void nimreg_ber_refresh (UINT32 err_count, UINT32 rsubc);     //comment 
extern void nim_reg_print(void);                                     //comment 
extern void display_dynamic_vision(void);                            //comment
extern BOOL nim_reg_func_flag(UINT8 indx, BOOL *val, BOOL dir);      //comment


#ifdef __cplusplus
}
#endif


#endif    /* __LLD_NIM_S3202_H__ */




