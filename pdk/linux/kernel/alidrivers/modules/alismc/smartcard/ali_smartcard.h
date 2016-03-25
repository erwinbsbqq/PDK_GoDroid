/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard.h
 *
 *  Description: Head file of smart card reader
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  0. 
 ****************************************************************************/

#ifndef  __ALI_SMARTCARD_H__
#define  __ALI_SMARTCARD_H__

/* Standard kernel header */
#include <linux/compiler.h>
#include <asm/bug.h>
#include <asm/io.h>
#include <asm/mach-ali/m36_gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
//#include <linux/smp_lock.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>

/* Standard linux kernel header */
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#include <linux/preempt.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

#include <linux/ali_transport.h>
#include <ali_smc_common.h>
#include <ali_soc_common.h>

#include "ali_smartcard_reg.h"
#include "ali_smartcard_binding.h"
#include <alidefinition/adf_basic.h>


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define BOARD_SUPPORT_CLASS_A		(1<<0)
#define BOARD_SUPPORT_CLASS_B		(1<<1)
#define BOARD_SUPPORT_CLASS_C		(1<<2)

#define DFT_WORK_CLK	3600000
#define DFT_WORK_ETU	372

/* It's not real error, but need negative return value */
#define SMC_FAKE_RETURN_VALUE -9999

#define SMC_INVALID_TRANSPORT_ID -1


/*
 *  Structure smc_device, the basic structure of smart card reader device.
 */
struct smc_device
{
    /* Common */
    char   *dev_name;
    UINT32 reg_base;
    UINT32 io_base;
	int    irq;
    int    gpio_irq;
	UINT32 pwm_addr;
	UINT8  pwm_sel_ofst;
	UINT8  pwm_seh_ofst;
	UINT8  pwm_gpio_ofst;
	UINT8  pwm_cfg_ofst;
	UINT8  pwm_frac_ofst;
    irq_handler_t hsr;
    irq_handler_t lsr;
    int    in_use;
    int    dev_id;

    /* Reset param */
    UINT32 rst_addr;
    UINT32 rst_msk;

    /* Communication with APP */
    UINT32 port;
    
	/* Hardware privative structure */
	struct smartcard_private *priv;	
    struct device *smc_device_node;
    struct cdev smc_cdev;
    struct mutex smc_mutex;
    struct mutex smc_iso_mutex;
    spinlock_t smc_spinlock;
    spinlock_t smc_txrx_spinlock;
    spinlock_t smc_status_spinlock;
    spinlock_t smc_irq_spinlock;
    struct mutex smc_irq_mutex;
    wait_queue_head_t smc_wq_head;
    struct semaphore smc_semaphore;
    struct workqueue_struct *smc_notification_workqueue;
    struct work_struct smc_notification_work;
    struct work_struct smc_reset_work;
    struct work_struct smc_atr_config_work;
    unsigned long wq_flags;

    int smc_reset_result;
    int smc_atr_cfg_result;
    int b_reset_over;
    int b_atr_cfg_over;
};

/* Max slot of smart card */
#define SMC_DEV_NUM 2

#define SMC_INVALID_DEV -1

/* Smart card private data */
struct smartcard_private
{
	volatile UINT8 inserted;			/* smart card inserted or not,	1 inserted */
	volatile UINT8 reseted;			    /* smart card has been reset or not, 1 reseted */
	UINT8 inverse_convention;           /* smart card is using direct or inverse convention, 1 inverse 0 direct */
//	UINT8 parity;				        /* 0 none parity, 1 even parity, 2 odd parity */
//	UINT8 reset_on_high;		        /* 1 reset on high, 0 reset on low */
//	UINT8 card_name[32];

    void *atr;
	UINT32 F;
	UINT32 D;
	UINT32 I;
	UINT32 P;
	UINT8 N;
	UINT8 WI;
	UINT8 T;
	UINT8 CWI;
	UINT8 BWI;
	UINT8 error_check_type;
	UINT8 TA2_spec;                     /* for TA(2) bit 5, indicate if to use the default F/D */

	volatile UINT8 isr0_interrupt_status;
	volatile UINT8 isr1_interrupt_status;
	volatile UINT8 trans_trigger;
	UINT32 smc_clock;
	UINT32 smc_etu;
//	UINT32 uart_baudrate;	
//	UINT32 guard_time;	                /* extra guard time */
	UINT32 first_cwt;
	UINT32 cwt;
	volatile UINT8 * smc_tx_buf;
	volatile UINT32 smc_tx_rd;
	volatile UINT32 smc_tx_wr;
	volatile UINT8 * smc_rx_buf;
	volatile UINT8 * smc_rx_tmp_buf;
	volatile UINT32 smc_rx_head;
	volatile UINT32 smc_rx_tail;
    UINT32 got_first_byte :1;
	UINT32 use_default_etu:1;	
	UINT32 use_gpio_cd:1;
	UINT32 parity_disable: 1;
	UINT32 parity_odd: 1;
	UINT32 apd_disable: 1;
	UINT32 warm_reset_enable: 1;
	UINT32 use_gpio_vpp:1;
	UINT32 disable_pps:1;
	UINT32 ts_auto_detect:1;
	UINT32 internal_ctrl_vpp:1;
	UINT32 invert_power: 1;
	UINT32 invert_detect: 1;
	UINT32 class_selection_supported: 1;
	UINT32 board_supported_class: 6;
	UINT32 smc_supported_class: 6;
    UINT32 sys_clk_trigger: 1; 
    UINT32 	en_power_open_drain:1;
	UINT32 	en_clk_open_drain:1;
	UINT32 	en_data_open_drain:1;
	UINT32 	en_rst_open_drain:1;	
	UINT32 	open_drain_supported:1;
    UINT32 reserved : 1;
	UINT32 *init_clk_array;
	UINT32 init_clk_number;
	UINT32 init_clk_idx;
	UINT32 default_etu;
	UINT16 gpio_cd_pol:1;		        /* Polarity of GPIO, 0 or 1 active */
	UINT16 gpio_cd_io:1;			    /* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16 gpio_cd_pos: 14;	
	UINT16 gpio_vpp_pol:1;		        /* Polarity of GPIO, 0 or 1 active */
	UINT16 gpio_vpp_io:1;			    /* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16 gpio_vpp_pos: 14;	
	UINT8  force_tx_rx_triger;
	UINT8  auto_tx_rx_triger;
	UINT8  force_tx_rx_thld;
	UINT8  force_tx_rx_state;
	UINT32 char_frm_dura;		        /*In unit of 1/1000000 second*/
	UINT32 warm_reset;
	UINT32 the_last_send;		        /*for 3602*/
	void   (*class_select)(enum class_selection );
	enum class_selection smc_current_select;
	void   *T1;

    /* Those parameters are bound IC */
    UINT32 scr_sys_clk;
    UINT32 pwm_sys_clk;
    UINT32 smc_chip_id;
    UINT32 smc_chip_version;
    UINT16 smc_tx_fifo_size;
    UINT16 smc_rx_fifo_size;

    UINT32 smc_sys_clk;    
};

enum SMC_STATE_REPORT{
	SMC_INSERTED = 0x1,
	SMC_REMOVED = 0x2,
	SMC_TX_FINISHED = 0x4,
	SMC_RX_FINISHED = 0x8,
	SMC_RX_BYTE_RCV = 0x10
};

int smc_debug(char *fmt, ...);
int smc_error(char *fmt, ...);
void smc_dump(char *s, char *buf, int size);

#endif /*__SMC_UART_H__*/
