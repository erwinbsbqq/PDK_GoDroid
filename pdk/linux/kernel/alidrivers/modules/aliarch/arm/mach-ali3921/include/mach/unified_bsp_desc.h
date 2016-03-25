#ifndef _UNIFIED_BSP_DESC_H_
#define _UNIFIED_BSP_DESC_H_

#include <mach/unified_bsp_board_attr.h>
#include <mach/unified_bsp_macro.h>

#ifndef M36_CHIP_NAME_LEN
	#define M36_CHIP_NAME_LEN	24
#endif

#ifndef M36_BOARD_NAME_LEN
	#define M36_BOARD_NAME_LEN	24
#endif

typedef struct unified_bsp_i2c_pin_desc_t {
	unsigned short clock_period;	
	/* gpio number for SDA Pin */
	unsigned int sda;	
	/* gpio number for SCL Pin */
	unsigned int scl;	
} i2c_pin_desc_t;

typedef struct unified_bsp_i2c_gpio_desc_t {
	/* I2C pins */
	i2c_pin_desc_t i2c_pin_desc[M36_I2C_GPIO_NUM];	
	/* I2C bus number */
	unsigned int i2c_gpio_num;	
} i2c_gpio_desc_t;

typedef struct unified_bsp_irq_desc_t {	/* non caller? */
	int sys_rpc_addr;
	unsigned int sys_rpc_mask;
	unsigned int sys_rpc_irq1_mask;
	unsigned int sys_rpc_irq2_mask;
} irq_desc_t;

typedef struct unified_bsp_i2c_desc_t {
	/* Pin reverse for some error of platform, such as ? */
	unsigned long pin_reverse; 
	unsigned long pin_pull_high;	
	/* I2C GPIO pins alloced */
	i2c_gpio_desc_t i2c_gpio_desc;	
} i2c_desc_t;

typedef struct unified_bsp_chip_desc_t {
	irq_desc_t irq_desc;
	i2c_desc_t i2c_desc;
} chip_desc_t;

typedef struct unified_bsp_board_desc_t {
	chip_desc_t chip_desc;
	board_attr_t board_attr;
} board_desc_t;

typedef enum {
	IO_REG8_BIT_SET = 0,
	IO_REG8_BITS_SET,
	IO_REG8_WRITE,
	IO_REG32_BIT_SET,
	IO_REG32_BITS_SET,
	IO_REG32_WRITE,
	SHORT_DELAY,
	OTP_SET_VDAC_FS,
	HDMI_ENABLE_HDCP,
} init_cmd_type_t;

typedef struct unified_bsp_init_cmd_t {
	init_cmd_type_t cmd;
	unsigned long addr;
	unsigned long bits_seleted;
	unsigned long val;
} init_cmd_t;

typedef struct unified_bsp_init_cmd_desc_t {
	unsigned long len;
	init_cmd_t *p_init_cmd_stream;
} init_cmd_desc_t;

typedef struct unified_bsp_porting_info_t {
	unsigned long total_length;
	unsigned long porting_info_length;
	unsigned long board_desc_length;
	unsigned long init_cmd_stream_length;
} porting_info_t;

typedef enum {
	CHIP = 0,
	IRQ,
	I2C,
} req_desc_t;

void board_porting_init(void);
init_cmd_desc_t * get_init_cmd_desc(void);
board_attr_t * get_board_attr(void);
chip_desc_t * get_chip_desc(void);
irq_desc_t * get_irq_desc(void);
i2c_desc_t * get_i2c_desc(void);
void * request_desc(req_desc_t req);

#endif
