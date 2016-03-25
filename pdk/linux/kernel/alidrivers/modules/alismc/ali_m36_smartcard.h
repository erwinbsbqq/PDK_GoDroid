/****************************************************************************
*
*  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
*
*  File: smartcard_m3602.h
*
*  Description: Head file of M3602 CI controler
*  History:
*      Date        Author         Version   Comment
*      ====        ======         =======   =======
*  1. July.17.2006   Joey Gao      0.1	   Initial
****************************************************************************/

#ifndef  __ALI_M36_SMARTCARD_M3602_H__
#define  __ALI_M36_SMARTCARD_M3602_H__

#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/poll.h>
//#include <linux/smp_lock.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0))
#include <linux/firmware.h>
#endif
#include <asm/mach-ali/typedef.h>
#include <linux/crc32.h>
#include <linux/cdev.h>
#include <linux/dvb/ca.h>


#define SUCCESS         0       /* Success return */

#define ERR_NO_MEM      -1      /* Not enough memory error */
#define ERR_LONG_PACK   -2      /* Package too long */
#define ERR_RUNT_PACK   -3      /* Package too short */
#define ERR_TX_BUSY     -4      /* TX descriptor full */
#define ERR_DEV_ERROR   -5      /* Device work status error */
#define ERR_DEV_CLASH   -6      /* Device clash for same device in queue */
#define ERR_QUEUE_FULL  -7      /* Queue node count reached the max. val*/
#define ERR_NO_DEV      -8      /* Device not exist on PCI */
#define ERR_FAILURE		-9      /* Common error, operation not success */
/* Compatible with previous written error*/
#define ERR_FAILUE		-9

#define ERR_PARA        -20     /* Parameter is invalid */
#define ERR_ID_FULL     -21     /* No more ID available */
#define ERR_ID_FREE     -22     /* Specified ID isn't allocated yet */

#define ERR_OFF_SCRN    -30     /* Block is out off the screen */
#define ERR_V_OVRLAP    -31     /* Block is overlaped in vertical */
#define ERR_BAD_CLR     -32     /* Invalid Color Mode code */
#define ERR_OFF_BLOCK   -33     /* Bitmap is out off the block */
#define ERR_TIME_OUT    -34     /* Waiting time out */

/* add by Sen */
#define ERR_FAILED		-40
#define ERR_BUSY		-41
#define ERR_ADDRESS		-42

#define OSAL_E_OK					0
#define OSAL_E_FAIL					(-1)
#define OSAL_E_TIMEOUT				(-2)

#define SMC_IO_ON		1
#define SMC_IO_OFF		0

#define SMC_STATUS_OK			0
#define SMC_STATUS_NOT_EXIST	1
#define SMC_STATUS_NOT_RESET	2


//#define DEBUG_SMC_T1
#ifdef DEBUG_SMC_T1
#define T1PRINTF printk
#else
#define T1PRINTF(...)		do{}while(0)
#endif
#define REG_SCR_CTRL		0x00
#define REG_ICCR		0x01
#define REG_FIFO_CTRL		0x02
#define REG_CLK_SEL		0x03
#define REG_IER0		0x04
#define REG_IER1		0x05
#define REG_ISR0		0x06
#define REG_ISR1		0x07
#define REG_ICCSR		0x08
#define REG_PDBR		0x09
#define REG_RBR			0x0A
#define REG_THR			0x0B
#define REG_ETU0		0x0C
#define REG_ETU1		0x0D
#define REG_GTR0		0x0E
#define REG_GTR1		0x0F
#define REG_CBWTR0		0x10
#define REG_CBWTR1		0x11
#define REG_CBWTR2		0x12
#define REG_CBWTR3		0x13
#define REG_RFIFO_CNT		0x14
#define REG_RCVPR		0x15
#define REG_RCNT_ETU		0x16
#define REG_PIN_VALUE		0x18
#define REG_RXTX_PP		0x19
#define REG_GT_CNT		0x1a
#define REG_WT_CNT		0x1c
#define REG_FSM_STATE		0x20
#define REG_COUNT_DLY		0x22
#define REG_CLKL_SEL		0x24
#define REG_CLKH_SEH		0x26
#define REG_RCNT_3ETU		0x28
#define REG_TX_CNT		    0x2a
#define REG_RX_CNT8		    0x2c
#define REG_CLK_FRAC	    0x2d
#define REG_DEV_CTRL	    0x2e
#define REG_CLK_VPP			0x2f
#define REG_VPP_GPIO	    0x30

#define REG_PWML		        0x00
#define REG_PWMH		        0x02
#define REG_PWM_CONFIG		    0x04
#define REG_PWM_FRAC_DIVISION	0x05

#define SMC_RB_ICCR_PRT_EN	0x30
#define SMC_RB_ICCR_CLK		0x04
#define SMC_RB_ICCR_RST		0x02
#define SMC_RB_ICCR_DIO		0x08
#define SMC_RB_ICCR_VCC		0x01
#define SMC_RB_ICCR_OP			0x0e
#define SMC_RB_ICCR_AUTO_PRT	0x10
#define SMC_RB_CTRL_VPP		0x04

#define SMC_SCR_CTRL_OP		0xe0
#define SMC_SCR_CTRL_INVESE	0x08
#define SMC_SCR_CTRL_TRANS	0x04
#define SMC_SCR_CTRL_RECV	0x02

#define SMC_ISR0_FIFO_EMPTY	0x80
#define SMC_ISR0_FIFO_TRANS	0x40
#define SMC_ISR0_FIFO_RECV	0x20
#define SMC_ISR0_TIMEOUT	0x10
#define SMC_ISR0_BYTE_RECV	0x04
#define SMC_ISR0_PE_RECV	0x01
#define SMC_ISR0_PE_TRANS	0x02

#define SMC_FIFO_CTRL_EN	0x80
#define SMC_FIFO_CTRL_TX_OP	0x40
#define SMC_FIFO_CTRL_RX_OP	0x20
#define SMC_FIFO_CTRL_ENABLE	0x80
#define SMC_FIFO_CTRL_TX_INIT	0x40
#define SMC_FIFO_CTRL_RX_INIT	0x20


#define SMC_ISR1_RST_HIGH		0x01
#define SMC_ISR1_RST_LOW		0x02
#define SMC_ISR1_RST_NATR		0x08
#define SMC_ISR1_CARD_INSERT	0x10
#define SMC_ISR1_CARD_REMOVE	0x20
#define SMC_ISR1_COUNT_ST		0x40

#define SMC_IER0_BYTE_RECV_TRIG	0x04
#define SMC_IER0_BYTE_TRANS_TRIG	0x08
#define SMC_IER0_RECV_FIFO_TRIG	0x20
#define SMC_IER0_TRANS_FIFO_TRIG	0x40
#define SMC_IER0_TRANS_FIFO_EMPY	0x80


/* Input / output micro for IO space */
#define INPUT_UINT8(addr)		(__REG8ALI(addr))
#define INPUT_UINT16(addr)		(__REG16ALI(addr))
#define INPUT_UINT32(addr)		(__REG32ALI(addr))
#define OUTPUT_UINT8(addr,val)	(__REG8ALI(addr) = (val))
#define OUTPUT_UINT16(addr,val)	((__REG16ALI(addr)) = (val))
#define OUTPUT_UINT32(addr,val)	((__REG32ALI(addr)) = (val))

#define WRITE_RX_CNT(ioaddr, val)					\
	do												\
	{												\
		OUTPUT_UINT8(ioaddr+REG_RFIFO_CNT, val);	\
		OUTPUT_UINT8(ioaddr+REG_RX_CNT8, 			\
		(INPUT_UINT8(ioaddr+REG_RX_CNT8)&0xfe)|(((val)&0x100)>>8));	\
	}												\
	while(0)
#define READ_RX_CNT(ioaddr)		(INPUT_UINT8(ioaddr+REG_RFIFO_CNT)|((INPUT_UINT8(ioaddr+REG_RX_CNT8)&0x1)<<8))


/////////////////////////////////////////////////////////////////
//activation&deactivation timming: refer to ST8024
////////////////////////////////////////////////////////////////
#define BASIC_T			(26<<6)
#define ATV_VCC2IO		((BASIC_T*5)>>1)
#define ATV_IO2CLK		(5)
#define DATV_RST2CLK	(BASIC_T>>1)
#define DATV_CLK2IO		(BASIC_T>>1)
#define DATV_IO2VCC		(BASIC_T>>1)
/////////////////////////////////////////////////////////////////
//atr
////////////////////////////////////////////////////////////////
/* Paramenters */
#define ATR_MAX_SIZE 		33	/* Maximum size of ATR byte array */
#define ATR_MAX_HISTORICAL	15	/* Maximum number of historical bytes */
#define ATR_MAX_PROTOCOLS	7	/* Maximun number of protocols */
#define ATR_MAX_IB		4	/* Maximum number of interface bytes per protocol */
#define ATR_CONVENTION_DIRECT	0	/* Direct convention */
#define ATR_CONVENTION_INVERSE	1	/* Inverse convention */
#define ATR_PROTOCOL_TYPE_T0	0	/* Protocol type T=0 */
#define ATR_PROTOCOL_TYPE_T1	1	/* Protocol type T=1 */
#define ATR_PROTOCOL_TYPE_T2	2	/* Protocol type T=2 */
#define ATR_PROTOCOL_TYPE_T3	3	/* Protocol type T=3 */
#define ATR_PROTOCOL_TYPE_T14	14	/* Protocol type T=14 */
#define ATR_INTERFACE_BYTE_TA	0	/* Interface byte TAi */
#define ATR_INTERFACE_BYTE_TB	1	/* Interface byte TBi */
#define ATR_INTERFACE_BYTE_TC	2	/* Interface byte TCi */
#define ATR_INTERFACE_BYTE_TD	3	/* Interface byte TDi */
#define ATR_PARAMETER_F		0	/* Parameter F */
#define ATR_PARAMETER_D		1	/* Parameter D */
#define ATR_PARAMETER_I		2	/* Parameter I */
#define ATR_PARAMETER_P		3	/* Parameter P */
#define ATR_PARAMETER_N		4	/* Parameter N */
#define ATR_INTEGER_VALUE_FI	0	/* Integer value FI */
#define ATR_INTEGER_VALUE_DI	1	/* Integer value DI */
#define ATR_INTEGER_VALUE_II	2	/* Integer value II */
#define ATR_INTEGER_VALUE_PI1	3	/* Integer value PI1 */
#define ATR_INTEGER_VALUE_N	4	/* Integer value N */
#define ATR_INTEGER_VALUE_PI2	5	/* Integer value PI2 */

/* Default values for paramenters */
#define ATR_DEFAULT_F	372
#define ATR_DEFAULT_D	1
#define ATR_DEFAULT_I 	50
#define ATR_DEFAULT_N	0
#define ATR_DEFAULT_P	5
#define ATR_DEFAULT_WI	10

#define ATR_DEFAULT_BWI	4	
#define ATR_DEFAULT_CWI	13

#define ATR_DEFAULT_IFSC	32
#define ATR_DEFAULT_IFSD	32
#define ATR_DEFAULT_BGT	22
#define ATR_DEFAULT_CHK	0	//default checksum is LRC

#define DFT_WORK_CLK	3600000//3579545	 
#define DFT_WORK_ETU	372
enum{
	TRANSMIT_MODE = 0,
	RECEIVE_MODE,
};

#if 0//def SMALL_FIFO_3602
#define TX_FIFO_SIZE	8
#define RX_FIFO_SIZE	32
#else
#if 0//(SYS_CHIP_MODULE == ALI_S3602)
#define TX_FIFO_SIZE	8
#define RX_FIFO_SIZE	32
#else
#define TX_FIFO_SIZE	64//32//8
#define RX_FIFO_SIZE	64//32//32
#endif
#endif

#define FIRST_CWT_VAL		1000  //ms
#define CWT_VAL			300 //ms

 /* T=1 protocol constants */
#define T1_I_BLOCK		0x00
#define T1_R_BLOCK		0x80
#define T1_S_BLOCK		0xC0
#define T1_MORE_BLOCKS		0x20
#define T1_BUFFER_SIZE		(3 + 254 + 2)

enum {
	IFD_PROTOCOL_RECV_TIMEOUT = 0x0000,
	IFD_PROTOCOL_T1_BLOCKSIZE,
	IFD_PROTOCOL_T1_CHECKSUM_CRC,
	IFD_PROTOCOL_T1_CHECKSUM_LRC,
	IFD_PROTOCOL_T1_IFSC,
	IFD_PROTOCOL_T1_IFSD,
	IFD_PROTOCOL_T1_STATE,
	IFD_PROTOCOL_T1_MORE
};

enum smc_atr_result
{
	SMC_ATR_NONE = 0,
	SMC_ATR_WRONG,
	SMC_ATR_OK
};

struct ali_smc_dev_config			//Struct for smc driver configure
{
	UINT32	init_clk_trigger : 1;	/*0, use default initial clk 3.579545MHz. 
								1, use configed initial clk.*/
	UINT32	def_etu_trigger : 1;	/*0, use HW detected ETU as initial ETU. 
								1, use configed ETU as initial ETU.*/
	UINT32	sys_clk_trigger : 1; 	/*Currently, useless*/
	UINT32	gpio_cd_trigger : 1;	/*Current down detecting, , while power off, a gpio int will notify
								CPU to do deactivation*/
	UINT32	gpio_cs_trigger : 1;	/*Currently, useless*/
	UINT32    force_tx_rx_trigger: 1; /*Support TX/RX timely switch*/
	UINT32	parity_disable_trigger: 1; /*0, disable parity check while get ATR. 
									1, enable parity check while get ATR.*/
	UINT32	parity_odd_trigger: 1; /*0, use even parity while get ATR. 
									1, use odd parity while get ATR.*/
	UINT32	apd_disable_trigger: 1;/*0, enable auto pull down function while get ATR. 
									1, disable auto pull down while get ATR.*/
	UINT32	type_chk_trigger : 1;		/*0, don't care card type check, 1, check card type
									is A, B or AB type according to interface device setting*/	
	UINT32 	warm_reset_trigger: 1;	/*0, all the reset are cold reset. 1, all the reset are warm 
									reset except the first one.*/	
	UINT32	gpio_vpp_trigger : 1;		/*Use a gpio pin to provide Vpp signal*/
	UINT32 	disable_pps: 1;		
	UINT32	invert_power: 1;
	UINT32	invert_detect: 1;	
	UINT32 	en_power_open_drain:1;
	UINT32 	en_clk_open_drain:1;
	UINT32 	en_data_open_drain:1;
	UINT32 	en_rst_open_drain:1;		
	UINT32	reserved : 13;									
	UINT32	init_clk_number;
	UINT32 *	init_clk_array;
	UINT32	default_etu;
	UINT32	smc_sys_clk;
	UINT16 	gpio_cd_pol:1;		/* Polarity of GPIO, 0 or 1 active */
	UINT16 	gpio_cd_io:1;		/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16 	gpio_cd_pos: 14;	
	UINT16 	gpio_cs_pol:1;		/* Polarity of GPIO, 0 or 1 active */
	UINT16 	gpio_cs_io:1;		/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16 	gpio_cs_pos: 14;	
	UINT8     force_tx_rx_cmd;
	UINT8     force_tx_rx_cmd_len;
	UINT8    	intf_dev_type;
	UINT8    	reserved1;
	UINT16 	gpio_vpp_pol:1;		/* Polarity of GPIO, 0 or 1 active */
	UINT16 	gpio_vpp_io:1;		/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16 	gpio_vpp_pos: 14;	
	UINT32	ext_cfg_tag;
	void * 	ext_cfg_pointer;
};



typedef struct
{
	UINT8 length;
	UINT8 TS;
	UINT8 T0;

	struct
	{
		UINT8 value;
		BOOL present;
	} ib[ATR_MAX_PROTOCOLS][ATR_MAX_IB], TCK;

	UINT8 pn;
	UINT8 hb[ATR_MAX_HISTORICAL];
	UINT8 hbn;
}atr_t;

/* T1 protocol private*/
typedef struct {
	INT32		state;  /*internal state*/

	UINT8		ns;	/* reader side  Send sequence number */
	UINT8		nr;	/* card side  RCV sequence number*/
	UINT32		ifsc;
	UINT32		ifsd;

	UINT8		wtx;		/* block waiting time extention*/
	UINT32		retries;
	INT32		rc_bytes; 	/*checksum bytes, 1 byte for LRC, 2 for CRC*/

	UINT32 		BGT;
	UINT32 		BWT;
	UINT32 		CWT;
	
	UINT32		(*checksum)(UINT8* data, UINT32 len, UINT8* rc);

	INT8			more;	/* more data bit */
	UINT8		previous_block[4];	/* to store the last R-block */
	UINT8		sdata[T1_BUFFER_SIZE];
	UINT8		rdata[T1_BUFFER_SIZE];
} t1_state_t;

struct smc_hb_t		//Struct to retrieve Historical Byte from ATR
{
	UINT8 hb[33];
	UINT8 hbn;
};


struct smartcard_m36_private
{
	UINT32 base_addr;
	//OSAL_ID	smc_sema_id;
	struct mutex smc_mutex;
	
	//OSAL_ID smc_flag_id;
	UINT32 smc_flag_id;
	volatile UINT8 inserted;			//smart card inserted or not,	1 inserted
	volatile UINT8 reseted;			//smart card has been reset or not,	1 reseted
	UINT8 inverse_convention;//smart card is using direct or inverse convention, 1 inverse 0 direct
//	UINT8 parity;				// 0 none parity, 1 even parity, 2 odd parity
//	UINT8 reset_on_high;		// 1 reset on high, 0 reset on low
//	UINT8 card_name[32];

	UINT8 atr[ATR_MAX_SIZE];
	UINT16 atr_size;
	
	atr_t *atr_info;
	enum smc_atr_result atr_rlt;
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
	UINT8 TA2_spec;        //for TA(2) bit 5, indicate if to use the default F/D

	volatile UINT8 isr0_interrupt_status;
	volatile UINT8 isr1_interrupt_status;
	volatile UINT8 trans_trigger;
	UINT32 smc_clock;
	UINT32 smc_etu;
//	UINT32 uart_baudrate;	
//	UINT32 guard_time;	//extra guard time
	UINT32 first_cwt;
	UINT32 cwt;
	volatile UINT8 * smc_tx_buf;
	volatile UINT32 smc_tx_rd;
	volatile UINT32 smc_tx_wr;
	volatile UINT8 * smc_rx_buf;
	volatile UINT8 * smc_rx_tmp_buf;
	volatile UINT8 * smc_rx_tmp_buf_addr;
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
	UINT32 	en_power_open_drain:1;
	UINT32 	en_clk_open_drain:1;
	UINT32 	en_data_open_drain:1;
	UINT32 	en_rst_open_drain:1;	
	UINT32 	open_drain_supported:1;
    	UINT32 reserved : 14;
	UINT32 * init_clk_array;
	UINT32 init_clk_number;
	UINT32 init_clk_idx;
	UINT32 default_etu;
	INT32 hsr;
	UINT16 gpio_cd_pol:1;		/* Polarity of GPIO, 0 or 1 active */
	UINT16 gpio_cd_io:1;			/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16 gpio_cd_pos: 14;	
	UINT16 gpio_vpp_pol:1;		/* Polarity of GPIO, 0 or 1 active */
	UINT16 gpio_vpp_io:1;			/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16 gpio_vpp_pos: 14;	
	UINT8  force_tx_rx_triger;
	UINT8  auto_tx_rx_triger;
	UINT8  force_tx_rx_thld;
	UINT8  force_tx_rx_state;
	UINT32 char_frm_dura;		/*In unit of 1/1000000 second*/
	UINT32 warm_reset;
	UINT32 the_last_send;		/*for 3602*/
	t1_state_t          T1;
};


typedef enum smc_error_e {
	SMART_NO_ERROR,
	SMART_WRONG_ATR,
	SMART_TOO_MANY_RETRIES,
	SMART_OVERRUN_ERROR,
	SMART_FRAME_ERROR,
	SMART_PTS_NOT_SUPPORTED,
	SMART_INVALID_STATUS_BYTES_SEQUENCE,
	SMART_INVALID_CLASS,
	SMART_INVALID_CODE,
	SMART_INCORRECT_REFERENCE,
	SMART_INCORRECT_LENGTH,
	SMART_UNKNOWN_ERROR,
	SMART_NOT_INSERTED,
	SMART_NOT_RESETED,
	SMART_INVALID_PROTOCOL,
	SMART_USER_ABORT,
	SMART_BAD_COMMAND,
	SMART_WRITE_ERROR ,
	SMART_READ_ERROR,
	SMART_NO_ANSWER,
	SMART_PARITY_ERROR,
} smc_error_t;

enum	SMC_STATE_REPORT{
	SMC_INSERTED = 0x1,
	SMC_REMOVED = 0x2,
	SMC_TX_FINISHED = 0x4,
	SMC_RX_FINISHED = 0x8,
	SMC_RX_BYTE_RCV = 0x10
};

enum smc_device_ioctrl_command
{
	SMC_DRIVER_SET_IO_ONOFF = 0,	/*en/dis smart card io*/	
	SMC_DRIVER_SET_ETU,			/*set working etu*/
	SMC_DRIVER_SET_WWT,			/*set block waiting time, in unit of ms*/			
	SMC_DRIVER_SET_GUARDTIME,	/*to do*/
	SMC_DRIVER_SET_BAUDRATE,		/*to do*/
	SMC_DRIVER_CHECK_STATUS,		
	SMC_DRIVER_CLKCHG_SPECIAL,	/*to do*/
	SMC_DRIVER_FORCE_SETTING,	/*to do*/
	SMC_DRIVER_SET_CWT,			/*set character waiting time, in unit of ms*/
	SMC_DRIVER_GET_F,				/*get F factor value*/
	SMC_DRIVER_GET_D,				/*get D factor value*/
	SMC_DRIVER_GET_ATR_RESULT, 	/*check ATR status*/
	SMC_DRIVER_GET_ATR,			/* get ATR */
	SMC_DRIVER_GET_HB	,			/*get History Bytes*/
	SMC_DRIVER_GET_PROTOCOL,		/*get card current protocol*/
	SMC_DRIVER_SET_WCLK,			/*set the working clock of smc, */
									/*the new setting value will be used from */
									/*next time reset*/
	SMC_DRIVER_SEND_PPS,
	SMC_DRIVER_SET_PROTOCOL,	/* to do */
	SMC_DRIVER_SET_OPEN_DRAIN,	/* open drain */
	SMC_DRIVER_SET_DEBUG_LEVEL,	/* debug level */
};


#define SMC_TX_BUF_SIZE 	256
#define SMC_RX_BUF_SIZE 	512


struct ali_smartcard_device
{
	struct cdev cdev;
	struct smartcard_m36_private *priv;
};

#endif /*__ALI_M36_SMARTCARD_M3602_H__*/
