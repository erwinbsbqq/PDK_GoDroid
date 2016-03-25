/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File: trng.h
*   
*    Description: this file is used to define some interface for 
      true ramdon number generator

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef  _TRNG_H_
#define  _TRNG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define ALI_TRNG_MAX_GROUP 16
#define ALI_TRNG_64BITS_SIZE 8

/*****************************************************************************
 * Function: trng_generate_byte
 * Description: 
 *    This function is used to get 1 byte TRNG data from driver.
 * Input: 
 *		None
 * Output: 
 *		UINT8 *data:  Random data buffer.
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE trng_generate_byte(UINT8 *data);
/*****************************************************************************
 * Function: trng_generate_byte
 * Description: 
 *    This function is used to get 64 bits TRNG data from driver.
 * Input: 
 *		None
 * Output: 
 *		UINT8 *data:  Random data buffer.
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE trng_generate_64bits(UINT8 *data);
/*****************************************************************************
 * Function: trng_generate_byte
 * Description: 
 *    This function is used to get several groups 64 bits TRNG data from driver.
 * Input: 
 *		UINT32 n: Group count wants to get. Recommend n value is 8. Size is n*8Bytes. 
 * Output: 
 *		UINT8 *data:  Random data buffer.
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE trng_get_64bits(UINT8 *data,UINT32 n);

void lld_trng_m36f_callee( UINT8 *msg );

#ifdef __cplusplus
}
#endif

#endif  /*_TRNG_H_*/
