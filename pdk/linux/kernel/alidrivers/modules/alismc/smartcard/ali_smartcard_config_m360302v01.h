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

/*
 *  This config parameters should be write to a separated file
 *  which could be copied according to the board selection
 *  when do menuconfig, so did the board class selection function
 */

#define SMC_GPIO_BOARD_CLASS_SELECTION 73
#define SMC_GPIO_BOARD_CLASS_VOLTAGE   94

static inline void smc_config_board_class_select(enum class_selection smc_class)
{
    int val = 1;

    if (SMC_CLASS_B_SELECT == smc_class)
        val = 0;
    ali_gpio_direction_output(SMC_GPIO_BOARD_CLASS_VOLTAGE, val);
}

/* Smartcard device attach parameters */
UINT32 init_clk = 3600000;
static struct smc_dev_cfg config_param = {
    .init_clk_trigger = 1,
	.init_clk_number = 1,
	.force_tx_rx_trigger = 1,
	.def_etu_trigger = 1,
	.default_etu = 372,
    .warm_reset_trigger = 1,
	.force_tx_rx_cmd = 0xdd,  
	.force_tx_rx_cmd_len = 5, 
	.invert_detect=0,
	.init_clk_array = &init_clk,
	.class_selection_supported = 1,
	.board_supported_class = BOARD_SUPPORT_CLASS_A | BOARD_SUPPORT_CLASS_B,
	.class_select = smc_config_board_class_select,
};
