/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2012-03-28 01:40:32 #$
  File Revision : $Revision:: 4945 $
------------------------------------------------------------------------------*/

#include "sony_i2c.h"

#include "sony_stdlib.h" /* for memcpy */
#include "sony_demod.h"
#include "nim_cxd2838.h"

#define BURST_WRITE_MAX 128 /* Max length of burst write */

sony_result_t cxd2838_i2c_CommonReadRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t* pData, uint32_t size)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_t *pDemod = (sony_demod_t *)pI2c->pDemod;

    SONY_TRACE_I2C_ENTER("sony_i2c_CommonReadRegister");

    if(!pI2c){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    pData[0] = subAddress;
    if ( SUCCESS != i2c_write_read(pI2c->i2c_type_id, deviceAddress, pData, 1, size) )
     result = SONY_RESULT_ERROR_I2C;
    
    CXD2838_LOG_I2C(pDemod, result, 1, deviceAddress, &subAddress, 1);
    CXD2838_LOG_I2C(pDemod, result, 0, deviceAddress, pData, size);

    //osal_mutex_unlock(pDemod->i2c_mutex_id);
    
    SONY_TRACE_I2C_RETURN(result);
}

sony_result_t cxd2838_i2c_CommonWriteRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, const uint8_t* pData, uint32_t size)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t buffer[BURST_WRITE_MAX + 1];
    sony_demod_t *pDemod = (sony_demod_t *)pI2c->pDemod;

    SONY_TRACE_I2C_ENTER("sony_i2c_CommonWriteRegister");

    if(!pI2c){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    if(size > BURST_WRITE_MAX){
        /* Buffer is too small... */
        CXD2838_LOG(pDemod, "s%(): buffer[%d] is too small: pData[%d]\r\n", __FUNCTION__, BURST_WRITE_MAX, size);
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    //osal_mutex_lock(pDemod->i2c_mutex_id, OSAL_WAIT_FOREVER_TIME);
    
    buffer[0] = subAddress;
    sony_memcpy(&(buffer[1]), pData, size);

    /* send the new buffer */
    //result = pI2c->Write(pI2c, deviceAddress, buffer, size+1, SONY_I2C_START_EN | SONY_I2C_STOP_EN);
    if ( SUCCESS != i2c_write(pI2c->i2c_type_id, deviceAddress, buffer, size+1) )
        result = SONY_RESULT_ERROR_I2C;
    
    CXD2838_LOG_I2C(pDemod, result, 1, deviceAddress, buffer, size+1);

    //osal_mutex_unlock(pDemod->i2c_mutex_id);
    
    SONY_TRACE_I2C_RETURN(result);
}

sony_result_t cxd2838_i2c_CommonWriteOneRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data)
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
sony_result_t cxd2838_i2c_SetRegisterBits(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data, uint8_t mask)
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


sony_result_t cxd2838_i2c_TunerGateway(void* nim_dev_priv, UINT8	tuner_address , UINT8* wdata , int wlen , UINT8* rdata , int rlen)
{
    uint8_t buffer[BURST_WRITE_MAX + 2]={0};//Protect against overflow: GW Sub Adress (1) + Tuner address (1)
    sony_demod_t *pDemod = (sony_demod_t*)nim_dev_priv;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_I2C_ENTER("sony_i2c_TunerGateway");

    if( (0 == wlen) && (0 == rlen) )
    {
        CXD2838_LOG(pDemod, "s%(): No data for read/write.\r\n", __FUNCTION__);
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
	if ( wlen != 0 ) 
	{
	    
		if(wlen > BURST_WRITE_MAX)    //Protect against overflow: GW Sub Adress (1) + Tuner address (1)
		{
			CXD2838_LOG(pDemod, "s%(): buffer[%d] is too small: Write wdata[%d]\r\n", __FUNCTION__, BURST_WRITE_MAX, wlen);
			SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
		}
		CXD2838_PRINTF("\n write to tuner \n");
		//osal_mutex_lock(pDemod->i2c_mutex_id, OSAL_WAIT_FOREVER_TIME);
		//Payload: [GWSub] [TunerAddress] [Data0] ... [DataN]
		//{S} [Addr] [GW Sub] [TunerAddr+Write]
		buffer[0] = 0x09;   //The tuner gateway register of Sony demod. 
		buffer[1] = tuner_address & 0xFE;   //[TunerAddr+Write]
		sony_memcpy (&(buffer[2]), wdata, wlen);

		if ( SUCCESS != i2c_write(pDemod->pI2c->i2c_type_id, pDemod->i2cAddressSLVT, buffer, wlen+2))
			result = SONY_RESULT_ERROR_I2C;
		CXD2838_LOG_I2C(pDemod, result, 1, pDemod->i2cAddressSLVT, buffer, wlen+2);

		SONY_TRACE_I2C_RETURN(result);
	}

	if ( rlen != 0 ) //read from tuner
	{
	    CXD2838_PRINTF("\n read from tuner \n");
		if(rlen > BURST_WRITE_MAX)
		{
			CXD2838_LOG(pDemod, "s%(): buffer[%d] is too small: Read rdata[%d]\r\n", __FUNCTION__, BURST_WRITE_MAX, rlen);
			SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
		}
		//{S} [Addr] [GW Sub] [TunerAddr+Read]
		//hostI2C->Write (hostI2C, pDemod->i2cAddress, data, sizeof (data), SONY_I2C_START_EN)
		//{SR} [Addr+Read] [Read0] [Read1] ... [ReadN] {P} 
		//hostI2C->Read (hostI2C, pI2c->gwAddress, pData, size, SONY_I2C_START_EN | SONY_I2C_STOP_EN);

		//osal_mutex_lock(pDemod->i2c_mutex_id, OSAL_WAIT_FOREVER_TIME);
		//{S} [Addr] [GW Sub] [TunerAddr+Read]
		buffer[0] = 0x09;   //The tuner gateway register of Sony demod. 
		buffer[1] = tuner_address | 0x1;   //[TunerAddr+Read]

		CXD2838_LOG_I2C(pDemod, result, 1, pDemod->i2cAddressSLVT, buffer, 2);
		if ( SUCCESS != i2c_write_read(pDemod->pI2c->i2c_type_id, pDemod->i2cAddressSLVT, buffer, 2, rlen) )
			result = SONY_RESULT_ERROR_I2C;
		CXD2838_LOG_I2C(pDemod, result, 0, pDemod->i2cAddressSLVT, buffer, rlen);
		if (SONY_RESULT_OK == result )
			sony_memcpy(rdata, buffer, rlen);
		SONY_TRACE_I2C_RETURN(result);
	}
	SONY_TRACE_I2C_RETURN(result);
}




