/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2005 Copyright (C)
 *
 *  File: smc.c
 *
 *  Description: This file contains all functions definition
 *		             of smart card reader interface driver.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  0.                 Victor Chen            Ref. code
 *  1. 2005.9.8  Gushun Chen     0.1.000    Initial
 *
 ****************************************************************************/
#include <hld_cfg.h>
#include <adr_retcode.h>
#include <osal/osal.h>
#include <hld/adr_hld_dev.h>
#include <hld/smc/adr_smc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <linux/dvb/ca.h>
#include "smc_priv.h"


static char g_ali_smc_dev_name[HLD_MAX_NAME_SIZE] = "ali_smc_0";
static struct smc_dev_cfg g_ali_smc_config_tmp;
static smc_device_status_e g_ali_smc_device_status = SMC_DEV_DETACH;
static void *g_ali_smc_priv = NULL;					/* pointer to private data */
/* dbg info */
static struct smc_dbg g_ali_smc_dbg = {SMC_DBG_LEVEL_DEFAULT};


#define SMC_PRINTF(fmt, args...)				\
{										\
	if (0 !=  (g_ali_smc_dbg.level & SMC_DBG_LEVEL_HLD))				\
	{									\
		ADR_DBG_PRINT(SMC, fmt, ##args);					\
	}									\
}

#define SMC_ERR_PRINTF(fmt, args...)		\
{										\
	ADR_DBG_PRINT(SMC, fmt, ##args);						\
}


struct smc_dbg *smc_dbg_get_info(struct smc_device *dev)
{	
	if(NULL== dev)
	{						
		SMC_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return NULL;
	}
	
	return &g_ali_smc_dbg;
}

INT32 smc_dbg_set_info(struct smc_device *dev, struct smc_dbg *info)
{
	if(NULL==dev)
	{						
		SMC_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return ERR_PARA;
	}
	
	memcpy(&g_ali_smc_dbg, info, sizeof(g_ali_smc_dbg));	
	return SUCCESS;
}

/*
 * 	Name		:   smc_monitor()
 *	Description	:   monitor notification from driver
 */
static void smc_monitor(UINT32 upara1,UINT32 upara2)
{
    struct smc_device *dev = (struct smc_device *)upara1;
    smc_notification_param_t *p_smc_notification = NULL;
    struct smc_hld_private *priv = NULL;
    

	priv = (struct smc_hld_private *)g_ali_smc_priv;
    for ( ; ; )
    {
        p_smc_notification = (smc_notification_param_t *)rfk_receive_msg(priv->rfk_port);
		if (NULL == p_smc_notification) 
		{
			osal_task_sleep(10);
			continue;
		}
        SMC_PRINTF("HLD SMC: In %s Notification from kernel T %d L %d V %d\n", 
                       __FUNCTION__, 
                       p_smc_notification->smc_msg_tag,
                       p_smc_notification->smc_msg_len,
                       p_smc_notification->smc_msg_buf[0]);

        switch (p_smc_notification->smc_msg_tag)
        {
            case SMC_MSG_CARD_STATUS:
            {
                static int b_last_status = -1;
				/* avoid fast standby,don't throw message from second time */
                /* if (b_last_status == p_smc_notification->smc_msg_buf[0]) break; */
                if (dev->callback) dev->callback(p_smc_notification->smc_msg_buf[0]);
                b_last_status = p_smc_notification->smc_msg_buf[0];
                break;
            }
            default:
                break;
        }
    }
}

static int smc_register_monitor(struct smc_device *dev)
{
    OSAL_T_CTSK t_ctsk;
	struct smc_hld_private *priv = (struct smc_hld_private *)g_ali_smc_priv;
		
    
    t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 15;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.para1 = (UINT32)(dev);
	t_ctsk.para2 = 0;
	t_ctsk.task = (FP)smc_monitor;
    t_ctsk.name[0]  =   'S';
    t_ctsk.name[1]  =   'M';
    t_ctsk.name[2]  =   'C';
	priv->task_id = osal_task_create(&t_ctsk);
	if (OSAL_INVALID_ID == priv->task_id)
	{
		SMC_ERR_PRINTF("HLD SMC: In %s Fatal error Create moniter task fail\n", __FUNCTION__);
		return RET_FAILURE;
	}

	osal_task_sleep(5);

    return SUCCESS;
}

static void smc_unregister_monitor(struct smc_device *dev)
{
	struct smc_hld_private *priv = (struct smc_hld_private *)g_ali_smc_priv;
	
    if (OSAL_INVALID_ID != priv->task_id)
    {
		osal_task_delete(priv->task_id);
        priv->task_id = OSAL_INVALID_ID;
    }
}


INT32 smc_open(struct smc_device *dev, void (*callback)(UINT32 param))
{
	INT32 result = SUCCESS;
	struct smc_hld_private *priv = NULL;
	int smc_hdl = -1;
	UINT8 i = 0;
	UINT8 smc_dev_name[HLD_MAX_NAME_SIZE];

	
	SMC_PRINTF("[ %s %d ]\n", __FUNCTION__, __LINE__);
	if ((SMC_DEV_ATTACH != g_ali_smc_device_status) && (SMC_DEV_CLOSE != g_ali_smc_device_status))
	{				
		SMC_ERR_PRINTF("[ %s %d ], status error, now = %d(want %d)\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status, SMC_DEV_OPEN);		

		return (SMC_DEV_OPEN == g_ali_smc_device_status) ? SUCCESS : ERR_STATUS;		
	}

	if(NULL==dev)
	{						
		SMC_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return ERR_PARA;
	}

	priv = (struct smc_hld_private *)g_ali_smc_priv;	
	
	/* Open this device */
	memset(smc_dev_name, 0x00, sizeof(smc_dev_name));	
	snprintf(smc_dev_name, sizeof(smc_dev_name), "/dev/%s", g_ali_smc_dev_name);
	smc_hdl = open(smc_dev_name, O_RDWR | O_CLOEXEC);	
	if (smc_hdl < 0)
	{
		SMC_ERR_PRINTF("open %s(%s) handle fail\n", smc_dev_name, g_ali_smc_dev_name);
		perror("");
		result = RET_FAILURE;
		goto err0;
	}
	else
	{
		priv->rfk_port = rfk_get_port();
		priv->smc_hdl=smc_hdl;
		if(priv->rfk_port <= 0)
		{
			SMC_ERR_PRINTF("%s : rfk get port fail\n", __FUNCTION__);
			result = RET_FAILURE;
			goto err1;
		}
		else 
		{			
			ioctl(priv->smc_hdl, SMC_CMD_SET_TRANSPORT_ID, &priv->rfk_port);   
		}

		if (0 == g_ali_smc_config_tmp.use_default_cfg)
		{
			/* Set the card parameter here is we have */
			SMC_PRINTF("[ %s %d ]\n", __FUNCTION__, __LINE__);
			if (0 != ioctl(smc_hdl, SMC_CMD_CONFIG, &g_ali_smc_config_tmp))
			{
				SMC_ERR_PRINTF("open SmartCard handle failed\n");
				perror("");
				result=ERR_DEV_ERROR;
				goto err2;
			}
		}
        /* Get the inserted status for application */
		if (callback)
		{
    		dev->callback = callback;
		}    
		
	}
	
   	smc_register_monitor(dev); 	
	g_ali_smc_device_status = SMC_DEV_OPEN;
	SMC_PRINTF("[ %s %d ], g_ali_smc_device_status = %d\n",
		__FUNCTION__, __LINE__, g_ali_smc_device_status);

	
	return SUCCESS;

err2:
	osal_task_delete(priv->task_id);
	priv->task_id = OSAL_INVALID_ID;
	rfk_free_port(priv->rfk_port);
err1:
	close(smc_hdl);
err0:

	SMC_PRINTF("[ %s %d ]\n", __FUNCTION__, __LINE__);
	return result;
}


/*
 * 	Name		:   smc_close()
 *	Description	:   Close a smc device
 *	Parameter	:	struct smc_device *dev		: Device to be closed
 *	Return		:	INT32 						: Return value
 *
 */
INT32 smc_close(struct smc_device *dev)
{
	INT32 result =  SUCCESS;
	struct smc_hld_private *priv = NULL;
	

	SMC_PRINTF("[ %s %d ]\n", __FUNCTION__, __LINE__);
	if (SMC_DEV_OPEN != g_ali_smc_device_status)
	{				
		SMC_ERR_PRINTF("[ %s %d ], status error, now = %d(want %d)\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status, SMC_DEV_CLOSE);		

		return (SMC_DEV_CLOSE == g_ali_smc_device_status) ? SUCCESS : ERR_STATUS;	
	}

	if(NULL== dev)
	{						
		SMC_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return ERR_PARA;
	}

	priv = (struct smc_hld_private *)g_ali_smc_priv;
	ioctl(priv->smc_hdl, SMC_CMD_DECONFIG, NULL);
	close(priv->smc_hdl);
	osal_task_delete(priv->task_id);
	priv->task_id = OSAL_INVALID_ID;
	rfk_free_port(priv->rfk_port);

	/* Update flags */
	g_ali_smc_device_status = SMC_DEV_CLOSE;

	SMC_PRINTF("[ %s %d ]\n", __FUNCTION__, __LINE__);

	return result;
}

/*
 * 	Name		:   smc_card_exist()
 *	Description	:   Reset smart card
 *	Parameter	:	struct smc_device *dev		: Device structuer
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_card_exist(struct smc_device *dev)
{
	struct smc_hld_private *priv = NULL;
	ca_slot_info_t smc_info;
    INT32 b_inserted = FALSE;
    int ret = 0;


	if (SMC_DEV_OPEN != g_ali_smc_device_status)
	{				
		SMC_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status);		

		return ERR_STATUS;			
	}
	
	if (NULL == dev)
	{						
		SMC_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return ERR_PARA;
	}

	priv = (struct smc_hld_private *)g_ali_smc_priv;	
	ret = ioctl(priv->smc_hdl, SMC_CMD_GET_CARD_STATUS, &b_inserted);
    if (ret < 0)
    {
        SMC_ERR_PRINTF("HLD SMC: In %s Warning - device %s card check failure.\n",
                      __FUNCTION__, dev->name);
    }
    

    return b_inserted ? SUCCESS : RET_FAILURE;
}


/*
 * 	Name		:   smc_reset()
 *	Description	:   Reset smart card
 *	Parameter	:	struct smc_device *dev		: Device structuer
 *					UINT8 *buffer				: Buffer pointer
 *					UINT16 *atr_size				: Size of ATR
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_reset(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size)
{
	struct smc_hld_private *priv = NULL;
	ca_msg_t smc_msg;
	INT32 ret = SUCCESS;
	smc_atr_t atr;

	
	SMC_PRINTF("[ %s %d ]\n", __FUNCTION__, __LINE__);
	if (SMC_DEV_OPEN != g_ali_smc_device_status)
	{				
		SMC_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status);		

		return ERR_STATUS;			
	}
	
	if ((NULL== buffer) || (NULL == dev))
	{
		SMC_ERR_PRINTF("[ %s %d ] error:  Invalid parameters!\n", __FUNCTION__, __LINE__);
		return ERR_PARA;
	}

	priv = (struct smc_hld_private *)g_ali_smc_priv;


	/* ioctl will return 0 for success, -1 for failure */
	ret = ioctl(priv->smc_hdl, SMC_CMD_RESET, &atr);
    if (ret < 0)
    {
    	*atr_size = atr.atr_size = 0;
        SMC_ERR_PRINTF("[ %s %d ], Error, %d\n", __FUNCTION__, __LINE__, ret);
		perror("");        
    }

    if (0 != atr.atr_size)
    {
        memcpy(buffer, atr.atr_buf, atr.atr_size);
        *atr_size = atr.atr_size;
    }
    else
    {
        return RET_FAILURE;
    }

	return ret;
}

/*
 * 	Name		:   smc_reset()
 *	Description	:   Reset smart card
 *	Parameter	:	struct smc_device *dev		: Device structuer
 *					UINT8 *buffer				: Buffer pointer
 *					UINT16 *atr_size				: Size of ATR
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_deactive(struct smc_device *dev)
{
	struct smc_hld_private *priv = NULL;	
	
	if (SMC_DEV_OPEN != g_ali_smc_device_status)
	{				
		SMC_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status);		

		return ERR_STATUS;			
	}
	
	if(NULL== dev)
	{						
		SMC_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return ERR_PARA;
	}

	priv = (struct smc_hld_private *)g_ali_smc_priv;
	SMC_PRINTF("[ %s %d ]\n", __FUNCTION__, __LINE__);

	return ioctl(priv->smc_hdl, SMC_CMD_DEACTIVE, NULL);
}


/*
 * 	Name		:   smc_raw_read()
 *	Description	:   Read data from smart card
 *	Parameter	:	struct smc_device *dev			: Device structuer
 *					UINT8 *buffer					: Buffer pointer
 *					INT16 size					: Number of read in data
  *					INT16 *actlen					: Number of actual read in data
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_raw_read(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen)
{
	struct smc_hld_private *priv = NULL;
	ca_msg_t smc_msg;
	INT32 rlt = SUCCESS;
	UINT16 i = 0;		
    int ret = 0;

    *actlen = 0;

	SMC_PRINTF("[ %s %d ], size = %d\n", __FUNCTION__, __LINE__, size);
	if (SMC_DEV_OPEN != g_ali_smc_device_status)
	{				
		SMC_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status);		

		return ERR_STATUS;			
	}
	
	if ((NULL == buffer) || (NULL == dev))
	{
		SMC_ERR_PRINTF("[ %s %d ] error:  Invalid parameters!\n", __FUNCTION__, __LINE__);
		return ERR_PARA;
	}

	if ((UINT16)size > sizeof(smc_msg.msg))
	{
		SMC_ERR_PRINTF("[ %s %d ] error:  Invalid parameters!size = %d\n", __FUNCTION__, __LINE__, size);
		return ERR_PARA;
	}

	priv = (struct smc_hld_private *)g_ali_smc_priv;
	if (-1 == (ret = read(priv->smc_hdl, buffer, size)))
	{
        /* Failure, maybe need to try one more time */
        if (EINTR == errno)
        {
            SMC_PRINTF("HLD SMC: In %s Info - read data is interrupted, "
                          "you need try again.\n", __FUNCTION__);
            ret = read(priv->smc_hdl, buffer, size);
        }
        else if (EAGAIN == errno)
        {
            SMC_PRINTF("HLD SMC: In %s Info - read data is blocked "
                          "for lack of data, try again.\n", __FUNCTION__);
            ret = read(priv->smc_hdl, buffer, size);
        }        
	}

    if (-1 == ret)
    {
        SMC_ERR_PRINTF("HLD SMC: In %s Error - device can't read data.\n", __FUNCTION__);
        return RET_FAILURE;
    }
    else if (0 == ret)
    {
        SMC_ERR_PRINTF("HLD SMC: In %s Error - reach the end of data.\n", __FUNCTION__);
        return RET_FAILURE;
    }

again: /* When reach here, we may also fail, but we have some data
        * For application, this should be notified successful with not enouth data 
        */
    if ((ret < size) && (0 != ret) && (-1 != ret))
    {
        SMC_ERR_PRINTF("HLD SMC: In %s Warning - read data is not "
                      "enough, read the remainant.\n", __FUNCTION__);
        size -= ret;
        *actlen += ret;
        ret = read(priv->smc_hdl, buffer, size);
        /* Allow block here ?? */
        if (dev->blockable) goto again;
    }

    *actlen += ret;
    
	return SUCCESS;
}


/*
 * 	Name		:   smc_raw_write()
 *	Description	:   Write data to smart card
 *	Parameter	:	struct smc_device *dev			: Device structuer
 *					UINT8 *buffer					: Buffer pointer
 *					INT16 size					: Number of write out data
 *					INT16 *actlen					: Number of actual write out data 
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_raw_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen)
{
	struct smc_hld_private *priv = NULL;
	ca_msg_t smc_msg;
	INT32 rlt = SUCCESS;
	UINT16 i = 0;
    int ret = 0;
    /* For smart card write, we want all data to be written one time
     * So, we never care about the remainant part */

    *actlen = 0;


	SMC_PRINTF("[ %s %d ], size = %d\n", __FUNCTION__, __LINE__, size);
	if (SMC_DEV_OPEN != g_ali_smc_device_status)
	{				
		SMC_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status);		

		return ERR_STATUS;			
	}
	
	if ((NULL == buffer) || (NULL == dev))
	{
		SMC_ERR_PRINTF("[ %s %d ] error:  Invalid parameters!\n", __FUNCTION__, __LINE__);
		return ERR_PARA;
	}

	if ((UINT16)size > sizeof(smc_msg.msg))
	{
		SMC_ERR_PRINTF("[ %s %d ] error:  Invalid parameters!size = %d\n", __FUNCTION__, __LINE__, size);
		return ERR_PARA;
	}

	priv = (struct smc_hld_private *)g_ali_smc_priv;
	if (0 !=  (g_ali_smc_dbg.level & SMC_DBG_LEVEL_HLD))
	{
		SMC_PRINTF("W %d bytes : ", size);
		for(i=0; i<size; i++)
		{
			SMC_PRINTF("%02x ", buffer[i]);
		}		
		SMC_PRINTF("\n");
	}

    if (-1 == (ret = write(priv->smc_hdl, buffer, size)))
    {
        /* Failure, maybe need to try one more time */
        if (EINTR == errno)
        {
            SMC_PRINTF("HLD SMC: In %s Info - write data is interrupted, "
                          "you need try again.\n", __FUNCTION__);
            ret = write(priv->smc_hdl, buffer, size);
        }
    }

    if (-1 == ret)
    {
        SMC_ERR_PRINTF("HLD SMC: In %s Error - device can't write data.\n", __FUNCTION__);
        return RET_FAILURE;
    }

    *actlen += ret;

	return SUCCESS;
}


INT32 smc_raw_fifo_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen)
{
	struct smc_hld_private *priv = NULL;
	ca_msg_t smc_msg;
	INT32 rlt = SUCCESS;
    int ret = 0;
    /* For smart card write, we want all data to be written one time
     * So, we never care about the remainant part */

    *actlen = 0;


	SMC_PRINTF("[ %s %d ], size = %d\n", __FUNCTION__, __LINE__, size);
	if (SMC_DEV_OPEN != g_ali_smc_device_status)
	{				
		SMC_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status);		

		return ERR_STATUS;			
	}
	
	if ((NULL == buffer) || (NULL == dev))
	{
		SMC_ERR_PRINTF("[ %s %d ] error:  Invalid parameters!\n", __FUNCTION__, __LINE__);
		return ERR_PARA;
	}

	if ((UINT16)size > sizeof(smc_msg.msg))
	{
		SMC_ERR_PRINTF("[ %s %d ] error:  Invalid parameters!size = %d\n", __FUNCTION__, __LINE__, size);
		return ERR_PARA;
	}

	priv = (struct smc_hld_private *)g_ali_smc_priv;
    if (-1 == (ret = write(priv->smc_hdl, buffer, size)))
    {
        /* Failure, maybe need to try one more time */
        if (EINTR == errno)
        {
            SMC_PRINTF("HLD SMC: In %s Info - write data is interrupted, "
                          "you need try again.\n", __FUNCTION__);
            ret = write(priv->smc_hdl, buffer, size);
        }
    }

    if (-1 == ret)
    {
        SMC_ERR_PRINTF("HLD SMC: In %s Error - device can't write data.\n", __FUNCTION__);
        return RET_FAILURE;
    }

    *actlen += ret;

	return SUCCESS;
}
/*
 * 	Name		:   smc_iso_transfer()
 *	Description	:   Combines the functionality of both write and read.
 *					Implement ISO7816-3 command transfer. 
 *	Parameter	:	struct smc_device *dev		: Device structuer
 *					UINT8 *command				: ISO7816 command buffer pointer
 *					INT16 num_to_write			: Number to write 
 *					UINT8 *response, 				: Response data buffer pointer
 *					INT16 num_to_read			: Number to read
 *					INT16 *actual_size				: actual size got
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_iso_transfer(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read, INT16 *actual_size)
{
	struct smc_hld_private *priv = NULL;
	ca_msg_t smc_msg;	
	UINT16 i = 0;
    smc_iso_transfer_t iso_transfer;
    INT32 ret = SUCCESS;
	
    iso_transfer.actual_size = 0;
    iso_transfer.command = command;
    iso_transfer.num_to_read = num_to_read;
    iso_transfer.num_to_write= num_to_write;
    iso_transfer.response = response;
    iso_transfer.transfer_err = SMART_NO_ERROR;
	
	SMC_PRINTF("[ %s %d ], cmd_len:%d, rd_len: %d\n", __FUNCTION__, __LINE__, num_to_write, num_to_read);
	if (SMC_DEV_OPEN != g_ali_smc_device_status)
	{				
		SMC_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status);		

		return ERR_STATUS;			
	}
	if ((NULL==command) || (NULL == dev))
	{
		SMC_ERR_PRINTF("[ %s %d ] error:  Invalid parameters!\n", __FUNCTION__, __LINE__);
		return ERR_PARA;
	}

	if ((num_to_read > 0) && (NULL == response))
	{
		SMC_ERR_PRINTF("[ %s %d ] error:  Invalid parameters!\n", __FUNCTION__, __LINE__);
		return ERR_PARA;
	}

	if (((UINT16)num_to_read > sizeof(smc_msg.msg)) ||((UINT16)num_to_write > sizeof(smc_msg.msg)))
	{
		SMC_ERR_PRINTF("[ %s %d ] error:  Invalid parameters!num_to_read = %d, num_to_write = %d\n", 
			__FUNCTION__, __LINE__, num_to_read, num_to_write);
		return ERR_PARA;
	}

	priv = (struct smc_hld_private *)g_ali_smc_priv;
	ret = ioctl(priv->smc_hdl, SMC_CMD_ISO_TRANS, &iso_transfer);
    if (ret < 0)
    {
        SMC_ERR_PRINTF("HLD SMC: In %s Warning - iso transfer error\n", __FUNCTION__);
    }
    *actual_size = iso_transfer.actual_size;
    SMC_PRINTF("[ %s %d ],actual_size: %d\n", __FUNCTION__, __LINE__, *actual_size);

    
    return iso_transfer.transfer_err;
}


/*
 * 	Name		:   smc_iso_transfer_t1()
 *	Description	:   Combines the functionality of both write and read.
 *					Implement ISO7816-3 command transfer. 
 *	Parameter	:	struct smc_device *dev		: Device structuer
 *					UINT8 *command				: ISO7816 command buffer pointer
 *					INT16 num_to_write			: Number to write 
 *					UINT8 *response, 				: Response data buffer pointer
 *					INT16 num_to_read			: Number to read
 *					INT32 *actual_size				: Actually returned data size
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_iso_transfer_t1(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read,INT32 *actual_size)
{
	int ret = 0;
	smc_iso_transfer_t iso_transfer;    
    struct smc_hld_private *priv = (struct smc_hld_private *)g_ali_smc_priv;

    iso_transfer.actual_size = 0;
    iso_transfer.command = command;
    iso_transfer.num_to_read = num_to_read;
    iso_transfer.num_to_write= num_to_write;
    iso_transfer.response = response;
    iso_transfer.transfer_err = SMART_NO_ERROR;

	if (SMC_DEV_OPEN != g_ali_smc_device_status)
	{				
		SMC_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status);		

		return ERR_STATUS;			
	}
	
	if (NULL == dev)
	{						
		SMC_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return ERR_PARA;
	}	

	ret = ioctl(priv->smc_hdl, SMC_CMD_ISO_TRANS_T1, &iso_transfer);
    if (ret < 0)
    {
        SMC_ERR_PRINTF("HLD SMC: In %s Warning - iso transfer error\n", __FUNCTION__);
    }

    return iso_transfer.transfer_err;
}


/*
 * 	Name		:   smc_iso_transfer_t14()
 *	Description	:   Combines the functionality of both write and read.
 *					Implement ISO7816-3 command transfer. 
 *	Parameter	:	struct smc_device *dev		: Device structuer
 *					UINT8 *command				: ISO7816 command buffer pointer
 *					INT16 num_to_write			: Number to write 
 *					UINT8 *response, 				: Response data buffer pointer
 *					INT16 num_to_read			: Number to read
 *					INT32 *actual_size				: Actually returned data size
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_iso_transfer_t14(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read,INT32 *actual_size)
{
	smc_iso_transfer_t iso_transfer;
    int ret = 0;
    struct smc_hld_private *priv = (struct smc_hld_private *)g_ali_smc_priv;

    iso_transfer.actual_size = 0;
    iso_transfer.command = command;
    iso_transfer.num_to_read = num_to_read;
    iso_transfer.num_to_write= num_to_write;
    iso_transfer.response = response;
    iso_transfer.transfer_err = SMART_NO_ERROR;

	if (SMC_DEV_OPEN != g_ali_smc_device_status)
	{				
		SMC_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status);		

		return ERR_STATUS;			
	}
	
	if (NULL == dev)
	{						
		SMC_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return ERR_PARA;
	}	

	ret = ioctl(priv->smc_hdl, SMC_CMD_ISO_TRANS_T1, &iso_transfer);
    if (ret < 0)
    {
        SMC_ERR_PRINTF("HLD SMC: In %s Warning - iso transfer error\n", __FUNCTION__);
    }

    return iso_transfer.transfer_err;
}

/*
 * 	Name		:   smc_io_control()
 *	Description	:   Smart card control command
 *	Parameter	:	struct sto_device *dev		: Device
 *					INT32 cmd					: IO command
 *					UINT32 param				: Param
 *	Return		:	INT32 						: Result
 *
 */
INT32 smc_io_control(struct smc_device *dev, INT32 cmd, UINT32 param)
{
	struct smc_hld_private *priv = NULL;
	struct ali_smc_ioctl_command io_param;
	INT32 rlt = SUCCESS;
	UINT32 cmd_param = param;


	SMC_PRINTF("[ %s %d ], cmd = %d, param = %d\n", __FUNCTION__, __LINE__, cmd, (unsigned int)param);
	if (SMC_DEV_OPEN != g_ali_smc_device_status)
	{				
		SMC_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status);		

		return ERR_STATUS;			
	}
	
	if (NULL == dev)
	{						
		SMC_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return ERR_PARA;
	}

	priv = (struct smc_hld_private *)g_ali_smc_priv;	

    switch (cmd)
	{
		case SMC_DRIVER_SET_IO_ONOFF:
			cmd = SMC_CMD_SET_IO_ONOFF;
			break;
		case SMC_DRIVER_CHECK_STATUS:
            cmd = SMC_CMD_CHECK_STATUS;
			break;
		case SMC_DRIVER_SET_WWT:
            cmd = SMC_CMD_SET_WWT;
            cmd_param = (UINT32)&param;
			break;
		case SMC_DRIVER_SET_CWT:
            cmd = SMC_CMD_SET_CWT;
            cmd_param = (UINT32)&param;
			break;
		case SMC_DRIVER_SET_ETU:
            cmd = SMC_CMD_SET_ETU;
            cmd_param = (UINT32)&param;
			break;
		case SMC_DRIVER_GET_F:
            cmd = SMC_CMD_GET_F;
			break;
		case SMC_DRIVER_GET_D:
            cmd = SMC_CMD_GET_D;			
			break;	
		case SMC_DRIVER_GET_ATR_RESULT:
            cmd = SMC_CMD_GET_ATR_RESULT;			
			break;
		case SMC_DRIVER_GET_PROTOCOL:
            cmd = SMC_CMD_GET_PROTOCOL;
			break;
		case SMC_DRIVER_GET_HB:            
            cmd = SMC_CMD_GET_HB;
			break;
		case SMC_DRIVER_SET_WCLK:
            cmd = SMC_CMD_SET_WCLK;
            cmd_param = (UINT32)&param;
			break;
		case SMC_DRIVER_GET_CLASS:
            cmd = SMC_CMD_GET_CLASS;
			break;
		case SMC_DRIVER_SET_CLASS:
            cmd = SMC_CMD_SET_CLASS;
            cmd_param = (UINT32)&param;
			break;			
		case SMC_DRIVER_SEND_PPS:
			cmd = SMC_CMD_SEND_PPS;
            cmd_param = (UINT32)&param;
			break;			
		case SMC_DRIVER_SET_OPEN_DRAIN:		
			cmd = SMC_CMD_SET_OPEN_DRAIN;
            cmd_param = (UINT32)&param;
			break;			
		case SMC_DRIVER_SET_DEBUG_LEVEL:		
			cmd = SMC_CMD_SET_DEBUG_LEVEL;
            cmd_param = (UINT32)&param;
			break;	
		default:
			break;
	}

	return ioctl(priv->smc_hdl, cmd, cmd_param);
}


INT32 smc_t1_transfer(struct smc_device*dev, UINT8 dad, const void *snd_buf, UINT32 snd_len, void *rcv_buf, UINT32 rcv_len)
{
    smc_t1_trans_t p_t1_trans;
    struct smc_hld_private *priv = (struct smc_hld_private *)g_ali_smc_priv;

    p_t1_trans.dad = dad;
    p_t1_trans.rcv_buf = rcv_buf;
    p_t1_trans.rcv_len = rcv_len;
    p_t1_trans.send_buf = (void *)snd_buf;
    p_t1_trans.send_len = snd_len;

	if (SMC_DEV_OPEN != g_ali_smc_device_status)
	{				
		SMC_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status);		

		return ERR_STATUS;			
	}
	
	if (NULL == dev)
	{						
		SMC_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return ERR_PARA;
	}	

	return ioctl(priv->smc_hdl, SMC_CMD_T1_TRANS, &p_t1_trans);
}


INT32 smc_t1_xcv(struct smc_device *dev, UINT8*sblock, UINT32 slen, UINT8 *rblock, UINT32 rmax, UINT32 *ractual)
{
    smc_t1_xcv_t p_t1_xcv;
    struct smc_hld_private *priv = (struct smc_hld_private *)g_ali_smc_priv;

    p_t1_xcv.actual_size = 0;
    p_t1_xcv.rblock = rblock;
    p_t1_xcv.rmax = rmax;
    p_t1_xcv.sblock = sblock;
    p_t1_xcv.slen = slen;

	if (SMC_DEV_OPEN != g_ali_smc_device_status)
	{				
		SMC_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status);		

		return ERR_STATUS;			
	}
	
	if (NULL == dev)
	{						
		SMC_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return ERR_PARA;
	}	

    return ioctl(priv->smc_hdl, SMC_CMD_T1_XCV, &p_t1_xcv);
}


INT32 smc_t1_negociate_ifsd(struct smc_device*dev, UINT32 dad, INT32 ifsd)
{
    smc_t1_nego_ifsd_t p_t1_nego_ifsd;
    struct smc_hld_private *priv = (struct smc_hld_private *)g_ali_smc_priv;

    p_t1_nego_ifsd.dad = dad;
    p_t1_nego_ifsd.ifsd = ifsd;

	if (SMC_DEV_OPEN != g_ali_smc_device_status)
	{				
		SMC_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status);		

		return ERR_STATUS;			
	}
	
	if (NULL == dev)
	{						
		SMC_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return ERR_PARA;
	}
	
    return ioctl(priv->smc_hdl, SMC_CMD_T1_NEGO_IFSD, &p_t1_nego_ifsd);
}


/******************************************************************************************************
 * 	Name		:	smc_dev_attach()
 *	Description	:	Smart card reader init funciton.
 *	Parameter	:	int dev_id		: Index of smart card slot.
 *	Return		:	INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
INT32 smc_dev_attach(int dev_id, struct smc_dev_cfg * config_param)
{
	struct smc_device *dev = NULL;
	struct smartcard_m36_private *tp = NULL;	
	UINT8 dev_num;
	
	
	SMC_PRINTF("[ %s %d ]\n", __FUNCTION__, __LINE__);
	if (SMC_DEV_DETACH != g_ali_smc_device_status)
	{		
		SMC_ERR_PRINTF("[ %s %d ], status error, now = %d(want %d)\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status, SMC_DEV_ATTACH);	

		return (SMC_DEV_ATTACH == g_ali_smc_device_status) ? SUCCESS : ERR_STATUS;
	}

	if (NULL == config_param)
	{
		SMC_ERR_PRINTF("[ %s %d ] error:  Invalid parameters!\n", __FUNCTION__, __LINE__);
		return ERR_PARA;
	}	
    
	g_ali_smc_config_tmp.use_default_cfg = config_param->use_default_cfg;
		
	if(dev_id == 0)
	{
		g_ali_smc_dev_name[strlen(g_ali_smc_dev_name) - 1] = '0';
	}
	else if(dev_id == 1)
	{
		g_ali_smc_dev_name[strlen(g_ali_smc_dev_name) - 1] = '1';	
	}
	else
	{
		return ERR_FAILURE;
	}
		
	SMC_PRINTF("%s\n",g_ali_smc_dev_name);
	#ifdef ADR_IPC_ENABLE	
		//adr_hld_global_semlock();
	#endif	
		
	dev = dev_alloc(g_ali_smc_dev_name, HLD_DEV_TYPE_SMC,sizeof(struct smc_device));
	
	if (dev == NULL)
	{
		SMC_ERR_PRINTF("Error: Alloc smart card reader error!\n");
		#ifdef ADR_IPC_ENABLE	
			//adr_hld_global_semunlock();
		#endif	
		return ERR_NO_MEM;
	}

	dev->blockable = FALSE;

	g_ali_smc_priv=malloc(sizeof(struct smc_hld_private));
	if (NULL == g_ali_smc_priv)
	{
		SMC_ERR_PRINTF("Error: Alloc smart card private memory error!\n");
		dev_free(dev);
		#ifdef ADR_IPC_ENABLE	
			//adr_hld_global_semunlock();
		#endif	
		return ERR_NO_MEM;
	}

	memset(g_ali_smc_priv, 0, sizeof(struct smc_hld_private));

	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS)
	{
		SMC_ERR_PRINTF("Error: Register smart card reader device error!\n");
		free(g_ali_smc_priv);
		dev_free(dev);
		#ifdef ADR_IPC_ENABLE	
			//adr_hld_global_semunlock();
		#endif	
		return ERR_FAILURE;
	}

	#ifdef ADR_IPC_ENABLE	
		//adr_hld_global_semunlock();
	#endif	
	if ((0 == g_ali_smc_config_tmp.use_default_cfg) && (config_param->init_clk_number > 0))
	{
		memcpy(&g_ali_smc_config_tmp, config_param, sizeof(struct smc_dev_cfg));
		g_ali_smc_config_tmp.init_clk_array=malloc(config_param->init_clk_number*sizeof(UINT32));
		if (NULL == g_ali_smc_config_tmp.init_clk_array)
		{
			return ERR_NO_MEM;
		}
		memcpy(g_ali_smc_config_tmp.init_clk_array, config_param->init_clk_array, config_param->init_clk_number*sizeof(UINT32));	
	}
	g_ali_smc_device_status = SMC_DEV_ATTACH;
	SMC_PRINTF("[ %s %d ], smart card %d attach ok\n", __FUNCTION__, __LINE__, dev_id);
	
	return SUCCESS;
}


/******************************************************************************************************
 * 	Name		:	smc_dev_dettach()
 *	Description	:	Smart card reader dettach funciton.
 *	Parameter	:	int dev_id		: Index of smart card slot.
 *	Return		:	INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
INT32 smc_dev_dettach(int dev_id)
{
	struct smc_device *dev = NULL;
	

	SMC_PRINTF("[ %s %d ]\n", __FUNCTION__, __LINE__);
	if ((SMC_DEV_CLOSE != g_ali_smc_device_status) && (SMC_DEV_ATTACH != g_ali_smc_device_status))
	{				
		SMC_ERR_PRINTF("[ %s %d ], status error, now = %d(want %d)\n", __FUNCTION__, __LINE__, 
			g_ali_smc_device_status, SMC_DEV_DETACH);

		return (SMC_DEV_DETACH == g_ali_smc_device_status) ? SUCCESS : ERR_STATUS;	
	}
	
	if ((dev_id != 0) && (dev_id != 1))
	{
		SMC_ERR_PRINTF("Error: Invalid parameters! dev_id = %d\n", dev_id);		
		return ERR_PARA;
	}
	
	dev = (struct smc_device *)dev_get_by_id(HLD_DEV_TYPE_SMC, 0);
	if(NULL != dev)
	{
		free(g_ali_smc_config_tmp.init_clk_array);
		g_ali_smc_config_tmp.init_clk_array = NULL;
		
		free(g_ali_smc_priv);
		g_ali_smc_priv = NULL;
		
		dev_free(dev);
		dev = NULL;
	}
	else
	{
		SMC_ERR_PRINTF("Error: No dev!\n");	
		return ERR_NO_DEV;
	}
	
	SMC_PRINTF("[ %s %d ], smart card %d dettach ok\n", __FUNCTION__, __LINE__, dev_id);
	g_ali_smc_device_status = SMC_DEV_DETACH;

	
	return SUCCESS;	
}


