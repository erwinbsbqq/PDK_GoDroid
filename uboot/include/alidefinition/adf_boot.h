
#ifndef __ADF_BOOT__
#define __ADF_BOOT__

/*! @addtogroup ��������
 *  @{
 */

#define MEDIA_BLOCK_MAXCOUNT 0x8
#define MEDIA_PLAYLIST_MAXCOUNT 64

#define TVE_SYSTEM_NUM              20
#define TVE_ADJ_FIELD_NUMBER    100
#define TVE_ADJ_REG_NUM             100
#define TVE_ADV_ADJ_REG_NUM        100
#define BOOT_MAGIC_NUM 0xAA5555AA

/*! @struct ADF_BOOT_HDCP
@brief HDCP��ֵ��Ϣ
*/
typedef struct
{
	unsigned char hdcp[288];//!<HDCP��ֵ
}ADF_BOOT_HDCP;

/*! @struct ADF_BOOT_AVINFO
@brief ����Ƶ��Ϣ
*/
typedef struct
{
	unsigned char tvSystem;
	unsigned char progressive;
	unsigned char tv_ratio;
	unsigned char display_mode;
	
	unsigned char scart_out;
	unsigned char vdac_out[6];
	unsigned char video_format;
	
	unsigned char audio_output;
	unsigned char brightness;
	unsigned char contrast;
	unsigned char saturation;
	
	unsigned char sharpness;
	unsigned char hue;
	unsigned char snd_mute_gpio;
	unsigned char snd_mute_polar;
	unsigned char hdcp_disable;
	unsigned char resv[7];
}ADF_BOOT_AVINFO;

/*! @struct ADF_BOOT_MAC_PHYADDR
@brief Mac�����ַ��Ϣ
*/
typedef struct
{
	unsigned char phyaddr1[8];	
	unsigned char phyaddr2[8];	
	unsigned char phyaddr3[8];	
	unsigned char phyaddr4[8];	
}ADF_BOOT_MAC_PHYADDR;

/*! @struct ADF_BOOT_MEMMAPINFO
@brief �ڴ�ֲ���Ϣ
*/
typedef struct
{   
	unsigned int kernel_start;
	unsigned int kernel_len;

	unsigned int boot_see_start;
	unsigned int boot_see_len;
	
	unsigned int see_start;
	unsigned int see_len;

	unsigned int ae_start;
	unsigned int ae_len;

	unsigned int mcapi_start;
	unsigned int mcapi_len;
	
	unsigned int vbv_start;
	unsigned int vbv_len;

	unsigned int decv_fb_start;//video frame buffer
	unsigned int decv_fb_len;

	unsigned int decv_raw_start;
	unsigned int decv_raw_len;

	unsigned int osd_fb_start;
	unsigned int osd_fb_len;

	unsigned int boot_media_start;
	unsigned int boot_media_len;
	
	unsigned int ramdisk_start;
	unsigned int ramdisk_len;

	unsigned int cmd_queue_buffer_start;
	unsigned int cmd_queue_buffer_len;

	unsigned int vcap_buffer_start;
	unsigned int vcap_buffer_len;

	unsigned int boot_media_mkv_buf_start;
	unsigned int boot_media_mkv_buf_len;
	
	unsigned int reserve[772];
}ADF_BOOT_MEMMAPINFO; // maximun is 800X4 bytes

#define MAX_REGISTER_NUM		64
#define REGISTER_VALID_MAGIC		0x78AC88EF

/*! @struct REGISTER_SETTING
@brief �������õļĴ���ֵ��Ϣ
*/
struct REGISTER_SETTING
{
	unsigned int magic;
	unsigned int addr;
	unsigned int bits_offset;
	unsigned int bits_size;
	unsigned int bits_value;
};

/*! @struct ADF_REGISTERINFO
@brief �������õļĴ���������Ϣ
*/
typedef struct
{	
	unsigned int valid_count;
	struct REGISTER_SETTING unit[MAX_REGISTER_NUM];
}ADF_REGISTERINFO;

struct tve_adjust_element
{
	unsigned int field_index;
	unsigned int field_value;
};

struct tve_adjust_table
{
	unsigned int index;
	struct tve_adjust_element field_info[TVE_ADJ_FIELD_NUMBER];
};

struct sd_tve_adjust_table 
{
	unsigned char type;
	unsigned char sys;
	unsigned int value;
};

/*! @struct ADF_BOOT_TVEINFO
@brief TVE������Ϣ
*/
typedef struct
{
	struct tve_adjust_table tve_adjust_table_info[TVE_SYSTEM_NUM];       //804 * 20
	struct sd_tve_adjust_table sd_tve_adj_table_info[TVE_ADJ_REG_NUM];      //8 * 100
	struct sd_tve_adjust_table sd_tve_adv_adj_table_info[TVE_ADV_ADJ_REG_NUM];    // 8 * 100
	unsigned char reserve[8944];
}ADF_BOOT_TVEINFO; // maximun is 26k = 26 X 1024 =  26624 bytes

enum ADF_BOOT_MEDIA_CMD
{
	BOOT_MEDIA_IDLE = 1,
	BOOT_MEDIA_START,
	BOOT_MEDIA_PLAYING,
	BOOT_MEDIA_FINISHED,
};

enum ADF_BOOT_MEDIA_STATUS
{
	MEDIA_FILE_IDLE = 1,
	MEDIA_FILE_START,
	MEDIA_FILE_PLAYING,
	MEDIA_FILE_FINISHED,
};

/*! @struct ADF_BOOT_MEDIAINFO
@brief �����Ķ�ý����Ϣ
*/
typedef struct
{
	unsigned int play_enable;
	enum ADF_BOOT_MEDIA_CMD start_cmd;
	enum ADF_BOOT_MEDIA_CMD finish_cmd;	
	enum ADF_BOOT_MEDIA_STATUS jpeg_show_status;
	enum ADF_BOOT_MEDIA_STATUS mpeg2_show_status;
	enum ADF_BOOT_MEDIA_STATUS mkv_show_status;
	unsigned int smart_output_enable;
}ADF_BOOT_MEDIAINFO;


/*! @struct ADF_BOOT_GMA_INFO
@brief ������GMA��Ϣ
*/
typedef struct
{
	int gma_enable;
	int gma_layer_id;

	int format;//see adf_gma.h enum GMA_FORMAT
	
	int x, y;
	int w, h;

	unsigned int gma_buffer;
	unsigned int gma_pitch;

	unsigned int pallett[256];
	int full_screen;
}ADF_BOOT_GMA_INFO;

/*! @struct ADF_SEE_HEART_BEAT
@brief ����ʱSEE CPU������Ϣ
*/
typedef struct
{
	int live_flag;
	int live_tick;
}ADF_SEE_HEART_BEAT;

/*! @struct ADF_BOOT_BOARD_INFO
@brief ����������Ϣ
*/
typedef struct
{
	ADF_BOOT_HDCP hdcp;	
	ADF_BOOT_AVINFO avinfo;		
	ADF_BOOT_MEMMAPINFO memmap_info;
	ADF_BOOT_TVEINFO tve_info;
	ADF_REGISTERINFO reg_info;
	ADF_BOOT_MAC_PHYADDR macinfo;
	ADF_BOOT_MEDIAINFO	media_info;
	ADF_BOOT_GMA_INFO gma_info;
  ADF_SEE_HEART_BEAT heart_beat;
}ADF_BOOT_BOARD_INFO; // the maximun size is 128K

#define PRE_DEFINED_ADF_BOOT_START			(0x84000000 - 0x20000)//!< ���������ڴ��ַ

/*! @enum ADF_BOOT_MEDIA_TYPE
@brief ������ý����Ϣ������
*/
enum ADF_BOOT_MEDIA_TYPE
{
	BOOT_MEDIA_TYPE_MIN = 10,
		
	// logo
	BOOT_MEDIA_MPEG2_Logo,
	BOOT_MEDIA_JPEG_Logo,

	// video
	BOOT_MEDIA_MKV_Video = 110,

	BOOT_MEDIA_TYPE_MAX,		
};

#define ADF_BOOT_MEDIA_MAGIC		"adfbootmedia"
#define ADF_BOOT_MEDIA_MAGIC_LEN 	16

/*! @struct ADF_BOOT_MEDIA_BLOCK_HEADER
@brief ������ý����ʾ��Ԫͷ��Ϣ
*/
typedef struct
{
	unsigned int block_offset;//!<ý���ļ���ƫ��
	unsigned int block_len;//!<ý���ļ��ĳ���
	enum ADF_BOOT_MEDIA_TYPE media_type;//!<ý����Ϣ������
	unsigned int play_time; //!<����ʱ��
}ADF_BOOT_MEDIA_BLOCK_HEADER;

/*! @struct ADF_BOOT_MEDIA_BLOCK_HEADER
@brief ������ý��ͷ��Ϣ
*/
typedef struct
{
	unsigned char magic[ADF_BOOT_MEDIA_MAGIC_LEN];//!<Magic��
	unsigned short media_block_count;//!<��ʶ�����Ķ�ý��ĸ���
	unsigned short play_count;//!<ʵ�ʲ��ŵĶ�ý��ĸ���
	unsigned char play_select_index[MEDIA_PLAYLIST_MAXCOUNT];//!<ѡ�񲥷ŵĶ�ý������
	ADF_BOOT_MEDIA_BLOCK_HEADER block_hdr[MEDIA_BLOCK_MAXCOUNT];//!<ÿ����ý�嵥Ԫ��Ϣ
	unsigned char reserve[44];
}ADF_BOOT_MEDIA_HEADER;

/*!
 * @}
 */

#endif

