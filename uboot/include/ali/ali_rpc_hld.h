#ifndef __ALI_RPC_HLD_H
#define __ALI_RPC_HLD_H

#include <ali/basic_types.h>


/*
  * this struct is used to save device IO parameter
  */

void ali_rpc_hld_base_callee(UINT8 *msg);
INT32 hld_dev_add_remote(struct hld_device *dev, UINT32 dev_addr);
INT32 hld_dev_remove_remote(struct hld_device *dev);
void hld_dev_memcpy(void *dest, const void *src, unsigned int len);

#endif
