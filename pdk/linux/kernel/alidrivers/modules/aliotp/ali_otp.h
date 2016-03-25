#ifndef _ALI_OTP_H_
#define _ALI_OTP_H_


struct ali_otp_dev
{
    struct mutex ioctl_mutex;
    dev_t dev_id;
    struct cdev cdev;
};

extern struct ali_otp_dev g_ali_otp_device;

#endif