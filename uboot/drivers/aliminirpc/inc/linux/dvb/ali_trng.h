#ifndef  _TRNG_H_
#define  _TRNG_H_
#include <linux/types.h>
#include <ali_magic.h>

#define ALI_TRNG_HLD_PARAM_MAX_SIZE 4
struct ali_trng_hld_param
{
    __u32 p[ALI_TRNG_HLD_PARAM_MAX_SIZE];    
};

#define ALI_TRNG_BASE_KERNEL		(ALI_TRNG_MAGIC<<8)

#define ALI_TRNG_GENERATE_BYTE      (ALI_TRNG_BASE_KERNEL + 0)
#define ALI_TRNG_GENERATE_64bits    (ALI_TRNG_BASE_KERNEL + 1)
#define ALI_TRNG_GET_64bits         	(ALI_TRNG_BASE_KERNEL + 2)

#endif  /*_TRNG_H_*/