#ifndef DIB0090_PRIV_H
#define DIB0090_PRIV_H

/* Krosus Registers */

#define KROSUS_P1C    0x001

#define CTRL_BB_1 1
#define CTRL_BB_2 2
#define CTRL_BB_3 3
#define CTRL_BB_4 4
#define CTRL_BB_5 5
#define CTRL_BB_6 6
#define CTRL_BB_7 7
#define CTRL_RXRF1 8
#define CTRL_RXRF2 9
#define CTRL_RXRF3 10
#define CTRL_RF_SW 11
#define CTRL_RF_V2I 12
#define CTRL_RF_MIX  13
#define CTRL_RF_LOAD 14
#define CTRL_RF_LT 15
#define CTRL_WBDMUX 16
#define CTRL_TX 17
#define IQ_ADC_CTRL 18
#define CTRL_BIAS 19
#define CTRL_CRYSTAL 20
#define CTRL_LO_1 21
#define CTRL_LO_2 22
#define CTRL_LO_3 23
#define CTRL_LO_4 24
#define CTRL_LO_5 25
#define DIG_CFG_RO 26
#define SLEEP_EN 27
#define CTRL_LO_6 28
#define ADCVAL 29
#define ADCCLK 30
#define VGA_MODE 31
#define DIG_CFG_3 32
#define PLL_CFG 33
#define CALIBRATE 34
#define DIG_CFG 35
#define TUNER_EN 36
#define EFUSE_1 37
#define EFUSE_2 38
#define EFUSE_3 39
#define EFUSE_4 40
#define EFUSE_CTRL 41
#define RF_RAMP1 42
#define RF_RAMP2 43
#define RF_RAMP3 44
#define RF_RAMP4 45
#define RF_RAMP5 46
#define RF_RAMP6 47
#define RF_RAMP7 48
#define RF_RAMP8 49
#define RF_RAMP9 50
#define BB_RAMP1 51
#define BB_RAMP2 52
#define BB_RAMP3 53
#define BB_RAMP4 54
#define BB_RAMP5 55
#define BB_RAMP6 56
#define BB_RAMP7 57
#define CURRGAINBB 58
#define CURRGAINRF 59
#define PWM1_REG 60
#define PWM2_REG 61
#define GAIN4_1 62
#define GAIN4_2 63

#define EN_LNA0      0x8000
#define EN_LNA1      0x4000
#define EN_LNA2      0x2000
#define EN_LNA3      0x1000
#define EN_MIX0      0x0800
#define EN_MIX1      0x0400
#define EN_MIX2      0x0200
#define EN_MIX3      0x0100
#define EN_IQADC     0x0040
#define EN_PLL       0x0020
#define EN_TX        0x0010
#define EN_BB        0x0008
#define EN_LO        0x0004
#define EN_BIAS      0x0001

#define EN_IQANA     0x0002
#define EN_DIGCLK    0x0080 /* not in the TUNER_EN reg, only in SLEEP_EN */
#define EN_CRYSTAL   0x0002

#define EN_UHF		 0x22E9
#define EN_VHF		 0x44E9
#define EN_LBD		 0x11E9
#define EN_SBD		 0x44E9
#define EN_CAB		 0x88E9

// Define the Mixer settings
#define MIX_S		0x485a
#define V2I_S		0x0240
#define LOAD_S		0x87d0

struct dc_calibration;

struct dib0090_state {
    struct dibTunerInfo info;
    struct dibFrontend *fe;
    struct dibFrontend *twin;

    const struct dib0090_config *config;

    uint16_t revision;

    uint16_t wbd_offset;
    int16_t wbd_target; /* in dB */

    int16_t rf_gain_limit;  /* take-over-point: where to split between bb and rf gain */
    int16_t current_gain; /* keeps the currently programmed gain */
    uint8_t agc_step;     /* new binary search */

    uint16_t gain[2]; /* for channel monitoring */

    const uint16_t *rf_ramp;
    const uint16_t *bb_ramp;

    /* for the software AGC ramps */
    uint16_t bb_1_def;
    uint16_t rf_lt_def;
    uint16_t gain_reg[4];


#ifdef CONFIG_TUNER_DIB0090_CAPTRIM_MEMORY
#define MEM_DEPTH 8
    struct {
        uint32_t freq;
        uint8_t cap;
    } memory[MEM_DEPTH];
	uint8_t memory_index;
#endif

    /* for the captrim/dc-offset search */
    int8_t step;
    int16_t adc_diff;
    int16_t min_adc_diff;

    int8_t captrim;
    int8_t fcaptrim;

    const struct dc_calibration *dc;
    uint16_t bb6, bb7;

    const struct dib0090_tuning *current_tune_table_index;
    const struct dib0090_pll *current_pll_table_index;

    int (*tune)(struct dibFrontend *, struct dibChannel *);

#ifdef CONFIG_TUNER_DIB0090_P1B_SUPPORT
    uint16_t bsearch_step;
    uint8_t rf_ramp_slope;
    int16_t max_rfgain; /* in 10th dB - depending on the tuned band */
    int16_t max_bbgain;
#endif

    uint8_t tuner_is_tuned;
    uint8_t agc_freeze;
    uint8_t current_band;

    uint8_t reset;
};

extern int dib0090_p1b_register(struct dibFrontend *fe);

extern const struct dibDebugObject dib0090_dbg;

#endif
