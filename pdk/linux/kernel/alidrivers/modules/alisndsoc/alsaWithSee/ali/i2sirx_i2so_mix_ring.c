#include <linux/kernel.h>
#include <linux/slab.h>

static __u8 *ring_base;
static __u32 ring_wr;
static __u32 ring_len;


__s32 i2sirx_i2so_mix_ring_init
(
    __u8 *base,
    __u32 len//unit: bytes
)
{
    ring_base = base;        
    ring_len = len;
	ring_wr = 0;

	return(0);
}



#if 0
__u32 i2sirx_i2so_mix_ring_write
(
    __u8 *data,
    __u32 len
)
{
    __u32 remain_len;
    __u32 avail_len;
    __u32 cpy_len;
    __u32 real_wr;
	
    remain_len = len;

	while (remain_len > 0)
	{
		real_wr = (ring_wr % ring_len);

	    #if 0
        printk("%s,%d,ring_wr:%d,ring_len:%d,wr remain_len:%d\n", __FUNCTION__, 
			__LINE__, ring_wr, ring_len, remain_len);
		#endif
		
        avail_len = ring_len - real_wr;
    		
    	if (avail_len > remain_len)
    	{
            cpy_len = remain_len;
    	}
    	else
    	{
            cpy_len = avail_len;
    	}
        
        memcpy(ring_base + real_wr, data, cpy_len);

		#if 1
        if (0 != memcmp(data, ring_base + real_wr, cpy_len))
        {
            printk("%s,%d,ring_wr:%d,ring_len:%d,real_wr:%d,wr remain_len:%d\n", __FUNCTION__, 
    			__LINE__, ring_wr, ring_len, real_wr, remain_len);
		}
		#endif
        
    	ring_wr += cpy_len;
    
        remain_len -= cpy_len;
	}
	
	return(len);
}
#else
__u32 i2sirx_i2so_mix_ring_write
(
    __u8 *data,
    __u32 len
)
{
#if 0
    __u32 remain_len;
    __u32 avail_len;
    __u32 cpy_len;
	
    remain_len = len;

	while (remain_len > 0)
	{
	    #if 0
        printk("%s,%d,ring_wr:%d,ring_len:%d,wr remain_len:%d\n", __FUNCTION__, 
			__LINE__, ring_wr, ring_len, remain_len);
		#endif
		
        avail_len = ring_len - ring_wr;
    		
    	if (avail_len > remain_len)
    	{
            cpy_len = remain_len;
    	}
    	else
    	{
            cpy_len = avail_len;
    	}
        
        memcpy(ring_base + ring_wr, data, cpy_len);

		#if 1
        if (0 != memcmp(data, ring_base + ring_wr, cpy_len))
        {
            printk("%s,%d,ring_wr:%d,ring_len:%d,wr remain_len:%d\n", __FUNCTION__, 
    			__LINE__, ring_wr, ring_len, remain_len);
		}
		#endif
        
    	ring_wr += cpy_len;
    
    	if (ring_wr >= ring_len)
    	{
            ring_wr = 0;
    	}
    
        remain_len -= cpy_len;
	}
	
	return(len);
#else

	memcpy(ring_base + ring_wr, data, len);

    ring_wr += len;

	if (ring_wr >= ring_len)
	{
        ring_wr = 0;
	}

	return(len);
#endif
}
#endif

__u32 i2sirx_i2so_mix_ring_wr_get
(
    void
)
{
    return(ring_wr);
}


__u8 *i2sirx_i2so_mix_ring_base_get
(
    void
)
{
    return(ring_base);
}


