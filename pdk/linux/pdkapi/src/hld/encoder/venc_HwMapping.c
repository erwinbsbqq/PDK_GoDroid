/*******************************************************************************

File name   : venc_HwMapping.c

Description : Video encoder driver stack Linux platform OS Hw mapping source file

Author      : Vic Zhang <Vic.Zhang@Alitech.com>

Create date : Oct 20, 2013

COPYRIGHT (C) ALi Corporation 2013

Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 20, 2013                   Created               V0.1             Vic.Zhang

*******************************************************************************/

/* Includes ----------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>


#include "venc_HwMapping.h"


int twist_buf_size = 0;
unsigned char *twist_buf = NULL;


/**
	\brief tile coordinate

	Tile coordinate define a 3D space.
	The horizontal axis x scan tiles from left to right start from 0.
	The vertical axis y scan tiles from top to bottom start from 0.
	Offset as the 3rd dimension to locate a sample within tile, the 1st sample is indexed by 0.
 */
struct TileCoordinate
{
    int x;
    int y;
    int offset;
};

/* Functions ---------------------------------------------------------------- */


/*******************************************************************************
Name        : transform_to_tile
Description : Handle the processes to transform all tiles.
Parameters  :
Assumptions :
Limitations :
Returns     : void value
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 20, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 20, 2013                  Created               V0.1             Vic.Zhang
*******************************************************************************/
static struct TileCoordinate transform_to_tile(int x, int y)
{
    struct TileCoordinate tc;
    int local_x, local_y;
    struct TileCoordinate subtile;
    int sub_x, sub_y;
    int line_id_in_scan;
    int scan_id;
    int prologue_samples;
    int subtile_id;

    tc.x = x / TILE_WIDTH;
    tc.y = y / TILE_HEIGHT;

    // local raster (x, y) coordinate within tile
    local_x = x % TILE_WIDTH;
    local_y = y % TILE_HEIGHT;

    subtile.x = local_x / SAMPLES_SCANNED_PER_LINE;
    subtile.y = local_y / LINES_PER_SUBTILE;
    // local raster (x,y) coordinate within subtile
    sub_x = local_x % SAMPLES_SCANNED_PER_LINE;
    sub_y = local_y % LINES_PER_SUBTILE;

    line_id_in_scan = sub_y / INTERLACE_NUMBER;
    scan_id         = sub_y % INTERLACE_NUMBER;

    //VENC_PRINTF("local (%d, %d)\n", local_x, local_y);
    //VENC_PRINTF("subtile (%d, %d), sub (%d, %d)\n", subtile.x, subtile.y, sub_x, sub_y);
    //VENC_PRINTF("scan id=%d, line=%d\n", scan_id, line_id_in_scan);

    prologue_samples = scan_id * SAMPLES_PER_SCAN;
    subtile.offset = prologue_samples + line_id_in_scan * SAMPLES_SCANNED_PER_LINE + sub_x;

    subtile_id = subtile.y * SUBTILES_PER_ROW + subtile.x;
    tc.offset = subtile_id * SUBTILE_SIZE + subtile.offset;
    return tc;
}

/*******************************************************************************
Name        : bitblt
Description : Handle the processes to transform all tiles.
Parameters  :
Assumptions :
Limitations :
Returns     : void value
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 20, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 20, 2013                  Created               V0.1             Vic.Zhang
*******************************************************************************/
static void bitblt(unsigned char *dst, int dst_pitch, const unsigned char *src, int src_pitch, int nline, int samples_per_line)
{
    int line;

    for (line = 0; line < nline; line++)
    {
        memcpy(dst, src, samples_per_line);
        dst += dst_pitch;
        src += src_pitch;
    }
}


/*******************************************************************************
Name        : copy_line_to_tile
Description : Handle the processes to transform all tiles.
Parameters  :
Assumptions :
Limitations :
Returns     : void value
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 20, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 20, 2013                  Created               V0.1             Vic.Zhang
*******************************************************************************/
static void copy_line_to_tile(unsigned char *tile_buf, int tile_pitch,
                              const unsigned char *line_buf, int line_index, int nsample)
{
    struct TileCoordinate tc = transform_to_tile(0, line_index);
    unsigned char *t = tile_buf + TILE_SIZE * tile_pitch * tc.y + tc.offset;
    const unsigned char *s = line_buf;
    int nsubtile = TILE_WIDTH / SAMPLES_SCANNED_PER_LINE;
    int tile_span = nsample / TILE_WIDTH;
    int i;
    int nsample_left;

    //assert(tc.x == 0);
    for (i = 0; i < nsubtile; i++)
    {
        bitblt(t, TILE_SIZE, s, TILE_WIDTH, tile_span, SAMPLES_SCANNED_PER_LINE);
        t += SUBTILE_SIZE;
        s += SAMPLES_SCANNED_PER_LINE;
    }

    nsample_left = nsample % TILE_WIDTH;
    // number of sample left not enough for a tile, degrade to low performance algorithm
    while (nsample_left > 0)
    {
        int x = nsample - nsample_left;
        struct TileCoordinate tc = transform_to_tile(x, line_index);
        unsigned char *t = tile_buf + TILE_SIZE * (tile_pitch * tc.y + tc.x) + tc.offset;
        int size_to_copy = nsample_left > SAMPLES_SCANNED_PER_LINE ? SAMPLES_SCANNED_PER_LINE : nsample_left;

	memcpy(t, line_buf + x, size_to_copy);
        nsample_left -= size_to_copy;
    }
}


/*******************************************************************************
Name        : TileLuma
Description : Handle the processes to transform all tiles.
Parameters  :
Assumptions :
Limitations :
Returns     : void value
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 20, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 20, 2013                  Created               V0.1             Vic.Zhang
*******************************************************************************/
static void TileLuma(struct ImgDram dram, enum PicMode encoding_target, struct ImgPlane *image)
{
    unsigned char *luma_tile = (unsigned char *) dram.luma_addr; // mips
    unsigned char *buf_y = image->buf_y;
    int luma_tile_pitch = dram.stride;
    int buf_width = image->buf_width;
    int buf_height = image->buf_height;
    int pitch = image->pitch_y;
    int step = encoding_target == PIC_FRAME ? 1 : 2;
    int init_row = encoding_target == PIC_BOTTOM ? 1 : 0;
    int nrow = buf_height * step;
    int row;

    for (row = init_row; row < nrow; row += step)
    {
        copy_line_to_tile(luma_tile, luma_tile_pitch, buf_y, row, buf_width);
        buf_y += pitch;
    }
}


/*******************************************************************************
Name        : mux_lines_to_tile
Description : Handle the processes to transform all tiles.
Parameters  :
Assumptions :
Limitations :
Returns     : void value
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 20, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 20, 2013                  Created               V0.1             Vic.Zhang
*******************************************************************************/
static void mux_lines_to_tile(unsigned char *tile_buf, int tile_pitch,
                              const unsigned char *u, const unsigned char *v, int line_index, int nsample)
{
    int nbyte = nsample * 2;
    int i;
    unsigned char *p;
    if (nbyte > twist_buf_size)
    {
        if (twist_buf)
        {
            //free(twist_buf);
            free(twist_buf);
        }
        //VENC_PRINTF("twist_buf\n");
        //(unsigned char *) malloc(nbyte);
        twist_buf = (unsigned char *)malloc(nbyte);

        if (NULL == twist_buf)
        {
            return;
        }
        twist_buf_size = nbyte;
    }
    p = twist_buf;
    for (i = 0; i < nsample; i++)
    {
        *p++ = *u++;
        *p++ = *v++;
    }
    copy_line_to_tile(tile_buf, tile_pitch, twist_buf, line_index, nbyte);
}


/*******************************************************************************
Name        : TileChroma
Description : Handle the processes to transform all tiles.
Parameters  :
Assumptions :
Limitations :
Returns     : void value
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 20, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 20, 2013                  Created               V0.1             Vic.Zhang
*******************************************************************************/
static void TileChroma(struct ImgDram dram, enum PicMode encoding_target, struct ImgPlane *image)
{
    unsigned char *chroma_tile = (unsigned char *) dram.chroma_addr; // mips
    unsigned char *buf_cb = image->buf_cb;
    unsigned char *buf_cr = image->buf_cr;
    int buf_width = image->buf_width_c;
    int buf_height = image->buf_height_c;
    int pitch = image->pitch_uv;
    int chroma_tile_picth = dram.stride;
    int step = encoding_target == PIC_FRAME ? 1 : 2;
    int init_row = encoding_target == PIC_BOTTOM ? 1 : 0;
    int nrow = buf_height * step;
    int row;

    for (row = init_row; row < nrow; row += step)
    {
        mux_lines_to_tile(chroma_tile, chroma_tile_picth, buf_cb, buf_cr, row, buf_width);
        buf_cb += pitch;
        buf_cr += pitch;
    }
}


/*******************************************************************************
Name        : LoadEncFrmToDram
Description : Handle the processes to transform all tiles.
Parameters  :
Assumptions :
Limitations :
Returns     : void value
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 20, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 20, 2013                  Created               V0.1             Vic.Zhang
*******************************************************************************/
void LoadEncFrmToDram(struct ImgDram *dram_p, enum PicMode encode_target, struct ImgPlane *image)
{
    // always do, for reference pictures
    TileLuma(*dram_p, encode_target, image);
    TileChroma(*dram_p, encode_target, image);
}

