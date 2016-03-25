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
*					2012-09-30	 Leo.Ma      Ver 1.0	Create File
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

#include <linux/Ali_DmxLibInternal.h>
#include <linux/Ali_DmxLib.h>
//#include <linux/dvb/ali_dmx.h>
#include <ali_dmx_common.h>
#include <hld_cfg.h>


#define __ADR_DMX_DBG
#ifdef __ADR_DMX_DBG
#define DMX_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(DMX, fmt, ##args); \
			} while(0)

#else
#define DMX_DBG_PRINT(...)
#endif

#define DMX_DBG_CMD_NR		10
#define DMX_DBG_PRINT_INTRV	5000

typedef enum dmx_dbg_info_type {
	DMX_DBG_SHOW_STATISTICS = 0,
	DMX_DBG_SHOW_STREAM_INFO = 1,
	DMX_DBG_SHOW_STREAM_INFO_SEC_SLOT = 2,
	DMX_DBG_SHOW_STREAM_INFO_SEC_FLT = 4,
	DMX_DBG_SHOW_STREAM_INFO_SEC_CH = 8,
} DMX_DBG_INFO_TYPE;

typedef enum dmx_dbg_level
{
	DMX_DBG_LEVEL_HLD = 0x1,
    DMX_DBG_LEVEL_KERNEL = 0x2,
    DMX_DBG_LEVEL_SEE = 0x4,
} DMX_DBG_LEVEL;

typedef struct dmx_dbg_stat_ctrl
{
	int KernGlbFd;
	int SeeGlbFd;
	int HwRegFd;

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


extern UINT8 * Ali_DmxId2Path(enum ALI_DMX_IN_OUT, UINT32);
extern INT32 Ali_StreamIdx2Id(enum ALI_DMX_STREAM_TYPE, INT32);
extern INT32 Ali_DmxTsStreamErrStatGet(INT32, struct Ali_DmxLibTsStrmStatInfo *);
extern INT32 Ali_DmxSecStreamErrStatGet(INT32, struct Ali_DmxLibSecStrmStatInfo *);
extern INT32 Ali_DmxSecSlotErrStatGet(INT32, struct Ali_DmxLibSecSlotStatInfo *);
extern INT32 Ali_DmxSecFltErrStatGet(INT32, struct Ali_DmxLibSecFltStatInfo *);
extern INT32 Ali_DmxSecChErrStatGet(INT32, struct Ali_DmxLibSecChStatInfo *);
extern INT32 Ali_DmxSecChErrStatGet(INT32, struct Ali_DmxLibSecChStatInfo *);
extern INT32 Ali_DmxSecChErrStatGet(INT32, struct Ali_DmxLibSecChStatInfo *);
extern INT32 Ali_DmxVideoStreamErrStatGet(INT32, struct Ali_DmxLibVideoStrmStatInfo *);
extern INT32 Ali_DmxAudioStreamErrStatGet(INT32, struct Ali_DmxLibAudioStrmStatInfo *);
extern INT32 Ali_DmxPcrStreamErrStatGet(INT32, struct Ali_DmxLibPcrStrmStatInfo *);
	
static struct dmx_dbg_stat_ctrl DmxDbgCtrl;

INT32 DmxDbgStatDeviceOpen(UINT32 DmxId)
{
	INT32	Ret;
	UINT8 * DmxPath;

	DmxPath = (UINT8 *)Ali_DmxId2Path(ALI_DMX_OUTPUT, DmxId);

    DmxDbgCtrl.KernGlbFd = open(DmxPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

	if (DmxDbgCtrl.KernGlbFd < 0)
    {
        DMX_DBG_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return (-1);
    }	
	
    DmxDbgCtrl.SeeGlbFd = open(DmxPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

	if (DmxDbgCtrl.SeeGlbFd < 0)
    {
        DMX_DBG_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return (-1);
    }	
	
    DmxDbgCtrl.HwRegFd = open(DmxPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

	if (DmxDbgCtrl.HwRegFd < 0)
    {
        DMX_DBG_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return (-1);
    }	
	
	Ret = ioctl(DmxDbgCtrl.KernGlbFd, ALI_DMX_KERN_GLB_STREAM_CFG, 0);

	if (Ret < 0)
	{
	    DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	Ret = ioctl(DmxDbgCtrl.SeeGlbFd, ALI_DMX_SEE_GLB_STREAM_CFG, 0);

	if (Ret < 0)
	{
	    DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	Ret = ioctl(DmxDbgCtrl.HwRegFd, ALI_DMX_HW_REG_STREAM_CFG, 0);

	if (Ret < 0)
	{
	    DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}
	
	return (0);
}

void DmxDbgStatDeviceClose(UINT32 DmxId)
{
	UINT8 * DmxPath;

	DmxPath = (UINT8 *)Ali_DmxId2Path(ALI_DMX_OUTPUT, DmxId);

    if (DmxDbgCtrl.KernGlbFd > 0)
    {
		close(DmxDbgCtrl.KernGlbFd);
    }

    if (DmxDbgCtrl.SeeGlbFd > 0)
    {
		close(DmxDbgCtrl.SeeGlbFd);
    }
	
    if (DmxDbgCtrl.HwRegFd > 0)
    {
		close(DmxDbgCtrl.HwRegFd);
    }
	
	memset(&DmxDbgCtrl, 0, sizeof(DmxDbgCtrl));

	return;
}

INT32 PrintKernelGlobalStatInfo(UINT32 SysTickInc)
{
	INT32								Ret;
	UINT32              	        	Bitrate;
    UINT32                  	    	TsInCntInc;
	static UINT32               		LastTsInCnt;
	struct Ali_DmxKernGlobalStatInfo	GlobalStatInfo;
	
	if (DmxDbgCtrl.KernGlbFd < 0)
	{
	    DMX_DBG_PRINT("Invalid DeMux debug kernel global FD: %d\n", DmxDbgCtrl.KernGlbFd);

		return (-1);
	}

	if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
	{	
		memset(&GlobalStatInfo, 0, sizeof(GlobalStatInfo));

		Ret = ioctl(DmxDbgCtrl.KernGlbFd, ALI_DMX_KERN_GLB_STREAM_INFO_GET, &GlobalStatInfo);

	    if (Ret < 0)
	    {
	        DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

			return (-1);
	    }

		DMX_DBG_PRINT("\nKernel Global:\n");

		if (SysTickInc != 0 && GlobalStatInfo.TotalTsInCnt > LastTsInCnt)
		{
			TsInCntInc = GlobalStatInfo.TotalTsInCnt - LastTsInCnt;
		    Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
		    DMX_DBG_PRINT("TotalBitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
		}
		LastTsInCnt = GlobalStatInfo.TotalTsInCnt;
		
		DMX_DBG_PRINT("-----------------------------------------------\n");

		DMX_DBG_PRINT("TotalTsInCnt:%u\n", GlobalStatInfo.TotalTsInCnt);

		DMX_DBG_PRINT("OverlapCnt:%u\n", GlobalStatInfo.OverlapCnt);

		if (GlobalStatInfo.PlayBusyCnt != 0)
		{
			DMX_DBG_PRINT("PlayBusyCnt:%u\n", GlobalStatInfo.PlayBusyCnt);
		}

		if (GlobalStatInfo.DmxBufEmptyCnt != 0)
		{
			DMX_DBG_PRINT("DmxBufEmptyCnt:%u\n", GlobalStatInfo.DmxBufEmptyCnt);
		}

		if (GlobalStatInfo.NobodyCareCnt != 0)
		{
			DMX_DBG_PRINT("NobodyCareCnt:%u\n", GlobalStatInfo.NobodyCareCnt);
		}
	}
	
	return (0);
}

INT32 PrintSeeGlobalStatInfo(UINT32 SysTickInc)
{
	INT32                           Ret;
	UINT32                          Bitrate;
    UINT32                          TsInCntInc;
	static UINT32                   LastTsInCnt;	
	struct Ali_DmxSeeGlobalStatInfo GlobalStatInfo;

	if (DmxDbgCtrl.SeeGlbFd < 0)
	{
	    DMX_DBG_PRINT("Invalid DeMux debug SEE global FD: %d\n", DmxDbgCtrl.SeeGlbFd);

		return (-1);
	}

	if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_SEE)
	{	
		memset(&GlobalStatInfo, 0, sizeof(GlobalStatInfo));

		Ret = ioctl(DmxDbgCtrl.SeeGlbFd, ALI_DMX_SEE_GLB_STREAM_INFO_GET, &GlobalStatInfo);

	    if (Ret < 0)
	    {
	        DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

			return (-1);
	    }

		DMX_DBG_PRINT("\nSEE Global:\n");

		if (SysTickInc != 0 && GlobalStatInfo.TotalTsInCnt > LastTsInCnt)
		{
			TsInCntInc = GlobalStatInfo.TotalTsInCnt - LastTsInCnt;
		    Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
	    	DMX_DBG_PRINT("TotalBitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
		}
		LastTsInCnt = GlobalStatInfo.TotalTsInCnt;
		
		DMX_DBG_PRINT("-----------------------------------------------\n");

		DMX_DBG_PRINT("TotalTsInCnt:%u\n", GlobalStatInfo.TotalTsInCnt);

		if (GlobalStatInfo.TsOddCnt != 0)
		{
		    DMX_DBG_PRINT("TsOddCnt:%u\n", GlobalStatInfo.TsOddCnt);
		}
		
		if (GlobalStatInfo.TsSyncErrCnt != 0)
		{
	    	DMX_DBG_PRINT("TsSyncErrCnt:%u\n", GlobalStatInfo.TsSyncErrCnt);
		}
		
	    DMX_DBG_PRINT("TsDecrySucCnt:%u\n", GlobalStatInfo.TsDecrySucCnt);

		if (GlobalStatInfo.TsDecryFailCnt != 0)
		{
		    DMX_DBG_PRINT("TsDecryFailCnt:%u\n", GlobalStatInfo.TsDecryFailCnt);
		}
		
		if (GlobalStatInfo.TsDecryEmptyCnt != 0)
		{
		    DMX_DBG_PRINT("TsDecryEmptyCnt:%u\n", GlobalStatInfo.TsDecryEmptyCnt);
		}
		
		DMX_DBG_PRINT("TsRetToMainCnt:%u\n", GlobalStatInfo.TsRetToMainCnt);
	}
	
	return (0);
}

static void PrintTsStatInfoHld(struct Ali_DmxTsStreamInfo* StreamInfo, INT32 FltIdx)
{
	struct Ali_DmxLibTsStrmStatInfo TsStrmStatInfo;

	if (NULL == StreamInfo)
	{
		return;
	}

	memset(&TsStrmStatInfo, 0, sizeof(TsStrmStatInfo));

	if (Ali_DmxTsStreamErrStatGet(StreamInfo->Idx + ALI_DMX_STREAM_TYPE_TS, &TsStrmStatInfo))
	{
		return;
	}

	DMX_DBG_PRINT("\nTS Statistics StrIdx:%u PID:%u (HLD Part)\n", 
		StreamInfo->Idx, StreamInfo->StreamParam.PidList[FltIdx]);

	if (TsStrmStatInfo.InvPathCnt != 0)
	{
		DMX_DBG_PRINT("InvPathCnt:%u\n", TsStrmStatInfo.InvPathCnt);
	}

	if (TsStrmStatInfo.IoOpenFailCnt != 0)
	{
		DMX_DBG_PRINT("IoOpenFailCnt:%u\n", TsStrmStatInfo.IoOpenFailCnt);
	}

	if (TsStrmStatInfo.IoCfgFailCnt != 0)
	{
		DMX_DBG_PRINT("IoCfgFailCnt:%u\n", TsStrmStatInfo.IoCfgFailCnt);
	}

	if (TsStrmStatInfo.IoStartFailCnt != 0)
	{
		DMX_DBG_PRINT("IoStartFailCnt:%u\n", TsStrmStatInfo.IoStartFailCnt);
	}

	if (TsStrmStatInfo.IoStopFailCnt != 0)
	{
		DMX_DBG_PRINT("IoStopFailCnt:%u\n", TsStrmStatInfo.IoStopFailCnt);
	}

	return;
}

static INT32 PrintTsStatInfoKernel(struct Ali_DmxTsStreamInfo* StreamInfo, INT32 FltIdx, UINT32 SysTickInc)
{
	INT32							Ret;
	UINT32							Bitrate;
    UINT32							TsInCntInc;
	static UINT32					LastTsInCnt;
	struct Ali_DmxDrvTsStrmStatInfo	TsStrmStatInfo;
	struct Ali_DmxDrvTsFltStatInfo	TsFltStatInfo;

	if (NULL == StreamInfo)
	{
		return (-1);
	}

	memset(&TsStrmStatInfo, 0, sizeof(TsStrmStatInfo));

	memset(&TsFltStatInfo, 0, sizeof(TsFltStatInfo));

	TsStrmStatInfo.TsFltIdx = StreamInfo->StreamParam.TsFltId[FltIdx];

	TsFltStatInfo.TsFltIdx = StreamInfo->StreamParam.TsFltId[FltIdx];

	Ret = ioctl(StreamInfo->Fd, ALI_DMX_TS_STREAM_INFO_GET, &TsStrmStatInfo);

    if (Ret < 0)
    {
        DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
    }

	Ret = ioctl(StreamInfo->Fd, ALI_DMX_TS_FILTER_INFO_GET, &TsFltStatInfo);

    if (Ret < 0)
    {
        DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
    }

	DMX_DBG_PRINT("\nTS Statistics StrIdx:%u TsFltIdx:%u PID:%u (Kernel Part)\n", 
		StreamInfo->Idx, TsFltStatInfo.TsFltIdx, StreamInfo->StreamParam.PidList[FltIdx]);

	if (SysTickInc != 0 && TsFltStatInfo.TsInCnt > LastTsInCnt)
	{
		TsInCntInc = TsFltStatInfo.TsInCnt - LastTsInCnt;
	    Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
	    DMX_DBG_PRINT("Bitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
	}
	LastTsInCnt = TsFltStatInfo.TsInCnt;
	
	DMX_DBG_PRINT("------------------------------------------------------\n");

	if (TsStrmStatInfo.StatErrCnt != 0)
	{
		DMX_DBG_PRINT("StatErrCnt:%u\n", TsStrmStatInfo.StatErrCnt);
	}

	if (TsStrmStatInfo.StrTypeErrCnt != 0)
	{
		DMX_DBG_PRINT("StrTypeErrCnt:%u\n", TsStrmStatInfo.StrTypeErrCnt);
	}

	if (TsStrmStatInfo.RdByteErrCnt != 0)
	{
		DMX_DBG_PRINT("RdByteErrCnt:%u\n", TsStrmStatInfo.RdByteErrCnt);
	}

	if (TsStrmStatInfo.WrByteErrCnt != 0)
	{
		DMX_DBG_PRINT("WrByteErrCnt:%u\n", TsStrmStatInfo.WrByteErrCnt);
	}

	if (TsStrmStatInfo.NoPidCnt != 0)
	{
		DMX_DBG_PRINT("NoPidCnt:%u\n", TsStrmStatInfo.NoPidCnt);
	}

	if (TsStrmStatInfo.CopyErrCnt != 0)
	{
		DMX_DBG_PRINT("CopyErrCnt:%u\n", TsStrmStatInfo.CopyErrCnt);
	}

	if (TsStrmStatInfo.CfgFailCnt != 0)
	{
		DMX_DBG_PRINT("CfgFailCnt:%u\n", TsStrmStatInfo.CfgFailCnt);
	}

	if (TsStrmStatInfo.StartFailCnt != 0)
	{
		DMX_DBG_PRINT("StartFailCnt:%u\n", TsStrmStatInfo.StartFailCnt);
	}

	if (TsStrmStatInfo.StopFailCnt != 0)
	{
		DMX_DBG_PRINT("StopFailCnt:%u\n", TsStrmStatInfo.StopFailCnt);
	}

	if (TsStrmStatInfo.CloseFailCnt != 0)
	{
		DMX_DBG_PRINT("CloseFailCnt:%u\n", TsStrmStatInfo.CloseFailCnt);
	}

	DMX_DBG_PRINT("TsInCnt:%u\n", TsFltStatInfo.TsInCnt);

	DMX_DBG_PRINT("TsScrmbCnt:%u\n", TsFltStatInfo.TsScrmbCnt);

	if (TsFltStatInfo.TsSyncByteErrCnt != 0)
	{
		DMX_DBG_PRINT("TsSyncByteErrCnt:%u\n", TsFltStatInfo.TsSyncByteErrCnt);
	}
	
	if (TsFltStatInfo.TsErrCnt != 0)
	{
		DMX_DBG_PRINT("TsErrCnt:%u\n", TsFltStatInfo.TsErrCnt);
	}
	
	if (TsFltStatInfo.TsDupCnt != 0)
	{
		DMX_DBG_PRINT("TsDupCnt:%u\n", TsFltStatInfo.TsDupCnt);
	}
	
	if (TsFltStatInfo.TsLostCnt != 0)
	{
		DMX_DBG_PRINT("TsLostCnt:%u\n", TsFltStatInfo.TsLostCnt);
	}
					
	return (0);
}

static void PrintSecStrmStatInfoHld(struct Ali_DmxSecStreamInfo* StreamInfo)
{
	struct Ali_DmxLibSecStrmStatInfo SecStrmStatInfo;

	if (NULL == StreamInfo)
	{
		return;
	}

	memset(&SecStrmStatInfo, 0, sizeof(SecStrmStatInfo));

	if (Ali_DmxSecStreamErrStatGet(StreamInfo->Idx + ALI_DMX_STREAM_TYPE_SEC, &SecStrmStatInfo))
	{
		return;
	}
	
	DMX_DBG_PRINT("\nSection Stream Statistics StrIdx:%u PID:%u (HLD Part)\n", 
		StreamInfo->Idx, StreamInfo->StreamParam.Pid);

	DMX_DBG_PRINT("----------------------------------------------------------\n");

	if (SecStrmStatInfo.InvPathCnt != 0)
	{
		DMX_DBG_PRINT("InvPathCnt:%u\n", SecStrmStatInfo.InvPathCnt);
	}

	if (SecStrmStatInfo.CrcErrCnt != 0)
	{
		DMX_DBG_PRINT("CrcErrCnt:%u\n", SecStrmStatInfo.CrcErrCnt);
	}

	if (SecStrmStatInfo.IoOpenFailCnt != 0)
	{
		DMX_DBG_PRINT("IoOpenFailCnt:%u\n", SecStrmStatInfo.IoOpenFailCnt);
	}

	if (SecStrmStatInfo.IoCfgFailCnt != 0)
	{
		DMX_DBG_PRINT("IoCfgFailCnt:%u\n", SecStrmStatInfo.IoCfgFailCnt);
	}

	if (SecStrmStatInfo.IoStartFailCnt != 0)
	{
		DMX_DBG_PRINT("IoStartFailCnt:%u\n", SecStrmStatInfo.IoStartFailCnt);
	}

	if (SecStrmStatInfo.IoStopFailCnt != 0)
	{
		DMX_DBG_PRINT("IoStopFailCnt:%u\n", SecStrmStatInfo.IoStopFailCnt);
	}
	
	return;
}

static void PrintSecSlotStatInfoHld(struct Ali_DmxSecSlotInfo* SlotInfo)
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
	
	DMX_DBG_PRINT("\nSection Slot Statistics SlotIdx:%u StrIdx:%u PID:%u (HLD Part)\n", 
		SlotInfo->Idx, SlotInfo->SecStreamId, SlotInfo->SlotParam.Pid);

	DMX_DBG_PRINT("---------------------------------------------------------\n");

	DMX_DBG_PRINT("CallbackCnt:%u\n", SecSlotStatInfo.CallbackCnt);

	if (SecSlotStatInfo.CbTimeOutCnt != 0)
	{
		DMX_DBG_PRINT("CbTimeOutCnt:%u\n", SecSlotStatInfo.CbTimeOutCnt);
	}
	
	if (SecSlotStatInfo.StatErrCnt != 0)
	{
		DMX_DBG_PRINT("StatErrCnt:%u\n", SecSlotStatInfo.StatErrCnt);
	}
	
	if (SecSlotStatInfo.NoSecBufCnt != 0)
	{
		DMX_DBG_PRINT("NoSecBufCnt:%u\n", SecSlotStatInfo.NoSecBufCnt);
	}
	
	if (SecSlotStatInfo.CallbackErrCnt != 0)
	{
		DMX_DBG_PRINT("CallbackErrCnt:%u\n", SecSlotStatInfo.CallbackErrCnt);
	}
	
	if (SecSlotStatInfo.ThreadCreatErrCnt != 0)
	{
		DMX_DBG_PRINT("ThreadCreatErrCnt:%u\n", SecSlotStatInfo.ThreadCreatErrCnt);
	}

	if (SecSlotStatInfo.StrmOpenFailCnt != 0)
	{
		DMX_DBG_PRINT("StrmOpenFailCnt:%u\n", SecSlotStatInfo.StrmOpenFailCnt);
	}

	if (SecSlotStatInfo.StrmCfgFailCnt != 0)
	{
		DMX_DBG_PRINT("StrmCfgFailCnt:%u\n", SecSlotStatInfo.StrmCfgFailCnt);
	}

	if (SecSlotStatInfo.StrmStartFailCnt != 0)
	{
		DMX_DBG_PRINT("StrmStartFailCnt:%u\n", SecSlotStatInfo.StrmStartFailCnt);
	}

	if (SecSlotStatInfo.StrmStartFailCnt != 0)
	{
		DMX_DBG_PRINT("StrmStartFailCnt:%u\n", SecSlotStatInfo.StrmStartFailCnt);
	}

	if (SecSlotStatInfo.StrmCloseFailCnt != 0)
	{
		DMX_DBG_PRINT("StrmCloseFailCnt:%u\n", SecSlotStatInfo.StrmCloseFailCnt);
	}	

	return;
}

static void PrintSecFltStatInfoHld(struct Ali_DmxSecFltInfo* FltInfo)
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
	
	DMX_DBG_PRINT("\nSection Filter Statistics FltIdx:%u ChIdx:%u SlotId:0x%x (HLD Part)\n", 
		FltInfo->Idx, FltInfo->ChIdx, FltInfo->SecSlotId);

	DMX_DBG_PRINT("----------------------------------------------------------------------\n");

	DMX_DBG_PRINT("CallbackCnt:%u\n", SecFltStatInfo.CallbackCnt);

	if (SecFltStatInfo.StatErrCnt != 0)
	{
		DMX_DBG_PRINT("StatErrCnt:%u\n", SecFltStatInfo.StatErrCnt);
	}
	
	if (SecFltStatInfo.SlotOpenFailCnt != 0)
	{
		DMX_DBG_PRINT("SlotOpenFailCnt:%u\n", SecFltStatInfo.SlotOpenFailCnt);
	}
	
	if (SecFltStatInfo.SlotCfgFailCnt != 0)
	{
		DMX_DBG_PRINT("SlotCfgFailCnt:%u\n", SecFltStatInfo.SlotCfgFailCnt);
	}
	
	if (SecFltStatInfo.SlotStartFailCnt != 0)
	{
		DMX_DBG_PRINT("SlotStartFailCnt:%u\n", SecFltStatInfo.SlotStartFailCnt);
	}
	
	if (SecFltStatInfo.SlotStopFailCnt != 0)
	{
		DMX_DBG_PRINT("SlotStopFailCnt:%u\n", SecFltStatInfo.SlotStopFailCnt);
	}

	if (SecFltStatInfo.SlotCloseFailCnt != 0)
	{
		DMX_DBG_PRINT("SlotCloseFailCnt:%u\n", SecFltStatInfo.SlotCloseFailCnt);
	}

	return;
}

static void PrintSecChStatInfoHld(struct Ali_DmxSecChInfo* ChInfo)
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
	
	DMX_DBG_PRINT("\nSection Channel Statistics ChIdx:%u PID:%u (HLD Part)\n", 
		ChInfo->Idx, ChInfo->ChParam.Pid);

	DMX_DBG_PRINT("---------------------------------------------------\n");
	
	if (SecChStatInfo.StatErrCnt != 0)
	{
		DMX_DBG_PRINT("StatErrCnt:%u\n", SecChStatInfo.StatErrCnt);
	}
	
	if (SecChStatInfo.DupFltCnt != 0)
	{
		DMX_DBG_PRINT("DupFltCnt:%u\n", SecChStatInfo.DupFltCnt);
	}
	
	if (SecChStatInfo.DupPidCnt != 0)
	{
		DMX_DBG_PRINT("DupPidCnt:%u\n", SecChStatInfo.DupPidCnt);
	}

	return;
}

static INT32 PrintSectionStatInfoKernel(struct Ali_DmxSecStreamInfo* StreamInfo, UINT32 SysTickInc)
{
	INT32								Ret;
	UINT32								Bitrate;
    UINT32								TsInCntInc;
	static UINT32						LastTsInCnt;
	struct Ali_DmxDrvSecStrmStatInfo	SecStrmStatInfo;
	struct Ali_DmxDrvTsFltStatInfo		TsFltStatInfo;
	struct Ali_DmxDrvSecFltStatInfo		SecFltStatInfo;

	if (NULL == StreamInfo)
	{
		return (-1);
	}
	
	memset(&TsFltStatInfo, 0, sizeof(TsFltStatInfo));

	memset(&SecStrmStatInfo, 0, sizeof(SecStrmStatInfo));

	memset(&SecFltStatInfo, 0, sizeof(SecFltStatInfo));

	Ret = ioctl(StreamInfo->Fd, ALI_DMX_TS_FILTER_INFO_GET, &TsFltStatInfo);

	if (Ret < 0)
	{
		DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	Ret = ioctl(StreamInfo->Fd, ALI_DMX_SEC_STREAM_INFO_GET, &SecStrmStatInfo);

	if (Ret < 0)
	{
		DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}	

	Ret = ioctl(StreamInfo->Fd, ALI_DMX_SEC_FILTER_INFO_GET, &SecFltStatInfo);

	if (Ret < 0)
	{
		DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	DMX_DBG_PRINT("\nSection Statistics StrIdx:%u TsFltIdx:%u SecFltIdx:%u PID:%u (Kernel Part)\n", 
		StreamInfo->Idx, TsFltStatInfo.TsFltIdx, SecFltStatInfo.SecFltIdx, StreamInfo->StreamParam.Pid);

	if (SysTickInc != 0 && TsFltStatInfo.TsInCnt > LastTsInCnt)
	{
		TsInCntInc = TsFltStatInfo.TsInCnt - LastTsInCnt;
		Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
		DMX_DBG_PRINT("Bitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
	}
	LastTsInCnt = TsFltStatInfo.TsInCnt;

	DMX_DBG_PRINT("--------------------------------------------------------------------------------\n");

	if (SecStrmStatInfo.CbTypeErrCnt != 0)
	{
		DMX_DBG_PRINT("CbTypeErrCnt:%u\n", SecStrmStatInfo.CbTypeErrCnt);
	}

	if (SecStrmStatInfo.StrTypeErrCnt != 0)
	{
		DMX_DBG_PRINT("StrTypeErrCnt:%u\n", SecStrmStatInfo.StrTypeErrCnt);
	}

	if (SecStrmStatInfo.StatErrCnt != 0)
	{
		DMX_DBG_PRINT("StatErrCnt:%u\n", SecStrmStatInfo.StatErrCnt);
	}

	if (SecStrmStatInfo.WrByteErrCnt != 0)
	{
		DMX_DBG_PRINT("WrByteErrCnt:%u\n", SecStrmStatInfo.WrByteErrCnt);
	}

	if (SecStrmStatInfo.RdBufShortCnt != 0)
	{
		DMX_DBG_PRINT("RdBufShortCnt:%u\n", SecStrmStatInfo.RdBufShortCnt);
	}

	if (SecStrmStatInfo.CopyErrCnt != 0)
	{
		DMX_DBG_PRINT("CopyErrCnt:%u\n", SecStrmStatInfo.CopyErrCnt);
	}

	if (SecStrmStatInfo.CfgFailCnt != 0)
	{
		DMX_DBG_PRINT("CfgFailCnt:%u\n", SecStrmStatInfo.CfgFailCnt);
	}

	if (SecStrmStatInfo.StartFailCnt != 0)
	{
		DMX_DBG_PRINT("StartFailCnt:%u\n", SecStrmStatInfo.StartFailCnt);
	}

	if (SecStrmStatInfo.StopFailCnt != 0)
	{
		DMX_DBG_PRINT("StopFailCnt:%u\n", SecStrmStatInfo.StopFailCnt);
	}

	if (SecStrmStatInfo.CloseFailCnt != 0)
	{
		DMX_DBG_PRINT("CloseFailCnt:%u\n", SecStrmStatInfo.CloseFailCnt);
	}

	DMX_DBG_PRINT("TsInCnt:%u\n", TsFltStatInfo.TsInCnt);

	DMX_DBG_PRINT("TsScrmbCnt:%u\n", TsFltStatInfo.TsScrmbCnt);

	if (TsFltStatInfo.TsSyncByteErrCnt != 0)
	{
		DMX_DBG_PRINT("TsSyncByteErrCnt:%u\n", TsFltStatInfo.TsSyncByteErrCnt);
	}
	
	if (TsFltStatInfo.TsErrCnt != 0)
	{
		DMX_DBG_PRINT("TsErrCnt:%u\n", TsFltStatInfo.TsErrCnt);
	}

	if (TsFltStatInfo.TsDupCnt != 0)
	{
		DMX_DBG_PRINT("TsDupCnt:%u\n", TsFltStatInfo.TsDupCnt);
	}

	if (TsFltStatInfo.TsLostCnt != 0)
	{
		DMX_DBG_PRINT("TsLostCnt:%u\n", TsFltStatInfo.TsLostCnt);
	}

	DMX_DBG_PRINT("SecInCnt:%u\n", SecFltStatInfo.SecInCnt);

	DMX_DBG_PRINT("SecOutCnt:%u\n", SecFltStatInfo.SecOutCnt);

	if (SecFltStatInfo.SecTsNoPayloadCnt != 0)
	{
		DMX_DBG_PRINT("SecTsNoPayloadCnt:%u\n", SecFltStatInfo.SecTsNoPayloadCnt);
	}

	if (SecFltStatInfo.SecTsScrmbCnt != 0)
	{
		DMX_DBG_PRINT("SecTsScrmbCnt:%u\n", SecFltStatInfo.SecTsScrmbCnt);
	}
				
	if (SecFltStatInfo.SecTsLostCnt != 0)
	{
		DMX_DBG_PRINT("SecTsLostCnt:%u\n", SecFltStatInfo.SecTsLostCnt);
	}
				
	if (SecFltStatInfo.SecPtErrCnt != 0)
	{
		DMX_DBG_PRINT("SecPtErrCnt:%u\n", SecFltStatInfo.SecPtErrCnt);
	}
				
	if (SecFltStatInfo.SecHdrTooShortCnt != 0)
	{
		DMX_DBG_PRINT("SecHdrTooShortCnt:%u\n", SecFltStatInfo.SecHdrTooShortCnt);
	}
				
	if (SecFltStatInfo.SecDataNotFullCnt != 0)
	{
		DMX_DBG_PRINT("SecDataNotFullCnt:%u\n", SecFltStatInfo.SecDataNotFullCnt);
	}
				
	if (SecFltStatInfo.SecDataTooShortCnt != 0)
	{
		DMX_DBG_PRINT("SecDataTooShortCnt:%u\n", SecFltStatInfo.SecDataTooShortCnt);
	}
				
	if (SecFltStatInfo.SecMaskTooLongCnt != 0)
	{
		DMX_DBG_PRINT("SecMaskTooLongCnt:%u\n", SecFltStatInfo.SecMaskTooLongCnt);		
	}
				
	if (SecFltStatInfo.SecMaskMismatchCnt != 0)
	{
		DMX_DBG_PRINT("SecMaskMismatchCnt:%u\n", SecFltStatInfo.SecMaskMismatchCnt);
	}
				
	if (SecFltStatInfo.SecBufOverflowCnt != 0)
	{
		DMX_DBG_PRINT("SecBufOverflowCnt:%u\n", SecFltStatInfo.SecBufOverflowCnt);
	}
				
	return (0);
}

static void PrintVideoStatInfoHld(struct Ali_DmxVideoStreamInfo* StreamInfo)
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

	DMX_DBG_PRINT("\nVideo Statistics StrIdx:%u PID:%u (HLD Part)\n", 
		StreamInfo->Idx, StreamInfo->StreamParam.Pid);

	DMX_DBG_PRINT("-------------------------------------------------\n");

	if (VideoStatInfo.InvPathCnt != 0)
	{
		DMX_DBG_PRINT("InvPathCnt:%u\n", VideoStatInfo.InvPathCnt);
	}

	if (VideoStatInfo.IoOpenFailCnt != 0)
	{
		DMX_DBG_PRINT("IoOpenFailCnt:%u\n", VideoStatInfo.IoOpenFailCnt);
	}

	if (VideoStatInfo.IoCfgFailCnt != 0)
	{
		DMX_DBG_PRINT("IoCfgFailCnt:%u\n", VideoStatInfo.IoCfgFailCnt);
	}

	if (VideoStatInfo.IoStartFailCnt != 0)
	{
		DMX_DBG_PRINT("IoStartFailCnt:%u\n", VideoStatInfo.IoStartFailCnt);
	}

	if (VideoStatInfo.IoStopFailCnt != 0)
	{
		DMX_DBG_PRINT("IoStopFailCnt:%u\n", VideoStatInfo.IoStopFailCnt);
	}
	
	return;
}

static INT32 PrintVideoStatInfoKernel(struct Ali_DmxVideoStreamInfo* StreamInfo, UINT32 SysTickInc)
{
	INT32								Ret;
	UINT32								Bitrate;
    UINT32								TsInCntInc;
	static UINT32						LastTsInCnt;
	struct Ali_DmxDrvTsFltStatInfo		TsFltStatInfo;
	struct Ali_DmxDrvVideoStrmStatInfo	VideoStrmStatInfo;

	if (NULL == StreamInfo)
	{
		return (-1);
	}

	memset(&VideoStrmStatInfo, 0, sizeof(VideoStrmStatInfo));
	
	memset(&TsFltStatInfo, 0, sizeof(TsFltStatInfo));

	Ret = ioctl(StreamInfo->Fd, ALI_DMX_VIDEO_STREAM_INFO_GET, &VideoStrmStatInfo);

	if (Ret < 0)
	{
		DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}	

	Ret = ioctl(StreamInfo->Fd, ALI_DMX_VIDEO_FILTER_INFO_GET, &TsFltStatInfo);

	if (Ret < 0)
	{
		DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	DMX_DBG_PRINT("\nVideo Statistics StrIdx:%u TsFltIdx:%u PID:%u (Kernel Part)\n", 
		StreamInfo->Idx, TsFltStatInfo.TsFltIdx, StreamInfo->StreamParam.Pid);

	if (SysTickInc != 0 && TsFltStatInfo.TsInCnt > LastTsInCnt)
	{
		TsInCntInc = TsFltStatInfo.TsInCnt - LastTsInCnt;
		Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
		DMX_DBG_PRINT("Bitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
	}
	LastTsInCnt= TsFltStatInfo.TsInCnt;
	
	DMX_DBG_PRINT("----------------------------------------------------------\n");

	if (VideoStrmStatInfo.StatErrCnt != 0)
	{
		DMX_DBG_PRINT("StatErrCnt:%u\n", VideoStrmStatInfo.StatErrCnt);
	}

	if (VideoStrmStatInfo.StrTypeErrCnt != 0)
	{
		DMX_DBG_PRINT("StrTypeErrCnt:%u\n", VideoStrmStatInfo.StrTypeErrCnt);
	}

	if (VideoStrmStatInfo.CopyErrCnt != 0)
	{
		DMX_DBG_PRINT("CopyErrCnt:%u\n", VideoStrmStatInfo.CopyErrCnt);
	}

	if (VideoStrmStatInfo.CfgFailCnt != 0)
	{
		DMX_DBG_PRINT("CfgFailCnt:%u\n", VideoStrmStatInfo.CfgFailCnt);
	}

	if (VideoStrmStatInfo.StartFailCnt != 0)
	{
		DMX_DBG_PRINT("StartFailCnt:%u\n", VideoStrmStatInfo.StartFailCnt);
	}

	if (VideoStrmStatInfo.StopFailCnt != 0)
	{
		DMX_DBG_PRINT("StopFailCnt:%u\n", VideoStrmStatInfo.StopFailCnt);
	}

	if (VideoStrmStatInfo.CloseFailCnt != 0)
	{
		DMX_DBG_PRINT("CloseFailCnt:%u\n", VideoStrmStatInfo.CloseFailCnt);
	}

	DMX_DBG_PRINT("TsInCnt:%u\n", TsFltStatInfo.TsInCnt);

	DMX_DBG_PRINT("TsScrmbCnt:%u\n", TsFltStatInfo.TsScrmbCnt);

	if (TsFltStatInfo.TsSyncByteErrCnt != 0)
	{
		DMX_DBG_PRINT("TsSyncByteErrCnt:%u\n", TsFltStatInfo.TsSyncByteErrCnt);
	}
	
	if (TsFltStatInfo.TsErrCnt != 0)
	{
		DMX_DBG_PRINT("TsErrCnt:%u\n", TsFltStatInfo.TsErrCnt);
	}
	
	if (TsFltStatInfo.TsDupCnt != 0)
	{
		DMX_DBG_PRINT("TsDupCnt:%u\n", TsFltStatInfo.TsDupCnt);
	}
	
	if (TsFltStatInfo.TsLostCnt != 0)
	{
		DMX_DBG_PRINT("TsLostCnt:%u\n", TsFltStatInfo.TsLostCnt);
	}
	
	return (0);
}

static INT32 PrintVideoStatInfoSee(struct Ali_DmxVideoStreamInfo* StreamInfo)
{
	INT32							Ret;
	struct Ali_DmxSeePlyChStatInfo	PesStatInfo;

	if (NULL == StreamInfo)
	{
		return (-1);
	}
	
	memset(&PesStatInfo, 0, sizeof(PesStatInfo));
				
	Ret = ioctl(StreamInfo->Fd, ALI_DMX_VIDEO_SEE_INFO_GET, &PesStatInfo);

	if (Ret < 0)
	{
		DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	DMX_DBG_PRINT("\nVideo Statistics PID:%u (SEE Part)\n", PesStatInfo.PlyChPid);

	DMX_DBG_PRINT("------------------------------------\n");

	DMX_DBG_PRINT("TsInCnt:%u\n", PesStatInfo.TsInCnt);

	DMX_DBG_PRINT("PesHdrCnt:%u\n", PesStatInfo.PesHdrCnt);

	if (PesStatInfo.TsPlayBusyCnt != 0)
	{
		DMX_DBG_PRINT("TsPlayBusyCnt:%u\n", PesStatInfo.TsPlayBusyCnt);
	}

	if (PesStatInfo.TsScrmbCnt != 0)
	{
		DMX_DBG_PRINT("TsScrmbCnt:%u\n", PesStatInfo.TsScrmbCnt);
	}

	if (PesStatInfo.TsErrCnt != 0)
	{
		DMX_DBG_PRINT("TsErrCnt:%u\n", PesStatInfo.TsErrCnt);
	}

	if (PesStatInfo.PesTsErrCnt != 0)
	{
		DMX_DBG_PRINT("PesTsErrCnt:%u\n", PesStatInfo.PesTsErrCnt);
	}
			
	if (PesStatInfo.PesTsScrmbCnt != 0)
	{
		DMX_DBG_PRINT("PesTsScrmbCnt:%u\n", PesStatInfo.PesTsScrmbCnt);
	}
			
	if (PesStatInfo.PesTsDupCnt != 0)
	{
		DMX_DBG_PRINT("PesTsDupCnt:%u\n", PesStatInfo.PesTsDupCnt);
	}

	if (PesStatInfo.PesTsLostCnt != 0)
	{
		DMX_DBG_PRINT("PesTsLostCnt:%u\n", PesStatInfo.PesTsLostCnt);
	}

	if (PesStatInfo.PesHdrLenErrCnt != 0)
	{
		DMX_DBG_PRINT("PesHdrLenErrCnt:%u\n", PesStatInfo.PesHdrLenErrCnt);
	}

	if (PesStatInfo.PesHdrScErrCnt != 0)
	{
		DMX_DBG_PRINT("PesHdrScErrCnt:%u\n", PesStatInfo.PesHdrScErrCnt);
	}

	if (PesStatInfo.PesStreamIdErrCnt != 0)
	{
		DMX_DBG_PRINT("PesStreamIdErrCnt:%u\n", PesStatInfo.PesStreamIdErrCnt);
	}
			
	if (PesStatInfo.PesScrmbCnt != 0)
	{
		DMX_DBG_PRINT("PesScrmbCnt:%u\n", PesStatInfo.PesScrmbCnt);
	}

	if (PesStatInfo.PesHdrPayloadLenErrCnt != 0)
	{
		DMX_DBG_PRINT("PesHdrPayloadLenErr:%u\n", PesStatInfo.PesHdrPayloadLenErrCnt);
	}

	if (PesStatInfo.PesCallbackNobufCnt != 0)
	{
		DMX_DBG_PRINT("PesCallbackNobufCnt:%u\n", PesStatInfo.PesCallbackNobufCnt);
	}

	if (PesStatInfo.PesReqBufBusyCnt != 0)
	{
		DMX_DBG_PRINT("PesReqBufBusyCnt:%u\n", PesStatInfo.PesReqBufBusyCnt);
	}

	if (PesStatInfo.PesReqDecStateErrCnt != 0)
	{
		DMX_DBG_PRINT("PesReqDecStateErrCnt:%u\n", PesStatInfo.PesReqDecStateErrCnt);
	}

	if (PesStatInfo.PesTsNoPayloadCnt != 0)
	{
		DMX_DBG_PRINT("PesTsNoPayloadCnt:%u\n", PesStatInfo.PesTsNoPayloadCnt);
	}

	if (PesStatInfo.PesBufOverflowCnt != 0)
	{
		DMX_DBG_PRINT("PesBufOverflowCnt:%u\n", PesStatInfo.PesBufOverflowCnt);
	}

	DMX_DBG_PRINT("PesBufReqCnt:%u\n", PesStatInfo.PesBufReqCnt);

	DMX_DBG_PRINT("PesBufUpdateCnt:%u\n", PesStatInfo.PesBufUpdateCnt);

	return (0);
}

static void PrintAudioStatInfoHld(struct Ali_DmxAudioStreamInfo* StreamInfo)
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
	
	DMX_DBG_PRINT("\nAudio Statistics StrIdx:%u PID:%u (HLD Part)\n", 
		StreamInfo->Idx, StreamInfo->StreamParam.Pid);

	DMX_DBG_PRINT("-------------------------------------------------\n");


	if (AudioStatInfo.InvPathCnt != 0)
	{
		DMX_DBG_PRINT("InvPathCnt:%u\n", AudioStatInfo.InvPathCnt);
	}

	if (AudioStatInfo.IoOpenFailCnt != 0)
	{
		DMX_DBG_PRINT("IoOpenFailCnt:%u\n", AudioStatInfo.IoOpenFailCnt);
	}

	if (AudioStatInfo.IoCfgFailCnt != 0)
	{
		DMX_DBG_PRINT("IoCfgFailCnt:%u\n", AudioStatInfo.IoCfgFailCnt);
	}

	if (AudioStatInfo.IoStartFailCnt != 0)
	{
		DMX_DBG_PRINT("IoStartFailCnt:%u\n", AudioStatInfo.IoStartFailCnt);
	}

	if (AudioStatInfo.IoStopFailCnt != 0)
	{
		DMX_DBG_PRINT("IoStopFailCnt:%u\n", AudioStatInfo.IoStopFailCnt);
	}
	
	return;
}

static INT32 PrintAudioStatInfoKernel(struct Ali_DmxAudioStreamInfo* StreamInfo, UINT32 SysTickInc)
{
	INT32								Ret;
	UINT32								Bitrate;
    UINT32								TsInCntInc;
	static UINT32						LastTsInCnt;
	struct Ali_DmxDrvTsFltStatInfo		TsFltStatInfo;
	struct Ali_DmxDrvAudioStrmStatInfo	AudioStrmStatInfo;

	if (NULL == StreamInfo)
	{
		return (-1);
	}
	
	memset(&AudioStrmStatInfo, 0, sizeof(AudioStrmStatInfo));

	memset(&TsFltStatInfo, 0, sizeof(TsFltStatInfo));

	Ret = ioctl(StreamInfo->Fd, ALI_DMX_AUDIO_STREAM_INFO_GET, &AudioStrmStatInfo);

	if (Ret < 0)
	{
		DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	Ret = ioctl(StreamInfo->Fd, ALI_DMX_AUDIO_FILTER_INFO_GET, &TsFltStatInfo);

	if (Ret < 0)
	{
		DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	DMX_DBG_PRINT("\nAudio Statistics StrIdx:%u TsFltIdx:%u PID:%u (Kernel Part)\n", 
		StreamInfo->Idx, TsFltStatInfo.TsFltIdx, StreamInfo->StreamParam.Pid);

	if (SysTickInc != 0 && TsFltStatInfo.TsInCnt > LastTsInCnt)
	{
		TsInCntInc = TsFltStatInfo.TsInCnt - LastTsInCnt;
		Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
		DMX_DBG_PRINT("Bitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
	}
	LastTsInCnt= TsFltStatInfo.TsInCnt;
	
	DMX_DBG_PRINT("----------------------------------------------------------\n");

	if (AudioStrmStatInfo.StatErrCnt != 0)
	{
		DMX_DBG_PRINT("StatErrCnt:%u\n", AudioStrmStatInfo.StatErrCnt);
	}

	if (AudioStrmStatInfo.StrTypeErrCnt != 0)
	{
		DMX_DBG_PRINT("StrTypeErrCnt:%u\n", AudioStrmStatInfo.StrTypeErrCnt);
	}

	if (AudioStrmStatInfo.CopyErrCnt != 0)
	{
		DMX_DBG_PRINT("CopyErrCnt:%u\n", AudioStrmStatInfo.CopyErrCnt);
	}

	if (AudioStrmStatInfo.CfgFailCnt != 0)
	{
		DMX_DBG_PRINT("CfgFailCnt:%u\n", AudioStrmStatInfo.CfgFailCnt);
	}

	if (AudioStrmStatInfo.StartFailCnt != 0)
	{
		DMX_DBG_PRINT("StartFailCnt:%u\n", AudioStrmStatInfo.StartFailCnt);
	}

	if (AudioStrmStatInfo.StopFailCnt != 0)
	{
		DMX_DBG_PRINT("StopFailCnt:%u\n", AudioStrmStatInfo.StopFailCnt);
	}

	if (AudioStrmStatInfo.CloseFailCnt != 0)
	{
		DMX_DBG_PRINT("CloseFailCnt:%u\n", AudioStrmStatInfo.CloseFailCnt);
	}

	DMX_DBG_PRINT("TsInCnt:%u\n", TsFltStatInfo.TsInCnt);

	DMX_DBG_PRINT("TsScrmbCnt:%u\n", TsFltStatInfo.TsScrmbCnt);

	if (TsFltStatInfo.TsSyncByteErrCnt != 0)
	{
		DMX_DBG_PRINT("TsSyncByteErrCnt:%u\n", TsFltStatInfo.TsSyncByteErrCnt);
	}
	if (TsFltStatInfo.TsErrCnt != 0)
	{
		DMX_DBG_PRINT("TsErrCnt:%u\n", TsFltStatInfo.TsErrCnt);
	}
	if (TsFltStatInfo.TsDupCnt != 0)
	{
		DMX_DBG_PRINT("TsDupCnt:%u\n", TsFltStatInfo.TsDupCnt);
	}
	if (TsFltStatInfo.TsLostCnt != 0)
	{
		DMX_DBG_PRINT("TsLostCnt:%u\n", TsFltStatInfo.TsLostCnt);
	}
			
	return (0);
}

static INT32 PrintAudioStatInfoSee(struct Ali_DmxAudioStreamInfo* StreamInfo)
{
	INT32							Ret;
	struct Ali_DmxSeePlyChStatInfo	PesStatInfo;

	if (NULL == StreamInfo)
	{
		return (-1);
	}
	
	memset(&PesStatInfo, 0, sizeof(PesStatInfo));
				
	Ret = ioctl(StreamInfo->Fd, ALI_DMX_AUDIO_SEE_INFO_GET, &PesStatInfo);

	if (Ret < 0)
	{
		DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	DMX_DBG_PRINT("\nAudio Statistics PID:%u (SEE Part)\n", PesStatInfo.PlyChPid);

	DMX_DBG_PRINT("------------------------------------\n");

	DMX_DBG_PRINT("TsInCnt:%u\n", PesStatInfo.TsInCnt);

	DMX_DBG_PRINT("PesHdrCnt:%u\n", PesStatInfo.PesHdrCnt);

	if (PesStatInfo.TsPlayBusyCnt != 0)
	{
		DMX_DBG_PRINT("TsPlayBusyCnt:%u\n", PesStatInfo.TsPlayBusyCnt);
	}

	if (PesStatInfo.TsScrmbCnt != 0)
	{
		DMX_DBG_PRINT("TsScrmbCnt:%u\n", PesStatInfo.TsScrmbCnt);
	}

	if (PesStatInfo.TsErrCnt != 0)
	{
		DMX_DBG_PRINT("TsErrCnt:%u\n", PesStatInfo.TsErrCnt);
	}

	if (PesStatInfo.PesTsErrCnt != 0)
	{
		DMX_DBG_PRINT("PesTsErrCnt:%u\n", PesStatInfo.PesTsErrCnt);
	}
			
	if (PesStatInfo.PesTsScrmbCnt != 0)
	{
		DMX_DBG_PRINT("PesTsScrmbCnt:%u\n", PesStatInfo.PesTsScrmbCnt);
	}
			
	if (PesStatInfo.PesTsDupCnt != 0)
	{
		DMX_DBG_PRINT("PesTsDupCnt:%u\n", PesStatInfo.PesTsDupCnt);
	}

	if (PesStatInfo.PesTsLostCnt != 0)
	{
		DMX_DBG_PRINT("PesTsLostCnt:%u\n", PesStatInfo.PesTsLostCnt);
	}

	if (PesStatInfo.PesHdrLenErrCnt != 0)
	{
		DMX_DBG_PRINT("PesHdrLenErrCnt:%u\n", PesStatInfo.PesHdrLenErrCnt);
	}

	if (PesStatInfo.PesHdrScErrCnt != 0)
	{
		DMX_DBG_PRINT("PesHdrScErrCnt:%u\n", PesStatInfo.PesHdrScErrCnt);
	}

	if (PesStatInfo.PesStreamIdErrCnt != 0)
	{
		DMX_DBG_PRINT("PesStreamIdErrCnt:%u\n", PesStatInfo.PesStreamIdErrCnt);
	}
			
	if (PesStatInfo.PesScrmbCnt != 0)
	{
		DMX_DBG_PRINT("PesScrmbCnt:%u\n", PesStatInfo.PesScrmbCnt);
	}

	if (PesStatInfo.PesHdrPayloadLenErrCnt != 0)
	{
		DMX_DBG_PRINT("PesHdrPayloadLenErrCnt:%u\n", PesStatInfo.PesHdrPayloadLenErrCnt);
	}

	if (PesStatInfo.PesCallbackNobufCnt != 0)
	{
		DMX_DBG_PRINT("PesCallbackNobufCnt:%u\n", PesStatInfo.PesCallbackNobufCnt);
	}

	if (PesStatInfo.PesReqBufBusyCnt != 0)
	{
		DMX_DBG_PRINT("PesReqBufBusyCnt:%u\n", PesStatInfo.PesReqBufBusyCnt);
	}

	if (PesStatInfo.PesReqDecStateErrCnt != 0)
	{
		DMX_DBG_PRINT("PesReqDecStateErrCnt:%u\n", PesStatInfo.PesReqDecStateErrCnt);
	}

	if (PesStatInfo.PesTsNoPayloadCnt != 0)
	{
		DMX_DBG_PRINT("PesTsNoPayloadCnt:%u\n", PesStatInfo.PesTsNoPayloadCnt);
	}

	if (PesStatInfo.PesBufOverflowCnt != 0)
	{
		DMX_DBG_PRINT("PesBufOverflowCnt:%u\n", PesStatInfo.PesBufOverflowCnt);
	}

	DMX_DBG_PRINT("PesBufReqCnt:%u\n", PesStatInfo.PesBufReqCnt);

	DMX_DBG_PRINT("PesBufUpdateCnt:%u\n", PesStatInfo.PesBufUpdateCnt);

	return (0);
}

static void PrintPcrStatInfoHld(struct Ali_DmxPcrStreamInfo* StreamInfo)
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

	DMX_DBG_PRINT("\nPCR Statistics StrIdx:%u PID:%u (HLD Part)\n", 
		StreamInfo->Idx, StreamInfo->StreamParam.Pid);

	DMX_DBG_PRINT("-------------------------------------------------\n");


	if (PcrStatInfo.InvPathCnt != 0)
	{
		DMX_DBG_PRINT("InvPathCnt:%u\n", PcrStatInfo.InvPathCnt);
	}

	if (PcrStatInfo.IoOpenFailCnt != 0)
	{
		DMX_DBG_PRINT("IoOpenFailCnt:%u\n", PcrStatInfo.IoOpenFailCnt);
	}

	if (PcrStatInfo.IoCfgFailCnt != 0)
	{
		DMX_DBG_PRINT("IoCfgFailCnt:%u\n", PcrStatInfo.IoCfgFailCnt);
	}

	if (PcrStatInfo.IoStartFailCnt != 0)
	{
		DMX_DBG_PRINT("IoStartFailCnt:%u\n", PcrStatInfo.IoStartFailCnt);
	}

	if (PcrStatInfo.IoStopFailCnt != 0)
	{
		DMX_DBG_PRINT("IoStopFailCnt:%u\n", PcrStatInfo.IoStopFailCnt);
	}
	
	return;
}

static INT32 PrintPcrStatInfoKernel(struct Ali_DmxPcrStreamInfo* StreamInfo, UINT32 SysTickInc)
{
	INT32								Ret;
	UINT32								Bitrate;
    UINT32								TsInCntInc;
	static UINT32						LastTsInCnt;	
	struct Ali_DmxDrvTsFltStatInfo		TsFltStatInfo;
	struct Ali_DmxDrvPcrStrmStatInfo	PcrStrmStatInfo;

	if (NULL == StreamInfo)
	{
		return (-1);
	}
	
	memset(&PcrStrmStatInfo, 0, sizeof(PcrStrmStatInfo));

	memset(&TsFltStatInfo, 0, sizeof(TsFltStatInfo));

	Ret = ioctl(StreamInfo->Fd, ALI_DMX_PCR_STREAM_INFO_GET, &PcrStrmStatInfo);

	if (Ret < 0)
	{
		DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}
	
	Ret = ioctl(StreamInfo->Fd, ALI_DMX_PCR_FILTER_INFO_GET, &TsFltStatInfo);

	if (Ret < 0)
	{
		DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

	DMX_DBG_PRINT("\nPCR Statistics StrIdx:%u TsFltIdx:%u PID:%u (Kernel Part)\n", 
		StreamInfo->Idx, TsFltStatInfo.TsFltIdx, StreamInfo->StreamParam.Pid);

	if (SysTickInc != 0 && TsFltStatInfo.TsInCnt > LastTsInCnt)
	{
		TsInCntInc = TsFltStatInfo.TsInCnt - LastTsInCnt;
		Bitrate = (TsInCntInc * 188 * 8 / SysTickInc) * 1000;
		DMX_DBG_PRINT("Bitrate:%ubps,TsInc:%u,TickInc:%u\n", Bitrate, TsInCntInc, SysTickInc);
	}
	LastTsInCnt= TsFltStatInfo.TsInCnt;
	
	DMX_DBG_PRINT("------------------------------------------------------------\n");

	if (PcrStrmStatInfo.StatErrCnt != 0)
	{
		DMX_DBG_PRINT("StatErrCnt:%u\n", PcrStrmStatInfo.StatErrCnt);
	}

	if (PcrStrmStatInfo.StrTypeErrCnt != 0)
	{
		DMX_DBG_PRINT("StrTypeErrCnt:%u\n", PcrStrmStatInfo.StrTypeErrCnt);
	}

	if (PcrStrmStatInfo.RdByteErrCnt != 0)
	{
		DMX_DBG_PRINT("RdByteErrCnt:%u\n", PcrStrmStatInfo.RdByteErrCnt);
	}

	if (PcrStrmStatInfo.RdBufShortCnt != 0)
	{
		DMX_DBG_PRINT("RdBufShortCnt:%u\n", PcrStrmStatInfo.RdBufShortCnt);
	}
	
	if (PcrStrmStatInfo.CopyErrCnt != 0)
	{
		DMX_DBG_PRINT("CopyErrCnt:%u\n", PcrStrmStatInfo.CopyErrCnt);
	}

	if (PcrStrmStatInfo.CfgFailCnt != 0)
	{
		DMX_DBG_PRINT("CfgFailCnt:%u\n", PcrStrmStatInfo.CfgFailCnt);
	}

	if (PcrStrmStatInfo.StartFailCnt != 0)
	{
		DMX_DBG_PRINT("StartFailCnt:%u\n", PcrStrmStatInfo.StartFailCnt);
	}

	if (PcrStrmStatInfo.StopFailCnt != 0)
	{
		DMX_DBG_PRINT("StopFailCnt:%u\n", PcrStrmStatInfo.StopFailCnt);
	}

	if (PcrStrmStatInfo.CloseFailCnt != 0)
	{
		DMX_DBG_PRINT("CloseFailCnt:%u\n", PcrStrmStatInfo.CloseFailCnt);
	}

	DMX_DBG_PRINT("TsInCnt:%u\n", TsFltStatInfo.TsInCnt);

	DMX_DBG_PRINT("TsScrmbCnt:%u\n", TsFltStatInfo.TsScrmbCnt);

	if (TsFltStatInfo.TsSyncByteErrCnt != 0)
	{
		DMX_DBG_PRINT("TsSyncByteErrCnt:%u\n", TsFltStatInfo.TsSyncByteErrCnt);
	}
	
	if (TsFltStatInfo.TsErrCnt != 0)
	{
		DMX_DBG_PRINT("TsErrCnt:%u\n", TsFltStatInfo.TsErrCnt);
	}
	
	if (TsFltStatInfo.TsDupCnt != 0)
	{
		DMX_DBG_PRINT("TsDupCnt:%u\n", TsFltStatInfo.TsDupCnt);
	}

	if (TsFltStatInfo.TsLostCnt != 0)
	{
		DMX_DBG_PRINT("TsLostCnt:%u\n", TsFltStatInfo.TsLostCnt);
	}

	return (0);
}

static INT32 ShowTsStreamInfo(struct Ali_DmxTsStreamInfo* StreamInfo, INT32 FltIdx)
{
	if (NULL == StreamInfo)
	{
		return (-1);
	}

	DMX_DBG_PRINT("\nTS Stream Info");
	//if (1 == StreamInfo->StreamParam.NeedDiscramble)
		//DMX_DBG_PRINT(" (Scrambled)");
	DMX_DBG_PRINT(":\n--------------------------\n");
	DMX_DBG_PRINT("PID: %u\n", StreamInfo->StreamParam.PidList[FltIdx]);
	DMX_DBG_PRINT("Dmx Id: %u\n", StreamInfo->DmxId);
	DMX_DBG_PRINT("File descriptor: %d\n", StreamInfo->Fd);
	DMX_DBG_PRINT("Stream Id: 0x%x\n", Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_TS, StreamInfo->Idx));
	DMX_DBG_PRINT("Stream Idx: %u\n", StreamInfo->Idx);
	DMX_DBG_PRINT("Filter Id: 0x%x\n", StreamInfo->StreamParam.TsFltId[FltIdx]);

	return (0);
}

static INT32 ShowVideoStreamInfo(struct Ali_DmxVideoStreamInfo* StreamInfo)
{
	if (NULL == StreamInfo)
	{
		return (-1);
	}

	DMX_DBG_PRINT("\nVideo Stream Info");
	if (StreamInfo->StreamParam.NeedDiscramble)
		DMX_DBG_PRINT(" (Scrambled)");
	DMX_DBG_PRINT(":\n--------------------------\n");
	DMX_DBG_PRINT("PID: %u\n", StreamInfo->StreamParam.Pid);
	DMX_DBG_PRINT("Dmx Id: %u\n", StreamInfo->DmxId);
	DMX_DBG_PRINT("File descriptor: %d\n", StreamInfo->Fd);
	DMX_DBG_PRINT("Stream Id: 0x%x\n", Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_VIDEO, StreamInfo->Idx));
	DMX_DBG_PRINT("Stream Idx: %u\n", StreamInfo->Idx);	

	return (0);
}

static INT32 ShowAudioStreamInfo(struct Ali_DmxAudioStreamInfo* StreamInfo)
{
	if (NULL == StreamInfo)
	{
		return (-1);
	}

	DMX_DBG_PRINT("\nAudio Stream Info");
	if (StreamInfo->StreamParam.NeedDiscramble)
		DMX_DBG_PRINT(" (Scrambled)");
	DMX_DBG_PRINT(":\n--------------------------\n");
	DMX_DBG_PRINT("PID: %u\n", StreamInfo->StreamParam.Pid);
	DMX_DBG_PRINT("Dmx Id: %u\n", StreamInfo->DmxId);
	DMX_DBG_PRINT("File descriptor: %d\n", StreamInfo->Fd);
	DMX_DBG_PRINT("Stream Id: 0x%x\n", Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_AUDIO, StreamInfo->Idx));
	DMX_DBG_PRINT("Stream Idx: %u\n", StreamInfo->Idx);

	return (0);
}

static INT32 ShowPcrStreamInfo(struct Ali_DmxPcrStreamInfo* StreamInfo)
{
	if (NULL == StreamInfo)
	{
		return (-1);
	}
	
	DMX_DBG_PRINT("\nPCR Stream Info");
	if (StreamInfo->StreamParam.NeedDiscramble)
		DMX_DBG_PRINT(" (Scrambled)");
	DMX_DBG_PRINT(":\n--------------------------\n");
	DMX_DBG_PRINT("PID: %u\n", StreamInfo->StreamParam.Pid);
	DMX_DBG_PRINT("Dmx Id: %u\n", StreamInfo->DmxId);
	DMX_DBG_PRINT("File descriptor: %d\n", StreamInfo->Fd);
	DMX_DBG_PRINT("Stream Id: 0x%x\n", Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_PCR, StreamInfo->Idx));
	DMX_DBG_PRINT("Stream Idx: %u\n", StreamInfo->Idx);	

	return (0);
}

static INT32 ShowSectionStreamInfo(struct Ali_DmxSecStreamInfo* StreamInfo)
{
	UINT8 i;
	
	if (NULL == StreamInfo)
	{
		return (-1);
	}

	DMX_DBG_PRINT("\nSection Stream Info");
	if (1 == StreamInfo->StreamParam.SecMask.Flags)
		DMX_DBG_PRINT(" (CRC)");	
	if (StreamInfo->StreamParam.NeedDiscramble)
		DMX_DBG_PRINT(" (Scrambled)");	
	DMX_DBG_PRINT(":\n----------------------------------\n");
	DMX_DBG_PRINT("PID: %u\n", StreamInfo->StreamParam.Pid);
	DMX_DBG_PRINT("Dmx Id: %u\n", StreamInfo->DmxId);
	DMX_DBG_PRINT("File descriptor: %d\n", StreamInfo->Fd);
	DMX_DBG_PRINT("Stream id: 0x%x\n", Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_SEC, StreamInfo->Idx));
	DMX_DBG_PRINT("Stream index: %u\n", StreamInfo->Idx);
	DMX_DBG_PRINT("Total byte in: %u\n", StreamInfo->InByteCnt);
	DMX_DBG_PRINT("Mask length: %u\n", StreamInfo->StreamParam.SecMask.MatchLen);
	DMX_DBG_PRINT("Mask:");
	for (i = 0; i < StreamInfo->StreamParam.SecMask.MatchLen; i++)
		DMX_DBG_PRINT(" %02x", StreamInfo->StreamParam.SecMask.Mask[i]);
	DMX_DBG_PRINT("\nMatch:");
	for (i = 0; i < StreamInfo->StreamParam.SecMask.MatchLen; i++)
		DMX_DBG_PRINT(" %02x", StreamInfo->StreamParam.SecMask.Match[i]);
	DMX_DBG_PRINT("\nNegate:");
	for (i = 0; i < StreamInfo->StreamParam.SecMask.MatchLen; i++)
		DMX_DBG_PRINT(" %02x", StreamInfo->StreamParam.SecMask.Negate[i]);
	DMX_DBG_PRINT("\n");

	return (0);
}

static INT32 ShowSectionSlotInfo(struct Ali_DmxSecSlotInfo* SlotInfo)
{
	UINT8 i;
	
	if (NULL == SlotInfo)
	{
		return (-1);
	}

	DMX_DBG_PRINT("\nSection Slot Info");
	if (1 == SlotInfo->SlotParam.SecMask.Flags)
		DMX_DBG_PRINT(" (CRC)");
	if (SlotInfo->SlotParam.NeedDiscramble)
		DMX_DBG_PRINT(" (Scrambled)");	
	DMX_DBG_PRINT(":\n---------------------------------\n");
	DMX_DBG_PRINT("PID: %u\n", SlotInfo->SlotParam.Pid);
	DMX_DBG_PRINT("Dmx Id: %u\n", SlotInfo->DmxId);
	DMX_DBG_PRINT("Slot index: %u\n", SlotInfo->Idx);
	DMX_DBG_PRINT("Stream id: 0x%x\n", SlotInfo->SecStreamId);
	DMX_DBG_PRINT("Total byte in: %u\n", SlotInfo->InByteCnt);
	DMX_DBG_PRINT("Timeout: %u\n", SlotInfo->SlotParam.Timeout);
	DMX_DBG_PRINT("Callback param: %u\n", SlotInfo->SlotParam.CbParam);
	DMX_DBG_PRINT("Tick from last callback: %u\n", SlotInfo->StartTime);
	DMX_DBG_PRINT("Mask length: %u\n", SlotInfo->SlotParam.SecMask.MatchLen);
	DMX_DBG_PRINT("Mask:");
	for (i = 0; i < SlotInfo->SlotParam.SecMask.MatchLen; i++)
		DMX_DBG_PRINT(" %02x", SlotInfo->SlotParam.SecMask.Mask[i]);
	DMX_DBG_PRINT("\nMatch:");
	for (i = 0; i < SlotInfo->SlotParam.SecMask.MatchLen; i++)
		DMX_DBG_PRINT(" %02x", SlotInfo->SlotParam.SecMask.Match[i]);
	DMX_DBG_PRINT("\nNegate:");
	for (i = 0; i < SlotInfo->SlotParam.SecMask.MatchLen; i++)
		DMX_DBG_PRINT(" %02x", SlotInfo->SlotParam.SecMask.Negate[i]);
	DMX_DBG_PRINT("\n");
	
	return (0);
}

static INT32 ShowSectionFltInfo(struct Ali_DmxSecFltInfo* FltInfo)
{
	UINT8 i;
	
	if (NULL == FltInfo)
	{
		return (-1);
	}

	DMX_DBG_PRINT("\nSection Filter Info");
	if (1 == FltInfo->FltParam.SecMask.Flags)
		DMX_DBG_PRINT(" (CRC)");
	DMX_DBG_PRINT(":\n-------------------------------\n");
	DMX_DBG_PRINT("Slot id: 0x%x\n", FltInfo->SecSlotId);
	DMX_DBG_PRINT("Filter index: %u\n", FltInfo->Idx);
	DMX_DBG_PRINT("Channel index: %u\n", FltInfo->ChIdx);
	DMX_DBG_PRINT("Total byte in: %u\n", FltInfo->InByteCnt);
	DMX_DBG_PRINT("Mask length: %u\n", FltInfo->FltParam.SecMask.MatchLen);
	DMX_DBG_PRINT("Mask:");
	for (i = 0; i < FltInfo->FltParam.SecMask.MatchLen; i++)
		DMX_DBG_PRINT(" %02x", FltInfo->FltParam.SecMask.Mask[i]);
	DMX_DBG_PRINT("\nMatch:");
	for (i = 0; i < FltInfo->FltParam.SecMask.MatchLen; i++)
		DMX_DBG_PRINT(" %02x", FltInfo->FltParam.SecMask.Match[i]);
	DMX_DBG_PRINT("\nNegate:");
	for (i = 0; i < FltInfo->FltParam.SecMask.MatchLen; i++)
		DMX_DBG_PRINT(" %02x", FltInfo->FltParam.SecMask.Negate[i]);
	DMX_DBG_PRINT("\n");
	
	return (0);
}

static INT32 ShowSectionChInfo(struct Ali_DmxSecChInfo* ChInfo)
{
	if (NULL == ChInfo)
	{
		return (-1);
	}

	DMX_DBG_PRINT("\nSection Channel Info");
	if (ChInfo->ChParam.NeedDiscramble)
		DMX_DBG_PRINT(" (Scrambled)");	
	DMX_DBG_PRINT(":\n--------------------------\n");
	DMX_DBG_PRINT("PID: %u\n", ChInfo->ChParam.Pid);
	DMX_DBG_PRINT("Dmx Id: %u\n", ChInfo->DmxId);
	DMX_DBG_PRINT("Channel index: %u\n", ChInfo->Idx);
	DMX_DBG_PRINT("Total byte in: %u\n", ChInfo->InByteCnt);

	return (0);
}

INT32 DmxDbgShowHwRegInfo(void)
{
	INT32	Ret;
	UINT32	HwRegTable[18][5];
	UINT8	i, j;

	if (DmxDbgCtrl.HwRegFd < 0)
	{
	    DMX_DBG_PRINT("Invalid DeMux debug FD: %d\n", DmxDbgCtrl.HwRegFd);

		return (-1);
	}
	
	if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
	{
		memset(&HwRegTable, 0, sizeof(HwRegTable));

		Ret = ioctl(DmxDbgCtrl.HwRegFd, ALI_DMX_HW_REG_STREAM_INFO_GET, &HwRegTable);

	    if (Ret < 0)
	    {
	        DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

			return (-1);
	    }

		DMX_DBG_PRINT("\nHardware register dump:\n");

		DMX_DBG_PRINT("------------------------------------\n");
		
		for (i = 0; i < 18; i++)
		{
			if (7 == i || 9 == i || 10 == i || 11 == i)
			{
				DMX_DBG_PRINT("\t...\n");
			}
			
			DMX_DBG_PRINT("0x%08x:", HwRegTable[i][0]);

			for (j = 0; j < 4; j++)
			{
				DMX_DBG_PRINT(" %08x", HwRegTable[i][j + 1]);			
			}

			DMX_DBG_PRINT("\n");
		}
	}
	
	return (0);
}

INT32 DmxDbgShowKernelGlobalInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{
	if (DMX_DBG_SHOW_STATISTICS == info_type)
	{
		PrintKernelGlobalStatInfo(SysTickInc);		
	}

	return (0);
}

INT32 DmxDbgShowSeeGlobalInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{
	if (DMX_DBG_SHOW_STATISTICS == info_type)
	{
		PrintSeeGlobalStatInfo(SysTickInc);
	}

	return (0);
}

INT32 DmxDbgShowTsInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{
	INT32						StrIdx;
	INT32						FltIdx;
	INT32						StrmId;
	INT32						StreamExist;
	struct ALi_DmxSysInfo		DmxSysInfo;
	struct Ali_DmxTsStreamInfo	StreamInfo;

	memset(&DmxSysInfo, 0, sizeof(DmxSysInfo));
		
	if (ALi_DmxSysInfoGet(&DmxSysInfo))
	{
	    DMX_DBG_PRINT("DeMux library not yet initialized!\n");

		return (-1);
	}

	StreamExist = 0;

    for (StrIdx = 0; StrIdx < DmxSysInfo.TsStreamCntTotal; StrIdx++)
    { 
    	StrmId = Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_TS, StrIdx);

		memset(&StreamInfo, 0, sizeof(StreamInfo));

		if (Ali_DmxTsStreamInfoGet(StrmId, &StreamInfo))
		{
		    //DMX_DBG_PRINT("No ts stream in dmx library!\n");

			continue;
		}
		
        if (StreamInfo.Fd > 0 && ALI_DMX_STREAM_STATE_RUN == StreamInfo.State)
        {
        	for (FltIdx = 0; FltIdx < StreamInfo.StreamParam.PidCnt; FltIdx++)
        	{
        		if (DMX_DBG_SHOW_STREAM_INFO & info_type)
        		{
        			ShowTsStreamInfo(&StreamInfo, FltIdx);
        		}
        		else if (DMX_DBG_SHOW_STATISTICS == info_type)
        		{
					if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
					{
						PrintTsStatInfoHld(&StreamInfo, FltIdx);
					}
					
					if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
					{
						PrintTsStatInfoKernel(&StreamInfo, FltIdx, SysTickInc);
					}
        		}
        	}
			
			StreamExist = 1;
		}
    }

	if (!StreamExist)
	{
		DMX_DBG_PRINT("\nNo TS stream opened in DeMux!\n");
	}
	
	return (0);
}

INT32 DmxDbgShowVideoInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{
	INT32							StreamExist;
	struct ALi_DmxSysInfo			DmxSysInfo;
	struct Ali_DmxVideoStreamInfo	StreamInfo;

	memset(&DmxSysInfo, 0, sizeof(DmxSysInfo));
		
	if (ALi_DmxSysInfoGet(&DmxSysInfo))
	{
	    DMX_DBG_PRINT("DeMux library not yet initialized!\n");

		return (-1);
	}

	memset(&StreamInfo, 0, sizeof(StreamInfo));

	if (Ali_DmxVideoStreamInfoGet(ALI_DMX_STREAM_ID_VIDEO_START, &StreamInfo))
	{
	    DMX_DBG_PRINT("\nNo video stream in dmx library!\n");

		return (-1);
	}

	StreamExist = 0;
	
    if (StreamInfo.Fd > 0 && ALI_DMX_STREAM_STATE_RUN == StreamInfo.State)
    {
   		if (DMX_DBG_SHOW_STREAM_INFO & info_type)
   		{
   			ShowVideoStreamInfo(&StreamInfo);
   		}
		else if (DMX_DBG_SHOW_STATISTICS == info_type)
		{
			if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
			{
				PrintVideoStatInfoHld(&StreamInfo);
	    	}
			
			if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
			{
				PrintVideoStatInfoKernel(&StreamInfo, SysTickInc);
	    	}

			if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_SEE)
			{
				PrintVideoStatInfoSee(&StreamInfo);
			}
		}

		StreamExist = 1;
	}

	if (!StreamExist)
	{
		DMX_DBG_PRINT("\nNo Video stream opened in DeMux!\n");
	}
	
	return (0);
}

INT32 DmxDbgShowAudioInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{	
	INT32							StreamExist;
	struct ALi_DmxSysInfo			DmxSysInfo;
	struct Ali_DmxAudioStreamInfo	StreamInfo;

	memset(&DmxSysInfo, 0, sizeof(DmxSysInfo));
		
	if (ALi_DmxSysInfoGet(&DmxSysInfo))
	{
	    DMX_DBG_PRINT("DeMux library not yet initialized!\n");

		return (-1);
	}

	memset(&StreamInfo, 0, sizeof(StreamInfo));

	if (Ali_DmxAudioStreamInfoGet(ALI_DMX_STREAM_ID_AUDIO_START, &StreamInfo))
	{
	    DMX_DBG_PRINT("\nNo audio stream in dmx library!\n");

		return (-1);
	}
	
	StreamExist = 0;

    if (StreamInfo.Fd > 0 && ALI_DMX_STREAM_STATE_RUN == StreamInfo.State)
    {
   		if (DMX_DBG_SHOW_STREAM_INFO & info_type)
   		{
   			ShowAudioStreamInfo(&StreamInfo);
   		}
		else if (DMX_DBG_SHOW_STATISTICS == info_type)
		{    
			if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
			{
				PrintAudioStatInfoHld(&StreamInfo);
	    	}
			
			if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
			{
				PrintAudioStatInfoKernel(&StreamInfo, SysTickInc);
			}

			if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_SEE)
			{		
				PrintAudioStatInfoSee(&StreamInfo);
			}
		}

		StreamExist = 1;
	}

	if (!StreamExist)
	{
		DMX_DBG_PRINT("\nNo Audio stream opened in DeMux!\n");
	}
	
	return (0);
}

INT32 DmxDbgShowPcrInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{
	INT32						StreamExist;
	struct ALi_DmxSysInfo		DmxSysInfo;
	struct Ali_DmxPcrStreamInfo	StreamInfo;

	memset(&DmxSysInfo, 0, sizeof(DmxSysInfo));

	if (ALi_DmxSysInfoGet(&DmxSysInfo))
	{
	    DMX_DBG_PRINT("DeMux library not yet initialized!\n");

		return (-1);
	}

	memset(&StreamInfo, 0, sizeof(StreamInfo));

	if (Ali_DmxPcrStreamInfoGet(ALI_DMX_STREAM_ID_PCR_START, &StreamInfo))
	{
	    DMX_DBG_PRINT("\nNo PCR stream in dmx library!\n");

		return (-1);
	}

	StreamExist = 0;
	
    if (StreamInfo.Fd > 0 && ALI_DMX_STREAM_STATE_RUN == StreamInfo.State)
    {
   		if (DMX_DBG_SHOW_STREAM_INFO & info_type)
   		{
   			ShowPcrStreamInfo(&StreamInfo);
   		}
		else if (DMX_DBG_SHOW_STATISTICS == info_type)
		{    
			if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
			{
				PrintPcrStatInfoHld(&StreamInfo);
	    	}
			
			if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
			{
				PrintPcrStatInfoKernel(&StreamInfo, SysTickInc);
			}
		}

		StreamExist = 1;
	}

	if (!StreamExist)
	{
		DMX_DBG_PRINT("\nNo PCR stream opened in DeMux!\n");
	}
	
	return (0);
}

INT32 DmxDbgShowSectionInfo(DMX_DBG_INFO_TYPE info_type, UINT32 SysTickInc)
{
	INT32						StrIdx;
	INT32						SlotIdx;
	INT32						FltIdx;
	INT32						ChIdx;
	INT32						StrmId;
	INT32						StreamExist;
	struct ALi_DmxSysInfo		DmxSysInfo;
	struct Ali_DmxSecStreamInfo	StreamInfo;
	struct Ali_DmxSecSlotInfo	SlotInfo;
	struct Ali_DmxSecFltInfo	FltInfo;
	struct Ali_DmxSecChInfo		ChInfo;

	memset(&DmxSysInfo, 0, sizeof(DmxSysInfo));
		
	if (ALi_DmxSysInfoGet(&DmxSysInfo))
	{
	    DMX_DBG_PRINT("DeMux library not yet initialized!\n");

		return (-1);
	}

	StreamExist = 0;
	
    for (StrIdx = 0; StrIdx < DmxSysInfo.SecStreamCntTotal; StrIdx++)
    {
    	StrmId = Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_SEC, StrIdx);

		memset(&StreamInfo, 0, sizeof(StreamInfo));

		if (Ali_DmxSecStreamInfoGet(StrmId, &StreamInfo))
		{
		    //DMX_DBG_PRINT("No section stream in dmx library!\n");

			continue;
		}

        if (StreamInfo.Fd > 0 && ALI_DMX_STREAM_STATE_RUN == StreamInfo.State)
        {
	   		if (DMX_DBG_SHOW_STREAM_INFO & info_type)
	   		{
	   			ShowSectionStreamInfo(&StreamInfo);
	   		}
			else if (DMX_DBG_SHOW_STATISTICS == info_type)
			{        
				if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
				{        
					PrintSecStrmStatInfoHld(&StreamInfo);
				}

				if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_KERNEL)
				{        
					PrintSectionStatInfoKernel(&StreamInfo, SysTickInc);
				}
	        }

			StreamExist = 1;
        }
    }

    for (SlotIdx = 0; SlotIdx < DmxSysInfo.SecSlotCntTotal; SlotIdx++)
    {
		memset(&SlotInfo, 0, sizeof(SlotInfo));

    	if (Ali_DmxSecSlotInfoGet(SlotIdx + ALI_DMX_STREAM_ID_SEC_SLOT_START, &SlotInfo))
		{
		    //DMX_DBG_PRINT("No section slot in dmx library!\n");

			continue;
		}

        if (ALI_DMX_STREAM_STATE_RUN == SlotInfo.State)
        {
	   		if (DMX_DBG_SHOW_STREAM_INFO_SEC_SLOT & info_type)
	   		{
	   			ShowSectionSlotInfo(&SlotInfo);
	   		}
			else if (DMX_DBG_SHOW_STATISTICS == info_type)
			{        
				if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
				{        
					PrintSecSlotStatInfoHld(&SlotInfo);
				}
	        }        
        }		
    }

    for (ChIdx = 0; ChIdx < DmxSysInfo.SecChCntTotal; ChIdx++)
    {
		memset(&ChInfo, 0, sizeof(ChInfo));

    	if (Ali_DmxSecChInfoGet(ChIdx + ALI_DMX_STREAM_ID_SEC_CH_START, &ChInfo))
		{
		    //DMX_DBG_PRINT("No section channel in dmx library!\n");

			continue;
		}

        if (ALI_DMX_STREAM_STATE_RUN == ChInfo.State)
        {
	   		if (DMX_DBG_SHOW_STREAM_INFO_SEC_CH & info_type)
	   		{
	   			ShowSectionChInfo(&ChInfo);
	   		}
			else if (DMX_DBG_SHOW_STATISTICS == info_type)
			{        
				if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
				{        
					PrintSecChStatInfoHld(&ChInfo);
				}
	        }
        }		
    }
	
    for (FltIdx = 0; FltIdx < DmxSysInfo.SecFltCntTotal; FltIdx++)
    {
		memset(&FltInfo, 0, sizeof(FltInfo));

    	if (Ali_DmxSecFltInfoGet(FltIdx + ALI_DMX_STREAM_ID_SEC_FLT_START, &FltInfo))
		{
		    //DMX_DBG_PRINT("No section filter in dmx library!\n");

			continue;
		}

        if (ALI_DMX_STREAM_STATE_RUN == FltInfo.State)
        {
	   		if (DMX_DBG_SHOW_STREAM_INFO_SEC_FLT & info_type)
	   		{
	   			ShowSectionFltInfo(&FltInfo);
	   		}
			else if (DMX_DBG_SHOW_STATISTICS == info_type)
			{        
				if (DmxDbgCtrl.DbgLevel & DMX_DBG_LEVEL_HLD)
				{        
					PrintSecFltStatInfoHld(&FltInfo);
				}
	        }        
        }
    }
	
	if (!StreamExist)
	{
		DMX_DBG_PRINT("\nNo section stream opened in DeMux!\n");
	}
	
	return (0);
}

void dmx_dbg_task(UINT32 param1, UINT32 param2)
{
    UINT32 CurShowSysTickCnt;
    UINT32 PrevShowSysTickCnt = 0;
    UINT32 SysTickInc;

	for (;;)
	{
	    CurShowSysTickCnt = osal_get_tick();
	    SysTickInc = CurShowSysTickCnt - PrevShowSysTickCnt;
	    PrevShowSysTickCnt = CurShowSysTickCnt;
	
		if (DmxDbgCtrl.DmxDbgStatEn)
		{
			if (DmxDbgCtrl.HwRegDumpEn)
			{
				DmxDbgShowHwRegInfo();
			}

			if (DmxDbgCtrl.KernGlbStatEn)
			{
				DmxDbgShowKernelGlobalInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}
			
			if (DmxDbgCtrl.SeeGlbStatEn)
			{
				DmxDbgShowSeeGlobalInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}

			if (DmxDbgCtrl.VidStatEn)
			{
				DmxDbgShowVideoInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}

			if (DmxDbgCtrl.AudStatEn)
			{
				DmxDbgShowAudioInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}

			if (DmxDbgCtrl.PcrStatEn)
			{
				DmxDbgShowPcrInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}

			if (DmxDbgCtrl.SecStatEn)
			{
				DmxDbgShowSectionInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}

			if (DmxDbgCtrl.TsStatEn)
			{
				DmxDbgShowTsInfo(DMX_DBG_SHOW_STATISTICS, SysTickInc);
			}
			
			if (DmxDbgCtrl.DbgShowTms > 0 && 0 == --DmxDbgCtrl.DbgShowTms)
			{
				DMX_DBG_PRINT("\nTimes up! End of printout.\n");
				DmxDbgCtrl.DmxDbgStatEn = FALSE;
			}			
		}

		osal_task_sleep(DmxDbgCtrl.DbgShowIntv);
	}

	return;
}


static RET_CODE dmx_dbg_task_Init()
{
	OSAL_T_CTSK t_ctsk;	
	memset(&t_ctsk, 0, sizeof(OSAL_T_CTSK ));
	t_ctsk.task = (FP)dmx_dbg_task;
	t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 10;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.name[0] = 'D';
	t_ctsk.name[1] = 'D';
	t_ctsk.name[2] = 'T';
	DmxDbgCtrl.TaskId = osal_task_create(&t_ctsk);
	if (OSAL_INVALID_ID == DmxDbgCtrl.TaskId)
	{
		DMX_DBG_PRINT("dmx debug task init fail\n");
		return RET_FAILURE;
	}
	
	return RET_SUCCESS;
}

static void dmx_dbg_init(void)
{
	if (!DmxDbgCtrl.DmxDbgInit)
	{
		memset(&DmxDbgCtrl, 0, sizeof(DmxDbgCtrl));
		DmxDbgCtrl.DmxDbgInit = TRUE;
		DmxDbgCtrl.DmxDbgStatEn = TRUE;
		DmxDbgCtrl.DbgLevel = DMX_DBG_LEVEL_KERNEL | DMX_DBG_LEVEL_SEE;
		DmxDbgCtrl.DbgShowIntv = DMX_DBG_PRINT_INTRV;
		if (DmxDbgStatDeviceOpen(0) < 0)
		{
			//DMX_DBG_PRINT("Demux debug device open fail!\n");
			return;
		}
	}

	return;
}

static void dmx_dbg_exit(int argc, char **argv)
{
	if (DmxDbgCtrl.DmxDbgTskInit)
		osal_task_delete(DmxDbgCtrl.TaskId);

	DmxDbgStatDeviceClose(0);
	DMX_DBG_PRINT("DeMux debug statistics closed.\n");
	return;
}

static void dmx_dbg_realtime_on(int argc, char **argv)
{
	int Ret;

	if (!DmxDbgCtrl.DmxDbgInit)
	{
		dmx_dbg_init();
	}
	
	Ret = ioctl(DmxDbgCtrl.KernGlbFd, ALI_DMX_KERN_GLB_STREAM_REALTIME_SET, 1);

	if (Ret < 0)
	{
	    DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return;
	}

	Ret = ioctl(DmxDbgCtrl.SeeGlbFd, ALI_DMX_SEE_GLB_STREAM_REALTIME_SET, 1);

	if (Ret < 0)
	{
	    DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return;
	}

	return;
}

static void dmx_dbg_realtime_off(int argc, char **argv)
{
	int Ret;
    
	if (!DmxDbgCtrl.DmxDbgInit)
	{
		dmx_dbg_init();
	}

	Ret = ioctl(DmxDbgCtrl.KernGlbFd, ALI_DMX_KERN_GLB_STREAM_REALTIME_SET, 0);

	if (Ret < 0)
	{
	    DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return;
	}

	Ret = ioctl(DmxDbgCtrl.SeeGlbFd, ALI_DMX_SEE_GLB_STREAM_REALTIME_SET, 0);

	if (Ret < 0)
	{
	    DMX_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return;
	}

	return;
}

static void dmx_dbg_start(int argc, char **argv)
{
	if (!DmxDbgCtrl.DmxDbgInit)
		dmx_dbg_init();

	if (!DmxDbgCtrl.DmxDbgTskInit)
	{
		if (dmx_dbg_task_Init())
		{
			DMX_DBG_PRINT("Demux debug task initialized fail!\n");
			return;
		}
		DmxDbgCtrl.DmxDbgTskInit = 1;
	}

	DmxDbgCtrl.DmxDbgStatEn = TRUE;

	return;
}

static void dmx_dbg_pause(int argc, char **argv)
{
	if (!DmxDbgCtrl.DmxDbgTskInit)
	{
		if (dmx_dbg_task_Init())
		{
			DMX_DBG_PRINT("Demux debug task initialized fail!\n");
			return;
		}
		DmxDbgCtrl.DmxDbgTskInit = 1;
	}

	DmxDbgCtrl.DmxDbgStatEn = FALSE;
	return;
}

static void dmx_dbg_show_help(int argc, char **argv)
{
	DMX_DBG_PRINT("\nModule name:\n");
	DMX_DBG_PRINT("\tdmx\n");

	DMX_DBG_PRINT("\nModule Description:\n");
	DMX_DBG_PRINT("\tDeMux diagnostic debugging module.\n");	

	DMX_DBG_PRINT("\nSyntax:\n");
	DMX_DBG_PRINT("\tdmx -option [arguments]...\n");

	DMX_DBG_PRINT("\nDescription:\n");
	DMX_DBG_PRINT("\t-h, --help\n");
	DMX_DBG_PRINT("\t\tSelf description of this debugging module.\n");
	DMX_DBG_PRINT("\t-s, --start\n");
	DMX_DBG_PRINT("\t\tStart DeMux debugging printout task.\n");
	DMX_DBG_PRINT("\t-p, --pause\n");
	DMX_DBG_PRINT("\t\tPause DeMux debugging printout task.\n");
	DMX_DBG_PRINT("\t-e, --exit\n");
	DMX_DBG_PRINT("\t\tExit DeMux debugging module.\n");
	DMX_DBG_PRINT("\t-i, --info items...\n");
	DMX_DBG_PRINT("\t\tShow TS, Audio, Video & Section stream infomation, such as PID, stream id, filter id & mask/value.\n");
	DMX_DBG_PRINT("\t\tall\t--All the infomation.\n");
	DMX_DBG_PRINT("\t\tvid\t--Video stream infomation.\n");
	DMX_DBG_PRINT("\t\taud\t--Audio stream infomation.\n");
	DMX_DBG_PRINT("\t\tpcr\t--PCR stream infomation.\n");
	DMX_DBG_PRINT("\t\tpes\t--PES stream infomation.\n");
	DMX_DBG_PRINT("\t\tts\t--TS stream infomation.\n");
	DMX_DBG_PRINT("\t\tsec\t--Section stream infomation.\n");
	DMX_DBG_PRINT("\t\tsecslot\t--Section slot infomation.\n");
	DMX_DBG_PRINT("\t\tsecflt\t--Section filter infomation.\n");
	DMX_DBG_PRINT("\t\tsecch\t--Section channel infomation.\n");
	DMX_DBG_PRINT("\t-op, --open items...\n");
	DMX_DBG_PRINT("\t\tOpen debugging item statistics printout.\n");
	DMX_DBG_PRINT("\t\tall\t--All the items.\n");
	DMX_DBG_PRINT("\t\tkglb\t--Kernel global statistics.\n");
	DMX_DBG_PRINT("\t\tsglb\t--SEE global statistics.\n");
	DMX_DBG_PRINT("\t\tvid\t--Video stream statistics.\n");
	DMX_DBG_PRINT("\t\taud\t--Audio stream statistics.\n");
	DMX_DBG_PRINT("\t\tpcr\t--PCR stream statistics.\n");
	DMX_DBG_PRINT("\t\tpes\t--PES stream statistics.\n");
	DMX_DBG_PRINT("\t\tts\t--TS stream statistics.\n");
	DMX_DBG_PRINT("\t\tsec\t--Section stream statistics.\n");
	DMX_DBG_PRINT("\t\treg\t--Hardware register content dump.\n");
	DMX_DBG_PRINT("\t\tiocmd\t--IO command printout.\n");
	DMX_DBG_PRINT("\t-cl, --close items...\n");
	DMX_DBG_PRINT("\t\tClose debugging item statistics printout.\n");
	DMX_DBG_PRINT("\t\tall\t--All the items.\n");
	DMX_DBG_PRINT("\t\tkglb\t--Kernel global statistics.\n");
	DMX_DBG_PRINT("\t\tsglb\t--SEE global statistics.\n");
	DMX_DBG_PRINT("\t\tvid\t--Video stream statistics.\n");
	DMX_DBG_PRINT("\t\taud\t--Audio stream statistics.\n");
	DMX_DBG_PRINT("\t\tpcr\t--PCR stream statistics.\n");
	DMX_DBG_PRINT("\t\tpes\t--PES stream statistics.\n");
	DMX_DBG_PRINT("\t\tts\t--TS stream statistics.\n");
	DMX_DBG_PRINT("\t\tsec\t--Section stream statistics.\n");
	DMX_DBG_PRINT("\t\treg\t--Hardware register content dump.\n");
	DMX_DBG_PRINT("\t\tiocmd\t--IO command printout.\n");
	DMX_DBG_PRINT("\t-terv number\n");
	DMX_DBG_PRINT("\t\tSet time interval of debugging printout task(The default value is \"5000\").\n");
	DMX_DBG_PRINT("\t-tms, --times number\n");
	DMX_DBG_PRINT("\t\tSet times of debugging printout task(The default value is \"0\").\n");
	DMX_DBG_PRINT("\t-l, --level number\n");
	DMX_DBG_PRINT("\t\tSet debugging printout level(The default value is \"6\").\n");
	DMX_DBG_PRINT("\t\t0\t--No printout.\n");
	DMX_DBG_PRINT("\t\t1\t--Show HLD printout infomation.\n");
	DMX_DBG_PRINT("\t\t2\t--Show Kernel printout infomation.\n");
	DMX_DBG_PRINT("\t\t3\t--Show HLD & Kernel printout infomation.\n");
	DMX_DBG_PRINT("\t\t4\t--Show SEE printout infomation.\n");
	DMX_DBG_PRINT("\t\t5\t--Show HLD & SEE printout infomation.\n");
	DMX_DBG_PRINT("\t\t6\t--Show Kernel & SEE printout infomation.\n");
	DMX_DBG_PRINT("\t\t7\t--Show HLD & Kernel & SEE printout infomation.\n");
	DMX_DBG_PRINT("\t-rt\n");
	DMX_DBG_PRINT("\t\tOpen realtime debugging printout.\n");
	DMX_DBG_PRINT("\t-urt\n");
	DMX_DBG_PRINT("\t\tClose realtime debugging printout.\n");

	DMX_DBG_PRINT("\nUsage example:\n");
	DMX_DBG_PRINT("\tdmx -h[--help]\n");
	DMX_DBG_PRINT("\tdmx -s\n");
	DMX_DBG_PRINT("\tdmx -p\n");
	DMX_DBG_PRINT("\tdmx -e[--exit]\n");
	DMX_DBG_PRINT("\tdmx -i[--info] all [ts] [aud] [vid] [sec] [secslot] [secflt] [secch]\n");
	DMX_DBG_PRINT("\tdmx -op[--open] [all] [kglb] [sglb] [ts] [aud] [vid] [sec] [reg]\n");
	DMX_DBG_PRINT("\tdmx -cl[--close] [all] [kglb] [sglb] [ts] [aud] [vid] [sec] [reg]\n");
	DMX_DBG_PRINT("\tdmx -terv 5000\n");
	DMX_DBG_PRINT("\tdmx -tms[--times] 0\n");
	DMX_DBG_PRINT("\tdmx -l[--level] 6\n");
	DMX_DBG_PRINT("\tdmx -rt\n");
	DMX_DBG_PRINT("\tdmx -urt\n");

	return;
}

static void dmx_app_show_help(int argc, char **argv)
{
	DMX_DBG_PRINT("\nModule name:\n");
	DMX_DBG_PRINT("\tdmx\n");

	DMX_DBG_PRINT("\nModule Description:\n");
	DMX_DBG_PRINT("\tDeMux diagnostic debugging module.\n");	

	DMX_DBG_PRINT("\nSyntax:\n");
	DMX_DBG_PRINT("\tdmx -option [arguments]...\n");

	DMX_DBG_PRINT("\nDescription:\n");
	DMX_DBG_PRINT("\t-h, --help\n");
	DMX_DBG_PRINT("\t\tSelf description of this debugging module.\n");
	DMX_DBG_PRINT("\t-e, --exit\n");
	DMX_DBG_PRINT("\t\tExit DeMux debugging module.\n");
	DMX_DBG_PRINT("\t-i, --info items...\n");
	DMX_DBG_PRINT("\t\tShow TS, Audio, Video & Section stream infomation, such as PID, stream id, filter id & mask/value.\n");
	DMX_DBG_PRINT("\t\tall\t--All the infomation.\n");
	DMX_DBG_PRINT("\t\tvid\t--Video stream infomation.\n");
	DMX_DBG_PRINT("\t\taud\t--Audio stream infomation.\n");
	DMX_DBG_PRINT("\t\tpcr\t--PCR stream infomation.\n");
	DMX_DBG_PRINT("\t\tts\t--TS stream infomation.\n");
	DMX_DBG_PRINT("\t\tsec\t--Section stream infomation.\n");
	DMX_DBG_PRINT("\t\tsecslot\t--Section slot infomation.\n");
	DMX_DBG_PRINT("\t\tsecflt\t--Section filter infomation.\n");
	DMX_DBG_PRINT("\t\tsecch\t--Section channel infomation.\n");
	DMX_DBG_PRINT("\t-rt\n");
	DMX_DBG_PRINT("\t\tOpen realtime debugging printout.\n");
	DMX_DBG_PRINT("\t-urt\n");
	DMX_DBG_PRINT("\t\tClose realtime debugging printout.\n");

	DMX_DBG_PRINT("\nUsage example:\n");
	DMX_DBG_PRINT("\tdmx -h[--help]\n");
	DMX_DBG_PRINT("\tdmx -e[--exit]\n");
	DMX_DBG_PRINT("\tdmx -i[--info] all [ts] [aud] [vid] [sec] [secslot] [secflt] [secch]\n");
	DMX_DBG_PRINT("\tdmx -rt\n");
	DMX_DBG_PRINT("\tdmx -urt\n");

	return;
}

static ARG_CHK dmx_dbg_no_para_preview(int argc, char **argv, char *option)
{
	if (argc != 0)
	{
		DMX_DBG_PRINT("Option \"%s\": Should not be with any argument(s)!\n", option);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}

static void dmx_dbg_show_info(int argc, char **argv)
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
			DmxDbgShowVideoInfo(info_type, 0);
			DmxDbgShowAudioInfo(info_type, 0);
			DmxDbgShowPcrInfo(info_type, 0);
			DmxDbgShowSectionInfo(info_type, 0);		
			DmxDbgShowTsInfo(info_type, 0);
		}
		else if (!strcmp(*argv, "vid"))
		{
			DmxDbgShowVideoInfo(DMX_DBG_SHOW_STREAM_INFO, 0);
		}
		else if (!strcmp(*argv, "aud"))
		{
			DmxDbgShowAudioInfo(DMX_DBG_SHOW_STREAM_INFO, 0);
		}
		else if (!strcmp(*argv, "pcr"))
		{
			DmxDbgShowPcrInfo(DMX_DBG_SHOW_STREAM_INFO, 0);
		}
		else if (!strcmp(*argv, "secslot"))
		{
			info_type |= DMX_DBG_SHOW_STREAM_INFO_SEC_SLOT;
			DmxDbgShowSectionInfo(info_type, 0);
		}
		else if (!strcmp(*argv, "secflt"))
		{
			info_type |= DMX_DBG_SHOW_STREAM_INFO_SEC_FLT;
			DmxDbgShowSectionInfo(info_type, 0);
		}
		else if (!strcmp(*argv, "secch"))
		{
			info_type |= DMX_DBG_SHOW_STREAM_INFO_SEC_CH;
			DmxDbgShowSectionInfo(info_type, 0);
		}
		else if (!strcmp(*argv, "sec"))
		{
			info_type |= DMX_DBG_SHOW_STREAM_INFO;
			DmxDbgShowSectionInfo(info_type, 0);
		}		
		else if (!strcmp(*argv, "ts"))
		{
			DmxDbgShowTsInfo(DMX_DBG_SHOW_STREAM_INFO, 0);
		}		
		else
		{
			DMX_DBG_PRINT("Unrecognized argument(s)!\n");
			break;
		}

		argv++;
	}	
}

static ARG_CHK dmx_dbg_info_preview(int argc, char **argv, char *option)
{
	while (argc-- > 0)
	{
		if (!strcmp(*argv, "kglb") ||
			!strcmp(*argv, "sglb") ||
			!strcmp(*argv, "reg") ||
			!strcmp(*argv, "iocmd"))
		{
			DMX_DBG_PRINT("Option \"%s\": Unsupport argument \"%s\"!\n", option, *argv);
			return DBG_ARG_INVALID;
		}
		else if (strcmp(*argv, "all") &&
				strcmp(*argv, "ts") &&
				strcmp(*argv, "vid") &&
				strcmp(*argv, "aud") &&
				strcmp(*argv, "pcr") &&
				strcmp(*argv, "secslot") &&
				strcmp(*argv, "secflt") &&
				strcmp(*argv, "secch") &&
				strcmp(*argv, "sec"))
		{
			DMX_DBG_PRINT("Option \"%s\": Unrecognized argument(s) \"%s\"!\n", option, *argv);
			return DBG_ARG_INVALID;
		}

		argv++;
	}	

	return DBG_ARG_VALID;
}

static void dmx_dbg_switch_on(int argc, char **argv)
{	
	if (!DmxDbgCtrl.DmxDbgInit)
		dmx_dbg_init();	

	if (!DmxDbgCtrl.DmxDbgTskInit)
	{
		if (dmx_dbg_task_Init())
		{
			DMX_DBG_PRINT("Demux debug task initialized fail!\n");
			return;
		}
		DmxDbgCtrl.DmxDbgTskInit = 1;
	}

	if (0 == argc)
		goto swith_on_all;

	while (argc-- > 0)
	{
		if (!strcmp(*argv, "all"))
		{
swith_on_all:
			ioctl(DmxDbgCtrl.KernGlbFd, ALI_DMX_KERN_GLB_STREAM_START, 0);
			ioctl(DmxDbgCtrl.SeeGlbFd, ALI_DMX_SEE_GLB_STREAM_START, 0);
			ioctl(DmxDbgCtrl.HwRegFd, ALI_DMX_HW_REG_STREAM_START, 0);
			DmxDbgCtrl.KernGlbStatEn = TRUE;
			DmxDbgCtrl.SeeGlbStatEn = TRUE;
			DmxDbgCtrl.VidStatEn = TRUE;
			DmxDbgCtrl.AudStatEn = TRUE;
			DmxDbgCtrl.PcrStatEn = TRUE;
			DmxDbgCtrl.SecStatEn = TRUE;
			DmxDbgCtrl.PesStatEn = TRUE;
			DmxDbgCtrl.TsStatEn = TRUE;
			DmxDbgCtrl.HwRegDumpEn = TRUE;
			DmxDbgCtrl.IoCmdStatEn = TRUE;
			DMX_DBG_PRINT("Kernel global statistics enabled.\n");
			DMX_DBG_PRINT("SEE global statistics enabled.\n");
			DMX_DBG_PRINT("Video statistics enabled.\n");
			DMX_DBG_PRINT("Audio statistics enabled.\n");
			DMX_DBG_PRINT("PCR statistics enabled.\n");
			DMX_DBG_PRINT("Section statistics enabled.\n");
			DMX_DBG_PRINT("Pes statistics enabled.\n");
			DMX_DBG_PRINT("TS(Rec) statistics enabled.\n");
			DMX_DBG_PRINT("DeMux hardware register dump enabled.\n");
			DMX_DBG_PRINT("DeMux I/O commands list enabled.\n");	
		}		
		else if (!strcmp(*argv, "kglb"))
		{
			ioctl(DmxDbgCtrl.KernGlbFd, ALI_DMX_KERN_GLB_STREAM_START, 0);
			DmxDbgCtrl.KernGlbStatEn = TRUE;
			DMX_DBG_PRINT("Kernel global statistics enabled.\n");
		}
		else if (!strcmp(*argv, "sglb"))
		{
			ioctl(DmxDbgCtrl.SeeGlbFd, ALI_DMX_SEE_GLB_STREAM_START, 0);
			DmxDbgCtrl.SeeGlbStatEn = TRUE;
			DMX_DBG_PRINT("SEE global statistics enabled.\n");
		}
		else if (!strcmp(*argv, "vid"))
		{
			DmxDbgCtrl.VidStatEn = TRUE;
			DMX_DBG_PRINT("Video statistics enabled.\n");
		}
		else if (!strcmp(*argv, "aud"))
		{
			DmxDbgCtrl.AudStatEn = TRUE;
			DMX_DBG_PRINT("Audio statistics enabled.\n");
		}
		else if (!strcmp(*argv, "pcr"))
		{
			DmxDbgCtrl.PcrStatEn = TRUE;
			DMX_DBG_PRINT("PCR statistics enabled.\n");
		}
		else if (!strcmp(*argv, "sec"))
		{
			DmxDbgCtrl.SecStatEn = TRUE;
			DMX_DBG_PRINT("Section statistics enabled.\n");
		}
		else if (!strcmp(*argv, "pes"))
		{
			DmxDbgCtrl.PesStatEn = TRUE;
			DMX_DBG_PRINT("PES statistics enabled.\n");
		}
		else if (!strcmp(*argv, "ts"))
		{
			DmxDbgCtrl.TsStatEn = TRUE;
			DMX_DBG_PRINT("Ts(Rec) statistics enabled.\n");
		}
		else if (!strcmp(*argv, "reg"))
		{
			ioctl(DmxDbgCtrl.HwRegFd, ALI_DMX_HW_REG_STREAM_START, 0);
			DmxDbgCtrl.HwRegDumpEn = TRUE;
			DMX_DBG_PRINT("DeMux hardware register dump enabled.\n");
		}
		else if (!strcmp(*argv, "iocmd"))
		{
			DmxDbgCtrl.IoCmdStatEn = TRUE;
			DMX_DBG_PRINT("DeMux I/O commands list enabled.\n");
		}
		else
		{
			DMX_DBG_PRINT("Unrecognized argument(s)!\n");
		}

		argv++;
	}

	Ali_DmxLibRealTimePrintSet(DmxDbgCtrl.IoCmdStatEn);

	DmxDbgCtrl.DmxDbgStatEn = TRUE;

	DMX_DBG_PRINT("Waiting for statistics printout...\n");
	
	return;	
}

static void dmx_dbg_switch_off(int argc, char **argv)
{
	if (!DmxDbgCtrl.DmxDbgInit)
		dmx_dbg_init();	

	if (!DmxDbgCtrl.DmxDbgTskInit)
	{
		if (dmx_dbg_task_Init())
		{
			DMX_DBG_PRINT("Demux debug task initialized fail!\n");
			return;
		}
		DmxDbgCtrl.DmxDbgTskInit = 1;
	}

	if (0 == argc)
		goto switch_off_all;

	while (argc-- > 0)
	{
		if (!strcmp(*argv, "all"))
		{
switch_off_all:
			ioctl(DmxDbgCtrl.KernGlbFd, ALI_DMX_KERN_GLB_STREAM_STOP, 0);
			ioctl(DmxDbgCtrl.SeeGlbFd, ALI_DMX_SEE_GLB_STREAM_STOP, 0);
			ioctl(DmxDbgCtrl.HwRegFd, ALI_DMX_HW_REG_STREAM_STOP, 0);			
			DmxDbgCtrl.KernGlbStatEn = FALSE;
			DmxDbgCtrl.SeeGlbStatEn = FALSE;
			DmxDbgCtrl.VidStatEn = FALSE;
			DmxDbgCtrl.AudStatEn = FALSE;
			DmxDbgCtrl.PcrStatEn = FALSE;
			DmxDbgCtrl.SecStatEn = FALSE;
			DmxDbgCtrl.PesStatEn = FALSE;
			DmxDbgCtrl.TsStatEn = FALSE;
			DmxDbgCtrl.HwRegDumpEn = FALSE;
			DmxDbgCtrl.IoCmdStatEn = FALSE;
			DMX_DBG_PRINT("Kernel global statistics disdabled.\n");
			DMX_DBG_PRINT("SEE global statistics disdabled.\n");
			DMX_DBG_PRINT("Video statistics disdabled.\n");
			DMX_DBG_PRINT("Audio statistics disdabled.\n");
			DMX_DBG_PRINT("PCR statistics disdabled.\n");
			DMX_DBG_PRINT("Section statistics disabled.\n");
			DMX_DBG_PRINT("PES statistics disabled.\n");
			DMX_DBG_PRINT("TS(Rec) statistics disabled.\n");
			DMX_DBG_PRINT("DeMux hardware register dump disdabled.\n");
			DMX_DBG_PRINT("DeMux I/O commands list disdabled.\n");	
		}
		else if (!strcmp(*argv, "kglb"))
		{
			ioctl(DmxDbgCtrl.KernGlbFd, ALI_DMX_KERN_GLB_STREAM_STOP, 0);
			DmxDbgCtrl.KernGlbStatEn = FALSE;
			DMX_DBG_PRINT("Kernel global statistics disdabled.\n");
		}
		else if (!strcmp(*argv, "sglb"))
		{
			ioctl(DmxDbgCtrl.SeeGlbFd, ALI_DMX_SEE_GLB_STREAM_STOP, 0);
			DmxDbgCtrl.SeeGlbStatEn = FALSE;
			DMX_DBG_PRINT("SEE global statistics disdabled.\n");
		}
		else if (!strcmp(*argv, "vid"))
		{
			DmxDbgCtrl.VidStatEn = FALSE;
			DMX_DBG_PRINT("Video statistics disdabled.\n");
		}
		else if (!strcmp(*argv, "aud"))
		{
			DmxDbgCtrl.AudStatEn = FALSE;
			DMX_DBG_PRINT("Audio statistics disdabled.\n");
		}
		else if (!strcmp(*argv, "pcr"))
		{
			DmxDbgCtrl.PcrStatEn = FALSE;
			DMX_DBG_PRINT("PCR statistics disdabled.\n");
		}
		else if (!strcmp(*argv, "sec"))
		{
			DmxDbgCtrl.SecStatEn = FALSE;
			DMX_DBG_PRINT("Section statistics disabled.\n");
		}
		else if (!strcmp(*argv, "pes"))
		{
			DmxDbgCtrl.PesStatEn = FALSE;
			DMX_DBG_PRINT("PES statistics disabled.\n");
		}
		else if (!strcmp(*argv, "ts"))
		{
			DmxDbgCtrl.TsStatEn = FALSE;
			DMX_DBG_PRINT("Ts(Rec) statistics disabled.\n");
		}		
		else if (!strcmp(*argv, "reg"))
		{
			ioctl(DmxDbgCtrl.HwRegFd, ALI_DMX_HW_REG_STREAM_STOP, 0);			
			DmxDbgCtrl.HwRegDumpEn = FALSE;
			DMX_DBG_PRINT("DeMux hardware register dump disdabled.\n");
		}
		else if (!strcmp(*argv, "iocmd"))
		{
			DmxDbgCtrl.IoCmdStatEn = FALSE;
			DMX_DBG_PRINT("DeMux I/O commands list disdabled.\n");
		}
		else
		{
			DMX_DBG_PRINT("Unrecognized argument(s)!\n");
		}

		argv++;
	}

	DmxDbgCtrl.DmxDbgStatEn = TRUE;
	
	Ali_DmxLibRealTimePrintSet(DmxDbgCtrl.IoCmdStatEn);

	return;	
}

static ARG_CHK dmx_dbg_swi_preview(int argc, char **argv, char *option)
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
			DMX_DBG_PRINT("Option \"%s\": Unrecognized argument \"%s\"!\n", option, *argv);
			return DBG_ARG_INVALID;
		}

		argv++;
	}

	return DBG_ARG_VALID;
}

static void dmx_dbg_set_level(int argc, char **argv)
{
	int number, ret;

	if (0 == argc)
	{
		DMX_DBG_PRINT("Lack of valid argument(ex: \"100\")!\n");
		return;
	}

	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0 ||
		number > (DMX_DBG_LEVEL_KERNEL | DMX_DBG_LEVEL_HLD | DMX_DBG_LEVEL_SEE))
	{
		DMX_DBG_PRINT("Argument \"%s\" not a number or out of range 0 ~ 7!\n", *argv);
		return;
	}

	if (!DmxDbgCtrl.DmxDbgInit)
		dmx_dbg_init();	

	DmxDbgCtrl.DbgLevel = number;
	DmxDbgCtrl.DmxDbgStatEn = TRUE;

	return;
}

static ARG_CHK dmx_dbg_level_preview(int argc, char **argv, char *option)
{
	int number, ret;

	if (0 == argc)
	{
		DMX_DBG_PRINT("Option \"%s\": Lack of valid argument(ex: number)!\n", option);
		return DBG_ARG_INVALID;
	}

	if (argc != 1)
	{
		DMX_DBG_PRINT("Option \"%s\": Only one argument admitted!\n", option);
		return DBG_ARG_INVALID;
	}
	
	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0 ||
		number > (DMX_DBG_LEVEL_KERNEL | DMX_DBG_LEVEL_HLD | DMX_DBG_LEVEL_SEE))
	{
		DMX_DBG_PRINT("Option \"%s\": Argument \"%s\" not a number or out of range 0-7!\n", option, *argv);
		return DBG_ARG_INVALID;
	}
	
	return DBG_ARG_VALID;
}

static void dmx_dbg_set_intv(int argc, char **argv)
{
	int number, ret;

	if (0 == argc)
	{
		DMX_DBG_PRINT("Lack of valid argument(ex: \"5000\")!\n");
		return;
	}
	
	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		DMX_DBG_PRINT("Argument \"%s\" not a number or less than zero!\n", *argv);
		return;
	}

	if (!DmxDbgCtrl.DmxDbgInit)
		dmx_dbg_init();		

	DmxDbgCtrl.DbgShowIntv = number;
	DmxDbgCtrl.DmxDbgStatEn = TRUE;

	return;
}

static void dmx_dbg_set_tms(int argc, char **argv)
{
	int number, ret;

	if (0 == argc)
	{
		DMX_DBG_PRINT("Lack of valid argument(ex: \"5\")!\n");
		return;
	}

	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		DMX_DBG_PRINT("Argument \"%s\" not a number or less than zero!\n", *argv);
		return;
	}

	if (!DmxDbgCtrl.DmxDbgInit)
		dmx_dbg_init();		

	DmxDbgCtrl.DbgShowTms = number;
	DmxDbgCtrl.DmxDbgStatEn = TRUE;

	return;
}

static ARG_CHK dmx_dbg_num_preview(int argc, char **argv, char *option)
{
	int number, ret;
	char *p;

	if (0 == argc)
	{
		DMX_DBG_PRINT("Option \"%s\": Lack of valid argument(ex: number)!\n", option);
		return DBG_ARG_INVALID;
	}

	if (argc != 1)
	{
		DMX_DBG_PRINT("Option \"%s\": Only one argument admitted!\n", option);
		return DBG_ARG_INVALID;
	}
	
	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		DMX_DBG_PRINT("Option \"%s\": Argument \"%s\" not a number or less than zero!\n", option, *argv);
		return DBG_ARG_INVALID;
	}
	
	return DBG_ARG_VALID;
}

static PARSE_COMMAND_LIST dmxdbg_command[] = {
	{ { NULL, NULL }, dmx_dbg_start, dmx_dbg_no_para_preview, "start", "s", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_pause, dmx_dbg_no_para_preview, "pause", "p", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_show_help, dmx_dbg_no_para_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_show_info, dmx_dbg_info_preview, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_switch_on, dmx_dbg_swi_preview, "open", "op", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_switch_off, dmx_dbg_swi_preview, "close", "cl", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_set_level, dmx_dbg_level_preview, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_set_intv, dmx_dbg_num_preview, NULL, "terv", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_set_tms, dmx_dbg_num_preview, "times", "tms", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_exit, dmx_dbg_no_para_preview, "exit", "e", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_realtime_on, dmx_dbg_no_para_preview, NULL, "rt", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_realtime_off, dmx_dbg_no_para_preview, NULL, "urt", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};

static PARSE_COMMAND_LIST dmxdbg_app_command[] = {
	//{ { NULL, NULL }, dmx_dbg_start, dmx_dbg_no_para_preview, "start", "s", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, dmx_dbg_pause, dmx_dbg_no_para_preview, "pause", "p", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_app_show_help, dmx_dbg_no_para_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_show_info, dmx_dbg_info_preview, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, dmx_dbg_switch_on, dmx_dbg_swi_preview, "open", "op", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, dmx_dbg_switch_off, dmx_dbg_swi_preview, "close", "cl", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, dmx_dbg_set_level, dmx_dbg_level_preview, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, dmx_dbg_set_intv, dmx_dbg_num_preview, NULL, "terv", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, dmx_dbg_set_tms, dmx_dbg_num_preview, "times", "tms", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_exit, dmx_dbg_no_para_preview, "exit", "e", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_realtime_on, dmx_dbg_no_para_preview, NULL, "rt", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dmx_dbg_realtime_off, dmx_dbg_no_para_preview, NULL, "urt", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};

void dmxdbg_module_register(void)
{
	debug_module_add("dmx", &dmxdbg_command[0], ARRAY_SIZE(dmxdbg_command));
}

INT32 dmx_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &dmxdbg_command[0];
	*list_cnt = ARRAY_SIZE(dmxdbg_command);

	if (!DmxDbgCtrl.DmxDbgInit)
		dmx_dbg_init();

	return RET_SUCCESS;
}

INT32 dmx_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &dmxdbg_app_command[0];
	*list_cnt = ARRAY_SIZE(dmxdbg_app_command);

	if (!DmxDbgCtrl.DmxDbgInit)
		dmx_dbg_init();

	return RET_SUCCESS;
}
