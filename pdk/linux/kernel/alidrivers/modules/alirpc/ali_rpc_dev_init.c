/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_rpc_dev_init.c
 *  (I)
 *  Description: initialize the remote devices in the SEE CPU
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.06.02			Sam			Create
 ****************************************************************************/
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
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
//#include <ali_hdmi_common.h>

#include <rpc_hld/ali_rpc_hld.h>
#include <rpc_hld/ali_rpc_hld_dis.h>
#include <rpc_hld/ali_rpc_hld_decv.h>
#include <rpc_hld/ali_rpc_hld_deca.h>
#include <rpc_hld/ali_rpc_hld_snd.h>
#include <rpc_hld/ali_rpc_hld_gma.h>
#include <rpc_hld/ali_rpc_hld_vbi.h>
#include <rpc_hld/ali_rpc_hld_sdec.h>
#include <rpc_hld/ali_rpc_hld_dsc.h>
#include <rpc_hld/ali_rpc_hld_avsync.h>
#include <rpc_hld/ali_rpc_hld_dmx_see_init.h>
#include <ali_board_config.h>

#include "ali_rpc.h"

#if 0
#define PRF printk
#else
#define PRF(...) 	do{}while(0)
#endif

//#define JTAG_DEBUG

void hld_decv_set_cur_dev(int dev);

extern unsigned long __G_MM_AVC_FB_NUM;
extern unsigned long __G_MM_AVC_EXTRA_FB_NUM;      
extern unsigned long __G_MM_AVC_FB_ADDR;           
extern unsigned long __G_MM_AVC_FB_LEN;            
extern unsigned long __G_MM_AVC_DVIEW_ADDR;        
extern unsigned long __G_MM_AVC_DVIEW_LEN;        
extern unsigned long __G_MM_AVC_MV_ADDR;           
extern unsigned long __G_MM_AVC_MV_LEN;            
extern unsigned long __G_MM_AVC_VBV_ADDR; 		 
extern unsigned long __G_MM_AVC_VBV_LEN;		     
extern unsigned long __G_MM_AVC_CMD_QUEUE_ADDR;    
extern unsigned long __G_MM_AVC_CMD_QUEUE_LEN;     
extern unsigned long __G_MM_AVC_MB_COL_ADDR; 	     
extern unsigned long __G_MM_AVC_MB_COL_LEN;	     
extern unsigned long __G_MM_AVC_MB_NEI_ADDR; 	     
extern unsigned long __G_MM_AVC_MB_NEI_LEN;	     
extern unsigned long __G_MM_AVC_LAF_RW_BUF_ADDR;   
extern unsigned long __G_MM_AVC_LAF_RW_BUF_LEN;    
extern unsigned long __G_MM_AVC_LAF_FLAG_BUF_ADDR; 
extern unsigned long __G_MM_AVC_LAF_FLAG_BUF_LEN;  
extern unsigned long __G_MM_VDEC_FB_ADDR;           
extern unsigned long __G_MM_VDEC_FB_LEN;            
extern unsigned long __G_MM_VDEC_VBV_START_ADDR;    
extern unsigned long __G_MM_VDEC_VBV_LEN;           
extern unsigned long __G_MM_VDEC_LAF_FLAG_BUF_ADDR; 
extern unsigned long __G_MM_VDEC_LAF_FLAG_BUF_LEN;  
extern unsigned long g_support_continue_memory;
extern unsigned long g_support_afd_scale;
extern unsigned long g_vdec_first_i_freerun;
extern unsigned long __G_MM_VIDEO_TOP_ADDR;
extern unsigned long __G_MM_VIDEO_START_ADDR;
extern unsigned long __G_MM_VDEC_CMD_QUEUE_ADDR;

extern unsigned long __G_MM_PRIVATE_AREA_TOP_ADDR;
extern unsigned long __G_MM_PRIVATE_AREA_START_ADDR;

extern unsigned long __G_MM_BOOTLOGO_DATA_START_ADDR;

extern unsigned long __G_MM_OSD_BK_ADDR;
extern unsigned long __G_MM_VCAP_FB_SIZE;
extern unsigned long __G_MM_VCAP_FB_ADDR;
extern struct tve_adjust g_tve_adjust_table[];
extern struct tve_adjust g_sd_tve_adjust_table[];
extern struct tve_adjust g_sd_tve_adjust_table_adv[];
extern T_TVE_ADJUST_TABLE g_tve_adjust_table_total[];

extern void set_video_info_to_hdmi(UINT32 param);
extern void set_audio_info_to_hdmi(UINT32 param);
extern void ali_hdmi_set_bootloader_param(void *pParam);

/* default scart output 
	1 : RGB
	2 : YUV
*/
extern long g_tve_default_scart_output;

/* default tv mode */
extern long g_tve_hd_default_tv_mode;
extern long g_tve_sd_default_tv_mode;

/* whether use the CVBS output. the invalid value is -1 */
extern long g_tve_dac_use_cvbs_type;

/* whether use the SVIDEO output. the invalid value is -1 */
extern long g_tve_dac_use_svideo_type;

/* whether use the RGB output. the invalid value is -1 */
extern long g_tve_dac_use_rgb_type;

/* whether use the YUV output. the invalie value is -1 */
extern long g_tve_dac_use_yuv_type;
extern unsigned long support_isdbt_cc;

//unsigned long g_see_heap_top_addr = 0;

static int m_video_dview_disable = 0;
	
struct vdec_device*  g_decv_dev0 = NULL;
struct vdec_device*  g_decv_avc_dev0 = NULL;
struct vdec_device*  g_decv_avs_dev0 = NULL;
struct vpo_device*   g_vpo_dev = NULL;
EXPORT_SYMBOL(g_vpo_dev);
struct vpo_device*   g_sd_vpo_dev = NULL;
EXPORT_SYMBOL(g_sd_vpo_dev);

struct gma_device *g_gma_dev0 = NULL;

//#define __MM_VDEC_VBV_LEN				0x380000 //(3.5M)
#define __MM_VDEC_CMD_QUEUE_LEN 		(0x10000)
#define __MM_VDEC_LAF_FLAG_BUF_LEN 	(0x10800) // (0xC00*22)
#ifndef __MM_OSD2_LEN
#define __MM_OSD2_LEN					(0x1FA400) // (1920 * 1080)
#endif
#define __MM_TTX_BS_LEN				(0x5000)
#define __MM_TTX_PB_LEN	            		(0xCB200)
#define __MM_TTX_SUB_PAGE_LEN       		(0x14500)
#define __MM_TTX_P26_NATION_LEN   		(0x61A80)
#define __MM_TTX_P26_DATA_LEN       		(0x3E8)
#define __MM_SUB_BS_LEN				(0xA000)
#ifndef __MM_SUB_PB_LEN
#define __MM_SUB_PB_LEN				(0x50000)
#endif
#define __MM_SUB_HW_DATA_LEN 		(0xC000)

#define __MM_ISDBT_BS_LEN			(0x8EC0)
#define __MM_ISDBT_PB_LEN			(0x7E900)

extern unsigned long __G_MM_OSD_BK_ADDR;
extern unsigned long __G_MM_TTX_BS_START_ADDR;
extern unsigned long __G_MM_TTX_PB_START_ADDR;
extern unsigned long __G_MM_TTX_SUB_PAGE_BUF_ADDR;
extern unsigned long __G_MM_TTX_P26_NATION_BUF_ADDR;
extern unsigned long __G_MM_TTX_P26_DATA_BUF_ADDR;
extern unsigned long __G_MM_SUB_BS_START_ADDR;
extern unsigned long __G_MM_SUB_HW_DATA_ADDR;
extern unsigned long __G_MM_SUB_PB_START_ADDR;

extern unsigned long __G_MM_STILL_FRAME_SIZE;
extern unsigned long __G_MM_STILL_FRAME_ADDR;

extern unsigned long __G_MM_MP_MEM_TOP_ADDR;
extern unsigned long __G_MM_MP_MEM_START_ADDR;

extern unsigned long __G_SUB_PB_START_ADDR;
extern unsigned long __G_TTX_SUB_PAGE_BUF_START_ADDR;

struct vbi_device *g_vbi_config_dev = NULL;
struct vbi_config_par g_vbi_config;

#ifndef CONFIG_RPC_HLD_DECV 
RET_CODE vdec_io_control(struct vdec_device *dev, UINT32 io_code, UINT32 param)
{
}

RET_CODE vdec_set_output(struct vdec_device *dev, enum VDecOutputMode eMode,struct VDecPIPInfo *pInitInfo,  struct MPSource_CallBack *pMPCallBack, struct PIPSource_CallBack *pPIPCallBack)
{
}

RET_CODE vdec_profile_level(struct vdec_device *dev, UINT8 uProfileLevel,VDEC_BEYOND_LEVEL cb_beyond_level)
{
}

RET_CODE vdec_sync_mode(struct vdec_device *dev, UINT8 uSyncMode,UINT8 uSyncLevel)
{
}
#endif

#ifndef CONFIG_RPC_HLD_SND
RET_CODE snd_io_control(struct snd_device * dev, UINT32 cmd, UINT32 param)
{
}
#endif

#define 	OSD_TRANSPARENT_COLOR		0xFF

struct sdec_feature_config g_sdec_config;
struct sdec_device *g_sdec_device = NULL;

struct gma_device *g_gma_dev1 = NULL;

#ifdef CONFIG_RPC_HLD_DIS
static void vpo_dev_attach(void)
{

	struct VP_Feature_Config vp_config;
	struct Tve_Feature_Config tve_config;
    	struct Tve_Feature_Config sd_tve_config;	
    	vcap_attach_t vcap_param;	
	extern unsigned int g_otp_set_vdac_fs;

	memset((void *)&tve_config,0,sizeof(tve_config));
	memset((void *)&vp_config,0,sizeof(vp_config));
	memset((void *)&sd_tve_config, 0, sizeof(sd_tve_config));
	memset((void *)&vcap_param, 0, sizeof(vcap_param));
	
	vp_config.bAvoidMosaicByFreezScr = FALSE;    /* macro VDEC_AVOID_MOSAIC_BY_FREEZE_SCR */
	vp_config.bMHEG5Enable = FALSE;                  /* macro  _MHEG5_ENABLE_ */
	vp_config.bOsdMulitLayer = FALSE;                  /* macro OSD_MULTI_LAYER */
	vp_config.bOvershootSolution = FALSE;            /* macro VIDEO_OVERSHOOT_SOLUTION */
	vp_config.bP2NDisableMAF= TRUE;
	vp_config.bSupportExtraWin = FALSE;                /* macro VIDEO_SUPPORT_EXTRA_DVIEW_WINDOW */
	vp_config.bADPCMEnable= FALSE;                    /* macro VIDEO_ADPCM_ONOFF(VIDEO_ADPCM_ON:TRUE VIDEO_ADPCM_OFF:FALSE) */
	vp_config.pMPGetMemInfo = NULL;
	vp_config.pSrcAspRatioChange_CallBack = NULL;
	tve_config.config = YUV_SMPTE|TVE_TTX_BY_VBI|TVE_CC_BY_VBI;
	tve_config.config |= TVE_NEW_CONFIG_1;
	tve_config.tve_tbl_all = g_tve_adjust_table_total;
	vpo_m36f_attach(&vp_config, &tve_config);
	PRF("attach hd vpo done\n\n");
	
	vcap_param.fb_size = __G_MM_VCAP_FB_SIZE;//__MM_VCAP_FB_SIZE;
	vcap_param.fb_addr = __G_MM_VCAP_FB_ADDR;//(UINT32)kmalloc(vcap_param.fb_size + 255, GFP_KERNEL);	
	vcap_param.fb_addr &= ~255;
	vcap_m36f_attach(&vcap_param);
	PRF("attach vcap done %x\n\n", (int)vcap_param.fb_addr);
	
	sd_tve_config.config = YUV_SMPTE|TVE_TTX_BY_VBI|TVE_CC_BY_VBI;
	sd_tve_config.tve_adjust = g_sd_tve_adjust_table;
	sd_tve_config.tve_adjust_adv = g_sd_tve_adjust_table_adv;
	vpo_m36f_sd_attach(&vp_config, &sd_tve_config);

	vpo_ioctl((struct vpo_device*) hld_dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_OTP_SET_VDAC_FS, g_otp_set_vdac_fs);
	
	PRF("attach sd vpo done\n\n");
}

#ifdef CONFIG_RPC_HLD_SDEC
static void osd_dev_attach(void)
{
//#define __MM_OSD2_LEN					(0x1FA400) // (1920 * 1080)

	OSD_DRIVER_CONFIG osd_config;

	memset((void *)&osd_config,0,sizeof(OSD_DRIVER_CONFIG));
	osd_config.uMemBase = 0;
	osd_config.uMemSize = 0;
	osd_config.bStaticBlock = TRUE;

	osd_config.bDirectDraw = FALSE;
	osd_config.bCacheable = TRUE;
	osd_config.bVFilterEnable = TRUE;
	osd_config.bP2NScaleInNormalPlay = FALSE;
	osd_config.bP2NScaleInSubtitlePlay = TRUE;
	osd_config.uDViewScaleRatio[OSD_PAL][0] = 384;
	osd_config.uDViewScaleRatio[OSD_PAL][1] = 510;
	osd_config.uDViewScaleRatio[OSD_NTSC][0] = 384;
	osd_config.uDViewScaleRatio[OSD_NTSC][1] = 426;
	osd_config.af_par[0].id = 0;
	osd_config.af_par[0].vd = 1;
	osd_config.af_par[0].af = 1;
	osd_config.af_par[0].id = 1;
	osd_config.af_par[0].vd = 1;
	osd_config.af_par[0].af = 1;
	osd_m36f_attach("OSD_M36F_0", &osd_config);

	osd_config.bP2NScaleInNormalPlay = TRUE;
	osd_config.bP2NScaleInSubtitlePlay = TRUE;
	osd_config.uMemBase = __G_MM_OSD_BK_ADDR;
	osd_config.uMemSize = __MM_OSD2_LEN;
	osd_config.bP2NScaleInNormalPlay = TRUE;
	osd_m36f_attach("OSD_M36F_1", &osd_config);
}
#endif

#endif

#ifdef CONFIG_RPC_HLD_DECV	
/*
	VBV buffer : in the private buf area
	FB and MAF buffer : < 0xa7000000
*/

static void avc_config(struct vdec_avc_config_par *par)
{
#define MAX_EXTRA_FB_NUM        3
#define MAX_MB_WIDTH            120 //(1920/16)
#define MAX_MB_HEIGHT           68  //(1088/16)
#define MAX_MB_STRIDE           120 //120 MB alignment to improve AVC performance
#define ONE_FB_SIZE             (MAX_MB_STRIDE*MAX_MB_HEIGHT*256*3/2)
#define ONE_DV_FB_SIZE          ((MAX_MB_WIDTH*MAX_MB_HEIGHT*256*3/2)/4)
#define ONE_MV_SIZE             64*(MAX_MB_WIDTH*MAX_MB_HEIGHT)     //522240
#define AVC_FB_LEN		        ONE_FB_SIZE*(4+MAX_EXTRA_FB_NUM)    //0x1700000
#define AVC_DVIEW_LEN           ONE_DV_FB_SIZE*(4+MAX_EXTRA_FB_NUM) //0xb00000
#define AVC_MV_LEN		        ONE_MV_SIZE*(4+MAX_EXTRA_FB_NUM)    //0x37c800//0x2FD000
#define AVC_MB_COL_LEN		    0x11000 //0x22000
#define AVC_MB_NEI_LEN		    0x8000
#define AVC_CMD_QUEUE_LEN       0x10000 //In allegro test stream, this length could be bigger than 128k, however, in realistic, 0x10000 should be enough
#define AVC_LAF_RW_BUF_LEN      (((((MAX_MB_WIDTH*MAX_MB_HEIGHT)*48*2)+1023)&0x0ffffc00)*2)
#define AVC_LAF_FLAG_BUF_LEN    (0xc00*22) //when enable dual output, we need 1 more laf buffer

//#define AVC_VBV_LEN		    __MM_VDEC_VBV_LEN //if this size is set to 0x200000, vbv buffer overflow could happen in 20061007d_0.ts
#define AVC_VBV_ADDR 		    __G_MM_VDEC_VBV_START_ADDR //(__G_MM_PRIVATE_AREA_TOP_ADDR - AVC_VBV_LEN) 	//256 bytes alignment
#define AVC_CMD_QUEUE_ADDR      ((AVC_VBV_ADDR - AVC_CMD_QUEUE_LEN)&0xffffff00)  	//256 bytes alignment
#define AVC_LAF_FLAG_BUF_ADDR   ((AVC_CMD_QUEUE_ADDR - AVC_LAF_FLAG_BUF_LEN)&0xfffffc00)  //1024 bytes alignment

	memset((void *)par, 0, sizeof(*par));

    if(__G_MM_AVC_FB_ADDR != 0)
    {
       // unsigned long avc_fb_num            = __G_MM_AVC_FB_NUM;
        unsigned long avc_extra_fb_num      = __G_MM_AVC_EXTRA_FB_NUM;

        unsigned long avc_fb_addr 		    = __G_MM_AVC_FB_ADDR;
        unsigned long avc_fb_len		    = __G_MM_AVC_FB_LEN;
        unsigned long avc_dview_addr 	    = __G_MM_AVC_DVIEW_ADDR;
        unsigned long avc_dview_len         = __G_MM_AVC_DVIEW_LEN;
        unsigned long avc_mv_addr 		    = __G_MM_AVC_MV_ADDR;
        unsigned long avc_mv_len		    = __G_MM_AVC_MV_LEN;
        unsigned long avc_cpu_start_addr    = avc_mv_addr;

        unsigned long avc_vbv_addr 		    = __G_MM_AVC_VBV_ADDR;
        unsigned long avc_vbv_len		    = __G_MM_AVC_VBV_LEN;
        unsigned long avc_cmd_queue_addr    = __G_MM_AVC_CMD_QUEUE_ADDR;
        unsigned long avc_cmd_queue_len     = __G_MM_AVC_CMD_QUEUE_LEN;
        unsigned long avc_mb_col_addr 	    = __G_MM_AVC_MB_COL_ADDR;
        unsigned long avc_mb_col_len	    = __G_MM_AVC_MB_COL_LEN;
        unsigned long avc_mb_nei_addr 	    = __G_MM_AVC_MB_NEI_ADDR;
        unsigned long avc_mb_nei_len	    = __G_MM_AVC_MB_NEI_LEN;
        unsigned long avc_laf_rw_buf_addr   = __G_MM_AVC_LAF_RW_BUF_ADDR;
        unsigned long avc_laf_rw_buf_len    = __G_MM_AVC_LAF_RW_BUF_LEN;
        unsigned long avc_laf_flag_buf_addr = __G_MM_AVC_LAF_FLAG_BUF_ADDR;
        unsigned long avc_laf_flag_buf_len  = __G_MM_AVC_LAF_FLAG_BUF_LEN;
        unsigned long avc_see_start_addr    = avc_laf_flag_buf_addr;

        unsigned int chip_id = ali_sys_ic_get_chip_id();
    	
    	memset((void *)par, 0, sizeof(*par));

        if(chip_id == ALI_S3503)
        {
    		avc_mb_col_addr       = avc_mv_addr;
    		avc_mb_col_len        = avc_mv_len;
    		avc_mv_addr           = 0;
    		avc_mv_len            = 0;
    		avc_cpu_start_addr    = avc_mb_col_addr;
    		
    		avc_mb_nei_len        = 0x20000;
        }

    	par->max_additional_fb_num = avc_extra_fb_num;
    	par->memmap.support_multi_bank = FALSE;
    	par->memmap.frame_buffer_base = avc_fb_addr;
    	par->memmap.frame_buffer_len = avc_fb_len;
    	par->memmap.dv_frame_buffer_base = avc_dview_addr;
    	par->memmap.dv_frame_buffer_len = avc_dview_len;
        par->memmap.mv_buffer_base = avc_mv_addr;
    	par->memmap.mv_buffer_len = avc_mv_len;
    	par->memmap.mb_col_buffer_base = avc_mb_col_addr;
    	par->memmap.mb_col_buffer_len = avc_mb_col_len;
    	par->memmap.mb_neighbour_buffer_base = avc_mb_nei_addr;
    	par->memmap.mb_neighbour_buffer_len = avc_mb_nei_len;
    	par->memmap.cmd_queue_buffer_base = avc_cmd_queue_addr;
    	par->memmap.cmd_queue_buffer_len = avc_cmd_queue_len;
    	par->memmap.vbv_buffer_base = (avc_vbv_addr & 0x0fffffff) | 0x80000000;
    	par->memmap.vbv_buffer_len = avc_vbv_len;
    	par->memmap.laf_rw_buf = avc_laf_rw_buf_addr;
    	par->memmap.laf_rw_buffer_len = avc_laf_rw_buf_len;
    	par->memmap.laf_flag_buf = avc_laf_flag_buf_addr;
    	par->memmap.laf_flag_buffer_len = avc_laf_flag_buf_len;

        /* for seamless switch needs dynamic malloc fb */
        par->memmap.support_conti_memory = g_support_continue_memory;
        if(par->memmap.support_conti_memory == 2)
        {
            if(chip_id == ALI_S3503)
            {
                par->memmap.avc_mem_addr = avc_dview_addr;
                par->memmap.avc_mem_len  = __G_MM_VIDEO_TOP_ADDR - avc_dview_addr;
            }
            else
            {
                par->memmap.avc_mem_addr = avc_mv_addr;
                par->memmap.avc_mem_len  = __G_MM_VIDEO_TOP_ADDR - avc_mv_addr;
            }
        }
        par->dtg_afd_parsing = g_support_afd_scale;

        printk("AVC memory top address: CPU 0x%lx SEE 0x%lx\n",__G_MM_VIDEO_TOP_ADDR, __G_MM_PRIVATE_AREA_TOP_ADDR);
        printk("AVC CPU memory size %ld\n",__G_MM_VIDEO_TOP_ADDR - avc_cpu_start_addr);
        printk("AVC SEE memory size %ld\n",__G_MM_PRIVATE_AREA_TOP_ADDR - avc_see_start_addr);
    }
    else
    {
    	unsigned int avc_buf = __G_MM_VIDEO_TOP_ADDR;
    		
    	par->max_additional_fb_num = MAX_EXTRA_FB_NUM;
    	par->memmap.support_multi_bank = FALSE;

    	if(m_video_dview_disable == 1)
    	{
    		par->memmap.dv_frame_buffer_base = 0;
    		par->memmap.dv_frame_buffer_len = 0;
    	}
    	else
    	{
    		avc_buf = (avc_buf - AVC_FB_LEN)&0xfffffe00;		
    		par->memmap.frame_buffer_base = avc_buf;// AVC_FB_ADDR;
    		par->memmap.frame_buffer_len = AVC_FB_LEN;
    	
    		avc_buf = ((avc_buf -  AVC_DVIEW_LEN)&0xfffffe00);		
    		par->memmap.dv_frame_buffer_base = avc_buf;// AVC_DVIEW_ADDR;
    		par->memmap.dv_frame_buffer_len = AVC_DVIEW_LEN;
    	}

    	avc_buf = ((avc_buf - AVC_MV_LEN)&0xffffff00);
    	par->memmap.mv_buffer_base = avc_buf;//AVC_MV_ADDR;
    	par->memmap.mv_buffer_len = AVC_MV_LEN;

    	avc_buf = ((avc_buf - AVC_MB_COL_LEN)&0xffffff00);
    	par->memmap.mb_col_buffer_base = avc_buf;// AVC_MB_COL_ADDR;
    	par->memmap.mb_col_buffer_len = AVC_MB_COL_LEN;

    	avc_buf = ((avc_buf - AVC_MB_NEI_LEN)&0xffffff00);		
    	par->memmap.mb_neighbour_buffer_base = avc_buf;// AVC_MB_NEI_ADDR;
    	par->memmap.mb_neighbour_buffer_len = AVC_MB_NEI_LEN;
    	
    	par->memmap.vbv_buffer_base = (AVC_VBV_ADDR & 0x1fffffff) | 0x80000000;
    	par->memmap.vbv_buffer_len = __G_MM_VDEC_VBV_LEN;

    	avc_buf = ((avc_buf - AVC_LAF_RW_BUF_LEN)&0xfffffc00);
    	par->memmap.laf_rw_buf = avc_buf;// AVC_LAF_RW_BUF_ADDR;
    	par->memmap.laf_rw_buffer_len = AVC_LAF_RW_BUF_LEN;
    	
    	if(AVC_VBV_ADDR == 0)
    	{
    		avc_buf = (avc_buf - AVC_FB_LEN)&0xfffffe00;		
    		par->memmap.frame_buffer_base = avc_buf;// AVC_FB_ADDR;
    		par->memmap.frame_buffer_len = AVC_FB_LEN;
    		
    		par->memmap.cmd_queue_buffer_base = __G_MM_VDEC_CMD_QUEUE_ADDR;
    		par->memmap.cmd_queue_buffer_len = AVC_CMD_QUEUE_LEN;	
    		par->memmap.laf_flag_buf = __G_MM_VDEC_LAF_FLAG_BUF_ADDR;
    		par->memmap.laf_flag_buffer_len = AVC_LAF_FLAG_BUF_LEN;
    	}
    	else
    	{
    		par->memmap.cmd_queue_buffer_base = AVC_CMD_QUEUE_ADDR;
    		par->memmap.cmd_queue_buffer_len = AVC_CMD_QUEUE_LEN;	
    		par->memmap.laf_flag_buf = AVC_LAF_FLAG_BUF_ADDR;
    		par->memmap.laf_flag_buffer_len = AVC_LAF_FLAG_BUF_LEN;
    	}
    }
}

static void mpg2_config(struct vdec_config_par *par)
{
//#define __MM_VBV_LEN					__MM_VDEC_VBV_LEN//0x380000
#define __MM_VBV_START_ADDR			__G_MM_VDEC_VBV_START_ADDR //((__G_MM_PRIVATE_AREA_TOP_ADDR - __MM_VBV_LEN)&0XFFFFFF00)

#define __MM_FB_LEN			    		0x19c6200
#define __MM_FB_LEN_NO_DEVIEW		0x1700000
#define __MM_MAF_LEN					0x198C00// 0xd0000
#define __MM_DVW_LEN					0


	memset((void *)par, 0, sizeof(*par));

    if(0/*__G_MM_VDEC_FB_ADDR != 0*/)
    {
        par->mem_map.frm0_Y_size = __G_MM_VDEC_FB_LEN;
    	par->mem_map.frm0_C_size = 0;
    	par->mem_map.frm1_Y_size = 0;
    	par->mem_map.frm1_C_size = 0;
    	par->mem_map.frm2_Y_size = 0;
    	par->mem_map.frm2_C_size = 0;

    	par->mem_map.frm0_Y_start_addr = __G_MM_VDEC_FB_ADDR;
    	par->mem_map.frm0_C_start_addr = 0;
    	par->mem_map.frm1_Y_start_addr = 0;
    	par->mem_map.frm1_C_start_addr = 0;
    	par->mem_map.frm2_Y_start_addr = 0;
    	par->mem_map.frm2_C_start_addr = 0;

    	par->mem_map.dvw_frm_size = 0;
    	par->mem_map.dvw_frm_start_addr = 0;

    	par->mem_map.maf_size = __G_MM_VDEC_LAF_FLAG_BUF_LEN;
    	par->mem_map.maf_start_addr = __G_MM_VDEC_LAF_FLAG_BUF_ADDR;

    	par->mem_map.vbv_size = ((__G_MM_VDEC_VBV_LEN-4)&0xFFFFFF00);
    	par->mem_map.vbv_start_addr = ((__G_MM_VDEC_VBV_START_ADDR&0x1fffffff)|0x80000000);
    	par->mem_map.vbv_end_addr = ((par->mem_map.vbv_start_addr) +  par->mem_map.vbv_size - 1);

    	par->user_data_parsing = TRUE;     /* macro USER_DATA_PARSING */
    	par->dtg_afd_parsing = g_support_afd_scale; /* macro DTG_AFD_PARSE */
    	par->drop_freerun_pic_before_firstpic_show = (g_vdec_first_i_freerun==0)?TRUE:FALSE; /* macro VDEC27_DROP_FREERUN_BEFORE_FIRSTSHOW */
    	par->reset_hw_directly_when_stop = TRUE;  /* macro VDEC27_STOP_REST_HW_DIRECTLY*/
    	par->show_hd_service = FALSE;      /* macro SUPPORT_SHOW_HD_SERVICE */
    	par->still_frm_in_cc = FALSE;      /* macro STILL_FRAME_IN_CC */
    	par->not_show_mosaic = FALSE;      /* macro VDEC_AVOID_MOSAIC_BY_FREEZE_SCR */
    	par->adpcm.adpcm_mode = FALSE;     /* macro VIDEO_ADPCM_ONOFF(VIDEO_ADPCM_ON:TRUE VIDEO_ADPCM_OFF:FALSE)*/
    	par->adpcm.adpcm_ratio = 0;          
    	par->sml_frm.sml_frm_mode = FALSE; /* macro VDEC27_SML_FRM_ONOFF(VDEC27_SML_FRM_OFF: FALSE   VDEC27_SML_FRM_ON: TRUE)*/
    	par->return_multi_freebuf = TRUE;  /* macro VDEC27_SUPPORT_RETURN_MULTI_FREEBUF */ 
    	par->sml_frm.sml_frm_size = 0;     /* macro VDEC27_SML_FRM_SIZE*/
    	par->lm_shiftthreshold = 2;        /* macro VDEC27_LM_SHIFTTHRESHOLD*/
    	par->vp_init_phase = 0;            /* macro DEFAULT_MP_FILTER_ENABLE*/
    	par->preview_solution = VDEC27_PREVIEW_DE_SCALE; /* macro VDEC27_PREVIEW_SOLUTION(VDEC27_PREVIEW_VE_SCALE or VDEC27_PREVIEW_DE_SCALE)*/

        printk("VDEC memory top address: CPU 0x%lx SEE 0x%lx\n",__G_MM_VIDEO_TOP_ADDR,__G_MM_PRIVATE_AREA_TOP_ADDR);
        printk("VDEC CPU memory size %ld\n",__G_MM_VIDEO_TOP_ADDR - __G_MM_VDEC_FB_ADDR);
        printk("VDEC SEE memory size %ld\n",__G_MM_PRIVATE_AREA_TOP_ADDR - __G_MM_VDEC_LAF_FLAG_BUF_ADDR);
    }
    else
    {
        unsigned int mpeg_buf = 0;

    	if(m_video_dview_disable == 1)
    		par->mem_map.frm0_Y_size  = __MM_FB_LEN_NO_DEVIEW;
    	else
    		par->mem_map.frm0_Y_size  = __MM_FB_LEN;

    	mpeg_buf = ((__G_MM_VIDEO_TOP_ADDR - par->mem_map.frm0_Y_size)&0XFFFFFF00);
    	par->mem_map.frm0_C_size = 0;
    	par->mem_map.frm1_Y_size  = 0;
    	par->mem_map.frm1_C_size = 0;
    	par->mem_map.frm2_Y_size = 0;
    	par->mem_map.frm2_C_size = 0;

    	par->mem_map.frm0_Y_start_addr = mpeg_buf;
    	par->mem_map.frm0_C_start_addr = 0;
    	par->mem_map.frm1_Y_start_addr = 0;
    	par->mem_map.frm1_C_start_addr = 0;
    	par->mem_map.frm2_Y_start_addr = 0;
    	par->mem_map.frm2_C_start_addr = 0;

    	par->mem_map.dvw_frm_size = 0;
    	par->mem_map.dvw_frm_start_addr = 0;

    	par->mem_map.maf_size = __MM_MAF_LEN;
    	
    	mpeg_buf = (mpeg_buf - par->mem_map.maf_size)&0XFFFFFC00;
    	
    	par->mem_map.maf_start_addr = mpeg_buf;// __MM_MAF_START_ADDR;

    	if(__G_MM_VDEC_VBV_LEN == 0)
    	{
    		int vbv_len = __G_MM_MP_MEM_TOP_ADDR - __G_MM_MP_MEM_START_ADDR;
    		UINT32 vbv_start = (__G_MM_MP_MEM_START_ADDR + 15) & (0xFFFFFFF0);

    		vbv_len -= vbv_start - __G_MM_MP_MEM_START_ADDR;
    		
    		par->mem_map.vbv_size = ((vbv_len - 4)&0xFFFFFF00);
    		par->mem_map.vbv_start_addr = ((vbv_start&0x1fffffff)|0x80000000);
    		par->mem_map.vbv_end_addr = ((par->mem_map.vbv_start_addr) +  par->mem_map.vbv_size - 1);		
    	}
    	else
    	{
    		par->mem_map.vbv_size = ((__G_MM_VDEC_VBV_LEN-4)&0xFFFFFF00);
    		par->mem_map.vbv_start_addr = ((__MM_VBV_START_ADDR&0x1fffffff)|0x80000000);
    		par->mem_map.vbv_end_addr = ((par->mem_map.vbv_start_addr) +  par->mem_map.vbv_size - 1);
    	}

    	par->user_data_parsing = TRUE; /* macro USER_DATA_PARSING */
    	par->dtg_afd_parsing = FALSE; /* macro DTG_AFD_PARSE */
    	par->drop_freerun_pic_before_firstpic_show = FALSE; /* macro VDEC27_DROP_FREERUN_BEFORE_FIRSTSHOW */
    	par->reset_hw_directly_when_stop = TRUE;  /* macro VDEC27_STOP_REST_HW_DIRECTLY*/
    	par->show_hd_service = FALSE;      /* macro SUPPORT_SHOW_HD_SERVICE */
    	par->still_frm_in_cc = FALSE;          /* macro STILL_FRAME_IN_CC */
    	par->not_show_mosaic = FALSE;      /* macro VDEC_AVOID_MOSAIC_BY_FREEZE_SCR */
    	par->adpcm.adpcm_mode = FALSE;  /* macro VIDEO_ADPCM_ONOFF(VIDEO_ADPCM_ON:TRUE VIDEO_ADPCM_OFF:FALSE)*/
    	par->adpcm.adpcm_ratio = 0;          
    	par->sml_frm.sml_frm_mode = FALSE;  /* macro VDEC27_SML_FRM_ONOFF(VDEC27_SML_FRM_OFF: FALSE   VDEC27_SML_FRM_ON: TRUE)*/
    	par->return_multi_freebuf = TRUE;        /* macro VDEC27_SUPPORT_RETURN_MULTI_FREEBUF */ 
    	par->sml_frm.sml_frm_size = 0;           /* macro VDEC27_SML_FRM_SIZE*/
    	par->lm_shiftthreshold = 2;                   /* macro VDEC27_LM_SHIFTTHRESHOLD*/
    	par->vp_init_phase = 0;                        /* macro DEFAULT_MP_FILTER_ENABLE*/
    	par->preview_solution = VDEC27_PREVIEW_DE_SCALE; /* macro VDEC27_PREVIEW_SOLUTION(VDEC27_PREVIEW_VE_SCALE or VDEC27_PREVIEW_DE_SCALE)*/
    }
}
#ifdef CONFIG_RPC_HLD_DECV_AVS
static void avs_config(struct vdec_avs_config_par *par)
{	
	unsigned int avs_buf = 0;

#define MAX_EXTRA_FB_NUM 3
#define AVS_MAX_MB_WIDTH 120 //(1920/16)
#define AVS_MAX_MB_HEIGHT 68 //(1088/16)
#define AVS_MAX_MB_STRIDE 120 //120 MB alignment to improve AVC performance
#define AVS_ONE_FB_SIZE (AVS_MAX_MB_STRIDE*AVS_MAX_MB_HEIGHT*256*3/2)
#define AVS_ONE_DV_FB_SIZE ((AVS_MAX_MB_WIDTH*AVS_MAX_MB_HEIGHT*256*3/2)/4)
#define AVS_ONE_MV_SIZE 64*(AVS_MAX_MB_WIDTH*AVS_MAX_MB_HEIGHT) //522240
#define AVS_FB_LEN		AVS_ONE_FB_SIZE*(4+MAX_EXTRA_FB_NUM) //0x1700000
#define AVS_DVIEW_LEN   AVS_ONE_DV_FB_SIZE*(4+MAX_EXTRA_FB_NUM) //0xb00000
#define AVS_MV_LEN		AVS_ONE_MV_SIZE*(4+MAX_EXTRA_FB_NUM) //0x37c800//0x2FD000
#define AVS_MB_COL_LEN		0x11000 //0x22000
#define AVS_MB_NEI_LEN		0x8000
#define AVS_CMD_QUEUE_LEN 0x10000 //In allegro test stream, this length could be bigger than 128k, however, in realistic, 0x10000 should be enough
#define AVS_LAF_RW_BUF_LEN (((((AVS_MAX_MB_WIDTH*AVS_MAX_MB_HEIGHT)*48*2)+1023)&0x0ffffc00)*2)
#define AVS_LAF_FLAG_BUF_LEN (0xc00*22) //when enable dual output, we need 1 more laf buffer
#define AVS_VBV_LEN		0x300000 //if this size is set to 0x200000, vbv buffer overflow could happen in 20061007d_0.ts

#define AVS_VBV_ADDR	(__G_MM_VDEC_VBV_START_ADDR & 0xffffff00)	//256 bytes alignment
#define AVS_CMD_QUEUE_ADDR	((AVS_VBV_ADDR - AVS_CMD_QUEUE_LEN)&0xffffff00)  	//256 bytes alignment
#define AVS_LAF_FLAG_BUF_ADDR		((AVS_CMD_QUEUE_ADDR - AVS_LAF_FLAG_BUF_LEN)&0xfffffc00)  //1024 bytes alignment

	par->memmap.vbv_buffer_base = (AVS_VBV_ADDR & 0x1fffffff) | 0x80000000;
    	par->memmap.vbv_buffer_len = AVS_VBV_LEN;

	par->memmap.cmd_queue_buffer_base = AVS_CMD_QUEUE_ADDR;
    	par->memmap.cmd_queue_buffer_len = AVS_CMD_QUEUE_LEN;

	par->memmap.laf_flag_buf = AVS_LAF_FLAG_BUF_ADDR;
    	par->memmap.laf_flag_buffer_len = AVS_LAF_FLAG_BUF_LEN;

	avs_buf = (__G_MM_VIDEO_TOP_ADDR - AVS_FB_LEN)&0xfffffe00;
	par->memmap.support_multi_bank = FALSE;
    	par->memmap.frame_buffer_base = avs_buf;
    	par->memmap.frame_buffer_len = AVS_FB_LEN;

	avs_buf = ((avs_buf -  AVS_DVIEW_LEN)&0xfffffe00);
    	par->memmap.dv_frame_buffer_base = avs_buf;
    	par->memmap.dv_frame_buffer_len = AVS_DVIEW_LEN;

	avs_buf = ((avs_buf -  AVS_MV_LEN)&0xffffff00);
    	par->memmap.mv_buffer_base = avs_buf;
    	par->memmap.mv_buffer_len = AVS_MV_LEN;

	avs_buf = ((avs_buf -  AVS_MB_COL_LEN)&0xffffff00);
    	par->memmap.mb_col_buffer_base = avs_buf;
    	par->memmap.mb_col_buffer_len = AVS_MB_COL_LEN;

	avs_buf = ((avs_buf -  AVS_MB_NEI_LEN)&0xffffff00);
    	par->memmap.mb_neighbour_buffer_base = avs_buf;
    	par->memmap.mb_neighbour_buffer_len = AVS_MB_NEI_LEN;

	avs_buf = ((avs_buf -  AVS_LAF_RW_BUF_LEN)&0xfffffc00);
	par->memmap.laf_rw_buf = avs_buf;
    	par->memmap.laf_rw_buffer_len = AVS_LAF_RW_BUF_LEN;

}
#endif
static void vdec_dev_attach(void)
{
	/* !!!!!Note !!!!!
	If you copy this code to other project, please check feature configuration firstly
	!!!!!Note !!!!!
	*/
	struct vdec_config_par vdec_mpg_cfg;
	struct vdec_avc_config_par vdec_avc_cfg;
	//struct vdec_avs_config_par vdec_avs_cfg;

	if((__G_MM_VIDEO_TOP_ADDR - __G_MM_VIDEO_START_ADDR) < 0x02000000)
		m_video_dview_disable = 1;
	else
		m_video_dview_disable = 0;
		
	mpg2_config(&vdec_mpg_cfg);
	vdec_m36_attach(&vdec_mpg_cfg);
	PRF("attach mpeg decv done\n\n");

	g_decv_dev0 = (struct vdec_device *)hld_dev_get_by_name("DECV_S3601_0");    
	vdec_enable_advance_play(g_decv_dev0); 
	PRF("enable decv advance play done\n\n");

	avc_config(&vdec_avc_cfg);
	vdec_avc_attach(&vdec_avc_cfg);
	PRF("attach avc decv done\n\n");

#ifdef CONFIG_RPC_HLD_DECV_AVS
	avs_config(&vdec_avs_cfg);
	vdec_avs_attach(&vdec_avs_cfg);
	PRF("attach avs decv done\n\n");
#endif

	if(m_video_dview_disable == 1)
	{
		vdec_disable_dview(g_decv_dev0);
	}

	if((__G_MM_STILL_FRAME_ADDR == 0) 
		|| (__G_MM_STILL_FRAME_SIZE == 0))
	{
		vdec_io_control(g_decv_dev0, VDEC_IO_SET_STILL_FRAME_MODE, TRUE);
	}	
}
#endif

#ifdef CONFIG_RPC_HLD_DIS
int m_boot_vpo_active = 0;
enum TVSystem m_boot_vpo_system;
int m_boot_vpo_progressive;
enum TVMode m_boot_vpo_eTVAspect;
UINT32 m_boot_media_addr = 0;
static UINT32 m_boot_media_len = 0;
int m_boot_media_flag = 0;

static void init_boot_vpo_pars(struct VP_InitInfo *pars)
{
	int i;

	if((UINT32)m_boot_vpo_system > LINE_1080P_60_VESA)
	{
		m_boot_vpo_system = LINE_1080_25;
		m_boot_vpo_progressive = FALSE;
	}
	
	memset((void *)pars, 0, sizeof(*pars));
	
	//api set backgound color]
	pars->tInitColor.uY = 0x10;
	pars->tInitColor.uCb= 0x80;
	pars->tInitColor.uCr= 0x80;

	//set advanced control
	pars->bBrightnessValue = 0;
	pars->fBrightnessValueSign = TRUE;
	pars->wContrastValue = 0;
	pars->fContrastSign = TRUE;
	pars->wSaturationValue = 0;
	pars->fSaturationValueSign = TRUE;
	pars->wSharpness = 0;
	pars->fSharpnessSign = TRUE;
	pars->wHueSin = 0;
	pars->fHueSinSign = TRUE;
	pars->wHueCos = 0;
	pars->fHueCosSign = TRUE;
	pars->bCCIR656Enable = FALSE;
	
	//VPO_Zoom
	pars->tSrcRect.uStartX = 0;
	pars->tSrcRect.uWidth= PICTURE_WIDTH;
	pars->tSrcRect.uStartY= 0;
	pars->tSrcRect.uHeight= PICTURE_HEIGHT;
	pars->DstRect.uStartX = 0;
	pars->DstRect.uWidth= SCREEN_WIDTH;
	pars->DstRect.uStartY= 0;
	pars->DstRect.uHeight= SCREEN_HEIGHT;

	//VPO_SetAspect
	pars->eTVAspect = m_boot_vpo_eTVAspect;
	pars->eDisplayMode = NORMAL_SCALE;//LETTERBOX;
	pars->uNonlinearChangePoint = 111;
	pars->uPanScanOffset = 90;
	
	//VPO_SetOutput
	for(i=0;i<VP_DAC_TYPENUM;i++)
		pars->pDacConfig[i].bEnable = FALSE;

	if((m_boot_vpo_system >= LINE_576P_50_VESA) && (m_boot_vpo_system <= LINE_1080P_60_VESA))
	{
		pars->pDacConfig[RGB_1].bEnable = TRUE;
		pars->pDacConfig[RGB_1].tDacIndex.uDacFirst = DAC1;
		pars->pDacConfig[RGB_1].tDacIndex.uDacSecond = DAC2;
		pars->pDacConfig[RGB_1].tDacIndex.uDacThird= DAC0;
		pars->pDacConfig[RGB_1].eVGAMode= VGA_NOT_USE;
		pars->pDacConfig[RGB_1].bProgressive= TRUE;
	}
	else
	{
	pars->pDacConfig[YUV_1].bEnable = TRUE;
	pars->pDacConfig[YUV_1].tDacIndex.uDacFirst = g_tve_dac_yuv_y;
	pars->pDacConfig[YUV_1].tDacIndex.uDacSecond = g_tve_dac_yuv_u;
	pars->pDacConfig[YUV_1].tDacIndex.uDacThird= g_tve_dac_yuv_v;
	pars->pDacConfig[YUV_1].eVGAMode= VGA_NOT_USE;
	pars->pDacConfig[YUV_1].bProgressive= m_boot_vpo_progressive ? TRUE : FALSE;
	}

	pars->eTVSys = m_boot_vpo_system;
	pars->bWinOnOff = FALSE;
	pars->uWinMode = VPO_MAINWIN;//|VPO_PIPWIN;
	pars->tPIPCallBack.RequestCallback = NULL;
	pars->tPIPCallBack.ReleaseCallback= NULL;
	pars->pSrcChange_CallBack=NULL;
}

static void init_boot_vpo_pars_sd(struct VP_InitInfo *pars)
{
	int i;

	memset((void *)pars, 0, sizeof(*pars));

	pars->device_priority = VPO_AUTO_DUAL_OUTPUT;    // S3602F use VCAP

	//api set backgound color]
	pars->tInitColor.uY = 0x10;
	pars->tInitColor.uCb= 0x80;
	pars->tInitColor.uCr= 0x80;

	//set advanced control
	pars->bBrightnessValue = 0;
	pars->fBrightnessValueSign = TRUE;
	pars->wContrastValue = 0;
	pars->fContrastSign = TRUE;
	pars->wSaturationValue = 0;
	pars->fSaturationValueSign = TRUE;
	pars->wSharpness = 0;
	pars->fSharpnessSign = TRUE;
	pars->wHueSin = 0;
	pars->fHueSinSign = TRUE;
	pars->wHueCos = 0;
	pars->fHueCosSign = TRUE;
	
	//VPO_Zoom
	pars->tSrcRect.uStartX = 0;
	pars->tSrcRect.uWidth= PICTURE_WIDTH;
	pars->tSrcRect.uStartY= 0;
	pars->tSrcRect.uHeight= PICTURE_HEIGHT;
	pars->DstRect.uStartX = 0;
	pars->DstRect.uWidth= SCREEN_WIDTH;
	pars->DstRect.uStartY= 0;
	pars->DstRect.uHeight= SCREEN_HEIGHT;

	//VPO_SetAspect
	pars->eTVAspect = TV_4_3;
	pars->eDisplayMode = NORMAL_SCALE;//LETTERBOX;
	pars->uNonlinearChangePoint = 111;
	pars->uPanScanOffset = 90;
	
	//VPO_SetOutput
	for(i=0;i<VP_DAC_TYPENUM;i++)
		pars->pDacConfig[i].bEnable = FALSE;

	pars->eTVSys = PAL;
	pars->bWinOnOff = FALSE;

	pars->uWinMode = VPO_MAINWIN;//|VPO_PIPWIN;
	pars->tPIPCallBack.RequestCallback = NULL;
	pars->tPIPCallBack.ReleaseCallback= NULL;
	pars->pSrcChange_CallBack=NULL;
}

static void init_vpo_pars(struct VP_InitInfo *pars)
{
	int i;
	int vdac_progressive = FALSE;

	if(m_boot_vpo_active)
		return init_boot_vpo_pars(pars);
	
	memset((void *)pars, 0, sizeof(*pars));
	
	//api set backgound color]
	pars->tInitColor.uY = 0x10;
	pars->tInitColor.uCb= 0x80;
	pars->tInitColor.uCr= 0x80;

	//set advanced control
	pars->bBrightnessValue = 0;
	pars->fBrightnessValueSign = TRUE;
	pars->wContrastValue = 0;
	pars->fContrastSign = TRUE;
	pars->wSaturationValue = 0;
	pars->fSaturationValueSign = TRUE;
	pars->wSharpness = 0;
	pars->fSharpnessSign = TRUE;
	pars->wHueSin = 0;
	pars->fHueSinSign = TRUE;
	pars->wHueCos = 0;
	pars->fHueCosSign = TRUE;
	pars->bCCIR656Enable = FALSE;
	
	//VPO_Zoom
	pars->tSrcRect.uStartX = 0;
	pars->tSrcRect.uWidth= PICTURE_WIDTH;
	pars->tSrcRect.uStartY= 0;
	pars->tSrcRect.uHeight= PICTURE_HEIGHT;
	pars->DstRect.uStartX = 0;
	pars->DstRect.uWidth= SCREEN_WIDTH;
	pars->DstRect.uStartY= 0;
	pars->DstRect.uHeight= SCREEN_HEIGHT;

	//VPO_SetAspect
	pars->eTVAspect = TV_4_3;
	pars->eDisplayMode = NORMAL_SCALE;//LETTERBOX;
	pars->uNonlinearChangePoint = 111;
	pars->uPanScanOffset = 90;
	
	//VPO_SetOutput
	for(i=0;i<VP_DAC_TYPENUM;i++)
		pars->pDacConfig[i].bEnable = FALSE;

	//CVBS_1
#ifndef JTAG_DEBUG 	
	if(g_tve_dac_use_cvbs_type >= 0)
#endif
	{
#ifndef JTAG_DEBUG
		pars->pDacConfig[g_tve_dac_use_cvbs_type].bEnable = FALSE;
#else
		pars->pDacConfig[g_tve_dac_use_cvbs_type].bEnable = TRUE;
#endif
		pars->pDacConfig[g_tve_dac_use_cvbs_type].tDacIndex.uDacFirst = g_tve_dac_cvbs;
		pars->pDacConfig[g_tve_dac_use_cvbs_type].eVGAMode= VGA_NOT_USE;
		pars->pDacConfig[g_tve_dac_use_cvbs_type].bProgressive= FALSE;
	}

#ifndef JTAG_DEBUG
	if(g_tve_dac_use_svideo_type >= 0){
		pars->pDacConfig[g_tve_dac_use_svideo_type].bEnable = FALSE;
		pars->pDacConfig[g_tve_dac_use_svideo_type].tDacIndex.uDacFirst = g_tve_dac_svideo_y;
		pars->pDacConfig[g_tve_dac_use_svideo_type].tDacIndex.uDacSecond= g_tve_dac_svideo_c;
		pars->pDacConfig[g_tve_dac_use_svideo_type].eVGAMode= VGA_NOT_USE;
		pars->pDacConfig[g_tve_dac_use_svideo_type].bProgressive= FALSE;
	}

	//YUV
	if(g_tve_dac_use_yuv_type >= 0){
		pars->pDacConfig[g_tve_dac_use_yuv_type].bEnable = FALSE;
		pars->pDacConfig[g_tve_dac_use_yuv_type].tDacIndex.uDacFirst = 
			g_tve_dac_yuv_y;
		pars->pDacConfig[g_tve_dac_use_yuv_type].tDacIndex.uDacSecond=
			g_tve_dac_yuv_u;
		pars->pDacConfig[g_tve_dac_use_yuv_type].tDacIndex.uDacThird= 
			g_tve_dac_yuv_v;
		pars->pDacConfig[g_tve_dac_use_yuv_type].eVGAMode= VGA_NOT_USE;
		pars->pDacConfig[g_tve_dac_use_yuv_type].bProgressive= FALSE;
	}

	//RGB
	if(g_tve_dac_use_rgb_type >= 0){
		pars->pDacConfig[g_tve_dac_use_rgb_type].bEnable = FALSE;
		pars->pDacConfig[g_tve_dac_use_rgb_type].tDacIndex.uDacFirst = 
			g_tve_dac_rgb_r;
		pars->pDacConfig[g_tve_dac_use_rgb_type].tDacIndex.uDacSecond= 
			g_tve_dac_rgb_g;
		pars->pDacConfig[g_tve_dac_use_rgb_type].tDacIndex.uDacThird= 
			g_tve_dac_rgb_b;
		pars->pDacConfig[g_tve_dac_use_rgb_type].eVGAMode= VGA_NOT_USE;
		pars->pDacConfig[g_tve_dac_use_rgb_type].bProgressive= FALSE;
	}

	if (g_tve_default_scart_output == 3)
	{
		if(g_tve_hd_default_tv_mode < 0)
			g_tve_hd_default_tv_mode = LINE_1080_30;
		
		pars->eTVSys = g_tve_hd_default_tv_mode;

		if (g_tve_hd_default_tv_mode == LINE_720_30 
			|| g_tve_hd_default_tv_mode == LINE_720_25
			||g_tve_hd_default_tv_mode == LINE_1080_50 
			|| g_tve_hd_default_tv_mode == LINE_1080_60 
			|| g_tve_hd_default_tv_mode == LINE_1080_24){
			vdac_progressive = TRUE;
		}
		
		if(g_tve_dac_use_cvbs_type >= 0)
			pars->pDacConfig[g_tve_dac_use_cvbs_type].bEnable = FALSE;

		if(g_tve_dac_use_svideo_type >= 0)
			pars->pDacConfig[g_tve_dac_use_svideo_type].bEnable = FALSE;			
			
		if(g_tve_dac_use_yuv_type >= 0){
			pars->pDacConfig[g_tve_dac_use_yuv_type].bEnable = TRUE;
			pars->pDacConfig[g_tve_dac_use_yuv_type].bProgressive = vdac_progressive;
		}

		if(g_tve_dac_use_rgb_type >= 0)
			pars->pDacConfig[g_tve_dac_use_rgb_type].bEnable = FALSE;
	}
	else
	{
		pars->eTVSys = PAL;
		
		if(g_tve_dac_use_cvbs_type >= 0)
			pars->pDacConfig[g_tve_dac_use_cvbs_type].bEnable = TRUE;

		if(g_tve_dac_use_svideo_type >= 0)
			pars->pDacConfig[g_tve_dac_use_svideo_type].bEnable = FALSE;	
		
		if(g_tve_dac_use_yuv_type >= 0)
			pars->pDacConfig[g_tve_dac_use_yuv_type].bEnable = FALSE;
		
		if(g_tve_dac_use_rgb_type >= 0)
			pars->pDacConfig[g_tve_dac_use_rgb_type].bEnable = TRUE;
	}
#else
	pars->eTVSys = PAL;
#endif

	pars->bWinOnOff = FALSE;
	pars->uWinMode = VPO_MAINWIN;//|VPO_PIPWIN;
	pars->tPIPCallBack.RequestCallback = NULL;
	pars->tPIPCallBack.ReleaseCallback= NULL;
	pars->pSrcChange_CallBack=NULL;
}

static void init_vpo_pars_sd(struct VP_InitInfo *pars)
{
	int i;

	if(m_boot_vpo_active)
		return init_boot_vpo_pars_sd(pars);
	
	memset((void *)pars, 0, sizeof(*pars));

	pars->device_priority = VPO_AUTO_DUAL_OUTPUT;    // S3602F use VCAP

	//api set backgound color]
	pars->tInitColor.uY = 0x10;
	pars->tInitColor.uCb= 0x80;
	pars->tInitColor.uCr= 0x80;

	//set advanced control
	pars->bBrightnessValue = 0;
	pars->fBrightnessValueSign = TRUE;
	pars->wContrastValue = 0;
	pars->fContrastSign = TRUE;
	pars->wSaturationValue = 0;
	pars->fSaturationValueSign = TRUE;
	pars->wSharpness = 0;
	pars->fSharpnessSign = TRUE;
	pars->wHueSin = 0;
	pars->fHueSinSign = TRUE;
	pars->wHueCos = 0;
	pars->fHueCosSign = TRUE;
	
	//VPO_Zoom
	pars->tSrcRect.uStartX = 0;
	pars->tSrcRect.uWidth= PICTURE_WIDTH;
	pars->tSrcRect.uStartY= 0;
	pars->tSrcRect.uHeight= PICTURE_HEIGHT;
	pars->DstRect.uStartX = 0;
	pars->DstRect.uWidth= SCREEN_WIDTH;
	pars->DstRect.uStartY= 0;
	pars->DstRect.uHeight= SCREEN_HEIGHT;

	//VPO_SetAspect
	pars->eTVAspect = TV_4_3;
	pars->eDisplayMode = NORMAL_SCALE;//LETTERBOX;
	pars->uNonlinearChangePoint = 111;
	pars->uPanScanOffset = 90;
	
	//VPO_SetOutput
	for(i=0;i<VP_DAC_TYPENUM;i++)
		pars->pDacConfig[i].bEnable = FALSE;

#ifndef JTAG_DEBUG
	//CVBS_1
	if(g_tve_dac_use_cvbs_type >= 0){
		pars->pDacConfig[g_tve_dac_use_cvbs_type].bEnable = TRUE;
		pars->pDacConfig[g_tve_dac_use_cvbs_type].tDacIndex.uDacFirst = g_tve_dac_cvbs;
		pars->pDacConfig[g_tve_dac_use_cvbs_type].eVGAMode= VGA_NOT_USE;
		pars->pDacConfig[g_tve_dac_use_cvbs_type].bProgressive= FALSE;
	}

	if(g_tve_sd_default_tv_mode >= 0)
	{
		//if(g_tve_sd_default_tv_mode >= NTSC_443)
		//	pars->eTVSys = g_tve_sd_default_tv_mode;
		//else
			pars->eTVSys = g_tve_sd_default_tv_mode;
		pars->bWinOnOff = TRUE;
	}
	else
	{
		pars->eTVSys = NTSC; // PAL
		pars->bWinOnOff = FALSE;
	}
#else
	pars->eTVSys = PAL;
	pars->bWinOnOff = FALSE;
#endif
	pars->uWinMode = VPO_MAINWIN;//|VPO_PIPWIN;
	pars->tPIPCallBack.RequestCallback = NULL;
	pars->tPIPCallBack.ReleaseCallback= NULL;
	pars->pSrcChange_CallBack=NULL;
}
#endif


static void video_config(void)
{
#ifdef CONFIG_RPC_HLD_DIS	
#ifdef CONFIG_RPC_HLD_DECV	
	if(RET_SUCCESS!=vdec_open(g_decv_dev0)){
		PRF("%s : vdec open fail\n\n", __FUNCTION__);
		return;
	}
	PRF("open decv done\n\n");
	
#endif	

	{
		struct VP_InitInfo	tVPInitInfo;
		
		if(0 == m_boot_vpo_active)
		{
		if((*(unsigned long *)0xb8006000) & 1)
		{    			
			UINT8 *buf = (UINT8 *)(__G_MM_PRIVATE_AREA_START_ADDR - 0x1000 + 288);

			m_boot_vpo_active = 1;
			m_boot_vpo_system = *buf;
			m_boot_vpo_progressive = *(buf + 1);
			m_boot_vpo_eTVAspect = *(buf + 2);			
			if((m_boot_vpo_system > LINE_1080P_60_VESA)
				|| (m_boot_vpo_eTVAspect > TV_AUTO))
			{
				m_boot_vpo_system = LINE_1080_25;
				m_boot_vpo_progressive = FALSE;
				m_boot_vpo_eTVAspect = TV_AUTO;
				
				printk("boot vpo fail buf %x tv %d p %d aspect %d\n", (int)buf, (int)*buf, (int)*(buf + 1), (int)*(buf + 2));
			}
							
			PRF("buf %x tv %d p %d aspect %d\n", (UINT32)buf, *buf, *(buf + 1), *(buf + 2));

			m_boot_media_addr = *(UINT32 *)(buf + 44);// 44 = 20 + 8 + 16
			m_boot_media_len = *(UINT32 *)(buf + 48); // 48 = 20 + 8 + 16 + 4;
			m_boot_media_flag = 0;
			if((m_boot_media_addr & 0xA0000000) && (!(m_boot_media_addr & 0x3)) && (m_boot_media_len & 0x00F00000))
			{
#define EBML_ID_HEADER                0xA3DF451A

				// check mkv header
				if(*(volatile UINT32 *)m_boot_media_addr == EBML_ID_HEADER)
				{
					m_boot_media_flag = 1;

					//for 128M solution, the stream size of boot media Must be < 6M. added by Sam at 2013.05.15
					if((__G_MM_MP_MEM_TOP_ADDR - __G_MM_MP_MEM_START_ADDR) < 0x00400000)
					{
						if(m_boot_media_len > 0x00600000)
						{
							m_boot_media_len = 0x00600000;
							*(volatile UINT32 *)(buf + 48) = m_boot_media_len;
						}					
					}
					else	 if(m_boot_media_len > 0x00900000)
					{
						m_boot_media_len = 0x00900000;
						*(volatile UINT32 *)(buf + 48) = m_boot_media_len;
					}
							
					printk("%s : boot media addr 0x%08x len 0x%08x\n", __FUNCTION__, \
					(unsigned int)m_boot_media_addr, (unsigned int)m_boot_media_len);										
				}
				else
				{
					printk("%s : boot media fail for no MKV stream\n", __FUNCTION__);
				}
			
			}		
    }
		else		
		{
			m_boot_vpo_active = 0;
			m_boot_media_addr = 0;
			m_boot_media_len = 0;
			m_boot_media_flag = 0;
		}
		}

		if ( ((*(unsigned long *)0xb8006190) & 0x00200000) )
		{
			if ( __G_MM_BOOTLOGO_DATA_START_ADDR )
			{
				UINT8 *buf = (UINT8 *)__G_MM_BOOTLOGO_DATA_START_ADDR;
				vdec_io_control(g_decv_dev0, VDEC_IO_SET_BOOTLOGO_ADDR, (UINT32)buf);
				PRF("set the logo address %x\n", (UINT32)buf);
			}
		}
		
		init_vpo_pars(&tVPInitInfo);
		if(RET_SUCCESS!=vpo_open(g_vpo_dev,&tVPInitInfo)){
			PRF("%s : vpo open fail\n\n", __FUNCTION__);
			return;
		}
		PRF("open hd vpo done\n\n");

		init_vpo_pars_sd(&tVPInitInfo);
		if(RET_SUCCESS!=vpo_open(g_sd_vpo_dev,&tVPInitInfo)){
			PRF("%s : vpo open fail\n\n", __FUNCTION__);
			return;
		}		
		PRF("open sd vpo done\n\n");
	}

	vpo_register_cb_routine();
	
	vdec_profile_level(g_decv_dev0, MP_HL, NULL);
	PRF("set decv profile done\n\n");
	
	{
		struct VDecPIPInfo vInitInfo;
		struct MPSource_CallBack vMPCallBack;
		struct PIPSource_CallBack vPIPCallBack;	
		struct vpo_io_get_info dis_info;

		memset((void *)&vInitInfo, 0, sizeof(vInitInfo));
		memset((void *)&vMPCallBack, 0, sizeof(vMPCallBack));
		memset((void *)&vPIPCallBack, 0, sizeof(vPIPCallBack));
		
		vpo_ioctl(g_vpo_dev, VPO_IO_GET_INFO, (UINT32) &dis_info);
		PRF("vpo get info done\n\n");

		vInitInfo.adv_setting.init_mode = 0;
		vInitInfo.adv_setting.out_sys = dis_info.tvsys;
		vInitInfo.adv_setting.bprogressive = dis_info.bprogressive;
		vdec_set_output(g_decv_dev0, MP_MODE, &vInitInfo, &vMPCallBack, &vPIPCallBack);
		PRF("decv set output done\n\n");
		
		vpo_win_mode(g_vpo_dev, VPO_MAINWIN, &vMPCallBack, &vPIPCallBack);
		PRF("vpo set win mode done\n\n");
		
		vdec_sync_mode(g_decv_dev0, 0x00,VDEC_SYNC_I|VDEC_SYNC_P|VDEC_SYNC_B);
		PRF("decv set sync mode done\n\n");
	}
#endif	
}


#ifdef CONFIG_RPC_HLD_DECA
static void deca_dev_attach(void)
{
	struct deca_feature_config deca_cfg;
	
	memset(&deca_cfg,0,sizeof(struct deca_feature_config));
        deca_cfg.detect_sprt_change = 1;			
       // deca_cfg.support_desc = 1;
	deca_m36_attach(&deca_cfg);
	   
	 if(m_video_dview_disable)//for NMP memory cost down
		return;
	 
	deca_m36_ext_dec_enable((struct deca_device * )hld_dev_get_by_id(HLD_DEV_TYPE_DECA, 0), &deca_cfg);
	deca_m36_dvr_enable((struct deca_device*)hld_dev_get_by_id(HLD_DEV_TYPE_DECA, 0));
	deca_m36_init_tone_voice((struct deca_device*)hld_dev_get_by_id(HLD_DEV_TYPE_DECA, 0)); 
	


	return;
}
#endif

#ifdef CONFIG_RPC_HLD_SND
static void snd_dev_attach(void)
{
	struct snd_feature_config snd_cfg;
	extern UINT32 g_snd_mute_gpio;
	
	memset(&snd_cfg,0,sizeof(struct snd_feature_config));
	snd_cfg.output_config.mute_num = g_snd_mute_gpio;
	snd_cfg.output_config.mute_polar 	= 0;
	snd_cfg.output_config.dac_precision = 24;
	snd_cfg.output_config.dac_format 	= CODEC_I2S;
	snd_cfg.output_config.is_ext_dac 	= 0;
	snd_cfg.output_config.ext_mute_mode = MUTE_BY_GPIO;	
	snd_cfg.support_spdif_mute = 1;
	snd_cfg.output_config.chip_type_config = 1; //QFP
	snd_cfg.conti_clk_while_ch_chg = 1;
       // snd_cfg.support_desc = 0x3;
	snd_m36_attach(&snd_cfg);
	snd_register_cb_routine();
	
	 if(m_video_dview_disable)//for NMP memory cost down
	 	return;
		 
	snd_m36_init_tone_voice((struct snd_device *)hld_dev_get_by_id(HLD_DEV_TYPE_SND, 0));
	//void test_start_ali_audio_test_task(void);
	//test_start_ali_audio_test_task();
	return;
}
#endif

#ifdef CONFIG_RPC_HLD_GMA
static void gma_dev_attach(void)
{
	gma_attach_m36f(2);
	PRF("attach gma done\n");
}
#endif

#ifdef CONFIG_RPC_HLD_AVSYNC
static void avsync_dev_attach(void)
{
	avsync_attach();		
	PRF("attach avsync module done\n");		
}
#endif

static void SetTTxParamater(struct ttx_config_par *pconfig_par)
{    
	//ttx_mem_attr_t *p_ttx_mem_attr = (ttx_mem_attr_t *)request_attr(TTX_MEM);
//#ifndef DUAL_ENABLE    
//    pconfig_par->osd_get_scale_para = ap_get_osd_scale_param;
//#else
    pconfig_par->osd_get_scale_para = NULL;
//#endif
	pconfig_par->osd_layer_id = 1;
    #ifdef	SUPPORT_ERASE_UNKOWN_PACKET
	pconfig_par->erase_unknown_packet = TRUE;
    #else
	pconfig_par->erase_unknown_packet = FALSE;
    #endif

    //#ifdef	TTX_SUB_PAGE
	pconfig_par->ttx_sub_page = TRUE;
    pconfig_par->ttx_subpage_addr = __G_TTX_SUB_PAGE_BUF_START_ADDR;//ttx_sub_page_buf_start_addr;
 //   #else
	//pconfig_par->ttx_sub_page = FALSE;
 //   pconfig_par->ttx_subpage_addr = 0;
 //   #endif
    
    #ifdef	USER_FAST_TEXT
	pconfig_par->user_fast_text = TRUE;
    #else
	pconfig_par->user_fast_text = FALSE;
    #endif

    #ifdef NO_TTX_DESCRIPTOR
    pconfig_par->no_ttx_descriptor = TRUE;
    #else
    pconfig_par->no_ttx_descriptor = FALSE;
    #endif

    //#if(SYS_SDRAM_SIZE == 2)
    //pconfig_par->sys_sdram_size_2m = TRUE;
    //#else
    pconfig_par->sys_sdram_size_2m = FALSE;
    //#endif

    //#ifdef HDTV_SUPPORT
    pconfig_par->hdtv_support_enable = TRUE;
    //#else
    //pconfig_par->hdtv_support_enable = FALSE;
    //#endif

}

static void ttx_attach(void)
{
	struct ttx_config_par ttx_cfg;
	SetTTxParamater(&ttx_cfg);
	TTXEng_Attach(&ttx_cfg);
}
static void SetSubtitleParamater(struct subt_config_par *pconfig_par)
{
	//subtitle_mem_attr_t *p_subtitle_mem_attr = (subtitle_mem_attr_t *)request_attr(SUBTITLE_MEM);
 //   pconfig_par->sys_sdram_size = SYS_SDRAM_SIZE;
    pconfig_par->max_subt_height = 576;
	pconfig_par->osd_layer_id = 1;  
    
    #ifdef CC_BY_VBI
    pconfig_par->cc_by_vbi = TRUE;
    #else
    pconfig_par->cc_by_vbi = FALSE;
    #endif

    #ifdef CC_BY_OSD
    pconfig_par->cc_by_osd = TRUE;
    #else
    pconfig_par->cc_by_osd = FALSE;
    #endif

 //   #ifdef OSD_BLOCKLINK_SUPPORT
 //   pconfig_par->osd_blocklink_enable = TRUE;
 //   pconfig_par->g_ps_buf_addr = 0;
	//pconfig_par->g_ps_buf_len = 0;
 //   #else
    pconfig_par->osd_blocklink_enable = FALSE;
    pconfig_par->g_ps_buf_addr = (UINT8*)__G_SUB_PB_START_ADDR;//sub_pb_start_addr;
	pconfig_par->g_ps_buf_len = __MM_SUB_PB_LEN;
    //#endif

    #ifdef SPEED_UP_SUBT
    pconfig_par->speed_up_subt_enable = TRUE;
    #else
    pconfig_par->speed_up_subt_enable = FALSE;
    #endif

    //#ifdef HD_SUBTITLE_SUPPORT
    pconfig_par->hd_subtitle_support = TRUE;
    pconfig_par->osd_get_scale_para = NULL;//ap_get_osd_scale_param; 
    //#else
   // pconfig_par->hd_subtitle_support = FALSE;
   // pconfig_par->osd_get_scale_para = NULL;
    //#endif
}

static void subt_attach(void)
{
	struct subt_config_par subt_cfg;
	SetSubtitleParamater(&subt_cfg);
	lib_subt_attach(&subt_cfg);
}

static void ttx_subt_attach(void)
{
#ifdef CONFIG_RPC_HLD_VBI
//#if (TTX_ON == 1)
	memset(&g_vbi_config,0,sizeof(struct vbi_config_par));

	g_vbi_config.ttx_by_vbi = TRUE;
	g_vbi_config.cc_by_vbi = FALSE;
	g_vbi_config.vps_by_vbi = FALSE;
	g_vbi_config.wss_by_vbi = FALSE;
	
	g_vbi_config.hamming_24_16_enable = FALSE;
	g_vbi_config.hamming_8_4_enable = FALSE;
	g_vbi_config.erase_unknown_packet = TRUE;

//#ifdef SUPPORT_PACKET_26
	g_vbi_config.parse_packet26_enable = TRUE;
	g_vbi_config.mem_map.p26_data_buf_start_addr = __G_MM_TTX_P26_DATA_BUF_ADDR;
	g_vbi_config.mem_map.p26_data_buf_size = __MM_TTX_P26_DATA_LEN;
	g_vbi_config.mem_map.p26_nation_buf_start_addr = __G_MM_TTX_P26_NATION_BUF_ADDR;
	g_vbi_config.mem_map.p26_nation_buf_size = __MM_TTX_P26_NATION_LEN;
//#endif

//#ifdef TTX_SUB_PAGE
	g_vbi_config.ttx_sub_page = TRUE;
	g_vbi_config.mem_map.sub_page_start_addr = __G_MM_TTX_SUB_PAGE_BUF_ADDR;
	g_vbi_config.mem_map.sub_page_size = __MM_TTX_SUB_PAGE_LEN;
//#endif

	g_vbi_config.user_fast_text = FALSE;

	g_vbi_config.mem_map.sbf_start_addr = __G_MM_TTX_BS_START_ADDR;
	g_vbi_config.mem_map.sbf_size = __MM_TTX_BS_LEN;
	g_vbi_config.mem_map.pbf_start_addr = __G_MM_TTX_PB_START_ADDR;

	vbi_m33_attach(&g_vbi_config);

	g_vbi_config_dev = (struct vbi_device *)hld_dev_get_by_type(NULL, HLD_DEV_TYPE_VBI);

	PRF("ttx_subt_attach(): g_vbi_config_dev(0x%08x)\n", (u32)g_vbi_config_dev);
	
	vbi_enable_ttx_by_osd(g_vbi_config_dev);

#endif //CONFIG_RPC_HLD_VBI (TTX_ON == 1)


#ifdef CONFIG_RPC_HLD_SDEC
//#if (SUBTITLE_ON == 1)
//#ifdef SUBT_FUNC_POINTER_ENABLE
//	lib_subt_init();
//#endif
	if(support_isdbt_cc == 0)
	{
	memset(&g_sdec_config, 0, sizeof(struct sdec_feature_config));
	g_sdec_config.temp_buf_len = 0xa000;//(8*1024);  // > segment len 	
	g_sdec_config.temp_buf = NULL;
	g_sdec_config.bs_hdr_buf_len = 200;
	g_sdec_config.bs_hdr_buf = 0;
	g_sdec_config.bs_buf = (UINT8*)__G_MM_SUB_BS_START_ADDR;
	g_sdec_config.bs_buf_len = __MM_SUB_BS_LEN;
	g_sdec_config.pixel_buf = (UINT8*)__G_MM_SUB_PB_START_ADDR;

//	#ifdef OSD_BLOCKLINK_SUPPORT
//	cfg_param.pixel_buf_len = 0;
//	#else
//	cfg_param.pixel_buf_len = __MM_SUB_PB_LEN;//(100*1024)
//	#endif

	g_sdec_config.tsk_qtm = 2;//10;	//fix BUG05435
	g_sdec_config.tsk_pri = OSAL_PRI_HIGH;//OSAL_PRI_NORMAL;//OSAL_PRI_HIGH
	g_sdec_config.transparent_color = OSD_TRANSPARENT_COLOR;

//	#ifdef OSD_BLOCKLINK_SUPPORT
//#ifdef DUAL_ENABLE

	g_sdec_config.sdec_hw_buf = (UINT8*)__G_MM_SUB_HW_DATA_ADDR;
	g_sdec_config.sdec_hw_buf_len = __MM_SUB_HW_DATA_LEN;
//#else	
//	cfg_param.subt_draw_pixel = osd_subt_draw_pixel;
//	cfg_param.region_is_created = osd_region_is_created;
//	cfg_param.subt_create_region = osd_subt_create_region;
//	cfg_param.subt_region_show = osd_subt_region_show;
//	cfg_param.subt_delete_region = osd_subt_delete_region;
//#endif    
//	#else
//	cfg_param.subt_draw_pixelmap = osd_subt_draw_pixelmap;
//	#endif
    
	sdec_m33_attach(&g_sdec_config);
	
//	#ifdef OSD_BLOCKLINK_SUPPORT
	//g_sdec_device = (struct sdec_device *)hld_dev_get_by_type(NULL, HLD_DEV_TYPE_SDEC);

	g_sdec_device = (struct sdec_device *)hld_dev_get_by_id(HLD_DEV_TYPE_SDEC, 0);	


	PRF("ttx_subt_attach(): g_sdec_device(0x%08x)\n", (u32)g_sdec_device);


	subt_disply_bl_init(g_sdec_device);
//	#else
//	subt_disply_init((struct sdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SDEC));
//	#endif
	}
	else
	{
		/* isdbt cc attach */
		memset(&g_sdec_config, 0, sizeof(struct sdec_feature_config));
		g_sdec_config.bs_hdr_buf = 0;
		g_sdec_config.bs_hdr_buf_len = 200;

		g_sdec_config.bs_buf = (UINT8 *)__G_MM_SUB_BS_START_ADDR;
		g_sdec_config.bs_buf_len = 0x8EC0;

		g_sdec_config.tsk_qtm = 10;	//fix BUG05435
		g_sdec_config.tsk_pri = OSAL_PRI_HIGH;//OSAL_PRI_NORMAL;//OSAL_PRI_HIGH
		g_sdec_config.transparent_color = OSD_TRANSPARENT_COLOR;
		lib_isdbtcc_init();
		isdbtcc_dec_attach(&g_sdec_config);

		g_sdec_device = (struct sdec_device *)hld_dev_get_by_id(HLD_DEV_TYPE_SDEC, 0);	
		isdbtcc_disply_init(g_sdec_device);

		
	}
#endif //CONFIG_RPC_HLD_SDEC (SUBTITLE_ON == 1)
}


static void dev_attach(void)
{
#ifdef CONFIG_RPC_HLD_DIS

	vpo_dev_attach();

	g_vpo_dev = (struct vpo_device *)hld_dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
	g_sd_vpo_dev = (struct vpo_device *)hld_dev_get_by_id(HLD_DEV_TYPE_DIS, 1);	
	
	PRF("attach vpo devices done hd %x sd %x\n\n", (int)g_vpo_dev, (int)g_sd_vpo_dev);
#ifdef CONFIG_RPC_HLD_SDEC	
	osd_dev_attach();
#endif

#ifdef CONFIG_RPC_HLD_DECV
	vdec_dev_attach();

	// g_decv_dev0 = (struct vdec_device *)hld_dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
	g_decv_avc_dev0 = (struct vdec_device *)hld_dev_get_by_name("DECV_AVC_0");
	PRF("attach decv devices done mpg %x avc %x\n\n", (int)g_decv_dev0, (int)g_decv_avc_dev0);

#ifdef CONFIG_RPC_HLD_DECV_AVS
	g_decv_avs_dev0 = (struct vdec_device *)hld_dev_get_by_name("DECV_AVS_0");
	PRF("attach decv devices done avs %x\n\n", (int)g_decv_avs_dev0);
#endif
	
#endif

#endif

#ifdef CONFIG_RPC_HLD_DECA
	deca_dev_attach();

#ifdef CONFIG_RPC_HLD_SND
	snd_dev_attach();
#endif

#endif

#ifdef CONFIG_RPC_HLD_GMA
	gma_dev_attach();

	g_gma_dev0 = (struct gma_device *)hld_dev_get_by_id(HLD_DEV_TYPE_GMA, 0);
	if(g_gma_dev0 == NULL){
		PRF("gma dev attach fail\n");
	}
	else{
		PRF("gma dev %x\n", (int)g_gma_dev0);
	}
	g_gma_dev1 = (struct gma_device *)hld_dev_get_by_id(HLD_DEV_TYPE_GMA, 1);
	if(g_gma_dev1 == NULL){
		PRF("gma dev attach fail\n");
	}
	else{
		PRF("gma dev %x\n", (int)g_gma_dev1);
	}	
#endif

	ttx_subt_attach();
	ttx_attach();
	subt_attach();

#ifdef CONFIG_RPC_HLD_AVSYNC
	avsync_dev_attach();
#endif

#ifdef CONFIG_DVB_ALI_M36_DMX
	sed_dmx_attach(0);
#endif

#ifdef CONFIG_ALI_DSC
    //ali_m36_csa_see_init();
	ali_m36_dsc_see_init();
#endif
}

#if 0
void hdmi_config(void)
{
	struct vpo_device* dis = (struct vpo_device*) hld_dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
	struct snd_device* snd = (struct snd_device*) hld_dev_get_by_id(HLD_DEV_TYPE_SND, 0);    

	/* register Callback to DIS/SND TDS Driver */  
	if(dis != NULL){
		if(vpo_ioctl(dis, VPO_IO_REG_CB_HDMI, (UINT32)set_video_info_to_hdmi) != RET_SUCCESS)
			PRF("register video callback of HDMI failed!\n");  
		// msleep(100);
		// vpo_ioctl(dis, VPO_IO_HDMI_OUT_PIC_FMT, RGB_MODE2);
		msleep(100);	
		vpo_ioctl(dis, VPO_IO_HDMI_OUT_PIC_FMT, RGB_MODE1);		
		vpo_ioctl(dis, VPO_IO_ENABLE_DE_AVMUTE_HDMI, TRUE);		
	}
	
	if(snd != NULL){
		if(snd_io_control(snd, SND_REG_HDMI_CB, (UINT32)set_audio_info_to_hdmi) != RET_SUCCESS)
			PRF("register sound callback of HDMI failed!\n");   
	}
}
#else

void hdmi_config(void)
{
	struct vpo_device* dis = (struct vpo_device*) hld_dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
	struct snd_device* snd = (struct snd_device*) hld_dev_get_by_id(HLD_DEV_TYPE_SND, 0);    

		typedef struct
		{
			unsigned char hdmi_sig[4];
			unsigned char setting_valid;
			unsigned long hdcp;
			unsigned char reserved;
		} HDMI_BOOT_PARAM, *PHDMI_BOOT_PARAM;
	
		HDMI_BOOT_PARAM hdmi_param;
	
	/* register Callback to DIS/SND TDS Driver */  
	if(dis != NULL){
		if(vpo_ioctl(dis, VPO_IO_REG_CB_HDMI, (UINT32)set_video_info_to_hdmi) != RET_SUCCESS)
			PRF("register video callback of HDMI failed!\n");
	}
	
	if(snd != NULL){
		if(snd_io_control(snd, SND_REG_HDMI_CB, (UINT32)set_audio_info_to_hdmi) != RET_SUCCESS)
			PRF("register sound callback of HDMI failed!\n");   
	}

	hdmi_param.setting_valid = 0;
	if(m_boot_vpo_active == 1)
	{
		hdmi_param.setting_valid = 1;
		hdmi_param.hdcp = (unsigned long)(__G_MM_PRIVATE_AREA_START_ADDR - 0x1000);
		ali_hdmi_set_bootloader_param(&hdmi_param);
	}
}

#endif

static void vbi_config(void)
{
#ifdef CONFIG_RPC_HLD_VBI
	T_VBIRequest VBIRequest;

	memset((void *)&VBIRequest,0,sizeof(VBIRequest));
	if(0!=vbi_open(g_vbi_config_dev))
	{
		PRF("vbi_open failed!!\n");
		return;
	}
	
	vbi_setoutput(g_vbi_config_dev, &VBIRequest);
	vpo_ioctl(g_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)VBIRequest);

//#ifdef HDTV_SUPPORT   
	enable_vbi_transfer(FALSE);//cloud
//#endif
#endif
}

static void boot_config(void)
{
	/* set the boot up done flag */
	vpo_ioctl(g_vpo_dev,VPO_IO_BOOT_UP_DONE, 0);
}


static void dev_config(void)
{
	video_config();
	vbi_config();
	hdmi_config();
	boot_config();
}
	
void rpc_remote_dev_init(void)
{

	if(g_see_heap_top_addr != 0)
		hld_dev_see_init((void *)g_see_heap_top_addr);
	
	PRF("start to init remote rpc devices\n\n");
	dev_attach();
	dev_config();
	PRF("end the init job \n\n");
}

//add by phil for boot-media. only support mkv container. added by sam at 2013_05_14
void rpc_remote_boot_media(void)
{	
	if((m_boot_media_flag == 1) && (m_video_dview_disable == 1))
	{
#define EBML_ID_HEADER                0xA3DF451A

		// check mkv header
		if(*(volatile UINT32 *)m_boot_media_addr == EBML_ID_HEADER)
		{				
			printk("<0>""%s : boot media start addr 0x%08x len 0x%08x\n", __FUNCTION__, \
			       (unsigned int)m_boot_media_addr, (unsigned int)m_boot_media_len);
		
			deca_open((struct deca_device * )hld_dev_get_by_id(HLD_DEV_TYPE_DECA, 0), AUDIO_MPEG2, AUDIO_SAMPLE_RATE_48, AUDIO_QWLEN_24, 2, 0);
			//deca_io_control((struct deca_device * )hld_dev_get_by_id(HLD_DEV_TYPE_DECA, 0),DECA_SET_STR_TYPE,AUDIO_MPEG_ADTS_AAC);
			snd_open((struct snd_device *)hld_dev_get_by_id(HLD_DEV_TYPE_SND, 0));
			snd_set_volume((struct snd_device *)hld_dev_get_by_id(HLD_DEV_TYPE_SND, 0),SND_SUB_OUT,50);
			
			vdec_io_control(g_decv_dev0, VDEC_IO_SET_FB_COST_DOWN_NUM, 2);

			boot_media_start(__G_MM_VIDEO_START_ADDR, 2 * 0x2FD000);		
		}
	}
}

