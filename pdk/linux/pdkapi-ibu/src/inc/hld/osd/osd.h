/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: osd.h
 *
 *  Description: This file contains the definition to operate on On
 *               Screen Display device.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2002.11.30  Liu Lan     0.1.000  Initial
 *  2.  2003.3.10   Xianyu Nie  0.1.100  Create new file
 *  3.  2003.3.25   Liu Lan     0.1.200  Merge
 *
 ****************************************************************************/

#ifndef __OSD_H__
#define __OSD_H__

#include <hld/osd/osd_dev.h>

/* Definition for TV System */
#define NTSC    0
#define PAL     1

/* Definition for TV Mode */
#define INTERLACED      0
#define PROGRESSIVE     1

/* Color Look-up Table define */
//#define CLUT1   0
#define CLUT2   0
#define CLUT4   1
#define CLUT8   2

/* Color System define */
#define YUV     0
#define RGB     1

/* Alpha Value definition */
#define OPAQUE          0
#define TRANSLUCENT     0x80
#define TRANSPARENT     0xFF


/* Structure to define the Palette */
struct clut_desc
{
    UINT8 type;         /* Type of color system */
    UINT8 colorMode;    /* Color mode */
    UINT8 *desc;        /* Pointer to a buffer contains CLUT */
};

/* Structure to define the Bitmap */
struct bmp_desc
{
    UINT16 width;       /* Width of the pixmap */
    UINT16 height;      /* Height of the pixmap */
    UINT16 stride;      /* Number of bytes per line in the pixmap */
    UINT8 colorMode;    /* Color mode */
};


#define osd_open(dev, sys, mode) \
      dev->open(dev, sys, mode)
#define osd_close(dev) \
      dev->close(dev)
#define osd_block_create(dev, id, x, y, w, h, c, a) \
      dev->block_create(dev, id, x, y, w, h, c, a)
#define osd_block_reset(dev, id) \
      dev->block_reset(dev, id)
#define osd_block_delete(dev, id) \
      dev->block_delete(dev, id)
#define osd_block_show(dev, id) \
      dev->block_show(dev, id)
#define osd_block_hide(dev, id) \
      dev->block_hide(dev, id)
#define osd_block_set_clut(dev, id, clut) \
      dev->block_set_clut(dev, id, clut)
#define osd_block_draw_pixmap(dev, id, x, y, bmp, data) \
      dev->block_draw_pixmap(dev, id, x, y, bmp, data)


#if __OSDDRV_DESCRIPTION__
/*
    On Screen Display device function list
*/
INT32 osd_open
(
    struct osd_device *dev,     /*I Device pointer */
    UINT8 tv_system,            /*I TV System */
    UINT8 tv_mode               /*I TV Mode */
);

INT32 osd_close
(
    struct osd_device *dev      /*I Device pointer */
);

INT32 osd_block_create
(
    struct osd_device *dev,     /*I Device pointer */
    UINT8 *block_id,            /*O Block ID */
    UINT16 x,                   /*I X coordinate of Upper-Left */
    UINT16 y,                   /*I Y coordinate of Upper-Left */
    UINT16 w,                   /*I Block width */
    UINT16 h,                   /*I Block height */
    UINT8 color_mode,           /*I Color mode */
    UINT8 global_alpha          /*I Alpha blending mix ratio */
);

INT32 osd_block_reset
(
    struct osd_device *dev,     /*I Device pointer */
    UINT8 *block_id             /*I Block ID */
);

INT32 osd_block_delete
(
    struct osd_device *dev,     /*I Device pointer */
    UINT8 *block_id             /*I Block ID */
);

INT32 osd_block_show
(
    struct osd_device *dev,     /*I Device pointer */
    UINT8 *block_id             /*I Block ID */
);

INT32 osd_block_hide
(
    struct osd_device *dev,     /*I Device pointer */
    UINT8 *block_id             /*I Block ID */
);

INT32 osd_block_set_clut
(
    struct osd_device *dev,     /*I Device pointer */
    UINT8 *block_id,            /*I Block ID */
    struct clut_desc *clut      /*I New CLUT's descriptor */
);

INT32 osd_block_draw_pixmap
(
    struct osd_device *dev,     /*I Device pointer */
    UINT8 *block_id,            /*I Block ID */
    INT16 x,                    /*I X coordinate in block */
    INT16 y,                    /*I Y coordinate in block */
    struct bmp_desc *bmp,       /*I New Bitmap's descriptor */
    UINT8 *data                 /*I New bitmap's data */
);
#endif /* __OSDDRV_DESCRIPTION__ */

#endif /* __OSD_H__ */
