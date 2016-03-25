/*
 * Copyright (c) 2010, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the <ORGANIZATION> nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */



#include "openmcapi.h"

#if defined(__ALI_TDS__)
#include <pr.h>
#elif defined(__ALI_LINUX__) || defined(__ALI_LINUX_KERNEL__)
#include "pr.h"
#endif

extern mcapi_endpoint_t     MCAPI_CTRL_RX_Endp;
extern mcapi_endpoint_t     MCAPI_CTRL_TX_Endp;
extern MCAPI_BUF_QUEUE      MCAPI_RX_Queue[MCAPI_PRIO_COUNT];

static void mcapi_connect_endpoints(MCAPI_GLOBAL_DATA *, unsigned char *,
                                    mcapi_status_t *);
static void mcapi_setup_connection(MCAPI_GLOBAL_DATA *, MCAPI_ENDPOINT *,
                                   unsigned char *, mcapi_status_t *, mcapi_uint16_t);
static void send_connect_response(unsigned char *, mcapi_status_t);

/*Added by tony*/
//#define __CTRL_MSG_DEBUG__  

#if defined(__ALI_TDS__)
	#define cntrl_printf libc_printf
#else
	#define cntrl_printf printk
#endif

extern MCAPI_COND_STRUCT       mcapi_ctrl_msg_cond;
extern MCAPI_BUF_QUEUE         ctrl_msg_rx_queue;
#define  MAX_CTRL_BUFF  64
MCAPI_BUFFER  g_ctrlmsgbuf[MAX_CTRL_BUFF];
MCAPI_MUTEX      MCAPI_MsgMutex; /*Added by tony*/
MCAPI_MUTEX      MCAPI_MsgDataMutex; /*Added by tony*/
int g_mcapi_response = 0;
int g_mcapi_response_endpoint = 0;
int g_rx_open = 0;
int g_tx_open = 0;
int g_rx_open_ack = 0;
int g_tx_open_ack = 0;
int g_total_cntrlmsg = 0;
int g_cntr_msg_pos = 0; /*current cntrl index*/
int g_cntr_msg_idx = 0;  /*valid max cntrl msg*/
int g_hsend_get_endp_response = 0;
extern void shm_free_buffer(MCAPI_BUFFER* buff);

MCAPI_THREAD_ENTRY(mcapi_process_ctrl_msg)
{
    mcapi_status_t          mcapi_status;
    unsigned char           buffer[MCAPI_CONTROL_MSG_LEN];
    size_t                  rx_size;
    int                     idx = 0;
    int                     cmd = 0;

    mcapi_printf("mcapi_ctrl_msg rvd thread entered!\n");
    /* This loop is active while the node is up and running. */
    for (;;)
    {
        /* Compatible all os types to check stop condition. */
        if (PR_ThreadTestCancel())
        {
            break;
        }
       
        /* Wait for data on the endpoint. */
        mcapi_msg_recv(MCAPI_CTRL_RX_Endp, &buffer,
                       MCAPI_CONTROL_MSG_LEN,
                       &rx_size, &mcapi_status);

#if defined(__CTRL_MSG_DEBUG__)
//	cntrl_printf("CTRL_MSG, len:%d,idx:%d\n", rx_size,idx);
#endif
        /* If a control message was received. */
        if ( (mcapi_status == MCAPI_SUCCESS) && (rx_size) )
        {
		/*Check if a valid control msg*/
		cmd = MCAPI_GET16(buffer, MCAPI_PROT_TYPE);
		if(cmd<MCAPI_GETENDP_REQUEST || cmd>MCAPI_CONNECT_FIN){
			mcapi_printf("Received a bad control msg!\n");
			continue;
		}
		g_total_cntrlmsg++;
		
        	/*Put it to queue and notify process thread*/

		MCAPI_Obtain_Mutex(&MCAPI_MsgDataMutex);

        	memcpy(g_ctrlmsgbuf[idx].buf_ptr, buffer, rx_size);
        	g_ctrlmsgbuf[idx].buf_size = rx_size;
#if 0        	
        	mcapi_enqueue(&ctrl_msg_rx_queue, &g_ctrlmsgbuf[idx]);
#endif
					idx++;
					/*Here we donn't consider overwrite issue because this control msg just is used in the initial statup of mcapi*/
					if(idx>=MAX_CTRL_BUFF)
						idx = 0;

		g_cntr_msg_idx = idx;
		MCAPI_Release_Mutex(&MCAPI_MsgDataMutex);

#if 1		
				MCAPI_Obtain_Mutex(&MCAPI_MsgMutex);
					PR_CondVarNotify(&(mcapi_ctrl_msg_cond.mcapi_cond));
				MCAPI_Release_Mutex(&MCAPI_MsgMutex);
#endif
		if(idx==0){
                  mcapi_printf("The cntrl msg processing is too slow! g_total_cntrlmsg:%d, g_cntr_msg_pos:%d, g_cntr_msg_idx:%d\n", g_total_cntrlmsg, g_cntr_msg_pos, g_cntr_msg_idx);
		    PR_uSleep(100000); //100ms
		}	

        }
        else{
		mcapi_printf("Received a NULL control msg, mcapi_status:%d!\n", mcapi_status);
        	PR_uSleep(50000);
        }
    }
}

/*************************************************************************
*
*   FUNCTION
*
*       mcapi_process_ctrl_msg_proc
*
*   DESCRIPTION
*
*       Processes incoming control messages sent from other nodes in
*       the system.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       None.
*
*************************************************************************/
MCAPI_THREAD_ENTRY(mcapi_process_ctrl_msg_proc)
{
    mcapi_status_t          mcapi_status;
    //unsigned char           buffer[MCAPI_CONTROL_MSG_LEN];
    unsigned char           *buffer = NULL;
    mcapi_request_t         *request, mcapi_request;
    size_t                  rx_size;
    MCAPI_GLOBAL_DATA       *node_data;
    mcapi_node_t            node_id;
    mcapi_port_t            port_id;
    MCAPI_ENDPOINT          *endp_ptr;
    mcapi_endpoint_t        endpoint;
    const int               timeout = 100;
    int 										rv;
    MCAPI_BUFFER						*mcapi_buf_ptr = NULL;
    void *pHead = NULL;

    /* This loop is active while the node is up and running. */
    int debugcount = 0;
    int debugcount2 = 0;

    mcapi_printf("Ctrl_msg_proc thread entered!\n");
    for (;;)
    {
        /* Compatible all os types to check stop condition. */
        if (PR_ThreadTestCancel())
        {
            break;
        }
       
        /* Wait for data on the endpoint. */
        /*mcapi_msg_recv(MCAPI_CTRL_RX_Endp, &buffer,
                       MCAPI_CONTROL_MSG_LEN,
                       &rx_size, &mcapi_status);
				*/
	/*Added by tony*/
#if 1
        MCAPI_Obtain_Mutex(&MCAPI_MsgMutex);
				/*Enhanced by tony on 2013/10/31*/
				//rv = PR_CondVarInit(&mcapi_ctrl_msg_cond, &MCAPI_MsgMutex);
				//rv = PR_CondVarWait(&mcapi_ctrl_msg_cond, PR_TIMEOUT_INFINITE);
				#if defined(__ALI_LINUX_KERNEL__)
    					init_completion(&(mcapi_ctrl_msg_cond.mcapi_cond.cv));
				#endif
				rv = PR_CondVarWait(&mcapi_ctrl_msg_cond.mcapi_cond, timeout);
         MCAPI_Release_Mutex(&MCAPI_MsgMutex);
#endif


		              if(debugcount==0){
					mcapi_printf("Cntrl MSG Process thread timeout or waitup, debugcount:%d!\n", debugcount);
		              }
				debugcount++;
				/*get the received data*/
			while (1)
    	{
 #if 0  	
					MCAPI_Obtain_Mutex(&MCAPI_MsgDataMutex);
					pHead = ctrl_msg_rx_queue.head;
					if(!pHead){
						MCAPI_Release_Mutex(&MCAPI_MsgDataMutex);
                                                break;
				
					}
					MCAPI_Release_Mutex(&MCAPI_MsgDataMutex);
#endif
					MCAPI_Obtain_Mutex(&MCAPI_MsgDataMutex);
#if 0
					mcapi_buf_ptr = mcapi_dequeue(&ctrl_msg_rx_queue);
#endif
					if(g_cntr_msg_pos==g_cntr_msg_idx){
						MCAPI_Release_Mutex(&MCAPI_MsgDataMutex);
						break;
					}
					mcapi_buf_ptr = &g_ctrlmsgbuf[g_cntr_msg_pos++];
					buffer = mcapi_buf_ptr->buf_ptr;
					rx_size = mcapi_buf_ptr->buf_size;
					if(g_cntr_msg_pos>=MAX_CTRL_BUFF)
						g_cntr_msg_pos = 0;
					MCAPI_Release_Mutex(&MCAPI_MsgDataMutex);

					mcapi_status = MCAPI_SUCCESS;

					 if(debugcount2==0){
					 	debugcount2 = 1;
                                  		mcapi_printf("Cntrl MSG Process thread timeout or waitup II, debugcount:%d, g_total_cntrlmsg:%d!\n", debugcount, g_total_cntrlmsg);
					}
#if defined(__CTRL_MSG_DEBUG__)
	//cntrl_printf("CTRL_MSG_PROC, len:%d\n", rx_size);
#endif
        /* If a control message was received. */
        if ( (mcapi_status == MCAPI_SUCCESS) && (rx_size) )
        {
//#if defined(__ALI_LINUX__)
            /* Get the lock. */
            //mcapi_lock_node_data();
//#endif
            /* Get a pointer to the global node list. */
            node_data = mcapi_get_node_data();

            /* Determine the type of message. */
            switch (MCAPI_GET16(buffer, MCAPI_PROT_TYPE))
            {
                case MCAPI_GETENDP_REQUEST:
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_GETENDP_REQUEST\n");
#endif
			if(g_hsend_get_endp_response)
				break;
                    /* Extract the target port from the packet. */
                    port_id = MCAPI_GET16(buffer, MCAPI_GETENDP_PORT);

                    /* Get a pointer to the endpoint. */
                    endp_ptr = mcapi_find_local_endpoint(node_data,
                                                         MCAPI_Node_ID,
                                                         port_id);

                    /* If the endpoint was found. */
                    if (endp_ptr)
                    {
                        /* Set the type. */
                        MCAPI_PUT16(buffer, MCAPI_PROT_TYPE,
                                    MCAPI_GETENDP_RESPONSE);

                        /* Set the status. */
                        MCAPI_PUT32(buffer, MCAPI_GETENDP_STATUS, MCAPI_SUCCESS);

                        /* Extract the destination endpoint. */
                        endpoint = MCAPI_GET32(buffer, MCAPI_GETENDP_ENDP);

                        /* Put the target endpoint in the buffer. */
                        MCAPI_PUT32(buffer, MCAPI_GETENDP_ENDP,
                                    mcapi_encode_endpoint(endp_ptr->mcapi_node_id,
                                    endp_ptr->mcapi_port_id));

        		   mcapi_printf("Will send the get_end_p_response!\n");
		          g_hsend_get_endp_response = 1;

												/* Release the lock. by tony*/
           						  //mcapi_unlock_node_data();
           						  
                        /* Send the packet back to the caller. */
                        msg_send(MCAPI_CTRL_TX_Endp, endpoint, buffer,
                                 MCAPI_GET_ENDP_LEN, MCAPI_DEFAULT_PRIO,
                                 &mcapi_request, &mcapi_status, 0xffffffff);
			   if(mcapi_status != MCAPI_SUCCESS){
			   	mcapi_printf("msg_send failed I, status:%d\n",  mcapi_status);
				g_hsend_get_endp_response = 0;//allow send again
			   }
                        /* Get the lock. by tony*/
            						//mcapi_lock_node_data();
                    }
/*Enhanced by tony on 2013/11/07*/		   
 		    else
		    {
			mcapi_printf("Cntrl msg donn't found endp_ptr for get_endp_request!\n");; /*do nothing*/

		    }
#if 0
                    /* The endpoint has not been created. */
                    else
                    {
                        /* Decode the requestor's information. */
                        mcapi_decode_endpoint(MCAPI_GET32(buffer, MCAPI_GETENDP_ENDP),
                                              &node_id, &port_id);

                        /* Ensure another thread on this same remote node
                         * is not already waiting for the endpoint to be
                         * created.
                         */
                        request = node_data->mcapi_foreign_req_queue.flink;

                        /* Check each request. */
                        while (request)
                        {
                            /* If this is an endpoint request for the same endpoint
                             * from the same node, do not add it again since all
                             * tasks waiting for this endpoint will be resumed by
                             * one call.
                             */
                            if ( (request->mcapi_type == MCAPI_REQ_CREATED) &&
                                 (request->mcapi_target_node_id == MCAPI_Node_ID) &&
                                 (request->mcapi_target_port_id ==
                                  MCAPI_GET16(buffer, MCAPI_GETENDP_PORT)) &&
                                 (request->mcapi_requesting_node_id == node_id) )
                            {
                                request->mcapi_pending_count ++;
                                break;
                            }

                            request = request->mcapi_next;
                        }

                        /* If the remote node is not already pending creation
                         * of the target endpoint.
                         */
                        if (!request)
                        {
                            /* Reserve a global request structure. */
                            request = mcapi_get_free_request_struct();

                            /* If a free structure is available. */
                            if (request)
                            {
                                /* Set the type. */
                                request->mcapi_type = MCAPI_REQ_CREATED;

                                /* Set up the request structure. */
                                request->mcapi_target_port_id =
                                    MCAPI_GET16(buffer, MCAPI_GETENDP_PORT);
                                request->mcapi_target_node_id = MCAPI_Node_ID;
                                request->mcapi_requesting_node_id = node_id;
                                request->mcapi_requesting_port_id = port_id;
                                request->mcapi_status = MCAPI_PENDING;
                                request->mcapi_pending_count = 1;

                                /* Add the structure to the wait list. */
                                mcapi_enqueue(&node_data->mcapi_foreign_req_queue, request);
                            }

                            /* Otherwise, send an error to the caller. */
                            else
                            {
                                /* Set an error. */
                                MCAPI_PUT32(buffer, MCAPI_GETENDP_STATUS,
                                            (mcapi_uint32_t)MCAPI_ERR_REQUEST_LIMIT);

                                /* Set the type. */
                                MCAPI_PUT16(buffer, MCAPI_PROT_TYPE,
                                            MCAPI_GETENDP_RESPONSE);

                                /* Extract the destination endpoint. */
                                endpoint = MCAPI_GET32(buffer, MCAPI_GETENDP_ENDP);

                                /* Put the target endpoint in the packet. */
                                MCAPI_PUT32(buffer, MCAPI_GETENDP_ENDP,
                                            mcapi_encode_endpoint(MCAPI_Node_ID, port_id));
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_GETENDP_REQUEST, msg_send error\n");
#endif
																/* Release the lock. by tony*/
           						          //mcapi_unlock_node_data();
                                /* Send the packet back to the caller. */
                                msg_send(MCAPI_CTRL_TX_Endp, endpoint, buffer,
                                         MCAPI_GET_ENDP_LEN, MCAPI_DEFAULT_PRIO,
                                         &mcapi_request, &mcapi_status, 0xffffffff);
                                /* Get the lock. by tony*/
           						          //mcapi_lock_node_data();
                            }
                        }
                    }
#endif

                    break;

                case MCAPI_GETENDP_RESPONSE:

#if defined(__CTRL_MSG_DEBUG__)

        cntrl_printf("CTRL_MSG, MCAPI_GETENDP_RESPONSE\n");
#endif
			if(g_mcapi_response)
				break;
                    /* Extract the status from the packet. */
                    mcapi_status = MCAPI_GET32(buffer, MCAPI_GETENDP_STATUS);


#if defined(__CTRL_MSG_DEBUG__)

        cntrl_printf("CTRL_MSG, MCAPI_GETENDP_RESPONSE, status:%d\n",mcapi_status);
#endif
                    /* Wake the task that requested this data. */
                    mcapi_check_resume(MCAPI_REQ_CREATED,
                                       MCAPI_GET32(buffer, MCAPI_GETENDP_ENDP),
                                       MCAPI_NULL, 0, mcapi_status);

		    g_mcapi_response_endpoint = MCAPI_GET32(buffer, MCAPI_GETENDP_ENDP);
		    g_mcapi_response = 1;

                    break;

                case MCAPI_CANCEL_MSG:

#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_CANCEL_MSG\n");
#endif

                    /* Decode the requestor's information. */
                    mcapi_decode_endpoint(MCAPI_GET32(buffer, MCAPI_GETENDP_ENDP),
                                          &node_id, &port_id);

                    /* Find the request message in the list. */
                    request = node_data->mcapi_foreign_req_queue.flink;

                    /* Check each request. */
                    while (request)
                    {
                        /* If this request matches. */
                        if ( (request->mcapi_type == MCAPI_REQ_CREATED) &&
                             (request->mcapi_target_node_id == MCAPI_Node_ID) &&
                             (request->mcapi_target_port_id ==
                              MCAPI_GET16(buffer, MCAPI_GETENDP_PORT)) &&
                             (request->mcapi_requesting_node_id == node_id) )
                        {
                            /* Decrement the number of tasks pending completion
                             * of this request.
                             */
                            request->mcapi_pending_count --;

                            /* If this was the only task waiting for the request
                             * to complete.
                             */
                            if (request->mcapi_pending_count == 0)
                            {
                                /* Remove the request from the list. */
                                mcapi_remove(&node_data->mcapi_foreign_req_queue, request);
                            }

                            break;
                        }

                        /* Get the next request. */
                        request = request->mcapi_next;
                    }

                    break;

                case MCAPI_CONNECT_REQUEST:
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_CONNECT_REQUEST\n");
#endif
                    /* If the node ID of the send side is the local node,
                     * connect the send side.  The requestor should only
                     * send the connection request to the send side.
                     */
                    if (MCAPI_GET16(buffer, MCAPI_CNCT_RX_NODE) == MCAPI_Node_ID)
                    {
                        /* Release the lock. by tony*/
           						  //mcapi_unlock_node_data();
           						  //mcapi_printf("Will send the connect SYN!\n");
                        /* Send a SYN request to the receive side. */
                        mcapi_connect_endpoints(node_data, buffer, &mcapi_status);
                        
                        /* If the connection is not possible, send an error
                         * to the caller.
                         */
                        if (mcapi_status != MCAPI_SUCCESS)
                        {
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_CONNECT_REQUEST, send_connect_response\n");
#endif                        
												    /* Send the response. */
                            send_connect_response(buffer, mcapi_status);
                            
                        }
                        else
                        	; //mcapi_printf("Send connect SYNC success!\n");
                        
                        /* Get the lock. by tony*/
           						  //mcapi_lock_node_data();	
                    }

                    break;

                case MCAPI_CONNECT_SYN:
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_CONNECT_SYN\n");
#endif
                    /* Extract the node ID and port ID. */
                    node_id = MCAPI_GET16(buffer, MCAPI_CNCT_TX_NODE);
                    port_id = MCAPI_GET16(buffer, MCAPI_CNCT_TX_PORT);

                    /* Get a pointer to the endpoint. */
                    endp_ptr = mcapi_find_local_endpoint(node_data, node_id,
                                                         port_id);

                    /* If the endpoint was found. */
                    if (endp_ptr)
                    {
                        /* Release the lock. by tony*/
           						  //mcapi_unlock_node_data();
                        /* Make the call to set up the connection. */
                        mcapi_setup_connection(node_data, endp_ptr, buffer,
                                               &mcapi_status, MCAPI_TX_SIDE);
                        /* Get the lock. by tony*/
           						  //mcapi_lock_node_data();
                    }

                    /* The endpoint is not valid. */
                    else
                    {
                        mcapi_status = MCAPI_ERR_ENDP_INVALID;
                    }

                    /* Set the type to ACK. */
                    MCAPI_PUT16(buffer, MCAPI_PROT_TYPE, MCAPI_CONNECT_ACK);

                    /* Set the status. */
                    MCAPI_PUT32(buffer, MCAPI_CNCT_STATUS, mcapi_status);

                    /* Send the status response to the transmit node. */
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_CONNECT_SYN, msg_send status:%d\n",mcapi_status);
#endif              
                    /* Release the lock. by tony*/
           					//mcapi_unlock_node_data();      
                    msg_send(MCAPI_CTRL_TX_Endp,
                             mcapi_encode_endpoint(MCAPI_GET16(buffer, MCAPI_CNCT_RX_NODE),
                             MCAPI_RX_CONTROL_PORT), buffer,
                             MCAPI_CONNECT_MSG_LEN, MCAPI_DEFAULT_PRIO,
                             &mcapi_request, &mcapi_status, 0xffffffff);
                    /* Get the lock. by tony*/
           					//mcapi_lock_node_data();

                    /* If the request was successful. */
                    if (mcapi_status == MCAPI_SUCCESS)
                    {
                        /* If the open call has been made from the application,
                         * send the open request to the send side.
                         */
                        if ( (endp_ptr) &&
                             (endp_ptr->mcapi_state & MCAPI_ENDP_TX) )
                        {
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_CONNECT_SYN, mcapi_tx_open\n");
#endif                     
                            /* Release the lock. by tony*/
           					        //mcapi_unlock_node_data();    
                            /* Send the open call to the other side. */
                            mcapi_tx_open(buffer, endp_ptr,
                                          endp_ptr->mcapi_node_id,
                                          endp_ptr->mcapi_port_id,
                                          endp_ptr->mcapi_foreign_node_id,
                                          endp_ptr->mcapi_foreign_port_id,
                                          MCAPI_OPEN_TX,
                                          MCAPI_GET16(buffer, MCAPI_CNCT_CHAN_TYPE),
                                          &mcapi_status);
                           /* Get the lock. by tony*/
           					       //mcapi_lock_node_data();
                        }
                    }

                    break;

                case MCAPI_CONNECT_ACK:
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_CONNECT_ACK\n");
#endif
                    /* Extract the node ID and port ID. */
                    node_id = MCAPI_GET16(buffer, MCAPI_CNCT_RX_NODE);
                    port_id = MCAPI_GET16(buffer, MCAPI_CNCT_RX_PORT);

                    /* Get a pointer to the endpoint. */
                    endp_ptr = mcapi_find_local_endpoint(node_data, node_id,
                                                         port_id);

                    /* Get the status from the packet. */
                    mcapi_status = MCAPI_GET32(buffer, MCAPI_CNCT_STATUS);

                    /* Ensure the endpoint wasn't deleted while waiting for
                     * the ACK from the other side.
                     */
                    if (endp_ptr && (mcapi_status == MCAPI_SUCCESS))
                    {
                    	/* Release the lock. by tony*/
           					  //mcapi_unlock_node_data();
           					  
                        mcapi_setup_connection(node_data, endp_ptr, buffer,
                                               &mcapi_status, MCAPI_RX_SIDE);

	                    /* Send the response to the requestor.  If the endpoint was
	                     * deleted, it's OK to report success to the connection
	                     * requestor since an error will be returned to the
	                     * RX side when it tries to open.
	                     */
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_CONNECT_ACK,send_connect_response, status:%d\n",mcapi_status);
#endif	               
                               
	                    send_connect_response(buffer, mcapi_status);
	                    
                        /* If the open call has been made by the application,
                         * send the open request to the receive side.
                         */
                        if (endp_ptr->mcapi_state & MCAPI_ENDP_RX)
                        {
                            /* Send the open call to the other side. */
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_CONNECT_ACK,mcapi_tx_open\n");
#endif                            
                            
                            mcapi_tx_open(buffer, endp_ptr,
                                          endp_ptr->mcapi_foreign_node_id,
                                          endp_ptr->mcapi_foreign_port_id,
                                          endp_ptr->mcapi_node_id,
                                          endp_ptr->mcapi_port_id,
                                          MCAPI_OPEN_RX,
                                          MCAPI_GET16(buffer, MCAPI_CNCT_CHAN_TYPE),
                                          &mcapi_status);
                            
                        }
                        /* Get the lock. by tony*/
           					    //mcapi_lock_node_data();
                    }

                    /* The other side could not open the connection. */
                    else
                    {
                        /* Clear the "connecting" flag since this connection
                         * could not be made.
                         */
                        endp_ptr->mcapi_state &= ~MCAPI_ENDP_CONNECTING;
                    }

                    break;

                case MCAPI_OPEN_TX:
                	  g_tx_open = 1; //enhance handshake by tony
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_OPEN_TX\n");
#endif
                    mcapi_status = MCAPI_SUCCESS;

                    /* Extract the node ID and port ID. */
                    node_id = MCAPI_GET16(buffer, MCAPI_CNCT_RX_NODE);
                    port_id = MCAPI_GET16(buffer, MCAPI_CNCT_RX_PORT);

                    /* Get a pointer to the endpoint. */
                    endp_ptr = mcapi_find_local_endpoint(node_data, node_id,
                                                         port_id);

                    /* If the endpoint was found. */
                    if (endp_ptr)
                    {
                        /* Set the state to indicate the send side has
                         * opened.
                         */
                        endp_ptr->mcapi_state |= MCAPI_ENDP_TX_ACKED;

                        /* If the receive side has also opened. */
                        if (endp_ptr->mcapi_state & MCAPI_ENDP_RX_ACKED)
                        {
                            /* Indicate that the connection is complete. */
                            endp_ptr->mcapi_state |= MCAPI_ENDP_CONNECTED;

                            /* Check if any tasks are waiting for the receive side
                             * to be opened.
                             */
                            mcapi_check_resume(MCAPI_REQ_RX_OPEN,
                                               endp_ptr->mcapi_endp_handle,
                                               MCAPI_NULL, 0, mcapi_status);
                        }
                    }

                    else
                    {
                        mcapi_status = MCAPI_ERR_ENDP_INVALID;
                    }

                    /* Set the type to TX_ACK. */
                    MCAPI_PUT16(buffer, MCAPI_PROT_TYPE, MCAPI_OPEN_TX_ACK);

                    /* Put the status in the packet. */
                    MCAPI_PUT32(buffer, MCAPI_CNCT_STATUS, mcapi_status);

                    /* Send a status back to the send side so they
                     * know whether the connection should proceed.
                     */
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_OPEN_TX, msg_send, status:%d\n", mcapi_status);
#endif              
                    /* Release the lock. by tony*/
           					//mcapi_unlock_node_data();         
                    msg_send(MCAPI_CTRL_TX_Endp,
                             mcapi_encode_endpoint(MCAPI_GET16(buffer,
                             MCAPI_CNCT_TX_NODE), MCAPI_RX_CONTROL_PORT),
                             buffer, MCAPI_CONNECT_MSG_LEN,
                             MCAPI_DEFAULT_PRIO,
                             &mcapi_request, &mcapi_status, 0xffffffff);
                    /* Get the lock. by tony*/
           					//mcapi_lock_node_data();

                    break;

                case MCAPI_OPEN_RX:
                	 g_rx_open = 1; //enhance handshake by tony
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_OPEN_RX\n");
#endif
                    mcapi_status = MCAPI_SUCCESS;

                    /* Extract the node ID and port ID. */
                    node_id = MCAPI_GET16(buffer, MCAPI_CNCT_TX_NODE);
                    port_id = MCAPI_GET16(buffer, MCAPI_CNCT_TX_PORT);

                    /* Get a pointer to the endpoint. */
                    endp_ptr = mcapi_find_local_endpoint(node_data, node_id,
                                                         port_id);

                    /* If the endpoint was found. */
                    if (endp_ptr)
                    {
                        /* Set the state to indicate the other side has
                         * opened.
                         */
                        endp_ptr->mcapi_state |= MCAPI_ENDP_RX_ACKED;

                        /* If the send side has opened and been ACKed. */
                        if (endp_ptr->mcapi_state & MCAPI_ENDP_TX_ACKED)
                        {
                            /* Indicate that the connection is completed. */
                            endp_ptr->mcapi_state |= MCAPI_ENDP_CONNECTED;

                            /* Check if any tasks are waiting for the send side
                             * to be opened.
                             */
                            mcapi_check_resume(MCAPI_REQ_TX_OPEN,
                                               endp_ptr->mcapi_endp_handle,
                                               MCAPI_NULL, 0, mcapi_status);
                        }
                    }

                    else
                    {
                        mcapi_status = MCAPI_ERR_ENDP_INVALID;
                    }

                    /* Set the type to RX_ACK. */
                    MCAPI_PUT16(buffer, MCAPI_PROT_TYPE, MCAPI_OPEN_RX_ACK);

                    /* Put the status in the packet. */
                    MCAPI_PUT32(buffer, MCAPI_CNCT_STATUS, mcapi_status);

                    /* Send a status back to the receive side so they
                     * know whether the connection should proceed.
                     */
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_OPEN_RX, msg_send, status:%d\n",mcapi_status);
#endif       
                    /* Release the lock. by tony*/
           					//mcapi_unlock_node_data();               
                    msg_send(MCAPI_CTRL_TX_Endp,
                             mcapi_encode_endpoint(MCAPI_GET16(buffer,
                             MCAPI_CNCT_RX_NODE), MCAPI_RX_CONTROL_PORT),
                             buffer, MCAPI_CONNECT_MSG_LEN,
                             MCAPI_DEFAULT_PRIO,
                             &mcapi_request, &mcapi_status, 0xffffffff);
                    /* Get the lock. by tony*/
           					//mcapi_lock_node_data();

                    break;

                case MCAPI_OPEN_RX_ACK:
                	 g_rx_open_ack = 1; //enhance handshake by tony
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_OPEN_RX_ACK\n");
#endif
                    /* Extract the node ID and port ID. */
                    node_id = MCAPI_GET16(buffer, MCAPI_CNCT_RX_NODE);
                    port_id = MCAPI_GET16(buffer, MCAPI_CNCT_RX_PORT);

                    /* Get a pointer to the endpoint. */
                    endp_ptr = mcapi_find_local_endpoint(node_data, node_id,
                                                         port_id);

                    /* If the endpoint was found. */
                    if (endp_ptr)
                    {
                        /* Extract the status from the packet. */
                        mcapi_status = MCAPI_GET32(buffer, MCAPI_CNCT_STATUS);

                        /* If the ACK is successful. */
                        if (mcapi_status == MCAPI_SUCCESS)
                        {
                            /* Set the flag indicating that the RX side is
                             * open.
                             */
                            endp_ptr->mcapi_state |= MCAPI_ENDP_RX_ACKED;

                            /* If both sides have issued successful open calls. */
                            if (endp_ptr->mcapi_state & MCAPI_ENDP_TX_ACKED)
                            {
                                /* Indicate that the connection is connected. */
                                endp_ptr->mcapi_state |= MCAPI_ENDP_CONNECTED;

                                /* Check if any tasks are waiting for the receive side
                                 * to be opened.
                                 */
                                mcapi_check_resume(MCAPI_REQ_RX_OPEN,
                                                   endp_ptr->mcapi_endp_handle,
                                                   MCAPI_NULL, 0, mcapi_status);
                            }
                        }

                        else
                        {
                            /* Clear the flag. */
                            endp_ptr->mcapi_state &= ~MCAPI_ENDP_RX;

                            /* Check if any tasks are waiting for the receive side
                             * to be opened.
                             */
                            mcapi_check_resume(MCAPI_REQ_RX_OPEN,
                                               endp_ptr->mcapi_endp_handle,
                                               MCAPI_NULL, 0, mcapi_status);
                        }
                    }

                    break;

                case MCAPI_OPEN_TX_ACK:
                	  g_tx_open_ack = 1; //enhance handshake by tony
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_OPEN_TX_ACK\n");
#endif
                    /* Extract the status from the packet. */
                    mcapi_status = MCAPI_GET32(buffer, MCAPI_CNCT_STATUS);

                    /* Extract the node ID and port ID. */
                    node_id = MCAPI_GET16(buffer, MCAPI_CNCT_TX_NODE);
                    port_id = MCAPI_GET16(buffer, MCAPI_CNCT_TX_PORT);

                    /* Get a pointer to the endpoint. */
                    endp_ptr =
                        mcapi_find_local_endpoint(node_data, node_id, port_id);

                    /* If the endpoint was found. */
                    if (endp_ptr)
                    {
                        /* If the status of the ACK is success. */
                        if (mcapi_status == MCAPI_SUCCESS)
                        {
                            /* Set the state to indicate that the TX side
                             * is open.
                             */
                            endp_ptr->mcapi_state |= MCAPI_ENDP_TX_ACKED;

                            /* If both sides have issued successful open calls. */
                            if (endp_ptr->mcapi_state & MCAPI_ENDP_RX_ACKED)
                            {
                                /* Indicate that the connection is connected. */
                                endp_ptr->mcapi_state |= MCAPI_ENDP_CONNECTED;

                                /* Check if any tasks are waiting for the send side
                                 * to be opened.
                                 */
                                mcapi_check_resume(MCAPI_REQ_TX_OPEN,
                                                   endp_ptr->mcapi_endp_handle,
                                                   MCAPI_NULL, 0, mcapi_status);
                            }
                        }

                        else
                        {
                            /* Clear the transmit and connecting flags. */
                            endp_ptr->mcapi_state &= ~MCAPI_ENDP_TX;

                            /* Check if any tasks are waiting for the send side
                             * to be opened.
                             */
                            mcapi_check_resume(MCAPI_REQ_TX_OPEN,
                                               endp_ptr->mcapi_endp_handle,
                                               MCAPI_NULL, 0, mcapi_status);
                        }
                    }

                    break;

                case MCAPI_CONNECT_RESPONSE:
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_CONNECT_RESPONSE\n");
#endif
                    /* Get the status. */
                    mcapi_status = MCAPI_GET32(buffer, MCAPI_CNCT_STATUS);

                    /* Get the transmit node. */
                    node_id = MCAPI_GET16(buffer, MCAPI_CNCT_TX_NODE);

                    /* Get the transmit port. */
                    port_id = MCAPI_GET16(buffer, MCAPI_CNCT_TX_PORT);

                    /* Check if any tasks are waiting for this connection
                     * to be created.
                     */
                    mcapi_check_resume(MCAPI_REQ_CONNECTED,
                                       mcapi_encode_endpoint(node_id, port_id),
                                       MCAPI_NULL, 0, mcapi_status);

                    break;

                case MCAPI_CONNECT_FIN:
#if defined(__CTRL_MSG_DEBUG__)
        cntrl_printf("CTRL_MSG, MCAPI_CONNECT_FIN\n");
#endif
                    /* Extract the port ID. */
                    port_id = MCAPI_GET16(buffer, MCAPI_CNCT_FIN_PORT);

                    /* Get a pointer to the endpoint. */
                    endp_ptr = mcapi_find_local_endpoint(node_data, MCAPI_Node_ID,
                                                         port_id);

                    /* If the endpoint was found. */
                    if (endp_ptr)
                    {
                        /* Clear the "connected" flag since the other side
                         * has shutdown the read/write side.
                         */
                        endp_ptr->mcapi_state &= ~MCAPI_ENDP_CONNECTED;
                        endp_ptr->mcapi_state &= ~MCAPI_ENDP_CONNECTING;

                        /* Resume any threads that are suspended on this endpoint
                         * for any reason.
                         */
                        mcapi_check_resume(MCAPI_REQ_CLOSED,
                                           endp_ptr->mcapi_endp_handle, MCAPI_NULL,
                                           0, MGC_MCAPI_ERR_NOT_CONNECTED);

                    }

                    break;

                default:
			  mcapi_printf("Receviced a bad control msg II!\n");
                    break;
            }
//#if defined(__ALI_LINUX__)
            /* Release the lock. */
            //mcapi_unlock_node_data();
//#endif
        }

        /* The application has called mcapi_finalize().  Exit the loop. */
        else
        {
            break;
        }
        
     }//end while 

     /*Added by tony*/
        //MCAPI_Release_Mutex(&MCAPI_MsgMutex);
    }//end for
    mcapi_printf("ctrl msg process thread exit!\n");
     PR_CondVarDestroy(&mcapi_ctrl_msg_cond.mcapi_cond);
    /* Terminate this task. */
    MCAPI_Cleanup_Task();

}

/*************************************************************************
*
*   FUNCTION
*
*       mcapi_connect_endpoints
*
*   DESCRIPTION
*
*       Issues a request to an endpoint to connect the endpoint.
*
*   INPUTS
*
*       *node_data              A pointer to the global node data.
*       *buffer                 A pointer to the buffer containing
*                               the request.  This buffer will be
*                               reused to send the SYN to the receive
*                               endpoint.
*       *status                 A pointer that will be filled in with
*                               the status of the request.
*
*   OUTPUTS
*
*       None.
*
*************************************************************************/
static void mcapi_connect_endpoints(MCAPI_GLOBAL_DATA *node_data,
                                    unsigned char *buffer,
                                    mcapi_status_t *mcapi_status)
{
    MCAPI_ENDPOINT      *endp_ptr;
    mcapi_node_t        local_node_id, foreign_node_id;
    mcapi_port_t        local_port_id;
    mcapi_request_t     request;

    /* Get the local node. */
    local_node_id = MCAPI_GET16(buffer, MCAPI_CNCT_RX_NODE);

    /* Get the local port. */
    local_port_id = MCAPI_GET16(buffer, MCAPI_CNCT_RX_PORT);

    /* Get the foreign node. */
    foreign_node_id = MCAPI_GET16(buffer, MCAPI_CNCT_TX_NODE);

    /* Get a pointer to the endpoint. */
    endp_ptr = mcapi_find_local_endpoint(node_data, local_node_id,
                                         local_port_id);

    /* If the endpoint was found. */
    if (endp_ptr)
    {
        /* If the endpoint is not already connected. */
        if (!(endp_ptr->mcapi_state & MCAPI_ENDP_CONNECTING))
        {
            /* Set the type to connected so another node doesn't
             * try to issue a connection while this connection
             * is in progress.
             */
            endp_ptr->mcapi_state |= MCAPI_ENDP_CONNECTING;

            /* Clear the disconnected status in case it is set. */
            endp_ptr->mcapi_state &= ~MCAPI_ENDP_DISCONNECTED;

            /* Set the packet type. */
            MCAPI_PUT16(buffer, MCAPI_PROT_TYPE, MCAPI_CONNECT_SYN);

            /* Send a message to the foreign node to see if
             * the connection can be made.
             */
            //mcapi_printf("msg send sync\n");
            msg_send(MCAPI_CTRL_TX_Endp,
                     mcapi_encode_endpoint(foreign_node_id,
                     MCAPI_RX_CONTROL_PORT), buffer,
                     MCAPI_CONNECT_MSG_LEN, MCAPI_DEFAULT_PRIO,
                     &request, mcapi_status, 0xffffffff);
        }

        /* This endpoint is already connected. */
        else
        {
             *mcapi_status = MCAPI_ERR_CHAN_CONNECTED;
        }
    }

    /* This endpoint is not a valid endpoint. */
    else
    {
    	  //mcapi_printf("not a valid endpoint\n");
        *mcapi_status = MCAPI_ERR_ENDP_INVALID;
    }

}

/*************************************************************************
*
*   FUNCTION
*
*       mcapi_setup_connection
*
*   DESCRIPTION
*
*       Sets up a connection over an endpoint.
*
*   INPUTS
*
*       *node_data              A pointer to the global node data.
*       *buffer                 A pointer to the buffer containing the
*                               connection SYN.
*       *mcapi_status           A pointer that will be filled in with
*                               the status of the operation.
*       connect_side            The local node's side of the connection:
*                               MCAPI_TX_SIDE or MCAPI_RX_SIDE.
*
*   OUTPUTS
*
*       None.
*
*************************************************************************/
static void mcapi_setup_connection(MCAPI_GLOBAL_DATA *node_data,
                                   MCAPI_ENDPOINT *endp_ptr, unsigned char *buffer,
                                   mcapi_status_t *mcapi_status,
                                   mcapi_uint16_t connect_side)
{
    int                 node_idx;
    MCAPI_NODE          *node_ptr;

/*Ehanced by tony on 2013/12/25*/
   mcapi_lock_node_data();

    /* If the endpoint is not already connected.  Note that the
     * transmit side is set to connected when the initial connect
     * request is made.
     */
    if ( (connect_side == MCAPI_RX_SIDE) ||
         (!(endp_ptr->mcapi_state & MCAPI_ENDP_CONNECTING)) )
    {
        /* Get the index of the local node. */
        node_idx = mcapi_find_node(endp_ptr->mcapi_node_id, node_data);

        if (node_idx != -1)
        {
            /* Get a pointer to the node structure. */
            node_ptr = &node_data->mcapi_node_list[node_idx];

            /* Set up routes between the two endpoints. */
            endp_ptr->mcapi_route =
                mcapi_find_route(endp_ptr->mcapi_node_id, node_ptr);

            /* Ensure the receive side is reachable from this node. */
            if (endp_ptr->mcapi_route)
            {
                /* Ensure the attributes of both endpoints match. */

                /* Set the state of the send endpoint. */
                endp_ptr->mcapi_state |= MCAPI_ENDP_CONNECTING;

                /* Clear the disconnected status in case it is set. */
                endp_ptr->mcapi_state &= ~MCAPI_ENDP_DISCONNECTED;

                /* Get the index of the local node. */
                if (connect_side == MCAPI_TX_SIDE)
                {
                    /* Get the receive node. */
                    endp_ptr->mcapi_foreign_node_id =
                        MCAPI_GET16(buffer, MCAPI_CNCT_RX_NODE);

                    /* Get the receive port. */
                    endp_ptr->mcapi_foreign_port_id =
                        MCAPI_GET16(buffer, MCAPI_CNCT_RX_PORT);

                    /* Set the state indicating that this side is connecting
                     * as a sender.
                     */
                    endp_ptr->mcapi_state |= MCAPI_ENDP_CONNECTING_TX;
                }

                else
                {
                    /* Get the receive node. */
                    endp_ptr->mcapi_foreign_node_id =
                        MCAPI_GET16(buffer, MCAPI_CNCT_TX_NODE);

                    /* Get the receive port. */
                    endp_ptr->mcapi_foreign_port_id =
                        MCAPI_GET16(buffer, MCAPI_CNCT_TX_PORT);

                    /* Set the state indicating that this side is connecting
                     * as a receiver.
                     */
                    endp_ptr->mcapi_state |= MCAPI_ENDP_CONNECTING_RX;
                }

                /* Set the channel type. */
                endp_ptr->mcapi_chan_type =
                    MCAPI_GET16(buffer, MCAPI_CNCT_CHAN_TYPE);

                /* Extract the requestor's node ID and port ID. */
                endp_ptr->mcapi_req_node_id =
                    MCAPI_GET16(buffer, MCAPI_CNCT_REQ_NODE);

                endp_ptr->mcapi_req_port_id =
                    MCAPI_GET16(buffer, MCAPI_CNCT_REQ_PORT);
            }

            /* The node is not reachable from this node. */
            else
            {
                *mcapi_status = MCAPI_ERR_ENDP_INVALID;
            }
        }

        /* The node is not reachable from this node. */
        else
        {
            *mcapi_status = MCAPI_ERR_ENDP_INVALID;
        }
    }

    else
    {
        *mcapi_status = MCAPI_ERR_CHAN_CONNECTED;
    }

    mcapi_unlock_node_data();

}

/*************************************************************************
*
*   FUNCTION
*
*       send_connect_response
*
*   DESCRIPTION
*
*       Sends a response to a connection request.
*
*   INPUTS
*
*       *buffer                 A pointer to the buffer containing the
*                               outgoing response.
*       status                  The status to insert in the response.
*
*   OUTPUTS
*
*       None.
*
*************************************************************************/
static void send_connect_response(unsigned char *buffer, mcapi_status_t status)
{
    mcapi_node_t    node_id;
    mcapi_port_t    port_id;
    mcapi_status_t  mcapi_status;
    mcapi_request_t request;

    /* Get the requestor node. */
    node_id = MCAPI_GET16(buffer, MCAPI_CNCT_REQ_NODE);

    /* Get the requestor port. */
    port_id = MCAPI_GET16(buffer, MCAPI_CNCT_REQ_PORT);

    /* Set the error message in the packet. */
    MCAPI_PUT32(buffer, MCAPI_CNCT_STATUS, status);

    /* Set the type to response. */
    MCAPI_PUT16(buffer, MCAPI_PROT_TYPE, MCAPI_CONNECT_RESPONSE);

    //mcapi_printf("send the connect error response!\n");
    /* Send the error response to the requestor. */
    msg_send(MCAPI_CTRL_TX_Endp,
             mcapi_encode_endpoint(node_id, port_id),
             buffer, MCAPI_CONNECT_MSG_LEN,
             MCAPI_DEFAULT_PRIO,
             &request, &mcapi_status, 0xffffffff);

}

/*************************************************************************
*
*   FUNCTION
*
*       mcapi_tx_response
*
*   DESCRIPTION
*
*       Transmits a response to a foreign node for a pending request.
*
*   INPUTS
*
*       *node_data              A pointer to the global MCAPI node data
*                               structure.
*       *request                The request structure associated with the
*                               foreign node to resume.
*
*   OUTPUTS
*
*       None.
*
*************************************************************************/
void mcapi_tx_response(MCAPI_GLOBAL_DATA *node_data, mcapi_request_t *request)
{
    mcapi_status_t      tx_status;
    mcapi_endpoint_t    remote_endp;
    mcapi_request_t     tx_request;
    unsigned char       buffer[MCAPI_GET_ENDP_LEN];

    /* If the get request was successful, send the response. */
    if (request->mcapi_status != MCAPI_ERR_REQUEST_CANCELLED)
    {
        MCAPI_PUT16(buffer, MCAPI_PROT_TYPE,  MCAPI_GETENDP_RESPONSE);

        /* Set the status. */
        MCAPI_PUT32(buffer, MCAPI_GETENDP_STATUS, request->mcapi_status);
    }

    /* If the get request is being canceled, send a cancel message. */
    else
    {
        MCAPI_PUT16(buffer, MCAPI_PROT_TYPE,  MCAPI_CANCEL_MSG);
    }

    /* Put the target port in the buffer. */
    MCAPI_PUT16(buffer, MCAPI_GETENDP_PORT, request->mcapi_target_port_id);

    remote_endp = mcapi_encode_endpoint(request->mcapi_requesting_node_id,
                                        request->mcapi_requesting_port_id);

    /* Put the target endpoint in the packet. */
    MCAPI_PUT32(buffer, MCAPI_GETENDP_ENDP,
                mcapi_encode_endpoint(MCAPI_Node_ID,
                request->mcapi_target_port_id));

    /* Send the packet back to the caller. */
    msg_send(MCAPI_CTRL_TX_Endp, remote_endp, buffer, MCAPI_GET_ENDP_LEN,
             MCAPI_DEFAULT_PRIO, &tx_request, &tx_status,
             0xffffffff);

}


/*************************************************************************
*
*   FUNCTION
*
*       mcapi_rx_data
*
*   DESCRIPTION
*
*       Process incoming data from driver level interfaces.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       None.
*
*************************************************************************/
void mcapi_rx_data(void)
{
    MCAPI_ENDPOINT      *endp_ptr;
    MCAPI_BUFFER        *mcapi_buf_ptr;
    MCAPI_GLOBAL_DATA   *node_data;
    mcapi_int_t         cookie;
    mcapi_node_t        dest_node;
    mcapi_port_t        dest_port;
    int                 i;

    mcapi_lock_node_data();

    /* Get a pointer to the global node list. */
    node_data = mcapi_get_node_data();

    for (i = 0; i < MCAPI_PRIO_COUNT; i++)
    {
        /* Process all data on the incoming receive queue. */
        while (MCAPI_RX_Queue[i].head)
        {
            /* Disable interrupts - the HISR could add to this queue
             * while we remove from it.
             */
            cookie = MCAPI_Lock_RX_Queue();

            /* Pull the buffer off the queue. */
            mcapi_buf_ptr = mcapi_dequeue(&MCAPI_RX_Queue[i]);

            /* Restore interrupts to the previous level. */
            MCAPI_Unlock_RX_Queue(cookie);

            /* Extract the destination node. */
            dest_node =
                MCAPI_GET16(mcapi_buf_ptr->buf_ptr, MCAPI_DEST_NODE_OFFSET);

            /* Extract the destination port. */
            dest_port =
                MCAPI_GET16(mcapi_buf_ptr->buf_ptr, MCAPI_DEST_PORT_OFFSET);

            /* Decode message header and obtain RX endpoint */
            endp_ptr =
                mcapi_find_local_endpoint(node_data, dest_node, dest_port);

            /* If the packet is for this node. */
            if (endp_ptr)
            {
                /* Enqueue the new buffer onto the receive queue for
                 * the endpoint.
                 */
                mcapi_enqueue(&endp_ptr->mcapi_rx_queue, mcapi_buf_ptr);

                /* Check if any tasks are waiting to receive data on this
                 * endpoint.
                 */
                mcapi_check_resume(MCAPI_REQ_RX_FIN,
                                   endp_ptr->mcapi_endp_handle, endp_ptr,
                                   mcapi_buf_ptr->buf_size - MCAPI_HEADER_LEN,
                                   MCAPI_SUCCESS);
            }

            else
#if (MCAPI_ENABLE_FORWARDING == 1)
            {
                /* Attempt to forward the packet. */
                mcapi_forward(node_data, mcapi_buf_ptr, 
                INDEX_OF_MCAPI_BUFFER(mcapi_buf_ptr), dest_node);
            }
#else
            {
		/*Enhanced by tony on 2013/12/13*/
#if 0
                /* The packet is not destined for this node, and forwarding
                 * capabilities are disabled.
                 */
                ((MCAPI_INTERFACE*)(mcapi_buf_ptr->mcapi_dev_ptr))->
                    mcapi_recover_buffer(mcapi_buf_ptr);
#endif
		shm_free_buffer(mcapi_buf_ptr);
		
            }
#endif
        }
    }

    /* Release the lock. */
    mcapi_unlock_node_data();

}
