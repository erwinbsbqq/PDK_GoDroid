/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation. 2010 Copyright (C)
 *  (C)
 *  File: m36_platform.c
 *  (I)
 *  Description: mangage the platform devices
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.06.03			Sam			Create
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 *
 ****************************************************************************/
 
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c-gpio.h>
#include <asm/mach-ali/m6303.h>
#include <linux/dma-mapping.h>
#include <ali_board_config.h>
#include <ali_gpio.h>


extern struct fb_var_screeninfo g_alifb_default_var;
extern unsigned long __G_ALI_BOARD_TYPE;

#define ENABLE_USB_CONTROLLOR1
//#define ENABLE_USB_CONTROLLOR2
//#define ENABLE_USB_DEVICE


static struct platform_device ali_uart8250_device = {
	.name		= "ali_uart",
	.id			= -1,
};

/* really no use !!*/
#if 0
static struct platform_device ali_hcd_device = {
	.name		= "ali_hcd",
	.id			= -2,
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

/* FB platformat device2 for GMA2. 
only support CLUT8 */
static struct platform_device ali_dev_fb2 = {
	.name		  = "alifb",
	.id		         = 3,
};

#define GPIO_I2C_SDA     GPIO_NOT_USED
#define GPIO_I2C_SCL     GPIO_NOT_USED

/* SDI */
#define ALI_MCI_SDI 0x5A000000
#define ALI_MCI_SZ  0x00100000 //1M
#define IRQ_SDI     18         //???

static struct resource ali_sdi_resource[] = {
	[0] = {
		.start = ALI_MCI_SDI,
		.end   = ALI_MCI_SDI + ALI_MCI_SZ - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SDI,
		.end   = IRQ_SDI,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device ali_mci_sdi_device = {
	.name		  = "ali-mci",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(ali_sdi_resource),
	.resource	  = ali_sdi_resource,
};

static struct i2c_gpio_platform_data ali_hdmi_eddc = {
		.sda_pin    = HDMI_EDDC_SDA,
		.scl_pin    = HDMI_EDDC_SCL,
		.udelay     = 5,     // SCL frequency is (500 / udelay) kHz
		//.timeout    = 100,
		.sda_is_open_drain  = 0,
		.scl_is_open_drain  = 0,
		.scl_is_output_only = 0    
};

static struct platform_device m3602_gpio_i2c_device = {
	.name		= "i2c-gpio",
	.id			= 5,
	.dev		= {
		.platform_data	= &ali_hdmi_eddc,
	},
};


static struct i2c_gpio_platform_data gpio_i2c_panel_data = {
		.sda_pin    = GPIO_PORT_MAX + 1,
		.scl_pin    =   GPIO_PORT_MAX + 1,
		.udelay     = 6,     // SCL frequency is (500 / udelay) kHz
		//.timeout    = 100,
		.sda_is_open_drain  = 0,
		.scl_is_open_drain  = 0,
		.scl_is_output_only = 0    
};

static struct platform_device gpio_i2c_panel = {
	.name		= "i2c-gpio",
	.id			= 0xff,
	.dev		= {
		.platform_data	= &gpio_i2c_panel_data,
	},
};

#ifdef ENABLE_USB_CONTROLLOR1
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
#endif

#ifdef ENABLE_USB_CONTROLLOR2
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
#endif


#ifdef ENABLE_USB_DEVICE
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
	int * usb_5v_control_gpio;   /* for M3515 BGA No GPIO control usb port 0 5V, set 0*/
	int * usb_5v_detect_gpio;    /* for M3515 BGA GPIO 84 control usb port device port 5V detect*/
};

/* Define Platform-dependent data for ali-mci host */
static struct ali_usb_gadget_platform_data ali_usb_gadget_data = 
{
    .usb_5v_control_gpio = (int *) &g_usb_p0_host_5v_gpio,      /* 73 */
    .usb_5v_detect_gpio  = (int *) &g_usb_device_5v_detect_gpio, /* 84 */
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
#endif

static u64 nand_dmamask = DMA_BIT_MASK(32);

#define ALI_IRQ_NFLASH        (32 + 16 + 8)
static struct resource ali_nand_resources[] = {
	
	[0] = {
		.start	= ALI_NANDREG_BASE,
		.end		= ALI_NANDREG_BASE + ALI_NANDREG_LEN - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= ALI_NANDSRAM_BASE,
		.end		= ALI_NANDSRAM_BASE + ALI_NANDSRAM_LEN - 1,
		.flags	= IORESOURCE_MEM,
	},	
	[2] = {
		.start	= ALI_NANDBUF1_BASE,
		.end		= ALI_NANDBUF1_BASE + ALI_NANDBUF_LEN - 1,
		.flags	= IORESOURCE_DMA,
	},
	[3] = {
		.start	= ALI_IRQ_NFLASH,
		.end		= ALI_IRQ_NFLASH,
		.flags	= IORESOURCE_IRQ,
	},
/*	
	[3] = {
		.start	= ALI_NANDBUF2_BASE,
		.end		= ALI_NANDBUF2_BASE + ALI_NANDBUF_LEN - 1,
		.flags	= IORESOURCE_DMA,
	},		*/
};

static struct platform_device ali_nand_device = {
	.name			= "ali_nand",
	.id				= 5,
	.dev = {
		.dma_mask			= &nand_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
	.num_resources	= ARRAY_SIZE(ali_nand_resources),
	.resource			= ali_nand_resources,
};

static struct platform_device *ali_platform_devices[] __initdata = {
	&ali_uart8250_device,

#ifdef ENABLE_USB_DEVICE	
	&ali_usb_gadget_device,
#endif	

#ifdef ENABLE_USB_CONTROLLOR1
	&ali_usb1_ohci_device,
	&ali_usb1_ehci_device,
#endif	

#ifdef ENABLE_USB_CONTROLLOR2
	&ali_usb2_ohci_device,
	&ali_usb2_ehci_device,
#endif	

	&ali_dev_fb0,/* FB platformat devices */
	&ali_dev_fb1,
	&ali_dev_fb2,
	&m3602_gpio_i2c_device,
	&ali_mci_sdi_device,	
	&ali_nand_device,
	&gpio_i2c_panel,
};


static int __init ali_add_devices(void)
{
	int err;

	
   	/*if(0x3701==*((volatile unsigned short *)0xb8000002))
	{	   			
		ali_hdmi_eddc.sda_pin = 33;
		ali_hdmi_eddc.scl_pin = 34;						
   	} */ 	

	gpio_i2c_panel.id = g_gpio_i2c_panel_id;
   	gpio_i2c_panel_data.sda_pin = g_gpio_i2c_panel_sda;
   	gpio_i2c_panel_data.scl_pin = g_gpio_i2c_panel_scl;
	  
	err = platform_add_devices(ali_platform_devices, ARRAY_SIZE(ali_platform_devices));
	if (err)
	{
		printk("[ %s %d ], ali platform err %d\n", __FUNCTION__, __LINE__, err);
	}


	return 0;
}
arch_initcall(ali_add_devices);

