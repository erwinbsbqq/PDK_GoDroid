////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2013 ALI, Inc.  All rights reserved.
//
// ALI, Inc.
// ALI IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
// COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
// ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
// STANDARD, ALI IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
// IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
// FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
// ALI EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
// THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
// ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
// FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Id: board_config.c,v 1.0 2013/07/13 17:02:53 $
//
//////////////////////////////////////////////////////////////////////////////

/*****************************************************************************/
/**
*
* @file board_config.h
*
* The XTemac driver. Functions in this file are the minimum required functions
* for this driver. See xtemac.h for a detailed description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a joy  2013/07/13 First release
* </pre>
******************************************************************************/
#ifndef ALI_BOARD_CFG 
#define ALI_BOARD_CFG

//#include <asm/pgtable.h>

#define FB_WIDTH                                    1280
#define FB_HEIGHT                                   720
#define FB_BPP                                      4//bytes per pixel
#define FB_PITCH                                    1280
#define FB3_WIDTH                                   1280//1920
#define FB3_HEIGHT                                  720//1080
#define FB3_BPP                                     4// bytes per pixel
#define FB3_PITCH                                   1280//1920
#define FB_MEM_SIZE                                 (((FB_HEIGHT * FB_PITCH * FB_BPP) + 0xFFF) & 0xFFFFF000)

/*<==================== DRAM MEMORY MAP FOR S3921 QFP256M START ====================>*/
/* Anchor positions.
 * Warning: These positions are bind to bootloader, DO NOT modify these unless you KNOW exactly 
 * what you are doing.
 */
/* 1, NOR FLash
*/
//#define __MM_NOR_FLASH_MEM_START                   0x0C000000
//#define __MM_NOR_FLASH_MEM_END                     0x10000000

/* 2, MCAPI
*/  
#define __MM_MCAPI_MEM_START                       0x83EFD000
#define __MM_MCAPI_MEM_END                         0x83EFF000

/* 3, Boot LOGO
*/
#define __MM_DRAM_BOOTLOGO_DATA_START              0x83EFF000 
#define __MM_DRAM_BOOTLOGO_DATA_END                0x83FFF000

/* 4, User configuration.
*/
#define __MM_DRAM_USER_DATA_START                  0x83FFF000 
#define __MM_DRAM_USER_DATA_END                    0x84000000

/* 5, SEE PRIVATE MEM
*/
#define __MM_DRAM_SEE_PRIVATE_START                0x84000000 
#define __MM_DRAM_SEE_PRIVATE_END                  0x86000000

/* 6, SEE MAIN SHARED MEM
*/
#define __MM_DRAM_SEE_MAIN_SHARED_MEM_START        0x86000000 
#define __MM_DRAM_SEE_MAIN_SHARED_MEM_END          0x86000200


/* Moveable positions: Could be dynamically configured within DRAM space.
 * Caution: DO NOT overlap with anchor positions!!
*/
/* 1, FRAME BUFFER, must be 4k aligned.
*/ 
#define __MM_DRAM_FB_MEM_START                     0x86008000 
#define __MM_DRAM_FB_MEM_END                       (__MM_DRAM_FB_MEM_START + FB_MEM_SIZE)

/* 2, TSG
*/
#define __MM_DRAM_TSG_BUF_START                    (__MM_DRAM_FB_MEM_END)
#define __MM_DRAM_TSG_BUF_END                      (__MM_DRAM_TSG_BUF_START + 0x40000)

/* 3, DMX
*/
#define __MM_DRAM_DMX_BUF_START                    (__MM_DRAM_TSG_BUF_END)
#define __MM_DRAM_DMX_BUF_END                      (__MM_DRAM_DMX_BUF_START + ((0x00100000 * 2) + (0xBC000 * 3)))

/* 4, Image decoder
*/
#define __MM_DRAM_IMAGE_DECODER_MEM_START          (__MM_DRAM_DMX_BUF_END)
#define __MM_DRAM_IMAGE_DECODER_MEM_END            (__MM_DRAM_IMAGE_DECODER_MEM_START + 0x00200000) 

/* 5, MEDIA PLAYER
*/
#define __MM_DRAM_MP_MEM_START                     (__MM_DRAM_IMAGE_DECODER_MEM_END)
#define __MM_DRAM_MP_MEM_END                       (__MM_DRAM_MP_MEM_START + 0x00C00000)

/* 6, VDEC_LAF_FLAG
*/
#define __MM_DRAM_VDEC_LAF_FLAG_BUF_START          (__MM_DRAM_MP_MEM_END)
#define __MM_DRAM_VDEC_LAF_FLAG_BUF_END            (__MM_DRAM_VDEC_LAF_FLAG_BUF_START + 0x10800)

/* 7, VDEC CMDQ
*/
#define __MM_DRAM_VDEC_CMD_QUEUE_START             (__MM_DRAM_VDEC_LAF_FLAG_BUF_END)
#define __MM_DRAM_VDEC_CMD_QUEUE_END               (__MM_DRAM_VDEC_CMD_QUEUE_START + 0x10000)

/* 8, VBV buffer
*/
#define __MM_DRAM_VBV_BUF_START                    (__MM_DRAM_VDEC_CMD_QUEUE_END)
#define __MM_DRAM_VBV_BUF_END                      (__MM_DRAM_VBV_BUF_START + 0x600000)

/* 9, STILL FRAME
*/
#define __MM_DRAM_STILL_FRAME_START                (__MM_DRAM_VBV_BUF_END)
#define __MM_DRAM_STILL_FRAME_END                  (__MM_DRAM_STILL_FRAME_START + 0x300000) 

/* 10, VCAP
*/
#define __MM_DRAM_VCAP_FB_START                    (__MM_DRAM_STILL_FRAME_END)
#define __MM_DRAM_VCAP_FB_END                      (__MM_DRAM_VCAP_FB_START + (736*576*2*4)) 

/* 11, VIDEO
*/
#define __MM_DRAM_VIDEO_BUF_START                  (__MM_DRAM_VCAP_FB_END)
#define __MM_DRAM_VIDEO_BUF_END                    (__MM_DRAM_VIDEO_BUF_START + 0x02000000)

/*<==================== DRAM MEMORY MAP FOR S3921 QFP256M END   ====================>*/

/*<==================== Function Prototype Need To Be Implemented By Each Board Start ====================>*/

/*<==================== Function Prototype Need To Be Implemented By Each Board End ====================>*/

#endif


