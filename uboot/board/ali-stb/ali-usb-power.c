/*
 * Power On USB for ALi M39xx
 *
 * Copyright (C) 2011, ALitech.com
 *
 */
 
#include <common.h>

// DB_M3911_01V02, DB_S3911_01V01
#define DB_3911_GPIO_USB_PWR_CTRL1			(64+7)  	/* GPIO3[7] 1:poweron */
#define DB_3911_GPIO_USB_PWR_CTRL2			(64+5)  	/* GPIO3[5] 1:poweron */

// DB_S3901_01V01
#define DB_3901_GPIO_USB_PWR_CTRL1			(15)    	/* GPIO15 1:poweron */
#define DB_3901_GPIO_USB_PWR_CTRL2			(96+3)  	/* GPIO4[3] 1:poweron */

#define GPIO_CTL_REG						0xb8000430	/* GPIO  */
#define GPIOA_CTL_REG						0xb8000434	/* GPIO2 */
#define GPIOB_CTL_REG						0xb8000438	/* GPIO3 */
#define GPIOC_CTL_REG						0xb800043c	/* GPIO4 */
#define GPIOD_CTL_REG						0xb8000440	/* GPIO5 */
                  		            		                		
#define HAL_GPIO_I_DIR						0
#define HAL_GPIO_O_DIR						1
                                    		
#define HAL_GPIO_DIR_REG					0xb8000058
#define HAL_GPIO_DIR_GET()					(*(volatile u32 *)HAL_GPIO_DIR_REG)
#define HAL_GPIO_DIR_SET(mode)				(*(volatile u32 *)HAL_GPIO_DIR_REG = (mode))
#define HAL_GPIO1_DIR_REG					0xb80000d8
#define HAL_GPIO1_DIR_GET()					(*(volatile u32 *)HAL_GPIO1_DIR_REG)
#define HAL_GPIO1_DIR_SET(mode)				(*(volatile u32 *)HAL_GPIO1_DIR_REG = (mode))
#define HAL_GPIO2_DIR_REG					0xb80000f8
#define HAL_GPIO2_DIR_GET()					(*(volatile u32 *)HAL_GPIO2_DIR_REG)
#define HAL_GPIO2_DIR_SET(mode)				(*(volatile u32 *)HAL_GPIO2_DIR_REG = (mode))
#define HAL_GPIO3_DIR_REG					0xb8000358
#define HAL_GPIO3_DIR_GET()					(*(volatile u32 *)HAL_GPIO3_DIR_REG)
#define HAL_GPIO3_DIR_SET(mode)				(*(volatile u32 *)HAL_GPIO3_DIR_REG = (mode))
		
#define HAL_GPIO_DO_REG						0xb8000054
#define HAL_GPIO1_DO_REG					0xb80000d4
#define HAL_GPIO2_DO_REG					0xb80000f4
#define HAL_GPIO3_DO_REG					0xb8000354		
                                    		
#define HAL_GPIO_WRITE(val)					(*(volatile u32 *)HAL_GPIO_DO_REG   = (val))
#define HAL_GPIO1_WRITE(val)				(*(volatile u32 *)HAL_GPIO1_DO_REG  = (val))
#define HAL_GPIO2_WRITE(val)				(*(volatile u32 *)HAL_GPIO2_DO_REG  = (val))
#define HAL_GPIO3_WRITE(val)				(*(volatile u32 *)HAL_GPIO3_DO_REG  = (val))

#define HAL_GPIO_BIT_DIR_SET(pos, val)	\
		do {							\
			( ( pos < 32) ? HAL_GPIO_DIR_SET((HAL_GPIO_DIR_GET()   & ~(1 << (pos)))      | ((val) << (pos))) \
			: ((pos < 64) ? HAL_GPIO1_DIR_SET((HAL_GPIO1_DIR_GET() & ~(1 << (pos - 32))) | ((val) << (pos - 32))) \
			: ((pos < 96) ? HAL_GPIO2_DIR_SET((HAL_GPIO2_DIR_GET() & ~(1 << (pos - 64))) | ((val) << (pos - 64))) \
			:               HAL_GPIO3_DIR_SET((HAL_GPIO3_DIR_GET() & ~(1 << (pos - 96))) | ((val) << (pos - 96))))));\
		} while (0)
		
#define HAL_GPIO_BIT_SET(pos, val)		\
		do {							\
			( ( pos < 32) ? HAL_GPIO_WRITE(( (*(volatile u32 *)HAL_GPIO_DO_REG)  & ~(1 << (pos)))      | ((val) << (pos))) \
			: ((pos < 64) ? HAL_GPIO1_WRITE(((*(volatile u32 *)HAL_GPIO1_DO_REG) & ~(1 << (pos - 32))) | ((val) << (pos - 32))) \
			: ((pos < 96) ? HAL_GPIO2_WRITE(((*(volatile u32 *)HAL_GPIO2_DO_REG) & ~(1 << (pos - 64))) | ((val) << (pos - 64))) \
			:               HAL_GPIO3_WRITE(((*(volatile u32 *)HAL_GPIO3_DO_REG) & ~(1 << (pos - 96))) | ((val) << (pos - 96)))))); \
		} while (0)		
		
static inline int gpio_direction_output(unsigned gpio, int value)
{
	HAL_GPIO_BIT_DIR_SET(gpio, HAL_GPIO_O_DIR);
	HAL_GPIO_BIT_SET(gpio, (value)? 1 : 0);
	return 0;
}

//Power On USB.
int usb_power_on(void)
{
	u32 data;
	

#if 0 // 3911
    data = *((volatile unsigned long *) GPIOB_CTL_REG);
    data |= (1 << 5);           /* GPIO3[5] DB_3911_GPIO_USB_PWR_CTRL2 */
    data |= (1 << 7);           /* GPIO3[7] DB_3911_GPIO_USB_PWR_CTRL1 */
    *((volatile unsigned long *) GPIOB_CTL_REG) = data;
    gpio_direction_output(DB_3911_GPIO_USB_PWR_CTRL1, 1);
    gpio_direction_output(DB_3911_GPIO_USB_PWR_CTRL2, 1); 
        
#endif

#if 1 // 3901
    data = *((volatile unsigned long *) GPIO_CTL_REG);
    data |= (1 << 15);          /* GPIO15 DB_3901_GPIO_USB_PWR_CTRL1 */
    *((volatile unsigned long *) GPIO_CTL_REG) = data;

    data = *((volatile unsigned long *) GPIOC_CTL_REG);
    data |= (1 << 3);           /* GPIO4[3] DB_3901_GPIO_USB_PWR_CTRL2 */
    *((volatile unsigned long *) GPIOC_CTL_REG) = data;
    gpio_direction_output(DB_3901_GPIO_USB_PWR_CTRL1, 1);
    gpio_direction_output(DB_3901_GPIO_USB_PWR_CTRL2, 1);     
#endif  	

	return 0;	
}
