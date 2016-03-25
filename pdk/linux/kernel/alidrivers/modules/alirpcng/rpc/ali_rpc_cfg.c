/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: ali_rpc_cfg.c
 *
 *  Description: Ali Remote Prcedure Call driver between main and see CPU.
 *
 *  History:
 *      Date              Author              Version              Comment
 *      ====            ======          =======         =======
 *  1.  2013.03.14     Tony.Zh            0.1.000             Initial
 ****************************************************************************/
#include <ali_rpc_cfg.h>
#include <ali_rpc_errno.h>
#include <ali_rpc_util.h>

HashTable *g_ht = NULL;
extern const struct _RpcSymbol __start___rpctab[];
extern const struct _RpcSymbol __stop___rpctab[];
extern Uint32 g_TotalService;

/*-----------------------------------------------------------------------------------------
* [Function]    CfgInit
*
* [Description]This interface constructs the static RPC look up table for service symbal.
*
* [Return]  0->Success,   othe value->Failed.
*------------------------------------------------------------------------------------------*/
Int32 CfgInit(void)
	{
		Int32	count = 0;
	
		if ((g_ht = HashTableCreate(MAX_SERVICE_HASHTB_BUCKET_SIZE, HashFunctionImt, HashComparatorKey, HashComparatorValue,
									NULL, NULL)) == NULL)
		{
			Log(LOG_ERR, "[CfgInit] HashTable Create Failed!\n");
			return	RPC_UTIL_ERR;
		}
	
		/*Read All Service Symbol to add into hash table from section "___rpctab" */
		RpcSymbol *symbol = NULL;
		RpcSymbol *lastsymbol = NULL;
	
		symbol = (RpcSymbol *) &__start___rpctab;
		lastsymbol = (RpcSymbol *) &__stop___rpctab;
	
		Uint32 rpctab_num = ((Uint32)lastsymbol - (Uint32)symbol)/sizeof(RpcSymbol);
		Uint32 *compare_array = (Uint32 *)PR_Malloc(rpctab_num*sizeof(Uint32));
		Uint32 index = 0;
		Uint32 i = 0;
	
		Log(LOG_ERR, "__start___rpctab:0x%x, __stop___rpctab:0x%x\n", symbol, lastsymbol);
		while (symbol < lastsymbol)
		{
			if (symbol->name)
			{
				symbol->hash = hash(symbol->name);
			}
			
			//find whether there is a same has, then warring
			for(i=0; i<index; i++)
			{
			   if(symbol->hash == *(compare_array+i))
			   {
				  Log(LOG_ERR, "^^^^^^^^^^^Error:find same hash value!!!^^^^^^^^^^^^\n", __func__);
				  break;
			   }
			}
	
			//save the hash to a compare array
			*(compare_array+index) = symbol->hash;
			index++;
			
			Log(LOG_DEBUG, "[CfgInit] Will add hash:0x%x, func:0x%x, funcname:%s to hash table\n", symbol->hash, symbol->func, symbol->name);
			if (HashTableAdd(g_ht, (const void *)&symbol->hash, (void *)&symbol->func))
			{
				count++;
			}
			else
			{
				Log(LOG_ERR, "[CfgInit] HashTableAdd Failed! count:%d\n", count);
			}
			symbol++;
		}
	
		g_TotalService = count;
		Log(LOG_ERR, "[CfgInit] Created Service Symbol Hash table size:%d\n", count);
	
	
		return	RPC_SUCCESS_VALUE;
	}


/*-----------------------------------------------------------------------------------------
* [Function]    CfgDeinit
*
* [Description]This interface destructs the RPC look up table, including statically built table and dynamicaly built table.
*
* [Return]  0->Success,   othe value->Failed.
*------------------------------------------------------------------------------------------*/
Int32 CfgDeinit(void)
{
    HashTableDestroy(g_ht);

    return  RPC_SUCCESS_VALUE;
}


/*
* Hash Function caculation implementation.
* Return: HashNumber value
*/
Uint32 HashFunctionImt(const void *key)
{
    Uint32  ui = 0;

    if (!key)
    {
        return 0;
    }
    ui = *((Uint32 *)key);

    return ui;
}

/*
* Hash Comparator Implementation.
* Return: True->op1 equal op2,  False->not equal
*/
Bool  HashComparatorKey(void *op1, void *op2)
{
    if ((*(Uint32 *)op1) == (*(Uint32 *)op2))
    {
        return True;
    }
    else
    {
        return False;
    }
}

/*
* Hash Comparator Implementation.
* Return: True->op1 equal op2,  False->not equal
*/
Bool  HashComparatorValue(void *op1, void *op2)
{
    if ((*(Ulong *)op1) == (*(Ulong *)op2))
    {
        return True;
    }
    else
    {
        return False;
    }

}


