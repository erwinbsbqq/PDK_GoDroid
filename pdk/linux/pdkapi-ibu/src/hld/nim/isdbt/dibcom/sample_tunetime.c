#include <sys/time.h>
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

        0,     // dib0070_freq_offset_khz_uhf
        -143,    // dib0070_freq_offset_khz_vhf

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

        0,     // dib0070_freq_offset_khz_uhf
        -143,    // dib0070_freq_offset_khz_vhf

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

        0,     // dib0070_freq_offset_khz_uhf
        0,     // dib0070_freq_offset_khz_vhf

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

        0,     // dib0070_freq_offset_khz_uhf
        0,// dib0070_freq_offset_khz_vhf

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



struct AvgTime
{
  unsigned int nb;
  unsigned int min;
  unsigned int max;
  unsigned int avg;
  struct timeval start;
};

struct AvgTime Avg[10];

void AvgTimeInit(void)
{
  unsigned int i;
  for (i=0;i<10;i++)
  {
    Avg[i].nb = 0;
    Avg[i].min = 0xFFFFFFFF;
    Avg[i].max = 0;
    Avg[i].avg = 0;
  }
}

void AvgTimeAdd(unsigned int index, unsigned int time)
{
  Avg[index].avg += time;
  Avg[index].nb ++;

  if (time < Avg[index].min)
    Avg[index].min = time;
  if (time > Avg[index].max)
    Avg[index].max = time;
}

void AvgTimeGet(unsigned int index, unsigned int *avg, unsigned int *min, unsigned int*max)
{
  *avg = Avg[index].avg / Avg[index].nb;
  *min = Avg[index].min;
  *max = Avg[index].max;
}

void AvgTimeStart(unsigned int index)
{
   gettimeofday(&Avg[index].start, NULL);
}
unsigned int AvgTimeEnd(unsigned int index)
{
  struct timeval end;
  unsigned int diffms;

  gettimeofday(&end, NULL);

  if (end.tv_sec > (Avg[index].start.tv_sec))
  {
    if (end.tv_sec > (Avg[index].start.tv_sec+1))
    {
      diffms = (end.tv_sec - Avg[index].start.tv_sec - 1)*1000;
    }
    else
      diffms = 0;

    diffms += (1000000 - Avg[index].start.tv_usec)/1000;
    diffms += (end.tv_usec)/1000;
  }
  else
  {
    diffms = (end.tv_usec - Avg[index].start.tv_usec)/1000;
  }
  return diffms;

}

struct dibFrontend fe[4];
struct dibChannel ch;
struct dibDemodMonitor mon[4];
struct dibDataBusHost *i2c;
struct dibDataBusHost *b;

int TestInit(unsigned char NbChip)
{
    i2c = open_spp_i2c();

    if (NbChip > 0)
    {
      frontend_init(&fe[0]); /* initializing the frontend-structure */
      frontend_set_id(&fe[0], 0); /* assign an absolute ID to the frontend */
      frontend_set_description(&fe[0], "ISDB-T #0 Master");
      if ( dib8090_sip_register(&fe[0], i2c, 0x90, &tfe8096md4_config[0]) == NULL)
        return DIB_RETURN_ERROR;
      b = dib8000_get_i2c_master(&fe[0], DIBX000_I2C_INTERFACE_GPIO_1_2, 1);
      tfe8096md4_tuner_tune_save = fe[0].tuner_info->ops.tune_digital;
      fe[0].tuner_info->ops.tune_digital = tfe8096md4_tuner_tune;
    }

    if (NbChip > 1)
    {
      frontend_init(&fe[1]); /* initializing the frontend-structure */
      frontend_set_id(&fe[1], 1); /* assign an absolute ID to the frontend */
      frontend_set_description(&fe[1], "ISDB-T #1 Slave");
      if ( dib8090_sip_register(&fe[1], b, 0x92, &tfe8096md4_config[1]) == NULL)
          return DIB_RETURN_ERROR;
      fe[1].tuner_info->ops.tune_digital = tfe8096md4_tuner_tune;
    }


    if (NbChip > 2)
    {
      frontend_init(&fe[2]); /* initializing the frontend-structure */
      frontend_set_id(&fe[2], 2); /* assign an absolute ID to the frontend */
      frontend_set_description(&fe[2], "ISDB-T #2 Slave");
      if ( dib8090_sip_register(&fe[2], b, 0x94, &tfe8096md4_config[2]) == NULL)
          return DIB_RETURN_ERROR;
      fe[2].tuner_info->ops.tune_digital = tfe8096md4_tuner_tune;
    }

    if (NbChip > 3)
    {
      frontend_init(&fe[3]); /* initializing the frontend-structure */
      frontend_set_id(&fe[3], 3); /* assign an absolute ID to the frontend */
      frontend_set_description(&fe[3], "ISDB-T #3 Slave");
        if ( dib8090_sip_register(&fe[3], b, 0x96, &tfe8096md4_config[3]) == NULL)
          return DIB_RETURN_ERROR;
      fe[3].tuner_info->ops.tune_digital = tfe8096md4_tuner_tune;

    }

       /* do the i2c-enumeration for 4 demod and use 0x90 as the I2C address for first device */
      b = data_bus_client_get_data_bus(demod_get_data_bus_client(&fe[0]));
      dib8000_i2c_enumeration(b, 1, 0x22, 0x90);
        b = data_bus_client_get_data_bus(demod_get_data_bus_client(&fe[1]));
      dib8000_i2c_enumeration(b, 3, DIB8090_DEFAULT_I2C_ADDRESS, 0x92);

    if (NbChip > 0)
    {
      dib8090_set_wbd_target(&fe[0], 250, 425);
      frontend_reset(&fe[0]);
    }
    else if (NbChip > 1)
    {
      dib8090_set_wbd_target(&fe[0], 250, 425);
      dib8090_set_wbd_target(&fe[1], 250, 425);
      frontend_reset(&fe[0]);
      frontend_reset(&fe[1]);
    }
    else if (NbChip > 2)
    {
      dib8090_set_wbd_target(&fe[0], 250, 425);
      dib8090_set_wbd_target(&fe[1], 250, 425);
      dib8090_set_wbd_target(&fe[2], 250, 425);
      frontend_reset(&fe[0]);
      frontend_reset(&fe[1]);
      frontend_reset(&fe[2]);
    }
    else if (NbChip > 3)
    {
      dib8090_set_wbd_target(&fe[0], 250, 425);
      dib8090_set_wbd_target(&fe[1], 250, 425);
      dib8090_set_wbd_target(&fe[2], 250, 425);
      dib8090_set_wbd_target(&fe[3], 250, 425);
      frontend_reset(&fe[0]);
      frontend_reset(&fe[1]);
      frontend_reset(&fe[2]);
      frontend_reset(&fe[3]);
    }

   return 0;
}

void TestTune(unsigned char NbChip, struct dibChannel *ch)
{

    /*** Tune ***/
    tune_diversity(&fe[0], NbChip, ch);

    /*** Wait for MPEG Lock ***/
    if (frontend_get_status(&fe[0]) == FE_STATUS_LOCKED)
    {
        do
        {
            demod_get_monitoring(&fe[0], &mon[0]);
        }
        while ((mon[0].locks.fec_mpeg == 0) && (mon[0].locks.fec_mpeg_b == 0) && (mon[0].locks.fec_mpeg_c == 0));
    }
}

void TestInitChannelNoSignal(struct dibChannel *ch)
{
    INIT_CHANNEL(ch, STANDARD_ISDBT);
    ch->RF_kHz = 665143;
    ch->bandwidth_kHz = 6000;
}

void TestInitChannelSignalLock(struct dibChannel *ch)
{
    INIT_CHANNEL(ch, STANDARD_ISDBT);
    ch->RF_kHz = 737143;
    ch->bandwidth_kHz = 6000;
}

int main (void)
{
    unsigned int min;
    unsigned int max;
    unsigned int avg;
    unsigned int i;
    unsigned int j;
    unsigned int NbChannel;
    struct dibChannel chnosignal;
    struct dibChannel chsignallock;

    unsigned int TotalChannelNumber = 4;
    unsigned int NbLoop = 10;

    TestInit(TotalChannelNumber);

    /*** First Tune to skip Init time ***/
    TestInitChannelNoSignal(&chnosignal);
    TestTune(4,&chnosignal);

    for (j=0;j < TotalChannelNumber;j++)
    {

      NbChannel = j+1;

      AvgTimeInit();

      for (i=0;i<NbLoop;i++)
      {
        /******* Tune autosearch no signal ******/
        TestInitChannelNoSignal(&chnosignal);
        AvgTimeStart(0);
        printf("Tune Freq: %d KHz TMCC: %d   Nb Channel: %d\n",chnosignal.RF_kHz,chnosignal.context.status ,NbChannel);
        TestTune(NbChannel,&chnosignal);
        AvgTimeAdd(0,AvgTimeEnd(0));

        /******* Tune autosearch signal *********/
        TestInitChannelSignalLock(&chsignallock);
        AvgTimeStart(1);
        printf("Tune Freq: %d KHz TMCC: %d   Nb Channel: %d\n",chsignallock.RF_kHz,chsignallock.context.status ,NbChannel);
        TestTune(NbChannel,&chsignallock);
        AvgTimeAdd(1,AvgTimeEnd(1));

        /******* Tune autosearch no signal ******/
        TestInitChannelNoSignal(&chnosignal);
        AvgTimeStart(0);
        printf("Tune Freq: %d KHz TMCC: %d   Nb Channel: %d\n",chnosignal.RF_kHz,chnosignal.context.status ,NbChannel);
        TestTune(NbChannel,&chnosignal);
        AvgTimeAdd(0,AvgTimeEnd(0));

          /******* Tune Tmcc Known signal ********/
        AvgTimeStart(2);
        printf("Tune Freq: %d KHz TMCC: %d   Nb Channel: %d\n",chsignallock.RF_kHz,chsignallock.context.status ,NbChannel);
        TestTune(NbChannel,&chsignallock);
        AvgTimeAdd(2,AvgTimeEnd(2));
      }

      printf("############################################################################\n");
      AvgTimeGet(0,&avg,&min,&max);
      printf("--- No Signal (Nb Channel: %d) ---  Min: %d ms Max= %d ms Avg= %d ms\n",NbChannel,min,max,avg);
      AvgTimeGet(1,&avg,&min,&max);
      printf("--- Signal Lock Autosearch (Nb Channel: %d) ---  Min: %d ms Max= %d ms Avg= %d ms\n",NbChannel,min,max,avg);
      AvgTimeGet(2,&avg,&min,&max);
      printf("--- Signal Lock Tmcc known (Nb Channel: %d) ---  Min: %d ms Max= %d ms Avg= %d ms\n",NbChannel,min,max,avg);
      printf("############################################################################\n");
   }


    DibDbgPrint("-I-  Cleaning up\n");
    frontend_unregister_components(&fe[0]);
    frontend_unregister_components(&fe[1]);
    frontend_unregister_components(&fe[2]);
    frontend_unregister_components(&fe[3]);

    close_spp_i2c();

    return 0;
}
