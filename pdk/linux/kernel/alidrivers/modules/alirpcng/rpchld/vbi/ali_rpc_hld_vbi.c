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

#include <rpc_hld/ali_rpc_hld_vbi.h>

//#include "../ali_rpc.h"

#include <ali_rpcng.h>

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
    #if 0
	register RET_CODE ret asm("$2");
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(1<<16)|FUNC_VBI_OPEN, NULL);
	return ret;
    #endif
    INT32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    ret = RpcCallCompletion(RPC_vbi_open,&p1,NULL);
    return ret;
    
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
    #if 0
	register RET_CODE ret asm("$2");
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(1<<16)|FUNC_VBI_CLOSE, NULL);
	return ret;
    #endif
    INT32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    ret = RpcCallCompletion(RPC_vbi_close,&p1,NULL);
    return ret;

}


#if 1
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
	//register RET_CODE ret asm("$2");
	UINT32 *desc = NULL;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);               
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, 4, &cmd);               
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, 4, &param);               

    switch(cmd)
    {
        case IO_VBI_ENGINE_GET_STATE:
            //desc = desc_vbi_io_ctrl_p_uint32;
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, 4, &param); /*Changed by tony on 2014/05/14*/
            break;
        case IO_VBI_ENGINE_UPDATE_INIT_PAGE:
        case IO_VBI_ENGINE_UPDATE_SUBT_PAGE:
    	    //desc = desc_vbi_io_ctrl_ttx_page_info;
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Ttx_page_info_rpc, sizeof(Ttx_page_info_rpc), param);
    	    break;
    
        case IO_VBI_ENGINE_SET_CUR_LANGUAGE:
    		RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, 4, param); 
    		break;
    		
        case IO_VBI_WORK_MODE_SEPERATE_TTX_SUBT:
        case IO_VBI_WORK_MODE_HISTORY :
        case IO_VBI_ENGINE_OPEN:
        case IO_VBI_ENGINE_CLOSE:
        case IO_VBI_ENGINE_UPDATE_PAGE:
        case IO_VBI_ENGINE_SHOW_ON_OFF:
        case IO_VBI_ENGINE_SEND_KEY:
        default:
            //desc = NULL;
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, 4, &param);               
            break;
    }
	//jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(3<<16)|FUNC_VBI_IO_CONTROL, desc);
    Int32 ret;

        	printk("%s,%d,page_addr:%x\n", __FUNCTION__, __LINE__);			
    ret = RpcCallCompletion(RPC_vbi_ioctl,&p1,&p2,&p3,NULL);  

        	printk("%s,%d,page_addr:%x\n", __FUNCTION__, __LINE__);		
	return ret;
}
#else
INT32 vbi_ioctl(struct vbi_device *dev, UINT32 cmd, UINT32 param)
{
	//register RET_CODE ret asm("$2");
	UINT32 *desc = NULL;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);               
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, 4, &cmd);               
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, 4, &param);               

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
        //desc = NULL;
        RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, 4, &param);               
        break;
    case IO_VBI_ENGINE_GET_STATE:
        //desc = desc_vbi_io_ctrl_p_uint32;
        RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, 4, param);
        break;
    case IO_VBI_ENGINE_UPDATE_INIT_PAGE:
    case IO_VBI_ENGINE_UPDATE_SUBT_PAGE:
	//desc = desc_vbi_io_ctrl_ttx_page_info;
    RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Ttx_page_info_rpc, sizeof(Ttx_page_info_rpc), param);
	break;
    }
	//jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(3<<16)|FUNC_VBI_IO_CONTROL, desc);
    Int32 ret;
        
    ret = RpcCallCompletion(RPC_vbi_ioctl,&p1,&p2,&p3,NULL);  
    
	return ret;
}

#endif

void vbi_setoutput(struct vbi_device *dev,T_VBIRequest *pVBIRequest)
{
	//jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(2<<16)|FUNC_VBI_SET_OUTPUT, desc_vbi_p_uint32);
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    //RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_UINT32, sizeof(UINT32), &pVBIRequest);
    
    //printk("%s,%d\n", __FUNCTION__, __LINE__);	
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_UINT32, sizeof(UINT32), pVBIRequest);
    //printk("%s,%d\n", __FUNCTION__, __LINE__);	    
    RpcCallCompletion(RPC_vbi_setoutput,&p1,&p2,NULL);  
}

INT32 vbi_start(struct vbi_device *dev,t_TTXDecCBFunc pCBFunc)
{
    #if 0
	register RET_CODE ret asm("$2");
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(2<<16)|FUNC_VBI_START, NULL);
	return ret;
    #endif
    INT32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &pCBFunc);

    ret = RpcCallCompletion(RPC_vbi_start,&p1,&p2,NULL);
    return ret;
    
}

INT32 vbi_stop(struct vbi_device *dev)
{
    #if 0
	register RET_CODE ret asm("$2");
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(1<<16)|FUNC_VBI_STOP, NULL);
	return ret;
    #endif
    INT32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    ret = RpcCallCompletion(RPC_vbi_stop,&p1,NULL);
    return ret;
   
}

void ttx_default_g0_set(struct vbi_device *dev, UINT8 default_g0_set)
{
	//jump_to_func(NULL, ali_rpc_call, dev, (HLD_VBI_MODULE<<24)|(2<<16)|FUNC_VBI_DEFAULT_GO_SET, NULL);

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &default_g0_set);

    RpcCallCompletion(RPC_ttx_default_g0_set,&p1,&p2,NULL);

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
    //jump_to_func(NULL, ali_rpc_call, cfg_param, (LLD_VBI_M33_MODULE<<24)|(1<<16)|FUNC_VBI_M33_ATTACH, desc_cfg_param);

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Vbi_config_par_rpc, sizeof(Vbi_config_par_rpc), cfg_param);


    RpcCallCompletion(RPC_vbi_m33_attach,&p1,NULL);

}

void vbi_enable_ttx_by_osd(struct vbi_device*pdev)
{
    //jump_to_func(NULL, ali_rpc_call, pdev, (LLD_VBI_M33_MODULE<<24)|(1<<16)|FUNC_VBI_M33_ENABLE_TTX_BY_OSD, NULL);

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &pdev);

    RpcCallCompletion(RPC_vbi_enable_ttx_by_osd,&p1,NULL);
}

void enable_vbi_transfer(BOOL enable)
{
    //jump_to_func(NULL, ali_rpc_call, enable, (LLD_VBI_M33_MODULE<<24)|(1<<16)|FUNC_VBI_M33_ENABLE_VBI_TRANSFER, NULL);

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_BOOL, sizeof(BOOL), &enable);

    RpcCallCompletion(RPC_enable_vbi_transfer,&p1,NULL);

}

UINT16 get_inital_page()
{
    #if 0
	register RET_CODE ret asm("$2");
	jump_to_func(NULL, ali_rpc_call, null, (LLD_VBI_M33_MODULE<<24)|(0<<16)|FUNC_VBI_M33_GET_INITIAL_PAGE, NULL);
	return ret;
    #endif
    Int32 ret;
    ret = RpcCallCompletion(RPC_get_inital_page,NULL);
    return (UINT16)ret;
}

UINT8 get_inital_page_status()
{
    #if 0
	register RET_CODE ret asm("$2");
	jump_to_func(NULL, ali_rpc_call, null, (LLD_VBI_M33_MODULE<<24)|(0<<16)|FUNC_VBI_M33_GET_INITIAL_PAGE_STATUS, NULL);
	return ret;
    #endif
    Int32 ret;
    ret = RpcCallCompletion(RPC_get_inital_page_status,NULL);
    return (UINT8)ret;

}

UINT16 get_first_ttx_page()
{
    #if 0
	register RET_CODE ret asm("$2");
	jump_to_func(NULL, ali_rpc_call, null, (LLD_VBI_M33_MODULE<<24)|(0<<16)|FUNC_VBI_M33_GET_FIRST_TTX_PAGE, NULL);
	return ret;
    #endif
    Int32 ret;
    ret = RpcCallCompletion(RPC_get_first_ttx_page,NULL);
    return (UINT16)ret;

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
	//jump_to_func(NULL, ali_rpc_call, null, (LIB_TTX_MODULE<<24)|(0<<16)|FUNC_LIB_TTX_INIT, NULL);

    RpcCallCompletion(RPC_TTXEng_Init,NULL);
}

void  TTXEng_Attach(struct ttx_config_par *pconfig_par)
{
	//jump_to_func(NULL, ali_rpc_call, pconfig_par, (LIB_TTX_MODULE<<24)|(1<<16)|FUNC_LIB_TTX_Attach, desc_ttx_eng_attach);

    Ttx_config_par_rpc ttx_con;

    ttx_con.erase_unknown_packet = pconfig_par->erase_unknown_packet;
    ttx_con.ttx_sub_page         = pconfig_par->ttx_sub_page;
    ttx_con.parse_packet26_enable= pconfig_par->parse_packet26_enable;
    ttx_con.user_fast_text       = pconfig_par->user_fast_text;
    ttx_con.no_ttx_descriptor    = pconfig_par->no_ttx_descriptor;
    ttx_con.sys_sdram_size_2m    = pconfig_par->sys_sdram_size_2m;
    ttx_con.hdtv_support_enable  = pconfig_par->hdtv_support_enable;
    ttx_con.ttx_vscrbuf          = pconfig_par->ttx_vscrbuf;
    ttx_con.ttx_pallette         = pconfig_par->ttx_pallette;
    ttx_con.ttx_cyrillic_1_support= pconfig_par->ttx_cyrillic_1_support;
    ttx_con.ttx_cyrillic_2_support= pconfig_par->ttx_cyrillic_2_support;
    ttx_con.ttx_cyrillic_3_support= pconfig_par->ttx_cyrillic_3_support;
    ttx_con.ttx_greek_support     = pconfig_par->ttx_greek_support;
    ttx_con.ttx_arabic_support    = pconfig_par->ttx_arabic_support;
    ttx_con.ttx_hebrew_support    = pconfig_par->ttx_hebrew_support;
    ttx_con.ttx_cyrillic_g2_support=pconfig_par->ttx_cyrillic_g2_support;
    ttx_con.ttx_cyrillic_g2_support=pconfig_par->ttx_cyrillic_g2_support;
    ttx_con.ttx_arabic_g2_support =pconfig_par->ttx_arabic_g2_support;
    ttx_con.ttx_g3_support        =pconfig_par->ttx_g3_support;
    ttx_con.ttx_color_number      =pconfig_par->ttx_color_number;
    ttx_con.ttx_subpage_addr      =pconfig_par->ttx_subpage_addr;
    ttx_con.osd_layer_id          =pconfig_par->osd_layer_id;
    
    ttx_con.get_ttxchar_from_cyrillic_1 = (UINT32)pconfig_par->get_ttxchar_from_cyrillic_1;
    ttx_con.get_ttxchar_from_cyrillic_2 = (UINT32)pconfig_par->get_ttxchar_from_cyrillic_2;
    ttx_con.get_ttxchar_from_cyrillic_3 = (UINT32)pconfig_par->get_ttxchar_from_cyrillic_3; 
    ttx_con.get_ttxchar_from_greek      = (UINT32)pconfig_par->get_ttxchar_from_greek;
    ttx_con.get_ttxchar_from_arabic     = (UINT32)pconfig_par->get_ttxchar_from_arabic;
    ttx_con.get_ttxchar_from_hebrew     = (UINT32)pconfig_par->get_ttxchar_from_hebrew;
    ttx_con.get_ttxchar_from_g2         = (UINT32)pconfig_par->get_ttxchar_from_g2;
    ttx_con.get_ttxchar_from_cyrillic_g2= (UINT32)pconfig_par->get_ttxchar_from_cyrillic_g2;
    ttx_con.get_ttxchar_from_greek_g2   = (UINT32)pconfig_par->get_ttxchar_from_greek_g2;
    ttx_con.get_ttxchar_from_arabic_g2  = (UINT32)pconfig_par->get_ttxchar_from_arabic_g2;
    ttx_con.get_ttxchar_from_g3         = (UINT32)pconfig_par->get_ttxchar_from_g3;
    ttx_con.ttx_drawchar                = (UINT32)pconfig_par->ttx_drawchar;
    ttx_con.osd_get_scale_para          = (UINT32)pconfig_par->osd_get_scale_para;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Ttx_config_par_rpc, sizeof(Ttx_config_par_rpc), &ttx_con);

    RpcCallCompletion(RPC_TTXEng_Attach,&p1,NULL);

}

