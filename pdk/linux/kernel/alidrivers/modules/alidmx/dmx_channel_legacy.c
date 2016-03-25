
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
#include <linux/sched.h>
#include <linux/sched/rt.h>
#else
#include <linux/sched.h>
#endif
#include <linux/kthread.h>
#include <asm/io.h>
#include <ali_interrupt.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/sched.h> 
#include <linux/mm.h>

//#include <ali_dmx_common.h>

#include <linux/slab.h>

#include "dmx_stack.h"
#include "dmx_see_interface.h"

#include <linux/ali_dsc.h>
#include "dmx_dbg.h"

#if 1

#if 0
#define DMX_CH_LEGA_DBG printk
#else
#define DMX_CH_LEGA_DBG(...)
#endif


struct dmx_channel_module_legacy ali_dmx_channel_module_legacy;

extern struct dmx_ts_flt_module ali_dmx_ts_flt_module;
extern struct dmx_pcr_flt_module ali_dmx_pcr_flt_module;
extern struct dmx_sec_flt_module ali_dmx_sec_flt_module;
extern struct dmx_pes_flt_module ali_dmx_pes_flt_module;
extern struct dmx_see_device ali_dmx_see_dev[1];
extern struct Ali_DmxKernGlobalStatInfo g_stat_info;
extern void dmx_internal_init(void);


__s32 dmx_channel_get_pkt_len
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                     ret;
    __u32                     len;
    struct dmx_channel_param *usr_para;

    DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

    /* ch startus validation, return success in all state.
    */
    if (ch->state == DMX_CHANNEL_STATE_IDLE)
    {
        return(0);
    }

    ret = 0;

    usr_para = &ch->usr_param;

    switch(usr_para->output_format)
    {
        case DMX_CHANNEL_OUTPUT_FORMAT_SEC:
        {
            len = dmx_data_buf_first_pkt_len(&ch->data_buf);
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_PES:
        {
            len = dmx_data_buf_first_pkt_len(&ch->data_buf);
        }
        break;        

        case DMX_CHANNEL_OUTPUT_FORMAT_TS:
        {
            len = dmx_data_buf_total_len(&ch->data_buf);
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
            //DMX_CH_LEGA_DBG("len 2:%d\n", len);
    
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



__s32 dmx_bitrate_get
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch,
    unsigned int              cmd,
    unsigned long             arg
)

{
    __s32 ret;
    __u32 bitrate;

	/* Temparary, need to be implemented by upper layer.
	*/
    bitrate = 4 * 1024 * 1024;

    ret = copy_to_user((void __user *)arg, &bitrate, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        ret = -ENOTTY;
    }   

    return(ret);
}





__s32 dmx_hw_buf_free_len
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __u32 pkt_cnt;
    __s32 ret;
    __u32 rd_idx;
    __u32 wr_idx;
    __u32 end_idx;

    ret = 0;

    DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

    /* ch startus validation, return success in all state.
    */
    if (ch->state == DMX_CHANNEL_STATE_IDLE)
    {
        return(0);
    }

    wr_idx = dmx_hw_buf_wr_get(dev->src_hw_interface_id);

    rd_idx = dmx_hw_buf_rd_get(dev->src_hw_interface_id);

    if (wr_idx >= rd_idx)
    {
        end_idx = dmx_hw_buf_end_get(dev->src_hw_interface_id);
        
        pkt_cnt = end_idx - wr_idx + rd_idx - 1;
    }
    else
    {
        pkt_cnt = rd_idx - wr_idx - 1;
    }

    ret = copy_to_user((void __user *)arg, &pkt_cnt, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        ret = -ENOTTY;
    }    
    
    DMX_CH_LEGA_DBG("%s,%d,pkt_cnt:%d\n", __FUNCTION__, __LINE__, pkt_cnt);

    return(ret);
}


/* Note: Called in inerrupt context, so this function may not sleep.
*/
__s32 dmx_channel_pcr_wr
(
    __u32 pcr,
    __u32 param
)
{
    dmx_see_set_pcr(pcr);

    return(0);
}


__s32 dmx_channel_kern_glb_cfg
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch
)
{
	if (NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    /* stream state validation. 
     */
    if ((DMX_CHANNEL_STATE_CFG != ch->state) &&
        (DMX_CHANNEL_STATE_STOP != ch->state))
    {
        return(-EPERM);
    }

    ch->state = DMX_CHANNEL_STATE_STOP;

	memset(&g_stat_info, 0, sizeof(struct Ali_DmxKernGlobalStatInfo));

    return(0);
}



__s32 dmx_channel_kern_glb_start
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch
)
{    
	if (NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    /* channel state validation. 
     */
    if (ch->state != DMX_CHANNEL_STATE_STOP)
    {
        return(-EPERM);
    }

    ch->state = DMX_CHANNEL_STATE_RUN;

    return(0);
}



__s32 dmx_channel_kern_glb_stop
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch
)
{
	if (NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (ch->state != DMX_CHANNEL_STATE_RUN)
    {
        return(-EPERM);
    }

    ch->state = DMX_CHANNEL_STATE_STOP;

    return(0);
}


__s32 dmx_channel_kern_glb_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32 ret;

	if (0 == arg || NULL == ch)
    {
        DMX_API_DBG("%s(), L[%d] \n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

#if 0	
    if (stream->type != DMX_STREAM_TYPE_KERN_GLB)
    {
        return(-EPERM);
    }
#endif


    if (ch->state != DMX_CHANNEL_STATE_RUN)
    {
    	DMX_API_DBG("%s(), L[%d], state[%d]\n", __FUNCTION__, __LINE__, ch->state);
        
        return(-EPERM);
    }

	ret = copy_to_user((void __user *)arg, &g_stat_info, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }
    
    return(0);
}


__s32 dmx_channel_kern_glb_realtime_set
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
	if (NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    /* stream state validation. 
     */
    if (DMX_CHANNEL_STATE_IDLE == ch->state)
    {
        return(-EPERM);
    }

	g_stat_info.RealTimePrintEn = arg;

    return(0);
}


__s32 dmx_channel_see_glb_cfg
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch
)
{
	volatile struct Ali_DmxSeeGlobalStatInfo *p_StatInfo;

	if (NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    /* stream state validation. 
     */
    if ((DMX_CHANNEL_STATE_CFG != ch->state) &&
        (DMX_CHANNEL_STATE_STOP != ch->state))
    {
        return(-EPERM);
    }

    ch->state = DMX_CHANNEL_STATE_STOP;

	p_StatInfo = ali_dmx_see_dev[0].see_buf_init.GlobalStatInfo;

	memset((void *)p_StatInfo, 0, sizeof(struct Ali_DmxSeeGlobalStatInfo));

    return(0);
}



__s32 dmx_channel_see_glb_start
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch
)
{    
	if (NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    /* Channel state validation. 
     */
    if (ch->state != DMX_CHANNEL_STATE_STOP)
    {
        return(-EPERM);
    }

    ch->state = DMX_CHANNEL_STATE_RUN;

    return(0);
}



__s32 dmx_channel_see_glb_stop
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch
)
{
	if (NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (ch->state != DMX_CHANNEL_STATE_RUN)
    {
        return(-EPERM);
    }

    ch->state = DMX_CHANNEL_STATE_STOP;

    return(0);
}



__s32 dmx_channel_see_glb_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32 ret;
	volatile struct Ali_DmxSeeGlobalStatInfo *p_StatInfo;

	if (0 == arg || NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }
	
    if (ch->state != DMX_CHANNEL_STATE_RUN)
    {
        return(-EPERM);
    }

	p_StatInfo = ali_dmx_see_dev[0].see_buf_init.GlobalStatInfo;
			
	ret = copy_to_user((void __user *)arg, (const void *)p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }
    
    return(0);
}


__s32 dmx_channel_see_glb_realtime_set
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
	if (NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    /* stream state validation. 
     */
    if (DMX_CHANNEL_STATE_IDLE == ch->state)
    {
        return(-EPERM);
    }

	ali_dmx_see_dev[0].see_buf_init.statistics->RealTimePrintEn = arg;

    return(0);
}

__s32 dmx_channel_hw_reg_cfg
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch
)
{
    /* channel state validation. 
     */
    if (ch->state != DMX_CHANNEL_STATE_CFG &&
		ch->state != DMX_CHANNEL_STATE_STOP)
    {
        return(-EPERM);
    }

    ch->state = DMX_CHANNEL_STATE_STOP;

    return(0);
}




__s32 dmx_channel_hw_reg_start
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch
)
{    	

    /* channel state validation. 
     */
    if (ch->state != DMX_CHANNEL_STATE_STOP)
    {
        return(-EPERM);
    }

    ch->state = DMX_CHANNEL_STATE_RUN;

    return(0);
}



__s32 dmx_channel_hw_reg_stop
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch
)
{
    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);


    if (ch->state != DMX_CHANNEL_STATE_RUN)
    {
        return(-EPERM);
    }

    ch->state = DMX_CHANNEL_STATE_STOP;

    return(0);
}



__s32 dmx_channel_hw_reg_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32 ret;
	__u32 HwRegTable[18][5];
	__u32 i, j, k, DmxBaseAddr;
	extern __u32 dmx_hw_id2base_m37(__u32);
	extern __u32 AliRegGet32(__u32);


    if (ch->state != DMX_CHANNEL_STATE_RUN)
    {
        return(-EPERM);
    }

	DmxBaseAddr = dmx_hw_id2base_m37(0);
	
	memset(HwRegTable, 0, sizeof(HwRegTable));

	for (i = 0, k = 0; i < 0x370; i += 16)
	{
		if (i > 0x64 && i < 0x300)
		{
			if (i != 0xB0 && i != 0xC0 && i != 0x140 && i != 0x1C0)
			{
				continue;
			}
		}

		if (!(i & 0xf))
		{
			HwRegTable[k][0] = DmxBaseAddr + i;
		}

		for (j = 0; j < 4; j++)
		{
			HwRegTable[k][j + 1] = AliRegGet32(HwRegTable[k][0] + j * 4);
		}

		k++;
	}

    ret = copy_to_user((void __user *)arg, &HwRegTable, sizeof(HwRegTable));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);
    
    return(0);
}


__s32 dmx_channel_ts_wr
(
    struct dmx_ts_pkt_inf *pkt_inf,
    __u32                  param
)
{
    __s32                    byte_cnt;
	__s32                    dsc_byte_cnt;
    struct ali_dmx_data_buf *dest_buf;
	struct dmx_channel      *ch;
	__s32                    ret;
	__s32                    cur_data_len;
	__s32                    deencrypt_pkt_cnt;
	
    byte_cnt = 0;
	
	ch = (struct dmx_channel *)param;

	if((pkt_inf->scramble_flag != 0) && (ch->detail.ts_ch.enc_para != NULL))
	{
        dest_buf = &(ch->data_buf_orig);

		/* Write to interchange buffer.
		*/
        byte_cnt = dmx_data_buf_wr_data(dest_buf, pkt_inf->pkt_addr, 188, 
                                        DMX_DATA_SRC_TYPE_KERN);

	    cur_data_len = dmx_data_buf_total_len(dest_buf);

        /*  Read out TS packet from interchange buffer, de-encrypt it, write back to normal buffer,
         *  wait to be read out to userspace.
		*/
		if (cur_data_len >= DMX_DEENCRYPT_BUF_LEN)
		{
    		/* Wrap to multiple of 64 pakcets to meet DSC requirement.
    		 * DMX_DEENCRYPT_BUF_LEN must be multiple of 64 TS pakcets.
    		 */
    		deencrypt_pkt_cnt = (DMX_DEENCRYPT_BUF_LEN / 188);
    		
            dsc_byte_cnt = dmx_data_buf_rd(ali_dmx_channel_module_legacy.dmx_de_enc_input_buf,
											&(ch->data_buf_orig), DMX_DEENCRYPT_BUF_LEN, DMX_DATA_CPY_DEST_KERN); 
    
            //DMX_CH_LEGA_DBG("dmx_de_enc_input_buf:%x,dmx_de_enc_output_buf:%x\n", ali_dmx_channel_module_legacy.dmx_de_enc_input_buf,
				    //ali_dmx_channel_module_legacy.dmx_de_enc_output_buf);

			/* De-encrypt ts packets.
			*/
#ifdef CONFIG_ALI_DSC
            ret = ali_DeEncrypt(ch->detail.ts_ch.enc_para, ali_dmx_channel_module_legacy.dmx_de_enc_input_buf, 
				                ali_dmx_channel_module_legacy.dmx_de_enc_output_buf, deencrypt_pkt_cnt);
#endif

            if (ret != 0)
            {
                DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
    
                return(-EFAULT);     
            }	    

	        /* Write back De-encrypted ts packets to normal buffer to be read out by userspace. 
			*/
            dest_buf = &(ch->data_buf);
        	
            dsc_byte_cnt = dmx_data_buf_wr_data(dest_buf, ali_dmx_channel_module_legacy.dmx_de_enc_output_buf,
												DMX_DEENCRYPT_BUF_LEN, DMX_DATA_SRC_TYPE_KERN);
			if (dsc_byte_cnt < 0)
			{		
				/* Must be buffer overflow, flush all data of this stream 
				* to free memory.
				*/
				dmx_data_buf_flush_all_pkt(dest_buf);		
			}			
		}

        return(byte_cnt);
	}

	/*when ts scramble status change, we will handle original data to data buffer*/
	dest_buf = &(ch->data_buf_orig);
    cur_data_len = dmx_data_buf_total_len(dest_buf);
	if((cur_data_len > 0) && (ch->detail.ts_ch.enc_para != NULL))
	{
		/* Wrap to multiple of 64 pakcets to meet DSC requirement.
		*/		
		deencrypt_pkt_cnt = (cur_data_len / 188);

		deencrypt_pkt_cnt /= 64;

		deencrypt_pkt_cnt *= 64;	

        dsc_byte_cnt = dmx_data_buf_rd(ali_dmx_channel_module_legacy.dmx_de_enc_input_buf,
										&(ch->data_buf_orig), deencrypt_pkt_cnt * 188, DMX_DATA_CPY_DEST_KERN); 

        //DMX_CH_LEGA_DBG("dmx_de_enc_input_buf:%x,dmx_de_enc_output_buf:%x\n", ali_dmx_channel_module_legacy.dmx_de_enc_input_buf,
			    //ali_dmx_channel_module_legacy.dmx_de_enc_output_buf);

		/* De-encrypt ts packets.
		*/
		#ifdef CONFIG_ALI_DSC
        ret = ali_DeEncrypt(ch->detail.ts_ch.enc_para, ali_dmx_channel_module_legacy.dmx_de_enc_input_buf, 
			                ali_dmx_channel_module_legacy.dmx_de_enc_output_buf, deencrypt_pkt_cnt);

        if (ret != 0)
        {
            DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);

            return(-EFAULT);     
        }	    
		#endif
        /* Write back De-encrypted ts packets to normal buffer to be read out by userspace. 
		*/
        dest_buf = &(ch->data_buf);
    	
        dsc_byte_cnt = dmx_data_buf_wr_data(dest_buf, ali_dmx_channel_module_legacy.dmx_de_enc_output_buf,
											deencrypt_pkt_cnt * 188, DMX_DATA_SRC_TYPE_KERN);
		if (dsc_byte_cnt < 0)
		{		
			/* Must be buffer overflow, flush all data of this stream 
			* to free memory.
			*/
			dmx_data_buf_flush_all_pkt(dest_buf);		
		}	
	}

    dest_buf = &(((struct dmx_channel *)param)->data_buf);

    byte_cnt = dmx_data_buf_wr_data(dest_buf, pkt_inf->pkt_addr, 188, 
                                    DMX_DATA_SRC_TYPE_KERN);
	if (byte_cnt < 0)
	{		
		/* Must be buffer overflow, flush all data of this stream 
		* to free memory.
		*/
		dmx_data_buf_flush_all_pkt(dest_buf);		
	}

    return(byte_cnt);
}


__s32 dmx_channel_ts_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                           ret;
	__u32                           ts_flt_idx;
	__u32                           pid_flt_idx;
    __u32							flt_list_idx;
	struct Ali_DmxDrvTsStrmStatInfo *p_StatInfo;  
	struct Ali_DmxDrvTsStrmStatInfo *p_UsrInfo;

	p_UsrInfo = (struct Ali_DmxDrvTsStrmStatInfo *)arg;

	if (NULL == p_UsrInfo || NULL == ch)
    {
        DMX_API_DBG("%s(), line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

	if((DMX_CHANNEL_OUTPUT_FORMAT_TS != ch->usr_param.output_format)
	    ||(DMX_OUTPUT_SPACE_USER != ch->usr_param.output_space))
	{
	    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
	    
	    return(-EPERM);
	}

//    ts_flt_idx = p_UsrInfo->TsFltIdx;

    flt_list_idx = p_UsrInfo->TsFltIdx;
    ts_flt_idx = ch->detail.ts_ch.ts_flt_id[flt_list_idx];	


	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);
	
	p_StatInfo = &ch->detail.ts_ch.stat_info;

	p_StatInfo->TsInCnt = ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info.TsInCnt;

    ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}


__s32 dmx_channel_ts_filter_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                          ret;
	__u32                          ts_flt_idx;
	__u32                          pid_flt_idx;
    __u32 						   flt_list_idx;
	struct Ali_DmxDrvTsFltStatInfo *p_StatInfo;
	struct Ali_DmxDrvTsFltStatInfo *p_UsrInfo;

	p_UsrInfo = (struct Ali_DmxDrvTsFltStatInfo *)arg;

	if (NULL == p_UsrInfo || NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

	if((DMX_CHANNEL_OUTPUT_FORMAT_TS != ch->usr_param.output_format)
	    ||(DMX_OUTPUT_SPACE_USER != ch->usr_param.output_space))
	{
	    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
	    
	    return(-EPERM);
	}    


	//Get the info about filter for Rec(TS)
    //ts_flt_idx = p_UsrInfo->TsFltIdx;
    flt_list_idx = p_UsrInfo->TsFltIdx;
    ts_flt_idx = ch->detail.ts_ch.ts_flt_id[flt_list_idx];	
    
    p_StatInfo = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info;


	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);

	ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}


__s32 dmx_channel_ts_read
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch,
    __s32                     flags,
    char __user              *usr_buf,
    size_t                    usr_rd_len
)
{
   __s32 byte_cnt;
   __s32  ret;
   size_t to_cpy_len;
   __s32  cur_data_len;
   __s32  to_cpy_pkt_cnt;
   
    if (ch->state != DMX_CHANNEL_STATE_RUN)
    {
        return(-EPERM);
    }

    if (0 == usr_rd_len)
    {
        //DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }    

    #if 0 
	if (ch->detail.ts_ch.enc_para != NULL)
	{
	    /* Atmost DMX_DEENCRYPT_BUF_LEN bytes could be read out for one read,
	     * due to DMX_DEENCRYPT_BUF limitation.
		*/
        if (usr_rd_len > DMX_DEENCRYPT_BUF_LEN)
        {
    	   to_cpy_len = DMX_DEENCRYPT_BUF_LEN;
    	}
    	else
    	{
    	   to_cpy_len = usr_rd_len;
    	}

		/* Atmost dmx_data_buf_total_len could be read out.
		*/
	    cur_data_len = dmx_data_buf_total_len(&(ch->data_buf));

		if (cur_data_len < to_cpy_len)
		{
            to_cpy_len = cur_data_len;
		}

		/* Wrap to multiple of 64 pakcets to meet DSC requirement.
		*/
		to_cpy_pkt_cnt = (to_cpy_len / 188);
		
		to_cpy_pkt_cnt /= 64;

		to_cpy_pkt_cnt *= 64;
		
        byte_cnt = dmx_data_buf_rd(ali_dmx_channel_module_legacy.dmx_de_enc_input_buf,
			                       &(ch->data_buf), to_cpy_pkt_cnt * 188, DMX_DATA_CPY_DEST_KERN); 

        if (byte_cnt > 0)
        {
            if (byte_cnt != (to_cpy_pkt_cnt * 188))
            {
                DMX_CH_LEGA_DBG("%s,%d,byte_cnt:%d,to_cpy_pkt_cnt * 188:%d\n", __FUNCTION__, __LINE__, byte_cnt, to_cpy_pkt_cnt * 188);
    
                return(-EFAULT);  
			}

            //DMX_CH_LEGA_DBG("dmx_de_enc_input_buf:%x,dmx_de_enc_output_buf:%x\n", ali_dmx_channel_module_legacy.dmx_de_enc_input_buf,
				    //ali_dmx_channel_module_legacy.dmx_de_enc_output_buf);
			
			#ifdef CONFIG_ALI_DSC
            ret = ali_DeEncrypt(ch->detail.ts_ch.enc_para, ali_dmx_channel_module_legacy.dmx_de_enc_input_buf, 
				                ali_dmx_channel_module_legacy.dmx_de_enc_output_buf, to_cpy_pkt_cnt);

            if (ret != 0)
            {
                DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
    
                return(-EFAULT);     
            }		
            #endif

            ret = copy_to_user(usr_buf, ali_dmx_channel_module_legacy.dmx_de_enc_output_buf, to_cpy_pkt_cnt * 188);
			
            if (ret != 0)
            {
                DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
    
                return(-EFAULT);     
            }			
        }
		else
		{
            DMX_CH_LEGA_DBG("%s,%d,byte_cnt:%d\n", __FUNCTION__, __LINE__,
                            byte_cnt);
		}

		return(to_cpy_pkt_cnt * 188);
	}
	#endif

    /* byte_cnt may be less than need_cpy_len if data_buf contains less
     * data than need_cpy_len required.
     */
    byte_cnt = dmx_data_buf_rd(usr_buf, &(ch->data_buf), usr_rd_len, 
                               DMX_DATA_CPY_DEST_USER); 

    if (byte_cnt < 0)
    {
        DMX_CH_LEGA_DBG("%s,%d,byte_cnt:%d\n", __FUNCTION__, __LINE__,
                      byte_cnt);
    }

    return(byte_cnt);
}


/* TS stream always readable to offload CPU loading from calling of 
 * wake_up_interruptible() for each TS pakcer received, with may be as 
 * much as 30,000 times per second.
*/
__s32 dmx_channel_ts_poll
(
    struct file              *filp,
    struct poll_table_struct *wait,
    struct dmx_output_device *dev,
    struct dmx_channel       *ch
)
{
    return(POLLIN | POLLRDNORM);
}



__s32 dmx_channel_sec_ts_filter_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                          ret;
	__s32                          ts_flt_idx;
//	__u32                          pid_flt_idx;
	struct Ali_DmxDrvTsFltStatInfo *p_StatInfo;

	if ( NULL == ch )
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

	if(DMX_OUTPUT_SPACE_USER != ch->usr_param.output_space)
	{
	    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
	    
	    return(-EPERM);
	}    


	//Get the info about filter for Section filter

    ts_flt_idx = ch->detail.sec_ch.sec_flt_id;
    
    p_StatInfo = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info;

//	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);

	ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}


__s32 dmx_channel_sec_stream_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                            ret;
	__u32                            sec_flt_idx;
	__u32                            ts_flt_idx;
//	__u32                            pid_flt_idx;
	struct dmx_sec_flt               *sec_flt;
	struct dmx_ts_flt                *ts_flt;
	struct Ali_DmxDrvSecStrmStatInfo *p_StatInfo;

	if (0 == arg || NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }
    
	if(DMX_OUTPUT_SPACE_USER != ch->usr_param.output_space)
	{
	    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
	    
	    return(-EPERM);
	}    

    sec_flt_idx = ch->detail.sec_ch.sec_flt_id ;

	ts_flt_idx = dmx_sec_flt_link_ts_flt_idx(sec_flt_idx);

//	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);

	sec_flt = &ali_dmx_sec_flt_module.sec_flt[sec_flt_idx];

	ts_flt = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx];

	p_StatInfo = &ch->detail.sec_ch.stat_info;
	
	p_StatInfo->TsInCnt = ts_flt->stat_info.TsInCnt;

	p_StatInfo->SecInCnt = sec_flt->stat_info.SecInCnt;

	p_StatInfo->SecOutCnt = sec_flt->stat_info.SecOutCnt;
	
    ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }
    
    return(0);
}


__s32 dmx_channel_sec_filter_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                           ret;
	__u32                           sec_flt_idx;
	__u32                           ts_flt_idx;
//	__u32                           pid_flt_idx;
	struct dmx_sec_flt              *sec_flt;
	struct Ali_DmxDrvSecFltStatInfo *p_StatInfo;

	if (0 == arg || NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

	if(DMX_OUTPUT_SPACE_USER != ch->usr_param.output_space)
	{
	    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
	    
	    return(-EPERM);
	} 

	sec_flt_idx = ch->detail.sec_ch.sec_flt_id;

	ts_flt_idx = dmx_sec_flt_link_ts_flt_idx(sec_flt_idx);

//	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);

	sec_flt = &ali_dmx_sec_flt_module.sec_flt[sec_flt_idx];

	p_StatInfo = &sec_flt->stat_info;
	
    ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}



__s32 dmx_channel_sec_wr
(
    __u8                     *src,
    __s32                     len,
    enum DMX_SEC_FLT_CB_TYPE  cb_type,
    __u32                     ch
)
{
    __s32                    ret;
    struct ali_dmx_data_buf *dest_buf;

    ret = 0;

    dest_buf = &(((struct dmx_channel *)ch)->data_buf);

    if (DMX_SEC_FLT_CB_TYPE_ERR == cb_type)
    {
    	//DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);
        dmx_data_buf_drop_incomplete_pkt(dest_buf);
		//DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }
    else if (DMX_SEC_FLT_CB_TYPE_PKT_DATA == cb_type)
    {
        ret = dmx_data_buf_wr_data(dest_buf, src, len, DMX_DATA_SRC_TYPE_KERN);
    
        if (ret < len)
        {
            //DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);
#if 0			
            dmx_data_buf_drop_incomplete_pkt(dest_buf);
#else
            dmx_data_buf_flush_all_pkt(dest_buf);
#endif
        	//DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);
			
            return(ret);
        }
    }
    /*DMX_SEC_FLT_CB_TYPE_PKT_END == cb_type 
    */
    else 
    {   
       ret = dmx_data_buf_wr_pkt_end(dest_buf);

       /* Section data successfully stored in buffer.
       */
       if (0 == ret)
       {
           wake_up_interruptible(&(dest_buf->rd_wq));
       }
    }

    return(ret);
}





__s32 dmx_channel_sec_read
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch,
    __s32                     flags,
    char __user              *usr_buf,
    size_t                    usr_rd_len
)
{
    __u32 pkt_len;

    DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

    if (ch->state != DMX_CHANNEL_STATE_RUN)
    {
        return(-EPERM);
    }

    if (0 == usr_rd_len)
    {
        return(0);
    }    

    for (;;)
    {
        pkt_len = dmx_data_buf_first_pkt_len(&(ch->data_buf));

        if (pkt_len > 0)
        {
            if (pkt_len > usr_rd_len)
            {
                /* Do nothing if user buffer is shorter than pkt len.
                 */
                return(-EFBIG);
            }

            /* Else data could be read out. */
            break;
        }  

        /* pkt_len <= 0, then nothing could be read out.
        */
        if (flags & O_NONBLOCK)
        {
            return(-EAGAIN);
        }    
		
		DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

        dmx_mutex_output_unlock(dev->src_hw_interface_id);

		DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

        if (wait_event_interruptible(ch->data_buf.rd_wq, 
            dmx_data_buf_first_pkt_len(&(ch->data_buf)) > 0))
        {
            return(-ERESTARTSYS);
        }

		DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

        if (dmx_mutex_output_lock(dev->src_hw_interface_id))
        {    
            return(-ERESTARTSYS);
        }    
    }

    DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

    dmx_data_buf_rd(usr_buf, &(ch->data_buf), pkt_len, DMX_DATA_CPY_DEST_USER);

    DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

    return(pkt_len);
}







__s32 dmx_channel_sec_poll
(
    struct file              *filp,
    struct poll_table_struct *wait,
    struct dmx_output_device *dev,
    struct dmx_channel       *ch
)
{
    __u32 pkt_len;
    __s32 mask;
    
    poll_wait(filp, &(ch->data_buf.rd_wq), wait);
      
    mask = 0;

    pkt_len = dmx_data_buf_first_pkt_len(&ch->data_buf);
    
    if (pkt_len > 0)
    {    
        mask |= (POLLIN | POLLRDNORM); 
    }

    return(mask);
}


/**************************************************************
* Add for PES
***************************************************************/
__s32 dmx_channel_pes_stream_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                            ret;
	__u32                            pes_flt_idx;
	__u32                            ts_flt_idx;
//	__u32                            pid_flt_idx;
	struct dmx_pes_flt               *pes_flt;
	struct dmx_ts_flt                *ts_flt;
	struct Ali_DmxDrvPesStrmStatInfo *p_StatInfo;

	if (0 == arg || NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }
    
	if(DMX_OUTPUT_SPACE_USER != ch->usr_param.output_space)
	{
	    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
	    
	    return(-EPERM);
	}    

    pes_flt_idx = ch->detail.pes_ch.pes_flt_id ;

	ts_flt_idx = dmx_pes_flt_link_ts_flt_idx(pes_flt_idx);

//	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);

	pes_flt = &ali_dmx_pes_flt_module.pes_flt[pes_flt_idx];

	ts_flt = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx];

	p_StatInfo = &ch->detail.pes_ch.stat_info;
	
	p_StatInfo->TsInCnt = ts_flt->stat_info.TsInCnt;

	p_StatInfo->PesInCnt = pes_flt->stat_info.PesInCnt;

	p_StatInfo->PesOutCnt = pes_flt->stat_info.PesOutCnt;
	
    ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }
    
    return(0);
}


__s32 dmx_channel_pes_filter_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                           ret;
	__u32                           pes_flt_idx;
	__u32                           ts_flt_idx;
//	__u32                           pid_flt_idx;
	struct dmx_pes_flt              *pes_flt;
	struct Ali_DmxDrvPesFltStatInfo *p_StatInfo;

	if (0 == arg || NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

	if(DMX_OUTPUT_SPACE_USER != ch->usr_param.output_space)
	{
	    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
	    
	    return(-EPERM);
	} 

	pes_flt_idx = ch->detail.pes_ch.pes_flt_id;

	ts_flt_idx = dmx_pes_flt_link_ts_flt_idx(pes_flt_idx);

//	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);

	pes_flt = &ali_dmx_pes_flt_module.pes_flt[pes_flt_idx];

	p_StatInfo = &pes_flt->stat_info;
	
    ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}

__s32 dmx_channel_pes_ts_filter_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                          ret;
	__s32                          ts_flt_idx;
//	__s32                          pid_flt_idx;
	struct Ali_DmxDrvTsFltStatInfo *p_StatInfo;

	if ( NULL == ch )
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

	if(DMX_OUTPUT_SPACE_USER != ch->usr_param.output_space)
	{
	    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
	    
	    return(-EPERM);
	}    


	//Get the info about filter for Section filter

    ts_flt_idx = ch->detail.pes_ch.pes_flt_id;
    
    p_StatInfo = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info;

//	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);

	ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}


__s32 dmx_channel_pes_wr
(
    __u8                     *src,
    __s32                     len,
    enum DMX_PES_FLT_CB_TYPE  cb_type,
    __u32                     ch
)
{
    __s32                    ret;
    struct ali_dmx_data_buf *dest_buf;

    ret = 0;

//    DMX_CH_LEGA_DBG("%s(),L[%d],src[0x%x], len[%d] \n",__FUNCTION__, __LINE__, src, len);

    dest_buf = &(((struct dmx_channel *)ch)->data_buf);

    if (DMX_PES_FLT_CB_TYPE_ERR == cb_type)
    {
		DMX_CH_LEGA_DBG("%s(),L[%d], DMX_PES_FLT_CB_TYPE_ERR \n", __FUNCTION__, __LINE__);
        
        dmx_data_buf_drop_incomplete_pkt(dest_buf);
        
        return(0);
    }
    else if (DMX_PES_FLT_CB_TYPE_PKT_DATA == cb_type)
    {
//    	DMX_CH_LEGA_DBG("%s(),L[%d], PKT_DATA \n", __FUNCTION__,__LINE__);
        
        ret = dmx_data_buf_wr_data(dest_buf, src, len, DMX_DATA_SRC_TYPE_KERN);
    
        if (ret < len)
        {
            //DMX_CH_LEGA_DBG("%s(),L[%d]--2-11- \n", __FUNCTION__, __LINE__);

            dmx_data_buf_flush_all_pkt(dest_buf);
            
            return(ret);
        }
    }
    /*DMX_PES_FLT_CB_TYPE_PKT_END == cb_type  */
    else 
    {   
// 		DMX_CH_LEGA_DBG("%s(),L[%d], PKT_END \n", __FUNCTION__, __LINE__);
    
		ret = dmx_data_buf_wr_pkt_end(dest_buf);

		/* PES data successfully stored in buffer. */
		if (0 == ret)
		{
		   wake_up_interruptible(&(dest_buf->rd_wq));
		}

    }

//	DMX_CH_LEGA_DBG("%s(),L[%d]\n", __FUNCTION__, __LINE__);
    
    return(ret);
}


__s32 dmx_channel_pes_read
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch,
    __s32                     flags,
    char __user              *usr_buf,
    size_t                    usr_rd_len
)
{
    __u32 pkt_len ,len;


    DMX_CH_LEGA_DBG("%s(),L[%d],buf[0x%x], usr_rd_len[%d]\n", \
        				__FUNCTION__, __LINE__, usr_buf, usr_rd_len);

    if (ch->state != DMX_CHANNEL_STATE_RUN)
    {
		DMX_CH_LEGA_DBG("%s(),L[%d], ch_state[%d] \n", __FUNCTION__, __LINE__, ch->state );
        
        return(-EPERM);
    }

    if (0 == usr_rd_len)
    {
    	DMX_CH_LEGA_DBG("%s(),L[%d] \n", __FUNCTION__, __LINE__);
        
        return(-EINVAL);
    }    


	pkt_len = dmx_data_buf_first_pkt_len(&(ch->data_buf));

    DMX_CH_LEGA_DBG("%s(),L[%d], pkt_len[%d], flags[%x] \n", __FUNCTION__, __LINE__, pkt_len, flags);

	/* nothing could be read out.
	*/
	if ((pkt_len <= 0)&&(flags & O_NONBLOCK))
	{
		return(-EAGAIN);
	}   


    if(pkt_len < usr_rd_len)
    {
		len = pkt_len;
    } 
    else
	{
		len = usr_rd_len;
    }


    dmx_data_buf_rd(usr_buf, &(ch->data_buf), len, DMX_DATA_CPY_DEST_USER);

    DMX_CH_LEGA_DBG("%s(),L[%d]\n", __FUNCTION__, __LINE__);

    return(len);
}


__s32 dmx_channel_pes_poll
(
    struct file              *filp,
    struct poll_table_struct *wait,
    struct dmx_output_device *dev,
    struct dmx_channel       *ch
)
{
    __u32 pkt_len;
    __s32 mask;

//	DMX_CH_LEGA_DBG("%s(),L[%d]\n", __FUNCTION__, __LINE__);
    
    poll_wait(filp, &(ch->data_buf.rd_wq), wait);
      
    mask = 0;

    pkt_len = dmx_data_buf_first_pkt_len(&ch->data_buf);
    
    if (pkt_len > 0)
    {    
        mask |= (POLLIN | POLLRDNORM); 
    }

//	DMX_CH_LEGA_DBG("%s(),L[%d]\n", __FUNCTION__, __LINE__);
    
    return(mask);
}



/*******************************************************
*For Video Info
********************************************************/
__s32 dmx_channel_video_stream_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                              ret;
	__u32                              ts_flt_idx;
//	__s32                              pid_flt_idx;
	struct Ali_DmxDrvVideoStrmStatInfo *p_StatInfo;

	if (0 == arg || NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if((DMX_CHANNEL_OUTPUT_FORMAT_TS != ch->usr_param.output_format)
        ||(DMX_OUTPUT_SPACE_KERNEL != ch->usr_param.output_space))
    {
		DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
        
        return(-EPERM);
    }
    

    ts_flt_idx = ch->detail.ts_ch.ts_flt_id[0];

//	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);

	p_StatInfo = (struct Ali_DmxDrvVideoStrmStatInfo *)(&ch->detail.ts_ch.stat_info);

	p_StatInfo->TsInCnt = ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info.TsInCnt;

    ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}


__s32 dmx_channel_video_filter_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                          ret;
	__u32                          ts_flt_idx;

	struct Ali_DmxDrvTsFltStatInfo *p_StatInfo;


	if ( NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

	if((DMX_CHANNEL_OUTPUT_FORMAT_TS != ch->usr_param.output_format)
		||(DMX_OUTPUT_SPACE_KERNEL != ch->usr_param.output_space))
    {
		DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
        
        return(-EPERM);
    }


	//Get the info about filter for Video
    ts_flt_idx = ch->detail.ts_ch.ts_flt_id[0];
    
    p_StatInfo = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info;

	ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}



__s32 dmx_channel_video_see_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                                   ret;
    
	volatile struct Ali_DmxSeePlyChStatInfo *p_StatInfo;

	if (0 == arg || NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }


    if((DMX_CHANNEL_OUTPUT_FORMAT_TS != ch->usr_param.output_format)
        ||(DMX_OUTPUT_SPACE_KERNEL != ch->usr_param.output_space))
    {
		DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
        
        return(-EPERM);
    }
	
	p_StatInfo = &ali_dmx_see_dev[0].see_buf_init.PlyChStatInfo[0];

    ret = copy_to_user((void __user *)arg, (const void *)p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

	return(0);
}


/*******************************************************
*For Audio Info
********************************************************/
__s32 dmx_channel_audio_stream_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                              ret;
	__u32                              ts_flt_idx;
	struct Ali_DmxDrvVideoStrmStatInfo *p_StatInfo;

	if (0 == arg || NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if((DMX_CHANNEL_OUTPUT_FORMAT_TS != ch->usr_param.output_format)
        ||(DMX_OUTPUT_SPACE_KERNEL != ch->usr_param.output_space))
    {
		DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
        
        return(-EPERM);
    }
    

    ts_flt_idx = ch->detail.ts_ch.ts_flt_id[0];

	p_StatInfo = (struct Ali_DmxDrvVideoStrmStatInfo *)(&ch->detail.ts_ch.stat_info);

	p_StatInfo->TsInCnt = ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info.TsInCnt;

    ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}


__s32 dmx_channel_audio_filter_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                          ret;
	__u32                          ts_flt_idx;

	struct Ali_DmxDrvTsFltStatInfo *p_StatInfo;


	if ( NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

	if((DMX_CHANNEL_OUTPUT_FORMAT_TS != ch->usr_param.output_format)
		||(DMX_OUTPUT_SPACE_KERNEL != ch->usr_param.output_space))
    {
		DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
        
        return(-EPERM);
    }


	//Get the info about filter for audio
    ts_flt_idx = ch->detail.ts_ch.ts_flt_id[0];
    
    p_StatInfo = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info;

	ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}



__s32 dmx_channel_audio_see_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                                   ret;
    
	volatile struct Ali_DmxSeePlyChStatInfo *p_StatInfo;

	if (0 == arg || NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }


    if((DMX_CHANNEL_OUTPUT_FORMAT_TS != ch->usr_param.output_format)
        ||(DMX_OUTPUT_SPACE_KERNEL != ch->usr_param.output_space))
    {
		DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
        
        return(-EPERM);
    }
	
	p_StatInfo = &ali_dmx_see_dev[0].see_buf_init.PlyChStatInfo[1];

    ret = copy_to_user((void __user *)arg, (const void *)p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

	return(0);
}


__s32 dmx_channel_pcr_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                            ret;
	__u32                            ts_flt_idx;
	__u32                            pid_flt_idx;
//	__u32                            pcr_flt_idx;    
//	struct dmx_pcr_flt               *pcr_flt;
	struct dmx_ts_flt                *ts_flt;
	struct Ali_DmxDrvPcrStrmStatInfo *p_StatInfo;

	if (0 == arg || NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

#if 0
    if((DMX_CHANNEL_OUTPUT_FORMAT_TS != ch->usr_param.output_format)
        ||(DMX_OUTPUT_SPACE_KERNEL != ch->usr_param.output_space))
    {
		DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);  
        
        return(-EPERM);
    }
#endif

    ts_flt_idx = ch->detail.pcr_ch.pcr_flt_id;
    
	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);
	
//	pcr_flt = &ali_dmx_pcr_flt_module.pcr_flt[pcr_flt_idx];

	ts_flt = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx];

	p_StatInfo = &ch->detail.pcr_ch.stat_info;

	p_StatInfo->TsInCnt = ts_flt->stat_info.TsInCnt;

	p_StatInfo->LastPcrVal = ch->detail.pcr_ch.latest_pcr;
	
	ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }
    
    return(0);
}

__s32 dmx_channel_pcr_filter_info_get
(
    struct dmx_output_device *dev,
    struct dmx_channel        *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                          ret;
	__u32                          ts_flt_idx;

	struct Ali_DmxDrvTsFltStatInfo *p_StatInfo;


	if ( NULL == ch)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }


    if (DMX_OUTPUT_SPACE_KERNEL != ch->usr_param.output_space)
    {
        DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);
    
        return(-EFAULT);
    }


	//Get the info about filter for audio
    ts_flt_idx = ch->detail.pcr_ch.pcr_flt_id;
    
    p_StatInfo = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info;

	ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}



__s32 dmx_channel_start
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    struct dmx_ts_channel     *ts_ch;
    struct dmx_sec_channel    *sec_ch;
    struct dmx_pcr_channel    *pcr_ch;
    struct dmx_pes_channel    *pes_ch;
    struct dmx_channel_param  *usr_para;
    __s32                      ret;
    __u32                      i;
    __u32                      j;
    struct ali_dmx_data_buf   *data_buf;
    struct Ali_DmxSecMaskInfo  sec_mask;
	__u32  ts_src_hw_interface_id;

    /* ch state validation.
    */
    if (ch->state != DMX_CHANNEL_STATE_CFG)
    {
        return(-EPERM);
    }

            //DMX_CH_LEGA_DBG("%s,%d\n",__FUNCTION__,__LINE__);
    usr_para = &(ch->usr_param);

    ret = copy_from_user(usr_para, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    data_buf = &(ch->data_buf);

    switch(usr_para->output_format)
    {
        case DMX_CHANNEL_OUTPUT_FORMAT_TS:
        {
            DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

            ts_ch = &(ch->detail.ts_ch);

            memset(ts_ch, 0, sizeof(struct dmx_ts_channel));

            if (DMX_OUTPUT_SPACE_KERNEL == usr_para->output_space)
            {
                DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

                ret = dmx_ts_flt_register(dev->src_hw_interface_id,
					                    /* App may add bit 13~14 of PID as a 
					                    *  video/audio format identification.
					                    *  Ugly.
				                        */
                                        usr_para->ts_param.pid_list[0] & 0x1FFF, 
                                        dmx_see_buf_wr_ts, dev->src_hw_interface_id);
                
                if (ret < 0)
                {
                    DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
                    
                    return(ret);
                }

                ts_ch->ts_flt_id[0] = ret;

				dmx_ts_flt_start(ts_ch->ts_flt_id[0]);
            }
            else if (DMX_OUTPUT_SPACE_USER == usr_para->output_space)
            {
                //DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

                dmx_data_buf_list_init(&(ch->data_buf));

				dmx_data_buf_list_init(&(ch->data_buf_orig));
				if(1 == usr_para->rec_whole_tp)
				{
				    dmx_dbg_ch_legacy_api_show("%s,line:%d\n", __FUNCTION__, __LINE__);

                   	ret = dmx_pid_flt_register(dev->src_hw_interface_id, DMX_WILDCARD_PID, 
                   							   dmx_channel_ts_wr, (__u32)ch);
                   	
                   	if (ret < 0)
                   	{                   
                   		dmx_dbg_ch_legacy_api_show("%s,%d,src_hw_interface_id:%d\n", 
							                       __FUNCTION__, __LINE__, dev->src_hw_interface_id);
                   	
                   		return(ret);
                   	}

                    ts_ch->ts_flt_id[0] = ret;
					
                    ret = dmx_pid_flt_start(ts_ch->ts_flt_id[0]);
                
                    if (ret < 0)
                    {
                        dmx_dbg_ch_legacy_api_show("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
                
                        return(ret);
                    }
				}
                else
                {
                    dmx_dbg_ch_legacy_api_show("%s,%d,usr_para->dec_para:%d,usr_para->enc_para\n",
						                       __FUNCTION__, __LINE__, usr_para->dec_para,
						                       usr_para->enc_para);

                    if(usr_para->enc_para != NULL)
                    {
                        ts_ch->enc_para = (void *)kmalloc(sizeof(DEEN_CONFIG), GFP_KERNEL);
    
                        if(NULL == ts_ch->enc_para)
                        {
                            DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, -EFAULT);
                        
                            return(-EFAULT);
    					}						
						
                        if (0 != copy_from_user(ts_ch->enc_para, 
							                    (void __user *)(usr_para->enc_para), sizeof(DEEN_CONFIG)))
                        {
                            DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, -EFAULT);
                        
                            return(-EFAULT);
                        } 
                    }

                    for (i = 0; i < usr_para->ts_param.pid_list_len; i++)
                    {
                    	dmx_dbg_ch_legacy_api_show("%s,%d,PID:%d\n", __FUNCTION__, __LINE__,
							                       usr_para->ts_param.pid_list[i]);

						if (1 != usr_para->ts_param.needdiscramble[i])
						{
							ts_src_hw_interface_id = dev->src_hw_interface_id;
						}
						else
						{
							Sed_DmxSee2mainPidAdd(usr_para->ts_param.pid_list[i]);
						
							ts_ch->ts_flt_id2see[i] = dmx_ts_flt_register(dev->src_hw_interface_id, usr_para->ts_param.pid_list[i], 
																			dmx_see_buf_wr_ts,
																			dev->src_hw_interface_id);
							
							if (ts_ch->ts_flt_id2see[i] < 0)
							{
								dmx_dbg_ch_legacy_api_show("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);

								for (j = 0; j < i; j++)
								{
									dmx_ts_flt_unregister(ts_ch->ts_flt_id[j]);
									if (1 == usr_para->ts_param.needdiscramble[j])
									{
										dmx_ts_flt_unregister(ts_ch->ts_flt_id2see[j]);
									}
								}
						
								return(-EMFILE);
							}
						
							ts_src_hw_interface_id = 6;
						}
							
                        ts_ch->ts_flt_id[i] = dmx_ts_flt_register(ts_src_hw_interface_id,
																	usr_para->ts_param.pid_list[i], 
																	dmx_channel_ts_wr, (__u32)ch);
                        if (ts_ch->ts_flt_id[i] < 0)
                        {
                            DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);
                            
                            for (j = 0; j < i; j++)
                            {
                                dmx_ts_flt_unregister(ts_ch->ts_flt_id[j]);
								if (1 == usr_para->ts_param.needdiscramble[j])
								{
									dmx_ts_flt_unregister(ts_ch->ts_flt_id2see[j]);
								}
                            }
                        
                            return(-EMFILE);
                        }
                        else
                        {
							dmx_ts_flt_start(ts_ch->ts_flt_id[i]);

							if (1 == usr_para->ts_param.needdiscramble[i])
							{
					        	dmx_ts_flt_start(ts_ch->ts_flt_id2see[i]);
							}
                        }    
                    }
				}
            }
            else 
            {
                dmx_dbg_ch_legacy_api_show("%s,%d\n", __FUNCTION__, __LINE__);

                return(-EFAULT);
            }
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_SEC:
        {
            DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

            if (DMX_OUTPUT_SPACE_USER != usr_para->output_space)
            {
                DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

                return(-EFAULT);
            }

            sec_ch = &(ch->detail.sec_ch);

            memset(sec_ch, 0, sizeof(struct dmx_sec_channel));

            dmx_data_buf_list_init(&(ch->data_buf));

            /* Section mask translation from legacy section param to
             * current section param.
            */
            memset(&sec_mask, 0, sizeof(sec_mask));

            sec_mask.MatchLen = usr_para->sec_param.mask_len;

            memcpy(&(sec_mask.Mask), &(usr_para->sec_param.mask),
                   sec_mask.MatchLen);

            memcpy(&(sec_mask.Match), &(usr_para->sec_param.value),
                   sec_mask.MatchLen);

            ret = dmx_sec_flt_register(dev->src_hw_interface_id, 
                                       usr_para->sec_param.pid, 
                                       &sec_mask,
                                       dmx_channel_sec_wr,
                                       (__u32)(ch));
            
            if (ret < 0)
            {
                DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
            
                return(ret);
            }

            sec_ch->sec_flt_id = ret;

            dmx_sec_flt_start(sec_ch->sec_flt_id);

        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_PES:
        {
            DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);
        
            if (DMX_OUTPUT_SPACE_USER != usr_para->output_space)
            {
                DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);
        
                return(-EFAULT);
            }
        
            pes_ch = &(ch->detail.pes_ch);
        
            memset(pes_ch, 0, sizeof(struct dmx_pes_channel));
        
            dmx_data_buf_list_init(&(ch->data_buf));
        
            /* Pes mask translation from legacy Pes param to
             * current Pes param.
            */        
            ret = dmx_pes_flt_register(dev->src_hw_interface_id, 
                                       usr_para->pes_param.pid, 
                                       dmx_channel_pes_wr,
                                       (__u32)(ch));
            
            if (ret < 0)
            {
                DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
            
                return(ret);
            }
        
            pes_ch->pes_flt_id = ret;
        
            dmx_pes_flt_start(pes_ch->pes_flt_id);        
        }      
        break;



        case DMX_CHANNEL_OUTPUT_FORMAT_PCR:
        {
            //DMX_CH_LEGA_DBG("%s,%d\n",__FUNCTION__,__LINE__);

            if (DMX_OUTPUT_SPACE_KERNEL != usr_para->output_space)
            {
                DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

                return(-EFAULT);
            }

            pcr_ch = &(ch->detail.pcr_ch);

            memset(pcr_ch, 0, sizeof(struct dmx_pcr_channel));

            //DMX_CH_LEGA_DBG("%s,%d,pid:%d\n",__FUNCTION__,__LINE__, usr_para->pcr_param.pid);

            ret = dmx_pcr_flt_register(dev->src_hw_interface_id, usr_para->pcr_param.pid, 
                                       dmx_channel_pcr_wr, (__u32)(ch), 
                                       NULL, dev->src_hw_interface_id);
            
            if (0 != ret)
            {
                DMX_CH_LEGA_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);
            
                return(ret);
            }

            //DMX_CH_LEGA_DBG("%s,%d,pid:%d\n",__FUNCTION__,__LINE__, usr_para->pcr_param.pid);
			
            pcr_ch->pcr_flt_id = ret;
            
            ret = dmx_pcr_flt_start(pcr_ch->pcr_flt_id);
            
            if (ret < 0)
            {
                DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);
            
                return(ret);
            }       

            //DMX_CH_LEGA_DBG("%s,%d,pid:%d\n",__FUNCTION__,__LINE__, usr_para->pcr_param.pid);
			
        }
        break;

        default:
        {
            return(-EINVAL);
        }
        break;
    }

    ch->state = DMX_CHANNEL_STATE_RUN;

    return(0);
}




__s32 dmx_channel_stop
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch
)
{
    __u32                     i;
    __s32                     ret;
    struct dmx_ts_channel    *ts_ch;
    struct dmx_sec_channel   *sec_ch;
    struct dmx_pes_channel   *pes_ch;
    struct dmx_pcr_channel   *pcr_ch;
    struct dmx_channel_param *usr_para;
	struct dmx_ts_flt		 *ts_flt;
	struct dmx_sec_flt		 *sec_flt;
	struct dmx_pes_flt		 *pes_flt;

    DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

    /* ch state validation, return success in all state.
    */
    if (ch->state != DMX_CHANNEL_STATE_RUN)
    {
        return(0);
    }

    usr_para = &ch->usr_param;

	memset(&g_stat_info, 0, sizeof(g_stat_info));

    switch(usr_para->output_format)
    {
        case DMX_CHANNEL_OUTPUT_FORMAT_TS:
        {
            ts_ch = &(ch->detail.ts_ch);
            
			memset(&ts_ch->stat_info, 0, sizeof(ts_ch->stat_info));

            if (DMX_OUTPUT_SPACE_KERNEL == usr_para->output_space)
            {
                DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

				ts_flt = &ali_dmx_ts_flt_module.ts_flt[ts_ch->ts_flt_id[0]];

				memset(&ts_flt->stat_info, 0, sizeof(ts_flt->stat_info));

                ret = dmx_ts_flt_stop(ts_ch->ts_flt_id[0]);
                
                if (ret < 0)
                {
                    DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
                    
                    return(ret);
                }
            }
            else if (DMX_OUTPUT_SPACE_USER == usr_para->output_space)
            {
				if(1 == usr_para->rec_whole_tp)
				{
				    dmx_dbg_ch_legacy_api_show("%s,%d\n", __FUNCTION__, __LINE__);
                    dmx_pid_flt_stop(ts_ch->ts_flt_id[0]);
				}
				else
				{
				    dmx_dbg_ch_legacy_api_show("%s,%d\n", __FUNCTION__, __LINE__);
                    for (i = 0; i < usr_para->ts_param.pid_list_len; i++)
                    {
    					ts_flt = &ali_dmx_ts_flt_module.ts_flt[ts_ch->ts_flt_id[i]];
    
    					memset(&ts_flt->stat_info, 0, sizeof(ts_flt->stat_info));
    
                        dmx_ts_flt_stop(ts_ch->ts_flt_id[i]);

						if (1 == usr_para->ts_param.needdiscramble[i])
						{
				        	dmx_ts_flt_stop(ts_ch->ts_flt_id2see[i]);
						}						
                    }
				}
                                   
                dmx_data_buf_flush_all_pkt(&ch->data_buf);
				dmx_data_buf_flush_all_pkt(&ch->data_buf_orig);
            }
            else 
            {
                dmx_dbg_ch_legacy_api_show("%s,%d\n", __FUNCTION__, __LINE__);

                return(-EFAULT);
            }
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_SEC:
        {
            DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);
            
            if (DMX_OUTPUT_SPACE_USER != usr_para->output_space)
            {
                DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

                return(-EFAULT);
            }
            
            sec_ch = &(ch->detail.sec_ch);

			memset(&sec_ch->stat_info, 0, sizeof(sec_ch->stat_info));
            
			sec_flt = &ali_dmx_sec_flt_module.sec_flt[sec_ch->sec_flt_id];

			memset(&sec_flt->stat_info, 0, sizeof(sec_flt->stat_info));

            ret = dmx_sec_flt_stop(sec_ch->sec_flt_id);
            
            if (ret < 0)
            {
                DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
                
                return(ret);
            }
            
            dmx_data_buf_flush_all_pkt(&ch->data_buf);

            DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_PES:
		{
		    DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);
		    
		    if (DMX_OUTPUT_SPACE_USER != usr_para->output_space)
		    {
		        DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

		        return(-EFAULT);
		    }
		    
		    pes_ch = &(ch->detail.pes_ch);
		    
			memset(&pes_ch->stat_info, 0, sizeof(pes_ch->stat_info));
            
			pes_flt = &ali_dmx_pes_flt_module.pes_flt[pes_ch->pes_flt_id];

			memset(&pes_flt->stat_info, 0, sizeof(pes_flt->stat_info));

		    ret = dmx_pes_flt_stop(pes_ch->pes_flt_id);
		    
		    if (ret < 0)
		    {
		        DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
		        
		        return(ret);
		    }
		    
		    dmx_data_buf_flush_all_pkt(&ch->data_buf);

		    DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);
		}
        break;        

        case DMX_CHANNEL_OUTPUT_FORMAT_PCR:
        {
            DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

            if (DMX_OUTPUT_SPACE_KERNEL != usr_para->output_space)
            {
                DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

                return(-EFAULT);
            }

            pcr_ch = &(ch->detail.pcr_ch);

			memset(&pcr_ch->stat_info, 0, sizeof(pcr_ch->stat_info));

            ret = dmx_pcr_flt_stop(pcr_ch->pcr_flt_id);
            
            if (ret < 0)
            {
                DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
                
                return(ret);
            }
        }
        break;

        default:
        {
            DMX_CH_LEGA_DBG("%s,line:%d,format:%d\n", __FUNCTION__, __LINE__, usr_para->output_format);
            
            ret = -EINVAL;
        }
        break;
    }

    ch->state = DMX_CHANNEL_STATE_STOP;

    return(0);
}



__s32 dmx_channel_open
(
    struct inode *inode,
    struct file  *file
)
{
    __u32                     i;
    struct dmx_output_device *dev;
    struct dmx_channel       *ch;

    //DMX_CH_LEGA_DBG("%s, go\n", __FUNCTION__);

    /* For standby test. Root cause still unkonwn.
     * Date:2014.11.13
     */	
    //dmx_see_init();
    dmx_internal_init();

    dev = container_of(inode->i_cdev, struct dmx_output_device, cdev);

    if (dmx_mutex_output_lock(dev->src_hw_interface_id))
    {
        return(-ERESTARTSYS);
    }

    for (i = 0; i < DMX_CHANNEL_CNT; i++)
    {
        ch = &(ali_dmx_channel_module_legacy.channel[i]);

        if (DMX_CHANNEL_STATE_IDLE == ch->state)
        {
            ch->state = DMX_CHANNEL_STATE_CFG;

            ch->dmx_output_device = (void *)dev;

            file->private_data = ch;

            //DMX_CH_LEGA_DBG("Got idle stream %d\n", i);

            break;
        }
    }

    dmx_mutex_output_unlock(dev->src_hw_interface_id);

    if (i >= DMX_CHANNEL_CNT)
    {
        //DMX_CH_LEGA_DBG("No idle ch!\n");

        return(-EMFILE);
    }

    return(0);
}




__s32 dmx_channel_close
(
    struct inode *inode,
    struct file  *filep
)
{
    __s32                     ret;
    __u32                     i;
    struct dmx_channel       *ch;
    struct dmx_ts_channel    *ts_ch;
    struct dmx_sec_channel   *sec_ch;
    struct dmx_pes_channel   *pes_ch;
    struct dmx_pcr_channel   *pcr_ch;
    struct dmx_channel_param *usr_para;
    struct dmx_output_device *dev;

    ch = filep->private_data;

    dev = (struct dmx_output_device *)ch->dmx_output_device;

    if (dmx_mutex_output_lock(dev->src_hw_interface_id))
    {
        return(-ERESTARTSYS);
    }

    ret = 0;
    
    usr_para = &(ch->usr_param);

    switch(usr_para->output_format)
    {
        case DMX_CHANNEL_OUTPUT_FORMAT_TS:
        {
            ts_ch = &(ch->detail.ts_ch);
            
            if (DMX_OUTPUT_SPACE_KERNEL == usr_para->output_space)
            {
                DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

                ret = dmx_ts_flt_unregister(ts_ch->ts_flt_id[0]);
                
                if (ret < 0)
                {
                    DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);
                }
            }
            else if (DMX_OUTPUT_SPACE_USER == usr_para->output_space)
            {
                DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

				if(1 == usr_para->rec_whole_tp)
				{
				    dmx_dbg_ch_legacy_api_show("%s,%d\n", __FUNCTION__, __LINE__);
                    dmx_pid_flt_unregister(ts_ch->ts_flt_id[0]);
				}				
                else
                {
                    dmx_dbg_ch_legacy_api_show("%s,%d\n", __FUNCTION__, __LINE__);
                    for (i = 0; i < usr_para->ts_param.pid_list_len; i++)
                    {
                        ret = dmx_ts_flt_unregister(ts_ch->ts_flt_id[i]);
    
                        if (ret < 0)
                        {
                            DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);
                        }

						/* Need Discrambling?
						*/
						if (1 == usr_para->ts_param.needdiscramble[i])
						{
				        	dmx_ts_flt_unregister(ts_ch->ts_flt_id2see[i]);
							
						    Sed_DmxSee2mainPidDel(usr_para->ts_param.pid_list[i]);
						}						
                    }
				}

                if (DMX_CHANNEL_STATE_RUN == ch->state)
                {
                    dmx_data_buf_flush_all_pkt(&ch->data_buf);
					dmx_data_buf_flush_all_pkt(&ch->data_buf_orig);
                }
				
                if(NULL != ts_ch->enc_para)
                {
                    kfree(ts_ch->enc_para);
					ts_ch->enc_para = NULL;
                    dmx_dbg_ch_legacy_api_show("%s,%d\n", __FUNCTION__, __LINE__);
				}					
            }
            else 
            {
                dmx_dbg_ch_legacy_api_show("%s,%d\n", __FUNCTION__, __LINE__);

                ret = -EFAULT;
            }
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_SEC:
        {
            DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);
            
            if (DMX_OUTPUT_SPACE_USER != usr_para->output_space)
            {
                DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

                ret = -EFAULT;
            }
            
            sec_ch = &(ch->detail.sec_ch);
            
            ret = dmx_sec_flt_unregister(sec_ch->sec_flt_id);
            
            if (ret < 0)
            {
                DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
            }

            if (DMX_CHANNEL_STATE_RUN == ch->state)
            {
                dmx_data_buf_flush_all_pkt(&ch->data_buf);
            }
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_PES:
        {
            DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);
            
            if (DMX_OUTPUT_SPACE_USER != usr_para->output_space)
            {
                DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);
        
                ret = -EFAULT;
            }
            
            pes_ch = &(ch->detail.pes_ch);
            
            ret = dmx_pes_flt_unregister(pes_ch->pes_flt_id);
            
            if (ret < 0)
            {
                DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
            }
        
            if (DMX_CHANNEL_STATE_RUN == ch->state)
            {
                dmx_data_buf_flush_all_pkt(&ch->data_buf);
            }
        }

        break;


        case DMX_CHANNEL_OUTPUT_FORMAT_PCR:
        {
            DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

            if (DMX_OUTPUT_SPACE_KERNEL != usr_para->output_space)
            {
                DMX_CH_LEGA_DBG("%s,line:%d\n", __FUNCTION__, __LINE__);

                ret = -EFAULT;
            }
            else
            {
                pcr_ch = &(ch->detail.pcr_ch);
                
                ret = dmx_pcr_flt_unregister(pcr_ch->pcr_flt_id);
                
                if (ret < 0)
                {
                    DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
                }
            }
        }
        break;


        default:
        {
            DMX_CH_LEGA_DBG("%s,line:%d,format:%d\n", __FUNCTION__, __LINE__, usr_para->output_format);
            
            ret = -EINVAL;
        }
        break;
    }

    ch->state = DMX_CHANNEL_STATE_IDLE;

    dmx_mutex_output_unlock(dev->src_hw_interface_id);

    return(ret);

}


__s32 dmx_channel_read
(
    struct file *file,
    char __user *buf,
    size_t       count,
    loff_t      *ppos
)
{
    __s32                     ret;
    struct dmx_output_device *dev;
    struct dmx_channel       *ch; 
    struct dmx_channel_param *usr_para;

    DMX_CH_LEGA_DBG("%s, %d\n", __FUNCTION__, __LINE__);

    ch = file->private_data;

    dev = (struct dmx_output_device *)ch->dmx_output_device;

    if (dmx_mutex_output_lock(dev->src_hw_interface_id))
    {
        return(-ERESTARTSYS);
    }

    ret = 0;

    usr_para = &ch->usr_param;

    switch(usr_para->output_format)
    {
        //DMX_CH_LEGA_DBG("%s, %d\n", __FUNCTION__, __LINE__);

        case DMX_CHANNEL_OUTPUT_FORMAT_TS:
        {
            //DMX_CH_LEGA_DBG("%s, %d\n", __FUNCTION__, __LINE__);
            
            ret = dmx_channel_ts_read(dev, ch, file->f_flags, buf, count);
        }
        break; 

        case DMX_CHANNEL_OUTPUT_FORMAT_SEC:
        {
            DMX_CH_LEGA_DBG("%s(), %d,DMX_CHANNEL_OUTPUT_FORMAT_SEC \n", __FUNCTION__, __LINE__);
            
            ret = dmx_channel_sec_read(dev, ch, file->f_flags, buf, count);
			
            DMX_CH_LEGA_DBG("%s(), %d, DMX_CHANNEL_OUTPUT_FORMAT_SEC \n", __FUNCTION__, __LINE__);
        }
        break; 

        case DMX_CHANNEL_OUTPUT_FORMAT_PES:
        {
            DMX_CH_LEGA_DBG("%s(), %d, DMX_CHANNEL_OUTPUT_FORMAT_PES \n", __FUNCTION__, __LINE__);
            
            ret = dmx_channel_pes_read(dev, ch, file->f_flags, buf, count);
			
            DMX_CH_LEGA_DBG("%s(), %d, DMX_CHANNEL_OUTPUT_FORMAT_PES\n", __FUNCTION__, __LINE__);
        }
        break; 


        default:
        {
            DMX_CH_LEGA_DBG("%s, %d\n", __FUNCTION__, __LINE__);
            
            ret = -EPERM;
        }
        break;  
    }

    dmx_mutex_output_unlock(dev->src_hw_interface_id);

    DMX_CH_LEGA_DBG("%s, %d\n", __FUNCTION__, __LINE__);

    return(ret);
}


__s32 dmx_tp_autoscan_write
(
    struct dmx_ts_pkt_inf *pkt_inf,
    __u32                  param
)
{
    __u32                            data_len;
    __s32                            byte_cnt;
    struct ali_dmx_data_buf         *dest_buf;
    struct dmx_tp_autoscan          *autoscan;

    //DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);
	
	autoscan = (struct dmx_tp_autoscan *)param;

    dest_buf = &(autoscan->tp_data_buf);

    byte_cnt = dmx_data_buf_wr_data(dest_buf, pkt_inf->pkt_addr, 188, 
                                    DMX_DATA_SRC_TYPE_KERN);

	if (byte_cnt < 0)
	{		
        DMX_CH_LEGA_DBG("%s,%d,wr len:%d,buf len:%d,ret:%d,pkt_inf:%x\n",
               __FUNCTION__, __LINE__, 188, 
               dest_buf->cur_len, byte_cnt, (__u32)pkt_inf);

        /* Must be buffer overflow, flush all section data of this stream 
         * to free memory.
		 */
		dmx_data_buf_flush_all_pkt(dest_buf);		
	}

    #if 0
    /* Wakeup waiting poll only if we are storing into an empty buffer,
     * to offload CPU loading.
    */
    if (data_len <= 0)
    {
        //DMX_CH_LEGA_DBG("%s,%d,data_len:%d\n", __FUNCTION__, __LINE__, data_len);
    	
        wake_up_interruptible(&(dest_buf->rd_wq));
    }
	#endif

    data_len = dmx_data_buf_total_len(dest_buf);

    //DMX_CH_LEGA_DBG("%s,%d,data_len:%d\n", __FUNCTION__, __LINE__, data_len);

    return(byte_cnt);
}




__s32 dmx_tp_autoscan_read
(
    void
)
{
    __s32                   byte_cnt;
    struct dmx_tp_autoscan *autoscan;
	
    //DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

	autoscan = &(ali_dmx_channel_module_legacy.autoscan);
    	
    byte_cnt = dmx_data_buf_rd(autoscan->tp_scan_buf, &(autoscan->tp_data_buf), 660 * 188, 
    						   DMX_DATA_CPY_DEST_KERN); 
    
    if (byte_cnt < 0)
    {
    	DMX_CH_LEGA_DBG("%s,%d,byte_cnt:%d\n", __FUNCTION__, __LINE__, byte_cnt);
    }

    //DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

    return(byte_cnt / 188);
}




__s32 dmx_tp_autoscan_stop
(
    void
)
{
    __s32                   ret;
    struct dmx_tp_autoscan *autoscan;
	
    //DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

	autoscan = &(ali_dmx_channel_module_legacy.autoscan);

    if (autoscan->state != DMX_TPSCAN_STATE_RUN)
    {
        return(-EPERM);
    }		
    
    ret = dmx_pid_flt_stop(autoscan->tp_filter_id);

    if (ret < 0)
    {
        DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);

        return(ret);
    }

    ret = dmx_pid_flt_unregister(autoscan->tp_filter_id);

    if (ret < 0)
    {
        DMX_CH_LEGA_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);

        return(ret);
    }
	
    dmx_data_buf_flush_all_pkt(&(autoscan->tp_data_buf));

    autoscan->state = DMX_TPSCAN_STATE_IDLE;
	
    //DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);
	
	return(0);
}




__s32 dmx_tp_autoscan_start
(
    struct dmx_output_device *dev,
    struct dmx_channel       *ch,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                     ret;
    struct dmx_tp_autoscan   *autoscan;
	__s32                     byte_cnt;

    //DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

	autoscan = &(ali_dmx_channel_module_legacy.autoscan);

    if (autoscan->state != DMX_TPSCAN_STATE_IDLE)
    {
        return(-EPERM);
    }	
	
    autoscan->tp_scan_buf = arg;
	autoscan->tp_scan_buf_wr = 0;
	
    dmx_data_buf_list_init(&(autoscan->tp_data_buf));
    
    autoscan->tp_filter_id = dmx_pid_flt_register(dev->src_hw_interface_id, DMX_WILDCARD_PID, 
    							                  dmx_tp_autoscan_write, (__u32)(autoscan));
    
    if (autoscan->tp_filter_id < 0)
    {        
    	DMX_CH_LEGA_DBG("%s,%d,tp_filter_id:%d\n", __FUNCTION__, __LINE__, autoscan->tp_filter_id);
    
    	return(autoscan->tp_filter_id);
    }	

    ret = dmx_pid_flt_start(autoscan->tp_filter_id);

    if (ret < 0)
    {
        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);

        return(ret);
    }

    //DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

    autoscan->state = DMX_TPSCAN_STATE_RUN;
	
    //DMX_CH_LEGA_DBG("%s,%d,byte_cnt:%d\n", __FUNCTION__, __LINE__, byte_cnt);
	
	return(0);
}


__s32 dmx_tp_autoscan_task
(
    void *param
)
{
    __s32                     ret;
    struct dmx_tp_autoscan   *autoscan;
	__s32                     byte_cnt;

    //DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

	autoscan = &(ali_dmx_channel_module_legacy.autoscan);
	
    for(;;)
    {
        msleep_interruptible(5);

        if (dmx_mutex_output_lock(0))
        {
            return(-ERESTARTSYS);
        }
    	
		if (DMX_TPSCAN_STATE_RUN == autoscan->state)
		{
    		if (autoscan->tp_scan_buf_wr < (660 * 188))
    		{
                byte_cnt = dmx_data_buf_rd(autoscan->tp_scan_buf + autoscan->tp_scan_buf_wr, &(autoscan->tp_data_buf), 660 * 188, 
                						   DMX_DATA_CPY_DEST_KERN);             
                if (byte_cnt <= 0)
                {
                	//DMX_CH_LEGA_DBG("%s,%d,byte_cnt:%d\n", __FUNCTION__, __LINE__, byte_cnt);
                }
    			else
    			{
                    autoscan->tp_scan_buf_wr += byte_cnt;
                	//DMX_CH_LEGA_DBG("%s,%d,tp_scan_buf_wr:%d\n", __FUNCTION__, __LINE__, autoscan->tp_scan_buf_wr);				
    			}
    		}	
		}
		
		dmx_mutex_output_unlock(0);
    }

	return;
}


#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long dmx_channel_ioctl
(
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
#else
__s32 dmx_channel_ioctl
(
    struct inode *inode,
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
#endif
{
    struct dmx_channel          *ch; 
    struct dmx_output_device    *dev;
    __s32                        ret;
    struct Ali_DmxSeeStatistics  see_statistics;

    ret = 0;

    ch = filp->private_data;

    dev = (struct dmx_output_device *)ch->dmx_output_device;

    if (dmx_mutex_output_lock(dev->src_hw_interface_id))
    {
        return(-ERESTARTSYS);
    }

    //DMX_CH_LEGA_DBG("%s,%d,cmd:%x\n", __FUNCTION__, __LINE__, cmd);

    switch(cmd)
    {
        case ALI_DMX_CHANNEL_START:
        {
            ret = dmx_channel_start(dev, ch, cmd, arg);
        }
        break;

        case ALI_DMX_CHANNEL_STOP:
        {
            ret = dmx_channel_stop(dev, ch);
        }
        break;

        case ALI_DMX_CHANNEL_GET_CUR_PKT_LEN:
        {
            ret = dmx_channel_get_pkt_len(dev, ch, cmd, arg);
        }
        break;

        case ALI_DMX_HW_GET_FREE_BUF_LEN:
        {
            ret = dmx_hw_buf_free_len(dev, ch, cmd, arg);
        }
        break;

        case ALI_DMX_GET_PROG_BITRATE:
        {
            ret = dmx_bitrate_get(dev, ch, cmd, arg);
        }
        break;

        case ALI_DMX_IO_SET_BYPASS_MODE:
        {
			ret = dmx_tp_autoscan_start(dev, ch, cmd, arg);
        }
        break;
        
        case ALI_DMX_IO_BYPASS_GETDATA:
        {		
			ret = dmx_tp_autoscan_read();
        }
        break;	
        
        case ALI_DMX_IO_CLS_BYPASS_MODE:
        {	
			ret = dmx_tp_autoscan_stop();
        }
        break;
              
        case ALI_DMX_BYPASS_ALL:
        {
            DMX_CH_LEGA_DBG("%s,%d,arg:%x\n", __FUNCTION__, __LINE__, (__s32)arg);
        }
        break;
		
        case ALI_DMX_RESET_BITRATE_DETECT:
        {

        }
        break;
		
        case ALI_DMX_SET_HW_INFO:
        {

        }
        break;

		/* Add for sat2ip
		*  TODO:implement
		*/
        case ALI_DMX_CHANNEL_ADD_PID:
        {
            //ret = dmx_channel_add_pid(dmx, channel, cmd, arg);
            //break;
            return RET_SUCCESS;              
        }
        break;

		/* Add for sat2ip
		*  TODO:implement
		*/		
        case ALI_DMX_CHANNEL_DEL_PID:
        {
            //ret = dmx_channel_del_pid(dmx, channel, cmd, arg);
            //break;
            return RET_SUCCESS;    			
        }			
        break;


        /* All below commnad are Debug statistics, not direcly related to 
         * legacy commands.
		*/
#if 0
        case ALI_DMX_GET_LATEST_PCR:
        {
             latest_pcr = dmx_pcr_get_latest(dmx);
             
             copy_to_user((void *)arg, &latest_pcr, sizeof(UINT32));
             
             break;
        }
#endif

        case ALI_DMX_SEE_GET_STATISTICS:
        {            
             dmx_see_get_statistics(&see_statistics);
             
             ret = copy_to_user((void *)arg, &see_statistics, 
                                sizeof(struct Ali_DmxSeeStatistics));

             if (ret != 0)
             {
                 DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);
         
                 return(-EFAULT);
             }
             
             break;
        }

        case ALI_DMX_CHAN_KERN_GLB_CFG:
        {
			ret = dmx_channel_kern_glb_cfg(dev, ch);
            
			break;
        }

        case ALI_DMX_CHAN_KERN_GLB_START:
        {
			ret = dmx_channel_kern_glb_start(dev, ch);
            
			break;
        }

        case ALI_DMX_CHAN_KERN_GLB_STOP:
        {
			ret = dmx_channel_kern_glb_stop(dev, ch);
            
			break;
        }
        
        case ALI_DMX_CHAN_KERN_GLB_INFO_GET:
        {
			ret = dmx_channel_kern_glb_info_get(dev, ch, cmd, arg);
            
			break;
        }
		
        case ALI_DMX_CHAN_KERN_GLB_REALTIME_SET:
        {
			ret = dmx_channel_kern_glb_realtime_set(dev, ch, cmd, arg);
            
			break;
        }

        case ALI_DMX_CHAN_SEE_GLB_CFG:
        {
			ret = dmx_channel_see_glb_cfg(dev, ch);
            
			break;
        }

        case ALI_DMX_CHAN_SEE_GLB_START:
        {
			ret = dmx_channel_see_glb_start(dev, ch);
            
			break;
        }

        case ALI_DMX_CHAN_SEE_GLB_STOP:
        {
			ret = dmx_channel_see_glb_stop(dev, ch);
            
			break;
        }
        
        case ALI_DMX_CHAN_SEE_GLB_INFO_GET:
        {
			ret = dmx_channel_see_glb_info_get(dev, ch, cmd, arg);
            
			break;
        }

        case ALI_DMX_CHAN_SEE_GLB_REALTIME_SET:
        {
			ret = dmx_channel_see_glb_realtime_set(dev, ch, cmd, arg);
            
			break;
        }

        case ALI_DMX_CHAN_HW_REG_CFG:
        {
			ret = dmx_channel_hw_reg_cfg(dev, ch);
            
			break;
        }

        case ALI_DMX_CHAN_HW_REG_START:
        {
			ret = dmx_channel_hw_reg_start(dev, ch);
            
			break;
        }

        case ALI_DMX_CHAN_HW_REG_STOP:
        {
			ret = dmx_channel_hw_reg_stop(dev, ch);
            
			break;
        }
        
        case ALI_DMX_CHAN_HW_REG_INFO_GET:
        {
			ret = dmx_channel_hw_reg_info_get(dev, ch, cmd, arg);
            
			break;
        }

        case ALI_DMX_CHAN_TS_INFO_GET:
        {
            ret = dmx_channel_ts_info_get(dev, ch, cmd, arg);

            break;
        }

        case ALI_DMX_CHAN_TS_FILTER_INFO_GET:
        {
            ret = dmx_channel_ts_filter_info_get(dev, ch, cmd, arg);
        }
        break;

        case ALI_DMX_CHAN_SEC_TS_FILTER_INFO_GET:
        {
            ret = dmx_channel_sec_ts_filter_info_get(dev, ch, cmd, arg);

            break;
        }

        case ALI_DMX_CHAN_SEC_INFO_GET:
        {
            ret = dmx_channel_sec_stream_info_get(dev, ch, cmd, arg);

            break;
        }

        case ALI_DMX_CHAN_SEC_FILTER_INFO_GET:
        {
            ret = dmx_channel_sec_filter_info_get(dev, ch, cmd, arg);

            break;
        }

        case ALI_DMX_CHAN_PES_TS_FILTER_INFO_GET:
        {
            ret = dmx_channel_pes_ts_filter_info_get(dev, ch, cmd, arg);

            break;
        }
 
        case ALI_DMX_CHAN_PES_INFO_GET:
        {
            ret = dmx_channel_pes_stream_info_get(dev, ch, cmd, arg);

            break;
        }

        case ALI_DMX_CHAN_PES_FILTER_INFO_GET:
        {
            ret = dmx_channel_pes_filter_info_get(dev, ch, cmd, arg);

            break;
        }

        case ALI_DMX_CHAN_VIDEO_INFO_GET:
        {
			ret = dmx_channel_video_stream_info_get(dev, ch, cmd, arg);

            break;
        }

        case ALI_DMX_CHAN_VIDEO_FILTER_INFO_GET:
        {
            ret = dmx_channel_video_filter_info_get(dev, ch, cmd, arg);

            break;
        }

        case ALI_DMX_CHAN_VIDEO_SEE_INFO_GET:
        {
			ret = dmx_channel_video_see_info_get(dev, ch, cmd, arg);

			break;
        }

        case ALI_DMX_CHAN_AUDIO_INFO_GET:
        {
			ret = dmx_channel_audio_stream_info_get(dev, ch, cmd, arg);

            break;
        }

        case ALI_DMX_CHAN_AUDIO_FILTER_INFO_GET:
        {
            ret = dmx_channel_audio_filter_info_get(dev, ch, cmd, arg);

            break;
        }

        case ALI_DMX_CHAN_AUDIO_SEE_INFO_GET:
        {
			ret = dmx_channel_audio_see_info_get(dev, ch, cmd, arg);

			break;
        }

        case ALI_DMX_CHAN_PCR_INFO_GET:
        {
            ret = dmx_channel_pcr_info_get(dev, ch, cmd, arg);

            break;
        }

        case ALI_DMX_CHAN_PCR_FILTER_INFO_GET:
        {
            ret = dmx_channel_pcr_filter_info_get(dev, ch, cmd, arg);

            break;
        }

        default: 
        {
			DMX_CH_LEGA_DBG("%s,%d,cmd:%x\n", __FUNCTION__, __LINE__, cmd);

            ret = -ENOTTY;
        }
        break;
    }

    //DMX_CH_LEGA_DBG("%s,%d,cmd:%x\n", __FUNCTION__, __LINE__, cmd);

    dmx_mutex_output_unlock(dev->src_hw_interface_id);

    return(ret);
}



__u32 dmx_channel_poll
(
    struct file              *filp,
    struct poll_table_struct *wait
)
{
    __s32                     mask;
    struct dmx_channel       *ch; 
    struct dmx_output_device *dev;
    struct dmx_channel_param *usr_para;

    ch = filp->private_data;

	if (DMX_CHANNEL_STATE_RUN != ch->state)
	{
        DMX_CH_LEGA_DBG("%s,%d,state:%d\n", __FUNCTION__, __LINE__, ch->state);

        return(0);
	}	

    dev = (struct dmx_output_device *)ch->dmx_output_device;

    if (dmx_mutex_output_lock(dev->src_hw_interface_id))
    {
        return(0);
    }

    usr_para = &ch->usr_param;
	mask = 0;

    switch(usr_para->output_format)
    {
        case DMX_CHANNEL_OUTPUT_FORMAT_SEC:
        {
            mask = dmx_channel_sec_poll(filp, wait, dev, ch);
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_PES:
        {
            mask = dmx_channel_pes_poll(filp, wait, dev, ch);
        }
        break;

        case DMX_CHANNEL_OUTPUT_FORMAT_TS:
        {
            mask = dmx_channel_ts_poll(filp, wait, dev, ch);
        }
        break;

        default:
        {
            DMX_CH_LEGA_DBG("%s,line:%d,format:%d\n", __FUNCTION__, __LINE__, usr_para->output_format);

            //mask = -EINVAL;
        }
        break;
    }

    dmx_mutex_output_unlock(dev->src_hw_interface_id);

    return(mask);
}





extern unsigned long __G_ALI_MM_DMX_MEM_SIZE;
extern unsigned long __G_ALI_MM_DMX_MEM_START_ADDR;

__s32 dmx_channel_module_legacy_init
(
    void
)
{
    __u32 i;
	struct task_struct *p;
	struct sched_param param = {.sched_priority = MAX_RT_PRIO - 1};

    for (i = 0; i < DMX_CHANNEL_CNT; i++)
    {
        ali_dmx_channel_module_legacy.channel[i].state = DMX_CHANNEL_STATE_IDLE;
    }

	ali_dmx_channel_module_legacy.dmx_de_enc_input_buf = (__G_ALI_MM_DMX_MEM_START_ADDR + __G_ALI_MM_DMX_MEM_SIZE - (3 * DMX_SEE_BUF_SIZE) - DMX_DEENCRYPT_BUF_LEN);
	ali_dmx_channel_module_legacy.dmx_de_enc_output_buf = ali_dmx_channel_module_legacy.dmx_de_enc_input_buf - DMX_DEENCRYPT_BUF_LEN;
		
    //DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

	p = kthread_create(dmx_tp_autoscan_task, NULL, "audoscan");

	if (IS_ERR(p))
	{
		return(PTR_ERR(p));	
	}

	sched_setscheduler(p, SCHED_RR, &param);

	wake_up_process(p);
	
    //DMX_CH_LEGA_DBG("%s,%d\n", __FUNCTION__, __LINE__);

    return(0);
}
#endif

