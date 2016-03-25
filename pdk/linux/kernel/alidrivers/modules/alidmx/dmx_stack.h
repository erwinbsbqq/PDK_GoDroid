
#ifndef _ALI_DMX_STACK_H_
#define _ALI_DMX_STACK_H_

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
#include <linux/spinlock.h>
#include <linux/version.h>
#include <asm/io.h>
#include <ali_reg.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <ali_basic_common.h>
#include <linux/ali_rpc.h>
#include <linux/list.h>

#include <rpc_hld/ali_rpc_hld.h>
//#include <linux/dvb/ali_dmx.h>
#include <ali_dmx_common.h>
#include <linux/Ali_DmxLib.h>
#include <linux/Ali_DmxLibInternal.h>

/* Macroes for debug printf.
*/
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
#define DMX_API_DBG(...)
#else
#define DMX_API_DBG printk
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





#define ALI_HW_REG_R32(x)        (__REG32ALI(x))
#define ALI_HW_REG_W32(x, d)  do {(__REG32ALI(x)) = (d);} while(0)

#define ALI_HW_REG_R16(x)        (__REG16ALI(x))
#define ALI_HW_REG_W16(x, d)  do {(__REG16ALI(x)) = (d);} while(0)

#define ALI_HW_REG_R8(x)         (__REG8ALI(x))
#define ALI_HW_REG_W8(x, d)   do {(__REG8ALI(x)) = (d);} while(0)



#define dmx_assert(...)

#define MEMCPY memcpy

#define list_last_entry(ptr, type, member) list_entry((ptr)->prev, type, member)


/************************** Constant Definitions ****************************/

#define DMX_LINUX_OUTPUT_DEV_CNT         12
#define DMX_LINUX_INPUT_DEV_CNT          5


#define DMX_DATA_ENG_OUTPUT_INTERVAL     2//10 /* counted in milisecond */

#define DMX_SEC_FLT_CNT                  256

#define DMX_PES_FLT_CNT                  20

#define DMX_PCR_FLT_CNT                  1

#define DMX_TS_FLT_CNT                   (DMX_SEC_FLT_CNT + \
                                          DMX_PES_FLT_CNT + \
                                          DMX_PCR_FLT_CNT + \
                                          16)

#define DMX_STREAM_CNT                   DMX_TS_FLT_CNT

#define DMX_INSTREAM_RAM_CNT             DMX_LINUX_INPUT_DEV_CNT

/* For HW security solution.
*/
#define DMX_DEENCRYPT_BUF_LEN            (48 * 30 * 188)//Must be Multiple of 64 TS pakcets to meet DSC requirement

/* Provide backward compartibility with old linux native APIs.
*/
#define DMX_CHANNEL_CNT                  DMX_TS_FLT_CNT
#define DMX_LINUX_OUTPUT_DEV_CNT_LEGACY  3
#define DMX_LINUX_INPUT_DEV_CNT_LEGACY   1



#define DMX_PID_FLT_MAX_DEV_CNT          9
#define DMX_PID_FLT_MAX_HW_FLT_EACH_DEV  128
#define DMX_PID_FLT_MAX_CNT              DMX_TS_FLT_CNT

#define DMX_DATA_POOL_NODE_SIZE          (5 * 188)//1024//1024//4136 /* 22 * 188bytes */
#define DMX_DATA_POOL_NODE_CNT           9362//8192//8192//5120

#define DMX_DATA_ENG_RUNBACK_BUF_LEN     512


#define DMX_INVALID_PID                  0x1FFF

/* For retrieving all TS packet from one TP.
*/
#define DMX_WILDCARD_PID                 0xFFFF

/* Linux interface.
*/
enum DMX_LINUX_INTERFACE_TYPE
{
    DMX_LINUX_INTERFACE_TYPE_OUTPUT,
    DMX_LINUX_INTERFACE_TYPE_INPUT
};


enum DMX_HW_INTERFACE_TYPE
{
    DMX_HW_INTERFACE_TYPE_HW,
    DMX_HW_INTERFACE_TYPE_USR,
    DMX_HW_INTERFACE_TYPE_SEE
};


enum DMX_DATA_ENGINE_SRC
{
    DMX_DATA_ENGINE_SRC_REAL_HW, 
    DMX_DATA_ENGINE_SRC_VIRTUAL_HW
};

enum DMX_HW_INTERFACE_STATE
{
    DMX_HW_INTERFACE_STATE_IDLE = 0,
	DMX_HW_INTERFACE_STATE_RUN
};


enum DMX_TPSCAN_STATE
{
    DMX_TPSCAN_STATE_IDLE = 0,
	DMX_TPSCAN_STATE_RUN
};


enum DMX_DATA_ENGINE_TASK_HW_STATE
{
    DMX_DATA_ENGINE_TASK_HW_STATE_IDLE = 0,
    DMX_DATA_ENGINE_TASK_HW_STATE_RUN,      
};


enum DMX_TS_PKT_CONTINU
{
    DMX_TS_PKT_CONTINU_OK,
    DMX_TS_PKT_CONTINU_LOST,
    DMX_TS_PKT_CONTINU_DUPLICATE
};


/* TS pakcet type.
*/
enum DMX_TS_ADAPTATION_CTRL
{
    TS_RESERVED          = 0x0,
    TS_PAYLOAD_ONLY      = 0x1,
    TS_ADAPT_ONLY        = 0x2, 
    TS_ADAPT_AND_PAYLOAD = 0x3
};

/* PES stream type.
*/
enum PES_STREAM_ID
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


/* PES stream type.
*/
enum PES_FIRST_STATUS
{
    DMX_PES_OTHER_PKT = 0,
    DMX_PES_FIRST_PKT 
};


enum DMX_PID_FLT_SW_STATE
{
    DMX_PID_FLT_SW_STATE_IDLE,
    DMX_PID_FLT_SW_STATE_STOP,
    DMX_PID_FLT_SW_STATE_RUN,
};



enum DMX_PID_FLT_HW_STATE
{
    DMX_PID_FLT_HW_STATE_IDLE,
    DMX_PID_FLT_HW_STATE_STOP,
    DMX_PID_FLT_HW_STATE_RUN,
};


enum DMX_TS_FLT_STATE
{
    DMX_TS_FLT_STATE_IDLE,
    DMX_TS_FLT_STATE_STOP,
    DMX_TS_FLT_STATE_RUN
};


enum DMX_SEC_FLT_STATE
{
    DMX_SEC_FLT_STATE_IDLE,
    DMX_SEC_FLT_STATE_STOP,
    DMX_SEC_FLT_STATE_FIRST_TS,
    DMX_SEC_FLT_STATE_REMAIN_TS
};

enum DMX_PES_FLT_STATE
{
    DMX_PES_FLT_STATE_IDLE,
    DMX_PES_FLT_STATE_STOP,
    DMX_PES_FLT_STATE_FIRST_TS,
    DMX_PES_FLT_STATE_REMAIN_TS
};


enum DMX_PCR_FLT_STATE
{
    DMX_PCR_FLT_STATE_IDLE,
    DMX_PCR_FLT_STATE_STOP,
    DMX_PCR_FLT_STATE_RUN
};



/* How to copy ts pakcet to upper layer buffer.
*/
enum DMX_DATA_CPY_METHOD
{
    DMX_MEM_CPY_CPU = 0,
    DMX_MEM_CPY_DMA = 1
};


enum DMX_SEC_FLT_CB_TYPE
{
    DMX_SEC_FLT_CB_TYPE_PKT_DATA = 0,
    DMX_SEC_FLT_CB_TYPE_PKT_END = 1,
    DMX_SEC_FLT_CB_TYPE_ERR = 2,
};

enum DMX_PES_FLT_CB_TYPE
{
    DMX_PES_FLT_CB_TYPE_PKT_DATA = 0,
    DMX_PES_FLT_CB_TYPE_PKT_END = 1,
    DMX_PES_FLT_CB_TYPE_ERR = 2,
};

enum DMX_ERR_CODE
{
    DMX_ERR_OK = 0,
    DMX_ERR_BUF_BUSY = -5001,

    /* Section error.
     */
    DMX_ERR_SEC_FLT_NO_BUF = -6001,
    DMX_ERR_SEC_FLT_BAD_POINTER_FIELD = -6002,
    DMX_ERR_SEC_FLT_MASK_MISSMATCH = -6003,
    DMX_ERR_SEC_FLT_MASK_TOO_LONG = -6004,
    DMX_ERR_SEC_FLT_CRC_FAIL = -6005,
    DMX_ERR_SEC_FLT_BAD_SEC_LEN = -6006,
    DMX_ERR_SEC_FLT_SEC_UNFINISHED = -6007,
    DMX_ERR_SEC_FLT_TS_LOST = -6008,
    DMX_ERR_SEC_FLT_TS_SCRAMBLED = -6009,

    /* DMX_SEC_ERR_CODE_TS_DUPLICATE is a warining, not an error.
     */
    DMX_ERR_SEC_FLT_TS_DUPLICATE = -6100,
    DMX_ERR_SEC_FLT_EXHAUST = -6101,
    DMX_ERR_SEC_FLT_OPERATION_DENIED = -6102,

    /* Ts error.
     */
    DMX_ERR_TS_FLT_EXHAUST = -6200,
    DMX_ERR_TS_FLT_OPERATION_DENIED = -6201,
    DMX_ERR_TS_FLT_SYNC_BYTE_ERR = -6202,
    DMX_ERR_TS_FLT_PKT_ERR = -6203,

    /* Pid filter error.
     */
    DMX_ERR_PID_FLT_EXHAUST = -6300,
    DMX_ERR_PID_FLT_OPERATION_DENIED = -6301,

    DMX_ERR_PID_FLT_HW_EXHAUST = -6302,
    DMX_ERR_PID_FLT_SUSPICIOUS_TS = -6303,

    /* PES service error.
     */
    DMX_ERR_PES_FLT_EXHAUST = -6400,
    DMX_ERR_PES_FLT_OPERATION_DENIED = -6401,
    DMX_ERR_PES_FLT_TS_BAD_STATE = -6402,
    DMX_ERR_PES_FLT_TS_DUMPLICATE = -6403,
    DMX_ERR_PES_FLT_TS_SCRAMBLED = -6404,
    DMX_ERR_PES_FLT_TS_LOST = -6405,

    /* Pcr service error.
     */
    DMX_ERR_PCR_SERV_EXHAUST = -6500,
    DMX_ERR_PCR_SERV_OPERATION_DENIED = -6501,

    /* Data buffer error.
     */
    DMX_ERR_DATA_BUF_EMPTY = -6600,

    /* HW error.
    */
    DMX_ERR_HW_NOT_EXIST = -6700,
    DMX_ERR_HW_REG_ISR_FAIL = -6701,
    DMX_ERR_HW_PCR_NO_LIVE_FLT = -6702,
    DMX_ERR_HW_NOT_PERMIT = -6703,
    DMX_ERR_HW_NO_FUNCTION = -6704,

    /* Linux interface error.
    */
    DMX_ERR_LINUX_INTERFACE_TYPE_ILLEGAL = -6800,
    DMX_ERR_LINUX_INTERFACE_INIT_FAIL = -6801,

    /* Linux data engine error.
    */
    DMX_ERR_DATA_ENGINE_TYPE_ILLEGAL = -6900,
    DMX_ERR_DATA_ENGINE_INIT_FAIL = -6901,
    DMX_ERR_DATA_ENGINE_NEED_RESYNC_PKT = -6902,
    DMX_DATA_ENGINE_INVALID_PKT_LEN = -6903,

    /* Pcr filter errors.
    */
    DMX_ERR_PCR_FLT_EXHAUST = -7000,
    DMX_ERR_PCR_FLT_OPERATION_DENIED = -7001,
};


enum DMX_PKT_STATUS
{
    /* Packet is receiving. 
    */
    DMX_PKT_STATUS_LOADING,

    /* Packet has been fully received.
    */
    DMX_PKT_STATUS_LOADED
};



enum DMX_DATA_SRC_TYPE
{
    DMX_DATA_SRC_TYPE_KERN,
    DMX_DATA_SRC_TYPE_USER,
    DMX_DATA_SRC_TYPE_FAST
};

enum DMX_DATA_CPY_DEST
{
    DMX_DATA_CPY_DEST_KERN,
    DMX_DATA_CPY_DEST_USER
};

enum DMX_STREAM_TYPE
{
    DMX_STREAM_TYPE_SEC = 1,
    DMX_STREAM_TYPE_TS = 2,
    DMX_STREAM_TYPE_AUDIO = 3,
    DMX_STREAM_TYPE_VIDEO = 4,
    DMX_STREAM_TYPE_PCR = 5,
    DMX_STREAM_TYPE_ES = 6,
    DMX_STREAM_TYPE_PES = 7,
    DMX_STREAM_TYPE_PS = 8,
    DMX_STREAM_TYPE_TP = 9,    
    DMX_STREAM_TYPE_KERN_GLB = 10,
    DMX_STREAM_TYPE_SEE_GLB = 11,
    DMX_STREAM_TYPE_HW_REG = 12,
};

enum DMX_STREAM_STATE
{
    DMX_STREAM_STATE_IDLE = 0,
    DMX_STREAM_STATE_CFG = 1,
    DMX_STREAM_STATE_STOP = 2,
    DMX_STREAM_STATE_RUN = 3,
};


enum DMX_CHANNEL_STATE
{
    DMX_CHANNEL_STATE_IDLE = 0,
    DMX_CHANNEL_STATE_CFG = 1,
    DMX_CHANNEL_STATE_STOP = 2,
    DMX_CHANNEL_STATE_RUN = 3,
};


enum DMX_INSTREAM_RAM_STATE_LEGACY
{
    DMX_INSTREAM_RAM_STATE_IDLE_LEGACY = 0,
    DMX_INSTREAM_RAM_STATE_RUN_LEGACY = 1,
};





struct ali_dmx_data_node
{
    /* Link to next data node.
    */
    struct list_head link; 

    __u32 len;

    __u32 rd;

    __u32 wr;
};


struct ali_dmx_data_pkt
{
    enum DMX_PKT_STATUS status;

    /* Link to next data packet. */
    struct list_head link; 

    __u32 len;

    /* Link to data nodes contain data belongs current packet. */
    struct list_head data_node;
};


struct ali_dmx_data_buf
{
    __u32 cur_len;

    struct mutex rw_mutex;

    wait_queue_head_t wr_wq;

    wait_queue_head_t rd_wq;

    /* Link to data packets manegered by this buffer. */
    struct list_head data_pkt;

    __s32 fst_cpy_slot;  

    __s32 fst_cpy_size; 
    __s32 exp_cpy_size;
   
};



struct dmx_ts_pkt_inf
{
    /* Provides continuty info of current TS pakcet to upper layer.
     */
    enum DMX_TS_PKT_CONTINU continuity; 

    /* TS pakcet info for a TS pakcet.
     */
    __u32 pid;
    __u32 unit_start;
    __u32 priority;
    __u32 scramble_flag;
    __u32 adapt_ctrl;
    __u32 disconti_flag;
    __u32 conti_cnt;

    /* Start address of TS packet.
     */
    __u8 *pkt_addr;

    /* Start address of TS adaptation field.
     */
    __u8 *adapt_addr;

    /* adapt_len + payload_len = 184.
     */
    __s32 adapt_len; 

    /* Start address of TS payload.
     */
    __u8 *payload_addr;

    /* adapt_len + payload_len = 184.
     */
    __s32 payload_len;

    /* VOB start in this TS packet (For PVR)?
     */
    __u32 vob_type;
};


/* Link to dmx_pid_flt_hw, presents a virtual pid filter that has a predefined
*  behavior to upper layer. HW differences are handled here, the goal is to let
*  the upper layer see a consistent filter layer with no difference on diffrent
*  HW platform. 
*/
struct dmx_pid_flt_sw
{
    /* Link to privious & next dmx_pid_flt_sw in sw_run_list.
    */
    struct list_head link; 

    enum DMX_PID_FLT_SW_STATE state;

    __u32 src_hw_interface_id;
    
    __s32 flt_hw_id;

    __s32 (*pid_flt_cb)(struct dmx_ts_pkt_inf *pkt_inf,
                        __u32                  cb_param);
    
    __u32 cb_param;

    __u32 ts_in_cnt;
};


struct dmx_pid_flt_hw
{
    /* Link to privious & next dmx_pid_flt_hw in hw_run_list.
    */
    struct list_head link; 

    enum DMX_PID_FLT_HW_STATE state;

    /* PID of this hw filter.
     */
    __u32 pid;

    /* Point to SW filter linked to this hw filter.
    */
    struct list_head sw_run_list;   
};


struct dmx_pid_flt_hw_abstraction
{
    /* One-to-one relationship with HW filter.
    */
    struct dmx_pid_flt_hw flt_hw[DMX_PID_FLT_MAX_HW_FLT_EACH_DEV];

    /* A list of running HW filter.
    */
    struct list_head hw_run_list;

    /* A list of running TP filter.
    */
    struct list_head tp_run_list;   
};



struct dmx_pid_flt_module
{
    struct dmx_pid_flt_hw_abstraction hw_abstraction[DMX_PID_FLT_MAX_DEV_CNT];

    struct dmx_pid_flt_sw flt_sw[DMX_PID_FLT_MAX_CNT];
};


struct dmx_ts_flt
{
    /* State of this ts filter.
    */
    enum DMX_TS_FLT_STATE state;

    /* DMX ID this ts filter belongs to.
    */
    __u32 dmx_id;

    __u32 pid;

    /* PID filter index of this ts service.
    */
    __u32 pid_flt_idx;

    /* For TS packet continuty check.
    */
    __u32 last_ts_adapt_ctrl;
    __u32 last_ts_conti_cnt;

    /* Call back fuction registered by upper layer for retrieving TS packet.
    */
    __s32 (*ts_pkt_cb)(struct dmx_ts_pkt_inf *pkt_inf,
                       __u32                  param);

    /* Call back fuction param registered by upper layer, it is used as the 2nd
     * param in ts_pkt_cb().
     */
    __u32 cb_param;

    /* TS packet statistics infomation.
    */
    struct Ali_DmxDrvTsFltStatInfo stat_info;
};



struct dmx_ts_flt_module
{
    struct dmx_ts_flt ts_flt[DMX_TS_FLT_CNT];
};



struct dmx_sec_flt
{
    enum DMX_SEC_FLT_STATE state;
   
    __u16 pid;

    /* DMX ID this ts filter belongs to.
    */
    __u32 dmx_id;

    __u32 ts_flt_idx;

    __u32 sec_len;

    __u32 sec_copied_len;

    struct Ali_DmxSecMaskInfo MaskInfo;

    /* Store first bytes of section data in case these bytes are not contained
     * in one single TS pakcet.
     * The purpose of doing this is to do section data mask direcly in HW buffer
     * to offload CPU loading by reducing memory copy.
     */
    __u8 mask_sec_data[ALI_DMX_SEC_MATCH_MAX_LEN];
    
    __u32 mask_sec_data_wr;

    /* Store first bytes of section data in case these bytes are not contained
     * in one single TS pakcet.
     * The purpose of doing this is to get section length in one separate buffer
     * if first 3 bytes of a section are not contained in one TS packet.
     */
    __u8 partial_hdr[3];
    
    __u32 partial_hdr_wr;

    enum DMX_DATA_CPY_METHOD cpy_method;

    __s32 (*sec_data_cb) (__u8                     *src,
                          __s32                     len,
                          enum DMX_SEC_FLT_CB_TYPE  cb_type,
                          __u32                     cb_param);

    __u32 cb_param;

    /* Section data statistics infomation.
    */
    struct Ali_DmxDrvSecFltStatInfo stat_info;
};




struct dmx_sec_flt_module
{
    struct dmx_sec_flt sec_flt[DMX_SEC_FLT_CNT];
};



struct dmx_pes_flt
{
    enum DMX_PES_FLT_STATE state;
    
    enum DMX_DATA_CPY_METHOD cpy_method;
   
    __u16 pid;

    /* DMX ID this ts filter belongs to.
    */
    __u32 dmx_id;

    __u32 ts_flt_idx;
    
    /* pkt info */
    __u16 pkt_len ;

    __u16 pkt_copied_len ;


    __s32 (*pes_data_cb) (__u8                     *src,
                          __s32                     len,
                          enum DMX_PES_FLT_CB_TYPE  cb_type,
                          __u32                     cb_param);

    __u32 cb_param;

    /* PES data statistics infomation.
    */
    struct Ali_DmxDrvPesFltStatInfo stat_info;
};




struct dmx_pes_flt_module
{
    struct dmx_pes_flt pes_flt[DMX_PES_FLT_CNT];
};




struct dmx_pcr_flt
{
    enum DMX_PCR_FLT_STATE state;
   
    __u16 pid;

    /* DMX ID this ts filter belongs to.
    */
    __u32 dmx_id;

    __u32 ts_flt_idx;

    __s32 (*pcr_value_cb)(__u32 pcr, __u32 cb_param);
    
    __u32 pcr_cb_param;

    __s32 (*pcr_pkt_cb)(struct dmx_ts_pkt_inf *pkt_inf,
                        __u32                   pkt_cb_param);
    
    __u32 pcr_pkt_param;
};



struct dmx_pcr_flt_module
{
    struct dmx_pcr_flt pcr_flt[DMX_PCR_FLT_CNT];
};



/* Buffers for Linux API.
 */
struct dmx_data_buf_module
{
#if 0
    struct mutex data_node_mutex;
    struct mutex data_pkt_mutex;
#else
    spinlock_t data_node_spinlock;
    spinlock_t data_pkt_spinlock;
#endif

    struct list_head data_node_pool;
    
    struct list_head data_pkt_pool;
    
    __u8 *data_pkt_raw;
    
    __u8 *data_node_raw;
};


struct dmx_data_engine_output
{
    enum DMX_DATA_ENGINE_TASK_HW_STATE state;

    __u32 src_hw_interface_id;

    enum DMX_DATA_ENGINE_SRC src_type;
        
    //struct workqueue_struct *workqueue;
    
    //struct work_struct work;

    __u32 pkt_len;

    __s32 pkt_synced;

    __u8 *runback_pkt_buf;

    __u32 runback_pkt_buf_wr;

    __u32 runback_pkt_buf_len;

    __u32 ts_in_cnt;
};


struct dmx_data_engine_module
{
    struct dmx_data_engine_output engine_output[DMX_LINUX_OUTPUT_DEV_CNT];
};




struct dmx_sec_stream
{
    /* Same as struct dmx_sec_stream_param.
     */
    struct Ali_DmxSecStreamParam param;

    struct Ali_DmxDrvSecStrmStatInfo stat_info;

    __s32 sec_flt_id;

    /* For sending TS pakcet of this section stream to SEE for descrambling. 
    */
    __s32 ts_flt_id2see;
};  


struct dmx_ts_stream
{
    struct Ali_DmxTsStreamParam param;

    struct Ali_DmxDrvTsStrmStatInfo stat_info;

    __s32 ts_flt_id[ALI_DMX_TS_STREAM_MAX_PID_CNT];

    /* For sending TS pakcet of this TS stream to SEE for descrambling. 
    */
    __s32 ts_flt_id2see[ALI_DMX_TS_STREAM_MAX_PID_CNT];
};


struct dmx_audio_stream
{
    struct Ali_DmxAudioStreamParam param;

    struct Ali_DmxDrvAudioStrmStatInfo stat_info;

    __u32 ts_flt_id;
};


struct dmx_video_stream
{
    struct Ali_DmxVideoStreamParam param;

    struct Ali_DmxDrvVideoStrmStatInfo stat_info;

    __u32 ts_flt_id;
};

struct dmx_pcr_stream
{
    struct Ali_DmxPcrStreamParam param;

    struct Ali_DmxDrvPcrStrmStatInfo stat_info;

    __u32 pcr_flt_id;

    __u32 latest_pcr;
};

struct dmx_pes_stream
{
    /* Same as struct dmx_pes_stream_param.
     */
    struct Ali_DmxPesStreamParam param;

    struct Ali_DmxDrvPesStrmStatInfo stat_info;

    __s32 pes_flt_id;
};  


struct dmx_tp_stream
{
    /* Same as struct dmx_pes_stream_param.
     */
    struct Ali_DmxTpStreamParam param;

    struct Ali_DmxDrvTpStrmStatInfo stat_info;

    /* The PID if this pid filter is configured to DMX_WILDCARD_PID to match
     * all received TS packets in this TP.
     */
    __s32 pid_flt_id;
};  





/* A stream could only be one of the following types.
*/
union dmx_stream_detail
{
    struct dmx_sec_stream sec_stream;    
    
    struct dmx_ts_stream ts_stream;
    
    struct dmx_audio_stream audio_stream;
    
    struct dmx_video_stream video_stream;
    
    struct dmx_pcr_stream pcr_stream;

    struct dmx_pes_stream pes_stream; 

    struct dmx_tp_stream tp_stream;
};


struct dmx_stream
{
    enum DMX_STREAM_STATE state;

    enum DMX_STREAM_TYPE type;

    union dmx_stream_detail detail;

    /* Data buffer. 
     */
    struct ali_dmx_data_buf data_buf;

    __u8 *dmx_output_device;

    __u32 stream_id;
};


struct dmx_stream_module
{
    struct dmx_stream stream[DMX_STREAM_CNT];
};



struct dmx_ts_in_ram_stream
{
    enum DMX_STREAM_STATE state;

    enum DMX_STREAM_TYPE type;

    __u8 *dmx_input_device;

    wait_queue_head_t wr_wq;

	struct Ali_DmxDrvTsInRamStrmStatInfo stat_info; 
};


struct dmx_ts_in_ram_module
{
    struct dmx_ts_in_ram_stream stream[DMX_INSTREAM_RAM_CNT];
};


struct dmx_mutex_module
{
    struct mutex output_dev_mutex[DMX_LINUX_OUTPUT_DEV_CNT];

    struct mutex input_dev_mutex[DMX_LINUX_INPUT_DEV_CNT];
};




/* Legacy structure.
*/
struct dmx_sec_channel
{
    /* Same as struct dmx_sec_stream_param.
     */
    struct Ali_DmxSecStreamParam param;

    struct Ali_DmxDrvSecStrmStatInfo stat_info;

    __s32 sec_flt_id;
};  


struct dmx_ts_channel
{
    struct Ali_DmxTsStreamParam param;

    struct Ali_DmxDrvTsStrmStatInfo stat_info;

    __u32 ts_flt_id[ALI_DMX_TS_STREAM_MAX_PID_CNT];

	/* For sending TS pakcet of this TS stream to SEE for descrambling. 
	*/	
	__s32 ts_flt_id2see[ALI_DMX_TS_STREAM_MAX_PID_CNT];

    __u32 enc_para;	
};


struct dmx_pes_channel
{
    /* Same as struct dmx_pes_stream_param.
     */
    struct Ali_DmxPesStreamParam param;

    struct Ali_DmxDrvPesStrmStatInfo stat_info;

    __s32 pes_flt_id;
};  


struct dmx_pcr_channel
{
    struct Ali_DmxPcrStreamParam param;

    struct Ali_DmxDrvPcrStrmStatInfo stat_info;

    __u32 pcr_flt_id;

    __u32 latest_pcr;
};


/* A stream could only be one of the following types.
*/
union dmx_channel_detail
{
    struct dmx_sec_channel sec_ch;   
    
    struct dmx_ts_channel ts_ch;
    
    struct dmx_pcr_channel pcr_ch;

    struct dmx_pes_channel pes_ch;
};


struct dmx_channel
{
    enum DMX_CHANNEL_STATE state;

    struct dmx_channel_param usr_param;

    union dmx_channel_detail detail;

    /* Data buffer. 
     */
    struct ali_dmx_data_buf data_buf;

    /* Interchange ts packet buffer for AS solution.
     * Temporarily store orignal TS pakcets, these pakcets will be send to 
     * DSC to do de-encryption and finally store into data_buf to be read out
     * by userspace.
     */
    struct ali_dmx_data_buf data_buf_orig;
    __u8 *dmx_output_device;
};




struct dmx_tp_autoscan
{
    enum DMX_TPSCAN_STATE state;
    __s32 tp_filter_id;
	__u32 tp_scan_buf;
	__u32 tp_scan_buf_wr;
	struct ali_dmx_data_buf tp_data_buf;
};


struct dmx_channel_module_legacy
{
    struct dmx_channel channel[DMX_CHANNEL_CNT];
	struct dmx_tp_autoscan autoscan;

	__u32 dmx_de_enc_input_buf;
	__u32 dmx_de_enc_output_buf;
};


struct dmx_instream_ram_legacy
{
    enum DMX_INSTREAM_RAM_STATE_LEGACY state;

    __u8 *dmx_input_device;
	
	struct Ali_DmxDrvTsInRamStrmStatInfo stat_info; 
};


struct dmx_instream_ram_module_legacy
{
    struct dmx_instream_ram_legacy stream[DMX_LINUX_INPUT_DEV_CNT_LEGACY];
};






struct dmx_output_device 
{
    /* Char dev id in linux system.
    */
    dev_t linux_dev_id;

    /* Linux char device for dmx. 
     */
    struct cdev cdev;

    struct device *device;

    /* Data source device id of this output device.
    */
    __u32 src_hw_interface_id;
};


struct dmx_input_device 
{
    /* Char dev id in linux system.
    */
    dev_t linux_dev_id;

    /* Linux char device for dmx. 
     */
    struct cdev cdev;

    struct device *device;

    /* Data destination device id of this input device.
    */  
    __u32 dest_dev_id;
};


struct dmx_linux_interface_module
{
    struct class *output_device_class;
    struct dmx_output_device output_device[DMX_LINUX_OUTPUT_DEV_CNT];

    struct class *input_device_class;
    struct dmx_input_device input_device[DMX_LINUX_INPUT_DEV_CNT];
};


/* Provide backward compartibility with old linux native APIs.
*/
struct dmx_linux_interface_module_legacy
{
    struct class *output_device_class;
    struct dmx_output_device output_device[DMX_LINUX_OUTPUT_DEV_CNT_LEGACY];

    struct class *input_device_class;
    struct dmx_input_device input_device[DMX_LINUX_INPUT_DEV_CNT_LEGACY];
};


struct dmx_hw_interface
{
    //enum DMX_HW_INTERFACE_TYPE type;
    enum DMX_HW_INTERFACE_STATE State;

    /* ID of this HW interface.
	*/
	__u32 self_id;
	
    /* Combin with hw_dev_id to identify an unique HW.
	*/
    enum DMX_HW_INTERFACE_TYPE hw_dev_type;

    /* Combin with hw_dev_type to identify  an unique HW.
	*/
    __u32 hw_dev_id; 
    
    /* Interface callbacks for hw operation.
    */  
    __s32 (*hw_flt_pid_set)(__u32 dev_id, __u32  flt_idx, __u32 pid);
    __s32 (*hw_flt_enable)(__u32 dev_id, __u32 flt_idx);
    __s32 (*hw_flt_disable)(__u32 dev_id, __u32 flt_idx);
    __u32 (*hw_flt_total_cnt_get)(__u32 dev_id);

    __u32 (*hw_buf_rd_get)(__u32 dev_id);
    __s32 (*hw_buf_rd_set)(__u32 dev_id, __u32 rd);
    __u32 (*hw_buf_wr_get)(__u32 dev_id);
    __s32 (*hw_buf_wr_set)(__u32 dev_id, __u32 rd);
    __u32 (*hw_buf_end_get)(__u32 dev_id);
    __u32 (*hw_buf_start_addr_get)(__u32 dev_id);

    __s32 (*hw_pcr_enable)(__u32 dev_id, __u32 pcr_pid, __s32 pcr_value_cb(__u32 pcr, __u32 param), __u32 param);
    __s32 (*hw_pcr_disable)(__u32 dev_id, __u32 pcr_pid);

    __s32 (*hw_bypass_enable)(__u32 dev_id);
    __s32 (*hw_bypass_disable)(__u32 dev_id);   
};


struct dmx_hw_interface_module
{
    struct dmx_hw_interface interface[DMX_LINUX_OUTPUT_DEV_CNT];
};

#ifdef CONFIG_FAST_COPY

enum DMX_PLAYBACK_IN_STATUS
{
    DMX_PLAYBACK_IN_IDLE,
    //DMX_SEE_STATUS_READY,
    DMX_PLAYBACK_IN_RUN
};

enum DMX_TYPE
{
    DMX_TYPE_HW,
    DMX_TYPE_SW
};
#endif



__s32 dmx_pid_flt_parse(__u32 dmx_id, __u8 *ts_pkt_addr);
__s32 dmx_pid_flt_register(__u32 dmx_id, __u32 pid, __s32(* pid_flt_cb)(struct dmx_ts_pkt_inf * pkt_inf, __u32 cb_param), __u32 cb_param);
__s32 dmx_pid_flt_start(__s32 flt_idx);
__s32 dmx_pid_flt_stop(__s32 flt_idx);
__s32 dmx_pid_flt_unregister(__s32 flt_idx);
__u32 dmx_pid_flt_ts_in_cnt_get(__s32 flt_idx);
__s32 dmx_pid_flt_module_init(void);


__s32 dmx_ts_flt_register(__u32 dmx_id, __u16 pid, __s32(*ts_pkt_cb)(struct dmx_ts_pkt_inf *pkt_inf, __u32 cb_param), __u32 cb_param);
__s32 dmx_ts_flt_start(__s32 flt_idx);
__s32 dmx_ts_flt_stop(__s32 flt_idx);
__s32 dmx_ts_flt_unregister(__s32 flt_idx);
__u32 dmx_ts_flt_link_pid_flt_idx(__s32 ts_flt_idx);
__s32 dmx_ts_flt_module_init(void);


__s32 dmx_sec_flt_register(__u32 dmx_id, __u16 pid, struct Ali_DmxSecMaskInfo *mask, __s32 sec_data_cb(__u8 *src, __s32 len, enum DMX_SEC_FLT_CB_TYPE cb_type, __u32 cb_param), __u32 cb_param);
__s32 dmx_sec_flt_start(__s32 flt_idx);
__s32 dmx_sec_flt_stop(__s32 flt_idx);
__s32 dmx_sec_flt_unregister(__s32 flt_idx);
__u32 dmx_sec_flt_link_ts_flt_idx(__s32 sec_flt_idx);
__s32 dmx_sec_flt_module_init(void);

__s32 dmx_pcr_flt_register(__u32 dmx_id, __u16 pid, __s32 pcr_value_cb(__u32 pcr, __u32 value_cb_param), __u32 value_cb_param, __s32 pcr_pkt_cb(struct dmx_ts_pkt_inf *pkt_inf, __u32 pkt_cb_param), __u32 pkt_cb_param);
__s32 dmx_pcr_flt_start(__s32 flt_idx);
__s32 dmx_pcr_flt_stop(__s32 flt_idx);
__s32 dmx_pcr_flt_unregister(__s32 flt_idx);
__u32 dmx_pcr_flt_link_ts_flt_idx(__s32 pcr_flt_idx);
__s32 dmx_pcr_flt_module_init(void);


__s32 dmx_pes_flt_register(__u32 dmx_id,__u16 pid,__s32 (*pes_data_cb)(__u8 *src,__s32 len, enum DMX_PES_FLT_CB_TYPE cb_type,__u32 cb_param),__u32  cb_param);
__s32 dmx_pes_flt_start(__s32 flt_idx);
__s32 dmx_pes_flt_stop(__s32 flt_idx);
__s32 dmx_pes_flt_unregister(__s32 flt_idx);
__u32 dmx_pes_flt_link_ts_flt_idx(__s32 pes_flt_idx);
__s32 dmx_pes_flt_module_init( void );


__u32 dmx_data_buf_first_pkt_len(struct ali_dmx_data_buf * data_buf);
__s32 dmx_data_buf_drop_incomplete_pkt(struct ali_dmx_data_buf * data_buf);
__s32 dmx_data_buf_flush_all_pkt(struct ali_dmx_data_buf * data_buf);
__s32 dmx_data_buf_wr_data(struct ali_dmx_data_buf *dest_buf, __u8 *src, __s32 wr_len, enum DMX_DATA_SRC_TYPE data_src);
__s32 dmx_data_buf_wr_pkt_end(struct ali_dmx_data_buf *data_buf);
__s32 dmx_data_buf_rd(void *dest, struct ali_dmx_data_buf *src_buf, __s32 rd_len, enum DMX_DATA_CPY_DEST data_pkt_dest);
__s32 dmx_data_buf_list_init(struct ali_dmx_data_buf *data_buf);
__u32 dmx_data_buf_total_len(struct ali_dmx_data_buf *data_buf);
__s32 dmx_data_buf_module_init(void);
__s32 dmx_data_buf_module_release(void);


__s32 dmx_stream_read(struct file * file, char __user * buf, size_t count, loff_t * ppos);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long dmx_stream_ioctl(struct file * filp, unsigned int cmd, unsigned long arg);
#else
__s32 dmx_stream_ioctl(struct inode * inode, struct file * filp, unsigned int cmd, unsigned long arg);
#endif

__s32 dmx_stream_open(struct inode * inode, struct file * file);
__s32 dmx_stream_close(struct inode * inode, struct file * file);
__u32 dmx_stream_poll(struct file * filp, struct poll_table_struct * wait);
__s32 dmx_stream_module_init(void);


__s32 dmx_hw_flt_pid_set(__u32 dev_id, __s32 flt_idx, __s32 pid);
__s32 dmx_hw_flt_enable(__u32 dev_id, __u32 flt_idx);
__s32 dmx_hw_flt_disable(__u32 dev_id, __u32 flt_idx);
__u32 dmx_hw_flt_total_cnt_get(__u32 dev_id);
__u32 dmx_hw_buf_rd_get(__u32 dev_id);
__s32 dmx_hw_buf_rd_set(__u32 dev_id, __u32 rd);
__u32 dmx_hw_buf_wr_get(__u32 dev_id);
__s32 dmx_hw_buf_wr_set(__u32 dev_id, __u32 wr);
__u32 dmx_hw_buf_end_get(__u32 dev_id);
__u32 dmx_hw_buf_start_addr_get(__u32 dev_id);
__s32 dmx_hw_pcr_detect_enable(__u32 dev_id, __u32 pid, __s32 pcr_value_cb(__u32 pcr, __u32 param), __u32 param);
__s32 dmx_hw_pcr_detect_disable(__u32 dev_id, __u32 pid);
__s32 dmx_hw_bypass_enable(__u32 dev_id);
__s32 dmx_hw_bypass_disable(__u32 dev_id);
__s32 dmx_hw_init_m37(__u32 dev_id, struct dmx_hw_interface *interface);
__s32 dmx_hw_from_user_init(__u32 dev_id, struct dmx_hw_interface * interface);
__s32 dmx_hw_from_see_init(__u32 dev_id, struct dmx_hw_interface * interface);

__s32 dmx_hw_interface_init(__u32 hw_interface_id, enum DMX_HW_INTERFACE_TYPE hw_dev_type, __u32 hw_dev_id);

__s32 dmx_ts_in_ram_stream_write(struct file * file, const char __user * src_buf, size_t usr_wr_len, loff_t * ppos);
__s32 dmx_ts_in_ram_stream_open(struct inode * inode, struct file * file);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long dmx_ts_in_ram_stream_ioctl(struct file * filp, unsigned int cmd, unsigned long arg);
#else
__s32 dmx_ts_in_ram_stream_ioctl(struct inode * inode, struct file * filp, unsigned int cmd, unsigned long arg);
#endif
__s32 dmx_ts_in_ram_stream_close(struct inode * inode, struct file * file);
__u32 dmx_ts_in_ram_stream_poll(struct file * filp, struct poll_table_struct * wait);


__s32 dmx_mutex_output_lock(__u32 dmx_id);
__s32 dmx_mutex_output_unlock(__u32 dmx_id);
__s32 dmx_mutex_input_lock(__u32 dmx_id);
__s32 dmx_mutex_input_unlock(__u32 dmx_id);
__s32 dmx_mutex_module_init(void);

__s32 dmx_mutex_lock(__u32 dmx_id);
__s32 dmx_mutex_unlock(__u32 dmx_id);

__u32 dmx_data_engine_output_ts_in_cnt_get(__u32 src_hw_interface_id);
__s32 dmx_data_engine_module_init_kern(__u32 src_hw_interface_id, __u8 * engine_name, enum DMX_DATA_ENGINE_SRC src_type);
__s32 dmx_data_engine_module_init_usr(__u32 src_hw_interface_id, __u8 * engine_name, enum DMX_DATA_ENGINE_SRC src_type);

__s32 dmx_linux_output_interface_init(__u32 src_hw_interface_id, __u8 *interface_name);
__s32 dmx_linux_input_interface_init(__u32 dest_dev_id, __u8 *interface_name);

/* Support linux legacy interface.
*/
__s32 dmx_channel_read(struct file * file, char __user * buf, size_t count, loff_t * ppos);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long dmx_channel_ioctl(struct file * filp, unsigned int cmd, unsigned long arg);
#else
__s32 dmx_channel_ioctl(struct inode * inode, struct file * filp, unsigned int cmd, unsigned long arg);
#endif
__s32 dmx_channel_open(struct inode * inode, struct file * file);
__s32 dmx_channel_close(struct inode * inode, struct file * file);
__u32 dmx_channel_poll(struct file * filp, struct poll_table_struct * wait);
__s32 dmx_channel_module_legacy_init(void);

__s32 dmx_instream_ram_legacy_write(struct file * file, const char __user * src_buf, size_t usr_wr_len, loff_t * ppos);
__s32 dmx_instream_ram_legacy_open(struct inode * inode, struct file * file);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long dmx_instream_ram_legacy_ioctl(struct file * filp, unsigned int cmd, unsigned long arg);
#else
__s32 dmx_instream_ram_legacy_ioctl(struct inode * inode, struct file * filp, unsigned int cmd, unsigned long arg);
#endif
__s32 dmx_instream_ram_legacy_close(struct inode * inode, struct file * file);
__u32 dmx_instream_ram_legacy_poll(struct file * filp, struct poll_table_struct * wait);


__s32 dmx_linux_interface_module_legacy_init(__u32 dmx_id, __u8 * interface_name, enum DMX_LINUX_INTERFACE_TYPE interface_type, __u32 interface_id);

__s32 dmx_self_test_init(void);


__s32 dmx_subt_if_init(__u32 src_hw_if_id, __u8 *this_if_name);


__s32 dmx_statistic_show_init(void);

#endif




