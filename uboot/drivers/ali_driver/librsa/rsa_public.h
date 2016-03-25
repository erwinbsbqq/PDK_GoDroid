/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File: rsa_public.h
*   
*    Description: 
*       
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _RSA_PUBLIC_
#define _RSA_PUBLIC_


#ifdef __cplusplus
extern "C" {
#endif

#include <ali/basic_types.h>
#include <ali/ali_param.h>
                        
/* Macros. */
#define LOW_HALF(x) ((x) & MAX_ALI_HALF_DIGIT)
#define HIGH_HALF(x) (((x) >> ALI_HALF_DIGIT_BITS) & MAX_ALI_HALF_DIGIT)
#define TO_HIGH_HALF(x) (((UINT32)(x)) << ALI_HALF_DIGIT_BITS)
#define DIGIT_MSB(x) (unsigned int)(((x) >> (ALI_DIGIT_BITS - 1)) & 1)
#define DIGIT_2MSB(x) (unsigned int)(((x) >> (ALI_DIGIT_BITS - 2)) & 3)
#define ALI_ASSIGN_DIGIT(a, b, digits) {ali_assign_zero (a, digits); a[0] = b;}



/* Length of digit in bits */
#define ALI_DIGIT_BITS 32
#define ALI_HALF_DIGIT_BITS 16
/* Length of digit in bytes */
#define ALI_DIGIT_LEN (ALI_DIGIT_BITS / 8)
/* Maximum length in digits */
#define MAX_ALI_DIGITS ((ALI_RSA_MODULUS_LEN + ALI_DIGIT_LEN - 1) \
                / ALI_DIGIT_LEN + 1)
/* Maximum digits */
#define MAX_ALI_DIGIT 0xffffffff
#define MAX_ALI_HALF_DIGIT 0xffff



#define ALIASIX_HASH_DIGEST_LEN               32
#define ALIASIX_RSA_COMPARE_LEN               256

RET_CODE ali_rsa_result_compare(UINT8 *rsa_result, void *sha_result);
RET_CODE ali_rsapublicfunc(UINT8 *output , UINT32 *outputlen,
                      UINT8 *input, UINT32 inputlen, 
                      R_RSA_PUBLIC_KEY *publickey);

#define DOUBLE_FOR(i,j,len,step)  for((i)=0,(j)=(len);\
                                   (i)<(len)&&(j>0);\
                                    (i)+=(step),(j)-=(step))

#ifdef __cplusplus
 }
#endif

#endif /*_RSA_PUBLIC_*/
