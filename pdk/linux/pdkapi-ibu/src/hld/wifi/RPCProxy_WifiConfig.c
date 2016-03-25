/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               RPCProxy_WifiConfig.c
 *  @brief              Sent message to wifi_daemon to set WIFI config.
 *
 *  @version            1.0
 *  @date               06/29/2013 03:33:37 PM
 *  @revision           none
 *
 *  @author             Stephen Xiao <stephen.xiao@alitech.com>
 */


#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>

#include "netinterface.h"
#include "alirpc_binder.h"
#include "RPC_WifiConfig.h"


static struct binder_state *bs;
static void *alirpc_handle = NULL;

#define ALIRPC_CALL_CHECK \
	do{     \
		if( alirpc_handle == NULL) \
		{ \
			RPC_WIFI_LOG_ERROR("RPC not init yet!"); \
			return -1; \ 
		} \
	}while(0)



int wifi_start_config_network(networkCmdNotify ntf, NET_SERVICE_MODE mode)
{
	ENTRY;
	//Step 1: Set the RPC srevice module name.
	//g_p_aliIntegrationLayer_servic_name = strdup(str_aliIntegrationLayer_module_name);

	/* Step 1: Init debug print level*/
	/*
	if (dbg_log_init("client", LOG_ERR|LOG_WARN|LOG_INFO|LOG_DEBUG0) != 0) {
	//if (dbg_log_init("client", LOG_ERR|LOG_WARN) != 0) {
		printf("dbg_log_init failed ");
		return (-1);
	}
	*/
	
	//Step 2: Open the binder.	
  	bs = binder_open(128*1024);
    	RPC_WIFI_LOG_DUMMY("client open bs = %x", bs);
		
	//Try look up the service
	alirpc_handle = alirpc_binder_try_get_rpc_handle(bs, WIFI_IPC_SERVICE_NAME);
	if(0 == alirpc_handle )
	{
		RPC_WIFI_LOG_DUMMY("Failed to found the service:%s.",WIFI_IPC_SERVICE_NAME);
	}
	
	//Step 3: register the callback function to dvb layer.
	//ALi_Stb_Register_callback_func_to_send_event_to_ail(AIL_Proxy_DVB_func_remote_call_AIL);

	//TODO. Other init code for RPC Client.

	RPC_WIFI_LOG_DUMMY("IPC init Done!");

	return 0;
}

int wifi_get_network_device_list(network_dev_list *device)
{
	ENTRY;
	ALIRPC_CALL_CHECK;
	DESC_PARG_OUTPUT(0, sizeof(network_dev_list));
	ALIRPC_CALL_P(1,SERVICE_WIFI_GET_NETWORK_DEVICE_LIST);
}

int wifi_interface_up(const char * net_dev)
{
	ENTRY;
	ALIRPC_CALL_CHECK;
    	DESC_PARG(0, sizeof(WIRELESS_DEV));
	ALIRPC_CALL_P(1,SERVICE_WIFI_INTERFACE_UP);
}

int wifi_interface_down(const char * net_dev)
{
	ENTRY;
	ALIRPC_CALL_CHECK;
    	DESC_PARG(0, sizeof(WIRELESS_DEV));
	ALIRPC_CALL_P(1,SERVICE_WIFI_INTERFACE_DOWN);
}


int wifi_get_ap_list(ap_list *ap)
{
	ENTRY;
	ALIRPC_CALL_CHECK;
	DESC_PARG_OUTPUT(0, sizeof(ap_list));
	ALIRPC_CALL_P(1,SERVICE_WIFI_GET_AP_LIST);
}


int wifi_scan_hide_ssid(char *hide_ssid)
{
	ENTRY;
	ALIRPC_CALL_CHECK;
	RPC_WIFI_LOG_ERROR("Not Support Yet!");
}


int wifi_check_operstate(const char * net_dev)
{
	ENTRY;
	ALIRPC_CALL_CHECK;
    	DESC_PARG(0, sizeof(WIRELESS_DEV));
	ALIRPC_CALL_P(1,SERVICE_WIFI_CHECK_OPERSTATE);
}

int wifi_check_carrier(const char * net_dev)
{
	ENTRY;
	ALIRPC_CALL_CHECK;
    	DESC_PARG(0, sizeof(WIRELESS_DEV));
	ALIRPC_CALL_P(1,SERVICE_WIFI_CHECK_CARRIER);
}


int wifi_set_wireless_dhcp(char *ap_name, char *password,WIFI_ENCRYPTION_TYPE encrption)
{
	ENTRY;
	ALIRPC_CALL_CHECK;
    	DESC_PARG2(0, strlen(ap_name)+1, 1, strlen(password)+1);
	ALIRPC_CALL_P(3,SERVICE_WIFI_SET_WIRELESS_DHCP);
}

int wifi_set_wireless_fixedip(char *ap_name, char *password, ip_info *ip)
{
	ENTRY;
	ALIRPC_CALL_CHECK;
	RPC_WIFI_LOG_ERROR("Not Support Yet!");
}


int wifi_get_ap_quality(const char *net_dev)
{
	ENTRY;
	ALIRPC_CALL_CHECK;
    	DESC_PARG(0, sizeof(WIRELESS_DEV));
	ALIRPC_CALL_P(1,SERVICE_WIFI_GET_AP_QUALITY);
}

int wifi_ap_disconnect(const char* net_dev)
{
	ENTRY;
	ALIRPC_CALL_CHECK;
    	DESC_PARG(0, sizeof(WIRELESS_DEV));
	ALIRPC_CALL_P(1,SERVICE_WIFI_AP_DISCONNECT);
}

int wifi_network_get_ip_address(const char *net_dev)
{
	ENTRY;
	ALIRPC_CALL_CHECK;
    	DESC_PARG(0, sizeof(WIRELESS_DEV));
	ALIRPC_CALL_P(1,SERVICE_WIFI_NETWORK_GET_IP_ADDRESS);
}
