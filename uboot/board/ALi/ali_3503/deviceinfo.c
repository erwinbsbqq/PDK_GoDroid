#include <common.h>
#include "deviceinfo.h"

static struct device_info *deviceinfo=NULL;

struct device_info * get_deviceinfo()
{
	int ret;
	const char *partName = "deviceinfo";

	if (deviceinfo)
		return deviceinfo;		
	
	deviceinfo = (struct device_info*)malloc(sizeof(struct device_info));

#if defined(CONFIG_NAND_ALI) 
	ret = load_part_sector(partName, (u_char *)deviceinfo, sizeof(struct device_info));
#endif

 #if defined(CONFIG_ALI_MMC)      //emmc
	    ret = load_part_sector_emmc(partName, (u_char *)deviceinfo, sizeof(struct device_info));
#endif

	if (ret < 0)
	{
		printf("get deviceinfo fail.\n");	
		free(deviceinfo);
		return NULL;
	}
	
	return deviceinfo;	
}

int set_mac(void)
{
	struct device_info *deviceinfo = get_deviceinfo();
	u_char *mac = deviceinfo->firmware.mac;
	char macStr[20];
	
	sprintf(macStr,"%02x:%02x:%02x:%02x:%02x:%02x",
		mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	
	setenv("ethaddr", macStr);
	
	return 0;
}

unsigned int get_hdmiinfo(unsigned char **hdmiinfo)
{
	struct device_info *deviceinfo = get_deviceinfo();

	if (deviceinfo == NULL)
	{
		printf("get hdmiinfo fail.\n");		
		*hdmiinfo = NULL;
		return 0;
	}

	*hdmiinfo = &deviceinfo->hdmi;

	return sizeof(HDMI_INFO);
}

unsigned int get_mac(unsigned char **mac)
{
	struct device_info *deviceinfo = get_deviceinfo();

	if (deviceinfo == NULL)
	{
		printf("get mac fail.\n");			
		*mac = NULL;
		return 0;
	}

	*mac = &deviceinfo->firmware.mac;
	return STB_FIRMINFO_MAC_LEN*4;
}

