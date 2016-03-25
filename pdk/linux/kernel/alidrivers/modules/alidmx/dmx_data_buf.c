
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include "dmx_stack.h"

#if 0
#define DMX_DATA_PKT_DEBUG printk
#else
#define DMX_DATA_PKT_DEBUG(...)
#endif

#ifdef CONFIG_FAST_COPY
#define         VIRTUAL_QUANTUM_SIZE 256*188
#endif
struct dmx_data_buf_module *ali_dmx_data_buf_module;


/* Get a data node from free data node pool to store data. 
*/
static __inline__ struct ali_dmx_data_node* dmx_data_node_req
(
    void
)
{
    struct ali_dmx_data_node *node;
    struct list_head         *node_pool;

    spin_lock(&ali_dmx_data_buf_module->data_node_spinlock);

    node_pool = &(ali_dmx_data_buf_module->data_node_pool);

    if (list_empty(node_pool))
    {
        spin_unlock(&ali_dmx_data_buf_module->data_node_spinlock);
    
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(NULL);
    }

    node = list_first_entry(node_pool, struct ali_dmx_data_node, link);

    //node->len = DMX_DATA_POOL_NODE_SIZE - sizeof(struct ali_dmx_data_node);
    //node->len = DMX_DATA_POOL_NODE_SIZE;

    node->rd = sizeof(struct ali_dmx_data_node);

    node->wr = sizeof(struct ali_dmx_data_node);

    list_del_init(&node->link);

    spin_unlock(&ali_dmx_data_buf_module->data_node_spinlock);

    return(node);
}



/* Return a data node to free data node pool. 
*/
static __inline__ struct ali_dmx_data_node* dmx_data_node_ret
(
    struct ali_dmx_data_node *data
)
{
    struct list_head *node_pool;
    struct list_head *data_link;

    spin_lock(&ali_dmx_data_buf_module->data_node_spinlock);

    data_link = &(data->link);

    node_pool = &(ali_dmx_data_buf_module->data_node_pool);

    list_add_tail(data_link, node_pool);

    spin_unlock(&ali_dmx_data_buf_module->data_node_spinlock);

    return(data);
}


/* Get a data pakcet structure from free data pkt structure pool to store
 * data.
 */
struct ali_dmx_data_pkt *dmx_data_pkt_req
(
    void
)
{
    struct ali_dmx_data_pkt *pkt;
    struct list_head        *pkt_pool;

    spin_lock(&ali_dmx_data_buf_module->data_pkt_spinlock);

    pkt_pool = &(ali_dmx_data_buf_module->data_pkt_pool);

    if (list_empty(pkt_pool))
    {
        spin_unlock(&(ali_dmx_data_buf_module->data_pkt_spinlock));

        printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(NULL);
    }

    pkt = list_first_entry(pkt_pool, struct ali_dmx_data_pkt, link);

    list_del_init(&pkt->link);

    pkt->status = DMX_PKT_STATUS_LOADING;

    pkt->len = 0;

    INIT_LIST_HEAD(&(pkt->data_node));

    spin_unlock(&(ali_dmx_data_buf_module->data_pkt_spinlock));

    return(pkt);
}


/* Return a data pakcet structure to free data pakcet structure pool.
*/
struct ali_dmx_data_pkt* dmx_data_pkt_ret
(
    struct ali_dmx_data_pkt *pkt
)
{
    struct list_head *pkt_pool;
    struct list_head *pkt_link;

    spin_lock(&ali_dmx_data_buf_module->data_pkt_spinlock);

    pkt_link = &(pkt->link);

    pkt_pool = &(ali_dmx_data_buf_module->data_pkt_pool);

    list_add_tail(pkt_link, pkt_pool);

    spin_unlock(&(ali_dmx_data_buf_module->data_pkt_spinlock));

    return(pkt);
}





/* Read data from.
*/
__s32 dmx_data_buf_rd
(
    void                    *dest,
    struct ali_dmx_data_buf *src_buf,
    __s32                    rd_len,
    enum DMX_DATA_CPY_DEST   data_dest
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

#ifdef CONFIG_FAST_COPY
    void *qbuf = (void *)dest;
    __s32  quantum_id = (int)(((__u32)dest>>16) & 0xffff);
    __s32  offset = (int)((__u32)dest & 0xffff);
    
    if(feature_is_fastcopy_enable() && src_buf->fst_cpy_slot >= 0)
        qbuf = fcp_get_quantum(quantum_id, 0) + offset; 

#endif
    

    if (src_buf->cur_len <= 0)
    {
        DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        return(0);
    }

    /* copied_rd_len + remain_rd_len == rd_len.
    */
    copied_rd_len = 0;

    remain_rd_len = rd_len;

    usr_dest = (char __user*)dest;
    
    list_for_each_entry_safe(cur_data_pkt, next_data_pkt, &(src_buf->data_pkt),
                             link)
    {
#if 0
        if (DMX_PKT_STATUS_LOADED != cur_data_pkt->status)
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

//                printk("%s():L[%d], cpy_len[%d], remain_rd_len[%d] \n",
//                      __FUNCTION__, __LINE__, cpy_len, remain_rd_len);
            }

            if (DMX_DATA_CPY_DEST_USER == data_dest)
            {

#ifdef CONFIG_FAST_COPY
                if (feature_is_fastcopy_enable())
                {
                    if(src_buf->fst_cpy_slot >= 0)
                    {
                        memcpy(qbuf+src_buf->fst_cpy_size, cpy_src, cpy_len);        
                    }
                    else 
                    {
                        if (copy_to_user(usr_dest + copied_rd_len, cpy_src, cpy_len))
                        {
                            DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

                            return(-EFAULT);     
                        }
                    }
                } 
                else 
                {
                    if (copy_to_user(usr_dest + copied_rd_len, cpy_src, cpy_len))
                    {
                        DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

                        return(-EFAULT);     
                    }
                }
#else            
                if (copy_to_user(usr_dest + copied_rd_len, cpy_src, cpy_len))
                {
                    printk("%s,%d,usr_dest:%x,copied_rd_len:%d,cpy_src:%x,cpy_len:%x\n",
                           __FUNCTION__, __LINE__, (__u32)usr_dest, copied_rd_len, (__u32)cpy_src,
                           cpy_len);
       
                    return(-EFAULT);     
                }
#endif
                
            }
            else
            {
                memcpy(dest + copied_rd_len, cpy_src, cpy_len);
            }

            cur_data_node->rd += cpy_len;

            cur_data_pkt->len -= cpy_len;
    

            if (remain_rd_len >= node_len)
            {
                /* Remove current data node from data node list.
                */
                list_del(&(cur_data_node->link));
    
                dmx_data_node_ret(cur_data_node);
            }

            copied_rd_len += cpy_len;

            remain_rd_len -= cpy_len; 

            src_buf->cur_len -= cpy_len;

#ifdef CONFIG_FAST_COPY

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
#endif

            /* Case 0: All data required by user has been satisfied. 
            */
            if (remain_rd_len <= 0)
            {
                if (list_empty(&(cur_data_pkt->data_node)))
                {
                    /* Remove current data packet from data packet list.
                    */
                    list_del(&(cur_data_pkt->link));  
            
                    dmx_data_pkt_ret(cur_data_pkt);
                }

                return(copied_rd_len);  
            }
        }

        /* Remove current data packet from data packet list.
        */
        list_del(&(cur_data_pkt->link));  

        dmx_data_pkt_ret(cur_data_pkt);
    }

    /* Case 1: All data in current buffer is less than user required. 
    */
    return(copied_rd_len); 
}






__s32 dmx_data_buf_wr_data
(
    struct ali_dmx_data_buf *dest_buf,
    __u8                    *src,
    __s32                    wr_len,
    enum DMX_DATA_SRC_TYPE   src_type
)
{
    struct ali_dmx_data_pkt  *data_pkt;
    struct ali_dmx_data_node *data_node;
    __s32                     remain_wr_len;
    __s32                     copied_wr_len;
    __s32                     data_node_len;
    __s32                     cpy_len;
    char __user              *usr_src;

#ifdef CONFIG_FAST_COPY
    void  *qbuf = (void *)src;
    __s32  quantum_id = (__s32)(((__u32)src>>16) & 0xffff);
    __s32  offset = (__s32)((__u32)src & 0xffff);
    
    if (feature_is_fastcopy_enable() && DMX_DATA_SRC_TYPE_FAST == src_type)
    {
        qbuf = fcp_get_quantum(quantum_id, 1) + offset;         
    }  
    
#endif

    //DMX_DATA_PKT_DEBUG("%s,%d,l:%d\n", __FUNCTION__, __LINE__, wr_len);

    /* If empty buffer.
    */
    if (list_empty(&(dest_buf->data_pkt)))
    {
        data_pkt = dmx_data_pkt_req();

        if (NULL == data_pkt)
        {
            //printk("%s, %d\n", __FUNCTION__, __LINE__);
    
            return(-ENOMEM);
        }

        list_add_tail(&data_pkt->link, &dest_buf->data_pkt);
    }
    else
    {
        data_pkt = list_last_entry(&(dest_buf->data_pkt), 
                                   struct ali_dmx_data_pkt, link); 

#if 0
        /* pkt list empty and no pkt in this buffer, this should never happen.
         * Just for safty.
         */
        if (NULL == data_pkt)
        {
            DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    
            return(-ENOMEM);
        }  
#endif 
    }

    /* Store in new pakcet if privous packet has been fihished. */
    if (DMX_PKT_STATUS_LOADED == data_pkt->status)
    {
        data_pkt = dmx_data_pkt_req();

        /* Run out of packet node, drop this pakcet. */
        if (NULL == data_pkt)
        {
            //printk("%s, %d\n", __FUNCTION__, __LINE__);
    
            return(-ENOMEM);
        }

        list_add_tail(&data_pkt->link, &dest_buf->data_pkt);
    }

    //DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    /* If empty pakcet. */
    if (list_empty(&data_pkt->data_node))
    {
        data_node = dmx_data_node_req();

        /* Run out of data node, drop this pakcet. */
        if (NULL == data_node)
        {
            //printk("%s, %d\n", __FUNCTION__, __LINE__);
    
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
        /* Data node full, get new data node from data node pool.
        */
        data_node = dmx_data_node_req();

        /* Run out of data node, drop this pakcet. */
        if (NULL == data_node)
        {
            //printk("%s, %d\n", __FUNCTION__, __LINE__);
    
            return(-ENOMEM);
        }

        list_add_tail(&data_node->link, &data_pkt->data_node);
    }

    remain_wr_len = wr_len;

    copied_wr_len = 0;

    usr_src = (char __user *)src;

    /* Store all the data into data node.
    */
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

        if (DMX_DATA_SRC_TYPE_KERN == src_type)
        {
			memcpy(((void *)data_node) + data_node->wr,
			                   	((void *)src + copied_wr_len), cpy_len);
        }
        else
        {

#ifdef CONFIG_FAST_COPY
            if (feature_is_fastcopy_enable())
            {
                if(DMX_DATA_SRC_TYPE_FAST == src_type)
                {   
                    if(dest_buf->fst_cpy_size + cpy_len > VIRTUAL_QUANTUM_SIZE)
                        cpy_len = VIRTUAL_QUANTUM_SIZE - dest_buf->fst_cpy_size; 
                    
                    memcpy(((void *)data_node) + data_node->wr,
                                qbuf + dest_buf->fst_cpy_size, cpy_len);
                    
                    DMX_DATA_PKT_DEBUG("%s(): %d,copy %d\n", __FUNCTION__, __LINE__,cpy_len);

                    dest_buf->fst_cpy_size += cpy_len;
                    dest_buf->exp_cpy_size -= cpy_len;
                    if(dest_buf->exp_cpy_size <= 0)
                    {
                        if(dest_buf->exp_cpy_size != 0)
                        {
                            DMX_DATA_PKT_DEBUG("%s(): %d, dmx fast read more data than expected !\n", \
                                            __FUNCTION__, __LINE__);
                        }
                        
                        fcp_put_quantum(quantum_id);
                        dest_buf->fst_cpy_size = 0;

                    }
                }
                else
                {
                    if (copy_from_user(((void *)data_node) + data_node->wr,usr_src + copied_wr_len, cpy_len))
                    {
                        DMX_DATA_PKT_DEBUG("%s(), %d, usr_dest[%x],usr_src[%x],copied_wr_len[%d],cpy_len[%x] \n",\
                               __FUNCTION__, __LINE__, (((void *)data_node) + data_node->wr), usr_src,copied_wr_len,cpy_len);
                    
                        return(-EFAULT);     
                    }
                }
            }
#else
        
            if (copy_from_user(((void *)data_node) + data_node->wr,usr_src + copied_wr_len, cpy_len))
            {
                DMX_DATA_PKT_DEBUG("%s(), %d, usr_dest[%x],usr_src[%x],copied_wr_len[%d],cpy_len[%x] \n",\
                       __FUNCTION__, __LINE__, (((void *)data_node) + data_node->wr), usr_src,copied_wr_len,cpy_len);

                return(-EFAULT);     
            }

#endif

        }

        data_node->wr += cpy_len;

        data_pkt->len += cpy_len;

        dest_buf->cur_len += cpy_len;

        remain_wr_len -= cpy_len;

        copied_wr_len += cpy_len;

        //DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        /* Get a new data node if current data node can not store all
         * the data.
         */
        if (remain_wr_len > 0)
        {
            data_node = dmx_data_node_req();
    
            /* Run out of data node, drop this pakcet. */
            if (NULL == data_node)
            {
                //printk("%s, %d\n", __FUNCTION__, __LINE__);
        
                return(copied_wr_len);
            }
            
            list_add_tail(&data_node->link, &data_pkt->data_node);
        }

        //DMX_DATA_PKT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    }

    return(copied_wr_len);
}



__s32 dmx_data_buf_wr_pkt_end
(
    struct ali_dmx_data_buf *data_buf
)
{
    struct ali_dmx_data_pkt *data_pkt;

    if (list_empty(&(data_buf->data_pkt)))
    {
        DMX_DATA_PKT_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        return(DMX_ERR_DATA_BUF_EMPTY);
    }

    data_pkt = list_last_entry(&(data_buf->data_pkt), 
                               struct ali_dmx_data_pkt, link);    

    if (NULL == data_pkt)
    {
        DMX_DATA_PKT_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }

    data_pkt->status = DMX_PKT_STATUS_LOADED;

    return(0);
}



struct ali_dmx_data_pkt* dmx_data_buf_force_unlink_first_pkt
(
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

    dmx_data_pkt_ret(data_pkt);

    return(data_pkt);
}




#if 1

__s32 dmx_data_buf_drop_incomplete_pkt
(
    struct ali_dmx_data_buf *data_buf
)
{
    struct ali_dmx_data_node *cur_data_node;
    struct ali_dmx_data_node *next_data_node;
    struct ali_dmx_data_pkt  *data_pkt;
    
    //printk("%s,%d\n", __FUNCTION__, __LINE__);

    if (list_empty(&(data_buf->data_pkt)))
    {
        //printk("%s,%d\n", __FUNCTION__, __LINE__);
        
        return(0);
    }

    //printk("%s,%d\n", __FUNCTION__, __LINE__);

    /* Find last data packet.
    */
    data_pkt = list_last_entry(&(data_buf->data_pkt), struct ali_dmx_data_pkt, 
                               link);

    /* If last pakcet has been completed, do nothing.
     */
    if (DMX_PKT_STATUS_LOADED == data_pkt->status)
    {
        //printk("%s,%d\n", __FUNCTION__, __LINE__);
        
        return(0);
    }

    //printk("%s,%d\n", __FUNCTION__, __LINE__);

    list_for_each_entry_safe(cur_data_node, next_data_node,
                             &(data_pkt->data_node), link)
    {
        //printk("%s,%d\n", __FUNCTION__, __LINE__);
        
        /* Remove current data node from data node list.
        */
        list_del(&(cur_data_node->link));

        //printk("%s,%d\n", __FUNCTION__, __LINE__);
    
        dmx_data_node_ret(cur_data_node);

        //printk("%s,%d\n", __FUNCTION__, __LINE__);
        
    }

    //printk("%s,%d\n", __FUNCTION__, __LINE__);

    data_buf->cur_len -= data_pkt->len;
    
    /* Remove current data packet from data packet list.
    */
    list_del(&(data_pkt->link));  
    
    dmx_data_pkt_ret(data_pkt);

    //printk("%s,%d\n", __FUNCTION__, __LINE__);

    return(0);
}
#else

__s32 dmx_data_buf_drop_incomplete_pkt
(
    struct ali_dmx_data_buf *data_buf
)
{
    struct ali_dmx_data_pkt  *cur_data_pkt;
    struct ali_dmx_data_pkt  *next_data_pkt;
    struct ali_dmx_data_node *cur_data_node;
    struct ali_dmx_data_node *next_data_node;
    struct ali_dmx_data_pkt  *data_pkt;

    printk("%s,%d\n", __FUNCTION__, __LINE__);

    if (list_empty(&(data_buf->data_pkt)))
    {
        return(0);
    }

    printk("%s,%d\n", __FUNCTION__, __LINE__);
    
    /* Find last data packet.
    */
    data_pkt = list_last_entry(&(data_buf->data_pkt), struct ali_dmx_data_pkt, 
                               link);

    printk("%s,%d\n", __FUNCTION__, __LINE__);

    list_for_each_entry_safe(cur_data_node, next_data_node,
                             &(cur_data_pkt->data_node), link)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        
        /* Remove current data node from data node list.
        */
        list_del(&(cur_data_node->link));
    
        dmx_data_node_ret(cur_data_node);
    }

    printk("%s,%d\n", __FUNCTION__, __LINE__);

    /* Remove current data packet from data packet list.
    */
    list_del(&(cur_data_pkt->link));  
    
    dmx_data_pkt_ret(cur_data_pkt);

    data_buf->cur_len -= cur_data_pkt->len;

    printk("%s,%d\n", __FUNCTION__, __LINE__);

    return(0);

}


#endif




__s32 dmx_data_buf_flush_all_pkt
(
    struct ali_dmx_data_buf *data_buf
)
{
    struct ali_dmx_data_pkt  *cur_data_pkt;
    struct ali_dmx_data_pkt  *next_data_pkt;
    struct ali_dmx_data_node *cur_data_node;
    struct ali_dmx_data_node *next_data_node;

    //printk("%s,%d,buf len:%d\n", __FUNCTION__, __LINE__, data_buf->cur_len);

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
            /* Remove current data node from data node list.
            */
            list_del(&(cur_data_node->link));
        
            dmx_data_node_ret(cur_data_node);
        }

        /* Remove current data packet from data packet list.
        */
        list_del(&(cur_data_pkt->link));  
        
        dmx_data_pkt_ret(cur_data_pkt);
    }

    data_buf->cur_len = 0;

    return(0);
}






__u32 dmx_data_buf_first_pkt_len
(
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

    if (DMX_PKT_STATUS_LOADING == data_pkt->status)
    {
        return(0);
    }

    return(data_pkt->len);
}





__u32 dmx_data_buf_total_len
(
    struct ali_dmx_data_buf *data_buf
)
{
    if (list_empty(&(data_buf->data_pkt)))
    {
        return(0);
    }

    return(data_buf->cur_len);
}


__s32 dmx_data_buf_list_init
(
    struct ali_dmx_data_buf *data_buf
)
{
    data_buf->cur_len = 0;
    
    mutex_init(&(data_buf->rw_mutex));
    
    init_waitqueue_head(&(data_buf->rd_wq));
    
    init_waitqueue_head(&(data_buf->wr_wq));
    
    INIT_LIST_HEAD(&(data_buf->data_pkt));

    return(0);
}




#if 0

__s32 dmx_data_buf_module_release
(
    void
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

    if (NULL != dmx_data_buf.data_node_raw)
    {
        free_pages((__u32)dmx_data_buf.data_node_raw, node_buf_order);
    }


    pkt_len = sizeof(struct ali_dmx_data_pkt);

    pkt_buf_len = DMX_DATA_POOL_NODE_CNT * pkt_len;

    pkt_buf_order = get_order(pkt_buf_len);

    if (NULL != dmx_data_buf.data_pkt_raw)
    {
        free_pages((__u32)dmx_data_buf.data_pkt_raw, pkt_buf_order);
    }

    return(0);
}
#else
__s32 dmx_data_buf_module_release
(
    void
)
{
    if (NULL != ali_dmx_data_buf_module->data_node_raw)
    {
        vfree(ali_dmx_data_buf_module->data_node_raw);
    }

    if (NULL != ali_dmx_data_buf_module->data_pkt_raw)
    {
        vfree(ali_dmx_data_buf_module->data_pkt_raw);
    }

    return(0);
}


#endif

#if 0
__s32 dmx_data_buf_module_init
(
     void
)
{
    __u32                     i;
    __u32                     data_node_size;
    __u32                     data_node_cnt;
    __u32                     node_len;
    __u32                     node_buf_len;
    __u32                     node_buf_order;
    __u32                     pkt_len;
    __u32                     pkt_buf_len;
    __u32                     pkt_buf_order;
    struct ali_dmx_data_node *node;
    struct ali_dmx_data_pkt  *pkt;
    __u32                     data_pkt_cnt;

    data_node_size = DMX_DATA_POOL_NODE_SIZE;

    data_node_cnt = DMX_DATA_POOL_NODE_CNT;

    node_len = data_node_size + sizeof(struct ali_dmx_data_node);

    node_buf_len = data_node_cnt * node_len; 

    node_buf_order = get_order(node_buf_len);

    dmx_data_buf.data_node_raw = (void *)__get_free_pages(GFP_KERNEL, node_buf_order);

    if (!(dmx_data_buf.data_node_raw))
    {
        panic("%s,%d,get %d bytes ram fail, node_size:%d, node_cnt:%d\n",
              __FUNCTION__, __LINE__, node_buf_len, data_node_size, 
              data_node_cnt);

        return(-ENOMEM);
    }

    /* data_pkt_cnt == data_node_cnt to gurantee one-to-one relationship 
     * between data_pkt_cnt and data_node_cnt in worst case.
     */
    data_pkt_cnt = data_node_cnt;

    pkt_len = sizeof(struct ali_dmx_data_pkt);

    pkt_buf_len = data_pkt_cnt * pkt_len;

    pkt_buf_order = get_order(pkt_buf_len);

    dmx_data_buf.data_pkt_raw = (void *)__get_free_pages(GFP_KERNEL, pkt_buf_order);

    if (!(dmx_data_buf.data_node_raw))
    {
        DMX_DATA_PKT_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        free_pages((__u32)dmx_data_buf.data_node_raw, node_buf_order);

        return(-ENOMEM);
    }

    DMX_DATA_PKT_DEBUG("data_node_raw:0x%x~0x%x\n", 
                           (__u32)dmx_data_buf.data_node_raw,
                           (__u32)(dmx_data_buf.data_node_raw + node_buf_len));
    
    mutex_init(&(dmx_data_buf.data_node_mutex));
    
    mutex_init(&(dmx_data_buf.data_pkt_mutex));

    INIT_LIST_HEAD(&(dmx_data_buf.data_node_pool));

    INIT_LIST_HEAD(&(dmx_data_buf.data_pkt_pool));

    /* Init free memory pools.
    */
    for (i = 0; i < data_node_cnt; i++)
    {
        node = (struct ali_dmx_data_node *)(dmx_data_buf.data_node_raw + i * node_len);

        node->len = node_len;

        node->rd = 0;

        node->wr = 0;

        dmx_data_node_ret(node);
    }

    for (i = 0; i < data_pkt_cnt; i++)
    {
        pkt = (struct ali_dmx_data_pkt *)(dmx_data_buf.data_pkt_raw + i * pkt_len);

        pkt->len = 0;

        dmx_data_pkt_ret(pkt);
    }

    return(0);
}
#else

__s32 dmx_data_buf_module_init
(
     void
)
{
    __u32                     i;
    __u32                     data_node_size;
    __u32                     data_node_cnt;
    __u32                     node_len;
    __u32                     node_buf_len;
    __u32                     pkt_len;
    __u32                     pkt_buf_len;
    struct ali_dmx_data_node *node;
    struct ali_dmx_data_pkt  *pkt;
    __u32                     data_pkt_cnt;

    data_node_size = DMX_DATA_POOL_NODE_SIZE;

    data_node_cnt = DMX_DATA_POOL_NODE_CNT;

    node_len = data_node_size + sizeof(struct ali_dmx_data_node);

    node_buf_len = data_node_cnt * node_len; 

    ali_dmx_data_buf_module = kmalloc(sizeof(struct dmx_data_buf_module), GFP_KERNEL);
    
    if (!(ali_dmx_data_buf_module))
    {
        panic("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(-ENOMEM);
    }
	
    ali_dmx_data_buf_module->data_node_raw = (void *)vmalloc(node_buf_len);
    
    if (!(ali_dmx_data_buf_module->data_node_raw))
    {
        panic("%s,%d,get %d bytes ram fail, node_size:%d, node_cnt:%d\n",
              __FUNCTION__, __LINE__, node_buf_len, data_node_size, 
              data_node_cnt);
    
        return(-ENOMEM);
    }

    /* data_pkt_cnt == data_node_cnt to gurantee one-to-one relationship 
     * between data_pkt_cnt and data_node_cnt in worst case.
     */
    data_pkt_cnt = data_node_cnt;

    pkt_len = sizeof(struct ali_dmx_data_pkt);

    pkt_buf_len = data_pkt_cnt * pkt_len;

    ali_dmx_data_buf_module->data_pkt_raw = (void *)vmalloc(pkt_buf_len);
    
    if (NULL == ali_dmx_data_buf_module->data_pkt_raw)
    {    
        vfree(ali_dmx_data_buf_module->data_node_raw);
    
        panic("%s,%d,vmalloc fail, pkt_buf_len:%d\n",
              __FUNCTION__, __LINE__, pkt_buf_len);
    }
    
    DMX_DATA_PKT_DEBUG("data_node_raw:0x%x~0x%x\n", 
                           (__u32)ali_dmx_data_buf_module->data_node_raw,
                           (__u32)(ali_dmx_data_buf_module->data_node_raw + node_buf_len));

#if 0	
    mutex_init(&(ali_dmx_data_buf_module->data_node_mutex));
    
    mutex_init(&(ali_dmx_data_buf_module->data_pkt_mutex));
#else
	spin_lock_init(&(ali_dmx_data_buf_module->data_node_spinlock));
	spin_lock_init(&(ali_dmx_data_buf_module->data_pkt_spinlock));
#endif

    INIT_LIST_HEAD(&(ali_dmx_data_buf_module->data_node_pool));

    INIT_LIST_HEAD(&(ali_dmx_data_buf_module->data_pkt_pool));

    /* Init free memory pools.
    */
    for (i = 0; i < data_node_cnt; i++)
    {
        node = (struct ali_dmx_data_node *)(ali_dmx_data_buf_module->data_node_raw + i * node_len);

        node->len = node_len;

        node->rd = 0;

        node->wr = 0;

        dmx_data_node_ret(node);
    }

    for (i = 0; i < data_pkt_cnt; i++)
    {
        pkt = (struct ali_dmx_data_pkt *)(ali_dmx_data_buf_module->data_pkt_raw + i * pkt_len);

        pkt->len = 0;

        dmx_data_pkt_ret(pkt);
    }

    return(0);
}

#endif







