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

/*************************************************************************
*
*   FUNCTION
*
*       mcapi_packetchan_recv_close_i
*
*   DESCRIPTION
*
*       Non-blocking API routine to close the receive side of a packet
*       channel.  Close calls are required on both the send and receive
*       side to properly close the channel.  All pending packets are
*       discarded from the endpoint.
*
*   INPUTS
*
*       receive_handle          The local receive handle identifier for
*                               the receive side of the connection being
*                               closed.
*       *request                A pointer to memory that will be filled in
*                               with data relevant to the operation, so the
*                               status of the operation can later be checked.
*       *mcapi_status           A pointer to memory that will be filled in
*                               with the status of the call.
*
*   OUTPUTS
*
*       None.
*
*************************************************************************/
void mcapi_packetchan_recv_close_i(mcapi_pktchan_recv_hndl_t receive_handle,
                                   mcapi_request_t *request,
                                   mcapi_status_t *mcapi_status)
{
    MCAPI_GLOBAL_DATA   *node_data;
    mcapi_port_t        port_id;
    mcapi_node_t        node_id;
    MCAPI_ENDPOINT      *endp_ptr;

    /* Ensure the status pointer is valid. */
    if (mcapi_status)
    {
#if defined(__ALI_LINUX__)
        /* Get the lock. */
        mcapi_lock_node_data();
#endif
        /* Get a pointer to the global node list. */
        node_data = mcapi_get_node_data();

        /* Get a pointer to the endpoint. */
        endp_ptr = mcapi_decode_local_endpoint(node_data, &node_id, &port_id,
                                               receive_handle, mcapi_status);

        if (endp_ptr)
        {
            /* Close the receive side of the connection. */
            mcapi_close(endp_ptr, MCAPI_CHAN_PKT_TYPE, request,
                        MCAPI_ENDP_RX, mcapi_status);
        }

        /* The receive handle is invalid. */
        else if (*mcapi_status == MCAPI_ERR_ENDP_INVALID)
        {
            *mcapi_status = MCAPI_ERR_CHAN_INVALID;
        }
#if defined(__ALI_LINUX__)
        /* Release the lock. */
        mcapi_unlock_node_data();
#endif
    }

}
