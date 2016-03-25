#ifndef _SD_DEV_H_
#define _SD_DEV_H_
#include <hld/hld_dev.h>

#define CARD_REMOVE 0
#define CARD_INSERT	1

struct sd_device{

	struct sd_device *next;
	INT32 type;
	INT8 name[HLD_MAX_NAME_SIZE];
	INT32 flags;

	INT32 hardware;
	INT32 busy;
	INT32 minor;

	void *priv;	
	UINT32 bass_addr;	
	UINT32 sd_card_connect;

};
#endif/*_SD_DEV_H_*/
