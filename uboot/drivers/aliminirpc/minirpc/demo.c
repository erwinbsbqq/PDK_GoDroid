/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2013 Copyright (C)
 *
 *  File: demo.c
 *
 *  Description: Demo For Using Ali Remote Prcedure Call driver.
 *
 *  History:
 *      Date              Author              Version              Comment
 *      ====            ======          =======         =======
 *  1.  2013.03.14     Tony.Zh            0.1.000             Initial
 ****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <getopt.h>
#include <linux/types.h>

#include "ali_rpc.h"


#define  MAX_TEST_SYMBOL  100
#define  MAX_TEST_INVOKER 16
HashTable *test_g_ht = NULL;
typedef struct Test_RpcSymbol
{
    Int32 hash;
    Ulong func;
} Test_RpcSymbol;
Test_RpcSymbol  test_symbol[MAX_TEST_SYMBOL];

static char test_rid_tMap[RETRUN_BITMAP_LEN] = {0};
static Int32    test_rid_Buff[RETURN_BUFFER_MAX] = {0};

#define MAX_TEST_THREAD  3
Thread gtid[MAX_TEST_THREAD];
Int32  gthreadContex[MAX_TEST_THREAD] = {0};

/*Test RpcCall common thread*/
static void TestRpcThreadCommon(void *contex)
{
    Int32 threadindex = *((Int32 *)contex);
    struct timeval start, end;
    Ulong interval = 0;
    Int32 ret = 0;

    Log(LOG_DEBUG, "TestRpcThreadCommon threadindex:%d\n", threadindex);

    while (1)
    {
        PR_Sleep(threadindex + 2);
        Log(LOG_DEBUG, "{threadindex:%d} will send RpcCall for Demo_Service%d\n", threadindex, threadindex + 2);
        gettimeofday(&start, NULL);

        switch (threadindex)
        {
            case 0:
                ret = RpcCallCompletion(Demo_Service2, NULL);
                break;
            case 1:
                ret = RpcCallCompletion(Demo_Service3, NULL);
                break;
            case 2:
                ret = RpcCallCompletion(Demo_Service4, NULL);
                break;
            case 3:
                ret = RpcCallCompletion(Demo_Service5, NULL);
                break;
            case 4:
                ret = RpcCallCompletion(Demo_Service6, NULL);
                break;
            case 5:
                ret = RpcCallCompletion(Demo_Service7, NULL);
                break;
            case 6:
                ret = RpcCallCompletion(Demo_Service8, NULL);
                break;
            case 7:
                ret = RpcCallCompletion(Demo_Service9, NULL);
                break;
            default:
                break;

        }

        gettimeofday(&end, NULL);
        interval = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);


        Log(LOG_DEBUG, "{threadindex:%d} RpcCallCompletion return, ret:%d, consumed time: %f\n", threadindex, ret, interval / 1000.0);
    }

}

/*Services*/
static Int32 Demo_Service1(Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
{
    Log(LOG_DEBUG, "<Main>Entered Demo_Service1 execution!\n");

    return  0;
}

static Int32 Demo_Service2(Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
{
    Log(LOG_DEBUG, "<Main>Entered Demo_Service2 execution!\n");

    return  0;
}

static Int32 Demo_Service3(Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
{
    Log(LOG_DEBUG, "<Main>Entered Demo_Service3 execution!\n");

    return  0;
}

static Int32 Demo_Service4(Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
{
    Log(LOG_DEBUG, "<Main>Entered Demo_Service4 execution!\n");

    return  0;
}

static Int32 Demo_Service5(Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
{
    Log(LOG_DEBUG, "<Main>Entered Demo_Service5 execution!\n");

    return  0;
}

static Int32 Demo_Service6(Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
{
    Log(LOG_DEBUG, "<Main>Entered Demo_Service6 execution!\n");

    return  0;
}

EXPORT_RPC(Demo_Service1);
EXPORT_RPC(Demo_Service2);
EXPORT_RPC(Demo_Service3);
EXPORT_RPC(Demo_Service4);
EXPORT_RPC(Demo_Service5);
EXPORT_RPC(Demo_Service6);

static void HashTableTest(void)
{
    int i = 0;
    Ulong *pfunc = NULL;

    printf("\nStartting HashtableTest!\n");
    /*Initial the test data*/
    for (i = 0; i < MAX_TEST_SYMBOL; i++)
    {
        test_symbol[i].hash = i;
        test_symbol[i].func = 0xf0000000 + i;
    }
    /*Init hash table*/
    if ((test_g_ht = HashTableCreate(128, HashFunctionImt, HashComparatorKey, HashComparatorValue,
                                     NULL, NULL)) == NULL)
    {
        Log(LOG_ERR, "[HashTableTest] HashTable Create Failed!\n");
        return;
    }
    for (i = 0; i < MAX_TEST_SYMBOL; i++)
    {
        if ((HashTableAdd(test_g_ht, (const void *)&test_symbol[i].hash, (void *)&test_symbol[i].func)) == NULL)
        {
            Log(LOG_ERR, "[HashTableTest] HashTableAdd Failed! i:%d\n", i);
            goto funcOut;
        }
    }
    /*Lookup hash table*/
    for (i = 0; i < MAX_TEST_SYMBOL + 10; i++)
    {
        if ((pfunc = HashTableLookupConst(test_g_ht, (void *)&i)) != NULL)
        {
            Log(LOG_DEBUG, "Kit, [key:0x%x, func:0x%x]\n", i, *pfunc);
        }
        else
        {
            Log(LOG_DEBUG, "Not Kit, [key:0x%x]\n", i);
        }

    }

funcOut:
    if (test_g_ht)
    {
        HashTableDestroy(test_g_ht);
    }

    printf("\nCompleting HashtableTest!\n");
}

static void ListTableTest(void)
{
    printf("\nStartting ListtableTest!\n");
    Invoker test_invoker[MAX_TEST_INVOKER];
    int i = 0;
    CList test_list;
    CList *e = NULL;
    Invoker *pinvoker = NULL;

    /*Init the List table*/
    for (i = 0; i < MAX_TEST_INVOKER; i++)
    {
        test_invoker[i].rid = i + 0xF0;
    }
    LIST_INIT(&test_list);
    /*Add all invoker to List table*/
    for (i = 0; i < MAX_TEST_INVOKER; i++)
    {
        LIST_APPEND_LINK(&test_invoker[i].list, &test_list);
    }
    /*Lookup list table*/
    for (i = 0; i < MAX_TEST_INVOKER + 10; i++)
    {
        if (LIST_IS_EMPTY(&test_list))
        {
            Log(LOG_DEBUG, "List Not Hit, [index:%d, List Max Size:%d]\n", i, MAX_TEST_INVOKER);
            continue;
        }
        e = LIST_HEAD(&test_list);
        if (e)
        {
            LIST_REMOVE_LINK(e);
        }
        pinvoker = (Invoker *)e;
        if (pinvoker)
        {
            Log(LOG_DEBUG, "List Hit, [index:%d, rid:0x%x]\n", i, pinvoker->rid);
        }
    }

    printf("\nCompleting ListtableTest!\n");

}

#define TEST_SVC_QUEUE_SIZE 2
static void QueueTest(void)
{
    Queue test_queue;
    Int32 test_task[TEST_SVC_QUEUE_SIZE + 10] = {0}, i = 0, *pi = NULL, *pi2 = NULL;
    Uint64 ltemp1 = 0, ltemp2 = 0;

    printf("\nStartting QueueTest!\n");
    /*Initializing svcQueue of InvokerManager*/
    //Log(LOG_DEBUG,"QueueTest start I\n");
    if (QueueInit((void *)&test_queue, sizeof(void *), SVC_QUEUE_SIZE) != 0)
    {
        return;
    }
    //Log(LOG_DEBUG,"QueueTest start II\n");

    /*Push all the elements to Queue*/
    for (i = 0; i < TEST_SVC_QUEUE_SIZE; i++)
    {
        test_task[i] = i + 0xF0;
        QueuePush(&test_queue, (void *)&test_task[i]);
    }
    pi = pi2 = &test_task[0];
    //Log(LOG_DEBUG,"QueueTest start III, pi:0x%x, value:0x%x\n", pi, *pi2);
    /*lookup Queue*/
    //for(i=0;i<TEST_SVC_QUEUE_SIZE+10;i++){
    for (i = 0; i < 1; i++)
    {
        pi = (Int32 *)QueueFront(&test_queue);
        //Log(LOG_DEBUG,"QueueTest start III_2, test_task[%d]_addr:0x%x, pi:0x%x\n",i, &test_task[i], pi);
        if (pi)
        {
            ltemp1 = pi;
            ltemp2 = pi;
            //Log(LOG_DEBUG,"ltemp1:%2x%2x%2x%2x%2x%2x%2x%2x\n",(ltemp1>>56)&0xff,(ltemp1>>48)&0xff,(ltemp1>>40)&0xff,(ltemp1>>32)&0xff,
            //(ltemp1>>24)&0xff,(ltemp1>>16)&0xff,(ltemp1>>8)&0xff,ltemp1 & 0xff );
            QueuePop(&test_queue);
            Log(LOG_DEBUG, "Queue Kit, [index:%d, value:0x%x]\n", i, *pi);
        }
        else
        {
            Log(LOG_DEBUG, "Queue Not Kit, [index:%d, QUEUE_SIZE:%d]\n", i, SVC_QUEUE_SIZE);
        }
    }
    /*Try to free 20 Queue*/
    Log(LOG_DEBUG, "Try to free 20 Queue element\n");
    /*for(i=0;i<TEST_SVC_QUEUE_SIZE+10;i++)
        QueuePop(&test_queue);*/

    /*Try to insert 16 element to Queue*/
    Log(LOG_DEBUG, "Try to insert 16 element to Queue\n");
    for (i = 2; i < TEST_SVC_QUEUE_SIZE + 2; i++)
    {
        test_task[i] = i + 0x200;
        QueuePush(&test_queue, (void *)&test_task[i]);
    }

    /*Try to lookup Queue element again*/
    Log(LOG_DEBUG, "Try to lookup Queue element again\n");
    for (i = 0; i < TEST_SVC_QUEUE_SIZE + 10; i++)
    {
        pi = (Int32 *)QueueFront(&test_queue);
        if (pi)
        {
            QueuePop(&test_queue);
            Log(LOG_DEBUG, "Queue Kit II, [index:%d, value:0x%x]\n", i, *pi);
        }
        else
        {
            Log(LOG_DEBUG, "II Queue Not Kit, [index:%d, QUEUE_SIZE:%d]\n", i, SVC_QUEUE_SIZE);
        }
    }

    printf("\nCompleting QueueTest!\n");
}

static void RidTest(void)
{
    Int32 test_nrFree = 0, test_lastRid = 0, i = 0, offset = 0, rid = 0;

    printf("\nStartting RidTest!\n");
    /*Init the buffer value*/
    for (i = 0; i < RETURN_BUFFER_MAX; i++)
    {
        test_rid_Buff[i] = i + 0x100;
    }
    test_nrFree = RETURN_BUFFER_MAX;
    test_lastRid = -1;

    /*Lookup all the elements*/
    for (i = 0; i < RETURN_BUFFER_MAX + 10; i++)
    {
        rid = test_lastRid + 1;
        offset = rid & BITS_PER_RETMAP_MASK;
        if (!test_nrFree)
        {
            Log(LOG_DEBUG, "Rid allocated Full! i:%d, MAX_ALLOCATED:%d\n", i, RETURN_BUFFER_MAX);
            continue;
        }
        offset = Rid_find_next_zero_bit(&test_rid_tMap, BITS_PER_RETMAP, offset);
        if (BITS_PER_RETMAP != offset && !Rid_test_and_set_bit(offset, &test_rid_tMap))
        {
            --test_nrFree;
            test_lastRid = offset;
            Log(LOG_DEBUG, "Rid allocated success, rid:%d, i:%d\n", offset, i);
        }
        else
        {
            Log(LOG_DEBUG, "Rid allocated Full II! offset:%d, nrFree:%d, i:%d, MAX_ALLOCATED:%d\n", offset, test_nrFree, i, RETURN_BUFFER_MAX);
            continue;
        }

    }

    /*Try to free the rid*/
    Log(LOG_DEBUG, "Try to free the first 20 rid\n");
    for (i = 0; i < 20; i++)
    {
        Rid_clear_bit(i, &test_rid_tMap);
        ++test_nrFree;
    }
    /*Lookup all the elements*/
    for (i = 0; i < RETURN_BUFFER_MAX + 10; i++)
    {
        rid = test_lastRid + 1;
        offset = rid & BITS_PER_RETMAP_MASK;
        if (!test_nrFree)
        {
            Log(LOG_DEBUG, "Rid allocated Full! i:%d, MAX_ALLOCATED:%d\n", i, RETURN_BUFFER_MAX);
            continue;
        }
        offset = Rid_find_next_zero_bit(&test_rid_tMap, BITS_PER_RETMAP, offset);
        if (BITS_PER_RETMAP != offset && !Rid_test_and_set_bit(offset, &test_rid_tMap))
        {
            --test_nrFree;
            test_lastRid = offset;
            Log(LOG_DEBUG, "Rid allocated success, rid:%d, i:%d\n", offset, i);
        }
        else
        {
            Log(LOG_DEBUG, "Rid allocated Full II! offset:%d, nrFree:%d, i:%d, MAX_ALLOCATED:%d\n", offset, test_nrFree, i, RETURN_BUFFER_MAX);
            continue;
        }

    }

    printf("\nCompleting RidTest!\n");

}

extern const struct _RpcSymbol __start___rpctab[];
extern const struct _RpcSymbol __stop___rpctab[];

static void ServiceSymbolTest(void)
{
    Int32 count = 0;
    PRpcFunc pFunc = NULL;
    Ulong *utemp = NULL;

    printf("\nStartting ServiceSymbolTest! RpcSymbol size:%d\n", sizeof(RpcSymbol));

    if ((test_g_ht = HashTableCreate(MAX_SERVICE_HASHTB_BUCKET_SIZE, HashFunctionImt, HashComparatorKey, HashComparatorValue,
                                     NULL, NULL)) == NULL)
    {
        Log(LOG_ERR, "HashTable Create Failed!\n");
        return;
    }

    /*Read All Service Symbol to add into hash table from section "___rpctab" */
    RpcSymbol *symbol = NULL, *lastsymbol = NULL;

    symbol = (RpcSymbol *) &__start___rpctab;
    lastsymbol = (RpcSymbol *) &__stop___rpctab;

    //Log(LOG_DEBUG,"symbol section total symbol:0x%x, lastsymbol:0x%x, length:0x%x\n", symbol, lastsymbol,(Ulong)((Ulong)lastsymbol-(Ulong)symbol));
    while (symbol < lastsymbol)
    {
        if (symbol->name)
        {
            symbol->hash = hash(symbol->name);
        }
        Log(LOG_DEBUG, "count:%d, hash:0x%x, name:%s, func:0x%x\n", count, symbol->hash, symbol->name, symbol->func);
        if (HashTableAdd(test_g_ht, (const void *)&symbol->hash, (void *)&symbol->func))
        {
            count++;
        }
        else
        {
            Log(LOG_ERR, "HashTableAdd Failed! count:%d\n", count);
        }
        //Log(LOG_DEBUG,"symbol pointer debug, old symbol:0x%x, symbol->hash:0x%x, symbol->func:0x%x\n", symbol,&symbol->hash, &symbol->func);
        symbol++;
        //Log(LOG_DEBUG,"symbol pointer debug, ++ symbol:0x%x\n", symbol);
    }
    Log(LOG_DEBUG, "Total added %d element into service hashtable!\n", count);

    /*Lookup service hash table*/
    Uint32 itemp = 0;

    itemp = HASH_STR(Demo_Service1);
    utemp = HashTableLookupConst(test_g_ht, &itemp);
    Log(LOG_DEBUG, "Demo_Service1 hash:0x%x\n", itemp);
    pFunc = (PRpcFunc)(*utemp);
    if (pFunc)
    {
        (pFunc)(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    }

    itemp = HASH_STR(Demo_Service2);
    Log(LOG_DEBUG, "Demo_Service2 hash:0x%x\n", itemp);
    utemp = HashTableLookupConst(test_g_ht, &itemp);
    pFunc = (PRpcFunc)(*utemp);
    if (pFunc)
    {
        (pFunc)(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    }

    itemp = HASH_STR(Demo_Service3);
    Log(LOG_DEBUG, "Demo_Service3 hash:0x%x\n", itemp);
    utemp = HashTableLookupConst(test_g_ht, &itemp);
    pFunc = (PRpcFunc)(*utemp);
    if (pFunc)
    {
        (pFunc)(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    }


    itemp = HASH_STR(Demo_Service4);
    Log(LOG_DEBUG, "Demo_Service4 hash:0x%x\n", itemp);
    utemp = HashTableLookupConst(test_g_ht, &itemp);
    pFunc = (PRpcFunc)(*utemp);
    if (pFunc)
    {
        (pFunc)(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    }

    itemp = HASH_STR(Demo_Service5);
    Log(LOG_DEBUG, "Demo_Service5 hash:0x%x\n", itemp);
    utemp = HashTableLookupConst(test_g_ht, &itemp);
    pFunc = (PRpcFunc)(*utemp);
    if (pFunc)
    {
        (pFunc)(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    }

    itemp = HASH_STR(Demo_Service6);
    Log(LOG_DEBUG, "Demo_Service6 hash:0x%x\n", itemp);
    utemp = HashTableLookupConst(test_g_ht, &itemp);
    pFunc = (PRpcFunc)(*utemp);
    if (pFunc)
    {
        (pFunc)(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    }

    itemp = HASH_STR(Demo_Service7);
    utemp = HashTableLookupConst(test_g_ht, &itemp);
    if (!utemp)
    {
        Log(LOG_DEBUG, "Try to lookup not exit 'Demo_Service7' in hash table, Not Hit!\n");
    }

    printf("\nCompleting ServiceSymbolTest!\n");
}

static void GetDebugInfo(void)
{
    RPCDBGINFO info;

    /*Try to get local debug info*/
    Log(LOG_DEBUG, "GetDebugInfo will  get local debug info\n");
    RpcGetDetailInfo(&info, 0);

    /*Try to get remote debug info*/
    //TBD...
}

void RpcCallTest(Int32 localNode, Int32 remoteNode)
{
    Int32 ret = 0, i = 0;
    struct timeval start, end;
    Ulong interval = 0;
    TestStruct  tstruct, tstruct_ret;

    Log(LOG_DEBUG, "Will startting RpcCallTest! localNode:%d, remoteNode:%d\n", localNode, remoteNode);

    if (RpcInit(localNode, remoteNode) != 0)
    {
        Log(LOG_DEBUG, "RpcInit() call failed!\n");
        return;
    }
#if 1
    /*startup multi threads to do parallel rpcCall test*/
    for (i = 0; i < MAX_TEST_THREAD; i++)
    {
        gthreadContex[i] = i;
        if (PR_ThreadCreate(&gtid[i], &TestRpcThreadCommon, (void *)&gthreadContex[i], 0, 0, NULL) != RPC_OS_OK)
        {
            Log(LOG_ERR, "RpcCallTest create test thread failed!,i:%d\n", i);
            return;
        }
    }
#endif

    tstruct.ii = -111;
    tstruct.uii = 0x55aa;
    tstruct.cc = 'T';
    tstruct.ucc = 0x33;
    tstruct.ll = 0xaaaa;
    tstruct.ull = 0xaaaa5555;
    tstruct.bb = True;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_TESTSTRUCT, sizeof(TestStruct), &tstruct);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_TESTSTRUCT, sizeof(TestStruct), &tstruct_ret);

    while (1)
    {

        Log(LOG_DEBUG, "\n~~~~~Will send RpcCall~~~~~~\n");
        RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_TESTSTRUCT, sizeof(TestStruct), &tstruct);
        RPC_PARAM_UPDATE(p2, PARAM_OUT, PARAM_TESTSTRUCT, sizeof(TestStruct), &tstruct_ret);
        memset(&tstruct_ret, 0, sizeof(TestStruct));
        gettimeofday(&start, NULL);

#if 1

        ret = RpcCallCompletion(Demo_Service1, &p1, &p2, NULL);
        gettimeofday(&end, NULL);
        interval = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);



        Log(LOG_DEBUG, "RpcCallCompletion return, ret:%d, consumed time: %f\n", ret, interval / 1000.0);
        Log(LOG_DEBUG, "RpcCallCompletion, [peer tstruct_ret result] ii:%d, uii:0x%x, cc:%c, ucc:0x%x, ll:0x%x, ull:0x%x, bb:0x%x\n",
            tstruct_ret.ii, tstruct_ret.uii, tstruct_ret.cc, tstruct_ret.ucc, tstruct_ret.ll, tstruct_ret.ull, tstruct_ret.bb);

#endif

        /*For non-blocking rpcCall*/
#if 0
        rid = RpcCall(Demo_Service1, &p1, &p2, NULL);
        Log(LOG_DEBUG, "RpcCall non-blocking call return,[rid]:%d\n", rid);
        if (rid != RPC_ERROR_VALUE)
        {

            ret = RpcWait(rid, 1500);
            if (ret == RPC_OS_OK)
            {

                result = RpcExtractRet(rid);
                Log(LOG_DEBUG, "RpcCall returned result:%d from peer\n", result);
            }
            else
            {
                Log(LOG_DEBUG, "RpcWait timeout or error return, ret:%d\n", ret);
            }
            RpcFreeRetBuf(rid);
        }
        else
        {
            Log(LOG_ERR, "RpcCall failed!\n");
        }
#endif
        /*Non-blocking Call end*/


        PR_Sleep(5);
        //GetDebugInfo();

        //PR_uSleep(10000*1000);
    }


    Log(LOG_DEBUG, "Completing RpcCallTest!\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Arguments not right, Usage example: alirpcdemo hashtabletest\n");
        printf("Supportted commands:\nhashtabletest\nlisttabletest\nqueuetest\nridtest\nservicesymboltest\ngetdebuginfo\nrpccalltest\n");
        return -1;
    }

    printf("long:%d, void*:%d, Uint32*:%d, int:%d, short:%d, char:%d\n", sizeof(unsigned long), sizeof(void *), sizeof(Uint32 *), sizeof(unsigned int),
           sizeof(unsigned short), sizeof(char));

    if (!strcmp(argv[1], "hashtabletest"))
    {
        HashTableTest();
    }
    else if (!strcmp(argv[1], "listtabletest"))
    {
        ListTableTest();
    }
    else if (!strcmp(argv[1], "queuetest"))
    {
        QueueTest();
    }
    else if (!strcmp(argv[1], "ridtest"))
    {
        RidTest();
    }
    else if (!strcmp(argv[1], "servicesymboltest"))
    {
        ServiceSymbolTest();
    }
    else if (!strcmp(argv[1], "getdebuginfo"))
    {
        GetDebugInfo();
    }
    else if (!strcmp(argv[1], "rpccalltest"))
    {
        if (argc != 4)
        {
            printf("Please input 'rpccalltest' correct argument, example: ./demo rpccalltest 1 2\n");
            return -1;
        }
        RpcCallTest(atoi(argv[2]), atoi(argv[3]));
    }
    else
    {
        printf("\nDoesn't support this command:%s\n", argv[1]);
    }


    return  0;
}


