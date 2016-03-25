#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <ctype.h>
#include"config.h"
#include"qlog.h"

/*
strip right space
 */
char *strtrimr(char *pstr, int n)
{
    int i = 0;
    i = strlen(pstr);
    if (i >= n) {
        i = n-1;
    }
    while ((i>=0) && isspace(pstr[i]))
        pstr[i--] = '\0';
    return pstr;
}

/*
strip left space
 */
char *strtriml(char *pstr, int n)
{
    int i = 0,j;
    j = strlen(pstr);
    if (j >= n) {
        j = n-1;
        pstr[j] = '\0';
    }
    while ((i < j) && isspace(pstr[i]))
        i++;
    if (i > 0)
        strcpy(pstr, &pstr[i]);
    return pstr;
}
/*
strip right and left space
 */
char *strtrim(char *pstr, int n)
{
    char *p;
    p = strtrimr(pstr, n);
    return strtriml(p, n);
}


/*
get key and value from one line
 */
int  get_item_from_line(char *line,  ITEM *item, int n)
{
    char *p = NULL;
    int len = 0;
    int tmp_len = 0;
    p = strtrim(line, n);
    len = strlen(p);
    if(len <= 0){
        return -1;
    } else if(p[0]=='#'){
        return -2;
    }else{
        char *p2 = strchr(p, '=');
        *p2++ = '\0';
        item->key = (char *)malloc(strlen(p) + 1);
        if(!item->key) {
            return -3;
        }
        tmp_len = strlen(p2);
        if(tmp_len  >= 1024) {
        	tmp_len = 1023;
        	p2[1023] = '\0';
        }
        item->value = (char *)malloc(tmp_len+1);
        if(!item->value) {
            free(item->key);
            return -3;
        }
        strcpy(item->key,p);
        strcpy(item->value,p2);
        }
    return 0;
}

int file_to_items(ITEM *items, int *num)
{
    char line[1024];
    FILE *fp;
    int tmp_len = 0;
    char * p = NULL;
    fp = fopen(NET_CONFIG_FILE, "r");
    if(fp == NULL)
        return -1;
    int i = 0;
    while(fgets(line, 1024, fp)){
        line[1023] = '\0';
    	p = strtrim(line, 1024);
        int len = strlen(p);
        if(len <= 0){
            continue;
        }
        else if(p[0]=='#'){
            continue;
        }else{
            char *p2 = strchr(p, '=');
            if(p2 == NULL)
                continue;
            *p2++ = '\0';
            tmp_len = strlen(p);
            if(tmp_len >= 1024) {
            	tmp_len = 1023;
            	p[1023] = '\0';
            }
            items[i].key = (char *)malloc(tmp_len + 1);
            if(!items[i].key) {
            	goto err_out;
            }
            tmp_len = strlen(p2);
            if(tmp_len >= 1024) {
            	tmp_len = 1023;
            	p2[1023] = '\0';
            }
            items[i].value = (char *)malloc(tmp_len + 64);
            if(!items[i].value) {
            	free(items[i].key);
            	goto err_out;
            }
            strcpy(items[i].key,p);
            strcpy(items[i].value,p2);
            i++;
        }
    }
    (*num) = i;
    fclose(fp);
    return 0;
err_out:
	for(i=0; i<24; i++) {
		if(items[i].key) {
			free(items[i].key);
		}
		if(items[i].value) {
			free(items[i].value);
		}
	}
	fclose(fp);
	return -1;
}

/*
 *get value
 */
int read_conf_value(const char *key, char *value)
{
    char line[1024];
    FILE *fp;
    ITEM item;
    int bfind = 0;
    fp = fopen(NET_CONFIG_FILE, "r");
    if(fp == NULL)
        return -1;
    while (fgets(line, 1024, fp)){
        line[1023] = '\0';
        if (get_item_from_line(line, &item, 1024) < 0) {
            continue;
        }
        if(!strcmp(item.key,key)){
            strcpy(value,item.value);
            free(item.key);
            free(item.value);
            bfind = 1;
            break;
        }
        free(item.key);
        free(item.value);
    }
    fclose(fp);
    return bfind ? 0 : -1;
}

int write_conf_value(const char *key,char *value)
{
    ITEM items[24];
    int num = 0;
	int bfind = 0;
    int i=0;
    int ret = 0;
    for(i=0; i++; i<24) {
    	memset(&items[i], 0, sizeof(ITEM));
    }
    if(file_to_items(items, &num) == 0)
    {
        for(i=0;i<num;i++){
            if(!strcmp(items[i].key, key)){
                strcpy(items[i].value, value);
                bfind = 1;
                break;
            }
        }

    }

	if(!bfind)
	{
        items[num].key = (char *)malloc(strlen(key) + 1);
        if (!items[num].key) {
            ret = -1;
        	goto out;
        }
        items[num].value = (char *)malloc(strlen(value) + 1);
        if (!items[num].value) {
            free(items[num].key);
        	items[num].key = NULL;
        	ret = -1;
        	goto out;
        }
        strcpy(items[num].key, key);
        strcpy(items[num].value, value);
		num += 1;
	}

    FILE *fp;
    fp = fopen(NET_CONFIG_FILE, "w+");
    if(fp == NULL) {
    	ret = -1;
    	goto out;
   }

    for(i=0;i<num;i++){
        fprintf(fp,"%s=%s\n",items[i].key, items[i].value);
    //  dbg_log_print(LOG_DEBUG0,"%s=%s\n",items[i].key, items[i].value);
    }
    fclose(fp);
out:
    for(i=0;i<num;i++){
    	if(items[i].key) {
        free(items[i].key);
    	}
    	if(items[i].value) {
        free(items[i].value);
    	}
    }
    return ret;
}

int get_configinfo(network_config * _network_config)
{
	ITEM items[24];
    int num = 0;
    int i=0;
    for(i=0; i<24; i++) {
    	memset(&items[i], 0, sizeof(ITEM));
    }
    if(file_to_items(items, &num) < 0)
		return -1;


    for(i=0;i<num;i++){
		dbg_log_print(LOG_INFO, "%s=%s", items[i].key, items[i].value);
		if(!strcmp(items[i].key, "net_connect_type"))
			_network_config->net_connect_type = atoi(items[i].value);
		else if(!strcmp(items[i].key, "wired_ip_config.ipaddr"))
			strcpy(_network_config->wired_ip_config.ipaddr, items[i].value);
		else if(!strcmp(items[i].key, "wired_ip_config.submask"))
			strcpy(_network_config->wired_ip_config.submask, items[i].value);
		else if(!strcmp(items[i].key, "wired_ip_config.gatway"))
			strcpy(_network_config->wired_ip_config.gatway, items[i].value);
		else if(!strcmp(items[i].key, "wired_ip_config.dns1"))
			strcpy(_network_config->wired_ip_config.dns1, items[i].value);
		else if(!strcmp(items[i].key, "wireless_ip_config.ipaddr"))
			strcpy(_network_config->wireless_ip_config.ipaddr, items[i].value);
		else if(!strcmp(items[i].key, "wireless_ip_config.submask"))
			strcpy(_network_config->wireless_ip_config.submask, items[i].value);
		else if(!strcmp(items[i].key, "wireless_ip_config.gatway"))
			strcpy(_network_config->wireless_ip_config.gatway, items[i].value);
		else if(!strcmp(items[i].key, "wireless_ip_config.dns1"))
			strcpy(_network_config->wireless_ip_config.dns1, items[i].value);
		else if(!strcmp(items[i].key, "wifi_config_info.encryption"))
			_network_config->wifi_config_info.encryption = atoi(items[i].value);
		else if(!strcmp(items[i].key, "wifi_config_info.hide"))
			_network_config->wifi_config_info.hide = atoi(items[i].value);
		else if(!strcmp(items[i].key, "wifi_config_info.ssid"))
			strcpy(_network_config->wifi_config_info.ssid, items[i].value);			
		else if(!strcmp(items[i].key, "wifi_config_info.key"))
			strcpy(_network_config->wifi_config_info.key, items[i].value);			
		else if(!strcmp(items[i].key, "str_account"))
			strcpy(_network_config->str_account, items[i].value);	
		else if(!strcmp(items[i].key, "str_password"))
			strcpy(_network_config->str_password, items[i].value);				
    }
	
    for(i=0;i<24;i++){
    	if(items[i].key) {
        free(items[i].key);
    	}
    	if(items[i].value) {
        free(items[i].value);
    }
    }
	return 0;
}

void save_config_info(network_config * _network_config)
{
		char tmp_buf[12] = {0};
		memset(tmp_buf, 0, 12);
		
		sprintf(tmp_buf, "%d", _network_config->net_connect_type);
		write_conf_value("net_connect_type", tmp_buf);
		write_conf_value("net_dev", _network_config->net_dev);
		dbg_log_print(LOG_INFO, "save_config_info called");
		
		if(TYPE_WIRED_FIXEDIP == _network_config->net_connect_type)
		{
			dbg_log_print(LOG_INFO,"saved TYPE_WIRED_FIXEDIP  ");
			write_conf_value("wired_ip_config.ipaddr", _network_config->wired_ip_config.ipaddr);
			write_conf_value("wired_ip_config.gatway", _network_config->wired_ip_config.gatway);
			write_conf_value("wired_ip_config.submask", _network_config->wired_ip_config.submask);
			write_conf_value("wired_ip_config.dns1", _network_config->wired_ip_config.dns1);
		}
		else if(TYPE_WIRED_DHCP == _network_config->net_connect_type)
		{
			dbg_log_print(LOG_INFO,"saved TYPE_WIRED_DHCP  ");
		}
		else if(TYPE_WIRED_PPPOE== _network_config->net_connect_type)
		{
			write_conf_value("str_account", _network_config->str_account);
			write_conf_value("str_password", _network_config->str_password);
			dbg_log_print(LOG_INFO,"saved TYPE_WIRED_PPPOE  ");
		}
		else if(TYPE_WIRELESS_DHCP== _network_config->net_connect_type)
		{
			memset(tmp_buf, 0, 12);
			sprintf(tmp_buf, "%d", _network_config->wifi_config_info.encryption);
			write_conf_value("wifi_config_info.encryption", tmp_buf);
			memset(tmp_buf, 0, 12);
			sprintf(tmp_buf, "%d", _network_config->wifi_config_info.hide);
			write_conf_value("wifi_config_info.hide", tmp_buf);
			write_conf_value("wifi_config_info.ssid", _network_config->wifi_config_info.ssid);
			write_conf_value("wifi_config_info.key", _network_config->wifi_config_info.key);
			dbg_log_print(LOG_INFO,"saved TYPE_WIRELESS_DHCP  ");
			
		}
		else if(TYPE_WIRELESS_FIXEDIP== _network_config->net_connect_type)
		{
			memset(tmp_buf, 0, 12);
			sprintf(tmp_buf, "%d", _network_config->wifi_config_info.encryption);
			write_conf_value("wifi_config_info.encryption", tmp_buf);
			memset(tmp_buf, 0, 12);
			sprintf(tmp_buf, "%d", _network_config->wifi_config_info.hide);
			
			dbg_log_print(LOG_INFO,"saved TYPE_WIRELESS_FIXEDIP  ");
			write_conf_value("wifi_config_info.hide", tmp_buf);
			write_conf_value("wifi_config_info.ssid", _network_config->wifi_config_info.ssid);
			write_conf_value("wifi_config_info.key", _network_config->wifi_config_info.key);
			write_conf_value("wireless_ip_config.ipaddr", _network_config->wireless_ip_config.ipaddr);
			write_conf_value("wireless_ip_config.gatway", _network_config->wireless_ip_config.gatway);
			write_conf_value("wireless_ip_config.submask", _network_config->wireless_ip_config.submask);
			write_conf_value("wireless_ip_config.dns1", _network_config->wireless_ip_config.dns1);
			dbg_log_print(LOG_INFO,"saved TYPE_WIRELESS_FIXEDIP  ");
		}
}

