/*****************************************************************************
*    Copyright (C)2003 Ali Corporation. All Rights Reserved.
*
*    File:    nim.c
*
*    Description:    This file contains mt312 basic function in HLD.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Feb.16.2003      Justin Wu       Ver 0.1    Create file.
*	2.	Jun.12.2003      George jiang    Ver 0.2    Porting NIM.
*   3.  Aug.22.2003      Justin Wu       Ver 0.3    Update.
*	4. July 17.2006	Joey Gao		Ver 0.4 		Add Diseqc 2.X support.
*****************************************************************************/


#ifndef	__HLD_NIM_H__
#define __HLD_NIM_H__

#include <types.h>
#include <hld/nim/nim_dev.h>



//ATSC
#if (SYS_PROJECT_FE == PROJECT_FE_ATSC)	//dedicate for DVBT
//#define SYS_ATSC		0x00
//#define SYS_CATV		0x01
#define CABLE_STD		0x01
#define CABLE_HRC		0x02
#define CABLE_IRC		0x03

#define MOD_8VSB		0x00
#define MOD_16VSB		0x01
#define MOD_64QAM		0x02
#define MOD_256QAM		0x03
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* Socket management functions */
INT32 nim_open(struct nim_device *dev);
INT32 nim_close(struct nim_device *dev);
INT32 nim_io_control(struct nim_device *dev, INT32 cmd, UINT32 param);
INT32 nim_get_lock(struct nim_device *dev, UINT8 *lock);
INT32 nim_get_freq(struct nim_device *dev, UINT32 *freq);
INT32 nim_get_FEC(struct nim_device *dev, UINT8 *fec);
INT32 nim_get_SNR(struct nim_device *dev, UINT8 *snr);
INT32 nim_ioctl_ext(struct nim_device *dev, INT32 cmd, void * param_list);

#if ( SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2 )	//dedicate for DVBS

INT32 nim_set_polar(struct nim_device *dev, UINT8 polar);
INT32 nim_set_12v(struct nim_device *dev, UINT8 flag);
INT32 nim_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);
INT32 nim_channel_search(struct nim_device *dev, UINT32 freq);
INT32 nim_DiSEqC_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt);
INT32 nim_DiSEqC2X_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt, UINT8 *rt_value, UINT8 *rt_cnt);

INT32 nim_get_sym(struct nim_device *dev, UINT32 *sym);
INT32 nim_get_BER(struct nim_device *dev, UINT32 *ber);
INT32 nim_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32* start_adr);
INT32 nim_get_AGC(struct nim_device *dev, UINT8 *agc);

#elif (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ATSC) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT) //dedicate for DVBT

INT32 nim_disable(struct nim_device *dev);//Sam_chen 20050616
INT32 nim_channel_change(struct nim_device * dev, UINT32 freq, UINT32 bandwidth,UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT8 priority);
INT32 nim_channel_search(struct nim_device *dev, UINT32 freq,UINT32 bandwidth,UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT16 freq_offset, UINT8 priority);
INT32 nim_get_gi(struct nim_device *dev, UINT8 *guard_interval);
INT32 nim_get_fftmode(struct nim_device *dev, UINT8 *fft_mode);
INT32 nim_get_modulation(struct nim_device *dev, UINT8 *modulation);
INT32 nim_get_spec_inv(struct nim_device *dev, UINT8 *inv);
INT32 nim_get_AGC(struct nim_device *dev, UINT8 *agc);
INT32 nim_get_HIER_mode(struct nim_device *dev, UINT8*hier);
INT8 nim_get_priority(struct nim_device *dev, UINT8*priority);
#ifdef SMART_ANT_SUPPORT
INT32 nim_Set_Smartenna(struct nim_device *dev, UINT8 position,UINT8 gain,UINT8 pol,UINT8 channel);
INT32 nim_Get_SmartennaSetting(struct nim_device *dev,UINT8 *pPosition,UINT8 *pGain,UINT8 *pPol,UINT8 *pChannel);
INT32 nim_Get_SmartennaMetric(struct nim_device *dev, UINT8 metric,UINT16 *pMetric);
INT32 nim_get_VSB_AGC(struct nim_device *dev, UINT16 *agc);
INT32 nim_get_VSB_SNR(struct nim_device *dev, UINT16 *snr);
INT32 nim_get_VSB_PER(struct nim_device *dev, UINT32 *per);
#endif




#elif ( SYS_PROJECT_FE == PROJECT_FE_DVBC )	//dedicate for DVBC
INT32 nim_get_sym(struct nim_device *dev, UINT32 *sym);
INT32 nim_get_BER(struct nim_device *dev, UINT32 *ber);
INT32 nim_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);
INT32 nim_quick_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);
INT32 nim_s3202_attach(struct QAM_TUNER_CONFIG_API * ptrQAM_Tuner);
INT32 nim_reg_read(struct nim_device *dev, UINT8 RegAddr, UINT8 *pData, UINT8 bLen);
INT32 nim_reg_write(struct nim_device *dev, UINT8 RegAddr, UINT8 *pData, UINT8 bLen);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __HLD_NIM_H__ */
