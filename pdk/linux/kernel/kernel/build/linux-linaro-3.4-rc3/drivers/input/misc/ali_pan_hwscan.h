/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:    ali_pan_hwscan.h
*
*    Description:    This file contains head file definition
*		             of HW scan front panel driver.
*
*           Date            Athor        Version          Reason
*	    ======= ======== ========   =========	=================
*	1.   2010.1.8     Martin Xia   Ver 0.1          Create file.
*	2.	 2010.1.13	  Elliott Hsu  Ver 0.2			Update for pan work.
*****************************************************************************/
#ifndef _PAN_HWSCAN_H_
#define _PAN_HWSCAN_H_


#define HAL_GPIO_I_DIR		0
#define HAL_GPIO_O_DIR		1

#define FP_LOCK_GPIO_NUM            127
#define FP_STANDBY_GPIO_NUM         127
#define FP_CLOCK_GPIO_NUM           127
#define FP_DATA_GPIO_NUM            127
#define FP_CS_GPIO_NUM            	127
#define FP_KEY1_GPIO_NUM            127
#define FP_COM1_GPIO_NUM            127
#define FP_COM2_GPIO_NUM            127
#define FP_COM3_GPIO_NUM            127
#define FP_COM4_GPIO_NUM            127

/* ESC command: 27 (ESC code), PAN_ESC_CMD_xx (CMD type), param1, param2 */
#define	PAN_ESC_CMD_LBD			'L'		/* LBD operate command */
#define PAN_ESC_CMD_LED			'E'		/* LED operate command */
#define PAN_ESC_CMD_LED_LEVEL	0		/* Level status LED */

#define	PAN_ESC_CMD_LBD_ON		1		/* Set LBD to turn on status */
#define	PAN_ESC_CMD_LBD_OFF		0		/* Set LBD to turn off status */

struct pan_gpio_info		/* Total 2 byte */
{
	unsigned short	polar	: 1;	/* Polarity of GPIO, 0 or 1 active(light) */
	unsigned short	io		: 1;	/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	unsigned short	position: 14;	/* GPIO index, upto over 64 GPIO */
} __attribute__((packed));

struct pan_hw_info			/* Total 16 bytes */
{
	/* Offset 0: Panel device type */
	unsigned char					type_kb	: 2;	/* Key board (array) type */
	unsigned char					type_scan:1;	/* 0: slot scan, 1: shadow scan */
	unsigned char					type_key: 1;	/* Key exit or not */
	unsigned char					type_irp: 3;	/* 0: not IRP, 1: NEC, 2: LAB */
	unsigned char					type_mcu: 1;	/* MCU exit or not */
	/* Offset 1: GPIO number */
	unsigned char					num_com : 4;	/* Number of COM PIN, 0: no com; <= 8 */
	unsigned char					pos_colon:4;	/* Position of colon flag, 0 to 7, 8 no colon */
	/* Offset 2: */
	unsigned char					num_scan: 2;	/* Number of scan PIN, 0: no scan; <= 2 */
	unsigned char					rsvd_bits:6;	/* in M3101: 0:show time,1:show "off ",2:show blank, 3:show " off"*/
	/* Offset 3: */
	unsigned char 					rsvd_byte;	/* Reserved for alignment */
	/* Offset 4: Panel shift latch */
	struct pan_gpio_info	flatch;			/* Shifter latch PIN */
	struct pan_gpio_info	fclock;			/* Shifter clock PIN */
	struct pan_gpio_info	fdata;			/* Shifter data PIN */
	/* Offset 10: Panel scan control */
	struct pan_gpio_info	scan[2];			/* Panel scan PIN */
	/* Offset 14: Panel com PIN */
	struct pan_gpio_info	com[8];			/* COM PIN */
	/* Offset 30: Panel LBD control */
	struct pan_gpio_info	lbd[4];			/* LBD GPIO PIN */
	struct pan_gpio_info	rsvd_hw;			/* Reserved for alignment */
	
	/* Offset 40: Panel input attribute */
	unsigned long	delay;				/* Repeat interval first in mS */
    //44
	unsigned long	interval;					/* Repeat interval in mS */
    //48
	unsigned long	release;					/* Release interval in mS */
    //52
	//unsigned long	(*hook_scan)(struct pan_device *dev, unsigned long key);
    //56
	//unsigned long	(*hook_show)(struct pan_device *dev, char *data, unsigned long len);
    //60
} __attribute__((packed));


#define PAN_HWSCAN_CHAR_LIST_NUM 64

struct ali_pan_hwscan_private
{
	struct input_dev			*ali_pan_hwscan_input;
	struct pan_hw_info 		*hw_info;
	struct task_struct *thread_id;
	int run;
	long						bitmap_len;
	struct led_bitmap  		*bitmap_list;
	unsigned char				dotmap;
	unsigned char				colonmap;
	unsigned char				blankmap;
	unsigned char				is_local;
	unsigned short			idlekey;			/* Idel key value */
	unsigned short			maskkey;			/* Active key bit mask */
	unsigned long				key_cnt;			/* Continue press key times */
	unsigned long				keypress_cnt;		/* Continue press key counter */
	unsigned long 				keypress_intv;		/* Continue press key interval */
	unsigned long				keypress_bak;		/* Pressed key saver */
	unsigned char 			(*read8)(unsigned long addr);
	unsigned short 			(*read16)(unsigned long addr);
	void 					(*write8)(unsigned long addr, unsigned char val);
	void 					(*write16)(unsigned long addr, unsigned short val);
	void						(*gpio_dir_set)(long pos, long dir);
	void						(*gpio_val_set)(long pos, long val);
};

/*
long pan_hwscan_open(struct pan_device *dev);
long pan_hwscan_close(struct pan_device *dev);
long pan_hwscan_ioctl(struct pan_device *dev, long cmd, unsigned long param);
void  pan_hwscan_display(struct pan_device *dev, char *data, unsigned long len);
*/

/* LED bitmap */
struct led_bitmap
{
	unsigned char character;
	unsigned char bitmap;
};

/* IO control command */
enum pan_device_panel_ioctrl_command
{
	PAN_DRIVER_SET_HW_INFO	= 0,		/* Front panel driver set hw info command */
	PAN_DRIVER_GET_HW_INFO	= 1,		/* Front panel driver get hw info command */
	PAN_DRIVER_SUSPEND	= 2,		/* Front panel driver suspend command */
	PAN_DRIVER_RESUME	= 3,		/* Front panel driver resume command */	
	PAN_DRIVER_READ_LNB_POWER = 4,			/* Front panel driver NIM LNB power protect status */
	PAN_DRIVER_WRITE_LED_ONOFF =5,
	PAN_DRIVER_UART_SELECT = 6,
	PAN_DRIVER_SET_GPIO	= 7,					/* Front panel driver set gpio command */
	PAN_DRIVER_GET_GPIO = 8,					/* Front panel driver get gpio value */
	PAN_DRIVER_STANDBY  = 9,        				/* Front panel driver enter low power mode */
	PAN_DRIVER_SK_DETECT_POLAR = 10,			/* Front panel driver set key detect polor, only enable for shadow scan */
	PAN_DRIVER_WRITE_REG	= 11,				//!<Front panel driver write register command 
	PAN_DRIVER_READ_REG	= 12,				//!<Front panel driver read register command 	
};

enum pan_key_type
{
	PAN_KEY_TYPE_INVALID	= 0,	/* Invalid key type */
	PAN_KEY_TYPE_REMOTE		= 1,	/* Remote controller */
	PAN_KEY_TYPE_PANEL		= 2,	/* Front panel */
	PAN_KEY_TYPE_JOYSTICK	= 3,	/* Game joy stick */
	PAN_KEY_TYPE_KEYBOARD	= 4		/* Key board */
};

enum pan_key_press
{
	PAN_KEY_RELEASE		= 0,
	PAN_KEY_PRESSED		= 1,
	PAN_KEY_REPEAT		= 2
};

struct pan_device_stats
{
	unsigned short	display_num;			/* Number of display data */
};

struct pan_key
{
	unsigned char  type;					/* The key type */
	unsigned char  state;					/* The key press state */
	unsigned short count;					/* The key counter */
	unsigned long code;					/* The value */
};

#define PAN_KEY_INVALID			0xFFFFFFFF

#endif	/* _PAN_HWSCAN_H_ */

