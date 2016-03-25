#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
//#include "client_hal.h"
#include "netinterface.h"


void g_networkCmdNotify(NTCmdEvent env ,NET_SERVICE_MODE mode )
{
	printf("===================== NTCmdEvent = %d , NET_SERVICE_MODE = %d \n", env, mode );
	//	start_config_network(g_networkCmdNotify, NET_SERVICE_MODE_AUTO);
	exit(0);
}

int main(int argc, char** argv) 
{
	if(argc < 2)
	{
		printf("wrong params return \n");
		return 0;
	}
	int type = atoi(argv[1]);

	wifi_start_config_network(g_networkCmdNotify, NET_SERVICE_MODE_IN_SETTING);
	ip_info m_ip_info;
	m_ip_info.ip_address = "192.168.9.166";
	m_ip_info.mask= "255.255.255.0";
	m_ip_info.gateway= "192.168.9.254";
	m_ip_info.dns= "192.168.9.11";
	ap_list m_ap_list;
	memset(&m_ap_list, 0, sizeof(m_ap_list));
	int ret = -1;	
	switch(type)
	{
		case 1:
			{

				/*
				   network_dev_list* p_network_dev_list = (network_dev_list*)malloc(sizeof(network_dev_list));
				//memset(p_network_dev_list, 0, sizeof(network_dev_list));

				printf("device = %p\n",&p_network_dev_list);
				wifi_get_network_device_list(&p_network_dev_list);
				printf("%s TEST.c is NETWORK_DEVICE_AVAILABLE \n", p_network_dev_list->dev0.name);
				printf("%s TEST.c is NETWORK_DEVICE_AVAILABLE \n", p_network_dev_list->dev1.name);
				 */
				network_dev_list m_network_dev_list;
				memset(&m_network_dev_list, 0, sizeof(m_network_dev_list));

				printf("device = %p\n",&m_network_dev_list);
				wifi_get_network_device_list(&m_network_dev_list);
				if(m_network_dev_list.dev0.status)
				{
					printf("%s is NETWORK_DEVICE_AVAILABLE \n", m_network_dev_list.dev0.name);
				}
				if(m_network_dev_list.dev1.status)
				{
					printf("%s is NETWORK_DEVICE_AVAILABLE \n", m_network_dev_list.dev1.name);
				}

			}
			break;
		case 2:
			{		
				ret = wifi_interface_up("ra0");
				printf("Stephen wifi test wifi_interface_up ret = %d\n",ret);
				break;
			}
		case 3:
			{		
				ret = wifi_interface_down("ra0");
				printf("Stephen wifi test wifi_interface_down ret = %d\n",ret);
				break;
			}
		case 4:
			{		
				int i = 0;
				wifi_get_ap_list(&m_ap_list);
				for(i=0; i<m_ap_list.ap_count; i++)
				{
					printf("%d  %s %s %d \n", m_ap_list.ap_info[i].encypted, m_ap_list.ap_info[i].essid,
							m_ap_list.ap_info[i].mac, m_ap_list.ap_info[i].quality);
				}
				break;
			}
		case 5:
			{		
				ret = wifi_check_operstate("ra0");
				printf("Stephen wifi test wifi_check_operstate ret = %d\n",ret);
				break;
			}
		case 6:
			{		
				ret = wifi_check_carrier("ra0");
				printf("Stephen wifi test wifi_check_carrier ret = %d\n",ret);
				break;
			}
		case 7:
			{		
				ret = wifi_set_wireless_dhcp ("eric_miiicasa", "a1234567",WIRELESS_NONE);
				printf("Stephen wifi test wifi_set_wireless_dhcp ret = %d\n",ret);
				break;
			}
		case 8:
			{		
				ret = wifi_get_ap_quality ("ra0");
				printf("Stephen wifi test wifi_get_ap_quality ret = %d\n",ret);
				break;
			}
		case 9:
			{		
				ret = wifi_ap_disconnect ("ra0");
				printf("Stephen wifi test wifi_ap_disconnect ret = %d\n",ret);
				break;
			}
		case 10:
			{		
				ret = wifi_network_get_ip_address ("ra0");
				printf("Stephen wifi test wifi_network_get_ip_address ret = %d\n",ret);
				break;
			}

		default:
			break;
	}
	return (0);
}
