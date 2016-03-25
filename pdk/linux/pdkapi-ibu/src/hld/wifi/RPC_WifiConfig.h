#ifndef RPC_WIFICONFIG_H_
#define RPC_WIFICONFIG_H_

#include "netinterface.h"

#define WIFI_IPC_SERVICE_NAME "rpc_service_wifi_config"	

//#define ENTRY fprintf(stdout,"ENTRY| %d %s %s \n", __LINE__, __FILE__,  __PRETTY_FUNCTION__)

//#define  RPC_WIFI_DEBUG_ENABLED

// Basic  print mode 
#define _RPC_PROXY_LOG(level, fmt, ...)										\
	do {																\
		fprintf(stderr, "RPC Wifi Config " level ": " fmt " (%s:%d::%s)\n", ##__VA_ARGS__, __FILE__, __LINE__, __FUNCTION__); \
	} while (0)

#ifdef RPC_WIFI_DEBUG_ENABLED

#define ENTRY fprintf(stdout,"ENTRY| %d %s %s \n", __LINE__, __FILE__,  __PRETTY_FUNCTION__)
#define RPC_WIFI_LOG_DUMMY(fmt, ...) _RPC_PROXY_LOG(" debug   ", fmt, ##__VA_ARGS__)
#define RPC_WIFI_LOG_WARNING(fmt, ...) _RPC_PROXY_LOG(" warning   ", fmt, ##__VA_ARGS__)
#define RPC_WIFI_LOG_ERROR(fmt, ...) _RPC_PROXY_LOG(" error   ", fmt, ##__VA_ARGS__)

#else

#define ENTRY 
#define RPC_WIFI_LOG_DUMMY(fmt, ...)
#define RPC_WIFI_LOG_WARNING(fmt, ...)
#define RPC_WIFI_LOG_ERROR(fmt, ...) _RPC_PROXY_LOG(" error   ", fmt, ##__VA_ARGS__)

#endif

typedef enum _SERVICE_FUNC_WIFI_CONFIG{
	SERVICE_NONE_FUNCTION = 0,
	SERVICE_WIFI_GET_NETWORK_DEVICE_LIST,
	SERVICE_WIFI_INTERFACE_UP,
	SERVICE_WIFI_INTERFACE_DOWN,
	SERVICE_WIFI_GET_AP_LIST,
	SERVICE_WIFI_CHECK_OPERSTATE,
	SERVICE_WIFI_CHECK_CARRIER,
	SERVICE_WIFI_SET_WIRELESS_DHCP,
	SERVICE_WIFI_GET_AP_QUALITY,
	SERVICE_WIFI_AP_DISCONNECT,
	SERVICE_WIFI_NETWORK_GET_IP_ADDRESS,
	SERVICE_MAX_INDEX
}SERVICE_FUNC_WIFI_CONFIG;


int service_wifi_interface_up(const char * net_dev);
int service_wifi_interface_down(const char * net_dev);
int service_wifi_get_network_device_list(network_dev_list * device);
int service_check_wpa_supplicant();
int service_wifi_get_ap_list(ap_list *ap);
int service_wifi_check_operstate(const char * net_dev);
int service_wifi_check_carrier(const char * net_dev);
int service_wifi_set_wireless_dhcp(char *ap_name, char *password, WIFI_ENCRYPTION_TYPE encrption);
int service_wifi_get_ap_quality(const char * net_dev);
int service_wifi_ap_disconnect(const char * net_dev);
int service_wifi_network_get_ip_address(const char * net_dev);


#endif
