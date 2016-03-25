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

#include <rpc_hld/ali_rpc_hld_vbi.h>

#include "../ali_rpc.h"


/*****************************************************************************
*	HLD_VBI															  		  *
*****************************************************************************/
enum HLD_VBI_FUNC{
    FUNC_VBI_OPEN = 0,
    FUNC_VBI_CLOSE,
    FUNC_VBI_IO_CONTROL,
    FUNC_VBI_SET_OUTPUT,
    FUNC_VBI_START,
    FUNC_VBI_STOP,
    FUNC_VBI_DEFAULT_GO_SET,
};

UINT32 desc_vbi_p_uint32[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, 4),  
  1, DESC_P_PARA(0, 1, 0),
  //desc of pointer ret
  0,                          
  0,
};

UINT32 desc_vbi_io_ctrl_p_uint32[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, 4),  
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};

UINT32 desc_vbi_io_ctrl_ttx_page_info[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct ttx_page_info)),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

/*
 * 	Name		:   vbi_open()   	
 *	Description	:   Open a vbi device
 *	Parameter	:	struct vbi_device *dev		: Device to be openned
 *	Return		:	INT32 						: Return value
 *
 */
INT32 vbi_open(struct vbi_device *dev)
{
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(1<<16)|FUNC_VBI_OPEN, NULL);
	
}

/*
 * 	Name		:   vbi_close()   	
 *	Description	:   Close a vbi device
 *	Parameter	:	struct vbi_device *dev		: Device to be closed
 *	Return		:	INT32 						: Return value
 *
 */
INT32 vbi_close(struct vbi_device *dev)
{
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(1<<16)|FUNC_VBI_CLOSE, NULL);
	
}


/*
 * 	Name		:   vbi_io_control()   	
 *	Description	:   vbiel IO control command
 *	Parameter	:	struct sto_device *dev		: Device
 *					INT32 cmd					: IO command
 *					UINT32 param				: Param
 *	Return		:	INT32 						: Result
 *
 */
INT32 vbi_ioctl(struct vbi_device *dev, UINT32 cmd, UINT32 param)
{
	
	UINT32 *desc = NULL;

    switch(cmd)
    {
    case IO_VBI_WORK_MODE_SEPERATE_TTX_SUBT:
    case IO_VBI_WORK_MODE_HISTORY :
    case IO_VBI_ENGINE_OPEN:
    case IO_VBI_ENGINE_CLOSE:
    case IO_VBI_ENGINE_UPDATE_PAGE:
    case IO_VBI_ENGINE_SHOW_ON_OFF:
    case IO_VBI_ENGINE_SEND_KEY:
    case IO_VBI_ENGINE_SET_CUR_LANGUAGE:
    default:
        desc = NULL;
        break;
    case IO_VBI_ENGINE_GET_STATE:
        desc = desc_vbi_io_ctrl_p_uint32;
        break;
    case IO_VBI_ENGINE_UPDATE_INIT_PAGE:
    case IO_VBI_ENGINE_UPDATE_SUBT_PAGE:
	desc = desc_vbi_io_ctrl_ttx_page_info;
	break;
    }
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(3<<16)|FUNC_VBI_IO_CONTROL, desc);

	
}

void vbi_setoutput(struct vbi_device *dev,T_VBIRequest *pVBIRequest)
{
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(2<<16)|FUNC_VBI_SET_OUTPUT, desc_vbi_p_uint32);
}

INT32 vbi_start(struct vbi_device *dev,t_TTXDecCBFunc pCBFunc)
{
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(2<<16)|FUNC_VBI_START, NULL);
	
}

INT32 vbi_stop(struct vbi_device *dev)
{
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(1<<16)|FUNC_VBI_STOP, NULL);
	
}

void ttx_default_g0_set(struct vbi_device *dev, UINT8 default_g0_set)
{
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(2<<16)|FUNC_VBI_DEFAULT_GO_SET, NULL);
}

//For S3602F, below 2 functions shouldnt be called from CPU, leaving them is just for link issue when link to old DMX
/*
INT32 vbi_request_write(void * pdev, UINT32 uSizeRequested,void ** ppuData,UINT32* puSizeGot,struct control_block* pDataCtrlBlk)
{
    ASSERT(0);
}

void vbi_update_write(void * pdev, UINT32 uDataSize)
{
    ASSERT(0);
}
*/

/*****************************************************************************
*	LLD_VBI																	  *
*****************************************************************************/
enum LLD_VBI_FUNC{
    FUNC_VBI_M33_ATTACH = 0,   
    FUNC_VBI_M33_ENABLE_TTX_BY_OSD,
    FUNC_VBI_M33_ENABLE_VBI_TRANSFER,
    FUNC_VBI_M33_GET_INITIAL_PAGE,
    FUNC_VBI_M33_GET_INITIAL_PAGE_STATUS,
    FUNC_VBI_M33_GET_FIRST_TTX_PAGE,
};


static UINT32 desc_cfg_param[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct vbi_config_par)), 
  1, DESC_P_PARA(0, 0, 0), 
  //desc of pointer ret
  0,                          
  0,
};

void vbi_m33_attach(struct vbi_config_par * cfg_param)
{
    jump_to_func(NULL, ali_rpc_call, cfg_param, (LLD_VBI_M33_MODULE<<24)|(1<<16)|FUNC_VBI_M33_ATTACH, desc_cfg_param);
}

void vbi_enable_ttx_by_osd(struct vbi_device*pdev)
{
    jump_to_func(NULL, ali_rpc_call, pdev, (LLD_VBI_M33_MODULE<<24)|(1<<16)|FUNC_VBI_M33_ENABLE_TTX_BY_OSD, NULL);
}

void enable_vbi_transfer(BOOL enable)
{
    jump_to_func(NULL, ali_rpc_call, enable, (LLD_VBI_M33_MODULE<<24)|(1<<16)|FUNC_VBI_M33_ENABLE_VBI_TRANSFER, NULL);
}

UINT16 get_inital_page()
{
	
	jump_to_func(NULL, ali_rpc_call, null, (LLD_VBI_M33_MODULE<<24)|(0<<16)|FUNC_VBI_M33_GET_INITIAL_PAGE, NULL);
	
}

UINT8 get_inital_page_status()
{
	
	jump_to_func(NULL, ali_rpc_call, null, (LLD_VBI_M33_MODULE<<24)|(0<<16)|FUNC_VBI_M33_GET_INITIAL_PAGE_STATUS, NULL);
	
}

UINT16 get_first_ttx_page()
{
	
	jump_to_func(NULL, ali_rpc_call, null, (LLD_VBI_M33_MODULE<<24)|(0<<16)|FUNC_VBI_M33_GET_FIRST_TTX_PAGE, NULL);
	
}


/*****************************************************************************
*	LIB_TTX																	  *
*****************************************************************************/
enum LIB_TTX_FUNC{
    FUNC_LIB_TTX_INIT = 0,   
    FUNC_LIB_TTX_Attach,
};

UINT32 desc_ttx_eng_attach[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct ttx_config_par)),
  1, DESC_P_PARA(0, 0, 0), 
  //desc of pointer ret
  0,                          
  0,
};

void TTXEng_Init(void)
{
	jump_to_func(NULL, ali_rpc_call, null, (LIB_TTX_MODULE<<24)|(0<<16)|FUNC_LIB_TTX_INIT, NULL);
}

void  TTXEng_Attach(struct ttx_config_par *pconfig_par)
{
	jump_to_func(NULL, ali_rpc_call, pconfig_par, (LIB_TTX_MODULE<<24)|(1<<16)|FUNC_LIB_TTX_Attach, desc_ttx_eng_attach);
}

