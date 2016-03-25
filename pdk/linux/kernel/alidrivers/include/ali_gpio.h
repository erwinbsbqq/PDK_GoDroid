#ifndef _ALI_GPIO_H_
#define _ALI_GPIO_H_

#if defined(CONFIG_ARM)
	#define ALI_SOC_BASE	0x18000000
	#include <linux/ali_reg.h>
#elif defined(CONFIG_MIPS)
	#define ALI_SOC_BASE	0xb8000000
	//#define __REGALIRAW(x)        (x)
#endif

#define GPIO_GROUP_MAX	    5
#define GPIO_PORT_MAX		159


#ifdef CONFIG_GPIOLIB
#include <asm-generic/gpio.h>

#define	SYS_GPIO_GET_VAL	1
#define	SYS_GPIO_SET_VAL	2
#define	SYS_GPIO_DIR_IN	3
#define	SYS_GPIO_DIR_OUT	4

void gpio_irq_type(unsigned offset, unsigned trigger);
void gpio_irq_enable(unsigned offset);
void gpio_irq_disable(unsigned offset);
void gpio_irq_clear(unsigned offset);
int gpio_irq_get_status(unsigned offset);
#else
int gpio_is_valid(int gpio);
int gpio_request(unsigned gpio, const char *label);
void gpio_free(unsigned gpio);
#endif

int gpio_enable_pin(int number);
int gpio_disable_pin(int number);
int enable_gpio_interrupt_pin(int number);
int disable_gpio_interrupt_pin(int number);
//set gpio interrupt as rising edge
int set_gpio_rising_ir_pin(int number, int val);
//set gpio interrupt as falling edge
int set_gpio_falling_ir_pin(int number, int val);
//clear interrupt status
int clear_gpio_interrupt_status(int number);
int clear_gpio_point_isr(int number);
//get interrupt status
int get_gpio_interrupt_status(int number);
int gpio_set_output(unsigned char gpio);
/*  End  martin.zhu 2011-06-29  */
int ali_gpio_direction_input(unsigned char gpio);
int ali_gpio_direction_output(unsigned char gpio, int value);
int ali_gpio_get_value(unsigned char gpio);
int ali_gpio_set_value(unsigned char gpio, int value);


#endif /* _ALI_GPIO_H_ */
