/*****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2004 Copyright (C)
 *
 *  File: basic_types.h
 *
 *  Contents: 	This file define the basic data types which may be used
 *			throughout the project.
 *  History:
 *		Date		Author      		Version 	Comment
 *		==========	==================	========== 	=======
 *  1.  03/09/2004  Tom Gao     		0.1.000 	Initial
 *
 *****************************************************************************/
#ifndef __BASIC_TYPES_H__
#define __BASIC_TYPES_H__



typedef char			INT8;
typedef unsigned char	UINT8;

typedef short			INT16;
typedef unsigned short	UINT16;

typedef long			INT32;
typedef unsigned long	UINT32;

typedef unsigned long long UINT64;
typedef long long INT64;

//we recommand not using BOOL as return value, just use RET_CODE
typedef int				BOOL;

#ifndef FALSE
#define	FALSE			(0)
#endif
#ifndef	TRUE
#define	TRUE			(!FALSE)
#endif

//RET for return code, you mey define your privte RET_??? code from
//0x80000000 to 0xFFFFFFFF.
//typedef INT32			RET_CODE;
//#define RET_SUCCESS		((INT32)0)
//#define	RET_FAILURE		((INT32)1)
//#define RET_BUSY		((INT32)2)
//#define	RET_STA_ERR	((INT32)3)


#define NULL 			((void *)0)

//use void, NO VOID exist

//typedef UINT32			HANDLE;
//#define	INVALID_HANDLE	((HANDLE)0xFFFFFFFF)
//typedef void*              VP;


#endif	//__BASIC_TYPES_H__

