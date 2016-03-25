
#ifndef _ALIDRIVERS_BOARD_CONFIG_H_

#define _ALIDRIVERS_BOARD_CONFIG_H_

#define DEFINE_BORAD_VARIABLES

#define CONFIG_ALI_CHIP_M3921

#ifndef DEFINE_BORAD_VARIABLES

/* chip type and version definition start*/
/* this area is decided by the ali's chipset.
it is updated with the new chipset.

	chip types description :
		name 		value
		M3602		0
		M3603		1
		M3921(arm)		2

	chip version description :
		version		value
		invalid            -1
		ver0			0
		ver1   		1
		...
		vern			n
*/
extern unsigned long __G_ALI_CHIP_TYPE;
extern long __G_ALI_CHIP_VERSION; 

/* chip type and version definition end*/

/* hardware board definition start */
extern unsigned long __G_ALI_BOARD_TYPE;
extern long __G_ALI_BOARD_VERSION;
/* hardware board name definition end */



/* memory mapping configuration start */

extern unsigned long __G_MM_TOP_ADDR;

extern unsigned long __G_MM_VIDEO_TOP_ADDR;
extern unsigned long __G_MM_VIDEO_START_ADDR;

extern unsigned long __G_MM_VCAP_FB_SIZE;
extern unsigned long __G_MM_VCAP_FB_ADDR;

extern unsigned long __G_MM_STILL_FRAME_SIZE;
extern unsigned long __G_MM_STILL_FRAME_ADDR;

/* shared memory area reserved for the RPC driver*/
extern unsigned long __G_MM_SHARED_MEM_TOP_ADDR;
extern unsigned long __G_MM_SHARED_MEM_START_ADDR;
extern unsigned long __G_RPC_MM_LEN;

/* private memory area reserved for the SEE CPU */
extern unsigned long __G_MM_PRIVATE_AREA_TOP_ADDR;
extern unsigned long __G_MM_PRIVATE_AREA_START_ADDR;
extern unsigned long __G_MM_VDEC_VBV_START_ADDR;
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
extern unsigned long __G_MM_SUB_PB_START_ADDR ;
extern unsigned long __G_MM_VDEC_VBV_LEN;

extern unsigned long g_fb3_max_width;
extern unsigned long g_fb3_max_height;
extern unsigned long g_fb3_pitch;
extern unsigned long g_fb3_bpp;

/* user data memory area reserved for bootloader -> kenerl */
extern unsigned long __G_MM_USER_DATA_MEM_LEN;
extern unsigned long __G_MM_USER_DATA_MEM_START;

extern unsigned long __G_MM_RESERVED_MEM_ADDR;
extern unsigned long __G_MM_RESERVED_MEM_SIZE;
extern unsigned long __G_MM_BOARD_SETTING_ADDR;

extern unsigned long __G_MM_BOOTLOGO_DATA_LEN;
extern unsigned long __G_MM_BOOTLOGO_DATA_START_ADDR;

extern unsigned long __G_MM_SGDMA_MEM_END;
extern unsigned long __G_MM_SGDMA_MEM_START;

extern unsigned long __G_SEE_DMX_SRC_BUF_END;
extern unsigned long __G_SEE_DMX_SRC_BUF_START;

extern unsigned long __G_SEE_DMX_DECRYPTO_BUF_END;
extern unsigned long __G_SEE_DMX_DECRYPTO_BUF_START;

extern unsigned long __G_MM_DMX_MEM_TOP_ADDR;
extern unsigned long __G_MM_DMX_MEM_START_ADDR;

extern unsigned long __G_MM_TSG_BUF_LEN;
extern unsigned long __G_MM_TSG_BUF_START_ADDR;

extern unsigned long __G_MM_MP_MEM_TOP_ADDR;
extern unsigned long __G_MM_MP_MEM_START_ADDR;

extern unsigned long __G_MM_FB_SIZE;
extern unsigned long __G_GE_CMD_SIZE;
extern unsigned long __G_MM_FB_START_ADDR;
extern unsigned long g_fb_max_width;
extern unsigned long g_fb_max_height;
extern unsigned long g_fb_pitch;
extern unsigned long g_fb_bpp;

extern unsigned long g_support_standard_fb;

extern unsigned long __G_MM_NIM_J83B_MEM_LEN;
extern unsigned long __G_MM_NIM_J83B_MEM_START_ADDR;
	
extern unsigned long __G_MM_IMAGE_DECODER_MEM_LEN;
extern unsigned long __G_MM_IMAGE_DECODER_MEM_START_ADDR;

extern unsigned long __G_MM_MCAPI_MEM_LEN;
extern unsigned long __G_MM_MCAPI_MEM_START_ADDR;

extern unsigned long __G_MM_AUDIO_MEM_LEN;
extern unsigned long __G_MM_AUDIO_MEM_START_ADDR;

/* the top address of the MAIN CPU */
#define MAX_MEM_REGION	8
extern unsigned long __G_MM_MAIN_MEM_NUM;
extern unsigned long __G_MM_MAIN_MEM[MAX_MEM_REGION][2];

extern unsigned long g_see_heap_top_addr;

/* memory mapping configuration end */


/* tv encoder parameters setting start*/

/* default scart output 
       0 : CVBS
	1 : RGB
	2 : SVIDEO
	3 : YUV
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

/* tv encoder parameters setting end*/

/* GPIO definition start */
extern unsigned long g_snd_mute_gpio;//37;
/* GPIO definition end */

/* PDK Freautes start */
extern unsigned long support_isdbt_cc;
//hdmi hdcp on/off
extern unsigned long g_hdmi_hdcp_enable;

extern unsigned int g_ali_ovg_phy_addr; 
extern unsigned int g_ali_ovg_mem_size;

extern unsigned int g_otp_set_vdac_fs;
/* PDK Freautes end */

#else

/* chip type and version definition start*/
/* this area is decided by the ali's chipset.
it is updated with the new chipset.

	chip types description :
		name 		value
		M3602		0
		M3603		1
		M3921(arm)		2

	chip version description :
		version		value
		invalid            -1
		ver0			0
		ver1   		1
		...
		vern			n
*/
unsigned long __G_ALI_CHIP_TYPE = -1;
long __G_ALI_CHIP_VERSION = -1; 

/* chip type and version definition end*/

/* hardware board definition start */
unsigned long __G_ALI_BOARD_TYPE = -1;
long __G_ALI_BOARD_VERSION = -1;
/* hardware board name definition end */

/* memory mapping configuration start */

unsigned long __G_MM_TOP_ADDR = 0;

unsigned long __G_MM_VIDEO_TOP_ADDR = 0;
unsigned long __G_MM_VIDEO_START_ADDR = 0;

unsigned long __G_MM_VCAP_FB_SIZE = 0;
unsigned long __G_MM_VCAP_FB_ADDR = 0;

unsigned long __G_MM_STILL_FRAME_SIZE = 0;
unsigned long __G_MM_STILL_FRAME_ADDR = 0;

/* shared memory area reserved for the RPC driver*/
unsigned long __G_MM_SHARED_MEM_TOP_ADDR = 0;
unsigned long __G_MM_SHARED_MEM_START_ADDR = 0;
unsigned long __G_RPC_MM_LEN = 0;

/* private memory area reserved for the SEE CPU */
unsigned long __G_MM_PRIVATE_AREA_TOP_ADDR = 0;
unsigned long __G_MM_PRIVATE_AREA_START_ADDR = 0;
unsigned long __G_MM_VDEC_VBV_START_ADDR = 0;
unsigned long __G_MM_VDEC_CMD_QUEUE_ADDR = 0;
unsigned long __G_MM_VDEC_LAF_FLAG_BUF_ADDR = 0;
unsigned long __G_MM_OSD_BK_ADDR = 0;
unsigned long __G_MM_TTX_BS_START_ADDR = 0;
unsigned long __G_MM_TTX_PB_START_ADDR = 0;
unsigned long __G_MM_TTX_SUB_PAGE_BUF_ADDR = 0;
unsigned long __G_MM_TTX_P26_NATION_BUF_ADDR = 0;
unsigned long __G_MM_TTX_P26_DATA_BUF_ADDR = 0;
unsigned long __G_MM_SUB_BS_START_ADDR =0;
unsigned long __G_MM_SUB_HW_DATA_ADDR = 0;
unsigned long __G_MM_SUB_PB_START_ADDR = 0;
unsigned long g_fb3_max_width = 0;
unsigned long g_fb3_max_height = 0;
unsigned long g_fb3_pitch = 0;
unsigned long g_fb3_bpp = 0;
unsigned long __G_MM_VDEC_VBV_LEN = 0;

/* user data memory area reserved for bootloader -> kenerl */
unsigned long __G_MM_USER_DATA_MEM_LEN = 0;
unsigned long __G_MM_USER_DATA_MEM_START = 0;

unsigned long __G_MM_RESERVED_MEM_ADDR = 0;
unsigned long __G_MM_RESERVED_MEM_SIZE = 0;
unsigned long __G_MM_BOARD_SETTING_ADDR = 0;

unsigned long __G_MM_BOOTLOGO_DATA_LEN = 0;
unsigned long __G_MM_BOOTLOGO_DATA_START_ADDR = 0;

unsigned long __G_MM_SGDMA_MEM_END = 0;
unsigned long __G_MM_SGDMA_MEM_START = 0;

unsigned long __G_MM_DMX_MEM_TOP_ADDR = 0;
unsigned long __G_MM_DMX_MEM_START_ADDR = 0;

unsigned long __G_MM_TSG_BUF_LEN = 0;
unsigned long __G_MM_TSG_BUF_START_ADDR = 0;

unsigned long __G_MM_MP_MEM_TOP_ADDR = 0;
unsigned long __G_MM_MP_MEM_START_ADDR = 0;

unsigned long __G_MM_FB_SIZE = 0;
unsigned long __G_GE_CMD_SIZE = 0;
unsigned long __G_MM_FB_START_ADDR = 0;
unsigned long g_fb_max_width = 0;
unsigned long g_fb_max_height = 0;
unsigned long g_fb_pitch = 0;
unsigned long g_fb_bpp = 0;

unsigned long g_support_standard_fb = -1;

unsigned long __G_MM_NIM_J83B_MEM_LEN = 0;
unsigned long __G_MM_NIM_J83B_MEM_START_ADDR = 0;
	
unsigned long __G_MM_IMAGE_DECODER_MEM_LEN = 0;
unsigned long __G_MM_IMAGE_DECODER_MEM_START_ADDR = 0;

unsigned long __G_MM_MCAPI_MEM_LEN = 0;
unsigned long __G_MM_MCAPI_MEM_START_ADDR = 0;

unsigned long __G_MM_AUDIO_MEM_LEN = 0;
unsigned long __G_MM_AUDIO_MEM_START_ADDR = 0;

/* the top address of the MAIN CPU */
#define MAX_MEM_REGION	8
unsigned long __G_MM_MAIN_MEM_NUM = 0;
unsigned long __G_MM_MAIN_MEM[MAX_MEM_REGION][2];

unsigned long g_see_heap_top_addr = 0;

/* memory mapping configuration end */


/* tv encoder parameters setting start*/

/* default scart output 
       0 : CVBS
	1 : RGB
	2 : SVIDEO
	3 : YUV
*/
long g_tve_default_scart_output = 0;

/* default tv mode */
long g_tve_hd_default_tv_mode = -1;
long g_tve_sd_default_tv_mode = -1;

/* whether use the CVBS output. the invalid value is -1 */
long g_tve_dac_use_cvbs_type = -1;

/* whether use the SVIDEO output. the invalid value is -1 */
long g_tve_dac_use_svideo_type = -1;

/* whether use the RGB output. the invalid value is -1 */
long g_tve_dac_use_rgb_type = -1;

/* whether use the YUV output. the invalie value is -1 */
long g_tve_dac_use_yuv_type = -1;

/* CVBS dac definition */
unsigned long g_tve_dac_cvbs = -1;

/* SVIDEO dac definition */
unsigned long g_tve_dac_svideo_y = 0;
unsigned long g_tve_dac_svideo_c = 0;

/* RGB dac definition */
unsigned long g_tve_dac_rgb_r = 0;
unsigned long g_tve_dac_rgb_g = 0;
unsigned long g_tve_dac_rgb_b = 0;

/* YUV dac definition */
unsigned long g_tve_dac_yuv_y = 0;
unsigned long g_tve_dac_yuv_u = 0;
unsigned long g_tve_dac_yuv_v = 0;

/* tv encoder parameters setting end*/

/* GPIO definition start */
unsigned long g_snd_mute_gpio = -1;//37;
/* GPIO definition end */

/* PDK Freautes start */
unsigned long support_isdbt_cc = 0;
//hdmi hdcp on/off
unsigned long g_hdmi_hdcp_enable = 0;

unsigned int g_ali_ovg_phy_addr = 0;
unsigned int g_ali_ovg_mem_size = 0;

unsigned int g_otp_set_vdac_fs = 0;
/* PDK Freautes end */

#endif

void customize_board_setting(void);

#endif

