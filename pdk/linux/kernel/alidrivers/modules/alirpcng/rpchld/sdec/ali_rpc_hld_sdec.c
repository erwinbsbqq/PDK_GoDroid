#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
#include <linux/smp_lock.h>
#endif
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/vt.h>
#include <linux/init.h>
#include <linux/linux_logo.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/console.h>
#include <linux/kmod.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/efi.h>
#include <linux/fb.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/ali_rpc.h>

#include <rpc_hld/ali_rpc_hld_sdec.h>

//#include "../ali_rpc.h"

#include <ali_rpcng.h>
/*****************************************************************************
*	LLD_SDEC															  		  *
*****************************************************************************/
enum LLD_SDEC_SW_FUNC{
    FUNC_SDEC_M33_ATTACH = 0,   
    FUNC_SDEC_SBUT_DISPLAY_BL_INIT,
};

static UINT32 desc_cfg_param[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct sdec_feature_config)), 
  1, DESC_P_PARA(0, 0, 0), 
  //desc of pointer ret
  0,                          
  0,
};

INT32 sdec_m33_attach(struct sdec_feature_config * cfg_param)
{
#if 0
	register RET_CODE ret asm("$2");
	jump_to_func(NULL, ali_rpc_call, cfg_param, (LLD_SDEC_SW_MODULE<<24)|(1<<16)|FUNC_SDEC_M33_ATTACH, desc_cfg_param);
	return ret;
#endif
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Sdec_feature_config_rpc, sizeof(Sdec_feature_config_rpc), cfg_param);
    
    ret = RpcCallCompletion(RPC_sdec_m33_attach, &p1, NULL);
    return ret;
}

void subt_disply_bl_init(struct sdec_device *dev)
{
#if 0
	jump_to_func(NULL, ali_rpc_call, dev, (LLD_SDEC_SW_MODULE<<24)|(1<<16)|FUNC_SDEC_SBUT_DISPLAY_BL_INIT, NULL);
#endif

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    RpcCallCompletion(RPC_subt_disply_bl_init, &p1, NULL);
}

/*
//For S3602F, below 2 functions shouldnt be called from CPU, leaving them is just for link issue when link to old DMX
INT32 sdec_request_write(void * pdev, UINT32 uSizeRequested,void ** ppuData,UINT32* puSizeGot,struct control_block* pSDataCtrlBlk)
{
    SDEC_ASSERT(0);    
}

void sdec_update_write(void * pdev,UINT32 uDataSize)
{
    SDEC_ASSERT(0);
}
*/

/*****************************************************************************
*	HLD_SDEC															  		  *
*****************************************************************************/
enum HLD_SDEC_FUNC{
    FUNC_SDEC_OPEN = 0,
    FUNC_SDEC_CLOSE,
    FUNC_SDEC_START,
    FUNC_SDEC_STOP,
    FUNC_SDEC_PAUSE,
};

/*
 * 	Name		:   sdec_open()   	
 *	Description	:   Open a sdec device
 *	Parameter	:	struct sdec_device *dev		: Device to be openned
 *	Return		:	INT32 						: Return value
 *
 */
INT32 sdec_open(struct sdec_device *dev)
{
#if !defined(CONFIG_ALI_RPCNG)
	register RET_CODE ret asm("$2");
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_SDEC_MODULE<<24)|(1<<16)|FUNC_SDEC_OPEN, NULL);
	return ret;
#else

    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    
    ret = RpcCallCompletion(RPC_sdec_open, &p1, NULL);
    return ret;
#endif
}

/*
 * 	Name		:   sdec_close()   	
 *	Description	:   Close a sdec device
 *	Parameter	:	struct sdec_device *dev		: Device to be closed
 *	Return		:	INT32 						: Return value
 *
 */
INT32 sdec_close(struct sdec_device *dev)
{
#if !defined(CONFIG_ALI_RPCNG)
	register RET_CODE ret asm("$2");
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_SDEC_MODULE<<24)|(1<<16)|FUNC_SDEC_CLOSE, NULL);
	return ret;
#else
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    
    ret = RpcCallCompletion(RPC_sdec_close, &p1, NULL);
    return ret;
#endif    
}

INT32  sdec_start(struct sdec_device *dev,UINT16 composition_page_id,UINT16 ancillary_page_id)
{
#if !defined(CONFIG_ALI_RPCNG)
	register RET_CODE ret asm("$2");
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_SDEC_MODULE<<24)|(3<<16)|FUNC_SDEC_START, NULL);
	return ret;
#else
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT16, sizeof(Uint16), &composition_page_id);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT16, sizeof(Uint16), &ancillary_page_id);
    
    ret = RpcCallCompletion(RPC_sdec_start, &p1, &p2,&p3,NULL);
    return ret;
#endif    
}

INT32  sdec_stop(struct sdec_device *dev)
{
#if !defined(CONFIG_ALI_RPCNG)
	register RET_CODE ret asm("$2");
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_SDEC_MODULE<<24)|(1<<16)|FUNC_SDEC_STOP, NULL);
	return ret;
#else
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    
    ret = RpcCallCompletion(RPC_sdec_stop, &p1, NULL);
    return ret;
#endif
}

INT32  sdec_pause(struct sdec_device *dev)
{
#if !defined(CONFIG_ALI_RPCNG)
	register RET_CODE ret asm("$2");
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_SDEC_MODULE<<24)|(1<<16)|FUNC_SDEC_PAUSE, NULL);
	return ret;
#else
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    
    ret = RpcCallCompletion(RPC_sdec_pause, &p1, NULL);
    return ret;
#endif
}


/*****************************************************************************
*	LIB_SDEC															  		  *
*****************************************************************************/
enum LIB_SUBT_FUNC{
    FUNC_LIB_SUBT_ATTACH = 0,   
    FUNC_OSD_SUBT_ENTER,
    FUNC_OSD_SUBT_LEAVE,
// add for DCII Subt (also called SCTE Subt) , North American Cable market    
    FUNC_LIB_SUBT_ATSC_SET_BUF,	//vicky20110322    
    FUNC_LIB_ATSC_STREAM_IDENTIFY,	    
    FUNC_LIB_SUBT_ATSC_CREATE_TASK,		
    FUNC_LIB_SUBT_ATSC_TERMINATE_TASK,   	
    FUNC_LIB_SUBT_ATSC_CLEAN_UP,   	
    FUNC_LIB_SUBT_ATSC_DELETE_TIMER,   	
    FUNC_LIB_SUBT_ATSC_SHOW_ONOFF,
    FUNC_LIB_SUBT_ATSC_SECTION_PARSE_CREATE_TASK,
    FUNC_LIB_SUBT_ATSC_SECTION_PARSE_TERMINATE_TASK,
};

UINT32 desc_lib_subt_attach[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct subt_config_par)),
  1, DESC_P_PARA(0, 0, 0), 
  //desc of pointer ret
  0,                          
  0,
};

void lib_subt_attach(struct subt_config_par *psubt_config_par)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, psubt_config_par, (LIB_SUBT_MODULE<<24)|(1<<16)|FUNC_LIB_SUBT_ATTACH, desc_lib_subt_attach);
#else

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Subt_config_par_rpc, sizeof(Subt_config_par_rpc), psubt_config_par);
    RpcCallCompletion(RPC_lib_subt_attach, &p1, NULL);
#endif
}

void osd_subt_enter()
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_OSD_SUBT_ENTER, NULL);
#else

    RpcCallCompletion(RPC_osd_subt_enter, NULL);
#endif
}

void osd_subt_leave()
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_OSD_SUBT_LEAVE, NULL);
#else

    RpcCallCompletion(RPC_osd_subt_leave, NULL);
#endif
}

/* lib ISDBT CC */
enum LIB_ISDBT_FUNC{
    FUNC_LIB_ISDBT_INIT = 0,   
    FUNC_OSD_ISDBT_ENTER,
    FUNC_OSD_ISDBT_LEAVE,
    FUNC_LIB_ISDBT_ATTACH,   
};

static UINT32 desc_cc_fg_para[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct isdbtcc_config_par)), 
  1, DESC_P_PARA(0, 0, 0), 
  //desc of pointer ret
  0,                          
  0,
};

void lib_isdbtcc_init(void)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, null, (LIB_ISDBTCC_MODULE<<24)|(0<<16)|FUNC_LIB_ISDBT_INIT, NULL);
#else

    RpcCallCompletion(RPC_lib_isdbtcc_init, NULL);
#endif
}

void osd_isdbtcc_enter(void)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, null, (LIB_ISDBTCC_MODULE<<24)|(0<<16)|FUNC_OSD_ISDBT_ENTER, NULL);
#else

    RpcCallCompletion(RPC_osd_isdbtcc_enter, NULL);
#endif

}

void osd_isdbtcc_leave(void)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, null, (LIB_ISDBTCC_MODULE<<24)|(0<<16)|FUNC_OSD_ISDBT_LEAVE, NULL);
#endif

    RpcCallCompletion(RPC_osd_isdbtcc_leave, NULL);
}

void lib_isdbtcc_attach(struct isdbtcc_config_par *pcc_config_par)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, null, (LIB_ISDBTCC_MODULE<<24)|(1<<16)|FUNC_LIB_ISDBT_ATTACH, desc_cc_fg_para);
#else

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Isdbtcc_config_par_rpc, sizeof(Isdbtcc_config_par_rpc), pcc_config_par);
    RpcCallCompletion(RPC_lib_isdbtcc_attach, &p1, NULL);
#endif
}

/* lld isdbt CC */

enum LLD_ISDBTCC_FUNC{
    FUNC_ISDBTCC_DEC_ATTACH = 0,   
    FUNC_ISDBTCC_DISPLAY_INIT,
    FUNC_ISDBTCC_GET_CUR_LANG,  
};

static UINT32 desc_cfg_param2[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct sdec_feature_config)), 
  1, DESC_P_PARA(0, 0, 0), 
  //desc of pointer ret
  0,                          
  0,
};

/* ATSC SUBT */
UINT32 desc_lib_subt_atsc_set_buf[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct atsc_subt_config_par)),
  1, DESC_P_PARA(0, 0, 0), 
  //desc of pointer ret
  0,                          
  0,
};

INT32 isdbtcc_dec_attach(struct sdec_feature_config * cfg_param)
{
#if !defined(CONFIG_ALI_RPCNG)

	register RET_CODE ret asm("$2");	
	
    jump_to_func(NULL, ali_rpc_call, cfg_param, (LLD_ISDBTCC_MODULE<<24)|(1<<16)|FUNC_ISDBTCC_DEC_ATTACH, desc_cfg_param2);

	return ret;
#else
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Sdec_feature_config_rpc, sizeof(Sdec_feature_config_rpc), cfg_param);
    
    ret = RpcCallCompletion(RPC_isdbtcc_dec_attach, &p1, NULL);
    return ret;
#endif
}

void isdbtcc_disply_init(struct sdec_device *dev)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, dev, (LLD_ISDBTCC_MODULE<<24)|(1<<16)|FUNC_ISDBTCC_DISPLAY_INIT, NULL);
#else

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RpcCallCompletion(RPC_isdbtcc_disply_init, &p1, NULL);
#endif
}

void  isdbtcc_get_cur_lang_by_pid(UINT16 pid, UINT32 para)
{
#if !defined(CONFIG_ALI_RPCNG)

    jump_to_func(NULL, ali_rpc_call, pid, (LLD_ISDBTCC_MODULE<<24)|(2<<16)|FUNC_ISDBTCC_GET_CUR_LANG, NULL);
#else

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT16, sizeof(Uint16), &pid);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(Uint32), &para);

    RpcCallCompletion(RPC_isdbtcc_get_cur_lang_by_pid, &p1, &p2, NULL);
#endif
}

//For S3602F, below 2 functions shouldnt be called from CPU, leaving them is just for link issue when link to old DMX
INT32 sdec_request_write(void * pdev, UINT32 uSizeRequested,void ** ppuData,UINT32* puSizeGot,struct control_block* pSDataCtrlBlk)
{
    do{}while(0);

	return 0;
}

void sdec_update_write(void * pdev,UINT32 uDataSize)
{
    do{}while(0);
}
INT32 atsc_sdec_request_write(void* pdev,UINT32 uSizeRequested,UINT8** ppuData,UINT32* puSizeGot)
{
	do{}while(0);
	
		return 0;

 
}
void atsc_sdec_update_write(void* pdev,UINT32 uSize)
{
	do{}while(0);
	
		return 0;

	
}


void lib_subt_atsc_set_buf(struct atsc_subt_config_par* p_config)
{
#if !defined(CONFIG_ALI_RPCNG)

    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(1<<16)|FUNC_LIB_SUBT_ATSC_SET_BUF, desc_lib_subt_atsc_set_buf);
#else

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Atsc_subt_config_par_rpc, sizeof(Atsc_subt_config_par_rpc), p_config);
    RpcCallCompletion(RPC_lib_subt_atsc_set_buf, &p1, NULL);
#endif
}

UINT16 lib_subt_atsc_stream_identify(UINT16 length,UINT8 *data)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(2<<16)|FUNC_LIB_ATSC_STREAM_IDENTIFY, NULL);
#else

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &length);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), data);

    RpcCallCompletion(RPC_lib_subt_atsc_stream_identify, &p1, &p2, NULL);
#endif
}

BOOL lib_subt_atsc_create_task(void)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_LIB_SUBT_ATSC_CREATE_TASK, NULL);
#else

    RpcCallCompletion(RPC_lib_subt_atsc_create_task, NULL);
#endif
}

BOOL lib_subt_atsc_terminate_task(void)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_LIB_SUBT_ATSC_TERMINATE_TASK, NULL);
#else

    RpcCallCompletion(RPC_lib_subt_atsc_terminate_task, NULL);
#endif
}

BOOL lib_subt_atsc_section_parse_create_task(void)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_LIB_SUBT_ATSC_SECTION_PARSE_CREATE_TASK, NULL);
#else

    RpcCallCompletion(RPC_lib_subt_atsc_section_parse_create_task, NULL);
#endif
}

BOOL lib_subt_atsc_section_parse_terminate_task(void)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_LIB_SUBT_ATSC_SECTION_PARSE_TERMINATE_TASK, NULL);
#else

    RpcCallCompletion(RPC_lib_subt_atsc_section_parse_terminate_task, NULL);
#endif
}

void lib_subt_atsc_clean_up(void)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_LIB_SUBT_ATSC_CLEAN_UP, NULL);
#else

    RpcCallCompletion(RPC_lib_subt_atsc_clean_up, NULL);
#endif
}

void lib_subt_atsc_delete_timer(void)
{
#if !defined(CONFIG_ALI_RPCNG)

    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_LIB_SUBT_ATSC_DELETE_TIMER, NULL);
#else

    RpcCallCompletion(RPC_lib_subt_atsc_delete_timer, NULL);
#endif
}

void lib_subt_atsc_show_onoff(BOOL onoff)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(1<<16)|FUNC_LIB_SUBT_ATSC_SHOW_ONOFF, NULL);
#else

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_BOOL, sizeof(Bool), &onoff);
    RpcCallCompletion(RPC_lib_subt_atsc_show_onoff, &p1, NULL);
#endif
}


