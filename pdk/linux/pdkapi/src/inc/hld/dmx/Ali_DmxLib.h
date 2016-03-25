/* DO NOT modify this header file!!!! */
#ifndef _ALI_DMX_LIB_H_
#define _ALI_DMX_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <adr_basic_types.h>
/*! @addtogroup dmxlib
 *  @{
 */

#define ALI_DMX_SEC_MATCH_MAX_LEN     32 //!<section mask��ȣ�����Ķ���
#define ALI_DMX_TS_STREAM_MAX_PID_CNT 96 //!<һ��TS stream������ͬʱ��ȡ��TS PID����������Ķ���


/*! @enum ALI_DMX_ERR_CODE
@brief ALI DMX LIB�Ĵ����룬����ALI DMX LIB�Ľӿ�ʱ����ע���鷵��ֵ��
       ���ݷ���ֵ�����жϽӿڵ����Ƿ���������Լ����ֵ�����ԭ��
*/
enum ALI_DMX_ERR_CODE
{
    ALI_ERR_DMX_OK = 0,//!<OK

	//ALI_ERR_DMX_TIMEOUT = 0,//!<��ʱ

    ALI_ERR_DMX_NOT_EXIST = -5000,//!<ָ����DMX������
    ALI_ERR_DMX_NOT_INITILAIZED = -5001,//!<ָ����DMXδ��ʼ��
    ALI_ERR_DMX_HW_FLT_EXHAUST = -5002,//!<ָ����DMXӲ������������
    ALI_ERR_DMX_PTHREAD_CREATE_FAIL = -5003,
    ALI_ERR_DMX_INVALID_ID = -5004,//!<DMX ID������Χ
    ALI_ERR_DMX_OPER_DENIED = -5005,//!<��DMX�Ƿ�����
    ALI_ERR_DMX_INVALID_PID = -5006,//!<PID��������Χ��(0 ~ 0x1FFF)
	ALI_ERR_DMX_INVALID_PARAM = -5007,//!<�Ƿ������ò���

    ALI_ERR_DMX_SEC_STREAM_EXHAUST = -5100,//!<section stream����
    ALI_ERR_DMX_SEC_STREAM_INVALID_ID = -5101,//!<section stream ID������Χ
    ALI_ERR_DMX_SEC_STREAM_MASK_INVALID = -5102,//!<section stream mask���ô���
    ALI_ERR_DMX_SEC_STREAM_TIMEOUT = -5103,//!<section���ճ�ʱ
    ALI_ERR_DMX_SEC_STREAM_OPER_DENIED = -5104,//!<section stream�ӿڷǷ�����
	ALI_ERR_DMX_SEC_STREAM_CRC_FAILED = -5105,//!<section stream����CRCУ��ʧ��

    ALI_ERR_DMX_SEC_SLOT_EXHAUST = -5200,//!<section slot����
    ALI_ERR_DMX_SEC_SLOT_INVALID_ID = -5201,//!<section slot ID������Χ
    ALI_ERR_DMX_SEC_SLOT_NO_BUF = -5202,//!<section slot�����ڴ�ʧ��
	ALI_ERR_DMX_SEC_SLOT_OPER_DENIED = -5203,//!<section slot�ӿڷǷ�����

    ALI_ERR_DMX_SEC_CH_EXHAUST = -5300,//!<section channel����
    ALI_ERR_DMX_SEC_CH_INVALID_ID = -5301,//!<section channel ID������Χ
    ALI_ERR_DMX_SEC_CH_OPER_DENIED = -5302,//!<section channel�ӿڷǷ�����
	ALI_ERR_DMX_SEC_CH_DUPED_PID = -5303,//!<������������ͬ��PID����ͬ��channel��

    ALI_ERR_DMX_SEC_FLT_EXHAUST = -5400,//!<section filter����
    ALI_ERR_DMX_SEC_FLT_INVALID_ID = -5401,//!<section filter ID������Χ
    ALI_ERR_DMX_SEC_FLT_OPER_DENIED = -5402,//!<section filter�ӿڷǷ�����

    ALI_ERR_DMX_VIDEO_STREAM_EXHAUST = -5500,//!<Video stream����
    ALI_ERR_DMX_VIDEO_STREAM_INVALID_ID = -5501,//!<Video stream ID������Χ
    ALI_ERR_DMX_VIDEO_STREAM_OPER_DENIED = -5502,//!<Video stream�ӿڷǷ�����

    ALI_ERR_DMX_AUDIO_STREAM_EXHAUST = -5600,//!<Audio stream����
    ALI_ERR_DMX_AUDIO_STREAM_INVALID_ID = -5601,//!<Audio stream ID������Χ
    ALI_ERR_DMX_AUDIO_STREAM_OPER_DENIED = -5602,//!<Audio stream�ӿڷǷ�����

    ALI_ERR_DMX_PCR_STREAM_EXHAUST = -5700,//!<Audio stream����
    ALI_ERR_DMX_PCR_STREAM_INVALID_ID = -5701,//!<Audio stream ID������Χ
    ALI_ERR_DMX_PCR_STREAM_OPER_DENIED = -5702,//!<Audio stream�ӿڷǷ�����

    ALI_ERR_DMX_TS_STREAM_EXHAUST = -5800,//!<TS stream����
    ALI_ERR_DMX_TS_STREAM_INVALID_ID = -5801,//!<TS stream ID������Χ
    ALI_ERR_DMX_TS_STREAM_OPER_DENIED = -5802,//!<TS stream�ӿڷǷ�����

    ALI_ERR_DMX_TS_IN_RAM_STREAM_EXHAUST = -5900,//!<TS in stream����(��RAM����TS����ʽ��DMX)
    ALI_ERR_DMX_TS_IN_RAM_STREAM_INVALID_ID = -5901,//!<TS in stream ID������Χ
    ALI_ERR_DMX_TS_IN_RAM_STREAM_OPER_DENIED = -5902,//!<TS in stream�ӿڷǷ�����

    ALI_ERR_DMX_SRC_CTRL_STREAM_EXHAUST = -6000,//!<DMX��Դ�ڵ�����������
    ALI_ERR_DMX_SRC_CTRL_STREAM_INVALID_ID = -6001,//!<DMX��Դ�ڵ�������ID������Χ
    ALI_ERR_DMX_SRC_CTRL_STREAM_OPER_DENIED = -6002,//!<DMX��Դ�ڵ��������ӿڷǷ�����

    ALI_ERR_DMX_DESCRAM_KEY_INVALID = -6100,//!<�Ƿ���CW�֡�
    ALI_ERR_DMX_DESCRAM_KEY_STREAM_EXHAUST = -6101,//!<Deacrambler Key stream����
    ALI_ERR_DMX_DESCRAM_DRV_ERR = -6102,//!<�ڲ�Deacrambler Driver����
    ALI_ERR_DMX_DESCRAM_KEY_STREAM_OPER_DENIED = -6103,//!<Deacrambler Key stream�ӿڷǷ�����

    ALI_ERR_DMX_TP_STREAM_EXHAUST = -6200,//!<TP stream����
    ALI_ERR_DMX_TP_STREAM_INVALID_ID = -6201,//!<TP stream ID������Χ
    ALI_ERR_DMX_TP_STREAM_OPER_DENIED = -6202,//!<TP stream�ӿڷǷ�����
};


/*! @enum ALI_DMX_STREAM_STATE
@brief ALI DMX LIB��stream״̬����
*/
enum ALI_DMX_STREAM_STATE
{
    ALI_DMX_STREAM_STATE_IDLE = 0,
    ALI_DMX_STREAM_STATE_CFG = 1,
    ALI_DMX_STREAM_STATE_STOP = 2,
    ALI_DMX_STREAM_STATE_RUN = 3,
};


/*! @struct Ali_DmxTsStreamParam
@brief ���ô�DMX��ȡTS���Ĳ�����һ��TS������ͬʱ���ն��PID��TS�����û�����
       �ڲ�����ָ����Ҫ���յ�TS��PID��
*/
struct Ali_DmxTsStreamParam
{  
    UINT32 PidList[ALI_DMX_TS_STREAM_MAX_PID_CNT]; //!< PID���飬һ����Ԫ��Ӧһ����Ҫ���յ�TS����PID��
    INT32  PidCnt; //!<��Ҫ���յ�PID�ĸ�����
    UINT32 NeedDiscramble[ALI_DMX_TS_STREAM_MAX_PID_CNT]; //!<ÿһ����Ԫ��ӦPidList�е�һ��PID����ʾ���PID�������Ƿ���Ҫ���š���0����Ҫ; 0,����Ҫ��
	UINT32 TsFltId[ALI_DMX_TS_STREAM_MAX_PID_CNT]; //!<ÿһ����Ԫ��Ӧһ��PID��������������
};


/*! @struct Ali_DmxAudioStreamParam
@brief ����ALI DMX Audio stream�Ĳ�����
*/
struct Ali_DmxAudioStreamParam
{  
    UINT32 Pid;//!<Audio stream��PID��
	UINT32 NeedDiscramble;//!Audio�����Ƿ���Ҫ���š�1����Ҫ; 0,����Ҫ��
};


/*! @struct Ali_DmxVideoStreamParam
@brief ����ALI DMX Audio stream�Ĳ�����
*/
struct Ali_DmxVideoStreamParam
{  
    UINT32 Pid;//!<Video stream��PID��
	UINT32 NeedDiscramble;//!Video�����Ƿ���Ҫ���š�1����Ҫ; 0,����Ҫ��
};


/*! @struct Ali_DmxPcrStreamParam
@brief ����ALI DMX PCR stream�Ĳ�����
*/
struct Ali_DmxPcrStreamParam
{  
    UINT32 Pid;//!<PCR stream��PID��
	UINT32 NeedDiscramble;//!PCR�����Ƿ���Ҫ���š�1����Ҫ; 0,����Ҫ��
};


/*! @struct Ali_DmxSecMaskFlag
@brief ����ALI DMX section Mask�Ĺ��˺��������
*/
enum Ali_DmxSecMaskFlag
{
    ALI_DMX_SEC_MASK_FLAG_CRC_EN = 0x1,//!<�������е�Section�����Ƿ���ҪCRCУ�飬δͨ��CRCУ���Section�ᱻ������1����Ҫ; 0,����Ҫ��
};


/*! @struct Ali_DmxSecMaskInfo
@brief ����ALI DMX section stream��MASK������

       ���˹���:
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
           //����sectionͨ���������������͸��û���
       }
       ELSE
       {
           //sectionδͨ������������������������
       }
*/
struct Ali_DmxSecMaskInfo
{
    UINT8 Mask[ALI_DMX_SEC_MATCH_MAX_LEN];//!<Section stream��Mask��
    UINT8 Match[ALI_DMX_SEC_MATCH_MAX_LEN];//!<Section stream��Match��
    UINT8 Negate[ALI_DMX_SEC_MATCH_MAX_LEN];//!<Section stream��Negate��
    UINT32 MatchLen;//!<Section stream��MatchLen��
    enum Ali_DmxSecMaskFlag Flags;//!<Section stream�Ĺ��˺��������
};



/*! @struct Ali_DmxSecStreamParam
@brief ����ALI DMX section stream��PID�����˲���
*/
struct Ali_DmxSecStreamParam
{
    UINT32 Pid;//!<Section stream��PID��    

    struct Ali_DmxSecMaskInfo SecMask;//!<Section stream��Mask��  

	UINT32 NeedDiscramble;//!section TS�������Ƿ���Ҫ���š�1����Ҫ; 0,����Ҫ��	
};


/*! @struct Ali_DmxPesStreamParam
@brief ����ALI DMX PES stream�Ĳ�����
*/
struct Ali_DmxPesStreamParam
{  
    UINT32 Pid;//!<PES stream��PID��
	UINT32 NeedDiscramble;//!<PES�����Ƿ���Ҫ���š�1����Ҫ; 0,����Ҫ��
}; 



/*! @struct Ali_DmxSecSlotParam
@brief ����ALI DMX section slot�Ĳ���
*/
struct Ali_DmxSecSlotParam
{
    UINT32 Pid;//!<Section slot��PID��  

    struct Ali_DmxSecMaskInfo SecMask;//!<Section Slot��Mask��

    UINT32 Timeout;//!<��ʱʱ�䣬�Ժ���Ϊ��λ��

    INT32 (*SecRcvCallback)(UINT32  CbParam,//!<�û�ע��Ļص�������
                            INT32  ErrCode,//!<�����롣
                            UINT8  *SecBuf,//!<section���ݵ���ʼ��ַ��
                            UINT32  SecLen);//!<section�ĳ��ȡ�

    UINT32 CbParam;//!<�ص�����SecRcvCallback�ĵ�һ��������

    UINT32 NeedDiscramble;//!section TS�������Ƿ���Ҫ���š�1����Ҫ; 0,����Ҫ��	
};


/*! @struct Ali_DmxSecChParam
@brief ����ALI DMX section channel�Ĳ���
*/
struct Ali_DmxSecChParam
{
    UINT32 Pid;//!<Section channel��PID�� 
    
	UINT32 NeedDiscramble;//!section TS�������Ƿ���Ҫ���š�1����Ҫ; 0,����Ҫ��

    UINT32 NeedNoDupPidCheck; //!�ж��Ƿ���Ҫִ�м���ظ�PID�Ĳ�����1������Ҫ��飻0����Ҫ���
};


/*! @struct Ali_DmxSecFltParam
@brief ����ALI DMX section fiter�Ĳ���
*/
struct Ali_DmxSecFltParam
{
    struct Ali_DmxSecMaskInfo SecMask;//!<Section Filter��Mask��

    UINT32 Timeout;//!<��ʱʱ�䣬�Ժ���Ϊ��λ��

    INT32 (*SecRcvCallback)(UINT32  CbParam,//!<�û�ע��Ļص�������
                            INT32  ErrCode,//!<�����롣
                            UINT8  *SecBuf,//!<section���ݵ���ʼ��ַ��
                            UINT32  SecLen);//!<section�ĳ��ȡ�

    UINT32 CbParam;//!<�ص�����SecRcvCallback�ĵ�һ��������
};


/*! @struct Ali_DmxTpStreamParam
@brief ����¼������TP���ݵĲ���
*/
struct Ali_DmxTpStreamParam
{
    UINT32 Reserved;//!<������Ŀǰ��ʵ����;��
};



/*! @struct Ali_DmxSubStreamParam
@brief ����subttile TS����PID
*/
struct Ali_DmxSubtStreamParam
{
    UINT32 Pid;//!<subtitle TS����PID�� 
};



/*! @struct Ali_DmxTsInRamStreamParam
@brief ����ALI DMX TS IN RAM�Ĳ���
*/
struct Ali_DmxTsInRamStreamParam
{
    UINT32 Reserved;//!<������Ŀǰ��ʵ����;��
};


/*! @struct Ali_DmxStreamPollInfo
@brief ����stream poll�Ĳ���
*/
struct Ali_DmxStreamPollInfo
{
    INT32 StreamId;//!<��Ҫ��ѯ��Stream��ID��

    INT32 StreamStatus;//!<��Ҫ��ѯ��Stream�Ƿ������ݿɹ��������пռ�ɹ�д��1�����Զ�/д��0,��δΪ��/д׼����
};


/*! @enum ALI_DMX_INPUT_PORT_ID
@brief ALIӲDMX����Դ�˿ڡ�
*/
enum ALI_DMX_INPUT_PORT_ID
{    
    ALI_DMX_INPUT_PORT_SPI_0   = 0x0,//!<ӲDMX������˿�ΪSPI 0.
    ALI_DMX_INPUT_PORT_SPI_1   = 0x1,//!<ӲDMX������˿�ΪSPI 1.

    ALI_DMX_INPUT_PORT_TSG     = 0x2,//!<ӲDMX������˿�ΪTSG.

    ALI_DMX_INPUT_PORT_SSI_0   = 0x4,//!<ӲDMX������˿�ΪSSI 0.
    ALI_DMX_INPUT_PORT_SSI_1   = 0x5,//!<ӲDMX������˿�ΪSSI 1.

    ALI_DMX_INPUT_PORT_PARA    = 0x6,//!<����.

    ALI_DMX_INPUT_PORT_SPI2B_0 = 0x8,//!<ӲDMX������˿�ΪASSI 0.
    ALI_DMX_INPUT_PORT_SPI2B_1 = 0x9,//!<ӲDMX������˿�ΪASSI 1.

    ALI_DMX_INPUT_PORT_SPI4B_0 = 0xA,//!<����.
    ALI_DMX_INPUT_PORT_SPI4B_1 = 0xB,//!<����.

    ALI_DMX_INPUT_PORT_SSI_2   = 0xC,//!<ӲDMX������˿�ΪSSI 2.
    ALI_DMX_INPUT_PORT_SSI_3   = 0xD,//!<ӲDMX������˿�ΪSSI 3.

    ALI_DMX_INPUT_PORT_SPI2B_2 = 0xE,//!<ӲDMX������˿�ΪASSI 2.
    ALI_DMX_INPUT_PORT_SPI2B_3 = 0xF,//!<ӲDMX������˿�ΪASSI 3.

    ALI_DMX_INPUT_PORT_SPI4B_2 = 0x10,//!<����.
    ALI_DMX_INPUT_PORT_SPI4B_3 = 0x11,//!<����.

    
};



/*! @enum ALI_DMX_INPUT_PORT_ID
@brief ALIӲDMX����Դ����ͨ����
*/
enum ALI_DMX_INPUT_PATH_ID
{    
    ALI_DMX_INPUT_PATH_0   = 0x0,//!<ӲDMX����������Դ����ͨ��PATH A.
    ALI_DMX_INPUT_PATH_1   = 0x1,//!<ӲDMX����������Դ����ͨ��PATH B.
    ALI_DMX_INPUT_PATH_2   = 0x2,//!<ӲDMX����������Դ����ͨ��PATH C.
    ALI_DMX_INPUT_PATH_3   = 0x3,//!<ӲDMX����������Դ����ͨ��PATH D.    
};


/*! @struct Ali_DmxSrcCtrlStreamParam
@brief ALIӲDMX����Դ���á�
*/
struct Ali_DmxSrcCtrlStreamParam
{
    UINT32                     DmxId;//!<ӲDMX��ID��
    
    enum ALI_DMX_INPUT_PORT_ID InPutPortId;//!<ӲDMX������Դ�˿ںţ���μ�enum ALI_DMX_INPUT_PORT_ID��
    UINT32                     InputPortAttr;//!<ӲDMX������Դ�˿ڵĿ��ƼĴ�������;(bit0~bit7)����ο�Ӳ��TSI SPEC; (bit8), ����SPI/SSI Error Signal Disable Control.
    
    enum ALI_DMX_INPUT_PATH_ID InputPathId;//!<ӲDMX��Դ����ͨ������ο�enum ALI_DMX_INPUT_PATH_ID��
};


/*! @struct Ali_DmxDescramKeyStreamParam
@brief ����CSA���Ų�����
*/
struct Ali_DmxDescramKeyStreamParam
{
    UINT16 Pid;
};



/*! @struct ALi_DmxSysInfo
@brief ALI DMX��ȫ����Ϣ��
*/
struct ALi_DmxSysInfo
{
    INT32 HwDmxCntTotal;//!<ӲDMX���ܸ�����
	INT32 SwDmxCntTotal;//!<��DMX���ܸ�����

    INT32 SecStreamCntTotal;//!<Section stream���ܸ�����
    INT32 SecStreamCntFree;//!<��δ��ռ�ã��ɹ�ʹ�õ�Section stream�ĸ�����

    INT32 SecSlotCntTotal;//!<Section slot���ܸ�����
    INT32 SecSlotCntFree;//!<��δ��ռ�ã��ɹ�ʹ�õ�Section slot�ĸ�����

    INT32 SecChCntTotal;//!<Section channel���ܸ�����
    INT32 SecChCntFree;//!<��δ��ռ�ã��ɹ�ʹ�õ�Section channel�ĸ�����

    INT32 SecFltCntTotal;//!<Section filter���ܸ�����
    INT32 SecFltCntFree;//!<��δ��ռ�ã��ɹ�ʹ�õ�Section filter�ĸ�����

	INT32 VideoStreamCntTotal;//!<Video filter���ܸ�����
    INT32 VideoStreamCntFree;//!<��δ��ռ�ã��ɹ�ʹ�õ�Video filter�ĸ�����

	INT32 AudioStreamCntTotal;//!<Audio stream���ܸ�����
    INT32 AudioStreamCntFree;//!<��δ��ռ�ã��ɹ�ʹ�õ�Audio stream�ĸ�����

    INT32 PcrStreamCntTotal;//!<Pcr stream���ܸ�����
    INT32 PcrStreamCntFree;//!<��δ��ռ�ã��ɹ�ʹ�õ�Pcr stream�ĸ�����

    INT32 TsStreamCntTotal;//!<TS stream���ܸ�����
    INT32 TsStreamCntFree;//!<��δ��ռ�ã��ɹ�ʹ�õ�TS stream�ĸ�����

    INT32 TsInRamStreamCntTotal;//!<TS in ram stream���ܸ�����
    INT32 TsInRamStreamCntFree;//!<��δ��ռ�ã��ɹ�ʹ�õ�TS in ram stream stream�ĸ�����

    INT32 SrcCtrlStreamCntTotal;//!<Source control stream���ܸ�����
    INT32 SrcCtrlStreamCntFree;//!<��δ��ռ�ã��ɹ�ʹ�õ�Source control stream�ĸ�����
};

/*! @struct ALi_DmxDevInfo
@brief һ��DMX��״̬��������Ϣ��
*/
struct ALi_DmxDevInfo
{
    UINT32 OutputDevTsRcvCnt;//!<������յ���TS�����ܸ�����
};


/*! @struct Ali_DmxSecStreamInfo
@brief ALI DMX Section stream����Ϣ��
*/
struct Ali_DmxSecStreamInfo
{
    enum ALI_DMX_STREAM_STATE State;//!<Section stream��״̬��

	UINT32 DmxId;//!<Section stream��Ӧ��DMX��ID��

    INT32 Idx;//!<Section stream���ڲ������е�Index��	

    INT32 Fd;//!<Section stream��Ӧ��linux DMX driver���ļ��������� 

    struct Ali_DmxSecStreamParam StreamParam;//!<�û����õ�Section stream������ 

	UINT32 InByteCnt;//!<Section stream�ܹ��յ���byte������	
};



/*! @struct Ali_DmxSecSlotInfo
@brief ALI DMX Section slot����Ϣ��
*/
struct Ali_DmxSecSlotInfo
{
    enum ALI_DMX_STREAM_STATE State;//!<Section slot��״̬��

	UINT32 DmxId;//!<Section slot��Ӧ��DMX��ID��

    INT32 Idx;//!<Section slot���ڲ������е�Index��

    INT32 SecStreamId;//!<Section slot��Ӧ��secton stream��ID��

    INT8 *SecBuf;//!<Section slot�ڲ�ʹ�õĻ���buffer�ĵ�ַ��

    UINT32 StartTime;//!<Section slot���һ�δ�callback����ʱ��ʱ�䡣

    struct Ali_DmxSecSlotParam SlotParam;//!<�û����õ�Section slot������ 

    //pthread_t WorkThreadId;//!<ALI DMX lib�ڲ����Ե���callback������thread��ID�� 

	UINT32 InByteCnt;//!<Section slot�ܹ��յ���byte������		
};



/*! @struct Ali_DmxSecChInfo
@brief ALI DMX Section channel����Ϣ��
*/
struct Ali_DmxSecChInfo
{
    enum ALI_DMX_STREAM_STATE State;//!<Section channel��״̬�� 

    UINT32 DmxId;//!<Section channel��Ӧ��DMX��ID��

    INT32 Idx;//!<Section channel���ڲ������е�Index��

    struct Ali_DmxSecChParam ChParam;//!<�û����õ�Section channel������

	UINT32 InByteCnt;//!<Section channel�ܹ��յ���byte����.
};





/*! @struct Ali_DmxSecFltInfo
@brief ALI DMX Section filter����Ϣ��
*/
struct Ali_DmxSecFltInfo
{
    enum ALI_DMX_STREAM_STATE State;//!<Section filter��״̬��

    INT32 Idx; //!<Section filter���ڲ������е�Index��	

    INT32 ChIdx;//!<Section filter���ڵĵ�Section channel��IDX��

    INT32 SecSlotId;//!<Section filterʹ�õ�Section slot��IDX��

    struct Ali_DmxSecFltParam FltParam;//!<�û����õ�Section filter������

    UINT32 InByteCnt;//!<Section filter�ܹ��յ���byte����.
};




/*! @struct Ali_DmxVideoStreamInfo
@brief ALI DMX Video stream����Ϣ��
*/
struct Ali_DmxVideoStreamInfo
{
    enum ALI_DMX_STREAM_STATE State;//!<Video stream��״̬.

	UINT32 DmxId;//!<Video stream��Ӧ��DMX��ID��	

    UINT32 Idx; //!<Video stream���ڲ������е�Index��	

    INT32 Fd;//!<Video stream��Ӧ��linux DMX driver���ļ���������  

    struct Ali_DmxVideoStreamParam StreamParam;//!<�û����õ�Video stream������

    UINT32 InByteCnt;//!<Video Stream�ܹ��յ���byte����.	
    
	UINT32 DmaChannelId;//!<Driver�ڲ�ʹ�õĴ���VIDEO���ݵ�DMA Channel��ID.	

    /* TS layer info.
	*/
    UINT32 TsInCnt;
    UINT32 TsScrmbCnt;
    UINT32 TsSyncByteErrCnt;
	UINT32 TsDupCnt;
	UINT32 TsLostCnt;
    UINT32 TsErrCnt;
};




/*! @struct Ali_DmxAudioStreamInfo
@brief ALI DMX Audio stream����Ϣ��
*/
struct Ali_DmxAudioStreamInfo
{
    enum ALI_DMX_STREAM_STATE State;//!<Audio stream��״̬.

	UINT32 DmxId;//!<Audio stream��Ӧ��DMX��ID��	

    UINT32 Idx;//!<Audio stream���ڲ������е�Index��

    INT32 Fd;//!<Audio stream��Ӧ��linux DMX driver���ļ��������� 

    struct Ali_DmxAudioStreamParam StreamParam;//!<�û����õ�Audio stream������

    UINT32 InByteCnt;//!<Audio Stream�ܹ��յ���byte����. 

    /* TS layer info.
	*/
    UINT32 TsInCnt;
    UINT32 TsScrmbCnt;
    UINT32 TsSyncByteErrCnt;
	UINT32 TsDupCnt;
	UINT32 TsLostCnt;
    UINT32 TsErrCnt;
};




/*! @struct Ali_DmxPcrStreamInfo
@brief ALI DMX Pcr stream����Ϣ��
*/
struct Ali_DmxPcrStreamInfo
{
    enum ALI_DMX_STREAM_STATE State;//!<Pcr stream��״̬.

	UINT32 DmxId;//!<Pcr stream��Ӧ��DMX��ID��	

    UINT32 Idx;//!<Pcr stream���ڲ������е�Index��	

    INT32 Fd;//!<Pcr stream��Ӧ��linux DMX driver���ļ��������� 

    struct Ali_DmxPcrStreamParam StreamParam;//!<�û����õ�Pcr stream������ 

    UINT32 InByteCnt;//!<Pcr Stream�ܹ��յ���byte����.	
};



/*! @struct Ali_DmxTsStreamInfo
@brief ALI DMX TS stream����Ϣ��
*/
struct Ali_DmxTsStreamInfo
{
    enum ALI_DMX_STREAM_STATE State;//!<TS stream��״̬.

	UINT32 DmxId;//!<TS stream��Ӧ��DMX��ID��	

    UINT32 Idx; //!<TS stream���ڲ������е�Index��

    INT32 Fd;//!<TS stream��Ӧ��linux DMX driver���ļ�������.

    struct Ali_DmxTsStreamParam StreamParam;//!<�û����õ�TS stream������  

    UINT32 InByteCnt;//!<TS Stream�ܹ��յ���byte����.	
};




/*! @struct Ali_DmxTsInRamStreamInfo
@brief ALI DMX TS IN RAM stream����Ϣ��
*/
struct Ali_DmxTsInRamStreamInfo
{
    enum ALI_DMX_STREAM_STATE State;//!<TS IN RAM stream��״̬.

	UINT32 DmxId;//!<TS IN RAM stream��Ӧ��DMX��ID��

    UINT32 Idx; //!<TS IN RAM stream���ڲ������е�Index��	

    INT32 Fd;//!<TS IN RAM stream��Ӧ��linux DMX driver���ļ�������.

    struct Ali_DmxTsInRamStreamParam StreamParam;//!<�û����õ�TS IN RAM stream����.

    UINT32 InByteCnt;//!<TS IN RAM Stream�ܹ��յ����û�д���byte����.
};




/*! @struct Ali_DmxSrcCtrlStreamInfo
@brief ALI DMX source control stream����Ϣ��
*/
struct Ali_DmxSrcCtrlStreamInfo
{
    enum ALI_DMX_STREAM_STATE State;//!<source control stream��״̬.

	UINT32 DmxId;//!<source control stream��Ӧ��DMX��ID��

    UINT32 Idx;//!<source control stream���ڲ������е�Index��	

    INT32 Fd;//!<source control stream��Ӧ��linux DMX driver���ļ�������.

    struct Ali_DmxSrcCtrlStreamParam StreamParam;//!<�û����õ�source control stream����. 
};




/*!
@brief �û�ͨ���˽ӿ���DMX����һ��SECTION���͵�����ͨ����

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] DmxId Ŀ��DMX��ID��DMX 0��IDΪ0��DMX 1��IDΪ1���Դ����ơ�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,�򿪵�Section stream��ID��
*/
INT32 Ali_DmxSecStreamOpen(UINT32 DmxId);


/*!
@brief �û�ͨ���˽ӿ�����һ������õ���section Stream��

       �û���Ҫ�Բ�����ʽ���ݸ�DMXһ�����˲�����
       
       Section stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxSecStreamOpen()�õ���
@param[in] StreamParam��һ��ָ��ṹ��struct Ali_DmxSecStreamParam��ָ�룬
           �û�ͨ�����ָ���֪DMX��Ҫ��ȡ��SECTON���ݵ����á�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxSecStreamCfg(INT32 StreamId, struct Ali_DmxSecStreamParam *StreamParam);


/*!
@brief �û�ͨ���˽ӿ�ʹ��һ��SECTION���͵�����ͨ����

       ʹ�ܳɹ���DEMUX������û������ù��˳�SECTION���ݣ�
       
       �û�����ͨ��Ali_DmxSecStreamRead()�ӿڶ�ȡͨ�����˵�SECTION��
       
       Section stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxSecStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,ʹ�ܳɹ���
*/
INT32 Ali_DmxSecStreamStart(INT32 StreamId);


/*!
@brief �û�ͨ���˽ӿ�ֹͣһ��section stream��

       Stream�رպ�DMXֹͣ��ȡ���stream�����ݣ��������stream��ص���Դ����
       ���ͷţ��û����Ե���Ali_DmxSecStreamCfg()�ӿ������������stream��
       
       ��ע������ӿ���streamΪRUN��״̬�²���Ч��
       
       Section stream�Ĳ���������ο�ALI_DMX_MES.doc��
@param[in] StreamId����ֵ�ɵ���Ali_DmxSecStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxSecStreamStop(INT32 StreamId);


/*!
@brief �û�ͨ���˽ӿڹر�һ��section stream��

       �û������ڳ�IDLE֮����κ�״̬�µ�������ӿڡ�
       
       Stream��Close�����������stream��ص���Դ���ᱻ�ͷţ�
       ���stream����������ȴ���һ�����±�openʹ�á�
       
       Section stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxSecStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxSecStreamClose(INT32 StreamId);


/*!
@brief �û�ͨ���˽ӿ���DEMUX��ȡһ���Ѿ��򿪵�Section stream�����ݡ�

       �����Ҫ��ȡ��stream�������ݣ��û�����ͨ��Ali_DmxStreamPoll()�ӿ�ʵ��
       ���ݵȴ���
       
	   �û���Ҫ�ṩһ�����Ա���Section���ݵĻ������ĵ�ַ��DMX�������ַ�ĳ�����
	   Ҫ�㹻����һ��������Section�����Ƽ���ֵΪ4K�������п��ܳ�����Ϊ����������
	   �������޷��õ����ݵĴ���
	   
	   �û����Ե���Ali_DmxStreamPoll()�ӿ�ʵ�ֶԶ��stream�ĵȴ�����μ�
	   Ali_DmxStreamPoll()�ӿ�˵����
	   
	   ע��read����ֻ����section streamΪRUN��״̬�²���Ч��
	   
       Section stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxSecStreamOpen()�õ���
@param[out] Buf���û��ṩ�����Դ洢Section�Ļ����ַ��
@param[in] BufLen���û��ṩ���峤�ȡ�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,��ȡ�������ݵĳ��ȣ���byteΪ��λ��
*/
INT32 Ali_DmxSecStreamRead(INT32 StreamId, UINT8 *Buf, UINT32 BufLen);


/*!
@brief �û�ͨ���˽ӿ���DMX��ѯһ��section stream��״̬��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] StreamId ��Ҫ��ѯ��section stream��ID��
@param[out] SecStreamInfo���û��ṩ�����Դ����ѯ�����ָ�룬ָ�����ݽṹ
            struct Ali_DmxSecStreamInfo��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,��ѯ�ɹ���
*/
INT32 Ali_DmxSecStreamInfoGet(INT32 StreamId, struct Ali_DmxSecStreamInfo *SecStreamInfo);




/*!
@brief �û�ͨ���˽ӿ���DMX�������һ��section slot��

       ����ɹ���DMX�᷵��һ�� slot ID���˺����ͨ�����slot ID���������slot��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��

       Section slot�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] DmxId Ŀ��DMX��ID��DMX 0��IDΪ0��DMX 1��IDΪ1���Դ����ơ�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,�򿪵�section slot��ID��
*/
INT32 Ali_DmxSecSlotOpen(UINT32 DmxId);



/*!
@brief �û�ͨ���˽ӿ�����һ������õ���section slot��

	   �û�ͬʱ��Ҫ�Բ�����ʽ���ݸ�DMXһ�����˲�����
	   
	   DMX�յ�section���ݺ󣬻��������������������ݽ��й��ˣ�ֻ�з��Ϲ���
	   ������section���ݲŻᱻ���͸��û���
	   
	   �û�ͬʱ��Ҫ�Բ�����ʽ���ݸ�DMXһ��callback������DMX�յ�ͨ������������
	   section���ݺ��������callback����֪ͨ�û��յ������ݵ�ַ������.
	   
	   �û�Ҳ��Ҫ����һ��Timeout�����������Timeoutʱ�����δ�յ����ݣ�DMX��
	   ����callback����֪ͨ�û���DMX���callback������ErrCode
	   ����ΪALI_ERR_DMX_SEC_STREAM_TIMEOUT��֪ͨ�û���ʱ�¼��ķ�����
	   
	   ��ʱʱ�䣬0Ϊ���̳�ʱ��0xFFFFFFFFΪ��Զ����ʱ������Ϊ��ʱ�ȴ�ʱ�䣬
	   �Ժ���Ϊ��λ��
	   
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section slot�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] SlotId����ֵ�ɵ���Ali_DmxSecSlotOpen()�õ���
@param[in] SecSlotParamָ�����ݽṹstruct Ali_DmxSecSlotParam��ָ�룬�û�
           ͨ�����ָ������Section Slot�Ĳ�����
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxSecSlotCfg(INT32 SlotId, struct Ali_DmxSecSlotParam *SecSlotParam);


/*!
@brief �û�ͨ���˽ӿڿ���һ��section slot��

       slot������������յ����Ϲ���������section���ͻ�ͨ������
       SecRcvCallback()���û��������ݡ�
       
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section slot�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] SlotId����ֵ�ɵ���Ali_DmxSecSlotOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxSecSlotStart(INT32 SlotId);


/*!
@brief �û�ͨ���˽ӿ�ֹͣһ��section slot��

       Slot�رպ������slot��ص���Դ���ᱻ�ͷţ��û����Ե���
       Ali_DmxSecSlotCfg()�ӿ������������slot��
       
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section slot�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] SlotId����ֵ�ɵ���Ali_DmxSecSlotOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,Slotֹͣ�ɹ���
*/
INT32 Ali_DmxSecSlotStop(INT32 SlotId);


/*!
@brief �û�ͨ���˽ӿ��ͷ�һ��section slot��

       Slot�ͷź����������slot��ص���Դ���ᱻ�ͷţ����slot����������ȴ�
       ��һ�����·���ʹ�á�
       
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section slot�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] SlotId����ֵ�ɵ���Ali_DmxSecSlotOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxSecSlotClose(INT32 SlotId);



/*!
@brief �û�ͨ���˽ӿ���DMX��ѯһ��section slot��״̬��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section slot�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] SlotId ��Ҫ��ѯ��section slot��ID��
@param[out] SecSlotInfo���û��ṩ�����Դ����ѯ�����ָ�룬ָ�����ݽṹ
            struct Ali_DmxSecSlotInfo��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,��ѯ�ɹ���
*/
INT32 Ali_DmxSecSlotInfoGet(INT32 SlotId, struct Ali_DmxSecSlotInfo * SecSlotInfo);


/*!
@brief �û�ͨ���˽ӿ���DMX�������һ��section channel��

       ����ɹ���DMX�᷵��һ��channel ID���˺����ͨ�����channel ID���������
       channel��
       
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section channel�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] DmxId Ŀ��DMX��ID��DMX 0��IDΪ0��DMX 1��IDΪ1���Դ����ơ�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,�򿪵�section channel��ID��
*/
INT32 Ali_DmxSecChOpen(UINT32 DmxId);

/*!
@brief �û�ͨ���˽ӿ�����һ������õ���section channel��

	   �û���Ҫ�Բ�����ʽ���ݸ�channelһ������,���������һ��ָ��
	   ���ݽṹstruct Ali_DmxSecChParam��ָ�룬DMX������û�������������������
	   ���channel��
	   
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section Channel�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] ChId����ֵ�ɵ���Ali_DmxSecChOpen()�õ���
@param[in] ChParamָ�����ݽṹstruct Ali_DmxSecChParam��ָ�룬�û�
           ͨ�����ָ������Section Channel�Ĳ�����
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxSecChCfg(INT32 ChId, struct Ali_DmxSecChParam *ChParam);

/*!
@brief �û�ͨ���˽ӿڿ���һ��section Channel��

       Channel�������û����������channel�Ͽ���section filter��ͨ��
       section filter��ȡ���ݡ�
       
	   ���ر�ע�⣬ֻ����channel����RUN��״̬�£����������channel�ϵ�filter��
	   ���յ����ݡ�

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section Channel�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] ChId����ֵ�ɵ���Ali_DmxSecChOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxSecChStart(INT32 ChId);

/*!
@brief �û�ͨ���˽ӿ�ֹͣһ��section Channel��

       ���ô˽ӿڳɹ������й��������Channel�ϵ�Filter�����ܽ��ܵ����ݣ�һֱ��
       ���channel������start��ſ��Լ������յ����ݡ�
       
       ���ô˽ӿڳɹ����û����Ե���Ali_DmxSecChCfg()�ӿ������������Channel��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section Channel�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] ChId����ֵ�ɵ���Ali_DmxSecChOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxSecChStop(INT32 ChId);

/*!
@brief �û�ͨ���˽ӿ��ͷ�һ��section Channel��

       Channel�ͷź����������Channel��ص���Դ���ᱻ�ͷţ����Channel�����
       �����ȴ���һ�����·���ʹ�á�
       
	   ��ע�⣬Channel�ͷ�ʱ��ֻҪ�����ڹ��������Channel��û��close����Filter��
	   �ͻ᷵��ʧ��ֵALI_ERR_DMX_SEC_CH_OPER_DENIED���û�������close��������һ
	   ��Channel�ϵ�����Filter�󣬲���close�����Channel��
	   
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section Channel�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] ChId����ֵ�ɵ���Ali_DmxSecChOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxSecChClose(INT32 ChId);


/*!
@brief �û�ͨ���˽ӿ���DMX��ѯһ��section channel��״̬��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section channel�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] ChId����Ҫ��ѯ��section channel��ID��
@param[out] SecChInfo���û��ṩ�����Դ����ѯ�����ָ�룬ָ�����ݽṹ
            struct Ali_DmxSecChInfo��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,��ѯ�ɹ���
*/
INT32 Ali_DmxSecChInfoGet(INT32 ChId, struct Ali_DmxSecChInfo *SecChInfo);


/*!
@brief �û�ͨ���˽ӿ���һ���Ѵ򿪵�channel�������һ��section Filter��

       ����ɹ���DMX�᷵��һ��Filter ID���˺����ͨ�����Filter ID���������
       Filter��
       
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section Filter�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] ChId Ŀ��Channel��ID��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,�򿪵�section Filter��ID��
*/
INT32 Ali_DmxSecFltOpen(INT32 ChId);

/*!
@brief �û�ͨ���˽ӿ�����һ������õ���Filter slot��

	   �û�ͬʱ��Ҫ�Բ�����ʽ���ݸ�DMXһ�����˲�����
	   
	   �û�ͬʱ��Ҫ�Բ�����ʽ���ݸ�DMXһ��callback������DMX�յ�section����ͨ��
	   �����������������callback����֪ͨ�û��յ������ݵ�ַ�����ȡ�
	   
	   �û�Ҳ��Ҫ����һ��Timeout�����������Timeoutʱ�����δ�յ����ݣ�DMX�����
	   callback����֪ͨ�û���

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section Filter�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] FltId����ֵ�ɵ���Ali_DmxSecFltOpen()�õ���
@param[in] FltParamָ�����ݽṹstruct Ali_DmxSecFltParam��ָ�룬�û�
           ͨ�����ָ������Section Filter�Ĳ�����
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxSecFltCfg(INT32 FltId, struct Ali_DmxSecFltParam *FltParam);


/*!
@brief �û�ͨ���˽ӿڿ���һ��section Filter��

       Filter�����ɹ���������յ����Ϲ���������section��
       �ͻ�ͨ������DmxSecFltCfg()�ӿ�ע���SecRcvCallback()�������û��������ݡ�

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section Filter�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] FltId����ֵ�ɵ���Ali_DmxSecFltOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxSecFltStart(INT32 FltId);


/*!
@brief �û�ͨ���˽ӿ�ֹͣһ��section Filter��

       Filterֹͣ�������filter��ص���Դ���ᱻ�ͷţ��û����Ե���
       Ali_DmxSecFltCfg()�ӿ������������Filter��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section Filter�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] FltId����ֵ�ɵ���Ali_DmxSecFltOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxSecFltStop(INT32 FltId);


/*!
@brief �û�ͨ���˽ӿ��ͷ�һ��section Filter��

       Filter�ͷź����������Filter��ص���Դ���ᱻ�ͷţ����Filter�������
       ���ȴ���һ�����·���ʹ�á�ͬʱ���filterҲ��ӱ����ص�channel��ȡ�߶���
       �ٴ��������channel��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section Filter�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] FltId����ֵ�ɵ���Ali_DmxSecFltOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxSecFltClose(INT32 FltId);



/*!
@brief �û�ͨ���˽ӿ���DMX��ѯһ��section filiter��״̬��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Section filiter�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] FltId����Ҫ��ѯ��section filiter��ID��
@param[out] SecFltInfo���û��ṩ�����Դ����ѯ�����ָ�룬ָ�����ݽṹ
            struct Ali_DmxSecFltInfo��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,��ѯ�ɹ���
*/
INT32 Ali_DmxSecFltInfoGet(INT32 FltId, struct Ali_DmxSecFltInfo * SecFltInfo);




/*!
@brief �û�ͨ����API������һ��ץȡVideo PID��filter��

       ��filter�õ�������ֱ�Ӵ��䵽Video Decoder��Video filter���˳���TS������
       ���͵��û��ռ䡣
       
       ��ע��ĿǰALI DMX֧��һ·Video filter��
       
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Video Stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] DmxId Ŀ��DMX��ID��DMX 0��IDΪ0��DMX 1��IDΪ1���Դ����ơ�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,�򿪵�Video stream��ID��
*/
INT32 Ali_DmxVideoStreamOpen(UINT32 DmxId);

/*!
@brief �û�ͨ���˽ӿ�����һ������õ���Video Stream��

       ��ע��ĿǰALI DMX֧��һ·Video filter��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Video Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������Video Stream��ID. 
           ��ֵ�ɵ���Ali_DmxVideoStreamOpen()�õ���
@param[in] StreamParam��ָ�����ݽṹstruct Ali_DmxVideoStreamParam��ָ�룬�û�
           ͨ�����ָ������Video Stream�Ĳ�����
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxVideoStreamCfg(INT32 StreamId, struct Ali_DmxVideoStreamParam *StreamParam);

/*!
@brief �û�ͨ���˽ӿ�ʹ��һ��Video Stream������ͨ����

       ʹ�ܳɹ���DMX������û������ù��˳�Video Stream���ݣ�ֱ�Ӵ����
       Video Decoder��
       
       ��ע��ĿǰALI DMX֧��һ·Video filter��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       Video Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������Video Stream��ID. 
           ��ֵ�ɵ���Ali_DmxVideoStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxVideoStreamStart(INT32 StreamId);

/*!
@brief �û�ͨ���˽ӿ�ֹͣһ��Video Stream��

       Stream�رպ�DMXֹͣ��ȡ���Video Stream�����ݣ��������Stream��ص���
       Դ���ᱻ�ͷţ����Ե���Ali_DmxVideoStreamCfg()�ӿ������������stream��

       ��ע��ĿǰALI DMX֧��һ·Video filter��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       Video Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������Video Stream��ID. 
           ��ֵ�ɵ���Ali_DmxVideoStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxVideoStreamStop(INT32 StreamId);

/*!
@brief �û�ͨ���˽ӿڹر�һ��Video Stream��

       �����ڳ�IDLE֮����κ�״̬�µ�������ӿڡ�
       
       Video Stream��Close�����������stream��ص���Դ���ᱻ�ͷţ����
       stream����������ȴ���һ�����±�openʹ�á�

       ��ע��ĿǰALI DMX֧��һ·Video filter��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Video Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������Video Stream��ID. 
           ��ֵ�ɵ���Ali_DmxVideoStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxVideoStreamClose(INT32 StreamId);



/*!
@brief �û�ͨ���˽ӿ���DMX��ѯһ��Video stream��״̬��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Video stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] StreamId ��Ҫ��ѯ��Video stream��ID��
@param[out] VideoStreamInfo���û��ṩ�����Դ����ѯ�����ָ�룬ָ�����ݽṹ
            struct Ali_DmxVideoStreamInfo��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,��ѯ�ɹ���
*/
INT32 Ali_DmxVideoStreamInfoGet(INT32 StreamId, struct Ali_DmxVideoStreamInfo * VideoStreamInfo);



/*!
@brief �û�ͨ���˽ӿ�������һ��ץȡAudio PID��filter��

       ��filter�õ�������ֱ�Ӵ��䵽Audio Decoder��Audio filter���˳���TS������
       ���͵��û��ռ䡣
       
       ��ע��ĿǰALI DMX֧��һ·Audio filter��
       
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Audio stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] DmxId Ŀ��DMX��ID��DMX 0��IDΪ0��DMX 1��IDΪ1���Դ����ơ�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,�򿪵�Audio stream��ID��
*/
INT32 Ali_DmxAudioStreamOpen(UINT32 DmxId);

/*!
@brief �û�ͨ���˽ӿ�����һ������õ���Audio Stream��

       ��ע��ĿǰALI DMX֧��һ·Audio filter��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Audio Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������Audio Stream��ID. 
           ��ֵ�ɵ���Ali_DmxAudioStreamOpen()�õ���
@param[in] StreamParam��ָ�����ݽṹstruct Ali_DmxAudioStreamParam��ָ�룬�û�
           ͨ�����ָ������Audio Stream�Ĳ�����
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxAudioStreamCfg(INT32 StreamId, struct Ali_DmxAudioStreamParam *StreamParam);

/*!
@brief �û�ͨ���˽ӿ�ʹ��һ��Audio Stream������ͨ����

       ʹ�ܳɹ���DMX������û������ù��˳�Audio Stream���ݣ�ֱ�Ӵ����
       Audio Decoder��
       
       ��ע��ĿǰALI DMX֧��һ·Audio filter��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Audio Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������Audio Stream��ID. 
           ��ֵ�ɵ���Ali_DmxAudioStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxAudioStreamStart(INT32 StreamId);

/*!
@brief �û�ͨ���˽ӿ�ֹͣһ��Audio Stream��

       Stream�رպ�DMXֹͣ��ȡ���Audio Stream�����ݣ��������Stream��ص���
       Դ���ᱻ�ͷţ����Ե���Ali_DmxAudioStreamCfg()�ӿ������������stream��

       ��ע��ĿǰALI DMX֧��һ·Audio filter��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Audio Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������Audio Stream��ID. 
           ��ֵ�ɵ���Ali_DmxAudioStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxAudioStreamStop(INT32 StreamId);


/*!
@brief �û�ͨ���˽ӿڹر�һ��Audio Stream��

       �����ڳ�IDLE֮����κ�״̬�µ�������ӿڡ�
       
       Audio Stream��Close�����������stream��ص���Դ���ᱻ�ͷţ����
       stream����������ȴ���һ�����±�openʹ�á�

       ��ע��ĿǰALI DMX֧��һ·Audio filter��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Video Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������Audio Stream��ID. 
           ��ֵ�ɵ���Ali_DmxVideoStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxAudioStreamClose(INT32 StreamId);


/*!
@brief �û�ͨ���˽ӿ���DMX��ѯһ��Audio stream��״̬��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Audio stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ��ѯ��Audio stream��ID��
@param[out] AudioStreamInfo���û��ṩ�����Դ����ѯ�����ָ�룬ָ�����ݽṹ
            struct Ali_DmxAudioStreamInfo��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,��ѯ�ɹ���
*/
INT32 Ali_DmxAudioStreamInfoGet(INT32 StreamId, struct Ali_DmxAudioStreamInfo * AudioStreamInfo);


/*!
@brief �û�ͨ���˽ӿ�������һ��ץȡPCR PID��filter��

       ��filter�õ���������Ϣ�ᱻֱ�Ӵ��䵽ALI AVSYNCģ�飬������ΪALI�ڲ�
       ��AV sync�Ĳο���Ϣ��PCR filter���˳���TS�����ᱻ�͵��û��ռ䡣
       
       �û��ڲ���Audio/Videoʱ����Ҫ���ô˽ӿ�ָ��AV sync�����PCR PID��Ϣ��
       
       ��ע��ĿǰALI DMX֧��һ·PCR filter��
       
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       PCR stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] DmxId Ŀ��DMX��ID��DMX 0��IDΪ0��DMX 1��IDΪ1���Դ����ơ�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,�򿪵�PCR stream��ID��
*/
INT32 Ali_DmxPcrStreamOpen(UINT32 DmxId);

/*!
@brief �û�ͨ���˽ӿ�����һ������õ���PCR Stream��

       ��ע��ĿǰALI DMX֧��һ·PCR filter��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       PCR Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������PCR Stream��ID. 
           ��ֵ�ɵ���Ali_DmxPcrStreamOpen()�õ���
@param[in] StreamParam��ָ�����ݽṹstruct Ali_DmxPcrStreamParam��ָ�룬�û�
           ͨ�����ָ������PCR Stream�Ĳ�����
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxPcrStreamCfg(INT32 StreamId, struct Ali_DmxPcrStreamParam *StreamParam);


/*!
@brief �û�ͨ���˽ӿ�ʹ��һ��PCR Stream������ͨ����

       ʹ�ܳɹ���DMX������û������ù��˳�PCR Stream���ݣ�ֱ�Ӵ����
       ALI AVSYNCģ�顣
       
       ��ע��ĿǰALI DMX֧��һ·PCR  filter��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       PCR  Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������PCR  Stream��ID. 
           ��ֵ�ɵ���Ali_DmxPcrStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxPcrStreamStart(INT32 StreamId);

/*!
@brief �û�ͨ���˽ӿ�ֹͣһ��PCR Stream��

       Stream�رպ�DMXֹͣ��ȡ���PCR Stream�����ݣ��������Stream��ص���
       Դ���ᱻ�ͷţ����Ե���Ali_DmxPcrStreamCfg()�ӿ������������stream��

       ��ע��ĿǰALI DMX֧��һ·Audio filter��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Ali_DmxPcrStreamCfg Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������Ali_DmxPcrStreamCfg Stream��ID. 
           ��ֵ�ɵ���Ali_DmxPcrStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxPcrStreamStop(INT32 StreamId);

/*!
@brief �û�ͨ���˽ӿڹر�һ��PCR Stream��

       �����ڳ�IDLE֮����κ�״̬�µ�������ӿڡ�
       
       PCR Stream��Close�����������stream��ص���Դ���ᱻ�ͷţ����
       stream����������ȴ���һ�����±�openʹ�á�

       ��ע��ĿǰALI DMX֧��һ·PCR filter��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       PCR Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������PCR Stream��ID. 
           ��ֵ�ɵ���Ali_DmxPcrStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxPcrStreamClose(INT32 StreamId);


/*!
@brief �û�ͨ���˽ӿ���DMX��ѯһ��Pcr stream��״̬��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       Pcr stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] StreamId ��Ҫ��ѯ��Pcr stream��ID��
@param[out] PcrStreamInfo���û��ṩ�����Դ����ѯ�����ָ�룬ָ�����ݽṹ
            struct Ali_DmxPcrStreamInfo��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,��ѯ�ɹ���
*/
INT32 Ali_DmxPcrStreamInfoGet(INT32 StreamId, struct Ali_DmxPcrStreamInfo * PcrStreamInfo);


/*!
@brief �û�ͨ���˽ӿ�������һ��ץȡTS����filter��

       ץȡ����TS��������DMX�ڲ���buffer�л��棬�û�����ͨ��
       Ali_DmxTsStreamRead()�ӿڶ�ȡfilter���յ���TS�����ݡ�

       ����ͬһ��TS stream,�û��������ö��PID��DMX������з�������������TS��
       ������ͬһ�ڲ������еȴ��û���ȡ��������Զ�PVRӦ�ñȽϷ��㡣
       
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       PCR stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] DmxId Ŀ��DMX��ID��DMX 0��IDΪ0��DMX 1��IDΪ1���Դ����ơ�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,�򿪵�TS stream��ID��
*/
INT32 Ali_DmxTsStreamOpen(UINT32 DmxId);


/*!
@brief �û�ͨ���˽ӿ�����һ������õ���TS Stream��

       ����ͬһ��TS stream,�û��������ö��PID��DMX������з�������������TS��
       ������ͬһ�ڲ������еȴ��û���ȡ��������Զ�PVRӦ�ñȽϷ��㡣

       PID�б�PID������Ҫͨ������StreamParam���ݸ�DMX��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       PCR Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������TS Stream��ID. 
           ��ֵ�ɵ���Ali_DmxPcrStreamOpen()�õ���
@param[in] StreamParam��ָ�����ݽṹstruct Ali_DmxTsStreamParam��ָ�룬�û�
           ͨ�����ָ������TS Stream�Ĳ�����
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxTsStreamCfg(INT32 StreamId, struct Ali_DmxTsStreamParam *StreamParam);


/*!
@brief �û�ͨ���˽ӿ�ʹ��һ��TS���͵�����ͨ����

       ʹ�ܳɹ���DEMUX������û������ù��˳�TS���ݣ�
       
       �û�����ͨ��Ali_DmxTsStreamRead()�ӿڶ�ȡͨ�����˵�TS����
       
       TS stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxSecStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,ʹ�ܳɹ���
*/
INT32 Ali_DmxTsStreamStart(INT32 StreamId);



/*!
@brief �û�ͨ���˽ӿ�ֹͣһ��TS stream��

       Stream�رպ�DMXֹͣ��ȡ���stream�����ݣ��������stream��ص���Դ����
       ���ͷţ��û����Ե���Ali_DmxTsStreamCfg()�ӿ������������stream��
       
       ��ע������ӿ���streamΪRUN��״̬�²���Ч��
       
       Section stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxTsStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxTsStreamStop(INT32 StreamId);



/*!
@brief �û�ͨ���˽ӿڹر�һ��TS stream��

       �û������ڳ�IDLE֮����κ�״̬�µ�������ӿڡ�
       
       TS Stream��Close�����������stream��ص���Դ���ᱻ�ͷţ����TS stream
       ����������ȴ���һ�����±�openʹ�á�
       
       Section stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxTsStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxTsStreamClose(INT32 StreamId);


/*!
@brief �û�ͨ���˽ӿ���DEMUX��ȡһ���Ѿ��򿪵�TS stream�����ݡ�

       �����Ҫ��ȡ��stream�������ݣ��û�����ͨ��Ali_DmxStreamPoll()�ӿ�ʵ��
       ���ݵȴ���
       
	   �û���Ҫ�ṩһ�����Ա���TS���ݵĻ������ĵ�ַ��DMX�������ַ�ĳ�����
	   Ҫ�㹻����һ��������TS��(188 bytes)���Ƽ���ֵΪ47K�������������Ҫ����
       ��ϵͳ���ö�Ӱ��ϵͳ���ܡ�
	   
	   �û����Ե���Ali_DmxStreamPoll()�ӿ�ʵ�ֶԶ��stream�ĵȴ�����μ�
	   Ali_DmxStreamPoll()�ӿ�˵����
	   
	   ע��read����ֻ����TS streamΪRUN��״̬�²���Ч��
	   
       TS stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxTsStreamOpen()�õ���
@param[out] Buf���û��ṩ�����Դ洢TS�Ļ����ַ��
@param[in] BufLen���û��ṩ���峤�ȡ�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,��ȡ�������ݵĳ��ȣ���byteΪ��λ��
*/
INT32 Ali_DmxTsStreamRead(INT32 StreamId, UINT8 *Buf, UINT32 BufLen);




/*!
@brief �û�ͨ���˽ӿ���DMX��ѯһ��TS stream��״̬��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       TS stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] StreamId ��Ҫ��ѯ��TS stream��ID��
@param[out] TsStreamInfo���û��ṩ�����Դ����ѯ�����ָ�룬ָ�����ݽṹ
            struct Ali_DmxTsStreamInfo��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,��ѯ�ɹ���
*/
INT32 Ali_DmxTsStreamInfoGet(INT32 StreamId, struct Ali_DmxTsStreamInfo * TsStreamInfo);




/*!
@brief �û�ͨ���˽ӿ�������һ��ͨ���ڴ�������DMX��������(TS��ʽ)��

       �û�����ͨ������򿪵�Stream ID����DMX����TS��ʽ������������DMX��Ҫ��
       ����TS���еõ���������������
       
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       TS stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] DmxId Ŀ��DMX��ID��DMX 0��IDΪ0��DMX 1��IDΪ1���Դ����ơ�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,�򿪵�TS IN RAM stream��ID��
*/
INT32 Ali_DmxTsInRamStreamOpen(UINT32 DmxId);


/*!
@brief �û�ͨ���˽ӿ�����һ������õ���TS IN RAM Stream��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       TS IN RAM Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������TS IN RAM Stream��ID. 
           ��ֵ�ɵ���Ali_DmxTsInRamStreamOpen()�õ���
@param[in] StreamParam��ָ�����ݽṹstruct Ali_DmxTsStreamParam��ָ�룬�û�
           ͨ�����ָ������TS Stream�Ĳ�����
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxTsInRamStreamCfg(INT32 StreamId, struct Ali_DmxTsInRamStreamParam *StreamParam);


/*!
@brief �û�ͨ���˽ӿ�ʹ��һ��TS IN RAM���͵�����ͨ����

       ʹ�ܳɹ����û�������򿪵�TS IN RAM streamд��TS��ʽ�����ݡ� ��Ӧ����
       DMX�ᴦ�������TS��Ϊ�û��ṩ����

       �û�����ͨ��Ali_DmxTsInRamStreamWrite()�ӿ�������ͨ��д�����ݡ�
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       TS IN RAM stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxTsInRamStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,ʹ�ܳɹ���
*/
INT32 Ali_DmxTsInRamStreamStart(INT32 StreamId);



/*!
@brief �û�ͨ���˽ӿ�ֹͣһ��TS IN RAM stream��

       Stream�رպ�DMXֹͣ��ȡ���stream�����ݣ��������stream��ص���Դ����
       ���ͷţ��û����Ե���Ali_DmxTsInRamStreamCfg()�ӿ������������stream��
       
       ��ע������ӿ���streamΪRUN��״̬�²���Ч��
       
       TS IN RAM stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxTsInRamStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxTsInRamStreamStop(INT32 StreamId);



/*!
@brief �û�ͨ���˽ӿڹر�һ��TS IN RAM stream��

       �û������ڳ�IDLE֮����κ�״̬�µ�������ӿڡ�
       
       Stream��Close�����������stream��ص���Դ���ᱻ�ͷţ����stream�����
       �����ȴ���һ�����±�openʹ�á�
       
       TS IN RAM stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxTsInRamStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxTsInRamStreamClose(INT32 StreamId);


/*!
@brief �û�ͨ���˽ӿ���һ����DMXд��TS��ʽ�����ݡ�

       �����DMX�ڲ�buffer�������û�����ͨ��Ali_DmxStreamPoll()�ӿ�ʵ��
       ���ݵȴ������ߵȴ�һ��ʱ���buffer����������һ��ʱ����ټ���д�롣

       �û����Ե���Ali_DmxStreamPoll()�ӿ�ʵ�ֶԶ��stream�ĵȴ�����μ�
      	   Ali_DmxStreamPoll()�ӿ�˵����
       
	   �û���Ҫ�ṩһ������TS���ݵĵ�ַ��DMX�������ַ�ĳ�����
	   Ҫ��һ��TS������(188 bytes)�����������Ƽ���ֵΪ47K�������������Ҫ����
       ��ϵͳ���ö�Ӱ��ϵͳ���ܡ�
	   
	   ע��read����ֻ����TS streamΪRUN��״̬�²���Ч��
	   
       TS stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxTsInRamStreamOpen()�õ���
@param[in] Buf���û��ṩ�����Դ洢TS�Ļ����ַ��
@param[in] BufLen���û��ṩ���峤�ȡ�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,��ȡ�������ݵĳ��ȣ���byteΪ��λ��
*/
INT32 Ali_DmxTsInRamStreamWrite(INT32 StreamId, UINT8 *Buf, UINT32 BufLen);



/*!
@brief �û�ͨ���˽ӿ���DMX��ѯһ��TS IN RAM stream��״̬��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       TS IN RAM stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] StreamId ��Ҫ��ѯ��TS IN RAM stream��ID��
@param[out] TsInRamStreamInfo���û��ṩ�����Դ����ѯ�����ָ�룬ָ�����ݽṹ
            struct Ali_DmxTsInRamStreamInfo��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,��ѯ�ɹ���
*/
INT32 Ali_DmxTsInRamStreamInfoGet(INT32 StreamId, struct Ali_DmxTsInRamStreamInfo * TsInRamStreamInfo);

/*!
@brief �û�ͨ���˽ӿ�������һ������ӲDMXǰ������Դ���õĿ��ƶ˿ڡ�

       ͬһDMXͬʱֻ�ܴ���һ������Դ����stream��

       ע���Source control stream������Ali_DmxStreamPoll()�б�poll��

       �û�����ͨ������ӿ�����Ӳ��DMX����Դ����ALI DMX��Ӳ��DMX����Դ��Ӳ��
       TSI�����û��ɲο�ALI TSI��Ӳ��SPECȷ����������ֵ��
       
@param[in] DmxId Ŀ��DMX��ID��DMX 0��IDΪ0��DMX 1��IDΪ1���Դ����ơ�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,�򿪵�dmx source control stream��ID��
*/
INT32 ALi_DmxSrcCtrlStreamOpen(UINT32 DmxId);


/*!
@brief �û�ͨ���˽ӿ����ر�һ������ӲDMXǰ������Դ���õĿ��ƶ˿ڡ�

       ע���Source control stream������Ali_DmxStreamPoll()�б�poll��

       �رճɹ����û�������ͨ������˿�������DMX����Դ�������Ҫ�ٴ����ã�
       ��Ҫ����ALi_DmxSrcCtrlStreamOpen()���´����ö˿ڡ�

       �û�����ͨ������ӿ�����Ӳ��DMX����Դ����ALI DMX��Ӳ��DMX����Դ��Ӳ��
       TSI�����û��ɲο�ALI TSI��Ӳ��SPECȷ����������ֵ��
       
@param[in] StreamId����ֵ�ɵ���ALi_DmxSrcCtrlStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ����0,�����ɹ���
*/
INT32 Ali_DmxSrcCtrlStreamClose(INT32 StreamId);


/*!
@brief �û�ͨ���˽ӿ�������ӲDMXǰ������Դ��

	   ��ALI DMX��Ӳ��DMX����Դ��Ӳ��TSI�����û��ɲο�ALI TSI��Ӳ��SPECȷ��
	   ��������ֵ��

       ע���Source control stream������Ali_DmxStreamPoll()�б�poll��


       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��

@param[in] StreamId����ֵ�ɵ���ALi_DmxSrcCtrlStreamOpen()�õ���
@param[in] Param���û��ṩ�����Դ���TSI���õ�ָ�룬ָ�����ݽṹ
           struct Ali_DmxSrcCtrlStreamInfo��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ����0,�����ɹ���
*/
INT32 Ali_DmxSrcCtrlStreamWrite(INT32 StreamId, struct Ali_DmxSrcCtrlStreamParam *Param);


/*!
@brief �û�ͨ���˽ӿ�����ȡӲDMXǰ������Դ��������Ϣ��

	   ��ALI DMX��Ӳ��DMX����Դ��Ӳ��TSI�����û��ɲο�ALI TSI��Ӳ��SPEC��

       ע���Source control stream������Ali_DmxStreamPoll()�б�poll��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��

@param[in] StreamId����ֵ�ɵ���ALi_DmxSrcCtrlStreamOpen()�õ���
@param[out] Param���û��ṩ�����Դ���TSI���õ�ָ�룬ָ�����ݽṹ
            struct Ali_DmxSrcCtrlStreamInfo��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ����0,�����ɹ���
*/
INT32 Ali_DmxSrcCtrlStreamRead(INT32 StreamId, struct Ali_DmxSrcCtrlStreamParam *Param);



/*!
@brief �û�ͨ���˽ӿ���DMX��ѯһ��DMX source control stream��״̬��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       source control stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] StreamId ��Ҫ��ѯ��source control stream��ID��
@param[out] SrcCtrlStreamInfo���û��ṩ�����Դ����ѯ�����ָ�룬ָ�����ݽṹ
            struct Ali_DmxSrcCtrlStreamInfo��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,��ѯ�ɹ���
*/
INT32 Ali_DmxSrcCtrlStreamInfoGet(INT32 StreamId, struct Ali_DmxSrcCtrlStreamInfo *SrcCtrlStreamInfo);



/*!
@brief �û�ͨ���˽ӿ�����ѯָ����Stream�Ƿ������ݿɹ����������пռ�ɹ�д�롣

	   ���û���ͬʱ��ѯ���Stream�Ƿ����ݿɹ����������пռ�ɹ�д��ʱ����ʹ�ñ�
       �ӿڵõ�������Ϣ��

       �û��ڲ���PollList��������Ҫ��ѯ��Stream��ID�б��ò���PollUnitCnt��֪
       DMX��Ҫ��ѯ��Stream�ĸ������ò���Timeout��֪DMX��ʱʱ�䣬�Ժ���Ϊ��λ��

       �ɹ�ʱ��Ali_DmxStreamPoll()����PollList�������ݿɶ�/д��Stream��
       ����ڳ�ʱǰû���κ��¼�������Ali_DmxStreamPoll()����ALI_ERR_DMX_TIMEOUT��
       ʧ��ʱ��Ali_DmxStreamPoll()���ش����롣

       ��Ali_DmxStreamPoll()����ʱ��ÿ��Ali_DmxStreamPollInfo����StreamStatus
       ��ʾ�Ŷ�Ӧ��stream�Ƿ�ɶ�/д��

@param[out] PollList��struct Ali_DmxStreamPollInfo�ṹ�����飬ÿһ����Ԫ����һ��
            ��Ҫ��ѯ��Stream��
@param[in] PollUnitCnt����Ҫ��ѯ��Stream�ĸ�����
@param[in] Timeout����ʱʱ�䣬�Ժ���Ϊ��λ��

@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ����0,�����ɹ���
*/
INT32 Ali_DmxStreamPoll(struct Ali_DmxStreamPollInfo * PollList, UINT32 PollUnitCnt, INT32 Timeout);


/*!
@brief �û�ͨ���˽ӿڲ�ѯDMXģ��ȫ�����ü�������Ϣ��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
@param[out] DmxSysInfo���û��ṩ�����Դ����ѯ�����ָ�룬ָ�����ݽṹ
            struct ALi_DmxSysInfo��DMX���������ַд���ѯ�����
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,��ѯ�ɹ���
*/
INT32 ALi_DmxSysInfoGet(struct ALi_DmxSysInfo *DmxSysInfo);


/*!
@brief �û�ͨ���˽ӿڲ�ѯÿ��DMXģ��������ü�������Ϣ��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
@param[in] DmxId���û�ָ������Ҫ��ѯ��DMX��       
@param[out] DmxDevInfo���û��ṩ�����Դ����ѯ�����ָ�룬ָ�����ݽṹ
            struct ALi_DmxDevInfo����ѯ�����д�������ַ��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,��ѯ�ɹ���
*/
INT32 ALi_DmxDevInfoGet(UINT32 DmxId, struct ALi_DmxDevInfo *DmxDevInfo);


/*!
@brief �û�ͨ���˽ӿ���һ��Descrambler����ͨ������ż�����֡�
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��

       ����DMX��֧�����ÿ����֡�

@param[in] KeyStreamId���û�ָ������Ҫ���ÿ����ֵ�Key Stream��Id�����ID����
           ����Ali_DmxDescramKeyOpen()�õ���       
@param[in] EvenKey��ָ����Ҫ���õ�ż�����ֵ�ָ��,���ȱ�����8��byte��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,���óɹ���
*/
INT32 Ali_DmxDescramKeySetEven(UINT32 KeyStreamId, UINT8 * EvenKey);


/*!
@brief �û�ͨ���˽ӿ���һ��Descrambler����ͨ������������֡�
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��

       ����DMX��֧�����ÿ����֡�

@param[in] KeyStreamId���û�ָ������Ҫ���ÿ����ֵ�Key Stream��Id�����ID����
           ����Ali_DmxDescramKeyOpen()�õ���       
@param[in] OddKey��ָ����Ҫ���õ�������ֵ�ָ��,���ȱ�����8��byte��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,���óɹ���
*/
INT32 Ali_DmxDescramKeySetOdd(UINT32 KeyStreamId, UINT8 * OddKey);


/*!
@brief �û�ͨ���˽ӿ���Descramblerģ������һ�����ſ�����ͨ��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM������DMX��֧�����ÿ����֡�

       ����DMX��֧�����ÿ����֡�

@param[in] DmxId���û�ָ������Ҫ���ŵ�TS�����ڵ�DMX��       
@param[in] Param���û�ָ������Ҫ���ŵ�TS����������Ϣ������TS����PID��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ����0���ɹ�����õ��Ľ��ſ���ͨ����ID���û�ͨ�����ID���ú��������֡�
*/
INT32 Ali_DmxDescramKeyOpen(UINT32 DmxId, struct Ali_DmxDescramKeyStreamParam * Param);


/*!
@brief �û�ͨ���˽ӿ���Descramblerģ���ͷ�һ�����ſ�����ͨ��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM������DMX��֧�����ÿ����֡�

       ����DMX��֧�����ÿ����֡�

@param[in] KeyStreamId,�û�ָ������Ҫ�ͷŵĽ���ͨ����ID��      
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ����0���ͷųɹ�
*/
INT32 Ali_DmxDescramKeyClose(INT32 KeyStreamId);




/*!
@brief �û�ͨ���˽ӿ���Descramblerģ������Ĭ�Ͽ����֡�

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM������DMX��֧�����ÿ����֡�
       
       ����DMX��֧�����ÿ����֡�

@param[in] DmxId,�û�ָ������Ҫ���ŵ�TS�����ڵ�DMX��      
@param[in] Pid���û�ָ������Ҫ���ŵ�TS����PID��
@param[in] DefaultKey��ָ����Ҫ���õ�Ĭ�Ͽ����ֵ�ָ�롣
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,��ѯ�ɹ���
*/
INT32 Ali_DmxDescramDefaultKeySet(UINT32 DmxId, UINT32 Pid, UINT8 *DefaultKey);

/*!
@brief �û�ͨ���˽ӿ���Descramblerģ���ѯ��Ӧ��DSC Channel�Ƿ��Ѿ�work.

       ����Ѿ�����Ӧ��DSC Channel���򷵻���Ӧ��StreamID;���򷵻�-1.
     
@param[in] KeyStreamInfo�е�Pid���û�ָ������Ҫ���ŵ�TS����PID��
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ����ֵ:�Ѿ�ʹ�õ�StreamIDx
*/
INT32 Ali_DmxDescramKeyInfoGet(struct Ali_DmxDescramKeyStreamParam  KeyStreamInfo);


/*!
@brief �û�ͨ���˽ӿ�������һ�����Զ�ȡ����TP������

       TS��������DMX�ڲ���buffer�л��棬�û�����ͨ��
       Ali_DmxTpStreamRead()�ӿڶ�ȡDMX���յ���TP�����ݡ�
       
       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       TP stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] DmxId Ŀ��DMX��ID��DMX 0��IDΪ0��DMX 1��IDΪ1���Դ����ơ�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,�򿪵�TS stream��ID��
*/
INT32 Ali_DmxTpStreamOpen(UINT32 DmxId);


/*!
@brief �û�ͨ���˽ӿ�����һ������õ���TP Stream��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       TP Stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����Ҫ������TS Stream��ID. 
           ��ֵ�ɵ���Ali_DmxTpStreamOpen()�õ���
@param[in] StreamParam��ָ�����ݽṹstruct Ali_DmxTsStreamParam��ָ�룬�û�
           ͨ�����ָ������TS Stream�Ĳ�����
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxTpStreamCfg(INT32 StreamId, struct Ali_DmxTpStreamParam *StreamParam);


/*!
@brief �û�ͨ���˽ӿ�ʹ��һ��TP���͵�����ͨ����
       
       �û�����ͨ��Ali_DmxTpStreamRead()�ӿڶ�ȡTP����
       
       TP stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxTpStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,ʹ�ܳɹ���
*/
INT32 Ali_DmxTpStreamStart(INT32 StreamId);



/*!
@brief �û�ͨ���˽ӿ�ֹͣһ��TP stream��

       Stream�رպ�DMXֹͣ��ȡ���stream�����ݣ��������stream��ص���Դ����
       ���ͷţ��û����Ե���Ali_DmxTpStreamCfg()�ӿ������������stream��
       
       ��ע������ӿ���streamΪRUN��״̬�²���Ч��
       
       TP stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxTsStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxTpStreamStop(INT32 StreamId);



/*!
@brief �û�ͨ���˽ӿڹر�һ��TS stream��

       �û������ڳ�IDLE֮����κ�״̬�µ�������ӿڡ�
       
       TS Stream��Close�����������stream��ص���Դ���ᱻ�ͷţ����TP stream
       ����������ȴ���һ�����±�openʹ�á�
       
       TP stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxTsStreamOpen()�õ���
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,�����ɹ���
*/
INT32 Ali_DmxTpStreamClose(INT32 StreamId);


/*!
@brief �û�ͨ���˽ӿ���DEMUX��ȡһ���Ѿ��򿪵�TP stream�����ݡ�

       �����Ҫ��ȡ��stream�������ݣ��û�����ͨ��Ali_DmxStreamPoll()�ӿ�ʵ��
       ���ݵȴ���
       
	   �û���Ҫ�ṩһ�����Ա���TS���ݵĻ������ĵ�ַ��DMX�������ַ�ĳ�����
	   Ҫ�㹻����һ��������TS��(188 bytes)���Ƽ���ֵΪ470K�������������Ҫ����
       ��ϵͳ���ö�Ӱ��ϵͳ���ܡ�
	   
	   �û����Ե���Ali_DmxStreamPoll()�ӿ�ʵ�ֶԶ��stream�ĵȴ�����μ�
	   Ali_DmxStreamPoll()�ӿ�˵����
	   
	   ע��read����ֻ����TS streamΪRUN��״̬�²���Ч��
	   
       TP stream�Ĳ���������ο�ALI_DMX_MES.doc��
       
@param[in] StreamId����ֵ�ɵ���Ali_DmxTpStreamOpen()�õ���
@param[out] Buf���û��ṩ�����Դ洢TS�Ļ����ַ��
@param[in] BufLen���û��ṩ���峤�ȡ�
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ���ڵ���0,��ȡ�������ݵĳ��ȣ���byteΪ��λ��
*/
INT32 Ali_DmxTpStreamRead(INT32 StreamId, UINT8 *Buf, UINT32 BufLen);



/*!
@brief �û�ͨ���˽ӿ���DMX��ѯһ��TP stream��״̬��

       ALI DMX֧��2��Ӳ��DMX(DmxId 0 ~ 1)��2����DMX(DmxId 3 ~ 4),
       ֻ����DMX֧��TS_IN_RAM_STREAM��
       
       TP stream�Ĳ���������ο��ĵ�ALI_DMX_MES.doc��
       
@param[in] StreamId ��Ҫ��ѯ��TS stream��ID��
@param[out] TsStreamInfo���û��ṩ�����Դ����ѯ�����ָ�룬ָ�����ݽṹ
            struct Ali_DmxTpStreamInfo��
@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval 0,��ѯ�ɹ���
*/
//INT32 Ali_DmxTpStreamInfoGet(INT32 StreamId, struct Ali_DmxTpStreamInfo * TpStreamInfo);


INT32 Ali_DmxSubtStreamOpen(UINT32 DmxId);
INT32 Ali_DmxSubtStreamCfg(INT32 StreamId, struct Ali_DmxSubtStreamParam * StreamParam);
INT32 Ali_DmxSubtStreamStart(INT32 StreamId);
INT32 Ali_DmxSubtStreamStop(INT32 StreamId);
INT32 Ali_DmxSubtStreamClose(INT32 StreamId);



/*!
@brief ALI DMX LIB�ͷŽӿ�

       ����û�������Ҫʹ��ALI DMX LIB�����ñ��ӿڻ��ͷ�����ALI DMX LIBռ��
       ����Դ��

@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ����0,�����ɹ���
*/
INT32 Ali_DmxLibExit(void);


/*!
@brief ALI DMX LIB��ʼ���ӿڡ�

       Ҫʹ��ALI DMX LIB��������ʹ���κ�����LI DMX LIB�ӿ�֮ǰ���ȵ��ñ��ӿڡ�

       ���ӿڲ����ظ����á�

@return ALI_DMX_ERR_CODE
@retval С��0,�����롣�����붨����ο�enum ALI_DMX_ERR_CODE��
@retval ����0,�����ɹ���
*/
INT32 Ali_DmxLibInit(void);

/*!
@}
*/

#ifdef __cplusplus
}
#endif

#endif
