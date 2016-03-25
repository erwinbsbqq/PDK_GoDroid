/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_s3503.h
*
*    Description:  s3503 header
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#ifndef __LLD_NIM_S3503_H__
#define __LLD_NIM_S3503_H__


#if defined(__NIM_LINUX_PLATFORM__)
#include "../porting_linux_header.h"
#include "porting_m3501_linux.h"
#include "unified_bsp_board_attr.h"
//#include "nim_s3501_autosearch.h"
#elif defined(__NIM_TDS_PLATFORM__)
#include "porting_tds_header.h"
#include "porting_m3501_tds.h"
//#include "..\m36\nim_s3501_autosearch.h"
#endif



//#include "Porting.h"
//#include "nim_dev.h"
//#define NIM_3501_FUNC_EXT

//------------------------DEFINE for Work Mode--------------------------------//
//#define HW_ADPT_CR // when print, need to open
//#define RPT_CR_SNR_EST // print OldAdpt EST_NOISE value
#define NIM_CAPTURE_SUPPORT
#define ADC2DRAM_SUPPORT
#define CHANNEL_CHANGE_ASYNC





// #define AUTOSCAN_DEBUG            // print log about AutoScan
// #define DEBUG_SOFT_SEARCH      // Try someone TP for special case, for DEbug
//#define DISEQC_OUT_INVERT       //DISEQC for Special Case

//----------------------End of DEFINE for Work Mode-----------------------------//

//----------------------------DEFINE for debug---------------------------------//
//-----defines for CR debug-------
//#define HW_ADPT_CR_MONITOR // print OldAdpt parameters

//#define HW_ADPT_NEW_CR_MONITOR // print CR coefficients, no matter OldAdpt or NewAdpt
//#define NEW_CR_ADPT_SNR_EST_RPT // print c/n dB value
//#define LLR_SHIFT_DEBUG // enable all LLR_SHIFT if 113[0]=1
//#define SW_ADPT_CR
//#define SW_SNR_RPT_ONLY

#define VERSION_S3503    "V1.0"         //20131024

#if 0
#define PRINTK_INFO 	nim_print
#else
#define PRINTK_INFO(...)
#endif


#if 0
#define NIM_PRINTF   nim_print

#else
#define NIM_PRINTF(...)
#endif

//#define AUTOSCAN_DEBUG


#ifdef HW_ADPT_CR_MONITOR
#define ADPT_CR_PRINTF  nim_print
#else
#define ADPT_CR_PRINTF(...)
#endif

#ifdef HW_ADPT_NEW_CR_MONITOR
#define ADPT_NEW_CR_PRINTF  nim_print
#else
#define ADPT_NEW_CR_PRINTF(...)
#endif


#ifdef AUTOSCAN_DEBUG
#define AUTOSCAN_PRINTF  nim_print

#else
#define AUTOSCAN_PRINTF(...)
#endif
#define BYPASS_BUF_SIZE_DMA 0x10000  //64K bytes, size of ADC2MEM block in unit BYTE


#define RET_CONTINUE    0xFF

#define NIM_CHIP_ID_M3503			0x350100C0

#define	FFT_BITWIDTH				10
#define	STATISTIC_LENGTH			2

#define FS_MAXNUM 					15000
#define TP_MAXNUM 					2000

//average length of data to determine threshold
//0:2;1:4;2:8

#define NIM_S3503_BASE_ADDR  		0xB8028000
#define NIM_S3503_SUB_ID  			0x0
#define NIM_C3503_SUB_ID 	 		0x1

#define	MAX_CH_NUMBER				32//maximum number of channels that can be stored
#define s3501_loacl_freq   			5150
#define s3501_debug_flag			0
#define QPSK_TUNER_FREQ_OFFSET		4
#define M3501_IQ_AD_SWAP   			0x04
#define M3501_EXT_ADC   	   		0x02
#define M3501_QPSK_FREQ_OFFSET 		0x01
#define M3501_I2C_THROUGH   		0x08
#define M3501_NEW_AGC1  			0x20
#define M3501_POLAR_REVERT  		0x10
#define M3501_1BIT_MODE 			0x00
#define M3501_2BIT_MODE 			0x40
#define M3501_4BIT_MODE 			0x80
#define M3501_8BIT_MODE 			0xc0
#define M3501_AGC_INVERT			0x100
//#define M3501_SPI_SSI_MODE		0x200     8bit mode(SPI) 1bit mode(SSI)
#define M3501_USE_188_MODE			0x400
#define M3501_DVBS_MODE				0x00
#define M3501_DVBS2_MODE			0x01
#define M3501_SIGNAL_DISPLAY_LIN	0x800


/* DiSEqC mode */
#define NIM_DISEQC_MODE_22KOFF		0	/* 22kHz off */
#define	NIM_DISEQC_MODE_22KON		1	/* 22kHz on */
#define	NIM_DISEQC_MODE_BURST0		2	/* Burst mode, on for 12.5mS = 0 */
#define	NIM_DISEQC_MODE_BURST1		3	/* Burst mode, modulated 1:2 for 12.5mS = 1 */
#define	NIM_DISEQC_MODE_BYTES		4	/* Modulated with bytes from DISEQC INSTR */
#define	NIM_DISEQC_MODE_ENVELOP_ON	5	/* Envelop enable*/
#define	NIM_DISEQC_MODE_ENVELOP_OFF	6	/* Envelop disable, out put 22K wave form*/
#define	NIM_DISEQC_MODE_OTHERS		7	/* Undefined mode */
#define	NIM_DISEQC_MODE_BYTES_EXT_STEP1		8	/*Split NIM_DISEQC_MODE_BYTES to 2 steps to improve the speed,*/
#define	NIM_DISEQC_MODE_BYTES_EXT_STEP2		9	/*(30ms--->17ms) to fit some SPEC */

/* Polarization */
#define NIM_PORLAR_HORIZONTAL	0x00
#define NIM_PORLAR_VERTICAL		0x01
#define NIM_PORLAR_LEFT			0x02
#define NIM_PORLAR_RIGHT		0x03

#define NIM_PORLAR_REVERSE		0x01
#define NIM_PORLAR_SET_BY_22K	0x02

#define DISEQC2X_ERR_NO_REPLY			0x01
#define DISEQC2X_ERR_REPLY_PARITY		0x02
#define DISEQC2X_ERR_REPLY_UNKNOWN	0x03
#define DISEQC2X_ERR_REPLY_BUF_FUL	0x04


#define NIM_FREQ_RETURN_SET			1
#define NIM_FREQ_RETURN_REAL		0
#define NIM_TUNER_SET_STANDBY_CMD	0xffffffff




#define NIM_GET_DWORD(i)	(*(volatile UINT32 *)(i))
#define NIM_SET_DWORD(i,d)  (*(volatile UINT32 *)(i)) = (d)

#define NIM_GET_WORD(i) 	(*(volatile UINT16 *)(i))
#define NIM_SET_WORD(i,d)   (*(volatile UINT16 *)(i)) = (d)

#define NIM_GET_BYTE(i) 	(*(volatile UINT8 *)(i))
#define NIM_SET_BYTE(i,d)   (*(volatile UINT8 *)(i)) = (d)

#define S3501_FREQ_OFFSET 			1
#define LNB_LOACL_FREQ 				5150
#define AS_FREQ_MIN 				900
#define AS_FREQ_MAX 				2200

#define NIM_OPTR_CHL_CHANGE0		0x70
#define NIM_OPTR_CHL_CHANGE			0x00
#define NIM_OPTR_SOFT_SEARCH		0x01
#define NIM_OPTR_FFT_RESULT			0x02
#define NIM_OPTR_DYNAMIC_POW		0x03
#define NIM_OPTR_DYNAMIC_POW0		0x73
#define NIM_OPTR_IOCTL				0x04
#define NIM_OPTR_HW_OPEN			0x05
#define NIM_OPTR_HW_CLOSE			0x06

#define NIM_DEMOD_CTRL_0X50			0x50
#define NIM_DEMOD_CTRL_0X51			0x51
#define NIM_DEMOD_CTRL_0X90			0x90
#define NIM_DEMOD_CTRL_0X91			0x91
#define NIM_DEMOD_CTRL_0X02			0x02
#define NIM_DEMOD_CTRL_0X52			0x52

#define NIM_SIGNAL_INPUT_OPEN		0x01
#define NIM_SIGNAL_INPUT_CLOSE		0x02

#define NIM_LOCK_STUS_NORMAL		0x00
#define NIM_LOCK_STUS_SETTING		0x01
#define NIM_LOCK_STUS_CLEAR			0x02

#define NIM_FLAG_CHN_CHG_START		(1<<0)
#define NIM_FLAG_CHN_CHANGING		(1<<1)
#define NIM_SWITCH_TR_CR			0x01
#define NIM_SWITCH_RS				0x02
#define NIM_SWITCH_FC				0x04
#define NIM_SWITCH_HBCD				0x08

#define TS_DYM_HEAD0 				0x47
#define TS_DYM_HEAD1 				0x1f
#define TS_DYM_HEAD2 				0xff
#define TS_DYM_HEAD3 				0x10
#define TS_DYM_HEAD4 				0x00
//------------------------ defines for CR adaptive ---------------------------------
#define SNR_TAB_SIZE 				19 // for SI, use macro instead of enum
#define PSK8_TAB_SIZE 				14
#define QPSK_TAB_SIZE 				3
#define APSK16_TAB_SIZE 			11


//----------------end of defines for CR adaptive----------------
/* 3503 register define for DVBS/DVBS2 */
enum NIM3501_REGISTER_ADDRESS
{
    R00_CTRL = 0x00,			// NIM3501 control register
    R01_ADC = 0x01,				// ADC Configuration Register
    R02_IERR = 0x02,			// Interrupt Events Register
    R03_IMASK = 0x03,			// Interrupt Mask Register
    R04_STATUS = 0x04,			// Status Register
    R05_TIMEOUT_TRH = 0x05,		// HW Timeout Threshold Register(LSB)
    R07_AGC1_CTRL = 0x07,		// AGC1 reference value register
    R0A_AGC1_LCK_CMD = 0x0a,	// AGC1 lock command register
    R0E_ADPT_CR_CTRL = 0x0e,
    R10_DCC_CFG = 0x10,			// DCC Configure Register
    R11_DCC_OF_I = 0x11,		// DCC Offset I monitor Register
    R12_DCC_OF_Q = 0x12,		// DCC Offset Q monitor Register
    R13_IQ_BAL_CTRL = 0x13,		// IQ Balance Configure Register
    R15_FLT_ROMINDX = 0x15,		// Filter Bank Rom Index Register
    R16_AGC2_REFVAL = 0x16,		// AGC2 Reference Value Register
    R17_AGC2_CFG = 0x17,		// AGC2 configure register
    R18_TR_CTRL = 0x18,			// TR acquisition gain register
    R1B_TR_TIMEOUT_BAND = 0x1b,	// TR Time out band register
    R21_BEQ_CTRL = 0x21,		// BEQ Control REgister
    R22_BEQ_CMA_POW = 0x22,		// BEQ CMA power register
    R24_MATCH_FILTER = 0x24,	// Match Filter Register
    R25_BEQ_MASK = 0x25,		// BEQ Mask Register
    R26_TR_LD_LPF_OPT = 0x26,	// TR LD LPF Output register
    R28_PL_TIMEOUT_BND = 0x28,	// PL Time out Band REgister
    R2A_PL_BND_CTRL = 0x2a,		// PL Time Band Control
    R2E_PL_ANGLE_UPDATE = 0x2e,	// PL Angle Update High/Low limit register
    R30_AGC3_CTRL = 0x30,		// AGC3  Control Register
    R33_CR_CTRL = 0x33,			// CR DVB-S/DVBS-S2  CONTROL register
    R45_CR_LCK_DETECT = 0x45,	// CR lock detecter lpf monitor register
    R47_HBCD_TIMEOUT = 0x47,	// HBCD Time out band register
    R48_VITERBI_CTRL = 0x48,	// Viterbi module control register
    R54_VITERBI_FRAME_SYNC = 0x54,
    R57_LDPC_CTRL = 0x57,		// LDPC control register
    R5B_ACQ_WORK_MODE = 0x5b,	// Acquiescent work mode register
    R5C_ACQ_CARRIER = 0x5c,		// Acquiescent carrier control register
    R5F_ACQ_SYM_RATE = 0x5f,	// Acquiescent symbol rate register
    R62_FC_SEARCH = 0x62,		// FC Search Range Register
    R64_RS_SEARCH = 0x64,		// RS Search Range Register
    R66_TR_SEARCH = 0x66,		// TR Search Step register
    R67_VB_CR_RETRY = 0x67,		// VB&CR Maximum Retry Number Register
    R68_WORK_MODE = 0x68,		// Work Mode Report Register
    R69_RPT_CARRIER = 0x69,		// Report carrier register
    R6C_RPT_SYM_RATE = 0x6c,	// report symbol rate register
    R6F_FSM_STATE = 0x6f,		// FSM State Moniter Register
    R70_CAP_REG = 0x70,			// Capture Param register
    R74_PKT_STA_NUM = 0x74,		// Packet Statistic Number Register
    R76_BIT_ERR = 0x76,			// Bit Error Register
    R79_PKT_ERR = 0x79,			// Packet Error Register
    R7B_TEST_MUX = 0x7b,		// Test Mux Select REgister
    R7C_DISEQC_CTRL = 0x7c,		// DISEQC Control Register
    R86_DISEQC_RDATA = 0x86,	// Diseqc data for read
    R8E_DISEQC_TIME = 0x8e,		// Diseqc time register
    R90_DISEQC_CLK_RATIO = 0x90,// Diseqc clock ratio register
    R97_S2_FEC_THR = 0x97,		// S2 FEC Threshold register
    R99_H8PSK_THR = 0x99,		// H8PSK CR Lock Detect threshold register
    R9C_DEMAP_BETA = 0x9c,		// Demap Beta register
    R9D_RPT_DEMAP_BETA = 0x9d, // Report demap beta value / CR Table addr
    RA0_RXADC_REG = 0xa0,		// RXADC ANATST/POWER register
    RA3_CHIP_ID = 0xa3,			// Chip ID REgister
    RA5_VER_ID = 0xa5,			// version ID register
    RA6_VER_SUB_ID = 0xa6,			// version sub ID register
    RA7_I2C_ENHANCE = 0xa7,		// I2C Enhance Register
    RA8_M90_CLK_DCHAN = 0xa8,	// M90 clock delay chain register
    RA9_M180_CLK_DCHAN = 0xa9,	// M180 Clock delay chain register
    RAA_S2_FEC_ITER = 0xaa,		// S2 FEC iteration counter register
    RAB_CHIP_SUB_ID = 0xab,		// S2 FEC iteration counter register
    RAD_TSOUT_SYMB = 0xad,		// ts  out setting SYMB_PRD_FORM_REG
    RAF_TSOUT_PAD = 0xaf,		// TS out setting and pad driving register
    RB0_PLL_CONFIG = 0xb0,		// PLL configure REgister
    RB1_TSOUT_SMT = 0xb1,		// TS output Setting and Pad driving
    RB3_PIN_SHARE_CTRL = 0xb3,	// Pin Share Control register
    RB5_CR_PRS_TRA = 0xb5,		// CR DVB-S/S2 PRS in Tracking State
    RB6_H8PSK_CETA = 0xb6,		// H8PSK COS/SIN Ceta Value Register
    RB8_LOW_RS_CLIP = 0xb8,		// Low RS Clip Value REgister
    RBA_AGC1_REPORT = 0xba,		// AGC1 report register
    RBB_SNR_RPT1 = 0xbb,
    RBC_SNR_RPT2 = 0xbc,
    RBD_CAP_PRM = 0xbd,			// Capture Config/Block register
    RBF_S2_FEC_DBG = 0xbf,		// DVB-S2 FEC Debug REgister
    RC0_BIST_LDPC_REG = 0xc0,	// LDPC Average Iteration counter register
    RC1_DVBS2_FEC_LDPC = 0xc1,	// DVBS2 FEC LDPC Register
    RC8_BIST_TOLERATOR = 0xc8,	// 0xc0	Tolerator MBIST register
    RC9_CR_OUT_IO_RPT = 0xc9,	// Report CR OUT I Q
    // for s3501B
    RCB_I2C_CFG = 0xcb,			// I2C Slave Configure Register
    RCC_STRAP_PIN_CLOCK = 0xcc,	// strap pin and clock enable register
    RCD_I2C_CLK = 0xcd,			// I2C AND CLOCK ENABLE REGISTER
    RCE_TS_FMT_CLK = 0xce,		// TS Format and clock enable register
    RD0_DEMAP_NOISE_RPT = 0xd0,	// demap noise rtp register
    RD3_BER_REG = 0xd3,			// BER register
    RD6_LDPC_REG = 0xd6,		// LDPC register
    RD7_EQ_REG = 0xd7,			// EQ register
    RD8_TS_OUT_SETTING = 0xd8,	// TS output setting register
    RD9_TS_OUT_CFG = 0xd9,		// BYPASS register
    RDA_EQ_DBG = 0xda,			// EQ Debug Register
    RDC_EQ_DBG_TS_CFG = 0xdc,	// EQ debug and ts config register
    RDD_TS_OUT_DVBS = 0xdd,		// TS output dvbs mode setting
    RDF_TS_OUT_DVBS2 = 0xdf,	// TS output dvbs2 mode setting
    RE0_PPLL_CTRL = 0xe0,
    RF0_HW_TSO_CTRL = 0xf0,
    RF1_DSP_CLK_CTRL = 0xf1,
    RF8_MODCOD_RPT = 0xf8,
    RFA_RESET_CTRL = 0xfa,
    RFF_TSO_CLS = 0xff,
    R113_NEW_CR_ADPT_CTRL = 0x113,
    R114_DISEQC_TIME_SET = 0x114,
    R11C_MAP_IN_I = 0x11c,
    R124_HEAD_DIFF_NFRAME = 0x124,
    r12d_ldpc_sel = 0x12d,
    R130_CR_PARA_DIN = 0x130,
    r13b_est_noise = 0x13b,
    r13d_adpt_cr_para_0 = 0x13d,
    R140_ADPT_CR_PARA_1 = 0x140,
    R144_ADPT_CR_PARA_2 = 0x144,
};



//----------------------------DEFINE for variable---------------------------------//
//extern varible
extern INT32 	fft_energy_1024[1024];
extern INT32 	fft_energy_1024_tmp[1024];
extern INT32 	frequency_est[TP_MAXNUM];
extern INT32 	symbol_rate_est[TP_MAXNUM];
extern INT32 	tp_number;
extern INT32 	*channel_spectrum;
extern INT32 	*channel_spectrum_tmp;
extern INT32 	last_tuner_if;
extern INT32 	chlspec_num;
extern INT32 	called_num;
extern INT32	final_est_freq;
extern INT32 	final_est_sym_rate;
extern INT32 	max_fft_energy;
extern INT32 	s3503_snr_initial_en;
extern UINT8 	s3503_debug_flag;


#define nim_reg_read 	nim_s3503_read
#define nim_reg_write 	nim_s3503_write


INT32 nim_s3503_read(struct nim_device *dev, UINT16 b_mem_adr, UINT8 *p_data, UINT8 b_len);
INT32 nim_s3503_write(struct nim_device *dev, UINT16 b_mem_adr, UINT8 *p_data, UINT8 b_len);

//INT32 nim_s3503_attach(struct nim_device *dev, struct QPSK_TUNER_CONFIG_API *ptrQPSK_Tuner);
INT32 nim_s3503_attach(struct QPSK_TUNER_CONFIG_API *ptr_qpsk_tuner);
void nim_s3503_task(UINT32 param1, UINT32 param2);
INT32 nim_s3503_hw_init(struct nim_device *dev);



INT32 nim_s3503_set_rs_search_range(struct nim_device *dev, UINT8 s_case, UINT32 rs);
INT32 nim_s3503_set_fc_search_range(struct nim_device *dev, UINT8 s_case, UINT32 rs);
INT32 nim_s3503_sym_config(struct nim_device *dev, UINT32 sym);

void  nim_s3503_cap_fft_result_read(struct nim_device *dev);
void  nim_s3503_set_rs(struct nim_device *dev, UINT32 rs);


INT32 nim_s3503_hw_check(struct nim_device *dev);

INT32 nim_s3503_set_demod_ctrl(struct nim_device *dev, UINT8 c_value);
INT32 nim_s3503_set_polar(struct nim_device *dev, UINT8 polar);
INT32 nim_s3503_set_12v(struct nim_device *dev, UINT8 flag);
INT32 nim_s3503_set_ext_lnb(struct nim_device *dev, struct QPSK_TUNER_CONFIG_API *ptr_qpsk_tuner);
INT32 nim_s3503_set_adc(struct nim_device *dev);
INT32 nim_s3503_set_dsp_clk(struct nim_device *dev, UINT8 clk_sel);
INT32 nim_s3503_set_agc1(struct nim_device *dev, UINT8 low_sym, UINT8 s_case);
INT32 nim_s3503_set_acq_workmode(struct nim_device *dev, UINT8 s_case);
INT32 nim_s3503_set_hw_timeout(struct nim_device *dev, UINT8 time_thr);
INT32 nim_s3503_set_hbcd_timeout(struct nim_device *dev, UINT8 s_case);
INT32 nim_s3503_set_dynamic_power(struct nim_device *dev, UINT8 snr);
INT32 nim_s3503_tr_cr_setting(struct nim_device *dev, UINT8 s_case);

void nim_s3503_after_reset_set_param(struct nim_device *dev);
INT32 nim_s3503_hbcd_timeout(struct nim_device *dev, UINT8 s_case);



//-------function for digital signal processing------
INT32 nim_s3503_fft(struct nim_device *dev, UINT32 start_freq);
void nim_s3503_fft_set_para(struct nim_device *dev);

// -------function  for tuner DiSEqC------------

INT32 nim_s3503_di_seq_c_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt);
INT32 nim_s3503_di_seq_c2x_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt,
                                 UINT8 *rt_value, UINT8 *rt_cnt);

// -------function  for interrupt------------
INT32 nim_s3503_interrupt_mask_clean(struct nim_device *dev);
void  nim_s3503_interrupt_clear(struct nim_device *dev);

// -------function  for tuner FEC------------
INT32 nim_s3503_fec_set_ldpc(struct nim_device *dev, UINT8 s_case, UINT8 c_ldpc, UINT8 c_fec);
void nim_s3503_fec_set_demap_noise(struct nim_device *dev);
INT32 nim_s3503_fec_llr_shift(struct nim_device *dev);

INT32 nim_s3503_cap_fft_find_channel(struct nim_device *dev, UINT32 *tune_freq);

// -------function for frequency offset------------
INT32 nim_s3503_freq_offset_set(struct nim_device *dev, UINT8 low_sym, UINT32 *s_freq);
INT32 nim_s3503_freq_offset_reset(struct nim_device *dev, UINT8 low_sym);
INT32 nim_s3503_freq_offset_reset1(struct nim_device *dev, UINT8 low_sym, INT32 delfreq);

// -------function for TR------------
INT32 nim_s3503_tr_setting(struct nim_device *dev, UINT8 s_case);

// -------function for CR------------
INT32 nim_s3503_cr_setting(struct nim_device *dev, UINT8 s_case);

INT32 nim_s3503_cr_sw_adaptive(struct nim_device *dev);
INT32 nim_s3503_cr_sw_snr_rpt(struct nim_device *dev);
INT32 nim_s3503_cr_set_phase_noise(struct nim_device *dev);
INT32 nim_s3503_cr_adaptive_configure (struct nim_device *dev, UINT32 sym);

#ifdef HW_ADPT_CR_MONITOR
void nim_s3503_cr_adaptive_monitor (struct nim_device *dev);
#endif

INT32 nim_s3503_cr_new_modcod_table_init(struct nim_device *dev, UINT32 sym);
INT32 nim_s3503_cr_new_adaptive_unlock_monitor(struct nim_device *dev);

INT32 nim_s3503_tso_off (struct nim_device *dev);
INT32 nim_s3503_tso_on (struct nim_device *dev);
INT32 nim_s3503_tso_dummy_on (struct nim_device *dev);
INT32 nim_s3503_tso_soft_cbr_off (struct nim_device *dev);

// -------function for ADC2DRAM------------


INT32 nim_s3503_adc2mem_start(struct nim_device *dev, UINT32 start_freq, UINT32 sym, UINT32 *cap_buffer,
                               UINT32 dram_len);
INT32 nim_s3503_adc2mem_entity(struct nim_device *dev,  UINT8 *cap_buffer, UINT32 dram_len);
INT32 nim_s3503_adc2mem_calculate_energy(struct nim_device *dev, UINT8 *cap_buffer, UINT32 dram_len);


//--------Function get demod register---------
INT32 nim_s3503_get_lock(struct nim_device *dev, UINT8 *lock);
INT32 nim_s3503_get_freq(struct nim_device *dev, UINT32 *freq);
INT32 nim_s3503_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate);
INT32 nim_s3503_get_code_rate(struct nim_device *dev, UINT8 *code_rate);

INT32 nim_s3503_get_agc(struct nim_device *dev, UINT8 *agc);
INT32 nim_s3503_get_snr(struct nim_device *dev, UINT8 *snr);
INT32 nim_s3503_get_ber(struct nim_device *dev, UINT32 *rs_ubc);



INT32 nim_s3503_get_per(struct nim_device *dev, UINT32 *rs_ubc);
INT32 nim_s3503_get_ldpc(struct nim_device *dev, UINT32 *rs_ubc);


INT32 nim_s3503_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32 *start_adr);
INT32 nim_s3503_get_type(struct nim_device *dev);
INT32 nim_s3503_get_dsp_clk(struct nim_device *dev, UINT32 *sample_rate);



INT32 nim_s3503_get_tune_freq(struct nim_device *dev, INT32 *freq);
UINT8 nim_s3503_get_snr_index(struct nim_device *dev);


INT32 nim_s3503_get_bit_rate(struct nim_device *dev, UINT8 work_mode, UINT8 map_type,
                              UINT8 code_rate, UINT32 rs, UINT32 *bit_rate);


INT32 nim_s3503_get_new_ber(struct nim_device *dev, UINT32 *ber);

INT32 nim_s3503_check_ber(struct nim_device *dev, UINT32 *rs_ubc);

INT32 nim_s3503_get_new_per(struct nim_device *dev, UINT32 *per);
INT32 nim_s3503_get_mer(struct nim_device *dev, UINT32 *mer);



INT32 nim_s3503_get_snr_db(struct nim_device *dev,UINT16 *snr_db);
INT32 nim_s3503_get_phase_error(struct nim_device *dev, INT32 *phase_error);

INT32 nim_s3503_get_bypass_buffer(struct nim_device *dev);
UINT32 nim_s3503_get_curfreq(struct nim_device *dev);



UINT8 nim_s3503_get_crnum(struct nim_device *dev);
INT32 nim_s3503_get_bitmode(struct nim_device *dev, UINT8 *bit_mode);



INT32 nim_s3503_reg_get_freq(struct nim_device *dev, UINT32 *freq);
INT32 nim_s3503_reg_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate);
INT32 nim_s3503_reg_get_code_rate(struct nim_device *dev, UINT8 *code_rate);
INT32 nim_s3503_reg_get_map_type(struct nim_device *dev, UINT8 *map_type);
INT32 nim_s3503_reg_get_work_mode(struct nim_device *dev, UINT8 *work_mode);
INT32 nim_s3503_reg_get_iqswap_flag(struct nim_device *dev, UINT8 *iqswap_flag);
INT32 nim_s3503_reg_get_roll_off(struct nim_device *dev, UINT8 *roll_off);
INT32 nim_s3503_reg_get_modcod(struct nim_device *dev, UINT8 *modcod);
INT32 nim_s3503_nframe_step_tso_setting(struct nim_device *dev, UINT32 *sym_rate, UINT8 s_case);

INT32 nim_s3501_autoscan_signal_input(struct nim_device *dev, UINT8 s_case);



INT32 nim_s3503_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);
INT32 nim_s3503_channel_search(struct nim_device *dev, UINT32 crnum);
INT32 nim_s3503_adc2mem_cap(struct nim_device *dev, UINT32 start_freq, UINT32 sym, UINT32 *cap_buffer, UINT32 dram_len);
void nim_s3503_cap_calculate_energy(struct nim_device *dev);

#ifdef NIM_CAPTURE_SUPPORT
INT32 nim_s3503_cap_start(struct nim_device *dev, UINT32 start_freq, UINT32 sym, UINT32 *cap_buffer);
INT32 nim_s3503_cap(struct nim_device *dev, UINT32 start_freq, UINT32 *cap_buffer, UINT32 sym);
#endif

INT32 nim_s3503_wideband_scan_open(struct nim_device *dev, UINT32 start_freq, UINT32 end_freq,
                                  UINT32 step_freq);
INT32 nim_s3503_wideband_scan_close();
INT32 nim_s3503_wideband_scan(struct nim_device *dev, UINT32 start_freq, UINT32 end_freq);



INT32 nim_s3503_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param);
INT32 nim_s3503_autoscan(struct nim_device *dev, NIM_AUTO_SCAN_T *pst_auto_scan);
INT32 nim_s3503_tuner_lock(struct nim_device *dev, UINT8 *tun_lock);


INT32 nim_s3503_cr_new_tab_init(struct nim_device *dev);
INT32     nim_s3503_waiting_channel_lock(struct nim_device *dev, UINT32 freq, UINT32 sym);
INT32 nim_s3503_autoscan_initial (struct nim_device *dev);
INT32 nim_s3503_debug_intask(struct nim_device *dev);
INT32 nim_s3503_set_err(struct nim_device *dev);
void nim_s3503_set_cr_new_value(struct nim_device *dev,UINT8 tabid,UINT8 cellid,UINT32 value);
INT32 nim_s3503_tso_initial (struct nim_device *dev, UINT8 insert_dummy, UINT8 tso_mode);
INT32 nim_s3503_di_seq_c_initial(struct nim_device *dev);
INT32 nim_s3503_cr_adaptive_initial (struct nim_device *dev);
INT32 nim_s3503_cr_adaptive_method_choice(struct nim_device *dev, UINT8 choice_type);
void nim_s3503_set_freq_offset(struct nim_device *dev, INT32 delfreq);
INT32 nim_s3503_tso_dummy_off (struct nim_device *dev);
INT32 nim_s3503_set_32apsk_target(struct nim_device *dev);
INT32 nim_s3503_cr_tab_init(struct nim_device *dev);

extern void R2FFT(INT32 *FFT_I_1024, INT32 *FFT_Q_1024);



//Nim Autosearch,R2FFT
extern INT32 nim_s3501_autosearch(INT32 *success, INT32 *delta_fc_est, INT32 *symbol_rate_est,
                                  INT32 *m_if_freq, INT32 *m_ch_number);

extern void    nim_s3501_smoothfilter(void);
extern  void   nim_s3501_median_filter(INT32 flength, INT32 *fdata, INT32 scan_mode);
extern INT32 nim_s3501_search_tp(INT32 chlspec_num,
                                INT32 *channel_spectrum,
                                UINT32 sfreq,
                                UINT32 adc_sample_freq,
                                INT32 loop);
extern INT32 nim_s3501_fft_wideband_scan(UINT32 tuner_if, UINT32 adc_sample_freq);




#endif    // __LLD_NIM_S3501_H__ */


