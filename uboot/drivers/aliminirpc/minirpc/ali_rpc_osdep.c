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
Int32 PR_CondVarWait_Safe(Int32 *cvar, Uint32 timeout)
{
	 while(timeout--)
	 {
		 if(1 == (*cvar))
		 {
	//	    printf("ok, recv nofity!\n");
		    return 0;
		 }
	   	 mdelay(1);	
	 }
	 
	 return -1;
}

/*a wrap function for adding mutex lock*/
Int32 PR_CondVarNotify_Safe(Int32 *cvar)
{
    *cvar = 1;
    return  0;

}

