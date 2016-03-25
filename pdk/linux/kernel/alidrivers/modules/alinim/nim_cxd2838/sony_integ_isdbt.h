/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : 2014-09-02
  File Revision : 1.0.8.0
------------------------------------------------------------------------------*/

#ifndef SONY_INTEG_ISDBT_H
#define SONY_INTEG_ISDBT_H

#include "sony_demod_isdbt.h"
//#include "sony_tuner_isdbt.h"


/*------------------------------------------------------------------------------
  Defines
------------------------------------------------------------------------------*/
#define SONY_ISDBT_WAIT_DEMOD_LOCK          1500    /**< 1.5s timeout for wait demodulator lock process for ISDB-T channels */
#define SONY_ISDBT_WAIT_TS_LOCK             1500    /**< 1.5s timeout for wait TS lock process for ISDB-T channels */
#define SONY_ISDBT_WAIT_LOCK_INTERVAL       10      /**< 10ms polling interval for demodulator and TS lock functions */

/*------------------------------------------------------------------------------
  Structs
------------------------------------------------------------------------------*/
/**
 @brief The high level driver object.
        This is the primary interface used for controlling the demodulator and 
        connected tuner devices.

        This object is the combination of the demodulator and the tuner devices
        to represent a single front end system. It can be used wholly, partly or 
        just as a reference for developing the application software.
*/

typedef struct sony_integ_t {
    sony_demod_t * pDemod;                /**< Instance of the demodulator. */
    /*sony_tuner_isdbt_t * pTuner; */         /**< The connected tuner for ISDB-T systems. */

    sony_atomic_t cancel;                 /**< Cancellation indicator variable. */

    void * user;                          /**< User data. */
} sony_integ_t;

/**
 @brief The parameters used for scanning.
*/
typedef struct sony_integ_isdbt_scan_param_t {
    /**
     @brief The start frequency in kHz for scanning
    */
    uint32_t startFrequencyKHz;

    /**
     @brief The end frequency in kHz for scanning
    */
    uint32_t endFrequencyKHz;

    /**
     @brief The step frequency in kHz for scanning
    */
    uint32_t stepFrequencyKHz;

    /**
     @brief The bandwidth to use for tuning during the scan
    */
    sony_demod_bandwidth_t bandwidth;
} sony_integ_isdbt_scan_param_t;

/**
 @brief The structure used to return a channel located or progress update 
        as part of scan.
*/
typedef struct sony_integ_isdbt_scan_result_t {
    /**
     @brief Indicates the current frequency just attempted for the scan.  This would
            primarily be used to calculate scan progress from the scan parameters.
    */
    uint32_t centerFreqKHz;

    /**
     @brief Indicates if the tune result at the current frequency.  SONY_RESULT_OK
            means that a channel is locked and the tuneParam structure contains 
            the channel infomration.
    */
    sony_result_t tuneResult;

    /**
     @brief The tune params for located ISDB-T channel.
    */
    sony_isdbt_tune_param_t tuneParam;
} sony_integ_isdbt_scan_result_t;

/*------------------------------------------------------------------------------
  Function Pointers
------------------------------------------------------------------------------*/
/**
 @brief Callback function that is called for every attempted frequency during a 
        scan.  For successful channel results the function is called after 
        demodulator lock but before TS lock is achieved.
 
 @param pInteg The driver instance.
 @param pResult The current scan result.
 @param pScanParam The current scan parameters.
*/
typedef void (*sony_integ_isdbt_scan_callback_t)(sony_integ_t*                   pInteg,
                                                 sony_integ_isdbt_scan_result_t* pResult,
                                                 sony_integ_isdbt_scan_param_t*  pScanParam);

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Construct the driver.

        This function is called by the application in order to setup the 
        ::sony_demod_t structure and provide references for the 
        ::sony_integ_t::pDemod and ::sony_integ_t::pTuner members.

        This MUST be called before calling ::sony_integ_Initialize

 @note  Passing NULL as pTuner will disable communication
        with the device without throwing an arguement error.

        Notes on driver configuration:
        - By default, the demodulator uses an inverted AGC.  If the connected 
          tuner uses a non-inverted IFAGC, call ::sony_demod_SetConfig with 
          ::SONY_DEMOD_CONFIG_IFAGCNEG = 0 to setup the demodulator with postive 
          IFAGC sense, after calling ::sony_integ_Initialize.

 @note  Memory is not allocated dynamically.

 @param pInteg The driver object to create. It must be a valid pointer to 
        allocated memory for a ::sony_integ_t structure.
 @param xtalFreq The frequency of the demod crystal.
 @param i2cAddress The demod I2C address in 8-bit form.
 @param pDemodI2c The I2C driver that the demod will use as the I2C interface.
 @param pDemod Reference to memory allocated for the demodulator instance. The 
        create function will setup this demodulator instance also.
 @param pTuner The tuner driver to use with this instance of the driver.
        Note : The tuner I2C interface should have been setup before this call.
 
 @return SONY_RESULT_OK if successfully created integration layer driver structure.
*/
sony_result_t sony_integ_Create (sony_integ_t * pInteg,
                                 sony_demod_xtal_t xtalFreq,
                                 uint8_t i2cAddress,
                                 sony_i2c_t * pDemodI2c,
                                 sony_demod_t * pDemod
                                 /*sony_tuner_isdbt_t * pTuner*/);

/**
 @brief Initialize the demodulator and tuner.  After this operation,
        the demodulator and tuner are in a low power state (demod 
        state = ::SONY_DEMOD_STATE_SLEEP) awaiting tune or scan commands.

        Should only be used from Power On (::SONY_DEMOD_STATE_UNKNOWN).  Calling 
        from Active or Shutdown states will cause a demodulator reset, clearing
        all current configuration settings.

 @param pInteg The driver instance.

 @return SONY_RESULT_OK if OK.
*/
sony_result_t sony_integ_Initialize(sony_integ_t* pInteg);

/**
 @brief Put the demodulator and tuner devices into a low power state.
        This function can be called from SHUTDOWN or ACTIVE states.
        In ::SONY_DEMOD_STATE_SLEEP GPIO and demodulator configuration options are 
        available.  Calling this function from ::SONY_DEMOD_STATE_SLEEP will 
        reconfigure the demodulator for use with ISDB-T.

 @param pInteg The driver instance.

 @return SONY_RESULT_OK if OK.
*/
sony_result_t sony_integ_Sleep(sony_integ_t* pInteg);

/**
 @brief Shutdown the demodulator and tuner parts into a low power disabled state.
        ::sony_integ_Shutdown can be directly called from SLEEP or ACTIVE states.

 @param pInteg The driver instance.

 @return SONY_RESULT_OK if OK.
*/
sony_result_t sony_integ_Shutdown(sony_integ_t* pInteg);

/**
 @brief Cancels current Tune or Scan operation in the demod and tuner parts.
        This function is thread safe, calling thread will get the result
        SONY_RESULT_ERROR_CANCEL.

 @param pInteg The driver instance.

 @return SONY_RESULT_OK if able to cancel the pending operation.
*/
sony_result_t sony_integ_Cancel(sony_integ_t* pInteg);

/**
 @brief Performs acquisition to a ISDB-T channel. 
        Blocks the calling thread until the TS has locked or has timed out.
        Use ::sony_integ_Cancel to cancel the operation at any time.

 @param pInteg The driver instance.
 @param pTuneParam The parameters required for the tune.

 @return SONY_RESULT_OK if tuned successfully to the channel.
*/
sony_result_t sony_integ_isdbt_Tune(sony_demod_t * pDemod, sony_isdbt_tune_param_t* pTuneParam);

/**
 @brief Performs acquisition to a ISDB-T channel but in EWS mode.
        In EWS mode, TS is not output and the demod waiting EWS flag.
        Blocks the calling thread until the Demod has locked or has timed out.
        Use ::sony_integ_Cancel to cancel the operation at any time.

 @param pInteg The driver instance.
 @param pTuneParam The parameters required for the tune.

 @return SONY_RESULT_OK if tuned successfully to the channel.
*/
sony_result_t sony_integ_isdbt_EWSTune(sony_integ_t* pInteg, sony_isdbt_tune_param_t* pTuneParam);

/**
 @brief Performs a scan over the spectrum specified. 

        Blocks the calling thread while scanning. Use ::sony_integ_Cancel to cancel 
        the operation at any time.
 
 @param pInteg The driver instance.
 @param pScanParam The scan parameters.
 @param callback User registered call-back to receive scan progress information and 
        notification of found channels. The call back is called for every attempted 
        frequency during a scan.

 @return SONY_RESULT_OK if scan completed successfully.
        
*/
sony_result_t sony_integ_isdbt_Scan(sony_integ_t*                    pInteg,
                                    sony_integ_isdbt_scan_param_t*   pScanParam,
                                    sony_integ_isdbt_scan_callback_t callback);

/**
 @brief Polls the demodulator waiting for Demod lock

 @param pInteg The driver instance

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_isdbt_WaitDemodLock(sony_demod_t * pDemod);

/**
 @brief Polls the demodulator waiting for TS lock

 @param pInteg The driver instance

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_isdbt_WaitTSLock(sony_demod_t * pDemod);

/**
 @brief This function returns the estimated RF level based on demodulator gain measurements
        and a tuner dependant conversion calculation. The calculation provided in this monitor
        may require modifications for your own HW integration.

 @param pInteg The driver instance
 @param pRFLeveldB The RF Level estimation in dB * 1000

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_integ_isdbt_monitor_RFLevel(sony_integ_t * pInteg, int32_t * pRFLeveldB);


#endif /* SONY_INTEG_ISDBT_H */
