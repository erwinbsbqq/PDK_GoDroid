/*
* ALi GPIO Driver.
*/
#include <linux/version.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <asm/errno.h>
#include <asm/mach-ali/m36_gpio.h>
#include <ali_interrupt.h>

#include <alidefinition/adf_basic.h>
#include <linux/ali_reg.h>



//#define ENABLE_M3701C_JTAG_DEBG

#define GPIO_CTL_REG        __REGALIRAW(0x18000430)  /* GPIO  */
#define GPIOA_CTL_REG       __REGALIRAW(0x18000434)  /* GPIO2 */
#define GPIOB_CTL_REG       __REGALIRAW(0x18000438)  /* GPIO3 */
#define GPIOC_CTL_REG       __REGALIRAW(0x1800043c)  /* GPIO4 */
#define GPIOD_CTL_REG		__REGALIRAW(0x18000440)	 /* GPIO5 */

#define HAL_GPIO_IER_REG    __REGALIRAW(0x18000044)
#define HAL_GPIO_REC_REG    __REGALIRAW(0x18000048)
#define HAL_GPIO_FEC_REG    __REGALIRAW(0x1800004c)
#define HAL_GPIO_ISR_REG    __REGALIRAW(0x1800005c)
#define HAL_GPIO_DIR_REG    __REGALIRAW(0x18000058)
#define HAL_GPIO_DI_REG     __REGALIRAW(0x18000050)
#define HAL_GPIO_DO_REG     __REGALIRAW(0x18000054)

#define HAL_GPIO1_IER_REG	__REGALIRAW(0x180000c4)
#define HAL_GPIO1_REC_REG	__REGALIRAW(0x180000c8)
#define HAL_GPIO1_FEC_REG	__REGALIRAW(0x180000cc)
#define HAL_GPIO1_ISR_REG	__REGALIRAW(0x180000dc)
#define HAL_GPIO1_DIR_REG	__REGALIRAW(0x180000d8)
#define HAL_GPIO1_DI_REG	__REGALIRAW(0x180000d0)
#define HAL_GPIO1_DO_REG	__REGALIRAW(0x180000d4)

#define HAL_GPIO2_IER_REG	__REGALIRAW(0x180000e4)
#define HAL_GPIO2_REC_REG	__REGALIRAW(0x180000e8)
#define HAL_GPIO2_FEC_REG	__REGALIRAW(0x180000ec)
#define HAL_GPIO2_ISR_REG	__REGALIRAW(0x180000fc)
#define HAL_GPIO2_DIR_REG	__REGALIRAW(0x180000f8)
#define HAL_GPIO2_DI_REG	__REGALIRAW(0x180000f0)
#define HAL_GPIO2_DO_REG	__REGALIRAW(0x180000f4)

#define HAL_GPIO3_IER_REG   __REGALIRAW(0x18000344)
#define HAL_GPIO3_REC_REG   __REGALIRAW(0x18000348)
#define HAL_GPIO3_FEC_REG   __REGALIRAW(0x1800034c)
#define HAL_GPIO3_ISR_REG   __REGALIRAW(0x1800035c)
#define HAL_GPIO3_DIR_REG   __REGALIRAW(0x18000358)
#define HAL_GPIO3_DI_REG    __REGALIRAW(0x18000350)
#define HAL_GPIO3_DO_REG    __REGALIRAW(0x18000354)

#define HAL_GPIO4_IER_REG   __REGALIRAW(0x18000444)
#define HAL_GPIO4_REC_REG   __REGALIRAW(0x18000448)
#define HAL_GPIO4_FEC_REG   __REGALIRAW(0x1800044c)
#define HAL_GPIO4_ISR_REG   __REGALIRAW(0x1800045c)
#define HAL_GPIO4_DIR_REG   __REGALIRAW(0x18000458)
#define HAL_GPIO4_DI_REG    __REGALIRAW(0x18000450)
#define HAL_GPIO4_DO_REG    __REGALIRAW(0x18000454)


#define HAL_GPIO_I_DIR      0
#define HAL_GPIO_O_DIR      1
#define HAL_GPIO_INT_EN     0
#define HAL_GPIO_INT_DIS    1
#define HAL_GPIO_EDG_EN     1
#define HAL_GPIO_EDG_DIS    0
#define HAL_GPIO_ENABLE     1
#define HAL_GPIO_DISABLE     0



/* The first set of GPIO */

#define HAL_GPIO_READ()             (*(volatile DWORD *)HAL_GPIO_DI_REG)

#define HAL_GPIO_WRITE(val)         (*(volatile DWORD *)HAL_GPIO_DO_REG = (val))
#define HAL_GPIO_DIR_GET()          (*(volatile DWORD *)HAL_GPIO_DIR_REG)
#define HAL_GPIO_DIR_SET(mode)      (*(volatile DWORD *)HAL_GPIO_DIR_REG = (mode))
#define HAL_GPIO_IER_SET(val)       (*(volatile DWORD *)HAL_GPIO_IER_REG = (val))
#define HAL_GPIO_RER_SET(val)       (*(volatile DWORD *)HAL_GPIO_REC_REG = (val))
#define HAL_GPIO_FER_SET(val)       (*(volatile DWORD *)HAL_GPIO_FEC_REG = (val))
#define HAL_GPIO_ISR_GET()          (*(volatile DWORD *)HAL_GPIO_ISR_REG)
#define HAL_GPIO_ISR_SET(val)       (*(volatile DWORD *)HAL_GPIO_ISR_REG = (val))

#define HAL_GPIO1_READ()            (*(volatile DWORD *)HAL_GPIO1_DI_REG)
#define HAL_GPIO1_WRITE(val)        (*(volatile DWORD *)HAL_GPIO1_DO_REG = (val))
#define HAL_GPIO1_DIR_GET()         (*(volatile DWORD *)HAL_GPIO1_DIR_REG)
#define HAL_GPIO1_DIR_SET(mode)     (*(volatile DWORD *)HAL_GPIO1_DIR_REG = (mode))
#define HAL_GPIO1_IER_SET(val)      (*(volatile DWORD *)HAL_GPIO1_IER_REG = (val))
#define HAL_GPIO1_RER_SET(val)      (*(volatile DWORD *)HAL_GPIO1_REC_REG = (val))
#define HAL_GPIO1_FER_SET(val)      (*(volatile DWORD *)HAL_GPIO1_FEC_REG = (val))
#define HAL_GPIO1_ISR_GET()         (*(volatile DWORD *)HAL_GPIO1_ISR_REG)
#define HAL_GPIO1_ISR_SET(val)      (*(volatile DWORD *)HAL_GPIO1_ISR_REG = (val))


#define HAL_GPIO2_READ()            (*(volatile DWORD *)HAL_GPIO2_DI_REG)
#define HAL_GPIO2_WRITE(val)        (*(volatile DWORD *)HAL_GPIO2_DO_REG = (val))
#define HAL_GPIO2_DIR_GET()         (*(volatile DWORD *)HAL_GPIO2_DIR_REG)
#define HAL_GPIO2_DIR_SET(mode)     (*(volatile DWORD *)HAL_GPIO2_DIR_REG = (mode))
#define HAL_GPIO2_IER_SET(val)      (*(volatile DWORD *)HAL_GPIO2_IER_REG = (val))
#define HAL_GPIO2_RER_SET(val)      (*(volatile DWORD *)HAL_GPIO2_REC_REG = (val))
#define HAL_GPIO2_FER_SET(val)      (*(volatile DWORD *)HAL_GPIO2_FEC_REG = (val))
#define HAL_GPIO2_ISR_GET()         (*(volatile DWORD *)HAL_GPIO2_ISR_REG)
#define HAL_GPIO2_ISR_SET(val)      (*(volatile DWORD *)HAL_GPIO2_ISR_REG = (val))

#define HAL_GPIO3_READ()            (*(volatile DWORD *)HAL_GPIO3_DI_REG)
#define HAL_GPIO3_WRITE(val)        (*(volatile DWORD *)HAL_GPIO3_DO_REG = (val))
#define HAL_GPIO3_DIR_GET()         (*(volatile DWORD *)HAL_GPIO3_DIR_REG)
#define HAL_GPIO3_DIR_SET(mode)     (*(volatile DWORD *)HAL_GPIO3_DIR_REG = (mode))
#define HAL_GPIO3_IER_SET(val)      (*(volatile DWORD *)HAL_GPIO3_IER_REG = (val))
#define HAL_GPIO3_RER_SET(val)      (*(volatile DWORD *)HAL_GPIO3_REC_REG = (val))
#define HAL_GPIO3_FER_SET(val)      (*(volatile DWORD *)HAL_GPIO3_FEC_REG = (val))
#define HAL_GPIO3_ISR_GET()         (*(volatile DWORD *)HAL_GPIO3_ISR_REG)
#define HAL_GPIO3_ISR_SET(val)      (*(volatile DWORD *)HAL_GPIO3_ISR_REG = (val))

#define HAL_GPIO4_READ()            (*(volatile DWORD *)HAL_GPIO4_DI_REG)
#define HAL_GPIO4_WRITE(val)        (*(volatile DWORD *)HAL_GPIO4_DO_REG = (val))
#define HAL_GPIO4_DIR_GET()         (*(volatile DWORD *)HAL_GPIO4_DIR_REG)
#define HAL_GPIO4_DIR_SET(mode)     (*(volatile DWORD *)HAL_GPIO4_DIR_REG = (mode))
#define HAL_GPIO4_IER_SET(val)      (*(volatile DWORD *)HAL_GPIO4_IER_REG = (val))
#define HAL_GPIO4_RER_SET(val)      (*(volatile DWORD *)HAL_GPIO4_REC_REG = (val))
#define HAL_GPIO4_FER_SET(val)      (*(volatile DWORD *)HAL_GPIO4_FEC_REG = (val))
#define HAL_GPIO4_ISR_GET()         (*(volatile DWORD *)HAL_GPIO4_ISR_REG)
#define HAL_GPIO4_ISR_SET(val)      (*(volatile DWORD *)HAL_GPIO4_ISR_REG = (val))

/*  End  */

// get input gpio value
#define HAL_GPIO_BIT_GET(pos)			\
			((pos < 32) ? ((HAL_GPIO_READ() >> (pos)) & 1) \
			: ((pos < 64) ? ((HAL_GPIO1_READ() >> (pos - 32)) & 1) \
			: ((pos < 96) ? ((HAL_GPIO2_READ() >> (pos - 64)) & 1) \
			: ((pos < 128) ? ((HAL_GPIO3_READ() >> (pos - 96)) & 1) \
			: ((HAL_GPIO4_READ() >> (pos - 128)) & 1)))))

#define HAL_GPIO_BIT_DIR_GET(pos)		\
			((pos < 32) ? ((HAL_GPIO_DIR_GET() >> (pos)) & 1) \
			: ((pos < 64) ? ((HAL_GPIO1_DIR_GET() >> (pos - 32)) & 1) \
			: ((pos < 96) ? ((HAL_GPIO2_DIR_GET() >> (pos - 64)) & 1) \
			: ((pos < 128) ? ((HAL_GPIO3_DIR_GET() >> (pos - 96)) & 1) \
			: ((HAL_GPIO4_DIR_GET() >> (pos - 128)) & 1))))

#define HAL_GPIO_BIT_DIR_SET(pos, val)	\
		do { \
			((pos < 32) ? HAL_GPIO_DIR_SET((HAL_GPIO_DIR_GET() & ~(1 << (pos))) | ((val) << (pos))) \
			: ((pos < 64) ? HAL_GPIO1_DIR_SET((HAL_GPIO1_DIR_GET() & ~(1 << (pos - 32))) | ((val) << (pos - 32))) \
			: ((pos < 96) ? HAL_GPIO2_DIR_SET((HAL_GPIO2_DIR_GET() & ~(1 << (pos - 64))) | ((val) << (pos - 64))) \
			: ((pos < 128) ? HAL_GPIO3_DIR_SET((HAL_GPIO3_DIR_GET() & ~(1 << (pos - 96))) | ((val) << (pos - 96))) \
			: HAL_GPIO4_DIR_SET((HAL_GPIO4_DIR_GET() & ~(1 << (pos - 128))) | ((val) << (pos - 128)))))));\
		} while (0)

#define HAL_GPIO_BIT_SET(pos, val)		\
		do { \
			((pos < 32)	? HAL_GPIO_WRITE(((*(volatile DWORD *)HAL_GPIO_DO_REG) & ~(1 << (pos))) | ((val) << (pos))) \
			: ((pos < 64) ? HAL_GPIO1_WRITE(((*(volatile DWORD *)HAL_GPIO1_DO_REG) & ~(1 << (pos - 32))) | ((val) << (pos - 32))) \
			: ((pos < 96) ? HAL_GPIO2_WRITE(((*(volatile DWORD *)HAL_GPIO2_DO_REG) & ~(1 << (pos - 64))) | ((val) << (pos - 64))) \
			: ((pos < 128) ? HAL_GPIO3_WRITE(((*(volatile DWORD *)HAL_GPIO3_DO_REG) & ~(1 << (pos - 96))) | ((val) << (pos - 96))) \
			: HAL_GPIO4_WRITE(((*(volatile DWORD *)HAL_GPIO4_DO_REG) & ~(1 << (pos - 128))) | ((val) << (pos - 128))))))); \
		} while (0)

#define HAL_GPIO_INT_SET(pos, en)		\
		do { \
			((pos < 32)					\
			? HAL_GPIO_IER_SET(((*(volatile DWORD *)HAL_GPIO_IER_REG) & ~(1 << (pos))) | ((en) << (pos))) \
			: ((pos < 64) 				\
			? HAL_GPIO1_IER_SET(((*(volatile DWORD *)HAL_GPIO1_IER_REG) & ~(1 << (pos - 32))) | ((en) << (pos - 32))) \
			: ((pos < 96) 				\
			? HAL_GPIO2_IER_SET(((*(volatile DWORD *)HAL_GPIO2_IER_REG) & ~(1 << (pos - 64))) | ((en) << (pos - 64))) \
			: ((pos < 128) 				\
			? HAL_GPIO3_IER_SET(((*(volatile DWORD *)HAL_GPIO3_IER_REG) & ~(1 << (pos - 96))) | ((en) << (pos - 96))) \
			: HAL_GPIO4_IER_SET(((*(volatile DWORD *)HAL_GPIO4_IER_REG) & ~(1 << (pos - 128))) | ((en) << (pos - 128))))))); \
		} while (0)

#define HAL_GPIO_INT_REDG_SET(pos, rise)	\
		do { \
			((pos < 32)					\
			? HAL_GPIO_RER_SET(((*(volatile DWORD *)HAL_GPIO_REC_REG) & ~(1 << (pos))) | ((rise) << (pos))) \
			: ((pos < 64) 				\
			? HAL_GPIO1_RER_SET(((*(volatile DWORD *)HAL_GPIO1_REC_REG) & ~(1 << (pos - 32))) | ((rise) << (pos - 32))) \
			: ((pos < 96) 				\
			? HAL_GPIO2_RER_SET(((*(volatile DWORD *)HAL_GPIO2_REC_REG) & ~(1 << (pos - 64))) | ((rise) << (pos - 64))) \
			: ((pos < 128) 				\
			? HAL_GPIO3_RER_SET(((*(volatile DWORD *)HAL_GPIO3_REC_REG) & ~(1 << (pos - 96))) | ((rise) << (pos - 96))) \
			: HAL_GPIO4_RER_SET(((*(volatile DWORD *)HAL_GPIO4_REC_REG) & ~(1 << (pos - 128))) | ((rise) << (pos - 128))))))); \
		} while (0)

#define HAL_GPIO_INT_FEDG_SET(pos, fall)	\
		do { \
			((pos < 32)					\
			? HAL_GPIO_FER_SET(((*(volatile DWORD *)HAL_GPIO_FEC_REG) & ~(1 << (pos))) | ((fall) << (pos))) \
			: ((pos < 64)				\
			? HAL_GPIO1_FER_SET(((*(volatile DWORD *)HAL_GPIO1_FEC_REG) & ~(1 << (pos - 32))) | ((fall) << (pos - 32))) \
			: ((pos < 96) 				\
			? HAL_GPIO2_FER_SET(((*(volatile DWORD *)HAL_GPIO2_FEC_REG) & ~(1 << (pos - 64))) | ((fall) << (pos - 64))) \
			: ((pos < 128) 				\
			? HAL_GPIO3_FER_SET(((*(volatile DWORD *)HAL_GPIO3_FEC_REG) & ~(1 << (pos - 96))) | ((fall) << (pos - 96))) \
			: HAL_GPIO4_FER_SET(((*(volatile DWORD *)HAL_GPIO4_FEC_REG) & ~(1 << (pos - 128))) | ((fall) << (pos - 128))))))); \
		} while (0)

#define HAL_GPIO_INT_EDG_SET(pos, rise, fall)	\
		do { \
			((pos < 32)					\
			? (HAL_GPIO_RER_SET(((*(volatile DWORD *)HAL_GPIO_REC_REG) & ~(1 << (pos))) | ((rise) << (pos))), \
			  HAL_GPIO_FER_SET(((*(volatile DWORD *)HAL_GPIO_FEC_REG) & ~(1 << (pos))) | ((fall) << (pos)))) \
			: ((pos < 64)				\
			? (HAL_GPIO1_RER_SET(((*(volatile DWORD *)HAL_GPIO1_REC_REG) & ~(1 << (pos - 32))) | ((rise) << (pos - 32))), \
			  HAL_GPIO1_FER_SET(((*(volatile DWORD *)HAL_GPIO1_FEC_REG) & ~(1 << (pos - 32))) | ((fall) << (pos - 32)))) \
			: ((pos < 96)				\
			?  (HAL_GPIO2_RER_SET(((*(volatile DWORD *)HAL_GPIO2_REC_REG) & ~(1 << (pos - 64))) | ((rise) << (pos - 64))), \
			  HAL_GPIO2_FER_SET(((*(volatile DWORD *)HAL_GPIO2_FEC_REG) & ~(1 << (pos - 64))) | ((fall) << (pos - 64)))) \
			  : ((pos < 128)				\
			?  (HAL_GPIO3_RER_SET(((*(volatile DWORD *)HAL_GPIO3_REC_REG) & ~(1 << (pos - 96))) | ((rise) << (pos - 96))), \
			  HAL_GPIO3_FER_SET(((*(volatile DWORD *)HAL_GPIO3_FEC_REG) & ~(1 << (pos - 96))) | ((fall) << (pos - 96)))) \
			:  (HAL_GPIO4_RER_SET(((*(volatile DWORD *)HAL_GPIO4_REC_REG) & ~(1 << (pos - 128))) | ((rise) << (pos - 128))), \
			  HAL_GPIO4_FER_SET(((*(volatile DWORD *)HAL_GPIO4_FEC_REG) & ~(1 << (pos - 128))) | ((fall) << (pos - 128)))))))); \
		} while (0)

#define HAL_GPIO_INT_STA_GET(pos)		\
			((pos < 32) ? ((HAL_GPIO_ISR_GET() >> (pos)) & 1) \
			: ((pos < 64) ? ((HAL_GPIO1_ISR_GET() >> (pos - 32)) & 1) \
			: ((pos < 96) ? ((HAL_GPIO2_ISR_GET() >> (pos - 64)) & 1) \
			: ((pos < 128) ? ((HAL_GPIO3_ISR_GET() >> (pos - 96)) & 1) \
			: ((HAL_GPIO4_ISR_GET() >> (pos - 128)) & 1)))))

#define HAL_GPIO_INT_CLEAR(pos)		\
			((pos < 32) ? (HAL_GPIO_ISR_SET(1 << (pos))) \
			: ((pos < 64) ? (HAL_GPIO1_ISR_SET(1 << (pos-32))) \
			: ((pos < 96) ? (HAL_GPIO2_ISR_SET(1 << (pos-64))) \
			: ((pos < 128) ? (HAL_GPIO3_ISR_SET(1 << (pos-96))) \
			: (HAL_GPIO4_ISR_SET(1 << (pos-128)))))))


/*add by martin.zhu 2011-06-29 */
/*Some pinmux pin default value is 0, some is 1, define the  enable value .
  Use to enable other Function ( GPIO)
*/
#define HAL_PINMUX_DEFALT_VALUE        0x4E461CD3
#define HAL_PINMUX1_DEFALT_VALUE       0x100793C8

#define HAL_PINMUX_CTRL_REG            __REGALIRAW(0x18000088)
#define HAL_PINMUX_CTRL1_REG           __REGALIRAW(0x1800008c)
// PINMUX REG4 is revert

#define HAL_PINMUX_SET(val)           (*(volatile DWORD *)HAL_PINMUX_CTRL_REG = (val))
#define HAL_PINMUX1_SET(val)          (*(volatile DWORD *)HAL_PINMUX_CTRL1_REG = (val))

#define GET_PINMAX_BIT(val, pos)      (((val >> (pos)) & 1) << pos)
#define GET_BYTE(addr)            		 (*(volatile unsigned char *)(addr))
#define SET_BITE(addr, pos, val)       *(volatile unsigned long *)(addr) = (((*(volatile DWORD *)addr) & ~(1 << (pos))) | ((val) << (pos)))
#define WRITE_DATA_OR(add, val)        *(volatile unsigned long *)(add) |= (unsigned long)(val)
#define WRITE_DATA_AND(add, val)       *(volatile unsigned long *)(add) &= (unsigned long)(val)
#define WRITE_DATA_EQUAL(addr, val)    *(volatile unsigned long *)(addr)= (unsigned long)(val)


#define HAL_PINMUX_GPIO_ENABLE(pos)       \
    do { \
        ((pos < 32)                 \
        ? HAL_PINMUX_SET(((*(volatile DWORD *)HAL_PINMUX_CTRL_REG) & ~(1 << (pos))) | (GET_PINMAX_BIT(HAL_PINMUX_DEFALT_VALUE , pos) )) \
        : ((pos < 64)               \
        ? HAL_PINMUX1_SET(((*(volatile DWORD *)HAL_PINMUX_CTRL1_REG) & ~(1 << (pos - 32))) | (GET_PINMAX_BIT(HAL_PINMUX1_DEFALT_VALUE , pos))) \
        : 1) );\
    } while (0)

#define HAL_GPIO_CTL_SET(val)       (*(volatile DWORD *)GPIO_CTL_REG = (val))
#define HAL_GPIOA_CTL_SET(val)       (*(volatile DWORD *)GPIOA_CTL_REG = (val))
#define HAL_GPIOB_CTL_SET(val)       (*(volatile DWORD *)GPIOB_CTL_REG = (val))
#define HAL_GPIOC_CTL_SET(val)       (*(volatile DWORD *)GPIOC_CTL_REG = (val))
#define HAL_GPIOD_CTL_SET(val)       (*(volatile DWORD *)GPIOD_CTL_REG = (val))

//enable/disable  GPIO function
#define HAL_GPIO_FUNC_ENABLE(pos, en)       \
        do { \
            ((pos < 32)                 \
            ? HAL_GPIO_CTL_SET(((*(volatile DWORD *)GPIO_CTL_REG) &~(1<<(pos))) | (en << (pos))) \
            : ((pos < 64)               \
            ? HAL_GPIOA_CTL_SET(((*(volatile DWORD *)GPIOA_CTL_REG) &~(1<<(pos - 32)))  | (en << (pos - 32))) \
            : ((pos < 96)               \
            ? HAL_GPIOB_CTL_SET(((*(volatile DWORD *)GPIOB_CTL_REG) &~(1<<(pos - 64))) | (en << (pos - 64))) \
             : ((pos < 128)               \
            ? HAL_GPIOC_CTL_SET(((*(volatile DWORD *)GPIOC_CTL_REG) &~(1<<(pos - 96)))  | (en << (pos - 96))) \
		: HAL_GPIOD_CTL_SET(((*(volatile DWORD *)GPIOD_CTL_REG) &~(1<<(pos - 128))) | (en << (pos - 128))))))); \
		} while (0)
        

#define HAL_GPIO_POINT_INT_CLEAR(pos)		\
			((pos < 32) ? (HAL_GPIO_ISR_SET(HAL_GPIO_ISR_GET())) \
			: ((pos < 64) ? (HAL_GPIO1_ISR_SET(HAL_GPIO1_ISR_GET())) \
			: ((pos < 96) ? (HAL_GPIO2_ISR_SET(HAL_GPIO2_ISR_GET())) \
			: ((pos < 128) ? (HAL_GPIO3_ISR_SET(HAL_GPIO3_ISR_GET())) \
			: (HAL_GPIO4_ISR_SET(HAL_GPIO4_ISR_GET()))))))		
extern u32 ali_i2c_gpio_init(void);

static spinlock_t m36_gpio_lock = __SPIN_LOCK_UNLOCKED(m36_gpio_lock);


#ifndef CONFIG_GPIOLIB
int gpio_is_valid(int gpio)
{
    return gpio >= 0 && gpio < GPIO_PORT_MAX;
}

int gpio_request(unsigned gpio, const char *label)
{
    printk(KERN_ALERT "gpio_request: GPIO number(%d), label %s\n", gpio, label);
    return 0;
}

void gpio_free(unsigned gpio)
{
}
#endif


int gpio_enable_pin(int number)
{
    if( !gpio_is_valid(number))
        return -1;
//    HAL_PINMUX_GPIO_ENABLE(number);
	HAL_GPIO_FUNC_ENABLE(number, HAL_GPIO_ENABLE);
	return 0;
}

int gpio_disable_pin(int number)
{
    if( !gpio_is_valid(number))
        return -1;
//    HAL_PINMUX_GPIO_ENABLE(number);
	HAL_GPIO_FUNC_ENABLE(number, HAL_GPIO_DISABLE);
	return 0;
}


int enable_gpio_interrupt_pin(int number)
{
    if( !gpio_is_valid(number))
        return -1;
    HAL_GPIO_INT_SET(number, 1);
	return 0;
}		

int disable_gpio_interrupt_pin(int number)
{
    if( !gpio_is_valid(number))
        return -1;
    HAL_GPIO_INT_SET(number, 0);
	return 0;
}	

//set gpio interrupt as rising edge
int set_gpio_rising_ir_pin(int number, int val)
{
    if( !gpio_is_valid(number))
        return -1;
    HAL_GPIO_INT_REDG_SET(number, val);
	return 0;
}	

//set gpio interrupt as falling edge
int set_gpio_falling_ir_pin(int number, int val)
{
    if( !gpio_is_valid(number))
        return -1;
    HAL_GPIO_INT_FEDG_SET(number, val);
	return 0;
}	

//clear interrupt status
int clear_gpio_interrupt_status(int number)
{
    if( !gpio_is_valid(number))
        return -1;
    HAL_GPIO_INT_CLEAR(number);
	return 0;
}

int clear_gpio_point_isr(int number)
{
    if( !gpio_is_valid(number))
        return -1;
    return HAL_GPIO_POINT_INT_CLEAR(number);
}

//get interrupt status
int get_gpio_interrupt_status(int number)
{
    if( !gpio_is_valid(number))
        return -1;
    return HAL_GPIO_INT_STA_GET(number);
}

int gpio_set_output(unsigned char gpio)
{
    if( !gpio_is_valid(gpio))
		return -1;

    HAL_GPIO_BIT_DIR_SET(gpio, HAL_GPIO_O_DIR);
    return 0;
}


/*  End  martin.zhu 2011-06-29  */
int ali_gpio_direction_input(unsigned char gpio)
{
    if( !gpio_is_valid(gpio))
		return -1;
	
	/* fix IC bug */
	if ((68 == gpio) || (69 == gpio) || (70 == gpio) || (145 == gpio) || (146 == gpio))
	{
		HAL_GPIO_BIT_DIR_SET(gpio, HAL_GPIO_O_DIR);
	}
	else
	{
		HAL_GPIO_BIT_DIR_SET(gpio, HAL_GPIO_I_DIR);
	}
    
    return 0;
}

int ali_gpio_direction_output(unsigned char gpio, int value)
{
    if( !gpio_is_valid(gpio))
		return -1;

	/* fix IC bug */
	if ((68 == gpio) || (69 == gpio) || (70 == gpio) || (145 == gpio) || (146 == gpio))
	{
		HAL_GPIO_BIT_DIR_SET(gpio, HAL_GPIO_I_DIR);
	}
	else
	{
		HAL_GPIO_BIT_DIR_SET(gpio, HAL_GPIO_O_DIR);
	}
    
    HAL_GPIO_BIT_SET(gpio, (value)? 1 : 0);
    return 0;
}


int ali_gpio_get_value(unsigned char gpio)
{	
	
    	if( !gpio_is_valid(gpio))
		return -1;

	/* fix IC bug */
	if ((68 == gpio) || (69 == gpio) || (70 == gpio) || (145 == gpio) || (146 == gpio))
	{
		HAL_GPIO_BIT_DIR_SET(gpio, HAL_GPIO_O_DIR);
	}
	else
	{
		HAL_GPIO_BIT_DIR_SET(gpio, HAL_GPIO_I_DIR);
	}
	
    
    	/* GPIO can never have been requested or set as {in,out}put */
    	return HAL_GPIO_BIT_GET(gpio);
}


int ali_gpio_set_value(unsigned char gpio, int value)
{	
    	if( !gpio_is_valid(gpio))
		return -1;

	/* fix IC bug */
	if ((68 == gpio) || (69 == gpio) || (70 == gpio) || (145 == gpio) || (146 == gpio))
	{
		HAL_GPIO_BIT_DIR_SET(gpio, HAL_GPIO_I_DIR);
	}
	else
	{
		HAL_GPIO_BIT_DIR_SET(gpio, HAL_GPIO_O_DIR);
	}
    	
    	/* GPIO can never have been requested or set as output */
    	HAL_GPIO_BIT_SET(gpio, (value)? 1 : 0);    

	 return 0;
}


static int m36_direction_input(struct gpio_chip *chip, unsigned offset)
{
	unsigned long flags;
	
	//printk("%s->%d\n", __FUNCTION__, offset);	
	
#ifdef ENABLE_M3701C_JTAG_DEBG
	if((offset == 17) || (offset == 18)
		|| (offset == 4) || (offset == 1)
		|| (offset == 32))
	return 0;
#endif	

	spin_lock_irqsave(&m36_gpio_lock, flags);
	/* fix IC bug */
	if ((__REG16ALI(0x18000002) == 0x3701) 
		&& ((68 == offset) || (69 == offset) || (70 == offset) || (145 == offset) || (146 == offset)))
	{
		HAL_GPIO_BIT_DIR_SET(offset, HAL_GPIO_O_DIR);		
	}
	else
	{
		HAL_GPIO_BIT_DIR_SET(offset, HAL_GPIO_I_DIR);
	}

	HAL_GPIO_FUNC_ENABLE(offset, HAL_GPIO_ENABLE);
	
	spin_unlock_irqrestore(&m36_gpio_lock, flags);

	return 0;
}


static int m36_direction_output(struct gpio_chip *chip, unsigned int offset, int value)
{
	unsigned long flags;

	//printk("%s->%d val->%d\n", __FUNCTION__, offset, value);
	
#ifdef ENABLE_M3701C_JTAG_DEBG
	if((offset == 17) || (offset == 18)
		|| (offset == 4) || (offset == 1)
		|| (offset == 32))
	return 0;
#endif

	spin_lock_irqsave(&m36_gpio_lock, flags);
	/* fix IC bug */
	if ((__REG16ALI(0x18000002) == 0x3701) 
		&& ((68 == offset) || (69 == offset) || (70 == offset) || (145 == offset) || (146 == offset)))
	{
		HAL_GPIO_BIT_DIR_SET(offset, HAL_GPIO_I_DIR);		
	}
	else
	{
		HAL_GPIO_BIT_DIR_SET(offset, HAL_GPIO_O_DIR);
	}

	HAL_GPIO_FUNC_ENABLE(offset, HAL_GPIO_ENABLE);

	if(value)
		HAL_GPIO_BIT_SET(offset, 1);
	else
		HAL_GPIO_BIT_SET(offset, 0);
	
	spin_unlock_irqrestore(&m36_gpio_lock, flags);
	
	return 0;
}


static int m36_gpio_get(struct gpio_chip *chip, unsigned offset)
{	
	unsigned long flags;
	int val;
	
	//printk("%s->%d\n", __FUNCTION__, offset);

#ifdef ENABLE_M3701C_JTAG_DEBG
	if((offset == 17) || (offset == 18)
		|| (offset == 4) || (offset == 1)
		|| (offset == 32))
	return 0;
#endif

	spin_lock_irqsave(&m36_gpio_lock, flags);


	/* fix IC bug */
	if ((__REG16ALI(0x18000002) == 0x3701) && (69 == offset))
	{		
		__REG32ALI(0x1805c057) |= (1<<1);		/*enable XPMU_GPIO[1] as PMU internal GPIO */
		val = (__REG32ALI(0x1805c054) >> 1) & 1;	/* read bit 1 */
		__REG32ALI(0x1805c057) &= ~(1<<1);		/*enable XPMU_GPIO[1] as PMU internal GPIO */		
	}
	else
	{
		val = HAL_GPIO_BIT_GET(offset);
	}
	
	spin_unlock_irqrestore(&m36_gpio_lock, flags);
	
	return val;
}

static void m36_gpio_set(struct gpio_chip *chip, unsigned offset, int val)
{
	unsigned long flags;

	//printk("%s->%d val->%d\n", __FUNCTION__, offset,val);

#ifdef ENABLE_M3701C_JTAG_DEBG
	if((offset == 17) || (offset == 18)
		|| (offset == 4) || (offset == 1)
		|| (offset == 32))
	return;
#endif

	spin_lock_irqsave(&m36_gpio_lock, flags);

	if(val)
		HAL_GPIO_BIT_SET(offset, 1);
	else
		HAL_GPIO_BIT_SET(offset, 0);
		
	spin_unlock_irqrestore(&m36_gpio_lock, flags);

}


#ifdef CONFIG_MIPS
int gpio_to_irq(unsigned gpio)
{
	return INT_ALI_GPIO;
}


int irq_to_gpio(unsigned irq)
{
	printk(KERN_ERR "m36_irq_to_gpio: not support(irq = %d)\n",irq);
	return -EINVAL;
}
#endif

void gpio_irq_enable(unsigned offset)
{
	unsigned long flags;
	
	//printk("%s->%d\n", __FUNCTION__, offset);

#ifdef ENABLE_M3701C_JTAG_DEBG
	if((offset == 17) || (offset == 18)
		|| (offset == 4) || (offset == 1)
		|| (offset == 32))
	return;
#endif

	spin_lock_irqsave(&m36_gpio_lock, flags);

	HAL_GPIO_INT_SET(offset, 1);	
		
	spin_unlock_irqrestore(&m36_gpio_lock, flags);
}

void gpio_irq_disable(unsigned offset)
{
	unsigned long flags;
	
#ifdef ENABLE_M3701C_JTAG_DEBG
	if((offset == 17) || (offset == 18)
		|| (offset == 4) || (offset == 1)
		|| (offset == 32))
	return;
#endif

	//printk("%s->%d\n", __FUNCTION__, offset);

	spin_lock_irqsave(&m36_gpio_lock, flags);

	HAL_GPIO_INT_SET(offset, 0);	
		
	spin_unlock_irqrestore(&m36_gpio_lock, flags);
}	

void gpio_irq_type(unsigned offset, unsigned trigger)
{
	unsigned long flags;

	//printk("%s->%d type->0x%x\n", __FUNCTION__, offset, trigger);

#ifdef ENABLE_M3701C_JTAG_DEBG
	if((offset == 17) || (offset == 18)
		|| (offset == 4) || (offset == 1)
		|| (offset == 32))
	return;
#endif

	spin_lock_irqsave(&m36_gpio_lock, flags);

	if(trigger & IRQ_TYPE_EDGE_RISING)
		HAL_GPIO_INT_REDG_SET(offset, 1);
	else
		HAL_GPIO_INT_REDG_SET(offset, 0);

	if(trigger & IRQ_TYPE_EDGE_FALLING)
		HAL_GPIO_INT_FEDG_SET(offset, 1);
	else
		HAL_GPIO_INT_FEDG_SET(offset, 0);
	
	spin_unlock_irqrestore(&m36_gpio_lock, flags);
}

void gpio_irq_clear(unsigned offset)
{
	unsigned long flags;

	//printk("%s->%d\n", __FUNCTION__, offset);
	
#ifdef ENABLE_M3701C_JTAG_DEBG
	if((offset == 17) || (offset == 18)
		|| (offset == 4) || (offset == 1)
		|| (offset == 32))
	return;
#endif

	spin_lock_irqsave(&m36_gpio_lock, flags);

	HAL_GPIO_INT_CLEAR(offset);	
		
	spin_unlock_irqrestore(&m36_gpio_lock, flags);
}


int gpio_irq_get_status(unsigned offset)
{
    unsigned long flags;
    int irq_status = 0;
    
#ifdef ENABLE_M3701C_JTAG_DEBG
	if((offset == 17) || (offset == 18)
		|| (offset == 4) || (offset == 1)
		|| (offset == 32))
	return 0;
#endif

	spin_lock_irqsave(&m36_gpio_lock, flags);
    irq_status = HAL_GPIO_INT_STA_GET(offset);
    spin_unlock_irqrestore(&m36_gpio_lock, flags);

    return irq_status;
}


static struct gpio_chip m36_gpio_chip = {
	.label			= "m36",
	.direction_input	= m36_direction_input,
	.direction_output	= m36_direction_output,
	.set			= m36_gpio_set,
	.get			= m36_gpio_get,
	.base			= 0,
	.ngpio			= GPIO_PORT_MAX + 1,
};

int __init m36_init_gpio(void)
{
	return gpiochip_add(&m36_gpio_chip);
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0))
#else
arch_initcall(m36_init_gpio);
#endif



