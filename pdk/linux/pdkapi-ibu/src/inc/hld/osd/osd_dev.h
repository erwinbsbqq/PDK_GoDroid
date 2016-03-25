/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: osd_dev.h
 *
 *  Description: Define the device structure of On Screen Display
 *               deivce.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2003.3.10   Xianyu Nie  0.1.000  Create File
 *  2.  2003.3.25   Liu Lan     0.1.100  Re-structure
 *
 ****************************************************************************/

#ifndef __OSD_DEV_H__
#define __OSD_DEV_H__

#include <types.h>
#include <api/libbuf/bufman.h>
#include <hld/hld_dev.h>
#include <hld/osd/osd.h>

#define OSD_DEV_INITIALIZED     0
#define OSD_DEV_OPENED          1

struct osd_device
{
    /* Common device structure member */
    struct hld_device *next;            /* link to next device */
    UINT32 type;                        /* Interface hardware type */
    INT8 name[HLD_MAX_NAME_SIZE];       /* Device name */

    /* Device related structure member */
    UINT8 status;                       /* Device status */

    UINT16 max_y;                       /* TV System related */
    struct _tag *osd_mem;               /* For memory management */

    /* Device related functions */
    INT32 (*open)(struct osd_device *, UINT8, UINT8);
    INT32 (*close)(struct osd_device *);
    INT32 (*block_create)(struct osd_device *, UINT8 *, UINT16, \
          UINT16, UINT16, UINT16, UINT8, UINT8);
    INT32 (*block_reset)(struct osd_device *, UINT8);
    INT32 (*block_delete)(struct osd_device *, UINT8);
    INT32 (*block_show)(struct osd_device *, UINT8);
    INT32 (*block_hide)(struct osd_device *, UINT8);
    INT32 (*block_set_clut)(struct osd_device *, UINT8, \
          void *);
    INT32 (*block_draw_pixmap)(struct osd_device *, UINT8, INT16, \
          INT16, void *, UINT8 *);
};

#endif /* __OSD_DEV_H__ */
