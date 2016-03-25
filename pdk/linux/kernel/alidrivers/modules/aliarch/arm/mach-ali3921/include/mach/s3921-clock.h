/* arch/arm/mach-ali3921/include/mach/s3921-clock.h
 *
 * Copyright 2013 ALI Tech
 *
 * S3921 clock register definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __MACH_S3921_CLOCK_H
#define __MACH_S3921_CLOCK_H

/*#include "ali-s3921.h"*/
#include <mach/ali-s3921.h>

#include <asm/io.h>

/* Define the return  SUCCESS/FAIL code */
#define S3921_CLK_SUCCESS              0
#define S3921_CLK_FAIL                      1

/*ALI S3921 clock registers address macro definition*/
#define S3921_CLKREG(x)                     (PHYS_SYSTEM + (x))


#define S3921_STRAP_INFO_CLOCK     S3921_CLKREG(0x70)
#define S3921_STRAP_CTRL_CLOCK     S3921_CLKREG(0x74)
#define S3921_CPU_CTRL_CLOCK         S3921_CLKREG(0x94)
#define S3921_PLL_CTRL_CLOCK		 S3921_CLKREG(0x94)
#define S3921_CA9_CTRL_CLOCK		 S3921_CLKREG(0x0C)


#define CA9_Clock_DIV_1                 1
#define CA9_Clock_DIV_2                 2
#define CA9_Clock_DIV_4                 4
#define CA9_Clock_BYPASS_CLK     3



/* Strap Pin control register  shift bits definition for setting function  */
#define BYPASS_EN_TRIG              (0x1 << 26)
#define CPLL_FOUTSEL_TRIG        (0x1 << 22)
#define BYPASS_EN                        (0x1 << 14)
#define CPLL_FOUTSEL_800M       (0x0 << 8)
#define CPLL_FOUTSEL_900M       (0x1 << 8)
#define CPLL_FOUTSEL_1000M     (0x2 << 8)
#define CPLL_FOUTSEL_1100M     (0x3 << 8)
#define CPLL_FOUTSEL_1200M     (0x4 << 8)
#define CPLL_FOUTSEL_1300M     (0x5 << 8)
#define CPLL_FOUTSEL_1400M     (0x6 << 8)
#define CPLL_FOUTSEL_1500M     (0x7 << 8)


/* CPU control  register shift bits definition for setting function  */
#define PHERI_CLK_RATIO               (0x01 << 31)
#define ACP_CLK_RATIO                   (0x01 << 30)
#define PL310_CLK_RATIO_00	    (0x00 << 28)
#define PL310_CLK_RATIO_01	    (0x01 << 28)
#define PL310_CLK_RATIO_10	    (0x02 << 28)
#define PL310_CLK_RATIO_11       (0x03 << 28)
#define L2_TCLK_RATIO_00            (0x00<<26)
#define L2_TCLK_RATIO_01            (0x01<<26)
#define L2_TCLK_RATIO_10            (0x02<<26)
#define L2_TCLK_RATIO_11            (0x03<<26)
#define L2_DCLK_RATIO_00            (0x00<<24)
#define L2_DCLK_RATIO_01            (0x01<<24)
#define L2_DCLK_RATIO_10            (0x02<<24)
#define L2_DCLK_RATIO_11            (0x03<<24)
#define PL310_CLK_RATIO_M00	    (0x00<<22)
#define PL310_CLK_RATIO_M01	    (0x01<<22)
#define PL310_CLK_RATIO_M10	    (0x02<<22)
#define PL310_CLK_RATIO_M11     (0x03<<22)
#define TS_EN                                    (0x01<<20)
#define DBGCLK_ON                          (0x01<<19)
#define ACLK_ON                               (0x01<<18)
#define PLL_DIV_RATIO_00	            (0x0<<11)
#define PLL_DIV_RATIO_01            (0x1<<11)
#define PLL_DIV_RATIO_10	            (0x2<<11)
#define PLL_DIV_RATIO_11	            (0x3<<11)

/* PLL Control Register shift bits definition for setting function  */
#define CPLL_TFCKSEL_00             (0x0 << 5)
#define CPLL_TFCKSEL_01             (0x1 << 5)
#define CPLL_TFCKSEL_10             (0x2 << 5)
#define CPLL_TFCKSEL_11             (0x3 << 5)

#define CPLL_CP_00                       (0x0 << 3)
#define CPLL_CP_01                       (0x1 << 3)
#define CPLL_CP_10                       (0x2 << 3)
#define CPLL_CP_11                       (0x3 << 3)

#define CPLL_AFCPD_0                  (0x0 << 1)
#define CPLL_AFCPD_1                  (0x1 << 1)

#define CPLL_FINSEL_0                 (0x0 << 7)
#define CPLL_FINSEL_1                 (0x1 << 7)

#define CPLL_PD_0                         0x0
#define CPLL_PD_1                         0x1

#define CPLL_TFCKPD_0                (0x0 << 2)
#define CPLL_TFCKPD_1                (0x1 << 2)

#if 1

/* Define the cpll  output frequency selection enum */
enum CPLL_FOUT_SELECT
{
     CPLL_FOUT_SELECT_BEGIN =0,
    CPLL_FOUTSEL_SELECT_800M  = CPLL_FOUT_SELECT_BEGIN,   
    CPLL_FOUTSEL_SELECT_900M,                                                    
    CPLL_FOUTSEL_SELECT_1000M,      
    CPLL_FOUTSEL_SELECT_1100M,         
    CPLL_FOUTSEL_SELECT_1200M,                                                 
    CPLL_FOUTSEL_SELECT_1300M,
    CPLL_FOUTSEL_SELECT_1400M,
    CPLL_FOUTSEL_SELECT_1500M,
    CPLL_FOUT_SELECT_END
};


/* Define the cpll  output frequency selection enum */
enum CA9_CLOCK_SELECT
{
    CA9_CLOCK_SELECT_BEGIN =0,
    CA9_CLOCK_SELECT_200M  = CA9_CLOCK_SELECT_BEGIN,   
    CA9_CLOCK_SELECT_225M,
    CA9_CLOCK_SELECT_250M,
    CA9_CLOCK_SELECT_275M,
    CA9_CLOCK_SELECT_300M,
    CA9_CLOCK_SELECT_325M,
    CA9_CLOCK_SELECT_350M,
    CA9_CLOCK_SELECT_375M,
    
    CA9_CLOCK_SELECT_400M,
    CA9_CLOCK_SELECT_450M,
    CA9_CLOCK_SELECT_500M,
    CA9_CLOCK_SELECT_550M,
    CA9_CLOCK_SELECT_600M,
    CA9_CLOCK_SELECT_650M,
    CA9_CLOCK_SELECT_700M,
    CA9_CLOCK_SELECT_750M,

     CA9_CLOCK_SELECT_800M,
    CA9_CLOCK_SELECT_900M,
    CA9_CLOCK_SELECT_1000M,
    CA9_CLOCK_SELECT_1100M,
    CA9_CLOCK_SELECT_1200M,
    CA9_CLOCK_SELECT_1300M,
    CA9_CLOCK_SELECT_1400M,
    CA9_CLOCK_SELECT_1500M,
    CA9_CLOCK_SELECT_END

};

#endif

#if 0

/* Define the cpll  output frequency selection enum */
enum CPLL_FOUT_SELECT
{
     CPLL_FOUT_SELECT_BEGIN =0,
    CPLL_FOUTSEL_SELECT_800M  = CPLL_FOUT_SELECT_BEGIN,   
    CPLL_FOUTSEL_SELECT_900M,                                                    
    CPLL_FOUTSEL_SELECT_1000M,      
    CPLL_FOUT_SELECT_END
};


/* Define the cpll  output frequency selection enum */
enum CA9_CLOCK_SELECT
{
    CA9_CLOCK_SELECT_BEGIN =0,
    CA9_CLOCK_SELECT_200M  = CA9_CLOCK_SELECT_BEGIN,   
    CA9_CLOCK_SELECT_225M,
    CA9_CLOCK_SELECT_250M,
    
    CA9_CLOCK_SELECT_400M,
    CA9_CLOCK_SELECT_450M,
    CA9_CLOCK_SELECT_500M,

     CA9_CLOCK_SELECT_800M,
    CA9_CLOCK_SELECT_900M,
    CA9_CLOCK_SELECT_1000M,
    CA9_CLOCK_SELECT_END
};

#endif

#undef  readb
#undef  readw
#undef  readl

#undef  writeb
#undef  writew
#undef  writel




/* register read/write operations */



#define readb(addr)            (*(volatile unsigned char *)(ioremap((addr), sizeof(unsigned char))))
#define readw(addr)           (*(volatile unsigned short *)(ioremap((addr), sizeof( unsigned short))))
#define readl(addr)             (*(volatile unsigned int *)(ioremap((addr), sizeof(unsigned int))))

#define writeb(b,addr)       (*(volatile unsigned char *)(ioremap((addr), sizeof(unsigned char)))) = (b)
#define writew(b,addr)      (*(volatile unsigned short *)(ioremap((addr), sizeof( unsigned short)))) = (b)
#define writel(b,addr)        (*(volatile unsigned int *)(ioremap((addr), sizeof(unsigned int))))= (b)


/*
#define readb            __raw_readb;
#define readw           __raw_readw;
#define readl              __raw_readl; 

#define writeb           __raw_writeb
#define writew          __raw_writew
#define writel            __raw_writel
*/


extern void s3921_setup_clocks(void);




/* s3921_register_clocks - register clocks for s3921 */
extern void s3921_register_clocks(void);



#endif /* __MACH_S3921_CLOCK_H */

