/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: nim_cxd2837_linux.c
*
*    Description: cxd2837 nim driver for linux api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "porting_cxd2837_linux.h"
#include "nim_cxd2837.h"
#include "sony_demod.h"
#include "sony_demod_integ.h"



#define SONY_NIM_DEVICE_NAME         "ali_nim_cxd2837"

#define MAX_TUNER_SUPPORT_NUM        4
#define NIM_TUNER_SET_STANDBY_CMD    0xffffffff
#define sony_demod_Create            cxd2837_demod_Create


static struct nim_device             ali_cxd2837_nim_dev;
struct sony_demod_t                  *ali_cxd2837_nim_priv = NULL;
static struct class                  *ali_cxd2837_nim_class;
static struct device                 *ali_cxd2837_nim_dev_node;
static sony_i2c_t                    demodI2c;
static INT32                         g_is_rest =0;

static void nim_cxd2837_set_config(struct nim_device *dev, struct ali_nim_mn88436_cfg *nim_cfg)
{
    struct sony_demod_t 	    *priv = dev->priv;
    TUNER_IO_FUNC               *p_io_func=NULL;

	
    memcpy(&priv->tuner_control.config_data, &nim_cfg->cofdm_data, sizeof(struct COFDM_TUNER_CONFIG_DATA));
    memcpy(&priv->tuner_control.tuner_config, &nim_cfg->tuner_config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
    memcpy(&priv->tuner_control.ext_dm_config, &nim_cfg->ext_dm_config, sizeof(struct EXT_DM_CONFIG));
    priv->tuner_control.tuner_id = nim_cfg->tuner_id;

    PRINTK_INFO("[%s] line=%d,i2c_type=%d,i2c_addr=0x%x\n", __FUNCTION__, __LINE__,
		                             // priv->tuner_control.config_data.flag,
		                              priv->tuner_control.tuner_config.i2c_type_id,
		                              priv->tuner_control.tuner_config.c_tuner_base_addr);

	p_io_func = tuner_setup(NIM_DVBT,nim_cfg->tuner_id);
	if(NULL != p_io_func)
	{
		priv->tuner_control.nim_tuner_init    = (tuner_init_callback )(p_io_func->pf_init);
		priv->tuner_control.nim_tuner_control = (tuner_control_callback )(p_io_func->pf_control);
		priv->tuner_control.nim_tuner_status  = (tuner_status_callback )(p_io_func->pf_status);
		priv->tuner_control.nim_tuner_close   = (tuner_close_callback )(p_io_func->pf_close);
	}
	else
	{
		PRINTK_INFO("[%s]line=%d,Tuner API is NULL!\n",__FUNCTION__,__LINE__);
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

static INT32 ali_cxd2837_nim_hw_initialize(struct nim_device *dev, struct ali_nim_mn88436_cfg *nim_cfg)
{

    sony_demod_t *priv = dev->priv;
    DEM_WRITE_READ_TUNER ThroughMode;

	if((NULL == nim_cfg) || (NULL == dev))
	{
		PRINTK_INFO("Tuner Configuration API structure is NULL!/n");
		return ERR_NO_DEV;
	}
	PRINTK_INFO("[%s] line=%d,start:\n", __FUNCTION__, __LINE__);

  if(priv->m_pfn_reset_cxd2837)
		priv->m_pfn_reset_cxd2837(priv->tuner_id);
	//Setup demod I2C interfaces.
	demodI2c.i2c_type_id      = nim_cfg->ext_dm_config.i2c_type_id;
	demodI2c.ReadRegister     = cxd2837_i2c_CommonReadRegister;
	demodI2c.WriteRegister    = cxd2837_i2c_CommonWriteRegister;
	demodI2c.WriteOneRegister = cxd2837_i2c_CommonWriteOneRegister;

    PRINTK_INFO("ext_dm_config.i2c_base_addr = 0x%x\n",nim_cfg->ext_dm_config.i2c_base_addr);
    /* Create demodulator instance */
	if (sony_demod_Create (priv, SONY_DEMOD_XTAL_20500KHz, nim_cfg->ext_dm_config.i2c_base_addr, &demodI2c) != SONY_RESULT_OK)
	{
	    PRINTK_INFO("sony_demod_Create error!\n");
		return ERR_NO_DEV;
	}
	nim_cxd2837_set_config(dev,nim_cfg);
	PRINTK_INFO("i2cAddressSLVX = 0x%x\n",priv->i2cAddressSLVX);
	/* ---------------------------------------------------------------------------------
	* Configure the Demodulator
	* ------------------------------------------------------------------------------ */
	/* DVB-T demodulator IF configuration for terrestrial / cable tuner */
	priv->iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT_5MHz_IF);
	priv->iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT_6MHz_IF);
	priv->iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT_7MHz_IF);
	priv->iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT_8MHz_IF);
	
	/* DVB-T2 demodulator IF configuration for terrestrial / cable tuner */
	priv->iffreqConfig.configDVBT2_1_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT2_1_7MHz_IF);
	priv->iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT2_5MHz_IF);
	priv->iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT2_6MHz_IF);
	priv->iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT2_7MHz_IF);
	priv->iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT2_8MHz_IF);

	/* DVB-C demodulator IF configuration for terrestrial / cable tuner */
	priv->iffreqConfig.configDVBC_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBC_6MHz_IF);
	priv->iffreqConfig.configDVBC_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBC_7MHz_IF);
	priv->iffreqConfig.configDVBC_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBC_8MHz_IF);

	priv->tunerOptimize = SONY_DEMOD_TUNER_OPTIMIZE_ASCOT3; 
	
    PRINTK_INFO("[%s]:line=%d,system:%d\r\n",__FUNCTION__,__LINE__,priv->system);
	
    if(priv->tuner_control.nim_tuner_init != NULL)
    {
    	if(priv->tuner_control.nim_tuner_init(&(priv->tuner_id), &(priv->tuner_control.tuner_config)) != SUCCESS)
        {
            PRINTK_INFO("Error: Init Tuner Failure!\r\n");
            return ERR_NO_DEV;
        }
    	ThroughMode.nim_dev_priv = dev->priv;
    	ThroughMode.Dem_Write_Read_Tuner = (INTERFACE_DEM_WRITE_READ_TUNER)cxd2837_i2c_TunerGateway;
    	tun_cxd_ascot3_command(priv->tuner_id, NIM_TUNER_SET_THROUGH_MODE, (UINT32)&ThroughMode);
    }
	else
	{
		PRINTK_INFO("Error: Init Tuner is NULL!\r\n");
	}
    if (SONY_RESULT_OK != nim_cxd2837_open(&ali_cxd2837_nim_dev))
    {
        PRINTK_INFO("[%s] line=%d,nim_cxd2837_open failed!\n", __FUNCTION__, __LINE__);
		return ERR_FAILURE;
	}
    PRINTK_INFO("[%s] line=%d,end!\n", __FUNCTION__, __LINE__);
    return SUCCESS;
}

#if 0
static INT32 ali_cxd2837_tuner_adaption(struct nim_device *dev, struct ali_nim_mn88436_cfg *nim_cfg)
{
	struct nim_cxd2837_private *priv = dev->priv;
	UINT8 data = 0;
	
	nim_cxd2837_set_config(dev,nim_cfg);

    if (SUCCESS == nim_cxd2837_read(dev, 
		                        priv->tuner_control.tuner_config.c_tuner_base_addr, 
		                        &data, 
		                        1))
    {	
		PRINTK_INFO("[%s] line=%d,adaption success!,i2c_type=%d,i2c_addr=0x%x\n", 
			      __FUNCTION__, __LINE__,
			      priv->tuner_control.tuner_config.i2c_type_id,
			      priv->tuner_control.tuner_config.c_tuner_base_addr);

		return SUCCESS;
    }

   return -1;	
}
#endif
static int ali_cxd2837_nim_ioctl(struct file *file, unsigned int cmd, unsigned long parg)
{
    struct nim_device *dev = file->private_data;
	sony_demod_t *priv = dev->priv;
    int ret = 0;
	
    switch(cmd)
	{
	
		case ALI_NIM_TUNER_SELT_ADAPTION_C:
	    {
	        struct ali_nim_mn88436_cfg nim_param;

	        if(copy_from_user(&nim_param, (struct ali_nim_mn88436_cfg *)parg, sizeof(struct ali_nim_mn88436_cfg))>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}
	        //ret = ali_cxd2837_tuner_adaption(dev, &nim_param);

			break;
		}
	    case ALI_NIM_HARDWARE_INIT_T:
	    {
	        struct ali_nim_mn88436_cfg nim_param;

	        memset((void*)&nim_param,0,sizeof(struct ali_nim_mn88436_cfg));
	        if(copy_from_user(&nim_param, (struct ali_nim_mn88436_cfg *)parg, sizeof(struct ali_nim_mn88436_cfg))>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}

	        ret = ali_cxd2837_nim_hw_initialize(dev, &nim_param);

	        break;
	    }
		case ALI_NIM_STOP_AUTOSCAN:

		    priv->autoscan_stop_flag = parg;

			break;	
	    case ALI_NIM_CHANNEL_CHANGE:
	    {
	        NIM_CHANNEL_CHANGE_T nim_param;

	        PRINTK_INFO("[%s] line=%d,nim_param.fec=%d\n", __FUNCTION__, __LINE__, nim_param.fec);

			memset((void*)&nim_param,0,sizeof(NIM_CHANNEL_CHANGE_T));
			
	        if(copy_from_user(&nim_param, (NIM_CHANNEL_CHANGE_T *)parg, sizeof(NIM_CHANNEL_CHANGE_T))>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}
			if(SUCCESS != nim_cxd2837_channel_change_smart(dev, &nim_param))
			{
				PRINTK_INFO("[%s] line=%d,nim_cxd2837_channel_change_smart failed!\n", __FUNCTION__, __LINE__);
			}

	        if(copy_to_user((NIM_CHANNEL_CHANGE_T *)parg, &nim_param, sizeof(NIM_CHANNEL_CHANGE_T))>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}
			
	        break;
	    }
		case ALI_NIM_T2_SIGNAL_ONLY:
	    {

			priv->search_t2_only = parg;	
			
			break;
		}

	    case ALI_NIM_GET_LOCK_STATUS:
	    {
	        UINT8 lock = 0;
			nim_cxd2837_get_lock(dev, &lock);
	        ret = lock;
	        break;
	    }

	    case ALI_NIM_READ_AGC:
	    {
	        UINT8 agc = 0;

	        nim_cxd2837_get_SSI(dev, &agc);
	        ret = agc;
	        break;
	    }
		#if 0
		case ALI_NIM_LOG_LEVEL:
		{
	        int log_level= 0;

			if(copy_from_user(&log_level, (int *)parg, sizeof(int))>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}
	        set_log_level(log_level);

			ret = SUCCESS;
			break;
		}
		#endif
		case ALI_NIM_READ_SNR:
		{
			UINT8 snr = 0;
			nim_cxd2837_get_SQI(dev, &snr);
			ret = snr;
			
			break;
		}
		case ALI_NIM_READ_CODE_RATE:
		{
	        UINT8 code_rate = 0;
			
			nim_cxd2837_get_FEC(dev, &code_rate);

			ret=code_rate;
			break;
			
		}
		case ALI_NIM_READ_QPSK_BER:
		{
	        UINT32 ber = 0;
	        
	        nim_cxd2837_get_BER(dev, &ber);
			
	        ret = ber;

			break;
		}
		case ALI_NIM_READ_FREQ:
	    {
	        UINT32 freq = 0;
	     
	        nim_cxd2837_get_freq(dev, &freq);
	        ret = freq;
	        break;
	    }	
	    case ALI_NIM_GET_FFT_MODE:
        {
			UINT8 fftmode = 0;
			 
			nim_cxd2837_get_fftmode(dev,&fftmode);
			ret = fftmode;
			 
	        break;  
	    }
	    case ALI_NIM_GET_MODULATION:
		{
			UINT8 modulation = 0;
			 
	    	nim_cxd2837_get_modulation(dev,&modulation);
			ret = modulation;
			break;
	    }
		case ALI_NIM_GET_GUARD_INTERVAL:
		{
			UINT8 guard_interval = 0;	
			 
			nim_cxd2837_get_GI(dev,&guard_interval);
			ret = guard_interval;
			break;
		}
		case ALI_NIM_GET_SPECTRUM_INV:

			 break;
		case ALI_NIM_DRIVER_SET_RESET_CALLBACK:
		{
			sony_demod_t * pDemod = (sony_demod_t *)dev->priv;
			pDemod->m_pfn_reset_cxd2837 = (pfn_nim_reset_callback ) parg;
		}
		break;
	    default:
	    {
	        PRINTK_INFO("ERROR-->[%s]line=%d,cmd=%d error!\n", __FUNCTION__, __LINE__,cmd);
	        ret = -ENOIOCTLCMD;
	        break;
	    }
	 }
    return ret;
}

static RET_CODE ali_cxd2837_nim_release(struct inode *inode, struct file *file)
{
	INT32 result = SUCCESS;
	struct nim_device *dev = file->private_data;

    //close demod
	result = nim_cxd2837_close(dev);
	if(SUCCESS != result)
	   return result;
	
	//close tuner
	result = tun_cxd_ascot3_release();
	return result;
 
}

static int match(struct gpio_chip *chip,void *data)
{
    if (0 == strcmp(chip->label, data))
    {
        return 1;
    }
	return 0;
}
/*****************************************************************************
* static void nim_cxd2838_hwreset(void)
* Description: cxd2838 device reset
*
* Arguments:
*  
*
* Return Value: success
*****************************************************************************/

static void nim_cxd2837_hwreset(void)
{

	 struct gpio_chip *gpio_chip;
	 int gpio_index  = 84;	//gpio[84]
 
	 gpio_chip = gpiochip_find("m36", match);

	 //set gpio to high
     gpio_chip->direction_output(gpio_chip,gpio_index,1);
	 msleep(10);
     //set gpio to low
	 gpio_chip->direction_output(gpio_chip,gpio_index,0);
	 msleep(10);	 
	 //set gpio to high
	 gpio_chip->direction_output(gpio_chip,gpio_index,1);
}


static RET_CODE ali_cxd2837_nim_open(struct inode *inode, struct file *file)
{
    struct sony_demod_t *priv = ali_cxd2837_nim_priv;

    PRINTK_INFO("[%s] line=%d,enter\n", __FUNCTION__, __LINE__);
	
    ali_cxd2837_nim_dev.priv = (void *)priv;
    file->private_data = (void *) &ali_cxd2837_nim_dev;
	if(!g_is_rest)
    {
		nim_cxd2837_hwreset();
		g_is_rest = 1;
    }
	return SUCCESS;

}

static const struct file_operations ali_cxd2837_nim_fops =
{
    .owner		    = THIS_MODULE,
    .unlocked_ioctl	= ali_cxd2837_nim_ioctl,
    .open		    = ali_cxd2837_nim_open,
    .release	    = ali_cxd2837_nim_release,
};

static int __devinit ali_cxd2837_nim_init(void)
{
    INT32 ret = 0;
    dev_t devno;

    PRINTK_INFO("[%s] line=%d,enter\n", __FUNCTION__, __LINE__);
    ali_cxd2837_nim_priv = kmalloc(sizeof(struct sony_demod_t), GFP_KERNEL);
    if (!ali_cxd2837_nim_priv)
    {
		return -ENOMEM;
    }	
	
    memset(ali_cxd2837_nim_priv, 0, sizeof(struct sony_demod_t));

	 //mutex_init(&ali_cxd2837_nim_priv->i2c_mutex_id);
	
	 mutex_init(&ali_cxd2837_nim_priv->demodMode_mutex_id);
   	
	 //mutex_init(&ali_cxd2837_nim_priv->flag_id);
	 //ali_cxd2837_nim_priv->flag_lock.flagid_rwlk = __RW_LOCK_UNLOCKED(ali_cxd2837_nim_priv->flagid_rwlk);
   //mutex_init(&ali_cxd2837_nim_priv->i2c_mutex_id);
	
    ret = alloc_chrdev_region(&devno, 0, 1, SONY_NIM_DEVICE_NAME);
    if (ret < 0)
    {
        printk("[NIMTRACE] Alloc device region failed, err: %d.\n", (int)ret);
        return ret;
    }


    cdev_init(&ali_cxd2837_nim_dev.cdev, &ali_cxd2837_nim_fops);
    ali_cxd2837_nim_dev.cdev.owner = THIS_MODULE;
    ali_cxd2837_nim_dev.cdev.ops = &ali_cxd2837_nim_fops;
    ret = cdev_add(&ali_cxd2837_nim_dev.cdev, devno, 1);
    if (ret)
    {
        printk("[NIMTRACE] Alloc NIM device failed, err: %d.\n", (int)ret);
        goto error1;
    }

    PRINTK_INFO("register NIM device end.\n");

    ali_cxd2837_nim_class = class_create(THIS_MODULE, "ali_cxd2837_nim_class");

    if (IS_ERR(ali_cxd2837_nim_class))
    {
        ret = PTR_ERR(ali_cxd2837_nim_class);
        goto error2;
    }

    ali_cxd2837_nim_dev_node = device_create(ali_cxd2837_nim_class, NULL, devno, &ali_cxd2837_nim_dev,
                                           "ali_cxd2837_nim0");
    if (IS_ERR(ali_cxd2837_nim_dev_node))
    {
        printk(KERN_ERR "device_create() failed!\n");

        ret = PTR_ERR(ali_cxd2837_nim_dev_node);

        goto error3;
    }

    return ret;

error3:
    class_destroy(ali_cxd2837_nim_class);
error2:
    cdev_del(&ali_cxd2837_nim_dev.cdev);
error1:
    
    //mutex_destroy(&ali_cxd2837_nim_priv->i2c_mutex_id);
	mutex_destroy(&ali_cxd2837_nim_priv->demodMode_mutex_id);
	//mutex_destroy(&ali_cxd2837_nim_priv->flag_id);
    kfree(ali_cxd2837_nim_priv);

    return ret;
}

static void __exit ali_cxd2837_nim_exit(void)
{
    PRINTK_INFO("[%s] line=%d,enter\n", __FUNCTION__, __LINE__);
    if (ali_cxd2837_nim_dev_node != NULL)
    {
		device_del(ali_cxd2837_nim_dev_node);
    }	

    if (ali_cxd2837_nim_class != NULL)
    {
		class_destroy(ali_cxd2837_nim_class);
    }	
    cdev_del(&ali_cxd2837_nim_dev.cdev);
	//mutex_destroy(&ali_cxd2837_nim_priv->i2c_mutex_id);
	mutex_destroy(&ali_cxd2837_nim_priv->demodMode_mutex_id);
	//mutex_destroy(&ali_cxd2837_nim_priv->flag_id);
    kfree(ali_cxd2837_nim_priv);
}




module_init(ali_cxd2837_nim_init);
module_exit(ali_cxd2837_nim_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dennis");
MODULE_DESCRIPTION("SONY CXD2837 NIM driver");





