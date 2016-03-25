/*                                                     
 * $Id: hellop.c,v 1.4 2004/09/26 07:02:43 gregkh Exp $ 
 */                                                    
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/freezer.h>
#include <linux/features.h>

#include <asm/io.h>

#include <ali_basic_common.h>


#include <ali_interrupt.h>


#include "dmx_internal.h"

#include <linux/dvb/ali_dsc.h> 

MODULE_LICENSE("GPL");
                                                                                                 
#include <asm/mach-ali/unified_bsp_board_attr.h>                                        

struct dmx_device g_ali_dmx_devices[2];

struct class *g_ali_m36_dmx_class;

struct dmx_enc_info g_ali_dmx_enc_info;

struct file_operations g_ali_m36_dmx_fops = {
	.owner =    THIS_MODULE,
	//.llseek =   scull_llseek,
	.read =     dmx_channel_read,
	//.write =    dmx_write,
	.unlocked_ioctl =    dmx_ioctl,
	.open =     dmx_channel_open,
	.release =  dmx_channel_release,
    .poll = dmx_channel_poll,
};


#if 0
/*
 * Create a group of attributes so that we can create and destory them all
 * at once.
 */
static struct attribute *dmx_attrs[] = 
{
    &(g_ali_dmx_devices[0].attr_ts_in_cnt.attr),
    &(g_ali_dmx_devices[0].attr_ts_services.attr),
     
    //&dmx_ts_in_cnt.attr,
	//&baz_attribute.attr,
	//&bar_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};



/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group dmx_attr_group = {
	.attrs = dmx_attrs,
};
#endif

/* Check if dmx hw buffer is overflow, clean hw buffer if it is. */
__s32 dmx_hw_buf_overflow_proc
(
    struct dmx_device *dmx
)
{
    DMX_UINT32  rd_idx;
    DMX_UINT32  wr_idx;
    DMX_UINT32  next_wr_idx;

    /* Check if HW buffer is overflow. */
    rd_idx = dmx->pfunc.dmx_hw_buf_rd_get(dmx->base_addr);

    wr_idx = dmx->pfunc.dmx_hw_buf_wr_get(dmx->base_addr);

    next_wr_idx = wr_idx + 1;

    if (next_wr_idx >= dmx->ts_buf_end_idx)
    {
        next_wr_idx = 0;
    }

    /* Overflow. */
    if (next_wr_idx == rd_idx)
    {
        printk("%s, hw buf overflow,rd:%d,wr:%d,last parsed pid:%d\n",
               dmx->name, rd_idx, wr_idx, dmx->last_parsed_pid);

        dmx->pfunc.dmx_hw_buf_rd_set(dmx->base_addr, wr_idx);

        /* Overflow handled. */
        return(1);
    }

    /* No overflow found. */
    return(0);
}

static RET_CODE dmx_find_scrambled_pkt(UINT8* buf, UINT32 total_length)
{
    UINT32 i = 0;

    for(i = 0; i < total_length; i++)
    {  
         if(((buf[3] & 0xC0) >> 6) != 0)
		     return RET_SUCCESS;
          buf += 188;
    }
	
    return !RET_SUCCESS;
 
}

/*****************************************************************************/
/**
*
* Retrieve TS packets from demux hw buffer, dispatch them to upper layer.
*
* @param
* - param1: Not used.
*
* - param2: Not used
*
* @note
*
* @return
* - NONE
*
******************************************************************************/
void dmx_ca_record_task
(
	struct work_struct *work
)
{
    struct dmx_ts_channel_info   *ts_ch_info;
    DMX_UINT32 enc_blk_len, enc_blk_tail_len;
    struct dmx_enc_info *enc_info = &g_ali_dmx_enc_info;
    int i;
    //DEEN_CONFIG config;

	//if (!feature_is_alias_enable())
		//return;

    set_freezable();
    for(;;)
    {
        try_to_freeze();     
        msleep(30);
        
        for(i = 0; i < MAX_CA_RECORD_CHANNEL; i++)
            if(enc_info->channel[i] != NULL && enc_info->channel[i]->ts_ch_info.status !=  ALI_DMX_CA_RECORD_PAUSED)
            {
                pause_exit:
                if(enc_info->channel[i]->ts_ch_info.status == ALI_DMX_CA_RECORD_PAUSE)
                {
                   enc_info->channel[i]->ts_ch_info.status =  ALI_DMX_CA_RECORD_PAUSED;
                   continue;
                }
                ts_ch_info = &(enc_info->channel[i]->ts_ch_info);
                enc_blk_len = (ts_ch_info->enc_blk_len / DMX_ENC_UNIT_LEN) * DMX_ENC_UNIT_LEN;
                enc_blk_tail_len = DMX_ENC_BLK_LEN - ts_ch_info->enc_blk_rd;
                if(enc_blk_tail_len > enc_blk_len) enc_blk_tail_len = enc_blk_len;
                
                if(enc_blk_len >= DMX_ENC_UNIT_LEN)
                { 
                    try_to_freeze();
                    extern int ali_DeEncrypt(pDEEN_CONFIG p_DeEn,UINT8 *input,UINT8 *output , UINT32 total_length);
                    extern int ali_dsc_deal_quantum_for_mixed_ts(pDEEN_CONFIG p_DeEn, UINT32 temp_length);
					
                    if(ts_ch_info->enc_para==NULL) 
                        continue;
                    //if (0 != copy_from_user(&config, (void __user *)(ts_ch_info->enc_para), sizeof(DEEN_CONFIG)))
                    //{
                    //    return(-EFAULT);
                    //}   
                    
                    //Wait dst buffer empty
            	      while((DMX_ENC_BLK_LEN - ts_ch_info->dst_blk_len) <= enc_blk_tail_len)
            	      {
            	          try_to_freeze();
            	          msleep(1);
                          if(enc_info->channel[i]->ts_ch_info.status == ALI_DMX_CA_RECORD_PAUSE)
                            goto pause_exit;
            	      }
            	      /*Process all data in enc buffer till buffer end, enc_blk_rd always equal to dst_blk_wr*/
                      if(RET_SUCCESS == dmx_find_scrambled_pkt((DMX_UINT8 *)(ts_ch_info->enc_blk+ts_ch_info->enc_blk_rd), enc_blk_tail_len / 188))
                      {
                        
                           ali_DeEncrypt(ts_ch_info->enc_para,\
                                        (DMX_UINT8 *)(ts_ch_info->enc_blk + ts_ch_info->enc_blk_rd), \
                                        (DMX_UINT8 *)(ts_ch_info->dst_blk + ts_ch_info->dst_blk_wr), \
                                        enc_blk_tail_len / 188);
                        /* printk("DeEn config: %x,%x,%x,%x,%x\n",(pDEEN_CONFIG)(ts_ch_info->enc_para),\
                                           (DMX_UINT8 *)(ts_ch_info->enc_blk+ts_ch_info->enc_blk_rd), \
                                           (DMX_UINT8 *)(ts_ch_info->dst_blk+ts_ch_info->dst_blk_wr), \
                                           (DMX_UINT8 *)(ts_ch_info->enc_blk+ts_ch_info->enc_blk_rd)[180], \
                                           (DMX_UINT8 *)(ts_ch_info->dst_blk+ts_ch_info->dst_blk_wr)[180]\
                                  ); */
                      }
                      else
                      {
                          MEMCPY((DMX_UINT8 *)(ts_ch_info->dst_blk + ts_ch_info->dst_blk_wr),\
                                 (DMX_UINT8 *)(ts_ch_info->enc_blk + ts_ch_info->enc_blk_rd),\
                                 enc_blk_tail_len);
						  ali_dsc_deal_quantum_for_mixed_ts(ts_ch_info->enc_para,enc_blk_tail_len / 188);
                      }
                    mutex_lock_interruptible(&enc_info->io_mutex);
					//printk("MAIN %d, Len 0x%x, Tail 0x%x\n",(tick2-tick1)/300,enc_blk_len,enc_blk_tail_len);
                    if((ts_ch_info->enc_blk_rd + enc_blk_tail_len) >= DMX_ENC_BLK_LEN)
                        ts_ch_info->enc_blk_rd = 0;
                    else
                        ts_ch_info->enc_blk_rd += enc_blk_tail_len;
                    ts_ch_info->dst_blk_wr =  ts_ch_info->enc_blk_rd;
                    ts_ch_info->enc_blk_len -= enc_blk_tail_len;
                    ts_ch_info->dst_blk_len += enc_blk_tail_len;  
                    enc_blk_len -= enc_blk_tail_len;
                    mutex_unlock(&enc_info->io_mutex); 
                    if(enc_blk_len != 0)
                    {
                        //Wait dst buffer empty
                        while((DMX_ENC_BLK_LEN - ts_ch_info->dst_blk_len) <= enc_blk_len)
                        {
                            try_to_freeze();
                            msleep(1);
                            if(enc_info->channel[i]->ts_ch_info.status == ALI_DMX_CA_RECORD_PAUSE)
                               goto pause_exit;
                        }
                        if(RET_SUCCESS == dmx_find_scrambled_pkt((DMX_UINT8 *)(ts_ch_info->enc_blk+ts_ch_info->enc_blk_rd), enc_blk_len  / 188))
                        {
                            ali_DeEncrypt(ts_ch_info->enc_para,\
                                          (DMX_UINT8 *)(ts_ch_info->enc_blk), \
                                          (DMX_UINT8 *)(ts_ch_info->dst_blk), \
                                          enc_blk_len/188);
                        }
                        else
                        {
                            MEMCPY((DMX_UINT8 *)(ts_ch_info->dst_blk + ts_ch_info->dst_blk_wr),\
                                   (DMX_UINT8 *)(ts_ch_info->enc_blk + ts_ch_info->enc_blk_rd),\
                                   enc_blk_len);
							ali_dsc_deal_quantum_for_mixed_ts(ts_ch_info->enc_para,enc_blk_len / 188);
                        }
                        mutex_lock_interruptible(&enc_info->io_mutex);                    
                        ts_ch_info->enc_blk_rd = ts_ch_info->dst_blk_wr = enc_blk_len;
                        ts_ch_info->enc_blk_len -= enc_blk_len;
                        ts_ch_info->dst_blk_len += enc_blk_len;  
                        mutex_unlock(&enc_info->io_mutex);
                    }
                              
                }
             } 
    }               
}

UINT32 dmx_get_current_timeMS
(
    void
)
{
    struct timeval tv;
	do_gettimeofday(&tv);
	return (tv.tv_sec*1000) + (tv.tv_usec/1000);
}

void dmx_detect_bitrate(struct dmx_device     *dmx)
{
    UINT32 curTime;

    curTime = dmx_get_current_timeMS();
    if (mutex_lock_interruptible(&dmx->io_mutex))
    {
        return(-ERESTARTSYS);
    }
    if(dmx->last_rd_time>0)
    {
        dmx->pkts_dura += curTime - dmx->last_rd_time;
    }
    dmx->last_rd_time = curTime;

	if(dmx->pkts_dura > 100000)
	{
		dmx->pkt_total_in_cnt = 0;
		dmx->last_pack_num=0;
		dmx->pkts_dura = 1;
		dmx->last_dura=1;
		//priv->bitrate_detect=0;
	}

    if ( (dmx->bitrate_detect==1) &&((dmx->pkts_dura-dmx->last_dura)>30))
    {
         UINT8 cnt=0;
         UINT32 bit_rate=0;

        bit_rate = (((dmx->pkt_total_in_cnt-dmx->last_pack_num)* 188 * 8) / (dmx->pkts_dura-dmx->last_dura)) * 1000;

        for(cnt=0;cnt<7;cnt++)
        {
             dmx->last_rate[cnt]=dmx->last_rate[cnt+1];
        }
        dmx->last_rate[7]=bit_rate;

        if(dmx->last_rate[0]==0)
           dmx->bit_rate= bit_rate;
        else
        {
            bit_rate=(dmx->last_rate[0]+dmx->last_rate[1]+dmx->last_rate[2]+dmx->last_rate[3]+
                          dmx->last_rate[4]+dmx->last_rate[5]+dmx->last_rate[6]+dmx->last_rate[7])>>3;
            dmx->bit_rate= bit_rate;
        }
           
        dmx->last_pack_num=dmx->pkt_total_in_cnt;
        dmx->last_dura=dmx->pkts_dura;
        
     }
     mutex_unlock(&dmx->io_mutex);
    return ;
}

extern BOOL dmx_api_check_fe_mismatch();
extern void dmx_check_front_end(UINT32 fe,UINT32 nim_chip_id);

/*****************************************************************************/
/**
*
* Retrieve TS packets from demux hw buffer, dispatch them to upper layer.
*
* @param
* - param1: Not used.
*
* - param2: Not used
*
* @note
*
* @return
* - NONE
*
******************************************************************************/
void dmx_hw_parse_task
(
	struct work_struct *work
)
{
    //DMX_UINT32           inv_len;
    struct dmx_device     *dmx;
    struct dmx_services   *services;
    struct dmx_ts_pkt_inf *ts_pkt_inf;

    
	dmx_check_front_end(0,0xFFFF);
    dmx = container_of((void *)work, struct dmx_device, work); 

    services = &dmx->services;

    for(;;)
    {    
        /* Check dmx buffer every DMX_HW_PARSE_TASK_WAIT_TIME miliseconds.
         * This should be enough to meet all time reqirments.
         */
        msleep(DMX_HW_PARSE_TASK_WAIT_TIME);
        extern unsigned long ali_dmx_m3501_autoscan;
				if(ali_dmx_m3501_autoscan == 1)
					continue;

        if(dmx_api_check_fe_mismatch())
		{
			
			UINT32 data = 0;
			data = *(volatile UINT32 *)(0xb8006300);		
			data &= 0xfffffffe;
			*(volatile UINT32 *)(0xb8006300) = data;
		}
        /* Loop to process TS packets, copy the TS packet payload to 
         * upper layer buffers.
         */
        dmx->ts_buf_rd_idx = dmx->pfunc.dmx_hw_buf_rd_get(dmx->base_addr);

        dmx->ts_buf_wr_idx = dmx->pfunc.dmx_hw_buf_wr_get(dmx->base_addr);

        if (dmx->ts_buf_wr_idx == dmx->ts_buf_rd_idx)
        {
            continue;
        }

#if 0
        /* Invalidate cach for HW buffer. */
        if (dmx->ts_buf_rd_idx > dmx->ts_buf_wr_idx)
        {
            inv_len = dmx->ts_buf_end_idx - dmx->ts_buf_rd_idx;

            ts_pkt_inf = &(dmx->pkt_inf[dmx->ts_buf_rd_idx]);

            dma_cache_inv((DMX_UINT32)(ts_pkt_inf->pkt_addr), inv_len * 188);

            inv_len = dmx->ts_buf_wr_idx;

            ts_pkt_inf = &(dmx->pkt_inf[0]);

            dma_cache_inv((DMX_UINT32)(ts_pkt_inf->pkt_addr), inv_len * 188);
        }
        else
        {
            inv_len = dmx->ts_buf_wr_idx - dmx->ts_buf_rd_idx;

            ts_pkt_inf = &(dmx->pkt_inf[dmx->ts_buf_rd_idx]);

            dma_cache_inv((DMX_UINT32)(ts_pkt_inf->pkt_addr), inv_len * 188);
        }
#endif

        /* If HW buffer not empty, proccess TS packet. */
        while(dmx->ts_buf_rd_idx != dmx->ts_buf_wr_idx)
        { 
            ts_pkt_inf = &(dmx->pkt_inf[dmx->ts_buf_rd_idx]);

            dma_cache_inv((DMX_UINT32)(ts_pkt_inf->pkt_addr), 188);
            if (mutex_lock_interruptible(&dmx->io_mutex))
               continue;
            dmx_ts_service_parse(dmx, ts_pkt_inf);
        
            mutex_unlock(&dmx->io_mutex);
            /* Move hardware rd pointer. */
            dmx->ts_buf_rd_idx++;

            if (dmx->ts_buf_rd_idx >= dmx->ts_buf_end_idx)
            {
                dmx->ts_buf_rd_idx = 0;
            }

            dmx->pkt_total_in_cnt++;
        }
        dmx_detect_bitrate(dmx);

        if (0 == dmx_hw_buf_overflow_proc(dmx))
        {
            dmx->pfunc.dmx_hw_buf_rd_set(dmx->base_addr, dmx->ts_buf_rd_idx);
        }
        
    }

    return;
}




void dmx_from_hw_cleanup
(
    struct dmx_device *dmx,
    __u32              error_code
)
{
    DMX_PLAYBACK_DEBUG("%s\n", __FUNCTION__);
    __u8 i;
    
    if (0x1 != (error_code & 0x1))
    {
		free_irq(dmx->irq_num, NULL);
    }

    if (0x2 != (error_code & 0x2))
    {
		dmx->pfunc.dmx_hw_reg_reset(dmx->base_addr, dmx->ts_buf_start_addr, dmx->ts_buf_end_idx);
    }

    /* Close playback out device. */
    if (0x4 != (error_code & 0x4))
    {
        cdev_del(&dmx->cdev);
    }

    if (0x8 != (error_code & 0x8))
    {
        unregister_chrdev_region(dmx->dev_id, 1);
    }

    /* Destroy workqueue. */
    if (0x10 != (error_code & 0x10))
    {
        flush_workqueue(dmx->workqueue);

    	destroy_workqueue(dmx->workqueue);
	}

    /* Release memory. */
    if (0x20 != (error_code & 0x20))
    {
        dmx_data_pool_release(dmx);
    }

    /* Free memory used for track hw buffers. */
    if (0x40 != (error_code & 0x40))
    {
        kfree(dmx->pkt_inf);
    }

    if (0x80 != (error_code & 0x80))
    {
        sysfs_remove_group(dmx->sysfs_dir_kobj, &dmx->sysfs_attr_group);
    }

    /* Release "status" dir in sysfs. */
    if (0x100 != (error_code & 0x100))
    {
        kobject_put(dmx->sysfs_dir_kobj);
    }

    /* Distroy sysfs entry. */
    if (0x200 != (error_code & 0x200))
    {
        device_destroy(g_ali_m36_dmx_class, dmx->dev_id);
    }
    if (0x400 != (error_code & 0x400))
    {
        for(i=0;i<MAX_CA_RECORD_CHANNEL;i++)
        {
            if(dmx->enc_buffer[i]!= NULL)
            {
                kfree(dmx->enc_buffer[i]);
                dmx->enc_buffer[i] = NULL;
            }
            if(dmx->dst_buffer[i]!= NULL)
            {
                kfree(dmx->dst_buffer[i]);
                dmx->dst_buffer[i] = NULL;
            }
        }
    }
    

    return;
}

DMX_INT32 dmx_form_hw_setup
(
    struct dmx_device      *dmx,
    DMX_UINT8              *name,
    struct class           *ali_m36_dmx_class,
    struct file_operations *fops, 
    DMX_UINT32              hw_reg_base,
    DMX_UINT32              defined_hw_buf,
    DMX_UINT32              hw_buf_len,
    DMX_UINT32              irq_num
)
{
    DMX_UINT32               i;
    DMX_UINT32               ts_buf_pkt_cnt;
	DMX_INT32                result;
    DMX_UINT32               ts_buf_start;
    DMX_UINT32               physical_ts_buf_start;
    DMX_UINT8               *ts_buf_addr;
    DMX_INT32                err;
	int                      retval;
    struct attribute       **attrs;

    result = 0;

    memset(dmx, 0, sizeof(struct dmx_device));

    if (dmx_data_pool_init(dmx, DMX_DATA_POOL_NODE_SIZE, DMX_DATA_POOL_NODE_CNT)
        < 0)
    {
        dmx_from_hw_cleanup(dmx, 0x0);

        return(-1);
    }

    dmx->dmx_type = DMX_TYPE_HW;

	result = alloc_chrdev_region(&dmx->dev_id, 0, 1, name);

	if (result < 0) 
    {
        DMX_INIT_DEBUG("%s, %d, failed.\n", __FUNCTION__, __LINE__);

        dmx_from_hw_cleanup(dmx, 0x20);

        return(-1);
	}

    DMX_INIT_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

	cdev_init(&(dmx->cdev), fops);

	dmx->cdev.owner = THIS_MODULE;

	err = cdev_add(&dmx->cdev, dmx->dev_id, 1);

	/* Fail gracefully if need be. */
	if (err)
    {
        DMX_INIT_DEBUG("%s, %d, failed.\n", __FUNCTION__, __LINE__);

        dmx_from_hw_cleanup(dmx, 0x20 | 0x8);

        return(-1);
    }

    


    /* Access dmx hw buffer through cached address to improve pefermance. */
    ts_buf_start = defined_hw_buf & 0x8FFFFFFF;

    if (ali_sys_ic_get_chip_id() != ALI_S3503 && ali_sys_ic_get_chip_id() != ALI_C3701 && \ 
        ((ts_buf_start + hw_buf_len - 1) & 0x0F800000) !=
        (ts_buf_start & 0x0F800000))
    {
        printk("%s,%d,faital error! hw buf:0x%x~0x%x,accross 8M boundary!\n",
               __FUNCTION__, __LINE__, ts_buf_start, 
               (ts_buf_start + hw_buf_len - 1));

        dmx_from_hw_cleanup(dmx, 0x20 | 0x8 | 0x4);

        return(-1);
    }

    /* Physical address lies below 512M in MIPS system. */
    physical_ts_buf_start = defined_hw_buf & 0x0FFFFFFF;

    /* Physical address must be below 128M for DMX hw buffer. */
    if (ali_sys_ic_get_chip_id() != ALI_S3503 && ali_sys_ic_get_chip_id() != ALI_C3701 && \ 
        physical_ts_buf_start > 0x08000000)
    {
        DMX_INIT_DEBUG("%s,%d,defined_hw_buf:0x%x larger than 128M! fail.\n",  
                       __FUNCTION__, __LINE__, defined_hw_buf);

        dmx_from_hw_cleanup(dmx, 0x20 | 0x4 | 0x8);

        return(-1);
    }

    printk("%s,%d,ts_buf_start:0x%x,physical_ts_buf_start:0x%x\n", 
           __FUNCTION__, __LINE__, ts_buf_start, physical_ts_buf_start);

    /* Step 2: Init software stuctures. */
    ts_buf_pkt_cnt = hw_buf_len / 188;

    if (ts_buf_pkt_cnt > DMX_HW_MAX_TS_BUF_CNT)
    {
        printk("%s,%d,buf len %d excceed HW capacity, now use %d instead!\n", 
               __FUNCTION__, __LINE__, hw_buf_len, DMX_HW_MAX_TS_BUF_CNT);

        ts_buf_pkt_cnt = DMX_HW_MAX_TS_BUF_CNT;
    }

    /* Allocate memory for hw ts buffer info. */
    dmx->pkt_inf = (struct dmx_ts_pkt_inf *)kmalloc(ts_buf_pkt_cnt *
                    sizeof(struct dmx_ts_pkt_inf), GFP_KERNEL);

    if (NULL == dmx->pkt_inf)
    {
        dmx_from_hw_cleanup(dmx, 0x20 | 0x4 | 0x8);

        return(-1);
    }

    DMX_INIT_DEBUG("priv->pkt_inf: 0x%x, ts_buf_pkt_cnt: 0x%x\n",
                  (DMX_UINT32)dmx->pkt_inf, ts_buf_pkt_cnt);

    ts_buf_addr = (DMX_UINT8 *)ts_buf_start;

    for (i = 0; i < ts_buf_pkt_cnt; i++)
    {
        dmx->pkt_inf[i].pkt_addr = ts_buf_addr;

        ts_buf_addr += 188;
    }

    /* Step 3: Populate private structure. */
    dmx->ts_buf_start_addr = ts_buf_start;

    dmx->ts_buf_end_idx = ts_buf_pkt_cnt;

    if (strlen(name) > sizeof(dmx->name))
    {
        memcpy(dmx->name, name, sizeof(dmx->name));
    }
    else
    {
        memcpy(dmx->name, name, strlen(name));
    }

    DMX_INIT_DEBUG("DMX name: %s\n", dmx->name);

    dmx->base_addr = hw_reg_base;

    /* Init sw services. */
    dmx_ts_service_init(dmx);

    dmx_ts2pes_service_init(dmx);

    dmx_ts2sec_service_init(dmx);

    dmx_pcr_service_init(dmx);

    dmx_channel_init(dmx);

    memset(&dmx->pfunc,0,sizeof(T_DMX_HW_FUNC));
    dmx_hw_get_func(&dmx->pfunc);

    dmx->pfunc.dmx_hw_reg_reset(hw_reg_base, dmx->ts_buf_start_addr, dmx->ts_buf_end_idx);
    /* Register ISR. */
	result = request_irq(irq_num, dmx->pfunc.dmx_hw_isr, 0, name, (void *)dmx);

	if (result)
    {
		DMX_INIT_DEBUG("ali dmx: request_irq() failed\n");

        dmx_from_hw_cleanup(dmx, 0x20 | 0x4 | 0x8 | 0x40);

        return(-1);
	}

    dmx->irq_num = irq_num;

    /* Step 4: Init hw regs. */

    /* Step 5: Create Mutex. */
	mutex_init(&(dmx->io_mutex));

    /* Step 6: Init TS retrieving task. */
	dmx->workqueue = create_workqueue(name);

	if (!(dmx->workqueue))
    {
		DMX_INIT_DEBUG("Failed to allocate work queue\n");

        dmx_from_hw_cleanup(dmx, 0x20 | 0x4 | 0x8 | 0x40 | 0x1);

        return(-1);
	}

	INIT_WORK(&dmx->work, dmx_hw_parse_task);

	queue_work(dmx->workqueue, &dmx->work);

	dmx->device = device_create(ali_m36_dmx_class, NULL, dmx->dev_id, 
                                dmx, name);

	if (IS_ERR(dmx->device))
    {
		DMX_INIT_DEBUG(KERN_ERR "device_create() failed!\n");

		result = PTR_ERR(dmx->device);

        dmx_from_hw_cleanup(dmx, 0x20 | 0x4 | 0x8 | 0x40 | 0x1 | 0x10);

        return(-1);
	}

    dmx->attr_ts_in_cnt.attr.mode = S_IRUGO;

    dmx->attr_ts_in_cnt.attr.name = "global";

    dmx->attr_ts_in_cnt.show = dmx_global_stat_show;

    dmx->attr_ts_in_cnt.store = NULL;


    dmx->attr_ts_services.attr.mode = S_IRUGO;

    dmx->attr_ts_services.attr.name = "ts_services";

    dmx->attr_ts_services.show = dmx_ts_services_show;

    dmx->attr_ts_services.store = NULL;


    attrs = kmalloc(GFP_KERNEL, sizeof(*attrs) * 12);

    if (NULL == attrs)
    {
        DMX_INIT_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

        dmx_from_hw_cleanup(dmx, 0x20 | 0x4 | 0x8 | 0x40 | 0x1 | 0x10);

        return(-1);
    }

    memset(attrs, 0, sizeof(*attrs) * 12);

    attrs[0] = &(dmx->attr_ts_in_cnt.attr);

    attrs[1] = &(dmx->attr_ts_services.attr);

    attrs[2] = NULL;

    retval = dmx_sysfs_entry_create(dmx, attrs);

	if (retval)
    {
		dev_warn(dmx->device, "create sysfs group err: %d\n", retval);

        kfree(attrs);

        dmx_from_hw_cleanup(dmx, 0x20 | 0x4 | 0x8 | 0x40 | 0x1 | 0x10);

        return(-1);
    }
    for(i=0;i<MAX_CA_RECORD_CHANNEL;i++)
    {
        dmx->enc_buffer[i] = NULL;
        dmx->enc_buffer[i] = kmalloc(DMX_ENC_BLK_LEN, GFP_KERNEL);
        if(dmx->enc_buffer[i] == NULL)
        {
             dmx_from_hw_cleanup(dmx, 0x20 | 0x4 | 0x8 | 0x40 | 0x1 | 0x10);
             return(-1);
        }
    }

    for(i=0;i<MAX_CA_RECORD_CHANNEL;i++)
    {
        dmx->dst_buffer[i] = NULL;
        dmx->dst_buffer[i] = kmalloc(DMX_ENC_BLK_LEN, GFP_KERNEL);
        if(dmx->dst_buffer[i] == NULL)
        {
             dmx_from_hw_cleanup(dmx, 0x20 | 0x4 | 0x8 | 0x40 | 0x1 | 0x10 | 0x400 );
             return(-1);
        }
    }

    /* Step 9:  Succeed */
	return(0); 
}




static int  see_disable = 0;

static int dmx_from_hw_suspend(struct device *dev, pm_message_t state)
{    
    struct dmx_device     *dmx = dev_get_drvdata(dev);    

#if 0
    extern UINT32 sed_standby(UINT32 status);

    if (!see_disable)
    {
        sed_standby(1);
        see_disable = 1;
        mdelay(100);
    }
#endif

    return 0;
}

static int dmx_from_hw_resume(struct device *dev)
{    
    struct dmx_device     *dmx = dev_get_drvdata(dev);

#if 0
    extern UINT32 sed_standby(UINT32 status);

    if (see_disable)
    {
        sed_standby(0);
        see_disable = 0;
    }
#endif

    return 0;
}

static int __init dmx_from_hw_init(void)
{
    DMX_UINT32 hw_buf_len;
    DMX_INT32  ret;
    struct dmx_enc_info *enc_info = &g_ali_dmx_enc_info;
    int i;
	dmx_mem_attr_t *p_dmx_mem_attr = (dmx_mem_attr_t *)request_attr(DMX_MEM);
    
    hw_buf_len = (p_dmx_mem_attr->dmx_mem_end_addr - p_dmx_mem_attr->dmx_mem_start_addr) / 2;

    /* Step 7: create dev file in /dev. */
	g_ali_m36_dmx_class = class_create(THIS_MODULE, "ali_m36_dmx_class");

	if (IS_ERR(g_ali_m36_dmx_class))
    {
		PTR_ERR(g_ali_m36_dmx_class);

        DMX_INIT_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

		return(-1);
	}

    ret = dmx_form_hw_setup(&(g_ali_dmx_devices[0]),
                            "ali_m36_dmx_0",
                            g_ali_m36_dmx_class,  
                            &g_ali_m36_dmx_fops,
                            DMX0_BASE_ADDR,
                            p_dmx_mem_attr->dmx_mem_start_addr,
                            hw_buf_len,
                            M36_IRQ_DMX1);

    if (ret != 0)
    {
        class_destroy(g_ali_m36_dmx_class);

        DMX_INIT_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

		return(-1);
    }

    ret = dmx_form_hw_setup(&(g_ali_dmx_devices[1]),
                            "ali_m36_dmx_1",
                            g_ali_m36_dmx_class,  
                            &g_ali_m36_dmx_fops,
                            DMX1_BASE_ADDR,
                            p_dmx_mem_attr->dmx_mem_start_addr+ hw_buf_len,
                            hw_buf_len,
                            M36_IRQ_DMX2);

    if (ret != 0)
    {
        dmx_from_hw_cleanup(&(g_ali_dmx_devices[0]), 0xFFFFFFFF);

        class_destroy(g_ali_m36_dmx_class);

        DMX_INIT_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

		return(-1);
    }

    enc_info->ca_workqueue = create_workqueue("ali_m36_ca");
    if (!enc_info->ca_workqueue)
    {
        return(-1);
    }
    
    INIT_WORK(&enc_info->ca_work, dmx_ca_record_task);
    
    queue_work(enc_info->ca_workqueue, &enc_info->ca_work);
	
    for(i = 0; i < MAX_CA_RECORD_CHANNEL; i++)
        enc_info->channel[i] = NULL;
    
    mutex_init(&(enc_info->io_mutex));                		
    DMX_INIT_DEBUG("%s, %d, failed.\n", __FUNCTION__, __LINE__);

    g_ali_m36_dmx_class->suspend = dmx_from_hw_suspend;
    g_ali_m36_dmx_class->resume = dmx_from_hw_resume;

    return 0;
}

static void __exit dmx_from_hw_exit(void)
{
    struct dmx_device *dmx;

    DMX_INIT_DEBUG(KERN_ALERT "Goodbye, cruel world\n");

    dmx = &(g_ali_dmx_devices[0]);

    dmx_from_hw_cleanup(dmx, 0xFFFFFFFF);

    dmx = &(g_ali_dmx_devices[1]);

    dmx_from_hw_cleanup(dmx, 0xFFFFFFFF);

    return;
}


module_init(dmx_from_hw_init);
//device_initcall_sync(ali_m3602_dmx_init);
module_exit(dmx_from_hw_exit);

