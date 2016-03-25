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
@brief 初始化网络调用，注册回调函数和工作模式
@param ntf:网络设备回调注册
@param mode: 	NET_SERVICE_MODE_AUTO			:进入工作模式
				NET_SERVICE_MODE_IN_SETTING		:进入设定模式
@return 返回获取的结果，0为成功, -1为失败
**********************************************************/
int wifi_start_config_network(networkCmdNotify ntf, NET_SERVICE_MODE mode)
{
	return init_network_service( ntf, mode);
}

/***********************************************************
@brief 获取网络设备列表
@param device:外部传入的network_dev_list指针,用于存在网络设备的状态
@return 返回获取的结果，0为成功, -1为失败
**********************************************************/
int wifi_get_network_device_list(network_dev_list *device)
{
	if (NULL == device) 
		return -1;
	
	check_all_dev_extra(device);
	return 0;
}


/***********************************************************
@brief 获取无线发射台的列表
@param ap:外部传入的ap_list指针,用于存放无线发射台的信息
@return 返回获取的结果，0为成功, -1为失败
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
@brief 查询网络设备工作状态

@param device_name: 	网络接口设备名;		eth0:有线设备	ra0:无线设备
@param check_ip: 		是否需要检查IP地址

@return 返回获取的结果:
							return
							0				0:网线没有插入或者无线没有连接/1:网线已经插入或者无线已握手
							1				原则上没有意义
							0				0:连接没有建立/1:连接已经建立
							1				0:连接没有建立或者ip无效/1:已获得ip地址
**********************************************************/
int get_dev_is_connected(char *device_name, int check_ip)
{
	return dev_connected_extra(device_name, check_ip);
}

/***********************************************************
@brief 扫描隐藏的无线发射台，调用此接口用，可以通过get_ap_list来更新ap的列表
@param hide_ssid:		隐藏ssid的名字
**********************************************************/
void wifi_scan_hide_ssid(char *hide_ssid)
{
	add_hide_ssid(hide_ssid);
}

/***********************************************************
@brief 设置有线网络的DHCP工作模式
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
@brief 设置有线网络的手动设置IP工作模式
@param ip: 传入ip_info类型的指针，传递需要设置的IP,mask,gateway,dns.
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
@brief 设置有线网络播放方式的工作模式
@param param:传入dialup_info类型的指针，传递需要设置的帐号，密码.
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
@brief 设置无线网络的手动设置IP工作模式
@param 	ap_name: 	外部传入的字符串指针,ap的名字(ssid)
		password:	外部传入的字符串指针,密码
		ip:传入ip_info类型的指针，传递需要设置的IP,mask,gateway,dns.
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


