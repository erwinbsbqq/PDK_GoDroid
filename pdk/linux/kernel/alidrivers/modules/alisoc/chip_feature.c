/*****************************************************************************
*    Copyright (C) 2010 ALi Corp. All Rights Reserved.
*    
*    Company confidential and Properietary information.       
*    This information may not be disclosed to unauthorized  
*    individual.    
*    File: chip_feature.c
*   
*    Description: 
*    
*    History: 
*    Date           Athor        Version        Reason
*    ========       ========     ========       ========
*    2010/8/30      tt         
*        
*****************************************************************************/

//#include <linux/dvb/sys_config.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/init.h>
#include <ali_soc_common.h>
#include <linux/kernel.h> 
#include <ali_reg.h>
#include <rpc_hld/ali_rpc_hld.h>
#include <ali_otp_common.h>

#ifndef FALSE
#define	FALSE			(0)
#endif
#ifndef	TRUE
#define	TRUE			(!FALSE)
#endif

#if defined(CONFIG_MIPS)
extern void c3603_get_bonding(void);
extern int sys_ic_get_hd_enabled_ex(void);
extern int sys_ic_dram_scramble_enabled(void);
extern int sys_ic_io_security_enabled(void);
extern int sys_ic_split_enabled(void);
extern int sys_ic_uart_enabled(void);
extern int sys_ic_ejtag_enabled(void);
//extern int sys_ic_mv_is_enabled(void);
extern int sys_ic_ac3_is_enabled_ex(void);
extern int sys_ic_ddplus_is_enabled_ex(void);		   
extern int sys_ic_XD_is_enabled_ex(void);
extern int sys_ic_XDplus_is_enabled_ex(void);
extern int sys_ic_aac_is_enabled_ex(void);
extern int sys_ic_h264_is_enabled_ex(void);
extern int sys_ic_mp4_is_enabled_ex(void);

extern UINT32 sys_ic_get_product_id(void);
extern UINT32 sys_ic_get_rev_id(void);
extern UINT32 sys_ic_get_chip_id(void);
extern int sys_ic_get_product_id_ex(void);
extern int sys_ic_mg_is_enabled_ex(void);
extern int sys_ic_XD_is_enabled_ex(void);
extern int sys_ic_XDplus_is_enabled_ex(void);
extern int _get_otp_asm_(int addr);


#endif

#ifndef __instrument
#define __instrument
#define __noinstrument __attribute__ ((no_instrument_function))
#endif

/* Need to sync with ZHA, __noinstrument is masked out to pass compilation for
 * GCC ver 3.04.
 * Date: 2012.03.19 by Joy.
 */
#if ((__GNUC__ == 3) && (__GNUC_MINOR__ == 4))
extern UINT32 sys_ic_get_cpu_clock(void);

#else
extern UINT32 __noinstrument sys_ic_get_cpu_clock(void);
#endif

#if defined(CONFIG_MIPS)
extern UINT32 sys_ic_get_dram_clock(void);
extern int sys_reboot_get_timer(UINT32 *time_exp, UINT32 *time_cur);
extern void sys_ic_enter_standby(unsigned int time_exp, unsigned int time_cur);
extern UINT32 product_feature[];
#endif

// functions for drivers
//int sys_ic_get_usb_num(void);    // return USB host number.  0 -- No usb, 1 -- 1 USB host, 2 -- 2 USB host
//int sys_ic_get_ci_num(void);     // return CI slot number.   0 -- No CI.
//int sys_ic_get_mac_num(void);    // return MAC number.       0 -- No MAC.
//int sys_ic_hd_is_enabled(void);	 // 0: enable SD only, 1: enable SD/HD

typedef struct
{
	UINT8 usb_num;
	UINT8 ci_slot_num;
	UINT8 mac_num;
	UINT8 hd_enabled;
	UINT8 m3501_support;				// 3501 support
	UINT8 nim_support;					// other DVB-T/C Demo suppport
	UINT8 sata_enable;
	UINT8 tuner_num;
} PRODUCT_BND;

struct OTP_CONFIG_VPFF //C3503&C3701c chip feature OTP
{
    UINT32 Security_Num:6;    
    UINT32 Certification_Num:7;
    UINT32 Package_Num:3;    
    UINT32 Version_Num:4;
    UINT32 Product_Num:5;    
    UINT32 reserved:6;
};

struct OTP_CONFIG_C3921_VPFF
{
    UINT32 Security_Num:6;      //[5:0]   
    UINT32 Certification_Num:7; //[12:6]
    UINT32 Package_Num1:3;      //[15:13]
    UINT32 Version_Num:4;       //[19:16]
    UINT32 Product_Num:5;       //[24:20]
    UINT32 Package_Num2:2;      //[26:25]
    UINT32 MECO_Num:3;          //[29:27]
    UINT32 Full_Mask_Num:2;     //[31:30]
};

static const PRODUCT_BND m_product_bnd[] = 
{
	{/*"M3601E",*/ 1, 0, 0, 1, 0, 1, 0, 1},    // 0
	{/*"M3381E",*/ 1, 0, 0, 0, 1, 1, 0, 2},    // 1
	{/*"M3606", */ 2, 0, 1, 1, 1, 1, 0, 2},    // 2
	{/*"M3606B",*/ 1, 0, 1, 1, 1, 1, 1, 2},    // 3
	{/*"M3606C",*/ 2, 1, 1, 1, 1, 1, 0, 2},    // 4
	{/*"M3606D",*/ 1, 1, 1, 1, 1, 1, 1, 2},    // 5
	{/*"M3386", */ 2, 0, 1, 0, 1, 1, 0, 2},    // 6
	{/*"M3386B",*/ 1, 0, 1, 0, 1, 1, 1, 2},    // 7
	{/*"M3386C",*/ 2, 1, 1, 0, 1, 1, 0, 2},    // 8
	{/*"M3386D",*/ 1, 1, 1, 0, 1, 1, 1, 2},    // 9
	{/*"M3701E",*/ 2, 0, 1, 1, 1, 1, 0, 2},    // 10
	{/*"M3701D",*/ 1, 0, 1, 1, 1, 1, 1, 2},    // 11
	{/*"M3701C",*/ 2, 0, 1, 1, 1, 1, 0, 2},    // 12
	{/*"M3701F",*/ 1, 0, 1, 1, 1, 1, 1, 2},    // 13
	{/*"M3603", */ 2, 2, 1, 1, 1, 1, 1, 2},    // 14
	{/*"M3383", */ 2, 2, 1, 0, 1, 1, 1, 2},    // 15
	{/*"M3601S",*/ 1, 0, 0, 1, 1, 1, 0, 1},    // 16
};


static UINT32 reg_88, reg_ac, reg_2e000, reg_70;


#define PRODUCT_NUM_C3603   (sizeof(m_product_bnd)/sizeof(m_product_bnd[0]))

UINT32 ali_sys_ic_get_chip_id(void);

/*
 *
 *The API below specified for KERNEL call!
 *
*/

void ali_sys_ic_enter_standby(UINT32 time_exp, UINT32 time_cur)
{
	#if defined(CONFIG_MIPS)
    	sys_ic_enter_standby(time_exp, time_cur);
    #else
    	return;
    #endif
}

int ali_sys_reboot_get_timer(unsigned long *time_exp, unsigned long *time_cur)
{
	#if defined(CONFIG_MIPS)
    	return sys_reboot_get_timer(time_exp, time_cur);
    #else
    	return 0;
    #endif
}

/************************
    Product Number
        M3503
     0x84h [24:20]
     
    M3510	00000
    M3511	00001
    M3512	00010
    M3516	00011
    M3315P	00100
    M3515B	00101
    M3715P	00110
    M3615P	00111
    M3315	01000
    M3515	01001
    M3715	01010
    M3615	01111

**************************/

/******************************
    Product Number
        M3701c
     0x84h [24:20]
product name  0x84 value [24:20]

M3701C-ATCA   0x00400080  00100
M3701G-ATCA   0x00000080  00000
M3912-ATAA    0x00200000  00010
M3912-ATOA    0x00200038  00010
M3901C-ATAA   0x00300000  00011
M3901C-ATOA   0x00300038  00011
M3701H-ALCA   0x00100080  00001
M3616-APAA    0x01102000  10001
M3616-APCA    0x01102080  10001
M3713-APAC    0x01002003  10000
M3713-APCC    0x01002083  10000
M3713P-A2CV   0x01302095  10011
M3713P-APAC   0x01302003  10011
M3713P-APCC   0x01302083  10011

*******************************/
UINT32 sys_ic_get_product_id(void)
{
    UINT32 product_id = 0;
    UINT32 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    struct OTP_CONFIG_VPFF *g_reg = NULL;
    struct OTP_CONFIG_C3921_VPFF *g_3921_reg = NULL;
    
    if (chip_id == ALI_S3503 || chip_id == ALI_C3701)
    {
        OTP_value = otp_get_value(0x84*4);
        g_reg = (struct OTP_CONFIG_VPFF *)&OTP_value;
        product_id = g_reg->Product_Num;     
    }
    else if(chip_id == ALI_C3921)
    { 
        product_id = otp_get_value(0x84*4);
        g_3921_reg = (struct OTP_CONFIG_C3921_VPFF *)&OTP_value;
        product_id = g_3921_reg->Product_Num;   
    }
    else
    {
        #if defined(CONFIG_MIPS)
            product_id = sys_ic_get_product_id_ex();
        #endif
    }
    return product_id;
}

UINT32 sys_otp_get_version(void)
{
    UINT32 version_num = 0;
    UINT32 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    struct OTP_CONFIG_VPFF *g_reg = NULL;
    struct OTP_CONFIG_C3921_VPFF *g_3921_reg = NULL;
        
    if (chip_id == ALI_S3503 || chip_id == ALI_C3701)
    {
        OTP_value = otp_get_value(0x84*4);
        g_reg = (struct OTP_CONFIG_VPFF *)&OTP_value;
        version_num = g_reg->Version_Num;     
    }
    else if(chip_id == ALI_C3921)
    {
        OTP_value = otp_get_value(0x84*4);
        g_3921_reg = (struct OTP_CONFIG_C3921_VPFF *)&OTP_value;
        version_num = g_3921_reg->Product_Num;   
    }
    
    return version_num;
}

UINT32 sys_otp_get_package(void)
{
    UINT32 package_num1 = 0;    
    UINT32 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    struct OTP_CONFIG_VPFF *g_reg = NULL;
    struct OTP_CONFIG_C3921_VPFF *g_3921_reg = NULL;

    if (chip_id == ALI_S3503 || chip_id == ALI_C3701)
    {
        OTP_value = otp_get_value(0x84*4);
        g_reg = (struct OTP_CONFIG_VPFF *)&OTP_value;
        package_num1 = g_reg->Package_Num;     
    }
    else if (chip_id == ALI_C3921)
    {
        OTP_value = otp_get_value(0x84*4);
        g_3921_reg = (struct OTP_CONFIG_C3921_VPFF *)&OTP_value;
        package_num1 = (g_3921_reg->Package_Num2) << 3; 
        package_num1 |= g_3921_reg->Package_Num1;
    }
    return package_num1;
}

UINT32 sys_otp_get_certification(void)
{
    UINT32 certification_num = 0;
    UINT32 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    struct OTP_CONFIG_VPFF *g_reg = NULL;
    struct OTP_CONFIG_C3921_VPFF *g_3921_reg = NULL;
    
    if (chip_id == ALI_S3503 || chip_id == ALI_C3701)
    {
        OTP_value = otp_get_value(0x84*4);
        g_reg = (struct OTP_CONFIG_VPFF *)&OTP_value;
        certification_num = g_reg->Certification_Num;     
    }
    else if (chip_id == ALI_C3921)
    {
        OTP_value = otp_get_value(0x84*4);
        g_3921_reg = (struct OTP_CONFIG_C3921_VPFF *)&OTP_value;
        certification_num = g_3921_reg->Certification_Num;   
    }
    
    return certification_num;
}

UINT32 sys_otp_get_security(void)
{
    UINT32 security_num = 0;
    UINT32 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    struct OTP_CONFIG_VPFF *g_reg = NULL;
    struct OTP_CONFIG_C3921_VPFF *g_3921_reg = NULL;
    
    if (chip_id == ALI_S3503 || chip_id == ALI_C3701)
    {
        OTP_value = otp_get_value(0x84*4);
        g_reg = (struct OTP_CONFIG_VPFF *)&OTP_value;
        security_num = g_reg->Security_Num;     
    }
    else if (chip_id == ALI_C3921)
    {
        OTP_value = otp_get_value(0x84*4);
        g_3921_reg = (struct OTP_CONFIG_C3921_VPFF *)&OTP_value;
        security_num = g_3921_reg->Security_Num; 
    }
    return security_num;
}

UINT32 ali_sys_ic_get_boot_type(void)
{
    UINT8 ret = 0;

    #if defined(CONFIG_MIPS)
    	//ret = board_is_nand_boot();
    #endif
    
    return ret;
}

/*****************************************************************    
return Enable/Disable AC3(DD).    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_ac3_is_enabled(void)
{
    UINT8 ret = FALSE;
    UINT8 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    
    if(chip_id == ALI_S3503 || chip_id == ALI_C3921)
    {
        OTP_value = sys_otp_get_certification();
        ret = OTP_value & 0x1;
        return ret;
    }
    else
    {
    	#if defined(CONFIG_MIPS)
            ret = sys_ic_ac3_is_enabled_ex();
        #endif
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable DD+.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_ddplus_is_enabled(void)
{
    UINT8 ret = FALSE;
    UINT8 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    
    if(chip_id == ALI_S3503 || chip_id == ALI_C3921)
    {
        OTP_value = sys_otp_get_certification();
        ret = (OTP_value & 0x2)>>1;
        return ret;
    }
    else
    {
    	#if defined(CONFIG_MIPS)
	        ret = sys_ic_ddplus_is_enabled_ex();
	    #endif
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable MS10.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_ms10_is_enabled(void)
{
    UINT8 ret = FALSE;
    UINT8 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    
    if(chip_id == ALI_S3503)
    {
        OTP_value = sys_otp_get_certification();
        ret = (OTP_value & 0x4)>>2;
        return ret;
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable MS11.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_ms11_is_enabled(void)
{
    UINT8 ret = FALSE;
    UINT8 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    
    if(chip_id == ALI_S3503)
    {
        OTP_value = sys_otp_get_certification();
        ret = (OTP_value & 0x8)>>3;
        return ret;
    }
    else if (chip_id == ALI_C3921)
    {
        OTP_value = sys_otp_get_certification();
        ret = (OTP_value & 0x4)>>2;
        return ret;
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable MG.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_mg_is_enabled(void)
{
    UINT8 ret = FALSE;
    UINT8 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    
    if(chip_id == ALI_S3503)
    {
        OTP_value = sys_otp_get_certification();
        ret = (OTP_value & 0x10)>>4;
        return ret;
    }
    else
    {
    	#if defined(CONFIG_MIPS)
	        ret = sys_ic_mg_is_enabled_ex();
	    #endif
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable XD.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_xd_is_enabled(void)
{
    UINT8 ret = FALSE;
    UINT8 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    
    if(chip_id == ALI_S3503)
    {
        OTP_value = sys_otp_get_certification();
        ret = (OTP_value & 0x20)>>5;
        return ret;
    }
    else
    {
    	#if defined(CONFIG_MIPS)
        	ret = sys_ic_XD_is_enabled_ex();
        #endif
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable XDplus.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_xdplus_is_enabled(void)
{
    UINT8 ret = FALSE;
    UINT8 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    
    if(chip_id == ALI_S3503)
    {
        OTP_value = sys_otp_get_certification();
        ret = (OTP_value & 0x40)>>6;
        return ret;
    }
    else
    {
    	#if defined(CONFIG_MIPS)
        	ret = sys_ic_XDplus_is_enabled_ex();
        #endif
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable H264.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_h264_is_enabled(void)
{
    UINT8 ret = FALSE;
    UINT32 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    
    if(chip_id == ALI_S3503 || chip_id == ALI_C3921)
    {
        OTP_value = otp_get_value(0x83*4);
        ret = (OTP_value & 0x2)>>1;
        return (ret==0?TRUE:FALSE);
    }
    else
    {
    	#if defined(CONFIG_MIPS)
	        ret = sys_ic_h264_is_enabled_ex();
	    #endif
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable MP4.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_mp4_is_enabled(void)
{
    UINT8 ret = FALSE;
    UINT32 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    
    if(chip_id == ALI_S3503 || chip_id == ALI_C3921)
    {
        OTP_value = otp_get_value(0x83*4);
        ret = (OTP_value & 0x4)>>2;
        return (ret==0?TRUE:FALSE);
    }
    else
    {
    	#if defined(CONFIG_MIPS)
        	ret = sys_ic_mp4_is_enabled_ex();
        #endif
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable RMVB.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_rmvb_is_enabled(void)
{
    UINT8 ret = FALSE;
    UINT32 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
        
    if(chip_id == ALI_S3503)
    {
        OTP_value = otp_get_value(0x83*4);
        ret = (OTP_value & 0x8)>>3;
        return (ret==0?TRUE:FALSE);
    }
    else if (chip_id == ALI_C3921)
    {
        OTP_value = sys_otp_get_certification();
        ret = (OTP_value & 0x8)>>3;
        return ret;
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable VC1.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_vc1_is_enabled(void)
{
    UINT8 ret = FALSE;
    UINT32 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    
    if(chip_id == ALI_S3503 || chip_id == ALI_C3921)
    {
        OTP_value = otp_get_value(0x83*4);
        ret = (OTP_value & 0x10)>>4;
        return (ret==0?TRUE:FALSE);
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable AVS.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_avs_is_enabled(void)
{
    UINT8 ret = FALSE;
    UINT32 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    
    if(chip_id == ALI_S3503 || chip_id == ALI_C3921)
    {
        OTP_value = otp_get_value(0x83*4);
        ret = (OTP_value & 0x20)>>5;
        return (ret==0?TRUE:FALSE);
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable VP8.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_vp8_is_enabled(void)
{
    UINT8 ret = FALSE;
    UINT32 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    
    if(chip_id == ALI_S3503 || chip_id == ALI_C3921)
    {
        OTP_value = otp_get_value(0x83*4);
        ret = (OTP_value & 0x40)>>6;
        return (ret==0?TRUE:FALSE);
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable HD.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_get_hd_enabled(void)
{
    UINT8 ret = FALSE;
    UINT32 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    
    if(chip_id == ALI_S3503 || chip_id == ALI_C3921)
    {
        OTP_value = otp_get_value(0x83*4);
        ret = (OTP_value & 0x1000)>>12;
        return (ret==0?TRUE:FALSE);
    }
    else
    {
    	#if defined(CONFIG_MIPS)
	        ret = sys_ic_get_hd_enabled_ex();
	    #endif
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable FLV.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_flv_is_enabled(void)
{
    UINT8 ret = FALSE;
    UINT32 OTP_value = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    
    if(chip_id == ALI_S3503 || chip_id == ALI_C3921)
    {
        OTP_value = otp_get_value(0x83*4);
        ret = (OTP_value & 0x8000)>>15;
        return (ret==0?TRUE:FALSE);
    }
    return ret;
}

/*****************************************************************    
return Enable/Disable AAC.    
return 1, enable    
return 0, disable
*****************************************************************/
UINT32 sys_ic_aac_is_enabled(void)
{
    UINT8 ret = FALSE;
    #if defined(CONFIG_MIPS)
	    ret = sys_ic_aac_is_enabled_ex();
	#endif
	
    return ret;
}

/*get bonding with c3603*/
void ali_sys_ic_get_bonding(void)
{
    //here we just reserve this for later use
    /*UINT32 chip_id = ali_sys_ic_get_chip_id();
    if(chip_id == ALI_S3602F)
    {
        return c3603_get_bonding();
    }
    else{
        if(chip_id == ALI_OTHERS)
        {
            return other_get_bonding();
        }   
    }
    */
    #if defined(CONFIG_MIPS)
		c3603_get_bonding();
	#endif
}

/*return product id*/
UINT32 ali_sys_ic_get_product_id(void)
{
	  UINT32 product_id;

	  product_id = sys_ic_get_product_id();

    return product_id;
}


/*return c3603 product id*/
UINT32 ali_sys_ic_get_c3603_product(void)
{
    UINT32 product_id;

    product_id = ali_sys_ic_get_product_id();

    if (product_id == 0)
    {
        UINT32 chip_id = __REG32ALI(0x18000000);
        chip_id &= 0xf00;
        chip_id >>= 8;

        if (chip_id == 0x0c)
            product_id = 14;    // M3603
        else if (chip_id == 0x05 || chip_id == 0x07)
            product_id = 11;    // M3701D, 1 USB
//        else if (chip_id == 0x0b)
//			product_id = 2;
//        else if (chip_id == 0x09)
//			product_id = 3;
    }

    return product_id;
}

/*return chip id*/
UINT32 ali_sys_ic_get_chip_id(void)
{
	UINT32 chip_id,chip_id_reg;

	chip_id_reg = __REG32ALI(0x18000000);
	chip_id_reg = chip_id_reg>>16;
	
	if(chip_id_reg ==0x3327){
		chip_id = ALI_M3327;
	}else if(chip_id_reg ==0x3101){
		chip_id = ALI_M3327;
	}else if(chip_id_reg ==0x3202){
		chip_id = ALI_M3327;
	}else if(chip_id_reg ==0x3329){
		chip_id = ALI_M3329E;
	}else if(chip_id_reg ==0x3602){
		chip_id = ALI_S3602;
	}else if(chip_id_reg ==0x3603){
		chip_id = ALI_S3602F;
	}else if(chip_id_reg ==0x3901){
		chip_id = ALI_S3901;
	}else if(chip_id_reg ==0x3701){
		chip_id = ALI_C3701;
	}else if(chip_id_reg ==0x3921){
		chip_id = ALI_C3921;
	}else if(chip_id_reg ==0x3503){
		chip_id = ALI_S3503;	
	}else{
		chip_id = SYS_DEFINE_NULL;
	}

	return chip_id;
}
EXPORT_SYMBOL(ali_sys_ic_get_chip_id);



/*return revised id*/
UINT32 ali_sys_ic_get_rev_id(void)
{
	UINT32 rev_id,chip_id_reg;

	chip_id_reg = __REG8ALI(0x18000000);

	if(chip_id_reg ==0){
		rev_id = IC_REV_0;
	}else if(chip_id_reg ==1){
		rev_id = IC_REV_1;
	}else if(chip_id_reg ==2){
		rev_id = IC_REV_2;
	}else if(chip_id_reg ==3){
		rev_id = IC_REV_3;
	}else if(chip_id_reg ==4){
		rev_id = IC_REV_4;
	}else{
		rev_id = IC_REV_4;
	}

	return rev_id;
}

/*return cpu clock*/
UINT32 ali_sys_ic_get_cpu_clock(void)
{
	unsigned long strap_pin_reg = 0, pll_reg = 0, M = 0, N = 0, L = 0, cpu_clock = 0;
	
    UINT32 chip_id = ali_sys_ic_get_chip_id();

	if (chip_id >= ALI_C3921)
	{
		strap_pin_reg = __REG32ALI(0x18000070);
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
		
	}
    else{ // 3701c
		//cpu_clock = sys_ic_get_cpu_clock();

		strap_pin_reg = __REG32ALI(0x18000070);
		strap_pin_reg = (strap_pin_reg>>7)&0x07;
		if(strap_pin_reg == 0)
		{
			cpu_clock = 600;	//600MHz
		}
		else if(strap_pin_reg == 1)
		{
			cpu_clock = 450;	//450MHz
		}
		else if(strap_pin_reg == 2)
		{
			cpu_clock = 396;	//396MHz
		}
		else if(strap_pin_reg == 3)
		{
			cpu_clock = 297;	//297MHz
		}
		else if(strap_pin_reg == 4)
		{	// cpu use pll clk
			//5:0	Digital PLL function L control Register
			//13:8	Digital PLL function N Control Register
			//25:16	Digital PLL function M Control Register
			//CPU_CLK = 27*(MCTRL+1)/( NCTRL+1)/( LCTRL+1)
			pll_reg = __REG32ALI(0x180004d0);
			M= (pll_reg>>16)&0x3FF;
			N= (pll_reg>>8)&0x3F;
			L= (pll_reg>>0)&0x3F;
			cpu_clock=27*(M+1)/(N+1)/(L+1);
		 }
    	}
	return cpu_clock;
}


/*
	3701c STRAP_INFO Strap pin information. Reflect the strap pin status that latched into C3701C when power up or warm reset.
Bit 3-0 :	WORK_MODE[3:0]
Bit 4:		PLL_BYPASS
Bit 6-5:	MCLK_SEL[1:0]
Bit 7: 	CPUCLK_SEL
Bit8:     reserved
Bit 9:		reserved 
Bit11-10:	reserved
Bit12:	CRYSTAL_54M_SEL
Bit 13:	UART_UPGRADE_EN
Bit 14:	SINGLE_CPU_SEL
Bit 15:	EN_EXT_SFLASH
Bit 16:	FUNCTION_MODE
Bit 17:	MCLK_SEL[2]
Bit 18:	NF_SF_SEL (1: NAND boot , default , 0: NOR boot)
Bit 19:    CPU_Probe Enable

6:5	MEM_CLK_SEL[2:0]
. (MEM_CLK_SEL[2] is BIT[17] of 74H register)
000: 166M
001: 133M ;
010: 100M
011: 1.68M
100: Reserved
101:83.3M
110:reserved

#define MEM_CLK_166M	0x00	// DDR3 1333Mbps DDR2:NULL, BGA256 only, QFP not support
#define MEM_CLK_133M	0x01	//DDR3/2 1066Mbps  internal bus:133MHz external clk:533 MHz 
#define MEM_CLK_100M	0x02	//DDR3/2 800Mbps 
#define MEM_CLK_2d06M	0x03	//16.5Mbps
#define MEM_CLK_200M	0x04	//not support
#define MEM_CLK_83M		0x05	// DD2 667Mbps 
#define MEM_CLK_16d5M	0x03	// 2.06*8
#define MEM_CLK_27M	0x06	//0xb8000074 bit[4] 0:enable pll 1: Pll by pass, can not trigger by sw, just can be set in strap pin

*/

/*return dram clock*/
UINT32 ali_sys_ic_get_dram_clock()
{
	unsigned long strap_pin_reg,dram_clock;
    UINT32 chip_id = ali_sys_ic_get_chip_id();

    if (chip_id >= ALI_C3921)
    {
		strap_pin_reg = __REG32ALI(0x18000070);
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
    	
    }else{ // 3701c
		strap_pin_reg = __REG32ALI(0x18000070);
		strap_pin_reg = (strap_pin_reg>>5)&0x03;
		if(strap_pin_reg == 0)
		{
			dram_clock = 166*8;	//1333Mbps 
		}
		else if(strap_pin_reg == 1)
		{
			dram_clock = 133*8;	//1066Mbps
		}
		else if(strap_pin_reg == 2)
		{
			dram_clock = 100*8;	//800Mbps
		}
		else
		{
			dram_clock = 83*8;	//667Mbps
		}
	}
	
	return dram_clock;
}


/*return dram size*/
//0x18001000 bit[2:0] Organization
// 000:  16 MB@16bits    32 MB@32bits
// 001:  32 MB@16bits    64 MB@32bits	
// 010:  64 MB@16bits   128 MB@32bits
// 011: 128 MB@16bits  256 MB@32bits
// 100: 256 MB@16bits  512 MB@32bits	
// 101: 512 MB@16bits  1GB@32bits
// 110: 128MB (DDR2: 8bits x 2, Just 16 bits Interleave Mapping)	
// 111: 1GB@16bits 2GB@32bits		
unsigned long ali_sys_ic_get_dram_size(void) 
{
	unsigned long data = 0,dram_size = 0;
    UINT32 chip_id = ali_sys_ic_get_chip_id();

	
    if (chip_id >= ALI_C3921)
    {
		data = __REG32ALI(0x18001000) & 0x07;				
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
    }
    
	
	return dram_size;
}


/*return usb number supported by chip*/
int ali_sys_ic_get_usb_num(void)
{
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    UINT32 product_id;

    // C3603
    if (chip_id == ALI_S3602F && ali_sys_ic_get_rev_id() >= IC_REV_1)
    {
        product_id = ali_sys_ic_get_c3603_product();
        if (product_id < PRODUCT_NUM_C3603)
        {
            return m_product_bnd[product_id].usb_num;
        }
        return 1;
    }
    if(chip_id == ALI_S3503)
        return 2;  
    
    return 1;
}

/*return NIM number supported by chip*/
int ali_sys_ic_get_ci_num(void)
{
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    UINT32 product_id;

    if (chip_id == ALI_S3602F)
    {
        // S3602F
        if (ali_sys_ic_get_rev_id() == IC_REV_0)
            return 2;

        // C3603
        product_id = ali_sys_ic_get_c3603_product();
        if (product_id < PRODUCT_NUM_C3603)
        {
            return m_product_bnd[product_id].ci_slot_num;
        }
        return 0;
    }

    if (chip_id == ALI_S3602)
        return 2;
    if (chip_id == ALI_S3503)
        return 1; 

    return 2;   // S3329E
}


/*return TUNER number supported by chip*/
int ali_sys_ic_get_tuner_num(void)
{
	UINT32 chip_id = ali_sys_ic_get_chip_id();
    UINT32 product_id;

    if (chip_id == ALI_S3602F)
    {
        // S3602F
        if (ali_sys_ic_get_rev_id() == IC_REV_0)
            return 2;

        // C3603
        product_id = ali_sys_ic_get_c3603_product();
        if (product_id < PRODUCT_NUM_C3603)
        {
            return m_product_bnd[product_id].tuner_num;
        }
    }
	return 2;	
}

/*return MAC number supported by chip*/
int ali_sys_ic_get_mac_num(void)
{
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    UINT32 product_id;

    if (chip_id == ALI_S3602F)
    {
        // S3602F
        if (ali_sys_ic_get_rev_id() == IC_REV_0)
            return 0;

        // C3603
        product_id = ali_sys_ic_get_c3603_product();
        if (product_id < PRODUCT_NUM_C3603)
        {
            return m_product_bnd[product_id].mac_num;
        }
        return 0;
    }

    return 0;
}


int ali_sys_ic_get_hd_enabled(void)
{
	int hd_enabled;

	hd_enabled = sys_ic_get_hd_enabled();

	return hd_enabled;
}

int ali_sys_ic_dram_scramble_enabled(void)
{
    int scra_enabled = 0;

    #if defined(CONFIG_MIPS)
    	scra_enabled = sys_ic_dram_scramble_enabled();
    #endif
    
    return scra_enabled;
}

int ali_sys_ic_io_security_enabled(void)
{
    int sec_enabled = 0;

    #if defined(CONFIG_MIPS)
    	sec_enabled = sys_ic_io_security_enabled();
    #endif
    
    return sec_enabled;
}

int ali_sys_ic_split_enabled(void)
{
   int spl_enabled = 0;

   #if defined(CONFIG_MIPS)
       spl_enabled = sys_ic_split_enabled();
   #endif
   
   return spl_enabled;
}

int aii_sys_ic_uart_enabled(void)
{
   int uart_enabled = 0;

   #if defined(CONFIG_MIPS)
   	   uart_enabled = sys_ic_uart_enabled();
   #endif
   
   return uart_enabled;
}

int ali_sys_ic_ejtag_enabled(void)
{
    int ejt_enabled = 0;

    #if defined(CONFIG_MIPS)
    	ejt_enabled = sys_ic_ejtag_enabled();
    #endif
    
    return ejt_enabled;
}

int ali_sys_ic_mv_is_enabled(void)
{
   int mv_enabled = 0;

   #if defined(CONFIG_MIPS)
       //mv_enabled = sys_ic_mv_is_enabled();
   #endif
   
   return mv_enabled;
}
int ali_sys_ic_mg_is_enabled(void)
{
   int mv_enabled = 0;
   
   mv_enabled = sys_ic_mg_is_enabled();
   
   return mv_enabled;
}

int ali_sys_ic_ac3_is_enabled(void)
{
    int ac3_enabled = 0;
    
    ac3_enabled = sys_ic_ac3_is_enabled();
    
    return ac3_enabled;
}

int ali_sys_ic_ddplus_is_enabled(void)
{
   int ddp_enabled = 0;
   
   ddp_enabled = sys_ic_ddplus_is_enabled();
   
   return ddp_enabled;
}

int ali_sys_ic_XD_is_enabled(void)
{
    int XD_enabled = 0;

    #if defined(CONFIG_MIPS)
    	XD_enabled = sys_ic_XD_is_enabled_ex();
    #endif
    
    return XD_enabled;
}

int ali_sys_ic_XDplus_is_enabled(void)
{
    int XD_enabled= 0;

    #if defined(CONFIG_MIPS)
    	XD_enabled = sys_ic_XDplus_is_enabled_ex();
    #endif
    
    return XD_enabled;
}

int ali_sys_ic_aac_is_enabled(void)
{
    int aac_enabled = 0;
    
    aac_enabled = sys_ic_aac_is_enabled();
    
    return aac_enabled;
}


int ali_sys_ic_h264_is_enabled(void)
{
    int h264_enabled = 0;
    
    h264_enabled = sys_ic_h264_is_enabled();
    
    return h264_enabled;
}

int ali_sys_ic_mp4_is_enabled(void)
{
    int mp4_enabled = 0;
    
    mp4_enabled = sys_ic_mp4_is_enabled();
    
    return mp4_enabled;
}

int ali_sys_ic_ms10_is_enabled(void)
{
    int ms10_enabled = 0;
    
    ms10_enabled = sys_ic_ms10_is_enabled();
    
    return ms10_enabled;
}

int ali_sys_ic_ms11_is_enabled(void)
{
    int ms11_enabled = 0;
    
    ms11_enabled = sys_ic_ms10_is_enabled();
    
    return ms11_enabled;
}

int ali_sys_ic_rmvb_is_enabled(void)
{
    int rmvb_enabled = 0;
    
    rmvb_enabled = sys_ic_rmvb_is_enabled();
    
    return rmvb_enabled;
}

int ali_sys_ic_vc1_is_enabled(void)
{
    int vc1_enabled = 0;
    
    vc1_enabled = sys_ic_vc1_is_enabled();
    
    return vc1_enabled;
}
    
int ali_sys_ic_avs_is_enabled(void)
{
    int avs_enabled = 0;
    
    avs_enabled = sys_ic_avs_is_enabled();
    
    return avs_enabled;
}

int ali_sys_ic_vp8_is_enabled(void)
{
    int vp8_enabled = 0;
    
    vp8_enabled = sys_ic_vp8_is_enabled();
    
    return vp8_enabled;
}

int ali_sys_ic_flv_is_enabled(void)
{
    int flv_enabled= 0;
    
    flv_enabled = sys_ic_flv_is_enabled();
    
    return flv_enabled;
}

int ali_sys_ic_hd_is_enabled(void)	// 0: enable SD only, 1: enable SD/HD
{
    UINT32 product_id = 0;

    if (ALI_S3602F == ali_sys_ic_get_chip_id())
    {
        // S3602F
        if (ali_sys_ic_get_rev_id() == IC_REV_0)
            return 1;

        // C3603
        product_id = ali_sys_ic_get_c3603_product();
        if (product_id < PRODUCT_NUM_C3603)
        {
            return m_product_bnd[product_id].hd_enabled;
        }
    }

    return ali_sys_ic_get_hd_enabled();
}



int ali_sys_ic_usb_port_enabled(UINT32 port)
{
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    UINT32 product_id;
		int ret;
    // C3603
    if (chip_id == ALI_S3602F && ali_sys_ic_get_rev_id() >= IC_REV_1)
    {
        product_id = ali_sys_ic_get_c3603_product();
        if (product_id == 0 || product_id == 16)
        {
            ret = (port==1)?1:0;   //M3601E port 0 always enabled, 1 always disabled
        }else
        {
        //    if(((~(product_feature[1]>>20))&0x3)&(1<<port))
            	ret = 1;
        //    else
         //   	ret = 0;
      	}
        return ret;
    }

    return ((port==0)?1:0);
}


int ali_sys_ic_sata_enable(void)
{
	UINT32 chip_id = ali_sys_ic_get_chip_id();
    UINT32 product_id;

    if (chip_id == ALI_S3602F)
    {
        // S3602F
        if (ali_sys_ic_get_rev_id() == IC_REV_0)
            return 0;

        // C3603
        product_id = ali_sys_ic_get_c3603_product();
        if (product_id < PRODUCT_NUM_C3603)
        {
            return m_product_bnd[product_id].sata_enable;
        }
        return 0;
    }
    if((ALI_S3503 == chip_id)&&(ali_sys_ic_get_rev_id() > IC_REV_0))
        return 1;

    return 0;
}

int ali_sys_ic_nim_support(void) //BOOL
{
	UINT32 chip_id = ali_sys_ic_get_chip_id();
    UINT32 product_id;
	int ret = 0; //BOOL FALSE

	if (ALI_S3602F == chip_id)
	{
		product_id = ali_sys_ic_get_c3603_product();
		if (product_id < PRODUCT_NUM_C3603)
		{
			if (1 == m_product_bnd[product_id].nim_support)
				ret = 1; //TRUE
			else
				ret = 0; //FALSE
		}
	}
    if((ALI_S3503 == chip_id)&&(ali_sys_ic_get_rev_id() > IC_REV_0))
        ret = TRUE;
    
	return ret;
}


int ali_sys_ic_nim_m3501_support(void) //BOOL
{
	UINT32 chip_id = ali_sys_ic_get_chip_id();
    UINT32 product_id;
	int ret = 0; //BOOL FALSE

	if (ALI_S3602F == chip_id)
	{
		product_id = ali_sys_ic_get_c3603_product();	
		if (product_id < PRODUCT_NUM_C3603)
		{
			if (1 == m_product_bnd[product_id].m3501_support)
				ret = 1; //TRUE
			else
				ret = 0; //FALSE
		}
	}
    if((ALI_S3503 == chip_id)&&(ali_sys_ic_get_rev_id() > IC_REV_0))
        ret= TRUE;
	return ret;
}

int sys_ic_get_mac_num(void)
{
    UINT32 chip_id = ali_sys_ic_get_chip_id();
    UINT32 product_id;

    if (chip_id == ALI_S3602F)
    {
        // S3602F
        if (ali_sys_ic_get_rev_id() == IC_REV_0)
            return 0;

        // C3603
        product_id = ali_sys_ic_get_c3603_product();
        if (product_id < PRODUCT_NUM_C3603)
        {
            return m_product_bnd[product_id].mac_num;
        }
        return 0;
    }
	if(chip_id == ALI_S3811)	
	{
		return 1;	
    }
    if ((ALI_S3503 == chip_id)&&(ali_sys_ic_get_rev_id() > IC_REV_0))
       return 1;	
    return 0;
}
int ali_sys_ic_change_boot_type(unsigned int type)
{
    UINT32 chip_id, chip_package;
    UINT32 tmp32;

    chip_id = ali_sys_ic_get_chip_id();
    chip_package = (UINT8)(readl(ALI_SOC_BASE) >> 8) & 0x0F;	

    switch(type)
    {
        case ALI_SOC_BOOT_TYPE_NOR:
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
            
        case ALI_SOC_BOOT_TYPE_NAND:
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
            return 1;
    }

	return 0;
}
int ali_sys_ic_revert_boot_type(void)
{
	UINT32 chip_id,chip_package;
	UINT32 tmp32;

    chip_id = ali_sys_ic_get_chip_id();
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
	return 0;
}

#define M3515_SEEROM_ENABLE (1<<8)   /*OTP configure 0x03*/
int ali_sys_ic_seerom_is_enabled(void)
{
		UINT32  OTP_value;    
    if (ali_sys_ic_get_chip_id() >= ALI_S3503)
    {
        OTP_value = otp_get_value(0x3*4);        
        return (OTP_value & M3515_SEEROM_ENABLE);
    }
		return  0;
}
