#ifndef __LLD_NIM_DIB8000_H__
#define __LLD_NIM_DIB8000_H__

#include <adr_retcode.h>
#include <hld/nim/adr_nim_dev.h>

struct nim_dib8000_private
{
	struct COFDM_TUNER_CONFIG_API Tuner_Control;
};

#define SYS_COFDM_DIB8000_CHIP_ADRRESS  0x12


/* ALi scan info (common) */
struct dib8000_Lock_Info
{
	UINT8  FECRates;
	UINT8  HPRates;
	UINT8  LPRates;
	UINT8  Modulation;
	UINT8  Mode;
	UINT8  Guard;
	UINT8  Force;
	UINT8  Hierarchy;
	UINT8  Priority;
	UINT8  Spectrum;
	UINT8  ChannelBW;
	UINT8  TrlTunning;
	UINT32 Frequency;
	INT8   EchoPos;
	INT32  FreqOffset;
	UINT8  Dis_TS_Output;

	//CC_tracking
	UINT8  CC_Tracking_Scan_Type;  
	UINT8  CC_Tracking_FECRates;
	UINT8  CC_Tracking_HPRates;
	UINT8  CC_Tracking_LPRates;
	UINT8  CC_Tracking_Modulation;
	UINT8  CC_Tracking_Mode;
	UINT8  CC_Tracking_Guard;
	UINT8  CC_Tracking_Hierarchy;
	UINT8  CC_Tracking_Spectrum;
	UINT8  CC_Tracking_ChannelBW;
	UINT32 CC_Tracking_Frequency;
	UINT8  CC_Tracking_flag;
	UINT8  CC_Tracking_staus;
	UINT8  Lock_Val;
	UINT16 AGC_Val ;  
	UINT8  SNR_Val;
	UINT32 BER_Val;
	UINT32 PER_Val;    
	UINT8  BER_HB;
	UINT8  BER_LB;
	UINT8  PER_HB;
	UINT8  PER_LB;
};

/* type of modulation (common) */
typedef enum DIBTUNER_Modulation_e
{
	DIBTUNER_MOD_NONE   = 0x00,	 /* Modulation unknown */
	DIBTUNER_MOD_ALL    = 0x1FF, /* Logical OR of all MODs */
	DIBTUNER_MOD_QPSK   = 0,//1,
	DIBTUNER_MOD_8PSK   = (1 << 1),
	DIBTUNER_MOD_QAM    = (1 << 2),
	DIBTUNER_MOD_16QAM  = 1,//(1 << 3),
	DIBTUNER_MOD_32QAM  = 2,//(1 << 4),
	DIBTUNER_MOD_64QAM  = 3,//(1 << 5),
	DIBTUNER_MOD_128QAM = (1 << 6),
	DIBTUNER_MOD_256QAM = (1 << 7),
	DIBTUNER_MOD_BPSK   = (1 << 8)
} DIBTUNER_Modulation_t;

/* mode of OFDM signal */
typedef enum DIBTUNER_Mode_e
{
	DIBTUNER_MODE_2K,
	DIBTUNER_MODE_8K,
	DIBTUNER_MODE_4K
} DIBTUNER_Mode_t;

/* guard of OFDM signal */
typedef enum DIBTUNER_Guard_e
{
	DIBTUNER_GUARD_1_32,			   /* Guard interval = 1/32 */
	DIBTUNER_GUARD_1_16,			   /* Guard interval = 1/16 */
	DIBTUNER_GUARD_1_8,				   /* Guard interval = 1/8  */
	DIBTUNER_GUARD_1_4				   /* Guard interval = 1/4  */
} DIBTUNER_Guard_t;

typedef enum DIBTUNER_Spectrum_e
{
	DIBTUNER_INVERSION_NONE = 0,
	DIBTUNER_INVERSION      = 1,
	DIBTUNER_INVERSION_AUTO = 2,
	DIBTUNER_INVERSION_UNK  = 4
} DIBTUNER_Spectrum_t;

typedef enum DIBTUNER_Force_e
{
	DIBTUNER_FORCENONE  = 0,
	DIBTUNER_FORCE_M_G  = 1
} DIBTUNER_Force_t;

typedef enum DIBTUNER_ChannelBW_e
{
	DIBTUNER_CHAN_BW_6M  = 6,
	DIBTUNER_CHAN_BW_7M  = 7,
	DIBTUNER_CHAN_BW_8M  = 8
} DIBTUNER_ChannelBW_t;

typedef enum DIBTUNER_FECRate_e
{
	DIBTUNER_FEC_NONE = 0x00,	 /* no FEC rate specified */
	DIBTUNER_FEC_ALL = 0xFF,	 /* Logical OR of all FECs */
	DIBTUNER_FEC_1_2 = 1,
	DIBTUNER_FEC_2_3 = 2,//(1 << 1),
	DIBTUNER_FEC_3_4 = 3,//(1 << 2),
	DIBTUNER_FEC_4_5 = 4,//(1 << 3),
	DIBTUNER_FEC_5_6 = 5,//(1 << 4),
	DIBTUNER_FEC_6_7 = 6,//(1 << 5),
	DIBTUNER_FEC_7_8 = 7,//(1 << 6),
	DIBTUNER_FEC_8_9 = 8 //(1 << 7)
} DIBTUNER_FECRate_t;

enum signal_flag
{
	SIGNAL_CHECK_NORMAL,	
	SIGNAL_CHECK_PAUSE,
	SIGNAL_CHECK_RESET,
};

//ALi-defined values ==> 需改用上面的ST-defined enum values, 並改寫相關API parameter and return value!! --- [begin]
#define FEC_1_2   0
#define FEC_2_3   1
#define FEC_3_4   2
#define FEC_5_6   3
#define FEC_7_8   4

#define guard_1_32  0x20
#define guard_1_16  0x10
#define guard_1_8   0x08
#define guard_1_4   0x04

#define MODE_2K     0x02
#define MODE_8K     0x08

#define FFT_MODE_2K 0x0
#define FFT_MODE_8K 0x1

#define TPS_CONST_QPSK  0x04 
#define TPS_CONST_16QAM 0x10
#define TPS_CONST_64QAM 0x40

#define HIER_NONE  1
#define HIER_1     1
#define HIER_2     2
#define HIER_4     4
//ALi-defined values ==> 需改用上面的ST-defined enum values, 並改寫相關API parameter and return value!! --- [begin]

//privat
INT8 f_dib8000_read(UINT8 dev_addr, UINT16 reg_addr, UINT16 *data, UINT8 len);
INT8 f_dib8000_write(UINT8 dev_addr, UINT16 reg_addr, UINT16 data, UINT8 len);
static INT32 f_dib8000_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param);
static INT32 f_dib8000_get_lock_status(struct nim_device *dev, UINT8 *lock);
static INT32 f_dib8000_channel_search(struct nim_device *dev, UINT32 freq, UINT32 bandwidth, 
									   UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT16 freq_offset, UINT8 priority);
static INT32 f_dib8000_open(struct nim_device *dev);

//public
INT32 f_dib8000_attach(struct COFDM_TUNER_CONFIG_API *ptrCOFDM_Tuner);
int f_dib8000_hw_init(struct nim_device *dev);
static INT32 f_dib8000_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth, 
									   UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT8 priority);

INT32 nim_dib8000_ioctl_ext(struct nim_device *dev, INT32 cmd, void *param_list);
static INT32 f_dib8000_get_BER(struct nim_device *dev, UINT32 *vbber);

#endif  /* __LLD_NIM_DIB8000_H__ */
