#ifndef __TUN_COMMON_H__
#define __TUN_COMMON_H__



#include "basic_types.h"

TUNER_IO_FUNC *tuner_setup(UINT32 type,UINT32 tuner_id);



//tda18250
extern INT32 tun_tda18250_init(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_tda18250_control_X(UINT32 Tun_id, UINT32 freq, UINT32 sym);
extern INT32 tun_tda18250_status(UINT32 Tun_id, UINT8 *lock);
extern INT32 tun_tda18250_close(UINT32 tuner_id);

//tda18250ab
extern INT32 tun_tda18250ab_init(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_tda18250ab_control(UINT32 Tun_id, UINT32 freq, UINT32 sym);
extern INT32 tun_tda18250ab_status(UINT32 Tun_id, UINT8 *lock);

//mxl603
extern INT32 tun_mxl603_init_DVBC(UINT32 *tuner_id , struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_mxl603_control_DVBC_X(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 tun_mxl603_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 tun_mxl603_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_mxl603_init_ISDBT(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)  ;
//extern INT32 tun_mxl603_init_ISDBT(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)  ;
extern INT32 tun_mxl603_init_CDT_MN88472(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config) ;
extern INT32 tun_mxl603_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth)	;

//av2012
extern INT32 ali_nim_av2011_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 ali_nim_av2011_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 ali_nim_av2011_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 ali_nim_av2011_close(void);


//sharp_vz7306
extern INT32 ali_nim_vz7306_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 ali_nim_vz7306_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 ali_nim_vz7306_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 ali_nim_vz7306_close(void);


//dct70701
extern INT32 tun_dct70701_init(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_dct70701_control(UINT32 Tun_id, UINT32 freq, UINT32 sym);//, UINT8 AGC_Time_Const, UINT8 _i2c_cmd
extern INT32 tun_dct70701_status(UINT32 Tun_id, UINT8 *lock);
extern INT32 tun_dct70701_release(void);


//sony cxd 2872 Tuner
extern INT32 tun_cxd_ascot3_Init(INT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_cxd_ascot3_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_cxd_ascot3_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_cxd_ascot3_release(void);
extern INT32 tun_cxd_ascot3_command(UINT32 tuner_id, INT32 cmd, UINT32 param);



#endif
