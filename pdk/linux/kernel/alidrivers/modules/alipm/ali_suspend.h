/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_suspend.h
 *  (I)
 *  Description: ALi power management implementation
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2011.03.07				Owen			Creation
 ****************************************************************************/
 
#ifndef __INCLUDE_KERNEL_ALI_SUSPEND_H____
#define __INCLUDE_KERNEL_ALI_SUSPEND_H____

#include "ali_pm.h"

extern void ali_suspend_register_ops(void);
extern void ali_suspend_set_resume_key(pm_key_t *pm_key);
extern void ali_suspend_set_standby_param(pm_param_t *p_standby_param);

#endif
