#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
//#include "client_hal.h"
#include "netinterface.h"
#include "qlog.h"
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
	
	switch(type)
	{
		case 1:
			{
				network_dev_list m_network_dev_list;
				wifi_get_network_device_list(&m_network_dev_list);
				if(m_network_dev_list.dev0.status)
				{
					printf("%s is 1NETWORK_DEVICE_AVAILABLE \n", m_network_dev_list.dev0.name);
				}
				if(m_network_dev_list.dev1.status)
				{
					printf("%s is NETWORK_DEVICE_AVAILABLE \n", m_network_dev_list.dev1.name);
				}
			}
			break;
		case 2:
			{
				int i = 0;
				wifi_get_ap_list(&m_ap_list);
				for(i=0; i<m_ap_list.ap_count; i++)
				{
					printf("%d  %s %s %d \n", m_ap_list.ap_info[i].encypted, m_ap_list.ap_info[i].essid,
						m_ap_list.ap_info[i].mac, m_ap_list.ap_info[i].quality);
				}
			}
			break;
		case 3:
			printf(" %s %d \n", argv[2], get_dev_is_connected(argv[2], 1) );
			break;
		case 4: //add hide ssid
			{
				wifi_scan_hide_ssid(argv[2]);
			}
			break;
		case 5: // set wired_dhcp
			{
				set_wired_dhcp();
			}
			break;
		case 6:  // set wried_fixeip
			{
				set_wired_fixedip(&m_ip_info);
			}
			break;
		case 7:  // set wried_fixeip
			{
				dialup_info m_dialup_info;
				m_dialup_info.account = "zhsb12345";
				m_dialup_info.password = "12345678";
				set_wired_dialup(&m_dialup_info);
			}
			break;
	
		case 8:  //set wireless dhcp
			{
				wifi_set_wireless_dhcp("eric_miiicasa", "a1234567",0);
			}
			break;
	
		case 9://set wireless fixedip
			{
				wifi_set_wireless_fixedip("eric_miiicasa", "a1234567", 	&m_ip_info);	
			}
			break;
		case 10://print SSID connection spec
			{
				//network_config* m_network_config = get_current_network_config();
				//printf("wifi's SSID = %s\n",m_network_config->wifi_config_info.ssid);
			}
			break;
		case 11:
			{
				
				
				printf("wifi's SSID");
				dbg_log_print(LOG_INFO, "run_cmd");
				//get_wifi_status("ra0");
			}
			break;
		case 12:
			{
				printf("1wifi AP's quality");
				int i = 0 ;
				wifi_get_ap_quality("ra0");
				//get_wifi_status("ra0");
			}
			break;
		case 13:
			{
				printf("wifi_ap_disconnect\n");
				//int i = 0 ;
				wifi_ap_disconnect("ra0");
				//get_wifi_status("ra0");
			}
			break;
		case 14:
			{
				printf("wifi_network_get_ip_address");
				int i = 0 ;
				wifi_network_get_ip_address("ra0");
				//get_wifi_status("ra0");
			}
			break;			
		default:
			break;
	}
	if(type > 4)
		sleep(10);
	wifi_start_config_network(g_networkCmdNotify, NET_SERVICE_MODE_AUTO);

	if(type > 4)
		sleep(5);

    return (0);
}
