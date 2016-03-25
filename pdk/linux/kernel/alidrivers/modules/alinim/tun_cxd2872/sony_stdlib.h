/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2012-06-14 10:22:50 #$
  File Revision : $Revision:: 5570 $
------------------------------------------------------------------------------*/
/**

 @file    sony_stdlib.h

          This file provides the C standard lib function mapping.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_STDLIB_H
#define SONY_STDLIB_H

#include "porting_cxd2872_linux.h"



/*
 PORTING. Please modify if ANCI C standard library is not available.
*/
//#include <api/libc/string.h>

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/

/**
 @brief Alias for memcpy.
*/
#define sony_memcpy  memcpy

/**
 @brief Alias for memset.
*/
#define sony_memset  memset

#endif /* SONY_STDLIB_H */
