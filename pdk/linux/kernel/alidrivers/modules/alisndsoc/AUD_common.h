/*****************************************************************************

File Name :  AUD_common.h

Description: header file for Audio common

******************************************************************************/

/* Prevent recursive inclusion */
#ifndef __AUD_COMMON_H
#define __AUD_COMMON_H
#include <linux/delay.h>
#include <ali_reg.h>
#include <ali_interrupt.h>
/* Common unsigned types */

/* Boolean type (values should be among TRUE and FALSE constants only) */
#ifndef AUD_BOOL
#define AUD_BOOL       int
#endif

#ifndef AUD_BYTE
#define AUD_BYTE       unsigned char
#endif

#ifndef AUD_WORD
#define AUD_WORD       unsigned short int
#endif

#ifndef AUD_DWORD
#define AUD_DWORD      unsigned long
#endif

#ifndef AUD_UINT8
#define AUD_UINT8      unsigned char
#endif

#ifndef AUD_UINT16
#define AUD_UINT16     unsigned short int
#endif

#ifndef AUD_UINT32
#define AUD_UINT32     unsigned int
#endif

#ifndef AUD_INT8
#define AUD_INT8       signed char
#endif

#ifndef AUD_INT16
#define AUD_INT16      signed short
#endif

#ifndef AUD_INT32
#define AUD_INT32      signed int
#endif

#define NULL ((void *)0)

typedef volatile AUD_UINT8      SYS_UINT8;
typedef volatile AUD_UINT16     SYS_UINT16;
typedef volatile AUD_UINT32     SYS_UINT32;

#ifndef BOOL
//we recommand not using BOOL as return value, just use RET_CODE
typedef int				BOOL;
#endif

#ifndef FALSE
#define	FALSE			(0)
#endif
#ifndef	TRUE
#define	TRUE			(!FALSE)
#endif
#ifndef SDBBP
#define        SDBBP()     printk("Please check %s, line %d!!!\n", __FUNCTION__,__LINE__)
#endif
/* Define control Macro ----------------------------------------------------- */

#define AUD_ReadRegDev8(Address_p)  __REG8ALI(Address_p)


#define AUD_WriteRegDev8(Address_p, Value)                             \
{                                                                      \
    __REG8ALI(Address_p) = (AUD_UINT8) (Value);                \
}


#define AUD_ReadRegDev16(Address_p) __REG16ALI(Address_p)


#define AUD_WriteRegDev16(Address_p, Value)                            \
{                                                                      \
    __REG16ALI(Address_p)= (AUD_UINT16) (Value);              \
}


#define AUD_ReadRegDev32(Address_p) __REG32ALI(Address_p)


#define AUD_WriteRegDev32(Address_p, Value)                            \
{                                                                      \
    __REG32ALI(Address_p) = (AUD_UINT32) (Value);              \
}

/***********************************************************************
Audio Driver Device name
***********************************************************************/
#define HLD_MAX_NAME_SIZE    16  /* 15 characters plus '\0' */
typedef char               DeviceName_t[HLD_MAX_NAME_SIZE];


/* General purpose string type */
typedef char *           String_t;


/* Function return code */
typedef AUD_UINT32         RetCode_t;

/* Revision structure */
typedef const char *     AUD_Revision_t;

/* Audio device Handle */
typedef AUD_UINT32         AUD_Handle_t;

/* Audio device Type */
typedef AUD_UINT32         AUD_DeviceType_t;

/* Audio device ID */
typedef AUD_UINT16         AUD_DeviceID_t;

/* Audio evnet handle */
typedef AUD_UINT32         AUD_EventHandle_t;


typedef struct list_head   ListHead_t;


#ifndef PARTITION_T
#define PARTITION_T
typedef AUD_UINT32         Partition_t;
#endif

//AUD_INTERRUPT_NUMBER will define for interrupt number for audio device
#define AUD_INTERRUPT_NUMBER                      13





/* Common driver error constants */
#define DRIVER_ID          0
#define DRIVER_BASE        (DRIVER_ID << 16)

typedef enum AUD_RetCode_e
{
    NO_ERR = DRIVER_BASE,          /* 0  Funtions called success    */
    ERR_BAD_PARAMETER,             /* 1  Bad parameter passed       */
    ERR_NO_MEMORY,                 /* 2  Memory allocation failed   */
    ERR_UNKNOWN_DEVICE,            /* 3  Unknown device name        */
    ERR_ALREADY_INITIALIZED,       /* 4  Device already initialized */
    ERR_NO_FREE_HANDLES,           /* 5  Cannot open device again   */
    ERR_OPEN_HANDLE,               /* 6  At least one open handle   */
    ERR_INVALID_HANDLE,            /* 7  Handle is not valid        */
    ERR_FEATURE_NOT_SUPPORTED,     /* 8  Feature unavailable        */
    ERR_HARDWARE_CONFIG,           /* 9  Hardware config failed     */
    ERR_TIMEOUT,                   /* 10 Timeout occured            */
    ERR_DEVICE_BUSY,               /* 11 Device is currently busy   */
    ERR_TASK_CREATE,               /* 12 Task create failed         */
    ERR_SEM_CREATE                 /* 13 Semaphore create failed    */

}AUD_RetCode_t;

#if 0
enum 
{
  AUD_DEVICE_C3701 = 0,
  AUD_DEVICE_C3503,
  AUD_DEVICE_S3921,
  AUD_DEVICE_S3821
  
} AUD_DeviceID_e;

#endif

#define MAGIC_OPEN          19
#define MAGIC_CLOSE         75

/* Define type enum ------------------------------------------------- */

typedef enum I2SOutFormat_e
{
   I2S_FORMAT = 0,
   LEFT_JUSTIFIED,
   RIGHT_JUSTIFIED_24BIT,
   RIGHT_JUSTIFIED_16BIT
    
} I2S_OUT_FORMAT_t;

typedef enum BitClkMode_e
{
	BICK_LRCK_32 = 0,
	BICK_LRCK_48,
	BICK_LRCK_64,
	BICK_LRCK_128
	
} BIT_CLOCK_MODE_t;


typedef enum MasterClkMode_e
{
	M_CLK_X1 = 0,
	M_CLK_X2,
	M_CLK_X4,
	M_CLK_X8,
	M_CLK_X3,
	M_CLK_X6,
	M_CLK_X12,
	M_CLK_X24
	
} MASTER_CLOCK_MODE_t;


typedef enum FadeDirect_e
{
	FADE_IN = 0,
	FADE_OUT
	
} FADE_DIRECT_t;


typedef enum PostProcess_e
{	
	PP_CH_8 = 8,
	PP_BASS = 4,
	PP_EQ   = 2,
	PP_VS   = 1
	
} POST_PROCESS_t;


typedef enum DACFormat_e
{
    DAC_I2S,
    DAC_LEFT_JUSTIED,
    DAC_RIGHT_JUSTIED
    
} DAC_FORMAT_t;

typedef enum AUD_SubBlock_e
{
	AUD_SUB_PP           = 0x01,    // Audio post-process.
	AUD_SUB_IN           = 0x02,    // General audio input interface.
	AUD_SUB_OUT          = 0x04,    // General audio output interface.
	AUD_SUB_MIC0         = 0x08,	// Micro phone 0 input interface.
	AUD_SUB_MIC1         = 0x10,	// Micro phone 1 input interface.
	AUD_SUB_SPDIFIN      = 0x20,	// SPDIF input interface.
	AUD_SUB_SPDIFOUT     = 0x40,    // SPDIF output interface.
	AUD_SUB_SPDIFOUT_DDP = 0x80,
	AUD_SUB_ALL	         = 0xff		// All IO enabled.
	
} AUD_SubBlock_t;

typedef enum AUD_Interrupt_e
{
      AUD_I2SO_BUFF_RESUME_INT,
      AUD_I2SO_SAMPLE_COUNT_INT,
      AUD_I2SO_BUFF_UNDERRUN_INT,

      AUD_SPO_BUFF_RESUME_INT,
      AUD_SPO_SAMPLE_COUNT_INT,
      AUD_SPO_BUFF_UNDERRUN_INT,

      AUD_DDPSPO_BUFF_RESUME_INT,
      AUD_DDPSPO_SAMPLE_COUNT_INT,
      AUD_DDPSPO_BUFF_UNDERRUN_INT,
}AUD_Interrupt_t;

enum AudDupChannel
{
	AUD_DUP_NONE,
	AUD_DUP_L,
	AUD_DUP_R,
	AUD_DUP_MONO
};


enum AudOutSpdifType
{
	AUD_OUT_SPDIF_INVALID = -1, 
	AUD_OUT_SPDIF_PCM = 0,
	AUD_OUT_SPDIF_BS = 1,
	AUD_OUT_SPDIF_FORCE_DD = 2
};

typedef struct AUD_SPDIF_SCMS_s
{
	AUD_UINT8              Copyright:1;
	AUD_UINT8              Reserved7:7;
	AUD_UINT8              LeftBit:1;
	AUD_UINT8              CategoryCode:7;
	AUD_UINT16             Reserved16;

} AUD_SPDIF_SCMS_t;

/* Define type enum ------------------------------------------------- */
typedef enum AUD_Precision_e
{
    Precision_16_BITS = 16,
    Precision_24_BITS = 24
        
} AUD_Precision_t;


typedef enum AUD_ChanNum_e
{
    CHAN_NUM_1 = 1,
    CHAN_NUM_2 = 2,
    CHAN_NUM_4 = 4,
    CHAN_NUM_8 = 8
    
} AUD_ChanNum_t;


typedef enum AUD_Endian_Mode_e
{
    AUD_LITTLE_ENDIAN = 0,
    AUD_BIG_ENDIAN

} AUD_Endian_Mode_t;


typedef enum DAC_I2S_Source_e
{
    I2S_FROM_INTERNEL = 0,
    I2S_FROM_EXTERNAL
    
} DAC_I2S_Source_t;


typedef enum DAC_I2S_TriggerMode_e
{
    I2S_SCLK_NEGATIVE = 0,
    I2S_SCLK_POSITIVE
    
} DAC_I2S_TriggerMode_t;


typedef enum ADAC_OutputMode_e
{
    DIFFERENTIAL_OUTPUT = 0,
    SINGLE_OUTPUT
    
} ADAC_OutputMode_t;


typedef enum ADAC_OutputDC_Offset_e
{
    OFFSET_0MV = 0,
    OFFSET_N_12MV,
    OFFSET_N_12MV_1,
    OFFSET_N_24MV,
    OFFSET_N_24MV_1,
    OFFSET_N_36MV,
    OFFSET_N_36MV_1,
    OFFSET_N_48MV
    
} ADAC_OutputDC_Offset_t;


typedef enum DAC_I2S_InterfaceMode_e
{
    INTF_LEFT_JUSTIED = 0,
    INTF_RIGHT_JUSTIED,
    INTF_I2S
    
} DAC_I2S_InterfaceMode_t;


typedef enum DAC_I2S_WSEL_PolarInv_e
{
    START_FROM_LOW = 0,
    START_FROM_HIGH
    
} DAC_I2S_WSEL_PolarInv_t;


typedef enum ADAC_MuteMode_e
{
    ATTENUATION_0DB = 0,
    ATTENUATION_N_1_23DB,
    ATTENUATION_N_2_79DB,
    ATTENUATION_N_4_69DB,
    ATTENUATION_N_7_31DB,
    ATTENUATION_N_10_6DB,
    ATTENUATION_N_16_3DB,
    ATTENUATION_N_39_7DB
    
} ADAC_MuteMode_t;


typedef enum DAC_DIG_ChanMode_e
{
    CODEC_DIG_STEREO = 0,
    CODEC_DIG_MONO
    
} DAC_DIG_ChanMode_t;


typedef enum DAC_I2S_WSEL_Len_e
{
    I2S_WSEL_16_BITS = 0,
    I2S_WSEL_24_BITS,
    I2S_WSEL_32_BITS,
    I2S_WSEL_48_BITS,
    I2S_WSEL_64_BITS
    
} DAC_I2S_WSEL_Len_t;


typedef enum DAC_I2S_SDAT_Len_e
{
    SDAT_16_BITS = 0,
    SDAT_24_BITS
    
} DAC_I2S_SDAT_Len_t;


typedef enum SPO_ChanStatusReg_e
{
    LEFT_CHAN_STATUS_REG = 0,
    RIGHT_CHAN_STATUS_REG
    
} SPO_ChanStatusReg_t;


typedef enum I2SO_MCLK_Mode_e
{
    I2S_SOURCE_CLK_DIV_0 = 0,
    I2S_SOURCE_CLK_DIV_2,
    I2S_SOURCE_CLK_DIV_4,
    I2S_SOURCE_CLK_DIV_8
    
} I2SO_MCLK_Mode_t;


typedef enum I2SO_DRAM_BitNum_e
{
    BIT_NUM_8  = 0,
    BIT_NUM_16,
    BIT_NUM_24,
    BIT_NUM_32
    
} I2SO_DRAM_BitNum_t;


typedef enum SPO_DataSource_e
{
    FROM_I2SO_DMA = 0,
    FROM_SPO_DMA
    
} SPO_DataSource_t;


typedef enum SPO_MCLK_DivMode_e
{
    SPO_CLK_DIV_0 = 0,
    SPO_CLK_DIV_2,
    SPO_CLK_DIV_4,
    SPO_CLK_DIV_1_5
    
} SPO_MCLK_DivMode_t;


typedef enum DDP_SPO_OutputMode_e
{
    DDPSPO_OUT_CLK_DATA = 0,
    SPO_OUT_CLK_DATA,
    
} DDP_SPO_OutputMode_t;


typedef enum SPO_AudioContentFlag_e
{
    LINEAR_PCM = 0,
    NON_PCM,
    
} SPO_AudioContentFlag_t;


typedef enum SPO_DMA_DataLen_e
{
    DATA_LEN_16_BITS = 0,
    DATA_LEN_24_BITS
    
} SPO_DMA_DataLen_t;


typedef enum SPO_PCM_OutLFEMode_e
{
    SPO_R_I2SO_LFE = 0,
    SPO_L_I2SO_LFE
    
} SPO_PCM_OutLFEMode_t;


typedef enum SPO_PCM_OutChanMode_e
{
    I2SO_DL_DR = 0,
    I2SO_FL_FR,
    I2SO_SL_SR,
    I2SO_C_LEF
    
} SPO_PCM_OutChanMode_t;


typedef enum DDPSPO_MCLK_DivMode_e
{
    DDPSPO_MCLK_DIV_0 = 0,
    DDPSPO_MCLK_DIV_2,
    DDPSPO_MCLK_DIV_4,
    DDPSPO_MCLK_DIV_1_5
    
} DDPSPO_MCLK_DivMode_t;


typedef enum DDPSPO_DataSource_e
{
    DDP_FROM_I2SO_DMA = 0,
    DDP_FROM_DDPSPO_DMA
    
} DDPSPO_DataSource_t;


typedef enum DDPSPO_DMA_DataLen_e
{
    DDP_DATA_LEN_16_BITS = 0,
    DDP_DATA_LEN_24_BITS
    
} DDPSPO_DMA_DataLen_t;


typedef enum DDPSPO_ChanStatusReg_e
{
    DDP_LEFT_CHAN_STATUS_REG = 0,
    DDP_RIGHT_CHAN_STATUS_REG
    
} DDPSPO_ChanStatusReg_t;


typedef enum DDPSPO_PCM_OutLFEMode_e
{
    DDPSPO_R_I2SO_LFE = 0,
    DDPSPO_L_I2SO_LFE
    
} DDPSPO_PCM_OutLFEMode_t;


typedef enum DDPSPO_PCM_OutChanMode_e
{
    DDP_I2SO_DL_DR = 0,
    DDP_I2SO_FL_FR,
    DDP_I2SO_SL_SR,
    DDP_I2SO_C_LEF
    
} DDPSPO_PCM_OutChanMode_t;


typedef enum DDPSPO_AudioContentFlag_e
{
    DDP_LINEAR_PCM = 0,
    DDP_NON_PCM,
    
} DDPSPO_AudioContentFlag_t;


typedef enum I2SO_MCLK_DivMode_e
{
    I2SO_MCLK_DIV_1 = 0,
    I2SO_MCLK_DIV_2,
    I2SO_MCLK_DIV_4,
    I2SO_MCLK_DIV_8,
    I2SO_MCLK_DIV_3,
    I2SO_MCLK_DIV_6,
    I2SO_MCLK_DIV_12,
    I2SO_MCLK_DIV_24,
    I2SO_MCLK_DIV_16,
    I2SO_MCLK_DIV_32,
    I2SO_MCLK_DIV_64
    
} I2SO_MCLK_DivMode_t;


typedef enum I2SO_BCLK_DivMode_e
{
    I2SO_BCLK_DIV_1 = 0,
    I2SO_BCLK_DIV_2,
    I2SO_BCLK_DIV_4,
    I2SO_BCLK_DIV_8,
    I2SO_BCLK_DIV_16,
    I2SO_BCLK_DIV_32
    
} I2SO_BCLK_DivMode_t;


typedef enum I2SO_F128_DivMode_e
{
    I2SO_F128_DIV_3 = 0,
    I2SO_F128_DIV_6,
    I2SO_F128_DIV_12,
    I2SO_F128_DIV_24,
    I2SO_F128_DIV_48,
    I2SO_F128_DIV_96,
    I2SO_F128_DIV_192
    
} I2SO_F128_DivMode_t;


typedef enum AUD_SampleRate_e
{
    SAMPLE_RATE_8000    = 8000,
    SAMPLE_RATE_16000   = 16000,
    SAMPLE_RATE_32000   = 32000,
    SAMPLE_RATE_64000   = 64000,
    SAMPLE_RATE_128000  = 128000,
    SAMPLE_RATE_256000  = 256000,
    SAMPLE_RATE_512000  = 512000,
    
    SAMPLE_RATE_11025   = 11025,
    SAMPLE_RATE_22050   = 22050,
    SAMPLE_RATE_44100   = 44100,
    SAMPLE_RATE_88200   = 88200,
    SAMPLE_RATE_176400  = 176400,
    SAMPLE_RATE_352800  = 352800,
    SAMPLE_RATE_705600  = 705600,
    
    SAMPLE_RATE_12000   = 12000,
    SAMPLE_RATE_24000   = 24000,
    SAMPLE_RATE_48000   = 48000,
    SAMPLE_RATE_96000   = 96000,
    SAMPLE_RATE_192000  = 192000,
    SAMPLE_RATE_384000  = 384000,
    SAMPLE_RATE_768000  = 768000,

    SAMPLE_RATE_1024000 = 102400,
    SAMPLE_RATE_1536000 = 1536000,
    SAMPLE_RATE_1411200 = 1411200
    
} AUD_SampleRate_t;


typedef enum SPO_CLK_SW_DivMode_e
{
    SPO_CLK_SW_DIV_1 = 0,
    SPO_CLK_SW_DIV_2,
    SPO_CLK_SW_DIV_3,
    SPO_CLK_SW_DIV_6,
    SPO_CLK_SW_DIV_12,
    SPO_CLK_SW_DIV_24,
    SPO_CLK_SW_DIV_48,
    SPO_CLK_SW_DIV_96,
    SPO_CLK_SW_DIV_192
    
} SPO_CLK_SW_DivMode_t;


typedef enum SPO_LRCLK_DivMode_e
{
    SPO_LRCLK_DIV_1 = 0,
    SPO_LRCLK_DIV_2,
    SPO_LRCLK_DIV_3,
    SPO_LRCLK_DIV_6
    
} SPO_LRCLK_DivMode_t;


typedef enum AUD_F128_FS_Mode_e
{
    AUD_128xI2SO_FS = 0,
    AUD_128xSPO_FS,
    AUD_128xDDPSPO_FS
    
} AUD_F128_FS_Mode_t;


typedef enum DDPSPO_CLK_SW_DivMode_e
{
    DDPSPO_CLK_SW_DIV_1 = 0,
    DDPSPO_CLK_SW_DIV_2,
    DDPSPO_CLK_SW_DIV_3,
    DDPSPO_CLK_SW_DIV_6,
    DDPSPO_CLK_SW_DIV_12,
    DDPSPO_CLK_SW_DIV_24,
    DDPSPO_CLK_SW_DIV_48,
    DDPSPO_CLK_SW_DIV_96,
    DDPSPO_CLK_SW_DIV_192
    
} DDPSPO_CLK_SW_DivMode_t;


typedef enum DAC_CLK_SW_DivMode_e
{
    DAC_CLK_SW_DIV_2 = 0,
    DAC_CLK_SW_DIV_4,
    DAC_CLK_SW_DIV_8,
    DAC_CLK_SW_DIV_16,
    DAC_CLK_SW_DIV_32
    
} DAC_CLK_SW_DivMode_t;


typedef enum DAC_I2S_SDAT_SwapMode_e
{
    SDAT_OUT_MSB_FIRST = 0,
    SDAT_OUT_LSB_FIRST
    
} DAC_I2S_SDAT_SwapMode_t;


typedef enum DAC_I2S_ChanNum_e
{
    DAC_I2S_SINGLE_CHAN = 0,
    DAC_I2S_TWO_CHAN
    
} DAC_I2S_ChanNum_t;


typedef enum ADAC_RandomSource_e
{
    FROM_SELF_HALF_CLK = 0,
    FROM_RDM_IN
    
} ADAC_RandomSource_t;


typedef enum ADAC_BistFormat_e
{
    ADAC_BIST_SNR = 0,
    ADAC_BIST_L_CT,
    ADAC_BIST_R_CT,
    ADAC_BIST_THD_N,
    ADAC_BIST_N_20DB_DR,
    ADAC_BIST_N_60DB_DR
    
} ADAC_BistFormat_t;


typedef enum ADAC_Analog_ClkSource_e
{
    ANALOG_CLK_FROM_APLL = 0,
    ANALOG_CLK_FROM_ADAC
    
} ADAC_Analog_ClkSource_t;


typedef enum DAC_Invert_DsmDataMode_e
{
    DAC_INV_DSM_DATA = 0,
    DAC_ORG_DSM_DATA
    
} DAC_Invert_DsmDataMode_t;


typedef enum DAC_Invert_DsmClkMode_e
{
    DAC_ORG_DSM_CLK = 0,
    DAC_INV_DSM_CLK
    
} DAC_Invert_DsmClkMode_t;

typedef enum AUD_SPOChanPCMMode_e
{
    SRC_FLR  = 0,
    SRC_SLR  = 1,
    SRC_CSW  = 2,
    SRC_DMLR = 3,
    SRC_EXLR = 4,
    SRC_BUF  = 7,
    SRC_LFEC = 1
    
} AUD_SPOChanPCMMode_t;

typedef enum AUD_SPOSrcMode_e
{
	SPDIF_OUT_INVALID = -1, 
	SPDIF_OUT_PCM = 0,
	SPDIF_OUT_BS = 1,
	SPDIF_OUT_FORCE_DD = 2
	
} AUD_SPOSrcMode_t;


typedef struct AUD_OutputParams_s
{
    enum AudioStreamType   StreamType;
    AUD_SampleRate_t       SampleRate;
    AUD_UINT32             SampleNum;
    I2S_OUT_FORMAT_t       Format;
    AUD_Precision_t        Precision;
    AUD_ChanNum_t          ChannelNum;
    AUD_Endian_Mode_t      Endian;
    AUD_UINT8              MainClk;
    
    AUD_SPOSrcMode_t       SPOSrcMode;
    AUD_SPOChanPCMMode_t   SPOChlPCMType;
    
    AUD_SPOSrcMode_t       DDPSPOSrcMode;
    AUD_SPOChanPCMMode_t   DDPSPOChlPCMType;

    AUD_UINT32             BitStreamSetByUser;    
    
    AUD_BOOL               MPEGM8dbEnableFlag;
    AUD_BOOL               DMADataHeaderFlag;
    AUD_BOOL               SoftwareCfgClkFlag;
    
    AUD_BOOL               SPODMAForPCMOutFlag;
    AUD_BOOL               DDPSPODMAForPCMOutFlag;
    
} AUD_OutputParams_t;

#define GET_BIT(n) (1<<n)

#define AUD_Delay(Ms)                                msleep(Ms)

#endif /* __AUD_COMMON_H */

/* End of AUD_COMMON.h */
