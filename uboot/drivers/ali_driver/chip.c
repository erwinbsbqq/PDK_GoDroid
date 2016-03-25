#include <common.h>
#include <ali/sys_define.h>
#include <ali/retcode.h>
#include <ali/basic_types.h>
#include <ali/osal.h>

#ifdef  ALI_ARM_STB
#define ALI_SOC_BASE			              0x18000000	// 3921 arm
#else
#define ALI_SOC_BASE			              0xb8000000	// mips
#endif

static UINT32 reg_88, reg_ac, reg_2e000, reg_70;

UINT8 readb(UINT32 addr)
{
	return *(const volatile UINT8 *) addr;
}

UINT16 readw(UINT32 addr)
{
	return *(const volatile UINT16 *) addr;
}

UINT32 readl(UINT32 addr)
{
	return *(const volatile UINT32 *) addr;
}

void writeb(UINT8 b, UINT32 addr)
{
	*(volatile UINT8 *) addr = b;
}

void writew(UINT16 b, UINT32 addr)
{
	*(volatile UINT16 *) addr = b;
}

void writel(UINT32 b, UINT32 addr)
{
	*(volatile UINT32 *) addr = b;
}

UINT32 sys_ic_get_chip_id()   
{  
    unsigned int id = readl(ALI_SOC_BASE)  >> 16;
    if( 0x3701 == id )
        return ALI_C3701;
    else if(0x3503 == id)
		return ALI_S3503;
	else if(0x3603 == id)
        return ALI_S3602F;
	else if(0x3901 == id)
        return ALI_S3901;
	else if(0x3921 == id)
        return ALI_S3921;
    else
		return 0 ;
}

UINT32 sys_ic_get_rev_id()   
{
    unsigned int id = readl(ALI_SOC_BASE);
    if((id&0xff) == 0)
    	return IC_REV_0;
    else
    	return IC_REV_1;
}

UINT32 sys_ic_get_cpu_clock()
{
	UINT32  reg ;
	unsigned long strap_pin_reg, cpu_clock;
	
	if( sys_ic_get_chip_id() == ALI_C3701 ){
		reg = readl(ALI_SOC_BASE+0x70) & (0x7<<7) ;
		if(reg == 0 )
			return 600 ;
		else if(reg == 1)
			return 450 ;
		else if(reg == 2)
			return 396 ;
		else if(reg == 3)
			return 297 ;
		else
			return 396 ;

	}else if( sys_ic_get_chip_id() == ALI_S3503 ){
		reg = readl(ALI_SOC_BASE+0x70) & (0x3<<7) ;
		if(reg == 0 )
			return 594 ;
		else if(reg == 1)
			return 450 ;
		else if(reg == 2)
			return 396 ;
		else 
			return 550 ;

	}else if ( sys_ic_get_chip_id() == ALI_S3921 )
	{
		strap_pin_reg = readl(ALI_SOC_BASE+0x70);
		strap_pin_reg = (strap_pin_reg>>8)&0x07;
		if(strap_pin_reg == 0)
		{
			cpu_clock = 800;	// 800MHz
		}
		else if(strap_pin_reg == 1)
		{
			cpu_clock = 900;	// 900MHz
		}
		else if(strap_pin_reg == 2)
		{
			cpu_clock = 1000;	// 1000MHz
		}
		else if(strap_pin_reg == 3)
		{
			cpu_clock = 1100;	// 1100MHz
		}
		else if(strap_pin_reg == 4)
		{
			cpu_clock = 1200;	// 1200MHz
		}
		else if(strap_pin_reg == 5)
		{
			cpu_clock = 1300;	// 1300MHz
		}
		else if(strap_pin_reg == 6)
		{
			cpu_clock = 1400;	// 1400MHz
		}
		else if(strap_pin_reg == 7)
		{
			cpu_clock = 1500;	// 1500MHz
		} 
		return cpu_clock;
	}
		return 0;
	
}

UINT32 sys_ic_get_SEE_clock()
{
	UINT32  reg ;
	if( sys_ic_get_chip_id() == ALI_C3701 ){
		reg = readl(ALI_SOC_BASE+0x7c) & (0x3<<26) ;
		if(reg == 0 )
			return 456 ;
		else if(reg == 1)
			return 396 ;
		else if(reg == 2)
			return 297 ;
		else if(reg == 3)
			return 297 ;
	}else if( sys_ic_get_chip_id() == ALI_S3503 ){
		reg = readl(ALI_SOC_BASE+0x7C) & (0x3<<26) ;
		if(reg == 0 )
			return 450 ;
		else if(reg == 1)
			return 396 ;
		else if(reg == 2)
			return 337 ;

	}else if( sys_ic_get_chip_id() == ALI_S3921 ){
		reg = readl(ALI_SOC_BASE+0x7C)  ;
		reg = (reg>>20)&0x03;
		if(reg == 0 )
			return 594 ;
		else if(reg == 1)
			return 450 ;
		else if(reg == 2)
			return 396 ;

	}else
		return 0;
	
}

UINT32 sys_ic_get_mem_clock()
{
	UINT32  reg ;
	unsigned long strap_pin_reg, dram_clock;
	
	if( sys_ic_get_chip_id() == ALI_C3701 ){
		reg = readl(ALI_SOC_BASE+0x70) & (0x3<<5) ;
		if(reg == 0 )
			return 166*8 ;
		else if(reg == 1)
			return 133*8 ;
		else if(reg == 2)
			return 100*8 ;
		else if(reg == 3)
			return 20 ;
	}else if( sys_ic_get_chip_id() == ALI_S3503 ){
		reg = readl(ALI_SOC_BASE+0x70) & (0x7<<4) ;
		if(reg == 0 )
			return 20 ;
		else if(reg == 1)
			return 264*2 ;
		else if(reg == 2)
			return 475*2 ;
		else if(reg == 3)
			return 370*2 ;
		else if(reg == 4)
			return 514*2 ;
		else if(reg == 5)
			return 554*2 ;
		else if(reg == 6)
			return 316*2 ;
		else if(reg == 7)
			return 396*2 ;
		
	}else if ( sys_ic_get_chip_id() == ALI_S3921 )    {
		strap_pin_reg = readl(ALI_SOC_BASE+0x70);
		strap_pin_reg = (strap_pin_reg>>5)&0x07;
		if(strap_pin_reg == 0)
		{
			dram_clock = 33*2;	// bypass
		}
		else if(strap_pin_reg == 1)
		{
			dram_clock = 264*2;	// 528Mbps
		}
		else if(strap_pin_reg == 2)
		{
			dram_clock = 330*2;	// 688Mbps
		}
		else if(strap_pin_reg == 3)
		{
			dram_clock = 396*2;	// 800Mbps
		}
		else if(strap_pin_reg == 4)
		{
			dram_clock = 528*2;	// 1066Mbps
		}
		else if(strap_pin_reg == 5)
		{
			dram_clock = 660*2;	// 1333Mbps
		}
		else
		{
			dram_clock = 792*2;	// 1600Mbps
		}
		return dram_clock;
    	
    }else
		return 0;
	
}

UINT32 sys_ic_get_mem_size()
{
	UINT32  reg ;
	unsigned long data, dram_size;
	
	if( (sys_ic_get_chip_id() == ALI_C3701) || \
		(sys_ic_get_chip_id() == ALI_S3503)){
		reg = readl(ALI_SOC_BASE+0x1000) & (0x7) ;
		if(reg == 3 )
			return 128 ;
		else if(reg == 4)
			return 256 ;
		else if(reg == 5)
			return 512 ;		
	}else if (sys_ic_get_chip_id() == ALI_S3921)   {
		data = readl(ALI_SOC_BASE+0x1000) & 0x07;				
		if(data == 0)
		{
			dram_size = 32;		// 32M
		}
		else if(data == 1)
		{
			dram_size = 64;		// 64M
		}
		else if(data == 2)
		{
			dram_size = 128;	// 128M
		}
		else if(data == 3)
		{
			dram_size = 256;	// 256M
		}
		else if(data == 4)
		{
			dram_size = 512;	// 512M
		}
		else if(data == 5)
		{
			dram_size = 1024;	// 1024M
		}
		else if(data == 6)
		{
			dram_size = 128;	// 128M
		}
		else
		{
			dram_size = 2048;	// 2048M
		}    	
		return dram_size;
    }else
		return 0;	
}
#if 1
INT32 sys_ic_current_boot_type()
{
	UINT32 chip_id, chip_package, reg;
	chip_id = sys_ic_get_chip_id();
    chip_package = (UINT8)(readl(ALI_SOC_BASE) >> 8) & 0x0F;	

	reg = readl(ALI_SOC_BASE + 0x70);

	if ((ALI_C3701 == chip_id) || ((ALI_S3901 == chip_id) && (0x2 == chip_package)))
	{
		if( reg & (0x1<<18))
			return BOOT_TYPE_NAND;
		else
			return BOOT_TYPE_NOR;
			
	}else if(ALI_S3503 == chip_id){		
		if(( reg &(0x1<<18)) &&(!(reg &(0x1<<17))))
			return BOOT_TYPE_NAND;
		else if( !(reg &(0x1<<18)) && (reg &(0x1<<17)) )
			return BOOT_TYPE_NOR; /*intern nor*/
		else if( (!(reg &(0x1<<18))) && (!(reg &(0x1<<17))) )
			return BOOT_TYPE_NOR; /*extern nor*/
		else 
			return -1;
	}else
		return -1 ;
	
}

UINT32 sys_ic_change_boot_type(UINT32 type)
{
    UINT32 chip_id, chip_package;
    UINT32 tmp32;

    MUTEX_ENTER();		// mutex
    chip_id = sys_ic_get_chip_id();
    chip_package = (UINT8)(readl(ALI_SOC_BASE) >> 8) & 0x0F;	

    switch(type)
    {
        case BOOT_TYPE_NOR:
            if ((ALI_C3701 == chip_id) || ((ALI_S3901 == chip_id) && (0x2 == chip_package)))
            {// chip package 0x2 is M3701G
                reg_70 = readl(ALI_SOC_BASE + 0x70);
            	tmp32 = reg_70;
                tmp32 &= ~(1<<18);
            	tmp32 |= (1<<30);
            	writel(tmp32, ALI_SOC_BASE + 0x74);
            }
            else if(ALI_S3503 == chip_id)
            {   
                reg_70 = readl(ALI_SOC_BASE + 0x70);
                tmp32 = reg_70;
                tmp32 &= ~(1<<18);
                tmp32 |= ((1<<30)|(1<<29)|(1<<24)); // trigger bit
                writel(tmp32, ALI_SOC_BASE + 0x74);
            }

            break;
        case BOOT_TYPE_NAND:
            if ((ALI_C3701 == chip_id) || ((ALI_S3901 == chip_id) && (0x2 == chip_package)))
            {// chip package 0x2 is M3701G
                reg_70 = readl(ALI_SOC_BASE + 0x70);
            	tmp32 = reg_70;
            	tmp32 |= ((1<<18)|(1<<30));
            	writel(tmp32, ALI_SOC_BASE + 0x74);
            }
            else if((ALI_S3602F == chip_id )&&((0x0b == chip_package)||(0x7 == chip_package)))
            {// chip package 0x0b is M3606, chip package 0x7 is M3701E, 
            	reg_88 = readl(ALI_SOC_BASE+0x88);
            	reg_ac = readl(ALI_SOC_BASE+0xAC);
            	reg_2e000 = readl(ALI_SOC_BASE+0x2e000);

            	tmp32 = readl(ALI_SOC_BASE+0x88);
            	tmp32 &= ~((1<<0)|(1<<1)|(1<<20));	//CI UART2
            	tmp32 &= ~((3<<20)|(3<<16));	//URAT2 _216_SEL, UART2_BGA_SEL
            	writel(tmp32,ALI_SOC_BASE+0x88);
            	
            	tmp32 = readl(ALI_SOC_BASE+0xAC);
            	tmp32 |= (1<<22) | (1<<24);	
            	tmp32 &= ~1;		// SFLASH CS1 SEL(216PIN) 0: GPIO[79] 1: SFLASH CS[1]
            	writel(tmp32,ALI_SOC_BASE+0xAC);
            	//flash reg CPU_CTRL_DMA
            	//bit8: PIO_arbit_fuc_en 1 sflash/pflash/ci can share the bus with flash arbiter
            	//bit9:cpu_set_arbit_en 
            	//bit12:10 cpu_set_arbit_en 001 sflash is enable 010 pflash is enable 100 CI is enable
            	tmp32 = readl(ALI_SOC_BASE+0x2E000);
            	tmp32 &= ~(0x00001F00);	
            	tmp32 |= 0x00001200 ;	
            	writel(tmp32, ALI_SOC_BASE+0x2E000);
            }
            else if(ALI_S3503 == chip_id)
            {   
                reg_70 = readl(ALI_SOC_BASE + 0x70);
                tmp32 = reg_70;
                tmp32 |= (1<<18);
                tmp32 &= ~((1<<17)|(1<<15));
                tmp32 |= ((1<<30)|(1<<29)|(1<<24)); // trigger bit
                writel(tmp32, ALI_SOC_BASE + 0x74);
            }

            break;
        default:
            return RET_FAILURE;
    }

	return RET_SUCCESS;
}

UINT32 sys_ic_revert_boot_type()
{
	UINT32 chip_id,chip_package;
	UINT32 tmp32;

    chip_id = sys_ic_get_chip_id();
    chip_package = (UINT8)(readl(ALI_SOC_BASE) >> 8) & 0x0F;	

    if ((ALI_C3701 == chip_id) || ((ALI_S3901 == chip_id) && (0x2 == chip_package)))
    {// chip package 0x2 is M3701G
        tmp32 = reg_70;
		tmp32 |= 1<<30;
		writel(tmp32, ALI_SOC_BASE + 0x74); 
    }
    else if((ALI_S3602F == chip_id )&&((0x0b == chip_package)||(0x7 == chip_package)))
    {// chip package 0x0b is M3606, chip package 0x7 is M3701E, 
		writel(reg_88, ALI_SOC_BASE+0x88);
		writel(reg_ac, ALI_SOC_BASE+0xac);
		writel(reg_2e000, ALI_SOC_BASE+0x2E000);
    }
    else if(ALI_S3503 == chip_id)
    {   
        tmp32 = reg_70;
        tmp32 |= ((1<<30)|(1<<29)|(1<<24));
        writel(tmp32, ALI_SOC_BASE + 0x74);
    }

	MUTEX_LEAVE();		// mutex
	return RET_SUCCESS;
}
/*
RET_CODE sys_ic_get_pan_gpio(struct pan_gpio_info *pan_gpio)
{
	UINT32 chip_id;
	UINT32 tmp32;

    chip_id = sys_ic_get_chip_id();
    if (ALI_C3701 == chip_id)
    {
        pan_gpio[0].polar = 1;
        pan_gpio[0].io = HAL_GPIO_O_DIR;
        pan_gpio[0].position = 68;

        pan_gpio[1].polar = 1;
        pan_gpio[1].io = HAL_GPIO_O_DIR;
        pan_gpio[1].position = 69;

        pan_gpio[2].polar = 0;
        pan_gpio[2].io = HAL_GPIO_O_DIR;
        pan_gpio[2].position = 20;
    }
    else if(ALI_S3503 == chip_id)
    {   
        pan_gpio[0].polar = 1;
        pan_gpio[0].io = HAL_GPIO_O_DIR;
        pan_gpio[0].position = 134;

        pan_gpio[1].polar = 1;
        pan_gpio[1].io = HAL_GPIO_O_DIR;
        pan_gpio[1].position = 135;

        pan_gpio[2].polar = 0;
        pan_gpio[2].io = HAL_GPIO_O_DIR;
        pan_gpio[2].position = 20;
    }
    else
    {
        return RET_FAILURE;
    }

	return RET_SUCCESS;
}
*/
#endif

void stop_watchdog()
{
	*(volatile unsigned char *)(ALI_SOC_BASE + 0x18504) = 0;	
}
static void wdt_reboot_from_nand(void)
{     
     *(volatile unsigned int *)(ALI_SOC_BASE + 0x74) |= 1<<18;  
     *(volatile unsigned int *)(0x18000074) |= 1<<30;      
}

static void wdt_reboot_from_nor(void)
{   
   *(volatile unsigned int *)(ALI_SOC_BASE+0x74) &= 0xFFFBFFFF;       
   *(volatile unsigned int *)(ALI_SOC_BASE+0x74) |= 1<<30;  
}

void hw_watchdog_reboot()
{
#if 0
#ifdef NAND_BOOT
    sys_ic_change_boot_type(BOOT_TYPE_NAND);
#else
    sys_ic_change_boot_type(BOOT_TYPE_NOR);
#endif
#endif
   *(volatile unsigned int *)(ALI_SOC_BASE+0x18500) = 0xfffff000;    
   *(volatile unsigned int *)(ALI_SOC_BASE+0x18504) = 0x67;  
   do{}while(1);
}

void ali_see_stop()
{
    *(volatile unsigned int *)(ALI_SOC_BASE+0x40038) =0 ; // disable see interrupt
    *(volatile unsigned int *)(ALI_SOC_BASE+0x4003C) =0 ; // disable see interrupt
    osal_delay(10);
    *(volatile unsigned int *)(ALI_SOC_BASE+0x220) &=~(0x1 <<1) ;// dis see 
    *(volatile UINT32*)(ALI_SOC_BASE+0x20C) |= 1 ; // cold boot up flag 
}



