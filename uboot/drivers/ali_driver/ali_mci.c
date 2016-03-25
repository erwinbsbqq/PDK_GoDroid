/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2013 Copyright (C)
 *
 *  File: alimci.c
 *
 *  Description: SD/MMC Card Host driver for ALI STB platform
 *
 *  History:
 *          Date                    Author          Comment
 *          ====                    ======          =======
 * 1.       2013.11.04               David Chen   porting from S3921 Linux SD/MMC driver
 ****************************************************************************/

#include <common.h>
#include <mmc.h>
#include <asm/io.h>		/* for virt_to_phys() prototype */
#include <malloc.h>		/* for free() prototype */
#include <asm/errno.h>
#include <ali_gpio.h>
#include "ali_mci.h"

#define STA_INF_BUF_SZ  1500 
#define MAX_WAIT_TIMES 20000
#define NORMAL_OCR_FOR_EMMC 0x40FF8080

#define ALI_SD_DRV_VER "SD02_U20131214"

#define SDIO_DBG

#ifndef SDIO_DBG
//#define alimci_log(format, ...) do{}while(0)
#define alimci_log
#endif

#define alimci_err(format, args...)		printf("<%s> line: %d: "format,  __FUNCTION__, __LINE__, ##args)

static ALI_SDIO_MEDIATYPE cur_media_type = ALI_SDIO_MT_UNKNOWN;

static u16 alimci_get_clk(void);
static int alimci_card_present(struct mmc *mmc);
static int alimci_status_polling(struct alimci_host *host, struct mmc_cmd *cmd);
static void alimci_pinmux_set(struct alimci_host *host, int callline);
static void alimci_pinmux_restore(struct alimci_host *host, int callline);

static ALI_SDIO_MEDIATYPE alimci_get_media_type(void)
{
	return cur_media_type;
}

static const unsigned char SDIO_MediaType[][20]=
{
    "Unknown",
    "SD 2.0+",
    "SD 1.10-",  //don't support SD_SEND_IF_COND
    "MMC",
    "eMMC"
};

static void alimci_get_status(struct alimci_host *host, char *buf)
{
    alimci_pinmux_set(host,__LINE__);
    sprintf(buf, "SoC Base addr: 0x%08x | SoC ChipID:  0x%08x | SDIO Base addr: 0x%08x | DetectDelay: %d ms\n",
             p_host->soc_base,
              (u32)ALI_SOC_RD32(SOC_CHIP_VER),
            p_host->sdio_base,
             host->detect_delay_ms);
#if defined(CONFIG_ARM)
    sprintf(buf, "%sDriver Ver:%s | SoC PinMux1: 0x%08x (0x%08x) | PinMux2: 0x%08x (0x%08x)\n",
             buf,
             ALI_SD_DRV_VER,
             (u32)ALI_SOC_RD32(SOC_PINMUX_REG1),
             host->pinmux1,
             (u32)ALI_SOC_RD32(SOC_PINMUX_REG2),
             host->pinmux2
             );
#elif defined(CONFIG_MIPS)
    sprintf(buf, "%sDriver Ver:%s | SoC PinMux1: 0x%08x (0x%08x) | PinMux2: 0x%08x (0x%08x) | SoC PinMux3: 0x%08x (0x%08x) | PinMux4: 0x%08x (0x%08x)\n",
             buf,
             ALI_SD_DRV_VER,
             (u32)ALI_SOC_RD32(SOC_PINMUX_REG1),
             host->pinmux1,
             (u32)ALI_SOC_RD32(SOC_PINMUX_REG2),
             host->pinmux2,
             (u32)ALI_SOC_RD32(SOC_PINMUX_REG3),
             host->pinmux3,
             (u32)ALI_SOC_RD32(SOC_PINMUX_REG4),
             host->pinmux4
             );
#endif
    sprintf(buf, "%sGPIO CDN: %d%s | WP: %d%s | PWR: %d%s\n",
             buf,
             host->gpio_cd,
             host->gpio_cd_invert ? "<INVERT>":"",
             host->gpio_card_ro,
             host->gpio_card_ro_invert ? "<INVERT>":"",
             host->gpio_power,
             host->gpio_power_invert ? "<INVERT>":""
             );

    sprintf(buf, "%sCard Inserted: %s | Card WP: %s | GPIO_Power: %d | MediaType: %s\n",
             buf,
             alimci_card_present(host->base_mmc) ? "Yes" : "No",
             "NA",
             gpio_get_value(host->gpio_power),
             SDIO_MediaType[alimci_get_media_type()] 
             );

    sprintf(buf, "%sDMA RW: %s | Cur BitMode: %s | InitClkDiv: 0x%04x | TranClkDiv: 0x%04x | CurClkDiv: 0x%04x\n",
             buf,
             host->usedma ? "Yes":"No",
             MMC_BUS_WIDTH_8 == host->bus_width ? "8 Bits" : (MMC_BUS_WIDTH_4 == host->bus_width ? "4 Bit":"1 Bit"),
             host->InitClkDiv,
             host->TranClkDiv,
             alimci_get_clk()
             );
    sprintf(buf, "%s------------------------------------------------------------------------------------\n",buf);
    sprintf(buf, "%sSD Control:    0x%08x | Argument:        0x%08x  (Last CMD:%d  Status: 0x%x)\n",
             buf,
             (u32)ALI_MCI_RD32(SD_CTRL_REG),
             (u32)ALI_MCI_RD32(SD_ARG_REG),
             (u8)(ALI_MCI_RD8(SD_CMD_REG) & 0x3f),
             (u8)ALI_MCI_RD8(SD_STATUS_REG)
             );
    sprintf(buf, "%sSD BlkCtrl:    0x%08x | PIO Ctrl:        0x%08x\n",
             buf,
             (u32)ALI_MCI_RD32(SD_BLKLEN_REG),
             (u32)ALI_MCI_RD32(SD_PIO_DATA));

    sprintf(buf, "%sDMA RD Addr:   0x%08x | DMA WR Addr:     0x%08x\n",
             buf,
             (u32)ALI_MCI_RD32(SD_DMA_R_ADDR),
             (u32)ALI_MCI_RD32(SD_DMA_W_ADDR));

    sprintf(buf, "%sDMA RD Size:   0x%08x | DMA WR Size:     0x%08x\n",
             buf,
             (u32)ALI_MCI_RD32(SD_DMA_R_SIZE),
             (u32)ALI_MCI_RD32(SD_DMA_W_SIZE));

    sprintf(buf, "%sDMA Ctrl:      0x%08x | CLK DIV High:    0x%08x\n",
             buf,
             (u32)ALI_MCI_RD32(SD_DMA_CTRL),
             (u32)ALI_MCI_RD32(SD_CLKDIV_H_REG));

    sprintf(buf, "%sClk Delay:     0x%08x | Curr Addr:       0x%08x\n",
             buf,
             (u32)ALI_MCI_RD32(SD_CLK_DLY_REG),
             (u32)ALI_MCI_RD32(SD_CUR_ADDR));

    sprintf(buf, "%sCurr Blk Info: 0x%08x | IP version:      0x%08x\n\n",
             buf,
             (u32)ALI_MCI_RD32(SD_CUR_BLK_IFO),
             (u32)ALI_MCI_RD32(SD_IP_VER));
    sprintf(buf, "%s------------------------------------------------------------------------------------\n",buf);
    sprintf(buf, "%sPart Conf(179):    0x%02x | Part No.:        0x%02x\n",
             buf,
             host->base_mmc->part_config,  //MMCPART_NOAVAILABLE = 0xFF
             host->base_mmc->part_num
             );
    alimci_pinmux_restore(host,__LINE__);
}

#ifdef SDIO_DBG
//on u-boot command line, use "setenv SdDbgOut On" to open debug log message.

static int alimci_log(const char *fmt, ...)
{
	va_list args;
	uint i;
	char printbuffer[STA_INF_BUF_SZ];
	bool       DbgOut = FALSE;
	char *s = getenv("SdDbgOut");

	if (s != NULL)
	{
		if (0 == strcmp(s,"on") || 0 == strcmp(s,"On")  || 0 == strcmp(s,"ON"))
			DbgOut = TRUE;
	}

    if (DbgOut)
    {
	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, sizeof(printbuffer), fmt, args);
	va_end(args);

	/* Print the string */
	puts(printbuffer);
    }
	return i;
}

static void alimci_dump_status(struct alimci_host *host, const char *callfunc, const unsigned int callline)
{
    char buf[STA_INF_BUF_SZ];

     //dump_stack();
    alimci_log( "\n\n=========== STATUS DUMP START @ Line: %d of <%s>=======\n",callline,callfunc);
    alimci_get_status(host,buf);
    alimci_log(buf);
    alimci_log( "=================================================\n\n");
}

static void alimci_dumphex(const unsigned char *buff, const int len, const char *callfunc, const unsigned int callline)
{
	int i,j;
	unsigned char t;

	printf("---------Read data %d bytes to 0x%08x----------\n",len,buff);
	for(i=0;i<len/16;i++)
	{
	    for(j=0;j<16;j++)
	    {
//	            t = buff[i*16+j];
	            t = *(volatile uint8_t *)(buff+i*16+j);
	            printf("%02X ", t);
	    }
	    printf(" ");
	    for(j=0;j<16;j++)
	    {
	            t = buff[i*16+j];
	            if (t>31 && t < 128)
	                printf("%c", t);
	            else
	                printf(".");
	    }
	    printf("\n");
	}
}
#endif

static void get_chip_info(u16 *id, u8 *package, u8 *version)
{
    u32 ALi_Chip_Version = ALI_SOC_RD32(SOC_CHIP_VER);
    *id = ALi_Chip_Version >> 16;
    *package = (u8)(ALi_Chip_Version >> 8) & 0x0F;
    *version = (u8) ALi_Chip_Version & 0xFF;
}

static int alimci_dma_start(u32 dir)
{
    ALI_MCI_WR8(SD_DMA_CTRL, (ALI_MCI_RD8(SD_DMA_CTRL) & 0xfd) | dir);
    ALI_MCI_WR8(SD_DMA_CTRL, ALI_MCI_RD8(SD_DMA_CTRL) | DMA_START);
    return 0;
}

static int alimci_set_block_size(u32 block_size)
{
    unsigned short size = (block_size & 0x0000ffff);

    ALI_MCI_WR8(SD_BLKLEN_REG + 1, (size >> 8) & 0xff);
    ALI_MCI_WR8(SD_BLKLEN_REG, size & 0xff);
    return 0;
}

//set block num
static void alimci_set_block_num(u32 block_num)
{
    ALI_MCI_WR8(SD_BLKNUM_L_REG, block_num);
    ALI_MCI_WR8(SD_BLKNUM_H_REG, (u8)(block_num >> 8));
}

//set dma addr, 
// David.Chen: addr should be 0x20 align: if not 0x04 align, system will hangup, if not 0x20 align, "mount /dev/mmcblk0 /mnt/sd" will failure.
static void alimci_set_dmar_addr(u32 addr)
{
    addr = __CTDADDRALI(virt_to_phys((void*)addr));
            alimci_log("\n#########Line:%d---alimci_set_dmar_addr(0x%x) #########\n\n",__LINE__,addr);
    ALI_MCI_WR32(SD_DMA_R_ADDR, addr);
}

//set dmar size
static void alimci_set_dmar_size(u32 size)
{
    ALI_MCI_WR32(SD_DMA_R_SIZE, size);
}

//set dmaw addr
static void alimci_set_dmaw_addr(u32 addr)
{
    //addr &= 0x0fffffff;
    addr = __CTDADDRALI(virt_to_phys((void*)addr));
    ALI_MCI_WR32(SD_DMA_W_ADDR, addr);
}

//set dmaw size
static void alimci_set_dmaw_size(u32 size)
{
    ALI_MCI_WR32(SD_DMA_W_SIZE, size);
}

//Just for sdio card
static int alimci_set_bus_width(unsigned char bus_width, int callline)
{
    alimci_log("\n\n#########<alimci_set_bus_width> is called from Line:%d---bus_width =0x%x #########\n\n",callline,bus_width);

    switch (bus_width)
    {
        case MMC_BUS_WIDTH_1: //SD_BUS_WIDTH_1:
            ALI_MCI_WR8(SD_BITCTRL_REG, ((ALI_MCI_RD8(SD_BITCTRL_REG) & (~0x0e)) | (1 << 2))); //sdio 1bit mode
            break;

        case MMC_BUS_WIDTH_8:
            ALI_MCI_WR8(SD_BITCTRL_REG, ((ALI_MCI_RD8(SD_BITCTRL_REG) & (~0x0e)) | (1 << 1))); //mmc 8bit mode
            break;

        case MMC_BUS_WIDTH_4: //SD_BUS_WIDTH_4:
        default:
            ALI_MCI_WR8(SD_BITCTRL_REG, ((ALI_MCI_RD8(SD_BITCTRL_REG) & (~0x0e)) | (1 << 3))); //sdio 4bit mode
            break;
    }

    return 0;
}

/********************************************************************
*
* The following data and formual is provided by Holon.zhang:
*
* SD_CLK = SD_IP_CLK/(2(SDFREQDIV-1)), SDFREQDIV = (SD_IP_CLK/(SD_CLK * 2)) + 1
*
* SD_CLK:      Per SD Card/eMMC Spec, It's suggested to set SD_CLK to several hundreds KHz (100-400KHz from Ali SDIO spec) when initializing and identifying.
*                    when R/W, C3921/3821 can reach up to 27MHz. S3921 is up to 10Mhz.
* SD_IP_CLK: It is 108MHz for C3921/3821, for S3921 it is MEM_CLK (DDR3 Freg/4), Eg. 1066MHz/4
*
********************************************************************/
static void alimci_change_clk(u16 clk_div)
{
    if (0 == clk_div)
    {
        alimci_err("---Error: Clock Divide can't be 0---\n");
        return;
    }
    ALI_MCI_WR8(SD_CLKDIV_L_REG, (u8)clk_div);
    ALI_MCI_WR8(SD_CLKDIV_H_REG, (u8)((clk_div & 0xFF00) >> 8));
}

static u16 alimci_get_clk(void)
{
    u16 clk_div = 0;

    clk_div = (ALI_MCI_RD8(SD_CLKDIV_H_REG) << 8) | ALI_MCI_RD8(SD_CLKDIV_L_REG);
    return clk_div;
}

//host reset sdio, success return 0, fail return 1
static void alimci_host_reset(struct mmc *mmc)
{
	struct alimci_host *host = (struct alimci_host *)mmc->priv;

    if (M3921 == host->ChipId || M3821 == host->ChipId)
    {
        ALI_MCI_WR8(SD_DMA_CTRL,ALI_MCI_RD8(SD_DMA_CTRL)|DMA_RESET);    
        ALI_SOC_WR32(LOCAL_DEV_RESET_CTRL_REG, ALI_SOC_RD32(LOCAL_DEV_RESET_CTRL_REG) | SDIO_SW_RESET);
        mdelay(1);
        ALI_SOC_WR32(LOCAL_DEV_RESET_CTRL_REG, ALI_SOC_RD32(LOCAL_DEV_RESET_CTRL_REG) & (~SDIO_SW_RESET));
        mdelay(1);
        return ;
    }
}

static void alimci_hw_gating(struct mmc *mmc,unsigned char gating)
{
	struct alimci_host *host = (struct alimci_host *)mmc->priv;

    if (M3921 == host->ChipId || M3821 == host->ChipId)
    {
        if (gating)
        {
            ALI_SOC_WR32(LOCAL_CLK_GATE_CTRL_REG1, ALI_SOC_RD32(LOCAL_CLK_GATE_CTRL_REG1) | SDIO_M_CLK_GATE);
        }
        else
        {
            ALI_SOC_WR32(LOCAL_CLK_GATE_CTRL_REG1, ALI_SOC_RD32(LOCAL_CLK_GATE_CTRL_REG1) & (~SDIO_M_CLK_GATE));
#if defined(CONFIG_ARM)
	     if (M3921 == host->ChipId && 0x00 == host->ChipVer) //S3921 default pad driving is 2.5mA, C3921 and C3821 is 5mA defaultly.
	     {
            	   ALI_SOC_WR32(PAD_DRVING_CTRL_REG1, ALI_SOC_RD32(PAD_DRVING_CTRL_REG1) | SDIO_5MA);
	     }
#endif
        }
        return ;
    }
}

static void alimci_pinmux_restore(struct alimci_host *host, int callline)
{
    //if strap pin is set for emmc, we do nothing
	int info_strapin = ALI_SOC_RD32(CHIP_STRAP_PIN); //read strap pin
	int boot_dev = BOOT_DEV(info_strapin);	
	if(BOOT_DEV_EMMC(boot_dev))//if emmc boot up
	{
	   return;	//do nothing
	}

#if defined(CONFIG_ARM)
    if (host->pinmux1)
    {
        ALI_SOC_WR32(SOC_PINMUX_REG1, host->pinmux1);
        ALI_SOC_WR32(SOC_PINMUX_REG2, host->pinmux2);
        //alimci_log("******<alimci_pinmux_restore> ** RESTORE PINMUX: 0x%08X,0x%08X  ***call from Line: %d******\n",host->pinmux1,host->pinmux2, callline);
    }
    if (M3921 == host->ChipId)
    {
        mutex_unlock(&ali_sto_mutex);
    }
#elif defined(CONFIG_MIPS)
    if (host->pinmux3)
    {
        ALI_SOC_WR32(SOC_PINMUX_REG3, host->pinmux3);
        //alimci_log("******<alimci_pinmux_restore> ** RESTORE PINMUX: 0x%08X,  ***call from Line: %d******\n",host->pinmux3,callline);
    }
#endif
}

static void alimci_pinmux_set(struct alimci_host *host, int callline)
{
#if defined(CONFIG_ARM)
	//if strap pin is set for emmc, we do nothing
    int info_strapin = ALI_SOC_RD32(CHIP_STRAP_PIN); //read strap pin
	int boot_dev = BOOT_DEV(info_strapin);	
	if(BOOT_DEV_EMMC(boot_dev))//if emmc boot up
    {
       return;  //do nothing
	}

    u32 PinMuxCtrl1;
    u32 PinMuxCtrl2;

    if (M3921 == host->ChipId) //On S3921, nand flash and SDIO share pin, we introduce ali_sto_mutex to share the shared pins.
    {
        mutex_lock(&ali_sto_mutex);
    }
    //PinMux selection
    PinMuxCtrl1 = ALI_SOC_RD32(SOC_PINMUX_REG1);
    PinMuxCtrl2 = ALI_SOC_RD32(SOC_PINMUX_REG2);
    if (0 == host->pinmux1)
    {
        host->pinmux1 = PinMuxCtrl1;
        host->pinmux2 = PinMuxCtrl2;
    }
	
	//S3921
    if ((M3921 == host->ChipId) && (M3921_S == host->ChipVer))     
    {
        //alimci_log("----------<alimci_pinmux_set> before setting ,called from line:%d--------pinmux1:0x%08x, pinmux2:0x%08x (StrapinInfo:0x%x, StrapinCtrl:0x%x)------------------------\n",callline,PinMuxCtrl1,PinMuxCtrl2,ALI_SOC_RD32(SOC_STRAPINFO_REG),ALI_SOC_RD32(SOC_STRAPCTRL_REG));
        ALI_SOC_WR32(SOC_PINMUX_REG2, PinMuxCtrl2 & (~(STRAPIN_SEL_ENABLE))); //disable REGISTER_SEL_ENABLE, will use pinmux selection
        ALI_SOC_WR32(SOC_PINMUX_REG2, ALI_SOC_RD32(SOC_PINMUX_REG2) | SD_FUNC_SEL);
        ALI_SOC_WR32(SOC_PINMUX_REG1, PinMuxCtrl1 & (~(NF_FUNC_SEL))); //disable nand flash select
        //sdrw.c of uflashrw use the following pinmux values.
        //ALI_SOC_WR32(SOC_PINMUX_REG1, 0x04000440);
        //ALI_SOC_WR32(SOC_PINMUX_REG2, 0x00000020);
        return ;
	}
	
	//C3921
	if ((M3921 == host->ChipId) && (M3921_C == host->ChipVer))       
	{  
		   
#if defined(DRV_4_eMMC)
//emmc
		if(M3921_BGA445 == host->ChipPkg)		  //C3921 BGA
	    {
	        ALI_SOC_WR32(SOC_GPIO_EN_CTRL_REG1, 0); 
			ALI_SOC_WR32(SOC_GPIO_EN_CTRL_REG2, 0); 
			ALI_SOC_WR32(SOC_GPIO_EN_CTRL_REG3, 0); 
			ALI_SOC_WR32(SOC_GPIO_EN_CTRL_REG4, 0x20000); 

	        PinMuxCtrl2 = ALI_SOC_RD32(SOC_PINMUX_REG2);
	        PinMuxCtrl2&=~(1<<29);
			ALI_SOC_WR32(SOC_PINMUX_REG2, PinMuxCtrl2); 
			
			PinMuxCtrl2 = ALI_SOC_RD32(SOC_PINMUX_REG2);
			PinMuxCtrl2&=~(1<<31);
			ALI_SOC_WR32(SOC_PINMUX_REG2, PinMuxCtrl2); 

			PinMuxCtrl2 = ALI_SOC_RD32(SOC_PINMUX_REG2);
			PinMuxCtrl2|=(1<<5);
			ALI_SOC_WR32(SOC_PINMUX_REG2, PinMuxCtrl2);

			return ;
		}
		else if(M3921_QFP256 == host->ChipPkg)	  //C3921 QFP
		{
			ALI_SOC_WR32(SOC_GPIO_EN_CTRL_REG1, 0); 
			ALI_SOC_WR32(SOC_GPIO_EN_CTRL_REG2, 0); 
			ALI_SOC_WR32(SOC_GPIO_EN_CTRL_REG3, 0); 
			ALI_SOC_WR32(SOC_GPIO_EN_CTRL_REG4, 0x20000); 

	        PinMuxCtrl2 = ALI_SOC_RD32(SOC_PINMUX_REG2);
	        PinMuxCtrl2&=~(1<<29);
			ALI_SOC_WR32(SOC_PINMUX_REG2, PinMuxCtrl2); 
			
			PinMuxCtrl2 = ALI_SOC_RD32(SOC_PINMUX_REG2);
			PinMuxCtrl2&=~(1<<31);
			ALI_SOC_WR32(SOC_PINMUX_REG2, PinMuxCtrl2); 

			PinMuxCtrl2 = ALI_SOC_RD32(SOC_PINMUX_REG2);
			PinMuxCtrl2|=(1<<5);
			ALI_SOC_WR32(SOC_PINMUX_REG2, PinMuxCtrl2);

			return ;
		}
#else    
//sd
		if(M3921_BGA445 == host->ChipPkg)		  //C3921 BGA
		{
		    ALI_SOC_WR32(SOC_GPIO_EN_CTRL_REG2, 0); 
	 	
			//C3921¨®??a¨°?¡Á¨¦
			PinMuxCtrl1 = 0x04000420;
			PinMuxCtrl2 = 0x0;
	        
			ALI_SOC_WR32(SOC_PINMUX_REG1, PinMuxCtrl1); 
			ALI_SOC_WR32(SOC_PINMUX_REG2, PinMuxCtrl2); 

			return ;
		}
		else if(M3921_QFP256 == host->ChipPkg)	  //C3921 QFP
		{
		    ALI_SOC_WR32(SOC_GPIO_EN_CTRL_REG2, 0); 
	 	
			//C3921¨®??a¨°?¡Á¨¦
			PinMuxCtrl1 = 0x04000420;
			PinMuxCtrl2 = 0x0;
	        
			ALI_SOC_WR32(SOC_PINMUX_REG1, PinMuxCtrl1); 
			ALI_SOC_WR32(SOC_PINMUX_REG2, PinMuxCtrl2); 

			return ;
		}
#endif
		   		
    }
#elif defined(CONFIG_MIPS)
    u32 PinMuxCtrl3;

    PinMuxCtrl3 = ALI_SOC_RD32(SOC_PINMUX_REG3);
    if (0 == host->pinmux3)
    {
        host->pinmux3 = PinMuxCtrl3;
    }

    if (M3821 == host->ChipId)
    {
        //alimci_log("----------<alimci_pinmux_set> before setting ,called from line:%d--------pinmux3:0x%08x, (StrapinInfo:0x%x, StrapinCtrl:0x%x)------------------------\n",callline,PinMuxCtrl3,ALI_SOC_RD32(SOC_STRAPINFO_REG),ALI_SOC_RD32(SOC_STRAPCTRL_REG));
        ALI_SOC_WR32(SOC_PINMUX_REG3, ALI_SOC_RD32(SOC_PINMUX_REG3) | (eMMC_SEL | SD_8BIT_SEL | SD_IF_SEL));
        return ;
    }
#endif
}

static void alimci_write_cmd(u8 ctr_reg, u8 cmd_reg, u32 arg_reg)
{
    ALI_MCI_WR32(SD_ARG_REG, arg_reg);
    ALI_MCI_WR8(SD_CMD_REG, cmd_reg);
    ALI_MCI_WR8(SD_CTRL_REG, ctr_reg);
}

static bool alimci_is_app_cmd(struct alimci_host *host, u32 cmd)
{
    bool ret = FALSE;

    if (host->LstCmd55 && cmd == host->cmd->cmdidx)
        ret = TRUE;

    return ret;
}

static void alimci_preprocess(struct alimci_host *host, struct mmc_cmd *cmd)
{
    switch (cmd->cmdidx)
    {
        case MMC_CMD_GO_IDLE_STATE:
	     if (host->InitClkDiv != alimci_get_clk())
	     {
                alimci_change_clk(host->InitClkDiv);
	     }
	     if (MMC_BUS_WIDTH_1 != host->bus_width)
	     {
                host->bus_width = MMC_BUS_WIDTH_1;
                alimci_set_bus_width(host->bus_width,__LINE__);
	     }
            break;

#if defined(DRV_4_eMMC) && defined(CONFIG_CMD_MMC) //if used in u-boot
	//In u-boot eMMC driver, if the argument of CMD1 is 0, the eMMC can't be power up, so no correct response is return.
        case MMC_CMD_SEND_OP_COND:
            mdelay(50);
            cmd->cmdarg = NORMAL_OCR_FOR_EMMC;
            break;
#endif

        default:
            break;
    }
}

static int alimci_start_cmd(struct alimci_host *host, struct mmc_cmd *cmd)
{
    u8 ctr_reg = ALI_SEND_CMD;
    u8 cmd_reg = (cmd->cmdidx & SD_CMD_IDX_MASK);
    u8 dir = WR_TO_CARD;
    u8 CmdType = 0;
    struct mmc_data *data = NULL;

    //WARN_ON(cmd != NULL);
    //host->cmd = cmd;

    alimci_preprocess(host, cmd);

    if (MMC_BUS_WIDTH_4 == host->bus_width)
    {
        ctr_reg |= SD_4BIT;
    }
    else
    {
        ctr_reg |= SD_MMC_1BIT;
    }

    switch (cmd->resp_type)
    {
        case MMC_RSP_NONE:
            ctr_reg |= ALI_MCI_RSPTYPE_NONE;
            break;
        case MMC_RSP_R1b:
            ctr_reg |= ALI_MCI_RSPTYPE_R1;
            break;
        case MMC_RSP_R2:
            ctr_reg |= ALI_MCI_RSPTYPE_R2;
            break;
        default:
            switch (cmd->cmdidx)
            {
#if !defined(CONFIG_CMD_MMC) //if NOT used in u-boot
                case MMC_FAST_IO:
                case MMC_GO_IRQ_STATE:
                case SD_IO_SEND_OP_COND:
                case SD_IO_RW_DIRECT:
                case SD_IO_RW_EXTENDED:
#endif
                case MMC_CMD_SET_RELATIVE_ADDR:
                case SD_CMD_SEND_IF_COND:
                    ctr_reg |= ALI_MCI_RSPTYPE_R4_7;
                    break;

                case MMC_CMD_SEND_OP_COND:
                case MMC_CMD_SPI_READ_OCR:
                    ctr_reg |= ALI_MCI_RSPTYPE_R3;
                    break;

                default:
                    if (alimci_is_app_cmd(host,SD_CMD_APP_SEND_OP_COND))
                        ctr_reg |= ALI_MCI_RSPTYPE_R3;
                    else
                        ctr_reg |= ALI_MCI_RSPTYPE_R1;
                    break;
            }
            break;
    }

    switch (cmd->cmdidx)
    {
        case MMC_CMD_GO_IDLE_STATE:
        case MMC_CMD_SET_DSR:
            CmdType = ALI_MCI_CMDTYPE_BC;
            break;

        case MMC_CMD_SEND_OP_COND:
        case MMC_CMD_ALL_SEND_CID:
        case SD_CMD_APP_SEND_OP_COND:
            CmdType = ALI_MCI_CMDTYPE_BCR;
            break;

        case MMC_CMD_SELECT_CARD:
        case MMC_CMD_SEND_CSD:
        case MMC_CMD_SEND_CID:
        case MMC_CMD_STOP_TRANSMISSION:
        case MMC_CMD_SEND_STATUS:
        case MMC_CMD_SET_BLOCKLEN:
        case MMC_CMD_ERASE_GROUP_START:
        case MMC_CMD_ERASE_GROUP_END:
        case MMC_CMD_ERASE:
        case MMC_CMD_APP_CMD:
        case MMC_CMD_SPI_READ_OCR:
        case MMC_CMD_SPI_CRC_ON_OFF:
        case SD_CMD_ERASE_WR_BLK_START:
        case SD_CMD_ERASE_WR_BLK_END:
            CmdType = ALI_MCI_CMDTYPE_AC;
            break;

        case MMC_CMD_READ_SINGLE_BLOCK:
        case MMC_CMD_READ_MULTIPLE_BLOCK:
        case MMC_CMD_WRITE_SINGLE_BLOCK:
        case MMC_CMD_WRITE_MULTIPLE_BLOCK:
        case SD_CMD_APP_SEND_SCR:
	     CmdType = ALI_MCI_CMDTYPE_ADTC_WR;
            break;

        case MMC_CMD_SWITCH:	//AC
        //case SD_CMD_APP_SET_BUS_WIDTH:	//AC
        //case SD_CMD_SWITCH_FUNC: //ADTC
        	if (host->data)
        	{
			CmdType = ALI_MCI_CMDTYPE_ADTC_WR;
        	}
		else
		{
			CmdType = ALI_MCI_CMDTYPE_AC;
		}
		break;

        case MMC_CMD_SEND_EXT_CSD: //ADTC
        //case SD_CMD_SEND_IF_COND:	//BCR
        	if (host->data)
        	{
			CmdType = ALI_MCI_CMDTYPE_ADTC_WR;
        	}
		else
		{
			CmdType = ALI_MCI_CMDTYPE_BCR;
		}
		break;

        case MMC_CMD_SET_RELATIVE_ADDR: //AC
        //case SD_CMD_SEND_RELATIVE_ADDR: //BCR
        	if (MMC_RSP_R6 == cmd->resp_type)
        	{
			CmdType = ALI_MCI_CMDTYPE_BCR;
        	}
		else
		{
			CmdType = ALI_MCI_CMDTYPE_AC;
		}
		break;

        default:
            alimci_err("unknown MMC command: %d...\n",cmd->cmdidx);
            break;
    }

	if (ALI_MCI_CMDTYPE_ADTC_WR == CmdType)
	{
		data = host->data;
            if (data->flags & MMC_DATA_WRITE)
            {
                ctr_reg |= ALI_MCI_CMDTYPE_ADTC_WR;
                dir = WR_TO_CARD;
            }
            else
            {
                ctr_reg |= ALI_MCI_CMDTYPE_ADTC_RD;
                dir = RD_FROM_CARD;
            }
	}
	else
	{
		ctr_reg |= CmdType;
	}


    cmd_reg |= dir;

    //Only SD2.0 and plus card response to SD_SEND_IF_COND
    //Only MMC/eMMC response to MMC_SEND_OP_COND
    //so I'm using Polling method to handle these 2 cmds.
    if (SD_CMD_SEND_IF_COND == cmd->cmdidx  && !data)
    {
        host->IntMode = SD_INT_DISABLE;
    }
#if !defined(DRV_4_eMMC)
    else if (MMC_CMD_SEND_OP_COND == cmd->cmdidx)
    {
        host->IntMode = SD_INT_DISABLE;
    }
#endif
    else
    {
        host->IntMode = SD_INT_ENABLE;
    }
    cmd_reg |= host->IntMode;

#if defined(CONFIG_ARM)
    alimci_log("\n\n---<u-boot>---%s%d---ctr_reg = 0x%02x, cmd_reg = 0x%02x, arg_reg = 0x%08x--RspType:%d--Pinmux1:0x%08x,Pinmux2:0x%08x,SD_BITCTRL_REG=0x%x----host->data_op_type=%d,CurClkDiv: 0x%04x ---\n",
            host->LstCmd55 ? "ACMD" : "CMD",
            cmd->cmdidx,
            ctr_reg, 
            cmd_reg, 
            cmd->cmdarg,
            (ctr_reg >> 4) & 0x7,
            ALI_SOC_RD32(SOC_PINMUX_REG1),
            ALI_SOC_RD32(SOC_PINMUX_REG2),
            ALI_MCI_RD8(SD_BITCTRL_REG),
            host->data_op_type,
            alimci_get_clk());
#elif defined(CONFIG_MIPS)
    alimci_log("\n\n---<u-boot>---%s%d---ctr_reg = 0x%02x, cmd_reg = 0x%02x, arg_reg = 0x%08x--RspType:%d--Pinmux1:0x%08x,Pinmux2:0x%08x,Pinmux3:0x%08x,Pinmux4:0x%08x,SD_BITCTRL_REG=0x%x----host->data_op_type=%d,CurClkDiv: 0x%04x ---\n",
            host->LstCmd55 ? "ACMD" : "CMD",
            cmd->cmdidx,
            ctr_reg, 
            cmd_reg, 
            cmd->cmdarg,
            (ctr_reg >> 4) & 0x7,
            ALI_SOC_RD32(SOC_PINMUX_REG1),
            ALI_SOC_RD32(SOC_PINMUX_REG2),
            ALI_SOC_RD32(SOC_PINMUX_REG3),
            ALI_SOC_RD32(SOC_PINMUX_REG4),
            ALI_MCI_RD8(SD_BITCTRL_REG),
            host->data_op_type,
            alimci_get_clk());
#endif
    alimci_write_cmd(ctr_reg, cmd_reg, cmd->cmdarg);
//    if (!IntMode)
        return alimci_status_polling(host,cmd);
}

//pio enable, success return 0, fail return 1
static int alimci_pio_enable(u32 enable)
{
    if (enable == 0)
    {
        ALI_MCI_WR8(SD_PIO_CTRL, ALI_MCI_RD8(SD_PIO_CTRL) & (~PIO_EN));
    }
    else
    {
        ALI_MCI_WR8(SD_PIO_CTRL, ALI_MCI_RD8(SD_PIO_CTRL) | PIO_EN);
    }
    return 0;
}


static int alimci_prepare_dma(struct alimci_host *host, struct mmc_data *data)
{
    int IsWrite = data->flags & MMC_DATA_WRITE;
    u32 *p  = NULL;
    u32 block_num = data->blocks;
    u32 byte_num;
    u32 block_size;

    BUG_ON((data->flags & BOTH_DIR) == BOTH_DIR);

    block_size = data->blocksize;
    alimci_set_block_size(block_size);

    alimci_pio_enable(0);   //disable PIO

    byte_num = block_size * block_num;

    alimci_set_block_num(block_num - 1); //register value = blockNum - 1

    if (IsWrite)
    {
	p = (uint32_t *)data->src;
        alimci_set_dmaw_addr((u32)p);
        alimci_set_dmaw_size(byte_num);
        host->data_op_type = DATA_OP_WRITE_1;
    }
    else
    {
	p = (uint32_t *)data->dest;
        alimci_set_dmar_addr((u32)p);
        alimci_set_dmar_size(byte_num);
        host->data_op_type = DATA_OP_READ;
    }


    alimci_dma_start(IsWrite ? DMA_WR_TO_CARD : DMA_RD_FROM_CARD);


    return 0;
}

static void alimci_finish_request(struct alimci_host *host,bool finish, int callline)
{
    //struct mmc_request *mrq = host->mrq;
    if (finish)
    {
        host->LstCmd55 = (MMC_CMD_APP_CMD == host->cmd->cmdidx ? TRUE : FALSE);
    }
    //alimci_log("******<alimci_finish_request> LINE:%d ** PinMux1: 0x%08x (0x%08x) | PinMux2: 0x%08x (0x%08x)   finish=%d ***call from Line: %d******\n",__LINE__,(u32)ALI_SOC_RD32(SOC_PINMUX_REG1),host->pinmux1,(u32)ALI_SOC_RD32(SOC_PINMUX_REG2),host->pinmux2, finish, callline);
    alimci_pinmux_restore(host, callline);
    //mmc_request_done(host->base_mmc, mrq);
}

static void alimci_postprocess(struct alimci_host *host, struct mmc_cmd *cmd)
{
    if (alimci_is_app_cmd(host,SD_CMD_APP_SET_BUS_WIDTH))
    {
        //cmd->arg = MMC_BUS_WIDTH_1; //try 1 bit read for some issue cards. Eg. B-00100
        alimci_set_bus_width(cmd->cmdarg,__LINE__);
        host->bus_width = cmd->cmdarg;
        alimci_change_clk(host->TranClkDiv);
    }
    else if (MMC_CMD_SWITCH == cmd->cmdidx && !(host->data)) //SD_SWITCH is 6 too, but it is ADTC
    {
        u8 index;
        u8 value;
        u8 width = 0;

        index = (cmd->cmdarg >> 16) & 0xff;
        value = (cmd->cmdarg >> 8) & 0xff;
        if (EXT_CSD_BUS_WIDTH == index)
        {
            switch (value)
            {
                case EXT_CSD_BUS_WIDTH_8:
                    width = MMC_BUS_WIDTH_8;
                    break;

                case EXT_CSD_BUS_WIDTH_4:
                    width = MMC_BUS_WIDTH_4;
                    break;

                case EXT_CSD_BUS_WIDTH_1:
                default:
                    width = MMC_BUS_WIDTH_1;
                    break;
            }

            alimci_set_bus_width(width,__LINE__);
            host->bus_width = width;
            alimci_change_clk(host->TranClkDiv);
        }
    }
}

static int alimci_cmd_tasklet(unsigned long para)
{
    int times = 0;
    u32 rspReg0 = 0;
    struct alimci_host *host = (struct alimci_host *)para;
    struct mmc_cmd *cmd = host->cmd;
    u8 status = ALI_MCI_RD8(SD_STATUS_REG);

    ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) | CLK_ALWAYS_EN));
    status = ALI_MCI_RD8(SD_STATUS_REG);

    while (SD_STATUS_OK_MASK != status)
    {
        if (times < 10)
            alimci_log("#### CMD:%d #####---wrong STATUS=0x%x####times=%d#####\n\n",cmd->cmdidx,status,times);
        times++;
#if 1
        if (times > MAX_WAIT_TIMES)
        {
            ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) & ~CLK_ALWAYS_EN));
            alimci_finish_request(host,FALSE,__LINE__);
            return ALIMCI_FAIL;
        }
#endif
      
        udelay(50);
        status = ALI_MCI_RD8(SD_STATUS_REG);
    };
    ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) & ~CLK_ALWAYS_EN));
    if (times > 10)
        alimci_log("#### CMD:%d #####---STATUS=0x%x####times=%d#####\n\n",cmd->cmdidx,status,times);

    if (cmd->resp_type & MMC_RSP_PRESENT)
    {
        if (cmd->resp_type & MMC_RSP_136)
        {
            /* Read card response from controller. */
            cmd->response[0] = ALI_MCI_RD32((SD_RESP_REG+12));
            cmd->response[1] = ALI_MCI_RD32((SD_RESP_REG+8));
            cmd->response[2] = ALI_MCI_RD32((SD_RESP_REG+4));
            cmd->response[3] = ALI_MCI_RD32(SD_RESP_REG);
        }
        else
        {
            rspReg0 = (__REGALIRAW(p_host->sdio_base + SD_RESP_REG + 1));
            cmd->response[0] = GET_UINT32_UNALIGNED(rspReg0);
        }
    }

    alimci_log("<alimci_cmd_tasklet> Line:%d---%s%d, SD_STATUS_REG=0x%02X---cmd->resp[0]=0x%08x,----%s-----------------\n",
        __LINE__,
        host->LstCmd55 ? "ACMD" : "CMD",
        cmd->cmdidx,
        status,cmd->response[0],
        (cmd->resp_type & MMC_RSP_136) ? "LONG Response" : (cmd->resp_type & MMC_RSP_PRESENT) ? "Short Response" : "No Reponse"
        );

#if 1
    if (cmd->resp_type & MMC_RSP_136)
    {
        alimci_log("===Resp[0]=0x%08x,Resp[1]=0x%08x,Resp[2]=0x%08x,Resp[3]=0x%08x===\n",cmd->response[0],cmd->response[1],cmd->response[2],cmd->response[3]);
    }
#endif

    if (SD_STATUS_OK_MASK == status)
        alimci_postprocess(host, cmd);
    //host->complete_what = COMPLETION_FINALIZE;
    alimci_finish_request(host,TRUE,__LINE__);
	return ALIMCI_SUCESS;

}

static int alimci_data_tasklet(unsigned long para)
{
    int times = 0;
    u32 rspReg0 = 0;
    u32 dma_ctrl_reg = 0;
    u32 idle_mask = 0;
    struct alimci_host *host = (struct alimci_host *)para;
    struct mmc_cmd *cmd = host->cmd;
    u8 status = ALI_MCI_RD8(SD_STATUS_REG);
//    struct mmc_data *data = host->data;

    ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) | CLK_ALWAYS_EN));
    status = ALI_MCI_RD8(SD_STATUS_REG);
    while (SD_STATUS_OK_MASK != status)
    {
//        if (times < 10)
//            alimci_err("#### CMD:%d #####---wrong STATUS=0x%x####WaitStatusLoops=%d#####\n\n",cmd->cmdidx,status,times);
        times++;

#if 1
        if (times > MAX_WAIT_TIMES)
        {
            ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) & ~CLK_ALWAYS_EN));
            alimci_finish_request(host,FALSE,__LINE__);
            return ALIMCI_FAIL;
        }
#endif
      
        udelay(50);
        status = ALI_MCI_RD8(SD_STATUS_REG);
    };
    ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) & ~CLK_ALWAYS_EN));
    if (times > 10)
        alimci_log("#### CMD:%d #####---STATUS=0x%x####WaitStatusLoops=%d times#####\n\n",cmd->cmdidx,status,times);

    if (cmd->resp_type & MMC_RSP_PRESENT)
    {
        if (cmd->resp_type & MMC_RSP_136)
        {
            /* Read card response from controller. */
            cmd->response[0] = ALI_MCI_RD32((SD_RESP_REG+12));
            cmd->response[1] = ALI_MCI_RD32((SD_RESP_REG+8));
            cmd->response[2] = ALI_MCI_RD32((SD_RESP_REG+4));
            cmd->response[3] = ALI_MCI_RD32(SD_RESP_REG);
        }
        else
        {
            rspReg0 = (__REGALIRAW(p_host->sdio_base + SD_RESP_REG + 1));
            cmd->response[0] = GET_UINT32_UNALIGNED(rspReg0);
        }
    }

    alimci_log("<alimci_data_tasklet> Line:%d---%s%d, SD_STATUS_REG=0x%02X,DMA_STATUS=0x%08X---cmd->resp[0]=0x%08X,----%s-----WaitStatusLoops=%d------------\n",
        __LINE__,
        host->LstCmd55 ? "ACMD" : "CMD",
        cmd->cmdidx,
        status,
        ALI_MCI_RD32(SD_DMA_CTRL),
        cmd->response[0],
        (cmd->resp_type & MMC_RSP_136) ? "LONG Response" : (cmd->resp_type & MMC_RSP_PRESENT) ? "Short Response" : "No Reponse",
        times
        );

    if (SD_STATUS_OK_MASK != status)
    {
        if (0 != (status & (CRC16_ERR | CRC7_ERR)))
        {
            alimci_err("-----------CRC error,status=0x%02X---------------\n",status);
        }
        else if (0 != (status & (CMD_BUSY | DATA_BUSY)))
        {
            alimci_err("-----------BUSY,status=0x%02X---------------\n", status);
        }
        else
        {
            alimci_err("-----------Ohter error,status=0x%02X---------------\n", status);
        }
    }

    if (DATA_OP_READ == host->data_op_type)  //read irq
    {
        //host->complete_what = COMPLETION_FINALIZE;
        host->data_op_type = NO_DATA_OP;
        if (SD_STATUS_OK_MASK == status)
        {
        	//alimci_dumphex(host->data->dest,host->data->blocksize * host->data->blocks, __FUNCTION__, __LINE__);
        }
    }
    else if (DATA_OP_WRITE_1 == host->data_op_type)      //First irq for write
    {
        host->data_op_type = DATA_OP_WRITE_2;

        alimci_log("<alimci_data_tasklet> Line:%d---SD_STATUS_REG=0x%02X,DMA_STATUS=0x%08x--------------------\n",__LINE__,status,ALI_MCI_RD32(SD_DMA_CTRL));

        //Enable sd bus idle interrupt
        ALI_MCI_WR32(SD_DMA_CTRL, (dma_ctrl_reg | TX_BUS_IDLE_INT_ENABLE));
	if (SD_INT_ENABLE == host->IntMode) //Poll interrupt status to ensure some CMDs (Eg. Read/Write) finish.
	{
		while(0 == (ALI_MCI_RD8(SD_DMA_CTRL) & SYS_INT_STATUS))
			udelay(1);
		ALI_MCI_WR8(SD_DMA_CTRL, (ALI_MCI_RD8(SD_DMA_CTRL) | SYS_INT_STATUS)); //clear SD interrupt
		alimci_log("****** LINE:%d *****CMD: %d, Recieve interrupt signal******\n",__LINE__,cmd->cmdidx);
	}
        dma_ctrl_reg = ALI_MCI_RD32(SD_DMA_CTRL);
        idle_mask = (TX_BUS_IDLE_INT_STATUS | TX_BUS_IDLE_INT_ENABLE);
        alimci_log("<alimci_data_tasklet> Line:%d---SD_STATUS_REG=0x%02X,DMA_STATUS=0x%08x --------------------\n",__LINE__, status,ALI_MCI_RD32(SD_DMA_CTRL));
        if ((dma_ctrl_reg & idle_mask) == idle_mask)   //the second irq for write
        {

            host->data_op_type = NO_DATA_OP;

            //disable sd bus idle interrupt
            ALI_MCI_WR32(SD_DMA_CTRL, (ALI_MCI_RD32(SD_DMA_CTRL) & ~TX_BUS_IDLE_INT_ENABLE));
            //clear sd bus idle interrupt status
            ALI_MCI_WR32(SD_DMA_CTRL, (ALI_MCI_RD32(SD_DMA_CTRL) | TX_BUS_IDLE_INT_STATUS));

            //host->complete_what = COMPLETION_FINALIZE;
            //then enter the following send STOP cmd.

        }
    }

    alimci_finish_request(host,TRUE,__LINE__);
    return ALIMCI_SUCESS;

}

static int alimci_status_polling(struct alimci_host *host, struct mmc_cmd *cmd)
{
    u8 status;
    unsigned long times = 0;

	if (SD_INT_ENABLE == host->IntMode) //Poll interrupt status to ensure some CMDs (Eg. Read/Write) finish.
	{
		while(0 == (ALI_MCI_RD8(SD_DMA_CTRL) & SYS_INT_STATUS))
			udelay(1);
		ALI_MCI_WR8(SD_DMA_CTRL, (ALI_MCI_RD8(SD_DMA_CTRL) | SYS_INT_STATUS)); //clear SD interrupt
		alimci_log("****** LINE:%d *****CMD: %d, Recieve interrupt signal******\n",__LINE__,cmd->cmdidx);
	}

    do
    {
        times++;
        if (times > MAX_WAIT_TIMES)
            break;
        udelay(1);
        ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) | CLK_ALWAYS_EN));
        status = ALI_MCI_RD8(SD_STATUS_REG);
        ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) & ~CLK_ALWAYS_EN));
        if (1 == times && SD_STATUS_OK_MASK != status)
            alimci_log("****** LINE:%d *****CMD: %d, status=0x%x******\n",__LINE__,cmd->cmdidx,status);
    }while(SD_STATUS_OK_MASK != status);

    if (SD_STATUS_OK_MASK != status && times > MAX_WAIT_TIMES)
    {
        //timeout, maybe this card doesn't support this command.
        alimci_log("****** LINE:%d *****excute CMD: %d TIMEOUT, status=0x%x,times=%d******\n",__LINE__,cmd->cmdidx,status,times);
//	if (MMC_CMD_SEND_EXT_CSD == cmd->cmdidx)
//		alimci_dump_status(host,__FUNCTION__,__LINE__);

        cmd->response[0] = RESP_4_TIMEOUT;
        if (cmd->resp_type & MMC_RSP_136)
        {
            cmd->response[1] = RESP_4_TIMEOUT;
            cmd->response[2] = RESP_4_TIMEOUT;
            cmd->response[3] = RESP_4_TIMEOUT;
        }
        alimci_finish_request(host,TRUE,__LINE__);
        return ALIMCI_FAIL;
    }

    if (NO_DATA_OP != host->data_op_type)
    {
        return alimci_data_tasklet((unsigned long) host);
    }
    else     //cmd irq
    {
        return alimci_cmd_tasklet((unsigned long) host);
    }
}

//handler data cmd, divide into read cmd and write cmd, success return 0, fail return 1
static int alimci_prepare_data(struct alimci_host *host, struct mmc_data *data)
{
    u32 ret = 0;

    alimci_log("<alimci_prepare_data> %s %d blocks of %d bytes \n",
            (data->flags & MMC_DATA_WRITE) ? "Write" : "Read",
            data->blocks, data->blocksize);

    if ((u32) (data->src) % ARCH_DMA_MINALIGN)
    {
        alimci_err("--------Address 0x%x is not 0x%x aligned---------------\n",data->src, ARCH_DMA_MINALIGN);
        BUG_ON((u32) (data->src) % ARCH_DMA_MINALIGN);
    }

    if (host->usedma)
    {
        ret = alimci_prepare_dma(host, data);
    }
#if 0
    else
    {
        ret = alimci_prepare_pio(host, data);
    }
#endif

    return ret;
}

//handler mmc_core request.
static int alimci_send_request(struct mmc *mmc, struct mmc_cmd *cmd,struct mmc_data *data)
{
    bool CmdSupported = TRUE;
    struct alimci_host *host = (struct alimci_host *)mmc->priv;

    host->cmd = cmd;
    host->data = data;
    host->data_op_type = NO_DATA_OP;
    if (host->data)
    {
        int res = alimci_prepare_data(host, host->data);
        if (res)
        {
            alimci_err("alimci_prepare_data error!\n");
            alimci_finish_request(host,FALSE,__LINE__);
            return ALIMCI_FAIL;
        }
    }

    switch (cmd->cmdidx)
    {
#if 0
        /* Ali SD controller does not support SDIO now */
        case SD_IO_RW_DIRECT:
        case SD_IO_SEND_OP_COND:
            CmdSupported = FALSE;
            break;
#endif

        default:
            // Both MMC_CMD_SEND_EXT_CSD and SD_CMD_SEND_IF_COND has same value(8), but the previous one is ADTC type.
            if (SD_CMD_SEND_IF_COND == cmd->cmdidx && !data) //cmd8 for SD2.0 and plus
            {
                if (ALI_SDIO_MT_UNKNOWN != alimci_get_media_type() && ALI_SDIO_MT_SD20PLUS != alimci_get_media_type())
                {
                    CmdSupported = FALSE;
                }
            }
            else if (MMC_CMD_SEND_EXT_CSD == cmd->cmdidx && data) //cmd8 for MMC/eMMC
            {
                if (ALI_SDIO_MT_eMMC != alimci_get_media_type() && ALI_SDIO_MT_MMC != alimci_get_media_type())
                {
                    CmdSupported = FALSE;
                }
            }
            else if (MMC_CMD_APP_CMD == cmd->cmdidx) //cmd55 is NOT supported by MMC/eMMC?
            {
                if (ALI_SDIO_MT_eMMC == alimci_get_media_type() || ALI_SDIO_MT_MMC == alimci_get_media_type())
                {
                    CmdSupported = FALSE;
                }
            }
            else if (MMC_CMD_SWITCH == cmd->cmdidx && !(host->data)) //SD_SWITCH is 6 too, but it is ADTC
            {
			u8 index;
			u8 value;

			index = (cmd->cmdarg >> 16) & 0xff;
			value = (cmd->cmdarg >> 8) & 0xff;
			if (EXT_CSD_BUS_WIDTH == index && EXT_CSD_BUS_WIDTH_8 == value)
//			if (EXT_CSD_BUS_WIDTH == index && EXT_CSD_BUS_WIDTH_1 != value)
			{
				//NOTE: Ali SD slot only has 4 lines connected. so SD/MMC card doesn't support 8 bit mode. But eMMC does support 8 bit mode.
#if defined(DRV_4_eMMC)
				//if (ALI_SDIO_MT_eMMC != alimci_get_media_type())
				if (ALI_SDIO_MT_eMMC != alimci_get_media_type() && ALI_SDIO_MT_MMC != alimci_get_media_type())
				{
				    CmdSupported = FALSE;
				}
#else
				CmdSupported = FALSE;
#endif
			}
            }
    }

    if (CmdSupported)
    {
        alimci_pinmux_set(host, __LINE__);
	return alimci_start_cmd(host, cmd);
    }
    else
    {
        alimci_log("---------Non-supported CMD: %d, return directly-------------\n",cmd->cmdidx);
        alimci_finish_request(host,FALSE,__LINE__);
	return TIMEOUT; //ALIMCI_FAIL;
    }
}

// return 1 when card present, 0 for card not exist
static int alimci_card_present(struct mmc *mmc)
{
#if defined(DRV_4_eMMC)
    return 1;
#else
    struct alimci_host *host = (struct alimci_host *)mmc->priv;
    int ret;

    ret = gpio_get_value(host->gpio_cd);
    if (host->gpio_cd_invert)
        ret = !ret;

    alimci_log("-----------Line:%d,GPIO_CD=%d,ret=0x%x,gpio_cd_invert=0x%x---------------\n", __LINE__,host->gpio_cd,ret,host->gpio_cd_invert);
    return ret;
#endif
}

static void alimci_set_ios(struct mmc *mmc)
{
	return ;
}

static int alimci_reg_gpio(struct alimci_host *host)
{
    int ret, gpio_cd = -1, gpio_ro = -1, gpio_power = -1;
    bool gpio_power_invert;
    bool gpio_cd_invert;

    switch (host->ChipId)
    {
#if defined(CONFIG_ARM)
        case M3921:
            if ((M3921_S == host->ChipVer) && (M3921_BGA445 == host->ChipPkg))      //S3921 BGA
            {
                gpio_cd = GPIO_BGA_CD;
                gpio_ro = GPIO_BGA_WP;
                gpio_power = GPIO_BGA_PWR;
                gpio_power_invert = TRUE;
                gpio_cd_invert = CARD_DETECT_INVERT;
				host->InitClkDiv = ALI_INIT_CLKDIV_S3921;
                host->TranClkDiv = ALI_TRAN_CLKDIV_S3921;
            }
            else if ((M3921_S == host->ChipVer) && (M3921_QFP256 == host->ChipPkg))   //S3921 QFP
            {
                gpio_cd = GPIO_QFP_CD;
                gpio_ro = GPIO_QFP_WP;
                gpio_power = GPIO_QFP_PWR;
                gpio_power_invert = TRUE;
                gpio_cd_invert = CARD_DETECT_INVERT;
				host->InitClkDiv = ALI_INIT_CLKDIV_S3921;
                host->TranClkDiv = ALI_TRAN_CLKDIV_S3921;
            }
			else if ((M3921_C == host->ChipVer) && (M3921_QFP256 == host->ChipPkg))   //C3921 QFP
            {
                gpio_cd = 113;
                gpio_ro = GPIO_QFP_WP;
                //gpio_power = GPIO_QFP_PWR;
                gpio_power_invert = TRUE;
                gpio_cd_invert = CARD_DETECT_INVERT;
				host->InitClkDiv = ALI_INIT_CLKDIV_C3921;
                host->TranClkDiv = ALI_TRAN_CLKDIV_C3921;
            }
			else if ((M3921_C == host->ChipVer) && (M3921_BGA445 == host->ChipPkg))   //C3921 BGA unknown
            {
                gpio_cd = 113;
                gpio_ro = GPIO_QFP_WP;
                //gpio_power = GPIO_QFP_PWR;
                gpio_power_invert = TRUE;
                gpio_cd_invert = CARD_DETECT_INVERT;
				host->InitClkDiv = ALI_INIT_CLKDIV_C3921;
                host->TranClkDiv = ALI_TRAN_CLKDIV_C3921;          
			}
			
#if !defined(DRV_4_eMMC)
            host->irq_cd = ALI_ARM_GPIO_IRQ;
#endif
            break;

#elif defined(CONFIG_MIPS)
        case M3821:
            if (M3821_QFP156 == host->ChipPkg)
            {
                gpio_cd = GPIO_QFP156_CD;
                gpio_ro = GPIO_QFP156_WP;
                gpio_power = GPIO_QFP156_PWR;
                gpio_power_invert = TRUE; //???
                gpio_cd_invert = CARD_DETECT_INVERT;
            }
            else
            {
                gpio_cd = GPIO_BGA_CD;
                gpio_ro = GPIO_BGA_WP;
                gpio_power = GPIO_BGA_PWR;
                gpio_power_invert = TRUE; //???FALSE;
                gpio_cd_invert = CARD_DETECT_INVERT;
            }
    #if !defined(DRV_4_eMMC)
            host->irq_cd = ALI_MIPS_GPIO_IRQ;
    #endif
            host->InitClkDiv = ALI_INIT_CLKDIV_C3821;
            host->TranClkDiv = ALI_TRAN_CLKDIV_C3821;
            break;
#endif

        default:
#if defined(CONFIG_ARM)
            break;
#elif defined(CONFIG_MIPS)
            break;
#endif
    }
    
    if (host)
    {
        host->gpio_cd = gpio_cd;
        host->gpio_cd_invert = gpio_cd_invert;
        host->gpio_card_ro = gpio_ro;
        host->gpio_card_ro_invert = FALSE; // ???
        host->gpio_power = gpio_power;
        host->gpio_power_invert = gpio_power_invert;
    }

    if (gpio_is_valid(host->gpio_power))
    {
        ret = gpio_request(host->gpio_power, "mmc card power");
        if (ret)
        {
            alimci_err("Failed requesting gpio_power %d,ret=0x%x\n", host->gpio_power,ret);
            goto out;
        }
        //gpio_get_value(host->gpio_power_orig);
        gpio_direction_output(host->gpio_power,host->gpio_power_invert ? GPIO_LOW:GPIO_HIGH);
    }

    if (gpio_is_valid(gpio_cd))
    {
        ret = gpio_request(gpio_cd, "mmc card detect");
        if (ret)
        {
            alimci_err("Failed requesting gpio_cd %d\n", gpio_cd);
            goto err_gpio_cd;
        }
        gpio_direction_input(gpio_cd);
    }
    return 0;

#if !defined(DRV_4_eMMC)
err_request_irq:
    gpio_free(gpio_cd);
#endif
err_gpio_cd:
    gpio_free(gpio_ro);
err_gpio_ro:
    gpio_free(gpio_power);
out:
    return 1;
}

static int ali_mmc_init(struct mmc *mmc)
{
	struct alimci_host *host = (struct alimci_host *)mmc->priv;

	alimci_hw_gating(mmc,CLK_GATE_ON);
	alimci_pinmux_set(host, __LINE__);
	alimci_host_reset(mmc);
	alimci_reg_gpio(host);

	return 0;
}

int ali_mmc_register(bd_t *bis)
{
	struct mmc *mmc;
	struct alimci_host *host;
	int ret = -ENOMEM;

	mmc = malloc(sizeof(struct mmc));
	if (!mmc)
		goto err0;

	host = malloc(sizeof(struct alimci_host));
	if (!host)
		goto err1;

    p_host = host;
    host->detect_delay_ms = DETECT_DELAY;
    host->data_op_type = NO_DATA_OP;
    host->pinmux1 = 0;
    host->pinmux2 = 0;
    host->bus_width = MMC_BUS_WIDTH_1;
    host->usedma = TRUE; //dma or pio
    host->base_mmc = mmc;

    host->soc_base = PHYS_SOC_BASE_ADDR;
    get_chip_info(&host->ChipId, &host->ChipPkg, &host->ChipVer);

    switch (host->ChipId)
    {
#if defined(CONFIG_ARM)
        case M3921:
            host->sdio_base = PHYS_SD_BASE_ADDR;
            break;
#endif

#if defined(CONFIG_MIPS)
        case M3821:
        case M3503:
            host->sdio_base = PHYS_SD_BASE_ADDR;
            break;
#endif

        default:
            break;
    }

	mmc->priv = host;
	ali_mmc_init(mmc);

	sprintf(mmc->name, "ALiMMC");
	mmc->send_cmd	= alimci_send_request;
	mmc->set_ios	= alimci_set_ios;
	mmc->init	= ali_mmc_init;
	mmc->getcd	= alimci_card_present;

	mmc->voltages	= MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->f_max	= ALIMMC_MAX_SPEED;
	mmc->f_min	= ALIMMC_MIN_SPEED;
	mmc->host_caps	= ALIMMC_HOST_CAPS;

	mmc->b_max = 0;


	mmc_register(mmc);
	alimci_pinmux_restore(host, __LINE__);

	return 0;

err2:
	free(host);
err1:
	free(mmc);
err0:
	alimci_pinmux_restore(host, __LINE__);
	return ret;
}

#define RETRY_TIMES 3
int alimci_detect_media_type(struct mmc *mmc)
{
	int err;
	int i;

	cur_media_type = ALI_SDIO_MT_UNKNOWN;

	if (!alimci_card_present(mmc))
	{
		printk("Card is not inserted!\n");
		return 1;
	}

	// fix bug:14484: SDIO compatibility, can't identify a MMC card (2-1-X-V-00377)
	// Root cause: sometimes the card doesn't response to CMD1 in alimci_detect_media_type(), may because of timing issue, this result in <core> layer treat this card as a SD card, then CMD8 doesn't reponse.
	// Solution: Try up to 3 times CMD1 to detect the media type. Now the card can be detected correctly.
	i = 0;
	do
	{
		i++;
		alimci_host_reset(mmc);
		//Firstly, send cmd1 (MMC_SEND_OP_COND) to try to find MMC/eMMC
		//before CMD1, we'd better send CMD0 to reset the card, but the following mmc_send_op_cond() already includes the call of mmc_go_idle().
		//mmc_go_idle(mmc);
		//mdelay(50);
		err = mmc_send_op_cond(mmc);
		if (!err)
		{
			cur_media_type = ALI_SDIO_MT_MMC;
		}
	}while (ALI_SDIO_MT_UNKNOWN == cur_media_type && i < RETRY_TIMES);

	//Secondly, send cmd8 (SD_SEND_IF_COND) to try to find SD2.0 and plus card,need reset and send cmd0 again.
	if (ALI_SDIO_MT_UNKNOWN == cur_media_type)
	{
		alimci_host_reset(mmc);
		mmc_go_idle(mmc);
		mdelay(1);
		err = mmc_send_if_cond(mmc);
		if (0 == err)
		{
			cur_media_type = ALI_SDIO_MT_SD20PLUS;
		}
	}

	//Thirdly, send Acmd41 (MMC_CMD_APP_CMD) to try to find SD1.10 and minus card,need reset and send cmd0 again.
	if (ALI_SDIO_MT_UNKNOWN == cur_media_type)
	{
		alimci_host_reset(mmc);
		mmc_go_idle(mmc);
		mdelay(1);
		err = sd_send_op_cond(mmc);
		if (0 == err)
		{
			cur_media_type = ALI_SDIO_MT_SD110MINUS;
		}
	}
	alimci_host_reset(mmc); //reset host to enter normal/original card identification process.
	alimci_log("\n----<%s> Line:%d,cur_media_type=%d,i=%d,err=%d,card_present=0x%x------\n\n",__FUNCTION__, __LINE__,cur_media_type,i,err,alimci_card_present(mmc));
	
	return 0;
}

void alimmc_info(void)
{
	char buf[STA_INF_BUF_SZ];

	if(p_host)
	{
		printf( "\n=========== ALi SDIO status dump start =======\n");
		alimci_get_status(p_host,buf);
		puts(buf);
		printf( "=========== ALi SDIO status dump done =======\n");
	}
}


