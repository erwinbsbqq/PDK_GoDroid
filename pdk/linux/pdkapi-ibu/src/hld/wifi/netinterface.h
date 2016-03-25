/************************************************************************ 
  * Copyright(c) 2011-2012 Ali Tech., Corporation 
  * All Right Reserved 
  *  
  * File name : netinterface.h
  * Description : Support a interface for app connect to wifi ap.
  *  
  * History
  * ---------------------------
  * 1. 2012-11-21, Port from C2000 
  * 2. 2012-12-12, Stephen.Xiao modified
  * ---------------------------
  ************************************************************************/ 


#ifndef NET_INTERFACE_H_
#define NET_INTERFACE_H_
#define WIRED_DEV "eth0"
#define WIRELESS_DEV "ra0"
#define PPPOE_DEV "ppp0"
#define UDP_PORT 8901
#define MAX_BUF_SIZE 512



#define SHARED_FILENAME "/dev/shm/tmp_network"


//#include "client_hal.h"
typedef enum NET_CONNECT_TYPE_ {
	TYPE_WIRED_DHCP = 0,
	TYPE_WIRED_FIXEDIP,
	TYPE_WIRED_PPPOE,
	TYPE_WIRELESS_DHCP,
	TYPE_WIRELESS_FIXEDIP,
	
}NET_CONNECT_TYPE;

typedef enum WIFI_ENCRYPTION_TYPE_ {
	WIRELESS_NONE = 0,
	WIRELESS_WEP,
	WIRELESS_WPA_WPA2,
}WIFI_ENCRYPTION_TYPE;

typedef enum NET_SERVICE_MODE_ {
 	NET_SERVICE_MODE_AUTO = 0,				//AUTO module
	NET_SERVICE_MODE_IN_SETTING, 			//setting module
} NET_SERVICE_MODE;

typedef struct _wifi_config_{
    WIFI_ENCRYPTION_TYPE encryption;
	int  hide ;
    char  ssid[64];
    char  key[128];
} wifi_config, *p_wifi_config;

typedef struct _ip_config_{
    char ipaddr[16];
    char gatway[16];
    char submask[16];
    char dns1[16];
}ip_config, *p_ip_config;

typedef struct _network_config_{
	NET_SERVICE_MODE work_mode;
	NET_CONNECT_TYPE net_connect_type;
	ip_config wired_ip_config;
	ip_config wireless_ip_config;
	wifi_config wifi_config_info;
	char str_account[32];
	char str_password[32];
	char * net_dev;
} network_config, *p_network_config;

typedef struct _system_config_{
	char *name;
	char *value;
} system_cfg, *p_system_cfg;

typedef struct _net_status_{
	NET_CONNECT_TYPE net_connect_type;
	int dev_status;
	NET_SERVICE_MODE work_mode;
} net_status, *p_net_status;

typedef enum _NTCmdEvent_
{
	CMD_WIFI_CONNECTED,					
	CMD_WIFI_DISCONNECTED,				
	CMD_WIFI_SINGNAL_CHANGED,				
	CMD_LOCAL_CONNECTED,					
	CMD_LOCAL_DISCONNECTED,				
	CMD_PPPOE_CONNECTED,					
	CMD_PPPOE_DISCONNECTED,					
	CMD_CONNECT_FAILED,			
}NTCmdEvent;



typedef void (*networkCmdNotify)(NTCmdEvent ,NET_SERVICE_MODE );

typedef enum NETWORK_DEVICE_STATUS_ {
	NETWORK_DEVICE_UNAVAILABLE  = 0,		//didn't have wifi device pulg into block.
	NETWORK_DEVICE_AVAILABLE,			//already have wifi device pulg into block.
	NETWORK_DEVICE_INACTIVE,				//already have wifi device pulg into block,but the status is disenable.
	NETWORK_DEVICE_ACTIVE,				//already have wifi device pulg into block,and the status is enable.
}NETWORK_DEVICE_STATUS;

typedef struct network_dev_detail_ {
	NETWORK_DEVICE_STATUS status;
    char name[32];							//net device name ,such as "ra0"
}network_dev_detail;

typedef struct ap_detail_ {
	unsigned int encypted;			
	unsigned int ID;				
	unsigned int quality;				
    char essid[128];						
	char mac[32];							
}ap_detail;

typedef struct network_dev_list_ {
	network_dev_detail dev0;				//dev0 , local wire device
	network_dev_detail dev1;				//dev1, USB wireless device
}network_dev_list;	

typedef struct ap_list_ {
	ap_detail ap_info[64];					//max app num from scanning
	unsigned int ap_count;					
}ap_list;	

typedef struct ip_info_ {
	char *ip_address;
	char *mask;
	char *gateway;
	char *dns;
}ip_info;	

typedef struct dialup_info_ {
	char *account;
	char *password;
}dialup_info;	

/*
typedef struct wifi_connect_status_ {
	ap_detail *connect_ap_detail;
	ip_info *connect_ip_info;
}wifi_connect_status;	
*/


/***********************************************************
@brief init the network middle ware, register callback function and set service model. 
@param ntf:callback function name
@param mode: 	NET_SERVICE_MODE_AUTO		
				NET_SERVICE_MODE_IN_SETTING
@return  0:OK 
	     -1:Failed
**********************************************************/
int wifi_start_config_network(networkCmdNotify ntf, NET_SERVICE_MODE mode);


/***********************************************************
@brief get the net device list
@return  0:OK 
	     -1:Failed
**********************************************************/
int wifi_get_network_device_list(network_dev_list *device);


/***********************************************************
@brief get wifi ap list from scaning
@return  0:OK 
	     -1:Failed
**********************************************************/
int wifi_get_ap_list(ap_list *ap);


/***********************************************************
@brief scanning hide ssid, need user input the AP' ssid name
@param hide_ssid:		ssid name
**********************************************************/
int wifi_scan_hide_ssid(char *hide_ssid);


/***********************************************************
@brief Set the wireless connect spec in DHCP model.
@param 	ap_name: 	ap's name (ssid)
		password:	ap's password
		encrption:	ap's encrption model
**********************************************************/
int wifi_set_wireless_dhcp(char *ap_name, char *password, WIFI_ENCRYPTION_TYPE encrption);


/***********************************************************
@brief Set the wireless connect spec in SETTING model
@param 	ap_name: 	ap's name (ssid)
		password:	ap's password
			   ip:	IP spec, such as IP,mask,gateway,dns.		
**********************************************************/
int wifi_set_wireless_fixedip(char *ap_name, char *password, ip_info *ip);


/***********************************************************
@brief     set up the net device
@param   net_dev: the net device name, just like "ra0"
@Return   0 sucess 
		-1 failed
**********************************************************/
int wifi_interface_up(const char * net_dev);


/***********************************************************
@brief:   set down the net device
@param  net_dev: the net device name, just like "ra0"
@Return: 0 sucess 
		-1 failed
**********************************************************/
int wifi_interface_down(const char * net_dev);


/***********************************************************
@brief: check the wifi device status.
@param  net_dev: the net device name, just like "ra0"
@Return: 0 means disenable,1 means enable, -1 get status failed
**********************************************************/
int wifi_check_operstate(const char * net_dev);


/***********************************************************
@brief: check the wifi connect status.
@param  net_dev: the net device name, just like "ra0"
@Return: 0 means wifi device was enable but  link down,
		 1 means wifi device was enable and link up some AP 
		-1 get status failed
**********************************************************/
int wifi_check_carrier(const char * net_dev);


/***********************************************************
@brief:  get the quality of current wifi connected ap.
@param  net_dev: the net device name, just like "ra0"
@return  0:OK 
	     -1:Failed
**********************************************************/
int wifi_get_ap_quality(const char *net_dev);



/***********************************************************
@brief:  disconnect to current wifi ap
@param  net_dev: the net device name, just like "ra0"
@return  0:OK 
	     -1:Failed
**********************************************************/
int wifi_ap_disconnect(const char*net_dev);


/***********************************************************
@brief:  get current wireless adapter's IP address 
@param  net_dev: the net device name, just like "ra0"
@return  IP address,just like s_addr,
		if IP is "192.168.8.5" the return value is "0X0508A8C0"
**********************************************************/
int wifi_network_get_ip_address(const char *net_dev);

#endif
