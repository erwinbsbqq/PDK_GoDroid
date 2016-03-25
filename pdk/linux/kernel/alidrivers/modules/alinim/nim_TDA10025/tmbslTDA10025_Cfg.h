/**
 * Copyright (C) 2005 Koninklijke Philips Electronics N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of Koninklijke Philips Electronics N.V. and is confidential in
 * nature. Under no circumstances is this software to be  exposed to or placed
 * under an Open Source License of any type without the expressed written
 * permission of Koninklijke Philips Electronics N.V.
 *
 * \file          tmbslTDA10025_Cfg.h
 *
 * \date          02-Oct-2007
 *
 * \brief         -
 *
 * REFERENCE DOCUMENTS :
 *
 * Detailled description may be added here.
 *
 * \section info Change Information
 *
 * \par Changelog
 *  -0.0.0.8 Make XTAL PLL configurable
 *  -0.0.0.7 Add component under XML
 *  -0.0.0.7 First version for release maker
 *
 * \par Version_Definition
 *  VERSION_TAG:TDA10025_SETTINGS_CUSTOMER_NUM.TDA10025_SETTINGS_PROJECT_NUM.TDA10025_SETTINGS_MAJOR_VER.TDA10025_SETTINGS_MINOR_VER
*/

#ifndef TDA10025_CFG_H 
#define TDA10025_CFG_H

/*============================================================================*/
/*                       INCLUDE FILES                                        */
/*============================================================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/* Types and defines:                                                         */
/*============================================================================*/
/* Driver settings version definition */
#define TDA10025_SETTINGS_CUSTOMER_NUM 0  /* SW Settings Customer Number */
#define TDA10025_SETTINGS_PROJECT_NUM  0  /* SW Settings Project Number  */
#define TDA10025_SETTINGS_MAJOR_VER    0  /* SW Settings Major Version   */
#define TDA10025_SETTINGS_MINOR_VER    8  /* SW Settings Minor Version   */

/*============================================================================*/
/*                       ENUM OR TYPE DEFINITION                              */
/*============================================================================*/
/*============================================================================*/
/* macro define                                                               */
/*============================================================================*/

/*============================================================================*/
/* bsl type define                                                            */
/*============================================================================*/
#define TDA10025_Cfg_VERSION 8

/*============================================================================*/
/* SET current configuration HERE                                             */
/*============================================================================*/
/*============================================================================*/
/* All paths with the same configuration                                      */
/* Flow: IF0->CDP1->TS0                                                       */
/*       IF1->CDP0->TS1                                                       */
/* TS:   serial                                                               */
/* TS:   clock output                                                         */
/*============================================================================*/

#define TMBSL_TDA10025_INSTANCE_CUSTOM_0                           \
    TDA10025_Cfg_VERSION,                      /* CompatibilityNb_U */ \
    TDA10025_IF_0,                             /* eIF               */ \
    TDA10025_CDP_1,                            /* eCDP              */ \
    TDA10025_TS_0,                             /* eTS               */ \
    TDA10025_TsModeSerial,                     /* eTsMode           */ \
    TDA10025_TsCfg_ClkDirIsOutput,             /* eTsClkDir         */ \
    TDA10025_TsCfg_ClkPolInversion,            /* eTsClkPol         */ \
    TDA10025_TsCfg_ClkInStdbyOff,              /* eTsClkStdby       */ \
    2,                                         /* ulTsClkDiv        */ \
    TDA10025_ExtendSymbolRateModeEnable1500ppm
    //TDA10025_ExtendSymbolRateModeEnable700ppm  /* eExtendSRMode     */
    //

#if 1

#define TMBSL_TDA10025_INSTANCE_CUSTOM_1                           \
    TDA10025_Cfg_VERSION,                      /* CompatibilityNb_U */ \
    TDA10025_IF_0,                             /* eIF               */ \
    TDA10025_CDP_1,                            /* eCDP              */ \
    TDA10025_TS_0,                             /* eTS               */ \
    TDA10025_TsModeSerial,                     /* eTsMode           */ \
    TDA10025_TsCfg_ClkDirIsOutput,             /* eTsClkDir         */ \
    TDA10025_TsCfg_ClkPolInversion,            /* eTsClkPol         */ \
    TDA10025_TsCfg_ClkInStdbyOff,              /* eTsClkStdby       */ \
    2,                                         /* ulTsClkDiv        */ \
    TDA10025_ExtendSymbolRateModeEnable1500ppm
    //TDA10025_ExtendSymbolRateModeEnable700ppm  /* eExtendSRMode     */
    //



#define TMBSL_TDA10025_INSTANCE_CUSTOM_2                           \
    TDA10025_Cfg_VERSION,                      /* CompatibilityNb_U */ \
    TDA10025_IF_0,                             /* eIF               */ \
    TDA10025_CDP_1,                            /* eCDP              */ \
    TDA10025_TS_0,                             /* eTS               */ \
    TDA10025_TsModeSerial,                     /* eTsMode           */ \
    TDA10025_TsCfg_ClkDirIsOutput,             /* eTsClkDir         */ \
    TDA10025_TsCfg_ClkPolInversion,            /* eTsClkPol         */ \
    TDA10025_TsCfg_ClkInStdbyOff,              /* eTsClkStdby       */ \
    2,                                         /* ulTsClkDiv        */ \
    TDA10025_ExtendSymbolRateModeEnable1500ppm
    //TDA10025_ExtendSymbolRateModeEnable700ppm  /* eExtendSRMode     */
    //



#else
    
#define TMBSL_TDA10025_INSTANCE_CUSTOM_1                           \
    TDA10025_Cfg_VERSION,                      /* CompatibilityNb_U */ \
    TDA10025_IF_1,                             /* eIF               */ \
    TDA10025_CDP_0,                            /* eCDP              */ \
    TDA10025_TS_1,                             /* eTS               */ \
    TDA10025_TsModeSerial,                     /* eTsMode           */ \
    TDA10025_TsCfg_ClkDirIsOutput,             /* eTsClkDir         */ \
    TDA10025_TsCfg_ClkPolInversion,            /* eTsClkPol         */ \
    TDA10025_TsCfg_ClkInStdbyOff,              /* eTsClkStdby       */ \
    2,                                         /* ulTsClkDiv        */ \
    TDA10025_ExtendSymbolRateModeEnable700ppm  /* eExtendSRMode     */

#define TMBSL_TDA10025_INSTANCE_CUSTOM_2                           \
    TDA10025_Cfg_VERSION,                      /* CompatibilityNb_U */ \
    TDA10025_IF_0,                             /* eIF               */ \
    TDA10025_CDP_1,                            /* eCDP              */ \
    TDA10025_TS_0,                             /* eTS               */ \
    TDA10025_TsModeSerial,                     /* eTsMode           */ \
    TDA10025_TsCfg_ClkDirIsOutput,             /* eTsClkDir         */ \
    TDA10025_TsCfg_ClkPolInversion,            /* eTsClkPol         */ \
    TDA10025_TsCfg_ClkInStdbyOff,              /* eTsClkStdby       */ \
    2,                                         /* ulTsClkDiv        */ \
    TDA10025_ExtendSymbolRateModeEnable700ppm  /* eExtendSRMode     */

#endif

#define TMBSL_TDA10025_INSTANCE_CUSTOM_3                           \
    TDA10025_Cfg_VERSION,                      /* CompatibilityNb_U */ \
    TDA10025_IF_1,                             /* eIF               */ \
    TDA10025_CDP_0,                            /* eCDP              */ \
    TDA10025_TS_1,                             /* eTS               */ \
    TDA10025_TsModeSerial,                     /* eTsMode           */ \
    TDA10025_TsCfg_ClkDirIsOutput,             /* eTsClkDir         */ \
    TDA10025_TsCfg_ClkPolInversion,            /* eTsClkPol         */ \
    TDA10025_TsCfg_ClkInStdbyOff,              /* eTsClkStdby       */ \
    2,                                         /* ulTsClkDiv        */ \
    TDA10025_ExtendSymbolRateModeEnable700ppm  /* eExtendSRMode     */

#define TMBSL_TDA10025_INSTANCE_CUSTOM_4                           \
    TDA10025_Cfg_VERSION,                      /* CompatibilityNb_U */ \
    TDA10025_IF_0,                             /* eIF               */ \
    TDA10025_CDP_1,                            /* eCDP              */ \
    TDA10025_TS_0,                             /* eTS               */ \
    TDA10025_TsModeSerial,                     /* eTsMode           */ \
    TDA10025_TsCfg_ClkDirIsOutput,             /* eTsClkDir         */ \
    TDA10025_TsCfg_ClkPolInversion,            /* eTsClkPol         */ \
    TDA10025_TsCfg_ClkInStdbyOff,              /* eTsClkStdby       */ \
    2,                                         /* ulTsClkDiv        */ \
    TDA10025_ExtendSymbolRateModeEnable700ppm  /* eExtendSRMode     */

#define TMBSL_TDA10025_INSTANCE_CUSTOM_5                           \
    TDA10025_Cfg_VERSION,                      /* CompatibilityNb_U */ \
    TDA10025_IF_1,                             /* eIF               */ \
    TDA10025_CDP_0,                            /* eCDP              */ \
    TDA10025_TS_1,                             /* eTS               */ \
    TDA10025_TsModeSerial,                     /* eTsMode           */ \
    TDA10025_TsCfg_ClkDirIsOutput,             /* eTsClkDir         */ \
    TDA10025_TsCfg_ClkPolInversion,            /* eTsClkPol         */ \
    TDA10025_TsCfg_ClkInStdbyOff,              /* eTsClkStdby       */ \
    2,                                         /* ulTsClkDiv        */ \
    TDA10025_ExtendSymbolRateModeEnable700ppm  /* eExtendSRMode     */

/* PLL configuration */
#define TDA10025_DEFAULT_XTAL TDA10025_XtalFreq_16MHz // current default value
/* #define TDA10025_DEFAULT_XTAL TDA10025_XtalFreq_27MHz */

/*============================================================================*/
/*                       EXTERN FUNCTION PROTOTYPES                           */
/*============================================================================*/

#endif
/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/
