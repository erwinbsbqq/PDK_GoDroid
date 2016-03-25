/*****************************************************************************
*    Copyright (C)2006 Ali Corporation. All Rights Reserved.
*
*    File:    nim_sharp6158.h
*
*    Description:    Header file in LLD.
*    History:
*           Date            Athor        Version          Reason
*       ============    =============   =========   =================
*   1.  4.27.2006        Joey Gao        Ver 0.1       Create file.
*   
*****************************************************************************/

#ifndef __LLD_NIM_CXD2834_H__
#define __LLD_NIM_CXD2834_H__

#include "../porting_linux_header.h"
#include "porting_cxd2837_linux.h"
#include "sony_demod.h"
#include "sony_demod_integ.h"
#include "sony_demod_dvbt2_monitor.h"
#include "sony_demod_dvbt_monitor.h"


typedef struct AGC_Table
{
	UINT32 AGC;
	UINT32 SignalStrength;
}AGC_Table_t;

//define the current demo type as the same as S3821(DVBT)
#define    ISDBT_TYPE        0
#define    DVBT_TYPE         1
#define    DVBT2_TYPE        2
#define    DVBT2_COMBO       3

#if 0
#define PRINTK_INFO(x...) printk(KERN_INFO x)
#else
#define PRINTK_INFO(x...)
#endif

void  cxd2837_log_i2c(sony_demod_t* param, UINT8 err, UINT8 write, UINT8 slv_addr, UINT8 *data, int len);
void  nim_cxd2837_switch_lock_led(struct nim_device *dev, BOOL On);
BOOL need_to_lock_DVBT_signal(struct nim_device *dev, NIM_CHANNEL_CHANGE_T *change_para, BOOL NeedToConfigTuner);
INT32 need_to_lock_DVBT2_signal(struct nim_device *dev, NIM_CHANNEL_CHANGE_T *change_para,BOOL NeedToConfigTuner, BOOL *p_play_program);
INT32 try_to_lock_DVBT_signal(struct nim_device *dev, BOOL NeedToInitSystem,BOOL NeedToConfigTuner,NIM_CHANNEL_CHANGE_T *change_para);
INT32 DVBT2_change_PLP(struct nim_device *dev, UINT8 plp_id, BOOL NeedToInitSystem, BOOL NeedToConfigTuner, BOOL AUTOSCAN);
INT32 try_to_lock_next_data_plp(struct nim_device *dev, BOOL NeedToInitSystem, BOOL NeedToConfigTuner);
INT32 try_to_lock_DVBT2_signal(struct nim_device *dev, BOOL NeedToInitSystem, BOOL NeedToConfigTuner);
INT32 nim_cxd2837_channel_change_smart(struct nim_device *dev, NIM_CHANNEL_CHANGE_T *change_para);
UINT8 cxd2837_modulation_map_to_ali_modulation(sony_dtv_system_t system, UINT8 modulation);
INT32 nim_cxd2837_get_modulation(struct nim_device *dev, UINT8 *modulation);
UINT8 cxd2837_FEC_map_to_ali_FEC(sony_dtv_system_t system, UINT8 fec);
INT32 nim_cxd2837_get_FEC(struct nim_device *dev, UINT8* FEC);
UINT8 cxd2837_gi_map_to_ali_gi(sony_dtv_system_t system, UINT8 guard_interval);
INT32 nim_cxd2837_get_GI(struct nim_device *dev, UINT8 *guard_interval);
UINT8 cxd2837_fft_mode_map_to_ali_fft_mode(sony_dtv_system_t system, UINT8 fft_mode);
INT32 nim_cxd2837_get_fftmode(struct nim_device *dev, UINT8 *fft_mode);
INT32 nim_cxd2837_get_freq(struct nim_device *dev, UINT32 *freq);
INT32 nim_cxd2837_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth, UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT8 priority);
INT32 nim_cxd2837_get_SSI(struct nim_device *dev, UINT8 *data);
INT32 nim_cxd2837_get_SQI(struct nim_device *dev, UINT8 *snr);
INT32 nim_cxd2837_get_BER(struct nim_device *dev, UINT32 *BER);
INT32 nim_cxd2837_get_lock(struct nim_device *dev, UINT8 *lock);
INT32 nim_cxd2837_close(struct nim_device *dev);
INT32 nim_cxd2837_open(struct nim_device *dev);
void cxd2837_log_i2c(sony_demod_t* param, UINT8 err, UINT8 write, UINT8 slv_addr, UINT8 *data, int len);




#endif  /* __LLD_NIM_CXD2834_H__ */

