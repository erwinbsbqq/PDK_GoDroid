/*
 * ALi M36 ALSA ASoC audio support.
 *
 * (c) 2007-2008 ALi Tech. Corp.
 *	Xavier Chiang <xavier.chiang@alitech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * NOTE: all of these drivers can only work with a SINGLE instance
 *	 of a PSC. Multiple independent audio devices are impossible
 *	 with ASoC v1.
 */

#ifndef _ALI_PCM_H
#define _ALI_PCM_H

#define	ALI_M36_AUDIO_BASE		0xB8002000 /* Memory base for ALI_M3602_AUDIO */
#define	ALI_M36_IO_BASE			0xB8000000

enum SND_M36_REGISTER_ADDRESS{
	R_AUD_CTRL				= 0x00,		// Audio control register 
	R_AUD_INT				= 0x01,		// Audio interrupt control&status register 
	R_I2SO_CLK_INTF_CTRL	= 0x02,		// I2S out interface clock select&interface control 
	R_AUD_FRAME_INFO		= 0x03,		// PTS[0] & audio frame info
	R_AUD_PTS				= 0x04,		// PTS[32:1]
	R_AUD_FRAME_COUNT	= 0x08,		// audio frame counter (unit: 64 bits)
	R_AUD_FRAME_LEN		= 0x0a,		// audio frame length (unit: 64 bits)
	R_LAST_VALID_INDEX	= 0x0c,		// last valid sample index (unit: 64 bits)
	R_TIME_CHK_THRESHOLD	= 0x0e,		// timing check threshold
	R_I2SO_SAM_COUNT_THD	= 0x10,		// i2so sample counter threshold
	R_I2SO_SAM_COUNT		= 0x12,		// i2so out sample counter
	R_BUF_UNDRUN_CTRL	= 0x14,		// buffer under run control
	R_DMA_CTRL				= 0x15,		// dma control
	R_DEBUG_CTRL			= 0x16,		// reserved
	R_I2SO_DMA_BASE_ADDR= 0x18,		// i2so dma base address
	R_I2SO_DMA_CUR_INDEX	= 0x1c,		// dma index (unit: 64 bits)
	R_I2SO_DMA_LENG		= 0x1e,		// dma length (unit: 64 bits)
	R_SPO_CTRL				= 0x20,		// s/pdif out interface control
	R_IEC_CTRL				= 0x21,		// s/pdif IEC61937 control 
	R_SPO_SAM_COUNT_THD	= 0x22,		// s/pdif sample counter threshold
	R_SPO_CS				= 0x24,		// s/pdif channel status register
	R_SPO_LEFT_USR_DATA_BASE	= 0x28,		// s/pdif left user data
	R_SPO_RIGHT_USR_DATA_BASE	= 0x40,		//s/pdif right user data
	R_IEC_PC				= 0x58,		// pc(bust info)
	R_IEC_PD				= 0x5a,		// pa(length code)
	R_IEC_NULL				= 0x5c,		// null data frames
	R_IEC_DATA				= 0x5e,		// data frames
	R_SPO_DMA_BASE_ADDR	= 0x60,		// s/pdif dma base address
	R_SPO_DMA_CUR_INDEX	= 0x64,		// s/pdif out dma index(unit:64 bits)
	R_SPO_DMA_LENG		= 0x66,		// dma length
	R_BASS_CTRL			= 0x68,		// post-process & bass control coefficient
	R_BASS_71_CTRL		= 0x6c,		// bass 7.1 channel control coefficient
	R_FADE_SPEED			= 0x6d,		// fade speed
	R_TARGET_VOLUME		= 0x6e,		// target volume
	R_CUR_VOLUME			= 0x6f,		// current volume
	R_VS_IIR1_COEFF_0_31	= 0x70,		// A1,A0 coefficient	
	R_VS_IIR1_COEFF_32_63	= 0x74,		// B1, A2 
	R_VS_IIR1_COEFF_64_79	= 0x78,		// B2 
	R_VS_IIR2_COEFF_0_15	= 0x7a,		// A0 
	R_VS_IIR2_COEFF_16_47	= 0x7c,		// A2,A1
	R_VS_IIR2_COEFF_48_79	= 0x80,		// B2,B1	
	R_VS_GAIN_COEFF_0_31	= 0x84,		// B0,A0
	R_VS_GAIN_COEFF_32_63= 0x88,		// D0,C0
	R_EQ_IIR1_COEFF_0_31	= 0x8c,		// A1,A0
	R_EQ_IIR1_COEFF_32_63	= 0x90,		// B1,A2
	R_EQ_IIR1_COEFF_64_79	= 0x94,		//B2
	R_EQ_IIR2_COEFF_0_15	= 0x96,		//A0
	R_EQ_IIR2_COEFF_16_47	= 0x98,		//A2,A1
	R_EQ_IIR2_COEFF_48_79	= 0x9c,		// B2,B1	
	R_EQ_IIR3_COEFF_0_31	= 0xa0,		// A1,A0
	R_EQ_IIR3_COEFF_32_63	= 0xa4,		// B1,A2
	R_EQ_IIR3_COEFF_64_79	= 0xa8,		// B2
	R_EQ_IIR4_COEFF_0_15	= 0xaa,		// A0
	R_EQ_IIR4_COEFF_16_47	= 0xac,		// A2,A1
	R_EQ_IIR4_COEFF_48_79	= 0xb0,		// B2,B1	
	R_BASS_IIR1_COEFF_0_31= 0xb4,		// A1,A0
	R_BASS_IIR1_COEFF_32_63= 0xb8,	// B1, A2
	R_BASS_IIR1_COEFF_64_79= 0xbc,	// B2
	R_BASS_IIR2_COEFF_0_15= 0xbe,		// A0
	R_BASS_IIR2_COEFF_16_47= 0xc0,	// A2,A1
	R_BASS_IIR2_COEFF_48_79= 0xc4,	// B2,B1
	R_BASS_IIR3_COEFF_0_31= 0xc8,		// A1,A0
	R_BASS_IIR3_COEFF_32_63= 0xcc,	// B1, A2
	R_BASS_IIR3_COEFF_64_79= 0xd0,	// B2
	R_BASS_IIR4_COEFF_0_15= 0xd2,		// A0
	R_BASS_IIR4_COEFF_16_47= 0xd4,	// A2,A1
	R_BASS_IIR4_COEFF_48_79= 0xd8,	// B2,B1
	R_BASS_GAIN_0			= 0xdc,		// LFE gain 0
	R_BASS_GAIN_1			= 0xde,		// LFE gain 1
	R_BASS_GAIN_2			= 0xe0,		// LFE gain 2
	R_BASS_TRIM			= 0xe2,		// subwoofer output trim control
	R_COUNT_F0_SET		= 0xe4,		// F0 frequency configure
	R_COUNT_F0_CTRL		= 0xe6,		// F0 counter control
	R_COUNT_F0_0			= 0xe7,		// F0[0]
	R_COUNT_F0_1_32		= 0xe8,		// F0[1:32]
	R_COUNT_F1_SET		= 0xec,		// F1 frequency configure
	R_COUNT_F1_CTRL		= 0xee,		// F1 counter control	
	R_COUNT_F1_0			= 0xef,		// F1[0]
	R_COUNT_F1_1_32		= 0xf0,		// F1[1:32]
	R_RESERVED_1			= 0xf4,
	R_RESERVED_2			= 0xf8,
	R_RESERVED_3			= 0xfc,

	R_DISIR_0_7             = 0xf4,        /* DAC I2S interface register*/
	R_DISIR_8_15            = 0xf5,
	R_DISIR_16_23           = 0xf6,
	R_DISIR_24_32           = 0xf7,

	/* s3602f add */
	R_DACR_0_7              = 0xf8,     /* DAC analog control register*/
	R_DACR_8_15             = 0xf9,
	R_DACR_16_23            = 0xfa,
	R_DDCR_0_7              = 0xfb,     /* DAC digital control register*/
	R_DDCR_8_15             = 0xfc,
	R_DDCR_16_23            = 0xfd,
	R_DDCR_24_31            = 0xfe,
	R_DDDR_0_7              = 0xff,     /* DAC digital data register*/
	R_DDDR_8_15             = 0x100,
	R_DDDR_16_23            = 0x101,
	R_DDDR_24_31            = 0x102,
	R_DGLR                  = 0x103,    /* DAC gain left*/
	R_DOLR                  = 0x104,    /* DAC offset left */
	R_DGRR                  = 0x105,    /* DAC gain right */
	R_DORR                  = 0x106,    /* DAC offset right */
	R_DDIR                  = 0x107,     /* DAC digital interrupt */

	R_I2SI_INTF_CTRL_0      = 0x108,
	R_I2SI_INTF_CTRL_1      = 0x109,
	R_I2SI_INTF_CTRL_3      = 0X10b,
	R_I2SI_VOLUME_CTRL      = 0x10c,
	R_I2SI_START_CTRL       = 0x10e,
	R_I2SI_INT_CTRL         = 0x10f,

	R_DDP_SPO_CTRL          = 0x162,
};

#define AUDIO_RATE_DEFAULT	44100

#define	ALI_M36_IOR_BYTE(A)				(*((volatile u8*)(ALI_M36_IO_BASE+A)))
#define   ALI_M36_IOW_BYTE(A,D)				((*((volatile u8*)(ALI_M36_IO_BASE+A)))=D)
#define	ALI_M36_IOR_DW(A)				(*((volatile u32*)(ALI_M36_IO_BASE+A)))
#define   ALI_M36_IOW_DW(A,D)				((*((volatile u32*)(ALI_M36_IO_BASE+A)))=D)

#define SND_GET_DWORD(i)           (*(volatile u32 *)((ALI_M36_AUDIO_BASE)+i))
#define SND_SET_DWORD(i,d)        (*(volatile u32 *)((ALI_M36_AUDIO_BASE)+i)) = (d)
#define SND_GET_WORD(i)             (*(volatile u16 *)((ALI_M36_AUDIO_BASE)+i))
#define SND_SET_WORD(i,d)          (*(volatile u16 *)((ALI_M36_AUDIO_BASE)+i)) = (d)
#define SND_GET_BYTE(i)             (*(volatile u8*)((ALI_M36_AUDIO_BASE)+i))
#define SND_SET_BYTE(i,d)          (*(volatile u8 *)((ALI_M36_AUDIO_BASE)+i)) = (d)

#define PCM_BUF_FRM_NUM		10		// 10
#define MAX_PCM_FRAME_LEN		6148	// 6148 = (1536*4+4) = 8 channel ac3 audio frame size
#define PCM_BUFF_SIZE			(MAX_PCM_FRAME_LEN*PCM_BUF_FRM_NUM)	// unit: 64 bits, 8*64 bits align
#define RAW_DATA_BUFF_SIZE          (MAX_PCM_FRAME_LEN*PCM_BUF_FRM_NUM)	// unit: 64 bits
#define M36_PERIOD_MIN_BYTES	9216	// 9216 = 1152*8 for Gstreamer MP3	//9224 = (1152+1)*8

#define M36_PCM_INTF	0
#define M36_SPDIF_INTF	1

#define SND_MUTE_POLAR			0

#endif
