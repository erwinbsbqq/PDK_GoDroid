#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <linux/dma-mapping.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <linux/ali_rpc.h>
#include "sndrpc.h"
#include "i2sirx_see2main_ring.h"


static struct i2sirx_see2main_ring_info *ring;


static __s32 i2sirx_see2main_ring_init_see
(
    void *ring_info
)
{
	__s32 ret = 0;  
	__u32 see_addr;

	see_addr = MAIN_ADDR_TO_SEE_ADDR(ring_info);

	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &see_addr);   
		
	ret = RpcCallCompletion(RPC_i2sirx_see2main_ring_init_see, &p1, NULL);

	return(ret);    
}


__s32 i2sirx_see2main_ring_init
(
    __u8 *buf_base,
    __u32 buf_len//unit: bytes
)
{
    /* Use start of ring_base as i2so_see2main_ring_info, 
     * Since ring_base MUST point to memory which could be accessed by both
     * main & SEE, hence i2so_see2main_ring_info could be shared by main & SEE,
     * thus main & SEE could know each other's state.
	 */
    ring = (struct i2sirx_see2main_ring_info *)buf_base;

	memset(ring, 0, sizeof(struct i2sirx_see2main_ring_info));

	ring->len = buf_len - sizeof(struct i2sirx_see2main_ring_info);

	printk("%s,%d,ring_info->len:%d\n", __FUNCTION__, __LINE__, ring->len);
		
    /*  
     * round up to the next power of 2, since our 'let the indices  
     * wrap' tachnique works only in this case.  
     */   
    if (ring->len & (ring->len - 1)) 
	{   
        BUG_ON(ring->len > 0x80000000);   
        ring->len = rounddown_pow_of_two(ring->len);   

		printk("%s,%d,ring_info->len:%d\n", __FUNCTION__, __LINE__, ring->len);
    }  	

	ring->buf_main = buf_base + sizeof(struct i2sirx_see2main_ring_info);

    ring->buf_see = MAIN_ADDR_TO_SEE_ADDR(ring->buf_main);

	i2sirx_see2main_ring_init_see(ring);
	
	return(0);
}


__s32 i2sirx_see2main_ring_onoff
(
    __u32 onoff//0,off;1,on;
)
{

	__s32 ret = 0;  
#if 0
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, sizeof(INT32), &onoff);   
		
	ret = RpcCallCompletion(RPC_i2sirx_see2main_ring_onoff_see, &p1, NULL);
#else
    ring->status = onoff;
#endif	

	return(ret);
}




__u32 i2sirx_see2main_ring_read
(
    __u8 *buf, 
    __u32 size
)
{
    __u32 len = 0;


    /* Data avail length.
    */	
    size = min(size, ring->wr - ring->rd);   
	
    /* first get the data from fifo->out until the end of the buffer
    */
    len = min(size, ring->len - (ring->rd & (ring->len - 1)));

	//printk("wr:%d,rd:%d,buf:%x,len:%d,buf_main:%d\n", ring->wr, ring->rd, buf, ring->buf_main);
	
    memcpy(buf, ring->buf_main + (ring->rd & (ring->len - 1)), len);
	
    /* then get the rest (if any) from the beginning of the buffer 
	*/
	//printk("wr:%d,rd:%d,buf:%x,len:%d,buf_main:%d\n", ring->wr, ring->rd, buf, ring->buf_main);
    memcpy(buf + len, ring->buf_main, size - len);
	
    ring->rd += size;

	//printk("wr:%d,rd:%d,buf:%x,len:%d,buf_main:%d\n", ring->wr, ring->rd, buf, ring->buf_main);
	
    return(size);
}


__u32 i2sirx_see2main_ring_avail
(
    void
)
{
	return(ring->wr - ring->rd);	
}


