/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    pan.c
*
*    Description:    This file contains all functions definition
*		             of Front panel driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Apr.21.2003      Justin Wu       Ver 0.1    Create file.
*****************************************************************************/

#include <boot_common.h>
#include "ali_ir_priv.h"
#include <ali_ir.h>


//#include <retcode.h>
//#include <api/libc/alloc.h>
//#include <api/libc/string.h>
//#include <osal/osal.h>
//#include <hld/hld_dev.h>

//#include <hld/pan/pan.h>
//#include <hld/pan/pan_dev.h>

#define SUCCESS         0       /* Success return */
#define OSAL_WAIT_FOREVER_TIME		0xFFFFFFFF


#define PAN_RX_BUFF_SIZE		8

static UINT8 pan_enable_repeat = 1;
static UINT8 pan_rx_buff_head = 0;
static UINT8 pan_rx_buff_tail = 0;
static struct pan_key	g_ali_pan_key;				/* Current input key */
static UINT32 pan_rx_buff[PAN_RX_BUFF_SIZE][sizeof(struct pan_key)];


INT32 pan_buff_clear(void);

/*
 * 	Name		:   pan_open()
 *	Description	:   Open a pan device
 *	Parameter	:	struct pan_device *dev		: Device to be openned
 *	Return		:	INT32 						: Return value
 *
 */
INT32 pan_open()
{
	INT32 result = SUCCESS;
   
	pan_buff_clear();	

	return result;
}

/*
 * 	Name		:   pan_close()
 *	Description	:   Close a pan device
 *	Parameter	:	struct pan_device *dev		: Device to be closed
 *	Return		:	INT32 						: Return value
 *
 */
INT32 pan_close()
{
	INT32 result = SUCCESS;
    

	return result;
}

/*
 * 	Name		:   pan_get_key()
 *	Description	:   Get a input code
 *	Parameter	:	struct pan_device *dev		: Device get code from
 *					UINT32 timeout				: Timeout mode
 *	Return		:	UINT32 						: Return code
 *
 */
struct pan_key * pan_get_key(UINT32 timeout)
{    	
	while (1)
	{
	    if (pan_rx_buff_head != pan_rx_buff_tail)
		{			
			break;
		}
		if (timeout != OSAL_WAIT_FOREVER_TIME)
		{			
			if (timeout-- == 0)
			{				
				return NULL;
			}
		}
		mdelay(1);
	}

	memcpy(&g_ali_pan_key, pan_rx_buff[pan_rx_buff_tail], sizeof(struct pan_key));
	pan_rx_buff_tail++;
    pan_rx_buff_tail %= PAN_RX_BUFF_SIZE;    
   

	return &g_ali_pan_key;
}


/*
 * 	Name		:   pan_buff_clear()
 *	Description	:   Clear key buffer.
 *	Parameter	:
 *	Return		:
 *
 */
INT32 pan_buff_clear()
{	
	pan_rx_buff_head = pan_rx_buff_tail = 0;	
}

/*
 * 	Name		:   pan_buff_queue_tail()
 *	Description	:   Add a code to list tail.
 *	Parameter	:
 *	Return		:
 *
 */
INT32 pan_buff_queue_tail(struct pan_key *key)
{	
	memcpy(pan_rx_buff[pan_rx_buff_head], key, sizeof(struct pan_key));
	pan_rx_buff_head++;
	pan_rx_buff_head %= PAN_RX_BUFF_SIZE;	

	return SUCCESS;
}

void pan_buff_set_repeat(UINT8 enable_repeat)
{
	pan_enable_repeat = enable_repeat;
}

UINT8 pan_buff_get_repeat()
{
	return pan_enable_repeat;
}



