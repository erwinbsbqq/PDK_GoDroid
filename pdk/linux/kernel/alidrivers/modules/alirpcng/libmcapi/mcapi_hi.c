/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: mcapi_hi.c
 *
 *  Description: MCAPI lib test.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.01.25      David.L         0.1.000         Initial
 ****************************************************************************/

#if defined(__ALI_TDS__) || defined(__ALI_LINUX__)
#include <stdlib.h>
#elif defined(__ALI_LINUX_KERNEL__)
#include <linux/kernel.h>
#endif

#if defined(__ALI_TDS__)
#include <pr.h>
#elif defined(__ALI_LINUX__) || defined(__ALI_LINUX_KERNEL__)
#include <pr.h>
#endif

#include "mcapi.h"

#define WAIT_TIMEOUT 0xFFFFFFFF

#if defined(__ALI_TDS__) || defined(__ALI_LINUX__)

#define mcapi_assert_success(s) \
	if (s != MCAPI_SUCCESS) { PR_LOG("%s:%d status %d\n", __FILE__, __LINE__, s); abort(); }

#elif defined(__ALI_LINUX_KERNEL__)

#define mcapi_assert_success(s) \
	if (s != MCAPI_SUCCESS) { panic("%s:%d status %d\n", __FILE__, __LINE__, s); }

#endif

const int tx_port = 1000;
const int rx_port = 1001;

mcapi_pktchan_recv_hndl_t	send_handle;
mcapi_pktchan_recv_hndl_t	recv_handle;

static void connect(int local, int remote)
{
	mcapi_endpoint_t local_send_endpoint; 
	mcapi_endpoint_t local_recv_endpoint;
	mcapi_endpoint_t remote_recv_endpoint;
	mcapi_request_t  request;
	mcapi_request_t  send_request;
	mcapi_request_t  recv_request;
	mcapi_status_t   status;
	size_t           size;
	
	PR_LOG("Node %d: Creating tx port %d\n", local, tx_port);
	local_send_endpoint = mcapi_create_endpoint(tx_port, &status);
	mcapi_assert_success(status);

	PR_LOG("Node %d: Creating rx port %d\n", local, rx_port);
	local_recv_endpoint = mcapi_create_endpoint(rx_port, &status);
	mcapi_assert_success(status);

	remote_recv_endpoint = mcapi_get_endpoint(remote, rx_port, &status);
	mcapi_assert_success(status);

	PR_LOG("Node %d: Connecting %d:%d to %d:%d\n",local, local, tx_port,
	       remote, rx_port);
	mcapi_connect_pktchan_i(local_send_endpoint, remote_recv_endpoint,
	                        &request, &status);
	mcapi_assert_success(status);

	mcapi_wait(&request, &size, &status, WAIT_TIMEOUT);	
	mcapi_assert_success(status);

	PR_LOG("Node %d: Connection complete\n", local);

	PR_LOG("Node %d: Opening send endpoint\n", local);
	mcapi_open_pktchan_send_i(&send_handle, local_send_endpoint, &send_request,
							  &status);

	PR_LOG("Node %d: Opening receive endpoint\n", local);
	mcapi_open_pktchan_recv_i(&recv_handle, local_recv_endpoint, &recv_request,
							  &status);

	mcapi_wait(&send_request, &size, &status, WAIT_TIMEOUT);	
	mcapi_assert_success(status);

	mcapi_wait(&recv_request, &size, &status, WAIT_TIMEOUT);	
	mcapi_assert_success(status);
	
	PR_LOG("Node %d: MCAPI negotiation complete! \n", local);
}

void startup(unsigned int local, unsigned int remote)
{
	mcapi_status_t status;
	mcapi_version_t version;

	PR_LOG("Node %d: MCAPI Initialized\n",local);
	mcapi_initialize(local, &version, &status);
	mcapi_assert_success(status);

	connect(local, remote);
}

void demo(unsigned int node, int loop)
{
	char outgoing[16];
	char *incoming = NULL;
	size_t bytes;
	mcapi_status_t status;

	do {
		memset(outgoing, 0, 16);
		sprintf(outgoing, "hi from node %d", node);

		mcapi_pktchan_send(send_handle, outgoing, strlen(outgoing)+1,
			&status);
		mcapi_assert_success(status);

		mcapi_pktchan_recv(recv_handle, (void *)&incoming, &bytes,
			&status);
		if(incoming)
		{
			PR_LOG("received message: %s\n", incoming);
			mcapi_pktchan_free(incoming, &status);
		}
		mcapi_assert_success(status);

		PR_Sleep(1);
	} while (loop);
}

#if defined(__ALI_TDS__) && defined(__MCAPI_TEST__)

#include <types.h>
#include <retcode.h>

void MCAPI_Hi(UINT32 param1,UINT32 param2)
{
	unsigned long local = 1;
	unsigned long remote = 0;
	int loop = 1;

	startup(local, remote);

	demo(local, loop);

}

#elif defined(__ALI_LINUX__)

int main(int argc, char *argv[])
{
	unsigned long local = 0;
	unsigned long remote = 1;
	int loop = 1;

	startup(local, remote);

	demo(local, loop);

	return 0;
}

#elif defined(__ALI_LINUX_KERNEL__) && defined(__MCAPI_TEST__)

void MCAPI_Hi(void *arg)
{
	unsigned long local = 0;
	unsigned long remote = 1;
	int loop = 1;

	startup(local, remote);

	demo(local, loop);

}

#endif

