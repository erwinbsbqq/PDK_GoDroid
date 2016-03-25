
#ifndef __OTP_BUS_H__
#define __OTP_BUS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ali/otp.h>

#define OTP_ERR_NOSUPPORT 		(-1)
#define OTP_ERR_LOCKTMO			(-2)

#define OTP_VOLTAGE_6V5 65    //OTP voltage 6.5V
#define OTP_VOLTAGE_1V8 18    //OTP voltage 1.8V

enum OTP_RETURN_VALUE
{
    OTP_READ_ERROR = -2,
    OTP_WRITE_ERROR = -1,
};

enum EJTAG_PROTECT_LEVEL
{
	EJTAG_CLOSE,
	EJTAG_PASSWORD_SEE_ROM_FLOW,
	EJTAG_PASSWORD_FLOW,
};

#define OTP_WRITE32(reg, data)	(*((volatile UINT32 *)(otp_reg_base + reg)) = (data))
#define OTP_READ32(reg)			(*((volatile UINT32 *)(otp_reg_base + reg)))
#define OTP_WRITE8(reg, data)	(*((volatile UINT8 *)(otp_reg_base + reg)) = (data))
#define OTP_READ8(reg)			(*((volatile UINT8 *)(otp_reg_base + reg)))

/* Basic OTP Function of M3603/M3281 */
INT32 otp_m33_init(OTP_CONFIG *cfg);
INT32 otp_m33_read(UINT32 offset, UINT8 *buf, INT32 len);
INT32 otp_m33_write(UINT8 *buf, UINT32 offset, INT32 len);

/* Basic OTP Function of M3701/M35xx*/
INT32 otp_m37_read(UINT32 offset, UINT8 *buf, INT32 len);
INT32 otp_m37_write(UINT8 *buf, UINT32 offset, INT32 len);
INT32 otp_m37_init(void);
INT32 otp_m35_init(void);

#ifdef __cplusplus
 }
#endif

#endif    /* __OTP_H__ */


