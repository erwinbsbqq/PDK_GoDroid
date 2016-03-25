#ifndef SIP_DIB8090_H
#define SIP_DIB8090_H

#include <adapter/common.h>
//#include <tuner/dib0090.h>

struct dib8090_config {
    uint8_t output_mpeg2_in_188_bytes;
    int (*update_lna) (struct dibFrontend *, uint16_t);

#define DIB8090_GPIO_DEFAULT_DIRECTIONS 0xffff
    uint16_t gpio_dir;

#define DIB8090_GPIO_DEFAULT_VALUES     0x0000
    uint16_t gpio_val;

#define DIB8090_GPIO_DEFAULT_PWM_POS    0xffff
    uint16_t gpio_pwm_pos;

    int dib0090_freq_offset_khz_uhf;
    int dib0090_freq_offset_khz_vhf;

    uint8_t osc_buffer_state; /* 0= normal, 1= tri-state */
    uint32_t clock_khz;
    uint8_t clock_pad_drive;

    uint8_t tuner_gated_i2c;

    uint8_t invert_iq;

    uint8_t use_high_level_adjustment;
    uint8_t force_crystal_mode; /* if == 0 -> decision is made in the driver */

    uint16_t dib8k_drives;
    uint16_t diversity_delay;
    uint16_t div_cfg;

	uint8_t clkouttobamse : 1;
        uint8_t clkoutdrive;
    uint8_t refclksel;
    //uint8_t fref_clock_ratio;

    //const struct dib0090_wbd_slope *dib0090_wbd_table;
};

#define DIB8090_DEFAULT_I2C_ADDRESS     0x12
struct dibDataBusHost;

extern void dib8090_set_wbd_target(struct dibFrontend *fe, uint16_t vhf, uint16_t uhf);

//! Register one DiB8090-SIP to a Frontend using I2C
/**
 * This function can be used to register a DiB8090-SIP connected to an I2C-bus to a frontend. It represents the digital demodulator part and the RF tuner part of a frontend. A SIP is a complete frontend.
 * \param fe dibFrontend to register this SIP to
 * \param host the DataBusHost (I2C-Adapter) which can be used to reach this demodulator
 * \param addr I2C of this demodulator on the I2C-Bus (the I2C address of the tuner is fixed in a SIP)
 * \param cfg pointer to a configuration-struct describing the demod/tuner-setup. Has to be given by the caller.
 * \return fe on success , NULL otherwise.
 */
extern struct dibFrontend * dib8090_sip_register(struct dibFrontend *fe, struct dibDataBusHost *host, uint8_t addr, const struct dib8090_config *cfg);
//extern int dib8090_set_wbd_table(struct dibFrontend *fe, const struct dib0090_wbd_slope *wbd);

#endif
