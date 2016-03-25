
#include"netinterface.h"
typedef struct item_t {
    char *key;
    char *value;
}ITEM;

int read_conf_value(const char *key, char *value);
int write_conf_value(const char *key,char *value);
int file_to_items(ITEM *items, int *num);
int get_configinfo(network_config * _network_config);
void save_config_info(network_config * _network_config);


#define NET_CONFIG_FILE "/tmp/Settings/NMP/network.conf"
