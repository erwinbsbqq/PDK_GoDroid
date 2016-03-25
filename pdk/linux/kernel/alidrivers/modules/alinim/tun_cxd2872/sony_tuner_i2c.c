/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2012-03-28 01:40:32 #$
  File Revision : $Revision:: 4945 $
------------------------------------------------------------------------------*/


#include "sony_i2c.h"
#include "sony_tuner_ascot3.h"
#include "sony_stdlib.h"        /* for memcpy */

#define BURST_WRITE_MAX 128 /* Max length of burst write */

sony_result_t cxd2872_i2c_CommonReadRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t* pData, uint32_t size)
{
    sony_result_t result = SONY_RESULT_OK;
    // sony_demod_t *pDemod = (sony_demod_t *)pI2c->pDemod;

    SONY_TRACE_I2C_ENTER("sony_i2c_CommonReadRegister");

    if(!pI2c){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    pData[0] = subAddress;
    CXD2872_PRINTF("\n i2c_type_id = 0x%x, addr = 0x%x \n",pI2c->i2c_type_id,deviceAddress);
    if ( SUCCESS != nim_i2c_write_read(pI2c->i2c_type_id, deviceAddress, pData, 1, size) )
     result = SONY_RESULT_ERROR_I2C;
    SONY_TRACE_I2C_RETURN(result);
}

sony_result_t cxd2872_i2c_CommonWriteRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, const uint8_t* pData, uint32_t size)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t buffer[BURST_WRITE_MAX + 1];
   // sony_demod_t *pDemod = (sony_demod_t *)pI2c->pDemod;

    SONY_TRACE_I2C_ENTER("sony_i2c_CommonWriteRegister");

    if(!pI2c){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    if(size > BURST_WRITE_MAX){
        /* Buffer is too small... */
        //CXD2837_LOG(pDemod, "s%(): buffer[%d] is too small: pData[%d]\r\n", __FUNCTION__, BURST_WRITE_MAX, size);
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    buffer[0] = subAddress;
    sony_memcpy(&(buffer[1]), pData, size);

	CXD2872_PRINTF("\n i2c_type_id = 0x%x, addr = 0x%x \n",pI2c->i2c_type_id,deviceAddress);

	if ( SUCCESS != nim_i2c_write(pI2c->i2c_type_id, deviceAddress, buffer, size+1) )
        result = SONY_RESULT_ERROR_I2C;
    
    //CXD2837_LOG_I2C(pDemod, result, 1, deviceAddress, buffer, size+1);
    
    SONY_TRACE_I2C_RETURN(result);
}

sony_result_t cxd2872_i2c_CommonWriteOneRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_I2C_ENTER("sony_i2c_CommonWriteOneRegister");

    if(!pI2c){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    result = pI2c->WriteRegister(pI2c, deviceAddress, subAddress, &data, 1);
    SONY_TRACE_I2C_RETURN(result);
}

/* For Read-Modify-Write */
sony_result_t cxd2872_i2c_SetRegisterBits(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data, uint8_t mask)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_I2C_ENTER("sony_i2c_SetRegisterBits");

    if(!pI2c){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    if(mask == 0x00){
        /* Nothing to do */
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    }
    
    if(mask != 0xFF){
        uint8_t rdata = 0x00;
        result = pI2c->ReadRegister(pI2c, deviceAddress, subAddress, &rdata, 1);
        if(result != SONY_RESULT_OK){ SONY_TRACE_I2C_RETURN(result); }
        data = (uint8_t)((data & mask) | (rdata & (mask ^ 0xFF)));
    }

    result = pI2c->WriteOneRegister(pI2c, deviceAddress, subAddress, data);
    SONY_TRACE_I2C_RETURN(result);
}



