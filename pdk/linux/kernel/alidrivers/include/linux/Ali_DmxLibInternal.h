#ifndef _ALI_DMX_LIB_INTERNAL_H_
#define _ALI_DMX_LIB_INTERNAL_H_

#include <linux/types.h>
/*! @addtogroup DeviceDriver
 *  @{
 */

/*! @addtogroup ALiDMX
 *  @{
 */


/* HW DMX Linux output inerface dev name, used by kernel.
*/
#define ALI_HWDMX0_OUTPUT_NAME "ali_hwdmx0_output"
#define ALI_HWDMX1_OUTPUT_NAME "ali_hwdmx1_output"
#define ALI_HWDMX2_OUTPUT_NAME "ali_hwdmx2_output"
#define ALI_HWDMX3_OUTPUT_NAME "ali_hwdmx3_output"
#define ALI_HWDMX4_OUTPUT_NAME "ali_hwdmx4_output"
#define ALI_HWDMX5_OUTPUT_NAME "ali_hwdmx5_output"
#define ALI_HWDMX6_OUTPUT_NAME "ali_hwdmx6_output"
#define ALI_HWDMX7_OUTPUT_NAME "ali_hwdmx7_output"

/* SW DMX Linux output inerface dev name, used by kernel.
*/
#define ALI_SWDMX0_OUTPUT_NAME "ali_swdmx0_output"
#define ALI_SWDMX1_OUTPUT_NAME "ali_swdmx1_output"
#define ALI_SWDMX2_OUTPUT_NAME "ali_swdmx2_output"
#define ALI_SWDMX3_OUTPUT_NAME "ali_swdmx3_output"
#define ALI_SWDMX4_OUTPUT_NAME "ali_swdmx4_output"
#define ALI_SWDMX5_OUTPUT_NAME "ali_swdmx5_output"
#define ALI_SWDMX6_OUTPUT_NAME "ali_swdmx6_output"
#define ALI_SWDMX7_OUTPUT_NAME "ali_swdmx7_output"


/* SW DMX Linux input inerface dev name, used by kernel.
*/
#define ALI_SWDMX0_INPUT_NAME "ali_swdmx0_input"
#define ALI_SWDMX1_INPUT_NAME "ali_swdmx1_input"
#define ALI_SWDMX2_INPUT_NAME "ali_swdmx2_input"
#define ALI_SWDMX3_INPUT_NAME "ali_swdmx3_input"
#define ALI_SWDMX4_INPUT_NAME "ali_swdmx4_input"
#define ALI_SWDMX5_INPUT_NAME "ali_swdmx5_input"
#define ALI_SWDMX6_INPUT_NAME "ali_swdmx6_input"
#define ALI_SWDMX7_INPUT_NAME "ali_swdmx7_input"


/* HW DMX Linux output inerface dev path, used by user space.
*/
#define ALI_HWDMX0_OUTPUT_PATH "/dev/ali_hwdmx0_output"	//!< Output device path of hardware demux0
#define ALI_HWDMX1_OUTPUT_PATH "/dev/ali_hwdmx1_output"	//!< Output device path of hardware demux1
#define ALI_HWDMX2_OUTPUT_PATH "/dev/ali_hwdmx2_output"	//!< Output device path of hardware demux2
#define ALI_HWDMX3_OUTPUT_PATH "/dev/ali_hwdmx3_output"	//!< Output device path of hardware demux3
#define ALI_HWDMX4_OUTPUT_PATH "/dev/ali_hwdmx4_output"	//!< Output device path of hardware demux4
#define ALI_HWDMX5_OUTPUT_PATH "/dev/ali_hwdmx5_output"	//!< Output device path of hardware demux5
#define ALI_HWDMX6_OUTPUT_PATH "/dev/ali_hwdmx6_output"	//!< Output device path of hardware demux6
#define ALI_HWDMX7_OUTPUT_PATH "/dev/ali_hwdmx7_output"	//!< Output device path of hardware demux7

/* SW DMX Linux output inerface dev name, used by user space.
*/
#define ALI_SWDMX0_OUTPUT_PATH "/dev/ali_swdmx0_output"	//!< Output device path of software demux0
#define ALI_SWDMX1_OUTPUT_PATH "/dev/ali_swdmx1_output"	//!< Output device path of software demux1
#define ALI_SWDMX2_OUTPUT_PATH "/dev/ali_swdmx2_output"	//!< Output device path of software demux2
#define ALI_SWDMX3_OUTPUT_PATH "/dev/ali_swdmx3_output"	//!< Output device path of software demux3
#define ALI_SWDMX4_OUTPUT_PATH "/dev/ali_swdmx4_output"	//!< Output device path of software demux4
#define ALI_SWDMX5_OUTPUT_PATH "/dev/ali_swdmx5_output"	//!< Output device path of software demux5
#define ALI_SWDMX6_OUTPUT_PATH "/dev/ali_swdmx6_output"	//!< Output device path of software demux6
#define ALI_SWDMX7_OUTPUT_PATH "/dev/ali_swdmx7_output"	//!< Output device path of software demux7


/* SW DMX Linux input inerface dev name, used by user space.
*/
#define ALI_SWDMX0_INPUT_PATH "/dev/ali_swdmx0_input"	//!< Input device path of software demux0
#define ALI_SWDMX1_INPUT_PATH "/dev/ali_swdmx1_input"	//!< Input device path of software demux1
#define ALI_SWDMX2_INPUT_PATH "/dev/ali_swdmx2_input"	//!< Input device path of software demux2
#define ALI_SWDMX3_INPUT_PATH "/dev/ali_swdmx3_input"	//!< Input device path of software demux3
#define ALI_SWDMX4_INPUT_PATH "/dev/ali_swdmx4_input"	//!< Input device path of software demux4
#define ALI_SWDMX5_INPUT_PATH "/dev/ali_swdmx5_input"	//!< Input device path of software demux5
#define ALI_SWDMX6_INPUT_PATH "/dev/ali_swdmx6_input"	//!< Input device path of software demux6
#define ALI_SWDMX7_INPUT_PATH "/dev/ali_swdmx7_input"	//!< Input device path of software demux7

/* DMX HW inerface ID, data source from tuner, used by kernel.
*/
#define ALI_HWDMX0_OUTPUT_HWIF_ID 0
#define ALI_HWDMX1_OUTPUT_HWIF_ID 1
#define ALI_HWDMX2_OUTPUT_HWIF_ID 2
#define ALI_HWDMX3_OUTPUT_HWIF_ID 3

/* DMX HW inerface ID, data source from user space, used by kernel.
*/
#define ALI_SWDMX0_OUTPUT_HWIF_ID 4
#define ALI_SWDMX1_OUTPUT_HWIF_ID 5

/* DMX HW inerface ID, data source from SEE, used by kernel.
*/
#define ALI_SEETOMAIN_BUF_HWIF_ID 6


/* HW DMX data engine task name.
*/
#define ALI_HWDMX0_ENGINE_NAME "ali_hwdmx0_engine"
#define ALI_HWDMX1_ENGINE_NAME "ali_hwdmx1_engine"
#define ALI_HWDMX2_ENGINE_NAME "ali_hwdmx2_engine"
#define ALI_HWDMX3_ENGINE_NAME "ali_hwdmx3_engine"
#define ALI_HWDMX4_ENGINE_NAME "ali_hwdmx4_engine"
#define ALI_HWDMX5_ENGINE_NAME "ali_hwdmx5_engine"
#define ALI_HWDMX6_ENGINE_NAME "ali_hwdmx6_engine"
#define ALI_HWDMX7_ENGINE_NAME "ali_hwdmx7_engine"

/* SW DMX data engine task name.
*/
#define ALI_SWDMX0_ENGINE_NAME "ali_swdmx0_engine"
#define ALI_SWDMX1_ENGINE_NAME "ali_swdmx1_engine"
#define ALI_SWDMX2_ENGINE_NAME "ali_swdmx2_engine"
#define ALI_SWDMX3_ENGINE_NAME "ali_swdmx3_engine"
#define ALI_SWDMX4_ENGINE_NAME "ali_swdmx4_engine"
#define ALI_SWDMX5_ENGINE_NAME "ali_swdmx5_engine"
#define ALI_SWDMX6_ENGINE_NAME "ali_swdmx6_engine"
#define ALI_SWDMX7_ENGINE_NAME "ali_swdmx7_engine"


/* SEE to main data engine task name.
*/
#define ALI_DMX_SEE2MAIN0_ENGINE_NAME "ali_dmx_see2main0_engine"
#define ALI_DMX_SEE2MAIN1_ENGINE_NAME "ali_dmx_see2main1_engine"
#define ALI_DMX_SEE2MAIN2_ENGINE_NAME "ali_dmx_see2main2_engine"
#define ALI_DMX_SEE2MAIN3_ENGINE_NAME "ali_dmx_see2main3_engine"
#define ALI_DMX_SEE2MAIN4_ENGINE_NAME "ali_dmx_see2main4_engine"
#define ALI_DMX_SEE2MAIN5_ENGINE_NAME "ali_dmx_see2main5_engine"
#define ALI_DMX_SEE2MAIN6_ENGINE_NAME "ali_dmx_see2main6_engine"
#define ALI_DMX_SEE2MAIN7_ENGINE_NAME "ali_dmx_see2main7_engine"

/*! @struct Ali_DmxKernGlobalStatInfo
@brief Global state Info of demux in kernel.
*/
struct Ali_DmxKernGlobalStatInfo
{
	__u32 TotalTsInCnt;		//!< Count of TS package
	__u32 DmxBufEmptyCnt;	//!< Count of Demux buffer empty
	__u32 OverlapCnt;		//!< Count of Demux overlap, increase the value when hardware buf overflow happened
	__u32 PlayBusyCnt;		//!< Count of Demux play busy
	__u32 NobodyCareCnt;	//!< Nobody cares this TS packet. It must be something wrong.
	__u32 RealTimePrintEn;	//!< Interval realtime for print
};

/*! @struct Ali_DmxDrvTsStrmStatInfo
@brief Information of TS stream
*/
struct Ali_DmxDrvTsStrmStatInfo
{
	__u32 TsFltIdx;			//!< Index of TS filter
	__u32 TsInCnt;			//!< Count of TS package
	__u32 StrTypeErrCnt;	//!< Count of stream type error
	__u32 StatErrCnt;		//!< Count of stream state error
	__u32 WrByteErrCnt;		//!< Count of stream write error
	__u32 RdByteErrCnt;		//!< Count of stream read error
	__u32 CopyErrCnt;		//!< Count of copy stream parameter error
	__u32 CloseFailCnt;		//!< Count of close stream error
	__u32 CfgFailCnt;		//!< Count of config stream error
	__u32 StartFailCnt;		//!< Count of start stream error
	__u32 StopFailCnt;		//!< Count of stop stream error
	__u32 NoPidCnt;			//!< Count of no pid stream error
};

/*! @struct Ali_DmxDrvTsInRamStrmStatInfo
@brief Information of TS in ram stream
*/
struct Ali_DmxDrvTsInRamStrmStatInfo
{
	__u32 StatErrCnt;		//!< Count of stream state error
	__u32 CopyErrCnt;		//!< Count of copy stream parameter error
};

/*! @struct Ali_DmxDrvVideoStrmStatInfo
@brief Information of video stream
*/
struct Ali_DmxDrvVideoStrmStatInfo
{
	__u32 TsInCnt;			//!< Count of TS package
	__u32 StrTypeErrCnt;	//!< Count of stream type error
	__u32 StatErrCnt;		//!< Count of stream state error
	__u32 CopyErrCnt;		//!< Count of copy stream parameter error
	__u32 CloseFailCnt;		//!< Count of close stream error
	__u32 CfgFailCnt;		//!< Count of config stream error
	__u32 StartFailCnt;		//!< Count of start stream error
	__u32 StopFailCnt;		//!< Count of stop stream error
};

/*! @struct Ali_DmxDrvAudioStrmStatInfo
@brief Information of audio stream
*/
struct Ali_DmxDrvAudioStrmStatInfo
{
	__u32 TsInCnt;			//!< Count of TS package
	__u32 StrTypeErrCnt;	//!< Count of stream type error
	__u32 StatErrCnt;		//!< Count of stream state error
	__u32 CopyErrCnt;		//!< Count of copy stream parameter error
	__u32 CloseFailCnt;		//!< Count of close stream error
	__u32 CfgFailCnt;		//!< Count of config stream error
	__u32 StartFailCnt;		//!< Count of start stream error
	__u32 StopFailCnt;		//!< Count of stop stream error
};

/*! @struct Ali_DmxDrvPcrStrmStatInfo
@brief Information of PCR stream
*/
struct Ali_DmxDrvPcrStrmStatInfo
{
	__u32 TsInCnt;			//!< Count of TS package
	__u32 PcrInCnt;			//!< Count of PCR
	__u32 LastPcrVal;		//!< last PCR value
	__u32 StrTypeErrCnt;	//!< Count of stream type error
	__u32 StatErrCnt;		//!< Count of stream state error
	__u32 RdByteErrCnt;		//!< Count of stream read error
	__u32 RdBufShortCnt;	//!< Count of user buffer short
	__u32 CopyErrCnt;		//!< Count of copy stream parameter error
	__u32 CloseFailCnt;		//!< Count of close stream error
	__u32 CfgFailCnt;		//!< Count of config stream error
	__u32 StartFailCnt;		//!< Count of start stream error
	__u32 StopFailCnt;		//!< Count of stop stream error
};

/*! @struct Ali_DmxDrvSecStrmStatInfo
@brief Information of section stream
*/
struct Ali_DmxDrvSecStrmStatInfo
{
	__u32 TsInCnt;			//!< Count of TS package
	__u32 ByteInCnt;		//!< Not use now
	__u32 SecInCnt;			//!< Count of section input
	__u32 SecOutCnt;		//!< Count of section output
	__u32 CbTypeErrCnt;		//!< Count of callback type error
	__u32 StrTypeErrCnt;	//!< Count of stream type error
	__u32 StatErrCnt;		//!< Count of stream state error
	__u32 WrByteErrCnt;		//!< Count of stream write error
	__u32 RdBufShortCnt;	//!< Count of user buffer short
	__u32 CopyErrCnt;		//!< Count of copy stream parameter error
	__u32 CloseFailCnt;		//!< Count of close stream error
	__u32 CfgFailCnt;		//!< Count of config stream error
	__u32 StartFailCnt;		//!< Count of start stream error
	__u32 StopFailCnt;		//!< Count of stop stream error
};

/*! @struct Ali_DmxDrvPesStrmStatInfo
@brief Information of PES stream
*/
struct Ali_DmxDrvPesStrmStatInfo
{
	__u32 TsInCnt;			//!< Count of ts package
	__u32 PesInCnt;			//!< Count of pes input
	__u32 PesOutCnt;		//!< Count of pes output
	__u32 CbTypeErrCnt;		//!< Count of callback type error
	__u32 StrTypeErrCnt;	//!< Count of stream type error
	__u32 StatErrCnt;		//!< Count of stream state error
	__u32 WrByteErrCnt;		//!< Count of stream write error
	__u32 RdBufShortCnt;	//!< Count of user buffer short
	__u32 CopyErrCnt;		//!< Count of copy stream parameter error
	__u32 CloseFailCnt;		//!< Count of close stream error
	__u32 CfgFailCnt;		//!< Count of config stream error
	__u32 StartFailCnt;		//!< Count of start stream error
	__u32 StopFailCnt;		//!< Count of stop stream error
};

/*! @struct Ali_DmxDrvTpStrmStatInfo
@brief Information of TP stream
*/
struct Ali_DmxDrvTpStrmStatInfo
{
	__u32 TsInCnt;			//!< Count of TS package
	__u32 ByteInCnt;		//!< Not use now
	__u32 StrTypeErrCnt;	//!< Count of stream type error
	__u32 StatErrCnt;		//!< Count of stream state error
	__u32 WrByteErrCnt;		//!< Count of stream write error
	__u32 RdByteErrCnt;		//!< Count of stream read error
	__u32 RdBufShortCnt;	//!< Count of user buffer short
	__u32 CopyErrCnt;		//!< Count of copy stream parameter error
	__u32 CloseFailCnt;		//!< Count of close stream error
	__u32 CfgFailCnt;		//!< Count of config stream error
	__u32 StartFailCnt;		//!< Count of start stream error
	__u32 StopFailCnt;		//!< Count of stop stream error
};

/*! @struct Ali_DmxDrvTsFltStatInfo
@brief State information of TS filter
*/
struct Ali_DmxDrvTsFltStatInfo
{
	__u32 TsFltIdx;			//!< Index of TS filter
    __u32 TsInCnt;			//!< Count of TS package
    __u32 TsScrmbCnt;		//!< Count of scrambled TS package
    __u32 TsSyncByteErrCnt;	//!< Count of sync error TS package
	__u32 TsDupCnt;			//!< Count of duplicated TS package
	__u32 TsLostCnt;		//!< Count of lost TS package
    __u32 TsErrCnt;			//!< Count of error TS package
};

/*! @struct Ali_DmxDrvSecFltStatInfo
@brief State information of section filter
*/
struct Ali_DmxDrvSecFltStatInfo
{
	__u32 SecFltIdx;			//!< Index of section filter
	__u32 SecInCnt;				//!< Count of section input
	__u32 SecOutCnt;			//!< Count of section output
	__u32 SecTsNoPayloadCnt;	//!< Count of not TS payload
	__u32 SecTsScrmbCnt;		//!< Count of scrambled TS package
	__u32 SecTsDupCnt;			//!< Count of duplicated TS package
	__u32 SecTsLostCnt;			//!< Count of error TS package
	__u32 SecPtErrCnt;			//!< Count of section pointer error
	__u32 SecHdrTooShortCnt;	//!< Count of section header too short error
	__u32 SecDataNotFullCnt;	//!< Count of section data not full error
	__u32 SecDataTooShortCnt;	//!< Count of section data too short error
	__u32 SecMaskTooLongCnt;	//!< Count of section mask too long error
	__u32 SecMaskMismatchCnt;	//!< Count of section mask mismatch error
	__u32 SecBufOverflowCnt;	//!< Count of section buffer overflow error
};

/*! @struct Ali_DmxDrvPesFltStatInfo
@brief State information of PES filter
*/
struct Ali_DmxDrvPesFltStatInfo
{
	__u32 PesFltIdx;			//!< Index of PES filter
	__u32 PesInCnt;				//!< Count of PES input
	__u32 PesOutCnt;			//!< Count of PES output
	__u32 PesHdrErrCnt;			//!< Count of PES header error
	__u32 PesTsNoPayloadCnt;	//!< Count of TS no payload error
	__u32 PesTsScrmbCnt;		//!< Count of TS scrambled
	__u32 PesTsDupCnt;			//!< Count of TS dupicated
	__u32 PesTsLostCnt;			//!< Count of TS lost
	__u32 PesBufOverflowCnt;	//!< Count of PES buffer overflow error
};


/*! @struct Ali_DmxSubtStreamInfo
@brief Information of TP stream.
*/ 
struct Ali_DmxSubtStreamInfo
{
	__s32 StreamIdx;	//!< Stream index
	__s32 StreamState;	//!< Stream state
};


/*! @struct Ali_DmxSeeGlobalStatInfo
@brief Global state Info of demux in see.SEE dmx statistics.
*/ 
struct Ali_DmxSeeGlobalStatInfo
{
	__u32 TotalTsInCnt;		//!< Count of TS package
	__u32 TsOddCnt;			//!< Count of odd TS package
	__u32 TsDecrySucCnt;	//!< Count of decrypt success
	__u32 TsDecryFailCnt;	//!< Count of decrypt fail
	__u32 TsDecryEmptyCnt;	//!< Count of decrypt empty
	__u32 TsRetToMainCnt;	//!< Count of return to main cpu
	__u32 TsSyncErrCnt;		//!< Count of TS sync error
	__u32 Pcr2AvsyncCnt;	//!< Count of Pcr to ansync
	__u32 PcrLastValue;		//!< Last Pcr value
	__u32 ParseStatus;		//!< Parse status
};

/*! @struct Ali_DmxSeePlyChStatInfo
@brief State information of play channel in SEE CPU.
*/
struct Ali_DmxSeePlyChStatInfo 
{
    __u32 TsInCnt;				//!< Count of TS package
    __u32 TsPlayBusyCnt;		//!< Count of play busy
    __u32 TsScrmbCnt;			//!< Count of scrambled TS package
	__u32 TsErrCnt;				//!< Count of error TS package
	
	__u32 PesTsErrCnt;			//!< Count of error TS package in PES parse
	__u32 PesTsScrmbCnt;		//!< Count of scrambled Ts package in PES parse
    __u32 PesTsDupCnt;			//!< Count of duplicated Ts package in PES parse
    __u32 PesTsLostCnt;			//!< Count of lost TS package in PES parse
    __u32 PesHdrLenErrCnt;		//!< Count of PES header length too small error
    __u32 PesHdrScErrCnt;		//!< 
    __u32 PesHdrCnt;			//!< Count of PES header
    __u32 PesScrmbCnt;			//!< Count of scrambled PES
    __u32 PesStreamIdErrCnt;	//!< Count of PES stream id error
    __u32 PesHdrPayloadLenErrCnt;//!< Count of PES header payload length error 
    __u32 PesCallbackNobufCnt;	//!< Count of request PES not buffer error
    __u32 PesReqBufBusyCnt;		//!< Count of request PES busy buffer error
    __u32 PesReqDecStateErrCnt;	//!< Count of request decoded state error
    __u32 PesTsNoPayloadCnt;	//!< Count of not TS payload
    __u32 PesBufOverflowCnt;	//!< Count of PES buffer overflow
    __u32 PesBufUpdateCnt;		//!< Count of PES update
    __u32 PesBufReqCnt;			//!< Count of PES request

	__u16 PlyChPid;				//!< Play channel Pid
};


/*! @struct Ali_DmxSeeStatistics
@brief Demux statistics info in SEE CPU.Must keep compartible with TDS struct Ali_DmxSeeStatistics.
*/
struct Ali_DmxSeeStatistics
{
	/* Video stream statistics
	*/
	__u32 VideoTsInCnt;				//!< Count of video TS package
	__u32 VideoTsLostCnt;			//!< Count of lost video TS package
	__u32 VideoTsDupCnt;			//!< Count of dupicated video TS package
	__u8 VideoTsSrcScramFlag;		//!< Video TS src scramble flag
	__u8 VideoTsCurScramFlag;		//!< Video TS scramble flag
	__u8 VideoTsPusiMismatchFlag;	//!< Video TS pusi mismatch flag
	__u8 VideoReserved;				//!< Reserved

	/* Audio stream statistics
	*/
	__u32 AudioTsInCnt;				//!< Count of audio TS package
	__u32 AudioTsLostCnt;			//!< Count of lost audio TS package
	__u32 AudioTsDupCnt;			//!< Count of dupicated audio TS package
	__u8 AudioTsSrcScramFlag;		//!< Audio TS src scramble flag
	__u8 AudioTsCurScramFlag;		//!< Audio TS scramble flag
	__u8 AudioTsPusiMismatchFlag;	//!< Audio TS pusi mismatch flag
	__u8 AudioReserved;				//!< Reserved

	/* PCR.
	*/
	__u32 Pcr;						//!< Pcr value

	/* Realtime Enable.
	*/
	__u32 RealTimePrintEn;			//!< Interval realtime for print
};

/*! @struct Ali_DmxLibTsStrmStatInfo
@brief Not use now.
*/
struct Ali_DmxLibTsStrmStatInfo
{
	UINT32 StatErrCnt;
	UINT32 InvPathCnt;
	UINT32 IoOpenFailCnt;
	UINT32 IoCfgFailCnt;
	UINT32 IoStartFailCnt;
	UINT32 IoStopFailCnt;
};

/*! @struct Ali_DmxLibTsInRamStrmStatInfo
@brief Not use now.
*/
struct Ali_DmxLibTsInRamStrmStatInfo
{
	UINT32 StatErrCnt;
};

/*! @struct Ali_DmxLibVideoStrmStatInfo
@brief Not use now.
*/
struct Ali_DmxLibVideoStrmStatInfo
{
	UINT32 StatErrCnt;
	UINT32 InvPathCnt;
	UINT32 IoOpenFailCnt;
	UINT32 IoCfgFailCnt;
	UINT32 IoStartFailCnt;
	UINT32 IoStopFailCnt;
};

/*! @struct Ali_DmxLibAudioStrmStatInfo
@brief Not use now.
*/
struct Ali_DmxLibAudioStrmStatInfo
{
	UINT32 StatErrCnt;
	UINT32 InvPathCnt;
	UINT32 IoOpenFailCnt;
	UINT32 IoCfgFailCnt;
	UINT32 IoStartFailCnt;
	UINT32 IoStopFailCnt;
};

/*! @struct Ali_DmxLibPcrStrmStatInfo
@brief Not use now.
*/
struct Ali_DmxLibPcrStrmStatInfo
{
	UINT32 StatErrCnt;
	UINT32 InvPathCnt;
	UINT32 IoOpenFailCnt;
	UINT32 IoCfgFailCnt;
	UINT32 IoStartFailCnt;
	UINT32 IoStopFailCnt;
};

/*! @struct Ali_DmxLibSecStrmStatInfo
@brief Not use now.
*/
struct Ali_DmxLibSecStrmStatInfo
{
	UINT32 StatErrCnt;
	UINT32 InvPathCnt;
	UINT32 CrcErrCnt;
	UINT32 IoOpenFailCnt;
	UINT32 IoCfgFailCnt;
	UINT32 IoStartFailCnt;
	UINT32 IoStopFailCnt;
};

/*! @struct Ali_DmxLibSecSlotStatInfo
@brief Not use now.
*/
struct Ali_DmxLibSecSlotStatInfo
{
	UINT32 CallbackCnt;
	UINT32 CbTimeOutCnt;
	UINT32 StatErrCnt;
	UINT32 NoSecBufCnt;
	UINT32 CallbackErrCnt;
	UINT32 ThreadCreatErrCnt;
	UINT32 StrmOpenFailCnt;
	UINT32 StrmCfgFailCnt;
	UINT32 StrmStartFailCnt;
	UINT32 StrmStopFailCnt;
	UINT32 StrmCloseFailCnt;
};

/*! @struct Ali_DmxLibSecFltStatInfo
@brief Not use now.
*/
struct Ali_DmxLibSecFltStatInfo
{
	UINT32 CallbackCnt;
	UINT32 StatErrCnt;
	UINT32 SlotOpenFailCnt;
	UINT32 SlotCfgFailCnt;
	UINT32 SlotStartFailCnt;
	UINT32 SlotStopFailCnt;
	UINT32 SlotCloseFailCnt;
};

/*! @struct Ali_DmxLibSecChStatInfo
@brief Not use now.
*/
struct Ali_DmxLibSecChStatInfo
{
	UINT32 StatErrCnt;
	UINT32 DupPidCnt;
	UINT32 DupFltCnt;
};

/*! @struct Ali_DmxLibTpStrmStatInfo
@brief Not use now.
*/
struct Ali_DmxLibTpStrmStatInfo
{
	UINT32 StatErrCnt;
	UINT32 InvPathCnt;
	UINT32 IoOpenFailCnt;
	UINT32 IoCfgFailCnt;
	UINT32 IoStartFailCnt;
	UINT32 IoStopFailCnt;
};

/*! @struct Ali_DmxSlotSecStatInfo
@brief Not use now.
*/
struct Ali_DmxSlotSecStatInfo
{
	UINT32 IoOpenFailCnt;
	UINT32 IoStartFailCnt;
	UINT32 SecMaskMissCnt;
	UINT32 SecCrcErrCnt;
	UINT32 SecCallbackCnt;
};

/*! @struct Ali_DmxSlotPesStatInfo
@brief Not use now.
*/
struct Ali_DmxSlotPesStatInfo
{
	UINT32 IoOpenFailCnt;
	UINT32 IoStartFailCnt;
	UINT32 PesHdrErrCnt;
	UINT32 PesBufReqCnt;
	UINT32 PesBufRetCnt;
	UINT32 PesBufEmptyCnt;
	UINT32 PesReqFailCnt;
};

#define ALI_DMX_IOC_MAGIC  0xA1


/* Main line IO commands.
 */

/*!
  \def ALI_DMX_SEC_STREAM_CFG
  Config section stream with parameter.
  struct Ali_DmxSecStreamParam-parameter of section stream.
*/
#define ALI_DMX_SEC_STREAM_CFG _IOW(ALI_DMX_IOC_MAGIC, 500, struct Ali_DmxSecStreamParam)

/*!
  \def ALI_DMX_SEC_STREAM_START
  Start section stream.
*/
#define ALI_DMX_SEC_STREAM_START _IO(ALI_DMX_IOC_MAGIC, 501)

/*!
  \def ALI_DMX_SEC_STREAM_STOP
  Stop section stream.
*/
#define ALI_DMX_SEC_STREAM_STOP _IO(ALI_DMX_IOC_MAGIC, 502)

/*!
  \def ALI_DMX_SEC_STREAM_INFO_GET
  Get info of TS stream.
  struct Ali_DmxDrvSecStrmStatInfo-info of section stream.
*/
#define ALI_DMX_SEC_STREAM_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 503, struct Ali_DmxDrvSecStrmStatInfo)

/*!
  \def ALI_DMX_SEC_FILTER_INFO_GET
  Get info of section filter.
  struct Ali_DmxDrvSecFltStatInfo-info of section filter.
*/
#define ALI_DMX_SEC_FILTER_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 504, struct Ali_DmxDrvSecFltStatInfo)

/*!
  \def ALI_DMX_TS_STREAM_CFG
  Config TS stream with parameter.
  struct Ali_DmxTsStreamParam-parameter of ts stream.
*/
#define ALI_DMX_TS_STREAM_CFG _IOW(ALI_DMX_IOC_MAGIC, 510, struct Ali_DmxTsStreamParam)

/*!
  \def ALI_DMX_TS_STREAM_START
  Start TS stream.
*/
#define ALI_DMX_TS_STREAM_START _IO(ALI_DMX_IOC_MAGIC, 511)

/*!
  \def ALI_DMX_TS_STREAM_STOP
  Stop ts stream.
*/
#define ALI_DMX_TS_STREAM_STOP _IO(ALI_DMX_IOC_MAGIC, 512)

/*!
  \def ALI_DMX_TS_STREAM_INFO_GET
  Get info of TS stream.
  struct Ali_DmxDrvTsStrmStatInfo-info of ts stream.
*/
#define ALI_DMX_TS_STREAM_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 513, struct Ali_DmxDrvTsStrmStatInfo)

/*!
  \def ALI_DMX_TS_FILTER_INFO_GET
  Get info of TS filter.
  struct Ali_DmxDrvTsFltStatInfo-info of ts filter.
*/
#define ALI_DMX_TS_FILTER_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 514, struct Ali_DmxDrvTsFltStatInfo)

/*!
  \def ALI_DMX_AUDIO_STREAM_CFG
  Config audio stream with parameter.
  struct Ali_DmxAudioStreamParam-parameter of audio stream.
*/
#define ALI_DMX_AUDIO_STREAM_CFG _IOW(ALI_DMX_IOC_MAGIC, 520, struct Ali_DmxAudioStreamParam)

/*!
  \def ALI_DMX_AUDIO_STREAM_START
  Start audio stream.
*/
#define ALI_DMX_AUDIO_STREAM_START _IO(ALI_DMX_IOC_MAGIC, 521)

/*!
  \def ALI_DMX_AUDIO_STREAM_STOP
  Stop audio stream.
*/
#define ALI_DMX_AUDIO_STREAM_STOP _IO(ALI_DMX_IOC_MAGIC, 522)

/*!
  \def ALI_DMX_AUDIO_STREAM_INFO_GET
  Get info of audio stream.
  struct Ali_DmxDrvAudioStrmStatInfo-info of audio stream.
*/
#define ALI_DMX_AUDIO_STREAM_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 523, struct Ali_DmxDrvAudioStrmStatInfo)

/*!
  \def ALI_DMX_AUDIO_FILTER_INFO_GET
  Get info of audio filter.
  struct Ali_DmxDrvTsFltStatInfo-info of audio filter.
*/
#define ALI_DMX_AUDIO_FILTER_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 524, struct Ali_DmxDrvTsFltStatInfo)

/*!
  \def ALI_DMX_AUDIO_SEE_INFO_GET
  Get info of demux audio in see cpu.
  struct Ali_DmxSeePlyChStatInfo-info of demux audio in see cpu.
*/
#define ALI_DMX_AUDIO_SEE_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 525, struct Ali_DmxSeePlyChStatInfo)

/*!
  \def ALI_DMX_VIDEO_STREAM_CFG
  Config video stream with parameter.
  struct Ali_DmxVideoStreamParam-parameter of video stream.
*/
#define ALI_DMX_VIDEO_STREAM_CFG _IOW(ALI_DMX_IOC_MAGIC, 530, struct Ali_DmxVideoStreamParam)

/*!
  \def ALI_DMX_VIDEO_STREAM_START
  Start video stream.
*/
#define ALI_DMX_VIDEO_STREAM_START _IO(ALI_DMX_IOC_MAGIC, 531)

/*!
  \def ALI_DMX_VIDEO_STREAM_STOP
  Stop video stream.
*/
#define ALI_DMX_VIDEO_STREAM_STOP _IO(ALI_DMX_IOC_MAGIC, 532)

/*!
  \def ALI_DMX_VIDEO_STREAM_INFO_GET
  Get info of video stream.
  struct Ali_DmxDrvVideoStrmStatInfo-info of video stream.
*/
#define ALI_DMX_VIDEO_STREAM_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 533, struct Ali_DmxDrvVideoStrmStatInfo)

/*!
  \def ALI_DMX_VIDEO_FILTER_INFO_GET
  Get info of video filter.
  struct Ali_DmxDrvTsFltStatInfo-info of video filter.
*/
#define ALI_DMX_VIDEO_FILTER_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 534, struct Ali_DmxDrvTsFltStatInfo)

/*!
  \def ALI_DMX_VIDEO_SEE_INFO_GET
  Get info of demux video in SEE CPU.
  struct Ali_DmxSeePlyChStatInfo-info of demux video in see cpu.
*/
#define ALI_DMX_VIDEO_SEE_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 535, struct Ali_DmxSeePlyChStatInfo)

/*!
  \def ALI_DMX_PCR_STREAM_CFG
  Config pcr stream with parameter.
  struct Ali_DmxPcrStreamParam-parameter of PCR stream.
*/
#define ALI_DMX_PCR_STREAM_CFG _IOW(ALI_DMX_IOC_MAGIC, 540, struct Ali_DmxPcrStreamParam)

/*!
  \def ALI_DMX_PCR_STREAM_START
  Start PCR stream.
*/
#define ALI_DMX_PCR_STREAM_START _IO(ALI_DMX_IOC_MAGIC, 541)

/*!
  \def ALI_DMX_PCR_STREAM_STOP
  Stop PCR stream.
*/
#define ALI_DMX_PCR_STREAM_STOP _IO(ALI_DMX_IOC_MAGIC, 542)

/*!
  \def ALI_DMX_PCR_STREAM_INFO_GET
  Get info of pcr stream.
  struct Ali_DmxDrvPcrStrmStatInfo-info of pcr stream.
*/
#define ALI_DMX_PCR_STREAM_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 543, struct Ali_DmxDrvPcrStrmStatInfo)

/*!
  \def ALI_DMX_PCR_FILTER_INFO_GET
  Get info of PCR filter.
  struct Ali_DmxDrvTsFltStatInfo-info of pcr filter.
*/
#define ALI_DMX_PCR_FILTER_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 544, struct Ali_DmxDrvTsFltStatInfo)

/*!
  \def ALI_DMX_TS_IN_RAM_STREAM_CFG
  Config TS in ram stream with parameter.
  struct Ali_DmxTsInRamStreamParam-parameter of ts in ram stream.
*/
#define ALI_DMX_TS_IN_RAM_STREAM_CFG _IOW(ALI_DMX_IOC_MAGIC, 550, struct Ali_DmxTsInRamStreamParam)

/*!
  \def ALI_DMX_TS_IN_RAM_STREAM_START
  Start TS in ram stream.
*/
#define ALI_DMX_TS_IN_RAM_STREAM_START _IO(ALI_DMX_IOC_MAGIC, 551)

/*!
  \def ALI_DMX_TS_IN_RAM_STREAM_STOP
  Stop TS in ram stream.
*/
#define ALI_DMX_TS_IN_RAM_STREAM_STOP _IO(ALI_DMX_IOC_MAGIC, 552)

/*!
  \def ALI_DMX_TS_IN_RAM_STREAM_INFO_GET
  Get info of TS in ram stream.
  struct Ali_DmxDrvTsInRamStrmStatInfo-info of ts in ram stream.
*/
#define ALI_DMX_TS_IN_RAM_STREAM_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 553, struct Ali_DmxDrvTsInRamStrmStatInfo)

/*!
  \def ALI_DMX_HW_REG_STREAM_CFG
  Config to get hardware register of demux stream.
*/
#define ALI_DMX_HW_REG_STREAM_CFG _IO(ALI_DMX_IOC_MAGIC, 560)

/*!
  \def ALI_DMX_HW_REG_STREAM_START
  Start to get hardware register of demux stream.
*/
#define ALI_DMX_HW_REG_STREAM_START _IO(ALI_DMX_IOC_MAGIC, 561)

/*!
  \def ALI_DMX_HW_REG_STREAM_STOP
  Stop to get hardware register of demux stream	.
*/
#define ALI_DMX_HW_REG_STREAM_STOP _IO(ALI_DMX_IOC_MAGIC, 562)

/*!
  \def ALI_DMX_HW_REG_STREAM_INFO_GET
  Get register info of demux hardware.
  __u32-hardware register buffer pointer, size must sizof(__u32 HwRegTable[18][5]).
*/
#define ALI_DMX_HW_REG_STREAM_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 563, __u32)

/*!
  \def ALI_DMX_KERN_GLB_STREAM_CFG
  Config to get kernel global info of demux stream.
*/
#define ALI_DMX_KERN_GLB_STREAM_CFG _IO(ALI_DMX_IOC_MAGIC, 570)

/*!
  \def ALI_DMX_KERN_GLB_STREAM_START
  Start to get kernel global info of demux stream.
*/
#define ALI_DMX_KERN_GLB_STREAM_START _IO(ALI_DMX_IOC_MAGIC, 571)

/*!
  \def ALI_DMX_KERN_GLB_STREAM_STOP
  Stop to get kernel global info of demux stream.
*/
#define ALI_DMX_KERN_GLB_STREAM_STOP _IO(ALI_DMX_IOC_MAGIC, 572)

/*!
  \def ALI_DMX_KERN_GLB_STREAM_INFO_GET
  Get kernel global info of demux stream.
  struct Ali_DmxKernGlobalStatInfo-kernel global info.
*/
#define ALI_DMX_KERN_GLB_STREAM_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 573, struct Ali_DmxKernGlobalStatInfo)

/*!
  \def ALI_DMX_KERN_GLB_STREAM_REALTIME_SET
  For get kernel global info of demux stream, set interval realtime .
  struct Ali_DmxKernGlobalStatInfo-interval realtime(regard as __U32).
*/
#define ALI_DMX_KERN_GLB_STREAM_REALTIME_SET _IOW(ALI_DMX_IOC_MAGIC, 574, struct Ali_DmxKernGlobalStatInfo)

/*!
  \def ALI_DMX_SEE_GLB_STREAM_CFG
  Config to get see global info of demux stream.
*/
#define ALI_DMX_SEE_GLB_STREAM_CFG _IO(ALI_DMX_IOC_MAGIC, 580)

/*!
  \def ALI_DMX_SEE_GLB_STREAM_START
  Start to get see global info of demux stream.
*/
#define ALI_DMX_SEE_GLB_STREAM_START _IO(ALI_DMX_IOC_MAGIC, 581)

/*!
  \def ALI_DMX_SEE_GLB_STREAM_STOP
  Stop to get see global info of demux stream.
*/
#define ALI_DMX_SEE_GLB_STREAM_STOP _IO(ALI_DMX_IOC_MAGIC, 582)

/*!
  \def ALI_DMX_SEE_GLB_STREAM_INFO_GET
  Get see global info of demux stream.
  struct Ali_DmxSeeGlobalStatInfo-see global info.
*/
#define ALI_DMX_SEE_GLB_STREAM_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 583, struct Ali_DmxSeeGlobalStatInfo)

/*!
  \def ALI_DMX_SEE_GLB_STREAM_REALTIME_SET
  For get see global info of demux stream, set interval realtime .
  struct Ali_DmxSeeGlobalStatInfo-interval realtime(regard as __U32).
*/
#define ALI_DMX_SEE_GLB_STREAM_REALTIME_SET _IOW(ALI_DMX_IOC_MAGIC, 584, struct Ali_DmxSeeGlobalStatInfo)

/*!
  \def ALI_DMX_TP_STREAM_CFG
  Config TP stream with parameter.
  struct Ali_DmxTpStreamParam-parameter of tp stream.
*/
#define ALI_DMX_TP_STREAM_CFG _IOW(ALI_DMX_IOC_MAGIC, 590, struct Ali_DmxTpStreamParam)

/*!
  \def ALI_DMX_TP_STREAM_START
  Start TP stream.
*/
#define ALI_DMX_TP_STREAM_START _IO(ALI_DMX_IOC_MAGIC, 591)

/*!
  \def ALI_DMX_TP_STREAM_STOP
  Stop TP stream.
*/
#define ALI_DMX_TP_STREAM_STOP _IO(ALI_DMX_IOC_MAGIC, 592)

/*!
  \def ALI_DMX_TP_STREAM_INFO_GET
  Get info of TP stream.
  struct Ali_DmxDrvTpStrmStatInfo-info of tp stream.
*/
#define ALI_DMX_TP_STREAM_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 593, struct Ali_DmxDrvTpStrmStatInfo)

/*!
  \def ALI_DMX_SUBT_STREAM_CFG
  Config subtitle stream with parameter.
  struct Ali_DmxSubtStreamParam-parameter of subtitle stream.
*/
#define ALI_DMX_SUBT_STREAM_CFG _IOW(ALI_DMX_IOC_MAGIC, 600, struct Ali_DmxSubtStreamParam)

/*!
  \def ALI_DMX_SUBT_STREAM_START
  Start subtitle stream.
*/
#define ALI_DMX_SUBT_STREAM_START _IO(ALI_DMX_IOC_MAGIC, 601)

/*!
  \def ALI_DMX_SUBT_STREAM_STOP
  Stop subtitle stream.
*/
#define ALI_DMX_SUBT_STREAM_STOP _IO(ALI_DMX_IOC_MAGIC, 602)

/*!
  \def ALI_DMX_SUBT_STREAM_INFO_GET
  Get info of subtitle stream.
  struct Ali_DmxSubtStreamInfo-info of subtitle stream.
*/
#define ALI_DMX_SUBT_STREAM_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 603, struct Ali_DmxSubtStreamInfo)

/*!
  \def ALI_DMX_IN_OUT
  Not use now.
*/
enum ALI_DMX_IN_OUT
{
    ALI_DMX_OUTPUT,
    ALI_DMX_INPUT
};

/*!
  \def ALI_DMX_STREAM_TYPE
  Not use now.
*/
enum ALI_DMX_STREAM_TYPE
{
    ALI_DMX_STREAM_TYPE_SEC = 0x0,
	ALI_DMX_STREAM_TYPE_TS = 0x1000,
	ALI_DMX_STREAM_TYPE_PES = 0x2000,
	ALI_DMX_STREAM_TYPE_PCR = 0x3000,
	ALI_DMX_STREAM_TYPE_AUDIO = 0x4000,
	ALI_DMX_STREAM_TYPE_VIDEO = 0x5000,
	ALI_DMX_STREAM_TYPE_TS_IN_RAM = 0x6000,	
    ALI_DMX_STREAM_TYPE_SEC_SLOT = 0x7000,
    ALI_DMX_STREAM_TYPE_SEC_CH = 0x8000,
    ALI_DMX_STREAM_TYPE_SEC_FLT = 0x9000,
    ALI_DMX_STREAM_TYPE_SRC_CTRL = 0xA000,
    ALI_DMX_STREAM_TYPE_DESCRAM_KEY = 0xB000,   
    ALI_DMX_STREAM_TYPE_TP = 0xC000,
    ALI_DMX_STREAM_TYPE_SUBT = 0xD000,
};

/*!
  \def ALI_DMX_STREAM_ID_RANGE
  Not use now.
*/
enum ALI_DMX_STREAM_ID_RANGE
{
    ALI_DMX_STREAM_ID_SEC_START = 0x0,
	ALI_DMX_STREAM_ID_SEC_END = 0x1000,

    ALI_DMX_STREAM_ID_TS_START = 0x1000,
	ALI_DMX_STREAM_ID_TS_END = 0x2000,

    ALI_DMX_STREAM_ID_PES_START = 0x2000,
	ALI_DMX_STREAM_ID_PES_END = 0x3000,

    ALI_DMX_STREAM_ID_PCR_START = 0x3000,
	ALI_DMX_STREAM_ID_PCR_END = 0x4000,

    ALI_DMX_STREAM_ID_AUDIO_START = 0x4000,
	ALI_DMX_STREAM_ID_AUDIO_END = 0x5000,

    ALI_DMX_STREAM_ID_VIDEO_START = 0x5000,
	ALI_DMX_STREAM_ID_VIDEO_END = 0x6000,

    ALI_DMX_STREAM_ID_IN_TS_RAM_START = 0x6000,
	ALI_DMX_STREAM_ID_IN_TS_RAM_END = 0x7000,

    ALI_DMX_STREAM_ID_SEC_SLOT_START = 0x7000,
	ALI_DMX_STREAM_ID_SEC_SLOT_END = 0x8000,

    ALI_DMX_STREAM_ID_SEC_CH_START = 0x8000,
	ALI_DMX_STREAM_ID_SEC_CH_END = 0x9000,

    ALI_DMX_STREAM_ID_SEC_FLT_START = 0x9000,
	ALI_DMX_STREAM_ID_SEC_FLT_END = 0xA000,

    ALI_DMX_STREAM_ID_SRC_CTRL_START = 0xA000,
	ALI_DMX_STREAM_ID_SRC_CTRL_END = 0xB000,

    ALI_DMX_STREAM_ID_DESCRAM_KEY_START = 0xB000,
	ALI_DMX_STREAM_ID_DESCRAM_KEY_END = 0xC000,	

    ALI_DMX_STREAM_ID_TP_START = 0xC000,
	ALI_DMX_STREAM_ID_TP_END = 0xD000,	

    ALI_DMX_STREAM_ID_SUBT_START = 0xD000,
	ALI_DMX_STREAM_ID_SUBT_END = 0xE000,
};

/*!
@}
*/

/*!
@}
*/
#endif
