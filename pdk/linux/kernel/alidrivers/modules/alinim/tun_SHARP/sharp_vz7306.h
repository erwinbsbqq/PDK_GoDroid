#ifndef _SHARP_VZ7306_H_
#define _SHARP_VZ7306_H_




#include "../basic_types.h"


#define REF_OSC_FREQ	4000 /* 4MHZ */
  
#ifdef __cplusplus
extern "C"
{
#endif



INT32 ali_nim_vz7306_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 ali_nim_vz7306_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
INT32 ali_nim_vz7306_status(UINT32 tuner_id, UINT8 *lock);
INT32 ali_nim_vz7306_close(void);



#ifdef __cplusplus
}
#endif

#endif  /* _SHARP_VZ7306_H_ */

