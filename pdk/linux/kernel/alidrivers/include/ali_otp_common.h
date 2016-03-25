#ifndef _ALI_OTP_COMMON_H
#define _ALI_OTP_COMMON_H 

#define MAX_ALI_OTP_PARAS 0x80
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


#define ALI_OTP_READ  _IOR('S', 1, struct otp_read_paras)
#define ALI_OTP_WRITE _IOW('S', 2, struct otp_write_paras)

int ali_otp_read(unsigned long offset, unsigned char *buf, int len);
int ali_otp_write(unsigned char *buf, unsigned long offset, int len);
int ali_otp_hw_init(void);

unsigned int otp_get_value(unsigned int addr);
unsigned int otp_get_control(void);

#define ENABLE   1
#define DISABLE  0
inline unsigned int is_otp_uart_enable(void); // if enable return 1, else return 0 ;
inline unsigned int is_otp_usb_enable(void);
inline unsigned int is_otp_ethernet_enable(void);

#endif 
