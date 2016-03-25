#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
//#include "client_hal.h"
//#include "netservice.h"
//#include "../../../app/hybrid/wifi_deamon/netservice.h"
#include "netinterface.h"
#include "qlog.h"

int wifi_start_config_network(networkCmdNotify ntf, NET_SERVICE_MODE mode)
{
	return init_network_service( ntf, mode);
}

int wifi_get_network_device_list(network_dev_list *device)
{
	if (NULL == device) 
		return -1;

	check_all_dev_extra(device);
	return 0;
}


int wifi_get_ap_list(ap_list *ap)
{
	if (NULL == ap) 
		return -1;

	scan_ap_list_extra(ap);
	return 0;
}

extern int dev_connected_extra(char * net_dev, int check_ip);
int get_dev_is_connected(char *device_name, int check_ip)
{
	return dev_connected_extra(device_name, check_ip);
}

void wifi_scan_hide_ssid(char *hide_ssid)
{
	add_hide_ssid(hide_ssid);
}

void set_wired_dhcp(void)
{
	if(select_dev(WIRELESS_DEV) < 0)
	{
		printf("please call wifi_start_config_network first \n");
		return ;
	}
	select_connect_type(TYPE_WIRED_DHCP);
	send_package();
}

void set_wired_fixedip(ip_info *ip)
{
	if(select_dev(WIRELESS_DEV) < 0)
	{
		printf("please call wifi_start_config_network first \n");
		return ;
	}
	select_connect_type(TYPE_WIRED_FIXEDIP);
	set_ip_infos(WIRED_DEV, ip->ip_address, ip->mask, ip->gateway, ip->dns);
	send_package();
}


void set_wired_dialup(dialup_info *param)
{
	if(select_dev(WIRELESS_DEV) < 0)
	{
		printf("please call wifi_start_config_network first \n");
		return ;
	}
	select_connect_type(TYPE_WIRED_PPPOE);
	set_pppoe(param->account, param->password);
	send_package();
}

/***********************************************************
  @brief Set the wireless connect spec in DHCP model.
  @param 	ap_name: 	ap's name (ssid)
password:	ap's password
encrption:	ap's encrption model
 **********************************************************/
void wifi_set_wireless_dhcp(char *ap_name, char *password,WIFI_ENCRYPTION_TYPE encrption)
{
	g_network_config->work_mode = m_network_config.work_mode;
	gs_net_status->work_mode =  m_network_config.work_mode;		
	if(NET_SERVICE_MODE_AUTO == m_network_config.work_mode)
	{
		dbg_log_print(LOG_ERR, "process_UDP_msg: change to NET_SERVICE_MODE_AUTO");
		return 0;
	}
	g_network_config->net_connect_type = m_network_config.net_connect_type;
	gs_net_status->net_connect_type = g_network_config->net_connect_type;

	if(TYPE_WIRED_FIXEDIP == m_network_config.net_connect_type)
	{
		g_network_config->net_dev = WIRED_DEV;
		memcpy(&g_network_config->wired_ip_config, &m_network_config.wired_ip_config, sizeof(ip_config));
		dbg_log_print(LOG_INFO, "process_UDP_msg: TYPE_WIRED_FIXEDIP");
	}
	else if(TYPE_WIRED_DHCP == m_network_config.net_connect_type)
	{
		g_network_config->net_dev = WIRED_DEV;
		dbg_log_print(LOG_INFO, "process_UDP_msg: TYPE_WIRED_DHCP");

	}
	else if(TYPE_WIRED_PPPOE== m_network_config.net_connect_type)
	{
		memcpy(g_network_config->str_account, m_network_config.str_account, 32);
		memcpy(g_network_config->str_password, m_network_config.str_password, 32);
		g_network_config->net_dev = PPPOE_DEV;
		dbg_log_print(LOG_INFO, "process_UDP_msg: TYPE_WIRED_PPPOE");
	}
	else if(TYPE_WIRELESS_DHCP== m_network_config.net_connect_type)
	{
		g_network_config->net_dev = WIRELESS_DEV;
		memcpy(&g_network_config->wifi_config_info , &m_network_config.wifi_config_info, sizeof(wifi_config));
		dbg_log_print(LOG_INFO, "process_UDP_msg: TYPE_WIRELESS_DHCP");
	}
	else if(TYPE_WIRELESS_FIXEDIP== m_network_config.net_connect_type)
	{
		g_network_config->net_dev = WIRELESS_DEV;
		memcpy(&g_network_config->wifi_config_info , &m_network_config.wifi_config_info, sizeof(wifi_config));
		memcpy(&g_network_config->wireless_ip_config, &m_network_config.wireless_ip_config, sizeof(ip_config));
		dbg_log_print(LOG_INFO, "process_UDP_msg: TYPE_WIRELESS_FIXEDIP");
	}

	sem_post(&g_sem);

	/*
	   if(select_dev(WIRELESS_DEV) < 0)
	   {
	   printf("please call wifi_start_config_network first \n");
	   return ;
	   }
	   select_connect_type(TYPE_WIRELESS_DHCP);
	   set_wifi_ssid(ap_name, password,encrption);
	   send_package();
	 */
}


void wifi_set_wireless_fixedip(char *ap_name, char *password, ip_info *ip)
{
	if(select_dev(WIRELESS_DEV) < 0)
	{
		printf("please call wifi_start_config_network first \n");
		return ;
	}
	select_connect_type(TYPE_WIRELESS_FIXEDIP);
	set_ip_infos(WIRELESS_DEV, ip->ip_address, ip->mask, ip->gateway, ip->dns);
	set_wifi_ssid(ap_name, password,0);
	send_package();
}

