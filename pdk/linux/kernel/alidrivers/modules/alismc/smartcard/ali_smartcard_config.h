/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_config.h
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard reader config.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
//#include <linux/smp_lock.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <ali_smc_common.h>


/*
 *  This config parameters should be write to a separated file
 *  which could be copied according to the board selection
 *  when do menuconfig, so did the board class selection function
 */

extern struct smc_dev_cfg config_param_def;

