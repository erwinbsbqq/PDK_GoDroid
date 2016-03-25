
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/features.h>

#include <linux/vmalloc.h>


#include <asm/io.h>


#include "dmx_internal.h"

#define ALI_DMX_DATA_PKT_DEBUG(...) //printk

#define DMX_DATA_PKT_VER_2 1


#define VIRTUAL_QUANTUM_SIZE 256*188
extern void *fcp_get_quantum(int,int);
extern void fcp_put_quantum(int);

struct ali_dmx_data_node* dmx_data_node_get
(
    struct dmx_device *dmx
)
{
    struct ali_dmx_data_node *node;
    struct list_head         *node_pool;

    node_pool = &(dmx->data_node_pool);

    spin_lock(&dmx->data_node_spinlock);

    if (list_empty(node_pool))
    {
        spin_unlock(&dmx->data_node_spinlock);
        //printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(NULL);
    }

    node = list_first_entry(node_pool, struct ali_dmx_data_node, link);

    //node->len = DMX_DATA_POOL_NODE_SIZE - sizeof(struct ali_dmx_data_node);
    //node->len = DMX_DATA_POOL_NODE_SIZE;

    node->rd = sizeof(struct ali_dmx_data_node);

    node->wr = sizeof(struct ali_dmx_data_node);

    list_del(&node->link);

    spin_unlock(&dmx->data_node_spinlock);

    return(node);
}


struct ali_dmx_data_node* dmx_data_node_put
(
    struct dmx_device        *dmx,
    struct ali_dmx_data_node *data
)
{
    struct list_head *node_pool;
    struct list_head *data_link;

    data_link = &(data->link);

    spin_lock(&dmx->data_node_spinlock);

    node_pool = &(dmx->data_node_pool);

    list_add_tail(data_link, node_pool);

    spin_unlock(&dmx->data_node_spinlock);

    return(data);
}




struct ali_dmx_data_pkt *dmx_data_pkt_get
(
    struct dmx_device *dmx
)
{
    struct ali_dmx_data_pkt *pkt;
    struct list_head        *pkt_pool;

    pkt_pool = &dmx->data_pkt_pool;

    spin_lock(&dmx->data_pkt_spinlock);

    if (list_empty(pkt_pool))
    {
        spin_unlock(&dmx->data_pkt_spinlock);
        //printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(NULL);
    }

    pkt = list_first_entry(pkt_pool, struct ali_dmx_data_pkt, link);

    list_del(&pkt->link);

    pkt->status = ALI_DMX_PKT_STATUS_LOADING;

    pkt->len = 0;

    INIT_LIST_HEAD(&(pkt->data_node));

    spin_unlock(&dmx->data_pkt_spinlock);

    return(pkt);
}



struct ali_dmx_data_pkt* dmx_data_pkt_put
(
    struct dmx_device       *dmx,
    struct ali_dmx_data_pkt *pkt

)
{
    struct list_head *pkt_pool;
    struct list_head *pkt_link;

    spin_lock(&dmx->data_pkt_spinlock);

    pkt_link = &(pkt->link);

    pkt_pool = &(dmx->data_pkt_pool);

    list_add_tail(pkt_link, pkt_pool);

    spin_unlock(&dmx->data_pkt_spinlock);

    return(pkt);
}


__s32 dmx_data_pool_release
(
    struct dmx_device *dmx
)
{
    __u32 node_len;
    __u32 node_buf_len;
    __u32 node_buf_order;
    __u32 pkt_len;
    __u32 pkt_buf_len;
    __u32 pkt_buf_order;

    node_len = DMX_DATA_POOL_NODE_SIZE + sizeof(struct ali_dmx_data_node);

    node_buf_len  = DMX_DATA_POOL_NODE_CNT * node_len; 

    node_buf_order = get_order(node_buf_len);

    if (NULL != dmx->data_node_raw)
    {
        free_pages((__u32)dmx->data_node_raw, node_buf_order);
    }


    pkt_len = sizeof(struct ali_dmx_data_pkt);

    pkt_buf_len = DMX_DATA_POOL_NODE_CNT * pkt_len;

    pkt_buf_order = get_order(pkt_buf_len);

    if (NULL != dmx->data_pkt_raw)
    {
        free_pages((__u32)dmx->data_pkt_raw, pkt_buf_order);
    }

    return(0);
}



__s32 dmx_data_pool_init
(
    struct dmx_device *dmx,
    __u32              data_node_size,
    __u32              data_node_cnt
)
{
    __u32                     i;
    __u32                     node_len;
    __u32                     node_buf_len;
    __u32                     node_buf_order;
    __u32                     pkt_len;
    __u32                     pkt_buf_len;
    __u32                     pkt_buf_order;
    struct ali_dmx_data_node *node;
    struct ali_dmx_data_pkt  *pkt;
    __u32                     data_pkt_cnt;


    node_len = data_node_size + sizeof(struct ali_dmx_data_node);

    node_buf_len  = data_node_cnt * node_len; 

    node_buf_order = get_order(node_buf_len);

    dmx->data_node_raw = (void *)__get_free_pages(GFP_KERNEL, node_buf_order);

	if (!(dmx->data_node_raw))
    {
        ALI_DMX_DATA_PKT_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

		return(-ENOMEM);
    }

    /* data_pkt_cnt == data_node_cnt to gurantee one-to-one relationship in 
     * worst case.
     */
    data_pkt_cnt = data_node_cnt;

    pkt_len = sizeof(struct ali_dmx_data_pkt);

    pkt_buf_len = data_pkt_cnt * pkt_len;

    pkt_buf_order = get_order(pkt_buf_len);

    dmx->data_pkt_raw = (void *)__get_free_pages(GFP_KERNEL, pkt_buf_order);

	if (!(dmx->data_node_raw))
    {
        ALI_DMX_DATA_PKT_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        free_pages((__u32)dmx->data_node_raw, node_buf_order);

		return(-ENOMEM);
    }

    ALI_DMX_DATA_PKT_DEBUG("data_node_raw:0x%x~0x%x\n", 
                           (__u32)dmx->data_node_raw,
                           (__u32)(dmx->data_node_raw + node_buf_len));

    ALI_DMX_DATA_PKT_DEBUG("data_pkt_raw :0x%x~0x%x\n", 
                           (__u32)dmx->data_pkt_raw, 
                           (__u32)(dmx->data_pkt_raw +  pkt_buf_len));

    INIT_LIST_HEAD(&(dmx->data_node_pool));

    INIT_LIST_HEAD(&(dmx->data_pkt_pool));

    /* Init memory pools. */
    for (i = 0; i < data_node_cnt; i++)
    {
        node = (struct ali_dmx_data_node *)(dmx->data_node_raw + i * node_len);

        node->len = node_len;

        node->rd = 0;

        node->wr = 0;

        dmx_data_node_put(dmx, node);
    }

    for (i = 0; i < data_pkt_cnt; i++)
    {
        pkt = (struct ali_dmx_data_pkt *)(dmx->data_pkt_raw + i * pkt_len);

        pkt->len = 0;

        dmx_data_pkt_put(dmx, pkt);
    }

    spin_lock_init(&dmx->data_node_spinlock);

    spin_lock_init(&dmx->data_pkt_spinlock);

    return(0);
}






__s32 dmx_data_buf_rd
(
    struct dmx_device          *dmx,
    void                       *dest,
    struct ali_dmx_data_buf    *src_buf,
    __s32                       rd_len,
    enum ali_dmx_data_buf_dest  data_pkt_dest
)
{
    struct ali_dmx_data_pkt  *cur_data_pkt;
    struct ali_dmx_data_pkt  *next_data_pkt;
    struct ali_dmx_data_node *cur_data_node;
    struct ali_dmx_data_node *next_data_node;
    __s32                     copied_rd_len;
    __s32                     remain_rd_len;
    __s32                     node_len;
    void                     *cpy_src;
    __u32                     cpy_len;
    char __user              *usr_dest;

    void *qbuf = (void *)dest;
    int  quantum_id = (int)(((__u32)dest>>16) & 0xffff);
    int  offset = (int)((__u32)dest & 0xffff);
    if(feature_is_fastcopy_enable() && \
	   src_buf->fst_cpy_slot >= 0)
        qbuf = fcp_get_quantum(quantum_id, 0) + offset; 
    if (src_buf->cur_len <= 0)
    {
        ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        return(0);
    }

    /* copied_rd_len + remain_rd_len == rd_len */
    copied_rd_len = 0;

    remain_rd_len = rd_len;

    usr_dest = (char __user*)dest;
    
    list_for_each_entry_safe(cur_data_pkt, next_data_pkt, &(src_buf->data_pkt),
                             link)
    {
#if 0
        if (ALI_DMX_PKT_STATUS_LOADED != cur_data_pkt->status)
        {
            return(copied_rd_len); 
        }
#endif

        list_for_each_entry_safe(cur_data_node, next_data_node,
                                 &(cur_data_pkt->data_node), link)
        {
            node_len = cur_data_node->wr - cur_data_node->rd;
    
            cpy_src = ((void *)cur_data_node) + cur_data_node->rd;

            if (remain_rd_len >= node_len)
            {
                cpy_len = node_len;
            }
            else
            {
                cpy_len = remain_rd_len;
            }

            if (ALI_DMX_DATA_BUF_DEST_USER == data_pkt_dest)
            {
				if (feature_is_fastcopy_enable())
				{
	                if(src_buf->fst_cpy_slot >= 0)
		            {
			            memcpy(qbuf+src_buf->fst_cpy_size, cpy_src, cpy_len);        
	              	}else
		          	{
			            if (copy_to_user(usr_dest + copied_rd_len, cpy_src, cpy_len))
				        {
					        ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
       
						    return(-EFAULT);     
	                    }
		            }
				} else {
					if (copy_to_user(usr_dest + copied_rd_len, cpy_src, cpy_len))
	                {
		                ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
		
			            return(-EFAULT);     
				    }
				}
#if 0
                /* Find start code. */
                if (1 == src_buf->find_mpeg2_startcode)
                {
                    A_SgdmaChCpy(src_buf->sgdma_ch_idx, 
                                 src_buf->sgdma_tmp_buf + copied_rd_len,
                                 cpy_src, cpy_len);

                    A_SgdmaChFlush(src_buf->sgdma_ch_idx, ALI_SGDMA_FLUSH_MODE_SPINWAIT);

                    A_SgdmaChGetStartcode(src_buf->sgdma_ch_idx);
                } 
#endif
            }
            else
            {
                memcpy(dest + copied_rd_len, cpy_src, cpy_len);
            }

            if (remain_rd_len >= node_len)
            {
                /* Remove current data node from data node list. */
                list_del(&(cur_data_node->link));
    
                dmx_data_node_put(dmx, cur_data_node);
            }

            copied_rd_len += cpy_len;

            remain_rd_len -= cpy_len; 

            cur_data_node->rd += cpy_len;
    
            cur_data_pkt->len -= cpy_len;
    
            src_buf->cur_len -= cpy_len;

			if (feature_is_fastcopy_enable())
			{
	            if(src_buf->fst_cpy_slot >= 0)
		        {
			        src_buf->fst_cpy_size += cpy_len;
				    if(src_buf->fst_cpy_size >= VIRTUAL_QUANTUM_SIZE)
					{
						if(src_buf->fst_cpy_size != VIRTUAL_QUANTUM_SIZE)
							printk("dmx fast copy more data than 47K\n");
	                    fcp_put_quantum(quantum_id);
		                src_buf->fst_cpy_size = 0;
			        }
				}
			}
            /* Case 0: All data required by user has been satisfied. */
	        if (remain_rd_len <= 0)
		    {
			    if (list_empty(&(cur_data_pkt->data_node)))
				{
                    /* Remove current data packet from data packet list. */
                    list_del(&(cur_data_pkt->link));  
            
                    dmx_data_pkt_put(dmx, cur_data_pkt);
                }

                return(copied_rd_len);  
            }
        }

        /* Remove current data packet from data packet list. */
        list_del(&(cur_data_pkt->link));  

        dmx_data_pkt_put(dmx, cur_data_pkt);
    }

    /* Case 1: All data in current buffer is less than user required. */
    return(copied_rd_len); 
}






__s32 dmx_data_buf_wr_data
(
    struct dmx_device         *dmx,
    struct ali_dmx_data_buf   *dest_buf,
    __u8                      *src,
    __s32                      wr_len,
    enum ali_dmx_data_buf_src  data_pkt_src
)
{
    struct ali_dmx_data_pkt  *data_pkt;
    struct ali_dmx_data_node *data_node;
    __s32                     remain_wr_len;
    __s32                     copied_wr_len;
    __s32                     data_node_len;
    __s32                     cpy_len;
    char __user              *usr_src;

    void *qbuf = (void *)src;
    int  quantum_id = (int)(((__u32)src>>16) & 0xffff);
    int  offset = (int)((__u32)src & 0xffff);
    if (feature_is_fastcopy_enable() && \
		ALI_DMX_DATA_BUF_SRC_FAST == data_pkt_src)
    {
        qbuf = fcp_get_quantum(quantum_id, 1) + offset; 
		
    }     
    //ALI_DMX_DATA_PKT_DEBUG("%s,%d,l:%d\n", __FUNCTION__, __LINE__, wr_len);

    /* If empty buffer. */
    if (list_empty(&(dest_buf->data_pkt)))
    {
        data_pkt = dmx_data_pkt_get(dmx);

        if (NULL == data_pkt)
        {
            ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    
            return(-ENOMEM);
        }

        list_add_tail(&data_pkt->link, &dest_buf->data_pkt);
    }
    else
    {
        data_pkt = list_last_entry(&(dest_buf->data_pkt), 
                                   struct ali_dmx_data_pkt, link); 

        if (NULL == data_pkt)
        {
            ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    
            return(-ENOMEM);
        }   
    }

    /* Store in new pakcet if privous packet has been fihished. */
    if (ALI_DMX_PKT_STATUS_LOADED == data_pkt->status)
    {
        data_pkt = dmx_data_pkt_get(dmx);

        if (NULL == data_pkt)
        {
            ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    
            return(-ENOMEM);
        }

        list_add_tail(&data_pkt->link, &dest_buf->data_pkt);
    }

    //ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    /* If empty pakcet. */
    if (list_empty(&data_pkt->data_node))
    {
        data_node = dmx_data_node_get(dmx);

        if (NULL == data_node)
        {
            ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    
            return(-ENOMEM);
        }

        list_add_tail(&data_node->link, &data_pkt->data_node);
    }
    else
    {
        data_node = list_last_entry(&(data_pkt->data_node), 
                                    struct ali_dmx_data_node, link); 
    }

    if (0 == (data_node->len - data_node->wr))
    {
        data_node = dmx_data_node_get(dmx);

        if (NULL == data_node)
        {
            //ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    
            return(-ENOMEM);
        }

        list_add_tail(&data_node->link, &data_pkt->data_node);
    }

    remain_wr_len = wr_len;

    copied_wr_len = 0;

    usr_src = (char __user *)src;

    while(remain_wr_len > 0)
    {
        data_node_len = data_node->len - data_node->wr;

        if (data_node_len >= remain_wr_len)
        {
            cpy_len = remain_wr_len;
        }
        else
        {
            cpy_len = data_node_len;
        }

        if (ALI_DMX_DATA_BUF_SRC_KERN == data_pkt_src)
        {
            memcpy(((void *)data_node) + data_node->wr,
                   src + copied_wr_len, cpy_len);
        }
        else
        {   
			if (feature_is_fastcopy_enable())
			{
	            if (ALI_DMX_DATA_BUF_SRC_FAST == data_pkt_src)
		        {
#if 1	
			        if(dest_buf->fst_cpy_size + cpy_len > VIRTUAL_QUANTUM_SIZE)
				       cpy_len = VIRTUAL_QUANTUM_SIZE - dest_buf->fst_cpy_size; 
					memcpy(((void *)data_node) + data_node->wr,
	                   qbuf + dest_buf->fst_cpy_size, cpy_len);
		            ALI_DMX_DATA_PKT_DEBUG("copy %d\n", cpy_len);
			         dest_buf->fst_cpy_size += cpy_len;
					 dest_buf->exp_cpy_size -= cpy_len;
					if(dest_buf->exp_cpy_size <= 0)
	                {
		                if(dest_buf->exp_cpy_size != 0)
			                printk("dmx fast read more data than expected\n");
				        fcp_put_quantum(quantum_id);
					    dest_buf->fst_cpy_size = 0;
					
	                }
#else
		            memcpy(((void *)data_node) + data_node->wr,
			           qbuf + copied_wr_len, cpy_len);
#endif
            
				}else
				{
	                copy_from_user(((void *)data_node) + data_node->wr,    
		                       usr_src + copied_wr_len, cpy_len);
			    }
			} else {
	            copy_from_user(((void *)data_node) + data_node->wr,    
		                       usr_src + copied_wr_len, cpy_len);
			}
        }

        data_node->wr += cpy_len;

        data_pkt->len += cpy_len;

        dest_buf->cur_len += cpy_len;

        remain_wr_len -= cpy_len;

        copied_wr_len += cpy_len;

        //ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        if (remain_wr_len > 0)
        {
            data_node = dmx_data_node_get(dmx);
    
            if (NULL == data_node)
            {
                //ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
        
                return(copied_wr_len);
            }
            
            list_add_tail(&data_node->link, &data_pkt->data_node);
        }

        //ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    }

    return(copied_wr_len);
}



__s32 dmx_data_buf_wr_pkt_end
(
    struct dmx_device       *dmx,
    struct ali_dmx_data_buf *data_buf
)
{
    struct ali_dmx_data_pkt  *data_pkt;

    if (list_empty(&(data_buf->data_pkt)))
    {
        ALI_DMX_DATA_PKT_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }

    data_pkt = list_last_entry(&(data_buf->data_pkt), 
                                struct ali_dmx_data_pkt, link);    

    if (NULL == data_pkt)
    {
        ALI_DMX_DATA_PKT_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }

    data_pkt->status = ALI_DMX_PKT_STATUS_LOADED;

    return(0);
}



struct ali_dmx_data_pkt* dmx_data_buf_force_unlink_first_pkt
(
    struct dmx_device       *dmx,
    struct ali_dmx_data_buf *src_buf
)
{
    struct ali_dmx_data_pkt *data_pkt;

    if (list_empty(&(src_buf->data_pkt)))
    {
        return(NULL);
    }

    data_pkt = list_first_entry(&(src_buf->data_pkt), struct ali_dmx_data_pkt, 
                                link);

    if (list_empty(&(data_pkt->data_node)))
    {
        return(NULL);
    }

    list_del(&(data_pkt->link));

    src_buf->cur_len -= data_pkt->len;

    dmx_data_pkt_put(dmx, data_pkt);

    return(data_pkt);
}



__s32 dmx_data_buf_flush_last_pkt
(
    struct dmx_device       *dmx,
    struct ali_dmx_data_buf *data_buf
)
{
    struct ali_dmx_data_pkt  *data_pkt;
    struct ali_dmx_data_node *cur_data_node;
    struct ali_dmx_data_node *next_data_node;

    if (list_empty(&(data_buf->data_pkt)))
    {
        return(0);
    }

    data_pkt = list_last_entry(&(data_buf->data_pkt), struct ali_dmx_data_pkt, 
                               link);

    list_for_each_entry_safe(cur_data_node, next_data_node,
                             &(data_pkt->data_node), link)
    {
        /* Remove current data node from data node list. */
        list_del(&(cur_data_node->link));
    
        dmx_data_node_put(dmx, cur_data_node);
    }
    
    data_buf->cur_len -= data_pkt->len;
    
    /* Remove current data packet from data packet list. */
    list_del(&(data_pkt->link));  
    
    dmx_data_pkt_put(dmx, data_pkt);

    //ALI_DMX_DATA_PKT_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    return(0);
}




__s32 dmx_data_buf_flush_all_pkt
(
    struct dmx_device       *dmx,
    struct ali_dmx_data_buf *data_buf
)
{
    struct ali_dmx_data_pkt  *cur_data_pkt;
    struct ali_dmx_data_pkt  *next_data_pkt;
    struct ali_dmx_data_node *cur_data_node;
    struct ali_dmx_data_node *next_data_node;

    if (list_empty(&(data_buf->data_pkt)))
    {
        return(0);
    }

    list_for_each_entry_safe(cur_data_pkt, next_data_pkt, &(data_buf->data_pkt),
                             link)
    {
        list_for_each_entry_safe(cur_data_node, next_data_node,
                                 &(cur_data_pkt->data_node), link)
        {
            /* Remove current data node from data node list. */
            list_del(&(cur_data_node->link));
        
            dmx_data_node_put(dmx, cur_data_node);
        }

        /* Remove current data packet from data packet list. */
        list_del(&(cur_data_pkt->link));  
        
        dmx_data_pkt_put(dmx, cur_data_pkt);
    }

    data_buf->cur_len = 0;

    return(0);
}






__s32 dmx_data_buf_first_pkt_len
(
    struct dmx_device       *dmx,
    struct ali_dmx_data_buf *data_buf
)
{
    struct ali_dmx_data_pkt *data_pkt;

    if (list_empty(&(data_buf->data_pkt)))
    {
        return(0);
    }

    data_pkt = list_first_entry(&(data_buf->data_pkt), struct ali_dmx_data_pkt, 
                                link);

    if (ALI_DMX_PKT_STATUS_LOADING == data_pkt->status)
    {
        return(0);
    }

    return(data_pkt->len);
}


__s32 dmx_data_buf_len
(
    struct dmx_device       *dmx,
    struct ali_dmx_data_buf *data_buf
)
{
    if (list_empty(&(data_buf->data_pkt)))
    {
        return(0);
    }

    return(data_buf->cur_len);
}


__s32 dmx_data_buf_setup
(
    struct dmx_device       *dmx,
    struct ali_dmx_data_buf *data_buf,
    __u32                    buf_max_len,
    __u32                    buf_node_len
)
{
    data_buf->cur_len = 0;
    
    data_buf->max_len = buf_max_len;
    
    data_buf->data_node_size = buf_node_len;
    
    mutex_init(&(data_buf->rw_mutex));
    
    init_waitqueue_head(&(data_buf->rd_wq));
    
    init_waitqueue_head(&(data_buf->wr_wq));
    
    INIT_LIST_HEAD(&(data_buf->data_pkt));

    data_buf->fst_cpy_slot = -1;
    data_buf->fst_cpy_size = 0;
    return(0);
}


__s32 dmx_data_buf_usr_wr_data
(
    struct dmx_device       *dmx,
    struct file             *file, 
    struct ali_dmx_data_buf *dest_buf,
    const char __user       *src,
    __s32                    wr_len,
    __s32                    fast
)
{
    __s32 total_wr_cnt;
    __s32 wr_cnt;

    if (0 == wr_len)
    {
        return(0);
    }

    total_wr_cnt = 0;

    for(;;)
    {
        wr_cnt = 0;

        /* One usr each write. */
    	if (mutex_lock_interruptible(&(dest_buf->rw_mutex)))
        {
            ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    
            return(-ERESTARTSYS);
        }

        /* Too much data in buffer. */
        if (dest_buf->cur_len + wr_len > dest_buf->max_len)
        {
            if (file->f_flags & O_NONBLOCK)
            {
                mutex_unlock(&(dest_buf->rw_mutex));

                return(-EAGAIN);
            }

            //ALI_DMX_DATA_PKT_DEBUG("%s,%d,%d,%d,%d\n", __FUNCTION__, __LINE__,
                                   //dest_buf->cur_len, wr_len, dest_buf->max_len);

        }
        else if(dmx->isRadio_playback == 1 && dest_buf->cur_len>20*188)
        {
            mutex_unlock(&(dest_buf->rw_mutex));
            
            return(-EAGAIN);
        }
        else
        { 
            if(fast)
            {
                wr_cnt = dmx_data_buf_wr_data(dmx, dest_buf,
                                             (void *)(src + total_wr_cnt),
                                             wr_len - total_wr_cnt, 
                                             ALI_DMX_DATA_BUF_SRC_FAST);
            }else
            {
                wr_cnt = dmx_data_buf_wr_data(dmx, dest_buf,
                                             (void *)(src + total_wr_cnt),
                                             wr_len - total_wr_cnt, 
                                             ALI_DMX_DATA_BUF_SRC_USER);
            }
        }

        mutex_unlock(&(dest_buf->rw_mutex));

        if (wr_cnt < 0)
        { 
            if (file->f_flags & O_NONBLOCK)
            {
                return(-EAGAIN);
            }
        }
        else
        {
            total_wr_cnt += wr_cnt;
        }

        //wake_up_interruptible(&(dest_buf->rd_wq));

        if (total_wr_cnt >= wr_len)
        {
            break;
        }

		if(dmx_see_paused())
		{
            ALI_DMX_DATA_PKT_DEBUG("wr_len=%d total_wr_cnt=%d wr_cnt=%x\r\n",wr_len,total_wr_cnt,wr_cnt);
            break;
        }
        msleep(10);
    }

    return(total_wr_cnt);
}



int dmx_data_buf_usr_rd_data
(
    struct dmx_device       *dmx,
    struct file             *file,
    struct ali_dmx_data_buf *src_buf,
    char __user             *usr_buf,
    size_t                   rd_len
)
{
    __s32 byte_cnt;

    if (0 == rd_len)
    {
        ALI_DMX_DATA_PKT_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }    

    /* One usr each read. */
    if (mutex_lock_interruptible(&(src_buf->rw_mutex)))
    {
        ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        return(-ERESTARTSYS);
    }

    /* Wait until buffer is not empty. */
    while (src_buf->cur_len <= 0)
    {
        if (file->f_flags & O_NONBLOCK)
        {
            //ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

            mutex_unlock(&src_buf->rw_mutex);

            return(-EAGAIN);
        }

        mutex_unlock(&src_buf->rw_mutex);
	if (wait_event_interruptible(src_buf->rd_wq, src_buf->cur_len > 0))
        {
            ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
            return(-ERESTARTSYS);
        }
	  mutex_lock_interruptible(&(src_buf->rw_mutex));	
    }

    byte_cnt = dmx_data_buf_rd(dmx, usr_buf, src_buf, rd_len, 
                               ALI_DMX_DATA_BUF_DEST_USER);

    //wake_up_interruptible(&(src_buf->wr_wq));

    mutex_unlock(&(src_buf->rw_mutex));

    return(byte_cnt);
}




int dmx_data_buf_usr_rd_pkt
(
    struct dmx_device       *dmx,
    struct file             *file,
    struct ali_dmx_data_buf *src_buf,
    char __user             *usr_buf,
    size_t                   rd_len
)
{
    __s32 pkt_len;
    __s32 cpy_len;

    if (0 == rd_len)
    {
        ALI_DMX_DATA_PKT_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }    

    /* One usr each read. */
    if (mutex_lock_interruptible(&(src_buf->rw_mutex)))
    {
        ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        return(-ERESTARTSYS);
    }

    while (dmx_data_buf_first_pkt_len(dmx, src_buf) <= 0)
    {
        if (file->f_flags & O_NONBLOCK)
        {
            mutex_unlock(&src_buf->rw_mutex);

            return(-EAGAIN);
        }
         mutex_unlock(&src_buf->rw_mutex);
        if (wait_event_interruptible(src_buf->rd_wq, 
            dmx_data_buf_first_pkt_len(dmx, src_buf) > 0))
        {
            ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
            return(-ERESTARTSYS);
        }
	mutex_lock_interruptible(&(src_buf->rw_mutex));		
		
    }

    pkt_len = dmx_data_buf_first_pkt_len(dmx, src_buf);

    if (pkt_len > rd_len)
    {
        cpy_len = rd_len;
    }
    else
    {
        cpy_len = pkt_len;
    }

    dmx_data_buf_rd(dmx, usr_buf, src_buf, cpy_len, ALI_DMX_DATA_BUF_DEST_USER);

    if (pkt_len > rd_len)
    {
        dmx_data_buf_flush_last_pkt(dmx, src_buf);
        
        ALI_DMX_DATA_PKT_DEBUG("%s,%d,rd len:%d<pkt_len:%d,"
                               "%d remaining bytes dropped!\n",
                               __FUNCTION__, __LINE__, rd_len, pkt_len,
                               pkt_len - rd_len);
    }

    //wake_up_interruptible(&(src_buf->wr_wq));

    mutex_unlock(&(src_buf->rw_mutex));

    return(cpy_len);
}









__s32 dmx_data_buf_kern_wr_data
(
    struct dmx_device       *dmx,
    struct ali_dmx_data_buf *dest_buf,
    __u8                    *src,
    __s32                    wr_len
)
{
    __s32 byte_cnt;

#if 1
	if (mutex_lock_interruptible(&(dest_buf->rw_mutex)))
    {
        ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        return(-ERESTARTSYS);
    }
#else
    spin_lock(&dmx->data_node_spinlock);
#endif

    byte_cnt = dmx_data_buf_wr_data(dmx, dest_buf, src, wr_len, 
                                    ALI_DMX_DATA_BUF_SRC_KERN);

#if 1
    /* Reduce system call frecunccy to offload CPU loading. */
    if (dmx_data_buf_len(dmx, dest_buf) > 0x10000 /* 64k */)
    {
        wake_up_interruptible(&(dest_buf->rd_wq));
    }
#endif

#if 1
    mutex_unlock(&(dest_buf->rw_mutex));
#else
    spin_unlock(&dmx->data_node_spinlock);
#endif

    return(byte_cnt);    
}


__s32 dmx_data_buf_kern_wr_pkt_end
(
    struct dmx_device       *dmx,
    struct ali_dmx_data_buf *data_buf
)
{
    __s32 ret;

#if 1
    /* One usr each write. */
	if (mutex_lock_interruptible(&(data_buf->rw_mutex)))
    {
        ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        return(-ERESTARTSYS);
    }
#else
    spin_lock(&dmx->data_node_spinlock);
#endif

    ret = dmx_data_buf_wr_pkt_end(dmx, data_buf);

    wake_up_interruptible(&(data_buf->rd_wq));

#if 1
    mutex_unlock(&(data_buf->rw_mutex));
#else
    spin_unlock(&dmx->data_node_spinlock);
#endif

    return(ret);
}




struct ali_dmx_data_pkt* dmx_data_buf_kern_force_unlink_first_pkt
(
    struct dmx_device       *dmx,
    struct ali_dmx_data_buf *data_buf
)
{    
    struct ali_dmx_data_pkt *data_pkt;

#if 1
	if (mutex_lock_interruptible(&(data_buf->rw_mutex)))
    {
        ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        return(NULL);
    }
#endif

    data_pkt = dmx_data_buf_force_unlink_first_pkt(dmx, data_buf);

    //wake_up_interruptible(&(data_buf->wr_wq));

#if 1
    mutex_unlock(&(data_buf->rw_mutex));
#endif

    return(data_pkt);
}



__s32 dmx_data_buf_kern_flush_all_pkt
(
    struct dmx_device       *dmx,
    struct ali_dmx_data_buf *data_buf
)
{
    /* One usr each write. */
	if (mutex_lock_interruptible(&(data_buf->rw_mutex)))
    {
        ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        return(-ERESTARTSYS);
    }

    dmx_data_buf_flush_all_pkt(dmx, data_buf);

    //wake_up_interruptible(&(data_buf->wr_wq));

    mutex_unlock(&(data_buf->rw_mutex));

    return(0);
}



__s32 dmx_data_buf_kern_flush_last_pkt
(
    struct dmx_device       *dmx,
    struct ali_dmx_data_buf *data_buf
)
{
    /* One usr each write. */
	if (mutex_lock_interruptible(&(data_buf->rw_mutex)))
    {
        ALI_DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        return(-ERESTARTSYS);
    }

    dmx_data_buf_flush_last_pkt(dmx, data_buf);

    //wake_up_interruptible(&(data_buf->wr_wq));

    mutex_unlock(&(data_buf->rw_mutex));

    return(0);
}















