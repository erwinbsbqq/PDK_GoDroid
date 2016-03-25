/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: ali_rpc_argsformat.c
 *
 *  Description: Ali Remote Prcedure Call driver between main and see CPU.
 *
 *  History:
 *      Date              Author              Version              Comment
 *      ====            ======          =======         =======
 *  1.  2013.03.14     Tony.Zh            0.1.000             Initial
 ****************************************************************************/
#include <ali_rpc_errno.h>
#include <ali_rpc_argsformat.h>
#include <ali_rpc_xdr.h>
#include <ali_minirpc_service.h>
#include "ali_rpc_debug.h"

const  Uint32 argindexmask =  0xff;
const  Uint32 argtypemask = 0x7f00;
const  Uint32 xflagmask = 0x8000;
const char packet_caller_mask[] = "CALL";
const char packet_return_mask[] = "RET ";

extern XdrOpTable_minirpc gXdrOpTable_minirpc[];
XdrOpFunc_minirpc FindMatchXDRProFunc_minirpc(Long paramId)
{
    XdrOpFunc_minirpc pFunc_minirpc = NULL;
    Int32 i=0;

    if (paramId < 0 || paramId >= PARAM_MINIRPC_ID_MAX)
    {
        Log(LOG_ERR, "FindMatchXDRProFunc_minirpc invalid paramId:%d\n", paramId);
        return NULL;
    }
    /*check if proc table same index is matched*/
    if (gXdrOpTable_minirpc[paramId].id == paramId)
    {
        pFunc_minirpc = gXdrOpTable_minirpc[paramId].op;
    }
    else
    {
        /*Loop check the pro table*/
        for (i = 0; i < PARAM_MINIRPC_ID_MAX; i++)
        {
            if (gXdrOpTable_minirpc[i].id == paramId)
            {
                pFunc_minirpc = gXdrOpTable_minirpc[i].op;
                break;
            }
        }
    }
    return pFunc_minirpc;
}


Bool XdrProcWrap(XDR *pxdr, void *xdrbuf, Int32 xdrbuflen, void *pelement, Int32 elementsize, enum XDR_OP op, Long paramId)
{
    XdrOpFunc_minirpc pFunc_minirpc = NULL;
    Bool bResult = True;

    if (!pxdr)
    {
        if (!xdrbuf)
        {
            Log(LOG_ERR, "XdrProcWrap xdrbuf NULL!\n");
            return False;
        }
        XDR_Create(pxdr, (Int8 *)xdrbuf, xdrbuflen, op);
    }
    if (!pelement)
    {
        Log(LOG_ERR, "XdrProcWrap pelement NULL!\n");
        return False;
    }
    if ((pFunc_minirpc = FindMatchXDRProFunc_minirpc(paramId)) == NULL)
    {
        Log(LOG_ERR, "XdrProcWrap can not findout the matched XDR func, paramId:%d\n", paramId);
        return False;
    }
    switch (paramId)
    {
	        case PARAM_MINIRPC_VOID:
	            bResult = pFunc_minirpc();
	            break;
	        case PARAM_MINIRPC_INT32:
	            bResult = pFunc_minirpc(pxdr, pelement);
	            break;
	        case PARAM_MINIRPC_UINT32:
	            bResult = pFunc_minirpc(pxdr, pelement);
	            break;
	        case PARAM_MINIRPC_LONG:
	            bResult = pFunc_minirpc(pxdr, pelement);
	            break;
	        case PARAM_MINIRPC_ULONG:
	            bResult = pFunc_minirpc(pxdr, pelement);
	            break;
	        case PARAM_MINIRPC_INT16:
	            bResult = pFunc_minirpc(pxdr, pelement);
	            break;
	        case PARAM_MINIRPC_UINT16:
	            bResult = pFunc_minirpc(pxdr, pelement);
	            break;
	        case PARAM_MINIRPC_BOOL:
	            bResult = pFunc_minirpc(pxdr, pelement);
	            break;
	        case PARAM_MINIRPC_ENUM:
	            bResult = pFunc_minirpc(pxdr, pelement);
	            break;
	        case PARAM_MINIRPC_ARRAY:
	            Log(LOG_ERR, "XdrProcWrap doesn't support Array now!");
	            bResult = False;
	            break;
	        case PARAM_MINIRPC_BYTES:
	            bResult = pFunc_minirpc(pxdr, &pelement, &elementsize, elementsize);
	            break;
	        case PARAM_MINIRPC_OPAQUE:
	            bResult = pFunc_minirpc(pxdr, pelement, elementsize);
	            break;
	        case PARAM_MINIRPC_STRING:
	            bResult = pFunc_minirpc(pxdr, &pelement, elementsize);
	            break;
	        case PARAM_MINIRPC_UNION:
	            Log(LOG_ERR, "XdrProcWrap doesn't support Union now!");
	            bResult = False;
	            break;
	        case PARAM_MINIRPC_CHAR:
	            bResult = pFunc_minirpc(pxdr, pelement);
	            break;
	        case PARAM_MINIRPC_UCHAR:
	            bResult = pFunc_minirpc(pxdr, pelement);
	            break;
	        case PARAM_MINIRPC_VECTOR:
	            Log(LOG_ERR, "XdrProcWrap doesn't support Vector now!");
	            bResult = False;
	            break;
	        case PARAM_MINIRPC_FLOAT:
	            bResult = pFunc_minirpc(pxdr, pelement);
	            break;
	        case PARAM_MINIRPC_DOUBLE:
	            Log(LOG_ERR, "XdrProcWrap doesn't support Double now!");
	            bResult = False;
	            break;
	        case PARAM_MINIRPC_REFERENCE:
	            Log(LOG_ERR, "XdrProcWrap doesn't support Reference now!");
	            bResult = False;
	            break;
	        case PARAM_MINIRPC_POINTER:
	            Log(LOG_ERR, "XdrProcWrap doesn't support Pointer now!");
	            bResult = False;
	            break;
	        case PARAM_MINIRPC_WRAPSTRING:
	            bResult = pFunc_minirpc(pxdr, &pelement);
	            break;
	        case PARAM_MINIRPC_STRARRAY:
	            Log(LOG_ERR, "XdrProcWrap doesn't support STArray now!");
	            bResult = False;
	            break;
	        default:
	            //Log(LOG_DEBUG,"XdrProcWrap, Will execut XDR proc:%d\n",paramId);
	            bResult = pFunc_minirpc(pxdr, pelement, elementsize);
	            break;
	    }
    return bResult;
}

/*Format the arguments when calling service
*
* b31                                                            b0
* --------------------------------------
* |                        FuncID                             |
* --------------------------------------
* |       rid                 |      Reserved              |
* --------------------------------------
* |    arg length         | | arg type | arg index |
* --------------------------------------
* |    paramId                |      Reserved               |
* --------------------------------------
* |                         data                                |
* --------------------------------------
* |    arg length         | | arg type | arg index |
* --------------------------------------
* |    paramId                |      Reserved               |
* --------------------------------------
* |                         data                                |
* --------------------------------------
* |                         ......                                |
* --------------------------------------
*/
Int32  FormatCallerArgsEncode(Uint8 *buf, Int32 bufsize, Uint32 funcid, Int32 rid, Int32 xflag, Param *args[])
{
    XDR xdrs;
    XDR *pxdr = &xdrs;
    Uint32 ui = 0;
    Uint32 xflagvalue = 0;
    Int32 i = 0;

	
    XDR_Create(pxdr, (Int8 *)buf, bufsize, XDR_ENCODE);

    if (xflag)
    {
        xflagvalue = xflagmask;
    }
    else
    {
        xflagvalue = 0;
    }

    if (XDR_PUTBYTES(pxdr, &packet_caller_mask, 4) == False)
    {
        return RPC_ARGS_ERR;
    }

    //Log(LOG_DEBUG,"FormatCallerArgsEncode, XDR pos I:%d\n",XDR_GETPOS(pxdr));

    if (XDR_Uint32(pxdr, &funcid) == False)
    {
        return RPC_ARGS_ERR;
    }

    //Log(LOG_DEBUG,"FormatCallerArgsEncode, XDR pos II:%d, funcid:0x%x\n",XDR_GETPOS(pxdr), funcid);

    ui = (rid << 16) | xflagvalue;
    if (XDR_Uint32(pxdr, &ui) == False)
    {
        return RPC_ARGS_ERR;
    }

    //Log(LOG_DEBUG,"FormatCallerArgsEncode, XDR pos III:%d, ui:0x%x\n",XDR_GETPOS(pxdr), ui);
    /*format the Params*/
    for (i = 0; i < 8; i++)
    {
        if (!args[i])
        {
            continue;
        }
        if (!args[i]->len || !args[i]->pData)
        {
            continue;
        }

		Log(LOG_DEBUG,"ARG[%d]->len:%d type:%d index:%d\n",i,(args[i]->len),(args[i]->type),(i + 1));

		ui = ((args[i]->len) << 16) | (((args[i]->type) << 8) & argtypemask) | ((i + 1) & argindexmask);
        if (XDR_Uint32(pxdr, &ui) == False)
        {
            return RPC_ARGS_ERR;
        }
        Log(LOG_DEBUG,"FormatCallerArgsEncode, IV ARG[%d], ui:0x%x, XDR pos:%d \n", i, ui, XDR_GETPOS(pxdr));
        ui = (args[i]->paramId) << 16;
        if (XDR_Long(pxdr, &ui) == False)
        {
            return RPC_ARGS_ERR;
        }
        //Log(LOG_DEBUG,"FormatCallerArgsEncode, V paramId, ARG[%d], ui:0x%x \n", i, ui, XDR_GETPOS(pxdr));

        /*if ( XDR_PUTBYTES(pxdr, args[i]->pData, args[i]->len) == False) {
            return RPC_ARGS_ERR;
        }*/
        if (XdrProcWrap(pxdr, buf, bufsize, args[i]->pData, args[i]->len, XDR_ENCODE, args[i]->paramId) == False)
        {
            Log(LOG_ERR, "FormatCallerArgsEncode VI, args[%d] call xdrproc failed!\n", i);
            return RPC_ARGS_ERR;
        }

    }

    /*free XDR*/
    //XDR_DESTROY(pxdr);
    //Log(LOG_DEBUG,"FormatCallerArgsEncode, encodedf buf[]:0x%x,0x%x,0x%x,0x%x\n",buf[0],buf[1],buf[2],buf[3]);

    /*For debug usage*/
    /*for(i=0;i<XDR_GETPOS(pxdr);i++){
        ui = buf[i];
        Log(LOG_DEBUG,"Encode II,[%d]:0x%x\n",i,ui);
    }*/

    return (Int32)(XDR_GETPOS(pxdr));

}

/*Format the arguments when returning from service
*
* b31                                                            b0
* --------------------------------------
* |                        FuncID                             |
* --------------------------------------
* |       rid                 | | arg type |     status  |
* --------------------------------------
* |                    Return value                         |
* --------------------------------------
* |    arg length         | | arg type | arg index |
* --------------------------------------
* |    paramId                |      Reserved               |
* --------------------------------------
* |                         data                                |
* --------------------------------------
* |    arg length         | | arg type | arg index |
* --------------------------------------
* |    paramId                |      Reserved               |
* --------------------------------------
* |                         data                                |
* --------------------------------------
* |                         ......                                |
* --------------------------------------
*/
Int32  FormatReturnArgsEncode(Int32 result, Uint8 *buf, Int32 bufsize, Uint32 funcid, Int32 rid, Int32 xflag, Param *args[])
{
    XDR xdrs;
    XDR *pxdr = &xdrs;
    Uint32 ui = 0;
    Uint32 xflagvalue = 0;
    Int32 i=0;

    XDR_Create(pxdr, (Int8 *)buf, bufsize, XDR_ENCODE);

    if (xflag)
    {
        xflagvalue = xflagmask;
    }
    else
    {
        xflagvalue = 0;
    }

    if (XDR_PUTBYTES(pxdr, &packet_return_mask, 4) == False)
    {
        return RPC_ARGS_ERR;
    }

    if (XDR_Uint32(pxdr, &funcid) == False)
    {
        return RPC_ARGS_ERR;
    }
    ui = (rid << 16) | xflagvalue;
    if (XDR_Uint32(pxdr, &ui) == False)
    {
        return RPC_ARGS_ERR;
    }
    if (XDR_Int32(pxdr, &result) == False)
    {
        return RPC_ARGS_ERR;
    }
    /*format the Params*/
    for (i = 0; i < 8; i++)
    {
        if (!args[i])
        {
            continue;
        }
        if (!args[i]->len || !args[i]->pData)
        {
            continue;
        }
        Log(LOG_DEBUG, "FormatReturnArgsEncode args.type%d: args.len:%d\n", args[i]->type, args[i]->len);
        if (args[i]->type == PARAM_INOUT || args[i]->type == PARAM_OUT)
        {
            ui = ((args[i]->len) << 16) | (((args[i]->type) << 8) & argtypemask) | ((i + 1) & argindexmask);
            if (XDR_Uint32(pxdr, &ui) == False)
            {
                return RPC_ARGS_ERR;
            }
            ui = (args[i]->paramId) << 16;
            if (XDR_Long(pxdr, &ui) == False)
            {
                return RPC_ARGS_ERR;
            }
            //Log(LOG_DEBUG,"FormatReturnArgsEncode, paramId, ARG[%d], ui:0x%x \n", i, ui);

            /*if ( XDR_PUTBYTES(pxdr, args[i]->pData, args[i]->len) == False) {
                return RPC_ARGS_ERR;
            }*/
            if (XdrProcWrap(pxdr, buf, bufsize, args[i]->pData, args[i]->len, XDR_ENCODE, args[i]->paramId) == False)
            {
                Log(LOG_ERR, "FormatReturnArgsEncode II, args[%d] call xdrproc failed!\n", i);
                return RPC_ARGS_ERR;
            }
        }
    }

    /*free XDR*/
    //XDR_DESTROY(pxdr);

    return (Int32)(XDR_GETPOS(pxdr));


}

/*Arguments decode for encoded caller arguments
*
* b31                                                            b0
* --------------------------------------
* |                        FuncID                             |
* --------------------------------------
* |       rid                 |      Reserved              |
* --------------------------------------
* |    arg length         | | arg type | arg index |
* --------------------------------------
* |    paramId                |      Reserved               |
* --------------------------------------
* |                         data                                |
* --------------------------------------
* |    arg length         | | arg type | arg index |
* --------------------------------------
* |    paramId                |      Reserved               |
* --------------------------------------
* |                         data                                |
* --------------------------------------
* |                         ......                                |
* --------------------------------------
*/
Int32  FormatCallerArgsDecode(Uint8 *buf, Int32 buflen, TaskDesc *task)
{
    XDR xdrs;
    XDR *pxdr = &xdrs;
    Uint32 ui = 0;
    Size_t  arglen = 0;
    Long  argtype = 0;
    Uint32  argindex = 0;
    Uint8  *pData = NULL;
    char  packet_mask[5]={0};
    Int32 i = 0;
    Long paramId = 0;

    if (!buf || !buflen || !task)
    {
        return RPC_ARGS_ERR;
    }

    /*For debug usage*/
    /*for(i=0;i<buflen;i++){
        ui = buf[i];
        Log(LOG_DEBUG,"Decode ,[%d]:0x%x\n",i,ui);
    }*/

    /*!!Doesn't allow to touch task->used */
    task->func = NULL;
    memset(&task->param1, 0, PARAM1_LEN);
    memset(&task->param2, 0, PARAM2_LEN);
    memset(&task->param3, 0, PARAM3_LEN);
    memset(&task->param4, 0, PARAM4_LEN);
    memset(&task->param5, 0, PARAM5_LEN);
    memset(&task->param6, 0, PARAM6_LEN);
    memset(&task->param7, 0, PARAM7_LEN);
    memset(&task->param8, 0, PARAM8_LEN);

    XDR_Create(pxdr, (Int8 *)buf, buflen, XDR_DECODE);

    if (XDR_GETBYTES(pxdr, &packet_mask, 4) == False)
    {
        return RPC_ARGS_ERR;
    }

    if (XDR_Uint32(pxdr, &task->funcid) == False)
    {
        return RPC_ARGS_ERR;
    }
    if (XDR_Uint32(pxdr, &ui) == False)
    {
        return RPC_ARGS_ERR;
    }
    task->rid = ui >> 16;
    task->xflag = (ui & xflagmask) >> 15;

    //Log(LOG_DEBUG,"FormatCallerArgsDecode, funcid:0x%x, ui:0x%x\n", task->funcid, ui);

    /*format the Params*/
    for (i = 0; i < 8; i++)
    {
        if (XDR_Uint32(pxdr, &ui) == False)
        {
            Log(LOG_DEBUG, "[FormatCallerArgsDecode] Complete Decode I, i:%d, rid:%d, xflag:%d\n", i, task->rid, task->xflag);
            return  RPC_SUCCESS_VALUE;
        }
        if (ui == 0)
        {
            Log(LOG_DEBUG, "[FormatCallerArgsDecode] Complete Decode II, i:%d\n", i);
            return  RPC_SUCCESS_VALUE;
        }
        arglen = ui >> 16;
        argtype = (ui & argtypemask) >> 8;
        argindex = ui & argindexmask;

        Log(LOG_DEBUG, "FormatCallerArgsDecode, ARG[%d], ui:0x%x\n", i, ui);

        if (arglen == 0 || argindex < 1 || argindex > 8)
        {
            Log(LOG_ERR, "[FormatCallerArgsDecode] argument attribute invalid! arglen:%d, argtype:%d, argindex:%d\n", (Int32)arglen, (Int32)argtype, 
            (Int32)argindex);
            return  RPC_ARGS_ERR;
        }

        if (XDR_Long(pxdr, &ui) == False)
        {
            Log(LOG_ERR, "[FormatCallerArgsDecode] paramId xdr decode failed!\n");
            return RPC_ARGS_ERR;
        }
        paramId = ui >> 16;
        Log(LOG_DEBUG, "[FormatCallerArgsDecode], paramId:%d ARG[%d]\n", paramId,  i);

        switch (argindex)
        {
            case 1:
                pData = task->param1;
                break;
            case 2:
                pData = task->param2;
                break;
            case 3:
                pData = task->param3;
                break;
            case 4:
                pData = task->param4;
                break;
            case 5:
                pData = task->param5;
                break;
            case 6:
                pData = task->param6;
                break;
            case 7:
                pData = task->param7;
                break;
            case 8:
                pData = task->param8;
                break;
            default:
                return  RPC_ARGS_ERR;

        }
        memcpy(pData, &argtype, sizeof(Long));
        memcpy(pData + sizeof(Long), &paramId, sizeof(Long));
        memcpy(pData + sizeof(Long) + sizeof(Long), &arglen, sizeof(Size_t));
        /*if ( XDR_GETBYTES(pxdr, pData+sizeof(Long)+sizeof(Size_t), arglen) == False)
        {
            Log(LOG_ERR, "[FormatCallerArgsDecode] XDR_GETBYTES failed!!\n");
            return RPC_ARGS_ERR;
        }*/
        if (XdrProcWrap(pxdr, buf, buflen, pData + sizeof(Long) + sizeof(Long) + sizeof(Size_t), arglen, XDR_DECODE, paramId) == False)
        {
            Log(LOG_ERR, "[FormatCallerArgsDecode], args[%d] call xdrproc failed!\n", i);
            return RPC_ARGS_ERR;
        }
    }

    return  0;
}

/*Arguments decode for encoded return arguments
*
* b31                                                            b0
* --------------------------------------
* |                        FuncID                             |
* --------------------------------------
* |       rid                 | | arg type |     status  |
* --------------------------------------
* |                    Return value                         |
* --------------------------------------
* |    arg length         | | arg type | arg index |
* --------------------------------------
* |    paramId                |      Reserved               |
* --------------------------------------
* |                         data                                |
* --------------------------------------
* |    arg length         | | arg type | arg index |
* --------------------------------------
* |    paramId                |      Reserved               |
* --------------------------------------
* |                         data                                |
* --------------------------------------
* |                         ......                                |
* --------------------------------------
*/
Int32  FormatReturnArgsDecode(Uint8 *buf, Int32 buflen,  Uint32 *rid, Int32 *result, Int32 *xflag, Param *args[])
{
    XDR xdrs;
    XDR *pxdr = &xdrs;
    Uint32 ui = 0;
    Uint32 xflagvalue = 0;
    Size_t    arglen = 0;
    Long    argtype = 0;
    Uint32  argindex = 0;
    Uint32  funcid = 0;
    Param *parg = NULL;
    char  packet_mask[5] = {0};
    Int32 i = 0;
    Long paramId = 0;

    Log(LOG_DEBUG, "~~~FormatReturnArgsDecode Start ~~~\n");

    if (!buf || !buflen || !rid || !result || !xflag)
    {
        Log(LOG_ERR, "[%s] Error A.\n",__func__);
        return RPC_ARGS_ERR;
    }

    XDR_Create(pxdr, (Int8 *)buf, buflen, XDR_DECODE);

    if (XDR_GETBYTES(pxdr, &packet_mask, 4) == False)
    {
        Log(LOG_ERR, "[%s] Error B.\n",__func__);
        return RPC_ARGS_ERR;
    }

    if (XDR_Uint32(pxdr, &funcid) == False)
    {
        Log(LOG_ERR, "[%s] Error C.\n",__func__);
        return RPC_ARGS_ERR;
    }
    if (XDR_Uint32(pxdr, &ui) == False)
    {
        Log(LOG_ERR, "[%s] Error D.\n",__func__);
        return RPC_ARGS_ERR;
    }
    *rid = ui >> 16;
    xflagvalue = (ui & xflagmask) >> 15;
    *xflag = xflagvalue;

    if (XDR_Uint32(pxdr, result) == False)
    {
        Log(LOG_ERR, "[%s] Error E.\n",__func__);
        return RPC_ARGS_ERR;
    }

    if (xflagvalue)
    {
        Log(LOG_DEBUG, "[FormatReturnArgsDecode] It's a nonsynchronous call return packate.\n");
        return  RPC_SUCCESS_VALUE;
    }

    /*format the Params*/
    for (i = 0; i < 8; i++)
    {
        if (XDR_Uint32(pxdr, &ui) == False)
        {
            Log(LOG_DEBUG, "[FormatReturnArgsDecode] Complete Decode I, i:%d\n", i);
            return  RPC_SUCCESS_VALUE;
        }
        if (ui == 0)
        {
            Log(LOG_DEBUG, "[FormatReturnArgsDecode] Complete Decode II, i:%d\n", i);
            return  RPC_SUCCESS_VALUE;
        }
        arglen = ui >> 16;
        argtype = (ui & argtypemask) >> 8;
        argindex = ui & argindexmask;

        if (arglen == 0 || argindex < 1 || argindex > 8)
        {
            Log(LOG_ERR, "[FormatReturnArgsDecode] argument attribute invalid! arglen:%d, argtype:%d, argindex:%d\n", (Int32)arglen, (Int32)argtype,
            (Int32)argindex);
            return  RPC_ARGS_ERR;
        }

        if (arglen > ARG1_LEN)
        {
            Log(LOG_ERR, "[FormatReturnArgsDecode] arglen:%d is abnormal!\n", arglen);
            return  RPC_ARGS_ERR;
        }

        if (XDR_Long(pxdr, &ui) == False)
        {
            Log(LOG_ERR, "[FormatReturnArgsDecode] paramId xdr decode failed!\n");
            return RPC_ARGS_ERR;
        }
        paramId = ui >> 16;
        Log(LOG_DEBUG, "[FormatReturnArgsDecode], paramId:%d ARG[%d]\n", paramId,  i);

        parg = args[argindex - 1];

        if (!parg)
        {
            Log(LOG_ERR, "[FormatReturnArgsDecode] Fatal Error!! the args[%d] is NULL!! argtype:%d, arglen:%d, rid:%d\n", argindex - 1, argtype, arglen, *rid);
            return RPC_ARGS_ERR;
        }

        /*parg->type = argtype;
        parg->len = arglen;
        parg->paramId = paramId;
        if ( XDR_GETBYTES(pxdr, parg->pData, arglen) == False)
        {
            Log(LOG_ERR, "[FormatReturnArgsDecode] XDR_GETBYTES failed!!\n");
            return RPC_ARGS_ERR;
        }*/

        if (XdrProcWrap(pxdr, buf, buflen, parg->pData, arglen, XDR_DECODE, paramId) == False)
        {
            Log(LOG_ERR, "[FormatReturnArgsDecode], args[%d] call xdrproc failed!\n", i);
            return RPC_ARGS_ERR;
        }


    }

    Log(LOG_DEBUG, "~~~FormatReturnArgsDecode End ~~~\n");
    return  RPC_SUCCESS_VALUE;
}


/*
* Check the packet type if caller encoded or return packet.
* Return: 0->Caller Encoded Packet, 1->Return result packet, (<0)->Invalid packet
*/
Int32  FormatCheckPacketType(Uint8 *buf, Int32 buflen, Uint32 *rid)
{
    XDR xdrs;
    XDR *pxdr = &xdrs;
    char  packet_mask[5] = {0};
    Uint32 funcid = 0;
    Uint32 ui = 0;

    if (!buf || !rid)
    {
        if (!buf)
        {
            Log(LOG_DEBUG, "FormatCheckPacketType arguments error, buf is NULL!\n");
        }
        else
        {
            Log(LOG_DEBUG, "FormatCheckPacketType arguments error, rid is NULL!\n");
        }
        return  RPC_ARGS_ERR;
    }

    Log(LOG_DEBUG,"FormatCheckPacketType, before XDR_DECODE, buf:0x%08x buflen=%d buf[]:0x%x,0x%x,0x%x,0x%x\n",(Int8 *)buf, buflen, buf[0],buf[1],buf[2],buf[3]);
    memset(packet_mask, 0, 5);

    XDR_Create(pxdr, (Int8 *)buf, buflen, XDR_DECODE);

    if (XDR_GETBYTES(pxdr, &packet_mask, 4) == False)
    {
        return RPC_ARGS_ERR;
    }
    Log(LOG_DEBUG,"FormatCheckPacketType, XDR_DECODE I, packet_mask:0x%x,0x%x,0x%x,0x%x 0x%x\n",packet_mask[0],packet_mask[1],packet_mask[2],packet_mask[3],packet_mask[4]);
    if (XDR_Uint32(pxdr, &funcid) == False)
    {
        return RPC_ARGS_ERR;
    }
	Log(LOG_DEBUG,"FormatCheckPacketType, XDR_DECODE II, funcid:0x%x\n",funcid);
    if (XDR_Uint32(pxdr, &ui) == False)
    {
        return RPC_ARGS_ERR;
    }
    *rid = ui >> 16;

    Log(LOG_DEBUG, "[%s] rid:%d  packet_mask:%s\n",__func__, *rid, packet_mask);
    Log(LOG_DEBUG,"FormatCheckPacketType, buf[]:0x%x,0x%x,0x%x,0x%x, packet_mask:0x%x,0x%x,0x%x,0x%x\n",buf[0],buf[1],buf[2],buf[3], packet_mask[0],
     packet_mask[1],packet_mask[2],packet_mask[3]);
    if (!strcmp(packet_mask, packet_caller_mask))
    {
        Log(LOG_DEBUG, "[%s] packet_mask = packet_caller_mask!\n",__func__);
        return    0;
    }
    else if (!strcmp(packet_mask, packet_return_mask))
    {
        Log(LOG_DEBUG, "[%s] packet_mask = packet_return_mask!\n",__func__);
        return    1;
    }
    else
    {
        return    RPC_ARGS_ERR;
    }

}


