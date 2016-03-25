/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: ali_rpc_osdep.c
 *
 *  Description: Ali Remote Prcedure Call driver between main and see CPU.
 *
 *  History:
 *      Date              Author              Version              Comment
 *      ====            ======          =======         =======
 *  1.  2013.03.14     Tony.Zh            0.1.000             Initial
 ****************************************************************************/
#include <ali_rpc_osdep.h>

/*a wrap function for adding mutex lock*/
Int32 PR_CondVarWait_Safe(CondVar *cvar, Uint32 timeout)
{
    Int32 ret = 0;

    PR_Lock(cvar->mutex);
#if defined(__ALI_LINUX_KERNEL__)
    init_completion(&cvar->cv);    /*Added by tony on 2013/05/18*/
#endif
    ret = PR_CondVarWait(cvar, timeout);

    PR_Unlock(cvar->mutex);

    return  ret;
}

/*a wrap function for adding mutex lock*/
Int32 PR_CondVarNotify_Safe(CondVar *cvar)
{

    Int32 ret = 0;

    PR_Lock(cvar->mutex);

    ret = PR_CondVarNotify(cvar);

    PR_Unlock(cvar->mutex);

    return  ret;

}

