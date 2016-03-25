/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2009 Copyright (C)
 *  (C)
 *  File: alifb.h
 *  (I)
 *  Description: the frame buffer driver header file
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2009.9.8				Sam		Create
 ****************************************************************************/

 #ifndef __DRIVERS_VIDEO_ALIFB_H
#define __DRIVERS_VIDEO_ALIFB_H

#include <alidefinition/adf_gma.h>

#ifndef CONFIG_RPC_HLD_GMA
/* fb create new region */
#define ALI_FB_CREATE_NEW
#endif

#if 0
#define FB_PRF(arg, value...)  \
			{\
				printk("kernel debug : file : %s function : %s line %d\n", __FILE__, __FUNCTION__, __LINE__);\
				printk(arg, ##value);\
				printk("kernel debug done\n\n");\
			}
#else
#define FB_PRF(fmt, args...) \
	    do \
	    { \
		    if(g_fb_dbg_flag) \
		    { \
                printk("[ali_fb]"); \
			    printk(fmt, ##args); \
		    } \
	    }while(0)
#endif

extern unsigned long g_fb_dbg_flag;

extern unsigned long __G_ALI_MM_STILL_FRAME_SIZE;
extern unsigned long __G_ALI_MM_STILL_FRAME_START_ADDR;

#define ALIFB_W8(uAddr, bVal) \
							do{__REG8ALI(uAddr) = (bVal);wmb();}while(0)
#define ALIFB_W16(uAddr, wVal) \
							do{__REG16ALI)(uAddr) = (wVal);wmb();}while(0)
#define ALIFB_W32(uAddr, dwVal) \
							do{__REG32ALI(uAddr) = (dwVal);wmb();}while(0)
#define ALIFB_R8(uAddr) \
							({ \
								volatile unsigned char bVal; \
								bVal = (__REG8ALI)(uAddr)); \
								bVal; \
							})
#define ALIFB_R16(uAddr) \
							({ \
								volatile unsigned short wVal; \
								wVal = (__REG16ALI(uAddr)); \
								wVal; \
							})

#define ALIFB_R32(uAddr) \
							({ \
								volatile unsigned long dwVal; \
								dwVal = (__REG32ALI(uAddr)); \
								dwVal; \
							})


/* GMA header format in memory of m36f, 10 DWORD */
struct alifb_gma_header
{
    /*  DW0 */
    uint32 last_block:1;        // 0 
    uint32 alpha_close:1;       // 1     if ycbcr == 0, set alpha to 0
    uint32 scale_on:1;          // 2     0 - scale off, 1 - scale on
    uint32 ep_on:1;             // 3     0 - edge preserve off, 1 - edge preserve on
    uint32 gma_mode:4;          // 7:4   gma color format
    uint32 color_by_color:1;    // 8   /* only available for M3202C */
    uint32 clut_mode:1;         // 9     0 - PIO mode only for debug, 1 - DMA mode
    uint32 clut_update:1;       // 10
    uint32 pre_mul:1;           // 11    1 - GMA do pre-multiplied on RGB
    uint32 csc_mode:1;          // 12    0 - BT601, 1 - BT709
    uint32 rsved2:3;            // 15:13
    uint32 ep_reduce_thr:7;     // 22:16 value[0,127], default 32
    uint32 rsved3:1;            // 23
    uint32 ep_avg_thr:8;        // 31:24 value[0,255], default 170

    /*  DW1 */
    uint32 clut_base;           // 31:5  /* CLUT memory base address. 32Byte aligned */

    /*  DW2 */
    uint32 start_x:11;          // 10:0  /* start x position on screen */
    uint32 rsved4:5;            // 15:11
    uint32 end_x:11;            // 26:16    /* end x position on screen */
    uint32 rsved5:5;            // 31:27

    /*  DW3 */
    uint32 start_y:11;          // 10:0  /* start y position on screen */
    uint32 rsved6:5;            // 15:11
    uint32 end_y:11;            // 26:16    /* end y position on screen */
    uint32 rsved7:5;            // 31:27

    /*  DW4 */
    uint32 source_width:11;     // 10:0  /* source width in pixel */
    uint32 rsved8:5;            // 15:11
    uint32 source_height:11;    // 10:0  /* source height in pixel */
    uint32 rsved9:5;            // 31:27

    /*  DW5 */
    uint32 global_alpha:8;      // 7:0   /* only available for M3202C */
    uint32 clut_segment:4;      // 11:8  /* color pallette segment selection */
    uint32 rgb_order:2;         // 13:12 /* both for CLUT and true color mode */
    uint32 rsved11:2;           // 15:14
    uint32 pitch:14;            // 29:16 /* the number of bytes between the conjoint pixel */
    uint32 rsved12:2;           // 31:30

    /*  DW6 */
    uint32 next_head;           // 31:5 /* address fo next block's header, 32Byte aligned */

    /*  DW7 */
    uint32 bitmap_addr;         // 31:0 aligned on 1 pixel

    /*  DW8 */
    uint32 scale_mode:1;        // 0    0 - filter mode, default; 1 - duplicate mode, only for debug
    uint32 rsved13:31;

    /*  DW9 */
    uint32 incr_h_fra:12;       // 11:0  /* hor scale increment frament part */
    uint32 incr_h_int:4;        // 15:12 /* hor scale increment integer part */
    uint32 incr_v_fra:12;       // 27:16 /* vertical scale increment frament part */
    uint32 incr_v_int:4;        // 31:28 /* vertical scale increment integer part */
};

#define ALIFB_GMA_FLAG_SCALE_PENDING			0x00000001
#define ALIFB_GMA_FLAG_RECT_PENDING			0x00000002
#define ALIFB_GMA_FLAG_BUF_PENDING			0x00000004

/* private information about the GMA */
struct alifb_gma_info{
	int layer_id;
	int region_id;
	int valid;

	uint32 pending_flag;

	int glob_alpha_en;
	uint32 glob_alpha; 

	uint32 data_buffer;
	uint32 dis_format;
	int bpp; // bytes per pixel
	int pitch;
	int x;
	int y;
	int w;
	int h;
	int width;
       int height;

	/* scale parameters */
	enum ALIFB_GMA_SCALE_MODE scale_mode;
	int scale_dir;
	int ep_on;
	uint32 h_scale_param;
	uint32 v_scale_param;
	int tv_sys;
	int h_dst;
	int h_src;
	int v_dst;
	int v_src;

       unsigned long u_scale_cmd;
       unsigned long uScaleParam; 
    
       uint8   pallette_sel;   // pallette index for CLUT4
       uint32  bitmap_addr;    // 0 - use uMemBase(internal memory) which is set by ge_attach(gma_layer_config_t *);
	                        // bitmap_addr not 0 - use external memory address as region bitmap addr
	uint32  bitmap_x;       // x offset from the top_left pixel in bitmap_addr or internal memory
	uint32  bitmap_y;       // y offset from the top_left pixel in bitmap_addr or internal memory
	uint32  bitmap_w;       // bitmap_w must >= bitmap_x + region_w, both for internal memory or external memory
	uint32  bitmap_h;       // bitmap_h must >= bitmap_y + region_h, both for internal memory or external memory

	/* hardware gma header pointer */
	struct alifb_gma_header *gma_header;

};

#define ALIFB_ACCEL_CMD_BUF_SIZE			0x400	

struct alifb_accel_info{
	uint32 cmd_buf_start_addr;
	uint32 cmd_buf_start;
	int cmd_buf_size;

	struct semaphore sem;
};

struct alifb_info {
	struct device *dev;

	/*fbinfo handle */
	struct fb_info *fbinfo;
	
	volatile unsigned int palette_pending;
	int pallet_seg;
	/* keep these registers in case we need to re-write palette */
	void *palette_buffer;
	int pallete_size;
	enum ALIFB_PLT_ALPHA_LEVEL plt_alpha_level;
	enum ALIFB_PLT_TYPE plt_type;
	
	enum ALIFB_DE_LAYER DE_layer;
	uint32 dis_format;
	
	/* GMA info */
	unsigned char rgb_order;        // enum GE_RGB_ORDER
	unsigned char alpha_range;      // enum GE_ALPHA_RANGE
	unsigned char alpha_pol;        // enum GE_ALPHA_POLARITY
	int standard_gma_region_init;
	int gma_init;
	void *gma_buf_start_addr;
	void *gma_buf_start;
	int gma_buf_size;
	int gma_layer_id;
	int cur_region_id;
	struct alifb_gma_info gma_info[VIDEO_MAX_GMA_REGION_NUM];
	void *first_gma_header;	
	int virtual_gma_enable;
	struct alifb_gma_info virtual_gma;
	enum ALIFB_GMA_SCALE_MODE h_gma_scale_mode;
	enum ALIFB_GMA_SCALE_MODE v_gma_scale_mode;
	int scale_para_pending;

	/* accel info */
	int accel_flag;	
	struct alifb_accel_info accel_info;

	int id;

	int open_cnt;
};
 #endif
