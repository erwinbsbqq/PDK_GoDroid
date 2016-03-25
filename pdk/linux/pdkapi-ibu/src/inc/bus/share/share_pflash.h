#ifndef __SHARE_PFLASH_H__
#define __SHARE_PFLASH_H__
#include <basic_types.h>
typedef struct {
	UINT16	cs_by_gpio: 1;  /*1: we need use one GPIO do Chip Select.*/
	UINT16	timming_diff: 1;/*1: Share Pin device has different timming setting with pflash chip*/
	UINT16    rst_by_gpio: 1;/*1: we need use one GPIO do Chip Reset.*/
	UINT16   reserved1: 13;
	UINT16	cs_polar	: 1;	/* Polarity of GPIO, 0 or 1 active(light) */
	UINT16	cs_io		: 1;	/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16	cs_position: 14;	/* GPIO index, upto over 64 GPIO */
	UINT16	rst_polar	: 1;	/* Polarity of GPIO, 0 or 1 active(light) */
	UINT16	rst_io		: 1;	/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16	rst_position: 14;	/* GPIO index, upto over 64 GPIO */
	UINT32    timming_setting; /*Store Share Pin device timming setting here*/
}SHARE_PFLASH_CONFIG;

void share_pflash_reset_chip(void);
void share_pflash_init(SHARE_PFLASH_CONFIG * cfg);
void pflash_bus_read( UINT32 addr, UINT32 len, UINT8 * buf );
void pflash_bus_write( UINT32 addr, UINT32 len, UINT8 * buf );
#endif /*__SHARE_PFLASH_H__*/
