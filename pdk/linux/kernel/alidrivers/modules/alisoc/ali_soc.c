
#include <linux/version.h>

#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 35)
#include <linux/slab.h> 
#include <asm/io.h>
#include <linux/dma-mapping.h>
#endif

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/semaphore.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/ali_rpc.h>
#include "ali_soc_priv.h"
#include <rpc_hld/ali_rpc_hld.h>
#include <ali_reg.h>
#include <ali_soc_common.h>

#include <ali_board_config.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#if defined(CONFIG_ARM)
#if defined(CONFIG_ALI_AS)
#define _CAS9_CA_ENABLE_
#endif
#endif

//#define PRINTF_TO_MEM
//#define AE_PRINTF_TO_MEM

#define SEE_VER_MAX_LEN					(128)

#ifdef PRINTF_TO_MEM
#define ALI_PRINTF_MEM_BASE				(0xc277f800)
#define ALI_PRINTF_MEM_MAX_SIZE			(0x10000)		// 1M
#define ALI_PRINTF_MEM_BUFFER_SIZE		(256)	
#define ALI_PRINTF_MEM_MAX_INDEX		(ALI_PRINTF_MEM_MAX_SIZE/ALI_PRINTF_MEM_BUFFER_SIZE)	

#define ALI_PRINTF_MEM_HEAD_SIZE		(4)
#define ALI_PRINTF_MEM_HEAD_FLAG		(0)
#define ALI_PRINTF_MEM_HEAD_len			(1)

static UINT8 *g_ali_mem_base = (UINT8 *)(ALI_PRINTF_MEM_BASE);	
static UINT8 *g_ali_mem_head = (UINT8 *)(ALI_PRINTF_MEM_BASE);	
static UINT8 *g_ali_mem_data = (UINT8 *)(ALI_PRINTF_MEM_BASE+4);	

static u8 g_ali_buffer[ALI_PRINTF_MEM_BUFFER_SIZE];
#endif

#ifdef AE_PRINTF_TO_MEM
#define ALI_AE_PRINTF_MEM_BASE			(0xc278f800)
#define ALI_AE_PRINTF_MEM_MAX_SIZE		(0x10000)		// 1M
#define ALI_AE_PRINTF_MEM_BUFFER_SIZE	(256)	
#define ALI_AE_PRINTF_MEM_MAX_INDEX		(ALI_AE_PRINTF_MEM_MAX_SIZE/ALI_AE_PRINTF_MEM_BUFFER_SIZE)	

#define ALI_AE_PRINTF_MEM_HEAD_SIZE		(4)
#define ALI_AE_PRINTF_MEM_HEAD_FLAG		(0)
#define ALI_AE_PRINTF_MEM_HEAD_len		(1)

static UINT8 *g_ali_ae_mem_base = (UINT8 *)(ALI_AE_PRINTF_MEM_BASE);	
static UINT8 *g_ali_ae_mem_head = (UINT8 *)(ALI_AE_PRINTF_MEM_BASE);	
static UINT8 *g_ali_ae_mem_data = (UINT8 *)(ALI_AE_PRINTF_MEM_BASE+4);	

static u8 g_ali_ae_buffer[ALI_AE_PRINTF_MEM_BUFFER_SIZE];
#endif

#if (defined(PRINTF_TO_MEM) || defined(AE_PRINTF_TO_MEM))
struct task_struct *g_ali_soc_thread_id;
#endif


struct class *g_ali_soc_class;

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long ali_soc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#else
__s32 ali_soc_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#endif
__s32 ali_soc_open(struct inode *inode, struct file  *file);
__s32 ali_soc_release(struct inode *inode, struct file  *file);

struct file_operations g_ali_soc_fops =
{
    .owner = THIS_MODULE,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
    .unlocked_ioctl = ali_soc_ioctl,
#else
    .ioctl = ali_soc_ioctl,
#endif    
    .open = ali_soc_open,
    .release = ali_soc_release,
};

struct ali_soc_dev
{

    struct mutex ioctl_mutex;

    dev_t dev_id;

    struct cdev cdev;

};
struct ali_soc_dev g_ali_soc_device;

static __u8 *g_see_ver = NULL;	
static unsigned char soc_debug = 0;
#define SOC_PRINTK(fmt, args...)				\
{										\
	if (0 !=  soc_debug)					\
	{									\
		printk(fmt, ##args);					\
	}									\
}

#define SOC_ERR_PRINTK(fmt, args...)		\
{										\
	printk(fmt, ##args);						\
}


ADF_BOOT_BOARD_INFO * ali_get_user_data(void)
{
	return (ADF_BOOT_BOARD_INFO *)(/*__G_MM_PRIVATE_AREA_START_ADDR - 0x1000*/__REGALIRAW(0x13fff000) /*+ 288*/);		
}


__s32 ali_read8(__u32 addr) 
{    
	__u8 ret = 0;	

	if (mutex_lock_interruptible(&g_ali_soc_device.ioctl_mutex)) 
	{
    		return(-ERESTARTSYS);
	}			

	ret = __REG8ALI(addr & 0x1fffffff);
	SOC_PRINTK("[ %s %d ]: read 0x%08x = 0x%02x\n", __FUNCTION__,  __LINE__, (__u32)addr, ret);		
  
    	mutex_unlock(&g_ali_soc_device.ioctl_mutex);
		
    	return ret;
}


__s32 ali_read16(__u32 addr)
{    
	__u16 ret = 0;	

	if (mutex_lock_interruptible(&g_ali_soc_device.ioctl_mutex))
	{
    		return(-ERESTARTSYS);
	}			

	ret = __REG16ALI(addr & 0x1fffffff);
	SOC_PRINTK("[ %s %d ]: read 0x%08x = 0x%04x\n", __FUNCTION__,  __LINE__, (__u32)addr, ret);		
  
    	mutex_unlock(&g_ali_soc_device.ioctl_mutex);
		
    	return ret;
}


__s32 ali_read32(__u32 addr)
{    
	__u32 ret = 0;	

	if (mutex_lock_interruptible(&g_ali_soc_device.ioctl_mutex))
	{
    		return(-ERESTARTSYS);
	}	

	ret = __REG32ALI(addr & 0x1fffffff);
	SOC_PRINTK("[ %s %d ]: read 0x%08x = 0x%08x\n", __FUNCTION__,  __LINE__, (__u32)addr, ret);		
  
	mutex_unlock(&g_ali_soc_device.ioctl_mutex);
	
	return ret;
}


__s32 ali_write8(__u32 addr, __u8 data)
{    
	if (mutex_lock_interruptible(&g_ali_soc_device.ioctl_mutex))
	{
    		return(-ERESTARTSYS);
	}	

	__REG8ALI(addr & 0x1fffffff) = data;
	SOC_PRINTK("[ %s %d ]: write 0x%08x = 0x%02x\n", 
		__FUNCTION__,  __LINE__, (__u32)addr, (__u8)(__REG8ALI(addr & 0x1fffffff)));		

  
    	mutex_unlock(&g_ali_soc_device.ioctl_mutex);
    	return 0;
}


__s32 ali_write16(__u32 addr, __u16 data)
{    
	if (mutex_lock_interruptible(&g_ali_soc_device.ioctl_mutex))
	{
    		return(-ERESTARTSYS);
	}	

	__REG16ALI(addr & 0x1fffffff) = data;
	SOC_PRINTK("[ %s %d ]: write 0x%08x = 0x%02x\n", 
		__FUNCTION__,  __LINE__, (__u32)addr, (__u16)(__REG16ALI(addr & 0x1fffffff)));		

  
    	mutex_unlock(&g_ali_soc_device.ioctl_mutex);
    	return 0;
}


__s32 ali_write32(__u32 addr, __u64 data)
{    
	if (mutex_lock_interruptible(&g_ali_soc_device.ioctl_mutex))
	{
    		return(-ERESTARTSYS);
	}	

	__REG32ALI(addr & 0x1fffffff) = data;
	SOC_PRINTK("[ %s %d ]: write 0x%08x = 0x%08x\n", 
		__FUNCTION__,  __LINE__, (unsigned int)addr, (__u32)(__REG32ALI(addr & 0x1fffffff)));		

  
    	mutex_unlock(&g_ali_soc_device.ioctl_mutex);
    	return 0;
}


/**
 * Back up the register value
 */
typedef struct _pm_reg {
    unsigned int vdac_pd_b8008084;
    unsigned int adac_pd_b80020f8;
    unsigned int hdmi_pd_b800006c;
    unsigned int rxadc_pd_b8028421;
    unsigned int rxpll_pd_b80000b4;	
    unsigned int clk_gating_b8000060;
    unsigned int clk_gating_b8000064;	
}pm_reg_t;

static pm_reg_t __pm_reg_value;

/**
 * update for 3912 hdmi dongle power save
 */
static  void operate_vdac(bool power_down)
{
    if (power_down)
    {
        __pm_reg_value.vdac_pd_b8008084 = __REG32ALI(0x18008084);
        __REG32ALI(0x18008084) |= 0x00000f00;
    }
    else
        __REG32ALI(0x18008084) = __pm_reg_value.vdac_pd_b8008084;
}

static  void operate_adac(bool power_down)
{
    if (power_down)
    {
        __pm_reg_value.adac_pd_b80020f8 = *(unsigned int *)0x180020f8;
        __REG32ALI(0x180020f8) |= 0x00073f0d;
    }
    else
        __REG32ALI(0x180020f8) = __pm_reg_value.adac_pd_b80020f8;
}

static  void operate_rxadc(bool power_down)
{
    if (power_down)
    {
        __pm_reg_value.rxadc_pd_b8028421 = __REG32ALI(0x18028421);
        __REG32ALI(0x18028421) |= (1<<1)|(1<<2)|(1<<3);	// 
    }
    else
        __REG32ALI(0x18028421) = __pm_reg_value.rxadc_pd_b8028421;
}

static  void operate_rxpll(bool power_down)
{
    if (power_down)
    {
        __pm_reg_value.rxpll_pd_b80000b4 = __REG32ALI(0x180000b4);
        __REG32ALI(0x180000b4) |= (1<<12);	// bit 12
    }
    else
        __REG32ALI(0x180000b4) = __pm_reg_value.rxpll_pd_b80000b4;
}

static  void operate_clk(bool power_down)
{
    if (power_down)
    {
        __pm_reg_value.clk_gating_b8000060 = __REG32ALI(0x18000060);
        __REG32ALI(0x18000060) |= (1<<12)|(1<<11)|(1<<9)|(1<<8)|(1<<7)|(1<<6) ;
	
        __pm_reg_value.clk_gating_b8000064 = __REG32ALI(0x18000064);
        __REG32ALI(0x18000064) |= (1<<28) | (1<<27)|(1<<26)  |(1<<25)|(1<<23) |(1<<17)\
			|(1<<11)|(1<<10)|(1<<9)|(1<<8)|(1<<5)|(1<<3)|(1<<0);	//bit 25
    }
    else{
        __REG32ALI(0x18000060) = __pm_reg_value.clk_gating_b8000060;
	__REG32ALI(0x18000064)= __pm_reg_value.clk_gating_b8000064;
    	}
}

static void operate_power_down(bool enter)
{
	//set analog first,then gating clk;
	operate_vdac(enter);
	operate_adac(enter);
	operate_rxadc(enter);
	operate_rxpll(enter);
	operate_clk(enter);
}


__s32 ali_per_read32(__u32 addr)
{    
	__u32 ret = 0;	
	__u32 virt_addr = 0;

	if (mutex_lock_interruptible(&g_ali_soc_device.ioctl_mutex))
	{
    		return(-ERESTARTSYS);
	}	

	#if defined(CONFIG_ALI_CHIP_M3921)
	virt_addr = VIRT_ARM_PERIPHBASE + addr;
	SOC_PRINTK("[ %s %d ]: VIRT_ARM_PERIPHBASE = 0x%08x, addr = 0x%08x, virt_addr = 0x%08x\n", 
		__FUNCTION__,  __LINE__, (__u32)VIRT_ARM_PERIPHBASE, addr, virt_addr);		
	#endif
	
	ret = readl(virt_addr);	
	SOC_PRINTK("[ %s %d ]: read 0x%08x = 0x%08x\n", 
		__FUNCTION__,  __LINE__, virt_addr, ret);		
  
	mutex_unlock(&g_ali_soc_device.ioctl_mutex);
	
	return ret;
}




__s32 ali_per_write32(__u32 addr, __u64 data)
{    
	__u32 virt_addr = 0;

	
	if (mutex_lock_interruptible(&g_ali_soc_device.ioctl_mutex))
	{
    		return(-ERESTARTSYS);
	}	

	SOC_PRINTK("[ %s %d ]: addr = 0x%08x, data = 0x%08x\n", 
		__FUNCTION__,  __LINE__, (__u32)addr, (__u32)data);	

	#if defined(CONFIG_ALI_CHIP_M3921)
	virt_addr = VIRT_ARM_PERIPHBASE + addr;
	SOC_PRINTK("[ %s %d ]: VIRT_ARM_PERIPHBASE = 0x%08x, virt_addr = 0x%08x\n", 
		__FUNCTION__,  __LINE__, (__u32)VIRT_ARM_PERIPHBASE, virt_addr);	
	#endif
	
	writel(data, virt_addr);	
	
	SOC_PRINTK("[ %s %d ]: write 0x%08x = 0x%08x\n", 
		__FUNCTION__,  __LINE__, virt_addr, (__u32)data);		
  
    mutex_unlock(&g_ali_soc_device.ioctl_mutex);
    return 0;
}



#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long ali_soc_ioctl(struct file   *filp, unsigned int   cmd, unsigned long  arg)
#else
__s32 ali_soc_ioctl(struct inode  *inode, struct file   *filp, unsigned int   cmd, unsigned long  arg)
#endif
{
    __s32 ret = 0;
    unsigned long          paras[MAX_ALI_SOC_PARAS];    
	

    SOC_PRINTK("[ %s %d ]: cmd = %d, size = %d\n", 
    	__FUNCTION__,  __LINE__, _IOC_NR(cmd), _IOC_SIZE(cmd));	

    if (_IOC_SIZE(cmd) > (MAX_ALI_SOC_PARAS * 4))
    {
    	SOC_ERR_PRINTK("[ %s, %d ], param size %d error.\n", 
    		__FUNCTION__, __LINE__, _IOC_SIZE(cmd));
        return(-EFAULT);
    }

    ret = copy_from_user(&paras, (void __user *)arg, _IOC_SIZE(cmd));
    if (0 != ret)
    {
    	SOC_ERR_PRINTK("%s, %d.\n", __FUNCTION__, __LINE__);
        	return(-EFAULT);
    }

    switch(cmd) 
    {
		case ALI_SOC_GET_MEMORY_MAP:
    	{
	        struct soc_memory_map smm;

			memset((void *)&smm, 0, sizeof(smm));
			#if !defined(CONFIG_ALI_CHIP_M3921)
			smm.main_start = __G_ALI_MM_MAIN_MEM[0][0];
			smm.main_end  = __G_ALI_MM_MAIN_MEM[__G_ALI_MM_MAIN_MEM_NUM-1][1];
			#endif
			smm.fb_start = (__G_ALI_MM_FB0_START_ADDR & 0x1FFFFFFF);
			smm.osd_bk = (__G_ALI_MM_FB2_START_ADDR & 0x1FFFFFFF);
			#ifdef CONFIG_DVB_ALI_M36_DMX		
			smm.see_dmx_src_buf_start = 0;
			smm.see_dmx_src_buf_end  = 0;
			smm.see_dmx_decrypto_buf_start = 0;
			smm.see_dmx_decrypto_buf_end  = 0;
			smm.dmx_start = (__G_ALI_MM_DMX_MEM_START_ADDR & 0x1FFFFFFF);
			smm.dmx_top = ((__G_ALI_MM_DMX_MEM_START_ADDR + __G_ALI_MM_DMX_MEM_SIZE) & 0x1FFFFFFF);
			#endif		
			smm.see_start   = (__G_ALI_MM_PRIVATE_AREA_START_ADDR & 0x1FFFFFFF);
			smm.see_top     = ((__G_ALI_MM_PRIVATE_AREA_START_ADDR + __G_ALI_MM_PRIVATE_AREA_SIZE) & 0x1FFFFFFF);
			smm.video_start = (__G_ALI_MM_VIDEO_START_ADDR & 0x1FFFFFFF);
			smm.video_top   = ((__G_ALI_MM_VIDEO_START_ADDR + __G_ALI_MM_VIDEO_SIZE) & 0x1FFFFFFF);
			smm.frame = (__G_ALI_MM_STILL_FRAME_START_ADDR & 0x1FFFFFFF);
			smm.frame_size = __G_ALI_MM_STILL_FRAME_SIZE;
			smm.vcap_fb      = (__G_ALI_MM_VCAP_FB_START_ADDR & 0x1FFFFFFF);
			smm.vcap_fb_size = __G_ALI_MM_VCAP_FB_SIZE;
			smm.vdec_vbv_start = (__G_ALI_MM_VDEC_VBV_START_ADDR & 0x1FFFFFFF);
			smm.vdec_vbv_len = __G_ALI_MM_VDEC_VBV_SIZE;
			smm.shared_start = (__G_ALI_MM_SHARED_MEM_START_ADDR & 0x1FFFFFFF);
			smm.shared_top = ((__G_ALI_MM_SHARED_MEM_START_ADDR + __G_ALI_MM_SGDMA_MEM_SIZE) & 0x1FFFFFFF);
			smm.reserved_mem_addr = (__G_ALI_MM_SPECIAL_RESERVED0_START_ADDR & 0x1FFFFFFF);
			smm.reserved_mem_size = __G_ALI_MM_SPECIAL_RESERVED0_SIZE;
			smm.media_buf_addr = (__G_ALI_MM_APE_MEM_START_ADDR & 0x1FFFFFFF);
			smm.media_buf_size = __G_ALI_MM_APE_MEM_SIZE;
			smm.mcapi_buf_addr = (__G_ALI_MM_MCAPI_MEM_START_ADDR & 0x1FFFFFFF);
			smm.mcapi_buf_size = __G_ALI_MM_MCAPI_MEM_SIZE;
			
            		if (0 != copy_to_user((void*)arg, &smm, sizeof(smm)))
			{
				SOC_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}	   
			ret = 0;
        }	
        break;
		
    	case ALI_SOC_SET_DEBUG_LEVEL:
		{			
			struct debug_level_paras *par = (struct debug_level_paras *)paras;    
			soc_debug = (unsigned char)par->level;
			SOC_PRINTK("[ %s %d ], set soc_debug = %d, par->level = %ld\n", __FUNCTION__, __LINE__, soc_debug, par->level);			
		}
		break;

	case ALI_SOC_READ8:
    	{
	        struct soc_opt_paras8 *par = (struct soc_opt_paras8 *)paras;     	   
	        ret = ali_read8(par->addr);	   
    	}
    	break;

		case ALI_SOC_READ16:
    	{
	        struct soc_opt_paras16 *par = (struct soc_opt_paras16 *)paras;     	   
	        ret = ali_read16(par->addr);	   
    	}
    	break;
	
		case ALI_SOC_READ32:
    	{
	        struct soc_opt_paras32 *par = (struct soc_opt_paras32 *)paras;     	   
	        ret = ali_read32(par->addr);	   
    	}
    	break;

		case ALI_SOC_WRITE8:
        {
            struct soc_opt_paras8 *par = (struct soc_opt_paras8 *)paras;        	  	
            ret = ali_write8(par->addr, par->data);
        }
        break;

		case ALI_SOC_WRITE16:
        {
            struct soc_opt_paras16 *par = (struct soc_opt_paras16 *)paras;        	  	
            ret = ali_write16(par->addr, par->data);
        }
        break;

		case ALI_SOC_WRITE32:
        {
            struct soc_opt_paras32 *par = (struct soc_opt_paras32 *)paras;        	  	
            ret = ali_write32(par->addr, par->data);
        }
        break;

    	case ALI_SOC_PER_READ32:
        {
            struct soc_opt_paras32 *par = (struct soc_opt_paras32 *)paras;                
            ret = ali_per_read32(par->addr);	            
        }
        break;

    	case ALI_SOC_PER_WRITE32:
        {
            struct soc_opt_paras32 *par = (struct soc_opt_paras32 *)paras;              
            ret = ali_per_write32(par->addr, par->data);            
        }
        break;
        case ALI_SOC_READ:
        {
            struct soc_op_paras * par = (struct soc_op_paras*) paras ;            
            ret = copy_to_user((void __user *)par->to, par->from,par->len);
            SOC_PRINTK("ali_soc_read : to= 0x%x, from =0x%x ,len = 0x%x \n ",
            	(unsigned int)(par->to), (unsigned int)(par->from), par->len);
        }
        break;
        case ALI_SOC_WRITE:
        {
            struct soc_op_paras * par = (struct soc_op_paras*) paras ;       
            ret = copy_from_user(par->to, (const void __user *)par->from,par->len);
            SOC_PRINTK("ali_soc_write : to= 0x%x, from =0x%x ,len = 0x%x \n ",
            	(unsigned int)(par->to), (unsigned int)(par->from), par->len);
        }
        break;
        
        case ALI_SOC_CHIP_ID:
        {
        		ret = ali_sys_ic_get_chip_id();
        }
        break;
        
        case ALI_SOC_PRODUCT_ID:
        {
         		ret = ali_sys_ic_get_product_id();
        }
        break;
        
        case ALI_SOC_GET_BONDING:
        {
        		ali_sys_ic_get_bonding();
        }
        break;
        
        case ALI_SOC_REV_ID:
        {
        		ret = ali_sys_ic_get_rev_id();
        }
        break;
        
        case ALI_SOC_CPU_CLOCK:
        {
        		ret = ali_sys_ic_get_cpu_clock();
        }
        break;
        
        case ALI_SOC_DRAM_CLOCK:
        {
        		ret = ali_sys_ic_get_dram_clock();
        }
        break;

        case ALI_SOC_DRAM_SIZE:
        {
        	ret = ali_sys_ic_get_dram_size();        	
        }
        break;
        
        case ALI_SOC_HD_ENABLED:
        {
            ret = ali_sys_ic_get_hd_enabled();
        }
        break;
        
        case ALI_SOC_C3603_PRODUCT:
        {
            ret = ali_sys_ic_get_c3603_product();
        }
        break;
        
        case ALI_SOC_USB_NUM:
        {
            ret = ali_sys_ic_get_usb_num();
        }
        break;
        
        case ALI_SOC_USB_PORT_ENABLED:
        {
            struct soc_usb_port *port = (struct soc_usb_port *)paras;
            ret = ali_sys_ic_usb_port_enabled(port->usb_port);
        }
        break;
        
        case ALI_SOC_NIM_M3501_SUPPORT:
        {
            ret = ali_sys_ic_nim_m3501_support();
        }
        break;
        
        case ALI_SOC_NIM_SUPPORT:
        {
            ret = ali_sys_ic_nim_support();
        }
        break;
        
        case ALI_SOC_CI_NUM:
        {
            ret = ali_sys_ic_get_ci_num();
        }
        break;
        
        case ALI_SOC_MAC_NUM:
        {
            ret = ali_sys_ic_get_mac_num();
        }
        break;
        
        case ALI_SOC_TUNER_NUM:
        {
            ret = ali_sys_ic_get_tuner_num();
        }
        break;
        
        case ALI_SOC_HD_IS_ENABLED:
        {
            ret = ali_sys_ic_hd_is_enabled();
        }
        break;
        
        case ALI_SOC_SATA_EANBLE:
        {
            ret = ali_sys_ic_sata_enable();
        }
        break;
        
        case ALI_SOC_SCRAM_ENABLE:
        {
            ret = ali_sys_ic_dram_scramble_enabled();
        }
        break;
        
        case ALI_SOC_SECU_ENABLE:
        {
            ret = ali_sys_ic_io_security_enabled();
        }
        break;
        
        case ALI_SOC_SPL_ENABLE:
        {
            ret = ali_sys_ic_split_enabled();
        }
        break;
        
        case ALI_SOC_UART_ENABLE:
        {
            ret = aii_sys_ic_uart_enabled();
        }
        break;
        
        case ALI_SOC_ETJT_ENABLE:
        {
            ret = ali_sys_ic_ejtag_enabled();
        }
        break;
        
        case ALI_SOC_MV_ENABLE:
        {
            ret = ali_sys_ic_mv_is_enabled();
        }
        break;
        
        case ALI_SOC_AC3_ENABLE:
        {
            ret = ali_sys_ic_ac3_is_enabled();
        }
        break;
        
        case ALI_SOC_DDP_ENABLE:
        {
            ret = ali_sys_ic_ddplus_is_enabled();
        }
        break;
        
        case ALI_SOC_XD_ENABLE:
        {
            ret = ali_sys_ic_XD_is_enabled();
        }
        break;
        
        case ALI_SOC_XDP_ENABLE:
        {
            ret = ali_sys_ic_XDplus_is_enabled();
        }
        break;
        
        case ALI_SOC_AAC_ENABLE:
        {
            ret = ali_sys_ic_aac_is_enabled();
        }
        break;
        
        case ALI_SOC_H264_ENABLE:
        {
            ret = ali_sys_ic_h264_is_enabled();
        }
        break;
        
        case ALI_SOC_MP4_ENABLE:
        {
            ret = ali_sys_ic_mp4_is_enabled();
        }
        break;
        
        case ALI_SOC_MS10_ENABLE:
        {
            ret = ali_sys_ic_ms10_is_enabled();
        }
        break;
                
        case ALI_SOC_MS11_ENABLE:
        {
            ret = ali_sys_ic_ms11_is_enabled();
        }
        break;
        
        case ALI_SOC_RMVB_ENABLE:
        {
            ret = ali_sys_ic_rmvb_is_enabled();
        }
        break;

        case ALI_SOC_VC1_ENABLE:
        {
            ret = ali_sys_ic_vc1_is_enabled();
        }
        break;

        case ALI_SOC_AVS_ENABLE:
        {
            ret = ali_sys_ic_avs_is_enabled();
        }
        break;
        
        case ALI_SOC_VP8_ENABLE:
        {
            ret = ali_sys_ic_vp8_is_enabled();
        }
        break;
                
        case ALI_SOC_FLV_ENABLE:
        {
            ret = ali_sys_ic_flv_is_enabled();
        }
        break;
        
        case ALI_SOC_REBOOT_GET_TIMER:
        {
            struct reboot_timer *par = (struct reboot_timer *)paras;  
            ret = ali_sys_reboot_get_timer(par->time_exp,par->time_cur); 
        }
        break;
        case ALI_SOC_ENTER_STANDBY:
        {
            struct boot_timer *par = (struct boot_timer *)paras;
            ali_sys_ic_enter_standby(par->time_exp,par->time_cur);
        }
        break;

	case ALI_SOC_GET_SEE_VER:
        {         			
		memset(g_see_ver, 0x00, sizeof(g_see_ver));				
		//hld_get_see_version(g_see_ver);		
		SOC_PRINTK("[ %s %d ]: %s\n", 	__FUNCTION__,  __LINE__, g_see_ver);	
		if (0 != copy_to_user((void*)arg, g_see_ver, SEE_VER_MAX_LEN))
		{
			SOC_ERR_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
			return -EFAULT;
		}			
		
		ret = 0;
        }	
        break;
	 case ALI_SOC_DISABLE_SEE_PRINTF:	 		
	 	//hld_disable_see_printf(paras[0]);
	 	break;
	 case ALI_SOC_HIT_SEE_HEART:
	 	//hld_hit_see_heart();
	 	break;
	 case ALI_SOC_ENABLE_SEE_EXCEPTION:
	 	//hld_enable_see_exception();
	 	break;
	case ALI_SOC_REBOOT:
	{
	     //hw_watchdog_reboot();	
	} 
	break;
    case ALI_DSC_ACC_CE_DIS:
    {            
       //ret = sys_ic_dsc_access_ce_is_dis() ;  
    }
    break;
    case ALI_SOC_MG_ENABLE:
        {
            ret = ali_sys_ic_mg_is_enabled();
        }
        break;
	case ALI_SOC_ENABLE_POWER_DOWN:
		operate_power_down(TRUE);
		break;
	case ALI_SOC_SHOW_SEE_PLUGIN_INFO:
		//hld_show_see_plugin_info();
		break;			
	 default:
        {
			ret = -EINVAL;
			SOC_PRINTK("[ %s %d ]: cmd = %d\n", 	__FUNCTION__,  __LINE__, _IOC_NR(cmd));	            
        }
        break;
    }

	//SOC_PRINTK("[ %s %d ]\n", 	__FUNCTION__,  __LINE__);	
    return (ret);
}



__s32 ali_soc_open(struct inode *inode, struct file  *file)
{
	if(NULL == g_see_ver)
	{
		g_see_ver = (UINT8 *)kmalloc(SEE_VER_MAX_LEN, GFP_KERNEL);
		if(NULL == g_see_ver)
		{
			SOC_ERR_PRINTK("[ %s, %d ], fail.\n", __FUNCTION__, __LINE__);
			return (-ENOMEM);		
		}
		// g_see_ver = (UINT8 *)((((UINT32)(g_see_ver))&0x1fffffff)|0xa0000000);	
	}

    	//SOC_ERR_PRINTK("[ %s, %d ], system start running.\n", __FUNCTION__, __LINE__);

    	return (0);
}


#ifdef AE_PRINTF_TO_MEM
static void ae_printf_debug(void)
{
	u8 *rpc_head = g_ali_ae_mem_head;	
	u8 *rpc_data = g_ali_ae_mem_data;	
	u8 len = 0;
	u32 i = 0;	


	for (i=0; i<ALI_AE_PRINTF_MEM_MAX_INDEX; i++)
	{
		rpc_head = g_ali_ae_mem_head + ALI_AE_PRINTF_MEM_BUFFER_SIZE * i;	
		rpc_data = rpc_head + 4;	
		memset(g_ali_ae_buffer, 0x00, sizeof(g_ali_ae_buffer));
		
		if((1<<0) & rpc_head[0])
		{
			//len = (rpc_head[2] << 8) | (rpc_head[1]);
			len = rpc_head[1];			

			if (len > (ALI_AE_PRINTF_MEM_BUFFER_SIZE - ALI_AE_PRINTF_MEM_HEAD_SIZE))
			{
				len = (ALI_AE_PRINTF_MEM_BUFFER_SIZE - ALI_AE_PRINTF_MEM_HEAD_SIZE);				
			}
			else
			{
				memcpy(g_ali_ae_buffer, rpc_data, len);
				printk("[ soc ], %s", g_ali_ae_buffer);		
			}
			
			memset(rpc_head, 0x00, ALI_AE_PRINTF_MEM_BUFFER_SIZE);
		}			
	}
}
#endif


#ifdef PRINTF_TO_MEM
static void see_printf_debug(void)
{
	u8 *rpc_head = g_ali_mem_head;	
	u8 *rpc_data = g_ali_mem_data;	
	u8 len = 0;
	u32 i = 0;	


	for (i=0; i<ALI_PRINTF_MEM_MAX_INDEX; i++)
	{
		rpc_head = g_ali_mem_head + ALI_PRINTF_MEM_BUFFER_SIZE * i;	
		rpc_data = rpc_head + 4;	
		memset(g_ali_buffer, 0x00, sizeof(g_ali_buffer));
		
		if((1<<0) & rpc_head[0])
		{
			//len = (rpc_head[2] << 8) | (rpc_head[1]);
			len = rpc_head[1];			

			if (len > (ALI_PRINTF_MEM_BUFFER_SIZE - ALI_PRINTF_MEM_HEAD_SIZE))
			{
				len = (ALI_PRINTF_MEM_BUFFER_SIZE - ALI_PRINTF_MEM_HEAD_SIZE);				
			}
			else
			{
				memcpy(g_ali_buffer, rpc_data, len);
				printk("[ soc ], %s", g_ali_buffer);				
			}
			
			memset(rpc_head, 0x00, ALI_PRINTF_MEM_BUFFER_SIZE);
		}			
	}
}
#endif


#if (defined(PRINTF_TO_MEM) || defined(AE_PRINTF_TO_MEM))
static int soc_kthread(void *param)
{
	while (!kthread_should_stop())
	{
	    #ifdef PRINTF_TO_MEM
		see_printf_debug();
        #endif
        
        #ifdef AE_PRINTF_TO_MEM
        ae_printf_debug();
        #endif
        
		msleep(1000);	
	}
}


static int ali_soc_start_kthread(void)
{	
	g_ali_soc_thread_id = kthread_create(soc_kthread, NULL, "ali_soc");
	if(IS_ERR(g_ali_soc_thread_id)){
		printk("soc kthread create fail\n");
		g_ali_soc_thread_id = NULL;
		return -1;
	}
	wake_up_process(g_ali_soc_thread_id);
	
	return 0;
}
#endif


__s32 ali_soc_release(struct inode *inode, struct file  *file)
{		
	//SOC_ERR_PRINTK("[ %s, %d ], system exit.\n", __FUNCTION__, __LINE__); 
	return 0;
}

static int __init ali_soc_init
(
    void
)
{
    int            result;
    struct device          *clsdev;
    struct ali_soc_dev     *soc;

    SOC_PRINTK("%s, %d.\n", __FUNCTION__, __LINE__);


	#ifdef CONFIG_MIPS
    *(volatile unsigned char *)__REGALIRAW(0x18018504) = 0; //disable watch dog
    printk("ali_soc_init stop watch dog\n");
	#endif

    /* Enable CSA module in SEE by PRC. */
    //ali_m36_csa_see_init();

    soc = &g_ali_soc_device;

    mutex_init(&soc->ioctl_mutex);	
    
    result = alloc_chrdev_region(&soc->dev_id, 0, 1, "ali_soc");

    if (result < 0) 
    {
        SOC_ERR_PRINTK("%s, %d\n", __FUNCTION__, __LINE__);

        goto fail;
    }

    SOC_PRINTK("%s, dev_id:%d.\n", __FUNCTION__, soc->dev_id);

    cdev_init(&(soc->cdev), &g_ali_soc_fops);

    soc->cdev.owner = THIS_MODULE;

    result = cdev_add(&soc->cdev, soc->dev_id, 1);

    /* Fail gracefully if need be. */
    if (result)
    {
        SOC_ERR_PRINTK("cdev_add() failed, result:%d\n", result);

        goto fail;
    }

    g_ali_soc_class = class_create(THIS_MODULE, "ali_soc_class");

    if (IS_ERR(g_ali_soc_class))
    {
        result = PTR_ERR(g_ali_soc_class);

        goto fail;
    }

    SOC_PRINTK("%s, %d.\n", __FUNCTION__, __LINE__);

    clsdev = device_create(g_ali_soc_class, NULL, soc->dev_id, 
                           soc, "ali_soc");

    if (IS_ERR(clsdev))
    {
        SOC_ERR_PRINTK(KERN_ERR "device_create() failed!\n");

        result = PTR_ERR(clsdev);

        goto fail;
    }

	/* just for test , TEMP_UNIFY_HLD */
	#ifdef _CAS9_CA_ENABLE_
	result = ali_read32(0x18042074);	
	printk("%s, %d,  result = 0x%x\n", __FUNCTION__, __LINE__, result);
	
	result |= 0x10000;
	ali_write32(0x18042074, result);

	result = ali_read32(0x18042074);	
	printk("%s, %d,  result = 0x%x\n", __FUNCTION__, __LINE__, result);
	#endif

	#if (defined(PRINTF_TO_MEM) || defined(AE_PRINTF_TO_MEM))
		ali_soc_start_kthread();
	#endif

    SOC_PRINTK("%s, %d.\n", __FUNCTION__, __LINE__);

    return(0);

fail:
    return(-1);
}



static void __exit ali_soc_exit(void)
{
	#if (defined(PRINTF_TO_MEM) || defined(AE_PRINTF_TO_MEM))
	if (g_ali_soc_thread_id)
	{
		kthread_stop(g_ali_soc_thread_id);
		g_ali_soc_thread_id = NULL;		
	}
	#endif
    
	SOC_PRINTK("[ %s, %d ]\n", __FUNCTION__, __LINE__);
}

module_init(ali_soc_init);
module_exit(ali_soc_exit);



