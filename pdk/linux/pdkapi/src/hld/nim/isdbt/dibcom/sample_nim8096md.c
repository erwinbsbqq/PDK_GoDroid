#include "sample.h"
#include "interface/host.h"

#include <adapter/frontend_tune.h>
#include <adapter/sip.h>
#include <sip/dib8090.h>
#include <demod/dib8000.h>
#include "monitor/monitor.h"
#include <tuner/dib0090.h>

static const struct dib8090_config nim8096md_config[2] = {
    {
        1,
        NULL, // update_lna

        DIB8090_GPIO_DEFAULT_DIRECTIONS,
        DIB8090_GPIO_DEFAULT_VALUES,
        DIB8090_GPIO_DEFAULT_PWM_POS,

        -63,     // dib0070_freq_offset_khz_uhf
        -143,     // dib0070_freq_offset_khz_vhf

        0,
        12000, // clock_khz
        4,
        0,
        0,
        0,
        0,

        0x2d98, // dib8k_drives
        48,   //diversity_delay
        0x31,  //div_cfg
        1, //clkouttobamse
        1, // clkoutdrive
        3, // refclksel
    },
    {
        1,
        NULL, // update_lna

        DIB8090_GPIO_DEFAULT_DIRECTIONS,
        DIB8090_GPIO_DEFAULT_VALUES,
        DIB8090_GPIO_DEFAULT_PWM_POS,

        -63,     // dib0070_freq_offset_khz_uhf
        -143,     // dib0070_freq_offset_khz_vhf

        0,
        12000, // clock_khz
        4,
        0,
        0,
        0,
        0,

        0x2d08, // dib8k_drives
        1,   //diversity_delay
        0x31,  //div_cfg
        1, //clkouttobamse
        1, // clkoutdrive
        3, // refclksel
    },
};

int (*nim8096md_tuner_tune) (struct dibFrontend *fe, struct dibChannel *channel);
static int dib8090_tuner_tune(struct dibFrontend *fe , struct dibChannel *channel)
{
    if (fe->tune_state == CT_TUNER_START) {
        switch(BAND_OF_FREQUENCY(channel->RF_kHz)) {
        default:
            dbgpl(NULL,"Warning : this frequency is not in the supported range, using VHF switch");
        case BAND_VHF: //gpio5 : 0; gpio7 : 0
            demod_set_gpio(fe, 3, 0, 1);
            break;
        case BAND_UHF: //gpio5 : 0; gpio7 : 1
            demod_set_gpio(fe, 3, 0, 0);
            break;
        }
    }

    return nim8096md_tuner_tune(fe, channel);
}


int main (void)
{
    // default I2C implementation is based on parallel port but user can connect its
    // own I2C driver using host_i2c_interface_attach();
    // implementation is done in sample/interface/host.c
    //struct dibDataBusHost *i2c = host_i2c_interface_attach(NULL);
	struct dibDataBusHost *i2c = open_spp_i2c();
	struct dibFrontend fe[2];
	struct dibChannel channel;
	struct dibDemodMonitor mon[2];

	DibZeroMemory(&mon, sizeof(mon));

	if (i2c == NULL)
		return 1;

	frontend_init(&fe[0]);      /* initializing the frontend-structure */
	frontend_set_id(&fe[0], 0); /* assign an absolute ID to the frontend */
	frontend_set_description(&fe[0], "ISDB-T #0 Master");
	frontend_init(&fe[1]);      /* initializing the frontend-structure */
	frontend_set_id(&fe[1], 1); /* assign an absolute ID to the frontend */
	frontend_set_description(&fe[1], "ISDB-T #1 Slave");

	if ( dib8090_sip_register(&fe[0], i2c, 0x90, &nim8096md_config[0]) == NULL)
        return DIB_RETURN_ERROR;
	if ( dib8090_sip_register(&fe[1], i2c, 0x92, &nim8096md_config[1]) == NULL)
        return DIB_RETURN_ERROR;

    dib8000_i2c_enumeration(i2c, 2, DIB8090_DEFAULT_I2C_ADDRESS, 0x90); /* do the i2c-enumeration for 2 demod and use 0x90 as the I2C address for first device */
    nim8096md_tuner_tune = fe[0].tuner_info->ops.tune_digital;
    fe[0].tuner_info->ops.tune_digital = dib8090_tuner_tune;
    fe[1].tuner_info->ops.tune_digital = dib8090_tuner_tune;

    dib8090_set_wbd_target(&fe[0], 250, 425);
    dib8090_set_wbd_target(&fe[1], 250, 425);

    frontend_reset(&fe[0]);
    frontend_reset(&fe[1]);

    INIT_CHANNEL(&channel, STANDARD_ISDBT);
    channel.RF_kHz = 665143;
    channel.bandwidth_kHz = 6000;

    tune_diversity(fe, 2, &channel);

    DibDbgPrint("-I-  Tuning done <enter>\n");
    getchar();

    while (1) {
        /* after enumerating on the same i2c-bus, the i2c-addresses of the bus will be 0x80 for the diversity master and 0x82 for the slave */
        demod_get_monitoring(&fe[0], &mon[0]);
        demod_get_monitoring(&fe[1], &mon[1]);
        dib7000_print_monitor(mon, NULL, 0 ,2);
        usleep(100000);
    }

    DibDbgPrint("-I-  Cleaning up\n");
    frontend_unregister_components(&fe[1]);
    frontend_unregister_components(&fe[0]);

    //host_i2c_release(i2c);
	close_spp_i2c();

	return 0;
}
