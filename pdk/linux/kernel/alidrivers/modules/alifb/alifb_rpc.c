/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2009 Copyright (C)
 *  (C)
 *  File: alifb_rpc.c
 *  (I)
 *  Description: rpc hld vpo opeartion
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.04.09				Sam		Create
 ****************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/div64.h>

#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_hld_dis.h>
#include <rpc_hld/ali_rpc_hld_decv.h>
#include <rpc_hld/ali_rpc_hld_gma.h>

#include <ali_video_common.h>
#include <ali_decv_plugin_common.h>

#include <ali_cache.h>
#include <ali_shm.h>

#ifndef CONFIG_RPC_HLD_GMA
#include "ali_fb.h"
#include "ali_gma_m36f_lld.h"
#endif

extern volatile unsigned long *g_ali_fb_rpc_arg[MAX_FB_RPC_ARG_NUM];
extern volatile int g_ali_fb_rpc_arg_size[MAX_FB_RPC_ARG_NUM];
extern struct vpo_device *g_vpo_dev;
extern struct vpo_device *g_sd_vpo_dev;
extern struct gma_device *g_gma_dev0;

extern unsigned long __G_ALI_MM_STILL_FRAME_SIZE;
extern unsigned long __G_ALI_MM_STILL_FRAME_START_ADDR;

#define Y_BUF_SIZE	 0x1FE000
#define C_BUF_SIZE	 0xFF000
#define MAF_BUF_SIZE 0x2000

extern int g_fb_gma_disable_enhance;

static int m_still_frame_init = 0;
static int m_still_frame_fail = 0;
static UINT32 m_frame_y_buffer = 0;
static UINT32 m_frame_c_buffer = 0;
static UINT32 m_frame_maf_buffer = 0;

struct backup_pic_addr
{
    UINT8 *backup_pic_y_buffer;
    UINT8 *backup_pic_c_buffer;
    UINT8 *backup_pic_maf_buffer;
};

static struct backup_pic_addr cc_backup_info;

static int io_control(struct vpo_device *dev)
{
	int ret = RET_FAILURE;

    #ifdef CONFIG_RPC_HLD_GMA
	if(*(unsigned long *)(g_ali_fb_rpc_arg[0]) == VPO_IO_GET_CURRENT_DISPLAY_INFO)
	{
		struct vpo_io_get_picture_info *dis_info
			= (struct vpo_io_get_picture_info *)g_ali_fb_rpc_arg[1];

		if(m_still_frame_fail == 1)
			return ret;

		if(m_still_frame_init == 0)
		{
			if((__G_ALI_MM_STILL_FRAME_SIZE < (Y_BUF_SIZE + C_BUF_SIZE + MAF_BUF_SIZE))
				|| (__G_ALI_MM_STILL_FRAME_START_ADDR == 0))
			{
				m_still_frame_fail = 1;
				return ret;
			}

            #if defined(CONFIG_ALI_CHIP_M3921)
			m_frame_y_buffer = __G_ALI_MM_STILL_FRAME_START_ADDR;
            #else
			m_frame_y_buffer = __G_ALI_MM_STILL_FRAME_START_ADDR & 0x9FFFFFFF;
            #endif

			m_frame_c_buffer = m_frame_y_buffer + Y_BUF_SIZE;
			m_frame_maf_buffer = m_frame_c_buffer + C_BUF_SIZE;
			m_still_frame_init = 1;
		}

		if(dis_info->de_index > 0)
			return ret;

		ret = vpo_ioctl(dev, VPO_IO_GET_CURRENT_DISPLAY_INFO, (UINT32)dis_info);
		if((ret == RET_SUCCESS) 
			&& (dis_info->status == 2))
		{
			//printk("dis info y_add %x y_size %x c_add %x c_size %x m_addr %x m_size %x\n"
			//	, dis_info->top_y, dis_info->y_buf_size, dis_info->top_c, dis_info->c_buf_size
			//	, dis_info->maf_buffer, dis_info->maf_buf_size);
			void *src = NULL;

            #if defined(CONFIG_ALI_CHIP_M3921)
			src = (void *)__VSTMALI(dis_info->top_y);
            #else
			src = (void *)((unsigned long)dis_info->top_y & 0x9FFFFFFF);
            #endif
			memcpy((void *)m_frame_y_buffer, src, dis_info->y_buf_size);
			__CACHE_FLUSH_ALI((unsigned long)m_frame_y_buffer, dis_info->y_buf_size);

            #if defined(CONFIG_ALI_CHIP_M3921)
			src = (void *)__VSTMALI(dis_info->top_c);
            #else
			src = (void *)((unsigned long)dis_info->top_c & 0x9FFFFFFF);
            #endif			
			memcpy((void *)m_frame_c_buffer, src, dis_info->c_buf_size);			
			__CACHE_FLUSH_ALI((unsigned long)m_frame_c_buffer, dis_info->c_buf_size);			
			
			if((dis_info->maf_buf_size != 0) && (dis_info->maf_buf_size < MAF_BUF_SIZE))
			{
                #if defined(CONFIG_ALI_CHIP_M3921)
				src = (void *)__VSTMALI(dis_info->maf_buffer);
                #else
				src = (void *)((unsigned long)dis_info->maf_buffer & 0x9FFFFFFF);
                #endif				
				memcpy((void *)m_frame_maf_buffer, src, dis_info->maf_buf_size);
				
				__CACHE_FLUSH_ALI((unsigned long)m_frame_maf_buffer, dis_info->maf_buf_size);
			}		
			
			dis_info->top_y = __VMTSALI(m_frame_y_buffer);
			dis_info->top_c = __VMTSALI(m_frame_c_buffer);
			dis_info->maf_buffer = __VMTSALI(m_frame_maf_buffer);
			vpo_ioctl(dev, VPO_IO_BACKUP_CURRENT_PICTURE, (UINT32)dis_info);			
		}
		
		return ret;
	}

	if(*(unsigned long *)(g_ali_fb_rpc_arg[0]) == VPO_IO_CLEAN_CURRENT_STILL_PICTURE)
	{
		struct vpo_io_get_picture_info disp_info;

		if(m_still_frame_fail == 1)
			return ret;

		if(m_still_frame_init == 0)
		{
			if((__G_ALI_MM_STILL_FRAME_SIZE < (Y_BUF_SIZE + C_BUF_SIZE + MAF_BUF_SIZE))
				|| (__G_ALI_MM_STILL_FRAME_START_ADDR == 0))
			{
				m_still_frame_fail = 1;
				return ret;
			}
			
			m_frame_y_buffer = __G_ALI_MM_STILL_FRAME_START_ADDR & 0x9FFFFFFF;
			m_frame_c_buffer = m_frame_y_buffer + Y_BUF_SIZE;
			m_frame_maf_buffer = m_frame_c_buffer + C_BUF_SIZE;
			m_still_frame_init = 1;
		}

		disp_info.de_index = 0;
		ret = vpo_ioctl(dev, VPO_IO_GET_CURRENT_DISPLAY_INFO, (UINT32)&disp_info);

		if((ret == RET_SUCCESS) && (disp_info.status == 2))
		{
			memset((void *)m_frame_y_buffer, 0x10, disp_info.y_buf_size);
			__CACHE_FLUSH_ALI((unsigned long)m_frame_y_buffer, disp_info.y_buf_size);

			memset((void *)m_frame_c_buffer, 0x80, disp_info.c_buf_size);
			__CACHE_FLUSH_ALI((unsigned long)m_frame_c_buffer, disp_info.c_buf_size);
				
			disp_info.top_y = m_frame_y_buffer;
			disp_info.top_c = m_frame_c_buffer;
			disp_info.maf_buffer = 0;//m_frame_maf_buffer;			
			vpo_ioctl(dev, VPO_IO_BACKUP_CURRENT_PICTURE, (UINT32)&disp_info);
		}

		return ret;
	}
    #else
    if(*(unsigned long *)(g_ali_fb_rpc_arg[0]) == VPO_IO_BACKUP_CURRENT_PICTURE)
	{
        struct vpo_io_get_picture_info info_bak;
        struct vpo_io_get_picture_info *dis_info = (struct vpo_io_get_picture_info *)g_ali_fb_rpc_arg[1];
        UINT32 blk_pic = dis_info->reserved[0];
        
        if(dis_info->de_index > 0)
			return ret;

		ret = vpo_ioctl(dev, VPO_IO_GET_CURRENT_DISPLAY_INFO, (UINT32)dis_info);
		if((ret == RET_SUCCESS) && (dis_info->status == 2))
		{
            if(cc_backup_info.backup_pic_y_buffer != NULL || cc_backup_info.backup_pic_c_buffer != NULL || cc_backup_info.backup_pic_maf_buffer)
                return RET_FAILURE;

            cc_backup_info.backup_pic_y_buffer = __VSTMALI(__G_ALI_MM_STILL_FRAME_START_ADDR); 
            cc_backup_info.backup_pic_c_buffer = __VSTMALI(((__G_ALI_MM_STILL_FRAME_START_ADDR + 255) & 0xFFFFFF00) + Y_BUF_SIZE);   

            if(cc_backup_info.backup_pic_y_buffer == NULL || cc_backup_info.backup_pic_c_buffer == NULL)
                return RET_FAILURE;

            memset(&info_bak, 0, sizeof(struct vpo_io_get_picture_info));
            
            if(cc_backup_info.backup_pic_y_buffer != NULL)
            {
                info_bak.top_y = ((UINT32)cc_backup_info.backup_pic_y_buffer+255) & 0xFFFFFF00;
                if(blk_pic == TRUE)
                {
                    memset((void *)info_bak.top_y, 0x10, dis_info->y_buf_size);
                }
                else
                {
                memcpy((void *)info_bak.top_y, (void *)((unsigned long)dis_info->top_y & 0x8FFFFFFF), dis_info->y_buf_size);
                }
    			dma_cache_wback((unsigned long)info_bak.top_y, dis_info->y_buf_size);
            }
            
            if(cc_backup_info.backup_pic_c_buffer != NULL)
            {
                info_bak.top_c = ((UINT32)cc_backup_info.backup_pic_c_buffer+255) & 0xFFFFFF00;
                if(blk_pic == TRUE)
                {
                    memset((void *)info_bak.top_c, 0x80, dis_info->c_buf_size);
                }
                else
                {
    			memcpy((void *)info_bak.top_c, (void *)((unsigned long)dis_info->top_c & 0x8FFFFFFF), dis_info->c_buf_size);
                }
    			dma_cache_wback((unsigned long)info_bak.top_c, dis_info->c_buf_size);	
            }
			if(dis_info->maf_buf_size != 0)
			{
                cc_backup_info.backup_pic_maf_buffer = (UINT32)kmalloc(dis_info->maf_buf_size + 256, GFP_KERNEL);
                if(cc_backup_info.backup_pic_maf_buffer != NULL)
                {
                    info_bak.maf_buffer = ((UINT32)cc_backup_info.backup_pic_maf_buffer+255) & 0xFFFFFF00;
                    if(blk_pic == TRUE)
                    {
                        memset((void *)info_bak.maf_buffer, 0x00, dis_info->maf_buf_size);
                    }
                    else
                    {
    				memcpy((void *)info_bak.maf_buffer, (void *)((unsigned long)dis_info->maf_buffer & 0x8FFFFFFF), dis_info->maf_buf_size);
                    }
    				dma_cache_wback((unsigned long)info_bak.maf_buffer, dis_info->maf_buf_size);
                }
			}		
		
			vpo_ioctl(dev, VPO_IO_BACKUP_CURRENT_PICTURE, &info_bak);	
		}
        
        return ret;
    }
    else if(*(unsigned long *)(g_ali_fb_rpc_arg[0]) == VPO_IO_FREE_BACKUP_PICTURE)
	{
        if(cc_backup_info.backup_pic_maf_buffer != NULL)
        {
            kfree(cc_backup_info.backup_pic_maf_buffer);
        }

		cc_backup_info.backup_pic_y_buffer = NULL;
		cc_backup_info.backup_pic_c_buffer = NULL;
		cc_backup_info.backup_pic_maf_buffer = NULL;
		
        return RET_SUCCESS;
    }    
	#endif

	switch(*(unsigned long *)(g_ali_fb_rpc_arg[0]))
	{
		case VPO_IO_REG_DAC:
		case VPO_IO_SET_BG_COLOR:
		case VPO_IO_SET_PARAMETER:	
		case VPO_IO_VIDEO_ENHANCE:
        case VPO_IO_GET_INFO:		
        case VPO_IO_GET_REAL_DISPLAY_MODE:	
		case VPO_IO_GET_TV_ASPECT:				
		case VPO_IO_GET_SRC_ASPECT:		
		case VPO_IO_GET_OUT_MODE:
		case VPO_IO_WRITE_TTX:
		case VPO_IO_SET_CUTLINE:
		case VPO_IO_DISPLAY_3D_ENABLE:
		case VPO_IO_SET_3D_ENH:
        case VPO_IO_SET_CGMS_INFO:
        case VPO_IO_AFD_CONFIG:
        case VPO_IO_GET_CURRENT_DISPLAY_INFO:
        case VPO_IO_SET_OSD_SHOW_TIME:
        case VPO_IO_GET_OSD0_SHOW_TIME:
        case VPO_IO_GET_OSD1_SHOW_TIME:
        case VPO_IO_GET_MP_SCREEN_RECT:
        case VPO_IO_GET_DISPLAY_MODE:
			ret = vpo_ioctl(dev, *(unsigned long *)(g_ali_fb_rpc_arg[0])
				, (UINT32)(g_ali_fb_rpc_arg[1]));

            #ifdef CONFIG_RPC_HLD_GMA
			if(g_fb_gma_disable_enhance == 0)
			{
				if((*(unsigned long *)(g_ali_fb_rpc_arg[0])) == VPO_IO_VIDEO_ENHANCE)
				{	
					gma_io_control(g_gma_dev0, GMA_IO_SET_ENHANCE_PAR, (UINT32)(g_ali_fb_rpc_arg[1]));
				}
			}
            #else
            gma_m36f_ioctl(0, GE_IO_SET_ENHANCE_PAR, (UINT32)(g_ali_fb_rpc_arg[1]));
			#endif
			break;
		case VPO_IO_UNREG_DAC:
		case VPO_IO_WRITE_WSS:
		case VPO_IO_MHEG_SCENE_AR:			
		case VPO_IO_MHEG_IFRAME_NOTIFY:   	
		case VPO_IO_DISAUTO_WIN_ONOFF:   	
		case VPO_IO_ENABLE_VBI:			
		case VPO_IO_PLAYMODE_CHANGE:
        case VPO_IO_DIT_CHANGE:			
		case VPO_IO_SWAFD_ENABLE:			
	    case VPO_IO_704_OUTPUT:
		case VPO_IO_SET_PREVIEW_MODE:
		case VPO_IO_HDMI_OUT_PIC_FMT:	
		case VPO_IO_SET_VBI_STARTLINE:
        case VPO_IO_SET_MULTIVIEW_MODE:
		case VPO_IO_SET_LAYER_ORDER:
		case VPO_IO_SET_DISPLAY_STYLE:
		case VPO_IO_ALWAYS_OPEN_CGMS_INFO:
        case VPO_IO_TVESDHD_SOURCE_SEL:
        case VPO_IO_SD_CC_ENABLE:
        case VPO_IO_SET_PREVIEW_SAR_MODE:
        case VPO_FULL_SCREEN_CVBS:
			ret = vpo_ioctl(dev, *(unsigned long *)(g_ali_fb_rpc_arg[0])
				, *(unsigned long *)(g_ali_fb_rpc_arg[1]));
			break;
		default:
			break;
	}

	return ret;
}

int hld_dis_rpc_operation(int hd_dev, int API_idx)
{
#define RPC_FB_OPEN					1
#define RPC_FB_CLOSE				2
#define RPC_FB_WIN_ON_OFF			3
#define RPC_FB_WIN_MODE			    4
#define RPC_FB_ZOOM					5
#define RPC_FB_ASPECT_MODE			6
#define RPC_FB_TVSYS				7
#define RPC_FB_TVSYS_EX				8
#define RPC_FB_IOCTL				9
#define RPC_FB_CONFIG_SOURCE_WINDOW	10
#define RPC_FB_SET_PROGRES_INTERL	11
#define RPC_FB_WIN_ON_OFF_EX        12
#define RPC_FB_ZOOM_EX              13

	int ret = RET_SUCCESS;
	struct vpo_device *cur_dev = NULL;

	cur_dev = hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
	switch(API_idx)
	{

	    #ifdef CONFIG_RPC_HLD_GMA
		case RPC_FB_OPEN:
			vpo_open(cur_dev, (struct VP_InitInfo *)g_ali_fb_rpc_arg[0]);
			break;
		case RPC_FB_CLOSE:
			vpo_close(cur_dev);
			break;
	    #endif
        
		case RPC_FB_WIN_ON_OFF:
			vpo_win_onoff(cur_dev, *(unsigned long *)(g_ali_fb_rpc_arg[0]));
			break;
            
	    #if 0//ndef CONFIG_RPC_HLD_GMA
        case RPC_FB_WIN_ON_OFF_EX:
			vpo_win_onoff_ext(cur_dev, *(unsigned long *)(g_ali_fb_rpc_arg[0])
               , *(unsigned long *)(g_ali_fb_rpc_arg[1]));
			break;
        #endif		
        
		case RPC_FB_WIN_MODE:
			vpo_win_mode(cur_dev, *(unsigned long *)(g_ali_fb_rpc_arg[0])
				, (struct MPSource_CallBack *)(g_ali_fb_rpc_arg[1])
				, (struct PIPSource_CallBack *)(g_ali_fb_rpc_arg[2]));
			break;
		case RPC_FB_ZOOM:
			vpo_zoom(cur_dev, (struct Rect *)(g_ali_fb_rpc_arg[0])
				, (struct Rect *)(g_ali_fb_rpc_arg[1]));
			break;
            
	    #if 0//ndef CONFIG_RPC_HLD_GMA			
        case RPC_FB_ZOOM_EX:
			vpo_zoom_ext(cur_dev, (struct Rect *)(g_ali_fb_rpc_arg[0])
				, (struct Rect *)(g_ali_fb_rpc_arg[1])
				, *(unsigned long *)(g_ali_fb_rpc_arg[2]));
			break;
        #endif
       
		case RPC_FB_ASPECT_MODE:
			vpo_aspect_mode(cur_dev, *(unsigned long *)(g_ali_fb_rpc_arg[0])
				, *(unsigned long *)(g_ali_fb_rpc_arg[1]));
			break;
		case RPC_FB_TVSYS:
			vpo_tvsys(cur_dev, *(unsigned long *)(g_ali_fb_rpc_arg[0]));
			break;
		case RPC_FB_TVSYS_EX:
			vpo_tvsys_ex(cur_dev, *(unsigned long *)(g_ali_fb_rpc_arg[0])
				, *(unsigned long *)(g_ali_fb_rpc_arg[1]));
			break;
		case RPC_FB_IOCTL:
			ret = io_control(cur_dev);
			break;
		case RPC_FB_CONFIG_SOURCE_WINDOW:
			ret = vpo_config_source_window(cur_dev, (struct vp_win_config_para *)(g_ali_fb_rpc_arg[0]));
			break;
		case RPC_FB_SET_PROGRES_INTERL:
			ret = vpo_set_progres_interl(cur_dev, *(unsigned long *)(g_ali_fb_rpc_arg[0]));
			break;
		default:
			break;
	}

	if(ret != RET_SUCCESS)
		ret = -1;	
	
	return ret;
}


#ifndef CONFIG_RPC_HLD_GMA
int hld_dis_fb_full_screen(int layer_id, int src_width, int src_height)
{
    struct vpo_io_get_info vpo_info;
	struct scale_pars pars;	
	gma_scale_param_t scale_pars;	
    
    memset((void *)&scale_pars, 0, sizeof(scale_pars));
    memset((void *)&pars, 0, sizeof(pars));
    memset((void *)&vpo_info, 0, sizeof(vpo_info));

	vpo_ioctl(g_vpo_dev, VPO_IO_GET_INFO, (UINT32)(&vpo_info));
	
	pars.scale_type = GMA_SCALE_FILTER;
	pars.h_src = src_width;	
	pars.v_src = src_height;	
    pars.h_dst = vpo_info.des_width;
    pars.v_dst = vpo_info.des_height;
            
  	scale_pars.tv_sys = vpo_info.tvsys;
   	scale_pars.h_div = pars.h_src;
  	scale_pars.h_mul = pars.h_dst;
   	scale_pars.v_div = pars.v_src;
  	scale_pars.v_mul = pars.v_dst;
    FB_PRF("gma scale full screen <%d %d> => <%d %d> tysys %d\n",
        pars.h_src, pars.v_src, pars.h_dst, pars.v_dst, vpo_info.tvsys);
    if(gma_m36f_scale (layer_id, GE_SET_SCALE_PARAM, (uint32)&scale_pars) == RET_SUCCESS) 
    {
		FB_PRF("gma scale to full screen done\n");
	}
	else
    {	
		FB_PRF("gma scale to full screen fail\n");
	}

	return 0;
}
#endif

