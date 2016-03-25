#include <ali/sys_define.h>
#include <ali/retcode.h>
#include <ali/basic_types.h>
#include <ali/osal.h>

#define IO_PRIVATE_REG1             0xB8000110
#define IO_PRIVATE_REG2             0xB8042074

#define OTP_BASE_ADDR_V1    0xb8000100     //C3603/C3281
#define OTP_0X2_V1    0x24     //C3603/C3281
#define OTP_0X3_V1    0x28     //C3603/C3281
#define OTP_0X5F_V1    0x2c     //C3603/C3281
#define OTP_BASE_ADDR_V2    0xb8042000     //C3701C/C3503 and other chipset
#define OTP_0X2_V2    0x40    
#define OTP_0X3_V2    0x44  
#define OTP_0X5F_V2    0x48  
#define OTP_0X80_V2    0x4c    
#define OTP_0X81_V2    0x50  
#define OTP_0X82_V2    0x6c  
#define OTP_0X83_V2    0x70    
#define OTP_0XDC_V2    0x78
#define OTP_0XDD_V2    0x7c
#define OTP_0XE4_V2    0x80
#define OTP_0XE5_V2    0x84
#define OTP_0XE6_V2    0x88
#define OTP_0XE7_V2    0x8c
#define OTP_0X8C_V2    0x90
#define OTP_0X8D_V2    0x94



#define C3503B_REV 2
#define KL_0_INDEX 0
#define KL_2_INDEX 2
#define KL_3_INDEX 3
#define KL_CTRL_VALUE_0 0
#define KL_CTRL_VALUE_1 1
#define KL_CTRL_VALUE_2 2
#define KL_CTRL_VALUE_3 3

//OTP 0x3
#define AS_BOOT_EN  1
#define SEE_BOOT_ROM_EN 8
#define BL_ENCRYPT_FLOW 24

static UINT32 reg_88, reg_ac, reg_2e000;
static UINT32 reg_70 = 0;
static UINT32 boottype_mutex_id = OSAL_INVALID_ID;
static void *mmem_start_addr;

INT32 sys_get_otp_from_shadom(UINT32 addr, UINT32 *buffer)
{
    UINT32 otp_base=0;
    UINT32 chip_id = sys_ic_get_chip_id();
    
    if(NULL == buffer)
        return RET_FAILURE;
    if( (chip_id == ALI_S3602F) || (chip_id == ALI_S3281))
    {
        otp_base = OTP_BASE_ADDR_V1;
        if(addr>0x5f)
            return RET_FAILURE;
    }
    else 
    {
        otp_base = OTP_BASE_ADDR_V2;
    }
    switch(addr)
    {
        case 0x2*4:
            if(OTP_BASE_ADDR_V1==otp_base)
                *buffer = readl(otp_base+OTP_0X2_V1);
            else
                *buffer = readl(otp_base+OTP_0X2_V2);        
            break;
        case 0x3*4:
            if(OTP_BASE_ADDR_V1==otp_base)
                *buffer = readl(otp_base+OTP_0X3_V1);
            else
                *buffer = readl(otp_base+OTP_0X3_V2);        
            break;
        case 0x5f*4:
            if(OTP_BASE_ADDR_V1==otp_base)
                *buffer = readl(otp_base+OTP_0X5F_V1);
            else
                *buffer = readl(otp_base+OTP_0X5F_V2);        
            break;
        case 0x80*4:
            *buffer = readl(otp_base+OTP_0X80_V2);
            break;
        case 0x81*4:
            *buffer = readl(otp_base+OTP_0X81_V2);
            break;
        case 0x82*4:
            *buffer = readl(otp_base+OTP_0X82_V2);
            break;
        case 0x83*4:
            *buffer = readl(otp_base+OTP_0X83_V2);
            break;            
        case 0xdc*4:
            *buffer = readl(otp_base+OTP_0XDC_V2);
            break; 
        case 0xdd*4:
            *buffer = readl(otp_base+OTP_0XDD_V2);
            break;     
        case 0xe4*4:
            *buffer = readl(otp_base+OTP_0XE4_V2);
            break; 
        case 0xe5*4:
            *buffer = readl(otp_base+OTP_0XE5_V2);
            break; 
        case 0xe6*4:
            *buffer = readl(otp_base+OTP_0XE6_V2);
            break; 
        case 0xe7*4:
            *buffer = readl(otp_base+OTP_0XE7_V2);
            break;             
        case 0x8c*4:
            *buffer = readl(otp_base+OTP_0X8C_V2);
            break; 
        case 0x8d*4:
            *buffer = readl(otp_base+OTP_0X8D_V2);
            break;             
        default:
            break;
    }
    return RET_SUCCESS;
}


UINT32 sys_ic_get_package_id(void)
{
    UINT32 chip_package = (UINT32)(readl(ALI_SOC_BASE) >> 8) & 0x0F;
    return chip_package;
}

INT32 sys_ic_get_kl_key_mode(UINT32 root_index)
{
	UINT32 otp_03 = 0;
	UINT32 otp_dc = 0;
	UINT32 flag = 0;
	UINT32 rret = 0;
	UINT32 root = root_index;
	UINT32 rev_id = (readl(ALI_SOC_BASE))&0xff;
	UINT32 chip_id = sys_ic_get_chip_id();

	{
		if(((ALI_S3503 == chip_id) && rev_id < C3503B_REV) || (ALI_S3602F == chip_id) || (ALI_S3281 == chip_id)
			|| (ALI_C3701 == chip_id)) //Advance Security chipset
		{
			root = KL_0_INDEX;
		}

		if(chip_id >= ALI_C3701)
		{
			rret = sys_get_otp_from_shadom(0x3*4, &otp_03);
			if(root >= KL_2_INDEX)
			{
				rret = sys_get_otp_from_shadom(0xdc*4, &otp_dc);
			}
		}
		else if(ALI_S3281 == chip_id)
		{
			rret = sys_get_otp_from_shadom(0x3*4, &otp_03);
		}

		if(root < KL_2_INDEX)
		{
			flag = (otp_03 >> 14) & 0x3;   //0x03[15,14]
			switch(flag)
			{
				case KL_CTRL_VALUE_1: 
						rret = 2;
						break;
				case KL_CTRL_VALUE_3: 
						rret = 3;
						break;
				default: 
						rret = 1;
						break; 	
			}
		}
		else
		{
			if(KL_2_INDEX == root)
			{
				flag = (otp_dc >> 14) & 0x3;	 //0xDC[15,14]
			}
			else if(KL_3_INDEX == root)
			{
				flag = (otp_dc >> 16) & 0x3;	 //0xDC[17,16]
			}
			else 
			{
				flag = 0;
			}
			switch(flag)
			{
				case KL_CTRL_VALUE_2: 
						rret = 2;
						break;
				case KL_CTRL_VALUE_3: 
						rret = 3;
						break;
				default: 
						rret = 1;
						break; 	
			}
		}
	}
	
	return rret;
}

/*
Get the flash map status from OTP information.
Input: NULL
Return value:
1: old flash memory mapping with BL chunk header.
2: new flash memory mapping with new BL header (include memory parameters, PK etc)
*/
INT32 sys_ic_get_secure_flash_map(UINT32 *reserved)
{
    INT32 ret = 0;
    UINT32 otp_value = 0;
    UINT32 chip_id = sys_ic_get_chip_id();
    UINT32 chip_ver = sys_ic_get_rev_id();

    ret = sys_get_otp_from_shadom(0x3*4, &otp_value);
    if(RET_FAILURE == ret)
        return RET_FAILURE;

    if(((otp_value&(1<<AS_BOOT_EN))==0) && (chip_id != ALI_S3921)) //none security boot project
        return 1;
    else if(((otp_value&(1<<AS_BOOT_EN))==1))
    {
        if(chip_id<ALI_S3503)
            return 1;
        else if((chip_id==ALI_S3503)&&(chip_ver<IC_REV_2))
            return 1;
        else
            return 2;
    }
    else 
    {
        return 2;
    }
	return 2;
}

/*
Get the flash map status from OTP information.
Input: bootloader mapping address(start address)
Return value:
*/
extern UINT32 __RAM_BASE;
UINT32 sys_ic_get_boot_map_addr(UINT32 reserved)
{
#ifdef __ARM__
	return __RAM_BASE;
#else
    asm volatile(";\
                la $4, __RAM_BASE;\
                sw $4, mmem_start_addr;\
    			"::);
    return (UINT32)mmem_start_addr;
#endif
}

void sys_ic_enable_io_private(void)
{
    writel(IO_PRIVATE_REG1, (readl(IO_PRIVATE_REG1)|(1<<5)));
    writel(IO_PRIVATE_REG2, (readl(IO_PRIVATE_REG2)|(1<<16)));
}

