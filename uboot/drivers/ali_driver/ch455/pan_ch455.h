#ifndef _PAN_CH455_H_
#define _PAN_CH455_H_

#include <common.h>
#include <i2c.h>
#include <ali/basic_types.h>
#include <ali/sys_config.h>
#include <ali/osal.h>

#include <ali/hld_dev.h>
#include <ali/pan.h>
#include <ali/pan_dev.h>

//#define PAN_CH455_DEBUG

#ifdef PAN_CH455_DEBUG
#define PAN_CH455_PRINTF		printf
#else
#define PAN_CH455_PRINTF(...)	do{}while(0)
#endif


#define SETING_ADDR	0x48
#define DIG0_ADDR	0x68
#define DIG1_ADDR	0x6a
#define DIG2_ADDR	0x6c
#define DIG3_ADDR	0x6e

#define KEY_ADDR	0x4f

#define CH455_KEY_STATUS_MASK		0x40
#define CH455_KEY_ROW_MASK		0x38
#define CH455_KEY_COLUMN_MASK	0x03

#define CH455_KEY_POWER	0x4C
#define CH455_KEY_ENTER	
#define CH455_KEY_EXIT	
#define CH455_KEY_MENU		0x45
#define CH455_KEY_LEFT	
#define CH455_KEY_UP	
#define CH455_KEY_DOWN	
#define CH455_KEY_RIGHT
#define CH455_KEY_INVALID	0xff

#define CH455_STATUS_UP		0
#define CH455_STATUS_DOWN		1

//scan keyboard every 100 ms
#define CH455_KEY_INTERVAL 100

//0	[INTENS]	[7SEG]	[SLEEP]	0	[ENA]
//0	000		0		0		0	1
#define CH455_MODE	0x01


struct pan_ch455_private
{
	struct pan_hw_info	*hw_info;
	
	struct led_bitmap		*bitmap_list;
	int					bitmap_len;
	OSAL_ID				task_id;

	UINT8	i2c_id;
	UINT8	mode;
	UINT8	dotmap;
	UINT8	colonmap;
	UINT8	blankmap;
	UINT8	is_local;
	UINT16	idlekey;					/* Idel key value */
	UINT8	mask_status;				/* key status bit mask */
	UINT8	mask_row;				/* key code row bit mask */
	UINT8	mask_column;			/* key code column bit mask */

	UINT32	key_cnt;
	UINT32	keypress_cnt;			/* Continue press key counter */
	UINT32	keypress_intv;			/* Continue press key interval */
	UINT32	keypress_bak;			/* Pressed key saver */
	UINT8	bak_status;
	
	UINT32	(*read8)(struct pan_ch455_private *tp,  UINT8 *data);
	UINT32	(*write8)(struct pan_ch455_private *tp, UINT8 dig_addr, UINT8 data);

	UINT8	lock[8]; // support display two lock status.
	UINT8 	disp_buff[8];

};

INT32 pan_ch455_attach(struct pan_configuration *config);
void pan_ch455_id_set(struct pan_device *dev, UINT8 id);

#endif /*_PAN_CH455_H_*/
