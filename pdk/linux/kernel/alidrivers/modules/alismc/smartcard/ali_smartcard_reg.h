/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_reg.h
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard reader management.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/

#ifndef _ALI_SMARTCARD_REG_H_
#define _ALI_SMARTCARD_REG_H_

/* SMC controller registers */
#define REG_SCR_CTRL	0x00
#define REG_ICCR		0x01
#define REG_FIFO_CTRL	0x02
#define REG_CLK_SEL		0x03
#define REG_IER0		0x04
#define REG_IER1		0x05
#define REG_ISR0		0x06
#define REG_ISR1		0x07
#define REG_ICCSR		0x08
#define REG_PDBR		0x09
#define REG_RBR			0x0A
#define REG_THR			0x0B
#define REG_ETU0		0x0C
#define REG_ETU1		0x0D
#define REG_GTR0		0x0E
#define REG_GTR1		0x0F
#define REG_CBWTR0		0x10
#define REG_CBWTR1		0x11
#define REG_CBWTR2		0x12
#define REG_CBWTR3		0x13
#define REG_RFIFO_CNT	0x14
#define REG_RCVPR		0x15
#define REG_RCNT_ETU	0x16
#define REG_PIN_VALUE	0x18
#define REG_RXTX_PP		0x19
#define REG_GT_CNT		0x1a
#define REG_WT_CNT		0x1c
#define REG_FSM_STATE	0x20
#define REG_COUNT_DLY	0x22
#define REG_CLKL_SEL	0x24
#define REG_CLKH_SEH	0x26
#define REG_RCNT_3ETU	0x28
#define REG_TX_CNT		0x2a
#define REG_RX_CNT8		0x2c
#define REG_CLK_FRAC	0x2d
#define REG_DEV_CTRL	0x2e
#define REG_CLK_VPP		0x2f
#define REG_VPP_GPIO	0x30

/* MAX register space */
#define REG_MAX_LIMIT   0x200

/* Smart card status */
#define SMC_RB_ICCR_PRT_EN	0x30
#define SMC_RB_ICCR_CLK		0x04
#define SMC_RB_ICCR_RST		0x02
#define SMC_RB_ICCR_DIO		0x08
#define SMC_RB_ICCR_VCC		0x01
#define SMC_RB_ICCR_OP			0x0e
#define SMC_RB_ICCR_AUTO_PRT	0x10
#define SMC_RB_CTRL_VPP		0x04

#define SMC_SCR_CTRL_OP		0xe0
#define SMC_SCR_CTRL_INVESE	0x08
#define SMC_SCR_CTRL_TRANS	0x04
#define SMC_SCR_CTRL_RECV	0x02

#define SMC_ISR0_FIFO_EMPTY	0x80
#define SMC_ISR0_FIFO_TRANS	0x40
#define SMC_ISR0_FIFO_RECV	0x20
#define SMC_ISR0_TIMEOUT	0x10
#define SMC_ISR0_BYTE_RECV	0x04
#define SMC_ISR0_PE_RECV	0x01
#define SMC_ISR0_PE_TRANS	0x02

#define SMC_FIFO_CTRL_EN	0x80
#define SMC_FIFO_CTRL_TX_OP	0x40
#define SMC_FIFO_CTRL_RX_OP	0x20
#define SMC_FIFO_CTRL_ENABLE	0x80
#define SMC_FIFO_CTRL_TX_INIT	0x40
#define SMC_FIFO_CTRL_RX_INIT	0x20

#define SMC_ISR1_RST_HIGH		0x01
#define SMC_ISR1_RST_LOW		0x02
#define SMC_ISR1_RST_NATR		0x08
#define SMC_ISR1_CARD_INSERT	0x10
#define SMC_ISR1_CARD_REMOVE	0x20
#define SMC_ISR1_COUNT_ST		0x40

#define SMC_IER0_BYTE_RECV_TRIG	0x04
#define SMC_IER0_BYTE_TRANS_TRIG	0x08
#define SMC_IER0_RECV_FIFO_TRIG	0x20
#define SMC_IER0_TRANS_FIFO_TRIG	0x40
#define SMC_IER0_TRANS_FIFO_EMPY	0x80

/* SMC reset address */
#define SMC_DEV_RST_ADDR_0  0x18000000
#define SMC_DEV_RST_ADDR_1  0x18000060
#define SMC_DEV_RST_ADDR_2  0x18000080

#endif
 