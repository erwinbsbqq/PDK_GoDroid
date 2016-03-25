#ifndef _I2SIRX_SEE2MAIN_RING_H_
#define _I2SIRX_SEE2MAIN_RING_H_

#include <linux/kernel.h>
#include <linux/slab.h>

struct i2sirx_see2main_ring_info
{
    /* Should only modified by SEE.
	*/
	__u32 wr;
	
    /* Should only modified by main.
	*/
	__u32 rd;

    /* Should only inited by main.
	*/
	__u32 len;

    /* Main side virtual address of data storage address, should point to the
     * same physical address as buf_see. Inited by main.
	 */
	__u8 *buf_main;

    /* SEE side virtual address of data storage address, should point to the
     * same physical address as buf_main. Inited by main.
	 */
	__u8 *buf_see;

    /* 0, stop; 1, run.
	*/
	__u32 status;	
};

__s32 i2sirx_see2main_ring_init(__u8 *buf_base, __u32 buf_len);
__u32 i2sirx_see2main_ring_read(__u8 * data, __u32 len);
__u32 i2sirx_see2main_ring_avail(void);

#endif

