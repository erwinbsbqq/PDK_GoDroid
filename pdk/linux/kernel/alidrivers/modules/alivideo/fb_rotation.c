/*********************************************************************
*
* Copyright (C), 1988-2013,Ali Corp. All rights reserved.
*
* Filename:       fb_rotation.c
* Description:    frame buffer rotation function
* Author:         Jianhua <Jianhua.Hu@Alitech.com>
* Create date:    2013/8/2
*
* Revision history:
* --Revision----Author---------Date----------Reason----------
*   1.0         Jianhua Hu    2013/8/2       Create
*
***********************************************************************/

#if defined(CONFIG_ALI_CHIP_M3921)


#else

#include "fb_rotation.h" 

#define prefetch_read(addr)       __builtin_prefetch(addr,0,0)
#define prefetch_write(addr)      __builtin_prefetch(addr,1,0)
#define SRC_MB_TITE_WIDTH 32
#define SRC_MB_TITE_HEIGHT 16

void mb_rotation(unsigned char *mb_in, unsigned char *mb_out)
{
    int hor = 0;
    int ver = 0;
    unsigned int t0, t1, t2, t3;
    unsigned int r0, r1, r2, r3;
    unsigned int *mb_in_dw = (unsigned int*)mb_in;
    unsigned int *mb_out_dw = (unsigned int*)mb_out;
    unsigned int *p_in = (unsigned int*)mb_in;
    unsigned int *p_out= (unsigned int*)mb_out;

    for(ver=0; ver<SRC_MB_TITE_HEIGHT; ver+=4)
    {
        p_in = mb_in_dw + ((ver*SRC_MB_TITE_WIDTH)>>2);
        prefetch_read(p_in + 32);
        prefetch_read(p_in + 40);
        prefetch_read(p_in + 48);
        prefetch_read(p_in + 56);
        p_out = mb_out_dw + ((SRC_MB_TITE_HEIGHT - ver - 1)>>2);
        for(hor=0; hor<SRC_MB_TITE_WIDTH; hor+=4)
        {
            t0 = p_in[0];
            t1 = p_in[8];
            t2 = p_in[16];
            t3 = p_in[24];
            p_in++;
            r0 =((t0&0xff)<<24)|((t1&0xff)<<16)|((t2&0xff)<<8)|((t3&0xff));
            t0>>=8; t1>>=8; t2>>=8; t3>>=8;
            r1 =((t0&0xff)<<24)|((t1&0xff)<<16)|((t2&0xff)<<8)|((t3&0xff));
            t0>>=8; t1>>=8; t2>>=8; t3>>=8;
            r2 =((t0&0xff)<<24)|((t1&0xff)<<16)|((t2&0xff)<<8)|((t3&0xff));
            t0>>=8; t1>>=8; t2>>=8; t3>>=8;
            r3 =((t0&0xff)<<24)|((t1&0xff)<<16)|((t2&0xff)<<8)|((t3&0xff));
            p_out[0] = r0;
            p_out[4] = r2;
            p_out[8] = r1;
            p_out[12] = r3;
            p_out+=SRC_MB_TITE_HEIGHT;
        }
    }
}


void mb_c_rotation(unsigned char *mb_in,unsigned char *mb_out)
{
    //mapping 32X8 to 16X16
    int hor =0;
    int ver =0;
    unsigned int t0, t1, t2, t3;
    unsigned int r0, r1, r2, r3;
    unsigned int *p_word_in = (unsigned int*)mb_in;
    unsigned int *p_word_out = (unsigned int*)mb_out;
    unsigned int *p_in = (unsigned int*)mb_in;
    unsigned int *p_out= (unsigned int*)mb_out;
    //cb cr as a unit
    for(ver=0;ver<8;ver=ver+2)
    {
        p_in = p_word_in + ver*8;
        prefetch_read(p_in + 16);
        prefetch_read(p_in + 24);
        p_out = p_word_out + 3 - (ver/2);
        for(hor=0;hor<32;hor= hor+8)
        {
            t0 = p_in[0];
            t1 = p_in[1];
            t2 = p_in[8];
            t3 = p_in[9];
            p_in = p_in + 2;
            r0 = (((t0&0xffff)<<16) | (t2&0xffff));
            t0>>=16;  t2>>=16;
            r1 = (((t0&0xffff)<<16) | (t2&0xffff));
            r2 = (((t1&0xffff)<<16) | (t3&0xffff));
            t1>>=16;  t3>>=16;
            r3 = (((t1&0xffff)<<16) | (t3&0xffff));
            p_out[0] = r0;
            p_out[4] = r2;
            p_out[8] = r1;
            p_out[12] = r3;
            p_out+=16;
        }
    }
}


void rotation_90(unsigned char *fb_in_y, unsigned char *fb_in_c,
                 unsigned char *fb_out_y, unsigned char *fb_out_c,
                 int pic_width, int pic_height)
{
    unsigned char *p_mb_in;
    unsigned char *p_mb_out;
    unsigned int hor_mb_num =0;
    unsigned int ver_mb_num =0;
    unsigned int mb_num =0;
    unsigned int hor_num =0;
    unsigned int ver_num =0;
    unsigned int new_pic_width = 0;
    unsigned int new_pic_height = 0;
    unsigned int dst_hor_pos =0;
    unsigned int src_ver_pos =0;
    unsigned int src_c_top_ver_pos = 0;
    unsigned int src_c_bottom_ver_pos = 0;
    unsigned int dst_c_top_hor_pos = 0;
    unsigned int dst_c_bottom_hor_pos = 0;
    unsigned int dst_c_left_top_hor_pos = 0;
    unsigned int dst_c_left_bottom_hor_pos = 0;
    unsigned int src_hor_pos =0;
    unsigned int hor_num_div2 =0;
    unsigned int dst_c_ver_offset = 0;

    new_pic_width = pic_height;
    new_pic_height = pic_width;
    hor_mb_num =(pic_width+SRC_MB_TITE_WIDTH-1) / SRC_MB_TITE_WIDTH;
    ver_mb_num =(pic_height+SRC_MB_TITE_HEIGHT-1) / SRC_MB_TITE_HEIGHT;
    mb_num = hor_mb_num*ver_mb_num;
    //y
    for(ver_num=0; ver_num<ver_mb_num; ver_num++)
    {
        src_ver_pos = ver_num*pic_width*SRC_MB_TITE_HEIGHT;
        dst_hor_pos = ver_mb_num - ver_num - 1;
        for(hor_num=0; hor_num<hor_mb_num; hor_num++)
        {
            p_mb_in = fb_in_y + src_ver_pos + (hor_num<<9);
            //hor->ver ver->hor
            p_mb_out = fb_out_y + hor_num*new_pic_width*SRC_MB_TITE_WIDTH + (dst_hor_pos<<9); //tbd
            //get 32X16 mapping mb data rotion to 16X32 mapping
            mb_rotation(p_mb_in, p_mb_out);
        }
    }
    //c
    ver_mb_num = ver_mb_num>>1;
    new_pic_width = pic_height;
    for(ver_num=0;ver_num<ver_mb_num;ver_num++)
    {
        src_c_top_ver_pos = ver_num*SRC_MB_TITE_HEIGHT*pic_width;
        src_c_bottom_ver_pos = src_c_top_ver_pos + 256;
        dst_c_top_hor_pos = (((ver_mb_num<<1) - (ver_num<<1))<<9) - 512;//((ver_mb_num<<1) - (ver_num<<1) -1)*512;
        dst_c_bottom_hor_pos = dst_c_top_hor_pos + 256;
        dst_c_left_top_hor_pos = dst_c_top_hor_pos - 512;
        dst_c_left_bottom_hor_pos =  dst_c_top_hor_pos  - 256; //dst_c_left_top_hor_pos + 256
        for(hor_num=0; hor_num<hor_mb_num; hor_num++)  //one time deal src one mapping,32X16, then mapping to dst part of two mapping unit
        {
            src_hor_pos = hor_num<<9;
            hor_num_div2 = hor_num>>1;
            dst_c_ver_offset = hor_num_div2*new_pic_width*SRC_MB_TITE_WIDTH;
            //top mapping top 32X8 -> dst 16X16
            p_mb_in = fb_in_c + src_c_top_ver_pos + src_hor_pos;// //get top 32X8
            if((hor_num&0x1) == 0) //maping the dst 16x32 top 16X16
            {
                p_mb_out = fb_out_c + dst_c_ver_offset + dst_c_top_hor_pos;
            }
            else  //maping the dst 16x32 bottom 16X16
            {
                p_mb_out = fb_out_c + dst_c_ver_offset + dst_c_bottom_hor_pos;
            }
            //get 32X16 mapping mb data to mb_in rotion to 16X32 mapping
            mb_c_rotation(p_mb_in,p_mb_out);
            //bottom mapping top 32X8 -> dst 16X16
            p_mb_in = fb_in_c + src_c_bottom_ver_pos + src_hor_pos;  //get bottom 32X8
            if((hor_num&0x1) == 0) //maping the dst 16x32 top 16X16
            {
                p_mb_out = fb_out_c + dst_c_ver_offset + dst_c_left_top_hor_pos;
            }
            else  //maping the dst 16x32 bottom 16X16
            {
                p_mb_out = fb_out_c + dst_c_ver_offset + dst_c_left_bottom_hor_pos;
            }
            //get 32X16 mapping mb data to mb_in rotion to 16X32 mapping
            mb_c_rotation(p_mb_in,p_mb_out);
        }
    }
}

void mb_y_270_rotation(unsigned char *mb_in,unsigned char *mb_out)

{
    int hor = 0;
    int ver = 0;
    unsigned int t0, t1, t2, t3;
    unsigned int r0, r1, r2, r3;
    unsigned int *mb_in_dw = (unsigned int*)mb_in;
    unsigned int *mb_out_dw = (unsigned int*)mb_out;
    unsigned int *p_in = (unsigned int*)mb_in;
    unsigned int *p_out= (unsigned int*)mb_out;
    for(ver=0; ver<SRC_MB_TITE_HEIGHT; ver+=4)
    {
        p_in = mb_in_dw + ((ver*SRC_MB_TITE_WIDTH)>>2);
        prefetch_read(p_in + 32);
        prefetch_read(p_in + 40);
        prefetch_read(p_in + 48);
        prefetch_read(p_in + 56);
        p_out = mb_out_dw + 112 +  (ver>>2);
        prefetch_write(p_out + 1);
        for(hor=0; hor<SRC_MB_TITE_WIDTH; hor+=4)
        {
            t0 = p_in[0];
            t1 = p_in[8];
            t2 = p_in[16];
            t3 = p_in[24];
            p_in++;
            r0 =((t0&0xff))|((t1&0xff)<<8)|((t2&0xff)<<16)|((t3&0xff)<<24);
            t0>>=8; t1>>=8; t2>>=8; t3>>=8;
            r1 =((t0&0xff))|((t1&0xff)<<8)|((t2&0xff)<<16)|((t3&0xff)<<24);
            t0>>=8; t1>>=8; t2>>=8; t3>>=8;
            r2 =((t0&0xff))|((t1&0xff)<<8)|((t2&0xff)<<16)|((t3&0xff)<<24);
            t0>>=8; t1>>=8; t2>>=8; t3>>=8;
            r3 =((t0&0xff))|((t1&0xff)<<8)|((t2&0xff)<<16)|((t3&0xff)<<24);
            p_out[0] = r3;
            p_out[4] = r1;
            p_out[8] = r2;
            p_out[12] = r0;
            p_out-=16;
        }
    }
}

void mb_c_270_rotation(unsigned char *mb_in,unsigned char *mb_out)
{
    //mapping 32X8 to 16X16
    int hor =0;
    int ver =0;
    unsigned int t0, t1, t2, t3;
    unsigned int r0, r1, r2, r3;
    unsigned int *p_word_in = (unsigned int*)mb_in;
    unsigned int *p_word_out = (unsigned int*)mb_out;
    unsigned int *p_in = (unsigned int*)mb_in;
    unsigned int *p_out= (unsigned int*)mb_out;
    //cb cr as a unit
    for(ver=0;ver<8;ver=ver+2)   //src 32x8
    {
        p_in = p_word_in + ver*8;
        prefetch_read(p_in + 16);
        prefetch_read(p_in + 24);
        p_out = p_word_out + 48 + (ver/2);
        prefetch_write(p_out + 1);
        for(hor=0;hor<32;hor= hor+8)
        {
            t0 = p_in[0];  //cbcr cbcr
            t1 = p_in[1];  //cbcr cbcr
            t2 = p_in[8];  //cbcr cbcr
            t3 = p_in[9];  //cbcr cbcr
            p_in = p_in + 2;
            r0 = ((t0&0xffff) | ((t2&0xffff)<<16));
            t0>>=16;  t2>>=16;
            r1 =  ((t0&0xffff) | ((t2&0xffff)<<16));
            r2 =  ((t1&0xffff) | ((t3&0xffff)<<16));
            t1>>=16;  t3>>=16;
            r3 =  ((t1&0xffff) | ((t3&0xffff)<<16));
            p_out[0] = r3;
            p_out[4] = r1;
            p_out[8] = r2;
            p_out[12] = r0;
            p_out-=16;
        }
    }
}

void rotation_270(unsigned char *fb_in_y, unsigned char *fb_in_c,
                  unsigned char *fb_out_y, unsigned char *fb_out_c,
                  int pic_width, int pic_height)
{
  unsigned char *mb_in;
  unsigned char *mb_out;

  int src_mb_hor_width = 0;  //32
  int src_mb_ver_width = 0;  //16
  int hor_mb_num =0;
  int ver_mb_num =0;
  int mb_num =0;
  int i= 0;
  int hor_num =0;
  int ver_num =0;
  int c_judge_hor_num = 0;
  int c_unalign_mapping_flag = 0;
  int new_pic_width = 0;
  int new_pic_height = 0;
  int in_ver_pos = 0;
  int out_ver_pos = 0;

  int j =0;

  src_mb_hor_width = SRC_MB_TITE_WIDTH;
  src_mb_ver_width = SRC_MB_TITE_HEIGHT;

  hor_mb_num = pic_width/src_mb_hor_width;
  ver_mb_num = pic_height/src_mb_ver_width;
  mb_num =hor_mb_num*ver_mb_num;

  new_pic_width = pic_height;
  new_pic_height = pic_width;
  //y
  for(ver_num=0;ver_num<ver_mb_num;ver_num++)
  {
    in_ver_pos = ver_num*pic_width*src_mb_ver_width;
    out_ver_pos = (ver_num<<9);  //according to out hor pos

    for(hor_num=0;hor_num<hor_mb_num;hor_num++)
    {
      mb_in = fb_in_y + in_ver_pos + hor_num*512;
      mb_out = fb_out_y + (hor_mb_num - hor_num -1)*src_mb_hor_width*new_pic_width + out_ver_pos;
      mb_y_270_rotation(mb_in,mb_out);
    }
  }

  //c
  ver_mb_num = ver_mb_num/2;
  new_pic_width = pic_height;
  c_unalign_mapping_flag = hor_mb_num%2;

  for(ver_num=0;ver_num<ver_mb_num;ver_num++)
  {
    in_ver_pos =  ver_num*pic_width*src_mb_ver_width;
    out_ver_pos = (ver_num<<10);
    for(hor_num=0;hor_num<hor_mb_num;hor_num++)  //one time deal src one mapping,32X16, then mapping to dst part of two mapping unit
    {
      //top mapping top 32X8  ->dst 16X16
      mb_in = fb_in_c + in_ver_pos + hor_num*512;// //get top 32X8
      if(c_unalign_mapping_flag)
      {
          c_judge_hor_num = hor_num - 1;  //when hor_mb_num odd  , add 1
      }
      else
      {
           c_judge_hor_num = hor_num;
      }
      if(c_judge_hor_num%2 == 1) //maping the dst 16x32 top 16X16  last oven value
      {
        mb_out = fb_out_c + (hor_mb_num/2 - c_judge_hor_num/2  - 1)*src_mb_hor_width*new_pic_width + out_ver_pos;
      }
      else  //maping the dst 16x32 bottom 16X16
      {
        mb_out = fb_out_c + (hor_mb_num/2 - c_judge_hor_num/2 - 1)*src_mb_hor_width*new_pic_width + out_ver_pos + 256;
      }

      if((hor_num == 0)&&(c_unalign_mapping_flag==1))  //force put in top 16x32 pos
      {
           mb_out = fb_out_c + (hor_mb_num/2)*src_mb_hor_width*new_pic_width + out_ver_pos;
      }
      //get 32X16 mapping mb data to mb_in rotion to 16X32 mapping
      mb_c_270_rotation(mb_in,mb_out);

      //bottom mapping top 32X8  ->dst 16X16
      mb_in = fb_in_c + in_ver_pos + (hor_num<<9) + 256;  //get bottom 32X8
      if(c_judge_hor_num%2 == 1) //maping the dst 16x32 top 16X16
      {
        mb_out = fb_out_c + (hor_mb_num/2 - c_judge_hor_num/2 -1)*new_pic_width*src_mb_hor_width + out_ver_pos + 512;  // - 1 shift to before mapping
      }
      else  //maping the dst 16x32 bottom 16X16
      {
        mb_out = fb_out_c + (hor_mb_num/2 - c_judge_hor_num/2 -1)*new_pic_width*src_mb_hor_width + out_ver_pos + 768;  //add 256
      }
      if((hor_num == 0)&&(c_unalign_mapping_flag==1)) //force put in top 16x32 pos         
      {
          mb_out = fb_out_c + (hor_mb_num/2)*new_pic_width*src_mb_hor_width + out_ver_pos + 512;  // - 1 shift to before mapping
      }
      //get 32X16 mapping mb data to mb_in rotion to 16X32 mapping
      mb_c_270_rotation(mb_in,mb_out);

    }
  }
}

void mb_y_180_rotation(unsigned char *mb_in,unsigned char *mb_out)
{
    int hor = 0;
    int ver = 0;
    unsigned int t0;
    unsigned int r0;
    unsigned int *mb_in_dw = (unsigned int*)mb_in;
    unsigned int *mb_out_dw = (unsigned int*)mb_out;
    unsigned int *p_in = (unsigned int*)mb_in;
    unsigned int *p_out= (unsigned int*)mb_out;
    for(ver=0; ver<SRC_MB_TITE_HEIGHT; ver++)
    {
        p_in = mb_in_dw + ((ver*SRC_MB_TITE_WIDTH)>>2);
        prefetch_read(p_in + 8);
        p_out = mb_out_dw + (((SRC_MB_TITE_HEIGHT - ver)*SRC_MB_TITE_WIDTH -1)>>2);
        for(hor=0; hor<SRC_MB_TITE_WIDTH; hor+=4)
        {
            t0 = p_in[0];
            p_in++;
            r0 =t0&0xff;
            t0>>=8;
            r0<<=8;
            r0 =(r0 | (t0&0xff));
            t0>>=8;
            r0<<=8;
            r0 =(r0 | (t0&0xff));
            t0>>=8;
            r0<<=8;
            r0 =(r0 | (t0&0xff));
            p_out[0] = r0;
            p_out--;
        }
    }
}

void mb_c_180_rotation(unsigned char *mb_in,unsigned char *mb_out)
{
    int hor =0;
    int ver =0;
    unsigned int t0;
    unsigned int r0;
    unsigned int *mb_in_dw = (unsigned int*)mb_in;
    unsigned int *mb_out_dw = (unsigned int*)mb_out;
    unsigned int *p_in = (unsigned int*)mb_in;
    unsigned int *p_out= (unsigned int*)mb_out;
    for(ver=0;ver<SRC_MB_TITE_HEIGHT;ver++)  //0~15
    {
       p_in = mb_in_dw + ((ver*SRC_MB_TITE_WIDTH)>>2);
       prefetch_read(p_in + 8);
       p_out = mb_out_dw + (((SRC_MB_TITE_HEIGHT - ver)*SRC_MB_TITE_WIDTH -1)>>2);
      for(hor=0;hor<SRC_MB_TITE_WIDTH;hor+=4)  //0~31
      {
          t0 = p_in[0];
          p_in++;
          r0 =t0&0xffff;
          t0>>=16;
          r0<<=16;
          r0 =(r0 | (t0&0xffff));
          p_out[0] = r0;
          p_out--;
      }
    }
}

void rotation_180(unsigned char *fb_in_y, unsigned char *fb_in_c,
                  unsigned char *fb_out_y, unsigned char *fb_out_c,
                  int pic_width, int pic_height)
{
  unsigned char *mb_in;
  unsigned char *mb_out;
  int src_mb_hor_width = 0;
  int src_mb_ver_width = 0;
  int hor_mb_num =0;
  int ver_mb_num =0;
  int mb_num =0;
  int i= 0;
  int j =0;
  int hor_num =0;
  int ver_num =0;
  int new_pic_width = 0;
  int new_pic_height = 0;
  int in_ver_pos = 0;
  int out_ver_pos = 0;
  src_mb_hor_width = SRC_MB_TITE_WIDTH;
  src_mb_ver_width = SRC_MB_TITE_HEIGHT;
  hor_mb_num = pic_width/src_mb_hor_width;
  ver_mb_num = pic_height/src_mb_ver_width;
  mb_num =hor_mb_num*ver_mb_num;
  //y
  for(ver_num=0;ver_num<ver_mb_num;ver_num++)
  {
       in_ver_pos = ver_num*pic_width*src_mb_ver_width;
       out_ver_pos = (ver_mb_num - ver_num -1)*src_mb_ver_width*pic_width;
       for(hor_num=0;hor_num<hor_mb_num;hor_num++)
       {
           //mapping 32x16 to 32x16
           mb_in = fb_in_y + in_ver_pos + (hor_num<<9);  //left 16x16
           mb_out = fb_out_y + out_ver_pos + ((hor_mb_num - hor_num -1)<<9);
           mb_y_180_rotation(mb_in,mb_out);
       }
  }
  //c
  ver_mb_num = ver_mb_num/2;
  for(ver_num=0;ver_num<ver_mb_num;ver_num++)
  {
       in_ver_pos = ver_num*pic_width*src_mb_ver_width;
       out_ver_pos = (ver_mb_num - ver_num -1)*src_mb_ver_width*pic_width;
       for(hor_num=0;hor_num<hor_mb_num;hor_num++)
       {
           mb_in = fb_in_c + in_ver_pos + (hor_num<<9);  //left 16x16
           mb_out = fb_out_c + out_ver_pos + ((hor_mb_num - hor_num -1)<<9);
           mb_c_180_rotation(mb_in,mb_out);
       }
  }
}

/**
  * @brief  rotation frame buffer data with 90deg, 180deg or 270deg
  * @param *fb_in_y : input frame buffer of y data
  * @param *fb_in_c : input frame buffer of c data
  * @param *fb_out_y : input frame buffer of y data
  * @param *fb_out_c : input frame buffer of c data
  * @param  pic_width : picture width in pixel
  * @param  pic_height : picture height in pixel
  * @param  rotation_angle : rotation angle, only 90, 180, 270 support
  * @retval  true or false
  */
int fb_rotation(unsigned char *fb_in_y, unsigned char *fb_in_c,
                unsigned char *fb_out_y, unsigned char *fb_out_c,
                int pic_width, int pic_height, int rotation_angle)
{
    if(rotation_angle == 90)
    {
        rotation_90(fb_in_y, fb_in_c, fb_out_y, fb_out_c, pic_width, pic_height);
    }
    else if(rotation_angle == 180)
    {
        rotation_180(fb_in_y,fb_in_c,fb_out_y,fb_out_c, pic_width,pic_height);
    }
    else if(rotation_angle == 270)
    {
        rotation_270(fb_in_y,fb_in_c,fb_out_y,fb_out_c, pic_width,pic_height);
    }
    else
    {
        return 0;
    }

    return 1;
}

#endif

