/*******************************************************************************

File name   : venc_HwMapping.h

Description : Video encoder driver stack Linux platform OS Hw mapping header file

Author      : Vic Zhang <Vic.Zhang@Alitech.com>

Create date : Oct 20, 2013

COPYRIGHT (C) ALi Corporation 2013

Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 20, 2013                   Created               V0.1             Vic.Zhang

*******************************************************************************/

/* Define to prevent recursive inclusion */

#ifndef __VIDEO_ENCODER_HWMAPPING_H__
#define __VIDEO_ENCODER_HWMAPPING_H__

/* Includes ----------------------------------------------------------------- */



/* C++ support */
#ifdef __cplusplus
extern "C" {
#endif

/* Exported Constants ------------------------------------------------------- */

enum PicMode
{
    PIC_TOP    = 0,
    PIC_BOTTOM = 1,
    PIC_FRAME  = 2,
    NONE,
};

/* Exported Macros ---------------------------------------------------------- */

#define TILE_WIDTH 16      //tile width
#define TILE_HEIGHT 32     //tile height
#define LINE_PER_SCAN 2
#define INTERLACE_NUMBER 2
#define SAMPLES_SCANNED_PER_LINE 16
#define TILE_SIZE (TILE_WIDTH*TILE_HEIGHT)
#define SAMPLES_PER_SCAN (SAMPLES_SCANNED_PER_LINE*LINE_PER_SCAN)
#define SUBTILE_SIZE (INTERLACE_NUMBER*SAMPLES_PER_SCAN)
#define SUBTILES_PER_ROW (TILE_WIDTH/SAMPLES_SCANNED_PER_LINE)
#define LINES_PER_SUBTILE (LINE_PER_SCAN*INTERLACE_NUMBER)


/** Image Dram
* @note Image Dram
*/
struct ImgDram
{
    unsigned int luma_addr;
    unsigned int chroma_addr;
    unsigned int stride; /**< fb_stride, or luma stride */
};

/** Image Resource
* @note Image Resource
*/
struct ImgPlane
{
    unsigned char *buf_y;
    unsigned char *buf_cb;
    unsigned char *buf_cr;
    int pitch_y;
    int pitch_uv;
    int buf_width;
    int buf_height;
    int buf_width_c;
    int buf_height_c;
};

/* Exported Functions ------------------------------------------------------- */

void LoadEncFrmToDram(struct ImgDram *dram_p, enum PicMode encode_target, struct ImgPlane *image);


/* C++ support */
#ifdef __cplusplus
}
#endif

#endif /* #ifndef __VIDEO_ENCODER_HWMAPPING_H__ */

/* End of venc_HwMapping.h */

