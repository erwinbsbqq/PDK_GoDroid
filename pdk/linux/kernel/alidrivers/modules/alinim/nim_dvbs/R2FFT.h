/*****************************************************************************
*    Copyright (C)2003 Ali Corporation. All Rights Reserved.
*
*    File:     R2FFT.h
*
*    Description:    header file for Radix 2 FFT for software blind search
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	2003/12/08        Oversea       Ver 0.1       Create file.
*	2.  2004/01/09		  Oversea 	  	Ver 0.2       Modified by Oversea
*	3.  2005/06/27        Berg Xing     Ver 0.3       Code size optimize
*   4.  2005/06/28        Berg Xing     Ver 0.4       Remove warning information when compiling
*
*****************************************************************************/

#ifndef __R2FFT_H__
#define __R2FFT_H__

//#include <linux/ali_basic.h>
//

#if defined(__NIM_LINUX_PLATFORM__)
#include "../basic_types.h"
#elif defined(__NIM_TDS_PLATFORM__)
#include <types.h>

#endif

#ifdef __cplusplus 
extern "C" { 
#endif 

#define		FFT_BITWIDTH			10
#define		FFT_LAYER				10

//void R2FFT(int *FFT_I_1024, int *FFT_Q_1024);
void R2FFT(INT32 *FFT_I_1024, INT32 *FFT_Q_1024);

#ifdef __cplusplus 
} 
#endif 
#endif /* __R2FFT_H__*/

