/*
 * arch/arm/mach-ali3921/ali-s3921.c
 *
 * ALi S3921 Development Board Setup
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include <linux/version.h> 
#include <linux/memblock.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/export.h>

#include <asm/smp_scu.h>
#include <asm/smp_twd.h>

#include <mach/hardware.h>
#include <mach/ali-s3921.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
#include <linux/irqchip/arm-gic.h>
#else
#include <asm/hardware/gic.h>
#endif
#include <asm/hardware/cache-l2x0.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/mach/map.h>
#include <asm/timex.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0))
#else
#include <asm/localtimer.h>
#endif
#include <asm/io.h>

#include <linux/dma-mapping.h>

#include <ali_reg.h>
#include <mach/s3921-clock.h>
#include "board_config.h"
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/spi/spi.h>
#include <ali_board_config.h>

#ifdef CONFIG_ALI_KEYPAD
#include <linux/input.h>
#include <linux/gpio_keys.h>
#endif

extern void ali_hdmi_set_module_init_hdcp_onoff(bool bOnOff);

extern void arm_remap_lowmem_nocache(unsigned int start, unsigned int len);
int __init m36_init_gpio(void);

//extern void (*_machine_halt)(void);		//arm/kernel/process.c have no _machine_halt
void ali_s3921_poweroff(void);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
extern void ali_s3921_timer_init(void);
extern struct smp_operations s3921_smp_ops;
#else
struct sys_timer;

extern struct sys_timer ali_s3921_timer;
#endif

/**/
/* Platform-dependent data for ali-mci host */
//struct alimci_host_platform_data;

/*
Define the host switch type
C3921 use the  Pinmux Switch to support the eMMC and SD host type Switch
It means that host controller  can be either eMMC active or SD active in one time
*/
enum alimci_hostswitch_type
{
	FORCE_EMMC_ACTIVE = 0,   /* host switch to eMMC active */
	FORCE_SD_ACTIVE,              /* host switch to SD active */
};


/**
 * struct alimci_host_platform_data - Platform-dependent data for ali-mci host
 * @host_switch_type: Define the host switch type, either to be eMMC active or to be SD active
 * @timeout: clock stretching timeout in jiffies
 */
struct alimci_host_platform_data {
	unsigned int	host_switch_type;
	int		timeout;
};

/**/
/*  ali_s3921_map_io */
static struct map_desc ali_s3921_io_desc[] __initdata = 
{
    {
        .virtual        = VIRT_SYSTEM,
        .pfn            = __phys_to_pfn(PHYS_SYSTEM),
        .length         = SIZE_ALIIO,
        .type           = MT_DEVICE,
    },
    {
        .virtual    = VIRT_ARM_PERIPHBASE,
        .pfn            = __phys_to_pfn(PHYS_ARM_PERIPHBASE),
        .length         = SIZE_ARM_PERIPHBASE,
        .type           = MT_DEVICE,
    },  
    
};
#ifdef CONFIG_SENSORS_MPU6050
#include <linux/input/mpu6050.h>

static struct mpu6050_platform_data mpu6050_pdata = {
       .gpio_en=0,
	.gpio_int=0,
	.int_flags=0,
	.use_int=0,
	.place=0,
};

static struct i2c_board_info mpu6050_device[] __initdata = {
        {
                I2C_BOARD_INFO("mpu6050", 0x68),
                .platform_data = &mpu6050_pdata,
        },
};
#endif
/* Mutex to protect requesting and releasing pinmux 1 and 2. This is shared by
   Nand Flash, SDIO and Nor Flash */
DEFINE_MUTEX(ali_sto_mutex);
EXPORT_SYMBOL(ali_sto_mutex);

void __init ali_s3921_map_io(void)
{
    printk("System IO Base Addr = 0x%.08x\n", VIRT_SYSTEM);
    printk("ARM Peripheral Global Timer Base Addr = 0x%.08x\n", (unsigned int) A9_MPCORE_GIT);
    printk("ARM Peripheral GIC CPU Interface Base Addr = 0x%.08x\n", (unsigned int) A9_MPCORE_GIC_CPU);
    printk("ARM Peripheral GIC Distributor Base Addr = 0x%.08x\n",(unsigned int)  A9_MPCORE_GIC_DIST);
    printk("ARM Peripheral PL310 L2Cache Controller Base Addr = 0x%.08x\n",(unsigned int)  A9_L2_BASE_ADDR);
    iotable_init(ali_s3921_io_desc, ARRAY_SIZE(ali_s3921_io_desc));

#ifdef CONFIG_ARM_ALIS3921_CPUFREQ

#if 0
    /*  Setup the s3921 system clock, initialize the ARM CA9 clock to default 1 GHZ  */
    s3921_setup_clocks();
#endif

    /* Register the system clock */
    s3921_register_clocks();
#endif

    printk("%s,%d\n", __FUNCTION__, __LINE__);
    //dump_stack();   
}

/*  ali_s3921_init_early */
extern ALI_L2X0_Init(unsigned long arg1, unsigned long arg2);
void ali_s3921_l2_cache_init(void)
{
#ifdef CONFIG_CACHE_L2X0
    u32 data_rd, data_wr, data_setup;
    u32 tag_rd, tag_wr, tag_setup;
    u32 aux_ctrl_enable, aux_ctrl_disable, cache_type, tag_latency, data_latency, soc_cache_size;

/*  top.CHIP.U_CORE.U_T2_CHIPSET.CA9_MISC_CTRL_250H[18:16]
    CACHE_SIZE:
    001: 128KB
    010: 256KB
    011: 512KB
    Other: non-define, don't use
*/
    printk("L2x0 init before: CPU_REG_CTRL:0x%x, TAG RAM CTRL:0x%x, DATA RAM CTRL:0x%x\n", __REG32ALI(0x18000094), readl_relaxed(A9_L2_BASE_ADDR+L2X0_TAG_LATENCY_CTRL), readl_relaxed(A9_L2_BASE_ADDR+L2X0_DATA_LATENCY_CTRL));
    ALI_L2X0_Init(__REGALIRAW(0x18000094), A9_L2_BASE_ADDR);
    printk("L2x0 init after: CPU_REG_CTRL:0x%x, TAG RAM CTRL:0x%x, DATA RAM CTRL:0x%x\n", __REG32ALI(0x18000094), readl_relaxed(A9_L2_BASE_ADDR+L2X0_TAG_LATENCY_CTRL), readl_relaxed(A9_L2_BASE_ADDR+L2X0_DATA_LATENCY_CTRL));

    /* 8-way associativity, Way size - 64KB */
    aux_ctrl_enable = (0x1 << L2X0_AUX_CTRL_EARLY_BRESP_SHIFT) | (0x0 << L2X0_AUX_CTRL_ASSOCIATIVITY_SHIFT) | (0x03 << L2X0_AUX_CTRL_WAY_SIZE_SHIFT);
    aux_ctrl_enable |= ((1 << L2X0_AUX_CTRL_DATA_PREFETCH_SHIFT) | (1 << L2X0_AUX_CTRL_INSTR_PREFETCH_SHIFT) | (1 << L2X0_AUX_CTRL_EARLY_BRESP_SHIFT));
    aux_ctrl_enable |= (0x01 << 23) | 1; //Non-write allocate
    aux_ctrl_enable |= (0x01 << L2X0_AUX_CTRL_SHARE_OVERRIDE_SHIFT); //Mali Integration Guide Pg 2-11 enable bit 22      
    aux_ctrl_disable = L2X0_AUX_CTRL_MASK;
    l2x0_init(A9_L2_BASE_ADDR , aux_ctrl_enable, aux_ctrl_disable);
    /*Enhance memory performance on 6/6 */
//printk("L2x0 end: CPU_REG_CTRL:0x%x, L2X0_CTRL:0x%x, TAG RAM CTRL:0x%x, DATA RAM CTRL:0x%x\n", __REG32ALI(0x18000094),readl_relaxed(A9_L2_BASE_ADDR+L2X0_CTRL), readl_relaxed(A9_L2_BASE_ADDR+L2X0_TAG_LATENCY_CTRL), readl_relaxed(A9_L2_BASE_ADDR+L2X0_DATA_LATENCY_CTRL));
printk("L2x0 end: CPU_REG_CTRL:0x%x, L2X0_CTRL:0x%x, TAG RAM CTRL:0x%x, DATA RAM CTRL:0x%x, L2X0_PREFETCH_CTRL:0x%x\n", __REG32ALI(0x18000094),readl_relaxed(A9_L2_BASE_ADDR+L2X0_CTRL), readl_relaxed(A9_L2_BASE_ADDR+L2X0_TAG_LATENCY_CTRL), readl_relaxed(A9_L2_BASE_ADDR+L2X0_DATA_LATENCY_CTRL), readl_relaxed(A9_L2_BASE_ADDR+L2X0_PREFETCH_CTRL));
    data_rd = readl_relaxed(A9_L2_BASE_ADDR+L2X0_PREFETCH_CTRL);
    data_rd |= (1<<23) | (1<<24) | (1<<27) | (1<<28) | (1<<29) | (1<<30);
    data_rd &= (~0x1f);
    writel_relaxed(data_rd,A9_L2_BASE_ADDR+L2X0_PREFETCH_CTRL);
    printk("MOdifiy L2X0_PREFETCH_CTRL value to:0x%x\n", data_rd);   
   
#endif
}

void __iomem *ali_m3921_get_l2cache_base(void)
{
	u32	l2cache_base;

	l2cache_base =A9_L2_BASE_ADDR;
	return l2cache_base;
}

static void __init ali_s3921_init_early(void)
{
    printk("%s,%d\n", __FUNCTION__, __LINE__);

    //dump_stack();
    ali_s3921_l2_cache_init();
    //ali_s3921_uart_init_early();

    //ali_driver_param_setting();	
    mutex_init(&ali_sto_mutex);
	printk("--------------ali_sto_mutex is initilized...----------------------------\n",__FUNCTION__);
}

#if 0
/*  ali_s3921_init_irq */
static void ali_s3921_irq_enable(struct irq_data *d)
{
    unsigned int irq;

    if(!d)
        return;

    irq = d->irq;
  //  printk("irq_enable:%d\n",d->irq);

    if(irq>=64 || irq<0)
    {
        while(1);
    }

    if(irq<32)
    {
        return;
    }
    irq -= 32;
    if (irq >= 32) {

        *(u32 *)(SYS_INT_ENABLE2) |= (u32)(1<<(irq - 32));
    }
    else {
        *(u32 *)(SYS_INT_ENABLE1) |= (u32)(1<<irq);
    }





    return;
}

static void ali_s3921_irq_disable(struct irq_data *d)
{
    unsigned int irq;

    if(!d)
        return;

    irq = d->irq;

    if(irq<32)
    {
        return;
    }
    irq -= 32;

    if (irq >= 32) {
        *(u32 *)(SYS_INT_ENABLE2) &= ~((u32)(1<<(irq-32)));
    }
    else {
        *(u32 *)(SYS_INT_ENABLE1) &= ~((u32)(1<<irq));
    }
}
#endif

static void ali_s3921_irq_enable(struct irq_data *d)
{   
unsigned int  irq_no, irq_reg;

    if (d->irq > (96 + ALI_SYS_IRQ_BASE) || d->irq < (ALI_SYS_IRQ_BASE) )
        return; 

    irq_no = (d->irq -ALI_SYS_IRQ_BASE) % 32;
    irq_reg = (d->irq -ALI_SYS_IRQ_BASE) /32;

    switch(irq_reg)
    {
        case 0:
            *(u32 *)(SYS_INT_ENABLE1) |= (u32) (1 << irq_no);
            break;
        case 1:
            *(u32 *)(SYS_INT_ENABLE2) |= (u32) (1 << irq_no);
            //printk("%s INT_REG%d, bit %x\n", __FUNCTION__, irq_reg+1, irq_no);
            break;
        case 2:         
            *(u32 *)(SYS_INT_ENABLE3) |= (u32) (1 << irq_no);
            //printk("%s INT_REG%d, bit %x\n", __FUNCTION__, irq_reg+1, irq_no);
            break;  
        default:
            break;
    }   
}

static void ali_s3921_irq_disable(struct irq_data *d)
{   
    unsigned int  irq_no, irq_reg;

    if (d->irq > (96 + 32) || d->irq < (32) )
        return; 

    irq_no = (d->irq -32) %32;
    irq_reg = (d->irq -32) /32;

    switch(irq_reg)
    {
        case 0:
            *(u32 *)(SYS_INT_ENABLE1) &= ~(u32) (1 << irq_no);
            break;
        case 1:
            *(u32 *)(SYS_INT_ENABLE2) &= ~(u32) (1 << irq_no);
            break;
        case 2:         
            *(u32 *)(SYS_INT_ENABLE3) &= ~(u32) (1 << irq_no);
            break;  
        default:
            break;
    }
    //printk("%s INT_REG%d, bit %x\n", __FUNCTION__, irq_reg+1, irq_no);
}

#ifdef CONFIG_HAVE_ARM_TWD
static DEFINE_TWD_LOCAL_TIMER(twd_local_timer, A9_MPCORE_TWD, IRQ_LOCALTIMER);

static void __init ali_s3921_twd_init(void)
{
    int err = twd_local_timer_register(&twd_local_timer);
    if (err)
        pr_err("twd_local_timer_register failed %d\n", err);
}
#else
#define ali_s3921_twd_init()    do {} while(0)
#endif

static void __init ali_s3921_init_irq(void)
{
    int i;
    void __iomem *distbase;

    gic_arch_extn.irq_ack       = ali_s3921_irq_disable;
    gic_arch_extn.irq_eoi       = ali_s3921_irq_enable;
    gic_arch_extn.irq_mask      = ali_s3921_irq_disable;
    gic_arch_extn.irq_unmask    = ali_s3921_irq_enable;
//  gic_arch_extn.irq_retrigger = NULL;
//  gic_arch_extn.flags = IRQCHIP_MASK_ON_SUSPEND | IRQCHIP_SKIP_SET_WAKE;

    gic_init(0, GIC_PPI_START, A9_MPCORE_GIC_DIST, A9_MPCORE_GIC_CPU);



//  clk_init();
}



#ifndef CONFIG_ALIUART2
/*  ali_s3921_init_machine */
 static struct platform_device ali_uart = {
    .name       = "ali_uart",
    .id         = -1,
};
#else
/*  ali uart0 */
static struct platform_device ali_uart0 = {
    .name       = "ali_uart",
    .id         = 0,
};
/*  ali uart1 */
 static struct platform_device ali_uart1 = {
    .name       = "ali_uart",
    .id         = 1,
};
#endif

/* OHCI (USB full speed host controller) */
static struct resource ali_usb_ohci_resources_1[] = {
    [0] = {
        .start  = ALI_USB1_OHCI_PHY_BASE,
        .end    = ALI_USB1_OHCI_PHY_BASE+ ALI_USB1_OHCI_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = ALI_IRQ_USB1_OHCI,
        .end    = ALI_IRQ_USB1_OHCI,
        .flags  = IORESOURCE_IRQ,
    },
    [2] = {
        .start  = ALI_USB1_OHCI_PCI_PHY_BASE,
        .end    = ALI_USB1_OHCI_PCI_PHY_BASE+ ALI_USB1_OHCI_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
};


static struct resource ali_usb_ohci_resources_2[] = {
    [0] = {
        .start  = ALI_USB2_OHCI_PHY_BASE,
        .end    = ALI_USB2_OHCI_PHY_BASE+ ALI_USB2_OHCI_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = ALI_IRQ_USB2_OHCI,
        .end    = ALI_IRQ_USB2_OHCI,
        .flags  = IORESOURCE_IRQ,
    },
    [2] = {
        .start  = ALI_USB2_OHCI_PCI_PHY_BASE,
        .end    = ALI_USB2_OHCI_PCI_PHY_BASE+ ALI_USB2_OHCI_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
};

/* The dmamask must be set for OHCI to work */
static u64 ohci_dmamask = DMA_BIT_MASK(32);
static struct platform_device ali_usb1_ohci_device = {
    .name           = "ali-ohci",
    .id         = 0,
    .dev = {
        .dma_mask           = &ohci_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
    },
    .num_resources  = ARRAY_SIZE(ali_usb_ohci_resources_1),
    .resource           = ali_usb_ohci_resources_1,
};

static struct platform_device ali_usb2_ohci_device = {
    .name           = "ali-ohci",
    .id         = 1,
    .dev = {
        .dma_mask           = &ohci_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
    },
    .num_resources  = ARRAY_SIZE(ali_usb_ohci_resources_2),
    .resource           = ali_usb_ohci_resources_2,
};

/* EHCI (USB high speed host controller) */
static struct resource ali_usb_ehci_resources_1[] = {
    [0] = {
        .start  = ALI_USB1_EHCI_PHY_BASE,
        .end        = ALI_USB1_EHCI_PHY_BASE + ALI_USB1_EHCI_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = ALI_IRQ_USB1_EHCI,
        .end        = ALI_IRQ_USB1_EHCI,
        .flags  = IORESOURCE_IRQ,
    },
    [2] = {
        .start  = ALI_USB1_EHCI_PCI_PHY_BASE,
        .end        = ALI_USB1_EHCI_PCI_PHY_BASE + ALI_USB1_EHCI_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
    [3] = {
        .start  = ALI_USB1_HOST_GENERAL_PURPOSE_BASE,
        .end        = ALI_USB1_HOST_GENERAL_PURPOSE_BASE + ALI_USB1_HOST_GENERAL_PURPOSE_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
};


static struct resource ali_usb_ehci_resources_2[] = {
    [0] = {
        .start  = ALI_USB2_EHCI_PHY_BASE,
        .end        = ALI_USB2_EHCI_PHY_BASE + ALI_USB2_EHCI_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = ALI_IRQ_USB2_EHCI,
        .end        = ALI_IRQ_USB2_EHCI,
        .flags  = IORESOURCE_IRQ,
    },
    [2] = {
        .start  = ALI_USB2_EHCI_PCI_PHY_BASE,
        .end        = ALI_USB2_EHCI_PCI_PHY_BASE + ALI_USB1_EHCI_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
    [3] = {
        .start  = ALI_USB2_HOST_GENERAL_PURPOSE_BASE,
        .end        = ALI_USB2_HOST_GENERAL_PURPOSE_BASE + ALI_USB2_HOST_GENERAL_PURPOSE_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
};

static u64 ehci_dmamask = DMA_BIT_MASK(32);
static struct platform_device ali_usb1_ehci_device = {
    .name           = "ali-ehci",
    .id         = 0,
    .dev = {
        .dma_mask       = &ehci_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
    },
    .num_resources  = ARRAY_SIZE(ali_usb_ehci_resources_1),
    .resource   = ali_usb_ehci_resources_1,
};

static struct platform_device ali_usb2_ehci_device = {
    .name           = "ali-ehci",
    .id         = 1,
    .dev = {
        .dma_mask       = &ehci_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
    },
    .num_resources  = ARRAY_SIZE(ali_usb_ehci_resources_2),
    .resource   = ali_usb_ehci_resources_2,
};

/* gadget (USB high speed dev controller) */
static u64 gadget_dmamask = DMA_BIT_MASK(32);
static struct resource ali_usb_gadget_resources[] = {
	[0] = {
		.start	= USB_ALI_GADGET_BASE,
		.end	= USB_ALI_GADGET_BASE + USB_ALI_GADGET_LEN - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= ALI_IRQ_USB_GADGET,
		.end	= ALI_IRQ_USB_GADGET,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.start	= USB_ALI_GADGET_DMA_BASE,
		.end	= USB_ALI_GADGET_DMA_BASE + USB_ALI_GADGET_DMA_LEN - 1,
		.flags	= IORESOURCE_DMA,
	},
};

struct ali_usb_gadget_platform_data {
	int *usb_5v_control_gpio;   /* for C3921 BGA GPIO 73 control usb port 0 5V*/
	int	*usb_5v_detect_gpio;    /* for C3921 BGA GPIO 84 control usb port device port 5V detect*/
};
    
/* Define Platform-dependent data for ali-mci host */
static struct ali_usb_gadget_platform_data ali_usb_gadget_data = 
{
    .usb_5v_control_gpio = &g_usb_p0_host_5v_gpio,      // 73
    .usb_5v_detect_gpio  = &g_usb_device_5v_detect_gpio, // 84
};

static struct platform_device ali_usb_gadget_device = {
	.name		= "alidev_udc",
	.id		= 0,
	.dev = {
		.dma_mask		= &gadget_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
        .platform_data = &ali_usb_gadget_data,		
	},
	.num_resources	= ARRAY_SIZE(ali_usb_gadget_resources),
	.resource	= ali_usb_gadget_resources,
};


// NAND FLASH
static struct resource ali_nand_resources[] = {
    
    [0] = {
        .start  = ALI_NANDREG_BASE,
        .end        = ALI_NANDREG_BASE + ALI_NANDREG_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = ALI_NANDSRAM_BASE,
        .end        = ALI_NANDSRAM_BASE + ALI_NANDSRAM_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },  
    [2] = {
        .start  = ALI_NANDBUF1_BASE,
        .end        = ALI_NANDBUF1_BASE + ALI_NANDBUF_LEN - 1,
        .flags  = IORESOURCE_DMA,
    },
    
    [3] = {
	.start	= ALI_IRQ_NFLASH,
	.end	= ALI_IRQ_NFLASH,
	.flags	= IORESOURCE_IRQ,
    },
/*  
    [3] = {
        .start  = ALI_NANDBUF2_BASE,
        .end        = ALI_NANDBUF2_BASE + ALI_NANDBUF_LEN - 1,
        .flags  = IORESOURCE_DMA,
    },
*/
};

static struct resource ali_spi_resources[] = {
    
    [0] = {
        .start  = ALI_SPIREG_BASE,
        .end    = ALI_SPIREG_BASE + ALI_SPIREG_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = ALI_SPI_CS0_REG_BASE,
        .end    = ALI_SPI_CS0_REG_BASE + ALI_SPIREG_CS0_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
    [2] = {
        .start  = ALI_SPI_CS1_REG_BASE,
        .end    = ALI_SPI_CS1_REG_BASE + ALI_SPIREG_CS1_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
    [3] = {
        .start  = ALI_SPIBUF_BASE,
        .end    = ALI_SPIBUF_BASE + ALI_SPIBUF_LEN - 1,
        .flags  = IORESOURCE_DMA,
    },
    
    [4] = {
			.start	= ALI_IRQ_SPI,
			.end	= ALI_IRQ_SPI,
			.flags	= IORESOURCE_IRQ,
    },
};


static u64 nand_dmamask = DMA_BIT_MASK(32);
static struct platform_device ali_nand_device = {
    .name                   = "ali_nand",
    .id                     = 5,
    .dev = {
        .dma_mask           = &nand_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
    },
    .num_resources          = ARRAY_SIZE(ali_nand_resources),
    .resource                   = ali_nand_resources,
};

static u64 spi_dmamask = DMA_BIT_MASK(32);
static struct platform_device ali_spi_device = {
    .name                   = "ali_spi_bus",
    .id                     = 1,
    .dev = {
        .dma_mask           = &spi_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),		
    },
    .num_resources          = ARRAY_SIZE(ali_spi_resources),
    .resource               = ali_spi_resources,
};


// SDIO/eMMC 
static struct resource ali_sdio_resources[] = {
    
    [0] = {
        .start  = ALI_SDREG_BASE,
        .end        = ALI_SDREG_BASE + ALI_SDREG_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = ALI_SDSRAM_BASE,
        .end        = ALI_NANDSRAM_BASE + ALI_SDSRAM_LEN - 1,
        .flags  = IORESOURCE_MEM,
    },  
    [2] = {
        .start  = ALI_IRQ_SDIO,
        .end        = ALI_IRQ_SDIO,
        .flags  = IORESOURCE_IRQ,
    },
};

/* Define Platform-dependent data for ali-mci host */
static struct alimci_host_platform_data alimci_host_data = 
{
    .host_switch_type = FORCE_SD_ACTIVE,    
    .timeout = HZ/5,
};


/* Define Platform-dependent data for ali-mci host */
static struct alimci_host_platform_data alimci_host_emmc_data = 
{
    .host_switch_type = FORCE_EMMC_ACTIVE,    
    .timeout = HZ/5,
};

static struct platform_device ali_sdio_device = {
    .name                   = SDIO_DRIVER_NAME,
    .id                     = 1,
    .dev = 
    {
        .platform_data = &alimci_host_data,
    },
    .num_resources          = ARRAY_SIZE(ali_sdio_resources),
    .resource                   = ali_sdio_resources,
};


static struct platform_device ali_sdio_emmc_device = {
    .name                   = SDIO_DRIVER_NAME,
    .id                     = 2,
    .dev = 
    {
        .platform_data = &alimci_host_emmc_data,
    },    
};

#if 0
static struct platform_device ali_sdio_device = {
    .name                   = SDIO_DRIVER_NAME,
    .id                     = -1,
    .num_resources          = ARRAY_SIZE(ali_sdio_resources),
    .resource                   = ali_sdio_resources,
};
#endif

/* FB platformat device0 for the GMA1. 
support CLUT8, ARGB1555 and ARGB8888 */
static struct platform_device ali_dev_fb0 = {
	.name		  = "alifb",
	.id		         = 1,
};

/* FB platformat device1 for the video */
static struct platform_device ali_dev_fb1 = {
	.name		  = "alifb",
	.id		         = 2,
};

/* FB platformat device2 for GMA2. */
static struct platform_device ali_dev_fb2 = {
	.name		  = "alifb",
	.id		         = 3,
};


#if defined CONFIG_ALI_PAN_CH455 || defined CONFIG_ALI_PAN_CH454
/* Define I2C GPIO driver platform data as the basic configuration */
static struct i2c_gpio_platform_data i2c_data = 
{
    .sda_pin = I2C_GPIO_SDA_PIN,
    .scl_pin = I2C_GPIO_SCL_PIN,
    .udelay = 5,
    //.timeout = HZ/5,
    .scl_is_open_drain = 0,
    .sda_is_open_drain = 0,
};

/* Define I2C device object using the GPIO PIN  bit-shift emulation adapter*/
static struct platform_device i2c_gpio = 
{
    .name = "i2c-gpio",
    .id = I2C_DEVICE_ID, 
    .dev = 
    {
        .platform_data = &i2c_data,
    },
};
#endif

#ifdef CONFIG_SOUNDBAR_DVBT
/* Define I2C GPIO driver platform data as the basic configuration */
static struct i2c_gpio_platform_data i2c_dvbt2 = 
{
    .sda_pin = I2C_GPIO_SOUNDBAR_DVBT_SDA_PIN,
    .scl_pin = I2C_GPIO_SOUNDBAR_DVBT_SCL_PIN,
    .udelay = 0,
    //.timeout = HZ/5,
    .scl_is_open_drain = 0,
    .sda_is_open_drain = 0,
};

/* Define I2C device object using the GPIO PIN  bit-shift emulation adapter*/
static struct platform_device i2c_gpio_dvbt2 = 
{
    .name = "i2c-gpio",
    .id = I2C_DEVICE_DVBT2_ID, 
    .dev = 
    {
        .platform_data = &i2c_dvbt2,
    },
};
#endif

/* Define I2C GPIO driver platform data as the basic configuration */
static struct i2c_gpio_platform_data i2c_data_hdmi = 
{
    .sda_pin = 74,
    .scl_pin = 75,
    .udelay = 6,
    //.timeout = HZ/5,
    .scl_is_open_drain = 0,
    .sda_is_open_drain = 0,
};

/* Define I2C device object using the GPIO PIN  bit-shift emulation adapter */
static struct platform_device i2c_gpio_hdmi = 
{
    .name = "i2c-gpio",
    .id = 5, 
    .dev = 
    {
        .platform_data = &i2c_data_hdmi,
    },
};

#if 0
/* I2S controller */

static struct resource s3921_i2s_resources[] = {
	{
		.start	= s3921_AIC_BASE_ADDR,
		.end	= s3921_AIC_BASE_ADDR + 0x38 - 1,
		.flags	= IORESOURCE_MEM,
	},
};

struct platform_device s3921_i2s_device = {
	.name		= "m3701-i2s",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(s3921_i2s_resources),
	.resource	= s3921_i2s_resources,
};

/* PCM */
struct platform_device s3921_pcm_device = {
	.name		= "m3701-pcm-audio",
	.id		= -1,
};

/* Codec */
static struct resource s3921_codec_resources[] = {
	{
		.start	= s3921_AIC_BASE_ADDR + 0x80,
		.end	= s3921_AIC_BASE_ADDR + 0x88 - 1,
		.flags	= IORESOURCE_MEM,
	},
};

struct platform_device s3921_codec_device = {
	.name		= "codec_c3701",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(s3921_codec_resources),
	.resource	= s3921_codec_resources,
};
#else
/* I2S controller */

struct platform_device ali_iis_cpu_s3921_device = {
	.name		= "iis_cpu_s3921_dev",
	.id		= -1,
};

/* platform */
struct platform_device ali_asoc_iis_platform_s39_device = {
	.name		= "asoc_iis_platform_s39_dev",
	.id		= -1,
};

/* Codec */


struct platform_device ali_iis_codec_s3921_device = {
	.name		= "iis_codec_s3921_dev",
	.id		= -1,

};

/* spdif controller */



/* platform */
struct platform_device ali_spdif_platform_s39xx_dev= {
	.name		= "asoc_spdif_platform_s39_dev",
	.id		= -1,
};

/* CPU */

struct platform_device ali_spdif_cpu_s3921_device = {
	.name		= "spdif_cpu_s3921_dev",
	.id		= -1,
};


/* Codec */

struct platform_device ali_spdif_codec_s3921_device = {
	.name		= "spdif_codec_s3921_dev",
	.id		= -1,

};


/* ALSA: i2si & i2so mix out device.
 * Date:2015.01.19 by Jingang Chu.
*/
struct platform_device ali_asoc_i2sirx_i2so_mix_capture_device = {
	.name		= "ali_asoc_i2sirx_i2so_mix_cap",
	.id		= -1,
};
#endif

/* ali soundbar device definition */
#ifdef CONFIG_ALI_VOLBAR
struct ali_volbar_platform_data {
	int *volbar_volup_gpio;
	int *volbar_voldn_gpio;
};
    
/* Define Platform-dependent data for ali-mci host */
static struct ali_volbar_platform_data ali_volbar_gpio_data = 
{
    .volbar_volup_gpio = &g_gpio_vol_up,
    .volbar_voldn_gpio  = &g_gpio_vol_down,
};

static struct resource ali_volbar_resources[] = {   
    [0] = {
	.start = ALI_IRQ_GPIO,
	.end	= ALI_IRQ_GPIO,
	.flags = IORESOURCE_IRQ,
    },
};

static struct platform_device ali_volbar_device = {
    .name = "ali_volbar",
    .id = 7,
    .dev = {
       .platform_data = &ali_volbar_gpio_data,
    },
    .num_resources = ARRAY_SIZE(ali_volbar_resources),
    .resource = ali_volbar_resources,
};
#endif

#ifdef CONFIG_ALI_KEYPAD
static struct gpio_keys_button gpio_buttons[] = {
	{
		.gpio = 15,
		.type = EV_KEY,
		.code = KEY_SETUP,
	},
};

struct gpio_keys_platform_data keypad_data = {
	.buttons = gpio_buttons,
	.nbuttons = ARRAY_SIZE(gpio_buttons),
};

static struct resource ali_keypad_resources[] = {   
    [0] = {
	.start = ALI_IRQ_GPIO,
	.end	= ALI_IRQ_GPIO,
	.flags = IORESOURCE_IRQ,
    },
};

static struct platform_device ali_keypad_device = {
	.name = "ali_keypad",
	.id = 8,
	.dev = {
		.platform_data = &keypad_data,
	},
    	.num_resources = ARRAY_SIZE(ali_keypad_resources),
    	.resource = ali_keypad_resources,
};
#endif

#ifdef CONFIG_ALI_PWM
static struct platform_device ali_pwm = {
    .name       = "ali_pwm",
    .id         = -1,
};
#endif

/***********************************************************************/
/***********************************************************************/

static struct platform_device *devices[] __initdata = {
#ifndef CONFIG_ALIUART2
    &ali_uart,
#else
    &ali_uart0,
    &ali_uart1,
#endif
    &ali_nand_device,
    &ali_usb_gadget_device,
    &ali_usb1_ohci_device,
    &ali_usb2_ohci_device,
    &ali_usb1_ehci_device,
    &ali_usb2_ehci_device,
    &ali_dev_fb0,/* FB platformat devices */
    &ali_dev_fb1,
    &ali_dev_fb2,    
	
	#if defined CONFIG_ALI_PAN_CH455 || defined CONFIG_ALI_PAN_CH454
	/* GPIO I2C device object */
	&i2c_gpio,                /*Added by Kinson 2013/07/24*/
	#endif
    &i2c_gpio_hdmi,
#ifdef CONFIG_SOUNDBAR_DVBT
    &i2c_gpio_dvbt2,
#endif
    &ali_iis_cpu_s3921_device,
    &ali_asoc_iis_platform_s39_device,
    &ali_iis_codec_s3921_device,    
    &ali_spdif_platform_s39xx_dev,
    &ali_spdif_cpu_s3921_device,
    &ali_spdif_codec_s3921_device,

    /* ALSA: i2si & i2so mix out device.
     * Date:2015.01.19 by Jingang Chu.
    */	
    &ali_asoc_i2sirx_i2so_mix_capture_device,
	
    &ali_sdio_emmc_device,  
    &ali_sdio_device,
#ifdef CONFIG_ALI_SPI
    &ali_spi_device,
#endif
    #ifdef CONFIG_ALI_VOLBAR
    &ali_volbar_device,
    #endif
    #ifdef CONFIG_ALI_KEYPAD
    &ali_keypad_device,
    #endif

#ifdef CONFIG_ALI_PWM
	&ali_pwm,
#endif
};

static void __init ali_s3921_map_hwbuf
(
    void
)
{
	int                    i;
	int                    hwbuf_desc_cnt;
	struct ali_hwbuf_desc *hwbuf_desc;
	unsigned long          start;
	unsigned long          len;
	unsigned long          end;

	printk("%s,%d\n", __FUNCTION__, __LINE__);

	hwbuf_desc = ali_get_privbuf_desc(&hwbuf_desc_cnt);

	for (i = 0; i < hwbuf_desc_cnt; i++)
	{	
		start = ALI_MEMALIGNDOWN(hwbuf_desc[i].phy_start);

		end = ALI_MEMALIGNUP(hwbuf_desc[i].phy_end);

		len = end - start;

		arm_remap_lowmem_nocache(start, len);
	}

	printk("%s,%d\n", __FUNCTION__, __LINE__);
	
	return;	
}

#ifdef CONFIG_TOUCHSCREEN_FT5X0X
#include <linux/interrupt.h>
#include <ft5x06_ts.h>
#include <s3cfb.h>
static struct ft5x0x_platform_data ft5x0x_pdata = {
        .irq =  11,
        .irq_cfg = 10,
        .x_max = 720, //800,
	.y_max = 480,
	.irqflags = IRQF_TRIGGER_FALLING,
	.reset = 38,
};

static struct i2c_board_info i2c_gpio_devices[] __initdata = {
        {
                I2C_BOARD_INFO("ft5x0x_ts", 0x38),
                .platform_data = &ft5x0x_pdata,
        },
};

static struct s3cfb_lcd at070tn92 = {
        .width = 800,
        .height = 480,
        .bpp = 32,
        .freq = 60,

	.timing = {
		.h_fp	= 210,
		.h_bp	= 38,
		.h_sw	= 10,
		.v_fp	= 22,
		.v_fpe	= 1,
		.v_bp	= 18,
		.v_bpe	= 1,
		.v_sw	= 7,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static void at070tn92_cfg_gpio(struct platform_device *pdev)
{
        int i;

        /*for (i = 0; i < 8; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF0(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF0(i), S3C_GPIO_PULL_NONE);
        }

        for (i = 0; i < 8; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF1(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF1(i), S3C_GPIO_PULL_NONE);
        }

        for (i = 0; i < 8; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF2(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF2(i), S3C_GPIO_PULL_NONE);
        }
        for (i = 0; i < 4; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF3(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF3(i), S3C_GPIO_PULL_NONE);
        }
        */
        /* mDNIe SEL: why we shall write 0x2 ? */
        //writel(0x2, S5P_MDNIE_SEL);
        /* drive strength to max */
       /* writel(0xffffffff, S5PV210_GPF0_BASE + 0xc);
        writel(0xffffffff, S5PV210_GPF1_BASE + 0xc);
        writel(0xffffffff, S5PV210_GPF2_BASE + 0xc);
        writel(0x000000ff, S5PV210_GPF3_BASE + 0xc);
        */
}

static int at070tn92_backlight_on()
{
	// open lcd backlight, gpio16 set 1
    int ret = ali_gpio_set_value(g_lcd_backlight_gpio, 1);
    if(ret < 0)
        printk("at070tn92_backlight_on, gpio16 return error, ret: %d\n"+ret);
    else
        printk("at070tn92_backlight_on, gpio16 return success\n");

	return ret;
}

static int at070tn92_backlight_off()
{
	// open lcd backlight, gpio16 set 1
    int ret = ali_gpio_set_value(g_lcd_backlight_gpio, 0);
    if(ret < 0)
        printk("at070tn92_backlight_off, gpio16 return error, ret: %d\n"+ret);
    else
        printk("at070tn92_backlight_off, gpio16 return success\n");

	return ret;
}

static int at070tn92_reset_lcd(struct platform_device *pdev)
{
	/* LCD_5V */
	//s3c_gpio_cfgpin(S5PV210_GPH0(5), S3C_GPIO_OUTPUT);
	//s3c_gpio_setpull(S5PV210_GPH0(5), S3C_GPIO_PULL_UP);
	//gpio_set_value(S5PV210_GPH0(5), 1);

	/* LCD_33 */
	//s3c_gpio_cfgpin(S5PV210_GPH2(0), S3C_GPIO_OUTPUT);
	//s3c_gpio_setpull(S5PV210_GPH2(0), S3C_GPIO_PULL_DOWN);
	//gpio_set_value(S5PV210_GPH2(0), 0);

	/* wait a moment */
	mdelay(300);

	return 0;
}

static struct s3c_platform_fb at070tn92_fb_data __initdata = {
        .hw_ver = 0x62,
        .clk_name       = "sclk_fimd",
        .nr_wins = 5,
        .default_win = 2,
        .swap = FB_SWAP_WORD | FB_SWAP_HWORD,

        .lcd = &at070tn92,
        .cfg_gpio       = at070tn92_cfg_gpio,
        .backlight_on   = at070tn92_backlight_on,
        .backlight_onoff    = at070tn92_backlight_off,
        .reset_lcd      = at070tn92_reset_lcd,
};

static void enable_dvo_output() 
{
    __REG32ALI(0x18006600) = 0x00268085;
}
#endif

static struct spi_board_info bfin_spi_board_info[] __initdata = {
#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
        {
          .modalias = "spidev",
          .max_speed_hz = 3125000,   /* max spi clock (SCK) speed in HZ */
          .bus_num = 1,
          .chip_select = 0,
        },
#endif
#ifdef CONFIG_INPUT_MAX31855_SPI
        {
          .modalias = "max31855",
          .bus_num = 1,
          .chip_select = 0,
        },
#endif
};

static void __init ali_s3921_init_machine
(
    void
)
{

    printk("%s,%d\n", __FUNCTION__, __LINE__);

    m36_init_gpio();
    platform_add_devices(devices, ARRAY_SIZE(devices));

    ali_s3921_map_hwbuf();
    customize_board_setting();

#ifdef CONFIG_TOUCHSCREEN_FT5X0X

    i2c_register_board_info(0, i2c_gpio_devices, ARRAY_SIZE(i2c_gpio_devices));

     // off lcd backlight, gpio37 set 1
    at070tn92_backlight_off();
    
    enable_dvo_output();

    mdelay(100);

    at070tn92_backlight_on();

    // set gpio  42 for touchscreen reset
    int ret = ali_gpio_set_value(g_lcd_reset_gpio, 0);
    mdelay(10);
    ret = ali_gpio_set_value(g_lcd_reset_gpio, 1);
    if(ret < 0)
        printk("ali_gpio_set_value, gpio42 return error, ret: %d\n"+ret);
    else
        printk("ali_gpio_set_value, gpio42 return success\n");
    
#endif    

   /*SPLL patch for just S3921 ic bug*/
   __REG32ALI(0x180000C0)  =  0x02440308;
   printk("ali_s3921_init_machine SPLL_CPLL_CTRL_REG_3 (0xC0):0x%x\n", __REG32ALI(0x180000C0));

 //      _machine_halt = ali_s3921_poweroff;
        pm_power_off = ali_s3921_poweroff;
 #ifdef CONFIG_SENSORS_MPU6050
	i2c_register_board_info(1, mpu6050_device, ARRAY_SIZE(mpu6050_device));
 #endif
    spi_register_board_info(bfin_spi_board_info, ARRAY_SIZE(bfin_spi_board_info));
	
	return;
}

void __init ali_s3921_reserve_mem
(
    void
)
{
	int                    i;
	int                    hwbuf_desc_cnt;
	struct ali_hwbuf_desc *hwbuf_desc;
	unsigned long          start;
	unsigned long          len;

	printk("%s,%d\n", __FUNCTION__, __LINE__);

	/* Reserve memory for HW buffer.
	*/
	hwbuf_desc = ali_get_privbuf_desc(&hwbuf_desc_cnt);

	for (i = 0; i < hwbuf_desc_cnt; i++)
	{
		start = ALI_MEMALIGNDOWN(hwbuf_desc[i].phy_start);

		len = ALI_MEMALIGNUP(hwbuf_desc[i].phy_end) - ALI_MEMALIGNDOWN(hwbuf_desc[i].phy_start);

		memblock_reserve(start, len);
	}

	printk("%s,%d\n", __FUNCTION__, __LINE__);

	return;
}


void ali_s3921_restart(char mode, const char *cmd)
{


	__REG32ALI(0x18018500) = (0xffffffff - 0x0000000f);	//watch dog count
	__REG8ALI(0x18018504) = 0x67;	//enable watch dog

	while(1);
	return;
}

void ali_s3921_poweroff(void)
{

//power_down_board:
//pmu power down mode
	__REG8ALI(0x1805c101) = 0x01;	 // pmu power down
//pmu gpio mode
//	__REG8ALI(0x1805c05f) |= (0x01<<3);	// power down gpio sel
//	__REG8ALI(0x1805c05e) |= (0x01<<3);	// power down gpio output enable
//	__REG8ALI(0x1805c05d) &= ~(0x01<<3);	// power down gpio output low
	
	while(1);
	return;
}

/*  MACHINE_START */
MACHINE_START(ALI_S3921, BOARD_NAME)
    .atag_offset    = 0x100,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))    
    .smp		= smp_ops(s3921_smp_ops),
#endif    
    .map_io         = ali_s3921_map_io,
    .init_early     = ali_s3921_init_early,
    .init_irq       = ali_s3921_init_irq,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
    .init_time      = ali_s3921_timer_init,
#else
    .timer          = &ali_s3921_timer,
    .handle_irq     = gic_handle_irq,
#endif
    .reserve        = ali_s3921_reserve_mem,//ali_s3921_board_setting,//ali_s3921_reserve_mem,
    .init_machine   = ali_s3921_init_machine,
    .restart	= ali_s3921_restart,
MACHINE_END

