#include "porting_tda10025_linux.h"

#define ALI_NIM_DEVICE_NAME 		"ali_nim_tda10025"





struct nim_device 					ali_tda10025_nim_dev[MAX_TUNER_SUPPORT_NUM];
struct nim_tda10025_private 		*ali_tda10025_nim_priv[MAX_TUNER_SUPPORT_NUM]={NULL,NULL,NULL} ;
struct class 						*ali_tda10025_nim_class =NULL;
struct device 						*ali_tda10025_nim_dev_node[MAX_TUNER_SUPPORT_NUM] = {NULL};



void nim_tda10025_hwreset(void)
{
    int gpio_index=1;  //10

    __REG32ALI(0x18000430) &= 0 << gpio_index;
    __REG32ALI(0x18000058) |= 1 << gpio_index;  //set output
    __REG32ALI(0x18000054) &= 0 << gpio_index;
    msleep(20);
    __REG32ALI(0x18000054) |= 1 << gpio_index;
	__REG32ALI(0x18000058) |= 1 << gpio_index;
	__REG32ALI(0x18000430) |= 1 << gpio_index;
   
}


static INT32 nim_tda10025_hw_initialize(struct nim_device *dev, struct ali_nim_m3200_cfg *nim_cfg)
{
	TUNER_IO_FUNC                       *p_io_func=NULL;
	struct nim_tda10025_private *priv = (struct nim_tda10025_private *)(dev->priv);
	
	TDA10025_PRINTF("[%s] line=%d,enter!\n", __FUNCTION__,__LINE__);
	/* tuner configuration function */
	comm_memcpy((void*)&(priv->tuner_config_data), (void*)&(nim_cfg->tuner_config_data), sizeof(struct QAM_TUNER_CONFIG_DATA));
	comm_memcpy((void*)&(priv->tuner_config_ext), (void*)&(nim_cfg->tuner_config_ext), sizeof(struct QAM_TUNER_CONFIG_EXT));
	comm_memcpy((void*)&(priv->ext_dem_config), (void*)&(nim_cfg->ext_dem_config), sizeof(struct EXT_DM_CONFIG));

	priv->qam_mode = nim_cfg->qam_mode;
	p_io_func = tuner_setup(NIM_DVBC,nim_cfg->tuner_id);
	if(NULL != p_io_func)
	{
		priv->nim_tuner_init = (dvbc_tuner_init_callback*)p_io_func->pf_init;
		priv->nim_tuner_control = p_io_func->pf_control;
		priv->nim_tuner_status = p_io_func->pf_status;
		priv->nim_tuner_close = p_io_func->pf_close;
	}

	
	 

	/* Tuner Initial */
	if (priv->nim_tuner_init != NULL)	
	{	
		if (priv->nim_tuner_init(&(priv->tuner_id), &(priv->tuner_config_ext)) != SUCCESS)
		{
			return ERR_NO_DEV;
		}	
	}
    
	return nim_tda10025_dev_init(dev);

}



RET_CODE nim_tda10025_open(struct inode *inode, struct file *file)
{
	UINT8 dev_idx = 0;

    dev_idx = MINOR(inode->i_rdev);

    TDA10025_PRINTF("[%s] line=%d,dev_idx=%d,enter!\n", __FUNCTION__,__LINE__,dev_idx);
	
	file->private_data=(void*)&ali_tda10025_nim_dev[dev_idx];
	
	//ali_tda10025_nim_dev.priv = (void *)ali_tda10025_nim_priv;
	
    
	 nim_tda10025_hwreset(); 
	return RET_SUCCESS;
}




int nim_tda10025_ioctl(struct file *file, unsigned int cmd, unsigned long parg)
{
	struct nim_device *dev =(struct nim_device *) file->private_data;
	struct nim_tda10025_private *priv = (struct nim_tda10025_private *)(dev->priv);
	int ret = 0;


	switch(cmd)
	{
		case ALI_NIM_HARDWARE_INIT_C:
		{
		    struct ali_nim_m3200_cfg nim_param;
			
			if(copy_from_user(&nim_param, (struct ali_nim_m3200_cfg *)parg, sizeof(struct ali_nim_m3200_cfg))>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}				
           
			ret = nim_tda10025_hw_initialize(dev, &nim_param);
            
			break;
		}
        
		case ALI_NIM_READ_QPSK_BER:
		{
			UINT32 ber = 0;

			nim_tda10025_get_ber(dev, &ber);
			ret = ber;
			break;
		}

        case ALI_NIM_TURNER_SET_STANDBY:
        {

             ret = nim_tda10025_enter_standby(dev);
              break;
         }

		case ALI_NIM_CHANNEL_CHANGE:
		{
            NIM_CHANNEL_CHANGE_T nim_param;

			if(copy_from_user(&nim_param, (NIM_CHANNEL_CHANGE_T *)parg, sizeof(NIM_CHANNEL_CHANGE_T))>0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}				

			TDA10025_PRINTF("[NOTE] NIM change:\n");
			TDA10025_PRINTF("\t.freq = %d\n", nim_param.freq);
			TDA10025_PRINTF("\t.sym = %d\n", nim_param.sym);
			TDA10025_PRINTF("\t.modulation = %d\n", nim_param.modulation);
			

			if (0 == nim_param.fec)
 			{
				ret = nim_tda10025_channel_change(dev, &nim_param);
			}	
			else
			{
				ret = nim_tda10025_channel_change(dev, &nim_param);
			}	
			break;
		}

        case ALI_NIM_GET_LOCK_STATUS:
		{

			UINT8 lock = 0;
			
			nim_tda10025_get_lock(dev, &lock);
			ret=lock;
		
			break;
		}
                
		case ALI_NIM_READ_SNR:
		{

			UINT8 snr = 0;
			nim_tda10025_get_snr(dev, &snr);
			ret=snr;
		
			break;
		}

		case ALI_NIM_READ_FREQ:
		{
			UINT32 freq;
			
			nim_tda10025_get_freq(dev, &freq);
			ret=freq;
		
			break;
		}

		case ALI_NIM_READ_AGC:
		{

			UINT8 agc = 0;
           
			nim_tda10025_get_agc(dev, &agc);
			ret=agc;
		
			break;
		}

		case ALI_NIM_READ_SYMBOL_RATE:
		{

			UINT32 sym = 0;
			
			nim_tda10025_get_symbolrate(dev, &sym);
			ret=sym;
		
			break;
		}

	       case ALI_NIM_READ_CODE_RATE:
	       {    
		     UINT8 fec = 0;
			 
		     nim_tda10025_get_qam(dev, &fec);
		     ret=fec;
		
		     break;
	       }
        
             case ALI_NIM_SET_POLAR:
             case ALI_NIM_AUTO_SCAN :
            // case ALI_NIM_DISEQC_OPERATE:
             case ALI_NIM_GET_RF_LEVEL:
             case  ALI_NIM_GET_CN_VALUE:
             case ALI_NIM_SET_PERF_LEVEL:
             case ALI_NIM_REG_RW :
             case ALI_NIM_SET_NETLINKE_ID :
             case ALI_NIM_AS_SYNC_WITH_LIB :
             case ALI_NIM_STOP_AUTOSCAN:
             case ALI_NIM_DRIVER_GET_CUR_FREQ:
             case ALI_NIM_DRIVER_READ_SUMPER:
             case ALI_NIM_DRIVER_SET_MODE:
             case ALI_NIM_REG_RW_EXT:
             case ALI_SYS_REG_RW:

             case ALI_NIM_DRIVER_SET_RESET_CALLBACK:
             case ALI_NIM_DRIVER_GET_ID:
	      default:
			break;
	}

	return ret;
}

RET_CODE nim_tda10025_release(struct inode *inode, struct file *file)
{
	UINT8  data = 0;
	struct nim_device *dev = file->private_data;
	struct nim_tda10025_private *priv = dev->priv;

    TDA10025_PRINTF("[%s] line=%d enter!\n", __FUNCTION__,__LINE__);
    if(NULL != priv->nim_tuner_close)
    {
		priv->nim_tuner_close(priv->tuner_id);
		priv->nim_tuner_close = NULL;
    }
	nim_tda10025_dev_close(dev);
	
    return RET_SUCCESS;
}

static struct file_operations ali_tda10025_nim_fops = {
	.owner						= THIS_MODULE,
	.write						= NULL, 
	.unlocked_ioctl				= nim_tda10025_ioctl,
	.open						= nim_tda10025_open,
	.release					=  nim_tda10025_release,
};


INT32 nim_tda10025_init(void)
{
	INT32 i = 0;
	INT32 ret = SUCCESS;
	dev_t devno;
	
	TDA10025_PRINTF("[%s] line=%d enter,max_tuner=%d!\n", __FUNCTION__,__LINE__,MAX_TUNER_SUPPORT_NUM);
	ret=alloc_chrdev_region(&devno, 0, MAX_TUNER_SUPPORT_NUM, ALI_NIM_DEVICE_NAME);
	if(ret<0)
	{
		TDA10025_PRINTF("Alloc device region failed, err: %d.\n",ret);
		return ret;
	}

	ali_tda10025_nim_class = class_create(THIS_MODULE, "ali_tda10025_nim_class");
	if (IS_ERR(ali_tda10025_nim_class))
	{
		TDA10025_PRINTF("[%s]line=%d,class_create error,back!\n", __FUNCTION__,__LINE__);
		ret = PTR_ERR(ali_tda10025_nim_class);
		return ret;
	}


    for(i = 0; i < MAX_TUNER_SUPPORT_NUM; i++)
    {
        ali_tda10025_nim_priv[i] = kmalloc(sizeof(struct nim_tda10025_private), GFP_KERNEL);
        if (!ali_tda10025_nim_priv[i])
        {
            TDA10025_PRINTF("kmalloc failed!\n");
            ret = -ENOMEM;
            break;
        }
        comm_memset(ali_tda10025_nim_priv[i], 0, sizeof(struct nim_tda10025_private));
        mutex_init(&ali_tda10025_nim_priv[i]->i2c_mutex);
       
        cdev_init(&ali_tda10025_nim_dev[i].cdev, &ali_tda10025_nim_fops);
        ali_tda10025_nim_dev[i].cdev.owner = THIS_MODULE;
        ali_tda10025_nim_dev[i].cdev.ops = &ali_tda10025_nim_fops;

        ali_tda10025_nim_dev[i].priv = (void *)ali_tda10025_nim_priv[i];
        ali_tda10025_nim_priv[i]->dev_idx=i;
		ali_tda10025_nim_priv[i]->tuner_id = i;
		
        ret = cdev_add(&ali_tda10025_nim_dev[i].cdev, devno + i, 1);
        if(ret)
        {
            TDA10025_PRINTF("Alloc NIM device failed, err: %d.\n", (int)ret);
            mutex_destroy(&ali_tda10025_nim_priv[i]->i2c_mutex);
            kfree(ali_tda10025_nim_priv[i]);
        }

        ali_tda10025_nim_dev_node[i] = device_create(ali_tda10025_nim_class, NULL, MKDEV(MAJOR(devno), i),
                                    &ali_tda10025_nim_dev[i], "ali_tda10025_nim%d", i);
        if(IS_ERR(ali_tda10025_nim_dev_node[i]))
        {
            TDA10025_PRINTF("device_create() failed!\n");
            ret = PTR_ERR(ali_tda10025_nim_dev_node[i]);
            cdev_del(&ali_tda10025_nim_dev[i].cdev);
            mutex_destroy(&ali_tda10025_nim_priv[i]->i2c_mutex);
            kfree(ali_tda10025_nim_priv[i]);
        }
    }


	nim_tda10025_hwreset();

    return ret;

}


static void __exit nim_tda10025_exit(void)
{

    UINT8 i = 0;

    TDA10025_PRINTF("[%s]line=%d,enter!\n", __FUNCTION__, __LINE__);

    if(ali_tda10025_nim_class != NULL)
    {
		class_destroy(ali_tda10025_nim_class);
    }	

    for (i = 0; i < MAX_TUNER_SUPPORT_NUM; i++)
    {
        if(ali_tda10025_nim_dev_node[i] != NULL)
        {
			device_del(ali_tda10025_nim_dev_node[i]);
        }	


        cdev_del(&ali_tda10025_nim_dev[i].cdev);
        mutex_destroy(&ali_tda10025_nim_priv[i]->i2c_mutex);


        kfree(ali_tda10025_nim_priv[i]);
    }
    TDA10025_PRINTF("[%s]line=%d,end!\n", __FUNCTION__, __LINE__);
	
	
}



module_init(nim_tda10025_init);
module_exit(nim_tda10025_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael Chen");
MODULE_DESCRIPTION("Ali tda10025 full NIM driver");


