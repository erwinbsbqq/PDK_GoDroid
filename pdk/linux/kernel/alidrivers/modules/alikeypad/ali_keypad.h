/*
 *      Alitech Keypad Driver
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */
#ifndef __ALI_KEYPAD_H__
#define __ALI_KEYPAD_H__

#include <linux/input.h>

struct gpio_button {
	int gpio;
	unsigned int code;
	unsigned int type;
};

struct keypad_drvdata {
	struct input_dev *input;
	int irq;
	unsigned int nbuttons;
	struct gpio_button button[0];
};

#endif /*__ALI_KEYPAD_H__*/
