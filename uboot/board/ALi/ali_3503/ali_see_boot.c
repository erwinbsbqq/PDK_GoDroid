#include <common.h>

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

/* FB3 Size */
#define FB3_VIRTUAL_WIDTH                   1280//1920
#define FB3_VIRTUAL_HEIGHT                  720//1080
#define FB3_WIDTH                                   1280//1920
#define FB3_HEIGHT                                  720//1080
#define FB3_BPP                                        4// bytes per pixel
#define FB3_PITCH                                    FB3_VIRTUAL_WIDTH

#define FB3_MEM_SIZE                              (((FB3_VIRTUAL_HEIGHT * FB3_PITCH * FB3_BPP) + 0xFFF) & 0xFFFFF000)

/* Video Size */
#define VDEC_FB_MEM_SIZE		   	   	   (0x02000000)
#define VCAP_FB_MEM_SIZE				   (736*576*2*4)
#define STILL_FB_MEM_SIZE		   		   (0x300000)
#define VDEC_VBV_MEM_SIZE				   (0x600000)
#define VDEC_CMD_QUEUE_MEM_SIZE 		   (0x10000)
#define VDEC_LAF_FLAG_MME_SIZE 	          (0x10800)

/* MCAPI Size */
#define MCAPI_MEM_SIZE					   (0x2000)

/* SEE Size */
#define SHARED_MEM_SIZE  				   (4096)
#define DSC_MEM_SIZE					   (10240) //  DSC , tmp added for MCAPI limitation
#define SEE_MEM_SIZE					   (0x02000000 - SHARED_MEM_SIZE - VDEC_VBV_MEM_SIZE - MCAPI_MEM_SIZE - DSC_MEM_SIZE)

/* Audio Size */
#define AUDIO_MEM_SIZE				   	   (0x100000)

/* User Confg Size*/
#define USER_DATA_MEM_SIZE  			   (0x1000)		// 4k

/* NOR Flash Size */


/* TTX/SUB Size */


/* Boot Logo Size */
#define BOOTLOGO_DATA_MEM_SIZE 	    	(0x100000)

/*Dmx Size*/
#define DMX_SEE_BUF_SIZE  			    (0xBC000)
#define DMX_MAIN_BUF_SIZE		           (0x00100000)

#define DMX_MEM_SIZE				    (DMX_MAIN_BUF_SIZE * 2 + DMX_SEE_BUF_SIZE * 3)

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
#define MALI_UMP_MEM_SIZE                (0x02000000) 

/* AES Size */
#define ALI_DSC_KERNEL_BUFFER_SIZE          (0x100000)

#define ALI_DSC_KERNEL_BUFFER_SHA_OUT_SIZE  (0x80)
#define ALI_DSC_KERNEL_BUFFER_SHA_SIZE       (0x200000)

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

/* SHARED
*/
#define __MM_SEE_MAIN_SHARED_MEM_START         (__MM_SEE_PRIVATE_END) 
#define __MM_SEE_MAIN_SHARED_MEM_END             (__MM_SEE_MAIN_SHARED_MEM_START + SHARED_MEM_SIZE)

/* MCAPI
*/ 
#define __MM_MCAPI_MEM_START                           	(__MM_SEE_MAIN_SHARED_MEM_END)
#define __MM_MCAPI_MEM_END                       		(__MM_MCAPI_MEM_START + MCAPI_MEM_SIZE)

/* DSC , tmp added for MCAPI limitation
*/ 
#define __MM_DSC_MEM_START                           	(__MM_MCAPI_MEM_END)
#define __MM_DSC_MEM_END                       			(__MM_DSC_MEM_START + DSC_MEM_SIZE)

/* Video VBV
*/
#define __MM_VBV_BUF_START                    			(__MM_DSC_MEM_END) 
#define __MM_VBV_BUF_END                      			(__MM_VBV_BUF_START + VDEC_VBV_MEM_SIZE)

/* Video Frame Buffer
*/
#define __MM_VIDEO_BUF_START                  		((__MM_VBV_BUF_END + 0xFFF) & 0xFFFFF000)
#define __MM_VIDEO_BUF_END                    			(__MM_VIDEO_BUF_START + VDEC_FB_MEM_SIZE)

/* AUDIO
*/
#define __MM_AUDIO_ENGINE_START                		(__MM_VIDEO_BUF_END) 
#define __MM_AUDIO_ENGINE_END                  		(__MM_AUDIO_ENGINE_START + AUDIO_MEM_SIZE)

/* below the __MM_RESERVED_ADDR1 */
/* User configuration.
*/
#define __MM_USER_DATA_END                    		(__MM_RESERVED_ADDR1)
#define __MM_USER_DATA_START                  		(__MM_USER_DATA_END - USER_DATA_MEM_SIZE) 

/* VDEC LAF
*/
#define __MM_VDEC_LAF_FLAG_BUF_START          	((__MM_USER_DATA_START - VDEC_LAF_FLAG_MME_SIZE)\
													& 0xFFFFFC00)
#define __MM_VDEC_LAF_FLAG_BUF_END            	(__MM_VDEC_LAF_FLAG_BUF_START + VDEC_LAF_FLAG_MME_SIZE)

/* VDEC CMD Queue
*/
#define __MM_VDEC_CMD_QUEUE_START             	((__MM_VDEC_LAF_FLAG_BUF_START - VDEC_CMD_QUEUE_MEM_SIZE)\
													& 0xFFFFFF00)
#define __MM_VDEC_CMD_QUEUE_END               		(__MM_VDEC_CMD_QUEUE_START + VDEC_CMD_QUEUE_MEM_SIZE)

/* VCAP FB
*/
#define __MM_VCAP_FB_START                    			((__MM_VDEC_CMD_QUEUE_START - VCAP_FB_MEM_SIZE)\
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

/* Boot Logo
*/
#define __MM_BOOTLOGO_DATA_START			(__MM_NIM_J83B_MEM_START - BOOTLOGO_DATA_MEM_SIZE)
#define __MM_BOOTLOGO_DATA_END				(__MM_BOOTLOGO_DATA_START + BOOTLOGO_DATA_MEM_SIZE)

/*  AES
*/
#define __MM_AES_INPUT_DATA_START                 (__MM_BOOTLOGO_DATA_START - ALI_DSC_KERNEL_BUFFER_SIZE)
#define __MM_AES_INPUT_DATA_END                   (__MM_AES_INPUT_DATA_START + ALI_DSC_KERNEL_BUFFER_SIZE)

#define __MM_AES_OUTPUT_DATA_START                 (__MM_AES_INPUT_DATA_START - ALI_DSC_KERNEL_BUFFER_SIZE)
#define __MM_AES_OUTPUT_DATA_END                   (__MM_AES_OUTPUT_DATA_START + ALI_DSC_KERNEL_BUFFER_SIZE)

/* SHA
*/
#define __MM_SHA_INPUT_DATA_START                 (__MM_AES_OUTPUT_DATA_START - ALI_DSC_KERNEL_BUFFER_SHA_SIZE)
#define __MM_SHA_INPUT_DATA_END                   (__MM_SHA_INPUT_DATA_START + ALI_DSC_KERNEL_BUFFER_SHA_SIZE)

#define __MM_SHA_OUTPUT_DATA_START                 (__MM_SHA_INPUT_DATA_START - ALI_DSC_KERNEL_BUFFER_SHA_OUT_SIZE)
#define __MM_SHA_OUTPUT_DATA_END                   (__MM_SHA_OUTPUT_DATA_START + ALI_DSC_KERNEL_BUFFER_SHA_OUT_SIZE)

/* TOP and Bottom1
*/
#define __MM_ALI_PRIVATE_MEM_START1			(__MM_SHA_OUTPUT_DATA_START)	
#define __MM_ALI_PRIVATE_MEM_END1				(__MM_AUDIO_ENGINE_END)

/* Second Channel Start Addr for 1G */
#define __MM_RESERVED_ADDR2_1G					(0xA0000000)

/* FB0
*/
#define __MM_FB_MEM_START_1G                     			(__MM_RESERVED_ADDR2_1G)
#define __MM_FB_MEM_END_1G                       			(__MM_FB_MEM_START_1G + FB0_MEM_SIZE)

/* FB3
*/
#define __MM_FB3_MEM_START_1G                     		((__MM_FB_MEM_END_1G + 0xFFF)\
														& 0xFFFFF000)
#define __MM_FB3_MEM_END_1G                       			(__MM_FB3_MEM_START_1G + FB3_MEM_SIZE)

/* MALI
*/
#define __MM_MALI_MEM_START_1G					((__MM_FB3_MEM_END_1G + 0xFF)\
														& 0xFFFFFF00)
#define __MM_MALI_MEM_END_1G						(__MM_MALI_MEM_START_1G + MALI_DEDICATED_MEM_SIZE)

/* MALI UMP
*/
#define __MM_MALI_UMP_MEM_START_1G				((__MM_MALI_MEM_END_1G + 0xFF)\
														& 0xFFFFFF00)
#define __MM_MALI_UMP_MEM_END_1G				(__MM_MALI_UMP_MEM_START_1G + MALI_UMP_MEM_SIZE)

/* TOP and Bottom2
*/
#define __MM_ALI_PRIVATE_MEM_START2_1G			(__MM_FB_MEM_START_1G)	
#define __MM_ALI_PRIVATE_MEM_END2_1G			(__MM_MALI_UMP_MEM_END_1G)

/* Second Channel Start Addr for 512M */
#define __MM_RESERVED_ADDR2_512M					(0x90000000)

/* FB0
*/
#define __MM_FB_MEM_START_512M                     			(__MM_RESERVED_ADDR2_512M)
#define __MM_FB_MEM_END_512M                       			(__MM_FB_MEM_START_512M + FB0_MEM_SIZE)

/* FB3
*/
#define __MM_FB3_MEM_START_512M                     			((__MM_FB_MEM_END_512M + 0xFFF)\
															& 0xFFFFF000)
#define __MM_FB3_MEM_END_512M                       			(__MM_FB3_MEM_START_512M + FB3_MEM_SIZE)

/* MALI
*/
#define __MM_MALI_MEM_START_512M						((__MM_FB3_MEM_END_512M + 0xFF)\
															& 0xFFFFFF00)
#define __MM_MALI_MEM_END_512M						(__MM_MALI_MEM_START_512M + MALI_DEDICATED_MEM_SIZE)

/* MALI UMP
*/
#define __MM_MALI_UMP_MEM_START_512M				((__MM_MALI_MEM_END_512M + 0xFF)\
															& 0xFFFFFF00)
#define __MM_MALI_UMP_MEM_END_512M					(__MM_MALI_UMP_MEM_START_512M + MALI_UMP_MEM_SIZE)

/* TOP and Bottom2
*/
#define __MM_ALI_PRIVATE_MEM_START2_512M			(__MM_FB_MEM_START_512M)	
#define __MM_ALI_PRIVATE_MEM_END2_512M				(__MM_MALI_UMP_MEM_END_512M)

/* define the memory address end */
/***************************************************************************/


/*<==================== DRAM MEMORY MAP FOR M3921 ALI PRIVATE USAGE END   ====================>*/



#define SET_DWORD(addr, d)              (*(volatile unsigned long *)(addr)) = (d)
#define GET_DWORD(addr)                 (*(volatile unsigned long *)(addr))

//#define DRAM_SPLIT_CTRL_BASE  (0x18041000)
#define DRAM_SPLIT_CTRL_BASE  (0xb8041000)
#define PVT_S_ADDR 0x10
#define PVT_E_ADDR 0x14
#define SHR_S_ADDR 0x18
#define SHR_E_ADDR 0x1c


void install_memory(void)
{
    unsigned long priv_mem_base_addr;
    unsigned long priv_mem_len;
    unsigned long share_mem_base_addr; 	
    unsigned long share_mem_len;			
    unsigned long arg_base;
    void *pbuf;

	unsigned long __G_MM_SHARED_MEM_START_ADDR = (unsigned long)0x85fff000;
	unsigned long __G_RPC_MM_LEN = (unsigned long)0x1000;

	unsigned long __G_MM_MCAPI_MEM_START_ADDR = (unsigned long)0x85FFD000;	
	unsigned long __G_MM_MCAPI_MEM_LEN = (unsigned long)0x2000;

    priv_mem_base_addr = (unsigned long)0xa4000000; 
	priv_mem_len = (unsigned long)0x85FFD000 - 0x84000000;   

    #if 1
	share_mem_base_addr = (unsigned long)(__G_MM_MCAPI_MEM_START_ADDR+0x200);
    share_mem_len = 0x400; 
    #else
    share_mem_base_addr = (unsigned long)0x85dfe000;
    share_mem_len = 0x1000;
    #endif

    //printf("priv_mem_base_addr:0x%x, priv_mem_len:0x%x, share_mem_base_addr:0x%x, __G_MM_MCAPI_MEM_START_ADDR:0x%x, __G_MM_MCAPI_MEM_LEN:0x%x\n", priv_mem_base_addr, priv_mem_len, share_mem_base_addr, __G_MM_MCAPI_MEM_START_ADDR, __G_MM_MCAPI_MEM_LEN);
    // Store MCAPI share memory start addr at __SEE_RAM_BASE -24
    // Store MCAPI share memory end addr at __SEE_RAM_BASE -20 
    // Store private memory start addr at   __SEE_RAM_BASE -16
    // Store private memory end addr at     __SEE_RAM_BASE -12 
    // Store share memory start addr at __SEE_RAM_BASE - 8 
    // Store share memory end addr at   __SEE_RAM_BASE - 4 
    arg_base = (priv_mem_base_addr+0x200 -16);

#ifdef CONFIG_ALI_MINIRPC
    /*Added by Kinson for minirpc share mem*/
	SET_DWORD((arg_base-12), (0x87000000&0x1fffffff)); //uboot notify the minirpc share mm addr to see
#endif	

    /*Added by tony*/
    SET_DWORD((arg_base-8), (__G_MM_MCAPI_MEM_START_ADDR&0x1fffffff));
    SET_DWORD((arg_base-4), ((__G_MM_MCAPI_MEM_START_ADDR&0x1fffffff)+__G_MM_MCAPI_MEM_LEN));
    pbuf = (u32 *)(__G_MM_MCAPI_MEM_START_ADDR);
    //printf("^^^^^^^^^^^^^^^^^MAIN pbuf:0x%x\n", pbuf);
    memset(pbuf, 0, __G_MM_MCAPI_MEM_LEN);
    /*Added end*/
    SET_DWORD(arg_base, (priv_mem_base_addr&0x1fffffff));   
    SET_DWORD((arg_base+4), ((priv_mem_base_addr&0x1fffffff)+priv_mem_len));    
    SET_DWORD((arg_base+8), (share_mem_base_addr&0x1fffffff));    
    SET_DWORD((arg_base+12), ((share_mem_base_addr&0x1fffffff)+share_mem_len));  
    if(GET_DWORD(DRAM_SPLIT_CTRL_BASE+PVT_S_ADDR) == 0)
    {
        #if 0
    	share_mem_base_addr =  __G_MM_SHARED_MEM_START_ADDR;
    	share_mem_len = __G_RPC_MM_LEN + __G_MM_MCAPI_MEM_LEN;    
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+PVT_S_ADDR, priv_mem_base_addr&0x1fffffff);	
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+PVT_E_ADDR, ((priv_mem_base_addr&0x1fffffff)+priv_mem_len));    
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+SHR_S_ADDR, share_mem_base_addr&0x1fffffff);    
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+SHR_E_ADDR, ((share_mem_base_addr&0x1fffffff)+share_mem_len));
        #else
    	//share_mem_base_addr =  __G_MM_SHARED_MEM_START_ADDR;
    	//share_mem_len = __G_RPC_MM_LEN + __G_MM_MCAPI_MEM_LEN;    
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+PVT_S_ADDR, priv_mem_base_addr&0x1fffffff);	
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+PVT_E_ADDR, ((priv_mem_base_addr&0x1fffffff)+priv_mem_len));    
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+SHR_S_ADDR, __G_MM_MCAPI_MEM_START_ADDR&0x1fffffff);    
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+SHR_E_ADDR, ((__G_MM_MCAPI_MEM_START_ADDR&0x1fffffff)+share_mem_len));	

        #endif
    }
    printf("\n/**************RPC addr as follow:*****************/\n");
	printf("priv_mem_base_addr:0x%x\n",priv_mem_base_addr);
	printf("priv_mem_end_addr:0x%x\n", priv_mem_base_addr + priv_mem_len);
	printf("share_mem_base_addr:0x%x\n",share_mem_base_addr);
	printf("share_mem_end_addr:0x%x\n",share_mem_base_addr + share_mem_len);
	printf("__G_ALI_MM_MCAPI_MEM_START_ADDR:0x%x\n", __G_MM_MCAPI_MEM_START_ADDR);
	printf("__G_MM_MCAPI_MEM_END_ADDR:0x%x\n",__G_MM_MCAPI_MEM_START_ADDR + __G_MM_MCAPI_MEM_LEN);
    printf("/***************************************************/\n\n");

		
}

#define __G_MM_PRIVATE_AREA_START_ADDR  (__MM_SEE_PRIVATE_START)

#define SEE_RUN_ADDR (__G_MM_PRIVATE_AREA_START_ADDR + 0x200)

void  g_see_boot_arm(void)
{
    u32 i=0;
	u32 addr = SEE_RUN_ADDR;

	addr = (addr & 0x0FFFFFFF) | 0xA0000000;
	printf("[%s]--%d  addr=0x%08x\n",__func__,__LINE__,addr);
	*(volatile unsigned long *)(0x18000220) &= ~0x2;
	*(volatile unsigned long *)(0x18000200) = addr;
	//mdelay(1);
	for(i=0;i<900000;i++)
	{;}
	*(volatile unsigned long *)(0x18000220) |= 0x2; 
}

void  g_see_boot(void)
{
    u32 i=0;
	u32 addr = SEE_RUN_ADDR;

	addr = (addr & 0x0FFFFFFF) | 0xA0000000;
	printf("[%s]--%d  addr=0x%08x\n",__func__,__LINE__,addr);
	*(volatile unsigned long *)(0xb8000220) &= ~0x2;
	*(volatile unsigned long *)(0xb8000200) = addr;
	//mdelay(1);
	for(i=0;i<900000;i++)
	{;}
	*(volatile unsigned long *)(0xb8000220) |= 0x2; 
}


#ifdef CONFIG_ALI_MINIRPC
/*
//wait see prepare for minirpc
*/
void wait_see_minirpc_prepared()
{
  unsigned long arg_base;
  unsigned long priv_mem_base_addr;
  
  priv_mem_base_addr = (unsigned long)0x84000000; 
  arg_base = (priv_mem_base_addr+0x200 -16);

  printf("now main cpu wait to see, because see may not has prepare for minirpc\n");
  	
  while(1)
  {
     if(0x30317a8b == GET_DWORD(arg_base-16)) //tds will write 0x50517a8b to this addr
     {
        printf("now see has prepared, main can go to follow!\n");
        break;
     }
	 mdelay(30);
  }
  
}
#endif


