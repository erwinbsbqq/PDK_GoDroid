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

#include <adr_retcode.h>
#include <hld_cfg.h>
#include <osal/osal_int.h>
#include <hld/adr_hld_dev.h>
#include <hld/nim/adr_nim_tuner.h>
#include <hld/nim/adr_nim_dev.h>
#include <hld/nim/adr_nim.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <err/errno.h>

#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>


#if 0
#define NIM_PRINTF(fmt, args...) ADR_DBG_PRINT(NIM, fmt, ##args)
#else
#define NIM_PRINTF(...) do{}while(0)
#endif

#define DVBS_MAX_DEV_NUM 4
#define DVBC_MAX_DEV_NUM 4  //poe add 2013/11/15

static int m_cur_hld = -1;

static int cur_demod_num = 0;

static struct NIM_CHANNEL_CHANGE m_cur_nim_param;
static struct ali_nim_m3501_cfg nim_m3501_cfg;
static struct ali_nim_m3200_cfg nim_m3200_cfg;


typedef struct _nim_dev_name_t
{
	UINT8   dev_name[20];
	UINT8   dev_path[32];
	UINT8   dev_opened;
}NIM_DEV_NAME;

NIM_DEV_NAME g_dev_name[]=
{
	{"NIM_QAM_S3202",     "/dev/ali_m3281_nim0" ,     0},
	{"FULLNIM_TDA10025_0","/dev/ali_tda10025_nim0" ,  0},
	{"FULLNIM_TDA10025_1","/dev/ali_tda10025_nim1" ,  0},
	{"FULLNIM_TDA10025_2","/dev/ali_tda10025_nim2" ,  0},
	{"NIM_QAM_MXL241",    "/dev/ali_mxl241_nim0",     0},
	{"NIM_S3501_0",       "/dev/ali_m3501_nim0",      0},
	{"NIM_S3501_1",       "/dev/ali_m3501_nim1",      0},
    {"NIM_S3501_2",       "/dev/ali_m3501_nim2",      0},		
    {"NIM_S3501_3",       "/dev/ali_m3501_nim3",      0},
	{"NIM_S3503_0",       "/dev/ali_m3503_nim0",      0},	
	{"NIM_S3503_1",       "/dev/ali_m3503_nim1",      0}
		
};



static struct
{	
	UINT32	cache_freq;
	UINT32	cache_symrate;
	UINT8	cache_modulation;
} s3202_lock_info_cache = {0, 0, 0};


NIM_DEV_NAME *nim_get_dev_name(UINT8 *dev_name)
{
	int i =0 ;
	int dev_count = 0;

    dev_count = sizeof(g_dev_name)/sizeof(NIM_DEV_NAME);
	
	for( i =0 ;i< dev_count;i++)
	{
		if(!strcmp((char*)dev_name,(char*)g_dev_name[i].dev_name))
		{
			NIM_PRINTF("find the device:[%s]=[%s]!\n",g_dev_name[i].dev_name,g_dev_name[i].dev_path);
			return &g_dev_name[i];
		}
	}

	NIM_PRINTF("No found device :%s!\n",dev_name);
	return NULL;
}



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
INT32 nim_open(struct nim_device *dev)
{
	INT32 result = SUCCESS;	
	int nim_hdl = 0;
	NIM_DEV_NAME *nim_dev_name =NULL;

    if (NULL == dev)
    {
		NIM_PRINTF("NIM hld device is NULL!\n");
        return RET_FAILURE;
    }

    nim_dev_name = nim_get_dev_name(dev->name);
    if(NULL == nim_dev_name)
    {
		NIM_PRINTF("NIM hld device NAME is NULL!\n");
        return RET_FAILURE;
    }
    if(nim_dev_name->dev_opened>0)
    {
	 	NIM_PRINTF("nim_open: warning - device %s openned already!\n", dev->name);
		return SUCCESS;
	}
	
	nim_hdl = open(nim_dev_name->dev_path, O_RDWR | O_CLOEXEC);
	if (nim_hdl < 0)
	{
		NIM_PRINTF("open NIM(dvbs2) handle fail!\n");
		return RET_FAILURE;
	}
	
	NIM_PRINTF("open NIM(dvbc) success,nim_dev=%s!\n",dev->name);	
	
	if (PROJECT_FE_DVBS2 == dev->dvb_mode)
	{
		if (0 == ioctl(nim_hdl, ALI_NIM_HARDWARE_INIT_S, &nim_m3501_cfg))
		{
			dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
			dev->priv = (void *)nim_hdl;
			m_cur_hld = nim_hdl;
			nim_dev_name->dev_opened =1;
			NIM_PRINTF("open NIM handle success.nim_hdl=%d\n",nim_hdl);
		}
		else
		{
			NIM_PRINTF("NIM(dvbs2) dev hwinit failed!\n");
			close(nim_hdl);
			result = ERR_DEV_ERROR;
		}

	}
	else if (PROJECT_FE_DVBC == dev->dvb_mode)
	{
		if (0 == ioctl(nim_hdl, ALI_NIM_HARDWARE_INIT_C, &nim_m3200_cfg))
		{
			dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
			dev->priv = (void *)nim_hdl;
			m_cur_hld = nim_hdl;
            nim_dev_name->dev_opened =1;
			NIM_PRINTF("open NIM handle success.\n");
		}
		else
		{
			NIM_PRINTF("NIM(dvbc) dev hwinit failed!\n");
			close(nim_hdl);
			result = ERR_DEV_ERROR;
		}

	}
	
	return result;
}

INT32 nim_tunerself_adaption(struct nim_device *dev)
{
    INT32 ret = -1;
	int nim_hdl = 0;
    NIM_DEV_NAME *nim_dev_name =NULL;
	
    if (NULL == dev)
    {
		NIM_PRINTF("NIM hld device is NULL!\n");
        return RET_FAILURE;
    }

    nim_dev_name = nim_get_dev_name(dev->name);
    if(NULL == nim_dev_name)
    {
		NIM_PRINTF("NIM hld device NAME is NULL!\n");
        return RET_FAILURE;
    }

	nim_hdl = open(nim_dev_name->dev_path, O_RDWR | O_CLOEXEC);
	if (nim_hdl < 0)
	{
		NIM_PRINTF("open NIM(dvbs2) handle fail!\n");
		return RET_FAILURE;
	}
	
	if (PROJECT_FE_DVBS2 == dev->dvb_mode)
	{
		ret = ioctl(nim_hdl, ALI_NIM_TUNER_SELT_ADAPTION_S, &nim_m3501_cfg);
		close(nim_hdl);
		return ret;

	}
	else if (PROJECT_FE_DVBC == dev->dvb_mode)
	{
		ret = ioctl(nim_hdl, ALI_NIM_TUNER_SELT_ADAPTION_C, &nim_m3200_cfg);
		close(nim_hdl);
		return ret;

	}
	
	return RET_FAILURE;

}


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
	NIM_DEV_NAME *nim_dev_name =NULL;
	
    if (NULL == dev)
	{
	    NIM_PRINTF("NIM hld device is NULL already!\n");
		return SUCCESS;
    }

	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		NIM_PRINTF("nim_close: warning - device %s closed already!\n", dev->name);
		return SUCCESS;
	}

	if (0 == close((int)dev->priv))
	{
		dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
		dev->priv = NULL;
		
        nim_dev_name = nim_get_dev_name(dev->name);
	    if(NULL == nim_dev_name)
	    {
			NIM_PRINTF("NIM hld device NAME is NULL!\n");
	        return RET_FAILURE;
	    }
		nim_dev_name->dev_opened = 0;

		return SUCCESS;
	}
	else
	{
		return RET_FAILURE;
	}
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
	{
        ioctl(m_cur_hld, ALI_NIM_CHANNEL_CHANGE, &m_cur_nim_param);
	}
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
	INT32 ret;

	ret = RET_SUCCESS;
	//NIM_PRINTF("%s:%d,tm:%d\n", __FUNCTION__, __LINE__, osal_get_tick());
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		//NIM_PRINTF("%s:%d,tm:%d\n", __FUNCTION__, __LINE__, osal_get_tick());
		return ERR_DEV_ERROR;
	}

	switch(cmd)
	{
		case NIM_DRIVER_AUTO_SCAN:          /* Do AutoScan Procedure */
			ret = RET_FAILURE;
			break;
		case NIM_DRIVER_CHANNEL_CHANGE:     /* Do Channel Change */
			{
				m_cur_hld = (int)dev->priv;
				m_cur_nim_param.freq = ((struct NIM_CHANNEL_CHANGE *) (param_list))->freq;
				m_cur_nim_param.sym = ((struct NIM_CHANNEL_CHANGE *) (param_list))->sym;
				m_cur_nim_param.fec = ((struct NIM_CHANNEL_CHANGE *) (param_list))->fec;
				m_cur_nim_param.modulation = ((struct NIM_CHANNEL_CHANGE *) (param_list))->modulation;

				s3202_lock_info_cache.cache_freq = m_cur_nim_param.freq;
				s3202_lock_info_cache.cache_symrate = m_cur_nim_param.sym;
				s3202_lock_info_cache.cache_modulation = m_cur_nim_param.modulation;

				return ioctl((int)dev->priv, ALI_NIM_CHANNEL_CHANGE, &m_cur_nim_param);
			}
		case NIM_DRIVER_QUICK_CHANNEL_CHANGE:     /* Do Channel Change */
			{
				m_cur_hld = (int)dev->priv;
				m_cur_nim_param.freq = ((struct NIM_CHANNEL_CHANGE *) (param_list))->freq;
				m_cur_nim_param.sym = ((struct NIM_CHANNEL_CHANGE *) (param_list))->sym;
				m_cur_nim_param.fec = ((struct NIM_CHANNEL_CHANGE *) (param_list))->fec;
				m_cur_nim_param.modulation = ((struct NIM_CHANNEL_CHANGE *) (param_list))->modulation;

				s3202_lock_info_cache.cache_freq = m_cur_nim_param.freq;
				s3202_lock_info_cache.cache_symrate = m_cur_nim_param.sym;
				s3202_lock_info_cache.cache_modulation = m_cur_nim_param.modulation;
				return ioctl((int)dev->priv, ALI_NIM_CHANNEL_CHANGE, &m_cur_nim_param);
			}
		case NIM_DRIVER_GET_CN_VALUE:
			*(UINT16*)param_list = ioctl((int)dev->priv,ALI_NIM_GET_CN_VALUE, 0);
			ret = RET_SUCCESS;
			break;
		case NIM_DRIVER_GET_RF_LEVEL:
			*(UINT16*)param_list = ioctl((int)dev->priv,ALI_NIM_GET_RF_LEVEL, 0);
			ret = RET_SUCCESS;
			break;
		case NIM_DRIVER_GET_BER_VALUE:
			*(UINT32*)param_list = ioctl((int)dev->priv,ALI_NIM_READ_QPSK_BER, 0);
			ret = RET_SUCCESS;
			break;
		case NIM_DRIVER_CHANNEL_SEARCH: /* Do Channel Search */
			ret = RET_FAILURE;
			break;
		case NIM_DRIVER_GET_ID:
			ret = RET_FAILURE;
			break;
		case NIM_DRIVER_RESET_FSM:
			//NIM_PRINTF("%s:%d,tm:%d\n", __FUNCTION__, __LINE__, osal_get_tick());
			ret = ioctl((int)dev->priv, ALI_NIM_RESET_FSM, 0);
			//NIM_PRINTF("%s:%d,tm:%d\n", __FUNCTION__, __LINE__, osal_get_tick());
			break;			
		case NIM_DRIVER_READ_TUNTYPE:
			ret = ioctl((int)(dev->priv), ALI_NIM_READ_TUNTYPE, 0);
			break;
		default:
			ret = RET_FAILURE;
			//NIM_PRINTF("%s:%d,cmd:%x,tm:%d\n", __FUNCTION__, __LINE__, cmd, osal_get_tick());
			break;
	}

	//NIM_PRINTF("%s:%d,tm:%d\n", __FUNCTION__, __LINE__, osal_get_tick());
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
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }		
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}
#if 0
	if((cmd & NIM_TUNER_COMMAND) && dev->do_ioctl)
        return dev->do_ioctl(dev, cmd, param);

    if (dev->do_ioctl)
    {
		return dev->do_ioctl(dev, cmd, param);
    }
    else
    {
    	return ERR_DEV_ERROR;
    }
#endif
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
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }		
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}
	*lock = ioctl((int)dev->priv, ALI_NIM_GET_LOCK_STATUS, 0);
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
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }		
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (s3202_lock_info_cache.cache_freq != 0)
	{
		*freq = s3202_lock_info_cache.cache_freq;
		//NIM_PRINTF("[CACHE] get cache freq = %d\n", *freq);
	}
	else
	{
	    *freq = ioctl((int)dev->priv, ALI_NIM_READ_FREQ, 0);
		s3202_lock_info_cache.cache_freq = *freq;
		//NIM_PRINTF("[CACHE] syscall get freq = %d\n", *freq);
	}

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
	NIM_PRINTF("%s: freq:%d, sym:%d, fec:%d\n", __FUNCTION__, freq, sym, fec);
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }		
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

    m_cur_hld = (int)dev->priv;
	memset(&m_cur_nim_param, 0, sizeof(struct NIM_CHANNEL_CHANGE));
    m_cur_nim_param.freq = freq;
    m_cur_nim_param.sym = sym;
    m_cur_nim_param.modulation = fec;
	if (PROJECT_FE_DVBS2 == dev->dvb_mode)
	{
		m_cur_nim_param.fec = fec;
	}
	else if (PROJECT_FE_DVBC == dev->dvb_mode)
	{
		m_cur_nim_param.fec = 0xff;
	}
	s3202_lock_info_cache.cache_freq = m_cur_nim_param.freq;
	s3202_lock_info_cache.cache_symrate = m_cur_nim_param.sym;
	s3202_lock_info_cache.cache_modulation = m_cur_nim_param.modulation;

	return ioctl((int)dev->priv, ALI_NIM_CHANNEL_CHANGE, &m_cur_nim_param);
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
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }		
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (PROJECT_FE_DVBS2 == dev->dvb_mode)
	{
		*fec = ioctl((int)dev->priv, ALI_NIM_READ_CODE_RATE, 0);
	}
	else if (PROJECT_FE_DVBC == dev->dvb_mode)
	{
		if (s3202_lock_info_cache.cache_modulation != 0)
		{
			*fec = s3202_lock_info_cache.cache_modulation;
			//NIM_PRINTF("[CACHE] get cache FEC = %d\n", *fec);
		}
		else
		{
			*fec = ioctl((int)dev->priv, ALI_NIM_READ_CODE_RATE, 0);
			s3202_lock_info_cache.cache_modulation = *fec;
			//NIM_PRINTF("[CACHE] syscall get FEC = %d\n", *fec);
		}
	}

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
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }		
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*agc = ioctl((int)dev->priv, ALI_NIM_READ_AGC, 0);

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
	if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}
	
	*snr = ioctl((int)dev->priv, ALI_NIM_READ_SNR, 0);
	
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
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }		
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*sym = ioctl((int)dev->priv, ALI_NIM_READ_SYMBOL_RATE, 0);

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
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }		
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*ber = ioctl((int)dev->priv, ALI_NIM_READ_QPSK_BER, 0);

	return SUCCESS;
}

INT32 nim_get_PER(struct nim_device *dev, UINT32 *per)
{
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }		
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	*per = ioctl((int)dev->priv, ALI_NIM_READ_RSUB, 0);

	return SUCCESS;
}

INT32 nim_get_PER_sum(struct nim_device *dev, UINT32 *per)
{
	INT32 per_sum;
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	per_sum = ioctl((int)dev->priv, ALI_NIM_DRIVER_READ_SUMPER, per);
	*per = per_sum; 

	return SUCCESS;
}


/*****************************************************/
/*               dedicate for DVB-S                  */
/*****************************************************/

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
		return ERR_FAILURE;
#endif
	return ERR_FAILURE;
}

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
	INT32 ret = 0, i = 0;
	struct ali_nim_diseqc_cmd diseqc_cmd;

	NIM_PRINTF("[%s]line=%d,enter,mode=%d!\n", __FUNCTION__, __LINE__,mode);
	
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }		
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	diseqc_cmd.mode=mode;
	for (i=0; i<cnt; i++)
	{
		diseqc_cmd.cmd[i]=cmd[i];
	}
	diseqc_cmd.cmd_size = cnt;
	diseqc_cmd.diseqc_type = 1; //nim_DiSEqC_operate;
		
	ret = ioctl((int)dev->priv, ALI_NIM_DISEQC_OPERATE, &diseqc_cmd);

	if (0 == ret)
	{
		NIM_PRINTF("%s %d success.\n", __FUNCTION__, mode);
		return SUCCESS;
	}
	else
	{
		NIM_PRINTF("%s %d failed!\n", __FUNCTION__, mode);
		return ret;
	}
}

INT32 nim_DiSEqC2X_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt, UINT8 *rt_value, UINT8 *rt_cnt)
{
	INT32 ret = 0;
	INT32 i = 0;
	struct ali_nim_diseqc_cmd diseqc_cmd;
	
	/* If device not running, exit */
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }		
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	diseqc_cmd.mode=mode;
	for (i=0; i<cnt; i++)
	{
		diseqc_cmd.cmd[i]=cmd[i];
	}
	diseqc_cmd.cmd_size = cnt;
	diseqc_cmd.diseqc_type = 2; //nim_DiSEqC2X_operate;
		
	ret = ioctl((int)dev->priv, ALI_NIM_DISEQC_OPERATE, &diseqc_cmd);

	if (0 == ret)
	{
		NIM_PRINTF("%s %d success.\n", __FUNCTION__, mode);
		for (i=0; i<diseqc_cmd.ret_len; i++)
		{
			rt_value[i] = diseqc_cmd.ret_bytes[i];
		}
		*rt_cnt = diseqc_cmd.ret_len;
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
	INT32 ret = 0;
	
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }		
	NIM_PRINTF("%s: dev:%s, nimhandle:%d, polar:%d\n", __FUNCTION__, dev->name, (int)dev->priv, polar);
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		NIM_PRINTF("%s: nim status error: %d\n", __FUNCTION__, dev->flags);
		return ERR_DEV_ERROR;
	}
	ret = ioctl((int)dev->priv, ALI_NIM_SET_POLAR, &polar);
	
	NIM_PRINTF("%s: return %d\n", __FUNCTION__, ret);
	return ret;
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

INT32 nim_m3501_attach(struct QPSK_TUNER_CONFIG_API * ptrQPSK_Tuner,UINT8 *dev_name)
{
    struct nim_device *dev =  NULL;
	
    dev = (struct nim_device *)dev_alloc(dev_name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if (dev == NULL)
    {
        NIM_PRINTF("Error: Alloc nim device error!\n");
        return ERR_NO_MEM;
    }

    nim_m3501_cfg.recv_freq_high  = ptrQPSK_Tuner->config_data.recv_freq_high;
    nim_m3501_cfg.recv_freq_low   = ptrQPSK_Tuner->config_data.recv_freq_low;
    nim_m3501_cfg.qpsk_config     = ptrQPSK_Tuner->config_data.qpsk_config;
    nim_m3501_cfg.demod_i2c_id    = ptrQPSK_Tuner->ext_dm_config.i2c_type_id;
    nim_m3501_cfg.demod_i2c_addr  = ptrQPSK_Tuner->ext_dm_config.i2c_base_addr;
    nim_m3501_cfg.tuner_i2c_id    = ptrQPSK_Tuner->tuner_config.i2c_type_id;
    nim_m3501_cfg.tuner_i2c_addr  = ptrQPSK_Tuner->tuner_config.c_tuner_base_addr;

	nim_m3501_cfg.tuner_id = ptrQPSK_Tuner->tuner_id;
	
    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS )
    {
        NIM_PRINTF("Error: Register nim device error!\n");
        dev_free(dev);
        return ERR_NO_DEV;
    }
	dev->dvb_mode = PROJECT_FE_DVBS2;
    dev->demod_index = ptrQPSK_Tuner->demod_index;
    
    return SUCCESS;
}

INT32 nim_m3501_dettach(struct nim_device *dev)
{
    if(NULL == dev)
    {
		return SUCCESS;
    }	
	
	if (dev->flags & (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING))
	{
		NIM_PRINTF("Error, NIM device still running.\n");
		return !SUCCESS;
	}
	dev_free(dev);
	return SUCCESS;
}


/*****************************************************/
/*               dedicate for DVB-C                  */
/*****************************************************/
INT32 nim_reg_read(struct nim_device *dev, UINT8 RegAddr, UINT8 *pData, UINT8 bLen)
{
	UINT8 reg_rw[16] = {0};
	INT32 ret = 0;
	
	reg_rw[0] = 1; //Read
	reg_rw[1] = RegAddr;
	reg_rw[2] = bLen;
	ret = ioctl((int)dev->priv, ALI_NIM_REG_RW, reg_rw);
	if (SUCCESS == ret)
	{
		memcpy(pData, &(reg_rw[3]), bLen);
	}
	return ret;
}

INT32 nim_reg_write(struct nim_device *dev, UINT8 RegAddr, UINT8 *pData, UINT8 bLen)
{
	UINT8 reg_rw[16] = {0};
	
	reg_rw[0] = 2; //Write
	reg_rw[1] = RegAddr;
	reg_rw[2] = bLen;
	memcpy(&(reg_rw[3]), pData, bLen);
	return ioctl((int)dev->priv, ALI_NIM_REG_RW, reg_rw);
}

/*
 * [NOTE]
 * as nim register RegAddr may be larger than 255, u should use the following set of APIs with suffix _ext
 * */
INT32 nim_reg_read_ext(struct nim_device *dev, UINT32 RegAddr, UINT8 *pData, UINT8 bLen)
{
    struct reg_access_t 
	{
	    UINT32 offset_addr;
		UINT8 reg_rword[16];
	};   
	struct reg_access_t reg_cmd;
	INT32 ret = 0; 
 
	reg_cmd.offset_addr = RegAddr;
	reg_cmd.reg_rword[0] = 1; //Read
	reg_cmd.reg_rword[1] = bLen;
						    
	ret = ioctl((int)dev->priv, ALI_NIM_REG_RW_EXT, &reg_cmd);
	if (SUCCESS == ret) 
	{    
	    memcpy(pData, &(reg_cmd.reg_rword[2]), bLen);
    }    
    return ret; 
}

INT32 nim_reg_write_ext(struct nim_device *dev, UINT32 RegAddr, UINT8 *pData, UINT8 bLen)
{
    struct reg_access_t 
	{
	    UINT32 offset_addr;
		UINT8 reg_rword[16];
    };   
    struct reg_access_t reg_cmd;
			    
    reg_cmd.offset_addr = RegAddr;
    reg_cmd.reg_rword[0] = 2; //Write
    reg_cmd.reg_rword[1] = bLen;
    memcpy(&(reg_cmd.reg_rword[2]), pData, bLen);
							    
    return ioctl((int)dev->priv, ALI_NIM_REG_RW_EXT, &reg_cmd);
}

INT32 nim_quick_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec)
{
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }	
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	m_cur_hld = (int)dev->priv;
	memset(&m_cur_nim_param, 0, sizeof(struct NIM_CHANNEL_CHANGE));
	m_cur_nim_param.freq = freq;
    m_cur_nim_param.sym = sym;
    m_cur_nim_param.fec = 0xff;
    m_cur_nim_param.modulation = fec;

    s3202_lock_info_cache.cache_freq = m_cur_nim_param.freq;
    s3202_lock_info_cache.cache_symrate = m_cur_nim_param.sym;
    s3202_lock_info_cache.cache_modulation = m_cur_nim_param.modulation;

	return ioctl((int)dev->priv, ALI_NIM_CHANNEL_CHANGE, &m_cur_nim_param);
}


INT32 nim_tuner_isenter_standby(struct nim_device *dev, UINT32 standby)
{
	if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }	
		
	if (standby)
	{
		return ioctl((int)dev->priv, ALI_NIM_TUNER_STANDBY, 0);
	}	

	return ioctl((int)dev->priv, ALI_NIM_TUNER_ACTIVE, 0);
}

INT32 nim_try_qammode(struct nim_device *dev, UINT32 *pmode)
{
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }	
	
	/* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{   
        return ERR_DEV_ERROR;
	}
	return ioctl((int)dev->priv, ALI_NIM_DRIVER_SET_MODE, pmode);
}

INT32 nim_s3202_attach(struct QAM_TUNER_CONFIG_API * ptrQAM_Tuner,UINT8 *dev_name)
{
    int demod_index;
	struct nim_device *dev = NULL;

    if(NULL == dev_name)
    {
		NIM_PRINTF("Error: dev_name is null!\n");
		return  ERR_NO_DEV;
    }

	/* tuner configuration function */
    memcpy((void*)&(nim_m3200_cfg.tuner_config_data), (void*)&(ptrQAM_Tuner->tuner_config_data), sizeof(struct QAM_TUNER_CONFIG_DATA));
    memcpy((void*)&(nim_m3200_cfg.tuner_config_ext), (void*)&(ptrQAM_Tuner->tuner_config_ext), sizeof(struct QAM_TUNER_CONFIG_EXT));
	memcpy((void*)&(nim_m3200_cfg.ext_dem_config), (void*)&(ptrQAM_Tuner->ext_dem_config), sizeof(struct EXT_DM_CONFIG));
    nim_m3200_cfg.qam_mode = ptrQAM_Tuner->qam_mode;
    demod_index            = ptrQAM_Tuner->demod_index;
	
	nim_m3200_cfg.tuner_id = ptrQAM_Tuner->tuner_id;

	
	dev = (struct nim_device *)dev_get_by_name(dev_name);  //nim_dvbc_local_name[demod_index]
	if (dev)
	{
		NIM_PRINTF("Note: nim device named %s has already exist!\n", dev_name);
        return SUCCESS;
	}

    /* Alloc structure space of tuner devive*/
	dev = (struct nim_device *)dev_alloc(dev_name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
	if (dev == NULL)
	{
		NIM_PRINTF("Error: Alloc nim device error!\n");
		return ERR_NO_MEM;
	}

	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS)
	{
		NIM_PRINTF("Error: Register nim device error!\n");
		//FREE(priv);
		dev_free(dev);
		return ERR_NO_DEV;
	}
	dev->dvb_mode = PROJECT_FE_DVBC;
    dev->demod_index = demod_index;

    tunerdbg_module_register();

	return SUCCESS;
}

INT32 nim_s3202_dettach(struct nim_device *dev)
{
	if (NULL == dev)
	{
	    NIM_PRINTF("NIM hld device is NULL already!\n");
	    return SUCCESS;
	}

	if (dev->flags & (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING))
	{
		NIM_PRINTF("Error, NIM device still running.\n");
		return !SUCCESS;
	}
	dev_free(dev);
	return SUCCESS;
}


///////////////////////////////////////////////////////////////////////
//                          ;-)                                      //
///////////////////////////////////////////////////////////////////////
/*---- ADC2DRAM func for DVBC ----*/
/*#ifdef ADC2DRAM_ENABLE*/
INT32 nim_adcdma_start(struct nim_device *dev, UINT8 *pData)
{
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }	
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	return ioctl((int)dev->priv, ALI_NIM_ADC2MEM_START, pData);
}

INT32 nim_adcdma_stop(struct nim_device *dev)
{
    if(NULL == dev)
    {
		return ERR_DEV_ERROR;
    }	
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	return ioctl((int)dev->priv, ALI_NIM_ADC2MEM_STOP, 0);
}

#include <errno.h>
INT32 nim_adcdma_dump_RF_data(struct nim_device *dev)
{
	UINT8 *tmp_buf = NULL;
	UINT32 buf_cur;
	UINT16 tmp[8];
	UINT8 convert_buf[10];
	UINT8 *tmp_txt_buf = NULL;
	UINT32 txt_buf_cur, tmp_buf_cur;
	UINT8 i, j;
    char * adc_dump_file_name = NULL;
	const char* dump_file_name = "/adc_dump_";
	const char* dump_file_suffix = ".dat";
	const char * dev_mnt_array[][5] = {
		{ "/mnt/sda", "/mnt/sda1", "/mnt/sda2", "/mnt/sda3", "/mnt/sda4" },
		{ "/mnt/sdb", "/mnt/sdb1", "/mnt/sdb2", "/mnt/sdb3", "/mnt/sdb4" },
		{ "/mnt/sdc", "/mnt/sdc1", "/mnt/sdc2", "/mnt/sdc3", "/mnt/sdc4" },
	};
	static UINT8 file_idx = 0;
	static UINT32 DUMP_LEN = 0x2000000;    //32M
	FILE *data_file = NULL;
	int ret = 0;

	adc_dump_file_name = (char *)malloc(strlen("/mnt/sda") + 1 + strlen(dump_file_name) + 1 + strlen(dump_file_suffix) + 1);
	if (NULL == adc_dump_file_name)
	{
		return ERR_NO_MEM;
	}	
	memset(adc_dump_file_name, 0, sizeof(adc_dump_file_name));

	for (i = 0; i < sizeof(dev_mnt_array)/sizeof(dev_mnt_array[0]); i++)
	{
		for (j = 1; j < 5; j++)
		{
			if (0 == access(dev_mnt_array[i][j], W_OK))
			{
				NIM_PRINTF("Directory \"%s\" mounted.\n", dev_mnt_array[i][j]);
				break;
			}
		}
		
		if (j < 5)
			break;
		else if (0 == access(dev_mnt_array[i][0], W_OK))
		{
			j = 0;
			break;
		}
	}

	if (5 == j && 3 == i)
	{
		NIM_PRINTF("No devices mounted or files unwritable.\n");
		ret = -__LINE__;
		goto exit;
	}

	sprintf(adc_dump_file_name, "%s%s%u%s", dev_mnt_array[i][j], dump_file_name, file_idx, dump_file_suffix);
	file_idx++;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		ret = -__LINE__;
		goto exit;
	}
	tmp_buf = malloc(8*1024);
    if (NULL == tmp_buf)
    {
		NIM_PRINTF("tmp_buf malloc failed.\n");
		ret = -__LINE__;
		goto exit;
	}
	
	tmp_txt_buf = malloc(1024);
    if (NULL == tmp_txt_buf)
    {
		NIM_PRINTF(tmp_txt_buf "malloc failed.\n");
		ret = -__LINE__;
		goto exit;
	}
	
	data_file = fopen(adc_dump_file_name, "wb+");
	if (data_file == NULL)
	{
		perror("create data_file fail\n");
		if (EIO == errno)
		{
			NIM_PRINTF("The device maybe broken! Please contact your manufacturer;-)\n");
		}	
		ret = ERR_DEV_ERROR;
		goto exit;
	}
	
	NIM_PRINTF("%s : data_file %s\n", __FUNCTION__, adc_dump_file_name);

	ioctl((int)dev->priv, ALI_NIM_ADC2MEM_SEEK_SET, 0);

	buf_cur = 0;

	NIM_PRINTF("%s : temp_buf: 0x%08x\n", __FUNCTION__, tmp_buf);

	while (buf_cur</*0x3000000*/DUMP_LEN)
	{
		ioctl((int)dev->priv, ALI_NIM_ADC2MEM_READ_8K, tmp_buf);
		buf_cur += 0x2000;

		//NIM_PRINTF("\t write: 0x%08x to file.\n", buf_cur);

#if 1
		fwrite(tmp_buf, 0x2000, 1, data_file);
		fflush(data_file);
#else
		txt_buf_cur = 0;
		tmp_buf_cur = 0;
		while (tmp_buf_cur<0x2000)
		{
			UINT8 j;
			memcpy(convert_buf, tmp_buf+tmp_buf_cur, 10);
			tmp_buf_cur+=10;

			j = 0xff;
			for (i=0; i<80; i+=8)
			{
				if (j != (UINT8)(i/10))
				{
					j = (i/10);
					tmp[j] = convert_buf[i/8];
				}
			}
		}
#endif
	}

	fsync(fileno(data_file));

exit:
	if (NULL != data_file)
	{
	    fclose(data_file);
	}

	if (NULL != tmp_buf)
	{
	    free(tmp_buf);
	}

	if (NULL != tmp_txt_buf)
	{
	    free(tmp_txt_buf);
	}

	if (NULL != adc_dump_file_name)
	{
	    free(adc_dump_file_name);
	}
	
	NIM_PRINTF("%s : dump data to file finished.ret:%d\n", __FUNCTION__, ret);

	return(ret);
}
/*#endif*/

