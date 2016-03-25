#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <hld_cfg.h>
//#include "client_hal.h"
//#include "netservice.h"
//#include "../../../app/hybrid/wifi_deamon/netservice.h"
#include "netinterface.h"
#include "qlog.h"


#define CLIENT_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(CLIENT, fmt, ##args); \
			} while(0)


/***********************************************************
@brief ��ʼ��������ã�ע��ص������͹���ģʽ
@param ntf:�����豸�ص�ע��
@param mode: 	NET_SERVICE_MODE_AUTO			:���빤��ģʽ
				NET_SERVICE_MODE_IN_SETTING		:�����趨ģʽ
@return ���ػ�ȡ�Ľ����0Ϊ�ɹ�, -1Ϊʧ��
**********************************************************/
int wifi_start_config_network(networkCmdNotify ntf, NET_SERVICE_MODE mode)
{
	return init_network_service( ntf, mode);
}

/***********************************************************
@brief ��ȡ�����豸�б�
@param device:�ⲿ�����network_dev_listָ��,���ڴ��������豸��״̬
@return ���ػ�ȡ�Ľ����0Ϊ�ɹ�, -1Ϊʧ��
**********************************************************/
int wifi_get_network_device_list(network_dev_list *device)
{
	if (NULL == device) 
		return -1;
	
	check_all_dev_extra(device);
	return 0;
}


/***********************************************************
@brief ��ȡ���߷���̨���б�
@param ap:�ⲿ�����ap_listָ��,���ڴ�����߷���̨����Ϣ
@return ���ػ�ȡ�Ľ����0Ϊ�ɹ�, -1Ϊʧ��
**********************************************************/
int wifi_get_ap_list(ap_list *ap)
{
	if (NULL == ap) 
		return -1;
	
	scan_ap_list_extra(ap);
	return 0;
}

extern int dev_connected_extra(char * net_dev, int check_ip);
/***********************************************************
@brief ��ѯ�����豸����״̬

@param device_name: 	����ӿ��豸��;		eth0:�����豸	ra0:�����豸
@param check_ip: 		�Ƿ���Ҫ���IP��ַ

@return ���ػ�ȡ�Ľ��:
							return
							0				0:����û�в����������û������/1:�����Ѿ������������������
							1				ԭ����û������
							0				0:����û�н���/1:�����Ѿ�����
							1				0:����û�н�������ip��Ч/1:�ѻ��ip��ַ
**********************************************************/
int get_dev_is_connected(char *device_name, int check_ip)
{
	return dev_connected_extra(device_name, check_ip);
}

/***********************************************************
@brief ɨ�����ص����߷���̨�����ô˽ӿ��ã�����ͨ��get_ap_list������ap���б�
@param hide_ssid:		����ssid������
**********************************************************/
void wifi_scan_hide_ssid(char *hide_ssid)
{
	add_hide_ssid(hide_ssid);
}

/***********************************************************
@brief �������������DHCP����ģʽ
**********************************************************/
void set_wired_dhcp(void)
{
	if(select_dev(WIRELESS_DEV) < 0)
	{
		CLIENT_DBG_PRINT("please call wifi_start_config_network first \n");
		return ;
	}
	select_connect_type(TYPE_WIRED_DHCP);
	send_package();
}

/***********************************************************
@brief ��������������ֶ�����IP����ģʽ
@param ip: ����ip_info���͵�ָ�룬������Ҫ���õ�IP,mask,gateway,dns.
**********************************************************/
void set_wired_fixedip(ip_info *ip)
{
	if(select_dev(WIRELESS_DEV) < 0)
	{
		CLIENT_DBG_PRINT("please call wifi_start_config_network first \n");
		return ;
	}
	select_connect_type(TYPE_WIRED_FIXEDIP);
	set_ip_infos(WIRED_DEV, ip->ip_address, ip->mask, ip->gateway, ip->dns);
	send_package();
}

/***********************************************************
@brief �����������粥�ŷ�ʽ�Ĺ���ģʽ
@param param:����dialup_info���͵�ָ�룬������Ҫ���õ��ʺţ�����.
**********************************************************/
void set_wired_dialup(dialup_info *param)
{
	if(select_dev(WIRELESS_DEV) < 0)
	{
		CLIENT_DBG_PRINT("please call wifi_start_config_network first \n");
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
	if(select_dev(WIRELESS_DEV) < 0)
	{
		CLIENT_DBG_PRINT("please call wifi_start_config_network first \n");
		return ;
	}
	select_connect_type(TYPE_WIRELESS_DHCP);
	set_wifi_ssid(ap_name, password,encrption);
	send_package();
}

/***********************************************************
@brief ��������������ֶ�����IP����ģʽ
@param 	ap_name: 	�ⲿ������ַ���ָ��,ap������(ssid)
		password:	�ⲿ������ַ���ָ��,����
		ip:����ip_info���͵�ָ�룬������Ҫ���õ�IP,mask,gateway,dns.
**********************************************************/
void wifi_set_wireless_fixedip(char *ap_name, char *password, ip_info *ip)
{
	if(select_dev(WIRELESS_DEV) < 0)
	{
		CLIENT_DBG_PRINT("please call wifi_start_config_network first \n");
		return ;
	}
	select_connect_type(TYPE_WIRELESS_FIXEDIP);
	set_ip_infos(WIRELESS_DEV, ip->ip_address, ip->mask, ip->gateway, ip->dns);
	set_wifi_ssid(ap_name, password,0);
	send_package();
}


