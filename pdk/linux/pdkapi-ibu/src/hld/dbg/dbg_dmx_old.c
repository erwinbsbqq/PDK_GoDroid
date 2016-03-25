/*****************************************************************************
*	Copyrights(C) 2012 Acer Laboratries inc. All Rights Reserved.
*
*	FILE NAME:		adr_dmx_dbg.c
*
*	DESCRIPTION:	DeMux debug module.
*
*	HISTORY:
*						Date 	 Author      Version 	  Notes
*					=========	=========	=========	===========
*					2012-09-30	Frank.Fan    Ver 1.0	Create File
*					2012-09-30	Leo.Ma       Ver 1.0	Update File
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <osal/osal_task.h>
#include <hld/adr_hld_dev.h>
#include <hld/dbg/adr_dbg_parser.h>
#include <hld/dmx/adr_dmx_inc.h>

#include <linux/Ali_DmxLib.h>
//#include <linux/dvb/ali_dmx.h>
#include <ali_dmx_common.h>
#include <hld_cfg.h>
#define __ADR_OLD_DMX_DBG 

#ifdef __ADR_OLD_DMX_DBG 
#define OLD_DMX_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(DMX, fmt, ##args); \
			} while(0)

#else
#define OLD_DMX_DBG_PRINT(...)
#endif


#define DMX_DEV_MAX_NUM		3

#define DMX_DSC_LINUX_DEV_PATH "/dev/ali_m36_dsc_0"

#define DMX_0_LINUX_DEV_PATH "/dev/ali_m36_dmx_0"

#define DMX_1_LINUX_DEV_PATH "/dev/ali_m36_dmx_1"

#define DMX_2_LINUX_DEV_PATH "/dev/ali_dmx_pb_0_out"

#define DMX_PB_IN_LINUX_DEV_PATH "/dev/ali_dmx_pb_0_in"

typedef enum dmx_chan_type
{
	DMX_CHAN_VIDEO_TYPE =0x1,
    DMX_CHAN_AUDIO_TYPE,
    DMX_CHAN_REC_TYPE,
    DMX_CHAN_SEC_TYPE,
    DMX_CHAN_PCR_TYPE,
    DMX_CHAN_PES_TYPE,    
}DMX_CHAN_TYPE;


#define DMX_DBG_CMD_NR		10
#define DMX_DBG_PRINT_INTRV	5000

typedef enum dmx_dbg_level
{
	DMX_DBG_LEVEL_HLD = 0x1,
    DMX_DBG_LEVEL_KERNEL = 0x2,
    DMX_DBG_LEVEL_SEE = 0x4,
} DMX_DBG_LEVEL;

typedef struct dmx_dbg_stat_ctrl
{
	INT32 KernGlbFd;
	INT32 SeeGlbFd;
	INT32 HwRegFd;

	UINT32 TaskId;

	UINT8 DmxDbgInit;
	UINT8 DmxDbgTskInit;

    UINT8 DmxDbgStatEn;
	UINT8 KernGlbStatEn;
	UINT8 SeeGlbStatEn;
    UINT8 IoCmdStatEn;
    UINT8 VidStatEn;
    UINT8 AudStatEn;
    UINT8 PcrStatEn;
    UINT8 SecStatEn;
    UINT8 PesStatEn;
    UINT8 TsStatEn;
	UINT8 MiscStatEn;
    UINT8 HwRegDumpEn;
    UINT32 DbgShowIntv; /* Unit: ms, default to 3000ms. */
	UINT32 DbgShowTms;

	DMX_DBG_LEVEL DbgLevel;
} dmx_dbg_stat_ctrl;


typedef enum dmx_dbg_info_type {
	DMX_DBG_SHOW_STATISTICS = 0,
	DMX_DBG_SHOW_STREAM_INFO = 1,
	DMX_DBG_SHOW_STREAM_INFO_SEC_SLOT = 2,
	DMX_DBG_SHOW_STREAM_INFO_SEC_FLT = 4,
	DMX_DBG_SHOW_STREAM_INFO_SEC_CH = 8,
} DMX_DBG_INFO_TYPE;

static struct dmx_dbg_stat_ctrl DmxOldDbgCtrl;

extern struct dmx_hld_porting g_dmx_hld_porting[3];



static INT32 DmxOldDbgStatDeviceOpen(UINT32 DmxId)
{

	INT32	Ret;
	UINT8 * DmxPath = NULL;

//	DmxPath = (UINT8 *)Ali_DmxId2Path(ALI_DMX_OUTPUT, DmxId);

    if(0 == DmxId)
    {
		DmxPath = (UINT8 *)DMX_0_LINUX_DEV_PATH;
    }
    else if (1 == DmxId)
    {
		DmxPath = (UINT8 *)DMX_1_LINUX_DEV_PATH;
    }
    else if (2 == DmxId)
    {
		DmxPath = (UINT8 *)DMX_2_LINUX_DEV_PATH;
    }

    DmxOldDbgCtrl.KernGlbFd = open(DmxPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

	if (DmxOldDbgCtrl.KernGlbFd < 0)
    {
        //OLD_DMX_DBG_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return (-1);
    }    

    DmxOldDbgCtrl.SeeGlbFd = open(DmxPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

	if (DmxOldDbgCtrl.SeeGlbFd < 0)
    {
        OLD_DMX_DBG_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return (-1);
    }    

    DmxOldDbgCtrl.HwRegFd = open(DmxPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

	if (DmxOldDbgCtrl.HwRegFd < 0)
    {
        OLD_DMX_DBG_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return (-1);
    }   

	Ret = ioctl(DmxOldDbgCtrl.KernGlbFd, ALI_DMX_CHAN_KERN_GLB_CFG, 0);

	if (Ret < 0)
	{
	    OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}


	Ret = ioctl(DmxOldDbgCtrl.SeeGlbFd, ALI_DMX_CHAN_SEE_GLB_CFG, 0);

	if (Ret < 0)
	{
	    OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	Ret = ioctl(DmxOldDbgCtrl.HwRegFd, ALI_DMX_CHAN_HW_REG_CFG, 0);

	if (Ret < 0)
	{
	    OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	return (0);
}

static void DmxOldDbgStatDeviceClose(UINT32 DmxId)
{
    if (DmxOldDbgCtrl.KernGlbFd > 0)
    {
		close(DmxOldDbgCtrl.KernGlbFd);
    }

    if (DmxOldDbgCtrl.SeeGlbFd > 0)
    {
		close(DmxOldDbgCtrl.SeeGlbFd);
    }
	
    if (DmxOldDbgCtrl.HwRegFd > 0)
    {
		close(DmxOldDbgCtrl.HwRegFd);
    }
	
	memset(&DmxOldDbgCtrl, 0, sizeof(DmxOldDbgCtrl));

	return;
}

static INT32 PrintOldKernelGlobalStatInfo(UINT32 SysTickInc)
{
	INT32								Ret;
	UINT32              	        	Bitrate;
    UINT32                  	    	TsInCntInc;
	static UINT32               		LastTsInCnt;
	struct Ali_DmxKernGlobalStatInfo	GlobalStatInfo;
	
	if (DmxOldDbgCtrl.KernGlbFd < 0)
	{
	    OLD_DMX_DBG_PRINT("Invalid DeMux debug kernel global FD: %d\n", DmxOldDbgCtrl.KernGlbFd);

		return (-1);
	}

	if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
	{	
		memset(&GlobalStatInfo, 0, sizeof(GlobalStatInfo));

		Ret = ioctl(DmxOldDbgCtrl.KernGlbFd, ALI_DMX_CHAN_KERN_GLB_INFO_GET, &GlobalStatInfo);

	    if (Ret < 0)
	    {
	        OLD_DMX_DBG_PRINT("RET=%d, %s(),L[%d],Fd[%d],%m\n", Ret, __FUNCTION__, __LINE__,DmxOldDbgCtrl.KernGlbFd ,errno);

			return (-1);
	    }

		OLD_DMX_DBG_PRINT("\nKernel Global:\n");

		if (SysTickInc != 0 && GlobalStatInfo.TotalTsInCnt > LastTsInCnt)
		{
			TsInCntInc = GlobalStatInfo.TotalTsInCnt - LastTsInCnt;
		    Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
		    OLD_DMX_DBG_PRINT("TotalBitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
		}
		LastTsInCnt = GlobalStatInfo.TotalTsInCnt;
		
		OLD_DMX_DBG_PRINT("-----------------------------------------------\n");

		OLD_DMX_DBG_PRINT("TotalTsInCnt:%u\n", GlobalStatInfo.TotalTsInCnt);

		OLD_DMX_DBG_PRINT("OverlapCnt:%u\n", GlobalStatInfo.OverlapCnt);

		if (GlobalStatInfo.PlayBusyCnt != 0)
		{
			OLD_DMX_DBG_PRINT("PlayBusyCnt:%u\n", GlobalStatInfo.PlayBusyCnt);
		}

		if (GlobalStatInfo.DmxBufEmptyCnt != 0)
		{
			OLD_DMX_DBG_PRINT("DmxBufEmptyCnt:%u\n", GlobalStatInfo.DmxBufEmptyCnt);
		}

		if (GlobalStatInfo.NobodyCareCnt != 0)
		{
			OLD_DMX_DBG_PRINT("NobodyCareCnt:%u\n", GlobalStatInfo.NobodyCareCnt);
		}
	}
	
	return (0);
}

static INT32 PrintOldSeeGlobalStatInfo(UINT32 SysTickInc)
{
	INT32                           Ret;
	UINT32                          Bitrate;
    UINT32                          TsInCntInc;
	static UINT32                   LastTsInCnt;	
	struct Ali_DmxSeeGlobalStatInfo GlobalStatInfo;

	if (DmxOldDbgCtrl.SeeGlbFd < 0)
	{
	    OLD_DMX_DBG_PRINT("Invalid DeMux debug SEE global FD: %d\n", DmxOldDbgCtrl.SeeGlbFd);

		return (-1);
	}

	if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_SEE)
	{	
		memset(&GlobalStatInfo, 0, sizeof(GlobalStatInfo));

		Ret = ioctl(DmxOldDbgCtrl.SeeGlbFd, ALI_DMX_CHAN_SEE_GLB_INFO_GET, &GlobalStatInfo);

	    if (Ret < 0)
	    {
	        OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

			return (-1);
	    }

		OLD_DMX_DBG_PRINT("\nSEE Global:\n");

		if (SysTickInc != 0 && GlobalStatInfo.TotalTsInCnt > LastTsInCnt)
		{
			TsInCntInc = GlobalStatInfo.TotalTsInCnt - LastTsInCnt;
		    Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
	    	OLD_DMX_DBG_PRINT("TotalBitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
		}
		LastTsInCnt = GlobalStatInfo.TotalTsInCnt;
		
		OLD_DMX_DBG_PRINT("-----------------------------------------------\n");

		OLD_DMX_DBG_PRINT("TotalTsInCnt:%u\n", GlobalStatInfo.TotalTsInCnt);

		if (GlobalStatInfo.TsOddCnt != 0)
		{
		    OLD_DMX_DBG_PRINT("TsOddCnt:%u\n", GlobalStatInfo.TsOddCnt);
		}
		
		if (GlobalStatInfo.TsSyncErrCnt != 0)
		{
	    	OLD_DMX_DBG_PRINT("TsSyncErrCnt:%u\n", GlobalStatInfo.TsSyncErrCnt);
		}
		
	    OLD_DMX_DBG_PRINT("TsDecrySucCnt:%u\n", GlobalStatInfo.TsDecrySucCnt);

		if (GlobalStatInfo.TsDecryFailCnt != 0)
		{
		    OLD_DMX_DBG_PRINT("TsDecryFailCnt:%u\n", GlobalStatInfo.TsDecryFailCnt);
		}
		
		if (GlobalStatInfo.TsDecryEmptyCnt != 0)
		{
		    OLD_DMX_DBG_PRINT("TsDecryEmptyCnt:%u\n", GlobalStatInfo.TsDecryEmptyCnt);
		}
		
		OLD_DMX_DBG_PRINT("TsRetToMainCnt:%u\n", GlobalStatInfo.TsRetToMainCnt);
	}
	
	return (0);
}

#if 0
static void PrintOldTsStatInfoHld(struct Ali_DmxTsStreamInfo* StreamInfo, INT32 FltIdx)
{
	struct Ali_DmxLibTsStrmStatInfo TsStrmStatInfo;

	if (NULL == StreamInfo)
	{
		return;
	}

	memset(&TsStrmStatInfo, 0, sizeof(TsStrmStatInfo));

#if 0
	if (Ali_DmxTsStreamErrStatGet(StreamInfo->Idx + ALI_DMX_STREAM_TYPE_TS, &TsStrmStatInfo))
	{
		return;
	}
#endif

	OLD_DMX_DBG_PRINT("\nTS Statistics StrIdx:%u PID:%u (HLD Part)\n", 
		StreamInfo->Idx, StreamInfo->StreamParam.PidList[FltIdx]);

	if (TsStrmStatInfo.InvPathCnt != 0)
	{
		OLD_DMX_DBG_PRINT("InvPathCnt:%u\n", TsStrmStatInfo.InvPathCnt);
	}

	if (TsStrmStatInfo.IoOpenFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoOpenFailCnt:%u\n", TsStrmStatInfo.IoOpenFailCnt);
	}

	if (TsStrmStatInfo.IoCfgFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoCfgFailCnt:%u\n", TsStrmStatInfo.IoCfgFailCnt);
	}

	if (TsStrmStatInfo.IoStartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoStartFailCnt:%u\n", TsStrmStatInfo.IoStartFailCnt);
	}

	if (TsStrmStatInfo.IoStopFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoStopFailCnt:%u\n", TsStrmStatInfo.IoStopFailCnt);
	}

	return;
}
#endif

/********************************************************
* Usd to Print the Record Stream info
*********************************************************/
static INT32 PrintOldTsStatInfoKernel(INT32 linux_fd, UINT32 pid, UINT32 FltListIdx, UINT32 SysTickInc)
{
	INT32							Ret;
	UINT32							Bitrate;
    UINT32							TsInCntInc;
	static UINT32					LastTsInCnt;
	struct Ali_DmxDrvTsStrmStatInfo	TsStrmStatInfo;
	struct Ali_DmxDrvTsFltStatInfo	TsFltStatInfo;


	OLD_DMX_DBG_PRINT("%s(): %d \n",__FUNCTION__, __LINE__);

	memset(&TsStrmStatInfo, 0, sizeof(TsStrmStatInfo));

	memset(&TsFltStatInfo, 0, sizeof(TsFltStatInfo));

	TsStrmStatInfo.TsFltIdx = FltListIdx; 

    Ret = ioctl(linux_fd, ALI_DMX_CHAN_TS_INFO_GET, &TsStrmStatInfo);

    if (Ret < 0)
    {
        OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
    }

	TsFltStatInfo.TsFltIdx = FltListIdx; 
    
    Ret = ioctl(linux_fd, ALI_DMX_CHAN_TS_FILTER_INFO_GET, &TsFltStatInfo);

    if (Ret < 0)
    {
        OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
    }

	OLD_DMX_DBG_PRINT("\n TS Statistics. PID:%u (Kernel Part)\n", pid); 


	if (SysTickInc != 0 && TsFltStatInfo.TsInCnt > LastTsInCnt)
	{
		TsInCntInc = TsFltStatInfo.TsInCnt - LastTsInCnt;
	    Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
	    OLD_DMX_DBG_PRINT("Bitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
	}
	LastTsInCnt = TsFltStatInfo.TsInCnt;
	
	OLD_DMX_DBG_PRINT("------------------------------------------------------\n");

	if (TsStrmStatInfo.StatErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StatErrCnt:%u\n", TsStrmStatInfo.StatErrCnt);
	}

	if (TsStrmStatInfo.StrTypeErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StrTypeErrCnt:%u\n", TsStrmStatInfo.StrTypeErrCnt);
	}

	if (TsStrmStatInfo.RdByteErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("RdByteErrCnt:%u\n", TsStrmStatInfo.RdByteErrCnt);
	}

	if (TsStrmStatInfo.WrByteErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("WrByteErrCnt:%u\n", TsStrmStatInfo.WrByteErrCnt);
	}

	if (TsStrmStatInfo.NoPidCnt != 0)
	{
		OLD_DMX_DBG_PRINT("NoPidCnt:%u\n", TsStrmStatInfo.NoPidCnt);
	}

	if (TsStrmStatInfo.CopyErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CopyErrCnt:%u\n", TsStrmStatInfo.CopyErrCnt);
	}

	if (TsStrmStatInfo.CfgFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CfgFailCnt:%u\n", TsStrmStatInfo.CfgFailCnt);
	}

	if (TsStrmStatInfo.StartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StartFailCnt:%u\n", TsStrmStatInfo.StartFailCnt);
	}

	if (TsStrmStatInfo.StopFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StopFailCnt:%u\n", TsStrmStatInfo.StopFailCnt);
	}

	if (TsStrmStatInfo.CloseFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CloseFailCnt:%u\n", TsStrmStatInfo.CloseFailCnt);
	}

	OLD_DMX_DBG_PRINT("TsInCnt:%u\n", TsFltStatInfo.TsInCnt);

	OLD_DMX_DBG_PRINT("TsScrmbCnt:%u\n", TsFltStatInfo.TsScrmbCnt);

	if (TsFltStatInfo.TsSyncByteErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsSyncByteErrCnt:%u\n", TsFltStatInfo.TsSyncByteErrCnt);
	}
	
	if (TsFltStatInfo.TsErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsErrCnt:%u\n", TsFltStatInfo.TsErrCnt);
	}
	
	if (TsFltStatInfo.TsDupCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsDupCnt:%u\n", TsFltStatInfo.TsDupCnt);
	}
	
	if (TsFltStatInfo.TsLostCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsLostCnt:%u\n", TsFltStatInfo.TsLostCnt);
	}
					
	return (0);
}


static void PrintOldSecStrmStatInfoHld(struct dmx_hld_porting_sec_slot *sec_info)
{
	UINT32 						pid = 0;
    UINT32 						i, j;
    struct get_section_param	*sec_para;
    struct restrict 			*mask_info;
	struct Ali_DmxSlotSecStatInfo *stat_info;

    sec_para = (struct get_section_param *)(sec_info->sec_para);
    mask_info = (struct restrict *)(sec_para->mask_value);
	stat_info = &sec_info->stat_info;

    pid = sec_para->pid;

	OLD_DMX_DBG_PRINT("\nSection Stream Statistics... PID:%u (HLD Part)\n", pid );

	OLD_DMX_DBG_PRINT("----------------------------------------------------------\n");

	OLD_DMX_DBG_PRINT("SecCallbackCnt:%u\n", stat_info->SecCallbackCnt);

	if (stat_info->IoOpenFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoOpenFailCnt:%u\n", stat_info->IoOpenFailCnt);
	}

	if (stat_info->IoStartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoStartFailCnt:%u\n", stat_info->IoStartFailCnt);
	}

	if (stat_info->SecMaskMissCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SecMaskMissCnt:%u\n", stat_info->SecMaskMissCnt);
	}

	if (stat_info->SecCrcErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SecCrcErrCnt:%u\n", stat_info->SecCrcErrCnt);
	}

	if (NULL != sec_para->buff)
	{
		OLD_DMX_DBG_PRINT("buff(Start addr):0x%x\n", sec_para->buff);
	}
    else
	{
        OLD_DMX_DBG_PRINT(" Invalid Start Buff \n");
    }        

	if (0 != sec_para->cur_pos )
	{
		OLD_DMX_DBG_PRINT("cur_pos:%u\n", sec_para->cur_pos);
	}

	if (0 != sec_para->buff_len )
	{
	    OLD_DMX_DBG_PRINT("buff_len:%u\n", sec_para->buff_len);
	}

    OLD_DMX_DBG_PRINT("sec_tbl_len:%u\n", sec_para->sec_tbl_len);
    OLD_DMX_DBG_PRINT("get_sec_len:%u\n", sec_para->get_sec_len);
    OLD_DMX_DBG_PRINT("conti_conter:%u\n", sec_para->conti_conter);
    OLD_DMX_DBG_PRINT("wai_flg_dly:%u\n", sec_para->wai_flg_dly);
    OLD_DMX_DBG_PRINT("dmx_state:%u\n", sec_para->dmx_state);

	if (NULL != sec_para->get_sec_cb )
	{
	    OLD_DMX_DBG_PRINT(" *get_sec_cb(Function Addr):0x%x\n", sec_para->get_sec_cb);
	}

    OLD_DMX_DBG_PRINT("sec_hit_num:%u\n", sec_para->sec_hit_num);
    OLD_DMX_DBG_PRINT(" continue_get_sec:%u\n", sec_para->continue_get_sec);
    OLD_DMX_DBG_PRINT("retrieve_sec_fmt:%u\n", sec_para->retrieve_sec_fmt);

    if(NULL != sec_para->mask_value )
	{
		OLD_DMX_DBG_PRINT("mask(HEX): ");	
        for( i = 0; i < MAX_SEC_MASK_LEN ; i++)
            OLD_DMX_DBG_PRINT("%2x ", mask_info->mask[i]);

		OLD_DMX_DBG_PRINT("\n Value(HEX): \n");
        for( i = 0; i <  MAX_MV_NUM; i++)
        {
        	for(j = 0; j < MAX_SEC_MASK_LEN; j++)
				OLD_DMX_DBG_PRINT("%2x ", mask_info->value[i][j]);

            OLD_DMX_DBG_PRINT("\n");
        }

		OLD_DMX_DBG_PRINT("\n mask_len: %u \n", mask_info->mask_len);

        OLD_DMX_DBG_PRINT("\n value_num: %u \n", mask_info->value_num);

		OLD_DMX_DBG_PRINT("\n multi_mask(HEX): \n");
		for( i = 0; i <  MAX_MV_NUM; i++)
		{
		    for(j = 0; j < MAX_SEC_MASK_LEN; j++)
		        OLD_DMX_DBG_PRINT("%2x ", mask_info->multi_mask[i][j]);

		    OLD_DMX_DBG_PRINT("\n");
		}

        OLD_DMX_DBG_PRINT("tb_flt_msk: %u \n", mask_info->tb_flt_msk);
	}

	return;
}

#if 0
static void PrintOldSecSlotStatInfoHld(struct Ali_DmxSecSlotInfo* SlotInfo)
{
	struct Ali_DmxLibSecSlotStatInfo SecSlotStatInfo;

	if (NULL == SlotInfo)
	{
		return;
	}

	memset(&SecSlotStatInfo, 0, sizeof(SecSlotStatInfo));

	if (Ali_DmxSecSlotErrStatGet(SlotInfo->Idx + ALI_DMX_STREAM_TYPE_SEC_SLOT, &SecSlotStatInfo))
	{
		return;
	}
	
	OLD_DMX_DBG_PRINT("\nSection Slot Statistics SlotIdx:%u StrIdx:%u PID:%u (HLD Part)\n", 
		SlotInfo->Idx, SlotInfo->SecStreamId, SlotInfo->SlotParam.Pid);

	OLD_DMX_DBG_PRINT("---------------------------------------------------------\n");

	OLD_DMX_DBG_PRINT("CallbackCnt:%u\n", SecSlotStatInfo.CallbackCnt);

	if (SecSlotStatInfo.CbTimeOutCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CbTimeOutCnt:%u\n", SecSlotStatInfo.CbTimeOutCnt);
	}
	
	if (SecSlotStatInfo.StatErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StatErrCnt:%u\n", SecSlotStatInfo.StatErrCnt);
	}
	
	if (SecSlotStatInfo.NoSecBufCnt != 0)
	{
		OLD_DMX_DBG_PRINT("NoSecBufCnt:%u\n", SecSlotStatInfo.NoSecBufCnt);
	}
	
	if (SecSlotStatInfo.CallbackErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CallbackErrCnt:%u\n", SecSlotStatInfo.CallbackErrCnt);
	}
	
	if (SecSlotStatInfo.ThreadCreatErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("ThreadCreatErrCnt:%u\n", SecSlotStatInfo.ThreadCreatErrCnt);
	}

	if (SecSlotStatInfo.StrmOpenFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StrmOpenFailCnt:%u\n", SecSlotStatInfo.StrmOpenFailCnt);
	}

	if (SecSlotStatInfo.StrmCfgFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StrmCfgFailCnt:%u\n", SecSlotStatInfo.StrmCfgFailCnt);
	}

	if (SecSlotStatInfo.StrmStartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StrmStartFailCnt:%u\n", SecSlotStatInfo.StrmStartFailCnt);
	}

	if (SecSlotStatInfo.StrmStartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StrmStartFailCnt:%u\n", SecSlotStatInfo.StrmStartFailCnt);
	}

	if (SecSlotStatInfo.StrmCloseFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StrmCloseFailCnt:%u\n", SecSlotStatInfo.StrmCloseFailCnt);
	}	

	return;
}


static void PrintOldSecFltStatInfoHld(struct Ali_DmxSecFltInfo* FltInfo)
{
	struct Ali_DmxLibSecFltStatInfo SecFltStatInfo;

	if (NULL == FltInfo)
	{
		return;
	}
	
	memset(&SecFltStatInfo, 0, sizeof(SecFltStatInfo));

	if (Ali_DmxSecFltErrStatGet(FltInfo->Idx + ALI_DMX_STREAM_TYPE_SEC_FLT, &SecFltStatInfo))
	{
		return;
	}
	
	OLD_DMX_DBG_PRINT("\nSection Filter Statistics FltIdx:%u ChIdx:%u SlotId:0x%x (HLD Part)\n", 
		FltInfo->Idx, FltInfo->ChIdx, FltInfo->SecSlotId);

	OLD_DMX_DBG_PRINT("----------------------------------------------------------------------\n");

	OLD_DMX_DBG_PRINT("CallbackCnt:%u\n", SecFltStatInfo.CallbackCnt);

	if (SecFltStatInfo.StatErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StatErrCnt:%u\n", SecFltStatInfo.StatErrCnt);
	}
	
	if (SecFltStatInfo.SlotOpenFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SlotOpenFailCnt:%u\n", SecFltStatInfo.SlotOpenFailCnt);
	}
	
	if (SecFltStatInfo.SlotCfgFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SlotCfgFailCnt:%u\n", SecFltStatInfo.SlotCfgFailCnt);
	}
	
	if (SecFltStatInfo.SlotStartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SlotStartFailCnt:%u\n", SecFltStatInfo.SlotStartFailCnt);
	}
	
	if (SecFltStatInfo.SlotStopFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SlotStopFailCnt:%u\n", SecFltStatInfo.SlotStopFailCnt);
	}

	if (SecFltStatInfo.SlotCloseFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SlotCloseFailCnt:%u\n", SecFltStatInfo.SlotCloseFailCnt);
	}

	return;
}

static void PrintOldSecChStatInfoHld(struct Ali_DmxSecChInfo* ChInfo)
{
	struct Ali_DmxLibSecChStatInfo SecChStatInfo;

	if (NULL == ChInfo)
	{
		return;
	}
	
	memset(&SecChStatInfo, 0, sizeof(SecChStatInfo));

	if (Ali_DmxSecChErrStatGet(ChInfo->Idx + ALI_DMX_STREAM_TYPE_SEC_CH, &SecChStatInfo))
	{
		return;
	}
	
	OLD_DMX_DBG_PRINT("\nSection Channel Statistics ChIdx:%u PID:%u (HLD Part)\n", 
		ChInfo->Idx, ChInfo->ChParam.Pid);

	OLD_DMX_DBG_PRINT("---------------------------------------------------\n");
	
	if (SecChStatInfo.StatErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StatErrCnt:%u\n", SecChStatInfo.StatErrCnt);
	}
	
	if (SecChStatInfo.DupFltCnt != 0)
	{
		OLD_DMX_DBG_PRINT("DupFltCnt:%u\n", SecChStatInfo.DupFltCnt);
	}
	
	if (SecChStatInfo.DupPidCnt != 0)
	{
		OLD_DMX_DBG_PRINT("DupPidCnt:%u\n", SecChStatInfo.DupPidCnt);
	}

	return;
}

#endif


static INT32 PrintOldSectionStatInfoKernel(INT32 linux_fd, UINT32 pid, UINT32 SysTickInc)
{
	INT32								Ret;
	UINT32								Bitrate;
    UINT32								TsInCntInc;
	static UINT32						LastTsInCnt;
    
	struct Ali_DmxDrvSecStrmStatInfo	SecStrmStatInfo;
	struct Ali_DmxDrvTsFltStatInfo		TsFltStatInfo;
	struct Ali_DmxDrvSecFltStatInfo		SecFltStatInfo;

	
	memset(&TsFltStatInfo, 0, sizeof(TsFltStatInfo));

	memset(&SecStrmStatInfo, 0, sizeof(SecStrmStatInfo));

	memset(&SecFltStatInfo, 0, sizeof(SecFltStatInfo));


	Ret = ioctl(linux_fd, ALI_DMX_CHAN_SEC_TS_FILTER_INFO_GET, &TsFltStatInfo);

	if (Ret < 0)
	{
		OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}


	Ret = ioctl(linux_fd, ALI_DMX_CHAN_SEC_INFO_GET, &SecStrmStatInfo);

	if (Ret < 0)
	{
		OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}	

	Ret = ioctl(linux_fd, ALI_DMX_CHAN_SEC_FILTER_INFO_GET, &SecFltStatInfo);

	if (Ret < 0)
	{
		OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	OLD_DMX_DBG_PRINT("\n Section Statistics.   PID:%u (Kernel Part)\n", pid ); 

	if (SysTickInc != 0 && TsFltStatInfo.TsInCnt > LastTsInCnt)
	{
		TsInCntInc = TsFltStatInfo.TsInCnt - LastTsInCnt;
		Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
		OLD_DMX_DBG_PRINT("Bitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
	}
	LastTsInCnt = TsFltStatInfo.TsInCnt;

	OLD_DMX_DBG_PRINT("--------------------------------------------------------------------------------\n");

	if (SecStrmStatInfo.CbTypeErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CbTypeErrCnt:%u\n", SecStrmStatInfo.CbTypeErrCnt);
	}

	if (SecStrmStatInfo.StrTypeErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StrTypeErrCnt:%u\n", SecStrmStatInfo.StrTypeErrCnt);
	}

	if (SecStrmStatInfo.StatErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StatErrCnt:%u\n", SecStrmStatInfo.StatErrCnt);
	}

	if (SecStrmStatInfo.WrByteErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("WrByteErrCnt:%u\n", SecStrmStatInfo.WrByteErrCnt);
	}

	if (SecStrmStatInfo.RdBufShortCnt != 0)
	{
		OLD_DMX_DBG_PRINT("RdBufShortCnt:%u\n", SecStrmStatInfo.RdBufShortCnt);
	}

	if (SecStrmStatInfo.CopyErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CopyErrCnt:%u\n", SecStrmStatInfo.CopyErrCnt);
	}

	if (SecStrmStatInfo.CfgFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CfgFailCnt:%u\n", SecStrmStatInfo.CfgFailCnt);
	}

	if (SecStrmStatInfo.StartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StartFailCnt:%u\n", SecStrmStatInfo.StartFailCnt);
	}

	if (SecStrmStatInfo.StopFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StopFailCnt:%u\n", SecStrmStatInfo.StopFailCnt);
	}

	if (SecStrmStatInfo.CloseFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CloseFailCnt:%u\n", SecStrmStatInfo.CloseFailCnt);
	}

	OLD_DMX_DBG_PRINT("TsInCnt:%u\n", TsFltStatInfo.TsInCnt);

	OLD_DMX_DBG_PRINT("TsScrmbCnt:%u\n", TsFltStatInfo.TsScrmbCnt);

	if (TsFltStatInfo.TsSyncByteErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsSyncByteErrCnt:%u\n", TsFltStatInfo.TsSyncByteErrCnt);
	}
	
	if (TsFltStatInfo.TsErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsErrCnt:%u\n", TsFltStatInfo.TsErrCnt);
	}

	if (TsFltStatInfo.TsDupCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsDupCnt:%u\n", TsFltStatInfo.TsDupCnt);
	}

	if (TsFltStatInfo.TsLostCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsLostCnt:%u\n", TsFltStatInfo.TsLostCnt);
	}

	OLD_DMX_DBG_PRINT("SecInCnt:%u\n", SecFltStatInfo.SecInCnt);

	OLD_DMX_DBG_PRINT("SecOutCnt:%u\n", SecFltStatInfo.SecOutCnt);

	if (SecFltStatInfo.SecTsNoPayloadCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SecTsNoPayloadCnt:%u\n", SecFltStatInfo.SecTsNoPayloadCnt);
	}

	if (SecFltStatInfo.SecTsScrmbCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SecTsScrmbCnt:%u\n", SecFltStatInfo.SecTsScrmbCnt);
	}
				
	if (SecFltStatInfo.SecTsLostCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SecTsLostCnt:%u\n", SecFltStatInfo.SecTsLostCnt);
	}
				
	if (SecFltStatInfo.SecPtErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SecPtErrCnt:%u\n", SecFltStatInfo.SecPtErrCnt);
	}
				
	if (SecFltStatInfo.SecHdrTooShortCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SecHdrTooShortCnt:%u\n", SecFltStatInfo.SecHdrTooShortCnt);
	}
				
	if (SecFltStatInfo.SecDataNotFullCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SecDataNotFullCnt:%u\n", SecFltStatInfo.SecDataNotFullCnt);
	}
				
	if (SecFltStatInfo.SecDataTooShortCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SecDataTooShortCnt:%u\n", SecFltStatInfo.SecDataTooShortCnt);
	}
				
	if (SecFltStatInfo.SecMaskTooLongCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SecMaskTooLongCnt:%u\n", SecFltStatInfo.SecMaskTooLongCnt);		
	}
				
	if (SecFltStatInfo.SecMaskMismatchCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SecMaskMismatchCnt:%u\n", SecFltStatInfo.SecMaskMismatchCnt);
	}
				
	if (SecFltStatInfo.SecBufOverflowCnt != 0)
	{
		OLD_DMX_DBG_PRINT("SecBufOverflowCnt:%u\n", SecFltStatInfo.SecBufOverflowCnt);
	}
				
	return (0);
}

#if 0
static void PrintOldVideoStatInfoHld(struct Ali_DmxVideoStreamInfo* StreamInfo)
{
	struct Ali_DmxLibVideoStrmStatInfo VideoStatInfo;

	if (NULL == StreamInfo)
	{
		return;
	}	
	
	memset(&VideoStatInfo, 0, sizeof(VideoStatInfo));

	if (Ali_DmxVideoStreamErrStatGet(StreamInfo->Idx + ALI_DMX_STREAM_TYPE_VIDEO, &VideoStatInfo))
	{
		return;
	}	

	OLD_DMX_DBG_PRINT("\nVideo Statistics StrIdx:%u PID:%u (HLD Part)\n", 
		StreamInfo->Idx, StreamInfo->StreamParam.Pid);

	OLD_DMX_DBG_PRINT("-------------------------------------------------\n");

	if (VideoStatInfo.InvPathCnt != 0)
	{
		OLD_DMX_DBG_PRINT("InvPathCnt:%u\n", VideoStatInfo.InvPathCnt);
	}

	if (VideoStatInfo.IoOpenFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoOpenFailCnt:%u\n", VideoStatInfo.IoOpenFailCnt);
	}

	if (VideoStatInfo.IoCfgFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoCfgFailCnt:%u\n", VideoStatInfo.IoCfgFailCnt);
	}

	if (VideoStatInfo.IoStartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoStartFailCnt:%u\n", VideoStatInfo.IoStartFailCnt);
	}

	if (VideoStatInfo.IoStopFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoStopFailCnt:%u\n", VideoStatInfo.IoStopFailCnt);
	}
	
	return;
}

#endif

/********************************************************
* Usd to Print the Video Stream info
*********************************************************/
static INT32 PrintOldVideoStatInfoKernel(INT32 linux_fd, UINT32 pid, UINT32 SysTickInc)
{
	INT32								Ret;
	UINT32								Bitrate;
    UINT32								TsInCntInc;
	static UINT32						LastTsInCnt;
	struct Ali_DmxDrvTsFltStatInfo		TsFltStatInfo;
	struct Ali_DmxDrvVideoStrmStatInfo	VideoStrmStatInfo;


	memset(&VideoStrmStatInfo, 0, sizeof(VideoStrmStatInfo));
	
	memset(&TsFltStatInfo, 0, sizeof(TsFltStatInfo));

	Ret = ioctl(linux_fd, ALI_DMX_CHAN_VIDEO_INFO_GET, &VideoStrmStatInfo);

	if (Ret < 0)
	{
		OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}	


	Ret = ioctl(linux_fd, ALI_DMX_CHAN_VIDEO_FILTER_INFO_GET, &TsFltStatInfo);

	if (Ret < 0)
	{
		OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

    OLD_DMX_DBG_PRINT("\n Video Statistics. PID:%u (Kernel Part)\n", pid); 


	if (SysTickInc != 0 && TsFltStatInfo.TsInCnt > LastTsInCnt)
	{
		TsInCntInc = TsFltStatInfo.TsInCnt - LastTsInCnt;
		Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
		OLD_DMX_DBG_PRINT("Bitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
	}
	LastTsInCnt= TsFltStatInfo.TsInCnt;
	
	OLD_DMX_DBG_PRINT("----------------------------------------------------------\n");

	if (VideoStrmStatInfo.StatErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StatErrCnt:%u\n", VideoStrmStatInfo.StatErrCnt);
	}

	if (VideoStrmStatInfo.StrTypeErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StrTypeErrCnt:%u\n", VideoStrmStatInfo.StrTypeErrCnt);
	}

	if (VideoStrmStatInfo.CopyErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CopyErrCnt:%u\n", VideoStrmStatInfo.CopyErrCnt);
	}

	if (VideoStrmStatInfo.CfgFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CfgFailCnt:%u\n", VideoStrmStatInfo.CfgFailCnt);
	}

	if (VideoStrmStatInfo.StartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StartFailCnt:%u\n", VideoStrmStatInfo.StartFailCnt);
	}

	if (VideoStrmStatInfo.StopFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StopFailCnt:%u\n", VideoStrmStatInfo.StopFailCnt);
	}

	if (VideoStrmStatInfo.CloseFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CloseFailCnt:%u\n", VideoStrmStatInfo.CloseFailCnt);
	}

	OLD_DMX_DBG_PRINT("TsInCnt:%u\n", TsFltStatInfo.TsInCnt);

	OLD_DMX_DBG_PRINT("TsScrmbCnt:%u\n", TsFltStatInfo.TsScrmbCnt);

	if (TsFltStatInfo.TsSyncByteErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsSyncByteErrCnt:%u\n", TsFltStatInfo.TsSyncByteErrCnt);
	}
	
	if (TsFltStatInfo.TsErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsErrCnt:%u\n", TsFltStatInfo.TsErrCnt);
	}
	
	if (TsFltStatInfo.TsDupCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsDupCnt:%u\n", TsFltStatInfo.TsDupCnt);
	}
	
	if (TsFltStatInfo.TsLostCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsLostCnt:%u\n", TsFltStatInfo.TsLostCnt);
	}
	
	return (0);
}


static INT32 PrintOldVideoStatInfoSee(INT32 linux_fd, UINT32 pid, UINT32 SysTickInc)
{
	INT32							Ret;
	struct Ali_DmxSeePlyChStatInfo	PesStatInfo;


	memset(&PesStatInfo, 0, sizeof(PesStatInfo));
				
	Ret = ioctl(linux_fd, ALI_DMX_CHAN_VIDEO_SEE_INFO_GET, &PesStatInfo);

	if (Ret < 0)
	{
		OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	OLD_DMX_DBG_PRINT("\nVideo Statistics PID:%u (SEE Part)\n", pid);

	OLD_DMX_DBG_PRINT("------------------------------------\n");

	OLD_DMX_DBG_PRINT("TsInCnt:%u\n", PesStatInfo.TsInCnt);

	OLD_DMX_DBG_PRINT("PesHdrCnt:%u\n", PesStatInfo.PesHdrCnt);

	if (PesStatInfo.TsPlayBusyCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsPlayBusyCnt:%u\n", PesStatInfo.TsPlayBusyCnt);
	}

	if (PesStatInfo.TsScrmbCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsScrmbCnt:%u\n", PesStatInfo.TsScrmbCnt);
	}

	if (PesStatInfo.TsErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsErrCnt:%u\n", PesStatInfo.TsErrCnt);
	}

	if (PesStatInfo.PesTsErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesTsErrCnt:%u\n", PesStatInfo.PesTsErrCnt);
	}
			
	if (PesStatInfo.PesTsScrmbCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesTsScrmbCnt:%u\n", PesStatInfo.PesTsScrmbCnt);
	}
			
	if (PesStatInfo.PesTsDupCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesTsDupCnt:%u\n", PesStatInfo.PesTsDupCnt);
	}

	if (PesStatInfo.PesTsLostCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesTsLostCnt:%u\n", PesStatInfo.PesTsLostCnt);
	}

	if (PesStatInfo.PesHdrLenErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesHdrLenErrCnt:%u\n", PesStatInfo.PesHdrLenErrCnt);
	}

	if (PesStatInfo.PesHdrScErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesHdrScErrCnt:%u\n", PesStatInfo.PesHdrScErrCnt);
	}

	if (PesStatInfo.PesStreamIdErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesStreamIdErrCnt:%u\n", PesStatInfo.PesStreamIdErrCnt);
	}
			
	if (PesStatInfo.PesScrmbCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesScrmbCnt:%u\n", PesStatInfo.PesScrmbCnt);
	}

	if (PesStatInfo.PesHdrPayloadLenErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesHdrPayloadLenErr:%u\n", PesStatInfo.PesHdrPayloadLenErrCnt);
	}

	if (PesStatInfo.PesCallbackNobufCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesCallbackNobufCnt:%u\n", PesStatInfo.PesCallbackNobufCnt);
	}

	if (PesStatInfo.PesReqBufBusyCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesReqBufBusyCnt:%u\n", PesStatInfo.PesReqBufBusyCnt);
	}

	if (PesStatInfo.PesReqDecStateErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesReqDecStateErrCnt:%u\n", PesStatInfo.PesReqDecStateErrCnt);
	}

	if (PesStatInfo.PesTsNoPayloadCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesTsNoPayloadCnt:%u\n", PesStatInfo.PesTsNoPayloadCnt);
	}

	if (PesStatInfo.PesBufOverflowCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesBufOverflowCnt:%u\n", PesStatInfo.PesBufOverflowCnt);
	}

	OLD_DMX_DBG_PRINT("PesBufReqCnt:%u\n", PesStatInfo.PesBufReqCnt);

	OLD_DMX_DBG_PRINT("PesBufUpdateCnt:%u\n", PesStatInfo.PesBufUpdateCnt);

	return (0);
}



#if 0
static void PrintOldAudioStatInfoHld(struct Ali_DmxAudioStreamInfo* StreamInfo)
{
	struct Ali_DmxLibAudioStrmStatInfo AudioStatInfo;

	if (NULL == StreamInfo)
	{
		return;
	}	
	
	memset(&AudioStatInfo, 0, sizeof(AudioStatInfo));

	if (Ali_DmxAudioStreamErrStatGet(StreamInfo->Idx + ALI_DMX_STREAM_TYPE_AUDIO, &AudioStatInfo))
	{
		return;
	}
	
	OLD_DMX_DBG_PRINT("\nAudio Statistics StrIdx:%u PID:%u (HLD Part)\n", 
		StreamInfo->Idx, StreamInfo->StreamParam.Pid);

	OLD_DMX_DBG_PRINT("-------------------------------------------------\n");


	if (AudioStatInfo.InvPathCnt != 0)
	{
		OLD_DMX_DBG_PRINT("InvPathCnt:%u\n", AudioStatInfo.InvPathCnt);
	}

	if (AudioStatInfo.IoOpenFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoOpenFailCnt:%u\n", AudioStatInfo.IoOpenFailCnt);
	}

	if (AudioStatInfo.IoCfgFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoCfgFailCnt:%u\n", AudioStatInfo.IoCfgFailCnt);
	}

	if (AudioStatInfo.IoStartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoStartFailCnt:%u\n", AudioStatInfo.IoStartFailCnt);
	}

	if (AudioStatInfo.IoStopFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoStopFailCnt:%u\n", AudioStatInfo.IoStopFailCnt);
	}
	
	return;
}
#endif

/********************************************************
* Usd to Print the Audio Stream info
*********************************************************/
static INT32 PrintOldAudioStatInfoKernel(INT32 linux_fd, UINT32 pid, UINT32 SysTickInc)
{
	INT32								Ret;
	UINT32								Bitrate;
    UINT32								TsInCntInc;
	static UINT32						LastTsInCnt;
	struct Ali_DmxDrvTsFltStatInfo		TsFltStatInfo;
	struct Ali_DmxDrvAudioStrmStatInfo	AudioStrmStatInfo;
	
	memset(&AudioStrmStatInfo, 0, sizeof(AudioStrmStatInfo));

	memset(&TsFltStatInfo, 0, sizeof(TsFltStatInfo));

	Ret = ioctl(linux_fd, ALI_DMX_CHAN_AUDIO_INFO_GET, &AudioStrmStatInfo);

	if (Ret < 0)
	{
		OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	Ret = ioctl(linux_fd, ALI_DMX_CHAN_AUDIO_FILTER_INFO_GET, &TsFltStatInfo);

	if (Ret < 0)
	{
		OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

    OLD_DMX_DBG_PRINT("\n Audio Statistics. PID:%u (Kernel Part)\n", pid); 

	if (SysTickInc != 0 && TsFltStatInfo.TsInCnt > LastTsInCnt)
	{
		TsInCntInc = TsFltStatInfo.TsInCnt - LastTsInCnt;
		Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
		OLD_DMX_DBG_PRINT("Bitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
	}
	LastTsInCnt= TsFltStatInfo.TsInCnt;
	
	OLD_DMX_DBG_PRINT("----------------------------------------------------------\n");

	if (AudioStrmStatInfo.StatErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StatErrCnt:%u\n", AudioStrmStatInfo.StatErrCnt);
	}

	if (AudioStrmStatInfo.StrTypeErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StrTypeErrCnt:%u\n", AudioStrmStatInfo.StrTypeErrCnt);
	}

	if (AudioStrmStatInfo.CopyErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CopyErrCnt:%u\n", AudioStrmStatInfo.CopyErrCnt);
	}

	if (AudioStrmStatInfo.CfgFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CfgFailCnt:%u\n", AudioStrmStatInfo.CfgFailCnt);
	}

	if (AudioStrmStatInfo.StartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StartFailCnt:%u\n", AudioStrmStatInfo.StartFailCnt);
	}

	if (AudioStrmStatInfo.StopFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StopFailCnt:%u\n", AudioStrmStatInfo.StopFailCnt);
	}

	if (AudioStrmStatInfo.CloseFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CloseFailCnt:%u\n", AudioStrmStatInfo.CloseFailCnt);
	}

	OLD_DMX_DBG_PRINT("TsInCnt:%u\n", TsFltStatInfo.TsInCnt);

	OLD_DMX_DBG_PRINT("TsScrmbCnt:%u\n", TsFltStatInfo.TsScrmbCnt);

	if (TsFltStatInfo.TsSyncByteErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsSyncByteErrCnt:%u\n", TsFltStatInfo.TsSyncByteErrCnt);
	}
	if (TsFltStatInfo.TsErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsErrCnt:%u\n", TsFltStatInfo.TsErrCnt);
	}
	if (TsFltStatInfo.TsDupCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsDupCnt:%u\n", TsFltStatInfo.TsDupCnt);
	}
	if (TsFltStatInfo.TsLostCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsLostCnt:%u\n", TsFltStatInfo.TsLostCnt);
	}
			
	return (0);
}

//static INT32 PrintOldAudioStatInfoSee(struct Ali_DmxAudioStreamInfo* StreamInfo)
static INT32 PrintOldAudioStatInfoSee(INT32 linux_fd, UINT32 pid, UINT32 SysTickInc)
{
	INT32							Ret;
	struct Ali_DmxSeePlyChStatInfo	PesStatInfo;
	
	memset(&PesStatInfo, 0, sizeof(PesStatInfo));
				
	Ret = ioctl(linux_fd, ALI_DMX_CHAN_AUDIO_SEE_INFO_GET, &PesStatInfo);

	if (Ret < 0)
	{
		OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	OLD_DMX_DBG_PRINT("\nAudio Statistics PID:%u (SEE Part)\n", PesStatInfo.PlyChPid);

	OLD_DMX_DBG_PRINT("------------------------------------\n");

	OLD_DMX_DBG_PRINT("TsInCnt:%u\n", PesStatInfo.TsInCnt);

	OLD_DMX_DBG_PRINT("PesHdrCnt:%u\n", PesStatInfo.PesHdrCnt);

	if (PesStatInfo.TsPlayBusyCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsPlayBusyCnt:%u\n", PesStatInfo.TsPlayBusyCnt);
	}

	if (PesStatInfo.TsScrmbCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsScrmbCnt:%u\n", PesStatInfo.TsScrmbCnt);
	}

	if (PesStatInfo.TsErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsErrCnt:%u\n", PesStatInfo.TsErrCnt);
	}

	if (PesStatInfo.PesTsErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesTsErrCnt:%u\n", PesStatInfo.PesTsErrCnt);
	}
			
	if (PesStatInfo.PesTsScrmbCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesTsScrmbCnt:%u\n", PesStatInfo.PesTsScrmbCnt);
	}
			
	if (PesStatInfo.PesTsDupCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesTsDupCnt:%u\n", PesStatInfo.PesTsDupCnt);
	}

	if (PesStatInfo.PesTsLostCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesTsLostCnt:%u\n", PesStatInfo.PesTsLostCnt);
	}

	if (PesStatInfo.PesHdrLenErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesHdrLenErrCnt:%u\n", PesStatInfo.PesHdrLenErrCnt);
	}

	if (PesStatInfo.PesHdrScErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesHdrScErrCnt:%u\n", PesStatInfo.PesHdrScErrCnt);
	}

	if (PesStatInfo.PesStreamIdErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesStreamIdErrCnt:%u\n", PesStatInfo.PesStreamIdErrCnt);
	}
			
	if (PesStatInfo.PesScrmbCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesScrmbCnt:%u\n", PesStatInfo.PesScrmbCnt);
	}

	if (PesStatInfo.PesHdrPayloadLenErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesHdrPayloadLenErrCnt:%u\n", PesStatInfo.PesHdrPayloadLenErrCnt);
	}

	if (PesStatInfo.PesCallbackNobufCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesCallbackNobufCnt:%u\n", PesStatInfo.PesCallbackNobufCnt);
	}

	if (PesStatInfo.PesReqBufBusyCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesReqBufBusyCnt:%u\n", PesStatInfo.PesReqBufBusyCnt);
	}

	if (PesStatInfo.PesReqDecStateErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesReqDecStateErrCnt:%u\n", PesStatInfo.PesReqDecStateErrCnt);
	}

	if (PesStatInfo.PesTsNoPayloadCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesTsNoPayloadCnt:%u\n", PesStatInfo.PesTsNoPayloadCnt);
	}

	if (PesStatInfo.PesBufOverflowCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesBufOverflowCnt:%u\n", PesStatInfo.PesBufOverflowCnt);
	}

	OLD_DMX_DBG_PRINT("PesBufReqCnt:%u\n", PesStatInfo.PesBufReqCnt);

	OLD_DMX_DBG_PRINT("PesBufUpdateCnt:%u\n", PesStatInfo.PesBufUpdateCnt);

	return (0);
}

#if 0
static void PrintOldPcrStatInfoHld(struct Ali_DmxPcrStreamInfo* StreamInfo)
{
	struct Ali_DmxLibPcrStrmStatInfo PcrStatInfo;

	if (NULL == StreamInfo)
	{
		return;
	}	
	
	memset(&PcrStatInfo, 0, sizeof(PcrStatInfo));

	if (Ali_DmxPcrStreamErrStatGet(StreamInfo->Idx + ALI_DMX_STREAM_TYPE_PCR, &PcrStatInfo))
	{
		return;
	}

	OLD_DMX_DBG_PRINT("\nPCR Statistics StrIdx:%u PID:%u (HLD Part)\n", 
		StreamInfo->Idx, StreamInfo->StreamParam.Pid);

	OLD_DMX_DBG_PRINT("-------------------------------------------------\n");


	if (PcrStatInfo.InvPathCnt != 0)
	{
		OLD_DMX_DBG_PRINT("InvPathCnt:%u\n", PcrStatInfo.InvPathCnt);
	}

	if (PcrStatInfo.IoOpenFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoOpenFailCnt:%u\n", PcrStatInfo.IoOpenFailCnt);
	}

	if (PcrStatInfo.IoCfgFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoCfgFailCnt:%u\n", PcrStatInfo.IoCfgFailCnt);
	}

	if (PcrStatInfo.IoStartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoStartFailCnt:%u\n", PcrStatInfo.IoStartFailCnt);
	}

	if (PcrStatInfo.IoStopFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoStopFailCnt:%u\n", PcrStatInfo.IoStopFailCnt);
	}
	
	return;
}
#endif

/********************************************************
* Usd to Print the PCR Stream info
*********************************************************/
static INT32 PrintOldPcrStatInfoKernel(INT32 linux_fd, UINT32 pid, UINT32 SysTickInc)
{
	INT32								Ret;
	UINT32								Bitrate;
    UINT32								TsInCntInc;
	static UINT32						LastTsInCnt;	
	struct Ali_DmxDrvTsFltStatInfo		TsFltStatInfo;
	struct Ali_DmxDrvPcrStrmStatInfo	PcrStrmStatInfo;

	
	memset(&PcrStrmStatInfo, 0, sizeof(PcrStrmStatInfo));

	memset(&TsFltStatInfo, 0, sizeof(TsFltStatInfo));

	Ret = ioctl(linux_fd, ALI_DMX_CHAN_PCR_INFO_GET, &PcrStrmStatInfo);

	if (Ret < 0)
	{
		OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}
	
	Ret = ioctl(linux_fd, ALI_DMX_CHAN_PCR_FILTER_INFO_GET, &TsFltStatInfo);

	if (Ret < 0)
	{
		OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	OLD_DMX_DBG_PRINT("\nPCR Statistics. PID:%u (Kernel Part)\n", pid);

	if (SysTickInc != 0 && TsFltStatInfo.TsInCnt > LastTsInCnt)
	{
		TsInCntInc = TsFltStatInfo.TsInCnt - LastTsInCnt;
		Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
		OLD_DMX_DBG_PRINT("Bitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
	}
	LastTsInCnt= TsFltStatInfo.TsInCnt;
	
	OLD_DMX_DBG_PRINT("------------------------------------------------------------\n");

	if (PcrStrmStatInfo.StatErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StatErrCnt:%u\n", PcrStrmStatInfo.StatErrCnt);
	}

	if (PcrStrmStatInfo.StrTypeErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StrTypeErrCnt:%u\n", PcrStrmStatInfo.StrTypeErrCnt);
	}

	if (PcrStrmStatInfo.RdByteErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("RdByteErrCnt:%u\n", PcrStrmStatInfo.RdByteErrCnt);
	}

	if (PcrStrmStatInfo.RdBufShortCnt != 0)
	{
		OLD_DMX_DBG_PRINT("RdBufShortCnt:%u\n", PcrStrmStatInfo.RdBufShortCnt);
	}
	
	if (PcrStrmStatInfo.CopyErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CopyErrCnt:%u\n", PcrStrmStatInfo.CopyErrCnt);
	}

	if (PcrStrmStatInfo.CfgFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CfgFailCnt:%u\n", PcrStrmStatInfo.CfgFailCnt);
	}

	if (PcrStrmStatInfo.StartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StartFailCnt:%u\n", PcrStrmStatInfo.StartFailCnt);
	}

	if (PcrStrmStatInfo.StopFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StopFailCnt:%u\n", PcrStrmStatInfo.StopFailCnt);
	}

	if (PcrStrmStatInfo.CloseFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CloseFailCnt:%u\n", PcrStrmStatInfo.CloseFailCnt);
	}

	OLD_DMX_DBG_PRINT("TsInCnt:%u\n", TsFltStatInfo.TsInCnt);

	OLD_DMX_DBG_PRINT("TsScrmbCnt:%u\n", TsFltStatInfo.TsScrmbCnt);

	if (TsFltStatInfo.TsSyncByteErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsSyncByteErrCnt:%u\n", TsFltStatInfo.TsSyncByteErrCnt);
	}
	
	if (TsFltStatInfo.TsErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsErrCnt:%u\n", TsFltStatInfo.TsErrCnt);
	}
	
	if (TsFltStatInfo.TsDupCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsDupCnt:%u\n", TsFltStatInfo.TsDupCnt);
	}

	if (TsFltStatInfo.TsLostCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsLostCnt:%u\n", TsFltStatInfo.TsLostCnt);
	}

	return (0);
}

static void PrintOldPesStatInfoHld(struct dmx_hld_porting_pes_slot *pes_slot)
{
	struct Ali_DmxSlotPesStatInfo *stat_info;

	if (NULL == pes_slot)
	{
		return;
	}	
	
	stat_info = &pes_slot->stat_info;

	OLD_DMX_DBG_PRINT("\nPES slot statistics PID:%u (HLD Part)\n", pes_slot->pid);

	OLD_DMX_DBG_PRINT("-------------------------------------------------\n");

	OLD_DMX_DBG_PRINT("PesBufReqCnt:%u\n", stat_info->PesBufReqCnt);

	OLD_DMX_DBG_PRINT("PesBufRetCnt:%u\n", stat_info->PesBufRetCnt);
	
	if (stat_info->IoOpenFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoOpenFailCnt:%u\n", stat_info->IoOpenFailCnt);
	}

	if (stat_info->IoStartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("IoStartFailCnt:%u\n", stat_info->IoStartFailCnt);
	}

	if (stat_info->PesHdrErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesHdrErrCnt:%u\n", stat_info->PesHdrErrCnt);
	}
	
	if (stat_info->PesBufEmptyCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesBufEmptyCnt:%u\n", stat_info->PesBufEmptyCnt);
	}

	if (stat_info->PesReqFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesReqFailCnt:%u\n", stat_info->PesReqFailCnt);
	}

	return;
}

static INT32 PrintOldPesStatInfoKernel(INT32 linux_fd, UINT32 pid, UINT32 SysTickInc)
{
	INT32								Ret;
	UINT32								Bitrate;
    UINT32								TsInCntInc;
	static UINT32						LastTsInCnt;
    
	struct Ali_DmxDrvTsFltStatInfo		TsFltStatInfo;
	struct Ali_DmxDrvPesStrmStatInfo	PesStrmStatInfo;
	struct Ali_DmxDrvPesFltStatInfo		PesFltStatInfo;

	memset(&TsFltStatInfo, 0, sizeof(TsFltStatInfo));

	memset(&PesStrmStatInfo, 0, sizeof(PesStrmStatInfo));

	memset(&PesFltStatInfo, 0, sizeof(PesFltStatInfo));

	Ret = ioctl(linux_fd, ALI_DMX_CHAN_PES_TS_FILTER_INFO_GET, &TsFltStatInfo);

	if (Ret < 0)
	{
		OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	Ret = ioctl(linux_fd, ALI_DMX_CHAN_PES_INFO_GET, &PesStrmStatInfo);

	if (Ret < 0)
	{
		OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}	

	Ret = ioctl(linux_fd, ALI_DMX_CHAN_PES_FILTER_INFO_GET, &PesFltStatInfo);

	if (Ret < 0)
	{
		OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	OLD_DMX_DBG_PRINT("\n Pestion Statistics.   PID:%u (Kernel Part)\n", pid); 

	if (SysTickInc != 0 && TsFltStatInfo.TsInCnt > LastTsInCnt)
	{
		TsInCntInc = TsFltStatInfo.TsInCnt - LastTsInCnt;
		Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
		OLD_DMX_DBG_PRINT("Bitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
	}
	LastTsInCnt = TsFltStatInfo.TsInCnt;

	OLD_DMX_DBG_PRINT("--------------------------------------------------------------------------------\n");

	if (PesStrmStatInfo.CbTypeErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CbTypeErrCnt:%u\n", PesStrmStatInfo.CbTypeErrCnt);
	}

	if (PesStrmStatInfo.StrTypeErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StrTypeErrCnt:%u\n", PesStrmStatInfo.StrTypeErrCnt);
	}

	if (PesStrmStatInfo.StatErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StatErrCnt:%u\n", PesStrmStatInfo.StatErrCnt);
	}

	if (PesStrmStatInfo.WrByteErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("WrByteErrCnt:%u\n", PesStrmStatInfo.WrByteErrCnt);
	}

	if (PesStrmStatInfo.RdBufShortCnt != 0)
	{
		OLD_DMX_DBG_PRINT("RdBufShortCnt:%u\n", PesStrmStatInfo.RdBufShortCnt);
	}

	if (PesStrmStatInfo.CopyErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CopyErrCnt:%u\n", PesStrmStatInfo.CopyErrCnt);
	}

	if (PesStrmStatInfo.CfgFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CfgFailCnt:%u\n", PesStrmStatInfo.CfgFailCnt);
	}

	if (PesStrmStatInfo.StartFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StartFailCnt:%u\n", PesStrmStatInfo.StartFailCnt);
	}

	if (PesStrmStatInfo.StopFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("StopFailCnt:%u\n", PesStrmStatInfo.StopFailCnt);
	}

	if (PesStrmStatInfo.CloseFailCnt != 0)
	{
		OLD_DMX_DBG_PRINT("CloseFailCnt:%u\n", PesStrmStatInfo.CloseFailCnt);
	}

	OLD_DMX_DBG_PRINT("TsInCnt:%u\n", TsFltStatInfo.TsInCnt);

	OLD_DMX_DBG_PRINT("TsScrmbCnt:%u\n", TsFltStatInfo.TsScrmbCnt);

	if (TsFltStatInfo.TsSyncByteErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsSyncByteErrCnt:%u\n", TsFltStatInfo.TsSyncByteErrCnt);
	}
	
	if (TsFltStatInfo.TsErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsErrCnt:%u\n", TsFltStatInfo.TsErrCnt);
	}

	if (TsFltStatInfo.TsDupCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsDupCnt:%u\n", TsFltStatInfo.TsDupCnt);
	}

	if (TsFltStatInfo.TsLostCnt != 0)
	{
		OLD_DMX_DBG_PRINT("TsLostCnt:%u\n", TsFltStatInfo.TsLostCnt);
	}

	OLD_DMX_DBG_PRINT("PesInCnt:%u\n", PesFltStatInfo.PesInCnt);

	OLD_DMX_DBG_PRINT("PesOutCnt:%u\n", PesFltStatInfo.PesOutCnt);

	if (PesFltStatInfo.PesTsNoPayloadCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesTsNoPayloadCnt:%u\n", PesFltStatInfo.PesTsNoPayloadCnt);
	}

	if (PesFltStatInfo.PesTsScrmbCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesTsScrmbCnt:%u\n", PesFltStatInfo.PesTsScrmbCnt);
	}
				
	if (PesFltStatInfo.PesTsLostCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesTsLostCnt:%u\n", PesFltStatInfo.PesTsLostCnt);
	}
				
	if (PesFltStatInfo.PesTsDupCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesTsDupCnt:%u\n", PesFltStatInfo.PesTsDupCnt);
	}
				
	if (PesFltStatInfo.PesHdrErrCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesHdrErrCnt:%u\n", PesFltStatInfo.PesHdrErrCnt);
	}

	if (PesFltStatInfo.PesBufOverflowCnt != 0)
	{
		OLD_DMX_DBG_PRINT("PesBufOverflowCnt:%u\n", PesFltStatInfo.PesBufOverflowCnt);
	}
				
	return (0);
}

#if 0
static INT32 ShowOldTsStreamInfo(struct Ali_DmxTsStreamInfo* StreamInfo, INT32 FltIdx)
{
	if (NULL == StreamInfo)
	{
		return (-1);
	}

	OLD_DMX_DBG_PRINT("\nTS Stream Info");
	if (StreamInfo->StreamParam.NeedDiscramble)
		OLD_DMX_DBG_PRINT(" (Scrambled)");
	OLD_DMX_DBG_PRINT(":\n--------------------------\n");
	OLD_DMX_DBG_PRINT("PID: %u\n", StreamInfo->StreamParam.PidList[FltIdx]);
	OLD_DMX_DBG_PRINT("Dmx Id: %u\n", StreamInfo->DmxId);
	OLD_DMX_DBG_PRINT("File descriptor: %d\n", StreamInfo->Fd);
	OLD_DMX_DBG_PRINT("Stream Id: 0x%x\n", Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_TS, StreamInfo->Idx));
	OLD_DMX_DBG_PRINT("Stream Idx: %u\n", StreamInfo->Idx);
	OLD_DMX_DBG_PRINT("Filter Id: 0x%x\n", StreamInfo->StreamParam.TsFltId[FltIdx]);

	return (0);
}

static INT32 ShowOldVideoStreamInfo(struct Ali_DmxVideoStreamInfo* StreamInfo)
{
	if (NULL == StreamInfo)
	{
		return (-1);
	}

	OLD_DMX_DBG_PRINT("\nVideo Stream Info");
	if (StreamInfo->StreamParam.NeedDiscramble)
		OLD_DMX_DBG_PRINT(" (Scrambled)");
	OLD_DMX_DBG_PRINT(":\n--------------------------\n");
	OLD_DMX_DBG_PRINT("PID: %u\n", StreamInfo->StreamParam.Pid);
	OLD_DMX_DBG_PRINT("Dmx Id: %u\n", StreamInfo->DmxId);
	OLD_DMX_DBG_PRINT("File descriptor: %d\n", StreamInfo->Fd);
	OLD_DMX_DBG_PRINT("Stream Id: 0x%x\n", Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_VIDEO, StreamInfo->Idx));
	OLD_DMX_DBG_PRINT("Stream Idx: %u\n", StreamInfo->Idx);	

	return (0);
}

static INT32 ShowOldAudioStreamInfo(struct Ali_DmxAudioStreamInfo* StreamInfo)
{
	if (NULL == StreamInfo)
	{
		return (-1);
	}

	OLD_DMX_DBG_PRINT("\nAudio Stream Info");
	if (StreamInfo->StreamParam.NeedDiscramble)
		OLD_DMX_DBG_PRINT(" (Scrambled)");
	OLD_DMX_DBG_PRINT(":\n--------------------------\n");
	OLD_DMX_DBG_PRINT("PID: %u\n", StreamInfo->StreamParam.Pid);
	OLD_DMX_DBG_PRINT("Dmx Id: %u\n", StreamInfo->DmxId);
	OLD_DMX_DBG_PRINT("File descriptor: %d\n", StreamInfo->Fd);
	OLD_DMX_DBG_PRINT("Stream Id: 0x%x\n", Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_AUDIO, StreamInfo->Idx));
	OLD_DMX_DBG_PRINT("Stream Idx: %u\n", StreamInfo->Idx);

	return (0);
}

static INT32 ShowOldPcrStreamInfo(struct Ali_DmxPcrStreamInfo* StreamInfo)
{
	if (NULL == StreamInfo)
	{
		return (-1);
	}
	
	OLD_DMX_DBG_PRINT("\nPCR Stream Info");
	if (StreamInfo->StreamParam.NeedDiscramble)
		OLD_DMX_DBG_PRINT(" (Scrambled)");
	OLD_DMX_DBG_PRINT(":\n--------------------------\n");
	OLD_DMX_DBG_PRINT("PID: %u\n", StreamInfo->StreamParam.Pid);
	OLD_DMX_DBG_PRINT("Dmx Id: %u\n", StreamInfo->DmxId);
	OLD_DMX_DBG_PRINT("File descriptor: %d\n", StreamInfo->Fd);
	OLD_DMX_DBG_PRINT("Stream Id: 0x%x\n", Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_PCR, StreamInfo->Idx));
	OLD_DMX_DBG_PRINT("Stream Idx: %u\n", StreamInfo->Idx);	

	return (0);
}

static INT32 ShowOldSectionStreamInfo(struct Ali_DmxSecStreamInfo* StreamInfo)
{
	UINT8 i;
	
	if (NULL == StreamInfo)
	{
		return (-1);
	}

	OLD_DMX_DBG_PRINT("\nSection Stream Info");
	if (1 == StreamInfo->StreamParam.SecMask.Flags)
		OLD_DMX_DBG_PRINT(" (CRC)");	
	if (StreamInfo->StreamParam.NeedDiscramble)
		OLD_DMX_DBG_PRINT(" (Scrambled)");	
	OLD_DMX_DBG_PRINT(":\n----------------------------------\n");
	OLD_DMX_DBG_PRINT("PID: %u\n", StreamInfo->StreamParam.Pid);
	OLD_DMX_DBG_PRINT("Dmx Id: %u\n", StreamInfo->DmxId);
	OLD_DMX_DBG_PRINT("File descriptor: %d\n", StreamInfo->Fd);
	OLD_DMX_DBG_PRINT("Stream id: 0x%x\n", Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_SEC, StreamInfo->Idx));
	OLD_DMX_DBG_PRINT("Stream index: %u\n", StreamInfo->Idx);
	OLD_DMX_DBG_PRINT("Total byte in: %u\n", StreamInfo->InByteCnt);
	OLD_DMX_DBG_PRINT("Mask length: %u\n", StreamInfo->StreamParam.SecMask.MatchLen);
	OLD_DMX_DBG_PRINT("Mask:");
	for (i = 0; i < StreamInfo->StreamParam.SecMask.MatchLen; i++)
		OLD_DMX_DBG_PRINT(" %02x", StreamInfo->StreamParam.SecMask.Mask[i]);
	OLD_DMX_DBG_PRINT("\nMatch:");
	for (i = 0; i < StreamInfo->StreamParam.SecMask.MatchLen; i++)
		OLD_DMX_DBG_PRINT(" %02x", StreamInfo->StreamParam.SecMask.Match[i]);
	OLD_DMX_DBG_PRINT("\nNegate:");
	for (i = 0; i < StreamInfo->StreamParam.SecMask.MatchLen; i++)
		OLD_DMX_DBG_PRINT(" %02x", StreamInfo->StreamParam.SecMask.Negate[i]);
	OLD_DMX_DBG_PRINT("\n");

	return (0);
}

#endif

/******************************************************************
* Add for show channel info in the old dmx hld driver
*******************************************************************/
static INT32 ShowOldChanInfo(UINT32 dmx_id,UINT32 pid, INT32 linux_fd, DMX_CHAN_TYPE chan_type)
{

	if(DMX_CHAN_VIDEO_TYPE == chan_type)
    {
		OLD_DMX_DBG_PRINT("\nVideo Stream Info:"); 
    }
    else if(DMX_CHAN_AUDIO_TYPE == chan_type)
    {
		OLD_DMX_DBG_PRINT("\nAudio Stream Info:"); 
    }
    else if(DMX_CHAN_REC_TYPE == chan_type)
    {
		OLD_DMX_DBG_PRINT("\nTS(Rec) Stream Info:");    
    }
    else if(DMX_CHAN_SEC_TYPE == chan_type)
    {
		OLD_DMX_DBG_PRINT("\nSec Stream Info:"); 
    }
    else if(DMX_CHAN_PCR_TYPE == chan_type)
    {
        OLD_DMX_DBG_PRINT("\nPCR Stream Info:"); 
    }
    else if(DMX_CHAN_PES_TYPE == chan_type)
    {
        OLD_DMX_DBG_PRINT("\nPES Stream Info:"); 
	}

	OLD_DMX_DBG_PRINT(":\n--------------------------\n");
	OLD_DMX_DBG_PRINT("PID: %u\n", pid);
	OLD_DMX_DBG_PRINT("Dmx Id: %u\n", dmx_id);
	OLD_DMX_DBG_PRINT("File descriptor: %d\n",linux_fd);    
        
	return (0);
}



static INT32 DmxOldDbgShowHwRegInfo(void)
{
	INT32	Ret;
	UINT32	HwRegTable[18][5];
	UINT8	i, j;

	if (DmxOldDbgCtrl.HwRegFd < 0)
	{
	    OLD_DMX_DBG_PRINT("Invalid DeMux debug FD: %d\n", DmxOldDbgCtrl.HwRegFd);

		return (-1);
	}
	
	if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
	{
		memset(&HwRegTable, 0, sizeof(HwRegTable));

		Ret = ioctl(DmxOldDbgCtrl.HwRegFd, ALI_DMX_CHAN_HW_REG_INFO_GET, &HwRegTable);

	    if (Ret < 0)
	    {
	        OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

			return (-1);
	    }

		OLD_DMX_DBG_PRINT("\nHardware register dump:\n");

		OLD_DMX_DBG_PRINT("------------------------------------\n");
		
		for (i = 0; i < 18; i++)
		{
			if (7 == i || 9 == i || 10 == i || 11 == i)
			{
				OLD_DMX_DBG_PRINT("\t...\n");
			}
			
			OLD_DMX_DBG_PRINT("0x%08x:", HwRegTable[i][0]);

			for (j = 0; j < 4; j++)
			{
				OLD_DMX_DBG_PRINT(" %08x", HwRegTable[i][j + 1]);			
			}

			OLD_DMX_DBG_PRINT("\n");
		}
	}
	
	return (0);
}

static INT32 DmxOldDbgShowKernelGlobalInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{
	if (DMX_DBG_SHOW_STATISTICS == info_type)
	{
		PrintOldKernelGlobalStatInfo(SysTickInc);		
	}

	return (0);
}

static INT32 DmxOldDbgShowSeeGlobalInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{
	if (DMX_DBG_SHOW_STATISTICS == info_type)
	{
		PrintOldSeeGlobalStatInfo(SysTickInc);
	}

	return (0);
}


/********************************************************
* Usd to Print the Record Stream info
*********************************************************/

static INT32 DmxOldDbgShowTsInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{

    INT32						StreamExist;
    UINT32 						DmxId;
    UINT32						RecSlotId;
    UINT32						PidListId;
    UINT32						pid, len;
    INT32 						linux_fd;
    
    StreamExist = 0;

	for(DmxId=0; DmxId<DMX_DEV_MAX_NUM; DmxId++)
    {        
		if( DMX_HLD_PORTING_STATUS_RUN == g_dmx_hld_porting[DmxId].status )
        {
			for(RecSlotId = 0; RecSlotId < DMX_HLD_PORTING_REC_SLOT_CNT; RecSlotId++)
			{
				linux_fd = g_dmx_hld_porting[DmxId].rec_slot[RecSlotId].linux_fd;

				if(linux_fd > 0)
				{	
					len = g_dmx_hld_porting[DmxId].rec_slot[RecSlotId].user_pid_list_len;
              
				    for(PidListId = 0; PidListId < len; PidListId++)
				    {
						pid = g_dmx_hld_porting[DmxId].rec_slot[RecSlotId].user_pid_list[PidListId];

                        if (DMX_DBG_SHOW_STREAM_INFO & info_type)
                        {
                            ShowOldChanInfo(DmxId, pid, linux_fd, DMX_CHAN_REC_TYPE);
                        }
                        else if (DMX_DBG_SHOW_STATISTICS == info_type)
                        {
                            if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
                            {
//                                PrintOldTsStatInfoHld(&StreamInfo, FltIdx);
								OLD_DMX_DBG_PRINT(" Add the function later!!!! \n");
                            }
                            
                            if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
                            {
								PrintOldTsStatInfoKernel(linux_fd, pid, PidListId,SysTickInc);
                            }
                        }
				    }

					StreamExist = 1;
				}							
			}
        }
    }
	
	if (!StreamExist)
	{
		OLD_DMX_DBG_PRINT("\nNo TS stream opened in DeMux!\n");
	}
	
	return (0);
}

static INT32 DmxOldDbgShowVideoInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{
    INT32						StreamExist;
    UINT32 						DmxId;
    UINT32						pid;
    INT32 						linux_fd;
       
    StreamExist = 0;

	for(DmxId=0; DmxId<DMX_DEV_MAX_NUM; DmxId++)
    {
        
		if( DMX_HLD_PORTING_STATUS_RUN == g_dmx_hld_porting[DmxId].status
           && DMX_HLD_PORTING_AV_STREAM_RUN == g_dmx_hld_porting[DmxId].av_stream_status )
        {
			linux_fd = g_dmx_hld_porting[DmxId].v_slot.linux_fd;

			if(linux_fd > 0)
			{	
                pid = g_dmx_hld_porting[DmxId].v_slot.pid ;

				if (DMX_DBG_SHOW_STREAM_INFO & info_type)
				{
				    ShowOldChanInfo(DmxId, pid, linux_fd, DMX_CHAN_VIDEO_TYPE);
				}
				else if (DMX_DBG_SHOW_STATISTICS == info_type)
				{
					if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
					{
						OLD_DMX_DBG_PRINT(" Add the function later!!!! \n");
					}

					if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
					{
						PrintOldVideoStatInfoKernel(linux_fd, pid, SysTickInc);
					}

					if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_SEE)
					{
						PrintOldVideoStatInfoSee(linux_fd, pid, SysTickInc);
					}
				}

				StreamExist = 1;
			}
        }
    }
    
	if (!StreamExist)
	{
		OLD_DMX_DBG_PRINT("\nNo Video stream opened in DeMux!\n");
	}
	
	return (0);
}

#if 0
INT32 DmxOldDbgShowAudioInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{	
	INT32							StreamExist;
	struct ALi_DmxSysInfo			DmxSysInfo;
	struct Ali_DmxAudioStreamInfo	StreamInfo;

	memset(&DmxSysInfo, 0, sizeof(DmxSysInfo));
		
	if (ALi_DmxSysInfoGet(&DmxSysInfo))
	{
	    OLD_DMX_DBG_PRINT("DeMux library not yet initialized!\n");

		return (-1);
	}

	memset(&StreamInfo, 0, sizeof(StreamInfo));

	if (Ali_DmxAudioStreamInfoGet(ALI_DMX_STREAM_ID_AUDIO_START, &StreamInfo))
	{
	    OLD_DMX_DBG_PRINT("\nNo audio stream in dmx library!\n");

		return (-1);
	}
	
	StreamExist = 0;

    if (StreamInfo.Fd > 0 && ALI_DMX_STREAM_STATE_RUN == StreamInfo.State)
    {
   		if (DMX_DBG_SHOW_STREAM_INFO & info_type)
   		{
   			ShowOldAudioStreamInfo(&StreamInfo);
   		}
		else if (DMX_DBG_SHOW_STATISTICS == info_type)
		{    
			if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
			{
				PrintOldAudioStatInfoHld(&StreamInfo);
	    	}
			
			if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
			{
				PrintOldAudioStatInfoKernel(&StreamInfo, SysTickInc);
			}

			if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_SEE)
			{		
				PrintOldAudioStatInfoSee(&StreamInfo);
			}
		}

		StreamExist = 1;
	}

	if (!StreamExist)
	{
		OLD_DMX_DBG_PRINT("\nNo Audio stream opened in DeMux!\n");
	}
	
	return (0);
}
#endif

static INT32 DmxOldDbgShowAudioInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{

    INT32						StreamExist;
    UINT32 						DmxId;
    UINT32						pid;
    INT32 						linux_fd;
       
    StreamExist = 0;

	for(DmxId=0; DmxId<DMX_DEV_MAX_NUM; DmxId++)
    {        
		if( DMX_HLD_PORTING_STATUS_RUN == g_dmx_hld_porting[DmxId].status
           && DMX_HLD_PORTING_AV_STREAM_RUN == g_dmx_hld_porting[DmxId].av_stream_status )
        {
			linux_fd = g_dmx_hld_porting[DmxId].a_slot.linux_fd;

			if(linux_fd > 0)
			{	

                pid = g_dmx_hld_porting[DmxId].a_slot.pid ;

				if (DMX_DBG_SHOW_STREAM_INFO & info_type)
				{
				    ShowOldChanInfo(DmxId, pid, linux_fd, DMX_CHAN_AUDIO_TYPE);
				}
				else if (DMX_DBG_SHOW_STATISTICS == info_type)
				{
					if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
					{
//						PrintOldAudioStatInfoHld(&StreamInfo);
                        OLD_DMX_DBG_PRINT(" Add the function later!!!! \n");
					}

					if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
					{
						PrintOldAudioStatInfoKernel(linux_fd, pid, SysTickInc);
					}

					if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_SEE)
					{
						PrintOldAudioStatInfoSee(linux_fd, pid, SysTickInc);
					}
				}

				StreamExist = 1;
			}
        }
    }
    
	if (!StreamExist)
	{
		OLD_DMX_DBG_PRINT("\nNo Audio stream opened in DeMux!\n");
	}
	
	return (0);
}


static INT32 DmxOldDbgShowPcrInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{
	INT32						StreamExist;
    UINT32 						DmxId;
    UINT32						pid;
    INT32 						linux_fd;

	StreamExist = 0;

	for(DmxId=0; DmxId<DMX_DEV_MAX_NUM; DmxId++)
	{	    
	    if( DMX_HLD_PORTING_STATUS_RUN == g_dmx_hld_porting[DmxId].status
	       && DMX_HLD_PORTING_AV_STREAM_RUN == g_dmx_hld_porting[DmxId].av_stream_status )
	    {
	        linux_fd = g_dmx_hld_porting[DmxId].p_slot.linux_fd;

	        if(linux_fd > 0)
	        {   
	            pid = g_dmx_hld_porting[DmxId].p_slot.pid ;

	            if (DMX_DBG_SHOW_STREAM_INFO & info_type)
	            {
	                ShowOldChanInfo(DmxId, pid, linux_fd, DMX_CHAN_PCR_TYPE);
	            }
	            else if (DMX_DBG_SHOW_STATISTICS == info_type)
	            {
	                if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
	                {
	                    OLD_DMX_DBG_PRINT(" Add the function later!!!! \n");
	                }

	                if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
	                {
	                    PrintOldPcrStatInfoKernel(linux_fd, pid, SysTickInc);
	                }
	            }

	            StreamExist = 1;
	        }
	    }
	}

	if (!StreamExist)
	{
		OLD_DMX_DBG_PRINT("\nNo PCR stream opened in DeMux!\n");
	}
	
	return (0);
}

static INT32 DmxOldDbgShowSectionInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{
    
    INT32							StreamExist;
    UINT32 							DmxId;
    UINT32							SecSlotId;
    UINT32 							SecSlotStatus;
    UINT32							pid;
    INT32 							linux_fd;
    
    struct dmx_hld_porting_sec_slot *sec_slot_info;
    
    StreamExist = 0;

	for(DmxId = 0; DmxId < DMX_DEV_MAX_NUM; DmxId++)
    {
		if( DMX_HLD_PORTING_STATUS_RUN == g_dmx_hld_porting[DmxId].status )
        {
			for(SecSlotId = 0; SecSlotId < DMX_HLD_PORTING_SEC_SLOT_CNT; SecSlotId++)
			{
				linux_fd = g_dmx_hld_porting[DmxId].sec_slot[SecSlotId].linux_fd;

                SecSlotStatus = g_dmx_hld_porting[DmxId].sec_slot[SecSlotId].status;

				if((linux_fd > 0)&&(DMX_HLD_PORTING_SEC_SLOT_RUN == SecSlotStatus))
				{
					pid = ((struct get_section_param *)(g_dmx_hld_porting[DmxId].sec_slot[SecSlotId].sec_para))->pid;
				
                    if (DMX_DBG_SHOW_STREAM_INFO & info_type)
                    {
						ShowOldChanInfo(DmxId, pid, linux_fd, DMX_CHAN_SEC_TYPE);
                    }
                    else if (DMX_DBG_SHOW_STATISTICS == info_type)
                    {       
						sec_slot_info =  &g_dmx_hld_porting[DmxId].sec_slot[SecSlotId];
                        
                        if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
                        {        
							PrintOldSecStrmStatInfoHld(sec_slot_info);
                        }
                    
                        if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
                        {        
							PrintOldSectionStatInfoKernel(linux_fd, pid, SysTickInc);
                        }
                    }
              
					StreamExist = 1;
				}							
			}
        }
    }
	
	if (!StreamExist)
	{
		OLD_DMX_DBG_PRINT("\nNo section stream opened in DeMux!\n");
	}
	
	return (0);
}

static INT32 DmxOldDbgShowPesInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{
    
    INT32							StreamExist;
    UINT32 							DmxId;
    UINT32							PesSlotId;
    UINT32 							PesSlotStatus;
    UINT32							pid;
    INT32 							linux_fd;
    
    struct dmx_hld_porting_pes_slot *pes_slot_info;
    
    StreamExist = 0;

	for(DmxId = 0; DmxId < DMX_DEV_MAX_NUM; DmxId++)
    {
		if( DMX_HLD_PORTING_STATUS_RUN == g_dmx_hld_porting[DmxId].status)
        {
			for(PesSlotId = 0; PesSlotId < DMX_HLD_PORTING_PES_SLOT_CNT; PesSlotId++)
			{
				linux_fd = g_dmx_hld_porting[DmxId].pes_slot[PesSlotId].linux_fd;

                PesSlotStatus = g_dmx_hld_porting[DmxId].pes_slot[PesSlotId].status;

				if((linux_fd > 0)&&(DMX_HLD_PORTING_SEC_SLOT_RUN == PesSlotStatus))
				{
					pid = g_dmx_hld_porting[DmxId].pes_slot[PesSlotId].pid;
				
                    if (DMX_DBG_SHOW_STREAM_INFO & info_type)
                    {
						ShowOldChanInfo(DmxId, pid, linux_fd, DMX_CHAN_PES_TYPE);
                    }
                    else if (DMX_DBG_SHOW_STATISTICS == info_type)
                    {       
						pes_slot_info =  &g_dmx_hld_porting[DmxId].pes_slot[PesSlotId];
                        
                        if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
                        {        
							PrintOldPesStatInfoHld(pes_slot_info);
                        }
                    
                        if (DmxOldDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
                        { 
							PrintOldPesStatInfoKernel(linux_fd, pid, SysTickInc);
                        }
                    }
              
					StreamExist = 1;
				}							
			}
        }
    }
	
	if (!StreamExist)
	{
		OLD_DMX_DBG_PRINT("\nNo PES stream opened in DeMux!\n");
	}
	
	return (0);
}

static void dmx_old_dbg_task(UINT32 param1, UINT32 param2)
{
    UINT32 CurShowSysTickCnt;
    UINT32 PrevShowSysTickCnt = 0;
    UINT32 SysTickInc;

	for (;;)
	{
	    CurShowSysTickCnt = osal_get_tick();
	    SysTickInc = CurShowSysTickCnt - PrevShowSysTickCnt;
	    PrevShowSysTickCnt = CurShowSysTickCnt;
	
		if (DmxOldDbgCtrl.DmxDbgStatEn)
		{
			if (DmxOldDbgCtrl.HwRegDumpEn)
			{
				DmxOldDbgShowHwRegInfo();
			}

			if (DmxOldDbgCtrl.KernGlbStatEn)
			{
				DmxOldDbgShowKernelGlobalInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}
			
			if (DmxOldDbgCtrl.SeeGlbStatEn)
			{
				DmxOldDbgShowSeeGlobalInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}

			if (DmxOldDbgCtrl.VidStatEn)
			{
				DmxOldDbgShowVideoInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}

			if (DmxOldDbgCtrl.AudStatEn)
			{
				DmxOldDbgShowAudioInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}

			if (DmxOldDbgCtrl.PcrStatEn)
			{
				DmxOldDbgShowPcrInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}

			if (DmxOldDbgCtrl.SecStatEn)
			{
				DmxOldDbgShowSectionInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}

			if (DmxOldDbgCtrl.PesStatEn)
			{
				DmxOldDbgShowPesInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}
			
			if (DmxOldDbgCtrl.TsStatEn)
			{
				DmxOldDbgShowTsInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}
			
			if (DmxOldDbgCtrl.DbgShowTms > 0 && 0 == --DmxOldDbgCtrl.DbgShowTms)
			{
				OLD_DMX_DBG_PRINT("\nTimes up! End of printout.\n");
				DmxOldDbgCtrl.DmxDbgStatEn = FALSE;
			}			
		}

		osal_task_sleep(DmxOldDbgCtrl.DbgShowIntv);
	}

	return;
}


static RET_CODE dmx_old_dbg_task_Init()
{
	OSAL_T_CTSK t_ctsk;	

	memset(&t_ctsk, 0, sizeof(OSAL_T_CTSK ));

	t_ctsk.task = (FP)dmx_old_dbg_task;
	t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 10;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.name[0] = 'D';
	t_ctsk.name[1] = 'D';
	t_ctsk.name[2] = 'T';
	DmxOldDbgCtrl.TaskId = osal_task_create(&t_ctsk);
	if (OSAL_INVALID_ID == DmxOldDbgCtrl.TaskId)
	{
		OLD_DMX_DBG_PRINT("dmx debug task init fail\n");
		return RET_FAILURE;
	}
	
	return RET_SUCCESS;
}

static void dmx_old_dbg_init(void)
{
	if (!DmxOldDbgCtrl.DmxDbgInit)
	{
		memset(&DmxOldDbgCtrl, 0, sizeof(DmxOldDbgCtrl));
		DmxOldDbgCtrl.DmxDbgInit = TRUE;
		DmxOldDbgCtrl.DmxDbgStatEn = TRUE;
		DmxOldDbgCtrl.DbgLevel = DMX_DBG_LEVEL_KERNEL | DMX_DBG_LEVEL_SEE;
		DmxOldDbgCtrl.DbgShowIntv = DMX_DBG_PRINT_INTRV;
		
		if (DmxOldDbgStatDeviceOpen(0) < 0)
		{
			//OLD_DMX_DBG_PRINT("Demux debug device open fail!\n");
			return;
		}
	}

	return;
}

static void dmx_old_dbg_exit(int argc, char **argv)
{
	if (DmxOldDbgCtrl.DmxDbgInit)
	{
		DmxOldDbgCtrl.DmxDbgInit = FALSE;
		osal_task_delete(DmxOldDbgCtrl.TaskId);
	}
	DmxOldDbgStatDeviceClose(0);
	OLD_DMX_DBG_PRINT("DeMux debug statistics closed.\n");
		
	return;
}

static void dmx_old_dbg_start(int argc, char **argv)
{
	if (!DmxOldDbgCtrl.DmxDbgInit)
		dmx_old_dbg_init();

	if (!DmxOldDbgCtrl.DmxDbgTskInit)
	{
		if (dmx_old_dbg_task_Init() != RET_SUCCESS)
		{
			OLD_DMX_DBG_PRINT("Demux debug task initialized fail!\n");
			return;
		}
		DmxOldDbgCtrl.DmxDbgTskInit = TRUE;
	}
    
	DmxOldDbgCtrl.DmxDbgStatEn = TRUE;

	return;
}

static void dmx_old_dbg_pause(int argc, char **argv)
{
	DmxOldDbgCtrl.DmxDbgStatEn = FALSE;

	return;
}

static void dmx_old_dbg_realtime_on(int argc, char **argv)
{
	INT32 Ret;

	Ret = ioctl(DmxOldDbgCtrl.KernGlbFd, ALI_DMX_CHAN_KERN_GLB_REALTIME_SET, 1);

	if (Ret < 0)
	{
	    OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return;
	}

	Ret = ioctl(DmxOldDbgCtrl.SeeGlbFd, ALI_DMX_CHAN_SEE_GLB_REALTIME_SET, 1);

	if (Ret < 0)
	{
	    OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return;
	}

	return;
}

static void dmx_old_dbg_realtime_off(int argc, char **argv)
{
	INT32 Ret;
    
	Ret = ioctl(DmxOldDbgCtrl.KernGlbFd, ALI_DMX_CHAN_KERN_GLB_REALTIME_SET, 0);

	if (Ret < 0)
	{
	    OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return;
	}

	Ret = ioctl(DmxOldDbgCtrl.SeeGlbFd, ALI_DMX_CHAN_SEE_GLB_REALTIME_SET, 0);

	if (Ret < 0)
	{
	    OLD_DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return;
	}

	return;
}

static void dmx_old_dbg_show_help(int argc, char **argv)
{
	OLD_DMX_DBG_PRINT("\nModule name:\n");
	OLD_DMX_DBG_PRINT("\tdmx0\n");

	OLD_DMX_DBG_PRINT("\nModule Description:\n");
	OLD_DMX_DBG_PRINT("\tDeMux diagnostic debugging module.\n");	

	OLD_DMX_DBG_PRINT("\nSyntax:\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -option [arguments]...\n");

	OLD_DMX_DBG_PRINT("\nDescription:\n");
	OLD_DMX_DBG_PRINT("\t-h, --help\n");
	OLD_DMX_DBG_PRINT("\t\tSelf description of this debugging moduld.\n");
	OLD_DMX_DBG_PRINT("\t-s, --start\n");
	OLD_DMX_DBG_PRINT("\t\tStart DeMux debugging printout task.\n");
	OLD_DMX_DBG_PRINT("\t-p, --pause\n");
	OLD_DMX_DBG_PRINT("\t\tPause DeMux debugging printout task.\n");
	OLD_DMX_DBG_PRINT("\t-e, --exit\n");
	OLD_DMX_DBG_PRINT("\t\tExit DeMux debugging module.\n");
	OLD_DMX_DBG_PRINT("\t-i, --info items...\n");
	OLD_DMX_DBG_PRINT("\t\tShow TS, Audio, Video & Section stream infomation, such as PID, stream id, filter id & mask/value.\n");
	OLD_DMX_DBG_PRINT("\t\tall\t--All the infomation.\n");
	OLD_DMX_DBG_PRINT("\t\tvid\t--Video stream infomation.\n");
	OLD_DMX_DBG_PRINT("\t\taud\t--Audio stream infomation.\n");
	OLD_DMX_DBG_PRINT("\t\tpcr\t--PCR stream infomation.\n");
	OLD_DMX_DBG_PRINT("\t\tpes\t--PES stream infomation.\n");
	OLD_DMX_DBG_PRINT("\t\tts\t--TS stream infomation.\n");
	OLD_DMX_DBG_PRINT("\t\tsec\t--Section stream infomation.\n");
	OLD_DMX_DBG_PRINT("\t-op, --open items...\n");
	OLD_DMX_DBG_PRINT("\t\tOpen debugging item statistics printout.\n");
	OLD_DMX_DBG_PRINT("\t\tall\t--All the items.\n");
	OLD_DMX_DBG_PRINT("\t\tkglb\t--Kernel global statistics.\n");
	OLD_DMX_DBG_PRINT("\t\tsglb\t--SEE global statistics.\n");
	OLD_DMX_DBG_PRINT("\t\tvid\t--Video stream statistics.\n");
	OLD_DMX_DBG_PRINT("\t\taud\t--Audio stream statistics.\n");
	OLD_DMX_DBG_PRINT("\t\tpcr\t--PCR stream statistics.\n");
	OLD_DMX_DBG_PRINT("\t\tpes\t--PES stream statistics.\n");
	OLD_DMX_DBG_PRINT("\t\tts\t--TS stream statistics.\n");
	OLD_DMX_DBG_PRINT("\t\tsec\t--Section stream statistics.\n");
	OLD_DMX_DBG_PRINT("\t\treg\t--Hardware register content dump.\n");
	OLD_DMX_DBG_PRINT("\t\tiocmd\t--IO command printout.\n");
	OLD_DMX_DBG_PRINT("\t-cl, --close items...\n");
	OLD_DMX_DBG_PRINT("\t\tClose debugging item statistics printout.\n");
	OLD_DMX_DBG_PRINT("\t\tall\t--All the items.\n");
	OLD_DMX_DBG_PRINT("\t\tkglb\t--Kernel global statistics.\n");
	OLD_DMX_DBG_PRINT("\t\tsglb\t--SEE global statistics.\n");
	OLD_DMX_DBG_PRINT("\t\tvid\t--Video stream statistics.\n");
	OLD_DMX_DBG_PRINT("\t\taud\t--Audio stream statistics.\n");
	OLD_DMX_DBG_PRINT("\t\tpcr\t--PCR stream statistics.\n");
	OLD_DMX_DBG_PRINT("\t\tpes\t--PES stream statistics.\n");
	OLD_DMX_DBG_PRINT("\t\tts\t--TS stream statistics.\n");
	OLD_DMX_DBG_PRINT("\t\tsec\t--Section stream statistics.\n");
	OLD_DMX_DBG_PRINT("\t\treg\t--Hardware register content dump.\n");
	OLD_DMX_DBG_PRINT("\t\tiocmd\t--IO command printout.\n");
	OLD_DMX_DBG_PRINT("\t-terv number\n");
	OLD_DMX_DBG_PRINT("\t\tSet time interval of debugging printout task(The default value is \"5000\").\n");
	OLD_DMX_DBG_PRINT("\t-tms, --times number\n");
	OLD_DMX_DBG_PRINT("\t\tSet times of debugging printout task(The default value is \"0\").\n");
	OLD_DMX_DBG_PRINT("\t-l --level number\n");
	OLD_DMX_DBG_PRINT("\t\tSet debugging printout level(The default value is \"6\").\n");
	OLD_DMX_DBG_PRINT("\t\t0\t--No printout.\n");
	OLD_DMX_DBG_PRINT("\t\t1\t--Show HLD printout infomation.\n");
	OLD_DMX_DBG_PRINT("\t\t2\t--Show Kernel printout infomation.\n");
	OLD_DMX_DBG_PRINT("\t\t3\t--Show HLD & Kernel printout infomation.\n");
	OLD_DMX_DBG_PRINT("\t\t4\t--Show SEE printout infomation.\n");
	OLD_DMX_DBG_PRINT("\t\t5\t--Show HLD & SEE printout infomation.\n");
	OLD_DMX_DBG_PRINT("\t\t6\t--Show Kernel & SEE printout infomation.\n");
	OLD_DMX_DBG_PRINT("\t\t7\t--Show HLD & Kernel & SEE printout infomation.\n");
	OLD_DMX_DBG_PRINT("\t-rt\n");
	OLD_DMX_DBG_PRINT("\t\tOpen realtime debugging printout.\n");
	OLD_DMX_DBG_PRINT("\t-urt\n");
	OLD_DMX_DBG_PRINT("\t\tClose realtime debugging printout.\n");	

	OLD_DMX_DBG_PRINT("\nUsage example:\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -h[--help]\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -s\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -p\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -e[--exit]\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -i[--info] all [ts] [aud] [vid] [sec] \n");
	OLD_DMX_DBG_PRINT("\tdmx0 -op[--open] [all] [kglb] [sglb] [ts] [aud] [vid] [sec] [reg]\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -cl[--close] [all] [kglb] [sglb] [ts] [aud] [vid] [sec] [reg]\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -terv 5000\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -tms[--times] 0\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -l[--level] 6\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -rt\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -urt\n");	

	return;
}

static void dmx_old_app_show_help(int argc, char **argv)
{
	OLD_DMX_DBG_PRINT("\nModule name:\n");
	OLD_DMX_DBG_PRINT("\tdmx0\n");

	OLD_DMX_DBG_PRINT("\nModule Description:\n");
	OLD_DMX_DBG_PRINT("\tDeMux diagnostic debugging module.\n");	

	OLD_DMX_DBG_PRINT("\nSyntax:\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -option [arguments]...\n");

	OLD_DMX_DBG_PRINT("\nDescription:\n");
	OLD_DMX_DBG_PRINT("\t-h, --help\n");
	OLD_DMX_DBG_PRINT("\t\tSelf description of this debugging moduld.\n");
	OLD_DMX_DBG_PRINT("\t-e, --exit\n");
	OLD_DMX_DBG_PRINT("\t\tExit DeMux debugging module.\n");
	OLD_DMX_DBG_PRINT("\t-i, --info items...\n");
	OLD_DMX_DBG_PRINT("\t\tShow TS, Audio, Video & Section stream infomation, such as PID, stream id, filter id & mask/value.\n");
	OLD_DMX_DBG_PRINT("\t\tall\t--All the infomation.\n");
	OLD_DMX_DBG_PRINT("\t\tvid\t--Video stream infomation.\n");
	OLD_DMX_DBG_PRINT("\t\taud\t--Audio stream infomation.\n");
	OLD_DMX_DBG_PRINT("\t\tpcr\t--PCR stream infomation.\n");
	OLD_DMX_DBG_PRINT("\t\tts\t--TS stream infomation.\n");
	OLD_DMX_DBG_PRINT("\t\tsec\t--Section stream infomation.\n");
	OLD_DMX_DBG_PRINT("\t-rt\n");
	OLD_DMX_DBG_PRINT("\t\tOpen realtime debugging printout.\n");
	OLD_DMX_DBG_PRINT("\t-urt\n");
	OLD_DMX_DBG_PRINT("\t\tClose realtime debugging printout.\n");	

	OLD_DMX_DBG_PRINT("\nUsage example:\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -h[--help]\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -e[--exit]\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -i[--info] all [ts] [aud] [vid] [sec] \n");
	OLD_DMX_DBG_PRINT("\tdmx0 -rt\n");
	OLD_DMX_DBG_PRINT("\tdmx0 -urt\n");	

	return;
}

static ARG_CHK dmx_old_dbg_no_para_preview(int argc, char **argv, char *option)
{
	if (argc > 0)
	{
		OLD_DMX_DBG_PRINT("Option \"%s\": Should not be with any argument(s)!\n", option);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}

static void dmx_old_dbg_show_info(int argc, char **argv)
{
	DMX_DBG_INFO_TYPE info_type = 0;
	
	if (0 == argc)
		goto show_info_all;

	while (argc-- > 0)
	{
		if (!strcmp(*argv, "all"))
		{
show_info_all:
			info_type = DMX_DBG_SHOW_STREAM_INFO |
						DMX_DBG_SHOW_STREAM_INFO_SEC_SLOT |
						DMX_DBG_SHOW_STREAM_INFO_SEC_FLT |
						DMX_DBG_SHOW_STREAM_INFO_SEC_CH;
			DmxOldDbgShowVideoInfo(info_type, 0);
			DmxOldDbgShowAudioInfo(info_type, 0);
			DmxOldDbgShowPcrInfo(info_type, 0);
			DmxOldDbgShowSectionInfo(info_type, 0);		
			DmxOldDbgShowTsInfo(info_type, 0);
		}
		else if (!strcmp(*argv, "vid"))
		{
			DmxOldDbgShowVideoInfo(DMX_DBG_SHOW_STREAM_INFO, 0);
		}
		else if (!strcmp(*argv, "aud"))
		{
			DmxOldDbgShowAudioInfo(DMX_DBG_SHOW_STREAM_INFO, 0);
		}
		else if (!strcmp(*argv, "pcr"))
		{
			DmxOldDbgShowPcrInfo(DMX_DBG_SHOW_STREAM_INFO, 0);
		}
		else if (!strcmp(*argv, "secslot"))
		{
			info_type |= DMX_DBG_SHOW_STREAM_INFO_SEC_SLOT;
			DmxOldDbgShowSectionInfo(info_type, 0);
		}
		else if (!strcmp(*argv, "secflt"))
		{
			info_type |= DMX_DBG_SHOW_STREAM_INFO_SEC_FLT;
			DmxOldDbgShowSectionInfo(info_type, 0);
		}
		else if (!strcmp(*argv, "secch"))
		{
			info_type |= DMX_DBG_SHOW_STREAM_INFO_SEC_CH;
			DmxOldDbgShowSectionInfo(info_type, 0);
		}
		else if (!strcmp(*argv, "sec"))
		{
			info_type |= DMX_DBG_SHOW_STREAM_INFO;
			DmxOldDbgShowSectionInfo(info_type, 0);
		}		
		else if (!strcmp(*argv, "ts"))
		{
			DmxOldDbgShowTsInfo(DMX_DBG_SHOW_STREAM_INFO, 0);
		}		
		else
		{
			OLD_DMX_DBG_PRINT("Unrecognized argument(s)!\n");
			break;
		}

		argv++;
	}	
}

static ARG_CHK dmx_old_dbg_info_preview(int argc, char **argv, char *option)
{
	while (argc-- > 0)
	{
		if (!strcmp(*argv, "kglb") ||
			!strcmp(*argv, "sglb") ||
			!strcmp(*argv, "reg") ||
			!strcmp(*argv, "iocmd"))
		{
			OLD_DMX_DBG_PRINT("Option \"%s\": Unsupport argument \"%s\"!\n", option, *argv);
			return DBG_ARG_INVALID;
		}
		else if (strcmp(*argv, "all") &&
				strcmp(*argv, "ts") &&
				strcmp(*argv, "vid") &&
				strcmp(*argv, "aud") &&
				strcmp(*argv, "pcr") &&
#if 0        
				strcmp(*argv, "secslot") &&
				strcmp(*argv, "secflt") &&
				strcmp(*argv, "secch") &&
#endif 
				strcmp(*argv, "sec"))
		{
			OLD_DMX_DBG_PRINT("Option \"%s\": Unrecognized argument(s) \"%s\"!\n", option, *argv);
			return DBG_ARG_INVALID;
		}

		argv++;
	}	

	return DBG_ARG_VALID;
}

static void dmx_old_dbg_switch_on(int argc, char **argv)
{	
	if (!DmxOldDbgCtrl.DmxDbgInit)
		dmx_old_dbg_init();	

	if (!DmxOldDbgCtrl.DmxDbgTskInit)
	{
		if (dmx_old_dbg_task_Init() != RET_SUCCESS)
		{
			OLD_DMX_DBG_PRINT("Demux debug task initialized fail!\n");
			return;
		}
		DmxOldDbgCtrl.DmxDbgTskInit = TRUE;
	}

	if (0 == argc)
		goto swith_on_all;

	while (argc-- > 0)
	{
		if (!strcmp(*argv, "all"))
		{
swith_on_all:
			ioctl(DmxOldDbgCtrl.KernGlbFd, ALI_DMX_CHAN_KERN_GLB_START, 0);
			ioctl(DmxOldDbgCtrl.SeeGlbFd, ALI_DMX_CHAN_SEE_GLB_START, 0);
			ioctl(DmxOldDbgCtrl.HwRegFd, ALI_DMX_CHAN_HW_REG_START, 0);          
			DmxOldDbgCtrl.KernGlbStatEn = TRUE;
			DmxOldDbgCtrl.SeeGlbStatEn = TRUE;
			DmxOldDbgCtrl.VidStatEn = TRUE;
			DmxOldDbgCtrl.AudStatEn = TRUE;
			DmxOldDbgCtrl.PcrStatEn = TRUE;
			DmxOldDbgCtrl.SecStatEn = TRUE;
			DmxOldDbgCtrl.PesStatEn = TRUE;
			DmxOldDbgCtrl.TsStatEn = TRUE;
			DmxOldDbgCtrl.HwRegDumpEn = TRUE;
			DmxOldDbgCtrl.IoCmdStatEn = TRUE;
			OLD_DMX_DBG_PRINT("Kernel global statistics enabled.\n");
			OLD_DMX_DBG_PRINT("SEE global statistics enabled.\n");
			OLD_DMX_DBG_PRINT("Video statistics enabled.\n");
			OLD_DMX_DBG_PRINT("Audio statistics enabled.\n");
			OLD_DMX_DBG_PRINT("PCR statistics enabled.\n");
			OLD_DMX_DBG_PRINT("Section statistics enabled.\n");
			OLD_DMX_DBG_PRINT("Pes statistics enabled.\n");
			OLD_DMX_DBG_PRINT("TS(Rec) statistics enabled.\n");
			OLD_DMX_DBG_PRINT("DeMux hardware register dump enabled.\n");
			OLD_DMX_DBG_PRINT("DeMux I/O commands list enabled.\n");	
		}		
		else if (!strcmp(*argv, "kglb"))
		{
			ioctl(DmxOldDbgCtrl.KernGlbFd, ALI_DMX_CHAN_KERN_GLB_START, 0);
			DmxOldDbgCtrl.KernGlbStatEn = TRUE;
			OLD_DMX_DBG_PRINT("Kernel global statistics enabled.\n");
		}
		else if (!strcmp(*argv, "sglb"))
		{
			ioctl(DmxOldDbgCtrl.SeeGlbFd, ALI_DMX_CHAN_SEE_GLB_START, 0);
			DmxOldDbgCtrl.SeeGlbStatEn = TRUE;
			OLD_DMX_DBG_PRINT("SEE global statistics enabled.\n");
		}
		else if (!strcmp(*argv, "vid"))
		{
			DmxOldDbgCtrl.VidStatEn = TRUE;
			OLD_DMX_DBG_PRINT("Video statistics enabled.\n");
		}
		else if (!strcmp(*argv, "aud"))
		{
			DmxOldDbgCtrl.AudStatEn = TRUE;
			OLD_DMX_DBG_PRINT("Audio statistics enabled.\n");
		}
		else if (!strcmp(*argv, "pcr"))
		{
			DmxOldDbgCtrl.PcrStatEn = TRUE;
			OLD_DMX_DBG_PRINT("PCR statistics enabled.\n");
		}
		else if (!strcmp(*argv, "sec"))
		{
			DmxOldDbgCtrl.SecStatEn = TRUE;
			OLD_DMX_DBG_PRINT("Section statistics enabled.\n");
		}
		else if (!strcmp(*argv, "pes"))
		{
			DmxOldDbgCtrl.PesStatEn = TRUE;
			OLD_DMX_DBG_PRINT("PES statistics enabled.\n");
		}
		else if (!strcmp(*argv, "ts"))
		{
			DmxOldDbgCtrl.TsStatEn = TRUE;
			OLD_DMX_DBG_PRINT("Ts(Rec) statistics enabled.\n");
		}
		else if (!strcmp(*argv, "reg"))
		{
			ioctl(DmxOldDbgCtrl.HwRegFd, ALI_DMX_CHAN_HW_REG_START, 0);
			DmxOldDbgCtrl.HwRegDumpEn = TRUE;
			OLD_DMX_DBG_PRINT("DeMux hardware register dump enabled.\n");
		}
		else if (!strcmp(*argv, "iocmd"))
		{
			DmxOldDbgCtrl.IoCmdStatEn = TRUE;
			OLD_DMX_DBG_PRINT("DeMux I/O commands list enabled.\n");
		}
		else
		{
			OLD_DMX_DBG_PRINT("Unrecognized argument(s)!\n");
		}

		argv++;
	}

	dmx_dbg_set(DmxOldDbgCtrl.IoCmdStatEn);

	DmxOldDbgCtrl.DmxDbgStatEn = TRUE;

	OLD_DMX_DBG_PRINT("Waiting for statistics printout...\n");
	
	return;	
}

static void dmx_old_dbg_switch_off(int argc, char **argv)
{
	if (!DmxOldDbgCtrl.DmxDbgInit)
		dmx_old_dbg_init();	

	if (!DmxOldDbgCtrl.DmxDbgTskInit)
	{
		if (dmx_old_dbg_task_Init() != RET_SUCCESS)
		{
			OLD_DMX_DBG_PRINT("Demux debug task initialized fail!\n");
			return;
		}
		DmxOldDbgCtrl.DmxDbgTskInit = TRUE;
	}

	if (0 == argc)
		goto switch_off_all;

	while (argc-- > 0)
	{
		if (!strcmp(*argv, "all"))
		{
switch_off_all:
			ioctl(DmxOldDbgCtrl.KernGlbFd, ALI_DMX_CHAN_KERN_GLB_STOP, 0);
			ioctl(DmxOldDbgCtrl.SeeGlbFd, ALI_DMX_CHAN_SEE_GLB_STOP, 0);
			ioctl(DmxOldDbgCtrl.HwRegFd, ALI_DMX_CHAN_HW_REG_STOP, 0);			
			DmxOldDbgCtrl.KernGlbStatEn = FALSE;
			DmxOldDbgCtrl.SeeGlbStatEn = FALSE;
			DmxOldDbgCtrl.VidStatEn = FALSE;
			DmxOldDbgCtrl.AudStatEn = FALSE;
			DmxOldDbgCtrl.PcrStatEn = FALSE;
			DmxOldDbgCtrl.SecStatEn = FALSE;
			DmxOldDbgCtrl.PesStatEn = FALSE;
			DmxOldDbgCtrl.TsStatEn = FALSE;
			DmxOldDbgCtrl.HwRegDumpEn = FALSE;
			DmxOldDbgCtrl.IoCmdStatEn = FALSE;
			OLD_DMX_DBG_PRINT("Kernel global statistics disdabled.\n");
			OLD_DMX_DBG_PRINT("SEE global statistics disdabled.\n");
			OLD_DMX_DBG_PRINT("Video statistics disdabled.\n");
			OLD_DMX_DBG_PRINT("Audio statistics disdabled.\n");
			OLD_DMX_DBG_PRINT("PCR statistics disdabled.\n");
			OLD_DMX_DBG_PRINT("Section statistics disabled.\n");
			OLD_DMX_DBG_PRINT("PES statistics disabled.\n");
			OLD_DMX_DBG_PRINT("TS(Rec) statistics disabled.\n");
			OLD_DMX_DBG_PRINT("DeMux hardware register dump disdabled.\n");
			OLD_DMX_DBG_PRINT("DeMux I/O commands list disdabled.\n");	
		}
		else if (!strcmp(*argv, "kglb"))
		{
			ioctl(DmxOldDbgCtrl.KernGlbFd, ALI_DMX_CHAN_KERN_GLB_STOP, 0);
			DmxOldDbgCtrl.KernGlbStatEn = FALSE;
			OLD_DMX_DBG_PRINT("Kernel global statistics disdabled.\n");
		}
		else if (!strcmp(*argv, "sglb"))
		{
			ioctl(DmxOldDbgCtrl.SeeGlbFd, ALI_DMX_CHAN_SEE_GLB_STOP, 0);
			DmxOldDbgCtrl.SeeGlbStatEn = FALSE;
			OLD_DMX_DBG_PRINT("SEE global statistics disdabled.\n");
		}
		else if (!strcmp(*argv, "vid"))
		{
			DmxOldDbgCtrl.VidStatEn = FALSE;
			OLD_DMX_DBG_PRINT("Video statistics disdabled.\n");
		}
		else if (!strcmp(*argv, "aud"))
		{
			DmxOldDbgCtrl.AudStatEn = FALSE;
			OLD_DMX_DBG_PRINT("Audio statistics disdabled.\n");
		}
		else if (!strcmp(*argv, "pcr"))
		{
			DmxOldDbgCtrl.PcrStatEn = FALSE;
			OLD_DMX_DBG_PRINT("PCR statistics disdabled.\n");
		}
		else if (!strcmp(*argv, "sec"))
		{
			DmxOldDbgCtrl.SecStatEn = FALSE;
			OLD_DMX_DBG_PRINT("Section statistics disabled.\n");
		}
		else if (!strcmp(*argv, "pes"))
		{
			DmxOldDbgCtrl.PesStatEn = FALSE;
			OLD_DMX_DBG_PRINT("PES statistics disabled.\n");
		}
		else if (!strcmp(*argv, "ts"))
		{
			DmxOldDbgCtrl.TsStatEn = FALSE;
			OLD_DMX_DBG_PRINT("Ts(Rec) statistics disabled.\n");
		}		
		else if (!strcmp(*argv, "reg"))
		{
			ioctl(DmxOldDbgCtrl.HwRegFd, ALI_DMX_CHAN_HW_REG_STOP, 0);			
			DmxOldDbgCtrl.HwRegDumpEn = FALSE;
			OLD_DMX_DBG_PRINT("DeMux hardware register dump disdabled.\n");
		}
		else if (!strcmp(*argv, "iocmd"))
		{
			DmxOldDbgCtrl.IoCmdStatEn = FALSE;
			OLD_DMX_DBG_PRINT("DeMux I/O commands list disdabled.\n");
		}
		else
		{
			OLD_DMX_DBG_PRINT("Unrecognized argument(s)!\n");
		}

		argv++;
	}

	DmxOldDbgCtrl.DmxDbgStatEn = TRUE;
	
	dmx_dbg_set(DmxOldDbgCtrl.IoCmdStatEn);

	return;	
}

static ARG_CHK dmx_old_dbg_swi_preview(int argc, char **argv, char *option)
{
	while (argc-- > 0)
	{
		if (strcmp(*argv, "all") &&
			strcmp(*argv, "kglb") &&
			strcmp(*argv, "sglb") &&
			strcmp(*argv, "reg") &&
			strcmp(*argv, "vid") &&
			strcmp(*argv, "aud") &&
			strcmp(*argv, "pcr") &&
			strcmp(*argv, "sec") &&
			strcmp(*argv, "ts") &&
			strcmp(*argv, "iocmd"))
		{
			OLD_DMX_DBG_PRINT("Option \"%s\": Unrecognized argument \"%s\"!\n", option, *argv);
			return DBG_ARG_INVALID;
		}

		argv++;
	}

	return DBG_ARG_VALID;
}

static void dmx_old_dbg_set_level(int argc, char **argv)
{
	INT32 number, ret;

	if (0 == argc)
	{
		OLD_DMX_DBG_PRINT("Lack of valid argument(ex: \"100\")!\n");
		return;
	}

	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0 ||
		number > (DMX_DBG_LEVEL_KERNEL | DMX_DBG_LEVEL_HLD | DMX_DBG_LEVEL_SEE))
	{
		OLD_DMX_DBG_PRINT("Argument \"%s\" not a number or out of range 0 ~ 7!\n", *argv);
		return;
	}

	if (!DmxOldDbgCtrl.DmxDbgInit)
		dmx_old_dbg_init();	

	DmxOldDbgCtrl.DbgLevel = number;
	DmxOldDbgCtrl.DmxDbgStatEn = TRUE;

	return;
}

static ARG_CHK dmx_old_dbg_level_preview(int argc, char **argv, char *option)
{
	INT32 number, ret;

	if (0 == argc)
	{
		OLD_DMX_DBG_PRINT("Option \"%s\": Lack of valid argument(ex: number)!\n", option);
		return DBG_ARG_INVALID;
	}

	if (argc != 1)
	{
		OLD_DMX_DBG_PRINT("Option \"%s\": Only one argument admitted!\n", option);
		return DBG_ARG_INVALID;
	}
	
	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0 ||
		number > (DMX_DBG_LEVEL_KERNEL | DMX_DBG_LEVEL_HLD | DMX_DBG_LEVEL_SEE))
	{
		OLD_DMX_DBG_PRINT("Option \"%s\": Argument \"%s\" not a number or out of range 0-7!\n", option, *argv);
		return DBG_ARG_INVALID;
	}
	
	return DBG_ARG_VALID;
}

static void dmx_old_dbg_set_intv(int argc, char **argv)
{
	INT32 number, ret;

	if (0 == argc)
	{
		OLD_DMX_DBG_PRINT("Lack of valid argument(ex: \"5000\")!\n");
		return;
	}
	
	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		OLD_DMX_DBG_PRINT("Argument \"%s\" not a number or less than zero!\n", *argv);
		return;
	}

	if (!DmxOldDbgCtrl.DmxDbgInit)
		dmx_old_dbg_init();		

	DmxOldDbgCtrl.DbgShowIntv = number;
	DmxOldDbgCtrl.DmxDbgStatEn = TRUE;

	return;
}

static void dmx_old_dbg_set_tms(int argc, char **argv)
{
	INT32 number, ret;

	if (0 == argc)
	{
		OLD_DMX_DBG_PRINT("Lack of valid argument(ex: \"5\")!\n");
		return;
	}

	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		OLD_DMX_DBG_PRINT("Argument \"%s\" not a number or less than zero!\n", *argv);
		return;
	}

	if (!DmxOldDbgCtrl.DmxDbgInit)
		dmx_old_dbg_init();		

	DmxOldDbgCtrl.DbgShowTms = number;
	DmxOldDbgCtrl.DmxDbgStatEn = TRUE;

	return;
}

static ARG_CHK dmx_old_dbg_num_preview(int argc, char **argv, char *option)
{
	INT32 number, ret, arg_cnt;
	char *p;

	if (0 == argc)
	{
		OLD_DMX_DBG_PRINT("Option \"%s\": Lack of valid argument(ex: number)!\n", option);
		return DBG_ARG_INVALID;
	}

	if (argc != 1)
	{
		OLD_DMX_DBG_PRINT("Option \"%s\": Only one argument admitted!\n", option);
		return DBG_ARG_INVALID;
	}
	
	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		OLD_DMX_DBG_PRINT("Option \"%s\": Argument \"%s\" not a number or less than zero!\n", option, *argv);
		return DBG_ARG_INVALID;
	}
	
	return DBG_ARG_VALID;
}

/*****************************************************************
* Used in the old DMX Driver *
******************************************************************/
static PARSE_COMMAND_LIST dmx0dbg_command[] = {
	{ { NULL, NULL }, dmx_old_dbg_start, dmx_old_dbg_no_para_preview, "start", "s", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_pause, dmx_old_dbg_no_para_preview, "pause", "p", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_show_help, dmx_old_dbg_no_para_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_show_info, dmx_old_dbg_info_preview, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_switch_on, dmx_old_dbg_swi_preview, "open", "op", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_switch_off, dmx_old_dbg_swi_preview, "close", "cl", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_set_level, dmx_old_dbg_level_preview, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_set_intv, dmx_old_dbg_num_preview, NULL, "terv", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_set_tms, dmx_old_dbg_num_preview, "times", "tms", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_exit, dmx_old_dbg_no_para_preview, "exit", "e", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_realtime_on, dmx_old_dbg_no_para_preview, NULL, "rt", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_realtime_off, dmx_old_dbg_no_para_preview, NULL, "urt", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};

static PARSE_COMMAND_LIST dmx0dbg_app_command[] = {
	//{ { NULL, NULL }, dmx_old_dbg_start, dmx_old_dbg_no_para_preview, "start", "s", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, dmx_old_dbg_pause, dmx_old_dbg_no_para_preview, "pause", "p", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_app_show_help, dmx_old_dbg_no_para_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_show_info, dmx_old_dbg_info_preview, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, dmx_old_dbg_switch_on, dmx_old_dbg_swi_preview, "open", "op", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, dmx_old_dbg_switch_off, dmx_old_dbg_swi_preview, "close", "cl", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, dmx_old_dbg_set_level, dmx_old_dbg_level_preview, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, dmx_old_dbg_set_intv, dmx_old_dbg_num_preview, NULL, "terv", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, dmx_old_dbg_set_tms, dmx_old_dbg_num_preview, "times", "tms", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_exit, dmx_old_dbg_no_para_preview, "exit", "e", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_realtime_on, dmx_old_dbg_no_para_preview, NULL, "rt", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_old_dbg_realtime_off, dmx_old_dbg_no_para_preview, NULL, "urt", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};

#if 0
void dmxdbg_old_module_register(void)
{
	debug_module_add("dmx0", &dmxdbg_old_command[0], ARRAY_SIZE(dmxdbg_old_command));
}
#endif

INT32 dmx0_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &dmx0dbg_command[0];
	*list_cnt = ARRAY_SIZE(dmx0dbg_command);

	if (!DmxOldDbgCtrl.DmxDbgInit)
		dmx_old_dbg_init();

	return RET_SUCCESS;
}

INT32 dmx0_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &dmx0dbg_app_command[0];
	*list_cnt = ARRAY_SIZE(dmx0dbg_app_command);

	if (!DmxOldDbgCtrl.DmxDbgInit)
		dmx_old_dbg_init();

	return RET_SUCCESS;
}
