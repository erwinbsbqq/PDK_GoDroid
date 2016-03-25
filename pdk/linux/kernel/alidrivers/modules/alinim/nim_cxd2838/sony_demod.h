/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2013-01-29 11:53:52 #$
  File Revision : $Revision:: 6576 $
------------------------------------------------------------------------------*/
/**
 @file    sony_demod.h

          This file provides the common demodulator control interface.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_H
#define SONY_DEMOD_H

#include "sony_i2c.h"
#include "sony_demod_isdbt.h"

/* IF Settings, update to match your tuner output */
#define SONY_DVBT_5MHz_IF           3.60
#define SONY_DVBT_6MHz_IF           3.60
#define SONY_DVBT_7MHz_IF           4.20
#define SONY_DVBT_8MHz_IF           4.80
#define SONY_DVBT2_1_7MHz_IF        3.50
#define SONY_DVBT2_5MHz_IF          3.60
#define SONY_DVBT2_6MHz_IF          3.60
#define SONY_DVBT2_7MHz_IF          4.20
#define SONY_DVBT2_8MHz_IF          4.80
#define SONY_DVBC_6MHz_IF           3.70 
#define SONY_DVBC_7MHz_IF           4.90 
#define SONY_DVBC_8MHz_IF           4.90 
#define SONY_DVBC2_6MHz_IF          3.70
#define SONY_DVBC2_8MHz_IF          4.90
#define SONY_ISDBT_6MHz_IF          3.55
#define SONY_ISDBT_7MHz_IF          4.15
#define SONY_ISDBT_8MHz_IF          4.75

/*------------------------------------------------------------------------------
  Defines
------------------------------------------------------------------------------*/
#define SONY_ISDBT_WAIT_DEMOD_LOCK          1500    /**< 1.5s timeout for wait demodulator lock process for ISDB-T channels */
#define SONY_ISDBT_WAIT_TS_LOCK             1500    /**< 1.5s timeout for wait TS lock process for ISDB-T channels */
#define SONY_ISDBT_WAIT_LOCK_INTERVAL       10      /**< 10ms polling interval for demodulator and TS lock functions */



#define SONY_DRIVER_BUILD_OPTION_CXD2838
//#define SONY_DRIVER_BUILD_OPTION_CXD2837
//#define SAME_TUNER_CONFIG_DVBT_T2//dennis.dai
#define SONY_DISABLE_I2C_REPEATER
//#define TUNER_IFAGCPOS
//#define TUNER_SPECTRUM_INV 
//#define RFAIN_ADC_ENABLE 
//#define TUNER_RFLVLMON_DISABLE 

/* Tuner Configuration */
/*#define TUNER_SONY_ASCOT3R*/      /**< Define for Sony Ascot3R tuner. */
#define TUNER_SONY_ASCOT3           /**< Define for Sony Ascot3 tuner. */
/*#define TUNER_SONY_ASCOT2E*/      /**< Define for Sony Ascot2E tuner. */
/*#define TUNER_SONY_ASCOT2D*/      /**< Define for Sony Ascot2D tuner. */
/*#define TUNER_IFAGCPOS*/          /**< Define for IFAGC sense positive. */
/*#define TUNER_SPECTRUM_INV*/      /**< Define for spectrum inversion. */
/*#define TUNER_SLEEP_DISABLEXTAL*/ /**< Define for disable Xtal in Sleep state. */
//#define TUNER_RFLVLMON_DISABLE      /**< Define to disable RF level monitoring. */
#define DEMOD_XTAL_205000      		/**< Define for 20.5MHz demodulator crystal frequency. */
/*#define DEMOD_XTAL_240000*/       /**< Define for 24MHz demodulator crystal frequency. */
/*#define DEMOD_XTAL_410000*/       /**< Define for 41MHz demodulator crystal frequency. */



/*------------------------------------------------------------------------------
  Recommended timeouts for wait for "lock"
------------------------------------------------------------------------------*/
#define DTV_DEMOD_DVB_UNLOCK_WAIT				100     /**< 100ms wait before reading unlock flags read. */

#define DTV_DEMOD_DVBT_WAIT_LOCK				1000    /**< DVB-T 1000ms for TPS Lock */
#define DTV_DEMOD_DVBT_WAIT_TS_LOCK			1000    /**< DVB-T 1000ms for TS (from TPS Lock) */

#define DTV_DEMOD_DVBC_WAIT_LOCK				1000    /**< DVB-C 1000ms for general wait period. */
#define DTV_DEMOD_DVBC_WAIT_TS_LOCK			1000    /**< DVB-C 1000ms for TS Lock */

#define DTV_DEMOD_DVBT2_P1_WAIT							300     /**< DVB-T2 300ms before checking early no P1 (T2 or FEF) indication. */
#define DTV_DEMOD_DVBT2_T2_P1_WAIT					600     /**< DVB-T2 600ms before checking for P1 (T2 only) indication. */
#define DTV_DEMOD_DVBT2_WAIT_LOCK 					3500    /**< DVB-T2 3500ms for demod lock indication. */
#define DTV_DEMOD_DVBT2_WAIT_TS_LOCK				1500    /**< DVB-T2 1500ms for TS lock. Assumes that demod core is locked. */
#define DTV_DEMOD_TUNE_T2_L1POST_TIMEOUT		300 //600     /**< DVB-T2 300ms timeout for L1Post Valid loop */
#define DTV_DEMOD_WAIT_LOCK_INTERVAL				10		/**< 10ms interval for demodulator lock polls */
#define DTV_DEMOD_DVBT2_LITE_WAIT_LOCK     5000    /**< 5.0s timeout for wait demodulator lock process for DVB-T2-Lite channels */

//#define DEMOD_MAX_TS_CLK_KHZ   10250   /**< Maximum allowed manual TS clock rate setting. */
#define LOG_STRING_BUFFER_SIZE    255


#if 1
#define CXD2838_LOG(param, fmt, args...) \
    do{\
            printk(fmt, ##args);\
    }while(0)
#else
#define CXD2837_LOG(param, fmt, args...) \
    do{\
        if ( param->output_buffer && param->fn_output_string )\
        {\
            sprintf(param->output_buffer, fmt, ##args);\
            param->fn_output_string(param->output_buffer);\
        };\
    }while(0)
#endif

#if 0
#define CXD2838_LOG_I2C		cxd2838_log_i2c
#else
#define CXD2838_LOG_I2C(...)	 	do{}while(0)
#endif

/*------------------------------------------------------------------------------
  Device Defines based on pre-compiler BUILD_OPTION
------------------------------------------------------------------------------*/

#if defined SONY_DRIVER_BUILD_OPTION_CXD2837   /* DVB-T/T2/C Demodulator */
#define SONY_DEMOD_SUPPORT_TERR_OR_CABLE       /**< Driver supports a Terrestrial or Cable system. */
#elif defined SONY_DRIVER_BUILD_OPTION_CXD2839 /* DVB-S/S2 Demodulator */
#define SONY_DEMOD_SUPPORT_DVBS_S2             /**< Driver supports DVBS and S2. */
#elif defined SONY_DRIVER_BUILD_OPTION_CXD2841 /* DVB-T/T2/C/C2/S/S2 Demodulator */
#define SONY_DEMOD_SUPPORT_DVBC2               /**< Driver supports DVBC2. */
#define SONY_DEMOD_SUPPORT_DVBS_S2             /**< Driver supports DVBS and S2. */
#define SONY_DEMOD_SUPPORT_TERR_OR_CABLE       /**< Driver supports a Terrestrial or Cable system. */
#elif defined SONY_DRIVER_BUILD_OPTION_CXD2842 /* DVB-T/T2/C/S/S2 Demodulator */
#define SONY_DEMOD_SUPPORT_DVBS_S2             /**< Driver supports DVBS and S2. */
#define SONY_DEMOD_SUPPORT_TERR_OR_CABLE       /**< Driver supports a Terrestrial or Cable system. */
#elif defined SONY_DRIVER_BUILD_OPTION_CXD2843 /* DVB-T/T2/C/C2 Demodulator */
#define SONY_DEMOD_SUPPORT_DVBC2               /**< Driver supports DVBC2. */
#define SONY_DEMOD_SUPPORT_TERR_OR_CABLE       /**< Driver supports a Terrestrial or Cable system. */
#elif defined SONY_DRIVER_BUILD_OPTION_CXD2838 /* DVB-ISDBT Demodulator */
#define SONY_DEMOD_SUPPORT_ISDBT               /**< Driver supports ISDBT. */
#define SONY_DEMOD_SUPPORT_TERR_OR_CABLE       /**< Driver supports a Terrestrial or Cable system. */

#else
#error SONY_DRIVER_BUILD_OPTION value not recognised
#endif


/*------------------------------------------------------------------------------
  Includes
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_i2c.h"
#include "sony_dtv.h"

//#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
//#include "sony_dvbt.h"
//#include "sony_dvbt2.h"
//#include "sony_dvbc.h"
//#endif

#ifdef SONY_DEMOD_SUPPORT_DVBC2
#include "sony_dvbc2.h"
#endif

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
#include "sony_dvbs.h"
#include "sony_dvbs2.h"
#endif

/*------------------------------------------------------------------------------
  Defines
------------------------------------------------------------------------------*/
#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
/**
 @brief Calculate the demodulator IF Freq setting ::sony_demod_t::iffreqConfig.
        ((IFFREQ/Sampling Freq at Down Converter DSP module) * Down converter's dynamic range + 0.5                
*/
//#define SONY_DEMOD_MAKE_IFFREQ_CONFIG(iffreq) ((uint32_t)(((iffreq)/41.0)*16777216.0 + 0.5))
//#define SONY_DEMOD_MAKE_IFFREQ_CONFIG_ISDBT(xTal, iffreq) ((xTal == SONY_DEMOD_XTAL_24000KHz) ? ((uint32_t)(((iffreq)/48.0)*16777216.0 + 0.5)) : ((uint32_t)(((iffreq)/41.0)*16777216.0 + 0.5)))
//#define SONY_DEMOD_MAKE_IFFREQ_CONFIG(xTal, iffreq) ((xTal == SONY_DEMOD_XTAL_24000KHz) ? ((uint32_t)(((iffreq)/48.0)*16777216.0 + 0.5)) : ((uint32_t)(((iffreq)/41.0)*16777216.0 + 0.5)))

#endif



#define SONY_DEMOD_MAX_TS_CLK_KHZ   10250   /**< Maximum allowed manual TS clock rate setting in parallel mode. Do not change. */

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

typedef enum {
    SONY_DEMOD_TERR_CABLE_SPECTRUM_NORMAL = 0,             /**< Spectrum normal sense. */
    SONY_DEMOD_TERR_CABLE_SPECTRUM_INV                     /**< Spectrum inverted. */
} sony_demod_terr_cable_spectrum_sense_t;


#if 0
/**
 @brief Demodulator crystal frequency.
*/

typedef enum {
    SONY_DEMOD_XTAL_20500KHz,           /**< 20.5 MHz */
	SONY_DEMOD_XTAL_24000KHz,			/**< 24 MHz */
    SONY_DEMOD_XTAL_41000KHz            /**< 41 MHz */
} sony_demod_xtal_t;

/**
 @brief The demodulator Chip ID mapping.
*/
typedef enum {
    SONY_DEMOD_CHIP_ID_CXD2837 = 0xB1,  /**< CXD2837  DVB-T/T2/C */
    SONY_DEMOD_CHIP_ID_CXD2839 = 0xA5,  /**< CXD2839  DVB-S/S2 */
    SONY_DEMOD_CHIP_ID_CXD2841 = 0xA7,  /**< CXD2841  DVB-T/T2/C/C2/S/S2 */
    SONY_DEMOD_CHIP_ID_CXD2842 = 0xA5,  /**< CXD2842  DVB-T/T2/C/S/S2 */
    SONY_DEMOD_CHIP_ID_CXD2843 = 0xA4,  /**< CXD2843  DVB-T/T2/C/C2 */
    SONY_DEMOD_CHIP_ID_CXD2838 = 0xB0,  /**< CXD2838 ISDB-T */
    SONY_DEMOD_CHIP_ID_CXD2828 = 0x22   /**< CXD2828 ISDB-T */
} sony_demod_chip_id_t;

/**
 @brief Demodulator software state.
*/
typedef enum {
    SONY_DEMOD_STATE_UNKNOWN,           /**< Unknown. */
    SONY_DEMOD_STATE_SHUTDOWN,          /**< Chip is in Shutdown state */
    SONY_DEMOD_STATE_SLEEP_T_C,         /**< Chip is in Sleep state for DVB-T / T2 / C / C2 */
    SONY_DEMOD_STATE_SLEEP_S,           /**< Chip is in Sleep state for DVB-S / S2 */
    SONY_DEMOD_STATE_SLEEP,         /**< Chip is in Sleep state */
    SONY_DEMOD_STATE_ACTIVE,
    SONY_DEMOD_STATE_ACTIVE_T_C,        /**< Chip is in Active state for DVB-T / T2 / C / C2 */
    SONY_DEMOD_STATE_ACTIVE_S,          /**< Chip is in Active state for DVB-S / S2 */
    SONY_DEMOD_STATE_EWS,
    SONY_DEMOD_STATE_INVALID            /**< Invalid, result of an error during a state change. */
} sony_demod_state_t;

#if 0 //isdbt state
typedef enum {
    SONY_DEMOD_STATE_UNKNOWN = 0,   /**< Unknown. */
    SONY_DEMOD_STATE_SHUTDOWN,      /**< Chip is in Shutdown state */
    SONY_DEMOD_STATE_SLEEP,         /**< Chip is in Sleep state */
    SONY_DEMOD_STATE_ACTIVE,        /**< Chip is in Active state */
    SONY_DEMOD_STATE_EWS,           /**< Chip is in EWS state */
    SONY_DEMOD_STATE_INVALID        /**< Invalid, result of an error during a state change. */
} sony_demod_state_t;
#endif


#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
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
 @brief The tune parameters for a ISDB-T signal
*/
typedef struct sony_isdbt_tune_param_t {
    uint32_t centerFreqKHz;             /**< Center frequency(kHz) of the ISDB-T channel */
    sony_demod_bandwidth_t bandwidth;   /**< Bandwidth of the ISDB-T channel */
} sony_isdbt_tune_param_t;


/**
 @brief Enumeration of supported sony tuner models used for optimising the 
        demodulator configuration.
*/
typedef enum {
    SONY_DEMOD_TUNER_OPTIMIZE_UNKNOWN,  /**< Non-Sony Tuner. */
    SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2D,  /**< Sony ASCOT2D derived tuners. */
    SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2E,  /**< Sony ASCOT2E derived tuners. */
    SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2XR, /**< Sony ASCOT2XR derived tuners. */
    SONY_DEMOD_TUNER_OPTIMIZE_ASCOT3    /**< Sony ASCOT3 derived tuners. */
} sony_demod_tuner_optimize_t;

/**
 @brief Enumeration of spectrum inversion monitor values.
*/
typedef enum {
    SONY_DEMOD_TERR_CABLE_SPECTRUM_NORMAL = 0,             /**< Spectrum normal sense. */
    SONY_DEMOD_TERR_CABLE_SPECTRUM_INV                     /**< Spectrum inverted. */
} sony_demod_terr_cable_spectrum_sense_t;
#endif

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
/**
 @brief Enumeration of I/Q inversion monitor values.
*/
typedef enum {
    SONY_DEMOD_SAT_IQ_SENSE_NORMAL = 0,   /**< I/Q normal sense. */
    SONY_DEMOD_SAT_IQ_SENSE_INV           /**< I/Q inverted. */
} sony_demod_sat_iq_sense_t;
#endif /* SONY_DEMOD_SUPPORT_DVBS_S2 */

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
     @brief Serial TS clock gated on valid TS data or is continuous.
            Value is stored in demodulator structure to be applied during Sleep to Active
            transition.  

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
            - 4 : Disable during TS payload
            - 8 : Disable during TS header
            - 16: Disable during TS sync
    */
    SONY_DEMOD_CONFIG_TSERR_MASK,

    /**
     @brief Configure the driving current for the TS Clk pin.

            - 0 : 8mA (Default)
            - 1 : 10mA
    */
    SONY_DEMOD_CONFIG_TSCLK_CURRENT_10mA,

    /**
     @brief Configure the driving current for the TS Sync / TS Valid 
            / TS Data / TS Error pins.

            - 0 : 8mA (Default)
            - 1 : 10mA
    */
    SONY_DEMOD_CONFIG_TS_CURRENT_10mA,

    /**
     @brief This configuration can be used to configure the demodulator to output a TS waveform that is
            backwards compatible with previous generation demodulators (CXD2834 / CXD2835 / CXD2836). 
            This option should not be used unless specifically required to overcome a HW configuration issue.

            The demodulator will have the following settings, which will override any prior individual 
            configuration:
            - Disable TS packet gap insertion.
            - Parallel TS maximum bit rate of 82MBps
            - Serial TS clock frequency fixed at 82MHz
            
            Values:
            - 0 : Backwards compatible mode disabled
            - 1 : Backwards compatible mode enabled
    */
    SONY_DEMOD_CONFIG_TS_BACKWARDS_COMPATIBLE,

    /**
    @brief Writes a 12-bit value to the PWM output.
        Please note the actual PWM precision.
        - 12-bit.
        0xFFF => DVDD
        0x000 => GND
    */
    SONY_DEMOD_CONFIG_PWM_VALUE,

#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
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
     @brief Configure the clock frequency for Serial TS in terrestrial and cable active states.
            Value is stored in demodulator structure to be applied during Sleep to Active
            transition.  
            Only valid when SONY_DEMOD_CONFIG_PARALLEL_SEL = 0 (serial TS).

            - 0 : Invalid
            - 1 : 82.00MHz (Default)
            - 2 : 65.60MHz
            - 3 : 54.67MHz
            - 4 : 41.00MHz 
            - 5 : 32.80MHz
    */
    SONY_DEMOD_CONFIG_TERR_CABLE_TS_SERIAL_CLK_FREQ,

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
     @brief Terrestrial / Cable tuner IF spectrum sense configuration. 
            
            Value: 
            - 0: IF spectrum sense is not same as RF. Used for Normal / Ordinary tuners i.e. ASCOT. (Default)
            - 1: IF spectrum sense is same as RF.
    */
    SONY_DEMOD_CONFIG_SPECTRUM_INV,

    /**
     @brief RFAIN ADC enable/disable.
            Must be asleep for this setting to take effect on next tune/scan.
            Use ::sony_demod_terr_cable_monitor_RFAIN to monitor the input.

            Value:
            - 0: Disable RFAIN ADC and monitor (Default).
            - 1: Enable RFAIN ADC and monitor.
    */
    SONY_DEMOD_CONFIG_RFAIN_ENABLE,

    /**
     @brief Configure the order in which systems are attempted in Blind Tune and 
            Scan.  This can be used to optimize scan duration where specifc
            details on system split ratio are known about the spectrum.

            Value:
            - 0: DVB-T followed by DVBT2 (default).
            - 1: DVB-T2 followed by DVBT.
    */
    SONY_DEMOD_CONFIG_TERR_BLINDTUNE_DVBT2_FIRST,

    /**
     @brief Set the measurment period for Pre-RS BER (DVB-T).

            This is a 5 bit value with a default of 11.
    */
    SONY_DEMOD_CONFIG_DVBT_BERN_PERIOD,

    /**
     @brief Set the measurment period for Pre-RS BER (DVB-C).

            This is a 5 bit value with a default of 11.
    */
    SONY_DEMOD_CONFIG_DVBC_BERN_PERIOD,

    /**
     @brief Set the measurment period for Pre-Viterbi BER (DVB-T).

            This is a 3 bit value with a default of 1.
    */
    SONY_DEMOD_CONFIG_DVBT_VBER_PERIOD,

    /**
     @brief Set the measurment period for Pre-BCH BER (DVB-T2/C2) and 
            Post-BCH FER (DVB-T2/C2).

            This is a 4 bit value with a default of 8.
    */
    SONY_DEMOD_CONFIG_DVBT2C2_BBER_MES,

    /**
     @brief Set the measurment period for Pre-LDPC BER (DVB-T2/C2).

            This is a 4 bit value with a default of 8.
    */
    SONY_DEMOD_CONFIG_DVBT2C2_LBER_MES,

    /**
     @brief Set the measurment period for PER (DVB-T).

            This is a 4 bit value with a default of 10.
    */
    SONY_DEMOD_CONFIG_DVBT_PER_MES,

    /**
     @brief Set the measurment period for PER (DVB-C).

            This is a 4 bit value with a default of 10.
    */
    SONY_DEMOD_CONFIG_DVBC_PER_MES,

    /**
     @brief Set the measurment period for PER (DVB-T2/C2).

            This is a 4 bit value with a default of 10.
    */
    SONY_DEMOD_CONFIG_DVBT2C2_PER_MES,
#endif

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
    /**
     @brief Configure the clock frequency for Serial TS in Satellite active states.
            Value is stored in demodulator structure to be applied during Sleep to Active
            transition.  
            Only valid when SONY_DEMOD_CONFIG_PARALLEL_SEL = 0 (serial TS).

            - 0 : 129.83MHz
            - 1 : 97.38MHz (Default)
            - 2 : 77.90MHz
            - 3 : 64.92MHz
            - 4 : 48.69MHz
            - 5 : 38.95MHz
    */
    SONY_DEMOD_CONFIG_SAT_TS_SERIAL_CLK_FREQ,

    /**
     @brief I/Q connection sense inversion between tuner and demod.

            - 0: Normal (Default).
            - 1: Inverted. (I/Q signal input to Q/I pin of demodulator)
    */
    SONY_DEMOD_CONFIG_SAT_TUNER_IQ_SENSE_INV,

    /**
     @brief IFAGC sense configuration for satellite.

            Value:
            - 0: Positive IFAGC (Default)
            - 1: Negative IFAGC.
    */
    SONY_DEMOD_CONFIG_SAT_IFAGCNEG,

    /**
     @brief Use TSDATA6 pin for DiSEqC RXEN and TXEN.
            Only valid when SONY_DEMOD_CONFIG_PARALLEL_SEL = 0 (serial TS).

            Value:
            - 0: Not used as DiSEqC. (Default)
            - 1: TX_EN
            - 2: RX_EN
    */
    SONY_DEMOD_CONFIG_SAT_TSDATA6_DISEQC,

    /**
     @brief Measurement period for Pre-RS BER(DVB-S), PER(DVB-S) and PER(DVB-S2).

            - The period is 2^(value) frames.
            - Valid range is 0 <= value <= 15.
    */
    SONY_DEMOD_CONFIG_SAT_MEAS_PERIOD1,

    /**
     @brief Measurement period for Pre-BCH BER(DVB-S2) and Post-BCH FER(DVB-S2).

            - The period is 2^(value) frames.
            - Valid range is 0 <= value <= 15.
    */
    SONY_DEMOD_CONFIG_SAT_MEAS_PERIOD2,

    /**
     @brief Measurement period for Pre-Viterbi BER(DVB-S).

            - The period is 2^(value) frames.
            - Valid range is 0 <= value <= 15.
    */
    SONY_DEMOD_CONFIG_SAT_MEAS_PERIOD3,

    /**
     @brief Measurement period for Pre-LDPC BER(DVB-S2).

            - The period is 2^(value) frames.
            - Valid range is 0 <= value <= 15.
    */
    SONY_DEMOD_CONFIG_SAT_MEAS_PERIOD4,

#endif /* SONY_DEMOD_SUPPORT_DVBS_S2 */
} sony_demod_config_id_t;

/**
 @brief Demodulator lock status.
*/
typedef enum {
    SONY_DEMOD_LOCK_RESULT_NOTDETECT, /**< Neither "Lock" or "Unlock" conditions are met, lock status cannot be determined */
    SONY_DEMOD_LOCK_RESULT_LOCKED,    /**< "Lock" condition is found. */
    SONY_DEMOD_LOCK_RESULT_UNLOCKED   /**< No signal was found or the signal was not the required system. */
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

    /** @brief GPIO pin is configured to output the FEF timing indicator (DVB-T2 Only) */
    SONY_DEMOD_GPIO_MODE_FEF_PART = 0x05, 

    /** @brief GPIO pin is configured to output DiSEqC Transmit Enable */
    SONY_DEMOD_GPIO_MODE_DISEQC_TX_EN = 0x08, 

    /** @brief GPIO pin is configured to output DiSEqC Receive Enable */
    SONY_DEMOD_GPIO_MODE_DISEQC_RX_EN = 0x09
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
} sony_demod_serial_ts_clk_t ;

/*------------------------------------------------------------------------------
  Structs
------------------------------------------------------------------------------*/
#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
/**
 @brief List of register values for IF frequency configuration.  Used for handling 
        tuners that output a different IF depending on the expected channel BW.
        Should be set using ::SONY_DEMOD_MAKE_IFFREQ_CONFIG macro.
*/
typedef struct {
    uint32_t configDVBT_5;              /**< DVB-T 5MHz */
    uint32_t configDVBT_6;              /**< DVB-T 6MHz */
    uint32_t configDVBT_7;              /**< DVB-T 7MHz */
    uint32_t configDVBT_8;              /**< DVB-T 8MHz */
    uint32_t configDVBT2_1_7;           /**< DVB-T2 1.7MHz */
    uint32_t configDVBT2_5;             /**< DVB-T2 5MHz */
    uint32_t configDVBT2_6;             /**< DVB-T2 6MHz */
    uint32_t configDVBT2_7;             /**< DVB-T2 7MHz */
    uint32_t configDVBT2_8;             /**< DVB-T2 8MHz */
    uint32_t configDVBC2_6;             /**< DVB-C2 6MHz */
    uint32_t configDVBC2_8;             /**< DVB-C2 8MHz */
    uint32_t configDVBC_6;              /**< DVB-C  6MHZ */
    uint32_t configDVBC_7;              /**< DVB-C  7MHZ */
    uint32_t configDVBC_8;              /**< DVB-C  8MHZ */
	uint32_t configIsdbt6;   			/**< ISDB-T 6MHz */
    uint32_t configIsdbt7;  			/**< ISDB-T 7MHz */
    uint32_t configIsdbt8;   			/**< ISDB-T 8MHz */
} sony_demod_iffreq_config_t;
#endif
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



typedef void (*LOG_STRING_FUNCTION)(char *string);

/**
 @brief The demodulator definition which allows control of the demodulator device 
        through the defined set of functions. This portion of the driver is seperate 
        from the tuner portion and so can be operated independently of the tuner.
*/

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
	/*Ali data end*/

	
    //Sony data
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
    @brief The current system.
    */
    sony_dtv_system_t system;

    /**
     @brief Auto detected chip ID at initialisation.
    */
    sony_demod_chip_id_t chipId;

#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
    /**
     @brief The current bandwidth, terrestrial and cable systems only.
    */
    sony_demod_bandwidth_t bandwidth;

    /**
     @brief IF frequency configuration for terrestrial and cable systems. Configure 
            prior to initialisation. Use the ::SONY_DEMOD_MAKE_IFFREQ_CONFIG macro for 
            configuration.
    */
    sony_demod_iffreq_config_t iffreqConfig;
    /**
     @brief Stores the terrestrial / cable tuner model for demodulator specific optimisations.
    */
    sony_demod_tuner_optimize_t tunerOptimize;

    /**
     @brief RFAIN ADC enable/disable. Must be configured prior to initialisation.
            Only change this indicator during the ::SONY_DEMOD_STATE_SLEEP_T_C state. 
            Use ::sony_demod_SetConfig to configure this flag.
    */
    uint8_t enableRfain;

    /**
     @brief Scan mode enable/disable. Only change this indicator during the
            SONY_DEMOD_STATE_SLEEP_T_C.
    */
    uint8_t scanMode;

    /**
     @brief The terrestrial / cable tuner IF spectrum sense configured on 
            the demodulator with ::sony_demod_SetConfig.
    */
    sony_demod_terr_cable_spectrum_sense_t confSense;

    /**
     @brief The serial TS clock frequency option for terrestrial and cable active states. 
            This is configured using ::sony_demod_SetConfig with the 
            SONY_DEMOD_CONFIG_TERR_CABLE_TS_SERIAL_CLK_FREQ option.
    */
    sony_demod_serial_ts_clk_t serialTsClkFreqTerrCable;

    /**
     @brief The order in which Blind Tune attempts acquisition.  This value can
            be configured using ::sony_demod_SetConfig with the 
            SONY_DEMOD_CONFIG_TERR_BLINDTUNE_DVBT2_FIRST option.
    */
    uint8_t blindTuneDvbt2First;
#endif

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2

    /**
     @brief The scan mode for DVB-S/S2.
    */
    uint8_t dvbss2ScanMode;

    /**
     @brief The I/Q sense configured on the demodulator with ::sony_demod_SetConfig.
    */
    sony_demod_sat_iq_sense_t satTunerIqSense;

    /**
     @brief The flag whether current mode is "Single cable" mode or not.
            with sony_integ_sat_device_ctrl_EnterSinglecable and
            sony_integ_sat_device_ctrl_ExitSinglecable.
    */
    uint8_t isSinglecable;

    /**
     @brief The flag whether the IQ polarity of single cable switch is invert.
            If it is necessary to change this value, please change it directly after initialize.
    */
    uint8_t isSinglecableIqInv;

    /**
     @brief The serial TS clock frequency option for Satellite active states.  This is 
            configured using ::sony_demod_SetConfig with the 
            SONY_DEMOD_CONFIG_SAT_TS_SERIAL_CLK_FREQ option.
    */
    sony_demod_serial_ts_clk_t serialTsClkFreqSat;
#endif /* SONY_DEMOD_SUPPORT_DVBS_S2 */

	/**
	   @brief The serial TS clock frequency option for active states. 
			  This is configured using ::sony_demod_SetConfig with the 
			  SONY_DEMOD_CONFIG_TS_SERIAL_CLK_FREQ option.
	  */
	  sony_demod_serial_ts_clk_t serialTsClkFreq;


    /**
     @brief The serial TS clock mode for all active states.  This is configured using
            ::sony_demod_SetConfig with the SONY_DEMOD_CONFIG_TSCLK_CONT option.
    */
    uint8_t serialTsClockModeContinuous;

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
 @brief Register definition structure for TS clock configurations.    
 */
typedef struct {
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

        This MUST be called before calling ::sony_demod_InitializeS or ::sony_demod_InitializeT_C.

 @param pDemod Reference to memory allocated for the demodulator instance. The create 
        function will setup this demodulator instance.
 @param xtalFreq The frequency of the demod crystal.
 @param i2cAddress The demod I2C address in 8-bit form.
 @param pDemodI2c The I2C driver that the demod will use as the I2C interface.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_Create (sony_demod_t * pDemod,
                                 sony_demod_xtal_t xtalFreq,
                                 uint8_t i2cAddress,
                                 sony_i2c_t * pDemodI2c);

/**
 @brief Initialize the demodulator, into Terrestrial / Cable mode from a power on state.
        For Satellite systems please use ::sony_demod_InitializeS.
        
        Can also be used to reset the demodulator from any state back to 
        ::SONY_DEMOD_STATE_SLEEP_T_C.  Please note this will reset all demodulator registers
        clearing any configuration settings.

        This API also clears the demodulator configuration memory table.

 @param pDemod The demodulator instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_InitializeT_C (sony_demod_t * pDemod);

sony_result_t sony_demod_Initialize_isdbt (sony_demod_t * pDemod);

/**
 @brief Initialize the demodulator, into Satellite mode from a power on state.
        For Terrestrial / Cable systems please use ::sony_demod_InitializeT_C.
        
        Can also be used to reset the demodulator from any state back to 
        ::SONY_DEMOD_STATE_SLEEP_S.  Please note this will reset all demodulator registers
        clearing any configuration settings.

        This API also clears the demodulator configuration memory table.

 @param pDemod The demodulator instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_InitializeS (sony_demod_t * pDemod);

/**
 @brief Put the demodulator into Sleep state for Terrestrial and Cable mode.  From
        this state the demodulator can be directly tuned to any T / T2 / C / C2 signal,
        or have the mode changed to Satellite by calling sony_demod_Sleep_S.

        If currently in ::SONY_DEMOD_STATE_SHUTDOWN the configuration memory will be loaded
        back into the demodulator.

 @param pDemod The demodulator instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_SleepT_C (sony_demod_t * pDemod);

/**
 @brief Put the demodulator into Sleep state for Terrestrial and Cable mode.  From
        this state the demodulator can be directly tuned to any S / S2 signal, or have 
        the mode changed to Terrestrial / Cable by calling sony_demod_Sleep_T_C.

        If currently in ::SONY_DEMOD_STATE_SHUTDOWN the configuration memory will be loaded
        back into the demodulator.

 @param pDemod The demodulator instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_SleepS (sony_demod_t * pDemod);

/**
 @brief Shutdown the demodulator.

        The device is placed in "Shutdown" state.
        ::sony_demod_SleepT_C or ::sony_demod_SleepS must be called to re-initialise the
        device and driver for future acquisitions.

 @param pDemod The demodulator instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t cxd2838_demod_Shutdown (sony_demod_t * pDemod);

/**
 @brief Completes the demodulator acquisition setup.
        Must be called after system specific demod and RF tunes.

 @param pDemod The demodulator instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_TuneEnd (sony_demod_t * pDemod);

/**
 @brief Soft reset the demodulator.
        The soft reset will begin the devices acquisition process.

 @param pDemod The demod instance.

 @return SONY_RESULT_OK if successfully reset.
*/
sony_result_t sony_demod_SoftReset (sony_demod_t * pDemod);

/**
 @brief Set configuration options on the demodulator.

 @param pDemod The demodulator instance.
 @param configId The configuration ID to set. See ::sony_demod_config_id_t.
 @param value The associated value. Depends on the configId.

 @return SONY_RESULT_OK if successfully set the configuration option.
*/
sony_result_t sony_demod_SetConfig (sony_demod_t * pDemod, 
                                    sony_demod_config_id_t configId, 
                                    int32_t value);

/**
 @brief Configure the demodulator to forward I2C messages to the
        output port for tuner control.

 @param pDemod The demodulator instance.
 @param enable Enable / Disable I2C repeater

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_I2cRepeaterEnable (sony_demod_t * pDemod, 
                                            uint8_t enable);

/**
 @brief Setup the GPIO.

 @param pDemod The demodulator instance.
 @param id GPIO number (0 or 1 or 2 ).
 @param enable Set enable (1) or disable (0).
 @param mode GPIO pin mode

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_GPIOSetConfig (sony_demod_t * pDemod, 
                                        uint8_t id, 
                                        uint8_t enable, 
                                        sony_demod_gpio_mode_t mode);

/**
 @brief Read the GPIO value.
        The GPIO should have been configured as an input (Read) GPIO.

 @param pDemod The demodulator instance.
 @param id GPIO number (0 or 1 or 2 ).
 @param pValue The current value of the GPIO.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_GPIORead (sony_demod_t * pDemod, 
                                   uint8_t id, 
                                   uint8_t * pValue);

/**
 @brief Write the GPIO value.
        The GPIO should have been configured as an output (Write) GPIO.

 @param pDemod The demodulator instance.
 @param id GPIO number (0 or 1 or 2 ).
 @param value The value to set as output.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_GPIOWrite (sony_demod_t * pDemod, 
                                    uint8_t id, 
                                    uint8_t value);

/**
 @brief Get the Chip ID of the connected demodulator.
 
 @param pDemod The demodulator instance.
 @param pChipId Pointer to receive the IP ID into.

 @return SONY_RESULT_OK if pChipId is valid.
*/
sony_result_t sony_demod_ChipID (sony_demod_t * pDemod, 
                                 sony_demod_chip_id_t * pChipId);

#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
/**
 @brief Monitor the value of the 10bit ADC on the RFAIN demodulator pin
 
 @param pDemod The demodulator instance.
 @param pRFAIN The value of the 10bit ADC RFAIN

 @return SONY_RESULT_OK if pRFAIN is valid.
*/
sony_result_t sony_demod_terr_cable_monitor_RFAIN (sony_demod_t * pDemod, 
                                                   uint16_t * pRFAIN);
#endif

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
sony_result_t sony_demod_SetAndSaveRegisterBits (sony_demod_t * pDemod, 
                                                 uint8_t slaveAddress, 
                                                 uint8_t bank, 
                                                 uint8_t registerAddress,
                                                 uint8_t value,
                                                 uint8_t bitMask);

#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
/**
 @brief Enable / disable scan mode for acquisition in the demodulator.

 @param pDemod The demodulator instance
 @param system The system used for scanning
 @param scanModeEnabled State of scan mode to set

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_terr_cable_SetScanMode (sony_demod_t * pDemod, 
                                      sony_dtv_system_t system,
                                      uint8_t scanModeEnabled);
#endif

/**
 @brief Clear the demodulator configuration memory table.  Use
        this API to empty the table of previous entries.  This is called 
        automatically in the sony_integ_InitializeT_C and 
        sony_integ_InitializeS API's.
*/
//static sony_result_t clearConfigMemory (sony_demod_t * pDemod);


/**
 @brief Set the TS clock mode and frequency based on the demod struct
        members.  Called internally as part of each Sleep to Active
        state transition.

 @param pDemod The demodulator instance
 @param system The tuning system

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_SetTsClockModeAndFreq (sony_demod_t * pDemod, sony_dtv_system_t system);
const char *FormatResult (sony_result_t result);

sony_result_t sony_integ_isdbt_Tune(sony_demod_t* pDemod, sony_isdbt_tune_param_t* pTuneParam);
sony_result_t sony_demod_isdbt_CheckDemodLock(sony_demod_t* pDemod,sony_demod_lock_result_t* pLock);
sony_result_t sony_integ_isdbt_WaitTSLock(sony_demod_t *pDemod);
sony_result_t sony_demod_ISDBT_TuneEnd(sony_demod_t * pDemod);
sony_result_t sony_demod_isdbt_EWSTune(sony_demod_t*  pDemod,sony_isdbt_tune_param_t* pTuneParam);
sony_result_t sony_demod_isdbt_CheckTSLock(sony_demod_t*pDemod,sony_demod_lock_result_t* pLock);
sony_result_t sony_demod_DisableTsDataPinHiZ(sony_demod_t * pDemod);
sony_result_t sony_demod_isdbt_SetTsClockModeAndFreq(sony_demod_t * pDemod);
sony_result_t sony_demod_isdbt_monitor_SyncStat(sony_demod_t* pDemod,uint8_t* pDmdLock,uint8_t* pTsLock,uint8_t* pUnlock);
sony_result_t sony_demod_isdbt_CheckTSLock(sony_demod_t*             pDemod,sony_demod_lock_result_t* pLock);
static sony_result_t ATtoAT(sony_demod_t* pDemod);

/*------------------------------------------------------------------------------
  Static Function Prototypes
------------------------------------------------------------------------------*/
/**
 @brief Configure the demodulator from any state to Sleep state.
        This is used as a demodulator reset, all demodulator configuration
        settings will be lost.
*/
sony_result_t XtoSLT(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from Shutdown to Sleep state
*/
sony_result_t SDtoSLT(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any Sleep state to Shutdown
*/
sony_result_t SLTtoSD(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any Sleep state to EWS
*/
sony_result_t SLTtoEWS(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any EWS state to Sleep
*/
sony_result_t EWStoSLT(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any Sleep state to Active
*/
sony_result_t SLTtoAT(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any Active state to Sleep
*/
sony_result_t ATtoSLT(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any EWS state to Active
*/
sony_result_t EWStoAT(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any Active state to EWS
*/
sony_result_t ATtoEWS(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any EWS state to EWS
*/
sony_result_t EWStoEWS(sony_demod_t* pDemod);

/**
 @brief Configure the demodulator from any Active state to Active
*/
sony_result_t ATtoAT(sony_demod_t* pDemod);


/**
 @brief set Each BW common setting

 @param  pDemod Reference to memory allocated for the demodulator instance. The create 
        function will setup this demodulator instance.
*/
sony_result_t setCommonDemodParam(sony_demod_t* pDemod);

/**
 @brief 8MHzBW setting

 @param  pDemod Reference to memory allocated for the demodulator instance. The create 
        function will setup this demodulator instance.
*/
sony_result_t setDemodParamFor8Mhz(sony_demod_t* pDemod);

/**
 @brief 7MHzBW setting

 @param  pDemod Reference to memory allocated for the demodulator instance. The create 
        function will setup this demodulator instance.
*/
sony_result_t setDemodParamFor7Mhz(sony_demod_t* pDemod);

/**
 @brief 6MHzBW setting

 @param  pDemod Reference to memory allocated for the demodulator instance. The create 
        function will setup this demodulator instance.
*/
sony_result_t setDemodParamFor6Mhz(sony_demod_t* pDemod);

#endif


#endif /* SONY_DEMOD_H */
