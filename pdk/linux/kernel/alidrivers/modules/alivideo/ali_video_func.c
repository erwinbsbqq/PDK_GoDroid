#include "ali_video.h"

static void de_get_1pixel(const unsigned char *Y_base, const unsigned char *C_base, \
                           unsigned int MB_stride, unsigned int l, unsigned int r, \
                           unsigned char *y, unsigned char *cb, unsigned char *cr)
{
    const unsigned char *pY;
    const unsigned char *pC;

	pY = Y_base + (((l >> 4) * MB_stride + (r >> 5)) << 9) \
         + ((l & 0x0F) << 5) + (r & 0x1F);
        
	pC = C_base + (((l >> 5) * MB_stride + (r >> 5)) << 9) \
         //+ ((l & 0x1E) << 4) + (r & 0x1F);
         + ((l & 0x1E) << 4) + (r & 0x1e);

    *y = pY[0];

    if ( (l&1) || (r&1))
        return;

    *cb = pC[0];
    *cr = pC[1];
}

static void yuv_set_1pixel(unsigned char *yuv_y, unsigned char *yuv_u, unsigned char *yuv_v, \
                            int stride_y, int stride_uv, unsigned int l, unsigned int r, \
                            unsigned char y, unsigned char cb, unsigned char cr)
{
    unsigned char *pY;
    unsigned char *pC;

    pY = yuv_y + l * stride_y + r;

    pY[0] = y;

    if ( (l&1) || (r&1))
        return;

    l >>= 1;

	pC = yuv_u + l * stride_uv + (r >> 1);

    pC[0] = cb;

	pC = yuv_v + l * stride_uv + (r >> 1);

    pC[0] = cr;
}

void convert_mpeg2_de2yuv(const unsigned char *Y_Addr, const unsigned char *C_Addr, \
                               unsigned int width, unsigned int height, \
                               unsigned char *yuv_y, unsigned char *yuv_u, unsigned char *yuv_v, \
                               unsigned int stride_y, unsigned int stride_uv)
{
    unsigned int i, j;
    unsigned char y, cb, cr;
    unsigned int MB_stride;

	MB_stride = (width + 31) >> 5;

    for (j = 0; j < height; j++)
    {
        for (i = 0; i < width; i++)
        {
            de_get_1pixel(Y_Addr, C_Addr, MB_stride, j, i, &y, &cb, &cr);
            yuv_set_1pixel(yuv_y, yuv_u, yuv_v, stride_y, stride_uv, j, i, y, cb, cr );
        }
    }

}

void convert_h264_de2yuv(const unsigned char *y_addr, const unsigned char *c_addr, 
                             unsigned int pic_width, unsigned int pic_height, 
                             unsigned char *yuv_y, unsigned char *yuv_u, 
                             unsigned char *yuv_v, unsigned int stride_y, unsigned int stride_uv)
{
    unsigned int l, r, i, j, k;
    unsigned char block_width = 16;
    unsigned char block_height = 32;
    unsigned char *ptr_ybuf = (UINT8 *)y_addr;
    unsigned char *ptr_cbuf = (UINT8 *)c_addr;
    unsigned char *ptr_yline = NULL, *ptr_uline = NULL, *ptr_vline = NULL;
    unsigned char *ptr_u = NULL, *ptr_v = NULL;
    unsigned int y_h_block_num = (stride_y + (block_width - 1)) / block_width;
    unsigned int y_v_block_num = (pic_height + (block_height - 1)) / block_height;
    unsigned int c_h_block_num = (stride_y + (block_width - 1)) / block_width;
    unsigned int c_v_block_num = ((pic_height >> 1) + (block_height - 1)) / block_height;

    for(l = 0; l < y_v_block_num; l++)
    {
        for(r = 0; r < y_h_block_num; r++)
        {
            ptr_yline = yuv_y + pic_width * l * block_height + r * block_width;

            for(j = 0; j < block_height; j+=4)
            {
                for (i = 0; i < 2; i++)
                {
                    if((r + 1) * block_width > pic_width)
                    {
                        ptr_ybuf += (block_width * 2);
                        continue;
                    }
                    
                    memcpy(ptr_yline, ptr_ybuf, block_width);
                    ptr_ybuf += block_width;

                    memcpy(ptr_yline+pic_width*2, ptr_ybuf, block_width);
                    ptr_ybuf += block_width;

                    ptr_yline += pic_width;
                }
                
                ptr_yline += pic_width * 2;
            }
        }
    }

    for(l = 0; l < c_v_block_num; l++)
    {
        for(r = 0; r < c_h_block_num; r++)
        {
            ptr_uline = yuv_u + (pic_width>>1) * l * block_height + r * block_width / 2;
            ptr_vline = yuv_v + (pic_width>>1) * l * block_height + r * block_width / 2;

            for (j = 0; j < block_height; j+=4)
            {
                for (i = 0; i < 2; i++)
                {
                    ptr_u = ptr_uline;
                    ptr_v = ptr_vline;

                    for (k = 0; k < block_width; k+=2)
                    {
                        if(ptr_u >= yuv_v || (r + 1) * block_width > pic_width)
                        {
                            ptr_cbuf += 2;
                            continue;
                        }
                        
                        *ptr_u++ = *ptr_cbuf++;
                        *ptr_v++ = *ptr_cbuf++;
                    }

                    if((r + 1) * block_width <= pic_width)
                    {
                        ptr_u = ptr_uline + (pic_width>>1) * 2;
                        ptr_v = ptr_vline + (pic_width>>1) * 2;
                    }

                    for (k = 0; k < block_width; k+=2)
                    {
                        if(ptr_u >= yuv_v || ((r + 1) * block_width > pic_width))
                        {
                            ptr_cbuf += 2;
                            continue;
                        }
                        
                        *ptr_u++ = *ptr_cbuf++;
                        *ptr_v++ = *ptr_cbuf++;
                    }

                    if((r + 1) * block_width <= pic_width)
                    {
                        ptr_uline += (pic_width>>1);
                        ptr_vline += (pic_width>>1);
                    }
                }

                if((r + 1) * block_width <= pic_width)
                {
                    ptr_uline += (pic_width>>1) * 2;
                    ptr_vline += (pic_width>>1) * 2;
                }
            }
        }
    }
}

