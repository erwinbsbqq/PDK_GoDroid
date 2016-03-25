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

#include <asm/pgtable.h>
#include <mach/ali-s3921.h>


#define BOARD_NAME	"Tiger Board"
#define CONFIG_SENSORS_MPU6050
/*<==================== DRAM MEMORY MAP FOR M3921 ALI PRIVATE USAGE START ====================>*/
/* Anchor positions.
 * Warning: These positions are bind to bootloader, DO NOT modify these unless you KNOW exactly 
 * what you are doing.
 */

/***************************************************************************/
/* define the memory size as below */

/* FB0 Size */
#define FB0_VIRTUAL_WIDTH			    1920
#define FB0_VIRTUAL_HEIGHT			    2160
#define FB0_WIDTH                                    1920
#define FB0_HEIGHT                                   1080
#define FB0_BPP                                         4//bytes per pixel
#define FB0_PITCH                                     FB0_VIRTUAL_WIDTH

#define FB0_MEM_SIZE                               (((FB0_VIRTUAL_HEIGHT * FB0_PITCH * FB0_BPP) + 0xFFF) & 0xFFFFF000)
#define FB0_CMAP_MEM_SIZE				(1024)

/* FB3 Size */
#define FB2_VIRTUAL_WIDTH                   1920
#define FB2_VIRTUAL_HEIGHT                  1080
#define FB2_WIDTH                                   1920
#define FB2_HEIGHT                                  1080
#define FB2_BPP                                        1// bytes per pixel
#define FB2_PITCH                                    FB2_VIRTUAL_WIDTH

#define FB2_MEM_SIZE                              (((FB2_VIRTUAL_HEIGHT * FB2_PITCH * FB2_BPP) + 0xFFF) & 0xFFFFF000)
#define FB2_CMAP_MEM_SIZE				(1024)

/* Video Size */
#define VDEC_FB_MEM_SIZE		   	   	   0x05A00000
#define VCAP_FB_MEM_SIZE				   (736*576*2*4)
#define STILL_FB_MEM_SIZE		   		   (0x300000)
#define VDEC_VBV_MEM_SIZE				   (0x600000)

#define VDEC_CMD_QUEUE_MEM_SIZE 		   (0x10000)
#define VDEC_LAF_FLAG_MEM_SIZE 	          (0x11000)
#define VDEC_RAW_DATA_MEM_SIZE		   (VDEC_CMD_QUEUE_MEM_SIZE + VDEC_LAF_FLAG_MEM_SIZE)

/* MCAPI Size */
#define MCAPI_MEM_SIZE					   (0x2000)

/* Define the DSC Size (DSC+AES+DES/TDES+CSA+SHA) */
#define ALI_DSC_KERNEL_BUFFER_SIZE          (0x100000)
#define SHARED_MEM_SIZE  				   (4096)
#define DSC_MEM_SIZE					   (ALI_DSC_KERNEL_BUFFER_SIZE*2) //DSC , input 1M + output 1M
#define SEE_MEM_SIZE					   (0x02000000 - SHARED_MEM_SIZE - VDEC_VBV_MEM_SIZE - MCAPI_MEM_SIZE - DSC_MEM_SIZE \
											- FB2_MEM_SIZE)

/* Audio Size */
#define AUDIO_MEM_SIZE				   	   (0x100000)

/* User Confg Size*/
#define USER_DATA_MEM_SIZE  			   (0x20000)		// 4k

/* NOR Flash Size */


/* TTX/SUB Size */


/* Boot Logo Size */
#define BOOTLOGO_DATA_MEM_SIZE 	    	(0x100000)

/*Dmx Size*/
#define DMX_SEE_BUF_SIZE  			    (0xBC000)
#define DMX_MAIN_BUF_SIZE		           (0x00100000)

#define DMX_MEM_SIZE				    (0x00600000)//(DMX_MAIN_BUF_SIZE * 2 + DMX_SEE_BUF_SIZE * 3)

/* TSG Size */
#define TSG_MEM_SIZE 			    (0x40000) /* 256K */

/* Media Player Size */
#define MP_MEM_SIZE 				       (0xC00000)

/* NIM mode J83B Size */
#define J83B_MEM_SIZE                			(0x00200000)

/* Image Decoder Size */
#define IMAGE_DECODER_MEM_SIZE                (0x00200000) 

/* Mali Dedicated Size */
#define MALI_DEDICATED_MEM_SIZE                (0x04000000) 

/* Mali UMP Size */
// Android modified
#define MALI_UMP_MEM_SIZE                (0x04000000) 



/* DECA */
#define ALI_DECA_MEM_SIZE                    (0x20000)

/* VENC added_by_VicZhang_on_20131111.  */
#define ALI_VENC_MEM_SIZE                    (0x2000000)
//#define ALSA_I2SIRX_I2SO_MIX_MEM_LEN (516 * 1024 )
#define ALSA_I2SIRX_I2SO_MIX_MEM_LEN (64 * 1024 + 1024 * 1024 + 1024)

/*Define the I2C GPIO emulation  serial date & clock pin  */

#ifdef CONFIG_ALI_PAN_CH455
#define I2C_GPIO_SDA_PIN    87
#define I2C_GPIO_SCL_PIN    12
#endif

#define I2C_GPIO_DVFS_QFP_SDA_PIN    89
#define I2C_GPIO_DVFS_QFP_SCL_PIN    90


typedef enum 
{
	I2C_DEVICE_ID = 4,
    I2C_DEVICE_HDMI_ID = 5,
} ALI_I2C_DEVICE_ID;



/********************************************/

/* Teletext size */
#define __MM_TTX_BS_LEN                     (0x5000)
#define __MM_TTX_PB_LEN                     (0xCB200)
#define __MM_TTX_SUB_PAGE_LEN               (0x14500)
#define __MM_TTX_P26_NATION_LEN             (0x61A80)
#define __MM_TTX_P26_DATA_LEN               (0x3E8)
#define __MM_TTX_PARAM_BUF_LEN              (4 * 1024)

/* Subtitle size */
#define __MM_SUB_BS_LEN                     (0x12000)
#define __MM_SUB_PB_LEN                     (0xA0000)
#define __MM_SUB_HW_DATA_LEN                (0xC000)


/* define the memory size end */
/***************************************************************************/

/***************************************************************************/
/* define the memory address as below */

#define __MM_RESERVED_ADDR1			     (0x84000000)

/* above the __MM_RESERVED_ADDR1 */
/* SEE
*/
#define __MM_SEE_PRIVATE_START                		(__MM_RESERVED_ADDR1) 
#define __MM_SEE_PRIVATE_END                  		(__MM_SEE_PRIVATE_START + SEE_MEM_SIZE)

/* FB2
*/
#define __MM_FB2_MEM_START_1G                     	(__MM_SEE_PRIVATE_END)
#define __MM_FB2_MEM_END_1G                       		(__MM_FB2_MEM_START_1G + FB2_MEM_SIZE)//0x384000

#define __MM_FB2_MEM_START_512M                     	(__MM_FB2_MEM_START_1G)
#define __MM_FB2_MEM_END_512M                       	(__MM_FB2_MEM_END_1G)

/* SHARED
*/
#define __MM_SEE_MAIN_SHARED_MEM_START         (__MM_FB2_MEM_END_1G) 
#define __MM_SEE_MAIN_SHARED_MEM_END             (__MM_SEE_MAIN_SHARED_MEM_START + SHARED_MEM_SIZE)//0x1000

/* MCAPI
*/ 
#define __MM_MCAPI_MEM_START                           	(__MM_SEE_MAIN_SHARED_MEM_END)
#define __MM_MCAPI_MEM_END                       		(__MM_MCAPI_MEM_START + MCAPI_MEM_SIZE)//0x2000

/* DSC ,
*/ 
#define __MM_DSC_MEM_START                           	(__MM_MCAPI_MEM_END)
#define __MM_DSC_MEM_END                       			(__MM_DSC_MEM_START + DSC_MEM_SIZE)//0x200000

/* Video VBV
*/
#define __MM_VBV_BUF_START                    			(__MM_DSC_MEM_END) 
#define __MM_VBV_BUF_END                      			(__MM_VBV_BUF_START + VDEC_VBV_MEM_SIZE)//0x600000


/* Subtitle
*/
#define __MM_SUBT_BUF_START                 (__MM_VBV_BUF_END )
#define __MM_SUBT_BUF_END                   (__MM_SUBT_BUF_START + __MM_SUB_BS_LEN + __MM_SUB_PB_LEN + __MM_SUB_HW_DATA_LEN)

/* Teletext
*/
#define __MM_TTX_BUF_START                  (__MM_SUBT_BUF_END)
#define __MM_TTX_BUF_END                    (__MM_TTX_BUF_START + __MM_TTX_BS_LEN + __MM_TTX_PB_LEN + __MM_TTX_SUB_PAGE_LEN + \
	                                         __MM_TTX_P26_NATION_LEN + __MM_TTX_P26_DATA_LEN)

#define __MM_TTX_PARAM_BUF_START                  (__MM_TTX_BUF_END)
#define __MM_TTX_PARAM_BUF_END                    (__MM_TTX_PARAM_BUF_START + __MM_TTX_PARAM_BUF_LEN)


/*
Alsa Capture Buffer
*/ 
#define __MM_ALSA_CAP_MEM_START                         (__MM_TTX_PARAM_BUF_END)
#define __MM_ALSA_CAP_MEM_END                       	(__MM_ALSA_CAP_MEM_START + 0X100000)

/*
 * Alsa Capture Buffer
*/ 
#define __MM_ALSA_I2SIRX_I2SO_MIX_MEM_START                         (__MM_ALSA_CAP_MEM_END)
#define __MM_ALSA_I2SIRX_I2SO_MIX_MEM_END                       	(__MM_ALSA_I2SIRX_I2SO_MIX_MEM_START + ALSA_I2SIRX_I2SO_MIX_MEM_LEN)


#define __MM_RESERVED_ADDR1_END	            (__MM_ALSA_I2SIRX_I2SO_MIX_MEM_END)//(__MM_ALSA_CAP_MEM_END)   //changed by kinson for alsa captuer buffer no cache  (__MM_TTX_BUF_END)
	                                         

/* below the __MM_RESERVED_ADDR1 */
/* User configuration.
*/
#define __MM_USER_DATA_END                    		(__MM_RESERVED_ADDR1)
#define __MM_USER_DATA_START                  		(__MM_USER_DATA_END - USER_DATA_MEM_SIZE) 

/* VDEC RAW Data
*/
#define __MM_VDEC_RAW_DATA_BUF_START          	((__MM_USER_DATA_START - VDEC_RAW_DATA_MEM_SIZE)\
													& 0xFFFFFC00)
#define __MM_VDEC_RAW_DATA_BUF_END            	(__MM_VDEC_RAW_DATA_BUF_START + VDEC_RAW_DATA_MEM_SIZE)

/* VCAP FB
*/
#define __MM_VCAP_FB_START                    			((__MM_VDEC_RAW_DATA_BUF_START - VCAP_FB_MEM_SIZE)\
													& 0xFFFFF000)
#define __MM_VCAP_FB_END                      			(__MM_VCAP_FB_START + VCAP_FB_MEM_SIZE) 

/* STILL FB
*/
#define __MM_STILL_FRAME_START                		((__MM_VCAP_FB_START - STILL_FB_MEM_SIZE)\
													& 0xFFFFF000)
#define __MM_STILL_FRAME_END                  		(__MM_STILL_FRAME_START + STILL_FB_MEM_SIZE) 

/* Meida Player
*/
#define __MM_MP_MEM_START                     			(__MM_STILL_FRAME_START - MP_MEM_SIZE)
#define __MM_MP_MEM_END                       			(__MM_MP_MEM_START + MP_MEM_SIZE)

/* Image Decoder
*/
#define __MM_IMAGE_DECODER_MEM_START          	((__MM_MP_MEM_START - IMAGE_DECODER_MEM_SIZE)\
													& 0xFFFFFF00)
#define __MM_IMAGE_DECODER_MEM_END            	(__MM_IMAGE_DECODER_MEM_START + IMAGE_DECODER_MEM_SIZE) 

/* TSG
*/
#define __MM_TSG_MEM_START                    		((__MM_IMAGE_DECODER_MEM_START - TSG_MEM_SIZE)\
													& 0xFFFFFF00)
#define __MM_TSG_MEM_END                      			(__MM_TSG_MEM_START + TSG_MEM_SIZE)

/* DMX
*/
#define __MM_DMX_MEM_START                    		((__MM_TSG_MEM_START - DMX_MEM_SIZE) \
													& 0xFFFFFF00)
#define __MM_DMX_MEM_END                      			(__MM_DMX_MEM_START + DMX_MEM_SIZE)

/* NIM J83B
*/
#define __MM_NIM_J83B_MEM_START 				(__MM_DMX_MEM_START - J83B_MEM_SIZE)
#define __MM_NIM_J83B_MEM_END 				(__MM_NIM_J83B_MEM_START + J83B_MEM_SIZE)

/* FB0 CMP ADDR
*/
#define __MM_FB0_CMAP_START					((__MM_NIM_J83B_MEM_START - FB0_CMAP_MEM_SIZE) & 0xFFFFFFE0)
#define __MM_FB0_CMAP_END						(__MM_FB0_CMAP_START + FB0_CMAP_MEM_SIZE)

/* FB2 CMP ADDR
*/
#define __MM_FB2_CMAP_START					((__MM_FB0_CMAP_START - FB2_CMAP_MEM_SIZE) & 0xFFFFFFE0)
#define __MM_FB2_CMAP_END						(__MM_FB2_CMAP_START + FB2_CMAP_MEM_SIZE)

/* Boot Logo
*/
#define __MM_BOOT_COMMAND_DATA_START			(__MM_FB2_CMAP_START - BOOTLOGO_DATA_MEM_SIZE)
#define __MM_BOOT_COMMAND_DATA_END				(__MM_BOOT_COMMAND_DATA_START + BOOTLOGO_DATA_MEM_SIZE)

/* DECA
*/
#define __MM_DECA_DATA_START                       (__MM_BOOT_COMMAND_DATA_START - ALI_DECA_MEM_SIZE)
#define __MM_DECA_DATA_END                         (__MM_DECA_DATA_START + ALI_DECA_MEM_SIZE)

/* TOP and Bottom1
*/
#define __MM_ALI_PRIVATE_MEM_START1			    ALI_MEMALIGNDOWN(__MM_DECA_DATA_START)
#define __MM_ALI_PRIVATE_MEM_END1				ALI_MEMALIGNUP(__MM_RESERVED_ADDR1_END)


/* Second Channel Start Addr */
#define __MM_RESERVED_ADDR2					(0x88000000)

/* AUDIO,ae code must be 0x88000000
*/
#define __MM_AUDIO_ENGINE_START                		(__MM_RESERVED_ADDR2) 
#define __MM_AUDIO_ENGINE_END                  		(__MM_AUDIO_ENGINE_START + AUDIO_MEM_SIZE)

#define __MM_BOOTMEDIA_MKV_START                       	(__MM_AUDIO_ENGINE_END)
#define __MM_BOOTMEDIA_MKV_END                         	(__MM_BOOTMEDIA_MKV_START+0x1000000)

/* Video Frame Buffer
*/
#define __MM_VIDEO_BUF_START                  		((__MM_BOOTMEDIA_MKV_END + 0xFFF) & 0xFFFFF000)
#define __MM_VIDEO_BUF_END                    			(__MM_VIDEO_BUF_START + VDEC_FB_MEM_SIZE)

/*
** VENC added_by_VicZhang_on_20131111.
*/
#define __MM_VENC_DATA_START                       	(__MM_VIDEO_BUF_END)
#define __MM_VENC_DATA_END                         		(__MM_VENC_DATA_START+ALI_VENC_MEM_SIZE)

#define __MM_RESERVED_ADDR2_END	   (__MM_VENC_DATA_END)

/* TOP and Bottom2
*/
#define __MM_ALI_PRIVATE_MEM_START2			    ALI_MEMALIGNDOWN(__MM_RESERVED_ADDR2)
#define __MM_ALI_PRIVATE_MEM_END2				ALI_MEMALIGNUP(__MM_RESERVED_ADDR2_END)



/* Third Channel Start Addr for 1G */
#define __MM_RESERVED_ADDR3_1G					(0xA0000000)

/* FB0
*/
#define __MM_FB_MEM_START_1G                     			(__MM_RESERVED_ADDR3_1G)
#define __MM_FB_MEM_END_1G                       			(__MM_FB_MEM_START_1G + FB0_MEM_SIZE)

/* MALI
*/
#define __MM_MALI_MEM_START_1G					(__MM_FB_MEM_END_1G)//((__MM_FB_MEM_END_1G + 0xFF) & 0xFFFFFF00)
#define __MM_MALI_MEM_END_1G					(__MM_FB_MEM_END_1G)//(__MM_MALI_MEM_START_1G + MALI_DEDICATED_MEM_SIZE)

/* MALI UMP
*/
#define __MM_MALI_UMP_MEM_START_1G				(__MM_MALI_MEM_END_1G)//((__MM_MALI_MEM_END_1G + 0xFF) & 0xFFFFFF00)
#define __MM_MALI_UMP_MEM_END_1G				(__MM_MALI_MEM_END_1G)//(__MM_MALI_UMP_MEM_START_1G + MALI_UMP_MEM_SIZE)

/* TOP and Bottom3
*/
#define __MM_ALI_PRIVATE_MEM_START3_1G			ALI_MEMALIGNDOWN(__MM_FB_MEM_START_1G)	
#define __MM_ALI_PRIVATE_MEM_END3_1G			ALI_MEMALIGNUP(__MM_MALI_UMP_MEM_END_1G)

/* Third Channel Start Addr for 512M */
#define __MM_RESERVED_ADDR3_512M					(0x90000000)

/* FB0
*/
#define __MM_FB_MEM_START_512M                     			(__MM_RESERVED_ADDR3_512M)
#define __MM_FB_MEM_END_512M                       			(__MM_FB_MEM_START_512M + FB0_MEM_SIZE)

/* MALI
*/
#define __MM_MALI_MEM_START_512M						(__MM_FB_MEM_END_512M)
#define __MM_MALI_MEM_END_512M						(__MM_FB_MEM_END_512M)

/* MALI UMP
*/
#define __MM_MALI_UMP_MEM_START_512M				(__MM_MALI_MEM_END_512M)
#define __MM_MALI_UMP_MEM_END_512M					(__MM_MALI_MEM_END_512M)

/* TOP and Bottom3
*/
#define __MM_ALI_PRIVATE_MEM_START3_512M			ALI_MEMALIGNDOWN(__MM_FB_MEM_START_512M)	
#define __MM_ALI_PRIVATE_MEM_END3_512M				ALI_MEMALIGNUP(__MM_MALI_UMP_MEM_END_512M)

/* define the memory address end */
/***************************************************************************/

/*<==================== DRAM MEMORY MAP FOR M3921 ALI PRIVATE USAGE END   ====================>*/


/*<==================== Function Prototype Need To Be Implemented By Each Board Start ====================>*/
struct ali_hwbuf_desc* ali_get_privbuf_desc(int * hwbuf_desc_cnt);
void customize_board_setting(void);
/*<==================== Function Prototype Need To Be Implemented By Each Board End ====================>*/

/* define AFD */
#ifdef CONFIG_ALI_SW_AFD
#define AFD_SW_SUPPORT
#endif
#ifdef CONFIG_ALI_HW_AFD
#define AFD_HW_SUPPORT
#endif

#if( defined (AFD_SW_SUPPORT)) || ( defined(AFD_HW_SUPPORT))
#define SUPPORT_AFD_PARSE
#define SUPPORT_AFD_SCALE
#define SUPPORT_AFD_WSS_OUTPUT
#endif



#endif

