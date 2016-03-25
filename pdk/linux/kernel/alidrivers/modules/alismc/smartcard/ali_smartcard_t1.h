/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_t1.h
 *
 *  Description: This file contains functions to manage T1 send/rcv
 *		             
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  1. 
 *  
 ****************************************************************************/

#ifndef _ALI_SMARTCARD_T1_H_
#define _ALI_SMARTCARD_T1_H_

#include "ali_smartcard.h"
#include "ali_smartcard_atr.h"
#include "ali_smartcard_txrx.h"
#include "ali_smartcard_dev.h"

/* T=1 protocol constants */
#define T1_I_BLOCK		0x00
#define T1_R_BLOCK		0x80
#define T1_S_BLOCK		0xC0
#define T1_MORE_BLOCKS		0x20
#define T1_BUFFER_SIZE		(3 + 254 + 2)

/********************************************
T1 related Macro define
*********************************************/
/* I block */
#define T1_I_SEQ_SHIFT		6

/* R block */
#define T1_IS_ERROR(pcb)	((pcb) & 0x0F)
#define T1_EDC_ERROR		0x01
#define T1_OTHER_ERROR		0x02
#define T1_R_SEQ_SHIFT		4

/* S block stuff */
#define T1_S_IS_RESPONSE(pcb)	((pcb) & T1_S_RESPONSE)
#define T1_S_TYPE(pcb)		((pcb) & 0x0F)
#define T1_S_RESPONSE		0x20
#define T1_S_RESYNC		    0x00
#define T1_S_IFS		    0x01
#define T1_S_ABORT		    0x02
#define T1_S_WTX		    0x03

/* T1 MACROs */
enum 
{
	SENDING, 
	RECEIVING, 
	RESYNCH, 
	DEAD
};

#define NAD 0
#define PCB 1
#define LEN 2
#define DATA 3

enum {
	IFD_PROTOCOL_RECV_TIMEOUT = 0x0000,
	IFD_PROTOCOL_T1_BLOCKSIZE,
	IFD_PROTOCOL_T1_CHECKSUM_CRC,
	IFD_PROTOCOL_T1_CHECKSUM_LRC,
	IFD_PROTOCOL_T1_IFSC,
	IFD_PROTOCOL_T1_IFSD,
	IFD_PROTOCOL_T1_STATE,
	IFD_PROTOCOL_T1_MORE
};

/* T1 protocol private*/
typedef struct {
	INT32		state;  /*internal state*/

	UINT8		ns;	/* reader side  Send sequence number */
	UINT8		nr;	/* card side  RCV sequence number*/
	UINT32		ifsc;
	UINT32		ifsd;

	UINT8		wtx;		/* block waiting time extention*/
	UINT32		retries;
	INT32		rc_bytes; 	/*checksum bytes, 1 byte for LRC, 2 for CRC*/

	UINT32 		BGT;
	UINT32 		BWT;
	UINT32 		CWT;
	
	UINT32		(*checksum)(UINT8* data, UINT32 len, UINT8* rc);

	INT8		more;	/* more data bit */
	UINT8		previous_block[4];	/* to store the last R-block */
	UINT8		sdata[T1_BUFFER_SIZE];
	UINT8		rdata[T1_BUFFER_SIZE];
} t1_state_t;

/* Buffer parameter */
typedef struct t1_buf 
{
	UINT8*		base;
	UINT32		head, tail, size;
	UINT32		overrun;
} t1_buf_t;

/* T1 Protocol operation */
extern int smc_t1_get(struct smartcard_private *tp);
extern void smc_t1_set_checksum(t1_state_t *t1, UINT8 csum);
extern INT32 smc_t1_set_param(t1_state_t *t1, INT32 type, INT32 value);
extern void smc_t1_init(t1_state_t *t1);
extern int smc_t1_bi_comm(struct smc_device *dev, smc_t1_trans_t *p_t1_trans);
extern int smc_t1_xcv(struct smc_device *dev, smc_t1_xcv_t *p_t1_xcv);
extern int smc_t1_negociate_ifsd(struct smc_device*dev, smc_t1_nego_ifsd_t *p_t1_nego_ifsd);

#endif
