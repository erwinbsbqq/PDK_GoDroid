/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : 2013-08-22
  File Revision : 1.0.4.0
------------------------------------------------------------------------------*/
#include "sony_demod_isdbt_monitor.h"
#include "sony_math.h"
#include "sony_stdlib.h"

#include "sony_common.h"


#define sony_math_log10            cxd2838_math_log10
#define sony_Convert2SComplement   cxd2838_Convert2SComplement

#if 1
/*------------------------------------------------------------------------------
  Static Function Prototypes
------------------------------------------------------------------------------*/
static sony_result_t isLock(sony_demod_t* pDemod);
static sony_result_t convertTmccSystem(uint8_t data, sony_isdbt_tmcc_system_t* pTmccSystem);
static sony_result_t convertTmccModulation(uint8_t data, sony_isdbt_tmcc_modulation_t* pTmccModulation);
static sony_result_t convertTmccCodingRate(uint8_t data, sony_isdbt_tmcc_coding_rate_t* pTmccCodingRate);
static sony_result_t convertTmccIlLength(uint8_t data, sony_isdbt_tmcc_il_length_t* pTmccIlLength);
static sony_result_t setTmccInfo(uint8_t* pData, sony_isdbt_tmcc_info_t* pTmccInfo);

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
sony_result_t sony_demod_isdbt_monitor_IFAGCOut(sony_demod_t* pDemod,
                                                uint32_t*     pIFAGC)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[2] = {0x00, 0x00};
    SONY_TRACE_ENTER("sony_demod_isdbt_monitor_IFAGCOut");

    if ((!pDemod) || (!pIFAGC)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x60 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*
     * slave    Bank    Addr    Bit              Name
     * ------------------------------------------------------------
     * <SLV-T>   60h     26h     [3:0]        IIFAGC_OUT[11:8]
     * <SLV-T>   60h     27h     [7:0]        IIFAGC_OUT[7:0]
     */
    
    if (pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x26, data, 2)
        != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    *pIFAGC = data[0] & 0x0F;
    *pIFAGC = (*pIFAGC << 8) + data[1];

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_isdbt_monitor_InternalDigitalAGCOut(sony_demod_t* pDemod,
                                                             uint32_t*     pInternalDigitalAGC)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[2] = {0x00, 0x00};
    SONY_TRACE_ENTER("sony_demod_isdbt_monitor_InternalDigitalAGCOut");

    if ((!pDemod) || (!pInternalDigitalAGC)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x11 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x11);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*
     * slave    Bank    Addr    Bit              Name
     * ------------------------------------------------------------
     * <SLV-T>   11h     6Dh     [5:0]        ITDA_DAGC_GAIN[13:8]
     * <SLV-T>   11h     6Eh     [7:0]        ITDA_DAGC_GAIN[7:0]
     */

    if (pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6D, data, 2)
        != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    *pInternalDigitalAGC = data[0] & 0x3F;
    *pInternalDigitalAGC = (*pInternalDigitalAGC << 8) + data[1];

    SONY_TRACE_RETURN(result);
}


sony_result_t sony_demod_isdbt_monitor_SyncStat(sony_demod_t* pDemod,
                                                uint8_t*      pDmdLock,
                                                uint8_t*      pTsLock,
                                                uint8_t*      pUnlock)
{
    uint8_t data = 0;
    sony_result_t result;
    SONY_TRACE_ENTER("sony_demod_isdbt_monitor_SyncStat");

    if ((!pDemod) || (!pDmdLock) || (!pTsLock) || (!pUnlock)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x60 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    /* Demodulator Lock Flag : IREG_DMDLOCK
     * TS Lock Flag          : ITSLOCK
     * slave    Bank    Addr    Bit              Name                meaning
     * --------------------------------------------------------------------------------
     * <SLV-T>   60h     10h     [1]          IREG_DMDLOCK        0:UNLOCK, 1:LOCK
     * <SLV-T>   60h     10h     [0]          ITSLOCK             0:UNLOCK, 1:LOCK
     * <SLV-T>   60h     10h     [4]          IEARLY_NOOFDM
     */
    result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, &data, 1);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_I2C);
    }

    if ((data & 0x02) > 0) {
        *pDmdLock = 1;
    } else {
        *pDmdLock = 0;
    }

    if ((data & 0x01) > 0) {
        *pTsLock = 1;
    } else {
        *pTsLock = 0;
    }

    if ((data & 0x10) > 0) {
        *pUnlock = 1;
    } else {
        *pUnlock = 0;
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}


sony_result_t sony_demod_isdbt_monitor_SpectrumSense(sony_demod_t*                 pDemod,
                                                     sony_demod_spectrum_sense_t * pSense)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;
    SONY_TRACE_ENTER("sony_demod_isdbt_monitor_SpectrumSense");

    if ((!pDemod) || (!pSense)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = SLVT_FreezeReg(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = isLock(pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0x60 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /*
     * slave    Bank    Addr    Bit          Signal name                Meaning
     * ---------------------------------------------------------------------------------
     * <SLV-T>   60h     3Fh     [0]          IREG_COSNE_SINV    0:Not inverted,  1:Inverted
     */
    result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x3F, &data, 1);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    SLVT_UnFreezeReg(pDemod);

    if (pDemod->confSense == SONY_DEMOD_SPECTRUM_INV) {
        *pSense = (data & 0x01) ? SONY_DEMOD_SPECTRUM_NORMAL : SONY_DEMOD_SPECTRUM_INV;
    } else {
        *pSense = (data & 0x01) ? SONY_DEMOD_SPECTRUM_INV : SONY_DEMOD_SPECTRUM_NORMAL;
    }

    SONY_TRACE_RETURN(result);
}


sony_result_t sony_demod_isdbt_monitor_CarrierOffset(sony_demod_t* pDemod,
                                                     int32_t*      pOffset)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x00};
    uint32_t iregCrcgCtrlval = 0;
    SONY_TRACE_ENTER("sony_demod_isdbt_monitor_CarrierOffset");

    if ((!pDemod) || (!pOffset)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = SLVT_FreezeReg(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = isLock(pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0x60 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /*
     * slave    Bank    Addr    Bit              Signal name
     * ---------------------------------------------------------------
     * <SLV-T>   60h     4Ch     [4:0]      IREG_CRCG_CTLVAL[28:24]
     * <SLV-T>   60h     4Dh     [7:0]      IREG_CRCG_CTLVAL[23:16]
     * <SLV-T>   60h     4Eh     [7:0]      IREG_CRCG_CTLVAL[15:8]
     * <SLV-T>   60h     4Fh     [7:0]      IREG_CRCG_CTLVAL[7:0]
     */
    result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x4C, data, 4);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    SLVT_UnFreezeReg(pDemod);

    iregCrcgCtrlval = data[0] & 0x1F;
    iregCrcgCtrlval = (iregCrcgCtrlval << 8) + data[1];
    iregCrcgCtrlval = (iregCrcgCtrlval << 8) + data[2];
    iregCrcgCtrlval = (iregCrcgCtrlval << 8) + data[3];

    /*
     * (1)For the tuner which spectrum sense is not same as RF, use these formulas.
     *  (Ordinary tuner, which is single converted and use upper local. also including ASCOT)
     *
     * Carrier Frequency Offset [Hz] = -( ( IREG_CRCG_CTLVAL / 2^28 ) * 8192 ) / ( 1008 * 10^-6 )   ---6MHz BW
     *                               = -( IREG_CRCG_CTLVAL * 8 / (2^18 * 1008 / 10^6) )
     *                               = -( IREG_CRCG_CTLVAL * 8 / 264.241 )
     *
     * Carrier Frequency Offset [Hz] = -( ( IREG_CRCG_CTLVAL / 2^28 ) * 8192 ) / (  882 * 10^-6 )   ---7MHz BW
     *                               = -( IREG_CRCG_CTLVAL * 8 / (2^18 * 882 / 10^6) )
     *                               = -( IREG_CRCG_CTLVAL * 8 / 231.211 )
     *
     * Carrier Frequency Offset [Hz] = -( ( IREG_CRCG_CTLVAL / 2^28 ) * 8192 ) / (  756 * 10^-6 )   ---8MHz BW
     *                               = -( IREG_CRCG_CTLVAL * 8 / (2^18 * 756 / 10^6) )
     *                               = -( IREG_CRCG_CTLVAL * 8 / 198.181 )
     *
     *  (2)For the tuner which spectrum sense is same as RF, use these formulas.
     *
     * Carrier Frequency Offset [Hz] = ( ( IREG_CRCG_CTLVAL / 2^28 ) * 8192 ) / ( 1008 * 10^-6 )   ---6MHz BW
     * Carrier Frequency Offset [Hz] = ( ( IREG_CRCG_CTLVAL / 2^28 ) * 8192 ) / (  882 * 10^-6 )   ---7MHz BW
     * Carrier Frequency Offset [Hz] = ( ( IREG_CRCG_CTLVAL / 2^28 ) * 8192 ) / (  756 * 10^-6 )   ---8MHz BW
     */
    
    *pOffset = sony_Convert2SComplement(iregCrcgCtrlval, 29);
    switch (pDemod->bandwidth) {
    case SONY_DEMOD_BW_6_MHZ:
        *pOffset = -1 * ((*pOffset) * 8 / 264);
        break;
    case SONY_DEMOD_BW_7_MHZ:
        *pOffset = -1 * ((*pOffset) * 8 / 231);
        break;
    case SONY_DEMOD_BW_8_MHZ:
        *pOffset = -1 * ((*pOffset) * 8 / 198);
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->confSense == SONY_DEMOD_SPECTRUM_INV) {
        *pOffset *= -1;
    }
    
    SONY_TRACE_RETURN(result);
}


sony_result_t sony_demod_isdbt_monitor_SNR(sony_demod_t* pDemod,
                                           int32_t*      pSNR)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[2] = {0x00, 0x00};
    uint16_t iregSnmonOd = 0x0000;
    SONY_TRACE_ENTER("sony_demod_isdbt_monitor_SNR");

    if ((!pDemod) || (!pSNR)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = SLVT_FreezeReg(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = isLock(pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0x60 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /*
     * slave    Bank    Addr    Bit              Name
     * ------------------------------------------------------------
     * <SLV-T>   60h     28h     [7:0]        IREG_SNMON_OD[15:8]
     * <SLV-T>   60h     29h     [7:0]        IREG_SNMON_OD[7:0]
     */
    result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x28, data, 2);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    SLVT_UnFreezeReg(pDemod);

    iregSnmonOd = data[0];
    iregSnmonOd = (iregSnmonOd << 8) + data[1];

    if (iregSnmonOd == 0) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
    }

    /* 
     *
     * Xtal = 20.5MHz or 41MHz and
     * BW8MHz:
     *           if IREG_SNMON_OD > 1143
     *           SNR[dB] = 10 * log10 ( 1143 / ( 1200 - 1143 ) ) + 22
     *           else
     *           SNR[dB] = 10 * log10 ( IREG_SNMON_OD / ( 1200 - IREG_SNMON_OD ) ) + 22
     *                   = 10 * (log10(IREG_SNMON_OD) - log10(1200 - IREG_SNMON_OD)) + 22
     *           sony_log10 returns log10(x) * 100
     *           Therefore SNR(dB) * 1000 :
     *                   = 10 * 10 * (sony_log10(IREG_SNMON_OD) - sony_log10(1200 - IREG_SNMON_OD) + 22
     *
     * Others:   SNR[dB] = 10 * log10 (IREG_SNMON_OD / 8 )
     *                   = 10 * (log10(IREG_SNMON_OD) - log10(8))
     *           sony_log10 returns log10(x) * 100
     *           Therefore SNR(dB) * 1000 :
     *                   = 10 * 10 * (sony_log10(IREG_SNMON_OD) - sony_log10(8))
     *                   = 10 * 10 * sony_log10(IREG_SNMON_OD) - 9031
     */

    if ((pDemod->bandwidth == SONY_DEMOD_BW_8_MHZ) &&
        ((pDemod->xtalFreq == SONY_DEMOD_XTAL_20500KHz) ||
        (pDemod->xtalFreq == SONY_DEMOD_XTAL_41000KHz))) {
        if (iregSnmonOd > 1143) {
            iregSnmonOd = 1143;
        }
        *pSNR = 10 * 10 * ((int32_t)sony_math_log10(iregSnmonOd) - (int32_t)sony_math_log10(1200 - iregSnmonOd));
        *pSNR += 22000;
    } else {
        *pSNR = 100 * (int32_t)sony_math_log10(iregSnmonOd) - 9031;
    }

    SONY_TRACE_RETURN(result);
}


sony_result_t sony_demod_isdbt_monitor_ModeGuard(sony_demod_t*       pDemod,
                                                 sony_isdbt_mode_t*  pMode,
                                                 sony_isdbt_guard_t* pGuard)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0x00;
    SONY_TRACE_ENTER("sony_demod_isdbt_monitor_ModeGuard");

    if ((!pDemod) || (!pMode) || (!pGuard)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = SLVT_FreezeReg(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = isLock(pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0x60 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /*
     *  slave    Bank    Addr    Bit           Name               Meaning
     * ---------------------------------------------------------------------------------
     * <SLV-T>   60h     40h     [3:2]        IREG_MODE[1:0]     0:Mode1, 1:Mode3, 2:Mode2
     * <SLV-T>   60h     40h     [1:0]        IREG_GI[1:0]       0:1/32, 1:1/16, 2:1/8, 3:1/4
     */
    result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x40, &data, 1);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    SLVT_UnFreezeReg(pDemod);
    
    switch ((data >> 2) & 0x03) {
    case 0:
        *pMode = SONY_ISDBT_MODE_1;
        break;
    case 1:
        *pMode = SONY_ISDBT_MODE_3;
        break;
    case 2:
        *pMode = SONY_ISDBT_MODE_2;
        break;
    default:
        *pMode = SONY_ISDBT_MODE_UNKNOWN;
        break;
    }

    switch (data & 0x03) {
    case 0:
        *pGuard = SONY_ISDBT_GUARD_1_32;
        break;
    case 1:
        *pGuard = SONY_ISDBT_GUARD_1_16;
        break;
    case 2:
        *pGuard = SONY_ISDBT_GUARD_1_8;
        break;
    case 3:
        *pGuard = SONY_ISDBT_GUARD_1_4;
        break;
    default:
        *pGuard = SONY_ISDBT_GUARD_UNKNOWN;
        break;
    }

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_isdbt_monitor_PreRSBER(sony_demod_t*               pDemod,
                                                sony_isdbt_monitor_target_t target,
                                                uint32_t*                   pBER)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t tmpPacketNum[2] = {0x00, 0x00};
    uint8_t tmpBitError[3] = {0x00, 0x00, 0x00};
    uint16_t packetNum = 0x0000;
    uint32_t bitError = 0x00000000;

    SONY_TRACE_ENTER("sony_demod_isdbt_monitor_PreRSBER");

    if ((!pDemod) || (!pBER)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* Invalid on EWS mode */
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = SLVT_FreezeReg(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = isLock(pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0x60 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /*
     *  slave     Bank   Addr    Bit          Name
     * ------------------------------------------------------------
     * <SLV-T>    60h    5Bh     [6:0]    OBER_CDUR_RSA[14:8]
     * <SLV-T>    60h    5Ch     [7:0]    OBER_CDUR_RSA[7:0]
     */
    result = pDemod->pI2c->ReadRegister(pDemod->pI2c,
                                        pDemod->i2cAddressSLVT,
                                        0x5B,
                                        tmpPacketNum,
                                        2);

    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    switch (target) {
    case SONY_ISDBT_MONITOR_TARGET_LAYER_A:
        /*
         *  slave     Bank   Addr    Bit          Name
         * ------------------------------------------------------------
         * <SLV-T>    60h    16h     [5:0]    IBER_BENUM_RSA[21:16]
         * <SLV-T>    60h    17h     [7:0]    IBER_BENUM_RSA[15:8]
         * <SLV-T>    60h    18h     [7:0]    IBER_BENUM_RSA[7:0]
         */
        result = pDemod->pI2c->ReadRegister(pDemod->pI2c,
                                            pDemod->i2cAddressSLVT,
                                            0x16,
                                            tmpBitError,
                                            3);
        break;
    case SONY_ISDBT_MONITOR_TARGET_LAYER_B:
        /*
         *  slave     Bank   Addr    Bit          Name
         * ------------------------------------------------------------
         * <SLV-T>    60h    19h     [5:0]    IBER_BENUM_RSB[21:16]
         * <SLV-T>    60h    1Ah     [7:0]    IBER_BENUM_RSB[15:8]
         * <SLV-T>    60h    1Bh     [7:0]    IBER_BENUM_RSB[7:0]
         */
        result = pDemod->pI2c->ReadRegister(pDemod->pI2c,
                                            pDemod->i2cAddressSLVT,
                                            0x19,
                                            tmpBitError,
                                            3);
        break;
    case SONY_ISDBT_MONITOR_TARGET_LAYER_C:
        /*
         *  slave     Bank   Addr    Bit          Name
         * ------------------------------------------------------------
         * <SLV-T>    60h    1Ch     [5:0]    IBER_BENUM_RSC[21:16]
         * <SLV-T>    60h    1Dh     [7:0]    IBER_BENUM_RSC[15:8]
         * <SLV-T>    60h    1Eh     [7:0]    IBER_BENUM_RSC[7:0]
         */
        result = pDemod->pI2c->ReadRegister(pDemod->pI2c,
                                            pDemod->i2cAddressSLVT,
                                            0x1C,
                                            tmpBitError,
                                            3);
        break;
    default:
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    SLVT_UnFreezeReg(pDemod);

    packetNum = tmpPacketNum[0];
    packetNum = (packetNum << 8) + tmpPacketNum[1];
    bitError = tmpBitError[0] & 0x7F;
    bitError = (bitError << 8) + tmpBitError[1];
    bitError = (bitError << 8) + tmpBitError[2];

    if (packetNum == 0) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
    }

    /*
        BER = (bitError * 10000000) / (packetNum * 8 * 204)

        BER = (bitError * 312500) / (packetNum * 51)

        BER = (bitError * 250 * 1250) / (packetNum * 51)

     */

    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        div = packetNum * 51;

        Q = (bitError * 250) / div;
        R = (bitError * 250) % div;

        R *= 1250;
        Q = Q * 1250 + R / div;
        R = R % div;

        if (R >= (div/2)) {
            *pBER = Q + 1;
        } else {
            *pBER = Q;
        }
    }

    SONY_TRACE_RETURN(result);
}


sony_result_t sony_demod_isdbt_monitor_PER(sony_demod_t*                pDemod,
                                           sony_isdbt_monitor_target_t  target,
                                           uint32_t*                    pPER)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t tmpPacketNum[2] = {0x00, 0x00};
    uint8_t tmpPacketError[2] = {0x00, 0x00};
    uint16_t packetNum = 0x0000;
    uint32_t packetError = 0x00000000;
    SONY_TRACE_ENTER("sony_demod_isdbt_monitor_PER");

    if ((!pDemod) || (!pPER)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* Invalid on EWS mode */
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = SLVT_FreezeReg(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = isLock(pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0x60 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }
    
    /*
     *  slave     Bank   Addr    Bit          Name
     * ------------------------------------------------------------
     * <SLV-T>    60h    5Bh     [6:0]    OBER_CDUR_RSA[14:8]
     * <SLV-T>    60h    5Ch     [7:0]    OBER_CDUR_RSA[7:0]
     */
    result = pDemod->pI2c->ReadRegister(pDemod->pI2c,
                                        pDemod->i2cAddressSLVT,
                                        0x5B,
                                        tmpPacketNum,
                                        2);

    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    switch (target) {
    case SONY_ISDBT_MONITOR_TARGET_LAYER_A:
        /*
         *  slave     Bank   Addr    Bit          Name
         * ------------------------------------------------------------
         * <SLV-T>    60h    1Fh     [6:0]    IBER_PENUM_RSA[14:8]
         * <SLV-T>    60h    20h     [7:0]    IBER_PENUM_RSA[7:0]
         */
        result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x1F, tmpPacketError, 2);
        break;
    case SONY_ISDBT_MONITOR_TARGET_LAYER_B:
        /*
         *  slave     Bank   Addr    Bit          Name
         * ------------------------------------------------------------
         * <SLV-T>    60h    21h     [6:0]    IBER_PENUM_RSB[14:8]
         * <SLV-T>    60h    22h     [7:0]    IBER_PENUM_RSB[7:0]
         */
        result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x21, tmpPacketError, 2);
        break;
    case SONY_ISDBT_MONITOR_TARGET_LAYER_C:
        /*
         *  slave     Bank   Addr    Bit          Name
         * ------------------------------------------------------------
         * <SLV-T>    60h    23h     [6:0]    IBER_PENUM_RSC[14:8]
         * <SLV-T>    60h    24h     [7:0]    IBER_PENUM_RSC[7:0]
         */
        result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x23, tmpPacketError, 2);
        break;
    default:
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    SLVT_UnFreezeReg(pDemod);
    
    packetError = tmpPacketError[0];
    packetError = (packetError << 8) + tmpPacketError[1];
    packetNum = tmpPacketNum[0];
    packetNum = (packetNum << 8) + tmpPacketNum[1];

    if (packetNum == 0) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
    }
    /*
      PER = packetError * 1000000 / packetNum

      PER = packetError * 1000 * 1000 / packetNum
     */
    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        div = packetNum;
        Q = (packetError * 1000) / div;
        R = (packetError * 1000) % div;

        R *= 1000;
        Q = Q * 1000 + R / div;
        R = R % div;

        if ((div != 1) && (R >= (div / 2))) {
            *pPER = Q + 1;
        } else {
            *pPER = Q;
        }
    }
    
    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_isdbt_monitor_TMCCInfo(sony_demod_t*           pDemod,
                                                sony_isdbt_tmcc_info_t* pTMCCInfo)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[11] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    SONY_TRACE_ENTER("sony_demod_isdbt_monitor_TMCCInfo");

    if ((!pDemod) || (!pTMCCInfo)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = SLVT_FreezeReg(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = isLock(pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0x60 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /*
     *  slave    Bank    Addr    Bit              Name        
     *  --------------------------------------------------------
     *  <SLV-T>   60h     32h    [7:0]          PIR_TMCC_SET_2
     *  <SLV-T>   60h     33h    [7:0]          PIR_TMCC_SET_3
     *  <SLV-T>   60h     34h    [7:0]          PIR_TMCC_SET_4
     *  <SLV-T>   60h     35h    [7:0]          PIR_TMCC_SET_5
     *  <SLV-T>   60h     36h    [7:0]          PIR_TMCC_SET_6
     *  <SLV-T>   60h     37h    [7:0]          PIR_TMCC_SET_7
     *  <SLV-T>   60h     38h    [7:0]          PIR_TMCC_SET_8
     *  <SLV-T>   60h     39h    [7:0]          PIR_TMCC_SET_9
     *  <SLV-T>   60h     3Ah    [7:0]          PIR_TMCC_SET_10
     *  <SLV-T>   60h     3Bh    [7:0]          PIR_TMCC_SET_11
     *  <SLV-T>   60h     3Ch    [7:0]          PIR_TMCC_SET_12
     *  <SLV-T>   60h     3Dh    [7:0]          PIR_TMCC_SET_13
     *  <SLV-T>   60h     3Eh    [7:0]          PIR_TMCC_SET_14
     */
    result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x32, data, 11);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    SLVT_UnFreezeReg(pDemod);

    result = setTmccInfo(data, pTMCCInfo);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    
    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_isdbt_monitor_PresetInfo(sony_demod_t*                   pDemod,
                                                  sony_demod_isdbt_preset_info_t* pPresetInfo)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_demod_isdbt_monitor_PresetInfo");

    if ((!pDemod) || (!pPresetInfo)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = SLVT_FreezeReg(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = isLock(pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0x60 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /*
     *  slave    Bank    Addr    Bit              Name        
     *  --------------------------------------------------------
     *  <SLV-T>   60h     32h    [7:0]          PIR_TMCC_SET_2
     *  <SLV-T>   60h     33h    [7:0]          PIR_TMCC_SET_3
     *  <SLV-T>   60h     34h    [7:0]          PIR_TMCC_SET_4
     *  <SLV-T>   60h     35h    [7:0]          PIR_TMCC_SET_5
     *  <SLV-T>   60h     36h    [7:0]          PIR_TMCC_SET_6
     *  <SLV-T>   60h     37h    [7:0]          PIR_TMCC_SET_7
     *  <SLV-T>   60h     38h    [7:0]          PIR_TMCC_SET_8
     *  <SLV-T>   60h     39h    [7:0]          PIR_TMCC_SET_9
     *  <SLV-T>   60h     3Ah    [7:0]          PIR_TMCC_SET_10
     *  <SLV-T>   60h     3Bh    [7:0]          PIR_TMCC_SET_11
     *  <SLV-T>   60h     3Ch    [7:0]          PIR_TMCC_SET_12
     *  <SLV-T>   60h     3Dh    [7:0]          PIR_TMCC_SET_13
     *  <SLV-T>   60h     3Eh    [7:0]          PIR_TMCC_SET_14
     */
    result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x32, pPresetInfo->data, 13);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    SLVT_UnFreezeReg(pDemod);

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_isdbt_monitor_PacketErrorNumber(sony_demod_t*               pDemod,
                                                         sony_isdbt_monitor_target_t target,
                                                         uint32_t*                   pPacketErrorNum)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[2];
    SONY_TRACE_ENTER("sony_demod_isdbt_monitor_PacketErrorNumber");

    if ((!pDemod) || (!pPacketErrorNum)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* Invalid on EWS mode */
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = SLVT_FreezeReg(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = isLock(pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0x60 */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x60);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    switch (target) {
    case SONY_ISDBT_MONITOR_TARGET_LAYER_A:
        /*
         * slave    Bank    Addr    Bit           Name
         * ------------------------------------------------------------
         * <SLV-T>   60h     A2h     [7:0]    ICWRJCTCNT_A[15:8]
         * <SLV-T>   60h     A3h     [7:0]    ICWRJCTCNT_A[7:0]
         */
        result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA2, data, 2);
        break;
    case SONY_ISDBT_MONITOR_TARGET_LAYER_B:
        /*
         * slave    Bank    Addr    Bit           Name
         * ------------------------------------------------------------
         * <SLV-T>   60h     A4h     [7:0]    ICWRJCTCNT_B[15:8]
         * <SLV-T>   60h     A5h     [7:0]    ICWRJCTCNT_B[7:0]
         */
        result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA4, data, 2);
        break;
    case SONY_ISDBT_MONITOR_TARGET_LAYER_C:
        /*
         * slave    Bank    Addr    Bit           Name
         * ------------------------------------------------------------
         * <SLV-T>   60h     A6h     [7:0]    ICWRJCTCNT_C[15:8]
         * <SLV-T>   60h     A7h     [7:0]    ICWRJCTCNT_C[7:0]
         */
        result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA6, data, 2);
        break;
    default:
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }
    
    SLVT_UnFreezeReg(pDemod);

    *pPacketErrorNum = 0;
    *pPacketErrorNum = data[0];
    *pPacketErrorNum = (*pPacketErrorNum << 8) + data[1];


    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_isdbt_monitor_ACEEWInfo(sony_demod_t*            pDemod,
                                                 uint8_t*                 pIsExist,
                                                 sony_isdbt_aceew_info_t* pACEEWInfo)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[36];
    
    SONY_TRACE_ENTER("sony_demod_isdbt_monitor_ACEEWInfo");

    if ((!pDemod) || (!pACEEWInfo) || (!pIsExist)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
    }

    result = SLVT_FreezeReg(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = isLock(pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    /*
     * slave    Bank    Addr      Bit           Name
     * ------------------------------------------------------------
     * <SLV-T>   61h     21h       [0]      IREG_AC_EMERGENCY_ACTIVATION
     * <SLV-T>   61h     22h       [7:0]    IREG_AC_EMERGENCY_PAGE0ALL1,IREG_AC_EMERGENCY_PAGE0SECOND_EN,IREG_AC_EMERGENCY_DECODE_EN_0,IREG_AC_EMERGENCY_DECODE_EN_1,IREG_AC_EMERGENCY_DECODE_EN_2
     * <SLV-T>   61h     23h - 2eh          IREG_AC_EMERGENCY_DECODE_0_1 - IREG_AC_EMERGENCY_DECODE_0_11
     * <SLV-T>   61h     2fh - 39h          IREG_AC_EMERGENCY_DECODE_1_1 - IREG_AC_EMERGENCY_DECODE_1_11
     * <SLV-T>   61h     3ah - 44h          IREG_AC_EMERGENCY_DECODE_2_1 - IREG_AC_EMERGENCY_DECODE_2_11
     */
    result = pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x61);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    result = pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x21, data, 36);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg(pDemod);
        SONY_TRACE_RETURN(result);
    }

    SLVT_UnFreezeReg(pDemod);
    
    if(data[0] & 0x01) {
        *pIsExist = 1;

        pACEEWInfo->startEndFlag = (uint8_t)((data[1] >> 6) & 0x03);
        pACEEWInfo->updateFlag = (uint8_t)((data[1] >> 4) & 0x03);
        pACEEWInfo->signalId = (uint8_t)((data[1] >> 1) & 0x07);

        pACEEWInfo->isAreaValid = 0;
        pACEEWInfo->isEpicenter1Valid = 0;
        pACEEWInfo->isEpicenter2Valid = 0;
        if(data[2] & 0x04) {
            /* Area data is valid */
            pACEEWInfo->isAreaValid = 1;
            sony_memcpy(pACEEWInfo->areaInfo.data, &data[3], 11);
        }
        if(data[2] & 0x02) {
            /* Epicenter1 data is valid */
            pACEEWInfo->isEpicenter1Valid = 1;
            sony_memcpy(pACEEWInfo->epicenter1Info.data, &data[14], 11);
        }
        if(data[2] & 0x01) {
            /* Epicenter2 data is valid */
            pACEEWInfo->isEpicenter2Valid = 1;
            sony_memcpy(pACEEWInfo->epicenter2Info.data, &data[25], 11);
        }
    } else {
        *pIsExist = 0;
    }

    SONY_TRACE_RETURN(result);
}

/*------------------------------------------------------------------------------
  Static Functions
------------------------------------------------------------------------------*/
static sony_result_t isLock(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t dmdLock = 0;
    uint8_t tsLock  = 0;
    uint8_t unlock  = 0;
    SONY_TRACE_ENTER("isLock");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_isdbt_monitor_SyncStat(pDemod, &dmdLock, &tsLock, &unlock);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    if (dmdLock) {
        SONY_TRACE_RETURN(SONY_RESULT_OK);
    } else {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
    }
}

static sony_result_t convertTmccSystem(uint8_t data, sony_isdbt_tmcc_system_t* pTmccSystem)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("convertTmccSystem");

    /* Null check */
    if(!pTmccSystem){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    switch(data & 0x03)
    {
    case 0:
        *pTmccSystem = SONY_ISDBT_TMCC_SYSTEM_ISDB_T;
        break;
    case 1:
        *pTmccSystem = SONY_ISDBT_TMCC_SYSTEM_ISDB_TSB;
        break;
    case 2:
        *pTmccSystem = SONY_ISDBT_TMCC_SYSTEM_RESERVE_2;
        break;
    case 3:
        *pTmccSystem = SONY_ISDBT_TMCC_SYSTEM_RESERVE_3;
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    SONY_TRACE_RETURN(result);
}

static sony_result_t convertTmccModulation(uint8_t data, sony_isdbt_tmcc_modulation_t* pTmccModulation)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("convertTmccModulation");

    /* Null check */
    if(!pTmccModulation){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    switch(data)
    {
    case 0:
        *pTmccModulation = SONY_ISDBT_TMCC_MODULATION_DQPSK;
        break;
    case 1:
        *pTmccModulation = SONY_ISDBT_TMCC_MODULATION_QPSK;
        break;
    case 2:
        *pTmccModulation = SONY_ISDBT_TMCC_MODULATION_16QAM;
        break;
    case 3:
        *pTmccModulation = SONY_ISDBT_TMCC_MODULATION_64QAM;
        break;
    case 4:
        *pTmccModulation = SONY_ISDBT_TMCC_MODULATION_RESERVED_4;
        break;
    case 5:
        *pTmccModulation = SONY_ISDBT_TMCC_MODULATION_RESERVED_5;
        break;
    case 6:
        *pTmccModulation = SONY_ISDBT_TMCC_MODULATION_RESERVED_6;
        break;
    case 7:
        *pTmccModulation = SONY_ISDBT_TMCC_MODULATION_UNUSED_7;
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    SONY_TRACE_RETURN(result);
}

static sony_result_t convertTmccCodingRate(uint8_t data, sony_isdbt_tmcc_coding_rate_t* pTmccCodingRate)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("convertTmccCodingRate");

    /* Null check */
    if(!pTmccCodingRate){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    switch(data)
    {
    case 0:
        *pTmccCodingRate = SONY_ISDBT_TMCC_CODING_RATE_1_2;
        break;
    case 1:
        *pTmccCodingRate = SONY_ISDBT_TMCC_CODING_RATE_2_3;
        break;
    case 2:
        *pTmccCodingRate = SONY_ISDBT_TMCC_CODING_RATE_3_4;
        break;
    case 3:
        *pTmccCodingRate = SONY_ISDBT_TMCC_CODING_RATE_5_6;
        break;
    case 4:
        *pTmccCodingRate = SONY_ISDBT_TMCC_CODING_RATE_7_8;
        break;
    case 5:
        *pTmccCodingRate = SONY_ISDBT_TMCC_CODING_RATE_RESERVED_5;
        break;
    case 6:
        *pTmccCodingRate = SONY_ISDBT_TMCC_CODING_RATE_RESERVED_6;
        break;
    case 7:
        *pTmccCodingRate = SONY_ISDBT_TMCC_CODING_RATE_UNUSED_7;
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    SONY_TRACE_RETURN(result);
}

static sony_result_t convertTmccIlLength(uint8_t data, sony_isdbt_tmcc_il_length_t* pTmccIlLength)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("convertTmccIlLength");

    /* Null check */
    if(!pTmccIlLength){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    switch(data)
    {
    case 0:
        *pTmccIlLength = SONY_ISDBT_TMCC_IL_LENGTH_0_0_0;
        break;
    case 1:
        *pTmccIlLength = SONY_ISDBT_TMCC_IL_LENGTH_4_2_1;
        break;
    case 2:
        *pTmccIlLength = SONY_ISDBT_TMCC_IL_LENGTH_8_4_2;
        break;
    case 3:
        *pTmccIlLength = SONY_ISDBT_TMCC_IL_LENGTH_16_8_4;
        break;
    case 4:
        *pTmccIlLength = SONY_ISDBT_TMCC_IL_LENGTH_RESERVED_4;
        break;
    case 5:
        *pTmccIlLength = SONY_ISDBT_TMCC_IL_LENGTH_RESERVED_5;
        break;
    case 6:
        *pTmccIlLength = SONY_ISDBT_TMCC_IL_LENGTH_RESERVED_6;
        break;
    case 7:
        *pTmccIlLength = SONY_ISDBT_TMCC_IL_LENGTH_UNUSED_7;
        break;
    default:
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    SONY_TRACE_RETURN(result);
}

static sony_result_t setTmccInfo(uint8_t* pData, sony_isdbt_tmcc_info_t* pTmccInfo)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("setTmccInfo");

    /* System */
    result = convertTmccSystem((uint8_t)sony_BitSplitFromByteArray(pData, 0, 2),
                               &(pTmccInfo->systemId));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* Count down index */
    pTmccInfo->countDownIndex = (uint8_t)sony_BitSplitFromByteArray(pData, 2, 4);

    /* EWS flag */
    pTmccInfo->ewsFlag = (uint8_t)sony_BitSplitFromByteArray(pData, 6, 1);

    /* Current */
    pTmccInfo->currentInfo.isPartial = (uint8_t)sony_BitSplitFromByteArray(pData, 7, 1);

    result = convertTmccModulation((uint8_t)sony_BitSplitFromByteArray(pData, 8, 3), &(pTmccInfo->currentInfo.layerA.modulation));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = convertTmccCodingRate((uint8_t)sony_BitSplitFromByteArray(pData, 11, 3), &(pTmccInfo->currentInfo.layerA.codingRate));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = convertTmccIlLength((uint8_t)sony_BitSplitFromByteArray(pData, 14, 3), &(pTmccInfo->currentInfo.layerA.ilLength));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    pTmccInfo->currentInfo.layerA.segmentsNum = (uint8_t)sony_BitSplitFromByteArray(pData, 17, 4);

    result = convertTmccModulation((uint8_t)sony_BitSplitFromByteArray(pData, 21, 3), &(pTmccInfo->currentInfo.layerB.modulation));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = convertTmccCodingRate((uint8_t)sony_BitSplitFromByteArray(pData, 24, 3), &(pTmccInfo->currentInfo.layerB.codingRate));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = convertTmccIlLength((uint8_t)sony_BitSplitFromByteArray(pData, 27, 3), &(pTmccInfo->currentInfo.layerB.ilLength));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    pTmccInfo->currentInfo.layerB.segmentsNum = (uint8_t)sony_BitSplitFromByteArray(pData, 30, 4);

    result = convertTmccModulation((uint8_t)sony_BitSplitFromByteArray(pData, 34, 3), &(pTmccInfo->currentInfo.layerC.modulation));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = convertTmccCodingRate((uint8_t)sony_BitSplitFromByteArray(pData, 37, 3), &(pTmccInfo->currentInfo.layerC.codingRate));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = convertTmccIlLength((uint8_t)sony_BitSplitFromByteArray(pData, 40, 3), &(pTmccInfo->currentInfo.layerC.ilLength));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    pTmccInfo->currentInfo.layerC.segmentsNum = (uint8_t)sony_BitSplitFromByteArray(pData, 43, 4);

    /* Next */
    pTmccInfo->nextInfo.isPartial = (uint8_t)sony_BitSplitFromByteArray(pData, 47, 1);

    result = convertTmccModulation((uint8_t)sony_BitSplitFromByteArray(pData, 48, 3), &(pTmccInfo->nextInfo.layerA.modulation));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = convertTmccCodingRate((uint8_t)sony_BitSplitFromByteArray(pData, 51, 3), &(pTmccInfo->nextInfo.layerA.codingRate));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = convertTmccIlLength((uint8_t)sony_BitSplitFromByteArray(pData, 54, 3), &(pTmccInfo->nextInfo.layerA.ilLength));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    pTmccInfo->nextInfo.layerA.segmentsNum = (uint8_t)sony_BitSplitFromByteArray(pData, 57, 4);

    result = convertTmccModulation((uint8_t)sony_BitSplitFromByteArray(pData, 61, 3), &(pTmccInfo->nextInfo.layerB.modulation));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = convertTmccCodingRate((uint8_t)sony_BitSplitFromByteArray(pData, 64, 3), &(pTmccInfo->nextInfo.layerB.codingRate));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = convertTmccIlLength((uint8_t)sony_BitSplitFromByteArray(pData, 67, 3), &(pTmccInfo->nextInfo.layerB.ilLength));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    pTmccInfo->nextInfo.layerB.segmentsNum = (uint8_t)sony_BitSplitFromByteArray(pData, 70, 4);

    result = convertTmccModulation((uint8_t)sony_BitSplitFromByteArray(pData, 74, 3), &(pTmccInfo->nextInfo.layerC.modulation));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = convertTmccCodingRate((uint8_t)sony_BitSplitFromByteArray(pData, 77, 3), &(pTmccInfo->nextInfo.layerC.codingRate));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    result = convertTmccIlLength((uint8_t)sony_BitSplitFromByteArray(pData, 80, 3), &(pTmccInfo->nextInfo.layerC.ilLength));
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    pTmccInfo->nextInfo.layerC.segmentsNum = (uint8_t)sony_BitSplitFromByteArray(pData, 83, 4);

    SONY_TRACE_RETURN(result);
}
#endif


