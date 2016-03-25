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

#include <types.h>
#include <retcode.h>
//corei#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
//#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/smc/smc.h>
#include <misc/rfk.h>

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

//#define SMC_RW_DEBUG
#ifdef SMC_RW_DEBUG
#define SMC_RW_PRINTF	printf
#else
#define SMC_RW_PRINTF
#endif

#define FAILURE ERR_FAILURE

/* Port ID invalid ?? */
#define SMC_PORT_ID_INVALID(dev) \
    ((dev->port < 0) || (dev->port > MAX_RFK_PORT_NUM))

/*
 * 	Name		:   smc_monitor()
 *	Description	:   monitor notification from driver
 */
static void smc_monitor(UINT32 upara1,UINT32 upara2)
{
    struct smc_device *dev = (struct smc_device *)upara1;
    smc_notification_param_t *p_smc_notification = NULL;

    if (SMC_PORT_ID_INVALID(dev))
    {
        SMC_RW_PRINTF("HLD SMC: In %s Fatal error - invalid port\n", __FUNCTION__);
        return;
    }

    for ( ; ; )
    {
        p_smc_notification = (smc_notification_param_t *)rfk_receive_msg(dev->port);
		if (NULL == p_smc_notification) 
		{
			osal_task_sleep(10);
			continue;
		}
        SMC_RW_PRINTF("HLD SMC: In %s Notification from kernel T %d L %d V %d\n", 
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
    
    t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 15;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.para1 = (UINT32)(dev);
	t_ctsk.para2 = 0;
	t_ctsk.task = smc_monitor;
    t_ctsk.name[0]  =   'S';
    t_ctsk.name[1]  =   'M';
    t_ctsk.name[2]  =   'C';
	dev->monitor_task_id = osal_task_create(&t_ctsk);
	if (OSAL_INVALID_ID == dev->monitor_task_id)
	{
		SMC_RW_PRINTF("HLD SMC: In %s Fatal error Create moniter task fail\n", __FUNCTION__);
		return FAILURE;
	}

	osal_task_sleep(5);

    return SUCCESS;
}

static void smc_unregister_monitor(struct smc_device *dev)
{
    if (OSAL_INVALID_ID != dev->monitor_task_id)
    {
		osal_task_delete(dev->monitor_task_id);
        dev->monitor_task_id = OSAL_INVALID_ID;
    }
}

/*
 * 	Name		:   smc_attach()
 *	Description	:   attach a smc device
 *	Parameter	:	int dev_id                  : device ID
 *					int use_default_cfg			: use default config in driver
 *												  if it's set, the cfg could be NULL
 *					struct smc_dev_cfg *cfg		: cfg of the card
 *												  if use_default_cfg is set, it'll be ignored
 *	Return		:	INT32 						: Return value
 *
 */
INT32 smc_attach(int dev_id, int use_default_cfg,
				 struct smc_dev_cfg *cfg)
{
    struct smc_device *dev = NULL;
    char smc_dev_name[32] = "ali_smc_0";

    SMC_RW_PRINTF("HLD SMC: In %s Request attach for device %d.\n", __FUNCTION__, dev_id);
    smc_dev_name[strlen("ali_smc_0") - 1] = dev_id + '0';
    dev = dev_alloc(smc_dev_name, HLD_DEV_TYPE_SMC,sizeof(struct smc_device));

    if (NULL == dev) 
    {
        SMC_RW_PRINTF("HLD SMC: In %s Error - device %s attach failure.\n",
                      __FUNCTION__, smc_dev_name);
        return FAILURE;
    }
    
    dev->base_addr = 0x0;
    dev->flags = 0;
    dev->port = MAX_RFK_PORT_NUM + 1000;
    dev->dev_id = dev_id;
    dev->irq = -1;
    dev->blockable = FALSE;
    dev->monitor_task_id = OSAL_INVALID_ID;

	if (NULL == cfg) 
       {
            dev->cfg.use_default_cfg = 1;
	}
	else 
        {
            dev->cfg.use_default_cfg = use_default_cfg;
	}

	if ((0 == dev->cfg.use_default_cfg) && (cfg->init_clk_number > 0))
	{
		memcpy(&dev->cfg, cfg, sizeof(struct smc_dev_cfg));           

    		dev->cfg.init_clk_array=malloc(dev->cfg.init_clk_number*sizeof(uint32));
    		if (NULL == dev->cfg.init_clk_array)
    		{
    			return ERR_NO_MEM;
    		}
    		memcpy(dev->cfg.init_clk_array, cfg->init_clk_array, cfg->init_clk_number*sizeof(uint32));
	}
    
    /* Add this device to queue */
	if (SUCCESS != dev_register(dev))
	{
		SMC_RW_PRINTF("HLD SMC: In %s Error: Register device error!\n", __FUNCTION__);
		dev_free(dev);
        return FAILURE;
	}

    return SUCCESS;
}

/*
 * 	Name		:   smc_open()
 *	Description	:   Open a smc device
 *	Parameter	:	struct smc_device *dev		: Device to be openned
 *	Return		:	INT32 						: Return value
 *
 */
INT32 smc_open(struct smc_device *dev, void (*callback)(UINT32 param))
{
    INT32 b_inserted = FALSE;
    char smc_dev_name[32] = "/dev/";

    SMC_RW_PRINTF("HLD SMC: In %s Request open for device %d.\n", 
                  __func__, dev->dev_id);

	/* If openned already, exit */
	if (dev->flags & HLD_DEV_STATS_UP)
	{
		SMC_RW_PRINTF("HLD SMC: In %s Warning - device %s openned already!\n", 
                       __FUNCTION__, dev->name);
		return SUCCESS;
	}

    strcat(smc_dev_name, dev->name);
	/* Open this device */
	if ((dev->priv = open(smc_dev_name, O_RDWR)) < 0)
	{
        SMC_RW_PRINTF("HLD SMC: In %s Error - device %s open failed!\n", 
                       __FUNCTION__, dev->name);
        return FAILURE;
	}
    
	/* Set the card parameter here is we have */
	ioctl(dev->priv, SMC_CMD_CONFIG, &dev->cfg);

    /* Get the port ID for communication */
    if ((dev->port = rfk_get_port()) < 0)
    {
        SMC_RW_PRINTF("HLD SMC: In %s Warning - device %s can't get port"
                      "ID, this may lose some real time message.\n", __FUNCTION__);
    }
    else
    {
        ioctl(dev->priv, SMC_CMD_SET_TRANSPORT_ID, &dev->port);   
    }

    /* Get the inserted status for application */
    if (callback)
    {
        ioctl(dev->priv, SMC_CMD_GET_CARD_STATUS, &b_inserted);
        /* 0 no card; 1 card hw OK; 3 card sw OK */
        /* For compatibity of the upper invocation, we do the transferation */
        if (b_inserted) b_inserted = 1;
        else b_inserted = 0;
        callback(b_inserted);
        dev->callback = callback;
    }    

    if (!(SMC_PORT_ID_INVALID(dev)) && \
        (SUCCESS != smc_register_monitor(dev)))
    {
        rfk_free_port(dev->port);
    }
    
	dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

	return SUCCESS;
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

    SMC_RW_PRINTF("HLD SMC: In %s Request close for device %d.\n", 
                   __func__, dev->dev_id);
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		SMC_RW_PRINTF("HLD SMC: In %s Warning - device %s closed already!\n", 
                       __FUNCTION__, dev->name);
		return SUCCESS;
	}

    if (!(SMC_PORT_ID_INVALID(dev)))
    {
        smc_unregister_monitor(dev);
        rfk_free_port(dev->port);
    }

	/* Close device, release configuration resource firstly */
	ioctl(dev->priv, SMC_CMD_DECONFIG, NULL);
	close((int)dev->priv);

	/* Update flags */
	dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

	return result;
}

/*
 * 	Name		:   smc_card_exist()
 *	Description	:   Reset smart card
 *	Parameter	:	struct smc_device *dev		: Device structuer
 *	Return		:	INT32 						: 0: not inserted; 1: HW inserted; 3: SW initialized
 *
 */
INT32 smc_card_exist(struct smc_device *dev)
{
    INT32 b_inserted = FALSE;
    int ret = 0;

    // SMC_RW_PRINTF("HLD SMC: In %s Device name %s\n", __FUNCTION__, dev->name);
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	ret = ioctl(dev->priv, SMC_CMD_GET_CARD_STATUS, &b_inserted);
    if (ret < 0)
    {
        SMC_RW_PRINTF("HLD SMC: In %s Warning - device %s card check failure.\n",
                      __FUNCTION__, dev->name);
    }

    // SMC_RW_PRINTF("HLD SMC: In %s Smart card status 0x%x\n", __FUNCTION__, b_inserted);

    return b_inserted ? SUCCESS : FAILURE;
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
    smc_atr_t atr;
    int ret = SUCCESS;
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

    /* ioctl will return 0 for success, -1 for failure */
	ret = ioctl(dev->priv, SMC_CMD_RESET, &atr);
    if (ret < 0)
    {
        SMC_RW_PRINTF("HLD SMC: In %s Warning - device %s card check failure.\n",
                       __FUNCTION__, dev->name);
        *atr_size = atr.atr_size = 0;
    }

    if (0 != atr.atr_size)
    {
        memcpy(buffer, atr.atr_buf, atr.atr_size);
        *atr_size = atr.atr_size;
    }
    else
        return RET_FAILURE;

    return ret;
}

/*
 * 	Name		:   smc_deactive()
 *	Description	:   Reset smart card
 *	Parameter	:	struct smc_device *dev		: Device structuer
 *					UINT8 *buffer				: Buffer pointer
 *					UINT16 *atr_size				: Size of ATR
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_deactive(struct smc_device *dev)
{
    int ret = SUCCESS;
    
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

    /* For SMC_CMD_DEACTIVE, no parameters need */
	return ioctl(dev->priv, SMC_CMD_DEACTIVE, NULL);
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
    int ret = 0;

    *actlen = 0;
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (-1 == (ret = read(dev->priv, buffer, size)))
	{
        /* Failure, maybe need to try one more time */
        if (EINTR == errno)
        {
            SMC_RW_PRINTF("HLD SMC: In %s Info - read data is interrupted, "
                          "you need try again.\n", __FUNCTION__);
            ret = read(dev->priv, buffer, size);
        }
        else if (EAGAIN == errno)
        {
            SMC_RW_PRINTF("HLD SMC: In %s Info - read data is blocked "
                          "for lack of data, try again.\n", __FUNCTION__);
            ret = read(dev->priv, buffer, size);
        }        
	}

    if (-1 == ret)
    {
        SMC_RW_PRINTF("HLD SMC: In %s Error - device can't read data.\n", __FUNCTION__);
        return FAILURE;
    }
    else if (0 == ret)
    {
        SMC_RW_PRINTF("HLD SMC: In %s Error - reach the end of data.\n", __FUNCTION__);
        return FAILURE;
    }

again: /* When reach here, we may also fail, but we have some data
        * For application, this should be notified successful with not enouth data 
        */
    if ((ret < size) && (0 != ret) && (-1 != ret))
    {
        SMC_RW_PRINTF("HLD SMC: In %s Warning - read data is not "
                      "enough, read the remainant.\n", __FUNCTION__);
        size -= ret;
        *actlen += ret;
        ret = read(dev->priv, buffer, size);
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
    int ret = 0;
    /* For smart card write, we want all data to be written one time
     * So, we never care about the remainant part */

    *actlen = 0;
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

    if (-1 == (ret = write(dev->priv, buffer, size)))
    {
        /* Failure, maybe need to try one more time */
        if (EINTR == errno)
        {
            SMC_RW_PRINTF("HLD SMC: In %s Info - write data is interrupted, "
                          "you need try again.\n", __FUNCTION__);
            ret = write(dev->priv, buffer, size);
        }
    }

    if (-1 == ret)
    {
        SMC_RW_PRINTF("HLD SMC: In %s Error - device can't write data.\n", __FUNCTION__);
        return FAILURE;
    }

    *actlen += ret;

	return SUCCESS;
}

/* Two same functions ???? */
INT32 smc_raw_fifo_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen)
{
    int ret = 0;
    /* For smart card write, we want all data to be written one time
     * So, we never care about the remainant part */

    *actlen = 0;
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

    if (-1 == (ret = write(dev->priv, buffer, size)))
    {
        /* Failure, maybe need to try one more time */
        if (EINTR == errno)
        {
            SMC_RW_PRINTF("HLD SMC: In %s Info - write data is interrupted, "
                          "you need try again.\n", __FUNCTION__);
            ret = write(dev->priv, buffer, size);
        }
    }

    if (-1 == ret)
    {
        SMC_RW_PRINTF("HLD SMC: In %s Error - device can't write data.\n", __FUNCTION__);
        return FAILURE;
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
    smc_iso_transfer_t iso_transfer;
    int ret = 0;

    iso_transfer.actual_size = 0;
    iso_transfer.command = command;
    iso_transfer.num_to_read = num_to_read;
    iso_transfer.num_to_write= num_to_write;
    iso_transfer.response = response;
    iso_transfer.transfer_err = SMART_NO_ERROR;
    
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	ret = ioctl(dev->priv, SMC_CMD_ISO_TRANS, &iso_transfer);
    if (ret < 0)
    {
        SMC_RW_PRINTF("HLD SMC: In %s Warning - iso transfer error\n", __FUNCTION__);
    }
    *actual_size=iso_transfer.actual_size;
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
	smc_iso_transfer_t iso_transfer;
    int ret = 0;

    iso_transfer.actual_size = 0;
    iso_transfer.command = command;
    iso_transfer.num_to_read = num_to_read;
    iso_transfer.num_to_write= num_to_write;
    iso_transfer.response = response;
    iso_transfer.transfer_err = SMART_NO_ERROR;
    
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	ret = ioctl(dev->priv, SMC_CMD_ISO_TRANS_T1, &iso_transfer);
    if (ret < 0)
    {
        SMC_RW_PRINTF("HLD SMC: In %s Warning - iso transfer error\n", __FUNCTION__);
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

    iso_transfer.actual_size = 0;
    iso_transfer.command = command;
    iso_transfer.num_to_read = num_to_read;
    iso_transfer.num_to_write= num_to_write;
    iso_transfer.response = response;
    iso_transfer.transfer_err = SMART_NO_ERROR;
    
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	ret = ioctl(dev->priv, SMC_CMD_ISO_TRANS_T1, &iso_transfer);
    if (ret < 0)
    {
        SMC_RW_PRINTF("HLD SMC: In %s Warning - iso transfer error\n", __FUNCTION__);
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
	UINT32 cmd_param = param;
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

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
            cmd_param = &param;
			break;
		case SMC_DRIVER_SET_CWT:
            cmd = SMC_CMD_SET_CWT;
            cmd_param = &param;
			break;
		case SMC_DRIVER_SET_ETU:
            cmd = SMC_CMD_SET_ETU;
            cmd_param = &param;
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
            /* Need to set HB buffer */
            cmd = SMC_CMD_GET_HB;
			break;
		case SMC_DRIVER_SET_WCLK:
            cmd = SMC_CMD_SET_WCLK;
            cmd_param = &param;
			break;
		case SMC_DRIVER_GET_CLASS:
            cmd = SMC_CMD_GET_CLASS;
			break;
		case SMC_DRIVER_SET_CLASS:
            cmd = SMC_CMD_SET_CLASS;
            cmd_param = &param;
			break;	
		default:
			break;
	}

	return ioctl(dev->priv, cmd, cmd_param);
}

INT32 smc_t1_transfer(struct smc_device*dev, UINT8 dad, const void *snd_buf, UINT32 snd_len, void *rcv_buf, UINT32 rcv_len)
{
    smc_t1_trans_t p_t1_trans;

    p_t1_trans.dad = dad;
    p_t1_trans.rcv_buf = rcv_buf;
    p_t1_trans.rcv_len = rcv_len;
    p_t1_trans.send_buf = snd_buf;
    p_t1_trans.send_len = snd_len;
    
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	return ioctl(dev->priv, SMC_CMD_T1_TRANS, &p_t1_trans);
}

INT32 smc_t1_xcv(struct smc_device *dev, UINT8 *sblock, UINT32 slen, UINT8 *rblock, UINT32 rmax, UINT32 *ractual)
{
    smc_t1_xcv_t p_t1_xcv;

    p_t1_xcv.actual_size = 0;
    p_t1_xcv.rblock = rblock;
    p_t1_xcv.rmax = rmax;
    p_t1_xcv.sblock = sblock;
    p_t1_xcv.slen = slen;
    
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

    return ioctl(dev->priv, SMC_CMD_T1_XCV, &p_t1_xcv);
}

INT32 smc_t1_negociate_ifsd(struct smc_device*dev, UINT32 dad, INT32 ifsd)
{
    smc_t1_nego_ifsd_t p_t1_nego_ifsd;

    p_t1_nego_ifsd.dad = dad;
    p_t1_nego_ifsd.ifsd = ifsd;
    
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

    return ioctl(dev->priv, SMC_CMD_T1_NEGO_IFSD, &p_t1_nego_ifsd);
}

