
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
#include <asm/uaccess.h>
#include "../../AUD_common.h"
#include "../../AUD_reg.h"
/* -----------------------------------------------------------------------------
S3921 Chip base address definition
----------------------------------------------------------------------------- */
#define S3921_SYS_BASE_ADDR                       0x18000000
#define S3921_AE_BASE_ADDR                        0x1804A000
#define S3921_SND_BASE_ADDR                       0x18002000


/* -----------------------------------------------------------------------------
S3921 Chip Registers definition
----------------------------------------------------------------------------- */
#define R_SYS_INT_POLARITY_0X28                   (S3921_SYS_BASE_ADDR + 0x28)
#define R_SYS_INT_STATUS_0X30                     (S3921_SYS_BASE_ADDR + 0x30)
#define R_SYS_INT_ENABLE_0X38                     (S3921_SYS_BASE_ADDR + 0x38)
#define R_SYS_DEVICE_CLK_GATING_CTRL0_0X60        (S3921_SYS_BASE_ADDR + 0x60)
#define R_SYS_DEVICE_CLK_GATING_CTRL1_0X64        (S3921_SYS_BASE_ADDR + 0x64)
#define R_SYS_DEVICE_CLK_GATING_CTRL3_0X90        (S3921_SYS_BASE_ADDR + 0x90)
#define R_SYS_DEVICE_RESET_CTRL_0X80              (S3921_SYS_BASE_ADDR + 0x80)
#define R_SYS_DEVICE_SPDIF_CTRL_0X88              (S3921_SYS_BASE_ADDR + 0x88)



/******************************************************************************/
/************ Audio Engine Registers definition *******************************/
/******************************************************************************/
/* SEE CPU Read-Only Reg, AUD CPU0 Read/Write Reg */
#define R_AUD_CPU0_CTRL_R_0X00                   (S3921_AE_BASE_ADDR + 0x400)
/* For Audio Decoder */
#define R_AUD_CPU0_DEC_DMA_CTRL_R_0X10           (S3921_AE_BASE_ADDR + 0x410)
#define R_AUD_CPU0_DEC_DMA_IN_RD_INDEX_0X14      (S3921_AE_BASE_ADDR + 0x414)
#define R_AUD_CPU0_DEC_DMA_OUT_WR_INDEX_0X18     (S3921_AE_BASE_ADDR + 0x418)
#define R_AUD_CPU0_DEC_DMA_IN_FRAME_LEN_0X1C     (S3921_AE_BASE_ADDR + 0x41C)
#define R_AUD_CPU0_DEC_DMA_IN_FRAME_INFO_0X20    (S3921_AE_BASE_ADDR + 0x420)
#define R_AUD_CPU0_DEC_DMA_OUT_FRAME_ST_0X24     (S3921_AE_BASE_ADDR + 0x424)
#define R_AUD_CPU0_DEC_DMA_OUT_FRAME_FS_0X28     (S3921_AE_BASE_ADDR + 0x428)
#define R_AUD_CPU0_ENC_DMA_CTRL_R_0X30           (S3921_AE_BASE_ADDR + 0x430)
#define R_AUD_CPU0_ENC_DMA_IN_RD_INDEX_0X34      (S3921_AE_BASE_ADDR + 0x434)
#define R_AUD_CPU0_ENC_DMA_OUT_WR_INDEX_0X38     (S3921_AE_BASE_ADDR + 0x438)
#define R_AUD_CPU0_ENC_DMA_IN_FRAME_INFO_0X3C    (S3921_AE_BASE_ADDR + 0x43C)
#define R_AUD_CPU0_ENC_DMA_IN_FRAME_FS_0X40      (S3921_AE_BASE_ADDR + 0x440)
#define R_AUD_CPU0_ENC_DMA_OUT_FRAME_ST_0X44     (S3921_AE_BASE_ADDR + 0x444)

/* SEE CPU0 Read/Write Reg, AUD CPU0 Read-Only Reg */
#define R_AUD_CPU0_CTRL_W_0X100                  (S3921_AE_BASE_ADDR + 0x500)
/* For Audio Decoder */
#define R_AUD_CPU0_DEC_DMA_CTRL_W_0X110          (S3921_AE_BASE_ADDR + 0x510)
#define R_AUD_CPU0_DEC_DMA_IN_BASE_ADDR_0X114    (S3921_AE_BASE_ADDR + 0x514)
#define R_AUD_CPU0_DEC_DMA_IN_LEN_0X118          (S3921_AE_BASE_ADDR + 0x518)
#define R_AUD_CPU0_DEC_DMA_IN_WR_INDEX_0X11C     (S3921_AE_BASE_ADDR + 0x51C)
#define R_AUD_CPU0_DEC_DMA_OUT_BASE_ADDR_0X120   (S3921_AE_BASE_ADDR + 0x520)
#define R_AUD_CPU0_DEC_DMA_OUT_LEN_0X124         (S3921_AE_BASE_ADDR + 0x524)
#define R_AUD_CPU0_DEC_DMA_OUT_RD_INDEX_0X128    (S3921_AE_BASE_ADDR + 0x528)
#define R_AUD_CPU0_DEC_PARA0_0X150               (S3921_AE_BASE_ADDR + 0x550)
#define R_AUD_CPU0_DEC_PARA1_0X154               (S3921_AE_BASE_ADDR + 0x554)
#define R_AUD_CPU0_DEC_PARA2_0X158               (S3921_AE_BASE_ADDR + 0x558)
#define R_AUD_CPU0_DEC_PARA3_0X15C               (S3921_AE_BASE_ADDR + 0x55C)
#define R_AUD_CPU0_DEC_PARA4_0X160               (S3921_AE_BASE_ADDR + 0x560)
#define R_AUD_CPU0_DEC_PARA5_0X164               (S3921_AE_BASE_ADDR + 0x564)

/* For Audio Encoder */
#define R_AUD_CPU0_ENC_DMA_CTRL_W_0X130          (S3921_AE_BASE_ADDR + 0x530)
#define R_AUD_CPU0_ENC_DMA_IN_BASE_ADDR_0X134    (S3921_AE_BASE_ADDR + 0x534)
#define R_AUD_CPU0_ENC_DMA_IN_LEN_0X138          (S3921_AE_BASE_ADDR + 0x538)
#define R_AUD_CPU0_ENC_DMA_IN_WR_INDEX_0X13C     (S3921_AE_BASE_ADDR + 0x53C)
#define R_AUD_CPU0_ENC_DMA_OUT_BASE_ADDR_0X140   (S3921_AE_BASE_ADDR + 0x540)
#define R_AUD_CPU0_ENC_DMA_OUT_LEN_0X144         (S3921_AE_BASE_ADDR + 0x544)
#define R_AUD_CPU0_ENC_DMA_OUT_RD_INDEX_0X148    (S3921_AE_BASE_ADDR + 0x548)


/* SEE CPU1 Read-Only Reg, AUD CPU1 Read/Write Reg */
#define R_AUD_CPU1_CTRL_R_0X80                   (S3921_AE_BASE_ADDR + 0x480)
/* For Audio Decoder */
#define R_AUD_CPU1_DEC_DMA_CTRL_R_0X90           (S3921_AE_BASE_ADDR + 0x490)
#define R_AUD_CPU1_DEC_DMA_IN_RD_INDEX_0X94      (S3921_AE_BASE_ADDR + 0x494)
#define R_AUD_CPU1_DEC_DMA_OUT_WR_INDEX_0X98     (S3921_AE_BASE_ADDR + 0x498)
#define R_AUD_CPU1_DEC_DMA_IN_FRAME_LEN_0X9C     (S3921_AE_BASE_ADDR + 0x49C)
#define R_AUD_CPU1_DEC_DMA_IN_FRAME_INFO_0XA0    (S3921_AE_BASE_ADDR + 0x4A0)
#define R_AUD_CPU1_DEC_DMA_OUT_FRAME_ST_0XA4     (S3921_AE_BASE_ADDR + 0x4A4)
#define R_AUD_CPU1_DEC_DMA_OUT_FRAME_FS_0XA8     (S3921_AE_BASE_ADDR + 0x4A8)
/* For Audio Encoder */
#define R_AUD_CPU1_ENC_DMA_CTRL_R_0XB0           (S3921_AE_BASE_ADDR + 0x4B0)
#define R_AUD_CPU1_ENC_DMA_IN_RD_INDEX_0XB4      (S3921_AE_BASE_ADDR + 0x4B4)
#define R_AUD_CPU1_ENC_DMA_OUT_WR_INDEX_0XB8     (S3921_AE_BASE_ADDR + 0x4B8)
#define R_AUD_CPU1_ENC_DMA_IN_FRAME_INFO_0XBC    (S3921_AE_BASE_ADDR + 0x4BC)
#define R_AUD_CPU1_ENC_DMA_IN_FRAME_FS_0XC0      (S3921_AE_BASE_ADDR + 0x4C0)
#define R_AUD_CPU1_ENC_DMA_OUT_FRAME_ST_0XC4     (S3921_AE_BASE_ADDR + 0x4C4)

/* SEE CPU1 Read/Write Reg, AUD CPU1 Read-Only Reg */
#define R_AUD_CPU1_CTRL_W_0X180                  (S3921_AE_BASE_ADDR + 0x580)
/* For Audio Decoder */
#define R_AUD_CPU1_DEC_DMA_CTRL_W_0X190          (S3921_AE_BASE_ADDR + 0x590)
#define R_AUD_CPU1_DEC_DMA_IN_BASE_ADDR_0X194    (S3921_AE_BASE_ADDR + 0x594)
#define R_AUD_CPU1_DEC_DMA_IN_LEN_0X198          (S3921_AE_BASE_ADDR + 0x598)
#define R_AUD_CPU1_DEC_DMA_IN_WR_INDEX_0X19C     (S3921_AE_BASE_ADDR + 0x59C)
#define R_AUD_CPU1_DEC_DMA_OUT_BASE_ADDR_0X1A0   (S3921_AE_BASE_ADDR + 0x5A0)
#define R_AUD_CPU1_DEC_DMA_OUT_LEN_0X1A4         (S3921_AE_BASE_ADDR + 0x5A4)
#define R_AUD_CPU1_DEC_DMA_OUT_RD_INDEX_0X1A8    (S3921_AE_BASE_ADDR + 0x5A8)
#define R_AUD_CPU1_DEC_PARA0_0X1D0               (S3921_AE_BASE_ADDR + 0x5D0)
#define R_AUD_CPU1_DEC_PARA1_0X1D4               (S3921_AE_BASE_ADDR + 0x5D4)
#define R_AUD_CPU1_DEC_PARA2_0X1D8               (S3921_AE_BASE_ADDR + 0x5D8)
#define R_AUD_CPU1_DEC_PARA3_0X1DC               (S3921_AE_BASE_ADDR + 0x5DC)
#define R_AUD_CPU1_DEC_PARA4_0X1E0               (S3921_AE_BASE_ADDR + 0x5E0)
#define R_AUD_CPU1_DEC_PARA5_0X1E4               (S3921_AE_BASE_ADDR + 0x5E4)

/* For Audio Encoder */
#define R_AUD_CPU1_ENC_DMA_CTRL_W_0X1B0          (S3921_AE_BASE_ADDR + 0x5B0)
#define R_AUD_CPU1_ENC_DMA_IN_BASE_ADDR_0X1B4    (S3921_AE_BASE_ADDR + 0x5B4)
#define R_AUD_CPU1_ENC_DMA_IN_LEN_0X1B8          (S3921_AE_BASE_ADDR + 0x5B8)
#define R_AUD_CPU1_ENC_DMA_IN_WR_INDEX_0X1BC     (S3921_AE_BASE_ADDR + 0x5BC)
#define R_AUD_CPU1_ENC_DMA_OUT_BASE_ADDR_0X1C0   (S3921_AE_BASE_ADDR + 0x5C0)
#define R_AUD_CPU1_ENC_DMA_OUT_LEN_0X1C4         (S3921_AE_BASE_ADDR + 0x5C4)
#define R_AUD_CPU1_ENC_DMA_OUT_RD_INDEX_0X1C8    (S3921_AE_BASE_ADDR + 0x5C8)


/* SEE CPU for Audio CPU0 registers */
#define R_AUD_NORTH_BRG_ID                        (S3921_AE_BASE_ADDR + 0x00)
#define R_AUD_ENGINE_ILB_SEL                      (S3921_AE_BASE_ADDR + 0x04)
#define R_SEE_AUD0_INT_STS_REG2                   (S3921_AE_BASE_ADDR + 0x34)
#define R_SEE_AUD0_INT_EN_REG2                    (S3921_AE_BASE_ADDR + 0x3C)

#define R_SEE_TO_AUD0_MAILBOX0                    0x200
#define R_SEE_TO_AUD0_MAILBOX1                    0x204
#define R_SEE_TO_AUD0_MAILBOX2                    0x208
#define R_SEE_TO_AUD0_MAILBOX3                    0x20C

#define R_AUD0_TO_SEE_MAILBOX0                    0x210
#define R_AUD0_TO_SEE_MAILBOX1                    0x214
#define R_AUD0_TO_SEE_MAILBOX2                    0x218
#define R_AUD0_TO_SEE_MAILBOX3                    0x21C

#define R_AUD0_BOOT_CODE0                         0x280
#define R_AUD0_BOOT_CODE1                         0x284
#define R_AUD0_BOOT_CODE2                         0x288
#define R_AUD0_BOOT_CODE3                         0x28C

/* SEE CPU for Audio CPU1 registers */
#define R_SEE_AUD1_INT_STS_REG2                   0x34 
#define R_SEE_AUD1_INT_EN_REG2                    0x3C

#define R_SEE_TO_AUD1_MAILBOX0                    0x200
#define R_SEE_TO_AUD1_MAILBOX1                    0x204
#define R_SEE_TO_AUD1_MAILBOX2                    0x208
#define R_SEE_TO_AUD1_MAILBOX3                    0x20C

#define R_AUD1_TO_SEE_MAILBOX0                    0x210
#define R_AUD1_TO_SEE_MAILBOX1                    0x214
#define R_AUD1_TO_SEE_MAILBOX2                    0x218
#define R_AUD1_TO_SEE_MAILBOX3                    0x21C

#define R_AUD1_BOOT_CODE0                         0x280
#define R_AUD1_BOOT_CODE1                         0x284
#define R_AUD1_BOOT_CODE2                         0x288
#define R_AUD1_BOOT_CODE3                         0x28C

/***********************************************************/
#define SEE_TO_AUD0_MB0_INT                       0x80000000
#define SEE_TO_AUD0_MB1_INT                       0x40000000
#define SEE_TO_AUD0_MB2_INT                       0x20000000
#define SEE_TO_AUD0_MB3_INT                       0x10000000

#define CLR_AUD0_TO_SEE_MB0_INT                   0x80000000
#define CLR_AUD0_TO_SEE_MB1_INT                   0x40000000
#define CLR_AUD0_TO_SEE_MB2_INT                   0x20000000
#define CLR_AUD0_TO_SEE_MB3_INT                   0x10000000

#define SEE_TO_AUD0_MB0_INT_AST   (SEE_TO_AUD0_MB0_INT >> 4)
#define SEE_TO_AUD0_MB1_INT_AST   (SEE_TO_AUD0_MB0_INT >> 4)
#define SEE_TO_AUD0_MB2_INT_AST   (SEE_TO_AUD0_MB0_INT >> 4)
#define SEE_TO_AUD0_MB3_INT_AST   (SEE_TO_AUD0_MB0_INT >> 4)

/***********************************************************/
#define SEE_TO_AUD1_MB0_INT                       0x80000000
#define SEE_TO_AUD1_MB1_INT                       0x40000000
#define SEE_TO_AUD1_MB2_INT                       0x20000000
#define SEE_TO_AUD1_MB3_INT                       0x10000000

#define CLR_AUD1_TO_SEE_MB0_INT                   0x80000000
#define CLR_AUD1_TO_SEE_MB1_INT                   0x40000000
#define CLR_AUD1_TO_SEE_MB2_INT                   0x20000000
#define CLR_AUD1_TO_SEE_MB3_INT                   0x10000000

#define SEE_TO_AUD1_MB0_INT_AST  (SEE_TO_AUD1_MB0_INT >> 4)
#define SEE_TO_AUD1_MB1_INT_AST  (SEE_TO_AUD1_MB1_INT >> 4)
#define SEE_TO_AUD1_MB2_INT_AST  (SEE_TO_AUD1_MB2_INT >> 4)
#define SEE_TO_AUD1_MB3_INT_AST  (SEE_TO_AUD1_MB3_INT >> 4)

/* mailbox irq id */
#define AUD0_TO_SEE_MAILBOX0_IRQ_ID               (27+32+8)
#define AUD0_TO_SEE_MAILBOX1_IRQ_ID               (26+32+8)
#define AUD0_TO_SEE_MAILBOX2_IRQ_ID               (25+32+8)
#define AUD0_TO_SEE_MAILBOX3_IRQ_ID               (24+32+8)

#define AUD1_TO_SEE_MAILBOX0_IRQ_ID               (31+32+8)
#define AUD1_TO_SEE_MAILBOX1_IRQ_ID               (30+32+8)
#define AUD1_TO_SEE_MAILBOX2_IRQ_ID               (29+32+8)
#define AUD1_TO_SEE_MAILBOX3_IRQ_ID               (28+32+8)




/******************************************************************************/
/************ S3921 Audio Registers definition ********************************/
/******************************************************************************/
/* AUD_I2SO Interface Description */
#define R_AUD_IP_VERSION_0X00                      (S3921_SND_BASE_ADDR + 0x00)
#define R_AUD_INTF_EN_0X04                         (S3921_SND_BASE_ADDR + 0x04)
#define R_AUD_INT_ST1_0X08                         (S3921_SND_BASE_ADDR + 0x08)
#define R_AUD_INT_ST2_0X0C                         (S3921_SND_BASE_ADDR + 0x0C)
#define R_AUD_SRC_CLK_SEL_0X10                     (S3921_SND_BASE_ADDR + 0x10)
#define R_AUD_FS_CFG1_0X14                         (S3921_SND_BASE_ADDR + 0x14)
#define R_AUD_FS_CFG2_0X18                         (S3921_SND_BASE_ADDR + 0x18)
#define R_IMB_ARBT_ST_0X20                         (S3921_SND_BASE_ADDR + 0x20)
#define R_I2SO_DMA_BASE_ADDR_0X30                  (S3921_SND_BASE_ADDR + 0x30)
#define R_I2SO_DMA_CFG_0X34                        (S3921_SND_BASE_ADDR + 0x34)
#define R_I2SO_DMA_INDEX_0X38                      (S3921_SND_BASE_ADDR + 0x38)
#define R_I2SO_DMA_UNDER_RUN_0X3C                  (S3921_SND_BASE_ADDR + 0x3C)
#define R_I2SO_FRAME_HEAD_INFO_0X40                (S3921_SND_BASE_ADDR + 0x40)
#define R_I2SO_FRAME_HEAD_INFO_PTS_0X44            (S3921_SND_BASE_ADDR + 0x44)
#define R_I2SO_DMA_SKIP_NUM_0X48                   (S3921_SND_BASE_ADDR + 0x48)
#define R_I2SO_FRAME_COUNTER_0X4C                  (S3921_SND_BASE_ADDR + 0x4C)
#define R_I2SO_INTF_CTRL_0X50                      (S3921_SND_BASE_ADDR + 0x50)
#define R_I2SO_FS_CFG_0X54                         (S3921_SND_BASE_ADDR + 0x54)
#define R_F128_FS_CFG_0X58                         (S3921_SND_BASE_ADDR + 0x58)
#define R_I2SO_SAMPLE_COUNTER_0X5C                 (S3921_SND_BASE_ADDR + 0x5C)
#define R_I2SO_DEBUG1_0X60                         (S3921_SND_BASE_ADDR + 0x60)
#define R_I2SO_DEBUG2_0X64                         (S3921_SND_BASE_ADDR + 0x64)
#define R_AUD_TEST_PIN_SEL_0X68                    (S3921_SND_BASE_ADDR + 0x68)
#define R_I2SO_FADE_CTRL_0X90                      (S3921_SND_BASE_ADDR + 0x90)
#define R_PRE_FADE_OUT_0X94                        (S3921_SND_BASE_ADDR + 0x94)
#define R_90K_F0_0XB0                              (S3921_SND_BASE_ADDR + 0xB0)
#define R_90K_F0_2_0XB4                            (S3921_SND_BASE_ADDR + 0xB4)
#define R_90K_F1_0XB8                              (S3921_SND_BASE_ADDR + 0xB8)
#define R_90K_F1_2_0XBC                            (S3921_SND_BASE_ADDR + 0xBC)
#define R_DAC_I2S_REG_0XD0                         (S3921_SND_BASE_ADDR + 0xD0)
#define R_DAC_CODEC_REG1_0XD4                      (S3921_SND_BASE_ADDR + 0xD4)
#define R_DAC_CODEC_REG2_0XD8                      (S3921_SND_BASE_ADDR + 0xD8)
#define R_DAC_CODEC_ERG3_0XDC                      (S3921_SND_BASE_ADDR + 0xDc)
/* AUD_SPO Interface Registers */
#define R_SPO_DMA_BASE_ADDR_0X100                  (S3921_SND_BASE_ADDR + 0x100)
#define R_SPO_DMA_CFG_0X104                        (S3921_SND_BASE_ADDR + 0x104)
#define R_SPO_DMA_INDEX_0X108                      (S3921_SND_BASE_ADDR + 0x108)
#define R_SPO_DMA_UNDER_RUN_0X10C                  (S3921_SND_BASE_ADDR + 0x10C)
#define R_SPO_INTF_CTRL_0X110                      (S3921_SND_BASE_ADDR + 0x110)
#define R_SPO_IEC_PC_0X114                         (S3921_SND_BASE_ADDR + 0x114)
#define R_SPO_FS_CFG_0X118                         (S3921_SND_BASE_ADDR + 0x118)
#define R_SPO_SAMPLE_COUNTER_0X11C                 (S3921_SND_BASE_ADDR + 0x11C)
#define R_SPO_FRAME_HEAD_INFO_0X120                (S3921_SND_BASE_ADDR + 0x120)
#define R_SPO_FRAME_HEAD_INFO2_0X124               (S3921_SND_BASE_ADDR + 0x124)
#define R_SPO_DMA_SKIP_NUM_0X128                   (S3921_SND_BASE_ADDR + 0x128)
#define R_SPO_LEFT_USER_DATA1_0X130                (S3921_SND_BASE_ADDR + 0x130)
#define R_SPO_LEFT_USER_DATA2_0X134                (S3921_SND_BASE_ADDR + 0x134)
#define R_SPO_LEFT_USER_DATA3_0X138                (S3921_SND_BASE_ADDR + 0x138)
#define R_SPO_LEFT_USER_DATA4_0X13C                (S3921_SND_BASE_ADDR + 0x13C)
#define R_SPO_LEFT_USER_DATA5_0X140                (S3921_SND_BASE_ADDR + 0x140)
#define R_SPO_LEFT_USER_DATA6_0X144                (S3921_SND_BASE_ADDR + 0x144)
#define R_SPO_RIGHT_USER_DATA1_0X148               (S3921_SND_BASE_ADDR + 0x148)
#define R_SPO_RIGHT_USER_DATA2_0X14C               (S3921_SND_BASE_ADDR + 0x14C)
#define R_SPO_RIGHT_USER_DATA3_0X150               (S3921_SND_BASE_ADDR + 0x150)
#define R_SPO_RIGHT_USER_DATA4_0X154               (S3921_SND_BASE_ADDR + 0x154)
#define R_SPO_RIGHT_USER_DATA5_0X158               (S3921_SND_BASE_ADDR + 0x158)
#define R_SPO_RIGHT_USER_DATA6_0X15C               (S3921_SND_BASE_ADDR + 0x15C)
#define R_SPO_CHANNEL_STATUS_L_0X160               (S3921_SND_BASE_ADDR + 0x160)
#define R_SPO_CHANNEL_STATUS_R_0X164               (S3921_SND_BASE_ADDR + 0x164)
#define R_SPO_DEBUG1_0X170                         (S3921_SND_BASE_ADDR + 0x170)
#define R_SPO_DEBUG2_0X174                         (S3921_SND_BASE_ADDR + 0x174)
#define R_SPO_DEBUG3_0X178                         (S3921_SND_BASE_ADDR + 0x178)
#define R_SPO_DEBUG4_0X17C                         (S3921_SND_BASE_ADDR + 0x17C)
#define R_DDP_SPO_DMA_BASE_ADDR_0X180              (S3921_SND_BASE_ADDR + 0x180)
#define R_DDP_SPO_DMA_CFG_0X184                    (S3921_SND_BASE_ADDR + 0x184)
#define R_DDP_SPO_DMA_INDEX_0X188                  (S3921_SND_BASE_ADDR + 0x188)
#define R_DDP_SPO_DMA_UNDER_RUN_0X18C              (S3921_SND_BASE_ADDR + 0x18C)
#define R_DDP_SPO_INTF_CTRL_0X190                  (S3921_SND_BASE_ADDR + 0x190)
#define R_DDP_SPO_IEC_PC_0X194                     (S3921_SND_BASE_ADDR + 0x194)
#define R_DDP_SPO_FS_CFG_0X198                     (S3921_SND_BASE_ADDR + 0x198)
#define R_DDP_SPO_SMAPLE_COUNTER_0X19C             (S3921_SND_BASE_ADDR + 0x19C)
#define R_DDP_SPO_FRAME_HEAD_INFO_0X1A0            (S3921_SND_BASE_ADDR + 0x1A0)
#define R_DDP_SPO_FRAME_HEAD_INFT2_0X1A4           (S3921_SND_BASE_ADDR + 0x1A4)
#define R_DDP_SPO_DMA_SKIP_NUM_0X1A8               (S3921_SND_BASE_ADDR + 0x1A8)
#define R_DDP_SPO_LEFT_USER_DATA1_0X1B0            (S3921_SND_BASE_ADDR + 0x1B0)
#define R_DDP_SPO_LEFT_USER_DATA2_0X1B4            (S3921_SND_BASE_ADDR + 0x1B4)
#define R_DDP_SPO_LEFT_USER_DATA3_0X1B8            (S3921_SND_BASE_ADDR + 0x1B8)
#define R_DDP_SPO_LEFT_USER_DATA4_0X1BC            (S3921_SND_BASE_ADDR + 0x1BC)
#define R_DDP_SPO_LEFT_USER_DATA5_0X1C0            (S3921_SND_BASE_ADDR + 0x1C0)
#define R_DDP_SPO_LEFT_USER_DATA6_0X1C4            (S3921_SND_BASE_ADDR + 0x1C4)
#define R_DDP_SPO_RIGHT_USER_DATA1_0X1C8           (S3921_SND_BASE_ADDR + 0x1C8)
#define R_DDP_SPO_RIGHT_USER_DATA2_0X1CC           (S3921_SND_BASE_ADDR + 0x1CC)
#define R_DDP_SPO_RIGHT_USER_DATA3_0X1D0           (S3921_SND_BASE_ADDR + 0x1D0)
#define R_DDP_SPO_RIGHT_USER_DATA4_0X1D4           (S3921_SND_BASE_ADDR + 0x1D4)
#define R_DDP_SPO_RIGHT_USER_DATA5_0X1D8           (S3921_SND_BASE_ADDR + 0x1D8)
#define R_DDP_SPO_RIGHT_USER_DATA6_0X1DC           (S3921_SND_BASE_ADDR + 0x1DC)
#define R_DDP_SPO_CHANNEL_STATUS_L_0X1E0           (S3921_SND_BASE_ADDR + 0x1E0)
#define R_DDP_SPO_CHANNEL_STATUS_R_0X1E4           (S3921_SND_BASE_ADDR + 0x1E4)
#define R_DDP_SPO_DEBUG1_0X1F0                     (S3921_SND_BASE_ADDR + 0x1F0)
#define R_DDP_SPO_DEBUG2_0X1F4                     (S3921_SND_BASE_ADDR + 0x1F4)
#define R_DDP_SPO_DEBUG3_0X1F8                     (S3921_SND_BASE_ADDR + 0x1F8)
#define R_DDP_SPO_DEBUG4_0X1FC                     (S3921_SND_BASE_ADDR + 0x1FC)
/* AUD_I2SI Interface Registers */
#define R_I2SI_RX_DMA_BASE_ADDR_0X200              (S3921_SND_BASE_ADDR + 0x200)
#define R_I2SI_RX_DMA_CFG_0X204                    (S3921_SND_BASE_ADDR + 0x204)
#define R_I2SI_RX_DMA_INDEX_0X208                  (S3921_SND_BASE_ADDR + 0x208)
#define R_I2SI_RX_DMA_OVER_FOLW_0X20C              (S3921_SND_BASE_ADDR + 0x20C)
#define R_I2SI_RX_SAMPLE_COUNTER_0X210             (S3921_SND_BASE_ADDR + 0x210)
#define R_I2SI_CODEC_REG_0X214                     (S3921_SND_BASE_ADDR + 0x214)
#define R_I2SI_VOLUME_CTRL_REG_0X218               (S3921_SND_BASE_ADDR + 0x218)
#define R_I2SI_RX_DEBUG1_0X220                     (S3921_SND_BASE_ADDR + 0x220)
#define R_I2SI_RX_DEBUG2_0X224                     (S3921_SND_BASE_ADDR + 0x224)
#define R_I2SI_TX_DMA_BASE_ADDR_0X230              (S3921_SND_BASE_ADDR + 0x230)
#define R_I2SI_TX_DMA_CFG_0X234                    (S3921_SND_BASE_ADDR + 0x234)
#define R_I2SI_TX_DMA_INDEX_0X238                  (S3921_SND_BASE_ADDR + 0x238)
#define R_I2SI_TX_DMA_UNDER_RUN_0X23C              (S3921_SND_BASE_ADDR + 0x23C)
#define R_I2SI_TX_SAMPLE_COUNTER_0X240             (S3921_SND_BASE_ADDR + 0x240)
#define R_I2SI_TX_PRE_FADE_OUT_0X244               (S3921_SND_BASE_ADDR + 0x244)
#define R_I2SI_TX_DEBUG1_0X250                     (S3921_SND_BASE_ADDR + 0x250)
#define R_I2SI_TX_DEBUG2_0X254                     (S3921_SND_BASE_ADDR + 0x254)
#define R_ADC_ANA_REG_0X260                        (S3921_SND_BASE_ADDR + 0x260)
#define R_ADC_DIG_REG_0X264                        (S3921_SND_BASE_ADDR + 0x264)
#define R_ADC_TEST_REG_0X268                       (S3921_SND_BASE_ADDR + 0x268)
/* AUD_PCM Interface Registers */
#define R_PCM_RX_DMA_BASE_ADDR_0X270               (S3921_SND_BASE_ADDR + 0x270)
#define R_PCM_RX_DMA_CFG_0X274                     (S3921_SND_BASE_ADDR + 0x274)
#define R_PCM_RX_DMA_INDEX_0X278                   (S3921_SND_BASE_ADDR + 0x278)
#define R_PCM_RX_DMA_OVER_FLOW_0X27C               (S3921_SND_BASE_ADDR + 0x27C)
#define R_PCM_RX_SAMPLE_COUNTER_0X280              (S3921_SND_BASE_ADDR + 0x280)
#define R_PCM_RX_INTF_CFG_0X284                    (S3921_SND_BASE_ADDR + 0x284)
#define R_PCM_RX_INTF_CFG2_0X288                   (S3921_SND_BASE_ADDR + 0x288)
#define R_PCM_RX_INTF_CTRL1_0X28C                  (S3921_SND_BASE_ADDR + 0x28C)
#define R_I2SI_2_CODEC_REG_0X2A0                   (S3921_SND_BASE_ADDR + 0x2A0)
#define R_I2SI_2_VOLUME_CTRL_REG_0X2A4             (S3921_SND_BASE_ADDR + 0x2A4)
#define R_PCM_RX_DEBUG1_0X2A8                      (S3921_SND_BASE_ADDR + 0x2A8)
#define R_PCM_RX_DEBUG2_0X2AC                      (S3921_SND_BASE_ADDR + 0x2AC)
#define R_PCM_TX_DMA_BASE_ADDR_0X2B0               (S3921_SND_BASE_ADDR + 0x2B0)
#define R_PCM_TX_DMA_CFG_0X2B4                     (S3921_SND_BASE_ADDR + 0x2B4)
#define R_PCM_TX_DMA_INDEX_0X2B8                   (S3921_SND_BASE_ADDR + 0x2B8)
#define R_PCM_TX_DMA_UNDER_RUN_0X2BC               (S3921_SND_BASE_ADDR + 0x2BC)
#define R_PCM_TX_SAMPLE_COUNTER_0X2C0              (S3921_SND_BASE_ADDR + 0x2C0)
#define R_PCM_TX_PRE_FADE_OUT_0X2C4                (S3921_SND_BASE_ADDR + 0x2C4)
#define R_PCM_TX_INTF_CTRL1_0X2C8                  (S3921_SND_BASE_ADDR + 0x2C8)
#define R_PCM_TX_INTF_CTRL2_0X2CC                  (S3921_SND_BASE_ADDR + 0x2CC)
#define R_I2SO_2_INTF_CTRL_0X2D0                   (S3921_SND_BASE_ADDR + 0x2D0)
#define R_I2SO_2_FS_CFG_0X2D4                      (S3921_SND_BASE_ADDR + 0x2D4)
#define R_PCM_TX_DEBUG1_0X2E0                      (S3921_SND_BASE_ADDR + 0x2E0)
#define R_PCM_TX_DEBUG2_0X2E4                      (S3921_SND_BASE_ADDR + 0x2E4)
/* AUD_PP Interface Registers */
#define R_PP_CTRL1_0X300                           (S3921_SND_BASE_ADDR + 0x300)
#define R_PP_CTRL2_0X304                           (S3921_SND_BASE_ADDR + 0x304)
#define R_VS_IIR1_COEFF1_0X310                     (S3921_SND_BASE_ADDR + 0x310)
#define R_VS_IIR1_COEEF2_0X314                     (S3921_SND_BASE_ADDR + 0x314)
#define R_VS_IIR1_COEEF3_0X318                     (S3921_SND_BASE_ADDR + 0x318)
#define R_VS_IIR2_COEFF1_0X320                     (S3921_SND_BASE_ADDR + 0x320)
#define R_VS_IIR2_COEEF2_0X324                     (S3921_SND_BASE_ADDR + 0x324)
#define R_VS_IIR2_COEEF3_0X328                     (S3921_SND_BASE_ADDR + 0x328)
#define R_VS_GAIN_COEEF1_0X330                     (S3921_SND_BASE_ADDR + 0x330)
#define R_VS_GAIN_COEEF2_0X334                     (S3921_SND_BASE_ADDR + 0x334)
#define R_EQ_IIR1_COEEF1_0X340                     (S3921_SND_BASE_ADDR + 0x340)
#define R_EQ_IIR1_COEEF2_0X344                     (S3921_SND_BASE_ADDR + 0x344)
#define R_EQ_IIR1_COEEF3_0X348                     (S3921_SND_BASE_ADDR + 0x348)
#define R_EQ_IIR2_COEEF1_0X350                     (S3921_SND_BASE_ADDR + 0x350)
#define R_EQ_IIR2_COEEF2_0X354                     (S3921_SND_BASE_ADDR + 0x354)
#define R_EQ_IIR2_COEEF3_0X358                     (S3921_SND_BASE_ADDR + 0x358)
#define R_EQ_IIR3_COEEF1_0X360                     (S3921_SND_BASE_ADDR + 0x360)
#define R_EQ_IIR3_COEEF2_0X364                     (S3921_SND_BASE_ADDR + 0x364)
#define R_EQ_IIR3_COEEF3_0X368                     (S3921_SND_BASE_ADDR + 0x368)
#define R_EQ_IIR4_COEEF1_0X370                     (S3921_SND_BASE_ADDR + 0x370)
#define R_EQ_IIR4_COEEF2_0X374                     (S3921_SND_BASE_ADDR + 0x374)
#define R_EQ_IIR4_COEEF3_0X378                     (S3921_SND_BASE_ADDR + 0x378)
#define R_BASS_IIR1_COEFF1_0X380                   (S3921_SND_BASE_ADDR + 0x380)
#define R_BASS_IIR1_COEFF2_0X384                   (S3921_SND_BASE_ADDR + 0x384)
#define R_BASS_IIR1_COEFF3_0X388                   (S3921_SND_BASE_ADDR + 0x388)
#define R_BASS_IIR2_COEFF1_0X390                   (S3921_SND_BASE_ADDR + 0x390)
#define R_BASS_IIR2_COEFF2_0X394                   (S3921_SND_BASE_ADDR + 0x394)
#define R_BASS_IIR2_COEFF3_0X398                   (S3921_SND_BASE_ADDR + 0x398)
#define R_BASS_IIR3_COEFF1_0X3A0                   (S3921_SND_BASE_ADDR + 0x3A0)
#define R_BASS_IIR3_COEFF2_0X3A4                   (S3921_SND_BASE_ADDR + 0x3A4)
#define R_BASS_IIR3_COEFF3_0X3A8                   (S3921_SND_BASE_ADDR + 0x3A8)
#define R_BASS_IIR4_COEFF1_0X3B0                   (S3921_SND_BASE_ADDR + 0x3B0)
#define R_BASS_IIR4_COEFF2_0X3B4                   (S3921_SND_BASE_ADDR + 0x3B4)
#define R_BASS_IIR4_COEFF3_0X3B8                   (S3921_SND_BASE_ADDR + 0x3B8)
#define R_BASS_GAIN_0X3C0                          (S3921_SND_BASE_ADDR + 0x3C0)
#define R_BASS_TRM_0X3C4                           (S3921_SND_BASE_ADDR + 0x3C4)
#define R_PP_DEBUG1_0X3D0                          (S3921_SND_BASE_ADDR + 0x3D0)
#define R_PP_DEBUG2_0X3D4                          (S3921_SND_BASE_ADDR + 0x3D4)


/* -----------------------------------------------------------------------------
System Hardware Register Control Functions
----------------------------------------------------------------------------- */

static void DisableAudClockGate(void)
{
    // reg offset: 0x60 ; bit: 5

    AUD_UINT8 RegValue = 0;
    
    RegValue = AUD_ReadRegDev8(R_SYS_DEVICE_CLK_GATING_CTRL0_0X60);
    if (RegValue & (0x1<<5))
    {
        AUD_WriteRegDev8(R_SYS_DEVICE_CLK_GATING_CTRL0_0X60, \
                         (RegValue&(~(0x1<<5))));
    }    
}



void ResetAudCore(void)
{
    // reg offset: 0x80 ; bit: 5
 
    AUD_UINT8 RegValue = 0;

    RegValue = AUD_ReadRegDev8(R_SYS_DEVICE_RESET_CTRL_0X80);

    AUD_WriteRegDev8(R_SYS_DEVICE_RESET_CTRL_0X80, \
                     ((RegValue&(~(0x1<<5))) | (0x1<<5)));

    AUD_Delay(3);

    AUD_WriteRegDev8(R_SYS_DEVICE_RESET_CTRL_0X80, \
                     (RegValue&(~(0x1<<5))));    
}

EXPORT_SYMBOL_GPL(ResetAudCore);

static void ConfigSystemCtrlReg(AUD_UINT32 Data)
{
    return;
}



static AUD_UINT32 GetSystemCtrlRegConfig(void)
{
    return 0;
}




/* -----------------------------------------------------------------------------
AUD0 Hardware Register Control Functions
----------------------------------------------------------------------------- */

/* Basic control ------------------------------------------------------------ */

static void StartAUD0(void)
{
    // reg offset: 0x100 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_CTRL_W_0X100);

    AUD_WriteRegDev32(R_AUD_CPU0_CTRL_W_0X100, \
                     ((RegValue&(~(0x01<<0))) | (0x01<<0)));
}



static void StopAUD0(void)
{
    // reg offset: 0x100 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_CTRL_W_0X100);

    AUD_WriteRegDev32(R_AUD_CPU0_CTRL_W_0X100, \
                     (RegValue&(~(0x01<<0))));
}



static void StartAUD0DecoderDMA(void)
{
    // reg offset: 0x110 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_CTRL_W_0X110);

    AUD_WriteRegDev32(R_AUD_CPU0_DEC_DMA_CTRL_W_0X110, \
                     ((RegValue&(~(0x01<<0))) | (0x01<<0)));
}



static void StopAUD0DecoderDMA(void)
{
    // reg offset: 0x110 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_CTRL_W_0X110);

    AUD_WriteRegDev32(R_AUD_CPU0_DEC_DMA_CTRL_W_0X110, \
                     (RegValue&(~(0x01<<0))));
}



static void StartAUD0EncoderDMA(void)
{
    // reg offset: 0x130 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_ENC_DMA_CTRL_W_0X130);

    AUD_WriteRegDev32(R_AUD_CPU0_ENC_DMA_CTRL_W_0X130, \
                     ((RegValue&(~(0x01<<0))) | (0x01<<0)));
}



static void StopAUD0EncoderDMA(void)
{
    // reg offset: 0x130 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_ENC_DMA_CTRL_W_0X130);

    AUD_WriteRegDev32(R_AUD_CPU0_ENC_DMA_CTRL_W_0X130, \
                     (RegValue&(~(0x01<<0))));
}



/* Audio CPU0 Register Operation Functions ---------------------------------- */

static void SetAUD0DecStreamType(enum AudioStreamType StreamType)
{
    // reg offset: 0x110 ; bit: 15-8

    AUD_UINT32 RegValue = 0;
    AUD_UINT32 Type = 0;
 
    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_CTRL_W_0X110);

    if (AUDIO_MPEG_AAC == StreamType)
    {
        Type = 6;
    }
    else if (AUDIO_MPEG_ADTS_AAC == StreamType)
    {
        Type = 5;
    }
    else if ((AUDIO_MPEG1 == StreamType) || (AUDIO_MPEG2 == StreamType)) 
    {
        Type = 4;
    }
    // else if (AUDIO_DOLBY_PULSE == StreamType)
    // {
    //     Type = 3;
    // }
    else if (AUDIO_EC3 == StreamType)
    {
        Type = 2;
    }
    else if (AUDIO_AC3 == StreamType)
    {
        Type = 1;
    }
    else
    {
        Type = 0;
    }
         
    AUD_WriteRegDev32(R_AUD_CPU0_DEC_DMA_CTRL_W_0X110,\
                     ((RegValue&(~(0xFF<<8))) | (Type<<8)));
}



static void SetAUD0DecDMAInAddr(AUD_UINT32 Addr)
{
    // reg offset: 0x114 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU0_DEC_DMA_IN_BASE_ADDR_0X114, \
                     (Addr&0xFFFFFFFF));
}



static void SetAUD0DecDMAInLength(AUD_UINT32 Length)
{
    // reg offset: 0x118 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU0_DEC_DMA_IN_LEN_0X118, \
                     (Length&0xFFFFFFFF));
}



static void SetAUD0DecDMAInWriteIdx(AUD_UINT32 WriteIndex)
{
    // reg offset: 0x11C ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU0_DEC_DMA_IN_WR_INDEX_0X11C, \
                     (WriteIndex&0xFFFFFFFF));
}



static void SetAUD0DecDMAOutAddr(AUD_UINT32 Addr)
{
    // reg offset: 0x120 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU0_DEC_DMA_OUT_BASE_ADDR_0X120, \
                     (Addr&0xFFFFFFFF));
}



static void SetAUD0DecDMAOutLength(AUD_UINT32 Length)
{
    // reg offset: 0x124 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU0_DEC_DMA_OUT_LEN_0X124, \
                     (Length&0xFFFFFFFF));
}



static void SetAUD0DecDMAOutReadIdx(AUD_UINT32 ReadIndex)
{
    // reg offset: 0x128 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU0_DEC_DMA_OUT_RD_INDEX_0X128, \
                     (ReadIndex&0xFFFFFFFF));
}



static void SetAUD0EncDMAInAddr(AUD_UINT32 Addr)
{
    // reg offset: 0x134 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU0_ENC_DMA_IN_BASE_ADDR_0X134, \
                      (Addr&0xFFFFFFFF));
}



static void SetAUD0EncDMAInLength(AUD_UINT32 Length)
{
    // reg offset: 0x138 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU0_ENC_DMA_IN_LEN_0X138, \
                     (Length&0xFFFFFFFF));
}



static void SetAUD0EncDMAInWriteIdx(AUD_UINT32 WriteIndex)
{
    // reg offset: 0x13C ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU0_ENC_DMA_IN_WR_INDEX_0X13C, \
                     (WriteIndex&0xFFFFFFFF));
}



static void SetAUD0EncDMAOutAddr(AUD_UINT32 Addr)
{
    // reg offset: 0x140 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU0_ENC_DMA_OUT_BASE_ADDR_0X140, \
                     (Addr&0xFFFFFFFF));
}



static void SetAUD0EncDMAOutLength(AUD_UINT32 Length)
{
    // reg offset: 0x144 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU0_ENC_DMA_OUT_LEN_0X144, \
                     (Length&0xFFFFFFFF));
}



static void SetAUD0EncDMAOutReadIdx(AUD_UINT32 ReadIndex)
{
    // reg offset: 0x148 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU0_ENC_DMA_OUT_RD_INDEX_0X148, \
                     (ReadIndex&0xFFFFFFFF));
}



static void SetAUD0DecPara(AUD_UINT32 *ParaSettingAddr)
{
    // reg offset: 0x150 - 0x164 ; bit: 31-0

    AUD_UINT32 Count = 0;

    for (Count=0; Count<5; Count++)
    {
        AUD_WriteRegDev32(R_AUD_CPU0_DEC_PARA0_0X150, \
                         (ParaSettingAddr[Count]&0xFFFFFFFF));
    }
}


static AUD_UINT8 GetAUD0CtrlStatus(void)
{
    // reg offset: 0x00 ; bit: 31-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_CTRL_R_0X00);

    return (AUD_UINT8)(RegValue&(0x01<<0));
}



static AUD_UINT8 GetAUD0DecDMACtrlStatus(void)
{
    // reg offset: 0x10 ; bit: 31-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_CTRL_R_0X10);

    return (AUD_UINT8)(RegValue&(0x01<<0));
}



static AUD_UINT32 GetAUD0DecDMAInReadIdx(void)
{
    // reg offset: 0x14 ; bit: 31-0

    return AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_IN_RD_INDEX_0X14);
}



static AUD_UINT32 GetAUD0DecDMAOutWriteIdx(void)
{
    // reg offset: 0x18 ; bit: 31-0

    return AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_OUT_WR_INDEX_0X18);
}



static AUD_UINT32 GetAUD0DecDMAInFrameLength(void)
{
    // reg offset: 0x1C ; bit: 31-0

    return AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_IN_FRAME_LEN_0X1C);
}



static AUD_UINT8 GetAUD0DecDMAInFrameStreamType(void)
{
    // reg offset: 0x20 ; bit: 23-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_IN_FRAME_INFO_0X20);

    return (AUD_UINT8)((RegValue&(0xFF<<16))>>16);
}



static AUD_UINT8 GetAUD0DecDMAInFrameChanNum(void)
{
    // reg offset: 0x20 ; bit: 15-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_IN_FRAME_INFO_0X20);

    return (AUD_UINT8)((RegValue&(0xFF<<8))>>8);
}



static AUD_UINT8 GetAUD0DecDMAInFrameSampRate(void)
{
    // reg offset: 0x20 ; bit: 7-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_IN_FRAME_INFO_0X20);

    return (AUD_UINT8)(RegValue&0xFF);
}



static AUD_UINT8 GetAUD0DecDMAOutFrameStatus(void)
{
    // reg offset: 0x24 ; bit: 31-24

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_OUT_FRAME_ST_0X24);

    return (AUD_UINT8)((RegValue&(0xFF<<24))>>24);
}



static AUD_UINT8 GetAUD0DecDMAOutFrameBitDepth(void)
{
    // reg offset: 0x24 ; bit: 23-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_OUT_FRAME_ST_0X24);

    return (AUD_UINT8)((RegValue&(0xFF<<16))>>16);
}



static AUD_UINT16 GetAUD0DecDMAOutFrameSampNum(void)
{
    // reg offset: 0x24 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_OUT_FRAME_ST_0X24);

    return (AUD_UINT16)(RegValue&0xFFFF);
}



static AUD_UINT16 GetAUD0DecDMAOutFrameSampRate(void)
{
    // reg offset: 0x28 ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_OUT_FRAME_FS_0X28);

    return (AUD_UINT16)((RegValue&(0xFFFF<<16))>>16);
}



static AUD_UINT16 GetAUD0DecDMAOutFrameChanNum(void)
{
    // reg offset: 0x28 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_DEC_DMA_OUT_FRAME_FS_0X28);

    return (AUD_UINT16)(RegValue&0xFFFF);
}



static AUD_UINT8 GetAUD0EncDMAStatus(void)
{
    // reg offset: 0x30 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_ENC_DMA_CTRL_R_0X30);

    return (AUD_UINT8)(RegValue&(0x01<<0));
}



static AUD_UINT32 GetAUD0EncDMAInReadIdx(void)
{
    // reg offset: 0x34 ; bit: 31-0

    return AUD_ReadRegDev32(R_AUD_CPU0_ENC_DMA_IN_RD_INDEX_0X34);
}



static AUD_UINT32 GetAUD0EncDMAOutWriteIdx(void)
{
    // reg offset: 0x38 ; bit: 31-0

    return AUD_ReadRegDev32(R_AUD_CPU0_ENC_DMA_OUT_WR_INDEX_0X38);
}



static AUD_UINT8 GetAUD0EncDMAInFrameDrcMode(void)
{
    // reg offset: 0x3C ; bit: 31-24

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_ENC_DMA_IN_FRAME_INFO_0X3C);

    return (AUD_UINT8)((RegValue&(0xFF<<24))>>24);
}



static AUD_UINT8 GetAUD0EncDMAInFrameBitRate(void)
{
    // reg offset: 0x3C ; bit: 23-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_ENC_DMA_IN_FRAME_INFO_0X3C);

    return (AUD_UINT8)((RegValue&(0xFF<<16))>>16);
}



static AUD_UINT8 GetAUD0EncDMAInFrameChanMode(void)
{
    // reg offset: 0x3C ; bit: 15-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_ENC_DMA_IN_FRAME_INFO_0X3C);

    return (AUD_UINT8)((RegValue&(0xFF<<8))>>8);
}



static AUD_UINT8 GetAUD0EncDMAInFrameBitDepth(void)
{
    // reg offset: 0x3C ; bit: 7-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_ENC_DMA_IN_FRAME_INFO_0X3C);

    return (AUD_UINT8)(RegValue&0xFF);
}



static AUD_UINT32 GetAUD0EncDMAInFrameSampRate(void)
{
    // reg offset: 0x40 ; bit: 31-0

    return AUD_ReadRegDev32(R_AUD_CPU0_ENC_DMA_IN_FRAME_FS_0X40);
}



static AUD_UINT16 GetAUD0EncDMAOutFrameStatus(void)
{
    // reg offset: 0x44 ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_ENC_DMA_OUT_FRAME_ST_0X44);

    return (AUD_UINT16)((RegValue&(0xFFFF<<16))>>16);
}



static AUD_UINT16 GetAUD0EncDMAOutFrameLen(void)
{
    // reg offset: 0x44 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU0_ENC_DMA_OUT_FRAME_ST_0X44);

    return (AUD_UINT16)(RegValue&0xFFFF);
}




/* -----------------------------------------------------------------------------
AUD1 Hardware Register Control Functions
----------------------------------------------------------------------------- */

/* Basic control ------------------------------------------------------------ */

static void StartAUD1(void)
{
    // reg offset: 0x180 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_CTRL_W_0X180);

    AUD_WriteRegDev32(R_AUD_CPU1_CTRL_W_0X180, \
                     ((RegValue&(~(0x01<<0))) | (0x01<<0)));
}



static void StopAUD1(void)
{
    // reg offset: 0x180 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_CTRL_W_0X180);

    AUD_WriteRegDev32(R_AUD_CPU1_CTRL_W_0X180, \
                     (RegValue&(~(0x01<<0))));
}



static void StartAUD1DecoderDMA(void)
{
    // reg offset: 0x190 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_CTRL_W_0X190);

    AUD_WriteRegDev32(R_AUD_CPU1_DEC_DMA_CTRL_W_0X190, \
                     ((RegValue&(~(0x01<<0))) | (0x01<<0)));
}



static void StopAUD1DecoderDMA(void)
{
    // reg offset: 0x190 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_CTRL_W_0X190);

    AUD_WriteRegDev32(R_AUD_CPU1_DEC_DMA_CTRL_W_0X190, \
                     (RegValue&(~(0x01<<0))));
}



static void StartAUD1EncoderDMA(void)
{
    // reg offset: 0x1B0 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_ENC_DMA_CTRL_W_0X1B0);

    AUD_WriteRegDev32(R_AUD_CPU1_ENC_DMA_CTRL_W_0X1B0, \
                     ((RegValue&(~(0x01<<0))) | (0x01<<0)));
}



static void StopAUD1EncoderDMA(void)
{
    // reg offset: 0x1B0 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_ENC_DMA_CTRL_W_0X1B0);

    AUD_WriteRegDev32(R_AUD_CPU1_ENC_DMA_CTRL_W_0X1B0, \
                     (RegValue&(~(0x01<<0))));
}



/* Audio CPU1 Register Operation Functions ---------------------------------- */

static void SetAUD1DecStreamType(enum AudioStreamType StreamType)
{
    // reg offset: 0x190 ; bit: 15-8

    AUD_UINT32 RegValue = 0;
    AUD_UINT32 Type = 0;
    
    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_CTRL_W_0X190);

    if (AUDIO_MPEG_AAC == StreamType)
    {
        Type = 6;
    }
    else if (AUDIO_MPEG_ADTS_AAC == StreamType)
    {
        Type = 5;
    }
    else if ((AUDIO_MPEG1 == StreamType) || (AUDIO_MPEG2 == StreamType)) 
    {
        Type = 4;
    }
    // else if (AUDIO_DOLBY_PULSE == StreamType)
    // {
    //     Type = 3;
    // }
    else if (AUDIO_EC3 == StreamType)
    {
        Type = 2;
    }
    else if (AUDIO_AC3 == StreamType)
    {
        Type = 1;
    }
    else
    {
        Type = 0;
    }
         
    AUD_WriteRegDev32(R_AUD_CPU0_DEC_DMA_CTRL_W_0X110,\
                     ((RegValue&(~(0xFF<<8))) | (Type<<8)));
}



static void SetAUD1DecDMAInAddr(AUD_UINT32 Addr)
{
    // reg offset: 0x194 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU1_DEC_DMA_IN_BASE_ADDR_0X194, \
                     (Addr&0xFFFFFFFF));
}



static void SetAUD1DecDMAInLength(AUD_UINT32 Length)
{
    // reg offset: 0x198 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU1_DEC_DMA_IN_LEN_0X198, \
                     (Length&0xFFFFFFFF));
}



static void SetAUD1DecDMAInWriteIdx(AUD_UINT32 WriteIndex)
{
    // reg offset: 0x19C ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU1_DEC_DMA_IN_WR_INDEX_0X19C, \
                     (WriteIndex&0xFFFFFFFF));
}



static void SetAUD1DecDMAOutAddr(AUD_UINT32 Addr)
{
    // reg offset: 0x1A0 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU1_DEC_DMA_OUT_BASE_ADDR_0X1A0, \
                     (Addr&0xFFFFFFFF));
}



static void SetAUD1DecDMAOutLength(AUD_UINT32 Length)
{
    // reg offset: 0x1A4 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU1_DEC_DMA_OUT_LEN_0X1A4, \
                     (Length&0xFFFFFFFF));
}



static void SetAUD1DecDMAOutReadIdx(AUD_UINT32 ReadIndex)
{
    // reg offset: 0x1A8 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU1_DEC_DMA_OUT_RD_INDEX_0X1A8, \
                     (ReadIndex&0xFFFFFFFF));
}



static void SetAUD1EncDMAInAddr(AUD_UINT32 Addr)
{
    // reg offset: 0x1B4 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU1_ENC_DMA_IN_BASE_ADDR_0X1B4, \
                     (Addr&0xFFFFFFFF));
}



static void SetAUD1EncDMAInLength(AUD_UINT32 Length)
{
    // reg offset: 0x1B8 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU1_ENC_DMA_IN_LEN_0X1B8, \
                     (Length&0xFFFFFFFF));
}



static void SetAUD1EncDMAInWriteIdx(AUD_UINT32 WriteIndex)
{
    // reg offset: 0x1BC ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU1_ENC_DMA_IN_WR_INDEX_0X1BC, \
                     (WriteIndex&0xFFFFFFFF));
}



static void SetAUD1EncDMAOutAddr(AUD_UINT32 Addr)
{
    // reg offset: 0x1C0 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU1_ENC_DMA_OUT_BASE_ADDR_0X1C0, \
                     (Addr&0xFFFFFFFF));
}



static void SetAUD1EncDMAOutLength(AUD_UINT32 Length)
{
    // reg offset: 0x1C4 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU1_ENC_DMA_OUT_LEN_0X1C4, \
                     (Length&0xFFFFFFFF));
}



static void SetAUD1EncDMAOutReadIdx(AUD_UINT32 ReadIndex)
{
    // reg offset: 0x1C8 ; bit: 31-0

    AUD_WriteRegDev32(R_AUD_CPU1_ENC_DMA_OUT_RD_INDEX_0X1C8, \
                     (ReadIndex&0xFFFFFFFF));
}



static void SetAUD1DecPara(AUD_UINT32 *ParaSettingAddr)
{
    // reg offset: 0x1D0 - 0x1E4 ; bit: 31-0

    AUD_UINT32 Count = 0;

    for (Count=0; Count<5; Count++)
    {
        AUD_WriteRegDev32(R_AUD_CPU1_DEC_PARA0_0X1D0, \
                         (ParaSettingAddr[Count]&0xFFFFFFFF));
    }
}


static AUD_UINT8 GetAUD1CtrlStatus(void)
{
    // reg offset: 0x80 ; bit: 31-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_CTRL_R_0X80);

    return (AUD_UINT8)(RegValue&(0x01<<0));
}



static AUD_UINT8 GetAUD1DecDMACtrlStatus(void)
{
    // reg offset: 0x90 ; bit: 31-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_CTRL_R_0X90);

    return (AUD_UINT8)(RegValue&(0x01<<0));
}



static AUD_UINT32 GetAUD1DecDMAInReadIdx(void)
{
    // reg offset: 0x94 ; bit: 31-0

    return AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_IN_RD_INDEX_0X94);
}



static AUD_UINT32 GetAUD1DecDMAOutWriteIdx(void)
{
    // reg offset: 0x98 ; bit: 31-0

    return AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_OUT_WR_INDEX_0X98);
}



static AUD_UINT32 GetAUD1DecDMAInFrameLength(void)
{
    // reg offset: 0x9C ; bit: 31-0

    return AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_IN_FRAME_LEN_0X9C);
}



static AUD_UINT8 GetAUD1DecDMAInFrameStreamType(void)
{
    // reg offset: 0xA0 ; bit: 23-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_IN_FRAME_INFO_0XA0);

    return (AUD_UINT8)((RegValue&(0xFF<<16))>>16);
}



static AUD_UINT8 GetAUD1DecDMAInFrameChanNum(void)
{
    // reg offset: 0xA0 ; bit: 15-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_IN_FRAME_INFO_0XA0);

    return (AUD_UINT8)((RegValue&(0xFF<<8))>>8);
}



static AUD_UINT8 GetAUD1DecDMAInFrameSampRate(void)
{
    // reg offset: 0xA0 ; bit: 7-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_IN_FRAME_INFO_0XA0);

    return (AUD_UINT8)(RegValue&0xFF);
}



static AUD_UINT8 GetAUD1DecDMAOutFrameStatus(void)
{
    // reg offset: 0xA4 ; bit: 31-24

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_OUT_FRAME_ST_0XA4);

    return (AUD_UINT8)((RegValue&(0xFF<<24))>>24);
}



static AUD_UINT8 GetAUD1DecDMAOutFrameBitDepth(void)
{
    // reg offset: 0xA4 ; bit: 23-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_OUT_FRAME_ST_0XA4);

    return (AUD_UINT8)((RegValue&(0xFF<<16))>>16);
}



static AUD_UINT16 GetAUD1DecDMAOutFrameSampNum(void)
{
    // reg offset: 0xA4 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_OUT_FRAME_ST_0XA4);

    return (AUD_UINT16)(RegValue&0xFFFF);
}



static AUD_UINT16 GetAUD1DecDMAOutFrameSampRate(void)
{
    // reg offset: 0xA8 ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_OUT_FRAME_FS_0XA8);

    return (AUD_UINT16)((RegValue&(0xFFFF<<16))>>16);
}



static AUD_UINT16 GetAUD1DecDMAOutFrameChanNum(void)
{
    // reg offset: 0xA8 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_DEC_DMA_OUT_FRAME_FS_0XA8);

    return (AUD_UINT16)(RegValue&0xFFFF);
}



static AUD_UINT8 GetAUD1EncDMAStatus(void)
{
    // reg offset: 0xB0 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_ENC_DMA_CTRL_R_0XB0);

    return (AUD_UINT8)(RegValue&(0x01<<0));
}



static AUD_UINT32 GetAUD1EncDMAInReadIdx(void)
{
    // reg offset: 0xB4 ; bit: 31-0

    return AUD_ReadRegDev32(R_AUD_CPU1_ENC_DMA_IN_RD_INDEX_0XB4);
}



static AUD_UINT32 GetAUD1EncDMAOutWriteIdx(void)
{
    // reg offset: 0xB8 ; bit: 31-0

    return AUD_ReadRegDev32(R_AUD_CPU1_ENC_DMA_OUT_WR_INDEX_0XB8);
}



static AUD_UINT8 GetAUD1EncDMAInFrameDrcMode(void)
{
    // reg offset: 0xBC ; bit: 31-24

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_ENC_DMA_IN_FRAME_INFO_0XBC);

    return (AUD_UINT8)((RegValue&(0xFF<<24))>>24);
}



static AUD_UINT8 GetAUD1EncDMAInFrameBitRate(void)
{
    // reg offset: 0xBC ; bit: 23-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_ENC_DMA_IN_FRAME_INFO_0XBC);

    return (AUD_UINT8)((RegValue&(0xFF<<16))>>16);
}



static AUD_UINT8 GetAUD1EncDMAInFrameChanMode(void)
{
    // reg offset: 0xBC ; bit: 15-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_ENC_DMA_IN_FRAME_INFO_0XBC);

    return (AUD_UINT8)((RegValue&(0xFF<<8))>>8);
}



static AUD_UINT8 GetAUD1EncDMAInFrameBitDepth(void)
{
    // reg offset: 0xBC ; bit: 7-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_ENC_DMA_IN_FRAME_INFO_0XBC);

    return (AUD_UINT8)(RegValue&0xFF);
}



static AUD_UINT32 GetAUD1EncDMAInFrameSampRate(void)
{
    // reg offset: 0xC0 ; bit: 31-0

    return AUD_ReadRegDev32(R_AUD_CPU1_ENC_DMA_IN_FRAME_FS_0XC0);
}



static AUD_UINT16 GetAUD1EncDMAOutFrameStatus(void)
{
    // reg offset: 0xC4 ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_ENC_DMA_OUT_FRAME_ST_0XC4);

    return (AUD_UINT16)((RegValue&(0xFFFF<<16))>>16);
}



static AUD_UINT16 GetAUD1EncDMAOutFrameLen(void)
{
    // reg offset: 0xC4 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_CPU1_ENC_DMA_OUT_FRAME_ST_0XC4);

    return (AUD_UINT16)(RegValue&0xFFFF);
}




/* -----------------------------------------------------------------------------
I2SO Hardware Register Control Functions
----------------------------------------------------------------------------- */

/* Basic setting ------------------------------------------------------------ */

static void SetI2SODMABufBaseAddr(AUD_UINT32 Addr)
{
    // reg offset: 0x30 ; bit: 31-0

    if ((Addr%32) != 0)
    {
        printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
    
    AUD_WriteRegDev32(R_I2SO_DMA_BASE_ADDR_0X30, \
                      (Addr&0x0FFFFFFF));
}



static void SetI2SODMABufLength(AUD_UINT16 Length)
{
    // reg offset: 0x34 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    if ((Length%2) != 0)
    {
        printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }

    RegValue = AUD_ReadRegDev32(R_I2SO_DMA_CFG_0X34);

    AUD_WriteRegDev32(R_I2SO_DMA_CFG_0X34, \
                      ((RegValue&(~(0xFFFF<<0))) | ((Length&0xFFFF)<<0)));
}



static void SetI2SODMABufLastIndex(AUD_UINT16 Index)
{
    // reg offset: 0x38 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_DMA_INDEX_0X38);

    AUD_WriteRegDev32(R_I2SO_DMA_INDEX_0X38, \
                      ((RegValue&(~(0xFFFF<<0))) | ((Index&0xFFFF)<<0)));
}



/* I2SO interface configure relative operation ------------------------------ */

static void SetI2SOTimeCheckThreshold(AUD_UINT16 Threshold)
{
    // reg offset: 0x4C ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_FRAME_COUNTER_0X4C);

    AUD_WriteRegDev32(R_I2SO_FRAME_COUNTER_0X4C, \
                      ((RegValue&(~(0xFFFF<<16))) | ((Threshold&0xFFFF)<<16)));
}



static void SetI2SOSampCountThreshold(AUD_UINT16 Threshold)
{
    // reg offset: 0x5C ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_SAMPLE_COUNTER_0X5C);

    AUD_WriteRegDev32(R_I2SO_SAMPLE_COUNTER_0X5C, \
                      ((RegValue&(~(0xFFFF<<0))) | ((Threshold&0xFFFF)<<0)));
}



void SetI2SODMADataWithHeader(AUD_BOOL Enable)
{
    // reg offset: 0x34 ; bit: 24

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_DMA_CFG_0X34);
    
    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_CFG_0X34, \
                          ((RegValue&(~(0x09<<24))) | (0x00<<24)));
        //with header mode should be normal mode
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_CFG_0X34, \
                          ((RegValue&(~(0x09<<24))) | (0x09<<24)));
        //without header mode should be ALSA mode

    }
    else
    {
        printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}


EXPORT_SYMBOL_GPL(SetI2SODMADataWithHeader);

static void SetI2SODMADataBitNum(I2SO_DRAM_BitNum_t Num)
{
    // reg offset: 0x34 ; bit: 20-21

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_DMA_CFG_0X34);

    if (BIT_NUM_8 == Num)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_CFG_0X34, \
                          ((RegValue&(~(0x03<<20))) | (0x00<<20)));
    }
    else if (BIT_NUM_16 == Num)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_CFG_0X34, \
                          ((RegValue&(~(0x03<<20))) | (0x01<<20)));
    }
    else if (BIT_NUM_24 == Num)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_CFG_0X34, \
                          ((RegValue&(~(0x03<<20))) | (0x02<<20)));
    }
    else if (BIT_NUM_32 == Num)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_CFG_0X34, \
                          ((RegValue&(~(0x03<<20))) | (0x03<<20)));
    }
    else
    {
        printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static I2SO_DRAM_BitNum_t GetI2SODMADataBitNum(void)
{
    // reg offset: 0x34 ; bit: 20   

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_DMA_CFG_0X34);
    RegValue &= (0x03 << 20);

    if (0 == RegValue)
    {
        return BIT_NUM_8;
    }
    else if (1 == RegValue)
    {
        return BIT_NUM_16;
    }
    else if (2 == RegValue)
    {
        return BIT_NUM_24;
    }
    else if (3 == RegValue)
    {
        return BIT_NUM_32;
    }
}



static void SetI2SODMADataLeftAlign(AUD_BOOL Enable)
{
    // reg offset: 0x34 ; bit: 25

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_DMA_CFG_0X34);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_CFG_0X34, \
                          ((RegValue&(~(0x01<<25))) | (0x01<<25)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_CFG_0X34, \
                          ((RegValue&(~(0x01<<25))) | (0x00<<25)));
    }
    else
    {
        printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SetI2SODMADataChannelNum(AUD_ChanNum_t Num)
{
    // reg offset: 0x34 ; bit: 17-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_DMA_CFG_0X34);

    if (CHAN_NUM_1 == Num)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_CFG_0X34, \
                          ((RegValue&(~(0x03<<16))) | (0x00<<16)));
    }
    else if (CHAN_NUM_2 == Num)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_CFG_0X34, \
                          ((RegValue&(~(0x03<<16))) | (0x01<<16)));
    }
    else if (CHAN_NUM_4 == Num)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_CFG_0X34, \
                          ((RegValue&(~(0x03<<16))) | (0x02<<16)));
    }
    else if (CHAN_NUM_8 == Num)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_CFG_0X34, \
                          ((RegValue&(~(0x03<<16))) | (0x03<<16)));
    }
    else
    {
        printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static AUD_ChanNum_t GetI2SODMADataChannelNum(void)
{
    // reg offset: 0x34 ; bit: 16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_DMA_CFG_0X34);
    RegValue &= (0x03 << 16);

    if (0 == RegValue)
    {
        return CHAN_NUM_1;
    }
    else if (1 == RegValue)
    {
        return CHAN_NUM_2;
    }
    else if (2 == RegValue)
    {
        return CHAN_NUM_4;
    }
    else if (3 == RegValue)
    {
        return CHAN_NUM_8;
    }
}



static void EnableI2SOInterfaceClock(AUD_BOOL Enable)
{
    // reg offset: 0x50 ; bit: 10-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_INTF_CTRL_0X50);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_INTF_CTRL_0X50, \
                          (RegValue&(~(0x07<<8))));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_INTF_CTRL_0X50, \
                          ((RegValue&(~(0x07<<8))) | (0x07<<8)));
    }
    else
    {
        printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SetI2SOInterfaceFormat(I2S_OUT_FORMAT_t Format)
{
    // reg offset: 0x50 ; bit: 1-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_INTF_CTRL_0X50);

    AUD_WriteRegDev32(R_I2SO_INTF_CTRL_0X50, \
                      ((RegValue&(~(0x03<<0))) | ((Format&0x03)<<0)));
}



// Only for C3701C
static void SelectI2SOMCLKMode(I2SO_MCLK_Mode_t Mode)
{
    /* Dummy function */
    return;
}



static void SetI2SOInterfaceBCLK2LRCLK(BIT_CLOCK_MODE_t Mode)
{
    // reg offset: 0x50 ; bit: 5-4

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_INTF_CTRL_0X50);

    AUD_WriteRegDev32(R_I2SO_INTF_CTRL_0X50, \
                      ((RegValue&(~(0x03<<4))) | ((Mode&0x03)<<4)));
}



static void EnableI2SOUnderRunFade(AUD_BOOL Enable)
{
    // reg offset: 0x3C ; bit: 6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_DMA_UNDER_RUN_0X3C);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_UNDER_RUN_0X3C, \
                          ((RegValue&(~(0x01<<6))) | (0x01<<6)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_UNDER_RUN_0X3C, \
                          ((RegValue&(~(0x01<<6))) | (0x00<<6)));
    }
    else
    {
        printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



// ????????
static void EnableI2SOAutoResume(AUD_BOOL Enable)
{
    /* Dummy function */
    return;
}



/* I2SO clock config start --> */
static void EnableI2SOFSSoftwareConfig(AUD_BOOL Enable)
{
    // reg offset: 0x54 ; bit: 31

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_FS_CFG_0X54);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_FS_CFG_0X54, \
                          ((RegValue&(~(0x01<<31))) | (0x01<<31)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_FS_CFG_0X54, \
                          ((RegValue&(~(0x01<<31))) | (0x00<<31)));
    }
    else
    {
        printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableI2SOInterfaceMCLKGated(AUD_BOOL Enable)
{
    // reg offset: 0x50 ; bit: 27

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_INTF_CTRL_0X50);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_INTF_CTRL_0X50, \
                          ((RegValue&(~(0x01<<27))) | (0x01<<27)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_INTF_CTRL_0X50, \
                          ((RegValue&(~(0x01<<27))) | (0x00<<27)));
    }
    else
    {
        printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableI2SOInterfaceBCLKGated(AUD_BOOL Enable)
{
    // reg offset: 0x50 ; bit: 6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_INTF_CTRL_0X50);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_INTF_CTRL_0X50, \
                          ((RegValue&(~(0x01<<6))) | (0x01<<6)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_INTF_CTRL_0X50, \
                          ((RegValue&(~(0x01<<6))) | (0x00<<6)));
    }
    else
    {
        printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SetI2SOMCLKDivBySoftware(I2SO_MCLK_DivMode_t MCLKDiv)
{
    // reg: 0x54 ; bit: 15-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_FS_CFG_0X54);

    switch (MCLKDiv)
    {
        case I2SO_MCLK_DIV_4:
        {
            AUD_WriteRegDev32(R_I2SO_FS_CFG_0X54, \
                              ((RegValue&(~(0xFF<<8))) | ((3&0xFF)<<8)));
            break;
        }

        case I2SO_MCLK_DIV_8:
        {
            AUD_WriteRegDev32(R_I2SO_FS_CFG_0X54, \
                              ((RegValue&(~(0xFF<<8))) | ((7&0xFF)<<8)));
            break;
        }

        case I2SO_MCLK_DIV_16:
        {
            AUD_WriteRegDev32(R_I2SO_FS_CFG_0X54, \
                              ((RegValue&(~(0xFF<<8))) | ((15&0xFF)<<8)));
            break;
        }

        case I2SO_MCLK_DIV_32:
        {
            AUD_WriteRegDev32(R_I2SO_FS_CFG_0X54, \
                              ((RegValue&(~(0xFF<<8))) | ((31&0xFF)<<8)));
            break;
        }

        case I2SO_MCLK_DIV_64:
        {
            AUD_WriteRegDev32(R_I2SO_FS_CFG_0X54, \
                              ((RegValue&(~(0xFF<<8))) | ((63&0xFF)<<8)));
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
    // reg: 0x54 ; bit: 29-24

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_FS_CFG_0X54);

    switch (BCLKDiv)
    {
        case I2SO_BCLK_DIV_1:
        {
            AUD_WriteRegDev32(R_I2SO_FS_CFG_0X54, \
                              ((RegValue&(~(0x3F<<24))) | ((1&0x3F)<<24)));
            break;
        }

        case I2SO_BCLK_DIV_2:
        {
            AUD_WriteRegDev32(R_I2SO_FS_CFG_0X54, \
                              ((RegValue&(~(0x3F<<24))) | ((2&0x3F)<<24)));
            break;
        }

        case I2SO_BCLK_DIV_4:
        {
            AUD_WriteRegDev32(R_I2SO_FS_CFG_0X54, \
                              ((RegValue&(~(0x3F<<24))) | ((4&0x3F)<<24)));
            break;
        }

        case I2SO_BCLK_DIV_8:
        {
            AUD_WriteRegDev32(R_I2SO_FS_CFG_0X54, \
                              ((RegValue&(~(0x3F<<24))) | ((8&0x3F)<<24)));
            break;
        }

        case I2SO_BCLK_DIV_16:
        {
            AUD_WriteRegDev32(R_I2SO_FS_CFG_0X54, \
                              ((RegValue&(~(0x3F<<24))) | ((16&0x3F)<<24)));
            break;
        }

        case I2SO_BCLK_DIV_32:
        {
            AUD_WriteRegDev32(R_I2SO_FS_CFG_0X54, \
                              ((RegValue&(~(0x3F<<24))) | ((32&0x3F)<<24)));
            break;
        }

        default:
        {
            break;
        }
    }
}



static void EnableI2SOF128FSSoftwareConfig(AUD_BOOL Enable)
{
    // reg offset: 0x58 ; bit: 16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_F128_FS_CFG_0X58);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_F128_FS_CFG_0X58, \
                          ((RegValue&(~(0x01<<16))) | (0x01<<16)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_F128_FS_CFG_0X58, \
                          ((RegValue&(~(0x01<<16))) | (0x00<<16)));
    }
    else
    {
        printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableI2SOF128FSGated(AUD_BOOL Enable)
{
    // reg offset: 0x58 ; bit: 21

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_F128_FS_CFG_0X58);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_F128_FS_CFG_0X58, \
                          ((RegValue&(~(0x01<<21))) | (0x01<<21)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_F128_FS_CFG_0X58, \
                          ((RegValue&(~(0x01<<21))) | (0x00<<21)));
    }
    else
    {
        printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SetI2SOF128FSSoftDiv(I2SO_F128_DivMode_t F128Div)
{
    // reg: 0x58 ; bit: 15-8
    

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_F128_FS_CFG_0X58);

    switch (F128Div)
    {
        case I2SO_F128_DIV_3:
        {
            AUD_WriteRegDev32(R_F128_FS_CFG_0X58, \
                              ((RegValue&(~(0xFF<<8))) | ((3&0xFF)<<8)));
            break;
        }

        case I2SO_F128_DIV_6:
        {
            AUD_WriteRegDev32(R_F128_FS_CFG_0X58, \
                              ((RegValue&(~(0xFF<<8))) | ((6&0xFF)<<8)));
            break;
        }

        case I2SO_F128_DIV_12:
        {
            AUD_WriteRegDev32(R_F128_FS_CFG_0X58, \
                              ((RegValue&(~(0xFF<<8))) | ((12&0xFF)<<8)));
            break;
        }

        case I2SO_F128_DIV_24:
        {
            AUD_WriteRegDev32(R_F128_FS_CFG_0X58, \
                              ((RegValue&(~(0xFF<<8))) | ((24&0xFF)<<8)));
            break;
        }

        case I2SO_F128_DIV_48:
        {
            AUD_WriteRegDev32(R_F128_FS_CFG_0X58, \
                              ((RegValue&(~(0xFF<<8))) | ((48&0xFF)<<8)));
            break;
        }

        case I2SO_F128_DIV_96:
        {
            AUD_WriteRegDev32(R_F128_FS_CFG_0X58, \
                              ((RegValue&(~(0xFF<<8))) | ((96&0xFF)<<8)));
            break;
        }

        case I2SO_F128_DIV_192:
        {
            AUD_WriteRegDev32(R_F128_FS_CFG_0X58, \
                              ((RegValue&(~(0xFF<<8))) | ((192&0xFF)<<8)));
            break;
        }

        default:
        {
            break;
        }
    }
}

/* --> I2SO clock config end */



/* FS hardware config */

static void EnableFSConfigByHardware(AUD_BOOL Enable)
{
    // reg offset: 0x10 ; bit: 31

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_SRC_CLK_SEL_0X10);

    if (TRUE == Enable) // Hardware auto config
    {
        AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                          ((RegValue&(~(0x01<<31))) | (0x01<<31)));
    }
    else if (FALSE == Enable) // Software config
    {
        AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                          ((RegValue&(~(0x01<<31))) | (0x00<<31)));
    }
    else
    {
        printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void ConfigI2SOFSByHardware(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x14 ; bit: 5-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_FS_CFG1_0X14);

    switch (SampleRate)
    {
        case SAMPLE_RATE_8000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x00<<0)));
            break;
        }
        
        case SAMPLE_RATE_16000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x01<<0)));
            break;
        }
        
        case SAMPLE_RATE_32000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x02<<0)));
            break;
        }
        
        case SAMPLE_RATE_64000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x03<<0)));
            break;
        }
        
        case SAMPLE_RATE_128000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x04<<0)));
            break;
        }
        
        case SAMPLE_RATE_11025:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x10<<0)));
            break;
        }
        
        case SAMPLE_RATE_22050:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x11<<0)));
            break;
        }
        
        case SAMPLE_RATE_44100:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x12<<0)));
            break;
        }
        
        case SAMPLE_RATE_88200:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x13<<0)));
            break;
        }
        
        case SAMPLE_RATE_176400:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x14<<0)));
            break;
        }
        
        case SAMPLE_RATE_12000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x20<<0)));
            break;
        }
        
        case SAMPLE_RATE_24000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x21<<0)));
            break;
        }
        
        case SAMPLE_RATE_48000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x22<<0)));
            break;
        }
        
        case SAMPLE_RATE_96000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x23<<0)));
            break;
        }
        
        case SAMPLE_RATE_192000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<0))) | (0x24<<0)));
            break;
        }

        default:
        {
            break;
        }
    }
}



static void ConfigI2SOFSBySoftware(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x10 ; bit: 17-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_SRC_CLK_SEL_0X10);

    switch (SampleRate)
    {
        case SAMPLE_RATE_8000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_64);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_16000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_32);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_32000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_16);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_64000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_8);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_128000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_4);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_11025:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_64);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_22050:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_32);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_44100:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_16);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_88200:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_8);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_176400:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_4);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_12000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_64);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_24000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_32);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_48000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_16);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_96000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_8);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_192000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetI2SOMCLKDivBySoftware(I2SO_MCLK_DIV_4);
            SetI2SOBCLKDivBySoftware(I2SO_BCLK_DIV_8);

            break;
        }
        
        default:
        {
            break;
        }
    }
}



void EnableI2SOInterface(AUD_BOOL Enable)
{
    // reg offset: 0x04 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<0))) | (0x01<<0)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<0))) | (0x00<<0)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}

EXPORT_SYMBOL_GPL(EnableI2SOInterface);

static AUD_BOOL CheckI2SOInfEnableStatus(void)
{
    // reg offset: 0x04 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

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
    // reg offset: 0x04 ; bit: 8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<8))) | (0x01<<8)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<8))) | (0x00<<8)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



/* I2SO DMA control --------------------------------------------------------- */

void StartI2SODMA(AUD_BOOL Enable)
{
    // reg offset: 0x04 ; bit: 16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);
    printk(KERN_ERR " %s, value %d \n ", __FUNCTION__,Enable);     

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<16))) | (0x01<<16)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<16))) | (0x00<<16)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}

EXPORT_SYMBOL_GPL(StartI2SODMA);
/* I2SO DMA control --------------------------------------------------------- */

AUD_BOOL GetI2SODMAStatus(void)
{
    // reg offset: 0x04 ; bit: 16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);
    if(RegValue&(0x01<<16))
      return TRUE;
    else
      return FALSE;

}

EXPORT_SYMBOL_GPL(GetI2SODMAStatus);

/* Get I2SO Interface information ------------------------------------------- */

static AUD_UINT16 GetI2SODMABufLastIndex(void)
{
    // reg offset: 0x38 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_DMA_INDEX_0X38);

    return (AUD_UINT16)(RegValue&0xFFFF);
}



static AUD_UINT16 GetI2SODMABufCurrentIndex(void)
{
    // reg offset: 0x38 ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_DMA_INDEX_0X38);

    return (AUD_UINT16)((RegValue&(0xFFFF<<16))>>16);
}



static AUD_UINT16 GetI2SOSampCount(void)
{
    // reg offset: 0x5C ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_SAMPLE_COUNTER_0X5C);

    return (AUD_UINT16)((RegValue&(0xFFFF<<16))>>16);
}



static AUD_UINT16 GetI2SOFrameCounter(void)
{
    // reg offset: 0x4C ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_FRAME_COUNTER_0X4C);

    return (AUD_UINT16)((RegValue&(0xFFFF<<0))>>0);
}


static AUD_UINT16 GetI2SOFrameLen(void)
{
    // reg offset: 0x40 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_FRAME_HEAD_INFO_0X40);

    return (AUD_UINT16)((RegValue&(0xFFFF<<0))>>0);
}



static AUD_UINT8 GetI2SOFrameHeaderInfo(void)
{
    // reg offset: 0x40 ; bit: 20-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_FRAME_HEAD_INFO_0X40);

    return (AUD_UINT8)((RegValue&(0x1F<<16))>>16);
}



static AUD_UINT32 GetI2SOCurrentFramePTS(void)
{
    // reg offset: 0x40 ; bit: 31-0

    return AUD_ReadRegDev32(R_I2SO_FRAME_HEAD_INFO_PTS_0X44);
}



static AUD_BOOL GetI2SOBufUnderRunStatus(void)
{
    // reg offset: 0x3C ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_DMA_UNDER_RUN_0X3C);

    if (RegValue&0x01)
    {
        AUD_WriteRegDev32(R_I2SO_DMA_UNDER_RUN_0X3C, RegValue);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL GetI2SOFIFOUnderRunStatus(void)
{
    // reg offset: 0x3C ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_DMA_UNDER_RUN_0X3C);

    if (RegValue&(0x01<<1))
    {
        // Make Sure do not clear I2SO Buf Under run status (0x3C[0])
        AUD_WriteRegDev32(R_I2SO_DMA_UNDER_RUN_0X3C, \
                          (RegValue&(~(0x01))));

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_UINT16 GetI2SODMASkipNum(void)
{
    // reg offset: 0x48 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_DMA_SKIP_NUM_0X48);

    return (AUD_UINT16)(RegValue&0xFFFF);
}



/* Enable I2SO interrupts --------------------------------------------------- */

static void EnableI2SOTimingCheckInt(AUD_BOOL Enable)
{
    // reg offset: 0x08 ; bit: 3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST1_0X08);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST1_0X08, \
                          ((RegValue&(~(0x01<<3))) | (0x01<<3)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST1_0X08, \
                          ((RegValue&(~(0x01<<3))) | (0x00<<3)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



void EnableI2SOSampCountInt(AUD_BOOL Enable)
{
    // reg offset: 0x08 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST1_0X08);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST1_0X08, \
                          ((RegValue&(~(0x01<<0))) | (0x01<<0)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST1_0X08, \
                          ((RegValue&(~(0x01<<0))) | (0x00<<0)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}

EXPORT_SYMBOL_GPL(EnableI2SOSampCountInt);


static void EnableI2SODMABufResumeInt(AUD_BOOL Enable)
{
    // reg offset: 0x0C ; bit: 16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          ((RegValue&(~(0x01<<16))) | (0x01<<16)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          ((RegValue&(~(0x01<<16))) | (0x00<<16)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



void EnableI2SODMAUnderRunInt(AUD_BOOL Enable)
{
    // reg offset: 0x0C ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          ((RegValue&(~(0x01<<0))) | (0x01<<0)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          ((RegValue&(~(0x01<<0))) | (0x00<<0)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}

EXPORT_SYMBOL_GPL(EnableI2SODMAUnderRunInt);

/* Get I2SO interrupt status ------------------------------------------------ */

static AUD_BOOL CheckI2SOTimingCheckInt(void)
{
    // reg offset: 0x08 ; bit: 3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST1_0X08);
    printk( " %s, %d!RegValue is %x\n",__FUNCTION__,__LINE__,RegValue);

    if (RegValue&(0x01<<3))
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
    // reg offset: 0x08 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST1_0X08);
  //  printk( " %s, %d!RegValue is %x\n",__FUNCTION__,__LINE__,RegValue);

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
    // reg offset: 0x08 ; bit: 19

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST1_0X08);

    if (RegValue&(0x01<<19))
    {
        AUD_WriteRegDev32(R_AUD_INT_ST1_0X08, \
                          (RegValue|(0x01<<19)));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



AUD_BOOL CheckI2SOSampCountIntStatus(void)
{
    // reg offset: 0x08 ; bit: 16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST1_0X08);
//    printk( " %s, %d!RegValue is %x\n",__FUNCTION__,__LINE__,RegValue);

    if (RegValue&(0x01<<16))
    {
        AUD_WriteRegDev32(R_AUD_INT_ST1_0X08, \
                          (RegValue|(0x01<<16)));
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
    // reg offset: 0x0C ; bit: 16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (RegValue&(0x01<<16))
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
    // reg offset: 0x0C ; bit: 24

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (RegValue&(0x01<<24))
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          (RegValue|(0x01<<24)));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL CheckI2SODMAUnderRunInt(void)
{
    // reg offset: 0x0C ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (RegValue&(0x01<<0))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



AUD_BOOL CheckI2SODMABufUnderRunIntStatus(void)
{
    // reg offset: 0x0C ; bit: 8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (RegValue&(0x01<<8))
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          (RegValue|(0x01<<8)));
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
    // reg offset: 0x100 ; bit: 31-0

    if ((Addr%32) != 0)
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
    
    AUD_WriteRegDev32(R_SPO_DMA_BASE_ADDR_0X100, \
                      (Addr&0x0FFFFFFF));
}



static void SetSPODMABufLength(AUD_UINT16 Length)
{
    // reg offset: 0x104 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    if ((Length%2) != 0)
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
    
    RegValue = AUD_ReadRegDev32(R_SPO_DMA_CFG_0X104);

    AUD_WriteRegDev32(R_SPO_DMA_CFG_0X104, \
                      ((RegValue&(~(0xFFFF<<0))) | ((Length&0xFFFF)<<0)));
}



static void SetSPODMABufLastIndex(AUD_UINT16 Index)
{
    // reg offset: 0x108 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_DMA_INDEX_0X108);

    AUD_WriteRegDev32(R_SPO_DMA_INDEX_0X108, \
                      ((RegValue&(~(0xFFFF<<0))) | ((Index&0xFFFF)<<0)));
}



static void SetSPOFramePCInfo(AUD_UINT16 PCInfo)
{
    // reg offset: 0x114 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_IEC_PC_0X114);

    AUD_WriteRegDev32(R_SPO_IEC_PC_0X114, \
                      ((RegValue&(~(0xFFFF<<0))) | ((PCInfo&0xFFFF)<<0)));
}



/* SPO interface confingure relative operation ------------------------------ */

static void SetSPOSampCountThreshold(AUD_UINT16 Threshold)
{
    // reg offset: 0x11C ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_SAMPLE_COUNTER_0X11C);

    AUD_WriteRegDev32(R_SPO_SAMPLE_COUNTER_0X11C, \
                      ((RegValue&(~(0xFFFF<<0))) | ((Threshold&0xFFFF)<<0)));
}



/* SPIDF Output DMA config start --> */

static void EnableSPODMADataWithHeader(AUD_BOOL Enable)
{
    // reg offset: 0x104 ; bit: 17

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_DMA_CFG_0X104);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_SPO_DMA_CFG_0X104, \
                          ((RegValue&(~(0x01<<17))) | (0x00<<17)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_SPO_DMA_CFG_0X104, \
                          ((RegValue&(~(0x01<<17))) | (0x01<<17)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SetSPODMADataLen(SPO_DMA_DataLen_t Len)
{
    // reg offset: 0x104 ; bit: 16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_DMA_CFG_0X104);

    if (DATA_LEN_16_BITS == Len)
    {
        AUD_WriteRegDev32(R_SPO_DMA_CFG_0X104, \
                          ((RegValue&(~(0x01<<16))) | (0x00<<16)));
    }
    else if (DATA_LEN_24_BITS == Len)
    {
        AUD_WriteRegDev32(R_SPO_DMA_CFG_0X104, \
                          ((RegValue&(~(0x01<<16))) | (0x01<<16)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}


/* <-- SPIDF Output DMA config end */



/* SPDIF output left channel status */
static void SetSPOLeftChanClkAccurary(AUD_UINT8 Accuray)
{
    // reg offset: 0x160 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_L_0X160, \
                      ((RegValue&(~(0x03<<28))) | ((Accuray&0x03)<<28)));
}



static AUD_UINT8 GetSPOLeftChanClkAccurary(void)
{
    // reg offset: 0x160 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    return (AUD_UINT8)((RegValue&(0x03<<28))>>28);
}



static void SetSPOLeftChanSampRate(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x160 ; bit: 31-30, 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT32 FS0 = 0;
    AUD_UINT32 FS1 = 0;

    switch (SampleRate)
    {
        case SAMPLE_RATE_22050:
        {
            FS0 = 0x04;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_44100:
        {
            FS0 = 0x00;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_88200:
        {
            FS0 = 0x08;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_176400:
        {
            FS0 = 0x0C;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_24000:
        {
            FS0 = 0x06;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_48000:
        {
            FS0 = 0x02;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_96000:
        {
            FS0 = 0x0A;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_192000:
        {
            FS0 = 0x0E;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_32000:
        {
            FS0 = 0x03;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_768000:
        {
            FS0 = 0x09;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_64000:
        {
            FS0 = 0x0B;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_128000:
        {
            FS0 = 0x0B;
            FS1 = 0x02;
            break;
        }

        case SAMPLE_RATE_256000:
        {
            FS0 = 0x0B;
            FS1 = 0x01;
            break;
        }

        case SAMPLE_RATE_512000:
        {
            FS0 = 0x0B;
            FS1 = 0x03;
            break;
        }

        case SAMPLE_RATE_1024000:
        {
            FS0 = 0x05;
            FS1 = 0x03;
            break;
        }

        case SAMPLE_RATE_384000:
        {
            FS0 = 0x05;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_1536000:
        {
            FS0 = 0x05;
            FS1 = 0x01;
            break;
        }

        case SAMPLE_RATE_352800:
        {
            FS0 = 0x0D;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_705600:
        {
            FS0 = 0x0D;
            FS1 = 0x02;
            break;
        }

        case SAMPLE_RATE_1411200:
        {
            FS0 = 0x0D;
            FS1 = 0x01;
            break;
        }

        default:
        {
            FS0 = 0x00;
            FS1 = 0x00;
            break;
        }
    }

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_L_0X160, \
                      ((RegValue&(~(0x0F<<24))) | ((FS0&0x0F)<<24) | ((FS1&0x03)<<30)));
}



static AUD_SampleRate_t GetSPOLeftChanSampRate(void)
{
    // reg offset: 0x160 ; bit: 31-30, 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT8 FS0 = 0;
    AUD_UINT8 FS1 = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    FS0 = (AUD_UINT8)((RegValue&(0x0F<<24))>>24);
    FS1 = (AUD_UINT8)((RegValue&(0x03<<30))>>30);

    if ((0x04 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_22050;
    }
    else if ((0x00 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_44100;
    }
    else if ((0x08 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_88200;
    }
    else if ((0x0C == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_176400;
    }
    else if ((0x06 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_24000;
    }
    else if ((0x02 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_48000;
    }
    else if ((0x0A == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_96000;
    }
    else if ((0x0E == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_192000;
    }
    else if ((0x03 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_32000;
    }
    else if ((0x09 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_768000;
    }
    else if ((0x0B == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_64000;
    }
    else if ((0x0B == FS0) && (0x02 == FS1))
    {
        return SAMPLE_RATE_128000;
    }
    else if ((0x0B == FS0) && (0x01 == FS1))
    {
        return SAMPLE_RATE_256000;
    }
    else if ((0x0B == FS0) && (0x03 == FS1))
    {
        return SAMPLE_RATE_512000;
    }
    else if ((0x05 == FS0) && (0x03 == FS1))
    {
        return SAMPLE_RATE_1024000;
    }
    else if ((0x05 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_384000;
    }
    else if ((0x05 == FS0) && (0x01 == FS1))
    {
        return SAMPLE_RATE_1536000;
    }
    else if ((0x0D == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_352800;
    }
    else if ((0x0D == FS0) && (0x02 == FS1))
    {
        return SAMPLE_RATE_705600;
    }
    else if ((0x0D == FS0) && (0x01 == FS1))
    {
        return SAMPLE_RATE_1411200;
    }
    else if ((0x08 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_88200;
    }
}



static void SetSPOLeftChanNum(AUD_UINT8 ChannelNum)
{
    // reg offset: 0x160 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_L_0X160, \
                      ((RegValue&(~(0x0F<<20))) | ((ChannelNum&0x0F)<<20)));
}



static AUD_UINT8 GetSPOLeftChanNum(void)
{
    // reg offset: 0x160 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    return (AUD_UINT8)((RegValue&(0x0F<<20))>>20);
}



static void SetSPOLeftChanSourcelNum(AUD_UINT8 SourcelNum)
{
    // reg offset: 0x160 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_L_0X160, \
                      ((RegValue&(~(0x0F<<16))) | ((SourcelNum&0x0F)<<16)));
}



static AUD_UINT8 GetSPOLeftChanSourcelNum(void)
{
    // reg offset: 0x160 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    return (AUD_UINT8)((RegValue&(0x0F<<16))>>16);
}



static void SetSPOLeftChanLBit(AUD_UINT8 LBit)
{
    // reg offset: 0x160 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_L_0X160, \
                      ((RegValue&(~(0x01<<15))) | ((LBit&0x01)<<15)));
}



static AUD_UINT8 GetSPOLeftChanLBit(void)
{
    // reg offset: 0x160 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    return (AUD_UINT8)((RegValue&(0x01<<15))>>15);
}



static void SetSPOLeftChanCategory(AUD_UINT8 Category)
{
    // reg offset: 0x160 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_L_0X160, \
                      ((RegValue&(~(0x7F<<8))) | ((Category&0x7F)<<8)));
}



static AUD_UINT16 GetSPOLeftChanCategory(void)
{
    // reg offset: 0x160 ; bit: 15-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    return (AUD_UINT16)((RegValue&(0xFF<<8))>>8);
}



static void SetSPOLeftChanStatusMode(AUD_UINT8 Mode)
{
    // reg offset: 0x160 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_L_0X160, \
                      ((RegValue&(~(0x03<<6))) | ((Mode&0x03)<<6)));
}



static AUD_UINT8 GetSPOLeftChanStatusMode(void)
{
    // reg offset: 0x160 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    return (AUD_UINT8)((RegValue&(0x03<<6))>>6);
}



static void SetSPOLeftChanAddFormatInfo(AUD_UINT8 AddFormatInfo)
{
    // reg offset: 0x160 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_L_0X160, \
                      ((RegValue&(~(0x07<<3))) | ((AddFormatInfo&0x07)<<3)));
}



static AUD_UINT8 GetSPOLeftChanAddFormatInfo(void)
{
    // reg offset: 0x160 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    return (AUD_UINT8)((RegValue&(0x07<<3))>>3);
}



static void SetSPOLeftChanCopyright(AUD_UINT8 Copyright)
{
    // reg offset: 0x160 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_L_0X160, \
                      ((RegValue&(~(0x01<<2))) | ((Copyright&0x01)<<2)));
}



static AUD_UINT8 GetSPOLeftChanCopyright(void)
{
    // reg offset: 0x160 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    return (AUD_UINT8)((RegValue&(0x01<<2))>>2);
}



static void SetSPOLeftChanAudContent(SPO_AudioContentFlag_t AudContent)
{
    // reg offset: 0x160 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_L_0X160, \
                      ((RegValue&(~(0x01<<1))) | ((AudContent&0x01)<<1)));
}



static AUD_UINT8 GetSPOLeftChanAudContent(void)
{
    // reg offset: 0x160 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    return (AUD_UINT8)((RegValue&(0x01<<1))>>1);
}



static void SetSPOLeftChanUseFlag(AUD_UINT8 UseFlag)
{
    // reg offset: 0x160 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_L_0X160, \
                      ((RegValue&(~0x01)) | (UseFlag&0x01)));
}



static AUD_UINT8 GetSPOLeftChanUseFlag(void)
{
    // reg offset: 0x160 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_L_0X160);

    return (AUD_UINT8)(RegValue&0x01);
}



static void ConfigSPOLeftChanUserData(AUD_UINT32 *DataAddr)
{
    // reg offset: 0x130 ; bit: 192-0

    AUD_UINT8 i = 0;

    for (i=0; i<6; i++)
    {
        AUD_WriteRegDev32((R_SPO_LEFT_USER_DATA1_0X130 + i*4), \
                          DataAddr[i]);
    }
}


/* SPDIF output right channel status */
static void SetSPORightChanClkAccurary(AUD_UINT8 Accuray)
{
    // reg offset: 0x164 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_R_0X164, \
                      ((RegValue&(~(0x03<<28))) | ((Accuray&0x03)<<28)));
}



static AUD_UINT8 GetSPORightChanClkAccurary(void)
{
    // reg offset: 0x164 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    return (AUD_UINT8)((RegValue&(0x03<<28))>>28);
}



static void SetSPORightChanSampRate(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x164 ; bit: 31-30, 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT32 FS0 = 0;
    AUD_UINT32 FS1 = 0;

    switch (SampleRate)
    {
        case SAMPLE_RATE_22050:
        {
            FS0 = 0x04;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_44100:
        {
            FS0 = 0x00;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_88200:
        {
            FS0 = 0x08;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_176400:
        {
            FS0 = 0x0C;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_24000:
        {
            FS0 = 0x06;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_48000:
        {
            FS0 = 0x02;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_96000:
        {
            FS0 = 0x0A;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_192000:
        {
            FS0 = 0x0E;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_32000:
        {
            FS0 = 0x03;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_768000:
        {
            FS0 = 0x09;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_64000:
        {
            FS0 = 0x0B;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_128000:
        {
            FS0 = 0x0B;
            FS1 = 0x02;
            break;
        }

        case SAMPLE_RATE_256000:
        {
            FS0 = 0x0B;
            FS1 = 0x01;
            break;
        }

        case SAMPLE_RATE_512000:
        {
            FS0 = 0x0B;
            FS1 = 0x03;
            break;
        }

        case SAMPLE_RATE_1024000:
        {
            FS0 = 0x05;
            FS1 = 0x03;
            break;
        }

        case SAMPLE_RATE_384000:
        {
            FS0 = 0x05;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_1536000:
        {
            FS0 = 0x05;
            FS1 = 0x01;
            break;
        }

        case SAMPLE_RATE_352800:
        {
            FS0 = 0x0D;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_705600:
        {
            FS0 = 0x0D;
            FS1 = 0x02;
            break;
        }

        case SAMPLE_RATE_1411200:
        {
            FS0 = 0x0D;
            FS1 = 0x01;
            break;
        }

        default:
        {
            FS0 = 0x00;
            FS1 = 0x00;            
        }
    }

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_R_0X164, \
                      ((RegValue&(~(0x0F<<24))) | ((FS0&0x0F)<<24) | ((FS1&0x03)<<30)));
}



static AUD_SampleRate_t GetSPORightChanSampRate(void)
{
    // reg offset: 0x160 ; bit: 31-30, 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT8 FS0 = 0;
    AUD_UINT8 FS1 = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    FS0 = (AUD_UINT8)((RegValue&(0x0F<<24))>>24);
    FS1 = (AUD_UINT8)((RegValue&(0x03<<30))>>30);

    if ((0x04 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_22050;
    }
    else if ((0x00 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_44100;
    }
    else if ((0x08 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_88200;
    }
    else if ((0x0C == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_176400;
    }
    else if ((0x06 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_24000;
    }
    else if ((0x02 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_48000;
    }
    else if ((0x0A == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_96000;
    }
    else if ((0x0E == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_192000;
    }
    else if ((0x03 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_32000;
    }
    else if ((0x09 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_768000;
    }
    else if ((0x0B == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_64000;
    }
    else if ((0x0B == FS0) && (0x02 == FS1))
    {
        return SAMPLE_RATE_128000;
    }
    else if ((0x0B == FS0) && (0x01 == FS1))
    {
        return SAMPLE_RATE_256000;
    }
    else if ((0x0B == FS0) && (0x03 == FS1))
    {
        return SAMPLE_RATE_512000;
    }
    else if ((0x05 == FS0) && (0x03 == FS1))
    {
        return SAMPLE_RATE_1024000;
    }
    else if ((0x05 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_384000;
    }
    else if ((0x05 == FS0) && (0x01 == FS1))
    {
        return SAMPLE_RATE_1536000;
    }
    else if ((0x0D == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_352800;
    }
    else if ((0x0D == FS0) && (0x02 == FS1))
    {
        return SAMPLE_RATE_705600;
    }
    else if ((0x0D == FS0) && (0x01 == FS1))
    {
        return SAMPLE_RATE_1411200;
    }
    else if ((0x08 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_88200;
    }
}



static void SetSPORightChanNum(AUD_UINT8 ChannelNum)
{
    // reg offset: 0x164 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_R_0X164, \
                      ((RegValue&(~(0x0F<<20))) | ((ChannelNum&0x0F)<<20)));
}



static AUD_UINT8 GetSPORightChanNum(void)
{
    // reg offset: 0x164 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    return (AUD_UINT8)((RegValue&(0x0F<<20))>>20);
}



static void SetSPORightChanSourcelNum(AUD_UINT8 SourcelNum)
{
    // reg offset: 0x164 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_R_0X164, \
                      ((RegValue&(~(0x0F<<16))) | ((SourcelNum&0x0F)<<16)));
}



static AUD_UINT8 GetSPORightChanSourcelNum(void)
{
    // reg offset: 0x164 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    return (AUD_UINT8)((RegValue&(0x0F<<16))>>16);
}



static void SetSPORightChanLBit(AUD_UINT8 LBit)
{
    // reg offset: 0x164 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_R_0X164, \
                      ((RegValue&(~(0x01<<15))) | ((LBit&0x01)<<15)));
}



static AUD_UINT8 GetSPORightChanLBit(void)
{
    // reg offset: 0x164 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    return (AUD_UINT8)((RegValue&(0x01<<15))>>15);
}



static void SetSPORightChanCategory(AUD_UINT8 Category)
{
    // reg offset: 0x164 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_R_0X164, \
                      ((RegValue&(~(0x7F<<8))) | ((Category&0x7F)<<8)));
}



static AUD_UINT16 GetSPORightChanCategory(void)
{
    // reg offset: 0x164 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    return (AUD_UINT16)((RegValue&(0x7F<<8))>>8);
}



static void SetSPORightChanStatusMode(AUD_UINT8 Mode)
{
    // reg offset: 0x164 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_R_0X164, \
                      ((RegValue&(~(0x03<<6))) | ((Mode&0x03)<<6)));
}



static AUD_UINT8 GetSPORightChanStatusMode(void)
{
    // reg offset: 0x164 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    return (AUD_UINT8)((RegValue&(0x03<<6))>>6);
}



static void SetSPORightChanAddFormatInfo(AUD_UINT8 AddFormatInfo)
{
    // reg offset: 0x164 ; bit: 5-3
    

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_R_0X164, \
                      ((RegValue&(~(0x07<<3))) | ((AddFormatInfo&0x07)<<3)));
}



static AUD_UINT8 GetSPORightChanAddFormatInfo(void)
{
    // reg offset: 0x164 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    return (AUD_UINT8)((RegValue&(0x07<<3))>>3);
}



static void SetSPORightChanCopyright(AUD_UINT8 Copyright)
{
    // reg offset: 0x164 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_R_0X164, \
                      ((RegValue&(~(0x01<<2))) | ((Copyright&0x01)<<2)));
}



static AUD_UINT8 GetSPORightChanCopyright(void)
{
    // reg offset: 0x164 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    return (AUD_UINT8)((RegValue&(0x01<<2))>>2);
}



static void SetSPORightChanAudContent(SPO_AudioContentFlag_t AudContent)
{
    // reg offset: 0x164 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_R_0X164, \
                      ((RegValue&(~(0x01<<1))) | ((AudContent&0x01)<<1)));
}



static AUD_UINT8 GetSPORightChanAudContent(void)
{
    // reg offset: 0x164 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    return (AUD_UINT8)((RegValue&(0x01<<1))>>1);
}



static void SetSPORightChanUseFlag(AUD_UINT8 UseFlag)
{
    // reg offset: 0x164 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    AUD_WriteRegDev32(R_SPO_CHANNEL_STATUS_R_0X164, \
                      ((RegValue&(~0x01)) | (UseFlag&0x01)));
}



static AUD_UINT8 GetSPORightChanUseFlag(void)
{
    // reg offset: 0x164 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_CHANNEL_STATUS_R_0X164);

    return (AUD_UINT8)(RegValue&0x01);
}



static void ConfigSPORightChanUserData(AUD_UINT32 *DataAddr)
{
    // reg offset: 0x148 ; bit: 192-0

    AUD_UINT8 i = 0;

    for (i=0; i<6; i++)
    {
        AUD_WriteRegDev32((R_SPO_RIGHT_USER_DATA1_0X148 + i*4), \
                          DataAddr[i]);
    }
}



/* SPDIF Output clock config start --> */

static void SelectSPOMCLKMode(SPO_MCLK_DivMode_t Mode)
{
    /* Dummy function */
    return;
}



static void EnableSPOFSSoftwareConfig(AUD_BOOL Enable)
{
    // reg offset: 0x118 ; bit: 16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_FS_CFG_0X118);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                          ((RegValue&(~(0x01<<16))) | (0x01<<16)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                          ((RegValue&(~(0x01<<16))) | (0x00<<16)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}




static void EnableSPOFSGated(AUD_BOOL Enable)
{
    // reg offset: 0x118 ; bit: 22

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_FS_CFG_0X118);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                          ((RegValue&(~(0x01<<22))) | (0x01<<22)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                          ((RegValue&(~(0x01<<22))) | (0x00<<22)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SetSPOMCLKDivBySoftware(SPO_CLK_SW_DivMode_t FSDiv)
{
    // reg: 0x118 ; bit: 15-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_FS_CFG_0X118);

    switch (FSDiv)
    {
        case SPO_CLK_SW_DIV_3:
        {
            AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                              ((RegValue&(~(0x0F<<8))) | ((3&0x0F)<<8)));
            break;
        }

        case SPO_CLK_SW_DIV_6:
        {
            AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                              ((RegValue&(~(0x0F<<8))) | ((6&0x0F)<<8)));
            break;
        }

        case SPO_CLK_SW_DIV_12:
        {
            AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                              ((RegValue&(~(0x0F<<8))) | ((12&0x0F)<<8)));
            break;
        }

        case SPO_CLK_SW_DIV_24:
        {
            AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                              ((RegValue&(~(0x0F<<8))) | ((24&0x0F)<<8)));
            break;
        }

        case SPO_CLK_SW_DIV_48:
        {
            AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                              ((RegValue&(~(0x0F<<8))) | ((48&0x0F)<<8)));
            break;
        }

        case SPO_CLK_SW_DIV_96:
        {
            AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                              ((RegValue&(~(0x0F<<8))) | ((96&0x0F)<<8)));
            break;
        }

        case SPO_CLK_SW_DIV_192:
        {
            AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                              ((RegValue&(~(0x0F<<8))) | ((192&0x0F)<<8)));
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
    // reg offset: 0x110 ; bit: 16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_INTF_CTRL_0X110);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_SPO_INTF_CTRL_0X110, \
                          ((RegValue&(~(0x01<<16))) | (0x01<<16)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                          ((RegValue&(~(0x01<<16))) | (0x00<<16)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableSPODataReorder(AUD_BOOL Enable)
{
    // reg offset: 0x110 ; bit: 6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_INTF_CTRL_0X110);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_SPO_INTF_CTRL_0X110, \
                          ((RegValue&(~(0x01<<6))) | (0x01<<6)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                          ((RegValue&(~(0x01<<6))) | (0x00<<6)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableSPOValidBit(AUD_BOOL Enable)
{
    // reg offset: 0x110 ; bit: 4

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_INTF_CTRL_0X110);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_SPO_INTF_CTRL_0X110, \
                          ((RegValue&(~(0x01<<4))) | (0x01<<4)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                          ((RegValue&(~(0x01<<4))) | (0x00<<4)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableSPONormalPlay(AUD_BOOL Enable)
{
    // reg offset: 0x110 ; bit: 17

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_INTF_CTRL_0X110);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_SPO_INTF_CTRL_0X110, \
                          ((RegValue&(~(0x01<<17))) | (0x00<<17)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                          ((RegValue&(~(0x01<<17))) | (0x01<<17)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectSPOChannelStatusReg(SPO_ChanStatusReg_t Mode)
{
    // reg offset: 0x110 ; bit: 5

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_INTF_CTRL_0X110);

    if (1 == Mode) // CSD_R use CSD_R register value
    {
        AUD_WriteRegDev32(R_SPO_INTF_CTRL_0X110, \
                          ((RegValue&(~(0x01<<5))) | (0x01<<5)));
    }
    else if (0 == Mode) // CSD_R use CSD_L register value
    {
        AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                          ((RegValue&(~(0x01<<5))) | (0x00<<5)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectSPODataSource(SPO_DataSource_t Source)
{
    // reg offset: 0x110 ; bit: 2
    
    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_INTF_CTRL_0X110);

    if (FROM_SPO_DMA == Source) 
    {
        AUD_WriteRegDev32(R_SPO_INTF_CTRL_0X110, \
                          ((RegValue&(~(0x01<<2))) | (0x01<<2)));
    }
    else if (FROM_I2SO_DMA == Source) // From I2SO DMA
    {
        AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                          ((RegValue&(~(0x01<<2))) | (0x00<<2)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}




static void SelectSPOPCMOutChan(SPO_PCM_OutChanMode_t Chan)
{
    // reg offset: 0x110 ; bit: 1-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_INTF_CTRL_0X110);

    AUD_WriteRegDev32(R_SPO_INTF_CTRL_0X110, \
                      ((RegValue&(~(0x03<<0))) | ((Chan&0x03)<<0)));
}



static void SelectSPOPCMOutLFEMode(SPO_PCM_OutLFEMode_t Mode)
{
    // reg offset: 0x110 ; bit: 3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_INTF_CTRL_0X110);

    if (SPO_L_I2SO_LFE == Mode) 
    {
        AUD_WriteRegDev32(R_SPO_INTF_CTRL_0X110, \
                          ((RegValue&(~(0x01<<3))) | (0x01<<3)));
    }
    else if (SPO_R_I2SO_LFE == Mode)
    {
        AUD_WriteRegDev32(R_SPO_FS_CFG_0X118, \
                          ((RegValue&(~(0x01<<3))) | (0x00<<3)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void ConfigSPOFSBySoftware(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x10 ; bit: 17-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_SRC_CLK_SEL_0X10);

    switch (SampleRate)
    {
        case SAMPLE_RATE_8000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_192);

            break;
        }
        
        case SAMPLE_RATE_16000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_96);

            break;
        }
        
        case SAMPLE_RATE_32000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_32000);
            SetSPORightChanSampRate(SAMPLE_RATE_32000);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_48);

            break;
        }
        
        case SAMPLE_RATE_64000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_64000);
            SetSPORightChanSampRate(SAMPLE_RATE_64000);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_24);

            break;
        }
        
        case SAMPLE_RATE_128000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_128000);
            SetSPORightChanSampRate(SAMPLE_RATE_128000);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_12);

            break;
        }
        
        case SAMPLE_RATE_256000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_256000);
            SetSPORightChanSampRate(SAMPLE_RATE_256000);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_6);

            break;
        }
        
        case SAMPLE_RATE_512000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_512000);
            SetSPORightChanSampRate(SAMPLE_RATE_512000);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_3);

            break;
        }
        
        case SAMPLE_RATE_11025:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_192);

            break;
        }
        
        case SAMPLE_RATE_22050:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_22050);
            SetSPORightChanSampRate(SAMPLE_RATE_22050);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_96);

            break;
        }
        
        case SAMPLE_RATE_44100:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_44100);
            SetSPORightChanSampRate(SAMPLE_RATE_44100);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_48);

            break;
        }
        
        case SAMPLE_RATE_88200:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_88200);
            SetSPORightChanSampRate(SAMPLE_RATE_88200);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_24);

            break;
        }
        
        case SAMPLE_RATE_176400:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_176400);
            SetSPORightChanSampRate(SAMPLE_RATE_176400);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_12);

            break;
        }
        
        case SAMPLE_RATE_352800:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_352800);
            SetSPORightChanSampRate(SAMPLE_RATE_352800);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_6);

            break;
        }
        
        case SAMPLE_RATE_705600:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_705600);
            SetSPORightChanSampRate(SAMPLE_RATE_705600);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_3);

            break;
        }
        
        case SAMPLE_RATE_12000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_192);

            break;
        }
        
        case SAMPLE_RATE_24000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_24000);
            SetSPORightChanSampRate(SAMPLE_RATE_24000);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_96);

            break;
        }
        
        case SAMPLE_RATE_48000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_48000);
            SetSPORightChanSampRate(SAMPLE_RATE_48000);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_48);

            break;
        }
        
        case SAMPLE_RATE_96000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_96000);
            SetSPORightChanSampRate(SAMPLE_RATE_96000);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_24);

            break;
        }
        
        case SAMPLE_RATE_192000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_192000);
            SetSPORightChanSampRate(SAMPLE_RATE_192000);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_12);

            break;
        }
        
        case SAMPLE_RATE_384000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_384000);
            SetSPORightChanSampRate(SAMPLE_RATE_384000);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_6);

            break;
        }
        
        case SAMPLE_RATE_768000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_768000);
            SetSPORightChanSampRate(SAMPLE_RATE_768000);

            SetSPOMCLKDivBySoftware(SPO_CLK_SW_DIV_3);

            break;
        }
        
        default:
        {
            break;
        }
    }
}



static void ConfigF128FSBySoftware(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x10 ; bit: 17-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_SRC_CLK_SEL_0X10);

    switch (SampleRate)
    {
        case SAMPLE_RATE_8000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_192);

            break;
        }
        
        case SAMPLE_RATE_16000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_96);

            break;
        }
        
        case SAMPLE_RATE_32000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_32000);
            SetSPORightChanSampRate(SAMPLE_RATE_32000);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_48);

            break;
        }
        
        case SAMPLE_RATE_64000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_64000);
            SetSPORightChanSampRate(SAMPLE_RATE_64000);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_24);

            break;
        }
        
        case SAMPLE_RATE_128000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_128000);
            SetSPORightChanSampRate(SAMPLE_RATE_128000);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_12);

            break;
        }
        
        case SAMPLE_RATE_256000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_256000);
            SetSPORightChanSampRate(SAMPLE_RATE_256000);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_6);

            break;
        }
        
        case SAMPLE_RATE_512000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_512000);
            SetSPORightChanSampRate(SAMPLE_RATE_512000);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_3);

            break;
        }
        
        case SAMPLE_RATE_11025:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_192);

            break;
        }
        
        case SAMPLE_RATE_22050:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_22050);
            SetSPORightChanSampRate(SAMPLE_RATE_22050);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_96);

            break;
        }
        
        case SAMPLE_RATE_44100:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_44100);
            SetSPORightChanSampRate(SAMPLE_RATE_44100);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_48);

            break;
        }
        
        case SAMPLE_RATE_88200:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_88200);
            SetSPORightChanSampRate(SAMPLE_RATE_88200);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_24);

            break;
        }
        
        case SAMPLE_RATE_176400:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_176400);
            SetSPORightChanSampRate(SAMPLE_RATE_176400);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_12);

            break;
        }
        
        case SAMPLE_RATE_352800:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_352800);
            SetSPORightChanSampRate(SAMPLE_RATE_352800);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_6);

            break;
        }
        
        case SAMPLE_RATE_705600:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_705600);
            SetSPORightChanSampRate(SAMPLE_RATE_705600);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_3);

            break;
        }
        
        case SAMPLE_RATE_12000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_192);

            break;
        }
        
        case SAMPLE_RATE_24000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_24000);
            SetSPORightChanSampRate(SAMPLE_RATE_24000);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_96);

            break;
        }
        
        case SAMPLE_RATE_48000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_48000);
            SetSPORightChanSampRate(SAMPLE_RATE_48000);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_48);

            break;
        }
        
        case SAMPLE_RATE_96000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_96000);
            SetSPORightChanSampRate(SAMPLE_RATE_96000);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_24);

            break;
        }
        
        case SAMPLE_RATE_192000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_192000);
            SetSPORightChanSampRate(SAMPLE_RATE_192000);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_12);

            break;
        }
        
        case SAMPLE_RATE_384000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_384000);
            SetSPORightChanSampRate(SAMPLE_RATE_384000);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_6);

            break;
        }
        
        case SAMPLE_RATE_768000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_768000);
            SetSPORightChanSampRate(SAMPLE_RATE_768000);

            SetI2SOF128FSSoftDiv(I2SO_F128_DIV_3);

            break;
        }
        
        default:
        {
            break;
        }
    }
}



static void ConfigSPOFSByHardware(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x14 ; bit: 21-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_FS_CFG1_0X14);

    switch (SampleRate)
    {
        case SAMPLE_RATE_8000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x00<<16)));

            break;
        }
        
        case SAMPLE_RATE_16000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x01<<16)));
            break;
        }
        
        case SAMPLE_RATE_32000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x02<<16)));
            
            SetSPOLeftChanSampRate(SAMPLE_RATE_32000);
            SetSPORightChanSampRate(SAMPLE_RATE_32000);

            break;
        }
        
        case SAMPLE_RATE_64000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x03<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_64000);
            SetSPORightChanSampRate(SAMPLE_RATE_64000);

            break;
        }
        
        case SAMPLE_RATE_128000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x04<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_128000);
            SetSPORightChanSampRate(SAMPLE_RATE_128000);

            break;
        }
        
        case SAMPLE_RATE_256000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x05<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_256000);
            SetSPORightChanSampRate(SAMPLE_RATE_256000);

            break;
        }
        
        case SAMPLE_RATE_512000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x06<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_512000);
            SetSPORightChanSampRate(SAMPLE_RATE_512000);

            break;
        }
        
        case SAMPLE_RATE_11025:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x10<<16)));

            break;
        }
        
        case SAMPLE_RATE_22050:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x11<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_22050);
            SetSPORightChanSampRate(SAMPLE_RATE_22050);

            break;
        }
        
        case SAMPLE_RATE_44100:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x12<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_44100);
            SetSPORightChanSampRate(SAMPLE_RATE_44100);

            break;
        }
        
        case SAMPLE_RATE_88200:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x13<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_88200);
            SetSPORightChanSampRate(SAMPLE_RATE_88200);

            break;
        }
        
        case SAMPLE_RATE_176400:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x14<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_176400);
            SetSPORightChanSampRate(SAMPLE_RATE_176400);

            break;
        }
        
        case SAMPLE_RATE_352800:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x15<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_352800);
            SetSPORightChanSampRate(SAMPLE_RATE_352800);

            break;
        }
        
        case SAMPLE_RATE_705600:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x16<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_705600);
            SetSPORightChanSampRate(SAMPLE_RATE_705600);

            break;
        }
        
        case SAMPLE_RATE_12000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x20<<16)));

            break;
        }
        
        case SAMPLE_RATE_24000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x21<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_24000);
            SetSPORightChanSampRate(SAMPLE_RATE_24000);

            break;
        }
        
        case SAMPLE_RATE_48000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x22<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_48000);
            SetSPORightChanSampRate(SAMPLE_RATE_48000);

            break;
        }
        
        case SAMPLE_RATE_96000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x23<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_96000);
            SetSPORightChanSampRate(SAMPLE_RATE_96000);

            break;
        }
        
        case SAMPLE_RATE_192000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x24<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_192000);
            SetSPORightChanSampRate(SAMPLE_RATE_192000);

            break;
        }
        
        case SAMPLE_RATE_384000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x25<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_384000);
            SetSPORightChanSampRate(SAMPLE_RATE_384000);

            break;
        }
        
        case SAMPLE_RATE_768000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<16))) | (0x26<<16)));

            SetSPOLeftChanSampRate(SAMPLE_RATE_768000);
            SetSPORightChanSampRate(SAMPLE_RATE_768000);

            break;
        }
        
        default:
        {
            break;
        }
    }
}


static void SelectF128LRCLKDivMode(SPO_LRCLK_DivMode_t DivMode)
{
    return;
}



static void SelectF128FSMode(AUD_F128_FS_Mode_t Mode)
{
    // reg: 0x14 ; bit: 9-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_FS_CFG1_0X14);

    switch (Mode)
    {
        case AUD_128xI2SO_FS:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x03<<8))) | (0x0<<8)));

            break;
        }

        case AUD_128xSPO_FS:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x03<<8))) | (0x1<<8)));

            break;
        }

        case AUD_128xDDPSPO_FS:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x03<<8))) | (0x2<<8)));

            break;
        }

        default:
        {
            break;
        }
    }
}



void EnableSPOInterface(AUD_BOOL Enable)
{
    // reg offset: 0x04 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<1))) | (0x01<<1)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<1))) | (0x00<<1)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}


EXPORT_SYMBOL_GPL(EnableSPOInterface);

static AUD_BOOL CheckSPOInfEnableStatus(void)
{
    // reg offset: 0x04 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

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
    // reg offset: 0x04 ; bit: 9

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<9))) | (0x01<<9)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<9))) | (0x00<<9)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



/* SPO DMA control ---------------------------------------------------------- */

void StartSPODMA(AUD_BOOL Enable)
{
    // reg offset: 0x04 ; bit: 17

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<17))) | (0x01<<17)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<17))) | (0x00<<17)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}

EXPORT_SYMBOL_GPL(StartSPODMA);
AUD_BOOL GetSPODMAStatus(void)
{
    // reg offset: 0x04 ; bit: 17

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if(RegValue&(0x01<<17))
      return TRUE;
    else
      return FALSE;

}

EXPORT_SYMBOL_GPL(GetSPODMAStatus);


/* Get SPO interface information -------------------------------------------- */

static AUD_UINT16 GetSPODMABufLastIndex(void)
{
    // reg offset: 0x108 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_DMA_INDEX_0X108);

    return (AUD_UINT16)(RegValue&0xFFFF);
}



static AUD_UINT16 GetSPODMABufCurrentIndex(void)
{
    // reg offset: 0x108 ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_DMA_INDEX_0X108);

    return (AUD_UINT16)((RegValue&(0xFFFF<<16))>>16);
}



static AUD_UINT16 GetSPOSampCount(void)
{
    // reg offset: 0x11C ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_SAMPLE_COUNTER_0X11C);

    return (AUD_UINT16)((RegValue&(0xFFFF<<16))>>16);
}



static AUD_UINT8 GetSPOFrameLenInfo(void)
{
    // reg offset: 0x120 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_FRAME_HEAD_INFO_0X120);

    return (AUD_UINT16)(RegValue&0xFFFF);
}



static AUD_UINT16 GetSPOFramePDInfo(void)
{
    // reg offset: 0x120 ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_FRAME_HEAD_INFO_0X120);

    return (AUD_UINT16)((RegValue&(0xFFFF<<16))>>16);
}



static AUD_UINT16 GetSPOFrameNullDataInfo(void)
{
    // reg offset: 0x124 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_FRAME_HEAD_INFO2_0X124);

    return (AUD_UINT16)(RegValue&0xFFFF);
}



static AUD_UINT16 GetSPOFrameBurstDataInfo(void)
{
    // reg offset: 0x124 ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_FRAME_HEAD_INFO2_0X124);

    return (AUD_UINT16)((RegValue&(0xFFFF<<16))>>16);
}



static AUD_BOOL GetSPOBufUnderRunStatus(void)
{
    // reg offset: 0x10C ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_DMA_UNDER_RUN_0X10C);

    if (RegValue&(0x01<<0))
    {
        AUD_WriteRegDev32(R_SPO_DMA_UNDER_RUN_0X10C, RegValue);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL GetSPOFIFOUnderRunStatus(void)
{
    // reg offset: 0x10C ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_DMA_UNDER_RUN_0X10C);

    if (RegValue&(0x01<<1))
    {
        AUD_WriteRegDev32(R_SPO_DMA_UNDER_RUN_0X10C, \
                          (RegValue&(~(0x01))));

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_UINT16 GetSPODMASkipNum(void)
{
    // reg offset: 0x128 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_DMA_SKIP_NUM_0X128);

    return (AUD_UINT16)(RegValue&0xFFFF);
}



/* Enable SPO interrupt ----------------------------------------------------- */

void EnableSPOSampCountInt(AUD_BOOL Enable)
{
    // reg offset: 0x08 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST1_0X08);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST1_0X08, \
                          ((RegValue&(~(0x01<<1))) | (0x01<<1)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST1_0X08, \
                          ((RegValue&(~(0x01<<1))) | (0x00<<1)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}

EXPORT_SYMBOL_GPL(EnableSPOSampCountInt);

static void EnableSPODMABufResumeInt(AUD_BOOL Enable)
{
    // reg offset: 0x0C ; bit: 17

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          ((RegValue&(~(0x01<<17))) | (0x01<<17)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          ((RegValue&(~(0x01<<17))) | (0x00<<17)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



void EnableSPODMAUnderRunInt(AUD_BOOL Enable)
{
    // reg offset: 0x0C ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          ((RegValue&(~(0x01<<1))) | (0x01<<1)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          ((RegValue&(~(0x01<<1))) | (0x00<<1)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}

EXPORT_SYMBOL_GPL(EnableSPODMAUnderRunInt);
    
/* Get SPO interrupt status ------------------------------------------------- */

AUD_BOOL CheckSPOSampCountInt(void)
{
    // reg offset: 0x08 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST1_0X08);

    if (RegValue&(0x01<<1))
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
    // reg offset: 0x08 ; bit: 17

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST1_0X08);

    if (RegValue&(0x01<<17))
    {
        AUD_WriteRegDev32(R_AUD_INT_ST1_0X08, \
                          (RegValue|(0x01<<17)));
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
    // reg offset: 0x0C ; bit: 17

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (RegValue&(0x01<<17))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL CheckSPODMABufResumeIntStatus(void)
{
    // reg offset: 0x0C ; bit: 25

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (RegValue&(0x01<<25))
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          (RegValue|(0x01<<25)));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL CheckSPODMAUnderRunInt(void)
{
    // reg offset: 0x0C ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (RegValue&(0x01<<1))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



AUD_BOOL CheckSPODMABufUnderRunIntStatus(void)
{
    // reg offset: 0x0C ; bit: 9

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (RegValue&(0x01<<9))
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          (RegValue|(0x01<<9)));
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
    // reg offset: 0x180 ; bit: 31-0

    if ((Addr%32) != 0)
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
    
    AUD_WriteRegDev32(R_DDP_SPO_DMA_BASE_ADDR_0X180, \
                      (Addr&0x0FFFFFFF));
}



static void SetDDPSPODMABufLength(AUD_UINT16 Length)
{
    // reg offset: 0x184 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    if ((Length%2) != 0)
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
    
    RegValue = AUD_ReadRegDev32(R_DDP_SPO_DMA_CFG_0X184);

    AUD_WriteRegDev32(R_DDP_SPO_DMA_CFG_0X184, \
                      ((RegValue&(~(0xFFFF<<0))) | ((Length&0xFFFF)<<0)));
}



static void SetDDPSPODMABufLastIndex(AUD_UINT16 Index)
{
    // reg offset: 0x188 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_DMA_INDEX_0X188);

    AUD_WriteRegDev32(R_DDP_SPO_DMA_INDEX_0X188, \
                      ((RegValue&(~(0xFFFF<<0))) | ((Index&0xFFFF)<<0)));
}



static void SetDDPSPOFramePCInfo(AUD_UINT16 PCInfo)
{
    // reg offset: 0x19C ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_IEC_PC_0X194);

    AUD_WriteRegDev32(R_DDP_SPO_IEC_PC_0X194, \
                      ((RegValue&(~(0xFFFF<<0))) | ((PCInfo&0xFFFF)<<0)));
}



/* DDP SPO interface confingure relative operation -------------------------- */

static void SetDDPSPOSampCountThreshold(AUD_UINT16 Threshold)
{
    // reg offset: 0x19C ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_SMAPLE_COUNTER_0X19C);

    AUD_WriteRegDev32(R_DDP_SPO_SMAPLE_COUNTER_0X19C, \
                      ((RegValue&(~(0xFFFF<<0))) | ((Threshold&0xFFFF)<<0)));
}



/* DDPlus SPIDF Output DMA config start --> */

static void EnableDDPSPODMADataWithHeader(AUD_BOOL Enable)
{
    // reg offset: 0x184 ; bit: 17

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_DMA_CFG_0X184);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_DMA_CFG_0X184, \
                          ((RegValue&(~(0x01<<17))) | (0x00<<17)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_DMA_CFG_0X184, \
                          ((RegValue&(~(0x01<<17))) | (0x01<<17)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SetDDPSPODMADataLen(DDPSPO_DMA_DataLen_t Len)
{
    // reg offset: 0x184 ; bit: 16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_DMA_CFG_0X184);

    if (DDP_DATA_LEN_16_BITS == Len)
    {
        AUD_WriteRegDev32(R_DDP_SPO_DMA_CFG_0X184, \
                          ((RegValue&(~(0x01<<16))) | (0x00<<16)));
    }
    else if (DDP_DATA_LEN_24_BITS == Len)
    {
        AUD_WriteRegDev32(R_SPO_DMA_CFG_0X104, \
                          ((RegValue&(~(0x01<<16))) | (0x01<<16)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}

/* <-- DDPlus SPIDF Output DMA config end */



static void SetDDPSPOLeftChanClkAccurary(AUD_UINT8 Accuray)
{
    // reg offset: 0x1E0 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0, \
                      ((RegValue&(~(0x03<<28))) | ((Accuray&0x03)<<28)));
}



static AUD_UINT8 GetDDPSPOLeftChanClkAccurary(void)
{
    // reg offset: 0x1E0 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    return (AUD_UINT8)((RegValue&(0x03<<28))>>28);
}



static void SetDDPSPOLeftChanSampRate(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x1E0 ; bit: 31-30, 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT32 FS0 = 0;
    AUD_UINT32 FS1 = 0;

    switch (SampleRate)
    {
        case SAMPLE_RATE_22050:
        {
            FS0 = 0x04;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_44100:
        {
            FS0 = 0x00;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_88200:
        {
            FS0 = 0x08;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_176400:
        {
            FS0 = 0x0C;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_24000:
        {
            FS0 = 0x06;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_48000:
        {
            FS0 = 0x02;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_96000:
        {
            FS0 = 0x0A;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_192000:
        {
            FS0 = 0x0E;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_32000:
        {
            FS0 = 0x03;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_768000:
        {
            FS0 = 0x09;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_64000:
        {
            FS0 = 0x0B;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_128000:
        {
            FS0 = 0x0B;
            FS1 = 0x02;
            break;
        }

        case SAMPLE_RATE_256000:
        {
            FS0 = 0x0B;
            FS1 = 0x01;
            break;
        }

        case SAMPLE_RATE_512000:
        {
            FS0 = 0x0B;
            FS1 = 0x03;
            break;
        }

        case SAMPLE_RATE_1024000:
        {
            FS0 = 0x05;
            FS1 = 0x03;
            break;
        }

        case SAMPLE_RATE_384000:
        {
            FS0 = 0x05;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_1536000:
        {
            FS0 = 0x05;
            FS1 = 0x01;
            break;
        }

        case SAMPLE_RATE_352800:
        {
            FS0 = 0x0D;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_705600:
        {
            FS0 = 0x0D;
            FS1 = 0x02;
            break;
        }

        case SAMPLE_RATE_1411200:
        {
            FS0 = 0x0D;
            FS1 = 0x01;
            break;
        }

        default:
        {
            FS0 = 0x00;
            FS1 = 0x00;            
        }
    }

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0, \
                      ((RegValue&(~(0x0F<<24))) | ((FS0&0x0F)<<24) | ((FS1&0x03)<<30)));
}



static AUD_SampleRate_t GetDDPSPOLeftChanSampRate(void)
{
    // reg offset: 0x1E0 ; bit: 31-30, 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT8 FS0 = 0;
    AUD_UINT8 FS1 = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    FS0 = (AUD_UINT8)((RegValue&(0x0F<<24))>>24);
    FS1 = (AUD_UINT8)((RegValue&(0x03<<30))>>30);

    if ((0x04 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_22050;
    }
    else if ((0x00 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_44100;
    }
    else if ((0x08 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_88200;
    }
    else if ((0x0C == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_176400;
    }
    else if ((0x06 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_24000;
    }
    else if ((0x02 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_48000;
    }
    else if ((0x0A == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_96000;
    }
    else if ((0x0E == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_192000;
    }
    else if ((0x03 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_32000;
    }
    else if ((0x09 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_768000;
    }
    else if ((0x0B == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_64000;
    }
    else if ((0x0B == FS0) && (0x02 == FS1))
    {
        return SAMPLE_RATE_128000;
    }
    else if ((0x0B == FS0) && (0x01 == FS1))
    {
        return SAMPLE_RATE_256000;
    }
    else if ((0x0B == FS0) && (0x03 == FS1))
    {
        return SAMPLE_RATE_512000;
    }
    else if ((0x05 == FS0) && (0x03 == FS1))
    {
        return SAMPLE_RATE_1024000;
    }
    else if ((0x05 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_384000;
    }
    else if ((0x05 == FS0) && (0x01 == FS1))
    {
        return SAMPLE_RATE_1536000;
    }
    else if ((0x0D == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_352800;
    }
    else if ((0x0D == FS0) && (0x02 == FS1))
    {
        return SAMPLE_RATE_705600;
    }
    else if ((0x0D == FS0) && (0x01 == FS1))
    {
        return SAMPLE_RATE_1411200;
    }
    else if ((0x08 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_88200;
    }
}



static void SetDDPSPOLeftChanNum(AUD_UINT8 ChannelNum)
{
    // reg offset: 0x1E0 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0, \
                      ((RegValue&(~(0x0F<<20))) | ((ChannelNum&0x0F)<<20)));
}



static AUD_UINT8 GetDDPSPOLeftChanNum(void)
{
    // reg offset: 0x1E0 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    return (AUD_UINT8)((RegValue&(0x0F<<20))>>20);
}



static void SetDDPSPOLeftChanSourcelNum(AUD_UINT8 SourcelNum)
{
    // reg offset: 0x1E0 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0, \
                      ((RegValue&(~(0x0F<<16))) | ((SourcelNum&0x0F)<<16)));
}



static AUD_UINT8 GetDDPSPOLeftChanSourcelNum(void)
{
    // reg offset: 0x1E0 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    return (AUD_UINT8)((RegValue&(0x0F<<16))>>16);
}



static void SetDDPSPOLeftChanLBit(AUD_UINT8 LBit)
{
    // reg offset: 0x1E0 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0, \
                      ((RegValue&(~(0x01<<15))) | ((LBit&0x01)<<15)));
}



static AUD_UINT8 GetDDPSPOLeftChanLBit(void)
{
    // reg offset: 0x1E0 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    return (AUD_UINT8)((RegValue&(0x01<<15))>>15);
}



static void SetDDPSPOLeftChanCategory(AUD_UINT8 Category)
{
    // reg offset: 0x1E0 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0, \
                      ((RegValue&(~(0x7F<<8))) | ((Category&0x7F)<<8)));
}



static AUD_UINT16 GetDDPSPOLeftChanCategory(void)
{
    // reg offset: 0x1E0 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    return (AUD_UINT16)((RegValue&(0x7F<<8))>>8);
}



static void SetDDPSPOLeftChanStatusMode(AUD_UINT8 Mode)
{
    // reg offset: 0x1E0 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0, \
                      ((RegValue&(~(0x03<<6))) | ((Mode&0x03)<<6)));
}



static AUD_UINT8 GetDDPSPOLeftChanStatusMode(void)
{
    // reg offset: 0x1E0 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    return (AUD_UINT8)((RegValue&(0x03<<6))>>6);
}



static void SetDDPSPOLeftChanAddFormatInfo(AUD_UINT8 AddFormatInfo)
{
    // reg offset: 0x1E0 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0, \
                      ((RegValue&(~(0x07<<3))) | ((AddFormatInfo&0x07)<<3)));
}



static AUD_UINT8 GetDDPSPOLeftChanAddFormatInfo(void)
{
    // reg offset: 0x1E0 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    return (AUD_UINT8)((RegValue&(0x07<<3))>>3);
}



static void EnableDDPSPOLeftChanCopyright(AUD_BOOL Enable)
{
    // reg offset: 0x1E0 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0, \
                          ((RegValue&(~(0x01<<2))) | (0x01<<2)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0, \
                          ((RegValue&(~(0x01<<2))) | (0x00<<2)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static AUD_UINT8 GetDDPSPOLeftChanCopyright(void)
{
    // reg offset: 0x1E0 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    return (AUD_UINT8)((RegValue&(0x01<<2))>>2);
}



static void SetDDPSPOLeftChanAudContent(DDPSPO_AudioContentFlag_t AudContent)
{
    // reg offset: 0x1E0 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0, \
                      ((RegValue&(~(0x01<<1))) | ((AudContent&0x01)<<1)));
}



static AUD_UINT8 GetDDPSPOLeftChanAudContent(void)
{
    // reg offset: 0x1E0 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    return (AUD_UINT8)((RegValue&(0x01<<1))>>1);
}



static void SetDDPSPOLeftChanUseFlag(AUD_UINT8 UseFlag)
{
    // reg offset: 0x1E0 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0, \
                      ((RegValue&(~0x01)) | (UseFlag&0x01)));
}



static AUD_UINT8 GetDDPSPOLeftChanUseFlag(void)
{
    // reg offset: 0x1E0 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_L_0X1E0);

    return (AUD_UINT8)(RegValue&0x01);
}



static void ConfigDDPSPOLeftChanUserData(AUD_UINT32 *DataAddr)
{
    // reg offset: 0x1B0 ; bit: 192-0

    AUD_UINT8 i = 0;

    for (i=0; i<6; i++)
    {
        AUD_WriteRegDev32((R_DDP_SPO_LEFT_USER_DATA1_0X1B0 + i*4), \
                          DataAddr[i]);
    }
}



static void SetDDPSPORightChanClkAccurary(AUD_UINT8 Accuray)
{
    // reg offset: 0x1E4 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4, \
                      ((RegValue&(~(0x03<<28))) | ((Accuray&0x03)<<28)));
}



static AUD_UINT8 GetDDPSPORightChanClkAccurary(void)
{
    // reg offset: 0x1E4 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    return (AUD_UINT8)((RegValue&(0x03<<28))>>28);
}



static void SetDDPSPORightChanSampRate(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x1E4 ; bit: 31-30, 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT32 FS0 = 0;
    AUD_UINT32 FS1 = 0;

    switch (SampleRate)
    {
        case SAMPLE_RATE_22050:
        {
            FS0 = 0x04;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_44100:
        {
            FS0 = 0x00;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_88200:
        {
            FS0 = 0x08;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_176400:
        {
            FS0 = 0x0C;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_24000:
        {
            FS0 = 0x06;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_48000:
        {
            FS0 = 0x02;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_96000:
        {
            FS0 = 0x0A;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_192000:
        {
            FS0 = 0x0E;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_32000:
        {
            FS0 = 0x03;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_768000:
        {
            FS0 = 0x09;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_64000:
        {
            FS0 = 0x0B;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_128000:
        {
            FS0 = 0x0B;
            FS1 = 0x02;
            break;
        }

        case SAMPLE_RATE_256000:
        {
            FS0 = 0x0B;
            FS1 = 0x01;
            break;
        }

        case SAMPLE_RATE_512000:
        {
            FS0 = 0x0B;
            FS1 = 0x03;
            break;
        }

        case SAMPLE_RATE_1024000:
        {
            FS0 = 0x05;
            FS1 = 0x03;
            break;
        }

        case SAMPLE_RATE_384000:
        {
            FS0 = 0x05;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_1536000:
        {
            FS0 = 0x05;
            FS1 = 0x01;
            break;
        }

        case SAMPLE_RATE_352800:
        {
            FS0 = 0x0D;
            FS1 = 0x00;
            break;
        }

        case SAMPLE_RATE_705600:
        {
            FS0 = 0x0D;
            FS1 = 0x02;
            break;
        }

        case SAMPLE_RATE_1411200:
        {
            FS0 = 0x0D;
            FS1 = 0x01;
            break;
        }

        default:
        {
            FS0 = 0x00;
            FS1 = 0x00;            
        }
    }

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4, \
                      ((RegValue&(~(0x0F<<24))) | ((FS0&0x0F)<<24) | ((FS1&0x03)<<30)));
}



static AUD_SampleRate_t GetDDPSPORightChanSampRate(void)
{
    // reg offset: 0x1E4 ; bit: 31-30, 27-24

    AUD_UINT32 RegValue = 0;
    AUD_UINT8 FS0 = 0;
    AUD_UINT8 FS1 = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    FS0 = (AUD_UINT8)((RegValue&(0x0F<<24))>>24);
    FS1 = (AUD_UINT8)((RegValue&(0x03<<30))>>30);

    if ((0x04 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_22050;
    }
    else if ((0x00 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_44100;
    }
    else if ((0x08 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_88200;
    }
    else if ((0x0C == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_176400;
    }
    else if ((0x06 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_24000;
    }
    else if ((0x02 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_48000;
    }
    else if ((0x0A == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_96000;
    }
    else if ((0x0E == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_192000;
    }
    else if ((0x03 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_32000;
    }
    else if ((0x09 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_768000;
    }
    else if ((0x0B == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_64000;
    }
    else if ((0x0B == FS0) && (0x02 == FS1))
    {
        return SAMPLE_RATE_128000;
    }
    else if ((0x0B == FS0) && (0x01 == FS1))
    {
        return SAMPLE_RATE_256000;
    }
    else if ((0x0B == FS0) && (0x03 == FS1))
    {
        return SAMPLE_RATE_512000;
    }
    else if ((0x05 == FS0) && (0x03 == FS1))
    {
        return SAMPLE_RATE_1024000;
    }
    else if ((0x05 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_384000;
    }
    else if ((0x05 == FS0) && (0x01 == FS1))
    {
        return SAMPLE_RATE_1536000;
    }
    else if ((0x0D == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_352800;
    }
    else if ((0x0D == FS0) && (0x02 == FS1))
    {
        return SAMPLE_RATE_705600;
    }
    else if ((0x0D == FS0) && (0x01 == FS1))
    {
        return SAMPLE_RATE_1411200;
    }
    else if ((0x08 == FS0) && (0x00 == FS1))
    {
        return SAMPLE_RATE_88200;
    }
}



static void SetDDPSPORightChanNum(AUD_UINT8 ChannelNum)
{
    // reg offset: 0x1E4 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4, \
                      ((RegValue&(~(0x0F<<20))) | ((ChannelNum&0x0F)<<20)));
}



static AUD_UINT8 GetDDPSPORightChanNum(void)
{
    // reg offset: 0x1E4 ; bit: 23-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    return (AUD_UINT8)((RegValue&(0x0F<<20))>>20);
}



static void SetDDPSPORightChanSourcelNum(AUD_UINT8 SourcelNum)
{
    // reg offset: 0x1E4 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4, \
                      ((RegValue&(~(0x0F<<16))) | ((SourcelNum&0x0F)<<16)));
}



static AUD_UINT8 GetDDPSPORightChanSourcelNum(void)
{
    // reg offset: 0x1E4 ; bit: 19-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    return (AUD_UINT8)((RegValue&(0x0F<<16))>>16);
}



static void SetDDPSPORightChanLBit(AUD_UINT8 LBit)
{
    // reg offset: 0x1E4 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4, \
                      ((RegValue&(~(0x01<<15))) | ((LBit&0x01)<<15)));
}



static AUD_UINT8 GetDDPSPORightChanLBit(void)
{
    // reg offset: 0x1E4 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    return (AUD_UINT8)((RegValue&(0x01<<15))>>15);
}



static void SetDDPSPORightChanCategory(AUD_UINT8 Category)
{
    // reg offset: 0x1E4 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4, \
                      ((RegValue&(~(0x7F<<8))) | ((Category&0x7F)<<8)));
}



static AUD_UINT16 GetDDPSPORightChanCategory(void)
{
    // reg offset: 0x1E4 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    return (AUD_UINT16)((RegValue&(0x7F<<8))>>8);
}



static void SetDDPSPORightChanStatusMode(AUD_UINT8 Mode)
{
    // reg offset: 0x1E4 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4, \
                      ((RegValue&(~(0x03<<6))) | ((Mode&0x03)<<6)));
}



static AUD_UINT8 GetDDPSPORightChanStatusMode(void)
{
    // reg offset: 0x1E4 ; bit: 7-6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    return (AUD_UINT8)((RegValue&(0x03<<6))>>6);
}



static void SetDDPSPORightChanAddFormatInfo(AUD_UINT8 AddFormatInfo)
{
    // reg offset: 0x1E4 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4, \
                      ((RegValue&(~(0x07<<3))) | ((AddFormatInfo&0x07)<<3)));
}



static AUD_UINT8 GetDDPSPORightChanAddFormatInfo(void)
{
    // reg offset: 0x1E4 ; bit: 5-3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    return (AUD_UINT8)((RegValue&(0x07<<3))>>3);
}



static void SetDDPSPORightChanCopyright(AUD_UINT8 Copyright)
{
    // reg offset: 0x1E4 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4, \
                      ((RegValue&(~(0x01<<2))) | ((Copyright&0x01)<<2)));
}



static AUD_UINT8 GetDDPSPORightChanCopyright(void)
{
    // reg offset: 0x1E4 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    return (AUD_UINT8)((RegValue&(0x01<<2))>>2);
}



static void SetDDPSPORightChanAudContent(DDPSPO_AudioContentFlag_t AudContent)
{
    // reg offset: 0x1E4 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4, \
                      ((RegValue&(~(0x01<<1))) | ((AudContent&0x01)<<1)));
}



static AUD_UINT8 GetDDPSPORightChanAudContent(void)
{
    // reg offset: 0x1E4 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    return (AUD_UINT8)((RegValue&(0x01<<1))>>1);
}



static void SetDDPSPORightChanUseFlag(AUD_UINT8 UseFlag)
{
    // reg offset: 0x1E4 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    AUD_WriteRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4, \
                      ((RegValue&(~0x01)) | (UseFlag&0x01)));
}



static AUD_UINT8 GetDDPSPORightChanUseFlag(void)
{
    // reg offset: 0x1E4 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_CHANNEL_STATUS_R_0X1E4);

    return (AUD_UINT8)(RegValue&0x01);
}



static void ConfigDDPSPORightChanUserData(AUD_UINT32 *DataAddr)
{
    // reg offset: 0x1C8 ; bit: 192-0

    AUD_UINT8 i = 0;

    for (i=0; i<6; i++)
    {
        AUD_WriteRegDev32((R_DDP_SPO_RIGHT_USER_DATA1_0X1C8 + i*4), \
                          DataAddr[i]);
    }
}



/* DDPlus SPDIF Output clock config start --> */

static void SelectDDPSPOMCLKDivMode(DDPSPO_MCLK_DivMode_t Mode)
{
    return;
}



static void EnableDDPSPOFSSoftwareConfig(AUD_BOOL Enable)
{
    // reg offset: 0x198 ; bit: 16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_FS_CFG_0X198);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_FS_CFG_0X198, \
                          ((RegValue&(~(0x01<<16))) | (0x01<<16)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_FS_CFG_0X198, \
                          ((RegValue&(~(0x01<<16))) | (0x00<<16)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableDDPSPOFSGated(AUD_BOOL Enable)
{
    // reg offset: 0x198 ; bit: 22

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_FS_CFG_0X198);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_FS_CFG_0X198, \
                          ((RegValue&(~(0x01<<22))) | (0x01<<22)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_FS_CFG_0X198, \
                          ((RegValue&(~(0x01<<22))) | (0x00<<22)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DivMode_t FSDiv)
{
    // reg: 0x198 ; bit: 15-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_FS_CFG_0X198);

    switch (FSDiv)
    {
        case DDPSPO_CLK_SW_DIV_3:
        {
            AUD_WriteRegDev32(R_DDP_SPO_FS_CFG_0X198, \
                              ((RegValue&(~(0x0F<<8))) | ((3&0x0F)<<8)));
            break;
        }

        case DDPSPO_CLK_SW_DIV_6:
        {
            AUD_WriteRegDev32(R_DDP_SPO_FS_CFG_0X198, \
                              ((RegValue&(~(0x0F<<8))) | ((6&0x0F)<<8)));
            break;
        }

        case DDPSPO_CLK_SW_DIV_12:
        {
            AUD_WriteRegDev32(R_DDP_SPO_FS_CFG_0X198, \
                              ((RegValue&(~(0x0F<<8))) | ((12&0x0F)<<8)));
            break;
        }

        case DDPSPO_CLK_SW_DIV_24:
        {
            AUD_WriteRegDev32(R_DDP_SPO_FS_CFG_0X198, \
                              ((RegValue&(~(0x0F<<8))) | ((24&0x0F)<<8)));
            break;
        }

        case DDPSPO_CLK_SW_DIV_48:
        {
            AUD_WriteRegDev32(R_DDP_SPO_FS_CFG_0X198, \
                              ((RegValue&(~(0x0F<<8))) | ((48&0x0F)<<8)));
            break;
        }

        case DDPSPO_CLK_SW_DIV_96:
        {
            AUD_WriteRegDev32(R_DDP_SPO_FS_CFG_0X198, \
                              ((RegValue&(~(0x0F<<8))) | ((96&0x0F)<<8)));
            break;
        }

        case DDPSPO_CLK_SW_DIV_192:
        {
            AUD_WriteRegDev32(R_DDP_SPO_FS_CFG_0X198, \
                              ((RegValue&(~(0x0F<<8))) | ((192&0x0F)<<8)));
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
    // reg offset: 0x190 ; bit: 16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_INTF_CTRL_0X190);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<16))) | (0x01<<16)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<16))) | (0x00<<16)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableDDPSPODataReorder(AUD_BOOL Enable)
{
    // reg offset: 0x190 ; bit: 6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_INTF_CTRL_0X190);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<6))) | (0x01<<6)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<6))) | (0x00<<6)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableDDPSPOValidBit(AUD_BOOL Enable)
{
    // reg offset: 0x190 ; bit: 4

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_INTF_CTRL_0X190);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<4))) | (0x01<<4)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<4))) | (0x00<<4)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableDDPSPONormalPlay(AUD_BOOL Enable)
{
    // reg offset: 0x190 ; bit: 17

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_INTF_CTRL_0X190);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<17))) | (0x00<<17)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<17))) | (0x01<<17)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectDDPSPOChannelStatusReg(DDPSPO_ChanStatusReg_t Mode)
{
    // reg offset: 0x190 ; bit: 5

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_INTF_CTRL_0X190);

    if (DDP_RIGHT_CHAN_STATUS_REG == Mode) // CSD_R use CSD_R register value
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<5))) | (0x01<<5)));
    }
    else if (DDP_LEFT_CHAN_STATUS_REG == Mode) // CSD_R use CSD_L register value
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<5))) | (0x00<<5)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectDDPSPOClkAndDataMode(DDP_SPO_OutputMode_t Mode)
{
    // reg offset: 0x190 ; bit: 24

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_SPO_INTF_CTRL_0X110);

    if (SPO_OUT_CLK_DATA == Mode) // From SPO DMA
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<24))) | (0x01<<24)));
    }
    else if (DDPSPO_OUT_CLK_DATA == Mode) // From DDP SPO DMA
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<24))) | (0x00<<24)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}


static AUD_BOOL CheckDDPSPOClkAndDataMode(void)
{
    // reg offset: 0x190 ; bit: 24

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_INTF_CTRL_0X190);

    if (RegValue & (0x01<<24))
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
    // reg offset: 0x190 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_INTF_CTRL_0X190);

    if (DDP_FROM_DDPSPO_DMA == Source)
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<2))) | (0x01<<2)));        
    }
    else if (DDP_FROM_I2SO_DMA == Source)
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          (RegValue&(~(0x01<<2))));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectDDPSPOPCMOutChan(DDPSPO_PCM_OutChanMode_t Mode)
{
    // reg offset: 0x190 ; bit: 1-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_INTF_CTRL_0X190);

    AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                      ((RegValue&(~(0x03<<0))) | ((Mode&0x03)<<0)));
}



static void SelectDDPSPOPCMOutLFEMode(DDPSPO_PCM_OutLFEMode_t Mode)
{
    // reg offset: 0x190 ; bit: 3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_INTF_CTRL_0X190);

    if (DDPSPO_L_I2SO_LFE == Mode) 
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<3))) | (0x01<<3)));
    }
    else if (DDPSPO_R_I2SO_LFE == Mode)
    {
        AUD_WriteRegDev32(R_DDP_SPO_INTF_CTRL_0X190, \
                          ((RegValue&(~(0x01<<3))) | (0x00<<3)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void ConfigDDPSPOFSBySoftware(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x10 ; bit: 17-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_SRC_CLK_SEL_0X10);

    switch (SampleRate)
    {
        case SAMPLE_RATE_8000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_192);

            break;
        }
        
        case SAMPLE_RATE_16000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_96);

            break;
        }
        
        case SAMPLE_RATE_32000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_32000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_32000);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_48);

            break;
        }
        
        case SAMPLE_RATE_64000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_64000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_64000);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_24);

            break;
        }
        
        case SAMPLE_RATE_128000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_128000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_128000);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_12);

            break;
        }
        
        case SAMPLE_RATE_256000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_256000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_256000);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_6);

            break;
        }
        
        case SAMPLE_RATE_512000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_512000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_512000);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_3);

            break;
        }
        
        case SAMPLE_RATE_11025:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_192);

            break;
        }
        
        case SAMPLE_RATE_22050:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_22050);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_22050);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_96);

            break;
        }
        
        case SAMPLE_RATE_44100:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_44100);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_44100);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_48);

            break;
        }
        
        case SAMPLE_RATE_88200:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_88200);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_88200);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_24);

            break;
        }
        
        case SAMPLE_RATE_176400:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_176400);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_176400);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_12);

            break;
        }
        
        case SAMPLE_RATE_352800:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_352800);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_352800);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_6);

            break;
        }
        
        case SAMPLE_RATE_705600:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_705600);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_705600);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_3);

            break;
        }
        
        case SAMPLE_RATE_12000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_192);

            break;
        }
        
        case SAMPLE_RATE_24000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_24000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_24000);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_96);

            break;
        }
        
        case SAMPLE_RATE_48000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_48000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_48000);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_48);

            break;
        }
        
        case SAMPLE_RATE_96000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_96000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_96000);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_24);

            break;
        }
        
        case SAMPLE_RATE_192000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_192000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_192000);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_12);

            break;
        }
        
        case SAMPLE_RATE_384000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_384000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_384000);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_6);

            break;
        }
        
        case SAMPLE_RATE_768000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_768000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_768000);

            SetDDPSPOMCLKDivBySoftware(DDPSPO_CLK_SW_DIV_3);

            break;
        }
        
        default:
        {
            break;
        }
    }
}



static void ConfigDDPSPOFSByHardware(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x14 ; bit: 29-24

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_FS_CFG1_0X14);

    switch (SampleRate)
    {
        case SAMPLE_RATE_8000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x00<<24)));

            break;
        }
        
        case SAMPLE_RATE_16000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x01<<24)));

            break;
        }
        
        case SAMPLE_RATE_32000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x02<<24)));
            
            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_32000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_32000);

            break;
        }
        
        case SAMPLE_RATE_64000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x03<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_64000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_64000);

            break;
        }
        
        case SAMPLE_RATE_128000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x04<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_128000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_128000);

            break;
        }
        
        case SAMPLE_RATE_256000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x05<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_256000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_256000);

            break;
        }
        
        case SAMPLE_RATE_512000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x06<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_512000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_512000);

            break;
        }
        
        case SAMPLE_RATE_11025:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x10<<24)));

            break;
        }
        
        case SAMPLE_RATE_22050:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x11<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_22050);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_22050);

            break;
        }
        
        case SAMPLE_RATE_44100:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x12<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_44100);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_44100);

            break;
        }
        
        case SAMPLE_RATE_88200:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x13<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_88200);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_88200);

            break;
        }
        
        case SAMPLE_RATE_176400:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x14<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_176400);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_176400);

            break;
        }
        
        case SAMPLE_RATE_352800:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x15<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_352800);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_352800);

            break;
        }
        
        case SAMPLE_RATE_705600:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x16<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_705600);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_705600);

            break;
        }
        
        case SAMPLE_RATE_12000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x20<<24)));

            break;
        }
        
        case SAMPLE_RATE_24000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x21<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_24000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_24000);

            break;
        }
        
        case SAMPLE_RATE_48000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x22<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_48000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_48000);

            break;
        }
        
        case SAMPLE_RATE_96000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x23<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_96000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_96000);

            break;
        }
        
        case SAMPLE_RATE_192000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x24<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_192000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_192000);

            break;
        }
        
        case SAMPLE_RATE_384000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x25<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_384000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_384000);

            break;
        }
        
        case SAMPLE_RATE_768000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG1_0X14, \
                              ((RegValue&(~(0x3F<<24))) | (0x26<<24)));

            SetDDPSPOLeftChanSampRate(SAMPLE_RATE_768000);
            SetDDPSPORightChanSampRate(SAMPLE_RATE_768000);

            break;
        }
        
        default:
        {
            break;
        }
    }
}



static void EnableDDPSPOInterface(AUD_BOOL Enable)
{
    // reg offset: 0x04 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<2))) | (0x01<<2)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<2))) | (0x00<<2)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static AUD_BOOL CheckDDPSPOInfEnableStatus(void)
{
    // reg offset: 0x04 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (RegValue&(0x01<<2))
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
    // reg offset: 0x04 ; bit: 10

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<10))) | (0x01<<10)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<10))) | (0x00<<10)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



/* DDP SPO DMA control ------------------------------------------------------ */

static void StartDDPSPODMA(AUD_BOOL Enable)
{
    // reg offset: 0x04 ; bit: 18

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<18))) | (0x01<<18)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<18))) | (0x00<<18)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



/* Get DDP SPO interface information ---------------------------------------- */

static AUD_UINT16 GetDDPSPODMABufLastIndex(void)
{
    // reg offset: 0x188 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_DMA_INDEX_0X188);

    return (AUD_UINT16)(RegValue&0xFFFF);
}



static AUD_UINT16 GetDDPSPODMABufCurrentIndex(void)
{
    // reg offset: 0x188 ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_DMA_INDEX_0X188);

    return (AUD_UINT16)((RegValue&(0xFFFF<<16))>>16);
}



static AUD_UINT16 GetDDPSPOSampCount(void)
{
    // reg offset: 0x19C ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_SMAPLE_COUNTER_0X19C);

    return (AUD_UINT16)((RegValue&(0xFFFF<<16))>>16);
}



static AUD_UINT8 GetDDPSPOFrameLenInfo(void)
{
    // reg offset: 0x1A0 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_FRAME_HEAD_INFO_0X1A0);

    return (AUD_UINT16)(RegValue&0xFFFF);
}



static AUD_UINT16 GetDDPSPOFramePDInfo(void)
{
    // reg offset: 0x1A0 ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_FRAME_HEAD_INFO_0X1A0);

    return (AUD_UINT16)((RegValue&(0xFFFF<<16))>>16);
}



static AUD_UINT16 GetDDPSPOFrameNullDataInfo(void)
{
    // reg offset: 0x1A4 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_FRAME_HEAD_INFT2_0X1A4);

    return (AUD_UINT16)(RegValue&0xFFFF);
}



static AUD_UINT16 GetDDPSPOFrameBurstDataInfo(void)
{
    // reg offset: 0x1A4 ; bit: 31-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_FRAME_HEAD_INFT2_0X1A4);

    return (AUD_UINT16)((RegValue&(0xFFFF<<16))>>16);
}



static AUD_UINT8 GetDDPSPOBufUnderRunStatus(void)
{
    // reg offset: 0x18C ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_DMA_UNDER_RUN_0X18C);

    if (RegValue&(0x01<<0))
    {
        AUD_WriteRegDev32(R_DDP_SPO_DMA_UNDER_RUN_0X18C, RegValue);

        return 1;
    }
    else
    {
        return 0;
    }
}



static AUD_UINT8 GetDDPSPOFIFOUnderRunStatus(void)
{
    // reg offset: 0x18C ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_DMA_UNDER_RUN_0X18C);

    if (RegValue&(0x01<<1))
    {
        AUD_WriteRegDev32(R_DDP_SPO_DMA_UNDER_RUN_0X18C, \
                          (RegValue&(~(0x01))));

        return 1;
    }
    else
    {
        return 0;
    }
}



static AUD_UINT16 GetDDPSPODMASkipNum(void)
{
    // reg offset: 0x1A8 ; bit: 15-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DDP_SPO_DMA_SKIP_NUM_0X1A8);

    return (AUD_UINT16)(RegValue&0xFFFF);
}



/* Enable DDP SPO interrupts ------------------------------------------------ */

static void EnableDDPSPOSampCountInt(AUD_BOOL Enable)
{
    // reg offset: 0x08 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST1_0X08);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST1_0X08, \
                          ((RegValue&(~(0x01<<2))) | (0x01<<2)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST1_0X08, \
                          ((RegValue&(~(0x01<<2))) | (0x00<<2)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableDDPSPODMABufResumeInt(AUD_BOOL Enable)
{
    // reg offset: 0x0C ; bit: 18

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          ((RegValue&(~(0x01<<18))) | (0x01<<18)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          ((RegValue&(~(0x01<<18))) | (0x00<<18)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableDDPSPODMAUnderRunInt(AUD_BOOL Enable)
{
    // reg offset: 0x0C ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          ((RegValue&(~(0x01<<2))) | (0x01<<2)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          ((RegValue&(~(0x01<<2))) | (0x00<<2)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



/* Get DDP SPO interrupt status --------------------------------------------- */

static AUD_BOOL CheckDDPSPOSampCountInt(void)
{
    // reg offset: 0x08 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST1_0X08);

    if (RegValue&(0x01<<2))
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
    // reg offset: 0x08 ; bit: 18

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST1_0X08);

    if (RegValue&(0x01<<18))
    {
        AUD_WriteRegDev32(R_AUD_INT_ST1_0X08, \
                          (RegValue|(0x01<<18)));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL CheckDDPSPODMABufResumeInt(void)
{
    // reg offset: 0x0C ; bit: 18

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (RegValue&(0x01<<18))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL CheckDDPSPODMABufResumeIntStatus(void)
{
    // reg offset: 0x0C ; bit: 26

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (RegValue&(0x01<<26))
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          (RegValue|(0x01<<26)));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL CheckDDPSPODMAUnderRunInt(void)
{
    // reg offset: 0x0C ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (RegValue&(0x01<<2))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



static AUD_BOOL CheckDDPSPODMABufUnderRunIntStatus(void)
{
    // reg offset: 0x0C ; bit: 10

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    if (RegValue&(0x01<<10))
    {
        AUD_WriteRegDev32(R_AUD_INT_ST2_0X0C, \
                          (RegValue&(0x01<<10)));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}




/* -----------------------------------------------------------------------------
I2SI Hardware Register Control Functions
----------------------------------------------------------------------------- */

static void ConfigI2SIFSByHardware(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x18 ; bit: 13-8

    AUD_UINT32 RegValue = 0;

    EnableI2SOInterfaceClock(TRUE);

    RegValue = AUD_ReadRegDev32(R_AUD_FS_CFG2_0X18);

    switch (SampleRate)
    {
        case SAMPLE_RATE_8000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x00<<8)));

            break;
        }
        
        case SAMPLE_RATE_16000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x01<<8)));

            break;
        }
        
        case SAMPLE_RATE_32000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x02<<8)));

            break;
        }
        
        case SAMPLE_RATE_64000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x03<<8)));

            break;
        }
        
        case SAMPLE_RATE_128000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x04<<8)));

            break;
        }
        
        case SAMPLE_RATE_11025:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x10<<8)));

            break;
        }
        
        case SAMPLE_RATE_22050:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x11<<8)));

            break;
        }
        
        case SAMPLE_RATE_44100:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x12<<8)));

            break;
        }
        
        case SAMPLE_RATE_88200:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x13<<8)));

            break;
        }
        
        case SAMPLE_RATE_176400:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x14<<8)));

            break;
        }
        
        case SAMPLE_RATE_12000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x20<<8)));

            break;
        }
        
        case SAMPLE_RATE_24000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x21<<8)));

            break;
        }
        
        case SAMPLE_RATE_48000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x22<<8)));

            break;
        }
        
        case SAMPLE_RATE_96000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x23<<8)));

            break;
        }
        
        case SAMPLE_RATE_192000:
        {
            AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                              ((RegValue&(~(0x3F<<8))) | (0x24<<8)));

            break;
        }

        default:
        {
            break;
        }
    }
}



static void EnableI2SITXInterface(AUD_BOOL Enable)
{
    // reg offset: 0x04 ; bit: 5

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<5))) | (0x01<<5)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<5))) | (0x00<<5)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



void EnableI2SIRXInterface(AUD_BOOL Enable)
{
    // reg offset: 0x04 ; bit: 4

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<4))) | (0x01<<4)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<4))) | (0x00<<4)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}


EXPORT_SYMBOL_GPL(EnableI2SIRXInterface);

static void StartI2SITXDMA(AUD_BOOL Enable)
{
    // reg offset: 0x04 ; bit: 21

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<21))) | (0x01<<21)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<21))) | (0x00<<21)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



void StartI2SIRXDMA(AUD_BOOL Enable)
{
    // reg offset: 0x04 ; bit: 20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<20))) | (0x01<<20)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<20))) | (0x00<<20)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}


EXPORT_SYMBOL_GPL(StartI2SIRXDMA);

/* -----------------------------------------------------------------------------
PCM Hardware Register Control Functions
----------------------------------------------------------------------------- */

static void ConfigPCMFSByHardware(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x18 ; bit: 0

    AUD_UINT32 RegValue = 0;

    EnableI2SOInterfaceClock(TRUE);

    RegValue = AUD_ReadRegDev32(R_AUD_FS_CFG2_0X18);
    
    if (SAMPLE_RATE_8000 == SampleRate)
    {
        AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                          ((RegValue&(~(0x01<<0))) | (0x00<<0)));
    }
    else if (SAMPLE_RATE_16000 == SampleRate)
    {
        AUD_WriteRegDev32(R_AUD_FS_CFG2_0X18, \
                          ((RegValue&(~(0x01<<0))) | (0x01<<0)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnablePCMTXInterface(AUD_BOOL Enable)
{
    // reg offset: 0x04 ; bit: 7

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<7))) | (0x01<<7)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<7))) | (0x00<<7)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnablePCMRXInterface(AUD_BOOL Enable)
{
    // reg offset: 0x04 ; bit: 6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<6))) | (0x01<<6)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<6))) | (0x00<<6)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void StartPCMTXDMA(AUD_BOOL Enable)
{
    // reg offset: 0x04 ; bit: 23

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<23))) | (0x01<<23)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<23))) | (0x00<<23)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void StartPCMRXDMA(AUD_BOOL Enable)
{
    // reg offset: 0x04 ; bit: 22

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<22))) | (0x01<<22)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_AUD_INTF_EN_0X04, \
                          ((RegValue&(~(0x01<<22))) | (0x00<<22)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



/* -----------------------------------------------------------------------------
DAC Hardware Register Control Functions
----------------------------------------------------------------------------- */

static void EnableDACFSSoftwareConfig(AUD_BOOL Enable)
{
    // reg offset: 0xD0 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_I2S_REG_0XD0);

    if (TRUE == Enable) // Use Software generate --> we config DAC by ourself
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<15))) | (0x01<<15)));        
    }
    else if (FALSE == Enable) // Use Software auto generate the DAC
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<15))) | (0x00<<15)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableDACFSGated(AUD_BOOL Enable)
{
    // reg offset: 0xD0 ; bit: 6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_I2S_REG_0XD0);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<6))) | (0x01<<6)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<6))) | (0x00<<6)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SetDACFSConfigDiv(DAC_CLK_SW_DivMode_t DivMode)
{
    // reg offset: 0xD0 ; bit: 13-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_I2S_REG_0XD0);

    switch (DivMode)
    {
        case DAC_CLK_SW_DIV_2:
        {
            AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                              ((RegValue&(~(0x3F<<8))) | ((2&0x3F)<<8)));
            break;
        }

        case DAC_CLK_SW_DIV_4:
        {
            AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                              ((RegValue&(~(0x3F<<8))) | ((4&0x3F)<<8)));
            break;
        }

        case DAC_CLK_SW_DIV_8:
        {
            AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                              ((RegValue&(~(0x3F<<8))) | ((8&0x3F)<<8)));
            break;
        }

        case DAC_CLK_SW_DIV_16:
        {
            AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                              ((RegValue&(~(0x3F<<8))) | ((16&0x3F)<<8)));
            break;
        }

        case DAC_CLK_SW_DIV_32:
        {
            AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                              ((RegValue&(~(0x3F<<8))) | ((32&0x3F)<<8)));
            break;
        }

        default:
        {
            break;
        }
    }
}



/* DAC I2S Register config */

static void SelectDACI2SSDATLen(DAC_I2S_SDAT_Len_t DataLen)
{
    // reg offset: 0xD0 ; bit: 31-30

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_I2S_REG_0XD0);

    if (SDAT_24_BITS == DataLen)
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x03<<30))) | (0x02<<30)));
    }
    else if (SDAT_16_BITS == DataLen)
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x03<<30))) | (0x00<<30)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectDACI2SWselLen(DAC_I2S_WSEL_Len_t WselLen)
{
    // reg offset: 0xD0 ; bit: 29-28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_I2S_REG_0XD0);

    if (I2S_WSEL_32_BITS == WselLen)
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x03<<28))) | (0x00<<28)));
    }
    else if (I2S_WSEL_48_BITS == WselLen)
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x03<<28))) | (0x02<<28)));
    }
    else if (I2S_WSEL_64_BITS == WselLen)
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x03<<28))) | (0x03<<28)));
    }    
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectDACI2SInterfaceMode(DAC_I2S_InterfaceMode_t Mode)
{
    // reg offset: 0xD0 ; bit: 27-26

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_I2S_REG_0XD0);

    AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                      ((RegValue&(~(0x03<<26))) | ((Mode&0x03)<<26)));
}



static void SelectDACI2SSDATSwapMode(DAC_I2S_SDAT_SwapMode_t SwapMode)
{
    // reg offset: 0xD0 ; bit: 17

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_I2S_REG_0XD0);

    if (SDAT_OUT_LSB_FIRST == SwapMode) // LSB first
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<17))) | (0x01<<17)));        
    }
    else if (SDAT_OUT_MSB_FIRST == SwapMode) // MSB first
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<17))) | (0x00<<17)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectDACI2SWselPolarInv(DAC_I2S_WSEL_PolarInv_t Polar)
{
    // reg offset: 0xD0 ; bit: 25

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_I2S_REG_0XD0);

    if (START_FROM_HIGH == Polar) // LRCLK start from high
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<25))) | (0x01<<25)));        
    }
    else if (START_FROM_LOW == Polar) // LRCLK start from low
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<25))) | (0x00<<25)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableDACI2SSDATInput(AUD_BOOL Enable)
{
    // reg offset: 0xD0 ; bit: 21

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_I2S_REG_0XD0);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<21))) | (0x01<<21)));        
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<21))) | (0x00<<21)));      
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectDACI2SChannelNum(DAC_I2S_ChanNum_t ChannelNum)
{
    // reg offset: 0xD0 ; bit: 24

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_I2S_REG_0XD0);

    if (DAC_I2S_TWO_CHAN == ChannelNum) // default
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<24))) | (0x01<<24)));        
    }
    else if (DAC_I2S_SINGLE_CHAN == ChannelNum)
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<24))) | (0x00<<24)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectDACI2SSource(DAC_I2S_Source_t Source)
{
    // reg offset: 0xD0 ; bit: 16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_I2S_REG_0XD0);

    if (I2S_FROM_EXTERNAL == Source) // I2S From external for test
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<16))) | (0x01<<16)));
    }
    else if (I2S_FROM_INTERNEL == Source) // I2S From internal (Default)
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<16))) | (0x00<<16)));
    }      
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableDACI2SDMA(AUD_BOOL Enable)
{
    // reg offset: 0xD0 ; bit: 20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_I2S_REG_0XD0);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<20))) | (0x01<<20)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<20))) | (0x00<<20)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectDACI2SOTriggerMode(DAC_I2S_TriggerMode_t Mode)
{
    // reg offset: 0xD0 ; bit: 18

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_I2S_REG_0XD0);

    if (I2S_SCLK_POSITIVE == Mode) // Trigger data at the positive edge of I2S_SCLK
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<18))) | (0x01<<18)));      
    }
    else if (I2S_SCLK_NEGATIVE == Mode) // Trigger data at the negative edge of I2S_SCLK
    {
        AUD_WriteRegDev32(R_DAC_I2S_REG_0XD0, \
                          ((RegValue&(~(0x01<<18))) | (0x00<<18)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



/* DAC Analog Register config */

static void EnableADACRightChanBS(AUD_BOOL Enable)
{
    // reg offset: 0xD4 ; bit: 7

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    if (TRUE == Enable) // Enable the right channel bit-stream data
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<7))) | (0x00<<7)));      
    }
    else if (FALSE == Enable) // Disable the right channel bit-stream data
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<7))) | (0x01<<7)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableADACLeftChanBS(AUD_BOOL Enable)
{
    // reg offset: 0xD4 ; bit: 6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    if (TRUE == Enable) // Enable the left channel bit-stream data
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<6))) | (0x00<<6)));      
    }
    else if (FALSE == Enable) // Disable the left channel bit-stream data
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<6))) | (0x01<<6)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableADACExchangeFunction(AUD_BOOL Enable)
{
    // reg offset: 0xD4 ; bit: 5

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    if (TRUE == Enable) // Exchange the left/right channel bit stream data
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<5))) | (0x01<<5)));      
    }
    else if (FALSE == Enable) // Normal
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<5))) | (0x00<<5)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableADACPDVolRefer(AUD_BOOL Enable)
{
    // reg offset: 0xD4 ; bit: 4

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    if (TRUE == Enable) // Switch on the internal reference voltage
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<4))) | (0x00<<4)));      
    }
    else if (FALSE == Enable) // Switch off the internal reference voltage
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<4))) | (0x01<<4)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableADACBiasCurrent(AUD_BOOL Enable)
{
    // reg offset: 0xD4 ; bit: 3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    if (TRUE == Enable) // self-bias current
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<3))) | (0x00<<3)));   
    }
    else if (FALSE == Enable) // select reference current
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<3))) | (0x01<<3)));   
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectADACOutputDCOffset(ADAC_OutputDC_Offset_t Offset)
{
    // reg offset: 0xD4 ; bit: 2-0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                      ((RegValue&(~(0x07<<0))) | ((Offset&0x07)<<0)));
}



static void SelectADACOutputMode(ADAC_OutputMode_t Mode)
{
    // reg offset: 0xD4 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    if (SINGLE_OUTPUT == Mode) // Single output mode
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<15))) | (0x01<<15)));      
    }
    else if (DIFFERENTIAL_OUTPUT == Mode) // differential output mode (S3921 donot support)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<15))) | (0x00<<15)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableADACPowerUp(AUD_BOOL Enable)
{
    // reg offset: 0xD4 ; bit: 14-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x7F<<8))) | (0x00<<8)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x7F<<8))) | (0x7F<<8)));        
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectADACRandomSource(ADAC_RandomSource_t Source)
{
    // reg offset: 0xD4 ; bit: 23

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    if (FROM_RDM_IN == Source) // From random in
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<23))) | (0x01<<23)));      
    }
    else if (FROM_SELF_HALF_CLK == Source) // From self half-clock
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<23))) | (0x00<<23)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableADACRandomSwitch(AUD_BOOL Enable)
{
    // reg offset: 0xD4 ; bit: 22

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<22))) | (0x01<<22)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<22))) | (0x00<<22)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectADACTestMode(AUD_UINT8 Mode)
{
    // reg offset: 0xD4 ; bit: 21-20

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                      ((RegValue&(~(0x03<<20))) | ((Mode&0x03)<<20)));
}



static void SetADACVolCtrlOutSwing(AUD_UINT8 Setting)
{
    // reg offset: 0xD4 ; bit: 19-18

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                      ((RegValue&(~(0x03<<18))) | ((Setting&0x03)<<18)));
}



static void SetADACCommonModeVolCtrlReg(AUD_UINT8 Setting)
{
    // reg offset: 0xD4 ; bit: 17-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                      ((RegValue&(~(0x03<<16))) | ((Setting&0x03)<<16)));
}



static void SelectADACBistFormat(ADAC_BistFormat_t Format)
{
    // reg offset: 0xD4 ; bit: 31-29

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                      ((RegValue&(~(0x07<<29))) | ((Format&0x07)<<29)));
}



static void EnableADACBist(AUD_BOOL Enable)
{
    // reg offset: 0xD4 ; bit: 28

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<28))) | (0x01<<28)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                          ((RegValue&(~(0x01<<28))) | (0x00<<28)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectADACAnalogClkSource(ADAC_Analog_ClkSource_t Source)
{
    return;
}



static void SetADACMuteMode(ADAC_MuteMode_t Mode)
{
    // reg offset: 0xD4 ; bit: 26-24

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG1_0XD4);

    AUD_WriteRegDev32(R_DAC_CODEC_REG1_0XD4, \
                      ((RegValue&(~(0x07<<24))) | ((Mode&0x07)<<24)));
}



/* DAC Digital Register config */

static void EnableDACDigtal(AUD_BOOL Enable)
{
    // reg offset: 0xD8 ; bit: 7

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG2_0XD8);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<7))) | (0x01<<7)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<7))) | (0x00<<7)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void ClearDACDigitalDSM(AUD_BOOL Clear)
{
    // reg offset: 0xD8 ; bit: 6

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG2_0XD8);

    if (TRUE == Clear)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<6))) | (0x01<<6)));      
    }
    else if (FALSE == Clear)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<6))) | (0x00<<6)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableDACDigtalMute(AUD_BOOL Enable)
{
    // reg offset: 0xD8 ; bit: 5

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG2_0XD8);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<5))) | (0x01<<5)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<5))) | (0x00<<5)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectDACDigitalDsmDataMode(DAC_Invert_DsmDataMode_t Mode)
{
    // reg offset: 0xD8 ; bit: 3

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG2_0XD8);

    if (DAC_ORG_DSM_DATA == Mode) // Select the original DSM data
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<3))) | (0x01<<3)));      
    }
    else if (DAC_INV_DSM_DATA == Mode) // Select the Inverted DSM data
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<3))) | (0x00<<3)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectDACDigitalChanMode(DAC_DIG_ChanMode_t Mode)
{
    // reg offset: 0xD8 ; bit: 2

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG2_0XD8);

    if (CODEC_DIG_MONO == Mode) // Mono Mode
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<2))) | (0x01<<2)));      
    }
    else if (CODEC_DIG_STEREO == Mode) // Stereo Mode
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<2))) | (0x00<<2)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SelectDACDigitalDsmClkMode(DAC_Invert_DsmClkMode_t Mode)
{
    // reg offset: 0xD8 ; bit: 1

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG2_0XD8);

    if (DAC_INV_DSM_CLK == Mode) // Select the Inverted DSM clock
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<1))) | (0x01<<1)));      
    }
    else if (DAC_ORG_DSM_CLK == Mode) // Select the original DSM clock
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<1))) | (0x00<<1)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void ClearDACDigitalBuf(AUD_BOOL Clear)
{
    // reg offset: 0xD8 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG2_0XD8);

    if (TRUE == Clear)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<0))) | (0x01<<0)));      
    }
    else if (FALSE == Clear)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<0))) | (0x01<<0)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableBSTransmitToADACMask(AUD_BOOL Enable)
{
    // reg offset: 0xD8 ; bit: 15

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG2_0XD8);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<15))) | (0x01<<15)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<15))) | (0x00<<15)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableClkTransmitToADACMask(AUD_BOOL Enable)
{
    // reg offset: 0xD8 ; bit: 14

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG2_0XD8);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<14))) | (0x01<<14)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<14))) | (0x00<<14)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void ClearDACDigtialDsmPara(AUD_BOOL Clear)
{
    // reg offset: 0xD8 ; bit: 12

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG2_0XD8);

    if (TRUE == Clear)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<12))) | (0x01<<12)));      
    }
    else if (FALSE == Clear)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<12))) | (0x00<<12)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableDACDIGDelayFunction(AUD_BOOL Enable)
{
    // reg offset: 0xD8 ; bit: 11

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG2_0XD8);

    if (TRUE == Enable) // Channel R delay one sample
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<11))) | (0x00<<11)));    
    }
    else if (FALSE == Enable) // no delay
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue&(~(0x01<<11))) | (0x01<<11)));  
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void EnableDACNewFilter(AUD_BOOL Enable)
{
    // reg offset: 0xD8 ; bit: 9

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_DAC_CODEC_REG2_0XD8);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue & (~(0x01<<9))) | (0x01<<9)));      
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_DAC_CODEC_REG2_0XD8, \
                          ((RegValue & (~(0x01<<9))) | (0x00<<9)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void ConfigDACFSBySoftware(AUD_SampleRate_t SampleRate)
{
    // reg offset: 0x10 ; bit: 17-16

    AUD_UINT32 RegValue = 0;

    EnableDACFSSoftwareConfig(1);

    RegValue = AUD_ReadRegDev32(R_AUD_SRC_CLK_SEL_0X10);

    switch (SampleRate)
    {
        case SAMPLE_RATE_8000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_32);

            break;
        }
        
        case SAMPLE_RATE_16000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_16);

            break;
        }
        
        case SAMPLE_RATE_32000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_64000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_4);

            break;
        }
        
        case SAMPLE_RATE_128000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x01<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_2);

            break;
        }
        
        case SAMPLE_RATE_11025:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_32);

            break;
        }
        
        case SAMPLE_RATE_22050:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_16);

            break;
        }
        
        case SAMPLE_RATE_44100:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_88200:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_4);

            break;
        }
        
        case SAMPLE_RATE_176400:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x01<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_2);

            break;
        }
        
        case SAMPLE_RATE_12000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_32);

            break;
        }
        
        case SAMPLE_RATE_24000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_16);

            break;
        }
        
        case SAMPLE_RATE_48000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_8);

            break;
        }
        
        case SAMPLE_RATE_96000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_4);

            break;
        }
        
        case SAMPLE_RATE_192000:
        {
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<16))) | (0x00<<16)));
            AUD_WriteRegDev32(R_AUD_SRC_CLK_SEL_0X10, \
                              ((RegValue&(~(0x01<<17))) | (0x00<<17)));

            SetDACFSConfigDiv(DAC_CLK_SW_DIV_2);

            break;
        }
        
        default:
        {
            break;
        }
    }
}



/* -----------------------------------------------------------------------------
STC Hardware Register Control Functions
----------------------------------------------------------------------------- */

static void EnableSTCPlay(AUD_UINT8 STCNum,
                   AUD_BOOL Enable)
{
    // reg offset: 0xB0 ; bit: 16 (STC0)
    // reg offset: 0xB0 ; bit: 17 (STC1)

    AUD_UINT32 RegValue = 0;
    AUD_UINT8 STCRegOffset = 0;

    if (1 == STCNum)
    {
        STCRegOffset = 16;
    }
    else if (2 == STCNum)
    {
        STCRegOffset = 17;
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }

    RegValue = AUD_ReadRegDev32(R_90K_F0_0XB0);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_90K_F0_0XB0, \
                          ((RegValue&(~(0x01<<STCRegOffset))) | (0x00<<STCRegOffset)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_90K_F0_0XB0, \
                          ((RegValue&(~(0x01<<STCRegOffset))) | (0x01<<STCRegOffset)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SetSTCDivisor(AUD_UINT8 STCNum,
                   AUD_UINT16 Divisor)
{
    // reg offset: 0xB0 ; bit: 15-0
    // reg offset: 0xB8 ; bit: 15-0

    AUD_UINT32 RegValue = 0;
    AUD_UINT32 STCRegAddr = 0;

    if (1 == STCNum)
    {
        STCRegAddr = R_90K_F0_0XB0;
    }
    else if (2 == STCNum)
    {
        STCRegAddr = R_90K_F1_0XB8;
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
    
    RegValue = AUD_ReadRegDev32(STCRegAddr);

    AUD_WriteRegDev32(STCRegAddr, \
                      ((RegValue&(~(0xFFFF<<0))) | ((Divisor&0xFFFF)<<0)));
}



static AUD_UINT16 GetSTCDivisor(AUD_UINT8 STCNum)
{
    // reg offset: 0xB0 ; bit: 15-0
    // reg offset: 0xB8 ; bit: 15-0

    AUD_UINT32 RegValue = 0;
    AUD_UINT32 STCRegAddr = 0;

    if (1 == STCNum)
    {
        STCRegAddr = R_90K_F0_0XB0;
    }
    else if (2 == STCNum)
    {
        STCRegAddr = R_90K_F1_0XB8;
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
    
    RegValue = AUD_ReadRegDev32(STCRegAddr);

    return (AUD_UINT16)((RegValue&(0xFFFF<<0))>>0);
}



static void SetSTCValue(AUD_UINT8 STCNum,
                 AUD_UINT32 Value)
{
    // reg offset: 0xB4 ; bit: 31-0
    // reg offset: 0xBC ; bit: 31-0

    AUD_UINT32 STCRegAddr = 0;

    if (1 == STCNum)
    {
        STCRegAddr = R_90K_F0_2_0XB4;
    }
    else if (2 == STCNum)
    {
        STCRegAddr = R_90K_F1_2_0XBC;
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
    
    AUD_WriteRegDev32(STCRegAddr, \
                      (Value & 0xFFFFFFFF));
}



static AUD_UINT32 GetSTCValue(AUD_UINT8 STCNum)
{
    // reg offset: 0xB4 ; bit: 31-0
    // reg offset: 0xBC ; bit: 31-0

    AUD_UINT32 STCRegAddr = 0;

    if (1 == STCNum)
    {
        STCRegAddr = R_90K_F0_2_0XB4;
    }
    else if (2 == STCNum)
    {
        STCRegAddr = R_90K_F1_2_0XBC;
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
    
    return AUD_ReadRegDev32(STCRegAddr);
}



/* -----------------------------------------------------------------------------
Volume Hardware Register Control Functions
----------------------------------------------------------------------------- */

static void SetTargetVolume(AUD_UINT8 Volume)
{
    // reg offset: 0x90 ; bit: 23-16

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_FADE_CTRL_0X90);

    AUD_WriteRegDev32(R_I2SO_FADE_CTRL_0X90, \
                     ((RegValue&(~(0xFF<<16))) | ((Volume&0xFF)<<16)));
}



static AUD_UINT8 GetCurrentVolume(void)
{
    // reg offset: 0x90 ; bit: 31-24

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_FADE_CTRL_0X90);

    return (AUD_UINT8)((RegValue&(0xFF<<24))>>24);
}



static void EnableFadeFunction(AUD_BOOL Enable)
{
    // reg offset: 0x90 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_FADE_CTRL_0X90);

    if (TRUE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_FADE_CTRL_0X90, \
                          ((RegValue&(~(0x01<<0))) | (0x00<<0)));
    }
    else if (FALSE == Enable)
    {
        AUD_WriteRegDev32(R_I2SO_FADE_CTRL_0X90, \
                          ((RegValue&(~(0x01<<0))) | (0x01<<0)));
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }
}



static void SetFadeSpeed(AUD_UINT8 FadeSpeed)
{
    // reg offset: 0x90 ; bit: 15-8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_I2SO_FADE_CTRL_0X90);

    AUD_WriteRegDev32(R_I2SO_FADE_CTRL_0X90, \
                      ((RegValue&(~(0xFF<<8))) | ((FadeSpeed&0xFF)<<8)));
}



/* -----------------------------------------------------------------------------
Audio Register layer function for HAL
----------------------------------------------------------------------------- */

static void OpenDAC(DAC_FORMAT_t Format,
			        AUD_UINT8 Precision)
{
    /* DAC I2S Register config */

    if (DAC_LEFT_JUSTIED == Format)
    {
        SelectDACI2SInterfaceMode(INTF_LEFT_JUSTIED);
        SelectDACI2SWselPolarInv(START_FROM_HIGH);
    }
    else if (DAC_RIGHT_JUSTIED == Format)
    {
        SelectDACI2SInterfaceMode(INTF_RIGHT_JUSTIED);
        SelectDACI2SWselPolarInv(START_FROM_HIGH);
    }
    else if (DAC_I2S == Format)
    {
        SelectDACI2SInterfaceMode(INTF_I2S);
        SelectDACI2SWselPolarInv(START_FROM_LOW);
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }

    if (Precision_16_BITS == Precision)
    {
        SelectDACI2SWselLen(I2S_WSEL_32_BITS);
        SelectDACI2SSDATLen(SDAT_16_BITS);
    }
    else if (Precision_24_BITS == Precision)
    {
        SelectDACI2SWselLen(I2S_WSEL_48_BITS);
        SelectDACI2SSDATLen(SDAT_24_BITS);
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }    

    /* DAC Analog Register config */
    
    SelectADACOutputMode(SINGLE_OUTPUT);
    EnableADACRightChanBS(TRUE);
    EnableADACLeftChanBS(TRUE);
    EnableADACPowerUp(TRUE);
    SetADACMuteMode(ATTENUATION_N_39_7DB);


    /* DAC Digital Register config */
    
    EnableBSTransmitToADACMask(FALSE);
    EnableClkTransmitToADACMask(FALSE);
    SelectDACDigitalChanMode(CODEC_DIG_STEREO);

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
    if (NULL == Params_p)
    {
        printk( "Input parameters are wrong in %s, %d !\n",__FUNCTION__,__LINE__);
        return;
    }

    if (TRUE == Params_p->SoftwareCfgClkFlag)
    {
        ConfigI2SOFSBySoftware(Params_p->SampleRate);
    }
    else
    {
         /* Enable I2SO clk before config */
        EnableI2SOInterfaceClock(TRUE);
        
        ConfigI2SOFSByHardware(Params_p->SampleRate);
    }

    SetI2SOInterfaceFormat(Params_p->Format);
    SetI2SOInterfaceBCLK2LRCLK(BICK_LRCK_48);

    SetI2SODMADataLeftAlign(TRUE);
    
    if (Precision_16_BITS == Params_p->Precision)
    {
        SetI2SODMADataBitNum(BIT_NUM_16);
    }
    else // Precision_24_BITS
    {
        SetI2SODMADataBitNum(BIT_NUM_24);
    }
    
    SetI2SODMADataChannelNum(Params_p->ChannelNum);

    if (TRUE == Params_p->DMADataHeaderFlag)
    {
        SetI2SODMADataWithHeader(TRUE);
    }
    else
    {
        SetI2SODMADataWithHeader(FALSE);
    }

    SetI2SOSampCountThreshold(Params_p->SampleNum);
    
    /* Make sure every call last index = 0 */
    SetI2SODMABufLastIndex(0);

    /* Enable I2SO interrupts */
    EnableI2SOTimingCheckInt(TRUE);
    EnableI2SOSampCountInt(TRUE);
    EnableI2SODMABufResumeInt(TRUE);
    EnableI2SODMAUnderRunInt(TRUE);

    /* Enalbe I2SO interface */
    EnableI2SOInterface(TRUE);
    EnableI2SOClock(TRUE);

    return;
}



static void ConfigSPOInterface(AUD_OutputParams_t *Params_p)
{
    AUD_UINT32 StrType = 0;
    AUD_SampleRate_t SPOSampleRate = 0;
    AUD_BOOL SPOIntEnableFlag = FALSE;
    
    if (NULL == Params_p)
    {
        printk( "Input parameters are wrong in %s, %d !\n",__FUNCTION__,__LINE__);
        return;
    }

    StrType = GET_BIT(Params_p->StreamType);

    if ((SRC_BUF == Params_p->SPOChlPCMType) && \
        (AUDIO_EC3 == Params_p->StreamType))
    {
        SPOSampleRate = Params_p->SampleRate * 4;
    }
    else
    {
        SPOSampleRate = Params_p->SampleRate;
    }

    if (TRUE == Params_p->SoftwareCfgClkFlag)
    {
        ConfigSPOFSBySoftware(SPOSampleRate);
        ConfigF128FSBySoftware(SPOSampleRate);
    }
    else
    {
        ConfigSPOFSByHardware(SPOSampleRate);
        SelectF128LRCLKDivMode(SPO_LRCLK_DIV_1);
    }

    EnableSPODMADataWithHeader(TRUE);
    SelectSPOChannelStatusReg(LEFT_CHAN_STATUS_REG);

    if ((SRC_DMLR == Params_p->SPOChlPCMType) || \
        ((SRC_BUF == Params_p->SPOChlPCMType) && \
         ((StrType&(GET_BIT(AUDIO_OGG)|GET_BIT(AUDIO_MP3)|GET_BIT(AUDIO_WMA))) || \
          (!(StrType&Params_p->BitStreamSetByUser)))))
    {
        SPOIntEnableFlag = FALSE;
        if (TRUE == Params_p->SPODMAForPCMOutFlag) // SPO output pcm by SPO DMA 
        {
            SelectSPODataSource(FROM_I2SO_DMA);
            EnableSPOBitStreamMode(FALSE);
            SetSPODMADataLen(DATA_LEN_24_BITS);
            SelectSPOPCMOutLFEMode(SPO_R_I2SO_LFE);
            SelectSPOPCMOutChan(I2SO_DL_DR);
            EnableSPOValidBit(FALSE);
        }
        else // SPO output pcm by I2S DMA 
        {
            SelectSPODataSource(FROM_I2SO_DMA);
            EnableSPOBitStreamMode(FALSE);
            SelectSPOPCMOutLFEMode(SPO_R_I2SO_LFE);
            SelectSPOPCMOutChan(I2SO_DL_DR);
            EnableSPOValidBit(FALSE);
        }

        SetSPOLeftChanAudContent(LINEAR_PCM); // Linear PCM
        SetSPORightChanAudContent(LINEAR_PCM); // Linear PCM
    }
    else if (SRC_BUF == Params_p->SPOChlPCMType) // SPO output bit stream by SPO DMA 
    {
        SPOIntEnableFlag = TRUE;
        SelectSPODataSource(FROM_SPO_DMA);
        EnableSPOBitStreamMode(TRUE);
        SetSPODMADataLen(DATA_LEN_16_BITS);
        EnableSPODataReorder(FALSE);
        SelectSPOPCMOutLFEMode(SPO_R_I2SO_LFE);
        EnableSPOValidBit(TRUE);

        SetSPOLeftChanAudContent(NON_PCM); // BS
        SetSPORightChanAudContent(NON_PCM); // BS
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }

    EnableSPONormalPlay(TRUE);

    if ((SRC_BUF == Params_p->SPOChlPCMType) && \
        (AUDIO_EC3 == Params_p->StreamType))
    {
        SetSPOSampCountThreshold(Params_p->SampleNum*4);
    }
    else
    {
        SetSPOSampCountThreshold(Params_p->SampleNum);
    }


    /* Make sure every call last index = 0 */
    SetSPODMABufLastIndex(0);

    /* Enable SPO interrupts */
    if (TRUE == SPOIntEnableFlag)
    {
        EnableSPOSampCountInt(TRUE);
        EnableSPODMAUnderRunInt(TRUE);
        EnableSPODMABufResumeInt(TRUE);                
    }

    /* Enable SPO interface */
    EnableSPOInterface(TRUE);
    EnableSPOClock(TRUE);

    return;
}



static void ConfigDDPSPOInterface(AUD_OutputParams_t *Params_p)
{
    AUD_UINT32 StrType = 0;
    AUD_SampleRate_t DDPSPOSampleRate = 0;
    AUD_BOOL DDPSPOIntEnableFlag = FALSE;

    if (NULL == Params_p)
    {
        printk( "Input parameters are wrong in %s, %d !\n",__FUNCTION__,__LINE__);
        return;
    }

    StrType = GET_BIT(Params_p->StreamType);

    if ((SRC_BUF == Params_p->DDPSPOChlPCMType) && \
        (AUDIO_EC3 == Params_p->StreamType))
    {
        DDPSPOSampleRate = Params_p->SampleRate * 4;
    }
    else
    {
        DDPSPOSampleRate = Params_p->SampleRate;
    }

    if (1 == Params_p->SoftwareCfgClkFlag)
    {
        ConfigDDPSPOFSBySoftware(DDPSPOSampleRate);
    }
    else
    {
        ConfigDDPSPOFSByHardware(DDPSPOSampleRate);
    }

    EnableDDPSPODMADataWithHeader(TRUE);
    SelectDDPSPOChannelStatusReg(DDP_LEFT_CHAN_STATUS_REG);

    if ((SRC_DMLR == Params_p->DDPSPOChlPCMType) || \
        ((SRC_BUF == Params_p->DDPSPOChlPCMType) && \
         ((StrType&(GET_BIT(AUDIO_OGG)|GET_BIT(AUDIO_MP3)|GET_BIT(AUDIO_WMA))) || \
          (!(StrType&Params_p->BitStreamSetByUser)))))
    {
        DDPSPOIntEnableFlag = FALSE;
        if (1 == Params_p->DDPSPODMAForPCMOutFlag) // DDP SPO output pcm by DDP SPO DMA 
        {
            SelectDDPSPODataSource(DDP_FROM_DDPSPO_DMA);
            EnableDDPSPOBitStreamMode(FALSE);
            SetDDPSPODMADataLen(DDP_DATA_LEN_24_BITS);
            SelectDDPSPOPCMOutLFEMode(DDPSPO_R_I2SO_LFE);
            SelectDDPSPOPCMOutChan(DDP_I2SO_DL_DR);
            EnableDDPSPOValidBit(FALSE);
        }
        else // DDP SPO output pcm by I2S DMA 
        {
            SelectDDPSPOClkAndDataMode(SPO_OUT_CLK_DATA);
            SelectDDPSPODataSource(DDP_FROM_I2SO_DMA);
            EnableDDPSPOBitStreamMode(FALSE);
            SelectDDPSPOPCMOutLFEMode(DDPSPO_R_I2SO_LFE);
            SelectDDPSPOPCMOutChan(DDP_I2SO_DL_DR);
            EnableDDPSPOValidBit(FALSE);
        }

        SetDDPSPOLeftChanAudContent(DDP_LINEAR_PCM); // Linear PCM
        SetDDPSPORightChanAudContent(DDP_LINEAR_PCM); // Linear PCM
    }
    else if (SRC_BUF == Params_p->DDPSPOChlPCMType) // DDP SPO output bit stream by DDP SPO DMA 
    {
        if ((AUDIO_EC3 == Params_p->StreamType) && \
            (Params_p->SampleRate != SAMPLE_RATE_32000))
        {
            DDPSPOIntEnableFlag = TRUE;
            SelectDDPSPOClkAndDataMode(DDPSPO_OUT_CLK_DATA);
            SelectDDPSPODataSource(DDP_FROM_DDPSPO_DMA);
            EnableDDPSPOBitStreamMode(TRUE);
            SetDDPSPODMADataLen(DDP_DATA_LEN_16_BITS);
            EnableDDPSPODataReorder(FALSE);
            SelectDDPSPOPCMOutLFEMode(DDPSPO_L_I2SO_LFE);
            EnableDDPSPOValidBit(TRUE);

            SetDDPSPOLeftChanAudContent(DDP_NON_PCM); // BS
            SetDDPSPORightChanAudContent(DDP_NON_PCM); // BS
        }
        else
        {
            DDPSPOIntEnableFlag = TRUE;
            SelectDDPSPOClkAndDataMode(SPO_OUT_CLK_DATA);
            EnableDDPSPOValidBit(TRUE);
        }
    }
    else
    {
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }

    EnableDDPSPONormalPlay(TRUE);

    if ((SRC_BUF == Params_p->DDPSPOChlPCMType) && \
        ((AUDIO_AC3 == Params_p->StreamType) || \
        (AUDIO_EC3 == Params_p->StreamType)))
    {
        SetDDPSPOSampCountThreshold(Params_p->SampleNum*4);
    }
    else
    {
        SetDDPSPOSampCountThreshold(Params_p->SampleNum);
    }

    /* Make sure every call last index = 0 */
    SetDDPSPODMABufLastIndex(0); 

    /* Enable DDP SPO interrupts */
    if (TRUE == DDPSPOIntEnableFlag)
    {
        EnableDDPSPOSampCountInt(TRUE);
        EnableDDPSPODMAUnderRunInt(TRUE);
        EnableDDPSPODMABufResumeInt(TRUE);
    }
    
    /* Enable DDP SPO interface */            
    EnableDDPSPOInterface(TRUE);
    EnableDDPSPOClock(TRUE);

    return;
}



static void AUDREG_HardwareStart(void)
{
    /* Start the DAC */
    StartDAC();

    /* Start DMA */
    StartI2SODMA(TRUE);
    StartSPODMA(TRUE);
    StartDDPSPODMA(TRUE);

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
                printk( "Error, please check  %s, %d!\n",__FUNCTION__,__LINE__);
    }

    return;
}



void AUDREG_ConfigInterface(AUD_SubBlock_t SubBlockIdx,
                                   AUD_OutputParams_t *Params_p)
{
    if (NULL == Params_p)
    {
        printk( "Input parameters are wrong in %s, %d !\n",__FUNCTION__,__LINE__);
        return;
    }

    DisableAudClockGate();
    
    switch (SubBlockIdx)
    {
        case AUD_SUB_OUT:
        {
            /* Config I2SO interface */
            ConfigI2SOInterface(Params_p);
            break;
        }

        case AUD_SUB_SPDIFOUT:
        {
            /* Config SPO interface */
            ConfigSPOInterface(Params_p);
            break;
        }

        case AUD_SUB_SPDIFOUT_DDP:
        {
            /* Config DDP SPO interface */
            ConfigDDPSPOInterface(Params_p);
            break;
        }

        default:
        {
            break;
        }
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
//txj add, tony suggested that we should use full address
#if 1
    switch (SubBlockIdx)
    {
        case AUD_SUB_OUT:
        {
            /* Config I2SO DMA */
            SetI2SODMABufBaseAddr(DMABase&0x0FFFFFFF);
            SetI2SODMABufLength(DMALen);
            
            break;
        }

        case AUD_SUB_SPDIFOUT:
        {
            /* Config SPO DMA */
            SetSPODMABufBaseAddr(DMABase&0x0FFFFFFF);
            SetSPODMABufLength(DMALen);
        }

        case AUD_SUB_SPDIFOUT_DDP:
        {
            /* Config DDP SPO DMA */
            SetDDPSPODMABufBaseAddr(DMABase&0x0FFFFFFF);
            SetDDPSPODMABufLength(DMALen);
        }

        default:
        {
            break;
        }
    }
  #else
      switch (SubBlockIdx)
    {
        case AUD_SUB_OUT:
        {
            /* Config I2SO DMA */
            SetI2SODMABufBaseAddr(DMABase&0xFFFFFFFF);
            SetI2SODMABufLength(DMALen);
            
            break;
        }

        case AUD_SUB_SPDIFOUT:
        {
            /* Config SPO DMA */
            SetSPODMABufBaseAddr(DMABase&0xFFFFFFFF);
            SetSPODMABufLength(DMALen);
        }

        case AUD_SUB_SPDIFOUT_DDP:
        {
            /* Config DDP SPO DMA */
            SetDDPSPODMABufBaseAddr(DMABase&0xFFFFFFFF);
            SetDDPSPODMABufLength(DMALen);
        }

        default:
        {
            break;
        }
    }
  #endif

    return;
}

EXPORT_SYMBOL_GPL(AUDREG_ConfigDMA);

void AUDREG_SetDMALastIndex(AUD_SubBlock_t SubBlockIdx,
                                   AUD_UINT16 Index)
{
    switch (SubBlockIdx)
    {
        case AUD_SUB_OUT:
        {
            SetI2SODMABufLastIndex(Index);
            break;
        }

        case AUD_SUB_SPDIFOUT:
        {
            SetSPODMABufLastIndex(Index);
            break;
        }

        case AUD_SUB_SPDIFOUT_DDP:
        {
            SetDDPSPODMABufLastIndex(Index);
            break;
        }

        default:
        {
            break;
        }
    }

    return;
}

EXPORT_SYMBOL_GPL(AUDREG_SetDMALastIndex);

AUD_UINT16 AUDREG_GetDMALastIndex(AUD_SubBlock_t SubBlockIdx)
{
    AUD_UINT16 Index = 0;

    switch (SubBlockIdx)
    {
        case AUD_SUB_OUT:
        {
            Index = GetI2SODMABufLastIndex();
            break;
        }

        case AUD_SUB_SPDIFOUT:
        {
            Index = GetSPODMABufLastIndex();
            break;
        }

        case AUD_SUB_SPDIFOUT_DDP:
        {
            Index = GetDDPSPODMABufLastIndex();
            break;
        }

        default:
        {
            break;
        }
    }

    return Index;
}

EXPORT_SYMBOL_GPL(AUDREG_GetDMALastIndex);

AUD_UINT16 AUDREG_GetDMACurrentIndex(AUD_SubBlock_t SubBlockIdx)
{
    AUD_UINT16 Index = 0;

    switch (SubBlockIdx)
    {
        case AUD_SUB_OUT:
        {
            Index = GetI2SODMABufCurrentIndex();
            break;
        }

        case AUD_SUB_SPDIFOUT:
        {
            Index = GetSPODMABufCurrentIndex();
            break;
        }

        case AUD_SUB_SPDIFOUT_DDP:
        {
            Index = GetDDPSPODMABufCurrentIndex();
            break;
        }

        default:
        {
            break;
        }
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
    AUD_UINT8 retValue=GetCurrentVolume();
    printk(KERN_ERR "in Function %s, line %d, the volume is %d \n", __FUNCTION__,__LINE__,retValue );
    return retValue;
    
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

void AUDREG_ClearAllAudioInterrupt(void)
{
    // reg offset: 0x08 ; bit: 18

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST1_0X08);

   AUD_WriteRegDev32(R_AUD_INT_ST1_0X08, \
                    (RegValue|(0xFF<<16)));

}

EXPORT_SYMBOL_GPL(AUDREG_ClearAllAudioInterrupt);

void AUDREG_DisableAllAudioInterrupt(void)
{
    // reg offset: 0x08 ; bit: 18

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST1_0X08);

   AUD_WriteRegDev32(R_AUD_INT_ST1_0X08, \
                    (RegValue&(~0x000000FF)));

}
EXPORT_SYMBOL_GPL(AUDREG_DisableAllAudioInterrupt);

AUD_UINT32 GetDMABufUnderRunIntStatus(void)
{
    // reg offset: 0x0C ; bit: 8

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INT_ST2_0X0C);

    return RegValue;

}

EXPORT_SYMBOL_GPL(GetDMABufUnderRunIntStatus);

AUD_UINT32 GetInterfaceEnableStatus(void)
{
    // reg offset: 0x04 ; bit: 0

    AUD_UINT32 RegValue = 0;

    RegValue = AUD_ReadRegDev32(R_AUD_INTF_EN_0X04);
    
    return RegValue;

}
EXPORT_SYMBOL_GPL(GetInterfaceEnableStatus);

void AudioPinmuxConfigurate(void)
{
    AUD_UINT32 tmp = 0;
    tmp = AUD_ReadRegDev32(R_SYS_DEVICE_CLK_GATING_CTRL0_0X60);
    if(tmp&(1<<10))
    {
        tmp &= (~(1<<10));
        AUD_WriteRegDev32(R_SYS_DEVICE_CLK_GATING_CTRL0_0X60,tmp);
    }

    // check AUD_CVBS_CLK_GATE
    tmp = AUD_ReadRegDev32(R_SYS_DEVICE_CLK_GATING_CTRL1_0X64);
    if(tmp&(1<<31))
    {
        tmp &= (~(1<<31));
        AUD_WriteRegDev32(R_SYS_DEVICE_CLK_GATING_CTRL1_0X64,tmp);
    }

    // check AUD_CORE_CLK_GATE
    tmp = AUD_ReadRegDev32(R_SYS_DEVICE_CLK_GATING_CTRL1_0X64);
    if(tmp&(1<<30))
    {
        tmp &= (~(1<<30));
        AUD_WriteRegDev32(R_SYS_DEVICE_CLK_GATING_CTRL1_0X64,tmp);
    }

    // enable Local Device Clock Gating Control Register 3
    AUD_WriteRegDev32(R_SYS_DEVICE_CLK_GATING_CTRL3_0X90, 0x00000000);

    // set pinmux to enable spdif
    AUD_WriteRegDev32(R_SYS_DEVICE_SPDIF_CTRL_0X88, AUD_ReadRegDev32(R_SYS_DEVICE_SPDIF_CTRL_0X88) | ((0x01<<25) | (0x01<<23) | (0x01<<13)));
    //SET_DWORD(0xB800008C, 0x00000000);
}
EXPORT_SYMBOL_GPL(AudioPinmuxConfigurate);

void PrintAudioConfiguratation(void)
{
    AUD_UINT32 tmp = 0;
    unsigned long buff[8];
    
    tmp = AUDREG_GetDMACurrentIndex(AUD_SUB_OUT);
    printk(KERN_ERR "DMACurrentIndex = %d",tmp);
    tmp = AUDREG_GetDMALastIndex(AUD_SUB_OUT);
    printk(KERN_ERR "  DMALastIndex = %d \n",tmp);    

    tmp = AUD_ReadRegDev32(R_I2SO_SAMPLE_COUNTER_0X5C);
    tmp=tmp&(0xFFFF);
    printk(KERN_ERR "  SampleCounterThres = %d ",tmp);   
    
    tmp =AUD_ReadRegDev32(R_I2SO_DMA_CFG_0X34);;
    tmp=tmp&(0xFFFF);
    printk(KERN_ERR "  DMAlength = %d ",tmp);   

    tmp =AUD_ReadRegDev32(R_I2SO_DMA_CFG_0X34);;
    tmp=tmp&(~(0x01<<24));
    if(tmp==0)
       printk(KERN_ERR "  DMA with header \n ");     
    else
       printk(KERN_ERR "  DMA without header \n ");    

    tmp = AUD_ReadRegDev32(R_I2SO_FADE_CTRL_0X90);
    tmp= (AUD_UINT8)((tmp&(0xFF<<24))>>24);
    printk(KERN_ERR "  Cur volume %d  ", tmp);    

    tmp = AUD_ReadRegDev32(R_I2SO_FADE_CTRL_0X90);
    tmp= (AUD_UINT8)((tmp &(0xFF<<16)) >>16);
    printk(KERN_ERR "  Target volume %d \n ", tmp);   

    tmp = AUD_ReadRegDev32(R_I2SO_DMA_BASE_ADDR_0X30)|0xa0000000;
    printk(KERN_ERR "  DMA address 0x%x  ", tmp);    

    copy_to_user(buff, tmp, 4);
    printk(KERN_ERR "  DMA value is 0x%x \n ", buff[0]);    
    
}
EXPORT_SYMBOL_GPL(PrintAudioConfiguratation);

void AudioConfiguratationPatch(void)
{
    AUD_UINT32 tmp = 0;
    unsigned int offset;
#if 0    
    offset=0x1f0;
    tmp=0x00000fa0;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);

    offset=0x19c;
    tmp=0x01480240;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);

    offset=0x190;
    tmp=0x01000000;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);    

    offset=0x18c;
    tmp=0x0000ff01;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  

    offset=0x184;
    tmp=0x00002010;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
    
    offset=0x180;
    tmp=0x04695000;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
    
    offset=0x170;
    tmp=0x00000fa0;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  

    offset=0x17c;
    tmp=0x00000003;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp); 
    
    offset=0x178;
    tmp=0;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  

 offset=0x110;
    tmp=0x01df0240;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  

   offset=0x10c;
    tmp=0x0000ff10;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  

   offset=0x108;
    tmp=0;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  

   offset=0x104;
    tmp=0x00002010;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
#endif     
#if 0
   offset=0x100;
    tmp=0x04674000;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
#endif
#if 1
   offset=0xd0;
    tmp=0xa9300088;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
#endif
#if 1
   offset=0xd4;
    tmp=0x68308001;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
#endif
#if 1
   offset=0xd8;
    tmp=0x00000a80;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
#endif
#if 0
   offset=0xbc;
    tmp=0x00011dcc;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
#endif
#if 0
   offset=0xb8;
    tmp=0x0034bb;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  

   offset=0xb4;
    tmp=0x00011dcb;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  

   offset=0xb0;
    tmp=0x800034bb;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
 

   offset=0x90;
    tmp=0x00552000;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  

   offset=0x60;
    tmp=0x00000e40;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
#endif
#if 0
   offset=0x5c;
    tmp=0x01a10240;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
   offset=0x4c;
    tmp=0x005a0000;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  

   offset=0x40;
    tmp=0xa2;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
   offset=0x3c;
    tmp=0x03d8ff51;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  

   offset=0x38;
    tmp=0x03d803d8;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
   offset=0x34;
    tmp=0x02212010;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  

   offset=0x14;
    tmp=0x12120012;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
   offset=0x0c;
    tmp=0x00010101;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  

   offset=0x08;
    tmp=0x80070001;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  

   offset=0x04;
    tmp=0x00070007;
    AUD_WriteRegDev32(S3921_SND_BASE_ADDR+offset, tmp);  
#endif
}
EXPORT_SYMBOL_GPL(AudioConfiguratationPatch);

