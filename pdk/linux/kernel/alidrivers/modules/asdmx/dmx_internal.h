/******************************************************************************
*
*       ALI IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR ALI DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, ALI IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  ALI EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2008-2009 ALI Inc.
*       All rights reserved.
*
******************************************************************************/

/*****************************************************************************/
/**
 *
 * file dmx_hal_task.c
 *
 * Implements demux parsing task. Dispatch TS pakcet to upper layer.
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who      Date     Changes
 * ----- -------  -------- --------------------------------------------------
 * 1.00  Joy Chu  09/03/11 First release
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include <asm/mach-ali/typedef.h>
#include <ali_basic_common.h>
#include <linux/spinlock.h>

#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>

#include <linux/ali_rpc.h>
#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_hld.h>
#include <ali_dmx_common.h>
#include <linux/dvb/Ali_DmxLib.h>
#include "dmx_hw.h"

struct Ali_DmxAvStreamParam
{  
    __u32 AudioPid;
    __u32 VideoPid;
    __u32 PcrPid;
};

#if 1
#define DMX_INIT_DEBUG(...)  
#else
#define DMX_INIT_DEBUG printk
#endif

#if 1
#define DMX_HW_DEBUG(...)
#else
#define DMX_HW_DEBUG printk
#endif

#if 1
#define DMX_TS_SERV_DEBUG(...)
#else
#define DMX_TS_SERV_DEBUG printk
#endif


#if 1
#define DMX_API_DEBUG(...)
#else
#define DMX_API_DEBUG printk
#endif

#if 1
#define DMX_SEE_DEBUG(...)  
#else
#define DMX_SEE_DEBUG printk
#endif


#if 1
#define DMX_PLAYBACK_DEBUG(...)  
#else
#define DMX_PLAYBACK_DEBUG printk
#endif

#if 1
#define DMX_LINUX_API_DEBUG(...)  
#else
#define DMX_LINUX_API_DEBUG printk
#endif





#define dmx_assert(...)

#define MEMCPY memcpy



#define DMX_DATA_POOL_NODE_SIZE   1024//4136 /* 22 * 188bytes */
#define DMX_DATA_POOL_NODE_CNT    1600


#define DMX_SGDMA_STARTCODE_TMP_BUF_LEN 0xFA000 /* 100K */

#define DMX_ENC_BLK_LEN (256*8*188)

#define DMX_ENC_UNIT_LEN (256*188)

#define MAX_CA_RECORD_CHANNEL 4
#if 0
/* For M3602. */
enum dmx_m33_reg_offset
{
    /* DEMUX control register */
    OFFSET_GLOBAL_CONTROL               = 0x00,    

    /* Filter Fifo Control Register */  
    OFFSET_FILTER_FIFO_CONTROL          = 0x10,
    /* DMA FIFO 0 control register */          
    OFFSET_DMA_FIFO_0_CONTROL           = 0x14,    
    /* DMA FIFO 1 control register */   
    OFFSET_DMA_FIFO_1_CONTROL           = 0x18,

    /* PCR content Register0*/      
    OFFSET_PCR_CONTENT_0                = 0x30,    
    /* PCR content Register1*/  
    OFFSET_PCR_CONTENT_1                = 0x34,

    /* Channel mask registers */     
    OFFSET_FILTER_EN_MASK_0             = 0x40,   

    OFFSET_FILTER_EN_MASK_1             = 0x44,   

    /* Channel de-scrambler mask register */      
    OFFSET_FILTER_DESCRAM_EN_MASK_0     = 0x50,

    OFFSET_FILTER_DESCRAM_EN_MASK_1     = 0x54,
 
    /* Filter mask & value & CW control register */      
    OFFSET_M_V_C_PORT_CONTROL           = 0x60,    
    /* Filter mask & value & CW data register 0 */  
    OFFSET_M_V_C_PORT_DATA_0            = 0x64,
    /* Filter mask & value & CW data register 1 */      
    OFFSET_M_V_C_PORT_DATA_1            = 0x68,    
    /* Filter mask & value & CW data register 2 */  
    OFFSET_M_V_C_PORT_DATA_2            = 0x6C,
    /* Filter mask & value & CW data register 3 */      
    OFFSET_M_V_C_PORT_DATA_3            = 0x70, 
   
    /* TODO: channel CW select registers. */

    /* Default CW data register 0 */    
    OFFSET_DEFULT_CW_DATA_0             = 0xA0,    
    /* Default CW data register 1 */    
    OFFSET_DEFULT_CW_DATA_1             = 0xA4,

    /* DMA segment start address register */    
    OFFSET_TS_BUF_BASE                  = 0xB0,    
    /* DMA channel 0 start address register */  
    OFFSET_TS_BUF_START_OFFSET          = 0xC0,    
    /* DMA channel 0 buffer length & Interrupt threshold register */    
    OFFSET_TS_BUF_LEN_THRESHOLD         = 0x140,
    /* DMA channel 0 write & read point regiter */      
    OFFSET_TS_BUF_W_R_PTR               = 0x1C0,      
    /* DMA threshold interrupt mask register */    
    OFFSET_TS_BUF_THRESHOLD_INT_EN_MASK = 0x300,    
    /* DMA threshold interrupt event register */      
    OFFSET_TS_BUF_THRESHOLD_INT         = 0x310,       
    /* DMA overlap interrupt mask register */
    OFFSET_TS_BUF_OVERFLOW_INT_EN_MASK  = 0x320,   
    /* DMA overlap interrupt event register */  
    TS_BUF_OVERFLOW                     = 0x330,   
    
    /* DMA FIFO overflow interrupt mask register */
    OFFSET_DMA_OVERFLOW_INT_EN_MASK     = 0x340,       
    /* DMA FIFO overflow interrupt event register */
    DMA_OVERFLOW                        = 0x254, 

    /* PCR detect enable register. */ 
    PCR_DETECT_EN                       = 0x350,
    /* PCR hit register. */
    PCR_HIT                             = 0x354     
};


enum dmx_mvc_type
{
    DMX_MVC_TYPE_MASK  = 0x00,
    DMX_MVC_TYPE_VALUE = 0x40,
    DMX_MVC_TYPE_CW    = 0x80
};

//#define   dmx_assert(level, warning, ...)    dmx_assert(level, warning, ...)


#endif





/************************** Constant Definitions ****************************/
#define DMX_HW_PARSE_TASK_WAIT_TIME     10 /* counted in milisecond */

#define DMX_TOTAL_TS2SEC_SERVICE        DMX_HW_TOTAL_FILTERS * 2

#define DMX_TOTAL_TS2PES_SERVICE        10

#define DMX_TOTAL_PCR_SERVICE           1

#define DMX_TOTAL_TS_SERVICE            DMX_HW_TOTAL_FILTERS * 3

#define DMX_INVALID_IDX                 0xFFFFFFFF

#define DMX_INVALID_PID                 0x1FFF

/* For linux API. */
#define DMX_LINUX_API_TOTAL_CHANNELS    DMX_TOTAL_TS_SERVICE

#define list_last_entry(ptr, type, member) list_entry((ptr)->prev, type, member)


enum DMX_TYPE
{
    DMX_TYPE_HW,
	DMX_TYPE_SW
};


enum VOB_TYPE 
{
	VOB_START	= 0,
	VOB_NORMAL	= 1,
	VOB_END		= 2
};

/* TS pakcet type */
enum dmx_ts_adaptaion_ctrl
{
    TS_RESERVED          = 0x0,
    TS_PAYLOAD_ONLY      = 0x1,
    TS_ADAPT_ONLY        = 0x2, 
    TS_ADAPT_AND_PAYLOAD = 0x3
};

/* PES stream type. */
enum pes_stream_id
{
    DMX_PES_STREAM_PROGRAM_MAP = 0xBC,
    DMX_PES_STREAM_PRIVATE_1   = 0xBD,
    DMX_PES_STREAM_PADDING     = 0xBE,
    DMX_PES_STREAM_PRIVATE_2   = 0xBF,
    
    DMX_PES_STREAM_MASK_AUDIO  = 0xE0,  
    DMX_PES_STREAM_AUDIO       = 0xC0, 
     
    DMX_PES_STREAM_MASK_VIDEO  = 0xF0,   
    DMX_PES_STREAM_VIDEO       = 0xE0,   
    
    DMX_PES_STREAM_ECM         = 0xF0,
    DMX_PES_STREAM_EMM         = 0xF1,
    DMX_PES_STREAM_DSMCC       = 0xF2,
    DMX_PES_STREAM_13522       = 0xF3,
    DMX_PES_STREAM_REC_H222_A  = 0xF4,
    DMX_PES_STREAM_REC_H222_B  = 0xF5,
    DMX_PES_STREAM_REC_H222_C  = 0xF6,
    DMX_PES_STREAM_REC_H222_D  = 0xF7,
    DMX_PES_STREAM_REC_H222_E  = 0xF8,
    
    DMX_PES_STREAM_ANCILLARY   = 0xF9,
    DMX_PES_STREAM_SL_PACKET   = 0xFA,
    DMX_PES_STREAM_LEXMUX      = 0xFB,
    
    DMX_PES_STREAM_DIRECTORY   = 0xFF
};


enum dmx_pid_flt_status
{
    DMX_PID_FLT_STATUS_IDLE,
    DMX_PID_FLT_STATUS_READY,
    DMX_PID_FLT_STATUS_RUN
};


enum dmx_parse_task_status
{
    DMX_PARSE_TASK_STATUS_IDLE,
    DMX_PARSE_TASK_STATUS_PLAY,
    DMX_PARSE_TASK_STATUS_PAUSE
};

enum dmx_pcr_serivce_status
{
    DMX_PCR_SERVICE_STATUS_IDLE,
    DMX_PCR_SERVICE_STATUS_READY,
    DMX_PCR_SERVICE_STATUS_RUN
};

enum dmx_ts_serivce_status
{
    DMX_TS_SERVICE_STATUS_IDLE,
    DMX_TS_SERVICE_STATUS_READY,
    DMX_TS_SERVICE_STATUS_RUN
};


enum dmx_ts_pkt_info_continuity
{
    DMX_TS_PKT_CONTINU_OK,
    DMX_TS_PKT_CONTINU_LOST,
    DMX_TS_PKT_CONTINU_DUPLICATE
};

enum dmx_ts2sec_service_status
{
    DMX_TS2SEC_SERVICE_STATUS_IDLE,
    DMX_TS2SEC_SERVICE_STATUS_READY,
    DMX_TS2SEC_SERVICE_STATUS_FIRST_TS,
    DMX_TS2SEC_SERVICE_STATUS_REMAIN_TS//,
    //DMX_TS2SEC_SERVICE_STATUS_PAUSE
};

enum dmx_ts2pes_service_status
{
    DMX_TS2PES_SERVICE_STATUS_IDLE,
    DMX_TS2PES_SERVICE_STATUS_READY,

    /* First TS packet for current PES packet. */
    DMX_TS2PES_SERVICE_STATUS_START_CODE,

    /* Remaining TS packet for current PES packet. */
    DMX_TS2PES_SERVICE_STATUS_DATA
};


/* How to copy ts pakcet to upper layer buffer */
enum dmx_ts_buf_cpy_type
{
    DMX_MEM_CPY_CPU = 0,
    DMX_MEM_CPY_DMA = 1
};

enum dmx_task_ctrl_cmd
{
    DMX_PARSE_TASK_CMD_START = 0x1,
    DMX_PARSE_TASK_CMD_STOP  = 0x2,
    DMX_PARSE_TASK_CMD_PAUSE = 0x4
};


enum PES_RETRIEVE_FMT
{
	PES_HEADER_DISCARDED = 7,
	PES_HEADER_INCLUDED
};


enum DMX_PLAYBACK_IN_STATUS
{
    DMX_PLAYBACK_IN_IDLE,
    //DMX_SEE_STATUS_READY,
    DMX_PLAYBACK_IN_RUN,
    DMX_PLAYBACK_IN_FLUSH,
};



enum dmx_channel_status
{
    DMX_CHANNEL_STATUS_IDLE,
    DMX_CHANNEL_STATUS_READY,
    DMX_CHANNEL_STATUS_RUN
};

enum dmx_ts2sec_serv_err_code
{
    DMX_SEC_ERR_CODE_OK,
    DMX_SEC_ERR_CODE_EMPTY_BUF,
    DMX_SEC_ERR_CODE_BAD_POINTER_FIELD,
    DMX_SEC_ERR_CODE_MASK_MISSMATCH,
    DMX_SEC_ERR_CODE_DATA_LOST,
    DMX_SEC_ERR_CODE_CRC_FAIL,
    DMX_SEC_ERR_CODE_TS_SCRAMBLED,
    DMX_SEC_ERR_CODE_BAD_SEC_LEN,
    DMX_SEC_ERR_CODE_SEC_UNFINISHED
};


enum ali_dmx_pkt_status
{
    ALI_DMX_PKT_STATUS_LOADING,

    ALI_DMX_PKT_STATUS_LOADED
};



enum ali_dmx_data_buf_src
{
    ALI_DMX_DATA_BUF_SRC_KERN,

    ALI_DMX_DATA_BUF_SRC_USER,

    ALI_DMX_DATA_BUF_SRC_FAST
};

enum ali_dmx_data_buf_dest
{
    ALI_DMX_DATA_BUF_DEST_KERN,

    ALI_DMX_DATA_BUF_DEST_USER
};


struct ali_dmx_data_node
{
    /* Link to next data node. */
    struct list_head link; 

    __u32 len;

    __u32 rd;

    __u32 wr;
};


struct ali_dmx_data_pkt
{
    enum ali_dmx_pkt_status status;

    /* Link to next data packet. */
    struct list_head link; 

    __u32 len;

    /* Link to data nodes contain data belongs current packet. */
    struct list_head data_node;
};


struct ali_dmx_data_buf
{
    __u32 max_len;
    __u32 cur_len;

    struct mutex rw_mutex;

    wait_queue_head_t wr_wq;

    wait_queue_head_t rd_wq;

    /* Link to data packets manegered by this buffer. */
    struct list_head data_pkt;

    __u32 data_node_size;
    __s32 fst_cpy_slot;  
    
    __s32 fst_cpy_size; 
    __s32 exp_cpy_size;
};

enum ali_dmx_ca_record_status
{
    ALI_DMX_CA_RECORD_BUSY,
        
    ALI_DMX_CA_RECORD_PAUSE,

    ALI_DMX_CA_RECORD_PAUSED,
};

struct dmx_ts_channel_info
{
    DMX_UINT32 ts_serv_id[DMX_REC_PID_LIST_MAX_LEN];
    DMX_UINT32 enc_para;
    DMX_UINT8 *enc_blk;
    DMX_UINT8 *dst_blk;
    DMX_UINT32 enc_blk_wr;
    DMX_UINT32 enc_blk_rd;
    DMX_UINT32 dst_blk_wr;
    DMX_UINT32 dst_blk_rd; 
    DMX_UINT32 enc_blk_len;
    DMX_UINT32 dst_blk_len;
    DMX_UINT8  ca_channel_id;
    enum ali_dmx_ca_record_status status;
};

struct dmx_enc_info
{
    struct workqueue_struct *ca_workqueue;
    struct work_struct ca_work;
    struct dmx_channel *channel[MAX_CA_RECORD_CHANNEL];
    struct mutex io_mutex;
};


struct dmx_sec_channel_info
{
    DMX_UINT32 pid;

    DMX_UINT32 sec_serv_id;
};





struct dmx_pes_channel_info
{
    DMX_UINT32 pid;

    DMX_UINT32 pes_serv_id;
};

struct dmx_pcr_channel_info
{
    DMX_UINT32 pid;

    DMX_UINT32 pcr_serv_id;
};

struct dmx_rec_all_channel_info
{
    DMX_UINT32 rec_whole_tp_rd;
    DMX_UINT32 rec_whole_tp_wd;

    DMX_INT32 fst_cpy_slot;  
    DMX_UINT32 fst_cpy_size; 
    DMX_UINT32 exp_cpy_size;
};



struct dmx_channel
{
    enum dmx_channel_status status;

    struct dmx_channel_param usr_param;

    struct dmx_ts_channel_info ts_ch_info;

    struct dmx_sec_channel_info sec_ch_info;

    struct dmx_pes_channel_info pes_ch_info;

    struct dmx_pcr_channel_info pcr_ch_info;

    struct dmx_rec_all_channel_info rec_all_ch_info;
    /* Data buffer. */
    struct ali_dmx_data_buf data_buf;

    DMX_UINT8 *dmx;
};







struct dmx_ts_pkt_inf
{
    /* Provides continuty info of current TS pakcet to upper layer. */
    enum dmx_ts_pkt_info_continuity continuity; 

    /* TS pakcet info for a TS pakcet.*/
    DMX_UINT32 pid;
    DMX_UINT32 unit_start;
    DMX_UINT32 priority;
    DMX_UINT32 scramble_flag;
    DMX_UINT32 adapt_ctrl;
    DMX_UINT32 disconti_flag;
    DMX_UINT32 conti_cnt;

    DMX_UINT8 *pkt_addr;
    DMX_UINT8 *adapt_addr;

    /* adapt_len + payload_len = 184 */
    __s32 adapt_len; 

    DMX_UINT8 *payload_addr;

    /* adapt_len + payload_len = 184 */
    __s32 payload_len;

    /* VOB start in this TS packet? */
    DMX_UINT32 vob_type;
};



struct dmx_ts_service 
{
    enum dmx_ts_serivce_status status;

    DMX_UINT32 pid;

    struct dmx_ts_service *pri;
    struct dmx_ts_service *next;

    DMX_UINT32 send_raw_ts;

    /* PID filter index of this ts service */
    DMX_UINT32 pid_flt_idx;

    DMX_UINT32 last_ts_unit_start;
    DMX_UINT32 last_ts_priority;
    DMX_UINT32 last_ts_scramble_flag;
    DMX_UINT32 last_ts_adapt_ctrl;
    DMX_UINT32 last_ts_disconti_indicator;
    DMX_UINT32 last_ts_conti_cnt;
    DMX_UINT32 last_ts_adapt_len;
    DMX_UINT32 last_ts_payload_len;

    DMX_UINT32 param;

    DMX_INT32 (*ali_dmx_ts_cb)(void                  *dmx,
                               struct dmx_ts_pkt_inf *pkt_inf,
                               DMX_UINT32             param);


    __u32 pkt_in_cnt;
    __u32 scram_cnt;
    __u32 duplicate_cnt;
    __u32 discon_cnt;
};


struct dmx_ts_service_list
{
    struct dmx_ts_service *head;
    struct dmx_ts_service *tail;
};


struct dmx_pcr_service
{
    enum dmx_pcr_serivce_status status;

    DMX_UINT16 pid;

    DMX_UINT32 ts_serv_idx;
};


struct dmx_ts2pes_service
{
    /* pes service index */
    enum dmx_ts2pes_service_status status;

    DMX_UINT32 pid;

    DMX_UINT32 id;

    DMX_UINT32 ts_serv_idx;

    DMX_UINT32 pkt_len;

    DMX_UINT32 pkt_copied_len;

    DMX_UINT32 param;

    /* Statistics. */
    __u32 pes_pkt_in_cnt;
    
 };


struct dmx_ts2sec_service
{
    enum dmx_ts2sec_service_status status;
   
    DMX_UINT16 pid;

    DMX_UINT32 ts_serv_idx;

    DMX_UINT8 is_mask_hit;

    DMX_UINT8 *buf;

    DMX_UINT32 buf_len;

    DMX_UINT32 buf_rw;

    DMX_UINT32 sec_len;

    __u32 sec_copied_len;

    DMX_UINT32 mask_len;

    DMX_UINT8 mask[DMX_SEC_MASK_MAX_LEN];

    DMX_UINT8 value[DMX_SEC_MASK_MAX_LEN];

    DMX_UINT8 partial_hdr[3];

    DMX_UINT32 partial_hdr_wr;

    enum dmx_ts_buf_cpy_type cpy_type;

    enum dmx_ts2sec_serv_err_code err_code;

    DMX_UINT32 param;
};





struct dmx_pid_flt 
{
    enum dmx_pid_flt_status status;

    /* PID of this hw filter */
    DMX_UINT32 pid;

    struct dmx_pid_flt *pri;
    struct dmx_pid_flt *next;

    /* ts services linked to this hw filter */
    struct dmx_ts_service_list ts_service_list;
};



struct dmx_pid_flt_list
{
    struct dmx_pid_flt *head;
    struct dmx_pid_flt *tail;
};





struct dmx_services
{
    struct dmx_pid_flt_list pid_flt_run_list;

    /* One-to-one bind to HW filers. */
    struct dmx_pid_flt pid_flt[DMX_HW_TOTAL_FILTERS];

    struct dmx_pcr_service pcr_service;

    struct dmx_ts_service ts_service[DMX_TOTAL_TS_SERVICE];

    struct dmx_ts2pes_service ts2pes_service[DMX_TOTAL_TS2PES_SERVICE];

    struct dmx_ts2sec_service ts2sec_service[DMX_TOTAL_TS2SEC_SERVICE];
};


#define DMX_LINUX_API_TOTAL_STREAMS 128



enum dmx_stream_type
{
    DMX_STREAM_TYPE_SEC = 1,
    DMX_STREAM_TYPE_TS = 2,
    DMX_STREAM_TYPE_AUDIO = 3,
    DMX_STREAM_TYPE_VIDEO = 4,
    DMX_STREAM_TYPE_PCR = 5,
    DMX_STREAM_TYPE_ES = 6,
    DMX_STREAM_TYPE_PES = 7,
    DMX_STREAM_TYPE_PS = 8,
    DMX_STREAM_TYPE_AV = 9,
};



enum dmx_stream_state
{
    DMX_STREAM_STATE_IDLE = 0,
    DMX_STREAM_STATE_CFG = 1,
    DMX_STREAM_STATE_STOP = 2,
    DMX_STREAM_STATE_RUN = 3,
};


struct dmx_sec_stream
{
    /* Same as struct dmx_sec_stream_param.
     */
    struct Ali_DmxSecStreamParam param;

    __u32 ts2sec_serv_id;
};  


struct dmx_ts_stream
{
    struct Ali_DmxTsStreamParam param;

    __u32 ts_serv_id[ALI_DMX_TS_STREAM_MAX_PID_CNT];
};


#if 1
struct dmx_audio_stream
{
    struct Ali_DmxAudioStreamParam param;

    __u32 ts_serv_id;
};


struct dmx_video_stream
{
    struct Ali_DmxVideoStreamParam param;

    __u32 ts_serv_id;
};

struct dmx_pcr_stream
{
    struct Ali_DmxPcrStreamParam param;

    __u32 ts_serv_id;
};
#endif


struct dmx_av_stream
{
    struct Ali_DmxAvStreamParam param;

    __u32 a_ts_serv_id;
    __u32 v_ts_serv_id;
    __u32 p_ts_serv_id;
};


struct dmx_stream
{
    enum dmx_stream_state state;

    enum dmx_stream_type type;

    struct dmx_sec_stream sec_stream;    

    struct dmx_ts_stream ts_stream;

    struct dmx_av_stream av_stream;

#if 1
    struct dmx_audio_stream audio_stream;

    struct dmx_video_stream video_stream;

    struct dmx_pcr_stream pcr_stream;
#endif

    /* Data buffer. 
     */
    struct ali_dmx_data_buf data_buf;

    __u8 *dmx;
};



struct dmx_device
{
    DMX_UINT8  name[16];
    DMX_UINT32 base_addr;

    enum DMX_TYPE dmx_type;

    /* 1. For data_source enum DMX_DATA_SRC_TYPE_REALTIME. */
    DMX_UINT16 hw_int_id;
    DMX_UINT32 ts_buf_start_addr;
    DMX_UINT32 ts_buf_end_idx;
    DMX_UINT32 ts_buf_rd_idx;
    DMX_UINT32 ts_buf_wr_idx;

    __u32 irq_num;

    /* Keep HW TS buffer info, total ts_buf_end_idx units,
     * 188 bytes for each unit.
     */
    struct dmx_ts_pkt_inf *pkt_inf;

    /* 2. For data_source DMX_DATA_SOURCE_TYPE_PLAYBACK. */
    enum DMX_PLAYBACK_IN_STATUS playback_in_status;

    struct cdev playback_in_cdev;

    dev_t playback_in_dev_id;

    struct ali_dmx_data_buf playback_in_buf;

    struct device *playback_in_ddevice;



    /* 3. Internal service presentation. */
    struct dmx_services services;

    /* 4. Linux api presentation. 
     * One to one relateionship:
     * linux file<-->channel(dmx linux api)<-->dmx service.
     */
    struct dmx_channel channels[DMX_LINUX_API_TOTAL_CHANNELS];

	struct mutex io_mutex;

    dev_t dev_id;

    DMX_UINT16 last_parsed_pid;

    /* Linux char device for dmx. */
    struct cdev cdev;

    struct device *device;

    /* 5. Data parsing task. */
    struct workqueue_struct *workqueue;

	struct work_struct work;

    struct workqueue_struct *workqueue_for_test;

	struct work_struct work_for_test;


    /* Buffers for Linux API. */
    struct list_head data_node_pool;

    struct list_head data_pkt_pool;

	spinlock_t data_node_spinlock;
	spinlock_t data_pkt_spinlock;

    __u8 *data_pkt_raw;
    
    __u8 *data_node_raw;

    /* Entities in sysfs for debug & status consulting. */
    struct kobject *sysfs_dir_kobj;

    struct attribute_group sysfs_attr_group;

    struct kobj_attribute attr_ts_in_cnt;

    struct kobj_attribute attr_ts_services;
    T_DMX_HW_FUNC pfunc;
    /* Statistics for sysfs. */
    /* total number of TS packets received. */
    __u32 pkt_total_in_cnt;

    __u32 pkt_total_sync_erro_cnt;
    __u32 pkt_total_erro_cnt;
    __u32 isRadio_playback;

    __u32 last_rd_time;
	__u32 last_dura;
	__u32 last_pack_num;
	__u32 last_rate[8];
	__u32 pkts_dura;
	__u32 bit_rate;
    
	__u8  bitrate_detect;	
    struct dmx_stream streams[DMX_LINUX_API_TOTAL_STREAMS];
    __u8  *enc_buffer[MAX_CA_RECORD_CHANNEL];
    __u8  *dst_buffer[MAX_CA_RECORD_CHANNEL];
};


enum DMX_SEE_STATUS
{
    DMX_SEE_STATUS_IDLE,
    DMX_SEE_STATUS_READY,
    DMX_SEE_STATUS_RUN,
    DMX_SEE_STATUS_PAUSE,
};


/* Must keep compatible with TDS macro: #define TS_BLOCK_SIZE 0xbc000. */
#define DMX_SEE_BUF_SIZE  0xBC000       



/* Must keep compatible with TDS enum: enum DEMUX_STATE. */
enum DEMUX_STATE{
    DEMUX_FIND_START = 0,
    DEMUX_HEADER,
    DEMUX_DATA,
    DEMUX_SKIP 
};



/* Must keep compatible with TDS enum: enum STREAM_TYPE. */
enum STREAM_TYPE
{
	UNKNOW_STR = 0,
	PRG_STR_MAP,
	PRIV_STR_1,
	PAD_STR,
	PRIV_STR_2,
	AUDIO_STR,
	VIDEO_STR,
	ECM_STR,
	EMM_STR,
	DSM_CC_STR,
	ISO_13522_STR,
	H2221_A_STR,
	H2221_B_STR,
	H2221_C_STR,
	H2221_D_STR,
	H2221_E_STR,
	ANCILLARY_STR,
	REV_DATA_STR,
	PRG_STR_DIR
};



/* Must keep compatible with TDS struct: struct pes_pkt_param. */
struct pes_pkt_param{
	DMX_UINT8 stream_id;
	DMX_UINT16 pes_pkt_len;
        /*
       DMX_UINT8 orig_or_copy:1;
       DMX_UINT8 copyright:1;
       DMX_UINT8 data_align_indi:1;
       DMX_UINT8 pes_priority:1;
       */
	DMX_UINT8 filled_byte_3:4;
	DMX_UINT8 pes_scramb_ctrl:2;
	DMX_UINT8 marker_10:2;
        /*
       DMX_UINT8 pes_ext_flag:1;
       DMX_UINT8 pes_crc_flag:1;
       DMX_UINT8 additional_copy_info_flag:1;
       DMX_UINT8 dsm_trick_md_flag:1;
       DMX_UINT8 es_rate_flag:1;
       DMX_UINT8 escr_flag:1;
        */
	DMX_UINT8 filled_byte_4:6;
	DMX_UINT8 pts_dts_flags:2;

	DMX_UINT8   pes_header_data_len;
	DMX_UINT16 pes_data_len;
        /*
       DMX_UINT8 marker_bit_0:1;
       DMX_UINT8 pts_32_30:3;
       DMX_UINT8 prefix_4_bits:4;

       DMX_UINT16 marker_bit_1:1;
       DMX_UINT16 pts_29_15:15;

       DMX_UINT16 marker_bit_2:1;
       DMX_UINT16 pts_14_0:15;
        */
        // DMX_UINT32  total_byte_len;
	enum DEMUX_STATE dmx_state ;
    enum STREAM_TYPE stream_type;
	DMX_UINT32 head_buf_pos;
	void * av_buf ;
	DMX_UINT32  av_buf_pos ;
	DMX_UINT32  av_buf_len ;
	struct control_block * ctrl_blk;
	void * device;
	DMX_INT32 (* request_write)(void *, DMX_UINT32, void **, DMX_UINT32 *, struct control_block *);
	void (* update_write)(void *, DMX_UINT32 );


	DMX_UINT8   get_pkt_len:1;
	DMX_UINT8   get_header_data_len:1;
	DMX_UINT8   get_pts:1;
	DMX_UINT8   str_confirm:1;
	DMX_UINT8   reserved:4;

	DMX_UINT8   conti_conter;
	DMX_UINT8 	*head_buf;

	DMX_UINT8 ch_num;
	struct dmx_channel *channel;

//cw 
  DMX_UINT8		cw_parity_num; /*for 3601, (actual index+1)
							1~8:  a ecw/ocw parity number,
							0: no cw parity set for this channel*/
							
//sgdma	
	DMX_UINT8		xfer_es_by_dma;//transfer it by dma or not
	DMX_UINT8		dma_ch_num;//dma channel of this stream
	DMX_UINT32	last_dma_xfer_id;//latest dma transfer id
	
//statistic
	DMX_UINT8		ts_err_code; //last err code of this channel
	DMX_UINT32	ovlp_cnt;	//ovlp INT cnt
	DMX_UINT32	discont_cnt;	
};


/* Used to communicate with SEE.
 * Must keep compartible with TDS:
 * struct SEE_PARSE_INFO
 * {
 *     DMX_UINT32 rd ;
 *     DMX_UINT32 wt ;
 *     DMX_UINT32 dmx_ts_blk ;
 *     ID mutex;
 * };
 */
struct dmx_see_raw_buf_info
{
    DMX_UINT32 rd;
    DMX_UINT32 wr;
    DMX_UINT32 buf;

    /* Not used. */
    DMX_UINT16 mutex;
};






struct dmx_see_buf_info
{
    volatile struct dmx_see_raw_buf_info *raw_buf;
    DMX_UINT32 decrypt_buf;
    DMX_UINT32 decrypt_buf_size;
};



/* Used to communicate with SEE.
 * Must keep compartible with TDS:
 *  struct dmx_see_init_param
 *  {
 *      volatile struct SEE_PARSE_INFO *p_see_parse_info;
 *      UINT32 see_blk_buf;
 *      UINT32 see_blk_buf_size;
 *  };
 */
struct dmx_see_init_param
{
    /* Main to SEE buffer.
    */
    volatile struct dmx_see_raw_buf_info *main2see_buf;

    /* SEE decrypt buffer.
    */
    __u32 decrypt_buf;
    __u32 decrypt_buf_size;

    /* SEE statistics.
    */  
    volatile struct Ali_DmxSeeStatistics *statistics;
	volatile struct Ali_DmxSeeGlobalStatInfo* GlobalStatInfo;
	volatile struct Ali_DmxSeePlyChStatInfo* PlyChStatInfo;
};


struct dmx_see_device
{
    DMX_UINT8 name[16];

    enum DMX_SEE_STATUS status;

    enum dmx_see_av_sync_mode av_sync_mode;

    dev_t dev_id;

    /* Linux char device for dmx. */
    struct cdev cdev;

    struct mutex mutex; 

    struct dmx_see_av_pid_info usr_param;

    DMX_UINT16 ttx_pid;

    DMX_UINT16 subt_pid;

    /* For SEE.*/
    struct dmx_see_init_param see_buf_init;
    //volatile struct dmx_see_buf_info *buf_info;

    /* For see pes parsing. */
    struct pes_pkt_param pes_para[6];

    struct control_block ctrl_blk[6];

    /* Old version of av scramble status in TDS. */
    __u8 av_scram_status;

    /* New version of av scramble status in TDS. */
    struct dmx_see_av_scram_info scram_param_linux;
    struct io_param_ex scram_param_tds;
};




#define A_SGDMA_HW_REG_BASE 0XB800F000 /* Main CPU SGDMA Base address. */

#define A_SGDMA_HW_REG_RANGE 0xC0

#define A_SGDMA_CHANNEL_CNT_SC 4

#define A_SGDMA_CHANNEL_CNT_NO_SC 4

#define A_SGDMA_CHANNEL_CNT (A_SGDMA_CHANNEL_CNT_SC + A_SGDMA_CHANNEL_CNT_NO_SC) 

/* HW support at most 255 nodes for each sgdma channel. */
#define A_SGDMA_CPY_NODE_CNT 255

/* 16 bytes each node. */
#define A_SGDMA_CPY_NODE_LEN 16

#define A_SGDMA_STARTCODE_BUF_LEN  0x1000 /* 4K */

enum A_SGDMA_CH_STATUS
{
    A_SGDMA_CH_STATUS_IDLE,
    A_SGDMA_CH_STATUS_RUN
};

enum A_SGDMA_CH_TYPE
{
    A_SGDMA_CH_TYPE_SC,
    A_SGDMA_CH_TYPE_NO_SC
};

enum A_SGDMA_FLUSH_MODE
{
    ALI_SGDMA_FLUSH_MODE_SPINWAIT,
    ALI_SGDMA_FLUSH_MODE_SLEEPWAIT
};


struct A_SdmaChannel
{
    enum A_SGDMA_CH_STATUS Status;

    enum A_SGDMA_CH_TYPE Type;

    __u32 id;

    void __iomem *NodeBuf;

    __u32 NodeCnt;

    void __iomem *NodeBufReg;

    void __iomem *NodeCntReg;

    void __iomem *NodeWrReg;

    void __iomem *NodeRdReg;


    __u32 FlushSpinCnt;

    __u32 FlushSleepCnt;
};



struct A_SgdmaDev
{
    void __iomem *Base;

    void __iomem *ChEnReg;

    void __iomem *ChEmptyReg;

    struct A_SdmaChannel Ch[A_SGDMA_CHANNEL_CNT];
};




struct A_SgdmaStarcodeCfg
{
    __u16 UpBound;

    __u16 LowBound;

    __u32 FindInBound;
};




DMX_UINT32 dmx_ts_service_register(struct dmx_device * dev, DMX_UINT16 pid, DMX_INT32(* ali_dmx_ts_cb)(void *dev, struct dmx_ts_pkt_inf * pkt_inf, DMX_UINT32 param), DMX_UINT32 param);

DMX_INT32 dmx_ts_service_enable(struct dmx_device * dev, DMX_UINT32 idx);

DMX_INT32 dmx_ts_service_unregister(struct dmx_device * dev, DMX_UINT32 idx);

DMX_INT32 dmx_ts_service_init(struct dmx_device * dev);

DMX_INT32 dmx_ts_service_parse(struct dmx_device * dev, struct dmx_ts_pkt_inf * pkt_inf);



DMX_UINT32 dmx_ts2pes_service_register(struct dmx_device * dmx, DMX_UINT16 pid, DMX_UINT32 param);
DMX_INT32 dmx_ts2pes_service_enable(struct dmx_device * dev, DMX_UINT32 idx);

DMX_INT32 dmx_ts2pes_service_unregister(struct dmx_device * dev, DMX_UINT32 idx);

DMX_INT32 dmx_ts2pes_service_init(struct dmx_device * dev);



DMX_UINT32 dmx_ts2sec_service_register(struct dmx_device * dmx, DMX_UINT16 pid, DMX_UINT32 mask_len, DMX_UINT8 * mask, DMX_UINT8 * value, DMX_UINT32 param);
DMX_INT32 dmx_ts2sec_service_enable(struct dmx_device * dev, DMX_UINT32 idx);

DMX_INT32 dmx_ts2sec_service_unregister(struct dmx_device * dev, DMX_UINT32 idx);

DMX_INT32 dmx_ts2sec_service_init(struct dmx_device * dev);



DMX_UINT32 dmx_pcr_service_register(struct dmx_device * dev, DMX_UINT16 pid);

DMX_INT32 dmx_pcr_service_enable(struct dmx_device * dev, DMX_UINT32 idx);

DMX_INT32 dmx_pcr_service_unregister(struct dmx_device * dev, DMX_UINT32 idx);

DMX_INT32 dmx_pcr_service_init(struct dmx_device * dev);


DMX_INT32 dmx_channel_start(struct dmx_device * dmx, struct dmx_channel * channel, unsigned int cmd, unsigned long arg);

ssize_t dmx_channel_read(struct file *file,char __user *buf,size_t count,loff_t *ppos);

int dmx_channel_open(struct inode *inode, struct file *file);

int dmx_channel_release(struct inode *inode, struct file  *file);

int dmx_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

unsigned int dmx_channel_poll(struct file *filp, struct poll_table_struct *wait);

int dmx_channel_init(struct dmx_device *dmx);


DMX_INT32 dmx_form_hw_init(struct dmx_device * dmx_dev, DMX_UINT8 * name, struct class * ali_m36_dmx_class, struct file_operations * fops, DMX_UINT32 hw_reg_base, DMX_UINT32 defined_hw_buf, DMX_UINT32 hw_buf_len, DMX_UINT32 irq_num);


DMX_INT32 dmx_self_test_init(void);

int dmx_test_pes_serv_req_buf(void *handle, void **buf_start, DMX_INT32 *ret_len, struct ctrl_blk *blk);

void dmx_test_pes_serv_ret_buf(void *device, DMX_INT32 len);


int dmx_see_init(struct dmx_device * dmx);

DMX_INT32 dmx_see_buf_wr_ts(struct dmx_device *dmx, struct dmx_ts_pkt_inf *pkt_inf, DMX_UINT32 param);



irqreturn_t dmx_hw_isr(int irq, void * dmx);
void dmx_hw_parse_task(struct work_struct * work);


ssize_t dmx_playback_buf_usr_wr(struct file * file, const char __user * buf, size_t count, loff_t * ppos);

DMX_INT32 dmx_playback_in_open(struct inode * inode, struct file * file);
DMX_INT32 dmx_playback_in_release(struct inode * inode, struct file * file);





typedef DMX_INT32 (*ts_serv_recv_func)(void *dev, struct dmx_ts_pkt_inf *pkt_inf, DMX_UINT32 param);   

int dmx_data_buf_usr_rd_data(struct dmx_device * dmx, struct file * file, struct ali_dmx_data_buf * src_buf, char __user * usr_buf, size_t rd_len);
__s32 dmx_data_buf_kern_wr_data(struct dmx_device * dmx, struct ali_dmx_data_buf * dest_buf, __u8 * src, __s32 wr_len);

__s32 dmx_data_buf_kern_flush_all_pkt(struct dmx_device * dmx, struct ali_dmx_data_buf * data_buf);

__s32 dmx_data_buf_usr_wr_data(struct dmx_device * dmx, struct file * file, struct ali_dmx_data_buf * dest_buf, const char __user * src, __s32 wr_len, __s32 fast);


struct ali_dmx_data_pkt* dmx_data_buf_kern_force_unlink_first_pkt(struct dmx_device * dmx, struct ali_dmx_data_buf * data_buf);

__s32 dmx_data_buf_kern_flush_last_pkt(struct dmx_device * dmx, struct ali_dmx_data_buf * data_buf);

__s32 dmx_data_buf_kern_wr_pkt_end(struct dmx_device * dmx, struct ali_dmx_data_buf * data_buf);

__s32 dmx_data_buf_usr_rd_pkt(struct dmx_device * dmx, struct file * file, struct ali_dmx_data_buf * src_buf, char __user * usr_buf, size_t rd_len);

__s32 dmx_data_buf_first_pkt_len(struct dmx_device * dmx, struct ali_dmx_data_buf * data_buf);

__s32 dmx_data_buf_len(struct dmx_device * dmx, struct ali_dmx_data_buf * data_buf);

__s32 dmx_data_buf_setup(struct dmx_device * dmx, struct ali_dmx_data_buf * data_buf, __u32 buf_max_len, __u32 buf_node_len);

__s32 dmx_data_pool_init(struct dmx_device * dmx, __u32 data_node_size, __u32 data_node_cnt);

struct ali_dmx_data_node* dmx_data_node_put(struct dmx_device * dmx, struct ali_dmx_data_node * data);

__s32 dmx_data_pool_release(struct dmx_device * dmx);


int dmx_playback_ioctl(struct file * filp, unsigned int cmd, unsigned long arg);


__s32 dmx_sysfs_entry_create(struct dmx_device * dmx, struct attribute * * attrs);

ssize_t dmx_global_stat_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf);

ssize_t dmx_ts_services_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf);


__s32 A_SgdmaInit(void);


__s32 A_SgdmaChOpen(enum A_SGDMA_CH_TYPE Type);


__s32 A_SgdmaChCpy(__u32 ChIdx, void * Dest, void * Src, __u32 Len);

__s32 A_SgdmaChFlush(__u32 ChIdx, enum A_SGDMA_FLUSH_MODE mode);


void dmx_check_ts_to_user(void *ts_buf, __u32 len);

void dmx_check_ts_to_kernel(void *ts_buf, __u32 len);

DMX_INT32 dmx_see_paused(void);


















