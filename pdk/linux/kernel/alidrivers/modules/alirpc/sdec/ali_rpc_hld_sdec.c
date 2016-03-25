#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
//#include <linux/smp_lock.h>
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

#include "../ali_rpc.h"

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

	jump_to_func(NULL, ali_rpc_call, cfg_param, (LLD_SDEC_SW_MODULE<<24)|(1<<16)|FUNC_SDEC_M33_ATTACH, desc_cfg_param);

}

void subt_disply_bl_init(struct sdec_device *dev)
{
	jump_to_func(NULL, ali_rpc_call, dev, (LLD_SDEC_SW_MODULE<<24)|(1<<16)|FUNC_SDEC_SBUT_DISPLAY_BL_INIT, NULL);
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
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_SDEC_MODULE<<24)|(1<<16)|FUNC_SDEC_OPEN, NULL);
	
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

	jump_to_func(NULL, ali_rpc_call, dev, (HLD_SDEC_MODULE<<24)|(1<<16)|FUNC_SDEC_CLOSE, NULL);

}

INT32  sdec_start(struct sdec_device *dev,UINT16 composition_page_id,UINT16 ancillary_page_id)
{

	jump_to_func(NULL, ali_rpc_call, dev, (HLD_SDEC_MODULE<<24)|(3<<16)|FUNC_SDEC_START, NULL);

}

INT32  sdec_stop(struct sdec_device *dev)
{

	jump_to_func(NULL, ali_rpc_call, dev, (HLD_SDEC_MODULE<<24)|(1<<16)|FUNC_SDEC_STOP, NULL);

}

INT32  sdec_pause(struct sdec_device *dev)
{

	jump_to_func(NULL, ali_rpc_call, dev, (HLD_SDEC_MODULE<<24)|(1<<16)|FUNC_SDEC_PAUSE, NULL);

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
    jump_to_func(NULL, ali_rpc_call, psubt_config_par, (LIB_SUBT_MODULE<<24)|(1<<16)|FUNC_LIB_SUBT_ATTACH, desc_lib_subt_attach);
}

void osd_subt_enter()
{
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_OSD_SUBT_ENTER, NULL);
}

void osd_subt_leave()
{
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_OSD_SUBT_LEAVE, NULL);
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
    jump_to_func(NULL, ali_rpc_call, null, (LIB_ISDBTCC_MODULE<<24)|(0<<16)|FUNC_LIB_ISDBT_INIT, NULL);
}

void osd_isdbtcc_enter(void)
{
    jump_to_func(NULL, ali_rpc_call, null, (LIB_ISDBTCC_MODULE<<24)|(0<<16)|FUNC_OSD_ISDBT_ENTER, NULL);
}

void osd_isdbtcc_leave(void)
{
    jump_to_func(NULL, ali_rpc_call, null, (LIB_ISDBTCC_MODULE<<24)|(0<<16)|FUNC_OSD_ISDBT_LEAVE, NULL);
}

void lib_isdbtcc_attach(struct isdbtcc_config_par *pcc_config_par)
{
    jump_to_func(NULL, ali_rpc_call, null, (LIB_ISDBTCC_MODULE<<24)|(1<<16)|FUNC_LIB_ISDBT_ATTACH, desc_cc_fg_para);
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
	

    jump_to_func(NULL, ali_rpc_call, cfg_param, (LLD_ISDBTCC_MODULE<<24)|(1<<16)|FUNC_ISDBTCC_DEC_ATTACH, desc_cfg_param2);


}

void isdbtcc_disply_init(struct sdec_device *dev)
{
    jump_to_func(NULL, ali_rpc_call, dev, (LLD_ISDBTCC_MODULE<<24)|(1<<16)|FUNC_ISDBTCC_DISPLAY_INIT, NULL);
}

void  isdbtcc_get_cur_lang_by_pid(UINT16 pid, UINT32 para)
{
    jump_to_func(NULL, ali_rpc_call, pid, (LLD_ISDBTCC_MODULE<<24)|(2<<16)|FUNC_ISDBTCC_GET_CUR_LANG, NULL);
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

UINT8 *g_sec_buf; // cpu use
void lib_subt_atsc_attach(void* p_config)
{
	struct atsc_subt_config_par* par=(struct atsc_subt_config_par*)p_config;	
	g_sec_buf=par->sec_buf_addr;
	//libc_printf("%s-%d\n",__FUNCTION__,__LINE__);
	//libc_printf("%x-g_sec_buf(%x), len(%d)\n",(UINT8*)p_config,par->sec_buf_addr,par->sec_buf_len);
	lib_subt_atsc_set_buf(p_config);
}

void lib_subt_atsc_set_buf(struct atsc_subt_config_par* p_config)
{
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(1<<16)|FUNC_LIB_SUBT_ATSC_SET_BUF, desc_lib_subt_atsc_set_buf);
}

UINT16 lib_subt_atsc_stream_identify(UINT16 length,UINT8 *data)
{
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(2<<16)|FUNC_LIB_ATSC_STREAM_IDENTIFY, NULL);
}

BOOL lib_subt_atsc_create_task(void)
{
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_LIB_SUBT_ATSC_CREATE_TASK, NULL);
}

BOOL lib_subt_atsc_terminate_task(void)
{
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_LIB_SUBT_ATSC_TERMINATE_TASK, NULL);
}

void lib_subt_atsc_clean_up(void)
{
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_LIB_SUBT_ATSC_CLEAN_UP, NULL);
}

void lib_subt_atsc_delete_timer(void)
{
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(0<<16)|FUNC_LIB_SUBT_ATSC_DELETE_TIMER, NULL);
}

void lib_subt_atsc_show_onoff(BOOL onoff)
{
    jump_to_func(NULL, ali_rpc_call, null, (LIB_SUBT_MODULE<<24)|(1<<16)|FUNC_LIB_SUBT_ATSC_SHOW_ONOFF, NULL);
}


