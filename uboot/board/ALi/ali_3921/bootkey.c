#include <common.h>
#include <ali_ir.h>
#include <asm/gpio.h>


#include "deviceinfo.h"

static int _gpio_check(BOOTKEY_CONFIG *config)
{
	unsigned int gpio;
	unsigned int polar;
	
	if (config->gpio_enable == 0)
	{
		printf("GPIO check disabled!\n");
		return 0;
	}


	gpio = config->gpio_position;
	polar = config->gpio_polar;
	
	gpio_direction_input(gpio);

	if (gpio_get_value(gpio) == polar) {
		mdelay(200);
		if (gpio_get_value(gpio) == polar) {
			printf("get gpio bootkey: %d\n",gpio);			  
			return 1;
		}
	}
	
	return 0;
}

static int _ir_check(BOOTKEY_CONFIG *config)
{
	unsigned int timeout;
	unsigned long long etime = 0;
	unsigned int key = 0;
	unsigned char repeat = 0xff;

	if (config->ir_enable == 0)
	{
		printf("IR check disabled!\n");
		return 0;
	}

	timeout = config->ir_timeout;

	irc_init();
	mdelay(150);

	irc_lsr();
	if (irc_get_ir_pulse() == 0)
		return 0;

	etime = get_ticks() + timeout;
	while (get_ticks() < etime)
	{			
		irc_lsr();
		if (/*irc_get_last_act_code() == 0 && */irc_get_ir_repeat() == 0)
		{
			irc_close();
			return 0;
		}
	}

	printf("u-boot got ir key!\n");

	irc_close();
	return 1;
}

static int _panel_check(BOOTKEY_CONFIG *config)
{
	if (config->panel_enable == 0)
		return 0;

	return 0;
}

int bootkey_check(void)
{
	BOOTKEY_CONFIG *config;

	if (get_bootkey((unsigned char **)&config) <= 0)
	{
		printf("get bootkey config fail.\n");
		return 0;
	}

	if (_gpio_check(config))
		return 1;

#ifdef CONFIG_ALI_IR
	if (_ir_check(config))
		return 2;
#endif

	if (_panel_check(config))
		return 3;

	printf("Can't check bootkey.\n");
	
	return 0;
}

