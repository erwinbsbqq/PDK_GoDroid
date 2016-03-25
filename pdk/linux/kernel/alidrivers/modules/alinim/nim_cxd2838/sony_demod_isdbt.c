/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : 2014-09-02
  File Revision : 1.0.16.0
------------------------------------------------------------------------------*/

#include "sony_demod_isdbt.h"
#include "sony_math.h"
#include "sony_stdlib.h"
#include "sony_demod_isdbt_monitor.h"

#include "sony_demod.h"
#include "sony_demod_isdbt.h"


#define sony_i2c_SetRegisterBits   				cxd2838_i2c_SetRegisterBits
#define sony_demod_TuneEnd        				cxd2838_demod_TuneEnd
#define sony_demod_ChipID          				cxd2838_demod_ChipID
#define sony_demod_SetAndSaveRegisterBits 		cxd2838_demod_SetAndSaveRegisterBits
#define sony_demod_SetTsClockModeAndFreq        cxd2838_demod_SetTsClockModeAndFreq



/*------------------------------------------------------------------------------
  Static Function Prototypes
------------------------------------------------------------------------------*/
/**
 @brief Configure the demodulator from any state to Sleep state.
        This is used as a demodulator reset, all demodulator configuration
        settings will be lost.
*/
static sony_result_t XtoSLT(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from Shutdown to Sleep state
*/
static sony_result_t SDtoSLT(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any Sleep state to Shutdown
*/
static sony_result_t SLTtoSD(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any Sleep state to EWS
*/
static sony_result_t SLTtoEWS(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any EWS state to Sleep
*/
static sony_result_t EWStoSLT(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any Sleep state to Active
*/
static sony_result_t SLTtoAT(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any Active state to Sleep
*/
static sony_result_t ATtoSLT(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any EWS state to Active
*/
static sony_result_t EWStoAT(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any Active state to EWS
*/
static sony_result_t ATtoEWS(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any EWS state to EWS
*/
static sony_result_t EWStoEWS(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any Active state to Active
*/
static sony_result_t ATtoAT(sony_demod_t* pDemod);


/**
 @brief set Each BW common setting

 @param  pDemod Reference to memory allocated for the demodulator instance. The create 
        function will setup this demodulator instance.
*/
static sony_result_t setCommonDemodParam(sony_demod_t* pDemod);

/**
 @brief 8MHzBW setting

 @param  pDemod Reference to memory allocated for the demodulator instance. The create 
        function will setup this demodulator instance.
*/
static sony_result_t setDemodParamFor8Mhz(sony_demod_t* pDemod);

/**
 @brief 7MHzBW setting

 @param  pDemod Reference to memory allocated for the demodulator instance. The create 
        function will setup this demodulator instance.
*/
static sony_result_t setDemodParamFor7Mhz(sony_demod_t* pDemod);

/**
 @brief 6MHzBW setting

 @param  pDemod Reference to memory allocated for the demodulator instance. The create 
        function will setup this demodulator instance.
*/
static sony_result_t setDemodParamFor6Mhz(sony_demod_t* pDemod);

/**
 @brief Clear the demodulator configuration memory table.  Use
        this API to empty the table of previous entries.  This is called 
        automatically in the sony_integ_Initialize API.
*/
static sony_result_t clearConfigMemory(sony_demod_t* pDemod);

/**
 @brief Iterate through the demodulator configuration memory table and write
        each entry to the device.  This is called automatically during a 
        transition from ::SONY_DEMOD_STATE_SHUTDOWN to 
        ::SONY_DEMOD_STATE_SLEEP.
*/
static sony_result_t loadConfigMemory (sony_demod_t * pDemod);

/**
 @brief Save an entry into the demodulator configuration memory table.
 
 @param pDemod The demodulator instance.
 @param slaveAddress Slave address of configuration setting
 @param bank Demodulator bank of configuration setting
 @param registerAddress Register address of configuration setting
 @param value The value being written to this register
 @param mask The bit mask used on the register
*/
static sony_result_t setConfigMemory (sony_demod_t * pDemod, 
                                      uint8_t slaveAddress, 
                                      uint8_t bank, 
                                      uint8_t registerAddress,
                                      uint8_t value,
                                      uint8_t bitMask);


/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
sony_result_t cxd2838_demod_Create(sony_demod_t*     pDemod,
                                sony_demod_xtal_t xtalFreq,
                                uint8_t           i2cAddress,
                                sony_i2c_t*       pDemodI2c)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_demod_Create");
    
    if ((!pDemod) || (!pDemodI2c)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Check for invalid crystal selection */
    if ((xtalFreq != SONY_DEMOD_XTAL_20500KHz) &&
        (xtalFreq != SONY_DEMOD_XTAL_24000KHz) &&
        (xtalFreq != SONY_DEMOD_XTAL_41000KHz)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    //sony_memset(pDemod, 0, sizeof(sony_demod_t));

    pDemod->xtalFreq       = xtalFreq;
    pDemod->i2cAddressSLVT = i2cAddress;
    pDemod->i2cAddressSLVX = i2cAddress + 4;
    pDemod->pI2c           = pDemodI2c;
    pDemod->state          = SONY_DEMOD_STATE_UNKNOWN;
    pDemod->serialTsClkFreq = SONY_DEMOD_SERIAL_TS_CLK_MID_FULL;
    pDemod->serialTsClockModeContinuous = 1;

    SONY_TRACE_RETURN(result);
}
sony_result_t sony_demod_Initialize(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_demod_Initialize");
    
    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pDemod->serialTsClkFreq = SONY_DEMOD_SERIAL_TS_CLK_MID_FULL;
    pDemod->serialTsClockModeContinuous = 1;

    /* Initialize causes demodulator register reset */
    result = XtoSLT(pDemod);
    if (result != SONY_RESULT_OK) {
        pDemod->state = SONY_DEMOD_STATE_INVALID;
        SONY_TRACE_RETURN(result);
    }

    pDemod->state = SONY_DEMOD_STATE_SLEEP;

    result = sony_demod_ChipID(pDemod, &(pDemod->chipId));
    if (result != SONY_RESULT_OK) {
        pDemod->state = SONY_DEMOD_STATE_INVALID;
        SONY_TRACE_RETURN (result);
    }

    if (pDemod->chipId != SONY_DEMOD_CHIP_ID_CXD2838) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }
    CXD2838_PRINTF("\n read demod chipid %d\n",pDemod->chipId);
    /* Clear Config memory in Initialize API */
    result = clearConfigMemory(pDemod);
    if (result != SONY_RESULT_OK) {
        pDemod->state = SONY_DEMOD_STATE_INVALID;
        SONY_TRACE_RETURN(result);
    }
    
    pDemod->state = SONY_DEMOD_STATE_SLEEP;
    
    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_Sleep(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_demod_Sleep");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    switch (pDemod->state) {
    case SONY_DEMOD_STATE_SHUTDOWN:
        result = SDtoSLT(pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
        result = loadConfigMemory(pDemod);
        break;
    case SONY_DEMOD_STATE_ACTIVE:
        result = ATtoSLT(pDemod);
        break;
    case SONY_DEMOD_STATE_EWS:
        result = EWStoSLT(pDemod);
        break;
    case SONY_DEMOD_STATE_SLEEP:
        /* do nothing */
        break;
    case SONY_DEMOD_STATE_UNKNOWN:
    default:
        result = SONY_RESULT_ERROR_SW_STATE;
        break;
    }

    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    pDemod->state = SONY_DEMOD_STATE_SLEEP;

    SONY_TRACE_RETURN(result);
}

 sony_result_t cxd2838_demod_Shutdown(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_demod_Shutdown");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    switch (pDemod->state) {
    case SONY_DEMOD_STATE_SHUTDOWN:
        /* do nothing */
        break;
    case SONY_DEMOD_STATE_ACTIVE:
        result = ATtoSLT(pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
        result = SLTtoSD(pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
        break;
    case SONY_DEMOD_STATE_EWS:
        result = EWStoSLT(pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
        result = SLTtoSD(pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
        break;
    case SONY_DEMOD_STATE_SLEEP:
        result = SLTtoSD(pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
        break;
    case SONY_DEMOD_STATE_UNKNOWN:
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    pDemod->state = SONY_DEMOD_STATE_SHUTDOWN;

    SONY_TRACE_RETURN(result);
}

 sony_result_t cxd2838_demod_SoftReset(sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_demod_SoftReset");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS) &&
        (pDemod->state != SONY_DEMOD_STATE_SLEEP)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    /* SW reset                              | SLV-T, 00h , FEh , 01h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xFE, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_TuneEnd(sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_demod_TuneEnd");
    
    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    /* SW reset                              | SLV-T, 00h , FEh , 01h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xFE, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    if (pDemod->state != SONY_DEMOD_STATE_EWS) {
        /* enable TS output                      | SLV-T, 00h , C3h , 00h WriteEnable TS output */
        result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC3, 0x00);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }
    
    SONY_TRACE_RETURN(result);
}

 sony_result_t cxd2838_demod_SetConfig(sony_demod_t*       pDemod,
                                   sony_demod_config_t config,
                                   uint32_t            value)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_demod_SetConfig");

    /* NULL Check */
    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS) &&
        (pDemod->state != SONY_DEMOD_STATE_SLEEP)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    switch (config) {
    case SONY_DEMOD_CONFIG_PARALLEL_SEL:
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
        /* slave    Bank    Addr    Bit    default    Name
         * ---------------------------------------------------
         * <SLV-T>  00h     C4h     [7]    1'b0       OSERIALEN
         *
         * 0: TS parallel (default), 1: TS serial
         */
        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xC4, (uint8_t) (value ? 0x00 : 0x80), 0x80);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }

        break;
    case SONY_DEMOD_CONFIG_SER_DATA_ON_MSB:
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
        /* slave    Bank    Addr    Bit    default    Name
         * ---------------------------------------------------
         * <SLV-T>  00h     C4h     [3]    1'b1       OSEREXCHGB7
         *
         * 0: TSDATA[0], 1: TSDATA[7] (default)
         */
        
        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xC4, (uint8_t) (value ? 0x08 : 0x00), 0x08);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }

        break;
    case SONY_DEMOD_CONFIG_OUTPUT_SEL_MSB:
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
        /* slave    Bank    Addr    Bit    default    Name
         * ---------------------------------------------------
         * <SLV-T>  00h     C4h     [4]    1'b0       OWFMT_LSB1STON
         *
         * bit order of TSDATA under TS parallel
         * 0: MSB on TSDATA[7] (default), 1:MSB on TSDATA[0]
         * bit order of TSDATA under TS serial
         * 0: MSB first (default), 1:LSB first
         */

        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xC4, (uint8_t) (value ? 0x00 : 0x10), 0x10);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }

        break;
    case SONY_DEMOD_CONFIG_TSVALID_ACTIVE_HI:
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
        /* slave    Bank    Addr    Bit    default    Name
         * ---------------------------------------------------
         * <SLV-T>  00h     C5h     [1]    1'b0       OWFMT_VALINV
         *
         * 0: active high, 1: active low
         */

        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xC5, (uint8_t) (value ? 0x00 : 0x02), 0x02);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }

        break;
    case SONY_DEMOD_CONFIG_TSSYNC_ACTIVE_HI:
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
        /* slave    Bank    Addr    Bit    default    Name
         * ---------------------------------------------------
         * <SLV-T>  00h     C5h     [2]    1'b0       OWFMT_STINV
         *
         * 0: active high, 1: active low
         */

        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xC5, (uint8_t) (value ? 0x00 : 0x04), 0x04);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }

        break;
    case SONY_DEMOD_CONFIG_TSERR_ACTIVE_HI:
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
        /* slave    Bank    Addr    Bit    default    Name
         * ---------------------------------------------------
         * <SLV-T>  00h     CBh     [0]    1'b0       OWFMT_ERRINV
         *
         * 0: active high(default), 1: active low
         */

        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xCB, (uint8_t) (value ? 0x00 : 0x01), 0x01);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }

        break;
    case SONY_DEMOD_CONFIG_LATCH_ON_POSEDGE:
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
        /* slave    Bank    Addr    Bit    default    Name
         * ---------------------------------------------------
         * <SLV-T>  00h     C5h     [0]    1'b1       OWFMT_CKINV
         *
         * 0: strobe TSDATA at falling edge, 1: strobe TSDATA at rising edge (default)
         */

        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xC5, (uint8_t) (value ? 0x01 : 0x00), 0x01);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }

        break;
    case SONY_DEMOD_CONFIG_TSCLK_CONT:
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }

        /* Store the serial clock mode */
        pDemod->serialTsClockModeContinuous = (uint8_t)value;

        break;
    case SONY_DEMOD_CONFIG_TSCLK_MASK:
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }

        /* slave    Bank    Addr    Bit    default      Name
         * ---------------------------------------------------
         * <SLV-T>  00h     C6h     [4:0]    5'b00000   OWFMT_CKDISABLE
         *
         * 0: active, 1: disabled
         * [4] : 1st byte (TS sync) [3] : 2nd . 4th byte (TS header)
         * [2] : 5th . 188th byte (TS payload) [1] : 189th . 224th byte (TS parity)
         * [0] : TS packet gap
         */

        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xC6, (uint8_t)value,  0x1F);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }

        break;
    case SONY_DEMOD_CONFIG_TSVALID_MASK:
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }

        /* slave    Bank    Addr    Bit    default     Name
         * ---------------------------------------------------
         * <SLV-T>  00h     C8h     [4:0]    5'b00011  OWFMT_VALDISABLE
         *
         * 0: active, 1: disabled
         * [4] : 1st byte (TS sync) [3] : 2nd . 4th byte (TS header)
         * [2] : 5th . 188th byte (TS payload) [1] : 189th . 224th byte (TS parity)
         * [0] : TS packet gap
         */

        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xC8, (uint8_t)value, 0x1F);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }

        break;
    case SONY_DEMOD_CONFIG_TSERR_MASK:
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }

        /* slave    Bank    Addr    Bit    default     Name
         * ---------------------------------------------------
         * <SLV-T>  00h     C9h     [4:0]    5'b00000  OWFMT_ERRDISABLE
         *
         * 0: active, 1: disabled
         * [4] : 1st byte (TS sync) [3] : 2nd . 4th byte (TS header)
         * [2] : 5th . 188th byte (TS payload) [1] : 189th . 224th byte (TS parity)
         * [0] : TS packet gap
         */

        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xC9, (uint8_t)value, 0x1F);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }

        break;
    case SONY_DEMOD_CONFIG_TSCLK_CURRENT_10mA:
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }

        /* slave    Bank    Addr    Bit    default    Name
         * --------------------------------------------------------
         * <SLV-T>  00h     83h     [0]    1'b0       OREG_TSCLK_C
         */

        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0x83, (uint8_t) (value ? 0x01 : 0x00), 0x01);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        break;
    case SONY_DEMOD_CONFIG_TS_CURRENT_10mA:
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }

        /* slave    Bank    Addr    Bit    default    Name
         * --------------------------------------------------------
         * <SLV-T>  00h     83h     [1]    1'b0       OREG_TSSYNC_C
         * <SLV-T>  00h     83h     [2]    1'b0       OREG_TSVALID_C
         */
        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0x83, (uint8_t) (value ? 0x06 : 0x00), 0x06);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* slave    Bank    Addr    Bit    default    Name
         * --------------------------------------------------------
         * <SLV-T>  00h     84h     [7:0]  8'h00      OREG_TSDATA_C
         */
        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0x84, (uint8_t) (value ? 0xFF : 0x00), 0xFF);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        {
            uint8_t data;

            /* Set SLV-X Bank : 0x00 */
            if (pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
            }

            /* slave    Bank    Addr    Bit    default   Value        Name        
             * -------------------------------------------------------------------
             * <SLV-X>  00h     A5h     [3:0]   8'h00     8'h04     OREG_GPIP2_SEL  
             */
            result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0xA5, &data, 1); 
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN(result);
            }

            /* Set GPIO2 Drivability, only if in TSERR mode */
            if ((data & 0x0F) == SONY_DEMOD_GPIO_MODE_TS_ERROR) {
                /* slave    Bank    Addr    Bit    default    Name
                 * --------------------------------------------------------
                 * <SLV-X>  00h     85h     [2]    1'b0       OREG_GPIO2_C
                 */
                result = sony_i2c_SetRegisterBits(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x85, (uint8_t) (value ? 0x04 : 0x00), 0x04);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN(result);
                }
            }
        }

        break;
    case SONY_DEMOD_CONFIG_TS_SERIAL_CLK_FREQ:
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            /* This api is accepted in SLEEP state only */
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
        }
        if ((value < 1) || (value > 6)) {
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_RANGE);
        }

        /* Store the clock frequency mode */
        pDemod->serialTsClkFreq = (sony_demod_serial_ts_clk_t) value;
        break;
    case SONY_DEMOD_CONFIG_TS_BACKWARDS_COMPATIBLE:
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            /* This api is accepted in SLEEP state only */
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
        }
        
        /* Slave    Bank    Addr    Bit      Default    Name
         * -----------------------------------------------------------
         * <SLV-T>  00h     D3h     [0]      1'b1       OTSRATECTRLOFF
         *
         * OTSRATECTRLOFF must be 1.
         */
        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xD3, 0x01, 0x01);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* Slave    Bank    Addr    Bit      Default    Name
         * ------------------------------------------------------
         * <SLV-T>  00h     DEh     [0]      1'b0       OTSIN_OFF
         */
        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xDE, (uint8_t)(value ? 0x01 : 0x00), 0x01);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        break;
    case SONY_DEMOD_CONFIG_TSIF_SDPR:
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            /* This api is accepted in SLEEP state only */
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
        }

        /* Check the new clock frequency.
         * Valid range = 320KHz (2.56Mbps) to 16MHz (128Mbps) */
        if ((value <= 320) || (value > 16000)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        }
        
        {
            uint32_t val = 0x00;
            uint8_t data[3];
            uint32_t uvalue = (uint32_t) value;

            /*
             * OREG_TSIF_WR_SMOOTH_DP = (X(MHz) / TSCLK(MHz)) * 2^16 ;
             * Xtal = 20.5MHz, 41MHz : X = 48.6875
             * Xtal = 24MHz          : X = 48
             * Notes: 48687.5 * (2^16) = 3190784000 
             *        48000   * (2^16) = 3145728000
             */
            if (pDemod->xtalFreq == SONY_DEMOD_XTAL_20500KHz ||
                pDemod->xtalFreq == SONY_DEMOD_XTAL_41000KHz) {
                val = (3190784000u + uvalue / 2) / uvalue;
            } else if (pDemod->xtalFreq == SONY_DEMOD_XTAL_24000KHz) {
                val = (3145728000u + uvalue / 2) / uvalue;
            } else {
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
            }

            data[0] = (uint8_t) ((val & 0xFF0000) >> 16);
            data[1] = (uint8_t) ((val & 0xFF00) >> 8);
            data[2] = (uint8_t) (val & 0xFF);
                
            /* Slave    Bank     Addr    Bit      Default    Value     Name
             * ----------------------------------------------------------------------------------
             * <SLV-T>   66h     1Bh     [7:0]    8'h00      8'hxx     OREG_WR_SMOOTH_DP_ISDBT[23:16]
             * <SLV-T>   66h     1Ch     [7:0]    8'h00      8'hxx     OREG_WR_SMOOTH_DP_ISDBT[15:8]
             * <SLV-T>   66h     1Dh     [7:0]    8'h00      8'hxx     OREG_WR_SMOOTH_DP_ISDBT[7:0]
             */
            result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x66, 0x1B, data[0], 0xFF);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x66, 0x1C, data[1], 0xFF);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x66, 0x1D, data[2], 0xFF);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        }
        
        break;
    case SONY_DEMOD_CONFIG_TS_AUTO_RATE_ENABLE:
        if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
            /* This api is accepted in SLEEP state only */
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
        }

        /* slave    Bank    Addr    Bit    default    Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>  66h     1Ah     [0]      8'h01      8'h00     OREG_AUTO_RATE_EN_ISDBT
         */

        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x66, 0x1A, (uint8_t)(value ? 0x01 : 0x00), 0x01);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        
        break;
    case SONY_DEMOD_CONFIG_PWM_VALUE:
        /* Slave    Bank    Addr    Bit      Default    Value    Name
         * -------------------------------------------------------------------
         * <SLV-T>  00h     B7h     [0]      1'b0       1'b1     OREG_RFAGCSEL
         */
        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xB7, value? 0x01 : 0x00, 0x01);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }

        /* slave    Bank    Addr    Bit    default    Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>  00h     B2h     [3:0]    8'h00      8'h0x     OREG_GDA_VAL_RFAGC[11:8]
         * <SLV-T>  00h     B3h     [7:0]    8'h00      8'hxx     OREG_GDA_VAL_RFAGC[7:0]
         */
        {
            uint8_t data[2];
            data[0] = (uint8_t) (((uint16_t)value >> 8) & 0x0F);
            data[1] = (uint8_t) ((uint16_t)value & 0xFF);

            result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xB2, data[0], 0xFF);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN(result);
            }

            result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x00, 0xB3, data[1], 0xFF);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN(result);
            }
        }

        break;
    case SONY_DEMOD_CONFIG_IFAGCNEG:
        /* slave    Bank    Addr    Bit    default     Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   10h     CBh     [6]      8'h48      8'h08      OCTL_IFAGCNEG
         */
        result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x10, 0xCB, (uint8_t) (value ? 0x40 : 0x00), 0x40);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        break;
    case SONY_DEMOD_CONFIG_IFAGC_ADC_FS:
        {
            uint8_t data;

            if (value == 0) {
                data = 0x50; /* 1.4Vpp - Default */
            }
            else if (value == 1) {
                data = 0x39; /* 1.0Vpp */
            }
            else if (value == 2) {
                data = 0x28; /* 0.7Vpp */
            }
            else {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
            }

            /* Slave     Bank    Addr   Bit      Default    Name
             * -------------------------------------------------------------------
             * <SLV-T>   10h     CDh    [7:0]    8'h50      OCTL_IFAGC_TARGET[7:0]
             */
            result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x10, 0xCD, data, 0xFF);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        }
        break;
    case SONY_DEMOD_CONFIG_SPECTRUM_INV:
        /* Store the configured sense. */
        if (value == 0) {
            pDemod->confSense = SONY_DEMOD_SPECTRUM_NORMAL;
        } else {
            pDemod->confSense = SONY_DEMOD_SPECTRUM_INV;
        }

        break;
    case SONY_DEMOD_CONFIG_BERPER_PERIOD:
        {
            uint8_t data[2];

            data[0] = (uint8_t)((value & 0x00007F00) >> 8);
            data[1] = (uint8_t)(value & 0x000000FF);

            /*
             * <SLV-T>    60h    5Bh     [6:0]    OBER_CDUR_RSA[14:8]
             * <SLV-T>    60h    5Ch     [7:0]    OBER_CDUR_RSA[7:0]
             */
            result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x60, 0x5B, data[0], 0x7F);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN(result);
            }
            result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x60, 0x5C, data[1], 0xFF);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN(result);
            }
        }

        break;
    case SONY_DEMOD_CONFIG_GPIO_EWS_FLAG:
        {
            uint8_t data;

            if (value == 0) {
                data = 0x01;
            } else if (value == 1) {
                data = 0x80;
            } else if (value == 2) {
                data = 0x81;
            } else {
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_RANGE);
            }

            result = sony_demod_SetAndSaveRegisterBits(pDemod, pDemod->i2cAddressSLVT, 0x63, 0x3B, data, 0xFF);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN(result);
            }
        }
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN(result);
}

 sony_result_t cxd2838_demod_GPIOSetConfig(sony_demod_t*          pDemod,
                                       uint8_t                id, 
                                       uint8_t                enable,
                                       sony_demod_gpio_mode_t mode)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t gpioModeSelAddr;
    uint8_t gpioBitSel;

    SONY_TRACE_ENTER("sony_demod_GPIOSetConfig");

    if ((!pDemod) || (id > 2)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) && (pDemod->state != SONY_DEMOD_STATE_EWS) && 
        (pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_SHUTDOWN)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* TS Error only available on GPIO2 */
    if ((mode == SONY_DEMOD_GPIO_MODE_TS_ERROR) && (id != 2)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    gpioModeSelAddr = 0xA3 + id;
    gpioBitSel = 1 << id;
    
    /* Set SLV-X Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* Slave    Bank    Addr    Bit      Default   Name
     * -----------------------------------------------------------   
     * <SLV-X>  00h     A3h     [3:0]    4'h00     OREG_GPIO0_SEL
     * <SLV-X>  00h     A4h     [3:0]    4'h00     OREG_GPIO1_SEL
     * <SLV-X>  00h     A5h     [3:0]    4'h00     OREG_GPIO2_SEL
     */
    result = sony_i2c_SetRegisterBits(pDemod->pI2c, pDemod->i2cAddressSLVX, gpioModeSelAddr, (uint8_t)mode, 0x0F);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    {
        uint8_t enableHiZ;

        if (mode == SONY_DEMOD_GPIO_MODE_INPUT) {
            /* HiZ enabled when pin is GPI */
            enableHiZ = 0x01 << id;
        } else {
            /* HiZ determined by enable parameter */
            enableHiZ = enable ? 0x00 : (uint8_t)(0x01 << id);
        }

        /* Set HiZ setting for selected pin */
        /* Slave    Bank    Addr    Bit      Default    Name			 Meaning
         * -----------------------------------------------------------------------------------
         * <SLV-X>  00h     82h     [2:0]    3'b111     OREG_GPIO_HIZ    0: HiZ Off, 1: HiZ On
         */
        result = sony_i2c_SetRegisterBits(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x82, enableHiZ, gpioBitSel);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    /* Set drivability current for GPIO 2 */
    if (id == 2) {
        uint8_t drivability = 0x00; /* Default 8mA */
        
        /* If GPIO2 is set to TSERR, set to same drivability as TS Valid */
        if (mode == SONY_DEMOD_GPIO_MODE_TS_ERROR) {
            uint8_t data;
            /* slave    Bank    Addr    Bit    default      Name
             * -----------------------------------------------------------
             * <SLV-T>  00h     83h     [2]    1'b0         OREG_TSVALID_C
             */
            if (pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x83, &data, 1) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
            }        

            if (data & 0x04) {
                /* TS pins set to 10mA */
                drivability = 0x04; 
            }
        }

        /* slave    Bank    Addr    Bit    default     Name
         * --------------------------------------------------------
         * <SLV-X>  00h     85h     [2]    1'b0        OREG_GPIO2_C
         */
        result = sony_i2c_SetRegisterBits(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x85, drivability, 0x04);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

 sony_result_t cxd2838_demod_GPIORead(sony_demod_t* pDemod,
                                  uint8_t       id,
                                  uint8_t*      pValue)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0x00;
    
    SONY_TRACE_ENTER("sony_demod_GPIORead");

    if ((!pDemod) || (id > 2) || (!pValue)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) && (pDemod->state != SONY_DEMOD_STATE_EWS) && 
        (pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_SHUTDOWN)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-X Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*
     *  The register get the level at the pin.
     *
     *   slave    Bank    Addr    Bit    default      Name              Remarks
     *  ---------------------------------------------------------------------------------
     *   <SLV-X>  00h     A0h     [0]     1'b0       IREG_GPIO_IN[0]    GPIO0 pin setting
     *   <SLV-X>  00h     A0h     [1]     1'b0       IREG_GPIO_IN[1]    GPIO1 pin setting
     *   <SLV-X>  00h     A0h     [2]     1'b0       IREG_GPIO_IN[2]    GPIO2 pin setting
     *
     *  [meaning]
     *  0 : Low
     *  1 : Hi
     */
    result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0xA0, &data, 1);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    *pValue = (data & (0x01 << id)) >> id;

    SONY_TRACE_RETURN(result);
}

 sony_result_t cxd2838_demod_GPIOWrite(sony_demod_t* pDemod,
                                   uint8_t       id,
                                   uint8_t       value)
{
    sony_result_t result = SONY_RESULT_OK;
    
    SONY_TRACE_ENTER("sony_demod_GPIOWrite");

    if ((!pDemod) || (id > 2)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) && (pDemod->state != SONY_DEMOD_STATE_EWS) && 
        (pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_SHUTDOWN)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*
     * The register output the level at the pin.

     * slave    Bank    Addr    Bit    default      Name              Remarks
     * ---------------------------------------------------------------------------------
     * <SLV-X>  00h     A2h     [0]     1'b0       OREG_GPIO_OUT[0]   GPIO0 pin setting
     * <SLV-X>  00h     A2h     [1]     1'b0       OREG_GPIO_OUT[1]   GPIO1 pin setting
     * <SLV-X>  00h     A2h     [2]     1'b0       OREG_GPIO_OUT[2]   GPIO2 pin setting
     *
     *  [setting]
     *    0 : GND(default)
     *    1 : DVDD
     */
    result = sony_i2c_SetRegisterBits(pDemod->pI2c, pDemod->i2cAddressSLVX, 0xA2, (uint8_t) (value ? (0x01 << id) : 0x00), (0x01 << id));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN(result);
}

 sony_result_t cxd2838_demod_ChipID(sony_demod_t*         pDemod,
                                sony_demod_chip_id_t* pChipId)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;
    SONY_TRACE_ENTER("sony_demod_ChipID");

    if ((!pDemod) || (!pChipId)) {
        SONY_TRACE_RETURN(result);
    }

    /* Chip ID is available on both banks, SLV-T register is aligned to legacy devices so check this first,
     * if this fails (due to device being in SHUTDOWN state) read from SLV-X 
     */

    /* Set SLV-T Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00);
    if (result == SONY_RESULT_OK) {
        /*
         *   slave    Bank    Addr    Bit               NAME
         *  -----------------------------------------------------------
         *  <SLV-T>   00h     FDh     [7:0]            CHIP_ID
         */
        result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xFD, &data, 1);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    } else if (result == SONY_RESULT_ERROR_I2C) {
        /* SLV-T failed, so try SLV-X */
        /* Set SLV-X Bank : 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*
         *   slave    Bank    Addr    Bit               NAME
         *  -----------------------------------------------------------
         *  <SLV-X>   00h     FDh     [7:0]            CHIP_ID_SYS
         */
        result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0xFD, &data, 1);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    } else {
        SONY_TRACE_RETURN(result);
    }
        
    *pChipId = (sony_demod_chip_id_t) data;
    
    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

 sony_result_t cxd2838_demod_I2cRepeaterEnable(sony_demod_t* pDemod,
                                           uint8_t       enable)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0x00;
    SONY_TRACE_ENTER("sony_demod_I2cRepeaterEnable");

    /*
     *  slave    Bank    Addr    Bit    default    Value          Name
     * ----------------------------------------------------------------------------------
     * <SLV-X>   00h     08h    [0]      8'h00      8'h01     OREG_REPEN
     */
    if (enable != 0) {
        data = 0x01;
    }
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x08, data);
   	if (result != SONY_RESULT_OK) {
       SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}

 sony_result_t cxd2838_demod_SetAndSaveRegisterBits(sony_demod_t * pDemod, 
                                                uint8_t slaveAddress, 
                                                uint8_t bank, 
                                                uint8_t registerAddress,
                                                uint8_t value,
                                                uint8_t bitMask)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("sony_demod_SetAndSaveRegisterBits");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS) &&
        (pDemod->state != SONY_DEMOD_STATE_SLEEP)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set the bank */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, slaveAddress, 0x00, bank);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* Write the register value */
    result = sony_i2c_SetRegisterBits(pDemod->pI2c, slaveAddress, registerAddress, value, bitMask);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* Store the updated setting */
    result = setConfigMemory (pDemod, slaveAddress, bank, registerAddress, value, bitMask);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}

 sony_result_t cxd2838_demod_SetTsClockModeAndFreq (sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t serialTs;
    uint8_t tsRateCtrlOff;
    uint8_t tsInOff;
    sony_demod_ts_clk_configuration_t tsClkConfiguration;

    const sony_demod_ts_clk_configuration_t serialTsClkSettings [2][6] = 
    {{ /* Gated Clock */
       /* OSERCKMODE  OSERDUTYMODE  OTSCKPERIOD  OREG_CKSEL_TSIF                            */
        {      3,          1,            8,             0        }, /* High Freq, full rate */
        {      3,          1,            8,             1        }, /* Mid Freq,  full rate */
        {      3,          1,            8,             2        }, /* Low Freq,  full rate */
        {      0,          2,            16,            0        }, /* High Freq, half rate */
        {      0,          2,            16,            1        }, /* Mid Freq,  half rate */
        {      0,          2,            16,            2        }  /* Low Freq,  half rate */
    },
    {  /* Continuous Clock */
       /* OSERCKMODE  OSERDUTYMODE  OTSCKPERIOD  OREG_CKSEL_TSIF                            */
        {      1,          1,            8,             0        }, /* High Freq, full rate */
        {      1,          1,            8,             1        }, /* Mid Freq,  full rate */
        {      1,          1,            8,             2        }, /* Low Freq,  full rate */
        {      2,          2,            16,            0        }, /* High Freq, half rate */
        {      2,          2,            16,            1        }, /* Mid Freq,  half rate */
        {      2,          2,            16,            2        }  /* Low Freq,  half rate */
    }};

    const sony_demod_ts_clk_configuration_t parallelTsClkSetting = 
    {  /* OSERCKMODE  OSERDUTYMODE  OTSCKPERIOD  OREG_CKSEL_TSIF */
               0,          0,            8,             0        
    }; 

    const sony_demod_ts_clk_configuration_t backwardsCompatibleSerialTsClkSetting [2] = 
    {  /* OSERCKMODE  OSERDUTYMODE  OTSCKPERIOD  OREG_CKSEL_TSIF                            */
        {      3,          1,            8,             1        }, /* Gated Clock          */
        {      1,          1,            8,             1        }  /* Continuous Clock     */
    }; 

    const sony_demod_ts_clk_configuration_t backwardsCompatibleParallelTsClkSetting = 
    {  /* OSERCKMODE  OSERDUTYMODE  OTSCKPERIOD  OREG_CKSEL_TSIF */
               0,          0,            8,             1        
    };

    SONY_TRACE_ENTER ("sony_demod_SetTsClockModeAndFreq");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_SLEEP) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* slave    Bank    Addr    Bit    default    Name
     * ---------------------------------------------------
     * <SLV-T>  00h     C4h     [7]    1'b0       OSERIALEN
     */
    result = pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC4, &serialTs, 1);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Slave    Bank    Addr    Bit      Default    Name
     * -----------------------------------------------------------
     * <SLV-T>  00h     D3h     [0]      1'b1       OTSRATECTRLOFF
     */
    result = pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD3, &tsRateCtrlOff, 1);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Slave    Bank    Addr    Bit      Default    Name
     * ------------------------------------------------------
     * <SLV-T>  00h     DEh     [0]      1'b0       OTSIN_OFF
     */
    result = pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xDE, &tsInOff, 1);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if ((tsRateCtrlOff & 0x01) == 0x00) {
        /* (OTSRATECTRLOFF = 0 is invalid */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    if (tsInOff & 0x01) {
        /* Backwards compatible mode */
        if (serialTs & 0x80) {
            /* Serial TS */
            tsClkConfiguration = backwardsCompatibleSerialTsClkSetting[pDemod->serialTsClockModeContinuous];
        } else {
            /* Parallel TS */
            tsClkConfiguration = backwardsCompatibleParallelTsClkSetting;
        }
    } else if (serialTs & 0x80) {
        tsClkConfiguration = serialTsClkSettings[pDemod->serialTsClockModeContinuous][(uint8_t)pDemod->serialTsClkFreq];
    } else {
        /* Parallel TS */
        tsClkConfiguration = parallelTsClkSetting;
    }
    
    if (serialTs & 0x80) {
        /* Serial TS, so set serial TS specific registers */        
                
        /* slave    Bank    Addr    Bit    default    Name
         * -----------------------------------------------------
         * <SLV-T>  00h     C4h     [1:0]  2'b01      OSERCKMODE
         */
        result = sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC4, tsClkConfiguration.serialClkMode, 0x03);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* slave    Bank    Addr    Bit    default    Name
         * -------------------------------------------------------
         * <SLV-T>  00h     D1h     [1:0]  2'b01      OSERDUTYMODE
         */
        result = sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD1, tsClkConfiguration.serialDutyMode, 0x03);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }

    /* slave    Bank    Addr    Bit    default    Name
     * ------------------------------------------------------
     * <SLV-T>  00h     D9h     [7:0]  8'h08      OTSCKPERIOD
     */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD9, tsClkConfiguration.tsClkPeriod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Disable TS IF Clock */
    /* slave    Bank    Addr    Bit    default    Name
     * -------------------------------------------------------
     * <SLV-T>  00h     32h     [0]    1'b1       OREG_CK_TSIF_EN
     */
    result = sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x32, 0x00, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* slave    Bank    Addr    Bit    default    Name
     * -------------------------------------------------------
     * <SLV-T>  00h     33h     [1:0]  2'b01      OREG_CKSEL_TSIF
     */
    result = sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x33, tsClkConfiguration.clkSelTsIf, 0x03);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Enable TS IF Clock */
    /* slave    Bank    Addr    Bit    default    Name
     * -------------------------------------------------------
     * <SLV-T>  00h     32h     [0]    1'b1       OREG_CK_TSIF_EN
     */
    result = sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x32, 0x01, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}


/*------------------------------------------------------------------------------
  Functions for ISDB-T
------------------------------------------------------------------------------*/
sony_result_t sony_demod_isdbt_SetPreset(sony_demod_t*                   pDemod,
                                         sony_demod_isdbt_preset_info_t* pPresetInfo)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[2] = {0x00, 0x00};
    SONY_TRACE_ENTER("sony_demod_isdbt_SetPreset");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS) &&
        (pDemod->state != SONY_DEMOD_STATE_SLEEP)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }


    /*
     * - Disable Fast Acquisition Mode
     *
     *  When the TMCC information is not known in advance, such as in channel scan 
     *  operation, disable Fast Acquisition Mode by setting the following register 
     *  values.
     *
     *  slave    Bank    Addr    Bit    default      Value        Name
     *  ---------------------------------------------------------------------------------
     *  <SLV-T>   60h     59h    [0]       8'h00       8'h00      OCTL_PRESET_EN
     *  <SLV-T>   60h     5Ah    [0]       8'h01       8'h00      OCTL_S2_FRAMESYNC_EN
     *
     * - Enable Fast Acquisition Mode
     *
     *  Once the TMCC information is known, such as in normal channel tuning, you can 
     *  enable Fast Acquisition Mode by setting the following register values.
     *
     *  slave    Bank    Addr    Bit    default      Value        Name
     *  ---------------------------------------------------------------------------------
     *  <SLV-T>   60h     59h    [0]       8'h00       8'h01      OCTL_PRESET_EN
     *  <SLV-T>   60h     5Ah    [0]       8'h01       8'h01      OCTL_S2_FRAMESYNC_EN
     */

    /*
     *       Bank  Addr  Bit    TMCC Bit assign.(*)   Default    Name
     *       -----------------------------------------------------------------------
     *       60h   62h  [7:0]   "B20-27"               8'h3D    PIR_HOST_TMCC_SET_2
     *       60h   63h  [7:0]   "B28-35"               8'h25    PIR_HOST_TMCC_SET_3
     *       60h   64h  [7:0]   "B36-43"               8'h8B    PIR_HOST_TMCC_SET_4
     *       60h   65h  [7:0]   "B44-51"               8'h4B    PIR_HOST_TMCC_SET_5
     *       60h   66h  [7:0]   "B52-59"               8'h3F    PIR_HOST_TMCC_SET_6
     *       60h   67h  [7:0]   "B60-67"               8'hFF    PIR_HOST_TMCC_SET_7
     *       60h   68h  [7:0]   "B68-75"               8'h25    PIR_HOST_TMCC_SET_8
     *       60h   69h  [7:0]   "B76-83"               8'h8B    PIR_HOST_TMCC_SET_9
     *       60h   6Ah  [7:0]   "B84-91"               8'h4B    PIR_HOST_TMCC_SET_10
     *       60h   6Bh  [7:0]   "B92-99"               8'h3F    PIR_HOST_TMCC_SET_11
     *       60h   6Ch  [7:0]   "B100-107"             8'hFF    PIR_HOST_TMCC_SET_12
     *       60h   6Dh  [7:0]   "B108-115"             8'hFF    PIR_HOST_TMCC_SET_13
     *       60h   6Eh  [7:0]   "B116-121,2'b00"       8'hFC    PIR_HOST_TMCC_SET_14
     */
    if (!pPresetInfo) {
        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0x59,
                                             data,
                                             2);
    } else {
        data[0] = 0x01;
        data[1] = 0x01;
        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0x59,
                                             data,
                                             2);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0x62,
                                             pPresetInfo->data,
                                             13);
    }

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_isdbt_Tune(sony_demod_t*            pDemod,
                                    sony_isdbt_tune_param_t* pTuneParam)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_demod_isdbt_Tune");

    if ((!pDemod) || (!pTuneParam)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pDemod->bandwidth = pTuneParam->bandwidth;

    switch (pDemod->state) {
    case SONY_DEMOD_STATE_SLEEP:
        result = SLTtoAT(pDemod);
        break;
    case SONY_DEMOD_STATE_ACTIVE:
        result = ATtoAT(pDemod);
        break;
    case SONY_DEMOD_STATE_EWS:
        result = EWStoAT(pDemod);
        break;
    case SONY_DEMOD_STATE_UNKNOWN:
    case SONY_DEMOD_STATE_SHUTDOWN:
    default:
        result = SONY_RESULT_ERROR_SW_STATE;
        break;
    }
    
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    pDemod->state = SONY_DEMOD_STATE_ACTIVE;

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_isdbt_EWSTune(sony_demod_t*            pDemod,
                                       sony_isdbt_tune_param_t* pTuneParam)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_demod_isdbt_EWSTune");

    if ((!pDemod) || (!pTuneParam)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pDemod->bandwidth = pTuneParam->bandwidth;
    
    switch (pDemod->state) {
    case SONY_DEMOD_STATE_SLEEP:
        result = SLTtoEWS(pDemod);
        break;
    case SONY_DEMOD_STATE_EWS:
        result = EWStoEWS(pDemod);
        break;
    case SONY_DEMOD_STATE_ACTIVE:
        result = ATtoEWS(pDemod);
        break;
    case SONY_DEMOD_STATE_UNKNOWN:
    case SONY_DEMOD_STATE_SHUTDOWN:
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }
    
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    pDemod->state = SONY_DEMOD_STATE_EWS;

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_isdbt_CheckDemodLock(sony_demod_t*             pDemod,
                                              sony_demod_lock_result_t* pLock)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t dmdLock = 0;
    uint8_t tsLock  = 0;
    uint8_t unlock  = 0;
    SONY_TRACE_ENTER("sony_demod_isdbt_CheckDemodLock");

    if ((!pDemod) || (!pLock)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_demod_isdbt_monitor_SyncStat(pDemod, &dmdLock, &tsLock, &unlock);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    if (unlock == 1) {
        *pLock = SONY_DEMOD_LOCK_RESULT_UNLOCK;
    } else if (dmdLock == 1) {
        *pLock = SONY_DEMOD_LOCK_RESULT_LOCK;
    } else {
        *pLock = SONY_DEMOD_LOCK_RESULT_NOT_DETECT;
    }

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_isdbt_CheckTSLock(sony_demod_t*             pDemod,
                                           sony_demod_lock_result_t* pLock)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t dmdLock = 0;
    uint8_t tsLock  = 0;
    uint8_t unlock  = 0;
    SONY_TRACE_ENTER("sony_demod_isdbt_CheckTSLock");

    if ((!pDemod) || (!pLock)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* In EWS state, TS lock flag is invalid. */
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_demod_isdbt_monitor_SyncStat(pDemod, &dmdLock, &tsLock, &unlock);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    if (unlock == 1) {
        *pLock = SONY_DEMOD_LOCK_RESULT_UNLOCK;
    } else if (tsLock == 1) {
        *pLock = SONY_DEMOD_LOCK_RESULT_LOCK;
    } else {
        *pLock = SONY_DEMOD_LOCK_RESULT_NOT_DETECT;
    }

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_DisableTsDataPinHiZ(sony_demod_t * pDemod)
{
    uint8_t data = 0;
    uint8_t tsDataHiZ = 0;

    SONY_TRACE_ENTER ("sony_demod_DisableTsDataPinHiZ");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in SLEEP and ACTIVE states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* slave    Bank    Addr    Bit    default    Name
     * ---------------------------------------------------
     * <SLV-T>  00h     C4h     [7]    1'b0       OSERIALEN
     * <SLV-T>  00h     C4h     [3]    1'b1       OSEREXCHGB7
     */

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC4, &data, 1) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    switch (data & 0x88) {
    case 0x80:
        /* Serial TS, output from TSDATA0 */
        tsDataHiZ = 0xFE;
        break;
    case 0x88:
        /* Serial TS, output from TSDATA7 */
        tsDataHiZ = 0x7F;
        break;
    case 0x08:
    case 0x00:
    default:
        /* Parallel TS */
        tsDataHiZ = 0x00;
        break;
    }

    /* slave    Bank    Addr    Bit    default    Name
     * ---------------------------------------------------
     * <SLV-T>   00h    81h    [7:0]    8'hFF   OREG_TSDATA_HIZ
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x81, tsDataHiZ) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}


/*------------------------------------------------------------------------------
  Static Functions
------------------------------------------------------------------------------*/
 sony_result_t XtoSLT(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("XtoSLT");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* 1. SLV-X all reg clear                   | SLV-X, 00h , 02h , 00h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x02, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }
    
    /* 2. wait for stabilization                |       <3msec wait> */
    SONY_SLEEP(3);
    
    /* set SLV-X bank to 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* 3. assert SW reset                       | SLV-X, 00h , 10h , 01h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x10, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /*
     * 4. select ADC clock mode                 | SLV-X, 00h , 13h , *1  Write
     * 5. select X'tal clock mode               | SLV-X, 00h , 14h , *2  Write
     */
    {
        uint8_t data[2] = {0x00, 0x00};
        
        switch (pDemod->xtalFreq) {
        case SONY_DEMOD_XTAL_20500KHz:
            data[0] = 0x00;
            data[1] = 0x00;
            break;
        case SONY_DEMOD_XTAL_24000KHz:
            data[0] = 0x05;
            data[1] = 0x03;
            break;
        case SONY_DEMOD_XTAL_41000KHz:
            data[0] = 0x00;
            data[1] = 0x01;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }
        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVX,
                                             0x13,
                                             &data[0],
                                             2);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    /* 6. negate SW reset                       | SLV-X, 00h , 10h , 00h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x10, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 7. wait for stabilization                |       <1msec wait> */
    SONY_SLEEP(1);

    /* set SLV-T bank to 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* 8. TADC Bias ON 1                        | SLV-T, 00h , 43h , 0Ah Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x43, 0x0A);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* 9. TADC Bias ON 2                        | SLV-T, 00h , 41h , 0Ah Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x41, 0x0A);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* set SLV-T bank to 0x60 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /*
     * Disable fast acquisition mode in default
     *
     *  slave    Bank    Addr    Bit    default      Value        Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   60h     5Ah    [0]       8'h01       8'h00      OCTL_S2_FRAMESYNC_EN
     */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x5A, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN(result);
}
static sony_result_t SDtoSLT(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("SDtoSLT");
    
    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    /* set SLV-X bank to 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }
    /* 1. assert SW reset                       | SLV-X, 00h , 10h , 01h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x10, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* 2. enable oscillator                     | SLV-X, 00h , 15h , 00h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x15, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }
    
    /* 3. wait for stabilization                |       <3msec wait> */
    SONY_SLEEP(3);

    /*
     * 4. select ADC clock mode                 | SLV-X, 00h , 13h , *1  Write
     * 5. select X'tal clock mode               | SLV-X, 00h , 14h , *2  Write
     */
    {
        uint8_t data[2] = {0x00, 0x00};
        
        switch (pDemod->xtalFreq) {
        case SONY_DEMOD_XTAL_20500KHz:
            data[0] = 0x00;
            data[1] = 0x00;
            break;
        case SONY_DEMOD_XTAL_24000KHz:
            data[0] = 0x05;
            data[1] = 0x03;
            break;
        case SONY_DEMOD_XTAL_41000KHz:
            data[0] = 0x00;
            data[1] = 0x01;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }
        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                        pDemod->i2cAddressSLVX,
                                        0x13,
                                        &data[0],
                                        2);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
        }
    }
    
    /* 6. negate SW reset                       | SLV-X, 00h , 10h , 00h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x10, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }
    
    /* 7. wait for stabilization                |       <1msec wait> */
    SONY_SLEEP(1);

    /* set SLV-T bank to 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* 8. TADC Bias ON 1                        | SLV-T, 00h , 43h , 0Ah Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x43, 0x0A);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* 9. TADC Bias ON 2                        | SLV-T, 00h , 41h , 0Ah Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x41, 0x0A);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* set SLV-T bank to 0x60 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /*
     * Disable fast acquisition mode in default
     *
     *  slave    Bank    Addr    Bit    default      Value        Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   60h     5Ah    [0]       8'h01       8'h00      OCTL_S2_FRAMESYNC_EN
     */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x5A, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t SLTtoSD(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("SLTtoSD");
    
    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    /* set SLV-X bank to 0x00 */
    if (pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00)
        != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* 1. disable oscillator                    | SLV-X, 00h , 15h , 01h Write */
    if (pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x15, 0x01)
        != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t SLTtoEWS(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("SLTtoEWS");
    
    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    /* Set SLV-T Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* 1. set demod clock setting               | SLV-T, 00h , 35h , 01h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x35, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }
    
    /* 2. enable demod clock                    | SLV-T, 00h , 2Ch , 01h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2C, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }
    
    /* 3. enable ADC clock                      | SLV-T, 00h , 30h , 00h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x30, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }
    
    /* 4. enable ADC 1                          | SLV-T, 00h , 41h , 1Ah Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x41, 0x1A);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }
    /*
     * 5. enable ADC 2                          | SLV-T, 00h , 43h , *5  Write
     * 6. enable ADC 3                          | SLV-T, 00h , 44h , *6  Write
     */
    {
        uint8_t data[2] = {0x00, 0x00};
        
        switch (pDemod->xtalFreq) {
        case SONY_DEMOD_XTAL_20500KHz:
            data[0] = 0x09;
            data[1] = 0x54;
            break;
        case SONY_DEMOD_XTAL_24000KHz:
            data[0] = 0x09;
            data[1] = 0x54;
            break;
        case SONY_DEMOD_XTAL_41000KHz:
            data[0] = 0x0A;
            data[1] = 0xD4;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }
        result = pDemod->pI2c->WriteRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x43, &data[0], 2);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
        }
    }
    
    /* Set SLV-X Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* 7. enable ADC 4                          | SLV-X, 00h , 18h , 00h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x18, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* 8. set demod parameter 1                 |       <see 6-1-0> */
    result = setCommonDemodParam(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 9. set demod parameter 2                 |       <see 6-1-1/6-1-2/6-1-3> */
    switch (pDemod->bandwidth) {
    case SONY_DEMOD_BW_6_MHZ:
        result = setDemodParamFor6Mhz(pDemod);
        break;
    case SONY_DEMOD_BW_7_MHZ:
        result = setDemodParamFor7Mhz(pDemod);
        break;
    case SONY_DEMOD_BW_8_MHZ:
        result = setDemodParamFor8Mhz(pDemod);
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    /* Set SLV-T Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    /* 10. disable Hi-Z setting 1                | SLV-T, 00h , 80h , 2Fh Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x80, 0x2F);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }
    
    /* 11. disable Hi-Z setting 2 */
    result = sony_demod_DisableTsDataPinHiZ(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    SONY_TRACE_RETURN(result);
}

static sony_result_t EWStoSLT(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("EWStoSLT");
    
    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Set SLV-T Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 1. enable Hi-Z setting 1                 | SLV-T, 00h , 80h , 3Fh Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x80, 0x3F);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 2. enable Hi-Z setting 2                 | SLV-T, 00h , 81h , FFh Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x81, 0xFF);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-X Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 3. disable ADC 1                         | SLV-X, 00h , 18h , 01h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x18, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 4. disable ADC 2                         | SLV-T, 00h , 43h , 0Ah Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x43, 0x0A);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 5. disable ADC 3                         | SLV-T, 00h , 41h , 0Ah Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x41, 0x0A);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 6. disable ADC clock                     | SLV-T, 00h , 30h , 00h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x30, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 7. disable demod clock                   | SLV-T, 00h , 2Ch , 00h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2C, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 8. cancel demod clock setting            | SLV-T, 00h , 35h , 00h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x35, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t SLTtoAT(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("SLTtoAT");
    
    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Configure TS clock Mode and Frequency */
    result = sony_demod_SetTsClockModeAndFreq(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* set SLV-T bank to 0x00 */
    if (pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00)
        != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }
    
    /* 1. enable demod clock                    | SLV-T, 00h , 2Ch , 01h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2C, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* 2. enable ADC clock                      | SLV-T, 00h , 30h , 00h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x30, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }
    
    /* 3. enable ADC 1                          | SLV-T, 00h , 41h , 1Ah Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x41, 0x1A);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /*
     * 4. enable ADC 2                          | SLV-T, 00h , 43h , *5  Write
     * 5. enable ADC 3                          | SLV-T, 00h , 44h , *6  Write
     */
    {
        uint8_t data[2] = {0x00, 0x00};
        
        switch (pDemod->xtalFreq) {
        case SONY_DEMOD_XTAL_20500KHz:
            data[0] = 0x09;
            data[1] = 0x54;
            break;
        case SONY_DEMOD_XTAL_24000KHz:
            data[0] = 0x09;
            data[1] = 0x54;
            break;
        case SONY_DEMOD_XTAL_41000KHz:
            data[0] = 0x0A;
            data[1] = 0xD4;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }
        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0x43,
                                             &data[0],
                                             2);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    /* Set SLV-X Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }
    
    /* 6. enable ADC 4                          | SLV-X, 00h , 18h , 00h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x18, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 7. set demod parameter 1                 |       <see 6-1-0> */
    result = setCommonDemodParam(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    /* 8. set demod parameter 2                 |       <see 6-1-1/6-1-2/6-1-3> */
    switch (pDemod->bandwidth) {
    case SONY_DEMOD_BW_6_MHZ:
        result = setDemodParamFor6Mhz(pDemod);
        break;
    case SONY_DEMOD_BW_7_MHZ:
        result = setDemodParamFor7Mhz(pDemod);
        break;
    case SONY_DEMOD_BW_8_MHZ:
        result = setDemodParamFor8Mhz(pDemod);
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    /* set SLV-T bank to 0x00 */
    if (pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00)
        != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* 9. disable Hi-Z setting 1                | SLV-T, 00h , 80h , 28h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x80, 0x28);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* 10. disable Hi-Z setting 2 */
    result = sony_demod_DisableTsDataPinHiZ(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t ATtoSLT(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("ATtoSLT");
    
    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* set SLV-T bank to 0x00*/
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 1. disable TS output                     | SLV-T, 00h , C3h , 01h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC3, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 2. enable Hi-Z setting 1                 | SLV-T, 00h , 80h , 3Fh Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x80, 0x3F);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 3. enable Hi-Z setting 2                 | SLV-T, 00h , 81h , FFh Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x81, 0xFF);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* set SLV-X bank to 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 4. disable ADC 1                         | SLV-X, 00h , 18h , 01h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVX, 0x18, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 5. disable ADC 2                         | SLV-T, 00h , 43h , 0Ah Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x43, 0x0A);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 6. disable ADC 3                     **12| SLV-T, 00h , 41h , 0Ah Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x41, 0x0A);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    /*7. disable ADC clock                     | SLV-T, 00h , 30h , 00h Write*/
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x30, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*8. disable demod clock                   | SLV-T, 00h , 2Ch , 00h Write*/
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2C, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    SONY_TRACE_RETURN(result);
}

static sony_result_t EWStoAT(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("EWStoAT");
    
    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Set SLV-T Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 1. cancel demod clock setting            | SLV-T, 00h , 35h , 00h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x35, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    /* 2. set demod parameter 1                 |       <see 6-1-0>*/
    result = setCommonDemodParam(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 3. set demod parameter 2                 |       <see 6-1-1/6-1-2/6-1-3> */
    switch (pDemod->bandwidth) {
    case SONY_DEMOD_BW_6_MHZ:
        result = setDemodParamFor6Mhz(pDemod);
        break;
    case SONY_DEMOD_BW_7_MHZ:
        result = setDemodParamFor7Mhz(pDemod);
        break;
    case SONY_DEMOD_BW_8_MHZ:
        result = setDemodParamFor8Mhz(pDemod);
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 4. disable Hi-Z setting 1                | SLV-T, 00h , 80h , 28h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x80, 0x28);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 5. disable Hi-Z setting 2                | SLV-T, 00h , 81h , 00h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x81, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t ATtoEWS(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("ATtoEWS");
    
    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }


    /* Set SLV-T Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    /* 1. disable TS output                     | SLV-T, 00h , C3h , 01h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC3, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    /* 2. set demod clock setting               | SLV-T, 00h , 35h , 01h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x35, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    /* 3. set demod parameter 1                 |       <see 6-1-0> */
    result = setCommonDemodParam(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 4. set demod parameter 2                 |       <see 6-1-1/6-1-2/6-1-3> */
    switch (pDemod->bandwidth) {
    case SONY_DEMOD_BW_6_MHZ:
        result = setDemodParamFor6Mhz(pDemod);
        break;
    case SONY_DEMOD_BW_7_MHZ:
        result = setDemodParamFor7Mhz(pDemod);
        break;
    case SONY_DEMOD_BW_8_MHZ:
        result = setDemodParamFor8Mhz(pDemod);
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 5. disable Hi-Z setting 1                | SLV-T, 00h , 80h , 2Fh Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x80, 0x2F);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 6. disable Hi-Z setting 2                | SLV-T, 00h , 81h , FFh Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x81, 0xFF);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    SONY_TRACE_RETURN(result);
}

static sony_result_t EWStoEWS(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("EWStoEWS");
    
    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* 1. set demod parameter 2                 |       <see 6-1-1/6-1-2/6-1-3>*/
    switch (pDemod->bandwidth) {
    case SONY_DEMOD_BW_6_MHZ:
        result = setDemodParamFor6Mhz(pDemod);
        break;
    case SONY_DEMOD_BW_7_MHZ:
        result = setDemodParamFor7Mhz(pDemod);
        break;
    case SONY_DEMOD_BW_8_MHZ:
        result = setDemodParamFor8Mhz(pDemod);
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t ATtoAT(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("ATtoAT");
    
    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    /* Set SLV-T Bank : 0x00 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
    
    /* 1. disable TS output                     | SLV-T, 00h , C3h , 01h Write */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC3, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* 2. set demod parameter 2                 |       <see 6-1-1/6-1-2/6-1-3> */
    switch (pDemod->bandwidth) {
    case SONY_DEMOD_BW_6_MHZ:
        result = setDemodParamFor6Mhz(pDemod);
        break;
    case SONY_DEMOD_BW_7_MHZ:
        result = setDemodParamFor7Mhz(pDemod);
        break;
    case SONY_DEMOD_BW_8_MHZ:
        result = setDemodParamFor8Mhz(pDemod);
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t setCommonDemodParam(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("setCommonDemodParam");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Set SLV-T Bank : 0x60 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* ==Packet Error Number monitor setting
     *
     * - 20.5MHz/41MHz Xtal
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   60h     A8h    [7:0]     8'h9C      8'hB9      OPEC_MAXCLKCNT[23:16]
     * <SLV-T>   60h     A9h    [7:0]     8'h67      8'hBA      OPEC_MAXCLKCNT[15:8]
     * <SLV-T>   60h     AAh    [7:0]     8'h10      8'h63      OPEC_MAXCLKCNT[7:0]
     *
     * - 24MHz Xtal
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   60h     A8h    [7:0]     8'h9C      8'hB7      OPEC_MAXCLKCNT[23:16]
     * <SLV-T>   60h     A9h    [7:0]     8'h67      8'h1B      OPEC_MAXCLKCNT[15:8]
     * <SLV-T>   60h     AAh    [7:0]     8'h10      8'h00      OPEC_MAXCLKCNT[7:0]
     */
    {
        uint8_t data[3] = {0x00, 0x00, 0x00};

        switch (pDemod->xtalFreq) {
        case SONY_DEMOD_XTAL_20500KHz:
        case SONY_DEMOD_XTAL_41000KHz:
            data[0] = 0xB9;
            data[1] = 0xBA;
            data[2] = 0x63;
            break;
        case SONY_DEMOD_XTAL_24000KHz:
            data[0] = 0xB7;
            data[1] = 0x1B;
            data[2] = 0x00;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }
        
        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0xA8,
                                             &data[0],
                                             3);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    /* Set SLV-T Bank : 0x10 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x10);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*
     * ==TSIF rate setting
     *
     * - 20.5MHz/41MHz Xtal
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     BFh    [7:0]     8'h52      8'h61      OREG_CKF_FREQ[15:8]
     * <SLV-T>   10h     C0h    [7:0]     8'h00      8'h60      OREG_CKF_FREQ[7:0]
     *
     * - 24MHz Xtal
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     BFh    [7:0]     8'h52      8'h60      OREG_CKF_FREQ[15:8]
     * <SLV-T>   10h     C0h    [7:0]     8'h00      8'h00      OREG_CKF_FREQ[7:0]
     */
    {
        uint8_t data[2] = {0x00, 0x00};

        switch (pDemod->xtalFreq) {
        case SONY_DEMOD_XTAL_20500KHz:
        case SONY_DEMOD_XTAL_41000KHz:
            data[0] = 0x61;
            data[1] = 0x60;
            break;
        case SONY_DEMOD_XTAL_24000KHz:
            data[0] = 0x60;
            data[1] = 0x00;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0xBF,
                                             &data[0],
                                             2);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    /*
     *  ==Reception performance optimization setting
     *   slave    Bank    Addr    Bit    default   Value          Name
     *  ---------------------------------------------------------------------------------
     *  <SLV-T>   10h     E2h    [7]       8'h4E      8'hCE      OREG_PNC_DISABLE
     */    
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xE2, 0xCE);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*
     * ==ASCOT setting OFF
     *  [*]If the tuner is not ASCOT, the following setting should be set.
     * 
     *  slave    Bank    Addr    Bit    default     Value          Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   10h     A5h     [0]      8'h01      8'h00      OREG_ITB_GDEQ_EN
     */
    if (pDemod->tunerOptimize == SONY_DEMOD_TUNER_OPTIMIZE_UNKNOWN) {
        result = sony_i2c_SetRegisterBits(pDemod->pI2c,
                                          pDemod->i2cAddressSLVT,
                                          0xA5,
                                          0x00,
                                          0x01);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t setDemodParamFor8Mhz(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("setDemodParamFor8Mhz");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    /* Set SLV-T Bank : 0x10 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x10);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*
     * ==Timing Recovery setting
     *
     * - 20.5MHz/41MHz Xtal
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     9Fh    [5:0]     8'h14      8'h0F      OREG_TRCG_NOMINALRATE[37:32]
     * <SLV-T>   10h     A0h    [7:0]     8'h2E      8'h22      OREG_TRCG_NOMINALRATE[31:24]
     * <SLV-T>   10h     A1h    [7:0]     8'h00      8'h80      OREG_TRCG_NOMINALRATE[23:16]
     * <SLV-T>   10h     A2h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[15:8]
     * <SLV-T>   10h     A3h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[7:0]
     *
     * - 24MHz Xtal
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     9Fh    [5:0]     8'h14      8'h11      OREG_TRCG_NOMINALRATE[37:32]
     * <SLV-T>   10h     A0h    [7:0]     8'h2E      8'hB8      OREG_TRCG_NOMINALRATE[31:24]
     * <SLV-T>   10h     A1h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[23:16]
     * <SLV-T>   10h     A2h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[15:8]
     * <SLV-T>   10h     A3h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[7:0]
     */
    {
        uint8_t data[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

        switch (pDemod->xtalFreq) {
        case SONY_DEMOD_XTAL_20500KHz:
        case SONY_DEMOD_XTAL_41000KHz:
            data[0] = 0x0F;
            data[1] = 0x22;
            data[2] = 0x80;
            data[3] = 0x00;
            data[4] = 0x00;
            break;
        case SONY_DEMOD_XTAL_24000KHz:
            data[0] = 0x11;
            data[1] = 0xB8;
            data[2] = 0x00;
            data[3] = 0x00;
            data[4] = 0x00;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0x9F,
                                             data,
                                             5);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    /*
     * ==The filter settings for ASCOT
     *  [*]ASCOT needs the following settings.
     * 
     *    slave    Bank    Addr    Bit    default   Value          Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   10h     A6h    [7:0]     8'h1E      coef01     OREG_ITB_COEF01[7:0]
     * <SLV-T>   10h     A7h    [7:0]     8'h1D      coef02     OREG_ITB_COEF02[7:0]
     * <SLV-T>   10h     A8h    [7:0]     8'h29      coef11     OREG_ITB_COEF11[7:0]
     * <SLV-T>   10h     A9h    [7:0]     8'hC9      coef12     OREG_ITB_COEF12[7:0]
     * <SLV-T>   10h     AAh    [7:0]     8'h2A      coef21     OREG_ITB_COEF21[7:0]
     * <SLV-T>   10h     ABh    [7:0]     8'hBA      coef22     OREG_ITB_COEF22[7:0]
     * <SLV-T>   10h     ACh    [7:0]     8'h29      coef31     OREG_ITB_COEF31[7:0]
     * <SLV-T>   10h     ADh    [7:0]     8'hAD      coef32     OREG_ITB_COEF32[7:0]
     * <SLV-T>   10h     AEh    [7:0]     8'h29      coef41     OREG_ITB_COEF41[7:0]
     * <SLV-T>   10h     AFh    [7:0]     8'hA4      coef42     OREG_ITB_COEF42[7:0]
     * <SLV-T>   10h     B0h    [7:0]     8'h29      coef51     OREG_ITB_COEF51[7:0]
     * <SLV-T>   10h     B1h    [7:0]     8'h9A      coef52     OREG_ITB_COEF52[7:0]
     * <SLV-T>   10h     B2h    [7:0]     8'h28      coef61     OREG_ITB_COEF61[7:0]
     * <SLV-T>   10h     B3h    [7:0]     8'h9E      coef62     OREG_ITB_COEF62[7:0]
     *
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *        Xtal  |  coef01 |  coef02 |  coef11 |  coef12 |  coef21 |  coef22 |  coef31
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *       20.5MHz|  8'h26  |  8'hAF  |  8'h06  |  8'hCD  |  8'h13  |  8'hBB  |  8'h28
     *         41MHz|         |         |         |         |         |         |
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *         24MHz|  8'h2F  |  8'hBA  |  8'h28  |  8'h9B  |  8'h28  |  8'h9D  |  8'h28
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *        Xtal  |  coef32 |  coef41 |  coef42 |  coef51 |  coef52 |  coef61 |  coef62
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *       20.5MHz|  8'hBA  |  8'h23  |  8'hA9  |  8'h1F  |  8'hA8  |  8'h2C  |  8'hC8
     *         41MHz|         |         |         |         |         |         |
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *         24MHz|  8'hA1  |  8'h29  |  8'hA5  |  8'h2A  |  8'hAC  |  8'h29  |  8'hB5
     *       -------+---------+---------+---------+---------+---------+---------+---------
     */

    {
        uint8_t itbCoef[3][14] = {
            {0x26, 0xAF, 0x06, 0xCD, 0x13, 0xBB, 0x28, 0xBA, 0x23, 0xA9, 0x1F, 0xA8, 0x2C, 0xC8},
            {0x2F, 0xBA, 0x28, 0x9B, 0x28, 0x9D, 0x28, 0xA1, 0x29, 0xA5, 0x2A, 0xAC, 0x29, 0xB5},
            {0x26, 0xAF, 0x06, 0xCD, 0x13, 0xBB, 0x28, 0xBA, 0x23, 0xA9, 0x1F, 0xA8, 0x2C, 0xC8},
        };

        if ((pDemod->tunerOptimize == SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2D) ||
            (pDemod->tunerOptimize == SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2E) ||
            (pDemod->tunerOptimize == SONY_DEMOD_TUNER_OPTIMIZE_ASCOT3)) {
            result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                                 pDemod->i2cAddressSLVT,
                                                 0xA6,
                                                 &itbCoef[pDemod->xtalFreq][0],
                                                 14);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN(result);
            }
        }
    }

    /*
     * ==IF freq setting
     * 
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     B6h    [7:0]     8'h18      8'hXX      OREG_DNCNV_LOFRQ[23:16]
     * <SLV-T>   10h     B7h    [7:0]     8'h15      8'hXX      OREG_DNCNV_LOFRQ[15:8]
     * <SLV-T>   10h     B8h    [7:0]     8'h68      8'hXX      OREG_DNCNV_LOFRQ[7:0]
     */
    {
        uint8_t data[3] = {0x00, 0x00, 0x00};
        data[0] = (uint8_t)(((pDemod->iffreqConfig.configIsdbt8) & 0x00FF0000) >> 16);
        data[1] = (uint8_t)(((pDemod->iffreqConfig.configIsdbt8) & 0x0000FF00) >> 8);
        data[2] = (uint8_t)(((pDemod->iffreqConfig.configIsdbt8) & 0x000000FF));
        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0xB6,
                                             &data[0],
                                             3);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    /*
     * ==System Bandwidth setting
     * 
     *  slave    Bank    Addr    Bit    default    Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     D7h    [2:0]     8'h04      8'h00      OREG_CHANNEL_WIDTH[2:0]
     */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD7, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*
     * ==Demod core latency setting
     *
     *  - 20.5MHz/41MHz Xtal
     * 
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     D9h    [5:0]     8'h1A      8'h15      OREG_CDRB_GTDOFST[13:8]
     * <SLV-T>   10h     DAh    [7:0]     8'hE2      8'hA8      OREG_CDRB_GTDOFST[7:0]
     *
     *  - 24MHz Xtal
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     D9h    [5:0]     8'h1A      8'h13      OREG_CDRB_GTDOFST[13:8]
     * <SLV-T>   10h     DAh    [7:0]     8'hE2      8'hFC      OREG_CDRB_GTDOFST[7:0]
     */
    {
        uint8_t data[2] = {0x00, 0x00};

        switch (pDemod->xtalFreq) {
        case SONY_DEMOD_XTAL_20500KHz:
        case SONY_DEMOD_XTAL_41000KHz:
            data[0] = 0x15;
            data[1] = 0xA8;
            break;
        case SONY_DEMOD_XTAL_24000KHz:
            data[0] = 0x13;
            data[1] = 0xFC;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0xD9,
                                             data,
                                             2);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    /*
     * ==Acquisition optimization setting
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   12h     71h    [2:0]     8'h07      8'h03      OREG_SYR_ACTMODE[2:0]
     * <SLV-T>   15h     BEh    [7:0]     8'h02      8'h03      OREG_FCS_SYMBOL_INTERVAL_ISDBT[7:0]
     * -------------------------------------------------------------------------------
     */

    /* Set SLV-T Bank : 0x12 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x12);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x71, 0x03);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0x15 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x15);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xBE, 0x03);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }


    SONY_TRACE_RETURN(result);
}

static sony_result_t setDemodParamFor7Mhz(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("setDemodParamFor7Mhz");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    /* Set SLV-T Bank : 0x10 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x10);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*
     * ==Timing Recovery setting
     * 
     *  - 20.5MHz/41MHz Xtal
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     9Fh    [5:0]     8'h14      8'h11      OREG_TRCG_NOMINALRATE[37:32]
     * <SLV-T>   10h     A0h    [7:0]     8'h2E      8'h4C      OREG_TRCG_NOMINALRATE[31:24]
     * <SLV-T>   10h     A1h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[23:16]
     * <SLV-T>   10h     A2h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[15:8]
     * <SLV-T>   10h     A3h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[7:0]
     *
     *  - 24MHz Xtal
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     9Fh    [5:0]     8'h14      8'h14      OREG_TRCG_NOMINALRATE[37:32]
     * <SLV-T>   10h     A0h    [7:0]     8'h2E      8'h40      OREG_TRCG_NOMINALRATE[31:24]
     * <SLV-T>   10h     A1h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[23:16]
     * <SLV-T>   10h     A2h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[15:8]
     * <SLV-T>   10h     A3h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[7:0]
     *
     */
    {
        uint8_t data[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

        switch (pDemod->xtalFreq) {
        case SONY_DEMOD_XTAL_20500KHz:
        case SONY_DEMOD_XTAL_41000KHz:
            data[0] = 0x11;
            data[1] = 0x4C;
            data[2] = 0x00;
            data[3] = 0x00;
            data[4] = 0x00;
            break;
        case SONY_DEMOD_XTAL_24000KHz:
            data[0] = 0x14;
            data[1] = 0x40;
            data[2] = 0x00;
            data[3] = 0x00;
            data[4] = 0x00;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0x9F,
                                             data,
                                             5);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    /*
     * ==The filter settings for ASCOT
     *  [*]ASCOT needs the following settings.
     * 
     *  slave    Bank    Addr    Bit    default   Value          Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   10h     A6h    [7:0]     8'h1E      coef01     OREG_ITB_COEF01[7:0]
     * <SLV-T>   10h     A7h    [7:0]     8'h1D      coef02     OREG_ITB_COEF02[7:0]
     * <SLV-T>   10h     A8h    [7:0]     8'h29      coef11     OREG_ITB_COEF11[7:0]
     * <SLV-T>   10h     A9h    [7:0]     8'hC9      coef12     OREG_ITB_COEF12[7:0]
     * <SLV-T>   10h     AAh    [7:0]     8'h2A      coef21     OREG_ITB_COEF21[7:0]
     * <SLV-T>   10h     ABh    [7:0]     8'hBA      coef22     OREG_ITB_COEF22[7:0]
     * <SLV-T>   10h     ACh    [7:0]     8'h29      coef31     OREG_ITB_COEF31[7:0]
     * <SLV-T>   10h     ADh    [7:0]     8'hAD      coef32     OREG_ITB_COEF32[7:0]
     * <SLV-T>   10h     AEh    [7:0]     8'h29      coef41     OREG_ITB_COEF41[7:0]
     * <SLV-T>   10h     AFh    [7:0]     8'hA4      coef42     OREG_ITB_COEF42[7:0]
     * <SLV-T>   10h     B0h    [7:0]     8'h29      coef51     OREG_ITB_COEF51[7:0]
     * <SLV-T>   10h     B1h    [7:0]     8'h9A      coef52     OREG_ITB_COEF52[7:0]
     * <SLV-T>   10h     B2h    [7:0]     8'h28      coef61     OREG_ITB_COEF61[7:0]
     * <SLV-T>   10h     B3h    [7:0]     8'h9E      coef62     OREG_ITB_COEF62[7:0]
     *
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *        Xtal  |  coef01 |  coef02 |  coef11 |  coef12 |  coef21 |  coef22 |  coef31
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *       20.5MHz|  8'h2C  |  8'hBD  |  8'h02  |  8'hCF  |  8'h04  |  8'hF8  |  8'h23
     *         41MHz|         |         |         |         |         |         |
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *         24MHz|  8'h30  |  8'hB1  |  8'h29  |  8'h9A  |  8'h28  |  8'h9C  |  8'h28
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *        Xtal  |  coef32 |  coef41 |  coef42 |  coef51 |  coef52 |  coef61 |  coef62
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *       20.5MHz|  8'hA6  |  8'h29  |  8'hB0  |  8'h26  |  8'hA9  |  8'h21  |  8'hA5
     *         41MHz|         |         |         |         |         |         |
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *         24MHz|  8'hA0  |  8'h29  |  8'hA2  |  8'h2B  |  8'hA6  |  8'h2B  |  8'hAD
     *       -------+---------+---------+---------+---------+---------+---------+---------
     */

    {
        uint8_t itbCoef[3][14] = {
            {0x2C, 0xBD, 0x02, 0xCF, 0x04, 0xF8, 0x23, 0xA6, 0x29, 0xB0, 0x26, 0xA9, 0x21, 0xA5},
            {0x30, 0xB1, 0x29, 0x9A, 0x28, 0x9C, 0x28, 0xA0, 0x29, 0xA2, 0x2B, 0xA6, 0x2B, 0xAD},
            {0x2C, 0xBD, 0x02, 0xCF, 0x04, 0xF8, 0x23, 0xA6, 0x29, 0xB0, 0x26, 0xA9, 0x21, 0xA5},
        };

        if ((pDemod->tunerOptimize == SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2D) ||
            (pDemod->tunerOptimize == SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2E) ||
            (pDemod->tunerOptimize == SONY_DEMOD_TUNER_OPTIMIZE_ASCOT3)) {
            result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                                 pDemod->i2cAddressSLVT,
                                                 0xA6,
                                                 &itbCoef[pDemod->xtalFreq][0],
                                                 14);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN(result);
            }
        }
    }

    /*
     * ==IF freq setting
     * 
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     B6h    [7:0]     8'h18      8'hXX      OREG_DNCNV_LOFRQ[23:16]
     * <SLV-T>   10h     B7h    [7:0]     8'h15      8'hXX      OREG_DNCNV_LOFRQ[15:8]
     * <SLV-T>   10h     B8h    [7:0]     8'h68      8'hXX      OREG_DNCNV_LOFRQ[7:0]
     */
    {
        uint8_t data[3] = {0x00, 0x00, 0x00};
        data[0] = (uint8_t)(((pDemod->iffreqConfig.configIsdbt7) & 0x00FF0000) >> 16);
        data[1] = (uint8_t)(((pDemod->iffreqConfig.configIsdbt7) & 0x0000FF00) >> 8);
        data[2] = (uint8_t)(((pDemod->iffreqConfig.configIsdbt7) & 0x000000FF));
 
        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0xB6,
                                             &data[0],
                                             3);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    /*
     * ==System Bandwidth setting
     * 
     *  slave    Bank    Addr    Bit    default    Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     D7h    [2:0]     8'h04      8'h02      OREG_CHANNEL_WIDTH[2:0]
     */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD7, 0x02);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*
     * ==Demod core latency setting
     * 
     *  - 20.5MHz/41MHz Xtal
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     D9h    [5:0]     8'h1A      8'h1B      OREG_CDRB_GTDOFST[13:8]
     * <SLV-T>   10h     DAh    [7:0]     8'hE2      8'h5D      OREG_CDRB_GTDOFST[7:0]
     *
     *  - 24MHz Xtal
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     D9h    [5:0]     8'h1A      8'h1A      OREG_CDRB_GTDOFST[13:8]
     * <SLV-T>   10h     DAh    [7:0]     8'hE2      8'hFA      OREG_CDRB_GTDOFST[7:0]
     *
     */
    {
        uint8_t data[2] = {0x00, 0x00};

        switch (pDemod->xtalFreq) {
        case SONY_DEMOD_XTAL_20500KHz:
        case SONY_DEMOD_XTAL_41000KHz:
            data[0] = 0x1B;
            data[1] = 0x5D;
            break;
        case SONY_DEMOD_XTAL_24000KHz:
            data[0] = 0x1A;
            data[1] = 0xFA;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0xD9,
                                             data,
                                             2);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    /*
     * ==Acquisition optimization setting
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   12h     71h    [2:0]     8'h07      8'h03      OREG_SYR_ACTMODE[2:0]
     * <SLV-T>   15h     BEh    [7:0]     8'h02      8'h02      OREG_FCS_SYMBOL_INTERVAL_ISDBT[7:0]
     * -------------------------------------------------------------------------------
     */

    /* Set SLV-T Bank : 0x12 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x12);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x71, 0x03);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0x15 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x15);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xBE, 0x02);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }


    SONY_TRACE_RETURN(result);
}


static sony_result_t setDemodParamFor6Mhz(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("setDemodParamFor6Mhz");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    /* Set SLV-T Bank : 0x10 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x10);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*
     * ==Timing Recovery setting
     *
     *  - 20.5MHz/41MHz Xtal
     * 
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     9Fh    [5:0]     8'h14      8'h14      OREG_TRCG_NOMINALRATE[37:32]
     * <SLV-T>   10h     A0h    [7:0]     8'h2E      8'h2E      OREG_TRCG_NOMINALRATE[31:24]
     * <SLV-T>   10h     A1h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[23:16]
     * <SLV-T>   10h     A2h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[15:8]
     * <SLV-T>   10h     A3h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[7:0]
     *
     *  - 24MHz Xtal
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     9Fh    [5:0]     8'h14      8'h17      OREG_TRCG_NOMINALRATE[37:32]
     * <SLV-T>   10h     A0h    [7:0]     8'h2E      8'hA0      OREG_TRCG_NOMINALRATE[31:24]
     * <SLV-T>   10h     A1h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[23:16]
     * <SLV-T>   10h     A2h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[15:8]
     * <SLV-T>   10h     A3h    [7:0]     8'h00      8'h00      OREG_TRCG_NOMINALRATE[7:0]
     *
     */
    {
        uint8_t data[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

        switch (pDemod->xtalFreq) {
        case SONY_DEMOD_XTAL_20500KHz:
        case SONY_DEMOD_XTAL_41000KHz:
            data[0] = 0x14;
            data[1] = 0x2E;
            data[2] = 0x00;
            data[3] = 0x00;
            data[4] = 0x00;
            break;
        case SONY_DEMOD_XTAL_24000KHz:
            data[0] = 0x17;
            data[1] = 0xA0;
            data[2] = 0x00;
            data[3] = 0x00;
            data[4] = 0x00;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0x9F,
                                             data,
                                             5);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    /*
     * ==The filter settings for ASCOT
     *  [*]ASCOT needs the following settings.
     * 
     *  slave    Bank    Addr    Bit    default   Value          Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   10h     A6h    [7:0]     8'h1E      coef01     OREG_ITB_COEF01[7:0]
     * <SLV-T>   10h     A7h    [7:0]     8'h1D      coef02     OREG_ITB_COEF02[7:0]
     * <SLV-T>   10h     A8h    [7:0]     8'h29      coef11     OREG_ITB_COEF11[7:0]
     * <SLV-T>   10h     A9h    [7:0]     8'hC9      coef12     OREG_ITB_COEF12[7:0]
     * <SLV-T>   10h     AAh    [7:0]     8'h2A      coef21     OREG_ITB_COEF21[7:0]
     * <SLV-T>   10h     ABh    [7:0]     8'hBA      coef22     OREG_ITB_COEF22[7:0]
     * <SLV-T>   10h     ACh    [7:0]     8'h29      coef31     OREG_ITB_COEF31[7:0]
     * <SLV-T>   10h     ADh    [7:0]     8'hAD      coef32     OREG_ITB_COEF32[7:0]
     * <SLV-T>   10h     AEh    [7:0]     8'h29      coef41     OREG_ITB_COEF41[7:0]
     * <SLV-T>   10h     AFh    [7:0]     8'hA4      coef42     OREG_ITB_COEF42[7:0]
     * <SLV-T>   10h     B0h    [7:0]     8'h29      coef51     OREG_ITB_COEF51[7:0]
     * <SLV-T>   10h     B1h    [7:0]     8'h9A      coef52     OREG_ITB_COEF52[7:0]
     * <SLV-T>   10h     B2h    [7:0]     8'h28      coef61     OREG_ITB_COEF61[7:0]
     * <SLV-T>   10h     B3h    [7:0]     8'h9E      coef62     OREG_ITB_COEF62[7:0]
     *
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *        Xtal  |  coef01 |  coef02 |  coef11 |  coef12 |  coef21 |  coef22 |  coef31
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *       20.5MHz|  8'h27  |  8'hA7  |  8'h28  |  8'hB3  |  8'h02  |  8'hF0  |  8'h01
     *         41MHz|         |         |         |         |         |         |
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *         24MHz|  8'h31  |  8'hA8  |  8'h29  |  8'h9B  |  8'h27  |  8'h9C  |  8'h28
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *        Xtal  |  coef32 |  coef41 |  coef42 |  coef51 |  coef52 |  coef61 |  coef62
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *       20.5MHz|  8'hE8  |  8'h00  |  8'hCF  |  8'h00  |  8'hE6  |  8'h23  |  8'hA4
     *         41MHz|         |         |         |         |         |         |
     *       -------+---------+---------+---------+---------+---------+---------+---------
     *         24MHz|  8'h9E  |  8'h29  |  8'hA4  |  8'h29  |  8'hA2  |  8'h29  |  8'hA8
     *       -------+---------+---------+---------+---------+---------+---------+---------
     */
    {
        uint8_t itbCoef[3][14] = {
            {0x27, 0xA7, 0x28, 0xB3, 0x02, 0xF0, 0x01, 0xE8, 0x00, 0xCF, 0x00, 0xE6, 0x23, 0xA4},
            {0x31, 0xA8, 0x29, 0x9B, 0x27, 0x9C, 0x28, 0x9E, 0x29, 0xA4, 0x29, 0xA2, 0x29, 0xA8},
            {0x27, 0xA7, 0x28, 0xB3, 0x02, 0xF0, 0x01, 0xE8, 0x00, 0xCF, 0x00, 0xE6, 0x23, 0xA4},
        };

        if ((pDemod->tunerOptimize == SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2D) ||
            (pDemod->tunerOptimize == SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2E) ||
            (pDemod->tunerOptimize == SONY_DEMOD_TUNER_OPTIMIZE_ASCOT3)) {
            result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                                 pDemod->i2cAddressSLVT,
                                                 0xA6,
                                                 &itbCoef[pDemod->xtalFreq][0],
                                                 14);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN(result);
            }
        }
    }

    /*
     * ==IF freq setting
     * 
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     B6h    [7:0]     8'h18      8'hXX      OREG_DNCNV_LOFRQ[23:16]
     * <SLV-T>   10h     B7h    [7:0]     8'h15      8'hXX      OREG_DNCNV_LOFRQ[15:8]
     * <SLV-T>   10h     B8h    [7:0]     8'h68      8'hXX      OREG_DNCNV_LOFRQ[7:0]
     */
    {
        uint8_t data[3] = {0x00, 0x00, 0x00};
        data[0] = (uint8_t)(((pDemod->iffreqConfig.configIsdbt6) & 0x00FF0000) >> 16);
        data[1] = (uint8_t)(((pDemod->iffreqConfig.configIsdbt6) & 0x0000FF00) >> 8);
        data[2] = (uint8_t)(((pDemod->iffreqConfig.configIsdbt6) & 0x000000FF));

        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0xB6,
                                             &data[0],
                                             3);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    /*
     * ==System Bandwidth setting
     * 
     *  slave    Bank    Addr    Bit    default    Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     D7h    [2:0]     8'h04      8'h04      OREG_CHANNEL_WIDTH[2:0]
     */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD7, 0x04);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*
     * ==Demod core latency setting
     * 
     *   - 20.5MHz/41MHz Xtal
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     D9h    [5:0]     8'h1A      8'h1F      OREG_CDRB_GTDOFST[13:8]
     * <SLV-T>   10h     DAh    [7:0]     8'hE2      8'hEC      OREG_CDRB_GTDOFST[7:0]
     *
     *   - 24MHz Xtal
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   10h     D9h    [5:0]     8'h1A      8'h1F      OREG_CDRB_GTDOFST[13:8]
     * <SLV-T>   10h     DAh    [7:0]     8'hE2      8'h79      OREG_CDRB_GTDOFST[7:0]
     *
     */
    {
        uint8_t data[2] = {0x00, 0x00};

        switch (pDemod->xtalFreq) {
        case SONY_DEMOD_XTAL_20500KHz:
        case SONY_DEMOD_XTAL_41000KHz:
            data[0] = 0x1F;
            data[1] = 0xEC;
            break;
        case SONY_DEMOD_XTAL_24000KHz:
            data[0] = 0x1F;
            data[1] = 0x79;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        result = pDemod->pI2c->WriteRegister(pDemod->pI2c,
                                             pDemod->i2cAddressSLVT,
                                             0xD9,
                                             data,
                                             2);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }
    

    /*
     * ==Acquisition optimization setting
     *
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   12h     71h    [2:0]     8'h07      8'h07      OREG_SYR_ACTMODE[2:0]
     * <SLV-T>   15h     BEh    [7:0]     8'h02      8'h02      OREG_FCS_SYMBOL_INTERVAL_ISDBT[7:0]
     * -------------------------------------------------------------------------------
     */

    /* Set SLV-T Bank : 0x12 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x12);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x71, 0x07);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0x15 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x15);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xBE, 0x02);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t clearConfigMemory(sony_demod_t* pDemod)
{
    SONY_TRACE_ENTER("clearConfigMemory");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pDemod->configMemoryLastEntry = 0;
    sony_memset(&(pDemod->configMemory), 0x00, sizeof(pDemod->configMemory));

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t loadConfigMemory (sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t i;

    SONY_TRACE_ENTER("loadConfigMemory");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    for (i = 0; i < pDemod->configMemoryLastEntry; i++) {
        /* Set the bank */
        result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c,
                                                pDemod->configMemory[i].slaveAddress,
                                                0x00,
                                                pDemod->configMemory[i].bank);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }

        /* Write the register value */
        result = sony_i2c_SetRegisterBits(pDemod->pI2c,
                                          pDemod->configMemory[i].slaveAddress,
                                          pDemod->configMemory[i].registerAddress,
                                          pDemod->configMemory[i].value,
                                          pDemod->configMemory[i].bitMask);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t setConfigMemory(sony_demod_t * pDemod, 
                                     uint8_t slaveAddress, 
                                     uint8_t bank, 
                                     uint8_t registerAddress,
                                     uint8_t value,
                                     uint8_t bitMask)
{
    uint8_t i;
    uint8_t valueStored = 0;

    SONY_TRACE_ENTER("setConfigMemory");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    /* Search for matching address entry already in table */
    for (i = 0; i < pDemod->configMemoryLastEntry; i++){
        if ((valueStored == 0) &&
            (pDemod->configMemory[i].slaveAddress == slaveAddress) &&
            (pDemod->configMemory[i].bank == bank) &&
            (pDemod->configMemory[i].registerAddress == registerAddress)) {
            /* Clear bits to overwrite / set  and then store the new value */
            pDemod->configMemory[i].value &= ~bitMask;
            pDemod->configMemory[i].value |= (value & bitMask);
            
            /* Add new bits to the bit mask */
            pDemod->configMemory[i].bitMask |= bitMask;
            
            valueStored = 1;
        }
    }
    
    /* If current register does not exist in the table, add a new entry to the end */
    if (valueStored == 0) {
        if (pDemod->configMemoryLastEntry < SONY_DEMOD_MAX_CONFIG_MEMORY_COUNT) {
            pDemod->configMemory[pDemod->configMemoryLastEntry].slaveAddress = slaveAddress;
            pDemod->configMemory[pDemod->configMemoryLastEntry].bank = bank;
            pDemod->configMemory[pDemod->configMemoryLastEntry].registerAddress = registerAddress;
            pDemod->configMemory[pDemod->configMemoryLastEntry].value = (value & bitMask);
            pDemod->configMemory[pDemod->configMemoryLastEntry].bitMask = bitMask;
            pDemod->configMemoryLastEntry++;
        } else {
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_OVERFLOW);
        }
    }
    
    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

