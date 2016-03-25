/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation. 2010 Copyright (C)
 *  (C)
 *  File: board_config.c
 *  (I)
 *  Description: configuration for the board
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.06.03			Sam			Create

*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version
* 2 of the License, or (at your option) any later version.
*

 ****************************************************************************/

#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/screen_info.h>

#include <asm/cpu.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/mach-ali/prom.h>
#include <asm/dma.h>
#include <asm/time.h>
#include <asm/traps.h>
#ifdef CONFIG_VT
#include <linux/console.h>
#endif

#include <asm/mach-ali/typedef.h>
#include <asm/mach-ali/m36_gpio.h>
#include <asm/reboot.h>

#include "tve_config.h"
#include "board_config.h"
#include <ali_board_config.h>
//#include <boot_types.h>

/* memory mapping configuration start */
/* mapping table description:
top:
	32M --> reserved for hardware accelerator with memory limitation
	32M --> reserved for SEE CPU
	...    --> reserved for MAIN CPU
bottom:
*/
/* the top address of the memory */
#if 0
static unsigned long __MM_TOP_ADDR = 0;

/* frame buffer area defined for video decoder, it is different with
GMA frame buffer. all the frame buffer must be allocated in the same 32-M segment */
static unsigned long __MM_VIDEO_AREA_LEN = 0;
static unsigned long __MM_VIDEO_START_ADDR = 0;

static unsigned long __MM_VCAP_FB_SIZE = 0;
static unsigned long __MM_VCAP_FB_ADDR = 0;

static unsigned long __MM_STILL_FRAME_SIZE = 0;
static unsigned long __MM_STILL_FRAME_ADDR = 0;

/* shared memory area reserved for the RPC driver*/
static unsigned long __MM_SHARED_MEM_LEN = 0;
static unsigned long __MM_SHARED_MEM_TOP_ADDR = 0; //(__MM_TOP_ADDR - 0x02000000)
static unsigned long __MM_SHARED_MEM_SATRT_ADDR = 0;

/* private memory area reserved for the SEE CPU */
static unsigned long __MM_PRIVATE_AREA_LEN = 0;//(0x02000000 - __MM_SHARED_MEM_LEN)
static unsigned long __MM_PRIVATE_AREA_SATRT_ADDR = 0;


#define FB3_WIDTH		1280	//1920
#define FB3_HEIGHT		720		//1080
#define FB3_BPP			4		// 4/1  bytes per pixel
#define FB3_PITCH		1280	//1920

static unsigned long __MM_VDEC_VBV_LEN = 0; //0x380000 //(3.5M)
static unsigned long __MM_VDEC_CMD_QUEUE_LEN = 0;
static unsigned long __MM_VDEC_LAF_FLAG_BUF_LEN = 0; // (0xC00*22)
static unsigned long __MM_OSD2_LEN = 0; // (1920 * 1080)
static unsigned long __MM_TTX_BS_LEN = 0;
static unsigned long __MM_TTX_PB_LEN = 0;
static unsigned long __MM_TTX_SUB_PAGE_LEN = 0;
static unsigned long __MM_TTX_P26_NATION_LEN = 0;
static unsigned long __MM_TTX_P26_DATA_LEN = 0;
static unsigned long __MM_SUB_BS_LEN = 0;
static unsigned long __MM_SUB_PB_LEN = 0;
static unsigned long __MM_SUB_HW_DATA_LEN =	0;

static unsigned long __MM_VDEC_VBV_START_ADDR =0;
static unsigned long __MM_VDEC_CMD_QUEUE_ADDR = 0;  	//256 bytes alignment
static unsigned long __MM_VDEC_LAF_FLAG_BUF_ADDR = 0;  //1024 bytes alignment
static unsigned long __MM_OSD_BK_ADDR2 = 0;
static unsigned long __MM_TTX_BS_START_ADDR	= 0;
static unsigned long __MM_TTX_PB_START_ADDR = 0;
static unsigned long __MM_TTX_SUB_PAGE_BUF_ADDR = 0;
static unsigned long __MM_TTX_P26_NATION_BUF_ADDR = 0;
static unsigned long __MM_TTX_P26_DATA_BUF_ADDR = 0;
static unsigned long __MM_SUB_BS_START_ADDR = 0;
static unsigned long __MM_SUB_HW_DATA_ADDR = 0;
static unsigned long __MM_SUB_PB_START_ADDR = 0;


/* user data memory area reserved for bootloader -> kenerl */
static unsigned long __MM_USER_DATA_MEM_LEN = 0;		// 4k
static unsigned long __MM_USER_DATA_MEM_START = 0;

static unsigned long __MM_BOOTLOGO_DATA_MEM_LEN = 0;
static unsigned long __MM_BOOTLOGO_DATA_MEM_START = 0;

/**********************************************************************
*SGDMA Buffer
*Warning: MUST KEEP THIS ADDRESS MAPPING TO PHSYCAL ADDRESS BELOW 256M!!
*		SINCE SGDMA HW CAN ONLY ACCESS PHYSICAL ADDRESS WHICH IS BELOW 256M!!
*
*Used for storing SGDMA copy node and sc buffer. Please don't modify this unless 
*you know what you are doing.
*
*Buffer for 8 List Node and 4 Start Code detect
***********************************************************************/

//#define __MM_SGDMA_MEM_LEN 0x8000 /* (256 * 16 * 8) + (4 * 4K) = 32K + 16K = 48K. */
static unsigned long __MM_SGDMA_NODE_MEM_LEN = 0; /* (256 * 16 * 8) = 32K  */
static unsigned long __MM_SGDMA_MEM_LEN = 0; /* (256 * 16 * 8) + (4 * 4K) = 32K + 16K = 48K. */
static unsigned long __MM_SGDMA_MEM_END = 0;
static unsigned long __MM_SGDMA_MEM_START = 0;

/* MAIN and SEE DMX data transfer buffers length.
 * Must keep compatible with TDS macro: #define TS_BLOCK_SIZE 0xbc000.
 */
#define TDS_DMX_SEE_BUF_SIZE  0xBC000  

/* MAIN CPU DMX HW buffer length.
 * Warning: MUST KEEP THIS ADDRESS MAPPING TO PHSYCLAL ADDRESS BELOW 256M!!!
 *          SINCE DMX HW CAN ONLY ACCESS PHSYCLAL ADDRESS WHICH IS BELOW 256M!
 */
static unsigned long __MM_DMX_MEM_LEN = 0; /* 1M */
static unsigned long __MM_DMX_MEM_TOP_ADDR = 0;
static unsigned long __MM_DMX_MEM_START_ADDR = 0;

/* TSG HW buffer.
*/
static unsigned long __MM_TSG_BUF_LEN = 0; /* 256K */
static unsigned long __MM_TSG_BUF_START_ADDR = 0;

/* media player buffer */
static unsigned long __MM_MP_MEM_LEN  = 0;
static unsigned long __MM_MP_MEM_TOP_ADDR = 0;
static unsigned long __MM_MP_MEM_START_ADDR = 0;


/* frame buffer */
#define FB_WIDTH		1280
#define FB_HEIGHT		720
#define FB_BPP			4 // bytes per pixel
#define FB_PITCH			1280

#define FB_MEM_SIZE		(((FB_HEIGHT * FB_PITCH * FB_BPP) + 0xFFF) & 0xFFFFF000)
#define GE_CMD_SIZE 0
static unsigned long __MM_FB_MEM_TOP_ADDR = 0;
static unsigned long __MM_FB_MEM_START_ADDR = 0;


/* NIM mode J83B buffer */
static unsigned long __MM_NIM_J83B_MEM_LEN = 0; /* 2M */
static unsigned long __MM_NIM_J83B_MEM_START_ADDR = 0;

/* image decoder buffer */
static unsigned long __MM_IMAGE_DECODER_MEM_LEN = 0; /* 2M */
static unsigned long __MM_IMAGE_DECODER_MEM_START_ADDR = 0;

/* ovg buffer */
static unsigned long __MM_OVG_MEM_LEN = 0; 
static unsigned long __MM_OVG_MEM_START_ADDR = 0;

#else

#define __MM_TOP_ADDR 	                (0xA0000000)
#define __MM_END_ADDR                   (0xAFFFFFFF)

/* The length of SEE memory */
#define __MM_SEE_TOP_ADDR               (0xA8000000)
#define __MM_SEE_MEM_LEN                (0x02000000)

#define __MM_KERNEL_HEAP_START_AREA_1   (0xA8000000)
#define __MM_KERNEL_HEAP_START_AREA_0   (__MM_TOP_ADDR)
#define __MM_KERNEL_HEAP_END_AREA_0     (0xA6000000)  

unsigned long __G_MM_OVG_MEM_LEN = 0x1000;
unsigned long __G_MM_MAIN_MEM_NUM = 1;
unsigned long __G_MM_MAIN_MEM[1][2] =  {
				{__MM_KERNEL_HEAP_START_AREA_0,__MM_KERNEL_HEAP_END_AREA_0},	\
			};
unsigned long __G_MM_SHARED_MEM_TOP_ADDR = 0xA8000000;	
unsigned long g_support_afd_scale = 0;
unsigned long g_support_afd_wss = 0;

#endif

/* memory mapping configuration end */

extern void boot_delay(u32 ms);

/*start of read otp function*/
#define OTP_BASE 			0xb8042000
#define OTP_ADDR			0x04
#define OTP_CTRL			0x0c
#define OTP_ST				0x10
#define OTP_RDATA			0x18
#define OTP_BIT_READ_BUSY	1<<8
#define OTP_BIT_READ_TRIG 	1<<8
#define OTP_BIT_PROG_BUSY 	1<<0
#if 0
static unsigned int ReadRegDword(unsigned int offset)
{
	return (*(volatile unsigned int *)(OTP_BASE + offset));
}

static void WriteRegDword(unsigned int offset, unsigned int value)
{
	*(volatile unsigned int *)(OTP_BASE + offset) = value;
}

static unsigned int otp_read_dword(unsigned short addr)
{
	while(ReadRegDword(OTP_ST)&(OTP_BIT_READ_BUSY|OTP_BIT_PROG_BUSY));
	WriteRegDword(OTP_ADDR, addr&0x3ff);
	WriteRegDword(OTP_CTRL, ReadRegDword(OTP_CTRL)|OTP_BIT_READ_TRIG);
	while(ReadRegDword(OTP_ST)&OTP_BIT_READ_BUSY);
	return (ReadRegDword(OTP_RDATA));	
}
static void otp_read_func(unsigned short addr, unsigned char *buf, unsigned short len)
{
    unsigned long i;
    for(i=0;i<len/4;i++,addr+=4)
    {
        buf[i] = otp_read_dword(addr);
    }
}
/*end of read otp function*/

static void phy_chip_setting(void)
{
	 unsigned long tmp_u32;

	if(__G_ALI_BOARD_TYPE >= 0x0003000) // M3701C
	{
		if((__G_ALI_BOARD_TYPE == 0x00003004) 
			|| (__G_ALI_BOARD_TYPE == 0x00003003)
			|| (__G_ALI_BOARD_TYPE == 0x00003002))
		{
			// pinmux RMII
			tmp_u32 = *((volatile unsigned long *)0xb8000088);
			tmp_u32 |= (1 << 27);  
			tmp_u32 &= ~((1 << 28)|(1 << 29)|(1 << 30)|(1 << 22)|(1<<23)|(1 << 24)|(1 << 25));  
			*((volatile unsigned long *)0xb8000088) = tmp_u32;
			
			tmp_u32 = *((volatile unsigned long *)0xb800008c);
			tmp_u32 &= ~((1 << 12));  
			*((volatile unsigned long *)0xb800008c) = tmp_u32;

			//NMP144 DSUB SEL
			if(__G_ALI_BOARD_TYPE == 0x00003002)
			{
				tmp_u32 = *((volatile unsigned long *)0xb800008c);
				tmp_u32 |= (1 << 17);  
				*((volatile unsigned long *)0xb800008c) = tmp_u32;
			}

			if((__G_ALI_BOARD_TYPE == 0x00003003) || (__G_ALI_BOARD_TYPE == 0x00003004))			
			{				
				//set latency counter for DE_O andi VINI
				
				tmp_u32 = *((volatile unsigned long *)0xb8001010);
				tmp_u32 &= ~(1 << 31);
				*((volatile unsigned long *)0xb8001010) = tmp_u32;

				tmp_u32 = *((volatile unsigned long *)0xb8001080);
				tmp_u32 &= 0x00ffffff;
				tmp_u32 |= 0x32000000;
				*((volatile unsigned long *)0xb8001080) = tmp_u32;
			}
			
				boot_delay(10);	

			// reset phy
			tmp_u32 = *((volatile unsigned long *)0xb8000430);
			tmp_u32 |= (1 << 29);  // GPIO29 PHY reset enable
			*((volatile unsigned long *)0xb8000430) = tmp_u32;
				
			tmp_u32 = *((volatile unsigned long *)0xb8000058);
			tmp_u32 |= (1 << 29);  // GPIO29 PHY reset output
			*((volatile unsigned long *)0xb8000058) = tmp_u32;

			tmp_u32 = *((volatile unsigned long *)0xb8000054);
			tmp_u32 |= (1 << 29);  // GPIO29 PHY reset output 1
			*((volatile unsigned long *)0xb8000054) = tmp_u32;
			
				boot_delay(10);		  	
			tmp_u32 = *((volatile unsigned long *)0xb8000054);
			tmp_u32 &= ~(1 << 29);  // GPIO29 PHY reset output 0
			*((volatile unsigned long *)0xb8000054) = tmp_u32;
			
				boot_delay(10);		  	
			tmp_u32 = *((volatile unsigned long *)0xb8000054);
			tmp_u32 |= (1 << 29);  // GPIO29 PHY reset output 1
			*((volatile unsigned long *)0xb8000054) = tmp_u32;
		}		
	}
}
#endif

#if 0
static void set_global_setting_256m(void)
{
	__G_ALI_CHIP_TYPE = 1;
	__G_ALI_CHIP_VERSION = 1; 
	__G_ALI_BOARD_TYPE = 0x00003003;
	__G_ALI_BOARD_VERSION = 1;

	/* memory mapping configuration start */
	/* mapping table description:
	top:
		32M --> reserved for hardware accelerator with memory limitation
		32M --> reserved for SEE CPU
		...    --> reserved for MAIN CPU
	bottom:
	*/
	/* the top address of the memory */
	__MM_TOP_ADDR = (0xA8000000);

	/* frame buffer area defined for video decoder, it is different with
	GMA frame buffer. all the frame buffer must be allocated in the same 32-M segment */
	__MM_VIDEO_AREA_LEN = (0x02000000);
	__MM_VIDEO_START_ADDR = (__MM_TOP_ADDR - __MM_VIDEO_AREA_LEN);

	__MM_VCAP_FB_SIZE = (736*576*2*4);
	__MM_VCAP_FB_ADDR = (__MM_VIDEO_START_ADDR - __MM_VCAP_FB_SIZE);

	__MM_STILL_FRAME_SIZE = (0x300000);
	__MM_STILL_FRAME_ADDR = (__MM_VCAP_FB_ADDR - __MM_STILL_FRAME_SIZE);

	/* shared memory area reserved for the RPC driver*/
	__MM_SHARED_MEM_LEN = (512);
	__MM_SHARED_MEM_TOP_ADDR = (__MM_TOP_ADDR - __MM_VIDEO_AREA_LEN  - __MM_VCAP_FB_SIZE - __MM_STILL_FRAME_SIZE ); //(__MM_TOP_ADDR - 0x02000000)
	__MM_SHARED_MEM_SATRT_ADDR = (__MM_SHARED_MEM_TOP_ADDR - __MM_SHARED_MEM_LEN);

	/* private memory area reserved for the SEE CPU */
	__MM_PRIVATE_AREA_LEN = (0x02000000 - __MM_SHARED_MEM_LEN - __MM_VCAP_FB_SIZE - __MM_STILL_FRAME_SIZE);//(0x02000000 - __MM_SHARED_MEM_LEN)
	__MM_PRIVATE_AREA_SATRT_ADDR = (__MM_SHARED_MEM_SATRT_ADDR - __MM_PRIVATE_AREA_LEN);


	__MM_VDEC_VBV_LEN = 0x600000; //0x380000 //(3.5M)
	__MM_VDEC_CMD_QUEUE_LEN = (0x10000);
	__MM_VDEC_LAF_FLAG_BUF_LEN = (0x10800); // (0xC00*22)
	__MM_OSD2_LEN = (FB3_PITCH * FB3_HEIGHT * FB3_BPP); // (1920 * 1080)
	__MM_TTX_BS_LEN = (0x5000);
	__MM_TTX_PB_LEN = (0xCB200);
	__MM_TTX_SUB_PAGE_LEN = (0x14500);
	__MM_TTX_P26_NATION_LEN = (0x61A80);
	__MM_TTX_P26_DATA_LEN = (0x3E8);
	__MM_SUB_BS_LEN = (0xA000);
#ifdef CONFIG_ALI_M3701G
	__MM_SUB_PB_LEN = (0x50000);
#else
	__MM_SUB_PB_LEN = (0x7E900);
#endif
	__MM_SUB_HW_DATA_LEN =	(0xC000);

	__MM_VDEC_VBV_START_ADDR =(__MM_SHARED_MEM_SATRT_ADDR - __MM_VDEC_VBV_LEN);
	__MM_VDEC_CMD_QUEUE_ADDR = ((__MM_VDEC_VBV_START_ADDR - __MM_VDEC_CMD_QUEUE_LEN)&0xFFFFFF00);  	//256 bytes alignment
	__MM_VDEC_LAF_FLAG_BUF_ADDR = ((__MM_VDEC_CMD_QUEUE_ADDR - __MM_VDEC_LAF_FLAG_BUF_LEN)&0xFFFFFC00);  //1024 bytes alignment
	__MM_OSD_BK_ADDR2 = ((__MM_VDEC_LAF_FLAG_BUF_ADDR - __MM_OSD2_LEN)&0xFFFFF800);
	__MM_TTX_BS_START_ADDR	= ((__MM_OSD_BK_ADDR2 - __MM_TTX_BS_LEN)&0xFFFFFFFC);
	__MM_TTX_PB_START_ADDR = ((__MM_TTX_BS_START_ADDR - __MM_TTX_PB_LEN)&0xFFFFFFFC);
	__MM_TTX_SUB_PAGE_BUF_ADDR = (__MM_TTX_PB_START_ADDR - __MM_TTX_SUB_PAGE_LEN);
	__MM_TTX_P26_NATION_BUF_ADDR = (__MM_TTX_SUB_PAGE_BUF_ADDR - __MM_TTX_P26_NATION_LEN);
	__MM_TTX_P26_DATA_BUF_ADDR = (__MM_TTX_P26_NATION_BUF_ADDR -  __MM_TTX_P26_DATA_LEN);
	__MM_SUB_BS_START_ADDR = ((__MM_TTX_P26_DATA_BUF_ADDR  - __MM_SUB_BS_LEN)&0xFFFFFFFC);
	__MM_SUB_HW_DATA_ADDR = ((__MM_SUB_BS_START_ADDR - __MM_SUB_HW_DATA_LEN)&0xFFFFFFF0);
	__MM_SUB_PB_START_ADDR = ((__MM_SUB_HW_DATA_ADDR - __MM_SUB_PB_LEN)&0xFFFFFFFC);


	/* user data memory area reserved for bootloader -> kenerl */
	__MM_USER_DATA_MEM_LEN = (0x1000);		// 4k
	__MM_USER_DATA_MEM_START = (__MM_PRIVATE_AREA_SATRT_ADDR - __MM_USER_DATA_MEM_LEN);
	

	__MM_BOOTLOGO_DATA_MEM_LEN = (0x100000);
	__MM_BOOTLOGO_DATA_MEM_START = (__MM_USER_DATA_MEM_START - __MM_BOOTLOGO_DATA_MEM_LEN);

	/**********************************************************************
	*SGDMA Buffer
	*Warning: MUST KEEP THIS ADDRESS MAPPING TO PHSYCAL ADDRESS BELOW 256M!!
	*		SINCE SGDMA HW CAN ONLY ACCESS PHYSICAL ADDRESS WHICH IS BELOW 256M!!
	*
	*Used for storing SGDMA copy node and sc buffer. Please don't modify this unless 
	*you know what you are doing.
	*
	*Buffer for 8 List Node and 4 Start Code detect
	***********************************************************************/

	//#define __MM_SGDMA_MEM_LEN 0x8000 /* (256 * 16 * 8) + (4 * 4K) = 32K + 16K = 48K. */
	__MM_SGDMA_NODE_MEM_LEN = 0x8000; /* (256 * 16 * 8) = 32K  */
	__MM_SGDMA_MEM_LEN = (0x8000+(4*0x1000)); /* (256 * 16 * 8) + (4 * 4K) = 32K + 16K = 48K. */
	__MM_SGDMA_MEM_END = __MM_BOOTLOGO_DATA_MEM_START;
	__MM_SGDMA_MEM_START = (__MM_SGDMA_MEM_END - __MM_SGDMA_MEM_LEN);	

	/* MAIN CPU DMX HW buffer length.
	 * Warning: MUST KEEP THIS ADDRESS MAPPING TO PHSYCLAL ADDRESS BELOW 256M!!!
	 *          SINCE DMX HW CAN ONLY ACCESS PHSYCLAL ADDRESS WHICH IS BELOW 256M!
	 */
	__MM_DMX_MEM_LEN = (0x00100000); /* 1M */
	__MM_DMX_MEM_TOP_ADDR = (__MM_SGDMA_MEM_START);
	__MM_DMX_MEM_START_ADDR = (__MM_DMX_MEM_TOP_ADDR - ((__MM_DMX_MEM_LEN * 2) + (TDS_DMX_SEE_BUF_SIZE * 3)));

	/* TSG HW buffer.
	*/
	__MM_TSG_BUF_LEN = 0x40000; /* 256K */
	__MM_TSG_BUF_START_ADDR = (__MM_DMX_MEM_START_ADDR - __MM_TSG_BUF_LEN);

	/* media player buffer */
	__MM_MP_MEM_LEN  = (0xC00000);
	__MM_MP_MEM_TOP_ADDR = (__MM_TSG_BUF_START_ADDR);
	__MM_MP_MEM_START_ADDR = (__MM_MP_MEM_TOP_ADDR - __MM_MP_MEM_LEN);	

	
	__MM_FB_MEM_TOP_ADDR = (__MM_MP_MEM_START_ADDR);
	__MM_FB_MEM_START_ADDR = ((__MM_FB_MEM_TOP_ADDR - FB_MEM_SIZE) & 0xFFFFFFF0);


	/* NIM mode J83B buffer */
	__MM_NIM_J83B_MEM_LEN = (0x00200000); /* 2M */
	__MM_NIM_J83B_MEM_START_ADDR = (__MM_FB_MEM_START_ADDR - __MM_NIM_J83B_MEM_LEN);

	/* image decoder buffer */
	__MM_IMAGE_DECODER_MEM_LEN = (0x00200000); /* 2M */
	__MM_IMAGE_DECODER_MEM_START_ADDR = (__MM_NIM_J83B_MEM_START_ADDR - __MM_IMAGE_DECODER_MEM_LEN);

	/* ovg buffer */
	if (0xFFFFFFFF != __G_MM_OVG_MEM_LEN)
	{
		__MM_OVG_MEM_LEN = __G_MM_OVG_MEM_LEN;
	}
	else
	{
		__MM_OVG_MEM_LEN = (0x00c00000); /* 12M */
	}
    __MM_OVG_MEM_START_ADDR = (__MM_IMAGE_DECODER_MEM_START_ADDR - __MM_OVG_MEM_LEN);

	/* memory mapping configuration end */

	 __G_MM_TOP_ADDR = __MM_TOP_ADDR;

	 __G_MM_VIDEO_TOP_ADDR = (__MM_TOP_ADDR);
	 __G_MM_VIDEO_START_ADDR = (__MM_VIDEO_START_ADDR);

	 __G_MM_VCAP_FB_SIZE = __MM_VCAP_FB_SIZE;
	 __G_MM_VCAP_FB_ADDR = __MM_VCAP_FB_ADDR;

	 __G_MM_STILL_FRAME_SIZE = __MM_STILL_FRAME_SIZE;
	 __G_MM_STILL_FRAME_ADDR = __MM_STILL_FRAME_ADDR;

	 __G_MM_SHARED_MEM_TOP_ADDR = (__MM_SHARED_MEM_TOP_ADDR);
	 __G_MM_SHARED_MEM_START_ADDR = (__MM_SHARED_MEM_SATRT_ADDR);
	 __G_RPC_MM_LEN = __MM_SHARED_MEM_LEN;


	 __G_MM_PRIVATE_AREA_TOP_ADDR= __MM_SHARED_MEM_SATRT_ADDR;
	 __G_MM_PRIVATE_AREA_START_ADDR = (__MM_SHARED_MEM_SATRT_ADDR - __MM_PRIVATE_AREA_LEN);


	 __G_MM_VDEC_VBV_START_ADDR = (__MM_VDEC_VBV_START_ADDR);

	 __G_MM_VDEC_CMD_QUEUE_ADDR = (__MM_VDEC_CMD_QUEUE_ADDR);
	 __G_MM_VDEC_LAF_FLAG_BUF_ADDR = (__MM_VDEC_LAF_FLAG_BUF_ADDR);
	 __G_MM_OSD_BK_ADDR = (__MM_OSD_BK_ADDR2);
	 __G_MM_TTX_BS_START_ADDR = (__MM_TTX_BS_START_ADDR);
	 __G_MM_TTX_PB_START_ADDR = (__MM_TTX_PB_START_ADDR);
	 __G_MM_TTX_SUB_PAGE_BUF_ADDR = (__MM_TTX_SUB_PAGE_BUF_ADDR);
	 __G_MM_TTX_P26_NATION_BUF_ADDR = (__MM_TTX_P26_NATION_BUF_ADDR);
	 __G_MM_TTX_P26_DATA_BUF_ADDR = (__MM_TTX_P26_DATA_BUF_ADDR);
	 __G_MM_SUB_BS_START_ADDR = (__MM_SUB_BS_START_ADDR);
	 __G_MM_SUB_HW_DATA_ADDR = (__MM_SUB_HW_DATA_ADDR);
	 __G_MM_SUB_PB_START_ADDR = (__MM_SUB_PB_START_ADDR);
	 __G_MM_VDEC_VBV_LEN = __MM_VDEC_VBV_LEN;
	 
	 g_fb3_max_width = (FB3_WIDTH);
	 g_fb3_max_height = (FB3_HEIGHT);
	 g_fb3_pitch = (FB3_PITCH);
	 g_fb3_bpp = (FB3_BPP);	 

	 __G_MM_BOOTLOGO_DATA_START_ADDR = (__MM_USER_DATA_MEM_START - __MM_BOOTLOGO_DATA_MEM_LEN);

	 __G_MM_SGDMA_MEM_END = (__MM_SGDMA_MEM_END);
	 __G_MM_SGDMA_MEM_START = (__MM_SGDMA_MEM_START);

	 __G_MM_DMX_MEM_TOP_ADDR = (__MM_DMX_MEM_TOP_ADDR);
	 __G_MM_DMX_MEM_START_ADDR = (__MM_DMX_MEM_START_ADDR);

	 __G_MM_TSG_BUF_LEN = __MM_TSG_BUF_LEN;
	 __G_MM_TSG_BUF_START_ADDR = (__MM_TSG_BUF_START_ADDR);

	 __G_MM_MP_MEM_TOP_ADDR = (__MM_MP_MEM_TOP_ADDR);
	 __G_MM_MP_MEM_START_ADDR = (__MM_MP_MEM_START_ADDR);

	 __G_MM_FB_SIZE = (FB_MEM_SIZE);

	__G_GE_CMD_SIZE = 0;
	__G_MM_FB_START_ADDR = (__MM_FB_MEM_START_ADDR);
	
	g_fb_max_width = (FB_WIDTH);
	g_fb_max_height = (FB_HEIGHT);
	g_fb_pitch = (FB_PITCH);
	g_fb_bpp = (FB_BPP);

	g_support_standard_fb = 1;


	__G_MM_NIM_J83B_MEM_LEN = __MM_NIM_J83B_MEM_LEN;
	__G_MM_NIM_J83B_MEM_START_ADDR = (__MM_NIM_J83B_MEM_START_ADDR);

	__G_MM_IMAGE_DECODER_MEM_LEN = __MM_IMAGE_DECODER_MEM_LEN;
	__G_MM_IMAGE_DECODER_MEM_START_ADDR = (__MM_IMAGE_DECODER_MEM_START_ADDR);
	
	__G_MM_OVG_MEM_LEN = __MM_OVG_MEM_LEN;	
	__G_MM_OVG_MEM_START_ADDR =  __MM_OVG_MEM_START_ADDR;

    
	__G_MM_MAIN_MEM_NUM = 2;
	__G_MM_MAIN_MEM[0][0] = 0xA0000000;		/* 2G */
	__G_MM_MAIN_MEM[0][1] = __MM_OVG_MEM_START_ADDR;
	__G_MM_MAIN_MEM[1][0] = __MM_TOP_ADDR;
	__G_MM_MAIN_MEM[1][1] = 0xB0000000-1;	
    
    

	g_see_heap_top_addr = 0;

	g_tve_default_scart_output = 3;

	g_tve_hd_default_tv_mode = LINE_720_25;
	g_tve_sd_default_tv_mode = PAL;
	g_tve_dac_use_cvbs_type = CVBS_1;
	g_tve_dac_use_svideo_type = -1;
	g_tve_dac_use_rgb_type = RGB_1;
	g_tve_dac_use_yuv_type = YUV_1;
	g_tve_dac_cvbs = DAC3;
	g_tve_dac_svideo_y = 0;
	g_tve_dac_svideo_c = 0;
	g_tve_dac_rgb_r = DAC0;
	g_tve_dac_rgb_g = DAC2;
	g_tve_dac_rgb_b = DAC1;
	g_tve_dac_yuv_y = DAC2;
	g_tve_dac_yuv_u = DAC1;
	g_tve_dac_yuv_v = DAC0;

	g_snd_mute_gpio = 102;//37;

	support_isdbt_cc = 0;
	g_hdmi_hdcp_enable = 0;

	
}


static void set_global_setting_dual(void)
{
	__G_ALI_CHIP_TYPE = 1;
	__G_ALI_CHIP_VERSION = 1; 
	__G_ALI_BOARD_TYPE = 0x00003004;
	__G_ALI_BOARD_VERSION = 1;


	/* memory mapping configuration start */
	/* mapping table description:
	top:
		32M --> reserved for hardware accelerator with memory limitation
		32M --> reserved for SEE CPU
		...    --> reserved for MAIN CPU
	bottom:
	*/
	/* the top address of the memory */
	__MM_TOP_ADDR = (0xA8000000);

	/* frame buffer area defined for video decoder, it is different with
	GMA frame buffer. all the frame buffer must be allocated in the same 32-M segment */
	__MM_VIDEO_AREA_LEN = (0x02000000);
	__MM_VIDEO_START_ADDR = (__MM_TOP_ADDR - __MM_VIDEO_AREA_LEN);

	__MM_VCAP_FB_SIZE = (736*576*2*4);
	__MM_VCAP_FB_ADDR = (__MM_VIDEO_START_ADDR - __MM_VCAP_FB_SIZE);

	__MM_STILL_FRAME_SIZE = (0x300000);
	__MM_STILL_FRAME_ADDR = (__MM_VCAP_FB_ADDR - __MM_STILL_FRAME_SIZE);

	//#define __MM_USER_SPACE2_SIZE		0//(0xA00000 -__MM_VCAP_FB_SIZE - __MM_STILL_FRAME_SIZE)
	//#define __MM_USER_SPACE2_ADDR	(__MM_TOP_ADDR - __MM_VIDEO_AREA_LEN  - __MM_VCAP_FB_SIZE - __MM_STILL_FRAME_SIZE - __MM_USER_SPACE2_SIZE)
	/* shared memory area reserved for the RPC driver*/
	__MM_SHARED_MEM_LEN = (512);
	__MM_SHARED_MEM_TOP_ADDR = (__MM_TOP_ADDR - __MM_VIDEO_AREA_LEN  - __MM_VCAP_FB_SIZE - __MM_STILL_FRAME_SIZE ); //(__MM_TOP_ADDR - 0x02000000)
	__MM_SHARED_MEM_SATRT_ADDR = (__MM_SHARED_MEM_TOP_ADDR - __MM_SHARED_MEM_LEN);

	/* private memory area reserved for the SEE CPU */
	__MM_PRIVATE_AREA_LEN = (0x02000000 - __MM_SHARED_MEM_LEN - __MM_VCAP_FB_SIZE - __MM_STILL_FRAME_SIZE);//(0x02000000 - __MM_SHARED_MEM_LEN)
	__MM_PRIVATE_AREA_SATRT_ADDR = (__MM_SHARED_MEM_SATRT_ADDR - __MM_PRIVATE_AREA_LEN);


	__MM_VDEC_VBV_LEN = 0x600000; //0x380000 //(3.5M)
	__MM_VDEC_CMD_QUEUE_LEN = (0x10000);
	__MM_VDEC_LAF_FLAG_BUF_LEN = (0x10800); // (0xC00*22)
	__MM_OSD2_LEN = (FB3_PITCH * FB3_HEIGHT * FB3_BPP); // (1920 * 1080)
	__MM_TTX_BS_LEN = (0x5000);
	__MM_TTX_PB_LEN = (0xCB200);
	__MM_TTX_SUB_PAGE_LEN = (0x14500);
	__MM_TTX_P26_NATION_LEN = (0x61A80);
	__MM_TTX_P26_DATA_LEN = (0x3E8);
	__MM_SUB_BS_LEN = (0xA000);
#ifdef CONFIG_ALI_M3701G
	__MM_SUB_PB_LEN = (0x50000);
#else
	__MM_SUB_PB_LEN = (0x7E900);
#endif
	__MM_SUB_HW_DATA_LEN =	(0xC000);

	__MM_OSD_BK_ADDR2 = ((__MM_SHARED_MEM_SATRT_ADDR - __MM_OSD2_LEN)&0xFFFFF000);
	__MM_TTX_BS_START_ADDR	= ((__MM_OSD_BK_ADDR2 - __MM_TTX_BS_LEN)&0xFFFFFFFC);
	__MM_TTX_PB_START_ADDR = ((__MM_TTX_BS_START_ADDR - __MM_TTX_PB_LEN)&0xFFFFFFFC);
	__MM_TTX_SUB_PAGE_BUF_ADDR = (__MM_TTX_PB_START_ADDR - __MM_TTX_SUB_PAGE_LEN);
	__MM_TTX_P26_NATION_BUF_ADDR = (__MM_TTX_SUB_PAGE_BUF_ADDR - __MM_TTX_P26_NATION_LEN);
	__MM_TTX_P26_DATA_BUF_ADDR = (__MM_TTX_P26_NATION_BUF_ADDR -  __MM_TTX_P26_DATA_LEN);
	__MM_SUB_BS_START_ADDR = ((__MM_TTX_P26_DATA_BUF_ADDR  - __MM_SUB_BS_LEN)&0xFFFFFFFC);
	__MM_SUB_HW_DATA_ADDR = ((__MM_SUB_BS_START_ADDR - __MM_SUB_HW_DATA_LEN)&0xFFFFFFF0);
	__MM_SUB_PB_START_ADDR = ((__MM_SUB_HW_DATA_ADDR - __MM_SUB_PB_LEN)&0xFFFFFFFC);

	__MM_VDEC_VBV_START_ADDR = (__MM_STILL_FRAME_ADDR + 0x10000000 - __MM_VDEC_VBV_LEN);
	__MM_VDEC_CMD_QUEUE_ADDR = ((__MM_VDEC_VBV_START_ADDR - __MM_VDEC_CMD_QUEUE_LEN)&0xFFFFFF00);  	//256 bytes alignment
	__MM_VDEC_LAF_FLAG_BUF_ADDR = ((__MM_VDEC_CMD_QUEUE_ADDR - __MM_VDEC_LAF_FLAG_BUF_LEN)&0xFFFFFC00);  //1024 bytes alignment

	/* media player buffer */
	__MM_MP_MEM_LEN = (0xC00000);

	__MM_MP_MEM_TOP_ADDR = (__MM_VDEC_LAF_FLAG_BUF_ADDR);
	__MM_MP_MEM_START_ADDR = (__MM_MP_MEM_TOP_ADDR - __MM_MP_MEM_LEN);

	/* user data memory area reserved for bootloader -> kenerl */
	__MM_USER_DATA_MEM_LEN = (0x1000);		// 4k
	__MM_USER_DATA_MEM_START = (__MM_PRIVATE_AREA_SATRT_ADDR - __MM_USER_DATA_MEM_LEN);

	__MM_BOOTLOGO_DATA_MEM_LEN = (0x100000);
	__MM_BOOTLOGO_DATA_MEM_START = (__MM_USER_DATA_MEM_START - __MM_BOOTLOGO_DATA_MEM_LEN);
	/**********************************************************************
	*SGDMA Buffer
	*Warning: MUST KEEP THIS ADDRESS MAPPING TO PHSYCAL ADDRESS BELOW 256M!!
	*		SINCE SGDMA HW CAN ONLY ACCESS PHYSICAL ADDRESS WHICH IS BELOW 256M!!
	*
	*Used for storing SGDMA copy node and sc buffer. Please don't modify this unless 
	*you know what you are doing.
	*
	*Buffer for 8 List Node and 4 Start Code detect
	***********************************************************************/

	//#define __MM_SGDMA_MEM_LEN 0x8000 /* (256 * 16 * 8) + (4 * 4K) = 32K + 16K = 48K. */
	__MM_SGDMA_NODE_MEM_LEN = 0x8000; /* (256 * 16 * 8) = 32K  */
	__MM_SGDMA_MEM_LEN = (0x8000+(4*0x1000)); /* (256 * 16 * 8) + (4 * 4K) = 32K + 16K = 48K. */
	__MM_SGDMA_MEM_END = __MM_BOOTLOGO_DATA_MEM_START;
	__MM_SGDMA_MEM_START = (__MM_SGDMA_MEM_END - __MM_SGDMA_MEM_LEN);	


	/* MAIN CPU DMX HW buffer length.
	 * Warning: MUST KEEP THIS ADDRESS MAPPING TO PHSYCLAL ADDRESS BELOW 256M!!!
	 *          SINCE DMX HW CAN ONLY ACCESS PHSYCLAL ADDRESS WHICH IS BELOW 256M!
	 */
	__MM_DMX_MEM_LEN = (0x00100000); /* 1M */

	__MM_DMX_MEM_TOP_ADDR = (__MM_SGDMA_MEM_START);
	__MM_DMX_MEM_START_ADDR = (__MM_DMX_MEM_TOP_ADDR - ((__MM_DMX_MEM_LEN * 2) + (TDS_DMX_SEE_BUF_SIZE * 3)));

	/* TSG HW buffer.
	*/
	__MM_TSG_BUF_LEN = 0x40000; /* 256K */
	__MM_TSG_BUF_START_ADDR = (__MM_DMX_MEM_START_ADDR - __MM_TSG_BUF_LEN);


	__MM_FB_MEM_TOP_ADDR = (__MM_TSG_BUF_START_ADDR);
	__MM_FB_MEM_START_ADDR = ((__MM_FB_MEM_TOP_ADDR - FB_MEM_SIZE) & 0xFFFFFFF0);


	/* NIM mode J83B buffer */
	__MM_NIM_J83B_MEM_LEN = (0x00200000); /* 2M */
	__MM_NIM_J83B_MEM_START_ADDR = (__MM_FB_MEM_START_ADDR - __MM_NIM_J83B_MEM_LEN);
	/* image decoder buffer */
	//#define __MM_IMAGE_DECODER_MEM_LEN                		(0x00200000) /* 2M */
	//#define __MM_IMAGE_DECODER_MEM_START_ADDR 	 	(__MM_FB_MEM_START_ADDR - __MM_IMAGE_DECODER_MEM_LEN)

	__MM_IMAGE_DECODER_MEM_LEN = (0x00200000); /* 2M */
	__MM_IMAGE_DECODER_MEM_START_ADDR =	(__MM_MP_MEM_START_ADDR - __MM_IMAGE_DECODER_MEM_LEN);

	/* ovg buffer */
	if (0xFFFFFFFF != __G_MM_OVG_MEM_LEN)
	{
		__MM_OVG_MEM_LEN = __G_MM_OVG_MEM_LEN;
	}
	else
	{
		__MM_OVG_MEM_LEN = (0x00c00000); /* 12M */
	}
    __MM_OVG_MEM_START_ADDR = (__MM_IMAGE_DECODER_MEM_START_ADDR - __MM_OVG_MEM_LEN);
	

	/* memory mapping configuration end */
	
	__G_MM_TOP_ADDR = __MM_TOP_ADDR;
	__G_MM_VIDEO_TOP_ADDR = (__MM_TOP_ADDR + 0x10000000);
	__G_MM_VIDEO_START_ADDR = (__MM_VIDEO_START_ADDR + 0x10000000);
	__G_MM_VCAP_FB_SIZE = __MM_VCAP_FB_SIZE;
	__G_MM_VCAP_FB_ADDR = __MM_VCAP_FB_ADDR + 0x10000000;
	__G_MM_STILL_FRAME_SIZE = __MM_STILL_FRAME_SIZE;
	__G_MM_STILL_FRAME_ADDR = __MM_STILL_FRAME_ADDR + 0x10000000;
	__G_MM_SHARED_MEM_TOP_ADDR = (__MM_SHARED_MEM_TOP_ADDR);
	__G_MM_SHARED_MEM_START_ADDR = (__MM_SHARED_MEM_SATRT_ADDR);
	__G_RPC_MM_LEN = __MM_SHARED_MEM_LEN;
	__G_MM_PRIVATE_AREA_TOP_ADDR = (__MM_SHARED_MEM_SATRT_ADDR);
	__G_MM_PRIVATE_AREA_START_ADDR = (__MM_SHARED_MEM_SATRT_ADDR										- __MM_PRIVATE_AREA_LEN);
	__G_MM_VDEC_VBV_START_ADDR = (__MM_VDEC_VBV_START_ADDR);
	__G_MM_VDEC_CMD_QUEUE_ADDR = (__MM_VDEC_CMD_QUEUE_ADDR);
	__G_MM_VDEC_LAF_FLAG_BUF_ADDR = (__MM_VDEC_LAF_FLAG_BUF_ADDR);
	
	__G_MM_MP_MEM_TOP_ADDR = (__MM_MP_MEM_TOP_ADDR);
	__G_MM_MP_MEM_START_ADDR = (__MM_MP_MEM_START_ADDR);
	
	__G_MM_OSD_BK_ADDR = (__MM_OSD_BK_ADDR2);
	__G_MM_TTX_BS_START_ADDR = (__MM_TTX_BS_START_ADDR);
	__G_MM_TTX_PB_START_ADDR = (__MM_TTX_PB_START_ADDR);
	__G_MM_TTX_SUB_PAGE_BUF_ADDR = (__MM_TTX_SUB_PAGE_BUF_ADDR);
	__G_MM_TTX_P26_NATION_BUF_ADDR = (__MM_TTX_P26_NATION_BUF_ADDR);
	__G_MM_TTX_P26_DATA_BUF_ADDR = (__MM_TTX_P26_DATA_BUF_ADDR);
	__G_MM_SUB_BS_START_ADDR = (__MM_SUB_BS_START_ADDR);
	__G_MM_SUB_HW_DATA_ADDR = (__MM_SUB_HW_DATA_ADDR);
	__G_MM_SUB_PB_START_ADDR = (__MM_SUB_PB_START_ADDR);
	g_fb3_max_width = (FB3_WIDTH);
	g_fb3_max_height = (FB3_HEIGHT);
	g_fb3_pitch = (FB3_PITCH);
	g_fb3_bpp = (FB3_BPP);
	__G_MM_VDEC_VBV_LEN = __MM_VDEC_VBV_LEN;
	
	__G_MM_BOOTLOGO_DATA_START_ADDR = (__MM_USER_DATA_MEM_START - __MM_BOOTLOGO_DATA_MEM_LEN);
	
	__G_MM_SGDMA_MEM_END = (__MM_SGDMA_MEM_END);
	__G_MM_SGDMA_MEM_START = (__MM_SGDMA_MEM_START);
	
	__G_MM_DMX_MEM_TOP_ADDR = (__MM_DMX_MEM_TOP_ADDR);
	__G_MM_DMX_MEM_START_ADDR = (__MM_DMX_MEM_START_ADDR);
	
	__G_MM_TSG_BUF_LEN = __MM_TSG_BUF_LEN;
	__G_MM_TSG_BUF_START_ADDR = (__MM_TSG_BUF_START_ADDR);
	
	__G_MM_FB_SIZE = (FB_MEM_SIZE);
	__G_GE_CMD_SIZE = (GE_CMD_SIZE);
	__G_MM_FB_START_ADDR = (__MM_FB_MEM_START_ADDR);
	g_fb_max_width = (FB_WIDTH);
	g_fb_max_height = (FB_HEIGHT);
	g_fb_pitch = (FB_PITCH);
	g_fb_bpp = (FB_BPP);	
	g_support_standard_fb = 1;

	__G_MM_NIM_J83B_MEM_LEN = __MM_NIM_J83B_MEM_LEN;
	__G_MM_NIM_J83B_MEM_START_ADDR = (__MM_NIM_J83B_MEM_START_ADDR);
	
	__G_MM_IMAGE_DECODER_MEM_LEN = __MM_IMAGE_DECODER_MEM_LEN;
	__G_MM_IMAGE_DECODER_MEM_START_ADDR = (__MM_IMAGE_DECODER_MEM_START_ADDR);

	__G_MM_OVG_MEM_LEN = __MM_OVG_MEM_LEN;	
	__G_MM_OVG_MEM_START_ADDR =  __MM_OVG_MEM_START_ADDR;
	
	
		
	g_tve_default_scart_output = 3;
	
	/* default tv mode */
	g_tve_hd_default_tv_mode = LINE_720_25;
	g_tve_sd_default_tv_mode = PAL;
	
	/* whether use the CVBS output. the invalid value is -1 */
	g_tve_dac_use_cvbs_type = CVBS_1;
	
	/* whether use the SVIDEO output. the invalid value is -1 */
	g_tve_dac_use_svideo_type = -1;
	
	/* whether use the RGB output. the invalid value is -1 */
	g_tve_dac_use_rgb_type = RGB_1;
	
	/* whether use the YUV output. the invalie value is -1 */
	g_tve_dac_use_yuv_type = YUV_1;
	
	/* CVBS dac definition */
	g_tve_dac_cvbs = DAC3;
	
	/* SVIDEO dac definition */
	g_tve_dac_svideo_y = 0;
	g_tve_dac_svideo_c = 0;
	
	/* RGB dac definition */
	g_tve_dac_rgb_r = DAC0;
	g_tve_dac_rgb_g = DAC2;
	g_tve_dac_rgb_b = DAC1;
	
	/* YUV dac definition */
	g_tve_dac_yuv_y = DAC2;
	g_tve_dac_yuv_u = DAC1;
	g_tve_dac_yuv_v = DAC0;
		
	/* GPIO definition start */
	g_snd_mute_gpio = 56;//37;
	
	/* GPIO definition end */
	
	/* feature list */
	support_isdbt_cc = 0;
	//hdmi hdcp on/off
	g_hdmi_hdcp_enable = 0;
	
	//set ali openvg mempool(including boardsetting) memory alloc config
	
}


static void set_global_setting_384m(void)
{
	set_global_setting_dual();
	
	__G_MM_MAIN_MEM_NUM = 2;
	
	__G_MM_MAIN_MEM[0][0] = 0xA0000000;
	__G_MM_MAIN_MEM[0][1] = __MM_NIM_J83B_MEM_START_ADDR;
	__G_MM_MAIN_MEM[1][0] = __MM_SHARED_MEM_TOP_ADDR;
	__G_MM_MAIN_MEM[1][1] = __MM_OVG_MEM_START_ADDR;	
}


static void set_global_setting_512m(void)
{
	set_global_setting_dual();
	
	__G_MM_MAIN_MEM_NUM = 3;
	
	__G_MM_MAIN_MEM[0][0] = 0xA0000000;
	__G_MM_MAIN_MEM[0][1] = __MM_NIM_J83B_MEM_START_ADDR;
	__G_MM_MAIN_MEM[1][0] = __MM_SHARED_MEM_TOP_ADDR;
	__G_MM_MAIN_MEM[1][1] = __MM_OVG_MEM_START_ADDR;
	__G_MM_MAIN_MEM[2][0] = 0xB8000000 + 0x20000000;
	__G_MM_MAIN_MEM[2][1] =  0xC0000000 + 0x20000000;
}

#endif
#if 0
static void set_global_setting(void)
{

	ali_user_data *user_data= NULL;	

	user_data = (ali_user_data *)ali_get_user_data();

	if (NULL != user_data)
	{
		printk("user_data->ddr_type = 0x%x\n", user_data->ddr_type);
		printk("user_data->ddr_size = 0x%x\n", user_data->ddr_size);

		if ((1 == user_data->ddr_type) && (0x100 == user_data->ddr_size))
		{
			printk("256m single\n");
			set_global_setting_256m();
		}
		else if((2 == user_data->ddr_type) && (0x180 == user_data->ddr_size))
		{
			printk("384m dual\n");
			set_global_setting_384m();
		}
		else if((2 == user_data->ddr_type) && (0x200 == user_data->ddr_size))
		{
			printk("512m dual\n");
			set_global_setting_512m();
		}
		else
		{
			printk("default, select 256m single\n");
			set_global_setting_256m();
		}
	}
	else
	{
		printk("default, select 256m single\n");
		set_global_setting_256m();
	}

}
#endif
#if 0
void show_board_setting(void)
{
	int i = 0;
	
	printk("__G_MM_TOP_ADDR                  0x%08x\n", 
	 		(unsigned int)__G_MM_TOP_ADDR);

    printk("__G_MM_VIDEO_START_ADDR           0x%08x-0x%08x\n",  
    		(unsigned int)__G_MM_VIDEO_START_ADDR, 
    		(unsigned int)(__G_MM_VIDEO_START_ADDR+__G_MM_VIDEO_TOP_ADDR));
	

    printk("__G_MM_VCAP_FB_ADDR               0x%08x-0x%08x\n",  
    		(unsigned int)__G_MM_VCAP_FB_ADDR, 
    		(unsigned int)(__G_MM_VCAP_FB_ADDR+__G_MM_VCAP_FB_SIZE));
   
    printk("__G_MM_STILL_FRAME_ADDR           0x%08x-0x%08x\n", 
    		(unsigned int)__G_MM_STILL_FRAME_ADDR, 
    		(unsigned int)(__G_MM_STILL_FRAME_ADDR + __G_MM_STILL_FRAME_SIZE));	        
	
    printk("__G_MM_SHARED_MEM_START_ADDR      0x%08x-0x%08x\n" ,
    		(unsigned int)__G_MM_SHARED_MEM_START_ADDR, 
    		(unsigned int)__G_MM_SHARED_MEM_TOP_ADDR);    
    
	printk("__G_RPC_MM_LEN                    0x%08x\n",  
			(unsigned int)__G_RPC_MM_LEN );			
   
    printk("__G_MM_PRIVATE_AREA_START_ADDR    0x%08x-0x%08x\n", 
    		(unsigned int)__G_MM_PRIVATE_AREA_START_ADDR,
    		(unsigned int)__G_MM_PRIVATE_AREA_TOP_ADDR);
    
    printk("__G_MM_VDEC_VBV_START_ADDR        0x%08x\n", 
    		(unsigned int)__G_MM_VDEC_VBV_START_ADDR );
    	
    printk("__G_MM_VDEC_CMD_QUEUE_ADDR        0x%08x\n",  
    		(unsigned int)__G_MM_VDEC_CMD_QUEUE_ADDR );
    	
    printk("__G_MM_VDEC_LAF_FLAG_BUF_ADDR     0x%08x\n", 
    		(unsigned int)__G_MM_VDEC_LAF_FLAG_BUF_ADDR );
    	
    printk("__G_MM_OSD_BK_ADDR                0x%08x\n",  
    		(unsigned int)__G_MM_OSD_BK_ADDR );
    	
    printk("__G_MM_TTX_BS_START_ADDR          0x%08x\n", 
    		(unsigned int)__G_MM_TTX_BS_START_ADDR );
    	
    printk("__G_MM_TTX_PB_START_ADDR          0x%08x\n", 
    		(unsigned int)__G_MM_TTX_PB_START_ADDR );
    	
    printk("__G_MM_TTX_SUB_PAGE_BUF_ADDR      0x%08x\n",  
    		(unsigned int)__G_MM_TTX_SUB_PAGE_BUF_ADDR );
    		
    printk("__G_MM_TTX_P26_NATION_BUF_ADDR    0x%08x\n",  
    		(unsigned int)__G_MM_TTX_P26_NATION_BUF_ADDR );
    		
    printk("__G_MM_TTX_P26_DATA_BUF_ADDR      0x%08x\n", 
    		(unsigned int)__G_MM_TTX_P26_DATA_BUF_ADDR );
    		
    printk("__G_MM_SUB_BS_START_ADDR          0x%08x\n",  
    		(unsigned int)__G_MM_SUB_BS_START_ADDR );
    		
    printk("__G_MM_SUB_HW_DATA_ADDR           0x%08x\n", 
    		(unsigned int)__G_MM_SUB_HW_DATA_ADDR );
    		
    printk("__G_MM_SUB_PB_START_ADDR          0x%08x\n",  
    		(unsigned int)__G_MM_SUB_PB_START_ADDR );
    		
    printk("__G_MM_VDEC_VBV_LEN               0x%08x\n",  
    		(unsigned int)__G_MM_VDEC_VBV_LEN );
    		
    printk("__G_MM_BOOTLOGO_DATA_START_ADDR   0x%08x\n", 
    		(unsigned int)__G_MM_BOOTLOGO_DATA_START_ADDR );
    		
    printk("__G_MM_SGDMA_MEM_START            0x%08x-0x%08x\n", 
    		(unsigned int)__G_MM_SGDMA_MEM_START,
    		(unsigned int)__G_MM_SGDMA_MEM_END);
    
    printk("__G_MM_DMX_MEM_START_ADDR         0x%08x-0x%08x\n",  
    		(unsigned int)__G_MM_DMX_MEM_START_ADDR,
    		(unsigned int)__G_MM_DMX_MEM_TOP_ADDR);

    
    printk("__G_MM_TSG_BUF_START_ADDR         0x%08x-0x%08x\n",  
    		(unsigned int)__G_MM_TSG_BUF_START_ADDR,
    		(unsigned int)(__G_MM_TSG_BUF_START_ADDR+__G_MM_TSG_BUF_LEN));
    		
 
    printk("__G_MM_MP_MEM_START_ADDR          0x%08x-0x%08x\n",  
    		(unsigned int)__G_MM_MP_MEM_START_ADDR,
    		(unsigned int)__G_MM_MP_MEM_TOP_ADDR);

    
    printk("__G_MM_FB_START_ADDR              0x%08x-0x%08x\n",  
    	(unsigned int)__G_MM_FB_START_ADDR,
    	(unsigned int)(__G_MM_FB_START_ADDR+__G_MM_FB_SIZE));


	printk("__G_GE_CMD_SIZE              	   0x%08x\n",  
    	(unsigned int)__G_GE_CMD_SIZE);

    printk("__G_MM_NIM_J83B_MEM_START_ADDR     0x%08x-0x%08x\n",  
    	(unsigned int)__G_MM_NIM_J83B_MEM_START_ADDR,
    	(unsigned int)(__G_MM_NIM_J83B_MEM_START_ADDR+__G_MM_NIM_J83B_MEM_LEN));

	printk("__G_MM_IMAGE_DECODER_MEM_START_ADDR     0x%08x-0x%08x\n",  
    	(unsigned int)__G_MM_IMAGE_DECODER_MEM_START_ADDR,
    	(unsigned int)(__G_MM_IMAGE_DECODER_MEM_START_ADDR+__G_MM_IMAGE_DECODER_MEM_LEN));

	printk("__G_MM_OVG_MEM_START_ADDR     0x%08x-0x%08x\n",  
    	(unsigned int)__G_MM_OVG_MEM_START_ADDR,
    	(unsigned int)(__G_MM_OVG_MEM_START_ADDR+__G_MM_OVG_MEM_LEN));


    for(i=0; i<__G_MM_MAIN_MEM_NUM; i++)
	{			
	    printk("[ %s %d ],__G_MM_MAIN_MEM[%d]	0x%08x-0x%08x\n", 
	    		__FUNCTION__, __LINE__, i,
	    		(unsigned int)__G_MM_MAIN_MEM[i][0], 
	    		(unsigned int)__G_MM_MAIN_MEM[i][1]);		
	}

	
}
#endif

void customize_board_setting(void)
{
#if 0
	unsigned long data = 0;
	unsigned char buf[4];
	set_global_setting();
	
		if(__G_ALI_BOARD_TYPE >= 0x0003000) // M3701C
		{
#if 0	
			ddr_reg_dump();
#endif
			// disable all GPIO
			*((volatile unsigned long *)0xb8000430) = 0;
			*((volatile unsigned long *)0xb8000434) = 0;
			*((volatile unsigned long *)0xb8000438) = 0;
			*((volatile unsigned long *)0xb800043c) = 0;
			*((volatile unsigned long *)0xb8000440) = 0;
	
			data = *(volatile unsigned long *)0xb8000000;
			data = (data>>8) & 0x7;
			switch(data)
			{
				case 1:
					__G_ALI_BOARD_TYPE = 0x00003001;
					break;
				case 2:
					__G_ALI_BOARD_TYPE = 0x00003004;
					break;
				case 3:
					__G_ALI_BOARD_TYPE = 0x00003003;
					break;
				case 4:
					__G_ALI_BOARD_TYPE = 0x00003002;
					break;
				default:
					break;
			}
	
			g_see_heap_top_addr = 0;
			if(__G_ALI_BOARD_TYPE == 0x00003001)
			{
				data = *((volatile unsigned long *)0xb8000088);
				data |= (1 << 19);	// i2c3
				*((volatile unsigned long *)0xb8000088) = data;
			}
			else if(__G_ALI_BOARD_TYPE == 0x00003002)
			{
				__G_MM_RESERVED_MEM_SIZE = 0x4000;
				__G_MM_RESERVED_MEM_ADDR = __G_MM_PRIVATE_AREA_START_ADDR
					- 0x1000 - __G_MM_RESERVED_MEM_SIZE;
				
				__G_MM_BOARD_SETTING_ADDR = __G_MM_RESERVED_MEM_ADDR + 0x300;
	
				g_see_heap_top_addr = __G_MM_PRIVATE_AREA_TOP_ADDR;
			}
			else if(__G_ALI_BOARD_TYPE == 0x00003003)
			{
			 
				data = *((volatile unsigned long *)0xb800008c);
				data &= ~(1 << 7);	// i2c3
				data |= (1 << 15);	// i2c3
				*((volatile unsigned long *)0xb800008c) = data;
				*((volatile unsigned long *)0xb8000434) |= 6; //for HDMI I2c, GPIO 33/34 
			}
			else if(__G_ALI_BOARD_TYPE == 0x00003004)
			{
				*((volatile unsigned long *)0xb8000434) |= 6; //for HDMI I2c, GPIO 33/34	
			}
		
			//ddr priority adjust	
			*(volatile unsigned long *)0xB800100C = 0xFB534055;
			*(volatile unsigned long *)0xB8001010 = 0xFDEFCFFF;
			*(volatile unsigned long *)0xB8001018 = 0x88888880;
			*(volatile unsigned long *)0xB8001024 = 0x0F0FFFFF;
			*(volatile unsigned long *)0xB800102C = 0x33330F00;
			*(volatile unsigned long *)0xB8000224 = 0x000020FF;
	
			boot_delay(10);
			phy_chip_setting();
	
			otp_read_func(0x85<<2, buf,4);
			g_otp_set_vdac_fs=(unsigned int)(buf[0]&0x07);
	
			// for see cpu dbg info
			*(volatile unsigned int *)0xB8040024 |= 1;
		}
#endif
}

