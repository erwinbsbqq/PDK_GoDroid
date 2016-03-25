/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2010 - 2013 Copyright (C)
 *
 *  File: alimci.c
 *
 *  Description: SD/MMC Card Host driver for ALI 36XX/39XX platform
 *
 *  History:
 *          Date                    Author          Comment
 *          ====                    ======          =======
 * 0.       2010.6.28               Alex Xia        Create
 * 1.       2013.8.14               David Chen   Re-architecture for S3921
 ****************************************************************************/

#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/mmc/host.h>
#include <linux/platform_device.h>
#include <linux/cpufreq.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <asm/dma.h>
#include <linux/interrupt.h>
#include <linux/scatterlist.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <asm/io.h> /* for virt_to_phys */   
#include <asm/uaccess.h> /* for copy_from_user() */
#include <linux/moduleparam.h>
#include <linux/version.h> //macro LINUX_VERSION_CODE
#include <linux/seq_file.h>
#include <linux/mmc/card.h>
#include <linux/genhd.h>

#include <ali_board_config.h>

#if defined(CONFIG_ARM)  //Currently only ARM(S3921 project) has these .h files.
#include <linux/ali_dma.h>
#endif
#include "ali_mci.h"
#include <asm/mach-ali/m36_gpio.h>
#include "../core/core.h"



static struct alimci_host *p_host_sd = NULL;
static struct alimci_host *p_host_emmc = NULL;

#define DFT_DBG_OUT          ALIMCI_OFF
#define STA_INF_BUF_SZ       1000  //1500 
#define MAX_WAIT_TIMES       10000 //timeout
#define MAX_POLLING_TIME     2500  //2500
#define WAIT_IRQ_TIMEOUT_MS  1000  //wait for irq timeout, msec


#define ALI_SD_DRV_VER "SD02_L20131215"

#define alimci_err(format, args...)		printk( KERN_ERR "<%s> line: %d: "format,  __FUNCTION__, __LINE__, ##args)

static bool wait_event_flag = FALSE;

static char *alimci_param = NULL;
module_param(alimci_param, charp, 0444);
MODULE_PARM_DESC(alimci_param, "Parameters from insmod command line.");

static u16 alimci_get_clk(void);
static int alimci_card_present(struct mmc_host *mmc);
static int alimci_get_ro(struct mmc_host *mmc);
static void alimci_status_polling(struct alimci_host *host, struct mmc_command *cmd);
static void alimci_status_polling_level2(struct alimci_host *host);
static void alimci_waitfor_irq_timeout_timer(unsigned long data);
static void alimci_sd_emmc_pinmux_restore(struct alimci_host *host, int callline);
static void alimci_sd_emmc_pinmux_set(struct alimci_host *host,  int callline);
static void alimci_finish_request(struct alimci_host *host,bool finish, int callline);
static void alimci_start_cmd_level2(struct alimci_host *host, struct mmc_command *cmd);

    
/**
 * printk - print a kernel message
 * @fmt: format string
 *
 * This is printk().  It can be called from any context.  We want it to work.
 *
 * We try to grab the console_lock.  If we succeed, it's easy - we log the output and
 * call the console drivers.  If we fail to get the semaphore we place the output
 * into the log buffer and return.  The current holder of the console_sem will
 * notice the new output in console_unlock(); and will send it to the
 * consoles before releasing the lock.
 *
 * One effect of this deferred printing is that code which calls printk() and
 * then changes console_loglevel may break. This is because console_loglevel
 * is inspected when the actual printing occurs.
 *
 * See also:
 * printf(3)
 *
 * See the vsnprintf() documentation for format string extensions over C99.
 */
static asmlinkage int alimci_log(const char *fmt, ...)
{
    va_list args;
    int r;
	struct alimci_host *host = NULL;

	host = p_current_host;
	
    if (!host || host->DbgOut)
    {
        va_start(args, fmt);
        r = vprintk(fmt, args);
        va_end(args);
    }

    return 0;
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
    alimci_sd_emmc_pinmux_set(host,__LINE__);
    
#if defined(CONFIG_ARM)
    sprintf(buf, "SoC Base addr: 0x%08x | SoC ChipID:  0x%08x | SDIO Base addr: 0x%08x  | PAD Drv Ctrl1: 0x%08x | DetectDelay: %d ms\n",
              host->soc_base,
              (u32)ALI_SOC_RD32(SOC_CHIP_VER),
              host->sdio_base,
              (u32)ALI_SOC_RD32(PAD_DRVING_CTRL_REG1),
             host->detect_delay_ms);
    sprintf(buf, "%sDriver Ver:%s | SoC PinMux1: 0x%08x (0x%08x) | PinMux2: 0x%08x (0x%08x)\n",
             buf,
             ALI_SD_DRV_VER,
             (u32)ALI_SOC_RD32(SOC_PINMUX_REG1),
             host->pinmux1,
             (u32)ALI_SOC_RD32(SOC_PINMUX_REG2),
             host->pinmux2
             );
#elif defined(CONFIG_MIPS)
    sprintf(buf, "SoC Base addr: 0x%08x | SoC ChipID:  0x%08x | SDIO Base addr: 0x%08x  | DetectDelay: %d ms\n",
             host->soc_base,
             (u32)ALI_SOC_RD32(SOC_CHIP_VER),
             host->sdio_base,
             host->detect_delay_ms);
    sprintf(buf, "%sDriver Ver:%s | SoC PinMux1: 0x%08x (0x%08x) | PinMux2: 0x%08x (0x%08x) | SoC PinMux1: 0x%08x (0x%08x) | PinMux2: 0x%08x (0x%08x)\n",
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
             alimci_get_ro(host->base_mmc) ? "Yes" : "No",
             ali_gpio_get_value(host->gpio_power),
             SDIO_MediaType[alimci_get_media_type(host->base_mmc)] 
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
    alimci_sd_emmc_pinmux_restore(host,__LINE__);
}

#define ALIMCI_DBG_PROC_SD_DIR    "alisd"
#define ALIMCI_DBG_PROC_INFO      "info"
#define ALIMCI_DBG_PROC_CTRL      "ctrl"
#define ALIMCI_DBG_PROC_EMMC_DIR  "aliemmc"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
ssize_t alimci_procfile_sd_ifo_read(struct file *file, char __user *ubuf, size_t size, loff_t *ppos)
{
    struct alimci_host *host = NULL;
    char buf[STA_INF_BUF_SZ];
    
    host = p_host_sd;
    if (host != NULL)
    {    
        memset(buf,0,sizeof(buf));
        alimci_get_status(host,buf);
        return simple_read_from_buffer(ubuf, size, ppos, buf,strlen(buf));
    }
    else
    {
        return 0;
    }
}
#else
int alimci_procfile_sd_ifo_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    struct alimci_host *host = NULL;
    char buf[STA_INF_BUF_SZ];
    
    host = p_host_sd;
    if (host != NULL)
    {    
        memset(buf,0,sizeof(buf));
        alimci_get_status(host,buf);
        return snprintf(page, count, buf);
    }
    else
    {
        return 0;
    }
}
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
ssize_t alimci_procfile_sd_ctrl_read(struct file *file, char __user *ubuf, size_t size, loff_t *ppos)
{
    struct alimci_host *host = NULL;
    static const char message[256];
    host = p_host_sd;
    if (host != NULL)
    {    
        snprintf(message, 256, "DbgOut=%d,ForceBitWidth=%d\n",
                                 host->DbgOut, host->ForceBitWidth);
        return simple_read_from_buffer(ubuf, size, ppos, message, strlen(message));
    }
    else
    {
        return 0;
    }
}
#else
int alimci_procfile_sd_ctrl_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    struct alimci_host *host = NULL;
    
    host = p_host_sd;
    if (host != NULL)
    {    
        return snprintf(page, count, "DbgOut=%d,ForceBitWidth=%d\n",
                                 host->DbgOut, host->ForceBitWidth);
    }
    else
    {
        return 0;
    }
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static ssize_t alimci_procfile_sd_ctrl_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
#else
int alimci_procfile_sd_ctrl_write(struct file *filp, const char __user *buffer, unsigned long count, void *data)
#endif
{
    char buf[128];
    int DbgOut = 0;
    int ForceBitWidth = FORCE_BW_OFF;
    struct alimci_host *host = NULL;

    
    host = p_host_sd;

    if (host != NULL)
    {  
        if(count<=0)
            return 0;

        memset(buf,0,sizeof(buf));
        if (copy_from_user(buf, buffer, count))  
            return -EFAULT; 
		
        if (sscanf(buf, "DbgOut=%d,ForceBitWidth=%d", &DbgOut,&ForceBitWidth) != 2)
            return 0;

        if (ALIMCI_OFF == DbgOut || ALIMCI_ON == DbgOut)
            host->DbgOut = DbgOut;

        if (FORCE_BW_1_BIT == ForceBitWidth || FORCE_BW_4_BIT == ForceBitWidth)
            host->ForceBitWidth = ForceBitWidth;

        return count;
    }
    else
    {
        return 0;
    }
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
ssize_t alimci_procfile_emmc_ifo_read(struct file *file, char __user *ubuf, size_t size, loff_t *ppos)
{
    struct alimci_host *host = NULL;
    char buf[STA_INF_BUF_SZ];
    
    host = p_host_emmc;
    if (host != NULL)
    {    
        memset(buf,0,sizeof(buf));
        alimci_get_status(host,buf);
        return simple_read_from_buffer(ubuf, size, ppos, buf,strlen(buf));
    }
    else
    {
        return 0;
    }
}
#else
int alimci_procfile_emmc_ifo_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    struct alimci_host *host = NULL;
    char buf[STA_INF_BUF_SZ];
    
    host = p_host_emmc;
    if (host != NULL)
    {    
        memset(buf,0,sizeof(buf));
        alimci_get_status(host,buf);
        return snprintf(page, count, buf);
    }
    else
    {
        return 0;
    }
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
ssize_t alimci_procfile_emmc_ctrl_read(struct file *file, char __user *ubuf, size_t size, loff_t *ppos)
{
    struct alimci_host *host = NULL;
    static const char message[256];
    host = p_host_emmc;
    if (host != NULL)
    {    
        snprintf(message, 256, "DbgOut=%d,ForceBitWidth=%d\n",
                                 host->DbgOut, host->ForceBitWidth);
        return simple_read_from_buffer(ubuf, size, ppos, message, strlen(message));
    }
    else
    {
        return 0;
    }
}
#else
int alimci_procfile_emmc_ctrl_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    struct alimci_host *host = NULL;
    
    host = p_host_emmc;
    if (host != NULL)
    {    
        return snprintf(page, count, "DbgOut=%d,ForceBitWidth=%d\n",
                                 host->DbgOut, host->ForceBitWidth);
    }
    else
    {
        return 0;
    }
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static ssize_t alimci_procfile_emmc_ctrl_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
#else
int alimci_procfile_emmc_ctrl_write(struct file *filp, const char __user *buffer, unsigned long count, void *data)
#endif
{
    char buf[128];
    int DbgOut = 0;
    int ForceBitWidth = FORCE_BW_OFF;
    struct alimci_host *host = NULL;

    host = p_host_emmc;

    if (host != NULL)
    {  
        if(count<=0)
            return 0;

        memset(buf,0,sizeof(buf));
        if (copy_from_user(buf, buffer, count))  
            return -EFAULT;  

        if (sscanf(buf, "DbgOut=%d,ForceBitWidth=%d", &DbgOut,&ForceBitWidth) != 2)
            return 0;

        if (ALIMCI_OFF == DbgOut || ALIMCI_ON == DbgOut)
            host->DbgOut = DbgOut;

        if (FORCE_BW_1_BIT == ForceBitWidth || FORCE_BW_4_BIT == ForceBitWidth)
            host->ForceBitWidth = ForceBitWidth;

        return count;
    }
    else
    {
        return 0;
    }
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static const struct file_operations alimci_sd_proc_info_fops = {
	.read = alimci_procfile_sd_ifo_read,
	.llseek = default_llseek,
};

static const struct file_operations alimci_sd_proc_ctrl_fops = {
	.read = alimci_procfile_sd_ctrl_read,
	.write = alimci_procfile_sd_ctrl_write,
	.llseek = default_llseek,
};

static const struct file_operations alimci_emmc_proc_info_fops = {
	.read = alimci_procfile_emmc_ifo_read,
	.llseek = default_llseek,
};

static const struct file_operations alimci_emmc_proc_ctrl_fops = {
	.read = alimci_procfile_emmc_ctrl_read,
	.write = alimci_procfile_emmc_ctrl_write,
	.llseek = default_llseek,
};

#endif


/*====================================================================*/
/* Support for /proc/emmc */
#ifdef CONFIG_PROC_FS

static struct proc_dir_entry *proc_emmc=NULL;

#define GPT_BLOCK_SIZE 512

struct blk_data {
	spinlock_t	lock;
	struct gendisk	*disk;
};

static int proc_show_emmc(struct seq_file *file, void *v)
{
    struct alimci_host *host = NULL;
    struct mmc_card *card;
    struct blk_data *md;
    struct gendisk *disk;
    int i;

    host = p_host_emmc;
    if (host == NULL)
        return 0;

    card = host->base_mmc->card;
    if (card == NULL)
        return 0;
    
    md = mmc_get_drvdata(card);
    if (md == NULL)
        return 0;

    disk = md->disk;
    if (disk == NULL)
        return 0;

	seq_puts(file, "dev:    size   erasesize  name\n");
  
    for (i=1; i<disk->part_tbl->len; i++)
    {
        seq_printf(file, "mmcblk0p%d: %8.8llx %8.8x \"%s\"\n", i,
            disk->part_tbl->part[i]->nr_sects*GPT_BLOCK_SIZE,
            GPT_BLOCK_SIZE,
            disk->part_tbl->part[i]->info->volname);
    }

    return 0;
}

static int proc_emmc_open(struct inode *inode, struct file *file)
{
    single_open(file, proc_show_emmc, NULL); 
    return 0;
}

static const struct file_operations proc_emmc_ops = {
	.open		= proc_emmc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif /* CONFIG_PROC_FS */

static int  alimci_dbg_procfs_init(struct alimci_host *host)
{
    char *p_alimci_dbg_proc_dir = NULL;
    
    if (FORCE_SD_ACTIVE == host->enum_host_switch_type)
    {
        p_alimci_dbg_proc_dir = ALIMCI_DBG_PROC_SD_DIR;
    }
    else
    {
        p_alimci_dbg_proc_dir = ALIMCI_DBG_PROC_EMMC_DIR;      
    }

    host->alimci_proc_dir = proc_mkdir(p_alimci_dbg_proc_dir, NULL);  

    if (host->alimci_proc_dir == NULL) 
    {  
        alimci_err("fail to create dir '%s' !\n", p_alimci_dbg_proc_dir);
        return -1;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
    if (FORCE_SD_ACTIVE == host->enum_host_switch_type)
    {
    	host->alimci_proc_info = proc_create(ALIMCI_DBG_PROC_INFO,0444,host->alimci_proc_dir, &alimci_sd_proc_info_fops);
    }
    else
    {
    	host->alimci_proc_info = proc_create(ALIMCI_DBG_PROC_INFO,0444,host->alimci_proc_dir, &alimci_emmc_proc_info_fops);
    }  
#else
    host->alimci_proc_info = create_proc_entry(ALIMCI_DBG_PROC_INFO, 0444, host->alimci_proc_dir);
#endif
    if(host->alimci_proc_info == NULL)
    {
        remove_proc_entry(p_alimci_dbg_proc_dir, NULL);
        alimci_err("could not initialize /proc/%s/%s\n", p_alimci_dbg_proc_dir,  ALIMCI_DBG_PROC_INFO);
        return -1;
    }

    alimci_log("/proc/%s/%s is created successfully.\n", p_alimci_dbg_proc_dir,  ALIMCI_DBG_PROC_INFO);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
    if (FORCE_SD_ACTIVE == host->enum_host_switch_type)
    {
    	host->alimci_proc_ctrl = proc_create(ALIMCI_DBG_PROC_CTRL,0644,host->alimci_proc_dir, &alimci_sd_proc_ctrl_fops);
    }
    else
    {
    	host->alimci_proc_ctrl = proc_create(ALIMCI_DBG_PROC_CTRL,0644,host->alimci_proc_dir, &alimci_emmc_proc_ctrl_fops);
    }  
#else
    host->alimci_proc_ctrl = create_proc_entry(ALIMCI_DBG_PROC_CTRL,0644,host->alimci_proc_dir);
#endif
    if(host->alimci_proc_ctrl == NULL)
    {
       remove_proc_entry(ALIMCI_DBG_PROC_INFO, host->alimci_proc_dir);
       remove_proc_entry(p_alimci_dbg_proc_dir, NULL);
       alimci_err("could not initialize /proc/%s/%s\n", p_alimci_dbg_proc_dir, ALIMCI_DBG_PROC_CTRL);
       return -1;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
#else
    if (FORCE_SD_ACTIVE == host->enum_host_switch_type)
    {
        host->alimci_proc_info->read_proc = alimci_procfile_sd_ifo_read;
        host->alimci_proc_info->write_proc = NULL;

        host->alimci_proc_ctrl->read_proc = alimci_procfile_sd_ctrl_read;
        host->alimci_proc_ctrl->write_proc = alimci_procfile_sd_ctrl_write;
    }
    else
    {
        host->alimci_proc_info->read_proc = alimci_procfile_emmc_ifo_read;
        host->alimci_proc_info->write_proc = NULL;

        host->alimci_proc_ctrl->read_proc = alimci_procfile_emmc_ctrl_read;
        host->alimci_proc_ctrl->write_proc = alimci_procfile_emmc_ctrl_write;
    }
#endif
    alimci_log("/proc/%s/%s is created successfully.\n", p_alimci_dbg_proc_dir,  ALIMCI_DBG_PROC_CTRL);

#ifdef CONFIG_PROC_FS
    if (FORCE_EMMC_ACTIVE == host->enum_host_switch_type)
    {    
        proc_emmc = proc_create("emmc", 0, NULL, &proc_emmc_ops);
    }
#endif /* CONFIG_PROC_FS */
    
    return 0;
}

static void alimci_dbg_procfs_exit(struct alimci_host *host)
{
    //struct alimci_host *host = (struct alimci_host *)p_host;
    char *p_alimci_dbg_proc_dir = NULL;
    
    if (FORCE_SD_ACTIVE == host->enum_host_switch_type)
    {
        p_alimci_dbg_proc_dir = ALIMCI_DBG_PROC_SD_DIR;
    }
    else
    {
        p_alimci_dbg_proc_dir = ALIMCI_DBG_PROC_EMMC_DIR;      
    }

    remove_proc_entry(ALIMCI_DBG_PROC_CTRL, host->alimci_proc_dir);
    remove_proc_entry(ALIMCI_DBG_PROC_INFO, host->alimci_proc_dir);
    remove_proc_entry(p_alimci_dbg_proc_dir, NULL);
    alimci_log("/proc/%s/%s has been removed.\n",p_alimci_dbg_proc_dir, ALIMCI_DBG_PROC_INFO);
    alimci_log("/proc/%s/%s has been removed.\n",p_alimci_dbg_proc_dir, ALIMCI_DBG_PROC_CTRL);

#ifdef CONFIG_PROC_FS
    if (FORCE_EMMC_ACTIVE == host->enum_host_switch_type)
    {    
        if (proc_emmc)
            remove_proc_entry( "emmc", NULL);
    }
#endif /* CONFIG_PROC_FS */
    
}

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
    alimci_log("alimci_set_dmar_addr(0x%x)\n",__LINE__,addr);
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
    addr = __CTDADDRALI(virt_to_phys((void*)addr));
    ALI_MCI_WR32(SD_DMA_W_ADDR, addr);
}

//set dmaw size
static void alimci_set_dmaw_size(u32 size)
{
    ALI_MCI_WR32(SD_DMA_W_SIZE, size);
}

//Just for sdio card
static int alimci_set_bus_width(unsigned char bus_width)
{
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
static void alimci_host_reset(struct mmc_host *mmc)
{
    struct alimci_host *host = mmc_priv(mmc);

	alimci_log("enter\n");

    if (M3921 == host->ChipId || M3821 == host->ChipId)
    {
        ALI_SOC_WR32(LOCAL_DEV_RESET_CTRL_REG, ALI_SOC_RD32(LOCAL_DEV_RESET_CTRL_REG) | SDIO_SW_RESET);
        mdelay(1);
        ALI_SOC_WR32(LOCAL_DEV_RESET_CTRL_REG, ALI_SOC_RD32(LOCAL_DEV_RESET_CTRL_REG) & (~SDIO_SW_RESET));
        return ;
    }
}


/*
* on 3921A, the sd and emmc share only one hardware controller, when sd reset the controller, the emmc may be abnormal, so protect it by mutex_lock
*/
static void alimci_host_reset_locked(struct mmc_host *mmc)
{
    struct alimci_host *host = mmc_priv(mmc);

	alimci_log("enter\n");
	
    if (M3921 == host->ChipId || M3821 == host->ChipId)
    {    

	   	if (M3921 == host->ChipId)
	    {
	        mutex_lock(&ali_sto_mutex);
	    }
		
        ALI_SOC_WR32(LOCAL_DEV_RESET_CTRL_REG, ALI_SOC_RD32(LOCAL_DEV_RESET_CTRL_REG) | SDIO_SW_RESET);
        mdelay(1);
        ALI_SOC_WR32(LOCAL_DEV_RESET_CTRL_REG, ALI_SOC_RD32(LOCAL_DEV_RESET_CTRL_REG) & (~SDIO_SW_RESET));


		if (M3921 == host->ChipId)
		{
			mutex_unlock(&ali_sto_mutex);
		}

		return ;
    }
	
}

static void alimci_hw_gating(struct mmc_host *mmc,unsigned char gating)
{
	struct alimci_host *host = mmc_priv(mmc);

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
				//ALI_SOC_WR32(PAD_DRVING_CTRL_REG1, ALI_SOC_RD32(PAD_DRVING_CTRL_REG1) | SDIO_10MA);
				ALI_SOC_WR32(PAD_DRVING_CTRL_REG1, ALI_SOC_RD32(PAD_DRVING_CTRL_REG1) | SDIO_5MA);
			}
#endif
		}
		return ;
	}
}


static void alimci_sd_emmc_pinmux_restore(struct alimci_host *host, int callline)
{  
	alimci_log("pinmux restore enter\n"); 	  

#if defined(CONFIG_ARM)    
    if (M3921 == host->ChipId)
    {
        /*
        0x1800008c->bit 5: EMMC_BOOT_SEL
        0x1800008c->bit 28:EMMC_SEL2
        0X18000088->bit 5: EMMC_SEL3
		*/
		ALI_SOC_WR32(SOC_PINMUX_REG2, ALI_SOC_RD32(SOC_PINMUX_REG2)&(~ALI_PINMUX_CTRL_BIT5));
		ALI_SOC_WR32(SOC_PINMUX_REG2, ALI_SOC_RD32(SOC_PINMUX_REG2)&(~ALI_PINMUX_CTRL_BIT28));
		ALI_SOC_WR32(SOC_PINMUX_REG1, ALI_SOC_RD32(SOC_PINMUX_REG1)&(~ALI_PINMUX_CTRL_BIT5));
		
		mutex_unlock(&ali_sto_mutex);
    }
#elif defined(CONFIG_MIPS)
    if (host->pinmux3)
    {
        ALI_SOC_WR32(SOC_PINMUX_REG3, host->pinmux3);
    }
#endif
}


static void alimci_sd_emmc_pinmux_set(struct alimci_host *host,  int callline)
{
    u32 PinMuxCtrl1;
    u32 PinMuxCtrl2;

    //PinMux selection
    if (M3921 == host->ChipId)
    {
        mutex_lock(&ali_sto_mutex);
    }

    p_current_host = host;
	
    //read current PinMux selection
    PinMuxCtrl1 = ALI_SOC_RD32(SOC_PINMUX_REG1);
    PinMuxCtrl2 = ALI_SOC_RD32(SOC_PINMUX_REG2);
    if (0 == host->pinmux1) //save current value
    {
        host->pinmux1 = PinMuxCtrl1;
        host->pinmux2 = PinMuxCtrl2;
    }

	if (M3921_S == host->ChipVer)     //S3921A
	{     
	    if (FORCE_EMMC_ACTIVE == host->enum_host_switch_type)  // eMMC card
	    {
		    /* disable nand flash select */
	        PinMuxCtrl1 = PinMuxCtrl1 & (~NF_FUNC_SEL);
			
	        PinMuxCtrl2 = PinMuxCtrl2 & (~STRAPIN_SEL_ENABLE);
	        PinMuxCtrl2 = PinMuxCtrl2 & (~eMMC_FUNC_SEL);
	        PinMuxCtrl2 = PinMuxCtrl2 | SD_FUNC_SEL;
	    }
	    else                                                   // SD card
	    {     
	        /* disable nand flash select */
	        PinMuxCtrl1 = PinMuxCtrl1 & (~NF_FUNC_SEL);
			
	        PinMuxCtrl2 = PinMuxCtrl2 & (~STRAPIN_SEL_ENABLE);    
	        PinMuxCtrl2 = PinMuxCtrl2 & (~eMMC_FUNC_SEL);
	        PinMuxCtrl2 = PinMuxCtrl2 | SD_FUNC_SEL;
	    }
	
	}
    else if(M3921_C == host->ChipVer)  //C3921A
    {     
	    if (FORCE_EMMC_ACTIVE == host->enum_host_switch_type)  //eMMC card
	    {
			ALI_SOC_WR32(0x438, ALI_SOC_RD32(0x438)&(~(1<<29))); 
			ALI_SOC_WR32(0x43C, ALI_SOC_RD32(0x43C)&(~((1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11)))); 

	        PinMuxCtrl2&=~ALI_PINMUX_CTRL_BIT29;
			PinMuxCtrl2&=~ALI_PINMUX_CTRL_BIT31;
			PinMuxCtrl2|=ALI_PINMUX_CTRL_BIT5;	

	    }
	    else                                                    //SD card
	    {
		   if(M3921_QFP256 == host->ChipPkg)  //QFP
		   {
#ifdef SD_PIN_SHARE_WITH_NAND
				ALI_SOC_WR32(0x438, ALI_SOC_RD32(0x438)&(~(1<<30)));
				ALI_SOC_WR32(0x43C, ALI_SOC_RD32(0x43C)&(~((1<<3)|(1<<5)|(1<<7)|(1<<8)|(1<<9)))); 
		 
				PinMuxCtrl1 &= ~(ALI_PINMUX_CTRL_BIT3);
				PinMuxCtrl1 |= ALI_PINMUX_CTRL_BIT23;
						
				PinMuxCtrl2 &= ~(ALI_PINMUX_CTRL_BIT1|ALI_PINMUX_CTRL_BIT18|ALI_PINMUX_CTRL_BIT19|ALI_PINMUX_CTRL_BIT20
						|ALI_PINMUX_CTRL_BIT21|ALI_PINMUX_CTRL_BIT28|ALI_PINMUX_CTRL_BIT31);
		       	PinMuxCtrl2 |= ALI_PINMUX_CTRL_BIT5;
#else
			    ALI_SOC_WR32(0x430, ALI_SOC_RD32(0x430)&(~((1<<11)|(1<<12)))); 
				ALI_SOC_WR32(0x434, ALI_SOC_RD32(0x434)&(~((1<<9)|(1<<10)|(1<<11)|(1<<12)))); 
				
				PinMuxCtrl1 &= ~(ALI_PINMUX_CTRL_BIT1|ALI_PINMUX_CTRL_BIT17|ALI_PINMUX_CTRL_BIT18|ALI_PINMUX_CTRL_BIT23);
				PinMuxCtrl1 |= ALI_PINMUX_CTRL_BIT5;
						
				PinMuxCtrl2 &= ~(ALI_PINMUX_CTRL_BIT1|ALI_PINMUX_CTRL_BIT18|ALI_PINMUX_CTRL_BIT20|ALI_PINMUX_CTRL_BIT21
						|ALI_PINMUX_CTRL_BIT22|ALI_PINMUX_CTRL_BIT26|ALI_PINMUX_CTRL_BIT29|ALI_PINMUX_CTRL_BIT31);
#endif


		   }
		   else                                           //BGA
		   {
			   ALI_SOC_WR32(0x434, ALI_SOC_RD32(0x434)&(~((1<<5)|(1<<10)))); 
			   ALI_SOC_WR32(0x434, ALI_SOC_RD32(0x434)&(~((1<<14)|(1<<16)|(1<<17)|(1<<19)))); 
				   
			   PinMuxCtrl1 &= ~(ALI_PINMUX_CTRL_BIT1|ALI_PINMUX_CTRL_BIT17);
			   PinMuxCtrl1 &= ~ALI_PINMUX_CTRL_BIT5;

			   PinMuxCtrl2 &= ~(ALI_PINMUX_CTRL_BIT1|ALI_PINMUX_CTRL_BIT18|ALI_PINMUX_CTRL_BIT20|ALI_PINMUX_CTRL_BIT26
					   |ALI_PINMUX_CTRL_BIT29|ALI_PINMUX_CTRL_BIT31);
			   PinMuxCtrl2|=ALI_PINMUX_CTRL_BIT28;

		   }
		}
    }
	
    ALI_SOC_WR32(SOC_PINMUX_REG1, PinMuxCtrl1); 
	ALI_SOC_WR32(SOC_PINMUX_REG2, PinMuxCtrl2); 
				
	alimci_log("pinmux1:0x%x, pinmux2:0x%x\n", ALI_SOC_RD32(SOC_PINMUX_REG1), ALI_SOC_RD32(SOC_PINMUX_REG2));

    return ;
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
    struct mmc_request *mrq = host->mrq;

    if (host->LstCmd55 && cmd == mrq->cmd->opcode)
        ret = TRUE;

    return ret;
}

static void alimci_preprocess(struct alimci_host *host, struct mmc_command *cmd)
{
    switch (cmd->opcode)
    {
        case MMC_GO_IDLE_STATE:
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0)
		//if (ALI_SDIO_MT_UNKNOWN == alimci_get_media_type())
		{
			alimci_log("\n#########Line:%d--- Reset host during MediaDetecting#########\n\n",__LINE__);
			alimci_host_reset(host->base_mmc);
			mdelay(1);
		}
#endif
            alimci_change_clk(host->InitClkDiv);
            host->bus_width = MMC_BUS_WIDTH_1;
            alimci_set_bus_width(host->bus_width);
			host->clkdiv_restore = host->InitClkDiv;//save for restore
			host->bus_width_restore = host->bus_width;//save for restore
            break;

        default:
			alimci_change_clk(host->clkdiv_restore);  //restore
            alimci_set_bus_width(host->bus_width_restore); //restore
            break;
    }
}

static void alimci_start_cmd(struct alimci_host *host, struct mmc_command *cmd)
{
    u8 ctr_reg = ALI_SEND_CMD;
    u8 cmd_reg = (cmd->opcode & SD_CMD_IDX_MASK);
    u8 dir = WR_TO_CARD;
    u8 IntMode = 0;
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

    switch (mmc_resp_type(cmd))
    {
        case MMC_RSP_NONE:
            ctr_reg |= ALI_MCI_RSPTYPE_NONE;
            break;
        case MMC_RSP_R1B:
            ctr_reg |= ALI_MCI_RSPTYPE_R1;
            break;
        case MMC_RSP_R2:
            ctr_reg |= ALI_MCI_RSPTYPE_R2;
            break;
        default:
            switch (cmd->opcode)
            {
#if !defined(CONFIG_CMD_MMC) //if NOT used in u-boot
                case MMC_FAST_IO:
                case MMC_GO_IRQ_STATE:
                case SD_IO_SEND_OP_COND:
                case SD_IO_RW_DIRECT:
                case SD_IO_RW_EXTENDED:
#endif
                case SD_SEND_RELATIVE_ADDR:
                case SD_SEND_IF_COND:
                    ctr_reg |= ALI_MCI_RSPTYPE_R4_7;
                    break;

                case MMC_SEND_OP_COND:
                case MMC_SPI_READ_OCR:
                    ctr_reg |= ALI_MCI_RSPTYPE_R3;
                    break;

                default:
                    if (alimci_is_app_cmd(host,SD_APP_OP_COND))
                        ctr_reg |= ALI_MCI_RSPTYPE_R3;
                    else
                        ctr_reg |= ALI_MCI_RSPTYPE_R1;
                    break;
            }
            break;
    }

    switch (mmc_cmd_type(cmd))
    {
        case MMC_CMD_BC:
            ctr_reg |= ALI_MCI_CMDTYPE_BC;
            break;
        case MMC_CMD_BCR:
            ctr_reg |= ALI_MCI_CMDTYPE_BCR;
            break;
        case MMC_CMD_AC:
            ctr_reg |= ALI_MCI_CMDTYPE_AC;
            break;
        case MMC_CMD_ADTC:
            data = cmd->data;
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
            break;
        default:
            dev_warn(mmc_dev(host->base_mmc),
                     "%s: unknown MMC command\n", __func__);
            break;
    }

    cmd_reg |= dir;

    //Only SD2.0 and plus card response to SD_SEND_IF_COND
    //Only MMC/eMMC response to MMC_SEND_OP_COND
    //so I'm using Polling method to handle these 2 cmds.
    //if (SD_SEND_IF_COND == cmd->opcode && MMC_CMD_ADTC != mmc_cmd_type(cmd))
    if (SD_SEND_IF_COND == cmd->opcode && !(cmd->data))
    {
        IntMode = SD_INT_DISABLE;
    }
    else if (MMC_SEND_OP_COND == cmd->opcode)
    {
        IntMode = SD_INT_DISABLE;
    }
    else
    {
        IntMode = SD_INT_ENABLE;
    }
    cmd_reg |= IntMode;

#if defined(CONFIG_ARM)
    alimci_log("\n\n--SD_STATUS_REG:0x%08x----%s%d---ctr_reg = 0x%02x, cmd_reg = 0x%02x, arg_reg = 0x%08x--RspType:%d--Pinmux1:0x%08x,Pinmux2:0x%08x,SD_BITCTRL_REG=0x%x----host->data_op_type=%d,CurClkDiv: 0x%04x ------------\n",
	ALI_MCI_RD8(SD_STATUS_REG),
host->LstCmd55 ? "ACMD" : "CMD",
            cmd->opcode,
            ctr_reg, 
            cmd_reg, 
            cmd->arg,
            (ctr_reg >> 4) & 0x7,
            ALI_SOC_RD32(SOC_PINMUX_REG1),
            ALI_SOC_RD32(SOC_PINMUX_REG2),
            ALI_MCI_RD8(SD_BITCTRL_REG),
            host->data_op_type,
            alimci_get_clk());
#elif defined(CONFIG_MIPS)
    alimci_log("\n\n------%s%d---ctr_reg = 0x%02x, cmd_reg = 0x%02x, arg_reg = 0x%08x--RspType:%d--Pinmux1:0x%08x,Pinmux2:0x%08x,Pinmux3:0x%08x,Pinmux4:0x%08x,SD_BITCTRL_REG=0x%x----host->data_op_type=%d,CurClkDiv: 0x%04x ------------\n",
            host->LstCmd55 ? "ACMD" : "CMD",
            cmd->opcode,
            ctr_reg, 
            cmd_reg, 
            cmd->arg,
            (ctr_reg >> 4) & 0x7,
            ALI_SOC_RD32(SOC_PINMUX_REG1),
            ALI_SOC_RD32(SOC_PINMUX_REG2),
            ALI_SOC_RD32(SOC_PINMUX_REG3),
            ALI_SOC_RD32(SOC_PINMUX_REG4),
            ALI_MCI_RD8(SD_BITCTRL_REG),
            host->data_op_type,
            alimci_get_clk());
#endif


    if (!IntMode)  //cmd to polling
    {
		alimci_write_cmd(ctr_reg, cmd_reg, cmd->arg);
        alimci_status_polling(host,cmd);
    }
	else  //cmd to wait for irq
	{ 
	    //start up a timer to check irq timeout or not
	    host->active_cmd = cmd;
		mod_timer(&host->waitfor_irq_timer, jiffies +  msecs_to_jiffies(WAIT_IRQ_TIMEOUT_MS));		

		//³õÊ¼»¯wait queue
		wait_event_flag = FALSE;	
		init_waitqueue_head(&host->finish_wq);

		//write cmd register
		alimci_write_cmd(ctr_reg, cmd_reg, cmd->arg);

		//wait for tasklet return
		wait_event(host->finish_wq,wait_event_flag);

		//mutex_unlock and return done to block layer
		alimci_finish_request(host,host->finish_status,host->finish_callline);
    }
}


static void alimci_start_cmd_level2(struct alimci_host *host, struct mmc_command *cmd)
{
    u8 ctr_reg = ALI_SEND_CMD;
    u8 cmd_reg = (cmd->opcode & SD_CMD_IDX_MASK);
    u8 dir = WR_TO_CARD;
    u8 IntMode = 0;
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

    switch (mmc_resp_type(cmd))
    {
        case MMC_RSP_NONE:
            ctr_reg |= ALI_MCI_RSPTYPE_NONE;
            break;
        case MMC_RSP_R1B:
            ctr_reg |= ALI_MCI_RSPTYPE_R1;
            break;
        case MMC_RSP_R2:
            ctr_reg |= ALI_MCI_RSPTYPE_R2;
            break;
        default:
            switch (cmd->opcode)
            {
#if !defined(CONFIG_CMD_MMC) //if NOT used in u-boot
                case MMC_FAST_IO:
                case MMC_GO_IRQ_STATE:
                case SD_IO_SEND_OP_COND:
                case SD_IO_RW_DIRECT:
                case SD_IO_RW_EXTENDED:
#endif
                case SD_SEND_RELATIVE_ADDR:
                case SD_SEND_IF_COND:
                    ctr_reg |= ALI_MCI_RSPTYPE_R4_7;
                    break;

                case MMC_SEND_OP_COND:
                case MMC_SPI_READ_OCR:
                    ctr_reg |= ALI_MCI_RSPTYPE_R3;
                    break;

                default:
                    if (alimci_is_app_cmd(host,SD_APP_OP_COND))
                        ctr_reg |= ALI_MCI_RSPTYPE_R3;
                    else
                        ctr_reg |= ALI_MCI_RSPTYPE_R1;
                    break;
            }
            break;
    }

    switch (mmc_cmd_type(cmd))
    {
        case MMC_CMD_BC:
            ctr_reg |= ALI_MCI_CMDTYPE_BC;
            break;
        case MMC_CMD_BCR:
            ctr_reg |= ALI_MCI_CMDTYPE_BCR;
            break;
        case MMC_CMD_AC:
            ctr_reg |= ALI_MCI_CMDTYPE_AC;
            break;
        case MMC_CMD_ADTC:
            data = cmd->data;
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
            break;
        default:
            dev_warn(mmc_dev(host->base_mmc),
                     "%s: unknown MMC command\n", __func__);
            break;
    }

    cmd_reg |= dir;

    //Only SD2.0 and plus card response to SD_SEND_IF_COND
    //Only MMC/eMMC response to MMC_SEND_OP_COND
    //so I'm using Polling method to handle these 2 cmds.
    //if (SD_SEND_IF_COND == cmd->opcode && MMC_CMD_ADTC != mmc_cmd_type(cmd))
    if (SD_SEND_IF_COND == cmd->opcode && !(cmd->data))
    {
        IntMode = SD_INT_DISABLE;
    }
    else if (MMC_SEND_OP_COND == cmd->opcode)
    {
        IntMode = SD_INT_DISABLE;
    }
    else
    {
        IntMode = SD_INT_ENABLE;
    }
    cmd_reg |= IntMode;

#if defined(CONFIG_ARM)
    alimci_log("\n\n--SD_STATUS_REG:0x%08x----%s%d---ctr_reg = 0x%02x, cmd_reg = 0x%02x, arg_reg = 0x%08x--RspType:%d--Pinmux1:0x%08x,Pinmux2:0x%08x,SD_BITCTRL_REG=0x%x----host->data_op_type=%d,CurClkDiv: 0x%04x ------------\n",
	ALI_MCI_RD8(SD_STATUS_REG),
host->LstCmd55 ? "ACMD" : "CMD",
            cmd->opcode,
            ctr_reg, 
            cmd_reg, 
            cmd->arg,
            (ctr_reg >> 4) & 0x7,
            ALI_SOC_RD32(SOC_PINMUX_REG1),
            ALI_SOC_RD32(SOC_PINMUX_REG2),
            ALI_MCI_RD8(SD_BITCTRL_REG),
            host->data_op_type,
            alimci_get_clk());
#elif defined(CONFIG_MIPS)
    alimci_log("\n\n------%s%d---ctr_reg = 0x%02x, cmd_reg = 0x%02x, arg_reg = 0x%08x--RspType:%d--Pinmux1:0x%08x,Pinmux2:0x%08x,Pinmux3:0x%08x,Pinmux4:0x%08x,SD_BITCTRL_REG=0x%x----host->data_op_type=%d,CurClkDiv: 0x%04x ------------\n",
            host->LstCmd55 ? "ACMD" : "CMD",
            cmd->opcode,
            ctr_reg, 
            cmd_reg, 
            cmd->arg,
            (ctr_reg >> 4) & 0x7,
            ALI_SOC_RD32(SOC_PINMUX_REG1),
            ALI_SOC_RD32(SOC_PINMUX_REG2),
            ALI_SOC_RD32(SOC_PINMUX_REG3),
            ALI_SOC_RD32(SOC_PINMUX_REG4),
            ALI_MCI_RD8(SD_BITCTRL_REG),
            host->data_op_type,
            alimci_get_clk());
#endif


    if (!IntMode)  //cmd to polling
    {
		alimci_write_cmd(ctr_reg, cmd_reg, cmd->arg);
        alimci_status_polling(host,cmd);
    }
	else  //cmd to wait for irq
	{
	    host->active_cmd = cmd;
		mod_timer(&host->waitfor_irq_timer, jiffies +  msecs_to_jiffies(WAIT_IRQ_TIMEOUT_MS));		
		alimci_write_cmd(ctr_reg, cmd_reg, cmd->arg);
    }
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

static dma_addr_t alimci_dma_map(struct alimci_host *host, void *buf, size_t count,enum dma_data_direction direction)
{
	struct device *dev = &(host->pdev->dev);
	dma_addr_t dma_dst;
	int ofs;

	/* Handle vmalloc address */
	if (buf >= high_memory) {
		struct page *page;

		if (((size_t) buf & PAGE_MASK) !=
		    ((size_t) (buf + count - 1) & PAGE_MASK))
			goto normal;
		page = vmalloc_to_page(buf);
		if (!page)
			goto normal;

		/* Page offset */
		ofs = ((size_t) buf & ~PAGE_MASK);
		host->page_dma = TRUE;

		/* DMA routine */
		dma_dst = dma_map_page(dev, page, ofs, count, direction);
	} else {
		/* DMA routine */
		host->page_dma = FALSE;
		dma_dst = dma_map_single(dev, buf, count, direction);
	}
	if (dma_mapping_error(dev, dma_dst)) {
		dev_err(dev, "Couldn't map a %d byte buffer for DMA\n", count);
		goto normal;
	}

	alimci_log("\n#########<%s> Line:%d, high_memory=0x%X,page_dma=%d---Buf:0x%X,Len:%d,Dir:%d#########\n\n",__FUNCTION__,__LINE__,high_memory,host->page_dma,buf,count,direction);
	return dma_dst;

normal:
	alimci_err("Couldn't map a %d byte buffer for DMA\n", count);
	return 0;
}

static void alimci_dma_unmap(struct alimci_host *host, dma_addr_t dma_handle, size_t count,enum dma_data_direction direction)
{
	struct device *dev = &(host->pdev->dev);

	if (host->page_dma)
		dma_unmap_page(dev, dma_handle, count, direction);
	else
		dma_unmap_single(dev, dma_handle, count, direction);
	alimci_log("\n#########<%s> Line:%d, page_dma=%d---Len:%d,Dir:%d#########\n\n",__FUNCTION__,__LINE__,host->page_dma,count,direction);
}

static int alimci_prepare_dma(struct alimci_host *host, struct mmc_data *data)
{
    //int dma_cnt, i;
    int IsWrite = data->flags & MMC_DATA_WRITE;
    //    u32 block_idx = (data->mrq->cmd->arg);
    u32 *p  = (u32 *)(sg_virt(data->sg));
    u32 block_num = data->blocks;
    u32 byte_num;
    u32 block_size;

    BUG_ON((data->flags & BOTH_DIR) == BOTH_DIR);

#if 0
    if ((u32)p % ARCH_DMA_MINALIGN)
    {
        alimci_err("--------Address 0x%x is not 0x%x aligned---------------\n",p, ARCH_DMA_MINALIGN);
        BUG_ON(((u32)p % ARCH_DMA_MINALIGN));
    }
#endif
	
    block_size = data->blksz;
    alimci_set_block_size(block_size);

    alimci_pio_enable(0);   //disable PIO

    byte_num = block_size * block_num;

    alimci_set_block_num(block_num - 1); //register value = blockNum - 1

    if (IsWrite)
    {
        alimci_set_dmaw_addr((u32)p);
        //Sync Cache data to this DRAM address
#if defined(CONFIG_ARM)
        host->dma_handle = alimci_dma_map(host,p,byte_num,DMA_TO_DEVICE);
#endif
        alimci_set_dmaw_size(byte_num);
        host->data_op_type = DATA_OP_WRITE_1;
    }
    else
    {
        alimci_set_dmar_addr((u32)p);
#if defined(CONFIG_ARM)
        host->dma_handle = alimci_dma_map(host,p,byte_num,DMA_FROM_DEVICE);
#endif
        alimci_set_dmar_size(byte_num);
        host->data_op_type = DATA_OP_READ;
    }


    alimci_dma_start(IsWrite ? DMA_WR_TO_CARD : DMA_RD_FROM_CARD);

#if 0
    dma_cnt = dma_map_sg(mmc_dev(host->base_mmc), data->sg, data->sg_len,
                         rw ? DMA_TO_DEVICE : DMA_FROM_DEVICE);

    if (dma_cnt == 0)
    {
        return -ENOMEM;
    }
#endif

    return 0;
}

static void alimci_finish_request(struct alimci_host *host,bool finish, int callline)
{
    struct mmc_request *mrq = host->mrq;
    struct mmc_command *cmd = mrq->cmd;
    if (finish)
    {
        if (cmd->data && cmd->error)
        {
            cmd->data->error = cmd->error;
        }

        //host->complete_what = COMPLETION_NONE;
        host->mrq = NULL;
        host->LstCmd55 = (MMC_APP_CMD == cmd->opcode ? TRUE : FALSE);
    }

    alimci_sd_emmc_pinmux_restore(host, callline);
    mmc_request_done(host->base_mmc, mrq);
}

//clear SD interrupt
static void alimci_clear_interrupt_status(void)
{
    ALI_MCI_WR8(SD_DMA_CTRL, (ALI_MCI_RD8(SD_DMA_CTRL) | SYS_INT_STATUS));
}

static void alimci_postprocess(struct alimci_host *host, struct mmc_command *cmd)
{
    if (alimci_is_app_cmd(host,SD_APP_SET_BUS_WIDTH))
    {
        alimci_set_bus_width(cmd->arg);
        host->bus_width = cmd->arg;
        alimci_change_clk(host->TranClkDiv);
		host->clkdiv_restore = host->TranClkDiv;//save for restore
		host->bus_width_restore = host->bus_width;//save for restore
    }
    else if (MMC_SWITCH == cmd->opcode && MMC_CMD_AC == mmc_cmd_type(cmd)) //SD_SWITCH is 6 too, but it is ADTC
    {
        u8 index;
        u8 value;
        u8 width = 0;

        index = (cmd->arg >> 16) & 0xff;
        value = (cmd->arg >> 8) & 0xff;
        if (EXT_CSD_BUS_WIDTH == index)
        {
            switch (value)
            {
                case EXT_CSD_BUS_WIDTH_8:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
                case EXT_CSD_DDR_BUS_WIDTH_8:
#endif
                    width = MMC_BUS_WIDTH_8;
                    break;

                case EXT_CSD_BUS_WIDTH_4:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
                case EXT_CSD_DDR_BUS_WIDTH_4:
#endif
                    width = MMC_BUS_WIDTH_4;
                    break;

                case EXT_CSD_BUS_WIDTH_1:
                default:
                    width = MMC_BUS_WIDTH_1;
                    break;
            }

            printk(KERN_ALERT"alimci_postprocess,  MMC_SWITCH,  host->bus_width = %d \n", width);
            alimci_set_bus_width(width);
            host->bus_width = width;
            alimci_change_clk(host->TranClkDiv);
			host->clkdiv_restore = host->TranClkDiv;//save for restore
			host->bus_width_restore = host->bus_width;//save for restore
        }
    }
}

static void alimci_status_polling_level2(struct alimci_host *host)
{
    int times = 0;
    u32 rspReg0 = 0;
    struct mmc_request *mrq = host->mrq;
    struct mmc_command *cmd = mrq->cmd;
    u8 status = ALI_MCI_RD8(SD_STATUS_REG);

    ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) | CLK_ALWAYS_EN));

    status = ALI_MCI_RD8(SD_STATUS_REG);

    while (SD_STATUS_OK_MASK != status)
    {
        if (times < 10)
            alimci_log("####<%s> CMD:%d #####---wrong STATUS=0x%x####times=%d#####\n\n",__FUNCTION__,cmd->opcode,status,times);

		times++;
		
        if (times > MAX_WAIT_TIMES)
        {
            mrq->cmd->error = -ETIMEDOUT;
            ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) & ~CLK_ALWAYS_EN));
            alimci_err("#### CMD:%d #####---STATUS=0x%x####times=%d#####\n\n",cmd->opcode,status,times);

			alimci_finish_request(host,FALSE,__LINE__);
            
            if (MMC_SWITCH == cmd->opcode && MMC_CMD_AC == mmc_cmd_type(cmd)) 
            {
                if (FORCE_EMMC_ACTIVE == host->enum_host_switch_type)
                {
                     //host->base_mmc->caps   &= (~MMC_CAP_8_BIT_DATA);  //eMMC doesn't support 8 bits .
                }
                
                printk("---alimci_cmd_tasklet,  MMC_SWITCH,  eMMC does not support  MMC_CAP_8_BIT_DATA\n\n");
            }
            return ;
        }      
        udelay(50);
        status = ALI_MCI_RD8(SD_STATUS_REG);
    };
    ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) & ~CLK_ALWAYS_EN));
    if (times > 10)
        alimci_log("####<%s> CMD:%d #####---STATUS=0x%x####times=%d#####\n\n",__FUNCTION__,cmd->opcode,status,times);

    if (cmd->flags & MMC_RSP_PRESENT)
    {
        if (cmd->flags & MMC_RSP_136)
        {
            /* Read card response from controller. */
            cmd->resp[0] = ALI_MCI_RD32((SD_RESP_REG+12));
            cmd->resp[1] = ALI_MCI_RD32((SD_RESP_REG+8));
            cmd->resp[2] = ALI_MCI_RD32((SD_RESP_REG+4));
            cmd->resp[3] = ALI_MCI_RD32(SD_RESP_REG);
        }
        else
        {        
            rspReg0 = (__REGALIRAW(host->sdio_base + SD_RESP_REG + 1));
            //rspReg0 = (__REGALIRAW(p_host->sdio_base + SD_RESP_REG + 1));
            cmd->resp[0] = GET_UINT32_UNALIGNED(rspReg0);
			
			//added by kinson.zhou for mmc bug: Multi Block Read Command sometimes causes  status bit 31(ADDRESS_OUT_OF_RANGE) to 1, we should ignore it
			if((cmd->resp[0] & R1_OUT_OF_RANGE) && (MMC_READ_MULTIPLE_BLOCK == cmd->opcode))
			{ 
			   cmd->resp[0]&=(~R1_OUT_OF_RANGE);
			   alimci_log("###Warnning:bit of R1_OUT_OF_RANGE is ignored! CMD%d status=0x%08x\n",cmd->opcode,cmd->resp[0]);
			}
        }
    }

    alimci_log("<alimci_cmd_tasklet> Line:%d---%s%d, SD_STATUS_REG=0x%02X---cmd->resp[0]=0x%08x,----%s-----------------\n",
        __LINE__,
        host->LstCmd55 ? "ACMD" : "CMD",
        cmd->opcode,
        status,cmd->resp[0],
        (cmd->flags & MMC_RSP_136) ? "LONG Response" : (cmd->flags & MMC_RSP_PRESENT) ? "Short Response" : "No Reponse"
        );

    if (cmd->flags & MMC_RSP_136)
    {
        alimci_log("===Resp[0]=0x%08x,Resp[1]=0x%08x,Resp[2]=0x%08x,Resp[3]=0x%08x===\n",cmd->resp[0],cmd->resp[1],cmd->resp[2],cmd->resp[3]);
    }

    if (SD_STATUS_OK_MASK == status)
        alimci_postprocess(host, cmd);
	
    //host->complete_what = COMPLETION_FINALIZE;

	alimci_finish_request(host,TRUE,__LINE__);

}

static void alimci_cmd_tasklet(unsigned long para)
{
    int times = 0;
    u32 rspReg0 = 0;
    struct alimci_host *host = (struct alimci_host *)para;
    struct mmc_request *mrq = host->mrq;
    struct mmc_command *cmd = mrq->cmd;
    u8 status = ALI_MCI_RD8(SD_STATUS_REG);

    ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) | CLK_ALWAYS_EN));

    status = ALI_MCI_RD8(SD_STATUS_REG);

    while (SD_STATUS_OK_MASK != status)
    {
        if (times < 10)
            alimci_log("####<%s> CMD:%d #####---wrong STATUS=0x%x####times=%d#####\n\n",__FUNCTION__,cmd->opcode,status,times);
        times++;

        if (times > MAX_WAIT_TIMES)
        {
            mrq->cmd->error = -ETIMEDOUT;
            ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) & ~CLK_ALWAYS_EN));
            alimci_err("#### CMD:%d #####---STATUS=0x%x####times=%d#####\n\n",cmd->opcode,status,times);

			host->finish_status = FALSE;
			host->finish_callline = __LINE__;
			wait_event_flag = TRUE;		
			wake_up(&host->finish_wq);

            if (MMC_SWITCH == cmd->opcode && MMC_CMD_AC == mmc_cmd_type(cmd)) 
            {
                if (FORCE_EMMC_ACTIVE == host->enum_host_switch_type)
                {
                     //host->base_mmc->caps   &= (~MMC_CAP_8_BIT_DATA);  //eMMC doesn't support 8 bits .
                }
                
                printk("---alimci_cmd_tasklet,  MMC_SWITCH,  eMMC does not support  MMC_CAP_8_BIT_DATA\n\n");
            }
            return ;
        }
      
        udelay(50);
        status = ALI_MCI_RD8(SD_STATUS_REG);
    };
    ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) & ~CLK_ALWAYS_EN));
    if (times > 10)
        alimci_log("####<%s> CMD:%d #####---STATUS=0x%x####times=%d#####\n\n",__FUNCTION__,cmd->opcode,status,times);

    if (cmd->flags & MMC_RSP_PRESENT)
    {
        if (cmd->flags & MMC_RSP_136)
        {
            /* Read card response from controller. */
            cmd->resp[0] = ALI_MCI_RD32((SD_RESP_REG+12));
            cmd->resp[1] = ALI_MCI_RD32((SD_RESP_REG+8));
            cmd->resp[2] = ALI_MCI_RD32((SD_RESP_REG+4));
            cmd->resp[3] = ALI_MCI_RD32(SD_RESP_REG);
			
#ifdef CONFIG_ALI_MMC_C3921_PATCH
			if(MMC_SEND_CSD == cmd->opcode)
			{
			  cmd->resp[3] = (cmd->resp[3]>>8)|((cmd->resp[2] & 0xff)<<24);
			  cmd->resp[2] = (cmd->resp[2] & 0xffff0000)|((cmd->resp[2]>>8 & 0xffff));
			}
#endif
        }
        else
        {        
            rspReg0 = (__REGALIRAW(host->sdio_base + SD_RESP_REG + 1));
            //rspReg0 = (__REGALIRAW(p_host->sdio_base + SD_RESP_REG + 1));
            cmd->resp[0] = GET_UINT32_UNALIGNED(rspReg0);
			
			//added by kinson.zhou for mmc bug: Multi Block Read Command sometimes causes  status bit 31(ADDRESS_OUT_OF_RANGE) to 1, we should ignore it
			if((cmd->resp[0] & R1_OUT_OF_RANGE) && (MMC_READ_MULTIPLE_BLOCK == cmd->opcode))
			{ 
			   cmd->resp[0]&=(~R1_OUT_OF_RANGE);
			   alimci_log("###Warnning:bit of R1_OUT_OF_RANGE is ignored! CMD%d status=0x%08x\n",cmd->opcode,cmd->resp[0]);
			}
        }
    }

    alimci_log("<alimci_cmd_tasklet> Line:%d---%s%d, SD_STATUS_REG=0x%02X---cmd->resp[0]=0x%08x,----%s-----------------\n",
        __LINE__,
        host->LstCmd55 ? "ACMD" : "CMD",
        cmd->opcode,
        status,cmd->resp[0],
        (cmd->flags & MMC_RSP_136) ? "LONG Response" : (cmd->flags & MMC_RSP_PRESENT) ? "Short Response" : "No Reponse"
        );

    if (cmd->flags & MMC_RSP_136)
    {
        alimci_log("===Resp[0]=0x%08x,Resp[1]=0x%08x,Resp[2]=0x%08x,Resp[3]=0x%08x===\n",cmd->resp[0],cmd->resp[1],cmd->resp[2],cmd->resp[3]);
    }

    if (SD_STATUS_OK_MASK == status)
        alimci_postprocess(host, cmd);
	
    //host->complete_what = COMPLETION_FINALIZE;
	host->finish_status = TRUE;
	host->finish_callline = __LINE__;
	wait_event_flag = TRUE;		
	wake_up(&host->finish_wq);

}

static void alimci_data_tasklet(unsigned long para)
{
    int times = 0;
    u32 rspReg0 = 0;
    u32 dma_ctrl_reg = 0;
    u32 idle_mask = 0;
    struct alimci_host *host = (struct alimci_host *)para;
    struct mmc_request *mrq = host->mrq;
    struct mmc_command *cmd = mrq->cmd;
    u8 status = ALI_MCI_RD8(SD_STATUS_REG);
    struct mmc_data *data = mrq->data;

    ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) | CLK_ALWAYS_EN));
    status = ALI_MCI_RD8(SD_STATUS_REG);
    while (SD_STATUS_OK_MASK != status)
    {
        if (times < 10)
            alimci_log("####<%s> CMD:%d #####---wrong STATUS=0x%x####times=%d#####\n\n",__FUNCTION__,cmd->opcode,status,times);
        times++;

        if (times > MAX_WAIT_TIMES)
        {
            mrq->cmd->error = -ETIMEDOUT;
            ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) & ~CLK_ALWAYS_EN));
            alimci_err("#### CMD:%d #####---STATUS=0x%x####times=%d#####\n\n",cmd->opcode,status,times);
			
			host->finish_status = FALSE;
			host->finish_callline = __LINE__;
			wait_event_flag = TRUE;		
			wake_up(&host->finish_wq);
            return ;
        }      
        udelay(50);
        status = ALI_MCI_RD8(SD_STATUS_REG);
    };
    ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) & ~CLK_ALWAYS_EN));
    if (times > 10)
        alimci_log("####<%s> CMD:%d #####---STATUS=0x%x####times=%d#####\n\n",__FUNCTION__,cmd->opcode,status,times);

    if (cmd->flags & MMC_RSP_PRESENT)
    {
        if (cmd->flags & MMC_RSP_136)
        {
            /* Read card response from controller. */
            cmd->resp[0] = ALI_MCI_RD32((SD_RESP_REG+12));
            cmd->resp[1] = ALI_MCI_RD32((SD_RESP_REG+8));
            cmd->resp[2] = ALI_MCI_RD32((SD_RESP_REG+4));
            cmd->resp[3] = ALI_MCI_RD32(SD_RESP_REG);
        }
        else
        {
            rspReg0 = (__REGALIRAW(host->sdio_base + SD_RESP_REG + 1));
            //rspReg0 = (__REGALIRAW(p_host->sdio_base + SD_RESP_REG + 1));
            cmd->resp[0] = GET_UINT32_UNALIGNED(rspReg0);
        }
    }

    alimci_log("<alimci_data_tasklet> Line:%d---%s%d, SD_STATUS_REG=0x%02X,DMA_STATUS=0x%08X---cmd->resp[0]=0x%08X,----%s-----------------\n",
        __LINE__,
        host->LstCmd55 ? "ACMD" : "CMD",
        cmd->opcode,
        status,
        ALI_MCI_RD32(SD_DMA_CTRL),
        cmd->resp[0],
        (cmd->flags & MMC_RSP_136) ? "LONG Response" : (cmd->flags & MMC_RSP_PRESENT) ? "Short Response" : "No Reponse"
        );

    if (SD_STATUS_OK_MASK != status)
    {
        if (0 != (status & (CRC16_ERR | CRC7_ERR)))
        {
            data->error = -EILSEQ;
            alimci_err("-----------CRC error,status=0x%02X---------------\n",status);
        }
        else if (0 != (status & (CMD_BUSY | DATA_BUSY)))
        {
            data->error = -ETIMEDOUT;
            alimci_err("-----------BUSY,status=0x%02X---------------\n", status);
        }
        else
        {
            data->error = -EILSEQ;
            alimci_err("-----------Ohter error,status=0x%02X---------------\n", status);
        }
    }

    /*
     * There appears to be a hardware design bug here.  There seems to
     * be no way to find out how much data was transferred to the card.
     * This means that if there was an error on any block, we mark all
     * data blocks as being in error.
     */
    if (!data->error)
    {
        data->bytes_xfered = data->blocks * data->blksz;
    }
    else
    {
        data->bytes_xfered = 0;
    }

    if (DATA_OP_READ == host->data_op_type)  //read irq
    {
        //host->complete_what = COMPLETION_FINALIZE;
        host->data_op_type = NO_DATA_OP;
        if (SD_STATUS_OK_MASK == status && data->bytes_xfered)
        {
#if defined(CONFIG_ARM)
                if (host->dma_handle)
                        alimci_dma_unmap(host,host->dma_handle,data->bytes_xfered,DMA_FROM_DEVICE);
#endif
        }
    }
    else if (DATA_OP_WRITE_2 == host->data_op_type)     //write irq
    {
        dma_ctrl_reg = ALI_MCI_RD32(SD_DMA_CTRL);
        idle_mask = (TX_BUS_IDLE_INT_STATUS | TX_BUS_IDLE_INT_ENABLE);
        alimci_log("<alimci_data_tasklet> Line:%d---SD_STATUS_REG=0x%02X,DMA_STATUS=0x%08x,jiffies = %lu --------------------\n",__LINE__, status,ALI_MCI_RD32(SD_DMA_CTRL),jiffies);
        if ((dma_ctrl_reg & idle_mask) == idle_mask)   //the second irq for write
        {

            host->data_op_type = NO_DATA_OP;

            //disable sd bus idle interrupt
            ALI_MCI_WR32(SD_DMA_CTRL, (ALI_MCI_RD32(SD_DMA_CTRL) & ~TX_BUS_IDLE_INT_ENABLE));
            //clear sd bus idle interrupt status
            ALI_MCI_WR32(SD_DMA_CTRL, (ALI_MCI_RD32(SD_DMA_CTRL) | TX_BUS_IDLE_INT_STATUS));

#if defined(CONFIG_ARM)
		if (host->dma_handle)
			alimci_dma_unmap(host,host->dma_handle,data->bytes_xfered,DMA_TO_DEVICE);
#endif
            //host->complete_what = COMPLETION_FINALIZE;
            //then enter the following send STOP cmd.

        }
    }
    else if (DATA_OP_WRITE_1 == host->data_op_type)      //First irq for write
    {
        host->data_op_type = DATA_OP_WRITE_2;

        alimci_log("<alimci_data_tasklet> Line:%d---SD_STATUS_REG=0x%02X, DMA_STATUS=0x%08x,jiffies = %lu, host->mrq->stop=0x%x--------------------\n",__LINE__,status,ALI_MCI_RD32(SD_DMA_CTRL),jiffies,host->mrq->stop);

        //Enable sd bus idle interrupt
        ALI_MCI_WR32(SD_DMA_CTRL, (dma_ctrl_reg | TX_BUS_IDLE_INT_ENABLE));
        return ;
    }

    //host->data = NULL;
    if (host->mrq->stop)
    {
        alimci_log("****** LINE:%d *****send STOP cmd******\n",__LINE__);
        alimci_start_cmd_level2(host, host->mrq->stop);
    }
    else
    {
        host->finish_status = TRUE;
		host->finish_callline = __LINE__;
		wait_event_flag = TRUE;		
		wake_up(&host->finish_wq);
    }

}


static void alimci_status_polling(struct alimci_host *host, struct mmc_command *cmd)
{
    u8 status = 0;
    unsigned long times = 0;

    do
    {
        times++;
        if (times > MAX_POLLING_TIME)
            break;
        udelay(1);
        ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) | CLK_ALWAYS_EN));
        status = ALI_MCI_RD8(SD_STATUS_REG);
        ALI_MCI_WR8(SD_CLK_FORCE_EN, (ALI_MCI_RD8(SD_CLK_FORCE_EN) & ~CLK_ALWAYS_EN));
        if (1 == times)
            alimci_log("****** LINE:%d *****CMD: %d, status=0x%x,jiffies = %lu******\n",__LINE__,cmd->opcode,status,jiffies);
    }while(SD_STATUS_OK_MASK != status);

    if (SD_STATUS_OK_MASK != status && times > MAX_POLLING_TIME)
    {
        //timeout, maybe this card doesn't support this command.
        alimci_host_reset(host->base_mmc);
        mdelay(1);
        alimci_log("***********excute CMD: %d timeout, status=0x%x***,jiffies = %lu***\n",cmd->opcode,status,jiffies);
        cmd->resp[0] = RESP_4_TIMEOUT;
        if (cmd->flags & MMC_RSP_136)
        {
            cmd->resp[1] = RESP_4_TIMEOUT;
            cmd->resp[2] = RESP_4_TIMEOUT;
            cmd->resp[3] = RESP_4_TIMEOUT;
        }
        cmd->error = -ETIMEDOUT;
        alimci_finish_request(host,TRUE,__LINE__);
        return ;
    }
    alimci_status_polling_level2(host);
}


/*
* on 3921A controller, when some special cmds were sent, there may not return an irq, when irq not coming, this timeout function will be called to finish the request
*/
static void alimci_waitfor_irq_timeout_timer(unsigned long data)
{
	struct alimci_host *host = NULL;
	struct mmc_command *active_cmd = NULL;
	
	host = (struct alimci_host *)data;
    if(NULL == host)
    {
        return;
    }
	active_cmd = host->active_cmd;
    
	if (host->mrq) 
	{
		alimci_err("host:%s, cmd:%d timeout\n", (FORCE_EMMC_ACTIVE == host->enum_host_switch_type)? "emmc" : "sd" ,active_cmd->opcode);

	    //reset host controller
		alimci_host_reset(host->base_mmc);
		mdelay(1);

        if(host->mrq->cmd)
    	{
            //notify error to block layer
    		if (host->mrq->cmd->data) 
    		{
    			host->mrq->cmd->data->error = -ETIMEDOUT;
    		} 
    		else 
    		{
    			host->mrq->cmd->error = -ETIMEDOUT;
    		}
        }

        //finish request and unlock
		host->finish_status = FALSE;
		host->finish_callline = __LINE__;
		wait_event_flag = TRUE;		
		wake_up(&host->finish_wq);
	}
	
	return;

}
    
//sdio cmd or data irq hanlder function
static irqreturn_t alimci_irq(int irq, void *dev_id)
{
    int handled = 0;
    struct alimci_host *host = dev_id;
    unsigned long iflags;

	host = p_current_host;
         
    if (host != NULL)
    {
        spin_lock_irqsave(&host->complete_lock, iflags);
        alimci_clear_interrupt_status();
        //irq comes ,so I delete the irq timeout timer
		del_timer(&host->waitfor_irq_timer);
        if (NO_DATA_OP != host->data_op_type)
        {
            tasklet_schedule(&host->data_tasklet);
        }
        else     //cmd irq
        {
            tasklet_schedule(&host->cmd_tasklet);
        }
        spin_unlock_irqrestore(&host->complete_lock, iflags);
    }
    else
    {
        alimci_err("alimci_irq  host == NULL\n");
    }
    
    handled = 1;
    return IRQ_RETVAL(handled);
}

//handler data cmd, divide into read cmd and write cmd, success return 0, fail return 1
static int alimci_prepare_data(struct alimci_host *host, struct mmc_data *data)
{
    u32 ret = 0;

    alimci_log("<alimci_prepare_data> CMD:%d, %s %s, %d blocks of %d bytes, sg_len=%d,sg_virt(data->sg)=0x%08x, buffer addr 0x20 align:%s \n",
            host->mrq->cmd->opcode,
            (data->flags & MMC_DATA_STREAM) ? "stream" : "block",
            (data->flags & MMC_DATA_WRITE) ? "write" : "read",
            data->blocks, data->blksz,data->sg_len,sg_virt(data->sg),(u32)sg_virt(data->sg)%0x20 ? "No" : "---YES---");

    if (1 != data->sg_len)
    {
        alimci_err("--------ERR: sg_len is NOT equal to 1---------------\n");
        BUG_ON(1 != data->sg_len);
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
static void alimci_send_request(struct mmc_host *mmc)
{
    bool CmdSupported = TRUE;
    struct alimci_host *host = mmc_priv(mmc);
    struct mmc_request *mrq = host->mrq;
    struct mmc_command *cmd = mrq->cmd;

    host->data_op_type = NO_DATA_OP;
    if (cmd->data)
    {
        int res = alimci_prepare_data(host, cmd->data);
        if (res)
        {
            alimci_err("alimci_prepare_data error!\n");
            cmd->error = res;
            cmd->data->error = res;
            alimci_finish_request(host,FALSE,__LINE__);
            return;
        }
    }

    switch (cmd->opcode)
    {
        /* Ali SD controller does not support SDIO now */
        case SD_IO_RW_DIRECT:
        case SD_IO_SEND_OP_COND:
            CmdSupported = FALSE;
            break;

        default:
            // Both MMC_SEND_EXT_CSD and SD_SEND_IF_COND has same value(8), but the previous one is ADTC type.
            //if (SD_SEND_IF_COND == cmd->opcode && MMC_CMD_BCR == mmc_cmd_type(cmd)) //cmd8 for SD2.0 and plus
            if (SD_SEND_IF_COND == cmd->opcode && !(cmd->data)) //cmd8 for SD2.0 and plus
            {
                if (ALI_SDIO_MT_UNKNOWN != alimci_get_media_type(mmc) && ALI_SDIO_MT_SD20PLUS != alimci_get_media_type(mmc))
                {
                    CmdSupported = FALSE;
                }
            }
            //else if (MMC_SEND_EXT_CSD == cmd->opcode && MMC_CMD_ADTC == mmc_cmd_type(cmd)) //cmd8 for MMC/eMMC
            else if (MMC_SEND_EXT_CSD == cmd->opcode && cmd->data) //cmd8 for MMC/eMMC
            {
                if (ALI_SDIO_MT_eMMC != alimci_get_media_type(mmc) && ALI_SDIO_MT_MMC != alimci_get_media_type(mmc))
                {
                    CmdSupported = FALSE;
                }
            }
            else if (MMC_APP_CMD == cmd->opcode) //cmd55 is NOT supported by MMC/eMMC?
            {
                if (ALI_SDIO_MT_eMMC == alimci_get_media_type(mmc) || ALI_SDIO_MT_MMC == alimci_get_media_type(mmc))
                {
                    CmdSupported = FALSE;
                }
            }
            else if (alimci_is_app_cmd(host,SD_APP_SET_BUS_WIDTH))
            {
			if (FORCE_BW_1_BIT == host->ForceBitWidth  && SD_BUS_WIDTH_4 == cmd->arg)
			{
				cmd->arg = SD_BUS_WIDTH_1;
				alimci_log("SD card: Force to 1 bit mode!");
			}
            }
            else if (MMC_SWITCH == cmd->opcode && !(cmd->data)) //SD_SWITCH is 6 too, but it is ADTC
            {
			u8 index = (cmd->arg >> 16) & 0xff;
			if (EXT_CSD_BUS_WIDTH == index)
			{
				u8 value = (cmd->arg >> 8) & 0xff;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
				if (EXT_CSD_BUS_WIDTH_8 == value ||EXT_CSD_DDR_BUS_WIDTH_8 == value)
#else
				if (EXT_CSD_BUS_WIDTH_8 == value)
#endif
				{
			            //NOTE: Ali SD slot only has 4 lines connected. so SD/MMC card doesn't support 8 bit mode. But eMMC does support 8 bit mode.

                    if (FORCE_EMMC_ACTIVE == host->enum_host_switch_type)
                    {
                                        //if (ALI_SDIO_MT_eMMC != alimci_get_media_type())
                        if (ALI_SDIO_MT_eMMC != alimci_get_media_type(mmc) && ALI_SDIO_MT_MMC != alimci_get_media_type(mmc))
					    {
					        CmdSupported = FALSE;
			            }
                    }
                    else
                    {
                        CmdSupported = FALSE;
                    }
				}
				else if (FORCE_BW_1_BIT == host->ForceBitWidth  && EXT_CSD_BUS_WIDTH_1 != value)
				{
					CmdSupported = FALSE;
					alimci_log("MMC card: Force to 1-bit mode,but this request (%d) is NOT 1-bit mode!",value);
				}
			}
            }
    }

    if (CmdSupported)
    {
        alimci_start_cmd(host, cmd);
    }
    else
    {     
        alimci_log("---------Non-supported CMD: %d, return directly-------------\n",cmd->opcode);
        mrq->cmd->error = -ETIMEDOUT;
        alimci_finish_request(host,FALSE,__LINE__);
    }
    /* Enable Interrupt */
   // enable_irq(host->irq);
}

// return 1 when card present, 0 for card not exist
static int alimci_card_present(struct mmc_host *mmc)
{
    struct alimci_host *host = mmc_priv(mmc);
    int ret = 0;
	int info_strapin = 0;
    int boot_dev = 0;
 
    if (FORCE_EMMC_ACTIVE == host->enum_host_switch_type)
    {
        info_strapin = ALI_SOC_RD32(CHIP_STRAP_PIN); //read strap pin
		boot_dev = BOOT_DEV(info_strapin);
		
		if(BOOT_DEV_EMMC(boot_dev))//if emmc boot up
        {
           ret = 1;  //present
		}
		else
		{
		   ret = 0; //not presen
		}
    }
    else
    {
        ret = ali_gpio_get_value(host->gpio_cd);
        if (host->gpio_cd_invert)
        {
            ret = !ret;
        }

        alimci_log("-----------Line:1285,GPIO_CD=%d,ret=0x%x,gpio_cd_invert=0x%x---------------\n", host->gpio_cd,ret,host->gpio_cd_invert);
    
    }

    return ret;

}

static void alimci_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
    struct alimci_host *host = mmc_priv(mmc);
    host->mrq = mrq;

    if (alimci_card_present(mmc) == 0)
    {
        alimci_log("Card is not present!\n");
        host->mrq->cmd->error = -ENOMEDIUM;
        mmc_request_done(host->base_mmc, mrq);

		//alimci_finish_request(host,FALSE,__LINE__);
        //alimci_dump_status(host,__FUNCTION__,__LINE__);
    }
    else
    {
        alimci_sd_emmc_pinmux_set(host,__LINE__);
        alimci_send_request(mmc);
    }
}

//handler host->setio()
static void alimci_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
     switch (ios->power_mode)
    {
        case MMC_POWER_ON:
        case MMC_POWER_UP:
            break;
        case MMC_POWER_OFF:
        default:
            break;
    }
}

//check card read-only, return 1 means read-only, return 0 r/w
static int alimci_get_ro(struct mmc_host *mmc)
{
    struct alimci_host *host = mmc_priv(mmc);
    int ret = -1;

	if(!host)
	{
	   return -ENOSYS;
	}

    if (FORCE_EMMC_ACTIVE != host->enum_host_switch_type)
    {
        if (gpio_is_valid(host->gpio_card_ro))
        {
            if (host->gpio_card_ro_invert)
            {
                ret = !ali_gpio_get_value(host->gpio_card_ro);
            }
            else
            {
                ret = ali_gpio_get_value(host->gpio_card_ro);
            }
            return ret;
        }
        return -ENOSYS;      
    }
    else
    {
        return 0; //always is writable for MMC card and eMMC.  
    }
}

static struct mmc_host_ops alimci_ops =
{
    .request    = alimci_request,
    .set_ios    = alimci_set_ios,
    .get_ro     = alimci_get_ro,
    .get_cd     = alimci_card_present,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
    .hw_reset  = alimci_host_reset_locked,
#endif
};

/*
 * ISR for the CardDetect Pin
*/
static irqreturn_t alimci_detect_irq(int irq, void *dev_id)
{
    struct alimci_host *host = dev_id;
    int present;
    int int_status;
	
    int_status = get_gpio_interrupt_status(host->gpio_cd);
    if (1 != int_status)
    {
        return IRQ_NONE; 
    }
	
    clear_gpio_interrupt_status(host->gpio_cd);
	
	present = alimci_card_present(host->base_mmc);	

     /*
     * we expect this irq on both insert and remove,
     * and use a short delay to debounce.
     */
    if (present != host->present)
    { 
        //printk(KERN_INFO"%s: card %s\n", mmc_hostname(host->base_mmc), present ? "insert" : "remove");

        host->present = present;

        if (!present)
        {
            alimci_reset_media_type(host->base_mmc);
        }
		
        /* 0.5s needed because of early card detect switch firing */
        mmc_detect_change(host->base_mmc, msecs_to_jiffies(host->detect_delay_ms));
    }
    return IRQ_HANDLED;
}


static int alimci_reg_gpio(struct platform_device *pdev, struct alimci_host *host)
{
    int ret, gpio_cd = -1, gpio_ro = -1, gpio_power = -1;
    bool gpio_power_invert = FALSE;
    bool gpio_cd_invert = FALSE;

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
			else if(M3921_C == host->ChipVer)
            {
                if(M3921_QFP256 == host->ChipPkg)  //qfp
	             {
	                gpio_cd = GPIO_QFP_CD_C3921; //113;
	                gpio_ro = GPIO_QFP_WP_C3921; //3;
	                //gpio_power = GPIO_QFP_PWR;
	                gpio_power_invert = TRUE;
	                gpio_cd_invert = CARD_DETECT_INVERT;
					host->InitClkDiv = ALI_INIT_CLKDIV_C3921;
	                host->TranClkDiv = ALI_TRAN_CLKDIV_C3921;
                }
				else  //bga
				{
				    gpio_cd = GPIO_BGA_CD_C3921;//113;
	                gpio_ro = GPIO_BGA_WP_C3921;//19;
	                //gpio_power = GPIO_QFP_PWR;
	                gpio_power_invert = TRUE;
	                gpio_cd_invert = CARD_DETECT_INVERT;
					host->InitClkDiv = ALI_INIT_CLKDIV_C3921;
	                host->TranClkDiv = ALI_TRAN_CLKDIV_C3921;
				}
            }

            if (FORCE_EMMC_ACTIVE != host->enum_host_switch_type)
            {
                host->irq_cd = ALI_ARM_GPIO_IRQ;
            }
			
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
            
            if (FORCE_EMMC_ACTIVE != host->enum_host_switch_type)
            {
                host->irq_cd = ALI_MIPS_GPIO_IRQ;
            }
            
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

   if (FORCE_EMMC_ACTIVE == host->enum_host_switch_type)
   {
       return 0;
   }

   if (gpio_is_valid(host->gpio_power))
   {
        ret = gpio_request(host->gpio_power, "mmc card power");
        if (ret)
        {
            alimci_err("Failed requesting gpio_power %d,ret=0x%x\n", host->gpio_power,ret);
            goto out;
        }

        gpio_enable_pin(host->gpio_power);
        ali_gpio_direction_output(host->gpio_power,host->gpio_power_invert ? GPIO_LOW:GPIO_HIGH);
    }
	
    if (gpio_is_valid(gpio_ro))
    {
        ret = gpio_request(gpio_ro, "mmc card read only");
        if (ret)
        {
            alimci_err("Failed requesting gpio_ro %d\n", gpio_ro);
            goto err_gpio_ro;
        }

		gpio_enable_pin(gpio_ro);
        ali_gpio_direction_input(gpio_ro);
    }
    if (gpio_is_valid(gpio_cd))
    {
        ret = gpio_request(gpio_cd, "mmc card detect");
        if (ret)
        {
            alimci_err("Failed requesting gpio_cd %d\n", gpio_cd);
            goto err_gpio_cd;
        }

        //gpio rising edge interrupt enable, 1: enable  0: disable
        set_gpio_rising_ir_pin(gpio_cd, 1);

        //gpio falling edge interrupt enable, 1: enable  0: disable
		set_gpio_falling_ir_pin(gpio_cd, 1);
		
		gpio_enable_pin(gpio_cd);
        ali_gpio_direction_input(gpio_cd);
        host->present = alimci_card_present(host->base_mmc);

        if (FORCE_EMMC_ACTIVE != host->enum_host_switch_type)
        {
           
            ret = request_irq(host->irq_cd, alimci_detect_irq, IRQF_SHARED, "mmc card detect", host);
            if (ret)
            {
                alimci_err("failed to request card detect IRQ,irq_cd=%d, ret=0x%x\n",host->irq_cd,ret);
                goto err_request_irq;
            }
        }
    }
    return 0;

err_request_irq:
    gpio_free(gpio_cd);
err_gpio_cd:
    gpio_free(gpio_ro);
err_gpio_ro:
    gpio_free(gpio_power);
out:
    return 1;
}

static void alimci_unreg_gpio(struct platform_device *pdev, struct alimci_host *host)
{
    int gpio_cd = -1, gpio_ro = -1, gpio_power = -1;

	if (!host)
    {
       return;
    }

    if (FORCE_EMMC_ACTIVE == host->enum_host_switch_type)
    { 
       return;
    }

    gpio_cd = host->gpio_cd;
    gpio_ro = host->gpio_card_ro;
    gpio_power = host->gpio_power;
    
    if (gpio_is_valid(gpio_power))
    {
        gpio_free(gpio_power);
    }
    if (gpio_is_valid(gpio_ro))
    {
        gpio_free(gpio_ro);
    }
    if (gpio_is_valid(gpio_cd))
    {

    if (FORCE_EMMC_ACTIVE != host->enum_host_switch_type)
    {
        free_irq(host->irq_cd, host);
    }
	
    gpio_free(gpio_cd);
   }

    return ;
}

static int __devinit alimci_probe(struct platform_device *pdev)
{
    struct alimci_host *host;
    struct mmc_host *mmc;
    int ret;
    int DbgOut = 0;
    int ForceBitWidth = FORCE_BW_OFF;
    static unsigned int ui_first_porbe = 0;
    struct alimci_host_platform_data  *pdata = NULL;

	printk("alimci probe\n");

    mmc = mmc_alloc_host(sizeof(struct alimci_host), &pdev->dev);
    if (!mmc)
    {
        alimci_err("----------Oops: I meet some issue here.---------\n");
        ret = -ENOMEM;
        goto probe_out;
    }

   if (!pdev->dev.platform_data)
   {
       return -ENXIO;
   }  

    host = mmc_priv(mmc);

    pdata = (struct alimci_host_platform_data  *)pdev->dev.platform_data;
    host->enum_host_switch_type = pdata->host_switch_type;
	if (FORCE_EMMC_ACTIVE == host->enum_host_switch_type )
    {
        p_host_emmc = host;   
		printk("host type:eMMC\n");
    }
    else
    {
        p_host_sd = host;
		printk("host type:SD\n");
    }
	
    if (NULL == p_current_host)
    {
        p_current_host = host;    
    }

    host->base_mmc   = mmc;
    host->pdev  = pdev;
    host->detect_delay_ms = DETECT_DELAY;
    host->data_op_type = NO_DATA_OP;
    host->pinmux1 = 0;
    host->pinmux2 = 0;
    host->bus_width = MMC_BUS_WIDTH_1;
    host->DbgOut = DFT_DBG_OUT;
    host->ForceBitWidth = FORCE_BW_OFF;
    host->dma_handle = 0;
    
    host->soc_base = PHYS_SOC_BASE_ADDR;
	
    get_chip_info(&host->ChipId, &host->ChipPkg, &host->ChipVer);

	printk("host->ChipId:%x, host->ChipVer:%x, host->ChipPkg:%x\n", host->ChipId, host->ChipVer, host->ChipPkg); 	  

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
    
    //Try to read command line parameters.
    if (alimci_param && sscanf(alimci_param, "DbgOut=%d,ForceBitWidth=%d", &DbgOut,&ForceBitWidth) == 2)
    {
        if (ALIMCI_OFF == DbgOut || ALIMCI_ON == DbgOut)
            host->DbgOut = DbgOut;
        if (FORCE_BW_1_BIT == ForceBitWidth || FORCE_BW_4_BIT == ForceBitWidth)
           host->ForceBitWidth = ForceBitWidth;
    }

    alimci_dbg_procfs_init(host);
    
    alimci_hw_gating(mmc,CLK_GATE_ON);

    alimci_sd_emmc_pinmux_set(host, __LINE__);

    alimci_host_reset(mmc);
	
    alimci_sd_emmc_pinmux_restore(host, __LINE__);

    spin_lock_init(&host->complete_lock);
    tasklet_init(&host->cmd_tasklet, alimci_cmd_tasklet, (unsigned long) host);
    tasklet_init(&host->data_tasklet, alimci_data_tasklet, (unsigned long) host);

	//create a timer for checking whether cmd timeout or not
	setup_timer(&host->waitfor_irq_timer, alimci_waitfor_irq_timeout_timer, (unsigned long)host);
   	
    host->usedma = TRUE; //use dma or not

    if(alimci_reg_gpio(pdev, host))
    {
        alimci_err("failed to register GPIO.\n");
        ret = -ENOENT;
        goto probe_free_host;
    }

    //get sdio irq_id
    host->irq = ALI_MCI_IRQ;

    if (0 == ui_first_porbe)
    {
        ui_first_porbe = 1;
        
        if (request_irq(host->irq, alimci_irq, IRQF_DISABLED, SDIO_DRIVER_NAME, host))
        {

            alimci_err("failed to request mci interrupt.\n");
            ret = -ENOENT;
            goto probe_free_irq_cd;
        }
    }
    

    
    disable_irq(host->irq);

    mmc->ops    = &alimci_ops;
    mmc->ocr_avail  = MMC_VDD_27_28 | MMC_VDD_35_36;  //Holon: from 2.7 to 3.3v.
    mmc->caps   = MMC_CAP_4_BIT_DATA; //Can the host do 4 bit transfers

#if 0
    if (FORCE_EMMC_ACTIVE == host->enum_host_switch_type)
    {
        mmc->caps   |= MMC_CAP_8_BIT_DATA; //eMMC support 8 bit transfter, while S3921 MMC doesn't support 8 bits since only 4 data lines is connected.
    }
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
    mmc->caps  |= MMC_CAP_HW_RESET;
#endif
    mmc->f_min  = 24000000 / 256; //set min work freq
    mmc->f_max  = 24000000;       //set max work freq

    mmc->max_blk_count  = 1024; //4095;
    mmc->max_blk_size   = 512; //4095;
    mmc->max_req_size   = 512 * 1024; //4095 * 512;
    mmc->max_seg_size   = mmc->max_req_size;

    ret = mmc_add_host(mmc);
    if (ret)
    {
        alimci_err("can't add host to mmc.\n");
        ret = -ENOENT;
        goto probe_free_irq;
    }
	
    platform_set_drvdata(pdev, mmc);

    enable_irq(host->irq);

    if (FORCE_EMMC_ACTIVE != host->enum_host_switch_type)
    {
        enable_gpio_interrupt_pin(host->gpio_cd/*,TRUE,TRUE*/);
    }
    	
    printk("alimci initialisation done.\n");
	
    return 0;

probe_free_irq:
    free_irq(host->irq, host);

probe_free_irq_cd:
    
    if (FORCE_EMMC_ACTIVE != host->enum_host_switch_type)
    {
        if (host->irq_cd >= 0)
        {
            free_irq(host->irq_cd, host);
        }     
    }

probe_free_host:
    alimci_sd_emmc_pinmux_restore(host, __LINE__);
    mmc_free_host(mmc);

probe_out:
    return ret;
}

static void alimci_shutdown(struct platform_device *pdev)
{
    struct mmc_host *mmc = platform_get_drvdata(pdev);
    alimci_hw_gating(mmc,CLK_GATE_OFF);
    mmc_remove_host(mmc);
}

static int __devexit alimci_remove(struct platform_device *pdev)
{
    struct mmc_host     *mmc  = platform_get_drvdata(pdev);
    struct alimci_host *host = mmc_priv(mmc);
    alimci_shutdown(pdev);
    alimci_unreg_gpio(pdev, host);
    tasklet_disable(&host->cmd_tasklet);
    tasklet_disable(&host->data_tasklet);
    free_irq(host->irq, host);
    mmc_free_host(mmc);
    platform_set_drvdata(pdev, NULL);
    alimci_dbg_procfs_exit(host);
    //alimci_dbg_procfs_exit();
    p_current_host = NULL;
    return 0;
}

#ifdef CONFIG_PM
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0)
static int alimci_suspend(struct platform_device *dev, pm_message_t state)
{
	struct mmc_host *mmc = platform_get_drvdata(dev);
	int ret = 0;

	if (mmc)
		ret = mmc_suspend_host(mmc, state);

	return ret;
}

static int alimci_resume(struct platform_device *dev)
{
	struct mmc_host *mmc = platform_get_drvdata(dev);
	int ret = 0;

	if (mmc)
		ret = mmc_resume_host(mmc);

	return ret;
}

#else
static int alimci_suspend(struct device *dev)
{
    struct mmc_host *mmc = dev_get_drvdata(dev);
    int ret = 0;

    if (mmc)
    {
        ret = mmc_suspend_host(mmc);
    }

    return ret;
}

static int alimci_resume(struct device *dev)
{
    struct mmc_host *mmc = dev_get_drvdata(dev);
    int ret = 0;

    if (mmc)
    {
        ret = mmc_resume_host(mmc);
    }

    return ret;
}
#endif
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
static const struct dev_pm_ops alimci_pm_ops =
{
    .suspend    = alimci_suspend,
    .resume     = alimci_resume,
};

static struct platform_driver alimci_sd_driver =
{
    .probe      = alimci_probe,
    .remove     = alimci_remove,
    .shutdown   = alimci_shutdown,
    .driver     = {
        .name   = SDIO_DRIVER_NAME,
        .owner  = THIS_MODULE,
#ifdef CONFIG_PM
        .pm = &alimci_pm_ops,
#endif
    },
};

#else

static struct platform_driver alimci_sd_driver = {
	.probe =	alimci_probe,
	.remove =	alimci_remove,
#ifdef CONFIG_PM
	.suspend =	alimci_suspend,
	.resume =	alimci_resume,
#endif
	.driver		= {
		.name = SDIO_DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};
#endif


#if defined(CONFIG_MIPS) //S3921 already register SD device in ali-s3921.c
static struct platform_device alimci_sd_device = {
	.name = SDIO_DRIVER_NAME,
	.id   = -1,
};
#endif

static int __init ali_mci_init(void)
{
	int ret;

	ret = platform_driver_register(&alimci_sd_driver);
#if defined(CONFIG_MIPS) //S3921 already register SD device in ali-s3921.c
	if (!ret) {
		ret = platform_device_register(&alimci_sd_device);
		if (ret)
			platform_driver_unregister(&alimci_sd_driver);
	}
#endif
	return ret;
}

static void __exit ali_mci_exit(void)
{
	printk("SDIO driver cleanup\n");
	platform_driver_unregister(&alimci_sd_driver);
}

module_init(ali_mci_init);
module_exit(ali_mci_exit);

MODULE_DESCRIPTION("ALI STB SD/MMC Host Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("<David.Chen@Alitech.com>");

