#ifndef TUNER_DIB0090_H
#define TUNER_DIB0090_H

#ifdef __cplusplus
extern "C" {
#endif

struct dibFrontend;
struct dibDataBusHost;

struct dib0090_io_config {
    uint32_t clock_khz;

    uint8_t pll_bypass  : 1;
    uint8_t pll_range   : 1;
    uint8_t pll_prediv  : 6;
    uint8_t pll_loopdiv : 6;

    uint8_t adc_clock_ratio; /* valid is 8, 7 ,6 */
    uint16_t pll_int_loop_filt; // internal loop filt value. If not fill in , default is 8165
};

struct dib0090_config {
    struct dib0090_io_config io;
    /* tuner pins controlled externally */
    int (*reset) (struct dibFrontend *, int);
    int (*sleep) (struct dibFrontend *, int);

    /*  offset in kHz */
    int freq_offset_khz_uhf;
    int freq_offset_khz_vhf;

    int (*get_adc_power) (struct dibFrontend *);

    uint8_t clkouttobamse : 1; /* activate or deactivate clock output */
    uint8_t analog_output;

    uint8_t i2c_address;
    /* add drives and other things if necessary */
    uint16_t wbd_vhf_offset;
    uint16_t wbd_cband_offset;
    uint8_t use_pwm_agc;
	uint8_t clkoutdrive;
};

extern struct dibFrontend * dib0090_firmware_register(struct dibFrontend *fe, struct dibDataBusHost *data, const struct dib0090_config *cfg);
extern struct dibFrontend * dib0090_register(struct dibFrontend *, struct dibDataBusHost *, const struct dib0090_config *);

extern struct dibFrontend * dib0090_reset_register(struct dibFrontend *, struct dibDataBusHost *, const struct dib0090_config *);

extern void dib0090_twin_setup(struct dibFrontend *master, struct dibFrontend *slave);

extern void dib0090_set_wbd_target(struct dibFrontend *, uint16_t);
extern int16_t dib0090_get_wbd_value(struct dibFrontend *);

extern int dib0090_ext_gain_control(struct dibFrontend *tuner, uint8_t idx, uint16_t gain);
extern int dib0090_get_digital_clk_out(struct dibFrontend *fe);

extern void dib0090_get_current_gain(struct dibFrontend *fe, uint16_t *rf, uint16_t *bb, uint16_t *rf_gain_limit, uint16_t *rflt);
extern void dib0090_pwm_gain_reset(struct dibFrontend *fe, struct dibChannel *ch);
extern int dib0090_gain_control(struct dibFrontend *fe, struct dibChannel *ch);

extern uint16_t dib0090_get_wbd_offset(struct dibFrontend * fe);
#ifdef CONFIG_DIB0090_USE_PWM_AGC
extern void dib0090_dcc_freq(struct dibFrontend *fe,uint8_t fast);
#endif

#ifdef __cplusplus
}
#endif

#endif
