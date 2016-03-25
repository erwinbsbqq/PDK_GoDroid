/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: pr_sleep.c
 *
 *  Description: Portable runtime os sleep (linux-kernel) implementation file.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.01.25      David.L         0.1.000         Initial
 ****************************************************************************/

#include <linux/delay.h>
#include "pr.h"

/**
 *  PR_uSleep costs cpu time at least (usec % 1000).
 */
void PR_uSleep(Uint32 usec)
{
	Uint32 temp;
    if (usec >= 1000)
    {
    	temp = usec % 1000;
        udelay(temp);
        msleep_interruptible(usec / 1000);
    }
    else
    {
        udelay(usec);
    }
}

/**
 *  PR_Sleep sleep cpu, does not cost cpu time.
 */
void PR_Sleep(Uint32 sec)
{
    msleep_interruptible(sec * 1000);
}




