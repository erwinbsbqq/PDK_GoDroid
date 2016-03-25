#ifndef __AVSYNC_H__
#define __AVSYNC_H__

/*! @addtogroup avsync
 *  @{
*/
#ifdef __cplusplus
extern "C" {
#endif

/*================================
  *  IO command start
  *================================*/

#define AVSYNC_IO_ENABLE_DDP_CERTIFICATION   0x01
#define AVSYNC_IO_GET_VIDEO_SMOOTHLY_PLAY_CFG   0x02
#define AVSYNC_IO_ENABLE_GET_STC   0x03
#define AVSYNC_IO_SET_PTS_UNIT_HZ   0x04
#define AVSYNC_IO_UNREG_CALLBACK   0x05
#define AVSYNC_IO_REG_CALLBACK   0x06
#define AVSYNC_IO_GET_CURRENT_PLAY_PTS   0x07
#define AVSYNC_IO_PAUSE   0x08	                   //!< Pause AVSYN module
#define AVSYNC_IO_RESUME   0x09	                  //!< Resume AVSYN module
#define AVSYNC_IO_GET_CURRENT_STC	0x0A              //!< Get current STC
#define AVSYNC_IO_CHANGE_AUDIO_TRACK            0x0B  //!<Change auido track resync

/*================================
  *  IO command end
  *================================*/


/*! @enum AVSYNC_MODE_E
@brief ����Ƶͬ��ģʽ, AVSYNCģ��Ĭ������ΪAVSYNC_MODE_PCR��AVSYNC_MODE_VIDEO�ݲ�֧��
*/
typedef enum 
{
	AVSYNC_MODE_PCR,       //!<��PCRΪ��׼
	AVSYNC_MODE_AUDIO,     //!<����ƵΪ��׼
	AVSYNC_MODE_VIDEO,     //!<����ƵΪ��׼, �ݲ�֧��
	AVSYNC_MODE_V_FREERUN, //!<��Ƶ����ͬ��,ֱ�Ӳ���
	AVSYNC_MODE_A_FREERUN, //!<��Ƶ����ͬ��,ֱ�Ӳ���
	AVSYNC_MODE_AV_FREERUN, //!<����Ƶ����ͬ��,ֱ�Ӳ���
}AVSYNC_MODE_E;

/*! @ enum AVSYNC_SRCTYPE_E
@brief ����Ƶ����Դ���ͣ�AVSYNCģ��Ĭ������ΪAVSYNC_SRC_TURNER
*/
typedef enum 
{
	AVSYNC_SRC_TURNER, //!<��������turner
	AVSYNC_SRC_SWDMX, //!<��������SW DMX
	AVSYNC_SRC_HDD_MP, //!<�������Ա���ý�岥��
	AVSYNC_SRC_NETWORK_MP, //!<������������ý�岥��
}AVSYNC_SRCTYPE_E;

/*! @ enum AVSYNC_DEVICE_STATUS
@brief AVSYNC�豸״̬
*/
typedef enum 
{
	AVSYNC_ATTACHED, //!<�豸�ѹҽ�
	AVSYNC_OPENED,    //!<�豸�Ѵ�
	AVSYNC_CLOSED,    //!<�豸�ѹر�
	AVSYNC_STARTED,  //!<�豸�ѿ�ʼ����
	AVSYNC_STOPPED,	//!<�豸��ֹͣ����	
}AVSYNC_DEV_STATUS;

/*! @ enum AVSYNC_VIDEO_SMOOTH_LEVEL
@brief ��Ƶƽ�����ŵȼ�
*/
typedef enum 
{
	AVSYNC_VIDEO_SMOOTH_LEVEL1, 
	AVSYNC_VIDEO_SMOOTH_LEVEL2, 
}AVSYNC_VIDEO_SMOOTH_LEVEL;

/*! @ struct avsync_cfg_param_t
@brief ����ͬ��������AVSYNCģ��Ĭ�����òο�����������
*/
typedef struct
{
	UINT32 vhold_thres; //!<��Ƶ�ظ����ޣ�Ĭ��ֵΪ80ms
	UINT32 vdrop_thres; //!<��Ƶ�������ޣ�Ĭ��ֵΪ80ms
	UINT32 ahold_thres; //!<��Ƶ�ظ����ޣ�Ĭ��ֵΪ40ms
	UINT32 adrop_thres; //!<��Ƶ�������ޣ�Ĭ��ֵΪ64ms
	AVSYNC_MODE_E sync_mode; //!<����Ƶͬ��ģʽ��Ĭ��ֵΪAVSYNC_MODE_PCR
	AVSYNC_SRCTYPE_E src_type; //!<����Ƶ����Դ���ͣ�Ĭ��ֵΪAVSYNC_SRC_TURNER
}avsync_cfg_param_t;

/*! @ struct avsync_adv_param_t
@brief �߼�ͬ��������AVSYNCģ��Ĭ�����òο�����������
*/
typedef struct
{
	UINT32 afreerun_thres; //!<��Ƶfree run ���ޣ�Ĭ��ֵΪ10s
	UINT32 vfreerun_thres; //!< ��Ƶfree run ���ޣ�Ĭ��ֵΪ10s
	UINT8  disable_monitor; //!< �ر�monitor ���ܱ�־,��Ĭ��ֵΪ1
	UINT8 disable_first_video_freerun; //!<������Ƶ��һ֡free run���ܱ�־��Ĭ��ֵΪ0
	UINT16  dual_output_sd_delay; //!< ��Ƶ˫�����������ӳ�
	UINT32  pts_adjust_threshold;
	UINT32 rsvd2; 
	UINT32 rsvd3;
}avsync_adv_param_t;

/*! @ struct avsync_status_t
@brief ģ��״̬
*/
typedef struct 
{
	AVSYNC_DEV_STATUS device_status; //!<�豸״̬
	UINT32 vpts_offset; //!<��ƵPTSƫ��
	UINT32 apts_offset; //!<��ƵPTSƫ��	
	UINT8 v_sync_flg; //!<��Ƶͬ����־
	UINT8 a_sync_flg; //!<��Ƶͬ����־
	UINT8 rsvd0; //!<Ԥ��
	UINT8 rsvd1; //!<Ԥ��

	UINT32 cur_vpts;//!< ��ǰ��ƵPTS
	UINT32 cur_apts;//!< ��ǰ��ƵPTS	
}avsync_status_t;

/*! @ struct AVSYNC_SRCTYPE_E
@brief ͬ��ͳ����Ϣ
*/
typedef struct 
{
	UINT32 total_v_play_cnt; //!<ͬ����Ƶ֡��
	UINT32 total_v_drop_cnt; //!<������Ƶ֡��
	UINT32 total_v_hold_cnt; // !<�ظ���Ƶ֡��
	UINT32 total_v_freerun_cnt; // !<free-run ��Ƶ֡��
	UINT32 total_a_play_cnt; //!<ͬ����Ƶ֡��
	UINT32 total_a_drop_cnt; //!<��Ƶ����֡��
	UINT32 total_a_hold_cnt; // !<��Ƶ�ظ�֡��
	UINT32 total_a_freerun_cnt;
}avsync_statistics_t;

typedef struct  
{
	UINT8 onoff;
	UINT8 interval;
	AVSYNC_VIDEO_SMOOTH_LEVEL level; 
}avsync_smoothly_play_cfg_t;

/*! @struct avsync_device
@brief �豸���Ͷ���, �����˳�Ա�����Լ��ṩ�Ĳ����ӿڡ�
*/struct avsync_device
{
	struct avsync_device  *next;  /*next device */
	/*struct module *owner;*/
	INT32 type;
	INT8 name[32];
	INT32 flags;

	void *priv;		/* Used to be 'private' but that upsets C++ */
	void      (*attach)();
	void      (*detach)(struct avsync_device **);
	RET_CODE	(*open)(struct avsync_device *);
	RET_CODE   	(*close)(struct avsync_device *);
	RET_CODE	(*start)(struct avsync_device *);
	RET_CODE   	(*stop)(struct avsync_device *);
	RET_CODE   	(*ioctl)(struct avsync_device *, UINT32 , UINT32);	
	RET_CODE     (*reset)(struct avsync_device*);
	RET_CODE     (*set_syncmode)(struct avsync_device*, AVSYNC_MODE_E);
	RET_CODE     (*get_syncmode)(struct avsync_device*, AVSYNC_MODE_E *);
	RET_CODE     (*set_sourcetype)(struct avsync_device*, AVSYNC_SRCTYPE_E);
	RET_CODE     (*get_sourcetype)(struct avsync_device*, AVSYNC_SRCTYPE_E *);
	RET_CODE     (*config_params)(struct avsync_device*, avsync_cfg_param_t *);
	RET_CODE     (*get_params)(struct avsync_device*, avsync_cfg_param_t *);
	RET_CODE     (*config_adv_params)(struct avsync_device*, avsync_adv_param_t *);
	RET_CODE     (*get_adv_params)(struct avsync_device*, avsync_adv_param_t *);
	RET_CODE     (*get_status)(struct avsync_device*, avsync_cfg_param_t *);
	RET_CODE     (*get_statistics)(struct avsync_device*, avsync_cfg_param_t *);
	RET_CODE		(*dbg_set_print_option)(UINT32);
};

/*!
@brief ע���豸avsync��������Դ�ȡ�
@note: ��������AVSYNCģ��ӿں���ǰ�����ȵ��øú�����
*/
RET_CODE avsync_attach(void);

/*!
@brief �Ƴ��豸avsync,������Դ�ȡ�
@param[in] dev ָ���豸avsync��ָ�롣
*/
void avsync_dettach(struct avsync_device*dev);

/*!
@brief ��avsyncģ�顣
@param[in] dev ָ��avsyncģ���ָ�롣
@return RET_CODE��
@retval  RET_SUCCESS       �򿪳ɹ���
@retval  !RET_SUCCESS     ��ʧ�ܣ����������״̬����
*/
RET_CODE avsync_open(struct avsync_device *dev);

/*!
@brief �ر�avsyncģ�顣
@param[in] dev ָ��avsyncģ���ָ�롣
@return RET_CODE��
@retval  RET_SUCCESS       �رճɹ���
@retval  !RET_SUCCESS     �ر�ʧ�ܣ����������״̬����
*/
RET_CODE avsync_close(struct avsync_device *dev);

/*!
@brief �豸avsync�ṩ��ioctl�����ӿڡ� 
@param[in] dev ָ���豸avsync ��ָ�롣
@param[in] io_code ��������
@param[in] param ���������
@return RET_CODE��
@retval  RET_SUCCESS       ���óɹ���
@retval  !RET_SUCCESS     ����ʧ�ܣ����������״̬����
*/
RET_CODE avsync_ioctl(struct avsync_device *dev, UINT32 io_code, UINT32 param);

/*!
@brief ��λͬ��ģ��, ����ͬ�� 
@param[in] dev ָ���豸avsync��ָ�롣
@return RET_CODE��
@retval  RET_SUCCESS       ��λ�ɹ���
@retval  !RET_SUCCESS     ��λʧ�ܣ����������״̬����
*/RET_CODE avsync_reset(struct avsync_device *dev);

/*!
@brief ����ͬ��ģʽ 
@param[in] dev ָ���豸avsync��ָ�롣
@param[in] mode ͬ��ģʽ��ͬ��ģʽ�ο�AVSYNC_MODE_E���塣
@return RET_CODE��
@retval  RET_SUCCESS       ���óɹ���
@retval  !RET_SUCCESS     ����ʧ�ܣ����������״̬����
@note: ��Ӧ�ò㲻���øýӿڣ�AVSYNCģ���ڲ�ʹ��Ĭ�����ã����AVSYNC_MODE_E����
*/
RET_CODE avsync_set_syncmode(struct avsync_device *dev, AVSYNC_MODE_E mode);

/*!
@brief ��ȡͬ��ģʽ 
@param[in] dev ָ���豸avsync��ָ�롣
@param[out] pmode ��ȡ����ͬ��ģʽ��
@return RET_CODE��
@retval  RET_SUCCESS       ��ȡ�ɹ���
@retval  !RET_SUCCESS     ��ȡʧ�ܣ����������״̬����
*/
RET_CODE avsync_get_syncmode(struct avsync_device *dev, AVSYNC_MODE_E *pmode);

/*!
@brief ��������Դ����,��ͬ������Դ����ͬ 
@param[in] dev ָ���豸avsync��ָ�롣
@param[in] type ����Դ����,�ο�AVSYNC_SRCTYPE_E���塣
@return RET_CODE��
@retval  RET_SUCCESS       ���óɹ���
@retval  !RET_SUCCESS     ����ʧ�ܣ����������״̬����
@note: ��Ӧ�ò㲻���øýӿڣ�AVSYNCģ���ڲ�ʹ��Ĭ�����ã����AVSYNC_SRCTYPE_E����
*/
RET_CODE avsync_set_sourcetype(struct avsync_device *dev, AVSYNC_SRCTYPE_E type);

/*!
@brief ��ȡ����Դ���� 
@param[in] dev ָ���豸avsync��ָ�롣
@param[out] ptype ��ȡ��������Դ���͡�
@return RET_CODE��
@retval  RET_SUCCESS       ��ȡ�ɹ���
@retval  !RET_SUCCESS     ��ȡʧ�ܣ����������״̬����
*/
RET_CODE avsync_get_sourcetype(struct avsync_device *dev, AVSYNC_SRCTYPE_E *ptype);

/*!
@brief ���û���ͬ������ 
@param[in] dev ָ���豸avsync��ָ�롣
@param[in] pcfg_params ����ͬ���������á�
@return RET_CODE��
@retval  RET_SUCCESS       ���óɹ���
@retval  !RET_SUCCESS     ����ʧ�ܣ����������״̬����
@note: ��Ӧ�ò㲻���øýӿڣ�AVSYNCģ���ڲ�ʹ��Ĭ�����ã����avsync_cfg_param_t����
*/
RET_CODE avsync_config_params(struct avsync_device *dev,  avsync_cfg_param_t *pcfg_params);

/*!
@brief ��ȡ����ͬ������ 
@param[in] dev ָ���豸avsync��ָ�롣
@param[out] pcfg_params ��ȡ����ͬ��������
@return RET_CODE��
@retval  RET_SUCCESS       ��ȡ�ɹ���
@retval  !RET_SUCCESS     ��ȡʧ�ܣ����������״̬����
*/
RET_CODE avsync_get_params(struct avsync_device *dev,  avsync_cfg_param_t *pcfg_params);

/*!
@brief ���ø߼�ͬ������ 
@param[in] dev ָ���豸avsync��ָ�롣
@param[in] pcfg_params �߼�ͬ���������á�
@return RET_CODE��
@retval  RET_SUCCESS       ���óɹ���
@retval  !RET_SUCCESS     ����ʧ�ܣ����������״̬����
@note: ��Ӧ�ò㲻���øýӿڣ�AVSYNCģ���ڲ�ʹ��Ĭ�����ã����avsync_adv_param_t����
*/
RET_CODE avsync_config_advance_params(struct avsync_device *dev,  avsync_adv_param_t *pcfg_params);

/*!
@brief ��ȡ�߼�ͬ������ 
@param[in] dev ָ���豸avsync��ָ�롣
@param[out] pcfg_params ��ȡ�߼�ͬ��������
@return RET_CODE��
@retval  RET_SUCCESS       ��ȡ�ɹ���
@retval  !RET_SUCCESS     ��ȡʧ�ܣ����������״̬����
*/
RET_CODE avsync_get_advance_params(struct avsync_device *dev,  avsync_adv_param_t *pcfg_params);

/*!
@brief ��ȡģ��״̬ 
@param[in] dev ָ���豸avsync��ָ�롣
@param[out] pstatus ��ȡ����ģ��״̬��
@return RET_CODE��
@retval  RET_SUCCESS       ��ȡ�ɹ���
@retval  !RET_SUCCESS     ��ȡʧ�ܣ����������״̬����
*/
RET_CODE avsync_get_status(struct avsync_device *dev, avsync_status_t *pstatus);

/*!
@brief ��ȡͬ��ͳ����Ϣ 
@param[in] dev ָ���豸avsync��ָ�롣
@param[out] pstatistics ��ȡ����ͬ��ͳ����Ϣ��
@return RET_CODE��
@retval  RET_SUCCESS       ��ȡ�ɹ���
@retval  !RET_SUCCESS     ��ȡʧ�ܣ����������״̬����
*/
RET_CODE avsync_get_statistics(struct avsync_device *dev, avsync_statistics_t *pstatistics);

/*!
@brief ����/�ر���Ƶƽ�����Ź��� 
@param[in] dev ָ���豸avsync��ָ�롣
@param[in] onoff ����/�ر�ƽ�����Ź���
@param[in] level ƽ�����ŵȼ�
@param[in] interval ƽ�����ż��(��λΪ֡��)
@return RET_CODE��
@retval  RET_SUCCESS       ���óɹ���
@retval  !RET_SUCCESS     ����ʧ�ܣ����������״̬����
*/
RET_CODE avsync_video_smoothly_play_onoff(struct avsync_device *dev, UINT8 onoff, AVSYNC_VIDEO_SMOOTH_LEVEL level, UINT8 interval);

/*==========================================
 *  AVSYNC Debug Start
 *==========================================*/
 
//ʵʱ��ӡѡ�� 
#define AVSYNC_DBG_PRINT_DEFAULT 1 //!<��ӡSTC����������Ƶ��ͬ����Ϣ
#define AVSYNC_DBG_PRINT_PCR	2  //!<��ӡPCR��PCR���
#define AVSYNC_DBG_PRINT_APTS	4  //!<��ӡ��ƵPTS��PTS���
#define AVSYNC_DBG_PRINT_VPTS 8  //!<��ӡ��ƵPTS��PTS���
#define AVSYNC_DBG_PRINT_A_SYNC 0x10 //!<��ӡ��Ƶ֡ͬ��ʱPTS/STCֵ
#define AVSYNC_DBG_PRINT_A_UNSYNC_PTS_STC 0x20 //!<��ӡ��Ƶ֡��ͬ��ʱPTS/STCֵ
#define AVSYNC_DBG_PRINT_A_UNSYNC_STCID_OFFSET 0x40 //!<��ӡ��Ƶ֡��ͬ��ʱSTCƫ��
#define AVSYNC_DBG_PRINT_A_UNSYNC_ESBUF 0x80 //!<��ӡ��Ƶ֡��ͬ��ʱ����ES buffer����
#define AVSYNC_DBG_PRINT_V_SYNC 0x100 //!<��ӡ��Ƶ֡ͬ��ʱPTS/STCֵ
#define AVSYNC_DBG_PRINT_V_UNSYNC_PTS_STC 0x200 //!<��ӡ��Ƶ֡��ͬ��ʱPTS/STCֵ
#define AVSYNC_DBG_PRINT_V_UNSYNC_STCID_OFFSET 0x400 //!<��ӡ��Ƶ֡��ͬ��ʱSTCƫ��
#define AVSYNC_DBG_PRINT_V_UNSYNC_VBVBUF 0x800 //!<��ӡ��Ƶ֡��ͬ��ʱ����VBV buffer����
#define AVSYNC_DBG_PRINT_PTS_OFFSET  0x1000    //!<��ӡPTSƫ�Ƶ�����Ϣ
#define AVSYNC_DBG_PRINT_STREAM_LOOP 0x2000    //!<��⵽����ͷʱ��ӡ

#define AVSYNC_DBG_PRINT_VESBUF_OF_UR 0x4000   //!<��ӡ��ƵES buffer�ջ���
#define AVSYNC_DBG_PRINT_AESBUF_OF_UR 0x8000   //!<��ӡ��ƵES buffer�ջ���

#define AVSYNC_DBG_FORCE_SYNCMODE_FREERUN 0x10000   //!<ͬ��ģʽǿ�Ƴ�AVSYNC_MODE_AV_FREERUN
#define AVSYNC_DBG_FORCE_SYNCMODE_PCR 0x20000   //!<ͬ��ģʽǿ�Ƴ�AVSYNC_MODE_AV_PCR
#define AVSYNC_DBG_FORCE_SYNCMODE_AUDIO 0x40000   //!<ͬ��ģʽǿ�Ƴ�AVSYNC_MODE_AUDIO
#define AVSYNC_DBG_FORCE_SYNCMODE_INVALID 0x80000   //!<����ͬ��ģʽ��Ч

#define AVSYNC_DBG_PRINT_API	0x10000000     //!<��ӡAPI����
#define AVSYNC_DBG_PRINT_LOG	0x20000000
#define AVSYNC_DBG_PRINT_ERR	0x40000000     //!<��ӡģ�������Ϣ

//��ѯ����ѡ��
#define AVSYNC_DBG_POLL_V_SYNC_STATISTICS  0x01
#define AVSYNC_DBG_POLL_A_SYNC_STATISTICS  0x02
#define AVSYNC_DBG_POLL_PCR_STATISTICS  0x04
#define AVSYNC_DBG_POLL_VBV_FULL_CNT  0x08
#define AVSYNC_DBG_POLL_PTS_OFFSET  0x10
#define AVSYNC_DBG_POLL_SYNC_MODE 0x20


/*!
@brief ����ʵʱ���Դ�ӡѡ�� 
@param[in] dev ָ���豸avsync��ָ�롣
@param[out] option ��ӡѡ�
@return RET_CODE��
@retval  RET_SUCCESS       ���óɹ���
@retval  !RET_SUCCESS     ����ʧ�ܡ�
*/
RET_CODE avsync_dbg_set_print_option(struct avsync_device *dev, UINT32 option);

/*!
@brief ����/�ر���ѯ����
@param[in] dev ָ���豸avsync��ָ�롣
@param[out] on_off ����/�ر���ѯ���ԡ�
@return RET_CODE��
@retval  RET_SUCCESS       ���óɹ���
@retval  !RET_SUCCESS     ����ʧ�ܡ�
*/
RET_CODE avsync_dbg_polling_onoff(struct avsync_device *dev, UINT32 on_off);

/*!
@brief ������ѯ����ѡ��
@param[in] dev ָ���豸avsync��ָ�롣
@param[out] option ��ѯ����ѡ�
@return RET_CODE��
@retval  RET_SUCCESS       ���óɹ���
@retval  !RET_SUCCESS     ����ʧ�ܡ�
*/
RET_CODE avsync_dbg_set_polling_option(struct avsync_device *dev, UINT32 option);

/*!
@brief ������ѯ���Դ�ӡ���
@param[in] dev ָ���豸avsync��ָ�롣
@param[out] interval_ms ��ѯ��ӡ�������λΪ����
@return RET_CODE��
@retval  RET_SUCCESS       ���óɹ���
@retval  !RET_SUCCESS     ����ʧ�ܡ�
*/
RET_CODE avsync_dbg_set_polling_interval(struct avsync_device *dev, UINT32 interval_ms);

/*==========================================
 *  AVSYNC Debug End
 *==========================================*/
#ifdef __cplusplus 
}
#endif
#endif
