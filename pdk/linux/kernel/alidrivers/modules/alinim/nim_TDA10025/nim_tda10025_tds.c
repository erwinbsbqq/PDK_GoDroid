#include "porting_tda10025_tds.h"



#if 0
#define TDA10025_PRINTF 	nim_print
#else
#define TDA10025_PRINTF(...)
#endif



static char nim_tda10025_name[HLD_MAX_NAME_SIZE] = "NIM_QAM_TDA10025";
struct nim_tda10025_private  *ali_tda10025_nim_priv=NULL;

static INT32 nim_tda10025_open(struct nim_device *dev);
static INT32 nim_tda10025_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param);
static INT32 nim_tda10025_ioctl_ext(struct nim_device *dev, INT32 cmd, void* param_list);
static INT32 nim_tda10025_close(struct nim_device *dev);



INT32 nim_tda10025_attach(struct QAM_TUNER_CONFIG_API * ptrQAM_Tuner)
{
	struct nim_device *dev = NULL;
	struct nim_tda10025_private *priv=NULL;

	
    TDA10025_PRINTF("[%s]line=%d,enter!\n",__FUNCTION__,__LINE__);

       /* Alloc structure space of tuner devive*/
	dev = (struct nim_device *)dev_alloc(nim_tda10025_name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
	if (dev == NULL)
	{
		TDA10025_PRINTF("Error: Alloc nim device error!\n");
		return ERR_NO_MEM;
	}

	/* Alloc structure space of private */
	priv = (struct nim_tda10025_private *)comm_malloc(sizeof(struct nim_tda10025_private));	
	if ((void*)priv == NULL)
	{
		dev_free(dev);
		TDA10025_PRINTF("Alloc nim device prive memory error!\n");
		return ERR_NO_MEM;
	}
	comm_memset((void*)priv, 0, sizeof(struct nim_tda10025_private));
	
	priv->i2c_mutex = OSAL_INVALID_ID;
	ali_tda10025_nim_priv=priv;
		
	/* tuner configuration function */
	comm_memcpy((void*)&(priv->tuner_config_data), (void*)&(ptrQAM_Tuner->tuner_config_data), 
	             sizeof(struct QAM_TUNER_CONFIG_DATA));
    comm_memcpy((void*)&(priv->tuner_config_ext), (void*)&(ptrQAM_Tuner->tuner_config_ext), 
                     sizeof(struct QAM_TUNER_CONFIG_EXT));
	comm_memcpy((void*)&(priv->ext_dem_config), (void*)&(ptrQAM_Tuner->ext_dem_config), 
                     sizeof(struct EXT_DM_CONFIG));
	
	priv->nim_tuner_init = ptrQAM_Tuner->nim_tuner_init;
	priv->nim_tuner_control = ptrQAM_Tuner->nim_tuner_control;
	priv->nim_tuner_status = ptrQAM_Tuner->nim_tuner_status;

	
	dev->priv = (void*)priv;
	/* Function point init */
	dev->init = nim_tda10025_attach;
	dev->open = nim_tda10025_open;
	dev->stop = nim_tda10025_close;
	dev->do_ioctl = nim_tda10025_ioctl;
	dev->do_ioctl_ext = nim_tda10025_ioctl_ext;
	dev->get_lock = nim_tda10025_get_lock;
	dev->get_freq = nim_tda10025_get_freq;
	dev->get_snr = nim_tda10025_get_snr; 
	dev->get_ber = nim_tda10025_get_ber;
	dev->get_agc = nim_tda10025_get_agc;
    dev->get_fec = nim_tda10025_get_qam;
	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS)
	{
		TDA10025_PRINTF("Error: Register nim device error!\n");
		comm_free(priv);
		dev_free(dev);
		return ERR_NO_DEV;
	}

	if (priv->nim_tuner_init != NULL)	
	{	
		if (priv->nim_tuner_init(&(priv->tuner_id), &(priv->tuner_config_ext)) != SUCCESS)
		{
			return ERR_NO_DEV;
		}	
	}

	TDA10025_PRINTF("[%s]line=%d,end!\n",__FUNCTION__,__LINE__);

	return nim_tda10025_dev_init(dev);
}


static INT32 nim_tda10025_open(struct nim_device *dev)
{
	struct nim_tda10025_private *priv = dev->priv;

    TDA10025_PRINTF("[%s]line=%d,enter!\n",__FUNCTION__,__LINE__);
	priv->i2c_mutex= os_create_mutex();
	if (priv->i2c_mutex == OSAL_INVALID_ID)
	{
		TDA10025_PRINTF("nim_tda10025_open: Create mutex failed!\n");
	}


	
	return RET_SUCCESS;
}


static INT32 nim_tda10025_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{
	INT32 rtn=SUCCESS;
	
	switch( cmd )
	{
	case NIM_DRIVER_READ_QPSK_BER:
	    rtn = nim_tda10025_get_ber(dev, (UINT32 *)param);
	    break;
	case NIM_DRIVER_READ_RSUB:
	  
	    break;
    case NIM_DRIVER_GET_AGC:
		rtn =nim_tda10025_get_agc(dev, (UINT8 *)param );
		break;
	default:
	    rtn = SUCCESS;
	    break;
	}
	return rtn;
}


static INT32 nim_tda10025_ioctl_ext(struct nim_device *dev, INT32 cmd, void* param_list)
{
	INT32 rtn = 0;

	switch( cmd )
	{
	case NIM_DRIVER_AUTO_SCAN:			/* Do AutoScan Procedure */
		//nim_s3202_AutoScan(dev, (struct NIM_Auto_Scan *) (param_list));
		rtn = SUCCESS;
		break;
	case NIM_DRIVER_CHANNEL_CHANGE:		/* Do Channel Change */
	case NIM_DRIVER_QUICK_CHANNEL_CHANGE:	
		rtn = nim_tda10025_channel_change(dev,(NIM_CHANNEL_CHANGE_T *) (param_list));

		break;
	case NIM_DRIVER_CHANNEL_SEARCH:	/* Do Channel Search */
		rtn= SUCCESS;
		break;
	case NIM_DRIVER_GET_RF_LEVEL:
		rtn= SUCCESS; 
		break;
	case NIM_DRIVER_GET_CN_VALUE:
		//rtn = nim_s3202_get_CN_value(dev, (UINT16 *)param_list);
		break;
	case NIM_DRIVER_GET_BER_VALUE:
		//rtn = nim_s3202_get_BER(dev, (UINT32 *)param_list);
		break;
	case NIM_DRIVER_SET_PERF_LEVEL:
		//rtn = nim_s3202_set_perf_level(dev, (UINT32)param_list);
		break;
	case NIM_TURNER_SET_STANDBY:
        {
          //   PRINTK_INFO("[%s] line=%d, 3!\n", __FUNCTION__,__LINE__);
             rtn = nim_tda10025_enter_standby(dev);
              break;
         }	
	case NIM_DRIVER_GET_I2C_INFO:
		{			

			break;
 		}
	default:
		rtn = SUCCESS;
	       break;
	}

	return rtn;
}


static INT32 nim_tda10025_close(struct nim_device *dev)
{
	struct nim_tda10025_private *priv = dev->priv;
	INT32 	ret=SUCCESS;

	TDA10025_PRINTF("[%s]line=%d,enter!\n",__FUNCTION__,__LINE__);
	
	if (RET_SUCCESS != os_delete_mutex(priv->i2c_mutex))
	{
		TDA10025_PRINTF("nim_tda10025_close: Delete mutex failed!\n");
		ret = ERR_FAILUE;
	}
    if(NULL != priv->nim_tuner_close)
    {
		priv->nim_tuner_close(priv->tuner_id);
    }
	
	TDA10025_PRINTF("[%s]line=%d,end!\n",__FUNCTION__,__LINE__);
	return ret;
}



