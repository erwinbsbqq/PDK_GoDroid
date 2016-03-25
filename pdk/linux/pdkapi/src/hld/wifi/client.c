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
#include <hld_cfg.h>

//#include "netservice.h"
//#include "../../../app/hybrid/wifi_deamon/netservice.h"
#include "netinterface.h"
#include "qlog.h"
network_config *m_network_config = NULL;
int socket_descriptor;
struct sockaddr_in address;
static p_net_status g_net_status;

#define WIFI_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(WIFI, fmt, ##args); \
			} while(0)


int send_package()
{
    int isend = 0;

    struct sockaddr_in sin;
	int  sin_len=sizeof(sin);

	if(NULL == m_network_config)
		return 0;
    isend = sendto(socket_descriptor, m_network_config, sizeof(network_config),
		0,(struct sockaddr *)&address, sin_len);
	dbg_log_print(LOG_INFO,"isend = %d \n", isend);
	WIFI_DBG_PRINT("File:%s,Function:%s,Line:%d   isend = [%d] \n",__FILE__,__FUNCTION__,__LINE__,isend);
//	irecv =  recvfrom(socket_descriptor, buf, 256,0,(struct sockaddr *)&sin, &sin_len);
	return isend;
}

network_config * get_current_network_config()
{
	return m_network_config;
}

p_net_status get_net_status()
{
	return g_net_status;
}

static networkCmdNotify g_networkCmdNotify_func = NULL;

void *thr_work_thread(void *arg)
{
	NTCmdEvent m_cmd_event = CMD_LOCAL_CONNECTED;
	p_net_status pcurrent_stauts;
	net_status  m_old_status;
	m_old_status.net_connect_type = 0;
	m_old_status.dev_status = 0;	
	while(1)
	{
		sleep(1);
		pcurrent_stauts = get_net_status();
		if((pcurrent_stauts->net_connect_type != m_old_status.net_connect_type ) ||
			(pcurrent_stauts->dev_status != m_old_status.dev_status))
		{
			if(pcurrent_stauts->net_connect_type < TYPE_WIRED_PPPOE)
			{	
				if(1 == pcurrent_stauts->dev_status)
					m_cmd_event = CMD_LOCAL_CONNECTED;
				else if(0 == pcurrent_stauts->dev_status)
					m_cmd_event = CMD_LOCAL_DISCONNECTED;
                else
					m_cmd_event = CMD_CONNECT_FAILED;
			}
			else if (pcurrent_stauts->net_connect_type > TYPE_WIRED_PPPOE)
			{
				if(1 == pcurrent_stauts->dev_status)
					m_cmd_event = CMD_WIFI_CONNECTED;
                else if(0 == pcurrent_stauts->dev_status)
                    m_cmd_event = CMD_WIFI_DISCONNECTED;
                else
                    m_cmd_event = CMD_CONNECT_FAILED;
			}
			else
			{
				if(1 == pcurrent_stauts->dev_status)
					m_cmd_event = CMD_PPPOE_CONNECTED;
                else if(0 == pcurrent_stauts->dev_status)
					m_cmd_event = CMD_PPPOE_DISCONNECTED;
                else 
                    m_cmd_event = CMD_CONNECT_FAILED;
			}
			
			m_old_status.dev_status = pcurrent_stauts->dev_status;
			m_old_status.net_connect_type = pcurrent_stauts->net_connect_type;

			if(NULL != g_networkCmdNotify_func)
				g_networkCmdNotify_func(m_cmd_event,  pcurrent_stauts->work_mode);
		}
	}
	return 0;
}
 

int init_network_service(networkCmdNotify ntf,NET_SERVICE_MODE _type)
{
	if(NULL == ntf )
		return -1;
	
	g_networkCmdNotify_func = ntf;

#if 0
    //if (dbg_log_init("client", LOG_ERR|LOG_WARN|LOG_INFO|LOG_DEBUG0) != 0) {
    if (dbg_log_init("client", LOG_ERR|LOG_WARN) != 0) {
        WIFI_DBG_PRINT("dbg_log_init failed ");
        return (-1);
    }
#endif

    dbg_log_init("client", LOG_ERR|LOG_WARN);
	if(NULL != m_network_config)
    {
        m_network_config->work_mode = _type;
        if(NET_SERVICE_MODE_AUTO  == _type)
        {
            send_package();
		}
		return 0;
	}
    int status_fd;
	pthread_t pid_work_thread;
	m_network_config = (network_config *) malloc(sizeof(network_config));
	if(!m_network_config) {
		return -1;
	}
	memset(m_network_config, 0, sizeof(network_config));
	net_status	m_net_status;
	memset(&m_net_status, 0, sizeof(net_status));
	
	//get this infos from config file.
	m_network_config->work_mode = _type;
	//get_configinfo(m_network_config );

	
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(UDP_PORT);
    socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

    if ((status_fd = open(SHARED_FILENAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) < 0) {
        dbg_log_print(LOG_ERR, "Cannot open the %s file!\n", SHARED_FILENAME);
        return (-1);
    }   

	write(status_fd, &m_net_status, sizeof(net_status));
    if (NULL == (g_net_status = (p_net_status) mmap(0, sizeof(net_status), 
        PROT_READ|PROT_WRITE, MAP_SHARED, status_fd, 0))) {
        dbg_log_print(LOG_ERR,"Error to map the temporary file!\n");
        close(status_fd);
        return (-1);
    }   


    close(status_fd);    
	if(pthread_create(&pid_work_thread, NULL, thr_work_thread, NULL))
	{
		dbg_log_print(LOG_ERR, "pthread_create thr_work_thread failed");
		return (-1);
	}

	return 0;
}

int select_dev(char * net_dev)
{
	if(NULL == m_network_config)
		return -1;
	
	if(!strcmp(net_dev, WIRED_DEV))
		m_network_config->net_dev = WIRED_DEV;
	else
		m_network_config->net_dev = WIRELESS_DEV;
	return 0;
}

void select_connect_type(int type)
{
	if(NULL == m_network_config)
		return ;
	m_network_config->net_connect_type = type;
}

void add_hide_ssid(char * ssid)
{
	if(NULL == ssid)
		return ;

	char cmd_buf[64] = {0};
	sprintf(cmd_buf, "iwconfig ra0 essid \"%s\" ", ssid);
	system(cmd_buf);
}

void set_ip_infos(char * net_dev, char *Ipaddr, char *mask,char *gateway, char * dns)
{
	if(NULL == m_network_config)
		return ;

	if(!strcmp(net_dev, WIRED_DEV))
	{
		strcpy(m_network_config->wired_ip_config.ipaddr, Ipaddr);
		strcpy(m_network_config->wired_ip_config.submask, mask);
		strcpy(m_network_config->wired_ip_config.gatway, gateway);	
		strcpy(m_network_config->wired_ip_config.dns1, dns);
	}
	else
	{
		strcpy(m_network_config->wireless_ip_config.ipaddr, Ipaddr);
		strcpy(m_network_config->wireless_ip_config.submask, mask);
		strcpy(m_network_config->wireless_ip_config.gatway, gateway);	
		strcpy(m_network_config->wireless_ip_config.dns1, dns);
	}
}

#if 0
void set_wifi_ssid(char * ssid, char *psk)
{
	if(NULL == m_network_config)
		return ;
	m_network_config->wifi_config_info.encryption = WIRELESS_WPA_WPA2;
	m_network_config->wifi_config_info.hide = 0;
	strcpy(m_network_config->wifi_config_info.ssid , ssid);	
	strcpy(m_network_config->wifi_config_info.key,  psk);
}
#endif

void set_wifi_ssid(char * ssid, char *psk, WIFI_ENCRYPTION_TYPE encrption)
{
	if(NULL == m_network_config)
		return ;
	m_network_config->wifi_config_info.encryption = encrption;
	m_network_config->wifi_config_info.hide = 0;
	strcpy(m_network_config->wifi_config_info.ssid , ssid);	
	strcpy(m_network_config->wifi_config_info.key,  psk);
}

void set_pppoe(char * ssid, char *psk)
{
	if(NULL == m_network_config)
		return ;
	strcpy(m_network_config->str_account,  ssid);	
	strcpy(m_network_config->str_account, psk);
}

