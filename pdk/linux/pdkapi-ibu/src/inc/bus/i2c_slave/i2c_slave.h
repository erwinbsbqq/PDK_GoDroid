/****************************************************************************
*
*  ALi (Zhuhai) Corporation, All Rights Reserved. 2007 Copyright (C)
*
*  File: i2c_slave.h
*
*  Description: Head file of I2C SLave driver.
*              
*  History:
*      Date        	    Author         	Version   	Comment
*      ====        	    ======         	=======   	=======
*  	1.  2007.02.26      Victor Chen         0.1.000         Initial
*
****************************************************************************/

#ifndef	__LLD_I2C_SLAVE_H__
#define __LLD_I2C_SLAVE_H__

#include <types.h>

#define I2C_SLAVE_BUF_LEN	256
#define I2C_SLAVE_FIFO_LEN	16

struct I2c_master_data
{
	UINT8 *buffer;
	UINT8 offset;
	UINT8 length;
	UINT8 active;
};

typedef void (*SCB_SLAVE_NOTIFY)(struct I2c_master_data* master_data);

INT32 i2c_slave_attach(int id, UINT32 slave_addr, SCB_SLAVE_NOTIFY callback);
INT32 i2c_slave_write(struct I2c_master_data *master_data_p);

#endif
