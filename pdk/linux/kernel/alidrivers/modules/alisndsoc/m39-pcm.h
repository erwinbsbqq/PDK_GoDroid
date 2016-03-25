/*
 * ALi M39 ALSA ASoC audio support.
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

#define	SYS_IO_BASE				0xB8000000
#define	AUDIO_IO_BASE				0xB8002000	/* Audio IO Base for M39 */

enum SND_M39_REGISTER_ADDRESS{
	R_AUD_CTRL				= 0x00,		// Audio control register 
	R_AUD_INT				= 0x01,		// Audio interrupt control&status register 
	R_I2SO_CLK_INTF_CTRL	= 0x02,		// I2S out interface clock select&interface control 
	R_AUD_FRAME_INFO		= 0x03,		// PTS[0] & audio frame info
	R_AUD_PTS				= 0x04,		// PTS[32:1]
	R_AUD_FRAME_COUNT	= 0x08,		// audio frame counter (unit: 64 bits)
	R_AUD_FRAME_LEN		= 0x0a,		// audio frame length (unit: 128 bits)
	R_LAST_VALID_INDEX	= 0x0c,		// last valid sample index (unit: 128 bits)
	R_TIME_CHK_THRESHOLD	= 0x0e,		// timing check threshold
	R_I2SO_SAM_COUNT_THD	= 0x10,		// i2so sample counter threshold (unit: samples)
	R_I2SO_SAM_COUNT		= 0x12,		// i2so out sample counter (unit: samples)
	R_BUF_UNDRUN_CTRL	= 0x14,		// buffer under run control
	R_DMA_CTRL				= 0x15,		// dma control
	R_DEBUG_CTRL			= 0x16,		// reserved
	R_I2SO_DMA_BASE_ADDR= 0x18,		// i2so dma base address
	R_I2SO_DMA_CUR_INDEX	= 0x1c,		// dma index (unit: 128 bits)
	R_I2SO_DMA_LENG		= 0x1e,		// dma length (unit: 128 bits)
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
	R_SPO_DMA_BASE_ADDR	= 0x60,		// s/pdif dma base address (256 bits align)
	R_SPO_DMA_CUR_INDEX	= 0x64,		// s/pdif out dma index(unit:128 bits)
	R_SPO_DMA_LENG		= 0x66,		// dma length

	/* differ from M36 */
	R_SPO_DMA_LAST_INDEX	= 0x68,		// s/pdif last valid sample indexdiffer (unit:128 bits)
	R_BASS_CTRL			= 0x6a,		// post-process & bass control coefficient

	R_BASS_71_CTRL		= 0x6c,		// bass 7.1 channel control coefficient
	R_FADE_SPEED			= 0x6d,		// fade speed
	R_TARGET_VOLUME		= 0x6e,		// target volume
	R_CUR_VOLUME			= 0x6f,		// current volume

#if 0 /* Remove in M39 */
	R_VS_IIR1_COEFF_0_31	= 0x70,		// A1,A0 coefficient	
	R_VS_IIR1_COEFF_32_63	= 0x74,		// B1, A2 
	R_VS_IIR1_COEFF_64_79	= 0x78,		// B2 
	R_VS_IIR2_COEFF_0_15	= 0x7a,		// A0 
	R_VS_IIR2_COEFF_16_47	= 0x7c,		// A2,A1
	R_VS_IIR2_COEFF_48_79	= 0x80,		// B2,B1	
	R_VS_GAIN_COEFF_0_31	= 0x84,		// B0,A0
	R_VS_GAIN_COEFF_32_63= 0x88,		// D0,C0
#endif

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

#if 0 /* Remove in M39 */
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
#endif

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

	R_DISIR_0_7			= 0xf4,		/* DAC I2S interface register*/
	R_DISIR_8_15			= 0xf5,
	R_DISIR_16_23			= 0xf6,
	R_DISIR_24_31			= 0xf7,

	/* New in M39 */
	R_DACR_0_7				= 0xf8,		/* DAC analog control register*/
	R_DACR_8_15			= 0xf9,
	R_DACR_16_23			= 0xfa,
	R_DDCR_24_31			= 0xfb, 
	R_DDCR_32_39           	= 0xfc,		/* DAC digital control register*/
	R_DDCR_40_47             	= 0xfd,
	R_DDCR_48_55            	= 0xfe,
	R_DDCR_56_63            	= 0xff,
	R_DDCR_64_71            	= 0x100,
	R_DDCR_72_79            	= 0x101,
	R_DDCR_80_87            	= 0x102,
	R_DDCR_88_95           	= 0x103,
	
#if 0
	/* s3602f add */
	R_DACR_0_7				= 0xf8,		/* DAC analog control register*/
	R_DACR_8_15			= 0xf9,
	R_DACR_16_23			= 0xfa,
	R_DDCR_0_7				= 0xfb,		/* DAC digital control register*/
	R_DDCR_8_15			= 0xfc,
	R_DDCR_16_23			= 0xfd,
	R_DDCR_24_31			= 0xfe,
	R_DDDR_0_7				= 0xff,		/* DAC digital data register*/
	R_DDDR_8_15			= 0x100,
	R_DDDR_16_23			= 0x101,
	R_DDDR_24_31			= 0x102,
	R_DGLR					= 0x103,		/* DAC gain left*/
	R_DOLR					= 0x104, 	/* DAC offset left */
	R_DGRR					= 0x105,		/* DAC gain right */
	R_DORR					= 0x106,		/* DAC offset right */
	R_DDIR					= 0x107,		/* DAC digital interrupt */
	R_DDP_SPO_CTRL			= 0x162,
#endif

    R_I2SI_INTF_CTRL_0      = 0x108,  // I2S in interface control register
    R_I2SI_INTF_CTRL_1      = 0x109,
    R_I2SI_INTF_CTRL_2      = 0x10a,
    R_I2SI_INTF_CTRL_3      = 0X10b,
    R_I2SI_VOLUME_CTRL      = 0x10c,  // I2S in volume control
    R_I2SI_START_CTRL       = 0x10e,  // I2S in start control
    R_I2SI_INT_CTRL         = 0x10f,  // I2S in interrupt control

    R_I2SI_DMA_BASE_ADDR    = 0x110,  // I2S in dma base address
    R_I2SI_RX_DMA_CUR_INDEX = 0x114,  // I2S in rx dma current index
    R_I2SI_TX_DMA_CUR_INDEX = 0x116,  // I2S in tx dma current index
    R_I2SI_DMA_LENG         = 0x118,  // I2S in dma length
    R_I2SI_TX_LAST_VALID_INDEX         = 0x11a, //I2S in tx last valid index
    R_I2SI_RX_SAMPLE_COUNTER_THRESHOLD = 0x11c,	//I2S in rx sample counter threshold
    R_I2SI_RX_SAMPLE_COUNTER           = 0x11e,	//I2S in rx sample counter
    R_I2SI_TX_SAMPLE_COUNTER_THRESHOLD = 0x16c,	//I2S in tx sample counter threshold
    R_I2SI_TX_SAMPLE_COUNTER           = 0x16e, //I2S in tx sample counter

    R_ADC_ALG_CTRL          = 0x170,  // ADC anaog conrol
    R_ADC_DIG_CTRL_0        = 0x172,  // ADC digital control
    R_ADC_DIG_CTRL_1        = 0x174,
    R_ADC_TEST_CTRL         = 0x176,  // ADC test control

    R_CONFIG_DATA_FMT       = 0x178,  // funtion id
    R_CONFIG_DATA_FMT_CTRL  = 0x179,  // version id 
    R_VERSION_ID            = 0x400,
};

#define AUDIO_RATE_DEFAULT	44100

#define	SYS_IOR_BYTE(A)		(*((volatile u8*)(SYS_IO_BASE+A)))
#define	SYS_IOW_BYTE(A,D)		((*((volatile u8*)(SYS_IO_BASE+A)))=D)
#define	SYS_IOR_DW(A)			(*((volatile u32*)(SYS_IO_BASE+A)))
#define	SYS_IOW_DW(A,D)		((*((volatile u32*)(SYS_IO_BASE+A)))=D)

#define SND_GET_DWORD(i)		(*(volatile u32 *)((AUDIO_IO_BASE)+i))
#define SND_SET_DWORD(i,d)		(*(volatile u32 *)((AUDIO_IO_BASE)+i)) = (d)
#define SND_GET_WORD(i)			(*(volatile u16 *)((AUDIO_IO_BASE)+i))
#define SND_SET_WORD(i,d)		(*(volatile u16 *)((AUDIO_IO_BASE)+i)) = (d)
#define SND_GET_BYTE(i)			(*(volatile u8*)((AUDIO_IO_BASE)+i))
#define SND_SET_BYTE(i,d)		(*(volatile u8 *)((AUDIO_IO_BASE)+i)) = (d)

#define PCM_BUF_FRM_NUM		    10
#define MAX_PCM_FRAME_LEN		1536	/* 2 channel audio frame size */
#define PCM_BUFF_SIZE			(MAX_PCM_FRAME_LEN * PCM_BUF_FRM_NUM)
#define M39_PERIOD_MIN_BYTES	12288	/* 12288 = 1536*8 */ 

#define SPDIF_BUF_FRM_NUM	        8    //alex
#define MAX_SPDIF_FRAME_LEN		   (1536 + 8)	/* 2 channel audio frame size */ //alex
#define SPDIF_BUFF_SIZE			(MAX_SPDIF_FRAME_LEN * SPDIF_BUF_FRM_NUM)  //alex
#define	M39_SPDIF_PERIOD_MIN_BYTES (MAX_SPDIF_FRAME_LEN * 4) //alex

#define M39_PCM_INTF	0
#define M39_SPDIF_INTF	1
#define M39_PCM_IN      2

#define SND_MUTE_POLAR			0
#endif
