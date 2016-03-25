/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: nim_s3281_linux.c
*
*    Description: s3281 nim driver for linux api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/



#include "nim_s3281.h"







#define ALI_NIM_DEVICE_NAME         "ali_nim_m3281"

#define MAX_TUNER_SUPPORT_NUM         2//1
#define NIM_TUNER_SET_STANDBY_CMD    0xffffffff


static struct nim_device             ali_m3281_nim_dev;
struct nim_s3281_private             *ali_m3281_nim_priv = NULL;
static struct class                 *ali_m3281_nim_class;
static struct device                 *ali_m3281_nim_dev_node;
static UINT8                         nim_sw_test_tread_status = 0;


static void nim_s3281_task_open(struct work_struct *nim_work)
{
    struct nim_device                 *dev;
    dev = container_of((void *)nim_work, struct nim_device, work);

    S3281_PRINTF("[kangzh]line=%d,%s enter!\n", __LINE__, __FUNCTION__);


    nim_s3281_dvbc_task((UINT32)dev, 0);


}

static void nim_s3281_task(struct work_struct *nim_work)
{
    struct nim_device 				*dev = NULL;
    dev = container_of((void *)nim_work, struct nim_device, work);

    S3281_PRINTF("[kangzh]line=%d,%s enter!\n", __LINE__, __FUNCTION__);


    nim_s3281_task_proc((UINT32)dev, 0);
}

INT32 nim_s3281_task_init(struct nim_device *dev)
{
   

    S3281_PRINTF("[%s]line=%d,enter!\n", __FUNCTION__, __LINE__);


    dev->workqueue = create_workqueue("nim0");
    if (!(dev->workqueue))
    {
        S3281_PRINTF("Failed to allocate work queue\n");
        return -1;
    }


    INIT_WORK(&dev->work, nim_s3281_task);
    nim_sw_test_tread_status = 1;
    queue_work(dev->workqueue, &dev->work);

    return SUCCESS;
}

static void nim_s3281_set_config(struct nim_device *dev, struct ali_nim_m3200_cfg *nim_cfg)
{
    struct nim_s3281_private 	*priv = dev->priv;
    TUNER_IO_FUNC               *p_io_func=NULL;

    memcpy((void *) & (priv->tuner_config_data), (void *) & (nim_cfg->tuner_config_data), sizeof(struct QAM_TUNER_CONFIG_DATA));
    memcpy((void *) & (priv->tuner_config_ext), (void *) & (nim_cfg->tuner_config_ext), sizeof(struct QAM_TUNER_CONFIG_EXT));
    priv->qam_mode = nim_cfg->qam_mode;

    S3281_PRINTF("[TRACE:%d], Qam_mode:%x, %x\n", __LINE__, (unsigned int)priv->qam_mode, priv->tuner_config_ext.w_tuner_if_j83ac_type);

    priv->i2c_type = nim_cfg->tuner_config_ext.i2c_type_id;

    S3281_PRINTF("[%s] line=%d,tuner_id=0x%x,enter\n", __FUNCTION__, __LINE__,nim_cfg->tuner_id);

	p_io_func = tuner_setup(NIM_DVBC,nim_cfg->tuner_id);
	if(NULL != p_io_func)
	{
		priv->nim_tuner_init = (dvbc_tuner_init_callback*)p_io_func->pf_init;
		priv->nim_tuner_control = p_io_func->pf_control;
		priv->nim_tuner_status = p_io_func->pf_status;
		priv->nim_tuner_close = p_io_func->pf_close;
	}
	
}


/*****************************************************************************
* INT32 ali_m3281_nim_hw_initialize(struct nim_device *dev)
* Description: S3202 open
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/

static INT32 ali_m3281_nim_hw_initialize(struct nim_device *dev, struct ali_nim_m3200_cfg *nim_cfg)
{

    nim_s3281_set_config(dev,nim_cfg);

    nim_s3281_hw_init(dev);

    S3281_PRINTF("[%s] line=%d,end\n", __FUNCTION__, __LINE__);

    return SUCCESS;
}

static INT32 ali_m3281_tuner_adaption(struct nim_device *dev, struct ali_nim_m3200_cfg *nim_cfg)
{
	struct nim_s3281_private *priv = dev->priv;
	UINT8 data = 0;
	
	nim_s3281_set_config(dev,nim_cfg);

    if (SUCCESS == nim_i2c_read(priv->tuner_config_ext.i2c_type_id, 
		                        priv->tuner_config_ext.c_tuner_base_addr, 
		                        &data, 
		                        1))
    {	
		S3281_PRINTF("[%s] line=%d,adaption success!,i2c_type=%d,i2c_addr=0x%x\n", 
			      __FUNCTION__, __LINE__,
			      priv->tuner_config_ext.i2c_type_id,
			      priv->tuner_config_ext.c_tuner_base_addr);

		return SUCCESS;
    }

   return -1;
	
}


static RET_CODE ali_m3281_nim_release(struct inode *inode, struct file *file)
{
    UINT8 data = 0;
    INT32 ret = SUCCESS;

#ifdef MONITOR_NIM_STATUS
    struct nim_device *dev = file->private_data;
    struct nim_s3281_private *priv = dev->priv;
#endif
	
    S3281_PRINTF("[%s] line=%d,enter\n", __FUNCTION__, __LINE__);
    //soft reset.
    data = 0x80;
    nim_s3281_dvbc_write(NIM_S3202_CONTROL1, &data, 1);

#ifdef MONITOR_NIM_STATUS
    if (1 == nim_sw_test_tread_status)
    {
        nim_sw_test_tread_status = 2;
        while(0 != nim_sw_test_tread_status)
        {
            msleep(10);
        }
        destroy_workqueue(priv->workqueue);
    }
#endif
    S3281_PRINTF("[%s] line=%d,end\n", __FUNCTION__, __LINE__);
    return ret;
}



static int ali_m3281_nim_ioctl(struct file *file, unsigned int cmd, unsigned long parg)
{
    struct nim_device *dev = file->private_data;
    struct nim_s3281_private *priv = dev->priv;
    int ret = 0;

    switch(cmd)
    {
	case ALI_NIM_TUNER_SELT_ADAPTION_C:
    {
        struct ali_nim_m3200_cfg nim_param;

        if(copy_from_user(&nim_param, (struct ali_nim_m3200_cfg *)parg, sizeof(struct ali_nim_m3200_cfg))>0)
		{
			printk("%s error line%d\n", __FUNCTION__, __LINE__);
			// Invalid user space address
			return -EFAULT;
		}
        ret = ali_m3281_tuner_adaption(dev, &nim_param);

		break;
	}
    case ALI_NIM_HARDWARE_INIT_C:
    {
        struct ali_nim_m3200_cfg nim_param;

        memset((void*)&nim_param,0,sizeof(struct ali_nim_m3200_cfg));
        if(copy_from_user(&nim_param, (struct ali_nim_m3200_cfg *)parg, sizeof(struct ali_nim_m3200_cfg))>0)
		{
			printk("%s error line%d\n", __FUNCTION__, __LINE__);
			// Invalid user space address
			return -EFAULT;
		}
			
        ret = ali_m3281_nim_hw_initialize(dev, &nim_param);

        break;
    }
    case ALI_NIM_CHANNEL_CHANGE:
    {
        NIM_CHANNEL_CHANGE_T nim_param;

        S3281_PRINTF("[%s] line=%d,nim_param.fec=%d\n", __FUNCTION__, __LINE__, nim_param.fec);

		memset((void*)&nim_param,0,sizeof(NIM_CHANNEL_CHANGE_T));
		
        if(copy_from_user(&nim_param, (NIM_CHANNEL_CHANGE_T *)parg, sizeof(NIM_CHANNEL_CHANGE_T))>0)
		{
			printk("%s error line%d\n", __FUNCTION__, __LINE__);
			// Invalid user space address
			return -EFAULT;
		}

        if (0 == nim_param.fec)
            return nim_s3281_dvbc_channel_change(dev, &nim_param);
        else
            return nim_s3281_dvbc_channel_change(dev, &nim_param);
        break;
    }
    case ALI_NIM_GET_LOCK_STATUS:
    {
        UINT8 lock = 0;
        
        nim_s3281_dvbc_get_lock(dev, &lock);
        ret = lock;
        break;
    }
    case ALI_NIM_READ_QPSK_BER:
    {
        UINT32 ber = 0;
        
        nim_s3281_dvbc_get_ber(dev, &ber);
        ret = ber;
        break;
    }
    case ALI_NIM_READ_RSUB:
    {
        UINT32 per = 0;

        nim_s3281_dvbc_get_per(dev, &per);
        ret = per;
        break;
    }
    case ALI_NIM_DRIVER_READ_SUMPER:
    {
        UINT32 per_sum = 0;


        if(copy_from_user(&per_sum, (UINT32 *)parg, sizeof(UINT32))>0)
		{
			printk("%s error line%d\n", __FUNCTION__, __LINE__);
			// Invalid user space address
			return -EFAULT;
		}
			
        nim_s3281_dvbc_get_per(dev, &per_sum);
        ret = per_sum;
        break;
    }
    case ALI_NIM_GET_RF_LEVEL:
    {
        UINT16 rf_level = 0;

        nim_s3281_dvbc_get_rf_level(dev, &rf_level);
        ret = rf_level;
        break;
    }
    case ALI_NIM_GET_CN_VALUE:
    {
        UINT16 cn_value = 0;

        nim_s3281_dvbc_get_cn_value(dev, &cn_value);
        ret = cn_value;
        break;
    }
    case ALI_NIM_SET_PERF_LEVEL:
    {
        ret = nim_s3281_dvbc_set_perf_level(dev, parg);
        break;
    }
    case ALI_NIM_READ_AGC:
    {
        UINT8 agc = 0;

        nim_s3281_dvbc_get_agc(dev, &agc);

        ret = agc;
        break;
    }
    case ALI_NIM_READ_SNR:
    {
        UINT8 snr = 0;

        nim_s3281_dvbc_get_snr(dev, &snr);
        ret = snr;
        break;
    }
    case ALI_NIM_READ_SYMBOL_RATE:
    {
        UINT32 sym = 0;

        nim_s3281_dvbc_get_symbol_rate(dev, &sym);
        ret = sym;
        break;
    }
    case ALI_NIM_READ_FREQ:
    {
        UINT32 freq = 0;

        nim_s3281_dvbc_get_freq(dev, &freq);
        ret = freq;
        break;
    }
    case ALI_NIM_READ_CODE_RATE:
    {
        UINT8 fec = 0;

        nim_s3281_dvbc_get_qam_order(dev, &fec);
        ret = fec;
        break;
    }
    case ALI_NIM_REG_RW:
    {
        UINT8 reg_rw_cmd[16] ={0};

        if(copy_from_user(reg_rw_cmd, (UINT8 *)parg, 16)>0)
		{
			printk("%s error line%d\n", __FUNCTION__, __LINE__);
			// Invalid user space address
			return -EFAULT;
		}			
        if (1 == reg_rw_cmd[0]) // Register Read
        {
            ret = nim_s3281_dvbc_read(reg_rw_cmd[1], &(reg_rw_cmd[3]), reg_rw_cmd[2]);
            if(copy_to_user((UINT8 *)parg, reg_rw_cmd, 16)>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}
							
        }
        else if (2 == reg_rw_cmd[0]) // Register Write
        {
            ret = nim_s3281_dvbc_write(reg_rw_cmd[1], &(reg_rw_cmd[3]), reg_rw_cmd[2]);
        }
        break;
    }
    case ALI_NIM_REG_RW_EXT:
    {
        struct reg_rw_cmd_t
        {
            UINT32 offset_addr;
            UINT8 reg_rw_cmd[16];
        };

        struct reg_rw_cmd_t reg_param;

        if(copy_from_user(&reg_param, (struct reg_rw_cmd_t *)parg, sizeof(struct reg_rw_cmd_t))>0)
		{
			printk("%s error line%d\n", __FUNCTION__, __LINE__);
			// Invalid user space address
			return -EFAULT;
		}			
        if (1 == reg_param.reg_rw_cmd[0]) // Register Read
        {
            ret = nim_s3281_dvbc_read(reg_param.offset_addr, &(reg_param.reg_rw_cmd[2]), reg_param.reg_rw_cmd[1]);
            if(copy_to_user(((struct reg_rw_cmd_t *)parg)->reg_rw_cmd, (reg_param.reg_rw_cmd), 16)>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}
							
        }
        else if (2 == reg_param.reg_rw_cmd[0]) // Register Write
        {
            ret = nim_s3281_dvbc_write(reg_param.offset_addr, &(reg_param.reg_rw_cmd[2]), reg_param.reg_rw_cmd[1]);
        }
        break;
    }
    case ALI_SYS_REG_RW:
    {
        struct reg_rw_cmd_t
        {
            UINT32 offset_addr;
            UINT8 reg_rw_cmd[16];
        };

		struct reg_rw_cmd_t reg_param;
		if(copy_from_user(&reg_param, (struct reg_rw_cmd_t *)parg, sizeof(struct reg_rw_cmd_t))>0)
		{
			printk("%s error line%d\n", __FUNCTION__, __LINE__);
			// Invalid user space address
			return -EFAULT;		
		}
		if (1 == reg_param.reg_rw_cmd[0]) // Register Read
		{
			system_reg_read(reg_param.offset_addr, &(reg_param.reg_rw_cmd[2]), reg_param.reg_rw_cmd[1]);
			if(copy_to_user(((struct reg_rw_cmd_t *)parg)->reg_rw_cmd, (reg_param.reg_rw_cmd), 16)>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}
			
		}
		else if (2 == reg_param.reg_rw_cmd[0]) // Register Write
		{
			system_reg_write(reg_param.offset_addr, &(reg_param.reg_rw_cmd[2]), reg_param.reg_rw_cmd[1]);
		}

        break;
    }
    case ALI_NIM_DRIVER_SET_MODE:
    {

        UINT8 mode_cmd[32] ={0};

        S3281_PRINTF("[%s] line=%d,18\n", __FUNCTION__, __LINE__);

        if(copy_from_user(mode_cmd, (UINT8 *)parg, 8)>0)
		{
			printk("%s error line%d\n", __FUNCTION__, __LINE__);
			// Invalid user space address
			return -EFAULT;
		}			
        if (4 == mode_cmd[0])
        {
#ifndef MONITOR_NIM_STATUS
            S3281_PRINTF("[TRCAE] print mode invalid for not start NIM monitor thread\n");
#else
            DEBUG_SHOW_NIM_STATUS = ((0 == DEBUG_SHOW_NIM_STATUS) ? 1 : 0);
            printk("[TRCAE] print mode %s\n", (1 == DEBUG_SHOW_NIM_STATUS) ? "ON" : "OFF");
#endif
        }
        else
        {
            mode_cmd[1] = ((priv->qam_mode) & 0x01);
            if(copy_to_user((UINT8 *)parg, mode_cmd, 8)>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}
						
            if ((mode_cmd[0] < 2) && (mode_cmd[1] != mode_cmd[0]))
            {
                priv->qam_mode &= ~(1 << 0);
                priv->qam_mode |= mode_cmd[0];
                printk("[TRACE:%d]NIM work mode changed to %s\n", __LINE__, (1 == priv->qam_mode) ? "J83AC" : "J83B");
                //kent
                //nim_s3281_dvbc_set_mode(priv, struct DEMOD_CONFIG_ADVANCED qam_config)

                //  nim_s3202_mode_set4start(priv);
            }
            ret = ((priv->qam_mode) & 0x01);
        }

        break;
    }
#ifdef ADC2DRAM_KENABLE
    case ALI_NIM_ADC2MEM_START:
    {
        S3281_PRINTF("[%s] line=%d,19\n", __FUNCTION__, __LINE__);

        adc_dma_is_test_mode[0] = ((UINT8 *)parg)[0];    //0x100
        adc_dma_is_test_mode[1] = ((UINT8 *)parg)[1];    //0x101
        adc_dma_is_test_mode[2] = ((UINT8 *)parg)[2];    //0x102
        adc_dma_is_test_mode[3] = ((UINT8 *)parg)[10];   //test mode?
        user_start_adc_dma = 1;
        ret = SUCCESS;
        break;
    }
    case ALI_NIM_ADC2MEM_STOP:
    {
        S3281_PRINTF("[%s] line=%d,20\n", __FUNCTION__, __LINE__);

        user_force_stop_adc_dma = 1;
        ret = SUCCESS;
        break;
    }
    case ALI_NIM_ADC2MEM_SEEK_SET:
    {
        S3281_PRINTF("[%s] line=%d,21\n", __FUNCTION__, __LINE__);

        g_rx_buf_start_addr = (UINT8 *)(((UINT32)g_rx_dma_buffer) & 0xfffffffc);
        g_rx_buf_cur = (UINT32)parg;
        printk("adc data buf: %p, cur: 0x%08x\n", g_rx_buf_start_addr, g_rx_buf_cur);
        break;
    }
    case ALI_NIM_ADC2MEM_READ_8K:
    {
        S3281_PRINTF("[%s] line=%d,22\n", __FUNCTION__, __LINE__);

        //S3202_PRINTF("copy to user: 0x%08x, from: 0x%08x\n", parg,  g_rx_buf_start_addr+g_rx_buf_cur);
        if(copy_to_user((UINT8 *)parg, g_rx_buf_start_addr + g_rx_buf_cur, 0x2000)>0)
		{
			printk("%s error line%d\n", __FUNCTION__, __LINE__);
			// Invalid user space address
			return -EFAULT;
		}
						
        g_rx_buf_cur += 0x2000;
        if (g_rx_buf_cur >= g_rx_buffer_len/*(32*1024*1024)*/)
            g_rx_buf_cur = 0;
        break;
    }
#endif
    default:
    {
        S3281_PRINTF("[%s] line=%d,23\n", __FUNCTION__, __LINE__);
        ret = -ENOIOCTLCMD;
        break;
    }
    }

    return ret;
}

static RET_CODE ali_m3281_nim_open(struct inode *inode, struct file *file)
{
    struct nim_s3281_private *priv = ali_m3281_nim_priv; //priv->priv;

    S3281_PRINTF("[%s] line=%d,enter\n", __FUNCTION__, __LINE__);

    //PRINTK_INFO("malloc priv OK: %08x\n", priv);
    ali_m3281_nim_dev.priv = (void *)priv;
    file->private_data = (void *) & ali_m3281_nim_dev;
    return RET_SUCCESS;
}



static struct file_operations ali_m3281_nim_fops =
{
    .owner		= THIS_MODULE,
    .write		= NULL,
    .unlocked_ioctl	= ali_m3281_nim_ioctl,
    .open		= ali_m3281_nim_open,
    .release	= ali_m3281_nim_release,
};




static int __devinit ali_m3281_nim_init(void)
{
    INT32 ret = 0;
    dev_t devno;

    S3281_PRINTF("[%s] line=%d,enter\n", __FUNCTION__, __LINE__);
    ali_m3281_nim_priv = kmalloc(sizeof(struct nim_s3281_private), GFP_KERNEL);
    if (!ali_m3281_nim_priv)
    {
		return -ENOMEM;
    }	
    memset(ali_m3281_nim_priv, 0, sizeof(struct nim_s3281_private));
    mutex_init(&ali_m3281_nim_priv->i2c_mutex);
    ret = alloc_chrdev_region(&devno, 0, 1, ALI_NIM_DEVICE_NAME);
    if (ret < 0)
    {
        printk("[NIMTRACE] Alloc device region failed, err: %d.\n", (int)ret);
        return ret;
    }
#ifdef CONFIG_ARM
	i2c_gpio_attach(2);
#endif

    cdev_init(&ali_m3281_nim_dev.cdev, &ali_m3281_nim_fops);
    ali_m3281_nim_dev.cdev.owner = THIS_MODULE;
    ali_m3281_nim_dev.cdev.ops = &ali_m3281_nim_fops;
    ret = cdev_add(&ali_m3281_nim_dev.cdev, devno, 1);
    if (ret)
    {
        printk("[NIMTRACE] Alloc NIM device failed, err: %d.\n", (int)ret);
        goto error1;
    }

    S3281_PRINTF("register NIM device end.\n");

    ali_m3281_nim_class = class_create(THIS_MODULE, "ali_m3281_nim_class");

    if (IS_ERR(ali_m3281_nim_class))
    {
        ret = PTR_ERR(ali_m3281_nim_class);

        goto error2;
    }

    ali_m3281_nim_dev_node = device_create(ali_m3281_nim_class, NULL, devno, &ali_m3281_nim_dev,
                                           "ali_m3281_nim0");
    if (IS_ERR(ali_m3281_nim_dev_node))
    {
        printk(KERN_ERR "device_create() failed!\n");

        ret = PTR_ERR(ali_m3281_nim_dev_node);

        goto error3;
    }

    return ret;
#if 0
    //Modify PinMux
    *((volatile unsigned long *)0xb8000088) = *((volatile unsigned long *)0xb8000088) | 0x00000080;

    //Enable TSI
    *((volatile unsigned long *)0xb801a000) = 0x00038383;

#if((SYS_MAIN_BOARD==BOARD_DB_M3602_02V01)||(SYS_MAIN_BOARD==BOARD_DB_M3602_04V01))
    //set to SPI_1 for demoboard 02v01 and 04v01
    *((volatile unsigned long *)0xb801a008) = 0x02000181;
#endif
#endif
error3:
    class_destroy(ali_m3281_nim_class);
error2:
    cdev_del(&ali_m3281_nim_dev.cdev);
error1:
    mutex_destroy(&ali_m3281_nim_priv->i2c_mutex);
    kfree(ali_m3281_nim_priv);

    return ret;
}

static void __exit ali_m3281_nim_exit(void)
{
    S3281_PRINTF("[%s] line=%d,enter\n", __FUNCTION__, __LINE__);
    if (ali_m3281_nim_dev_node != NULL)
    {
		device_del(ali_m3281_nim_dev_node);
    }	

    if (ali_m3281_nim_class != NULL)
    {
		class_destroy(ali_m3281_nim_class);
    }	
    cdev_del(&ali_m3281_nim_dev.cdev);
    mutex_destroy(&ali_m3281_nim_priv->i2c_mutex);
    kfree(ali_m3281_nim_priv);
}



module_init(ali_m3281_nim_init);
module_exit(ali_m3281_nim_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kent");
MODULE_DESCRIPTION("Ali M3281 full NIM driver");





