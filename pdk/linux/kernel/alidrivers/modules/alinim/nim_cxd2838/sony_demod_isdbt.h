/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : 2014-09-02
  File Revision : 1.0.12.0
------------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_ISDBT_H
#define SONY_DEMOD_ISDBT_H

#include "sony_common.h"
#include "sony_i2c.h"
#include "sony_isdbt.h"
#include "sony_demod.h"



/*------------------------------------------------------------------------------
  Enumerations
------------------------------------------------------------------------------*/
/**
 @brief System (DVB-T/T2/C/C2/S/S2) 
*/
typedef enum {
    SONY_DTV_SYSTEM_UNKNOWN,        /**< Unknown. */
    SONY_DTV_SYSTEM_DVBT,           /**< DVB-T. */
    SONY_DTV_SYSTEM_DVBT2,          /**< DVB-T2. */
    SONY_DTV_SYSTEM_DVBC,           /**< DVB-C. */
    SONY_DTV_SYSTEM_DVBC2,          /**< DVB-C2. */
    SONY_DTV_SYSTEM_DVBS,           /**< DVB-S. */
    SONY_DTV_SYSTEM_DVBS2,          /**< DVB-S2. */
    SONY_DTV_SYSTEM_ISDBT,          /**< ISDBT. */
    SONY_DTV_SYSTEM_ANY             /**< Used for multiple system scanning / blind tuning */
} sony_dtv_system_t;


/*------------------------------------------------------------------------------
  Defines
------------------------------------------------------------------------------*/
/**
 @brief Calculate the demodulator IF Freq setting ::sony_demod_t::iffreqConfig.
        ((IFFREQ/Sampling Freq at Down Converter DSP module) * Down converter's dynamic range + 0.5
*/
#define SONY_DEMOD_MAKE_IFFREQ_CONFIG(xTal, iffreq) ((xTal == SONY_DEMOD_XTAL_24000KHz) ? ((uint32_t)(((iffreq)/48.0)*16777216.0 + 0.5)) : ((uint32_t)(((iffreq)/41.0)*16777216.0 + 0.5)))


#define SONY_DEMOD_MAX_CONFIG_MEMORY_COUNT 100 /**< The maximum number of entries in the configuration memory table */

/**
 @brief Freeze all registers in the SLV-T device.  This API is used by the monitor functions to ensure multiple separate 
        register reads are from the same snapshot 

 @note This should not be manually called or additional instances added into the driver unless under specific instruction.
*/
#define SLVT_FreezeReg(pDemod) ((pDemod)->pI2c->WriteOneRegister ((pDemod)->pI2c, (pDemod)->i2cAddressSLVT, 0x01, 0x01))

/**
 @brief Unfreeze all registers in the SLV-T device 
*/
#define SLVT_UnFreezeReg(pDemod) ((void)((pDemod)->pI2c->WriteOneRegister ((pDemod)->pI2c, (pDemod)->i2cAddressSLVT, 0x01, 0x00)))

/*------------------------------------------------------------------------------
  Enumerations
------------------------------------------------------------------------------*/
/**
 @brief Demodulator crystal frequency.
*/
typedef enum {
    SONY_DEMOD_XTAL_20500KHz,          /**< 20.5 MHz */
    SONY_DEMOD_XTAL_24000KHz,          /**< 24 MHz */
    SONY_DEMOD_XTAL_41000KHz           /**< 41 MHz */
} sony_demod_xtal_t;

/**
 @brief The demodulator Chip ID mapping.
*/
typedef enum {
    SONY_DEMOD_CHIP_ID_CXD2838 = 0xB0, /**< CXD2838 ISDB-T */
    SONY_DEMOD_CHIP_ID_CXD2828 = 0x22  /**< CXD2828 ISDB-T */
} sony_demod_chip_id_t;

/**
 @brief Demodulator software state.
*/
typedef enum {
    SONY_DEMOD_STATE_UNKNOWN = 0,   /**< Unknown. */
    SONY_DEMOD_STATE_SHUTDOWN,      /**< Chip is in Shutdown state */
    SONY_DEMOD_STATE_SLEEP,         /**< Chip is in Sleep state */
    SONY_DEMOD_STATE_ACTIVE,        /**< Chip is in Active state */
    SONY_DEMOD_STATE_EWS,           /**< Chip is in EWS state */
    SONY_DEMOD_STATE_INVALID        /**< Invalid, result of an error during a state change. */
} sony_demod_state_t;

/**
 @brief System bandwidth.
*/
typedef enum {
    SONY_DEMOD_BW_UNKNOWN = 0,          /**< Unknown bandwidth */
    SONY_DEMOD_BW_6_MHZ = 6,            /**< 6MHz bandwidth */
    SONY_DEMOD_BW_7_MHZ = 7,            /**< 7MHz bandwidth */
    SONY_DEMOD_BW_8_MHZ = 8             /**< 8MHz bandwidth */
} sony_demod_bandwidth_t;

/**
 @brief Enumeration of supported sony tuner models used for optimising the 
        demodulator configuration.
*/
typedef enum {
    SONY_DEMOD_TUNER_OPTIMIZE_UNKNOWN = 0,  /**< Non-Sony Tuner. */
    SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2D,      /**< Sony ASCOT2D derived tuners. */
    SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2E,      /**< Sony ASCOT2E derived tuners. */
    SONY_DEMOD_TUNER_OPTIMIZE_ASCOT3        /**< Sony ASCOT3  derived tuners. */
} sony_demod_tuner_optimize_t;

/**
 @brief Enumeration of spectrum inversion monitor values.
*/
typedef enum {
    SONY_DEMOD_SPECTRUM_NORMAL = 0,          /**< Spectrum normal sense. */
    SONY_DEMOD_SPECTRUM_INV                  /**< Spectrum inverted. */
} sony_demod_spectrum_sense_t;

/**
 @brief Configuration options for the demodulator.
*/
typedef enum {
    /**
     @brief Parallel or serial TS output selection.

            Value:
            - 0: Serial output.
            - 1: Parallel output (Default).
    */
    SONY_DEMOD_CONFIG_PARALLEL_SEL,

    /**
     @brief Serial output pin of TS data.

            Value:
            - 0: Output from TSDATA0
            - 1: Output from TSDATA7 (Default).
    */
    SONY_DEMOD_CONFIG_SER_DATA_ON_MSB,

    /**
     @brief Parallel/Serial output bit order on TS data.

            Value (Parallel):
            - 0: MSB TSDATA[0]
            - 1: MSB TSDATA[7] (Default).
            Value (Serial):
            - 0: LSB first
            - 1: MSB first (Default).
    */
    SONY_DEMOD_CONFIG_OUTPUT_SEL_MSB,

    /**
     @brief TS valid active level.

            Value:
            - 0: Valid low.
            - 1: Valid high (Default).
    */
    SONY_DEMOD_CONFIG_TSVALID_ACTIVE_HI,

    /**
     @brief TS sync active level.

            Value:
            - 0: Valid low.
            - 1: Valid high (Default).
    */
    SONY_DEMOD_CONFIG_TSSYNC_ACTIVE_HI,

    /**
     @brief TS error active level.

            Value:
            - 0: Valid low.
            - 1: Valid high (Default).
    */
    SONY_DEMOD_CONFIG_TSERR_ACTIVE_HI,

    /**
     @brief TS clock inversion setting.

            Value:
            - 0: Falling/Negative edge.
            - 1: Rising/Positive edge (Default).
    */
    SONY_DEMOD_CONFIG_LATCH_ON_POSEDGE,

    /**
     @brief TS clock gated on valid TS data or is continuous.

            Value:
            - 0: Gated
            - 1: Continuous (Default)
    */
    SONY_DEMOD_CONFIG_TSCLK_CONT,

    /**
     @brief Disable/Enable TS clock during specified TS region.

            bit flags: ( can be bitwise ORed )
            - 0 : Always Active (default)
            - 1 : Disable during TS packet gap
            - 2 : Disable during TS parity
            - 4 : Disable during TS payload
            - 8 : Disable during TS header
            - 16: Disable during TS sync
    */
    SONY_DEMOD_CONFIG_TSCLK_MASK,

    /**
     @brief Disable/Enable TSVALID during specified TS region.

            bit flags: ( can be bitwise ORed )
            - 0 : Always Active 
            - 1 : Disable during TS packet gap (default)
            - 2 : Disable during TS parity (default)
            - 4 : Disable during TS payload
            - 8 : Disable during TS header
            - 16: Disable during TS sync
    */
    SONY_DEMOD_CONFIG_TSVALID_MASK,

    /**
     @brief Disable/Enable TSERR during specified TS region.

            bit flags: ( can be bitwise ORed )
            - 0 : Always Active (default)
            - 1 : Disable during TS packet gap
            - 2 : Disable during TS parity
            - 4 : Disable during TS payload
            - 8 : Disable during TS header
            - 16: Disable during TS sync
    */
    SONY_DEMOD_CONFIG_TSERR_MASK,

    /**
     @brief Configure the driving curent for the TS Clk pin.

            - 0 : 8mA (Default)
            - 1 : 10mA
    */
    SONY_DEMOD_CONFIG_TSCLK_CURRENT_10mA,

    /**
     @brief Configure the driving curent for the TS Sync / TS Valid 
            / TS Data / TS Error pins.

            - 0 : 8mA (Default)
            - 1 : 10mA
    */
    SONY_DEMOD_CONFIG_TS_CURRENT_10mA,

    /**
     @brief Configure the clock frequency for Serial TS.
            Value is stored in demodulator structure to be applied during Sleep to Active
            transition.  
            Only valid when SONY_DEMOD_CONFIG_PARALLEL_SEL = 0 (serial TS).

            For Xtal = 41MHz or 20.5MHz
            - 0 : Invalid
            - 1 : 97.38MHz (Default)
            - 2 : 77.90MHz
            - 3 : 64.92MHz
            - 4 : 48.69MHz
            - 5 : 38.95MHz

            For Xtal = 24MHz
            - 0 : Invalid
            - 1 : 96.00MHz (Default)
            - 2 : 76.80MHz
            - 3 : 64.00MHz
            - 4 : 48.00MHz
            - 5 : 38.40MHz
    */
    SONY_DEMOD_CONFIG_TS_SERIAL_CLK_FREQ,

    /**
     @brief This configuration can be used to configure the demodulator to output a TS waveform that is
            backwards compatible with previous generation demodulators (CXD2828). 
            This option should not be used unless specifically required to overcome a HW configuration issue.

            The demodulator will have the following settings, which will override any prior individual 
            configuration:
            - Disable TS packet gap insertion.
            - Parallel TS maximum bit rate of 97.38MBps
            - Serial TS clock frequency fixed at 97.38MHz

            Values:
            - 0 : Backwards compatible mode disabled
            - 1 : Backwards compatible mode enabled

    */
    SONY_DEMOD_CONFIG_TS_BACKWARDS_COMPATIBLE,

    /**
    @brief Set the TS clock rate (data period) manually.
           *NOT recommended for normal use (testing/debug only).*
           If SONY_DEMOD_CONFIG_PARALLEL_SEL = 0 (serial TS), then this configuration will
           have no effect.
           If SONY_DEMOD_CONFIG_TS_AUTO_RATE_ENABLE = 1, then this configuration will have no
           effect.
           Value:
           - Desired TS clock rate (data period) in kHz in the range 320KHz (2.56Mbps) 
           to 16000KHz (128Mbps)
    */
    SONY_DEMOD_CONFIG_TSIF_SDPR,

    /**
     @brief Enable or disable the auto TS clock rate (data period).
            *NOT recommended for normal use (testing/debug only).*
            If DEMOD_CONFIG_PARALLEL_SEL = 0 (serial TS), then this configuration will
            have no effect.
            Value:
            - 0: Disable the TS auto rate.
                 TS clock rate = SONY_DEMOD_CONFIG_TSIF_SDPR.
            - 1: Enable the TS auto rate (Default).
                 TS clock rate is automatic.

            @note Not available in DVB-C mode.
    */
    SONY_DEMOD_CONFIG_TS_AUTO_RATE_ENABLE,

    /**
    @brief Writes a 12-bit value to the PWM output.
        Please note the actual PWM precision.
        - 12-bit.
        0xFFF => DVDD
        0x000 => GND
        
        This configuration is available only while the device
        is in ACTIVE mode.
    */
    SONY_DEMOD_CONFIG_PWM_VALUE,

    /**
     @brief IFAGC sense configuration.

            Value:
            - 0: Positive IFAGC.
            - 1: Inverted IFAGC (Default)
    */
    SONY_DEMOD_CONFIG_IFAGCNEG,

    /**
     @brief Configure the full-scale range of the ADC input to the IFAGC.

            Value:
            - 0: 1.4Vpp (Default)
            - 1: 1.0Vpp
            - 2: 0.7Vpp
    */
    SONY_DEMOD_CONFIG_IFAGC_ADC_FS,

    /**
    @brief Spectrum inversion configuration for the terrestrial tuner. 
            
            Value: 
            - 0: Normal (Default).
            - 1: Inverted.
    */
    SONY_DEMOD_CONFIG_SPECTRUM_INV,

    /**
     @brief Set Number of Measured Packets - 15bit
    */
    SONY_DEMOD_CONFIG_BERPER_PERIOD,

    /**
    @brief Configure which type of Emergency Warning flags is to be output to GPIO (ISDB-T only).
           The EWS flag can be used to inform the existence of EWS( Emergency Warning Broadcast System)
           or AC EEW (Earthquake Early Warning by AC signal) through the GPIO output.
           The possible settings for this configuration are as follows:

           Value:
            - 0: EWS (Default)
            - 1: AC EEW
            - 2: EWS or AC EEW
    */
    SONY_DEMOD_CONFIG_GPIO_EWS_FLAG
} sony_demod_config_t;

/**
 @brief Demodulator lock status.
*/
typedef enum {
    SONY_DEMOD_LOCK_RESULT_NOT_DETECT, /**< "Lock" or "Unlock" conditions not met */
    SONY_DEMOD_LOCK_RESULT_LOCK,       /**< "Lock" condition is found. */
    SONY_DEMOD_LOCK_RESULT_UNLOCK      /**< No signal was found or the signal was not the required system. */
} sony_demod_lock_result_t;

/**
 @brief Mode select for the multi purpose GPIO pins
*/
typedef enum {
    /** @brief GPIO pin is configured as an output */
    SONY_DEMOD_GPIO_MODE_OUTPUT = 0x00, 

    /** @brief GPIO pin is configured as an input */
    SONY_DEMOD_GPIO_MODE_INPUT = 0x01, 
        
    /** 
     @brief GPIO pin is configured to output an PWM signal which can be configured using the 
            ::sony_demod_SetConfig function with the config ID ::SONY_DEMOD_CONFIG_PWM_VALUE.
    */
    SONY_DEMOD_GPIO_MODE_PWM = 0x03, 

    /** @brief GPIO pin is configured to output TS error */
    SONY_DEMOD_GPIO_MODE_TS_ERROR = 0x04, 

    /** @brief GPIO pin is configured to EWS flag */
    SONY_DEMOD_GPIO_MODE_EWS_FLAG = 0x06
} sony_demod_gpio_mode_t;

/**
 @brief TS serial clock frequency options
*/
typedef enum {
    SONY_DEMOD_SERIAL_TS_CLK_HIGH_FULL,   /** High frequency, full rate */
    SONY_DEMOD_SERIAL_TS_CLK_MID_FULL,    /** Mid frequency, full rate */
    SONY_DEMOD_SERIAL_TS_CLK_LOW_FULL,    /** Low frequency, full rate */
    SONY_DEMOD_SERIAL_TS_CLK_HIGH_HALF,   /** High frequency, half rate */
    SONY_DEMOD_SERIAL_TS_CLK_MID_HALF,    /** Mid frequency, half rate */
    SONY_DEMOD_SERIAL_TS_CLK_LOW_HALF     /** Low frequency, half rate */
} sony_demod_serial_ts_clk_t;

/*------------------------------------------------------------------------------
  Structs
------------------------------------------------------------------------------*/
/**
 @brief List of register values for IF frequency configuration.  Used for handling 
        tuners that output a different IF depending on the expected channel BW.
        Should be set using ::SONY_DEMOD_MAKE_IFFREQ_CONFIG macro.
*/
typedef struct sony_demod_iffreq_config_t {
    uint32_t configIsdbt6;   /**< ISDB-T 6MHz */
    uint32_t configIsdbt7;   /**< ISDB-T 7MHz */
    uint32_t configIsdbt8;   /**< ISDB-T 8MHz */
} sony_demod_iffreq_config_t;

/**
 @brief The demodulator configuration memory table entry. Used to store a register or
        bit modification made through either the ::sony_demod_SetConfig or 
        ::sony_demod_SetAndSaveRegisterBits APIs.
*/
typedef struct {
    uint8_t slaveAddress;               /**< Slave address of register */
    uint8_t bank;                       /**< Bank for register */
    uint8_t registerAddress;            /**< Register address */
    uint8_t value;                      /**< Value to write to register */
    uint8_t bitMask;                    /**< Bit mask to apply on the value */
} sony_demod_config_memory_t;

/**
 @brief The demodulator definition which allows control of the demodulator device 
        through the defined set of functions. This portion of the driver is seperate 
        from the tuner portion and so can be operated independently of the tuner.
*/
#if 0
typedef struct sony_demod_t {
    /**
    @brief The demodulator crystal frequency.
    */
    sony_demod_xtal_t xtalFreq;

    /**
     @brief SLVT I2C address (8-bit form - 8'bxxxxxxx0).
    */
    uint8_t i2cAddressSLVT;

    /**
     @brief SLVX I2C address (8-bit form - 8'bxxxxxxx0). Fixed to i2cAddressSLVT + 4.
    */
    uint8_t i2cAddressSLVX;

    /**
     @brief I2C API instance.
    */
    sony_i2c_t * pI2c;

    /**
    @brief The driver operating state.
    */
    sony_demod_state_t state;

    /**
     @brief Auto detected chip ID at initialisation.
    */
    sony_demod_chip_id_t chipId;

    /**
     @brief The current bandwidth.
    */
    sony_demod_bandwidth_t bandwidth;

    /**
     @brief IF frequency configuration. Configure prior to initialization.
            Use the ::SONY_DEMOD_MAKE_IFFREQ_CONFIG macro for configuration.
    */
    sony_demod_iffreq_config_t iffreqConfig;

    /**
     @brief Stores the tuner model for demodulator specific optimisations.
    */
    sony_demod_tuner_optimize_t tunerOptimize;

    /**
     @brief The sense configured on the demodulator with
            ::sony_demod_SetConfig.
    */
    sony_demod_spectrum_sense_t confSense;

    /**
     @brief The serial TS clock mode for all active states.  This is configured using
            ::sony_demod_SetConfig with the SONY_DEMOD_CONFIG_TSCLK_CONT option.
    */
    uint8_t serialTsClockModeContinuous;

    /**
     @brief The serial TS clock frequency option for active states. 
            This is configured using ::sony_demod_SetConfig with the 
            SONY_DEMOD_CONFIG_TS_SERIAL_CLK_FREQ option.
    */
    sony_demod_serial_ts_clk_t serialTsClkFreq;

    /**
     @brief A table of the demodulator configuration changes stored from the 
            ::sony_demod_SetConfig and ::sony_demod_SetAndSaveRegisterBits functions.
    */
    sony_demod_config_memory_t configMemory[SONY_DEMOD_MAX_CONFIG_MEMORY_COUNT];

    /**
     @brief The index of the last valid entry in the configMemory table
    */
    uint8_t configMemoryLastEntry;

    /**
     @brief User defined data.
    */
    void * user;
} sony_demod_t;
#endif
typedef void (*LOG_STRING_FUNCTION)(char *string);

typedef struct sony_demod_t {

	/*Ali data start*/
	struct COFDM_TUNER_CONFIG_API tuner_control;
	uint32_t tuner_id;
	
	struct mutex  i2c_mutex_id;
	struct mutex  demodMode_mutex_id;
	struct mutex  flag_id;
	
	uint32_t Frequency;
	//UINT32 bandwidth;
	uint16_t if_freq;

	uint32_t autoscan_stop_flag           :1;
	uint32_t do_not_wait_t2_signal_locked :1;
	uint32_t reserved                     :30;

	uint8_t priority;         //for DVB-T

	uint8_t t2_signal;        //0:DVB-T signal, 1:DVB-T2 signal 2:isdbt signal
	uint8_t plp_index;        //Current selected data PLP index.
	uint8_t plp_num;

	uint8_t  plp_id;          //plp_id of plp_index.  
	uint16_t t2_system_id;    //t2_system_id of this channel. 

	uint8_t all_plp_id[255];      //all plp_id. 
	uint8_t t2_profile;	

	LOG_STRING_FUNCTION fn_output_string;
	char *output_buffer;
	  /**
    @brief The current system.
    */
    sony_dtv_system_t system;
	pfn_nim_reset_callback 	    m_pfn_reset_cxd2838; 
	/*Ali data end*/

    /**
    @brief The demodulator crystal frequency.
    */
    sony_demod_xtal_t xtalFreq;

    /**
     @brief SLVT I2C address (8-bit form - 8'bxxxxxxx0).
    */
    uint8_t i2cAddressSLVT;

    /**
     @brief SLVX I2C address (8-bit form - 8'bxxxxxxx0). Fixed to i2cAddressSLVT + 4.
    */
    uint8_t i2cAddressSLVX;

    /**
     @brief I2C API instance.
    */
    sony_i2c_t * pI2c;

    /**
    @brief The driver operating state.
    */
    sony_demod_state_t state;

    /**
     @brief Auto detected chip ID at initialisation.
    */
    sony_demod_chip_id_t chipId;

    /**
     @brief The current bandwidth.
    */
    sony_demod_bandwidth_t bandwidth;

    /**
     @brief IF frequency configuration. Configure prior to initialization.
            Use the ::SONY_DEMOD_MAKE_IFFREQ_CONFIG macro for configuration.
    */
    sony_demod_iffreq_config_t iffreqConfig;

    /**
     @brief Stores the tuner model for demodulator specific optimisations.
    */
    sony_demod_tuner_optimize_t tunerOptimize;

    /**
     @brief The sense configured on the demodulator with
            ::sony_demod_SetConfig.
    */
    sony_demod_spectrum_sense_t confSense;

    /**
     @brief The serial TS clock mode for all active states.  This is configured using
            ::sony_demod_SetConfig with the SONY_DEMOD_CONFIG_TSCLK_CONT option.
    */
    uint8_t serialTsClockModeContinuous;

    /**
     @brief The serial TS clock frequency option for active states. 
            This is configured using ::sony_demod_SetConfig with the 
            SONY_DEMOD_CONFIG_TS_SERIAL_CLK_FREQ option.
    */
    sony_demod_serial_ts_clk_t serialTsClkFreq;

    /**
     @brief A table of the demodulator configuration changes stored from the 
            ::sony_demod_SetConfig and ::sony_demod_SetAndSaveRegisterBits functions.
    */
    sony_demod_config_memory_t configMemory[SONY_DEMOD_MAX_CONFIG_MEMORY_COUNT];

    /**
     @brief The index of the last valid entry in the configMemory table
    */
    uint8_t configMemoryLastEntry;

    /**
     @brief User defined data.
    */
    void * user;
} sony_demod_t;



/**
 @brief The preset information for a ISDB-T signal
*/
typedef struct sony_demod_isdbt_preset_info_t {
    uint8_t data[13];                   /**< TMCC information defined by ISDB-T */
} sony_demod_isdbt_preset_info_t;

/**
 @brief The tune parameters for a ISDB-T signal
*/
typedef struct sony_isdbt_tune_param_t {
    uint32_t centerFreqKHz;             /**< Center frequency(kHz) of the ISDB-T channel */
    sony_demod_bandwidth_t bandwidth;   /**< Bandwidth of the ISDB-T channel */
} sony_isdbt_tune_param_t;

typedef struct sony_demod_ts_clk_configuration_t {
    uint8_t serialClkMode;      /**< Serial clock mode (gated or continuous) */
    uint8_t serialDutyMode;     /**< Serial clock duty mode (full rate or half rate) */
    uint8_t tsClkPeriod;        /**< TS clock period */
    uint8_t clkSelTsIf;         /**< TS clock frequency (low, mid or high) */
} sony_demod_ts_clk_configuration_t;

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/

/**
  @brief Set up the demodulator.
         This function Must be called before calling sony_demod_Initialize.
  @param pDemod Reference to memory allocated for the demodulator instance.
  @param xtalFreq The frequency of the demod crystal.
  @param i2cAddress The demod I2C address in 8-bit form.
  @param pDemodI2c The I2C driver that the demod will use as the I2C interface.

  @return SONY_RESULT_OK if successful.
 */
sony_result_t cxd2838_demod_Create(sony_demod_t*     pDemod,
                                sony_demod_xtal_t xtalFreq,
                                uint8_t           i2cAddress,
                                sony_i2c_t*       pDemodI2c);

/**
 @brief Initialize the demodulator.
        Can also be used to reset the demodulator from any state back to 
        ::SONY_DEMOD_STATE_SLEEP.  Please note this will reset all demodulator registers
        clearing any configuration settings.

        This API also clears the demodulator configuration memory table.

 @param pDemod The demodulator instance.

 @return SONY_RESULT_OK if successful.
  */
sony_result_t sony_demod_Initialize(sony_demod_t* pDemod);

/**
 @brief Put the demodulator into Sleep state.  From
        this state the demodulator can be directly tuned.

        If currently in ::SONY_DEMOD_STATE_SHUTDOWN the configuration memory will be loaded
        back into the demodulator.

 @param pDemod The demodulator instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_Sleep(sony_demod_t* pDemod);

/**
 @brief Shutdown the demodulator.

        The device is placed in "Shutdown" state.
        sony_demod_Sleep must be called to re-initialise the
        device and driver for future acquisitions.

 @param pDemod The demodulator instance.

 @return SONY_RESULT_OK if successful.
 */
 sony_result_t cxd2838_demod_Shutdown(sony_demod_t* pDemod);

/**
 @brief Soft reset the demodulator.
        The soft reset will begin the devices acquisition process.

 @param pDemod The demod instance.

 @return SONY_RESULT_OK if successfully reset.
*/
 sony_result_t cxd2838_demod_SoftReset(sony_demod_t * pDemod);

/**
 @brief Completes the demodulator acquisition setup.
        Must be called after system specific demod and RF tunes.

 @param pDemod The demodulator instance.

 @return SONY_RESULT_OK if successful.
*/
 sony_result_t cxd2838_demod_TuneEnd(sony_demod_t * pDemod);

/**
 @brief Set configuration options on the demodulator.

 @param pDemod The demodulator instance.
 @param config The configuration ID to set. See ::sony_demod_config_id_t.
 @param value The associated value. Depends on the configId.

 @return SONY_RESULT_OK if successfully set the configuration option.
*/
 sony_result_t cxd2838_demod_SetConfig(sony_demod_t*       pDemod,
                                   sony_demod_config_t config,
                                   uint32_t            value);


/**
 @brief Setup the GPIO.

 @param pDemod The demodulator instance.
 @param id GPIO number (0 or 1 or 2 ).
 @param enable Set enable (1) or disable (0).
 @param mode GPIO pin mode

 @return SONY_RESULT_OK if successful.
*/
 sony_result_t cxd2838_demod_GPIOSetConfig(sony_demod_t*          pDemod,
                                       uint8_t                id, 
                                       uint8_t                enable,
                                       sony_demod_gpio_mode_t mode);

/**
 @brief Read the GPIO value.
        The GPIO should have been configured as an input (Read) GPIO.

 @param pDemod The demodulator instance.
 @param id GPIO number (0 or 1 or 2 ).
 @param pValue The current value of the GPIO.

 @return SONY_RESULT_OK if successful.
*/
 sony_result_t cxd2838_demod_GPIORead(sony_demod_t* pDemod,
                                  uint8_t       id,
                                  uint8_t*      pValue);

/**
 @brief Write the GPIO value.
        The GPIO should have been configured as an output (Write) GPIO.

 @param pDemod The demodulator instance.
 @param id GPIO number (0 or 1 or 2 ).
 @param value The value to set as output.

 @return SONY_RESULT_OK if successful.
*/
 sony_result_t cxd2838_demod_GPIOWrite(sony_demod_t* pDemod,
                                   uint8_t       id,
                                   uint8_t       value);

/**
 @brief Get the Chip ID of the connected demodulator.
 
 @param pDemod The demodulator instance.
 @param pChipId Pointer to receive the IP ID into.

 @return SONY_RESULT_OK if pChipId is valid.
*/
 sony_result_t cxd2838_demod_ChipID(sony_demod_t*         pDemod,
                                sony_demod_chip_id_t* pChipId);

/**
 @brief Configure the demodulator to forward I2C messages to the
        output port for tuner control.

 @param pDemod The demodulator instance.
 @param enable Enable(enable != 0) / Disable I2C repeater(enable == 0)

 @return SONY_RESULT_OK if successful.
*/
 sony_result_t cxd2838_demod_I2cRepeaterEnable(sony_demod_t* pDemod,
                                           uint8_t       enable);


/**
 @brief Set a specific value with bit mask to any demod register.  
        NOTE : This API should only be used under instruction from Sony 
        support. Manually modifying any demodulator register could have a negative 
        effect for performance or basic functionality.
         
 @param pDemod The demodulator instance.
 @param slaveAddress Slave address of configuration setting
 @param bank Demodulator bank of configuration setting
 @param registerAddress Register address of configuration setting
 @param value The value being written to this register
 @param bitMask The bit mask used on the register
 
 @return SONY_RESULT_OK if successful.
*/
 sony_result_t cxd2838_demod_SetAndSaveRegisterBits (sony_demod_t * pDemod, 
                                                 uint8_t slaveAddress, 
                                                 uint8_t bank, 
                                                 uint8_t registerAddress,
                                                 uint8_t value,
                                                 uint8_t bitMask);

/**
 @brief Set the TS clock mode and frequency based on the demod struct
        members.  Called internally as part of each Sleep to Active
        state transition.

 @param pDemod The demodulator instance
 @param system The tuning system

 @return SONY_RESULT_OK if successful.
*/
 sony_result_t cxd2838_demod_SetTsClockModeAndFreq (sony_demod_t * pDemod);

/*------------------------------------------------------------------------------
  Functions for ISDB-T
------------------------------------------------------------------------------*/
/**
 @brief Set Preset Infomation

 @param pDemod      The demodulator instance.
 @param pPresetInfo Preset information

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbt_SetPreset(sony_demod_t*                   pDemod,
                                         sony_demod_isdbt_preset_info_t* pPresetInfo);

/**
 @brief Enable acquisition on the demodulator for ISDB-T channels.  Called from
        the integration layer ::sony_integ_isdbt_Tune API.

 @param pDemod     The demodulator instance
 @param pTuneParam Tune parameters structure.

 @return SONY_RESULT_OK if successful.
 */
sony_result_t sony_demod_isdbt_Tune(sony_demod_t*            pDemod,
                                    sony_isdbt_tune_param_t* pTuneParam);
/**
 @brief Enable acquisition on the demodulator for ISDB-T channels in EWS mode.
        Called from the integration layer ::sony_integ_isdbt_EWSTune API.

 @param pDemod     The demodulator instance
 @param pTuneParam Tune parameters structure.
  
 @return SONY_RESULT_OK if successful.
 */
sony_result_t sony_demod_isdbt_EWSTune(sony_demod_t*            pDemod,
                                       sony_isdbt_tune_param_t* pTuneParam);


/**
@brief Check ISDB-T demodulator lock status.

 @param pDemod  The demodulator instance
 @param pLock   Demod lock state

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbt_CheckDemodLock(sony_demod_t*             pDemod,
                                              sony_demod_lock_result_t* pLock);

/**
 @brief Check TS lock status.

 @param pDemod  The demodulator instance
 @param pLock   TS lock state

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbt_CheckTSLock(sony_demod_t*             pDemod,
                                           sony_demod_lock_result_t* pLock);


/**
 @brief Disable TSDATA pin Hi-Z depend on TS output setting.
        Called internally as part of each Sleep to Active
        state transition.

 @param pDemod The demodulator instance

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_DisableTsDataPinHiZ(sony_demod_t * pDemod);

#endif /* SONY_DEMOD_ISDBT_H */
