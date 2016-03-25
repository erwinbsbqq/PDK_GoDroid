#ifndef _BOARD_ATTR_H_
#define _BOARD_ATTR_H_
#include <rpc_hld/ali_rpc_hld_dis.h>
#include "ali_smc_common.h"

//#define TVE_ADJUST_TABLE_LEN 	(sizeof(struct tve_adjust) * 24)
#define TVE_ADJUST_TABLE_LEN 	24	
//#define SD_TVE_ADJUST_TABLE_ADV_LEN	(sizeof(struct tve_adjust) * 20)
#define SD_TVE_ADJUST_TABLE_ADV_LEN	20
//#define TVE_ADJUST_TABLE_TOTAL_LEN	(sizeof(T_TVE_ADJUST_ELEMEMT) * TVE_ADJ_FIELD_NUM * TVE_SYS_NUM)
#define TVE_ADJUST_TABLE_TOTAL_LEN	(TVE_SYS_NUM)

#ifndef SMC_GPIO_CLASS_NUM
	#define SMC_GPIO_CLASS_NUM	2
#endif

typedef struct _smc_gpio_class_t {
	int gpio_selection[SMC_GPIO_CLASS_NUM];	
	int selectors_num;
	int pin_ext_diff;
	int power_volctrl_diff;
} smc_gpio_class_t;

typedef struct _smc_attr_t {
	struct smc_dev_cfg config_param;
	smc_gpio_class_t smc_gpio_class;
	int smc_strip_pin_bit;
	int smc_strip_pin_bit1;
	int smc_pin_mux_bit;
} smc_attr_t;

typedef struct _video_mem_attr_t {
	unsigned long video_end_addr;
	unsigned long video_start_addr;
	unsigned long private_video_end_addr;
	unsigned long private_video_start_addr;
} video_mem_attr_t;

typedef struct _avc_mem_attr_t {
	unsigned long avc_fb_num;
	unsigned long avc_extra_fb_num;
	unsigned long avc_fb_addr;
	unsigned long avc_fb_len;
	unsigned long avc_dview_addr;
	unsigned long avc_dview_len;
	unsigned long avc_mv_addr;
	unsigned long avc_mv_len;
	unsigned long avc_vbv_addr;
	unsigned long avc_vbv_len;
	unsigned long avc_cmd_queue_addr;
	unsigned long avc_cmd_queue_len;
	unsigned long avc_mb_col_addr;
	unsigned long avc_mb_col_len;
	unsigned long avc_mb_nei_addr;
	unsigned long avc_mb_nei_len;
	unsigned long avc_laf_rw_buf_addr;
	unsigned long avc_laf_rw_buf_len;
	unsigned long avc_laf_flag_buf_addr;
	unsigned long avc_laf_flag_buf_len;
} avc_mem_attr_t;

typedef struct _shared_mem_attr_t {
	unsigned long shared_mem_end_addr;
	unsigned long shared_mem_start_addr;
	unsigned long rpc_mem_size;
} shared_mem_attr_t;

typedef struct _private_mem_attr_t {
	unsigned long private_area_end_addr;
	unsigned long private_area_start_addr;
} private_mem_attr_t;

typedef struct _vdec_mem_attr_t {
	unsigned long vdec_vbv_start_addr;
	unsigned long vdec_vbv_end_addr;
	unsigned long vdec_cmd_queue_start_addr;	/* unused */
	unsigned long vdec_cmd_queue_end_addr;		/* unused */
	unsigned long vdec_laf_flag_buf_start_addr;	/* unused */
	unsigned long vdec_laf_flag_buf_end_addr;	/* unused */
	unsigned long vdec_vbv_addr;
	unsigned long vdec_vbv_len;
	unsigned long vdec_maf_addr;
	unsigned long vdec_maf_len;
} vdec_mem_attr_t;

typedef struct _osd_mem_addr_t {
	unsigned long osd_bk_start_addr;
	unsigned long osd_bk_end_addr;
	unsigned long osd2_bk_start_addr;
	unsigned long osd2_bk_end_addr;	/* Why there are two buffers related to OSD */
} osd_mem_attr_t;

typedef struct ttx_mem_attr_t {
	unsigned long ttx_bs_start_addr;
	unsigned long ttx_bs_end_addr;
	unsigned long ttx_pb_start_addr;
	unsigned long ttx_pb_end_addr;
	unsigned long ttx_sub_page_buf_start_addr;
	unsigned long ttx_sub_page_buf_end_addr;
	unsigned long ttx_p26_nation_buf_start_addr;
	unsigned long ttx_p26_nation_buf_end_addr;
	unsigned long ttx_p26_data_buf_start_addr;
	unsigned long ttx_p26_data_buf_end_addr;
} ttx_mem_attr_t;

typedef struct _subtitle_mem_attr_t {
	unsigned long sub_bs_start_addr;
	unsigned long sub_bs_end_addr;
	unsigned long sub_hw_data_start_addr;
	unsigned long sub_hw_data_end_addr;
	unsigned long sub_pb_start_addr;
	unsigned long sub_pb_end_addr;
	unsigned long subt_atsc_sec_start_addr;
	unsigned long subt_atsc_sec_end_addr;
} subtitle_mem_attr_t;

typedef struct _ge_mem_attr_t {
	unsigned long ge_cmdq_end_addr;
	unsigned long ge_cmdq_start_addr;
} ge_mem_attr_t;

typedef struct _fb_mem_attr_t {
	unsigned long fb3_max_width;
	unsigned long fb3_max_height;
	unsigned long fb3_pitch;
	unsigned long fb3_bpp;

	unsigned long fb_start_addr;
	unsigned long fb_size;

	unsigned long fb_max_width;
	unsigned long fb_max_height;
	unsigned long fb_pitch;
	unsigned long fb_bpp;

	unsigned long support_std_fb;
	
	unsigned long vpo_cap_fb_start_addr;
	unsigned long vpo_cap_fb_len;

	unsigned long vdec_fb_addr;
	unsigned long vdec_fb_len;

	unsigned long codec_fb_mem_len;
	unsigned long codec_fb_start_addr;
} fb_mem_attr_t;

typedef struct _dmx_mem_attr_t {
	unsigned long dmx_mem_end_addr;
	unsigned long dmx_mem_start_addr;
	int shared_dmx_mem_with_mp;
} dmx_mem_attr_t;

typedef struct _image_decoder_mem_attr_t {
	unsigned long image_decoder_mem_len;
	unsigned long image_decoder_mem_start_addr;
} image_decoder_mem_attr_t;

typedef struct _mp_mem_attr_t {
	unsigned long mp_mem_end_addr;
	unsigned long mp_mem_start_addr;
	unsigned long mp_mem_len;
	int reserve_mp_mem_bootmem;
	int mp_mem_page_align;
} mp_mem_attr_t;

typedef struct _file_signature_mem_attr_t {
	unsigned long file_signature_end_addr;
	unsigned long file_signature_start_addr;
} file_signature_mem_attr_t;

typedef struct _ui_rsc_mem_attr_t {
	unsigned long ui_rsc_map_start_addr;
	unsigned long ui_rsc_map_end_addr;
} ui_rsc_mem_attr_t;

typedef struct _upg_mem_attr_t {
	/* 
	 * Here don't conform with name rule "xx_start_addr <-> xx_end_addr, 
	 * because the memory should be alloced by kernel 
	 */
	unsigned long upg_mem_start_addr;
	unsigned long upg_mem_len;		
	unsigned long reserve_upg_mem_bootmem;
} upg_mem_attr_t;

typedef struct _scart_attr_t {
	long tve_default_scart_output;
} scart_attr_t;


typedef struct _tv_mode_attr_t {
	long tve_hd_default_tv_mode;
	long tve_sd_default_tv_mode;
} tv_mode_attr_t;

typedef struct _tve_dac_cvbs_attr_t {
	long tve_dac_use_cvbs_type;
	unsigned long tve_dac_cvbs;
} tve_dac_cvbs_attr_t;


typedef struct _tve_dac_svideo_attr_t {
	long tve_dac_use_svideo_type;
	unsigned long tve_dac_svideo_y;
	unsigned long tve_dac_svideo_c;
} tve_dac_svideo_attr_t;

typedef struct _tve_dac_rgb_attr_t {
	long tve_dac_use_rgb_type;
	unsigned long tve_dac_rgb_r;
	unsigned long tve_dac_rgb_g;
	unsigned long tve_dac_rgb_b;
} tve_dac_rgb_attr_t;

typedef struct _tve_dac_yuv_attr_t {
	long tve_dac_use_yuv_type;
	unsigned long tve_dac_yuv_y;
	unsigned long tve_dac_yuv_u;
	unsigned long tve_dac_yuv_v;
} tve_dac_yuv_attr_t;

typedef struct  _sound_attr_t {
	unsigned long sound_mute_gpio;
	unsigned long sound_mute_polar;
	unsigned long sound_qfp_bga;
} sound_attr_t;

typedef struct _tve_adjust_attr_t {
	struct tve_adjust *g_tve_adjust_table;
	struct tve_adjust *g_sd_tve_adjust_table;
	struct tve_adjust *g_sd_tve_adjust_table_adv;
	T_TVE_ADJUST_TABLE *g_tve_adjust_table_total;
	struct tve_adjust gg_tve_adjust_table[TVE_ADJUST_TABLE_LEN];
	struct tve_adjust gg_sd_tve_adjust_table[TVE_ADJUST_TABLE_LEN];
	struct tve_adjust gg_sd_tve_adjust_table_adv[SD_TVE_ADJUST_TABLE_ADV_LEN];
	T_TVE_ADJUST_TABLE gg_tve_adjust_table_total[TVE_ADJUST_TABLE_TOTAL_LEN];
	T_TVE_ADJUST_ELEMENT gg_tve_adjust_element_table[TVE_ADJUST_TABLE_TOTAL_LEN][TVE_ADJ_FIELD_NUM];
}  tve_adjust_attr_t;

typedef struct _tve_attr_t {
	scart_attr_t scart_attr;
	tv_mode_attr_t tv_mode_attr;
	tve_dac_cvbs_attr_t tve_dac_cvbs_attr;
	tve_dac_svideo_attr_t tve_dac_svideo_attr;
	tve_dac_rgb_attr_t tve_dac_rgb_attr;
	tve_dac_yuv_attr_t tve_dac_yuv_attr;
	tve_adjust_attr_t tve_adjust_attr;
} tve_attr_t;

typedef struct _mem_section_attr_t {
	unsigned long total_mem_start_addr;
	unsigned long total_mem_size;
	unsigned long see_start_addr;
	unsigned long see_mem_size;
#ifndef __MM_KERNEL_HEAP_AREA_SECTIONS
	#define __MM_KERNEL_HEAP_AREA_SECTIONS	2
#endif
	unsigned long main_mem_sections[__MM_KERNEL_HEAP_AREA_SECTIONS][2];
	unsigned long main_mem_sections_num;
} mem_section_attr_t;

typedef struct _feature_attr_t {
	unsigned long support_isdbt_cc;
	unsigned long support_afd_wss;
	unsigned long support_afd_scale;
	unsigned long support_continue_memory;
	unsigned long vdec_first_i_freerun;
	int support_hdmi;
	unsigned int otp_set_vdac_fs;
    unsigned long support_audio_desc;
} feature_attr_t; 


typedef struct _tuner_attr_t {
	char tuner_name[2][16];
	unsigned long tuner_num;
} tuner_attr_t;

typedef struct _board_attr_t {
	/* memory layout */
	mem_section_attr_t mem_section_attr;
	avc_mem_attr_t avc_mem_attr;
	video_mem_attr_t video_mem_attr;
	shared_mem_attr_t shared_mem_attr;
	private_mem_attr_t private_mem_attr;
	vdec_mem_attr_t vdec_mem_attr;
	osd_mem_attr_t osd_mem_attr;
	ttx_mem_attr_t ttx_mem_attr;
	subtitle_mem_attr_t subtitle_mem_attr;
	ge_mem_attr_t ge_mem_attr;
	fb_mem_attr_t fb_mem_attr;
	dmx_mem_attr_t dmx_mem_attr;
	image_decoder_mem_attr_t image_decoder_mem_attr;
	mp_mem_attr_t mp_mem_attr;
	file_signature_mem_attr_t file_signature_mem_attr;
	ui_rsc_mem_attr_t ui_rsc_mem_attr;
	upg_mem_attr_t upg_mem_attr;
	/* tv encoder parameters */
	tve_attr_t tve_attr;
	/* gpio pin setting*/
	sound_attr_t sound_attr;
	/* feature support or setting*/
	feature_attr_t feature_attr;
	tuner_attr_t tuner_attr;
	smc_attr_t smc_attr;
} board_attr_t;

typedef enum {
	VIDEO_MEM = 0,
	AVC_MEM,
	SHARE_MEM,
	PRIVATE_MEM,
	OSD_MEM,
	TTX_MEM,
	SUBTITLE_MEM,
	GE_MEM,
	FB_MEM,
	DMX_MEM,
	IMAGE_DECODER_MEM,
	MP_MEM,
	FILE_SIGNATURE_MEM,
	UI_RSC_MEM,
	UPG_MEM,
	TV_MODE,
	TVE_DAC_SVIDEO,
	TVE_DAC_RGB,
	TVE_DAC_YUV,
	SOUND,
	TVE_ADJUST,
	MEM_SECTION,
	TUNER,
	VDEC_MEM,
	TVE_DAC_CVBS,
	FEATURE,
	SCART,
	SMC,
} req_attr_t;

avc_mem_attr_t * get_avc_mem_attr(void);
image_decoder_mem_attr_t * get_image_decoder_mem_attr(void);
scart_attr_t * get_scart_attr(void);
vdec_mem_attr_t * get_vdec_mem_attr(void);
tve_dac_cvbs_attr_t  * get_tve_dac_cvbs_attr(void);
feature_attr_t * get_feature_attr(void);
video_mem_attr_t * get_video_mem_attr(void);
unsigned long get_video_end_addr(void);
shared_mem_attr_t * get_shared_mem_attr(void);
unsigned long get_shared_mem_start_addr(void);
unsigned long get_rpc_mem_size(void);
private_mem_attr_t * get_private_mem_attr(void);
unsigned long get_private_area_end_addr(void);
unsigned long get_private_area_start_addr(void);
osd_mem_attr_t * get_osd_mem_attr(void);
ttx_mem_attr_t * get_ttx_mem_attr(void);
subtitle_mem_attr_t * get_subtitle_mem_attr(void);
ge_mem_attr_t * get_ge_mem_attr(void);
fb_mem_attr_t * get_fb_mem_attr(void);
dmx_mem_attr_t * get_dmx_mem_attr(void);
mp_mem_attr_t * get_mp_mem_attr(void);
file_signature_mem_attr_t * get_file_signature_mem_attr(void);
ui_rsc_mem_attr_t * get_ui_rsc_mem_attr(void);
upg_mem_attr_t * get_upg_mem_attr(void);
tv_mode_attr_t * get_tv_mode_attr(void);
tve_dac_svideo_attr_t * get_tve_dac_svideo_attr(void);
tve_dac_rgb_attr_t * get_tve_dac_rgb_attr(void);
tve_dac_yuv_attr_t * get_tve_dac_yuv_attr(void);
sound_attr_t * get_sound_attr(void);
tve_adjust_attr_t * get_tve_adjust_attr(void);
mem_section_attr_t * get_mem_section_attr(void);
tuner_attr_t * get_tuner_attr(void);
void * request_attr(req_attr_t req);
smc_attr_t * get_smc_attr(void);
smc_gpio_class_t *get_smc_gpio_class(void);

#endif
