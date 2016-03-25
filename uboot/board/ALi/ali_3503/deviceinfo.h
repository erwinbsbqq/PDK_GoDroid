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


struct device_info
{
	/* 0 bytes */
	u_char magic[16];	// deviceinfo
	/* 16 bytes */
	HDMI_INFO hdmi;	
	/* 304 bytes */
	FIRMWARE_INFO firmware;
};

#endif // DEVICEINFO_H