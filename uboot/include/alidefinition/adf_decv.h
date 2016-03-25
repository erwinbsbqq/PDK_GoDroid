#ifndef __ADF_DECV__
#define __ADF_DECV__


/*! @addtogroup �豸����
 *  @{
*/

/*! @addtogroup ��Ƶ����
 *  @{
*/

#include "adf_basic.h"
#include "adf_media.h"

#define VDEC_SYNC_PTS           0x00 //!< ������Ƶͬ�����ܡ�
#define VDEC_SYNC_FREERUN       0x01//!<�ر�����Ƶͬ�����ܡ�

#define VDEC_SYNC_FIRSTFREERUN  0x01 //!< ���ɲ�����Ƶ��һ֡��
#define VDEC_SYNC_I             0x02 //!< δʵ�֡�
#define VDEC_SYNC_P             0x04 //!< δʵ�֡�
#define VDEC_SYNC_B             0x08 //!< δʵ�֡�
#define VDEC_SYNC_HIGH_LEVEL    0x10 //!< δʵ�֡�


#define VDEC_IO_FILL_FRM                0x03
#define VDEC_IO_REG_CALLBACK            0x04//!<ע��ص�����
#define VDEC_IO_GET_STATUS              0x05 //!<��ȡvdecģ���״̬��Ϣ
#define VDEC_IO_GET_MODE                0x06//!<��ȡ��ƵԴ����ʽ
#define VDEC_IO_GET_FRM                 0x07
#define VDEC_IO_WHY_IDLE                0x08
#define VDEC_IO_SMLBUF                  0x09
#define VDEC_IO_GET_FREEBUF             0x0A
#define VDEC_IO_FILLRECT_INBG           0x0B
#define VDEC_IO_PRIORITY_STOP_SML       0x0C
#define VDEC_IO_GET_FRM_ADVANCE         0x0D
#define VDEC_IO_DVIEW_EXTRA_MODE        0x0F
#define VDEC_IO_FILL_FRM2               0x10
#define VDEC_IO_ADPCM_CMD               0x13
#define VDEC_IO_ABS_CMD                 0x0E
#define VDEC_IO_ERR_CONCEAL_CMD         0x0F
#define VDEC_IO_PULLDOWN_ONOFF          0x11
#define VDEC_IO_DVIEW_CMD               0x12
#define VDEC_IO_COLORBAR                0x14
#define VDEC_IO_ENABLE_MPDVIEW          0x15
#define VDEC_IO_GET_VDEC_CONFIG         0x16
#define VDEC_IO_SET_SCALE_MODE          0x17
#define VDEC_IO_ENABLE_PREVIEW_PIP      0x18
#define VDEC_IO_FILL_PREVIEW_PIP        0x19
#define VDEC_IO_REG_SYNC_STATUS         0x1a
#define VDEC_IO_STOP_COPY_TASK          0x20
#define VDEC_IO_GET_OUTPUT_FORMAT       0x21
#define VDEC_IO_MAF_CMD                 0x22
#define VDEC_IO_RESET_VE_HW             0x23
#define VDEC_SET_DMA_CHANNEL            0x24
#define VDEC_IO_SWITCH                  0x25
#define VDEC_IO_GET_FRAME_STATISTIC     0x26
#define VDEC_IO_SET_DVIEW_PRECISION     0x27
#define VDEC_VBV_BUFFER_OVERFLOW_RESET  0x28
#define VDEC_IO_GET_I_FRAME_CNT         0x29
#define VDEC_IO_FIRST_I_FREERUN         0x2A
#define VDEC_IO_DISCARD_HD_SERVICE      0x2B
#define VDEC_IO_DONT_RESET_SEQ_HEADER   0x2C
#define VDEC_IO_SET_DROP_FRM            0x2D
#define VDEC_IO_MULTIVIEW_HWPIP         0x2E
#define VDEC_DTVCC_PARSING_ENABLE       0x2f
#define VDEC_CLOSECAPTION_UDMODE        0x30  
#define VDEC_IO_PLAYBACK_PS             0x31
#define VDEC_IO_PLAY_FROM_STORAGE       0x32
#define VDEC_IO_CHANCHG_STILL_PIC       0x33
#define VDEC_IO_SET_SYNC_DELAY          0x34
#define VDEC_IO_SAR_ENABLE              0x35
#define VDEC_IO_FAST_CHANNEL_CHANGE     0x36
#define VDEC_IO_DROP_FRAME_VBVFULL      0x37
#define VDEC_IO_SEAMLESS_SWITCH_ENABLE  0x38
#define VDEC_IO_PAUSE_VIDEO             0x39
#define VDEC_IO_CONTINUE_ON_ERROR       0x3a
#define VDEC_IO_SET_DECODER_MODE        0x40
#define VDEC_IO_SET_FREERUN_THRESHOLD   0x41
#define VDEC_IO_SET_OUTPUT_RECT         0x42
#define VDEC_IO_SET_AVSYNC_GRADUALLY    0x43
#define VDEC_IO_DBLOCK_ENABLE           0x44  
#define VDEC_IO_EN_MUTE_FIRST           0x45
#define VDEC_IO_GET_DECORE_STATUS       0x46
#define VDEC_IO_SET_DEC_FRM_TYPE        0x47
#define VDEC_IO_SET_DMA_CHANNEL_NUMBER  0x48
#define VDEC_IO_SET_BOOTLOGO_ADDR       0x49
#define VDEC_IO_SET_SMOOTH_PREVIEW      0x4a
#define VDEC_IO_SET_DEC_ONLY_ONE_FRAME  0x4b
#define VDEC_IO_CLEAN_STILL_FRAME       0x4c
#define VDEC_IO_GET_CAPTURE_FRAME_INFO  0x4e

#define VDEC_IO_FILL_BG_VIDEO           0x70
#define VDEC_IO_BG_FILL_BLACK           0x71
#define VDEC_IO_RESERVE_MEMORY          0x72    // reserve memory for some feature, i.e. capture displaying frame
#define VDEC_IO_GET_MULTIVIEW_BUF       0x73
#define VDEC_IO_SET_MULTIVIEW_WIN       0x74
#define VDEC_IO_SLOW_PLAY_BEFORE_SYNC   0x75
#define VDEC_IO_DONT_KEEP_PRE_FRAME     0x76
#define VDEC_IO_SET_SEND_GOP_GAP        0x77
#define VDEC_IO_SET_SIMPLE_SYNC         0x78
#define VDEC_IO_SET_TRICK_MODE          0x79
#define VDEC_IO_DYNAMIC_FB_ALLOC        0x80

#define VDEC_IO_ELE_BASE                    0x10000
#define VDEC_IO_PLAYBACK_STR                (VDEC_IO_ELE_BASE + 0x01)
#define VDEC_IO_REG_DROP_FUN                (VDEC_IO_ELE_BASE + 0x02)
#define VDEC_IO_REST_VBV_BUF                (VDEC_IO_ELE_BASE + 0x03)
#define VDEC_IO_KEEP_INPUT_PATH_INFO        (VDEC_IO_ELE_BASE + 0x04)
#define VDEC_IO_PLAY_MEDIA_STR              (VDEC_IO_ELE_BASE + 0x05)
#define VDEC_IO_LIVE_STR_MON                (VDEC_IO_ELE_BASE + 0x06)
#define VDEC_IO_DROP_BEF_FIRT_SHOW          (VDEC_IO_ELE_BASE + 0x07)
#define VDEC_IO_ENABLE_SW_MONITOR           (VDEC_IO_ELE_BASE + 0x08)
#define VDEC_IO_CAPTURE_DISPLAYING_FRAME    (VDEC_IO_ELE_BASE + 0x09)
#define VDEC_IO_PVR_STREAM_INDICATOR        (VDEC_IO_ELE_BASE + 0x0A)
#define VDEC_IO_SHOW_MOSAIC_LEVEL           (VDEC_IO_ELE_BASE + 0x0B)
#define VDEC_IO_REG_GET_SYNC_FLAG_CB        (VDEC_IO_ELE_BASE + 0x0C)
#define VDEC_IO_SET_QUICK_PLAY_MODE         (VDEC_IO_ELE_BASE + 0x0D)
#define VDEC_IO_SET_STILL_FRAME_MODE        (VDEC_IO_ELE_BASE + 0x0E)
#define VDEC_IO_SET_FB_COST_DOWN_NUM        (VDEC_IO_ELE_BASE + 0x0F)
#define VDEC_IO_SET_MODULE_INFO             (VDEC_IO_ELE_BASE + 0x10)

#define VDEC_IO_SET_ROTATION_ANGLE          (VDEC_IO_ELE_BASE + 0x100)//!<������ת�ĽǶ�
#define VDEC_IO_SET_RGB_OUTPUT_FLAG         (VDEC_IO_ELE_BASE + 0x101)//!<����RGB���

#define VDEC_DETACH                         10
#define VDEC_CLOSED                         11
#define VDEC_DECODING                       VDEC27_STARTED
#define VDEC_REVERSING                      12

#define DTVCC_USER_DATA_LENGTH_MAX	        210

struct user_data_pram
{
    UINT32 user_data_size;
    UINT8 user_data[DTVCC_USER_DATA_LENGTH_MAX];
};

struct vdec_capture_frm_info
{
    UINT32 pic_height;
    UINT32 pic_width;
    UINT32 pic_stride;
    UINT32 y_buf_addr;
    UINT32 y_buf_size;
    UINT32 c_buf_addr;
    UINT32 c_buf_size;
    UINT8 de_map_mode;
};

/*! @enum VDecCBType
@brief vdec ģ��Ļص��������͡�
*/
enum VDecCBType
{
    VDEC_CB_SETTING_CHG = 0, //!< δʵ�֡�
    VDEC_CB_REQ_DATA, //!< δʵ�֡�
    VDEC_CB_STOP, //!< δʵ�֡�
    VDEC_CB_FINISH_CUR_PIC, //!< δʵ�֡�
    VDEC_CB_FINISH_I_FRAME, //!< δʵ�֡�
    VDEC_CB_FINISH_TARGET_FRAME, //!< δʵ�֡�
    VDEC_CB_FIRST_SHOWED, //!< �ɹ���ʾ��һ֡
    VDEC_CB_MODE_SWITCH_OK, //!< δʵ�֡�
    VDEC_CB_BACKWARD_RESTART_GOP, //!< ��DVR ����Ӧ�����ش���Ƶ����
    VDEC_CB_OUTPUT_MODE_CHECK, //!< δʵ�֡�
    VDEC_CB_FIRST_HEAD_PARSED, //!< ��������һ��ͷ��Ϣ��
    VDEC_CB_MONITOR_FRAME_VBV, //!< δʵ�֡�
    VDEC_CB_FF_FB_SHOW,
    VDEC_CB_FIRST_I_DECODED,
    VDEC_CB_MONITOR_VDEC_START,
    VDEC_CB_MONITOR_VDEC_STOP,
    VDEC_CB_MONITOR_USER_DATA_PARSED,
};

/*! @enum VDecRotationAngle
@brief vdec ģ�����ת�Ƕȡ�
*/
enum VDecRotationAngle
{
    VDEC_ANGLE_0,
    VDEC_ANGLE_90,
    VDEC_ANGLE_180,
    VDEC_ANGLE_270,

    VDEC_ANGLE_MAX,
};

//! @typedef VDecCBFunc
//! @brief videoģ��Ļص��������͡�
typedef void (*VDecCBFunc)(UINT32 uParam1, UINT32 uParam2);

/*! @struct vdec_io_reg_callback_para
@brief VDEC_IO_REG_CALLBACKע��ص�������صĲ������塣
*/
struct vdec_io_reg_callback_para
{
    enum VDecCBType eCBType;//!< ��ע��ص����������͡�
    VDecCBFunc pCB; //!< ��ע��Ļص�����ָ�롣
    UINT32 monitor_rate;
};

/*! @struct vdec_frm_output_format
@brief decv�����ʽ��
*/
struct vdec_frm_output_format
{
    // VE config
    BOOL h_scale_enable;
    UINT32 h_scale_factor;//0:reserved, 1: Y h_scale only, 2: Y,C h_scale

    BOOL dview_enable;
    UINT32 dv_h_scale_factor;//0:no dview, 1: 1/2 dview, 2: 1/4 dview, 3: 1/8 dview
    UINT32 dv_v_scale_factor;//0:no dview, 1: 1/2 dview, 2: 1/4 dview, 3: 1/8 dview
    
    UINT32 dv_mode; 

    //DE config
    UINT32 field_src;//0: both fields, 1:top only field
    UINT32 scaler_src;//0: frame base, 1: field base
    UINT32 vpp_effort;//0:high, 1: middle, 2: low, 3:very low
};

typedef void (* VDEC_BEYOND_LEVEL)(void);

/*! @struct vdec_device
@brief decv�豸���Ͷ��塣
*/
struct vdec_device
{
    struct vdec_device  *next;  //!< �ڲ�ʹ�á�
        UINT32 type;  //!< �ڲ�ʹ�á�
    INT8 name[32]; //!< �ڲ�ʹ�á�
    UINT8  flags; //!< �ڲ�ʹ�á�

    UINT8 index; //!< �ڲ�ʹ�á�
    void *top_info; //!< �ڲ�ʹ�á�
    void *priv; //!< �ڲ�ʹ�á�

    RET_CODE    (*open)(struct vdec_device *);
    RET_CODE    (*close)(struct vdec_device *);
    RET_CODE    (*start)(struct vdec_device *);
    RET_CODE    (*stop)(struct vdec_device *,BOOL,BOOL);
    RET_CODE    (*vbv_request)(struct vdec_device *, UINT32, void **, UINT32 *, struct control_block *);
    void        (*vbv_update)(struct vdec_device *, UINT32);
    RET_CODE    (*set_output)(struct vdec_device *,  enum VDecOutputMode, struct VDecPIPInfo *, struct MPSource_CallBack *, struct PIPSource_CallBack *);
    RET_CODE    (*switch_pip)(struct vdec_device *, struct Position *);
    RET_CODE    (*switch_pip_ext)(struct vdec_device *, struct Rect*);
    RET_CODE    (*sync_mode)(struct vdec_device *,  UINT8,UINT8);
    RET_CODE    (*extrawin_store_last_pic)(struct vdec_device *, struct Rect *);
    RET_CODE    (*ioctl)(struct vdec_device *, UINT32 , UINT32);
    /* for advanced play */
    RET_CODE    (*playmode)(struct vdec_device *, enum VDecDirection , enum VDecSpeed );
    RET_CODE    (*step)(struct vdec_device *);
    RET_CODE    (*dvr_pause)(struct vdec_device *);
    RET_CODE    (*dvr_resume)(struct vdec_device *);
    RET_CODE    (*profile_level)(struct vdec_device *,  UINT8,VDEC_BEYOND_LEVEL);   
    RET_CODE    (*dvr_set_param)(struct vdec_device *, struct VDec_DvrConfigParam );
        /* end */
    RET_CODE    (*internal_set_output)(struct vdec_device *,  enum VDecOutputMode, struct VDecPIPInfo *, struct MPSource_CallBack *, struct PIPSource_CallBack *);
    RET_CODE    (*internal_set_frm_output_format)(struct vdec_frm_output_format *);
    void        (*de_hw_using)(struct vdec_device *, UINT8, UINT8, UINT8);

};


/*! @enum video_decoder_type
@brief vdec ���������͡�
*/
enum video_decoder_type
{
    MPEG2_DECODER = 0,
    H264_DECODER,
    AVS_DECODER
};




/*!
@brief ��vdec ģ�顣
@param[in] dev ָ��vdecģ�� ��ָ�롣
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE vdec_open(struct vdec_device *dev);

/*!
@brief �ر�vdec ģ�顣
@param[in] dev ָ��vdecģ�� ��ָ�롣
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE vdec_close(struct vdec_device *dev);

/*!
@brief ����vdec ģ��Ľ��빤����
@param[in] dev ָ��vdecģ�� ��ָ�롣
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE vdec_start(struct vdec_device *dev);

/*!
@brief ֹͣvdec ģ��Ľ��빤����
@param[in] dev ָ��vdec ģ�� ��ָ�롣
@param[in] bclosevp �ر�dis ģ��ı�־����0 ��ʾ�رգ���֮�򲻹رա�
@param[in] bfillblack ���dis �ڴ�ı�־����0 ��ʾ��ڣ���֮����ڡ�
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE vdec_stop(struct vdec_device *dev,BOOL bclosevp,BOOL bfillblack);

/*!
@brief ��vdec ģ������д����ڴ�����
@param[in] dev ָ��vdecģ�� ��ָ�롣
@param[in] uSizeRequested �������뵽���ڴ����� ���ȡ�
@param[out] ppVData  ���뵽���ڴ�����ĵ�ַ��
@param[out] puSizeGot ʵ�����뵽���ڴ� ���ȡ�
@param[in] ctrl_blk ��д��������صĿ�����Ϣ������ͬ����Ϣ�ȡ�
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE vdec_vbv_request(void *dev, UINT32 uSizeRequested, void ** ppVData, UINT32 * puSizeGot, struct control_block * ctrl_blk);

/*!
@brief ����vdecģ���д������ָ�롣
@param[in] dev ָ��vdecģ�� ��ָ�롣
@param[in] uDataSize �ɹ�д�����ݵĳ��ȡ�
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
void vdec_vbv_update(void *dev, UINT32 uDataSize);

/*!
@brief ����vdec ģ�� ����Ļ���ģʽ������ȫ����Ԥ��ģʽ��
@param[in] dev ָ��vdecģ�� ��ָ�롣
@param[in] eMode vdec ģ��Ĺ���ģʽ��
@param[in] pInitInfo ����ģʽ�ĳ�ʼ����Ϣ��
@param[out] pMPCallBack ��ͼ����صĻص�������
@param[out] pPIPCallBack ��ͼ����صĻص�������Ŀǰ��δʵ�֡�
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE vdec_set_output(struct vdec_device *dev, enum VDecOutputMode eMode,struct VDecPIPInfo *pInitInfo, struct MPSource_CallBack *pMPCallBack, struct PIPSource_CallBack *pPIPCallBack);

/*!
@brief ����vdec ģ�������Ƶͬ��ģʽ��
@param[in] dev ָ��vdecģ�� ��ָ�롣
@param[in] uSyncMode ͬ��ģʽ������VDEC_SYNC_PTS ��VDEC_SYNC_FREERUN ��
@param[in] uSyncLevel  ͬ������
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE vdec_sync_mode(struct vdec_device *dev, UINT8 uSyncMode,UINT8 uSyncLevel);

/*!
@brief vdec ģ���io contorl ������
@param[in] dev ָ��vdecģ�� ��ָ�롣
@param[in] io_code ���� ���������͡��ο�VDEC_IO_XX���塣
@param[in,out] param �����Ĳ��������ݲ�ͬ��������в�ͬ�Ĳ�����
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
@note  IO����dwCmd ����:
<table class="doxtable"  width="800" border="1" style="border-collapse:collapse;table-layout:fixed;word-break:break-all;" >
  <tr>
    <th width="200">����</th>
    <th width="200">����</th>
    <th width="80">��������</th>
    <th width="320">��ע</th>
  </tr>

  <tr align="center">
    <td>VDEC_IO_GET_STATUS</td>    
    <td>struct VDec_StatusInfo *</td>    
    <td>���</td>
    <td>��ȡ״̬��Ϣ</td>
  </tr>

  <tr align="center">
    <td>VDEC_IO_GET_MODE</td>    
    <td>enum TVSystem *</td>    
    <td>���</td>
    <td>��ȡ��ƵԴ��ģʽ</td>
  </tr>

  <tr align="center">
    <td>VDEC_IO_GET_OUTPUT_FORMAT</td>    
    <td>BOOL *</td>    
    <td>���</td>
    <td>��ȡ��ƵԴ����ɨ����Ϣ�����л�������</td>
  </tr>  

  <tr align="center">
    <td>VDEC_IO_FILL_FRM</td>    
    <td>struct YCbCrColor *</td>    
    <td>����</td>
    <td>����Ƶ������д���ض���ɫ</td>
  </tr>  

  <tr align="center">
    <td>VDEC_IO_REG_CALLBACK</td>    
    <td>struct vdec_io_reg_callback_para *</td>    
    <td>����</td>
    <td>ע��ص�����</td>
  </tr>  

  <tr align="center">
    <td>VDEC_IO_SET_OUTPUT_RECT</td>    
    <td>struct VDecPIPInfo *</td>    
    <td>����</td>
    <td>����Ԥ��ģʽ��صĲ���</td>
  </tr>  

 <tr align="center">
    <td>VDEC_IO_COLORBAR</td>    
    <td>��</td>    
    <td>����</td>
    <td>����Ƶ������д������ź�</td>
  </tr>  
*/
RET_CODE vdec_io_control(struct vdec_device *dev, UINT32 io_code, UINT32 param);

/*!
@brief ��鵱ǰ���е���Ƶ�����������͡�
@return BOOL ��
@retval  0 ��ǰ����Ƶ��������ΪMPEG2��
@retval  !0 ��ǰ����Ƶ��������ΪH.264��
*/
BOOL is_cur_decoder_avc(void);

/*!
@brief �õ���ǰ���е���Ƶ���������豸ָ�롣
@return struct vdec_device *��
@retval  !NULL ��õ�vdec ģ��ָ�롣 
@retval  NULL ���� ʧ�ܣ����������״̬����
*/
struct vdec_device * get_selected_decoder(void);

/*!
@brief ������Ƶ�����������͡�
@param[in] select ���ͱ�־����0��ʾMPEG2, 0��ʾH.264
@param[in] in_preview vdecģʽ�Ĺ���ģʽ����0��ʾԤ��ģʽ��0 ��ʾȫƴģʽ��
*/
void h264_decoder_select(int select, BOOL in_preview);

/*!
@brief ������Ƶ�������������½ӿڣ�֧��MPEG2��H.264��AVS��
@param[in] select ���ͱ�־��0��ʾMPEG2, 1��ʾH.264��2��ʾAVS��
@param[in] in_preview vdecģʽ�Ĺ���ģʽ����0��ʾԤ��ģʽ��0 ��ʾȫƴģʽ��
*/
void video_decoder_select(enum video_decoder_type select, BOOL in_preview);

/*!
@brief ��鵱ǰ���е���Ƶ�����������͡�
@return enum ��
@retval  0 ��ǰ����Ƶ��������ΪMPEG2��
@retval  1 ��ǰ����Ƶ��������ΪH.264��
@retval  2 ��ǰ����Ƶ��������ΪAVS��
*/
enum video_decoder_type get_current_decoder(void);


/*!
@brief ��DVR Ӧ���в��ż���������
@param[in] dev ָ��vdecģ�� ��ָ�롣
@param[in] param ���������ı�־��
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE vdec_dvr_set_param(struct vdec_device *dev, struct VDec_DvrConfigParam param);
    
/*!
@brief ��DVR Ӧ������ͣvdec ģ��Ľ��빤����
@param[in] dev ָ��vdecģ�� ��ָ�롣
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE vdec_dvr_pause(struct vdec_device *dev);

/*!
@brief ��DVR Ӧ���лָ�vdec ģ��Ľ��빤����
@param[in] dev ָ��vdecģ�� ��ָ�롣
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE vdec_dvr_resume(struct vdec_device *dev);

/*!
@brief ��DVR Ӧ��������vdec ģ��Ĺ���ģʽ��
@param[in] dev ָ��vdecģ�� ��ָ�롣
@param[in] direction ��Ƶ���ŵķ���ѡ��ǰ�����ߺ��˷�ʽ��
@param[in] speed ��Ƶ���ŵ��ٶȡ�
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE vdec_playmode(struct vdec_device *dev, enum VDecDirection direction, enum VDecSpeed speed);

/*!
@brief ��DVR Ӧ���а��ղ����ķ�ʽ������Ƶ��
@param[in] dev ָ��vdecģ�� ��ָ�롣
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE vdec_step(struct vdec_device *dev);

/*!
@}
*/

/*!
@}
*/

#endif

