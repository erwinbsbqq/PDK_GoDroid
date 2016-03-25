/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2005 Copyright (C)
 *
 *  File: smc.h
 *
 *  Description: This file contains all functions definition
 *		             of smart card reader interface driver.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  0.                 Victor Chen            Ref. code
 *  1. 2005.9.8  Gushun Chen     0.1.000    Initial
 *  2. 2006.12.5  Victor Chen    0.2.000    Update interface for s3602
 ****************************************************************************/

#ifndef __HLD_SMC_H__
#define __HLD_SMC_H__

#include <types.h>
#include <hld/hld_dev.h>
#include <ali_smc_common.h>
#include <rpc_hld/ali_rpc_hld.h>    //corei

#define BOARD_SUPPORT_CLASS_A		(1<<0)
#define BOARD_SUPPORT_CLASS_B		(1<<1)
#define BOARD_SUPPORT_CLASS_C		(1<<2)

/* I / O operation command */
enum smc_io_operation_command
{
	SMC_DRIVER_SET_IO_ONOFF = 0,	/*en/dis smart card io*/	
	SMC_DRIVER_SET_ETU,			    /*set working etu*/
	SMC_DRIVER_SET_WWT,			    /*set block waiting time, in unit of ms*/			
	SMC_DRIVER_SET_GUARDTIME,	    /*to do*/
	SMC_DRIVER_SET_BAUDRATE,		/*to do*/
	SMC_DRIVER_CHECK_STATUS,		
	SMC_DRIVER_CLKCHG_SPECIAL,	    /*to do*/
	SMC_DRIVER_FORCE_SETTING,	    /*to do*/
	SMC_DRIVER_SET_CWT,			    /*set character waiting time, in unit of ms*/
	SMC_DRIVER_GET_F,				/*get F factor value*/
	SMC_DRIVER_GET_D,				/*get D factor value*/
	SMC_DRIVER_GET_ATR_RESULT, 	    /*check ATR status*/
	SMC_DRIVER_GET_HB	,			/*get History Bytes*/
	SMC_DRIVER_GET_PROTOCOL,		/*get card current protocol*/
	SMC_DRIVER_SET_WCLK,			/*set the working clock of smc, */
									/*the new setting value will be used from */
									/*next time reset*/
	SMC_DRIVER_GET_CLASS,			/*return the currently selected classs*/
	SMC_DRIVER_SET_CLASS			/*setting new class selection if previous select fail*/
};

/*sub command set for SMC_DRIVER_FORCE_SETTING */
#define SMC_FORCE_BASE		0x1
#define SMC_FORCE_TX_RX	(SMC_FORCE_BASE + 1)
/***********************************************/

struct smc_gpio_info		/* Total 1 byte */
{
	UINT8	polar;	/* Polarity of GPIO, 0 or 1  is enable or for smart card */
	UINT8	io;	/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT8	position;	/* GPIO index, upto 64 GPIO */
};

struct smc_hw_info
{
	UINT8 io_type;		/*0 uart io; 1 iso_7816 io. Only can be 0 now*/
	UINT8 uart_id;		/*SCI_FOR_RS232 0, SCI_FOR_MDM 1*/
	UINT8 gpio_control;	/*1 gpio control*/
	UINT8 ext_clock;	/*0 internal clock; 1 external clock*/
	UINT8 shared_uart;	/*0 ; 1 shared with rs232 with uart_switch gpio*/
	UINT8 uart_with_appinit : 1;
	UINT8 reserved : 7;
	
	struct smc_gpio_info prest;
	struct smc_gpio_info power;
	struct smc_gpio_info reset;
	struct smc_gpio_info uart_switch;
	//add for external 6MHz clock
	struct smc_gpio_info clock_switch;
	UINT32 clock;			/*external clock*/
	//add for external 6MHz clock
	UINT32 clock_ext;		/*external clock 2*/
	UINT32 to_for_atr;		/*timeout for atr in ms*/
	UINT32 to_for_cmd;	/*timeout for command in ms*/
};

/*
 *  Structure smc_device, the basic structure between HLD and LLD smart card reader device.
 */

struct smc_device
{
	struct hld_device *next;		/* Next device structure */
	UINT32 type;					/* Interface hardware type */
	INT8 name[HLD_MAX_NAME_SIZE];	/* Device name */

	UINT16 flags;					/* Interface flags, status and ability */
	UINT32 base_addr;			/* Device IO base addr */

	UINT8  irq;					/*interrupt number using for s3602*/

	/* Hardware privative structure */
	void *priv;					/* pointer to private data */

    /* RFK port ID */
    UINT32 port;

    /* If allow block ? */
    UINT8 blockable;

    int dev_id;
    int monitor_task_id;

	void	(*callback)(UINT32 param);
    struct  smc_dev_cfg cfg;
};

#ifdef __cplusplus
extern "C"
{
#endif

INT32 smc_attach(int dev_id, int use_default_cfg,
                 struct smc_dev_cfg *cfg);
INT32 smc_open(struct smc_device *dev, void (*callback)(UINT32 param));
INT32 smc_close(struct smc_device *dev);
INT32 smc_card_exist(struct smc_device *dev);
INT32 smc_reset(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size);
INT32 smc_deactive(struct smc_device *dev);
INT32 smc_raw_read(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen);
INT32 smc_raw_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen);
INT32 smc_raw_fifo_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen);
INT32 smc_iso_transfer(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read, INT16 *actual_size);
INT32 smc_iso_transfer_t1(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read,INT32 *actual_size);
INT32 smc_iso_transfer_t14(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read,INT32 *actual_size);
INT32 smc_io_control(struct smc_device *dev, INT32 cmd, UINT32 param);
INT32 smc_t1_transfer(struct smc_device*dev, UINT8 dad, const void *snd_buf, UINT32 snd_len, void *rcv_buf, UINT32 rcv_len);

#ifdef __cplusplus
}
#endif

#endif /* __HLD_SMC_H__ */

