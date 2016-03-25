#ifndef _UNIFIED_BSP_MACRO_H	/* Start */
#define _UNIFIED_BSP_MACRO_H

#include <linux/delay.h>
#ifndef KERNEL_HEAHP_AREA_SECTIONS
	#define MM_KERNEL_HEAP_AREA_SECTIONS
#endif
/* 
 * board id is 32bit variable, and different contious bits has different usage, just like below:
 * [7 : 0]:	board version, like 01V03....
 * [15: 8]: soc type, like M3616,M3713....
 * [23:16]: chip type, like C3603,C3701C....
 * [31:24]: board usage, like DEMO,SORTING....
 * 
 */
#ifndef BOARD_VERSION_BITS_MASK
	#define BOARD_VERSION_BITS_MASK	((unsigned long)0xff)
#endif

#ifndef SOC_TYPE_BITS_MASK
	#define SOC_TYPE_BITS_MASK	((unsigned long)0xff00)
#endif

#ifndef CHIP_TYPE_BITS_MASK
	#define CHIP_TYPE_BITS_MASK		((unsigned long)0xff0000)
#endif

#ifndef BOARD_USAGE_BITS_MASK		
	#define BOARD_USAGE_BITS_MASK	((unsigned long)0xff000000)
#endif

#ifndef GET_BOARD_VERSION
	#define GET_BOARD_VERSION(board_id)		(board_id & BOARD_VERSION_BITS_MASK)
#endif

#ifndef GET_SOC_TYPE
	#define GET_SOC_TYPE(board_id)		(board_id & SOC_TYPE_BITS_MASK)
#endif

#ifndef GET_CHIP_TYPE
	#define GET_CHIP_TYPE(board_id)			(board_id & CHIP_TYPE_BITS_MASK)
#endif

#ifndef GET_BOARD_USAGE
	#define GET_BOARD_USAGE(board_id)		(board_id & BOARD_USAGE_BITS_MASK)
#endif


#ifndef BOARD_VERSION_01V01			/* board type supported */
	#define BOARD_VERSION_01V01		0x1
#endif
#ifndef BOARD_VERSION_01V02			
	#define BOARD_VERSION_01V02		0x2
#endif
#ifndef BOARD_VERSION_01V03	
	#define BOARD_VERSION_01V03		0x3
#endif

#ifndef SOC_TYPE_M3606		/* soc type supported */
	#define SOC_TYPE_M3606	0x100
#endif
#ifndef SOC_TYPE_M3616
	#define SOC_TYPE_M3616	0x200
#endif
#ifndef SOC_TYPE_M3713
	#define SOC_TYPE_M3713	0x300
#endif
#ifndef SOC_TYPE_M3516		
	#define SOC_TYPE_m3516	0x400
#endif

#ifndef CHIP_TYPE_C3603			/* chip type supported */
	#define CHIP_TYPE_C3603		0x10000
#endif
#ifndef CHIP_TYPE_M3701C
	#define CHIP_TYPE_M3701C	0x20000
#endif

#ifndef BOARD_USAGE_DEMO		/* board usage supported */
	#define BOARD_USAGE_DEMO	0x1000000
#endif
#ifndef BOARD_USAGE_SORTING
	#define BOARD_USAGE_SORTING	0x2000000
#endif

/* 
 * Boards and Chips that ALi supports
 */
#ifndef M3616_01V01_DEMO			/* board type supported */
	#define M3616_01V01_DEMO	(CHIP_TYPE_M3701C | SOC_TYPE_M3616 | BOARD_VERSION_01V01 | BOARD_USAGE_DEMO)		
#endif
#ifndef M3616_01V02_DEMO			
	#define M3616_01V02_DEMO	(CHIP_TYPE_M3701C | SOC_TYPE_M3616 | BOARD_VERSION_01V02 | BOARD_USAGE_DEMO)
#endif
#ifndef M3616_01V03_DEMO	
	#define M3616_01V03_DEMO	(CHIP_TYPE_M3701C | SOC_TYPE_M3616 | BOARD_VERSION_01V03 | BOARD_USAGE_DEMO)	
#endif

#ifndef M3713_01V01_DEMO
	#define M3713_01V01_DEMO	(CHIP_TYPE_M3701C | SOC_TYPE_M3713 | BOARD_VERSION_01V01 | BOARD_USAGE_DEMO)
#endif
#ifndef M3713_01V02_DEMO
	#define M3713_01V02_DEMO	(CHIP_TYPE_M3701C | SOC_TYPE_M3713 | BOARD_VERSION_01V02 | BOARD_USAGE_DEMO)
#endif
#ifndef M3713_01V03_DEMO
	#define M3713_01V03_DEMO	(CHIP_TYPE_M3701C | SOC_TYPE_M3713 | BOARD_VERSION_01V03 | BOARD_USAGE_DEMO)
#endif

#ifndef I2C_ERROR_BASE /* For I2C BUS */
	#define I2C_ERROR_BASE		-200
#endif
#ifndef ERR_I2C_SCL_LOCK
	#define ERR_I2C_SCL_LOCK	(I2C_ERROR_BASE - 1)	/* I2C SCL be locked */
#endif
#ifndef ERR_I2C_SDA_LOCK
	#define ERR_I2C_SDA_LOCK	(I2C_ERROR_BASE - 2)	/* I2C SDA be locked */
#endif
#ifndef ERR_I2C_NO_ACK
	#define ERR_I2C_NO_ACK		(I2C_ERROR_BASE - 3)	/* I2C slave no ack */
#endif
#ifndef I2C_RET_SUCCESS
	#define I2C_RET_SUCCESS 0
#endif
#ifndef I2C_RET_FAILURE 
	#define I2C_RET_FAILURE		-9
#endif
#ifndef ERR_TIME_OUT
	#define ERR_TIME_OUT    -34     /* Waiting time out */
#endif
#ifndef I2C_GPIO_NUM
	#define I2C_GPIO_NUM			2	//for hdmi - cmchen
#endif
#ifndef I2C_GPIO_TIMES_OUT
	#define I2C_GPIO_TIMES_OUT		10
#endif
#ifndef PIN_LEVEL_HIGH
	#define PIN_LEVEL_HIGH	1
#endif
#ifndef PIN_LEVEL_LOW
	#define PIN_LEVEL_LOW	0
#endif
#ifndef I2C_PIN_SDA
	#define I2C_PIN_SDA	0
#endif
#ifndef I2C_PIN_SCL
	#define I2C_PIN_SCL	1	
#endif
#ifndef M36_I2C_GPIO_NUM
	#define M36_I2C_GPIO_NUM	2
#endif 				/* For I2C BUS END */
#ifndef GPIO_DELAY 
	#define GPIO_DELAY(us)  udelay(us)//usleep(us)
#endif
#ifndef BIT_NON_EXIST
	#define BIT_NON_EXIST	0xffffffff
#endif
#ifndef REG32_BITS_LIMIT
	#define REG32_BITS_LIMIT	32
#endif

#endif	/* End */
