#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
//#include <net/if.h>
//#include <net/route.h>
//#include "netservice.h"
//#include "../../../app/hybrid/wifi_deamon/netservice.h"
#include "nmpnetapi.h"
#include "qlog.h"
#include "linux/wireless.h"
//#include "client_hal.h"
static int  wireless_device_status=0;



/**
 * We need filter the AP which SSID is same as some APs' in list. Such as CMCC.
 * For Fix BUG 4274
 * Added By Stephen
 */

static int SSID_areadly_in_list(ap_list *ap , ap_detail m_ap_info)
{
	int i = 0;
	for(i; i < ap->ap_count; i++)
	{	
		if(0 == strcmp(ap->ap_info[i].essid, m_ap_info.essid))
		{
			return 1;
		}
	}
	return 0;
}

int check_all_dev(char * device_list)
{
	if(!access("/sys/class/net/eth0/", F_OK))
	{
		strcpy(device_list,"eth0 ");
	}

	if(!access("/sys/class/net/ra0/", F_OK))
	{
		strcpy(device_list + strlen(device_list), "ra0");
	}

	return strlen(device_list);
}

static void check_all_dev_extra(network_dev_list *device)
{
	memset(device, 0, sizeof(network_dev_list));

	if(!access("/sys/class/net/eth0/", F_OK))
	{
		strcpy(device->dev0.name, "eth0");
		device->dev0.status = NETWORK_DEVICE_AVAILABLE;
	}

	if(!access("/sys/class/net/ra0/", F_OK))
	{
		strcpy(device->dev1.name, "ra0");
		device->dev1.status = NETWORK_DEVICE_AVAILABLE;
	}
}

int cmd_run(char * cmd, char* results, int len)
{
	FILE *fp;
	char buf[512] = {0};
	int totolen = 0;

	dbg_log_print(LOG_INFO, "run_cmd: [%s]", cmd);
	if((fp = popen(cmd, "r")) == NULL) {
		dbg_log_print(LOG_ERR, "Fail to popen %s", cmd);
		return -1;
	}   
	while(fgets(buf, 512, fp) != NULL) {
		strcpy(results + totolen, buf);
		totolen += strlen(buf);
		memset(buf, 0, sizeof(buf));
		if(totolen >= len)
			break;
	}   
	pclose(fp);

	return 0;
}


/************************************************* 
Function: get_ipaddr
Description: get ip addr 

Input:  net_dev  ipaddr
Output:  
Return: 0 sucess -1 failed
Others:
 *************************************************/
int get_ipaddr(const char *net_dev,  char* ipaddr)
{
	struct ifreq ifr;
	int fd = 0;
	int ret = -1;
	struct sockaddr_in *pAddr;

	if((NULL == net_dev) || (NULL == ipaddr))
	{
		dbg_log_print(LOG_ERR, "illegal call function SetGeneralIP!");
		return -1;
	}

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
	{
		dbg_log_print(LOG_ERR,"open socket failed");
		return -1;
	}

	memset(&ifr,0,sizeof(ifr));
	strcpy(ifr.ifr_name, net_dev);

	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
	{
		dbg_log_print(LOG_ERR,"SIOCGIFADDR socket failed");
		close(fd);
		return -1;
	}

	pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);

	strcpy(ipaddr, inet_ntoa(pAddr->sin_addr));

	if(strlen(ipaddr) ==0)
	{
		dbg_log_print(LOG_DEBUG0, "ipaddr = [%s] len = 0",ipaddr);
		ret = 0;
	}
	else
	{	
		ret = 1;
		dbg_log_print(LOG_DEBUG0, "ipaddr = [%s]",ipaddr);
	}
	close(fd);

	return ret;
}

void check_wpa_supplicant(int start)
{
	char buf[1024]={0};
	cmd_run("pidof wpa_supplicant", buf, 1024);
	if(start)
	{    
		if(0 == strlen(buf))
		{    
			dbg_log_print(LOG_INFO,  "start wpa_supplicant.....! ");
			system("wpa_supplicant -Dwext -ira0 -c/etc/wpa_supplicant.conf -B ");
			sleep(1);
		}    
	}    
	else 
	{    
		if(0 !=  strlen(buf))
		{    
			dbg_log_print(LOG_INFO,  "wpa_supplicant is running , kill it .");
			system("killall wpa_supplicant");
			sleep(1);		
			wifi_interface_up(WIRELESS_DEV);
		}    
	}    
}


static int scan_ap_list_extra(ap_list *ap)
{
	char results[512] = {0};
	char cmd[32] = {0};
	int totolen = 0;
	FILE *fp;
	char *p; 
	int i = 0;

	ap_detail m_ap_info;
	memset(&m_ap_info, 0, sizeof(m_ap_info));
	check_wpa_supplicant(1);

	system("wpa_cli -i ra0 scan ");
	sleep(1);

	sprintf(cmd, "wpa_cli -i ra0 scan_results");

	dbg_log_print(LOG_INFO, "run_cmd: [%s]", cmd);
	if((fp = popen(cmd, "r")) == NULL) 
	{
		dbg_log_print(LOG_ERR, "Fail to popen %s", cmd);
		return -1;
	} 

	while(fgets(results, 512, fp) != NULL) 
	{
		totolen += strlen(results);
		if(strstr(results, "signal level"))
		{
			memset(results, 0, sizeof(512));
			continue;
		}
		results[strlen(results) - 1] = 0;
		p = strtok(results, "\t");

		i = 0;
		memset(&m_ap_info, 0, sizeof(m_ap_info));
		while(p)
		{ 
			if (p)
			{
				if(0 == i)
				{
					strcpy(m_ap_info.mac, p);
				}
				else if(2 == i)
				{
					m_ap_info.quality= atoi(p);
				}
				else if(3 == i)
				{
					if(strstr(p, "WPA") || strstr(p, "WEP") )
						m_ap_info.encypted = 1;
					else
						m_ap_info.encypted = 0;

					if(!strstr(p, "["))
					{

						strcpy(m_ap_info.essid, p);
						i++;
					}

				}	
				else if(4 == i)
				{
					strcpy(m_ap_info.essid, p);
				}	
				i++;
			}
			p = strtok(NULL, "\t");
		}   
		if(strlen(m_ap_info.essid) > 0 && (0 == SSID_areadly_in_list(ap , m_ap_info)))
		{
			memcpy(&ap->ap_info[ap->ap_count], &m_ap_info, sizeof(m_ap_info));
			ap->ap_count++;
			if(ap->ap_count >= 64)
			{
				break;
			}
		}
		memset(results, 0, sizeof(512));
	}   
	pclose(fp);
	return 0;
}

int wifi_get_ap_list(ap_list *ap)
{
	if (NULL == ap) 
		return -1;

	scan_ap_list_extra(ap);
	return 0;
}

int dev_connected(char * net_dev, int check_carrie)
{
	int ret = 0;
	if(NULL == net_dev)
		return 0;

	if(wifi_check_operstate(net_dev) > 0)
	{
		if(1 == check_carrie)
		{
			if(wifi_check_carrier(net_dev) > 0)
				ret = 1;
			else
				ret = 0;
		}
		else
			ret = 1;
	}
	else
		ret = 0;

	dbg_log_print(LOG_DEBUG0, "dev_connected ret = %d ", ret);
	return ret;
}

int  ping_by_dev(char *net_dev, int check_ip_flag)
{
	char ipaddr[16] = {0};

	if(NULL == net_dev)
		return 0;

	if(strstr(net_dev, "ppp0"))
		check_ip_flag = 0;

	if(dev_connected(net_dev, 1) > 0)
	{
		if(1 == check_ip_flag)
		{
			if(get_ipaddr(net_dev, ipaddr) > 0)
			{
				return 1;
			}
			else 
				return 0;
		}
		else
			return 1;
	}
	else
		return 0;
}

int dev_connected_extra(char * net_dev, int check_ip_flag)
{
	char ipaddr[16] = {0};

	if(NULL == net_dev)
		return 0;

	if(strstr(net_dev, "ppp0"))
		check_ip_flag = 0;

	if(wifi_check_carrier(net_dev)  > 0)
	{
		if(1 == check_ip_flag)
		{
			if(get_ipaddr(net_dev, ipaddr) > 0)
			{
				return 1;
			}
			else 
			{
				return 0;
			}
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return 0;
	}

}



//return  0 when down
//return  1 when up
//return -1 when failed.
int wifi_check_operstate(const char * net_dev)
{
	char file_path[32] = {0};
	char buf[12] = {0};
	int rlen = 0;
	int ret = 0;

	sprintf(file_path, "/sys/class/net/%s/", net_dev);

	if(0 != access(file_path, F_OK))
	{
		dbg_log_print(LOG_ERR, "Access dir failed! path = %s",file_path);
		return -1;
	}

	sprintf(file_path, "/sys/class/net/%s/operstate", net_dev);
	int fd = open(file_path, O_RDONLY);
	if(fd > 0)
	{
		rlen = read(fd, buf, 12);
		dbg_log_print(LOG_DEBUG0, "wifi_check_operstate %s: %s ", file_path, buf);
		if(rlen > 0)
		{
			if(strncmp(buf, "down", 4))
			{
				ret = 1;
			}

		}
		if(wireless_device_status)
		{
			ret = 1;			
		}
		close(fd);
	}
	else
	{
		dbg_log_print(LOG_ERR,"open failed %s ", file_path);
		if(strstr(net_dev, WIRELESS_DEV))
		{
			check_wpa_supplicant(0); 
		}	
		ret =  -1;
	}
	return ret;
}






/************************************************* 
Function: interface_down
Description: close net device
Input:  net_dev  
Output:  
Return: 0 sucess -1 failed
Others:
 *************************************************/
int wifi_interface_down(const char * net_dev)
{
	if(strcmp(net_dev,"lo") == 0)
	{
		dbg_log_print(LOG_WARN,"You can't pull down interface lo");
		return 0;
	}

	int s = 0;
	char file_path[32] = {0};

	sprintf(file_path, "/sys/class/net/%s/", net_dev);
	if(access(file_path, F_OK))
		return -1;

	if(set_ipaddr(net_dev, "0") < 0)
		return -1;

	if((s = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		dbg_log_print(LOG_ERR, "wifi_interface_down: ioctl");
		return -1;
	}

	struct ifreq ifr;
	strcpy(ifr.ifr_name,net_dev);

	short flag;
	flag = ~IFF_UP;
	if(ioctl(s,SIOCGIFFLAGS,&ifr) < 0)
	{
		dbg_log_print(LOG_ERR,"net_dev %s not exists ", net_dev);
		close(s);
		return -1;
	}

	ifr.ifr_ifru.ifru_flags &= flag;

	if(ioctl(s,SIOCSIFFLAGS,&ifr) < 0)
	{
		dbg_log_print(LOG_ERR, "wifi_interface_down: ioctl");
		close(s);
		return -1;
	}

	dbg_log_print(LOG_DEBUG0,"wifi_interface_down[%s]",net_dev);
	wireless_device_status = 0;
	close(s);

	return 0;
}


/************************************************* 
Function: set_ipaddr
Description: set ip addr 
Input:  net_dev  ipaddr
Output:  
Return: 0 sucess -1 failed
Others:
 *************************************************/
int set_ipaddr(const char *net_dev, const char* ipaddr)
{
	struct ifreq ifr;
	int fd = 0;
	struct sockaddr_in *pAddr;

	if((NULL == net_dev) || (NULL == ipaddr))
	{
		dbg_log_print(LOG_ERR, "illegal call function SetGeneralIP!");
		return -1;
	}

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
	{
		dbg_log_print(LOG_ERR,"socket....setip..false!!!");
		return -1;
	}

	strcpy(ifr.ifr_name, net_dev);

	pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);
	bzero(pAddr, sizeof(struct sockaddr_in));
	pAddr->sin_addr.s_addr = inet_addr(ipaddr);
	pAddr->sin_family = AF_INET;
	if (ioctl(fd, SIOCSIFADDR, &ifr) < 0)
	{
		close(fd);
		dbg_log_print(LOG_ERR,"ioctl..set_ipaddr..false!!!");
		return -1;
	}
	close(fd);
	return 0;
}


static int  check_encryption_mode(network_config *_network_config, char * ssid)
{
	char ipaddr[16] = {0};
	char results[1024] = {0};
	char cmd[128] = {0};
	int iFind = 0;	
	check_wpa_supplicant(1);
	if( wifi_check_carrier(WIRELESS_DEV) > 0 ) {
		if( get_ipaddr(WIRELESS_DEV, ipaddr) <= 0){
			system("wpa_cli -i ra0 remove_network 0 ");
		}
	}

	if(_network_config->wifi_config_info.hide)
		return 1;

	system("wpa_cli -i ra0 scan ");
	sleep(1);
	sprintf(cmd, "wpa_cli -i ra0 scan_results | grep \"%s\" ", ssid);
	cmd_run(cmd, results, 1024);

	if(strlen(results) > 0)
	{
		if(strstr(results, "WPA"))
		{
			_network_config->wifi_config_info.encryption = WIRELESS_WPA_WPA2;
		}
		else if(strstr(results, "WEP"))
		{
			_network_config->wifi_config_info.encryption = WIRELESS_WEP;
		}
		else
		{
			_network_config->wifi_config_info.encryption = WIRELESS_NONE;
		}
		dbg_log_print(LOG_INFO, "checkEncryptionMode find essid  %s ", ssid );
		iFind = 1;
	}
	else
	{
		dbg_log_print(LOG_INFO, "checkEncryptionMode  essid %s  not find ", ssid );
		iFind = 0;
	}
	return iFind;
}

static int connect_wifi(network_config *_network_config, wifi_config *wifi_config_info, int *stop)
{
	char cmd_buf[512] = {0};
	int i = 0;
	int ret = 0;

	if(wifi_check_operstate(WIRELESS_DEV) < 0)
	{
		dbg_log_print(LOG_ERR, "connect failed ");
		return -1;
	}

	wifi_interface_up(WIRELESS_DEV);
	for(i=0; i<3; i++)
	{
		if(*stop)
		{
			dbg_log_print(LOG_ERR, "connect_wifi is break by client! ");
			*stop = 0;
			return -1;
		}

		ret = check_encryption_mode(_network_config, wifi_config_info->ssid);

		if(0 == ret )			
			sleep(2);
		else
			break;
	}

	if((i >= 3) && (0 == ret))
	{
		dbg_log_print(LOG_ERR, "can't find AP %s  \n", wifi_config_info->ssid);
		return -1;
	}

	system("wpa_cli -i ra0 remove_network 0 ");
	system("wpa_cli -i ra0 add_network");

	sprintf(cmd_buf, "wpa_cli -i ra0 set_network 0 ssid \'\"%s\"\'", wifi_config_info->ssid);
	system(cmd_buf);
	memset(cmd_buf, 0, 512);

	if( WIRELESS_NONE  == wifi_config_info->encryption )
	{
		system("wpa_cli -i ra0 set_network 0 key_mgmt NONE ");
	}
	else if( WIRELESS_WEP  == wifi_config_info->encryption )
	{
		system("wpa_cli -i ra0 set_network 0 key_mgmt NONE ");
		if(strlen(wifi_config_info->key) == 5 || strlen(wifi_config_info->key) == 13)
			sprintf(cmd_buf,  "wpa_cli -i ra0 set_network 0 wep_key0 \'\"%s\"\'",wifi_config_info->key); 
		else
			sprintf(cmd_buf,  "wpa_cli -i ra0 set_network 0 wep_key0 \'%s\'",wifi_config_info->key); 

		system(cmd_buf);
		memset(cmd_buf, 0, 512);		
	}
	else if( WIRELESS_WPA_WPA2  == wifi_config_info->encryption )
	{
		sprintf(cmd_buf, "wpa_cli -i ra0 set_network 0 psk \'\"%s\"\'", wifi_config_info->key);
		system(cmd_buf);
		memset(cmd_buf, 0, 512);
	}

	system("wpa_cli -i ra0 enable_network 0 ");
	for(i=0; i<15; i++)
	{
		if(*stop)
		{
			dbg_log_print(LOG_ERR, "connect_wifi is break by client! ");
			*stop = 0;
			return -1;
		}

		if(wifi_check_carrier(WIRELESS_DEV) > 0)
		{
			dbg_log_print(LOG_INFO, "connect susscess ");
			return 0;
		}
		else
		{		
			sleep(1);
			dbg_log_print(LOG_INFO, "wait for connect ");
		}
	}
	dbg_log_print(LOG_ERR, "connect failed ");
	return -1;
}


/***********************************************************
  @brief Set the wireless connect spec in DHCP model.
  @param 	ap_name: 	ap's name (ssid)
  password:	ap's password
  encrption:	ap's encrption model
 **********************************************************/
void wifi_set_wireless_dhcp(char *ap_name, char *password,WIFI_ENCRYPTION_TYPE encrption)
{
	int stop = 0; /*Not use it yet */
	network_config* g_network_config = NULL;
	g_network_config = (network_config *) malloc(sizeof(network_config));
	memset(g_network_config, 0, sizeof(network_config));

	/* Set wifi config info*/
	g_network_config->wifi_config_info.encryption = encrption;
	g_network_config->wifi_config_info.hide = 0;
	strcpy(g_network_config->wifi_config_info.ssid , ap_name);	
	strcpy(g_network_config->wifi_config_info.key,  password);


	g_network_config->work_mode = NET_SERVICE_MODE_AUTO;

	g_network_config->net_dev = WIRELESS_DEV;
	set_ipaddr(WIRELESS_DEV, "0");
	if(connect_wifi(g_network_config, &g_network_config->wifi_config_info , &stop ) >= 0)
	{

		system("killall -9 udhcpc");
		system("udhcpc -i ra0 -q -b -n");
		dbg_log_print(LOG_INFO,"try_connect udhcpc -i ra0 -q -b -n ");
	}
}


int wifi_get_network_device_list(network_dev_list *device)
{
	if (NULL == device) 
		return -1;

	check_all_dev_extra(device);
	return 0;
}

/************************************************* 
Function: interface_up
Description: open net device
Input:  net_dev  
Output:  
Return: 0 sucess -1 failed
Others:
 *************************************************/
int wifi_interface_up(const char * net_dev)
{
	int s;
	char file_path[32] = {0};
	sprintf(file_path, "/sys/class/net/%s/", net_dev);
	if(access(file_path, F_OK))
		return -1;

	if((s = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		dbg_log_print(LOG_ERR, "wifi_interface_up socket faild ");
		return -1;
	}

	struct ifreq ifr;
	strcpy(ifr.ifr_name,net_dev);

	short flag;
	flag = IFF_UP;
	if(ioctl(s,SIOCGIFFLAGS,&ifr) < 0)
	{
		dbg_log_print(LOG_ERR, "wifi_interface_up ioctl faild ");
		close(s);
		return -1;
	}

	ifr.ifr_ifru.ifru_flags |= flag;

	if(ioctl(s,SIOCSIFFLAGS,&ifr) < 0)
	{
		dbg_log_print(LOG_ERR, "wifi_interface_up ioctl faild ");
		close(s);
		return -1;
	}

	dbg_log_print(LOG_DEBUG0, "wifi_interface_up[%s]",net_dev);
	wireless_device_status=1;

	close(s);

	return 0;

}

//return  0 when down
//return  1 when up
//return -1 when failed.
int wifi_check_carrier(const char * net_dev)
{
	char file_path[32] = {0};
	char buf[12] = {0};
	int rlen = 0;
	int ret = 0;

	sprintf(file_path, "/sys/class/net/%s/", net_dev);
	if(access(file_path, F_OK))
		return -1;

	sprintf(file_path, "/sys/class/net/%s/carrier", net_dev);

	int fd = open(file_path, O_RDONLY);
	if(fd > 0)
	{
		rlen = read(fd, buf, 12);
		dbg_log_print(LOG_DEBUG0, "wifi_check_carrier %s: %s ", file_path, buf);
		if(rlen > 0)
		{
			ret = atoi(buf);
		}
		close(fd);
	}
	else
	{
		dbg_log_print(LOG_ERR, "open failed %s ", file_path);
		ret =  -1;
	}

	return ret;
}



/************************************************* 

 *************************************************/
int wifi_get_ap_quality(const char *net_dev)
{
	struct iwreq		wrq;
	int fd = 0;
	int ret = 0;
	struct iw_statistics* stats;
	stats = (struct iw_statistics*)malloc(sizeof(struct iw_statistics));

	if(NULL == net_dev)
	{
		dbg_log_print(LOG_ERR, "illegal call function SetGeneralIP!");
		return -1;
	}
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
	{
		dbg_log_print(LOG_ERR,"socket....get quality..false!!!");
		return -1;
	}

	strncpy(wrq.ifr_name, net_dev, IFNAMSIZ);
	wrq.u.data.pointer = (caddr_t)stats;
	wrq.u.data.length = sizeof(struct iw_statistics);
	wrq.u.data.flags = 1;		/* Clear updated flag */

	if (ioctl(fd, SIOCGIWSTATS, &wrq)< 0)
	{
		close(fd);
		//dbg_log_print(LOG_ERR,"2ioctl..get quality..false!!!");
		return -1;
	}
	ret = stats->qual.qual;
	free(stats);
	close(fd);
	return ret;
}


int wifi_ap_disconnect(const char* net_dev)
{
	if(NULL == net_dev)
	{
		dbg_log_print(LOG_ERR, "illegal call function disconnect_wifi_ap!");
		return -1;
	}
	int start = 0;
	check_wpa_supplicant(start);
	return 0;
}


int wifi_network_get_ip_address(const char *net_dev)
{

	struct ifreq ifr;
	int fd = 0;
	int ret = -1;
	struct sockaddr_in *pAddr;

	if(NULL == net_dev)
	{
		dbg_log_print(LOG_ERR, "illegal call function SetGeneralIP!");
		return -1;
	}

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
	{
		dbg_log_print(LOG_ERR,"open socket failed");
		return -1;
	}

	memset(&ifr,0,sizeof(ifr));
	strcpy(ifr.ifr_name, net_dev);

	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
	{
		dbg_log_print(LOG_INFO,"SIOCGIFADDR socket failed");
		close(fd);
		return -1;
	}
	char ipaddr[10];
	pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);    	
	strcpy(ipaddr, inet_ntoa(pAddr->sin_addr));	
	//printf("pAddr=%s\n",ipaddr);
	//printf("pAddr->sin_addr.s_addr=%p\n",(void*)pAddr->sin_addr.s_addr);
	ret =wifi_inet_aton(&pAddr->sin_addr.s_addr);
	close(fd);
	return ret;
}


int wifi_inet_aton(unsigned char *ipa)
{
	int i;
	int ipv = 0;
	for(i = 0; i < 4; i++)
		ipv = (ipv << 8) | ipa[i];
	return ipv;
}

int wifi_get_status(char * net_dev)
{
	printf("@@\n");
	if(NULL == net_dev)
		return -1 ;

	FILE *fp;
	char cmd_buf[64] = {0};
	sprintf(cmd_buf, "iwconfig %s", net_dev);
	//system(cmd_buf);	

	printf("@@\n");
	dbg_log_print(LOG_INFO, "run_cmd: [%s]", cmd_buf);

	if((fp = popen(cmd_buf, "r")) == NULL) 
	{
		dbg_log_print(LOG_ERR, "Fail to popen %s", cmd_buf);
		return -1;
	} 
	int totolen=0;
	char results[512] = {0};
	char statustr[1024]={0};
	while(fgets(results, 512, fp) != NULL) 
	{
		totolen += strlen(results);
		strcat(statustr,results);
		printf("1log is %s\n", results);
		printf("log is %d\n", totolen);
		char *p;
		p = strtok(results, "\t");
		while(p)
		{ 
			if (p)
			{
				printf("2log is %s\n", p);
			}
			p = strtok(NULL, "\t");
		}
		memset(results, 0, sizeof(512));
	}
	pclose(fp);

	printf("XXXXX %s\n", statustr);
	return 0;

}



int network_get_scan_result(const char *net_dev)
{
	struct iwreq		wrq;
	int fd = 0;
	int ret = 0;
	struct iw_statistics* stats;
	stats = (struct iw_statistics*)malloc(sizeof(struct iw_statistics));

	if(NULL == net_dev)
	{
		dbg_log_print(LOG_ERR, "illegal call function SetGeneralIP!");
		return -1;
	}
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) 
	{
		dbg_log_print(LOG_ERR,"socket....get quality..false!!!");
		return -1;
	}

	strncpy(wrq.ifr_name, net_dev, IFNAMSIZ);
	wrq.u.data.pointer = (caddr_t)stats;
	wrq.u.data.length = sizeof(struct iw_statistics);
	wrq.u.data.flags = 1;		/* Clear updated flag */

	if (ioctl(fd, SIOCGIWSTATS, &wrq)< 0)
	{
		close(fd);
		//dbg_log_print(LOG_ERR,"2ioctl..get quality..false!!!");
		return -1;
	}
	ret = stats->qual.qual;
	free(stats);
	close(fd);
	return ret;
}
