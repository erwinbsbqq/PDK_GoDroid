/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2010 - 2013 Copyright (C)
 *
 *  File: alimci_bsc.h
 *
 *  Description: SD/MMC Card Host driver for ALI 36XX/39XX platform
 *
 *  History:
 *          Date                    Author          Comment
 *          ====                    ======          =======
 * 1.       2013.11.02               David Chen   Board specific configure
 ****************************************************************************/
#ifndef _ALI_MCI_BSP_H_
#define _ALI_MCI_BSP_H_

/*chip DIE*/
#define M3921 0x3921
#define M3821 0x3821
#define M3503 0x3503

#if defined(CONFIG_ARM) && defined(CONFIG_MIPS)
    #error "Can not define CONFIG_ARM and CONFIG_MIPS at the same time."
#endif

#if !defined(CONFIG_ARM) && !defined(CONFIG_MIPS)
    #error "Neither CONFIG_ARM nor CONFIG_MIPS is defined."
#endif

#if defined(CONFIG_ARM)
    #include <linux/ali_interrupt.h>
    #include <linux/ali_reg.h>
    //#include <asm/mach-ali/m36_gpio.h>  //GPIO API had implemented in ali_gpio.c/.h for SDIO driver.
#else
    // TBD
#endif





#if defined(CONFIG_ARM)
    //The following parameters may be different according to SoC ID, Shuttle or C version.
    #define ALI_INIT_CLKDIV_S3921        0x3f
    #define ALI_TRAN_CLKDIV_S3921        0x9
    #define ALI_INIT_CLKDIV_C3921        0xff
    #define ALI_TRAN_CLKDIV_C3921        0x3

    #define GPIO_QFP_WP                         3   //write protect
    #define GPIO_QFP_CD                          4   //Card detect
    #define GPIO_QFP_PWR                       73  //power enable

    #define GPIO_BGA_WP                         79  //write protect
    #define GPIO_BGA_CD                          78  //Card detect
    #define GPIO_BGA_PWR                       73  //power enable

	#define GPIO_QFP_WP_C3921                    (-1)   //write protect
    #define GPIO_QFP_CD_C3921                    113   //Card detect

    #define GPIO_BGA_WP_C3921                    (-1)   //write protect
    #define GPIO_BGA_CD_C3921                    113   //Card detect
    
    #define CARD_DETECT_INVERT            TRUE

    #define BOOT_DEV(x)                                                        ((x>>15)&0xF)
	#define BOOT_DEV_NOR(x)                                               (((x)&0xC) == 0x0)
    #define BOOT_DEV_NF(x)                                                  (((x)&0xC) == 0x8)
    #define BOOT_DEV_SPI(x)                                                (((x)&0xC) == 0x4)  
    #define BOOT_DEV_EMMC(x)                                             (((x)&0xC) == 0xC) 
	
    #define CHIP_STRAP_PIN                  0x70
    #define SOC_PINMUX_REG1                 0x88
    #define         NF_FUNC_SEL                         (1 << 3)  //in SOC_PINMUX_REG1
    #define SOC_PINMUX_REG2                 0x8c
    #define         STRAPIN_SEL_ENABLE            (1 << 31)
    #define         SD_FUNC_SEL                         (1 << 5)  //in SOC_PINMUX_REG2
    
	 #define         eMMC_FUNC_SEL               (1 << 28)
	 
    #define LOCAL_DEV_RESET_CTRL_REG         0x80 //18000080:bit30
    #define     SDIO_SW_RESET   (1 << 30)
    #define LOCAL_CLK_GATE_CTRL_REG1         0x60
    #define     SDIO_M_CLK_GATE   (1 << 7)                     //0 for clock enable
    #define PAD_DRVING_CTRL_REG1         0x410 //bit 11-10  01-5.0mA 10-7.5mA 11-10mA
    #define     SDIO_5MA   (1 << 10)
    #define     SDIO_7d5MA   (2 << 10)
    #define     SDIO_10MA   (3 << 10)

    #define M3921_S                             0x0 // S3921
    #define M3921_C                             0x1 // C3921
    
    #define M3921_QFP256                        0x0 // LQFP256
    #define M3921_BGA445                        0x1 // BGA445

	

    //The following parameters should be same for all ARM platform.
    #define ALI_MCI_IRQ                         INT_ALI_SDIO
    #define ALI_ARM_GPIO_IRQ               INT_ALI_GPIO
    #define PHYS_SD_BASE_ADDR            0x18014000
    #define PHYS_SOC_BASE_ADDR          0x18000000

#else
    //The following parameters may be different according to SoC ID, Shuttle or C version.
    #define ALI_INIT_CLKDIV_C3821        0x88
    #define ALI_TRAN_CLKDIV_C3821       0x3

    #define GPIO_BGA_WP                         42  //write protect
    #define GPIO_BGA_CD                          41  //Card detect
    #define GPIO_BGA_PWR                       -1  //power enable

    //TODO: please check 3821 QFP package 
    #define GPIO_QFP256_WP                         -1   //write protect
    #define GPIO_QFP256_CD                          -1   //Card detect
    #define GPIO_QFP256_PWR                       -1  //power enable

    #define GPIO_QFP156_WP                         -1   //write protect
    #define GPIO_QFP156_CD                          -1   //Card detect
    #define GPIO_QFP156_PWR                       -1  //power enable

    #define M3821_BGA380                        0x0 // BGA380
    #define M3821_QFP256                        0x0 // LQFP256
    #define M3821_QFP156                        0x10 // LQFP256

    #define CARD_DETECT_INVERT            TRUE //?

    #define SOC_PINMUX_REG1                 0x488
    #define SOC_PINMUX_REG2                 0x48c
    #define SOC_PINMUX_REG3                 0x490
        #define         eMMC_SEL                         (1 << 18)  //in SOC_PINMUX_REG3
        #define         SD_8BIT_SEL                    (1 << 17)  //in SOC_PINMUX_REG3
        #define         SD_IF_SEL                         (1 << 16)  //in SOC_PINMUX_REG3
    #define SOC_PINMUX_REG4                 0x494

    #define SOC_STRAPINFO_REG                 0x70
    #define SOC_STRAPCTRL_REG                 0x74

    #define LOCAL_DEV_RESET_CTRL_REG         0x80 //18000080:bit30
    #define     SDIO_SW_RESET   (1 << 30)
    #define LOCAL_CLK_GATE_CTRL_REG1         0x60
    #define     SDIO_M_CLK_GATE   (1 << 1)                     //0 for clock enable
    //#define PAD_DRVING_CTRL_REG1         0x410 //bit 11-10  01   5.0ma 
    //#define     SDIO_5MA   (1 << 10)  //?

    //The following parameters should be same for all MIPS platform.

    /* CPU physical address -> DMA address */
    #define __CTDADDRALI(x)		(((unsigned long)x) & 0x7FFFFFFF)

    #define ALI_MCI_IRQ                           18
    #define ALI_MIPS_GPIO_IRQ               8
    #define PHYS_SD_BASE_ADDR            0xb8030000
    #define PHYS_SOC_BASE_ADDR          0xb8000000

    #define __REG8ALI(x)        (*((volatile unsigned char *)(x)))
    #define __REG32ALI(x)        (*((volatile unsigned long *)(x)))
    #define __REGALIRAW(x)        (x)
#endif


#endif
