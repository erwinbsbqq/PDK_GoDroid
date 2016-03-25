/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: ali_rpc_mcapi.c
 *
 *  Description: Ali Remote Prcedure Call driver between main and see CPU.
 *
 *  History:
 *      Date              Author              Version              Comment
 *      ====            ======          =======         =======
 *  1.  2013.03.14     Tony.Zh            0.1.000             Initial
 ****************************************************************************/
#include <ali_rpc_errno.h>
#include <ali_rpc_mcapi.h>

#if defined(CONFIG_ALI_MBX_AS)
/*AS solution usage*/
extern int mbx_init(void);
extern int mbx_handshake(void);
extern int mbx_read_data(void **buffer, unsigned int *len);
extern int mbx_send_data(void *buffer, Uint32 len);
extern int mbx_send_sync(void);

Int32 McapiInit(McapiNode node, mcapi_version_t *version)
{
    Int32 rc;

    rc = mbx_init();

    return  rc;
}

/*MCAPI protocols exit.*/
Int32 McapiDeinit(void)
{
 
    return  RPC_SUCCESS_VALUE;
}

Int32 McapiOpen_Pre(McapiConn *conn, Bool block, Int32 prio)
{
	
	  return  RPC_SUCCESS_VALUE;
}

Int32 McapiOpen(McapiConn *conn, Bool block, Int32 prio)
{
	  mbx_handshake();
	  return  RPC_SUCCESS_VALUE;
}

Int32 McapiClose(McapiConn *conn)
{
    return  RPC_SUCCESS_VALUE;
}

/*This interface sends out a message specified with buffer to the foreign endpoint, returns -1 if failed.*/
Int32 McapiMsgWrite(McapiConn *conn, void *buffer, Size_t size)
{

    return  RPC_SUCCESS_VALUE;
}

/*This interface receives a message from foreign endpoint, the parameter size is the buffer available length, returns -1 if fails,
// else the received message size.*/
Int32 McapiMsgRead(McapiConn *conn, void *buffer, Size_t size)
{
 
    return  RPC_SUCCESS_VALUE;
}

/*This interface sends out the message specified with buffer to the foreign endpoint in Packet Channels, returns -1 if failed.*/
Int32 McapiPktWrite(McapiConn *conn, void *buffer, Size_t size)
{
	  int ret = 0;
	  
    PR_Lock(&conn->mutex);
   
    ret = mbx_send_data(buffer, size);
   
    PR_Unlock(&conn->mutex);
    
    //Log(LOG_ERR,"McapiPktWrite ret:%d\n",ret);
    if(!ret)
    {
      //Log(LOG_ERR,"McapiPktWrite size:%d\n",(Int32)size);
      return  (Int32)size;
    }
    else
    	return 0;

}

/*This interface receives the message in Packet Channels, the parameter size is the received message length. It returns -1 if failed.*/
Int32 McapiPktRead(McapiConn *conn, void **buffer, Size_t *size)
{
    int ret = 0;
    ret = mbx_read_data(buffer, size);
    
    //Log(LOG_ERR,"McapiPktRead ret:%d\n",ret);
    if(ret<0)
    	return -1;
    else
      return  1;

}

/*This interface releases the system buffer, that has been filled in at McapiPktRead, returns -1 if failed*/
Int32 McapiPktFree(void *buffer)
{
    mbx_send_sync();
    return  RPC_SUCCESS_VALUE;
}

/*This interface sends out the scalar value in Scalar Channels, returns -1 if failed.*/
Int32 McapiSclWrite(McapiConn *conn, void *buffer, Size_t size)
{
    /*Doesn't implemented now.*/
    return  RPC_SUCCESS_VALUE;
}


Int32 McapiSclRead(McapiConn *conn, void *buffer, Size_t size)
{
    /*Doesn't implemented now.*/
    return  RPC_SUCCESS_VALUE;
}

Int32 McapiWait(McapiConn *conn, Uint32 timeout)
{

    return  RPC_SUCCESS_VALUE;
}


Int32 McapiAvail(McapiConn *conn)
{

    return  RPC_SUCCESS_VALUE;
}


#else
/*Non AS slution*/

#define MCAPI_TIME_DEBUG   0

/*MCAPI protocols initialization procedure.*/
Int32 McapiInit(McapiNode node, mcapi_version_t *version)
{
    mcapi_status_t status;
    
    if(!version)
    	return RPC_ERROR_VALUE;
    //mcapi_version_t version;
    mcapi_initialize(node, version, &status);

    Log(LOG_DEBUG, "[McapiInit] Mcapi node:%d, version:%d, status:%d\n", node, *version, status);

    return  RPC_SUCCESS_VALUE;
}

/*MCAPI protocols exit.*/
Int32 McapiDeinit(void)
{
    mcapi_status_t status;
    
    mcapi_finalize(&status);

    Log(LOG_DEBUG, "[McapiDeinit] status:%d\n", status);
    return  RPC_SUCCESS_VALUE;
}

Int32 McapiOpen_Pre(McapiConn *conn, Bool block, Int32 prio)
{
    mcapi_status_t   status;
    Ulong size = 0;
    const Ulong waittimeout = 0xffffffff;
    int trycount = 0;

    Log(LOG_ERR, "McapiOpen_Pre entered!\n");
    conn->local_endp = mcapi_create_endpoint(conn->local.port, &status);
    if (conn->conn == MCAPI_PKTCHAN && conn->dir != 0)
    {
        conn->foreign_endp = 0;    /*For pkt channel received*/
    }
    else
    {
        if (conn->conn == MCAPI_MSG && conn->dir)
        {
            conn->foreign_endp = 0;
            Log(LOG_DEBUG, "McapiOpen_Pre MCAPI_MSG endpoint creating completed for receiving! \n");
            return RPC_SUCCESS_VALUE;
        }
        Log(LOG_DEBUG, "McapiOpen_Pre will get remote endpoint, remote.node:%d, remote.port:%d\n", conn->remote.node, conn->remote.port);
        conn->foreign_endp = mcapi_get_endpoint(conn->remote.node, conn->remote.port, &status);
    }
    Log(LOG_ERR, "McapiOpen_Pre completed, status:%d, local.port:%d, remote.port:%d, conn:%d, dir:%d!\n", status, conn->local.port, conn->remote.port, 
    conn->conn, conn->dir);

    if (conn->dir == 0 && conn->conn == MCAPI_PKTCHAN)
    {
        Log(LOG_ERR, "McapiOpen_Pre, connecting remote node:%d, port:%d MCAPI_PKTCHAN\n", conn->remote.node, conn->remote.port);
	for(trycount=0; trycount<50; trycount++)
	{
        	mcapi_connect_pktchan_i(conn->local_endp, conn->foreign_endp,
                                &conn->request, &status);

		if(status!=MCAPI_SUCCESS)
        		Log(LOG_ERR, "McapiOpen_Pre, connect remote failed:%d, will try to connect again, trycount:%d!\n", status, trycount);
		else
			break;
		PR_uSleep(20000);
	}
	
	Log(LOG_ERR, "McapiOpen_Pre connect dir:%d, waiting ...!\n", conn->dir);
	mcapi_wait(&conn->request, &size, &status, waittimeout);
        Log(LOG_ERR, "McapiOpen_Pre, connecting remote completed!\n");
    }
     Log(LOG_ERR, "McapiOpen_Pre Finished, dir:%d!\n", conn->dir);


    return RPC_SUCCESS_VALUE;
}


/*This interface will create the local endpoint, in case of Packet Channels and Scalar Channels it will call the connection process, and 
all the operations will execute on the given blocking status. The prio is the Messages priority, and it has nothing to do with other two manners.*/
Int32 McapiOpen(McapiConn *conn, Bool block, Int32 prio)
{
    mcapi_status_t   status;
    int trycount = 0;
    
    if (conn->conn == MCAPI_PKTCHAN)
    {

      Log(LOG_ERR, "McapiOpen, MCAPI_PKTCHAN openning, dir:%d\n", conn->dir);
      for(trycount=0;trycount<50;trycount++)
      {
        if (conn->dir == 0)
        {

            /*Will create a send connection*/
            mcapi_open_pktchan_send_i(&conn->pkt_tx_handle, conn->local_endp, &conn->request,
                                      &status);
        }
        else
        {
            /*Will create a received connection*/
            mcapi_open_pktchan_recv_i(&conn->pkt_rx_handle, conn->local_endp, &conn->request,
                                      &status);
        }

	if((status!=MCAPI_SUCCESS) && (status!=MCAPI_ERR_CHAN_CONNECTED))
        	Log(LOG_ERR, "McapiOpen, MCAPI_PKTCHAN openning failed! status:%d, will try again, trycount:%d\n", status,trycount);
	else
		break;
        PR_uSleep(20000);
      }
    }
    else
    {
        Log(LOG_ERR, "McapiOpen, Doesn't support SCL Channel!\n");
        return  RPC_MCAPI_ERR;
    }

    Log(LOG_ERR, "McapiOpen finished, dir:%d!\n", conn->dir);
    return  RPC_SUCCESS_VALUE;

}

/*This interface will disconnect the foreign endpoint in case of Packet Channels and Scalar Channels, and delete the local endpoint.*/
Int32 McapiClose(McapiConn *conn)
{
    //TBD...
    return  RPC_SUCCESS_VALUE;
}

/*This interface sends out a message specified with buffer to the foreign endpoint, returns -1 if failed.*/
Int32 McapiMsgWrite(McapiConn *conn, void *buffer, Size_t size)
{
    mcapi_status_t   status;
    Int32 ret = 0;

    if (!conn || !buffer)
    {
        return RPC_MCAPI_ERR;
    }
    PR_Lock(&conn->mutex);
    mcapi_msg_send(conn->local_endp, conn->foreign_endp, buffer, size,
                   1,  &status);
    PR_Unlock(&conn->mutex);

    if (status != MCAPI_SUCCESS)
    {
        ret = 0;
    }
    else
    {
        ret = size;
    }

    return  ret;
}

/*This interface receives a message from foreign endpoint, the parameter size is the buffer available length, returns -1 if fails,
// else the received message size.*/
Int32 McapiMsgRead(McapiConn *conn, void *buffer, Size_t size)
{
    mcapi_status_t   status;
    Size_t received_size = 0;
    
    if (!conn || !buffer)
    {
        return RPC_MCAPI_ERR;
    }

    PR_Lock(&conn->mutex);
    mcapi_msg_recv(conn->local_endp, buffer,
                   size, &received_size,
                   &status);
    PR_Unlock(&conn->mutex);

    return  received_size;
}

/*This interface sends out the message specified with buffer to the foreign endpoint in Packet Channels, returns -1 if failed.*/
Int32 McapiPktWrite(McapiConn *conn, void *buffer, Size_t size)
{
    mcapi_status_t   status;
    Int32 ret = 0;

#if MCAPI_TIME_DEBUG
#if defined(__ALI_LINUX_KERNEL__)
    struct timeval start, end;
    Ulong interval = 0;
#endif
#endif

    if (!conn || !buffer)
    {
        return RPC_MCAPI_ERR;
    }

    //Log(LOG_DEBUG, "McapiPktWrite start\n");
    //PR_Lock(&conn->mutex);
#if MCAPI_TIME_DEBUG
#if defined(__ALI_LINUX_KERNEL__)
    do_gettimeofday(&start);
#endif
#endif
    mcapi_pktchan_send(conn->pkt_tx_handle,
                       buffer, size,
                       &status);
#if MCAPI_TIME_DEBUG
#if defined(__ALI_LINUX_KERNEL__)
    do_gettimeofday(&end);
    interval = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);

    //PR_Unlock(&conn->mutex);
    Log(LOG_ERR, "McapiPktWrite end, status:%d, time:%d\n", status, (Int32)interval);
#endif
#endif
    if (status != MCAPI_SUCCESS)
    {
        Log(LOG_ERR, "McapiPktWrite failed, status:%d\n", status);
        ret = 0;
    }
    else
    {
        ret = size;
    }

    return  ret;

}

/*This interface receives the message in Packet Channels, the parameter size is the received message length. It returns -1 if failed.*/
Int32 McapiPktRead(McapiConn *conn, void **buffer, Size_t *size)
{
    mcapi_status_t   status;
#if MCAPI_TIME_DEBUG
#if defined(__ALI_LINUX_KERNEL__)
    struct timeval start, end;
    Ulong interval = 0;
#endif
#endif

    if (!size || !conn || !buffer)
    {
        return RPC_MCAPI_ERR;
    }

    //PR_Lock(&conn->mutex);
#if MCAPI_TIME_DEBUG
#if defined(__ALI_LINUX_KERNEL__)
    do_gettimeofday(&start);
#endif
#endif
    mcapi_pktchan_recv(conn->pkt_rx_handle,
                       buffer, size,
                       &status);
#if MCAPI_TIME_DEBUG
#if defined(__ALI_LINUX_KERNEL__)
    do_gettimeofday(&end);
    interval = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);

    //PR_Unlock(&conn->mutex);

    Log(LOG_ERR, "McapiPktRead end, status:%d, time:%d\n", status, (Int32)interval);
#endif
#endif
    return  status;

}

/*This interface releases the system buffer, that has been filled in at McapiPktRead, returns -1 if failed*/
Int32 McapiPktFree(void *buffer)
{
    mcapi_status_t status;

    mcapi_pktchan_free(buffer, &status);

    return  status;
}

/*This interface sends out the scalar value in Scalar Channels, returns -1 if failed.*/
Int32 McapiSclWrite(McapiConn *conn, void *buffer, Size_t size)
{
    /*Doesn't implemented now.*/
    return  RPC_SUCCESS_VALUE;
}

/*This interface receives scalar value in Scalar Channels, returns -1 if failed.*/
Int32 McapiSclRead(McapiConn *conn, void *buffer, Size_t size)
{
    /*Doesn't implemented now.*/
    return  RPC_SUCCESS_VALUE;
}

/*This interface is a blocking call that processes all the non-blocking operation to the specified connection. 
//The parameter timeout is the number of milliseconds. It returns -1 if failed.*/
Int32 McapiWait(McapiConn *conn, Uint32 timeout)
{
    mcapi_boolean_t ret;
    Size_t size = 0;
    mcapi_status_t status;

    ret = mcapi_wait(&conn->request, &size,
                     &status,
                     timeout);
    return ret;
}


Int32 McapiAvail(McapiConn *conn)
{
    mcapi_status_t status;
    mcapi_uint_t    len;

    if (conn->conn == MCAPI_MSG)
        len = mcapi_msg_available(conn->local_endp,
                                  &status);
    else if (conn->conn == MCAPI_PKTCHAN)
        len = mcapi_pktchan_available(conn->pkt_rx_handle,
                                      &status);
    else
    {
        Log(LOG_ERR, "[McapiAvail] Doesn't support this connection type:%d\n", conn->conn);
        return  RPC_MCAPI_ERR;
    }

    return  len;
}

#endif

