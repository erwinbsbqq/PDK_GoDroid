/*****************************************************************************
*    Copyright (C)2003 Ali Corporation. All Rights Reserved.
*
*    File:     R2FFT.c
*
*    Description:    Source file for Radix 2 FFT for software blind search
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	2003/12/08        Oversea       Ver 0.1       Create file.
*	2.  2004/01/09		  Oversea 	  	Ver 0.2       Modified by Oversea
*	3.  2005/06/27        Berg Xing     Ver 0.3       Code size optimize
*   4.  2005/06/28        Berg Xing  Ver 0.4  Remove warning information when compiling
*
*****************************************************************************/
//#include <types.h>
#include "R2FFT.h"

#define MAGIC_CONST_256 256
#define MAGIC_CONST_15  15
#define MAGIC_CONST_5   5
#define MAGIC_CONST_9  9

static const INT16	neg_sin[257] =
{
    0    ,
    -3   ,
    -6   ,
    -9   ,
    -13  ,
    -16  ,
    -19  ,
    -22  ,
    -25  ,
    -28  ,
    -31  ,
    -35  ,
    -38  ,
    -41  ,
    -44  ,
    -47  ,
    -50  ,
    -53  ,
    -56  ,
    -60  ,
    -63  ,
    -66  ,
    -69  ,
    -72  ,
    -75  ,
    -78  ,
    -81  ,
    -84  ,
    -88  ,
    -91  ,
    -94  ,
    -97  ,
    -100 ,
    -103 ,
    -106 ,
    -109 ,
    -112 ,
    -115 ,
    -118 ,
    -121 ,
    -124 ,
    -127 ,
    -130 ,
    -134 ,
    -137 ,
    -140 ,
    -143 ,
    -146 ,
    -149 ,
    -152 ,
    -155 ,
    -158 ,
    -161 ,
    -164 ,
    -167 ,
    -170 ,
    -172 ,
    -175 ,
    -178 ,
    -181 ,
    -184 ,
    -187 ,
    -190 ,
    -193 ,
    -196 ,
    -199 ,
    -202 ,
    -205 ,
    -207 ,
    -210 ,
    -213 ,
    -216 ,
    -219 ,
    -222 ,
    -225 ,
    -227 ,
    -230 ,
    -233 ,
    -236 ,
    -239 ,
    -241 ,
    -244 ,
    -247 ,
    -250 ,
    -252 ,
    -255 ,
    -258 ,
    -261 ,
    -263 ,
    -266 ,
    -269 ,
    -271 ,
    -274 ,
    -277 ,
    -279 ,
    -282 ,
    -284 ,
    -287 ,
    -290 ,
    -292 ,
    -295 ,
    -297 ,
    -300 ,
    -302 ,
    -305 ,
    -308 ,
    -310 ,
    -313 ,
    -315 ,
    -317 ,
    -320 ,
    -322 ,
    -325 ,
    -327 ,
    -330 ,
    -332 ,
    -334 ,
    -337 ,
    -339 ,
    -342 ,
    -344 ,
    -346 ,
    -348 ,
    -351 ,
    -353 ,
    -355 ,
    -358 ,
    -360 ,
    -362 ,
    -364 ,
    -366 ,
    -369 ,
    -371 ,
    -373 ,
    -375 ,
    -377 ,
    -379 ,
    -381 ,
    -384 ,
    -386 ,
    -388 ,
    -390 ,
    -392 ,
    -394 ,
    -396 ,
    -398 ,
    -400 ,
    -402 ,
    -404 ,
    -406 ,
    -407 ,
    -409 ,
    -411 ,
    -413 ,
    -415 ,
    -417 ,
    -419 ,
    -420 ,
    -422 ,
    -424 ,
    -426 ,
    -427 ,
    -429 ,
    -431 ,
    -433 ,
    -434 ,
    -436 ,
    -438 ,
    -439 ,
    -441 ,
    -442 ,
    -444 ,
    -445 ,
    -447 ,
    -449 ,
    -450 ,
    -452 ,
    -453 ,
    -454 ,
    -456 ,
    -457 ,
    -459 ,
    -460 ,
    -461 ,
    -463 ,
    -464 ,
    -465 ,
    -467 ,
    -468 ,
    -469 ,
    -471 ,
    -472 ,
    -473 ,
    -474 ,
    -475 ,
    -477 ,
    -478 ,
    -479 ,
    -480 ,
    -481 ,
    -482 ,
    -483 ,
    -484 ,
    -485 ,
    -486 ,
    -487 ,
    -488 ,
    -489 ,
    -490 ,
    -491 ,
    -492 ,
    -493 ,
    -493 ,
    -494 ,
    -495 ,
    -496 ,
    -497 ,
    -497 ,
    -498 ,
    -499 ,
    -500 ,
    -500 ,
    -501 ,
    -502 ,
    -502 ,
    -503 ,
    -503 ,
    -504 ,
    -504 ,
    -505 ,
    -505 ,
    -506 ,
    -506 ,
    -507 ,
    -507 ,
    -508 ,
    -508 ,
    -509 ,
    -509 ,
    -509 ,
    -510 ,
    -510 ,
    -510 ,
    -510 ,
    -511 ,
    -511 ,
    -511 ,
    -511 ,
    -511 ,
    -512 ,
    -512 ,
    -512 ,
    -512 ,
    -512 ,
    -512 ,
    -512 ,
    -512
};

void	R2FFT(INT32 *fft_l_1024, INT32 *fft_q_1024)
{
    INT32	index1=0;
    INT32	index2=0; //unsigned
    INT32	number_wncoeff=0;
    INT32	number_butterfly_onewn=0; //unsigned
    INT32	data_real=0;
    INT32	data_imag=0; //signed
    INT32	data_real2=0;
    INT32 	data_imag2=0; //signed
    INT32	index_wncoeff=0;//unsigned
    INT32	wn_real=0;
    INT32	wn_imag=0; //signed
    INT32	data_temp1=0;
    INT32 	data_temp2=0; //signed
    INT32	data_temp3=0;//signed
    INT32	invert=0;
    INT32	l=0;
    INT32	k=0;
    INT32	i=0;

    for (l = 1; l <= FFT_LAYER; l++) //different FFT layer
    {
        number_wncoeff = 1;
        number_wncoeff <<= l - 1; //number of Wn coefficients in one layer
        number_butterfly_onewn = 1;
	 //number of butterfly in one Wn coefficient
        number_butterfly_onewn <<= FFT_LAYER - l;
        for (k = 0; k < number_wncoeff; k++) //different FFT Wn
        {

            //look up -sin table to get Wn
	    //index of Wn to look up table
            index_wncoeff = k * number_butterfly_onewn; 
            if (index_wncoeff > MAGIC_CONST_256)
            {
                index_wncoeff = 512 - index_wncoeff;
                invert = 0;
            }
            else
            {
                invert = 1;
            }
            wn_imag = neg_sin[index_wncoeff];
            index_wncoeff = 256 - index_wncoeff;
            wn_real = neg_sin[index_wncoeff];
            if (invert)
            {
                wn_real = -wn_real;
            }
            //look up -sin table to get Wn


            //butterfly
            index1 = k;
            for (i = 0; i < number_butterfly_onewn; i++)
            {
                index2 = index1 + number_wncoeff;
                data_real = *(fft_l_1024 + index2);
                data_imag = *(fft_q_1024 + index2);

                data_temp1 = data_real * wn_real; //21 bits
                data_temp2 = data_imag * wn_imag; //21 bits
                data_temp3 = data_temp1 - data_temp2;
		//divide result by 512 due to quantity of sin value
                data_temp3 >>= 9; 
		//exceed limit of 10 bits
                if ((((data_temp3 >> MAGIC_CONST_9) & MAGIC_CONST_15) != 0x0) && (((data_temp3 >> MAGIC_CONST_9) \
		       & MAGIC_CONST_15) != MAGIC_CONST_15)) 
                {
                    if ((data_temp3 >> 12) & 1)
                    {
                        data_temp3 = -512;
                    }
                    else
                    {
                        data_temp3 = 511;
                    }
                }
                data_real2 = data_temp3;

                data_temp1 = data_real * wn_imag; //21 bits
                data_temp2 = data_imag * wn_real; //21 bits
                data_temp3 = data_temp1 + data_temp2;
		//divide result by 512 due to quantity of sin value
                data_temp3 >>= 9; 
		//exceed limit of 10 bits
                if ((((data_temp3 >> MAGIC_CONST_9) & MAGIC_CONST_15) != 0x0) && (((data_temp3 >> MAGIC_CONST_9) \
		 & MAGIC_CONST_15) != MAGIC_CONST_15)) 
                {
                    if ((data_temp3 >> 12) & 1)
                    {
                        data_temp3 = -512;
                    }
                    else
                    {
                        data_temp3 = 511;
                    }
                }
                data_imag2 = data_temp3;

                data_real = *(fft_l_1024 + index1);
                data_imag = *(fft_q_1024 + index1);

                if ( MAGIC_CONST_5 == l )
                {
                    //improve SNR further by clip
                    //do clipping only once to assure signal not to overflow
		     //real part of No.1 operand
                    data_temp3 = data_real + data_real2;
		    //exceed limit of 10 bits
                    if (((data_temp3 >> 10) & 1) != ((data_temp3 >> 9) & 1)) 
                    {
                        if ((data_temp3 >> 10) & 1)
                        {
                            *(fft_l_1024 + index1) = -512;
                        }
                        else
                        {
                            *(fft_l_1024 + index1) = 511;
                        }
                    }
                    else
                    {
                        *(fft_l_1024 + index1) = data_temp3;
                    }
		    //imaginary part of No.1 operand
                    data_temp3 = data_imag + data_imag2; 
		    //exceed limit of 10 bits
                    if (((data_temp3 >> 10) & 1) != ((data_temp3 >> 9) & 1)) 
                    {
                        if ((data_temp3 >> 10) & 1)
                        {
                            *(fft_q_1024 + index1) = -512;
                        }
                        else
                        {
                            *(fft_q_1024 + index1) = 511;
                        }
                    }
                    else
                    {
                        *(fft_q_1024 + index1) = data_temp3;
                    }

		    //real part of No.2 operand
                    data_temp3 = data_real - data_real2; 
		    //exceed limit of 10 bits
                    if (((data_temp3 >> 10) & 1) != ((data_temp3 >> 9) & 1)) 
                    {
                        if ((data_temp3 >> 10) & 1)
                        {
                            *(fft_l_1024 + index2) = -512;
                        }
                        else
                        {
                            *(fft_l_1024 + index2) = 511;
                        }
                    }
                    else
                    {
                        *(fft_l_1024 + index2) = data_temp3;
                    }
		    //imaginary part of No.2 operand
                    data_temp3 = data_imag - data_imag2;

		    //exceed limit of 10 bits
                    if (((data_temp3 >> 10) & 1) != ((data_temp3 >> 9) & 1)) 
                    {
                        if ((data_temp3 >> 10) & 1)
                        {
                            *(fft_q_1024 + index2) = -512;
                        }
                        else
                        {
                            *(fft_q_1024 + index2) = 511;
                        }
                    }
                    else
                    {
                        *(fft_q_1024 + index2) = data_temp3;
                    }
                }
                else
                {
                    //divide by 2 to reduce previous quantity error
                    *(fft_l_1024 + index1) = (data_real + data_real2) >> 1;
                    *(fft_q_1024 + index1) = (data_imag + data_imag2) >> 1;
                    *(fft_l_1024 + index2) = (data_real - data_real2) >> 1;
                    *(fft_q_1024 + index2) = (data_imag - data_imag2) >> 1;
                }

                index1 += 2 * number_wncoeff;
            }
            //butterfly

        }
    }
}


