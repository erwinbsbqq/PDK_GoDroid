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


//#define   MCAPI_SM_DBG_SUPPORT

#include "mcapi.h"
#include "openmcapi.h"
#include "atomic.h"
#include "shm.h"
#include "shm_os.h"

#ifdef MCAPI_SM_DBG_SUPPORT
#include <stdio.h>
#endif


#if defined(__ALI_TDS__)
#include <pr.h>
#elif defined(__ALI_LINUX__) || defined(__ALI_LINUX_KERNEL__)
#include "pr.h"
#endif

#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
#include "../../lld/mbx/shm_comm_hw.h"

/*add for minirpc*/
extern unsigned long __G_MINIRPC_SHARED_MEM_START_ADDR;
MINIRPC_INTERFACE Minirpc_Interface;
MINIRPC_SHM_MGMT_BLOCK*  MINIRPC_SHM_Mgmt_Blk = NULL;
#endif

#define LOCK   1
#define UNLOCK 0

SHM_MGMT_BLOCK*  SHM_Mgmt_Blk = MCAPI_NULL;
SHM_BUFFER*      SHM_Buff_Array_Ptr;
MCAPI_INTERFACE* SHM_Current_Interface_Ptr;


extern MCAPI_BUF_QUEUE MCAPI_RX_Queue[MCAPI_PRIO_COUNT];
extern mcapi_node_t MCAPI_Node_ID;

extern MCAPI_MUTEX      MCAPI_SendMutex; /*Added by tony*/
#define OPTIMIZE_FOR_MAILBOX 1  /*Added by tony*/

/* This spin lock have to depend on the hardware support, so it's very different to different platform
 * by tony on 2013/05/07 */

#if defined(__MIPS32__)  || defined(__ARM_ARCH__)

static unsigned long flags;
static void shm_acquire_lock(shm_lock* plock)
{
#if OPTIMIZE_FOR_MAILBOX
	;
#else 

	const int lockVal = LOCK;
	int retVal;
	int count = 0;
/*
#ifdef __ALI_TDS__
	if(!check_inside_isr())
        osal_interrupt_disable();
#else
	preempt_disable();
    local_irq_save(flags);
#endif
*/
	do {
				retVal = *plock;
		/*Added by tony for avoding the loop wait here!*/
        count++;
        if(count>50000){
            if(count>50050){
                 PR_LOG("mcapi shm_acquire_lock happend fatal error!!!\n"); 
                 break;
            }
            PR_uSleep(1000); /*1ms sleep*/
        }
	} while (retVal==lockVal);
	*plock = lockVal;
/*
#ifdef __ALI_TDS__
	if(!check_inside_isr())
        osal_interrupt_enable();
#else
	local_irq_restore(flags);
    preempt_enable();
#endif	
*/

#endif	
}
static void shm_release_lock(shm_lock* plock)
{
#if OPTIMIZE_FOR_MAILBOX
        ;
#else

/*
#ifdef __ALI_TDS__
	if(!check_inside_isr())
        osal_interrupt_disable();
#else
	preempt_disable();
    local_irq_save(flags);
#endif
*/
	*plock = UNLOCK;
/*
#ifdef __ALI_TDS__
	if(!check_inside_isr())
        osal_interrupt_enable();
#else
	local_irq_restore(flags);
    preempt_enable();
#endif
*/

#endif
}
#else
static void shm_acquire_lock(shm_lock* plock)
{
	const int lockVal = LOCK;
	int retVal;
	int count = 0;

	do {
		retVal = mcapi_xchg(plock, lockVal);
		/*Added by tony for avoding the loop wait here!*/
                count++;
                if(count>50000){
                        if(count>50050){
                                PR_LOG("shm_acquire_lock happend fatal error!!!\n"); 
                                break;
                        }
                        PR_uSleep(1000);
                }
	} while (retVal==lockVal);
}
static void shm_release_lock(shm_lock* plock)
{
	mcapi_mb();

	*plock = UNLOCK;
}

#endif



static mcapi_uint32_t get_first_zero_bit(mcapi_int_t value)
{
#if 1 /* David */
	mcapi_uint32_t idx;
	mcapi_uint32_t tmp32;

	/* Invert value */
	value = ~value;

	/* (~value) & (2's complement of value) */
	value = (value & (-value)) - 1;

	/* log2(value) */

	tmp32 = value - ((value >> 1) & 033333333333)- ((value >> 2) & 011111111111);

	idx = ((tmp32 + (tmp32 >> 3))& 030707070707) % 63;

	/* Obtain index (compiler optimized ) */
	//GET_IDX(idx,value);

	return idx;
#else
	static const mcapi_uint32_t kBitIndex[] = {
	/* 0x0  0x1 0x2 0x3 0x4 0x5 0x6 0x7 0x8 0x9 0xa 0xb 0xc 0xd 0xe 0xf */
	   32,   0,  1,  0,  2,  0,  1,	 0,  3,  0,  1,  0,  2,  0,  1,  0
	};
	mcapi_uint32_t idx = 0;

	value = ~value;
	if (value & 0xff)
	{
		value &= 0xff;
	}
	else if (value & 0xff00)
	{
		idx += 8;
		value = (value >> 8) & 0xff;
	}
	else if (value & 0xff0000)
	{
		idx += 16;
		value = (value >> 16) & 0xff;
	}
	else if (value & 0xff000000)
	{
		idx += 24;
		value = (value >> 24) & 0xff;
	}
	if (!(value & 0x0f))
	{
		idx += 4;
		value >>= 4;
	}
	return idx + kBitIndex[value & 0xf];
#endif /* David */
}

static mcapi_status_t get_sm_buff_index(mcapi_int32_t* p_idx)
{
	mcapi_uint32_t i, tmp32;
	mcapi_status_t status = MCAPI_ERR_TRANSMISSION;
	mcapi_uint32_t bitmask;

	/* Find first available buffer */

	for (i = 0; i < BITMASK_WORD_COUNT; i++)
	{
		tmp32 = get_first_zero_bit(SHM_Mgmt_Blk->shm_buff_mgmt_blk.buff_bit_mask[i]);
		if (tmp32 < BITMASK_WORD_SIZE)
		{
			/* Calculate absolute index of the available buffer */
			*p_idx = tmp32 + (i * BITMASK_WORD_SIZE);

			/* Mark the buffer taken */
			/*Changed by tony*/
			bitmask = 1 << tmp32;
			while(!(SHM_Mgmt_Blk->shm_buff_mgmt_blk.buff_bit_mask[i] & bitmask)){
				SHM_Mgmt_Blk->shm_buff_mgmt_blk.buff_bit_mask[i] |= 1 << tmp32;
			}
			status = MCAPI_SUCCESS;

			break;
		}
	}
	

	return status;
}

static void clear_sm_buff_index(mcapi_uint32_t idx)
{
	mcapi_uint32_t *word;
	mcapi_uint32_t bit_msk_idx = idx/BITMASK_WORD_SIZE;
	mcapi_uint8_t  bit_idx = idx%BITMASK_WORD_SIZE;
	/*Enhanced by tony*/
	mcapi_uint32_t bitmask;

	bitmask = 1 << bit_idx;
	/* Mark the buffer available */
	word = &SHM_Mgmt_Blk->shm_buff_mgmt_blk.buff_bit_mask[bit_msk_idx];
	while((*word) & bitmask){
		//*word ^= 1 << bit_idx;
		*word &= ~bitmask;
	}
}

/*************************************************************************
*
*   FUNCTION
*
*       shm_get_buffer
*
*   DESCRIPTION
*
*       Obtain a Shared memory driver buffer.
*
*   INPUTS
*
*       *p_idx				Fill in SHM buffer index.
*
*   OUTPUTS
*
*       MCAPI_BUFFER*       Pointer to allocated MCAPI buffer
*
*************************************************************************/
/* Changed by tony, adding a mutex lock for avoiding conflict between mcapi internal control send and
 *  * mcapi user layer*/
static MCAPI_BUFFER* shm_get_buffer(mcapi_node_t node_id, size_t size,
                                    mcapi_int32_t *p_idx, mcapi_uint32_t priority)
{
	mcapi_uint32_t  idx;
	SHM_BUFFER*     p_sm_buff = MCAPI_NULL;
	MCAPI_BUFFER*   p_mcapi_buff = MCAPI_NULL;
	mcapi_status_t  status = MCAPI_SUCCESS;


	*p_idx = -1;
	/* Acquire lock of SM buffer management block */
	shm_acquire_lock(&SHM_Mgmt_Blk->shm_buff_mgmt_blk.lock);

	/* Check if obtained buff index is less than the buff count */
	/*Optimized by tony, here we donn't need these conditions*/
#if 0
	if ((priority == SHM_PRIO_0) ||
		(SHM_Mgmt_Blk->shm_buff_mgmt_blk.shm_buff_count <= SHM_LOW_PRI_BUF_CONT))
#endif
	
	{
		for (idx = 0; idx < CONFIG_SHM_NR_NODES; idx++)
		{
			if (SHM_Mgmt_Blk->shm_routes[idx].node_id == node_id)
			{
				/* Obtain the index of the first available SM buffer */
				/* Optomized by tony on 2013/06/06*/
				//status = get_sm_buff_index(p_idx);
				*p_idx = node_id;
				status = MCAPI_SUCCESS;
/*
#if defined(__ALI_TDS__)
				libc_printf("***** SEE Got share mem idx:%d\n", *p_idx);
#else
				printk("^^^^^^^ MAIN Got share mem idx:%d\n", *p_idx);
#endif
*/

#ifdef MCAPI_SM_DBG_SUPPORT
				printf("Get buffer - priority  = %d \r\n",priority);
				printf("Get buffer - obtained buffer index  = %d \r\n", *p_idx);
#endif

				if (status == MCAPI_SUCCESS)
				{
					/* Obtain the address of the SM buffer for the index */
					p_sm_buff = (SHM_BUFFER*)OFFSET_TO_ADDRESS(SHM_Mgmt_Blk->shm_buff_mgmt_blk.shm_buff_base_offset + (*p_idx)*(sizeof(SHM_BUFFER)));

					/* Obtain pointer to MCAPI buffer */
					p_mcapi_buff = &p_sm_buff->mcapi_buff;
#if OPTIMIZE_FOR_MAILBOX
					;
#else
					/* increment used buffer count */
					SHM_Mgmt_Blk->shm_buff_mgmt_blk.shm_buff_count++;
#endif
					break;
				}
			}
		}
	}

	/* Release lock of SM buffer management block */
	shm_release_lock(&SHM_Mgmt_Blk->shm_buff_mgmt_blk.lock);


	/* Return a MCAPI buffer to the caller */
	return p_mcapi_buff;
}

/*************************************************************************
*
*   FUNCTION
*
*       shm_free_buffer
*
*   DESCRIPTION
*
*       Free shared memory buffer.
*
*   INPUTS
*
*       SHM_BUFFER*          Pointer to SM buffer
*
*   OUTPUTS
*
*       None
*
*************************************************************************/
/*Enhanced by tony on 2013/11/06*/
//static void shm_free_buffer(MCAPI_BUFFER* buff)
void shm_free_buffer(MCAPI_BUFFER* buff)
{
/*Changed by tony for fix lose pakcet bug, the mailbox sync ack will be move to here to send*/

#if OPTIMIZE_FOR_MAILBOX

	openmcapi_shm_syncack();

#else

	mcapi_uint32_t idx;

	/* Obtain the index of the buffer */
	idx = INDEX_OF_MCAPI_BUFFER(buff);

	shm_acquire_lock(&SHM_Mgmt_Blk->shm_buff_mgmt_blk.lock);

	/* Mark the buffer available */
	clear_sm_buff_index(idx);

	/* Decrement used buffer count */
	/*Enhanced by tony*/
	if(SHM_Mgmt_Blk->shm_buff_mgmt_blk.shm_buff_count > 0)
		SHM_Mgmt_Blk->shm_buff_mgmt_blk.shm_buff_count--;

	shm_release_lock(&SHM_Mgmt_Blk->shm_buff_mgmt_blk.lock);

#ifdef MCAPI_SM_DBG_SUPPORT
	printf("Free buffer - freed index  = %d \r\n",idx);
#endif

	
#endif
}

/*************************************************************************
*
*   FUNCTION
*
*       get_sm_ring_q
*
*   DESCRIPTION
*
*       Obtain the SM packet descriptor ring queue for the node ID
*       requested.
*
*   INPUTS
*
*       mcapi_uint32_t      Destination node ID
*       mcapi_uint32_t*     Pointer to unit ID for the destination node
*
*   OUTPUTS
*
*       SHM_BUFF_DESC_Q*     Pointer to SM ring queue for requested node ID.
*
*************************************************************************/
static SHM_BUFF_DESC_Q* get_sm_ring_q(mcapi_uint32_t node_id,
                                      mcapi_uint32_t *p_unit_id)
{
	int idx;
	mcapi_uint32_t unit_id;
	SHM_BUFF_DESC_Q* p_sm_ring_queue = MCAPI_NULL;

	/* Look up routes for the requested node ID
	 * and obtain the corresponding unit ID and SM ring queue */

	for (idx = 0; idx < CONFIG_SHM_NR_NODES; idx++)
	{
		if (SHM_Mgmt_Blk->shm_routes[idx].node_id == node_id)
		{
			unit_id = SHM_Mgmt_Blk->shm_routes[idx].unit_id;

			/* Load unit ID for the caller */
			*p_unit_id = unit_id;

			/* Obtain pointer to ring queue */
			p_sm_ring_queue = &SHM_Mgmt_Blk->shm_queues[node_id];

			break;
		}
	}

	/* Return pointer to SM ring queue for the unit ID identified */
	return p_sm_ring_queue;
}

/*************************************************************************
*
*   FUNCTION
*
*       enqueue_sm_ring_q
*
*   DESCRIPTION
*
*       Enqueue a transmission request to the SM descriptor ring queue.
*
*   INPUTS
*
*       SHM_BUFF_DESC_Q*     Pointer to the SM packet descriptor queue
*       mcapi_uint16_t      Destination node ID
*       mcapi_int32_t       Index to the SM buffer (payload)
*       size_t              Size of the SM buffer (payload)
*       mcapi_uint8_t       Message type
*
*   OUTPUTS
*
*       mcapi_status_t      status of attempt to enqueue.
*
*************************************************************************/
static mcapi_status_t enqueue_sm_ring_q(SHM_BUFF_DESC_Q *shm_des_q,
                                        mcapi_node_t node_id,
                                        mcapi_int32_t buf_idx,
                                        mcapi_priority_t priority,
                                        size_t buff_size, mcapi_uint8_t type)
{
	mcapi_uint32_t idx;
	mcapi_status_t status = MCAPI_SUCCESS;
	SHM_BUFF_DESC* shm_desc;

	/* Acquire lock of the SM packet descriptor queue */
	shm_acquire_lock(&shm_des_q->lock);

	/* Obtain put index into the queue */
	idx = shm_des_q->put_idx;

	if (shm_des_q->count == SHM_BUFF_DESC_Q_SIZE)
	{
		/* Queue is full fail denqueue operation */
#if defined(__ALI_TDS__)
		libc_printf("++++++enqueue_sm_ring_q, Queue is full@@@@@@\n"); //for debug
#else		
		printk("@@@@@@enqueue_sm_ring_q, Queue is full@@@@@@\n"); //for debug
#endif
		status = MCAPI_ERR_TRANSMISSION;
	}
	else
	{
		/* Load packet descriptor */
		shm_desc = &shm_des_q->pkt_desc_q[idx];
		shm_desc->priority = priority;
		shm_desc->type = type;
		shm_desc->value = buf_idx;

		shm_des_q->put_idx = (shm_des_q->put_idx + 1) % SHM_BUFF_DESC_Q_SIZE;
		shm_des_q->count++;

		/* Enqueue operation successfully completed */
		status = MCAPI_SUCCESS;
	}

	/* Release lock of the SM packet descriptor queue */
	shm_release_lock(&shm_des_q->lock);

	return status;
}


#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
/*Begin:For MiniRPC*/
Int32 minirpc_shm_tx(unsigned int core_id, unsigned int data)
{
	Int32 rc = -1;

	rc = minirpc_shm_notify(core_id, data);
	if(rc)
	{
	   Log(LOG_ERR, "[%s] Err!\n",__func__);
	} 
	
	return rc;
}

unsigned char *minirpc_shm_get_buffer(unsigned int node_id)
{
    Log(LOG_DEBUG, "[%s] node_id:0x%08x \n",__func__, node_id);
	
    if(MAIN_CPU_ID == node_id)
    {
       //if(MINIRPC_SHM_Mgmt_Blk->buffer_array[0].busy_flag == 1)
       //{
          //MINIRPC_SHM_Mgmt_Blk->buffer_array[0].busy_flag = 1;
          return MINIRPC_SHM_Mgmt_Blk->buffer_array[0].minirpc_shm_buff;
       //}
	   //else
	   //{
	      
	      //Log(LOG_ERR, "[%s] error A busy_flag=%d\n",__func__,MINIRPC_SHM_Mgmt_Blk->buffer_array[0].busy_flag);
	   //}
    }
	
	if(SEE_CPU_ID == node_id)
    {
       //if(MINIRPC_SHM_Mgmt_Blk->buffer_array[1].busy_flag == 0)
       //{
          //MINIRPC_SHM_Mgmt_Blk->buffer_array[1].busy_flag = 1;
          return MINIRPC_SHM_Mgmt_Blk->buffer_array[1].minirpc_shm_buff;
       //}
	   //else
	   //{
	       //Log(LOG_ERR, "[%s] error B busy_flag=%d\n",__func__,MINIRPC_SHM_Mgmt_Blk->buffer_array[1].busy_flag);
	   //}
    }

	Log(LOG_ERR, "[%s] error \n",__func__);
	
	return NULL;
}

Int32 minirpc_shm_free_buffer(unsigned char* buff)
{
    int rc = -1;
    Int32 idx = ((unsigned char*)buff - (unsigned char*)MINIRPC_SHM_Mgmt_Blk)/sizeof(minirpc_buffer);

	memset(&(MINIRPC_SHM_Mgmt_Blk->buffer_array[idx]), 0, sizeof(minirpc_buffer));
	
	minirpc_shm_syncack();
	
    return rc;
}


void minirpc_shm_init()
{
     int rc=-1;
	 
     memcpy(Minirpc_Interface.minirpc_int_name,"minirpc",sizeof("minirpc"));
     Minirpc_Interface.minirpc_max_buf_size = MINIRPC_MAX_MBX_LEN;
     Minirpc_Interface.minirpc_get_buffer = minirpc_shm_get_buffer;
     Minirpc_Interface.minirpc_free_buffer = minirpc_shm_free_buffer;
     Minirpc_Interface.minirpc_tx_out = minirpc_shm_tx;

	 rc = minirpc_create_shm_thread();
	 if(rc)
	 {
	    Log(LOG_ERR, "[%s] minirpc create thread error!\n",__func__);
	 }
	 return;
}
/*End:For Minirpc*/
#endif

/*************************************************************************
*
*   FUNCTION
*
*       shm_tx
*
*   DESCRIPTION
*
*       Transmit data using SM driver.
*
*   INPUTS
*
*       buffer_idx         Index to the pass-in parameter buffer.
*
*   OUTPUTS
*
*       mcapi_status_t      Return status of initialization
*
*************************************************************************/
static mcapi_status_t shm_tx(MCAPI_BUFFER *buffer, mcapi_int32_t buffer_idx,
                             size_t buffer_size, mcapi_priority_t priority,
                             struct _mcapi_endpoint *tx_endpoint)
{
	SHM_BUFF_DESC_Q* shm_q;
	mcapi_uint32_t  unit_id;
	mcapi_uint32_t	node_id;
	mcapi_status_t  status = MCAPI_SUCCESS;

#ifdef MCAPI_SM_DBG_SUPPORT
	printf("TX buffer - transmitting buffer playload prototype  = %d \r\n",MCAPI_GET16(buffer->buf_ptr, MCAPI_PROT_TYPE + MCAPI_HEADER_LEN));
	printf("TX buffer - transmitting buffer address  = %p \r\n",buffer);
	printf("TX buffer - transmitting buffer offset   = 0x%x \r\n",ADDRESS_TO_OFFSET(buffer));
	printf("TX buffer - transmitting buffer index    = %d \r\n",buffer_idx);
	printf("TX buffer - transmitting buffer size     = %d \r\n",(int)buffer_size);
	printf("TX buffer - transmitting buffer priority = %d \r\n",priority);
#endif

	/* Obtain SM ring queue for the destination node ID */
	node_id = tx_endpoint->mcapi_foreign_node_id;
	shm_q = get_sm_ring_q(node_id, &unit_id);

	if (shm_q)
	{
	  /*Changed by tony*/
#if 0
		/* Enqueue request to transmit data */
		status = enqueue_sm_ring_q(shm_q, node_id, buffer_idx, priority,
			buffer_size, tx_endpoint->mcapi_chan_type);
#else
		status = MCAPI_SUCCESS;
#endif
/*Enhanced by tony, here we donn't need this resume*/
		status = MCAPI_SUCCESS;
#if 0
		/* Resume Tasks suspensed on TX */
/*		mcapi_check_resume(MCAPI_REQ_TX_FIN, tx_endpoint->mcapi_endp_handle,
						   MCAPI_NULL, (buffer->buf_size - MCAPI_HEADER_LEN), status);
*/
#endif

		/* Start data transmission */
		if (status == MCAPI_SUCCESS)
		{
		   /*Changed by tony for sending buffer idx value to peer*/
			//status = openmcapi_shm_notify(unit_id, node_id);
			status = openmcapi_shm_notify(unit_id, buffer_idx);

#ifdef MCAPI_SM_DBG_SUPPORT
			printf("TX buffer - TX success \r\n");
#endif
		}
	}
	else
	{
		/* TX request to unrecognized node ID */
		status = MCAPI_ERR_NODE_NOTINIT;

#ifdef MCAPI_SM_DBG_SUPPORT
		printf("TX buffer - TX Failed \r\n");
#endif
	}

	return status;

}

/*************************************************************************
*
*   FUNCTION
*
*       shm_finalize
*
*   DESCRIPTION
*
*       Finalization sequence for SM driver.
*
*   INPUTS
*
*       mcapi_node_t    Local node ID.
*
*       SHM_MGMT_BLOCK*  Pointer to SM driver management.
*       structure
*
*   OUTPUTS
*
*       mcapi_status_t          Initialization status.
*
*************************************************************************/
static mcapi_status_t shm_finalize(mcapi_node_t node_id,
                                   SHM_MGMT_BLOCK *SHM_Mgmt_Blk)
{
	int i;
	mcapi_status_t status = MCAPI_ERR_NODE_INITFAILED;

	for (i = 0; i < CONFIG_SHM_NR_NODES; i++)
	{
		if (SHM_Mgmt_Blk->shm_routes[i].node_id == node_id)
		{
			status = MCAPI_SUCCESS;

			SHM_Mgmt_Blk->shm_routes[i].node_id = SHM_INVALID_NODE;

			SHM_Mgmt_Blk->shm_routes[i].unit_id = SHM_INVALID_SCH_UNIT;

			break;
		}
	}

	/* Return finalization status */
	return status;
}

/*************************************************************************
*
*   FUNCTION
*
*       shm_ioctl
*
*   DESCRIPTION
*
*       IOCTL routine for the shared memory driver interface.
*
*   INPUTS
*
*       optname                 The name of the IOCTL option.
*       *option                 A pointer to memory that will be
*                               filled in if this is a GET option
*                               or the new value if this is a SET
*                               option.
*       optlen                  The length of the memory at option.
*
*   OUTPUTS
*
*       MCAPI_SUCCESS           The call was successful.
*       MCAPI_ERR_ATTR_NUM         Unrecognized option.
*       MCAPI_ERR_ATTR_SIZE        The size of option is invalid.
*
*************************************************************************/
static mcapi_status_t shm_ioctl(mcapi_uint_t optname, void *option,
                                size_t optlen)
{
	mcapi_status_t status = MCAPI_SUCCESS;

	switch (optname)
	{
		/* The total number of buffers in the system. */
		case MCAPI_ATTR_NO_BUFFERS:

			/* Ensure the buffer can hold the value. */
			if (optlen >= sizeof(mcapi_uint32_t))
				*(mcapi_uint32_t *)option = SHM_BUFF_COUNT;
			else
				status = MCAPI_ERR_ATTR_SIZE;

			break;

		/* The maximum size of an interface buffer. */
		case MCAPI_ATTR_BUFFER_SIZE:

			/* Ensure the buffer can hold the value. */
			if (optlen >= sizeof(mcapi_uint32_t))
				*(mcapi_uint32_t *)option = MCAPI_MAX_DATA_LEN;
			else
				status = MCAPI_ERR_ATTR_SIZE;

			break;

		/* The number of buffers available for receiving data. */
		case MCAPI_ATTR_RECV_BUFFERS_AVAILABLE:

			/* Ensure the buffer can hold the value. */
			if (optlen >= sizeof(mcapi_uint32_t))
				*(mcapi_uint32_t *)option = SHM_Mgmt_Blk->shm_buff_mgmt_blk.shm_buff_count;
			else
				status = MCAPI_ERR_ATTR_SIZE;

			break;

		/* The number of buffers available for receiving data. */
		case MCAPI_ATTR_NO_PRIORITIES:

			/* Ensure the buffer can hold the value. */
			if (optlen >= sizeof(mcapi_uint32_t))
				*(mcapi_uint32_t *)option = SHM_NUM_PRIORITIES;
			else
				status = MCAPI_ERR_ATTR_SIZE;

			break;

		/* The number of buffers available for receiving data. */
		case MCAPI_FINALIZE_DRIVER:

			/* Finalize OS layer */
			status = openmcapi_shm_os_finalize();

			if (status == MCAPI_SUCCESS)
			{
				/* Finalize SM driver */
				status = shm_finalize(MCAPI_Node_ID, SHM_Mgmt_Blk);
			}

			if (status == MCAPI_SUCCESS)
			{
				/* Unmap SM device */
				openmcapi_shm_unmap((void*)SHM_Mgmt_Blk);
			}

			break;

		default:

			status = MCAPI_ERR_ATTR_NUM;
			break;
	}

	return status;
}

/*************************************************************************
*
*   FUNCTION
*
*       shm_master_node_init
*
*   DESCRIPTION
*
*       Initialize Shared memory driver as the Master node.
*
*   INPUTS
*
*       mcapi_node_t    Local node ID.
*
*       SHM_MGMT_BLOCK*  Pointer to SM driver management.
*       structure
*
*   OUTPUTS
*
*       mcapi_status_t              Initialization status.
*
*************************************************************************/
static mcapi_status_t shm_master_node_init(mcapi_node_t node_id,
                                           SHM_MGMT_BLOCK* SHM_Mgmt_Blk)
{
	int i;
	mcapi_status_t status = MCAPI_SUCCESS;

	/* The current node is the first node executing in the system.
	 * Initialize SM driver as master node. */

	/* Initialize routes and SM buffer queue data structures */
	for (i = 0; i < CONFIG_SHM_NR_NODES; i++)
	{
		/* Initialize routes */
		SHM_Mgmt_Blk->shm_routes[i].node_id = SHM_INVALID_NODE;
		SHM_Mgmt_Blk->shm_routes[i].unit_id = SHM_INVALID_SCH_UNIT;
	}

	/* Obtain the offset of the SM buffer space */
	SHM_Mgmt_Blk->shm_buff_mgmt_blk.shm_buff_base_offset = ADDRESS_TO_OFFSET(SHM_Buff_Array_Ptr);

	/* Initialize all SM buffers */
	for (i = 0; i < SHM_BUFF_COUNT; i++)
	{
		/* Initialize index and offset */
		SHM_Buff_Array_Ptr[i].idx = i;
	}

	/* Make all SM buffers available */
	for (i = 0; i < BITMASK_WORD_COUNT; i++)
	{
		SHM_Mgmt_Blk->shm_buff_mgmt_blk.buff_bit_mask[i] = 0;
	}

	/* Initialize used buff count */

	SHM_Mgmt_Blk->shm_buff_mgmt_blk.shm_buff_count = 0;

	/* Load the route for the current node */
	SHM_Mgmt_Blk->shm_routes[0].node_id = node_id;
	SHM_Mgmt_Blk->shm_routes[0].unit_id = openmcapi_shm_localcoreid();

	/* Load shared memory initialization complete key */
	SHM_Mgmt_Blk->shm_init_field = SHM_INIT_COMPLETE_KEY;

	/* Return master node initialization status */
	return status;
}

/*************************************************************************
*
*   FUNCTION
*
*       shm_slave_node_init
*
*   DESCRIPTION
*
*       Initialization sequence for SM driver for a slave node.
*
*   INPUTS
*
*       mcapi_node_id   Local node ID.
*
*       SHM_MGMT_BLOCK*  Pointer to SM driver management structure.
*
*   OUTPUTS
*
*       mcapi_status_t          Initialization status.
*
*************************************************************************/
static mcapi_status_t shm_slave_node_init(mcapi_node_t node_id,
                                          SHM_MGMT_BLOCK* SHM_Mgmt_Blk)
{
	int i;
	mcapi_status_t status = MCAPI_SUCCESS;

	/* SM driver has already been initialized by the master node */
	/* Perform slave initialization of SM driver */

	/* Make sure the current node has not been initialized */
	for (i = 0; i < CONFIG_SHM_NR_NODES; i++)
	{
		if (SHM_Mgmt_Blk->shm_routes[i].node_id == node_id)
		{
			status = MCAPI_ERR_NODE_INITFAILED;

			break;
		}
	}

	if (status == MCAPI_SUCCESS)
	{
		/* Load the route for the current node */
		for (i = 0; i < CONFIG_SHM_NR_NODES; i++)
		{
			if (SHM_Mgmt_Blk->shm_routes[i].node_id == SHM_INVALID_NODE)
			{
				SHM_Mgmt_Blk->shm_routes[i].node_id = node_id;

				SHM_Mgmt_Blk->shm_routes[i].unit_id = openmcapi_shm_localcoreid();

				break;
			}
		}

	}

	/* Return slave node initialization status */
	return status;
}

/*************************************************************************
*
*   FUNCTION
*
*       openmcapi_shm_init
*
*   DESCRIPTION
*
*       Initialize the Shared Memory driver interface.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       mcapi_status_t  Return status of initialization
*
*************************************************************************/
mcapi_status_t openmcapi_shm_init(mcapi_node_t node_id,
                                  MCAPI_INTERFACE* int_ptr)
{
	mcapi_status_t status = MCAPI_SUCCESS;

	if (node_id >= CONFIG_SHM_NR_NODES)
		return MCAPI_ERR_NODE_INVALID;


	/* Store the name of this interface. */
	memcpy(int_ptr->mcapi_int_name, OPENMCAPI_SHM_NAME, MCAPI_INT_NAME_LEN);

	/* Set the maximum buffer size for incoming / outgoing data. */
	int_ptr->mcapi_max_buf_size = MCAPI_MAX_DATA_LEN;

	/* Set up function pointers for sending data, reserving an outgoing
	 * driver buffer, returning the buffer to the free list, and
	 * issuing ioctl commands.
	 */
	int_ptr->mcapi_tx_output = shm_tx;
	int_ptr->mcapi_get_buffer = shm_get_buffer;
	int_ptr->mcapi_recover_buffer = shm_free_buffer;
	int_ptr->mcapi_ioctl = shm_ioctl;

	/* Obtain Shared memory base address */
	SHM_Mgmt_Blk = openmcapi_shm_map();

	if (SHM_Mgmt_Blk != MCAPI_NULL)
	{
		/* Initialize the base of shared memory buffers  */
		SHM_Buff_Array_Ptr = (SHM_BUFFER*)(((mcapi_ulong_t)SHM_Mgmt_Blk + SHM_4K_ALIGN_SIZE));

		/* Initialize OS specific component */
		openmcapi_shm_os_init();

		/* Obtain SM driver initialization lock */
		shm_acquire_lock(&SHM_Mgmt_Blk->shm_init_lock);

		/* Has another node completed SM driver initialization */
		if (SHM_Mgmt_Blk->shm_init_field != SHM_INIT_COMPLETE_KEY)
		{
			/* Initialize SM driver as the Master node */
			status = shm_master_node_init(node_id, SHM_Mgmt_Blk);
		}
		else
		{
			/* Initialize SM driver as the Slave node */
			status = shm_slave_node_init(node_id, SHM_Mgmt_Blk);
		}

		/* Release SM driver initialization lock */
		shm_release_lock(&SHM_Mgmt_Blk->shm_init_lock);
	}
	else
	{
		status = MCAPI_ERR_GENERAL;
	}

	/* Obtain pointer to the local interface */
	SHM_Current_Interface_Ptr = int_ptr;

#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
    /*Begin:Added for Minirpc*/
    MINIRPC_SHM_Mgmt_Blk = (MINIRPC_SHM_MGMT_BLOCK*)__G_MINIRPC_SHARED_MEM_START_ADDR;
	memset(MINIRPC_SHM_Mgmt_Blk, 0, sizeof(MINIRPC_SHM_MGMT_BLOCK));
	minirpc_shm_init();
#endif

	/* Return status to caller */
	return status;
}

/* Return the first pending descriptor, or NULL. */
static SHM_BUFF_DESC* shm_desc_get_next(SHM_BUFF_DESC_Q* shm_des_q)
{
	/*Changed by tony for adding Lock*/
	SHM_BUFF_DESC *pdesc = MCAPI_NULL;

	shm_acquire_lock(&shm_des_q->lock);

	if (shm_des_q->count)
		pdesc = &shm_des_q->pkt_desc_q[shm_des_q->get_idx];

	shm_release_lock(&shm_des_q->lock);

	return pdesc;
}

static SHM_BUFF_DESC* shm_desc_get_next_nolock(SHM_BUFF_DESC_Q* shm_des_q)
{
        /*Changed by tony for adding Lock*/
        SHM_BUFF_DESC *pdesc = MCAPI_NULL;

        //shm_acquire_lock(&shm_des_q->lock);

        if (shm_des_q->count)
                pdesc = &shm_des_q->pkt_desc_q[shm_des_q->get_idx];

        //shm_release_lock(&shm_des_q->lock);

        return pdesc;
}

/* Make the first pending descriptor available to producers again. */
static void shm_desc_consume(SHM_BUFF_DESC_Q* shm_des_q)
{
	shm_acquire_lock(&shm_des_q->lock);

	/* Update index and count */
	shm_des_q->get_idx =  (shm_des_q->get_idx +1) % SHM_BUFF_DESC_Q_SIZE;
	shm_des_q->count--;

	shm_release_lock(&shm_des_q->lock);
}

static void shm_desc_consume_nolock(SHM_BUFF_DESC_Q* shm_des_q)
{
        //shm_acquire_lock(&shm_des_q->lock);

        /* Update index and count */
        shm_des_q->get_idx =  (shm_des_q->get_idx +1) % SHM_BUFF_DESC_Q_SIZE;
        shm_des_q->count--;

        //shm_release_lock(&shm_des_q->lock);
}

/*************************************************************************
*
*   FUNCTION
*
*       shm_poll
*
*   DESCRIPTION
*
*       RX HISR for the shared memory driver.
*
*   INPUTS
*
*       None
*
*   OUTPUTS
*
*       None
*
*************************************************************************/
//void shm_poll(void)
void shm_poll(uint32_t idxvalue)
{
	SHM_BUFF_DESC_Q* shm_des_q;
	SHM_BUFF_DESC*   shm_des;
	MCAPI_BUFFER*    rcvd_pkt;
	int              got_data = 0;
#if 0


#ifdef MCAPI_SM_DBG_SUPPORT
	printf("Received data\r\n");
#endif

	/* Obtain the SM ring queue for the current Node ID */
	shm_des_q = &SHM_Mgmt_Blk->shm_queues[MCAPI_Node_ID];

	/* Enqueue all available data packets for this node */
	for (;;)
	{
		/* Get next available SM buffer descriptor */
		//shm_des = shm_desc_get_next(shm_des_q);
		shm_des = shm_desc_get_next_nolock(shm_des_q);

		if (shm_des != MCAPI_NULL)
		{
			if (shm_des->priority < SHM_NUM_PRIORITIES)
			{
				/* Check packet type */
				if ((shm_des->type == MCAPI_MSG_TYPE) || \
					(shm_des->type == MCAPI_CHAN_PKT_TYPE) || \
					(shm_des->type == MCAPI_CHAN_SCAL_TYPE))
				{
					/* Packet buffer handling */
					rcvd_pkt = (MCAPI_BUFFER*)MCAPI_BUFFER_OF_INDEX(shm_des->value);

					/* Load current SM interface pointer */
					rcvd_pkt->mcapi_dev_ptr = (MCAPI_POINTER)SHM_Current_Interface_Ptr;

					/* Enqueue the packet received to the global queue */
					mcapi_enqueue(&MCAPI_RX_Queue[shm_des->priority],rcvd_pkt);

#ifdef MCAPI_SM_DBG_SUPPORT
					printf(" RX HISR - received buffer playload prototype  = %d \r\n",MCAPI_GET16(rcvd_pkt->buf_ptr, MCAPI_PROT_TYPE + MCAPI_HEADER_LEN));
					printf(" RX HISR - received buffer address  = %p \r\n", rcvd_pkt);
					printf(" RX HISR - received buffer offset   = 0x%x \r\n", ADDRESS_TO_OFFSET(rcvd_pkt));
					printf(" RX HISR - received buffer index    = %d \r\n", shm_des->value);
					printf(" RX HISR - received buffer size     = %d \r\n", (mcapi_uint32_t)rcvd_pkt->buf_size);
					printf(" RX HISR - received buffer priority = %d \r\n", shm_des->priority);
#endif
				}
				else
				{
					/* Scalar packet handling */

				}

				got_data = 1;

			}

			/* Consume current SM buffer descriptor */
			shm_desc_consume(shm_des_q);
		}
		else
		{
			break;
		}
	}
	/* Set notification event */
	if (got_data)
		MCAPI_Set_RX_Event();
		
#else /*Added by tony*/
/*
#if defined(__ALI_TDS__)
	libc_printf("SEE shm_poll, recv bufidx:%d\n",idxvalue);
#else
  printk("MAIN shm_poll, recv bufidx:%d\n",idxvalue);
#endif
*/
	if(idxvalue<SHM_BUFF_COUNT){
					/* Packet buffer handling */
					rcvd_pkt = (MCAPI_BUFFER*)MCAPI_BUFFER_OF_INDEX(idxvalue);

					/* Load current SM interface pointer */
					rcvd_pkt->mcapi_dev_ptr = (MCAPI_POINTER)SHM_Current_Interface_Ptr;

					/* Enqueue the packet received to the global queue */
					mcapi_enqueue(&MCAPI_RX_Queue[SHM_PRIO_0],rcvd_pkt);
					
					/* Set notification event */
					MCAPI_Set_RX_Event();
	}
	else{
#if defined(__ALI_TDS__)
	libc_printf("SEE shm_poll, recv a invalid bufidx:%d\n",idxvalue);
#else
  printk("MAIN shm_poll, recv a invalid bufidx:%d\n",idxvalue);
#endif	
	}
	
#endif
}
