/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2012-06-12 17:25:39 #$
  File Revision : $Revision:: 5553 $
------------------------------------------------------------------------------*/

#include "sony_common.h"

/* Utility function */
#define MASKUPPER(n) (((n) == 0) ? 0 : (0xFFFFFFFFU << (32 - (n))))
#define MASKLOWER(n) (((n) == 0) ? 0 : (0xFFFFFFFFU >> (32 - (n))))
/* Convert N (<32) bit 2's complement value to 32 bit signed value */

