/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_s3503_linux.c
*
*    Description:  s3503 nim driver for linux api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#include "nim_s3503.h"
#include "../tun_common.h"


#define TWO_TUNER_SUPPORT
#define MAX_TUNER_SUPPORT_NUM 	1
#define ALI_NIM_DEVICE_NAME 	"ali_nim_m3503"


struct nim_device 				ali_m3503_nim_dev[MAX_TUNER_SUPPORT_NUM];
struct nim_s3501_private 		*ali_m3503_nim_priv[MAX_TUNER_SUPPORT_NUM] = {NULL};
struct class 					*ali_m3503_nim_class;
struct device 					*ali_m3503_nim_dev_node[MAX_TUNER_SUPPORT_NUM];
static INT32 					fd_dmx;



static char nim_workq[MAX_TUNER_SUPPORT_NUM][5] =
{
    "nim0"
};

static char as_workq[MAX_TUNER_SUPPORT_NUM][12] =
{
    "as_workq0"
};




static void nim_s3503_task_open(struct work_struct *nim_work)
{
    struct nim_device 				*dev;
    dev = container_of((void *)nim_work, struct nim_device, work);

    PRINTK_INFO("[kangzh]line=%d,%s enter!\n", __LINE__, __FUNCTION__);


    nim_s3503_task((UINT32)dev, 0);


}

static INT32 nim_s3503_autoscan_open(struct work_struct *work)
{
    NIM_AUTO_SCAN_T					*pstauto_scan;
    struct nim_device 				*dev;
    struct nim_s3501_private 		*priv;

    //PRINTK_INFO("[kangzh]line=%d,%s enter!\n",__LINE__, __FUNCTION__);


    dev = container_of((void *)work, struct nim_device, as_work);
    priv = dev->priv;
    pstauto_scan = &priv->as_info;


    return nim_s3503_autoscan(dev, pstauto_scan);

}


static INT32 nim_s3503_task_init(struct nim_device *dev)
{
    UINT8 dev_idx = 0;
    struct nim_s3501_private 	*priv = dev->priv;

    PRINTK_INFO("[%s]line=%d,enter!\n", __FUNCTION__, __LINE__);

    if(priv->dev_idx<MAX_TUNER_SUPPORT_NUM)
    {
		dev_idx = priv->dev_idx;
		priv->dev_idx++;
    }	
    else
    {
		PRINTK_INFO("Failed to dev_idx =%d\n",priv->dev_idx);
        return -1;
    }
    dev->workqueue = create_workqueue(nim_workq[dev_idx]);

    if (!(dev->workqueue))
    {
        PRINTK_INFO("Failed to allocate work queue\n");
        return -1;
    }

    dev->autoscan_work_queue = create_workqueue(as_workq[dev_idx]);
    if (!(dev->autoscan_work_queue))
    {
        PRINTK_INFO("%s in:Failed to create nim autoscan workequeue!\n");
        destroy_workqueue(dev->workqueue);
        return -1;
    }
    init_waitqueue_head(&priv->as_sync_wait_queue);
    //g_work_alive = 1;
    priv->work_alive = 1;
    INIT_WORK(&dev->work, nim_s3503_task_open);
    INIT_WORK(&dev->as_work, nim_s3503_autoscan_open);
    queue_work(dev->workqueue, &dev->work);

    return SUCCESS;
}




static INT32 ali_nim_m3503_hw_initialize(struct nim_device *dev, struct ali_nim_m3501_cfg *nim_cfg)
{
    INT32 	ret = 0;
	TUNER_IO_FUNC               *p_io_func=NULL;
    struct nim_s3501_private 	*priv = dev->priv;

    PRINTK_INFO("[%s]line=%d,enter!\n", __FUNCTION__, __LINE__);

    priv->tuner_config_data.qpsk_config = nim_cfg->qpsk_config;
    priv->tuner_config_data.recv_freq_high = nim_cfg->recv_freq_high;
    priv->tuner_config_data.recv_freq_low = nim_cfg->recv_freq_low;
    priv->ext_dm_config.i2c_base_addr = nim_cfg->demod_i2c_addr;
    priv->ext_dm_config.i2c_type_id = nim_cfg->demod_i2c_id;
    priv->tuner_config.c_tuner_base_addr = nim_cfg->tuner_i2c_addr;
    priv->tuner_config.i2c_type_id = nim_cfg->tuner_i2c_id;
    priv->i2c_type_id = nim_cfg->tuner_i2c_id;

    priv->diseqc_info.diseqc_type = 0;
    priv->diseqc_info.diseqc_port = 0;
    priv->diseqc_info.diseqc_k22 = 0;

    if ((priv->tuner_config_data.qpsk_config & M3501_POLAR_REVERT) == M3501_POLAR_REVERT) //bit4: polarity revert.
        priv->diseqc_info.diseqc_polar = 2;//LNB_POL_V;
    else //default usage, not revert.
        priv->diseqc_info.diseqc_polar = 1;//LNB_POL_H;

    //   priv->diseqc_typex = 0;
    //   priv->diseqc_portx = 0;
    priv->ul_status.m_enable_dvbs2_hbcd_mode = 0;
    priv->ul_status.m_dvbs2_hbcd_enable_value = 0x7f;
    priv->ul_status.nim_s3501_sema = OSAL_INVALID_ID;
    priv->ul_status.s3501_autoscan_stop_flag = 0;
    priv->ul_status.s3501_chanscan_stop_flag = 0;
    priv->ul_status.old_ber = 0;
    priv->ul_status.old_per = 0;
    priv->ul_status.m_hw_timeout_thr = 0;
    priv->ul_status.old_ldpc_ite_num = 0;
    priv->ul_status.c_rs = 0;
    priv->ul_status.phase_err_check_status = 0;
    priv->ul_status.s3501d_lock_status = NIM_LOCK_STUS_NORMAL;
    priv->ul_status.m_s3501_type = 0x00;
    priv->ul_status.m_setting_freq = 123;
    priv->ul_status.m_err_cnts = 0x00;
    priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_NORMAL;
    priv->tsk_status.m_task_id = 0x00;
    priv->t_param.t_aver_snr = -1;
    priv->t_param.t_last_iter = -1;
    priv->t_param.t_last_snr = -1;
    priv->t_param.t_snr_state = 0;
    priv->t_param.t_snr_thre1 = 256;
    priv->t_param.t_snr_thre2 = 256;
    priv->t_param.t_snr_thre3 = 256;
    priv->t_param.t_dynamic_power_en = 0;
    priv->t_param.t_phase_noise_detected = 0;
    priv->t_param.t_reg_setting_switch = 0x0f;
    priv->t_param.t_i2c_err_flag = 0x00;
    priv->flag_id = OSAL_INVALID_ID;

    priv->blscan_mode = NIM_SCAN_SLOW;

	p_io_func = tuner_setup(NIM_DVBS,nim_cfg->tuner_id);
	if(NULL != p_io_func)
	{
		priv->nim_tuner_init = (dvbc_tuner_init_callback*)p_io_func->pf_init;
		priv->nim_tuner_control = p_io_func->pf_control;
		priv->nim_tuner_status = p_io_func->pf_status;
		priv->nim_tuner_close = p_io_func->pf_close;
	}



    // Initial the QPSK Tuner
    if (priv->nim_tuner_init != NULL)
    {
        PRINTK_INFO("[%s]line=%d,nim_tuner_init!\n", __FUNCTION__, __LINE__);
        NIM_PRINTF(" Initial the Tuner \n");
        if (priv->nim_tuner_init(&priv->tuner_id, &(priv->tuner_config)) != SUCCESS)
        {
            NIM_PRINTF("Error: Init Tuner Failure!\n");

            return ERR_NO_DEV;
        }
        priv->tuner_opened = 1;
    }


    nim_s3503_get_type(dev);


    if ((priv->ul_status.m_s3501_type == NIM_CHIP_ID_M3503) && 			// Chip 3501A
            ((priv->tuner_config_data.qpsk_config & 0xc0) == M3501_2BIT_MODE))	//TS 2bit mode
    {
        PRINTK_INFO("[kangzh]line=%d,%s 4!\n", __LINE__, __FUNCTION__);
        //for M3606+M3501A full nim ssi-2bit patch, auto change to 1bit mode.
        priv->tuner_config_data.qpsk_config &= 0x3f; // set to TS 1 bit mode
    }



    nim_s3503_set_acq_workmode(dev, NIM_OPTR_HW_OPEN);

    ret = nim_s3503_hw_check(dev);
    if (ret != SUCCESS)
    {
        NIM_PRINTF("[%s] line=%d,error,back!\n", __FUNCTION__, __LINE__);
        return ret;
    }
    ret = nim_s3503_hw_init(dev);

    nim_s3503_after_reset_set_param(dev);

    nim_s3503_hbcd_timeout(dev, NIM_OPTR_HW_OPEN);

    nim_s3503_task_init(dev);

#ifdef CHANNEL_CHANGE_ASYNC
    nim_flag_create(priv);
#endif
    //NIM_PRINTF("    Leave fuction nim_s3501_open\n");
    PRINTK_INFO("[%s]line=%d,end!\n", __FUNCTION__, __LINE__);
    return RET_SUCCESS;
}

RET_CODE ali_m3503_nim_release(struct inode *inode, struct file *file)
{
    struct nim_device 		  *dev = file->private_data;
    struct nim_s3501_private  *priv = dev->priv;

    PRINTK_INFO("[%s]line=%d,enter!\n", __FUNCTION__, __LINE__);


    if (priv->tuner_opened && priv->nim_tuner_close)
        priv->nim_tuner_close();
    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X90);
    nim_s3503_set_acq_workmode(dev, NIM_OPTR_HW_CLOSE);

    //g_work_alive = 0;
    priv->work_alive = 0;
    if (dev->autoscan_work_queue)
    {
        flush_workqueue(dev->autoscan_work_queue);
        destroy_workqueue(dev->autoscan_work_queue);
        dev->autoscan_work_queue = NULL;
    }

    if (dev->workqueue)
    {
        flush_workqueue(dev->workqueue);
        destroy_workqueue(dev->workqueue);
        dev->workqueue = NULL;
    }

#ifdef CHANNEL_CHANGE_ASYNC
    nim_flag_del(priv);
#endif
    PRINTK_INFO("[%s]line=%d,end!\n", __FUNCTION__, __LINE__);

    return RET_SUCCESS;
}



static int ali_m3503_nim_ioctl(struct file *file, unsigned int cmd, void *parg)
{
    INT32 ret = 0;
    INT32 curfreq = 0;

    struct nim_device 			*dev = file->private_data;
    struct nim_s3501_private 	*priv = dev->priv;
    unsigned long 				arg = (unsigned long) parg;

    switch (cmd)
    {
    case ALI_NIM_HARDWARE_INIT_S:
    {
        struct ali_nim_m3501_cfg nim_param;
		
        if(copy_from_user(&nim_param, parg, sizeof(struct ali_nim_m3501_cfg))>0)
		{
			printk("%s error line%d\n", __FUNCTION__, __LINE__);
			// Invalid user space address
			return -EFAULT;
		}			
        ret = ali_nim_m3503_hw_initialize(dev, &nim_param);
        break;
    }
    case ALI_NIM_SET_POLAR:
    {
        UINT8 polar_param = 0;

        get_user(polar_param, (unsigned char *)parg);
        ret = nim_s3503_set_polar(dev, polar_param);
        break;
    }
    case ALI_NIM_CHANNEL_CHANGE:
    {
        NIM_CHANNEL_CHANGE_T nim_param;

        if(copy_from_user(&nim_param, parg, sizeof(NIM_CHANNEL_CHANGE_T))>0)
		{
			printk("%s error line%d\n", __FUNCTION__, __LINE__);
			// Invalid user space address
			return -EFAULT;
		}			
        return nim_s3503_channel_change(dev, nim_param.freq, nim_param.sym, nim_param.fec);
    }
    case ALI_NIM_GET_LOCK_STATUS:
    {
        UINT8 lock = 0;

        nim_s3503_get_lock(dev, &lock);
        ret = lock;
        break;
    }
    case ALI_NIM_READ_QPSK_BER:
    {
        UINT32 ber = 0;

        nim_s3503_get_ber(dev, &ber);
        ret = ber;
        break;
    }
    case ALI_NIM_READ_RSUB:
    {
        UINT32 per = 0;

        nim_s3503_get_per(dev, &per);
        ret = per;
        break;
    }
    case ALI_NIM_READ_AGC:
    {
        UINT8 agc = 0;

        nim_s3503_get_agc(dev, &agc);
        ret = agc;
        break;
    }
    case ALI_NIM_READ_SNR:
    {
        UINT8 snr = 0;
 
        nim_s3503_get_snr(dev, &snr);
        ret = snr;
        break;
    }
    case ALI_NIM_READ_SYMBOL_RATE:
    {
        UINT32 sym = 0;

        nim_s3503_reg_get_symbol_rate(dev, &sym);
        ret = sym;
        break;
    }
    case ALI_NIM_READ_FREQ:
    {
        UINT32 freq = 0;

        nim_s3503_reg_get_freq(dev, &freq);
        ret = freq;
        break;
    }
    case ALI_NIM_READ_CODE_RATE:
    {
        UINT8 fec = 0;

        nim_s3503_reg_get_code_rate(dev, &fec);
        ret = fec;
        break;
    }
    case ALI_NIM_AUTO_SCAN:          /* Do AutoScan Procedure */
    {
        flush_workqueue(dev->autoscan_work_queue);

        NIM_AUTO_SCAN_T as_load;
        if(copy_from_user(&as_load, parg, sizeof(NIM_AUTO_SCAN_T))>0)
		{
			printk("%s error line%d\n", __FUNCTION__, __LINE__);
			// Invalid user space address
			return -EFAULT;
		}			
        priv->yet_return = FALSE;
        priv->as_info.sfreq = as_load.sfreq;
        priv->as_info.efreq = as_load.efreq;
        priv->as_info.unicable = as_load.unicable;
        priv->as_info.fub = as_load.fub;
        priv->as_info.callback = dvbs_as_cb2_ui;
        //kent
        ret = queue_work(dev->autoscan_work_queue, &dev->as_work);
        break;
    }
    case ALI_NIM_STOP_AUTOSCAN:

        priv->ul_status.s3501_autoscan_stop_flag = arg;
        break;
    case ALI_NIM_DISEQC_OPERATE:
    {
        struct ali_nim_diseqc_cmd dis_cmd;

        if(copy_from_user(&dis_cmd, parg, sizeof(struct ali_nim_diseqc_cmd))>0)
		{
			printk("%s error line%d\n", __FUNCTION__, __LINE__);
			// Invalid user space address
			return -EFAULT;
		}				
			
        switch(dis_cmd.diseqc_type)
        {
        case 1:
            ret = nim_s3503_di_seq_c_operate(dev, dis_cmd.mode, dis_cmd.cmd, dis_cmd.cmd_size);
            break;
        case 2:
            ret = nim_s3503_di_seq_c2x_operate(dev, dis_cmd.mode, \
                                             dis_cmd.cmd, dis_cmd.cmd_size, dis_cmd.ret_bytes, &dis_cmd.ret_len);
            if(copy_to_user(parg, &dis_cmd, sizeof(struct ali_nim_diseqc_cmd))>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}
								
            break;
        default:
            ret = -ENOIOCTLCMD;
            break;
        }
        break;
    }
    case ALI_NIM_SET_NETLINKE_ID:
    {

        priv->blind_msg.port_id = arg;
        break;
    }
    case ALI_NIM_AS_SYNC_WITH_LIB:
    {
        priv->as_status |= 0x01;
        wake_up_interruptible(&priv->as_sync_wait_queue);
        break;
    }
    case ALI_NIM_DRIVER_GET_CUR_FREQ:

        switch (arg)
        {
        case NIM_FREQ_RETURN_SET:
            return priv->ul_status.m_setting_freq;
        case NIM_FREQ_RETURN_REAL:
        default:
            curfreq = (INT32) nim_s3503_get_curfreq(dev);
            return curfreq;
        }
    case ALI_NIM_TURNER_SET_STANDBY:

        if (priv->nim_tuner_control != NULL)
        {
            priv->nim_tuner_control(priv->tuner_id, NIM_TUNER_SET_STANDBY_CMD, 0);
        }

        break;
    case ALI_NIM_DRIVER_GET_ID:

        ret = priv->ul_status.m_s3501_type;
        break;

    case ALI_NIM_REG_RW:
        {
	        UINT8 reg_rw_cmd[16] ={0};
            //printk("[%s] line=%d,ret=%d\n",__FUNCTION__,__LINE__,ret);
	        if(copy_from_user(reg_rw_cmd, (UINT8 *)parg, 16)>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}			
	        if (1 == reg_rw_cmd[0]) // Register Read
	        {
	            ret = nim_reg_read(dev,reg_rw_cmd[1], &(reg_rw_cmd[3]), reg_rw_cmd[2]);
	            if(copy_to_user((UINT8 *)parg, reg_rw_cmd, 16)>0)
				{
					printk("%s error line%d\n", __FUNCTION__, __LINE__);
					// Invalid user space address
					return -EFAULT;
				}
								
	        }
	        else if (2 == reg_rw_cmd[0]) // Register Write
	        {
				// printk("[%s] line=%d,reg_rw_cmd[3]=0x%x,len=%d\n",__FUNCTION__,__LINE__,reg_rw_cmd[3],reg_rw_cmd[2]);
	            ret = nim_reg_write(dev,reg_rw_cmd[1], &(reg_rw_cmd[3]), reg_rw_cmd[2]);
	        }
          

        }
		break;		
    case ALI_NIM_SET_LNB_POWER:
		break;
    case ALI_NIM_GET_LNB_POWER:
		break;
    case ALI_NIM_SET_LNB_FREQS:
		break;
	case ALI_NIM_GET_LNB_FREQS:
		break;
	case ALI_NIM_SET_LOOPTHRU:
		break;
	case ALI_NIM_GET_LOOPTHRU:
        break;   
		
    default:

        ret = -ENOIOCTLCMD;
        break;
    }

    return ret;
}



/*****************************************************************************
* INT32 ali_m3503_nim_open(struct nim_s3501_private *priv)
* Description: S3501 open
*
* Arguments:
*  Parameter1: struct nim_s3501_private *priv
*
* Return Value: INT32
*****************************************************************************/

static RET_CODE ali_m3503_nim_open(struct inode *inode, struct file *file)
{
    //struct nim_s3501_private *priv;
    UINT8 dev_idx = 0;

    dev_idx = MINOR(inode->i_rdev);

    PRINTK_INFO("[%s]line=%d,enter,dev_idx=%d!\n", __FUNCTION__, __LINE__, dev_idx);

    //priv = ali_m3503_nim_priv[dev_idx];

    //PRINTK_INFO("%s\n", __FUNCTION__);
    //ali_m3503_nim_dev[dev_idx].priv = (void *)priv;

    file->private_data = (void *)&ali_m3503_nim_dev[dev_idx];

    PRINTK_INFO("[%s]line=%d,end!\n", __FUNCTION__, __LINE__);
    return RET_SUCCESS;
}


static struct file_operations ali_m3503_nim_fops =
{
    .owner                        = THIS_MODULE,
    .write                        = NULL,
    .unlocked_ioctl                = ali_m3503_nim_ioctl,
    .open                        = ali_m3503_nim_open,
    .release                    =  ali_m3503_nim_release,
};



static int __devinit ali_m3503_nim_init(void)
{
    INT32 ret;
    dev_t devno;
    UINT8 i;
	
    PRINTK_INFO("[%s]line=%d,enter!\n", __FUNCTION__, __LINE__);
    ret = alloc_chrdev_region(&devno, 0, 4, ALI_NIM_DEVICE_NAME);
    if(ret < 0)
    {
        PRINTK_INFO("Alloc device region failed, err: %d.\n", ret);
        return ret;
    }
    ali_m3503_nim_class = class_create(THIS_MODULE, "ali_m3503_nim_class");
    if (IS_ERR(ali_m3503_nim_class))
    {
        PRINTK_INFO("[kangzh]line=%d,%s class_create error,back!\n", __LINE__, __FUNCTION__);
        ret = PTR_ERR(ali_m3503_nim_class);
        return ret;
    }

    for(i = 0; i < MAX_TUNER_SUPPORT_NUM; i++)
    {
        ali_m3503_nim_priv[i] = kmalloc(sizeof(struct nim_s3501_private), GFP_KERNEL);
        if (!ali_m3503_nim_priv[i])
        {
            PRINTK_INFO("kmalloc failed!\n");
            return -ENOMEM;
        }
        comm_memset(ali_m3503_nim_priv[i], 0, sizeof(struct nim_s3501_private));
        mutex_init(&ali_m3503_nim_priv[i]->i2c_mutex);
        ali_m3503_nim_priv[i]->flagid_rwlk = __RW_LOCK_UNLOCKED(ali_m3503_nim_priv[i]->flagid_rwlk);



        cdev_init(&ali_m3503_nim_dev[i].cdev, &ali_m3503_nim_fops);
        ali_m3503_nim_dev[i].cdev.owner = THIS_MODULE;
        ali_m3503_nim_dev[i].cdev.ops = &ali_m3503_nim_fops;

        ali_m3503_nim_dev[i].priv = (void *)ali_m3503_nim_priv[i];

        ret = cdev_add(&ali_m3503_nim_dev[i].cdev, devno + i, 1);
        if(ret)
        {
            PRINTK_INFO("Alloc NIM device failed, err: %d.\n", ret);
            mutex_destroy(&ali_m3503_nim_priv[i]->i2c_mutex);
            kfree(ali_m3503_nim_priv[i]);
        }

        ali_m3503_nim_dev_node[i] = device_create(ali_m3503_nim_class, NULL, MKDEV(MAJOR(devno), i),
                                    &ali_m3503_nim_dev[i], "ali_m3503_nim%d", i);
        if(IS_ERR(ali_m3503_nim_dev_node[i]))
        {
            PRINTK_INFO("device_create() failed!\n");
            ret = PTR_ERR(ali_m3503_nim_dev_node[i]);
            cdev_del(&ali_m3503_nim_dev[i].cdev);
            mutex_destroy(&ali_m3503_nim_priv[i]->i2c_mutex);
            kfree(ali_m3503_nim_priv[i]);
        }
    }
    PRINTK_INFO("[%s]line=%d,end!\n", __FUNCTION__, __LINE__);
    return ret;
}

static void __exit ali_m3503_nim_exit(void)
{
    UINT8 i;

    PRINTK_INFO("[%s]line=%d,enter!\n", __FUNCTION__, __LINE__);

    if(ali_m3503_nim_class != NULL)
        class_destroy(ali_m3503_nim_class);

    for (i = 0; i < MAX_TUNER_SUPPORT_NUM; i++)
    {
        if(ali_m3503_nim_dev_node[i] != NULL)
            device_del(ali_m3503_nim_dev_node[i]);


        cdev_del(&ali_m3503_nim_dev[i].cdev);
        mutex_destroy(&ali_m3503_nim_priv[i]->i2c_mutex);

        if (ali_m3503_nim_dev[i].autoscan_work_queue)
            destroy_workqueue(ali_m3503_nim_dev[i].autoscan_work_queue);
        if (ali_m3503_nim_dev[i].workqueue)
            destroy_workqueue(ali_m3503_nim_dev[i].workqueue);

        kfree(ali_m3503_nim_priv[i]);
    }
    PRINTK_INFO("[%s]line=%d,end!\n", __FUNCTION__, __LINE__);

}





module_init(ali_m3503_nim_init);
module_exit(ali_m3503_nim_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kent Kang");
MODULE_DESCRIPTION("Ali M3503 full NIM driver");





