#include <common.h>
#include <asm/io.h>

#if defined(CONFIG_ARM)
	#define OTP_BASE_ADDR	0x18042000
#elif defined(CONFIG_MIPS)
	#define OTP_BASE_ADDR	0xb8042000
#endif

#define OTP_0X2             0x40    
#define OTP_0X3             0x44  
#define OTP_0X5F            0x48  
#define OTP_0X80            0x4c    
#define OTP_0X81            0x50  
#define OTP_0X82            0x6c  
#define OTP_0X83            0x70    
#define OTP_0XDC            0x78
#define OTP_0XDD            0x7c
#define OTP_0XE4            0x80
#define OTP_0XE5            0x84
#define OTP_0XE6            0x88
#define OTP_0XE7            0x8c
#define OTP_0X8C            0x90
#define OTP_0X8D            0x94

/**
*
* @brief    This function get the specifial OTP value.
* @param[in] addr  The OTP address
* @param[in] buffer  Output OTP value
* @return int32_t
* @retval 0   The operation was completed successfully.
* @retval -1   Any other errors.
*
*/
int32_t sys_get_otp_from_shadom(uint32_t addr, uint32_t *buffer)
{
    uint32_t otp_base = 0;

    if (NULL == buffer)
    {
        return -1;
    }

    otp_base = OTP_BASE_ADDR;
    switch (addr)
    {
        case 0x2*4:
            *buffer = readl(otp_base+OTP_0X2);
            break;
        case 0x3*4:
            *buffer = readl(otp_base+OTP_0X3);
            break;
        case 0x5f*4:
            *buffer = readl(otp_base+OTP_0X5F);
            break;
        case 0x80*4:
            *buffer = readl(otp_base+OTP_0X80);
            break;
        case 0x81*4:
            *buffer = readl(otp_base+OTP_0X81);
            break;
        case 0x82*4:
            *buffer = readl(otp_base+OTP_0X82);
            break;
        case 0x83*4:
            *buffer = readl(otp_base+OTP_0X83);
            break;
        case 0xdc*4:
            *buffer = readl(otp_base+OTP_0XDC);
            break; 
        case 0xdd*4:
            *buffer = readl(otp_base+OTP_0XDD);
            break;
        case 0xe4*4:
            *buffer = readl(otp_base+OTP_0XE4);
            break; 
        case 0xe5*4:
            *buffer = readl(otp_base+OTP_0XE5);
            break; 
        case 0xe6*4:
            *buffer = readl(otp_base+OTP_0XE6);
            break; 
        case 0xe7*4:
            *buffer = readl(otp_base+OTP_0XE7);
            break;
        case 0x8c*4:
            *buffer = readl(otp_base+OTP_0X8C);
            break; 
        case 0x8d*4:
            *buffer = readl(otp_base+OTP_0X8D);
            break;
        default:
            break;
    }

    return 0;
}

/**
*
* @brief    This function check the Bootrom_0 signature enabled status.
* @param[out] en  0 - disabled, 1- enabled
* @return int32_t
* @retval 0   The operation was completed successfully.
* @retval -1   Any other errors.
*
*/
int32_t otp_get_bootrom0_sig_enabled(uint32_t *en)
{
    uint32_t otp_8c = 0;

    if (NULL == en)
    {
        return -1;
    }

    if (0 != sys_get_otp_from_shadom(0x8c*4, &otp_8c))
    {
        return -1;
    }

    *en = ((otp_8c >> 23) & 0x1);

    return 0;
}


