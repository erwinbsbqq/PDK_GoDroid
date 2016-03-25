#include <adapter/frontend.h>
#include <tuner/dib0090.h>

#include "dib0090_priv.h"

struct dib0090_fw_state
{
    const struct dib0090_config *cfg;
    struct dibTunerInfo info;
};

uint16_t dib0090_identify(struct dibFrontend *fe)
{
    struct dibDataBusClient *client = tuner_get_data_bus_client(fe);
    uint16_t v;

    v = data_bus_client_read16(client, DIG_CFG_RO);

#ifdef FIRMWARE_FIREFLY
    /* pll is not locked locked */
    if (!(v & 0x800))
        dbgpl(&dib0090_dbg, "FE%d : Identification : pll is not yet locked", fe->id);
#endif

    /* without PLL lock info */
    v &= 0x3ff;
    dbgpl(&dib0090_dbg, "P/V: %04x:", v);

    if((v >> 8)&0xf)
        dbgpl(&dib0090_dbg, "FE%d : Product ID = 0x%x : KROSUS", fe->id, (v >> 8)&0xf);
    else
        return 0xff;

    v&=0xff;
    if(((v>>5)&0x7) == 0x1)
        dbgpl(&dib0090_dbg, "FE%d : MP001 : 9090/8096", fe->id);
    else if(((v>>5)&0x7) == 0x4)
        dbgpl(&dib0090_dbg, "FE%d : MP005 : Single Sband", fe->id);
    else if(((v>>5)&0x7) == 0x6)
        dbgpl(&dib0090_dbg, "FE%d : MP008 : diversity VHF-UHF-LBAND", fe->id);
    else if(((v>>5)&0x7) == 0x7)
        dbgpl(&dib0090_dbg, "FE%d : MP009 : diversity 29098 CBAND-UHF-LBAND-SBAND", fe->id);
    else
        return 0xff;

    /* revision only */
    if ((v&0x1f) == 0x3)
        dbgpl(&dib0090_dbg, "FE%d : P1-D/E/F detected", fe->id);
    else if ((v&0x1f) == 0x1)
        dbgpl(&dib0090_dbg, "FE%d : P1C detected", fe->id);
    else if ((v&0x1f) == 0x0) {
#ifdef CONFIG_TUNER_DIB0090_P1B_SUPPORT
        dbgpl(&dib0090_dbg, "FE%d : P1-A/B detected: using previous driver - support will be removed soon", fe->id);
        dib0090_p1b_register(fe);
#else
        dbgpl(&dib0090_dbg, "FE%d : P1-A/B detected: driver is deactivated - not available", fe->id);
        return 0xff;
#endif
    }

    return v;
}

#define HARD_RESET(state) do {  if (cfg->reset) {  if (cfg->sleep) cfg->sleep(fe, 0); DibMSleep(10);  cfg->reset(fe, 1); DibMSleep(10);  cfg->reset(fe, 0); DibMSleep(10);  }  } while (0)

void dib0090_reset_digital(struct dibFrontend *fe, const struct dib0090_config *cfg)
{
    struct dibDataBusClient *client = tuner_get_data_bus_client(fe);

    HARD_RESET(state);

    data_bus_client_write16(client, TUNER_EN, EN_PLL);
    data_bus_client_write16(client, SLEEP_EN, EN_DIGCLK | EN_PLL | EN_CRYSTAL); /* PLL, DIG_CLK and CRYSTAL remain */

    /* adcClkOutRatio=8->7, release reset */
    data_bus_client_write16(client, DIG_CFG_3, ((cfg->io.adc_clock_ratio-1) << 11) | (0 << 10) | (1 << 9) | (1 << 8) | (0 << 4) | 0);
	if (cfg->clkoutdrive != 0)
		data_bus_client_write16(client, DIG_CFG, (0 << 15) | ((!cfg->analog_output) << 14) | (1 << 10) | (1 << 9) | (0 << 8) | (cfg->clkoutdrive << 5) | (cfg->clkouttobamse << 4) | (0 << 2) | (0));
	else
		data_bus_client_write16(client, DIG_CFG, (0 << 15) | ((!cfg->analog_output) << 14) | (1 << 10) | (1 << 9) | (0 << 8) | (7 << 5) | (cfg->clkouttobamse << 4) | (0 << 2) | (0));


    /* enable pll, de-activate reset, ratio: 2/1 = 60MHz */
    data_bus_client_write16(client, PLL_CFG, (cfg->io.pll_bypass << 15) | (1 << 13) | (cfg->io.pll_range << 12) | (cfg->io.pll_loopdiv << 6) | (cfg->io.pll_prediv));

    //data_bus_client_write16(client, PLL_CFG,   (1 << 13) | (1 << 12) | (8 << 6) | (1));
    //data_bus_client_write16(client, VGA_MODE, (0 << 4) | (0 << 3) | (1 << 2) | (0 << 0));
}

#ifdef CONFIG_BUILD_HOST
static int dib0090_fw_reset(struct dibFrontend *fe)
{
   struct dib0090_fw_state *state = fe->tuner_priv;
   dib0090_reset_digital(fe, state->cfg);
   return dib0090_identify(fe) == 0 ? DIB_RETURN_ERROR : DIB_RETURN_SUCCESS;
}

static int dib0090_fw_release(struct dibFrontend *tuner)
{
    struct dib0090_fw_state *st = tuner->tuner_priv;
    MemFree(st,sizeof(struct dib0090_fw_state));
    return DIB_RETURN_SUCCESS;
}

static const struct dibTunerInfo dib0090_fw_info = {
	INFO_TEXT("DiBcom DiB0090 (Krosus) Firmware"),   // name
	TUNER_CAN_VHF | TUNER_CAN_UHF | TUNER_CAN_LBAND | TUNER_CAN_SBAND, // caps

	{
        dib0090_fw_reset,   // reset
		NULL, // wakeup
		NULL, // sleep

		NULL, // tune_digital

		NULL,
		NULL,

        dib0090_fw_release, // release
	}
};

struct dibFrontend * dib0090_reset_register(struct dibFrontend *fe, struct dibDataBusHost *data, const struct dib0090_config *cfg)
{
	struct dib0090_fw_state *st = MemAlloc(sizeof(struct dib0090_fw_state));
	if (st == NULL)
		return NULL;
	DibZeroMemory(st,sizeof(struct dib0090_fw_state));

    frontend_register_tuner(fe, &st->info, &dib0090_fw_info, st);

    data_bus_client_init(tuner_get_data_bus_client(fe), &data_bus_client_template_8b_16b, data);

    st->cfg = cfg;

    return fe;
}
#endif
