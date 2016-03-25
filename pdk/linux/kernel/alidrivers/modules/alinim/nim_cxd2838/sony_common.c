/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2012-06-12 17:25:39 #$
  File Revision : $Revision:: 5553 $
------------------------------------------------------------------------------*/

#include "sony_common.h"

/* Utility function */
#define MASKUPPER(n) (((n) == 0) ? 0 : (0xFFFFFFFFU << (32 - (n))))
#define MASKLOWER(n) (((n) == 0) ? 0 : (0xFFFFFFFFU >> (32 - (n))))
/* Convert N (<32) bit 2's complement value to 32 bit signed value */
int32_t cxd2838_Convert2SComplement(uint32_t value, uint32_t bitlen)
{
    if((bitlen == 0) || (bitlen >= 32)){
        return (int32_t)value;
    }

    if(value & (uint32_t)(1 << (bitlen - 1))){
        /* minus value */
        return (int32_t)(MASKUPPER(32 - bitlen) | value);
    }
    else{
        /* plus value */
        return (int32_t)(MASKLOWER(bitlen) & value);
    }
}
uint32_t sony_BitSplitFromByteArray(uint8_t *pArray, uint32_t startBit, uint32_t bitNum)
{
    uint32_t value = 0;
    uint8_t *pArrayRead;
    uint8_t bitRead;
    uint32_t lenRemain;

    if(!pArray){return 0;}
    if((bitNum == 0) || (bitNum > 32)){return 0;}

    pArrayRead = pArray + (startBit/8);
    bitRead = (uint8_t)(startBit % 8);
    lenRemain = bitNum;

    if(bitRead != 0){
        if(((int32_t)lenRemain) <= 8 - bitRead){
            value = (*pArrayRead) >> ((8 - bitRead) - lenRemain);
            lenRemain = 0;
        }else{
            value = *pArrayRead++;
            lenRemain -= 8 - bitRead;
        }
    }

    while(lenRemain > 0){
        if(lenRemain < 8){
            value <<= lenRemain;
            value |= (*pArrayRead++ >> (8 - lenRemain));
            lenRemain = 0;
        }else{
            value <<= 8;
            value |= (uint32_t)(*pArrayRead++);
            lenRemain -= 8;
        }
    }

    value &= MASKLOWER(bitNum);

    return value;
}

