
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/features.h>

//#include <linux/vmalloc.h>

#include <asm/io.h>
//#include <asm/mach-m3602/m3602.h>
#include <ali_interrupt.h>

//#include <linux/ali_video.h>



#include <linux/semaphore.h>

#include <linux/wait.h>

#include <linux/poll.h>

#include <linux/delay.h>


#include "dmx_internal.h"

#include <linux/dvb/ali_dsc.h>

extern struct dmx_enc_info g_ali_dmx_enc_info;
unsigned long ali_dmx_m3501_autoscan = 0;

#define VIRTUAL_QUANTUM_SIZE 256*188
extern void *fcp_get_quantum(int,int);
extern void fcp_put_quantum(int);

#define SW_TYPE_PROJECT			0x00100000	/* Project */
#define PROJECT_FE_DVBS			(SW_TYPE_PROJECT + 0x101)
#define PROJECT_FE_DVBS2		(SW_TYPE_PROJECT + 0x105)

static UINT32 fe_mismatch = FALSE;
static UINT32 system_frond_end = SW_TYPE_PROJECT;

void dmx_check_front_end(UINT32 fe,UINT32 nim_chip_id)
{

	if(fe_mismatch)
		return;
	
	if(fe == PROJECT_FE_DVBS || fe == PROJECT_FE_DVBS2)
	{
		system_frond_end = PROJECT_FE_DVBS2;
	}
	if(((*(UINT32*)0xb8000004)&0xff) == 1)
	{
		system_frond_end = PROJECT_FE_DVBS2;
	}
	
	if(system_frond_end == PROJECT_FE_DVBS2)
	{
		// check frond end as 3501
		
		//NIM_CHIP_ID_M3501A or NIM_CHIP_ID_M3501B
		if((nim_chip_id) != 0x3501)
		{
			fe_mismatch = TRUE;
		}		
	}
}

BOOL dmx_api_check_fe_mismatch()
{
	return fe_mismatch;
}

/* For user space, blocks until get data, one section per read. */
int dmx_channel_usr_rd_pkt
(
    struct file *file,
    char __user *usr_buf,
    size_t       usr_rd_len,
    loff_t      *ppos
)
{
    __s32                    ret;
    struct dmx_channel      *channel; 
    struct ali_dmx_data_buf *src_buf;
    struct dmx_device       *dmx;

    if (0 >= usr_rd_len)
    {
        DMX_LINUX_API_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }    

    //DMX_LINUX_API_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    channel = file->private_data;

    dmx = (struct dmx_device *)channel->dmx;

    //sec_ch_info = &(channel->sec_ch_info);

    //src_buf = &(sec_ch_info->data_buf);

    src_buf = &channel->data_buf;

    ret = dmx_data_buf_usr_rd_pkt(dmx, file, src_buf, usr_buf, usr_rd_len);

    return(ret);
}


int dmx_channel_usr_rd_data
(
    struct file *file,
    char __user *usr_buf,
    size_t       usr_rd_len,
    loff_t      *ppos
)
{
    __s32                       len;
    struct dmx_channel         *channel; 
    struct ali_dmx_data_buf    *src_buf;
    struct dmx_device          *dmx;
    struct dmx_ts_channel_info   *ts_ch_info;
    size_t cpy_len;
    
    struct dmx_enc_info *enc_info = &g_ali_dmx_enc_info;
    DMX_UINT32 dst_blk_wr;
    DMX_UINT32 dst_blk_rd; 
    DMX_UINT32 dst_blk_len;
    if (0 == usr_rd_len)
    {
        DMX_LINUX_API_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }    

    //DMX_LINUX_API_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    channel = file->private_data;

    if(channel->status != DMX_CHANNEL_STATUS_RUN)
    {
        printk("%s, channel not run, channel_status: %d\n", __FUNCTION__, channel->status);
        return -1;
    }
    ts_ch_info = &channel->ts_ch_info;

    
    if(ts_ch_info->enc_para != NULL)
    {

        
        mutex_lock_interruptible(&enc_info->io_mutex);
        dst_blk_wr = ts_ch_info->dst_blk_wr;
        dst_blk_rd = ts_ch_info->dst_blk_rd;
        dst_blk_len = ts_ch_info->dst_blk_len;
        mutex_unlock(&enc_info->io_mutex);          

        cpy_len = (dst_blk_len <= usr_rd_len)?dst_blk_len:usr_rd_len;

		if (feature_is_fastcopy_enable())
		{
			void *qbuf = (void *)usr_buf;
			int  quantum_id = (int)(((__u32)usr_buf>>16) & 0xffff);
	        int  offset = (int)((__u32)usr_buf & 0xffff);
        
		    src_buf = &(channel->data_buf);
			if(src_buf->fst_cpy_slot >= 0)
	        {
		        qbuf = fcp_get_quantum(quantum_id, 0) + offset; 
			    memcpy(qbuf+src_buf->fst_cpy_size, 
					   (ts_ch_info->dst_blk + dst_blk_rd), cpy_len);
	            src_buf->fst_cpy_size += cpy_len;

               if(src_buf->fst_cpy_size >= VIRTUAL_QUANTUM_SIZE)
			    {
				    fcp_put_quantum(quantum_id);
					src_buf->fst_cpy_size = 0;
	            }
		    }else
			{
				if (copy_to_user(usr_buf, 
							    (ts_ch_info->dst_blk + dst_blk_rd), 
								cpy_len))
	            {      
		            return(-EFAULT);     
			    } 
			}
		} else {           		
			if (copy_to_user(usr_buf, 
						     (ts_ch_info->dst_blk + dst_blk_rd), 
							 cpy_len))
	        {      
		        return(-EFAULT);     
			}
		}
        mutex_lock_interruptible(&enc_info->io_mutex);

        ts_ch_info->dst_blk_rd = (dst_blk_rd + cpy_len)%DMX_ENC_BLK_LEN;
                    	
        ts_ch_info->dst_blk_len -= cpy_len;
        mutex_unlock(&enc_info->io_mutex);          
        return(cpy_len); 
    }
          
    


    src_buf = &(channel->data_buf);

    dmx = (struct dmx_device *)channel->dmx;

#if 1
    len = dmx_data_buf_usr_rd_data(dmx, file, src_buf, usr_buf, usr_rd_len);
#else
    if (1 == channel->ts_ch_info.mepg2_startcode_en)
    {
        if (usr_rd_len > DMX_SGDMA_STARTCODE_TMP_BUF_LEN)
        {
            printk("%s,%d,rd len:%d >", __FUNCTION__, __LINE__);

            printk("DMX_SGDMA_STARTCODE_TMP_BUF_LEN, pls enlarge");

            printk("DMX_SGDMA_STARTCODE_TMP_BUF_LEN or reduce read len.\n");
        }

        sc_buf = dmx->startcode_buf;

        len = dmx_data_buf_usr_rd_data(dmx, file, src_buf, sc_buf, usr_rd_len);

        A_SgdmaChCpy(sc_buf, src_buf, len);

        A_SgdmaChFlush();
    }
#endif

    return(len);
}








__inline DMX_INT32 dmx_channel_ts_kern_wr
(
    struct dmx_device     *dmx,
    struct dmx_ts_pkt_inf *pkt_inf,
    DMX_UINT32             param
)
{
    __s32 byte_cnt;
    __s32 pkt_len;
    struct ali_dmx_data_buf *dest_buf;
    struct dmx_channel *channel;
    struct dmx_ts_channel_info   *ts_ch_info;
    DMX_UINT8 *enc_blk;
    DMX_UINT32 enc_blk_wr;
    struct dmx_enc_info *enc_info = &g_ali_dmx_enc_info;
    
    channel = (struct dmx_channel *)param;
    
    ts_ch_info = &channel->ts_ch_info;
    
    if(ts_ch_info->enc_para != NULL)
    {
        if(ts_ch_info->enc_blk == NULL)
            ts_ch_info->enc_blk = dmx->enc_buffer[ts_ch_info->ca_channel_id];
        if(ts_ch_info->dst_blk == NULL)
            ts_ch_info->dst_blk = dmx->dst_buffer[ts_ch_info->ca_channel_id];

        if(ts_ch_info->enc_blk == NULL||ts_ch_info->dst_blk == NULL)
    	      return 188;

        mutex_lock_interruptible(&enc_info->io_mutex);
    	  /*Write into enc buffer*/
    	  enc_blk = ts_ch_info->enc_blk;
    	  enc_blk_wr = ts_ch_info->enc_blk_wr + 188;
    	  if(enc_blk_wr >= DMX_ENC_BLK_LEN) enc_blk_wr = 0;
    	  
    	  if(enc_blk_wr == ts_ch_info->enc_blk_rd)
    	  {
    	  	  mutex_unlock(&enc_info->io_mutex);
    	      return 188;
    	  }   
    	  memcpy(((DMX_UINT8 *)enc_blk) + ts_ch_info->enc_blk_wr, pkt_inf->pkt_addr, 188);
    	  ts_ch_info->enc_blk_wr = enc_blk_wr;
    	  ts_ch_info->enc_blk_len += 188;
    	  mutex_unlock(&enc_info->io_mutex);
          return 188;
  	}
    dest_buf = &(channel->data_buf);

    byte_cnt = dmx_data_buf_kern_wr_data(dmx, dest_buf, pkt_inf->pkt_addr, 188);

    /* Buffer overflow. */
    if (188 > byte_cnt)
    {
        DMX_LINUX_API_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        dmx_data_buf_kern_flush_all_pkt(dmx, dest_buf);
    }

    pkt_len = dmx_data_buf_first_pkt_len(dmx, dest_buf);

    if (pkt_len >= 0xBC00 /* 47K */)
    {
        dmx_data_buf_kern_wr_pkt_end(dmx, dest_buf);
    }

    return(byte_cnt);
}


void dmx_int_rec_all_ch_info
(
    struct dmx_rec_all_channel_info *pRec_all_ch_info,
    int fst_cpy_slot

)
{
    pRec_all_ch_info->rec_whole_tp_rd = 0;
    pRec_all_ch_info->rec_whole_tp_wd = 0;
    pRec_all_ch_info->fst_cpy_slot = fst_cpy_slot;
    pRec_all_ch_info->fst_cpy_size = 0;
    pRec_all_ch_info->exp_cpy_size = 0;
    return;
}

DMX_INT32 dmx_get_rec_all_valid_data_len
(
    struct dmx_device  *dmx,
    struct dmx_channel *channel
)
{
        struct dmx_channel_param *usr_para;
        struct dmx_rec_all_channel_info *rec_all_ch_info;
    
        int   nDataInBuffer = 0;
    
        usr_para = &channel->usr_param;

        rec_all_ch_info = &channel->rec_all_ch_info;
        rec_all_ch_info->rec_whole_tp_wd = dmx->pfunc.dmx_hw_buf_wr_get(dmx->base_addr);
        
        if (rec_all_ch_info->rec_whole_tp_wd == rec_all_ch_info->rec_whole_tp_rd)
        {
            return 0;
        }
        
        if(rec_all_ch_info->rec_whole_tp_wd>rec_all_ch_info->rec_whole_tp_rd)
        {
            nDataInBuffer = (rec_all_ch_info->rec_whole_tp_wd-rec_all_ch_info->rec_whole_tp_rd)*188;
        }
        else
        {
            nDataInBuffer = (dmx->ts_buf_end_idx - rec_all_ch_info->rec_whole_tp_rd + rec_all_ch_info->rec_whole_tp_wd)*188;
            
            
        }

        return nDataInBuffer;
}

DMX_INT32 dmx_channel_stop
(
    struct dmx_device  *dmx,
    struct dmx_channel *channel
)
{
    DMX_UINT32                   i;
    struct dmx_ts_channel_info  *ts_ch_info;
    struct dmx_sec_channel_info *sec_ch_info;
    struct dmx_pes_channel_info *pes_ch_info;
    struct dmx_channel_param    *usr_para;
    struct dmx_enc_info *enc_info = &g_ali_dmx_enc_info;
    
    DMX_LINUX_API_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    /* Channel startus validation, return success in all status. */
    if (channel->status != DMX_CHANNEL_STATUS_RUN)
    {
        return(0);
    }

    usr_para = &channel->usr_param;

    switch(usr_para->output_format)
    {
        case DMX_CHANNEL_OUTPUT_FORMAT_TS:
        {
            ts_ch_info = &channel->ts_ch_info;

            for (i = 0; i < usr_para->ts_param.pid_list_len; i++) 
            {
                dmx_ts_service_unregister(dmx, ts_ch_info->ts_serv_id[i]);
            }

            if (DMX_OUTPUT_SPACE_USER == usr_para->output_space)
            {
                dmx_data_buf_kern_flush_all_pkt(dmx, &channel->data_buf);
                if(usr_para->rec_whole_tp == TRUE)
                {
                    usr_para->rec_whole_tp = FALSE;
                    dmx_int_rec_all_ch_info(&(channel->rec_all_ch_info),-1);
                }
            }
            
            if(ts_ch_info->enc_para != NULL)
            {
                ts_ch_info->status = ALI_DMX_CA_RECORD_PAUSE;
                while(ts_ch_info->status != ALI_DMX_CA_RECORD_PAUSED)
                {
                    msleep(3);
                }
            	  mutex_lock_interruptible(&enc_info->io_mutex);
            	  ts_ch_info->enc_blk_len = ts_ch_info->dst_blk_len = 0; 
            	  ts_ch_info->enc_blk_wr = ts_ch_info->dst_blk_wr = 0;
                ts_ch_info->enc_blk_rd = ts_ch_info->dst_blk_rd = 0;
                for(i = 0; i < MAX_CA_RECORD_CHANNEL; i++)
                    if(enc_info->channel[i] == channel)
                    	 enc_info->channel[i] = NULL;

                if(ts_ch_info->enc_blk != NULL)
                    ts_ch_info->enc_blk = NULL;

                if(ts_ch_info->dst_blk != NULL)
                    ts_ch_info->dst_blk = NULL;
                
                kfree(ts_ch_info->enc_para);
                ts_ch_info->enc_para = NULL;
                mutex_unlock(&enc_info->io_mutex);
                ts_ch_info->status = ALI_DMX_CA_RECORD_BUSY;
            }
            
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_PES:
        {
            if (DMX_OUTPUT_SPACE_USER == usr_para->output_space)
            {
                pes_ch_info = &channel->pes_ch_info;
    
                dmx_ts2pes_service_unregister(dmx, pes_ch_info->pes_serv_id);
    
                dmx_data_buf_kern_flush_all_pkt(dmx, &channel->data_buf);
            }
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_SEC:
        {
            if (DMX_OUTPUT_SPACE_USER == usr_para->output_space)
            {
                sec_ch_info = &channel->sec_ch_info;
    
                dmx_ts2sec_service_unregister(dmx, sec_ch_info->sec_serv_id);
    
                dmx_data_buf_kern_flush_all_pkt(dmx, &channel->data_buf);
            }
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_PCR:
        {
            if (DMX_OUTPUT_SPACE_KERNEL == usr_para->output_space)
            {
                sec_ch_info = &channel->sec_ch_info;
    
                dmx_pcr_service_unregister(dmx, sec_ch_info->sec_serv_id);
            }
        }
        break;
        default:
        {
            DMX_LINUX_API_DEBUG("Invalid output_format! return fail.\n");

            return(-EINVAL);
        }
        break;
    }

    channel->status = DMX_CHANNEL_STATUS_READY;

    return(0);
}






DMX_INT32 dmx_channel_start
(
    struct dmx_device  *dmx,
    struct dmx_channel *channel,
    unsigned int        cmd,
    unsigned long       arg
)
{
    struct dmx_ts_channel_info   *ts_ch_info;
    struct dmx_sec_channel_info  *sec_ch_info;
    struct dmx_pes_channel_info  *pes_ch_info;
    struct dmx_rec_all_channel_info *rec_all_ch_info;
    struct dmx_channel_param     *usr_para;
    int                           ret;
    DMX_UINT32                    i;
    struct dmx_ts_kern_recv_info *ts_kern_recv_info;
    struct ali_dmx_data_buf      *data_buf;
	struct dmx_pcr_channel_info  *pcr_ch_info;
    struct dmx_enc_info *enc_info = &g_ali_dmx_enc_info;
    
    /* Channel startus validation. */
    if (channel->status != DMX_CHANNEL_STATUS_READY)
    {
        return(-EPERM);
    }

    ret = copy_from_user(&channel->usr_param, (void __user *)arg,
                         _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    usr_para = &channel->usr_param;

    data_buf = &(channel->data_buf);

    switch(usr_para->output_format)
    {
        case DMX_CHANNEL_OUTPUT_FORMAT_TS:
        {
            DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);

            ts_ch_info = &channel->ts_ch_info;
            rec_all_ch_info = &channel->rec_all_ch_info;
            memset(ts_ch_info, 0, sizeof(struct dmx_ts_channel_info));

            if (DMX_OUTPUT_SPACE_KERNEL == usr_para->output_space)
            {

//                dmx_check_front_end(usr_para->fe,usr_para->nim_chip_id);
                ts_kern_recv_info = &(usr_para->ts_param.kern_recv_info);

                DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);

                for (i = 0; i < usr_para->ts_param.pid_list_len; i++)
                {
                    ts_ch_info->ts_serv_id[i] = dmx_ts_service_register(dmx,
                        usr_para->ts_param.pid_list[i] & 0x1FFF, 
                        (ts_serv_recv_func)ts_kern_recv_info->kern_recv_routine,
                        ts_kern_recv_info->kern_recv_routine_para);
            
                    if (DMX_INVALID_IDX == ts_ch_info->ts_serv_id[i])
                    {
                        DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);
        
                        dmx_channel_stop(dmx, channel);
    
                        return(-EBUSY);
                    }
                }
            }
            else if (DMX_OUTPUT_SPACE_USER == usr_para->output_space)
            {
                DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);

                dmx_data_buf_setup(dmx, data_buf, 0x100000, 0x1000); /* 1M, 4K */
                data_buf->fst_cpy_slot = usr_para->fst_cpy_slot;
                if(usr_para->enc_para != NULL)
                {
                    //ts_ch_info->enc_para = usr_para->enc_para;
                    ts_ch_info->enc_para = kmalloc(sizeof(DEEN_CONFIG), GFP_KERNEL);
                    if(NULL == ts_ch_info->enc_para)
                        return(-EFAULT);
                    if (0 != copy_from_user(ts_ch_info->enc_para, (void __user *)(usr_para->enc_para), sizeof(DEEN_CONFIG)))
                    {
                        return(-EFAULT);
                    }       
                    ts_ch_info->enc_blk_wr = ts_ch_info->dst_blk_wr = 0;
                    ts_ch_info->enc_blk_rd = ts_ch_info->dst_blk_rd = 0;
                    ts_ch_info->enc_blk = ts_ch_info->dst_blk = NULL;
                    for(i = 0; i < MAX_CA_RECORD_CHANNEL; i++)
                        if(enc_info->channel[i] == NULL)
                        {
                        	 enc_info->channel[i] = channel;
                             ts_ch_info->ca_channel_id = i;
                             
                             break;
                        }
                    if(i == MAX_CA_RECORD_CHANNEL)
                        return (-EBUSY);
                }    
                for (i = 0; i < usr_para->ts_param.pid_list_len; i++)
                {
                    ts_ch_info->ts_serv_id[i] = dmx_ts_service_register(
                        dmx,
                        usr_para->ts_param.pid_list[i] & 0x1FFF, 
                        (ts_serv_recv_func)dmx_channel_ts_kern_wr,
                        (DMX_UINT32)channel);

                    if (DMX_INVALID_IDX == ts_ch_info->ts_serv_id[i])
                    {
                        DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);
        
                        dmx_channel_stop(dmx, channel);
    
                        return(-EBUSY);
                    }
                }
                if(usr_para->rec_whole_tp == TRUE)
                {
                    dmx_int_rec_all_ch_info(rec_all_ch_info,usr_para->fst_cpy_slot);
                }
            }
            else 
            {
                DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);

                dmx_channel_stop(dmx, channel);

                return(-EFAULT);
            }

            for (i = 0; i < usr_para->ts_param.pid_list_len; i++) 
            {
                dmx_ts_service_enable(dmx, ts_ch_info->ts_serv_id[i]);
            }
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_PES:
        { 
            DMX_LINUX_API_DEBUG("%s,line:%d,pid:0x%x\n",  __FUNCTION__, 
                                __LINE__, usr_para->pes_param.pid);

            pes_ch_info = &channel->pes_ch_info;

            if (DMX_OUTPUT_SPACE_USER == usr_para->output_space)
            {
                DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);

                dmx_data_buf_setup(dmx, data_buf, 0x80000, 0x1000); /* 512K, 4K */

                pes_ch_info->pes_serv_id = dmx_ts2pes_service_register(dmx, 
                                               usr_para->pes_param.pid & 0x1FFF,  
                                              (DMX_UINT32)(&channel->data_buf));

                if (DMX_INVALID_IDX == pes_ch_info->pes_serv_id)
                {
                    DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);
    
                    dmx_channel_stop(dmx, channel);

                    return(-EBUSY);
                }
            }
            else 
            {
                DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);

                dmx_channel_stop(dmx, channel);

                return(-EFAULT);
            }

            dmx_ts2pes_service_enable(dmx, pes_ch_info->pes_serv_id);
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_SEC:
        {
            DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);

            sec_ch_info = &channel->sec_ch_info;

            memset(sec_ch_info, 0, sizeof(struct dmx_sec_channel_info));

            if (DMX_OUTPUT_SPACE_USER == usr_para->output_space)
            {
                DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);

                dmx_data_buf_setup(dmx, data_buf, 0x80000, 0x1000); /* 512K, 4K */

                sec_ch_info->sec_serv_id = dmx_ts2sec_service_register(dmx, 
                                               usr_para->sec_param.pid & 0x1FFF, 
                                               usr_para->sec_param.mask_len,
                                               usr_para->sec_param.mask, 
                                               usr_para->sec_param.value,
                                               (DMX_UINT32)data_buf);
            
                if (DMX_INVALID_IDX == sec_ch_info->sec_serv_id)
                {
                    DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);
    
                    dmx_channel_stop(dmx, channel);
    
                    return(-EFAULT);
                }

                dmx_ts2sec_service_enable(dmx, sec_ch_info->sec_serv_id);
            }
            else
            {
                dmx_channel_stop(dmx, channel);

                DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);

                return(-EINVAL);
            }
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_PCR:
        {
            //printk("%s, line:%d\n", __FUNCTION__, __LINE__);

            pcr_ch_info = &channel->pcr_ch_info;

            memset(pcr_ch_info, 0, sizeof(struct dmx_pcr_channel_info));

            if (DMX_OUTPUT_SPACE_KERNEL == usr_para->output_space)
            {
                pcr_ch_info->pcr_serv_id = dmx_pcr_service_register(dmx, usr_para->pcr_param.pid);

                if (DMX_INVALID_IDX == pcr_ch_info->pcr_serv_id)
                {
                    DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);
    
                    dmx_channel_stop(dmx, channel);
    
                    return(-EFAULT);
                }		
				
                dmx_pcr_service_enable(dmx, pcr_ch_info->pcr_serv_id);
            }            
            else 
            {
                DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);

                dmx_channel_stop(dmx, channel);

                return(-EFAULT);
            }
        }
        break;

        default:
        {
            return(-EINVAL);
        }
        break;
    }

    channel->status = DMX_CHANNEL_STATUS_RUN;

    return(0);
}







DMX_INT32 dmx_data_buf_get_pkt_len
(
    struct dmx_device  *dmx,
    struct dmx_channel *channel,
    unsigned int        cmd,
    unsigned long       arg
)
{
    __s32                        ret;
    __u32                        len;
    struct dmx_channel_param    *usr_para;

    DMX_LINUX_API_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    /* Channel startus validation, return success in all status. */
    if (channel->status == DMX_CHANNEL_STATUS_IDLE)
    {
        return(0);
    }

    ret = 0;

    usr_para = &channel->usr_param;

    switch(usr_para->output_format)
    {
        case DMX_CHANNEL_OUTPUT_FORMAT_SEC:
        case DMX_CHANNEL_OUTPUT_FORMAT_PES:
        {
            //if (DMX_OUTPUT_SPACE_USER == usr_para->output_space)
            //{
            len = dmx_data_buf_first_pkt_len(dmx, &channel->data_buf);

            //printk("len:%d\n", len);
            //}
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_TS:
        {
            //if (DMX_OUTPUT_SPACE_USER == usr_para->output_space)
            //{
            struct dmx_ts_channel_info   *ts_ch_info = &channel->ts_ch_info;
            if(ts_ch_info->enc_para != NULL)
                len = ts_ch_info->dst_blk_len;
            else
                len = dmx_data_buf_len(dmx, &channel->data_buf);
            //}
        }
        break;

        default:
        {
            ret = -EINVAL;
        }
        break;
    }

    if (0 == ret)
    {
        if (len > 0)
        {
           // printk("len 2:%d\n", len);
    
            ret = copy_to_user((void __user *)arg, &len, _IOC_SIZE(cmd));
        
            if (0 != ret)
            {
                ret = -ENOTTY;
            }
        }
        else
        {
            ret = -EAGAIN;
        }
    }

    return(ret);
}






DMX_INT32 dmx_hw_get_free_buf_len
(
    struct dmx_device  *dmx,
    struct dmx_channel *channel,
    unsigned int        cmd,
    unsigned long       arg
)
{
    DMX_INT32 pkt_cnt;
    DMX_INT32 ret;
    DMX_UINT32         rd_idx;
    DMX_UINT32         wr_idx;

    ret = 0;

    DMX_LINUX_API_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    /* Channel startus validation, return success in all status. */
    if (channel->status == DMX_CHANNEL_STATUS_IDLE)
    {
        return(0);
    }

#if 0
    if (dmx->ts_buf_wr_idx >= dmx->ts_buf_rd_idx)
    {
        pkt_cnt = dmx->ts_buf_end_idx - dmx->ts_buf_wr_idx +
                  dmx->ts_buf_rd_idx - 1;
    }
    else
    {
        pkt_cnt = dmx->ts_buf_rd_idx - dmx->ts_buf_wr_idx - 1;
    }
#else
    //wr_idx = DMX_HW_TS_BUF_GET_WR_PTR(dmx->base_addr);
    wr_idx = dmx->pfunc.dmx_hw_buf_wr_get(dmx->base_addr);

    //rd_idx = DMX_HW_TS_BUF_GET_RD_PTR(dmx->base_addr);
    rd_idx = dmx->pfunc.dmx_hw_buf_rd_get(dmx->base_addr);

    if (wr_idx >= rd_idx)
    {
        pkt_cnt = dmx->ts_buf_end_idx - wr_idx + rd_idx - 1;
    }
    else
    {
        pkt_cnt = rd_idx - wr_idx - 1;
    }
#endif

    ret = copy_to_user((void __user *)arg, &pkt_cnt, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        ret = -ENOTTY;
    }    
    
#if 0
    DMX_LINUX_API_DEBUG("%s,%d,pkt_cnt:%d\n", __FUNCTION__, __LINE__, pkt_cnt);
#else
    //printk("%s,%d,pkt_cnt:%d\n", __FUNCTION__, __LINE__, pkt_cnt);
#endif

    return(ret);
}


ssize_t dmx_channel_recoder_all
(
    struct file *file,
    char __user *buf,
    size_t       count,
    loff_t      *ppos
)
{
    struct dmx_channel       *channel; 
    struct dmx_channel_param *usr_para;
    struct dmx_device        *dmx;
    struct dmx_ts_pkt_inf    *ts_pkt_inf;
    struct dmx_rec_all_channel_info *rec_all_ch_info;

    int                       ret;
    int                       nRemainLen = 0;
    int                       nDataInBuffer = 0;
    int                       nReadLen = 0;
    int                       rd_idx;
    int                       wr_idx;
    int                       next_wr_idx;

    void *qbuf = (void *)buf;
    int  quantum_id = (int)(((__u32)buf>>16) & 0xffff);
    int  offset = (int)((__u32)buf & 0xffff);
    
    channel = file->private_data;

    ret = 0;

    usr_para = &channel->usr_param;
    dmx = (struct dmx_device  *)channel->dmx;
    rec_all_ch_info = &channel->rec_all_ch_info;

    rec_all_ch_info->rec_whole_tp_wd = dmx->pfunc.dmx_hw_buf_wr_get(dmx->base_addr);
    //rec_all_ch_info->rec_whole_tp_rd = DMX_HW_TS_BUF_GET_RD_PTR(dmx->base_addr);

    //printk("R:%d W:%d\n",rec_all_ch_info->rec_whole_tp_rd,rec_all_ch_info->rec_whole_tp_wd);
    
    if (rec_all_ch_info->rec_whole_tp_wd == rec_all_ch_info->rec_whole_tp_rd)
    {
        return 0;
    }
    
    if(rec_all_ch_info->rec_whole_tp_wd>rec_all_ch_info->rec_whole_tp_rd)
    {
        nDataInBuffer = (rec_all_ch_info->rec_whole_tp_wd-rec_all_ch_info->rec_whole_tp_rd)*188;
        if(nDataInBuffer>=count)
        {
            nRemainLen = count;
        }
        else
        {
            nRemainLen = nDataInBuffer;
        }
        ret = nRemainLen;

        ts_pkt_inf = &(dmx->pkt_inf[rec_all_ch_info->rec_whole_tp_rd]);

        dma_cache_inv((DMX_UINT32)(ts_pkt_inf->pkt_addr), nRemainLen);

		if (feature_is_fastcopy_enable())
		{
	        if(rec_all_ch_info->fst_cpy_slot >= 0)
		    {
			    qbuf = fcp_get_quantum(quantum_id, 0) + offset; 
				memcpy(qbuf+rec_all_ch_info->fst_cpy_size, ts_pkt_inf->pkt_addr, nRemainLen);
	            rec_all_ch_info->fst_cpy_size += nRemainLen;
		        if(rec_all_ch_info->fst_cpy_size >= VIRTUAL_QUANTUM_SIZE)
			    {
				    fcp_put_quantum(quantum_id);
					rec_all_ch_info->fst_cpy_size = 0;
	            }
		    }
			else
	        {
		        copy_to_user(buf,ts_pkt_inf->pkt_addr,nRemainLen);
			}
		} else {
	        copy_to_user(buf,ts_pkt_inf->pkt_addr,nRemainLen);
		}

        rec_all_ch_info->rec_whole_tp_rd += (nRemainLen/188);
        if(rec_all_ch_info->rec_whole_tp_rd >= dmx->ts_buf_end_idx)
        {
            rec_all_ch_info->rec_whole_tp_rd = 0;
        }
        //printk("1 rec_whole_tp_rd=%d nRemainLen=%d \r\n",rec_all_ch_info->rec_whole_tp_rd,nRemainLen/188);
    }
    else
    {
        nDataInBuffer = (dmx->ts_buf_end_idx - rec_all_ch_info->rec_whole_tp_rd + rec_all_ch_info->rec_whole_tp_wd)*188;
        
        if(nDataInBuffer>=count)
        {
            nRemainLen = count;
        }
        else
        {
            nRemainLen = nDataInBuffer;
        }
        ret = nRemainLen;

        if((dmx->ts_buf_end_idx - rec_all_ch_info->rec_whole_tp_rd)*188>=nRemainLen)
        {
            nReadLen = nRemainLen;

        }
        else
        {
            nReadLen = (dmx->ts_buf_end_idx - rec_all_ch_info->rec_whole_tp_rd)*188;
        }
                                
        ts_pkt_inf = &(dmx->pkt_inf[rec_all_ch_info->rec_whole_tp_rd]);
        
        dma_cache_inv((DMX_UINT32)(ts_pkt_inf->pkt_addr), nReadLen);

		if (feature_is_fastcopy_enable())
		{
	        if(rec_all_ch_info->fst_cpy_slot >= 0)
		    {
			    qbuf = fcp_get_quantum(quantum_id, 0) + offset; 
				memcpy(qbuf+rec_all_ch_info->fst_cpy_size, ts_pkt_inf->pkt_addr, nReadLen);
	            rec_all_ch_info->fst_cpy_size += nReadLen;
		        if(rec_all_ch_info->fst_cpy_size >= VIRTUAL_QUANTUM_SIZE)
			    {
				    fcp_put_quantum(quantum_id);
					rec_all_ch_info->fst_cpy_size = 0;
	            }
		    }
			else
	        {
		        copy_to_user(buf,ts_pkt_inf->pkt_addr,nReadLen);
			}
		} else {
        copy_to_user(buf,ts_pkt_inf->pkt_addr,nReadLen);
		}

        rec_all_ch_info->rec_whole_tp_rd += (nReadLen/188);
        if(rec_all_ch_info->rec_whole_tp_rd >= dmx->ts_buf_end_idx)
        {
            rec_all_ch_info->rec_whole_tp_rd = 0;
        }
        //printk("2 rec_whole_tp_rd=%d nRemainLen=%d \r\n",rec_all_ch_info->rec_whole_tp_rd,nReadLen/188);

        if(nRemainLen>nReadLen)
        {
            ts_pkt_inf = &(dmx->pkt_inf[0]);
            
            dma_cache_inv((DMX_UINT32)(ts_pkt_inf->pkt_addr), nRemainLen - nReadLen);

			if (feature_is_fastcopy_enable())
			{
				if(rec_all_ch_info->fst_cpy_slot >= 0)
				{
		            memcpy(qbuf+rec_all_ch_info->fst_cpy_size, ts_pkt_inf->pkt_addr, nRemainLen - nReadLen);
				    rec_all_ch_info->fst_cpy_size += nRemainLen - nReadLen;
		            if(rec_all_ch_info->fst_cpy_size >= VIRTUAL_QUANTUM_SIZE)
				    {
						fcp_put_quantum(quantum_id);
		                rec_all_ch_info->fst_cpy_size = 0;
				    }
		        }
				else
		        {
				    copy_to_user(buf+nReadLen,ts_pkt_inf->pkt_addr,   nRemainLen - nReadLen);
		        }
			} else {     
		            copy_to_user(buf+nReadLen,ts_pkt_inf->pkt_addr,   nRemainLen - nReadLen);
			}
            rec_all_ch_info->rec_whole_tp_rd += ((nRemainLen - nReadLen)/188);
            //printk("3 rec_whole_tp_rd=%d nRemainLen=%d \r\n",rec_all_ch_info->rec_whole_tp_rd,(nRemainLen - nReadLen)/188);

        }
        
    }

    
   
    return ret;

}





ssize_t dmx_channel_read
(
    struct file *file,
    char __user *buf,
    size_t       count,
    loff_t      *ppos
)
{
    struct dmx_channel       *channel; 
    struct dmx_channel_param *usr_para;
    struct dmx_device        *dmx;

    int                       ret;
    
    //DMX_LINUX_API_DEBUG("dmx_channel_read()!\n");

    channel = file->private_data;

    ret = 0;

    usr_para = &channel->usr_param;
    dmx = (struct dmx_device  *)channel->dmx;
    
    switch(usr_para->output_format)
    {
        case DMX_CHANNEL_OUTPUT_FORMAT_TS:
        {
            //DMX_LINUX_API_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);
            if(usr_para->rec_whole_tp == TRUE)
            {
                ret = dmx_channel_recoder_all(file, buf, count, ppos);
            }
            else
            {
                ret = dmx_channel_usr_rd_data(file, buf, count, ppos);
            }
        }
        break;   

        case DMX_CHANNEL_OUTPUT_FORMAT_PES:
        case DMX_CHANNEL_OUTPUT_FORMAT_SEC:
        {
            ret = dmx_channel_usr_rd_pkt(file, buf, count, ppos);
        }
        break;  

        default:
        {
            //ret = ;

        }
        break;  

    }

    return(ret);
}


int dmx_channel_open
(
    struct inode *inode,
    struct file  *file
)
{
    int                 ret;
    int                 i;
    struct dmx_device  *dmx;
    struct dmx_channel *channel;

    DMX_LINUX_API_DEBUG("%s, go\n", __FUNCTION__);

    ret = 0;

    dmx = container_of(inode->i_cdev, struct dmx_device, cdev);

    if (mutex_lock_interruptible(&dmx->io_mutex))
    {
        return(-ERESTARTSYS);
    }

    for (i = 0; i < DMX_LINUX_API_TOTAL_CHANNELS; i++)
    {
        channel = &(dmx->channels[i]);

        if (DMX_CHANNEL_STATUS_IDLE == channel->status)
        {
            channel->status = DMX_CHANNEL_STATUS_READY;

            channel->dmx = (void *)dmx;

            file->private_data = channel;

            DMX_LINUX_API_DEBUG("Got idle channel %d\n", i);

            break;
        }
    }

    mutex_unlock(&dmx->io_mutex);

    if (i >= DMX_LINUX_API_TOTAL_CHANNELS)
    {
        DMX_LINUX_API_DEBUG("No idle channel!\n");

        ret = (-EMFILE);
    }

    return(ret);
}



int dmx_channel_release
(
    struct inode *inode,
    struct file  *file
)
{
    int                ret;
    struct dmx_device *dmx;

    struct dmx_channel *channel;

    channel = file->private_data;

    if ((channel->status != DMX_CHANNEL_STATUS_RUN) &&
        (channel->status != DMX_CHANNEL_STATUS_READY))
    {
        return(-EPERM);        
    }

    ret = 0;

    dmx = container_of(inode->i_cdev, struct dmx_device, cdev);

    if (mutex_lock_interruptible(&dmx->io_mutex))
    {
        return(-ERESTARTSYS);
    }

    if (DMX_CHANNEL_STATUS_RUN == channel->status)
    {
        dmx_channel_stop(dmx, channel);
    }

    channel->status = DMX_CHANNEL_STATUS_IDLE;
    
    channel->dmx = NULL;

    file->private_data = NULL;

    mutex_unlock(&dmx->io_mutex);

    return(ret);
}



unsigned int dmx_channel_poll
(
    struct file              *filp,
    struct poll_table_struct *wait
)
{
    __s32                     mask;
    struct dmx_channel       *channel; 
    struct dmx_channel_param *usr_para;
    struct dmx_device        *dmx;

    channel = filp->private_data;

    dmx = (struct dmx_device *)channel->dmx;

    usr_para = &channel->usr_param;

    mask = 0;

    //printk("%s,%d\n", __FUNCTION__, __LINE__);

    switch(usr_para->output_format)
    {
        case DMX_CHANNEL_OUTPUT_FORMAT_SEC:
        case DMX_CHANNEL_OUTPUT_FORMAT_PES:
        {
            //printk("%s,%d\n", __FUNCTION__, __LINE__);

            if (DMX_OUTPUT_SPACE_USER == usr_para->output_space)
            {
                //printk("%s,%d\n", __FUNCTION__, __LINE__);

                poll_wait(filp, &(channel->data_buf.rd_wq), wait);

                if (dmx_data_buf_first_pkt_len(dmx, &channel->data_buf) > 0)
                {
                    //printk("plen:%d", dmx_data_buf_first_pkt_len(&channel->data_buf));

        			mask |= (POLLIN | POLLRDNORM); /* readable. */
                }
            }
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_TS:
        {
            if (DMX_OUTPUT_SPACE_USER == usr_para->output_space)
            {
                extern UINT32 dmx_get_current_timeMS(void);
                static UINT32 poll_ltime = 0;
                UINT32 poll_time;
                if(channel->ts_ch_info.enc_para != NULL)
                {
                    if(0 == poll_ltime)
                    {
                        poll_ltime = dmx_get_current_timeMS();
                        poll_time = 0;
                    }else
                        poll_time = dmx_get_current_timeMS() - poll_ltime;
                         
                    if((poll_time > 30 && channel->ts_ch_info.dst_blk_len >= 0x4000) || \
                        (channel->ts_ch_info.dst_blk_len >= 0x8000) )
                    {
                        mask |= (POLLIN | POLLRDNORM); /* readable. */
                        poll_ltime = 0;
                    }
                    else
                    {
                        msleep(1);
                    }
                }
                else if(usr_para->rec_whole_tp == TRUE)
                {
                    if (dmx_get_rec_all_valid_data_len(dmx, channel) > 0x4000/* 64K */)
                    {
            			mask |= (POLLIN | POLLRDNORM); /* readable. */
                    }
                   else
                        msleep(2);
                }
                else
                {
                    poll_wait(filp, &(channel->data_buf.rd_wq), wait);
                    if (dmx_data_buf_len(dmx, &channel->data_buf) > 0x10000/* 64K */)
                    {
            			mask |= (POLLIN | POLLRDNORM); /* readable. */
                    }
                    else
                    {
                        msleep(2);
                    }
                }
            }
        }
        break;

        default:
        {
            DMX_LINUX_API_DEBUG("Invalid output_format! return fail.\n");

            return(-EINVAL);
        }
        break;
    }
    
    return(mask);
}


DMX_INT32 dmx_channel_add_pid
(
    struct dmx_device  *dmx,
    struct dmx_channel *channel,
    unsigned int        cmd,
    unsigned long       arg
)
{
    struct dmx_ts_channel_info   *ts_ch_info;
    struct dmx_sec_channel_info  *sec_ch_info;
    struct dmx_pes_channel_info  *pes_ch_info;
    struct dmx_rec_all_channel_info *rec_all_ch_info;
    
    struct dmx_channel_param     usr_para;
    int                          i,j,ret;

    struct dmx_ts_kern_recv_info *ts_kern_recv_info;

    
    /* Channel startus validation, return success in all status. */
    if (channel->status != DMX_CHANNEL_STATUS_RUN)
    {
        //printk("fail %s, line:%d\n", __FUNCTION__, __LINE__);
        return(0);
    }
    ret = copy_from_user(&usr_para, (void __user *)arg,
                         _IOC_SIZE(cmd));    
    if (0 != ret)
    {
        //printk("%s, line:%d\n", __FUNCTION__, __LINE__);
        return(-EFAULT);
    } 
    
    switch(usr_para.output_format)
    {
        case DMX_CHANNEL_OUTPUT_FORMAT_TS:
        {
            DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);

            ts_ch_info = &channel->ts_ch_info;
            rec_all_ch_info = &channel->rec_all_ch_info;
            //memset(ts_ch_info, 0, sizeof(struct dmx_ts_channel_info));

            if (DMX_OUTPUT_SPACE_KERNEL == channel->usr_param.output_space)
            {

                //dmx_check_front_end(usr_para->fe,usr_para->nim_chip_id);
                //ts_kern_recv_info = &(usr_para->ts_param.kern_recv_info);
                ts_kern_recv_info = &(channel->usr_param.ts_param.kern_recv_info);

                //printk("%s, line:%d\n", __FUNCTION__, __LINE__);

                for (i = channel->usr_param.ts_param.pid_list_len,j = 0; 
                    i < channel->usr_param.ts_param.pid_list_len + usr_para.ts_param.pid_list_len; i++,j++)
                {
                    if(i >= DMX_REC_PID_LIST_MAX_LEN)
                    {
                        //printk("error:pid full %s, line:%d\n", __FUNCTION__, __LINE__);
                        break;
                    }                    
                    ts_ch_info->ts_serv_id[i] = dmx_ts_service_register(dmx,
                        usr_para.ts_param.pid_list[j] & 0x1FFF, 
                        (ts_serv_recv_func)ts_kern_recv_info->kern_recv_routine,
                        ts_kern_recv_info->kern_recv_routine_para);
                    //printk("serv id:%d\n", ts_ch_info->ts_serv_id[i]);
                    if (DMX_INVALID_IDX == ts_ch_info->ts_serv_id[i])
                    {
                        //printk("%s, line:%d\n", __FUNCTION__, __LINE__);
        
                        //dmx_channel_stop(dmx, channel);
    
                        return(-EBUSY);
                    }
                    
                }
            }
            else if (DMX_OUTPUT_SPACE_USER == channel->usr_param.output_space)
            {
                j = 0;
                //printk("%s, len:%d\n", __FUNCTION__);
                for (i = channel->usr_param.ts_param.pid_list_len,j = 0; 
                    i < channel->usr_param.ts_param.pid_list_len + usr_para.ts_param.pid_list_len; j++,i++)
                {
                    if(i >= DMX_REC_PID_LIST_MAX_LEN)
                    {
                        //printk("error:pid full %s, line:%d\n", __FUNCTION__, __LINE__);
                        break;
                    }  
                    ts_ch_info->ts_serv_id[i] = dmx_ts_service_register(
                        dmx,
                        usr_para.ts_param.pid_list[j] & 0x1FFF, 
                        (ts_serv_recv_func)dmx_channel_ts_kern_wr,
                        (DMX_UINT32)channel);
                    
                    //printk("%d, serv id:%d\n",i, ts_ch_info->ts_serv_id[i]);
                    if (DMX_INVALID_IDX == ts_ch_info->ts_serv_id[i])
                    {
                        //printk("%s, line:%d\n", __FUNCTION__, __LINE__);
        
                        //dmx_channel_stop(dmx, channel);
    
                        return(-EBUSY);
                    }
                    
                }

            }
            else 
            {
                //DMX_API_DEBUG("%s, line:%d\n", __FUNCTION__, __LINE__);
                //printk("param error %s, line:%d\n", __FUNCTION__, __LINE__);
                return(-EFAULT);
            }
            //printk("%s, line:%d\n", __FUNCTION__, __LINE__);

            for (i = 0; i < channel->usr_param.ts_param.pid_list_len;i++) 
            {
                //printk("%d, serv_id %d\n",i,channel->ts_ch_info.ts_serv_id[i]);
            }
            j = 0;            
            for (i = channel->usr_param.ts_param.pid_list_len; 
                i < channel->usr_param.ts_param.pid_list_len + usr_para.ts_param.pid_list_len; i++)
            {

           
                if(i >= DMX_REC_PID_LIST_MAX_LEN)
                {
                    //printk("error:pid full %s, line:%d\n", __FUNCTION__, __LINE__);
                    break;
                }
                channel->usr_param.ts_param.pid_list[i] = usr_para.ts_param.pid_list[j];
                //printk("%d ker pid %d ,usr %d\n",i,channel->usr_param.ts_param.pid_list[i],usr_para.ts_param.pid_list[j]);
                j++;
                dmx_ts_service_enable(dmx, ts_ch_info->ts_serv_id[i]);
            }
            channel->usr_param.ts_param.pid_list_len = i;    
            //printk("%s, line:%d,len %d\n", __FUNCTION__, __LINE__,channel->usr_param.ts_param.pid_list_len);
        }
        break;
        default:
        {
            return(-EINVAL);
        }
        break;
    }
    return(0);
}

DMX_INT32 dmx_channel_del_pid
(
    struct dmx_device  *dmx,
    struct dmx_channel *channel,
    unsigned int        cmd,
    unsigned long       arg
)
{
    struct dmx_ts_channel_info   *ts_ch_info;
    struct dmx_sec_channel_info  *sec_ch_info;
    struct dmx_pes_channel_info  *pes_ch_info;
    struct dmx_rec_all_channel_info *rec_all_ch_info;
    
    struct dmx_channel_param     usr_para;
    int                          i,j,ret;
    UINT32                       old_val;
    struct dmx_ts_kern_recv_info *ts_kern_recv_info;
    
    
    /* Channel startus validation, return success in all status. */
    if (channel->status != DMX_CHANNEL_STATUS_RUN)
    {
        //printk("fail %s, line:%d\n", __FUNCTION__, __LINE__);
        return(0);
    }
    ret = copy_from_user(&usr_para, (void __user *)arg,
                         _IOC_SIZE(cmd));    
    if (0 != ret)
    {
        //printk("%s, line:%d\n", __FUNCTION__, __LINE__);
        return(-EFAULT);
    } 
    
    switch(usr_para.output_format)
    {
        case DMX_CHANNEL_OUTPUT_FORMAT_TS:
        {
            //printk("%s, line:%d\n", __FUNCTION__, __LINE__);

            ts_ch_info = &channel->ts_ch_info;
            rec_all_ch_info = &channel->rec_all_ch_info;
            //memset(ts_ch_info, 0, sizeof(struct dmx_ts_channel_info));

            //dmx_check_front_end(usr_para->fe,usr_para->nim_chip_id);
            //ts_kern_recv_info = &(usr_para->ts_param.kern_recv_info);
            //ts_kern_recv_info = &(channel->usr_param.ts_param.kern_recv_info);

            //printk("%s, line:%d\n", __FUNCTION__, __LINE__);
            //printk("%s, u_len:%d,%d\n", __FUNCTION__, usr_para.ts_param.pid_list_len,channel->usr_param.ts_param.pid_list_len);
            for (i = 0; i < usr_para.ts_param.pid_list_len; i++)
            {
                for(j =0;j < channel->usr_param.ts_param.pid_list_len; j++)
                {
                    if(channel->usr_param.ts_param.pid_list[j] == usr_para.ts_param.pid_list[i])
                    {
                        //printk("del %s, line:%d,%d\n", __FUNCTION__, __LINE__,j);
                        dmx_ts_service_unregister(dmx,ts_ch_info->ts_serv_id[j]);
                        ts_ch_info->ts_serv_id[j] = DMX_INVALID_IDX;
                        channel->usr_param.ts_param.pid_list[j] = 0x1fff;                        
                    }
                }
            }
            j = 0;
            for(i = 0; i < channel->usr_param.ts_param.pid_list_len; i++)
            {
                if((channel->usr_param.ts_param.pid_list[i] & 0x1fff) != 0x1fff)
                {
                    channel->usr_param.ts_param.pid_list[j] = channel->usr_param.ts_param.pid_list[i];
                    j++;                        
                }                    
            }
            old_val = channel->usr_param.ts_param.pid_list_len;
            channel->usr_param.ts_param.pid_list_len = j;
            j = 0;
            for(i = 0; i < old_val; i++)
            {
                if(ts_ch_info->ts_serv_id[i] != DMX_INVALID_IDX)
                {
                    ts_ch_info->ts_serv_id[j] = ts_ch_info->ts_serv_id[i];
                    j++;                        
                }                    
            }
                        
            
            //printk("%s, line:%d,len %d\n", __FUNCTION__, __LINE__,channel->usr_param.ts_param.pid_list_len);
        }
        break;
        default:
        {
            return(-EINVAL);
        }
        break;
    }
    return(0);    
}

int dmx_ioctl
(
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
{
    struct dmx_channel  *channel; 
    struct dmx_device   *dmx;
    int                  ret;

    ret = 0;

    channel = filp->private_data;

    dmx = (struct dmx_device *)channel->dmx;
    
    switch(cmd)
    {
        case ALI_DMX_CHANNEL_START:
        {
            if (mutex_lock_interruptible(&dmx->io_mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = dmx_channel_start(dmx, channel, cmd, arg);

            mutex_unlock(&dmx->io_mutex);
        }
        break;

        case ALI_DMX_CHANNEL_STOP:
        {
            if (mutex_lock_interruptible(&dmx->io_mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = dmx_channel_stop(dmx, channel);

            mutex_unlock(&dmx->io_mutex);
        }
        break;

        case ALI_DMX_CHANNEL_GET_CUR_PKT_LEN:
        {
            if (mutex_lock_interruptible(&dmx->io_mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = dmx_data_buf_get_pkt_len(dmx, channel, cmd, arg);

            mutex_unlock(&dmx->io_mutex);
        }
        break;

        case ALI_DMX_HW_GET_FREE_BUF_LEN:
        {
            if (mutex_lock_interruptible(&dmx->io_mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = dmx_hw_get_free_buf_len(dmx, channel, cmd, arg);

            mutex_unlock(&dmx->io_mutex);
        }
        break;

        case ALI_DMX_IO_SET_BYPASS_MODE:
        {
            dmx->pfunc.dmx_hw_reg_reset(dmx->base_addr, arg, 662);
            dmx->pfunc.dmx_hw_set_bypass_mode(dmx->base_addr);
            dmx->pfunc.dmx_hw_flt_enable(dmx->base_addr, 0);
            ali_dmx_m3501_autoscan=1;
        }
        break;
        
        case ALI_DMX_IO_BYPASS_GETDATA:
        {
            ret = dmx->pfunc.dmx_hw_buf_wr_get(dmx->base_addr);
            if(ret == 660)
            {
                dma_cache_wback(arg, 660*188);
                dmx->pfunc.dmx_hw_reg_reset(dmx->base_addr, arg, 662);
                dmx->pfunc.dmx_hw_set_bypass_mode(dmx->base_addr);
                dmx->pfunc.dmx_hw_flt_enable(dmx->base_addr, 0);
            };	
        }
        break;	
        
        case ALI_DMX_IO_CLS_BYPASS_MODE:
          {
             	dmx->pfunc.dmx_hw_reg_reset(dmx->base_addr, dmx->ts_buf_start_addr, dmx->ts_buf_end_idx);
							dmx->pfunc.dmx_hw_flt_enable(dmx->base_addr, 0);
							ali_dmx_m3501_autoscan=0;
          }

        break;
              
       case ALI_DMX_BYPASS_ALL:
       {
            
           if(arg)
               dmx->pfunc.dmx_hw_set_bypass_mode(dmx->base_addr);
           else
               dmx->pfunc.dmx_hw_clear_bypass_mode(dmx->base_addr);

       }
 
         break;
        case ALI_DMX_GET_PROG_BITRATE:
        {
 
             INT32 time_out = 500;
             UINT32 bit_rate = 0;
             if(dmx->bitrate_detect == 0)
             {
                 if (mutex_lock_interruptible(&dmx->io_mutex))
                 {
                     return(-ERESTARTSYS);
                 }
                 dmx->pkt_total_in_cnt = 0;
                 dmx->last_pack_num = 0;
                 dmx->pkts_dura = 1;
                 dmx->bitrate_detect = 1;
                 dmx->last_dura = 1;
                 dmx->bit_rate = 0;
                 dmx->last_rd_time = 0;
                 
                 dmx->last_rate[0] = dmx->last_rate[1] = dmx->last_rate[2] = dmx->last_rate[3] = 0;
                 dmx->last_rate[4] = dmx->last_rate[5] = dmx->last_rate[6] = dmx->last_rate[7] = 0;
                 mutex_unlock(&dmx->io_mutex);
                 while (dmx->bit_rate == 0)
                 {
                     msleep(2);
                     if (--time_out == 0)
                     {
                         break;
                     }
                 }
                 if (time_out == 0)
                     bit_rate = 0;
                 else
                     bit_rate = dmx->bit_rate;
 
             }
             else
             {
                 bit_rate = dmx->bit_rate;
             }
             copy_to_user(arg, &bit_rate, sizeof(UINT32));
             break;
        }

        case ALI_DMX_RESET_BITRATE_DETECT:
        {
             dmx->bitrate_detect = 0;
             break;
        }
#if 0 //by wen         
        case ALI_DMX_SET_HW_INFO:
        {
            UINT32 fe;
            if(((arg >> 16) & 0x1) == 1)
            {
                fe = PROJECT_FE_DVBS;
                dmx_check_front_end(fe,arg & 0xFFFF);
            }
            break;
        }       
		// add for sat2ip
        case ALI_DMX_CHANNEL_ADD_PID:
        {
            ret = dmx_channel_add_pid(dmx, channel, cmd, arg);
            break;
        }
        case ALI_DMX_CHANNEL_DEL_PID:
        {
            ret = dmx_channel_del_pid(dmx, channel, cmd, arg);
            break;
        }
#endif
		//////////////////////////////////////
        default:  
          return(-ENOTTY);
    }

    return(ret);
}



int dmx_channel_init
(
    struct dmx_device  *dmx
)
{
    DMX_UINT32 i;

    for (i = 0; i < DMX_LINUX_API_TOTAL_CHANNELS; i++)
    {
        dmx->channels[i].status = DMX_CHANNEL_STATUS_IDLE;

        dmx->channels[i].dmx = (DMX_UINT8 *)dmx;
    }

    return(0);
}













