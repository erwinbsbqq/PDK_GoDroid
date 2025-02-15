/*
 * USB device controllers have lots of quirks.  Use these macros in
 * gadget drivers or other code that needs to deal with them, and which
 * autoconfigures instead of using early binding to the hardware.
 *
 * This SHOULD eventually work like the ARM mach_is_*() stuff, driven by
 * some config file that gets updated as new hardware is supported.
 * (And avoiding all runtime comparisons in typical one-choice configs!)
 *
 * NOTE:  some of these controller drivers may not be available yet.
 * Some are available on 2.4 kernels; several are available, but not
 * yet pushed in the 2.6 mainline tree.
 */

#ifndef __GADGET_CHIPS_H
#define __GADGET_CHIPS_H

/*
 * NOTICE: the entries below are alphabetical and should be kept
 * that way.
 *
 * Always be sure to add new entries to the correct position or
 * accept the bashing later.
 *
 * If you have forgotten the alphabetical order let VIM/EMACS
 * do that for you.
 */

#define gadget_is_aliudc(g)		(!strcmp("alidev_udc", (g)->name)) 
#define gadget_is_amd5536udc(g)		(!strcmp("amd5536udc", (g)->name))
#define gadget_is_at91(g)		(!strcmp("at91_udc", (g)->name))
#define gadget_is_atmel_usba(g)		(!strcmp("atmel_usba_udc", (g)->name))
#define gadget_is_ci13xxx_msm(g)	(!strcmp("ci13xxx_msm", (g)->name))
#define gadget_is_ci13xxx_pci(g)	(!strcmp("ci13xxx_pci", (g)->name))
#define gadget_is_dummy(g)		(!strcmp("dummy_udc", (g)->name))
#define gadget_is_dwc3(g)		(!strcmp("dwc3-gadget", (g)->name))
#define gadget_is_fsl_qe(g)		(!strcmp("fsl_qe_udc", (g)->name))
#define gadget_is_fsl_usb2(g)		(!strcmp("fsl-usb2-udc", (g)->name))
#define gadget_is_goku(g)		(!strcmp("goku_udc", (g)->name))
#define gadget_is_imx(g)		(!strcmp("imx_udc", (g)->name))
#define gadget_is_langwell(g)		(!strcmp("langwell_udc", (g)->name))
#define gadget_is_m66592(g)		(!strcmp("m66592_udc", (g)->name))
#define gadget_is_musbhdrc(g)		(!strcmp("musb-hdrc", (g)->name))
#define gadget_is_net2272(g)		(!strcmp("net2272", (g)->name))
#define gadget_is_net2280(g)		(!strcmp("net2280", (g)->name))
#define gadget_is_omap(g)		(!strcmp("omap_udc", (g)->name))
#define gadget_is_pch(g)		(!strcmp("pch_udc", (g)->name))
#define gadget_is_pxa(g)		(!strcmp("pxa25x_udc", (g)->name))
#define gadget_is_pxa27x(g)		(!strcmp("pxa27x_udc", (g)->name))
#define gadget_is_r8a66597(g)		(!strcmp("r8a66597_udc", (g)->name))
#define gadget_is_renesas_usbhs(g)	(!strcmp("renesas_usbhs_udc", (g)->name))
#define gadget_is_s3c2410(g)		(!strcmp("s3c2410_udc", (g)->name))
#define gadget_is_s3c_hsotg(g)		(!strcmp("s3c-hsotg", (g)->name))
#define gadget_is_s3c_hsudc(g)		(!strcmp("s3c-hsudc", (g)->name))

/**
 * usb_gadget_controller_number - support bcdDevice id convention
 * @gadget: the controller being driven
 *
 * Return a 2-digit BCD value associated with the peripheral controller,
 * suitable for use as part of a bcdDevice value, or a negative error code.
 *
 * NOTE:  this convention is purely optional, and has no meaning in terms of
 * any USB specification.  If you want to use a different convention in your
 * gadget driver firmware -- maybe a more formal revision ID -- feel free.
 *
 * Hosts see these bcdDevice numbers, and are allowed (but not encouraged!)
 * to change their behavior accordingly.  For example it might help avoiding
 * some chip bug.
 */
static inline int usb_gadget_controller_number(struct usb_gadget *gadget)
{
	if (gadget_is_net2280(gadget))
		return 0x01;
	else if (gadget_is_dummy(gadget))
		return 0x02;
	else if (gadget_is_pxa(gadget))
		return 0x03;
	else if (gadget_is_goku(gadget))
		return 0x06;
	else if (gadget_is_omap(gadget))
		return 0x08;
	else if (gadget_is_pxa27x(gadget))
		return 0x11;
	else if (gadget_is_s3c2410(gadget))
		return 0x12;
	else if (gadget_is_at91(gadget))
		return 0x13;
	else if (gadget_is_imx(gadget))
		return 0x14;
	else if (gadget_is_musbhdrc(gadget))
		return 0x16;
	else if (gadget_is_atmel_usba(gadget))
		return 0x18;
	else if (gadget_is_fsl_usb2(gadget))
		return 0x19;
	else if (gadget_is_amd5536udc(gadget))
		return 0x20;
	else if (gadget_is_m66592(gadget))
		return 0x21;
	else if (gadget_is_fsl_qe(gadget))
		return 0x22;
	else if (gadget_is_ci13xxx_pci(gadget))
		return 0x23;
	else if (gadget_is_langwell(gadget))
		return 0x24;
	else if (gadget_is_r8a66597(gadget))
		return 0x25;
	else if (gadget_is_s3c_hsotg(gadget))
		return 0x26;
	else if (gadget_is_pch(gadget))
		return 0x27;
	else if (gadget_is_ci13xxx_msm(gadget))
		return 0x28;
	else if (gadget_is_renesas_usbhs(gadget))
		return 0x29;
	else if (gadget_is_s3c_hsudc(gadget))
		return 0x30;
	else if (gadget_is_net2272(gadget))
		return 0x31;
	else if (gadget_is_dwc3(gadget))
		return 0x32;
	else if (gadget_is_aliudc(gadget))
		{
		printk("is ali gadget\n");
		return 0x33;
		}


	return -ENOENT;
}


/**
 * gadget_supports_altsettings - return true if altsettings work
 * @gadget: the gadget in question
 */
static inline bool gadget_supports_altsettings(struct usb_gadget *gadget)
{
	/* PXA 21x/25x/26x has no altsettings at all */
	if (gadget_is_pxa(gadget))
		return false;

	/* PXA 27x and 3xx have *broken* altsetting support */
	if (gadget_is_pxa27x(gadget))
		return false;

	/* Everything else is *presumably* fine ... */
	return true;
}

#endif /* __GADGET_CHIPS_H */
