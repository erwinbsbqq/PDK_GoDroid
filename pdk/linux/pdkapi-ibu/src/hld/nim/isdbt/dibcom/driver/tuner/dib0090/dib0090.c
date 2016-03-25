#include <adapter/frontend.h>
#include <tuner/dib0090.h>
#include "dib0090_priv.h"

//#define DEBUG_AGC

#define ADC_TARGET -220
#define EFUSE

/* included in [0:962] => 10 bit unsigned => GAIN_ALPHA <= 5 */
#define GAIN_ALPHA 5

/* average the RF max gain with 2^WBD_ALPHA normally max=5 (target = -640 or
 * -100) => 10 bits signed => WBD_ALPHA <= 5
 *
 * we use 6, but it's risky, but it can work -> because we never have less than
 * 61dBm (610-100 = 510 == 9bits)
 */
#define WBD_ALPHA 6

#define LPF	100

DEBUG_OBJECT(dib0090_dbg, DEBUG_TUNER, "DiB0090")

static void dib0090_write_reg(struct dib0090_state *st, uint32_t r, uint16_t v)
{
      data_bus_client_write16(tuner_get_data_bus_client(st->fe), r, v);
}

#define dib0090_write_reg_async(st, r, v) dib0090_write_reg(st, r, v)

static uint16_t dib0090_read_reg(struct dib0090_state *st, uint32_t reg)
{
    return data_bus_client_read16(tuner_get_data_bus_client(st->fe), reg);
}

#ifdef CONFIG_DIB0090_USE_PWM_AGC

static void dib0090_write_regs(struct dib0090_state *state, uint8_t r, const uint16_t *b, uint8_t c)
{
    do {
        dib0090_write_reg(state, r++, *b++);
    } while (--c);
}
#endif

/* wakeup from sleep */
static int dib0090_wakeup(struct dibFrontend *fe)
{
    struct dib0090_state *state = fe->tuner_priv;
    if (state->config->sleep)
        state->config->sleep(fe, 0);
	return DIB_RETURN_SUCCESS;
}

static int dib0090_sleep(struct dibFrontend *fe)
{
    struct dib0090_state *state = fe->tuner_priv;
    if (state->config->sleep)
        state->config->sleep(fe, 1);
	return DIB_RETURN_SUCCESS;
}

int dib0090_get_digital_clk_out(struct dibFrontend *fe)
{
    struct dib0090_state *state = fe->tuner_priv;
    return (state->config->io.clock_khz * state->config->io.pll_loopdiv) / state->config->io.pll_prediv;
}

/********************DC Calib ***************************/
#define steps(u) (((u)>15)?((u)-16):(u))
#define INTERN_WAIT 10

static int dib0090_get_offset(struct dib0090_state *state, enum frontend_tune_state *tune_state)
{
    int ret = INTERN_WAIT * 10;

    switch (*tune_state) {
    case CT_TUNER_STEP_2:
        /* Turns to positive */
        dib0090_write_reg(state, VGA_MODE,0x7);
        *tune_state = CT_TUNER_STEP_3;
        break;

    case CT_TUNER_STEP_3:
        state->adc_diff = dib0090_read_reg(state, ADCVAL);

        /* Turns to negative */
        dib0090_write_reg(state, VGA_MODE, 0x4);
        *tune_state = CT_TUNER_STEP_4;
        break;

    case CT_TUNER_STEP_4:
        state->adc_diff -= dib0090_read_reg(state, ADCVAL);
        *tune_state = CT_TUNER_STEP_5;
        ret = 0;
        break;

    default:
        break;
    }

    // dbgpl(&dib0090_dbg, "Pos=%.3f;Neg=%.3f;Delta=%.3f", wbdp*1.8/1024, wbdn*1.8/1024,(wbdp-wbdn)*1.8/1024);
    return ret;
}

struct dc_calibration {
    uint8_t addr;
    uint8_t offset;
    uint8_t pga;
    uint16_t bb1;
    uint8_t i;
};

static const struct dc_calibration dc_table[] =
{
    /* Step1 BB gain1= 26 with boost 1, gain 2 = 0 */
    { CTRL_BB_6,  5, 1, (1 << 13) | (0  << 8) | (26 << 3), 1 }, // offset_trim2_i_chann  0   0   5    0    0    1    6     9    5
    { CTRL_BB_7, 11, 1, (1 << 13) | (0  << 8) | (26 << 3), 0 }, // offset_trim2_q_chann  0   0   5    0    0    1    7     15   11
    /* Step 2 BB gain 1 = 26 with boost = 1 & gain 2 = 29 */
    { CTRL_BB_6,  0, 0, (1 << 13) | (29 << 8) | (26 << 3), 1 }, // offset_trim1_i_chann  0   0   5    0    0    1    6     4    0
    { CTRL_BB_6, 10, 0, (1 << 13) | (29 << 8) | (26 << 3), 0 }, // offset_trim1_q_chann  0   0   5    0    0    1    6     14   10
    { 0 },
};

static void dib0090_set_trim(struct dib0090_state *state)
{
    uint16_t *val;

    if (state->dc->addr == CTRL_BB_7)
        val = &state->bb7;
    else
        val = &state->bb6;

    *val &= ~(0x1f << state->dc->offset);
    *val |= state->step << state->dc->offset;

    dib0090_write_reg(state, state->dc->addr, *val);
}

static int dib0090_dc_offset_calibration(struct dib0090_state *state, enum frontend_tune_state *tune_state)
{
    int ret = 0;

    switch (*tune_state) {

    case CT_TUNER_START:
        /* init */
        dbgpl(&dib0090_dbg, "Internal DC calibration");

        /* the LNA is off */
        dib0090_write_reg(state, TUNER_EN, 0x02ed);

        /* force vcm2 = 0.8V */
        state->bb6 = 0;
        state->bb7 = 0x040d;

        state->dc = dc_table;

        *tune_state = CT_TUNER_STEP_0;

        /* fall through */

    case CT_TUNER_STEP_0:
        dib0090_write_reg(state, CTRL_BB_1, state->dc->bb1);
        dib0090_write_reg(state, CTRL_BB_7, state->bb7 | (state->dc->i << 7));

        state->step = 0;

        state->min_adc_diff = 1023;

        *tune_state = CT_TUNER_STEP_1;
        ret = 50;
        break;

    case CT_TUNER_STEP_1:
        dib0090_set_trim(state);

        *tune_state = CT_TUNER_STEP_2;
        break;

    case CT_TUNER_STEP_2:
    case CT_TUNER_STEP_3:
    case CT_TUNER_STEP_4:
        ret = dib0090_get_offset(state, tune_state);
        break;

    case CT_TUNER_STEP_5: /* found an offset */
        dbgpl(&dib0090_dbg, "FE%d: IQC read=%d, current=%x", state->fe->id, (uint32_t) state->adc_diff, state->step);

        /* first turn for this frequency */
        if (state->step == 0) {
            if (state->dc->pga && state->adc_diff < 0)
                state->step = 0x10;
            if (state->dc->pga == 0 && state->adc_diff > 0)
                state->step = 0x10;
        }

        state->adc_diff = ABS(state->adc_diff);

        if (state->adc_diff < state->min_adc_diff && steps(state->step) < 15) { /* stop search when the delta to 0 is increasing */
            state->step++;
            state->min_adc_diff = state->adc_diff;
            *tune_state = CT_TUNER_STEP_1;
        } else {

            /* the minimum was what we have seen in the step before */
            state->step--;
            dib0090_set_trim(state);

            dbgpl(&dib0090_dbg, "FE%d: BB Offset Cal, BBreg=%hd,Offset=%hd,Value Set=%hd", state->fe->id, state->dc->addr, state->adc_diff, state->step);

            state->dc++;
            if (state->dc->addr == 0) /* done */
                *tune_state = CT_TUNER_STEP_6;
            else
                *tune_state = CT_TUNER_STEP_0;

        }
        break;

    case CT_TUNER_STEP_6:
        dib0090_write_reg(state, CTRL_BB_7, state->bb7 & ~0x0008); //Force the test bus to be off
        dib0090_write_reg(state, VGA_MODE, 0x7);
        *tune_state = CT_TUNER_START; /* reset done -> real tuning can now begin */
        state->reset &= ~0x1;
    default:
        break;
    }
    return ret;
}
/********************End DC Calib ***************************/

static int dib0090_wbd_calibration(struct dib0090_state *state, enum frontend_tune_state *tune_state)
{
    switch (*tune_state) {
    case CT_TUNER_START:
        /* WBD-mode=log, Bias=2, Gain=6, Testmode=1, en=1, WBDMUX=1 */
        dib0090_write_reg(state, CTRL_WBDMUX, 0xdb09 | (1<<10)); //Force the WBD enable
        dib0090_write_reg(state, TUNER_EN, EN_UHF & 0x0fff); //Discard the LNA

        *tune_state = CT_TUNER_STEP_0;
        return 90; /* wait for the WBDMUX to switch and for the ADC to sample */
    case CT_TUNER_STEP_0:
        state->wbd_offset = dib0090_read_reg(state, ADCVAL);
        dbgpl(&dib0090_dbg, "WBD calibration offset = %d", state->wbd_offset);

        *tune_state = CT_TUNER_START; /* reset done -> real tuning can now begin */
        state->reset &= ~0x2;
        break;
    default:
        break;
    }
    return 0;
}


/* Changes the baseband filter settings */
static void dib0090_set_bandwidth(struct dib0090_state *state, struct dibChannel *ch)
{
	uint16_t tmp;

    if (ch->bandwidth_kHz <= 5000)
        tmp = (3 << 14);
    else if (ch->bandwidth_kHz <= 6000)
        tmp = (2 << 14);
    else if (ch->bandwidth_kHz <= 7000)
        tmp = (1 << 14);
    else
        tmp = (0 << 14);

    state->bb_1_def &= 0x3fff;
    state->bb_1_def |= tmp;

    dib0090_write_reg(state, CTRL_BB_1, state->bb_1_def); /* be sure that we have the right bb-filter */
}

struct slope
{
    int16_t range;
    int16_t slope;
};

static uint16_t slopes_to_scale(const struct slope *slopes, uint8_t num, int16_t val)
{
    uint8_t i;
    uint16_t rest;
    uint16_t ret = 0;
    for (i = 0; i < num; i++) {
        if (val  > slopes[i].range)
            rest = slopes[i].range;
        else
            rest = val;
        ret += (rest * slopes[i].slope) / slopes[i].range;
        val -= rest;
    }
    return ret;
}

//static struct slope dib0090_wbd_slopes[] =
//{
//	{  0, 1000,642 },
//	{  70, 550,35 }, //Between Offset and Offset+70 the power is between -100dBm and -55dBm
//	{ 768, 300, 117}, //Between Offset+70 and Offset+768 the power is between -55dBm and -30dBm
//	{ 1024, 0,0 },//Between Offset+768 and Offset+1024 the power is between -30dBm and -10dBm
//};

/* this array only covers the variable part of the wbd values, above the floor */
static const struct slope dib0090_wbd_slopes[3] =
{
    {  66, 120 }, /* -64,-52: offset -   65 */
    { 600, 170 }, /* -52,-35: 65     -  665 */
    { 170, 250 }, /* -45,-10: 665    - 835 */
};

static int16_t dib0090_wbd_to_db(struct dib0090_state *state, uint16_t wbd)
{
    wbd &= 0x3ff;
    if (wbd < state->wbd_offset)
        wbd = 0;
    else
        wbd -= state->wbd_offset;
    /* -64dB is the floor */
    return -640 + (int16_t) slopes_to_scale(dib0090_wbd_slopes, ARRAY_SIZE(dib0090_wbd_slopes), wbd);
}

static void dib0090_set_rframp(struct dib0090_state *state, const uint16_t *cfg)
{
    state->rf_ramp = cfg;
}

static void dib0090_set_boost(struct dib0090_state *state, int onoff)
{
    state->bb_1_def &= 0xdfff;
    state->bb_1_def |= onoff << 13;
}

static void dib0090_set_bbramp(struct dib0090_state *state, const uint16_t *cfg)
{
    state->bb_ramp = cfg;
    dib0090_set_boost(state, cfg[0] > 500); /* we want the boost if the gain is higher that 50dB */
}

static const uint16_t rf_ramp_uhf[] =
{
    412, /* max RF gain in 10th of dB */
    132, 307, 127, /* LNA1  : total gain = 13.2dB, point on the ramp where this amp is full gain, value to write to get full gain */
    105, 412, 255, /* LNA2  : 10.5 dB */
     50,  50, 127, /* LNA3  :  5.0 dB */
    125, 175, 127, /* LNA4  : 12.5 dB */
      0,   0, 127, /* CBAND :  0.0 dB */
};

static const uint16_t rf_ramp_vhf[] =
{
    412, /* max RF gain in 10th of dB */
    132, 307, 127, /* LNA1,  13.2dB */
    105, 412, 255, /* LNA2,  10.5dB */
     50,  50, 127, /* LNA3,  5dB */
    125, 175, 127, /* LNA4,  12.5dB */
      0,   0, 127, /* CBAND, 0dB */
};

static const uint16_t rf_ramp_cband[] =
{
   332, /* max RF gain in 10th of dB */
   132, 252, 127, /* LNA1,  dB */
   80,  332, 255, /* LNA2,  dB */
   0,   0,   127, /* LNA3,  dB */
   0,   0,   127, /* LNA4,  dB */
   120, 120, 127, /* LT1 CBAND */
};

static const uint16_t rf_ramp_sband[] =
{
    253, /* max RF gain in 10th of dB */
    141, 141, 127, /* LNA1,  14.1dB */
    112, 253, 255, /* LNA2,  11.2dB */
      0,   0, 127, /* LNA3,  0dB */
      0,   0, 127, /* LNA4,  0dB */
      0,   0, 127, /* CBAND, 0dB */
};

static const uint16_t bb_ramp_boost[] =
{
    550, /* max BB gain in 10th of dB */
    260, 260,  26, /* BB1, 26dB */
    290, 550,  29, /* BB2, 29dB */
};

#if 0
static const uint16_t bb_ramp_normal[] =
{
    500, /* max BB gain in 10th of dB */
    210, 210,  21, /* BB1, 21dB */
    290, 500,  29, /* BB2, 29dB */
};
#endif

#ifdef CONFIG_DIB0090_USE_PWM_AGC

static void dib0090_set_rframp_pwm(struct dib0090_state *state, const uint16_t *cfg)
{
    state->rf_ramp = cfg;

    dib0090_write_reg(state, RF_RAMP1, 0xffff); //(state->config->io.clock_khz / cfg[1]) * 100);

    dbgpl(&dib0090_dbg, "total RF gain: %ddB, step: %d", (uint32_t) cfg[0], dib0090_read_reg(state, RF_RAMP1));

    dib0090_write_regs(state, RF_RAMP3, cfg + 3, 6);
    dib0090_write_regs(state, GAIN4_1,  cfg + 9, 2);
}

static void dib0090_set_bbramp_pwm(struct dib0090_state *state, const uint16_t *cfg)
{
    state->bb_ramp = cfg;

    dib0090_set_boost(state, cfg[0] > 500); /* we want the boost if the gain is higher that 50dB */

    dib0090_write_reg(state, BB_RAMP1, 0xffff); //(state->config->io.clock_khz / cfg[1]) * 100);
    dbgpl(&dib0090_dbg, "total BB gain: %ddB, step: %d", (uint32_t) cfg[0], dib0090_read_reg(state, BB_RAMP1));
    dib0090_write_regs(state, BB_RAMP3, cfg + 3, 4);
}

static const uint16_t rf_ramp_pwm_vhf[] =
{
    404, /* max RF gain in 10th of dB */
    25, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> RF_RAMP2 */
    1011, /* ramp_max = maximum X used on the ramp */
    (6  << 10) | 417, /* RF_RAMP3, LNA 1 = 13.2dB */
    (0  << 10) | 756, /* RF_RAMP4, LNA 1 */
    (16 << 10) | 756, /* RF_RAMP5, LNA 2 = 10.5dB */
    (0  << 10) | 1011, /* RF_RAMP6, LNA 2 */
    (16 << 10) | 290, /* RF_RAMP7, LNA 3 = 5dB */
    (0  << 10) | 417, /* RF_RAMP8, LNA 3 */
    (7  << 10) | 0, /* GAIN_4_1, LNA 4 = 12.5dB */
    (0  << 10) | 290, /* GAIN_4_2, LNA 4 */
};
static const uint16_t rf_ramp_pwm_uhf[] =
{
    404, /* max RF gain in 10th of dB */
    25, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> RF_RAMP2 */
    1011, /* ramp_max = maximum X used on the ramp */
    (6  << 10) | 417, /* RF_RAMP3, LNA 1 = 13.2dB */
    (0  << 10) | 756, /* RF_RAMP4, LNA 1 */
    (16 << 10) | 756, /* RF_RAMP5, LNA 2 = 10.5dB */
    (0  << 10) | 1011, /* RF_RAMP6, LNA 2 */
    (16 << 10) | 0, /* RF_RAMP7, LNA 3 = 5dB */
    (0  << 10) | 127, /* RF_RAMP8, LNA 3 */
    (7  << 10) | 127, /* GAIN_4_1, LNA 4 = 12.5dB */
    (0  << 10) | 417, /* GAIN_4_2, LNA 4 */
};
static const uint16_t rf_ramp_pwm_sband[] =
{
    253, /* max RF gain in 10th of dB */
    38, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> RF_RAMP2 */
    961,
    (4  << 10) | 0, /* RF_RAMP3, LNA 1 = 14.1dB */
    (0  << 10) | 508, /* RF_RAMP4, LNA 1 */
    (9  << 10) | 508, /* RF_RAMP5, LNA 2 = 11.2dB */
    (0  << 10) | 961, /* RF_RAMP6, LNA 2 */
    (0  << 10) | 0, /* RF_RAMP7, LNA 3 = 0dB */
    (0  << 10) | 0, /* RF_RAMP8, LNA 3 */
    (0  << 10) | 0, /* GAIN_4_1, LNA 4 = 0dB */
    (0  << 10) | 0, /* GAIN_4_2, LNA 4 */
};
static const uint16_t rf_ramp_pwm_cband[] =
{
    0, /* max RF gain in 10th of dB */
    0, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> RF_RAMP2 */
    0, /* ramp_max = maximum X used on the ramp */
    (0  << 10) | 0, /* RF_RAMP3, LNA 1 = 0dB */
    (0  << 10) | 0, /* RF_RAMP4, LNA 1 */
    (0  << 10) | 0, /* RF_RAMP5, LNA 2 = 0dB */
    (0  << 10) | 0, /* RF_RAMP6, LNA 2 */
    (0  << 10) | 0, /* RF_RAMP7, LNA 3 = 0dB */
    (0  << 10) | 0, /* RF_RAMP8, LNA 3 */
    (0  << 10) | 0, /* GAIN_4_1, LNA 4 = 0dB */
    (0  << 10) | 0, /* GAIN_4_2, LNA 4 */
};
static const uint16_t bb_ramp_pwm_normal[] =
{
    500, /* max RF gain in 10th of dB */
    8, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> BB_RAMP2 */
    400,
    (2  << 9) | 0, /* BB_RAMP3 = 21dB */
    (0  << 9) | 168, /* BB_RAMP4 */
    (2  << 9) | 168, /* BB_RAMP5 = 29dB */
    (0  << 9) | 400, /* BB_RAMP6 */
};
static const uint16_t bb_ramp_pwm_boost[] =
{
    550, /* max RF gain in 10th of dB */
    8, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> BB_RAMP2 */
    440,
    (2  << 9) | 0, /* BB_RAMP3 = 26dB */
    (0  << 9) | 208, /* BB_RAMP4 */
    (2  << 9) | 208, /* BB_RAMP5 = 29dB */
    (0  << 9) | 440, /* BB_RAMP6 */
};
#endif

static void dib0090_wbd_target(struct dib0090_state *state, uint32_t rf)
{
    uint16_t offset = 250;

    /* TODO : DAB digital N+/-1 interferer perfs : offset = 10 */

    if (state->current_band == BAND_VHF)
        offset = 650;
#ifndef FIRMWARE_FIREFLY
    if (state->current_band == BAND_VHF)
       offset = state->config->wbd_vhf_offset;
    if (state->current_band == BAND_CBAND)
       offset = state->config->wbd_cband_offset;
#endif
    //for (offset = 0; offset < 1000; offset += 4)
    //    dbgp("offset = %d -> %d\n", offset, dib0090_wbd_to_db(state, offset));

	state->wbd_target = dib0090_wbd_to_db(state, state->wbd_offset + offset); // get the value in dBm from the offset
    dbgpl(&dib0090_dbg, "wbd-target: %d dB", (uint32_t) state->wbd_target);
}

static const int gain_reg_addr[4] =
{
    CTRL_RXRF1, CTRL_RXRF3, CTRL_RF_LT, CTRL_BB_1
};

static void dib0090_gain_apply(struct dib0090_state *state, int16_t gain_delta, int16_t top_delta, uint8_t force)
{
    uint16_t rf, bb, ref;
    uint16_t i, v, gain_reg[4] = { 0 }, gain;
    const uint16_t *g;

	if (top_delta < -511)
		top_delta = -511;
	if (top_delta > 511)
		top_delta = 511;

    if (force) {
        top_delta *= (1 << WBD_ALPHA);
        gain_delta *= (1 << GAIN_ALPHA);
    }

    if(top_delta >= ((int16_t)(state->rf_ramp[0] << WBD_ALPHA) - state->rf_gain_limit)) /* overflow*/
        state->rf_gain_limit = state->rf_ramp[0] << WBD_ALPHA;
    else
        state->rf_gain_limit += top_delta;

    if (state->rf_gain_limit < 0) /*underflow*/
        state->rf_gain_limit = 0;

    /* use gain as a temporary variable and correct current_gain */
    gain = ((state->rf_gain_limit >> WBD_ALPHA) + state->bb_ramp[0]) << GAIN_ALPHA;
    if(gain_delta >= ((int16_t)gain - state->current_gain)) /* overflow*/
	  state->current_gain = gain ;
	else
	  state->current_gain += gain_delta;
    /* cannot be less than 0 (only if gain_delta is less than 0 we can have current_gain < 0) */
    if (state->current_gain < 0)
        state->current_gain = 0;

    /* now split total gain to rf and bb gain */
    gain = state->current_gain >> GAIN_ALPHA;

    /* requested gain is bigger than rf gain limit - ACI/WBD adjustment */
    if (gain > (state->rf_gain_limit >> WBD_ALPHA)) {
        rf = state->rf_gain_limit >> WBD_ALPHA;
        bb = gain - rf;
        if (bb > state->bb_ramp[0])
            bb = state->bb_ramp[0];
    } else { /* high signal level -> all gains put on RF */
        rf = gain;
        bb = 0;
    }

    state->gain[0] = rf;
    state->gain[1] = bb;

    /* software ramp */
    /* Start with RF gains */
    g = state->rf_ramp + 1; /* point on RF LNA1 max gain */
    ref = rf;
    for (i = 0; i < 7; i++) { /* Go over all amplifiers => 5RF amps + 2 BB amps = 7 amps */
        if (g[0] == 0 || ref < (g[1] - g[0])) /* if total gain of the current amp is null or this amp is not concerned because it starts to work from an higher gain value */
            v = 0; /* force the gain to write for the current amp to be null */
        else if (ref >= g[1]) /* Gain to set is higher than the high working point of this amp */
            v = g[2]; /* force this amp to be full gain */
        else /* compute the value to set to this amp because we are somewhere in his range */
            v = ((ref - (g[1] - g[0])) * g[2]) / g[0];

        if (i == 0) /* LNA 1 reg mapping */
            gain_reg[0] = v;
        else if(i == 1) /* LNA 2 reg mapping */
            gain_reg[0] |= v << 7;
        else if(i == 2) /* LNA 3 reg mapping */
            gain_reg[1] = v;
        else if(i == 3) /* LNA 4 reg mapping */
            gain_reg[1] |= v << 7;
        else if(i == 4) /* CBAND LNA reg mapping */
            gain_reg[2] = v | state->rf_lt_def;
        else if(i == 5) /* BB gain 1 reg mapping */
            gain_reg[3] = v << 3;
        else if(i == 6) /* BB gain 2 reg mapping */
            gain_reg[3] |= v << 8;

        g += 3; /* go to next gain bloc */

        /* When RF is finished, start with BB */
        if (i == 4) {
            g = state->bb_ramp + 1; /* point on BB gain 1 max gain */
            ref = bb;
        }
    }
    gain_reg[3] |= state->bb_1_def;
    gain_reg[3] |= ((bb % 10) * 100) / 125;

#ifdef DEBUG_AGC
    dbgpl(&dib0090_dbg, "GA CALC: DB: %3d(rf) + %3d(bb) = %3d gain_reg[0]=%04x gain_reg[1]=%04x gain_reg[2]=%04x gain_reg[0]=%04x", rf, bb, rf + bb,
        gain_reg[0], gain_reg[1], gain_reg[2], gain_reg[3]);
#endif

    /* Write the amplifier regs */
    for (i = 0; i < 4; i++) {
        v = gain_reg[i];
        if (force || state->gain_reg[i] != v) {
            state->gain_reg[i]  = v;
            dib0090_write_reg(state, gain_reg_addr[i], v);
        }
    }
}

#ifdef CONFIG_DIB0090_USE_PWM_AGC

extern void dib0090_dcc_freq(struct dibFrontend *fe,uint8_t fast)
{
  struct dib0090_state *state = fe->tuner_priv;
  if(fast)
    dib0090_write_reg(state, CTRL_BB_4, 0);//1kHz
  else
    dib0090_write_reg(state, CTRL_BB_4, 1);//almost frozen
}

#endif

#ifdef CONFIG_DIB0090_USE_PWM_AGC

void dib0090_pwm_gain_reset(struct dibFrontend *fe, struct dibChannel *ch)
{
    struct dib0090_state *state = fe->tuner_priv;
    /* reset the AGC */

    if (state->config->use_pwm_agc) {
#ifdef CONFIG_BAND_SBAND
        if (state->current_band == BAND_SBAND) {
            dib0090_set_rframp_pwm(state, rf_ramp_pwm_sband);
            dib0090_set_bbramp_pwm(state, bb_ramp_pwm_boost);
        } else
#endif
#ifdef CONFIG_BAND_CBAND
        if (state->current_band == BAND_CBAND) {
              dib0090_set_rframp_pwm(state, rf_ramp_pwm_cband);
              dib0090_set_bbramp_pwm(state, bb_ramp_pwm_normal);
        } else
#endif
#ifdef CONFIG_BAND_VHF
        if (state->current_band == BAND_VHF) {
            dib0090_set_rframp_pwm(state, rf_ramp_pwm_vhf);
            dib0090_set_bbramp_pwm(state, bb_ramp_pwm_normal);
        } else
#endif
        {
            dib0090_set_rframp_pwm(state, rf_ramp_pwm_uhf);
            dib0090_set_bbramp_pwm(state, bb_ramp_pwm_normal);
        }

        // activate the ramp generator using PWM control
        if (state->rf_ramp[0] != 0)
            dib0090_write_reg(state, RF_RAMP9, (3 << 11));
        else
            dib0090_write_reg(state, RF_RAMP9, (0 << 11));

        dib0090_write_reg(state, BB_RAMP7, (1 << 10));// 0 gain by default
    }
}
#endif

static void dib0090_monitoring_refresh(struct dib0090_state *state, struct dibChannelMonitor *mon, uint16_t wbd_val)
{
    /* monitoring refresh was requested */
#ifndef FIRMWARE_FIREFLY /* AGC monitoring on firefly takes 450 Bytes */
    if (mon->refresh & FE_COMPONENT_TUNER) {
#endif
        mon->agc_global    = (uint16_t) (((uint32_t) (state->current_gain >> GAIN_ALPHA) * 0xffff) / (state->rf_ramp[0] + state->bb_ramp[0]));
        mon->agc_rf        = (uint16_t) (((uint32_t) state->gain[0] * 0xffff) / state->rf_ramp[0]);
        mon->agc_bb        = (uint16_t) (((uint32_t) state->gain[1] * 0xffff) / state->bb_ramp[0]);
        mon->agc_wbd       = wbd_val << 2; /* here wbd is on 10 bits - previously it was on 12 */
        mon->agc_wbd_split = (uint16_t) (((state->rf_gain_limit >> WBD_ALPHA) * 0xff) / state->rf_ramp[0]);
#ifndef FIRMWARE_FIREFLY
        mon->refresh &= ~FE_COMPONENT_TUNER;
    }
#endif
}

/* software AGC function */
int dib0090_gain_control(struct dibFrontend *fe, struct dibChannel *ch)
{
    struct dib0090_state *state = fe->tuner_priv;
    enum frontend_tune_state *tune_state = &fe->tune_state;
    int ret = 10;
    struct dibChannelMonitor *mon = channel_frontend_monitoring(ch, fe->id);
    uint16_t wbd_val = 0;
    uint8_t apply_gain_immediatly = 1;
    int16_t wbd_error = 0, adc_error = 0;

    if (*tune_state == CT_AGC_START) {
        state->agc_freeze = 0;
        dib0090_write_reg(state, CTRL_BB_4, 0x0);

#ifdef CONFIG_BAND_SBAND
        if (state->current_band == BAND_SBAND) {
            dib0090_set_rframp(state, rf_ramp_sband);
            dib0090_set_bbramp(state, bb_ramp_boost);
        } else
#endif
#ifdef CONFIG_BAND_VHF
        if (state->current_band == BAND_VHF) {
            dib0090_set_rframp(state, rf_ramp_vhf);
            dib0090_set_bbramp(state, bb_ramp_boost);
        } else
#endif
#ifdef CONFIG_BAND_CBAND
        if (state->current_band == BAND_CBAND) {
            dib0090_set_rframp(state, rf_ramp_cband);
            dib0090_set_bbramp(state, bb_ramp_boost);
        } else
#endif
        {
            dib0090_set_rframp(state, rf_ramp_uhf);
            dib0090_set_bbramp(state, bb_ramp_boost);
        }

        // deactivate the ramp generator using PWM control
        dib0090_write_reg(state, RF_RAMP9, 0);
        dib0090_write_reg(state, BB_RAMP7, 0);

        dib0090_wbd_target(state, fe->current_rf);

        state->rf_gain_limit = state->rf_ramp[0] << WBD_ALPHA;
        state->current_gain = ((state->rf_ramp[0] + state->bb_ramp[0]) / 2) << GAIN_ALPHA;

        *tune_state = CT_AGC_STEP_0;
    } else if (!state->agc_freeze) {
        int16_t wbd;

        int adc;
        wbd_val = dib0090_read_reg(state, ADCVAL);

        /* read and calc the wbd power */
        wbd       = dib0090_wbd_to_db(state, wbd_val);
        wbd_error = state->wbd_target - wbd;

        if (*tune_state == CT_AGC_STEP_0) {
            if (wbd_error < 0 && state->rf_gain_limit > 0) {
#ifdef CONFIG_BAND_CBAND
                /* in case of CBAND tune reduce first the lt_gain2 before adjusting the RF gain */
                uint8_t ltg2 = (state->rf_lt_def >> 10) & 0x7;
                if (state->current_band == BAND_CBAND && ltg2) {
                    ltg2 >>= 1;
                    state->rf_lt_def &= ltg2 << 10; /* reduce in 3 steps from 7 to 0 */
                }
#endif
            } else {
                state->agc_step = 0;
                *tune_state = CT_AGC_STEP_1;
            }
        } else {
            /* calc the adc power */
            adc = state->config->get_adc_power(fe);
            adc = (adc * ((int32_t) 355774) + (((int32_t)1) << 20)) >> 21;  /* included in [0:-700] */

            adc_error = (int16_t)( ((int32_t) ADC_TARGET) - adc);
#ifdef CONFIG_STANDARD_DAB
            if (ch->type == STANDARD_DAB)
                adc_error += 130;
#endif
#ifdef CONFIG_STANDARD_DVBT
            if (ch->type == STANDARD_DVBT &&
                    (ch->u.dvbt.constellation == QAM_64QAM || ch->u.dvbt.constellation == QAM_16QAM) )
                adc_error += 60;
#endif
#ifdef CONFIG_STANDARD_ISDBT
            if ((ch->type == STANDARD_ISDBT)&&(
				 ((ch->u.isdbt.layer[0].nb_segments>0)
					&&((ch->u.isdbt.layer[0].constellation == QAM_64QAM)||(ch->u.isdbt.layer[0].constellation == QAM_16QAM)))
				 ||((ch->u.isdbt.layer[1].nb_segments>0)
					  &&((ch->u.isdbt.layer[1].constellation == QAM_64QAM)||(ch->u.isdbt.layer[1].constellation == QAM_16QAM)))
				 ||((ch->u.isdbt.layer[2].nb_segments>0)
					  &&((ch->u.isdbt.layer[2].constellation == QAM_64QAM)||(ch->u.isdbt.layer[2].constellation == QAM_16QAM)))
				 )
				)
			  adc_error += 60;
#endif

            if (*tune_state == CT_AGC_STEP_1) { /* quickly go to the correct range of the ADC power */
                if (ABS(adc_error) < 50 || state->agc_step++ > 5) {

#ifdef CONFIG_STANDARD_DAB
                    if (ch->type == STANDARD_DAB) {
                        dib0090_write_reg(state, CTRL_BB_2, (1<<15)|(15<<11)|(31<<6)|(63));/* cap value = 63 : narrow BB filter : Fc = 1.8MHz */
                        dib0090_write_reg(state, CTRL_BB_4, 0x0);
                    } else
#endif
                    {
                        dib0090_write_reg(state, CTRL_BB_2, (1<<15)|(3<<11)|(6<<6)|(32));
                        dib0090_write_reg(state, CTRL_BB_4, 0x01); /*0 = 1KHz ; 1 = 150Hz ; 2 = 50Hz ; 3 = 50KHz ; 4 = servo fast*/
                    }

                    *tune_state = CT_AGC_STOP;
                }
            } else {
                /* everything higher than or equal to CT_AGC_STOP means tracking */
                ret = 100; /* 10ms interval */
                apply_gain_immediatly = 0;
            }
        }
#ifdef DEBUG_AGC
        dbgpl(&dib0090_dbg, "FE: %d, tune state %d, ADC = %3ddB (ADC err %3d) WBD %3ddB (WBD err %3d, WBD val SADC: %4d), RFGainLimit (TOP): %3d, signal: %3ddBm",
        (uint32_t) fe->id, (uint32_t) *tune_state, (uint32_t) adc, (uint32_t) adc_error, (uint32_t) wbd, (uint32_t) wbd_error, (uint32_t) wbd_val,
        (uint32_t) state->rf_gain_limit >> WBD_ALPHA, (int32_t) 200 + adc - (state->current_gain >> GAIN_ALPHA));
#endif
    }

    dib0090_monitoring_refresh(state, mon, wbd_val);

    /* apply gain */
    if (!state->agc_freeze)
        dib0090_gain_apply(state, adc_error, wbd_error, apply_gain_immediatly);
    return ret;
}

void dib0090_get_current_gain(struct dibFrontend *fe, uint16_t *rf, uint16_t *bb, uint16_t *rf_gain_limit, uint16_t *rflt)
{
    struct dib0090_state *state = fe->tuner_priv;
    if (rf)
        *rf = state->gain[0];
    if (bb)
        *bb = state->gain[1];
    if (rf_gain_limit)
        *rf_gain_limit = state->rf_gain_limit;
    if (rflt)
        *rflt = (state->rf_lt_def >> 10) & 0x7;
}

struct dib0090_tuning {
    uint32_t max_freq; /* for every frequency less than or equal to that field: this information is correct */
    uint8_t switch_trim;
    uint8_t lna_tune;
    uint8_t lna_bias;
    uint16_t v2i;
    uint16_t mix;
    uint16_t load;
    uint16_t tuner_enable;
};

struct dib0090_pll {
    uint32_t max_freq; /* for every frequency less than or equal to that field: this information is correct */
    uint8_t vco_band;
    uint8_t hfdiv_code;
    uint8_t hfdiv;
    uint8_t topresc;
};

static const struct dib0090_pll dib0090_pll_table[] =
{
#ifdef CONFIG_BAND_CBAND
    { 56000, 0, 9, 48, 6},  // CAB
    { 70000, 1, 9, 48, 6},  // CAB
    { 87000, 0, 8, 32, 4},  // CAB
    { 105000, 1, 8, 32, 4}, // FM
    { 115000, 0, 7, 24, 6}, // FM
    { 140000, 1, 7, 24, 6}, // MID FM VHF
    { 170000, 0, 6, 16, 4}, // MID FM VHF
#endif
#ifdef CONFIG_BAND_VHF
    { 200000, 1, 6, 16, 4}, // VHF
    { 230000, 0, 5, 12, 6}, // VHF
    { 280000, 1, 5, 12, 6}, // MID VHF UHF
    { 340000, 0, 4, 8, 4},  // MID VHF UHF
    { 380000, 1, 4, 8, 4},  // MID VHF UHF
    { 450000, 0, 3, 6, 6},  // MID VHF UHF
#endif
#ifdef CONFIG_BAND_UHF
    { 580000, 1, 3,  6, 6}, // UHF
    { 700000, 0, 2,  4, 4}, // UHF
    { 860000, 1, 2,  4, 4}, // UHF
#endif
#ifdef CONFIG_BAND_LBAND
    { 1800000, 1, 0,  2, 4}, // LBD
#endif
#ifdef CONFIG_BAND_SBAND
    { 2900000, 0, 14, 1, 4}, // SBD
#endif
};

static const struct dib0090_tuning dib0090_tuning_table_fm_vhf_on_cband[] =
{
    //max_freq, switch_trim, lna_tune, lna_bias, v2i, mix, load, tuner_enable;
#ifdef CONFIG_BAND_CBAND
    { 184000,  4, 1, 15, 0x280, 0x2912, 0xb94e, EN_CAB }, // FM EN_CAB
    { 227000,  4, 3, 15, 0x280, 0x2912, 0xb94e, EN_CAB }, // FM EN_CAB
    { 380000,  4, 7, 15, 0x280, 0x2912, 0xb94e, EN_CAB }, // FM EN_CAB
#endif
#ifdef CONFIG_BAND_UHF
    { 520000,  2, 0, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 550000,  2, 2, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 650000,  2, 3, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 750000,  2, 5, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 850000,  2, 6, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 900000,  2, 7, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
#endif
#ifdef CONFIG_BAND_LBAND
    { 1500000, 4, 0, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
    { 1600000, 4, 1, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
    { 1800000, 4, 3, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
#endif
#ifdef CONFIG_BAND_SBAND
    { 2300000, 1, 4, 20, 0x300, 0x2d2A, 0x82c7, EN_SBD }, // SBD EN_SBD
    { 2900000, 1, 7, 20, 0x280, 0x2deb, 0x8347, EN_SBD }, // SBD EN_SBD
#endif
};

static const struct dib0090_tuning dib0090_tuning_table[] =
{
    //max_freq, switch_trim, lna_tune, lna_bias, v2i, mix, load, tuner_enable;
#ifdef CONFIG_BAND_CBAND
    { 170000,  4, 1, 15, 0x280, 0x2912, 0xb94e, EN_CAB }, // FM EN_CAB
#endif
#ifdef CONFIG_BAND_VHF
    { 184000,  1, 1, 15, 0x300, 0x4d12, 0xb94e, EN_VHF }, // VHF EN_VHF
    { 227000,  1, 3, 15, 0x300, 0x4d12, 0xb94e, EN_VHF }, // VHF EN_VHF
    { 380000,  1, 7, 15, 0x300, 0x4d12, 0xb94e, EN_VHF }, // VHF EN_VHF
#endif
#ifdef CONFIG_BAND_UHF
    { 520000,  2, 0, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 550000,  2, 2, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 650000,  2, 3, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 750000,  2, 5, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 850000,  2, 6, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 900000,  2, 7, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
#endif
#ifdef CONFIG_BAND_LBAND
    { 1500000, 4, 0, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
    { 1600000, 4, 1, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
    { 1800000, 4, 3, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
#endif
#ifdef CONFIG_BAND_SBAND
    { 2300000, 1, 4, 20, 0x300, 0x2d2A, 0x82c7, EN_SBD }, // SBD EN_SBD
    { 2900000, 1, 7, 20, 0x280, 0x2deb, 0x8347, EN_SBD }, // SBD EN_SBD
#endif
};

static int dib0090_tune(struct dibFrontend *fe, struct dibChannel *ch)
{
    struct dib0090_state *state = fe->tuner_priv;
    const struct dib0090_tuning *tune = state->current_tune_table_index;
    const struct dib0090_pll *pll = state->current_pll_table_index;
    enum frontend_tune_state *tune_state = &fe->tune_state;

    uint32_t rf;
    uint16_t lo4 = 0xe900, lo5, lo6, Den;
    uint32_t FBDiv, Rest, FREF, VCOF_kHz = 0;
    uint16_t tmp, adc;
    int8_t step_sign;
    int ret = 10; /* 1ms is the default delay most of the time */
    uint8_t c,i;

    state->current_band = (uint8_t)BAND_OF_FREQUENCY(ch->RF_kHz);
    rf = ch->RF_kHz + (state->current_band == BAND_UHF ? state->config->freq_offset_khz_uhf : state->config->freq_offset_khz_vhf);
    /* in any case we first need to do a reset if needed */
    if (state->reset & 0x1)
        return dib0090_dc_offset_calibration(state, tune_state);
    else if (state->reset & 0x2)
        return dib0090_wbd_calibration(state, tune_state);

    /************************* VCO ***************************/
    /* Default values for FG                                 */
    /* from these are needed :                               */
    /* Cp,HFdiv,VCOband,SD,Num,Den,FB and REFDiv             */

    // in ISDB-T sb mode we shift tuning frequency
#ifdef CONFIG_STANDARD_ISDBT
    if (ch->type == STANDARD_ISDBT && ch->u.isdbt.sb_mode == 1)
        rf += 850;
#endif

    //dbgpl(&dib0090_dbg, "fe%hd: Tuning for Band: %d (%d kHz); tune_state = %d",fe->id, state->current_band, rf, fe->tune_state);

    if (fe->current_rf != rf) {
        state->tuner_is_tuned = 0;

        tune = dib0090_tuning_table;

        tmp = (state->revision>>5)&0x7;
        if(tmp == 0x4 || tmp == 0x7 ) {
            /* CBAND tuner version for VHF */
            if(state->current_band == BAND_FM || state->current_band == BAND_VHF) {
                /* Force CBAND */
                state->current_band = BAND_CBAND;
                tune = dib0090_tuning_table_fm_vhf_on_cband;
            }
        }

        pll = dib0090_pll_table;
        /* Look for the interval */
        while (rf > tune->max_freq)
            tune++;
        while (rf > pll->max_freq)
            pll++;
        state->current_tune_table_index = tune;
        state->current_pll_table_index = pll;
    }

    //dbgpl(&dib0090_dbg, "FE %d TUNER STEP %d callback time = %d",fe->id, fe->tune_state,fe->tuner_info->callback_time);

    if (*tune_state == CT_TUNER_START) {

        if(state->tuner_is_tuned == 0)
            fe->current_rf = 0;

        if (fe->current_rf != rf) {
            // select internal switch
            dib0090_write_reg(state, CTRL_RF_SW, 0xb800 | (tune->switch_trim));

            /* external loop filter, otherwise:
             * lo5 = (0 << 15) | (0 << 12) | (0 << 11) | (3 << 9) | (4 << 6) | (3 << 4) | 4;
             * lo6 = 0x0e34 */
            if (pll->vco_band)
                lo5 = 0x049e;
            else if (state->config->analog_output)
                lo5 = 0x041d;
            else
                lo5 = 0x041c;

            lo5 |= (pll->hfdiv_code << 11) | (pll->vco_band << 7); /* bit 15 is the split to the slave, we do not do it here */

            //Internal loop filter set...
            if(!state->config->io.pll_int_loop_filt)
                lo6 = 0xff28;
            else
                lo6 = (state->config->io.pll_int_loop_filt << 3); // take the loop filter value given by the layout
            //dbgpl(&dib0090_dbg, "FE %d lo6 = 0x%04x", (uint32_t)fe->id, (uint32_t)lo6);

            // Find the VCO frequency in MHz
            VCOF_kHz = (pll->hfdiv * rf) * 2;

            FREF = state->config->io.clock_khz; // REFDIV is 1FREF Has to be as Close as possible to 10MHz

            // Determine the FB divider
            // The reference is 10MHz, Therefore the FBdivider is on the first digits
            FBDiv = (VCOF_kHz / pll->topresc / FREF);
            Rest  = (VCOF_kHz / pll->topresc) - FBDiv * FREF; //in kHz

            // Avoid Spurs in the loopfilter bandwidth
            if (Rest < LPF)                   Rest = 0;
            else if (Rest < 2 * LPF)          Rest = 2 * LPF;
            else if (Rest > (FREF - LPF))   { Rest = 0 ; FBDiv += 1; } //Go to the next FB
            else if (Rest > (FREF - 2 * LPF)) Rest = FREF - 2 * LPF;
            Rest = (Rest * 6528) / (FREF / 10);

            Den = 1;

            dbgpl(&dib0090_dbg, " *****  ******* Rest value = %d", Rest ) ; //state->wbd_offset);

            if (Rest > 0) {
                if (state->config->analog_output)
                    lo6 |= (1 << 2) | 2; //SigmaDelta and Dither
                else
                    lo6 |= (1 << 2) | 1; //SigmaDelta and Dither
                Den = 255;
            }

#ifdef CONFIG_BAND_SBAND
            if (state->current_band == BAND_SBAND)
                lo6 &= 0xfffb; //Remove the Dither for SBAND
#endif

            // Now we have to define the Num and Denum
            // LO1 gets the FBdiv
            dib0090_write_reg(state, CTRL_LO_1, (uint16_t) FBDiv);
            // LO2 gets the REFDiv
            dib0090_write_reg(state, CTRL_LO_2, (Den << 8) | 1);
            // LO3 for the Numerator
            dib0090_write_reg(state, CTRL_LO_3, (uint16_t)Rest);
            // VCO and HF DIV
            dib0090_write_reg(state, CTRL_LO_5, lo5);
            // SIGMA Delta
            dib0090_write_reg(state, CTRL_LO_6, lo6);


            // Check if the 0090 is analogged configured
            //Disable ADC and DigPLL =0xFF9F, 0xffbf for test purposes.
            //Enable The Outputs of the BB on DATA_Tx
            lo6 = tune->tuner_enable;
            if (state->config->analog_output)
                lo6 = (lo6 & 0xff9f) | 0x2;

            dib0090_write_reg(state, TUNER_EN, lo6 | EN_LO
#ifdef CONFIG_DIB0090_USE_PWM_AGC
                    | state->config->use_pwm_agc * EN_CRYSTAL
#endif
                    );

            fe->current_rf = rf;

            /* prepare a complete captrim */
            state->step = state->captrim = state->fcaptrim = 64;

        } else { /* we are already tuned to this frequency - the configuration is correct  */

            /* do a minimal captrim even if the frequency has not changed */
            state->step = 4;
            state->captrim = state->fcaptrim = dib0090_read_reg(state, CTRL_LO_4) & 0x7f;
        }
        state->adc_diff = 3000; // start with a unreachable high number

        dib0090_write_reg(state, CTRL_WBDMUX, 0x2B1);

        dib0090_write_reg(state, ADCCLK, 0x0032);

        ret = 20;
        *tune_state = CT_TUNER_STEP_1;
    } else if (*tune_state == CT_TUNER_STEP_0) {
        /* nothing */
    } else if (*tune_state == CT_TUNER_STEP_1) {
        state->step /= 2;
        dib0090_write_reg(state, CTRL_LO_4, lo4 | state->captrim);
        *tune_state = CT_TUNER_STEP_2;
    } else if (*tune_state == CT_TUNER_STEP_2) {

        adc = dib0090_read_reg(state, ADCVAL);
        dbgpl(&dib0090_dbg, "FE %d CAPTRIM=%d; ADC = %d (ADC) & %dmV", (uint32_t)fe->id, (uint32_t)state->captrim, (uint32_t)adc, (uint32_t) (adc)*(uint32_t)1800/(uint32_t)1024);

        if (adc >= 400) {
            adc -= 400;
            step_sign = -1;
        } else {
            adc = 400 - adc;
            step_sign = 1;
        }

        if (adc < state->adc_diff) {
            dbgpl(&dib0090_dbg, "FE %d CAPTRIM=%d is closer to target (%d/%d)", (uint32_t)fe->id, (uint32_t)state->captrim, (uint32_t)adc, (uint32_t)state->adc_diff);
            state->adc_diff = adc;
            state->fcaptrim = state->captrim;
            //we could break here, to save time, if we reached a close-enough value
            //e.g.: if (state->adc_diff < 20)
            //break;
        }

        state->captrim += step_sign * state->step;
        if (state->step >= 1)
            *tune_state = CT_TUNER_STEP_1;
        else
            *tune_state = CT_TUNER_STEP_3;

        ret = 15;
    } else if (*tune_state == CT_TUNER_STEP_3) {
        /*write the final cptrim config*/
        dib0090_write_reg(state, CTRL_LO_4, lo4 | state->fcaptrim);

#ifdef CONFIG_TUNER_DIB0090_CAPTRIM_MEMORY
        state->memory[state->memory_index].cap=state->fcaptrim; //Store the cap value in the short memory
#endif

        *tune_state = CT_TUNER_STEP_4;
    } else if (*tune_state == CT_TUNER_STEP_4) {
        dib0090_write_reg(state, ADCCLK, 0x07ff);

        dbgpl(&dib0090_dbg, "FE %d Final Captrim: %d",(uint32_t)fe->id, (uint32_t)state->fcaptrim);
        dbgpl(&dib0090_dbg, "FE %d HFDIV code: %d",(uint32_t)fe->id, (uint32_t)pll->hfdiv_code);
        dbgpl(&dib0090_dbg, "FE %d VCO = %d",(uint32_t)fe->id, (uint32_t)pll->vco_band);
        dbgpl(&dib0090_dbg, "FE %d VCOF in kHz: %d ((%d*%d) << 1))",(uint32_t)fe->id, (uint32_t)((pll->hfdiv * rf) * 2), (uint32_t)pll->hfdiv, (uint32_t)rf);
        dbgpl(&dib0090_dbg, "FE %d REFDIV: %d, FREF: %d", (uint32_t)fe->id, (uint32_t)1, (uint32_t)state->config->io.clock_khz);
        dbgpl(&dib0090_dbg, "FE %d FBDIV: %d, Rest: %d",(uint32_t)fe->id, (uint32_t)dib0090_read_reg(state, CTRL_LO_1), (uint32_t)dib0090_read_reg(state, CTRL_LO_3));
        dbgpl(&dib0090_dbg, "FE %d Num: %d, Den: %d, SD: %d",(uint32_t)fe->id, (uint32_t)dib0090_read_reg(state, CTRL_LO_3), (uint32_t)(dib0090_read_reg(state, CTRL_LO_2) >> 8), (uint32_t)dib0090_read_reg(state, CTRL_LO_6) & 0x3);

#define WBD     0x781 /* 1 1 1 1 0000 0 0 1 */
        c=4; //wbdmux_gain
        i=3; //wbdmux_bias
#if defined(CONFIG_BAND_LBAND) || defined(CONFIG_BAND_SBAND)
        if ((state->current_band==BAND_LBAND) || (state->current_band==BAND_SBAND)) {
            c=2;
            i=2;
        }
#endif
        dib0090_write_reg(state, CTRL_WBDMUX, (c << 13) | (i << 11) | (WBD
#ifdef CONFIG_DIB0090_USE_PWM_AGC
                    |(state->config->use_pwm_agc<<1)
#endif
                    ));
        dib0090_write_reg(state, CTRL_RXRF2, (tune->lna_tune << 5) | (tune->lna_bias << 0));
        dib0090_write_reg(state, CTRL_RF_V2I, tune->v2i);
        dib0090_write_reg(state, CTRL_RF_MIX, tune->mix);
        dib0090_write_reg(state, CTRL_RF_LOAD, tune->load);

        *tune_state = CT_TUNER_STEP_5;
    } else if (*tune_state == CT_TUNER_STEP_5) {

        /* initialize the lt gain register */
        state->rf_lt_def = 0x7c00;
        dib0090_write_reg(state, CTRL_RF_LT, state->rf_lt_def);

        dib0090_set_bandwidth(state, ch);
        state->tuner_is_tuned = 1;
        *tune_state = CT_TUNER_STOP;
    } else
        ret = FE_CALLBACK_TIME_NEVER;
    return ret;
}

static int dib0090_release(struct dibFrontend *tuner)
{
    struct dib0090_state *st = tuner->tuner_priv;
    MemFree(st,sizeof(struct dib0090_state));
    return 0;
}

uint16_t dib0090_get_wbd_offset(struct dibFrontend *tuner)
{
    struct dib0090_state *st = tuner->tuner_priv;
    return st->wbd_offset;
}

#define pgm_read_word(w) (*w)
static const uint16_t dib0090_defaults[] =
{
    // RF INITIALISATION + BB
    25, CTRL_BB_1, // nb of values, start register
        0x0000, // 0x01  CTRL_BB_1
        0x99a0,   // 0x02  CTRL_BB_2 Captrim
        0x6008, // 0x03  CTRL_BB_3
        0x0000, // 0x04  CTRL_BB_4 DC servo loop 1KHz
        0x8acb, // 0x05  CTRL_BB_5
        0x0000, // 0x06  CTRL_BB_6
        0x0405, // 0x07  CTRL_BB_7
        0x0000, // 0x08  CTRL_RF_1
        0x0000, // 0x09  CTRL_RF_2 Bias and Tune not set
        0x0000, // 0x0a	 CTRL_RF_3
        0xb802, // 0x0b	 CTRL_RF_SW
        0x0300, // 0x0c  CTRL_RF_V2I
        0x2d12, // 0x0d	 CTRL_RF_MIX
        0xbac0, // 0x0e  CTRL_LOAD
        0x7c00, // 0x0f	 CTRL_LT //CAB
        0xdbb9, // 0x10  CTRL_WBDMUX   // WBD MUX TOWARD BB
        0x0954, // 0x11  CTRL_TX
        0x0743,	// 0x12  IQ_ADC
        0x8000,	// 0x13	 CTRL_BIAS 0x4106 Internal bias
        0x0001, // 0x14  CTRL_CRYSTAL
        0x0040, // 0x15  CTRL_LO_1
        0x0100, // 0x16  CTRL_LO_2
        0x0000, // 0x17  CTRL_LO_3
        0xe910, // 0x18  CTRL_LO_4
        0x149e, // 0x19  CTRL_LO_5 1734

    1, CTRL_LO_6,
        0xff2d,	// 0x1a  CTRL_LO_6

    1, BB_RAMP7,
        0x0000,  // deactivate the ramp and ramp_mux to adjust the gain manually

    1, SLEEP_EN,
		EN_IQADC | EN_BB | EN_BIAS | EN_DIGCLK | EN_PLL | EN_CRYSTAL, //0x1b SLEEP_EN
    2, ADCCLK,
        0x07FF,	// ADC_CLK
        0x0007, // VGA_MODE

    1, TUNER_EN,
        EN_UHF | EN_CRYSTAL, // 0x33 TUNER_EN

    2, PWM1_REG,
        0x3ff, // pwm accu period
        0x111, // (alpha_bb<<8)|(resetn_pwm<<4)|(alpha_rf)
    0
};



extern uint8_t dib0090_identify(struct dibFrontend *fe);
extern void dib0090_reset_digital(struct dibFrontend *fe, const struct dib0090_config *cfg);

#ifdef EFUSE
static void dib0090_set_EFUSE(struct dib0090_state *state)
{
    uint8_t c,h,n;
    uint16_t e2,e4;

    e2=dib0090_read_reg(state,EFUSE_2);
    e4=dib0090_read_reg(state,EFUSE_4);
    //e2 = e4 = (5<<12) | (27<<6) | (0x18); /* Just for test */
    e2 &= e4; /* Remove the redundancy  */
    if (e2 != 0xffff) {
        c = e2 & 0x3f;
        n = (e2 >> 12) & 0xf;
        h= (e2 >> 6) & 0x3f;
        dib0090_write_reg(state,CTRL_BIAS, (h << 10)) ;
        e2 = (n<<11) | ((h>>2)<<6) | (c);
        dib0090_write_reg(state,CTRL_BB_2, e2) ; /* Load the BB_2 */
    }
}
#endif

static int dib0090_reset(struct dibFrontend *fe)
{
    struct dib0090_state *state = fe->tuner_priv;
    uint16_t l, r, *n;

    dib0090_reset_digital(fe, state->config);
    state->revision = dib0090_identify(fe);

    /* Revision definition */
    if (state->revision == 0xff)
        return DIB_RETURN_ERROR;
#ifdef EFUSE
    else if ((state->revision&0x1f) >= 3) /* Update the efuse : Only available for KROSUS > P1C */
        dib0090_set_EFUSE(state);
#endif

#ifdef CONFIG_TUNER_DIB0090_P1B_SUPPORT
    if (!(state->revision & 0x1)) /* it is P1B - reset is already done */
        return DIB_RETURN_SUCCESS;
#endif

    /* Upload the default values */
    n = (uint16_t *) dib0090_defaults;
    l = pgm_read_word(n++);
    while (l) {
        r = pgm_read_word(n++);
        do {
          /* DEBUG_TUNER */
          /* dbgpl(&dib0090_dbg,"%d, %d, %d", l, r, pgm_read_word(n)); */
            dib0090_write_reg(state, r, pgm_read_word(n++));
            r++;
        } while (--l);
	    l = pgm_read_word(n++);
    }

    /* Congigure in function of the crystal */
    if (state->config->io.clock_khz >= 24000)
        l = 1;
    else
        l = 2;
    dib0090_write_reg(state, CTRL_CRYSTAL, l);
    dbgpl(&dib0090_dbg, "Pll lock : %d", (dib0090_read_reg(state, DIG_CFG_RO)>>11)&0x1);

    state->reset = 3; /* enable iq-offset-calibration and wbd-calibration when tuning next time */

    return DIB_RETURN_SUCCESS;
}

#if !defined(FIRMWARE_FIREFLY) && !defined(CONFIG_BUILD_LEON)
static int dib0090_generic_monitoring(struct dibFrontend *fe, uint32_t cmd, uint8_t *b, uint32_t size, uint32_t offset)
{
    struct dib0090_state *state = fe->tuner_priv;
    int ret = DIB_RETURN_NOT_SUPPORTED;

    switch (cmd & ~GENERIC_MONIT_REQUEST_SIZE) {
    case GENERIC_MONIT_AGC_FREEZE:
        if(!state->config->use_pwm_agc) {
            ret = 0;
            state->agc_freeze = offset;
        }
        break;
    default:
        break;
    }

    return ret;
}
#endif

static const struct dibTunerInfo dib0090_info = {
    INFO_TEXT("DiBcom DiB0090 (Kroesus)"),   // name

    TUNER_CAN_VHF | TUNER_CAN_UHF | TUNER_CAN_LBAND | TUNER_CAN_FAST_TUNE, // caps

    { // ops
        dib0090_reset,   // reset
        dib0090_wakeup,  // wakeup
        dib0090_sleep,   // sleep

        dib0090_tune,  // tune_digital

        NULL,
#if !defined(FIRMWARE_FIREFLY) && !defined(CONFIG_BUILD_LEON)
        dib0090_generic_monitoring,
#else
        NULL,
#endif

        dib0090_release, // release
    }
};

struct dibFrontend * dib0090_register(struct dibFrontend *fe, struct dibDataBusHost *data, const struct dib0090_config *config)
{
	struct dib0090_state *st = MemAlloc(sizeof(struct dib0090_state));
	struct dibDataBusClient *client;
	if (st == NULL)
		return NULL;
	DibZeroMemory(st,sizeof(struct dib0090_state));

#ifdef CONFIG_DIB0090_USE_PWM_AGC
        if(!config->use_pwm_agc) {
#endif
    if (fe->demod_info == NULL)
        dbgpl(&dib0090_dbg, "this driver should have a demodulator in the same frontend - not registering the GAIN control functions");
    else
        fe->demod_info->ops.agc_startup = dib0090_gain_control;
#ifdef CONFIG_DIB0090_USE_PWM_AGC
        }
#endif
    frontend_register_tuner(fe, &st->info, &dib0090_info, st);

	client = tuner_get_data_bus_client(fe);

    switch(data_bus_host_type(data)) {
#ifndef CONFIG_BUILD_LEON
        case DATA_BUS_I2C:
            data_bus_client_init(client, &data_bus_client_template_i2c_8b_16b, data);
            data_bus_client_set_speed(client, 340);
            data_bus_client_set_device_id(client, config->i2c_address);
            break;
#endif
        case DATA_BUS_DIBCTRL_IF:
            data_bus_client_init(client, &data_bus_client_template_8b_16b, data);
            break;
        default:
            dbgpl(&dib0090_dbg, "data_bus host type not handled");
    }

    st->config = config;
    st->fe = fe;
    st->tune = dib0090_tune;

    return fe;
}

/* enable/disable the VCO splitting - fe is the MASTER */
static void dib0090_twin_share_vco(struct dibFrontend *fe, uint8_t onoff)
{
    struct dib0090_state *state = fe->tuner_priv;
    uint16_t val;

    dbgpl(&dib0090_dbg, "TWIN tuner: sharing VCO %s", onoff ? "on" : "off");

    /* MASTER */
    val = dib0090_read_reg(state, CTRL_LO_5) & 0x7fff;
    dib0090_write_reg(state, CTRL_LO_5, val | (onoff << 15));

    /* SLAVE */
    state = state->twin->tuner_priv;
    val = dib0090_read_reg(state, TUNER_EN) & ~(EN_LO);
    if (!onoff)
        val |= EN_LO;
    dib0090_write_reg(state, TUNER_EN, val);
}

static int dib0090_twin_tune_master(struct dibFrontend *fe, struct dibChannel *ch)
{
    struct dib0090_state *state = fe->tuner_priv;
    int ret, offset = 0;

    /* MASTER is tuned to the same frequency - now that is going to change the frequency */
    if (fe->tune_state == CT_TUNER_START)
        if (state->twin->current_rf != ch->RF_kHz)
            dib0090_twin_share_vco(fe, 0);

    /* work-around for P1C */
    if (state->revision & KROSUS_P1C &&
        channel_frequency_band(ch->RF_kHz) == BAND_UHF && ch->type != STANDARD_DAB)
        offset = 300;

    ch->RF_kHz += offset;

    ret = state->tune(fe, ch); /* tune the master normally */

    ch->RF_kHz -= offset;

    /* MASTER is now tuned */
    if (fe->tune_state == CT_TUNER_STOP)
        /* the twin (the SLAVE) is already tuned to the same frequency */
        if (state->twin->tune_state >= CT_TUNER_STOP && state->twin->current_rf == ch->RF_kHz)
            dib0090_twin_share_vco(fe, 1);

    return ret;
}

static int dib0090_twin_tune_slave(struct dibFrontend *fe, struct dibChannel *ch)
{
    struct dib0090_state *state = fe->tuner_priv;
    int ret, offset = 0;

    /* work-around for P1C */
    if (state->revision & KROSUS_P1C &&
            channel_frequency_band(ch->RF_kHz) == BAND_UHF && ch->type != STANDARD_DAB)
        offset = -300;

    ch->RF_kHz += offset;

    ret = state->tune(fe, ch); /* tune the slave normaly */

    ch->RF_kHz -= offset;

    if (fe->tune_state == CT_TUNER_STOP) { /* SLAVE is now tuned */
        /* the twin (the MASTER) is already tuned to the same frequency */
        if (state->twin->tune_state >= CT_TUNER_STOP && state->twin->current_rf == ch->RF_kHz)
            dib0090_twin_share_vco(state->twin, 1);
        else
            dib0090_twin_share_vco(state->twin, 0);
    }
    return ret;
}

static int dib0090_twin_sleep_master(struct dibFrontend *fe)
{
    struct dib0090_state *state = fe->tuner_priv;
    if (state->twin->tune_state >= CT_TUNER_STOP) /* slave is tuned */
        dib0090_twin_share_vco(fe, 0);
    return dib0090_sleep(fe);
}

static int dib0090_twin_sleep_slave(struct dibFrontend *fe)
{
    struct dib0090_state *state = fe->tuner_priv;
    dib0090_twin_share_vco(state->twin, 0);
    return dib0090_sleep(fe);
}

void dib0090_twin_setup(struct dibFrontend *master, struct dibFrontend *slave)
{
    struct dib0090_state *state = master->tuner_priv;
    state->twin = slave;
    master->tuner_info->ops.tune_digital = dib0090_twin_tune_master;
    master->tuner_info->ops.sleep = dib0090_twin_sleep_master;

    state = slave->tuner_priv;
    state->twin = master;
    slave->tuner_info->ops.tune_digital = dib0090_twin_tune_slave;
    slave->tuner_info->ops.sleep = dib0090_twin_sleep_slave;
    dbgpl(&dib0090_dbg, "master %p slave %p", master, slave);
}
