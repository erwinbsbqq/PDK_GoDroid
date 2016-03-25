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
	memset(deviceinfo, 0, sizeof(struct device_info));

	ret = load_part_sector(partName, (u_char *)deviceinfo, sizeof(struct device_info));
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

int get_sn(unsigned char *sn)
{
	if (sn == NULL)
	{
		printf("sn is NULL\n");
		return -1;
	}
	struct device_info *deviceinfo = get_deviceinfo();

	if (deviceinfo == NULL)
	{
		return -1;
	}

	memcpy(sn, deviceinfo->firmware.sn, STB_FIRMINFO_SN_LEN);

	//printf("get_sn: %s\n",sn);

	return STB_FIRMINFO_SN_LEN;
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

int get_hwver(unsigned char *hwver)
{
	if (hwver == NULL)
	{
		printf("hwver is NULL\n");
		return -1;
	}
	struct device_info *deviceinfo = get_deviceinfo();

	if (deviceinfo == NULL)
	{
		return -1;
	}

	memcpy(hwver, deviceinfo->firmware.hw_ver, sizeof(deviceinfo->firmware.hw_ver));

	return sizeof(deviceinfo->firmware.hw_ver);
}

unsigned int get_bootkey(unsigned char **bootkey)
{
	struct device_info *deviceinfo = get_deviceinfo();

	if (deviceinfo == NULL)
	{
		*bootkey = NULL;
		return 0;
	}

	*bootkey = &deviceinfo->bootkey;
	return sizeof(deviceinfo->bootkey);
}

unsigned int get_bootconfig(unsigned char **bootconfig)
{
	struct device_info *deviceinfo = get_deviceinfo();

	if (deviceinfo == NULL)
	{
		*bootconfig = NULL;
		return 0;
	}

	*bootconfig = &deviceinfo->bootconfig;
	return sizeof(deviceinfo->bootconfig);
}

