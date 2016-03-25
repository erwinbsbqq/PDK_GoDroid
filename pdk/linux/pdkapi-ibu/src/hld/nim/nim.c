/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    nim.c
*
*    Description:    This file contains all functions definition
*		             of TDS NIM driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Feb.16.2003      Justin Wu       Ver 0.1    Create file.
*	2.	Aug.21.2003      Justin Wu       Ver 0.2    Update interface.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
//corei#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
#include <osal/osal_int.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_tuner.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
//corei#include <err/errno.h>

#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
//#include <dvb_frontend.h>    //corei
#include <dvb_frontend_common.h>  //corei
#include <misc/rfk.h>
#if 0
#define NIM_PRINTF printf
#else
#define NIM_PRINTF(...) do{}while(0)
#endif

#define TWO_TUNER_SUPPORT

static int nim_hdl = 0;
static int m_cur_hld = -1;
static struct NIM_CHANNEL_CHANGE m_cur_nim_param;

#ifndef TWO_TUNER_SUPPORT
static struct ali_nim_m3501_cfg nim_m3501_cfg;
static struct ali_nim_m3200_cfg nim_m3200_cfg;
//kent,2013.4.7
static struct ali_nim_m3501_cfg nim_m3503_cfg;
//kent,2013.4.24
static struct ali_nim_m3200_cfg nim_m3281_cfg;
static struct ali_nim_mn88436_cfg nim_m3821_cfg;
static struct ali_nim_mn88436_cfg nim_cxd2837_cfg;
static struct ali_nim_mn88436_cfg nim_cxd2838_cfg;

#else
static struct ali_nim_m3501_cfg nim_m3501_cfg[MAX_TUNER_SUPPORT_NUM];
static struct ali_nim_m3200_cfg nim_m3200_cfg[MAX_TUNER_SUPPORT_NUM];
//kent,2013.4.7
static struct ali_nim_m3501_cfg nim_m3503_cfg[MAX_TUNER_SUPPORT_NUM];
//kent,2013.4.24
static struct ali_nim_m3200_cfg nim_m3281_cfg[MAX_TUNER_SUPPORT_NUM];

//michael,2013.6.4
static struct ali_nim_m3200_cfg nim_tda10025_cfg[MAX_TUNER_SUPPORT_NUM];

static struct ali_nim_mn88436_cfg nim_mn88436_cfg[MAX_TUNER_SUPPORT_NUM];
static struct ali_nim_mn88436_cfg nim_m3821_cfg[MAX_TUNER_SUPPORT_NUM];
static struct ali_nim_mn88436_cfg nim_cxd2837_cfg[MAX_TUNER_SUPPORT_NUM];
static struct ali_nim_mn88436_cfg nim_cxd2838_cfg[MAX_TUNER_SUPPORT_NUM];

#endif

static char nim_m3501_devname[MAX_TUNER_SUPPORT_NUM][24] = 
{
	"/dev/ali_m3501_nim0", "/dev/ali_m3501_nim1"
};
//kent,2013.4.7
static char nim_m3503_devname[MAX_TUNER_SUPPORT_NUM][24] = 
{
	"/dev/ali_m3503_nim0", "/dev/ali_m3503_nim1"
};


static char nim_m3200_devname[MAX_TUNER_SUPPORT_NUM][24] = 
{
	"/dev/ali_m3200_nim0", "/dev/ali_m3200_nim1"
};

//kent,2013.4.24
static char nim_m3281_devname[MAX_TUNER_SUPPORT_NUM][24] = 
{
	"/dev/ali_m3281_nim0", "/dev/ali_m3281_nim1"
};

//michael 2013.6.7
static char nim_tda10025_devname[MAX_TUNER_SUPPORT_NUM][24] = 
{
	"/dev/ali_tda10025_nim0", "/dev/ali_tda10025_nim1"
};

static char nim_mn88436_devname[MAX_TUNER_SUPPORT_NUM][24] = 
{
	"/dev/ali_mn88436_nim0", "/dev/ali_mn88436_nim1"
};
static char nim_s3821_devname[MAX_TUNER_SUPPORT_NUM][24]  = 
{
	"/dev/ali_s3821_nim0", "/dev/ali_s3821_nim1"
};
static char nim_cxd2837_devname[MAX_TUNER_SUPPORT_NUM][24] = 
{
	"/dev/ali_cxd2837_nim0", "/dev/ali_cxd2837_nim1"
};
static char nim_cxd2838_devname[MAX_TUNER_SUPPORT_NUM][24] = 
{
	"/dev/ali_cxd2838_nim0", "/dev/ali_cxd2838_nim1"
};


static char nim_m3501_name[3][HLD_MAX_NAME_SIZE] = 
{
	"NIM_S3501_0", "NIM_S3501_1", "NIM_S3501_2"
};

//kent,2013.4.7
static char nim_m3503_name[3][HLD_MAX_NAME_SIZE] = 
{
	"NIM_S3503_0", "NIM_S3503_1", "NIM_S3503_2"
};


static char nim_m3200_name[2][HLD_MAX_NAME_SIZE] = 
{
	"NIM_QAM_S3200", "NIM_QAM_S3200_1"
};

//kent,2013.4.24
static char nim_m3281_name[2][HLD_MAX_NAME_SIZE] = 
{
	"NIM_S3281_0", "NIM_S3281_1"
};


//michael,2013.6.4
static char nim_tda10025_name[2][HLD_MAX_NAME_SIZE] = 
{
	"ali_tda10025_nim0", "ali_tda10025_nim1"
};

static char nim_mn88436_name[2][HLD_MAX_NAME_SIZE] = 
{
	"ali_mn88436_nim0", "ali_mn88436_nim1"
};


static char nim_cxd2837_name[2][HLD_MAX_NAME_SIZE] = 
{
	"ali_cxd2837_nim0", "ali_cxd2837_nim1"
};

static char nim_cxd2838_name[2][HLD_MAX_NAME_SIZE] = 
{
	"ali_cxd2838_nim0", "ali_cxd2838_nim1"
};

static UINT8 	nim_dev_num = 0;
static int 		m_nim_hdl[2];



INT32 nim_DiSEqC_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt);
INT32 nim_DiSEqC2X_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt, \
	UINT8 *rt_value, UINT8 *rt_cnt);


/*****************************************************************************
* INT32 nim_open(struct nim_device *dev)
*
* NIM open operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
#ifndef TWO_TUNER_SUPPORT
INT32 nim_open(struct nim_device *dev)
{
	if (NULL == dev || nim_hdl == 0)
	{
		NIM_PRINTF("nim_open: device or handle invalid!\n");
		return ERR_FAILURE;
	}

	if (dev->flags & HLD_DEV_STATS_UP)
	{
		NIM_PRINTF("nim_open: warning - device %s opened already!\n", dev->name);
		return SUCCESS;
	}

	// Oncer 20110816: cause HLD/nim.c should not depend on frontend type,
	// so real device open operation should be move to attach process...
	if(0==ioctl(nim_hdl, ALI_NIM_HARDWARE_INIT_S, &nim_m3501_cfg))
	{
		dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
		dev->priv = (void *)nim_hdl;
		NIM_PRINTF("open NIM handle success\n");
	}
	else
	{
		NIM_PRINTF("open NIM handle failed\n");
		close(nim_hdl);
		return  ERR_FAILURE;
	}
	
	dev->net_link_portid = rfk_get_port();
	if(dev->net_link_portid <= 0)
	{
		NIM_PRINTF("%s in: HDMI get port fail\n", __FUNCTION__);
		close(nim_hdl);
		dev->flags = 0;
		return ERR_FAILURE;
	}	

	if (0 == ioctl((int)dev->priv, ALI_NIM_SET_NETLINKE_ID, dev->net_link_portid))
	{
		NIM_PRINTF("%s in: NIM register netlink port successfully\n",__FUNCTION__);
	}
	else
	{
		NIM_PRINTF("%s in: NIM register netlink port failed\n",__FUNCTION__);
	}
	return SUCCESS;
}
#else

INT32 nim_open(struct nim_device *dev)
{
	if (NULL == dev)
	{
		NIM_PRINTF("nim_open: Device Invalid!line=%d\n",__LINE__);
		return ERR_FAILURE;
	}

	if (0 == dev->priv)
	{
		NIM_PRINTF("nim_open: Handle Invalid!,line=%d\n",__LINE__);
		return ERR_FAILURE;
	}

	if (dev->flags & HLD_DEV_STATS_UP)
	{
		NIM_PRINTF("nim_open: warning - device %s opened already!\n", dev->name);
		return SUCCESS;
	}
	dev->flags |= HLD_DEV_STATS_UP;	
	dev->net_link_portid = rfk_get_port();
	NIM_PRINTF("net_link_portid: %d\n", dev->net_link_portid);
	if(dev->net_link_portid <= 0)
	{
		NIM_PRINTF("%s in: Nim get port fail\n", __FUNCTION__);
		close(dev->priv);
		dev->flags = 0;
		return ERR_FAILURE;
	}	

	if (0 == ioctl((int)dev->priv, ALI_NIM_SET_NETLINKE_ID, dev->net_link_portid))
	{
		NIM_PRINTF("%s in: NIM register netlink port successfully\n",__FUNCTION__);
	}
	else
	{
		NIM_PRINTF("%s in: NIM register netlink port failed\n",__FUNCTION__);
	}
	return SUCCESS;
}

#endif

/*****************************************************************************
* INT32 nim_close(struct nim_device *dev)
*
* NIM close operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_close(struct nim_device *dev)
{
	INT32 result = SUCCESS;

	if (NULL == dev)
	{
		NIM_PRINTF("nim_close: device handle invalid!\n");
		return ERR_FAILURE;
	}
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		NIM_PRINTF("nim_close: warning - device %s closed already!\n", dev->name);
		return SUCCESS;
	}

	if(0==close((int)dev->priv))
	{
		dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
	}

	/* Update flags */
	if (result == SUCCESS)
	{
		dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
		result = SUCCESS;
	}
	else
	{
		result = ERR_FAILURE;
	}

	return result;
}


/*****************************************************************************
* INT32 nim_tp_resume_lock(struct nim_device *dev)
*
* Resume from fast standby, lock the former TP again
*
* Arguments:
*  Parameter1: void
*
* Return Value: void
*****************************************************************************/
void nim_tp_resume_lock()
{
    if (m_cur_hld > 0)
        ioctl(m_cur_hld, ALI_NIM_CHANNEL_CHANGE, &m_cur_nim_param);
}

/*****************************************************************************
* INT32 nim_ioctl_ext(struct nim_device *dev, INT32 cmd, void * param_list);
*
* NIM device input/output control Extersion
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: INT32  cmd				: Cmd Code
*  Parameter3: void*  param_list		: the pointer to the Parameter List
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_ioctl_ext(struct nim_device *dev, INT32 cmd, void * param_list)
{
	INT32 ret = RET_SUCCESS;
	nim_diseqc_operate_para_t *diseqc_para;
	
	if (NULL == dev)
		return ERR_FAILURE;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

    switch( cmd )
	{
	case NIM_DRIVER_AUTO_SCAN:          /* Do AutoScan Procedure */
		ioctl((int)dev->priv,ALI_NIM_AUTO_SCAN,param_list);

		struct nim_device *nim = dev;//(struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
		unsigned char *nim_data = NULL;
		unsigned char polar = 0;
		unsigned char lck = 0;
		unsigned char fec = 0;
		unsigned short freq = 0;
		unsigned int sym = 0;
		unsigned char as_stat = 0;
		struct NIM_AUTO_SCAN *as_para = (struct NIM_AUTO_SCAN *)param_list;

		while(1)
		{
			nim_data = rfk_receive_msg(nim->net_link_portid);
			if (NULL == nim_data)
			{
				printf("%s in: receive msg fail!\n", __FUNCTION__);
				asm(".word 0x7000003f");
				return;
			}

			memcpy((void *)&lck, ((unsigned char *)nim_data) + 3,1);
			memcpy((void *)&polar, ((unsigned char *)nim_data) + 4,1);
			memcpy((void *)&fec, ((unsigned char *)nim_data) + 5, 1);
			memcpy((void *)&freq, ((unsigned char *)nim_data) + 6, 2);
			memcpy((void *)&sym, ((unsigned char *)nim_data) + 8, 4);
			memcpy((void *)&as_stat, ((unsigned char *)nim_data) + 12, 1);

			if (as_stat)
			{
				break;
			}
			
			if ((as_para)&&(as_para->callback))
			{
				  ret = as_para->callback(NULL,lck,polar, freq, sym, fec,as_stat); 
                if (ret != RET_SUCCESS)
                {   //may space full, so exit auto scan
                    nim_io_control(nim, NIM_DRIVER_STOP_ATUOSCAN, 1);
                }
			}
			ioctl((int)nim->priv,ALI_NIM_AS_SYNC_WITH_LIB,0);
		}

        return ret;
	case NIM_DRIVER_CHANNEL_CHANGE:     /* Do Channel Change */
		NIM_PRINTF(">>> NIM_DRIVER_CHANNEL_CHANGE\n");
		{
            m_cur_hld = (int)dev->priv;
			#if(SYS_PROJECT_FE != PROJECT_FE_ATSC)
		    m_cur_nim_param.freq=((struct NIM_CHANNEL_CHANGE *) (param_list))->freq;
		    m_cur_nim_param.sym=((struct NIM_CHANNEL_CHANGE *) (param_list))->sym;
		    m_cur_nim_param.fec=((struct NIM_CHANNEL_CHANGE *) (param_list))->fec;
		    m_cur_nim_param.modulation=((struct NIM_CHANNEL_CHANGE *) (param_list))->modulation;
			#else
			memcpy(&m_cur_nim_param,param_list,sizeof(struct NIM_CHANNEL_CHANGE));
			#endif
			printf("freq:%d,sym:%d,bandwidth:%d \n",m_cur_nim_param.freq,m_cur_nim_param.sym,m_cur_nim_param.bandwidth);
		    return ioctl((int)dev->priv, ALI_NIM_CHANNEL_CHANGE, &m_cur_nim_param);
		}
#if 0 //( SYS_PROJECT_FE == PROJECT_FE_DVBC )
	case NIM_DRIVER_QUICK_CHANNEL_CHANGE:     /* Do Channel Change */
		NIM_PRINTF(">>> NIM_DRIVER_CHANNEL_CHANGE\n");
		{
		    struct NIM_CHANNEL_CHANGE nim_param;
		    nim_param.freq=((struct NIM_Channel_Change *) (param_list))->freq;
		    nim_param.sym=((struct NIM_Channel_Change *) (param_list))->sym;
		    nim_param.fec=0xff;
		    nim_param.modulation=((struct NIM_Channel_Change *) (param_list))->modulation;
		    return ioctl((int)dev->priv, ALI_NIM_CHANNEL_CHANGE, &nim_param);
		}
	case NIM_DRIVER_GET_CN_VALUE:
		*(UINT16*)param_list = ioctl((int)dev->priv,ALI_NIM_GET_CN_VALUE, 0);
		ret=RET_SUCCESS;
		break;
	case NIM_DRIVER_GET_RF_LEVEL:
		*(UINT16*)param_list = ioctl((int)dev->priv,ALI_NIM_GET_RF_LEVEL, 0);
		ret=RET_SUCCESS;
		break;
	case NIM_DRIVER_GET_BER_VALUE:
		*(UINT32*)param_list = ioctl((int)dev->priv,ALI_NIM_READ_QPSK_BER, 0);
		ret=RET_SUCCESS;
		break;
#endif
	case NIM_DRIVER_CHANNEL_SEARCH: /* Do Channel Search */
		ret=ERR_FAILURE;
		break;
	case NIM_DRIVER_GET_ID:
		*((UINT32 *) param_list) = (UINT32)ioctl((int)dev->priv, ALI_NIM_DRIVER_GET_ID, 0);
		break;
	case NIM_DRIVER_GET_FFT_RESULT:
		ret = ERR_FAILURE;
		break;
#if	(SYS_PROJECT_FE	== PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)		
	case NIM_DRIVER_DISEQC_OPERATION:
		diseqc_para = (nim_diseqc_operate_para_t *)(param_list);		
		return nim_DiSEqC_operate(dev, diseqc_para->mode, diseqc_para->cmd, diseqc_para->cnt);
		//ret = RET_FAILURE;
		//break;
	case NIM_DRIVER_DISEQC2X_OPERATION:
		diseqc_para = (nim_diseqc_operate_para_t *)(param_list);
		return nim_DiSEqC2X_operate(dev, diseqc_para->mode, diseqc_para->cmd, diseqc_para->cnt, \
			diseqc_para->rt_value, diseqc_para->rt_cnt);
		//ret = RET_FAILURE;
		//break;
#endif		
	default:
	    ret=ERR_FAILURE;
		break;
	}
	return ret;
}


/*****************************************************************************
* INT32 nim_io_control(struct nim_device *dev, INT32 cmd, UINT32 param)
*
* NIM do input output control
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: INT32  cmd				: Command
*  Parameter3: INT32  param				: Parameter
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_io_control(struct nim_device *dev, INT32 cmd, UINT32 param)
{
	INT32 ret = RET_SUCCESS;
	
	if (NULL == dev)
		return ERR_FAILURE;
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	switch(cmd)
	{
		case NIM_DRIVER_READ_RSUB:
			*(UINT32 *)param = ioctl(dev->priv, ALI_NIM_READ_RSUB, 0);
			break;
		case NIM_DRIVER_READ_QPSK_BER:
			*(UINT32 *)param = ioctl(dev->priv, ALI_NIM_READ_QPSK_BER, 0);
			break;
		//	return RET_SUCCESS;
		case NIM_DRIVER_STOP_ATUOSCAN:
			ioctl(dev->priv, ALI_NIM_STOP_AUTOSCAN, param);
			return RET_SUCCESS;
		case NIM_DRIVER_GET_CUR_FREQ:
			return ioctl(dev->priv, ALI_NIM_DRIVER_GET_CUR_FREQ, param);
        case NIM_DRIVER_SET_RESET_CALLBACK:
			return RET_SUCCESS;
		case NIM_DRIVER_STOP_CHANSCAN:
			return RET_SUCCESS;
		case NIM_DRIVER_CHANGE_TS_GAP:
			return RET_SUCCESS;
		case NIM_DRIVER_SET_SSI_CLK:
			return RET_SUCCESS;
		case NIM_DRIVER_SET_BLSCAN_MODE:
			return RET_SUCCESS;
		case NIM_DRIVER_SET_POLAR:
			ioctl(dev->priv, ALI_NIM_SET_POLAR, &param);
			break;
		case NIM_DRIVER_GET_SYM:
			return RET_SUCCESS;
        case NIM_TUNER_POWER_CONTROL:
            ioctl(dev->priv, ALI_NIM_TURNER_SET_STANDBY, 0);
            break;
	    case NIM_DRIVER_SEARCH_T2_SIGNAL_ONLY:
   		ioctl(dev->priv, ALI_NIM_T2_SIGNAL_ONLY, param);
   		break;
		default:
			return RET_SUCCESS;
			
	}

	return RET_SUCCESS;
}

/*****************************************************************************
* INT32 nim_get_lock(struct nim_device *dev, UINT8 *lock)
*
* Get NIM lock status
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT8 *lock				: Lock status
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_lock(struct nim_device *dev, UINT8 *lock)
{	
    if (dev == NULL)
    {
        if (lock)
            *lock = 1;
       return SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}
	*lock=ioctl((int)dev->priv, ALI_NIM_GET_LOCK_STATUS, 0);

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_get_freq(struct nim_device *dev, UINT32 *freq)
*
* Get NIM frequence
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT32 *freq				: Frequence
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_freq(struct nim_device *dev, UINT32 *freq)
{
	if (NULL == dev)
	{
		if (freq)
			*freq = 5150 - 4000;
		return SUCCESS;
	}
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*freq=ioctl((int)dev->priv, ALI_NIM_READ_FREQ, 0);

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_get_FEC(struct nim_device *dev, UINT8 *fec)
*
* Get NIM code rate
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT8 *fec				: Code rate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_FEC(struct nim_device *dev, UINT8 *fec)
{
	int ret = 0;
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	ret = ioctl((int)dev->priv, ALI_NIM_READ_CODE_RATE, 0);
	if (ret < 0)
		return -1;
	else
		*fec = ret;

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_get_SNR(struct nim_device *dev, UINT8 *snr)
*
* Get NIM Signal Noise Rate
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT8 *snr				: SNR
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_SNR(struct nim_device *dev, UINT8 *snr)
{	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*snr = ioctl((int)dev->priv, ALI_NIM_READ_SNR, 0);

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_get_AGC(struct nim_device *dev, UINT8 *agc)
*
* NIM DiSEqC operation
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT8 *agc				: AGC
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_AGC(struct nim_device *dev, UINT8 *agc)
{
	if (NULL == dev)
	{
		if (agc)
			*agc = 94;
		return SUCCESS;
	}
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*agc=ioctl((int)dev->priv, ALI_NIM_READ_AGC, 0);

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_get_BER(struct nim_device *dev, UINT32 *ber)
*
* Get NIM Bit Error Rate
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT32 *ber				: BER
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_BER(struct nim_device *dev, UINT32 *ber)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*ber=ioctl((int)dev->priv, ALI_NIM_READ_QPSK_BER, 0);

	return SUCCESS;
}
//Android:
//shoudl always compile this function for combo project!
INT32 nim_get_modulation(struct nim_device *dev, UINT8 *modulation)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*modulation=ioctl((int)dev->priv, ALI_NIM_GET_MODULATION, 0);
		
	return SUCCESS;
}

#if ( SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2 )

/*****************************************************************************
* INT32 nim_DiSEqC_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt)
*
* NIM DiSEqC operation
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT32 mode				: DeSEqC mode
*  Parameter3: UINT8* cmd				: Command
*  Parameter4: UINT8  cnt				: Command length byte
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_DiSEqC_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt)
{
	INT32 ret, i;
	struct ali_nim_diseqc_cmd diseqc_cmd;
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	diseqc_cmd.mode=mode;
	for(i=0; i<cnt; i++)
	{
		diseqc_cmd.cmd[i]=cmd[i];
	}
	diseqc_cmd.cmd_size=cnt;
	diseqc_cmd.diseqc_type=1; //nim_DiSEqC_operate;
		
	ret=ioctl((int)dev->priv, ALI_NIM_DISEQC_OPERATE, &diseqc_cmd);

	if(0==ret)
	{
		NIM_PRINTF("%s %d success!\n", __FUNCTION__, mode);
		return SUCCESS;
	}
	else
	{
		NIM_PRINTF("%s %d success!\n", __FUNCTION__, mode);
		return ret;
	}

}

INT32 nim_DiSEqC2X_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt,
									UINT8 *rt_value, UINT8 *rt_cnt)
{
	INT32 ret, i;
	struct ali_nim_diseqc_cmd diseqc_cmd;
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	diseqc_cmd.mode=mode;
	for(i=0; i<cnt; i++)
	{
		diseqc_cmd.cmd[i]=cmd[i];
	}
	diseqc_cmd.cmd_size=cnt;
	diseqc_cmd.diseqc_type=2; //nim_DiSEqC2X_operate;
		
	ret=ioctl((int)dev->priv, ALI_NIM_DISEQC_OPERATE, &diseqc_cmd);

	if(0==ret)
	{
		NIM_PRINTF("%s %d success!\n", __FUNCTION__, mode);
		for(i=0; i<diseqc_cmd.ret_len; i++)
		{
			rt_value[i]=diseqc_cmd.ret_bytes[i];
		}
		*rt_cnt=diseqc_cmd.ret_len;
		return SUCCESS;
	}
	else
	{
		NIM_PRINTF("%s %d failed!\n", __FUNCTION__, mode);
		return ret;
	}
}

/*****************************************************************************
* INT32 nim_set_polar(struct nim_device *dev, UINT8 polar);
*
* NIM polarisation set up
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: INT8 polar				: Polarisation
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_set_polar(struct nim_device *dev, UINT8 polar)
{
#if 0 //useless
	INT32 ret;
	NIM_PRINTF("%s: dev:%s, nim_hdl:%d, polar:%d\n",__FUNCTION__, dev->name, (int)dev->priv, polar);
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		NIM_PRINTF("%s: nim status error. %d\n",__FUNCTION__, dev->flags);
		return ERR_DEV_ERROR;
	}
	ret = ioctl((int)dev->priv, ALI_NIM_SET_POLAR, polar);
	
	NIM_PRINTF("%s: return %d!\n",__FUNCTION__, ret);
	return ret;
#endif	
}

/*****************************************************************************
* INT32 nim_set_12v(struct nim_device *dev, UINT8 flag);
*
* LNB 12V supply
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: INT8 flag				: 12V enable or not
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_set_12v(struct nim_device *dev, UINT8 flag)
{
	/* If device not running, exit */
#if 0
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->set_12v)
	{
		return dev->set_12v(dev, flag);
	}
#endif

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);
*
* NIM channel change operation
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT32 freq				: Frequence in MHz
*  Parameter3: UINT32 sym				: Symbol rate in KHz
*  Parameter4: UINT8  fec				: Code rate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec)
{
	struct NIM_CHANNEL_CHANGE nim_param;
	NIM_PRINTF("%s: freq:%d, sym:%d, fec:%d\n",__FUNCTION__, freq, sym,fec);

	if (NULL == dev)
		return ERR_FAILURE;
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}
	memset(&nim_param,0, sizeof(struct NIM_CHANNEL_CHANGE));
	nim_param.freq=freq;
	nim_param.sym=sym;
//#if ( SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2 )
	nim_param.fec=fec;
	nim_param.bandwidth = sym;
//#elif ( SYS_PROJECT_FE == PROJECT_FE_DVBC )
//	nim_param.modulation=fec;
//#endif
	return ioctl((int)dev->priv, ALI_NIM_CHANNEL_CHANGE, &nim_param);
}

/*****************************************************************************
* INT32 nim_channel_search(struct nim_device *dev, UINT32 freq);
*
* NIM channel search operation
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter3: UINT8  freq				: Frequence in MHz
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_channel_search(struct nim_device *dev, UINT32 freq)
{
	/* If device not running, exit */
#if 0
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->channel_search)
	{
		return dev->channel_search(dev, freq);
	}
#endif

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_get_sym(struct nim_device *dev, UINT32 *sym)
*
* Get NIM symbol rate
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT32 *sym				: Symbol rate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_sym(struct nim_device *dev, UINT32 *sym)
{

    if (dev == NULL)
    {
        if (sym)
            *sym = 30000;
         return SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*sym=ioctl((int)dev->priv, ALI_NIM_READ_SYMBOL_RATE, 0);

	return SUCCESS;
}

INT32 nim_get_PER(struct nim_device *dev, UINT32 *per)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*per=ioctl((int)dev->priv, ALI_NIM_READ_RSUB, 0);

	return SUCCESS;
}

INT32 nim_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32* start_adr)
{
	/* If device not running, exit */
#if 0
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_fft_result)
	{
		return dev->get_fft_result(dev, freq, start_adr);
	}
	else
		return ERR_FAILUE;
#endif
}



void nim_m3501_resume_init(void)
{
    int i;
    for (i = 0; i <= nim_dev_num; i ++)
    {
    //    ioctl(m_nim_hdl[i], ALI_NIM_HARDWARE_INIT, &nim_m3501_cfg[i]);
    }
}

#ifndef TWO_TUNER_SUPPORT
INT32 nim_m3501_attach (struct QPSK_TUNER_CONFIG_API * ptrQPSK_Tuner)
{
    struct nim_device *dev;

	NIM_PRINTF("[kangzh] line=%d,function=%s!\n",__LINE__,__FUNCTION__);
	
	if (NULL == ptrQPSK_Tuner)
	{
		NIM_PRINTF("Tuner configuration API structure is NULL!\n");
		return ERR_NO_DEV;
	}
	if (2 <= nim_dev_num)
	{
		NIM_PRINTF("Can not support more than 2 3501!\n");
		return ERR_NO_DEV;
	}
    dev = (struct nim_device *)dev_alloc(nim_m3501_name[nim_dev_num], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if (dev == NULL)
    {
        NIM_PRINTF("Error: Alloc nim device error!\n");
        return ERR_NO_MEM;
    }
	
    nim_m3501_cfg.Recv_Freq_High=ptrQPSK_Tuner->config_data.Recv_Freq_High;
    nim_m3501_cfg.Recv_Freq_Low=ptrQPSK_Tuner->config_data.Recv_Freq_Low;
    nim_m3501_cfg.QPSK_Config=ptrQPSK_Tuner->config_data.QPSK_Config;
    nim_m3501_cfg.demod_i2c_id=ptrQPSK_Tuner->ext_dm_config.i2c_type_id;
    nim_m3501_cfg.demod_i2c_addr=ptrQPSK_Tuner->ext_dm_config.i2c_base_addr;
    nim_m3501_cfg.tuner_i2c_id=ptrQPSK_Tuner->tuner_config.i2c_type_id;
    nim_m3501_cfg.tuner_i2c_addr=ptrQPSK_Tuner->tuner_config.cTuner_Base_Addr;
	nim_m3501_cfg.tuner_id=ptrQPSK_Tuner->tuner_id; //kent
	
	
    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS )
    {
        NIM_PRINTF("Error: Register nim device error!\n");
        dev_free(dev);
        return ERR_NO_DEV;
    }

	// Oncer.Yu 20110816: device open moved here from nim_open 
	nim_hdl = open("/dev/ali_m3501_nim0", O_RDWR);
	if(nim_hdl == 0)
	{
		NIM_PRINTF("open NIM handle fail\n");
		return ERR_FAILURE;
	}
    m_nim_hdl[nim_dev_num] = nim_hdl;

	//kent
/*	if(0==ioctl(nim_hdl, ALI_NIM_HARDWARE_INIT, &nim_m3501_cfg))
	{		
		dev->flags |= HLD_DEV_STATS_RUNNING;
		dev->priv = (void *)nim_hdl;
		NIM_PRINTF("NIM Harware Init Success\n");
	}
	else
	{
		NIM_PRINTF("NIM Hardware Init Failed\n");
		close(nim_hdl);
		return  ERR_FAILURE;
	}
*/


	
    return SUCCESS;
}
#else
INT32 nim_m3501_attach (struct QPSK_TUNER_CONFIG_API * ptrQPSK_Tuner)
{
    struct nim_device *dev;
	static UINT8 nim_dev_num = 0;
	int hdl = 0;
	NIM_PRINTF("[kangzh] line=%d,function=%s!\n",__LINE__,__FUNCTION__);

	if (NULL == ptrQPSK_Tuner)
	{
		NIM_PRINTF("Tuner configuration API structure is NULL!\n");
		return ERR_NO_DEV;
	}
	if (2 <= nim_dev_num)
	{
		NIM_PRINTF("Can not support more than 2 3501!\n");
		return ERR_NO_DEV;
	}
    dev = (struct nim_device *)dev_alloc(nim_m3501_name[nim_dev_num], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if (dev == NULL)
    {
        NIM_PRINTF("Error: Alloc nim device error!\n");
        return ERR_NO_MEM;
    }

	/* Add this device to queue */
    if (dev_register(dev) != SUCCESS )
    {
    	NIM_PRINTF("Error: Register nim device error!\n");
       	dev_free(dev);
       	return ERR_NO_DEV;
    }

    nim_m3501_cfg[nim_dev_num].recv_freq_high=ptrQPSK_Tuner->config_data.recv_freq_high;
    nim_m3501_cfg[nim_dev_num].recv_freq_low=ptrQPSK_Tuner->config_data.recv_freq_low;
    nim_m3501_cfg[nim_dev_num].qpsk_config=ptrQPSK_Tuner->config_data.qpsk_config;
    nim_m3501_cfg[nim_dev_num].demod_i2c_id=ptrQPSK_Tuner->ext_dm_config.i2c_type_id;
    nim_m3501_cfg[nim_dev_num].demod_i2c_addr=ptrQPSK_Tuner->ext_dm_config.i2c_base_addr;
    nim_m3501_cfg[nim_dev_num].tuner_i2c_id=ptrQPSK_Tuner->tuner_config.i2c_type_id;
    nim_m3501_cfg[nim_dev_num].tuner_i2c_addr=ptrQPSK_Tuner->tuner_config.c_tuner_base_addr;
	nim_m3501_cfg[nim_dev_num].tuner_id=ptrQPSK_Tuner->tuner_id; //kent
	
	// Oncer.Yu 20110816: device open moved here from nim_open 
	hdl = open(nim_m3501_devname[nim_dev_num], O_RDWR);
	if(hdl <= 0)
	{
		NIM_PRINTF("Open NIM Handle Fail\n");
		return ERR_FAILURE;
	}
    m_nim_hdl[nim_dev_num] = hdl;    
	// Oncer 20110816: cause HLD/nim.c should not depend on frontend type,
	// so real device open operation should be move to attach process...
	if(0==ioctl(hdl, ALI_NIM_HARDWARE_INIT_S, &nim_m3501_cfg[nim_dev_num]))
	{		
		dev->flags |= HLD_DEV_STATS_RUNNING;
		dev->priv = (void *)hdl;
		NIM_PRINTF("NIM Harware Init Success\n");
	}
	else
	{
		NIM_PRINTF("NIM Hardware Init Failed\n");
		close(hdl);
		return  ERR_FAILURE;
	}

	nim_dev_num ++;
    return SUCCESS;
}

#endif

INT32 nim_m3501_dettach ( struct nim_device *dev)
{
	if(dev->flags & (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING))
	{
		NIM_PRINTF("Error, NIM device still running.\n");
		return !SUCCESS;
	}
	dev_free(dev);
	return SUCCESS;
}

#elif (SYS_PROJECT_FE == PROJECT_FE_DVBT)||(SYS_PROJECT_FE == PROJECT_FE_ATSC) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)

INT32 nim_mn88435_dettach ( struct nim_device *dev)
{
	if(dev->flags & (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING))
	{
		NIM_PRINTF("Error, NIM device still running.\n");
		return !SUCCESS;
	}
	dev_free(dev);
	return SUCCESS;
}

INT32 nim_mn88435_attach (struct COFDM_TUNER_CONFIG_API * ptrCOFDM_Tuner)
{
    struct nim_device *dev;
	static UINT8 nim_dev_num = 0;
	int hdl = 0;
	NIM_PRINTF("[kangzh] line=%d,function=%s!\n",__LINE__,__FUNCTION__);

	if (NULL == ptrCOFDM_Tuner)
	{
		NIM_PRINTF("Tuner configuration API structure is NULL!\n");
		return ERR_NO_DEV;
	}
	if (2 <= nim_dev_num)
	{
		NIM_PRINTF("Can not support more than 2 3501!\n");
		return ERR_NO_DEV;
	}
    dev = (struct nim_device *)dev_alloc(nim_mn88436_name[nim_dev_num], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if (dev == NULL)
    {
        NIM_PRINTF("Error: Alloc nim device error!\n");
        return ERR_NO_MEM;
    }

	/* Add this device to queue */
    if (dev_register(dev) != SUCCESS )
    {
    	NIM_PRINTF("Error: Register nim device error!\n");
       	dev_free(dev);
       	return ERR_NO_DEV;
    }
	memcpy(&nim_mn88436_cfg[nim_dev_num],ptrCOFDM_Tuner,sizeof(struct COFDM_TUNER_CONFIG_API));

	hdl = open(nim_mn88436_devname[nim_dev_num], O_RDWR);
	if(hdl <= 0)
	{
		NIM_PRINTF("Open NIM Handle Fail\n");
		return ERR_FAILURE;
	}
    m_nim_hdl[nim_dev_num] = hdl;    
	
	if(0==ioctl(hdl, ALI_NIM_HARDWARE_INIT_T, &nim_mn88436_cfg[nim_dev_num]))
	{		
		dev->flags |= HLD_DEV_STATS_RUNNING;
		dev->priv = (void *)hdl;
		NIM_PRINTF("NIM Harware Init Success\n");
	}
	else
	{
		NIM_PRINTF("NIM Hardware Init Failed\n");
		close(hdl);
		return  ERR_FAILURE;
	}

	nim_dev_num ++;
    return SUCCESS;
}



#ifdef SMART_ANT_SUPPORT
INT32 nim_Set_Smartenna(struct nim_device *dev, UINT8 position,UINT8 gain,UINT8 pol,UINT8 channel)
{
   	return SUCCESS;
}

INT32 nim_Get_SmartennaSetting(struct nim_device *dev,UINT8 *pPosition,UINT8 *pGain,UINT8 *pPol,UINT8 *pChannel)
{   
	return SUCCESS;
}

INT32 nim_Get_SmartennaMetric(struct nim_device *dev, UINT8 metric,UINT16 *pMetric)
{
	return SUCCESS;
}

INT32 nim_get_VSB_AGC(struct nim_device *dev, UINT16 *agc)
{
	return SUCCESS;
}

INT32 nim_get_VSB_SNR(struct nim_device *dev, UINT16 *snr)
{
	return SUCCESS;
}

INT32 nim_get_VSB_PER(struct nim_device *dev, UINT32 *per)
{
	return SUCCESS;
}
#endif

INT32 nim_disable(struct nim_device *dev)
{
	return SUCCESS;
}

INT32 nim_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT8 priority)
{	
	return SUCCESS;	
}

INT32 nim_channel_search(struct nim_device *dev, UINT32 freq,UINT32 bandwidth,UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT16 freq_offset, UINT8 priority)
{
    return SUCCESS;
}

INT32 nim_get_gi(struct nim_device *dev, UINT8 *guard_interval)
{
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*guard_interval=ioctl((int)dev->priv, ALI_NIM_GET_GUARD_INTERVAL, 0);

	
	return SUCCESS;
}


INT32 nim_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*fft_mode=ioctl((int)dev->priv, ALI_NIM_GET_FFT_MODE, 0);
	
	return SUCCESS;
}

INT32	nim_get_spec_inv(struct nim_device *dev, UINT8 *inv)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*inv=ioctl((int)dev->priv, ALI_NIM_GET_SPECTRUM_INV, 0);
			
	return SUCCESS;
}





#if (SYS_SDRAM_SIZE == 8)
INT32 nim_get_HIER_mode(struct nim_device *dev, UINT8*hier)
{	
	return SUCCESS;
}

INT8 nim_get_priority(struct nim_device *dev, UINT8*priority)
{
	return SUCCESS;
}
#endif

#if (SYS_SDRAM_SIZE == 8 || GET_BER == SYS_FUNC_ON)
INT32 nim_get_BER(struct nim_device *dev, UINT32 *ber)
{
	return SUCCESS;
}
#endif

#if ((SYS_SDRAM_SIZE == 2 || SYS_SDRAM_SIZE == 8) \
	||(SYS_PROJECT_FE==PROJECT_FE_ATSC) || (SYS_PROJECT_FE==PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT))
INT32 nim_get_freq_offset(struct nim_device *dev, INT32 *freq_offset)//051222 yuchun
{
	return SUCCESS;
}
#endif

#elif ( SYS_PROJECT_FE == PROJECT_FE_DVBC )

INT32 nim_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec)
{
	struct NIM_CHANNEL_CHANGE nim_param;
	NIM_PRINTF("%s: freq:%d, sym:%d, fec:%d\n",__FUNCTION__, freq, sym,fec);

	if (NULL == dev)
		return ERR_FAILURE;
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}
	memset(&nim_param,0, sizeof(struct NIM_CHANNEL_CHANGE));
	nim_param.freq=freq;
	nim_param.sym=sym;
//#if ( SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2 )
//	nim_param.fec=fec;
//#elif ( SYS_PROJECT_FE == PROJECT_FE_DVBC )
	nim_param.modulation=fec;
//#endif
	return ioctl((int)dev->priv, ALI_NIM_CHANNEL_CHANGE, &nim_param);
}

INT32 nim_quick_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec)
{
	struct NIM_Channel_Change dvb_c_CC_Param;

	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	memset(&dvb_c_CC_Param, 0, sizeof(struct NIM_CHANNEL_CHANGE));
	dvb_c_CC_Param.freq = freq;
	dvb_c_CC_Param.sym = sym;
	dvb_c_CC_Param.fec = 0xff;
	dvb_c_CC_Param.modulation = fec;

	return ioctl((int)dev->priv, ALI_NIM_CHANNEL_CHANGE, &dvb_c_CC_Param);
}

/*****************************************************************************
* INT32 nim_channel_search(struct nim_device *dev, UINT32 freq);
*
* NIM channel search operation
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter3: UINT8  freq				: Frequence in MHz
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_channel_search(struct nim_device *dev, UINT32 freq)
{
	/* If device not running, exit */
#if 0
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->channel_search)
	{
		return dev->channel_search(dev, freq);
	}
#endif

	return SUCCESS;
}

INT32 nim_get_sym(struct nim_device *dev, UINT32 *sym)
{
    if (dev == NULL)
    {
        if (sym)
            *sym = 6900;
         return SUCCESS;
    }
    
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*sym=ioctl((int)dev->priv, ALI_NIM_READ_SYMBOL_RATE, 0);

	return SUCCESS;
}


INT32 nim_reg_read(struct nim_device *dev, UINT8 RegAddr, UINT8 *pData, UINT8 bLen)
{
	UINT8 reg_rw[16];
	INT32 ret;
	reg_rw[0]=1; //Read
	reg_rw[1]=RegAddr;
	reg_rw[2]=bLen;
	ret=ioctl((int)dev->priv, ALI_NIM_REG_RW, reg_rw);
	if(SUCCESS==ret)
	{
		memcpy(pData, &(reg_rw[3]), bLen);
	}
	return ret;
}

INT32 nim_reg_write(struct nim_device *dev, UINT8 RegAddr, UINT8 *pData, UINT8 bLen)
{
	UINT8 reg_rw[16];
	reg_rw[0]=2; //Write
	reg_rw[1]=RegAddr;
	reg_rw[2]=bLen;
	memcpy(&(reg_rw[3]), pData, bLen);
	return ioctl((int)dev->priv, ALI_NIM_REG_RW, reg_rw);
}
#endif

/*****************************************************************************
* INT32 nim_s3503_attach (struct QPSK_TUNER_CONFIG_API * ptrQPSK_Tuner)
* Description: S3501 initialization
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/

//kent,2013.4.7
#ifndef TWO_TUNER_SUPPORT
INT32 nim_m3503_attach (struct QPSK_TUNER_CONFIG_API * ptrQPSK_Tuner)
{
    struct nim_device *dev;

	NIM_PRINTF("[kangzh] line=%d,function=%s!\n",__LINE__,__FUNCTION__);
	
	if (NULL == ptrQPSK_Tuner)
	{
		NIM_PRINTF("Tuner configuration API structure is NULL!\n");
		return ERR_NO_DEV;
	}
	if (2 <= nim_dev_num)
	{
		NIM_PRINTF("Can not support more than 2 3501!\n");
		return ERR_NO_DEV;
	}
    dev = (struct nim_device *)dev_alloc(nim_m3503_name[nim_dev_num], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if (dev == NULL)
    {
        NIM_PRINTF("Error: Alloc nim device error!\n");
        return ERR_NO_MEM;
    }
	
    nim_m3503_cfg.recv_freq_high=ptrQPSK_Tuner->config_data.Recv_Freq_High;
    nim_m3503_cfg.recv_freq_low=ptrQPSK_Tuner->config_data.Recv_Freq_Low;
    nim_m3503_cfg.qpsk_config=ptrQPSK_Tuner->config_data.QPSK_Config;
    nim_m3503_cfg.demod_i2c_id=ptrQPSK_Tuner->ext_dm_config.i2c_type_id;
    nim_m3503_cfg.demod_i2c_addr=ptrQPSK_Tuner->ext_dm_config.i2c_base_addr;
    nim_m3503_cfg.tuner_i2c_id=ptrQPSK_Tuner->tuner_config.i2c_type_id;
    nim_m3503_cfg.tuner_i2c_addr=ptrQPSK_Tuner->tuner_config.cTuner_Base_Addr;
	nim_m3503_cfg.tuner_id=ptrQPSK_Tuner->tuner_id; //kent
	
    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS )
    {
        NIM_PRINTF("Error: Register nim device error!\n");
        dev_free(dev);
        return ERR_NO_DEV;
    }

	// Oncer.Yu 20110816: device open moved here from nim_open 
	nim_hdl = open("/dev/ali_m3503_nim0", O_RDWR);
	if(nim_hdl == 0)
	{
		NIM_PRINTF("open NIM handle fail\n");
		return ERR_FAILURE;
	}
    m_nim_hdl[nim_dev_num] = nim_hdl;



	//kent
/*	if(0==ioctl(nim_hdl, ALI_NIM_HARDWARE_INIT, &nim_m3503_cfg))
	{		
		dev->flags |= HLD_DEV_STATS_RUNNING;
		dev->priv = (void *)nim_hdl;
		NIM_PRINTF("NIM Harware Init Success\n");
	}
	else
	{
		NIM_PRINTF("NIM Hardware Init Failed\n");
		close(nim_hdl);
		return  ERR_FAILURE;
	}
*/


	
    return SUCCESS;
}
#else
INT32 nim_m3503_attach (struct QPSK_TUNER_CONFIG_API * ptrQPSK_Tuner)
{
    struct nim_device *dev;
	static UINT8 nim_dev_num = 0;
	int hdl = 0;
	NIM_PRINTF("[kangzh] line=%d,function=%s!\n",__LINE__,__FUNCTION__);

	if (NULL == ptrQPSK_Tuner)
	{
		NIM_PRINTF("Tuner configuration API structure is NULL!\n");
		return ERR_NO_DEV;
	}
	if (2 <= nim_dev_num)
	{
		NIM_PRINTF("Can not support more than 2 3501!\n");
		return ERR_NO_DEV;
	}
    dev = (struct nim_device *)dev_alloc(nim_m3503_name[nim_dev_num], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if (dev == NULL)
    {
        NIM_PRINTF("Error: Alloc nim device error!\n");
        return ERR_NO_MEM;
    }

	/* Add this device to queue */
    if (dev_register(dev) != SUCCESS )
    {
    	NIM_PRINTF("Error: Register nim device error!\n");
       	dev_free(dev);
       	return ERR_NO_DEV;
    }

    nim_m3503_cfg[nim_dev_num].recv_freq_high=ptrQPSK_Tuner->config_data.recv_freq_high;
    nim_m3503_cfg[nim_dev_num].recv_freq_low=ptrQPSK_Tuner->config_data.recv_freq_low;
    nim_m3503_cfg[nim_dev_num].qpsk_config=ptrQPSK_Tuner->config_data.qpsk_config;
    nim_m3503_cfg[nim_dev_num].demod_i2c_id=ptrQPSK_Tuner->ext_dm_config.i2c_type_id;
    nim_m3503_cfg[nim_dev_num].demod_i2c_addr=ptrQPSK_Tuner->ext_dm_config.i2c_base_addr;
    nim_m3503_cfg[nim_dev_num].tuner_i2c_id=ptrQPSK_Tuner->tuner_config.i2c_type_id;
    nim_m3503_cfg[nim_dev_num].tuner_i2c_addr=ptrQPSK_Tuner->tuner_config.c_tuner_base_addr;
	nim_m3503_cfg[nim_dev_num].tuner_id=ptrQPSK_Tuner->tuner_id; //kent
	
	// Oncer.Yu 20110816: device open moved here from nim_open 
	hdl = open(nim_m3503_devname[nim_dev_num], O_RDWR);
	if(hdl <= 0)
	{
		NIM_PRINTF("Open NIM Handle Fail\n");
		return ERR_FAILURE;
	}
    m_nim_hdl[nim_dev_num] = hdl;    
	// Oncer 20110816: cause HLD/nim.c should not depend on frontend type,
	// so real device open operation should be move to attach process...
	if(0==ioctl(hdl, ALI_NIM_HARDWARE_INIT_S, &nim_m3503_cfg[nim_dev_num]))
	{		
		dev->flags |= HLD_DEV_STATS_RUNNING;
		dev->priv = (void *)hdl;
		NIM_PRINTF("NIM Harware Init Success\n");
	}
	else
	{
		NIM_PRINTF("NIM Hardware Init Failed\n");
		close(hdl);
		return  ERR_FAILURE;
	}

	nim_dev_num ++;
    return SUCCESS;
}

#endif




















//#define YS_DUAL_DVBC
#ifdef TWO_TUNER_SUPPORT
INT32 nim_s3202_attach(struct QAM_TUNER_CONFIG_API * ptrQAM_Tuner)
{
	struct nim_device *dev;
	static UINT8 nim_dev_idx = 0;
	int hdl = 0;

	if (NULL == ptrQAM_Tuner)
	{
		NIM_PRINTF("DVBC Tuner configuration API structure is NULL!\n");
		return ERR_NO_DEV;
	}
	if (2 <= nim_dev_idx)
	{
		NIM_PRINTF("Can not support more than 2 Tuner!\n");
		return ERR_NO_DEV;
	}
	
       /* Alloc structure space of tuner devive*/
	dev = (struct nim_device *)dev_alloc(nim_m3200_name[nim_dev_idx], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
	if (dev == NULL)
	{
		NIM_PRINTF("Error: Alloc nim device error!\n");
		return ERR_NO_MEM;
	}

#ifdef YS_DUAL_DVBC 
	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS)
	{
		NIM_PRINTF("Error: Register nim device error!\n");
		//free(priv);
		dev_free(dev);
		return ERR_NO_DEV;
	}
#endif
//	printf("APP Register NIM3200 Success!\n");

	/* tuner configuration function */
	memcpy((void*)&(nim_m3200_cfg[nim_dev_idx].tuner_config_data), (void*)&(ptrQAM_Tuner->tuner_config_data), 
				sizeof(struct QAM_TUNER_CONFIG_DATA));
    memcpy((void*)&(nim_m3200_cfg[nim_dev_idx].tuner_config_ext), (void*)&(ptrQAM_Tuner->tuner_config_ext), 
				sizeof(struct QAM_TUNER_CONFIG_EXT));

	nim_m3200_cfg[nim_dev_idx].tuner_id=ptrQAM_Tuner->tuner_id; //kent
	
	// Oncer 20110816: move from nim_open
	hdl = open(nim_m3200_devname[nim_dev_idx], O_RDWR);
	if(hdl == 0)
	{
		NIM_PRINTF("open NIM handle fail\n");
		return ERR_FAILURE;
	}
	if(0==ioctl(hdl, ALI_NIM_HARDWARE_INIT_C, &nim_m3200_cfg[nim_dev_idx]))
	{
#ifndef YS_DUAL_DVBC		
		/* Add this device to queue */
		if (dev_register(dev) != SUCCESS)
		{
			NIM_PRINTF("Error: Register nim device error!\n");
			//free(priv);
			dev_free(dev);
			close(hdl);
			return ERR_NO_DEV;
		}
#endif		
		dev->flags |= HLD_DEV_STATS_RUNNING;
		dev->priv = (void *)hdl;
		NIM_PRINTF("NIM Harware Init Success\n");
	}
	else
	{
		NIM_PRINTF("NIM Hardware Init Failed\n");
#ifndef YS_DUAL_DVBC
		dev_free(dev);
#endif
		close(hdl);
		return  ERR_FAILURE;
	}

	nim_dev_idx ++;
	return SUCCESS;
}


INT32 nim_s3202_dettach ( struct nim_device *dev)
{
	if(dev->flags & (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING))
	{
		NIM_PRINTF("Error, NIM device still running.\n");
		return !SUCCESS;
	}
	dev_free(dev);
	return SUCCESS;
}

#endif



#ifdef TWO_TUNER_SUPPORT
INT32 nim_mxl241_attach(struct QAM_TUNER_CONFIG_API * ptrQAM_Tuner)
{
	struct nim_device *dev;
	static UINT8 nim_dev_idx = 0;
	int hdl = 0;

	if (NULL == ptrQAM_Tuner)
	{
		NIM_PRINTF("DVBC Tuner configuration API structure is NULL!\n");
		return ERR_NO_DEV;
	}
	if (2 <= nim_dev_idx)
	{
		NIM_PRINTF("Can not support more than 2 Tuner!\n");
		return ERR_NO_DEV;
	}
	
       /* Alloc structure space of tuner devive*/
	//dev = (struct nim_device *)dev_alloc(nim_m3200_name[nim_dev_idx], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
	dev = (struct nim_device *)dev_alloc("NIM_QAM_MXL241_0", HLD_DEV_TYPE_NIM, sizeof(struct nim_device));   
	if (dev == NULL)
	{
		NIM_PRINTF("Error: Alloc nim device error!\n");
		return ERR_NO_MEM;
	}

	/* tuner configuration function */
	memcpy((void*)&(nim_m3200_cfg[nim_dev_idx].tuner_config_data), (void*)&(ptrQAM_Tuner->tuner_config_data), 
				sizeof(struct QAM_TUNER_CONFIG_DATA));
    memcpy((void*)&(nim_m3200_cfg[nim_dev_idx].tuner_config_ext), (void*)&(ptrQAM_Tuner->tuner_config_ext), 
				sizeof(struct QAM_TUNER_CONFIG_EXT));

	nim_m3200_cfg[nim_dev_idx].qam_mode=ptrQAM_Tuner->dem_config_advanced.qam_config_advanced;
	
	// Oncer 20110816: move from nim_open
	//hdl = open(nim_m3200_devname[nim_dev_idx], O_RDWR);
	hdl = open("/dev/ali_mxl241_nim0", O_RDWR);	
	if(hdl == 0)
	{
		printf("open NIM handle fail\n");
		return ERR_FAILURE;
	}
	if(0==ioctl(hdl, ALI_NIM_HARDWARE_INIT_C, &nim_m3200_cfg[nim_dev_idx]))
	{
		/* Add this device to queue */
		if (dev_register(dev) != SUCCESS)
		{
			NIM_PRINTF("Error: Register nim device error!\n");
			//free(priv);
			dev_free(dev);
			close(hdl);
			return ERR_NO_DEV;
		}
		dev->flags |= HLD_DEV_STATS_RUNNING;
		dev->priv = (void *)hdl;
		printf("NIM Harware Init Success\n");
	}
	else
	{
		printf("NIM Hardware Init Failed\n");
		dev_free(dev);
		close(hdl);
		return  ERR_FAILURE;
	}

	nim_dev_idx ++;
	return SUCCESS;
}


#endif




#ifdef TWO_TUNER_SUPPORT
INT32 nim_s3281_attach(struct QAM_TUNER_CONFIG_API * ptrQAM_Tuner)
{
	struct nim_device *dev;
	static UINT8 nim_dev_idx = 0;
	int hdl = 0;

	if (NULL == ptrQAM_Tuner)
	{
		NIM_PRINTF("DVBC Tuner configuration API structure is NULL!\n");
		return ERR_NO_DEV;
	}
	if (2 <= nim_dev_idx)
	{
		NIM_PRINTF("Can not support more than 2 Tuner!\n");
		return ERR_NO_DEV;
	}
	
       /* Alloc structure space of tuner devive*/
	dev = (struct nim_device *)dev_alloc(nim_m3281_name[nim_dev_idx], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
	if (dev == NULL)
	{
		NIM_PRINTF("Error: Alloc nim device error!\n");
		return ERR_NO_MEM;
	}

#ifdef YS_DUAL_DVBC 
	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS)
	{
		NIM_PRINTF("Error: Register nim device error!\n");
		//free(priv);
		dev_free(dev);
		return ERR_NO_DEV;
	}
#endif
//	printf("APP Register NIM3200 Success!\n");

	/* tuner configuration function */
	memcpy((void*)&(nim_m3281_cfg[nim_dev_idx].tuner_config_data), (void*)&(ptrQAM_Tuner->tuner_config_data), 
				sizeof(struct QAM_TUNER_CONFIG_DATA));
    memcpy((void*)&(nim_m3281_cfg[nim_dev_idx].tuner_config_ext), (void*)&(ptrQAM_Tuner->tuner_config_ext), 
				sizeof(struct QAM_TUNER_CONFIG_EXT));

	nim_m3281_cfg[nim_dev_idx].qam_mode=ptrQAM_Tuner->dem_config_advanced.qam_config_advanced;
	nim_m3281_cfg[nim_dev_idx].tuner_id=ptrQAM_Tuner->tuner_id; //kent
	
	// Oncer 20110816: move from nim_open
	hdl = open(nim_m3281_devname[nim_dev_idx], O_RDWR);
	if(hdl == 0)
	{
		NIM_PRINTF("open NIM handle fail\n");
		return ERR_FAILURE;
	}
	if(0==ioctl(hdl, ALI_NIM_HARDWARE_INIT_C, &nim_m3281_cfg[nim_dev_idx]))
	{
#ifndef YS_DUAL_DVBC		
		/* Add this device to queue */
		if (dev_register(dev) != SUCCESS)
		{
			NIM_PRINTF("Error: Register nim device error!\n");
			//free(priv);
			dev_free(dev);
			close(hdl);
			return ERR_NO_DEV;
		}
#endif		
		dev->flags |= HLD_DEV_STATS_RUNNING;
		dev->priv = (void *)hdl;
		NIM_PRINTF("NIM Harware Init Success\n");
	}
	else
	{
		NIM_PRINTF("NIM Hardware Init Failed\n");
#ifndef YS_DUAL_DVBC
		dev_free(dev);
#endif
		close(hdl);
		return  ERR_FAILURE;
	}

	nim_dev_idx ++;
	return SUCCESS;
}


INT32 nim_s3281_dettach ( struct nim_device *dev)
{
	if(dev->flags & (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING))
	{
		NIM_PRINTF("Error, NIM device still running.\n");
		return !SUCCESS;
	}
	dev_free(dev);
	return SUCCESS;
}

#endif




INT32 nim_tda10025_attach(struct QAM_TUNER_CONFIG_API * ptrQAM_Tuner)
{
	struct nim_device *dev;
	static UINT8 nim_dev_idx = 0;
	int hdl = 0;

	if (NULL == ptrQAM_Tuner)
	{
		NIM_PRINTF("DVBC Tuner configuration API structure is NULL!\n");
		return ERR_NO_DEV;
	}
	if (2 <= nim_dev_idx)
	{
		NIM_PRINTF("Can not support more than 2 Tuner!\n");
		return ERR_NO_DEV;
	}
	
       /* Alloc structure space of tuner devive*/
	//dev = (struct nim_device *)dev_alloc(nim_m3200_name[nim_dev_idx], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
	dev = (struct nim_device *)dev_alloc(nim_tda10025_name[nim_dev_idx], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));   
	if (dev == NULL)
	{
		NIM_PRINTF("Error: Alloc nim device error!\n");
		return ERR_NO_MEM;
	}

        
	/* tuner configuration function */
	memcpy((void*)&(nim_tda10025_cfg[nim_dev_idx].tuner_config_data), (void*)&(ptrQAM_Tuner->tuner_config_data), 
				sizeof(struct QAM_TUNER_CONFIG_DATA));
    memcpy((void*)&(nim_tda10025_cfg[nim_dev_idx].tuner_config_ext), (void*)&(ptrQAM_Tuner->tuner_config_ext), 
				sizeof(struct QAM_TUNER_CONFIG_EXT));

	nim_tda10025_cfg[nim_dev_idx].qam_mode=ptrQAM_Tuner->dem_config_advanced.qam_config_advanced;
	nim_tda10025_cfg[nim_dev_idx].tuner_id=ptrQAM_Tuner->tuner_id; //kent
		
	// Oncer 20110816: move from nim_open
	//hdl = open(nim_m3200_devname[nim_dev_idx], O_RDWR);
	hdl = open(nim_tda10025_devname[nim_dev_idx], O_RDWR);	
	if(hdl == 0)
	{
		printf("open NIM handle fail\n");
		return ERR_FAILURE;
	}
    
          
	if(0==ioctl(hdl, ALI_NIM_HARDWARE_INIT_C, &nim_tda10025_cfg[nim_dev_idx]))
	{

		if (dev_register(dev) != SUCCESS)
		{
			NIM_PRINTF("Error: Register nim device error!\n");
			//free(priv);
			dev_free(dev);
			close(hdl);
			return ERR_NO_DEV;
		}
		dev->flags |= HLD_DEV_STATS_RUNNING;
		dev->priv = (void *)hdl;
		NIM_PRINTF("NIM Harware Init Success\n");
	}
	else
	{
		NIM_PRINTF("NIM Hardware Init Failed\n");
		dev_free(dev);
		close(hdl);
		return  ERR_FAILURE;
	}
   
	nim_dev_idx ++;
	return SUCCESS;
}

#ifdef TWO_TUNER_SUPPORT
INT32 nim_cxd2837_attach(struct COFDM_TUNER_CONFIG_API * ptrcofdm_tuner)
{
	struct nim_device *dev;
	UINT8 nim_dev_idx = 0;
	int hdl = 0;

	if (NULL == ptrcofdm_tuner)
	{
		NIM_PRINTF("DVBC Tuner configuration API structure is NULL!\n");
		return ERR_NO_DEV;
	}
	if (2 <= nim_dev_idx)
	{
		NIM_PRINTF("Can not support more than 2 Tuner!\n");
		return ERR_NO_DEV;
	}
    /* Alloc structure space of tuner devive*/
	dev = (struct nim_device *)dev_alloc(nim_cxd2837_name[nim_dev_idx], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
	if (dev == NULL)
	{
		NIM_PRINTF("Error: Alloc nim device error!\n");
		return ERR_NO_MEM;
	}
	/* tuner configuration function */
	memcpy(&nim_cxd2837_cfg[nim_dev_idx].cofdm_data, &ptrcofdm_tuner->cofdm_data, 
				sizeof(struct COFDM_TUNER_CONFIG_DATA));
    memcpy(&nim_cxd2837_cfg[nim_dev_idx].tuner_config, &ptrcofdm_tuner->tuner_config, 
				sizeof(struct COFDM_TUNER_CONFIG_EXT));
	
    memcpy(&nim_cxd2837_cfg[nim_dev_idx].ext_dm_config,&ptrcofdm_tuner->ext_dm_config, 
				sizeof(struct EXT_DM_CONFIG));

    nim_cxd2837_cfg[nim_dev_idx].tuner_id = ptrcofdm_tuner->tuner_id; //kent
    libc_printf("ptrcofdm_tuner->tuner_id:0x%x\n", ptrcofdm_tuner->tuner_id);

	NIM_PRINTF("[%s]line=%d,here!devname=%s\n",__FUNCTION__,__LINE__,nim_cxd2837_devname[nim_dev_idx]);
	// Oncer 20110816: move from nim_open
	hdl = open(nim_cxd2837_devname[nim_dev_idx], O_RDWR);
	if(hdl <= 0)
	{
		NIM_PRINTF("open NIM handle fail\n");
		return ERR_FAILURE;
	}
	libc_printf("ALI_NIM_HARDWARE_INIT_T:0x%x\n", ALI_NIM_HARDWARE_INIT_T);
	//if(SUCCESS == ioctl(hdl, ALI_NIM_HARDWARE_INIT_T, &nim_cxd2837_cfg[nim_dev_idx]))
	if(SUCCESS == ioctl(hdl, ALI_NIM_HARDWARE_INIT_T, &nim_cxd2837_cfg[nim_dev_idx]))
	{
	    NIM_PRINTF("[%s]line=%d,here!\n",__FUNCTION__,__LINE__);
		/* Add this device to queue */
		if (dev_register(dev) != SUCCESS)
		{
			NIM_PRINTF("Error: Register nim device error!\n");
			//free(priv);
			dev_free(dev);
			close(hdl);
			return ERR_NO_DEV;
		}
		dev->flags |= HLD_DEV_STATS_RUNNING;
		dev->priv = (void *)hdl;
		NIM_PRINTF("NIM Harware Init Success\n");
	}
	else
	{
		NIM_PRINTF("NIM Hardware Init Failed\n");
		dev_free(dev);
		close(hdl);
		return  ERR_FAILURE;
	}
    NIM_PRINTF("[%s]line=%d,here!\n",__FUNCTION__,__LINE__);
	nim_dev_idx ++;
	return SUCCESS;
}

INT32 nim_cxd2837_dettach ( struct nim_device *dev)
{
	if(dev->flags & (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING))
	{
		NIM_PRINTF("Error, NIM device still running.\n");
		return !SUCCESS;
	}
	dev_free(dev);
	return SUCCESS;
}

INT32 nim_cxd2838_attach(struct COFDM_TUNER_CONFIG_API * ptrcofdm_tuner)
{
	struct nim_device *dev;
	UINT8 nim_dev_idx = 0;
	int hdl = 0;

	if (NULL == ptrcofdm_tuner)
	{
		NIM_PRINTF("DVBC Tuner configuration API structure is NULL!\n");
		return ERR_NO_DEV;
	}
	if (2 <= nim_dev_idx)
	{
		NIM_PRINTF("Can not support more than 2 Tuner!\n");
		return ERR_NO_DEV;
	}
    /* Alloc structure space of tuner devive*/
	dev = (struct nim_device *)dev_alloc(nim_cxd2838_name[nim_dev_idx], HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
	if (dev == NULL)
	{
		NIM_PRINTF("Error: Alloc nim device error!\n");
		return ERR_NO_MEM;
	}
	/* tuner configuration function */
	memcpy(&nim_cxd2838_cfg[nim_dev_idx].cofdm_data, &ptrcofdm_tuner->cofdm_data, 
				sizeof(struct COFDM_TUNER_CONFIG_DATA));
    memcpy(&nim_cxd2838_cfg[nim_dev_idx].tuner_config, &ptrcofdm_tuner->tuner_config, 
				sizeof(struct COFDM_TUNER_CONFIG_EXT));
	
    memcpy(&nim_cxd2838_cfg[nim_dev_idx].ext_dm_config,&ptrcofdm_tuner->ext_dm_config, 
				sizeof(struct EXT_DM_CONFIG));

    nim_cxd2838_cfg[nim_dev_idx].tuner_id = ptrcofdm_tuner->tuner_id; //kent
    libc_printf("ptrcofdm_tuner->tuner_id:0x%x\n", ptrcofdm_tuner->tuner_id);

	NIM_PRINTF("[%s]line=%d,here!devname=%s\n",__FUNCTION__,__LINE__,nim_cxd2837_devname[nim_dev_idx]);
	// Oncer 20110816: move from nim_open
	hdl = open(nim_cxd2838_devname[nim_dev_idx], O_RDWR);
	if(hdl <= 0)
	{
		NIM_PRINTF("open NIM handle fail\n");
		return ERR_FAILURE;
	}
	libc_printf("ALI_NIM_HARDWARE_INIT_T:0x%x\n", ALI_NIM_HARDWARE_INIT_T);
	if(SUCCESS == ioctl(hdl, ALI_NIM_HARDWARE_INIT_T, &nim_cxd2838_cfg[nim_dev_idx]))
	{
	    NIM_PRINTF("[%s]line=%d,here!\n",__FUNCTION__,__LINE__);
		/* Add this device to queue */
		if (dev_register(dev) != SUCCESS)
		{
			NIM_PRINTF("Error: Register nim device error!\n");
			//free(priv);
			dev_free(dev);
			close(hdl);
			return ERR_NO_DEV;
		}
		dev->flags |= HLD_DEV_STATS_RUNNING;
		dev->priv = (void *)hdl;
		NIM_PRINTF("NIM Harware Init Success\n");
	}
	else
	{
		NIM_PRINTF("NIM Hardware Init Failed\n");
		dev_free(dev);
		close(hdl);
		return  ERR_FAILURE;
	}
    NIM_PRINTF("[%s]line=%d,here!\n",__FUNCTION__,__LINE__);
	nim_dev_idx ++;
	return SUCCESS;
}

INT32 nim_cxd2838_dettach ( struct nim_device *dev)
{
	if(dev->flags & (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING))
	{
		NIM_PRINTF("Error, NIM device still running.\n");
		return !SUCCESS;
	}
	dev_free(dev);
	return SUCCESS;
}


#endif
