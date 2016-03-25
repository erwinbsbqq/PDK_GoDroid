
#if 0
#include <linux/kernel.h>
#include <linux/slab.h>

#include <linux/init.h>
//#include <linux/sched.h>
//#include <linux/ioport.h>
//#include <linux/pci.h>
//#include <linux/screen_info.h>

//#include <asm/cpu.h>
//#include <asm/bootinfo.h>
//#include <asm/irq.h>
//#include <asm/mach-ali/prom.h>
//#include <asm/dma.h>
//#include <asm/time.h>
//#include <asm/traps.h>
//#include <linux/console.h>

#include <asm/mach-ali/typedef.h>
//#include <asm/mach-ali/m36_gpio.h>
//#include <asm/reboot.h>


#include <linux/ctype.h>


/* memory mapping configuration start */

/* the top address of the memory */
extern unsigned long __G_MM_TOP_ADDR ;

/* frame buffer area defined for video decoder, it is different with
GMA frame buffer. all the frame buffer must be allocated in the same 32-M segment */
extern unsigned long __G_MM_VIDEO_TOP_ADDR;
extern unsigned long __G_MM_VIDEO_START_ADDR;

extern unsigned long __G_MM_VCAP_FB_SIZE;
extern unsigned long __G_MM_VCAP_FB_ADDR ;

extern unsigned long __G_MM_STILL_FRAME_SIZE;
extern unsigned long __G_MM_STILL_FRAME_ADDR ;

/* shared memory area reserved for the RPC driver*/
extern unsigned long __G_MM_SHARED_MEM_TOP_ADDR;
extern unsigned long __G_MM_SHARED_MEM_START_ADDR;
extern unsigned long __G_RPC_MM_LEN;

/* private memory area reserved for the SEE CPU */
extern unsigned long __G_MM_PRIVATE_AREA_TOP_ADDR ;
extern unsigned long __G_MM_PRIVATE_AREA_START_ADDR;

extern unsigned long __G_MM_VDEC_VBV_START_ADDR;
extern unsigned long __G_MM_VDEC_CMD_QUEUE_ADDR;
extern unsigned long __G_MM_VDEC_LAF_FLAG_BUF_ADDR;
extern unsigned long __G_MM_OSD_BK_ADDR;
extern unsigned long __G_MM_TTX_BS_START_ADDR;
extern unsigned long __G_MM_TTX_PB_START_ADDR;
extern unsigned long __G_MM_TTX_SUB_PAGE_BUF_ADDR;
extern unsigned long __G_MM_TTX_P26_NATION_BUF_ADDR;
extern unsigned long __G_MM_TTX_P26_DATA_BUF_ADDR;
extern unsigned long __G_MM_SUB_BS_START_ADDR;
extern unsigned long __G_MM_SUB_HW_DATA_ADDR;
extern unsigned long __G_MM_SUB_PB_START_ADDR;
extern unsigned long g_fb3_max_width;
extern unsigned long g_fb3_max_height;
extern unsigned long g_fb3_pitch;
extern unsigned long g_fb3_bpp;
extern unsigned long __G_MM_VDEC_VBV_LEN;

/* user data memory area reserved for bootloader -> kenerl */
extern unsigned long __G_MM_BOOTLOGO_DATA_START_ADDR;

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
extern unsigned long __G_MM_SGDMA_MEM_END;
extern unsigned long __G_MM_SGDMA_MEM_START;

/* SEE DMX buffer. */
/* Must keep compatible with TDS macro: #define TS_BLOCK_SIZE 0xbc000. */
extern unsigned long __G_SEE_DMX_SRC_BUF_END ;
extern unsigned long __G_SEE_DMX_SRC_BUF_START;

extern unsigned long __G_SEE_DMX_DECRYPTO_BUF_END;
extern unsigned long __G_SEE_DMX_DECRYPTO_BUF_START;

/* MAIN CPU DMX buffer 
 * Warning: MUST KEEP THIS ADDRESS MAPPING TO PHSYCLAL ADDRESS BELOW 256M!!!
 *          SINCE DMX HW CAN ONLY ACCESS PHSYCLAL ADDRESS WHICH IS BELOW 256M!
 */
extern unsigned long __G_MM_DMX_MEM_TOP_ADDR ;
extern unsigned long __G_MM_DMX_MEM_START_ADDR ;

extern unsigned long __G_MM_TSG_BUF_LEN;
extern unsigned long __G_MM_TSG_BUF_START_ADDR;

/* media player buffer */
extern unsigned long __G_MM_MP_MEM_TOP_ADDR;
extern unsigned long __G_MM_MP_MEM_START_ADDR;

/* frame buffer */

extern unsigned long __G_MM_FB_SIZE;

extern unsigned long __G_GE_CMD_SIZE;
extern unsigned long __G_MM_FB_START_ADDR;
extern unsigned long g_fb_max_width;
extern unsigned long g_fb_max_height;
extern unsigned long g_fb_pitch;
extern unsigned long g_fb_bpp;

/* FIXME - temp here */
extern unsigned long g_support_standard_fb;

/* NIM mode J83B buffer */

extern unsigned long __G_MM_NIM_J83B_MEM_LEN;
extern unsigned long __G_MM_NIM_J83B_MEM_START_ADDR;
	
/* image decoder buffer */
extern unsigned long __G_MM_IMAGE_DECODER_MEM_LEN;
extern unsigned long __G_MM_IMAGE_DECODER_MEM_START_ADDR;
/* the top address of the MAIN CPU */
extern unsigned long __G_MM_MAIN_MEM_NUM;
extern unsigned long __G_MM_MAIN_MEM[][2] ;

/* memory mapping configuration end */

//set ali openvg mempool(including boardsetting) memory alloc config
extern unsigned int g_ali_ovg_phy_addr;  //0xA7F9036C
extern unsigned int g_ali_ovg_mem_size ;

#define MAX_MEM_MAP 0x40

struct ali_mem_map_struct{
	char name[13];
	unsigned  long size;
};

struct ali_mem_map_struct ali_mem_map[MAX_MEM_MAP];


char find_ali_mem_map_index(char *s)
{
	char *src;
	unsigned int index;
	int ret;

	
	for(index=0;index<MAX_MEM_MAP;index++ )
	{
		src = ali_mem_map[index].name;
		ret=strcmp(src,s);
		if(0 == ret)
			return index;
	}

	printk("alimap can not find: %s\n", s);
	return 0xff;
}

void print_ali_mem_map(void)
{

	// set mem map
	printk("__G_MM_TOP_ADDR:%lx\n",__G_MM_TOP_ADDR);

	printk("__G_MM_VIDEO_TOP_ADDR:%lx\n ",__G_MM_VIDEO_TOP_ADDR);
	printk("__G_MM_VIDEO_START_ADDR:%lx\n",__G_MM_VIDEO_START_ADDR);

	printk("__G_MM_VCAP_FB_SIZE:%lx\n",__G_MM_VCAP_FB_SIZE);
	printk("__G_MM_VCAP_FB_ADDR:%lx\n",__G_MM_VCAP_FB_ADDR);

	printk("__G_MM_STILL_FRAME_SIZE:%lx\n",__G_MM_STILL_FRAME_SIZE);
	printk("__G_MM_STILL_FRAME_ADDR:%lx\n ",__G_MM_STILL_FRAME_ADDR);

	printk("__G_MM_SHARED_MEM_TOP_ADDR:%lx\n",__G_MM_SHARED_MEM_TOP_ADDR);
	printk("__G_MM_SHARED_MEM_START_ADDR:%lx\n",__G_MM_SHARED_MEM_START_ADDR);
	printk("__G_RPC_MM_LEN:%lx\n",__G_RPC_MM_LEN);

	printk("__G_MM_PRIVATE_AREA_TOP_ADDR:%lx\n ",__G_MM_PRIVATE_AREA_TOP_ADDR);
	printk("__G_MM_PRIVATE_AREA_START_ADDR:%lx\n",__G_MM_PRIVATE_AREA_START_ADDR);

	printk("__G_MM_VDEC_VBV_START_ADDR:%lx\n",__G_MM_VDEC_VBV_START_ADDR);
	printk("__G_MM_VDEC_CMD_QUEUE_ADDR:%lx\n",__G_MM_VDEC_CMD_QUEUE_ADDR);
	printk("__G_MM_VDEC_LAF_FLAG_BUF_ADDR:%lx\n",__G_MM_VDEC_LAF_FLAG_BUF_ADDR);
	printk("__G_MM_OSD_BK_ADDR:%lx\n",__G_MM_OSD_BK_ADDR);
	printk("__G_MM_TTX_BS_START_ADDR:%lx\n",__G_MM_TTX_BS_START_ADDR);
	printk("__G_MM_TTX_PB_START_ADDR:%lx\n",__G_MM_TTX_PB_START_ADDR);
	printk("__G_MM_TTX_SUB_PAGE_BUF_ADDR:%lx\n",__G_MM_TTX_SUB_PAGE_BUF_ADDR);
	printk("__G_MM_TTX_P26_NATION_BUF_ADDR:%lx\n",__G_MM_TTX_P26_NATION_BUF_ADDR);
	printk("__G_MM_TTX_P26_DATA_BUF_ADDR:%lx\n",__G_MM_TTX_P26_DATA_BUF_ADDR);
	printk("__G_MM_SUB_BS_START_ADDR:%lx\n",__G_MM_SUB_BS_START_ADDR);
	printk("__G_MM_SUB_HW_DATA_ADDR:%lx\n",__G_MM_SUB_HW_DATA_ADDR);
	printk("__G_MM_SUB_PB_START_ADDR:%lx\n",__G_MM_SUB_PB_START_ADDR);
	printk("g_fb3_max_width:%ld\n",g_fb3_max_width);
	printk("g_fb3_max_height:%ld\n",g_fb3_max_height);
	printk("g_fb3_pitch:%ld\n",g_fb3_pitch);
	printk("g_fb3_bpp:%ld\n",g_fb3_bpp);
	printk("__G_MM_VDEC_VBV_LEN:%lx\n",__G_MM_VDEC_VBV_LEN);

	printk("__G_MM_BOOTLOGO_DATA_START_ADDR:%lx\n",__G_MM_BOOTLOGO_DATA_START_ADDR);

	printk("__G_MM_SGDMA_MEM_END:%lx\n",__G_MM_SGDMA_MEM_END);
	printk("__G_MM_SGDMA_MEM_START:%lx\n",__G_MM_SGDMA_MEM_START);


	printk("__G_SEE_DMX_SRC_BUF_END:%lx\n ",__G_SEE_DMX_SRC_BUF_END);
	printk("__G_SEE_DMX_SRC_BUF_START:%lx\n",__G_SEE_DMX_SRC_BUF_START);

	printk("__G_SEE_DMX_DECRYPTO_BUF_END:%lx\n",__G_SEE_DMX_DECRYPTO_BUF_END);
	printk("__G_SEE_DMX_DECRYPTO_BUF_START:%lx\n",__G_SEE_DMX_DECRYPTO_BUF_START);

	printk("__G_MM_DMX_MEM_TOP_ADDR:%lx\n",__G_MM_DMX_MEM_TOP_ADDR);
	printk("__G_MM_DMX_MEM_START_ADDR:%lx\n",__G_MM_DMX_MEM_START_ADDR);

	printk("__G_MM_TSG_BUF_LEN:%lx\n",__G_MM_TSG_BUF_LEN);
	printk("__G_MM_TSG_BUF_START_ADDR:%lx\n",__G_MM_TSG_BUF_START_ADDR);

	printk("__G_MM_MP_MEM_TOP_ADDR:%lx\n",__G_MM_MP_MEM_TOP_ADDR);
	printk("__G_MM_MP_MEM_START_ADDR:%lx\n",__G_MM_MP_MEM_START_ADDR);

	printk("__G_MM_FB_SIZE:%lx\n",__G_MM_FB_SIZE);

	printk("__G_GE_CMD_SIZE:%lx\n",__G_GE_CMD_SIZE);
	printk("__G_MM_FB_START_ADDR:%lx\n",__G_MM_FB_START_ADDR);
	printk("g_fb_max_width:%ld\n",g_fb_max_width);
	printk("g_fb_max_height:%ld\n",g_fb_max_height);
	printk("g_fb_pitch:%ld\n",g_fb_pitch);
	printk("g_fb_bpp:%ld\n",g_fb_bpp);

	printk("g_support_standard_fb:%ld\n",g_support_standard_fb);


	printk("__G_MM_NIM_J83B_MEM_LEN:%lx\n",__G_MM_NIM_J83B_MEM_LEN);
	printk("__G_MM_NIM_J83B_MEM_START_ADDR:%lx\n",__G_MM_NIM_J83B_MEM_START_ADDR);

	printk("__G_MM_IMAGE_DECODER_MEM_LEN:%lx\n",__G_MM_IMAGE_DECODER_MEM_LEN);
	printk("__G_MM_IMAGE_DECODER_MEM_START_ADDR:%lx\n",__G_MM_IMAGE_DECODER_MEM_START_ADDR);
	printk("__G_MM_MAIN_MEM_NUM:%ld\n",__G_MM_MAIN_MEM_NUM);
	printk("__G_MM_MAIN_MEM[0][1]:%lx\n",__G_MM_MAIN_MEM[0][1]);
	printk("g_ali_ovg_phy_addr:%x\n",g_ali_ovg_phy_addr);
	printk("g_ali_ovg_mem_size:%x\n",g_ali_ovg_phy_addr);

	return;
	
}

void set_ali_mem_map(void)
{
	unsigned char index;
	
	unsigned long mm_top; //__G_MM_TOP_ADDR ;
	unsigned long video_size;//__G_MM_VIDEO_TOP_ADDR;
	unsigned long video_top;//__G_MM_VIDEO_TOP_ADDR;
	unsigned long video_start;//__G_MM_VIDEO_START_ADDR;
	unsigned long vcap_fb_size;//__G_MM_VCAP_FB_SIZE;
	unsigned long vcab_fb_addr;//__G_MM_VCAP_FB_ADDR ;
	unsigned long still_frame_size;//__G_MM_STILL_FRAME_SIZE;
	unsigned long still_frame_addr;//__G_MM_STILL_FRAME_ADDR ;
	unsigned long share_mem_size;//__G_MM_SHARED_MEM_TOP_ADDR;
	unsigned long share_mem_top;//__G_MM_SHARED_MEM_TOP_ADDR;
	unsigned long share_mem_start;//__G_MM_SHARED_MEM_START_ADDR;
	unsigned long rpc_mem_len;//__G_RPC_MM_LEN;
	unsigned long priv_mem_len;//__G_MM_PRIVATE_AREA_TOP_ADDR ;
	unsigned long priv_mem_top;//__G_MM_PRIVATE_AREA_TOP_ADDR ;
	unsigned long priv_mem_start;//__G_MM_PRIVATE_AREA_START_ADDR;
	unsigned long vdec_vbv_start;//__G_MM_VDEC_VBV_START_ADDR;
	unsigned long vdec_cmd_que_start;//__G_MM_VDEC_CMD_QUEUE_ADDR;
	unsigned long vdec_laf_flag_buf;//__G_MM_VDEC_LAF_FLAG_BUF_ADDR;
	unsigned long osd_bk_addr;//__G_MM_OSD_BK_ADDR;
	unsigned long ttx_bs_start;//__G_MM_TTX_BS_START_ADDR;
	unsigned long ttx_pb_start;//__G_MM_TTX_PB_START_ADDR;
	unsigned long ttx_sub_page_buf;//__G_MM_TTX_SUB_PAGE_BUF_ADDR;
	unsigned long ttx_p26_nation_buf;//__G_MM_TTX_P26_NATION_BUF_ADDR;
	unsigned long ttx_p26_data_buf;//__G_MM_TTX_P26_DATA_BUF_ADDR;
	unsigned long sub_bs_start;//__G_MM_SUB_BS_START_ADDR;
	unsigned long sub_hw_data;//__G_MM_SUB_HW_DATA_ADDR;
	unsigned long sub_pb_start;// __G_MM_SUB_PB_START_ADDR;
	unsigned long fb3_width;//g_fb3_max_width;
	unsigned long fb3_high;//g_fb3_max_height;
	unsigned long fb3_pitch;//g_fb3_pitch;
	unsigned long fb3_bpp;//g_fb3_bpp;
	unsigned long vdec_vbv_len;//__G_MM_VDEC_VBV_LEN;
	unsigned long user_data;//__MM_USER_DATA_MEM_START;
	unsigned long bootlogo;//__G_MM_BOOTLOGO_DATA_START_ADDR;
	unsigned long sgdma_end;//__G_MM_SGDMA_MEM_END;
	unsigned long sgdma_start;//__G_MM_SGDMA_MEM_START;
	unsigned long see_dmx_buf_end;//__G_SEE_DMX_SRC_BUF_END ;
	unsigned long see_dmx_buf_start;//__G_SEE_DMX_SRC_BUF_START;
	unsigned long see_dmx_decrypto_end;//__G_SEE_DMX_DECRYPTO_BUF_END;
	unsigned long see_dmx_decrypto_start;//__G_SEE_DMX_DECRYPTO_BUF_START;
	unsigned long dmx_top;//__G_MM_DMX_MEM_TOP_ADDR ;
	unsigned long dmx_start;//__G_MM_DMX_MEM_START_ADDR ;
	unsigned long tsg_buf_len;//__G_MM_TSG_BUF_LEN;
	unsigned long tsg_buf_start;//__G_MM_TSG_BUF_START_ADDR;
	unsigned long mplay_top;//__G_MM_MP_MEM_TOP_ADDR;
	unsigned long mplay_start;//__G_MM_MP_MEM_START_ADDR;
	unsigned long fb_size;//__G_MM_FB_SIZE;
//	unsigned long ge_cmd_size;//__G_GE_CMD_SIZE;
	unsigned long fb_start;//__G_MM_FB_START_ADDR;
	unsigned long fb_width;//g_fb_max_width;
	unsigned long fb_high;//g_fb_max_height;
	unsigned long fb_pitch;//g_fb_pitch;
	unsigned long fb_bpp;//g_fb_bpp;
	//unsigned long ;//g_support_standard_fb;
	unsigned long nim_j83b_len;//__G_MM_NIM_J83B_MEM_LEN;
	unsigned long nim_j83b_start;//__G_MM_NIM_J83B_MEM_START_ADDR;
	unsigned long img_decode_len;//__G_MM_IMAGE_DECODER_MEM_LEN;
	unsigned long img_decode_start;//__G_MM_IMAGE_DECODER_MEM_START_ADDR;
	unsigned long main_mem_num;//__G_MM_MAIN_MEM_NUM = 2;
	//unsigned long ;//__G_MM_MAIN_MEM[][2] = {{0xA0000000, __MM_IMAGE_DECODER_MEM_START_ADDR}
	//				   ,{0xA8000000, 0xAFFFFFFF}};
	unsigned int ovg_phy_addr;//g_ali_ovg_phy_addr = 0xA8000000;  //0xA7F9036C
	unsigned int ovg_mem_size;//g_ali_ovg_mem_size = 0x2000000;


	// check mem map
	index = find_ali_mem_map_index("top");
	if(0xff == index)
		goto fail;
	mm_top = ali_mem_map[index].size;
	index = find_ali_mem_map_index("video");
	if(0xff == index)
		goto fail;
	video_size =  ali_mem_map[index].size;
	video_top =mm_top;
	video_start = video_top - video_size;
	index = find_ali_mem_map_index("vcap_fb");
	if(0xff == index)
		goto fail;
	vcap_fb_size = ali_mem_map[index].size;
	vcab_fb_addr = video_start - vcap_fb_size;
	index = find_ali_mem_map_index("still_frame");
	if(0xff == index)
		goto fail;
	still_frame_size = ali_mem_map[index].size;
	still_frame_addr = vcab_fb_addr - still_frame_size;
//	index = find_ali_mem_map_index("sys2");
//	if(0xff = index)
//		goto fail;
	index = find_ali_mem_map_index("share");
	if(0xff == index)
		goto fail;
	share_mem_size = ali_mem_map[index].size;
	share_mem_top = mm_top - video_size - vcap_fb_size - still_frame_size;
	share_mem_start = share_mem_top - share_mem_size;
	rpc_mem_len = share_mem_size;
	index = find_ali_mem_map_index("priv");
	if(0xff == index)
		goto fail;
	priv_mem_len = ali_mem_map[index].size -share_mem_size - vcap_fb_size - still_frame_size;
	priv_mem_top = share_mem_start;
	priv_mem_start = priv_mem_top  - priv_mem_len;
	index = find_ali_mem_map_index("fb3_w");
	if(0xff == index)
		goto fail;
	fb3_width = ali_mem_map[index].size;
	index = find_ali_mem_map_index("fb3_h");
	if(0xff == index)
		goto fail;
	fb3_high = ali_mem_map[index].size;
	index = find_ali_mem_map_index("fb3_bpp");
	if(0xff == index)
		goto fail;
	fb3_bpp = ali_mem_map[index].size;
	index = find_ali_mem_map_index("fb3_pitch");
	if(0xff == index)
		goto fail;
	fb3_pitch = ali_mem_map[index].size;
	index = find_ali_mem_map_index("vdec_vbv");
	if(0xff == index)
		goto fail;
	vdec_vbv_len = ali_mem_map[index].size;
	vdec_vbv_start = share_mem_start - vdec_vbv_len;
	index = find_ali_mem_map_index("vdec_cmd_que");
	if(0xff == index)
		goto fail;
	vdec_cmd_que_start = (vdec_vbv_start - ali_mem_map[index].size)&0xFFFFFF00;	//256 bytes alignment
	index = find_ali_mem_map_index("vdec_laf_buf");
	if(0xff == index)
		goto fail;
	vdec_laf_flag_buf = (vdec_cmd_que_start -  ali_mem_map[index].size)&0xFFFFFC00;  //1024 bytes alignment
	index = find_ali_mem_map_index("osd_bk2");
	if(0xff == index)
		goto fail;
	osd_bk_addr = (vdec_laf_flag_buf - ali_mem_map[index].size)&0xFFFFF800;
	index = find_ali_mem_map_index("ttx_bs");
	if(0xff == index)
		goto fail;
	ttx_bs_start = (osd_bk_addr - ali_mem_map[index].size)&0xFFFFFFFC;
	index = find_ali_mem_map_index("ttx_pb");
	if(0xff == index)
		goto fail;
	ttx_pb_start = (ttx_bs_start -ali_mem_map[index].size)&0xFFFFFFFC;
	index = find_ali_mem_map_index("ttx_sub_page");
	if(0xff == index)
		goto fail;
	ttx_sub_page_buf = ttx_pb_start - ali_mem_map[index].size;
	index = find_ali_mem_map_index("ttx_p26_nation");
	if(0xff == index)
		goto fail;
	ttx_p26_nation_buf = ttx_sub_page_buf - ali_mem_map[index].size;
	index = find_ali_mem_map_index("ttx_p26_data");
	if(0xff == index)
		goto fail;
	ttx_p26_data_buf = ttx_p26_nation_buf - ali_mem_map[index].size;
	index = find_ali_mem_map_index("sub_bs");
	if(0xff == index)
		goto fail;
	sub_bs_start = (ttx_p26_data_buf - ali_mem_map[index].size)&0xFFFFFFFC;
	index = find_ali_mem_map_index("sub_hw_data");
	if(0xff == index)
		goto fail;
	sub_hw_data = (sub_bs_start - ali_mem_map[index].size)&0xFFFFFFF0;
	index = find_ali_mem_map_index("sub_pb");
	if(0xff == index)
		goto fail;
	sub_pb_start = (sub_hw_data -  ali_mem_map[index].size)&0xFFFFFFFC;

	
	index = find_ali_mem_map_index("user_data");
	if(0xff == index)
		goto fail;
	user_data = priv_mem_start - ali_mem_map[index].size;  //??????
	index = find_ali_mem_map_index("bootlogo");
	if(0xff == index)
		goto fail;
	bootlogo = user_data - ali_mem_map[index].size;
	index = find_ali_mem_map_index("sgdma");
	if(0xff == index)
		goto fail;
	sgdma_end = bootlogo;
	sgdma_start = sgdma_end - ali_mem_map[index].size;
	index = find_ali_mem_map_index("dmx_see");
	if(0xff == index)
		goto fail;
	see_dmx_buf_end = sgdma_start;
	see_dmx_buf_start = see_dmx_buf_end -ali_mem_map[index].size;
	see_dmx_decrypto_end = see_dmx_buf_start;
	see_dmx_decrypto_start = see_dmx_decrypto_end -ali_mem_map[index].size;
	index = find_ali_mem_map_index("dmx_main");
	if(0xff == index)
		goto fail;
	dmx_top = see_dmx_decrypto_start;
	dmx_start = dmx_top -ali_mem_map[index].size;
	index = find_ali_mem_map_index("tsg");
	if(0xff == index)
		goto fail;
	tsg_buf_len = ali_mem_map[index].size;;
	tsg_buf_start = dmx_start -tsg_buf_len;
	index = find_ali_mem_map_index("mplay");
	if(0xff == index)
		goto fail;
	mplay_top = tsg_buf_start;
	mplay_start = mplay_top - ali_mem_map[index].size;
	index = find_ali_mem_map_index("fb_w");
	if(0xff == index)
		goto fail;
	fb_width =  ali_mem_map[index].size;
	index = find_ali_mem_map_index("fb_h");
	if(0xff == index)
		goto fail;
	fb_high =  ali_mem_map[index].size;
	index = find_ali_mem_map_index("fb_bpp");
	if(0xff == index)
		goto fail;
	fb_bpp =  ali_mem_map[index].size;
	index = find_ali_mem_map_index("fb_pitch");
	if(0xff == index)
		goto fail;
	fb_pitch =  ali_mem_map[index].size;
	fb_size = ((fb_high*fb_pitch*fb_bpp)+0xfff)& 0xFFFFF000;			//(((FB_HEIGHT * FB_PITCH * FB_BPP) + 0xFFF) & 0xFFFFF000)
	fb_start = (mplay_start - fb_size)& 0xFFFFFFF0;	//((__MM_MP_MEM_START_ADDR - FB_MEM_SIZE) & 0xFFFFFFF0)
	
	index = find_ali_mem_map_index("nim_j83b");
	if(0xff == index)
		goto fail;
	nim_j83b_len = ali_mem_map[index].size;
	nim_j83b_start = fb_start - nim_j83b_len;
	index = find_ali_mem_map_index("img");
	if(0xff == index)
		goto fail;
	img_decode_len = ali_mem_map[index].size;
	img_decode_start = nim_j83b_start - img_decode_len;

	main_mem_num = 2;
	
	index = find_ali_mem_map_index("ovg_addr");
	if(0xff == index)
		goto fail;
	ovg_phy_addr = ali_mem_map[index].size;
	index = find_ali_mem_map_index("ovg_len");
	if(0xff == index)
		goto fail;
	ovg_mem_size= ali_mem_map[index].size;

	
	// set mem map
	__G_MM_TOP_ADDR =  mm_top;

	__G_MM_VIDEO_TOP_ADDR = video_top;
	__G_MM_VIDEO_START_ADDR = video_start;

	__G_MM_VCAP_FB_SIZE = vcap_fb_size;
	__G_MM_VCAP_FB_ADDR = vcab_fb_addr;

	__G_MM_STILL_FRAME_SIZE = still_frame_size;
	__G_MM_STILL_FRAME_ADDR = still_frame_addr;

	__G_MM_SHARED_MEM_TOP_ADDR = share_mem_top;
	__G_MM_SHARED_MEM_START_ADDR = share_mem_start;
	__G_RPC_MM_LEN = rpc_mem_len;

	__G_MM_PRIVATE_AREA_TOP_ADDR = priv_mem_top;
	__G_MM_PRIVATE_AREA_START_ADDR = priv_mem_start;

	__G_MM_VDEC_VBV_START_ADDR = vdec_vbv_start;
	__G_MM_VDEC_CMD_QUEUE_ADDR = vdec_cmd_que_start;
	__G_MM_VDEC_LAF_FLAG_BUF_ADDR = vdec_laf_flag_buf;
	__G_MM_OSD_BK_ADDR = osd_bk_addr;
	__G_MM_TTX_BS_START_ADDR = ttx_bs_start;
	__G_MM_TTX_PB_START_ADDR = ttx_pb_start;
	__G_MM_TTX_SUB_PAGE_BUF_ADDR = ttx_sub_page_buf;
	__G_MM_TTX_P26_NATION_BUF_ADDR = ttx_p26_nation_buf;
	__G_MM_TTX_P26_DATA_BUF_ADDR = ttx_p26_data_buf;
	__G_MM_SUB_BS_START_ADDR = sub_bs_start;
	__G_MM_SUB_HW_DATA_ADDR = sub_hw_data;
	__G_MM_SUB_PB_START_ADDR = sub_pb_start;
	g_fb3_max_width = fb_width;
	g_fb3_max_height = fb_high;
	g_fb3_pitch = fb_pitch;
	g_fb3_bpp = fb_bpp;
	__G_MM_VDEC_VBV_LEN = vdec_vbv_len;

	__G_MM_BOOTLOGO_DATA_START_ADDR = bootlogo;

	__G_MM_SGDMA_MEM_END = sgdma_end;
	__G_MM_SGDMA_MEM_START = sgdma_start;


	__G_SEE_DMX_SRC_BUF_END = see_dmx_buf_end;
	__G_SEE_DMX_SRC_BUF_START = see_dmx_buf_start;

	__G_SEE_DMX_DECRYPTO_BUF_END = see_dmx_decrypto_end;
	__G_SEE_DMX_DECRYPTO_BUF_START = see_dmx_decrypto_start;

	__G_MM_DMX_MEM_TOP_ADDR = dmx_top;
	__G_MM_DMX_MEM_START_ADDR = dmx_start;

	__G_MM_TSG_BUF_LEN = tsg_buf_len;
	__G_MM_TSG_BUF_START_ADDR = tsg_buf_start;

	__G_MM_MP_MEM_TOP_ADDR = mplay_top;
	__G_MM_MP_MEM_START_ADDR = mplay_start;

	__G_MM_FB_SIZE = fb_size;

//	__G_GE_CMD_SIZE = ge_cmd_size;
	__G_MM_FB_START_ADDR = fb_start;
	g_fb_max_width = fb_width;
	g_fb_max_height = fb_high;
	g_fb_pitch = fb_pitch;
	g_fb_bpp = fb_bpp;

//	g_support_standard_fb;


	__G_MM_NIM_J83B_MEM_LEN = nim_j83b_len;
	__G_MM_NIM_J83B_MEM_START_ADDR = nim_j83b_start;

	__G_MM_IMAGE_DECODER_MEM_LEN = img_decode_len;
	__G_MM_IMAGE_DECODER_MEM_START_ADDR = img_decode_start;
//	__G_MM_MAIN_MEM_NUM = main_mem_num;
	
	__G_MM_MAIN_MEM[0][1] = __G_MM_IMAGE_DECODER_MEM_START_ADDR;

	g_ali_ovg_phy_addr = ovg_phy_addr;  //0xA7F9036C
	g_ali_ovg_mem_size = ovg_mem_size;

	
fail:
	return;
	
}

char *new_ali_mem_map(char *s, char *name, unsigned  long *size)
{
	 char *name_start = NULL;
	int name_len;
	char delim;

	*size = memparse(s, &s);

	delim = 0;
        /* now look for name */
	if (*s == '(')
	{
		delim = ')';
	}

	if (delim)
	{
		char *p;

	    	name_start = ++s;
		p = strchr(name_start, delim);
		if (!p)
		{
			printk( "no closing %c found \n", delim);
			return NULL;
		}
		name_len = p - name_start;
		s = p + 1;
		strlcpy(name, name_start, name_len + 1);
	}


	return s;
}


void __init parse_ali_mem_map(char *s)
{
	char *s_ret;
	unsigned int i = 0;

		
	if( s == NULL)
		return;
	i = 0;
	while( 1)
	{
		s_ret = new_ali_mem_map(s,ali_mem_map[i].name,&(ali_mem_map[i].size));
		printk("ali_mem_map[%d]name:<%s>, size:0x%lx, \n",i,ali_mem_map[i].name,ali_mem_map[i].size);
		i++;

		s = s_ret;
		if (*s != ',')
			break;
		s++;
	}

}


 int __init program_setup_ali_mem_map(char *s)
{
	if(s)
	{
		//printk("cmdline: alimap = %s\n", s);
		parse_ali_mem_map(s);
		//printk("mem map from board_config.c: \n");
		//print_ali_mem_map();
		set_ali_mem_map();
		//printk("\n mem map from cmdline: \n");
		//print_ali_mem_map();
	}
	return 0;
}
 
__setup("alimap=", program_setup_ali_mem_map);
#endif

