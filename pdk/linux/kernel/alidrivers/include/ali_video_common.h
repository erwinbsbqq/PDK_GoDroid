/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2009 Copyright (C)
 *  (C)
 *  File: ali_video.h
 *  (I)
 *  Description: ali video header
 *  (S)
 *  History:(M)
 *          Date                    Author          Comment
 *          ====                    ======      =======
 * 0.       2009.12.22              Sam     Create
 ****************************************************************************/

#ifndef __ALI_VIDEO_COMMON_H
#define __ALI_VIDEO_COMMON_H

/*! @addtogroup Devicedriver
 *  @{
 */
 
/*! @addtogroup ALiVideo 
 *  @{
*/

#include "ali_basic_common.h"
#include "alidefinition/adf_basic.h"
#include "alidefinition/adf_media.h"
#include <alidefinition/adf_vpo.h>

#if 0
/* ali gma move from see to kernel */
#ifndef CONFIG_RPC_HLD_GMA
#define KERNEL_GMA_TEST
#endif
#endif

/* video output dac information */
#define MAX_VIDEO_DAC_TYPENUM   23
#define VIDEO_DAC0              0x01
#define VIDEO_DAC1              0x02
#define VIDEO_DAC2              0x04
#define VIDEO_DAC3              0x08
#define VIDEO_DAC4              0x10
#define VIDEO_DAC5              0x20


/*! @enum Video_DacType
@brief DAC type (Internal use)
*/
enum Video_DacType
{
    Video_CVBS_1 = 0,
    Video_CVBS_2,
    Video_CVBS_3,
    Video_CVBS_4,
    Video_CVBS_5,
    Video_CVBS_6,
    Video_SVIDEO_1,
    Video_SVIDEO_2,
    Video_SVIDEO_3,
    Video_YUV_1,
    Video_YUV_2,
    Video_RGB_1,
    Video_RGB_2,
    Video_SVIDEO_Y_1,
    Video_SECAM_CVBS1,
    Video_SECAM_CVBS2,
    Video_SECAM_CVBS3,
    Video_SECAM_CVBS4,
    Video_SECAM_CVBS5,
    Video_SECAM_CVBS6,
    Video_SECAM_SVIDEO1,
    Video_SECAM_SVIDEO2,
    Video_SECAM_SVIDEO3,
};

/*! @enum Video_VgaMode
@brief VGA mode (Internal use)
*/
enum Video_VgaMode
{
    Video_VGA_NOT_USE = 0,
    Video_VGA_640_480,
    Video_VGA_800_600
};

/*! @struct Video_DacIndex
@brief DAC index (Internal use)
*/
struct Video_DacIndex
{
    uint8 uDacFirst;     // For all (CVBS, YC_Y,YUV_Y,RGB_R)
    uint8 uDacSecond;    // For SVideo & YUV & RGB (YC_C ,YUV_U,RGB_G)
    uint8 uDacThird;     // For YUV & RGB (YUV_V,RGB_B)
};

/*! @struct Video_DacInfo
@brief DAC information (Internal use)
*/
struct Video_DacInfo
{
    int enable;
    enum Video_DacType type;
    enum Video_VgaMode mode;
    struct Video_DacIndex index;
    int progressive;
};

/*! @enum Video_TVSystem
@brief tv system (Internal use)
*/
enum Video_TVSystem
{
    Video_PAL = 0,       // PAL4.43(==PAL_BDGHI)        (Fh=15.625,fv=50)
    Video_NTSC,          // NTSC3.58                    (Fh=15.734,Fv=59.94)
    Video_PAL_M,         // PAL3.58                     (Fh=15.734,Fv=59.94)
    Video_PAL_N,         // PAL4.43(changed PAL mode)   (Fh=15.625,fv=50)
    Video_PAL_60,        //                             (Fh=15.734,Fv=59.94)
    Video_NTSC_443,      // NTSC4.43                    (Fh=15.734,Fv=59.94)
    Video_SECAM,
    Video_MAC,
    Video_LINE_720_25,   //Added for s3601
    Video_LINE_720_30,   //Added for s3601
    Video_LINE_1080_25,  //Added for s3601
    Video_LINE_1080_30,  //Added for s3601_
    Video_LINE_1080_50,  //Added for s3602f
    Video_LINE_1080_60,  //Added for s3602f
    Video_LINE_1080_24,  //Added for s3602f
    Video_LINE_1152_ASS, //Added for s3602f
    Video_LINE_1080_ASS, //Added for s3602f
    Video_PAL_NC,


    Video_LINE_800_600_VESA,
    Video_LINE_1024_768_VESA,
    Video_LINE_1360_768_VESA,
};


/*! @enum Video_TVAspect
@brief TV aspect ratio (Internal use)
*/
enum Video_TVAspect
{
    Video_TV_4_3 = 0,
    Video_TV_16_9,
    Video_TV_AUTO   //060517 yuchun for GMI Aspect Auto
};

/*! @enum Video_DisplayMode
@brief TV display mode (Internal use)
*/
enum Video_DisplayMode
{
    Video_PANSCAN = 0,
    Video_PANSCAN_NOLINEAR, //Non-linear pan&scan
    Video_LETTERBOX,
    Video_TWOSPEED, //Added by t2
    Video_PILLBOX,
    Video_VERTICALCUT,
    Video_NORMAL_SCALE,
    Video_LETTERBOX149,
    Video_DONT_CARE,
    Video_AFDZOOM,
    //BOTH
};

#define VIDEO_PIC_WIDTH  720
#define VIDEO_PIC_HEIGHT 2880   //!>2880 is the lease common multiple of screen height of Pal and ntsc
#define VIDEO_SCR_WIDTH  720
#define VIDEO_SCR_HEIGHT 2880

/*! @struct Video_Rect
@brief Displaying coordiante and resolution
*/
struct Video_Rect
{
    int32 x;    //!<Horizontal start point
    int32 y;    //!<Vertical start point
    int32 w;    //!<Horizontal size
    int32 h;    //!<Vertical size
};

/*! @struct Video_Pos
@brief Displaying coordiante and resolution
*/
struct Video_Pos
{
    int32 x;    //!<Horizontal start point
    int32 y;    //!<Vertical start point
    int32 w;    //!<Horizontal size
    int32 h;    //!<Vertical size
};

/*! @struct Video_YCbCrColor
@brief Parameter of Video color space
*/
struct  Video_YCbCrColor
{
    uint8 Y;
    uint8 Cb;
    uint8 Cr;
};

enum Video_PicFmt
{
    //YCbCr Format
    Video_PicFmt_YCBCR_411,
    Video_PicFmt_YCBCR_420,
    Video_PicFmt_YCBCR_422,
    Video_PicFmt_YCBCR_444,

    //RGB format
    Video_PicFmt_RGB_MODE1,     //rgb (16-235)
    Video_PicFmt_RGB_MODE2,     //rgb (0-255)
};

#define DIS_FORMAT_CLUT2                        0x1    //!<Not used temporarily
#define DIS_FORMAT_CLUT4                        0x2    //!<Not used temporarily
#define DIS_FORMAT_ACLUT88                      0x4    //!<Not used temporarily
#define DIS_FORMAT_ARGB4444                     0x6    //!<Not used temporarily
#define DIS_FORMAT_RGB444                       0x8    //!<Not used temporarily
#define DIS_FORMAT_RGB555                       0xa    //!<Not used temporarily

#define DIS_FORMAT_CLUT8                        0      //!Input data format CLUT8

#define DIS_FORMAT_MC420                        10     //!<Not used temporarily
#define DIS_FORMAT_MC422                        (DIS_FORMAT_MC420 + 1)    //!<Not used temporarily
#define DIS_FORMAT_MC444                        (DIS_FORMAT_MC420 + 2)    //!<Not used temporarily
#define DIS_FORMAT_AYCBCR8888                   (DIS_FORMAT_MC444 + 3)    //!<Not used temporarily

#define DIS_FORMAT_ARGB1555                     20     //!Input data format ARGB1555
#define DIS_FORMAT_RGB565                       (DIS_FORMAT_ARGB1555 + 1) //!Input data format RGB565

#define DIS_FORMAT_ARGB8888                     40     //!Input data format ARGB8888
#define DIS_FORMAT_RGB888                       41     //!Input data format RGB888

#define VIDEO_MAX_GMA_REGION_NUM                6      //!<Internal use
#define VIDEO_MAX_GMA_WIDTH                     4095   //!<Internal use
#define VIDEO_MAX_GMA_HEIGHT                    4095   //!<Internal use

/*! @enum ALIFB_DE_LAYER
@brief Video/Graphic layers (Internal use)
*/
enum ALIFB_DE_LAYER
{
    DE_LAYER0,    //!<Video main picture
    DE_LAYER1,    //!<Not used temporarily
    DE_LAYER2,    //!<GMA1
    DE_LAYER3,    //!<GMA2
    DE_LAYER4,    //!<Not used temporarily
    DE_LAYER5,    //!<Not used temporarily
};

/*! @enum ALIFB_DE_LAYER
@brief Video/Graphic color space (Internal use)
*/
enum ALIFB_COLOR_TYPE
{
    ALIFB_COLOR_YUV,    //!<YUV color space
    ALIFB_COLOR_RGB,    //!<RGB color space
};

#define FBIO_GET_FBINFO                         0x460001    //!<Get fb information defined in struct fb_info
#define FBIO_SET_OUTPUT_FRAME_PATH              0x460002    //!<No longer use
#define FBIO_OUTPUT_FRAME                       0x460003    //!<No longer use
#define FBIO_CHECK_FRAME_FREE                   0x460004    //!<No longer use
#define FBIO_GET_FBINFO_DATA                    0x460005    //!<Get fb information defined in struct alifbio_fbinfo_data_pars
#define FBIO_SET_COLOR_FMT                      0x460009    //!<No longer use
#define FBIO_SET_TVSYS                          0x460010    //!<Internal use
#define FBIO_GET_TVSYS                          0x460011    //!<Internal use
#define FBIO_WIN_ZOOM                           0x460012    //!<Internal use
#define FBIO_SET_ASPECT_MODE                    0x460013    //!<Internal use
#define FBIO_GET_ASPECT_MODE                    0x460014    //!<Internal use
#define FBIO_UNREG_DAC                          0x460015    //!<Internal use
#define FBIO_REG_DAC                            0x460016    //!<Internal use
#define FBIO_WIN_ONOFF                          0x460017    //!<Internal use
#define FBIO_SET_PALLETTE                       0x460018    //!<Set palette for clut input format
#define FBIO_GET_PALLETTE                       0x460019    //!<Get palette for clut input format
#define FBIO_DELETE_GMA_REGION                  0x460020    //!<Delete gma region
#define FBIO_CREATE_GMA_REGION                  0x460021    //!<Create gma region
#define FBIO_SET_GMA_SCALE_INFO                 0x460022    //!<Scale gma region
#define FBIO_GET_GMA_REG_INFO                   0x460023    //!<No longer use
#define FBIO_FLUSH_GMA_RECT                     0x460024    //!<No longer use
#define FBIO_MOVE_REGION                        0x460025    //!<Move gma region
#define FBIO_GET_WINONOFF_STATE                 0x460028    //!<Internal use
#define FBIO_SET_BG_COLOR                       0x460030    //!<Internal use
#define FBIO_REG_HDMI_CALLBACK_ROUTINE          0x460031    //!<Internal use
#define FBIO_SET_HDMI_PIC_FRMT                  0x460032    //!<Internal use
#define FBIO_FILL_COLOR                         0x460033    //!<Internal use
#define FBIO_SET_DE_LAYER                       0x460040    //!<Internal use
#define FBIO_RPC_OPERATION                      0x460041    //!<Internal use
#define FBIO_SET_GLOBAL_ALPHA                   0x460042    //!<Set global alpha
#define FBIO_GMA_SCALE                          0x460043    //!<No longer use
#define FBIO_DISABLE_GMA_ENHANCE                0x460044    //!<No longer use
#define FBIO_SET_REGION_SIDE_BY                 0x460045    //!<No longer use
#define FBIO_VIDEO_ENHANCE                      0x460046    //!<Set enhance function
#define FBIO_SET_MIX_ORDER                      0x46004f    //!<Not used temporarily

#define FBIO_SET_GE_CMDQ_BUF                    0x460100    //!<No longer use
#define FBIO_GET_UI_RSC_MAP                     0x460101    //!<Get infomation of memory allocated to fb
#define FBIO_GET_DISPLAY_RECT                   0X460102    //!<No longer use
#define FBIO_SET_DISPLAY_RECT                   0X460103    //!<No longer use
#define FBIO_SET_UI_CACHE_FLUSH                 0x460104    //!<No longer use
#define FBIO_GET_GE_CMDQ_BUF_ADDR_SIZE          0x460105    //!<No longer use
#define FBIO_GE_SYNC_TIMEOUT                    0x460106    //!<No longer use
#define FBIO_SET_GMA_ANTIFLICK                  0x460107    //!<Not used temporarily
#define FBIO_SET_GMA_DBG_FLAG                   0x460108    //!<enalbe/disable gma debug info

#define FBIO_REGISTER_ISR                       0x460200    //!<No longer use
#define FBIO_UNREGISTER_ISR                     0x460201    //!<No longer use
#define FBIO_FLAG_WAIT                          0x460202    //!<No longer use

#define VPO_SET_WIN_ONOFF                       0x480001    //!<Open/close main picture layer
#define VPO_SET_WIN_ONOFF_EX                    0x480002    //!<Not used temporarily
#define VPO_WIN_ZOOM                            0x480003    //!<Zoom main picture display rect
#define VPO_WIN_ZOOM_EX                         0x480004    //!<Not used temporarily
#define VPO_SET_TVSYS                           0x480005    //!<Set TV system, only support interlaced TV system
#define VPO_SET_TVSYS_EX                        0x480006    //!<Set TV system
#define VPO_SET_ASPECT_MODE                     0x480007    //!<Set TV aspect ratio
#define VPO_CONFIG_SOURCE_WINDOW                0x480008    //!<Config soure window
#define VPO_SET_WIN_MODE                        0x480009    //!<Set vpo working mode
#define VPO_SET_BG_COLOR                        0x480010    //!<Set backgroud color space
#define VPO_REG_DAC                             0x480011    //!<Registering DAC
#define VPO_SET_PARAMETER                       0x480012    //!<No longer use
#define VPO_VIDEO_ENHANCE                       0x480013    //!<Set video enhance param
#define VPO_SET_CGMS_INFO                       0x480014    //!<Set CGMS information
#define VPO_AFD_CONFIG                          0x480015    //!<Config sw/hw AFD param
#define VPO_GET_CURRENT_DISPLAY_INFO            0x480016    //!<Get current display picture information
#define VPO_BACKUP_CURRENT_PICTURE              0x480017    //!<Backup cureent display picture
#define VPO_FREE_BACKUP_PICTURE                 0x480018    //!<Not used temporarily
#define VPO_SET_OSD_SHOW_TIME                   0x480019    //!<Set osd show time
#define VPO_GET_MP_SCREEN_RECT                  0x480020    //!<Get main picture layer screen rect
#define VPO_GET_MP_INFO                         0x480021    //!<Get main picture layer information
#define VPO_GET_REAL_DISPLAY_MODE               0x480022    //!<No longer use
#define VPO_GET_TV_ASPECT                       0x480023    //!<Get TV aspect ratio
#define VPO_GET_SRC_ASPECT                      0x480024    //!<Get source aspect ratio
#define VPO_GET_DISPLAY_MODE                    0x480025    //!<Get TV display mode
#define VPO_GET_OSD0_SHOW_TIME                  0x480026    //!<Get OSD1 show time
#define VPO_GET_OSD1_SHOW_TIME                  0x480027    //!<Get OSD2 show time
#define VPO_SET_VBI_OUT                         0x480028    //!<Set vbi output callback
#define VPO_WRITE_WSS                           0x480029    //!<Internal use, write wss data, 
#define VPO_UNREG_DAC                           0x480030    //!<Unregistering DAC
#define VPO_MHEG_SCENE_AR                       0x480031    //!<No longer use
#define VPO_MHEG_IFRAME_NOTIFY                  0x480032    //!<No longer use
#define VPO_DISAUTO_WIN_ONOFF                   0x480033    //!<Disable DE driver auto open main picture layer when first picture show.
#define VPO_ENABLE_VBI                          0x480034    //!<Enable vbi output
#define VPO_PLAYMODE_CHANGE                     0x480035    //!<Notify play mode change 
#define VPO_DIT_CHANGE                          0x480036    //!<No longer use
#define VPO_SWAFD_ENABLE                        0x480037    //!<Enable software AFD
#define VPO_704_OUTPUT                          0x480038    //!<Internal use
#define VPO_SET_PREVIEW_MODE                    0x480039    //!<Notify dislay in preview mode 
#define VPO_HDMI_OUT_PIC_FMT                    0x480040    //!<Get HDMI output picture format
#define VPO_ALWAYS_OPEN_CGMS_INFO               0x480041    //!<Always enalbe CGMS
#define VPO_SET_LAYER_ORDER                     0x480042    //!<Set display layer order
#define VPO_TVESDHD_SOURCE_SEL                  0x480043    //!<HD tvencoder connect DEN or DEO 
#define VPO_SD_CC_ENABLE                        0x480044    //!<Enalbe SD tvencoder closecaption function
#define VPO_SET_PREVIEW_SAR_MODE                0x480045    //!<No longer use
#define VPO_SET_FULL_SCREEN_CVBS                0x480046    //!<No longer use
#define VPO_GET_OUT_MODE                        0x480047    //!<Get cureent TV system

#define FB_SET_SCALE_PARAM                      0x05        //!<No longer use
#define FB_GET_SCALE_PARAM                      0x06        //!<No longer use

/*! @struct alifbio_cache_flush
@brief Parameter of FBIO_SET_UI_CACHE_FLUSH (No longer use)
*/
struct alifbio_cache_flush
{
    unsigned long mem_start;
    int mem_size;
};

/*! @struct alifbio_cmd_buf_addr_size
@brief Parameter of FBIO_GET_GE_CMDQ_BUF_ADDR_SIZE (No longer use)
*/
struct alifbio_cmd_buf_addr_size
{
    unsigned addr;
    unsigned size;
};

/*! @enum ALIFB_OUTPUT_FRAME_PATH
@brief No longer use
*/
enum ALIFB_OUTPUT_FRAME_PATH
{
    FRAME_FROM_KERNEL,
    FRAME_FROM_USER,
};

/*! @struct alifbio_fbinfo_data_pars
@brief Parameter of FBIO_GET_FBINFO_DATA
*/
struct alifbio_fbinfo_data_pars
{
    unsigned long mem_start;    //!<Memory start address
    int mem_size;               //!<Memory size

    int xres_virtual;           //!<Virtual horizontal resolution
    int yres_virtual;           //!<Virtual vertical resolution
    int line_length;            //!<Length of a line in bytes
};

/*! @struct alifbio_tvsys_pars
@brief Parameter of FBIO_SET_TVSYS (Internal use)
*/
struct alifbio_tvsys_pars
{
    int progressive;
    enum Video_TVSystem tvsys;
};

/*! @struct alifbio_zoom_pars
@brief Parameter of FBIO_WIN_ZOOM (Internal use)
*/
struct alifbio_zoom_pars
{
    int user_screen_xres;
    int user_screen_yres;
    struct Video_Rect src_rect;
    struct Video_Rect dst_rect;
};

/*! @struct alifbio_aspect_pars
@brief Parameter of FBIO_ASPECT_MODE (Internal use)
*/
struct alifbio_aspect_pars
{
    enum Video_TVAspect aspect;
    enum Video_DisplayMode display_mode;
};

/*! @enum ALIFB_PLT_TYPE
@brief Palette color space
*/
enum ALIFB_PLT_TYPE
{
    PLT_RGB, //!<RGB type: B[0-7] G[8-15] R[16-23] A[24-31]
    PLT_YUV, //!<YUV type: V[0-7] U[8-15] Y[16-23] A[24-31]
};

/*! @enum ALIFB_PLT_ALPHA_LEVEL
@brief Palette type
*/
enum ALIFB_PLT_ALPHA_LEVEL
{
    PLT_ALPHA_16,   //!<16 color palette
    PLT_ALPHA_128,  //!<128 color palette
    PLT_ALPHA_256,  //!<256 color palette
};

/*! @struct alifbio_plt_pars
@brief Parameter of FBIO_SET_PALLETTE
*/
struct alifbio_plt_pars
{
    enum ALIFB_PLT_TYPE type;   //!<Palette color space
    enum ALIFB_PLT_ALPHA_LEVEL level;    //!<Palette type

    void *pallette_buf;         //!<Palette buffer
    int color_num;              //!<Palette color number

    unsigned char rgb_order;    //!<Not used temporarily
    unsigned char alpha_range;  //!<Not used temporarily
    unsigned char alpha_pol;    //!<Not used temporarily
};

/*! @struct alifbio_reg_pars
@brief Parameter of FBIO_CREATE_GMA_REGION
*/
struct alifbio_reg_pars
{
    int index;                  //!<Region ID
    struct Video_Rect rect;     //!<Region displaying coordinate and resolution
    uint32 dis_format;          //!<Region input color format

    void *mem_start;            //!<Region memory start address
    int mem_size;               //!<Region memory size
    int pitch;                  //!<Length of a line in bytes
};

/*! @enum ALIFB_GMA_SCALE_MODE
@brief gma scale mode
*/
enum ALIFB_GMA_SCALE_MODE
{
    GMA_RSZ_DISABLE = 0,
    GMA_RSZ_DIRECT_RESIZE = 4,  //!<Copy pixels directly
    GMA_RSZ_ALPHA_ONLY = 5,     //!<Filter active only on alpha channel
    GMA_RSZ_COLOR_ONLY = 6,     //!<Filter active only on color channel
    GMA_RSZ_ALPHA_COLOR = 7     //!<Filter active on alpha and color channel
};

/*! @struct alifbio_gma_scale_info_pars
@brief param of FBIO_SET_GMA_SCALE_INFO
*/
struct alifbio_gma_scale_info_pars
{
    enum ALIFB_GMA_SCALE_MODE scale_mode;   //!<Scale mode
    int h_dst;                  //!<Horizontal size of destination
    int h_src;                  //!<Horizontal size of source
    int v_dst;                  //!<Vertical size of destination
    int v_src;                  //!<Vertical size of source
    int tv_sys;                 //!<Not used temporarily
    unsigned long uScaleCmd;    //!<Not used temporarily
    unsigned long uScaleParam;  //!<Not used temporarily
};

#if 1
typedef enum
{
    //SD
    TV_MODE_FB_AUTO = 0,    // Switch by source
    TV_MODE_FB_PAL,
    TV_MODE_FB_PAL_M,       // PAL3.58
    TV_MODE_FB_PAL_N,
    TV_MODE_FB_NTSC358,     // NTSC3.58
    TV_MODE_FB_NTSC443,
    TV_MODE_FB_SECAM,
    //HD
    TV_MODE_FB_576P,
    TV_MODE_FB_480P,
    TV_MODE_FB_720P_50,
    TV_MODE_FB_720P_60,
    TV_MODE_FB_1080I_25,
    TV_MODE_FB_1080I_30,
    TV_MODE_FB_1080P_50,
    TV_MODE_FB_1080P_60,
    TV_MODE_FB_1080P_25,
    TV_MODE_FB_1080P_30,
    TV_MODE_FB_1080P_24,
}FB_TV_OUT_MODE;

enum TVSystem_FB
{
    PAL_FB = 0,         //PAL4.43(==PAL_BDGHI)        (Fh=15.625,fv=50)
    NTSC_FB,            //NTSC3.58                    (Fh=15.734,Fv=59.94)
    PAL_M_FB,           //PAL3.58                     (Fh=15.734,Fv=59.94)
    PAL_N_FB,           //PAL4.43(changed PAL mode)   (Fh=15.625,fv=50)
    PAL_60_FB,          //                            (Fh=15.734,Fv=59.94)
    NTSC_443_FB,        //NTSC4.43                    (Fh=15.734,Fv=59.94)
    SECAM_FB,
    MAC_FB,
    LINE_720_25_FB,     //Added for s3601
    LINE_720_30_FB,     //Added for s3601
    LINE_1080_25_FB,    //Added for s3601
    LINE_1080_30_FB,    //Added for s3601

    LINE_1080_50_FB,    //Added for s3602f
    LINE_1080_60_FB,    //Added for s3602f
    LINE_1080_24_FB,    //Added for s3602f
    LINE_1152_ASS_FB,   //Added for s3602f
    LINE_1080_ASS_FB,   //Added for s3602f
    PAL_NC_FB,
};
#endif

/*! @struct alifbio_gma_scale_pars
@brief Parameter of FBIO_GMA_SCALE (No longer use)
*/
struct alifbio_gma_scale_pars
{
    FB_TV_OUT_MODE tv_mode;
    int h_dst;
    int h_src;
    int v_dst;
    int v_src;
};

/*! @struct alifbio_flush_GMA_rect_pars
@brief Parameter of FBIO_FLUSH_GMA_RECT (No longer use)
*/
struct alifbio_flush_GMA_rect_pars
{
    int region_id;
    void *in_start;
    int in_pitch;
    struct Video_Rect rect;
};

/*! @struct alifbio_move_region_pars
@brief Parameter of FBIO_MOVE_REGION
*/
struct alifbio_move_region_pars
{
    int region_id;           //!<Region id
    struct Video_Pos pos;    //!<Coordinate of the region
};

/*! @struct alifbio_fill_color_pars
@brief Parameter of FBIO_FILL_COLOR (No longer use)
*/
struct alifbio_fill_color_pars
{
    enum ALIFB_COLOR_TYPE type;
    uint32 color;
};

/*! @struct alifbio_cmdq_buf
@brief Parameter of FBIO_SET_GE_CMDQ_BUF (No longer use)
*/
struct alifbio_cmdq_buf
{
    int cmdq_index;     //!<0 - HQ, 1 - LQ
    uint32 *cmdq_buf;   //!<Should be virtual address
    int cmdq_size;      //!<In bytes
};

/* FBIO_RPC_OPERATION :
It does Remote Process Call operation to the dis */
/*RPC API index definition */
#define RPC_FB_OPEN                 1   //!<Internal use
#define RPC_FB_CLOSE                2   //!<Internal use
#define RPC_FB_WIN_ON_OFF           3   //!<Internal use
#define RPC_FB_WIN_MODE             4   //!<Internal use
#define RPC_FB_ZOOM                 5   //!<Internal use
#define RPC_FB_ASPECT_MODE          6   //!<Internal use
#define RPC_FB_TVSYS                7   //!<Internal use
#define RPC_FB_TVSYS_EX             8   //!<Internal use
#define RPC_FB_IOCTL                9   //!<Internal use
#define RPC_FB_CONFIG_SOURCE_WINDOW 10  //!<Internal use
#define RPC_FB_SET_PROGRES_INTERL   11  //!<Internal use
#define RPC_FB_WIN_ON_OFF_EX        12  //!<Internal use
#define RPC_FB_ZOOM_EX              13  //!<Internal use

#define MAX_FB_RPC_ARG_NUM          4   //!<Internal use
#define MAX_FB_RPC_ARG_SIZE         512 //!<Internal use

/*! @struct ali_fb_rpc_arg
@brief Internal use
*/
struct ali_fb_rpc_arg
{
    void *arg;
    int arg_size;
    int out;
};

/*! @struct ali_fb_rpc_pars
@brief Internal use
*/
struct ali_fb_rpc_pars
{
    int hd_dev; /* 0 : SD output 1 : HD output*/
    int API_ID;
    struct ali_fb_rpc_arg arg[MAX_FB_RPC_ARG_NUM];
    int arg_num;
};

/*! @struct ali_fb_rsc_mem_map
@brief Define parameter of FBIO_GET_UI_RSC_MAP
*/
struct ali_fb_rsc_mem_map
{
    void *mem_start;           //!<Memory start address
    unsigned long mem_size;    //!<Memory size
};

#define FBIO_SET_ENHANCE_BRIGHTNESS 0x01    //!<Brightess enhance, value[0, 100], default 50
#define FBIO_SET_ENHANCE_CONTRAST   0x02    //!<Contrast enhance, value[0, 100], default 50
#define FBIO_SET_ENHANCE_SATURATION 0x04    //!<Saturation enhance, value[0, 100], default 50
#define FBIO_SET_ENHANCE_SHARPNESS  0x08    //!<Sharpness enhance, value[0, 10], default 5
#define FBIO_SET_ENHANCE_HUE        0x10    //!<Hue enhance, value[0, 100], default 50

/*! @struct ali_fb_video_enhance_pars
@brief Parameter of FBIO_VIDEO_ENHANCE
*/
struct ali_fb_video_enhance_pars
{
    unsigned char changed_flag;    //!<Enhance type
    unsigned short grade;          //!<Enhance value
};


typedef struct ctrl_blk ali_vdeo_ctrl_blk;

//! @typedef ali_video_request_buf
//! @brief Internal use
typedef ali_dec_request_buf ali_video_request_buf;

//! @typedef ali_video_update_buf
//! @brief Internal use
typedef ali_dec_update_buf ali_video_update_buf;

/*! @enum vdec_output_mode
@brief Video output mode
*/
enum vdec_output_mode
{
    VDEC_FULL_VIEW,    //!<Full screen display
    VDEC_PREVIEW,      //!<Preview display
    VDEC_SW_PASS,      //!<Do not decode, just consume data
};

/*! @enum vdec_output_mode
@brief Video synchronization mode
*/
enum vdec_avsync_mode
{
    VDEC_AVSYNC_PTS = 0,    //!<Do synchronization
    VDEC_AVSYNC_FREERUN,    //!<Don't do synchronization
};

/*! @enum vdec_playback_rate
@brief Video playback speed
*/
enum vdec_playback_rate
{
    VDEC_RATE_1_2,    //!<1/2 times the speed
    VDEC_RATE_1_4,    //!<1/4 times the speed
    VDEC_RATE_1_8,    //!<1/8 times the speed
    VDEC_RATE_STEP,   //!<Step play
    VDEC_RATE_1,      //!<Normal speed
    VDEC_RATE_2,      //!<2 times the speed
    VDEC_RATE_4,      //!<4 times the speed
    VDEC_RATE_8,      //!<8 times the speed
};

/*! @enum vdec_playback_dir
@brief Video playback direction
*/
enum vdec_playback_dir
{
    VDEC_PLAY_FORWARD = 0,    //!<Forward play
    VDEC_PLAY_BACKWARD,       //!<Backward play
};

/*! @enum vdec_type
@brief Video decoder type
*/
enum vdec_type
{
    VDEC_MPEG,    //!<MPEG1/2 decoder
    VDEC_AVC,     //!<H.264 decoder
    VDEC_AVS,     //!<AVS decoder
};

/*! @struct vdec_stop_param
@brief Define parameter of VDECIO_STOP
*/
struct vdec_stop_param
{
    int32 close_display;    //!<Whether close vpo
    int32 fill_black;       //!<Whether fill frame buffer black
};

/*! @enum vdec_status
@brief Video status
*/
enum vdec_status
{
    VDEC_STARTED = 1,    //!<Started state
    VDEC_STOPPED,        //!<Stopped state
    VDEC_PAUSED,         //!<Paused state
};

/*! @struct vdec_yuv_color
@brief Video yuv color
*/
struct vdec_yuv_color
{
    uint8 y;    //!<Y
    uint8 u;    //!<U
    uint8 v;    //!<V
};

/*! @struct vdec_sync_param
@brief Define parameter of VDECIO_SET_SYNC_MODE
*/
struct vdec_sync_param
{
    enum vdec_avsync_mode sync_mode;    //!<Synchronization mode
};

/*! @struct ali_video_mem_info
@brief Define parameter of VDECIO_GET_MEM_INFO
*/
struct ali_video_mem_info
{
    void *mem_start;               //!<Start address of memory allocated to video in Main CPU
    unsigned long mem_size;        //!<Size of memory allocated to video in Main CPU
    void *priv_mem_start;          //!<Start address of memory allocated to video in SEE CPU
    unsigned long priv_mem_size;   //!<Size of memory allocated to video in SEE CPU
    void *mp_mem_start;            //!<Start address of memory allocated to media player
    unsigned long mp_mem_size;     //!<Size of memory allocated to media player
};

/*! @struct vdec_playback_param
@brief Define parameter of VDECIO_SET_PLAY_MODE
*/
struct vdec_playback_param
{
    enum vdec_playback_dir direction;
    enum vdec_playback_rate rate;
};

/*! @struct vdec_pvr_param
@brief Define parameter of VDECIO_SET_PVR_PARAM
*/
struct vdec_pvr_param
{
    int32 is_scrambled;    //!<Whether the stream is scrambled
};

/*! @struct vdec_codec_param
@brief Define parameter of VDECIO_SELECT_DECODER
*/
struct vdec_codec_param
{
    enum vdec_type type;    //!<Video deocder type
    int32 preview;          //!<Whether in preview
};

/*! @struct vdec_status_info
@brief Define parameter of VDECIO_GET_STATUS
*/
struct vdec_status_info
{
    enum vdec_status status;      //!<Video decoder's state
    int8   first_header_parsed;   //!<Whether the first header is parsed
    int8   first_pic_decoded;     //!<Whether the first picture is decoded
    int8   first_pic_showed;      //!<Whether the first picture is displayed
    uint16 pic_width;             //!<Picture width
    uint16 pic_height;            //!<Picture height
    enum AspRatio aspect_ratio;   //!<Aspect ratio of the stream
    uint32 frames_displayed;      //!<Number of displayed frames
    uint32 frames_decoded;        //!<Number of decoded frames
    int64  frame_last_pts;        //!<PTS of last displayed frame
    int32  show_frame;            //!<Whether to display frame
    uint8  queue_count;           //!<Number of frames in display queue
    uint32 buffer_size;           //!<Total es buffer size
    uint32 buffer_used;           //!<Used size of es buffer
    uint32 frame_rate;            //!<Frame rate of the stream
    int32  interlaced_frame;      //!<Whether the stream is interlaced
    int32  top_field_first;       //!<Whether the stream is top field first
    int32  hw_dec_error;          //!<Decoder error
    int32  is_support;            //!<Whether the stream is supported
    enum vdec_output_mode output_mode;               //!<Video decoder's output mode
    struct vdec_playback_param playback_param;       //!<Video decoder's playback info
    struct vdec_playback_param api_playback_param;   //!<Playback info set by user
};

/*! @struct vdec_display_rect
@brief Display rectangle
*/
struct vdec_display_rect
{
    int32 src_x;    //!<Horizontal start point of source
    int32 src_y;    //!<Vertical start point of source
    int32 src_w;    //!<Horizontal size of source
    int32 src_h;    //!<Vertical size of source
    int32 dst_x;    //!<Horizontal start point of destination
    int32 dst_y;    //!<Vertical start point of destination
    int32 dst_w;    //!<Horizontal size of destination
    int32 dst_h;    //!<Vertical size of destination
};

/*! @struct vdec_output_param
@brief Define parameter of VDECIO_SET_OUTPUT_MODE
*/
struct vdec_output_param
{
    enum vdec_output_mode output_mode;      //!<Video decoder's output mode
    int32 smooth_switch;                    //!<Mode of full screen and preview switching
    enum TVSystem tv_sys;                   //!<Current TV system
    int32 progressive;                      //!<Current TV system
    struct MPSource_CallBack mp_callback;   //!<Main picture callback
    struct PIPSource_CallBack pip_callback; //!<PIP picture callback
};

/*! @enum ALI_VIDEO_DIRECTION
@brief Define parameter of ALIVIDEOIO_VIDEO_DIRECTION (No longer use)
*/
enum ALI_VIDEO_DIRECTION
{
    ALI_VIDEO_FORWARD,
    ALI_VIDEO_BACKWARD
};

/*! @enum ALI_VIDEO_SPEED
@brief Define parameter of ALIVIDEOIO_VIDEO_SPEED (No longer use)
*/
enum ALI_VIDEO_SPEED
{
    ALI_VIDEO_SPEEDX1,
    ALI_VIDEO_SPEEDX2,
    ALI_VIDEO_SPEEDX4,
    ALI_VIDEO_SPEEDX8,
    ALI_VIDEO_SPEEDX16,
    ALI_VIDEO_SPEEDX32,

    ALI_VIDEO_SPEED_1_2 = 0x11,
    ALI_VIDEO_SPEED_1_4,
    ALI_VIDEO_SPEED_1_8,
    ALI_VIDEO_SPEED_1_16,
    ALI_VIDEO_SPEED_1_32,

    ALI_VIDEO_SPEED_STEP = 0x101
};

#define ALI_VIDEO_SPEED_UP_MAX          ALI_VIDEO_SPEEDX32      //!<No longer use
#define ALI_VIDEO_SPEED_DOWN_MAX        ALI_VIDEO_SPEED_1_32    //!<No longer use

/*! @enum ALI_VIDEO_SYNC_MODE
@brief Define parameter of ALIVIDEOIO_VIDEO_SYNC_MODE (No longer use)
*/
enum ALI_VIDEO_SYNC_MODE
{
    ALI_VIDEO_SYNC_MODE_ENABLE,
    ALI_VIDEO_SYNC_MODE_DISABLE,
};

#define ALI_VIDEO_SYNC_FIRSTFREERUN     0x0001    //!<No longer use
#define ALI_VIDEO_SYNC_I                0x0002    //!<No longer use
#define ALI_VIDEO_SYNC_P                0x0004    //!<No longer use
#define ALI_VIDEO_SYNC_B                0x0008    //!<No longer use
#define ALI_VIDEO_SYNC_ALL              0xFFFF    //!<No longer use

/*! @struct ali_video_sync_pars
@brief No longer use
*/
struct ali_video_sync_pars
{
    enum ALI_VIDEO_SYNC_MODE sync_mode;
    uint32 sync_flags;
};

#define ALI_VIDEO_OUT_INFO_FLAG_FIRST_SHOWED        0x00000001    //!<Internal use
#define ALI_VIDEO_OUT_INFO_FLAG_FIRST_HEADER        0x00000002    //!<Internal use

/*! @struct ali_video_out_info_pars
@brief Internal use
*/
struct ali_video_out_info_pars
{
    int dev_idx;
    uint32 flag;
    int started;
    int width;
    int height;
    int frame_rate;
    int progressive;
    int display_idx;
    uint32 read_p_offset;
    uint32 write_p_offset;
    int valid_size;
    int is_support;
};

/*! @struct ali_video_preview_pars
@brief Define parameter of ALIVIDEOIO_SET_PREVIEW_PARS (No longer use)
*/
struct ali_video_preview_pars
{
    int preview_width;
    int preview_height;
};

/*! @struct ali_video_out_info_pars
@brief Define parameter of ALIVIDEOIO_SET_SEE_TEST_MODE (No longer use)
*/
enum ali_video_see_test_mode
{
    VDEC_TEST_NONE,
    VDEC_TEST_Check_NAL,
    VDEC_TEST_Check_HEADER,
};

/*! @struct ali_vpo_win_config_pars
@brief Define parameter of VPO_CONFIG_SOURCE_WINDOW
*/
struct ali_vpo_win_config_pars
{
    int hd_dev;
    struct vp_win_config_para win_config_para;
};

/*! @struct ali_vpo_winmode_pars
@brief Define parameter of VPO_SET_WIN_MODE
*/
struct ali_vpo_winmode_pars
{
    int hd_dev;                              //!<0: SD output; 1: HD output
    uint32 win_mode;                         //!<VPO working mode
    struct MPSource_CallBack mp_callback;    //!<Main picture callback
    struct PIPSource_CallBack pip_callback;  //!<PIP picture callback
};

/*! @struct ali_vpo_tvsys_pars
@brief Define parameter of VPO_SET_TVSYS_EX
*/
struct ali_vpo_tvsys_pars
{
    int hd_dev;             //!<0: SD output; 1: HD output
    int progressive;        //!<Whether TV system is progressive
    enum TVSystem tvsys;    //!<Current TV system
};

/*! @struct ali_vpo_zoom_pars
@brief Define parameter of VPO_WIN_ZOOM
*/
struct ali_vpo_zoom_pars
{
    int hd_dev;                     //!<0: SD output; 1: HD output
    struct Rect src_rect;           //!<Video source coordiante and resolution
    struct Rect dst_rect;           //!<Displaying coordiante and resolution     
    enum vp_display_layer layer;    //!<Video/Graphic layers 
};

/*! @struct ali_vpo_aspect_pars
@brief Define parameter of VPO_SET_ASPECT_MODE
*/
struct ali_vpo_aspect_pars
{
    int hd_dev;                     //!<0: SD output; 1: HD output
    enum TVMode aspect;             //!<Aspect ratio of the TV
    enum DisplayMode display_mode;  //!<Display mode of the TV
};

/*! @struct ali_vpo_win_status_pars
@brief define parameter of VPO_SET_WIN_ONOFF
*/
struct ali_vpo_win_status_pars
{
    int hd_dev;                     //!<0: SD output; 1: HD output
    int on;                         //!<0: off; 1: on 
    enum vp_display_layer layer;    //!<Video/Graphic layers 
};

/*! @struct ali_vpo_bgcolor_pars
@brief Define parameter of VPO_SET_BG_COLOR
*/
struct ali_vpo_bgcolor_pars
{
    int hd_dev;                     //!<0: SD output; 1: HD output
    struct YCbCrColor yuv_color;    //!<Backgroud color space
};

/*! @struct ali_vpo_dac_pars
@brief Define parameter of VPO_REG_DAC
*/
struct ali_vpo_dac_pars
{
    int hd_dev;                             //!<0: SD output; 1: HD output
    struct vp_io_reg_dac_para dac_param;    
};

/*! @struct ali_vpo_parameter_pars
@brief Define parameter of VPO_SET_PARAMETER
*/
struct ali_vpo_parameter_pars
{
    int hd_dev;
    struct vpo_io_set_parameter param;
};

/*! @struct ali_vpo_video_enhance_pars
@brief Define parameter of VPO_VIDEO_ENHANCE
*/
struct ali_vpo_video_enhance_pars
{
    int hd_dev;
    struct vpo_io_video_enhance video_enhance_param;
};

/*! @struct ali_vpo_cgms_info_pars
@brief Define parameter of VPO_SET_CGMS_INFO
*/
struct ali_vpo_cgms_info_pars
{
    int hd_dev;
    struct vpo_io_cgms_info cgms_info;
};

/*! @struct ali_vpo_afd_pars
@brief Define parameter of VPO_AFD_CONFIG
*/
struct ali_vpo_afd_pars
{
    int hd_dev;
    struct vp_io_afd_para afd_param;
};

/*! @struct ali_vpo_display_info_pars
@brief Define parameter of VPO_GET_CURRENT_DISPLAY_INFO
*/
struct ali_vpo_display_info_pars
{
    int hd_dev;
    struct vpo_io_get_picture_info display_info;
};

/*! @struct ali_vpo_osd_show_time_pars
@brief Define parameter of VPO_SET_OSD_SHOW_TIME
*/
struct ali_vpo_osd_show_time_pars
{
    int hd_dev;
    vpo_osd_show_time_t osd_show_time;
};

/*! @struct ali_vpo_screem_rect_pars
@brief Define parameter of VPO_GET_MP_SCREEN_RECT
*/
struct ali_vpo_screem_rect_pars
{
    int hd_dev;
    struct Rect mp_screem_rect;
};

/*! @struct ali_vpo_mp_info_pars
@brief Define parameter of VPO_GET_MP_INFO
*/
struct ali_vpo_mp_info_pars
{
    int hd_dev;
    struct vpo_io_get_info mp_info;
};

/*! @struct ali_vpo_ioctrl_pars
@brief Define parameter of io control
*/
struct ali_vpo_ioctrl_pars
{
    int hd_dev;
    uint32 param;
};

#define ALIVIDEOIO_VIDEO_STOP                           0x550001    //!<Internal use
#define ALIVIDEOIO_VIDEO_PLAY                           0x550002    //!<Internal use
#define ALIVIDEOIO_VIDEO_PAUSE                          0x550003    //!<No longer use
#define ALIVIDEOIO_VIDEO_RESUME                         0x550004    //!<No longer use
#define ALIVIDEOIO_VIDEO_STEP                           0x550005    //!<No longer use
#define ALIVIDEOIO_VIDEO_BLANK                          0x550006    //!<No longer use
#define ALIVIDEOIO_VIDEO_DIRECTION                      0x550007    //!<No longer use
#define ALIVIDEOIO_VIDEO_SPEED                          0x550008    //!<No longer use
#define ALIVIDEOIO_VIDEO_SYNC_MODE                      0x550009    //!<No longer use

#define ALIVIDEOIO_SET_DECODER_FORMAT                   0x550020    //!<No longer use
#define ALIVIDEOIO_GET_INPUT_CALLBACK_ROUTINE           0x550021    //!<No longer use
#define ALIVIDEOIO_SET_FBINFO_HANDLE                    0x550022    //!<No longer use
#define ALIVIDEOIO_SET_DMA_INFO                         0x550023    //!<No longer use
#define ALIVIDEOIO_GET_OUT_INFO                         0x550024    //!<Internal use
#define ALIVIDEOIO_OUTPUT_FIRST_FRAME                   0x550025    //!<No longer use
#define ALIVIDEOIO_SET_PREVIEW_PARS                     0x550026    //!<No longer use

#define ALIVIDEOIO_RPC_OPERATION                        0x550040    //!<Internal use
#define ALIVIDEOIO_SET_SOCK_PORT_ID                     0x550041    //!<Internal use
#define ALIVIDEOIO_GET_MEM_INFO                         0x550042    //!<Internal use
#define ALIVIDEOIO_SET_CTRL_BLK_INFO                    0x550043    //!<Internal use
#define ALIVIDEOIO_GET_BOOTMEDIA_INFO                   0x550044    //!<Internal use
#define ALIVIDEO_ROTATION                               0x550045    //!<Internal use
#define ALIVIDEOIO_SET_MODULE_INFO                      0x550046    //!<Internal use
#define ALIVIDEOIO_GET_BOOTMEDIA_TIME                   0x550047    //!<Internal use

#define ALIVIDEOIO_DBG_BASE                             0x55f000    //!<Internal use
#define ALIVIDEOIO_ENABLE_DBG_LEVEL                     (ALIVIDEOIO_DBG_BASE + 0x0001)    //!<Enable kernel/see debug info (Internal use)
#define ALIVIDEOIO_DISABLE_DBG_LEVEL                    (ALIVIDEOIO_DBG_BASE + 0x0002)    //!<Disable kernel/see debug info (Internal use)
#define ALIVIDEOIO_SET_SEE_TEST_MODE                    (ALIVIDEOIO_DBG_BASE + 0x0003)    //!<Internal use
#define ALIVIDEOIO_SET_APE_DBG_MODE                     (ALIVIDEOIO_DBG_BASE + 0x0004)    //!<Enable/Disable ape debug info (Internal use)
#define ALIVIDEOIO_GET_APE_DBG_MODE                     (ALIVIDEOIO_DBG_BASE + 0x0005)    //!<Get ape debug info (Internal use)

#define VDEC_MAGIC                                      0x56

#define VDECIO_START                                    _IO(VDEC_MAGIC, 0)    //!<Start video decoder
#define VDECIO_PAUSE                                    _IO(VDEC_MAGIC, 1)    //!<Pause display
#define VDECIO_RESUME                                   _IO(VDEC_MAGIC, 2)    //!<Resume diplay
#define VDECIO_STEP                                     _IO(VDEC_MAGIC, 3)    //!<Step play
#define VDECIO_DRAW_COLOR_BAR                           _IO(VDEC_MAGIC, 4)    //!<Display color bar

#define VDECIO_STOP                                     _IOR(VDEC_MAGIC, 0, struct vdec_stop_param)      //!<Stop video decoder
#define VDECIO_GET_CUR_DECODER                          _IOR(VDEC_MAGIC, 1, enum vdec_type)              //!<Get current video decoder's type
#define VDECIO_GET_STATUS                               _IOR(VDEC_MAGIC, 2, struct vdec_status_info)     //!<Get video's current status
#define VDECIO_GET_MEM_INFO                             _IOR(VDEC_MAGIC, 3, struct ali_video_mem_info)   //!<Get info of memory allocated to video

#define VDECIO_SET_SYNC_MODE                            _IOW(VDEC_MAGIC, 0, struct vdec_sync_param)      //!<Set synchronization mode
#define VDECIO_SET_PLAY_MODE                            _IOW(VDEC_MAGIC, 1, struct vdec_playback_param)  //!<Set playback mode
#define VDECIO_SET_PVR_PARAM                            _IOW(VDEC_MAGIC, 2, struct vdec_pvr_param)       //!<Set pvr parameter
#define VDECIO_SELECT_DECODER                           _IOW(VDEC_MAGIC, 3, struct vdec_codec_param)     //!<Select video decoder
#define VDECIO_FIRST_I_FREERUN                          _IOW(VDEC_MAGIC, 4, int)                         //!<Whether the first picture do synchronization or not
#define VDECIO_SET_SYNC_DELAY                           _IOW(VDEC_MAGIC, 5, unsigned long)               //!<Set synchronization deley [-500, 500]ms
#define VDECIO_CONTINUE_ON_ERROR                        _IOW(VDEC_MAGIC, 6, unsigned long)               //!<Whether continue to display error picture
#define VDECIO_SET_OUTPUT_RECT                          _IOW(VDEC_MAGIC, 7, struct vdec_display_rect)    //!<Set display rectangle
#define VDECIO_FILL_FRAME                               _IOW(VDEC_MAGIC, 8, struct vdec_yuv_color)       //!<Fill frame buffer with specified color
#define VDECIO_SET_DMA_CHANNEL                          _IOW(VDEC_MAGIC, 9, unsigned char)               //!<Set dma channel number
#define VDECIO_DTVCC_PARSING_ENABLE                     _IOW(VDEC_MAGIC, 10, int)                        //!<Whether to parse dtv cc
#define VDECIO_SAR_ENABLE                               _IOW(VDEC_MAGIC, 11, int)                        //!<Whether to enable sample aspect ratio
#define VDECIO_SET_DEC_FRM_TYPE                         _IOW(VDEC_MAGIC, 12, unsigned long)              //!<Set frame type to decode
#define VDECIO_SET_SOCK_PORT_ID                         _IOW(VDEC_MAGIC, 13, int)                        //!<Set sock port id
#define VDECIO_VBV_BUFFER_OVERFLOW_RESET                _IOW(VDEC_MAGIC, 14, int)                        //!<Whether to reset es buffer when buffer overflow
#define VDECIO_SET_SIMPLE_SYNC                          _IOW(VDEC_MAGIC, 15, int)                        //!<Set simple synchronization mode
#define VDECIO_SET_TRICK_MODE                           _IOW(VDEC_MAGIC, 16, struct vdec_playback_param) //!<Set trick play mode for playback
#define VDECIO_REG_CALLBACK                             _IOW(VDEC_MAGIC, 17, unsigned long)              //!<Register callback function
#define VDECIO_UNREG_CALLBACK                           _IOW(VDEC_MAGIC, 18, unsigned long)              //!<Unregister callback function

#define VDECIO_SET_OUTPUT_MODE                          _IOWR(VDEC_MAGIC, 0, struct vdec_output_param)   //!<Set video's output mode
#define VDECIO_CAPTURE_DISPLAYING_FRAME                 _IOWR(VDEC_MAGIC, 1, struct vdec_picture)        //!<Caputre displaying frame

#define VDECIO_MP_GET_STATUS                            _IOR(VDEC_MAGIC, 200, struct vdec_decore_status) //!<Get status in mp mode

#define VDECIO_MP_INITILIZE                             _IOW(VDEC_MAGIC, 200, struct vdec_mp_init_param) //!<Video initilization in mp mode
#define VDECIO_MP_RELEASE                               _IOW(VDEC_MAGIC, 201, struct vdec_mp_rls_param)  //!<Video deinitilization in mp mode
#define VDECIO_MP_FLUSH                                 _IOW(VDEC_MAGIC, 202, struct vdec_mp_flush_param)//!<Video flush in mp mode
#define VDECIO_MP_EXTRA_DATA                            _IOW(VDEC_MAGIC, 203, struct vdec_mp_extra_data) //!<Decode extra data in mp mode
#define VDECIO_MP_PAUSE                                 _IOW(VDEC_MAGIC, 204, struct vdec_mp_pause_param)//!<Pause decode/display in mp mode
#define VDECIO_MP_SET_SBM_IDX                           _IOW(VDEC_MAGIC, 205, struct vdec_mp_sbm_param)  //!<Set sbm to video
#define VDECIO_MP_SET_SYNC_MODE                         _IOW(VDEC_MAGIC, 206, struct vdec_mp_sync_param) //!<Set synchronizaion in mp mode
#define VDECIO_MP_SET_DISPLAY_RECT                      _IOW(VDEC_MAGIC, 207, struct vdec_display_rect)  //!<Set display rectangle in mp mode
#define VDECIO_MP_SET_QUICK_MODE                        _IOW(VDEC_MAGIC, 208, unsigned long)             //!<Set quick mode in mp mode
#define VDECIO_MP_SET_DEC_FRM_TYPE                      _IOW(VDEC_MAGIC, 209, unsigned long)             //!<Set frame type to decode in mp mode, 0: normal 1: first I
#define VDECIO_MP_DYNAMIC_FRAME_ALLOC                   _IOW(VDEC_MAGIC, 210, unsigned long)             //!<Set dynamic frame buffer allocation

#define VDECIO_MP_CAPTURE_FRAME                         _IOWR(VDEC_MAGIC, 200, struct vdec_picture)      //!<Caputre displaying frame in mp mode

#define ALI_VIDEO_REQ_RET_FAIL                          ALI_DECV_REQ_RET_FAIL    //!<Internal use
#define ALI_VIDEO_REQ_RET_OK                            ALI_DECV_REQ_RET_OK      //!<Internal use
#define ALI_VIDEO_REQ_RET_ERROR                         ALI_DECV_REQ_RET_ERROR   //!<Internal use

#define RPC_VIDEO_OPEN                1    //!<Internal use
#define RPC_VIDEO_CLOSE               2    //!<Internal use
#define RPC_VIDEO_START               3    //!<Internal use
#define RPC_VIDEO_STOP                4    //!<Internal use
#define RPC_VIDEO_VBV_REQUEST         5    //!<Internal use
#define RPC_VIDEO_VBV_UPDATE          6    //!<Internal use
#define RPC_VIDEO_SET_OUT             7    //!<Internal use
#define RPC_VIDEO_SYNC_MODE           8    //!<Internal use
#define RPC_VIDEO_PROFILE_LEVEL       9    //!<Internal use
#define RPC_VIDEO_IO_CONTROL          10   //!<Internal use
#define RPC_VIDEO_PLAY_MODE           11   //!<Internal use
#define RPC_VIDEO_DVR_SET_PAR         12   //!<Internal use
#define RPC_VIDEO_DVR_PAUSE           13   //!<Internal use
#define RPC_VIDEO_DVR_RESUME          14   //!<Internal use
#define RPC_VIDEO_DVR_STEP            15   //!<Internal use
#define RPC_VIDEO_SELECT_DEC          16   //!<Internal use
#define RPC_VIDEO_GET_DECODER         17   //!<Internal use
#define RPC_VIDEO_IS_AVC              18   //!<Internal use
#define RPC_VIDEO_SWITCH_DEC          19   //!<Internal use
#define RPC_VIDEO_GET_CURRENT_DEC     20   //!<Internal use
#define RPC_VIDEO_DECORE_IOCTL        21   //!<Internal use
#define RPC_VIDEO_DECODER_SELECT_NEW  22   //!<Internal use

#define RPC_VIDEO_INTENAL             0xF0000000    //!<Internal use
#define RPC_VIDEO_SET_DBG_FLAG        (RPC_VIDEO_INTENAL + 0x0001)    //!<Internal use

#define MAX_VIDEO_RPC_ARG_NUM         4    //!<Internal use
#define MAX_VIDEO_RPC_ARG_SIZE        2048 //!<Internal use

#define VDEC_SYNC_PTS                 0x00 //!<Internal use
#define VDEC_SYNC_FREERUN             0x01 //!<Internal use

/*! @enum VDEC_STATUS
@brief Internal use
*/
enum VDEC_STATUS {
    VDEC27_ATTACHED,
    VDEC27_STARTED,
    VDEC27_STOPPED,
    VDEC27_PAUSED,
};

/*! @struct VDec_StatusInfo
@brief Internal use
*/
struct VDec_StatusInfo
{
    enum VDEC_STATUS uCurStatus;
    int32 uFirstPicShowed;
    int32 bFirstHeaderGot;
    uint16 pic_width;
    uint16 pic_height;
    uint16 status_flag;
    uint32 read_p_offset;
    uint32 write_p_offset;
    uint32 display_idx;
    int32 use_sml_buf;
    enum VDecOutputMode output_mode;
    uint32 valid_size;
    uint32 MPEG_format;
    enum AspRatio aspect_ratio;
    uint16 frame_rate;
    uint32 bit_rate;
    int32 hw_dec_error;
    int32 display_frm;
    uint8 top_cnt;
    uint8 play_direction;
    uint8 play_speed;
    uint8 api_play_direction;
    uint8 api_play_speed;
    int32 is_support;
    uint32 vbv_size;
    uint8 cur_dma_ch;
    int32 progressive;
    int32 top_field_first;
    int32 first_pic_decoded;
    uint32 frames_decoded;
    uint32 frame_last_pts;
};

/*! @struct ali_video_rpc_arg
@brief Internal use
*/
struct ali_video_rpc_arg
{
    void *arg;
    int arg_size;
    int out;
};

/*! @struct ali_video_rpc_pars
@brief Internal use
*/
struct ali_video_rpc_pars
{
    int API_ID;
    struct ali_video_rpc_arg arg[MAX_VIDEO_RPC_ARG_NUM];
    int arg_num;
};

/*! @struct vdec_reserve_buf
@brief Internal use
*/
struct vdec_reserve_buf
{
    uint32 buf_addr;
    uint32 buf_size;
};

#define MSG_FIRST_SHOWED          1     //!<Message of first picture displayed
#define MSG_MODE_SWITCH_OK        2     //!<Not used temporarily
#define MSG_BACKWARD_RESTART_GOP  3     //!<Message of restarting GOP in backward play
#define MSG_FIRST_HEADRE_PARSED   4     //!<Message of first header parsed
#define MSG_UPDATE_OUT_INFO       5     //!<Internal use
#define MSG_FIRST_I_DECODED       6     //!<Message of first I picture decoded
#define MSG_FF_FB_SHOWED          7     //!<Message of first picture displayed in ff/fb play
#define MSG_USER_DATA_PARSED      8     //!<Message of user data parsed

// decode status machine
#define VDEC_NEW_VIDEO_PACKET     1     //!<Ready to decode new video packet(picture) (Internal use)
#define VDEC_WAIT_NEW_FB          2     //!<Waiting for free buffer to decode video (Internal use)
#define VDEC_NEW_SLICE            3     //!<Ready to decode new video slice (Internal use)
#define VDEC_ON_GOING             4     //!<Video decoding is on going (Internal use)
#define VDEC_POST_PROCESS         5     //!<Last picture decoding is done, ready to do some post process (Internal use)
#define VDEC_CONTINUE             6     //!<Decore needs to push last decoded P VOP into DQ in B-VOP decoding and continue B-VOP decoding (Internal use)

// decode mode
#define VDEC_MODE_NORMAL          0     //!<Normal decode
#define VDEC_MODE_VOL             1     //!<Parse all headers above (including) VOP level without VOP reconstuction (Internal use)
#define VDEC_MODE_HEADER          2     //!<Parse header to get current frame's prediction type (Internal use)
#define VDEC_MODE_SKIP_B_FRAME    3     //!<Skip b frame (Internal use)
#define VDEC_MODE_SKIP_B_P_FRAME  4     //!<Only decode i frame (Internal use)
#define VDEC_MODE_SBM             5     //!<Decode from sbm
#define VDEC_MODE_SBM_STREAM      6     //!<Decode from sbm

// decoder command
#define VDEC_CMD_INIT             0     //!<Initialize the decoder (Internal use)
#define VDEC_CMD_RELEASE          1     //!<Release the decoder (Internal use)
#define VDEC_CMD_DECODE_FRAME     2     //!<Decode a frame (Internal use)
#define VDEC_CMD_SW_RESET         3     //!<Reset the decoder SW status (Internal use)
#define VDEC_CMD_HW_RESET         4     //!<Reset the HW to be ready to decode next frame (Internal use)
#define VDEC_CMD_EXTRA_DADA       5     //!<Decode extra data before decode frame (Internal use)
#define VDEC_CMD_GET_STATUS       6     //!<Get decoder status (Internal use)
#define VDEC_CMD_PAUSE_DECODE     7     //!<Pause decode task running (Internal use)
#define VDEC_CMD_FF_FB            8     //!<Fast forward/Fast backward (Internal use)
#define VDEC_CMD_REG_CB           9     //!<Register callback function (Internal use)
#define VDEC_CMD_GET_TRICK_FRAME  10    //!<Internal use
#define VDEC_CMD_NORMAL_PLAY      11    //!<Internal use
#define VDEC_CMD_ROTATION_UPDATA_FRMBUF 12    //!<Internal use
#define VDEC_CMD_ROTATION_SET_FRMBUF    13    //!<Internal use
#define VDEC_CMD_ROTATION               14    //!<Internal use

#define VDEC_DQ_INIT              20    //!<*pParam1 -- de_index, *pParam2 --, int DQ for de_index (Internal use)
#define VDEC_DQ_ADD_NULL_FRAME    21    //!<*pParam1 -- de_index, *pParam2 --, Add an null frame into DQ (Internal use)
#define VDEC_DQ_PEEK_FRAME        22    //!<*pParam1 -- de_index, *pParam2 -- the next frame_id in DQ (Internal use)
#define VDEC_DQ_POP_FRAME         23    //!<*pParam1 -- de_index, *pParam2 -- the frame_id to be poped (Internal use)
#define VDEC_DQ_GET_COUNT         24    //!<*pParam1 -- de_index, *pParam2 -- the frame number in DQ (Internal use)
#define VDEC_FRAME_PEEK_FREE      25    //!<*pParam1 -- the next free frm_idx, *pParam2 -- the next free pip_idx (Internal use)
#define VDEC_FRAME_GET_INFO       26    //!<*pParam2 -- frame_id, pParam1 -- struct DisplayInfo * (Internal use)
#define VDEC_FRAME_GET_POC        27    //!<*pParam2 -- frame_id, pParam1 -- int *p_poc, (Internal use)
#define VDEC_FRAME_SET_FREE       28    //!<*pParam2 -- frame_id, bit16 must be set or clear, see DEC_LAST_REQ_FRAME_FLAG (Internal use)
#define VDEC_FRAME_REQUESTING     29    //!<*pParam2 -- frame_id, pParam1 -- struct OutputFrmManager *, (Internal use)
#define VDEC_FRAME_REQUEST_OK     30    //!<*pParam2 -- frame_id, *pParam1 -- de_index, (Internal use)
#define VDEC_FRAME_RELEASE_OK     31    //!<*pParam2 -- frame_id, *pParam1 -- de_index, (Internal use)
#define VDEC_CLEAR_SCREEN         32    //!<*pParam2 -- flag,     *pParam1 -- de_index, (Internal use)
#define VDEC_START_OUTPUT         33    //!<*pParam2 -- flag,     *pParam1 -- de_index, (Internal use)
#define VDEC_FRAME_GET_PTS        34    //!<*pParam2 -- frame_id, pParam1 -- uint32 *pts (Internal use)
#define VDEC_CFG_VIDEO_SBM_BUF    35    //!<*pParam1 -- smb idx for video pkt buf, *pParam2 -- sbm idx for video output queue (Internal use)
#define VDEC_CFG_DISPLAY_SBM_BUF  36    //!<*pParam1 -- smb idx for display queue (Internal use)
#define VDEC_CFG_SYNC_MODE        37    //!<*pParam1 -- av sync mode (Internal use)
#define VDEC_CFG_SYNC_THRESHOLD   38    //!<*pParam1 -- hold threshold, *pParam2 -- drop threshold (Internal use)
#define VDEC_CFG_SYNC_DELAY       39    //!<*pParam1 -- av sync delay [-500ms, 500ms] (Internal use)
#define VDEC_CFG_DISPLAY_RECT     40    //!<*pParam1 -- source rect, *pParam2 destination rect (Internal use)
#define VDEC_CFG_DECODE_MODE      41    //!<*pParam1 -- decode mode (Internal use)
#define VDEC_CFG_FREERUN_TRD      42    //!<*pParam1 -- hol2free threshold, *pParam2 -- drop2free threshold (Internal use)
#define VDEC_CFG_SYNC_PARAM       43    //!<*pParam1 -- 0:hold 1:drop 2:rollback, *pParam2 -- 0:increase 1:decrease (Internal use)
#define VDEC_CFG_QUICK_MODE       44    //!<*pParam1 -- quick mode (Internal use)
#define VDEC_CAPTURE_FRAME        45    //!<*pParam1 -- struct vdec_picture (Internal use)
#define VDEC_ROTATE_FRAME         46    //!<*pParam1 -- frame angle (Internal use)
#define VDEC_GET_CAPTURE_FRAME_INFO     47    //!<*pParam1 -- struct vdec_capture_frm_info
#define VDEC_DYNAMIC_FRAME_ALLOC  48    //!<*pParam1 -- enable frame malloc or not

// decode return values
#define VDEC_SUCCESS              0     //!<Internal use
#define VDEC_FAILURE              1     //!<Internal use
#define VDEC_FORBIDDEN_DECODE     2     //!<Some reason cannot decode (Internal use)
#define VDEC_INVALID_CMD          3     //!<Internal use
#define VDEC_FRAME_SKIPED         4     //!<Frame skiped according to decode mode (Internal use)
#define VDEC_NO_MEMORY            5     //!<Memory not enough (Internal use)
#define VDEC_BAD_FORMAT           6     //!<Internal use
#define VDEC_NOT_IMPLEMENTED      7     //!<New feature not supported (Internal use)
#define VDEC_PATCH_PSUDO_MB       8     //!<Patch IC issue (Internal use)
#define VDEC_CURRENT_CONTINUE     9     //!<Continue decode current frame (Internal use)

// decode error type (not supported features)
#define VDEC_ERR_NONE             0
#define VDEC_ERR_SHAPE           (1 << 0)  //!<Shape coding, unsupported
#define VDEC_ERR_SPRITE          (1 << 1)  //!<Static sprite, unsupported
#define VDEC_ERR_NOT8BIT         (1 << 2)  //!<Video data precision N-bit, unsupported
#define VDEC_ERR_NEWPRED         (1 << 3)  //!<Newpred mode, unsupported
#define VDEC_ERR_SCALAB          (1 << 4)  //!<Scalability, unsupported
#define VDEC_ERR_REDUCEDRES      (1 << 5)  //!<Reduced resolution vop, unsupported
#define VDEC_ERR_3POINTGMC       (1 << 6)  //!<3-point gmc, video may appear distorted
#define VDEC_ERR_DATA_PARTITION  (1 << 7)  //!<Data partition, unsupported
#define VDEC_ERR_RESOLUTION      (1 << 8)  //!<Resolution unsupported
#define VDEC_ERR_CODEC           (1 << 9)  //!<Codec unsupported
#define VDEC_ERR_NOMEMORY        (1 << 10) //!<Not enough memory

#define VDEC_FLAG_HAS_LICENSED   (1 << 0)  //!<Has full license (Internal use)
#define VDEC_FLAG_AVC1_FORMAT    (1 << 1)  //!<AVC nalu has nalu_size on the head
#define VDEC_FLAG_MPEG4_DECODER  (1 << 2)  //!<It's a general mpeg4 decoder (Internal use)
#define VDEC_FLAG_LAST_REQ_FRAME (1 << 16) //!<An flag to free the last requested frame (Internal use)

#define VDEC_FRM_IDX_NOT_DISPLAY  0xfe     //!<An indicator for not display frame (Internal use)
#define VDEC_FRM_IDX_INVALID      0xff     //!<Internal use

#define VDEC_MIN_FRAME_BUF_NUM    4        //!<Internal use
#define VDEC_MAX_FRAME_BUF_NUM    6        //!<Internal use

#define AV_NOPTS_VALUE            ((int64)(0x8000000000000000LL))    //!<PTS is invalid or no PTS

#define MKTAG(a, b, c, d)         (a | (b << 8) | (c << 16) | (d << 24))    //!<Make video decoder's ID
#define h264                      MKTAG('h','2','6','4')    //!<H.264 decoder's ID
#define xvid                      MKTAG('x','v','i','d')    //!<MPEG4 decoder's ID
#define mpg2                      MKTAG('m','p','g','2')    //!<MPEG1/2 decoder's ID
#define flv1                      MKTAG('f','l','v','1')    //!<FLV1 decoder's ID
#define vp8                       MKTAG('v','p','8',' ')    //!<VP8 decoder's ID
#define wvc1                      MKTAG('w','v','c','1')    //!<VC1 decoder's ID
#define wmv3                      MKTAG('w','m','v','3')    //!<WMV3 decoder's ID
#define wx3                       MKTAG('w','x','3',' ')    //!<WX3 decoder's ID
#define rmvb                      MKTAG('r','m','v','b')    //!<RV decoder's ID
#define mjpg                      MKTAG('m','j','p','g')    //!<MJPG decoder's ID
#define vc1                       MKTAG('v','c','1',' ')    //!<VC1 decoder's ID
#define XD                        MKTAG('x','d',' ',' ')    //!<XD decoder's ID

//! @typedef FourCC
//! @brief Video four character codes
typedef unsigned long FourCC;

/*! @struct VdecHWMemConfig
@brief (Internal use)
*/
typedef struct _VdecHWMemConfig
{
    uint32 fb_y[VDEC_MAX_FRAME_BUF_NUM];
    uint32 fb_c[VDEC_MAX_FRAME_BUF_NUM];
    uint32 dv_y[VDEC_MAX_FRAME_BUF_NUM];
    uint32 dv_c[VDEC_MAX_FRAME_BUF_NUM];
    uint32 fb_max_stride;
    uint32 fb_max_height;
    uint32 dv_max_stride;
    uint32 dv_max_height;
    uint32 fb_num;
    uint32 neighbor_mem;
    uint32 colocate_mem;
    uint32 cmdq_base;
    uint32 cmdq_len;
    uint32 vbv_start;
    uint32 vbv_len;
} VdecHWMemConfig;

/*! @struct DViewCtrl
@brief Internal use
*/
typedef struct _DViewCtrl
{
    uint8 dv_enable;
    uint8 dv_h_scale_factor;
    uint8 dv_v_scale_factor;
    uint8 interlaced:1;
    uint8 bottom_field_first:1;
}DViewCtrl;

/*! @struct VdecFBConfig
@brief Internal use
*/
typedef struct _VdecFBConfig
{
    uint32 FBStride;
    uint32 FBMaxHeight;
    uint8  *FrmBuf;     //!<Used only for SW_DECODE
    uint8  *DispBuf;    //!<In HW_DISPLAY mode, first 4 bytes store frame buffer address, last 4 byte stor dv buffer address
} VdecFBConfig;

/*! @struct VdecFormatInfo
@brief Describes a compressed or uncompressed video format (Internal use)
*/
typedef struct _VdecFormatInfo
{
    FourCC fourcc;
    int32 bits_per_pixel;
    int32 pic_width;
    int32 pic_height;
    int32 pic_inverted;
    int32 pixel_aspect_x;
    int32 pixel_aspect_y;
    int32 frame_rate;
    int32 frame_period;
    int32 frame_period_const;
} VdecFormatInfo;

/*! @enum VdecEventType
@brief Decore notify event to video player engine (Internal use)
*/
enum VdecEventType
{
    VDEC_EVENT_FRAME_FINISHED,   //!<One frame finished by VE
    VDEC_EVENT_FRAME_OUTPUT,     //!<One frame put into DQ
    VDEC_EVENT_DE_RELEASED,      //!<One frame released by DE
    VDEC_EVENT_DE_REQUESTING,    //!<Requesting frame by DE
    VDEC_EVENT_SLICE_FINISHED,   //!<One slice finished by VE
    VDEC_EVENT_FB_MALLOC_DONE,   //!<Frame buffer malloc done
    VDEC_EVENT_FIELD_PIC_FLAG,   //!<0 -- 1 frame 1 pts; 1 -- 1 field 1 pts
};

//! @typedef VdecDecoreEven
//! @brief Internal use
typedef int32 (* VdecDecoreEven)(enum VdecEventType event, uint32 param);

//! @typedef VdecDecodeFunction
//! @brief Internal use
typedef int32 (VdecDecodeFunction)(void *pHandle, int vdecCmd, void *pParam1, void *pParam2);

//! @typedef DisplayRequest
//! @brief Internal use
typedef int32 (* DisplayRequest)(void *, void *, void *);

//! @typedef DisplayRelease
//! @brief Internal use
typedef int32 (* DisplayRelease)(void *, void *);

/*! @struct VdecInit
@brief Structure with parameters used to instantiate a decoder (Internal use)
*/
typedef struct _VdecInit
{
    VdecFormatInfo fmt_out; //!<Desired output video format.
    VdecFormatInfo fmt_in;  //!<Given input video format
    const VdecFBConfig *pfrm_buf;
    const VdecHWMemConfig *phw_mem_cfg;
    VdecDecoreEven on_decode_event;
    DisplayRequest pfn_decore_de_request;
    DisplayRelease pfn_decore_de_release;
    uint32 decoder_flag;
    uint32 decode_mode;
    uint32 preview;
    uint32 sync_mode;
}VdecInit;

/*! @struct VdecVideoPacket
@brief Structure containing compressed video data (Internal use)
*/
typedef struct _VdecVideoPacket
{
    void *buffer;
    uint32 size;
    uint8 decoded;
}VdecVideoPacket;

/*! @struct VdecFrmBuf
@brief Internal use
*/
typedef struct _VdecFrmBuf
{
    uint8 *buf_addr; //!<In HW_DISPLAY mode, first 4 bytes store frame buffer addr, last 4 byte stor dv buffer addr
    uint8 buf_id;
    uint8 prediction_type;
    DViewCtrl dv_ctrl;
}VdecFrmBuf;

/*! @struct VdecFrameConfig
@brief Structure containing input bitstream and decoder frame buffer (Internal use)
*/
typedef struct _VdecFrameConfig
{
    int32 decode_buf_id;
    int32 decode_finish; //!<Non-zero value means that a frame was successfully decoded
    int32 decode_mode;
    int32 decode_status;
    int64 pts;
    int64 dts;
    VdecVideoPacket video_packet; //!<Input video packet to be decoded.
    DViewCtrl dview_ctrl;
    int64 pos;
}VdecFrameConfig;

/*! @struct VdecFrameInfo
@brief Structure that can be used to get extended information about a decoded frame (Internal use)
*/
typedef struct _VdecFrameInfo
{
    uint32 frame_length;
    uint32 frame_num;
    uint32 vop_coded;

    uint32 stride_y;
    uint32 stride_uv;

    int32 decode_status;
    int32 decode_error;

    uint8 disp_buf_id;
    uint8 recon_buf_id;
    uint8 ref_for_buf_id;
    uint8 ref_back_buf_id;

    uint32 disp_buf_prediction_type;
    uint32 recon_buf_prediction_type;
    uint32 for_buf_prediction_type;
    uint32 bak_buf_prediction_type;

    uint32 cur_frame_pts;   //!<Presentation timestamp of the decoded frame.

    DViewCtrl disp_buf_dv_ctrl;
    DViewCtrl for_buf_dv_ctrl;
    DViewCtrl bak_buf_dv_ctrl;
}VdecFrameInfo;

/*! @enum VdecFrmArrayType
@brief Structure used by display queue management (Internal use)
*/
enum VdecFrmArrayType
{
    VDEC_FRM_ARRAY_MP = 0x01,
    VDEC_FRM_ARRAY_DV = 0x02,

    VDEC_FRM_ARRAY_INVALID = 0xff,
};

/*! @struct OutputFrmManager
@brief (Internal use)
*/
struct OutputFrmManager
{
    uint32  display_index[2];

    enum VdecFrmArrayType de_last_request_frm_array[2];
    enum VdecFrmArrayType de_last_release_frm_array[2];

    uint8  de_last_request_idx[2];
    uint8  de_last_release_idx[2];

    int32 last_output_pic_released[2];
    int32 de_last_release_poc[2];
    int32 de_last_request_poc[2];

    uint8 frm_number;
    uint8 pip_frm_number;
};

/*! @enum vdec_out_pic_type
@brief YUV storage format that video output
*/
enum vdec_out_pic_type
{
    VDEC_PIC_IMC1,    //!<No longer use
    VDEC_PIC_IMC2,    //!<No longer use
    VDEC_PIC_IMC3,    //!<No longer use
    VDEC_PIC_IMC4,    //!<No longer use
    VDEC_PIC_YV12     //!<YU12 storage format
};

/*! @struct vdec_picture
@brief Define parameter of VDECIO_CAPTURE_DISPLAYING_FRAME
*/
struct vdec_picture
{
    enum vdec_out_pic_type type;    //!<Output YUV data format
    uint8 *out_data_buf;            //!<Output YUV data
    uint32 out_data_buf_size;       //!<Size of input buffer
    uint32 out_data_valid_size;     //!<Vaid size of output YUV data
    uint32 pic_width;               //!<Picture width
    uint32 pic_height;              //!<Picture height
};

/*! @struct vdec_decore_status
@brief Video decoder's status in media player mode
*/
struct vdec_decore_status
{
    uint32 decode_status;     //!<Decode state
    uint32 pic_width;         //!<Picture width
    uint32 pic_height;        //!<Picture height   
    uint32 sar_width;         //!<Sample aspect ratio width
    uint32 sar_heigth;        //!<Sample aspect ratio height
    uint32 frame_rate;        //!<Frame rate
    int32 interlaced_frame;   //!<Whether the stream is interlaced
    int32 top_field_first;    //!<Whether the stream is top field first
    int32 first_header_got;   //!<Whether the first header is parsed
    int32 first_pic_showed;   //!<Whether the first picture is displayed
    uint32 frames_decoded;    //!<Number of frames decoded
    uint32 frames_displayed;  //!<Number of frames displayed
    int64 frame_last_pts;     //!<PTS of last displayed frame
    uint32 buffer_size;       //!<Total es buffer size
    uint32 buffer_used;       //!<Used size of es buffer
    uint32 decode_error;      //!<Decoder error type
    uint32 decoder_feature;   //!<Decoder feature
    uint32 under_run_cnt;     //!<Times of under run
    int32 first_pic_decoded;  //!<Whether the first picture is decoded
    int32 output_mode;        //!<Decoder's output mode
    uint32 frame_angle;       //!<Decoder's output frame angle
	int64 first_i_frame_pts;
};

/*! @struct av_config
@brief Configuration of decoder's initialization
*/
struct av_config
{
   int32 av_sync_mode;           //!<A/V synchronization mode
   int32 hold_threshold;         //!<Hold threshold
   int32 drop_threshold;         //!<Drop threshold
   int32 av_sync_delay;          //!<A/V synchronization delay [-500, 500]ms
   int32 av_sync_unit;           //!<A/V synchronization unit
   int32 preview;                //!<Whether in preview
   struct Video_Rect src_rect;   //!<Display rectangle of source
   struct Video_Rect dst_rect;   //!<Display rectangle of destination
   int32 pause_decode;           //!<Whether to pause decode
   int32 pause_output;           //!<Whether to pause display
   int32 enable_output;          //!<Whether to enable decoder's output
   int32 high_threshold;         //!<High threshold of es buffer
   int32 low_threshold;          //!<Low threshold of es buffer
   int32 hold2free_trd;          //!<Hold to free run threshold
   int32 drop2free_trd;          //!<Drop to free run threshold
   uint32 quick_mode;            //!<Whether to enable quick mode
   uint32 decode_mode;           //!<Decode mode
};

/*! @enum av_sync_mode
@brief A/V synchronization mode
*/
enum av_sync_mode
{
    AV_SYNC_NONE,         //!<Don't do sync
    AV_SYNC_AUDIO,        //!<Sync to audio
    AV_SYNC_VIDEO,        //!<Sync to video (Not used temporarily)
    AV_SYNC_EXTERNAL,     //!<Sync to an external clock (Not used temporarily)
    AV_SYNC_AUDIO_N,      //!<Sync to audio, using new av sync module
    AV_SYNC_VIDEO_N,      //!<Sync to video, using new av sync module (Not used temporarily)
    AV_SYNC_EXTERNAL_N,   //!<Sync to external clock, using new av sync module (Not used temporarily)
};

/*! @enum av_picture_type
@brief Picture type
*/
enum av_picture_type
{
    PICTURE_TYPE_I = 1, //!<Intra
    PICTURE_TYPE_P,     //!<Predicted
    PICTURE_TYPE_B,     //!<Bi-dir predicted
    PICTURE_TYPE_S,     //!<S(GMC)-VOP MPEG4
    PICTURE_TYPE_SI,    //!<Switching Intra
    PICTURE_TYPE_SP,    //!<Switching Predicted
    PICTURE_TYPE_BI,    //!<BI type
};

/*! @struct av_panscan
@brief specifies the area which should be displayed (Not used temporarily)
*/
struct av_panscan
{
    int32 id;
    int32 width;
    int32 height;
    int16 position[3][2];    //!<Position of the top left corner in 1/16 pel for up to 3 fields/frames
};

/*! @struct av_rational
@brief Rational number numerator/denominator
*/
struct av_rational
{
    int32 num;    //!<Numerator
    int32 den;    //!<Denominator
};

/*! @struct rotation_rect
@brief (Internal use)
*/
struct rotation_rect
{
    int width;
    int height;
    unsigned char *src_y_addr;
    unsigned char *src_c_addr;
    int angle;
    unsigned char *dst_y_addr;
    unsigned char *dst_c_addr;
};

/*! @struct rotation_updata_info
@brief (Internal use)
*/
struct rotation_updata_info
{
    unsigned int sw_y_addr;
    unsigned int hw_y_addr;
    unsigned int sw_pre_y_addr;
    unsigned int hw_pre_y_addr;
    int disp_info_size;
};

#define AV_PKT_FLAG_CODEC_DATA    0x10000000    //!<The packet contains codec data
#define AV_PKT_FLAG_EOS           0x20000000    //!<The last packet

/*! @enum av_param_change_flags
@brief Define flags for paramter changing
*/
enum av_param_change_flags
{
    AV_PARAM_CHANGE_CHANNEL_COUNT  = 0x0001,    //!<Not used temporarily
    AV_PARAM_CHANGE_CHANNEL_LAYOUT = 0x0002,    //!<Not used temporarily
    AV_PARAM_CHANGE_SAMPLE_RATE    = 0x0004,    //!<Not used temporarily
    AV_PARAM_CHANGE_DIMENSIONS     = 0x0008,    //!<Dimensions change
};

/*! @struct av_packet
@brief Describe A/V packet
*/
struct av_packet
{
    int64 pts;                   //!<Presentation timestamp, can be AV_NOPTS_VALUE if it is not stored in the file
    int64 dts;                   //!<Decompression timestamp, can be AV_NOPTS_VALUE if it is not stored in the file
    uint8 *data;                 //!<A/V packet data
    int32 size;                  //!<A/V packet size
    int32 stream_index;          //!<Not used temporarily
    int32 flags;                 //!<Flag to specify the packet data
    uint16 width;                //!<Picture width
    uint16 height;               //!<Picture height
    int32 param_change_flags;    //!<Flag to specify the change of parameter
    int32 duration;              //!<Duration of this packet (Not used temporarily)
    void  (*destruct)(struct av_packet *);  //!<Not used temporarily
    void  *priv;                 //!<Not used temporarily
    int64 pos;                   //!<Byte position in stream, -1 if unknown (Not used temporarily)
    int64 convergence_duration;  //!<Not used temporarily
};

/*! @struct av_frame
@brief Describe video frame output by decoder
*/
struct av_frame
{
    uint8 *data[4];                 //!<Pointer to the picture planes
    int32 linesize[4];              //!<Size of picture planes
    uint8 *base[4];                 //!<Pointer to the first allocated byte of the picture
    int32 key_frame;                //!<1 -> keyframe, 0-> not
    enum av_picture_type pict_type; //!<Picture type of the frame
    int64 pts;                      //!<Presentation timestamp
    int32 coded_picture_number;     //!<Picture number in bitstream order
    int32 display_picture_number;   //!<Picture number in display order
    int32 quality;                  //!<Not used temporarily
    int32 age;                      //!<Frame rate
    int32 reference;                //!<Is this picture used as reference
    int8 *qscale_table;             //!<Not used temporarily
    int32 qstride;                  //!<Not used temporarily
    uint8 *mbskip_table;            //!<Not used temporarily
    uint16 (*motion_val[2])[2];     //!<Not used temporarily
    uint32 *mb_type;                //!<Not used temporarily
    uint8 motion_subsample_log2;    //!<Not used temporarily
    void *opaque;                   //!<Not used temporarily
    int64 error[4];                 //!<Not used temporarily
    int32 type;                     //!<Not used temporarily
    int32 repeat_pict;              //!<How much the picture must be delayed (Not used temporarily)
    int32 qscale_type;              //!<Not used temporarily
    int32 interlaced_frame;         //!<Whether the picture is interaced
    int32 top_field_first;          //!<Is top field displayed first
    struct av_panscan *pan_scan;    //!<Not used temporarily
    int32 palette_has_changed;      //!<Not used temporarily
    int32 buffer_hints;             //!<Codec suggestion on buffer type if != 0 (Not used temporarily)
    short *dct_coeff;               //!<Not used temporarily
    int8 *ref_index[2];             //!<Not used temporarily
    int64 reordered_opaque;         //!<Not used temporarily
    void *hwaccel_picture_private;  //!<Hardware accelerator private data
    int64 pkt_pts;                  //!<Presentation timestamp
    int64 pkt_dts;                  //!<Decode timestamp
    void *owner;                    //!<Not used temporarily
    void *thread_opaque;            //!<Not used temporarily
    int64 best_effort_timestamp;    //!<Not used temporarily
    int64 pkt_pos;                  //!<Not used temporarily
    struct av_rational sample_aspect_ratio;  //!<Sample aspect ratio of the frame
    int32 width, height;            //!<Width and height of the video frame
    int32 format;                   //!<Not used temporarily
};

/*! @struct vdec_mp_init_param
@brief Define parameter of VDECIO_MP_INITILIZE
*/
struct vdec_mp_init_param
{
    uint32 codec_tag;      //!<Specified decoder's type
    uint32 decode_mode;    //!<Decode mode
    uint32 decoder_flag;   //!<Decode flag
    uint32 preview;        //!<Whether in preview
    uint32 frame_rate;     //!<Frame rate
    int32  pic_width;      //!<Picture width
    int32  pic_height;     //!<Picture height
    int32  pixel_aspect_x; //!<Pixel aspect raio width
    int32  pixel_aspect_y; //!<Pixel aspect raio height
    uint32 dec_buf_addr;   //!<Frame buffer start address
    uint32 dec_buf_size;   //!<Frame buffer total size
};

/*! @struct vdec_mp_rls_param
@brief Define parameter of VDECIO_MP_RELEASE
*/
struct vdec_mp_rls_param
{
    int32 reserved;    //!<Not used temporarily
};

/*! @struct vdec_mp_flush_param
@brief Define parameter of VDECIO_MP_FLUSH
*/
struct vdec_mp_flush_param
{
    uint32 flush_flag;    //!<Setting to 3 if flush to FF/FB and setting to 1 if flush to normal play
};

/*! @struct vdec_mp_extra_data
@brief Define parameter of VDECIO_MP_EXTRA_DATA
*/
struct vdec_mp_extra_data
{
    uint8 *extra_data;      //!<Extra data buffer
    uint32 extra_data_size; //!<Extra data size
};

/*! @struct vdec_mp_pause_param
@brief Define parameter of VDECIO_MP_PAUSE
*/
struct vdec_mp_pause_param
{
    uint8 pause_decode;    //!<Whether to pause decode
    uint8 pause_display;   //!<Whether to pause display
};

/*! @struct vdec_mp_sbm_param
@brief Define parameter of VDECIO_MP_SET_SBM_IDX
*/
struct vdec_mp_sbm_param
{
    uint8 packet_header;   //!<SBM to hold packet header
    uint8 packet_data;     //!<SBM to hold packet data
    uint8 decode_output;   //!<SBM to hold decoder's output frames
    uint8 display_input;   //!<SBM to hold frames to display
};

/*! @struct vdec_mp_sync_param
@brief Define parameter of VDECIO_MP_SET_SYNC_MODE
*/
struct vdec_mp_sync_param
{
    enum av_sync_mode sync_mode;   //!<Synchronization mode
    uint8 sync_unit;               //!<Synchronization uint
};

#ifndef RPC_HLD_INTERNAL

#ifndef PDKAPI_INTERNAL

#define VDEC_IO_FILL_FRM                0x03
#define VDEC_IO_REG_CALLBACK            0x04
#define VDEC_IO_GET_STATUS              0x05
#define VDEC_IO_GET_MODE                0x06
#define VDEC_IO_GET_FRM                 0x07
#define VDEC_IO_WHY_IDLE                0x08
#define VDEC_IO_SMLBUF                  0x09
#define VDEC_IO_GET_FREEBUF             0x0A
#define VDEC_IO_FILLRECT_INBG           0x0B
#define VDEC_IO_PRIORITY_STOP_SML       0x0C
#define VDEC_IO_GET_FRM_ADVANCE         0x0D
#define VDEC_IO_DVIEW_EXTRA_MODE        0x0F
#define VDEC_IO_FILL_FRM2               0x10

#define VDEC_IO_ADPCM_CMD               0x13
#define VDEC_IO_ABS_CMD                 0x0E
#define VDEC_IO_ERR_CONCEAL_CMD         0x0F
#define VDEC_IO_PULLDOWN_ONOFF          0x11
#define VDEC_IO_DVIEW_CMD               0x12

#define VDEC_IO_COLORBAR                0x14
#define VDEC_IO_ENABLE_MPDVIEW          0x15
#define VDEC_IO_GET_VDEC_CONFIG         0x16
#define VDEC_IO_SET_SCALE_MODE          0x17
#define VDEC_IO_ENABLE_PREVIEW_PIP      0x18
#define VDEC_IO_FILL_PREVIEW_PIP        0x19
#define VDEC_IO_REG_SYNC_STATUS         0x1a
#define VDEC_IO_STOP_COPY_TASK          0x20
#define VDEC_IO_GET_OUTPUT_FORMAT       0x21
#define VDEC_IO_MAF_CMD                 0x22
#define VDEC_IO_RESET_VE_HW             0x23
#define VDEC_SET_DMA_CHANNEL            0x24
#define VDEC_IO_SWITCH                  0x25
#define VDEC_IO_GET_FRAME_STATISTIC     0x26
#define VDEC_IO_SET_DVIEW_PRECISION     0x27
#define VDEC_VBV_BUFFER_OVERFLOW_RESET  0x28
#define VDEC_IO_GET_I_FRAME_CNT         0x29
#define VDEC_IO_FIRST_I_FREERUN         0x2A
#define VDEC_IO_DISCARD_HD_SERVICE      0x2B
#define VDEC_IO_DONT_RESET_SEQ_HEADER   0x2C
#define VDEC_IO_SET_DROP_FRM            0x2D
#define VDEC_IO_MULTIVIEW_HWPIP         0x2E
#define VDEC_DTVCC_PARSING_ENABLE       0x2f
#define VDEC_CLOSECAPTION_UDMODE        0x30    // Set user_data mode for  CloseCaption issue, Costa 2008.12.12
#define VDEC_IO_PLAYBACK_PS             0x31    // Playback programe stream
#define VDEC_IO_PLAY_FROM_STORAGE       0x32    // Play stream from storage device
#define VDEC_IO_CHANCHG_STILL_PIC       0x33    // Show black screen or still picture when change channel
#define VDEC_IO_SET_SYNC_DELAY          0x34    // Set picture pts delay john
#define VDEC_IO_SAR_ENABLE              0x35    // Enable/disable sar aspect ratio
#define VDEC_IO_FAST_CHANNEL_CHANGE     0x36
#define VDEC_IO_DROP_FRAME_VBVFULL      0x37
#define VDEC_IO_SEAMLESS_SWITCH_ENABLE  0x38
#define VDEC_IO_PAUSE_VIDEO             0x39
#define VDEC_IO_CONTINUE_ON_ERROR       0x3a    // Continue decoding when error occurs, TRUE/FALSE
#define VDEC_IO_SET_DECODER_MODE        0x40
#define VDEC_IO_SET_FREERUN_THRESHOLD   0x41
#define VDEC_IO_SET_OUTPUT_RECT         0x42
#define VDEC_IO_SET_AVSYNC_GRADUALLY    0x43
#define VDEC_IO_DBLOCK_ENABLE           0x44
#define VDEC_IO_EN_MUTE_FIRST           0x45
#define VDEC_IO_GET_DECORE_STATUS       0x46
#define VDEC_IO_SET_DEC_FRM_TYPE        0x47
#define VDEC_IO_SET_DMA_CHANNEL_NUMBER  0x48
#define VDEC_IO_SET_BOOTLOGO_ADDR       0x49
#define VDEC_IO_SET_SMOOTH_PREVIEW      0x4a
#define VDEC_IO_CLEAN_STILL_FRAME       0x4c

#define VDEC_IO_FILL_BG_VIDEO           0x70
#define VDEC_IO_BG_FILL_BLACK           0x71
#define VDEC_IO_RESERVE_MEMORY          0x72    // Reserve memory for some feature, i.e. capture displaying frame
#define VDEC_IO_GET_MULTIVIEW_BUF       0x73
#define VDEC_IO_SET_MULTIVIEW_WIN       0x74
#define VDEC_IO_SLOW_PLAY_BEFORE_SYNC   0x75
#define VDEC_IO_DONT_KEEP_PRE_FRAME     0x76
#define VDEC_IO_SET_SEND_GOP_GAP        0x77
#define VDEC_IO_SET_SIMPLE_SYNC         0x78
#define VDEC_IO_SET_TRICK_MODE          0x79

#define VDEC_IO_ELE_BASE                        0x10000
#define VDEC_IO_PLAYBACK_STR                    (VDEC_IO_ELE_BASE + 0x01)
#define VDEC_IO_REG_DROP_FUN                    (VDEC_IO_ELE_BASE + 0x02)
#define VDEC_IO_REST_VBV_BUF                    (VDEC_IO_ELE_BASE + 0x03)
#define VDEC_IO_KEEP_INPUT_PATH_INFO            (VDEC_IO_ELE_BASE + 0x04)
#define VDEC_IO_PLAY_MEDIA_STR                  (VDEC_IO_ELE_BASE + 0x05)
#define VDEC_IO_LIVE_STR_MON                    (VDEC_IO_ELE_BASE + 0x06)
#define VDEC_IO_DROP_BEF_FIRT_SHOW              (VDEC_IO_ELE_BASE + 0x07)
#define VDEC_IO_ENABLE_SW_MONITOR               (VDEC_IO_ELE_BASE + 0x08)
#define VDEC_IO_CAPTURE_DISPLAYING_FRAME        (VDEC_IO_ELE_BASE + 0x09)
#define VDEC_IO_PVR_STREAM_INDICATOR            (VDEC_IO_ELE_BASE + 0x0A)
#define VDEC_IO_SHOW_MOSAIC_LEVEL               (VDEC_IO_ELE_BASE + 0x0B)
#define VDEC_IO_REG_GET_SYNC_FLAG_CB            (VDEC_IO_ELE_BASE + 0x0C)
#define VDEC_IO_SET_MODULE_INFO                 (VDEC_IO_ELE_BASE + 0x0D)

/*Container IO Control Command*/
#define CONTAINER_IO_CONTROL_BASE               0xffff
#define CONTAINER_IO_CONTROL_GET_TIME_MS        (CONTAINER_IO_CONTROL_BASE+1)
#define CONTAINER_IO_CONTROL_EN_AC3_BS_MODE     (CONTAINER_IO_CONTROL_BASE+2)
#define CONTAINER_IO_CONTROL_GET_CHAPTER_INFO   (CONTAINER_IO_CONTROL_BASE+3)
#define CONTAINER_IO_CONTROL_GET_TOTALTIME_MS   (CONTAINER_IO_CONTROL_BASE+4)
#define CONTAINER_IO_CONTROL_SET_AVSYNC_DELAY   (CONTAINER_IO_CONTROL_BASE+5)

#endif
#endif

/*! @enum GE_GMA_PIXEL_FORMAT
@brief Internal use
*/
enum GE_GMA_PIXEL_FORMAT
{
    // used by 3602
    GE_GMA_PF_RGB565        = 0x00,
    GE_GMA_PF_RGB888        = 0x01,
    GE_GMA_PF_RGB555        = 0x02,
    GE_GMA_PF_RGB444        = 0x03,
    GE_GMA_PF_ARGB565       = 0x04,
    GE_GMA_PF_ARGB8888      = 0x05,
    GE_GMA_PF_ARGB1555      = 0x06,
    GE_GMA_PF_ARGB4444      = 0x07,
    GE_GMA_PF_CLUT1         = 0x08,
    GE_GMA_PF_CLUT2         = 0x09,
    GE_GMA_PF_CLUT4         = 0x0A,
    GE_GMA_PF_CLUT8         = 0x0B,
    GE_GMA_PF_ACLUT88       = 0x0C,
    GE_GMA_PF_YCBCR888      = 0x10,
    GE_GMA_PF_YCBCR422      = 0x12,
    GE_GMA_PF_YCBCR422MB    = 0x13,
    GE_GMA_PF_YCBCR420MB    = 0x14,
    GE_GMA_PF_AYCBCR8888    = 0x15,
    GE_GMA_PF_A1            = 0x18,
    GE_GMA_PF_A8            = 0x19,
    GE_GMA_PF_CK_CLUT2      = 0x89,
    GE_GMA_PF_CK_CLUT4      = 0x8A,
    GE_GMA_PF_CK_CLUT8      = 0x8B,
    GE_GMA_PF_ABGR1555      = 0x90,
    GE_GMA_PF_ABGR4444      = 0x91,
    GE_GMA_PF_BGR565        = 0x92,
    GE_GMA_PF_ACLUT44       = 0x93,
    GE_GMA_PF_YCBCR444      = 0x94,
    GE_GMA_PF_YCBCR420      = 0x95,
    GE_GMA_PF_MASK_A1       = GE_GMA_PF_A1,
    GE_GMA_PF_MASK_A8       = GE_GMA_PF_A8,
};

/*! @struct gma_pal_attr_t
@brief Internal use
*/
typedef struct _gma_pal_attr_t
{
    unsigned char pal_type;         //!<GE_PAL_RGB or GE_PAL_YCBCR
    unsigned char rgb_order;        //!<enum GE_RGB_ORDER
    unsigned char alpha_range;      //!<enum GE_ALPHA_RANGE
    unsigned char alpha_pol;        //!<enum GE_ALPHA_POLARITY
} gma_pal_attr_t, *pgma_pal_attr_t;
typedef const gma_pal_attr_t *pcgma_pal_attr_t;

/*! @struct gma_scale_param_t
@brief Internal use
*/
typedef struct _gma_scale_param_t
{
    unsigned short tv_sys;
    unsigned short h_div;
    unsigned short v_div;
    unsigned short h_mul;
    unsigned short v_mul;
} gma_scale_param_t, *pgma_scale_param_t;
typedef const gma_scale_param_t *pcgma_scale_param_t;

/*! @struct gma_layer_config_t
@brief Internal use
*/
typedef struct _gma_layer_config_t
{
    unsigned long mem_base;
    unsigned long mem_size;
    unsigned char  hw_layer_id;         //!<Application can switch hw layer id
    unsigned char  color_format;        //!<Default region color format, enum GMA_PIXEL_FORMAT
    unsigned short width, height;       //!<Default region width and height
    unsigned short pixel_pitch;         //!<Default region pixel pitch

    unsigned long bScaleFilterEnable       :1;   //!<Enable/disable GMA scale filter
    unsigned long bP2NScaleInNormalPlay    :1;   //!<Enable/disable PAL/NTSC scale in normal play mode
    unsigned long bP2NScaleInSubtitlePlay  :1;   //!<Enable/disable PAL/NTSC scale in subtitle play mode
    unsigned long bDirectDraw              :1;   //!<For CPU direct draw, no GE draw
    unsigned long bCacheable               :1;   //!<For CPU direct draw, no GE draw
    unsigned long reserved                 :29;  //!<Reserved for future use
} gma_layer_config_t, *pgma_layer_config_t;

/*! @struct alifb_gma_region_t
@brief Internal use
*/
typedef struct _alifb_gma_region_t
{
    unsigned char  color_format;   // enum GMA_PIXEL_FORMAT
    unsigned char  galpha_enable;  // 0 - use color by color alpha; 1 - enable global alpha for this region
    unsigned char  global_alpha;   // If global alpha enable, please set global_alpha [0x00, 0xff]
    unsigned char  pallette_sel;   // pallette index for CLUT4

    unsigned short region_x;       // x offset of the region, from screen top_left pixel
    unsigned short region_y;       // y offset of the region, from screen top_left pixel
    unsigned short region_w;
    unsigned short region_h;

    unsigned long  bitmap_addr;    // 0 - use uMemBase(internal memory) which is set by ge_attach(gma_layer_config_t *);
                                   // bitmap_addr not 0 - use external memory address as region bitmap addr
    unsigned long  pixel_pitch;    // pixel pitch(not byte pitch) for internal memory or bitmap_addr

                                   // ge_create_region(): bitmap_addr and pixel_pitch determines the region bitmap address, total 4 cases:
                                   // Case 1: if bitmap_addr is 0, and pixel_pitch is 0, it will use region_w as pixel_pitch,
                                   //     and region bitmap addr will be allocated from uMemBase dynamically.
                                   // Case 2: if bitmap_addr is 0, and pixel_pitch is not 0, the region bitmap addr will be fixed:
                                   //     uMemBase + (pixel_pitch * bitmap_y + bitmap_x) * byte_per_pixel

                                   // Case 3: if bitmap_addr is not 0, and pixel_pitch is 0, the region bitmap addr will be:
                                   //     bitmap_addr + (bitmap_w * bitmap_y + bitmap_x) * byte_per_pixel
                                   // Case 4: if bitmap_addr is not 0, and pixel_pitch is not 0, the region bitmap addr will be:
                                   //     bitmap_addr + (pixel_pitch * bitmap_y + bitmap_x) * byte_per_pixel

                                   // ge_move_region(): region using internal memory can only change region_x, region_y, pal_sel;
                                   // ge_move_region(): region using external memory can change everyting in ge_region_t;

    unsigned long  bitmap_x;       // x offset from the top_left pixel in bitmap_addr or internal memory
    unsigned long  bitmap_y;       // y offset from the top_left pixel in bitmap_addr or internal memory
    unsigned long  bitmap_w;       // bitmap_w must >= bitmap_x + region_w, both for internal memory or external memory
    unsigned long  bitmap_h;       // bitmap_h must >= bitmap_y + region_h, both for internal memory or external memory
    unsigned long  region_index;
} alifb_gma_region_t, *pgma_gma_region_t;
typedef const alifb_gma_region_t *pcgma_region_t;

/*!
@}
*/

/*!
@}
*/

#endif

