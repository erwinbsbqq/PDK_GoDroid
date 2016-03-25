

#include <sys_config.h>
#include <basic_types.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
//#include <errno.h>

//#include <errno.h>

//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <math.h>
//corei#include <err/errno.h>

#include <poll.h>


#include <osal/osal.h>

#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#ifdef ADR_ALIDROID
#include <linux/sem.h>
#else
#include <sys/sem.h>
#endif
#include <sys/ipc.h>
#include <sys/vfs.h>

#include <linux/fb.h>
#include <errno.h>
//#include <ali_video_common.h>
//#include <ali_decv_plugin_common.h>
#include <ali_basic_common.h>
#include <linux/Ali_DmxLibInternal.h>
#include <ali_dmx_common.h>
#include <linux/dvb/audio.h>
//#include <linux/dvb/frontend.h>


#include <linux/input.h>




#include <unistd.h>   

#include <hld/hld_dev.h>


#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/decv.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/nim/nim_dev.h>

#include <bus/tsi/tsi.h>

//corei#include <api/libpvr/lib_pvr.h>



#include <ali_tsg_common.h>

#include <ali_tsi_common.h>

#include <ali_dsc_common.h>


#include <asm/unistd.h>


#define DMX_DSC_LINUX_DEV_PATH "/dev/ali_m36_dsc_0"

#define DMX_0_LINUX_DEV_PATH "/dev/ali_m36_dmx_0"

#define DMX_1_LINUX_DEV_PATH "/dev/ali_m36_dmx_1"

#define DMX_2_LINUX_DEV_PATH "/dev/ali_dmx_pb_0_out"

#define DMX_PB_IN_LINUX_DEV_PATH "/dev/ali_dmx_pb_0_in"


#define DMX_HLD_PORTING_SEC_SLOT_CNT 22

#define DMX_HLD_PORTING_PES_SLOT_CNT 10

#define DMX_HLD_PORTING_REC_SLOT_CNT 4

#define DMX_HLD_PORTING_CSA_SLOT_CNT 36

#define DMX_HLD_PORTING_SEC_REQ_DEFAULT_DELAY 6000

#define DMX_HLD_PORTING_PLAYBACK_REQ_LEN 3008 /* 16 * 188 */

#define DMX_HLD_PORTING_PLAYBACK_MAX_REMAIN_LEN 188*256

#define DMX_OTHER_PORTING_CNT (DMX_HLD_PORTING_CSA_SLOT_CNT-6)

#define DMX_HLD_PORTING_NEW_SERVICE_SLOT_CNT 16

#define PVR_DECRYPT_KEY_LEN 128

#define FAST_PLAYBACK_SPEED			10
#define NORMAL_PLAYBACK_SPEED			100

#if 0
#define DMX_DBG_API_PRINT printf
#else
#define DMX_DBG_API_PRINT(...)
#endif


#define DMX_HLD_PORTING_MAX_LINUX_DEV_PATH_LEN 128

#define PES_HEAD_PTS_LEN	184

enum DMX_PORTING_AV_STREAM_VALID
{
    DMX_PORTING_AV_V_VALID = 0x1,
    DMX_PORTING_AV_A_VALID = 0x2,
    DMX_PORTING_AV_AD_VALID = 0x4
};


enum DMX_HLD_PORTING_DATA_SRC
{
    DMX_HLD_PORTING_DMX_DATA_SRC_LIVE = 0,
    DMX_HLD_PORTING_DMX_DATA_SRC_PLAYBACK
};



enum DMX_HLD_PORTING_STATUS
{
    DMX_HLD_PORTING_STATUS_IDLE = 0,
    DMX_HLD_PORTING_STATUS_RUN,
    DMX_HLD_PORTING_STATUS_PAUSE,
    DMX_HLD_PORTING_STATUS_STOP,
    DMX_HLD_PORTING_STATUS_STOP_NEXT,
};


enum DMX_HLD_PORTING_AV_STREAM_STATUS
{
    DMX_HLD_PORTING_AV_STREAM_READY = 0,
    DMX_HLD_PORTING_AV_STREAM_RUN
};

enum DMX_HLD_PORTING_SEC_SLOT_STATUS
{
    DMX_HLD_PORTING_SEC_SLOT_IDLE = 0,
    DMX_HLD_PORTING_SEC_SLOT_PAUSE,
    DMX_HLD_PORTING_SEC_SLOT_RUN
};

enum DMX_HLD_PORTING_PES_SLOT_STATUS
{
    DMX_HLD_PORTING_PES_SLOT_IDLE = 0,
    DMX_HLD_PORTING_PES_SLOT_PAUSE,
    DMX_HLD_PORTING_PES_SLOT_RUN
};

enum DMX_HLD_PORTING_REC_SLOT_STATUS
{
    DMX_HLD_PORTING_REC_SLOT_IDLE = 0,
    DMX_HLD_PORTING_REC_SLOT_RUN
};


enum DMX_HLD_PORTING_PLAYBACK_SLOT_STATUS
{
    DMX_HLD_PORTING_PLAYBACK_READY = 0,
    DMX_HLD_PORTING_PLAYBACK_RUN
};

enum DMX_HLD_PORTING_REC_SLOT_FORMAT
{
    DMX_HLD_PORTING_REC_FORMAT_TS_MPEG2,
    DMX_HLD_PORTING_REC_FORMAT_TS_H264,
    DMX_HLD_PORTING_REC_FORMAT_TS_RAW,
    DMX_HLD_PORTING_REC_FORMAT_PS_MPEG2,

    DMX_HLD_PORTING_REC_FORMAT_PES,
    DMX_HLD_PORTING_REC_FORMAT_ES,
};


enum DMX_HLD_PORTING_TIMESHIFT_STATUS
{
    DMX_HLD_PORTING_TIMESHIFT_NONE,
    DMX_HLD_PORTING_TIMESHIFT_FROM_PLAY,
    DMX_HLD_PORTING_TIMESHIFT_FROM_IDLE
};

enum DMX_HLD_PORTING_CSA_SLOT_STATUS
{
    ALI_CSA_SLOT_STATUS_IDLE,
    ALI_CSA_SLOT_STATUS_READY,
    ALI_CSA_SLOT_STATUS_RUN,
};

enum DMX_HLD_PORTING_NEW_SERVICE_SLOT_STATUS
{
    DMX_HLD_PORTING_NEW_SERVICE_SLOT_IDLE = 0,
    DMX_HLD_PORTING_NEW_SERVICE_SLOT_PAUSE,
    DMX_HLD_PORTING_NEW_SERVICE_SLOT_RUN
};

struct dmx_hld_porting_avp_slot
{
    UINT16 pid;
    INT32 linux_fd;



    UINT32 csa_slot_idx;

};







struct dmx_hld_porting_sec_slot
{
    enum DMX_HLD_PORTING_SEC_SLOT_STATUS status;

    /* One linux dmx file for one sec slot. */
    INT32 linux_fd;
    UINT32 csa_slot_idx;
    struct get_section_param *sec_para;

    UINT32 id;

    /* Point back to parent struct dmx_hld_porting. */
    UINT8 *porting;
};


struct dmx_hld_porting_rec_slot
{
    enum DMX_HLD_PORTING_REC_SLOT_STATUS status;

    UINT16 user_pid_list[DMX_REC_PID_LIST_MAX_LEN];

    UINT32 user_pid_list_len; /* Counted in UINT16 unit. */

    UINT16 orig_pid_list[DMX_REC_PID_LIST_MAX_LEN];

    UINT32 orig_pid_list_len; /* Counted in UINT16 unit. */

    UINT32 dynamic_pid_en;

    UINT16 dynamic_pid_list[DMX_REC_PID_LIST_MAX_LEN];

    UINT32 dynamic_pid_list_len; /* Counted in UINT16 unit. */

    /* Linux dmx file for retrieving data for recording.*/
    INT32 linux_fd;

    /* Rec flags. */
    enum DMX_HLD_PORTING_REC_SLOT_FORMAT out_format;

    UINT32 rec_whole_tp;

    UINT8  *buf;
    
    UINT8   tail_buf[16];
    UINT32  rec_hdl;

    UINT32  buf_len;

    UINT32  buf_wr;

    UINT16  cur_buf_i_frm_offset;

    UINT16  pri_buf_i_frm_offset;

    /* For i frame seeking. */
    UINT8   last_5_byte_in_pri_ts[5];

    INT32 (*req_buf)(UINT32 hdl, UINT8 **buf, INT32 req_len, INT32 *got_len);

    BOOL (*ret_buf)(UINT32 hdl, UINT32 data_len, UINT16 i_fm_offset);  
    
    void *enc_para;
};


struct dmx_hld_porting_pes_slot
{
    enum DMX_HLD_PORTING_PES_SLOT_STATUS status;

    /* One linux dmx file for one pes slot. */
    INT32 linux_fd;

    UINT16 filter_id;   //Save the filter_ID from AP usage 
    UINT16 pid;  //Save the PID

    enum DMX_HLD_PORTING_REC_SLOT_FORMAT out_format;

	//Buffer management for PES Data 
    UINT8  *buf;

    UINT32  pes_hdl;
    
    UINT32  buf_len;
    UINT32  buf_wr;
    struct control_block pSDataCtrlBlk;	//Add to compat with TDS API

    /* Point back to parent struct dmx_hld_porting. */
    UINT8 *porting;    

	/* PES slot statistics information. */
	struct Ali_DmxSlotPesStatInfo stat_info;

	INT32 (*pes_req_buf)(void * pdev, UINT32 uSizeRequested,void ** ppuData,UINT32* puSizeGot,struct control_block* pSDataCtrlBlk);
    void (*pes_ret_buf)(void * pdev,UINT32 uDataSize);
};


struct dmx_hld_porting_playback_slot
{
    enum DMX_HLD_PORTING_PLAYBACK_SLOT_STATUS status;

    UINT8 linux_dev_path[DMX_HLD_PORTING_MAX_LINUX_DEV_PATH_LEN];
    
    /* Linux file for write playback stream data into. */
    INT32 linux_fd;

    UINT32 hnd;

    UINT8 h264_flag;

    UINT8 is_scrambled;

    UINT8 rec_type;   //0:TS, 1:PS.

    INT32 (*data_req)(UINT32 hdl, UINT8 **data, INT32 req_len, INT32 *vbu_flag); 

    INT32 (*request_key)(UINT32, UINT8 *, UINT32 *, UINT8 *, UINT8 *);

    __u32 need_start;

    __u32 need_stop;

    OSAL_ID pb_run_to_idle_done_sem;

    __u32 pb_task_interval;
    UINT8 is_radio;
};



struct dmx_hld_porting_ttx_slot
{
    //enum DMX_HLD_PORTING_SEC_SLOT_STATUS status;

    /* One linux dmx file for one sec slot. */
    INT32 linux_fd;

    UINT16 pid;
};


struct dmx_hld_porting_subt_slot
{
    //enum DMX_HLD_PORTING_SEC_SLOT_STATUS status;

    /* One linux dmx file for one sec slot. */
    INT32 linux_fd;

    UINT16 pid;
};

struct dmx_hld_porting_hw_ctl_slot
{
    INT32 linux_fd;
};

struct dmx_hld_porting_csa_slot
{
    UINT32 idx;

    enum DMX_HLD_PORTING_CSA_SLOT_STATUS status;
    
    UINT8 cw_pair[16];
    KEY_PARAM CsaParamList;
};

struct dmx_hld_porting_new_service_slot
{
    UINT16 pid;
    /* One linux dmx file for one sec slot. */
    INT32  linux_fd;
    UINT32 id;
    UINT8  *buf;
    UINT32  buf_len;
    UINT32  buf_wr;
    enum DMX_HLD_PORTING_NEW_SERVICE_SLOT_STATUS status;
    struct register_service_new new_service_param;
    struct control_block ctrl_blk;
};

struct dmx_hld_porting
{
    OSAL_ID ioctl_mutex;

    UINT16 tds_dmx_id;

    enum DMX_HLD_PORTING_DATA_SRC data_src;

    UINT8 linux_dmx_path[DMX_HLD_PORTING_MAX_LINUX_DEV_PATH_LEN];

    enum DMX_HLD_PORTING_STATUS status;

    /* AV play. */
    enum DMX_HLD_PORTING_AV_STREAM_STATUS av_stream_status;

    struct dmx_hld_porting_avp_slot v_slot;
    struct dmx_hld_porting_avp_slot a_slot;
    struct dmx_hld_porting_avp_slot p_slot;
    struct dmx_hld_porting_avp_slot ad_slot;
    struct dmx_hld_porting_avp_slot other_slot[DMX_OTHER_PORTING_CNT];
    struct dmx_hld_porting_new_service_slot new_service_slot[DMX_HLD_PORTING_NEW_SERVICE_SLOT_CNT];

    /* Teletext pid. */
    struct dmx_hld_porting_ttx_slot ttx_slot;

    /* Subtitle pid. */
    struct dmx_hld_porting_subt_slot subt_slot;

    /* Subtitle pid. */
    struct dmx_hld_porting_subt_slot dcii_subt_slot;

    /* PES slots. */
    struct dmx_hld_porting_pes_slot pes_slot[DMX_HLD_PORTING_PES_SLOT_CNT]; 
    OSAL_ID pes_dispatch_task_id;

	/* only used in teletext, subtitle and closecaption */
	enum demux_data_flow m_demux_data_flow;

	enum demux_data_flow m_demux_data_flow_ttx;

	enum demux_data_flow m_demux_data_flow_subt;

	enum demux_data_flow m_demux_data_flow_dcii_subt;

    /* Section slots. */
    struct dmx_hld_porting_sec_slot sec_slot[DMX_HLD_PORTING_SEC_SLOT_CNT]; 

    struct dmx_hld_porting_csa_slot csa_slot[DMX_HLD_PORTING_CSA_SLOT_CNT];
    OSAL_ID sec_dispath_task_id;

    OSAL_ID sec_hit_flag;

    /* Rec slots. */
    struct dmx_hld_porting_rec_slot rec_slot[DMX_HLD_PORTING_REC_SLOT_CNT];

    OSAL_ID rec_task_id;

    /* Playback slot. */
    struct dmx_hld_porting_playback_slot playback_slot;

    OSAL_ID playback_task_id;
    
    OSAL_ID new_service_task_id;

    INT32 timeshift_status;

    /* HW control slot. */
    struct dmx_hld_porting_hw_ctl_slot hw_ctl_slot;

    /* Linux dmx SEE file discriptor. */
    INT32 see_dev_fd;

    enum dmx_see_av_sync_mode see_av_sync_mode;

     /* CSA related. */
    UINT32 csa_en;
    UINT32 csa_ver;
    
    pDEEN_CONFIG p_DeEnconfig;
	
	UINT32 audio_disable;
    UINT32  fe;
    UINT32 nim_chip_id;
};


struct dmx_device g_tds_dmx_dev[3];

struct dmx_hld_porting g_dmx_hld_porting[3];

struct dmx_device *g_dmx_playing_av = NULL;
static UINT32 time_pcr = 0;

#if 0
RET_CODE dmx_see_av_scram_status
(
    struct dmx_device *dmx,
    UINT32             scram_status
)
{
    RET_CODE                      ret;
    struct dmx_hld_porting       *porting;

    //DMX_DBG_API_PRINT("%s,%d,mode:%d\n", __FUNCTION__, __LINE__, av_sync_mode);

    porting = (struct dmx_hld_porting *)dmx->priv;

    ret = ioctl(porting->see_dev_fd, ALI_DMX_SEE_AV_SCRAMBLE_STATUS,
                (UINT8 *)scram_status);

    if (ret < 0)
    {        
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }   

    if (*((UINT8 *)scram_status) != 0)
    {
        //DMX_DBG_API_PRINT("%s,%d,0x%x\n", __FUNCTION__, __LINE__, *((UINT8 *)scram_status));

        return(RET_SUCCESS);
    }

    //DMX_DBG_API_PRINT("%s,%d,0x%x\n", __FUNCTION__, __LINE__, *((UINT8 *)scram_status));

    return(!RET_SUCCESS);

    //DMX_DBG_API_PRINT("%s,%d,%s\n", __FUNCTION__, __LINE__, dmx->name);
}
#else
RET_CODE dmx_see_av_scram_status
(
    struct dmx_device *dmx,
    UINT32             scram_status
)
{
    RET_CODE                    ret;
    struct dmx_hld_porting     *porting;
	struct Ali_DmxSeeStatistics SeeStatistics;
    UINT32                      pcr;
	UINT8                       status;
	
	/* Test.
	 */
    //dmx_io_control(dmx, DMX_GET_LATEST_PCR, (UINT32)(&pcr));

	//DMX_DBG_API_PRINT("pcr:%x\n", pcr);

    //printf("%s,%d,%s\n", __FUNCTION__, __LINE__, dmx->name);

    porting = (struct dmx_hld_porting *)dmx->priv;

    ret = ioctl(porting->hw_ctl_slot.linux_fd, ALI_DMX_SEE_GET_STATISTICS,
                &SeeStatistics);

    if (ret < 0)
    {        
        printf("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }   

    status = 0;
	
    if (SeeStatistics.AudioTsCurScramFlag != 0)
    {
		status |= AUD_TS_SCRBL;
	}
	else if (SeeStatistics.AudioTsPusiMismatchFlag != 0)
    {
		status |= AUD_STR_INVALID;
    }
	
    if (SeeStatistics.VideoTsCurScramFlag != 0)
    {
		status |= VDE_TS_SCRBL;
	}
	else if (SeeStatistics.VideoTsPusiMismatchFlag != 0)
    {
		status |= VDE_STR_INVALID;
    }

    //printf("%s,%d,PCR:0x%x\n", __FUNCTION__, __LINE__, SeeStatistics.Pcr);
	
    if (status != 0)
	{
        *((UINT8 *)scram_status) = status;
		
        //printf("%s,%d,0x%x\n", __FUNCTION__, __LINE__, *((UINT8 *)scram_status));

        return(RET_SUCCESS);
    }

    //printf("%s,%d,0x%x\n", __FUNCTION__, __LINE__, *((UINT8 *)scram_status));

    return(!RET_SUCCESS);

    //DMX_DBG_API_PRINT("%s,%d,%s\n", __FUNCTION__, __LINE__, dmx->name);
}
#endif


RET_CODE dmx_see_av_scram_status_ext
(
    struct dmx_device *dmx,
    UINT32             param
)
{
    /* Warining! Potentially bug, in TDS app, struct io_param is used as param,
     * but in SEE driver, struct io_param_ext is used as param.
     * Whatever, there is no choice but to leave it as it is to keep compatible
     * with TDS.
     */
    struct io_param              *scram_param_tds;
    struct dmx_see_av_scram_info  scram_param_linux;
    RET_CODE                      ret;
    struct dmx_hld_porting       *porting;
    UINT16                       *pid_list;
	UINT8                         status;
	struct Ali_DmxSeeStatistics   SeeStatistics;	
	
    //printf("%s,%d,%s\n", __FUNCTION__, __LINE__, dmx->name);

    porting = (struct dmx_hld_porting *)dmx->priv;

    scram_param_tds = (struct io_param *)param;

    pid_list = (UINT16 *)(scram_param_tds->io_buff_in);

    #if 0
    scram_param_linux.pid[0] = pid_list[0];

    scram_param_linux.pid[1] = pid_list[1];    

    ret = ioctl(porting->see_dev_fd, ALI_DMX_SEE_AV_SCRAMBLE_STATUS_EXT,
                &scram_param_linux);

    if (ret < 0)
    {        
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }   

    *(scram_param_tds->io_buff_out) = scram_param_linux.scram_status;

    if (scram_param_linux.scram_status != 0)
    {
        printf("%s,%d,0x%x\n", __FUNCTION__, __LINE__, *(scram_param_tds->io_buff_out));

        return(RET_SUCCESS);
    }

    printf("%s,%d,0x%x\n", __FUNCTION__, __LINE__, (scram_param_tds->io_buff_out));

    return(!RET_SUCCESS);

    //DMX_DBG_API_PRINT("%s,%d,%s\n", __FUNCTION__, __LINE__, dmx->name);
	#else
    ret = ioctl(porting->hw_ctl_slot.linux_fd, ALI_DMX_SEE_GET_STATISTICS,
                &SeeStatistics);

    if (ret < 0)
    {        
        printf("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }   

    status = 0;

	if (porting->a_slot.pid == pid_list[1])
	{
        if (SeeStatistics.AudioTsCurScramFlag != 0)
        {
    		status |= AUD_TS_SCRBL;
    	}
    	else if (SeeStatistics.AudioTsPusiMismatchFlag != 0)
        {
    		status |= AUD_STR_INVALID;
        }
	}

	if (porting->v_slot.pid == pid_list[0])
	{	
        if (SeeStatistics.VideoTsCurScramFlag != 0)
        {
    		status |= VDE_TS_SCRBL;
    	}
    	else if (SeeStatistics.VideoTsPusiMismatchFlag != 0)
        {
    		status |= VDE_STR_INVALID;
        }
	}

    *(scram_param_tds->io_buff_out) = status;

    if (status != 0)
    {
        //printf("%s,%d,0x%x\n", __FUNCTION__, __LINE__, *(scram_param_tds->io_buff_out));

        return(RET_SUCCESS);
    }

    //printf("%s,%d,0x%x\n", __FUNCTION__, __LINE__, *(scram_param_tds->io_buff_out));

    return(!RET_SUCCESS);

	#endif
}




RET_CODE dmx_see_av_src_scram_status
(
	struct dmx_device *dmx,
	UINT32			   scram_status
)
{
    RET_CODE                    ret;
    struct dmx_hld_porting     *porting;
	struct Ali_DmxSeeStatistics SeeStatistics;
    UINT32                      pcr;
	UINT8                       status;
	
	/* Test.
	 */
    //dmx_io_control(dmx, DMX_GET_LATEST_PCR, (UINT32)(&pcr));

	//DMX_DBG_API_PRINT("pcr:%x\n", pcr);

    //DMX_DBG_API_PRINT("%s,%d,mode:%d\n", __FUNCTION__, __LINE__, av_sync_mode);

    porting = (struct dmx_hld_porting *)dmx->priv;

    ret = ioctl(porting->hw_ctl_slot.linux_fd, ALI_DMX_SEE_GET_STATISTICS,
                &SeeStatistics);

    if (ret < 0)
    {        
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }   

    status = 0;
	
    if (SeeStatistics.AudioTsSrcScramFlag != 0)
    {
		status |= AUD_TS_SCRBL;
	}
	
    if (SeeStatistics.VideoTsSrcScramFlag != 0)
    {
		status |= VDE_TS_SCRBL;
	}

    //DMX_DBG_API_PRINT("AudioTsCurScramFlag:%d\n", SeeStatistics.AudioTsCurScramFlag);
    //DMX_DBG_API_PRINT("AudioTsDupCnt:%d\n", SeeStatistics.AudioTsDupCnt);
    //DMX_DBG_API_PRINT("AudioTsLostCnt:%d\n", SeeStatistics.AudioTsLostCnt);
    //DMX_DBG_API_PRINT("AudioTsPusiMismatchFlag:%d\n", SeeStatistics.AudioTsPusiMismatchFlag);

    //DMX_DBG_API_PRINT("VideoTsCurScramFlag:%d\n", SeeStatistics.VideoTsCurScramFlag);
    //DMX_DBG_API_PRINT("VideoTsDupCnt:%d\n", SeeStatistics.VideoTsDupCnt);
    //DMX_DBG_API_PRINT("VideoTsLostCnt:%d\n", SeeStatistics.VideoTsLostCnt);
    //DMX_DBG_API_PRINT("VideoTsPusiMismatchFlag:%d\n", SeeStatistics.VideoTsPusiMismatchFlag);

    if (status != 0)
	{
        *((UINT8 *)scram_status) = status;
		
        //printf("%s,%d,0x%x\n", __FUNCTION__, __LINE__, *((UINT8 *)scram_status));

        return(RET_SUCCESS);
    }

    //printf("%s,%d,0x%x\n", __FUNCTION__, __LINE__, *((UINT8 *)scram_status));

    return(!RET_SUCCESS);

    //DMX_DBG_API_PRINT("%s,%d,%s\n", __FUNCTION__, __LINE__, dmx->name);
}



RET_CODE dmx_see_av_sync_mode_set
(
    struct dmx_device         *dmx,
    enum dmx_see_av_sync_mode  av_sync_mode
)
{
    RET_CODE                ret;
    struct dmx_hld_porting *porting;
    struct dmx_see_av_pid_info  see_av_info;

    //printf("%s,%d,mode:%d\n", __FUNCTION__, __LINE__, av_sync_mode);

    porting = (struct dmx_hld_porting *)dmx->priv;
    DMX_DBG_API_PRINT("%s,%d,mode:%d pmode=%d\n", __FUNCTION__, __LINE__, av_sync_mode,porting->see_av_sync_mode);
    porting->see_av_sync_mode = av_sync_mode;

    ret = ioctl(porting->see_dev_fd, ALI_DMX_SEE_AV_SYNC_MODE_SET,
                &porting->see_av_sync_mode);

    if (ret < 0)
    {        
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }   

    //DMX_DBG_API_PRINT("%s,%d,%s\n", __FUNCTION__, __LINE__, dmx->name);

    return(RET_SUCCESS);
}



RET_CODE dmx_see_av_stop
(
    struct dmx_device *dmx,
    UINT32 param
)
{
    struct dmx_hld_porting *porting;

    porting = (struct dmx_hld_porting *)dmx->priv;

    /* Step 1: Stop SEE parsing. */
    ioctl(porting->see_dev_fd, ALI_DMX_SEE_AV_STOP, param);

    return(RET_SUCCESS);
}


RET_CODE dmx_see_av_start
(
    struct dmx_device *dmx
)
{
    RET_CODE                   ret;
    struct dmx_hld_porting    *porting;
    struct dmx_see_av_pid_info     see_av_info;
    enum dmx_see_av_sync_mode  see_av_sync_mode;

    porting = (struct dmx_hld_porting *)dmx->priv;

    DMX_DBG_API_PRINT("%s, %d porting->see_av_sync_mode=%d\n", __FUNCTION__, __LINE__,porting->see_av_sync_mode);

#if 1

    see_av_sync_mode = porting->see_av_sync_mode;
    if(porting->see_av_sync_mode != DMX_SEE_AV_SYNC_MODE_TSG_TIMESHIT)
    {
        see_av_sync_mode = DMX_SEE_AV_SYNC_MODE_LIVE;    
    }

    if (DMX_HLD_PORTING_DMX_DATA_SRC_PLAYBACK == porting->data_src)
    {
        see_av_sync_mode = DMX_SEE_AV_SYNC_MODE_PLAYBACK;
    }
#else
    see_av_sync_mode = DMX_SEE_AV_SYNC_MODE_LIVE; 
#endif

    DMX_DBG_API_PRINT("%s, %d see_av_sync_mode=%d\n", __FUNCTION__, __LINE__,see_av_sync_mode);
    dmx_see_av_sync_mode_set(dmx, see_av_sync_mode);

    /* Step 2: Restart SEE parsing with new PIDs. */
    /* PID */
    see_av_info.v_pid = porting->v_slot.pid;

    see_av_info.a_pid = porting->a_slot.pid;

    see_av_info.ad_pid = porting->ad_slot.pid;

#if 1
    see_av_info.p_pid = porting->p_slot.pid;
#else
    see_av_info.p_pid = 0x1FFF;
#endif

    ret = ioctl(porting->see_dev_fd, ALI_DMX_SEE_AV_START, &see_av_info);

    if (ret < 0)
    {        
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }   

    return(RET_SUCCESS);
}




RET_CODE dmx_avp_slot_set_pid
(
    struct dmx_device               *dmx,
    struct dmx_hld_porting_avp_slot *slot,
    UINT16                           pid
)
{
    slot->pid = pid;

    DMX_DBG_API_PRINT("%s, %d, pid:%d\n", __FUNCTION__, __LINE__, pid&0x1FFF);

    return(RET_SUCCESS);
}


/* Pause A/V stream. */
RET_CODE dmx_avp_slot_stop
(
    struct dmx_device               *dmx,
    struct dmx_hld_porting_avp_slot *slot
)
{
    INT32 ret;
    struct dmx_hld_porting *porting = (struct dmx_hld_porting *)dmx->priv;

    DMX_DBG_API_PRINT("%s, %d\n", __FUNCTION__, __LINE__);
	pCSA_DEV pCsaDev = (pCSA_DEV)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);

    /* Stop filter file. */
    if (slot->linux_fd >= 0)
    {
        //dmx_channel_release sometimes can't be called because of f_count
        ioctl(slot->linux_fd, ALI_DMX_CHANNEL_STOP, 0);
        close(slot->linux_fd);

        slot->linux_fd = -1;
    }


	struct dmx_hld_porting_csa_slot *csa_slot;

	csa_slot = &(porting->csa_slot[slot->csa_slot_idx]);

    //printf("%s,%d,handle:%d\n", __FUNCTION__, __LINE__, csa_slot->CsaParamList.handle);

	if (csa_slot->status != ALI_CSA_SLOT_STATUS_IDLE)
	{
		//printf("%s, %d\n", __FUNCTION__, __LINE__);

		ret = csa_ioctl(pCsaDev, IO_DELETE_CRYPT_STREAM_CMD, (UINT32)(csa_slot->CsaParamList.handle));
		
		if(ret != RET_SUCCESS)
		{
			printf("%s, line:%d\n", __FUNCTION__, __LINE__);
			
			return(RET_FAILURE);
		}
		
		csa_slot->status = ALI_CSA_SLOT_STATUS_IDLE;
	}
    ioctl(slot->linux_fd, ALI_DMX_RESET_BITRATE_DETECT, 0);

    DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(RET_SUCCESS);
}



/* Start A/V stream. */
RET_CODE dmx_avp_slot_start
(
    struct dmx_device               *dmx,
    struct dmx_hld_porting_avp_slot *slot
)
{
    RET_CODE                      ret;
    struct dmx_hld_porting       *porting;
    struct dmx_channel_param      ch_para;
    struct dmx_ts_kern_recv_info *ts_kern_recv_info;

    DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    if (0x1FFF == (slot->pid & 0x1FFF))
    {
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_SUCCESS);
    }

	memset(&ch_para, 0, sizeof(struct dmx_channel_param));

    /* Open new fileres in main CPU. */
    porting = (struct dmx_hld_porting *)dmx->priv;

    slot->linux_fd = open(porting->linux_dmx_path, O_RDONLY);

    if (slot->linux_fd < 0)
    {        
        dmx_avp_slot_stop(dmx, slot);

        DMX_DBG_API_PRINT("%s, line:%d, %s\n", __FUNCTION__, __LINE__,
                          porting->linux_dmx_path);

        return(RET_FAILURE);
    }    

	if (slot == &(porting->p_slot))
	{
        //printf("%s,%d,pid;%d\n", __FUNCTION__, __LINE__, slot->pid);
	
        ch_para.output_format = DMX_CHANNEL_OUTPUT_FORMAT_PCR;
    
        ch_para.output_space = DMX_OUTPUT_SPACE_KERNEL;

		ch_para.pcr_param.pid = slot->pid;
	}
	else
	{
        //printf("%s, line:%d\n", __FUNCTION__, __LINE__);
    
        ch_para.output_format = DMX_CHANNEL_OUTPUT_FORMAT_TS;
    
        ch_para.output_space = DMX_OUTPUT_SPACE_KERNEL;
    
        ch_para.ts_param.pid_list[0] = slot->pid;
		ch_para.ts_param.needdiscramble[0] = 0;
    
        ch_para.ts_param.pid_list_len = 1;
        
        ch_para.fe = porting->fe;
        ch_para.nim_chip_id = porting->nim_chip_id;
    
        //printf("%s, line:%d\n", __FUNCTION__, __LINE__);
    
        ts_kern_recv_info = &ch_para.ts_param.kern_recv_info;
    
        ret = ioctl(porting->see_dev_fd, ALI_DMX_SEE_GET_TS_INPUT_ROUTINE,
                    ts_kern_recv_info);
    
        if (ret < 0)
        {        
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
    
            dmx_avp_slot_stop(dmx, slot);
    
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
    
            return(RET_FAILURE);
        }
	}

    /* Step 3: Start dmx filter. */
    ret = ioctl(slot->linux_fd, ALI_DMX_CHANNEL_START, &ch_para);

    if (ret < 0)
    {        
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        dmx_avp_slot_stop(dmx, slot);

        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }

    ret = ioctl(slot->linux_fd, ALI_DMX_RESET_BITRATE_DETECT, 0);

    DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(RET_SUCCESS);
}




RET_CODE dmx_av_stream_set_pid
(
    struct dmx_device                *dmx,
    UINT32                            param,
    enum DMX_PORTING_AV_STREAM_VALID  a_v_valid
)
{
    struct io_param        *io_param;
    UINT16                 *pid_list;
    struct dmx_hld_porting *porting;
    UINT16                  v_pid;
    UINT16                  a_pid;
    UINT16                  p_pid;
    UINT16                  ad_pid;
    UINT16                  i,j;
    UINT16                  pid_num = 0;
    //DMX_DBG_API_PRINT("%s,%d,%s\n", __FUNCTION__, __LINE__, dmx->name);
    
	struct nim_device *nim_dev = NULL;

    if (NULL == dmx)
    {
        DMX_DBG_API_PRINT("%s, %d\n", __FUNCTION__, __LINE__);
  
        return(RET_FAILURE);
    }

    if (0 == param)
    {
        //DMX_DBG_API_PRINT("%s, %d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }

    porting = (struct dmx_hld_porting *)dmx->priv;

    io_param = (struct io_param *)param;


    porting->fe = io_param->buff_out_len;
    nim_dev = (struct nim_dev *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    if(nim_dev == NULL)
    {
        porting->nim_chip_id = 0xFFFF;
        
    }
    else
    {
        nim_ioctl_ext(nim_dev, NIM_DRIVER_GET_ID, (UINT32)&porting->nim_chip_id);
        porting->nim_chip_id = porting->nim_chip_id>>16;
    }
    /* Get A/V PIDs.*/

    pid_list = (UINT16 *)io_param->io_buff_in;
    pid_num = io_param->buff_in_len/sizeof(UINT16);
    DMX_DBG_API_PRINT("%s %d %d\r\n",__FUNCTION__,__LINE__,pid_num);

    for(i=0;i<pid_num;i++)
    {
        DMX_DBG_API_PRINT("%d\r\n",pid_list[i]&0x1FFF);
    }
        
    
    if (0 != (a_v_valid & DMX_PORTING_AV_V_VALID))
    {
        //DMX_DBG_API_PRINT("%s, %d\n", __FUNCTION__, __LINE__);

        v_pid = pid_list[0];
    }
    else
    {
       // DMX_DBG_API_PRINT("%s, %d\n", __FUNCTION__, __LINE__);

        v_pid = 0x1FFF;
    }

    if (0 != (a_v_valid & DMX_PORTING_AV_A_VALID))
    {
        a_pid = pid_list[1];
		porting->audio_disable = 0;
    }
    else
    {
        a_pid = 0x1FFF;
    }

#if 0
       if (a_pid != 0x1FFF && v_pid!= 0x1FFF)
       {
           p_pid = pid_list[2];
       }
       else
       {
           p_pid = 0x1FFF;
       }
#else
     p_pid = pid_list[2];
#endif


    if(0!=(a_v_valid & DMX_PORTING_AV_AD_VALID))
    {
        ad_pid = pid_list[3];
    }
    else
    {
        ad_pid = 0x1FFF;
    }


    dmx_avp_slot_set_pid(dmx, &(porting->v_slot), v_pid);

    dmx_avp_slot_set_pid(dmx, &(porting->a_slot), a_pid);

    dmx_avp_slot_set_pid(dmx, &(porting->p_slot), p_pid);

    dmx_avp_slot_set_pid(dmx, &(porting->ad_slot), ad_pid);

    if(pid_num>6)
    {
        for(i=6;i<pid_num;i++)
        {
            BOOL bSet=1;
            for(j=0;j<6;j++)
            {
                if(pid_list[i] == pid_list[j])
                {
                    bSet = 0;
                    break;
                }
            }
            if(bSet == 1 && pid_list[i]!= 0x1FFF)
            {
                dmx_avp_slot_set_pid(dmx, &(porting->other_slot[i]), pid_list[i]);
            }
        }
    }

   // DMX_DBG_API_PRINT("%s, %d, v:%d,a:%d,p:%d\n", __FUNCTION__, __LINE__,
         //  pid_list[0], pid_list[1], pid_list[2]);

    return(RET_SUCCESS);
}







/* Pause A/V stream. */
RET_CODE dmx_av_stream_stop
(
    struct dmx_device *dmx,
    UINT32             param
)
{
    struct dmx_hld_porting *porting;
    UINT16 i;

    DMX_DBG_API_PRINT("%s, %d %s\n", __FUNCTION__, __LINE__,dmx->name);

    porting = (struct dmx_hld_porting *)dmx->priv;

    if (DMX_HLD_PORTING_AV_STREAM_READY == porting->av_stream_status)
    {
        return(RET_SUCCESS);
    }

    dmx_avp_slot_stop(dmx, &(porting->v_slot));

    dmx_avp_slot_stop(dmx, &(porting->a_slot));

    dmx_avp_slot_stop(dmx, &(porting->p_slot));

    dmx_avp_slot_stop(dmx, &(porting->ad_slot));

    for(i=6;i<DMX_OTHER_PORTING_CNT;i++)
    {
        if(porting->other_slot[i].pid!=0x1FFF)
        {
            dmx_avp_slot_stop(dmx, &(porting->other_slot[i]));
            porting->other_slot[i].pid = 0x1FFF;
        }
    }
    //DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    dmx_see_av_stop(dmx, param);

    DMX_DBG_API_PRINT("%s,%d,%s\n", __FUNCTION__, __LINE__, dmx->name);

    g_dmx_playing_av = NULL;
    porting->av_stream_status = DMX_HLD_PORTING_AV_STREAM_READY;
    time_pcr = 0;
    return(RET_SUCCESS);
}


RET_CODE dmx_av_stream_start
(
    struct dmx_device *dmx,
    UINT32             param
)
{
    RET_CODE                ret;
    struct dmx_hld_porting *porting;
    struct dmx_see_av_pid_info  see_av_info;
    UINT16 i,j;
    DMX_DBG_API_PRINT("%s, %d\n", __FUNCTION__, __LINE__);

    porting = (struct dmx_hld_porting *)dmx->priv;

    if(g_dmx_playing_av != dmx && g_dmx_playing_av != NULL)
    {
        dmx_av_stream_stop(g_dmx_playing_av, 0);
    }
    if (DMX_HLD_PORTING_AV_STREAM_RUN == porting->av_stream_status)
    {
        DMX_DBG_API_PRINT("%s, %d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }

    if (DMX_PORTING_AV_V_VALID == (param & DMX_PORTING_AV_V_VALID))
    {
        /* Start injecting TS stream to SEE dmx. */
        dmx_avp_slot_start(dmx, &(porting->v_slot));
    }

#if 1
    if (DMX_PORTING_AV_A_VALID == (param & DMX_PORTING_AV_A_VALID))
    {
        /* Start injecting TS stream to SEE dmx. */
        dmx_avp_slot_start(dmx, &(porting->a_slot));
    }

    if (DMX_PORTING_AV_AD_VALID == (param & DMX_PORTING_AV_AD_VALID))
    {
        /* Start injecting TS stream to SEE dmx. */
        //DMX_DBG_API_PRINT("start ad slot\r\n");
        dmx_avp_slot_start(dmx, &(porting->ad_slot));
    }

    #if 0
    if ((DMX_PORTING_AV_V_VALID == (param & DMX_PORTING_AV_V_VALID))       &&
        (DMX_PORTING_AV_A_VALID == (param & DMX_PORTING_AV_A_VALID))       &&
        ((porting->p_slot.pid & 0x1FFF) != (porting->v_slot.pid & 0x1FFF)) &&
        ((porting->p_slot.pid & 0x1FFF) != (porting->a_slot.pid & 0x1FFF)))
    #else
    if ((DMX_PORTING_AV_V_VALID == (param & DMX_PORTING_AV_V_VALID)) &&
        (DMX_PORTING_AV_A_VALID == (param & DMX_PORTING_AV_A_VALID)))
    #endif
    {
        //printf("%s,%d,p pid:%d\n", __FUNCTION__, __LINE__, porting->p_slot.pid);
    
        dmx_avp_slot_start(dmx, &(porting->p_slot));
	}
#endif

    for(i=6;i<DMX_OTHER_PORTING_CNT;i++)
    {
        if(porting->other_slot[i].pid!=0x1FFF)
        {
            DMX_DBG_API_PRINT("%s %d %d %x\r\n",__FUNCTION__,__LINE__,i,porting->other_slot[i].pid);
            dmx_avp_slot_start(dmx, &(porting->other_slot[i]));
            DMX_DBG_API_PRINT("%s %d %d %x\r\n",__FUNCTION__,__LINE__,i,porting->other_slot[i].linux_fd);
        }
    }

    dmx_see_av_start(dmx);


    g_dmx_playing_av = dmx;
    porting->av_stream_status = DMX_HLD_PORTING_AV_STREAM_RUN;

   // DMX_DBG_API_PRINT("%s,%d,%s\n", __FUNCTION__, __LINE__, dmx->name);

    return(RET_SUCCESS);
}




/* Need special treament since video could not be closed. */
RET_CODE dmx_av_stream_change_a_pid
(
    struct dmx_device *dmx,
    UINT32             param
)
{
    INT32                            ret;
    struct dmx_hld_porting          *porting;
    struct dmx_hld_porting_avp_slot *slot;
    struct dmx_hld_porting_avp_slot *ad_slot;
    struct dmx_channel_param         ch_para;
    UINT16                          *pid_list;
    struct dmx_ts_kern_recv_info    *ts_kern_recv_info;
    struct dmx_see_av_pid_info      see_av_pid_info;
    UINT16                  i,j;
    UINT16                  pid_num = 0;
    struct io_param        *io_param;

    DMX_DBG_API_PRINT("%s, %d\n", __FUNCTION__, __LINE__);


    porting = (struct dmx_hld_porting *)dmx->priv;
    io_param = (struct io_param *)param;

	if(porting->av_stream_status == DMX_HLD_PORTING_AV_STREAM_READY)
	{
        return(RET_SUCCESS);
	}

	memset(&ch_para, 0, sizeof(struct dmx_channel_param));

    pid_list = io_param->io_buff_in;  
    pid_num =  io_param->buff_in_len/sizeof(UINT16);
    DMX_DBG_API_PRINT("%s %d %d\r\n",__FUNCTION__,__LINE__,pid_num);
    for(i=0;i<pid_num;i++)
    {
        DMX_DBG_API_PRINT("%d\r\n",pid_list[i]&0x1FFF);
    }

    slot = &(porting->a_slot);
    ad_slot = &(porting->ad_slot);

	DMX_DBG_API_PRINT("%s %d %d\r\n",__FUNCTION__,__LINE__,porting->status);
	DMX_DBG_API_PRINT("%s, %d slot->pid=%d ad_slot->pid=%d %s\n", __FUNCTION__, __LINE__,slot->pid&0x1FFF,ad_slot->pid&0x1FFF,dmx->name);
    if(slot->pid == pid_list[1] && ad_slot->pid == pid_list[3] )
        return(RET_SUCCESS);

        
    /* Step 1: Stop filter file. */
    if (slot->linux_fd >= 0)
    {
        ioctl(slot->linux_fd, ALI_DMX_CHANNEL_STOP, 0);
        close(slot->linux_fd);

        slot->linux_fd = -1;
    }

    if(ad_slot->linux_fd >= 0 && ad_slot->pid != pid_list[3])
    {
        ioctl(ad_slot->linux_fd, ALI_DMX_CHANNEL_STOP, 0);
        close(ad_slot->linux_fd);

        ad_slot->linux_fd = -1;
    }

    /* Step 2: Change SEE audio PID. */

    DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
    
    see_av_pid_info.a_pid = pid_list[1];
    see_av_pid_info.ad_pid = pid_list[3];
    
    ioctl(porting->see_dev_fd, ALI_DMX_SEE_A_CHANGE_PID, &see_av_pid_info);
    DMX_DBG_API_PRINT("a_pid=%d ad_pid=%d\r\n",see_av_pid_info.a_pid&0x1FFF,see_av_pid_info.ad_pid&0x1FFF);    
    /* Step 3: Open filter file with new PID. */

    slot->pid = pid_list[1];
    ad_slot->pid = pid_list[3];

    for(i=0;i<DMX_OTHER_PORTING_CNT;i++)
    {
        if(slot->pid == porting->other_slot[i].pid && slot->pid!= 0x1FFF)
        {
            DMX_DBG_API_PRINT("%d from other to front other_slot[i].pid=%d %d\r\n",i,porting->other_slot[i].pid&0x1FFF,__LINE__);    
            slot->linux_fd = porting->other_slot[i].linux_fd;
            slot->csa_slot_idx = porting->other_slot[i].csa_slot_idx;

            porting->other_slot[i].linux_fd = -1;
            porting->other_slot[i].pid = 0x1FFF;
        }
        
        if(ad_slot->pid == porting->other_slot[i].pid && ad_slot->pid!= 0x1FFF)
        {
            DMX_DBG_API_PRINT("%d from other to front other_slot[i].pid=%d %d\r\n",i,porting->other_slot[i].pid&0x1FFF,__LINE__);    
            ad_slot->linux_fd = porting->other_slot[i].linux_fd;
            ad_slot->csa_slot_idx = porting->other_slot[i].csa_slot_idx;
            
            porting->other_slot[i].linux_fd = -1;
            porting->other_slot[i].pid = 0x1FFF;
        }
    }

    if(slot->linux_fd < 0)
    {
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
        slot->linux_fd = open(porting->linux_dmx_path, O_RDONLY);
        if (slot->linux_fd < 0)
        {        
            dmx_avp_slot_stop(dmx, slot);
    
            DMX_DBG_API_PRINT("%s, line:%d, %s\n", __FUNCTION__, __LINE__,
                              porting->linux_dmx_path);
    
            return(RET_FAILURE);
        }    
    
    
        ch_para.output_format = DMX_CHANNEL_OUTPUT_FORMAT_TS;
    
        ch_para.output_space = DMX_OUTPUT_SPACE_KERNEL;
    
        ch_para.ts_param.pid_list[0] = slot->pid;
		ch_para.ts_param.needdiscramble[0] = 0;
		
        ch_para.fe = porting->fe;
        ch_para.nim_chip_id = porting->nim_chip_id;
    
        ch_para.ts_param.pid_list_len = 1;

        if(ad_slot->pid != 0x1FFF && ad_slot->linux_fd<0)
        {
            ad_slot->linux_fd = open(porting->linux_dmx_path, O_RDONLY);
            if (ad_slot->linux_fd < 0)
            {        
                dmx_avp_slot_stop(dmx, ad_slot);
    
                DMX_DBG_API_PRINT("%s, line:%d, %s\n", __FUNCTION__, __LINE__,
                                  porting->linux_dmx_path);
    
            }
            DMX_DBG_API_PRINT("ad_slot->pid=%d\r\n",ad_slot->pid&0x1FFF);
            ch_para.ts_param.pid_list[1] = ad_slot->pid;
			ch_para.ts_param.needdiscramble[1] = 0;
            ch_para.ts_param.pid_list_len = 2;
        }
    
        ts_kern_recv_info = &ch_para.ts_param.kern_recv_info;
    
        ret = ioctl(porting->see_dev_fd, ALI_DMX_SEE_GET_TS_INPUT_ROUTINE,
                    ts_kern_recv_info);
    
        if (ret < 0)
        {        
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
    
            dmx_avp_slot_stop(dmx, slot);
    
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
    
            return(RET_FAILURE);
        }
    
        ret = ioctl(slot->linux_fd, ALI_DMX_CHANNEL_START, &ch_para);
    
        if (ret < 0)
        {        
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
    
            dmx_avp_slot_stop(dmx, slot);
    
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
    
            return(RET_FAILURE);
        }
    }
    else
    {
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
        if(pid_num>6)
        {
                
            for(i=6;i<pid_num;i++)
            {   
                BOOL bset=1;
                for(j=0;j<6;j++)
                {
                    if(pid_list[i] == pid_list[j])
                    {
                        DMX_DBG_API_PRINT("%s, line:%d %d\n", __FUNCTION__, __LINE__,i);
                        bset = 0;
                    }
                }

                if(bset == 1 && pid_list[i]!=0x1FFF && porting->other_slot[i].linux_fd<0)
                {
                    DMX_DBG_API_PRINT("%s, line:%d %d\n", __FUNCTION__, __LINE__,i);
                    dmx_avp_slot_set_pid(dmx, &(porting->other_slot[i]), pid_list[i]);
                    dmx_avp_slot_start(dmx, &(porting->other_slot[i]));
                }
            }
        }
    }
    

    return(RET_SUCCESS);
}






















RET_CODE dmx_av_stream_timeshift
(
    struct dmx_device *dmx,
    UINT32             param
)
{
    struct dmx_hld_porting *porting;

    porting = (struct dmx_hld_porting *)dmx->priv;

    /* Quit timeshift. */
    if (0 == param)
    {

        DMX_DBG_API_PRINT("%s, line:%d %s\n", __FUNCTION__, __LINE__,dmx->name);
#if 1
        /* Restore A/V stream only if recover from A/V play. */
        if (DMX_HLD_PORTING_TIMESHIFT_FROM_PLAY == porting->timeshift_status)
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            dmx_av_stream_start(dmx, DMX_PORTING_AV_V_VALID |
                                     DMX_PORTING_AV_A_VALID |
                                     DMX_PORTING_AV_AD_VALID);
        }
#endif
        porting->timeshift_status = DMX_HLD_PORTING_TIMESHIFT_NONE;

        return(RET_SUCCESS);
    }

    /* Do timeshift(Stop A/V play temerarilly). */
    if (DMX_HLD_PORTING_AV_STREAM_RUN == porting->av_stream_status)
    {
        DMX_DBG_API_PRINT("%s, line:%d %s\n", __FUNCTION__, __LINE__,dmx->name);

        dmx_av_stream_stop(dmx, 0);

        porting->timeshift_status = DMX_HLD_PORTING_TIMESHIFT_FROM_PLAY;

        return(RET_SUCCESS);
    }

    //DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

   // porting->timeshift_status = DMX_HLD_PORTING_TIMESHIFT_FROM_IDLE;

    return(RET_SUCCESS);
}



RET_CODE dmx_av_stream_is_in_timeshift
(
    struct dmx_device *dmx,
    UINT32             param
)
{
#if 1
    struct dmx_hld_porting *porting;

    porting = (struct dmx_hld_porting *)dmx->priv;

    if (DMX_HLD_PORTING_TIMESHIFT_NONE == porting->timeshift_status)
    {
        return(RET_FAILURE);
    }

    return(RET_SUCCESS);
#else
    return(RET_FAILURE);
#endif
}


RET_CODE dmx_sec_stop
(
    struct dmx_device *dev,
    UINT32             id
)
{
    UINT32                           i;
    struct dmx_hld_porting          *porting;
    struct dmx_hld_porting_sec_slot *sec_slot;

    porting = (struct dmx_hld_porting *)dev->priv;

    for (i = 0; i < DMX_HLD_PORTING_SEC_SLOT_CNT; i++)
    {
        sec_slot = &(porting->sec_slot[i]);

        if ((DMX_HLD_PORTING_SEC_SLOT_IDLE != sec_slot->status) &&
            (sec_slot->id == id))
        {
            break;
        }
    }

    if (i >= DMX_HLD_PORTING_SEC_SLOT_CNT)
    {
        DMX_PRINTF("%s, 2\n", __FUNCTION__);

        return(RET_FAILURE); 
    }

    osal_flag_clear(porting->sec_hit_flag, 1 << id);

    //DMX_DBG_API_PRINT("%s,%d,idx:%d,pid:%d\n", __FUNCTION__, __LINE__, id, sec_slot->sec_para->pid);

    if (DMX_HLD_PORTING_SEC_SLOT_RUN == sec_slot->status)
    //if (DMX_HLD_PORTING_SEC_SLOT_IDLE != sec_slot->status)//gavin test
    {
        //DMX_DBG_API_PRINT("%s,%d,pid:%d,fd:%d\n", __FUNCTION__, __LINE__, 
               //sec_slot->sec_para->pid, sec_slot->linux_fd);
        ioctl(sec_slot->linux_fd, ALI_DMX_CHANNEL_STOP, 0);
        close(sec_slot->linux_fd);
        sec_slot->linux_fd = -1;
    }

    sec_slot->status = DMX_HLD_PORTING_SEC_SLOT_IDLE;
    if(sec_slot->csa_slot_idx != 0xffffffff)
    {
        porting->csa_slot[sec_slot->csa_slot_idx].status = ALI_CSA_SLOT_STATUS_IDLE;
    }    

    //DMX_DBG_API_PRINT("%s,%d,idx:%d,pid:%d\n", __FUNCTION__, __LINE__, id, sec_slot->sec_para->pid);

    return(RET_SUCCESS); 
}





RET_CODE dmx_sec_start
(
    struct dmx_device        *dev, 
    struct get_section_param *sec_para,
    UINT8                    *slot_id
)
{
    int                              dmx_sec_fd;
    struct dmx_channel_param         dmx_channel_param;
    int                              ret;
    struct dmx_hld_porting          *porting;
    struct dmx_hld_porting_sec_slot *sec_slot;
    UINT32                           i;

    /* TODO: Need mutex protection. */

    //DMX_DBG_API_PRINT("%s,%d,pid:%d\n", __FUNCTION__, __LINE__, sec_para->pid);

    porting = (struct dmx_hld_porting *)dev->priv;

    for (i = 0; i < DMX_HLD_PORTING_SEC_SLOT_CNT; i++)
    {
        sec_slot = &(porting->sec_slot[i]);

        if (DMX_HLD_PORTING_SEC_SLOT_IDLE == sec_slot->status)
        {
            break;
        }
    }

    if (i >= DMX_HLD_PORTING_SEC_SLOT_CNT)
    {
        DMX_PRINTF("No idle sec slot available!\n");

        return(RET_FAILURE); 
    }

	memset(&dmx_channel_param, 0, sizeof(struct dmx_channel_param));

    //DMX_DBG_API_PRINT("%s, line:%d, %s\n", __FUNCTION__, __LINE__, 
                      //porting->linux_dmx_path);
    //sec_slot->linux_fd = open("/dev/ali_m3602_dmx0", O_RDWR | O_NONBLOCK);
    sec_slot->linux_fd = open(porting->linux_dmx_path, O_RDONLY | O_NONBLOCK);
    
    if (sec_slot->linux_fd < 0)
    {        
        DMX_DBG_API_PRINT("%s, line:%d, %s\n", __FUNCTION__, __LINE__,
                          porting->linux_dmx_path);

        return(RET_FAILURE); 
    }
    sec_slot->csa_slot_idx = 0xffffffff;

    //DMX_DBG_API_PRINT("%s,%d,pid:%d,fd:%d\n", __FUNCTION__, __LINE__, 
           //sec_para->pid, sec_slot->linux_fd);
    

    /* Retrieve all sections of this PID, later parse them in
     * dmx_sec_retrieve_task().
     */
    dmx_channel_param.output_space = DMX_OUTPUT_SPACE_USER;
    dmx_channel_param.output_format = DMX_CHANNEL_OUTPUT_FORMAT_SEC;
    
    dmx_channel_param.sec_param.pid = sec_para->pid;
    dmx_channel_param.sec_param.mask_len = 1;
    dmx_channel_param.sec_param.mask[0] = 0;
    dmx_channel_param.sec_param.value[0] = 0;
    dmx_channel_param.sec_param.timeout = 30;
    
    dmx_channel_param.sec_param.option = 0;
    
    /* Start DMX channel. */
    ret = ioctl(sec_slot->linux_fd, ALI_DMX_CHANNEL_START, &dmx_channel_param);
    
    if (ret < 0)
    {        
        DMX_PRINTF("ioctl ALI_DMX_CHANNEL_START failed!\n");

        return(RET_FAILURE);
    }   

    sec_slot->sec_para = sec_para;

    *slot_id = sec_slot->id;

    sec_slot->status = DMX_HLD_PORTING_SEC_SLOT_RUN;

    if (0xC01 == sec_slot->sec_para->pid)
    {
        DMX_DBG_API_PRINT("%s,%d,idx:%d,pid:0x%x\n", __FUNCTION__, __LINE__, sec_slot->id, sec_slot->sec_para->pid);
    }

    //DMX_DBG_API_PRINT("%s,%d,idx:%d\n", __FUNCTION__, __LINE__, sec_slot->id);

    return(RET_SUCCESS); 
}












RET_CODE dmx_async_sec_close_multi
(
    struct dmx_device *dev, 
    UINT32             mask
)
{
    INT32 i;

    for(i = 0; i < DMX_HLD_PORTING_SEC_SLOT_CNT; i++)
    {
        if (mask & (1 << i)) 
        {
            dmx_sec_stop(dev, i);
        }
    }

    return RET_SUCCESS;
}



/* Internal use, IOCTL command. */
RET_CODE dmx_async_sec_poll
(
    struct dmx_device *dev,
    UINT32             param
)
{
    UINT32 mask    = ((UINT32 *)param)[0];
    UINT32 timeout = ((UINT32 *)param)[1];
    UINT32 flag    = 0;

    struct dmx_hld_porting *porting;

    porting = (struct dmx_hld_porting *)dev->priv;

#if 0
    if(OSAL_E_OK == osal_flag_wait(&flag, porting->sec_hit_flag,
                                   mask | DMX_CMD_WAKEUP_SIAE, 
                                   OSAL_TWF_ORW | OSAL_TWF_CLR, timeout))
#else
    //timeout = OSAL_WAIT_FOREVER_TIME;
    //timeout = timeout * 3;

    if(OSAL_E_OK == osal_flag_wait(&flag, porting->sec_hit_flag,
                                   mask | DMX_CMD_WAKEUP_SIAE, 
                                   OSAL_TWF_ORW, timeout))
#endif
    {
        /* Pull succeed. */
        return (flag & mask);
    }

    /* Timeout. */
    return(0);
}

/* Internal use, IOCTL command. */
RET_CODE dmx_async_sec_poll_release_all
(
    struct dmx_device *dev,
    UINT32             param
)
{
    struct dmx_hld_porting *porting;

    porting = (struct dmx_hld_porting *)dev->priv;
 
    if (0 == param)
    {
        osal_flag_clear(porting->sec_hit_flag, DMX_CMD_WAKEUP_SIAE);

        return(RET_SUCCESS);
    }

    osal_flag_set(porting->sec_hit_flag, DMX_CMD_WAKEUP_SIAE);

    return(RET_SUCCESS);
}



RET_CODE dmx_async_sec_close
(
    struct dmx_device *dev,
    UINT32             id
)
{
    RET_CODE                ret;
    struct dmx_hld_porting *porting;

    porting = (struct dmx_hld_porting *)dev->priv;

	osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

    ret = dmx_sec_stop(dev, id);

    osal_mutex_unlock(porting->ioctl_mutex);

    return(ret);
}



RET_CODE dmx_async_req_section
(
    struct dmx_device        *dev, 
    struct get_section_param *sec_para,
    UINT8                    *slot_id
)
{
    RET_CODE                ret;
    struct dmx_hld_porting *porting;

    porting = (struct dmx_hld_porting *)dev->priv;

	osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

    ret = dmx_sec_start(dev, sec_para, slot_id);

    osal_mutex_unlock(porting->ioctl_mutex);

    return(ret);
}




RET_CODE dmx_req_section
(
    struct dmx_device        *dev,
    struct get_section_param *sec_para
)
{
    UINT8                   slot_id;
    UINT32                  wait_delay;
    UINT32                  flg_got;
    RET_CODE                ret;
    struct dmx_hld_porting *porting;

    DMX_PRINTF("%s,%d,pid:%d\n", __FUNCTION__, __LINE__, sec_para->pid);

    wait_delay = 0;

    flg_got = 0;

    /* TODO: Need mutex protection. */

    porting = (struct dmx_hld_porting *)dev->priv;

	osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

    ret = dmx_sec_start(dev, sec_para, &slot_id);
    osal_mutex_unlock(porting->ioctl_mutex);

	if (ret !=  RET_SUCCESS)
    {

        return(RET_FAILURE);
    }

    if(sec_para->wai_flg_dly != 0)
    {
        wait_delay = sec_para->wai_flg_dly;
    }
    else
    {
        wait_delay = DMX_HLD_PORTING_SEC_REQ_DEFAULT_DELAY;
    }

#if 0
    ret = osal_flag_wait(&flg_got, 
                         porting->sec_hit_flag, 
                         (1 << slot_id) | DMX_CMD_STOP_GET_SEC, 
                         OSAL_TWF_ORW | OSAL_TWF_CLR, wait_delay);
#else
    ret = osal_flag_wait(&flg_got, 
                         porting->sec_hit_flag, 
                         (1 << slot_id) | DMX_CMD_STOP_GET_SEC, 
                         OSAL_TWF_ORW | OSAL_TWF_CLR, wait_delay);
#endif

	osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);
    dmx_sec_stop(dev, slot_id);
    osal_mutex_unlock(porting->ioctl_mutex);
    if(OSAL_E_OK != ret)
    {
        ret = !RET_SUCCESS;
    }
    else if(flg_got & DMX_CMD_STOP_GET_SEC)
    {
        ret = !RET_SUCCESS;
    }
    else if(!(flg_got & (1 << slot_id)))
    {
        ret = !RET_SUCCESS;
    }


    DMX_PRINTF("%s,%d,pid:%d\n", __FUNCTION__, __LINE__, sec_para->pid);

	return ret;
}





/* Internal use, not HLD interface. */
RET_CODE dmx_req_section_release_all
(
    struct dmx_device *dev,
    UINT32             param
)
{
    struct dmx_hld_porting *porting;

    porting = (struct dmx_hld_porting *)dev->priv;

    osal_flag_set(porting->sec_hit_flag, DMX_CMD_STOP_GET_SEC);

    return(RET_SUCCESS);
}




RET_CODE dmx_sec_hit
(
    struct restrict *mask_value,
    UINT8           *buf,
    UINT16          *hit_num
)
{
    UINT8 i,j ;
    
    UINT8*mask ;
    UINT8*value ;
    UINT8 result1 ;
    UINT8 result2=0 ;
    UINT8 mask_len ;
    UINT8 value_num ;
    UINT16 sec_hit_event = 0;
    UINT16 tb_flt_msk ;

    if (NULL == mask_value)
    {
        return RET_SUCCESS;
    }

    mask_len = mask_value->mask_len;

    if(MAX_SEC_MASK_LEN<mask_len)
    {
        DMX_PRINTF("ERR: msk len > MAX_SEC_MASK_LEN\n");

        return!RET_SUCCESS ;
    }

    tb_flt_msk = mask_value->tb_flt_msk;

    if(tb_flt_msk)
    {
        if(0xFF00 != tb_flt_msk)
        {
            for(i=0;i<MAX_MV_NUM;i++)
            {
                if((tb_flt_msk>>i)&0x1)
                {
                    value = mask_value->value[i];
                    mask = mask_value->multi_mask[i];
                    result1=1;

                    for(j=0;j<mask_len;j++)
                    {
                        if(((mask[j]&buf[j])^value[j])!=0)
                        {   
                            result1=0;

                            break;
                        }
                    }

                    if(1==result1)
                    {   
                        result2|=1;

                        sec_hit_event |= (1<<i);
                    }
                 }
              }
          }
          else
          {
              UINT32 idx;
              UINT8  value_cmp;
              UINT8  mask_cmp;
              UINT32 hit_cnt = 0;
  
              struct restrict_ext *sec_filters;
  
              sec_filters = (struct restrict_ext *)mask_value;
  
              for(i = 0; i < sec_filters->multi_mask_num_ext; i++)
              {
                  if (0 == sec_filters->multi_mask_en_ext[i])
                  {
                      continue;
                  }
  
                  for (j = 0; j < sec_filters->multi_mask_len_ext; j++)
                  {
                      idx = i * sec_filters->multi_mask_len_ext + j;
  
                      value_cmp = sec_filters->multi_value_ext[idx];
  
                      mask_cmp = sec_filters->multi_mask_ext[idx];
  
                      if((value_cmp & mask_cmp) != (buf[j] & mask_cmp))
                      {   
                          break;
                      }
                  }
  
                  if (j < sec_filters->multi_mask_len_ext)
                  {
                      sec_filters->multi_mask_hit_ext[i] = 0;
                  }
                  else
                  {
                      sec_filters->multi_mask_hit_ext[i] = 1;
  
                      hit_cnt++;                  
                  }
              }
  
              if (0 == hit_cnt)
              {
                  return (!RET_SUCCESS);
              }
              else
              {
                  return(RET_SUCCESS);
              }
          }
    }
    else
    {
        mask=mask_value->mask;
        value_num=mask_value->value_num;

        if(value_num>MAX_MV_NUM)
        {
            DMX_PRINTF("val num>MAX_MV_NUM\n");

            return !RET_SUCCESS;
        }

        for(i = 0; i< value_num; i++)
        {
            value = &(mask_value->value[i][0]);
            result1 = 1;

            for(j = 0; j< mask_len; j++)
            {
                if(((mask[j]&buf[j])^value[j])!=0)
                {
                    result1 = 0 ;
                    break;
                }
            }

            if(1==result1)
            {
                result2|=1 ;
                sec_hit_event |= (1<<i);
                //break;
            }
        }
    }
    
    if(1==result2)
    {
        //*hit_num = (UINT16)i;
        *hit_num = sec_hit_event;

        return RET_SUCCESS ;
    }
    
    return!RET_SUCCESS ;
}



UINT16 dmx_sec_mask_first_hit
(
    UINT16 hit_mask
)
{
    UINT32 cnt;

    for(cnt = 0; cnt < MAX_MV_NUM; cnt++)
    {
        if(hit_mask & (1 << cnt))
        {
            return(cnt);
        }
    }

    return(0xFFFF);
}


#define SEC_BUFFER_LEN (1024*4)
static UINT8 tmpSecBuffer[SEC_BUFFER_LEN];

RET_CODE dmx_sec_dispath
(
    struct dmx_device               *dmx,
    struct dmx_hld_porting_sec_slot *sec_slot
)
{
#if 0
    UINT16 cnt;
    struct restrict_ext *sec_filters;
    struct get_section_param *sec_para;
    UINT16 hit_mask;
    RET_CODE is_sec_hit;
    INT32    bytes;
    struct dmx_hld_porting *porting;

    //DMX_PRINTF("dmx_sec_dispath go!\n");

    sec_para = sec_slot->sec_para;

    bytes = read(sec_slot->linux_fd, sec_para->buff, sec_para->buff_len);

    //DMX_PRINTF("task read len:%d\n", bytes);

    if (bytes <= 0)
    {
        return(RET_FAILURE);
    }

    //DMX_DBG_API_PRINT("%s,%d,pid:%d,len:%d\n", __FUNCTION__, __LINE__, sec_para->pid, bytes);

    sec_para->sec_tbl_len = bytes;

    is_sec_hit = dmx_sec_hit(sec_para->mask_value, sec_para->buff, &hit_mask);

    DMX_PRINTF("is_sec_hit:%d\n", is_sec_hit);

    if (is_sec_hit != RET_SUCCESS)
    {
        return(RET_FAILURE);
    }

    //DMX_DBG_API_PRINT("%s,%d,pid:%d,len:%d\n", __FUNCTION__, __LINE__, sec_para->pid, bytes);

    /* For one-shot. */
    if ((0 == sec_para->continue_get_sec) || (NULL == sec_para->get_sec_cb))
    {
        porting = (struct dmx_hld_porting *)dmx->priv;

        sec_para->sec_hit_num = dmx_sec_mask_first_hit(hit_mask);

        //DMX_DBG_API_PRINT("%s,%d,pid:%d,len:%d\n", __FUNCTION__, __LINE__, sec_para->pid, bytes);

        osal_flag_set(porting->sec_hit_flag, 1 << sec_slot->id);

        /* Emuliate TDS driver behavior, release this sec service if section has 
         * been got.
         */
        //dmx_sec_stop(dmx, sec_slot->id);
        sec_slot->status = DMX_HLD_PORTING_SEC_SLOT_PAUSE;

        //DMX_DBG_API_PRINT("%s,%d,pid:%d,fd:%d\n", __FUNCTION__, __LINE__, 
               //sec_para->pid, sec_slot->linux_fd);
        ioctl(sec_slot->linux_fd, ALI_DMX_CHANNEL_STOP, 0);
        close(sec_slot->linux_fd);

        //DMX_DBG_API_PRINT("%s,%d,pid:%d,fd:%d\n", __FUNCTION__, __LINE__, 
               //sec_para->pid, sec_slot->linux_fd);

        return(RET_SUCCESS);
    }

    DMX_PRINTF("mask_value:0x%x\n", sec_para->mask_value);

    /* For non-mask. */
    if(NULL == sec_para->mask_value)
    {
        DMX_PRINTF("get_sec_cb:1\n");

        sec_para->sec_hit_num = dmx_sec_mask_first_hit(hit_mask);

        //DMX_DBG_API_PRINT("%s,%d,pid:%d,len:%d\n", __FUNCTION__, __LINE__, sec_para->pid, bytes);

        sec_para->get_sec_cb(sec_para);

        return(RET_SUCCESS);
    }

    /* For muti-mask-multi-value and one-mask-multi-value. */
    if (0xFF00 != sec_para->mask_value->tb_flt_msk)
    {
        DMX_PRINTF("hit_mask:0x%x\n", hit_mask);

        for(cnt = 0; cnt < MAX_MV_NUM; cnt++)
        {
            if(hit_mask & (1 << cnt))
            {
                sec_para->sec_hit_num = cnt;

                //DMX_DBG_API_PRINT("%s,%d,pid:%d,len:%d\n", __FUNCTION__, __LINE__, sec_para->pid, bytes);

                sec_para->get_sec_cb(sec_para);
            }
        }

        return(RET_SUCCESS);
    }

    /* For extended multi-mask. */
    sec_filters = (struct restrict_ext *)sec_para->mask_value;

    for(cnt = 0; cnt < sec_filters->multi_mask_num_ext; cnt++)
    {
        if(1 == sec_filters->multi_mask_hit_ext[cnt])
        {
            sec_para->sec_hit_num = cnt;

            //DMX_DBG_API_PRINT("%s,%d,pid:%d,len:%d\n", __FUNCTION__, __LINE__, sec_para->pid, bytes);

            sec_para->get_sec_cb(sec_para);
        }
    }

    return(RET_SUCCESS);
#else
    UINT16 cnt;
    struct restrict_ext      *sec_filters;
    struct get_section_param *sec_para;
    UINT16                    hit_mask;
    RET_CODE                  is_sec_hit;
    INT32                     bytes;
    struct dmx_hld_porting   *porting;
    UINT32                    Loop = 0;

	if (NULL == dmx || NULL == dmx->priv || \
		NULL == sec_slot || NULL == sec_slot->sec_para)
		return !SUCCESS;

    //DMX_PRINTF("dmx_sec_dispath go!\n");
    porting = (struct dmx_hld_porting *)dmx->priv;
    
    sec_para = sec_slot->sec_para;

    /* Read until no more section in this filter. */
    for (;;)
    {
        Loop++;

        memset(tmpSecBuffer, 0, SEC_BUFFER_LEN);
        bytes = read(sec_slot->linux_fd, tmpSecBuffer, SEC_BUFFER_LEN);
        
    
        //DMX_PRINTF("task read len:%d\n", bytes);
    
        if (bytes <= 0)
        {
            //if (Loop > 1)
            //{
                //printf("PID:%d,Loop:%d\n", sec_para->pid, Loop);
            //}

            return(RET_FAILURE);
        }
    
        //DMX_DBG_API_PRINT("%s,%d,pid:%d,len:%d\n", __FUNCTION__, __LINE__, sec_para->pid, bytes);

    
        is_sec_hit = dmx_sec_hit(sec_para->mask_value, tmpSecBuffer, &hit_mask);
    
        DMX_PRINTF("is_sec_hit:%d\n", is_sec_hit);
    
        if (is_sec_hit != RET_SUCCESS)
        {
            //return(RET_FAILURE);
            continue;
        }
    

        if(sec_para->buff_len >= bytes)
        {
            memcpy(sec_para->buff,tmpSecBuffer,bytes);
   			sec_para->sec_tbl_len = bytes;
       }
        //DMX_DBG_API_PRINT("%s,%d,pid:%d,len:%d\n", __FUNCTION__, __LINE__, sec_para->pid, bytes);
    
        /* For one-shot. */
        if ((0 == sec_para->continue_get_sec) || (NULL == sec_para->get_sec_cb))
        {
    
            sec_para->sec_hit_num = dmx_sec_mask_first_hit(hit_mask);
    
            //DMX_DBG_API_PRINT("%s,%d,pid:%d,len:%d\n", __FUNCTION__, __LINE__, sec_para->pid, bytes);
    
            osal_flag_set(porting->sec_hit_flag, 1 << sec_slot->id);
    
            /* Emuliate TDS driver behavior, release this sec service if section has 
             * been got.
             */
            
            sec_slot->status = DMX_HLD_PORTING_SEC_SLOT_PAUSE;
    
            //DMX_DBG_API_PRINT("%s,%d,pid:%d,fd:%d\n", __FUNCTION__, __LINE__, 
                   //sec_para->pid, sec_slot->linux_fd);
            ioctl(sec_slot->linux_fd, ALI_DMX_CHANNEL_STOP, 0);
            close(sec_slot->linux_fd);
			sec_slot->linux_fd = -1;
            
            
            //DMX_DBG_API_PRINT("%s,%d,pid:%d,fd:%d\n", __FUNCTION__, __LINE__, 
                   //sec_para->pid, sec_slot->linux_fd);
    
            return(RET_SUCCESS);
        }
    
        DMX_PRINTF("mask_value:0x%x\n", sec_para->mask_value);
    
        /* For non-mask. */
        if(NULL == sec_para->mask_value)
        {
            DMX_PRINTF("get_sec_cb:1\n");
    
            sec_para->sec_hit_num = dmx_sec_mask_first_hit(hit_mask);
    
            //DMX_DBG_API_PRINT("%s,%d,pid:%d,len:%d\n", __FUNCTION__, __LINE__, sec_para->pid, bytes);
            osal_mutex_unlock(porting->ioctl_mutex);
			if(sec_para->get_sec_cb)
			{
            	sec_para->get_sec_cb(sec_para);
			}
            osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);
            
            //return(RET_SUCCESS);
            continue;
        }
    
        /* For muti-mask-multi-value and one-mask-multi-value. */
        if (0xFF00 != sec_para->mask_value->tb_flt_msk)
        {
            DMX_PRINTF("hit_mask:0x%x\n", hit_mask);
    
            for(cnt = 0; cnt < MAX_MV_NUM; cnt++)
            {
                if(hit_mask & (1 << cnt))
                {
                    sec_para->sec_hit_num = cnt;
    
                    //DMX_DBG_API_PRINT("%s,%d,pid:%d,len:%d\n", __FUNCTION__, __LINE__, sec_para->pid, bytes);
                    osal_mutex_unlock(porting->ioctl_mutex);
					if(sec_para->get_sec_cb)
					{
                    	sec_para->get_sec_cb(sec_para);
					}
                    osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);
                }
            }

			if (0 == sec_para->continue_get_sec)
			{
			
				sec_para->sec_hit_num = dmx_sec_mask_first_hit(hit_mask);
			
				//DMX_PRINTF("%s,%d,pid:%d,len:%d\n", __FUNCTION__, __LINE__, sec_para->pid, bytes);
			
				osal_flag_set(porting->sec_hit_flag, 1 << sec_slot->id);
			
				/* Emuliate TDS driver behavior, release this sec service if section has 
				 * been got.
				 */
				
				sec_slot->status = DMX_HLD_PORTING_SEC_SLOT_PAUSE;
			
				//DMX_PRINTF("%s,%d,pid:%d,fd:%d\n", __FUNCTION__, __LINE__, 
					   //sec_para->pid, sec_slot->linux_fd);
				ioctl(sec_slot->linux_fd, ALI_DMX_CHANNEL_STOP, 0);
				close(sec_slot->linux_fd);
				sec_slot->linux_fd = -1;
				
				
				//DMX_PRINTF("%s,%d,pid:%d,fd:%d\n", __FUNCTION__, __LINE__, 
					   //sec_para->pid, sec_slot->linux_fd);
			
				return(RET_SUCCESS);
			}

            //return(RET_SUCCESS);
            continue;
        }
    
        /* For extended multi-mask. */
        sec_filters = (struct restrict_ext *)sec_para->mask_value;
    
        for(cnt = 0; cnt < sec_filters->multi_mask_num_ext; cnt++)
        {
            if(1 == sec_filters->multi_mask_hit_ext[cnt])
            {
                sec_para->sec_hit_num = cnt;
    
                //DMX_DBG_API_PRINT("%s,%d,pid:%d,len:%d\n", __FUNCTION__, __LINE__, sec_para->pid, bytes);
                osal_mutex_unlock(porting->ioctl_mutex);
                if(sec_para->get_sec_cb)
                {
                	sec_para->get_sec_cb(sec_para);
                }
                osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);
            }
        }
    }

    return(RET_SUCCESS);
#endif
}



//__u32 LastTickAfterProc = 0;

void dmx_sec_retrieve_task
(
    UINT32 para1,
    UINT32 para2
)
{
    __u32                            i;
    __u32                            j;
    struct dmx_device               *dmx;
    struct dmx_hld_porting          *porting;
    struct dmx_hld_porting_sec_slot *sec_slot;
    struct pollfd                    sec_fd[DMX_HLD_PORTING_SEC_SLOT_CNT];
    __u32                            sec_fd2idx[DMX_HLD_PORTING_SEC_SLOT_CNT];
    //__u32                            TickBeforeProc;
    //__u32                            TickAfterProc;
    struct statfs stbuf;

    DMX_PRINTF("dmx_sec_retrieve_task() run!\n");

    dmx = (struct dmx_device *)para1;

    porting = (struct dmx_hld_porting *)dmx->priv;

    DMX_PRINTF("dmx_sec_retrieve_task() dmx name:%s\n", dmx->name);

    //DMX_DBG_API_PRINT("\n\n%s,%d,pid:%d\n\n", __FUNCTION__, __LINE__, syscall(__NR_gettid));

    for (;;)
    {
        if(porting->status != DMX_HLD_PORTING_STATUS_RUN)
        {
            if(porting->status == DMX_HLD_PORTING_STATUS_STOP)
                porting->status = DMX_HLD_PORTING_STATUS_STOP_NEXT;
            osal_task_sleep(20);
            continue;
        }
        osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);
        j = 0;
        
        for (i = 0; i < DMX_HLD_PORTING_SEC_SLOT_CNT; i++)
        {
            sec_slot = &(porting->sec_slot[i]);

            if (DMX_HLD_PORTING_SEC_SLOT_RUN == sec_slot->status)
            {
                if(fstatfs(sec_slot->linux_fd, &stbuf) != 0)
                {
//                    printf("invalid fd: \n");
                    continue;
                }
                sec_fd[j].fd = sec_slot->linux_fd;

                sec_fd[j].events = POLLIN;

                sec_fd2idx[j] = i;

                j++;
            }
        }
        
        //DMX_DBG_API_PRINT("%s,%d,n:%d\n", __FUNCTION__, __LINE__, n);
        
        if (poll(sec_fd, j, 10) > 0) 
        {
            //TickBeforeProc = osal_get_tick();

            //printf("TickBeforeProc:%d,Wait gap:%d\n", TickBeforeProc, TickBeforeProc - LastTickAfterProc);

            for (i = 0; i < j; i++)
            {
                if (sec_fd[i].revents)
                {
                    sec_slot = &(porting->sec_slot[sec_fd2idx[i]]);
                    //osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);
                    dmx_sec_dispath(dmx, sec_slot);
                    //osal_mutex_unlock(porting->ioctl_mutex);
                }
            }

            //TickAfterProc = osal_get_tick();

            //LastTickAfterProc = TickAfterProc;

            //printf("Tick:%d,Proc time:%d\n", TickAfterProc, TickAfterProc - TickBeforeProc);
        }
        osal_mutex_unlock(porting->ioctl_mutex);
        osal_task_sleep(5);    
        //DMX_PRINTF("dmx_sec_retrieve_task() sleep 10!\n");
    }

    return;
}

RET_CODE dmx_pes_start
(
    struct dmx_device        *dev, 
    struct get_pes_param     *pes_para,
    UINT8                    *slot_id
)
{
    int                              dmx_pes_fd;
    struct dmx_channel_param         dmx_channel_param;
    int                              ret;
    struct dmx_hld_porting          *porting;
    struct dmx_hld_porting_pes_slot *pes_slot;
    UINT32                           i;

    /* TODO: Need mutex protection. */
    DMX_DBG_API_PRINT("%s,L[%d],pid:%d\n", __FUNCTION__, __LINE__, pes_para->pid);

    porting = (struct dmx_hld_porting *)dev->priv;

    for (i = 0; i < DMX_HLD_PORTING_PES_SLOT_CNT; i++)
    {
        pes_slot = &(porting->pes_slot[i]);

        if (DMX_HLD_PORTING_PES_SLOT_IDLE == pes_slot->status)
        {
            break;
        }
    }

    if (i >= DMX_HLD_PORTING_PES_SLOT_CNT)
    {
        DMX_DBG_API_PRINT("%s():No idle pes slot available!\n", __FUNCTION__);

        return(RET_FAILURE); 
    }

    DMX_DBG_API_PRINT("%s(), L[%d], %s\n", __FUNCTION__, __LINE__, 
                      porting->linux_dmx_path);

	memset(&dmx_channel_param, 0, sizeof(struct dmx_channel_param));

    pes_slot->linux_fd = open(porting->linux_dmx_path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

    if (pes_slot->linux_fd < 0)
    {        
        DMX_DBG_API_PRINT("%s, line:%d, %s\n", __FUNCTION__, __LINE__,
                          porting->linux_dmx_path);
	
    	pes_slot->stat_info.IoOpenFailCnt++; 

        return(RET_FAILURE); 
    }
    DMX_DBG_API_PRINT("%s(),L[%d],pid:%d,fd:%d\n", __FUNCTION__, __LINE__, 
           				pes_para->pid, pes_slot->linux_fd);
    

    /* Retrieve all PES of this PID, later parse them in
     * dmx_pes_retrieve_task().
     */
    dmx_channel_param.output_space = DMX_OUTPUT_SPACE_USER;
    dmx_channel_param.output_format = DMX_CHANNEL_OUTPUT_FORMAT_PES;
    
    dmx_channel_param.pes_param.pid = pes_para->pid;
    dmx_channel_param.pes_param.timeout = 30;

	DMX_DBG_API_PRINT("%s():L[%d] \n", __FUNCTION__, __LINE__);
    
    /* Start DMX channel. */
    ret = ioctl(pes_slot->linux_fd, ALI_DMX_CHANNEL_START, &dmx_channel_param);    
    if (ret < 0)
    {        
        DMX_DBG_API_PRINT("%s(): L[%d],ioctl ALI_DMX_CHANNEL_START failed!\n", \
            		__FUNCTION__, __LINE__);

    	pes_slot->stat_info.IoStartFailCnt++; 

        return(RET_FAILURE);
    }   

    //pes_slot->pes_para = pes_para;
	pes_slot->pes_req_buf = pes_para->request;
    pes_slot->pes_ret_buf = pes_para->update;
    pes_slot->pes_hdl = (UINT32)(pes_para->dev); 
    pes_slot->filter_id = pes_para->filter_id;  //Value from AP to comprent with the TDS codes
    pes_slot->pid = pes_para->pid;


    DMX_DBG_API_PRINT("%s(): L[%d],PES_Slot info: \n",__FUNCTION__, __LINE__);
    DMX_DBG_API_PRINT("  req_buf[0x%x],ret_buf[0x%x]\n",pes_slot->pes_req_buf, pes_slot->pes_ret_buf);
    DMX_DBG_API_PRINT("  hdl[0x%x],linux_fd[0x%x] \n",pes_slot->pes_hdl, pes_slot->linux_fd);
    DMX_DBG_API_PRINT("  filter_id[0x%x],pid[0x%x] \n",pes_slot->filter_id, pes_slot->pid);

    DMX_DBG_API_PRINT("  &buf[0x%x],buf[0x%x] \n",&(pes_slot->buf), pes_slot->buf);
    DMX_DBG_API_PRINT("  &buf_len[0x%x],buf_len[0x%x] \n",&(pes_slot->buf_len), pes_slot->buf_len);
    DMX_DBG_API_PRINT("  &buf_wr[0x%x],buf_wr[0x%x] \n",&(pes_slot->buf_wr), pes_slot->buf_wr);
    DMX_DBG_API_PRINT("  &pSDataCtrlBlk[0x%x] \n \n \n",&(pes_slot->pSDataCtrlBlk));

    *slot_id = pes_slot->filter_id;  //i; 

    pes_slot->status = DMX_HLD_PORTING_PES_SLOT_RUN;

    DMX_DBG_API_PRINT("%s(),L[%d],filter_idx:%d\n", __FUNCTION__, __LINE__, pes_slot->filter_id);

    return(RET_SUCCESS); 
}


RET_CODE dmx_pes_stop
(
    struct dmx_device *dev,
    UINT32             id
)
{
    UINT32                           i;
    struct dmx_hld_porting          *porting;
    struct dmx_hld_porting_pes_slot *pes_slot;

    DMX_DBG_API_PRINT("%s(),L[%d], Start! \n", __FUNCTION__, __LINE__);

    porting = (struct dmx_hld_porting *)dev->priv;

    for (i = 0; i < DMX_HLD_PORTING_PES_SLOT_CNT; i++)
    {
        pes_slot = &(porting->pes_slot[i]);

        if ((DMX_HLD_PORTING_PES_SLOT_IDLE != pes_slot->status) &&
            (pes_slot->filter_id == id))
        {
            break;
        }
    }

    if (i >= DMX_HLD_PORTING_PES_SLOT_CNT)
    {
        DMX_DBG_API_PRINT("%s,L[%d],id:%d\n", __FUNCTION__, __LINE__, id);

        return(RET_FAILURE); 
    }

    
    DMX_DBG_API_PRINT("%s():L[%d],filter_idx[%d],pid:%d\n", \
    			__FUNCTION__, __LINE__, id, pes_slot->pid);

    if (DMX_HLD_PORTING_PES_SLOT_RUN == pes_slot->status)
    {
        DMX_DBG_API_PRINT("%s(),L[%d],pid:%d,fd:%d\n", __FUNCTION__, __LINE__, \
               				pes_slot->pid, pes_slot->linux_fd);
        ioctl(pes_slot->linux_fd, ALI_DMX_CHANNEL_STOP, 0);
        close(pes_slot->linux_fd);

        pes_slot->linux_fd = -1;
    }

    if (DMX_HLD_PORTING_PES_SLOT_PAUSE == pes_slot->status)
    {
        DMX_DBG_API_PRINT("%s(),L[%d],idx:%d,pid:%d\n",\
            				__FUNCTION__, __LINE__,id, pes_slot->pid);
    }

    pes_slot->status = DMX_HLD_PORTING_PES_SLOT_IDLE;

    DMX_DBG_API_PRINT("%s(),L[%d],idx:%d,pid:%d\n", \
    					__FUNCTION__, __LINE__, id, pes_slot->pid);

    return(RET_SUCCESS); 
}




RET_CODE dmx_req_pes
(
    struct dmx_device        *dev, 
    struct get_pes_param     *pes_para,
    UINT8                    *slot_id
)
{
    RET_CODE                ret;
    struct dmx_hld_porting *porting;

    porting = (struct dmx_hld_porting *)dev->priv;

	//osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);
	//dmx_mutex_lock(dev);

    ret = dmx_pes_start(dev, pes_para, slot_id);

    if( RET_SUCCESS != ret )
	{
		DMX_DBG_API_PRINT("%s(),L[%d], dmx_pes_start() fail! \n", __FUNCTION__, __LINE__);
    }

    //osal_mutex_unlock(porting->ioctl_mutex);
	//dmx_mutex_unlock(dev);

    return(ret);
}



enum STREAM_TYPE dmx_pes_stream_type(UINT8 stream_id)
{
    if((stream_id&0xe0)==0xc0)
        return AUDIO_STR ;
    else if((stream_id&0xf0)==0xe0)
        return VIDEO_STR ;
    else if((0xf9<stream_id)&&(stream_id<0xff))
        return REV_DATA_STR ;
    else if((0xbb<stream_id)&&(stream_id<0xc0))
        return (enum STREAM_TYPE)((UINT32)PRG_STR_MAP+stream_id-0xbc);
    else if((0xef<stream_id)&&(stream_id<0xfa))
        return (enum STREAM_TYPE)((UINT32)ECM_STR+stream_id-0xf0);
    else if(0xff == stream_id)
        return PRG_STR_DIR;
    else
        return UNKNOW_STR;
}


RET_CODE dmx_pes_get_head(UINT8 *buf,UINT32 uDataSize, UINT32 *payloadPos,struct control_block *ctrl_blk)
{	
    UINT8   pts_dts_flag = 0;
    UINT32	pts_value = 0;
    UINT32 	temp_stc, i;
    UINT8	stream_id =0, pes_pkt_head_len = 0;
    UINT16	pes_pkt_len =0;
    enum STREAM_TYPE str_type = 0;
    

	if((NULL == buf)||(uDataSize ==0 ))
	{
		DMX_DBG_API_PRINT("%s(): L[%d], Buffer is Empty! \n",__FUNCTION__, __LINE__);
   
        return (RET_FAILURE);               
	}
    
    if(uDataSize < 7)
    {
        DMX_DBG_API_PRINT("%s(): L[%d], PES len is less than 7! \n",__FUNCTION__, __LINE__);

        ctrl_blk->data_continue = 1;  

        ctrl_blk->pts_valid = 0;
    
        return (RET_FAILURE); 
    }       


	//Analyse the PES pkt buffer to analyse the PTS info
	if ((0x00 == *(buf)) && (0x00 ==  *(buf+1))&& (0x01 == *(buf+2)))
	{      
		stream_id = *(buf+3);
        
		pes_pkt_len = (*(buf+5))|((*(buf+4))<<8);
            
        str_type = dmx_pes_stream_type(stream_id);

        //Analyse the payload data according to 13818-1
		switch(str_type)
		{
			case PRG_STR_MAP:
			case PAD_STR:
			case PRIV_STR_2:
			case ECM_STR:
			case EMM_STR:
			case PRG_STR_DIR:
			case DSM_CC_STR:
			case H2221_E_STR:
				{
//					DMX_DBG_API_PRINT("%s(): L[%d],No PES head \n",__FUNCTION__, __LINE__); 

					*payloadPos = 6;  

					ctrl_blk->data_continue = 0;  

					ctrl_blk->pts_valid = 0;

					return  (RET_SUCCESS);                    
                    
				}
              
			default:
			    break;
		} 


		if(pes_pkt_len < 3)
        {
//			DMX_DBG_API_PRINT("%s(): L[%d],Part PES head \n",__FUNCTION__, __LINE__); 

			*payloadPos = 6 + pes_pkt_len;  

			ctrl_blk->data_continue = 0;  

			ctrl_blk->pts_valid = 0;

			return  (RET_SUCCESS);   			
        }
        else
        {
			pes_pkt_head_len = *(buf+8);
        }

		//Analyse the PTS info   
	    ctrl_blk->data_continue = 0; //1;

        *payloadPos =9 + pes_pkt_head_len;

//	    DMX_DBG_API_PRINT("\n %s(),L[%d],PES_start! buf_7[%x], buf_9[%x]\n",\
//	                   		__FUNCTION__, __LINE__, (*(buf+7)),(*(buf+9)) );  

	    //Find a new pts, update write and request new buffer immediately.	    
	    if((0!=((*(buf+7))&0x80))&&(0!=((*(buf+9))&0x20)))
	    {   
	        ctrl_blk->pts_valid=1;
	    
	        ((UINT8 *)(&ctrl_blk->pts))[3] = ((*(buf+9))&0x0e)<<4;
	        ((UINT8 *)(&ctrl_blk->pts))[3] |= ((*(buf+10))&0xf8)>>3;
	        ((UINT8 *)(&ctrl_blk->pts))[2] = ((*(buf+10))&0x07)<<5;
	        ((UINT8 *)(&ctrl_blk->pts))[2] |= ((*(buf+11))&0xf8)>>3;
	        ((UINT8 *)(&ctrl_blk->pts))[1] = ((*(buf+11))&0x06)<<5;
	        ((UINT8 *)(&ctrl_blk->pts))[1] |= ((*(buf+12))&0xfc)>>2;
	        ((UINT8 *)(&ctrl_blk->pts))[0] = ((*(buf+12))&0x03)<<6;
	        ((UINT8 *)(&ctrl_blk->pts))[0] |= ((*(buf+13))&0xfc)>>2;

		    DMX_DBG_API_PRINT("%s(),L[%d],pts_valid[1],pts_value[%x]\n",\
		                       __FUNCTION__, __LINE__,ctrl_blk->pts);  
	    }    
	}
    else
    {
        ctrl_blk->data_continue = 1;   //0 

        ctrl_blk->pts_valid = 0;

        *payloadPos = 0;
    
        DMX_DBG_API_PRINT("%s(),L[%d], The PES has no PTS head \n",\
            				__FUNCTION__, __LINE__);
    }


    return  (RET_SUCCESS);

}

RET_CODE dmx_pes_retrieve
(
    struct dmx_device               *dmx,
    struct dmx_hld_porting_pes_slot *pes_slot
)
{
    INT32  bytes, ret; 
    INT32  buf_remain_len;   
    UINT32  req_len, len;
    static UINT32 payloadPos = 0; //used to save payload pos in the PES packet Buffer
    UINT32 			CpyPesHeadLen = 0; //the copyed PES data len at the current time
    static UINT8	pes_head_buf[PES_HEAD_PTS_LEN];
	static UINT32  cur_pes_pkt_len = 0;	//the PES PKT total length
	static UINT32  cur_pes_pkt_rd = 0;  //the read bytes of PES PKT
    static UINT32  pes_head_len = 0;	//The temporary Buf len to store PES pkt head
    static UINT32  pes_head_rd = 0;  //the read bytes of the temporary Buf len for PES pkt head
    static struct control_block ctrl_blk;

 	//DMX_DBG_API_PRINT
    DMX_DBG_API_PRINT("\n %s(): L[%d], dmx[%x], pes_slot[%x] \n",\
        				__FUNCTION__, __LINE__, dmx, pes_slot);

	//Get the buf len to save the PES PKT
	if(0 == cur_pes_pkt_len)
	{
		//Get the PES PKT total length
        ioctl(pes_slot->linux_fd, ALI_DMX_CHANNEL_GET_CUR_PKT_LEN,
                   &cur_pes_pkt_len);
        
        DMX_DBG_API_PRINT("%s(): L[%d],  cpp_l[%d],cpp_rd[%d]\n", \
            				__FUNCTION__, __LINE__,cur_pes_pkt_len, cur_pes_pkt_rd);


		req_len = cur_pes_pkt_len;

        memset(pes_head_buf, 0, PES_HEAD_PTS_LEN);

//        memset(&ctrl_blk, 0, sizeof(struct control_block));
        
        cur_pes_pkt_rd = 0;
    }
    else
    {
        req_len = cur_pes_pkt_len - cur_pes_pkt_rd;

//	    DMX_DBG_API_PRINT("222, cpp_l[%d],cpp_rd[%d]\n", cur_pes_pkt_len, cur_pes_pkt_rd);
     
    } 

	//DMX have PES Data sent to AP
    if(cur_pes_pkt_len > 0)
	{
        DMX_DBG_API_PRINT("%s(),L[%d], req_len[%d] \n", \
    				__FUNCTION__, __LINE__, req_len);

		//Save the PES head pkt firstly to analyse PTS info for block info
		if( 0 == cur_pes_pkt_rd )
        {   
        	//Start the new PES Data
            if(cur_pes_pkt_len >= PES_HEAD_PTS_LEN)
            {   
                pes_head_len = PES_HEAD_PTS_LEN;
            }
            else
            {
                pes_head_len = cur_pes_pkt_len;
            }

            DMX_DBG_API_PRINT("%s(),L[%d], ----- pes_head_len[%d] \n",\
                                __FUNCTION__, __LINE__, pes_head_len);
            
			bytes = read(pes_slot->linux_fd, pes_head_buf, pes_head_len);
			if (bytes < 0)
			{
			    DMX_DBG_API_PRINT("%s(),L[%d], bytes[%d],pes_head_len[%d]---%m--- Read fail! \n",\
			                        __FUNCTION__, __LINE__, bytes,pes_head_len,errno);

			    return(RET_FAILURE);
			} 
            
			pes_head_rd = 0;

			memset(&ctrl_blk, 0, sizeof(struct control_block));

            ret = dmx_pes_get_head(pes_head_buf, pes_head_len,&payloadPos, &ctrl_blk);
            if(RET_FAILURE == ret)
            {
			    DMX_DBG_API_PRINT("%s(),L[%d], Analyse the PES head fail ! \n",\
			                        __FUNCTION__, __LINE__);

				pes_slot->stat_info.PesHdrErrCnt++; 

			    return(RET_FAILURE);
            }

            CpyPesHeadLen = payloadPos;
       
#if 0
			DMX_DBG_API_PRINT("----%s(),L[%d],pes_head_len[%d], payloadPos[%d]! \n",\
			           __FUNCTION__, __LINE__,pes_head_len, payloadPos);

			for(bytes = 0; bytes<pes_head_len; bytes++)
            {               
                DMX_DBG_API_PRINT(" %2x ", pes_head_buf[bytes]);
            }   
			DMX_DBG_API_PRINT("\n ");
            
            DMX_DBG_API_PRINT(" %s():L[%d],pts_valid[%d], pts[%x], data_continue[%d] \n",\
                    __FUNCTION__,__LINE__,ctrl_blk.pts_valid, ctrl_blk.pts, ctrl_blk.data_continue);
#endif
        }        
        else
        {
        	//Continue to get the remanent PES Data
			memset(&ctrl_blk, 0, sizeof(struct control_block));
            
			ctrl_blk.data_continue = 1; //0;
        }

		//Request buffer from AP 
        memcpy( &(pes_slot->pSDataCtrlBlk), &ctrl_blk, sizeof(struct control_block));
#if 0       
        DMX_DBG_API_PRINT("%s(): L[%d], pes_req_buf[0x%x] \n", __FUNCTION__, __LINE__,pes_slot->pes_req_buf);
        DMX_DBG_API_PRINT("   pes_hdl[%x], len[%x] \n", pes_slot->pes_hdl, req_len);
        DMX_DBG_API_PRINT("   &buf[%x], buf[%x] \n", &(pes_slot->buf), pes_slot->buf);
        DMX_DBG_API_PRINT("   &buf_len[%x], buf_len[%x] \n", &(pes_slot->buf_len), pes_slot->buf_len);
        DMX_DBG_API_PRINT("   &pSDataCtrlBlk[%x] \n \n \n", &(pes_slot->pSDataCtrlBlk));
#endif   
        pes_slot->pes_req_buf((void *)(pes_slot->pes_hdl),
								req_len ,
								(void **)(&(pes_slot->buf)),
								&(pes_slot->buf_len),
								&(pes_slot->pSDataCtrlBlk));

		if ((0 == pes_slot->buf_len) || (NULL == pes_slot->buf))
		{
		    pes_slot->buf_wr = 0;

		    pes_slot->buf_len = 0;

		    pes_slot->buf = NULL;

		    DMX_DBG_API_PRINT("%s(),L[%d],addr:0x%x,len:%d\n", \
		                    __FUNCTION__, __LINE__, pes_slot->buf, pes_slot->buf_len);

			pes_slot->stat_info.PesReqFailCnt++;

		    return(RET_FAILURE);
		}

		pes_slot->stat_info.PesBufReqCnt++;

        //Manage the buffer from AP
        //Copy the temporary PES head buffer into the buf from AP
        if( 0 == cur_pes_pkt_rd )
		{          
			//Copy the PES head to the buffer from AP
            
            if(pes_head_len > (pes_slot->buf_len))
            {
				len = pes_slot->buf_len;
            }
            else
            {
				len = pes_head_len;
            }

            //DMX_DBG_API_PRINT("%s(),L[%d], ---- cpy_tmp_head_len[%d]\n",\
            //                    __FUNCTION__, __LINE__, len);

            if(payloadPos >= len )
            {
            //    DMX_DBG_API_PRINT("%s(),L[%d], payloadPos[%d], len[%d]\n",\
            //                        __FUNCTION__, __LINE__, payloadPos, len); 
                
                CpyPesHeadLen = len;

				payloadPos = payloadPos - len; 

                pes_slot->buf_wr = 0;
            }
            else
            { 
            //    DMX_DBG_API_PRINT("%s(),L[%d], payloadPos[%d], len[%d]\n",\
            //                        __FUNCTION__, __LINE__, payloadPos, len); 
                
				memcpy((UINT8 *)(pes_slot->buf), (UINT8 *)(pes_head_buf+payloadPos), len-payloadPos);

                pes_slot->buf_wr = len-payloadPos; 

                CpyPesHeadLen = payloadPos;

                payloadPos = 0;
             
            }

			pes_head_rd += len;   

			cur_pes_pkt_rd += len;
               
        }
        else 
        {
            pes_slot->buf_wr = 0;   

            CpyPesHeadLen = 0;

            //The temporary PES PKT has remanent data to copy
			if(pes_head_rd < pes_head_len)
			{
				len = pes_head_len - pes_head_rd;

			    if(len > pes_slot->buf_len )
			    {
					len = pes_slot->buf_len;
			    }

				if( payloadPos > 0 )
                {
                    if(payloadPos >= len)
                    {
                        //DMX_DBG_API_PRINT("%s(),L[%d], payloadPos[%d, len[%d]]\n",\
                        //                    __FUNCTION__, __LINE__, payloadPos, len); 

                        CpyPesHeadLen = len;
                        
						payloadPos = payloadPos - len;

                        pes_slot->buf_wr = 0;

                    }
                    else
					{
                        //DMX_DBG_API_PRINT("%s(),L[%d], payloadPos[%d, len[%d]]\n",\
                        //                    __FUNCTION__, __LINE__, payloadPos, len); 
                      
						memcpy((UINT8 *)(pes_slot->buf), (UINT8 *)(pes_head_buf + pes_head_rd + payloadPos), len-payloadPos);

						pes_slot->buf_wr = len-payloadPos;

                        CpyPesHeadLen = payloadPos;

						payloadPos = 0;
					}

                }
                else
                {
					  
                    memcpy((UINT8 *)(pes_slot->buf), (UINT8 *)(pes_head_buf + pes_head_rd), len);

					pes_slot->buf_wr += len;  
                    
                }

			    pes_head_rd += len;
                
			    cur_pes_pkt_rd += len;

			}
        }

        //DMX_DBG_API_PRINT("%s(),L[%d],buf_len[%d] buf_wr[%d]\n",\
        //              __FUNCTION__, __LINE__, pes_slot->buf_len, pes_slot->buf_wr);
      

		//Check if the requested buf is free for more PES Data
		if((pes_slot->buf_len > (CpyPesHeadLen + pes_slot->buf_wr))
          &&(pes_head_len == pes_head_rd)&&(pes_head_len>0)
          &&(payloadPos == 0))
		{
			//Read PES PKT Data from kernel to user space in normal condition
            buf_remain_len = pes_slot->buf_len - pes_slot->buf_wr - CpyPesHeadLen;

//			DMX_DBG_API_PRINT("%s(),L[%d],linux_fd[%x],buf_addr[0x%x],buf_len[%d],CpyPesHeadLen[%d],buf_wr[%d],buf_remain_len[%d]\n", \
//			          __FUNCTION__, __LINE__,pes_slot->linux_fd, pes_slot->buf,pes_slot->buf_len,CpyPesHeadLen, pes_slot->buf_wr, buf_remain_len);

			bytes = read(pes_slot->linux_fd, pes_slot->buf + pes_slot->buf_wr,buf_remain_len);

			if (bytes < 0)
			{
				DMX_DBG_API_PRINT("%s(),L[%d], bytes[%d]---%m--- Read fail! \n",\
				              __FUNCTION__, __LINE__, bytes, errno);

				return(RET_FAILURE);
			}

            if(bytes != buf_remain_len)
			{
				DMX_DBG_API_PRINT("%s(),L[%d], bytes[%d]---%m--- Read Error! \n",\
				              __FUNCTION__, __LINE__, bytes, errno);

				return(RET_FAILURE);
            }

			pes_slot->buf_wr += bytes;

			cur_pes_pkt_rd += bytes;

			DMX_DBG_API_PRINT("%s(),L[%d],wr:%d\n", __FUNCTION__, __LINE__, pes_slot->buf_wr);

        }

 
		//Inform AP to finish data copy 
		
		if (cur_pes_pkt_rd <= cur_pes_pkt_len)
		{
//		    DMX_DBG_API_PRINT("%s(),L[%d], cur_pes_pkt_rd[%d],cur_pes_pkt_len[%d],buf_wr[%d] \n", \
//		        				__FUNCTION__, __LINE__, cur_pes_pkt_rd,cur_pes_pkt_len, pes_slot->buf_wr );

            if(0 == payloadPos)
            {
                pes_slot->pes_ret_buf((void *)(pes_slot->pes_hdl), pes_slot->buf_wr);
            } 
            else
			{
                pes_slot->pes_ret_buf((void *)(pes_slot->pes_hdl), 0);
            }  
	        
			pes_slot->stat_info.PesBufRetCnt++;

	        pes_slot->buf = NULL;

            pes_slot->buf_wr = 0;

            if(cur_pes_pkt_rd == cur_pes_pkt_len)
            {
				cur_pes_pkt_len = 0;
                
                cur_pes_pkt_rd = 0;
            }

//			DMX_DBG_API_PRINT("%s(),L[%d],wr:%d\n",\
//								__FUNCTION__, __LINE__, pes_slot->buf_wr);
		}  
        else
		{

			DMX_DBG_API_PRINT("%s(),L[%d],Read data is larger than Buf len! \n", \
               					__FUNCTION__, __LINE__);

            cur_pes_pkt_len = 0;
            
            cur_pes_pkt_rd = 0;
        }
	}
    else
	{
        DMX_DBG_API_PRINT("%s(),L[%d],DMX has no PES PKT !! \n", \
                        __FUNCTION__, __LINE__);

		pes_slot->stat_info.PesBufEmptyCnt++;
		
        return (RET_FAILURE);     
    }


    DMX_DBG_API_PRINT("%s(),L[%d],Finish! \n", __FUNCTION__, __LINE__);

    return(RET_SUCCESS);
}


void dmx_pes_retrieve_task
(
    UINT32 para1,
    UINT32 para2
)
{
    __u32                            i;
    __u32                            j;
    __s32                            n;
    __u32                            offset;
    struct dmx_hld_porting_pes_slot *pes_slot;
    __s32                            got_len;
    struct dmx_device               *dmx;
    struct dmx_hld_porting          *porting;
    struct pollfd                    pes_fd[DMX_HLD_PORTING_PES_SLOT_CNT];
    __u32                            pes_fd2idx[DMX_HLD_PORTING_PES_SLOT_CNT];

    DMX_DBG_API_PRINT("%s(): L[%d], Start! \n",__FUNCTION__, __LINE__);

    dmx = (struct dmx_device *)para1;

    porting = (struct dmx_hld_porting *)dmx->priv;

    //DMX_DBG_API_PRINT("\n\n%s,%d,pid:%d\n\n", __FUNCTION__, __LINE__, syscall(__NR_gettid));
	//osal_task_save_thread_info("PES");

    DMX_DBG_API_PRINT("%s(): L[%d] \n",__FUNCTION__, __LINE__);

    for (;;)
    {
        j = 0;
        for (i = 0; i < DMX_HLD_PORTING_PES_SLOT_CNT; i++)
        {
            pes_slot = &(porting->pes_slot[i]);

            if (DMX_HLD_PORTING_PES_SLOT_RUN == pes_slot->status)
            {
                pes_fd[j].fd = pes_slot->linux_fd;

                pes_fd[j].events = POLLIN;

                pes_fd2idx[j] = i;

                j++;

                //DMX_DBG_API_PRINT("%s(): L[%d], i[%d],j[%d],linux_fd[%x] \n",\
                //    		__FUNCTION__, __LINE__, i, j, pes_slot->linux_fd);
            }
        }

		if(j == 0)
		{
			osal_task_sleep(10);
			continue;
		}

        if(poll(pes_fd, j, 10) > 0)
        {
            //DMX_DBG_API_PRINT("%s(),L[%d],i[%d],j[%d],linux_fd[%x] \n", \
            //    				__FUNCTION__, __LINE__,i,j,pes_fd[j].fd);

            for (i = 0; i < j; i++)
            {
                if (pes_fd[i].revents)
                {
                    pes_slot = &(porting->pes_slot[pes_fd2idx[i]]);

                    DMX_DBG_API_PRINT("%s(),L[%d], pes_fd2idx[%d]=%d, linux_fd[%x] \n", \
                        				__FUNCTION__, __LINE__,i,pes_fd2idx[i], pes_slot->linux_fd);

                    dmx_pes_retrieve(dmx, pes_slot);
                }
            }
        }

		//osal_task_sleep(10);
    }

    return;
}


UINT32 dmx_rec_pure_pid
(
    UINT16 *raw_pid_list,
    UINT32  raw_pid_list_len,
    UINT16 *pured_pid_list,
    UINT32 *pured_pid_list_len
)
{
    /* Remove repeated pid.
     * i: index to orignal pid list.
     * j: index to no repeat pid list.
     * no_repeat_wr: temp variable used for index into no repeat pid list.
     */
    UINT32 i; 
    UINT32 j;
    UINT32 no_repeat_wr;
    __u16  raw_pid;
    
    no_repeat_wr = 0;

    for (i = 0; i < raw_pid_list_len; i++)
    {
       raw_pid = raw_pid_list[i] & 0x1FFF;

       if (0x1FFF != raw_pid)
       {
           for (j = 0; j < no_repeat_wr; j++)
           {
                if (raw_pid == pured_pid_list[j])
                {
                    break;
                }
           }
    
           /* If current pid in pid list is not in pured pid list yet,
            * add it into pured pid list.
            */
           if (j >= no_repeat_wr)
           {
               pured_pid_list[no_repeat_wr] = raw_pid;
    
               no_repeat_wr++;
           }
       }
    }

    *pured_pid_list_len = no_repeat_wr;

    return(RET_SUCCESS);
}


RET_CODE dmx_rec_stop
(
    struct dmx_device *dmx, 
    UINT32             rec_hdl
)
{

    UINT32                           i; 
    UINT32                           idx;
    struct dmx_hld_porting_rec_slot *rec_slot;
    struct dmx_hld_porting          *porting;

    //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    porting = (struct dmx_hld_porting *)dmx->priv;

    for (i = 0; i < DMX_HLD_PORTING_REC_SLOT_CNT; i++)
    {
        rec_slot = &(porting->rec_slot[i]);

        if ((rec_slot->status  != DMX_HLD_PORTING_REC_SLOT_IDLE) && 
            (rec_slot->rec_hdl == rec_hdl))
        {
            if( (rec_slot->rec_whole_tp != 0) )// && (dmx_rcd->record_ts_ps == 0))  /*Record All mode, ts mode*/
                ioctl(rec_slot->linux_fd, ALI_DMX_BYPASS_ALL, 0);
            ioctl(rec_slot->linux_fd, ALI_DMX_CHANNEL_STOP, 0);    
            close(rec_slot->linux_fd);

            rec_slot->status = DMX_HLD_PORTING_REC_SLOT_IDLE;
            rec_slot->rec_whole_tp = 0;

            //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            return(RET_SUCCESS);
        }
    }

  //  DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(RET_FAILURE);
}




RET_CODE dmx_rec_start
(
    struct dmx_device *dmx,
    UINT32             param
)
{   
    UINT32                           i,slot_id;
    INT32                            ret;
    struct dmx_hld_porting_rec_slot *rec_slot;
    struct pvr_rec_io_param         *rec_param;
    struct dmx_hld_porting          *porting;
    struct dmx_channel_param         dmx_para;

    //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    rec_param = (struct pvr_rec_io_param *)param;

    porting = (struct dmx_hld_porting *)dmx->priv;

    for (i = 0; i < DMX_HLD_PORTING_REC_SLOT_CNT; i++)
    {
        rec_slot = &(porting->rec_slot[i]);

        if (DMX_HLD_PORTING_REC_SLOT_IDLE == rec_slot->status)
        {
            break;
        }
    }

    if (i >= DMX_HLD_PORTING_REC_SLOT_CNT)
    {
        //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }

	memset(&dmx_para, 0, sizeof(struct dmx_channel_param));

    slot_id = i;
    /* Set record slot parameters. */
    memset(rec_slot, 0, sizeof(struct dmx_hld_porting_rec_slot));
    for(i=0;i<DMX_REC_PID_LIST_MAX_LEN;i++)
    {
        rec_slot->orig_pid_list[i]=0x1FFF;
        rec_slot->user_pid_list[i]=0x1FFF;
        rec_slot->dynamic_pid_list[i]=0x1FFF;
    }
    
    rec_slot->req_buf = rec_param->request;

    rec_slot->ret_buf = rec_param->update;

    rec_slot->rec_hdl = rec_param->hnd;

    rec_slot->rec_whole_tp = rec_param->record_whole_tp;

    /* H264.*/
    if (0 != rec_param->h264_flag)
    {
        rec_slot->out_format = DMX_HLD_PORTING_REC_FORMAT_TS_H264;
    }
    /* MPEG2 TS.*/
    else if (0 == rec_param->rec_type)
    {
        rec_slot->out_format = DMX_HLD_PORTING_REC_FORMAT_TS_MPEG2;
    }
    /* MPEG2 PS.*/
    else if (1 == rec_param->rec_type)
    {
        rec_slot->out_format = DMX_HLD_PORTING_REC_FORMAT_PS_MPEG2;
    }
    /* Raw TS.*/
    else if (2 == rec_param->rec_type)
    {
        rec_slot->out_format = DMX_HLD_PORTING_REC_FORMAT_TS_RAW;
    }
    /* PES.*/
    else if (3 == rec_param->rec_type)
    {
        rec_slot->out_format = DMX_HLD_PORTING_REC_FORMAT_PES;
    }
    /* ES.*/
    else if (4 == rec_param->rec_type)
    {
        rec_slot->out_format = DMX_HLD_PORTING_REC_FORMAT_ES;
    }
    else
    {
       // DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }

    rec_slot->cur_buf_i_frm_offset = 0xFFFF;

    rec_slot->pri_buf_i_frm_offset = 0xFFFF;

    memcpy(rec_slot->user_pid_list, rec_param->io_buff_in, 
           rec_param->buff_in_len);

    rec_slot->user_pid_list_len = (UINT8)(rec_param->buff_in_len >> 1);

#if 0
    for (i = 0; i < rec_slot->user_pid_list_len; i++)
    {
        DMX_DBG_API_PRINT("%s,%d,pid:%d\n", __FUNCTION__, __LINE__, rec_slot->user_pid_list[i]);
    }
#endif

    dmx_rec_pure_pid(rec_slot->user_pid_list, rec_slot->user_pid_list_len,
                     rec_slot->orig_pid_list, &rec_slot->orig_pid_list_len);

#if 0
    for (i = 0; i < rec_slot->orig_pid_list_len; i++)
    {
        DMX_DBG_API_PRINT("%s,%d,pid:%d\n", __FUNCTION__, __LINE__, 
               rec_slot->orig_pid_list[i]);
    }
#endif

    rec_slot->linux_fd = open(porting->linux_dmx_path, O_RDONLY | O_NONBLOCK);

    if (rec_slot->linux_fd < 0)
    {
        dmx_rec_stop(dmx, rec_slot->rec_hdl);

        //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }

    dmx_para.output_space = DMX_OUTPUT_SPACE_USER;
    /*p_DeEnconfig Should always be set everytime CREATE_RECORD_STR_EXT*/ 
    rec_slot->enc_para = dmx_para.enc_para =  (UINT32)(porting->p_DeEnconfig);
#ifdef CONFIG_FAST_COPY    
    dmx_para.fst_cpy_slot =  slot_id;
#else    
    dmx_para.fst_cpy_slot =  -1;
#endif   
    porting->p_DeEnconfig = NULL;

#ifdef SAT2IP_SERVER_SUPPORT
	if(rec_param->buff_out_len == TRUE)
        dmx_para.fst_cpy_slot =  -1; //add for sat2ip
#endif
    for (i = 0; i < 5; i++)
        rec_slot->tail_buf[0] = 0;
    /* PES.*/
    if (3 == rec_param->rec_type)
    {
        //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        dmx_para.output_format = DMX_CHANNEL_OUTPUT_FORMAT_PES;

        dmx_para.pes_param.pid = rec_slot->orig_pid_list[0];
    }
    else
    {
        //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        dmx_para.output_format = DMX_CHANNEL_OUTPUT_FORMAT_TS;
        dmx_para.rec_whole_tp = rec_slot->rec_whole_tp;

        memcpy(&(dmx_para.ts_param.pid_list), rec_slot->orig_pid_list,
               rec_slot->orig_pid_list_len * sizeof(UINT16));
		memset(&(dmx_para.ts_param.needdiscramble), 0, rec_slot->orig_pid_list_len * sizeof(UINT8));
	
        dmx_para.ts_param.pid_list_len = rec_slot->orig_pid_list_len;
    
#if 1
        for (i = 0; i < dmx_para.ts_param.pid_list_len; i++)
        {
           // DMX_DBG_API_PRINT("%s,%d,pid:%d\n", __FUNCTION__, __LINE__, dmx_para.ts_param.pid_list[i]);
        }
#endif
    }

    ret = ioctl(rec_slot->linux_fd, ALI_DMX_CHANNEL_START, &dmx_para);

    if (ret < 0)
    {        
       // DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        dmx_rec_stop(dmx, rec_slot->rec_hdl);

        return(RET_FAILURE);
    }

    if( (rec_slot->rec_whole_tp != 0) )// && (dmx_rcd->record_ts_ps == 0))  /*Record All mode, ts mode*/
    {
       ioctl(rec_slot->linux_fd, ALI_DMX_BYPASS_ALL, 1);
    }
    
    rec_slot->status = DMX_HLD_PORTING_REC_SLOT_RUN;

    //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(RET_SUCCESS);
}


#if 0
INT32 dmx_rec_get_byterate
(
    struct dmx_device *dmx,
    UINT32             param
)
{

    struct dmx_hld_porting_avp_slot slot_found;
    struct dmx_hld_porting          *porting;

    porting = (struct dmx_hld_porting *)dmx->priv;
    slot_found = porting->a_slot;
   
    ioctl(slot_found.linux_fd, ALI_DMX_GET_PROG_BITRATE,param);
    //*((UINT32*)param) = 5*1024*1024;

    return(RET_SUCCESS);
}
#else

INT32 dmx_get_cur_total_tsincnt
(
    struct dmx_device *dmx    
)
{
    struct dmx_hld_porting_avp_slot  slot_found;
    struct dmx_hld_porting          *porting;
    INT32                            ret;
	struct Ali_DmxDrvTsFltStatInfo	 TsFltStatInfo;
    INT32 							 linux_fd;	
	UINT32                           tsincnt;
	UINT32                           sec_slot_idx;
    UINT32 							 sec_slot_state;
	
	tsincnt = 0;
	porting = (struct dmx_hld_porting *)dmx->priv;

    /* Audio
	*/
    linux_fd = porting->a_slot.linux_fd;
	
	memset(&TsFltStatInfo, 0, sizeof(TsFltStatInfo));
	
	ret = ioctl(linux_fd, ALI_DMX_CHAN_AUDIO_FILTER_INFO_GET, &TsFltStatInfo);

	if (ret < 0)
	{
		printf("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);
	}
    else
    {
	    tsincnt += TsFltStatInfo.TsInCnt;
        printf("%s,%d,tsincnt:%d\n", __FUNCTION__, __LINE__, TsFltStatInfo.TsInCnt);
	}

	/* Video
	*/
    linux_fd = porting->v_slot.linux_fd;	
	
	memset(&TsFltStatInfo, 0, sizeof(TsFltStatInfo));

	ret = ioctl(linux_fd, ALI_DMX_CHAN_VIDEO_FILTER_INFO_GET, &TsFltStatInfo);

	if (ret < 0)
	{
		printf("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);		
	}
    else
    {
	    tsincnt += TsFltStatInfo.TsInCnt;

        printf("%s,%d,tsincnt:%d\n", __FUNCTION__, __LINE__, TsFltStatInfo.TsInCnt);			
	}

    /* Pcr.
	*/
    if ((porting->p_slot.pid != 0x1FFF) &&
        ((porting->p_slot.pid & 0x1FFF) != (porting->v_slot.pid & 0x1FFF)) &&
        ((porting->p_slot.pid & 0x1FFF) != (porting->a_slot.pid & 0x1FFF)))
    {
        linux_fd = porting->p_slot.linux_fd;    
    	ret = ioctl(linux_fd, ALI_DMX_CHAN_VIDEO_FILTER_INFO_GET, &TsFltStatInfo);
    
    	if (ret < 0)
    	{
    		printf("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);		
    	}
        else
        {
    	    tsincnt += TsFltStatInfo.TsInCnt;

            printf("%s,%d,tsincnt:%d\n", __FUNCTION__, __LINE__, TsFltStatInfo.TsInCnt);			
    	}
	}

	/* Section.
	*/
	for(sec_slot_idx = 0; sec_slot_idx < DMX_HLD_PORTING_SEC_SLOT_CNT; sec_slot_idx++)
	{
		linux_fd = porting->sec_slot[sec_slot_idx].linux_fd;

        sec_slot_state = porting->sec_slot[sec_slot_idx].status;

		if((linux_fd > 0)&&(DMX_HLD_PORTING_SEC_SLOT_RUN == sec_slot_state))
		{
			memset(&TsFltStatInfo, 0, sizeof(TsFltStatInfo));
        	ret = ioctl(linux_fd, ALI_DMX_CHAN_SEC_TS_FILTER_INFO_GET, &TsFltStatInfo);
        
        	if (ret < 0)
        	{
        		printf("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);		
        	}
            else
            {
        	    tsincnt += TsFltStatInfo.TsInCnt;
                printf("%s,%d,tsincnt:%d\n", __FUNCTION__, __LINE__, TsFltStatInfo.TsInCnt);					
        	}           
		}							
	}

	return(tsincnt);
}


INT32 dmx_rec_get_byterate
(
    struct dmx_device *dmx,
    UINT32             param
)
{
    struct dmx_hld_porting_avp_slot  slot_found;
    struct dmx_hld_porting          *porting;
    UINT32                           ts_cnt_before;
    UINT32                           ts_cnt_after;
    UINT32                           ts_cnt_inc;		
    UINT32                           tick_before;
    UINT32                           tick_after;
    UINT32                           tick_inc;
    UINT32                           byterate;
	
    porting = (struct dmx_hld_porting *)dmx->priv;
   
	if( DMX_HLD_PORTING_STATUS_RUN == porting->status )
    {
        return(0);
    }

    ts_cnt_before = dmx_get_cur_total_tsincnt(dmx);
	tick_before = osal_get_tick();

	osal_mutex_unlock(porting->ioctl_mutex);
	
	osal_task_sleep(500);

	osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

	tick_after = osal_get_tick();	
    ts_cnt_after = dmx_get_cur_total_tsincnt(dmx);

	ts_cnt_inc = ts_cnt_after - ts_cnt_before;
	tick_inc = tick_after - tick_before;

	byterate = (ts_cnt_inc * 188 * 1000) / tick_inc;

    //printf("%s,%d,byterate:%d\n", __FUNCTION__, __LINE__, byterate);		

	*((UINT32 *)param) = byterate;

    return(RET_SUCCESS);
}


#endif

UINT16 dmx_rec_find_i_frm
(
    struct dmx_device               *dmx,
    struct dmx_hld_porting_rec_slot *rec_slot
)
{
    UINT32 i, j, pos_payload;
    UINT16 pid;
    UINT32 ts_start;
    UINT8 *buf = rec_slot->buf;
    //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
#ifdef CONFIG_FAST_COPY    
    buf = fcp_quantum_address(((UINT32)rec_slot->buf>>16));
#endif

    for (i = 0; i < rec_slot->buf_wr; i += 188)
    {
        pid = ((buf[i + 1] & 0x1F) << 8) | buf[i + 2];

        //DMX_DBG_API_PRINT("exp pid:%d,cur pid:%d\n\n\n\n", rec_slot->orig_pid_list[0], pid);
        //Only check video packet
        if (pid != rec_slot->orig_pid_list[0])
            continue;
        
        //Get payload
        pos_payload = 4;
        if (0x20 & buf[i + 3])//adaption field
        {
            if (buf[i + 4] > 182)
                continue;
            pos_payload = 5 + buf[i + 4];
        }
        if ((buf[i + 1] & 0x40) && (188 - pos_payload) > 9)
            pos_payload += 9 + buf[i + pos_payload + 8];

        if(pos_payload > 188 - 5)
            continue;

        //Process start code on boundary 
        for(j = 0; j < 5; j++)
            rec_slot->tail_buf[5 + j] = buf[i + pos_payload + j];
        for(j = 0; j < 5; j++)
            if ((0 == rec_slot->tail_buf[j + 0]) && (0 == rec_slot->tail_buf[j + 1]) && 
                (1 == rec_slot->tail_buf[j + 2]) && (0 == rec_slot->tail_buf[j + 3]) &&
                (8 == (rec_slot->tail_buf[j + 5] & 0x38)))
            {
                return (i/188);
            }
        
        //Process start code on payload
        for(j = (i + pos_payload); j < (i + 188 - 5); j++)
            if ((0 == buf[j + 0]) && (0 == buf[j + 1]) && 
                (1 == buf[j + 2]) && (0 == buf[j + 3]) &&
                (8 == (buf[j + 5] & 0x38)))
            {
                return (i/188);
            }
        
        for(j = 0; j < 5; j++)
            rec_slot->tail_buf[j] = buf[(i + 188 - 5) + j];
    }

    //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(0xFFFF);
}

/* Change current pkt PID to orignal pkt pid.
 * This is ugly, a terrible design. But this is what the history is,
 * so we must do this for keeping backward compatibility.
 */
INT32 dmx_rec_dynamic_pid_process
(
    struct dmx_device               *dmx,
    struct dmx_hld_porting_rec_slot *rec_slot
)
{
    UINT32 i;
    UINT32 pkt_pid;
    UINT32 cmp_cnt;
    UINT32 cmp_idx;
    UINT32 orig_pid;
    UINT32 chk_pid;
    //UINT32 new_pid;
    UINT8 *buf = rec_slot->buf;
#ifdef CONFIG_FAST_COPY    
    buf = fcp_quantum_address(((UINT32)rec_slot->buf>>16));
#endif
    //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    if ((rec_slot->orig_pid_list_len > 3) && 
        (rec_slot->dynamic_pid_list_len > 3))
    {
        cmp_cnt = 3;
    }
    else if (rec_slot->orig_pid_list_len < rec_slot->dynamic_pid_list_len)
    {
        cmp_cnt = rec_slot->orig_pid_list_len;
    }
    else
    {
        cmp_cnt = rec_slot->dynamic_pid_list_len;
    }

    for (i = 0; i < rec_slot->buf_wr; i += 188)
    {
		pkt_pid = ((buf[i + 1] & 0x1F) << 8) | buf[i + 2];

        for (cmp_idx = 0; cmp_idx < cmp_cnt; cmp_idx++)
        {
            orig_pid = rec_slot->orig_pid_list[cmp_idx];
            
            chk_pid = rec_slot->dynamic_pid_list[cmp_idx];

            if ((pkt_pid == chk_pid) && (pkt_pid != orig_pid) && orig_pid != 0x1FFF)
            {
                //DMX_DBG_API_PRINT("pid:%d->%d\n", pkt_pid, orig_pid);

                buf[i + 1] &= 0xE0;

                buf[i + 1] |= (UINT8)((orig_pid >> 8) & 0x1F);

                buf[i + 2] = (UINT8)(orig_pid & 0xFF);

                //new_pid = ((rec_slot->buf[i + 1] & 0x1F) << 8) | rec_slot->buf[i + 2];

                //DMX_DBG_API_PRINT("new_pid:%d\n", new_pid);

                break;
            }
        }
    }

    return(RET_SUCCESS);
}



RET_CODE dmx_rec_change_pid
(
    struct dmx_device *dmx,
    UINT32             param
)
{
    UINT32                           i;
    INT32                            ret;
    struct io_param_ex              *chg_param;
    struct dmx_channel_param         dmx_para;
    struct dmx_hld_porting          *porting;
    struct dmx_hld_porting_rec_slot *rec_slot;

    DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    chg_param = (struct io_param_ex *)param;

    porting = (struct dmx_hld_porting *)dmx->priv;

    for (i = 0; i < DMX_HLD_PORTING_REC_SLOT_CNT; i++)
    {
        rec_slot = &(porting->rec_slot[i]);

        if ((DMX_HLD_PORTING_REC_SLOT_RUN == rec_slot->status) && 
            (rec_slot->rec_hdl == chg_param->hnd))
        {
            break;
        }
    }

    if (i >= DMX_HLD_PORTING_REC_SLOT_CNT)
    {
        DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }

	memset(&dmx_para, 0, sizeof(struct dmx_channel_param));

    for(i=0;i<DMX_REC_PID_LIST_MAX_LEN;i++)
    {
        rec_slot->dynamic_pid_list[i]=0x1FFF;
        dmx_para.ts_param.pid_list[i]=0x1FFF;
    }
    
    dmx_rec_pure_pid((UINT16 *)chg_param->io_buff_in, 
                     chg_param->buff_in_len >> 1, 
                     rec_slot->dynamic_pid_list,
                     &rec_slot->dynamic_pid_list_len);

    DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    dmx_para.output_space = DMX_OUTPUT_SPACE_USER;

    dmx_para.output_format = DMX_CHANNEL_OUTPUT_FORMAT_TS;

    memcpy(&(dmx_para.ts_param.pid_list), rec_slot->dynamic_pid_list,
           rec_slot->dynamic_pid_list_len * sizeof(UINT16));
	memset(&(dmx_para.ts_param.needdiscramble), 0, rec_slot->dynamic_pid_list_len * sizeof(UINT8));
	
    dmx_para.ts_param.pid_list_len = rec_slot->dynamic_pid_list_len;
    
#if 0
    for (i = 0; i < dmx_para.ts_param.pid_list_len; i++)
    {
        DMX_DBG_API_PRINT("%s,%d,pid:%d\n", __FUNCTION__, __LINE__, dmx_para.ts_param.pid_list[i]);
    }
#endif

    dmx_para.enc_para = rec_slot->enc_para;
    
    rec_slot->status = DMX_HLD_PORTING_REC_SLOT_IDLE;

    /* Close old rec. */
    ioctl(rec_slot->linux_fd, ALI_DMX_CHANNEL_STOP, 0);
    close(rec_slot->linux_fd);

    rec_slot->dynamic_pid_en = 1;

    /* Open new rec. */
    rec_slot->linux_fd = open(porting->linux_dmx_path, O_RDONLY | O_NONBLOCK);

    if (rec_slot->linux_fd < 0)
    {
        //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }

    ret = ioctl(rec_slot->linux_fd, ALI_DMX_CHANNEL_START, &dmx_para);

    if (ret < 0)
    {        
       // DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }

    rec_slot->status = DMX_HLD_PORTING_REC_SLOT_RUN;

    return(RET_SUCCESS);
}




INT32 dmx_rec_retrieve
(
    struct dmx_device               *dmx,
    struct dmx_hld_porting_rec_slot *rec_slot
)
{
    INT32  bytes; 
    UINT32 buf_remain_len;
    
    /* Useless, only for removing compiler warning. */
    INT32  got_len;
    UINT32 i;
    UINT16 cur_conti;
    UINT16 pid;
    UINT32  req_len;

    //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    buf_remain_len = rec_slot->buf_len - rec_slot->buf_wr;

    if (buf_remain_len <= 0)
    {
        //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        if (NULL != rec_slot->buf)
        {
            if (1 == rec_slot->dynamic_pid_en)
            {
                dmx_rec_dynamic_pid_process(dmx, rec_slot);
            }

            /* Only find I frame offset when recording format is
             * DMX_HLD_PORTING_REC_FORMAT_TS_MPEG2.
             */
            if ((DMX_HLD_PORTING_REC_FORMAT_TS_MPEG2 == rec_slot->out_format) && 
                (0xFFFF == rec_slot->pri_buf_i_frm_offset))
            {
                //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
                if(rec_slot->enc_para != NULL)
                    rec_slot->cur_buf_i_frm_offset = 0;
                else
                    rec_slot->cur_buf_i_frm_offset = dmx_rec_find_i_frm(dmx,
                                                     rec_slot);
            }
            else
            {
                //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

                rec_slot->cur_buf_i_frm_offset = 0xFFFF;
            }

            //DMX_DBG_API_PRINT("%s,%d,len:%d,offset:%d\n", __FUNCTION__, __LINE__, 
                   //rec_slot->buf_wr, rec_slot->cur_buf_i_frm_offset);

            rec_slot->ret_buf(rec_slot->rec_hdl, rec_slot->buf_wr, 
                              rec_slot->cur_buf_i_frm_offset);

            rec_slot->buf = NULL;
        }

        //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        if (DMX_HLD_PORTING_REC_FORMAT_PES == rec_slot->out_format)
        {
            ioctl(rec_slot->linux_fd, ALI_DMX_CHANNEL_GET_CUR_PKT_LEN,
                        &req_len);

            DMX_DBG_API_PRINT("req_len:%d\n", req_len);
        }
        else
        {
            req_len = REQ_DATA_LEN;
        }

        rec_slot->buf_len = rec_slot->req_buf(rec_slot->rec_hdl,
                                              &rec_slot->buf,
                                              req_len,
                                              &got_len /* Unused.*/);

        
#ifdef SAT2IP_SUPPORT
		if ((0 == rec_slot->buf_len) || (NULL == rec_slot->buf))
#else
		if ((rec_slot->buf_len != req_len) || (NULL == rec_slot->buf))
#endif
        {
            rec_slot->buf_wr = 0;

            rec_slot->buf_len = 0;

            rec_slot->buf = NULL;

            //DMX_DBG_API_PRINT("%s,%d,addr:0x%x,len:%d\n", __FUNCTION__, __LINE__, rec_slot->buf, rec_slot->buf_len);

            return(RET_FAILURE);
        }

        rec_slot->buf_wr = 0;

        rec_slot->pri_buf_i_frm_offset = rec_slot->cur_buf_i_frm_offset;

        buf_remain_len = rec_slot->buf_len;

        //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    }

    //DMX_DBG_API_PRINT("%s,%d,fd:%d,addr:0x%x,wr:%d,len:%d\n", __FUNCTION__, __LINE__,
           //rec_slot->linux_fd, rec_slot->buf, rec_slot->buf_wr, buf_remain_len);
#ifdef CONFIG_FAST_COPY
    bytes = read(rec_slot->linux_fd, rec_slot->buf,buf_remain_len);
#else
    bytes = read(rec_slot->linux_fd, rec_slot->buf + rec_slot->buf_wr,
                 buf_remain_len);
#endif
    if (bytes <= 0)
    {
        //perror("read fail");

        return(RET_FAILURE);
    }

    rec_slot->buf_wr += bytes;

    //DMX_DBG_API_PRINT("%s,%d,wr:%d\n", __FUNCTION__, __LINE__, rec_slot->buf_wr);

    return(RET_SUCCESS);
}



void dmx_rec_retrieve_task
(
    UINT32 para1,
    UINT32 para2
)
{
    __u32                            i;
    struct dmx_hld_porting_rec_slot *rec_slot;
    struct dmx_device               *dmx;
    struct dmx_hld_porting          *porting;

    dmx = (struct dmx_device *)para1;

    porting = (struct dmx_hld_porting *)dmx->priv;

    //DMX_DBG_API_PRINT("\n\n%s,%d,pid:%d\n\n", __FUNCTION__, __LINE__, syscall(__NR_gettid));

    for (;;)
    {
        if(porting->status != DMX_HLD_PORTING_STATUS_RUN)
        {
            if(porting->status == DMX_HLD_PORTING_STATUS_STOP_NEXT)
                porting->status = DMX_HLD_PORTING_STATUS_IDLE;
            osal_task_sleep(20);
            continue;
        }
        
        for (i = 0; i < DMX_HLD_PORTING_REC_SLOT_CNT; i++)
        {
            rec_slot = &(porting->rec_slot[i]);
            
            if (DMX_HLD_PORTING_REC_SLOT_RUN == rec_slot->status)
            {
                dmx_rec_retrieve(dmx, rec_slot);
            }
        }

		osal_task_sleep(10);
    }

    return;
}


RET_CODE dmx_new_service_start
(
    struct dmx_device           *dev, 
    struct register_service_new *service_para
)
{
    struct dmx_channel_param         dmx_channel_param;
    struct dmx_hld_porting           *porting = NULL;
    struct dmx_hld_porting_new_service_slot *new_service_slot = NULL;
    int                              ret;
    UINT16                           i;

    /* TODO: Need mutex protection. */

    DMX_DBG_API_PRINT("%s,%d,pid:%d\n", __FUNCTION__, __LINE__, service_para->service_pid);

    porting = (struct dmx_hld_porting *)dev->priv;

    for (i = 0; i < DMX_HLD_PORTING_NEW_SERVICE_SLOT_CNT; i++)
    {
        new_service_slot = &(porting->new_service_slot[i]);

        if (DMX_HLD_PORTING_NEW_SERVICE_SLOT_IDLE == new_service_slot->status)
        {
            new_service_slot->pid = service_para->service_pid;
            new_service_slot->buf = NULL;
            new_service_slot->buf_len = 0;
            new_service_slot->buf_wr = 0;
            memset(&(new_service_slot->ctrl_blk),0,sizeof(struct control_block));
            memcpy(&(new_service_slot->new_service_param),service_para,sizeof(struct register_service_new));
            service_para->service_id= new_service_slot->id;
            DMX_DBG_API_PRINT("%s,%d,id:%d\n", __FUNCTION__, __LINE__, service_para->service_id);
            break;
        }
    }

    if (i >= DMX_HLD_PORTING_NEW_SERVICE_SLOT_CNT)
    {
        DMX_DBG_API_PRINT("No idle new service slot available!\n");

        return(RET_FAILURE); 
    }

	memset(&dmx_channel_param, 0, sizeof(struct dmx_channel_param));

    new_service_slot->linux_fd = open(porting->linux_dmx_path, O_RDONLY | O_NONBLOCK);
    
    if (new_service_slot->linux_fd < 0)
    {        
        DMX_DBG_API_PRINT("%s, line:%d, %s\n", __FUNCTION__, __LINE__,
                          porting->linux_dmx_path);

        return(RET_FAILURE); 
    }


    dmx_channel_param.output_space = DMX_OUTPUT_SPACE_USER;
    switch(service_para->dmx_data_type)
    {
        case DMX_ES_DATA :
        {
            dmx_channel_param.output_format = DMX_CHANNEL_OUTPUT_FORMAT_TS;
            break;
        }
        
        case DMX_SEC_DATA :
        {
            dmx_channel_param.output_format = DMX_CHANNEL_OUTPUT_FORMAT_SEC;
            break;
        }


        case DMX_REC_DATA :
        {
            dmx_channel_param.output_format = DMX_CHANNEL_OUTPUT_FORMAT_TS;
            break;
        }

        case DMX_RAW_DATA :
        {
            DMX_DBG_API_PRINT("%s,%d,pid:%d DMX_RAW_DATA\n", __FUNCTION__, __LINE__);
            dmx_channel_param.output_format = DMX_CHANNEL_OUTPUT_FORMAT_TS;
            dmx_channel_param.fst_cpy_slot = -1;
            dmx_channel_param.enc_para = 0;
            dmx_channel_param.rec_whole_tp = FALSE;
            dmx_channel_param.ts_param.pid_list_len = 1;
            dmx_channel_param.ts_param.pid_list[0] =  service_para->service_pid;

			UINT32 needdiscramble = 0; 
			if(service_para->param != NULL)
				needdiscramble = *((UINT32 *)service_para->param);

			if(needdiscramble)
			{
				dmx_channel_param.ts_param.needdiscramble[0] = 1;
			}
			else
			{
				dmx_channel_param.ts_param.needdiscramble[0] = 0;
			}
            break;
        }
        
        case DMX_PES_DATA:
        {
            DMX_DBG_API_PRINT("%s,%d,pid:%d DMX_PES_DATA\n", __FUNCTION__, __LINE__);
            dmx_channel_param.output_format = DMX_CHANNEL_OUTPUT_FORMAT_PES;
            dmx_channel_param.pes_param.pid = service_para->service_pid;
            break;
        }
        
        default:
            break ;
    }
    dmx_channel_param.fe = porting->fe;
    dmx_channel_param.nim_chip_id = porting->nim_chip_id;
    
    /* Start DMX channel. */
    ret = ioctl(new_service_slot->linux_fd, ALI_DMX_CHANNEL_START, &dmx_channel_param);
    
    if (ret < 0)
    {        
        DMX_PRINTF("ioctl ALI_DMX_CHANNEL_START failed!\n");

        return(RET_FAILURE);
    }   

    new_service_slot->status = DMX_HLD_PORTING_NEW_SERVICE_SLOT_RUN;



    return(RET_SUCCESS); 
}

RET_CODE dmx_new_service_stop
(
    struct dmx_device *dev,
    struct register_service_new *service_para
)
{
    UINT32                           i;
    struct dmx_hld_porting          *porting;
    struct dmx_hld_porting_new_service_slot *new_service_slot;


    DMX_DBG_API_PRINT("%s,%d,pid:%d id:%d\n", __FUNCTION__, __LINE__, service_para->service_pid,service_para->service_id);

    porting = (struct dmx_hld_porting *)dev->priv;

    for (i = 0; i < DMX_HLD_PORTING_NEW_SERVICE_SLOT_CNT; i++)
    {
        new_service_slot = &(porting->new_service_slot[i]);

        if ((DMX_HLD_PORTING_NEW_SERVICE_SLOT_IDLE != new_service_slot->status) &&
            (new_service_slot->id == service_para->service_id))
        {
            break;
        }
    }

    if (i >= DMX_HLD_PORTING_NEW_SERVICE_SLOT_CNT)
    {
        DMX_DBG_API_PRINT("%s, 2\n", __FUNCTION__);

        return(RET_FAILURE); 
    }


    //DMX_DBG_API_PRINT("%s,%d,idx:%d,pid:%d\n", __FUNCTION__, __LINE__, id, sec_slot->sec_para->pid);

    if (DMX_HLD_PORTING_NEW_SERVICE_SLOT_RUN == new_service_slot->status)
    {
        //DMX_DBG_API_PRINT("%s,%d,pid:%d,fd:%d\n", __FUNCTION__, __LINE__, 
               //sec_slot->sec_para->pid, sec_slot->linux_fd);
        ioctl(new_service_slot->linux_fd, ALI_DMX_CHANNEL_STOP, 0);
        close(new_service_slot->linux_fd);
        new_service_slot->linux_fd = -1;
        new_service_slot->buf = NULL;
        new_service_slot->buf_len = 0;
        new_service_slot->buf_wr = 0;
        new_service_slot->pid = 0x1FFF;
        
    }

    new_service_slot->status = DMX_HLD_PORTING_NEW_SERVICE_SLOT_IDLE;

    return(RET_SUCCESS); 
}





INT32 dmx_new_service_retrieve
(
    struct dmx_device               *dmx,
    struct dmx_hld_porting_new_service_slot *new_service_slot
)
{
    INT32  bytes; 
    UINT32 buf_remain_len;
    
    UINT32 i;
    UINT32  req_len;
    struct register_service_new *service_para;

    DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if(new_service_slot == NULL)
    {
        return(RET_FAILURE);
    }

    service_para = &(new_service_slot->new_service_param);
    buf_remain_len = new_service_slot->buf_len - new_service_slot->buf_wr;

    if (buf_remain_len <= 0)
    {
        DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        if (NULL != new_service_slot->buf)
        {

            if(service_para->update_write !=NULL)
            {
                DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
                service_para->update_write(service_para->device, new_service_slot->buf_wr);
                new_service_slot->buf = NULL;
                new_service_slot->buf_wr = 0;
                new_service_slot->buf_len = 0;
            }
            else
            {
                return(RET_FAILURE);
            }

        }

        //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        if (DMX_PES_DATA == service_para->dmx_data_type || DMX_ES_DATA == service_para->dmx_data_type)
        {
            ioctl(new_service_slot->linux_fd, ALI_DMX_CHANNEL_GET_CUR_PKT_LEN,
                        &req_len);

            DMX_DBG_API_PRINT("req_len:%d\n", req_len);
        }
        else
        {
            req_len = REQ_DATA_LEN;
        }
        DMX_DBG_API_PRINT("req_len:%d\n", req_len);

        if(service_para->request_write !=  NULL)
        {
            //new_service_slot->buf_len = service_para->request_write(service_para->device,req_len,&(new_service_slot->buf),&(new_service_slot->buf_len),&(new_service_slot->ctrl_blk));
            service_para->request_write(service_para->device,req_len,&(new_service_slot->buf),&(new_service_slot->buf_len),&(new_service_slot->ctrl_blk));
        }
        else
        {
            return(RET_FAILURE);
        }

        if ((new_service_slot->buf_len != req_len) || (NULL == new_service_slot->buf))
        {
            new_service_slot->buf_wr = 0;

            new_service_slot->buf_len = 0;

            new_service_slot->buf = NULL;

            //DMX_DBG_API_PRINT("%s,%d,addr:0x%x,len:%d\n", __FUNCTION__, __LINE__, rec_slot->buf, rec_slot->buf_len);

            return(RET_FAILURE);
        }

        new_service_slot->buf_wr = 0;


        buf_remain_len = new_service_slot->buf_len;

        //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    }

    bytes = read(new_service_slot->linux_fd, new_service_slot->buf + new_service_slot->buf_wr,
                 buf_remain_len);
    if (bytes <= 0)
    {
        //perror("read fail");

        return(RET_FAILURE);
    }

    new_service_slot->buf_wr += bytes;

    if(new_service_slot->ctrl_blk.instant_update == 1)
    {
        if(service_para->update_write !=NULL)
        {
            service_para->update_write(service_para->device, new_service_slot->buf_wr);
            new_service_slot->buf = NULL;
            new_service_slot->buf_wr = 0;
            new_service_slot->buf_len = 0;
        }
    }
    DMX_DBG_API_PRINT("%s,%d,wr:%d\n", __FUNCTION__, __LINE__, new_service_slot->buf_wr);

    return(RET_SUCCESS);
}

void dmx_new_service_retrieve_task
(
    UINT32 para1,
    UINT32 para2
)
{
    __u32                            i;
    __u32                            j;
    struct dmx_device               *dmx;
    struct dmx_hld_porting          *porting;
    struct dmx_hld_porting_new_service_slot *new_service_slot = NULL;
    struct pollfd                    new_service_fd[DMX_HLD_PORTING_NEW_SERVICE_SLOT_CNT];
    __u32                            new_service_fd2idx[DMX_HLD_PORTING_NEW_SERVICE_SLOT_CNT];
    struct statfs stbuf;

    DMX_DBG_API_PRINT("%s run!\n",__FUNCTION__);

    dmx = (struct dmx_device *)para1;

    porting = (struct dmx_hld_porting *)dmx->priv;

    DMX_DBG_API_PRINT("%s dmx name:%s\n", __FUNCTION__,dmx->name);


    for (;;)
    {
        if(porting->status != DMX_HLD_PORTING_STATUS_RUN)
        {
            if(porting->status == DMX_HLD_PORTING_STATUS_STOP)
                porting->status = DMX_HLD_PORTING_STATUS_STOP_NEXT;
            osal_task_sleep(20);
            continue;
        }
        
        osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);
        j = 0;
        
        for (i = 0; i < DMX_HLD_PORTING_NEW_SERVICE_SLOT_CNT; i++)
        {
            new_service_slot = &(porting->new_service_slot[i]);

            if (DMX_HLD_PORTING_NEW_SERVICE_SLOT_RUN == new_service_slot->status)
            {
                if(fstatfs(new_service_slot->linux_fd, &stbuf) != 0)
                {
//                    printf("invalid fd: \n");
                    continue;
                }
                new_service_fd[j].fd = new_service_slot->linux_fd;

                new_service_fd[j].events = POLLIN;

                new_service_fd2idx[j] = i;

                j++;
            }
        }
        osal_mutex_unlock(porting->ioctl_mutex);
        //DMX_DBG_API_PRINT("%s,%d,n:%d\n", __FUNCTION__, __LINE__, n);
        
        if (poll(new_service_fd, j, 10) > 0) 
        {

            for (i = 0; i < j; i++)
            {
                if (new_service_fd[i].revents)
                {
                    new_service_slot = &(porting->new_service_slot[new_service_fd2idx[i]]);
                    osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);
                    dmx_new_service_retrieve(dmx, new_service_slot);
                    osal_mutex_unlock(porting->ioctl_mutex);
                }
            }

        }
        osal_task_sleep(5);    
    }

    return;
}


RET_CODE dmx_register_service_new
(
    struct dmx_device * dmx, 
    struct register_service_new * reg_serv
)
{
    RET_CODE  ret = RET_FAILURE;
    struct dmx_hld_porting          *porting;

    if(dmx == NULL || reg_serv == NULL)
    {
        return RET_FAILURE;
    }
    
    porting = (struct dmx_hld_porting *)dmx->priv;

    DMX_DBG_API_PRINT("%s %d %d\r\n",__FUNCTION__,__LINE__,reg_serv->dmx_data_type);

    osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

    switch(reg_serv->dmx_data_type)
    {
        case DMX_ES_DATA :
            DMX_DBG_API_PRINT("NOT SUPPORT FORMAT %s %d\r\n",__FUNCTION__,__LINE__);
        	break;

        case DMX_SEC_DATA :
        {
            struct get_section_param*sec_param = (struct get_section_param*)reg_serv->param;
            UINT8 slot_id;

            ret = dmx_sec_start(dmx,sec_param,&slot_id);
            reg_serv->service_pid = slot_id;

            break;
        }

        case DMX_REC_DATA :
        {
           struct pvr_rec_io_param *rec_param = (struct pvr_rec_io_param *)(reg_serv->param) ;
           
           ret = dmx_rec_start(dmx,rec_param);
           
           break;
        }
        
        case DMX_RAW_DATA :
        case DMX_PES_DATA :
            ret = dmx_new_service_start(dmx,reg_serv);
    		break;

    	default:
    		break ;
             
    };
    
    if (ret != RET_SUCCESS)
    {
        DMX_DBG_API_PRINT("%s,%d,SDBBP!!!!\n", __FUNCTION__, __LINE__);
	}

    osal_mutex_unlock(porting->ioctl_mutex);
    return ret ;
}

RET_CODE dmx_unregister_service_new
(
    struct dmx_device * dev, 
    struct register_service_new * reg_serv
)
{

    RET_CODE  ret=RET_FAILURE;
    
    if(dev == NULL || reg_serv == NULL)
    {
        return RET_FAILURE;
    }

    switch(reg_serv->dmx_data_type)
    {
        case DMX_ES_DATA :
            DMX_DBG_API_PRINT("NOT SUPPORT FORMAT %s %d\r\n",__FUNCTION__,__LINE__);
            break;

        case DMX_SEC_DATA :
        {
            ret =dmx_sec_stop(dev, reg_serv->service_pid);
            break;
        }


       case DMX_REC_DATA :
       {
            struct pvr_rec_io_param *rec_param = (struct pvr_rec_io_param *)reg_serv->param ;
            ret = dmx_rec_stop(dev, rec_param->hnd);
       }
        break;

        case DMX_RAW_DATA :
        case DMX_PES_DATA :
            ret =dmx_new_service_stop(dev, reg_serv);
        break;

        default:
         break;   
    }

    if (ret != RET_SUCCESS)
    //SDBBP();

    return ret ;


}


#if 0
inline void dmx_check_conti
(
    UINT8  *buf,
    UINT32  len
)
{
    static UINT8 last_conti;
    UINT8        cur_conti;
    UINT32       i;
    UINT16       pid;
    
    for (i = 0; i < len; i += 188)
    {
        pid = ((buf[i + 1] & 0x1F) << 8) | buf[i + 2];
    
        if (851 == pid)
        {
            cur_conti = buf[i + 3] & 0x0F;
    
            if (cur_conti == last_conti)
            {
    
            }
            else if (((last_conti + 1) & 0xF) != cur_conti)
            {
                DMX_DBG_API_PRINT("pid:%d,exp:%d,cur:%d\n", pid, (last_conti + 1) & 0xF, cur_conti);
            }
    
            last_conti = cur_conti;
        }
    }
    
    return;
}
#endif


static void dmx_playback_stream_control(UINT8 *buf, UINT32 len, UINT16 pid)
{
    static UINT32 time = 0;
    UINT32 pcr;
    struct timeval tv;
    UINT32 cur_time;
    UINT32 delay,interval;
    int i;
#ifdef CONFIG_FAST_COPY
    UINT32 _offset = (UINT32)buf&0xffff;
    buf = fcp_quantum_address(((UINT32)buf>>16)) + _offset;
#endif
    for(i = 0; i < len; i++, buf += 188)
    {
    if(buf[0] == 0x47 && \
    (((buf[1]&0x1f)<<8)|buf[2]) == pid )
    {
        if ((0x20 & buf[3]) && buf[4])//get adaption field
		{
			if (buf[5] & 0x10)//contain pcr
			{
			    ((UINT8 *) (&pcr))[0] = buf[9];
				((UINT8 *) (&pcr))[1] = buf[8];
				((UINT8 *) (&pcr))[2] = buf[7];
				((UINT8 *) (&pcr))[3] = buf[6];

                
                if(time_pcr == 0 || pcr < time_pcr)
                {
                    //printf("reset0 %x %x \n",time_pcr,pcr);
                   time = osal_get_tick();
                   time_pcr = pcr;
                   
                }else
                {
                    delay = (pcr - time_pcr)/45;
                    cur_time = osal_get_tick();
                    interval = cur_time - time;
                    if(delay > interval && (delay - interval) > 20)
                    {
                        if((delay - interval) > 300)
                        {
                            //printf("reset %x %x\n", delay,  interval);
                            time = osal_get_tick();
                            time_pcr = pcr;
                        }else
                        {
                           //printf("stream control 20 ms %x %x\n", delay, interval);
                           osal_task_sleep((delay - interval));
                           break;
                        }
                    }
                }
                
			}
        }
    }
    }
    
}

void dmx_playback_task
(
    UINT32 para1,
    UINT32 para2
)
{
    UINT8                                     *data;
    INT32                                      len;
    INT32                                      vobu_flag;
    struct dmx_device                         *dmx;
    struct dmx_hld_porting                    *porting;
    struct dmx_hld_porting_playback_slot      *pb_slot;
    enum DMX_HLD_PORTING_PLAYBACK_SLOT_STATUS  prev_status;
    UINT8                                      vobu_start_flag_pkt[188];
    UINT8                                      vobu_end_flag_pkt[188];
    UINT8                                      vobu_ca_start_pkt[188];
    UINT32                                     tick_1;
    UINT32                                     tick_2;
    RET_CODE                                ret;
 #ifdef CONFIG_FAST_COPY
    struct dmx_fast_copy_param    param;
 #endif
    UINT32                                    remain_len;
    UINT8                                      decrypt_key[PVR_DECRYPT_KEY_LEN];
    UINT8                                      decrypt_key_pos = 0xff;
    UINT32                                    decrypt_key_len = 0;
    UINT8                                     *vobu_start_pkt;                                      
	
    dmx = (struct dmx_device *)para1;

    porting = (struct dmx_hld_porting *)dmx->priv;

    pb_slot = &(porting->playback_slot);

    memset(vobu_start_flag_pkt, 0, sizeof(vobu_start_flag_pkt));

    vobu_start_flag_pkt[0] = 0xAA;

    vobu_start_flag_pkt[1] = 0x0;

    memset(vobu_end_flag_pkt, 0, sizeof(vobu_end_flag_pkt));

    vobu_end_flag_pkt[0] = 0xAA;

    vobu_end_flag_pkt[1] = 8;

    vobu_end_flag_pkt[2] = 0x00;

    vobu_end_flag_pkt[3] = 0x00;

    vobu_end_flag_pkt[4] = 0x01;

    vobu_end_flag_pkt[5] = 0xB7;

    vobu_end_flag_pkt[6] = 0xFF;

    vobu_end_flag_pkt[7] = 0xFF;

    vobu_end_flag_pkt[8] = 0xFF;

    vobu_end_flag_pkt[9] = 0xFF;

    //DMX_DBG_API_PRINT("\n\n%s,%d,pid:%d\n\n", __FUNCTION__, __LINE__, syscall(__NR_gettid));

    for (;;)
    {
        if (DMX_HLD_PORTING_PLAYBACK_RUN != pb_slot->status)
        {
            if (1 == pb_slot->need_start)
            {
                DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

                osal_semaphore_release(pb_slot->pb_run_to_idle_done_sem);

                pb_slot->status = DMX_HLD_PORTING_PLAYBACK_RUN;

                DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
            }

            osal_task_sleep(10);

            continue;
        }

        if (1 == pb_slot->need_stop)
        {
            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            pb_slot->status = DMX_HLD_PORTING_PLAYBACK_READY;

            osal_semaphore_release(pb_slot->pb_run_to_idle_done_sem);

            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            continue;
        }

        //tick_1 = osal_get_tick();

        len = pb_slot->data_req(pb_slot->hnd, &data, 
                                DMX_HLD_PORTING_PLAYBACK_REQ_LEN,
                                &vobu_flag);

        if (0 == len)
        {
            DMX_DBG_API_PRINT("%s,%d 0\n", __FUNCTION__, __LINE__);
            osal_task_sleep(10);
            continue;
        }

        //tick_2 = osal_get_tick();

        //dmx_check_conti(data, len);
        UINT32 first_key_pos = 0;
	 vobu_start_pkt = &vobu_start_flag_pkt[0];
	 if(pb_slot->request_key)
	 {
	     if(pb_slot->request_key(pb_slot->hnd, decrypt_key, &decrypt_key_len, &decrypt_key_pos, (UINT8 *)&first_key_pos) != -1)
       {
       	   memset(vobu_ca_start_pkt, 0x00, sizeof(vobu_ca_start_pkt));
           decrypt_key_len /= 8;
           vobu_ca_start_pkt[0] = 0xaa;
           vobu_ca_start_pkt[1] = 0;
           
           memcpy(&vobu_ca_start_pkt[2], (UINT8 *)(&decrypt_key_len),   4);
           memcpy(&vobu_ca_start_pkt[6], (UINT8 *)(&decrypt_key_pos),  1);
	       memcpy(&vobu_ca_start_pkt[10], (UINT8 *)(&first_key_pos),  4);	 
           memcpy(&vobu_ca_start_pkt[14], (UINT8 *)decrypt_key, decrypt_key_len);
           
           vobu_start_pkt = &vobu_ca_start_pkt[0];					
        }	
    }				

	 if (DMX_VOB_START == vobu_flag)
        {
        	  do{
                ret = write(pb_slot->linux_fd, vobu_start_pkt,
                  188);
                if(ret >= 0)
                    break;
                osal_task_sleep(10);
            }while(1);
        }

        //dmx_playback_stream_control(data, len, porting->p_slot.pid);
        //Wait for a while if there are much data pending in kernel buf(allow 500ms for SD and 50 ms for HD)
        /*do{
        remain_len = 0;
        ioctl(pb_slot->linux_fd, ALI_DMX_PLAYBACK_REMAIN_DATA, &remain_len);
        if(remain_len < DMX_HLD_PORTING_PLAYBACK_MAX_REMAIN_LEN)
            break;
        osal_task_sleep(5);
        }while(1);
        */
        do{
#ifdef CONFIG_FAST_COPY
            param.data = data;
            param.len = len;
            ret = ioctl(pb_slot->linux_fd, ALI_DMX_PLAYBACK_FSTCPY, &param);
#else
            ret = write(pb_slot->linux_fd, data, len);
#endif
            if(ret >= 0)
			{
			    if(ret < len)
				{
			    while(1)
			    {
				    if (DMX_HLD_PORTING_PLAYBACK_RUN != pb_slot->status)
					{
                    if (1 == pb_slot->need_start)
                    {
					    osal_semaphore_release(pb_slot->pb_run_to_idle_done_sem);
						pb_slot->status = DMX_HLD_PORTING_PLAYBACK_RUN;
						//printf("next trasnfer");
						do{
						data += ret;
						len -= ret;
#ifdef CONFIG_FAST_COPY
                        param.data = data;
                        param.len = len;
                        ret = ioctl(pb_slot->linux_fd, ALI_DMX_PLAYBACK_FSTCPY, &param);
#else
                        ret = write(pb_slot->linux_fd, data, len);
#endif
                        if(ret < 0 || len <= 0)
						  break;
						osal_task_sleep(10);  
                        }while(1);
						break;
					}
					osal_task_sleep(10);
					continue;
					}
					if (1 == pb_slot->need_stop)
                    {
					    if(porting->status != DMX_HLD_PORTING_STATUS_PAUSE)
						    break;
                        pb_slot->status = DMX_HLD_PORTING_PLAYBACK_READY;
						osal_semaphore_release(pb_slot->pb_run_to_idle_done_sem);
                    }
					osal_task_sleep(10);
				}
				}
                break;
            }   
            osal_task_sleep(10);
        }while(1);
        if (DMX_VOB_END == vobu_flag)
        {
            write(pb_slot->linux_fd, &vobu_end_flag_pkt,
                  sizeof(vobu_end_flag_pkt));
        }

        //DMX_DBG_API_PRINT("l:%d\n", len);
    }

    return;
}


INT32 dmx_playback_set_param
(
    struct dmx_device *dmx,
    UINT32             param
)
{
    struct dmx_hld_porting               *porting;
    struct pvr_play_io_param             *p_param;
    struct dmx_hld_porting_playback_slot *pb_slot;

    p_param = (struct pvr_play_io_param *)param;

    porting = (struct dmx_hld_porting *)dmx->priv;

    if (DMX_HLD_PORTING_DMX_DATA_SRC_PLAYBACK != porting->data_src)
    {
        return(RET_FAILURE);
    }

    pb_slot = &(porting->playback_slot);

    pb_slot->hnd = p_param->hnd;

    pb_slot->h264_flag = p_param->h264_flag;

    pb_slot->rec_type = p_param->rec_type;

    pb_slot->data_req = p_param->p_request;
    pb_slot->is_radio = p_param->isRadio;

    pb_slot->request_key = p_param->p_request_key;

    return(RET_SUCCESS);
}







INT32 dmx_hw_get_free_buf_len
(
    struct dmx_device *dmx,
    UINT32             param
)
{
    struct dmx_hld_porting             *porting;
    struct dmx_hld_porting_hw_ctl_slot *hw_ctl_slot;
    UINT32                              pkt_cnt;

    porting = (struct dmx_hld_porting *)dmx->priv;

    hw_ctl_slot = &(porting->hw_ctl_slot);

    if (hw_ctl_slot->linux_fd < 0)
    {
        return(RET_FAILURE);
    }

    ioctl(hw_ctl_slot->linux_fd, ALI_DMX_HW_GET_FREE_BUF_LEN, &pkt_cnt);

    *((UINT32 *)param) = pkt_cnt;

    //DMX_DBG_API_PRINT("%s,%d,cnt:%d\n", __FUNCTION__, __LINE__, *((UINT32 *)param));

    return(RET_SUCCESS);
}


RET_CODE dmx_tsg_playback_ctrl
(
    struct dmx_device *dmx,
    UINT32             param
)
{
    struct dmx_hld_porting *porting;

    //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    porting = (struct dmx_hld_porting *)dmx->priv;

    
    while(porting->status == DMX_HLD_PORTING_STATUS_PAUSE)
    {
        osal_task_sleep(10);
        
    };
}

///////////////////////////////////////////////////////////////////////////////
//   add for sat2ip
RET_CODE dmx_rec_add_pid(struct dmx_device *dev,UINT32 param)
{
    RET_CODE ret = SUCCESS;
    INT32 i;
    struct dmx_hld_porting          *porting = NULL;
    struct dmx_hld_porting_rec_slot *rec_slot = NULL;
    struct pvr_rec_io_param         *rec_param = NULL;
    struct dmx_channel_param         dmx_para;
    INT32 slot_id;

    porting = (struct dmx_hld_porting *)dev->priv;
    rec_param = (struct pvr_rec_io_param*)param;
    if(rec_param == NULL)
    {
        ret = RET_FAILURE;
        goto f_exit;
    }
    for (i = 0; i < DMX_HLD_PORTING_SEC_SLOT_CNT; i++)
    {
        rec_slot = &(porting->rec_slot[i]);
        if(rec_slot->rec_hdl == rec_param->hnd)
        {
            break;
        }
    }
    
    if(i == DMX_HLD_PORTING_SEC_SLOT_CNT)
    {
        ret = RET_FAILURE;
        goto f_exit;        
    }
    memset(&dmx_para,0x00,sizeof(struct dmx_channel_param));
    slot_id = i;
    memcpy(&(dmx_para.ts_param.pid_list), rec_param->io_buff_in,
           rec_param->buff_in_len * sizeof(UINT16));

    dmx_para.ts_param.pid_list_len = rec_param->buff_in_len;    
    dmx_para.output_space = DMX_OUTPUT_SPACE_USER;
    dmx_para.rec_whole_tp = rec_slot->rec_whole_tp;
    dmx_para.fst_cpy_slot =  -1;
    DMX_DBG_API_PRINT("%s pid:%d,num %d \n",__FUNCTION__,*(UINT16 *)(rec_param->io_buff_in),rec_param->buff_in_len);

    //rec_slot->linux_fd = open(porting->linux_dmx_path, O_RDONLY | O_NONBLOCK);
    //ret = ioctl(rec_slot->linux_fd, ALI_DMX_CHANNEL_START, &dmx_para);
    ret = ioctl(rec_slot->linux_fd, ALI_DMX_CHANNEL_ADD_PID, &dmx_para);
    if (ret < 0)
    {
        //DMX_DBG_API_PRINT("%s,%d ret %d \n", __FUNCTION__, __LINE__,ret);
        DMX_DBG_API_PRINT("%s ret:%d \n",__FUNCTION__,ret);
        ret = RET_FAILURE;
        goto f_exit;                
    }
    memcpy(&rec_slot->user_pid_list[rec_slot->user_pid_list_len],rec_param->io_buff_in,
           rec_param->buff_in_len * sizeof(UINT16));
    rec_slot->user_pid_list_len+= rec_param->buff_in_len;
    dmx_rec_pure_pid(rec_slot->user_pid_list, rec_slot->user_pid_list_len,
                 rec_slot->orig_pid_list, &rec_slot->orig_pid_list_len);
f_exit:;
    return ret;
}

RET_CODE dmx_rec_del_pid(struct dmx_device *dev,UINT32 param)
{
    RET_CODE                            ret = SUCCESS;
    INT32                               i,j;
    struct dmx_hld_porting              *porting = NULL;
    struct dmx_hld_porting_rec_slot     *rec_slot = NULL;
    struct pvr_rec_io_param             *rec_param = NULL;
    struct dmx_channel_param            dmx_para;
    INT32                               slot_id;
    UINT16                              *pid;

    porting = (struct dmx_hld_porting *)dev->priv;
    rec_param = (struct pvr_rec_io_param*)param;
    if(rec_param == NULL || rec_param->io_buff_in == NULL)
    {
        ret = RET_FAILURE;
        goto f_exit;
    }
    for (i = 0; i < DMX_HLD_PORTING_SEC_SLOT_CNT; i++)
    {
        rec_slot = &(porting->rec_slot[i]);
        if(rec_slot->rec_hdl == rec_param->hnd)
        {
            break;
        }
    }
    if(i == DMX_HLD_PORTING_SEC_SLOT_CNT)
    {
        ret = RET_FAILURE;
        goto f_exit;        
    }
    slot_id = i;
    memset(&dmx_para,0x00,sizeof(struct dmx_channel_param));
    
    memcpy(&(dmx_para.ts_param.pid_list), rec_param->io_buff_in,
           rec_param->buff_in_len * sizeof(UINT16));

    dmx_para.ts_param.pid_list_len = rec_param->buff_in_len;    
    dmx_para.output_space = DMX_OUTPUT_SPACE_USER;
    dmx_para.rec_whole_tp = rec_slot->rec_whole_tp;
    
    dmx_para.fst_cpy_slot =  -1;
    DMX_DBG_API_PRINT("%s pid:%d,num %d \n",__FUNCTION__,*(UINT16 *)(rec_param->io_buff_in),rec_param->buff_in_len);
    //ret = ioctl(rec_slot->linux_fd, ALI_DMX_CHANNEL_STOP, &dmx_para);
    ret = ioctl(rec_slot->linux_fd, ALI_DMX_CHANNEL_DEL_PID, &dmx_para);
    if (ret < 0)
    {
        ret = RET_FAILURE;
        goto f_exit;                
    }
    pid = (UINT16 *)(rec_param->io_buff_in);
    for(j = 0; j < rec_param->buff_in_len; j++)
    {
        for(i = 0; i < rec_slot->user_pid_list_len; i++)
        {
            if(pid[j] == rec_slot->user_pid_list[i])
                rec_slot->user_pid_list[i] = 0x1fff;             
        }
    }
    j = 0;
    for(i = 0; i < rec_slot->user_pid_list_len; i++)
    {
        if(rec_slot->user_pid_list[i] != 0x1fff)
        {
           rec_slot->user_pid_list[j] = rec_slot->user_pid_list[i];
           j++;            
        }
    }
    rec_slot->user_pid_list_len = j;
    dmx_rec_pure_pid(rec_slot->user_pid_list, rec_slot->user_pid_list_len,
                 rec_slot->orig_pid_list, &rec_slot->orig_pid_list_len);
f_exit:;
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


RET_CODE dmx_io_control
(
    struct dmx_device *dev,
    UINT32             cmd,
    UINT32             param
)
{
#if (defined _BUILD_UPG_LOADER_WITH_ALISL_ || defined _UPGLOADER_UPGRADE_SUPPORT_)//test share upg lib
	return 0;
#endif

    RET_CODE                   ret;
    enum dmx_see_av_sync_mode  see_av_sync_mode;
    struct dmx_hld_porting    *porting;
    struct dmx_see_av_pid_info     see_av_info;
    
    porting = (struct dmx_hld_porting *)dev->priv;

    ret = RET_FAILURE;

	//osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

    switch(cmd)
    {
         /* Following 3 io commands are A/V playing interfaces. */
        case IO_CREATE_AV_STREAM:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
            
	        osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

            ret = dmx_av_stream_set_pid(dev, param, DMX_PORTING_AV_V_VALID |
                                        DMX_PORTING_AV_A_VALID |DMX_PORTING_AV_AD_VALID);

            osal_mutex_unlock(porting->ioctl_mutex);
        }
        break;

        case IO_STREAM_ENABLE:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
           
     	    osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);
            
            
            ret = dmx_av_stream_start(dev, DMX_PORTING_AV_V_VALID |
                                      DMX_PORTING_AV_A_VALID |DMX_PORTING_AV_AD_VALID);

            osal_mutex_unlock(porting->ioctl_mutex);
        }
        break;

        case IO_STREAM_DISABLE:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
            
	        osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

            ret = dmx_av_stream_stop(dev, param);
            osal_mutex_unlock(porting->ioctl_mutex);

        }
        break;

        case IO_CREATE_AUDIO_STREAM:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_av_stream_set_pid(dev, param, DMX_PORTING_AV_A_VALID |DMX_PORTING_AV_AD_VALID);
        }
        break;

        case AUDIO_STREAM_ENABLE:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_av_stream_start(dev, DMX_PORTING_AV_A_VALID |DMX_PORTING_AV_AD_VALID);
        }
        break;


        case AUDIO_STREAM_DISABLE:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_av_stream_stop(dev, param);
        }
        break;

	#if 1
        case IO_CHANGE_AUDIO_STREAM:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_av_stream_change_a_pid(dev, param);
        }
        break;
	#else
		case IO_CHANGE_AUDIO_STREAM:
		{
			return(RET_SUCCESS);
		}
		break;

	#endif
        case IO_CREATE_VIDEO_STREAM:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_av_stream_set_pid(dev, param, DMX_PORTING_AV_V_VALID);
        }
        break;

        case VIDEO_STREAM_ENABLE:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
            
            ret = dmx_av_stream_start(dev, DMX_PORTING_AV_V_VALID);
        }
        break;

        case VIDEO_STREAM_DISABLE:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_av_stream_stop(dev, param);
        }
        break;

        case IS_AV_SCRAMBLED:
        {
            //DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_see_av_scram_status(dev, param);
        }
        break;

        case IS_AV_SCRAMBLED_EXT:
        {
            //DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
           
            ret = dmx_see_av_scram_status_ext(dev, param);
        }
        break;

        case IS_AV_SOURCE_SCRAMBLED:
        {
            printf("%s, line:%d\n", __FUNCTION__, __LINE__);
           
            ret = dmx_see_av_src_scram_status(dev, param);
        }
        break;		

        /* Following 4 commands are async section retrieving interfaces. */
        case IO_ASYNC_CLOSE:
        {
            //DMX_DBG_API_PRINT("%s,line:%d\n", __FUNCTION__, __LINE__);
            osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);
            ret = dmx_sec_stop(dev, param);
            osal_mutex_unlock(porting->ioctl_mutex);
        }
        break;

        case IO_ASYNC_ABORT:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
            osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);
            ret = dmx_async_sec_close_multi(dev, param);
            osal_mutex_unlock(porting->ioctl_mutex);
        }
        break;

        case IO_ASYNC_POLL:
        {
            //DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_async_sec_poll(dev, param);
        }
        break;

        case DMX_WAKEUP_SIAE:
        {
            //DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_async_sec_poll_release_all(dev, param);
        }
        break;


        /* Following 2 io commands are sync section retrieving interfaces. */
        case IO_STOP_GET_SECTION:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_req_section_release_all(dev, param);
        }
        break;

        /* sync section retrieving interface. */
        case CLEAR_STOP_GET_SECTION:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = RET_SUCCESS;
        }
        break;


        /* DMX recording commands. */
        case CREATE_RECORD_STR_EXT:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_rec_start(dev, param);
        }
        break;

        case DELETE_RECORD_STR:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_rec_stop(dev, param);
        }
        break;

        case DO_TIME_SHIFT:
        {
            DMX_DBG_API_PRINT("%s, line:%d,%d\n", __FUNCTION__, __LINE__, param);

            ret = dmx_av_stream_timeshift(dev, param);
        }
        break;

        case DMX_IS_IN_TIMESHIFT:
        {
            DMX_DBG_API_PRINT("%s, line:%d,%d\n", __FUNCTION__, __LINE__, param);

            ret = dmx_av_stream_is_in_timeshift(dev, param);
        }
        break;

        case RECORD_WHILE_PLAY:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = RET_SUCCESS;
        }
        break;

        case GET_PROG_BITRATE:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_rec_get_byterate(dev, param);
        }
        break;

        case DMX_DVR_CHANGE_PID:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_rec_change_pid(dev, param);
        }
        break;


        case CHECK_DMX_REMAIN_BUF:
        {
            //DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_hw_get_free_buf_len(dev, param);
        }
        break;

        case TSG_PLAYBACK_SYNC:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_tsg_playback_ctrl(dev, param);
             DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
            if (0 == param)
            {
                see_av_sync_mode = DMX_SEE_AV_SYNC_MODE_LIVE;
            }
            else
            {
                see_av_sync_mode = DMX_SEE_AV_SYNC_MODE_TSG_TIMESHIT;
            }

            ret = dmx_see_av_sync_mode_set(dev, see_av_sync_mode);
        }
        break;

        case SET_TSG_PLAYBACK:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            if (0 == param)
            {
                see_av_sync_mode = DMX_SEE_AV_SYNC_MODE_LIVE;
            }
            else
            {
                see_av_sync_mode = DMX_SEE_AV_SYNC_MODE_TSG_TIMESHIT;
            }

            ret = dmx_see_av_sync_mode_set(dev, see_av_sync_mode);
            
        }
        break;
        case IO_SET_TSG_AV_MODE:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_see_av_sync_mode_set(dev, DMX_SEE_AV_SYNC_MODE_TSG_LIVE);
        }
        break;

        case IO_CLEAR_TSG_AV_MODE:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_see_av_sync_mode_set(dev, DMX_SEE_AV_SYNC_MODE_LIVE);
        }
        break;

        /* Playback control only. */
        case IO_CREATE_AV_SET_PARAM:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = dmx_playback_set_param(dev, param);
        }
        break;

        case DMX_SPEED_UP:
        {
            if (porting->see_dev_fd < 0)
                break; 
            UINT32 speed = FAST_PLAYBACK_SPEED;
            ret = ioctl(porting->see_dev_fd , ALI_DMX_SEE_SET_PLAYBACK_SPEED, &speed);
        }
        break;
        
        case DMX_NORMAL_PLAY:
        {
            if (porting->see_dev_fd < 0)
                break; 
            UINT32 speed = NORMAL_PLAYBACK_SPEED;
            ret = ioctl(porting->see_dev_fd , ALI_DMX_SEE_SET_PLAYBACK_SPEED, &speed);              
        }
        break;
        case IO_SET_DEC_HANDLE:
        {
            struct dec_parse_param *p_param = (struct dec_parse_param *)param ;
            if (NULL != p_param->dec_dev)
                p_param->dec_dev = ((pCSA_DEV)(p_param->dec_dev))->priv;
            //printf("%s,%d,dmx dsc handle = 0x%x\n", __FUNCTION__, __LINE__, p_param->dec_dev);
            if (porting->see_dev_fd < 0)
                break; 
            ret = ioctl(porting->see_dev_fd , ALI_DMX_SEE_CRYPTO_TYPE_SET, param);  
        }
	    break;
        
        case IO_SET_DEC_STATUS:
        {
            if(param)
                ret = ioctl(porting->see_dev_fd , ALI_DMX_SEE_CRYPTO_START, 0);
            else
                ret = ioctl(porting->see_dev_fd , ALI_DMX_SEE_CRYPTO_STOP, 0);
        }
        break;
        
        case IO_SET_DEC_CONFIG:
        {
            pDEEN_CONFIG p_DeEn = (pDEEN_CONFIG)param ;

            if (NULL == p_DeEn->dec_dev)
                p_DeEn->dec_dev = NULL;
            else
                p_DeEn->dec_dev = ((pCSA_DEV)(p_DeEn->dec_dev))->priv;
            
             if (NULL == p_DeEn->enc_dev)
                p_DeEn->enc_dev = NULL;
            else
                p_DeEn->enc_dev = ((pCSA_DEV)(p_DeEn->enc_dev))->priv;
            //printf("%s,%d,dmx pDeEn handle = 0x%x,0x%x\n", __FUNCTION__, __LINE__, p_DeEn->dec_dev,p_DeEn->enc_dev);
            porting->p_DeEnconfig = (pDEEN_CONFIG)p_DeEn;
        }
        break ;
        case IO_GET_DISCONTINUE_COUNT:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = ioctl(porting->see_dev_fd , ALI_DMX_SEE_GET_DISCONTINUE_COUNT, param);
        }
        break;
        case IO_ClEAR_DISCONTINUE_COUNT:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = ioctl(porting->see_dev_fd , ALI_DMX_SEE_CLEAR_DISCONTINUE_COUNT, param);
        }
        break;

        case IO_GET_MAIN2SEEBUFFER_REMAIN_DATA_LEN:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            ret = ioctl(porting->see_dev_fd , ALI_DMX_SEE_GET_MAIN2SEEBUF_REAMIN_DATA_LEN, param);
        }
        break;
        case DMX_IO_DO_DDP_CERTIFICATION:
        {
            
            UINT32 enabel = 0;
            
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
            
            enabel = param;
            ret = ioctl(porting->see_dev_fd , ALI_DMX_SEE_DO_DDP_CERTIFICATION, &enabel);
                
        }
        break;
        case DMX_IO_SET_HW_INFO:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
            
            ret = ioctl(porting->hw_ctl_slot.linux_fd, ALI_DMX_SET_HW_INFO, param);
                
        }
        break;
		// add for sat2ip  
        case DMX_CMD_REC_ADD_PID:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
            ret = dmx_rec_add_pid(dev,param);
        }        
        break;
        case DMX_CMD_REC_DEL_PID:
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
            ret = dmx_rec_del_pid(dev,param);            
        }
        break;
        case DMX_SET_SAT2IP:
        {
            UINT32 enabel = 0;
            enabel = param;
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
            ret = ioctl(porting->see_dev_fd , ALI_DMX_SEE_SET_SAT2IP, &enabel);   
        }
		break;
        case DMX_IO_SET_DATA_FLOW:
        {
            porting->m_demux_data_flow = param;
            DMX_DBG_API_PRINT("%s, line:%d, data flow:%d\n", __FUNCTION__, __LINE__, porting->m_demux_data_flow);
        }
		break;
		////////////////////////////////////////////////////////
        default:
        {
            DMX_DBG_API_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, cmd);
        }
        break;
    }

    //osal_mutex_unlock(porting->ioctl_mutex);

    return(ret);
}



RET_CODE dmx_start(struct dmx_device *dmx)
{
#if (defined _BUILD_UPG_LOADER_WITH_ALISL_ || defined _UPGLOADER_UPGRADE_SUPPORT_) //test share upg lib
	return 0;
#endif

#if 1
    struct dmx_hld_porting *porting;
    struct dmx_hld_porting_playback_slot *pb_slot;

    //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    porting = (struct dmx_hld_porting *)dmx->priv;

    pb_slot = &(porting->playback_slot);

	osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

    if (DMX_HLD_PORTING_DMX_DATA_SRC_PLAYBACK == porting->data_src)
    {
        if (porting->playback_slot.status == DMX_HLD_PORTING_PLAYBACK_READY)
        {
            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            if(porting->status != DMX_HLD_PORTING_STATUS_PAUSE)
            {
                ioctl(pb_slot->linux_fd, ALI_DMX_CHANNEL_FLUSH_BUF, 0);
                ioctl(pb_slot->linux_fd, ALI_DMX_IO_SET_PLAYBACK_SRC_TYPE,pb_slot->is_radio);
            }
            pb_slot->need_start = 1;


            //osal_semaphore_capture(pb_slot->pb_run_to_idle_done_sem, 100);
            osal_semaphore_capture(pb_slot->pb_run_to_idle_done_sem, OSAL_WAIT_FOREVER_TIME);
            pb_slot->need_start = 0;


            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
        }

        //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    }
    porting->status = DMX_HLD_PORTING_STATUS_RUN;

	osal_mutex_unlock(porting->ioctl_mutex);

    return(RET_SUCCESS);
#else
    struct dmx_hld_porting *porting;

    //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    porting = (struct dmx_hld_porting *)dmx->priv;

    if (DMX_HLD_PORTING_STATUS_IDLE != porting->status)
    {
        DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }

    porting->status = DMX_HLD_PORTING_STATUS_RUN;

    return(RET_SUCCESS);
#endif
}






RET_CODE dmx_playback_pause(struct dmx_device *dmx)
{
    struct dmx_hld_porting *porting;

    struct dmx_hld_porting_playback_slot *pb_slot;

    porting = (struct dmx_hld_porting *)dmx->priv;

    pb_slot = &(porting->playback_slot);



    if (DMX_HLD_PORTING_DMX_DATA_SRC_PLAYBACK == porting->data_src)
    {
        if (DMX_HLD_PORTING_PLAYBACK_RUN == porting->playback_slot.status)
        {
            //porting->playback_slot.status = DMX_HLD_PORTING_PLAYBACK_READY;

            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            pb_slot->need_stop = 1;

            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            //osal_semaphore_capture(pb_slot->pb_run_to_idle_done_sem, 100);
            osal_semaphore_capture(pb_slot->pb_run_to_idle_done_sem, OSAL_WAIT_FOREVER_TIME);

            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);


            pb_slot->need_stop = 0;

            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
        }

        //osal_task_sleep(100);

        DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    }
    

    return(RET_SUCCESS);
}


RET_CODE dmx_stop(struct dmx_device *dmx)
{
#if (defined _BUILD_UPG_LOADER_WITH_ALISL_ || defined _UPGLOADER_UPGRADE_SUPPORT_)//test share upg lib
	return 0;
#endif

#if 1
    struct dmx_hld_porting *porting;

    struct dmx_hld_porting_playback_slot *pb_slot;

    porting = (struct dmx_hld_porting *)dmx->priv;

    pb_slot = &(porting->playback_slot);

	osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

    if (DMX_HLD_PORTING_DMX_DATA_SRC_PLAYBACK == porting->data_src)
    {
        if (DMX_HLD_PORTING_PLAYBACK_RUN == porting->playback_slot.status)
        {
            //porting->playback_slot.status = DMX_HLD_PORTING_PLAYBACK_READY;
            
            //ioctl(porting->see_dev_fd , ALI_DMX_SEE_PLAYBACK_PRE_PAUSE, 0);
            
            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            pb_slot->need_stop = 1;

            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            //osal_semaphore_capture(pb_slot->pb_run_to_idle_done_sem, 100);
            osal_semaphore_capture(pb_slot->pb_run_to_idle_done_sem, OSAL_WAIT_FOREVER_TIME);

            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            ioctl(pb_slot->linux_fd, ALI_DMX_CHANNEL_FLUSH_BUF, 0);

            pb_slot->need_stop = 0;

            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
        }

        //osal_task_sleep(100);

        DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    }
    porting->status = DMX_HLD_PORTING_STATUS_STOP;

	osal_mutex_unlock(porting->ioctl_mutex);
/*
    porting->status = DMX_HLD_PORTING_STATUS_STOP;
    //Wait for sec task & rec task idle
    while(porting->status != DMX_HLD_PORTING_STATUS_IDLE)
        osal_task_sleep(10);*/
    return(RET_SUCCESS);
#else
    UINT32                  i;
    struct dmx_hld_porting *porting;

    //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    porting = (struct dmx_hld_porting *)dmx->priv;

    if (DMX_HLD_PORTING_STATUS_IDLE == porting->status)
    {
        DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(RET_SUCCESS);
    }

    /* Close AV. */
    dmx_av_stream_stop(dmx, 0);

    /* Close REC slots. */
    for (i = 0; i < DMX_HLD_PORTING_REC_SLOT_CNT; i++)
    {
        if (porting->rec_slot[i].status != DMX_HLD_PORTING_REC_SLOT_IDLE)
        {
            dmx_rec_stop(dmx, porting->rec_slot[i].rec_hdl);
        }
    }  

    /* Close SEC slots. */
    for (i = 0; i < DMX_HLD_PORTING_SEC_SLOT_CNT; i++)
    {
        dmx_sec_stop(dmx, i);
    }
   
    porting->status = DMX_HLD_PORTING_STATUS_RUN;

    return(RET_SUCCESS);
#endif
}








RET_CODE dmx_pause(struct dmx_device *dmx)
{
#if (defined _BUILD_UPG_LOADER_WITH_ALISL_ || defined _UPGLOADER_UPGRADE_SUPPORT_) //test share upg lib
	return 0;
#endif

#if 1

    struct dmx_hld_porting *porting;
    struct dmx_hld_porting_hw_ctl_slot *hw_ctl_slot;
    
    //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    porting = (struct dmx_hld_porting *)dmx->priv;

    if (DMX_HLD_PORTING_DMX_DATA_SRC_PLAYBACK == porting->data_src)
    {
         porting->status = DMX_HLD_PORTING_STATUS_PAUSE;
         ioctl(porting->see_dev_fd , ALI_DMX_SEE_PLAYBACK_PRE_PAUSE, 0);
         dmx_playback_pause(dmx);
         ioctl(porting->see_dev_fd , ALI_DMX_SEE_PLAYBACK_PAUSE, 0);
        //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    }
    porting->status = DMX_HLD_PORTING_STATUS_PAUSE;
//    printf("dmx pause\n");
    //if (DMX_HLD_PORTING_DMX_DATA_SRC_LIVE == porting->data_src)
    {
        osal_task_sleep(100);
    }

    return(RET_SUCCESS);
#else
    struct dmx_hld_porting *porting;

    //DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    porting = (struct dmx_hld_porting *)dmx->priv;

    if (DMX_HLD_PORTING_STATUS_RUN != porting->status)
    {
        DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }

    porting->status = DMX_HLD_PORTING_STATUS_PAUSE;

    return(RET_SUCCESS);

#endif
}



RET_CODE dmx_register_service(struct dmx_device *dmx, UINT8 filter_idx, struct register_service * reg_serv)
{
    RET_CODE                      ret;
    struct dmx_hld_porting       *porting;
    struct dmx_channel_param      ch_para;
    struct dmx_ts_kern_recv_info *ts_kern_recv_info;
    INT32                         linux_fd;
    UINT16                        pid;

    struct get_pes_param          pes_para;
    UINT8 						  slot_id;

    if ((filter_idx != 3) && (filter_idx != 4)&&((filter_idx != 6)))
    {
        return(RET_FAILURE);
    }

	memset(&ch_para, 0, sizeof(struct dmx_channel_param));

    porting = (struct dmx_hld_porting *)dmx->priv;

	osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

	if(porting->m_demux_data_flow == DEMUX_DATA_TO_SEE_CPU_KERNEL_SPACE)
	{
	    pid = reg_serv->service_pid;

	    /* Open see. */
	    if (3 == filter_idx)
	    {
	        ret = ioctl(porting->see_dev_fd, ALI_DMX_SEE_TELETXT_START, &pid);
	    
	        if (ret < 0)
	        {        
	            osal_mutex_unlock(porting->ioctl_mutex);

	            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
	    
	            return(RET_FAILURE);
	        }   
	    }

	    if (4 == filter_idx)
	    {
	        ret = ioctl(porting->see_dev_fd, ALI_DMX_SEE_SUBTITLE_START, &pid);
	    
	        if (ret < 0)
	        {        
	            osal_mutex_unlock(porting->ioctl_mutex);

	            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
	    
	            return(RET_FAILURE);
	        }   
	    }

	    if (6 == filter_idx)
	    {
	        ret = ioctl(porting->see_dev_fd, ALI_DMX_SEE_DCII_SUBT_START, &pid);
	    
	        if (ret < 0)
	        {        
	            osal_mutex_unlock(porting->ioctl_mutex);

	            DMX_DBG_API_PRINT("%s,:%d\n", __FUNCTION__, __LINE__);
	    
	            return(RET_FAILURE);
	        }   
	    }

	    /* Opne new fileres in main CPU. */
	    linux_fd = open(porting->linux_dmx_path, O_RDONLY);

	    if (linux_fd < 0)
	    {        
	        osal_mutex_unlock(porting->ioctl_mutex);

	        DMX_DBG_API_PRINT("%s, line:%d, %s\n", __FUNCTION__, __LINE__,
	                          porting->linux_dmx_path);

	        return(RET_FAILURE);
	    }    

	    ch_para.output_format = DMX_CHANNEL_OUTPUT_FORMAT_TS;

	    ch_para.output_space = DMX_OUTPUT_SPACE_KERNEL;

	    ch_para.ts_param.pid_list[0] = pid;
		ch_para.ts_param.needdiscramble[0] = 0;
		
	    ch_para.ts_param.pid_list_len = 1;
	    ch_para.fe = porting->fe;
	    ch_para.nim_chip_id = porting->nim_chip_id;

	    ts_kern_recv_info = &ch_para.ts_param.kern_recv_info;

	    ret = ioctl(porting->see_dev_fd, ALI_DMX_SEE_GET_TS_INPUT_ROUTINE,
	                ts_kern_recv_info);

	    if (ret < 0)
	    {        
	        osal_mutex_unlock(porting->ioctl_mutex);

	        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

	        return(RET_FAILURE);
	    }

	    /* Step 3: Start dmx filter. */
	    ret = ioctl(linux_fd, ALI_DMX_CHANNEL_START, &ch_para);

	    if (ret < 0)
	    {        
	        osal_mutex_unlock(porting->ioctl_mutex);

	        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

	        return(RET_FAILURE);
	    }

	    if (3 == filter_idx)
	    {
	        porting->ttx_slot.pid = pid;

	        porting->ttx_slot.linux_fd = linux_fd;

			porting->m_demux_data_flow_ttx = porting->m_demux_data_flow;
	    }

	    if (4 == filter_idx)
	    {
	        porting->subt_slot.pid = pid;

	        porting->subt_slot.linux_fd = linux_fd;

			porting->m_demux_data_flow_subt = porting->m_demux_data_flow;
	    }

	    if (6 == filter_idx)
	    {
	        porting->dcii_subt_slot.pid = pid;

	        porting->dcii_subt_slot.linux_fd = linux_fd;

			porting->m_demux_data_flow_dcii_subt = porting->m_demux_data_flow;
	    }	

	}
	else if(porting->m_demux_data_flow == DEMUX_DATA_TO_MAIN_CPU_USER_SPACE)
	{
		pid = reg_serv->service_pid;

		DMX_DBG_API_PRINT("%s(): L[%d] \n",__FUNCTION__, __LINE__);

		/**********************************
		* filter_idx: 3---teletext filter
		*			  4---subtitle filter
		***********************************/
		if((3 == filter_idx) || (4 == filter_idx) || (6 == filter_idx))
		{
			DMX_DBG_API_PRINT("%s(): L[%d], service_pid[%x], filter_idx[%d] \n",\
								__FUNCTION__, __LINE__,reg_serv->service_pid, filter_idx);

			pes_para.dev = reg_serv->device;	   
			pes_para.pid = reg_serv->service_pid;
			pes_para.request = reg_serv->request_write;
			pes_para.update = reg_serv->update_write;
			pes_para.filter_id = filter_idx;
			pes_para.continue_get_pes = 1;

			ret = dmx_req_pes(dmx, &pes_para, &slot_id );	 
			if( RET_SUCCESS != ret )
			{
				DMX_DBG_API_PRINT("%s(),L[%d], dmx_pes_start() fail! \n", __FUNCTION__, __LINE__);

				osal_mutex_unlock(porting->ioctl_mutex);

				return(RET_FAILURE);	 
			}  
		}

	    if (3 == filter_idx)
	    {
			porting->m_demux_data_flow_ttx = porting->m_demux_data_flow;
	    }

	    if (4 == filter_idx)
	    {
			porting->m_demux_data_flow_subt = porting->m_demux_data_flow;
	    }

	    if (6 == filter_idx)
	    {
			porting->m_demux_data_flow_dcii_subt = porting->m_demux_data_flow;
	    }		

	}

    osal_mutex_unlock(porting->ioctl_mutex);

    return(RET_SUCCESS);
}

RET_CODE dmx_unregister_service(struct dmx_device *dmx, UINT8 filter_idx)
{
    struct dmx_hld_porting *porting;
	RET_CODE				ret;

    if ((filter_idx != 3) && (filter_idx != 4)&& (filter_idx != 6))
    {
        return(RET_FAILURE);
    }

    porting = (struct dmx_hld_porting *)dmx->priv;

	osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

    if ((3 == filter_idx) && (porting->m_demux_data_flow_ttx == DEMUX_DATA_TO_SEE_CPU_KERNEL_SPACE))
    {
        /* Stop filter file. */
        if (porting->ttx_slot.linux_fd >= 0)
        {   
            ioctl(porting->ttx_slot.linux_fd, ALI_DMX_CHANNEL_STOP, 0);
            close(porting->ttx_slot.linux_fd);
        }       

        porting->ttx_slot.linux_fd = -1;
    }

    if ((4 == filter_idx) && (porting->m_demux_data_flow_subt == DEMUX_DATA_TO_SEE_CPU_KERNEL_SPACE))
    {
        /* Stop filter file. */
        if (porting->subt_slot.linux_fd >= 0)
        {
            ioctl(porting->subt_slot.linux_fd, ALI_DMX_CHANNEL_STOP, 0);
            close(porting->subt_slot.linux_fd);
        }

        porting->subt_slot.linux_fd = -1;
    }

    if ((6 == filter_idx) && (porting->m_demux_data_flow_dcii_subt == DEMUX_DATA_TO_SEE_CPU_KERNEL_SPACE))
    {
        /* Stop filter file. */
        if (porting->dcii_subt_slot.linux_fd >= 0)
        {
            ioctl(porting->dcii_subt_slot.linux_fd, ALI_DMX_CHANNEL_STOP, 0);
            close(porting->dcii_subt_slot.linux_fd);
        }

        porting->dcii_subt_slot.linux_fd = -1;
    }	

	if(((3 == filter_idx) && (porting->m_demux_data_flow_ttx == DEMUX_DATA_TO_MAIN_CPU_USER_SPACE))
		|| ((4 == filter_idx) && (porting->m_demux_data_flow_subt == DEMUX_DATA_TO_MAIN_CPU_USER_SPACE))
		|| ((6 == filter_idx) && (porting->m_demux_data_flow_dcii_subt == DEMUX_DATA_TO_MAIN_CPU_USER_SPACE)))
	{
		//Stop the Filter to catch Subtitle/Teletext
		ret = dmx_pes_stop(dmx, filter_idx );

		if( RET_SUCCESS != ret )
		{
			DMX_DBG_API_PRINT("%s(),L[%d], dmx_pes_stop() fail! \n", __FUNCTION__, __LINE__);

			osal_mutex_unlock(porting->ioctl_mutex);

			return(RET_FAILURE);	 
		}

		porting->subt_slot.linux_fd = -1;
	}

    osal_mutex_unlock(porting->ioctl_mutex);

    DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(RET_SUCCESS);
}






INT32 dmx_csa_get_slot
(
    struct dmx_device *dmx,
    /* hw dmx filter idx in TDS, sec filter id in linux porting. */
    UINT32             idx 
)
{

    UINT32                           i;
    INT32                           *fd;
    UINT16                           ret;
    struct dmx_hld_porting          *porting;
    struct dmx_hld_porting_sec_slot *sec_slot;
    
    porting = (struct dmx_hld_porting *)dmx->priv;

    fd = NULL;
    
    idx &= 0x7F;
    
    switch(idx)
    {
        case 0:
        {
            if (DMX_HLD_PORTING_AV_STREAM_RUN == porting->av_stream_status)
            {
                fd = &porting->v_slot.linux_fd;
            }
        }
        break;

        case 1:
        {
            if (DMX_HLD_PORTING_AV_STREAM_RUN == porting->av_stream_status)
            {
                fd = &porting->a_slot.linux_fd;
            }
        }
        break;

        case 2:
        {
            if (DMX_HLD_PORTING_AV_STREAM_RUN == porting->av_stream_status)
            {
                fd = &porting->p_slot.linux_fd;
            }
        }
        break;

        case 3:
        {
            if (porting->ttx_slot.linux_fd >= 0)
            {
                fd = &porting->ttx_slot.linux_fd;
            }
        }
        break;

        case 4:
        {
            if (porting->subt_slot.linux_fd >= 0)
            {
                fd = &porting->subt_slot.linux_fd;
            }
        }
        break;

        default:
        {
            for (i = 0; i < DMX_HLD_PORTING_SEC_SLOT_CNT; i++)
            {
                sec_slot = &(porting->sec_slot[i]);

                if ((DMX_HLD_PORTING_SEC_SLOT_RUN == sec_slot->status) &&
                    (sec_slot->id == idx))
                {
                    fd = &(sec_slot->linux_fd);

                    break;
                }
            }
        }

        break;
    }
    
    if (NULL == fd)
        return -1;   
        
    if(idx < 5)
    {
        if(porting->csa_slot[idx].status == ALI_CSA_SLOT_STATUS_IDLE)
            porting->csa_slot[idx].status = ALI_CSA_SLOT_STATUS_READY;
        return idx;
    }   
      
    for (i = 0; i < DMX_HLD_PORTING_CSA_SLOT_CNT; i++)
        if(porting->csa_slot[i].idx == idx)
            return i;

    for (i = 5; i < DMX_HLD_PORTING_CSA_SLOT_CNT; i++)
        if(porting->csa_slot[i].status == ALI_CSA_SLOT_STATUS_IDLE)
        {
            porting->csa_slot[i].idx = idx;
            porting->csa_slot[i].status = ALI_CSA_SLOT_STATUS_READY;
            sec_slot->csa_slot_idx = i;
            return i;
        }   
    return(-1);
}


UINT16 dmx_csa_idx2pid
(
    struct dmx_device *dmx,
    UINT32             idx
)

{

    UINT32                           i;
    UINT16                           pid;
    struct dmx_hld_porting          *porting;
    struct dmx_hld_porting_sec_slot *sec_slot;

    porting = (struct dmx_hld_porting *)dmx->priv;

    pid = 0xFFFF;

    switch(idx & 0x7F)
    {
        case 0:
        {
            pid = porting->v_slot.pid;
        }
        break;

        case 1:
        {
            pid = porting->a_slot.pid;
        }
        break;

        case 2:
        {
            pid = porting->p_slot.pid;
        }
        break;

        case 3:
        {
            pid = porting->ttx_slot.pid;
        }
        break;

        case 4:
        {
            pid = porting->subt_slot.pid;
        }
        break;

        default:
        {
            for (i = 0; i < DMX_HLD_PORTING_SEC_SLOT_CNT; i++)
            {
                sec_slot = &(porting->sec_slot[i]);

                if ((DMX_HLD_PORTING_SEC_SLOT_IDLE != sec_slot->status) &&
                    (sec_slot->id == idx))
                {
                    pid = sec_slot->sec_para->pid;

                    break;
                }
            }
        }
        break;
    }

    return(pid);
}






UINT8 dmx_csa_pid2idx
(
    struct dmx_device *dmx,
    UINT16             pid
)
{
    UINT32                           i;
    struct dmx_hld_porting          *porting;
    struct dmx_hld_porting_sec_slot *sec_slot;

    porting = (struct dmx_hld_porting *)dmx->priv;

    if ((DMX_HLD_PORTING_AV_STREAM_RUN == porting->av_stream_status) &&
        (pid == porting->v_slot.pid))
    {
        return(0);
    }

    if ((DMX_HLD_PORTING_AV_STREAM_RUN == porting->av_stream_status) &&
        (pid == porting->a_slot.pid))
    {
        return(1);
    }

    if ((DMX_HLD_PORTING_AV_STREAM_RUN == porting->av_stream_status) &&
        (pid == porting->p_slot.pid))
    {
        return(2);
    }

    if ((porting->subt_slot.linux_fd > 0) &&
        (pid == porting->subt_slot.pid))
    {

        return(3);
    }

    if ((porting->ttx_slot.linux_fd > 0) &&
        (pid == porting->ttx_slot.pid))
    {
        return(4);
    }

    for (i = 0; i < DMX_HLD_PORTING_SEC_SLOT_CNT; i++)
    {
        sec_slot = &(porting->sec_slot[i]);
     
        if ((DMX_HLD_PORTING_SEC_SLOT_IDLE != sec_slot->status) &&
            (sec_slot->sec_para->pid == pid))
        {
            return(sec_slot->id);
        }
    }

    return(0xFF);
}

INT32 dmx_csa_cw_update(struct dmx_device *dmx, UINT32 idx, UINT16 pid, UINT8 cw_type, UINT32 *cw)
{ 
    KEY_PARAM *CsaParamList;
    pCSA_DEV pCsaDev = (pCSA_DEV)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);
    struct dmx_hld_porting *porting = (struct dmx_hld_porting *)dmx->priv;
    UINT32 tds_cmd;
    INT32  ret;

	//printf("%s,%d,idx:%d,pid:%d,cw_type:%d\n", __FUNCTION__, __LINE__,idx,pid,cw_type);

	
    if (2 == cw_type)
    {
        memcpy(&(porting->csa_slot[idx].cw_pair[8]), cw, ((porting->csa_ver == CSA3)?16:8));
    }

    if (3 == cw_type)
    {
        memcpy(&(porting->csa_slot[idx].cw_pair[0]), cw, ((porting->csa_ver == CSA3)?16:8));
    }    

	CsaParamList = &(porting->csa_slot[idx].CsaParamList);
	
    if ((CSA_1 == porting->csa_ver) || (CSA_2 == porting->csa_ver))
    {
        CsaParamList->key_length = 64;
    }
    else if (CSA_3 == porting->csa_ver)
    {
        CsaParamList->key_length = 128;
    }
            
    CsaParamList->pid_len = 1 ;
    CsaParamList->pid_list = &pid;
    CsaParamList->stream_id = porting->tds_dmx_id ;
    CsaParamList->p_csa_key_info =  (CSA_KEY_PARAM *)&(porting->csa_slot[idx].cw_pair[0]);

#if 0
    UINT32 i;

    for (i = 0; i < CsaParamList->key_length / 16; i++)
    {
		printf("%08x,", ((UINT32 *)(porting->csa_slot[idx].cw_pair))[i]);
    }

    printf("\n");
#endif
    
    if (ALI_CSA_SLOT_STATUS_READY == porting->csa_slot[idx].status)
    {
        CsaParamList->ctr_counter = NULL ;
        CsaParamList->force_mode = 0;
        CsaParamList->handle = 0xFF ;
        CsaParamList->init_vector = NULL ;
        tds_cmd = IO_CREAT_CRYPT_STREAM_CMD;
        porting->csa_slot[idx].status = ALI_CSA_SLOT_STATUS_RUN;

    	//printf("%s, %d\n", __FUNCTION__, __LINE__);

    }
    else if (ALI_CSA_SLOT_STATUS_RUN == porting->csa_slot[idx].status)
    {
    	//printf("%s, %d.\n", __FUNCTION__, __LINE__);

        tds_cmd = IO_KEY_INFO_UPDATE_CMD;	
		
    }else
    {
        //printf("%s, %d.\n", __FUNCTION__, __LINE__);
        return(RET_FAILURE);
    }

	//printf("%s, %d.\n", __FUNCTION__, __LINE__);

    ret = csa_ioctl(pCsaDev, tds_cmd, (UINT32)CsaParamList);
	
    if(ret != RET_SUCCESS)
    {
    	//printf("%s, line:%d\n", __FUNCTION__, __LINE__);
    	return(RET_FAILURE);
    }

	//printf("%s,%d,CsaParamList.handle:%d.\n", __FUNCTION__, __LINE__, CsaParamList->handle);	

    return(RET_SUCCESS);
}  



/* Code for CA. */

RET_CODE dmx_cfg_cw
(
    struct dmx_device *dmx,
    enum DES_STR       str_type,/* Actually means dmx filter index in TDS. */
    UINT8              cw_type,
    UINT32            *cw
)
{
    UINT32                          i;
    INT32                           ret;
    INT32                           csa_fd;
    INT32                           dmx_fd;
    UINT32                          idx;
    INT32                           slot;
    UINT16                          pid;
    struct dmx_hld_porting         *porting;
    struct dec_parse_param p_param;
    
    DMX_DBG_API_PRINT("%s,%d,%s,tick:%d,str_type:%d,cw_type:%d,cw[0]:%x,cw[1]:%x\n",
		              __FUNCTION__, __LINE__,dmx->name,osal_get_tick(),str_type,cw_type,cw[0],cw[1]);

    
    porting = (struct dmx_hld_porting *)dmx->priv;

    if (porting->av_stream_status != DMX_HLD_PORTING_AV_STREAM_RUN)
    {
       return(RET_FAILURE);
    }

    osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

    idx  = str_type;

    if (porting->csa_en == 0)
    {
        pCSA_DEV pCsaDev = (pCSA_DEV)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);\
        CSA_INIT_PARAM param ;
        
        memset(&param, 0, sizeof(CSA_INIT_PARAM));

        param.Dcw[0] = 0;

        param.Dcw[1] = 1;

        param.Dcw[2] = 2;

        param.Dcw[3] = 3;

        param.dma_mode = TS_MODE;

        param.key_from = KEY_FROM_SRAM;

        param.parity_mode = AUTO_PARITY_MODE0 ;

        param.pes_en = 1;  /*not used now*/

        param.scramble_control = 0; /*dont used default CW*/

        param.stream_id = porting->tds_dmx_id;

        param.version = CSA1;
        porting->csa_ver = CSA1;

        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        ret = csa_ioctl(pCsaDev ,IO_INIT_CMD , (UINT32)&param);

        if (ret !=  RET_SUCCESS)
        {        
            osal_mutex_unlock(porting->ioctl_mutex);

            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            return(RET_FAILURE);
        }

        DMX_DBG_API_PRINT("%s,line:%d,ret:%d\n", __FUNCTION__, __LINE__, ret);


        if (porting->see_dev_fd < 0)
        {        
            osal_mutex_unlock(porting->ioctl_mutex);

            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

            return(RET_FAILURE);
        }

        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
/*
        p_param.dec_dev = pCsaDev;
        
        crypto_type = DMX_SEE_CRYPTO_TYPE_CSA;

        ret = ioctl(porting->see_dev_fd , ALI_DMX_SEE_CRYPTO_TYPE_SET, &crypto_type);  

        DMX_DBG_API_PRINT("%s,line:%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
*/
        ioctl(porting->see_dev_fd , ALI_DMX_SEE_CRYPTO_START, 0);  

        DMX_DBG_API_PRINT("%s,line:%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
        porting->csa_en = 1;
    }


    slot = dmx_csa_get_slot(dmx, idx);

    if (slot < 0)
    {
       osal_mutex_unlock(porting->ioctl_mutex);

       DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

       return(RET_FAILURE);
    }
    
    pid = dmx_csa_idx2pid(dmx, idx);
    ret = dmx_csa_cw_update(dmx, slot, pid, cw_type, cw);

    if (ret < 0)
    {        
       osal_mutex_unlock(porting->ioctl_mutex);

       DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

       return(RET_FAILURE);
    }

       //add for update CW
	pDSC_DEV pDscDev = (pDSC_DEV)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	if(NULL == pDscDev)
	{
	    DMX_DBG_API_PRINT("\n %s()---%d---- get DSC Dev failed!\n",__FUNCTION__, __LINE__);
	}
    
	dsc_ioctl(pDscDev, IO_PARSE_DMX_ID_SET_CMD, porting->tds_dmx_id);
    osal_mutex_unlock(porting->ioctl_mutex);

    DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(RET_SUCCESS);
}









void dmx_csa_set_spec
(
    struct dmx_device *dmx,
    enum CSA_SPEC      csa_spec
)
{
    INT32                            ret;
    INT32                            csa_fd;
    struct dmx_hld_porting          *porting;
    pCSA_DEV pCsaDev = (pCSA_DEV)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);
    CSA_INIT_PARAM param ;
   
    porting = (struct dmx_hld_porting *)dmx->priv;

    osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

    param.Dcw[0] = 0 ;
    param.Dcw[1] = 1 ;
    param.Dcw[2] = 2 ;
    param.Dcw[3] = 3 ;
    param.dma_mode = TS_MODE ;
    param.key_from = KEY_FROM_SRAM ;
    param.parity_mode = AUTO_PARITY_MODE0 ;
    param.pes_en = 1;  /*not used now*/
    param.scramble_control = 0 ; /*dont used default CW*/
    param.stream_id = porting->tds_dmx_id ;
    if(DMX_CSA_1_1==csa_spec)
    {
        param.version = CSA1 ;
    }else if(DMX_CSA_2_0==csa_spec)
    {
        param.version = CSA2 ;
    }else
        param.version = CSA3 ;
    
    porting->csa_ver = param.version;
    ret = csa_ioctl(  pCsaDev ,IO_INIT_CMD , (UINT32)&param);
    if(ret != RET_SUCCESS)
    {
        DMX_DBG_API_PRINT("CSA IO_INIT_CMD fail\n");
    }

    osal_mutex_unlock(porting->ioctl_mutex);

    return;
}









RET_CODE dmx_pid_survey_for_csa
(
    struct dmx_device *dmx,
    UINT32             param
)
{
    RET_CODE         ret;
    UINT8            idx;
    UINT16           pid;
    struct io_param *io_param;

    io_param = (struct io_param*)param;

    /*Get Pid which need to find*/
    pid = *((UINT16*)(io_param->io_buff_in));

    idx = dmx_csa_pid2idx(dmx, pid);

    *(io_param->io_buff_out) = idx;

    if (0xFF == idx)
    {
        ret = RET_SUCCESS;    
    }
    else
    {
        ret = RET_SUCCESS;
    }

    return(ret);
}




RET_CODE dmx_csa_new_cw_parity(struct dmx_device *dev, struct cw_parity_t *cw_parity )
{

    return RET_SUCCESS;
}



void dmx_csa_del_cw_parity(struct dmx_device *dev, UINT8 cw_parity_handle)
{
    return;
}



void dmx_csa_set_cw_parity(struct dmx_device *dev, struct cw_sel_t *cw_sel )
{
    return;
}

void dmx_csa_deref_cw_parity(struct dmx_device *dev, UINT8 flt_idx)
{
    return;
}



/* Keep this interface for backward comparitbility.

 * Date: 2009.12.25 by Joy.

 */

RET_CODE dmx_ca_enable
(
    struct dmx_device *dev
)
{

    return(RET_SUCCESS);
}




RET_CODE dmx_open(struct dmx_device *dev)
{
#if (defined _BUILD_UPG_LOADER_WITH_ALISL_ || defined _UPGLOADER_UPGRADE_SUPPORT_) //test share upg lib
	return 0;
#endif

    return(RET_SUCCESS);
}


RET_CODE dmx_close(struct dmx_device *dmx)
{
#if (defined _BUILD_UPG_LOADER_WITH_ALISL_ || defined _UPGLOADER_UPGRADE_SUPPORT_)//test share upg lib
	return 0;
#endif

    struct dmx_hld_porting *porting;

    struct dmx_hld_porting_playback_slot *pb_slot;

    porting = (struct dmx_hld_porting *)dmx->priv;

    pb_slot = &(porting->playback_slot);

	osal_mutex_lock(porting->ioctl_mutex, OSAL_WAIT_FOREVER_TIME);

    if (DMX_HLD_PORTING_DMX_DATA_SRC_PLAYBACK == porting->data_src)
    {
        if (DMX_HLD_PORTING_PLAYBACK_RUN == porting->playback_slot.status)
        {
            //porting->playback_slot.status = DMX_HLD_PORTING_PLAYBACK_READY;
            
            //ioctl(porting->see_dev_fd , ALI_DMX_SEE_PLAYBACK_PRE_PAUSE, 0);
            
            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            pb_slot->need_stop = 1;

            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            //osal_semaphore_capture(pb_slot->pb_run_to_idle_done_sem, 100);
            osal_semaphore_capture(pb_slot->pb_run_to_idle_done_sem, OSAL_WAIT_FOREVER_TIME);

            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            ioctl(pb_slot->linux_fd, ALI_DMX_CHANNEL_FLUSH_BUF, 0);

            pb_slot->need_stop = 0;

            DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
        }

        //osal_task_sleep(100);

        DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    }

	osal_mutex_unlock(porting->ioctl_mutex);
    porting->status = DMX_HLD_PORTING_STATUS_STOP;
    //Wait for sec task & rec task idle
    while(porting->status != DMX_HLD_PORTING_STATUS_IDLE)
        osal_task_sleep(10);
    return(RET_SUCCESS);

}



RET_CODE dmx_reg_dev
(
    UINT8                         *name,
    enum DMX_HLD_PORTING_DATA_SRC  data_src,
    UINT8                         *linux_dmx_path,
    UINT8                         *linux_pb_in_dev_path,
    struct dmx_hld_porting        *porting,
    INT32                          see_dev_fd,
    UINT16                         tds_dmx_id      
)
{
#if (defined _BUILD_UPG_LOADER_WITH_ALISL_ || defined _UPGLOADER_UPGRADE_SUPPORT_) //test share upg lib
	return 0;
#endif

    UINT32                                i;
    INT32                                 ret;
    struct dmx_device                    *dmx;
    struct dmx_hld_porting_sec_slot      *sec_slot;
    OSAL_T_CTSK                           t_ctsk;
    struct dmx_hld_porting_avp_slot      *avp_slot;
    struct dmx_hld_porting_playback_slot *pb_slot;
    struct dmx_hld_porting_rec_slot      *rec_slot;
    struct dmx_hld_porting_new_service_slot *new_service_slot;

    porting->ioctl_mutex = osal_mutex_create();

    if (OSAL_INVALID_ID == porting->ioctl_mutex)
    {
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);  
    }

    /* Reigister dev in TDS. */
    dmx = dev_alloc(name, HLD_DEV_TYPE_DMX, sizeof(struct dmx_device));

    if(NULL == dmx)
    {
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);  
    }  

    DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    dmx->priv = (void *)porting;

    if (strlen(linux_dmx_path) > DMX_HLD_PORTING_MAX_LINUX_DEV_PATH_LEN)
    {
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        ASSERT(1);
    }

    memcpy(porting->linux_dmx_path, linux_dmx_path, strlen(linux_dmx_path));

    DMX_DBG_API_PRINT("%s, line:%d, %s\n", __FUNCTION__, __LINE__,
                      porting->linux_dmx_path);

    /* Section slots. */
    porting->sec_hit_flag = osal_flag_create(0);

    if (OSAL_INVALID_ID == porting->sec_hit_flag)
    {
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);        
    }

    for (i = 0; i < DMX_HLD_PORTING_SEC_SLOT_CNT; i++)
    {
        sec_slot = &(porting->sec_slot[i]);

        sec_slot->status = DMX_HLD_PORTING_SEC_SLOT_IDLE;

        sec_slot->id = i + 5;

        sec_slot->porting = (UINT8 *)porting;

        sec_slot->linux_fd = -1;


        sec_slot->porting = (UINT8 *)porting;
    }
    for (i = 0; i < DMX_HLD_PORTING_CSA_SLOT_CNT; i++)
    {
        porting->csa_slot[i].status = ALI_CSA_SLOT_STATUS_IDLE;
    }
    
    for (i = 0; i < DMX_OTHER_PORTING_CNT; i++)
    {

        porting->other_slot[i].linux_fd = -1;
        porting->other_slot[i].pid = 0x1FFF;
        porting->other_slot[i].csa_slot_idx= i+6;
        porting->csa_slot[i+6].idx = i+6;
    }
    
    for (i = 0; i < DMX_HLD_PORTING_NEW_SERVICE_SLOT_CNT; i++)
    {
        new_service_slot = &(porting->new_service_slot[i]);
        memset(new_service_slot,0,sizeof(struct dmx_hld_porting_new_service_slot));
        
        new_service_slot->status = DMX_HLD_PORTING_NEW_SERVICE_SLOT_IDLE;
        new_service_slot->id =i;
        new_service_slot->linux_fd = -1;
        new_service_slot->buf = NULL;
        new_service_slot->buf_len = 0;
        new_service_slot->buf_wr = 0;
        new_service_slot->pid = 0x1FFF;
        

    }
    
    /* A/V/P play slot. */
    porting->v_slot.linux_fd = -1;
    porting->v_slot.pid = 0x1FFF;
    porting->v_slot.csa_slot_idx = 0;
    porting->csa_slot[0].idx = 0;
    
    porting->a_slot.linux_fd = -1;
    porting->a_slot.pid = 0x1FFF;
    porting->a_slot.csa_slot_idx = 1;
    porting->csa_slot[1].idx = 1;

    porting->p_slot.linux_fd = -1;
    porting->p_slot.pid = 0x1FFF;
    porting->p_slot.csa_slot_idx = 2;
    porting->csa_slot[2].idx = 2;

    porting->ad_slot.linux_fd = -1;
    porting->ad_slot.pid = 0x1FFF;
    porting->ad_slot.csa_slot_idx = 3;
    porting->csa_slot[3].idx = 3;

    porting->see_dev_fd = see_dev_fd;

    porting->av_stream_status = DMX_HLD_PORTING_AV_STREAM_READY;

    porting->timeshift_status = DMX_HLD_PORTING_TIMESHIFT_NONE;

    porting->p_DeEnconfig = NULL;
     /* TTX slot. */

    porting->ttx_slot.linux_fd = -1;




     /* Subtitle slot. */

    porting->subt_slot.linux_fd = -1;

	porting->m_demux_data_flow = DEMUX_DATA_TO_SEE_CPU_KERNEL_SPACE;

	porting->m_demux_data_flow_ttx = DEMUX_DATA_TO_SEE_CPU_KERNEL_SPACE;

	porting->m_demux_data_flow_subt = DEMUX_DATA_TO_SEE_CPU_KERNEL_SPACE;

	porting->m_demux_data_flow_dcii_subt = DEMUX_DATA_TO_SEE_CPU_KERNEL_SPACE;


#if 1
    /* Rec slot. */
    rec_slot = &(porting->rec_slot[0]);

    memset(rec_slot, 0, sizeof(porting->rec_slot));

    for (i = 0; i < DMX_HLD_PORTING_REC_SLOT_CNT; i++)
    {
        rec_slot[i].linux_fd = -1;
    }
#endif

    /* Add this device to queue */
    if(dev_register(dmx) != SUCCESS)
    {
        dev_free(dmx);

        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);  
    }

    /* Create section dispach task. */
    t_ctsk.stksz    =   0x1000;
    t_ctsk.quantum  =   10;
    t_ctsk.itskpri  =   OSAL_PRI_HIGH;
    t_ctsk.name[0]  =   'D';
    t_ctsk.name[1]  =   'S';
    t_ctsk.name[2]  =   'E';
    t_ctsk.task = (FP)dmx_sec_retrieve_task;
    t_ctsk.para1 = (UINT32)dmx;
    t_ctsk.para2 = 0;

    porting->sec_dispath_task_id = osal_task_create(&t_ctsk);

    if(OSAL_INVALID_ID == porting->sec_dispath_task_id)
    {
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
    }

    /* Create PES dispach task. */
    t_ctsk.stksz    =   0x1000;
    t_ctsk.quantum  =   10;
    t_ctsk.itskpri  =   OSAL_PRI_HIGH;
    t_ctsk.name[0]  =   'P';
    t_ctsk.name[1]  =   'S';
    t_ctsk.name[2]  =   'E';
    t_ctsk.task = (FP)dmx_pes_retrieve_task;
    t_ctsk.para1 = (UINT32)dmx;
    t_ctsk.para2 = 0;

    porting->pes_dispatch_task_id = osal_task_create(&t_ctsk);

    if(OSAL_INVALID_ID == porting->pes_dispatch_task_id)
    {
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
    } 

    /* Create recording task. */
    t_ctsk.stksz    =   0x1000;
    t_ctsk.quantum  =   10;
    t_ctsk.itskpri  =   OSAL_PRI_HIGH;
    t_ctsk.name[0]  =   'R';
    t_ctsk.name[1]  =   'E';
    t_ctsk.name[2]  =   'C';
    t_ctsk.task = (FP)dmx_rec_retrieve_task;
    t_ctsk.para1 = (UINT32)dmx;
    t_ctsk.para2 = 0;

    porting->rec_task_id = osal_task_create(&t_ctsk);

    if(OSAL_INVALID_ID == porting->rec_task_id)
    {
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
    }


    /* Create new_service task. */
    t_ctsk.stksz    =   0x1000;
    t_ctsk.quantum  =   10;
    t_ctsk.itskpri  =   OSAL_PRI_HIGH;
    t_ctsk.name[0]  =   'N';
    t_ctsk.name[1]  =   'S';
    t_ctsk.name[2]  =   'V';
    t_ctsk.task = (FP)dmx_new_service_retrieve_task;
    t_ctsk.para1 = (UINT32)dmx;
    t_ctsk.para2 = 0;

    porting->new_service_task_id = osal_task_create(&t_ctsk);

    if(OSAL_INVALID_ID == porting->new_service_task_id)
    {
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
    }


    DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    /* Playback slot. */
    if (DMX_HLD_PORTING_DMX_DATA_SRC_PLAYBACK == data_src)
    {
        pb_slot = &(porting->playback_slot);
    
        memset(pb_slot, 0, sizeof(struct dmx_hld_porting_playback_slot));

        memcpy(pb_slot->linux_dev_path, linux_pb_in_dev_path, 
               strlen(linux_pb_in_dev_path));

        /* Open playback in dev. */
        pb_slot->linux_fd = open(pb_slot->linux_dev_path, O_RDWR);
    
        if (pb_slot->linux_fd < 0)
        {        
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
    
            return(RET_FAILURE);
        }

        DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        pb_slot->pb_run_to_idle_done_sem = osal_semaphore_create(0);

        DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);


        if (OSAL_INVALID_ID == pb_slot->pb_run_to_idle_done_sem)
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
    
            return(RET_FAILURE);
        }

        DMX_DBG_API_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        /* Create playback task. */
        t_ctsk.stksz    =   0x1000;
        t_ctsk.quantum  =   10;
        t_ctsk.itskpri  =   OSAL_PRI_HIGH;
        t_ctsk.name[0]  =   'D';
        t_ctsk.name[1]  =   'P';
        t_ctsk.name[2]  =   'L';
        t_ctsk.task = (FP)dmx_playback_task;
        t_ctsk.para1 = (UINT32)dmx;
        t_ctsk.para2 = 0;
    
        porting->playback_task_id = osal_task_create(&t_ctsk);
    
        if(OSAL_INVALID_ID == porting->playback_task_id)
        {
            DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
        }
    }

    porting->data_src = data_src;

    /* HW control slot. */
    porting->hw_ctl_slot.linux_fd = open(porting->linux_dmx_path, O_RDONLY);

    if (porting->hw_ctl_slot.linux_fd < 0)
    {        
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }    

    porting->tds_dmx_id = tds_dmx_id;

    return(RET_SUCCESS);
}












RET_CODE dmx_m36_attach(struct dmx_feature_config *config)
{
#if (defined _BUILD_UPG_LOADER_WITH_ALISL_ || defined _UPGLOADER_UPGRADE_SUPPORT_)//test share upg lib
	return 0;
#endif

    UINT32             i;
    struct dmx_device *dmx;
    INT32              see_dev_fd;

    DMX_DBG_API_PRINT("======dmx_m36_attach() go=========\n");

    /* Opne SEE. */
    see_dev_fd = open("/dev/ali_m36_dmx_see_0", O_RDWR);

    if (see_dev_fd < 0)
    {        
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }

    memset(g_dmx_hld_porting, 0, sizeof(g_dmx_hld_porting));

    dmx_reg_dev("DMX_S3601_0", DMX_HLD_PORTING_DMX_DATA_SRC_LIVE, DMX_0_LINUX_DEV_PATH, NULL, &g_dmx_hld_porting[0], see_dev_fd, 0);

    dmx_reg_dev("DMX_S3601_1", DMX_HLD_PORTING_DMX_DATA_SRC_LIVE, DMX_1_LINUX_DEV_PATH, NULL, &g_dmx_hld_porting[1], see_dev_fd, 1);

    dmx_reg_dev("DMX_S3601_2", DMX_HLD_PORTING_DMX_DATA_SRC_PLAYBACK, DMX_2_LINUX_DEV_PATH, DMX_PB_IN_LINUX_DEV_PATH, &g_dmx_hld_porting[2], see_dev_fd, 2);

    DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    for (i = 0; i < 3; i++)
    {
        dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, i);
    
        DMX_PRINTF("dmx->name:%s\n", dmx->name);
    }

    return(RET_SUCCESS);
}


#if 0
RET_CODE dmx_m36_dettach
(

)
{



}
#endif















/*<========================TSG porting=============================== */
/* For TSG porting. */

enum ali_porting_tsg_status 
{
    ALI_PORTING_TSG_STATUS_IDLE = 0,
    ALI_PORTING_TSG_STATUS_RUN
};

struct ali_porting_tsg
{
    enum ali_porting_tsg_status status;

    UINT8 output_clk;

    INT32 linux_fd;
};

struct ali_porting_tsg g_ali_porting_tsg[1];

#if 0
#define TSG_DBG_API_PRINT printf
#else
#define TSG_DBG_API_PRINT(...)
#endif





void tsg_wait
(
    UINT32 xfer_id
)
{
    TSG_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    return;
}


void tsg_set_clk_async
(
    UINT8 clk_sel
)
{
    struct ali_porting_tsg *tsg;

    tsg = &g_ali_porting_tsg[0];

    //TSG_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);
    //DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    if (ALI_PORTING_TSG_STATUS_RUN != tsg->status)
    {
        return;
    }

    ioctl(tsg->linux_fd, ALI_TSG_OUTPUT_CLK_SET, &clk_sel);

    TSG_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    return;
}



void tsg_set_clk
(
    UINT8 clk_sel
)
{
    TSG_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(tsg_set_clk_async(clk_sel));
}

RET_CODE tsg_check_buf_busy
(
    UINT32 buf_addr,
    UINT32 buf_len
)
{
    //TSG_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(!RET_SUCCESS);   
}

/* Acturally means getting total ts pkt cnt stored in TSG buffer which
 * has not been sent out by hw yet.
 */
UINT32 tsg_check_remain_buf
(
    void
)
{  
    UINT32                  cur_data_len;
    UINT32                  pkt_cnt;
    struct ali_porting_tsg *tsg;

    //TSG_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    tsg = &g_ali_porting_tsg[0];

    if (ALI_PORTING_TSG_STATUS_RUN != tsg->status)
    {
        return(RET_FAILURE);
    }

    ioctl(tsg->linux_fd, ALI_TSG_GET_CUR_DATA_LEN, &cur_data_len);

    pkt_cnt = cur_data_len / 188;

    TSG_DBG_API_PRINT("%s,line:%d,pkt:%d\n", __FUNCTION__, __LINE__, pkt_cnt);

    return(pkt_cnt);
}

void tsg_start
(
    UINT32 bitrate
)
{
    struct ali_porting_tsg *tsg;

    TSG_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    tsg = &g_ali_porting_tsg[0];

    if (ALI_PORTING_TSG_STATUS_RUN != tsg->status)
    {
        return;
    }

    ioctl(tsg->linux_fd, ALI_TSG_NULL_PKT_INSERTION_START, &bitrate);

    return;
}


void tsg_stop
(
    void
)
{
    struct ali_porting_tsg *tsg;

    tsg = &g_ali_porting_tsg[0];

    TSG_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    if (ALI_PORTING_TSG_STATUS_RUN != tsg->status)
    {
        return;
    }

    ioctl(tsg->linux_fd, ALI_TSG_NULL_PKT_INSERTION_STOP);

    return;
}


UINT32 g_dmx_test_last_ts_conti_cnt = 0;



UINT32 tsg_transfer
(
    void   *addr,
    UINT16  pkt_cnt,
    UINT8   sync,
    UINT32  bfc
)
{
    struct ali_porting_tsg *tsg;
    UINT32                  cur_conti_cnt;
    UINT32                  exp_conti_cnt;
    UINT32                  i;

#ifdef CONFIG_FAST_COPY
       struct tsg_fast_copy_param    param;
#endif
    TSG_DBG_API_PRINT("%s,%d,pkt_cnt:%d\n", __FUNCTION__, __LINE__, pkt_cnt);

    tsg = &g_ali_porting_tsg[0];

    if (ALI_PORTING_TSG_STATUS_RUN != tsg->status)
    {
        TSG_DBG_API_PRINT("%s,%d,pkt_cnt:%d\n", __FUNCTION__, __LINE__, pkt_cnt);

        return(RET_FAILURE);
    }

#if 0
    UINT32  pid;
    UINT8  *pkt_addr;

    pkt_addr = (UINT8 *)addr;

    DMX_DBG_API_PRINT("\n", cur_conti_cnt);

    for (i = 0; i < pkt_cnt; i++)
    {
        pid = ((pkt_addr[1] & 0x1F) << 8) | pkt_addr[2];

        if (0x205 == pid)
        {
            cur_conti_cnt = pkt_addr[3] & 0x0F;
    
            DMX_DBG_API_PRINT("%02d ", cur_conti_cnt);
    
            exp_conti_cnt = (g_dmx_test_last_ts_conti_cnt + 1) & 0x0F;
    
            if ((exp_conti_cnt != cur_conti_cnt) &&
                (g_dmx_test_last_ts_conti_cnt != cur_conti_cnt))
            {
                DMX_DBG_API_PRINT("<exp:%2d,cur:%2d>", exp_conti_cnt, cur_conti_cnt);
            }
    
            g_dmx_test_last_ts_conti_cnt = cur_conti_cnt;
        }

        pkt_addr += 188;
    }

    DMX_DBG_API_PRINT("\n", cur_conti_cnt);

#endif

#if 0//def CONFIG_FAST_COPY
    param.data = addr;
    param.len = pkt_cnt * 188;
    ioctl(tsg->linux_fd, ALI_TSG_WRITE_FSTCPY, &param);
#else
	if (1 == bfc)
	{
	#ifdef CONFIG_FAST_COPY
		param.data = addr;
    	param.len = pkt_cnt * 188;
    	ioctl(tsg->linux_fd, ALI_TSG_WRITE_FSTCPY, &param);
	#endif	
	}
	else
    	write(tsg->linux_fd, addr, pkt_cnt * 188);
#endif

    TSG_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(RET_SUCCESS);
}



RET_CODE tsg_exit
(
    void
)
{
    struct ali_porting_tsg *tsg;

    tsg = &g_ali_porting_tsg[0];

    if (ALI_PORTING_TSG_STATUS_RUN != tsg->status)
    {
        return(RET_FAILURE);
    }

    /* close TSG. */
    if (tsg->linux_fd >= 0)
    {        
        DMX_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        close(tsg->linux_fd);

        tsg->linux_fd = -1;

        return(RET_FAILURE);
    }

    tsg->status = ALI_PORTING_TSG_STATUS_IDLE;

    return(RET_SUCCESS);
}




RET_CODE tsg_init
(
    UINT8 output_lk
)
{
    struct ali_porting_tsg *tsg;

    tsg = &g_ali_porting_tsg[0];

    if (ALI_PORTING_TSG_STATUS_IDLE != tsg->status)
    {
        return(RET_FAILURE);
    }

    /* Open TSG. */
    tsg->linux_fd = open("/dev/ali_m36_tsg_0", O_RDWR);

    if (tsg->linux_fd < 0)
    {        
        TSG_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(RET_FAILURE);
    }

    tsg->status = ALI_PORTING_TSG_STATUS_RUN;

    TSG_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    tsg_set_clk_async(output_lk);

    TSG_DBG_API_PRINT("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(RET_SUCCESS);
}


/*<========================TSI porting=============================== */
/* For TSG porting. */


RET_CODE tsi_inputid_tds_to_linux
(
    UINT32                 tds_input_id,
    enum ali_tsi_input_id *linux_input_id
)
{
    RET_CODE ret;

    ret = RET_SUCCESS;

    switch (tds_input_id)
    {
        case TSI_SPI_0:
        {
            *linux_input_id = ALI_TSI_INPUT_SPI_0;
        }
        break;

        case TSI_SPI_1:
        {
            *linux_input_id = ALI_TSI_INPUT_SPI_1;
        }
        break;

        case TSI_SPI_TSG:
        {
            *linux_input_id = ALI_TSI_INPUT_TSG;
        }
        break;

		case TSI_SPI_3:
		{
			*linux_input_id = ALI_TSI_INPUT_SPI_3;
		}
		break;
        case TSI_SSI_0:
        {
            *linux_input_id = ALI_TSI_INPUT_SSI_0;
        }
        break;

        case TSI_SSI_1:
        {
            *linux_input_id = ALI_TSI_INPUT_SSI_1;
        }
        break;

        case TSI_SSI_2:
        {
            *linux_input_id = ALI_TSI_INPUT_SSI_2;
        }
        break;

        case TSI_SSI_3:
        {
            *linux_input_id = ALI_TSI_INPUT_SSI_3;
        }
        break;

        case TSI_SSI2B_0:
        {
            *linux_input_id = ALI_TSI_INPUT_SPI2B_0;
        }
        break;

        case TSI_SSI2B_1:
        {
            *linux_input_id = ALI_TSI_INPUT_SPI2B_1;
        }
        break;

        case TSI_SSI4B_0:
        {
            *linux_input_id = ALI_TSI_INPUT_SPI4B_0;
        }
        break;

        case TSI_SSI4B_1:
        {
            *linux_input_id = ALI_TSI_INPUT_SPI4B_1;
        }
        break;

        case PARA_MODE_SRC:
        {
            *linux_input_id = ALI_TSI_INPUT_PARA;
        }
        break;

        default:
        {
            //printf("%s, %d\n", __FUNCTION__, __LINE__);

            ret = RET_FAILURE;
        }
        break;
    }

    return(ret);
}


RET_CODE tsi_inputid_linux_to_tds
(
    enum ali_tsi_input_id  linux_input_id, 
    UINT8                 *tds_input_id
)
{
    RET_CODE ret;

    ret = RET_SUCCESS;

    switch (linux_input_id)
    {
        case ALI_TSI_INPUT_SPI_0:
        {
            *tds_input_id = TSI_SPI_0;
        }
        break;

        case ALI_TSI_INPUT_SPI_1:
        {
            *tds_input_id = TSI_SPI_1;
        }
        break;

        case ALI_TSI_INPUT_TSG:
        {
            *tds_input_id = TSI_SPI_TSG;
        }
        break;

        case ALI_TSI_INPUT_SSI_0:
        {
            *tds_input_id = TSI_SSI_0;
        }
        break;

        case ALI_TSI_INPUT_SSI_1:
        {
            *tds_input_id = TSI_SSI_1;
        }
        break;

        case ALI_TSI_INPUT_SSI_2:
        {
            *tds_input_id = TSI_SSI_2;
        }
        break;

        case ALI_TSI_INPUT_SSI_3:
        {
            *tds_input_id = TSI_SSI_3;
        }
        break;

        case ALI_TSI_INPUT_SPI2B_0:
        {
            *tds_input_id = TSI_SSI2B_0;
        }
        break;

        case ALI_TSI_INPUT_SPI2B_1:
        {
            *tds_input_id = TSI_SSI2B_1;
        }
        break;

        case ALI_TSI_INPUT_SPI4B_0:
        {
            *tds_input_id = TSI_SSI4B_0;
        }
        break;

        case ALI_TSI_INPUT_SPI4B_1:
        {
            *tds_input_id = TSI_SSI4B_1;
        }
        break;

        case ALI_TSI_INPUT_PARA:
        {
            *tds_input_id = PARA_MODE_SRC;
        }
        break;

        default:
        {
           // printf("%s, %d\n", __FUNCTION__, __LINE__);

            ret = RET_FAILURE;
        }
        break;
    }

    return(ret);
}








RET_CODE tsi_s3602_chg_tsiid
(
    INT32 raw_tsi,
    INT32 chg_tsi,
    INT32 chg_en
)
{
    INT32 tsi_linux_fd;

    //printf("\n\n\n\n\%s,%d\n\n\n\n", __FUNCTION__, __LINE__);

    tsi_linux_fd = open("/dev/ali_m36_tsi_0", O_WRONLY);

    if (tsi_linux_fd  < 0)
    {
        //printf("%s,%d\n", __FUNCTION__, __LINE__);

        return;
    }

    ioctl(tsi_linux_fd, ALI_TSI_CI_SPI_0_1_SWAP, &chg_en);

    close(tsi_linux_fd);

    //printf("%s,%d\n", __FUNCTION__, __LINE__);

    return;
}


void tsi_s3602_parallel_mode_set
(
    INT32 para
)
{
    INT32 tsi_linux_fd;

    //printf("\n\n\n\n\%s,%d\n\n\n\n", __FUNCTION__, __LINE__);

    tsi_linux_fd = open("/dev/ali_m36_tsi_0", O_WRONLY);

    if (tsi_linux_fd  < 0)
    {
        //printf("%s,%d\n", __FUNCTION__, __LINE__);

        return;
    }

    if(MODE_PARALLEL == para)
		para = ALI_TSI_CI_LINK_PARALLEL;
    else if(MODE_CHAIN == para)
        para = ALI_TSI_CI_LINK_CHAIN;

    ioctl(tsi_linux_fd, ALI_TSI_CI_LINK_MODE_SET, para);

    close(tsi_linux_fd);

    //printf("%s,%d\n", __FUNCTION__, __LINE__);

    return;

}



void tsi_s3602_para_src_select
(
    INT32  ci_id,
    UINT32 chg_en
)
{
	INT32 tsi_linux_fd;
    
    struct ali_tsi_ci_bypass_set_param  input;

	tsi_linux_fd = open("/dev/ali_m36_tsi_0", O_WRONLY);

	if (tsi_linux_fd  < 0)
	{
	    //printf("%s,%d\n", __FUNCTION__, __LINE__);

	    return;
	}


	//printf("%s,%d\n", __FUNCTION__, __LINE__);

    input.ci_id = ci_id;

	input.is_bypass = chg_en;

	ioctl(tsi_linux_fd, ALI_TSI_CI_BYPASS_SET, &input);

	close(tsi_linux_fd);

	//printf("%s,%d\n", __FUNCTION__, __LINE__);

}




/* Set TSI input attribute. */
void tsi_s3602_mode_set
(
    INT32 input_id,
    UINT8 attrib
)
{
    INT32                          tsi_linux_fd;
    struct ali_tsi_input_set_param input;

    //printf("\n\n\n\n\%s,%d\n\n\n\n", __FUNCTION__, __LINE__);

    tsi_linux_fd = open("/dev/ali_m36_tsi_0", O_RDONLY);

    if (tsi_linux_fd  < 0)
    {
       // printf("%s,%d\n", __FUNCTION__, __LINE__);

        return;
    }

    //printf("%s,%d\n", __FUNCTION__, __LINE__);

    if (RET_SUCCESS != tsi_inputid_tds_to_linux(input_id, &input.id))
    {
      //  printf("%s,%d\n", __FUNCTION__, __LINE__);
        close(tsi_linux_fd);
        return;
    }

    //printf("%s,%d\n", __FUNCTION__, __LINE__);

    input.attribute = attrib;

    ioctl(tsi_linux_fd, ALI_TSI_INPUT_SET, &input);

    close(tsi_linux_fd);

    //printf("%s,%d\n", __FUNCTION__, __LINE__);

    return;
}



void tsi_s3602_select
(
    int channel_id,
    int input_id
)
{
    INT32                            tsi_linux_fd;
    struct ali_tsi_channel_set_param channel;

    tsi_linux_fd = open("/dev/ali_m36_tsi_0", O_RDONLY);

    //printf("%s,%d,ch:%d,input_id:%d\n", __FUNCTION__, __LINE__,
           //channel_id, input_id);

    if (tsi_linux_fd  < 0)
    {
     //   printf("%s,%d\n", __FUNCTION__, __LINE__);

        return;
    }

    /* input_id linux definition same as TDS. */
#if 0
    channel.input_id = input_id;
#else
    if (RET_SUCCESS != tsi_inputid_tds_to_linux(input_id, &channel.input_id))
    {
       // printf("%s,%d\n", __FUNCTION__, __LINE__);
        close(tsi_linux_fd);
        return;
    }
#endif

    //printf("%s,%d\n", __FUNCTION__, __LINE__);

    if (TSI_TS_A == (channel_id & TSI_TS_A))
    {
       channel.channel_id = ALI_TSI_CHANNEL_0;

       //printf("%s,%d,channel_id:0x%x,TSI_TS_A:0x%x,&:0x%x,input_id:%d\n",
              //__FUNCTION__, __LINE__, channel_id, TSI_TS_A, 
              //channel_id & TSI_TS_A, channel.input_id);

       ioctl(tsi_linux_fd, ALI_TSI_CHANNEL_SET, &channel);
    }

    if (TSI_TS_B == (channel_id & TSI_TS_B))
    {
       channel.channel_id = ALI_TSI_CHANNEL_1;

       //printf("%s,%d,channel_id:0x%x,TSI_TS_B:0x%x,&:0x%x\n",
              //__FUNCTION__, __LINE__, channel_id, TSI_TS_B, channel_id & TSI_TS_B);

       ioctl(tsi_linux_fd, ALI_TSI_CHANNEL_SET, &channel);
    }

    //printf("%s,%d\n", __FUNCTION__, __LINE__);

    close(tsi_linux_fd);

    //printf("%s,%d\n", __FUNCTION__, __LINE__);

    return;
}

/* TS channel select. */
void tsi_s3602_dmx_src_select
(
    int dmx_id,
    int channel_id
)
{
    INT32                           tsi_linux_fd;
    struct ali_tsi_output_set_param output;

    //printf("%s,%d\n", __FUNCTION__, __LINE__);

    tsi_linux_fd = open("/dev/ali_m36_tsi_0", O_RDONLY);

    if (tsi_linux_fd  < 0)
    {
        //printf("%s,%d\n", __FUNCTION__, __LINE__);

        return;
    }

    if (TSI_TS_A == channel_id)
    {
        output.channel_id = ALI_TSI_CHANNEL_0;
    }
    else if (TSI_TS_B == channel_id)
    {
        output.channel_id = ALI_TSI_CHANNEL_1;
    }
    else
    {
        //printf("%s,%d\n", __FUNCTION__, __LINE__);
        close(tsi_linux_fd);
        return;
    }

    //printf("%s,%d\n", __FUNCTION__, __LINE__);

    if (TSI_DMX_0 == (dmx_id & TSI_DMX_0))
    {
       output.output_id = ALI_TSI_OUTPUT_DMX_0;

       ioctl(tsi_linux_fd, ALI_TSI_OUTPUT_SET, &output);
    }

    if (TSI_DMX_1 == (dmx_id & TSI_DMX_1))
    {
       output.output_id = ALI_TSI_OUTPUT_DMX_1;

       ioctl(tsi_linux_fd, ALI_TSI_OUTPUT_SET, &output);
    }

    close(tsi_linux_fd);

    //printf("%s,%d\n", __FUNCTION__, __LINE__);

    return;
}


/* TS output select. */
void tsi_s3602_check_dmx_src
(
    int    dmx_id,
    UINT8 *channel_id,
    UINT8 *input_id,
    UINT8 *ci_mode
)
{
    INT32                            tsi_linux_fd;
    struct ali_tsi_input_set_param   input;
    struct ali_tsi_channel_set_param channel;
    struct ali_tsi_output_set_param  output;
    enum ali_tsi_ci_link_mode        ci_link_mode;
 
    tsi_linux_fd = open("/dev/ali_m36_tsi_0", O_RDONLY);

    if (tsi_linux_fd  < 0)
    {
        //printf("%s,%d\n", __FUNCTION__, __LINE__);

        return;
    }

    if (TSI_DMX_0 == dmx_id)
    {
        output.output_id = ALI_TSI_OUTPUT_DMX_0;
    }
    else if (TSI_DMX_1 == dmx_id)
    {
        output.output_id = ALI_TSI_OUTPUT_DMX_1;
    }
    else
    {
       // printf("%s,%d\n", __FUNCTION__, __LINE__);
        close(tsi_linux_fd);
        return;
    }

    ioctl(tsi_linux_fd, ALI_TSI_OUTPUT_GET, &output);

    channel.channel_id = output.channel_id;

    ioctl(tsi_linux_fd, ALI_TSI_CHANNEL_GET, &channel);

    input.id = channel.input_id;

    ioctl(tsi_linux_fd, ALI_TSI_INPUT_GET, &input);

    ioctl(tsi_linux_fd, ALI_TSI_CI_LINK_MODE_GET, &ci_link_mode);

    if (ALI_TSI_CHANNEL_0 == output.channel_id)
    {
        *channel_id = TSI_TS_A;
    }
    else
    {
        *channel_id = TSI_TS_B;
    }

    if (ALI_TSI_CI_LINK_CHAIN == ci_link_mode)
    {
        *ci_mode = MODE_CHAIN;
    }
    else
    {
        *ci_mode = MODE_PARALLEL;
    }

    tsi_inputid_linux_to_tds(channel.input_id, input_id);
    close(tsi_linux_fd);
    return;   
}


