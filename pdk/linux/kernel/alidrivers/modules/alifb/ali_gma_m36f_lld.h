#ifndef __ALI_GMA_LLD_M36F_H__
#define __ALI_GMA_LLD_M36F_H__

#include "ali_gma_m36f_hal.h"
#include "ali_gma.h"
#include <alidefinition/adf_gma.h>

#define OSDDRV_BASIC_DEBUG  0

#ifndef _WINDOWS
#if (OSDDRV_BASIC_DEBUG)
#define GMA_PRINTF			printk
#define GMA_ERR_PRINTF		printk
#else
#define GMA_PRINTF(...)				do{}while(0)
#define GMA_ERR_PRINTF(...)			do{}while(0)
#endif

#else
#if (OSDDRV_BASIC_DEBUG)
#define GMA_PRINTF			printk
#define GMA_ERR_PRINTF		printk
#else
#define GMA_PRINTF
#define GMA_ERR_PRINTF
#endif
#endif

#define ATTR_ALIGN_32           __attribute__((aligned(32)))

#define GMA_SCALE_FACTOR_FLOAT_H 	(float)priv->h_mul/(float)priv->h_div // 2.667
#define GMA_SCALE_FACTOR_FLOAT_V 	(float)priv->v_mul/(float)priv->v_div // 1.875

#define GMA_SCALE_FACTOR_H      priv->h_mul/priv->h_div
#define GMA_SCALE_FACTOR_V      priv->v_mul/priv->v_div

#ifndef GMA_TRANSPARENT_COLOR
#define GMA_TRANSPARENT_COLOR 0xFF
#endif

#ifndef GMA_SCREEN_WIDTH
#define GMA_SCREEN_WIDTH    576
#endif
#ifndef GMA_SCREEN_HEIGHT
#define GMA_SCREEN_HEIGHT   480
#endif

#define GMA_M36F_LAYER_NUM  2

// Scale direction
#define GMA_SCALE_OFF       0
#define GMA_SCALE_UP        1
#define GMA_SCALE_DOWN      2

//#define GMA_SCALE_FILTER    GE_SCALE_FILTER
//#define GMA_SCALE_DUPLICATE GE_SCALE_DUPLICATE

#define GMA_SCALE_EP_OFF    0
#define GMA_SCALE_EP_ON	    1

#define GMA_SCALE_TAP_H2V2  2
#define GMA_SCALE_TAP_H4V3	3 // should be 3 after filling the co-efficients.

#if 0
// User pallette alpha range
enum GE_PALLET_ALPHA_LEVEL
{
    GMA_PALLET_ALPHA_16 = GE_ALPHA_RANGE_0_15,
    GMA_PALLET_ALPHA_128 = GE_ALPHA_RANGE_0_127,
    GMA_PALLET_ALPHA_256 = GE_ALPHA_RANGE_0_255,
};
#endif

#define BLOCK_SHOW  1
#define BLOCK_HIDE  0

#define HEADER_DW_NUM ((8 + 32 + 32 + 8)*2) // add enhance_coef_table = *(header_ptr +576)

#define HEADER_SIZE (sizeof(UINT32) * HEADER_DW_NUM) // IC need 8 QWORD aligned

#define GMA_REGION_MAX          			10
#define MAX_BLOCK_IN_REGION     			1 // for GE DST, one region must be one block
#define GMA_MAX_BLOCK   		(GMA_REGION_MAX * MAX_BLOCK_IN_REGION * GMA_M36F_LAYER_NUM)

#define GMA_HEADER_REALTIME_UPDATE
#define GMA_HEADER_QUEUE_SIZE   2

enum GMA_PIXEL_FORMAT
{
    GMA_PF_RGB888 = 0x00,
    GMA_PF_ARGB8888 = 0x01,
    GMA_PF_RGB444 = 0x02,
    GMA_PF_ARGB4444 = 0x03,
    GMA_PF_RGB555 = 0x04,
    GMA_PF_ARGB1555 = 0x05,
    GMA_PF_RGB565 = 0x06,
    GMA_PF_RESERVED0 = 0x07,
    GMA_PF_CLUT2 = 0x08,
    GMA_PF_RESERVED1 = 0x09,
    GMA_PF_CLUT4 = 0x0A,
    GMA_PF_ACLUT44 = 0x0B,
    GMA_PF_CLUT8 = 0x0C,
    GMA_PF_RESERVED2 = 0x0D,
    GMA_PF_ACLUT88 = 0x0E,
    GMA_PF_RESERVED3 = 0x0F,

    GMA_PF_YCbCr444 = 0x10,
    GMA_PF_YCbCr422 = 0x11,
    GMA_PF_YCbCr420 = 0x12,
    GMA_PF_AYCbCr8888 = 0x13,
};

#define M36F_V      0x01
#define M32C_V      0x02

#define GE_PF_INDEX     0
#define GMA_PF_INDEX    1
#define BPP_INDEX       2
#define IC_INDEX        3
#define IC_INDEX1       4

typedef struct _ge_to_gma_mode_t
{
    UINT8 ge_mode;
    UINT8 gma_mode;
    UINT8 byte_per_pixel;
    UINT8 ic_support_flag_layer0;
    UINT8 ic_support_flag_layer1;
}
ge_to_gma_mode_t;
enum GMA_REGION_PARAM_INDEX
{
    GMA_REGION_MODE,
    GMA_REGION_GALPHA_ENABLE,
    GMA_REGION_GALPHA,
    GMA_REGION_PAL_SEL,
    GMA_REGION_FACTOR,
};

#ifndef offsetof
#define offsetof(structure, member)    ((UINT32)&((structure *)0)->member)
#endif

struct _gma_region_t
{
    gma_rect_t tRegionRect;
    alifb_gma_region_t region_info;
    UINT8 bValid;
    UINT8 uStatus;              /* Region display status */
    UINT8 bUseExternalMem;
    UINT8 bUseMemSlice;
    struct gma_block_t *pBlock[MAX_BLOCK_IN_REGION];
};

typedef struct _gma_region_t gma_region_t;

struct gma_block_t
{
    struct gma_block_t *next;   /* Linkage */
    struct gma_block_t *prev;   /* Linkage */

    UINT8 uStatus;              /* Block display status */

    gma_rect_t tRect;

    UINT32 puHeadBuf[GMA_HEADER_QUEUE_SIZE];
    UINT8 *puBuf;
    UINT32 uSize;
    gma_head_m36f_t block_head;
};

typedef struct gma_block_t gma_block_t;

struct gma_m36f_private
{
    UINT8 uOSDLayer;
    UINT8 bOnOff;               /* Software OSD on/off state */

    alifb_gma_region_t region_info;
    gma_region_t tRegion[GMA_REGION_MAX];

    gma_block_t tBlockHead;
    gma_block_t *first_block;   // first visable block in this layer
    UINT8 uBlockInitialized;
    UINT8 first_block_changed;  // Need to set the first block header to HW
    UINT8 new_header_id;        // The block header ID which can be modified
    UINT8 header_need_sync;     // Need to sync the HW block header to SW block header

    // new added for HD scale
    UINT8 scale_dir;            // scale direction
    UINT8 scale_mode;           // 0 - filter mode, better quality; 1 - duplicate mode, for debug
    UINT8 ep_on;

    UINT16 h_mul;
    UINT16 h_div;
    UINT16 v_mul;
    UINT16 v_div;

    UINT16 h_scale_param;       // For block head, calc according to h_mul/h_div
    UINT16 v_scale_param;       // For block head, calc according to v_mul/v_div

    int filter_select; // 0 : bilinear filter; 1 : V 3-tap interpolation fiter, H 4-tap interpolation filter
	short h_coeff_table[65];
	short v_coeff_table[65];

	/* enhance parametesrs */
	INT32 enhance_coef_table[13];
	int brightness_grade;
	int contrast_grade;
	int saturation_grade;
	int hue_grade;
	int sharpness_grade;

    enum TVSystem tv_sys;       // enum TVSystem
    UINT16 screen_width;

    enum GE_CLIP_MODE eClipMode;
    gma_rect_t Clip_Rect;

    UINT8 *puPallette;
    UINT8 *puPalBase;
    UINT8 *puPalletteHW;
    UINT32 trans_color;
    UINT16 uPalletteColor;
    UINT8 bPalletteUpdated;
    UINT8 uPalFormatHW;         /* S3602F - RGB, S3202C - YCbCr */
    UINT8 uPalFormatSW;         /* SW pal RGB or YUV */
    UINT8 uPalRGBOrder;         /* SW pal RGB order */
    UINT8 uPalAlphaRange;       /* SW pal alpha range */

    UINT8 auto_clear_region;    /* 1 - fill transparent color in osddrv_create_region() */
    UINT8 region_id;            /* for set region pos */

    //for pallete modify
    UINT8 color_idx;            /* for pallete modify */
    UINT32 color;               /* for pallete modify */
    volatile UINT8 bPalMdfUpdated; /* for pallete modify */

    volatile UINT8 bSetRegionPos; /* for set region pos */
    gma_rect_t reg_rect_new;     /* for set region pos */

    volatile UINT8 bMoveRegion; /* for IO move region with external buffer */
    alifb_gma_region_t region_param; /* for IO move region with external buffer */

    volatile UINT8 bSetDisplayRect; /* for set display rect */
    gma_rect_t display_rect_new;     /* for set display rect */ 

    // GE cmd list
    //ge_cmd_list_hdl ge_cmd_list;

    gma_layer_config_t layer_config;
};

#define m_config priv->layer_config


void gma_m36f_attach (UINT32 layer_id, const gma_layer_config_t * attach_config);
RET_CODE gma_m36f_detach (UINT32 layer_id);
RET_CODE gma_m36f_open (UINT32 layer_id);
RET_CODE gma_m36f_close (UINT32 layer_id);
RET_CODE gma_m36f_ioctl (UINT32 layer_id, UINT32 dwCmd, UINT32 dwParam);
RET_CODE gma_m36f_show_onoff (UINT32 layer_id, BOOL uOnOff);
RET_CODE gma_m36f_set_pallette (UINT32 layer_id, const UINT8 * pal, UINT16 color_num, const gma_pal_attr_t * pattr);
RET_CODE gma_m36f_get_pallette (UINT32 layer_id, UINT8 * pal, UINT16 color_num, const gma_pal_attr_t * pattr);
RET_CODE gma_m36f_modify_pallette (UINT32 layer_id, UINT8 uIndex, UINT8 uY, UINT8 uCb, UINT8 uCr, UINT8 uK, const gma_pal_attr_t * pattr);

RET_CODE gma_m36f_create_region (UINT32 layer_id, UINT32 region_id, const alifb_gma_region_t * ptPara);
RET_CODE gma_m36f_delete_region (UINT32 layer_id, UINT32 region_id);
RET_CODE gma_m36f_move_region (UINT32 layer_id, UINT32 region_id, pcgma_region_t pregion_param);

RET_CODE gma_m36f_set_region_pos (UINT32 layer_id, UINT32 region_id, gma_rect_t * rect);
RET_CODE gma_m36f_get_region_info (UINT32 layer_id, UINT32 region_id, alifb_gma_region_t * ptPara);
RET_CODE gma_m36f_set_region_info (UINT32 layer_id, UINT32 region_id, alifb_gma_region_t * ptPara);
RET_CODE gma_m36f_region_showonoff (UINT32 layer_id, UINT32 region_id, BOOL bOn);

RET_CODE gma_m36f_scale (UINT32 layer_id, UINT32 uScaleCmd, UINT32 uScaleParam);

RET_CODE gma_m36f_set_clip_mode (UINT32 layer_id, enum GE_CLIP_MODE clip_mode);
RET_CODE gma_m36f_set_clip_rect (UINT32 layer_id, UINT32 clip_x, UINT32 clip_y, UINT32 clip_w, UINT32 clip_h);
RET_CODE gma_m36f_clear_clip (UINT32 layer_id);
//RET_CODE gma_m36f_set_region_to_ge (UINT32 layer_id, UINT32 region_id, ge_cmd_list_hdl cmd_list);

UINT32 ge_m36f_get_hw_pf (enum GMA_PIXEL_FORMAT pf);
enum GMA_PIXEL_FORMAT ge_m36f_get_sw_pf (UINT32 format);
extern UINT32 sys_ic_get_chip_id(void);

// scaler coeff generated by vpo driver
BOOL vpo_hal_generate_scale_coeff(
    short *pcoeff_table, UINT32 input_sample, UINT32 output_sample,
    UINT8 tap, UINT8 group_num, BOOL bDublicateCoeff
    );

#endif /* __GMA_LLD_M36F_H__ */
