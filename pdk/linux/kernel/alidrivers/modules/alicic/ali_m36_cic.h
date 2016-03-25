/****************************************************************************
*
*  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
*
*  File: cic_m3602.h
*
*  Description: Head file of M3602 CI controler
*  History:
*      Date        Author         Version   Comment
*      ====        ======         =======   =======
*  1. July.17.2006   Joey Gao      0.1	   Initial
****************************************************************************/

#ifndef  __CIC_M3602_H__
#define  __CIC_M3602_H__

#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/poll.h>
//#include <linux/byteorder/swabb.h>
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

#if 0
#define PRINTK_INFO(x...) printk(KERN_INFO x)
#else
#define PRINTK_INFO(x...) do{}while(0)
#endif

#define CIC_PRINTF PRINTK_INFO
/*
typedef char INT8;
typedef unsigned char UINT8;
typedef short INT16;
typedef unsigned short UINT16;
typedef int INT32;
typedef unsigned int UINT32;
*/

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




/* For FPGA PCI testing board */
#define M3602CIC_VID			0x8866
#define M3602CIC_DID			0x0101

/* M3602 register define */
enum M3602CIC_registers_address {
	R_TSCR0		= 0x0000,		/* TS Control Register 0 */
	R_TSCR1		= 0x0008,		/* TS Control Register 1 */
	R_CISEL		= 0x000c,			/*for CI sel, byte opration mode.*/
	R_CRVSR		= 0x0100,		/* Chip Revision Register */
	R_IFXSR		= 0x0101,		/* Interface Status Register */
	R_PWRCR		= 0x0102,		/* Power Control Register */
	R_IGCR		= 0x0103,		/* Interrupt and General Control Register */
	R_CSCR		= 0x0104,		/* Card Status Change Register */
	R_MICR		= 0x0105,		/* Management Interrupt Config Register */
	R_MER		= 0x0106,		/* Mapping Enable Register */
	R_IOWR		= 0x0107,		/* IO Windows Control Register */
	R_IOMSAR0	= 0x0108,		/* IO Map 0 Start Address Register */
	R_IOMEAR0	= 0x010a,		/* IO Map 0 End Address Register */
	R_IOMSAR1	= 0x010c,		/* IO Map 1 Start Address Register */
	R_IOMEAR1	= 0x010e,		/* IO Map 1 End Address Register */
	R_MMSAR0	= 0x0110,		/* Memory Map 0 Start Address Register */
	R_MMEAR0	= 0x0112,		/* Memory Map 0 End Address Register */
	R_MMOAR0	= 0x0114,		/* Memory Map 0 Offset Address Register */
	R_MISCCR	= 0x0116,		/* Misc Control Register */
	R_MMSAR1	= 0x0118,		/* Memory Map 1 Start Address Register */
	R_MMEAR1	= 0x011a,		/* Memory Map 1 End Address Register */
	R_MMOAR1	= 0x011c,		/* Memory Map 1 Offset Address Register */
	R_MMSAR2	= 0x0120,		/* Memory Map 2 Start Address Register */
	R_MMEAR2	= 0x0122,		/* Memory Map 2 End Address Register */
	R_MMOAR2	= 0x0124,		/* Memory Map 2 Offset Address Register */
	R_MMSAR3	= 0x0128,		/* Memory Map 3 Start Address Register */
	R_MMEAR3	= 0x012a,		/* Memory Map 3 End Address Register */
	R_MMOAR3	= 0x012c,		/* Memory Map 3 Offset Address Register */
	R_EIR		= 0x012e,		/* Extend Index Register */
	R_EDR		= 0x012f,		/* Extend Data Register */
	R_MMSAR4	= 0x0130,		/* Memory Map 4 Start Address Register */
	R_MMEAR4	= 0x0132,		/* Memory Map 4 End Address Register */
	R_MMOAR4	= 0x0134,		/* Memory Map 4 Offset Address Register */
	R_IOMOAR0	= 0x0136,		/* IO Map 0 Offset Address Register */
	R_IOMOAR1	= 0x0138,		/* IO Map 1 Offset Address Register */
	R_STM0		= 0x013a,		/* Setup Timing 0 */
	R_CTM0		= 0x013b,		/* Command Timing 0 */
	R_RTM0		= 0x013c,		/* Recovery Timing 0 */
	R_STM1		= 0x013d,		/* Setup Timing 1 */
	R_CTM1		= 0x013e,		/* Command Timing 1 */
	R_RTM1		= 0x013f,		/* Recovery Timing 1 */

	R_IOBASE	= 0x0200,		/* Card IO Space Base Address */
	R_MBASE		= 0x1000,		/* Card Memory Space Base Address */

	RE_EXTCR1	= 0x03,			/* Extension control */
	RE_SYSMMR	= 0x05,			/* System memory map upper address base */
	RE_CVSR		= 0x0a			/* Card voltage sense */
};

#define RB_TSCR_TSEN			0x80
#define RB_TSCR_ORDER			0x08
#define RB_TSCR_SWAP			0x01
#define RB_TSCR_CIBYPASS		0x80

#define RB_TSCR_QPSK			0
#define RB_TSCR_SPI				1
#define RB_TSCR_SSI1			2
#define RB_TSCR_SSI2			3

/* Input / output micro for IO space */
#define INPUT_UINT8(addr)		(*(volatile UINT8  *)(addr))
#define INPUT_UINT16(addr)		(*(volatile UINT16 *)(addr))
#define INPUT_UINT32(addr)		(*(volatile UINT32 *)(addr))
#define OUTPUT_UINT8(addr,val)	((*(volatile UINT8  *)(addr)) = (val))
#define OUTPUT_UINT16(addr,val)	((*(volatile UINT16 *)(addr)) = (val))
#define OUTPUT_UINT32(addr,val)	((*(volatile UINT32 *)(addr)) = (val))

/* Struncture for M3602 CIC private structure */
struct cic_m36_private
{
	void  *priv_addr;			/* Unaligned address of struct real located */
	UINT32 base_addr;

	//struct pci_dev *pci_dev;	/* PCI structure pointer */

	/* CAM command interface registers */
	UINT32 reg_data;			/* CI data register */
	UINT32 reg_cs;				/* CI control/status register */
	UINT32 reg_szl;				/* CI size low byte */
	UINT32 reg_szh;				/* CI size high byte */
	UINT32 slot_1_int_num;
	UINT32 slot_2_int_num;
	void (*callback)(UINT32);
};

struct ali_cic_device
{
	struct cdev cdev;
	struct cic_m36_private *priv;
	UINT32 in_use;
};

enum ali_ci_msg_type
{
	CIC_DATA	= 0,			/* CI data register */
	CIC_CSR,					/* CI command/stauts register */
	CIC_SIZELS,					/* CI size register low bytes */
	CIC_SIZEMS,					/* CI size register high bytes */
	CIC_MEMORY,				/* CI memory space*/
	CIC_BLOCK,					/* CI block data Read/Write */
};

enum ali_cic_device_signal
{
	CIC_ENSTREAM	= 0,		/* Emanciption stream (bypass stream to CAM) */
	CIC_ENSLOT,					/* Enable slot */
	CIC_RSTSLOT,				/* Reset slot */
	CIC_IOMEM,					/* Enable IO & memory space */
	CIC_SLOTSEL,				/* Select slot */
	CIC_CARD_DETECT,			/* CD information */
	CIC_CARD_READY				/* Ready/busy information */
};

#endif /*__CIC_M3602_H__*/
