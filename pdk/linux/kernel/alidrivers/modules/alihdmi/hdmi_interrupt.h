/***************************************************************************************************
*    Ali Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:
*		hdmi_interrupt.h
*
*    Description:
*		This file define private data type for hdmi driver processing.
*
*    History:
*	 	Date           Author        	Version     Reason
*	 	============	=============	=========	=================
*
*************************************************************************************************/
#ifndef	_HDMI_INTERRUPT_H_
#define	_HDMI_INTERRUPT_H_

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>

#if defined(CONFIG_ALI_M39)
#include <video/ali_hdmi.h>
#else
//#include <linux/ali_hdmi.h>
#endif

#include "hdmi_infoframe.h"

irqreturn_t hdmi_interrupt_handler(int irq, void *dev_id);

#endif

