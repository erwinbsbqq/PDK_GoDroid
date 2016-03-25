#ifndef __USB_DEFINE_NEW_H__
#define __USB_DEFINE_NEW_H__


/****************pipe operation**************/
/* USB endpoint type */
#define USB_ISO				0
#define USB_INT				1
#define USB_CTRL			2
#define USB_BULK			3

/* about USB protocol */
#define USB_DIR_IN		0x80
#define USB_DIR_OUT		0x00

#define usb_sndaddr0pipe()	(USB_CTRL << 30)
#define usb_rcvaddr0pipe()	((USB_CTRL << 30) | USB_DIR_IN)

#define usb_pipein(pipe)	((pipe) & USB_DIR_IN)
#define usb_pipeout(pipe)	(!usb_pipein(pipe))

#define usb_pipedevice(pipe)	(((pipe) >> 8) & 0x7f)
#define usb_pipeendpoint(pipe)	(((pipe) >> 15) & 0xf)

#define usb_pipetype(pipe)	(((pipe) >> 30) & 3)
#define usb_pipeisoc(pipe)	(usb_pipetype((pipe)) == USB_ISO)
#define usb_pipeint(pipe)	(usb_pipetype((pipe)) == USB_INT)
#define usb_pipecontrol(pipe)	(usb_pipetype((pipe)) == USB_CTRL)
#define usb_pipebulk(pipe)	(usb_pipetype((pipe)) == USB_BULK)

#define usb_pipepktsize_get(pipe)	((pipe)&0x7)
#define usb_pipepktsize_set(pipe, size) {pipe = ((pipe)&0xfffffff8)|(size&0x7);}

/* The D0/D1 toggle bits ... USE WITH CAUTION (they're almost hcd-internal) */
#define usb_gettoggle(dev, ep, out) (((dev)->toggle[out] >> (ep)) & 1)
#define usb_dotoggle(dev, ep, out)  ((dev)->toggle[out] ^= (1 << (ep)))
#define usb_settoggle(dev, ep, out, bit) \
		((dev)->toggle[out] = ((dev)->toggle[out] & ~(1 << (ep))) | \
		 ((bit) << (ep)))


#define __create_pipe(dev, endpoint) ((dev->base_addr << 8) | (endpoint << 15))
/* Create various pipes... */
#define usb_sndctrlpipe(dev,endpoint)	\
	((USB_CTRL << 30) | __create_pipe(dev,endpoint))
#define usb_rcvctrlpipe(dev,endpoint)	\
	((USB_CTRL << 30) | __create_pipe(dev,endpoint) | USB_DIR_IN)
#define usb_sndisocpipe(dev,endpoint)	\
	((USB_ISO << 30) | __create_pipe(dev,endpoint))
#define usb_rcvisocpipe(dev,endpoint)	\
	((USB_ISO << 30) | __create_pipe(dev,endpoint) | USB_DIR_IN)
#define usb_sndbulkpipe(dev,endpoint)	\
	((USB_BULK << 30) | __create_pipe(dev,endpoint))
#define usb_rcvbulkpipe(dev,endpoint)	\
	((USB_BULK << 30) | __create_pipe(dev,endpoint) | USB_DIR_IN)
#define usb_sndintpipe(dev,endpoint)	\
	((USB_INT << 30) | __create_pipe(dev,endpoint))
#define usb_rcvintpipe(dev,endpoint)	\
	((USB_INT << 30) | __create_pipe(dev,endpoint) | USB_DIR_IN)

/*
 * Endpoints
 */
#define USB_ENDPOINT_NUMBER_MASK	0x0F	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80
#define USB_ENDPOINT_XFERTYPE_MASK	0x03	/* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL	0
#define USB_ENDPOINT_XFER_ISOC		1
#define USB_ENDPOINT_XFER_BULK		2
#define USB_ENDPOINT_XFER_INT		3

#endif/*__USB_DEFINE_H__*/
