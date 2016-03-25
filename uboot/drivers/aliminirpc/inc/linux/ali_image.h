#ifndef __ALI_IMAGE_H
#define __ALI_IMAGE_H

#include <linux/types.h>
#include <ali_magic.h>

#include "ali_basic.h"
#include "ali_video.h"

typedef unsigned long file_h;

/*
#ifndef BOOL
#define BOOL int
#endif
*/

#define JPGD_GRAYSCALE 0
#define JPGD_YH1V1     1 //4:4:4
#define JPGD_YH2V1     2 //4:2:2
#define JPGD_YH1V2     3 //
#define JPGD_YH2V2     4 //4:2:0
#define JPGD_YH4V1     5 //4:1:1

//commad for decoder: init\release\decode\stop 
#define IMAGEDEC_CMD_INIT       _IOW(ALI_IMAGE_MAGIC, 10, struct image_init_config)
#define IMAGEDEC_CMD_RELEASE    _IO(ALI_IMAGE_MAGIC, 11)
#define IMAGEDEC_CMD_DECODE     _IOW(ALI_IMAGE_MAGIC, 12, struct image_engine_config)
#define IMAGEDEC_CMD_STOP       _IO(ALI_IMAGE_MAGIC, 13)
#define IMAGEDEC_GET_MEM_INFO   _IOR(ALI_IMAGE_MAGIC, 14, struct ali_image_mem_info)
#define IMAGEDEC_CMD_ROTATE     _IOW(ALI_IMAGE_MAGIC, 15, unsigned char)
#define IMAGEDEC_CMD_ZOOM       _IOW(ALI_IMAGE_MAGIC, 16, struct ali_image_pars)
#define IMAGEDEC_CMD_DISPLAY    _IOW(ALI_IMAGE_MAGIC, 17, struct image_display_t)
#define IMAGEDEC_CMD_GET_HWINFO _IOR(ALI_IMAGE_MAGIC, 18, struct image_hw_info)
#define IMAGEDEC_CMD_SET_MADDR  _IOW(ALI_IMAGE_MAGIC, 19, unsigned long)
#define IMAGEDEC_CMD_GET_MAP_INFO  _IOR(ALI_IMAGE_MAGIC, 20, struct ali_image_map_info)
#define IMAGEDEC_CMD_GET_IMG_INFO  _IOWR(ALI_IMAGE_MAGIC, 21, struct ali_image_pars)

#define __MM_FB0_Y_LEN			(1920*1088+1024)//(736*576+512)	//for high definition jpg decode
#define __MM_FB1_Y_LEN			__MM_FB0_Y_LEN
#define __MM_FB2_Y_LEN			__MM_FB0_Y_LEN

#define __MM_FB0_C_LEN			(__MM_FB0_Y_LEN/2)
#define __MM_FB1_C_LEN			__MM_FB0_C_LEN
#define __MM_FB2_C_LEN			__MM_FB0_C_LEN

#define __MM_FB3_Y_LEN			(736*576+1024)
#define __MM_FB3_C_LEN			(__MM_FB3_Y_LEN/2)
#define __MM_FB4_Y_LEN			__MM_FB3_Y_LEN
#define __MM_FB4_C_LEN			__MM_FB3_C_LEN
#define __MM_FB5_Y_LEN          		__MM_FB3_Y_LEN
#define __MM_FB5_C_LEN          		__MM_FB3_C_LEN
#define __MM_FB6_Y_LEN         		__MM_FB3_Y_LEN
#define __MM_FB6_C_LEN          		__MM_FB3_C_LEN

#define __MM_FB_LEN			    		0x19c6200

#define MAX_IMG_ARG_NUM		4

#define MAX_IMG_ARG_SIZE		512

struct ali_image_arg
{
	void *arg;
	int arg_size;
	int out;
};

struct ali_image_pars
{
	int type;
	int API_ID;
	struct ali_image_arg arg[MAX_IMG_ARG_NUM];
	int arg_num;
};

enum IMAGE_DIS_MODE
{
	IMAGEDEC_FULL_SRN = 1,
	IMAGEDEC_REAL_SIZE,
	IMAGEDEC_THUMBNAIL,
	IMAGEDEC_AUTO,
	IMAGEDEC_SIZEDEFINE,
	IMAGEDEC_MULTI_PIC,
	IMAGEDEC_NOT_DIS,
	IMAGEDEC_NOT_DISTHUM,
	RESERVED_MODE,
};
enum IMAGE_ANGLE
{
	ANG_ORI,
	ANG_90_A ,	/*Along the clock with 90*/
	ANG_180,
	ANG_90_C,	/*Counter the clock with 90*/	
};

enum IMAGE_SHOW_MODE
{
	M_NORMAL,
	M_SHUTTERS,
	M_BRUSH,
	M_SLIDE,
	M_RANDOM,
	M_FADE,
};

enum IMAGE_FORMAT
{
	IMG_JPEG = 1,
	IMG_PNG,
	IMG_BMP,
};

typedef struct Imagedec_Init_Config_t
{
	/*dst frame info*/
	uint32 frm_y_addr;
	uint32 frm_y_size;
	uint32 frm_c_addr;
	uint32 frm_c_size;
	uint32 frm2_y_addr;
	uint32 frm2_y_size;
	uint32 frm2_c_addr;
	uint32 frm2_c_size;
	uint32 frm_mb_type;/*reserved for 36XX series with different MB width. 33XX : 16  36XX : 32*/

	/*buf for dec internal usage*/
	uint8 *decoder_buf;
	uint32 decoder_buf_len;

	/*file operation callback function*/
	int (*fread_callback)(file_h fh,uint8 *buf, uint32 size);
	//BOOL (*fseek_callback)(file_h fh,int32 offset, uint32 origin);
	int (*ftell_callback)(file_h fh);
	/*external status info callback function*/
	uint32 (*imagedec_status)(void *value);

	//backup the old frame buffer
	uint32 frm3_y_addr;
	uint32 frm3_y_size;
	uint32 frm3_c_addr;
	uint32 frm3_c_size;

	uint32 frm4_y_addr;
	uint32 frm4_y_size;
	uint32 frm4_c_addr;
	uint32 frm4_c_size;
}Imagedec_Init_Config , *pImagedec_Init_Config;

typedef struct Imagedec_show_shutters_t
{
	uint8 direction; //0 : horizontal 1: vertical
	uint8 type;
	uint16 time; // ms
}Imagedec_show_shutters,*pImagedec_show_shutters;

typedef struct Imagedec_show_brush_t
{
	uint8 direction; //0 : from left to right 1: from top to bottom
	uint8 type;
	uint16 time; // ms
}Imagedec_show_brush,*pImagedec_show_brush;

typedef struct Imagedec_show_slide_t
{
	uint8 direction; //0 : from left to right 1: from top to bottom
	uint8 type;
	uint16 time; // ms	
}Imagedec_show_slide,*pImagedec_show_slide;

typedef struct Imagedec_show_random_t
{
	uint8 type;	// 0: random block operation
	uint8 res;
	uint16 time; // ms
}Imagedec_show_show_random,*pImagedec_show_random;

typedef struct Imagedec_show_fade_t
{
	uint8 type;
	uint8 res;
	uint16 time; // ms
}Imagedec_show_fade,*pImagedec_show_fade;

typedef struct Imagedec_Mode_Par_t
{
	struct Video_Rect src_rect;	/*source rectangule, only used in Real_size mode*/
	struct Video_Rect dis_rect;	/*display rectangule,only used in ThumbNail mode*/
	//BOOL pro_show;	/*whether show the part of the pic when decoding it*/
	//BOOL vp_close_control;	/*whether close vp once*/
	enum IMAGE_SHOW_MODE show_mode;
	uint8 *show_mode_par;
	uint8 need_logo;
	uint32 reserved;
}Imagedec_Mode_Par,*pImagedec_Mode_Par;

struct ali_image_mem_info
{
	void *mem_start;
	unsigned long mem_size;
    	void *priv_mem_start;
    	unsigned long priv_mem_size;
};

struct ali_image_map_info
{
	void *mem_start;
	unsigned long mem_size;
};

struct image_slideshow_effect
{	
	enum IMAGE_SHOW_MODE mode;
	union 
	{
		Imagedec_show_shutters 	shuttles_param;
		Imagedec_show_brush 	brush_param;
		Imagedec_show_slide 	slide_param;
		Imagedec_show_show_random
								random_param;
		Imagedec_show_fade 		fade_param;
	}mode_param;
	
};

struct image_engine_config
{
	//char *file_name;

	unsigned char	decode_mode;		
	unsigned char	show_mode;
	unsigned char	vpo_mode;
	unsigned char	rotate;

	//rect for source
	unsigned short	src_left;
	unsigned short	src_top;
	unsigned short	src_width;
	unsigned short	src_height;

	//rect for display
	unsigned short	dest_left;
	unsigned short	dest_top;
	unsigned short	dest_width;
	unsigned short	dest_height;

	//slide show mode 
	struct image_slideshow_effect *effect;

	enum IMAGE_FORMAT img_fmt;
	//callback function
	//mp_callback_func mp_cb;
};

struct image_init_config
{
	
	// dst frame info
	uint32 frm_y_addr;
	uint32 frm_y_size;
	uint32 frm_c_addr;
	uint32 frm_c_size;
	uint32 frm2_y_addr;
	uint32 frm2_y_size;
	uint32 frm2_c_addr;
	uint32 frm2_c_size;
	uint32 frm_mb_type;
	uint32 frm3_y_addr;
	uint32 frm3_y_size;
	uint32 frm3_c_addr;
	uint32 frm3_c_size;
	uint32 frm4_y_addr;
	uint32 frm4_y_size;
	uint32 frm4_c_addr;
	uint32 frm4_c_size;

	// buf for dec internal usage
	uint8 *decoder_buf;
	uint32 decoder_buf_len;

	enum IMAGE_FORMAT img_fmt;

	int pkt_sbm_idx;
	int info_sbm_idx;
    int info_end_idx;
	//callback function for application
	//mp_callback_func mp_cb;		
};

struct image_display_t 
{
	unsigned char	decode_mode;		
	unsigned char	show_mode;
	unsigned char	vpo_mode;
	unsigned char	rotate;

	//rect for source
	unsigned short	src_left;
	unsigned short	src_top;
	unsigned short	src_width;
	unsigned short	src_height;

	//rect for display
	unsigned short	dest_left;
	unsigned short	dest_top;
	unsigned short	dest_width;
	unsigned short	dest_height;

	struct image_slideshow_effect *effect;

    enum IMAGE_FORMAT img_fmt;

	uint32	y_addr;
	uint32	y_len; 
	uint32 	c_addr;
	uint32	c_len;

	uint32	width;
	uint32	height;

	uint8	sample_format;
};

struct image_hw_info
{
    uint32  y_addr;
    uint32  y_size;
    uint32  c_addr;
    uint32  c_size;

    uint32  width;
    uint32  height;
    uint32  w_stride;
    uint8   sample_format;
};

struct image_info
{
	unsigned long	fsize;
	unsigned long	width;
	unsigned long	height;
	unsigned long	bbp;
    	unsigned long   type;
};

struct info_end
{
	unsigned long magic_num;
	unsigned long flag_end:1;
	unsigned long reserved:31;
};

struct head_data
{
	unsigned long magic_num;     // special symbol
	unsigned long buf_size;          // size of raw data
	unsigned long offset;              // data offset from start address
	unsigned long flag_end:1;      // flag for EOF
	unsigned long reserved:31;    // reserved
};

#endif

