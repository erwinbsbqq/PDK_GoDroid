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
/*
#include <linux/version.h>
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
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
#include <linux/ali_hdmi.h>
*/

#include <rpc_hld/ali_rpc_hld_dsc.h>

#include <ali_reg.h>
#include <ali_cache.h>
#include <ali_shm.h>  //for __VMTSALI
//#include "ali_rpc.h"

#if 1
#define PRF printf
#else
#define PRF(...) 	do{}while(0)
#endif

//#define JTAG_DEBUG

extern unsigned long g_see_heap_top_addr;

void hld_decv_set_cur_dev(int dev);

#if 0
extern unsigned long __G_MM_VIDEO_TOP_ADDR;
extern unsigned long __G_MM_VIDEO_START_ADDR;

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

/* CVBS dac definition */
extern unsigned long g_tve_dac_cvbs;

/* SVIDEO dac definition */
extern unsigned long g_tve_dac_svideo_y;
extern unsigned long g_tve_dac_svideo_c;

/* RGB dac definition */
extern unsigned long g_tve_dac_rgb_r;
extern unsigned long g_tve_dac_rgb_g;
extern unsigned long g_tve_dac_rgb_b;

/* YUV dac definition */
extern unsigned long g_tve_dac_yuv_y;
extern unsigned long g_tve_dac_yuv_u;
extern unsigned long g_tve_dac_yuv_v;

extern unsigned long support_isdbt_cc;


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
#define __MM_OSD2_LEN					(0x1FA400) // (1920 * 1080)
#define __MM_TTX_BS_LEN				(0x5000)
#define __MM_TTX_PB_LEN	            		(0xCB200)
#define __MM_TTX_SUB_PAGE_LEN       		(0x14500)
#define __MM_TTX_P26_NATION_LEN   		(0x61A80)
#define __MM_TTX_P26_DATA_LEN       		(0x3E8)
#define __MM_SUB_BS_LEN				(0xA000)
#define __MM_SUB_PB_LEN				(0x50000)
#define __MM_SUB_HW_DATA_LEN 		(0xC000)

#define __MM_ISDBT_BS_LEN			(0x8EC0)
#define __MM_ISDBT_PB_LEN			(0x7E900)

extern unsigned long __G_MM_VDEC_VBV_START_ADDR;
extern unsigned long __G_MM_VDEC_VBV_LEN;
extern unsigned long __G_MM_VDEC_CMD_QUEUE_ADDR;
extern unsigned long __G_MM_VDEC_LAF_FLAG_BUF_ADDR;
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

struct vbi_device *g_vbi_config_dev = NULL;
struct vbi_config_par g_vbi_config;



#define 	OSD_TRANSPARENT_COLOR		0xFF

struct sdec_feature_config g_sdec_config;
struct sdec_device *g_sdec_device = NULL;

struct gma_device *g_gma_dev1 = NULL;


#endif



static void dev_attach(void)
{



//	ali_m36_dsc_see_init();
}

static void boot_config(void)
{
	/* set the boot up done flag */
//	vpo_ioctl(g_vpo_dev,VPO_IO_BOOT_UP_DONE, 0);
}

static void dev_config(void)
{
//	video_config();
     //   vbi_config();
//	hdmi_config();
    //    boot_config();
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
/*
//add by phil for boot-media. only support mkv container. added by sam at 2013_05_14
void rpc_remote_boot_media(void)
{	
	if((m_boot_media_flag == 1) && (m_video_dview_disable == 1))
	{
#define EBML_ID_HEADER                0xA3DF451A

		// check mkv header
		if(*(volatile UINT32 *)m_boot_media_addr == EBML_ID_HEADER)
		{				
			printk("<0>""%s : boot media start addr 0x%08x len 0x%08x\n", __FUNCTION__, m_boot_media_addr, m_boot_media_len);
		
			deca_open((struct deca_device * )hld_dev_get_by_id(HLD_DEV_TYPE_DECA, 0), AUDIO_MPEG2, AUDIO_SAMPLE_RATE_48, AUDIO_QWLEN_24, 2, 0);
			//deca_io_control((struct deca_device * )hld_dev_get_by_id(HLD_DEV_TYPE_DECA, 0),DECA_SET_STR_TYPE,AUDIO_MPEG_ADTS_AAC);
			snd_open((struct snd_device *)hld_dev_get_by_id(HLD_DEV_TYPE_SND, 0));
			snd_set_volume((struct snd_device *)hld_dev_get_by_id(HLD_DEV_TYPE_SND, 0),SND_SUB_OUT,50);
			
			vdec_io_control(g_decv_dev0, VDEC_IO_SET_FB_COST_DOWN_NUM, 2);

			boot_media_start(__VMTSALI(__G_MM_VIDEO_START_ADDR), 2 * 0x2FD000);		
		}
	}
}
*/

