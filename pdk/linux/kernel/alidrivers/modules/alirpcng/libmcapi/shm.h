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


#ifndef MCAPI_SM_DRV_H
#define MCAPI_SM_DRV_H

#include "openmcapi.h"
#include "atomic.h"

#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
#include "ali_rpc_ctrl.h"
#endif

/* Configure buffering system */
/* Optimized by tony, we just need 2 buffer now */
/* Define the number of shared memory buffers that will be present
 * in the system */
//#define SHM_BUFF_COUNT           128
#define SHM_BUFF_COUNT           4

/* Define the depth of the SM packet descriptor ring queue. */
#define SHM_BUFF_DESC_Q_SHIFT    6 //4
#define SHM_BUFF_DESC_Q_SIZE     (1<<SHM_BUFF_DESC_Q_SHIFT)

#define SHM_INVALID_NODE         0xFF
#define SHM_INVALID_SCH_UNIT     0xFF

#define SHM_4K_ALIGN_SIZE        4*1024
#define SHM_INIT_COMPLETE_KEY    0xEF56A55A

/* Definition for SM buffer management structure */
#define SHM_NUM_PRIORITIES       2
#define SHM_PRIO_0               0
#define SHM_PRIO_1               1

//#define SHM_LOW_PRI_BUF_CONT     (SHM_BUFF_COUNT - (SHM_BUFF_COUNT/4))
#define SHM_LOW_PRI_BUF_CONT     (SHM_BUFF_COUNT - 1)
//#define BITMASK_WORD_COUNT       (SHM_BUFF_COUNT/BITMASK_WORD_SIZE)
#define BITMASK_WORD_COUNT       (1)
#define BITMASK_WORD_SIZE        32


/* SM buffer */

struct _shm_buff_
{
    mcapi_uint32_t idx;
    MCAPI_BUFFER   mcapi_buff;
};

typedef struct _shm_buff_ SHM_BUFFER;

/* SM buffer array */

struct _shm_buff_array_
{
    SHM_BUFFER shm_buff_array[SHM_BUFF_COUNT];
};

typedef struct _shm_buff_array_ SHM_BUFF_ARRAY;

/* SM buffer descriptor */

struct _shm_buff_desc_
{
    mcapi_int32_t  value;
    mcapi_uint16_t type;
    mcapi_uint16_t priority;
};

typedef struct _shm_buff_desc_ SHM_BUFF_DESC;

/* SM buffer descriptor queue */

struct _shm_buff_desc_q_
{
    SHM_BUFF_DESC  pkt_desc_q[SHM_BUFF_DESC_Q_SIZE];
    shm_lock       lock;
    mcapi_uint32_t put_idx;
    mcapi_uint32_t count;
    mcapi_uint32_t get_idx;
};

/* Routes */

struct _shm_route_
{
    mcapi_uint32_t node_id;
    mcapi_uint32_t unit_id;
};

typedef struct _shm_buff_desc_q_ SHM_BUFF_DESC_Q;

/* SM buffer management block */

struct _shm_buff_mgmt_blk_
{
    shm_lock       lock;
    mcapi_uint32_t buff_bit_mask[BITMASK_WORD_COUNT];
    mcapi_uint32_t shm_buff_count;
    mcapi_uint32_t shm_buff_base_offset;
};

typedef struct _shm_buff_mgmt_blk_ SHM_BUFF_MGMT_BLK;

/* SM driver mamagement block */

struct _shm_drv_mgmt_struct_
{
    shm_lock                    shm_init_lock;
    mcapi_uint32_t              shm_init_field;
    struct _shm_route_          shm_routes[CONFIG_SHM_NR_NODES];
    struct _shm_buff_desc_q_    shm_queues[CONFIG_SHM_NR_NODES];
    struct _shm_buff_mgmt_blk_  shm_buff_mgmt_blk;
};

typedef struct _shm_drv_mgmt_struct_ SHM_MGMT_BLOCK;


#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
/*Begin:add for minirpc*/
#define MINIRPC_BUFF_NUM               2
#define MINIRPC_MAX_MBX_LEN            128 //64 //256 //please don't over 256
#define MINIRPC_MERGE_MAX_MBX_LEN     (16*1024)

struct _minirpc_buffer
{
    unsigned char minirpc_shm_buff[MINIRPC_MAX_MBX_LEN];
    unsigned long busy_flag;
};
typedef struct _minirpc_buffer minirpc_buffer;

struct _minirpc_shm_drv_mgmt_struct_
{
    minirpc_buffer buffer_array[MINIRPC_BUFF_NUM];
};

typedef struct _minirpc_shm_drv_mgmt_struct_ MINIRPC_SHM_MGMT_BLOCK;

/* An minirpc interface used to send data to an endpoint. */
#define MINIRPC_INT_NAME_LEN          8

struct _minirpc_interface
{
    char            minirpc_int_name[MINIRPC_INT_NAME_LEN];
	unsigned int    minirpc_max_buf_size;
	unsigned int (*minirpc_tx_out)(unsigned int, unsigned int);
	unsigned char *(*minirpc_get_buffer)(unsigned int);
	unsigned int (*minirpc_free_buffer)(unsigned char*);
 
};

typedef struct _minirpc_interface MINIRPC_INTERFACE;

/*End:add for minirpc*/
#endif

/* Macros for address to offset conversion */

#define OFFSET_TO_ADDRESS(offset)  (void*)((mcapi_ulong_t)SHM_Mgmt_Blk + offset)

#define ADDRESS_TO_OFFSET(address) (mcapi_uint_t)((char*)address - (char *)SHM_Mgmt_Blk)

/* Note: have to use member address for not using packet attribute. */
#define INDEX_OF_MCAPI_BUFFER(address) (mcapi_int32_t)(((char*)address - (char *)&SHM_Buff_Array_Ptr->mcapi_buff)/sizeof(SHM_BUFFER))

/* Note: have to use member address for not using packet attribute. */
#define MCAPI_BUFFER_OF_INDEX(index) (MCAPI_BUFFER *)((char *)&SHM_Buff_Array_Ptr->mcapi_buff + index * (sizeof(SHM_BUFFER)))

//void shm_poll(void);
void shm_poll(mcapi_uint32_t idxvalue);

#endif /* MCAPI_SM_DRV_H */
