#include "sample.h"
#include "interface/host.h"

#include <adapter/frontend_tune.h>
#include <adapter/sip.h>
#include <sip/dib8090.h>
#include <demod/dib8000.h>
#include "monitor/monitor.h"

static const struct dib8090_config tfe8096md4_config[4] = {
    {
        1,
        NULL, // update_lna

        DIB8090_GPIO_DEFAULT_DIRECTIONS,
        DIB8090_GPIO_DEFAULT_VALUES,
        DIB8090_GPIO_DEFAULT_PWM_POS,

        0,     // dib0090_freq_offset_khz_uhf
        -143,  // dib0090_freq_offset_khz_vhf

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
        3, // refclksel
    },
    {
        1,
        NULL, // update_lna

        DIB8090_GPIO_DEFAULT_DIRECTIONS,
        DIB8090_GPIO_DEFAULT_VALUES,
        DIB8090_GPIO_DEFAULT_PWM_POS,

        0,     // dib0090_freq_offset_khz_uhf
        -143,  // dib0090_freq_offset_khz_vhf

        0,
        12000, // clock_khz
        4,
        0,
        0,
        0,
        0,

        0x2d98, // dib8k_drives
        96,   //diversity_delay
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

        0,     // dib0090_freq_offset_khz_uhf
        -143,  // dib0090_freq_offset_khz_vhf

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

        0,     // dib0090_freq_offset_khz_uhf
        -143,  // dib0090_freq_offset_khz_vhf

        0,
        12000, // clock_khz
        4,
        0,
        0,
        0,
        0,

        0x2d98, // dib8k_drives
        1,   //diversity_delay
        0x31,  //div_cfg
        1, //clkouttobamse
        1, // clkoutdrive
        3, // refclksel
    },
};

static int (*tfe8096md4_tuner_tune_save) (struct dibFrontend *fe , struct dibChannel *channel);
static int tfe8096md4_tuner_tune (struct dibFrontend *fe , struct dibChannel *channel)
{
    int return_value;

    if (fe->tune_state == CT_TUNER_START) {
        switch(BAND_OF_FREQUENCY(channel->RF_kHz)) {
            case BAND_VHF: //gpio6 : 1
                demod_set_gpio(fe, 6, 0, 1);
                break;
            case BAND_UHF: //gpio6 : 0
                demod_set_gpio(fe, 6, 0, 0);
                break;
            default:
                dbgpl(NULL,"Warning : this frequency is not in the supported range, using VHF switch");
        }
    }
    return_value = tfe8096md4_tuner_tune_save(fe, channel);

    return return_value;
}


int main (void)
{
    // default I2C implementation is based on parallel port but user can connect its
    // own I2C driver using host_i2c_interface_attach();
    // implementation is done in sample/interface/host.c
    //struct dibDataBusHost *i2c = host_i2c_interface_attach(NULL), *b;
	struct dibDataBusHost *i2c = open_spp_i2c(), *b;
	struct dibFrontend fe[4];
	struct dibChannel ch;
	struct dibDemodMonitor mon[4];

	DibZeroMemory(&mon, sizeof(mon));

    if (i2c == NULL)
        return 1;

    frontend_init(&fe[0]); /* initializing the frontend-structure */
    frontend_set_id(&fe[0], 0); /* assign an absolute ID to the frontend */
    frontend_set_description(&fe[0], "ISDB-T #0 Master");

    frontend_init(&fe[1]); /* initializing the frontend-structure */
    frontend_set_id(&fe[1], 1); /* assign an absolute ID to the frontend */
    frontend_set_description(&fe[1], "ISDB-T #1 Slave");

    frontend_init(&fe[2]); /* initializing the frontend-structure */
    frontend_set_id(&fe[2], 2); /* assign an absolute ID to the frontend */
    frontend_set_description(&fe[2], "ISDB-T #2 Slave");

    frontend_init(&fe[3]); /* initializing the frontend-structure */
    frontend_set_id(&fe[3], 3); /* assign an absolute ID to the frontend */
    frontend_set_description(&fe[3], "ISDB-T #3 Slave");

    if ( dib8090_sip_register(&fe[0], i2c, 0x90, &tfe8096md4_config[0]) == NULL)
        return DIB_RETURN_ERROR;
    b = dib8000_get_i2c_master(&fe[0], DIBX000_I2C_INTERFACE_GPIO_1_2, 1);
    if ( dib8090_sip_register(&fe[1], b, 0x92, &tfe8096md4_config[1]) == NULL)
        return DIB_RETURN_ERROR;
    if ( dib8090_sip_register(&fe[2], b, 0x94, &tfe8096md4_config[2]) == NULL)
        return DIB_RETURN_ERROR;
    if ( dib8090_sip_register(&fe[3], b, 0x96, &tfe8096md4_config[3]) == NULL)
        return DIB_RETURN_ERROR;

    /* overide the tuner tune function to drive the external switch */
    tfe8096md4_tuner_tune_save = fe[0].tuner_info->ops.tune_digital;
    fe[0].tuner_info->ops.tune_digital = tfe8096md4_tuner_tune;
    fe[1].tuner_info->ops.tune_digital = tfe8096md4_tuner_tune;
    fe[2].tuner_info->ops.tune_digital = tfe8096md4_tuner_tune;
    fe[3].tuner_info->ops.tune_digital = tfe8096md4_tuner_tune;

     /* do the i2c-enumeration for 4 demod and use 0x90 as the I2C address for first device */
    b = data_bus_client_get_data_bus(demod_get_data_bus_client(&fe[0]));
    dib8000_i2c_enumeration(b, 1, 0x22, 0x90);
    b = data_bus_client_get_data_bus(demod_get_data_bus_client(&fe[1]));
    dib8000_i2c_enumeration(b, 3, DIB8090_DEFAULT_I2C_ADDRESS, 0x92);

    dib8090_set_wbd_target(&fe[0], 250, 425);
    dib8090_set_wbd_target(&fe[1], 250, 425);
    dib8090_set_wbd_target(&fe[2], 250, 425);
    dib8090_set_wbd_target(&fe[3], 250, 425);

    frontend_reset(&fe[0]);
    frontend_reset(&fe[1]);
    frontend_reset(&fe[2]);
    frontend_reset(&fe[3]);

    INIT_CHANNEL(&ch, STANDARD_ISDBT);
    ch.RF_kHz = 665143;
    ch.bandwidth_kHz = 6000;

    tune_diversity(&fe[0], 4, &ch);

    DibDbgPrint("-I-  Tuning done <enter>\n");
    getchar();

    while (1) {
        /* after enumerating on the same i2c-bus, the i2c-addresses of the bus will be 0x80 for the diversity master and 0x82 for the slave */
        //demod_agc_startup_ex(&fe, &ch);
        demod_get_monitoring(&fe[0], &mon[0]);
        demod_get_monitoring(&fe[1], &mon[1]);
        demod_get_monitoring(&fe[2], &mon[2]);
        demod_get_monitoring(&fe[3], &mon[3]);
        dib7000_print_monitor(mon, NULL, 0 ,4);
        usleep(100000);
    }

    DibDbgPrint("-I-  Cleaning up\n");
    frontend_unregister_components(&fe[0]);
    frontend_unregister_components(&fe[1]);
    frontend_unregister_components(&fe[2]);
    frontend_unregister_components(&fe[3]);

    //host_i2c_release(i2c);
	close_spp_i2c();

	return 0;
}
