#ifndef __TDAC_7_TUNER_H__
#define __TDAC_7_TUNER_H__

void Set_Tuner_TDAC2_DTMB(int channel_frequency_KHz);
void Tuner_I2C_Write(unsigned char  tuner_addr, unsigned char * data, unsigned char length);
void Tuner_I2C_Read(unsigned char  tuner_addr, unsigned char * data, unsigned char length);

#endif

