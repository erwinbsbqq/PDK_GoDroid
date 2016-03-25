/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_atr.h
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard reader attribute.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/

#ifndef _ALI_SMARTCARD_ATR_H_
#define _ALI_SMARTCARD_ATR_H_

#include "ali_smartcard_txrx.h"
#include "ali_smartcard_t1.h"

/////////////////////////////////////////////////////////////////
//atr
////////////////////////////////////////////////////////////////
/* Paramenters */
#define ATR_MAX_SIZE 		33	/* Maximum size of ATR byte array */
#define ATR_MAX_HISTORICAL	15	/* Maximum number of historical bytes */
#define ATR_MAX_PROTOCOLS	7	/* Maximun number of protocols */
#define ATR_MAX_IB		4	/* Maximum number of interface bytes per protocol */
#define ATR_CONVENTION_DIRECT	0	/* Direct convention */
#define ATR_CONVENTION_INVERSE	1	/* Inverse convention */
#define ATR_PROTOCOL_TYPE_T0	0	/* Protocol type T=0 */
#define ATR_PROTOCOL_TYPE_T1	1	/* Protocol type T=1 */
#define ATR_PROTOCOL_TYPE_T2	2	/* Protocol type T=2 */
#define ATR_PROTOCOL_TYPE_T3	3	/* Protocol type T=3 */
#define ATR_PROTOCOL_TYPE_T14	14	/* Protocol type T=14 */
#define ATR_INTERFACE_BYTE_TA	0	/* Interface byte TAi */
#define ATR_INTERFACE_BYTE_TB	1	/* Interface byte TBi */
#define ATR_INTERFACE_BYTE_TC	2	/* Interface byte TCi */
#define ATR_INTERFACE_BYTE_TD	3	/* Interface byte TDi */
#define ATR_PARAMETER_F		0	/* Parameter F */
#define ATR_PARAMETER_D		1	/* Parameter D */
#define ATR_PARAMETER_I		2	/* Parameter I */
#define ATR_PARAMETER_P		3	/* Parameter P */
#define ATR_PARAMETER_N		4	/* Parameter N */
#define ATR_INTEGER_VALUE_FI	0	/* Integer value FI */
#define ATR_INTEGER_VALUE_DI	1	/* Integer value DI */
#define ATR_INTEGER_VALUE_II	2	/* Integer value II */
#define ATR_INTEGER_VALUE_PI1	3	/* Integer value PI1 */
#define ATR_INTEGER_VALUE_N	4	/* Integer value N */
#define ATR_INTEGER_VALUE_PI2	5	/* Integer value PI2 */

/* Default values for paramenters */
#define ATR_DEFAULT_F	372
#define ATR_DEFAULT_D	1
#define ATR_DEFAULT_I 	50
#define ATR_DEFAULT_N	0
#define ATR_DEFAULT_P	5
#define ATR_DEFAULT_WI	10

#define ATR_DEFAULT_BWI	4	
#define ATR_DEFAULT_CWI	13

#define ATR_DEFAULT_IFSC	32
#define ATR_DEFAULT_IFSD	32
#define ATR_DEFAULT_BGT	22
#define ATR_DEFAULT_CHK	0	//default checksum is LRC

/*
 *  Smart Card attribute text
 */
typedef struct
{
	UINT8 length;
	UINT8 TS;
	UINT8 T0;
	struct
	{
		UINT8 value;
		BOOL present;
	}
	ib[ATR_MAX_PROTOCOLS][ATR_MAX_IB], TCK;
	UINT8 pn;
	UINT8 hb[ATR_MAX_HISTORICAL];
	UINT8 hbn;
}atr_t;

/* Decode attribute data in private data */
struct smartcard_atr{
    UINT8 atr_buf[ATR_MAX_SIZE];
    UINT16 atr_size;

    spinlock_t atr_spinlock;
    atr_t *atr_info;
    enum smc_atr_result atr_rlt;
};

/* Smart card ATR operation */
extern int smc_atr_get(struct smc_device *dev);
extern void *smc_atr_alloc(struct smartcard_private *tp);
extern void smc_atr_free(struct smartcard_private *tp);
extern int smc_atr_info_config(struct smc_device *dev);

#endif