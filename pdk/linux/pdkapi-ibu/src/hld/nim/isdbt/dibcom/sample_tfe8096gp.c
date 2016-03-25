#include "sample.h"
#include "interface/host.h"

#include <adapter/frontend_tune.h>
#include <adapter/sip.h>
#include <sip/dib8090.h>
#include <demod/dib8000.h>
#include "monitor/monitor.h"
#include <tuner/dib0090.h>

static const struct dib8090_config tfe8096gp_config = {
    1,
    NULL, // update_lna

    DIB8090_GPIO_DEFAULT_DIRECTIONS,
    DIB8090_GPIO_DEFAULT_VALUES,
    DIB8090_GPIO_DEFAULT_PWM_POS,

    0,     // dib0070_freq_offset_khz_uhf
    -143,     // dib0070_freq_offset_khz_vhf

    0,
    12000, // clock_khz
    4,
    0,
    0,
    0,
    0,

    0x2d98, // dib8k_drives
    144,   //diversity_delay
    0x31,  //div_cfg
    1, //clkouttobamse
    1, // clkoutdrive
};

static int (*tfe8096gp_agc_tune_save)(struct dibFrontend *fe, struct dibChannel *channel);

static int tfe8096gp_agc_tune (struct dibFrontend *fe , struct dibChannel *channel)
{
    int return_value;

    return_value = tfe8096gp_agc_tune_save(fe, channel);
    if (fe->tune_state == CT_AGC_STEP_0)
        demod_set_gpio(fe, 6, 0, 1);
    else if (fe->tune_state == CT_AGC_STEP_1) {
        /* check the high level */
        uint16_t ltgain, rf_gain_limit;
        dib0090_get_current_gain(fe, NULL, NULL, &rf_gain_limit, &ltgain);
        if ((channel_frequency_band(channel->RF_kHz) == BAND_CBAND) &&
                (rf_gain_limit < 2000)) /* activate the external attenuator in case of very high input power */
            demod_set_gpio(fe, 6, 0, 0);
    }
    return return_value;
}

static int (*tfe8096gp_tuner_tune_save) (struct dibFrontend *fe , struct dibChannel *channel);
static int tfe8096gp_tuner_tune (struct dibFrontend *fe , struct dibChannel *channel)
{
    int return_value;

    if (fe->tune_state == CT_TUNER_START) {
        switch(BAND_OF_FREQUENCY(channel->RF_kHz)) {
            default:
                dbgpl(NULL,"Warning : this frequency is not in the supported range, using VHF switch");
            case BAND_VHF: //gpio5 : 0; gpio7 : 0
                demod_set_gpio(fe, 9, 0, 0);
                demod_set_gpio(fe, 7, 0, 0);
                break;
            case BAND_UHF: //gpio5 : 0; gpio7 : 1
                demod_set_gpio(fe, 9, 0, 0);
                demod_set_gpio(fe, 7, 0, 1);
                break;
            case BAND_CBAND: //gpio5 : 1; gpio7 : 0
                demod_set_gpio(fe, 9, 0, 1);
                demod_set_gpio(fe, 7, 0, 0);
                break;
        }
    }
    return_value = tfe8096gp_tuner_tune_save(fe, channel);

    return return_value;
}

int main (void)
{
    // default I2C implementation is based on parallel port but user can connect its
    // own I2C driver using host_i2c_interface_attach();
    // implementation is done in sample/interface/host.c
    //struct dibDataBusHost *i2c = host_i2c_interface_attach(NULL);
	struct dibDataBusHost *i2c = open_spp_i2c();
	struct dibFrontend fe;
	struct dibChannel ch;
	struct dibDemodMonitor mon;

	DibZeroMemory(&mon, sizeof(mon));

    if (i2c == NULL)
        return 1;

    frontend_init(&fe); /* initializing the frontend-structure */
    frontend_set_id(&fe, 0); /* assign an absolute ID to the frontend */
    frontend_set_description(&fe, "ISDB-T #0 Master");

    if ( dib8090_sip_register(&fe, i2c, 0x90, &tfe8096gp_config) == NULL)
        return DIB_RETURN_ERROR;

    tfe8096gp_tuner_tune_save = fe.tuner_info->ops.tune_digital;
    fe.tuner_info->ops.tune_digital = tfe8096gp_tuner_tune;
    tfe8096gp_agc_tune_save = fe.demod_info->ops.agc_startup;
    fe.demod_info->ops.agc_startup = tfe8096gp_agc_tune;

	dib8090_set_wbd_target(&fe, 250, 425);

    dib8000_i2c_enumeration(i2c, 1, DIB8090_DEFAULT_I2C_ADDRESS, 0x90); /* do the i2c-enumeration for 4 demod and use 0x90 as the I2C address for first device */

    frontend_reset(&fe);

    INIT_CHANNEL(&ch, STANDARD_ISDBT);
    ch.RF_kHz = 767143;
    ch.bandwidth_kHz = 6000;

    tune_diversity(&fe, 1, &ch);

    DibDbgPrint("-I-  Tuning done <enter>\n");
    getchar();

    while (1) {
        /* after enumerating on the same i2c-bus, the i2c-addresses of the bus will be 0x80 for the diversity master and 0x82 for the slave */
        demod_get_monitoring(&fe, &mon);
        dib7000_print_monitor(&mon, NULL, 0 ,1);
        usleep(100000);
    }

    DibDbgPrint("-I-  Cleaning up\n");
    frontend_unregister_components(&fe);

    //host_i2c_release(i2c);
	close_spp_i2c();

	return 0;
}
