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
*   1.1         Jianhua Hu    2013/8/9       Support 180deg and 270deg
*
***********************************************************************/
#ifndef _FB_ROTATION_H_
#define _FB_ROTATION_H_

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
                int pic_width, int pic_height, int rotation_angle);

#endif //#ifndef _FB_ROTATION_H_
