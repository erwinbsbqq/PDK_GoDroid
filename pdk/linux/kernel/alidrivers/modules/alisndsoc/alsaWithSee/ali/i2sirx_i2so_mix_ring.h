#ifndef _I2SIRX_I2SO_MIX_RING_H_
#define _I2SIRX_I2SO_MIX_RING_H_

#include <linux/kernel.h>
#include <linux/slab.h>

__s32 i2sirx_i2so_mix_ring_init(__u8 *base, __u32 len);
__u32 i2sirx_i2so_mix_ring_write(__u8 *data, __u32 len);
__u32 i2sirx_i2so_mix_ring_wr_get(void);
__u8 *i2sirx_i2so_mix_ring_base_get(void);
__s32 i2sirx_see2main_ring_onoff(__u32 onoff);

#endif