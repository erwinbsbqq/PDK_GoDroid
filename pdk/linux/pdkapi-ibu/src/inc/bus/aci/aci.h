
#ifndef __HLD_ACI_DEV_H__
#define __HLD_ACI_DEV_H__

#include <types.h>
#include <hld/hld_dev.h>

struct aci_device
{
	#if(SMART_ANT_SUPPORT == SMART_ANT_BY_GPIO)
	struct hld_device   *next;		/* Next device structure */
	UINT32		type;						/* Interface hardware type */
	INT8		name[HLD_MAX_NAME_SIZE];	/* Device name */

	UINT16		flags;				/* Interface flags, status and ability */

	/* Hardware privative structure */
	void		*priv;				/* pointer to private data */

	int	gpio_aci;
	int	gpio_ant_detect;

	INT32	(*init)(int aci, int ant_detect);

	INT32	(*detect_antenna)(struct aci_device* dev);

	INT32	(*transmit_series_data)(struct aci_device* dev, UINT8 bits_num,UINT32 data);
	INT32	(*set_onoff)(struct aci_device* dev, BOOL onoff);
	INT32	(*do_ioctl)(struct aci_device *dev, INT32 cmd, UINT32 param);
	#elif(SMART_ANT_SUPPORT == SMART_ANT_BY_ALI)
	struct hld_device   *next;		/* Next device structure */
	UINT32		type;						/* Interface hardware type */
	INT8		name[HLD_MAX_NAME_SIZE];	/* Device name */

	UINT16		flags;				/* Interface flags, status and ability */
	UINT32		base_addr;
	/* Hardware privative structure */
	void		*priv;				/* pointer to private data */

	int	gpio_aci;
	int	gpio_ant_detect;

	INT32	(*init)(int aci, int ant_detect);

	INT32	(*detect_antenna)(struct aci_device* dev);

	INT32	(*transmit_series_data)(struct aci_device* dev, UINT8 bits_num,UINT32 data);
	INT32	(*set_onoff)(struct aci_device* dev, BOOL onoff);
	INT32	(*do_ioctl)(struct aci_device *dev, INT32 cmd, UINT32 param);

	
	#endif
};
INT32 aci_set_onoff(struct aci_device* dev, BOOL onoff);
INT32 aci_detect_antenna(struct aci_device* dev);
INT32 aci_transmit_series_data(struct aci_device* dev, UINT8 bits_num,UINT32 data);
INT32 aci_init(int aci, int ant_detect);

#endif

