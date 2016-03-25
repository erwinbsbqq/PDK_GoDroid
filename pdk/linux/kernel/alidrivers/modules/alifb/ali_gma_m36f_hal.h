#ifndef __ALI_GMA_M36_HAL_H__
#define __ALI_GMA_M36_HAL_H__

#define S3602F_GMA_REG_BASE       0xb8006100

#define VPOST_MIX_CTRL              0x0EC

#define VPOST_GMA_F_CTRL            0x200
#define VPOST_DMBA_GMA_F            0x204
#define VPOST_GMA_F_K               0x208
#define VPOST_GMA_F_CP_INDEX        0x20c
#define VPOST_GMA_F_CP              0x210
#define VPOST_GMA_F_H_COFF_INDEX    0x214
#define VPOST_GMA_F_H_COFF_DMSB     0x218
#define VPOST_GMA_F_H_COFF_DLSB     0x21c
#define VPOST_GMA_F_V_COFF_INDEX    0x220
#define VPOST_GMA_F_V_COFF          0x224

#define VPOST_GMA_S_CTRL            0x280
#define VPOST_DMBA_GMA_S            0x284
#define VPOST_GMA_S_K               0x288
#define VPOST_GMA_S_CP_INDEX        0x28c
#define VPOST_GMA_S_CP              0x290
#define VPOST_GMA_S_H_COFF_INDEX    0x294
#define VPOST_GMA_S_H_COFF_DMSB     0x298
#define VPOST_GMA_S_H_COFF_DLSB     0x29c
#define VPOST_GMA_S_V_COFF_INDEX    0x2a0
#define VPOST_GMA_S_V_COFF          0x2a4

#define VPOST_GMA_LINEBUFFER_CTR          0x2b8 //line buffer control

#define M3202C_GMA_REG_BASE         0xb8004100

#define M32_VPOST_MP_CTRL           0x90
#define M32_VPOST_SRC_FILED_MODE    0x94
#define M32_VPOST_GLOBAL_K          0x98
#define M32_VPOST_REQ_PRI_PN        0x9c
#define M32_VPOST_GMA_CTRL          0xa0

#define M32_VPOST_OSD_DMBA          0xa4
#define M32_VPOST_GMA_CP_READ       0xa8
#define M32_VPOST_OSD_CP_DATA       0xac
#define M32_VPOST_OSD_X             0xb0
#define M32_VPOST_OSD_Y             0xb4
#define M32_VPOST_OSD_FILTER        0xb8

#define M32_VPOST_ST_RLE_IO_INDEX   0xbc
#define M32_VPOST_ST_RLE_IO_DATA    0xc0

#define M32_VPOST_SUBT_DMBA         0xc4
#define M32_VPOST_SUBT_CP_DATA      0xc8
#define M32_VPOST_CHROMA_CLIP       0xcc
#define M32_VPOST_SUBT_X            0xd0
#define M32_VPOST_SUBT_Y            0xd4
#define M32_VPOST_SUBT_FILTER       0xec

#define VPOST_DMBA_GMA_F_HW         0xa04
#define VPOST_DMBA_GMA_S_HW         0xa84

/* GMA header format in memory, 10 DWORD */
typedef struct
{
    /*  DW0 */
    unsigned int last_block:1;        // 0 
    unsigned int alpha_close:1;       // 1     if ycbcr == 0, set alpha to 0
    unsigned int scale_on:1;          // 2     0 - scale off, 1 - scale on
    unsigned int ep_on:1;             // 3     0 - edge preserve off, 1 - edge preserve on
    unsigned int gma_mode:4;          // 7:4   gma color format
    unsigned int color_by_color:1;    // 8   /* only available for M3202C */
    unsigned int clut_mode:1;         // 9     0 - PIO mode only for debug, 1 - DMA mode
    unsigned int clut_update:1;       // 10
    unsigned int pre_mul:1;           // 11    1 - GMA do pre-multiplied on RGB
    unsigned int csc_mode:1;          // 12    0 - BT601, 1 - BT709
    unsigned int rsved2:3;            // 15:13
    unsigned int ep_reduce_thr:7;     // 22:16 value[0,127], default 32
    unsigned int rsved3:1;            // 23
    unsigned int ep_avg_thr:8;        // 31:24 value[0,255], default 170

    /*  DW1 */
    unsigned int clut_base;           // 31:5  /* CLUT memory base address. 32Byte aligned */


    /*  DW2 */
    unsigned int start_x:11;          // 10:0  /* start x position on screen */
    unsigned int rsved4:5;            // 15:11
    unsigned int end_x:11;            // 26:16    /* end x position on screen */
    unsigned int rsved5:5;            // 31:27

    /*  DW3 */
    unsigned int start_y:11;          // 10:0  /* start y position on screen */
    unsigned int rsved6:5;            // 15:11
    unsigned int end_y:11;            // 26:16    /* end y position on screen */
    unsigned int rsved7:5;            // 31:27

    /*  DW4 */
    unsigned int source_width:11;     // 10:0  /* source width in pixel */
    unsigned int rsved8:5;            // 15:11
    unsigned int source_height:11;    // 10:0  /* source height in pixel */
    unsigned int rsved9:5;            // 31:27

    /*  DW5 */
    unsigned int global_alpha:8;      // 7:0   /* only available for M3202C */
    unsigned int clut_segment:4;      // 11:8  /* color pallette segment selection */
    unsigned int rgb_order:2;         // 13:12 /* both for CLUT and true color mode */
    unsigned int rsved11:2;           // 15:14
    unsigned int pitch:14;            // 29:16 /* the number of bytes between the conjoint pixel */
    unsigned int rsved12:2;           // 31:30

    /*  DW6 */
    unsigned int next_head;           // 31:5 /* address fo next block's header, 32Byte aligned */

    /*  DW7 */
    unsigned int bitmap_addr;         // 31:0 aligned on 1 pixel

    /*  DW8 */
    unsigned int scale_mode:1;        // 0    0 - filter mode, default; 1 - duplicate mode, only for debug
    unsigned int rsved13:2;
    unsigned int filter_select:1;        // 0 -2 tap;1 - 3 tap
    unsigned int rsved14:28;

    /*  DW9 */
    unsigned int incr_h_fra:12;       // 11:0  /* hor scale increment frament part */
    unsigned int incr_h_int:4;        // 15:12 /* hor scale increment integer part */
    unsigned int incr_v_fra:12;       // 27:16 /* vertical scale increment frament part */
    unsigned int incr_v_int:4;        // 31:28 /* vertical scale increment integer part */
}
gma_head_m36f_t;
#endif /* __GMA_M36_HAL_H__ */
