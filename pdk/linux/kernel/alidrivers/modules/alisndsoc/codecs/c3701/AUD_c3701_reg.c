
/*********************************************************************
*
* Copyright (C), 1988-2012,Ali Corp. All rights reserved.
*
* Filename:       AUD_c3701_reg.h 
* Description:    Register control functions for Chip C3701C
* Author:         Jacket Wu <jacket.wu@alitech.com> 
* Create date:    Mar,15,2013 
*
*
* Revision history:
* --Revision----Author---------Date----------Reason----------
*   1.0         Jacket Wu      2013/03/15    Create file.
* 
***********************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>


#include "../../AUD_common.h"
#include "../../AUD_reg.h"



/* -----------------------------------------------------------------------------
C3701 Chip base address definition
----------------------------------------------------------------------------- */
#define C3701_SYS_BASE_ADDR                  0xB8000000
#define C3701_SND_BASE_ADDR                  0xB8002000


/* -----------------------------------------------------------------------------
C3701 Chip Registers definition
----------------------------------------------------------------------------- */
#define R_SYS_INT_POLARITY_0X28              (C3701_SYS_BASE_ADDR + 0x28) // 32 bits
#define R_SYS_INT_STATUS_0X30                (C3701_SYS_BASE_ADDR + 0x30) // 32 bits
#define R_SYS_INT_ENABLE_0X38                (C3701_SYS_BASE_ADDR + 0x38) // 32 bits
#define R_SYS_DEVICE_CLK_GATING_CTRL_0X60    (C3701_SYS_BASE_ADDR + 0x60) // 32 bits
#define R_SYS_DEVICE_RESET_CTRL_0X80         (C3701_SYS_BASE_ADDR + 0x80) // 32 bits
#define R_SYS_PINMUX_CTRL1_0X8C              (C3701_SYS_BASE_ADDR + 0x8C) // 32 bits
#define R_SYS_PINMUX_CTRL2_0XA8              (C3701_SYS_BASE_ADDR + 0xA8) // 32 bits
#define R_SYS_CTRL_0XB0                      (C3701_SYS_BASE_ADDR + 0xB0) // 32 bits



/* -----------------------------------------------------------------------------
C3701 Audio Registers definition
----------------------------------------------------------------------------- */
#define R_AUD_CTRL_0X00                      (C3701_SND_BASE_ADDR + 0x00) // 08 bits
#define R_AUD_INT_0X01                       (C3701_SND_BASE_ADDR + 0x01) // 08 bits
#define R_I2SO_CLK_INTF_CTRL_0X02            (C3701_SND_BASE_ADDR + 0x02) // 08 bits
#define R_I2SO_AUD_FRAME_INFO_0X03           (C3701_SND_BASE_ADDR + 0x03) // 08 bits
#define R_I2SO_AUD_PTS_0X04                  (C3701_SND_BASE_ADDR + 0x04) // 32 bits
#define R_I2SO_AUD_FRAME_COUNTER_0X08        (C3701_SND_BASE_ADDR + 0x08) // 16 bits
#define R_I2SO_AUD_FRAME_LEN_0X0A            (C3701_SND_BASE_ADDR + 0x0A) // 16 bits
#define R_I2SO_LAST_VALID_INDEX_0X0C         (C3701_SND_BASE_ADDR + 0x0C) // 16 bits
#define R_I2SO_TIME_CHECK_THD_0X0E           (C3701_SND_BASE_ADDR + 0x0E) // 16 bits
#define R_I2SO_SAMP_COUNTER_THD_0X10         (C3701_SND_BASE_ADDR + 0x10) // 16 bits
#define R_I2SO_SAMP_COUNTER_0X12             (C3701_SND_BASE_ADDR + 0x12) // 16 bits
#define R_BUF_UNDER_RUN_CTRL_0X14            (C3701_SND_BASE_ADDR + 0x14) // 08 bits
#define R_DMA_CTRL_0X15                      (C3701_SND_BASE_ADDR + 0x15) // 08 bits
#define R_DEBUG_CTRL_0X16                    (C3701_SND_BASE_ADDR + 0x16) // 08 bits
#define R_I2SO_LATENCY_COUNTER_0X17          (C3701_SND_BASE_ADDR + 0x17) // 08 bits
#define R_I2SO_DMA_BASE_ADDR_0X18            (C3701_SND_BASE_ADDR + 0x18) // 32 bits
#define R_I2SO_DMA_CUR_INDEX_0X1C            (C3701_SND_BASE_ADDR + 0x1C) // 16 bits
#define R_I2SO_DMA_LEN_0X1E                  (C3701_SND_BASE_ADDR + 0x1E) // 16 bits


#define R_SPO_CTRL_0X20                      (C3701_SND_BASE_ADDR + 0x20) // 08 bits
#define R_SPO_IEC_CTRL_0X21                  (C3701_SND_BASE_ADDR + 0x21) // 08 bits 
#define R_SPO_SAMP_COUNTER_TRHD_0X22         (C3701_SND_BASE_ADDR + 0x22) // 16 bits
#define R_SPO_CS_0X24                        (C3701_SND_BASE_ADDR + 0x24) // 32 bits
#define R_SPO_LEFT_USER_DATA_0X28            (C3701_SND_BASE_ADDR + 0x28) // 192 bits
#define R_SPO_RIGHT_USER_DATA_0X40           (C3701_SND_BASE_ADDR + 0x40) // 192 bits
#define R_SPO_IEC_PC_0X58                    (C3701_SND_BASE_ADDR + 0x58) // 16 bits
#define R_SPO_IEC_PD_0X5A                    (C3701_SND_BASE_ADDR + 0x5A) // 16 bits
#define R_SPO_IEC_NULL_0X5C                  (C3701_SND_BASE_ADDR + 0x5C) // 16 bits
#define R_SPO_IEC_DATA_0X5E                  (C3701_SND_BASE_ADDR + 0x5E) // 16 bits
#define R_SPO_DMA_BASE_ADDR_0X60             (C3701_SND_BASE_ADDR + 0x60) // 32 bits
#define R_SPO_DMA_CUR_INDEX_0X64             (C3701_SND_BASE_ADDR + 0x64) // 16 bits
#define R_SPO_DMA_LEN_0X66                   (C3701_SND_BASE_ADDR + 0x66) // 16 bits
#define R_SPO_DMA_LAST_INDEX_0X68            (C3701_SND_BASE_ADDR + 0x68) // 16 bits


#define R_EQ_CTRL_0X6A                       (C3701_SND_BASE_ADDR + 0x6A) // 16 bits
#define R_FADE_CTRL_0X6C                     (C3701_SND_BASE_ADDR + 0x6C) // 08 bits
#define R_FADE_SPEED_0X6D                    (C3701_SND_BASE_ADDR + 0x6D) // 08 bits
#define R_TARGET_VOLUME_0X6E                 (C3701_SND_BASE_ADDR + 0x6E) // 08 bits
#define R_CUR_VOLUME_0X6F                    (C3701_SND_BASE_ADDR + 0x6F) // 08 bits

#define R_EQ_IIR1_COEFF_0_31_0X8C            (C3701_SND_BASE_ADDR + 0x8C) // 32 bits
#define R_EQ_IIR1_COEFF_32_63_0X90           (C3701_SND_BASE_ADDR + 0x90) // 32 bits
#define R_EQ_IIR1_COEFF_64_79_0X94           (C3701_SND_BASE_ADDR + 0x94) // 16 bits
#define R_EQ_IIR2_COEFF_0_15_0X96            (C3701_SND_BASE_ADDR + 0x96) // 16 bits
#define R_EQ_IIR2_COEFF_16_47_0X98           (C3701_SND_BASE_ADDR + 0x98) // 32 bits
#define R_EQ_IIR2_COEFF_48_79_0X9C           (C3701_SND_BASE_ADDR + 0x9C) // 32 bits
#define R_EQ_IIR3_COEFF_0_31_0XA0            (C3701_SND_BASE_ADDR + 0xA0) // 32 bits
#define R_EQ_IIR3_COEFF_32_63_0XA4           (C3701_SND_BASE_ADDR + 0xA4) // 32 bits
#define R_EQ_IIR3_COEFF_64_79_0XA8           (C3701_SND_BASE_ADDR + 0xA8) // 16 bits
#define R_EQ_IIR4_COEFF_0_15_0XAA            (C3701_SND_BASE_ADDR + 0xAA) // 16 bits
#define R_EQ_IIR4_COEFF_16_47_0XAC           (C3701_SND_BASE_ADDR + 0xAC) // 32 bits
#define R_EQ_IIR4_COEFF_48_79_0XB0           (C3701_SND_BASE_ADDR + 0xB0) // 32 bits

#define R_COUNTER_F0_FRQ_0XE4                (C3701_SND_BASE_ADDR + 0xE4) // 16 bits
#define R_COUNTER_F0_CTRL_0XE6               (C3701_SND_BASE_ADDR + 0xE6) // 08 bits
#define R_COUNTER_F0_33_0_0XE7               (C3701_SND_BASE_ADDR + 0xE7) // 08 bits
#define R_COUNTER_F0_1_32_0XE8               (C3701_SND_BASE_ADDR + 0xE8) // 32 bits
#define R_COUNTER_F1_FRQ_0XEC                (C3701_SND_BASE_ADDR + 0xEC) // 16 bits
#define R_COUNTER_F1_CTRL_0XEE               (C3701_SND_BASE_ADDR + 0xEE) // 08 bits
#define R_COUNTER_F1_33_0_0XEF               (C3701_SND_BASE_ADDR + 0xEF) // 08 bits
#define R_COUNTER_F1_1_32_0XF0               (C3701_SND_BASE_ADDR + 0xF0) // 32 bits

#define R_DAC_I2S_REG_0_7_0XF4               (C3701_SND_BASE_ADDR + 0xF4) // 08 bits
#define R_DAC_I2S_REG_8_15_0XF5              (C3701_SND_BASE_ADDR + 0xF5) // 08 bits
#define R_DAC_I2S_REG_16_23_0XF6             (C3701_SND_BASE_ADDR + 0xF6) // 08 bits
#define R_DAC_I2S_REG_24_31_0XF7             (C3701_SND_BASE_ADDR + 0xF7) // 08 bits
#define R_DAC_CODEC_REG_0_7_0XF8             (C3701_SND_BASE_ADDR + 0xF8) // 08 bits
#define R_DAC_CODEC_REG_8_15_0XF9            (C3701_SND_BASE_ADDR + 0xF9) // 08 bits
#define R_DAC_CODEC_REG_16_23_0XFA           (C3701_SND_BASE_ADDR + 0xFA) // 08 bits
#define R_DAC_CODEC_REG_24_31_0XFB           (C3701_SND_BASE_ADDR + 0xFB) // 08 bits
#define R_DAC_CODEC_REG_32_39_0XFC           (C3701_SND_BASE_ADDR + 0xFC) // 08 bits
#define R_DAC_CODEC_REG_40_47_0XFD           (C3701_SND_BASE_ADDR + 0xFD) // 08 bits
#define R_DAC_CODEC_REG_48_63_0XFE           (C3701_SND_BASE_ADDR + 0xFE) // 16 bits
#define R_DAC_CODEC_REG_64_71_0X100          (C3701_SND_BASE_ADDR + 0x100) // 08 bits
#define R_DAC_CODEC_REG_72_79_0X101          (C3701_SND_BASE_ADDR + 0x101) // 08 bits
#define R_DAC_CODEC_REG_80_87_0X102          (C3701_SND_BASE_ADDR + 0x102) // 08 bits
#define R_DAC_CODEC_REG_88_95_0X103          (C3701_SND_BASE_ADDR + 0x103) // 08 bits

#define R_I2SI_INTF_CTRL_0_7_0X108           (C3701_SND_BASE_ADDR + 0x108) // 08 bits
#define R_I2SI_INTF_CTRL_8_15_0X109          (C3701_SND_BASE_ADDR + 0x109) // 08 bits
#define R_I2SI_INTF_CTRL_16_23_0X10A         (C3701_SND_BASE_ADDR + 0x10A) // 08 bits
#define R_I2SI_INTF_CTRL_24_31_0X10B         (C3701_SND_BASE_ADDR + 0x10B) // 08 bits
#define R_I2SI_VOLUME_CTRL_0X10C             (C3701_SND_BASE_ADDR + 0x10C) // 16 bits
#define R_I2SI_START_CTRL_0X10E              (C3701_SND_BASE_ADDR + 0x10E) // 08 bits
#define R_I2SI_INT_CTRL_0X10F                (C3701_SND_BASE_ADDR + 0x10F) // 08 bits

#define R_I2SI_DMA_BASE_ADDR_0X110           (C3701_SND_BASE_ADDR + 0x110) // 32 bits
#define R_I2SI_RX_DMA_CUR_INDEX_0X114        (C3701_SND_BASE_ADDR + 0x114) // 16 bits
#define R_I2SI_TX_DMA_CUR_INDEX_0X116        (C3701_SND_BASE_ADDR + 0x116) // 16 bits
#define R_I2SI_DMA_BASE_LEN_0X118            (C3701_SND_BASE_ADDR + 0x118) // 16 bits
#define R_I2SI_TX_LAST_VALID_INDEX_0X11A     (C3701_SND_BASE_ADDR + 0x11A) // 16 bits
#define R_I2SI_RX_SAMP_COUNTER_TRHD_0X11C    (C3701_SND_BASE_ADDR + 0x11C) // 16 bits
#define R_I2SI_RX_SAMP_COUNTER_0X11E         (C3701_SND_BASE_ADDR + 0x11E) // 16 bits

#define R_DDP_SPO_LEFT_USER_DATA_0X120       (C3701_SND_BASE_ADDR + 0x120) // 192 bits
#define R_DDP_SPO_RIGHT_USER_DATA_0X138      (C3701_SND_BASE_ADDR + 0x138) // 192 bits
#define R_DDP_SPO_CS_0X150                   (C3701_SND_BASE_ADDR + 0x150) // 32 bits
#define R_DDP_SPO_DMA_BASE_ADDR_0X154        (C3701_SND_BASE_ADDR + 0x154) // 32 bits
#define R_DDP_SPO_DMA_LEN_0X158              (C3701_SND_BASE_ADDR + 0x158) // 16 bits
#define R_DDP_SPO_DMA_CUR_INDEX_0X15A        (C3701_SND_BASE_ADDR + 0x15A) // 16 bits
#define R_DDP_SPO_SAMP_COUNTER_TRHD_0X15C    (C3701_SND_BASE_ADDR + 0x15C) // 16 bits
#define R_DDP_SPO_LAST_INDEX_0X15E           (C3701_SND_BASE_ADDR + 0x15E) // 16 bits
#define R_DDP_SPO_INT_0X160                  (C3701_SND_BASE_ADDR + 0x160) // 08 bits
#define R_DDP_SPO_IEC_CTRL_0X161             (C3701_SND_BASE_ADDR + 0x161) // 08 bits
#define R_DDP_SPO_CTRL_0X162                 (C3701_SND_BASE_ADDR + 0x162) // 16 bits
#define R_DDP_SPO_IEC_NULL_0X164             (C3701_SND_BASE_ADDR + 0x164) // 16 bits
#define R_DDP_SPO_IEC_DATA_0X166             (C3701_SND_BASE_ADDR + 0x166) // 16 bits
#define R_DDP_SPO_IEC_PC_0X168               (C3701_SND_BASE_ADDR + 0x168) // 16 bits
#define R_DDP_SPO_IEC_PD_0X16A               (C3701_SND_BASE_ADDR + 0x16A) // 16 bits

#define R_I2SI_TX_SAMP_COUNTER_TRHD_0X16C    (C3701_SND_BASE_ADDR + 0x16C) // 16 bits
#define R_I2SI_TX_SAMP_COUNTER_0X16E         (C3701_SND_BASE_ADDR + 0x16E) // 16 bits
#define R_ALSA_DATA_FMT_0X178                (C3701_SND_BASE_ADDR + 0x178) // 16 bits
#define R_UNDER_RUN_IMPROVE_0X17A            (C3701_SND_BASE_ADDR + 0x17A) // 16 bits

#define R_CHIP_ID_0X400                      (C3701_SND_BASE_ADDR + 0x400) // 32 bits



/* Define FS setting macro for C3701C --------------------------------------- */

#define TIME_CHECK_THRESHOLD       100


#define GET_BIT(n) (1<<n)

#define GETFS(setting) (setting >> 8)


// fs, 8 bit
#define FSSET(fs) (fs / 1000)


// mclk setting: 3 bit.
#define I2S_MCLKSET(mclk)                                     \
    (((24576000 == mclk) || (12288000 == mclk)) ?             \
        2 :                                                   \
        ((22579200 == mclk) || (11289600 == mclk)) ?          \
            3 :                                               \
            (18432000 == mclk) ?                              \
                0 :                                           \
                (16934400 == mclk) ?                          \
                    1 :                                       \
                    4)


// mclk-bck div setting: 3 bit
#define I2S_BCKSET(bckdiv)                                    \
    ((1 == bckdiv) ?                                          \
        0 :                                                   \
        (2 == bckdiv) ?                                       \
            1 :                                               \
            (4 == bckdiv) ?                                   \
                2 :                                           \
                (8 == bckdiv) ?                               \
                    3 :                                       \
                    (3 == bckdiv) ?                           \
                        4 :                                   \
                        (6 == bckdiv) ?                       \
                            5 :                               \
                            (12 == bckdiv) ?                  \
                                6 :                           \
                                7)

// bck-lrck div: 2 bit
#define I2SO_LRCKSET(lrckdiv)                                 \
    ((32 == lrckdiv) ?                                        \
        0 :                                                   \
        (48 == lrckdiv) ?                                     \
            1 :                                               \
            (64 == lrckdiv) ?                                 \
                2 :                                           \
                3)


// bit field: 8|3|3|2
#define I2SO_SETTING(fs, mclk, bck_div, lrck_div)             \
    ((FSSET(fs) << 8) |                                       \
        (I2S_MCLKSET(mclk) << 5) |                            \
            (I2S_BCKSET(bck_div) << 2) |                      \
                I2SO_LRCKSET(lrck_div))


#define SPO_CLKSEL_SET(clksel)	                              \
    ((1 == clksel) ?                                          \
        0 :                                                   \
        (2 == clksel) ?                                       \
            1 :                                               \
            (3 == clksel) ?                                   \
                2 :                                           \
                (6 == clksel) ?                               \
                    3 :                                       \
                    4)


#define SPO_SETTING(fs, clksel, cs)                           \
    ((FSSET(fs) << 8) |                                       \
        (SPO_CLKSEL_SET(clksel) << 4) |                       \
            cs)



static AUD_UINT16 DAC_24Bit_Setting[] = 
{
    // fs, mclk, mclk/bclk, bclk/lclk
    // fs, mclk, mclk/bick, bclk/lrck
    // 8, 3, 3, 2
    I2SO_SETTING(192000, 18432000, 2, 48),
    I2SO_SETTING(176400, 16934400, 2, 48),
    I2SO_SETTING(96000, 18432000, 4, 48),
    I2SO_SETTING(88200, 16934400, 4, 48),
    I2SO_SETTING(64000, 24576000, 8, 48),
    I2SO_SETTING(48000, 18432000, 8, 48),
    I2SO_SETTING(44100, 16934400, 8, 48),
    I2SO_SETTING(22050, 16934400, 8, 48),
    I2SO_SETTING(32000, 24576000, 8, 48),
    I2SO_SETTING(24000,18432000,8,48),
    I2SO_SETTING(16000, 24576000, 8, 48),
    I2SO_SETTING(8000, 24576000, 8, 48),
};



static AUD_UINT16 DAC_16Bit_Setting[] = 
{
    // fs, mclk, mclk/bclk, bclk/lclk
    // fs, mclk, mclk/bick, bclk/lrck
    // 8, 3, 3, 2
    I2SO_SETTING(192000, 24576000, 4, 32),
    I2SO_SETTING(176400, 22579200, 4, 32),
    I2SO_SETTING(96000, 24576000, 8, 32),
    I2SO_SETTING(88200, 22579200, 8, 32),
    I2SO_SETTING(64000, 24576000, 12, 32),
    I2SO_SETTING(48000, 18432000, 12, 32),
    I2SO_SETTING(44100, 16934400, 12, 32),
    I2SO_SETTING(22050, 16934400, 12, 32),
    I2SO_SETTING(32000, 24576000,  12, 32),
    I2SO_SETTING(24000, 18432000, 12, 32),
    I2SO_SETTING(16000, 24576000, 24, 32),
    I2SO_SETTING(8000, 24576000, 24, 32)
};



static AUD_UINT16 SPO_Setting[7][4] = 
{
     // fs, clksel, cs
     // 8, 4, 4
    {// 18.432Mhz Group
        SPO_SETTING(48000,3, 0x2),
        SPO_SETTING(24000,3, 0x6),
        0,
        0
    },
    {// 16.9344Mhz Group
        SPO_SETTING(44100, 3, 0x0),
        SPO_SETTING(22050, 3, 0x4),
        0,
        0
    },
    {// 24.576Mhz Group
        SPO_SETTING(32000, 6, 0x3),
        SPO_SETTING(64000, 3, 0x1),
        SPO_SETTING(96000, 2, 0xa),
        SPO_SETTING(192000,1, 0xe)
    },
    {// 22.5792Mhz
        SPO_SETTING(88200, 2, 0x8),
        SPO_SETTING(176400, 1, 0xc),
        0,
        0
    },
    {// 6.144Mhz Group
        SPO_SETTING(8000, 6, 0x9),
        0,
        0,
        0
    },
    {// 12.288Mhz Group
        SPO_SETTING(16000, 6, 0x7),
        0,
        0,
        0
    },
    {// unsupported
        0,
        0,
        0,
        0
    }

};



static AUD_UINT16 DDPSPO_Setting[7][4] =
{
     // fs, clksel, cs
     // 8, 4, 4
    {// 18.432Mhz Group
        SPO_SETTING(48000,3, 0x2),
        SPO_SETTING(24000,3, 0x6),
        0,
        0
    },
    {// 16.9344Mhz Group
        SPO_SETTING(44100, 3, 0x0),
        SPO_SETTING(22050, 3, 0x4),
        0,
        0
    },
    {// 24.576Mhz Group
        SPO_SETTING(32000, 6, 0x3),
        SPO_SETTING(64000, 3, 0x1),
        SPO_SETTING(96000, 2, 0xa),
        SPO_SETTING(192000,1, 0xe)
    },
    {// 22.5792Mhz
        SPO_SETTING(88200, 2, 0x8),
        SPO_SETTING(176400, 1, 0xc),
        0,
        0
    },
    {// 6.144Mhz Group
        SPO_SETTING(8000, 6, 0x9),
        0,
        0,
        0
    },
    {// 12.288Mhz Group
        SPO_SETTING(16000, 6, 0x7),
        0,
        0,
        0
    },
    {// unsupported
        0,
        0,
        0,
        0
    }
};




/* -----------------------------------------------------------------------------
System Hardware Register Control Functions
----------------------------------------------------------------------------- */

static void DisableAudClockGate(void)
{
    // reg offset: 0x60 ; bit: 5
    
    AUD_UINT8 RegValue = 0;
    
    RegValue = AUD_ReadRegDev8(R_SYS_DEVICE_CLK_GATING_CTRL_0X60);
    if (RegValue&(0x1<<5))
    {
        AUD_WriteRegDev8(R_SYS_DEVICE_CLK_GATING_CTRL_0X60, \
                         (RegValue&(~(0x1<<5))));
    }    
}



void ResetAudCore(void)
{
    // reg offset: 0x80 ; bit: 5
    
    AUD_UINT8 RegValue = 0;
    
    RegValue = AUD_ReadRegDev8(R_SYS_DEVICE_RESET_CTRL_0X80);

    AUD_WriteRegDev8(R_SYS_DEVICE_RESET_CTRL_0X80, \
                     (RegValue | (0x1<<5)));

    AUD_Delay(3);

    AUD_WriteRegDev8(R_SYS_DEVICE_RESET_CTRL_0X80, \
                     (RegValue&(~(0x1<<5))));    
}

EXPORT_SYMBOL_GPL(ResetAudCore);

static void ConfigSystemCtrlReg(AUD_UINT32 Data)
{
    // reg offset: 0xB0 ; bit: 24, 20, 6, 5, 2, 1, 0

    AUD_WriteRegDev32(R_SYS_CTRL_0XB0, Data);
}



static AUD_UINT32 GetSystemCtrlRegConfig(void)
{
    // reg offset: 0xB0 ; bit: 24, 20, 6, 5, 2, 1, 0
    
    return AUD_ReadRegDev32(R_SYS_CTRL_0XB0);
}




/* -----------------------------------------------------------------------------
AUD0 Hardware Register Control Functions
----------------------------------------------------------------------------- */

/* Basic control ------------------------------------------------------------ */

static void StartAUD0(void)
{
    /* Dummy function */
    return;
}



static void StopAUD0(void)
{
    /* Dummy function */
    return;
}



static void StartAUD0DecoderDMA(void)
{
    /* Dummy function */
    return;
}



static void StopAUD0DecoderDMA(void)
{
    /* Dummy function */
    return;
}



static void StartAUD0EncoderDMA(void)
{
    /* Dummy function */
    return;
}



static void StopAUD0EncoderDMA(void)
{
    /* Dummy function */
    return;
}



/* Audio CPU0 Register Operation Functions ---------------------------------- */

static void SetAUD0DecStreamType(enum AudioStreamType StreamType)
{
    /* Dummy function */
    return;
}



static void SetAUD0DecDMAInAddr(AUD_UINT32 Addr)
{
    /* Dummy function */
    return;
}



static void SetAUD0DecDMAInLength(AUD_UINT32 Length)
{
    /* Dummy function */
    return;
}



static void SetAUD0DecDMAInWriteIdx(AUD_UINT32 WriteIndex)
{
    /* Dummy function */
    return;
}



static void SetAUD0DecDMAOutAddr(AUD_UINT32 Addr)
{
    /* Dummy function */
    return;
}



static void SetAUD0DecDMAOutLength(AUD_UINT32 Length)
{
    /* Dummy function */
    return;
}



static void SetAUD0DecDMAOutReadIdx(AUD_UINT32 ReadIndex)
{
    /* Dummy function */
    return;
}



static void SetAUD0EncDMAInAddr(AUD_UINT32 Addr)
{
    /* Dummy function */
    return;
}



static void SetAUD0EncDMAInLength(AUD_UINT32 Length)
{
    /* Dummy function */
    return;
}



static void SetAUD0EncDMAInWriteIdx(AUD_UINT32 WriteIndex)
{
    /* Dummy function */
    return;
}



static void SetAUD0EncDMAOutAddr(AUD_UINT32 Addr)
{
    /* Dummy function */
    return;
}



static void SetAUD0EncDMAOutLength(AUD_UINT32 Length)
{
    /* Dummy function */
    return;
}



static void SetAUD0EncDMAOutReadIdx(AUD_UINT32 ReadIndex)
{
    /* Dummy function */
    return;
}



static void SetAUD0DecPara(AUD_UINT32 *ParaSettingAddr)
{
    /* Dummy function */
    return;
}


static AUD_UINT8 GetAUD0CtrlStatus(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD0DecDMACtrlStatus(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT32 GetAUD0DecDMAInReadIdx(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT32 GetAUD0DecDMAOutWriteIdx(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT32 GetAUD0DecDMAInFrameLength(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD0DecDMAInFrameStreamType(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD0DecDMAInFrameChanNum(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD0DecDMAInFrameSampRate(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD0DecDMAOutFrameStatus(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD0DecDMAOutFrameBitDepth(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT16 GetAUD0DecDMAOutFrameSampNum(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT16 GetAUD0DecDMAOutFrameSampRate(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT16 GetAUD0DecDMAOutFrameChanNum(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD0EncDMAStatus(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT32 GetAUD0EncDMAInReadIdx(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT32 GetAUD0EncDMAOutWriteIdx(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD0EncDMAInFrameDrcMode(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD0EncDMAInFrameBitRate(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD0EncDMAInFrameChanMode(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD0EncDMAInFrameBitDepth(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT32 GetAUD0EncDMAInFrameSampRate(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT16 GetAUD0EncDMAOutFrameStatus(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT16 GetAUD0EncDMAOutFrameLen(void)
{
    /* Dummy function */
    return 0;
}



/* -----------------------------------------------------------------------------
AUD1 Hardware Register Control Functions
----------------------------------------------------------------------------- */

/* Basic control ------------------------------------------------------------ */

static void StartAUD1(void)
{
    /* Dummy function */
    return;
}



static void StopAUD1(void)
{
    /* Dummy function */
    return;
}



static void StartAUD1DecoderDMA(void)
{
    /* Dummy function */
    return;
}



static void StopAUD1DecoderDMA(void)
{
    /* Dummy function */
    return;
}



static void StartAUD1EncoderDMA(void)
{
    /* Dummy function */
    return;
}



static void StopAUD1EncoderDMA(void)
{
    /* Dummy function */
    return;
}



/* Audio CPU1 Register Operation Functions ---------------------------------- */

static void SetAUD1DecStreamType(enum AudioStreamType StreamType)
{
    /* Dummy function */
    return;
}



static void SetAUD1DecDMAInAddr(AUD_UINT32 Addr)
{
    /* Dummy function */
    return;
}



static void SetAUD1DecDMAInLength(AUD_UINT32 Length)
{
    /* Dummy function */
    return;
}



static void SetAUD1DecDMAInWriteIdx(AUD_UINT32 WriteIndex)
{
    /* Dummy function */
    return;
}



static void SetAUD1DecDMAOutAddr(AUD_UINT32 Addr)
{
    /* Dummy function */
    return;
}



static void SetAUD1DecDMAOutLength(AUD_UINT32 Length)
{
    /* Dummy function */
    return;
}



static void SetAUD1DecDMAOutReadIdx(AUD_UINT32 ReadIndex)
{
    /* Dummy function */
    return;
}



static void SetAUD1EncDMAInAddr(AUD_UINT32 Addr)
{
    /* Dummy function */
    return;
}



static void SetAUD1EncDMAInLength(AUD_UINT32 Length)
{
    /* Dummy function */
    return;
}



static void SetAUD1EncDMAInWriteIdx(AUD_UINT32 WriteIndex)
{
    /* Dummy function */
    return;
}



static void SetAUD1EncDMAOutAddr(AUD_UINT32 Addr)
{
    /* Dummy function */
    return;
}



static void SetAUD1EncDMAOutLength(AUD_UINT32 Length)
{
    /* Dummy function */
    return;
}



static void SetAUD1EncDMAOutReadIdx(AUD_UINT32 ReadIndex)
{
    /* Dummy function */
    return;
}



static void SetAUD1DecPara(AUD_UINT32 *ParaSettingAddr)
{
    /* Dummy function */
    return;
}


static AUD_UINT8 GetAUD1CtrlStatus(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD1DecDMACtrlStatus(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT32 GetAUD1DecDMAInReadIdx(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT32 GetAUD1DecDMAOutWriteIdx(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT32 GetAUD1DecDMAInFrameLength(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD1DecDMAInFrameStreamType(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD1DecDMAInFrameChanNum(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD1DecDMAInFrameSampRate(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD1DecDMAOutFrameStatus(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD1DecDMAOutFrameBitDepth(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT16 GetAUD1DecDMAOutFrameSampNum(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT16 GetAUD1DecDMAOutFrameSampRate(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT16 GetAUD1DecDMAOutFrameChanNum(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD1EncDMAStatus(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT32 GetAUD1EncDMAInReadIdx(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT32 GetAUD1EncDMAOutWriteIdx(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD1EncDMAInFrameDrcMode(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD1EncDMAInFrameBitRate(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD1EncDMAInFrameChanMode(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetAUD1EncDMAInFrameBitDepth(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT32 GetAUD1EncDMAInFrameSampRate(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT16 GetAUD1EncDMAOutFrameStatus(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT16 GetAUD1EncDMAOutFrameLen(void)
{
    /* Dummy function */
    return 0;
}



/* -----------------------------------------------------------------------------
AUD1 Hardware Register Control Functions
----------------------------------------------------------------------------- */

/* Basic setting ------------------------------------------------------------ */

static void SetI2SODMABufBaseAddr(AUD_UINT32 Addr)
{
    // reg offset: 0x18 ; bit: 31-0

    AUD_WriteRegDev32(R_I2SO_DMA_BASE_ADDR_0X18, \
                      (Addr&0x0FFFFFFF));
}



static void SetI2SODMABufLength(AUD_UINT16 Length)
{
    // reg offset: 0x1E ; bit: 15-0

    AUD_WriteRegDev16(R_I2SO_DMA_LEN_0X1E, \
                      (Length&0xFFFF));
}



static void SetI2SODMABufLastIndex(AUD_UINT16 Index)
{
    // reg offset: 0x0C ; bit: 15-0

    AUD_WriteRegDev16(R_I2SO_LAST_VALID_INDEX_0X0C, \
                      (Index&0xFFFF));
}



/* I2SO interface configure relative operation ------------------------------ */

static void SetI2SOTimeCheckThreshold(AUD_UINT16 Threshold)
{
    // reg offset: 0x0E ; bit: 15-0
    
    AUD_WriteRegDev16(R_I2SO_TIME_CHECK_THD_0X0E, \
                      (Threshold&0xFFFF));
}



static void SetI2SOSampCountThreshold(AUD_UINT16 Threshold)
{
    // reg offset: 0x10 ; bit: 15-0

    AUD_WriteRegDev16(R_I2SO_SAMP_COUNTER_THD_0X10, \
                      (Threshold&0xFFFF));
}

static AUD_UINT16 GetI2SOSampCountThreshold(void)
{
    // reg offset: 0x10 ; bit: 15-0

    return AUD_ReadRegDev16(R_I2SO_SAMP_COUNTER_THD_0X10);
}


void SetI2SODMADataWithHeader(AUD_BOOL Enable)
{
    // reg offset: 0x15 ; bit: 7

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DMA_CTRL_0X15);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DMA_CTRL_0X15, \
                         (RegValue&(~(0x01<<7))));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DMA_CTRL_0X15, \
                         ((RegValue&(~(0x01<<7))) | (0x01<<7)));
    }
    else
    {
        SDBBP();
    }
}

EXPORT_SYMBOL_GPL(SetI2SODMADataWithHeader);

static void SetI2SODMADataBitNum(I2SO_DRAM_BitNum_t Num)
{
    // reg offset: 0x15 ; bit: 5

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DMA_CTRL_0X15);

    if (BIT_NUM_16 == Num)
    {
        AUD_WriteRegDev8(R_DMA_CTRL_0X15, \
                         (RegValue&(~(0x01<<5))));
    }
    else if (BIT_NUM_24 == Num)
    {
        AUD_WriteRegDev8(R_DMA_CTRL_0X15, \
                         ((RegValue&(~(0x01<<5))) | (0x01<<5)));
    }
    else
    {
        SDBBP();
    }
}



static I2SO_DRAM_BitNum_t GetI2SODMADataBitNum(void)
{
    // reg offset: 0x15 ; bit: 5
    
    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DMA_CTRL_0X15);

    return ((0 == (RegValue&(0x01<<5))) ? BIT_NUM_16 : BIT_NUM_24);
}



static void SetI2SODMADataLeftAlign(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



// ???? C3701C spec have not this bit
static void SetI2SODMADataChannelNum(AUD_ChanNum_t Num)
{
    // reg offset: 0x15 ; bit: 6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DMA_CTRL_0X15);

    if (Num <= CHAN_NUM_2)
    {
        AUD_WriteRegDev8(R_DMA_CTRL_0X15, \
                         (RegValue&(~(0x01<<6))));
    }
    else if (CHAN_NUM_8 == Num)
    {
        AUD_WriteRegDev8(R_DMA_CTRL_0X15, \
                         ((RegValue&(~(0x01<<6))) | (0x01<<6)));
    }
}



// ???? C3701C spec have not this bit
static AUD_ChanNum_t GetI2SODMADataChannelNum(void)
{
    // reg offset: 0x15 ; bit: 6
    
    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DMA_CTRL_0X15);

    return ((0 == (RegValue&(0x01<<6))) ? CHAN_NUM_2 : CHAN_NUM_8);
}



static void EnableI2SOInterfaceClock(AUD_BOOL Enable)
{
    // reg offset: 0x00 ; bit: 6-4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_CTRL_0X00);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_CTRL_0X00, \
                         (RegValue&(~(0x07<<4))));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_CTRL_0X00, \
                         ((RegValue&(~(0x07<<4))) | (0x07<<4)));
    }
    else
    {
        SDBBP();
    }
}



static void SetI2SOInterfaceFormat(I2S_OUT_FORMAT_t Format)
{
    // reg offset: 0x02 ; bit: 7-6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_I2SO_CLK_INTF_CTRL_0X02);

    AUD_WriteRegDev8(R_I2SO_CLK_INTF_CTRL_0X02, \
                     ((RegValue&(~(0x03<<6))) | ((Format&0x03)<<6)));
}



// Only for C3701C
static void SelectI2SOMCLKMode(I2SO_MCLK_Mode_t Mode)
{
    // reg offset: 0x00 ; bit: 3-2

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_CTRL_0X00);

    AUD_WriteRegDev8(R_AUD_CTRL_0X00, \
                     ((RegValue&(~(0x03<<2))) | ((Mode&0x03)<<2)));
}



static void SetI2SOInterfaceBCLK2LRCLK(BIT_CLOCK_MODE_t Mode)
{
    // reg offset: 0x02 ; bit: 4-3
    
    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_I2SO_CLK_INTF_CTRL_0X02);

    AUD_WriteRegDev8(R_I2SO_CLK_INTF_CTRL_0X02, \
                     ((RegValue&(~(0x03<<3))) | ((Mode&0x03)<<3)));
}



static void EnableI2SOUnderRunFade(AUD_BOOL Enable)
{
    // reg offset: 0x14 ; bit: 1

    AUD_UINT8 RegValue = 0;
    
    RegValue = AUD_ReadRegDev8(R_BUF_UNDER_RUN_CTRL_0X14);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_BUF_UNDER_RUN_CTRL_0X14, \
                         ((RegValue&(~(0x01<<1))) | (0x01<<1)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_BUF_UNDER_RUN_CTRL_0X14, \
                         (RegValue&(~(0x01<<1))));
    }
    else
    {
        SDBBP();
    }
}



static void EnableI2SOAutoResume(AUD_BOOL Enable)
{
    // reg offset: 0x14 ; bit: 2

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_BUF_UNDER_RUN_CTRL_0X14);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_BUF_UNDER_RUN_CTRL_0X14, \
                         ((RegValue&(~(0x01<<2))) | (0x01<<2)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_BUF_UNDER_RUN_CTRL_0X14, \
                         (RegValue&(~(0x01<<2))));
    }
    else
    {
        SDBBP();
    }
}



static void EnableI2SOFSSoftwareConfig(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void EnableI2SOInterfaceMCLKGated(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void EnableI2SOInterfaceBCLKGated(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void SetI2SOMCLKDivBySoftware(I2SO_MCLK_DivMode_t MCLKDiv)
{
    // reg: 0x02 ; bit: 2-0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_I2SO_CLK_INTF_CTRL_0X02);

    switch (MCLKDiv)
    {
        case I2SO_MCLK_DIV_1:
        {
            AUD_WriteRegDev8(R_I2SO_CLK_INTF_CTRL_0X02, \
                         ((RegValue&(~(0x07<<0))) | (0x00<<0)));

            break;
        }

        case I2SO_MCLK_DIV_2:
        {
            AUD_WriteRegDev8(R_I2SO_CLK_INTF_CTRL_0X02, \
                         ((RegValue&(~(0x07<<0))) | (0x01<<0)));

            break;
        }

        case I2SO_MCLK_DIV_4:
        {
            AUD_WriteRegDev8(R_I2SO_CLK_INTF_CTRL_0X02, \
                         ((RegValue&(~(0x07<<0))) | (0x02<<0)));

            break;
        }

        case I2SO_MCLK_DIV_8:
        {
            AUD_WriteRegDev8(R_I2SO_CLK_INTF_CTRL_0X02, \
                         ((RegValue&(~(0x07<<0))) | (0x03<<0)));

            break;
        }

        case I2SO_MCLK_DIV_3:
        {
            AUD_WriteRegDev8(R_I2SO_CLK_INTF_CTRL_0X02, \
                         ((RegValue&(~(0x07<<0))) | (0x04<<0)));

            break;
        }

        case I2SO_MCLK_DIV_6:
        {
            AUD_WriteRegDev8(R_I2SO_CLK_INTF_CTRL_0X02, \
                         ((RegValue&(~(0x07<<0))) | (0x05<<0)));

            break;
        }

        case I2SO_MCLK_DIV_12:
        {
            AUD_WriteRegDev8(R_I2SO_CLK_INTF_CTRL_0X02, \
                         ((RegValue&(~(0x07<<0))) | (0x06<<0)));

            break;
        }

        case I2SO_MCLK_DIV_24:
        {
            AUD_WriteRegDev8(R_I2SO_CLK_INTF_CTRL_0X02, \
                         ((RegValue&(~(0x07<<0))) | (0x07<<0)));

            break;
        }

        default:
        {
            break;
        }
    }
}



static void SetI2SOBCLKDivBySoftware(I2SO_BCLK_DivMode_t BCLKDiv)
{
    /* Dummy function */
    return;
}



static void EnableI2SOF128FSSoftwareConfig(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void EnableI2SOF128FSGated(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void SetI2SOF128FSSoftDiv(I2SO_F128_DivMode_t F128Div)
{
    /* Dummy function */
    return;
}

/* --> I2SO clock config end */



/* FS hardware config */

static void EnableFSConfigByHardware(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void ConfigI2SOFSByHardware(AUD_SampleRate_t SampleRate)
{
    /* Dummy function */
    return;
}



static void ConfigI2SOFSBySoftware(AUD_SampleRate_t SampleRate)
{
    /* Dummy function */
    return;
}



void EnableI2SOInterface(AUD_BOOL Enable)
{
    // reg offset: 0x00 ; bit: 0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_CTRL_0X00);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_CTRL_0X00, \
                         ((RegValue&(~(0x01<<0))) | (0x01<<0)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_CTRL_0X00, \
                         (RegValue&(~(0x01<<0))));
    }
    else
    {
        SDBBP();
    }
}

EXPORT_SYMBOL_GPL(EnableI2SOInterface);

static AUD_BOOL CheckI2SOInfEnableStatus(void)
{
    // reg offset: 0x00 ; bit: 0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_CTRL_0X00);

    if (RegValue&(0x01<<0))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static void EnableI2SOClock(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



/* I2SO DMA control --------------------------------------------------------- */

void StartI2SODMA(AUD_BOOL Enable)
{
    // reg offset: 0x15 ; bit: 4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DMA_CTRL_0X15);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DMA_CTRL_0X15, \
                         ((RegValue&(~(0x01<<4))) | (0x01<<4)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DMA_CTRL_0X15, \
                         (RegValue&(~(0x01<<4))));      
    }
    else
    {
        SDBBP();
    }
}

EXPORT_SYMBOL_GPL(StartI2SODMA);

/* Get I2SO Interface information ------------------------------------------- */

static AUD_UINT16 GetI2SODMABufLastIndex(void)
{
    // reg offset: 0x0C ; bit: 15-0

    return AUD_ReadRegDev16(R_I2SO_LAST_VALID_INDEX_0X0C);
}



static AUD_UINT16 GetI2SODMABufCurrentIndex(void)
{
    // reg offset: 0x1C ; bit: 15-0

    return AUD_ReadRegDev16(R_I2SO_DMA_CUR_INDEX_0X1C);    
}



static AUD_UINT16 GetI2SOSampCount(void)
{
    // reg offset: 0x12 ; bit: 15-0

    return AUD_ReadRegDev16(R_I2SO_SAMP_COUNTER_0X12);
}



static AUD_UINT16 GetI2SOFrameCounter(void)
{
    // reg offset: 0x08 ; bit: 15-0

    return AUD_ReadRegDev16(R_I2SO_AUD_FRAME_COUNTER_0X08);
}


static AUD_UINT16 GetI2SOFrameLen(void)
{
    // reg offset: 0x0A ; bit: 15-0

    return AUD_ReadRegDev16(R_I2SO_AUD_FRAME_LEN_0X0A);
}



static AUD_UINT8 GetI2SOFrameHeaderInfo(void)
{
    // reg offset: 0x03 ; bit: 4-0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_I2SO_AUD_FRAME_INFO_0X03);

    return (RegValue&0x1F);
}



static AUD_UINT32 GetI2SOCurrentFramePTS(void)
{
    // reg offset: 0x04 ; bit: 31-0

    return AUD_ReadRegDev32(R_I2SO_AUD_PTS_0X04);
}



static AUD_BOOL GetI2SOBufUnderRunStatus(void)
{
    // reg offset: 0x14 ; bit: 0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_BUF_UNDER_RUN_CTRL_0X14);

    if (RegValue&0x01)
    {
        AUD_WriteRegDev8(R_BUF_UNDER_RUN_CTRL_0X14, RegValue);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL GetI2SOFIFOUnderRunStatus(void)
{
    /* Dummy function */
    return FALSE;
}



static AUD_UINT16 GetI2SODMASkipNum(void)
{
    /* Dummy function */
    return 0;
}



/* Enable I2SO interrupts --------------------------------------------------- */

static void EnableI2SOTimingCheckInt(AUD_BOOL Enable)
{
    // reg offset: 0x01 ; bit: 1

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_INT_0X01);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_INT_0X01, \
                         ((RegValue&(~(0x01<<1))) | (0x01<<1)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_INT_0X01, \
                         (RegValue&(~(0x01<<1))));
    }
    else
    {
        SDBBP();
    }
}



static void EnableI2SOSampCountInt(AUD_BOOL Enable)
{
    // reg offset: 0x01 ; bit: 0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_INT_0X01);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_INT_0X01, \
                         ((RegValue&(~(0x01<<0))) | (0x01<<0)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_INT_0X01, \
                         (RegValue&(~(0x01<<0))));
    }
    else
    {
        SDBBP();
    }
}



static void EnableI2SODMABufResumeInt(AUD_BOOL Enable)
{
    // reg offset: 0x01 ; bit: 3

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_INT_0X01);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_INT_0X01, \
                         ((RegValue&(~(0x01<<3))) | (0x01<<3)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_INT_0X01, \
                         (RegValue&(~(0x01<<3))));
    }
    else
    {
        SDBBP();
    }
}



void EnableI2SODMAUnderRunInt(AUD_BOOL Enable)
{
    /* Dummy function */
    EnableI2SODMABufResumeInt(TRUE);
    return;
}


EXPORT_SYMBOL_GPL(EnableI2SODMAUnderRunInt);

/* Get I2SO interrupt status ------------------------------------------------ */

static AUD_BOOL CheckI2SOTimingCheckInt(void)
{
    // reg offset: 0x01 ; bit: 1

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_INT_0X01);

    if (RegValue&(0x01<<1))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



AUD_BOOL CheckI2SOSampCountInt(void)
{
    // reg offset: 0x01 ; bit: 0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_INT_0X01);

    if (RegValue&(0x01<<0))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

EXPORT_SYMBOL_GPL(CheckI2SOSampCountInt);


static AUD_BOOL CheckI2SOTimingCheckIntStatus(void)
{
    // reg offset: 0x01 ; bit: 5

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_INT_0X01);

    if (RegValue&(0x01<<5))
    {
        AUD_WriteRegDev8(R_AUD_INT_0X01, RegValue);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



AUD_BOOL CheckI2SOSampCountIntStatus(void)
{
    // reg offset: 0x01 ; bit: 4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_INT_0X01);

    if (RegValue&(0x01<<4))
    {
        AUD_WriteRegDev8(R_AUD_INT_0X01, RegValue);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

EXPORT_SYMBOL_GPL(CheckI2SOSampCountIntStatus);


static AUD_BOOL CheckI2SODMABufResumeInt(void)
{
    // reg offset: 0x01 ; bit: 3

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_INT_0X01);

    if (RegValue&(0x01<<3))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL CheckI2SODMABufResumeIntStatus(void)
{
    // reg offset: 0x01 ; bit: 3

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_INT_0X01);

    if (RegValue&(0x01<<3))
    {
        AUD_WriteRegDev8(R_AUD_INT_0X01, \
                         (RegValue | (0x01<<3)));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL CheckI2SODMAUnderRunInt(void)
{
    /* Dummy function */
    return FALSE;
}



AUD_BOOL CheckI2SODMABufUnderRunIntStatus(void)
{
    // reg offset: 0x14 ; bit: 0
    AUD_UINT8 status;
    AUD_UINT8 RegValue = 0;
    status=AUD_ReadRegDev8(R_AUD_INT_0X01);
    RegValue = AUD_ReadRegDev8(R_BUF_UNDER_RUN_CTRL_0X14);
    
    // under run resume int: bit 3:enable; bit7: status
    if((status&0x08)&&(0x80&status))//under run occured //maybe error
      {
            // write 1 (bit 7) to clear
        AUD_WriteRegDev8(R_AUD_INT_0X01, status|0x80);

        // write 1 (bit 0) to clear
        AUD_WriteRegDev8(R_BUF_UNDER_RUN_CTRL_0X14, RegValue|0x01);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

EXPORT_SYMBOL_GPL(CheckI2SODMABufUnderRunIntStatus);


/* -----------------------------------------------------------------------------
SPO Hardware Register Control Functions
----------------------------------------------------------------------------- */

/* Basic setting ------------------------------------------------------------ */

static void SetSPODMABufBaseAddr(AUD_UINT32 Addr)
{
    // reg offset: 0x60 ; bit: 31-0

    AUD_WriteRegDev32(R_SPO_DMA_BASE_ADDR_0X60, \
                      (Addr&0x0FFFFFFF));
}



static void SetSPODMABufLength(AUD_UINT16 Length)
{
    // reg offset: 0x66 ; bit: 15-0

    AUD_WriteRegDev16(R_SPO_DMA_LEN_0X66, \
                      (Length&0xFFFF));
}



static void SetSPODMABufLastIndex(AUD_UINT16 Index)
{
    // reg offset: 0x68 ; bit: 15-0

    AUD_WriteRegDev16(R_SPO_DMA_LAST_INDEX_0X68, \
                      (Index&0xFFFF));
}



static void SetSPOFramePCInfo(AUD_UINT16 PCInfo)
{
    // reg offset: 0x58 ; bit: 15-0

    AUD_WriteRegDev16(R_SPO_IEC_PC_0X58, \
                      (PCInfo&0xFFFF));
}



/* SPO interface confingure relative operation ------------------------------ */

static void SetSPOSampCountThreshold(AUD_UINT16 Threshold)
{
    // reg offset: 0x22 ; bit: 15-0

    AUD_WriteRegDev16(R_SPO_SAMP_COUNTER_TRHD_0X22, \
                      (Threshold&0xFFFF));
}



/* SPIDF Output DMA config start --> */

static void EnableSPODMADataWithHeader(AUD_BOOL Enable)
{
    // reg offset: 0x21 ; bit: 3

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_SPO_IEC_CTRL_0X21);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_SPO_IEC_CTRL_0X21, \
                         (RegValue&(~(0x01<<3))));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_SPO_IEC_CTRL_0X21, \
                         ((RegValue&(~(0x01<<3))) | (0x01<<3)));
    }
    else
    {
        SDBBP();
    }
}



static void SetSPODMADataLen(SPO_DMA_DataLen_t Len)
{
    // reg offset: 0x20 ; bit: 5

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_SPO_CTRL_0X20);

    if (DATA_LEN_16_BITS == Len)
    {
        AUD_WriteRegDev8(R_SPO_CTRL_0X20, \
                         (RegValue&(~(0x01<<5))));
    }
    else if (DATA_LEN_24_BITS == Len)
    {
        AUD_WriteRegDev8(R_SPO_CTRL_0X20, \
                         ((RegValue&(~(0x01<<5))) | (0x01<<5)));
    }
    else
    {
        SDBBP();
    }
}


/* <-- SPIDF Output DMA config end */



/* SPDIF output left channel status */
static void SetSPOLeftChanClkAccurary(AUD_UINT8 Accuray)
{
    // reg offset: 0x24 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x03<<28))) | ((Accuray&0x03)<<28)));
}



static AUD_UINT8 GetSPOLeftChanClkAccurary(void)
{
    // reg offset: 0x24 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x03<<28))>>28);
}



static void SetSPOLeftChanSampRate(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x24 ; bit: 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT8 FS = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    switch (SampleRate)
    {
        case SAMPLE_RATE_22050:
        {
            FS = 0x04;
            break;
        }

        case SAMPLE_RATE_44100:
        {
            FS = 0x00;
            break;
        }

        case SAMPLE_RATE_88200:
        {
            FS = 0x08;
            break;
        }

        case SAMPLE_RATE_176400:
        {
            FS = 0x0C;
            break;
        }

        case SAMPLE_RATE_24000:
        {
            FS = 0x06;
            break;
        }

        case SAMPLE_RATE_48000:
        {
            FS = 0x02;
            break;
        }

        case SAMPLE_RATE_96000:
        {
            FS = 0x0A;
            break;
        }

        case SAMPLE_RATE_192000:
        {
            FS = 0x0E;
            break;
        }

        case SAMPLE_RATE_32000:
        {
            FS = 0x03;
            break;
        }

        case SAMPLE_RATE_768000:
        {
            FS = 0x09;
            break;
        }

        default:
        {
            FS = 0x00;
            break;
        }
    }
    
    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x0F<<24))) | ((FS&0x0F)<<24)));
}



static AUD_SampleRate_t GetSPOLeftChanSampRate(void)
{
    // reg offset: 0x24 ; bit: 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT8 FS = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    FS = (AUD_UINT8)((RegValue&(0x0F<<24))>>24);

    if (0x04 == FS)
    {
        return SAMPLE_RATE_22050;
    }
    else if (0x00 == FS)
    {
        return SAMPLE_RATE_44100;
    }
    else if (0x08 == FS)
    {
        return SAMPLE_RATE_88200;
    }
    else if (0x0C == FS)
    {
        return SAMPLE_RATE_176400;
    }
    else if (0x06 == FS)
    {
        return SAMPLE_RATE_24000;
    }
    else if (0x02 == FS)
    {
        return SAMPLE_RATE_48000;
    }
    else if (0x0A == FS)
    {
        return SAMPLE_RATE_96000;
    }
    else if (0x0E == FS)
    {
        return SAMPLE_RATE_192000;
    }
    else if (0x03 == FS)
    {
        return SAMPLE_RATE_32000;
    }
    else if (0x09 == FS)
    {
        return SAMPLE_RATE_768000;
    }
}



static void SetSPOLeftChanNum(AUD_UINT8 ChannelNum)
{
    // reg offset: 0x24 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x0F<<20))) | ((ChannelNum&0x0F)<<20)));
}



static AUD_UINT8 GetSPOLeftChanNum(void)
{
    // reg offset: 0x24 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x0F<<20))>>20);
}



static void SetSPOLeftChanSourcelNum(AUD_UINT8 SourcelNum)
{
    // reg offset: 0x24 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x0F<<16))) | ((SourcelNum&0x0F)<<16)));
}



static AUD_UINT8 GetSPOLeftChanSourcelNum(void)
{
    // reg offset: 0x24 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x0F<<16))>>16);
}



static void SetSPOLeftChanLBit(AUD_UINT8 LBit)
{
    // reg offset: 0x24 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x01<<15))) | ((LBit&0x01)<<15)));
}



static AUD_UINT8 GetSPOLeftChanLBit(void)
{
    // reg offset: 0x24 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x01<<15))>>15);
}



static void SetSPOLeftChanCategory(AUD_UINT8 Category)
{
    // reg offset: 0x24 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x7F<<8))) | ((Category&0x7F)<<8)));
}



static AUD_UINT16 GetSPOLeftChanCategory(void)
{
    // reg offset: 0x24 ; bit: 15-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT16)((RegValue&(0xFF<<8))>>8);
}



static void SetSPOLeftChanStatusMode(AUD_UINT8 Mode)
{
    // reg offset: 0x24 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x03<<6))) | ((Mode&0x03)<<6)));
}



static AUD_UINT8 GetSPOLeftChanStatusMode(void)
{
    // reg offset: 0x24 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x03<<6))>>6);
}



static void SetSPOLeftChanAddFormatInfo(AUD_UINT8 AddFormatInfo)
{
    // reg offset: 0x24 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x07<<3))) | ((AddFormatInfo&0x07)<<3)));
}



static AUD_UINT8 GetSPOLeftChanAddFormatInfo(void)
{
    // reg offset: 0x24 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x07<<3))>>3);
}



static void SetSPOLeftChanCopyright(AUD_UINT8 Copyright)
{
    // reg offset: 0x24 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x01<<2))) | ((Copyright&0x01)<<2)));
}



static AUD_UINT8 GetSPOLeftChanCopyright(void)
{
    // reg offset: 0x24 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x01<<2))>>2);
}



static void SetSPOLeftChanAudContent(SPO_AudioContentFlag_t AudContent)
{
    // reg offset: 0x24 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x01<<1))) | ((AudContent&0x01)<<1)));
}



static AUD_UINT8 GetSPOLeftChanAudContent(void)
{
    // reg offset: 0x24 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x01<<1))>>1);
}



static void SetSPOLeftChanUseFlag(AUD_UINT8 UseFlag)
{
    // reg offset: 0x24 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~0x01)) | (UseFlag&0x01)));
}



static AUD_UINT8 GetSPOLeftChanUseFlag(void)
{
    // reg offset: 0x24 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)(RegValue&0x01);
}



static void ConfigSPOLeftChanUserData(AUD_UINT32 *DataAddr)
{
    // reg offset: 0x28 ; bit: 192-0

    AUD_UINT8 i = 0;

    for (i=0; i<6; i++)
    {
        AUD_WriteRegDev32((R_SPO_LEFT_USER_DATA_0X28 + i*4), \
                          DataAddr[i]);
    }
}



// C3701C use the same reg as L/R Channel status.

static void SetSPORightChanClkAccurary(AUD_UINT8 Accuray)
{
    // reg offset: 0x24 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x03<<28))) | ((Accuray&0x03)<<28)));
}



static AUD_UINT8 GetSPORightChanClkAccurary(void)
{
    // reg offset: 0x24 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x03<<28))>>28);
}



static void SetSPORightChanSampRate(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x24 ; bit: 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT8 FS = 0;
    
    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    switch (SampleRate)
    {
        case SAMPLE_RATE_22050:
        {
            FS = 0x04;
            break;
        }

        case SAMPLE_RATE_44100:
        {
            FS = 0x00;
            break;
        }

        case SAMPLE_RATE_88200:
        {
            FS = 0x08;
            break;
        }

        case SAMPLE_RATE_176400:
        {
            FS = 0x0C;
            break;
        }

        case SAMPLE_RATE_24000:
        {
            FS = 0x06;
            break;
        }

        case SAMPLE_RATE_48000:
        {
            FS = 0x02;
            break;
        }

        case SAMPLE_RATE_96000:
        {
            FS = 0x0A;
            break;
        }

        case SAMPLE_RATE_192000:
        {
            FS = 0x0E;
            break;
        }

        case SAMPLE_RATE_32000:
        {
            FS = 0x03;
            break;
        }

        case SAMPLE_RATE_768000:
        {
            FS = 0x09;
            break;
        }

        default:
        {
            FS = 0x00;
            break;
        }
    }

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x0F<<24))) | ((FS&0x0F)<<24)));
}



static AUD_SampleRate_t GetSPORightChanSampRate(void)
{
    // reg offset: 0x24 ; bit: 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT8 FS = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    FS = (AUD_UINT8)((RegValue&(0x0F<<24))>>24);

    if (0x04 == FS)
    {
        return SAMPLE_RATE_22050;
    }
    else if (0x00 == FS)
    {
        return SAMPLE_RATE_44100;
    }
    else if (0x08 == FS)
    {
        return SAMPLE_RATE_88200;
    }
    else if (0x0C == FS)
    {
        return SAMPLE_RATE_176400;
    }
    else if (0x06 == FS)
    {
        return SAMPLE_RATE_24000;
    }
    else if (0x02 == FS)
    {
        return SAMPLE_RATE_48000;
    }
    else if (0x0A == FS)
    {
        return SAMPLE_RATE_96000;
    }
    else if (0x0E == FS)
    {
        return SAMPLE_RATE_192000;
    }
    else if (0x03 == FS)
    {
        return SAMPLE_RATE_32000;
    }
    else if (0x09 == FS)
    {
        return SAMPLE_RATE_768000;
    }
}



static void SetSPORightChanNum(AUD_UINT8 ChannelNum)
{
    // reg offset: 0x24 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x0F<<20))) | ((ChannelNum&0x0F)<<20)));
}



static AUD_UINT8 GetSPORightChanNum(void)
{
    // reg offset: 0x24 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x0F<<20))>>20);
}



static void SetSPORightChanSourcelNum(AUD_UINT8 SourcelNum)
{
    // reg offset: 0x24 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x0F<<16))) | ((SourcelNum&0x0F)<<16)));
}



static AUD_UINT8 GetSPORightChanSourcelNum(void)
{
    // reg offset: 0x24 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x0F<<16))>>16);
}



static void SetSPORightChanLBit(AUD_UINT8 LBit)
{
    // reg offset: 0x24 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x01<<15))) | ((LBit&0x01)<<15)));
}



static AUD_UINT8 GetSPORightChanLBit(void)
{
    // reg offset: 0x24 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x01<<15))>>15);
}



static void SetSPORightChanCategory(AUD_UINT8 Category)
{
    // reg offset: 0x24 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x7F<<8))) | ((Category&0x7F)<<8)));
}



static AUD_UINT16 GetSPORightChanCategory(void)
{
    // reg offset: 0x24 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT16)((RegValue&(0x7F<<8))>>8);
}



static void SetSPORightChanStatusMode(AUD_UINT8 Mode)
{
    // reg offset: 0x24 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x03<<6))) | ((Mode&0x03)<<6)));
}



static AUD_UINT8 GetSPORightChanStatusMode(void)
{
    // reg offset: 0x24 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x03<<6))>>6);
}



static void SetSPORightChanAddFormatInfo(AUD_UINT8 AddFormatInfo)
{
    // reg offset: 0x24 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x07<<3))) | ((AddFormatInfo&0x07)<<3)));
}



static AUD_UINT8 GetSPORightChanAddFormatInfo(void)
{
    // reg offset: 0x24 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x07<<3))>>3);
}



static void SetSPORightChanCopyright(AUD_UINT8 Copyright)
{
    // reg offset: 0x24 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x01<<2))) | ((Copyright&0x01)<<2)));
}



static AUD_UINT8 GetSPORightChanCopyright(void)
{
    // reg offset: 0x24 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x01<<2))>>2);
}



static void SetSPORightChanAudContent(SPO_AudioContentFlag_t AudContent)
{
    // reg offset: 0x24 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~(0x01<<1))) | ((AudContent&0x01)<<1)));
}



static AUD_UINT8 GetSPORightChanAudContent(void)
{
    // reg offset: 0x24 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)((RegValue&(0x01<<1))>>1);
}



static void SetSPORightChanUseFlag(AUD_UINT8 UseFlag)
{
    // reg offset: 0x24 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    AUD_WriteRegDev32(R_SPO_CS_0X24, \
                      ((RegValue&(~0x01)) | (UseFlag&0x01)));
}



static AUD_UINT8 GetSPORightChanUseFlag(void)
{
    // reg offset: 0x24 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CS_0X24);

    return (AUD_UINT8)(RegValue&0x01);
}



static void ConfigSPORightChanUserData(AUD_UINT32 *DataAddr)
{
    // reg offset: 0x40 ; bit: 192-0

    AUD_UINT8 i = 0;

    for (i=0; i<6; i++)
    {
        AUD_WriteRegDev32((R_SPO_RIGHT_USER_DATA_0X40 + i*4), \
                          DataAddr[i]);
    }
}



/* SPDIF Output clock config start --> */

static void SelectSPOMCLKMode(SPO_MCLK_DivMode_t Mode)
{
    // reg offset: 0x20 ; bit: 7-6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_SPO_CTRL_0X20);

    AUD_WriteRegDev8(R_SPO_CTRL_0X20, \
                     ((RegValue&(~(0x03<<6))) | ((Mode&0x03)<<6)));
}



static void EnableSPOFSSoftwareConfig(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}




static void EnableSPOFSGated(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void SetSPOMCLKDivBySoftware(SPO_CLK_SW_DivMode_t FSDiv)
{
    // reg: 0x20 ; bit: 1-0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_SPO_CTRL_0X20);

    switch (FSDiv)
    {
        case SPO_CLK_SW_DIV_1:
        {
            AUD_WriteRegDev8(R_SPO_CTRL_0X20, \
                             ((RegValue&(~(0x03<<0))) | ((0&0x03)<<0)));
            break;
        }

        case SPO_CLK_SW_DIV_2:
        {
            AUD_WriteRegDev8(R_SPO_CTRL_0X20, \
                             ((RegValue&(~(0x03<<0))) | ((1&0x03)<<0)));
            break;
        }

        case SPO_CLK_SW_DIV_3:
        {
            AUD_WriteRegDev8(R_SPO_CTRL_0X20, \
                             ((RegValue&(~(0x03<<0))) | ((2&0x03)<<0)));
            break;
        }

        case SPO_CLK_SW_DIV_6:
        {
            AUD_WriteRegDev8(R_SPO_CTRL_0X20, \
                             ((RegValue&(~(0x03<<0))) | ((3&0x03)<<0)));
            break;
        }

        default:
        {
            break;
        }
    }
}

/* <-- SPDIF Output clock config end */



static void EnableSPOBitStreamMode(AUD_BOOL Enable)
{
    // reg offset: 0x21 ; bit: 4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_SPO_IEC_CTRL_0X21);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_SPO_IEC_CTRL_0X21, \
                         ((RegValue&(~(0x01<<4))) | (0x01<<4)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_SPO_IEC_CTRL_0X21, \
                         (RegValue&(~(0x01<<4))));      
    }
    else
    {
        SDBBP();
    }
}



static void EnableSPODataReorder(AUD_BOOL Enable)
{
    // reg offset: 0x20 ; bit: 3

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_SPO_CTRL_0X20);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_SPO_CTRL_0X20, \
                         ((RegValue&(~(0x01<<3))) | (0x01<<3)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_SPO_CTRL_0X20, \
                         (RegValue&(~(0x01<<3))));      
    }
    else
    {
        SDBBP();
    }
}



static void EnableSPOValidBit(AUD_BOOL Enable)
{
    // reg offset: 0x20 ; bit: 4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_SPO_CTRL_0X20);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_SPO_CTRL_0X20, \
                         ((RegValue&(~(0x01<<4))) | (0x01<<4)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_SPO_CTRL_0X20, \
                         (RegValue&(~(0x01<<4))));      
    }
    else
    {
        SDBBP();
    }
}



static void EnableSPONormalPlay(AUD_BOOL Enable)
{
    // reg offset: 0x21 ; bit: 5

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_SPO_IEC_CTRL_0X21);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_SPO_IEC_CTRL_0X21, \
                         ((RegValue&(~(0x01<<5))) | (0x01<<5)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_SPO_IEC_CTRL_0X21, \
                         (RegValue&(~(0x01<<5))));      
    }
    else
    {
        SDBBP();
    }
}



static void SelectSPOChannelStatusReg(SPO_ChanStatusReg_t Mode)
{
    // reg offset: 0x20 ; bit: 2

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_SPO_CTRL_0X20);

    AUD_WriteRegDev8(R_SPO_CTRL_0X20, \
                     (RegValue ^ (Mode<<2)));        
}



static void SelectSPODataSource(SPO_DataSource_t Source)
{
    // reg offset: 0x21 ; bit: 2

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_SPO_IEC_CTRL_0X21);

    if (FROM_SPO_DMA == Source)
    {
        AUD_WriteRegDev8(R_SPO_IEC_CTRL_0X21, \
                         ((RegValue&(~(0x01<<2))) | (0x01<<2)));        
    }
    else if (FROM_I2SO_DMA == Source)
    {
        AUD_WriteRegDev8(R_SPO_IEC_CTRL_0X21, \
                         (RegValue&(~(0x01<<2))));      
    }
    else
    {
        SDBBP();
    }
}




static void SelectSPOPCMOutChan(SPO_PCM_OutChanMode_t Chan)
{
    /* Dummy function */
    return;
}



static void SelectSPOPCMOutLFEMode(SPO_PCM_OutLFEMode_t Mode)
{
    /* Dummy function */
    return;
}



static void ConfigSPOFSBySoftware(AUD_UINT32 SampleRate)
{
    /* Dummy function */
    return;
}



static void ConfigF128FSBySoftware(AUD_UINT32 SampleRate)
{
    /* Dummy function */
    return;
}



static void ConfigSPOFSByHardware(AUD_UINT32 SampleRate)
{
    /* Dummy function */
    return;
}



static void SelectF128LRCLKDivMode(SPO_LRCLK_DivMode_t DivMode)
{
    // reg: 0x21 ; bit: 1-0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_SPO_IEC_CTRL_0X21);

    switch (DivMode)
    {
        case SPO_LRCLK_DIV_1:
        {
            AUD_WriteRegDev8(R_SPO_IEC_CTRL_0X21, \
                             ((RegValue&(~(0x03<<0))) | ((0&0x03)<<0)));
            break;
        }

        case SPO_LRCLK_DIV_2:
        {
            AUD_WriteRegDev8(R_SPO_IEC_CTRL_0X21, \
                             ((RegValue&(~(0x03<<0))) | ((1&0x03)<<0)));
            break;
        }

        case SPO_LRCLK_DIV_3:
        {
            AUD_WriteRegDev8(R_SPO_IEC_CTRL_0X21, \
                             ((RegValue&(~(0x03<<0))) | ((2&0x03)<<0)));
            break;
        }

        case SPO_LRCLK_DIV_6:
        {
            AUD_WriteRegDev8(R_SPO_IEC_CTRL_0X21, \
                             ((RegValue&(~(0x03<<0))) | ((3&0x03)<<0)));
            break;
        }

        default:
        {
            break;
        }
    }
}



static void SelectF128FSMode(AUD_F128_FS_Mode_t Mode)
{
    return;
}



void EnableSPOInterface(AUD_BOOL Enable)
{
    // reg offset: 0x00 ; bit: 1

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_CTRL_0X00);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_CTRL_0X00, \
                         ((RegValue&(~(0x01<<1))) | (0x01<<1)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_CTRL_0X00, \
                         (RegValue&(~(0x01<<1))));
    }
    else
    {
        SDBBP();
    }
}

EXPORT_SYMBOL_GPL(EnableSPOInterface);

static AUD_BOOL CheckSPOInfEnableStatus(void)
{
    // reg offset: 0x00 ; bit: 1

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_CTRL_0X00);

    if (RegValue&(0x01<<1))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static void EnableSPOClock(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



/* SPO DMA control --------------------------------------------------------- */

void StartSPODMA(AUD_BOOL Enable)
{
    // reg offset: 0x15 ; bit: 0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DMA_CTRL_0X15);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DMA_CTRL_0X15, \
                         ((RegValue&(~(0x01<<0))) | (0x01<<0)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DMA_CTRL_0X15, \
                         (RegValue&(~(0x01<<0))));      
    }
    else
    {
        SDBBP();
    }
}

EXPORT_SYMBOL_GPL(StartSPODMA);

/* Get SPO interface information -------------------------------------------- */

static AUD_UINT16 GetSPODMABufLastIndex(void)
{
    // reg offset: 0x68; bit: 15-0

    return AUD_ReadRegDev16(R_SPO_DMA_LAST_INDEX_0X68);
}



static AUD_UINT16 GetSPODMABufCurrentIndex(void)
{
    // reg offset: 0x64 ; bit: 15-0

    return AUD_ReadRegDev16(R_SPO_DMA_CUR_INDEX_0X64);
}



static AUD_UINT16 GetSPOSampCount(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetSPOFrameLenInfo(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT16 GetSPOFramePDInfo(void)
{
    // reg offset: 0x5A ; bit: 15-0

    return AUD_ReadRegDev16(R_SPO_IEC_PD_0X5A);
}



static AUD_UINT16 GetSPOFrameNullDataInfo(void)
{
    // reg offset: 0x5C ; bit: 15-0

    return AUD_ReadRegDev16(R_SPO_IEC_NULL_0X5C);
}



static AUD_UINT16 GetSPOFrameBurstDataInfo(void)
{
    // reg offset: 0x5E ; bit: 15-0

    return AUD_ReadRegDev16(R_SPO_IEC_DATA_0X5E);
}



static AUD_BOOL GetSPOBufUnderRunStatus(void)
{
    // reg offset: 0x14 ; bit: 6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_BUF_UNDER_RUN_CTRL_0X14);

    if (RegValue&(1<<6))
    {
        AUD_WriteRegDev8(R_BUF_UNDER_RUN_CTRL_0X14, RegValue);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL GetSPOFIFOUnderRunStatus(void)
{
    /* Dummy function */
    return FALSE;
}



static AUD_UINT16 GetSPODMASkipNum(void)
{
    /* Dummy function */
    return 0;
}



/* Enable SPO interrupt ----------------------------------------------------- */

static void EnableSPOSampCountInt(AUD_BOOL Enable)
{
    // reg offset: 0x01 ; bit: 2

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_INT_0X01);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_INT_0X01, \
                         ((RegValue&(~(0x01<<2))) | (0x01<<2)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_INT_0X01, \
                         (RegValue&(~(0x01<<2))));
    }
    else
    {
        SDBBP();
    }
}



static void EnableSPODMABufResumeInt(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



void EnableSPODMAUnderRunInt(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}
EXPORT_SYMBOL_GPL(EnableSPODMAUnderRunInt);

    
/* Get SPO interrupt status ------------------------------------------------ */

AUD_BOOL CheckSPOSampCountInt(void)
{
    // reg offset: 0x01 ; bit: 2

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_INT_0X01);

    if (RegValue&(0x01<<2))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

EXPORT_SYMBOL_GPL(CheckSPOSampCountInt);


AUD_BOOL CheckSPOSampCountIntStatus(void)
{
    // reg offset: 0x01 ; bit: 6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_INT_0X01);

    if (RegValue&(0x01<<6))
    {
        AUD_WriteRegDev8(R_AUD_INT_0X01, RegValue);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

EXPORT_SYMBOL_GPL(CheckSPOSampCountIntStatus);


static AUD_BOOL CheckSPODMABufResumeInt(void)
{
    /* Dummy function */
    return FALSE;
}



static AUD_BOOL CheckSPODMABufResumeIntStatus(void)
{
    /* Dummy function */
    return FALSE;
}



static AUD_BOOL CheckSPODMAUnderRunInt(void)
{
    /* Dummy function */
    return FALSE;
}



AUD_BOOL CheckSPODMABufUnderRunIntStatus(void)
{
    // reg offset: 0x14 ; bit: 6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_BUF_UNDER_RUN_CTRL_0X14);

    if (RegValue&(0x01<<6))
    {
        AUD_WriteRegDev8(R_BUF_UNDER_RUN_CTRL_0X14, RegValue);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

EXPORT_SYMBOL_GPL(CheckSPODMABufUnderRunIntStatus);



/* -----------------------------------------------------------------------------
DDP SPO Hardware Register Control Functions
----------------------------------------------------------------------------- */

/* Basic setting ------------------------------------------------------------ */

static void SetDDPSPODMABufBaseAddr(AUD_UINT32 Addr)
{
    // reg offset: 0x154 ; bit: 31-0

    AUD_WriteRegDev32(R_DDP_SPO_DMA_BASE_ADDR_0X154, \
                      (Addr&0x0FFFFFFF));
}



static void SetDDPSPODMABufLength(AUD_UINT16 Length)
{
    // reg offset: 0x158 ; bit: 15-0
    
    AUD_WriteRegDev16(R_DDP_SPO_DMA_LEN_0X158, \
                      (Length&0xFFFF));
}



static void SetDDPSPODMABufLastIndex(AUD_UINT16 Index)
{
    // reg offset: 0x15E ; bit: 15-0

    AUD_WriteRegDev16(R_DDP_SPO_LAST_INDEX_0X15E, \
                      (Index&0xFFFF));
}



static void SetDDPSPOFramePCInfo(AUD_UINT16 PCInfo)
{
    // reg offset: 0x168 ; bit: 15-0

    AUD_WriteRegDev16(R_DDP_SPO_IEC_PC_0X168, \
                      (PCInfo&0xFFFF));
}



/* DDP SPO interface confingure relative operation -------------------------- */

static void SetDDPSPOSampCountThreshold(AUD_UINT16 Threshold)
{
    // reg offset: 0x15C ; bit: 15-0

    AUD_WriteRegDev16(R_DDP_SPO_SAMP_COUNTER_TRHD_0X15C, \
                      (Threshold&0xFFFF));
}



/* DDPlus SPIDF Output DMA config start --> */

static void EnableDDPSPODMADataWithHeader(AUD_BOOL Enable)
{
    // reg offset: 0x162 ; bit: 7

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_CTRL_0X162);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                         (RegValue&(~(0x01<<7))));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                         ((RegValue&(~(0x01<<7))) | (0x01<<7)));
    }
    else
    {
        SDBBP();
    }
}



static void SetDDPSPODMADataLen(DDPSPO_DMA_DataLen_t Len)
{
    // reg offset: 0x162 ; bit: 0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_CTRL_0X162);

    if (DDP_DATA_LEN_16_BITS == Len)
    {
        AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                         (RegValue&(~(0x01<<0))));
    }
    else if (DDP_DATA_LEN_24_BITS == Len)
    {
        AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                         ((RegValue&(~(0x01<<0))) | (0x01<<0)));
    }
    else
    {
        SDBBP();
    }
}

/* <-- DDPlus SPIDF Output DMA config end */



static void SetDDPSPOLeftChanClkAccurary(AUD_UINT8 Accuray)
{
    // reg offset: 0x150 ; bit: 29-28
 
    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x03<<28))) | ((Accuray&0x03)<<28)));
}



static AUD_UINT8 GetDDPSPOLeftChanClkAccurary(void)
{
    // reg offset: 0x150 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x03<<28))>>28);
}



static void SetDDPSPOLeftChanSampRate(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x150 ; bit: 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT8 FS = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    switch (SampleRate)
    {
        case SAMPLE_RATE_22050:
        {
            FS = 0x04;
            break;
        }

        case SAMPLE_RATE_44100:
        {
            FS = 0x00;
            break;
        }

        case SAMPLE_RATE_88200:
        {
            FS = 0x08;
            break;
        }

        case SAMPLE_RATE_176400:
        {
            FS = 0x0C;
            break;
        }

        case SAMPLE_RATE_24000:
        {
            FS = 0x06;
            break;
        }

        case SAMPLE_RATE_48000:
        {
            FS = 0x02;
            break;
        }

        case SAMPLE_RATE_96000:
        {
            FS = 0x0A;
            break;
        }

        case SAMPLE_RATE_192000:
        {
            FS = 0x0E;
            break;
        }

        case SAMPLE_RATE_32000:
        {
            FS = 0x03;
            break;
        }

        case SAMPLE_RATE_768000:
        {
            FS = 0x09;
            break;
        }

        default:
        {
            FS = 0x00;
            break;
        }
    }
    
    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x0F<<24))) | ((FS&0x0F)<<24)));
}



static AUD_SampleRate_t GetDDPSPOLeftChanSampRate(void)
{
    // reg offset: 0x150 ; bit: 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT8 FS = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    FS = (AUD_UINT8)((RegValue&(0x0F<<24))>>24);

    if (0x04 == FS)
    {
        return SAMPLE_RATE_22050;
    }
    else if (0x00 == FS)
    {
        return SAMPLE_RATE_44100;
    }
    else if (0x08 == FS)
    {
        return SAMPLE_RATE_88200;
    }
    else if (0x0C == FS)
    {
        return SAMPLE_RATE_176400;
    }
    else if (0x06 == FS)
    {
        return SAMPLE_RATE_24000;
    }
    else if (0x02 == FS)
    {
        return SAMPLE_RATE_48000;
    }
    else if (0x0A == FS)
    {
        return SAMPLE_RATE_96000;
    }
    else if (0x0E == FS)
    {
        return SAMPLE_RATE_192000;
    }
    else if (0x03 == FS)
    {
        return SAMPLE_RATE_32000;
    }
    else if (0x09 == FS)
    {
        return SAMPLE_RATE_768000;
    }
}



static void SetDDPSPOLeftChanNum(AUD_UINT8 ChannelNum)
{
    // reg offset: 0x150 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x0F<<20))) | ((ChannelNum&0x0F)<<20)));
}



static AUD_UINT8 GetDDPSPOLeftChanNum(void)
{
    // reg offset: 0x150 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x0F<<20))>>20);
}



static void SetDDPSPOLeftChanSourcelNum(AUD_UINT8 SourcelNum)
{
    // reg offset: 0x150 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x0F<<16))) | ((SourcelNum&0x0F)<<16)));
}



static AUD_UINT8 GetDDPSPOLeftChanSourcelNum(void)
{
    // reg offset: 0x150 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x0F<<16))>>16);
}



static void SetDDPSPOLeftChanLBit(AUD_UINT8 LBit)
{
    // reg offset: 0x150 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x01<<15))) | ((LBit&0x01)<<15)));
}



static AUD_UINT8 GetDDPSPOLeftChanLBit(void)
{
    // reg offset: 0x150 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x01<<15))>>15);
}



static void SetDDPSPOLeftChanCategory(AUD_UINT8 Category)
{
    // reg offset: 0x150 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x7F<<8))) | ((Category&0x7F)<<8)));
}



static AUD_UINT16 GetDDPSPOLeftChanCategory(void)
{
    // reg offset: 0x150 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT16)((RegValue&(0x7F<<8))>>8);
}



static void SetDDPSPOLeftChanStatusMode(AUD_UINT8 Mode)
{
    // reg offset: 0x150 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x03<<6))) | ((Mode&0x03)<<6)));
}



static AUD_UINT8 GetDDPSPOLeftChanStatusMode(void)
{
    // reg offset: 0x150 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x03<<6))>>6);
}



static void SetDDPSPOLeftChanAddFormatInfo(AUD_UINT8 AddFormatInfo)
{
    // reg offset: 0x150 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x07<<3))) | ((AddFormatInfo&0x07)<<3)));
}



static AUD_UINT8 GetDDPSPOLeftChanAddFormatInfo(void)
{
    // reg offset: 0x150 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x07<<3))>>3);
}



static void EnableDDPSPOLeftChanCopyright(AUD_BOOL Enable)
{
    // reg offset: 0x150 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                          ((RegValue&(~(0x01<<2))) | (0x01<<2)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                          ((RegValue&(~(0x01<<2))) | (0x00<<2)));
    }
    else
    {
        SDBBP();
    }
}



static AUD_UINT8 GetDDPSPOLeftChanCopyright(void)
{
    // reg offset: 0x150 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x01<<2))>>2);
}



static void SetDDPSPOLeftChanAudContent(DDPSPO_AudioContentFlag_t AudContent)
{
    // reg offset: 0x150 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x01<<1))) | ((AudContent&0x01)<<1)));
}



static AUD_UINT8 GetDDPSPOLeftChanAudContent(void)
{
    // reg offset: 0x150 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x01<<1))>>1);
}



static void SetDDPSPOLeftChanUseFlag(AUD_UINT8 UseFlag)
{
    // reg offset: 0x150 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~0x01)) | (UseFlag&0x01)));
}



static AUD_UINT8 GetDDPSPOLeftChanUseFlag(void)
{
    // reg offset: 0x150 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)(RegValue&0x01);
}



static void ConfigDDPSPOLeftChanUserData(AUD_UINT32 *DataAddr)
{
    // reg offset: 0x120 ; bit: 192-0

    AUD_UINT8 i = 0;

    for (i=0; i<6; i++)
    {
        AUD_WriteRegDev32((R_DDP_SPO_LEFT_USER_DATA_0X120 + i*4), \
                          DataAddr[i]);
    }
}



// C3701C use the same reg as L/R Channel status.

static void SetDDPSPORightChanClkAccurary(AUD_UINT8 Accuray)
{
    // reg offset: 0x150 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x03<<28))) | ((Accuray&0x03)<<28)));
}



static AUD_UINT8 GetDDPSPORightChanClkAccurary(void)
{
    // reg offset: 0x150 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x03<<28))>>28);
}



static void SetDDPSPORightChanSampRate(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x150 ; bit: 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT8 FS = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    switch (SampleRate)
    {
        case SAMPLE_RATE_22050:
        {
            FS = 0x04;
            break;
        }

        case SAMPLE_RATE_44100:
        {
            FS = 0x00;
            break;
        }

        case SAMPLE_RATE_88200:
        {
            FS = 0x08;
            break;
        }

        case SAMPLE_RATE_176400:
        {
            FS = 0x0C;
            break;
        }

        case SAMPLE_RATE_24000:
        {
            FS = 0x06;
            break;
        }

        case SAMPLE_RATE_48000:
        {
            FS = 0x02;
            break;
        }

        case SAMPLE_RATE_96000:
        {
            FS = 0x0A;
            break;
        }

        case SAMPLE_RATE_192000:
        {
            FS = 0x0E;
            break;
        }

        case SAMPLE_RATE_32000:
        {
            FS = 0x03;
            break;
        }

        case SAMPLE_RATE_768000:
        {
            FS = 0x09;
            break;
        }

        default:
        {
            FS = 0x00;
            break;
        }
    }
    
    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x0F<<24))) | ((FS&0x0F)<<24)));
}



static AUD_SampleRate_t GetDDPSPORightChanSampRate(void)
{
    // reg offset: 0x150 ; bit: 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT8 FS = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    FS = (AUD_UINT8)((RegValue&(0x0F<<24))>>24);

    if (0x04 == FS)
    {
        return SAMPLE_RATE_22050;
    }
    else if (0x00 == FS)
    {
        return SAMPLE_RATE_44100;
    }
    else if (0x08 == FS)
    {
        return SAMPLE_RATE_88200;
    }
    else if (0x0C == FS)
    {
        return SAMPLE_RATE_176400;
    }
    else if (0x06 == FS)
    {
        return SAMPLE_RATE_24000;
    }
    else if (0x02 == FS)
    {
        return SAMPLE_RATE_48000;
    }
    else if (0x0A == FS)
    {
        return SAMPLE_RATE_96000;
    }
    else if (0x0E == FS)
    {
        return SAMPLE_RATE_192000;
    }
    else if (0x03 == FS)
    {
        return SAMPLE_RATE_32000;
    }
    else if (0x09 == FS)
    {
        return SAMPLE_RATE_768000;
    }
}



static void SetDDPSPORightChanNum(AUD_UINT8 ChannelNum)
{
    // reg offset: 0x150 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x0F<<20))) | ((ChannelNum&0x0F)<<20)));
}



static AUD_UINT8 GetDDPSPORightChanNum(void)
{
    // reg offset: 0x150 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x0F<<20))>>20);
}



static void SetDDPSPORightChanSourcelNum(AUD_UINT8 SourcelNum)
{
    // reg offset: 0x150 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x0F<<16))) | ((SourcelNum&0x0F)<<16)));
}



static AUD_UINT8 GetDDPSPORightChanSourcelNum(void)
{
    // reg offset: 0x150 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x0F<<16))>>16);
}



static void SetDDPSPORightChanLBit(AUD_UINT8 LBit)
{
    // reg offset: 0x150 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x01<<15))) | ((LBit&0x01)<<15)));
}



static AUD_UINT8 GetDDPSPORightChanLBit(void)
{
    // reg offset: 0x150 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x01<<15))>>15);
}



static void SetDDPSPORightChanCategory(AUD_UINT8 Category)
{
    // reg offset: 0x150 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x7F<<8))) | ((Category&0x7F)<<8)));
}



static AUD_UINT16 GetDDPSPORightChanCategory(void)
{
    // reg offset: 0x150 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT16)((RegValue&(0x7F<<8))>>8);
}



static void SetDDPSPORightChanStatusMode(AUD_UINT8 Mode)
{
    // reg offset: 0x150 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x03<<6))) | ((Mode&0x03)<<6)));
}



static AUD_UINT8 GetDDPSPORightChanStatusMode(void)
{
    // reg offset: 0x150 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x03<<6))>>6);
}



static void SetDDPSPORightChanAddFormatInfo(AUD_UINT8 AddFormatInfo)
{
    // reg offset: 0x150 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x07<<3))) | ((AddFormatInfo&0x07)<<3)));
}



static AUD_UINT8 GetDDPSPORightChanAddFormatInfo(void)
{
    // reg offset: 0x150 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x07<<3))>>3);
}



static void SetDDPSPORightChanCopyright(AUD_UINT8 Copyright)
{
    // reg offset: 0x150 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x01<<2))) | ((Copyright&0x01)<<2)));
}



static AUD_UINT8 GetDDPSPORightChanCopyright(void)
{
    // reg offset: 0x150 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x01<<2))>>2);
}



static void SetDDPSPORightChanAudContent(DDPSPO_AudioContentFlag_t AudContent)
{
    // reg offset: 0x150 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~(0x01<<1))) | ((AudContent&0x01)<<1)));
}



static AUD_UINT8 GetDDPSPORightChanAudContent(void)
{
    // reg offset: 0x150 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)((RegValue&(0x01<<1))>>1);
}



static void SetDDPSPORightChanUseFlag(AUD_UINT8 UseFlag)
{
    // reg offset: 0x150 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    AUD_WriteRegDev32(R_DDP_SPO_CS_0X150, \
                      ((RegValue&(~0x01)) | (UseFlag&0x01)));
}



static AUD_UINT8 GetDDPSPORightChanUseFlag(void)
{
    // reg offset: 0x150 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CS_0X150);

    return (AUD_UINT8)(RegValue&0x01);
}



static void ConfigDDPSPORightChanUserData(AUD_UINT32 *DataAddr)
{
    // reg offset: 0x138 ; bit: 192-0

    AUD_UINT8 i = 0;

    for (i=0; i<6; i++)
    {
        AUD_WriteRegDev32((R_DDP_SPO_RIGHT_USER_DATA_0X138 + i*4), \
                          DataAddr[i]);
    }
}



/* DDPlus SPDIF Output clock config start --> */

static void SelectDDPSPOMCLKDivMode(DDPSPO_MCLK_DivMode_t Mode)
{
    // reg offset: 0x161 ; bit: 3-2

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_IEC_CTRL_0X161);

    AUD_WriteRegDev8(R_DDP_SPO_IEC_CTRL_0X161, \
                     ((RegValue&(~(0x03<<2))) | ((Mode&0x03)<<2)));
}



static void EnableDDPSPOFSSoftwareConfig(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void EnableDDPSPOFSGated(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DivMode_t FSDiv)
{
    // reg: 0x161 ; bit: 1-0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_IEC_CTRL_0X161);

    switch (FSDiv)
    {
        case DDPSPO_CLK_SW_DIV_1:
        {
            AUD_WriteRegDev8(R_DDP_SPO_IEC_CTRL_0X161, \
                             ((RegValue&(~(0x03<<0))) | ((0&0x03)<<0)));
            break;
        }

        case DDPSPO_CLK_SW_DIV_2:
        {
            AUD_WriteRegDev8(R_DDP_SPO_IEC_CTRL_0X161, \
                             ((RegValue&(~(0x03<<0))) | ((1&0x03)<<0)));
            break;
        }

        case DDPSPO_CLK_SW_DIV_3:
        {
            AUD_WriteRegDev8(R_DDP_SPO_IEC_CTRL_0X161, \
                             ((RegValue&(~(0x03<<0))) | ((2&0x03)<<0)));
            break;
        }

        case DDPSPO_CLK_SW_DIV_6:
        {
            AUD_WriteRegDev8(R_DDP_SPO_IEC_CTRL_0X161, \
                             ((RegValue&(~(0x03<<0))) | ((3&0x03)<<0)));
            break;
        }

        default:
        {
            break;
        }
    }
}

/* <-- DDPLUS SPDIF Output clock config end */



static void EnableDDPSPOBitStreamMode(AUD_BOOL Enable)
{
    // reg offset: 0x162 ; bit: 1

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_CTRL_0X162);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                         ((RegValue&(~(0x01<<1))) | (0x01<<1)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                         (RegValue&(~(0x01<<1))));      
    }
    else
    {
        SDBBP();
    }
}



static void EnableDDPSPODataReorder(AUD_BOOL Enable)
{
    // reg offset: 0x162 ; bit: 2

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_CTRL_0X162);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                         ((RegValue&(~(0x01<<2))) | (0x01<<2)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                         (RegValue&(~(0x01<<2))));      
    }
    else
    {
        SDBBP();
    }
}



static void EnableDDPSPOValidBit(AUD_BOOL Enable)
{
    // reg offset: 0x162 ; bit: 5

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_CTRL_0X162);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                         ((RegValue&(~(0x01<<5))) | (0x01<<5)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                         (RegValue&(~(0x01<<5))));      
    }
    else
    {
        SDBBP();
    }
}



static void EnableDDPSPONormalPlay(AUD_BOOL Enable)
{
    // reg offset: 0x162 ; bit: 4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_CTRL_0X162);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                         ((RegValue&(~(0x01<<4))) | (0x01<<4)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                         (RegValue&(~(0x01<<4))));      
    }
    else
    {
        SDBBP();
    }
}



static void SelectDDPSPOChannelStatusReg(DDPSPO_ChanStatusReg_t Mode)
{
    // reg offset: 0x162 ; bit: 3

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_CTRL_0X162);

    AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                     (RegValue ^ (Mode<<3)));
}



static void SelectDDPSPOClkAndDataMode(DDP_SPO_OutputMode_t Mode)
{
    // reg offset: 0x162 ; bit: 6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_CTRL_0X162);

    if (SPO_OUT_CLK_DATA == Mode) /* From SPO clock&data */
    {
        AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                         ((RegValue&(~(0x01<<6))) | (0x01<<6)));        
    }
    else if (DDPSPO_OUT_CLK_DATA == Mode) /* From DDPPLUS SPO clock&data */
    {
        AUD_WriteRegDev8(R_DDP_SPO_CTRL_0X162, \
                         (RegValue&(~(0x01<<6))));      
    }
    else
    {
        SDBBP();
    }
}


static AUD_BOOL CheckDDPSPOClkAndDataMode(void)
{
    // reg offset: 0x162 ; bit: 6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_CTRL_0X162);

    if (RegValue&(0x01<<6))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static void SelectDDPSPODataSource(DDPSPO_DataSource_t Source)
{
    // reg offset: 0x161 ; bit: 6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_IEC_CTRL_0X161);

    if (DDP_FROM_DDPSPO_DMA == Source)
    {
        AUD_WriteRegDev8(R_DDP_SPO_IEC_CTRL_0X161, \
                         ((RegValue&(~(0x01<<6))) | (0x01<<6)));        
    }
    else if (DDP_FROM_I2SO_DMA == Source)
    {
        AUD_WriteRegDev8(R_DDP_SPO_IEC_CTRL_0X161, \
                         (RegValue&(~(0x01<<6))));      
    }
    else
    {
        SDBBP();
    }
}



static void SelectDDPSPOPCMOutChan(DDPSPO_PCM_OutChanMode_t Mode)
{
    /* Dummy function */
    return;
}



static void SelectDDPSPOPCMOutLFEMode(DDPSPO_PCM_OutLFEMode_t Mode)
{
    /* Dummy function */
    return;
}



static void ConfigDDPSPOFSBySoftware(AUD_SampleRate_t SampleRate)
{
    /* Dummy function */
    return;
}



static void ConfigDDPSPOFSByHardware(AUD_SampleRate_t SampleRate)
{
    /* Dummy function */
    return;
}



static void EnableDDPSPOInterface(AUD_BOOL Enable)
{
    // reg offset: 0x00 ; bit: 7

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_CTRL_0X00);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_CTRL_0X00, \
                         ((RegValue&(~(0x01<<7))) | (0x01<<7)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_AUD_CTRL_0X00, \
                         (RegValue&(~(0x01<<7))));
    }
    else
    {
        SDBBP();
    }
}



static AUD_BOOL CheckDDPSPOInfEnableStatus(void)
{
    // reg offset: 0x00 ; bit: 7

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_CTRL_0X00);

    if (RegValue&(0x01<<7))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static void EnableDDPSPOClock(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



/* DDP SPO DMA control ------------------------------------------------------ */

static void StartDDPSPODMA(AUD_BOOL Enable)
{
    // reg offset: 0x15 ; bit: 1

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DMA_CTRL_0X15);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DMA_CTRL_0X15, \
                         ((RegValue&(~(0x01<<1))) | (0x01<<1)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DMA_CTRL_0X15, \
                         (RegValue&(~(0x01<<1))));      
    }
    else
    {
        SDBBP();
    }
}



/* Get DDP SPO interface information ---------------------------------------- */

static AUD_UINT16 GetDDPSPODMABufLastIndex(void)
{
    // reg offset: 0x15E; bit: 15-0

    return AUD_ReadRegDev16(R_DDP_SPO_LAST_INDEX_0X15E);
}



static AUD_UINT16 GetDDPSPODMABufCurrentIndex(void)
{
    // reg offset: 0x15A ; bit: 15-0

    return AUD_ReadRegDev16(R_DDP_SPO_DMA_CUR_INDEX_0X15A);
}



static AUD_UINT16 GetDDPSPOSampCount(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT8 GetDDPSPOFrameLenInfo(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT16 GetDDPSPOFramePDInfo(void)
{
    // reg offset: 0x16A ; bit: 15-0

    return AUD_ReadRegDev16(R_DDP_SPO_IEC_PD_0X16A);
}



static AUD_UINT16 GetDDPSPOFrameNullDataInfo(void)
{
    // reg offset: 0x164 ; bit: 15-0

    return AUD_ReadRegDev16(R_DDP_SPO_IEC_NULL_0X164);
}



static AUD_UINT16 GetDDPSPOFrameBurstDataInfo(void)
{
    // reg offset: 0x166 ; bit: 15-0

    return AUD_ReadRegDev16(R_DDP_SPO_IEC_DATA_0X166);
}



static AUD_UINT8 GetDDPSPOBufUnderRunStatus(void)
{
    // reg offset: 0x14 ; bit: 7

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_BUF_UNDER_RUN_CTRL_0X14);

    if (RegValue&(0x01<<7))
    {
        AUD_WriteRegDev8(R_BUF_UNDER_RUN_CTRL_0X14, RegValue);

        return 1;
    }
    else
    {
        return 0;
    }
}



static AUD_UINT8 GetDDPSPOFIFOUnderRunStatus(void)
{
    /* Dummy function */
    return 0;
}



static AUD_UINT16 GetDDPSPODMASkipNum(void)
{
    /* Dummy function */
    return 0;
}



/* Enable DDP SPO interrupts ------------------------------------------------ */

static void EnableDDPSPOSampCountInt(AUD_BOOL Enable)
{
    // reg offset: 0x160 ; bit: 0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_INT_0X160);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DDP_SPO_INT_0X160, \
                         ((RegValue&(~(0x01<<0))) | (0x01<<0)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DDP_SPO_INT_0X160, \
                         (RegValue&(~(0x01<<0))));
    }
    else
    {
        SDBBP();
    }
}



static void EnableDDPSPODMABufResumeInt(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void EnableDDPSPODMAUnderRunInt(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



/* Get DDP SPO interrupt status --------------------------------------------- */

static AUD_BOOL CheckDDPSPOSampCountInt(void)
{
    // reg offset: 0x160 ; bit: 0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_INT_0X160);

    if (RegValue&(0x01<<0))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL CheckDDPSPOSampCountIntStatus(void)
{
    // reg offset: 0x160 ; bit: 4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DDP_SPO_INT_0X160);

    if (RegValue&(0x01<<4))
    {
        AUD_WriteRegDev8(R_DDP_SPO_INT_0X160, RegValue);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL CheckDDPSPODMABufResumeInt(void)
{
    /* Dummy function */
    return FALSE;
}



static AUD_BOOL CheckDDPSPODMABufResumeIntStatus(void)
{
    /* Dummy function */
    return FALSE;
}



static AUD_BOOL CheckDDPSPODMAUnderRunInt(void)
{
    /* Dummy function */
    return FALSE;
}



static AUD_BOOL CheckDDPSPODMABufUnderRunIntStatus(void)
{
    /* Dummy function */
    return FALSE;
}




/* -----------------------------------------------------------------------------
I2SI Hardware Register Control Functions
----------------------------------------------------------------------------- */

static void ConfigI2SIFSByHardware(AUD_SampleRate_t SampleRate)
{
    /* Dummy function */
    return;
}



static void EnableI2SITXInterface(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



void EnableI2SIRXInterface(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}

EXPORT_SYMBOL_GPL(EnableI2SIRXInterface);


static void StartI2SITXDMA(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



void StartI2SIRXDMA(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}

EXPORT_SYMBOL_GPL(StartI2SIRXDMA);


/* -----------------------------------------------------------------------------
PCM Hardware Register Control Functions
----------------------------------------------------------------------------- */

static void ConfigPCMFSByHardware(AUD_SampleRate_t SampleRate)
{
    /* Dummy function */
    return;
}



static void EnablePCMTXInterface(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void EnablePCMRXInterface(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void StartPCMTXDMA(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void StartPCMRXDMA(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



/* -----------------------------------------------------------------------------
DAC Hardware Register Control Functions
----------------------------------------------------------------------------- */

static void EnableDACFSSoftwareConfig(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void EnableDACFSGated(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



static void SetDACFSConfigDiv(DAC_CLK_SW_DivMode_t DivMode)
{
    /* Dummy function */
    return;
}



/* DAC I2S Register config */

static void SelectDACI2SSDATLen(DAC_I2S_SDAT_Len_t DataLen)
{
    // reg offset: 0xF5 ; bit: 7-6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_I2S_REG_8_15_0XF5);

    if (SDAT_24_BITS == DataLen)
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_8_15_0XF5, \
                         ((RegValue&(~(0x03<<6))) | (0x02<<6)));        
    }
    else if (SDAT_16_BITS == DataLen)
    {
        AUD_WriteRegDev8(R_DDP_SPO_IEC_CTRL_0X161, \
                         (RegValue&(~(0x03<<6))));      
    }
    else
    {
        SDBBP();
    }
}



static void SelectDACI2SWselLen(DAC_I2S_WSEL_Len_t WselLen)
{
    // reg offset: 0xF6 ; bit: 7-6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_I2S_REG_16_23_0XF6);

    switch (WselLen)
    {
        case I2S_WSEL_16_BITS:
        {
            AUD_WriteRegDev8(R_DAC_I2S_REG_16_23_0XF6, \
                             ((RegValue&(~(0x03<<6))) | (0x00<<6)));
            break;
        }

        case I2S_WSEL_24_BITS:
        {
            AUD_WriteRegDev8(R_DAC_I2S_REG_16_23_0XF6, \
                             ((RegValue&(~(0x03<<6))) | (0x02<<6)));
            break;
        }

        case I2S_WSEL_32_BITS:
        {
            AUD_WriteRegDev8(R_DAC_I2S_REG_16_23_0XF6, \
                             ((RegValue&(~(0x03<<6))) | (0x03<<6)));
            break;
        }

        default:
        {
            break;
        }
    }
}



static void SelectDACI2SInterfaceMode(DAC_I2S_InterfaceMode_t Mode)
{
    // reg offset: 0xF6 ; bit: 5-4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_I2S_REG_16_23_0XF6);

    AUD_WriteRegDev8(R_DAC_I2S_REG_16_23_0XF6, \
                     ((RegValue&(~(0x03<<4))) | ((Mode&0x03)<<4)));
}



static void SelectDACI2SSDATSwapMode(DAC_I2S_SDAT_SwapMode_t SwapMode)
{
    // reg offset: 0xF6 ; bit: 3

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_I2S_REG_16_23_0XF6);

    if (SDAT_OUT_LSB_FIRST == SwapMode) // LSB first
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_16_23_0XF6, \
                         ((RegValue&(~(0x01<<3))) | (0x01<<3)));        
    }
    else if (SDAT_OUT_MSB_FIRST == SwapMode) // MSB first
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_16_23_0XF6, \
                         (RegValue&(~(0x01<<3))));      
    }
    else
    {
        SDBBP();
    }
}



static void SelectDACI2SWselPolarInv(DAC_I2S_WSEL_PolarInv_t Polar)
{
    // reg offset: 0xF6 ; bit: 2

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_I2S_REG_16_23_0XF6);

    if (START_FROM_HIGH == Polar) // LRCLK start from high
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_16_23_0XF6, \
                         ((RegValue&(~(0x01<<2))) | (0x01<<2)));        
    }
    else if (START_FROM_LOW == Polar) // LRCLK start from low
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_16_23_0XF6, \
                         (RegValue&(~(0x01<<2))));      
    }
    else
    {
        SDBBP();
    }
}



static void EnableDACI2SSDATInput(AUD_BOOL Enable)
{
    // reg offset: 0xF6 ; bit: 1

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_I2S_REG_16_23_0XF6);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_16_23_0XF6, \
                         ((RegValue&(~(0x01<<1))) | (0x01<<1)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_16_23_0XF6, \
                         (RegValue&(~(0x01<<1))));      
    }
    else
    {
        SDBBP();
    }
}



static void SelectDACI2SChannelNum(DAC_I2S_ChanNum_t ChannelNum)
{
    // reg offset: 0xF6 ; bit: 0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_I2S_REG_16_23_0XF6);

    if (DAC_I2S_TWO_CHAN == ChannelNum) // default
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_16_23_0XF6, \
                         ((RegValue&(~(0x01<<0))) | (0x01<<0)));        
    }
    else if (DAC_I2S_SINGLE_CHAN == ChannelNum)
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_16_23_0XF6, \
                         (RegValue&(~(0x01<<0))));      
    }
    else
    {
        SDBBP();
    }
}



static void SelectDACI2SSource(DAC_I2S_Source_t Source)
{
    // reg offset: 0xF7 ; bit: 4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_I2S_REG_24_31_0XF7);

    if (I2S_FROM_EXTERNAL == Source) // I2S From external for test
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_24_31_0XF7, \
                         ((RegValue&(~(0x01<<4))) | (0x01<<4)));      
    }
    else if (I2S_FROM_INTERNEL == Source) // I2S From internal (Default)
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_24_31_0XF7, \
                         (RegValue&(~(0x01<<4))));        
    }      
    else
    {
        SDBBP();
    }
}



static void EnableDACI2SDMA(AUD_BOOL Enable)
{
    // reg offset: 0xF7 ; bit: 3

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_I2S_REG_24_31_0XF7);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_24_31_0XF7, \
                         ((RegValue&(~(0x01<<3))) | (0x01<<3)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_24_31_0XF7, \
                         (RegValue&(~(0x01<<3))));
    }
    else
    {
        SDBBP();
    }
}



static void SelectDACI2SOTriggerMode(DAC_I2S_TriggerMode_t Mode)
{
    // reg offset: 0xF7 ; bit: 0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_I2S_REG_24_31_0XF7);

    if (I2S_SCLK_POSITIVE == Mode) // Trigger data at the positive edge of I2S_SCLK
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_24_31_0XF7, \
                         ((RegValue&(~(0x01<<0))) | (0x01<<0)));      
    }
    else if (I2S_SCLK_NEGATIVE == Mode) // Trigger data at the negative edge of I2S_SCLK
    {
        AUD_WriteRegDev8(R_DAC_I2S_REG_24_31_0XF7, \
                         (RegValue&(~(0x01<<0))));
    }
    else
    {
        SDBBP();
    }
}



/* DAC Analog Register config */

static void EnableADACRightChanBS(AUD_BOOL Enable)
{
    // reg offset: 0xF8 ; bit: 7
    
    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_0_7_0XF8);

    if (TRUE == Enable) // Enable the right channel bit-stream data
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_0_7_0XF8, \
                         (RegValue&(~(0x01<<7))));      
    }
    else if (FALSE == Enable) // Disable the right channel bit-stream data
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_0_7_0XF8, \
                         ((RegValue&(~(0x01<<7))) | (0x01<<7)));
    }
    else
    {
        SDBBP();
    }
}



static void EnableADACLeftChanBS(AUD_BOOL Enable)
{
    // reg offset: 0xF8 ; bit: 6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_0_7_0XF8);

    if (TRUE == Enable) // Enable the left channel bit-stream data
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_0_7_0XF8, \
                         (RegValue&(~(0x01<<6))));      
    }
    else if (FALSE == Enable) // Disable the left channel bit-stream data
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_0_7_0XF8, \
                         ((RegValue&(~(0x01<<6))) | (0x01<<6)));
    }
    else
    {
        SDBBP();
    }
}



static void EnableADACExchangeFunction(AUD_BOOL Enable)
{
    // reg offset: 0xF8 ; bit: 5

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_0_7_0XF8);

    if (TRUE == Enable) // Exchange the left/right channel bit stream data
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_0_7_0XF8, \
                         ((RegValue&(~(0x01<<5))) | (0x01<<5)));      
    }
    else if (FALSE == Enable) // Normal
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_0_7_0XF8, \
                         ((RegValue&(~(0x01<<5))) | (0x00<<5)));
    }
    else
    {
        SDBBP();
    }
}



static void EnableADACPDVolRefer(AUD_BOOL Enable)
{
    // reg offset: 0xF8 ; bit: 4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_0_7_0XF8);

    if (TRUE == Enable) // Switch on the internal reference voltage
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_0_7_0XF8, \
                         (RegValue&(~(0x01<<4))));      
    }
    else if (FALSE == Enable) // Switch off the internal reference voltage
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_0_7_0XF8, \
                         ((RegValue&(~(0x01<<4))) | (0x01<<4)));
    }
    else
    {
        SDBBP();
    }
}



static void EnableADACBiasCurrent(AUD_BOOL Enable)
{
    // reg offset: 0xF8 ; bit: 3

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_0_7_0XF8);

    if (TRUE == Enable) // self-bias current
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_0_7_0XF8, \
                         (RegValue&(~(0x01<<3))));   
    }
    else if (FALSE == Enable) // select reference current
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_0_7_0XF8, \
                         ((RegValue&(~(0x01<<3))) | (0x01<<3)));   
    }
    else
    {
        SDBBP();
    }
}



static void SelectADACOutputDCOffset(ADAC_OutputDC_Offset_t Offset)
{
    // reg offset: 0xF8 ; bit: 2-0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_0_7_0XF8);

    AUD_WriteRegDev8(R_DAC_CODEC_REG_0_7_0XF8, \
                     ((RegValue&(~(0x07<<0))) | ((Offset&0x07)<<0)));
}



static void SelectADACOutputMode(ADAC_OutputMode_t Mode)
{
    // reg offset: 0xF9 ; bit: 7

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_8_15_0XF9);

    if (SINGLE_OUTPUT == Mode) // Single output mode
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_8_15_0XF9, \
                         ((RegValue&(~(0x01<<7))) | (0x01<<7)));      
    }
    else if (DIFFERENTIAL_OUTPUT == Mode) // differential output mode (c3701c donot support)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_8_15_0XF9, \
                         (RegValue&(~(0x01<<7))));
    }
    else
    {
        SDBBP();
    }
}



static void EnableADACPowerUp(AUD_BOOL Enable)
{
    // reg offset: 0xF9 ; bit: 6-0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_8_15_0XF9);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_8_15_0XF9, \
                         ((RegValue&(~(0x7F<<0))) | (0x00<<0)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_8_15_0XF9, \
                         ((RegValue&(~(0x7F<<0))) | (0x7F<<0)));        
    }
    else
    {
        SDBBP();
    }
}



static void SelectADACRandomSource(ADAC_RandomSource_t Source)
{
    // reg offset: 0xFA ; bit: 7

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_16_23_0XFA);

    if (FROM_RDM_IN == Source) // From random in
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_16_23_0XFA, \
                         ((RegValue&(~(0x01<<7))) | (0x01<<7)));      
    }
    else if (FROM_SELF_HALF_CLK == Source) // From self half-clock
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_16_23_0XFA, \
                         (RegValue&(~(0x01<<7))));
    }
    else
    {
        SDBBP();
    }
}



static void EnableADACRandomSwitch(AUD_BOOL Enable)
{
    // reg offset: 0xFA ; bit: 6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_16_23_0XFA);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_16_23_0XFA, \
                         ((RegValue&(~(0x01<<6))) | (0x01<<6)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_16_23_0XFA, \
                         (RegValue&(~(0x01<<6))));
    }
    else
    {
        SDBBP();
    }
}



static void SelectADACTestMode(AUD_UINT8 Mode)
{
    // reg offset: 0xFA ; bit: 5-4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_16_23_0XFA);

    AUD_WriteRegDev8(R_DAC_CODEC_REG_16_23_0XFA, \
                     ((RegValue&(~(0x03<<4))) | ((Mode&0x03)<<4)));
}



static void SetADACVolCtrlOutSwing(AUD_UINT8 Setting)
{
    // reg offset: 0xFA ; bit: 3-2

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_16_23_0XFA);

    AUD_WriteRegDev8(R_DAC_CODEC_REG_16_23_0XFA, \
                     ((RegValue&(~(0x03<<2))) | ((Setting&0x03)<<2)));
}



static void SetADACCommonModeVolCtrlReg(AUD_UINT8 Setting)
{
    // reg offset: 0xFA ; bit: 1-0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_16_23_0XFA);

    AUD_WriteRegDev8(R_DAC_CODEC_REG_16_23_0XFA, \
                     ((RegValue&(~(0x03<<0))) | ((Setting&0x03)<<0)));
}



static void SelectADACBistFormat(ADAC_BistFormat_t Format)
{
    // reg offset: 0xFB ; bit: 7-5

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_24_31_0XFB);

    AUD_WriteRegDev8(R_DAC_CODEC_REG_24_31_0XFB, \
                     ((RegValue&(~(0x07<<5))) | ((Format&0x07)<<5)));
}



static void EnableADACBist(AUD_BOOL Enable)
{
    // reg offset: 0xFB ; bit: 4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_24_31_0XFB);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_24_31_0XFB, \
                         ((RegValue&(~(0x01<<4))) | (0x01<<4)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_24_31_0XFB, \
                         (RegValue&(~(0x01<<4))));
    }
    else
    {
        SDBBP();
    }
}



static void SelectADACAnalogClkSource(ADAC_Analog_ClkSource_t Source)
{
    // reg offset: 0xFB ; bit: 3

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_24_31_0XFB);

    if (ANALOG_CLK_FROM_ADAC == Source)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_24_31_0XFB, \
                         ((RegValue&(~(0x01<<3))) | (0x01<<3)));      
    }
    else if (ANALOG_CLK_FROM_APLL == Source) // From APLL
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_24_31_0XFB, \
                         (RegValue&(~(0x01<<3))));
    }
    else
    {
        SDBBP();
    }
}



static void SetADACMuteMode(ADAC_MuteMode_t Mode)
{
    // reg offset: 0xFB ; bit: 2-0
    
    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_24_31_0XFB);

    AUD_WriteRegDev8(R_DAC_CODEC_REG_24_31_0XFB, \
                     ((RegValue&(~(0x07<<0))) | ((Mode&0x07)<<0)));
}



/* DAC Digital Register config */

static void EnableDACDigtal(AUD_BOOL Enable)
{
    // reg offset: 0xFC ; bit: 7

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_32_39_0XFC);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_32_39_0XFC, \
                         ((RegValue&(~(0x01<<7))) | (0x01<<7)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_32_39_0XFC, \
                         (RegValue&(~(0x01<<7))));
    }
    else
    {
        SDBBP();
    }
}



static void ClearDACDigitalDSM(AUD_BOOL Clear)
{
    // reg offset: 0xFC ; bit: 6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_32_39_0XFC);

    if (TRUE == Clear)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_32_39_0XFC, \
                         ((RegValue&(~(0x01<<6))) | (0x01<<6)));      
    }
    else if (FALSE == Clear)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_32_39_0XFC, \
                         (RegValue&(~(0x01<<6))));
    }
    else
    {
        SDBBP();
    }
}



static void EnableDACDigtalMute(AUD_BOOL Enable)
{
    // reg offset: 0xFC ; bit: 5

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_32_39_0XFC);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_32_39_0XFC, \
                         ((RegValue&(~(0x01<<5))) | (0x01<<5)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_32_39_0XFC, \
                         (RegValue&(~(0x01<<5))));
    }
    else
    {
        SDBBP();
    }
}



static void SelectDACDigitalDsmDataMode(DAC_Invert_DsmDataMode_t Mode)
{
    // reg offset: 0xFC ; bit: 3

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_32_39_0XFC);

    if (DAC_ORG_DSM_DATA == Mode) // Select the original DSM data
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_32_39_0XFC, \
                         ((RegValue&(~(0x01<<3))) | (0x01<<3)));      
    }
    else if (DAC_INV_DSM_DATA == Mode) // Select the Inverted DSM data
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_32_39_0XFC, \
                         (RegValue&(~(0x01<<3))));
    }
    else
    {
        SDBBP();
    }
}



static void SelectDACDigitalChanMode(DAC_DIG_ChanMode_t Mode)
{
    // reg offset: 0xFC ; bit: 2

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_32_39_0XFC);

    if (CODEC_DIG_MONO == Mode) // Mono Mode
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_32_39_0XFC, \
                         ((RegValue&(~(0x01<<2))) | (0x01<<2)));      
    }
    else if (CODEC_DIG_STEREO == Mode) // Stereo Mode
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_32_39_0XFC, \
                         (RegValue&(~(0x01<<2))));
    }
    else
    {
        SDBBP();
    }
}



static void SelectDACDigitalDsmClkMode(DAC_Invert_DsmClkMode_t Mode)
{
    // reg offset: 0xFC ; bit: 1

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_32_39_0XFC);

    if (DAC_INV_DSM_CLK == Mode) // Select the Inverted DSM clock
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_32_39_0XFC, \
                         ((RegValue&(~(0x01<<1))) | (0x01<<1)));      
    }
    else if (DAC_ORG_DSM_CLK == Mode) // Select the original DSM clock
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_32_39_0XFC, \
                         (RegValue&(~(0x01<<1))));
    }
    else
    {
        SDBBP();
    }
}



static void ClearDACDigitalBuf(AUD_BOOL Clear)
{
    // reg offset: 0xFC ; bit: 0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_32_39_0XFC);

    if (TRUE == Clear)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_32_39_0XFC, \
                         ((RegValue&(~(0x01<<0))) | (0x01<<0)));      
    }
    else if (FALSE == Clear)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_32_39_0XFC, \
                         (RegValue&(~(0x01<<0))));
    }
    else
    {
        SDBBP();
    }
}



static void EnableBSTransmitToADACMask(AUD_BOOL Enable)
{
    // reg offset: 0xFD ; bit: 7

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_40_47_0XFD);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_40_47_0XFD, \
                         ((RegValue&(~(0x01<<7))) | (0x01<<7)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_40_47_0XFD, \
                         (RegValue&(~(0x01<<7))));
    }
    else
    {
        SDBBP();
    }
}



static void EnableClkTransmitToADACMask(AUD_BOOL Enable)
{
    // reg offset: 0xFD ; bit: 6

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_40_47_0XFD);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_40_47_0XFD, \
                         ((RegValue&(~(0x01<<6))) | (0x01<<6)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_40_47_0XFD, \
                         (RegValue&(~(0x01<<6))));
    }
    else
    {
        SDBBP();
    }
}



static void ClearDACDigtialDsmPara(AUD_BOOL Clear)
{
    // reg offset: 0xFD ; bit: 4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_40_47_0XFD);

    if (TRUE == Clear)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_40_47_0XFD, \
                         ((RegValue&(~(0x01<<4))) | (0x01<<4)));      
    }
    else if (FALSE == Clear)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_40_47_0XFD, \
                         (RegValue&(~(0x01<<4))));
    }
    else
    {
        SDBBP();
    }
}



static void EnableDACDIGDelayFunction(AUD_BOOL Enable)
{
    // reg offset: 0xFD ; bit: 3

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_40_47_0XFD);

    if (TRUE == Enable) // Channel R delay one sample
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_40_47_0XFD, \
                         (RegValue&(~(0x01<<3))));    
    }
    else if (FALSE == Enable) // no delay
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_40_47_0XFD, \
                         ((RegValue&(~(0x01<<3))) | (0x01<<3)));  
    }
    else
    {
        SDBBP();
    }
}



static void EnableDACNewFilter(AUD_BOOL Enable)
{
    // reg offset: 0xFD ; bit: 1

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DAC_CODEC_REG_40_47_0XFD);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_40_47_0XFD, \
                         ((RegValue&(~(0x01<<1))) | (0x01<<1)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_DAC_CODEC_REG_40_47_0XFD, \
                         (RegValue&(~(0x01<<1))));
    }
    else
    {
        SDBBP();
    }
}



static void ConfigDACFSBySoftware(AUD_SampleRate_t SampleRate)
{
    /* Dummy function */
    return;
}



/* -----------------------------------------------------------------------------
STC Hardware Register Control Functions
----------------------------------------------------------------------------- */

static void EnableSTCPlay(AUD_UINT8 STCNum,
                   AUD_BOOL Enable)
{
    // reg offset: 0xE6 ; bit: 0 (STC0)
    // reg offset: 0xEE ; bit: 0 (STC1)

    AUD_UINT32 STCRegAddr = 0;
    AUD_UINT8 RegValue = 0;

    if (1 == STCNum)
    {
        STCRegAddr = R_COUNTER_F0_CTRL_0XE6;
    }
    else if (2 == STCNum)
    {
        STCRegAddr = R_COUNTER_F1_CTRL_0XEE;
    }
    else
    {
        SDBBP();
    }

    RegValue = AUD_ReadRegDev8(STCRegAddr);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(STCRegAddr, \
                         ((RegValue&(~(0x01<<0))) | (0x00<<0)));
    }
    else if (FALSE == Enable) // Pause
    {
        AUD_WriteRegDev8(STCRegAddr, \
                         ((RegValue&(~(0x01<<0))) | (0x01<<0)));
    }
    else
    {
        SDBBP();
    }
}




static void SetSTCDivisor(AUD_UINT8 STCNum,
                   AUD_UINT16 Divisor)
{
    // reg offset: 0xE4 ; bit: 15-0 (STC0)
    // reg offset: 0xEC ; bit: 15-0 (STC1)

    AUD_UINT32 STCRegAddr = 0;

    if (1 == STCNum)
    {
        STCRegAddr = R_COUNTER_F0_FRQ_0XE4;
    }
    else if (2 == STCNum)
    {
        STCRegAddr = R_COUNTER_F1_FRQ_0XEC;
    }
    else
    {
        SDBBP();
    }    

    AUD_WriteRegDev16(STCRegAddr, \
                      (Divisor&0xFFFF));
}



static AUD_UINT16 GetSTCDivisor(AUD_UINT8 STCNum)
{
    // reg offset: 0xE4 ; bit: 15-0 (STC0)
    // reg offset: 0xEC ; bit: 15-0 (STC1)

    AUD_UINT32 STCRegAddr = 0;

    if (1 == STCNum)
    {
        STCRegAddr = R_COUNTER_F0_FRQ_0XE4;
    }
    else if (2 == STCNum)
    {
        STCRegAddr = R_COUNTER_F1_FRQ_0XEC;
    }
    else
    {
        SDBBP();
    }

    return AUD_ReadRegDev16(STCRegAddr);
}



static void SetSTCValue(AUD_UINT8 STCNum,
                        AUD_UINT32 Value)
{
    // reg offset: 0xE8 ; bit: 31-0 (STC0)
    // reg offset: 0xE7 ; bit: 3    (STC0)
    // reg offset: 0xF0 ; bit: 31-0 (STC1)
    // reg offset: 0xEF ; bit: 3    (STC1)

    AUD_UINT8 RegValue = 0;
    AUD_UINT32 STCRegAddr = 0;
    
    if (1 == STCNum)
    {
        STCRegAddr = R_COUNTER_F0_1_32_0XE8;
    }
    else if (2 == STCNum)
    {
        STCRegAddr = R_COUNTER_F1_1_32_0XF0;
    }
    else
    {
        SDBBP();
    }

    AUD_WriteRegDev32(STCRegAddr, \
                      (Value&0xFFFFFFFF));

    if (1 == STCNum)
    {
        STCRegAddr = R_COUNTER_F0_33_0_0XE7;
    }
    else if (2 == STCNum)
    {
        STCRegAddr = R_COUNTER_F1_33_0_0XEF;
    }
    else
    {
        SDBBP();
    }
    
    // clear F1[33]
    RegValue = AUD_ReadRegDev8(STCRegAddr);

    AUD_WriteRegDev8(STCRegAddr, \
                     (RegValue&(~(0x01<<3))));
}



static AUD_UINT32 GetSTCValue(AUD_UINT8 STCNum)
{
    // reg offset: 0xE8 ; bit: 31-0 (STC0)
    // reg offset: 0xF0 ; bit: 31-0 (STC1)

    AUD_UINT32 STCRegAddr = 0;
    
    if (1 == STCNum)
    {
        STCRegAddr = R_COUNTER_F0_1_32_0XE8;
    }
    else if (2 == STCNum)
    {
        STCRegAddr = R_COUNTER_F1_1_32_0XF0;
    }
    else
    {
        SDBBP();
    }

    return AUD_ReadRegDev32(STCRegAddr);
}



/* -----------------------------------------------------------------------------
Volume Hardware Register Control Functions
----------------------------------------------------------------------------- */

static void SetTargetVolume(AUD_UINT8 Volume)
{
    // reg offset: 0x6E ; bit: 7-0

    AUD_WriteRegDev8(R_TARGET_VOLUME_0X6E, \
                     (Volume&0xFF));
}



static AUD_UINT8 GetCurrentVolume(void)
{
    // reg offset: 0x6F ; bit: 7-0

    return AUD_ReadRegDev8(R_CUR_VOLUME_0X6F);
}



static void EnableFadeFunction(AUD_BOOL Enable)
{
    // reg offset: 0x6C ; bit: 7

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_FADE_CTRL_0X6C);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev8(R_FADE_CTRL_0X6C, \
                         ((RegValue&(~(0x01<<7))) | (0x01<<7)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev8(R_FADE_CTRL_0X6C, \
                         (RegValue&(~(0x01<<7))));
    }
    else
    {
        SDBBP();
    }
}



static void SetFadeSpeed(AUD_UINT8 FadeSpeed)
{
    // reg offset: 0x6D ; bit: 7-0

    AUD_WriteRegDev8(R_FADE_SPEED_0X6D, \
                     (FadeSpeed&0xFF));
}


/* -----------------------------------------------------------------------------
Audio Register layer function for HAL
----------------------------------------------------------------------------- */

static void OpenDAC(DAC_FORMAT_t Format,
			        AUD_Precision_t Precision)
{
    /* DAC Analog Register config */
    EnableADACRightChanBS(TRUE);
    EnableADACLeftChanBS(TRUE);
    EnableADACExchangeFunction(FALSE);
    EnableADACPDVolRefer(TRUE);
    EnableADACBiasCurrent(TRUE);
    SelectADACOutputDCOffset(OFFSET_0MV);

    SelectADACOutputMode(SINGLE_OUTPUT);
    EnableADACPowerUp(TRUE);

    /* DAC I2S Register config */
    SelectDACI2SSource(I2S_FROM_INTERNEL);
    EnableDACI2SDMA(TRUE);
    SelectDACI2SOTriggerMode(I2S_SCLK_NEGATIVE);

    /* DAC Digital Register use default configure */

    return;
}



static void StartDAC(void)
{
    AUD_UINT8 DBStep = 8;
    AUD_UINT8 i = 0;

    /* enable digital DAC */
    EnableDACI2SSDATInput(TRUE);

    /* enable data input */
    EnableDACDigtal(TRUE);


    /* Other DAC regs use default setting */


    DBStep = 8;
    /* set MUTE[2:0] to 000(0db) */
    for (i=DBStep; i!=0; i--)
    {
        SetADACMuteMode((i-1));
        AUD_Delay(1000);
    }
            
    return;
}



static void StopDAC(void)
{
    AUD_UINT8 DBStep = 7;
    AUD_UINT8 i = 0;

    /* set MUTE[2:0] to 111(-39.7db\-32.3db) */    
    for (i=0; i<=DBStep; i++)
    {
        SetADACMuteMode(i);
        AUD_Delay(1000);
    }

    /* disable digital DAC */
    EnableDACDigtal(FALSE);

    AUD_Delay(1000); //sleep 1ms for coder disable.

    /* disable data input */
    EnableDACI2SSDATInput(FALSE);

    return;
}



static void CloseDAC(void)
{        
    SetADACMuteMode(ATTENUATION_N_39_7DB);
    
    EnableADACRightChanBS(FALSE);
    EnableADACLeftChanBS(FALSE);
    
    EnableADACPowerUp(FALSE);

    return;
}



static void ConfigI2SOInterface(AUD_OutputParams_t *Params_p)
{
    AUD_UINT16 *DAC_Setting = NULL;
    AUD_UINT32 DWordValue = 0;
    AUD_UINT16 ShortValue = 0;
    DAC_I2S_InterfaceMode_t I2SOInterfaceMode = 0;
    AUD_UINT32 i = 0;
    
    if (NULL == Params_p)
    {
        printk( "Input parameters are wrong!\n");
        return;
    }

    /* Enable I2SO clk before config */
    EnableI2SOInterfaceClock(TRUE);

    if (Precision_24_BITS == Params_p->Precision)
    {
        DAC_Setting = (AUD_UINT16 *)DAC_24Bit_Setting;
    }
    else
    {
        DAC_Setting = (AUD_UINT16 *)DAC_16Bit_Setting;
    }
    
    while (DAC_Setting[i] != 0)
    {
        if (GETFS(DAC_Setting[i]) == FSSET((AUD_UINT16)Params_p->SampleRate))
        {
            break;
        }
        i++;
    }
    if (0 == DAC_Setting[i])
    {
        printk( "This type I2SO configure is not surported yet.\n");        
    }
        
    ShortValue = DAC_Setting[i];
    ShortValue = (ShortValue>>5)&7; // bit 5:7 = mclk
    if (!(ShortValue != 4))
    {
        SDBBP();
    }

    /* Save the MainClk for SPO/DDPSPO clk config */
    Params_p->MainClk = ShortValue;

    DWordValue = GetSystemCtrlRegConfig();
    DWordValue &= 0xffffffdb; // clear bit5
    DWordValue |= (((ShortValue&0x2)<<1) | ((ShortValue&0x1)<<5));
    if (SAMPLE_RATE_32000 == Params_p->SampleRate)
    {
        DWordValue |= 0x40;
    }

    ConfigSystemCtrlReg(DWordValue);

    ShortValue = DAC_Setting[i];
   
    
    if (I2S_FORMAT == Params_p->Format)
    {
        I2SOInterfaceMode = INTF_I2S;
    }
    else if(LEFT_JUSTIFIED == Params_p->Format)
    {
        I2SOInterfaceMode = INTF_LEFT_JUSTIED;
    }
    else if ((RIGHT_JUSTIFIED_16BIT == Params_p->Format) || \
             (RIGHT_JUSTIFIED_24BIT == Params_p->Format))
    {
        I2SOInterfaceMode = INTF_RIGHT_JUSTIED;
    }
    else
    {
        SDBBP(); 
    }

    SetI2SOInterfaceFormat(Params_p->Format);
    SelectDACI2SInterfaceMode(I2SOInterfaceMode);

    
    // Config DAC I2S relative bit
  	SelectDACI2SWselLen(I2S_WSEL_24_BITS);
    //SelectDACI2SInterfaceMode(INTF_I2S);

    // Set MCLK/BCLK
    SetI2SOMCLKDivBySoftware((I2SO_MCLK_DivMode_t)((ShortValue>>2)&0x7));

    // Set BCLK/LCLK
    SetI2SOInterfaceBCLK2LRCLK((BIT_CLOCK_MODE_t)(ShortValue&0x3));
        
    if (Precision_16_BITS == Params_p->Precision)
    {
        if (SAMPLE_RATE_8000 == Params_p->SampleRate)
        {
            // MCLK/4
            SelectI2SOMCLKMode(I2S_SOURCE_CLK_DIV_4);
        }
        else if (SAMPLE_RATE_16000 == Params_p->SampleRate)
        {
            // MCLK/2
            SelectI2SOMCLKMode(I2S_SOURCE_CLK_DIV_2);
        }
        else if ((SAMPLE_RATE_22050 == Params_p->SampleRate) || \
                 (SAMPLE_RATE_24000 == Params_p->SampleRate) || \
                 (SAMPLE_RATE_32000 == Params_p->SampleRate))
        {
            // MCLK/2
            SelectI2SOMCLKMode(I2S_SOURCE_CLK_DIV_2);
        }
        else
        {
            // MCLK
            SelectI2SOMCLKMode(I2S_SOURCE_CLK_DIV_0);
        }

        // 16 Bit
        SelectDACI2SSDATLen(SDAT_16_BITS);
    }
    else // Precision = 24
    {
        if ((SAMPLE_RATE_8000 == Params_p->SampleRate))
        {
            // MCLK/8
            SelectI2SOMCLKMode(I2S_SOURCE_CLK_DIV_8);
        }
        else if ((SAMPLE_RATE_16000 == Params_p->SampleRate))
        {
            // MCLK/4
            SelectI2SOMCLKMode(I2S_SOURCE_CLK_DIV_4);
        }
        else if ((SAMPLE_RATE_22050 == Params_p->SampleRate) || \
                 (SAMPLE_RATE_24000 == Params_p->SampleRate) || \
                 (SAMPLE_RATE_32000 == Params_p->SampleRate))
        {
            // MCLK/2
            SelectI2SOMCLKMode(I2S_SOURCE_CLK_DIV_2);
        }
        else
        {
            // MCLK
            SelectI2SOMCLKMode(I2S_SOURCE_CLK_DIV_0);
        }	
        
        // 24 Bit
        SelectDACI2SSDATLen(SDAT_24_BITS);
    }
    
    if (TRUE == Params_p->DMADataHeaderFlag)
    {
        SetI2SODMADataWithHeader(TRUE);
    }
    else
    {
        SetI2SODMADataWithHeader(FALSE);
    }

    SetI2SODMADataBitNum(BIT_NUM_24);
    SetI2SODMADataChannelNum(CHAN_NUM_2);

    EnableI2SOAutoResume(TRUE);
    SetI2SOSampCountThreshold(Params_p->SampleNum);
    SetI2SOTimeCheckThreshold(TIME_CHECK_THRESHOLD);
        
    /* Make sure every call last index = 0 */
    SetI2SODMABufLastIndex(0);
    

    /* Enable I2SO interrupts */
    //EnableI2SOTimingCheckInt(TRUE);
    EnableI2SOSampCountInt(TRUE);
    EnableI2SODMABufResumeInt(TRUE);
    EnableI2SOUnderRunFade(TRUE);

    /* Enalbe I2SO interface */
    EnableI2SOInterface(TRUE);

    return;
}



static void ConfigSPOInterface(AUD_OutputParams_t *Params_p)
{
    AUD_UINT32 StrType = 0;
    AUD_UINT32 MainClkSetting = 0;           
    AUD_UINT32 ClkSelectTmp = 0;
    AUD_UINT32 i = 0;
    AUD_UINT8 bPcm = 1;
    AUD_UINT8 SPOSelect = 0;
    AUD_UINT8 ClkSelect = 0;
    AUD_UINT8 SPOFS = 0;
    AUD_UINT8 SPOClkDiv = 0;
    
    if (NULL == Params_p)
    {
        printk( "Input parameters are wrong!\n");
        return;
    }

    StrType = GET_BIT(Params_p->StreamType);
    
    if (SRC_BUF == Params_p->SPOChlPCMType)
    {
        if ((StrType&(GET_BIT(AUDIO_OGG)|GET_BIT(AUDIO_MP3)|GET_BIT(AUDIO_WMA))) || \
            !(StrType & Params_p->BitStreamSetByUser))
        {
            if ((TRUE == Params_p->MPEGM8dbEnableFlag) && \
                ((AUDIO_MPEG1 == Params_p->StreamType) || \
                 (AUDIO_MPEG2 == Params_p->StreamType)))
            {
                bPcm = 1;
                SPOSelect = 0x04; // Output MPEG as bit stream
                SelectSPODataSource(FROM_SPO_DMA);
                SelectF128LRCLKDivMode(SPO_LRCLK_DIV_1);
            }
            else
            {
                Params_p->SPOChlPCMType = SRC_DMLR;
                SPOSelect = 0x0;
                SelectSPODataSource(FROM_I2SO_DMA);
                SelectF128LRCLKDivMode(SPO_LRCLK_DIV_1);
            }
        }
        else
        {
            bPcm = 0;
            SPOSelect = 0x04;
            SelectSPODataSource(FROM_SPO_DMA);
            SelectF128LRCLKDivMode(SPO_LRCLK_DIV_1);
        }
    }
    else if (SRC_FLR == Params_p->SPOChlPCMType)
    {
        SPOSelect = 0x01;
        SelectSPODataSource(FROM_I2SO_DMA);
        SelectF128LRCLKDivMode(SPO_LRCLK_DIV_2);
    }
    else if (SRC_SLR == Params_p->SPOChlPCMType)
    {
        SPOSelect = 0x02;
        SelectSPODataSource(FROM_I2SO_DMA);
        SelectF128LRCLKDivMode(SPO_LRCLK_DIV_3);
    }
    else if (SRC_CSW == Params_p->SPOChlPCMType)
    {
        SPOSelect = 0x03;
        SelectSPODataSource(FROM_I2SO_DMA);
        SelectF128LRCLKDivMode(SPO_LRCLK_DIV_6);       
    }
    else if (SRC_DMLR == Params_p->SPOChlPCMType)
    {
        SPOSelect = 0x0;
        SelectSPODataSource(FROM_I2SO_DMA);
        SelectF128LRCLKDivMode(SPO_LRCLK_DIV_1);
    }
    else
    {
        printk( "SPDIF output source error!\n");
    }

    if (!bPcm)
    {
        EnableSPOBitStreamMode(TRUE);
    }


    while (SPO_Setting[Params_p->MainClk][i])
    {
        if (GETFS(SPO_Setting[Params_p->MainClk][i]) == \
            FSSET((AUD_UINT16)Params_p->SampleRate))
        {
            break;
        }
        
        i++;
    }
    
    if (0 == SPO_Setting[Params_p->MainClk][i])
    {
        printk( "SPDIF output sample rate error [%d][%d]!\n", \
             Params_p->MainClk, i);        
    }
    MainClkSetting = SPO_Setting[Params_p->MainClk][i];


    ClkSelectTmp = GetSystemCtrlRegConfig();
    
    if ((0 == Params_p->MainClk) || (1 == Params_p->MainClk))
    {
        ClkSelectTmp &= (~0x02);
        
    }
    else if ((2 == Params_p->MainClk) || (3 == Params_p->MainClk))
    {
        ClkSelectTmp |= 0x02;
    }

    ConfigSystemCtrlReg(ClkSelectTmp);
    
    if ((0 == bPcm) && \
        (AUDIO_EC3 == Params_p->StreamType) && \
        (Params_p->SampleRate != SAMPLE_RATE_32000))
    {
        ClkSelectTmp = GetSystemCtrlRegConfig(); 
        ClkSelectTmp = (ClkSelectTmp & (~0x2)) | 0x2;
        ConfigSystemCtrlReg(ClkSelectTmp);
    }

    if (SAMPLE_RATE_8000 == Params_p->SampleRate)
    {
        // MainClk/4
        SelectSPOMCLKMode(SPO_CLK_DIV_4);
    }
    else if ((SAMPLE_RATE_16000 == Params_p->SampleRate) || \
             (SAMPLE_RATE_22050 == Params_p->SampleRate) || \
             (SAMPLE_RATE_24000 == Params_p->SampleRate))
    {
        // mclk/2
        SelectSPOMCLKMode(SPO_CLK_DIV_2);
    }
    else
    {
        // mclk
        SelectSPOMCLKMode(SPO_CLK_DIV_0);
    }

    EnableSPONormalPlay(TRUE);
    EnableSPODMADataWithHeader(TRUE);
    
    
    if ((0 == bPcm) && \
        (AUDIO_EC3 == Params_p->StreamType) && \
        (Params_p->SampleRate != SAMPLE_RATE_32000))
    {

       SetSPOLeftChanSampRate(SAMPLE_RATE_192000);
       SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_1);

       SelectDDPSPOClkAndDataMode(DDPSPO_OUT_CLK_DATA);
    }
    else
    {
        SPOFS = MainClkSetting&0xf;
        if (0x00 == SPOFS)
        {
            SetSPOLeftChanSampRate(SAMPLE_RATE_44100);
        }
        else if (0x02 == SPOFS)
        {
            SetSPOLeftChanSampRate(SAMPLE_RATE_48000);
        }
        else if (0x03 == SPOFS)
        {
            SetSPOLeftChanSampRate(SAMPLE_RATE_32000);
        }
        else if (0x04 == SPOFS)
        {
            SetSPOLeftChanSampRate(SAMPLE_RATE_22050);
        }
        else if (0x06 == SPOFS)
        {
            SetSPOLeftChanSampRate(SAMPLE_RATE_24000);
        }
        else if (0x08 == SPOFS)
        {
            SetSPOLeftChanSampRate(SAMPLE_RATE_88200);
        }
        else if (0x09 == SPOFS)
        {
            SetSPOLeftChanSampRate(SAMPLE_RATE_768000);
        }
        else if (0x0A == SPOFS)
        {
            SetSPOLeftChanSampRate(SAMPLE_RATE_96000);
        }
        else if (0x0C == SPOFS)
        {
            SetSPOLeftChanSampRate(SAMPLE_RATE_176400);
        }
        else if (0x0E == SPOFS)
        {
            SetSPOLeftChanSampRate(SAMPLE_RATE_192000);
        }
        

        SPOClkDiv = (MainClkSetting>>4)&0x3;
        if (0x00 == SPOClkDiv)
        {
            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_1);
        }
        else if (0x01 == SPOClkDiv)
        {
            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_2);
        }
        else if (0x02 == SPOClkDiv)
        {
            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_3);
        }
        else if (0x03 == SPOClkDiv)
        {
            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_6);
        }

        // set SPO as hdmi output
        SelectDDPSPOClkAndDataMode(SPO_OUT_CLK_DATA);
    }

    SetSPOLeftChanAudContent((0==bPcm) ? NON_PCM : LINEAR_PCM);
    EnableSPOValidBit((0==bPcm) ? TRUE : FALSE);

    if (AUD_LITTLE_ENDIAN == Params_p->Endian)
    {
        EnableSPODataReorder(FALSE);
    }
    else // AUD_BIG_ENDIAN
    {
        EnableSPODataReorder(TRUE);
    }
    
    SetSPODMADataLen(DATA_LEN_16_BITS);

    SetSPOSampCountThreshold(Params_p->SampleNum);

    /* Make sure every call last index = 0 */
    SetSPODMABufLastIndex(0);

    /* Enable SPO interrupts */
    EnableSPOSampCountInt(TRUE);

    /* Enable SPO interface */
    EnableSPOInterface(TRUE);

    return;
}



static void ConfigDDPSPOInterface(AUD_OutputParams_t *Params_p)
{
    AUD_UINT32 StrType = 0;
    AUD_UINT32 MainClkSetting = 0;
    AUD_UINT32 ClkSelectTmp = 0;
    AUD_UINT32 i = 0;
    AUD_UINT8 bPcm = 1;
    AUD_UINT8 SPOSelect = 0;
    AUD_UINT8 ClkSelect = 0;
    AUD_UINT8 DDPSPOClkDiv = 0;

    if (NULL == Params_p)
    {
        printk( "Input parameters are wrong!\n");
        return;
    }

    StrType = GET_BIT(Params_p->StreamType);

    if (SRC_BUF == Params_p->DDPSPOChlPCMType)
    {
        if ((StrType & (GET_BIT(AUDIO_OGG)|GET_BIT(AUDIO_MP3)|GET_BIT(AUDIO_WMA))) || \
            !(StrType & Params_p->BitStreamSetByUser))
        {
            Params_p->DDPSPOChlPCMType = SRC_DMLR;
        }
        else
        {
            bPcm = 0;
        }
    }

    while (DDPSPO_Setting[Params_p->MainClk][i])
    {
        if (GETFS(DDPSPO_Setting[Params_p->MainClk][i]) == \
            FSSET((AUD_UINT16)Params_p->SampleRate))
        {
            break;
        }
        
        i++;
    }
    
    if (0 == DDPSPO_Setting[Params_p->MainClk][i])
    {
        printk( "DDP SPDIF output sample rate error [%d][%d]!\n", \
             Params_p->MainClk, i);
    }
    MainClkSetting = SPO_Setting[Params_p->MainClk][i];


    ClkSelectTmp = GetSystemCtrlRegConfig();
    
    if ((0 == Params_p->MainClk) || (1 == Params_p->MainClk))
    {
        ClkSelectTmp &= (~0x01);
        
    }
    else if ((2 == Params_p->MainClk) || (3 == Params_p->MainClk))
    {
        ClkSelectTmp |= 0x01;
    }

    ConfigSystemCtrlReg(ClkSelectTmp);
    
    if ((0 == bPcm) && \
        (AUDIO_EC3 == Params_p->StreamType) && \
        (Params_p->SampleRate != SAMPLE_RATE_32000))
    {
        ClkSelectTmp = GetSystemCtrlRegConfig(); 
        ClkSelectTmp = (ClkSelectTmp & (~0x1)) | 0x1;
        ConfigSystemCtrlReg(ClkSelectTmp);
    }

    if (SAMPLE_RATE_8000 == Params_p->SampleRate)
    {
        // MainClk/4
        SelectDDPSPOMCLKDivMode(DDPSPO_MCLK_DIV_4);
    }
    else if ((SAMPLE_RATE_16000 == Params_p->SampleRate) || \
             (SAMPLE_RATE_22050 == Params_p->SampleRate) || \
             (SAMPLE_RATE_24000 == Params_p->SampleRate))
    {
        // mclk/2
        SelectDDPSPOMCLKDivMode(DDPSPO_MCLK_DIV_2);
    }
    else
    {
        // mclk
        SelectDDPSPOMCLKDivMode(DDPSPO_MCLK_DIV_0);
    }

    EnableDDPSPONormalPlay(FALSE);
    
    SelectDDPSPODataSource(DDP_FROM_DDPSPO_DMA);
    SelectDDPSPOMCLKDivMode(DDPSPO_MCLK_DIV_0);
    SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_1);
    
    
    if (((0 == bPcm) && \
         (AUDIO_EC3 == Params_p->StreamType) && \
         (Params_p->SampleRate != SAMPLE_RATE_32000)) || \
        ((AUDIO_EC3 == Params_p->StreamType) && \
         (SAMPLE_RATE_44100 == Params_p->SampleRate)))
    {

       SetDDPSPOLeftChanSampRate(SAMPLE_RATE_192000);
       SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_1);

       SelectDDPSPOClkAndDataMode(DDPSPO_OUT_CLK_DATA);
    }
    else
    {
        SetDDPSPOLeftChanSampRate(SAMPLE_RATE_48000);

        DDPSPOClkDiv = (MainClkSetting>>4)&0x3;
        if (0x00 == DDPSPOClkDiv)
        {
            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_1);
        }
        else if (0x01 == DDPSPOClkDiv)
        {
            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_2);
        }
        else if (0x02 == DDPSPOClkDiv)
        {
            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_3);
        }
        else if (0x03 == DDPSPOClkDiv)
        {
            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_6);
        }

        // set ddp as hdmi output
        SelectDDPSPOClkAndDataMode(SPO_OUT_CLK_DATA);
    }

	if (SPDIF_OUT_FORCE_DD == Params_p->DDPSPOSrcMode)
	{
        SelectDDPSPOClkAndDataMode(SPO_OUT_CLK_DATA);
	}

    //HW will add Pa~Pd
    EnableDDPSPOBitStreamMode(TRUE);
    EnableDDPSPOValidBit(((0==bPcm) ? TRUE : FALSE));
    EnableDDPSPODataReorder(FALSE);
    
    SetDDPSPODMADataLen(DDP_DATA_LEN_16_BITS);


    EnableDDPSPOLeftChanCopyright(TRUE);
    
    // right channel
    SelectDDPSPOChannelStatusReg(DDP_RIGHT_CHAN_STATUS_REG);

    if (1 == bPcm)
    {
        /* if mpeg output, spdif as hdmi output */
        SelectDDPSPOClkAndDataMode(SPO_OUT_CLK_DATA);
    }

    SetDDPSPOSampCountThreshold(Params_p->SampleNum);

    /* Make sure every call last index = 0 */
    SetDDPSPODMABufLastIndex(0);

    /* Enable DDP SPO interrupts */
    EnableDDPSPOSampCountInt(TRUE);
    
    /* Enable DDP SPO interface */            
    EnableDDPSPOInterface(TRUE);

    return;
}



static void AUDREG_HardwareStart(void)
{
    /* Open the DAC */
    OpenDAC(DAC_I2S, Precision_24_BITS);
    
    /* Start the DAC */
    StartDAC();

#if 0
    /* Start DMA */
    StartI2SODMA(TRUE);
    StartSPODMA(TRUE);
    StartDDPSPODMA(TRUE);
#endif

    // start hdmi audio cap first
    // StartHDMIAudioCap(TRUE);

    return;
}



static void AUDREG_HardwarePause(void)
{
    /* Stop the DAC */
    StopDAC();

    return;
}



static void AUDREG_HardwareStop(void)
{    
    /* Stop hdmi audio cap first */
    // StopHDMIAudioCap(FALSE);

    /* Stop the DMA */
    StartI2SODMA(FALSE);
    StartSPODMA(FALSE);
    StartDDPSPODMA(FALSE);

    /* Close the DAC */
    CloseDAC();

    /* Reset the DMA */    
    SetI2SODMABufLastIndex(0);
    SetSPODMABufLastIndex(0);
    SetDDPSPODMABufLastIndex(0);
}



static void AUDREG_HardwareReset(void)
{

    ResetAudCore();
        
    return;
}



static void AUDREG_HardwareMute(AUD_BOOL Enable)
{
    if (TRUE == Enable)
    {
        SetTargetVolume(0);
    
        while (0 != GetCurrentVolume())
        {
            AUD_Delay(1);
        }
        
        StartDAC();
    }
    else if (FALSE == Enable)
    {
        StopDAC();
    }
    else
    {
        SDBBP();
    }

    return;
}



void AUDREG_ConfigInterface(AUD_SubBlock_t SubBlockIdx,
                                   AUD_OutputParams_t *Params_p)
{
    if (NULL == Params_p)
    {
        printk( "Input parameters are wrong!\n");
        return;
    }

    DisableAudClockGate();

    if (SubBlockIdx&AUD_SUB_OUT)
    {
        /* Config I2SO interface */
        ConfigI2SOInterface(Params_p);
    }

    if (SubBlockIdx&AUD_SUB_SPDIFOUT)
    {
        /* Config SPO interface */
        ConfigSPOInterface(Params_p);
    }

    if (SubBlockIdx&AUD_SUB_SPDIFOUT_DDP)
    {
        /* Config DDP SPO interface */
        ConfigDDPSPOInterface(Params_p);
    }

    /* Config Volume*/
    SetTargetVolume(30);

    EnableFadeFunction(TRUE);
    SetFadeSpeed(0x20);

    return;
}

EXPORT_SYMBOL_GPL(AUDREG_ConfigInterface);


static void AUDREG_DoCIPlusVerification(AUD_SPDIF_SCMS_t *ScmsInfo_p)
{
    if (NULL == ScmsInfo_p)
    {
        return;
    }
    
    // Right channel
    SetSPOLeftChanLBit(ScmsInfo_p->LeftBit);
    SetSPOLeftChanCopyright(ScmsInfo_p->Copyright);
    SetSPOLeftChanCategory(ScmsInfo_p->CategoryCode); 
    
    SelectSPOChannelStatusReg(RIGHT_CHAN_STATUS_REG);

    // Left channel
    SetSPOLeftChanLBit(ScmsInfo_p->LeftBit);
    SetSPOLeftChanCopyright(ScmsInfo_p->Copyright);
    SetSPOLeftChanCategory(ScmsInfo_p->CategoryCode);
    
    SelectSPOChannelStatusReg(LEFT_CHAN_STATUS_REG);
}



void AUDREG_ConfigDMA(AUD_SubBlock_t SubBlockIdx,
                             AUD_UINT32 DMABase,
                             AUD_UINT16 DMALen)
{
    if (SubBlockIdx&AUD_SUB_OUT)
    {
        /* Config I2SO DMA */
        SetI2SODMABufBaseAddr(DMABase&0x0FFFFFFF);
        SetI2SODMABufLength(DMALen);
    }

    if (SubBlockIdx&AUD_SUB_SPDIFOUT)
    {
        /* Config SPO DMA */
        SetSPODMABufBaseAddr(DMABase&0x0FFFFFFF);
        SetSPODMABufLength(DMALen);
    }

    if (SubBlockIdx&AUD_SUB_SPDIFOUT_DDP)
    {
        /* Config DDP SPO DMA */
        SetDDPSPODMABufBaseAddr(DMABase&0x0FFFFFFF);
        SetDDPSPODMABufLength(DMALen);
    }

    return;
}

EXPORT_SYMBOL_GPL(AUDREG_ConfigDMA);


static void AUDREG_StartDMA(AUD_SubBlock_t AudSubBlockIndex, 
                            AUD_BOOL Enable)
{
    if (AudSubBlockIndex&AUD_SUB_OUT)
    {
        StartI2SODMA(Enable);
    }

    if (AudSubBlockIndex&AUD_SUB_SPDIFOUT)
    {
        StartSPODMA(Enable);
    }

    if (AudSubBlockIndex&AUD_SUB_SPDIFOUT_DDP)
    {
        StartDDPSPODMA(Enable);
    }

    return;
}



void AUDREG_SetDMALastIndex(AUD_SubBlock_t SubBlockIdx,
                                   AUD_UINT16 Index)
{
    if (SubBlockIdx&AUD_SUB_OUT)
    {
        SetI2SODMABufLastIndex(Index);
    }

    if (SubBlockIdx&AUD_SUB_SPDIFOUT)
    {
        SetSPODMABufLastIndex(Index);
    }

    if (SubBlockIdx&AUD_SUB_SPDIFOUT_DDP)
    {
        SetDDPSPODMABufLastIndex(Index);
    }

    return;
}

EXPORT_SYMBOL_GPL(AUDREG_SetDMALastIndex);


AUD_UINT16 AUDREG_GetDMALastIndex(AUD_SubBlock_t SubBlockIdx)
{
    AUD_UINT16 Index = 0;

    if (SubBlockIdx&AUD_SUB_OUT)
    {
        Index = GetI2SODMABufLastIndex();
    }

    if (SubBlockIdx&AUD_SUB_SPDIFOUT)
    {
        Index = GetSPODMABufLastIndex();
    }

    if (SubBlockIdx&AUD_SUB_SPDIFOUT_DDP)
    {
        Index = GetDDPSPODMABufLastIndex();
    }

    return Index;
}

EXPORT_SYMBOL_GPL(AUDREG_GetDMALastIndex);


AUD_UINT16 AUDREG_GetDMACurrentIndex(AUD_SubBlock_t SubBlockIdx)
{
    AUD_UINT16 Index = 0;

    if (SubBlockIdx&AUD_SUB_OUT)
    {
        Index = GetI2SODMABufCurrentIndex();
    }

    if (SubBlockIdx&AUD_SUB_SPDIFOUT)
    {
        Index = GetSPODMABufCurrentIndex();
    }

    if (SubBlockIdx&AUD_SUB_SPDIFOUT_DDP)
    {
        Index = GetDDPSPODMABufCurrentIndex();
    }

    return Index;
}

EXPORT_SYMBOL_GPL(AUDREG_GetDMACurrentIndex);


static void AUDREG_SetSPOFramePCInfo(AUD_UINT16 PCInfo)
{
    SetSPOFramePCInfo(PCInfo);

    return;
}



static void AUDREG_SetDDPSPOFramePCInfo(AUD_UINT16 PCInfo)
{
    SetDDPSPOFramePCInfo(PCInfo);

    return;
}



void AUDREG_SetVolume(AUD_UINT8 Volume)
{    
    SetTargetVolume(Volume);

    return;
}

EXPORT_SYMBOL_GPL(AUDREG_SetVolume);


AUD_UINT8 AUDREG_GetVolume(void)
{    
    return GetCurrentVolume();
}

EXPORT_SYMBOL_GPL(AUDREG_GetVolume);



static void AUDREG_SetSTCValue(AUD_UINT8 STCNum,
                               AUD_UINT32 Value)
{
    SetSTCValue(STCNum, Value);

    return;
}



static AUD_UINT32 AUDREG_GetSTCValue(AUD_UINT8 STCNum)
{
    return GetSTCValue(STCNum);
}



static void AUDREG_SetSTCDivisor(AUD_UINT8 STCNum,
                                 AUD_UINT32 Divisor)
{
    SetSTCDivisor(STCNum, Divisor);

    return;
}



static AUD_UINT16 AUDREG_GetSTCDivisor(AUD_UINT8 STCNum)
{
    return GetSTCDivisor(STCNum);
}



static void AUDREG_Interrupt(AUD_UINT32 pdev)
{
    AUD_UINT8 status;
#if 0    
    if (CheckI2SOSampCountIntStatus())
    {
        if (InterruptServiceFunctionTable.AUD_I2SOSampleCountService)
        {
            osal_interrupt_register_hsr(InterruptServiceFunctionTable.AUD_I2SOSampleCountService, \
                                        (AUD_UINT32)0);
        }
    }

    if(CheckI2SODMABufUnderRunIntStatus())
    {
        if (InterruptServiceFunctionTable.AUD_I2SOBuffUnderrunService)
        {
            osal_interrupt_register_hsr(InterruptServiceFunctionTable.AUD_I2SOBuffUnderrunService, \
                                        (AUD_UINT32)0);
        }
    }      
#endif
    return;
}

#if 0

static AUD_UINT16 AUDREG_InterruptRegister(AUD_InterruptServiceTables_t *FunctionTable)
{
    if (FunctionTable)
    {
        InterruptServiceFunctionTable.AUD_DDPSPOBuffResumeService= \
            FunctionTable->AUD_DDPSPOBuffResumeService;
        InterruptServiceFunctionTable.AUD_DDPSPOBuffUnderrunService= \
            FunctionTable->AUD_DDPSPOBuffUnderrunService;
        InterruptServiceFunctionTable.AUD_DDPSPOSampleCountService= \
            FunctionTable->AUD_DDPSPOSampleCountService;

        InterruptServiceFunctionTable.AUD_SPOBuffResumeService= \
            FunctionTable->AUD_SPOBuffResumeService;
        InterruptServiceFunctionTable.AUD_SPOBuffUnderrunService= \
            FunctionTable->AUD_SPOBuffUnderrunService;
        InterruptServiceFunctionTable.AUD_SPOSampleCountService= \
            FunctionTable->AUD_SPOSampleCountService;

        InterruptServiceFunctionTable.AUD_I2SOBuffResumeService= \
            FunctionTable->AUD_I2SOBuffResumeService;
        InterruptServiceFunctionTable.AUD_I2SOBuffUnderrunService= \
            FunctionTable->AUD_I2SOBuffUnderrunService;
        InterruptServiceFunctionTable.AUD_I2SOSampleCountService= \
            FunctionTable->AUD_I2SOSampleCountService;

        return osal_interrupt_register_lsr(AUD_INTERRUPT_NUMBER, \
                                           AUDREG_Interrupt, \
                                           (AUD_UINT32)0);

    }
    return 0;
}
 #endif   


static void AUDREG_InterruptUnRegister(void)
{
#if 0
    osal_interrupt_unregister_lsr(AUD_INTERRUPT_NUMBER, AUDREG_Interrupt);

    InterruptServiceFunctionTable.AUD_DDPSPOBuffResumeService = NULL;
    InterruptServiceFunctionTable.AUD_DDPSPOBuffUnderrunService = NULL;
    InterruptServiceFunctionTable.AUD_DDPSPOSampleCountService = NULL;


    InterruptServiceFunctionTable.AUD_SPOBuffResumeService = NULL;
    InterruptServiceFunctionTable.AUD_SPOBuffUnderrunService = NULL;
    InterruptServiceFunctionTable.AUD_SPOSampleCountService = NULL;

    InterruptServiceFunctionTable.AUD_I2SOBuffResumeService = NULL;
    InterruptServiceFunctionTable.AUD_I2SOBuffUnderrunService = NULL;
    InterruptServiceFunctionTable.AUD_I2SOSampleCountService = NULL; 
#endif
    return;
}



static AUD_UINT16 AUDREG_GetI2SOSampCountThreshold(void)
{
    // reg offset: 0x10 ; bit: 15-0
    return GetI2SOSampCountThreshold();

}

AUD_BOOL GetI2SODMAStatus(void)
{

    // reg offset: 0x15 ; bit: 4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DMA_CTRL_0X15);

    if(RegValue&(0x01<<4))
      return TRUE;
    else
      return FALSE;


}

EXPORT_SYMBOL_GPL(GetI2SODMAStatus);

AUD_BOOL GetSPODMAStatus(void)
{

    // reg offset: 0x15 ; bit: 0

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_DMA_CTRL_0X15);

    if(RegValue&(0x01<<0))
      return TRUE;
    else
      return FALSE;


}

EXPORT_SYMBOL_GPL(GetSPODMAStatus);   

void AUDREG_ClearAllAudioInterrupt(void)
{
    // reg offset: 0x01 ; bit: 4-7

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_INT_0X01);


   AUD_WriteRegDev8(R_AUD_INT_0X01, \
                   (RegValue|0xF0));


}

EXPORT_SYMBOL_GPL(AUDREG_ClearAllAudioInterrupt);

void AUDREG_DisableAllAudioInterrupt(void)
{
    // reg offset: 0x01 ; bit: 0-4

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_INT_0X01);


   AUD_WriteRegDev8(R_AUD_INT_0X01, \
                   (RegValue&0xF0));


}
EXPORT_SYMBOL_GPL(AUDREG_DisableAllAudioInterrupt);

AUD_UINT32 GetDMABufUnderRunIntStatus(void)
{
    // reg offset: 0x14 ; bit: 7-6

    AUD_UINT8 RegValue = 0;
    
    RegValue = AUD_ReadRegDev8(R_BUF_UNDER_RUN_CTRL_0X14);

   return (RegValue&(0x3<<6));

}

EXPORT_SYMBOL_GPL(GetDMABufUnderRunIntStatus);

AUD_UINT32 GetInterfaceEnableStatus(void)
{
    // reg offset: 0x00 ; bit: 0-1

    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_AUD_CTRL_0X00);

   return (RegValue&(0x3<<0));

}
EXPORT_SYMBOL_GPL(GetInterfaceEnableStatus);



