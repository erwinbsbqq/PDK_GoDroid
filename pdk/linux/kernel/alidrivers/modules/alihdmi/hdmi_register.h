/***************************************************************************************************
*    Ali Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:
*		hdmi_register.h
*
*    Description:
*		This file contains all read/write functions in register level with HDMI
*
*    History:
*	 	Date            Author        	Version     Reason
*	 	============	=============	=========	=================
*		2010/08/01		Lucas Lai       1.0
***************************************************************************************************/

#ifndef	_HDMI_REGISTER_H_
#define	_HDMI_REGISTER_H_

//#define CONFIG_ALI_DIS_HTPLG
#ifdef CONFIG_ALI_CHIP_M3921
#include "ali_reg.h"
#endif

#include "hdmi_proc.h"
#include "hdmi_infoframe.h"
#include "hdmi_edid.h"

/***************************************************************************************************
 * Register Base Address
***************************************************************************************************/
#if defined(CONFIG_ARM)
/* System Base Address */
#define M36_SYS_REG_BASEADDR		0x18000000
/* Base I/O address with hdmi */
#define HDMI_REG_BASEADDR			0x1802A000
/* South Bridge Address */
#define M39_SYS_SB_IOBASE			0x18018D00 //SCB4 interface
#else
/* System Base Address */
#define M36_SYS_REG_BASEADDR		0xB8000000
/* Base I/O address with hdmi */
#define HDMI_REG_BASEADDR			0xB802A000
/* South Bridge Address */
#define M39_SYS_SB_IOBASE			0xB8018D00 //SCB4 interface
#endif
/***************************************************************************************************
 *  System Register Address
***************************************************************************************************/
/* ARM CPU architecture */
#if defined(CONFIG_ARM)
#define M36_SYS_REG_CLK_CTRL			(__REG32ALI(M36_SYS_REG_BASEADDR + 0x60))
#define M36_SYS_REG_HDMI_PWR_MODE_CTRL		(__REG32ALI(M36_SYS_REG_BASEADDR + 0x6C))
#define M36_SYS_REG_SYS_CTRL					(__REG32ALI(M36_SYS_REG_BASEADDR + 0x78))
#define M36_SYS_REG_RST_CTRL					(__REG32ALI(M36_SYS_REG_BASEADDR + 0x80))
#define M36_SYS_REG_PLL_SETTING					(__REG32ALI(M36_SYS_REG_BASEADDR + 0xC0))
#define M36_SYS_REG_FUNC_VER_ID		(__REG32ALI(M36_SYS_REG_BASEADDR + 0x2400))
#define M39_SYS_REG_SB_HOST			(__REG32ALI(M39_SYS_SB_IOBASE + 0x00))
#define M39_SYS_REG_SB_SLAVE			(__REG32ALI(M39_SYS_SB_IOBASE + 0x04))
#define M39_SYS_REG_SB_TIME			(__REG32ALI(M39_SYS_SB_IOBASE + 0x08))
#define M39_SYS_REG_SB_FIFO_CTL			(__REG8ALI(M39_SYS_SB_IOBASE + 0x0C))
#define M39_SYS_REG_SB_FIFO_DATA		(__REG8ALI(M39_SYS_SB_IOBASE + 0x10))
#define M39_SYS_REG_SB_SLAVE_SCB		(__REG8ALI(M39_SYS_SB_IOBASE + 0x2C))
#define M39_SYS_REG_SB_SLAVE_SEL		(__REG8ALI(M39_SYS_SB_IOBASE + 0x2D))
#define M39_SYS_REG_HDMI_3DPHY		(__REG8ALI(M36_SYS_REG_BASEADDR + 0x6E))
#else
#define M36_SYS_REG_CLK_CTRL	                (*(volatile unsigned int *)(M36_SYS_REG_BASEADDR + 0x60))
#define M36_SYS_REG_HDMI_PWR_MODE_CTRL	        (*(volatile unsigned int *)(M36_SYS_REG_BASEADDR + 0x6C))
#define M36_SYS_REG_SYS_CTRL					(*(volatile unsigned int *)(M36_SYS_REG_BASEADDR + 0x78))
#define M36_SYS_REG_RST_CTRL					(*(volatile unsigned int *)(M36_SYS_REG_BASEADDR + 0x80))
#define M36_SYS_REG_PLL_SETTING					(*(volatile unsigned int *)(M36_SYS_REG_BASEADDR + 0xC0))
#define	M36_SYS_REG_FUNC_VER_ID					(*(volatile unsigned int *)(M36_SYS_REG_BASEADDR + 0x2400))
#define M39_SYS_REG_SB_HOST                     (*(volatile unsigned int *)(M39_SYS_SB_IOBASE + 0x00))
#define M39_SYS_REG_SB_SLAVE                    (*(volatile unsigned int *)(M39_SYS_SB_IOBASE + 0x04))
#define M39_SYS_REG_SB_TIME                     (*(volatile unsigned int *)(M39_SYS_SB_IOBASE + 0x08))
#define M39_SYS_REG_SB_FIFO_CTL                 (*(volatile unsigned char *)(M39_SYS_SB_IOBASE + 0x0C))
#define M39_SYS_REG_SB_FIFO_DATA                (*(volatile unsigned char *)(M39_SYS_SB_IOBASE + 0x10))
#define M39_SYS_REG_SB_SLAVE_SCB                (*(volatile unsigned char *)(M39_SYS_SB_IOBASE + 0x2C))
#define M39_SYS_REG_SB_SLAVE_SEL                (*(volatile unsigned char *)(M39_SYS_SB_IOBASE + 0x2D))
#define M39_SYS_REG_HDMI_3DPHY                  (*(volatile unsigned char *)(M36_SYS_REG_BASEADDR + 0x6E))
#endif
// ALi 3821
#define M36F_SYS_REG_HDMI_PHY0_CTRL              (*(volatile unsigned char *)(0xB8000630))
#define M36F_SYS_REG_HDMI_PHY1_CTRL              (*(volatile unsigned char *)(0xB8000631))
#define M36F_SYS_REG_HDMI_PHY2_CTRL              (*(volatile unsigned char *)(0xB8000632))
#define M36F_SYS_REG_HDMI_PHY3_CTRL              (*(volatile unsigned char *)(0xB8000633))
/************************************************
 * HDMI Register Address
************************************************/
#if defined(CONFIG_ARM)
#define HDMI_REG_VEN_ID						 (__REG16ALI(HDMI_REG_BASEADDR + 0x00))
#define HDMI_REG_DEV_ID						 (__REG16ALI(HDMI_REG_BASEADDR + 0x02))
#define HDMI_REG_DEV_REV	                (__REG8ALI(HDMI_REG_BASEADDR + 0x04))
#define HDMI_REG_INT	                	 (__REG8ALI(HDMI_REG_BASEADDR + 0x05))
#define HDMI_REG_INT_MASK	                (__REG8ALI(HDMI_REG_BASEADDR + 0x06))
#define HDMI_REG_CTRL	                	 (__REG8ALI(HDMI_REG_BASEADDR + 0x07))
#define HDMI_REG_STATUS		               (__REG8ALI(HDMI_REG_BASEADDR + 0x08))
#define HDMI_REG_CFG0		                 (__REG8ALI(HDMI_REG_BASEADDR + 0x09))
#define HDMI_REG_CFG1		                 (__REG8ALI(HDMI_REG_BASEADDR + 0x0A))
#define HDMI_REG_CFG2		                (__REG8ALI(HDMI_REG_BASEADDR + 0x0B))
#define HDMI_REG_CFG3		                (__REG8ALI(HDMI_REG_BASEADDR + 0x0C))
#define HDMI_REG_CFG4		                 (__REG8ALI(HDMI_REG_BASEADDR + 0x0D))
#define HDMI_REG_CFG5		                 (__REG8ALI(HDMI_REG_BASEADDR + 0x0E))
#define HDMI_REG_CFG6		                 (__REG8ALI(HDMI_REG_BASEADDR + 0x0F))
#define HDMI_REG_HDCP_WR_BKSV0	             (__REG8ALI(HDMI_REG_BASEADDR + 0x10))	// 0x10 - 0x14 5 Bytes of BKsv
#define HDMI_REG_HDCP_WR_BKSV1	             (__REG8ALI(HDMI_REG_BASEADDR + 0x11))	// 0x10 - 0x14 5 Bytes of BKsv
#define HDMI_REG_HDCP_WR_BKSV2	             (__REG8ALI(HDMI_REG_BASEADDR + 0x12))	// 0x10 - 0x14 5 Bytes of BKsv
#define HDMI_REG_HDCP_WR_BKSV3	            (__REG8ALI(HDMI_REG_BASEADDR + 0x13))	// 0x10 - 0x14 5 Bytes of BKsv
#define HDMI_REG_HDCP_WR_BKSV4	             (__REG8ALI(HDMI_REG_BASEADDR + 0x14))	// 0x10 - 0x14 5 Bytes of BKsv
#define HDMI_REG_HDCP_WR_AN		             (__REG8ALI(HDMI_REG_BASEADDR + 0x15))	// 0x15 - 0x1C 8 Bytes of An
#define HDMI_REG_HDCP_RD_AKSV	             (__REG8ALI(HDMI_REG_BASEADDR + 0x1D))	// 0x1D - 0x21 5 Bytes of AKsv
#define HDMI_REG_HDCP_RI_1	                 (__REG16ALI(HDMI_REG_BASEADDR + 0x22))	// 0x22 - 0x23 2 Bytes of Ri
#define HDMI_REG_HDCP_RI_2	                (__REG16ALI(HDMI_REG_BASEADDR + 0x23))	// 0x22 - 0x23 2 Bytes of Ri
#define HDMI_REG_HDCP_KEY_PORT	            (__REG8ALI(HDMI_REG_BASEADDR + 0x24))	// 8-Bit Data Port for Key
#define HDMI_REG_HDCP_KSV_LIST	             (__REG8ALI(HDMI_REG_BASEADDR + 0x25))	// 8-Bit Data Port for KSV list
#define HDMI_REG_GMP				  (__REG8ALI(HDMI_REG_BASEADDR + 0x26))
#define HDMI_REG_BIST0		                (__REG8ALI(HDMI_REG_BASEADDR + 0x27))	// BIST[7:0]
#define HDMI_REG_BIST1		                (__REG8ALI(HDMI_REG_BASEADDR + 0x28))	// BIST[15:8]
#define HDMI_REG_BIST2		                 (__REG8ALI(HDMI_REG_BASEADDR + 0x29))	// BIST[23:16]
#define HDMI_REG_BIST3		                (__REG8ALI(HDMI_REG_BASEADDR + 0x2A))	// BIST[29:25]
#define HDMI_REG_HDCP_STATUS               	(__REG8ALI(HDMI_REG_BASEADDR + 0x2E))
#define HDMI_REG_HDCP_CTRL	                (__REG8ALI(HDMI_REG_BASEADDR + 0x2F))
#define HDMI_REG_HDCP_SHA_H0               (__REG32ALI(HDMI_REG_BASEADDR + 0x30))	// KSV List Verification Value from Repeater
#define HDMI_REG_HDCP_SHA_H1                (__REG32ALI(HDMI_REG_BASEADDR + 0x34))
#define HDMI_REG_HDCP_SHA_H2                (__REG32ALI(HDMI_REG_BASEADDR + 0x38))
#define HDMI_REG_HDCP_SHA_H3               (__REG32ALI(HDMI_REG_BASEADDR + 0x3C))
#define HDMI_REG_HDCP_SHA_H4               (__REG32ALI(HDMI_REG_BASEADDR + 0x40))
#define HDMI_REG_I2S_CHANNEL_STATUS         (__REG32ALI(HDMI_REG_BASEADDR + 0x50))	// 0x50 - 0x54 5 Bytes of Channel Status bits for Audio Sample via I2S
#define HDMI_REG_I2S_CHANNEL_STATUS_LAST_BYTE  (__REG8ALI(HDMI_REG_BASEADDR + 0x54))
#define HDMI_REG_I2S_UV				        (__REG8ALI(HDMI_REG_BASEADDR + 0x55))	// I2S Channel Enable, User bit Validity bit Setting
#define HDMI_REG_CTS_CTRL			        (__REG8ALI(HDMI_REG_BASEADDR + 0x58))
#define HDMI_REG_CTS0				       (__REG8ALI(HDMI_REG_BASEADDR + 0x59))
#define HDMI_REG_CTS1				       (__REG8ALI(HDMI_REG_BASEADDR + 0x5A))
#define HDMI_REG_CTS2				       (__REG8ALI(HDMI_REG_BASEADDR + 0x5B))
#define HDMI_REG_NCTS				       (__REG8ALI(HDMI_REG_BASEADDR + 0x61))
#define HDMI_REG_INFRM_DATA		        	(__REG8ALI(HDMI_REG_BASEADDR + 0x62))
#define HDMI_REG_INFRM_VER			       (__REG8ALI(HDMI_REG_BASEADDR + 0x67))
#define HDMI_REG_INFRM_TYPE			        (__REG8ALI(HDMI_REG_BASEADDR + 0x68))
#define HDMI_REG_INFRM_LENGTH		        (__REG8ALI(HDMI_REG_BASEADDR + 0x69))
#define HDMI_REG_PHY_DRV_CTRL		        (__REG16ALI(HDMI_REG_BASEADDR + 0x6A))
#define HDMI_REG_HDCP_PJ			       (__REG8ALI(HDMI_REG_BASEADDR + 0x6C))
#define HDMI_REG_OPT			        	(__REG8ALI(HDMI_REG_BASEADDR + 0x6D))
#define HDMI_REG_OPT1			        	(__REG8ALI(HDMI_REG_BASEADDR + 0x6E))
#define HDMI_REG_OPT2			        	(__REG8ALI(HDMI_REG_BASEADDR + 0x6F))
#define HDMI_REG_SRAM_ADDRESS		       (__REG8ALI(HDMI_REG_BASEADDR + 0x70))
#define HDMI_REG_SRAM_DATA			        (__REG8ALI(HDMI_REG_BASEADDR + 0x72))
#define HDMI_REG_CEC_STATUS		        	(__REG8ALI(HDMI_REG_BASEADDR + 0x71))
#define HDMI_REG_CEC_CTRL		        	(__REG8ALI(HDMI_REG_BASEADDR + 0x76))
#define HDMI_REG_CEC_TX_ADDR	        	(__REG8ALI(HDMI_REG_BASEADDR + 0x74))
#define HDMI_REG_CEC_TX_DATA	        	(__REG8ALI(HDMI_REG_BASEADDR + 0x75))
#define HDMI_REG_CEC_RX_ADDR	        	(__REG8ALI(HDMI_REG_BASEADDR + 0x77))
#define HDMI_REG_CEC_RX_DATA	        	(__REG8ALI(HDMI_REG_BASEADDR + 0x78))
#define HDMI_REG_CEC_BIU_FREQ	        	(__REG8ALI(HDMI_REG_BASEADDR + 0x79))
#define HDMI_REG_CEC_DIV_NUM	        	(__REG8ALI(HDMI_REG_BASEADDR + 0x7A))
#define HDMI_REG_CEC_LOW_CNT	        	(__REG8ALI(HDMI_REG_BASEADDR + 0x7B))
#define HDMI_REG_HDCP_TOP_MI	        	(__REG8ALI(HDMI_REG_BASEADDR + 0x80))
//S3921 3D TX phy, new feature
#define HDMI_REG_Y_L				(__REG8ALI(HDMI_REG_BASEADDR + 0x90))
#define HDMI_REG_Y_H				(__REG8ALI(HDMI_REG_BASEADDR + 0x91))
#define HDMI_REG_CB_L				(__REG8ALI(HDMI_REG_BASEADDR + 0x92))
#define HDMI_REG_CB_H				(__REG8ALI(HDMI_REG_BASEADDR + 0x93))
#define HDMI_REG_CR_L				(__REG8ALI(HDMI_REG_BASEADDR + 0x94))
#define HDMI_REG_CR_H				(__REG8ALI(HDMI_REG_BASEADDR + 0x95))
#define HDMI_REG_DP_REG0			(__REG8ALI(HDMI_REG_BASEADDR + 0x96))
#define HDMI_REG_DP_REG1			(__REG8ALI(HDMI_REG_BASEADDR + 0x97))
#define HDMI_REG_DP_REG2			(__REG8ALI(HDMI_REG_BASEADDR + 0x98))
#define HDMI_REG_3DPHY_REG		(__REG8ALI(HDMI_REG_BASEADDR + 0x99))
#define HDMI_REG_PP_REG0			(__REG8ALI(HDMI_REG_BASEADDR + 0x9A))
#define HDMI_REG_PP_REG1			(__REG8ALI(HDMI_REG_BASEADDR + 0x9B))
#define HDMI_REG_PP_REG2			(__REG8ALI(HDMI_REG_BASEADDR + 0x9C))
#define HDMI_REG_DUMMY_ADDR	        	    (__REG8ALI(HDMI_REG_BASEADDR + 0xFF))
#else
#define HDMI_REG_VEN_ID						(*(volatile unsigned short *)(HDMI_REG_BASEADDR + 0x00))
#define HDMI_REG_DEV_ID						(*(volatile unsigned short *)(HDMI_REG_BASEADDR + 0x02))
#define HDMI_REG_DEV_REV	                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x04))
#define HDMI_REG_INT	                	(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x05))
#define HDMI_REG_INT_MASK	                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x06))
#define HDMI_REG_CTRL	                	(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x07))
#define HDMI_REG_STATUS		                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x08))
#define HDMI_REG_CFG0		                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x09))
#define HDMI_REG_CFG1		                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x0A))
#define HDMI_REG_CFG2		                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x0B))
#define HDMI_REG_CFG3		                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x0C))
#define HDMI_REG_CFG4		                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x0D))
#define HDMI_REG_CFG5		                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x0E))
#define HDMI_REG_CFG6		                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x0F))
#define HDMI_REG_HDCP_WR_BKSV0	            (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x10))	// 0x10 - 0x14 5 Bytes of BKsv
#define HDMI_REG_HDCP_WR_BKSV1	            (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x11))	// 0x10 - 0x14 5 Bytes of BKsv
#define HDMI_REG_HDCP_WR_BKSV2	            (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x12))	// 0x10 - 0x14 5 Bytes of BKsv
#define HDMI_REG_HDCP_WR_BKSV3	            (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x13))	// 0x10 - 0x14 5 Bytes of BKsv
#define HDMI_REG_HDCP_WR_BKSV4	            (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x14))	// 0x10 - 0x14 5 Bytes of BKsv
#define HDMI_REG_HDCP_WR_AN		            (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x15))	// 0x15 - 0x1C 8 Bytes of An
#define HDMI_REG_HDCP_RD_AKSV	            (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x1D))	// 0x1D - 0x21 5 Bytes of AKsv
#define HDMI_REG_HDCP_RI_1	                (*(volatile unsigned short *)(HDMI_REG_BASEADDR + 0x22))	// 0x22 - 0x23 2 Bytes of Ri
#define HDMI_REG_HDCP_RI_2	                (*(volatile unsigned short *)(HDMI_REG_BASEADDR + 0x23))	// 0x22 - 0x23 2 Bytes of Ri
#define HDMI_REG_HDCP_KEY_PORT	            (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x24))	// 8-Bit Data Port for Key
#define HDMI_REG_HDCP_KSV_LIST	            (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x25))	// 8-Bit Data Port for KSV list
#define HDMI_REG_GMP				  (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x26))
#define HDMI_REG_BIST0		                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x27))	// BIST[7:0]
#define HDMI_REG_BIST1		                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x28))	// BIST[15:8]
#define HDMI_REG_BIST2		                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x29))	// BIST[23:16]
#define HDMI_REG_BIST3		                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x2A))	// BIST[29:25]
#define HDMI_REG_HDCP_STATUS               	(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x2E))
#define HDMI_REG_HDCP_CTRL	                (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x2F))
#define HDMI_REG_HDCP_SHA_H0                (*(volatile unsigned int *)	 (HDMI_REG_BASEADDR + 0x30))	// KSV List Verification Value from Repeater
#define HDMI_REG_HDCP_SHA_H1                (*(volatile unsigned int *)  (HDMI_REG_BASEADDR + 0x34))
#define HDMI_REG_HDCP_SHA_H2                (*(volatile unsigned int *)  (HDMI_REG_BASEADDR + 0x38))
#define HDMI_REG_HDCP_SHA_H3                (*(volatile unsigned int *)  (HDMI_REG_BASEADDR + 0x3C))
#define HDMI_REG_HDCP_SHA_H4                (*(volatile unsigned int *)  (HDMI_REG_BASEADDR + 0x40))
#define HDMI_REG_I2S_CHANNEL_STATUS         (*(volatile unsigned int *)  (HDMI_REG_BASEADDR + 0x50))	// 0x50 - 0x54 5 Bytes of Channel Status bits for Audio Sample via I2S
#define HDMI_REG_I2S_CHANNEL_STATUS_LAST_BYTE  (*(volatile unsigned char *)  (HDMI_REG_BASEADDR + 0x54))
#define HDMI_REG_I2S_UV				        (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x55))	// I2S Channel Enable, User bit Validity bit Setting
#define HDMI_REG_CTS_CTRL			        (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x58))
#define HDMI_REG_CTS0				        (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x59))
#define HDMI_REG_CTS1				        (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x5A))
#define HDMI_REG_CTS2				        (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x5B))
#define HDMI_REG_NCTS				        (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x61))
#define HDMI_REG_INFRM_DATA		        	(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x62))
#define HDMI_REG_INFRM_VER			        (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x67))
#define HDMI_REG_INFRM_TYPE			        (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x68))
#define HDMI_REG_INFRM_LENGTH		        (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x69))
#define HDMI_REG_PHY_DRV_CTRL		        (*(volatile unsigned short *)(HDMI_REG_BASEADDR + 0x6A))
#define HDMI_REG_HDCP_PJ			       (*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x6C))
#define HDMI_REG_OPT			        	(*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x6D))
#define HDMI_REG_OPT1			        	(*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x6E))
#define HDMI_REG_OPT2			        	(*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x6F))
#define HDMI_REG_SRAM_ADDRESS		       (*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x70))
#define HDMI_REG_SRAM_DATA			        (*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x72))
#define HDMI_REG_CEC_STATUS		        	(*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x71))
#define HDMI_REG_CEC_CTRL		        	(*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x76))
#define HDMI_REG_CEC_TX_ADDR	        	(*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x74))
#define HDMI_REG_CEC_TX_DATA	        	(*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x75))
#define HDMI_REG_CEC_RX_ADDR	        	(*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x77))
#define HDMI_REG_CEC_RX_DATA	        	(*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x78))
#define HDMI_REG_CEC_BIU_FREQ	        	(*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x79))
#define HDMI_REG_CEC_DIV_NUM	        	(*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x7A))
#define HDMI_REG_CEC_LOW_CNT	        	(*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x7B))
#define HDMI_REG_HDCP_TOP_MI	        	(*(volatile unsigned char *)(HDMI_REG_BASEADDR + 0x80))
//ALi 3821, 3921 new feature
#define HDMI_REG_Y_L				(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x90))
#define HDMI_REG_Y_H				(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x91))
#define HDMI_REG_CB_L				(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x92))
#define HDMI_REG_CB_H				(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x93))
#define HDMI_REG_CR_L				(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x94))
#define HDMI_REG_CR_H				(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x95))
#define HDMI_REG_DP_REG0			(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x96))
#define HDMI_REG_DP_REG1			(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x97))
#define HDMI_REG_DP_REG2			(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x98))
#define HDMI_REG_3DPHY_REG		(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x99))
#define HDMI_REG_PP_REG0			(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x9A))
#define HDMI_REG_PP_REG1			(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x9B))
#define HDMI_REG_PP_REG2			(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x9C))
#define HDMI_REG_DUMMY_ADDR	        	    (*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0xFF))
#endif

//ALi 3821
#define HDMI_REG_PHY_REG0				(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x6A))
#define HDMI_REG_PHY_REG1				(*(volatile unsigned char *) (HDMI_REG_BASEADDR + 0x6B))
/************************************************
 * hdmi register Bit Field Definition
************************************************/
/* HDMI_REG_INT (0x05), HDMI_REG_INT_MASK (0x06) */
#define B_INT_MDI					0x01
#define B_INT_HDCP					0x02
#define B_INT_FIFO_O				0x04
#define B_INT_FIFO_U				0x08
#define B_INT_IFM_ERR				0x10
#define B_INT_INF_DONE				0x20
#define B_INT_NCTS_DONE				0x40
#define B_INT_CTRL_PKT_DONE			0x80

/* HDMI_REG_CTRL (0x07) */
#define B_SRST						0x01
#ifdef CONFIG_ALI_M3602
#define B_PDB						0x02 // for ALI_M3602
#else
#define SEL_128FS					0x02 // After ALI M3603
#endif
#define B_GENERIC_EN				0x04
#define B_SPD_EN					0x08
#define B_MPEG_EN					0x10
#define B_AUDIO_EN					0x20
#define B_AVI_EN					0x40
#define B_AV_MUTE					0x80

/* HDMI_REG_STATUS (0x08) */
#define B_HTPLG						0x01
#define B_PORD						0x02
#define B_AUD_SAMPLE_DONE			0x04
#define B_GENERIC_DONE				0x08
#define B_SPD_DONE					0x10
#define B_MPEG_DONE					0x20
#define B_AUDIO_DONE				0x40
#define B_AVI_DONE					0x80

#ifdef CONFIG_ALI_DIS_HTPLG
#define HOT_PLUG_STATE              1//s3701c HTPLG and E/JTAG share pin. Disable HTPLG check.
#define HOT_PORD_STATE              1
#else
#define HOT_PLUG_STATE              ((HDMI_REG_STATUS & B_HTPLG) == B_HTPLG)
#define HOT_PORD_STATE              ((HDMI_REG_STATUS & B_PORD) == B_PORD)
#endif

/* HDMI_REG_CFG0 (0x09) */
#define B_I2S_MODE					0x03	// 00(Left Justifed) 01(Right Justified) 10(I2S)
#define B_LRCK						0x04	// For I2S mode: 0(left ch at low level) 1(left ch at high level)
											// For Non I2S mode: 0(left ch at high level) 1(left ch at low level)
#define B_AV_UNMUTE					0x08
#define B_W_LENGTH					0x30	// Word Length: 00(24bit) 01(16bit) 10(28bit) 11(Reserved)
#define B_VSYNC_SEL					0x40	// Set to 1 to fix HDCP Bug (16pixel ENC_DIS timing delay of win_of_opp)
#define B_NOTICE_HIGH				0x80	// Set to 1 to prevent audio sample packet jitter.

/* HDMI_REG_CFG1 (0x0A) */
#define B_HDMI_ENCDIS				0x01	// Enable ENC_EN/ENCDIS for DVI Mode (1:Enable 0:Disable)
#define B_SPDIF						0x02	// 1(SPDIF) 0(I2S)
#define B_DVI						0x04	// 1(DVI Mode) 0(HDMI Mode)
#define B_ONE_BIT_AUD				0x08
#define B_ANA_RST					0x10	// Removed from S3602F
#define B_MEM_SEL					0x10	// Set this bit to select 1(OTP) or 0(SRAM) mode.
#define B_RAMP_EN					0x20
#define B_LFSR_EN					0x40
#define B_CK_SEL					0x80	// HDMI_PHY Clock Source Selection 1(CLK_P) 0(CLK_N)
#define B_LDO_SEL0					0x80	// LDO output voltage control(bit[0]),

/* HDMI_REG_CFG2 (0x0B) */
#define B_TXP_CTL0_2_0				(0x07 <<  (0))	// TXP_CTL0[2:0]
#define B_TXN_CTL0_2_0				(0x07 <<  (3))	// TXN_CTL0[2:0]
#define B_TXP_CTL1_1_0				(0x03 <<  (6))	// TXP_CTL1[1:0]
// 3821 registers
#define B_IDA_CCNTL				0x07    
#define B_DATA_PD					0x08
#define B_ICK_CCNTL				0x70
#define B_CLK_PD					0x80

/* HDMI_REG_CFG3 (0x0C) */
#define B_TXP_CTL1_2				(0x01 <<  (0))	// TXP_CTL1[2]
#define B_TXN_CTL1_2_0				(0x07 <<  (1))	// TXN_CTL1[2:0]
#define B_TXP_CTL2_2_0				(0x07 <<  (4))	// TXP_CTL2[2:0]
#define B_TXN_CTL2_0				(0x01 <<  (7))	// TXN_CTL2[0]
// 3821 registers
#define B_IDA_FCNTL				0x03
#define B_ICK_FCNTL				0x0C
#define B_DTERM					0x70
#define B_PCG_RDY					0x80

/* HDMI_REG_CFG4 (0x0D) */
#define B_TXN_CTL2_2_1				(0x03 <<  (0))	// TXN_CTL2[2:1]
#define B_TXP_CTL3_2_0				(0x07 <<  (2))	// TXP_CTL3[2:0]
#define B_TXN_CTL3_2_0				(0x07 <<  (5))	// TXN_CTL[2:0]
// 3821 registers
#define B_CMU_SEL					0x0F
#define B_CTERM					0x70
#define B_CMU_RDY					0x80

/* HDMI_REG_CFG5 (0x0E) */
#define B_PLL_SEL_1_0				0x03	// PLL_SEL[1:0]	Clock range slection for PLL input.
#define B_ANA_PD					0x04	// Removed from S3602F
#define B_PDCLK						0x08	// Removed from S3602F
#define B_I2S_SWITCH				0x04	// Added from S3602F	set this bit=1 I2S input will always keep '0'
#define B_SPDIF_SWITCH				0x08	// Added from S3602F	set this bit=1, SPDIF input wil always kee[ '0'
#define B_CTS_UPDATE				0x10	// Added from S3602F Set this bit to update CTS value calculated by HW.
#define B_CHK_VALID					0x20	// Set bit=1 will cause no audio output in HDMI (default:0 bypass audio valid check)
#define B_MUTETYPE_SEL				0x40	// Set bit=1 to enable AV Mute/unmute asymmetric operation mode. (default 0)
#define B_T_SEL						0x80	// HDMI PHY Clock Polarity Selection
#define B_LDO_SEL1					0x80	// LDO output voltage control(bit[1]),
// 3821 registers
#define B_CMU_VCOSEL				0x03
#define B_CMU_PD					0x80

/* HDMI_REG_CFG6 (0x0F) */
#define B_AUDIO_CAP_RST				0x01
#define B_EMP_EN					0x06	// 3811 3701C 
#define B_PORD_MASK					0x08 	//PORD_MASK for 3701C
#define B_PLL_SEL_2					0x10
#define B_ASYNC_MODE				0x60
#define B_ASYNC_SET					0x80
// 3921 register
#define B_GCP_EN					0x10
// 3821 registers
#define B_CLK_SEL					0x02
#define B_PHY_ASYNC_SET 			0x04


/* HDMI_REG_HDCP_WR_BKSV (0x10 - 0x14 5 Bytes of BKsv) */
/* 40bit(5Bytes) BKsv, Byte#1 is LSB, Byte#5 is MSB */

/* HDMI_REG_HDCP_WR_AN (0x15 - 0x1C 8 Bytes of An) */

/* HDMI_REG_HDCP_RD_AKSV (0x1D - 0x21 5 Bytes of AKsv) */

/* HDMI_REG_HDCP_RI (0x22 - 0x23 2 Bytes of Ri) */

/* HDMI_REG_HDCP_KEY_PORT (0x24 8-Bit Data Port for Key) */

/* HDMI_REG_HDCP_KSV_LIST (0x25 8-Bit Data Port for KSV list) */

/* HDMI_REG_GMP (0x26) */
#define B_GMP_ON_NXT_FLD			0x01
#define B_GMP_EN					0x02
#define B_GMP_DONE				0x04
#define B_VSI_EN					0x08
#define B_VSI_DONE					0x10
#define B_HBR_MODE				0x20
#define B_HBR_RSVD				0x40
#define B_4SBPKT_EN				0x80

/* HDMI_REG_BIST0 - HDMI_REG_BIST3 (0x27- 0x2A BIST[29:0] */

/* HDMI_REG_HDCP_STATUS (0x2E) */
#define B_ENC_ON					0x01	// Status indicate Encryption is enable, it cleared by power on reset or CP_Rst
#define B_BKSV_ERR					0x02	// Indicate the BKSV not 20bits '0', 20bits '1'
#define B_RI_RDY					0x04	// Indicate Ri value is available.
#define B_V_MATCH					0x08	// Set by HW when V=V'
#define B_V_RDY						0x10	// Indicate KSV list integrity verification value V is available.
#define B_AKSV_RDY					0x20
#define B_PJ_RDY					0x40

/* HDMI_REG_HDCP_CTRL (0x2F) */
#define B_ENC_EN					0x01
#define B_AUTHEN_EN					0x02
#define B_CP_RST					0x04
#define B_AN_STOP					0x08
#define B_RX_RPTR					0x10
#define B_SCRAMBLE					0x20
#define B_HOST_KEY					0x40
#define B_SHA_EN					0x80

/* HDMI_REG_HDCP_SHA_H0 - HDMI_REG_HDCP_SHA_H4 (0x30 - 0x43) KSV List Verification Value from Repeater */

/* HDMI_REG_I2S_CHANNEL_STATUS (00x50 - 0x54 5 Bytes of Channel Status bits for Audio Sample via I2S) */

/* HDMI_REG_I2S_UV (0x55) */
#define B_USER_BIT					0x01	// User bit
#define B_VALIDITY_BIT				0x02	// Validity bit for audio sample supplied via I2S Channel
#define B_CH_EN						0x3C	// CH_En[3:0] I2S Channel enable.
#define B_NORMAL_MONO_SEL			0x40	// Set 0:Normal/1:Black Screen display.
#define B_RGB_YCBCR_MONO			0x80	// Switch 0:RGB/1:YCbCr mode for Black screen display

/* HDMI_REG_CTS_CTRL (0x58) */
#define B_SOFT						0x01	// 1: NCTS packet take the CTS value from register calculated by software, 0: take HW generated CTS value.
#define B_CTS_1_INI					0x02	// 1: Set CTS_REG to 20'b1 0: Set CTS_REG to 20'b0

/* HDMI_REG_CTS0 - HDMI_REG_CTS2 (0x59 - 0x5B) CTS[19:0] */

/* HDMI_REG_NCTS (0x61) */
/* Software calaulated data port */

/* HDMI_REG_INFRM_DATA (0x62) */

/* InfoFrame Header: HDMI_REG_INFRM_VER, HDMI_REG_INFRM_TYPE, HDMI_REG_INFRM_LENGTH (0x67 - 0x69) */

/* HDMI_REG_PHY_DRV_CTRL (0x6A - 0x6B) DRV_CTL[15:0] */
/* 	CH0_N/P - DRV_CTL[03:00]
	CH1_N/P - DRV_CTL[07:04]
	CH2_N/P - DRV_CTL[11:08]
	CLK_N/P - DRV_CTL[15:12] */

/* HDMI_REG_PHY_REG0 ( 0x6A) */
// 3821 registers
#define B_SEL0                      0x04    // for M3821, HDMI 1.4
#define B_SEL1_2                 0x18
#define B_SEL3                      0x20
#define B_SEL4                      0x40
#define B_SEL5                      0x80

/* HDMI_REG_PHY_REG1 (0x6B) */
// 3821 registers
#define B_CMU_MONVCO                0x0F    // for M3821, HDMI 1.4
#define B_DRVBST_CNTL               0x70
#define B_CAL_DN                    0x80    

/* HDMI_REG_HDCP_PJ (0x6C) */

/* HDMI_REG_OPT (0x6D) */
#define B_AC						0x01	// Advance Chipher for HDCP Encryption
#define B_ELINK						0x02	// Enhanced Link Verification
#define B_ACP_EN					0x04	//
#define B_ACP_DONE					0x08	//
#define B_ISRC1_EN					0x10
#define B_ISRC1_DONE				0x20
#define B_ISRC2_EN					0x40
#define B_ISRC2_DONE				0x80

/* HDMI_REG_OPT1 (0x6E) */
#define B_CEC_ADDR					0x0F	// CEC Logic Address
#define B_CEC_MSK					0x20	// CEC Interrupt Mask
#define B_PRO_SEL					0x40	// Select signal for hardware debug
#define B_PRO_SEL1					0x80

/* HDMI_REG_OPT2 (0x6F) */
#define B_HI2LO						0x01
#define B_LD_KSV					0x02
#define B_CEC_RST					0x04
#define B_CTL3_EN					0x08
#define B_RAM_EN					0x10
#define B_RAM_WR					0x20
#define B_WD_ADDRH					0x80

/* HDMI_REG_SRAM_ADDRESS (0x70) */
/* HDMI_REG_SRAM_DATA (0x72) */

/* HDMI_REG_CEC_STATUS (0x71) */
#define B_RCV_UNKNOW				0x01
#define B_STR_RISEDG				0x02
#define B_STR_FALEDG				0x04
#define B_BLK_RISEDG				0x08
#define B_BLK_FALEDG				0x10
#define B_TX_DONE					0x20
#define B_RX_EOM					0x40
#define B_REFILL					0x80

/* HDMI_REG_CEC_CTRL (0x76) */
#define B_CEC_RX_DATA_CLR			0x08
#define B_CEC_TX_TRY_DONE			0x20
#define B_CEC_TX_EOM				0x40
#define B_CEC_TX_EN					0x80

/* HDMI_REG_CEC_TX_ADDR (0x74) */
#define B_CEC_TX_REG_ADDR			0x0F
#define B_CEC_TX_REG_RW				0x10

/* HDMI_REG_CEC_TX_DATA (0x75) */

/* HDMI_REG_CEC_RX_ADDR (0x77) */
#define B_CEC_RX_BLOCK_CNT			0x0F

/* HDMI_REG_CEC_RX_DATA (0x78) */

/* HDMI_REG_CEC_BIU_FREQ (0x79) */

/* HDMI_REG_CEC_DIV_NUM (0x7A) */
#define B_DIV_NUM					0x3F
#define B_BYPASS					0x40
#define B_BRDCST					0x80

/* HDMI_REG_CEC_LOW_CNT (0x7B) */
#define B_LOW_CNT					0x0F

/* HDMI_REG_DP_REG0 (0x96) */
#define B_YUV_CH_SEL				0x07
#define B_RGB_YUV_SEL				0x08
#define B_DE_DATA_WIDTH			0x30
#define B_RND_EN					0x40
#define B_444TO422_EN				0x80

/* HDMI_REG_DP_REG1 (0x97) */
//3821 registers
#define B_ASYNC_FIFO_INI			0x03
#define B_PHY_ASYNC_INI			0x0C
//3921 registers
#define B_RX2_RSTN					0x01
#define B_VOU_BUF_ADDR_INI			0x06
#define B_13DOT5_MODE				0x08
// the same
#define B_DP_MODE					0x30
#define B_DATA_BUF_GRP_SEL			0xC0

/* HDMI_REG_DP_REG2 (0x98) */
#define B_DP_RSTN					0x01
// 3921 registers
#define B_VIDEO_MODE				0x02
#define B_HE_SE_SEL					0x04
#define B_DP_POLARITY				0x08

/* HDMI_3DPHY_REG (0x99) */
#define B_ENHPDRXSENSE				0x01
#define B_TX_READY					0x02
#define B_DTB						0x0C
#define B_RX_SENSE					0x10
#define B_VREGTEST					0x60
#define B_VREGLPN					0x80

/* HDMI_PP_REG1 (0x9B) */
#define B_PP_LINE_11_8				0x0F
#define B_PP_VALUE				0x70
#define B_CLEAR_PP_VALUE			0x80

/* HDMI_PP_REG2 (0x9C) */
#define B_PP_VALUE_INT				0x01
#define B_PP_VALUE_INT_MASK		0x02
/* HDMI_REG_HDCP_TOP_MI (0x80 - 0x87) HDCP_TOP_MI[63:0] */

void hdmi_set_txp_ctl0(unsigned char cur);
void hdmi_set_txn_ctl0(unsigned char cur);
void hdmi_set_txp_ctl1(unsigned char cur);
void hdmi_set_txn_ctl1(unsigned char cur);
void hdmi_set_txn_ctl2(unsigned char cur);
void hdmi_set_txp_ctl2(unsigned char cur);
void hdmi_set_txp_ctl3(unsigned char cur);
void hdmi_set_txn_ctl3(unsigned char cur);
void hdmi_set_emp_en(unsigned char emp_en);
void hdmi_set_ldo_sel(unsigned char ldo);
void hdmi_set_pll_sel(unsigned char pll_sel);
INT32 hdmi_get_hdmi_interrupt2_source(void);
//ALi 3821 phy setting
void hdmi_set_ida_ccntl(unsigned char cur);
void hdmi_set_ick_ccntl(unsigned char cur);
void hdmi_set_ida_fcntl(unsigned char cur);
void hdmi_set_ick_fcntl(unsigned char cur);
void hdmi_set_drvbst_cntl(unsigned char cur);
void hdmi_set_dterm(unsigned char cur);
INT32 hdmi_get_phy_interrupt_source(void);
void hdmi_set_pcg_rdy(bool b_enable);
INT32 hdmi_get_pcg_rdy(void);
void hdmi_set_cterm(unsigned char cur);
void hdmi_set_cmu_rdy(bool b_enable);
INT32 hdmi_get_cmu_rdy(void);
void hdmi_set_cmu_pd(bool b_enable);
void hdmi_set_cmu_vcosel(unsigned char cur);
void hdmi_set_phy_sel0(unsigned char cur);
void hdmi_set_phy_sel1_2(unsigned char cur);
void hdmi_set_phy_sel3(unsigned char cur);
void hdmi_set_async_fifo_ini(unsigned char cur);
void hdmi_set_phy_async_ini(unsigned char cur);
void hdmi_set_phy_pcg_powerdown(bool b_enable);
void hdmi_set_phy_reference_clk(bool b_enable);
void hdmi_set_phy_rst(bool b_enable);
void hdmi_set_phy_deepcolor_sel(unsigned char mode);
void hdmi_set_phy_pcg_sel(unsigned char select);

void hdmi_set_yuv_ch_sel(unsigned char cur);
void hdmi_set_de_data_width(unsigned char cur);
void hdmi_set_vou_buf_addr(unsigned char cur);
void hdmi_set_dp_mode(unsigned char cur);
void hdmi_set_data_buf_grp_sel(unsigned char cur);
unsigned char hdmi_get_pp_value(void);
#ifdef CONFIG_ALI_CHIP_M3921
void hdmi_set_3d_phy_power(bool bPower);
#endif
#endif //_HDMI_REGISTER_H_

