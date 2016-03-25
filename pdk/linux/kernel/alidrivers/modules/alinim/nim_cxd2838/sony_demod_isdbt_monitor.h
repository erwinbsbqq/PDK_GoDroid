/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : 2013-08-07
  File Revision : 1.0.7.0
------------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_ISDBT_MONITOR_H
#define SONY_DEMOD_ISDBT_MONITOR_H

#include "sony_common.h"
#include "sony_demod_isdbt.h"

/*------------------------------------------------------------------------------
  Enumerations
------------------------------------------------------------------------------*/
/**
 @brief Target ISDB-T hierarchical layer enumuration.
        This enum is used as a argument of monitor functions to specify the
        target layer.
*/
typedef enum {
    SONY_ISDBT_MONITOR_TARGET_LAYER_A = 0,    /**< Layer A */
    SONY_ISDBT_MONITOR_TARGET_LAYER_B,        /**< Layer B */
    SONY_ISDBT_MONITOR_TARGET_LAYER_C,        /**< Layer C */
    SONY_ISDBT_MONITOR_TARGET_LAYER_UNKNOWN   /**< Unknown */
} sony_isdbt_monitor_target_t;

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Monitor the IFAGC value of the demodulator.
        Actual dB gain dependent on attached tuner.

 @param pDemod  The demodulator instance.
 @param pIFAGC  The IFAGC output register value. Range 0x000 - 0xFFF. Unitless.

 @return SONY_RESULT_OK if pIFAGC is valid.
*/
sony_result_t sony_demod_isdbt_monitor_IFAGCOut(sony_demod_t* pDemod,
                                                uint32_t*     pIFAGC);

/**
 @brief
 @param pDemod               The demodulator instance.
 @param pInternalDigitalAGC  The Internal Digital AGC output register value.
                             Range 0x000 - 0x3FFF. Unitless.
*/
sony_result_t sony_demod_isdbt_monitor_InternalDigitalAGCOut(sony_demod_t* pDemod,
                                                             uint32_t*     pInternalDigitalAGC);


/**
 @brief Monitors the synchronisation state of the demodulator.

 @param pDemod   The demodulator instance.
 @param pDmdLock Address of demodulator lock flag
                 - 0: Not lock
                 - 1: Lock
 @param pTSLock  Indicates the TS lock condition.
                 - 0: TS not locked.
                 - 1: TS locked.
 @param pUnlock  Early unlock condition
                 - 0: No early unlock.
                 - 1: Early unlock detected.

 @return SONY_RESULT_OK if pSyncStat, pTSLockStat is valid, otherwise an error is returned.
*/
sony_result_t sony_demod_isdbt_monitor_SyncStat(sony_demod_t* pDemod,
                                                uint8_t*      pDmdLock,
                                                uint8_t*      pTsLock,
                                                uint8_t*      pUnlock);
/**
 @brief Monitors the spectrum sense used by the demodulator.

 @param pDemod  The demodulator instance.
 @param pSense  The detected spectrum sense.

 @return SONY_RESULT_OK if successful and pSense is valid.
*/
sony_result_t sony_demod_isdbt_monitor_SpectrumSense(sony_demod_t*                 pDemod,
                                                     sony_demod_spectrum_sense_t * pSense);

/**
 @brief Monitor the carrier offset of the currently tuned channel.
        This uses the continual pilot (CP) carrier offset estimation
        from the device.
        To get the estimated center frequency of the current channel:
        Fest = Ftune (Hz) + pOffset (Hz) ;

 @param pDemod  The demodulator instance.
 @param pOffset The detected carrier offset in Hz.

 @return SONY_RESULT_OK if successful and pOffset is valid.
*/
sony_result_t sony_demod_isdbt_monitor_CarrierOffset(sony_demod_t* pDemod,
                                                     int32_t*      pOffset);

/**
 @brief Monitor the SNR of the ISDB-T demodulator.

 @param pDemod  The demodulator instance.
 @param pSNR    The returned SNR in dB x 1000.

 @return SONY_RESULT_OK if successful and pSNR is valid.
*/
sony_result_t sony_demod_isdbt_monitor_SNR(sony_demod_t* pDemod,
                                           int32_t*      pSNR);
/**
 @brief Monitor the detected mode/guard.

 @param pDemod  The demodulator instance.
 @param pMode   Mode estimation result.
 @param pGuard  Guard interval estimation result.

 @return SONY_RESULT_OK if successful and pMode, pGuard are valid.
*/
sony_result_t sony_demod_isdbt_monitor_ModeGuard(sony_demod_t*       pDemod,
                                                 sony_isdbt_mode_t*  pMode,
                                                 sony_isdbt_guard_t* pGuard);
/**
 @brief Monitor the Pre-RS BER.

 @param pDemod  The demodulator instance.
 @target Target layer of BER estimation.
 @param pBER    BER value (Pre reed solomon decoder) x 1e7.

 @return SONY_RESULT_OK if successful and pBER is valid.
*/
sony_result_t sony_demod_isdbt_monitor_PreRSBER(sony_demod_t*               pDemod,
                                                sony_isdbt_monitor_target_t target,
                                                uint32_t*                   pBER);

/**
 @brief Monitor the PER (Packet Error Rate) parameters.

 @param pDemod  The demod instance.
 @param target  Target layer of PER estimation.
 @param pPER    The estimated PER x 1e6.
 
 @return SONY_RESULT_OK if successful and pPER is valid.
*/
sony_result_t sony_demod_isdbt_monitor_PER(sony_demod_t*               pDemod,
                                           sony_isdbt_monitor_target_t target,
                                           uint32_t*                   pPER);

/**
  @brief Monitor the TMCC informations

  @param pDemod    The demod instance.
  @param pTMCCInfo TMCC information struct instance.

  @return SONY_RESULT_OK if successful and pTMCCInfo is valid.
*/
sony_result_t sony_demod_isdbt_monitor_TMCCInfo(sony_demod_t*           pDemod,
                                                sony_isdbt_tmcc_info_t* pTMCCInfo);

/**
  @brief Monitor the preset informations

  @param pDemod      The demod instance.
  @param pPresetInfo Preset information struct instance.

  @return SONY_RESULT_OK if successful and pPresetInfo is valid.
*/
sony_result_t sony_demod_isdbt_monitor_PresetInfo(sony_demod_t*                   pDemod,
                                                  sony_demod_isdbt_preset_info_t* pPresetInfo);

/**
 @brief Monitors the number RS (Reed Solomon) errors detected by the 
        RS decoder over 1 second. Also known as the code word reject count.

 @param pDemod          The demodulator instance.
 @param target          Target layer to get packet error number.
 @param pPacketErrorNum Returned packet error number.

 @return SONY_RESULT_OK if successful and pPacketErrorNum is valid.
*/
sony_result_t sony_demod_isdbt_monitor_PacketErrorNumber(sony_demod_t*               pDemod,
                                                         sony_isdbt_monitor_target_t target,
                                                         uint32_t*                   pPacketErrorNum);

/**
  @brief Monitor the AC EEW (Earthquake Early Warning by AC signal) informations

  @param pDemod     pDemod the demod instance.
  @param pIsExist   ACEEW exist flag.
                    If you get "0", ACEEW information is not exist.
                    If you get "1", ACEEW information is exist, please check pACEEWInfo.
  @param pACEEWInfo ACEEW Information struct instance.

  @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbt_monitor_ACEEWInfo(sony_demod_t*            pDemod,
                                                 uint8_t*                 pIsExist,
                                                 sony_isdbt_aceew_info_t* pACEEWInfo);

#endif /* SONY_DEMOD_ISDBT_MONITOR_H */
