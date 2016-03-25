/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2012-06-28 18:05:36 #$
  File Revision : $Revision:: 5686 $
------------------------------------------------------------------------------*/
/**
 @file    sony_tuner_terr_cable.h

          This file provides the tuner control interface for Terr/Cable.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_TUNER_TERR_CABLE_H
#define SONY_TUNER_TERR_CABLE_H

#include "sony_common.h"
#include "sony_i2c.h"
#include "sony_dtv.h"
//#include "sony_demod.h"
/**
 @brief System bandwidth.
*/
typedef enum {
    SONY_DEMOD_BW_UNKNOWN = 0,          /**< Unknown bandwidth */
    SONY_DEMOD_BW_1_7_MHZ = 1,          /**< 1.7MHz bandwidth (Valid option for DVB-T2 only) */
    SONY_DEMOD_BW_5_MHZ = 5,            /**< 5MHz bandwidth (Valid option for DVB-T / T2) */
    SONY_DEMOD_BW_6_MHZ = 6,            /**< 6MHz bandwidth (Valid option for DVB-T / T2 / C2) */
    SONY_DEMOD_BW_7_MHZ = 7,            /**< 7MHz bandwidth (Valid option for DVB-T / T2) */
    SONY_DEMOD_BW_8_MHZ = 8             /**< 8MHz bandwidth (Valid option for DVB-T / T2 / C / C2) */
} sony_demod_bandwidth_t;


/**
 @brief Definition of the Terrestrial / Cable tuner driver API.
*/
typedef struct sony_tuner_terr_cable_t {
    uint8_t i2cAddress;                 /**< I2C address. */
    sony_i2c_t * pI2c;                  /**< I2C driver instance. */
    uint32_t flags;                     /**< Flags that can be used by tuner drivers. */
    uint32_t frequencyKHz;              /**< Current RF frequency(kHz) tuned. */
    sony_dtv_system_t system;           /**< Current system tuned. */
    sony_demod_bandwidth_t bandwidth;   /**< Cuurent bandwidth tuned. */

    /**
     @brief Initialize the tuner.

     @param pTuner Instance of the tuner driver.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*Initialize) (struct sony_tuner_terr_cable_t * pTuner);

    /**
     @brief Tune to a given frequency with a given bandwidth.
            Tuner driver can block while waiting for PLL locked indication (if required).

     @param pTuner Instance of the tuner driver.
     @param centerFrequencyKHz RF frequency to tune too (kHz)
     @param system The type of channel to tune too (DVB-T/C/T2).
     @param bandwidth The bandwidth of the channel.
            - DVB-T / T2: 5, 6, 7 or 8MHz.
            - DVB-C: 8MHz only.
            - DVB-C2: 6 or 8MHz

     @return SONY_RESULT_OK if tuner successfully tuned.
    */
    sony_result_t (*Tune) (struct sony_tuner_terr_cable_t * pTuner,
                           uint32_t centerFrequencyKHz,
                           sony_dtv_system_t system,
                           sony_demod_bandwidth_t bandwidth);

    /**
     @brief Sleep the tuner device (if supported).
     
     @param pTuner Instance of the tuner driver.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*Sleep) (struct sony_tuner_terr_cable_t * pTuner);

    /**
     @brief Shutdown the tuner device (if supported).
     
     @param pTuner Instance of the tuner driver.

     @return SONY_RESULT_OK if successful.
    */
    sony_result_t (*Shutdown) (struct sony_tuner_terr_cable_t * pTuner);
	/**
	   @brief Read RF level from the tuner device (if supported).
	
	   @param pTuner Instance of the tuner driver.
	   @param pRFLevel The RF Level estimation in dB * 1000.
	
	   @return SONY_RESULT_OK if tuner successfully tuned.
	  */
	  sony_result_t (*ReadRFLevel) (struct sony_tuner_terr_cable_t * pTuner,
									int32_t *pRFLevel);
	
	  /**
	   @brief Calculate RF level from demod AGC monitor value (if supported).
	
			  Note: This function should not access to tuner.
	
	   @param pTuner Instance of the tuner driver.
	   @param agcValue Sony demodulator AGC value.
	   @param pRFLevel The RF Level estimation in dB * 1000.
	
	   @return SONY_RESULT_OK if tuner successfully tuned.
	  */
	  sony_result_t (*CalcRFLevelFromAGC) (struct sony_tuner_terr_cable_t * pTuner,
										   uint32_t agcValue,
										   int32_t *pRFLevel);

    void * user;                /**< User defined data. */
} sony_tuner_terr_cable_t;

#endif /* SONY_TUNER_TERR_CABLE_H */
