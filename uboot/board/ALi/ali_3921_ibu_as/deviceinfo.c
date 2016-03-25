#include <common.h>
#include "deviceinfo.h"

#include <boot_common.h>
#include <ali/sys_parameters.h>

#define DEVICEINFO_HEADER 12
#define SIG_LEN   0x100

extern unsigned char g_uk_pos;
static struct device_info *deviceinfo=NULL;

struct device_info * get_deviceinfo()
{
	int ret;
	const char *partName = "deviceinfo";
    unsigned int rsa_data_len = 0;
	struct device_info *deviceinfo_tmp_buff =NULL;
       unsigned char *p_temp = NULL;

	if (deviceinfo)
		return deviceinfo;		
	// bl_tool will pad data to  align 0x20, so we need to align 0x20
	rsa_data_len = ((sizeof(struct device_info)+DEVICEINFO_HEADER+0x20-1) & ~(0x20-1)) + SIG_LEN;
	deviceinfo = (struct device_info*)malloc(sizeof(struct device_info));
	deviceinfo_tmp_buff = (struct device_info*)malloc(rsa_data_len);
    p_temp = deviceinfo_tmp_buff;

    printf("%s: rsa_data_len=0x%x\n", __FUNCTION__,rsa_data_len);
#if defined(CONFIG_NAND_ALI) 
	ret = load_part_sector(partName, (u_char *)deviceinfo_tmp_buff, rsa_data_len);
#endif

#if defined(CONFIG_ALI_MMC)      //emmc
    ret = load_part_sector_emmc(partName, (u_char *)deviceinfo_tmp_buff, rsa_data_len);
#endif
/*
	aes_cbc_decrypt_ram_chunk(g_uk_pos,deviceinfo, deviceinfo_tmp_buff, sizeof(struct device_info));

	if( version_check(CHUNKID_SEECODE,(UINT32)deviceinfo_tmp_buff,sizeof(struct device_info)) != 0){
		printf("deviceinfo  version check failed\n");
		return NULL;
	} 
*/
#if 1
    if(test_rsa_ram((UINT32)deviceinfo_tmp_buff,rsa_data_len) != 0)
	{
		printf("deviceinfo  rsa verify error\n");
		return NULL;
	}
#endif
	memcpy((void *)deviceinfo, (void *)(p_temp+DEVICEINFO_HEADER), sizeof(struct device_info));
	if (ret < 0)
	{
		printf("get deviceinfo fail.\n");	
		free(deviceinfo);
		return NULL;
	}
	free(deviceinfo_tmp_buff);
	return deviceinfo;	
}

int set_mac(void)
{
	struct device_info *deviceinfo = NULL;
	u_char *mac = NULL;
	char macStr[20];

    deviceinfo = get_deviceinfo();
    if(deviceinfo == NULL)
    {
        printf("%s: deviceinfo is NULL\n", __FUNCTION__);
        return -1;
    }
    mac = deviceinfo->firmware.mac;
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

