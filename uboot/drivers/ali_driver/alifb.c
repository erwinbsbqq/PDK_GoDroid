/*
 * uboot get GMA Lay1 mem buffer for OSD usage
 *
 */

#include <common.h>

#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <malloc.h>
#include <video_fb.h>
#include <alidefinition/adf_boot.h>

#define DEBUG

#ifdef DEBUG
#define DPRINT(x...) printf(x)
#else
#define DPRINT(x...) do{}while(0)
#endif

static GraphicDevice alifb;
static volatile ADF_BOOT_BOARD_INFO *info= NULL;

static void osd_see_boot()
{	
	int entry;

	memset(&(info->gma_info),0,sizeof(info->gma_info));
	
	info->gma_info.gma_enable = 1;
	info->gma_info.gma_layer_id = 1;//GMA0;GMA1
	info->gma_info.format = 0x0C;//GMA_FORMAT_CLUT8;see adf_gma.h
	info->gma_info.x = 0;
	info->gma_info.y = 0;
	info->gma_info.w = 1280;
	info->gma_info.h = 720;
	info->gma_info.pallett[0] = 0x0;
	info->gma_info.pallett[1] = 0xff0000ff;
	info->gma_info.pallett[2] = 0xffff0000;
	info->gma_info.pallett[3] = 0xff00ff00;
	info->gma_info.gma_buffer = NULL;

	set_mtdparts();
	set_mac();
	set_boardinfo(0);
	
	//step1:first load see and boot, to speed up show logo time;
	entry = (void *)(info->memmap_info.see_start |0x80000000)  - 64;//64byte ubo header size

	DPRINT("\nnew Start to load partition '%s' to 0x%x....\n", PART_NAME_SEE,entry);

	if (load_part_ubo(PART_NAME_SEE, entry)<0)
	{
		DPRINT("Load see code Fail...\n");
		return -1;
	}	
	DPRINT("Load see code Done...\n");
	
	if (part_ubo_check(entry)<0)
	{
		DPRINT("Check see code Fail...\n");
		return -1;
	}		

	install_memory();
	DPRINT("install_memory in uboot\n");
	g_see_boot();
	DPRINT("g_see_boot in uboot\n");	

}
	
void *video_hw_init(void)
{
	GraphicDevice *pGD = (GraphicDevice *) & alifb;
	info = (ADF_BOOT_BOARD_INFO *)(PRE_DEFINED_ADF_BOOT_START);

	osd_see_boot();//we need boot see to get gma buffer
	
	DPRINT("video hw init ali fb\n");

	/*gma buffer should start with 0x8xxxxxxx or 0xAxxxxxxx*/
 	while(info->gma_info.gma_buffer== NULL)
	{
		mdelay(100);
		DPRINT("buffer 0x%08x\n",info->gma_info.gma_buffer);
	//how to check valid buffer?
	}
	
	DPRINT("gma buffer 0x%08x value 0x%08x\n",&info->gma_info.gma_buffer,info->gma_info.gma_buffer);

	pGD->frameAdrs = info->gma_info.gma_buffer;
	pGD->memSize = info->gma_info.gma_pitch*info->gma_info.h;
	pGD->gdfBytesPP = info->gma_info.gma_pitch/info->gma_info.w;
	if(pGD->gdfBytesPP == 1)
	{
		pGD->gdfIndex = GDF__8BIT_INDEX;
	}
	else
		{
			pGD->gdfIndex = GDF_32BIT_X888RGB;
		}
	pGD->winSizeX = info->gma_info.w;
	pGD->winSizeY = info->gma_info.h;
	pGD->plnSizeX = info->gma_info.w;
	pGD->plnSizeY = info->gma_info.h;

	DPRINT("0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x\n",
		pGD->frameAdrs,pGD->memSize,pGD->gdfBytesPP,pGD->gdfIndex,pGD->winSizeX,pGD->winSizeY);
		
	return ((void *) pGD);

}

void video_set_lut(unsigned int index,	/* color number */
		    unsigned char r,	/* red */
		    unsigned char g,	/* green */
		    unsigned char b	/* blue */
		    )
{
	return;
}

