/*****************************************************************************

File Name :  AUD_C3701_REG.h

Description: header file for AUD_C3701_REG

******************************************************************************/

/* Prevent recursive inclusion */
#ifndef __AUD_C3701_REG_H
#define __AUD_C3701_REG_H

/***********************************************************/
/************ C3701 Chip Registers definition *************/
/***********************************************************/
#define SYS_BASE_ADDR           0x18000000


#define R_SYS_INT_POLARITY_0X28              0x28 // 32 bits
#define R_SYS_INT_STATUS_0X30                0x30 // 32 bits
#define R_SYS_INT_ENABLE_0X38                0x38 // 32 bits
#define R_SYS_DEVICE_CLK_GATING_CTRL_0X60    0x60 // 32 bits
#define R_SYS_DEVICE_RESET_CTRL_0X80         0x80 // 32 bits
#define R_SYS_PINMUX_CTRL1_0X8C              0x8C // 32 bits
#define R_SYS_PINMUX_CTRL2_0XA8              0xA8 // 32 bits
#define R_SYS_CTRL_0XB0                      0xB0 // 32 bits



/***********************************************************/
/************ C3701 Audio Registers definition *************/
/***********************************************************/
#define AUDIO_BASE_ADDRESS               0x18002000

#define R_AUD_CTRL_0X00                      0x00 // 08 bits
#define R_AUD_INT_0X01                       0x01 // 08 bits
#define R_I2SO_CLK_INTF_CTRL_0X02            0x02 // 08 bits
#define R_I2SO_AUD_FRAME_INFO_0X03           0x03 // 08 bits
#define R_I2SO_AUD_PTS_0X04                  0x04 // 32 bits
#define R_I2SO_AUD_FRAME_COUNTER_0X08        0x08 // 16 bits
#define R_I2SO_AUD_FRAME_LEN_0X0A            0x0A // 16 bits
#define R_I2SO_LAST_VALID_INDEX_0X0C         0x0C // 16 bits
#define R_I2SO_TIME_CHECK_THD_0X0E           0x0E // 16 bits
#define R_I2SO_SAMP_COUNTER_THD_0X10         0x10 // 16 bits
#define R_I2SO_SAMP_COUNTER_0X12             0x12 // 16 bits
#define R_BUF_UNDER_RUN_CTRL_0X14            0x14 // 08 bits
#define R_DMA_CTRL_0X15                      0x15 // 08 bits
#define R_DEBUG_CTRL_0X16                    0x16 // 08 bits
#define R_I2SO_LATENCY_COUNTER_0X17          0x17 // 08 bits
#define R_I2SO_DMA_BASE_ADDR_0X18            0x18 // 32 bits
#define R_I2SO_DMA_CUR_INDEX_0X1C            0x1C // 16 bits
#define R_I2SO_DMA_LEN_0X1E                  0x1E // 16 bits


#define R_SPO_CTRL_0X20                      0x20 // 08 bits
#define R_SPO_IEC_CTRL_0X21                  0x21 // 08 bits 
#define R_SPO_SAMP_COUNTER_TRHD_0X22         0x22 // 16 bits
#define R_SPO_CS_0X24                        0x24 // 32 bits
#define R_SPO_LEFT_USER_DATA_0X28            0x28 // 192 bits
#define R_SPO_RIGHT_USER_DATA_0X40           0x40 // 192 bits
#define R_SPO_IEC_PC_0X58                    0x58 // 16 bits
#define R_SPO_IEC_PD_0X5A                    0x5A // 16 bits
#define R_SPO_IEC_NULL_0X5C                  0x5C // 16 bits
#define R_SPO_IEC_DATA_0X5E                  0x5E // 16 bits
#define R_SPO_DMA_BASE_ADDR_0X60             0x60 // 32 bits
#define R_SPO_DMA_CUR_INDEX_0X64             0x64 // 16 bits
#define R_SPO_DMA_LEN_0X66                   0x66 // 16 bits
#define R_SPO_DMA_LAST_INDEX_0X68            0x68 // 16 bits


#define R_EQ_CTRL_0X6A                       0x6A // 16 bits
#define R_FADE_CTRL_0X6C                     0X6C // 08 bits
#define R_FADE_SPEED_0X6D                    0x6D // 08 bits
#define R_TARGET_VOLUME_0X6E                 0x6E // 08 bits
#define R_CUR_VOLUME_0X6F                    0x6F // 08 bits

#define R_EQ_IIR1_COEFF_0_31_0X8C            0x8C // 32 bits
#define R_EQ_IIR1_COEFF_32_63_0X90           0x90 // 32 bits
#define R_EQ_IIR1_COEFF_64_79_0X94           0x94 // 16 bits
#define R_EQ_IIR2_COEFF_0_15_0X96            0x96 // 16 bits
#define R_EQ_IIR2_COEFF_16_47_0X98           0x98 // 32 bits
#define R_EQ_IIR2_COEFF_48_79_0X9C           0x9C // 32 bits
#define R_EQ_IIR3_COEFF_0_31_0XA0            0xA0 // 32 bits
#define R_EQ_IIR3_COEFF_32_63_0XA4           0xA4 // 32 bits
#define R_EQ_IIR3_COEFF_64_79_0XA8           0xA8 // 16 bits
#define R_EQ_IIR4_COEFF_0_15_0XAA            0xAA // 16 bits
#define R_EQ_IIR4_COEFF_16_47_0XAC           0xAC // 32 bits
#define R_EQ_IIR4_COEFF_48_79_0XB0           0xB0 // 32 bits

#define R_COUNTER_F0_FRQ_0XE4                0xE4 // 16 bits
#define R_COUNTER_F0_CTRL_0XE6               0xE6 // 08 bits
#define R_COUNTER_F0_33_0_0XE7               0xE7 // 08 bits
#define R_COUNTER_F0_1_32_0XE8               0xE8 // 32 bits
#define R_COUNTER_F1_FRQ_0XEC                0xEC // 16 bits
#define R_COUNTER_F1_CTRL_0XEE               0xEE // 08 bits
#define R_COUNTER_F1_33_0_0XEF               0xEF // 08 bits
#define R_COUNTER_F1_1_32_0XF0               0xF0 // 32 bits

#define R_DAC_I2S_REG_0_7_0XF4               0xF4 // 08 bits
#define R_DAC_I2S_REG_8_15_0XF5              0xF5 // 08 bits
#define R_DAC_I2S_REG_16_23_0XF6             0xF6 // 08 bits
#define R_DAC_I2S_REG_24_31_0XF7             0xF7 // 08 bits
#define R_DAC_CODEC_REG_0_7_0XF8             0xF8 // 08 bits
#define R_DAC_CODEC_REG_8_15_0XF9            0xF9 // 08 bits
#define R_DAC_CODEC_REG_16_23_0XFA           0xFA // 08 bits
#define R_DAC_CODEC_REG_24_31_0XFB           0xFB // 08 bits
#define R_DAC_CODEC_REG_32_39_0XFC           0xFC // 08 bits
#define R_DAC_CODEC_REG_40_47_0XFD           0xFD // 08 bits
#define R_DAC_CODEC_REG_48_63_0XFE           0xFE // 16 bits
#define R_DAC_CODEC_REG_64_71_0X100          0x100 // 08 bits
#define R_DAC_CODEC_REG_72_79_0X101          0x101 // 08 bits
#define R_DAC_CODEC_REG_80_87_0X102          0x102 // 08 bits
#define R_DAC_CODEC_REG_88_95_0X103          0x103 // 08 bits

#define R_I2SI_INTF_CTRL_0_7_0X108           0x108 // 08 bits
#define R_I2SI_INTF_CTRL_8_15_0X109          0x109 // 08 bits
#define R_I2SI_INTF_CTRL_16_23_0X10A         0X10A // 08 bits
#define R_I2SI_INTF_CTRL_24_31_0X10B         0X10B // 08 bits
#define R_I2SI_VOLUME_CTRL_0X10C             0x10C // 16 bits
#define R_I2SI_START_CTRL_0X10E              0x10E // 08 bits
#define R_I2SI_INT_CTRL_0X10F                0x10F // 08 bits

#define R_I2SI_DMA_BASE_ADDR_0X110           0x110 // 32 bits
#define R_I2SI_RX_DMA_CUR_INDEX_0X114        0x114 // 16 bits
#define R_I2SI_TX_DMA_CUR_INDEX_0X116        0x116 // 16 bits
#define R_I2SI_DMA_BASE_LEN_0X118            0x118 // 16 bits
#define R_I2SI_TX_LAST_VALID_INDEX_0X11A     0x11A // 16 bits
#define R_I2SI_RX_SAMP_COUNTER_TRHD_0X11C    0x11C // 16 bits
#define R_I2SI_RX_SAMP_COUNTER_0X11E         0x11E // 16 bits

#define R_DDP_SPO_LEFT_USER_DATA_0X120       0x120 // 192 bits
#define R_DDP_SPO_RIGHT_USER_DATA_0X138      0x138 // 192 bits
#define R_DDP_SPO_CS_0X150                   0x150 // 32 bits
#define R_DDP_SPO_DMA_BASE_ADDR_0X154        0x154 // 32 bits
#define R_DDP_SPO_DMA_LEN_0X158              0x158 // 16 bits
#define R_DDP_SPO_DMA_CUR_INDEX_0X15A        0x15A // 16 bits
#define R_DDP_SPO_SAMP_COUNTER_TRHD_0X15C    0x15C // 16 bits
#define R_DDP_SPO_LAST_INDEX_0X15E           0x15E // 16 bits
#define R_DDP_SPO_INT_0X160                  0x160 // 08 bits
#define R_DDP_SPO_IEC_CTRL_0X161             0x161 // 08 bits
#define R_DDP_SPO_CTRL_0X162                 0x162 // 16 bits
#define R_DDP_SPO_IEC_NULL_0X164             0x164 // 16 bits
#define R_DDP_SPO_IEC_DATA_0X166             0x166 // 16 bits
#define R_DDP_SPO_IEC_PC_0X168               0x168 // 16 bits
#define R_DDP_SPO_IEC_PD_0X16A               0x16A // 16 bits

#define R_I2SI_TX_SAMP_COUNTER_TRHD_0X16C    0x16C // 16 bits
#define R_I2SI_TX_SAMP_COUNTER_0X16E         0x16E // 16 bits
#define R_ALSA_DATA_FMT_0X178                0x178 // 16 bits
#define R_UNDER_RUN_IMPROVE_0X17A            0x17A // 16 bits

#define R_CHIP_ID_0X400                      0x400  // 32 bits

#define AUD_ReadReg32(i)           (*(volatile UINT32 *)(AUDIO_BASE_ADDRESS+i))
#define AUD_WriteReg32(i,d)        (*(volatile UINT32 *)(AUDIO_BASE_ADDRESS+i)) = (d)

#define AUD_ReadReg16(i)             (*(volatile UINT16 *)(AUDIO_BASE_ADDRESS+i))
#define AUD_WriteReg16(i,d)          (*(volatile UINT16 *)(AUDIO_BASE_ADDRESS+i)) = (d)

#define AUD_ReadReg8(i)             (*(volatile UINT8 *)(AUDIO_BASE_ADDRESS+i))
#define AUD_WriteReg8(i,d)          (*(volatile UINT8 *)(AUDIO_BASE_ADDRESS+i)) = (d)



/* Define control Macro --------------------------------------------- */
#define SYS_ReadReg8(Address_p) (*((SYS_UINT8 *) (SYS_BASE_ADDR+Address_p)))


#define SYS_WriteReg8(Address_p, Value)                                 \
{                                                                      \
    *((SYS_UINT8 *) (SYS_BASE_ADDR+Address_p)) = (AUD_UINT8) (Value);                \
}


#define SYS_ReadReg16(Address_p) (*((SYS_UINT16 *) (SYS_BASE_ADDR+Address_p)))


#define SYS_WriteReg16(Address_p, Value)                                \
{                                                                      \
    *((SYS_UINT16 *) (SYS_BASE_ADDR+Address_p)) = (AUD_UINT16) (Value);              \
}


#define SYS_ReadReg32(Address_p) (*((SYS_UINT32 *) (SYS_BASE_ADDR+Address_p)))


#define SYS_WriteReg32(Address_p, Value)                                \
{                                                                      \
    *((SYS_UINT32 *) (SYS_BASE_ADDR+Address_p)) = (AUD_UINT32) (Value);              \
}

/* Define GPIO mute Macro --------------------------------------------- */
#define HAL_GPIO_I_DIR		0
#define HAL_GPIO_O_DIR		1

#endif /* __AUD_C3701_REG_H */

/* End of AUD_c3701_reg.h */
