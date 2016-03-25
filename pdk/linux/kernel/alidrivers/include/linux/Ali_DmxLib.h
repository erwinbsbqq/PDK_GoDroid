/* DO NOT modify this header file!!!! */
#ifndef _ALI_DMX_LIB_H_
#define _ALI_DMX_LIB_H_

#include <linux/types.h>
#include <linux/Ali_DmxLibInternal.h>
/*! @addtogroup DeviceDriver
 *  @{
 */

/*! @addtogroup ALiDMX
 *  @{
 */

#define INT8   __s8
#define UINT8  __u8
#define INT16  __s16
#define UINT16 __u16
#define INT32  __s32 
#define UINT32 __u32


#define ALI_DMX_SEC_MATCH_MAX_LEN     32 //!< Max filter depth of section 
#define ALI_DMX_TS_STREAM_MAX_PID_CNT 96 //!< Max PID filter of one ts stream


/*! @enum ALI_DMX_ERR_CODE
@brief ALI DMX LIB error code.
	When call ALI DMX LIB interface, please check the return value.
       According to the return value, we can find reason for failure.
*/
enum ALI_DMX_ERR_CODE
{
    ALI_ERR_DMX_OK = 0,//!<OK

	//ALI_ERR_DMX_TIMEOUT = 0,//!<timeout

    ALI_ERR_DMX_NOT_EXIST = -5000,//!<Demux does not exist.
    ALI_ERR_DMX_NOT_INITILAIZED = -5001,//!<Demux is not initilaized.
    ALI_ERR_DMX_HW_FLT_EXHAUST = -5002,//!<Exhaust demux hardware filter
    ALI_ERR_DMX_PTHREAD_CREATE_FAIL = -5003,//!<Failed to create a task.
    ALI_ERR_DMX_INVALID_ID = -5004,//!<Invalid demuxId
    ALI_ERR_DMX_OPER_DENIED = -5005,//!<Illegal operation
    ALI_ERR_DMX_INVALID_PID = -5006,//!<Invalid PID(0 ~ 0x1FFF)
	ALI_ERR_DMX_INVALID_PARAM = -5007,//!<Invalid parameter

    ALI_ERR_DMX_SEC_STREAM_EXHAUST = -5100,//!<Exhaust section stream
    ALI_ERR_DMX_SEC_STREAM_INVALID_ID = -5101,//!<Invalid section stream ID
    ALI_ERR_DMX_SEC_STREAM_MASK_INVALID = -5102,//!<Invalid section stream mask
    ALI_ERR_DMX_SEC_STREAM_TIMEOUT = -5103,//!<Section receive timeout
    ALI_ERR_DMX_SEC_STREAM_OPER_DENIED = -5104,//!<Illegal operation of section stream
	ALI_ERR_DMX_SEC_STREAM_CRC_FAILED = -5105,//!<Data crc failed of section stream

    ALI_ERR_DMX_SEC_SLOT_EXHAUST = -5200,//!<Exhaust section slot
    ALI_ERR_DMX_SEC_SLOT_INVALID_ID = -5201,//!<Invalid section slot ID
    ALI_ERR_DMX_SEC_SLOT_NO_BUF = -5202,//!<Section slot malloc buffer failed
	ALI_ERR_DMX_SEC_SLOT_OPER_DENIED = -5203,//!<Illegal operation of section slot

    ALI_ERR_DMX_SEC_CH_EXHAUST = -5300,//!<Exhaust section channel
    ALI_ERR_DMX_SEC_CH_INVALID_ID = -5301,//!<Invalid section channel ID
    ALI_ERR_DMX_SEC_CH_OPER_DENIED = -5302,//!<Illegal operation of section channel
	ALI_ERR_DMX_SEC_CH_DUPED_PID = -5303,//!<PID conflict of section channel

    ALI_ERR_DMX_SEC_FLT_EXHAUST = -5400,//!<Exhaust section filter
    ALI_ERR_DMX_SEC_FLT_INVALID_ID = -5401,//!<Invalid section filter ID
    ALI_ERR_DMX_SEC_FLT_OPER_DENIED = -5402,//!<Illegal operation of section filter

    ALI_ERR_DMX_VIDEO_STREAM_EXHAUST = -5500,//!<Exhaust video stream
    ALI_ERR_DMX_VIDEO_STREAM_INVALID_ID = -5501,//!<Invalid video stream ID
    ALI_ERR_DMX_VIDEO_STREAM_OPER_DENIED = -5502,//!<Illegal operation of video stream

    ALI_ERR_DMX_AUDIO_STREAM_EXHAUST = -5600,//!<Exhaust audio stream
    ALI_ERR_DMX_AUDIO_STREAM_INVALID_ID = -5601,//!<Invalid audio stream ID
    ALI_ERR_DMX_AUDIO_STREAM_OPER_DENIED = -5602,//!<Illegal operation of audio stream

    ALI_ERR_DMX_PCR_STREAM_EXHAUST = -5700,//!<Exhaust pcr stream
    ALI_ERR_DMX_PCR_STREAM_INVALID_ID = -5701,//!<Invalid pcr stream ID
    ALI_ERR_DMX_PCR_STREAM_OPER_DENIED = -5702,//!<Illegal operation of pcr stream

    ALI_ERR_DMX_TS_STREAM_EXHAUST = -5800,//!<Exhaust ts stream
    ALI_ERR_DMX_TS_STREAM_INVALID_ID = -5801,//!<Invalid ts stream ID
    ALI_ERR_DMX_TS_STREAM_OPER_DENIED = -5802,//!<Illegal operation of ts stream

    ALI_ERR_DMX_TS_IN_RAM_STREAM_EXHAUST = -5900,//!<Exhaust ts in ram stream
    ALI_ERR_DMX_TS_IN_RAM_STREAM_INVALID_ID = -5901,//!<Invalid ts in ram stream ID
    ALI_ERR_DMX_TS_IN_RAM_STREAM_OPER_DENIED = -5902,//!<Illegal operation of ts in ram stream

    ALI_ERR_DMX_SRC_CTRL_STREAM_EXHAUST = -6000,//!<Exhaust demux source stream
    ALI_ERR_DMX_SRC_CTRL_STREAM_INVALID_ID = -6001,//!<Invalid demux source stream
    ALI_ERR_DMX_SRC_CTRL_STREAM_OPER_DENIED = -6002,//!<Illegal operation of demux source stream

    ALI_ERR_DMX_DESCRAM_KEY_INVALID = -6100,//!<Invalid key of descrambler stream
    ALI_ERR_DMX_DESCRAM_KEY_STREAM_EXHAUST = -6101,//!<Exhaust deacrambler stream
    ALI_ERR_DMX_DESCRAM_DRV_ERR = -6102,//!<Deacrambler driver error
    ALI_ERR_DMX_DESCRAM_KEY_STREAM_OPER_DENIED = -6103,//!<Illegal operation of deacrambler stream

    ALI_ERR_DMX_TP_STREAM_EXHAUST = -6200,//!<Exhaust tp stream
    ALI_ERR_DMX_TP_STREAM_INVALID_ID = -6201,//!<Invalid tp stream ID
    ALI_ERR_DMX_TP_STREAM_OPER_DENIED = -6202,//!<Illegal operation of tp stream
};


/*! @enum ALI_DMX_STREAM_STATE
@brief State of demux stream
*/
enum ALI_DMX_STREAM_STATE
{
    ALI_DMX_STREAM_STATE_IDLE = 0,	//!< Idle state of demux stream
    ALI_DMX_STREAM_STATE_CFG = 1,	//!< Config state of demux stream
    ALI_DMX_STREAM_STATE_STOP = 2,	//!< Stop state of demux stream
    ALI_DMX_STREAM_STATE_RUN = 3,	//!< Run state of demux stream
};


/*! @struct Ali_DmxTsStreamParam
@brief Parameter of ts stream. A ts stream can receive multiple PID TS packet at the same time. Users can be specified in the parameter with multiple pid info.
*/
struct Ali_DmxTsStreamParam
{  
    UINT32 PidList[ALI_DMX_TS_STREAM_MAX_PID_CNT]; //!< PID array. The array element is pid value. 
    INT32  PidCnt; //!< The number of PID which need to receive
    UINT32 NeedDiscramble[ALI_DMX_TS_STREAM_MAX_PID_CNT]; //!<PID descramble attribute array. PidList array element and NeedDiscramble array element is one to one correspondence.1-need to descramble 0-not need.
	UINT32 TsFltId[ALI_DMX_TS_STREAM_MAX_PID_CNT]; //!<Index array of PID filter.PidList array element and TsFltId array element is one to one correspondence.
};


/*! @struct Ali_DmxAudioStreamParam
@brief Parameter of audio stream.
*/
struct Ali_DmxAudioStreamParam
{  
    UINT32 Pid;//!< Pid of audio stream
	UINT32 NeedDiscramble;//!< 1-audio need to descramble 0-not need.
};


/*! @struct Ali_DmxVideoStreamParam
@brief Parameter of video stream.
*/
struct Ali_DmxVideoStreamParam
{  
    UINT32 Pid;//!< Pid of video stream
	UINT32 NeedDiscramble;//!< 1-video need to descramble 0-not need.
};


/*! @struct Ali_DmxPcrStreamParam
@brief Parameter of pcr stream.
*/
struct Ali_DmxPcrStreamParam
{  
    UINT32 Pid;//!< Pid of PCR stream¡£
	UINT32 NeedDiscramble;//!< 1-pcr need to descramble 0-not need.
};


/*! @struct Ali_DmxSecMaskFlag
@brief Section stream mask flag
*/
enum Ali_DmxSecMaskFlag
{
    ALI_DMX_SEC_MASK_FLAG_CRC_EN = 0x1,//!< crc check section data
};


/*! @struct Ali_DmxSecMaskInfo
@brief Mask parameter of section stream

       section stream mask rule:
       FOR (i = 0; i < MatchLen; i++)
       {
           IF (Negate[i] == 0)
           {
               IF ((Section[i] & Mask [i]) != (Match[i] & Mask[i]))
               {
                   BREAK;
               }
           }
           ELSE
           {
               IF (Section[i] & Mask [i] == Match[i] &   Mask[i])
               {
                   BREAK;
               }
           }
       }
    
       IF (i >= MatchLen)
       {
           //section mask success and send to user
       }
       ELSE
       {
           //section mask failure and drop it
       }
*/
struct Ali_DmxSecMaskInfo
{
    UINT8 Mask[ALI_DMX_SEC_MATCH_MAX_LEN];//!< Mask of Section stream
    UINT8 Match[ALI_DMX_SEC_MATCH_MAX_LEN];//!< Match of Section stream
    UINT8 Negate[ALI_DMX_SEC_MATCH_MAX_LEN];//!< Negate of Section stream
    UINT32 MatchLen;//!< MatchLen of Section stream
    enum Ali_DmxSecMaskFlag Flags;//!< Section stream mask flag
};


/*! @struct Ali_DmxSecStreamParam
@brief Parameter of section stream
*/
struct Ali_DmxSecStreamParam
{
    UINT32 Pid;//!< Pid of section stream

    struct Ali_DmxSecMaskInfo SecMask;//!< Mask of section stream  

	UINT32 NeedDiscramble;//!< 1-section need to descramble 0-not need.	
};


/*! @struct Ali_DmxPesStreamParam
@brief Parameter of pes stream
*/
struct Ali_DmxPesStreamParam
{  
    UINT32 Pid;//!< Pid of pes stream
	UINT32 NeedDiscramble;//!< 1-pes need to descramble 0-not need.
}; 


/*! @struct Ali_DmxSecSlotParam
@brief Parameter of section slot
*/
struct Ali_DmxSecSlotParam
{
    UINT32 Pid;//!< Pid of Section slot

    struct Ali_DmxSecMaskInfo SecMask;//!<  Mask of Section Slot

    UINT32 Timeout;//!< Timeout unit is ms

    INT32 (*SecRcvCallback)(UINT32  CbParam,//!< Parameter of section receive callback
                            INT32  ErrCode,//!< Error code
                            UINT8  *SecBuf,//!< Section buffer 
                            UINT32  SecLen);//!< Section length

    UINT32 CbParam;//!< First parameter of section receive callback

    UINT32 NeedDiscramble;//!< 1-section need to descramble 0-not need.
};


/*! @struct Ali_DmxSecChParam
@brief Parameter of section channel
*/
struct Ali_DmxSecChParam
{
    UINT32 Pid;//!< Pid of Section channel
    
	UINT32 NeedDiscramble;//!< 1-section need to descramble 0-not need.

    UINT32 NeedNoDupPidCheck; //!< 1-Pid conflict not check 0-check
};


/*! @struct Ali_DmxSecFltParam
@brief Parameter of section filter
*/
struct Ali_DmxSecFltParam
{
    struct Ali_DmxSecMaskInfo SecMask;//!<Mask of Section Filter

    UINT32 Timeout;	//!< Timeout unit is ms    

    INT32 (*SecRcvCallback)(UINT32  CbParam,//!< Parameter of section receive callback
                            INT32  ErrCode,//!< Error code
                            UINT8  *SecBuf,//!< Section buffer 
                            UINT32  SecLen);//!< Section length                            

    UINT32 CbParam;//!< First parameter of section receive callback
};


/*! @struct Ali_DmxTpStreamParam
@brief Parameter of TP stream
*/
struct Ali_DmxTpStreamParam
{
    UINT32 Reserved;//!< Not use now
};


/*! @struct Ali_DmxSubStreamParam
@brief Parameter of subtitle stream
*/
struct Ali_DmxSubtStreamParam
{
    UINT32 Pid;//!< Pid of subtitle stream
};


/*! @struct Ali_DmxTsInRamStreamParam
@brief Parameter of TS in ram stream
*/
struct Ali_DmxTsInRamStreamParam
{
    UINT32 Reserved;//!< Not use now
};


/*! @struct Ali_DmxStreamPollInfo
@brief Parameter of stream poll
*/
struct Ali_DmxStreamPollInfo
{
    INT32 StreamId;//!< Poll stream Id

    INT32 StreamStatus;//!< Poll result is stream status. 1-read/write stream 0-not read/write stream
};


/*! @enum ALI_DMX_INPUT_PORT_ID
@brief Port of demux input source
*/
enum ALI_DMX_INPUT_PORT_ID
{    
    ALI_DMX_INPUT_PORT_SPI_0   = 0x0,//!< Port SPI 0 of input source.
    ALI_DMX_INPUT_PORT_SPI_1   = 0x1,//!< Port SPI 1  of input source.

    ALI_DMX_INPUT_PORT_TSG     = 0x2,//!< Port TSG of input source.

    ALI_DMX_INPUT_PORT_SSI_0   = 0x4,//!< Port SSI 0 of input source.
    ALI_DMX_INPUT_PORT_SSI_1   = 0x5,//!< Port SSI 1 of input source.

    ALI_DMX_INPUT_PORT_PARA    = 0x6,//!< Reserved

    ALI_DMX_INPUT_PORT_SPI2B_0 = 0x8,//!< Port ASSI 0 of input source.
    ALI_DMX_INPUT_PORT_SPI2B_1 = 0x9,//!< Port ASSI 1 of input source.

    ALI_DMX_INPUT_PORT_SPI4B_0 = 0xA,//!< Reserved
    ALI_DMX_INPUT_PORT_SPI4B_1 = 0xB,//!< Reserved

    ALI_DMX_INPUT_PORT_SSI_2   = 0xC,//!< Port SSI 2 of input source.
    ALI_DMX_INPUT_PORT_SSI_3   = 0xD,//!< Port SSI 3 of input source.

    ALI_DMX_INPUT_PORT_SPI2B_2 = 0xE,//!< Port ASSI 2 of input source.
    ALI_DMX_INPUT_PORT_SPI2B_3 = 0xF,//!< Port ASSI 3 of input source.

    ALI_DMX_INPUT_PORT_SPI4B_2 = 0x10,//!< Reserved
    ALI_DMX_INPUT_PORT_SPI4B_3 = 0x11,//!< Reserved
    
};


/*! @enum ALI_DMX_INPUT_PORT_ID
@brief demux input channel
*/
enum ALI_DMX_INPUT_PATH_ID
{    
    ALI_DMX_INPUT_PATH_0   = 0x0,//!< PATH A of input channel.
    ALI_DMX_INPUT_PATH_1   = 0x1,//!< PATH B of input channel.
    ALI_DMX_INPUT_PATH_2   = 0x2,//!< PATH C of input channel.
    ALI_DMX_INPUT_PATH_3   = 0x3,//!< PATH D of input channel.    
};


/*! @struct Ali_DmxSrcCtrlStreamParam
@brief Paramter of demux input
*/
struct Ali_DmxSrcCtrlStreamParam
{
    UINT32                     DmxId;//!< Demux Id
    
    enum ALI_DMX_INPUT_PORT_ID InPutPortId;//!< Port of input source
    UINT32                     InputPortAttr;//!< Port attribute, please refer tsi and frontend datasheet

    enum ALI_DMX_INPUT_PATH_ID InputPathId;//!< Path of input channel
};


/*! @struct Ali_DmxDescramKeyStreamParam
@brief Parameter of descrambler
*/
struct Ali_DmxDescramKeyStreamParam
{
    UINT16 Pid;//!< Pid of descrambler
};


/*! @struct ALi_DmxSysInfo
@brief global info of ALi demux
*/
struct ALi_DmxSysInfo
{
    INT32 HwDmxCntTotal;//!< Number of hardware demux
	INT32 SwDmxCntTotal;//!< Number of software demux

    INT32 SecStreamCntTotal;//!< Number of section stream
    INT32 SecStreamCntFree;//!< Number of free section stream

    INT32 SecSlotCntTotal;//!< Number of section slot
    INT32 SecSlotCntFree;//!< Number of free section slot

    INT32 SecChCntTotal;//!< Number of section channel
    INT32 SecChCntFree;//!< Number of free section channel
    
    INT32 SecFltCntTotal;//!< Number of section filter
    INT32 SecFltCntFree;//!< Number of free section filter

	INT32 VideoStreamCntTotal;//!< Number of video stream
    INT32 VideoStreamCntFree;//!< Number of free video stream

	INT32 AudioStreamCntTotal;//!< Number of audio stream
	INT32 AudioStreamCntFree;//!< Number of free audio stream

    INT32 PcrStreamCntTotal;//!< Number of pcr stream
    INT32 PcrStreamCntFree;//!< Number of free pcr stream

    INT32 TsStreamCntTotal;//!< Number of TS stream
    INT32 TsStreamCntFree;//!< Number of free TS stream

    INT32 TsInRamStreamCntTotal;//!< Number of TS in ram stream
    INT32 TsInRamStreamCntFree;//!< Number of free TS in ram stream stream

    INT32 SrcCtrlStreamCntTotal;//!< Number of source control stream
    INT32 SrcCtrlStreamCntFree;//!< Number of free source control stream
};


/*! @struct ALi_DmxDevInfo
@brief Info of demux device
*/
struct ALi_DmxDevInfo
{
    UINT32 OutputDevTsRcvCnt;//!< Count of received Ts package
};


/*! @struct Ali_DmxSecStreamInfo
@brief Info of ALi demux Section stream
*/
struct Ali_DmxSecStreamInfo
{
    enum ALI_DMX_STREAM_STATE State;//!< State of section stream

	UINT32 DmxId;//!< Demux Id of section stream

    INT32 Idx;//!< Index of section stream array	

    INT32 Fd;//!< Linux demux driver fd for section stream

    struct Ali_DmxSecStreamParam StreamParam;//!< Parameter of section stream from user

	UINT32 InByteCnt;//!< Count of received byte in the section stream	
};


/*! @struct Ali_DmxSecSlotInfo
@brief Info of ALi demux Section slot
*/
struct Ali_DmxSecSlotInfo
{
    enum ALI_DMX_STREAM_STATE State;//!<State of section slot

	UINT32 DmxId;//!< Demux Id of section slot
	
    INT32 Idx;//!< Index of section slot array

    INT32 SecStreamId;//!< Corresponding secton stream of section slot

    INT8 *SecBuf;//!< Buffer of section slot

    UINT32 StartTime;//!< Last callback time of section slot

    struct Ali_DmxSecSlotParam SlotParam;//!< Parameter of Section slot from user

    //pthread_t WorkThreadId;//!<ALI demux lib callback function run in thread, it thread id

	UINT32 InByteCnt;//!< Count of received byte in the section slot	
};


/*! @struct Ali_DmxSecChInfo
@brief Info of ALi demux Section channel
*/
struct Ali_DmxSecChInfo
{
    enum ALI_DMX_STREAM_STATE State;//!<State of section channel

    UINT32 DmxId;//!< Demux Id of section channel

    INT32 Idx;//!< Index of section channel array

    struct Ali_DmxSecChParam ChParam;//!< Parameter of section channelt from user

	UINT32 InByteCnt;//!< Count of received byte in the section channel
};


/*! @struct Ali_DmxSecFltInfo
@brief Info of ALi demux Section filter
*/
struct Ali_DmxSecFltInfo
{
    enum ALI_DMX_STREAM_STATE State;//!< State of section filter

    INT32 Idx; //!< Demux Id of section filter
    
    INT32 ChIdx;//!< Corresponding section channel index of section filter

    INT32 SecSlotId;//!< Corresponding section slot Id of section filter
    
    struct Ali_DmxSecFltParam FltParam;//!< Parameter of section filter from user

    UINT32 InByteCnt;//!< Count of received byte in the section filter
};


/*! @struct Ali_DmxVideoStreamInfo
@brief Info of ALi demux Video stream
*/
struct Ali_DmxVideoStreamInfo
{
    enum ALI_DMX_STREAM_STATE State;//!< State of video stream

	UINT32 DmxId;//!< Demux Id of video stream

    UINT32 Idx; //!< Index of video stream array

    INT32 Fd;//!< Linux demux driver fd for video stream  

    struct Ali_DmxVideoStreamParam StreamParam;//!< Parameter of video stream from user

    UINT32 InByteCnt;//!< Count of received byte in the video stream
    
	UINT32 DmaChannelId;//!< Dma channel Id for video stream data transport

    /* TS layer info.
	*/
    UINT32 TsInCnt;			//!< Count of Ts package in video stream
    UINT32 TsScrmbCnt;		//!< Count of scrambled Ts package in video stream
    UINT32 TsSyncByteErrCnt;//!< Count of sync error Ts package in video stream
	UINT32 TsDupCnt;		//!< Count of duplicated Ts package in video stream
	UINT32 TsLostCnt;		//!< Count of lost Ts package in video stream
    UINT32 TsErrCnt;		//!< Count of error Ts package in video stream
};


/*! @struct Ali_DmxAudioStreamInfo
@brief Info of ALi demux Audio stream
*/
struct Ali_DmxAudioStreamInfo
{
    enum ALI_DMX_STREAM_STATE State;//!< State if audio stream
    
	UINT32 DmxId;//!< Demux Id of audio stream

    UINT32 Idx;//!< Index of audio stream array

    INT32 Fd;//!< Linux demux driver fd for audio stream 

    struct Ali_DmxAudioStreamParam StreamParam;//!< Parameter of audio stream from user

    UINT32 InByteCnt;//!< Count of received byte in the audio stream

    /* TS layer info.
	*/
    UINT32 TsInCnt;			//!< Count of Ts package in audio stream
    UINT32 TsScrmbCnt;		//!< Count of scrambled Ts package in audio stream
    UINT32 TsSyncByteErrCnt;//!< Count of sync error Ts package in audio stream
	UINT32 TsDupCnt;		//!< Count of duplicated Ts package in audio stream
	UINT32 TsLostCnt;		//!< Count of lost Ts package in audio stream
    UINT32 TsErrCnt;		//!< Count of error Ts package in audio stream
};


/*! @struct Ali_DmxPcrStreamInfo
@brief Info of ALi demux Pcr stream
*/
struct Ali_DmxPcrStreamInfo
{
    enum ALI_DMX_STREAM_STATE State;//!< State of pcr stream

	UINT32 DmxId;//!< Demux Id of pcr stream	

    UINT32 Idx;//!< Index of pcr stream array

    INT32 Fd;//!< Linux demux driver fd for pcr stream 

    struct Ali_DmxPcrStreamParam StreamParam;//!< Parameter of pcr stream from user

    UINT32 InByteCnt;//!< Count of received byte in the pcr stream
};


/*! @struct Ali_DmxTsStreamInfo
@brief Info of ALi demux TS stream
*/
struct Ali_DmxTsStreamInfo
{
    enum ALI_DMX_STREAM_STATE State;//!< State of TS stream

	UINT32 DmxId;//!< Demux Id of TS stream	

    UINT32 Idx; //!< Index of TS stream array

    INT32 Fd;//!< Linux demux driver fd for TS stream

    struct Ali_DmxTsStreamParam StreamParam;//!< Parameter of TS stream from user

    UINT32 InByteCnt;//!< Count of received byte in the TS Stream
};


/*! @struct Ali_DmxTsInRamStreamInfo
@brief Info of ALi demux TS IN RAM stream
*/
struct Ali_DmxTsInRamStreamInfo
{
    enum ALI_DMX_STREAM_STATE State;//!< State of TS IN RAM stream

	UINT32 DmxId;//!< Demux Id of TS IN RAM stream

    UINT32 Idx; //!< Index of TS IN RAM stream array

    INT32 Fd;//!< Linux demux driver fd for TS IN RAM stream

    struct Ali_DmxTsInRamStreamParam StreamParam;//!< Parameter of TS IN RAM stream from user

    UINT32 InByteCnt;//!< Count of received byte in the TS IN RAM Stream
};


/*! @struct Ali_DmxSrcCtrlStreamInfo
@brief Info of ALi demux source control stream
*/
struct Ali_DmxSrcCtrlStreamInfo
{
    enum ALI_DMX_STREAM_STATE State;//!< State of source control stream
    
	UINT32 DmxId;//!< Demux Id of source control stream

    UINT32 Idx;//!< Index of source control stream	

    INT32 Fd;//!< Linux demux driver fd for source control stream

    struct Ali_DmxSrcCtrlStreamParam StreamParam;//!< Parameter of source control stream from user
};

/*!
@}
*/

/*!
@}
*/
#endif
