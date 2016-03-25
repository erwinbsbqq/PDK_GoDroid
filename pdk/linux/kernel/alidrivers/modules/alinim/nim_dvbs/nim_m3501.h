/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_m3501.h
*
*    Description:  m3501 header
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __LLD_NIM_M3501_H__
#define __LLD_NIM_M3501_H__

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


#ifdef __cplusplus
extern "C" {
#endif



//#define NIM_3501_FUNC_EXT

//------------------------DEFINE for Work Mode--------------------------------//
//#define HW_ADPT_CR // when print, need to open
//#define RPT_CR_SNR_EST // print OldAdpt EST_NOISE value
#define NIM_CAPTURE_SUPPORT
#define C3501C_ERRJ_LOCK
#define CHANNEL_CHANGE_ASYNC
#define C3501C_NEW_TSO
#define HW_ADPT_CR
#define NIM_TS_PORT_CAP
#define AUTOSCAN_FULL_SPECTRUM


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

#define VERSION_S3501    "V1.0"         //20131024

#if 0
#define PRINTK_INFO     nim_print
#else
#define PRINTK_INFO(...)
#endif

//#define AUTOSCAN_DEBUG

#if 0
#define NIM_PRINTF   nim_print
#else
#define NIM_PRINTF(...)
#endif



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



#define RET_CONTINUE    0xFF



#define EXT_QPSK_MODE_SPI         0
#define EXT_QPSK_MODE_SSI         1
#define NIM_CHIP_ID_M3501A        0x350100C0
#define NIM_CHIP_ID_M3501B        0x350100D0
#define NIM_CHIP_SUB_ID_S3501D    0x00
#define NIM_CHIP_SUB_ID_M3501B    0xC0
#define NIM_CHIP_SUB_ID_M3501C    0xCC
#define NIM_FREQ_RETURN_REAL      0
#define NIM_FREQ_RETURN_SET       1



#define NIM_TSO_STUS_UNLOCK       0x00
#define NIM_TSO_STUS_SETTING      0x01
#define NIM_TSO_STUS_LOCK         0x02


#define CRYSTAL_FREQ              13500                   //13.5

#define    FFT_BITWIDTH           10
#define    STATISTIC_LENGTH       2

#define FS_MAXNUM                 15000
#define TP_MAXNUM                 2000


#define TAB_SIZE                  14          // for SI, use macro instead of enum

//average length of data to determine threshold
//0:2;1:4;2:8

#define    MAX_CH_NUMBER          32//maximum number of channels that can be stored
#define s3501_loacl_freq          5150
#define s3501_debug_flag          0
#define QPSK_TUNER_FREQ_OFFSET    4
#define M3501_IQ_AD_SWAP          0x04
#define M3501_EXT_ADC             0x02
#define M3501_QPSK_FREQ_OFFSET    0x01
#define M3501_I2C_THROUGH         0x08
#define M3501_NEW_AGC1            0x20
#define M3501_POLAR_REVERT        0x10
#define M3501_1BIT_MODE           0x00
#define M3501_2BIT_MODE           0x40
#define M3501_4BIT_MODE           0x80
#define M3501_8BIT_MODE           0xc0
#define M3501_AGC_INVERT          0x100
//#define M3501_SPI_SSI_MODE      0x200     8bit mode(SPI) 1bit mode(SSI)
#define M3501_USE_188_MODE        0x400
#define M3501_DVBS_MODE           0x00
#define M3501_DVBS2_MODE          0x01
#define M3501_SIGNAL_DISPLAY_LIN  0x800
#define NIM_S3501_BASE_IO_ADR     0xB8003000



/* DiSEqC mode */
#define NIM_DISEQC_MODE_22KOFF           0    /* 22kHz off */
#define NIM_DISEQC_MODE_22KON            1    /* 22kHz on */
#define NIM_DISEQC_MODE_BURST0           2    /* Burst mode, on for 12.5mS = 0 */
#define NIM_DISEQC_MODE_BURST1           3    /* Burst mode, modulated 1:2 for 12.5mS = 1 */
#define NIM_DISEQC_MODE_BYTES            4    /* Modulated with bytes from DISEQC INSTR */
#define NIM_DISEQC_MODE_ENVELOP_ON       5    /* Envelop enable*/
#define NIM_DISEQC_MODE_ENVELOP_OFF      6    /* Envelop disable, out put 22K wave form*/
#define NIM_DISEQC_MODE_OTHERS           7    /* Undefined mode */
#define NIM_DISEQC_MODE_BYTES_EXT_STEP1  8    /*Split NIM_DISEQC_MODE_BYTES to 2 steps to improve the speed,*/
#define NIM_DISEQC_MODE_BYTES_EXT_STEP2  9    /*(30ms--->17ms) to fit some SPEC */

/* Polarization */
#define NIM_PORLAR_HORIZONTAL      0x00
#define NIM_PORLAR_VERTICAL        0x01
#define NIM_PORLAR_LEFT            0x02
#define NIM_PORLAR_RIGHT           0x03

#define NIM_PORLAR_REVERSE         0x01
#define NIM_PORLAR_SET_BY_22K      0x02

#define DISEQC2X_ERR_NO_REPLY      0x01
#define DISEQC2X_ERR_REPLY_PARITY  0x02
#define DISEQC2X_ERR_REPLY_UNKNOWN 0x03
#define DISEQC2X_ERR_REPLY_BUF_FUL 0x04


#define NIM_FREQ_RETURN_SET        1
#define NIM_FREQ_RETURN_REAL       0
#define NIM_TUNER_SET_STANDBY_CMD  0xffffffff




#define NIM_GET_DWORD(i)    (*(volatile UINT32 *)(i))
#define NIM_SET_DWORD(i,d)  (*(volatile UINT32 *)(i)) = (d)

#define NIM_GET_WORD(i)     (*(volatile UINT16 *)(i))
#define NIM_SET_WORD(i,d)   (*(volatile UINT16 *)(i)) = (d)

#define NIM_GET_BYTE(i)     (*(volatile UINT8 *)(i))
#define NIM_SET_BYTE(i,d)   (*(volatile UINT8 *)(i)) = (d)

#define S3501_FREQ_OFFSET         1
#define LNB_LOACL_FREQ            5150
#define AS_FREQ_MIN               900
#define AS_FREQ_MAX               2200

#define NIM_OPTR_CHL_CHANGE0      0x70
#define NIM_OPTR_CHL_CHANGE       0x00
#define NIM_OPTR_SOFT_SEARCH      0x01
#define NIM_OPTR_FFT_RESULT       0x02
#define NIM_OPTR_DYNAMIC_POW      0x03
#define NIM_OPTR_DYNAMIC_POW0     0x73
#define NIM_OPTR_IOCTL            0x04
#define NIM_OPTR_HW_OPEN          0x05
#define NIM_OPTR_HW_CLOSE         0x06

#define NIM_DEMOD_CTRL_0X50       0x50
#define NIM_DEMOD_CTRL_0X51       0x51
#define NIM_DEMOD_CTRL_0X90       0x90
#define NIM_DEMOD_CTRL_0X91       0x91
#define NIM_DEMOD_CTRL_0X02       0x02
#define NIM_DEMOD_CTRL_0X52       0x52

#define NIM_SIGNAL_INPUT_OPEN     0x01
#define NIM_SIGNAL_INPUT_CLOSE    0x02

#define NIM_LOCK_STUS_NORMAL      0x00
#define NIM_LOCK_STUS_SETTING     0x01
#define NIM_LOCK_STUS_CLEAR       0x02

#define NIM_FLAG_CHN_CHG_START    (1<<0)
#define NIM_FLAG_CHN_CHANGING     (1<<1)
#define NIM_SWITCH_TR_CR          0x01
#define NIM_SWITCH_RS             0x02
#define NIM_SWITCH_FC             0x04
#define NIM_SWITCH_HBCD           0x08

#define TS_DYM_HEAD0              0x47
#define TS_DYM_HEAD1              0x1f
#define TS_DYM_HEAD2              0xff
#define TS_DYM_HEAD3              0x10
#define TS_DYM_HEAD4              0x00
//------------------------ defines for CR adaptive ---------------------------------
#define SNR_TAB_SIZE              19 // for SI, use macro instead of enum
#define PSK8_TAB_SIZE             14
#define QPSK_TAB_SIZE             3
#define APSK16_TAB_SIZE           11


/* 3501 register define */
enum NIM3501_REGISTER_ADDRESS
{
    R00_CTRL = 0x00,              // NIM3501 control register
    R01_ADC = 0x01,               // ADC Configuration Register
    R02_IERR = 0x02,              // Interrupt Events Register
    R03_IMASK = 0x03,             // Interrupt Mask Register
    R04_STATUS = 0x04,            // Status Register
    R05_TIMEOUT_TRH = 0x05,       // HW Timeout Threshold Register(LSB)
    R07_AGC1_CTRL = 0x07,         // AGC1 reference value register
    R0A_AGC1_LCK_CMD = 0x0a,      // AGC1 lock command register
    R0E_ADPT_CR_CTRL = 0x0e,
    R10_DCC_CFG = 0x10,           // DCC Configure Register
    R11_DCC_OF_I = 0x11,          // DCC Offset I monitor Register
    R12_DCC_OF_Q = 0x12,          // DCC Offset Q monitor Register
    R13_IQ_BAL_CTRL = 0x13,       // IQ Balance Configure Register
    R15_FLT_ROMINDX = 0x15,       // Filter Bank Rom Index Register
    R16_AGC2_REFVAL = 0x16,       // AGC2 Reference Value Register
    R17_AGC2_CFG = 0x17,          // AGC2 configure register
    R18_TR_CTRL = 0x18,           // TR acquisition gain register
    R1B_TR_TIMEOUT_BAND = 0x1b,   // TR Time out band register
    R21_BEQ_CTRL = 0x21,          // BEQ Control REgister
    R22_BEQ_CMA_POW = 0x22,       // BEQ CMA power register
    R24_MATCH_FILTER = 0x24,      // Match Filter Register
    R25_BEQ_MASK = 0x25,          // BEQ Mask Register
    R26_TR_LD_LPF_OPT = 0x26,     // TR LD LPF Output register
    R28_PL_TIMEOUT_BND = 0x28,    // PL Time out Band REgister
    R2A_PL_BND_CTRL = 0x2a,       // PL Time Band Control
    R2E_PL_ANGLE_UPDATE = 0x2e,   // PL Angle Update High/Low limit register
    R30_AGC3_CTRL = 0x30,         // AGC3  Control Register
    R33_CR_CTRL = 0x33,           // CR DVB-S/DVBS-S2  CONTROL register
    R45_CR_LCK_DETECT = 0x45,     // CR lock detecter lpf monitor register
    R47_HBCD_TIMEOUT = 0x47,      // HBCD Time out band register
    R48_VITERBI_CTRL = 0x48,      // Viterbi module control register
    R54_VITERBI_FRAME_SYNC = 0x54,
    R57_LDPC_CTRL = 0x57,         // LDPC control register
    R5B_ACQ_WORK_MODE = 0x5b,     // Acquiescent work mode register
    R5C_ACQ_CARRIER = 0x5c,       // Acquiescent carrier control register
    R5F_ACQ_SYM_RATE = 0x5f,      // Acquiescent symbol rate register
    R62_FC_SEARCH = 0x62,         // FC Search Range Register
    R64_RS_SEARCH = 0x64,         // RS Search Range Register
    R66_TR_SEARCH = 0x66,         // TR Search Step register
    R67_VB_CR_RETRY = 0x67,       // VB&CR Maximum Retry Number Register
    R68_WORK_MODE = 0x68,         // Work Mode Report Register
    R69_RPT_CARRIER = 0x69,       // Report carrier register
    R6C_RPT_SYM_RATE = 0x6c,      // report symbol rate register
    R6F_FSM_STATE = 0x6f,         // FSM State Moniter Register
    R70_CAP_REG = 0x70,           // Capture Param register
    R74_PKT_STA_NUM = 0x74,       // Packet Statistic Number Register
    R76_BIT_ERR = 0x76,           // Bit Error Register
    R79_PKT_ERR = 0x79,           // Packet Error Register
    R7B_TEST_MUX = 0x7b,          // Test Mux Select REgister
    R7C_DISEQC_CTRL = 0x7c,       // DISEQC Control Register
    R86_DISEQC_RDATA = 0x86,      // Diseqc data for read
    R8E_DISEQC_TIME = 0x8e,       // Diseqc time register
    R90_DISEQC_CLK_RATIO = 0x90,  // Diseqc clock ratio register
    R97_S2_FEC_THR = 0x97,        // S2 FEC Threshold register
    R99_H8PSK_THR = 0x99,         // H8PSK CR Lock Detect threshold register
    R9C_DEMAP_BETA = 0x9c,        // Demap Beta register
    R9D_RPT_DEMAP_BETA = 0x9d,    // Report demap beta value / CR Table addr
    RA0_RXADC_REG = 0xa0,         // RXADC ANATST/POWER register
    RA3_CHIP_ID = 0xa3,           // Chip ID REgister
    RA5_VER_ID = 0xa5,            // version ID register
    RA7_I2C_ENHANCE = 0xa7,       // I2C Enhance Register
    RA8_M90_CLK_DCHAN = 0xa8,     // M90 clock delay chain register
    RA9_M180_CLK_DCHAN = 0xa9,    // M180 Clock delay chain register
    RAA_S2_FEC_ITER = 0xaa,       // S2 FEC iteration counter register
    RAB_CHIP_SUB_ID = 0xab,       // S2 FEC iteration counter register
    RAD_TSOUT_SYMB = 0xad,        // ts  out setting SYMB_PRD_FORM_REG
    RAF_TSOUT_PAD = 0xaf,         // TS out setting and pad driving register
    RB0_PLL_CONFIG = 0xb0,        // PLL configure REgister
    RB1_TSOUT_SMT = 0xb1,         // TS output Setting and Pad driving
    RB3_PIN_SHARE_CTRL = 0xb3,    // Pin Share Control register
    RB5_CR_PRS_TRA = 0xb5,        // CR DVB-S/S2 PRS in Tracking State
    RB6_H8PSK_CETA = 0xb6,        // H8PSK COS/SIN Ceta Value Register
    RB8_LOW_RS_CLIP = 0xb8,       // Low RS Clip Value REgister
    RBA_AGC1_REPORT = 0xba,       // AGC1 report register
    RBB_SNR_RPT1 = 0xbb,
    RBC_SNR_RPT2 = 0xbc,
    RBD_CAP_PRM = 0xbd,           // Capture Config/Block register
    RBF_S2_FEC_DBG = 0xbf,        // DVB-S2 FEC Debug REgister
    RC0_BIST_LDPC_REG = 0xc0,     // LDPC Average Iteration counter register
    RC1_DVBS2_FEC_LDPC = 0xc1,    // DVBS2 FEC LDPC Register
    RC8_BIST_TOLERATOR = 0xc8,    // 0xc0    Tolerator MBIST register
    RC9_CR_OUT_IO_RPT = 0xc9,     // Report CR OUT I Q
    // for s3501B
    RCB_I2C_CFG = 0xcb,           // I2C Slave Configure Register
    RCC_STRAP_PIN_CLOCK = 0xcc,   // strap pin and clock enable register
    RCD_I2C_CLK = 0xcd,           // I2C AND CLOCK ENABLE REGISTER
    RCE_TS_FMT_CLK = 0xce,        // TS Format and clock enable register
    RD0_DEMAP_NOISE_RPT = 0xd0,   // demap noise rtp register
    RD3_BER_REG = 0xd3,           // BER register
    RD6_LDPC_REG = 0xd6,          // LDPC register
    RD7_EQ_REG = 0xd7,            // EQ register
    RD8_TS_OUT_SETTING = 0xd8,    // TS output setting register
    RD9_TS_OUT_CFG = 0xd9,        // BYPASS register
    RDA_EQ_DBG = 0xda,            // EQ Debug Register
    RDC_EQ_DBG_TS_CFG = 0xdc,     // EQ debug and ts config register
    RDD_TS_OUT_DVBS = 0xdd,       // TS output dvbs mode setting
    RDF_TS_OUT_DVBS2 = 0xdf,      // TS output dvbs2 mode setting
    RE0_PPLL_CTRL = 0xe0,
    RF0_HW_TSO_CTRL = 0xf0,
    RF1_DSP_CLK_CTRL = 0xf1,
    RF8_MODCOD_RPT = 0xf8,
    RFA_RESET_CTRL = 0xfa,
    RFF_TSO_CLS = 0xff,
};


/***************************************************************
*    Structure of M3501 Autoscan tools
*
*
****************************************************************/
#ifdef NIM_S3501_ASCAN_TOOLS
#define VD_OSD_RPT_CNT            5         //max:5;  if update, please sync vs_s3501_ascan param
#define VD_REF_SATELLATE_CNT     10
#define VD_TP_CNT                100       //Don't change, or you know what
#define VD_TP_THR                3
enum
{
    ASCAN_ADD_REAL_TP = 0x00,
    ASCAN_ADD_LOST_TP,
    ASCAN_ADD_WAR_TP,
};
struct nim_s3501_tp
{
    UINT32 ref_freq;
    UINT32 print_freq;
    UINT32 freq;
    UINT32 symbolrate;
    UINT32 polarize;
    UINT32 lock_stat;
};
struct nim_s3501_ascan
{
    UINT32 va_ascan_g1_begin_time;
    UINT32 va_ascan_g1_real_begin_time;
    UINT32 va_ascan_g1_end_time;
    UINT32 va_ascan_g1_real_time_use;                //print time type: if 1:print real time(no win message)
    UINT32 va_ascan_g1_time[VD_OSD_RPT_CNT];
    UINT32 va_ascan_g2_tplist_index;
    UINT32 va_ascan_g3_real_tp_cnt[VD_OSD_RPT_CNT];  //scan real tps cnt for report to OSD
    UINT32 va_ascan_g4_open_log_print;               // 1:open driver print log
    UINT32 va_ascan_g5_lost_tp_cnt[VD_OSD_RPT_CNT];  //record lost tps each scan loop
    UINT32 va_ascan_g5_war_tp_cnt[VD_OSD_RPT_CNT];   //record war tps each scan loop
    UINT32 va_ascan_g6_scan_f_start;                 //scan freq rand begin
    UINT32 va_ascan_g6_scan_f_end;                   //scan freq range end
    UINT32 va_ascan_g7_scan_tp_only;                 // 1:not scan program
    UINT32 va_ascan_g8_loop_cnt;                     //all scan loop count
    UINT32 va_ascan_g8_loop_cur_idx;                 //LOOP cur index

};
//struct nim_s3501_tp va_g2_ascan_tps[VD_REF_SATELLATE_CNT][100];
//struct nim_s3501_tp va_g3_ascan_real_tps[100];
//struct nim_s3501_tp va_g3_ascan_lost_tps[100];
#endif

typedef struct _tag_phase_noise
{
    UINT8 work_mode;
    UINT8 map_type;
    UINT8 code_rate;
    UINT8 channel_change_flag;
    UINT32 rs;
}PHASE_NOISE_PARAM;

extern UINT8 m3501_debug_flag;

//----------------------------DEFINE for variable---------------------------------
//extern varible
extern INT32       fft_energy_1024[1024];
extern INT32       fft_energy_1024_tmp[1024];
extern INT32       frequency_est[TP_MAXNUM];
extern INT32       symbol_rate_est[TP_MAXNUM];
extern INT32       tp_number;
extern INT32       *channel_spectrum;
extern INT32       *channel_spectrum_tmp;
extern INT32       last_tuner_if;
extern INT32       chlspec_num;
extern INT32       called_num;
extern INT32       final_est_freq;
extern INT32       final_est_sym_rate;
extern INT32       max_fft_energy;
extern const UINT8 ssi_clock_tab[];
extern INT32       s3501_snr_initial_en;

#define nim_reg_read nim_s3501_read
#define nim_reg_write nim_s3501_write


INT32 nim_s3501_read(struct nim_device *dev, UINT8 b_mem_adr, UINT8 *p_data, UINT8 b_len);
INT32 nim_s3501_write(struct nim_device *dev, UINT8 b_mem_adr, UINT8 *p_data, UINT8 b_len);

INT32 nim_s3501_i2c_open(struct nim_device *dev);
INT32 nim_s3501_i2c_close(struct nim_device *dev);


INT32 nim_s3501_attach(struct QPSK_TUNER_CONFIG_API *ptr_qpsk_tuner);
void nim_s3501_task(UINT32 param1, UINT32 param2);
INT32 nim_s3501_hw_init(struct nim_device *dev);
INT32 nim_s3501_get_tune_freq(struct nim_device *dev, INT32 *freq);

INT32 nim_s3501_rs_search_range(struct nim_device *dev, UINT8 s_case, UINT32 rs);
INT32 nim_s3501_set_fc_search_range(struct nim_device *dev, UINT8 s_case, UINT32 rs);
INT32 nim_s3501_sym_config(struct nim_device *dev, UINT32 sym);


void  nim_s3501_set_rs(struct nim_device *dev, UINT32 rs);


INT32 nim_s3501_hw_check(struct nim_device *dev);
INT32 nim_m3501c_recover_moerrj (struct nim_device *dev);
INT32 nim_m3501c_invert_moerrj (struct nim_device *dev);

INT32 nim_s3501_demod_ctrl(struct nim_device *dev, UINT8 c_value);
INT32 nim_s3501_set_polar(struct nim_device *dev, UINT8 polar);
INT32 nim_s3501_set_12v(struct nim_device *dev, UINT8 flag);

INT32 nim_s3501_set_dsp_clk(struct nim_device *dev, UINT8 clk_sel);

INT32 nim_s3501_set_acq_workmode(struct nim_device *dev, UINT8 s_case);
INT32 nim_s3501_set_hw_timeout(struct nim_device *dev, UINT8 time_thr);

INT32 nim_s3501_dynamic_power(struct nim_device *dev, UINT8 snr);
INT32 nim_s3501_tr_cr_setting(struct nim_device *dev, UINT8 s_case);

void nim_s3501_after_reset_set_param(struct nim_device *dev);
INT32 nim_s3501_hbcd_timeout(struct nim_device *dev, UINT8 s_case);
INT32 nim_s3501_adc_setting(struct nim_device *dev);
INT32 nim_s3501_agc1_ctrl(struct nim_device *dev, UINT8 low_sym, UINT8 s_case);
INT32 nim_s3501_ldpc_setting(struct nim_device *dev, UINT8 s_case, UINT8 c_ldpc, UINT8 c_fec);


//-------function for digital signal processing------
INT32 nim_s3501_fft(struct nim_device *dev, UINT32 start_freq);
void  nim_s3501_fft_result_read(struct nim_device *dev);
INT32 nim_s3501_fft_find_channel(struct nim_device *dev, UINT32 *tune_freq);


// -------function  for tuner DiSEqC------------

INT32 nim_s3501_di_seq_c_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt);
INT32 nim_s3501_di_seq_c2x_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt, \
                                 UINT8 *rt_value, UINT8 *rt_cnt);

INT32 nim_set_ts_rs(struct nim_device *dev, UINT32 rs);

// -------function  for interrupt------------
void nim_s3501_clear_int(struct nim_device *dev);
INT32 nim_m3501c_get_int(struct nim_device *dev);
INT32 nim_s3501_interrupt_mask_clean(struct nim_device *dev);

// -------function for frequency offset------------
INT32 nim_s3501_freq_offset_set(struct nim_device *dev, UINT8 low_sym, UINT32 *s_freq);
INT32 nim_s3501_freq_offset_reset(struct nim_device *dev, UINT8 low_sym);
INT32 nim_s3501_freq_offset_reset1(struct nim_device *dev, UINT8 low_sym, INT32 delfreq);


INT32 nim_m3501_ts_on (struct nim_device *dev);
INT32 nim_m3501c_fec_ts_off (struct nim_device *dev);
INT32 nim_m3501c_fec_ts_on (struct nim_device *dev);
INT32 nim_m3501c_reset_tso (struct nim_device *dev);
INT32 nim_s3501_set_ts_mode(struct nim_device *dev, UINT8 work_mode, UINT8 map_type, UINT8 code_rate, \
                            UINT32 rs, UINT8 channel_change_flag);

INT32 nim_m3501c_open_dummy (struct nim_device *dev);
INT32 nim_m3501c_close_dummy (struct nim_device *dev);
INT32 nim_close_ts_dummy(struct nim_device *dev);
INT32 nim_m3501_ts_off (struct nim_device *dev);

// -------function for CR------------
INT32 nim_s3501_cr_setting(struct nim_device *dev, UINT8 s_case);

INT32 nim_s3501_set_ssi_clk(struct nim_device *dev, UINT8 bit_rate);
INT32 nim_s3501_open_ci_plus(struct nim_device *dev, UINT8 *ci_plus_flag);


//--------Function get demod register---------
INT32 nim_s3501_get_lock(struct nim_device *dev, UINT8 *lock);
INT32 nim_s3501_get_freq(struct nim_device *dev, UINT32 *freq);
INT32 nim_s3501_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate);
INT32 nim_s3501_reg_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate);
INT32 nim_s3501_get_code_rate(struct nim_device *dev, UINT8 *code_rate);

INT32 nim_s3501_get_agc(struct nim_device *dev, UINT8 *agc);
INT32 nim_s3501_get_snr(struct nim_device *dev, UINT8 *snr);
INT32 nim_s3501_get_ber(struct nim_device *dev, UINT32 *rs_ubc);

INT32 nim_s3501_get_per(struct nim_device *dev, UINT32 *rs_ubc);
INT32 nim_s3501_get_ldpc(struct nim_device *dev, UINT32 *rs_ubc);


INT32 nim_s3501_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32 *start_adr);
INT32 nim_s3501_get_type(struct nim_device *dev);
INT32 nim_s3501_get_dsp_clk(struct nim_device *dev, UINT32 *sample_rate);
UINT8 nim_s3501_get_snr_index(struct nim_device *dev);
INT32 nim_s3501_get_bit_rate(struct nim_device *dev, UINT8 work_mode, UINT8 map_type, UINT8 code_rate, \
                              UINT32 rs, UINT32 *bit_rate);

INT32 nim_s3501_get_new_ber(struct nim_device *dev, UINT32 *ber);
INT32 nim_s3501_get_new_per(struct nim_device *dev, UINT32 *per);

INT32 nim_s3501_set_phase_noise(struct nim_device *dev);
INT32 nim_s3501_get_phase_error(struct nim_device *dev, INT32 *phase_error);

INT32 nim_s3501_get_bypass_buffer(struct nim_device *dev);
UINT32 nim_s3501_get_curfreq(struct nim_device *dev);

UINT8 nim_s3501_get_crnum(struct nim_device *dev);
INT32 nim_s3501_get_bitmode(struct nim_device *dev, UINT8 *bit_mode);



INT32 nim_s3501_reg_get_freq(struct nim_device *dev, UINT32 *freq);

INT32 nim_s3501_reg_get_code_rate(struct nim_device *dev, UINT8 *code_rate);
INT32 nim_s3501_reg_get_map_type(struct nim_device *dev, UINT8 *map_type);
INT32 nim_s3501_reg_get_work_mode(struct nim_device *dev, UINT8 *work_mode);
INT32 nim_s3501_reg_get_iqswap_flag(struct nim_device *dev, UINT8 *iqswap_flag);
INT32 nim_s3501_reg_get_roll_off(struct nim_device *dev, UINT8 *roll_off);
INT32 nim_s3501_reg_get_modcod(struct nim_device *dev, UINT8 *modcod);


INT32 nim_s3501_autoscan_signal_input(struct nim_device *dev, UINT8 s_case);



INT32 nim_s3501_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);
INT32 nim_s3501_channel_search(struct nim_device *dev, UINT32 crnum);

#ifdef NIM_CAPTURE_SUPPORT
INT32 nim_s3501_cap_start(struct nim_device *dev, UINT32 start_freq, UINT32 sym, UINT32 *cap_buffer);
INT32 nim_s3501_cap(struct nim_device *dev, UINT32 start_freq, UINT32 *cap_buffer, UINT32 sym);
#endif


void  nim_s3501_calculate_energy(struct nim_device *dev);



INT32 nim_s3501_wideband_scan_open(struct nim_device *dev, UINT32 start_freq, UINT32 end_freq, UINT32 step_freq);

INT32 nim_s3501_wideband_scan_close();
INT32 nim_s3501_wideband_scan(struct nim_device *dev, UINT32 start_freq, UINT32 end_freq);


void  nim_s3501_set_fft_para(struct nim_device *dev);
INT32 nim_s3501_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param);
INT32 nim_s3501_open(struct nim_device *dev);
INT32 nim_change_ts_gap(struct nim_device *dev, UINT8 gap);
INT32 nim_s3501_ext_lnb_config(struct nim_device *dev, struct QPSK_TUNER_CONFIG_API *ptr_qpsk_tuner);
INT32 nim_s3501_tuner_lock(struct nim_device *dev, UINT8 *tun_lock);
INT32 nim_s3501_reg_get_freqoffset(struct nim_device *dev);

INT32 nim_s3501_autoscan(struct nim_device *dev, NIM_AUTO_SCAN_T *pst_auto_scan);


void   nim_hw_adaptive_cr_monitor(struct nim_device *dev);
INT32  nim_cr_tab_init(struct nim_device *dev);
INT32  nim_sw_snr_rpt(struct nim_device *dev);
INT32  nim_sw_adaptive_cr(struct nim_device *dev);
void   nim_s3501_set_freq_offset(struct nim_device *dev, INT32 delfreq);
UINT32 nim_s3501_get_tdata(UINT8 data);
void   nim_s3501_set_snr_status(struct nim_device *dev,UINT8 lock,UINT32 sym_rate);
INT32  nim_s3501_waiting_channel_lock(struct nim_device *dev, UINT32 freq, UINT32 sym);;

UINT32 nim_s3501_set_lock_times(UINT32 sym);



extern void R2FFT(INT32 *FFT_I_1024, INT32 *FFT_Q_1024);
extern INT32 nim_s3501_autosearch(INT32 *success, INT32 *delta_fc_est, INT32 *symbol_rate_est, \
                                    INT32 *m_if_freq, INT32 *m_ch_number);
extern void  nim_s3501_smoothfilter(void);
extern void  nim_s3501_median_filter(INT32 flength, INT32 *fdata, INT32 scan_mode);

extern INT32 nim_s3501_search_tp(INT32 chlspec_num,INT32 *channel_spectrum,UINT32 sfreq,\
                                   UINT32 adc_sample_freq,INT32 loop);

extern INT32 nim_s3501_fft_wideband_scan(UINT32 tuner_if, UINT32 adc_sample_freq);





#ifdef __cplusplus
}
#endif

#endif    // __LLD_NIM_S3501_H__ */


