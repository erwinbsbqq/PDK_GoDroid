/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: shm_comm.c
 *
 *  Description: shm_comm device low level implementation file.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.02.01      David.L         0.1.000         Initial
 *  2.  2013/05/25      Tony.Zh         0.2.000         Bug fix 
 ****************************************************************************/

#include "mcomm.h"
#include "mcomm_dev.h"
#include "shm_comm_hw.h"
#include "ali_shm.h"

#include "board_config.h"
#include "ali_board_config.h"
#include "ali_rpc_debug.h"

#define u32 unsigned int
/**
 * David - 
 * This file simulates Marvell's arch/arm/mach-feroceon-mv78xx0, implementing
 * the infrastructure mailbox hardware context exchanging. Therefore, this
 * module notifies and acks the handshake companying with the mailbox context,
 * which is different from the original.
 */

static struct ali_mcomm_struct {
	unsigned int localId;
	unsigned int remoteId;
    unsigned int syncflag;
    MBX_HAL      mbx;
} ali_mcomm_data;


static void mcomm_ali_send(u32 core_id, u32 ctxt)
{
    unsigned long flags;
	unsigned long i=0;
	unsigned long timeout=500;

    switch (core_id) 
	{
        case SEE_CPU_ID:
			
			if(1 != ali_mcomm_data.syncflag)
			{  
			    Log(LOG_ERR,"%s mcomm_ali_send error because not sync\n",__func__);
			}
			
			Log(LOG_DEBUG,"%s MBX0 ctxt:0x%08x\n",__func__,ctxt);
			
			ali_mcomm_data.syncflag = 0;
			
	    	ali_mcomm_data.mbx.tx(MBX0, ctxt);
			
		    for(i=0;i<timeout;i++)
		    {
		       if(1 == ali_mcomm_data.syncflag)
			   {
			   	  break;
		       }
		       mdelay(2);
		    }
	        break;
       default:
            break;
    }
}

static void mcomm_ali_ack(u32 core_id, u32 ctxt)
{
    unsigned long flags;

    switch (core_id) 
	{
    	case SEE_CPU_ID:
			Log(LOG_DEBUG,"%s MBX1 ctxt:0x%08x\n",__func__,ctxt);
        	ali_mcomm_data.mbx.tx(MBX1, ctxt);
        	break;
   		default:
            break;
    }
}

/**
 * This function called in a irq routine, therefor have to use spin_lock_irqsave
 * and spin_unlock_irqrestore, since there is no way to wake up a tasklet with
 * another parameter.
 */
static u32 mcomm_ali_recv(u32 mbx_index)
{
    u32 ctxt;
    unsigned long flags;
    ctxt = ali_mcomm_data.mbx.rx(mbx_index);
    return ctxt;
}

static void mcomm_ali_sync(void)
{
    ali_mcomm_data.syncflag = 1;
}


static unsigned long mcomm_ali_cpuid(u32 remote)
{
    if (remote)
        return ali_mcomm_data.remoteId;
	return ali_mcomm_data.localId;
}

static struct mcomm_platform_ops mcomm_ali_ops = {
	.send = mcomm_ali_send,
    .ack = mcomm_ali_ack,
	.recv = mcomm_ali_recv,
    .sync = mcomm_ali_sync,
	.cpuid = mcomm_ali_cpuid,
};


static void mcomm_ali_hwinit(void)
{
    ali_mcomm_data.mbx.init();
}

void rpc_ali_modinit(void);


static void mcomm_ali_hwuninit(void)
{
    ali_mcomm_data.mbx.deinit();
}

static int mcomm_remove(struct platform_device *pdev)
{
	return 0;
}


extern unsigned long __G_MM_PRIVATE_AREA_TOP_ADDR;
extern unsigned long __G_MM_PRIVATE_AREA_START_ADDR;
extern unsigned long __G_MM_SHARED_MEM_TOP_ADDR;
extern unsigned long __G_MM_SHARED_MEM_START_ADDR;
extern unsigned long __G_RPC_MM_LEN;
extern unsigned long __G_MM_MCAPI_MEM_LEN;
extern unsigned long __G_MM_MCAPI_MEM_START_ADDR;

#define SEE_RUN_ADDR (__G_MM_PRIVATE_AREA_START_ADDR + 0x200)


#define DRAM_SPLIT_CTRL_BASE  __REGALIRAW(0x18041000)
#define PVT_S_ADDR 0x10
#define PVT_E_ADDR 0x14
#define SHR_S_ADDR 0x18
#define SHR_E_ADDR 0x1c

#define SET_DWORD(addr, d)         (*(volatile u32 *)(addr)) = (d)
#define GET_DWORD(addr)            (*(volatile u32 *)(addr))


void ali_driver_param_setting(void)
{
    /*<==================== BOARD IDENTIFICATION START ====================>*/

    #ifdef CONFIG_ALI_CHIP_M3921
    __G_ALI_BOARD_TYPE = 0x00004003;
    #endif

    /* Need to confirm wiht kernel configuration.
	*/
	#if 0
    __G_ALI_CHIP_TYPE;
    __G_ALI_CHIP_VERSION; 
    __G_ALI_BOARD_VERSION;
    #endif
    /*<==================== BOARD IDENTIFICATION END ====================>*/
	
   
    /*<==================== DRVIER MEMORY MAP VARIABLES START ====================>*/
	/* Anchor postions.
	*/
    /* 1, MCAPI
    */  
    __G_MM_MCAPI_MEM_LEN =                (__MM_MCAPI_MEM_END -__MM_MCAPI_MEM_START);
    __G_MM_MCAPI_MEM_START_ADDR =         (unsigned long)(__MM_MCAPI_MEM_START);

	/* 2, Boot LOGO
	*/
    __G_MM_BOOTLOGO_DATA_START_ADDR =     (unsigned long)(__MM_DRAM_BOOTLOGO_DATA_START);	

	/* 3, User configuration.
	*/
	/* Nothing to do because this variable is refererenced by no where.
	 * need following the rule where it is used?
	*/

    /* 4, SEE PRIVATE MEM
    */
    __G_MM_PRIVATE_AREA_TOP_ADDR =        (unsigned long)(__MM_DRAM_SEE_PRIVATE_END);
    __G_MM_PRIVATE_AREA_START_ADDR =      (unsigned long)(__MM_DRAM_SEE_PRIVATE_START);	

    /* 5, SEE MAIN SHARED MEM
    */
    __G_MM_SHARED_MEM_TOP_ADDR =          (unsigned long)(__MM_DRAM_SEE_MAIN_SHARED_MEM_END);
    __G_MM_SHARED_MEM_START_ADDR =        (unsigned long)(__MM_DRAM_SEE_MAIN_SHARED_MEM_START);
    __G_RPC_MM_LEN =                      (__MM_DRAM_SEE_MAIN_SHARED_MEM_END - __MM_DRAM_SEE_MAIN_SHARED_MEM_START);	
	
	/* Movable postions.
	*/	
    /* 1, FRAME BUFFER
    */ 
    __G_MM_FB_SIZE =                      (FB_MEM_SIZE);
    __G_GE_CMD_SIZE =                     0;
    g_fb_max_width =                      (FB_WIDTH);
    g_fb_max_height =                     (FB_HEIGHT);
    g_fb_pitch =                          (FB_PITCH);
    g_fb_bpp =                            (FB_BPP);
    g_support_standard_fb =               1;
    g_fb3_max_width =                     (FB3_WIDTH);
    g_fb3_max_height =                    (FB3_HEIGHT);
    g_fb3_pitch =                         (FB3_PITCH);
    g_fb3_bpp =                           (FB3_BPP);
    __G_MM_FB_START_ADDR =                (unsigned long)(__MM_DRAM_FB_MEM_START);

    /* 2, TSG
    */
    __G_MM_TSG_BUF_LEN =                  (__MM_DRAM_TSG_BUF_END - __MM_DRAM_TSG_BUF_START);
    __G_MM_TSG_BUF_START_ADDR =           (unsigned long)(__MM_DRAM_TSG_BUF_START);	

    /* 3, DMX
    */
    __G_MM_DMX_MEM_TOP_ADDR =             (unsigned long)(__MM_DRAM_DMX_BUF_END);
    __G_MM_DMX_MEM_START_ADDR =           (unsigned long)(__MM_DRAM_DMX_BUF_START);

    /* 4, Image decoder
    */
    __G_MM_IMAGE_DECODER_MEM_LEN =        (__MM_DRAM_IMAGE_DECODER_MEM_END - __MM_DRAM_IMAGE_DECODER_MEM_START);
    __G_MM_IMAGE_DECODER_MEM_START_ADDR = (unsigned long)(__MM_DRAM_IMAGE_DECODER_MEM_START);	

    /* 5, MEDIA PLAYER
    */
    __G_MM_MP_MEM_TOP_ADDR =              (unsigned long)(__MM_DRAM_MP_MEM_END);
    __G_MM_MP_MEM_START_ADDR =            (unsigned long)(__MM_DRAM_MP_MEM_START);	

    /* 6, VDEC_LAF_FLAG
    */
    __G_MM_VDEC_LAF_FLAG_BUF_ADDR =       (unsigned long)(__MM_DRAM_VDEC_LAF_FLAG_BUF_START);	

    /* 7, VDEC CMDQ
    */
    __G_MM_VDEC_CMD_QUEUE_ADDR =          (unsigned long)(__MM_DRAM_VDEC_CMD_QUEUE_START);	

    /* 8, VBV buffer
    */
    __G_MM_VDEC_VBV_LEN =                 (__MM_DRAM_VBV_BUF_END - __MM_DRAM_VBV_BUF_START);
    __G_MM_VDEC_VBV_START_ADDR =          (unsigned long)(__MM_DRAM_VBV_BUF_START);	

    /* 9, STILL FRAME
    */
    __G_MM_STILL_FRAME_SIZE =             (__MM_DRAM_STILL_FRAME_END - __MM_DRAM_STILL_FRAME_START);
    __G_MM_STILL_FRAME_ADDR =             (unsigned long)(__MM_DRAM_STILL_FRAME_START);	

    /* 10, VCAP
    */
    __G_MM_VCAP_FB_SIZE =                 (__MM_DRAM_VCAP_FB_END - __MM_DRAM_VCAP_FB_START);
    __G_MM_VCAP_FB_ADDR =                 (unsigned long)(__MM_DRAM_VCAP_FB_START);	
    	
    /* 11, VIDEO
    */
    __G_MM_VIDEO_TOP_ADDR =               (unsigned long)(__MM_DRAM_VIDEO_BUF_END);
    __G_MM_VIDEO_START_ADDR =             (unsigned long)(__MM_DRAM_VIDEO_BUF_START);
    
    /* 12, NIM J83B
    */
     __G_MM_NIM_J83B_MEM_LEN =            0;
     __G_MM_NIM_J83B_MEM_START_ADDR =     0;
    /*<==================== DRVIER MEMORY MAP VARIABLES END ====================>*/
	
	return;
}


#if 0
static void install_memory(void)
{

    u32 priv_mem_base_addr;
    u32 priv_mem_len;
    u32 share_mem_base_addr; 	
    u32 share_mem_len;			
    u32 arg_base;
    void *pbuf;

    /*init all param setting*/
	ali_driver_param_setting();  

    priv_mem_base_addr = __G_MM_PRIVATE_AREA_START_ADDR;
    priv_mem_len = __G_MM_PRIVATE_AREA_TOP_ADDR - __G_MM_PRIVATE_AREA_START_ADDR;
    share_mem_base_addr =  __G_MM_SHARED_MEM_START_ADDR;
    share_mem_len = __G_RPC_MM_LEN;

    printf("priv_mem_base_addr:0x%x, priv_mem_len:0x%x, share_mem_base_addr:0x%x, __G_MM_MCAPI_MEM_START_ADDR:0x%x, __G_MM_MCAPI_MEM_LEN:0x%x\n", priv_mem_base_addr, priv_mem_len, share_mem_base_addr, __G_MM_MCAPI_MEM_START_ADDR, __G_MM_MCAPI_MEM_LEN);

	/*
        Store MCAPI share memory start addr at __SEE_RAM_BASE -24
        Store MCAPI share memory end addr at __SEE_RAM_BASE -20 
        Store private memory start addr at   __SEE_RAM_BASE -16
        Store private memory end addr at     __SEE_RAM_BASE -12 
        Store share memory start addr at __SEE_RAM_BASE - 8 
        Store share memory end addr at   __SEE_RAM_BASE - 4 
        */
     arg_base = priv_mem_base_addr+0x200 -16;

    SET_DWORD((arg_base-8), (__G_MM_MCAPI_MEM_START_ADDR&0x1fffffff));
    SET_DWORD((arg_base-4), ((__G_MM_MCAPI_MEM_START_ADDR&0x1fffffff)+__G_MM_MCAPI_MEM_LEN));
    pbuf = (u32 *)(__G_MM_MCAPI_MEM_START_ADDR);
    printf("^^^^^^^^^^^^^^^^^MAIN pbuf:0x%x\n", pbuf);
    memset(pbuf, 0, __G_MM_MCAPI_MEM_LEN);

    SET_DWORD(arg_base, (priv_mem_base_addr&0x1fffffff));   
    SET_DWORD((arg_base+4), ((priv_mem_base_addr&0x1fffffff)+priv_mem_len));    
    SET_DWORD((arg_base+8), (share_mem_base_addr&0x1fffffff));    
    SET_DWORD((arg_base+12), ((share_mem_base_addr&0x1fffffff)+share_mem_len));  

	if(GET_DWORD(DRAM_SPLIT_CTRL_BASE+PVT_S_ADDR) == 0)
    {
    	 share_mem_base_addr =  __G_MM_SHARED_MEM_START_ADDR;
    	 share_mem_len = __G_RPC_MM_LEN + __G_MM_MCAPI_MEM_LEN;    
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+PVT_S_ADDR, priv_mem_base_addr&0x1fffffff);	
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+PVT_E_ADDR, ((priv_mem_base_addr&0x1fffffff)+priv_mem_len));    
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+SHR_S_ADDR, share_mem_base_addr&0x1fffffff);    
        SET_DWORD(DRAM_SPLIT_CTRL_BASE+SHR_E_ADDR, ((share_mem_base_addr&0x1fffffff)+share_mem_len));	
    }	
}


void g_see_boot()
{	
    u32 i=0;
	u32 addr = SEE_RUN_ADDR;
	addr = (addr & 0x0FFFFFFF) | 0x80000000;
	printf("addr=0x%08x\n",addr);
	*(volatile unsigned long *)(0x18000220) &= ~0x2;
	*(volatile unsigned long *)(0x18000200) = addr;
	//mdelay(1);
	for(i=0;i<900000;i++)
	{;}
	*(volatile unsigned long *)(0x18000220) |= 0x2; 


}
#endif


#if defined(__MCAPI_TEST__)

#include "pr.h"

void MCAPI_Hi(void *arg);
//static Thread mcapi_task_id;

#endif

void stop_watchdog();
extern int ali_rpc_init_mutex(void);


/*MiniRPC*/
int mcomm_ali_modinit(void)
{
	int rc;

    printf("\n***MiniRPC main cpu mcomm_ali_modinit entered!***\n");

    /*Init the ali_mcomm_data*/
    ali_mcomm_data.localId = MAIN_CPU_ID;
    ali_mcomm_data.remoteId = SEE_CPU_ID;
    ali_mcomm_data.mbx.init = MbxHwMain_Init;
    ali_mcomm_data.mbx.deinit = MbxHwMain_Deinit;
    ali_mcomm_data.mbx.tx = MbxHwMain_Tx;
    ali_mcomm_data.mbx.rx = MbxHwMain_Rx;
	ali_mcomm_data.syncflag = 1;

    /*setting the share memory param*/
    ///install_memory();
	
	/*statup the see code*/
    ///printf("MAIN, try to bootup the see!\n");
    ///g_see_boot();  
    ///printf("MAIN, bootup the see completed!\n");

    /*init the _mcomm_devdata and mcomm_platform_ops */
	mcomm_init(&mcomm_ali_ops, 0);

	mcomm_ali_hwinit();

	MiniRpcInit();
	
	return rc;
}

static void mcomm_ali_modexit(void)
{
	//mcomm_pdev_release();
	//platform_driver_unregister(&mcomm_driver);
	//mcomm_exit();
}



