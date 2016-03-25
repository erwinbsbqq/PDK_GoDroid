/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2009 Copyright (C)
 *  (C)
 *  File: alifb.c
 *  (I)
 *  Description: the frame buffer driver for ali's STB chipset
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2009.12.16				Sam		Create
 * 1.       2012.10.24              Christian   Add  
 ****************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/version.h>
#include <linux/ali_reg.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/div64.h>

#ifdef CONFIG_RPC_HLD_GMA
#include <linux/i2c.h>
#include <rpc_hld/ali_rpc_hld_dis.h>
#endif

#include <ali_video_common.h>
#include <ali_decv_plugin_common.h>
#include <ali_interrupt.h>
#include <ali_cache.h>
#include <ali_shm.h>

#include "ali_fb.h"
#include "ali_gma.h"
#include "ali_vpo.h"
#include "ali_accel.h"

#define ALI_FB_DEVICE_NAME		"alifb"

unsigned long g_fb_dbg_flag = 0;

extern unsigned long __G_ALI_MM_GE_CMD_SIZE;
extern unsigned long __G_ALI_MM_GE_CMD_START_ADDR;

extern unsigned long __G_ALI_MM_FB0_SIZE;
extern unsigned long __G_ALI_MM_FB0_START_ADDR;

extern unsigned long g_fb0_max_width;
extern unsigned long g_fb0_max_height;
extern unsigned long g_fb0_width;
extern unsigned long g_fb0_height;
extern unsigned long g_fb0_pitch;
extern unsigned long g_fb0_bpp;

extern unsigned long __G_ALI_MM_FB2_SIZE;
extern unsigned long __G_ALI_MM_FB2_START_ADDR;

extern unsigned long g_fb2_max_width;
extern unsigned long g_fb2_max_height;
extern unsigned long g_fb2_width;
extern unsigned long g_fb2_height;
extern unsigned long g_fb2_pitch;
extern unsigned long g_fb2_bpp;

extern unsigned long __G_ALI_MM_FB0_CMAP_SIZE;
extern unsigned long __G_ALI_MM_FB0_CMAP_START_ADDR;

extern unsigned long __G_ALI_MM_FB2_CMAP_SIZE;
extern unsigned long __G_ALI_MM_FB2_CMAP_START_ADDR;

extern unsigned long g_support_standard_fb;

extern struct vpo_device *g_vpo_dev;
extern struct vpo_device *g_sd_vpo_dev;

extern unsigned long __G_GE_CMD_SIZE;

#define Y_BUF_SIZE	 0x1FE000
#define C_BUF_SIZE	 0xFF000
#define MAF_BUF_SIZE 0x2000

struct backup_pic_addr
{
    UINT8 *backup_pic_y_buffer;
    UINT8 *backup_pic_c_buffer;
    UINT8 *backup_pic_maf_buffer;
};

static struct backup_pic_addr cc_backup_info;

#ifdef CONFIG_RPC_HLD_GMA
int g_fb_gma_disable_enhance = 0;

/***phil add test start***/
#define DSUB_EDDC_ADAPTER_ID	4	/* D-Sub adapter id set in m39_setup.c */
#define RES_COMMON_ERROR	    0xFFFFF000
#define	RES_NO_CONNECT	        0xFFFFFF00
#define RES_WXGA                5   //1360*768
#define	RES_XGA                 6   //1024*768
#define	RES_VGA                 7   //800*600
static unsigned int GetSupportListDSUB(void)
{
    unsigned char segment = 0x00, word_offset = 0x00;
    unsigned char edid_data[128] = {0};     //c++test need not initialize
    struct i2c_msg msgs[] = {
        { /* DDC Addr 0x60, Segment Pointer = 0 */
            .addr	= 0x30,
            .flags	= I2C_M_IGNORE_NAK,
            .len	= 1,
            .buf	= &segment,
        },
        { /* DDC Addr 0xA0, Word Offset = 0 */
            .addr	= 0x50,
            .flags	= 0,
            .len	= 1,
            .buf	= &word_offset,
        },
        { /* DDC Addr 0xA1, WRead = 0 */
            .addr	= 0x50,
            .flags	= I2C_M_RD,
            .len	= 128,
            .buf	= edid_data,
        }
    };

	bool b_WXGA = false;
	bool b_XGA = false;
	bool b_VGA = false;
	bool b_16_9 = false;

	int i = 0;
	u32 tmp_u32 = 0;
	struct i2c_adapter *adapter;

	adapter = i2c_get_adapter(DSUB_EDDC_ADAPTER_ID);
	if (i2c_transfer(adapter, msgs, 3) == 3) {
	    //for(i=0; i<128; i++) {
	        //printk("%.2x %s", edid_data[i],(i%16 == 15) ? "\n":"");
		//}
	}
	else
	{
		return RES_NO_CONNECT;
	}

	/* EDID Header 8 bytes: 0x00 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0x00 */
	if (edid_data[0] != 0x0 || edid_data[1] != 0xFF || edid_data[2] != 0xFF || \
		edid_data[3] != 0xFF || edid_data[4] != 0xFF || edid_data[5] != 0xFF || \
		edid_data[6] != 0xFF || edid_data[7] != 0x0) {
			printk("RES_ERR0\n");
			return RES_COMMON_ERROR;	// error EDID Header
	}

	/* EDID version 1.4: Version(12h) 0x01 Revision(13h) 0x04 */
	if (edid_data[0x12] == 0x1 && edid_data[0x13] == 0x4) {
		/* Timings III:
		 EDID addr offset:	48  -  -  -  4C h	4D	h	18 bytes
							5A  -  -  -  5E h	5F	h	18 bytes
							6C  -  -  -  70 h	71	h	18 bytes
		 EDIE value:		(00	00 00 F7 00)h	0A	h */
		 for (i = 0; i < 54; i += 18) {
			if (edid_data[0x48+i] == 0x0 && edid_data[0x49+i] == 0x0 && \
				edid_data[0x4A+i] == 0x0 && edid_data[0x4B+i] == 0xF7 && \
				edid_data[0x4C+i] == 0x0 && edid_data[0x4D+i] == 0xA) {
				/*1360X768@60Hz: EDID addr offset 50/62/74 h
				
				v	1 _ _ _ _ _ _ _ 1360 x 768 @ 60 Hz
					_ 1 _ _ _ _ _ _ 1440 x 900 @ 60 Hz (RB)
					_ _ 1 _ _ _ _ _ 1440 x 900 @ 60 Hz
					_ _ _ 1 _ _ _ _ 1440 x 900 @ 75 Hz
					_ _ _ _ 1 _ _ _ 1440 x 900 @ 85 Hz
					_ _ _ _ _ 1 _ _ 1400 x 1050 @ 60 Hz (RB)
					_ _ _ _ _ _ 1 _ 1400 x 1050 @ 60 Hz
					_ _ _ _ _ _ _ 1 1400 x 1050 @ 75 Hz
				*/
				if ((edid_data[0x50+i] >> 7) == 1) {
					b_WXGA = true;
					b_16_9 = true;
					break;
				}
			}
		}
	}

	/* EDID version 1.4 & 1.3: Version(12h) 0x01 Revision(13h) 0x04 or 0x03	*/
	/* support version  1.x */
	if (edid_data[0x12] == 0x1 /*&& \
		(edid_data[0x13] == 0x4 || edid_data[0x13] == 0x3)*/) {
		/* Standard Timings: 8B C0h [1360 x 768(765) @ 60 Hz]
		Addr
		/Offset
				Value Stored (in hex) = (Horizontal addressable pixels ¡Ò 8) ¡V 31
		  26h	Range: 256 pixels ¡÷ 2288 pixels, in increments of 8 pixels
				00h Reserved: Do not use.

				Bit Definitions 			Description
				7 6 _ _ _ _ _ _ Image Aspect Ratio: bits 7 & 6
				0 0 _ _ _ _ _ _ 16 : 10 AR
		  		0 1 _ _ _ _ _ _ 4 : 3 AR
				1 0 _ _ _ _ _ _ 5 : 4 AR
		  27h	1 1 _ _ _ _ _ _ 16 : 9 AR

				5 4 3 2 1 0 	Field Refresh Rate: bits 5 ¡÷ 0
							Value Stored (in binary) = Field Refresh Rate (in Hz) ¡V 60
				n n n n n n
							Range: 60 Hz ¡÷ 123Hz 

		28h,	29h	Standard Timing 2: Stored values use the Standard Timing 1 byte 
										and bit definitions.
		2Ah,	2Bh	Standard Timing 3:
		2Ch,2Dh	Standard Timing 4:
		2Eh,2Fh	Standard Timing 5:
		30h,31h	Standard Timing 6:
		32h,33h	Standard Timing 7:
		34h,35h	Standard Timing 8:											*/
		if (!b_WXGA) {
			for (i=0; i < 8; i = i+2) {
				if (edid_data[0x26+i] == 0x1 && edid_data[0x27+i] == 0x1)
					continue;	/* Unused Standard Timing data fields */
				else if (edid_data[0x26+i] == 0X8B && edid_data[0x27+i] == 0xC0) {
					b_WXGA = true;
					b_16_9 = true;
					break;
				}
				else if ((edid_data[0x27+i] >> 6) == 0 || 	/* 16:10 */
					(edid_data[0x27+i] >> 6) == 3) {		/* 16:9 */
					b_16_9 = true;
				}
			}
		}

		/* 16:9 Aspect ration & 1360+ horizontal pixels in Preferred Timing -> WXGA */
/* EDID 1.4: 
Address  2 Bytes	Value 					Description
				01h ¡÷ FFh	If byte 16h ¡Ú 00h then byte 15h = Horizontal Screen Size in cm.
							(Range is 1 cm ¡÷ 255 cm)
  15h	1		01h ¡÷ FFh	If byte 16h = 00h then byte 15h = Aspect Ratio (Landscape)
							(Range is 1 : 1 AR ¡÷ 3.54 : 1 AR)
				00h 			If byte 15h = 00h then byte 16h = Aspect Ratio (Portrait)

				01h ¡÷ FFh	If byte 15h ¡Ú 00h then byte 16h = Vertical Screen Size in cm.
							(Range is 1 cm ¡÷ 255 cm)
				01h ¡÷ FFh	If byte 15h = 00h then byte 16h = Aspect Ratio (Portrait)
  16h	1					(Range is 0.28 : 1 AR ¡÷ 0.99 : 1 AR)
				00h			If byte 16h = 00h then byte 15h = Aspect Ratio (Landscape)

  15h,	2 		00h, 00h	If both bytes 15h and 16h = 00h then the screen size or aspect
  16h						ratio are unknown or undefined.

    EDID 1.3:   15h 	Maximum Horizontal Image Size (in centimeters).
			16h 	Maximum Vertical Image Size (in centimetres).

    Landscape Orientation:
	Given the Stored Value, the Aspect Ratio may be calculated by using the following equation:
						Aspect Ratio = (Stored Value + 99) ¡Ò 100
	Given the Aspect Ratio, the Stored Value may be calculated by using the following equation:
						Stored Value = (Aspect Ratio ¡Ñ 100) ¡V 99
	For an aspect ratio of 16 by 9, the stored value at address 15h is 79 (4Fh).
	For an aspect ratio of 16 by 10, the stored value at address 15h is 61 (3Dh).

    Portrait Orientation:
	Given the Stored Value, the Aspect Ratio may be calculated by using the following equation:
						Aspect Ratio = 100 ¡Ò (Stored Value + 99)
	Given the Aspect Ratio, the Stored Value may be calculated by using the following equation:
						Stored Value = (100 ¡Ò Aspect Ratio) ¡V 99
	For an aspect ratio of 9 by 16, the stored value at address 16h is 79 (4Fh).
	For an aspect ratio of 10 by 16, the stored value at address 16h is 61 (3Dh).
*/
		if (!b_16_9 && (edid_data[0x15] != 0 || edid_data[0x16] != 0)) {
			if (edid_data[0x15] != 0 && edid_data[0x16] != 0) {
				if ((edid_data[0x15] *10 / edid_data[0x16]) >= 16)	/* 16:9, 16:10 */
					b_16_9 = true;
			}
			else {
				if (edid_data[0x15] == 0x4F || edid_data[0x15] == 0x3D ||	/* Landscape */
					edid_data[0x16] == 0x4F || edid_data[0x16] == 0x3D)		/* Portrait */
					b_16_9 = true;
			}
		}

		/* Preferred Timing: EDID addr offset 36h ¡÷ 47h
			Addr#	# ofBytes	Value 		Detailed Timing Definitions
			38h 	1			00h ¡÷ FFh 	Horizontal Addressable Video in pixels 
											--- contains lower 8 bits
			3Ah		1			0<=HA<= Fh	Horizontal Addressable Video in pixels
											--- stored in Upper Nibble : contains upper 4 bits
		*/
		if (b_16_9) {	/* 1360+ horizontal pixels in Preferred Timing */
			tmp_u32 = edid_data[0x3A];
			if ((((tmp_u32 >> 4) << 8) + edid_data[0x38]) >= 1360) {
				b_WXGA = true;
			}
		}


		/* Timings I: EDID addr offset 23h
			ùÝ Supported Established Timings II include
		 	ùà 720x400@70Hz,
			ùà 720x400@88Hz,
		 	ùà 640x480@60Hz,
			ùà 640x480@67Hz,
		 	ùà 640x480@72Hz,
			ùà 640x480@75Hz,
		 	ùà 800x600@56Hz,
		v	ùã 800x600@60Hz


		    Timings II: EDID addr offset 24h
			ùÝ Supported Established Timings II include
		 	ùà 800x600@72Hz,
			ùà 800x600@75Hz,
			ùà 832x624@75Hz,
			ùá 1024x768@87Hz (I),
		v	ùà 1024x768@60Hz,
			ùà 1024x768@70Hz,
			ùà 1024x768@75Hz,
			ùã 1280x1024@75Hz
		*/
		if ((edid_data[0x24] & 0x8) > 0)
			b_XGA = true;
		if ((edid_data[0x23] & 0x01) == 1)
			b_VGA = true;
	}
	else {	// error or not support EDID Version
			printk("RES_ERR1\n");
		return RES_COMMON_ERROR;
	}

	/* Set support resoluiton into char* pstring */
	if (b_WXGA) {
		return RES_WXGA;
		//printk("1360x768\n");
	}
	if (b_XGA) {
		return RES_XGA;
		//printk("1024x768\n");
	}
	if (b_VGA) {
		return RES_VGA;
		//printk("800x600\n");
	}
	return RES_NO_CONNECT;

}

/***phil add end****/
#endif

int hld_dis_rpc_operation(int hd_dev, int API_idx);

#ifdef CONFIG_RPC_HLD_GMA
static struct alifb_info *m_fb_info = NULL;
#else
int hld_dis_fb_full_screen(int layer_id, int src_width, int src_height);
#endif

volatile unsigned long *g_ali_fb_rpc_arg[MAX_FB_RPC_ARG_NUM];
volatile int g_ali_fb_rpc_arg_size[MAX_FB_RPC_ARG_NUM];

static int init_gma_info(struct alifb_info *info)
{
	/* build the needed gma info */
	info->gma_init = 1;
	info->gma_buf_size = sizeof(struct alifb_gma_header) * VIDEO_MAX_GMA_REGION_NUM;
	
	info->gma_buf_start_addr = (void *)kmalloc(info->gma_buf_size + 31, GFP_KERNEL);//mach_info->mem_base2;
	if(info->gma_buf_start_addr == NULL){
		FB_PRF("malloc gma buffer fail\n");
		return 0;
	}

	info->gma_buf_start = (void *)(((uint32)info->gma_buf_start_addr + 31) & ~31);	
	memset(info->gma_buf_start, 0, info->gma_buf_size);	
	__CACHE_FLUSH_ALI((unsigned long)(info->gma_buf_start), info->gma_buf_size);
		
	info->cur_region_id = 0;	
	info->gma_layer_id = info->DE_layer - 2;

	info->virtual_gma_enable = 0;
	return 1;
}

static int init_stadard_gma_region(struct fb_info *fbinfo)
{
	struct alifb_info *info = (struct alifb_info *)fbinfo->par;
	struct alifb_gma_info *gma_info = NULL;
	struct fb_var_screeninfo *var = &fbinfo->var;
	struct fb_fix_screeninfo *fix = &fbinfo->fix;

	if(info->id == 1)
		info->DE_layer = DE_LAYER2;
	else if(info->id == 3)
		info->DE_layer = DE_LAYER3;
	else
	{
		FB_PRF("don't supported info id %d\n", info->id);
		return -1;
	}

	if(info->gma_init == 0)
    {
		init_gma_info(info);
		info->gma_init = 1;
	}

	gma_info = info->gma_info + 0;
	gma_info->layer_id = info->gma_layer_id;
	gma_info->bpp = var->bits_per_pixel>>3;
	gma_info->h_src = gma_info->h_dst = 1;
	gma_info->v_src = gma_info->v_dst = 1;
	gma_info->scale_mode = GMA_RSZ_DIRECT_RESIZE;
	gma_info->dis_format = info->dis_format;
	gma_info->glob_alpha_en = 0;
	gma_info->glob_alpha = 0x7F;
	gma_info->x = 0;
	gma_info->y = 0;
	gma_info->w = var->xres;
	gma_info->h = var->yres;
    if ((info->DE_layer == DE_LAYER3) && (var->vmode & FB_VMODE_YWRAP)) {
        gma_info->pitch = var->xres * gma_info->bpp;
        gma_info->data_buffer = (uint32)(fix->smem_start);
    } else {
	    gma_info->pitch = var->xres_virtual * gma_info->bpp;
        gma_info->data_buffer = (uint32)(fix->smem_start + gma_info->pitch * var->yoffset + var->xoffset * gma_info->bpp);
    }
	if(!ali_vpo_create_GMA_region(info, 0)){
		FB_PRF("gma create region fail %d\n", 0);
		return -1;
	}
	else
		FB_PRF("gma create region ok, de layer %d\n", info->DE_layer);

	/* clear the gma screen */
	if(gma_info->bpp > 1){
		void *buf = NULL;
		int len = 0;

        #if defined(CONFIG_ALI_CHIP_M3921)
		buf = (unsigned short *)(phys_to_virt(gma_info->data_buffer));
        #else
		buf = (unsigned short *)(gma_info->data_buffer | 0x80000000);
        #endif
		len = var->xres_virtual * var->yres_virtual * gma_info->bpp;

		static int m_fb_data_init = 0;

		if(m_fb_data_init == 0)
		{
			m_fb_data_init = 1;
			
			memset(buf, 0, len);
		
			__CACHE_FLUSH_ALI((unsigned long)buf, len);
		}
	}

	/* scale the region to the full screen */
    #ifdef CONFIG_RPC_HLD_GMA
	if(ali_vpo_set_full_screen(info, var->xres, var->yres))
	#else
    if(hld_dis_fb_full_screen(info->gma_layer_id, var->xres, var->yres))
	#endif
		return -1;

	/* show on the gma region */
	if(gma_info->bpp > 1){
		ali_vpo_show_GMA_layer(info, 1);
	}
	
	return 0;
}

#ifndef CONFIG_RPC_HLD_GMA
static int ali_ge_set_cmdq(struct alifbio_cmdq_buf *ge_cmdq)
{
    unsigned long pos, last_pos;
    
    if (ge_cmdq->cmdq_size > (__G_ALI_MM_GE_CMD_SIZE) / 2)
    {
        FB_PRF("ali_ge_set_cmdq out of memory%d\n", 0);
        return -1;
    } 
    

    if (ge_cmdq->cmdq_index == 0)
    {
        // HQ
        pos = __G_ALI_MM_GE_CMD_START_ADDR & 0x8fffffff;
    	copy_from_user((void *)pos, (void *)ge_cmdq->cmdq_buf, ge_cmdq->cmdq_size);
		dma_cache_wback((unsigned long)pos, ge_cmdq->cmdq_size);
        last_pos = pos + ge_cmdq->cmdq_size - 4;
        
        pos = pos & 0x0fffffff;
        last_pos = last_pos & 0x0fffffff;
        *(volatile unsigned long *)0xb800a010 = pos;
        *(volatile unsigned long *)0xb800a014 = last_pos;
    }
    else
    {
        // LQ
        pos = (__G_ALI_MM_GE_CMD_START_ADDR & 0x8fffffff) + (__G_ALI_MM_GE_CMD_SIZE)  / 2;
    	copy_from_user((void *)pos, (void *)ge_cmdq->cmdq_buf, ge_cmdq->cmdq_size);
		dma_cache_wback((unsigned long)pos, ge_cmdq->cmdq_size);
        last_pos = pos + ge_cmdq->cmdq_size - 4;
        pos = pos & 0x0fffffff;
        last_pos = last_pos & 0x0fffffff;
        *(volatile unsigned long *)0xb800a018 = pos;
        *(volatile unsigned long *)0xb800a01c = last_pos;
    }
    return 0;
}

static DECLARE_WAIT_QUEUE_HEAD(ge_wq);
/*note: below definitions must be consistent with user space:
 *platform/src/lld/ge/m36f/ge_m36f_lld.h */
#define GE_HQ_FINISH        0x01
#define GE_LQ_FINISH        0x02
#define GE_IO_FINISH        0x04
#define GE_LQ_BREAK         0x100
static const unsigned int m_ge_int_status_bit[] = { GE_HQ_FINISH, GE_LQ_FINISH, GE_IO_FINISH, GE_LQ_BREAK };
static volatile unsigned long * ge_reserved_register = (volatile unsigned long *)(0xB800A000 + 0xec);
/* 0xAC is reserved, so I choose it to be share between here and kernel interrupt 
 * isr, it's a dirty fix, but anyway, do you have good suggestion?
 */

#define _NEW_GE_

#ifndef _NEW_GE_
/* interrupt use 0xa address to escape cache problem */
static irqreturn_t ali_ge_interrupt(int irq, void * param)
{
    volatile unsigned long * int_status = (volatile unsigned long *)(0xB800A000 + 0x08);
    volatile unsigned long * ge_test    = (volatile unsigned long *)(0xB800A000 + 0x0d8);
    unsigned long status;
    unsigned int i;

    status = * int_status;
    *ge_test = 0xaaffcc88;

    if (status) {
        * int_status = status;
    }

    status &= GE_HQ_FINISH | GE_LQ_FINISH | GE_IO_FINISH;
    if (status)
    {
        for (i = 0; i < 3; i++)
        {
            if ((status & m_ge_int_status_bit[i]))
            {
                (*ge_reserved_register) &= ~m_ge_int_status_bit[i]; 
                wake_up(&ge_wq);
            }
        }
    }

	return IRQ_HANDLED;
}

#else  /* _NEW_GE_ */

/*
 * Cmdq buffer status, put at beginning of mmap HW cmdq buffer such
 * that both userspace and kernel will see it.
 */
struct cmdq_status {
    /* indicate node boundary */
    unsigned int node_min_addr; /* PHYSICAL address of buffer start */
    unsigned int node_max_addr; /* PHYSICAL address of buffer end */

    unsigned int ge_finish;     /* ISR set it when there are no GE node */
    unsigned int ge_serial;     /* every GE INT wil increase serial */

    unsigned int ge_is_reset;            /* flag that GE is idle */
    unsigned int before_wrap_end;       /* !! PHYSICAL address of tail. */

    /* !! PHYSICAL address where new NODE add from here, in order to
          avoid race condition, bit[0] is used for indicate wrap*/
    unsigned int add_from_here;
};

#define NODE_ADDR(add_from_here) ((add_from_here) & 0xfffffffe)
#define NODE_WRAP(add_from_here) ((add_from_here) & 0x01)

#define NODE_CLEAR_WRAP(add_from_here)              \
    do {                                            \
        add_from_here = add_from_here & 0xfffffffe; \
    } while(0)

#define NODE_SET_WRAP(add_from_here)            \
    do {                                        \
        add_from_here = add_from_here | 0x1;    \
    } while(0)

#define NODE_SET_ADDR(add_from_here, addr)              \
    do {                                                \
        add_from_here = addr | (add_from_here & 0x1);   \
    } while(0)

static unsigned int ali_ge_get_cmdq_buf_addr(void)
{
    static unsigned int cmdq_buf_addr = 0;
    if (0 == cmdq_buf_addr) {
        cmdq_buf_addr = __G_ALI_MM_GE_CMD_START_ADDR;
    }

    return cmdq_buf_addr;
}

static irqreturn_t ali_ge_interrupt(int irq, void *param)
{

    static unsigned int i = 0;
	volatile unsigned long *ge_test = (volatile unsigned long *) (0xB800A000 + 0x0d8);
    volatile unsigned long *int_status = (volatile unsigned long *) (0xB800A000 + 0x08);
	volatile unsigned long *hq_fst_ptr = (volatile unsigned long *) (0xB800A000 + 0x010);
	volatile unsigned long *hq_lst_ptr = (volatile unsigned long *) (0xB800A000 + 0x014);
	volatile unsigned long *ge_start   = (volatile unsigned long *) (0xB800A000 + 0x004);
	volatile unsigned long *ge_ctl     = (volatile unsigned long *) (0xB800A000 + 0x000);

    volatile struct cmdq_status *cmdq_status =
            (volatile struct cmdq_status *) ali_ge_get_cmdq_buf_addr();

	unsigned int buffer_wraped;
	unsigned long status;

    /* test if we enter GE INT */
    *ge_test = 0xdeadbeaf + i;
    i++;

    /* 1) clear INT status */
    status = *int_status;
    if (status) {
        *int_status = status;
    }

    cmdq_status->ge_serial++;           /* increase serial */

    buffer_wraped = NODE_WRAP(cmdq_status->add_from_here);;

    /* all work done, mark GE is finished so that userspace could see it. */
    if (NODE_ADDR(cmdq_status->add_from_here) == *hq_fst_ptr) {
        cmdq_status->ge_finish = 1;
        wake_up(&ge_wq);
    } else {
		/* new node available, so go on */
		if (buffer_wraped) {
			/* finish tail, now wrap over to start */
			if (*hq_fst_ptr == cmdq_status->before_wrap_end) {
				/* clear wrap flag */
				NODE_CLEAR_WRAP(cmdq_status->add_from_here);
				/* start from beginning */
				*hq_fst_ptr = cmdq_status->node_min_addr;
				*hq_lst_ptr = NODE_ADDR(cmdq_status->add_from_here) - 4;
			} else {
				/* tail do not finish, finish tail first */
				*hq_lst_ptr = cmdq_status->before_wrap_end - 4;
			}
		} else {
			/* no wrap, just start tailing */
			*hq_lst_ptr = NODE_ADDR(cmdq_status->add_from_here) - 4;
}

		*ge_ctl |= 0x80000000;	/* SW access memroy */
		*ge_ctl |= (1 << 17);	/* enable HQ IRQ */
		*ge_start |= 0x2;		/* start HQ */
	}

	return IRQ_HANDLED;
}
#endif  /* _NEW_GE_ */

static int fb_create_region (struct fb_info *fbinfo, struct alifb_info *info)
{
    int ret = 0;
    unsigned long flags;
    struct alifb_gma_info *gma_info = NULL;
    if(info->DE_layer!=DE_LAYER2 && info->DE_layer!=DE_LAYER3 )
        return -EINVAL;

    local_irq_save(flags);

    if(info->gma_init == 0)
    {
        init_gma_info(info);
	    info->gma_init = 1;
    }
    
    gma_info = info->gma_info + 0;
	gma_info->layer_id = info->gma_layer_id;
	gma_info->bpp = fbinfo->var.bits_per_pixel>>3;
	gma_info->h_src = gma_info->h_dst = 1;
	gma_info->v_src = gma_info->v_dst = 1;
	gma_info->scale_mode = GMA_RSZ_DIRECT_RESIZE;
	gma_info->dis_format = GE_GMA_PF_CLUT8;//info->dis_format;
	gma_info->glob_alpha_en = 0;
	gma_info->glob_alpha = 0x7F;
    #ifdef ALI_FB_CREATE_NEW
	gma_info->x = fbinfo->var.xoffset;
	gma_info->y = fbinfo->var.yoffset;
	gma_info->bitmap_addr = 0;
    #else
	gma_info->x = 0;
	gma_info->y = 0;
    #endif
	gma_info->w = fbinfo->var.xres;
	gma_info->h = fbinfo->var.yres;
	gma_info->pitch = fbinfo->var.xres_virtual * gma_info->bpp;
	gma_info->data_buffer = (uint32)(fbinfo->fix.smem_start + gma_info->pitch * fbinfo->var.yoffset + fbinfo->var.xoffset * gma_info->bpp);

    if(!ali_vpo_create_GMA_region(info, 0)){
		FB_PRF("create FB2 region fail %d\n", 0);
		return -1;
	}

    #ifdef ALI_FB_CREATE_NEW
    #else
	/* clear the gma screen */
    if(gma_info->bpp >= 1)
    {
		void *buf = NULL;
		int len = 0;
        gma_info->data_buffer = (uint32)(fbinfo->fix.smem_start );
		buf = (unsigned short *)(gma_info->data_buffer | 0x80000000);
		len = fbinfo->var.xres_virtual * fbinfo->var.yres_virtual * gma_info->bpp;

		memset(buf, 0, len);
		dma_cache_wback((unsigned long)buf, len);
	}
    #endif
    
    local_irq_restore(flags);

    return ret;
}
#endif

static int alifb_internal_rpc_ioctl(struct ali_fb_rpc_pars * rpc_pars)
{
    struct ali_fb_rpc_pars pars;
	int i = 0;
    int ret = 0;
    
    memset((void *)&pars, 0, sizeof(pars));
    memcpy((void *)&pars, rpc_pars,sizeof(pars)); 

    for(i = 0;i < pars.arg_num;i++)
    {
        g_ali_fb_rpc_arg_size[i] = pars.arg[i].arg_size;

        if(g_ali_fb_rpc_arg_size[i] > 0)
        {
	        if(g_ali_fb_rpc_arg[i] == NULL)
            {
		        FB_PRF("allocate rpc arg buf fail arg %d\n", i);
			    return -ENOMEM;
		    }

            memcpy((void *)g_ali_fb_rpc_arg[i], pars.arg[i].arg, g_ali_fb_rpc_arg_size[i]);
	    }
    }

    ret = hld_dis_rpc_operation(pars.hd_dev, pars.API_ID);
    return ret;
}

static int osd_is_run(int layer_id)
{
	unsigned int val[2] = {0,0};

    #if defined(CONFIG_ALI_CHIP_M3921)
    val[0] = __REG32ALI(0x180060b8);
    val[1] = __REG32ALI(0x180060bc);
    #else
	val[0] = *(volatile unsigned int *)0xb8006300;
    val[1] = *(volatile unsigned int *)0xb8006380;
    #endif
    
	return (val[layer_id]&0x1);
}

/*static*/ enum TVSystem_FB tv_mode_to_tv_sys(FB_TV_OUT_MODE eTvMode)
{
    enum TVSystem_FB tvs = PAL_FB;

	switch(eTvMode)
	{
    	case TV_MODE_FB_PAL:			return PAL_FB;
    	case TV_MODE_FB_PAL_N:			return PAL_N_FB;
    	case TV_MODE_FB_NTSC358:		return NTSC_FB;
    	case TV_MODE_FB_PAL_M:			return PAL_M_FB;
    	case TV_MODE_FB_NTSC443:		return NTSC_443_FB;
    	case TV_MODE_FB_SECAM:		    return SECAM_FB;
    	case TV_MODE_FB_480P:			return NTSC_FB;
    	case TV_MODE_FB_576P:			return PAL_FB;
    	case TV_MODE_FB_720P_50:		return LINE_720_25_FB;
    	case TV_MODE_FB_720P_60:		return LINE_720_30_FB;
    	case TV_MODE_FB_1080I_25:		return LINE_1080_25_FB;
    	case TV_MODE_FB_1080I_30:		return LINE_1080_30_FB;
    	case TV_MODE_FB_1080P_25:		return LINE_1080_25_FB;
    	case TV_MODE_FB_1080P_30:		return LINE_1080_30_FB;
    	case TV_MODE_FB_1080P_24:		return LINE_1080_24_FB;
    	case TV_MODE_FB_1080P_50:		return LINE_1080_50_FB;
    	case TV_MODE_FB_1080P_60:		return LINE_1080_60_FB;

        default:
    		return PAL_FB;
        break;
	}
    return tvs;
}


static UINT8 dis_format_to_ge(UINT8 dis_format)
{
#ifndef CONFIG_RPC_HLD_GMA
    enum GE_GMA_PIXEL_FORMAT color_format = GE_GMA_PF_CLUT8;

    switch(dis_format)
    {
        case DIS_FORMAT_CLUT8:
            color_format = GE_GMA_PF_CLUT8;
            break;
        case DIS_FORMAT_ARGB1555:
 	        color_format = GE_GMA_PF_ARGB1555;
            break;
        case DIS_FORMAT_ARGB4444:
            color_format = GE_GMA_PF_ARGB4444;
            break;
        case DIS_FORMAT_ARGB8888:
            color_format = GE_GMA_PF_ARGB8888;
            break;
        default:
            break;
    }
   
    return color_format;
#else
    return dis_format;
#endif
}

static int alifb_update_var(struct fb_info *fbinfo)
{
	struct alifb_info *ali_info = (struct alifb_info *)fbinfo->par;
	struct fb_var_screeninfo *var = &fbinfo->var;

   	switch(ali_info->dis_format)
	{
		case DIS_FORMAT_CLUT2:
			var->bits_per_pixel = 2;
			break;
		case DIS_FORMAT_CLUT4:
			var->bits_per_pixel = 4;
			break;
		case DIS_FORMAT_CLUT8:
			var->bits_per_pixel = 8;
			break;
		case DIS_FORMAT_ACLUT88:
		case DIS_FORMAT_RGB444:
		case DIS_FORMAT_RGB555:
		case DIS_FORMAT_RGB565:
		case DIS_FORMAT_ARGB1555:
		case DIS_FORMAT_ARGB4444:
			var->bits_per_pixel = 16;
			break;
		case DIS_FORMAT_RGB888:
		case DIS_FORMAT_ARGB8888:
			var->bits_per_pixel = 32;
			break;
		default:
			break;
	}

	return 0;
}

static int alifb_hld_ioctl(unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    struct vpo_device *cur_dev = NULL;

    switch(cmd)
    {
        case VPO_SET_WIN_ONOFF:
        {
            struct ali_vpo_win_status_pars win_status_pars;
            
			copy_from_user((void *)&win_status_pars, (void *)arg, sizeof(win_status_pars));
            cur_dev = win_status_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_win_onoff(cur_dev, win_status_pars.on);
			break;
        }
        
        #if 0
        case VPO_SET_WIN_ONOFF_EX:
        {
            struct ali_vpo_win_status_pars win_status_pars;
            
			copy_from_user((void *)&win_status_pars, (void *)arg, sizeof(win_status_pars));
            cur_dev = win_status_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_win_onoff_ext(cur_dev, win_status_pars.on, win_status_pars.layer);

			break;
        }
        #endif
        
		case VPO_SET_WIN_MODE:
        {
            struct ali_vpo_winmode_pars win_mode_pars;
            
			copy_from_user((void *)&win_mode_pars, (void *)arg, sizeof(win_mode_pars));	
            cur_dev = win_mode_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_win_mode(cur_dev, win_mode_pars.win_mode, \
				&win_mode_pars.mp_callback, \
				&win_mode_pars.pip_callback);
			break;
        }
        
		case VPO_WIN_ZOOM:
        {
            struct ali_vpo_zoom_pars zoom_pars;

			copy_from_user((void *)&zoom_pars, (void *)arg, sizeof(zoom_pars));
            cur_dev = zoom_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_zoom(cur_dev, &zoom_pars.src_rect, \
				&zoom_pars.dst_rect);
			break;
		}
        
        #if 0
        case VPO_WIN_ZOOM_EX:
        {
            struct ali_vpo_zoom_pars zoom_pars;

			copy_from_user((void *)&zoom_pars, (void *)arg, sizeof(zoom_pars));
            cur_dev = zoom_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_zoom_ext(cur_dev, &zoom_pars.src_rect, \
				&zoom_pars.dst_rect, zoom_pars.layer);
			break;
        }
        #endif
        
		case VPO_SET_ASPECT_MODE:
        {
            struct ali_vpo_aspect_pars aspect_par;
            
			copy_from_user((void *)&aspect_par, (void *)arg, sizeof(aspect_par));  
            cur_dev = aspect_par.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_aspect_mode(cur_dev, aspect_par.aspect, aspect_par.display_mode);
			break;
		}
		case VPO_SET_TVSYS:
        {
            struct ali_vpo_tvsys_pars tvsys_pars;

			copy_from_user((void *)&tvsys_pars, (void *)arg, sizeof(tvsys_pars));
            cur_dev = tvsys_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_tvsys(cur_dev, tvsys_pars.tvsys);
			break;
		}
		case VPO_SET_TVSYS_EX:
        {
            struct ali_vpo_tvsys_pars tvsys_pars;

			copy_from_user((void *)&tvsys_pars, (void *)arg, sizeof(tvsys_pars));
            cur_dev = tvsys_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_tvsys_ex(cur_dev, tvsys_pars.tvsys, tvsys_pars.progressive);
			break;
		}
		case VPO_CONFIG_SOURCE_WINDOW:
        {
            struct ali_vpo_win_config_pars win_config_pars;
            
			copy_from_user((void *)&win_config_pars, (void *)arg, sizeof(win_config_pars));
            cur_dev = win_config_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_config_source_window(cur_dev, &win_config_pars.win_config_para);
			break;
		}
        case VPO_SET_BG_COLOR:
        {
            struct ali_vpo_bgcolor_pars bg_color_pars;	

            copy_from_user((void *)&bg_color_pars, (void *)arg, sizeof(bg_color_pars));
            cur_dev = bg_color_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_SET_BG_COLOR, (uint32)(&bg_color_pars.yuv_color));
            break;
        }
        case VPO_REG_DAC:
        {
            struct ali_vpo_dac_pars dac_pars;	

            copy_from_user((void *)&dac_pars, (void *)arg, sizeof(dac_pars));
            cur_dev = dac_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_REG_DAC, (uint32)(&dac_pars.dac_param));
            break;
        }
        case VPO_SET_PARAMETER:
        {
            struct ali_vpo_parameter_pars parameter_pars;	

            copy_from_user((void *)&parameter_pars, (void *)arg, sizeof(parameter_pars));
            cur_dev = parameter_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_SET_PARAMETER, (uint32)(&parameter_pars.param));
            break;
        }
        case VPO_VIDEO_ENHANCE:
        {
            struct ali_vpo_video_enhance_pars video_enhance_pars;	

            copy_from_user((void *)&video_enhance_pars, (void *)arg, sizeof(video_enhance_pars));
            cur_dev = video_enhance_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_VIDEO_ENHANCE, (uint32)(&video_enhance_pars.video_enhance_param));
            break;
        }
        case VPO_SET_CGMS_INFO:
        {
            struct ali_vpo_cgms_info_pars cgms_info_pars;	

            copy_from_user((void *)&cgms_info_pars, (void *)arg, sizeof(cgms_info_pars));
            cur_dev = cgms_info_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_SET_CGMS_INFO, (uint32)(&cgms_info_pars.cgms_info));
            break;
        }
        case VPO_AFD_CONFIG:
        {
            struct ali_vpo_afd_pars afd_pars;	

            copy_from_user((void *)&afd_pars, (void *)arg, sizeof(afd_pars));
            cur_dev = afd_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_AFD_CONFIG, (uint32)(&afd_pars.afd_param));
            break;
        }
        case VPO_GET_CURRENT_DISPLAY_INFO:
        {
            struct ali_vpo_display_info_pars display_info_pars;

            copy_from_user((void *)&display_info_pars, (void *)arg, sizeof(display_info_pars));
            cur_dev = display_info_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_GET_CURRENT_DISPLAY_INFO, (uint32)(&display_info_pars.display_info));

            ret = copy_to_user((void __user *)arg, &display_info_pars, sizeof(display_info_pars));
            break;
        }
        case VPO_GET_MP_INFO:
        {
            struct ali_vpo_mp_info_pars mp_info_pars;

            copy_from_user((void *)&mp_info_pars, (void *)arg, sizeof(mp_info_pars));
            cur_dev = mp_info_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_GET_INFO, (uint32)(&mp_info_pars.mp_info));

            ret = copy_to_user((void __user *)arg, &mp_info_pars, sizeof(mp_info_pars));
            break;
        }
        case VPO_SET_OSD_SHOW_TIME:
        {
            struct ali_vpo_osd_show_time_pars osd_show_time_pars;

            copy_from_user((void *)&osd_show_time_pars, (void *)arg, sizeof(osd_show_time_pars));
            cur_dev = osd_show_time_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_SET_OSD_SHOW_TIME, (uint32)(&osd_show_time_pars.osd_show_time));
            break;
        }
        case VPO_GET_MP_SCREEN_RECT:
        {
            struct ali_vpo_screem_rect_pars screem_rect_pars;

            copy_from_user((void *)&screem_rect_pars, (void *)arg, sizeof(screem_rect_pars));
            cur_dev = screem_rect_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_GET_MP_SCREEN_RECT, (uint32)(&screem_rect_pars.mp_screem_rect));

            ret = copy_to_user((void __user *)arg, &screem_rect_pars, sizeof(screem_rect_pars));
            break;
        }
        case VPO_GET_REAL_DISPLAY_MODE:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_GET_REAL_DISPLAY_MODE, &ioctrl_pars.param);

            ret = copy_to_user((void __user *)arg, &ioctrl_pars, sizeof(ioctrl_pars));
            break;
        }
		case VPO_GET_TV_ASPECT:
        {  
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_GET_TV_ASPECT, &ioctrl_pars.param);

            ret = copy_to_user((void __user *)arg, &ioctrl_pars, sizeof(ioctrl_pars));
            break;
		}
		case VPO_GET_SRC_ASPECT:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_GET_SRC_ASPECT, &ioctrl_pars.param);

            ret = copy_to_user((void __user *)arg, &ioctrl_pars, sizeof(ioctrl_pars));
            break;
		}
        case VPO_GET_DISPLAY_MODE:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_GET_DISPLAY_MODE, &ioctrl_pars.param);

            ret = copy_to_user((void __user *)arg, &ioctrl_pars, sizeof(ioctrl_pars));
            break;
        }
        case VPO_GET_OUT_MODE:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_GET_OUT_MODE, &ioctrl_pars.param);

            ret = copy_to_user((void __user *)arg, &ioctrl_pars, sizeof(ioctrl_pars));
            break;
        }
        case VPO_GET_OSD0_SHOW_TIME:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_GET_OSD0_SHOW_TIME, &ioctrl_pars.param);

            ret = copy_to_user((void __user *)arg, &ioctrl_pars, sizeof(ioctrl_pars));
            break;
        }
        case VPO_GET_OSD1_SHOW_TIME:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_GET_OSD1_SHOW_TIME, &ioctrl_pars.param);

            ret = copy_to_user((void __user *)arg, &ioctrl_pars, sizeof(ioctrl_pars));
            break;
        }
        case VPO_SET_VBI_OUT:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_SET_VBI_OUT, (uint32)(ioctrl_pars.param));
            break;
        }
		case VPO_WRITE_WSS:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_WRITE_WSS, (uint32)(ioctrl_pars.param));
            break;
		}
		case VPO_UNREG_DAC:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_UNREG_DAC, (uint32)(ioctrl_pars.param));
            break;
		}
		case VPO_MHEG_SCENE_AR:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_MHEG_SCENE_AR, (uint32)(ioctrl_pars.param));
            break;
		}
		case VPO_MHEG_IFRAME_NOTIFY:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_MHEG_IFRAME_NOTIFY, (uint32)(ioctrl_pars.param));
            break;
		}
		case VPO_DISAUTO_WIN_ONOFF:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_DISAUTO_WIN_ONOFF, (uint32)(ioctrl_pars.param));
            break;
		}
		case VPO_ENABLE_VBI:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_ENABLE_VBI, (uint32)(ioctrl_pars.param));
            break;
		}
		case VPO_PLAYMODE_CHANGE:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_PLAYMODE_CHANGE, (uint32)(ioctrl_pars.param));
            break;
		}
        case VPO_DIT_CHANGE:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_DIT_CHANGE, (uint32)(ioctrl_pars.param));
            break;
        }
		case VPO_SWAFD_ENABLE:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_SWAFD_ENABLE, (uint32)(ioctrl_pars.param));
            break;
		}
	    case VPO_704_OUTPUT:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_704_OUTPUT, (uint32)(ioctrl_pars.param));
            break;
	    }
		case VPO_SET_PREVIEW_MODE:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_SET_PREVIEW_MODE, (uint32)(ioctrl_pars.param));
            break;
		}
		case VPO_HDMI_OUT_PIC_FMT:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_HDMI_OUT_PIC_FMT, (uint32)(ioctrl_pars.param));
            break;
		}
        case VPO_ALWAYS_OPEN_CGMS_INFO:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_ALWAYS_OPEN_CGMS_INFO, (uint32)(ioctrl_pars.param));
            break;
        }
        case VPO_SET_LAYER_ORDER:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_SET_LAYER_ORDER, (uint32)(ioctrl_pars.param));
            break;
        }
        case VPO_TVESDHD_SOURCE_SEL:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_TVESDHD_SOURCE_SEL, (uint32)(ioctrl_pars.param));
            break;
        }
        case VPO_SD_CC_ENABLE:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_SD_CC_ENABLE, (uint32)(ioctrl_pars.param));
            break;
        }
        case VPO_SET_PREVIEW_SAR_MODE: 
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_IO_SET_PREVIEW_SAR_MODE, (uint32)(ioctrl_pars.param));
            break;
        }
        case VPO_SET_FULL_SCREEN_CVBS: 
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            copy_from_user((void *)&ioctrl_pars, (void *)arg, sizeof(ioctrl_pars));
            cur_dev = ioctrl_pars.hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
			ret = vpo_ioctl(cur_dev, VPO_FULL_SCREEN_CVBS, (uint32)(ioctrl_pars.param));
            break;
        }
        default:
            break;
    }

    return ret;
}

static int alifb_backup_picture(struct ali_vpo_display_info_pars *display_info_pars)
{
    struct vpo_device *cur_dev = NULL;
    struct vpo_io_get_picture_info *dis_info = &(display_info_pars->display_info);
    struct vpo_io_get_picture_info info_bak;
    uint32 blk_pic = dis_info->reserved[0];
    int ret = 0;
    
    if(dis_info->de_index > 0)
    {
		ret = -EFAULT;
        goto EXIT;
    }

    cur_dev = display_info_pars->hd_dev == 0 ? g_sd_vpo_dev : g_vpo_dev;
	ret = vpo_ioctl(cur_dev, VPO_IO_GET_CURRENT_DISPLAY_INFO, (uint32)dis_info);
	if((ret == 0) && (dis_info->status == 2))
	{
        if(cc_backup_info.backup_pic_y_buffer != NULL || cc_backup_info.backup_pic_c_buffer != NULL || cc_backup_info.backup_pic_maf_buffer)
            return RET_FAILURE;

        cc_backup_info.backup_pic_y_buffer = __VSTMALI(__G_ALI_MM_STILL_FRAME_START_ADDR);
        cc_backup_info.backup_pic_c_buffer = __VSTMALI(((__G_ALI_MM_STILL_FRAME_START_ADDR + 255) & 0xFFFFFF00) + Y_BUF_SIZE);

        if(cc_backup_info.backup_pic_y_buffer == NULL || cc_backup_info.backup_pic_c_buffer == NULL)
            return RET_FAILURE;

        memset(&info_bak, 0, sizeof(struct vpo_io_get_picture_info));
        
        if(cc_backup_info.backup_pic_y_buffer != NULL)
        {
            info_bak.top_y = ((UINT32)cc_backup_info.backup_pic_y_buffer+255) & 0xFFFFFF00;
            if(blk_pic == TRUE)
            {
                memset((void *)info_bak.top_y, 0x10, dis_info->y_buf_size);
            }
            else
            {
                memcpy((void *)info_bak.top_y, (void *)__VSTMALI((unsigned long)dis_info->top_y & 0x8FFFFFFF), dis_info->y_buf_size);
            }
			__CACHE_FLUSH_ALI((unsigned long)info_bak.top_y, dis_info->y_buf_size);
        }
        
        if(cc_backup_info.backup_pic_c_buffer != NULL)
        {
            info_bak.top_c = ((UINT32)cc_backup_info.backup_pic_c_buffer+255) & 0xFFFFFF00;
            if(blk_pic == TRUE)
            {
                memset((void *)info_bak.top_c, 0x80, dis_info->c_buf_size);
            }
            else
            {
			    memcpy((void *)info_bak.top_c, (void *)__VSTMALI((unsigned long)dis_info->top_c & 0x8FFFFFFF), dis_info->c_buf_size);
            }
			__CACHE_FLUSH_ALI((unsigned long)info_bak.top_c, dis_info->c_buf_size);	
        }
		if(dis_info->maf_buf_size != 0)
		{
            cc_backup_info.backup_pic_maf_buffer = (UINT32)kmalloc(dis_info->maf_buf_size + 256, GFP_KERNEL);
            if(cc_backup_info.backup_pic_maf_buffer != NULL)
            {
                info_bak.maf_buffer = ((UINT32)cc_backup_info.backup_pic_maf_buffer+255) & 0xFFFFFF00;
                if(blk_pic == TRUE)
                {
                    memset((void *)info_bak.maf_buffer, 0x00, dis_info->maf_buf_size);
                }
                else
                {
				    memcpy((void *)info_bak.maf_buffer, (void *)__VSTMALI((unsigned long)dis_info->maf_buffer & 0x8FFFFFFF), dis_info->maf_buf_size);
                }
				__CACHE_FLUSH_ALI((unsigned long)info_bak.maf_buffer, dis_info->maf_buf_size);
            }
		}		

        info_bak.top_y = __VMTSALI(info_bak.top_y);
        info_bak.top_c = __VMTSALI(info_bak.top_c);
        info_bak.maf_buffer = __VMTSALI(info_bak.maf_buffer);
		vpo_ioctl(cur_dev, VPO_IO_BACKUP_CURRENT_PICTURE, &info_bak);	
	}
    else
    {
        ret = -EFAULT;
    }
    
EXIT:    
    return ret;

}

static int alifb_free_backup_picture(void)
{
    if(cc_backup_info.backup_pic_maf_buffer != NULL)
    {
        kfree(cc_backup_info.backup_pic_maf_buffer);
    }

	cc_backup_info.backup_pic_y_buffer = NULL;
	cc_backup_info.backup_pic_c_buffer = NULL;
	cc_backup_info.backup_pic_maf_buffer = NULL;
	
    return 0;
}
	
static int alifb_ioctl(struct fb_info *fbinfo, unsigned int cmd,
			unsigned long arg)
{
	struct alifb_info *info = (struct alifb_info *)fbinfo->par;
    static unsigned int s_irq_registered = 0;

	int ret = 0;

	//FB_PRF("%s : cmd %x arg %x id %d\n", __FUNCTION__, (int)cmd, (int)arg, info->id);
	
	switch(cmd){
		case FBIO_GET_FBINFO:
		{
			copy_to_user((void *)arg, (void *)&fbinfo, sizeof(fbinfo));
			break;
		}
		case FBIO_GET_FBINFO_DATA:
		{
			struct alifbio_fbinfo_data_pars fbinfo_pars;

			fbinfo_pars.mem_start = fbinfo->fix.smem_start;
			fbinfo_pars.mem_size = fbinfo->fix.smem_len;
			fbinfo_pars.xres_virtual = fbinfo->var.xres_virtual;
			fbinfo_pars.yres_virtual = fbinfo->var.yres_virtual;
			fbinfo_pars.line_length = fbinfo->fix.line_length;
			copy_to_user((void *)arg, (void *)&fbinfo_pars, sizeof(fbinfo_pars));
			break;
		}

        case VPO_SET_WIN_ONOFF:
        case VPO_SET_WIN_ONOFF_EX: 
		case VPO_SET_WIN_MODE:  
		case VPO_WIN_ZOOM:        
        case VPO_WIN_ZOOM_EX:        
		case VPO_SET_ASPECT_MODE:        
		case VPO_SET_TVSYS:        
		case VPO_SET_TVSYS_EX:
		case VPO_CONFIG_SOURCE_WINDOW:        
        case VPO_SET_BG_COLOR:        
        case VPO_REG_DAC:       
        case VPO_SET_PARAMETER:        
        case VPO_VIDEO_ENHANCE:
        case VPO_SET_CGMS_INFO:
        case VPO_AFD_CONFIG:       
        case VPO_GET_CURRENT_DISPLAY_INFO:       
        case VPO_SET_OSD_SHOW_TIME:        
        case VPO_GET_MP_SCREEN_RECT:
        case VPO_GET_MP_INFO:
        case VPO_GET_REAL_DISPLAY_MODE:
		case VPO_GET_TV_ASPECT:
		case VPO_GET_SRC_ASPECT:
        case VPO_GET_DISPLAY_MODE:
        case VPO_GET_OSD0_SHOW_TIME:
        case VPO_GET_OSD1_SHOW_TIME:
        case VPO_SET_VBI_OUT:
		case VPO_WRITE_WSS:
		case VPO_UNREG_DAC:
		case VPO_MHEG_SCENE_AR:
		case VPO_MHEG_IFRAME_NOTIFY:
		case VPO_DISAUTO_WIN_ONOFF:
		case VPO_ENABLE_VBI:
		case VPO_PLAYMODE_CHANGE:
        case VPO_DIT_CHANGE:
		case VPO_SWAFD_ENABLE:
	    case VPO_704_OUTPUT:
		case VPO_SET_PREVIEW_MODE:
		case VPO_HDMI_OUT_PIC_FMT:
        case VPO_ALWAYS_OPEN_CGMS_INFO:
        case VPO_SET_LAYER_ORDER:
        case VPO_TVESDHD_SOURCE_SEL:
        case VPO_SD_CC_ENABLE: 
        case VPO_SET_PREVIEW_SAR_MODE: 
        case VPO_SET_FULL_SCREEN_CVBS:
        case VPO_GET_OUT_MODE:
        {
            ret = alifb_hld_ioctl(cmd, arg);
            if(ret != 0)
            {
                ret = -EFAULT;
            }
            break;
        }
     
        case VPO_FREE_BACKUP_PICTURE:
        {
            ret = alifb_free_backup_picture();
            break;
        }
        case VPO_BACKUP_CURRENT_PICTURE:
        {
            struct ali_vpo_display_info_pars display_info_pars;

            copy_from_user((void *)&display_info_pars, (void *)arg, sizeof(display_info_pars));
			ret = alifb_backup_picture(&display_info_pars);
            if(ret != 0)
            {
                ret = -EFAULT;
            }
            break;
        }
		
        case FBIO_WIN_ZOOM:
		{
			struct alifbio_zoom_pars zoom;
            struct ali_fb_rpc_pars rpc_pars;
			copy_from_user((void *)&zoom, (void *)arg, sizeof(zoom));

			zoom.src_rect.x = zoom.src_rect.x*720/zoom.user_screen_xres;
            zoom.src_rect.y = zoom.src_rect.y*2880/zoom.user_screen_yres;
            zoom.src_rect.w = zoom.src_rect.w*720/zoom.user_screen_xres;
            zoom.src_rect.h= zoom.src_rect.h*2880/zoom.user_screen_yres;
			zoom.dst_rect.x = zoom.dst_rect.x*720/zoom.user_screen_xres;
            zoom.dst_rect.y = zoom.dst_rect.y*2880/zoom.user_screen_yres;
            zoom.dst_rect.w = zoom.dst_rect.w*720/zoom.user_screen_xres;
            zoom.dst_rect.h= zoom.dst_rect.h*2880/zoom.user_screen_yres;

            FB_PRF("src x %ld y %ld w %ld h %ld \n", zoom.src_rect.x, zoom.src_rect.y, \
                zoom.src_rect.w, zoom.src_rect.h);
	        FB_PRF("dst x %ld y %ld w %ld h %ld \n", zoom.dst_rect.x, zoom.dst_rect.y, \
                    zoom.dst_rect.w, zoom.dst_rect.h);	
					
		    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		    rpc_pars.hd_dev = 1;				
		    rpc_pars.API_ID = RPC_FB_ZOOM;
		    rpc_pars.arg_num = 2;
		    rpc_pars.arg[0].arg = (void *)(&zoom.src_rect);
		    rpc_pars.arg[0].arg_size = sizeof(zoom.src_rect);//sizeof(Video_Rect)		
		    rpc_pars.arg[1].arg = (void *)(&zoom.dst_rect);
		    rpc_pars.arg[1].arg_size = sizeof(zoom.dst_rect); //sizeof(Video_Rect)			

            ret = alifb_internal_rpc_ioctl(&rpc_pars);

            break;
		}
        case FBIO_SET_ASPECT_MODE:
		{
			struct alifbio_aspect_pars aspect_par;
            struct ali_fb_rpc_pars rpc_pars;
			copy_from_user((void *)&aspect_par, (void *)arg, sizeof(aspect_par));
         
		    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		    rpc_pars.hd_dev = 1;				
		    rpc_pars.API_ID = RPC_FB_ASPECT_MODE;
		    rpc_pars.arg_num = 2;
		    rpc_pars.arg[0].arg = (void *)(&aspect_par.aspect);
		    rpc_pars.arg[0].arg_size = sizeof(aspect_par.aspect); //sizeof(Video_TVAspect)		
		    rpc_pars.arg[1].arg = (void *)(&aspect_par.display_mode);
		    rpc_pars.arg[1].arg_size = sizeof(aspect_par.display_mode);//sizeof(Video_DisplayMode)
          
            //set aspect proc
            ret = alifb_internal_rpc_ioctl(&rpc_pars);
            break;
		}
		
		case FBIO_WIN_ONOFF:
		{
            struct ali_fb_rpc_pars pars;
            arg = (arg != 0) ? 1 : 0;
            
        	//disable_irq(info->irq_id);
			if(info->DE_layer == DE_LAYER0)
            {
                FB_PRF("open/close MP layer!\n");
                

                memset((void *)&pars, 0, sizeof(pars));
	            pars.hd_dev = 1;		
	            pars.API_ID = RPC_FB_WIN_ON_OFF;
	            pars.arg_num = 1;
	            pars.arg[0].arg = (void *)&arg;
	            pars.arg[0].arg_size = sizeof(arg);

                ret = alifb_internal_rpc_ioctl(&pars);	   
			}
			else if(info->DE_layer >= DE_LAYER2)
			{
                FB_PRF("open/close osd layer!\n");
                ali_vpo_show_GMA_layer(info, (arg != 0) ? 1 : 0);
            }
			else
            {
				ret = -EINVAL;
			}    
        	//enable_irq(info->irq_id);


			break;	
		}
		
        case FBIO_SET_COLOR_FMT:
		{
			//if(arg != info->dis_format)
			{
				info->dis_format = arg;
                info->DE_layer = DE_LAYER3;  
				alifb_update_var(fbinfo);
			}
			break;
		}
        case FBIO_GMA_SCALE:
		{
			struct alifbio_gma_scale_pars scale_pars;
            struct alifb_gma_info *gma_info = NULL;			
            unsigned int osd_onoff_flag = 0;
			int i = 0;
            
			copy_from_user((void *)&scale_pars, (void *)arg, sizeof(scale_pars));            
			info->scale_para_pending = 1;
            osd_onoff_flag = osd_is_run(info->DE_layer-2);
            if(osd_onoff_flag)
            {
                ali_vpo_show_GMA_layer(info, 0);
            }
            
			for(i = 0;i < VIDEO_MAX_GMA_REGION_NUM;i++) {
				gma_info = info->gma_info + i;
				if(gma_info->valid) {
                    #ifndef CONFIG_RPC_HLD_GMA
                    gma_info->uScaleParam = 0;
                    gma_info->tv_sys = (int)tv_mode_to_tv_sys(scale_pars.tv_mode);
                    gma_info->u_scale_cmd = FB_SET_SCALE_PARAM;
                    #else
                    gma_info->scale_mode = GMA_RSZ_DIRECT_RESIZE;
                    #endif
                    gma_info->h_src = scale_pars.h_src;
                    gma_info->v_src = scale_pars.v_src;
					gma_info->h_dst = scale_pars.h_dst;
					gma_info->v_dst = scale_pars.v_dst;
                                           
					gma_info->pending_flag |= ALIFB_GMA_FLAG_SCALE_PENDING;
					FB_PRF("gma scale: <%dx%d> => <%dx%d>\n"
						, gma_info->h_src, gma_info->v_src, gma_info->h_dst, gma_info->v_dst);
					ali_vpo_update_GMA_region(info, i);
				}
			}

            if(osd_onoff_flag)
            {
                ali_vpo_show_GMA_layer(info, 1);
            }
				
			break;
		}
		
		case FBIO_SET_PALLETTE:
		{
			struct alifbio_plt_pars plt_pars;

			copy_from_user((void *)&plt_pars, (void *)arg, sizeof(plt_pars));

			if(plt_pars.color_num == 256){
				copy_from_user(info->palette_buffer, plt_pars.pallette_buf, (plt_pars.color_num * 4));
				info->pallete_size = plt_pars.color_num;
				info->plt_type = plt_pars.type;			
                #ifdef CONFIG_RPC_HLD_GMA
				info->plt_alpha_level = PLT_ALPHA_128;
				#else
				info->plt_alpha_level = plt_pars.level;
				#endif
				
                #ifndef CONFIG_RPC_HLD_GMA
                info->rgb_order   =  plt_pars.rgb_order;
                info->alpha_range =  plt_pars.alpha_range;
                info->alpha_pol   =  plt_pars.alpha_pol;
				#endif

				__CACHE_FLUSH_ALI(info->palette_buffer, 1024);				
				ali_vpo_set_GMA_palette(info);			
			}
			else
				ret = -EINVAL;
				
			break;
		}
		case FBIO_GET_PALLETTE:
		{
			struct alifbio_plt_pars plt_pars;

			copy_from_user(&plt_pars, (void *)arg, sizeof(plt_pars));
			copy_to_user(plt_pars.pallette_buf, info->palette_buffer, info->pallete_size * 4);
			plt_pars.color_num = info->pallete_size;
			plt_pars.level = info->plt_alpha_level;
			copy_to_user((void *)arg, (void *)&plt_pars, sizeof(plt_pars));
			
			break;
		}
		case FBIO_DELETE_GMA_REGION:
		{
			int region_id = (int)arg;

			if(!ali_vpo_delete_GMA_region(info, region_id)){
				FB_PRF("gma delete region %d fail\n", region_id);
				ret = -EINVAL;
			}
			else
				FB_PRF("gma delete region %d ok\n", region_id);
				
			break;
		}
		case FBIO_CREATE_GMA_REGION:
		{
            #if 0//ndef CONFIG_RPC_HLD_GMA
            alifb_gma_region_t reg_par;
			struct alifb_gma_info *gma_info = NULL;
				
			if(info->gma_init == 0){
				init_gma_info(info);
				info->gma_init = 1;
			}		
            
			copy_from_user((void *)&reg_par, (void *)arg, sizeof(reg_par));		
			FB_PRF("dis format %d reg format %d, bpp %d\n", 
                (int)info->dis_format, (int)reg_par.color_format, fbinfo->var.bits_per_pixel>>3);
            
			if(1)   //info->dis_format == reg_par.dis_format){				
			{
                if(reg_par.region_index < VIDEO_MAX_GMA_REGION_NUM 
			      && reg_par.region_index >= 0)
			    {					
			        if((reg_par.region_w > VIDEO_MAX_GMA_WIDTH)
				        || (reg_par.region_h > VIDEO_MAX_GMA_HEIGHT))
				    {
						ret = -EINVAL;
						break;
					}
					
    		        gma_info = info->gma_info + reg_par.region_index;
                    
    				gma_info->layer_id = info->gma_layer_id;
    				gma_info->bpp = fbinfo->var.bits_per_pixel>>3;
                    gma_info->data_buffer = (uint32)fbinfo->fix.smem_start;
    				gma_info->h_src = gma_info->h_dst = 1;
    				gma_info->v_src = gma_info->v_dst = 1;
    				gma_info->scale_mode = GMA_RSZ_DIRECT_RESIZE;

                    gma_info->dis_format = reg_par.color_format;
    				gma_info->glob_alpha_en = reg_par.galpha_enable;
    				gma_info->glob_alpha = reg_par.global_alpha;
                    gma_info->pallette_sel = reg_par.pallette_sel;
                    
                    gma_info->x = reg_par.region_x;
    				gma_info->y = reg_par.region_y;
    				gma_info->w = reg_par.region_w;
    				gma_info->h = reg_par.region_h;
                    
                    gma_info->bitmap_addr = reg_par.bitmap_addr;
                    gma_info->pitch = reg_par.pixel_pitch;
                                  
                    gma_info->bitmap_x = reg_par.bitmap_x;
                    gma_info->bitmap_y = reg_par.bitmap_y;
                    gma_info->bitmap_w = reg_par.bitmap_w;
                    gma_info->bitmap_h = reg_par.bitmap_h;
 				               
        			if(!ali_vpo_create_GMA_region(info, reg_par.region_index))
                    {
        				printk("create gma region fail %ld\n", reg_par.region_index);
        			    ret = -EINVAL;
        		    }
        	    }
        		else
        	        ret = -EINVAL;
			}	
            #else
			struct alifbio_reg_pars reg_par;
			struct alifb_gma_info *gma_info = NULL;
				
			if(info->gma_init == 0){
				init_gma_info(info);
				info->gma_init = 1;
			}
			
			copy_from_user((void *)&reg_par, (void *)arg, sizeof(reg_par));		
			FB_PRF("dis format %d reg format %d\n"
				, (int)info->dis_format, (int)reg_par.dis_format);
            
			if(1){//info->dis_format == reg_par.dis_format){				
				if(reg_par.index < VIDEO_MAX_GMA_REGION_NUM 
					&& reg_par.index >= 0){		
					
					if((reg_par.rect.w > VIDEO_MAX_GMA_WIDTH)
						|| (reg_par.rect.h > VIDEO_MAX_GMA_HEIGHT)){
						ret = -EINVAL;
						break;
					}

					gma_info = info->gma_info + reg_par.index;
					if(gma_info->valid)
					{
						if((gma_info->x != reg_par.rect.x)
							|| (gma_info->y != reg_par.rect.y)
							|| (gma_info->w != reg_par.rect.w)
							|| (gma_info->h != reg_par.rect.h)
							|| (gma_info->dis_format != reg_par.dis_format))
						{
							ali_vpo_delete_GMA_region(info, reg_par.index);
						}			
						else
						{
							FB_PRF("already created region %d\n", reg_par.index);
							break;
						}
					}
				
					gma_info->layer_id = info->gma_layer_id;
					gma_info->bpp = fbinfo->var.bits_per_pixel>>3;
					gma_info->h_src = 1;
                    gma_info->h_dst = 1;
					gma_info->v_src = 1;
                    gma_info->v_dst = 1;
					gma_info->scale_mode = GMA_RSZ_DIRECT_RESIZE;
					gma_info->glob_alpha_en = 0;
					gma_info->glob_alpha = 0x7F;
					gma_info->x = reg_par.rect.x;
					gma_info->y = reg_par.rect.y;
					gma_info->w = reg_par.rect.w;
					gma_info->h = reg_par.rect.h;
                    #ifdef CONFIG_RPC_HLD_GMA
					gma_info->data_buffer = (uint32)reg_par.mem_start;
                    gma_info->pitch = reg_par.pitch;
                    gma_info->dis_format = reg_par.dis_format;
                    #else
                    gma_info->data_buffer = (uint32)fbinfo->fix.smem_start;
                    gma_info->pitch = fbinfo->fix.line_length;
                    gma_info->dis_format = dis_format_to_ge(reg_par.dis_format);
                    #endif
					if(!ali_vpo_create_GMA_region(info, reg_par.index)){
						FB_PRF("create gma region dis %d fail %d\n", (int)reg_par.dis_format, reg_par.index);
						ret = -EINVAL;
					}
					else
						FB_PRF("create gma region dis %d ok %d\n", (int)reg_par.dis_format, reg_par.index);
				}
				else
					ret = -EINVAL;
			}
			else
				ret = -EINVAL;
            #endif
			break;
		}
		case FBIO_SET_GMA_SCALE_INFO:
		{
			struct alifbio_gma_scale_info_pars scale_pars;
			struct alifb_gma_info *gma_info = NULL;			
			int i = 0;

			info->scale_para_pending = 1;
			copy_from_user((void *)&scale_pars, (void *)arg, sizeof(scale_pars));
			for(i = 0;i < VIDEO_MAX_GMA_REGION_NUM;i++){
				gma_info = info->gma_info + i;
				if(gma_info->valid){
                    #ifndef CONFIG_RPC_HLD_GMA
                    gma_info->u_scale_cmd = scale_pars.uScaleCmd;
                    gma_info->uScaleParam = scale_pars.uScaleParam;
                    gma_info->tv_sys = scale_pars.tv_sys;
                    if(gma_info->u_scale_cmd == GE_SET_SCALE_PARAM) {
    					gma_info->h_dst = scale_pars.h_dst;
    					gma_info->h_src = scale_pars.h_src;
    					gma_info->v_dst = scale_pars.v_dst;
    					gma_info->v_src = scale_pars.v_src;
                    }              
                    #else
					gma_info->scale_mode = scale_pars.scale_mode;
					gma_info->h_dst = scale_pars.h_dst;
					gma_info->h_src = scale_pars.h_src;
					gma_info->v_dst = scale_pars.v_dst;
					gma_info->v_src = scale_pars.v_src;
                    #endif  
					gma_info->pending_flag |= ALIFB_GMA_FLAG_SCALE_PENDING;
					FB_PRF("gma set scale info: h_src %d h_dst %d v_src %d v_dst %d\n"
						, gma_info->h_src, gma_info->h_dst, gma_info->v_src, gma_info->v_dst);
					ali_vpo_update_GMA_region(info, i);
				}
			}
			
			break;
		}
		case FBIO_GET_GMA_REG_INFO:
		{
			break;
		}
		case FBIO_MOVE_REGION:
		{
			struct alifb_gma_info *gma_info = NULL;	
			struct alifbio_move_region_pars pars;

			copy_from_user((void *)&pars, (void *)arg, sizeof(pars));
			gma_info = info->gma_info + pars.region_id;
			if(gma_info->valid){
				gma_info->x = pars.pos.x;
				gma_info->y = pars.pos.y;
                #ifndef CONFIG_RPC_HLD_GMA
                gma_info->width = gma_info->w;
				gma_info->height = gma_info->h;
                #endif
				gma_info->pending_flag |= ALIFB_GMA_FLAG_RECT_PENDING;
				ali_vpo_update_GMA_region(info, pars.region_id);
				FB_PRF("new gma region %d pos x %d y %d\n",
					pars.region_id, (int)pars.pos.x, (int)pars.pos.y);
			}
			
			break;
		}
		case FBIO_SET_DE_LAYER:
		{
			info->DE_layer = (enum ALIFB_DE_LAYER)arg;
			break;
		}
        
        #ifndef CONFIG_RPC_HLD_GMA
        case FBIO_GET_DISPLAY_RECT:
        {           
			struct gma_rect pars;
            ali_get_GMA_IOCtl(info,(uint32)&pars);
			copy_to_user((void *)arg, (void *)&pars, sizeof(pars));                      
            break;
        }
        case FBIO_SET_DISPLAY_RECT:
        {           
            struct gma_rect rect_pars;
            copy_from_user((void *)&rect_pars, (void *)arg, sizeof(rect_pars));
            ali_set_GMA_IOCtl(info,(uint32)&rect_pars);  
            
            FB_PRF("set display rect pos x %d y %d\n",rect_pars.x, rect_pars.y);
			break;
		}
		#endif
        
		case FBIO_RPC_OPERATION:
		{
			struct ali_fb_rpc_pars pars;
			int i = 0;

			copy_from_user((void *)&pars, (void *)arg, sizeof(pars));
			for(i = 0;i < pars.arg_num;i++){
				g_ali_fb_rpc_arg_size[i] = pars.arg[i].arg_size;
				if(g_ali_fb_rpc_arg_size[i] > 0){

					if(g_ali_fb_rpc_arg[i] == NULL){
						FB_PRF("allocate rpc arg buf fail arg %d\n", i);
						ret = -ENOMEM;
						goto RPC_EXIT;			
					}
				
					copy_from_user((void *)g_ali_fb_rpc_arg[i], pars.arg[i].arg, g_ali_fb_rpc_arg_size[i]);
				}
			}
			
            #ifdef CONFIG_RPC_HLD_GMA
			if(VPO_IO_GET_BEST_DESUB == *g_ali_fb_rpc_arg[0])
			{
				*g_ali_fb_rpc_arg[1] = GetSupportListDSUB();
			}
			else
			{
				ret = hld_dis_rpc_operation(pars.hd_dev, pars.API_ID);
			}
			#else
			ret = hld_dis_rpc_operation(pars.hd_dev, pars.API_ID);
			#endif
			
RPC_EXIT:			
			for(i = 0;i < pars.arg_num;i++){
				if(g_ali_fb_rpc_arg_size[i] != 0){
					if(pars.arg[i].out)
						copy_to_user(pars.arg[i].arg, (void *)g_ali_fb_rpc_arg[i], g_ali_fb_rpc_arg_size[i]);
				}			
			}
			
			break;			
		}
		case FBIO_SET_GLOBAL_ALPHA:
		{
			ali_vpo_set_GMA_alpha(info, (unsigned char)arg);
			break;
		}

        case FBIO_SET_GMA_DBG_FLAG:
        {
           if(arg & 1)
           {
               g_fb_dbg_flag = 1;
           }
           else if(arg == 0)
           {
               g_fb_dbg_flag = 0;
           }
           ali_gma_set_dbg_flag(info, (unsigned long)arg);
           break;
        }
		
        #ifdef CONFIG_RPC_HLD_GMA
		case FBIO_DISABLE_GMA_ENHANCE:
		{
			g_fb_gma_disable_enhance = (arg != 0)? 1 : 0;
			break;
		}
        #else
        
        case FBIO_SET_GE_CMDQ_BUF:
		{
            struct alifbio_cmdq_buf my_cmdq;
   			copy_from_user((void *)&my_cmdq, (void *)arg, sizeof(my_cmdq));
			ret = ali_ge_set_cmdq(&my_cmdq);
            FB_PRF("set ge cmdq buf 0x%x size 0x%x\n", my_cmdq.cmdq_buf, my_cmdq.cmdq_size);
			break;
		}
        case FBIO_SET_MIX_ORDER:
        {
            struct ali_fb_rpc_pars rpc_pars;
            unsigned int dwCmd = 0x10000 +0x05; // define in vpo.h  (VPO_IO_ELEPHANT_BASE + 0x05)
            unsigned int dwParam;
            unsigned int layer_order[4] = {0x3210, 0x3120, 0x3201,0x3102};  
            //MP_GMAS_GMAF_AUXP ,bit:--13:12 AUXP--,--9:8--GMAF,--5:4--GMAS,--1:0--MP.
            //arg   order           dwParam
            //0:    fb1+fb2+fb0     0x3210     
			//1:    fb1+fb0+fb2     0x3120
		    //2:    fb2+fb1+fb0     0x3201
 	        //3:    fb2+fb0+fb1     0x3102
 	        
            if(arg <= 3)
            {
                dwParam = layer_order[arg];   
            } else {
                ret = -EINVAL;
                break;
            }
                
            memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	        rpc_pars.hd_dev = 1;				
	        rpc_pars.API_ID = RPC_FB_IOCTL;
	        rpc_pars.arg_num = 2;
	        rpc_pars.arg[0].arg = (void *)&dwCmd;
	        rpc_pars.arg[0].arg_size = sizeof(dwCmd);
            rpc_pars.arg[1].arg = (void *)&dwParam;					
			rpc_pars.arg[1].arg_size = 4;

            ret = alifb_internal_rpc_ioctl(&rpc_pars);

            break;
        }
        
        #if 0
		case FBIO_GET_UI_RSC_MAP:
		{
            struct ali_fb_rsc_mem_map mem_map;
			ui_rsc_mem_attr_t *p_ui_rsc_mem_attr = (ui_rsc_mem_attr_t *)request_attr(UI_RSC_MEM);
            mem_map.mem_start = (void*)(p_ui_rsc_mem_attr->ui_rsc_map_start_addr & 0x0FFFFFFF);
            mem_map.mem_size = p_ui_rsc_mem_attr->ui_rsc_map_end_addr - p_ui_rsc_mem_attr->ui_rsc_map_start_addr;
            FB_PRF("ali_fb.c: mem_map.mem_start=0x%x, mem_map.mem_size=%d\n", \
                mem_map.mem_start, mem_map.mem_size);
            copy_to_user((void*)arg, (void*)(&mem_map), sizeof(mem_map));
			break;
		}
        #endif
        
		case FBIO_SET_UI_CACHE_FLUSH:
		{
            struct alifbio_cache_flush cache_flush;
			copy_from_user((void *)&cache_flush, (void *)arg, sizeof(cache_flush));
            dma_cache_wback(((cache_flush.mem_start&0x0fffffff) | 0x80000000), cache_flush.mem_size);	
            FB_PRF("UI cache flush addr 0x%x len 0x%x\n", cache_flush.mem_start, cache_flush.mem_size);
			break;
		}

        case FBIO_REGISTER_ISR:
            {
                #ifdef _NEW_GE_
                volatile unsigned long * ge_test    = (volatile unsigned long *)(0xB800A000 + 0x0d8);
                *ge_test = 0x99ffee77;
                if (!s_irq_registered) {
                    FB_PRF("GE register IRQ\n");
                    ret = request_irq(M36_IRQ_GE, ali_ge_interrupt, 0, "ge", 0);
                    s_irq_registered = 1;
                } else {
                    FB_PRF("GE IRQ already registered\n");
                }
                #else
                ret = request_irq(M36_IRQ_GE, ali_ge_interrupt, 0, "ge", 0);
                #endif
            }
            break;
        case FBIO_UNREGISTER_ISR:
            //TBD ... remove all wait task in the ge_wq
            {
                #ifdef _NEW_GE_
                FB_PRF("GE never unregister IRQ\n");
                #else
                free_irq(M36_IRQ_GE, 0);
                #endif
            }
            break;
        case FBIO_FLAG_WAIT:
        {
            unsigned int timeout, cmd_bit;
            timeout = arg & 0xffff;
            cmd_bit = (arg >> 16) & 0xffff;

            ret = wait_event_interruptible_timeout(ge_wq, !((* ge_reserved_register) & cmd_bit), timeout);
            if(ret <= 0) //timeout or receive a signal
            {
                 FB_PRF("wait ge command time out, cmd_bit: 0x%08lx, timeout = %d\n", cmd_bit, timeout);
                 return -2;  //timeout as osal_flag_wait return?
            }
            ret = 0; 
            break;
        }
        
        #ifdef _NEW_GE_
        case FBIO_GE_SYNC_TIMEOUT:
    		{
                unsigned int timeout = arg;
                volatile struct cmdq_status *cmdq_status =
                        (volatile struct cmdq_status *) ali_ge_get_cmdq_buf_addr();

                ret = wait_event_interruptible_timeout(ge_wq, cmdq_status->ge_finish, timeout);
                if (ret <= 0) {             /* timeout or receive a signal */
                    FB_PRF("wait ge command time out, timeout = %d\n", timeout);
                    return -2;  //timeout as osal_flag_wait return?
                }
                ret = 0;
    			break;
    		}
        #endif

        case FBIO_GET_GE_CMDQ_BUF_ADDR_SIZE:
    		{
                struct alifbio_cmd_buf_addr_size ge_cmd_buf_info;
                ge_cmd_buf_info.addr = __G_ALI_MM_GE_CMD_START_ADDR & ~(0xa0000000);
                ge_cmd_buf_info.size = __G_ALI_MM_GE_CMD_SIZE;
                copy_to_user((void*)arg, (void*)(&ge_cmd_buf_info), sizeof(ge_cmd_buf_info));
                FB_PRF("GE get cmdq addr 0x%x size 0x%x\n", ge_cmd_buf_info.addr, ge_cmd_buf_info.size);
    			break;
    		}        
        #endif
        
        #ifdef CONFIG_RPC_HLD_GMA
		case FBIO_SET_REGION_SIDE_BY:
		{
			ali_vpo_set_region_by(info, (unsigned char)arg);
			break;
		}
		case FBIO_VIDEO_ENHANCE:
		{
			struct ali_fb_video_enhance_pars enhance_pars;

			copy_from_user((void *)&enhance_pars, (void *)arg, sizeof(enhance_pars));
			ali_osd_set_enhance_pars(info, enhance_pars.changed_flag, enhance_pars.grade);
			break;
		}
        #endif		
		default:
			break;
	}

	//FB_PRF("%s : cmd %x arg %x id %d done\n", __FUNCTION__, (int)cmd, (int)arg, info->id);
		
	return ret;
}

static int alifb_check_var(struct fb_var_screeninfo *var, struct fb_info *fbinfo)
{		
	struct fb_var_screeninfo *cur_var = &fbinfo->var;
	
	if((var->xoffset + var->xres > var->xres_virtual)
		|| (var->yoffset + var->yres > var->yres_virtual)
#ifdef CONFIG_RPC_HLD_GMA
		|| (cur_var->xres_virtual < var->xres_virtual)
		|| (cur_var->yres_virtual < var->yres_virtual)
#endif
		)
	{
		FB_PRF("%s : resolution info fail x %d w %d y %d h %d cur virtual %d %d var virtual %d %d\n"
			, __FUNCTION__, var->xoffset, var->xres, var->yoffset, var->yres
			, cur_var->xres_virtual, cur_var->yres_virtual, var->xres_virtual, var->yres_virtual);
		
		return -1;
	}

	/* bits info */
	if(var->bits_per_pixel <= 8)
		var->bits_per_pixel = 8;
	else if(var->bits_per_pixel <= 16)
		var->bits_per_pixel = 16;
	else if(var->bits_per_pixel <= 32)
		var->bits_per_pixel = 32;
	
	switch(var->bits_per_pixel)
	{
		case 8:
			var->red.offset = 0;
			var->red.length = 8;
			var->red.msb_right = 0;
			var->green.offset = 0;
			var->green.length = 0;
			var->green.msb_right = 0;
			var->blue.offset = 0;
			var->blue.length = 0;
			var->blue.msb_right = 0;
			var->transp.offset = 0;
			var->transp.length = 0;
			var->transp.msb_right = 0;			
			break;
		case 16:
            #ifdef CONFIG_RPC_HLD_GMA	
			var->green.offset = 5;
			var->green.length = (var->green.length < 6) ? 5 : 6;
			var->green.msb_right = 0;
			var->blue.offset = 0;
			var->blue.length = 5;
			var->blue.msb_right = 0;
			var->transp.offset = (5 + var->red.offset) & 15;
			var->transp.length = 6 - var->green.length;
			var->transp.msb_right = 0;	
			var->red.offset = 5 + var->green.length;
			var->red.length = 5;
			var->red.msb_right = 0;			
            #else
			if(var->xres_virtual > 2560)
			{
				FB_PRF("don't support > 2560 x resolution when 16bit gma output\n");
				return -1;
			}		
			var->red.offset = 10;
			var->red.length = 5;
			var->red.msb_right = 0;
			var->green.offset = 5;
			var->green.length = 5;
			var->green.msb_right = 0;
			var->blue.offset = 0;
			var->blue.length = 5;
			var->blue.msb_right = 0;
			var->transp.offset = 15;
			var->transp.length = 1;
			var->transp.msb_right = 0;
            #endif
			break;
		case 32:
            #ifndef CONFIG_RPC_HLD_GMA
			if(var->xres_virtual > 1280)
			{
				FB_PRF("don't support > 1280 x resolution when 32bit gma output\n");
				return -1;
			}
			#endif
			var->red.offset = 16;
			var->red.length = 8;
			var->red.msb_right = 0;
			var->green.offset = 8;
			var->green.length = 8;
			var->green.msb_right = 0;
			var->blue.offset = 0;
			var->blue.length = 8;
			var->blue.msb_right = 0;
			if(var->transp.length)
			{
				var->transp.offset = 24;
				var->transp.length = 8;
				var->transp.msb_right = 0;			
			}
			break;
		default:
			return -1;
	}

	FB_PRF("%s : line %d fb%d bits %d x %d y %d w %d h %d max_w %d max_h %d\n"
		, __FUNCTION__, __LINE__, fbinfo->node, var->bits_per_pixel, var->xoffset
		, var->yoffset, var->xres, var->yres, var->xres_virtual
		, var->yres_virtual);
	
	return 0;
}

static int alifb_set_par(struct fb_info *fbinfo)
{
	struct alifb_info *info = (struct alifb_info *)fbinfo->par;
	struct fb_var_screeninfo *var = &fbinfo->var;
	int ret = 0;
    
    #ifndef CONFIG_RPC_HLD_GMA
    if (info->DE_layer == DE_LAYER3)
    {
        return 0;
    }
	#endif

	fbinfo->fix.line_length = (var->xres_virtual * var->bits_per_pixel) >> 3;	
	if(info->standard_gma_region_init)
    {
	
		#ifdef CONFIG_RPC_HLD_GMA
		struct alifb_gma_info *gma_info = info->gma_info + 0;

		if((gma_info->bpp == (var->bits_per_pixel>>3))
			&& (gma_info->w == var->xres)
			&& (gma_info->h == var->yres))
		{
			FB_PRF("fb has the same fb %d\n", info->id);
			goto EXIT;
		}
		#endif
		info->standard_gma_region_init = 0;
		ret = ali_vpo_delete_GMA_region(info, 0);
		if(ret < 0){
			FB_PRF("%s : delete gma region fail\n", __FUNCTION__);
			goto EXIT;
		}
	}

	{		
		if(var->bits_per_pixel == 8)
			info->dis_format = dis_format_to_ge(DIS_FORMAT_CLUT8);
		else if(var->bits_per_pixel == 16)
		{
			if(var->green.length == 6)
				info->dis_format = dis_format_to_ge(DIS_FORMAT_RGB565);
			else
				info->dis_format = dis_format_to_ge(DIS_FORMAT_ARGB1555);
		}
		else if(var->bits_per_pixel == 32)
		{
			if(var->transp.length == 8)
			info->dis_format = dis_format_to_ge(DIS_FORMAT_ARGB8888);
			else if(var->transp.length == 0)
				info->dis_format = dis_format_to_ge(DIS_FORMAT_RGB888);
			else
			{
				FB_PRF("%s : don't supported transp len %d\n"
					, __FUNCTION__, var->transp.length);
				goto EXIT;
			}
		}
		else{
			ret = -1;
			FB_PRF("%s : don't supported bits per pixel\n", __FUNCTION__);
			goto EXIT;
		}
        
		ret = init_stadard_gma_region(fbinfo);
		if(ret < 0){
			FB_PRF("%s : init standard gma region fail\n", __FUNCTION__);
			goto EXIT;
		}
		info->standard_gma_region_init = 1;
	}
	
EXIT:
	FB_PRF("exit %s : line %d fb%d bits %d x %d y %d w %d h %d max_w %d max_h %d\n"
		, __FUNCTION__, __LINE__, fbinfo->node, var->bits_per_pixel, var->xoffset
		, var->yoffset, var->xres, var->yres, var->xres_virtual
		, var->yres_virtual);	
	return ret;
}

static int alifb_pan_display(struct fb_var_screeninfo *var, struct fb_info *fbinfo)
{
	struct alifb_info *info = (struct alifb_info *)fbinfo->par;	
	struct alifb_gma_info *gma_info = NULL;
    struct gma_rect rect_pars;

    #ifndef CONFIG_RPC_HLD_GMA
    if (info->DE_layer == DE_LAYER3 && (var->vmode & FB_VMODE_YWRAP))
    {
        rect_pars.x = fbinfo->var.xoffset;
        rect_pars.y = fbinfo->var.yoffset;
        rect_pars.w = fbinfo->var.xres;
        rect_pars.h = fbinfo->var.yres;
        
        #ifdef ALI_FB_CREATE_NEW
        fb_create_region(fbinfo, info);
        #else
        fb_create_region(fbinfo, info);
        alifb_set_osd2_position(info, g_fb2_max_width, g_fb2_max_height, (uint32)&rect_pars);
	    ali_set_GMA_IOCtl(info,(uint32)&rect_pars); 
        #endif
        
        return 0;
    }
	#endif		
    
	if(var->xoffset + var->xres > var->xres_virtual
	   || var->yoffset + var->yres > var->yres_virtual){
		FB_PRF("%s : resolution info fail x %d w %d y %d h %d\n"
			, __FUNCTION__, (int)var->xoffset, (int)var->xres, (int)var->yoffset, (int)var->yres);
		return -1;
	}

	gma_info = info->gma_info + 0;
	if(gma_info->valid) {
        if ((info->DE_layer == DE_LAYER3) && (var->vmode & FB_VMODE_YWRAP)) {
            rect_pars.x = var->xoffset;
            rect_pars.y = var->yoffset;
            rect_pars.w = var->xres;
            rect_pars.h = var->yres;
            alifb_set_osd2_position(info, var->xres_virtual, var->yres_virtual, (unsigned long)&rect_pars);
            
            gma_info->x = rect_pars.x;
            gma_info->y = rect_pars.y;
            gma_info->w = rect_pars.w;
            gma_info->h = rect_pars.h;
    		gma_info->pending_flag |= ALIFB_GMA_FLAG_RECT_PENDING;
        } else {
    		gma_info->data_buffer = (uint32)(fbinfo->fix.smem_start 
    								+ gma_info->pitch * var->yoffset + var->xoffset * gma_info->bpp);
    		gma_info->pending_flag |= ALIFB_GMA_FLAG_BUF_PENDING;
        }
		ali_vpo_update_GMA_region(info, 0);
	}

	FB_PRF("%s : line %d fb%d bits %d x %d y %d w %d h %d max_w %d max_h %d\n"
		, __FUNCTION__, __LINE__, fbinfo->node, var->bits_per_pixel, var->xoffset
		, var->yoffset, var->xres, var->yres, var->xres_virtual
		, var->yres_virtual);
	
	return 0;
}

#ifdef CONFIG_RPC_HLD_GMA
static int alifb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			 u_int transp, struct fb_info *fbinfo)
{
	if(regno > 256)
		return -EINVAL;
		
	((uint32 *)(fbinfo->pseudo_palette))[regno] =
		((red   >>  0) & 0xf800) | ((green >>  5) & 0x07e0)|((blue  >> 11) & 0x001f);
	
	return 0;
}
#else
static int alifb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			 u_int transp, struct fb_info *fbinfo)
{
    struct alifb_info *fbi = (struct alifb_info *)fbinfo->par;
    unsigned int val;


	switch (fbi->dis_format)
	{
		case DIS_FORMAT_CLUT8:
			if(regno<256)
			{
				val = ((transp&0xff)<<24) | ((red&0xff)<<16) | ((green&0xff)<<8) | (blue&0xff);
		        ((uint32 *)(fbi->palette_buffer))[regno] = val;
  
                fbi->plt_type = PLT_RGB; // GE_PAL_RGB or GE_PAL_YCBCR
	            fbi->rgb_order = 0;      // enum GE_RGB_ORDER
	            fbi->alpha_range = 0;    // enum GE_ALPHA_RANGE
	            fbi->alpha_pol = 0;      // enum GE_ALPHA_POLARITY
                fbi->plt_alpha_level = PLT_ALPHA_256;
                fbi->pallete_size = 256;
                
                ali_vpo_set_GMA_palette(fbi);
			}
			break;
		case DIS_FORMAT_CLUT4:
			if(regno <= 16)
			{
				val = (0xff<<24) | ((red&0xff)<<16) | ((green&0xff)<<8) | (blue&0xff);
			    ((uint32 *)(fbi->palette_buffer))[regno] = val;

                fbi->plt_type = PLT_YUV;  // GE_PAL_RGB or GE_PAL_YCBCR
	            fbi->rgb_order = 1;       // enum GE_RGB_ORDER
	            fbi->alpha_range = 2;     // enum GE_ALPHA_RANGE
	            fbi->alpha_pol = 0;       // enum GE_ALPHA_POLARITY
                fbi->plt_alpha_level = PLT_ALPHA_16;
                fbi->pallete_size = 16;
                
                ali_vpo_set_GMA_palette(fbi);
			}
			break;
  
		case DIS_FORMAT_ARGB8888:
			break;
		default:
			break;
	}

	return 0;
}
#endif

/* set color registers in batch */
static int alifb_setcmap(struct fb_cmap *cmap, struct fb_info *fbinfo)
{
    #ifdef CONFIG_RPC_HLD_GMA
	struct alifb_info *info = (struct alifb_info *)fbinfo->par;
	int i = 0, start = 0;
	ushort *red, *green, *blue, *transp;

	FB_PRF("%s : line %d cmap_len %d cmap_start %d palette_buffer 0x%x.\n"
		, __FUNCTION__, __LINE__, cmap->len, cmap->start, info->palette_buffer);

	if(info->palette_buffer == NULL)
	{
		FB_PRF("palette buffer is valid\n");

		return -EINVAL;
	}
	
	red = cmap->red;
	green = cmap->green;
	blue = cmap->blue;
	transp = cmap->transp;
	start = cmap->start;

	for (i = 0; i < cmap->len; i++) 
	{
		memcpy((void *)(info->palette_buffer+i*4), (void *)blue, sizeof(unchar));blue++;
		memcpy((void *)(info->palette_buffer+i*4+1), (void *)green, sizeof(unchar));green++;
		memcpy((void *)(info->palette_buffer+i*4+2), (void *)red, sizeof(unchar));red++;
		memcpy((void *)(info->palette_buffer+i*4+3), (void *)transp, sizeof(unchar));transp++;
	}
	info->pallete_size = cmap->len;//256
	info->plt_type = PLT_RGB;
	info->plt_alpha_level = PLT_ALPHA_256;

	__CACHE_FLUSH_ALI(info->palette_buffer, 1024);
		
	ali_vpo_set_GMA_palette(info);
	#else
    struct alifb_info *fbi = (struct alifb_info *)fbinfo->par;
    unsigned int val;
	int i, start = 0;
	u16 *red, *green, *blue, *transp;
    u_int hred, hgreen, hblue, htransp = 0xffff;
    
    red = cmap->red;
	green = cmap->green;
	blue = cmap->blue;
	transp = cmap->transp;
	start = cmap->start;
    
    for (i = 0; i < cmap->len; i++) {
    	hred = *red++;
    	hgreen = *green++;
    	hblue = *blue++;
    	if (transp)
    		htransp = *transp++;
        val = ((htransp&0xff)<<24) | ((hred&0xff)<<16) | ((hgreen&0xff)<<8) | (hblue&0xff);
        ((uint32 *)(fbi->palette_buffer))[i] = val;
    }
    
    switch (fbi->dis_format)
	{
		case DIS_FORMAT_CLUT8:
            {
                fbi->plt_type = PLT_RGB;    // GE_PAL_RGB or GE_PAL_YCBCR
	            fbi->rgb_order = 0;         // enum GE_RGB_ORDER
	            fbi->alpha_range = 0;       // enum GE_ALPHA_RANGE
	            fbi->alpha_pol = 0;         // enum GE_ALPHA_POLARITY
                fbi->plt_alpha_level = PLT_ALPHA_256;
                fbi->pallete_size = 256;
                //ali_vpo_modify_pallete(fbi, regno, red, green, blue, transp);
                ali_vpo_set_GMA_palette(fbi);
	        }
		    break;
        
		case DIS_FORMAT_CLUT4:
			{
                fbi->plt_type = PLT_YUV;  // GE_PAL_RGB or GE_PAL_YCBCR
	            fbi->rgb_order = 1;       // enum GE_RGB_ORDER
	            fbi->alpha_range = 2;     // enum GE_ALPHA_RANGE
	            fbi->alpha_pol = 0;       // enum GE_ALPHA_POLARITY
                fbi->plt_alpha_level = PLT_ALPHA_16;
                fbi->pallete_size = 16;
                
                ali_vpo_set_GMA_palette(fbi);
			}
			break;
  
		case DIS_FORMAT_ARGB8888:
			break;
		default:
			break;
	}
	
	#endif

	return 0;
}

#if 0
static int alifb_mmap(struct fb_info *fbinfo, struct vm_area_struct *vma)
{
	unsigned long start = vma->vm_start;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long pos;
	struct alifb_info *info = (struct alifb_info *)fbinfo->par;
	struct alifb_gma_info *gma_info = NULL;

	if (size > fbinfo->fix.smem_len) {
		FB_PRF("fail size %x smem_len %x\n", (int)size, (int)fbinfo->fix.smem_len);
		return -EINVAL;
	}

	pos = (unsigned long)fbinfo->fix.smem_start;
	if (remap_pfn_range(vma, start, (pos>>PAGE_SHIFT), size, vma->vm_page_prot)) {
		FB_PRF("remap pfn fail\n");
		return -EINVAL;
	}
	
	FB_PRF("virtual add %x phy addr %x\n", (int)start, (int)pos);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
#else		
	vma->vm_flags |= VM_RESERVED | VM_IO;
#endif
	return 0;	
}
#endif
	
/**
    blank the display
*/
static int alifb_blank(int blank, struct fb_info *fbinfo)
{
#if 0    
    int ret = 0;
	struct OSDRect tOpenRect;
	struct alifb_info *info = (struct alifb_info *)fbinfo->par;
	struct OSDPara osd_para;
	HANDLE osd_dev = 0;

	if(info->DE_layer!=DE_LAYER_GMA0 && info->DE_layer!=DE_LAYER_GMA1 && info->DE_layer!=DE_LAYER_MP)
		return -EINVAL;

	if(DE_LAYER_GMA0==info->DE_layer)
		osd_dev = (HANDLE)get_osd_device(0);
	else
		osd_dev = (HANDLE)get_osd_device(1);

    if(0 == blank)
    {
		if(info->DE_layer==DE_LAYER_MP)
		{
			vpo_win_onoff(get_vpo_device_hd(), 1);
			return ret;
		}
		else if(DE_LAYER_GMA0==info->DE_layer)
        {
			tOpenRect.uLeft = fbinfo->var.xoffset;
			tOpenRect.uTop = fbinfo->var.yoffset;
			tOpenRect.uWidth = fbinfo->var.xres;//info->var.xres_virtual;
			tOpenRect.uHeight = fbinfo->var.yres;//info->var.yres_virtual;
		    printk("%s :%d: x=%d, y=%d, w=%d, h=%d\n", __func__, (int)info->DE_layer, tOpenRect.uLeft, tOpenRect.uTop,
		        tOpenRect.uWidth, tOpenRect.uHeight);

			osd_para.uGAlphaEnable = 0;
		    osd_para.uGAlpha = 0xff;
		    osd_para.uPalletteSel = 0;
			osd_para.eMode = OSD_HD_ARGB8888;
			if(info->dis_format == DIS_FORMAT_CLUT8)
		    	osd_para.eMode = OSD_256_COLOR;
			else if(info->dis_format == DIS_FORMAT_CLUT4)
                osd_para.eMode = OSD_16_COLOR;
			else if(info->dis_format == DIS_FORMAT_CLUT2)
			    osd_para.eMode =  OSD_4_COLOR;
			OSDDrv_ShowOnOff(osd_dev, TRUE);
			ret = (int)OSDDrv_CreateRegion(osd_dev, 0, &(tOpenRect), &osd_para);

		}
		else
			 OSDDrv_ShowOnOff(osd_dev, TRUE);
    }
    else
    {
      //  printk("%s:clear sreen !\n", __func__);
      //  ret = (int)OSDDrv_DeleteRegion(osd_dev, 0);
		if(info->DE_layer==DE_LAYER_MP)
			vpo_win_onoff(get_vpo_device_hd(), 0);
		else
            OSDDrv_ShowOnOff(osd_dev, 0);

    }
	return ret;
  #endif  
}

/**
    @info : virtual screen
    we only support virtual screen in vertical, that named FB_VMODE_YWRAP
    example:
    We have a virtual screen that can store double size of
    a real resolution screen need, then we can pass the yoffset to xx_pan_display
    to tell the FB which line you want to start to show.

  when the yoffset is equal to yres, that will affect a screen switch
  (screen1 is replaced by screen2)
  +++++++-------xres_virtual-----+
  ++++++|---------xres-----------|
  |   | |                        |
  |   | |                        |
  | yres|       screen1          |
  |   | |                        |
  |   | |                        |
  |  +++|------------------------|
yres_vir|                        |
  |     |                        |
  |     |                        |
  |     |       screen2          |
  |     |                        |
  |+++++|------------------------|
*/


#ifdef CONFIG_RPC_HLD_GMA
static int alifb_open(struct fb_info *fbinfo, int user)
{
	struct alifb_info *info = (struct alifb_info *)fbinfo->par;

	FB_PRF("%s : line %d\n", __FUNCTION__, __LINE__);
	
	if(user)
		info->open_cnt++;
	
	return 0;
}

static int alifb_release(struct fb_info *fbinfo, int user)
{
	struct alifb_info *info = (struct alifb_info *)fbinfo->par;

	FB_PRF("%s : line %d\n", __FUNCTION__, __LINE__);
	
	if(user)
		info->open_cnt--;
	
	return 0;
}

#else
/**
@info
This function is used for frame buffer open initialize
@para
*/
static int alifb_open(struct fb_info *fbinfo, int user_id)
{
	struct alifb_info *info = (struct alifb_info *)fbinfo->par;

    if(info->DE_layer == DE_LAYER2 || info->DE_layer == DE_LAYER3)
    {
        alifb_gma_open(info);
    }

    return 0;
}

/**
@info
This function is used for frame buffer close deinitialize
@para
*/
static int alifb_close(struct fb_info *fbinfo, int user_id)
{

	struct alifb_info *info = (struct alifb_info *)fbinfo->par;

    if(info->DE_layer == DE_LAYER0)
    {
        printk("user %d close fb1\n", user_id);
    }
    else if(info->DE_layer == DE_LAYER2)
	{
		printk("close gma0\n");
		//gma_m36f_close(0);
	}
	else if(info->DE_layer == DE_LAYER3)
	{
		printk("close gma1\n");
		//gma_m36f_close(1);
	}
    else
    {
        printk("user %d close fb error!!!\n", user_id);
        //SDBBP();
    }

    return 0;
}
#endif

#ifndef CONFIG_RPC_HLD_GMA
/*
 * alifb_mmap take care of 4 things:
 *  - mmap of frambuffer memory (copy from fbmem.c)
 *  - mmap of mmio registers    (copy from fbmem.c)
 *  - mmap of ALiGE CmdQ buffer and CLUT table (TODO)
 */
static int alifb_mmap(struct fb_info *info, struct vm_area_struct *vma)
{
	unsigned long off;
	unsigned long start;
	u32 len;
    char * mmap_type = NULL;

    /* FB_INFO("__G_MM_GE_CMDQ_START_ADDR = 0x%08x\n", __G_MM_GE_CMDQ_START_ADDR); */
    /* FB_INFO("__G_MM_GE_CMDQ_BUF_LEN = 0x%08x\n", __G_MM_GE_CMDQ_BUF_LEN); */

	off = vma->vm_pgoff << PAGE_SHIFT;

	/* frame buffer memory */
	start = info->fix.smem_start;
	len = PAGE_ALIGN((start & ~PAGE_MASK) + info->fix.smem_len);

    /* case 1): mmap of frambuffer memory */
    if (off < len) {
        mmap_type = "Map frambuffer";
        start &= PAGE_MASK;               /* down align to page boundry */
        if ((vma->vm_end - vma->vm_start + off) > len) {
            return -EINVAL;
        }

        /*
         * [Attention] :
         *    ajust offset to be the PHYSICAL address of frambuffer
         */
        off += start;
        goto do_mmap_job;
    }

    /* case 2): mmio */
    if ((off >= len) &&
        (off < len + info->fix.mmio_len)) {
        printk("%s() : [mmio] \n", __func__);
        mmap_type = "MMIO";

        off -= len;
        if (info->var.accel_flags) {
			return -EINVAL;
		}
		start = info->fix.mmio_start;
		len = PAGE_ALIGN((start & ~PAGE_MASK) + info->fix.mmio_len);

        /*
         * [Attention] :
         *    ajust offset to be PHYSICAL address of MMIO register.
         */
        off += start;
        goto do_mmap_job;
    }

    /* case 3): CmdQ Buffer and CLUT table*/
    /* first check alignment */
    if (ali_ge_get_cmdq_buf_addr() & ~PAGE_MASK) {
        printk("%s() : ***BUG***: GE cmdQ buffer is not aligned to page boundary!!!\n", __func__);
        printk("%s() :\tcmdQ buffer addr = 0x%08x, PAGE_MASK is 0x%08x)\n", __func__,
               ali_ge_get_cmdq_buf_addr(), PAGE_MASK);
        return -EINVAL;
    }
    /* then mmap GE cmdQ buffer and CLUT table*/
    if (off >= len + info->fix.mmio_len ) {
        mmap_type = "mmap of GE CmdQ buffer and CLUT table";
        printk("%s() : [mmap ge cmdq buffer and CLUT table] \n", __func__);

        //FB_INFO(" vma region size is : [0x%08lx, 0x%08lx] - size = 0x%08lx\n",
        //        vma->vm_start, vma->vm_end, vma->vm_end - vma->vm_start);

        /* must mmap all cmdq buf */
        if ((vma->vm_end - vma->vm_start) > __G_ALI_MM_GE_CMD_SIZE) {
            //FB_INFO("ERROR : Must mmap all GE cmdQ buffer\n");
            return -EINVAL;
        }

        /*
         * [Attention] :
         *     ajust off to be the physical address of GE Cmdq buffe.
         */
        if (ali_ge_get_cmdq_buf_addr() > 0xA0000000) {
            off = ali_ge_get_cmdq_buf_addr() & ~(0xa0000000);
        } else if(ali_ge_get_cmdq_buf_addr() > 0x80000000) {
            off = ali_ge_get_cmdq_buf_addr() & ~(0x80000000);
        }
        goto do_mmap_job;
    }

do_mmap_job:
	vma->vm_pgoff = off >> PAGE_SHIFT;

    /* This is an IO map - tell maydump to skip this VMA */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
    vma->vm_flags |= VM_IO;
#else	    
	vma->vm_flags |= VM_IO | VM_RESERVED;
#endif
	vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot); /* TODO */

    //FB_INFO("off = 0x%08lx\n", off);
	if (io_remap_pfn_range(vma, vma->vm_start, off >> PAGE_SHIFT,
						   vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
		return -EAGAIN;
    }

    printk("%s() : %s OK\n", __func__, mmap_type);
	return 0;
}
#endif

static struct fb_ops alifb_ops = {
	#ifdef CONFIG_RPC_HLD_GMA
	.fb_open        = alifb_open,
	.fb_release     = alifb_release,
	#endif
	.fb_ioctl       = alifb_ioctl,
	.fb_check_var	= alifb_check_var,
	.fb_set_par	    = alifb_set_par,
	.fb_setcolreg	= alifb_setcolreg,
	
    #ifndef CONFIG_RPC_HLD_GMA
    .fb_mmap        = alifb_mmap,
    #endif
    
	#ifdef CONFIG_RPC_HLD_GMA
	.fb_setcmap     = alifb_setcmap,
	#endif
	.fb_pan_display = alifb_pan_display,
	.fb_fillrect	= alifb_fillrect_accel,
	.fb_copyarea	= alifb_copyarea_accel,
	.fb_imageblit	= alifb_imageblit_accel,
};

static char driver_name[] = "alifb";

/* default var and fix value */
struct fb_var_screeninfo g_alifb_default_var_clut8 = 
{
	720, 576, 720, 576, 0, 0,  /* virtual and visible resolution */
	8, 0, /* bits per pixel and grayscale flag */
	{0, 8, 0}, {0, }, {0, }, {0, }, /*bit field for RGBT */ 
	0, 0, -1, -1, 	
	0,	/* accel_flags */
}; 
EXPORT_SYMBOL(g_alifb_default_var_clut8);

struct fb_var_screeninfo g_alifb_default_var_argb1555 = 
{
	720, 576, 720, 576, 0, 0,  /* virtual and visible resolution */
	16, 0, /* bits per pixel and grayscale flag */
	{10, 5, 0}, {5, 5, 0 }, {0, 5, 0 }, {15, 1, 0 }, /*bit field for RGBT */ 
	0, 0, -1, -1, 	
	0,	/* accel_flags */
}; 
EXPORT_SYMBOL(g_alifb_default_var_argb1555);

struct fb_var_screeninfo g_alifb_default_var_argb8888 = 
{
	720, 576, 720, 576, 0, 0,  /* virtual and visible resolution */
	32, 0, /* bits per pixel and grayscale flag */
	{16, 8, 0}, {8, 8, 0 }, {0, 8, 0}, {24, 8, 0 }, /*bit field for RGBT */ 
	0, 0, -1, -1, 	
	0,	/* accel_flags */
}; 
EXPORT_SYMBOL(g_alifb_default_var_argb8888);

struct fb_fix_screeninfo g_alifb_default_fix =
{
    .id			   = ALI_FB_DEVICE_NAME,
	.type		   = FB_TYPE_PACKED_PIXELS,	/* Packed Pixels	*/
	.visual		   = FB_VISUAL_TRUECOLOR,   /* or FB_VISUAL_PSEUDOCOLOR ?? */
	.type_aux	   = 0,						/* Interleave for interleaved Planes */
	.xpanstep	   = 1,						/* zero if no hardware panning       */
	.ypanstep	   = 1,						/* zero if no hardware panning       */
    .ywrapstep     = 1,                     /* zero if no hardware ywrap         */
	.mmio_start    = M36_GE_REG_BASEADDR,   /* No user space memory mapped register access for ALi devices */
	.mmio_len      = M36_GE_REG_LEN,        /* Length of Memory Mapped I/O       */
	.accel	       = FB_ACCEL_ALI_M36,	
};
EXPORT_SYMBOL(g_alifb_default_fix);
	
static int set_fbinfo_in_default(struct fb_info *fbinfo, int id)
{
	struct alifb_info *info = (struct alifb_info *)fbinfo->par;

	if(id == 3)
	{
		if((g_fb2_width == 0) || (g_fb2_height == 0)
			|| (g_fb2_max_width == 0) || (g_fb2_max_height == 0))
		{
			
			printk("set fb2 fail\n");
			return 0;
		}

        #ifdef CONFIG_RPC_HLD_GMA
		if(g_fb2_bpp == 1)
			memcpy((void *)&fbinfo->var, (void *)&g_alifb_default_var_clut8, sizeof(fbinfo->var));
		else  if(g_fb2_bpp == 2)
			memcpy((void *)&fbinfo->var, (void *)&g_alifb_default_var_argb1555, sizeof(fbinfo->var));
		else
			memcpy((void *)&fbinfo->var, (void *)&g_alifb_default_var_argb8888, sizeof(fbinfo->var));	
		#else
		if(g_fb2_bpp != 1)
		{
			FB_PRF("fb3 don't support color format bpp %d\n",(int)g_fb2_bpp);
			return 0;
		}
		memcpy((void *)&fbinfo->var, (void *)&g_alifb_default_var_clut8, sizeof(fbinfo->var));
		#endif

		fbinfo->var.xres = g_fb2_width;
		fbinfo->var.xres_virtual = g_fb2_max_width;
		fbinfo->var.yres = g_fb2_height;
		fbinfo->var.yres_virtual = g_fb2_max_height;		
	}
	else
	{
		if(g_fb0_bpp == 1)
			memcpy((void *)&fbinfo->var, (void *)&g_alifb_default_var_clut8, sizeof(fbinfo->var));
		else  if(g_fb0_bpp == 2)
			memcpy((void *)&fbinfo->var, (void *)&g_alifb_default_var_argb1555, sizeof(fbinfo->var));
		else
			memcpy((void *)&fbinfo->var, (void *)&g_alifb_default_var_argb8888, sizeof(fbinfo->var));	
		fbinfo->var.xres = g_fb0_width;
		fbinfo->var.xres_virtual = g_fb0_max_width;
		fbinfo->var.yres = g_fb0_height;
		fbinfo->var.yres_virtual = g_fb0_max_height;
	}
	
	memcpy((void *)&fbinfo->fix, (void *)&g_alifb_default_fix, sizeof(g_alifb_default_fix));	
	strcpy(g_alifb_default_fix.id, driver_name);

	if(id == 3)
	{
		fbinfo->fix.line_length = g_fb2_pitch * g_fb2_bpp;
		fbinfo->fix.smem_len = fbinfo->fix.line_length * fbinfo->var.yres_virtual;
		if(fbinfo->fix.smem_len > __G_ALI_MM_FB2_SIZE){
			FB_PRF("allocated fb buffer fail id %d start %x len %x\n", id, (int)__G_ALI_MM_FB2_START_ADDR
				, (int)__G_ALI_MM_FB2_SIZE);
			goto FAIL;
		}
		
        #if defined(CONFIG_ALI_CHIP_M3921)
		fbinfo->fix.smem_start = virt_to_phys(__G_ALI_MM_FB2_START_ADDR);
        #else
		fbinfo->fix.smem_start = __G_ALI_MM_FB2_START_ADDR & 0x1FFFFFFF;		
        #endif
		FB_PRF("frame buffer addr %x size %x pitch %d\n", (int)fbinfo->fix.smem_start, (int)fbinfo->fix.smem_len
			, (int)fbinfo->fix.line_length);
	}
	else if(id == 1)
	{
		if(__G_ALI_MM_FB0_SIZE & 0xFFF)
		{
			FB_PRF("original fb size fail %x. it should be 4K aligned\n", (int)__G_ALI_MM_FB0_SIZE);
			__G_ALI_MM_FB0_SIZE &= 0xFFFFF000;
		}
		fbinfo->fix.line_length = g_fb0_pitch * g_fb0_bpp;
		fbinfo->fix.smem_len = fbinfo->fix.line_length * fbinfo->var.yres_virtual;
		if(fbinfo->fix.smem_len > __G_ALI_MM_FB0_SIZE){
			FB_PRF("allocated fb buffer fail id %d start %x len %x\n", id, (int)__G_ALI_MM_FB0_START_ADDR
				, (int)__G_ALI_MM_FB0_SIZE);
			goto FAIL;
		}		
		fbinfo->fix.smem_len = __G_ALI_MM_FB0_SIZE;
        #if defined(CONFIG_ALI_CHIP_M3921)
		fbinfo->fix.smem_start = virt_to_phys(__G_ALI_MM_FB0_START_ADDR);
        #else
		fbinfo->fix.smem_start = __G_ALI_MM_FB0_START_ADDR & 0x1FFFFFFF;
        #endif		
		FB_PRF("frame buffer addr %x size %x pitch %d\n", (int)fbinfo->fix.smem_start, (int)fbinfo->fix.smem_len
			, (int)fbinfo->fix.line_length);		
	}
	
	info->id = id;
	
	fbinfo->node = -1;
	fbinfo->fbops = &alifb_ops;
	fbinfo->flags = FBINFO_FLAG_DEFAULT;
	fbinfo->pseudo_palette = kmalloc(1024, GFP_KERNEL);

    #if defined(CONFIG_ALI_CHIP_M3921)
	fbinfo->screen_base = (char __iomem *)(phys_to_virt(fbinfo->fix.smem_start));
    #else
	fbinfo->screen_base = (char __iomem *)(fbinfo->fix.smem_start | 0xA0000000);
    #endif
	fbinfo->screen_size = fbinfo->fix.smem_len;

	{
		info->pallete_size = 256;

		if(id == 3)
		{
			info->palette_buffer = __G_ALI_MM_FB2_CMAP_START_ADDR;
		}
		else if(id == 1)
		{
			info->palette_buffer = __G_ALI_MM_FB0_CMAP_START_ADDR;
		}

		if(info->palette_buffer)
		{
			memset(info->palette_buffer, 0, 1024);

			__CACHE_FLUSH_ALI(info->palette_buffer, 1024);
		}

	}

	switch(fbinfo->var.bits_per_pixel)
	{
		case 8:
			info->dis_format = DIS_FORMAT_CLUT8;
			break;
		case 16:
			info->dis_format = DIS_FORMAT_RGB565;
			break;
		case 32:
			info->dis_format = DIS_FORMAT_ARGB8888;
			break;
		default:
			break;
	}

	return 1;
	
FAIL:
	if(fbinfo->pseudo_palette != NULL)
	{
	    kfree(fbinfo->pseudo_palette);
	    fbinfo->pseudo_palette = NULL;
	}
	printk("%s : line %d\n", __FUNCTION__, __LINE__);
	return 0;
}

/*
 *  Initialisation
 */
     
static int __init alifb_probe(struct platform_device *pdev)
{
	struct alifb_info *info = NULL;
	struct fb_info *fbinfo = NULL;	
	int ret = 0;	
	int i = 0;
		
	/* allocate the frame buffer information and set it by the mach_info */
	fbinfo = framebuffer_alloc(sizeof(struct alifb_info), &pdev->dev);
	if (!fbinfo)
	{
		return -ENOMEM;
	}

	if(fb_alloc_cmap(&fbinfo->cmap, 256, 1) < 0)
	{
		FB_PRF("alloc cmap fail\n");
		goto dealloc_fb;
	}
	platform_set_drvdata(pdev, fbinfo);
	
	info = (struct alifb_info *)fbinfo->par;
	memset((void *)info, 0, sizeof(*info));	
	
	info->dev = &pdev->dev;
	info->fbinfo = fbinfo;

	if(g_ali_fb_rpc_arg[0] == NULL){
		for(i = 0;i < MAX_FB_RPC_ARG_NUM;i++){
			g_ali_fb_rpc_arg[i] = kmalloc(MAX_FB_RPC_ARG_SIZE, GFP_KERNEL);
			if(g_ali_fb_rpc_arg[i] == NULL){
				FB_PRF("ali fb malloc rpc arg buf fail\n");
				goto dealloc_fb;
			}
		}
		FB_PRF("ali fb malloc rpc done\n");
	}
	
	if(!set_fbinfo_in_default(fbinfo, pdev->id)) {
		dev_err(&pdev->dev, "fbinfo default fail\n");
		ret = -EINVAL;
		goto dealloc_fb;
	}
	
    #ifdef CONFIG_RPC_HLD_GMA
	if(pdev->id == 1)
		m_fb_info = info;
    #endif

		
	ret = register_framebuffer(fbinfo);
	if (ret < 0) {
		FB_PRF("Failed to register framebuffer device: %d\n",
			ret);
		goto dealloc_fb;
	}
	
	FB_PRF("fb%d: %s frame buffer device\n",
		fbinfo->node, fbinfo->fix.id);
    #ifdef ALI_FB_CREATE_NEW
	if (pdev->id == 1) {		/* OSD layer */
		info->DE_layer = DE_LAYER2;
		init_gma_info(info);
		/* attach and open it */
		ali_vpo_init_GMA(info);
	} else if (pdev->id == 3) {	/* SUBT layer */
		info->DE_layer = DE_LAYER3;
		info->dis_format = DIS_FORMAT_CLUT8;
		init_gma_info(info);
		/* attach and open it */
		ali_vpo_init_GMA(info);
	}
    #else
	if((g_support_standard_fb != 0) && (pdev->id == 1)){
		ret = init_stadard_gma_region(fbinfo);
		if(ret < 0){
			FB_PRF("%s : init standard gma region fail\n", __FUNCTION__);
			goto EXIT;
		}
		info->standard_gma_region_init = 1;
	}
    #endif

EXIT:
	return ret;

dealloc_fb:
	for(i = 0;i < MAX_FB_RPC_ARG_NUM;i++)
	{
		if(g_ali_fb_rpc_arg[i] != NULL)
			kfree((void *)g_ali_fb_rpc_arg[i]);
		g_ali_fb_rpc_arg[i] = NULL;
	}	
	platform_set_drvdata(pdev, NULL);
	fb_dealloc_cmap(&fbinfo->cmap);
	framebuffer_release(fbinfo);
	return ret;
}

/*
 *  Removement
 */
     
static int alifb_remove(struct platform_device *pdev)
{
	struct fb_info *fbinfo = platform_get_drvdata(pdev);
	struct alifb_info *info = NULL;
	int i = 0;

	FB_PRF("%s : line %d\n", __FUNCTION__, __LINE__);
	if (fbinfo) {
		info = (struct alifb_info *)fbinfo->par;
		if(info)
        {
			#ifdef CONFIG_RPC_HLD_GMA
			for(i = 0;i < VIDEO_MAX_GMA_REGION_NUM;i++)
			{
				if(info->gma_info[i].valid)
					ali_vpo_delete_GMA_region(info, i);
			}
			#endif
			
			#ifdef ALI_FB_CREATE_NEW
			ali_vpo_deinit_GMA(info);
			#endif
					
			if(info->gma_buf_start)
				kfree(info->gma_buf_start_addr);

			if(info->accel_flag)
				kfree((void *)info->accel_info.cmd_buf_start_addr);
		}

		if(g_ali_fb_rpc_arg[0] != NULL){		
			for(i = 0;i < MAX_FB_RPC_ARG_NUM;i++){
				if(g_ali_fb_rpc_arg[i] != NULL)
					kfree((void *)g_ali_fb_rpc_arg[i]);
				g_ali_fb_rpc_arg[i] = NULL;
			}
		}
		#ifdef CONFIG_RPC_HLD_GMA
		fb_dealloc_cmap(&fbinfo->cmap);
		#endif
		unregister_framebuffer(fbinfo);		
		framebuffer_release(fbinfo);
	}	
	return 0;
}

static int alifb_suspend(struct platform_device *pdev, pm_message_t state)
{
    #ifndef CONFIG_RPC_HLD_GMA
    unsigned int reg_addr = 0xb8006000;
    unsigned int reg_addr_deo = 0xb8007804;    
    unsigned int reg_value;
    unsigned int is_even_frame ;//= (readl(0xb8006004) >> 18) & 0x3;
    int time_out = 0;
    
    if(pdev->id == 2) 
    {
        while(1)
        {
            is_even_frame = (readl((void __iomem *) 0xb8006004) >> 18) & 0x3;
            reg_value = readl((void __iomem *) reg_addr);
            if((is_even_frame != 0) || (time_out > 20 ))
            {
                break;
            }
            time_out++;
            msleep(5); 
        }        
        reg_value = readl((void __iomem *) reg_addr);
        reg_value &= ~0x1;
        writel(reg_value, (void __iomem *)reg_addr);

        reg_value = readl((void __iomem *) reg_addr_deo);
        reg_value &= ~0x1;
        writel(reg_value, (void __iomem *)reg_addr_deo);        
        msleep(60);        
    }
    #endif

	FB_PRF("ali fb%d suspend\n", pdev->id);

	return 0;
}

static int alifb_resume(struct platform_device *pdev)
{
    #ifndef CONFIG_RPC_HLD_GMA
    unsigned int reg_addr = 0xb8006000;
    unsigned int reg_addr_deo = 0xb8007804;  
    unsigned int reg_value;

    if(pdev->id == 2) {
        reg_value = readl((void __iomem *) reg_addr);
        reg_value |= 0x1;
        writel(reg_value, (void __iomem *)reg_addr);
        
        reg_value = readl((void __iomem *) reg_addr_deo);
        reg_value |= 0x1;
        writel(reg_value, (void __iomem *)reg_addr_deo);        
    }
    #endif

	FB_PRF("ali fb%d resume\n", pdev->id);

	return 0;
}

static struct platform_driver alifb_driver = {
	.probe	 = alifb_probe,
	.remove  = alifb_remove,
	.suspend = alifb_suspend,
	.resume  = alifb_resume,
	.driver  = 
	{
		.name	= "alifb",
		.owner	= THIS_MODULE,			
	},
};

int __init alifb_init(void)
{
	int ret = 0;
	int i = 0;

	for(i = 0;i < MAX_FB_RPC_ARG_NUM;i++)
		g_ali_fb_rpc_arg[i] = NULL;
	
	ret = platform_driver_register(&alifb_driver);
	
	return ret;
}

static void __exit alifb_exit(void)
{
	platform_driver_unregister(&alifb_driver);
}

module_init(alifb_init);
module_exit(alifb_exit);

MODULE_AUTHOR("ALi Corporation");
MODULE_DESCRIPTION("Framebuffer driver for ali's chipset");
MODULE_LICENSE("GPL");
