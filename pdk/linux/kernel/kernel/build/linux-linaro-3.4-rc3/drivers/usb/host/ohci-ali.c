
#include <linux/platform_device.h>
#include <linux/signal.h>
#include "ali_reg.h"

static int __devinit ohci_ali_start(struct usb_hcd *hcd)
{
	struct ohci_hcd	*ohci = hcd_to_ohci(hcd);
	int ret;

	ohci_dbg(ohci, "ohci_ali_start, ohci:%p", ohci);

	if ((ret = ohci_init(ohci)) < 0)
		return ret;

	if ((ret = ohci_run(ohci)) < 0) {
		err ("can't start %s", hcd->self.bus_name);
		ohci_stop(hcd);
		return ret;
	}
	return 0;
}

static const struct hc_driver ohci_ali_hc_driver = {
	.description =		hcd_name,
	.product_desc =		"ALi OHCI",
	.hcd_priv_size =	sizeof(struct ohci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq =			ohci_irq,
	.flags =		HCD_USB11 | HCD_MEMORY,

	/*
	 * basic lifecycle operations
	 */
	.start =			ohci_ali_start,
	.stop =			ohci_stop,
	.shutdown =		ohci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		ohci_urb_enqueue,
	.urb_dequeue =		ohci_urb_dequeue,
	.endpoint_disable =	ohci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number =	ohci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data =	ohci_hub_status_data,
	.hub_control =		ohci_hub_control,
#ifdef	CONFIG_PM
	.bus_suspend =		ohci_bus_suspend,
	.bus_resume =		ohci_bus_resume,
#endif
	.start_port_reset =	ohci_start_port_reset,
};

static void ali_start_ohc(struct platform_device *pdev)
{
	/* enable host controller */

	/* enable EHCI mmio */
	u64 rsrc_start, rsrc_len;
	void __iomem		*pci_regs;

	rsrc_start = pdev->resource[2].start;
	rsrc_len = pdev->resource[2].end - pdev->resource[2].start + 1;
	
	if (!request_mem_region(rsrc_start, rsrc_len, "ALi-OHCI (PCI config regs)")) {
		pr_debug("request_mem_region failed");
	}

	pci_regs = ioremap(rsrc_start, rsrc_len);
	if (!pci_regs) {
		pr_debug("ioremap failed");
	}
	
	writel(0x02900157, (pci_regs + 0x04));
	//writel(0x00002008, (pci_regs + 0x0c)); //0x00008010;
	writel(pdev->resource[0].start, (pci_regs + 0x10));
	//writel(0x1000041E, (pci_regs + 0x44));	
	
#if 0
	if ((readl(0xB8000000) & 0xFFFF0000) != 0x37010000)
	{
		(*(volatile u32*)0xB803D810) = (*(volatile u32*)0xB803D810) | 0x0049AB64;
	}

	if ((readl(0xB8000000) & 0xFFFF0000) == 0x37010000)
	{
		
		//(*(volatile u32*)0xB803D814) =  0x0005C9D4;;	//20120808 change GPSR14 register 
		writel(0x0005C9D4, (gadget_regs + 0x14));
		//(*(volatile u32*)0xB803D810) &= 0x80FFFFFF;
		writel(readl(gadget_regs + 0x14) & 0x80FFFFFF, (gadget_regs + 0x14));
		//(*(volatile u32*)0xB803D810) |= 0x87FFFFFF;
		writel(readl(gadget_regs + 0x14) | 0x87FFFFFF, (gadget_regs + 0x14));
	}
#endif	
}

static void ali_stop_ohc(struct platform_device *pdev)
{
	/* Disable mem */
	u64 rsrc_start, rsrc_len;

	rsrc_start = pdev->resource[2].start;
	rsrc_len = pdev->resource[2].end - pdev->resource[2].start + 1;
	
	release_mem_region(rsrc_start, rsrc_len);
	
	/* Disable clock */
}

static int ohci_hcd_ali_drv_probe(struct platform_device *pdev)
{
	int ret;
	struct usb_hcd *hcd;

	printk("%s\n",__func__);
	
	if (0 == __REG32ALI(0x1803A000))
		return -ENOMEM;	
		
	if (pdev->resource[1].flags != IORESOURCE_IRQ) {
		pr_debug("resource[1] is not IORESOURCE_IRQ\n");
		return -ENOMEM;
	}

	hcd = usb_create_hcd(&ohci_ali_hc_driver, &pdev->dev, "ALi");
	if (!hcd)
		return -ENOMEM;

	hcd->rsrc_start = pdev->resource[0].start;
	hcd->rsrc_len = pdev->resource[0].end - pdev->resource[0].start + 1;

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		pr_debug("request_mem_region failed\n");
		ret = -EBUSY;
		goto err1;
	}

	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		pr_debug("ioremap failed\n");
		ret = -ENOMEM;
		goto err2;
	}

	ali_start_ohc(pdev);
	ohci_hcd_init(hcd_to_ohci(hcd));

	ret = usb_add_hcd(hcd, pdev->resource[1].start,
			  IRQF_DISABLED /*| IRQF_SHARED*/);
	if (ret == 0) {
		platform_set_drvdata(pdev, hcd);
		return ret;
	}

	ali_stop_ohc(pdev);
	iounmap(hcd->regs);
err2:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
err1:
	usb_put_hcd(hcd);
	return ret;
}

static int ohci_hcd_ali_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_remove_hcd(hcd);
	ali_stop_ohc(pdev);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

#ifdef CONFIG_PM
static int ohci_hcd_ali_drv_suspend(struct platform_device *dev, pm_message_t message)
{
	struct ohci_hcd	*ohci = hcd_to_ohci(platform_get_drvdata(dev));

	if (time_before(jiffies, ohci->next_statechange))
		msleep(5);
	ohci->next_statechange = jiffies;

	ali_stop_ohc(dev);
	ohci_to_hcd(ohci)->state = HC_STATE_SUSPENDED;
	return 0;
}

static int ohci_hcd_ali_drv_resume(struct platform_device *dev)
{
	struct usb_hcd	*hcd = platform_get_drvdata(dev);
	struct ohci_hcd	*ohci = hcd_to_ohci(hcd);

	if (time_before(jiffies, ohci->next_statechange))
		msleep(5);
	ohci->next_statechange = jiffies;

	ali_start_ohc(dev);
	ohci_finish_controller_resume(hcd);
	return 0;
}
#endif

static struct platform_driver ohci_hcd_ali_driver = {
	.probe		= ohci_hcd_ali_drv_probe,
	.remove		= ohci_hcd_ali_drv_remove,
	.shutdown	= usb_hcd_platform_shutdown,
#ifdef	CONFIG_PM
	.suspend		= ohci_hcd_ali_drv_suspend,
	.resume		= ohci_hcd_ali_drv_resume,
#endif
	.driver		= {
		.name	= "ali-ohci",
		.owner	= THIS_MODULE,
	},
};

MODULE_ALIAS("platform:ali-ohci");
