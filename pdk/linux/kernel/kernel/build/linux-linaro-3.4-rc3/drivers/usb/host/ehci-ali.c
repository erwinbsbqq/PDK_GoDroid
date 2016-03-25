#include <linux/platform_device.h>

#ifdef CONFIG_ARM
#include <mach/ali-s3921.h>
#endif
#include "ali_otp_common.h"
#include "ali_reg.h"
#include <asm/mach-ali/m36_gpio.h>
#include <ali_board_config.h>

#define ALI_USB_EHCI_IO1 0x1803A000
#define C3921	0x3921	
#define C3503 0x3503
#define C3821 0x3821
#define C3701 0x3701

struct ali_usb_setting {
	int reg_addr;   	
	int reg_val;    	 	
};

/* Define Platform-dependent data for ali-usb host */
/* c39_pkg_ver_setting
*/	
static struct ali_usb_setting C3921_XXX_V01_setting [6] = 
{
    [0] ={.reg_addr = 0x04,    .reg_val  = 0x00000000,},	/* Host Mode */
    [1] ={.reg_addr = 0x10,    .reg_val  = 0x83000000,},	/* pll */
    [2] ={.reg_addr = 0x14,    .reg_val  = 0x36e836e8,},	/* phy */    
    [3] ={.reg_addr = 0x24,    .reg_val  = 0x00080000,},	/* disconnect count */
    [4] ={.reg_addr = 0x20,    .reg_val  = 0x01000000,},	/* New CDR */
    [5] ={.reg_addr = -1,    	 .reg_val  = -1,},	/* end */
};    

static struct ali_usb_setting C3503_BGA_XXX_setting [5] = 
{
    [0] ={.reg_addr = 0x04,    .reg_val  = 0x00000000,},	/* Host Mode */
    [1] ={.reg_addr = 0x10,    .reg_val  = 0x07000000,},	/* pll */
    [2] ={.reg_addr = 0x14,    .reg_val  = 0x001F4A74,},	/* phy */    
    [3] ={.reg_addr = 0x24,    .reg_val  = 0x00080000,},	/* disconnect count */    
    [4] ={.reg_addr = -1,    	 .reg_val  = -1,},	/* end */
};    

static struct ali_usb_setting C3503_QFP_XXX_setting [5] = 
{
    [0] ={.reg_addr = 0x04,    .reg_val  = 0x00000000,},	/* Host Mode */
    [1] ={.reg_addr = 0x10,    .reg_val  = 0x07000000,},	/* pll */
    [2] ={.reg_addr = 0x14,    .reg_val  = 0x001D4954,},	/* phy */    
    [3] ={.reg_addr = 0x24,    .reg_val  = 0x00080000,},	/* disconnect count */    
    [4] ={.reg_addr = -1,    	 .reg_val  = -1,},	/* end */
};    

static struct ali_usb_setting C3821_QFP128_XXX_setting [8] = 
{
    [0] ={.reg_addr = 0x04,    .reg_val  = 0x00000000,},	/* Host Mode */
    [1] ={.reg_addr = 0x10,    .reg_val  = 0x03000000,},	/* pll */
    [2] ={.reg_addr = 0x14,    .reg_val  = 0x32E432E4,},	/* phy */    
    [3] ={.reg_addr = 0x18,    .reg_val  = 0x00000000,},	/*  */	
    [4] ={.reg_addr = 0x20,    .reg_val  = 0x01000000,},	/* New CDR */	
    [5] ={.reg_addr = 0x24,    .reg_val  = 0x00080000,},	/* disconnect count */        
    [6] ={.reg_addr = 0x2C,    .reg_val  = 0x00000000,},	/* TRIM_CTRL */	
    [7] ={.reg_addr = -1,    	 .reg_val  = -1,},	/* end */
};    

static struct ali_usb_setting C3821_QFP156_XXX_setting [8] = 
{
    [0] ={.reg_addr = 0x04,    .reg_val  = 0x00000000,},	/* Host Mode */
    [1] ={.reg_addr = 0x10,    .reg_val  = 0x07000000,},	/* pll */
    [2] ={.reg_addr = 0x14,    .reg_val  = 0x32083208,},	/* phy */    
    [3] ={.reg_addr = 0x18,    .reg_val  = 0x00000000,},	/*  */	
    [4] ={.reg_addr = 0x20,    .reg_val  = 0x01000000,},	/* New CDR */	
    [5] ={.reg_addr = 0x24,    .reg_val  = 0x00080000,},	/* disconnect count */        
    [6] ={.reg_addr = 0x2C,    .reg_val  = 0x00000000,},	/* TRIM_CTRL */	
    [7] ={.reg_addr = -1,    	 .reg_val  = -1,},	/* end */
}; 

static struct ali_usb_setting C3821_BGA_XXX_setting [8] = 
{
    [0] ={.reg_addr = 0x04,    .reg_val  = 0x00000000,},	/* Host Mode */
    [1] ={.reg_addr = 0x10,    .reg_val  = 0x03000000,},	/* pll */
    [2] ={.reg_addr = 0x14,    .reg_val  = 0x32E432E4,},	/* phy */    
    [3] ={.reg_addr = 0x18,    .reg_val  = 0x00000000,},	/*  */	
    [4] ={.reg_addr = 0x20,    .reg_val  = 0x01000000,},	/* New CDR */	
    [5] ={.reg_addr = 0x24,    .reg_val  = 0x00080000,},	/* disconnect count */        
    [6] ={.reg_addr = 0x2C,    .reg_val  = 0x00000000,},	/* TRIM_CTRL */	
    [7] ={.reg_addr = -1,    	 .reg_val  = -1,},	/* end */
};   

static struct ali_usb_setting C3701_XXX_V01_setting [4] = 
{
    [0] ={.reg_addr = 0x04,    .reg_val  = 0x00000000,},	/* Host Mode */
    [1] ={.reg_addr = 0x10,    .reg_val  = 0x87000000,},	/* pll */    
    [2] ={.reg_addr = 0x14,    .reg_val  = 0x00054954,},	/* phy */    
    [3] ={.reg_addr = -1,    	 .reg_val  = -1,},	/* end */
};   
 
bool patch_s3921 = 0;

static int ali_ehci_hc_reset(struct usb_hcd *hcd)
{
	int result;
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);

	result = ehci_setup(hcd);
	if (result)
	{
		printk("%s, init\n",__func__);
		return result;
	}	

	ehci_reset(ehci);
	return result;
}

static const struct hc_driver ehci_ali_hc_driver = {
	.description		= "ehci hcd",
	.product_desc		= "ALi EHCI",
	.hcd_priv_size		= sizeof(struct ehci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq			= ehci_irq,
	.flags			= HCD_MEMORY | HCD_USB2,

	/*
	 * basic lifecycle operations
	 *
	 * FIXME -- ehci_init() doesn't do enough here.
	 * See ehci-ppc-soc for a complete implementation.
	 */
	.reset			= ali_ehci_hc_reset,
	.start			= ehci_run,
	.stop			= ehci_stop,
	.shutdown		= ehci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue		= ehci_urb_enqueue,
	.urb_dequeue		= ehci_urb_dequeue,
	.endpoint_disable	= ehci_endpoint_disable,
	//.endpoint_reset		= ehci_endpoint_reset,

	/*
	 * scheduling support
	 */
	.get_frame_number	= ehci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
	.bus_suspend		= ehci_bus_suspend,
	.bus_resume		= ehci_bus_resume,
	.relinquish_port	= ehci_relinquish_port,
	.port_handed_over	= ehci_port_handed_over,

	//.clear_tt_buffer_complete	= ehci_clear_tt_buffer_complete,
};

static const struct hc_driver ehci_ali_hc_driver_2 = {
	.description		= "ehci hcd 2",
	.product_desc		= "ALi EHCI 2",
	.hcd_priv_size		= sizeof(struct ehci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq			= ehci_irq,
	.flags			= HCD_MEMORY | HCD_USB2,

	/*
	 * basic lifecycle operations
	 *
	 * FIXME -- ehci_init() doesn't do enough here.
	 * See ehci-ppc-soc for a complete implementation.
	 */
	.reset			= ali_ehci_hc_reset,
	.start			= ehci_run,
	.stop			= ehci_stop,
	.shutdown		= ehci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue		= ehci_urb_enqueue,
	.urb_dequeue		= ehci_urb_dequeue,
	.endpoint_disable	= ehci_endpoint_disable,
	//.endpoint_reset		= ehci_endpoint_reset,

	/*
	 * scheduling support
	 */
	.get_frame_number	= ehci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
	.bus_suspend		= ehci_bus_suspend,
	.bus_resume		= ehci_bus_resume,
	.relinquish_port	= ehci_relinquish_port,
	.port_handed_over	= ehci_port_handed_over,

	//.clear_tt_buffer_complete	= ehci_clear_tt_buffer_complete,
};

static void ali_start_ehc(struct platform_device *pdev)
{
	struct ali_usb_setting *hw_setting;
	u64 rsrc_start, rsrc_len;
	void __iomem	*pci_regs;
	void __iomem	*phy_regs;
	bool probe_1st = 0;
		
	u32 chip_id = 0;
	u32 chip_package = 0;
	u32 chip_ver = 0;	
	u32 trim = 0;
	int idx = 0;
	
	chip_id = (__REG32ALI(0x18000000) >> 16) & 0xFFFF;	
	chip_package = (__REG32ALI(0x18000000) >> 8) & 0xFF;	
	chip_ver = __REG32ALI(0x18000000) & 0xFF;	
		
	printk("Chip %04x, Package %02x, Version %02x\n", chip_id, chip_package, chip_ver);
	
	rsrc_start = pdev->resource[2].start;
	rsrc_len = pdev->resource[2].end - pdev->resource[2].start + 1;
	
	if (!request_mem_region(rsrc_start, rsrc_len, "ALi-EHCI (PCI config regs)")) {
		pr_debug("request_mem_region failed");
	}

	pci_regs = ioremap(rsrc_start, rsrc_len);
	if (!pci_regs) {
		pr_debug("pci ioremap failed");
	}
	printk("%s  pci_regs=(0x%llx, 0x%p)\n",__func__, rsrc_start, pci_regs);
	
	/* enable USB Host mode*/
	if (ALI_USB_EHCI_IO1 == rsrc_start)
	{	
		probe_1st = 1;		
		printk("probe 1st USB Host\n");	
	}
	else
	{
		printk("probe 2nd USB Host\n");	
	}
					
	writel(0x02900157, (pci_regs + 0x04));
	writel(0x00008008, (pci_regs + 0x0c)); 	
	writel(pdev->resource[0].start, (pci_regs + 0x10));	
	writel(0x1000041E, (pci_regs + 0x44));
	writel(0xC4000075, (pci_regs + 0x40)); 
	
	rsrc_start = pdev->resource[3].start;
	rsrc_len = pdev->resource[3].end - pdev->resource[3].start + 1;
	
	if (!request_mem_region(rsrc_start, rsrc_len, "ALi-EHCI (PHY regs)")) {
		pr_debug("request_mem_region failed");
	}

	phy_regs = ioremap(rsrc_start, rsrc_len);
	if (!phy_regs) {
		pr_debug("phy ioremap failed");
	}
	
	printk("%s  phy_regs=(0x%llx, 0x%p)\n",__func__, rsrc_start, phy_regs);		
	switch (chip_id)
	{
		case C3921:
			hw_setting = &C3921_XXX_V01_setting;
    	if (0x01 == chip_package)									/*BGA package*/
			{
					/*turn on 5V*/
					if (probe_1st)
					{						
						if (g_usb_p0_host_5v_gpio > 0)
						{	
							gpio_enable_pin(g_usb_p0_host_5v_gpio);
							ali_gpio_direction_output(g_usb_p0_host_5v_gpio, 1);
						}	
					}
			}					
			ali_otp_read(0xE1*4, (unsigned char *) &trim, 4);	
    	printk("trim = %x\n", trim);	
    	if (probe_1st)
    	{	
    		trim &= 0x003f0000; //[18:16][21:19]    			
    	}	
    	else
    	{
    		trim &= 0x00fc0000;	//[24:22][27:15]
    	}
    	writel(readl(phy_regs + 0x14) | trim , (phy_regs + 0x14));		
			break;
		
		case C3503:					
	    if ((chip_package == 0x10) && (chip_ver == 0x02))	
	    	hw_setting = &C3503_BGA_XXX_setting;
			else					
	    	hw_setting = &C3503_QFP_XXX_setting;
			break;
		
		case C3821:
			if ((0x01 == chip_package) || (0x00 == ((chip_package >>3) & 0x03)))	/* BGA pin*/
				hw_setting = &C3821_BGA_XXX_setting;
			else if (0x02 == ((chip_package >>3) & 0x03) )	/* QFP156 pin*/	
				hw_setting = &C3821_QFP156_XXX_setting;
			else																						/*QFP 128 pin*/				
				hw_setting = &C3821_QFP128_XXX_setting;
			break;	
		
		case C3701:
			hw_setting = &C3701_XXX_V01_setting;
			break;
			
		default:
			printk("Not Support Chip !!\n");	
			break;	
	}
	if (hw_setting)
	{
		while(1)
		{
			if (-1 == hw_setting[idx].reg_addr)
				break;
			writel(hw_setting[idx].reg_val, (phy_regs + hw_setting[idx].reg_addr));	
			printk("Wr USB setting (0x%02x ,0x%08x) (rd back 0x%08x)\n", hw_setting[idx].reg_addr, 
					hw_setting[idx].reg_val, readl(phy_regs + hw_setting[idx].reg_addr));	
			idx++;		
		}
	}		
}

static void ali_stop_ehc(struct platform_device *pdev)
{
	/* Disable mem */
	u64 rsrc_start, rsrc_len;

	rsrc_start = pdev->resource[2].start;
	rsrc_len = pdev->resource[2].end - pdev->resource[2].start + 1;	
	release_mem_region(rsrc_start, rsrc_len);	
	/* Disable EHC clock. If the HS PHY is unused disable it too. */
}

static int ehci_hcd_ali_drv_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	struct ehci_hcd *ehci;
	int ret;

	printk("%s\n",__func__);	

	if (pdev->resource[1].flags != IORESOURCE_IRQ) {
		pr_debug("resource[1] is not IORESOURCE_IRQ");
		return -ENOMEM;
	}

	hcd = usb_create_hcd(&ehci_ali_hc_driver, &pdev->dev, "ALi HCD");		
	if (!hcd)
		return -ENOMEM;

	hcd->rsrc_start = pdev->resource[0].start;
	hcd->rsrc_len = pdev->resource[0].end - pdev->resource[0].start + 1;
	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		pr_debug("request_mem_region failed");
		ret = -EBUSY;
		goto err1;
	}

	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		pr_debug("ioremap failed");
		ret = -ENOMEM;
		goto err2;
	}

	printk("%s, EHCI reg = 0x%x, 0x%p\n",__func__,hcd->rsrc_start , hcd->regs);
	ali_start_ehc(pdev);

	ehci = hcd_to_ehci(hcd);
	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs + HC_LENGTH(ehci, readl(&ehci->caps->hc_capbase));
	/* ali patch setting */
	writel((readl(&ehci->caps->hcc_params)&0xFFFE), &ehci->caps->hcc_params);   //32bits mode
	writel(((readl(&ehci->caps->hcs_params)&0xFFF0)|0x3), &ehci->caps->hcs_params);   //three ports in M3901
	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = readl(&ehci->caps->hcs_params);

	ret = usb_add_hcd(hcd, pdev->resource[1].start,
			  IRQF_DISABLED /*| IRQF_SHARED*/);
	if (ret == 0) {
		platform_set_drvdata(pdev, hcd);
			printk("%s OK\n",__func__);
		return ret;
	}

	ali_stop_ehc(pdev);
	iounmap(hcd->regs);
err2:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
err1:
	usb_put_hcd(hcd);
	return ret;
}

static int ehci_hcd_ali_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
	ali_stop_ehc(pdev);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static struct platform_driver ehci_hcd_ali_driver = {
	.probe		= ehci_hcd_ali_drv_probe,
	.remove		= ehci_hcd_ali_drv_remove,
	.shutdown	= usb_hcd_platform_shutdown,
	.driver = {
		.name	= "ali-ehci",
		.owner	= THIS_MODULE,
	}
};

MODULE_ALIAS("platform:ali-ehci");


