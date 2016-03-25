/*****************************************************************************
*    Copyright (C)2003 Ali Corporation. All Rights Reserved.
*
*    File:    nim_s3503_DVBS.h
*
*    Description:    Header file in LLD.
*    History:
*   		Date			Athor   	 	Version		  				Reason
*	============	=======	===============	=================
*   	1.  08/29/2012  	Russell		 	Ver 0.1			 Create file for S3503 DVBS2 project
*
*****************************************************************************/

#ifndef __LINUX_NIM_DEVICE_H__
#define __LINUX_NIM_DEVICE_H__





struct nim_device
{
    struct cdev cdev;
    void *priv;

    struct workqueue_struct 		*workqueue;
    struct work_struct 				work;
    struct workqueue_struct 		*autoscan_work_queue;
    struct work_struct				as_work;
};



#endif	// __LLD_NIM_S3501_H__ */


