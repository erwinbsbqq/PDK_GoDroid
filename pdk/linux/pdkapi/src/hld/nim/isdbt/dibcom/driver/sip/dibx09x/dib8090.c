#include <adapter/frontend.h>

#include <demod/dib8000.h>
#include <tuner/dib0090.h>
#include "dibx09x_common.h"
#include <sip/dib8090.h>

struct dib8090_state {
    struct dibSIPInfo info;
    struct dib0090_config dib0090_cfg;
    const struct dib8090_config *cfg;

    int (*update_lna)   (struct dibFrontend *, uint16_t);

    uint16_t wbd_target_uhf;
    uint16_t wbd_target_vhf;

};

static const struct dibDebugObject dib8090m_dbg = {
    DEBUG_SIP,
    "DiB8090"
};

const struct dibx000_agc_config dib8090_agc_config[2] = {
    {
        BAND_UHF | BAND_VHF | BAND_LBAND | BAND_SBAND,
        /* P_agc_use_sd_mod1=0, P_agc_use_sd_mod2=0, P_agc_freq_pwm_div=1, P_agc_inv_pwm1=0, P_agc_inv_pwm2=0,
         * P_agc_inh_dc_rv_est=0, P_agc_time_est=3, P_agc_freeze=0, P_agc_nb_est=5, P_agc_write=0 */
        (0 << 15) | (0 << 14) | (5 << 11) | (0 << 10) | (0 << 9) | (0 << 8) | (3 << 5) | (0 << 4) | (5 << 1) | (0 << 0), // setup

        787,// inv_gain = 1/ 90.4dB // no boost, lower gain due to ramp quantification
        10,  // time_stabiliz

        0,  // alpha_level
        118,  // thlock

        0,     // wbd_inv
        3530,  // wbd_ref
        1,     // wbd_sel
        5,     // wbd_alpha

#if 0
        27265,  // agc1_max     = 0.415 == 27229 to 27265 = 28 steps
        27265,  // agc1_min
#endif

        65535,  // agc1_max
        0,  // agc1_min

        32767,  // agc2_max
        0,      // agc2_min

        0,      // agc1_pt1
        32,     // agc1_pt2
        114,    // agc1_pt3  // 40.4dB
        143,    // agc1_slope1
        144,    // agc1_slope2
        114,    // agc2_pt1
        227,    // agc2_pt2
        116,    // agc2_slope1
        117,    // agc2_slope2

        28,  // alpha_mant // 5Hz with 90.2dB
        26,  // alpha_exp
        31,  // beta_mant
        51,  // beta_exp

        0,  // perform_agc_softsplit
    },
    {
        BAND_CBAND,
        /* P_agc_use_sd_mod1=0, P_agc_use_sd_mod2=0, P_agc_freq_pwm_div=1, P_agc_inv_pwm1=0, P_agc_inv_pwm2=0,
         * P_agc_inh_dc_rv_est=0, P_agc_time_est=3, P_agc_freeze=0, P_agc_nb_est=5, P_agc_write=0 */
        (0 << 15) | (0 << 14) | (5 << 11) | (0 << 10) | (0 << 9) | (0 << 8) | (3 << 5) | (0 << 4) | (5 << 1) | (0 << 0), // setup

        787,// inv_gain = 1/ 90.4dB // no boost, lower gain due to ramp quantification
        10,  // time_stabiliz

        0,  // alpha_level
        118,  // thlock

        0,     // wbd_inv
        3530,  // wbd_ref
        1,     // wbd_sel
        5,     // wbd_alpha

#if 0
        27265,  // agc1_max     = 0.415 == 27229 to 27265 = 28 steps
        27265,  // agc1_min
#endif

        0,  // agc1_max
        0,  // agc1_min

        32767,  // agc2_max
        0,      // agc2_min

        0,      // agc1_pt1
        32,     // agc1_pt2
        114,    // agc1_pt3  // 40.4dB
        143,    // agc1_slope1
        144,    // agc1_slope2
        114,    // agc2_pt1
        227,    // agc2_pt2
        116,    // agc2_slope1
        117,    // agc2_slope2

        28,  // alpha_mant // 5Hz with 90.2dB
        26,  // alpha_exp
        31,  // beta_mant
        51,  // beta_exp

        0,  // perform_agc_softsplit
    }
};

static int dib8090_tuner_sleep(struct dibFrontend *fe, int onoff)
{
    dbgpl(&dib8090m_dbg, "sleep dib0090: %d", onoff);
    demod_set_gpio(fe, 0, 0, (uint8_t)onoff);
    return DIB_RETURN_SUCCESS;
}

static int dib8090_tuner_reset(struct dibFrontend *fe, int onoff)
{
    dbgpl(&dib8090m_dbg, "reset dib0090: %d", onoff);
    demod_set_gpio(fe, 5, 0, !onoff);
    return DIB_RETURN_SUCCESS;
}

#ifdef CONFIG_DIB0090_USE_PWM_AGC
static int dib8090_agc_control(struct dibFrontend *fe, uint8_t restart)
{
    dbgpl(&dib8090m_dbg, "AGC control callback: %d", restart);
    dib0090_dcc_freq(fe, restart);
    return DIB_RETURN_SUCCESS;
}
#endif

/*static int dib8090_update_lna(struct dibFrontend *fe, uint16_t agc_global)
  {
  struct dib8090_state *st = fe->sip->priv;
  if (st->update_lna)
  st->update_lna(fe, agc_global);

  if (st->cfg->use_high_level_adjustment) {
  if (agc_global == 0 && dib0070_get_rf_output(fe) != 3) { // very high level
  dib0070_set_rf_output(fe, 3);
  dbgpl(&dib8070m_dbg, "setting RF switch to 3 because of very high input level = %d", agc_global);
  return 1;
  }
  }
  return 0;
  }*/

static const struct dibx000_bandwidth_config dib8090_pll_config_30mhz = {
    52500, 13125, // internal, sampling
    1, 7, 3, 1, 0, // pll_cfg: prediv, ratio, range, reset, bypass
    0, 0, 1, 1, 2, // misc: refdiv, bypclk_div, IO_CLK_en_core, ADClkSrc, modulo
    (3 << 14) | (1 << 12) | (599 << 0), // sad_cfg: refsel, sel, freq_15k
    (0 << 25) | 0, // ifreq = 0 MHz
    20776863, // timf
    30000000, // xtal_hz
};

static const struct dibx000_bandwidth_config dib8090_pll_config_15mhz = {
    52500, 13125, // internal, sampling
    1, 14, 3, 1, 0, // pll_cfg: prediv, ratio, range, reset, bypass
    0, 0, 1, 1, 2, // misc: refdiv, bypclk_div, IO_CLK_en_core, ADClkSrc, modulo
    (3 << 14) | (1 << 12) | (599 << 0), // sad_cfg: refsel, sel, freq_15k
    (0 << 25) | 0, // ifreq = 0 MHz
    20776863, // timf
    15000000, // xtal_hz
};

static const struct dibx000_bandwidth_config dib8090_pll_config_12mhz = {
    54000, 13500, // internal, sampling
    1, 18, 3, 1, 0, // pll_cfg: prediv, ratio, range, reset, bypass
    0, 0, 1, 1, 2, // misc: refdiv, bypclk_div, IO_CLK_en_core, ADClkSrc, modulo
    (3 << 14) | (1 << 12) | (599 << 0), // sad_cfg: refsel, sel, freq_15k
    (0 << 25) | 0, // ifreq = 0 MHz
    20199727, // timf
    12000000, // xtal_hz
};

static int dib8090_get_adc_power(struct dibFrontend *fe)
{
    return dib8000_get_adc_power(fe, 1);
}

static const struct dib8000_config default_dib8000_config = {
    0, // output_mpeg2_in_188_bytes
    0, // hostbus_diversity
    NULL, // LNA update

    2, // agc_config_count
    dib8090_agc_config,
    NULL,

    DIB8000_GPIO_DEFAULT_DIRECTIONS, // gpio_dir
    DIB8000_GPIO_DEFAULT_VALUES,  // gpio_val
    DIB8000_GPIO_DEFAULT_PWM_POS, // gpio_pwm_pos
    0,

#ifdef CONFIG_DIB0090_USE_PWM_AGC
    dib8090_agc_control,
#else
    NULL,
#endif
};

static const struct dib0090_config default_dib0090_config = {
    { 0 },
    dib8090_tuner_reset,
    dib8090_tuner_sleep,
    0,0,0,0,0,192,
    100,450,
    1, // use PWM
    7  // clkoutdrive
};

static int dib8090_agc_startup(struct dibFrontend *fe, struct dibChannel *ch)
{
    struct dib8090_state *state = fe->sip->priv;
    uint8_t band = (uint8_t) channel_frequency_band(ch->RF_kHz);
    int ret;
    uint16_t offset;

    if (fe->tune_state == CT_AGC_START) {
        dbgpl(&dib8090m_dbg, "not tuning in CBAND - standard AGC startup");
        offset = state->wbd_target_uhf;
        switch (band) {
        case BAND_VHF:
            offset = state->wbd_target_vhf;
            break;
        default:
            break;
        }
        offset += (dib0090_get_wbd_offset(fe) * 8 * 18 / 33 + 1) / 2;

        demod_set_wbd_ref(fe, offset);
    }

#ifdef CONFIG_DIB0090_USE_PWM_AGC
    if (band == BAND_CBAND) {
        if (fe->tune_state == CT_AGC_START) {
            dbgpl(&dib8090m_dbg, "tuning in CBAND - soft-AGC startup");
            /* TODO specific wbd target for dib0090 - needed for startup ? */
        }

        if (fe->tune_state < CT_AGC_STOP)
#endif
            ret = dib0090_gain_control(fe, ch);
#ifdef CONFIG_DIB0090_USE_PWM_AGC
        else
            ret = FE_CALLBACK_TIME_NEVER;

        if (fe->tune_state == CT_AGC_STOP) {
            dbgpl(&dib8090m_dbg, "switching to PWM AGC");
            dib0090_pwm_gain_reset(fe, ch);
            dib8000_pwm_agc_reset(fe, ch);
        }
    } else { /* for everything else than CBAND we are using standard AGC */
        if (fe->tune_state == CT_AGC_START)
            dib0090_pwm_gain_reset(fe, ch);
        ret = dib8000_agc_startup(fe, ch);
    }
#endif
    return ret;
}

static void dib8090_release(struct dibFrontend *fe)
{
    struct dib8090_state *state = fe->sip->priv;
    MemFree(state,sizeof(struct dib8090_state));
}

static const struct dibSIPInfo dib8090_info = {
    "DiBcom DiB8090MB",

    {
        dib8090_release
    }
};

void dib8090_set_wbd_target(struct dibFrontend *fe, uint16_t vhf, uint16_t uhf)
{
    struct dib8090_state *state = fe->sip->priv;
    state->wbd_target_vhf =  vhf;
    state->wbd_target_uhf =  uhf;
}

struct dibFrontend * dib8090_sip_register(struct dibFrontend *fe, struct dibDataBusHost *host, uint8_t addr, const struct dib8090_config *cfg)
{
    struct dib8090_state *state;
    struct dib8000_config dib8000_cfg;

    state = MemAlloc(sizeof(struct dib8090_state));
    if (state == NULL)
        return NULL;
    DibZeroMemory(state, sizeof(struct dib8090_state));

    frontend_register_sip(fe, &state->info, &dib8090_info, state);

    state->cfg = cfg;

    memcpy(&dib8000_cfg, &default_dib8000_config, sizeof(struct dib8000_config));
    memcpy(&state->dib0090_cfg, &default_dib0090_config, sizeof(struct dib0090_config));

    switch (cfg->clock_khz) {
    case 12000:
        dib8000_cfg.pll = &dib8090_pll_config_12mhz;
        memcpy(&state->dib0090_cfg.io, &dibx09x_io_12mhz_120, sizeof(dibx09x_io_12mhz_120));
        break;
    case 30000:
        dib8000_cfg.pll = &dib8090_pll_config_30mhz;
        memcpy(&state->dib0090_cfg.io, &dibx09x_io_30mhz_120, sizeof(dibx09x_io_30mhz_120));
        break;
    default:
        dbgpl(&dib8090m_dbg, "Error : %d is not a valid crystal frequency, please add the config for this frequency", cfg->clock_khz);
        goto error;
    }

    dib8000_cfg.output_mpeg2_in_188_bytes = cfg->output_mpeg2_in_188_bytes;
    dib8000_cfg.hostbus_diversity         = 1;
    dib8000_cfg.gpio_dir                  = cfg->gpio_dir;
    dib8000_cfg.gpio_val                  = cfg->gpio_val;
    dib8000_cfg.gpio_pwm_pos              = cfg->gpio_pwm_pos;
    dib8000_cfg.drives                    = cfg->dib8k_drives;
    dib8000_cfg.diversity_delay           = cfg->diversity_delay;
    dib8000_cfg.div_cfg                   = cfg->div_cfg;
    dib8000_cfg.refclksel                 = cfg->refclksel;

    state->update_lna                     = cfg->update_lna;


    state->dib0090_cfg.io.pll_bypass = 1;
    state->dib0090_cfg.freq_offset_khz_uhf = cfg->dib0090_freq_offset_khz_uhf;
    state->dib0090_cfg.freq_offset_khz_vhf = cfg->dib0090_freq_offset_khz_vhf;
    state->dib0090_cfg.clkouttobamse = cfg->clkouttobamse;
    state->dib0090_cfg.analog_output = 1;
    state->dib0090_cfg.clkoutdrive   = cfg->clkoutdrive;

    state->wbd_target_vhf =  330; // 500
    state->wbd_target_uhf =  500;

    if (dib8000_register(fe, host, addr, &dib8000_cfg) == NULL)
        goto error;

    state->dib0090_cfg.get_adc_power = dib8090_get_adc_power;
    host = dib8000_get_i2c_master(fe, DIBX000_I2C_INTERFACE_TUNER, cfg->tuner_gated_i2c);
    if (dib0090_register(fe, host, &state->dib0090_cfg) == NULL)
        goto tuner_error;

    /* custom AGC start for DiB8090 */
    fe->demod_info->ops.agc_startup = dib8090_agc_startup;

    return fe;

tuner_error:
    frontend_unregister_demod(fe);
error:
    MemFree(state, sizeof(struct dib8090_state));

    return NULL;
}

