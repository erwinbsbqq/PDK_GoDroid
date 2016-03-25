#ifndef DEVICEINFO_H
#define DEVICEINFO_H
 
typedef struct
{
	u_char hdcp[286];
	u_short hdcp_disable;
}HDMI_INFO;	/* 288 bytes */

#define STB_FIRMINFO_SN_LEN 			64
#define STB_FIRMINFO_MAC_LEN 			8
typedef struct
{
	u_char sn[STB_FIRMINFO_SN_LEN];
	u_char mac[STB_FIRMINFO_MAC_LEN];
	u_char mac2[STB_FIRMINFO_MAC_LEN];
	u_char mac3[STB_FIRMINFO_MAC_LEN];
	u_char mac4[STB_FIRMINFO_MAC_LEN];
	u_int oui;
	u_char hw_ver[128];
	u_char rsv[1820];	
}FIRMWARE_INFO;	 /* 2048 bytes */

typedef struct
{
	u_char bootkey_desc[16];    // "bootkey"
	
	u_int ir_enable;
	u_int ir_timeout;

	u_int panel_enable;
	u_int panel_count;
	u_int panel_timeout[2];
	u_int panel_keys[2];	

	u_int gpio_enable;
	u_int gpio_polar;
	u_int gpio_position;    
}BOOTKEY_CONFIG; /* 0x60 bytes */

typedef struct
{
	u_char bootconfig_desc[16];
	u_char bootserailno_enable;
	u_char bootblverison_enable;
	u_char bootmode_enable;
	u_char boothwversion_enable;
	u_int status;	
	u_char rsv[56];	
}BOOTCONFIG;

struct device_info
{
	/* 0 bytes */
	u_char magic[16];	// deviceinfo
	/* 16 bytes */
	HDMI_INFO hdmi;	
	/* 304 bytes */
	FIRMWARE_INFO firmware;
	BOOTKEY_CONFIG bootkey;    
    
	BOOTCONFIG bootconfig;    
};

unsigned int get_hdmiinfo(unsigned char **hdmiinfo);
int get_sn(unsigned char *sn);
unsigned int get_mac(unsigned char **mac);
unsigned int get_bootkey(unsigned char **bootkey);
unsigned int get_bootconfig(unsigned char **bootconfig);

#endif // DEVICEINFO_H