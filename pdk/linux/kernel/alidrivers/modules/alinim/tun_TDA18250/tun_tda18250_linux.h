#ifndef __LLD_TUN_TDA18250_LINUX_H__
#define __LLD_TUN_TDA18250_LINUX_H__


#include "../porting_linux_header.h"


#include "tmNxTypes.h"
#include "tmCompId.h"
#include "tmFrontEnd.h"
#include "tmbslFrontEndTypes.h"
#include "tmddTDA18250.h"
#include "tmbslTDA18250.h"

#include "tun_tda18250.h"



#ifdef __cplusplus
extern "C"
{
#endif







#define nim_print  					printk

#define comm_malloc(x)				kmalloc((x),GFP_KERNEL)//kmalloc((x), GFP_ATOMIC)
#define comm_memset 				memset
#define comm_free 					kfree
#define comm_delay					udelay
#define comm_sleep 					msleep
#define comm_memcpy 				memcpy

#define nim_i2c_read				ali_i2c_read
#define nim_i2c_write				ali_i2c_write
#define nim_i2c_write_read			ali_i2c_write_read


#define NIM_MUTEX_ENTER(priv)  \
	do \
	{ \
		mutex_lock(&priv->i2c_mutex); \
	}while(0)

#define NIM_MUTEX_LEAVE(priv) \
	do\
	{ \
		mutex_unlock(&priv->i2c_mutex);\
	}while(0)











#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_TDA18250_H__ */


