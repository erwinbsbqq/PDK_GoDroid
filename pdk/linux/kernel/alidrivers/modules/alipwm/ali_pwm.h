/*
 *      Alitech GPIO PWM Driver
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */
#ifndef __ALI_PWM_H__
#define __ALI_PWM_H__

/* IO control command */
enum pwm_ioctrl_command {
	PWM_GET = 0,
	PWM_SET_PARAM = 1,
	PWM_START = 2,	
	PWM_STOP = 3,
	PWM_START_FORCE = 4,
	PWM_RELEASE = 5,
	PWM_RESET = 6,
};

typedef struct pwm_param_t {
	unsigned int handle;
	unsigned int gpio;
	int dutyCycle;
	long  req_on;
	long  req_off;	
}pwm_param;

typedef struct pwm_data_t {
	int status;//bit0 - start/stop,bit1 - param set,bit2-gpio on/off,bit3-used or not
	unsigned int gpio;
	int irq;
	long  req_on;
	long  req_off;
	void *file;//file pointer
}pwm_data;

#endif /*__ALI_PWM_H__*/
