#ifndef _ALI_GMA_H_
#define _ALI_GMA_H_

#include <rpc_hld/ali_rpc_hld.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_array)      (sizeof(_array)/sizeof(_array[0]))
#endif

#define INLINE			inline

/* Definition for TV System */
//#define NTSC    0
//#define PAL     1

enum GE_CLIP_MODE
{
	GE_CLIP_DISABLE,
	GE_CLIP_INSIDE,
	GE_CLIP_OUTSIDE,
};

enum GE_RGB_ORDER
{
	GE_RGB_ORDER_ARGB, // RGB/ARGB
	GE_RGB_ORDER_ABGR, // BGR/ABGR
	GE_RGB_ORDER_RGBA,
	GE_RGB_ORDER_BGRA,

	GE_RGB_ORDER_AYCbCr = GE_RGB_ORDER_ARGB,
	GE_RGB_ORDER_ACrCbY = GE_RGB_ORDER_ABGR,
	GE_RGB_ORDER_YCbCrA = GE_RGB_ORDER_RGBA,
	GE_RGB_ORDER_CrCbYA = GE_RGB_ORDER_BGRA,
};

#define GE_ALPHA_RANGE_BASE_36F	0x00

/* define alpha range */
enum GE_ALPHA_RANGE
{
	GE_LITTLE_ALPHA, 		// alpha is in 0-127
	GE_LARGE_ALPHA,		// alpha is in 0-255

	GE_ALPHA_RANGE_0_255 = GE_ALPHA_RANGE_BASE_36F,
	GE_ALPHA_RANGE_0_127,
	GE_ALPHA_RANGE_0_15,    // GE not support, only for GMA pallette
};

#if 0
// TV system
enum TVSystem
{
	PAL		= 0	, //	PAL4.43(==PAL_BDGHI)		(Fh=15.625,fv=50)
	NTSC		, //	NTSC3.58					(Fh=15.734,Fv=59.94)
	PAL_M		, //	PAL3.58					(Fh=15.734,Fv=59.94)
	PAL_N		, //	PAL4.43(changed PAL mode)	(Fh=15.625,fv=50)
	PAL_60		, //							(Fh=15.734,Fv=59.94)
	NTSC_443	, //	NTSC4.43					(Fh=15.734,Fv=59.94)
	SECAM		,
	MAC			,
	LINE_720_25,	//added for s3601
	LINE_720_30,	//added for s3601
	LINE_1080_25,	//added for s3601
	LINE_1080_30,	//added for s3601

	LINE_1080_50,	//added for s3602f
	LINE_1080_60,   //added for s3602f
	LINE_1080_24,	//added for s3602f	
	LINE_1152_ASS,  //added for s3602f
	LINE_1080_ASS,  //added for s3602f
	PAL_NC		, //	PAL3.58(changed PAL mode)	(Fh=15.625,fv=50)
};
#endif

enum GMA_PIXEL_FORMAT_36F
{
    GMA36F_PF_RGB888 = 0x00,
    GMA36F_PF_ARGB8888 = 0x01,
    GMA36F_PF_RGB444 = 0x02,
    GMA36F_PF_ARGB4444 = 0x03,
    GMA36F_PF_RGB555 = 0x04,
    GMA36F_PF_ARGB1555 = 0x05,
    GMA36F_PF_RGB565 = 0x06,
    GMA36F_PF_RESERVED0 = 0x07,
    GMA36F_PF_CLUT2 = 0x08,
    GMA36F_PF_RESERVED1 = 0x09,
    GMA36F_PF_CLUT4 = 0x0A,
    GMA36F_PF_ACLUT44 = 0x0B,
    GMA36F_PF_CLUT8 = 0x0C,
    GMA36F_PF_RESERVED2 = 0x0D,
    GMA36F_PF_ACLUT88 = 0x0E,
    GMA36F_PF_RESERVED3 = 0x0F,

    GMA36F_PF_YCbCr444 = 0x10,
    GMA36F_PF_YCbCr422 = 0x11,
    GMA36F_PF_YCbCr420 = 0x12,
    GMA36F_PF_AYCbCr8888 = 0x13,

    GMA36F_PF_MASK_A1 = 0x1C,    // Only for 1bpp MASK
    GMA36F_PF_MASK_A8 = 0x1D,    // Only for 8bpp MASK
};

#define GE_PIXEL_FORMAT_MAX		(0x96)

enum GE_ALPHA_POLARITY
{
	GE_ALPHA_POLARITY_0,    // 0 - transparent, 1 - not transparent
	GE_ALPHA_POLARITY_1,    // 0 - not transparent, 1 - transparent
};

// added for new gma module
// gma hw layer ID
#define GMA_HW_LAYER0       0
#define GMA_HW_LAYER1       1
#define GMA_HW_LAYER2       2

// pallette type
#define	GE_PAL_RGB              0x00
#define	GE_PAL_YCBCR            0x01

// scale mode
#define GE_SCALE_DUPLICATE      0
#define GE_SCALE_FILTER         1

// Begin ge_gma_scale(dev, layer_id, scale_cmd, scale_param)
#define GE_VSCALE_OFF           0x01 // scale_param: enum TVSystem
#define GE_VSCALE_TTX_SUBT      0x02 // scale_param: enum TVSystem

#define GE_H_DUPLICATE_ON_OFF   0x03 // scale_param: TRUE or FALSE
#define GE_SET_SCALE_MODE       0x04 // GE_SCALE_FILTER or GE_SCALE_DUPLICATE

#define GE_SET_SCALE_PARAM      0x05 // Set arbitrary scale param. see struct gma_scale_param_t
#define GE_GET_SCALE_PARAM      0x06 // Get scale param. see struct gma_scale_param_t

/*ioctrl IO cmd list*/
#define OSD_IO_16M_MODE_29E					0x01
#define GE_IO_SCALE_OSD_29E					0x02
#define GE_IO_UPDATE_PALETTE_29E				0x03
#define GE_IO_ANTI_FLICK_29E					0x04
#define GE_IO_GLOBAL_ALPHA_29E				0x05
#define GE_IO_SET_TRANS_COLOR               			0x06
#define GE_IO_SWITCH_OPERATION_MODE			0x07
#define GE_IO_SET_CUR_REGION					0x08
#define GE_IO_SET_REG_PARS						0x09
#define GE_IO_GET_REG_PARS						0x0A
#define GE_IO_DRAW_BITMAP						0x0B
#define GE_IO_DRAW_LINE						0x0C
#define GE_IO_SET_ANTIFLK_PARA				0x0D
#define GE_IO_SET_DISPLAY_ADDR				0x0E    
#define GE_IO_SET_SCALE_MODE					0x0F   
#define GE_IO_SET_LAYER2_GLOBAL_ALPHA		0x10   
#define GE_IO_SCALE_OSD_DEO					0x11  
#define GE_IO_SET_EDGE_ALPHA_CTRL					0x12  


#define GE_IO_CTRL_BASE_GMA     				0x1000 // don't conflict with GE_IO_CTRL cmd

#define GE_IO_UPDATE_PALLETTE					(GE_IO_CTRL_BASE_GMA + 1) // Call it in DE ISR
#define GE_IO_RESPOND_API	    					(GE_IO_CTRL_BASE_GMA + 2)  // Call it in DE ISR
#define GE_IO_ENABLE_ANTIFLICK					(GE_IO_CTRL_BASE_GMA + 3)  // Enable/Disable GMA anti-flicker. io_param is TRUE or FLASE
#define GE_IO_SET_GLOBAL_ALPHA  				(GE_IO_CTRL_BASE_GMA + 4)  // Set GMA layer global alpha. io_param [0x00, 0xff], set 0xff will disable global alpha

#define GE_IO_GET_LAYER_ON_OFF  				(GE_IO_CTRL_BASE_GMA + 5)  // Get GMA layer show or hide(io_param is UINT32 *)

//#define GE_IO_SET_TRANS_COLOR   0x86 // Transparent color index for CLUT, default use 0xff; RGB will use 0 as transparent color

#define GE_IO_SET_AUTO_CLEAR_REGION 			(GE_IO_CTRL_BASE_GMA + 7)    /* Enable/Disable filling transparent color in ge_create_region().
                                              						After ge_open(), default is TRUE. Set it before ge_create_region().*/
#define GE_IO_SET_START_MODE        				(GE_IO_CTRL_BASE_GMA + 8)
#define GE_IO_SET_LAYER_DIS_SEQUENCE			(GE_IO_CTRL_BASE_GMA + 9)
#define GE_IO_SET_LATENCY_CNT					(GE_IO_CTRL_BASE_GMA + 0x0A)
#define GE_IO_SET_SYNC_MODE        				(GE_IO_CTRL_BASE_GMA + 0x0B)   /* enum GE_CMD_SYNC_MODE, default use interrupt sync mode */
#define GE_IO_SET_HW_CMD_BUFFER                 (GE_IO_CTRL_BASE_GMA + 0x0C)  //set com buffer and size
#define GE_IO_GET_DISPLAY_RECT					(GE_IO_CTRL_BASE_GMA + 0x0D)    /* struct ge_rect_t * */
#define GE_IO_SET_DISPLAY_RECT					(GE_IO_CTRL_BASE_GMA + 0x0E)    /* struct ge_rect_t * */
#define GE_IO_SET_ENHANCE_PAR                   (GE_IO_CTRL_BASE_GMA + 0x0F)    /*modified for 3701c*/

//#define GMA_ENHANCE_BRIGHTNESS		0x01
//#define GMA_ENHANCE_CONTRAST			0x02
//#define GMA_ENHANCE_SATURATION		0x03
//#define GMA_ENHANCE_SHARPNESS		0x04
//#define GMA_ENHANCE_HUE				0x05

typedef struct 
{
	INT32	left;		// start_x position related to base address
	INT32	top;		// start_y position related to base address
	INT32	width;		// width of input surface
	INT32	height;		// height of surface to be created    
}gma_rect_t;

typedef const gma_rect_t *pcge_rect_t;

static UINT8 m_gma_expf2inpf_36f[] = {
    GE_GMA_PF_RGB565, GMA36F_PF_RGB565,
    GE_GMA_PF_RGB888, GMA36F_PF_RGB888,
    GE_GMA_PF_RGB555, GMA36F_PF_RGB555,
    GE_GMA_PF_RGB444, GMA36F_PF_RGB444,
    GE_GMA_PF_ARGB565, GMA36F_PF_RGB565,
    GE_GMA_PF_ARGB8888, GMA36F_PF_ARGB8888,
    GE_GMA_PF_ARGB1555, GMA36F_PF_ARGB1555,
    GE_GMA_PF_ARGB4444, GMA36F_PF_ARGB4444,
    GE_GMA_PF_CLUT1, 0xFF,
    GE_GMA_PF_CLUT2, GMA36F_PF_CLUT2,
    GE_GMA_PF_CLUT4, GMA36F_PF_CLUT4,
    GE_GMA_PF_CLUT8, GMA36F_PF_CLUT8,
    GE_GMA_PF_ACLUT88, GMA36F_PF_ACLUT88,
    GE_GMA_PF_YCBCR888, 0xFF,
    GE_GMA_PF_YCBCR422, GMA36F_PF_YCbCr422,
    GE_GMA_PF_YCBCR422MB, 0xFF,
    GE_GMA_PF_YCBCR420MB, 0xFF,
    GE_GMA_PF_AYCBCR8888, GMA36F_PF_AYCbCr8888,
    GE_GMA_PF_A1, GMA36F_PF_MASK_A1,
    GE_GMA_PF_A8, GMA36F_PF_MASK_A8,
    GE_GMA_PF_CK_CLUT2, GMA36F_PF_CLUT2,
    GE_GMA_PF_CK_CLUT4, GMA36F_PF_CLUT4,
    GE_GMA_PF_CK_CLUT8, GMA36F_PF_CLUT8,
    GE_GMA_PF_ABGR1555, GMA36F_PF_ARGB1555,
    GE_GMA_PF_ABGR4444, GMA36F_PF_ARGB4444,
    GE_GMA_PF_BGR565, GMA36F_PF_RGB565,
    GE_GMA_PF_ACLUT44, GMA36F_PF_ACLUT44,
    GE_GMA_PF_YCBCR444, GMA36F_PF_YCbCr444,
    GE_GMA_PF_YCBCR420, GMA36F_PF_YCbCr420
};

#endif                            
