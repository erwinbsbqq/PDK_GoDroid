/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2012-08-10 08:43:12 #$
  File Revision : $Revision:: 5914 $
------------------------------------------------------------------------------*/

#ifndef PORTING_CXD2872_LINUX_H
#define PORTING_CXD2872_LINUX_H


/* Type definitions. */
/* <PORTING> Please comment out if conflicted */
#include "../porting_linux_header.h"
#include "../basic_types.h"


#define MAX_TUNER_SUPPORT_NUM         1

typedef INT32  (*INTERFACE_DEM_WRITE_READ_TUNER)(void * nim_dev_priv, UINT8 tuner_address, UINT8 *wdata, int wlen, UINT8* rdata, int rlen);
typedef struct
{
    void * nim_dev_priv; //for support dual demodulator.   
    //The tuner can not be directly accessed through I2C,
    //tuner driver summit data to dem, dem driver will Write_Read tuner.
    //INT32  (*Dem_Write_Read_Tuner)(void * nim_dev_priv, UINT8 slv_addr, UINT8 *wdata, int wlen, UINT8* rdata, int rlen);
    INTERFACE_DEM_WRITE_READ_TUNER  Dem_Write_Read_Tuner;
} DEM_WRITE_READ_TUNER;  //Dem control tuner by through mode (it's not by-pass mode).

#endif /* SONY_COMMON_H */
