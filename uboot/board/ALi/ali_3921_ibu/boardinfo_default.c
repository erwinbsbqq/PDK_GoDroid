#include <common.h>
#include <alidefinition/adf_boot.h>
#include <alidefinition/adf_vpo.h>
#include <alidefinition/adf_media.h>
#include <alidefinition/adf_snd.h>
#include <alidefinition/adf_gma.h>
#include "tve_info.h"

unsigned char  ali_mac_default[6] = {0xde,0xad,0xbe,0xef,0x01,0x01};
unsigned char  ali_cmdline_default[] = "ubi.mtd=10 root=ubi0:rootfs rootfstype=ubifs rw init=/init androidboot.console=ttyS0 mtdparts=ali_nand:8M@0M(boot),8M@8M(bootbak),8M@16M(bootargs),8M@24M(deviceinfo),8M@32M(baseparams),8M@40M(bootlogo),8M@48M(bootmedia),8M@56M(see),16M@64M(kernel),8M@80M(ae),500M@88M(rootfs),64M@588M(appdata)";
	
unsigned char  ali_cmdline_default_rcv[] = "ubi.mtd=10 root=ubi0:rootfs rootfstype=ubifs rw init=/init androidboot.console=ttyS0 mtdparts=ali_nand:8M@0M(boot),8M@8M(bootbak),8M@16M(bootargs),8M@24M(deviceinfo),8M@32M(baseparams),8M@40M(bootlogo),8M@48M(bootmedia),8M@56M(see),16M@64M(kernel),8M@80M(ae),500M@88M(rootfs),64M@588M(appdata)";
	
static int init_hdcp(ADF_BOOT_BOARD_INFO *boardinfo)
{
	static ADF_BOOT_HDCP hdcp_info;
	
	memset(&hdcp_info,0,sizeof(ADF_BOOT_HDCP));
	
	memset(&hdcp_info.hdcp,0,288);
//	hdcp_info.hdcp_disable = 1;//  1: disable hdmi hdcp move to  avinfo

	memcpy(&(boardinfo->hdcp),&hdcp_info,sizeof(ADF_BOOT_HDCP));

	return 0;
}

static int init_avinfo(ADF_BOOT_BOARD_INFO *boardinfo)
{
	static ADF_BOOT_AVINFO avinfo;
	
	memset(&avinfo,0,sizeof(ADF_BOOT_AVINFO));

	avinfo.tvSystem = LINE_720_30;
	
	avinfo.progressive = FALSE;//actually not used now
	avinfo.tv_ratio = TV_AUTO;//actually not used now
	avinfo.display_mode = LETTERBOX;//actually not used now
	
	avinfo.scart_out = 3;//do not change!
	avinfo.vdac_out[0] = CVBS_1;//actually not used now
	avinfo.vdac_out[1] = CVBS_2;//actually not used now
	avinfo.vdac_out[2] = CVBS_3;//actually not used now
	avinfo.vdac_out[3] = CVBS_4;//actually not used now
	avinfo.vdac_out[4] = CVBS_5;//actually not used now
	avinfo.vdac_out[5] = CVBS_6;//actually not used now
	avinfo.video_format = 0;//actually not used now
	avinfo.audio_output = SND_OUT_SPDIF_PCM;//actually not used now
	
	avinfo.brightness = 50;
	avinfo.contrast = 50;
	avinfo.saturation = 50;
	avinfo.sharpness = 5;
	avinfo.hue = 50;
	
	avinfo.snd_mute_gpio = 80;
	avinfo.snd_mute_polar = 0;

	avinfo.hdcp_disable = 1;//  1: disable hdmi hdcp move to 

	memset(&avinfo.resv,0,8);//be careful reserver size
	
	memcpy(&(boardinfo->avinfo),&avinfo,sizeof(ADF_BOOT_AVINFO));
	return 0;
}

static int init_memmapinfo(ADF_BOOT_BOARD_INFO *boardinfo)
{
	static ADF_BOOT_MEMMAPINFO memmap_info;
	
	memset(&memmap_info,0,sizeof(ADF_BOOT_MEMMAPINFO));
	
	memmap_info.kernel_start = 0x00008000;
	memmap_info.kernel_len = BOOT_MAGIC_NUM;

	memmap_info.boot_see_start = BOOT_MAGIC_NUM;
	memmap_info.boot_see_len = BOOT_MAGIC_NUM;
	
	memmap_info.see_start = 0x04000200;
	memmap_info.see_len = BOOT_MAGIC_NUM;

	memmap_info.ae_start = 0x05EFD200;
	memmap_info.ae_len = BOOT_MAGIC_NUM;

	memmap_info.mcapi_start = 0x05FFD000;
	memmap_info.mcapi_len = BOOT_MAGIC_NUM;
	
	memmap_info.vbv_start = 0x058FD000;
	memmap_info.vbv_len = 0x600000 ;

	memmap_info.decv_fb_start = 0x06200000;//video frame buffer
	memmap_info.decv_fb_len = 0x05A00000;

	memmap_info.decv_raw_start = 0x0575D800;
	memmap_info.decv_raw_len = 0x19F800;

	memmap_info.osd_fb_start = BOOT_MAGIC_NUM;
	memmap_info.osd_fb_len = BOOT_MAGIC_NUM;

	memmap_info.boot_media_start = 0x02E00000;
	memmap_info.boot_media_len = BOOT_MAGIC_NUM;
	
	memmap_info.ramdisk_start= 0x81008000;
	memmap_info.ramdisk_len= BOOT_MAGIC_NUM;

	memmap_info.cmd_queue_buffer_start= 0x0575D800;
	memmap_info.cmd_queue_buffer_len= 0x19F800;

	memmap_info.vcap_buffer_start= 0x05421000;
	memmap_info.vcap_buffer_len= 0x33c000;

	memmap_info.boot_media_mkv_buf_start= 0x0bc00000;
	memmap_info.boot_media_mkv_buf_len= 0x1000000;

	memset(&memmap_info.reserve,0,772);//be careful reserver size

	memcpy(&(boardinfo->memmap_info),&memmap_info,sizeof(ADF_BOOT_MEMMAPINFO));
	return 0;
}

static int init_tveinfo(ADF_BOOT_BOARD_INFO *boardinfo)
{
	static ADF_BOOT_TVEINFO tve_info;
	unsigned int i = 0;
	unsigned int j = 0;
	
	memset(&tve_info,0,sizeof(ADF_BOOT_TVEINFO));
	
	memset(&(tve_info.tve_adjust_table_info[0]),0,sizeof(tve_info.tve_adjust_table_info));
	memset(&(tve_info.sd_tve_adj_table_info[0]),0,sizeof(tve_info.sd_tve_adj_table_info));
	memset(&(tve_info.sd_tve_adv_adj_table_info[0]),0,sizeof(tve_info.sd_tve_adv_adj_table_info));

	memcpy(&(tve_info.tve_adjust_table_info[0]),&(g_tve_adjust_table_total[0]),sizeof(g_tve_adjust_table_total));
	
	for(i = 0;i<TVE_SYS_NUM;i++)
	{
		tve_info.tve_adjust_table_info[i].index  = g_tve_adjust_table_total[i].index;	
		memcpy(&tve_info.tve_adjust_table_info[i].field_info,g_tve_adjust_table_total[i].pTable,sizeof(T_TVE_ADJUST_ELEMENT));
	}

	memcpy(&(tve_info.sd_tve_adj_table_info[0]),&(g_sd_tve_adjust_table[0]),sizeof(g_sd_tve_adjust_table));
	memcpy(&(tve_info.sd_tve_adv_adj_table_info[0]),&(g_sd_tve_adjust_table_adv[0]),sizeof(g_sd_tve_adjust_table_adv));
	
	memcpy(&(boardinfo->tve_info),&tve_info,sizeof(ADF_BOOT_TVEINFO));
	return 0;
}


static int init_reginfo(ADF_BOOT_BOARD_INFO *boardinfo)
{
	static ADF_REGISTERINFO reg_info;
	
	memset(&reg_info,0,sizeof(ADF_REGISTERINFO));
	
	reg_info.valid_count = 0;
	memset(&reg_info.unit,0,sizeof(reg_info.unit));
	
	memcpy(&(boardinfo->reg_info),&reg_info,sizeof(ADF_REGISTERINFO));
	return 0;
}

static int init_macinfo(ADF_BOOT_BOARD_INFO *boardinfo)
{	
	memcpy(&(boardinfo->macinfo),ali_mac_default,sizeof(ADF_BOOT_MAC_PHYADDR));
	return 0;
}

static int init_mediainfo(ADF_BOOT_BOARD_INFO *boardinfo)
{
	static ADF_BOOT_MEDIAINFO media_info;
	
	memset(&media_info,0,sizeof(ADF_BOOT_MEDIAINFO));
	
	media_info.play_enable = 1;
	
	memcpy(&(boardinfo->media_info),&media_info,sizeof(ADF_BOOT_MEDIAINFO));
	return 0;
}

static int init_gmainfo(ADF_BOOT_BOARD_INFO *boardinfo)
{
	static ADF_BOOT_GMA_INFO gma_info;
	
	memset(&gma_info,0,sizeof(ADF_BOOT_GMA_INFO));
	
	gma_info.gma_enable = 0;//BOOT_MAGIC_NUM;
	memcpy(&(boardinfo->gma_info),&gma_info,sizeof(ADF_BOOT_GMA_INFO));
	return 0;
}


int set_default_boardinfo(void)
{
	int ret = 0;
	ADF_BOOT_BOARD_INFO *boardinfo = (ADF_BOOT_BOARD_INFO *)(PRE_DEFINED_ADF_BOOT_START);

	ret = init_hdcp(boardinfo);
	ret = init_avinfo(boardinfo);
	ret = init_memmapinfo(boardinfo);
	ret = init_tveinfo(boardinfo);
	ret = init_reginfo(boardinfo);
	ret = init_macinfo(boardinfo);
	ret = init_mediainfo(boardinfo);
	ret = init_gmainfo(boardinfo);

	if(ret != 0 )
	{
		printf("set boardinfo fail??\n");
	}
		
	return 0;
}

