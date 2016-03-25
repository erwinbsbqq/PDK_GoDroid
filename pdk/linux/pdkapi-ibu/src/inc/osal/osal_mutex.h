/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 - 2003 Copyright (C)
*
*    File:    osal_mutex.h
*
*    Description:    This file contains all functions declare
*		             of OSAL mutex.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Dec.15.2005      Goliath Peng  Ver 1.0    Create.
*****************************************************************************/

#ifndef __OSAL_MUTEX_H__
#define __OSAL_MUTEX_H__

#include <sys_config.h>
#include "osal.h"


#if (SYS_OS_MODULE == ALI_TDS2)

#define osal_mutex_create			OS_CreateMutex
#define osal_mutex_delete			OS_DeleteMutex
#define osal_mutex_lock				OS_LockMutex
#define osal_mutex_unlock			OS_UnlockMutex

#elif (SYS_OS_MODULE == LINUX_2_6_28)

#define osal_mutex_create			OS_CreateMutex
#define osal_mutex_delete			OS_DeleteMutex
#ifdef APP_LOCK_DEBUG  /* --Doy.Dong, 2013-1-15*/
#define osal_mutex_lock(a,b)		OS_LockMutex(a,b,__FUNCTION__,__LINE__)
#else
#define osal_mutex_lock				OS_LockMutex
#endif /****APP_LOCK_DEBUG****/
#define osal_mutex_unlock			OS_UnlockMutex

#else

#define osal_mutex_create(...)		OSAL_INVALID_ID
#define osal_mutex_delete(...)		OSAL_E_FAIL
#define osal_mutex_lock(...)		OSAL_E_FAIL
#define osal_mutex_unlock(...)		OSAL_E_FAIL

#endif


#endif /* __OSAL_MUTEX_H__ */

