#ifndef __HLD_APE_PRIV_H__
#define __HLD_APE_PRIV_H__

#include <hld_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APE_DEBUG(fmt, args...)  \
			do { \
				if (ape_get_dbg_on()) \
				{ \
					ADR_DBG_PRINT(APE, "%s->%s: L %d: "fmt"\n", __FILE__, \
						__FUNCTION__, __LINE__, ##args); \
				} \
			} while(0)

#define APE_LOG(fmt, args...)  \
			do { \
				{ \
					ADR_DBG_PRINT(APE, "%s->%s: L %d: "fmt"\n", __FILE__, \
						__FUNCTION__, __LINE__, ##args); \
				} \
			} while(0)			

#define APE_IN()			APE_DEBUG("PID %d in\n", getpid())
#define APE_OUT()			APE_DEBUG("PID %d out\n", getpid())

#define VPKT_HDR_SBM_IDX  0
#define VPKT_DATA_SBM_IDX 1
#define DECV_OUT_SBM_IDX  2
#define DISP_IN_SBM_IDX   3
#define APKT_HDR_SBM_IDX  4
#define APKT_DATA_SBM_IDX 5
#define DECA_OUT_SBM_IDX  6
#define SND_IN_SBM_IDX    7

#define SBMDEV_VPKT_HDR   "/dev/ali_sbm0"
#define SBMDEV_VPKT_DATA  "/dev/ali_sbm1"
#define SBMDEV_DECV_OUT   "/dev/ali_sbm2"
#define SBMDEV_DISP_IN    "/dev/ali_sbm3"
#define SBMDEV_APKT_HDR   "/dev/ali_sbm4"
#define SBMDEV_APKT_DATA  "/dev/ali_sbm5"
#define SBMDEV_DECA_OUT   "/dev/ali_sbm6"
#define SBMDEV_SND_IN     "/dev/ali_sbm7"

#define STCDEV0_FILENAME "/dev/ali_stc0"
#define STCDEV1_FILENAME "/dev/ali_stc1"

#define VIDEODEV          "/dev/ali_video0"
#define AUDIODEV          "/dev/ali_m36_audio0"

#define   VPO_IO_SET_ENHANCE_BRIGHTNESS 0x01    // value[0, 100], default 50
#define   VPO_IO_SET_ENHANCE_CONTRAST   0x02    // value[0, 100], default 50
#define   VPO_IO_SET_ENHANCE_SATURATION	0x04    // value[0, 100], default 50

#define VPO_IO_VIDEO_ENHANCE			    0x12

#define APE_MAGIC	"aliape01"

#define APE_BLOCK_MAGIC "9922337755664488"

enum APE_BLOCK_TYPE
{
	BLOCK_END = 0, 
	BLOCK_VIDEO_DATA = 1,
	BLOCK_AUDIO_DATA = 2,
	BLOCK_VIDEO_HEADER = 3,
	BLOCK_AUDIO_HEADER = 4,
	BLOCK_VIDEO_INIT_INFO = 5,
	BLOCK_AUDIO_INIT_INFO = 6,
	BLOCK_VIDEO_EXTRA_DATA = 7,
	BLOCK_AUDIO_EXTRA_DATA = 8,	
	BLOCK_VIDEO_CODEC_TYPE = 9,
};

typedef struct APE_BLOCK_HEADER_T
{
	enum APE_BLOCK_TYPE type;
	unsigned int len;
}ape_block_header_t;

typedef struct Vpo_Video_Enhance_T
{
	UINT8	changed_flag;
	UINT16   grade;
} vpo_video_enhance_t;

typedef struct Ape_Priv_T
{
	int v_init_flag;
	int a_init_flag;
	int vpkt_data_fd; 
  	int vpkt_hdr_fd; 
  	int decv_out_fd; 
  	int disp_in_fd; 
  	int apkt_data_fd; 
  	int apkt_hdr_fd; 
  	int deca_out_fd; 
  	int snd_in_fd;
	int pe_fd;
	int clock0_fd;
	int clock1_fd;
	int zoom_flag;
	int video_fd;
	int fd_open_flag;
	int av_output_mode;
	char dump_file_name[1024];
	unsigned int video_pts[10];
	unsigned int audio_pts[10];
	unsigned int video_hdr_num;
	unsigned int video_pkt_num;
	unsigned int audio_hdr_num;
	unsigned int audio_pkt_num;
	unsigned int data_dump_flag;
	unsigned int codec_tag;
	unsigned int pcm_buf_size;
	unsigned int pcm_buf_addr;
	FILE* dump_file;
	struct Video_Rect  src_rect;
    struct Video_Rect  dst_rect;

	audio_config_info_t	 audio_info;
#ifdef AUD_MERGE_BLOCK
    audio_merge_info_t audio_merge_info; //add by jacket for audio merge blocks
    unsigned int audio_merge_mode;
#endif
	video_config_info_t video_info;
	struct avsync_device *pavsync_dev;

	int sbm_total_size;
	int sbm_reserved_size;
	int sbm_lock;
	int sbm_last_size;

	int task_id;
	unsigned char vid_rot_en;
	unsigned int vid_rot_angle;
	int bdma_mode;
	unsigned char *phy_base;
	unsigned char * mmap_base;
	unsigned int mmap_size;
	unsigned int mmap_offset;

    int rotate_flag;
    enum VDecRotationAngle frame_angle;
} ape_priv_t;

ape_priv_t* ape_get_priv();
ape_priv_t* ape_malloc_priv();
void ape_free_priv(void);
int ape_check_fd(int fd);
void ape_store_pts(int fd, unsigned int pts);
unsigned int ape_get_pts(int fd);
int ape_adec_sbm_reset();
int ape_vdec_sbm_reset();
int ape_fd_open();
void ape_fd_close();
int ape_check_codec_support(unsigned int codec_tag);
int ape_video_select_decoder(ape_priv_t *ape_priv, int preview);
int ape_video_start(ape_priv_t *ape_priv);
int ape_video_stop(ape_priv_t *ape_priv, int bclosevp, int bfillblack);
int ape_video_decore_ioctl(ape_priv_t* ape_priv, int cmd, void *param1, void *param2);
int ape_config_video_init_par(video_config_info_t* info);
int ape_get_io_status(struct VDec_StatusInfo *status);
int ape_vpo_ioctl(UINT32 dwCmd, UINT32 dwParam);
int ape_config_audio_para(audio_config_info_t* config_info, int* reset);
int ape_audio_decore_ioctl(ape_priv_t* ape_priv, int cmd, void *param1, void *param2);

void ape_dump_enable(void *file_path);
void ape_playback(FILE *test_file);
void ape_playback_start(void *file_path);
void ape_playback_stop(void);
int ape_get_dbg_on();

#ifdef __cplusplus 
} 
#endif
#endif

