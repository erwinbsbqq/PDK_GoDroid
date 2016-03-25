#ifndef _ALI_SOC_H_
#define _ALI_SOC_H_

#include <linux/types.h>

#define OTP_ERR_NOSUPPORT		(-1)
#define OTP_ERR_LOCKTMO		(-2)

#define OTP_VOLTAGE_6V5 65    //OTP voltage 6.5V
#define OTP_VOLTAGE_1V8 18    //OTP voltage 1.8V

/*voltage control callback function, OTP driver will tell APP the correct voltage by 
* OTP_VOLTAGE_6V5 or OTP_VOLTAGE_6V5*/
typedef void(*ALI_SOC_OTP_VOLTAGE_CONTROL)(unsigned long voltage);

/* APP must make OTP driver to control programming voltage to guarantee program timming.
* So App can choose to register GPIO to OTP driver or register voltage control callback function to OTP driver*/
typedef struct {
	unsigned short	vpp_by_gpio: 1;		/*1: we need use one GPIO control vpp voltage.*/
								/*0: we use Jumper to switch vpp voltage.*/
	unsigned short	reserved1: 15;		/*reserved for future usage*/
	unsigned short	vpp_polar	: 1;		/* Polarity of GPIO, 0 or 1 active to set VPP to 6.5V*/
	unsigned short	vpp_io		: 1;		/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	unsigned short	vpp_position: 14;	/* GPIO Number*/
	ALI_SOC_OTP_VOLTAGE_CONTROL volctl_cb;		/*OTP program voltage control callback*/
										/*OTP_VOLTAGE_6V5 for 6.5V,OTP_VOLTAGE_1V8 for 1.8V*/
}SOC_OTP_CONFIG;

#define MAX_ALI_SOC_PARAS 32

struct otp_read_paras {
    unsigned long offset;
    unsigned char *buf;
    int len;
};                    


struct otp_write_paras {
    unsigned char *buf;
    unsigned long offset;
    int len;
};                    

struct reboot_timer{
    unsigned long *time_exp;
    unsigned long *time_cur;    
};

struct boot_timer{
    unsigned long time_exp;
    unsigned long time_cur;  
};
struct soc_usb_port {
    unsigned long usb_port;
};

struct soc_opt_paras8 {
    unsigned long addr;
    unsigned char data;    
};      

struct soc_opt_paras16 {
    unsigned long addr;
    unsigned short data;    
};      

struct soc_opt_paras32 {
    unsigned long addr;
    unsigned long data;    
};      

struct debug_level_paras {
    unsigned long level;   
};      


struct soc_memory_map
{	
	unsigned long main_start;		
	unsigned long main_end;	
	unsigned long fb_start;
	unsigned long osd_bk;	
	unsigned long see_dmx_src_buf_start;
	unsigned long see_dmx_src_buf_end;
	unsigned long see_dmx_decrypto_buf_start;
	unsigned long see_dmx_decrypto_buf_end;
	unsigned long dmx_start;
	unsigned long dmx_top;
	unsigned long see_start;
	unsigned long see_top;
	unsigned long video_start;
	unsigned long video_top;
	unsigned long frame;
	unsigned long frame_size;
	unsigned long vcap_fb;
	unsigned long vcap_fb_size;
	unsigned long vdec_vbv_start;
	unsigned long vdec_vbv_len;
	unsigned long shared_start;
	unsigned long shared_top;	

	unsigned long reserved_mem_addr;
	unsigned long reserved_mem_size;

	unsigned long media_buf_addr;
	unsigned long media_buf_size;

	unsigned long mcapi_buf_addr;
	unsigned long mcapi_buf_size;
};

struct soc_opt_see_ver {
    unsigned char *buf;
};      


#define ALI_SOC_OTP_READ  _IOR('S', 1, struct otp_read_paras)
#define ALI_SOC_OTP_WRITE _IOW('S', 2, struct otp_write_paras)

#define ALI_SOC_CHIP_ID       _IO('S', 3)
#define ALI_SOC_PRODUCT_ID    _IO('S', 4)
#define ALI_SOC_GET_BONDING   _IO('S', 5)
#define ALI_SOC_REV_ID        _IO('S', 6)
#define ALI_SOC_CPU_CLOCK     _IO('S', 7)
#define ALI_SOC_DRAM_CLOCK    _IO('S', 8)
#define ALI_SOC_HD_ENABLED    _IO('S', 9)
#define ALI_SOC_C3603_PRODUCT _IO('S', 10)
#define ALI_SOC_USB_NUM       _IO('S', 11)
#define ALI_SOC_USB_PORT_ENABLED   _IOR('S', 12, unsigned long)
#define ALI_SOC_NIM_M3501_SUPPORT  _IO('S' ,13)
#define ALI_SOC_NIM_SUPPORT   _IO('S', 14)
#define ALI_SOC_CI_NUM        _IO('S', 15)
#define ALI_SOC_MAC_NUM       _IO('S', 16)
#define ALI_SOC_TUNER_NUM     _IO('S', 17)
#define ALI_SOC_HD_IS_ENABLED    _IO('S', 18)
#define ALI_SOC_SATA_EANBLE   _IO('S', 19)
#define ALI_SOC_SCRAM_ENABLE  _IO('S', 20)
#define ALI_SOC_SECU_ENABLE   _IO('S', 21)
#define ALI_SOC_SPL_ENABLE    _IO('S', 22)
#define ALI_SOC_UART_ENABLE   _IO('S', 23)
#define ALI_SOC_ETJT_ENABLE   _IO('S', 24)
#define ALI_SOC_MV_ENABLE     _IO('S', 25)
#define ALI_SOC_AC3_ENABLE    _IO('S', 26)
#define ALI_SOC_DDP_ENABLE    _IO('S', 27)
#define ALI_SOC_XD_ENABLE    _IO('S', 28)
#define ALI_SOC_XDP_ENABLE   _IO('S', 29)
#define ALI_SOC_AAC_ENABLE    _IO('S', 30)
#define ALI_SOC_H264_ENABLE   _IO('S', 31)
#define ALI_SOC_MP4_ENABLE    _IO('S', 32)
#define ALI_SOC_REBOOT_GET_TIMER   _IOR('S', 33, struct reboot_timer)
#define ALI_SOC_ENTER_STANDBY      _IOW('S', 34 , struct boot_timer)
#define ALI_SOC_SET_DEBUG_LEVEL _IOW('S', 35, struct debug_level_paras)
#define ALI_SOC_GET_MEMORY_MAP _IOW('S', 36, struct soc_memory_map)

#define ALI_SOC_READ8  _IOR('S', 37, struct soc_opt_paras8)
#define ALI_SOC_WRITE8 _IOW('S', 38, struct soc_opt_paras8)
#define ALI_SOC_READ16  _IOR('S', 39, struct soc_opt_paras16)
#define ALI_SOC_WRITE16 _IOW('S', 40, struct soc_opt_paras16)
#define ALI_SOC_READ32  _IOR('S', 41, struct soc_opt_paras32)
#define ALI_SOC_WRITE32 _IOW('S', 42, struct soc_opt_paras32)
#define ALI_SOC_GET_SEE_VER _IOW('S', 43, struct soc_opt_see_ver)
#define ALI_SOC_DISABLE_SEE_PRINTF _IOR('S', 44, unsigned long)
#define ALI_SOC_HIT_SEE_HEART	_IO('S', 45)
#define ALI_SOC_ENABLE_SEE_EXCEPTION	_IO('S', 46)

/********************************************** 
*******  CHIP MACRO FROM sys_define.h  ********
**********************************************/

#define HW_TYPE_CHIP			  0x00010000
#define HW_TYPE_CHIP_REV		0x00020000

#define ALI_M3202				(HW_TYPE_CHIP + 10)
#define ALI_S3501				(HW_TYPE_CHIP + 50)
#define ALI_S3602				(HW_TYPE_CHIP + 70)
#define ALI_S3602F				(HW_TYPE_CHIP + 71)
#define ALI_S3811				(HW_TYPE_CHIP + 72)
#define ALI_S3901               (HW_TYPE_CHIP + 75)
#define ALI_C3701				(HW_TYPE_CHIP + 74)
#define ALI_M3327				  (HW_TYPE_CHIP + 4)
#define ALI_M3329E				(HW_TYPE_CHIP + 6)
#define ALI_M3327C				(HW_TYPE_CHIP + 7)

#define ALI_ARM_CHIP			(HW_TYPE_CHIP + 0x00050000)
#define ALI_C3921				(ALI_ARM_CHIP + 1)

#define IC_REV_0				(HW_TYPE_CHIP_REV + 1)
#define IC_REV_1				(HW_TYPE_CHIP_REV + 2)
#define IC_REV_2				(HW_TYPE_CHIP_REV + 3)
#define IC_REV_3				(HW_TYPE_CHIP_REV + 4)
#define IC_REV_4				(HW_TYPE_CHIP_REV + 5)
#define IC_REV_5				(HW_TYPE_CHIP_REV + 6)
#define IC_REV_6				(HW_TYPE_CHIP_REV + 7)
#define IC_REV_7				(HW_TYPE_CHIP_REV + 8)
#define IC_REV_8				(HW_TYPE_CHIP_REV + 9)

#define SYS_DEFINE_NULL			0x00000000	/* NULL define */

int ali_otp_init(SOC_OTP_CONFIG * cfg);
int ali_otp_read(unsigned long offset, unsigned char *buf, int len);
int ali_otp_write(unsigned char *buf, unsigned long offset, int len);


void ali_sys_ic_get_bonding(void);
void sys_ic_enter_standby(unsigned int time_exp, unsigned int  time_cur);
void ali_sys_ic_enter_standby(unsigned long time_exp, unsigned long  time_cur);

int ali_sys_ic_get_ci_num(void);
int ali_sys_ic_get_mac_num(void);
int ali_sys_ic_get_usb_num(void);
int ali_sys_ic_get_tuner_num(void);
int ali_sys_ic_nim_m3501_support(void);
int ali_sys_ic_nim_support(void);
int ali_sys_ic_hd_is_enabled(void);
int ali_sys_ic_usb_port_enabled(unsigned long port);
int ali_sys_ic_get_hd_enabled(void);
int ali_sys_ic_sata_enable(void);
int ali_sys_ic_dram_scramble_enabled(void);
int ali_sys_ic_io_security_enabled(void);
int ali_sys_ic_split_enabled(void);
int aii_sys_ic_uart_enabled(void);
int ali_sys_ic_ejtag_enabled(void);
int ali_sys_ic_mv_is_enabled(void);
int ali_sys_ic_ac3_is_enabled(void);
int ali_sys_ic_ddplus_is_enabled(void);
int ali_sys_ic_XD_is_enabled(void);
int ali_sys_ic_XDplus_is_enabled(void);
int ali_sys_ic_aac_is_enabled(void);
int ali_sys_ic_h264_is_enabled(void);
int ali_sys_ic_mp4_is_enabled(void);
int sys_reboot_get_timer(unsigned long *time_exp, unsigned long *time_cur);
int ali_sys_reboot_get_timer(unsigned long *time_exp, unsigned long *time_cur);


unsigned long ali_sys_ic_get_c3603_product(void);
unsigned long ali_sys_ic_get_product_id(void);
unsigned long ali_sys_ic_get_chip_id(void);
unsigned long ali_sys_ic_get_rev_id(void);
unsigned long ali_sys_ic_get_cpu_clock(void);
unsigned long ali_sys_ic_get_dram_clock(void);

#endif 
