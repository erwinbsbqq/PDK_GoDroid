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
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
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

#ifndef __MCOMM_H__
#define __MCOMM_H__



#define u32 unsigned int

typedef u32 mcomm_core_t;
typedef u32 mcomm_mbox_t;

typedef struct shm_comm_data SHM_COMM_DATA;
struct shm_comm_data {
    u32 remoteId;
    u32 data;
};

/* Specify the layout of the mailboxes in shared memory */
#define MCOMM_INIT          0 //_IOW('*', 0, int)

/* Get hardware-defined number for the current core */
#define MCOMM_LOCAL_CPUID   1 //_IOW('*', 1, mcomm_core_t)

/* Get hardware-defined number for the current core */
#define MCOMM_REMOTE_CPUID  2 //_IOW('*', 2, mcomm_core_t)

/* Block until data is available for specified mailbox */
#define MCOMM_WAIT_READ     3 //_IOW('*', 3, mcomm_mbox_t)

/* Notify the specified core that data has been made available to it */
#define MCOMM_SEND          4 //_IOR('*', 4, struct shm_comm_data)

/* Send Sync ack  to peer*/
#define MCOMM_SYNCACK       5 //_IOR('*', 5, int )

struct mcomm_platform_ops {

	void (*send)(u32 core_id, u32 ctxt);
    /** Ack the peer.
     */
	void (*ack)(u32 core_id, u32 ctxt);
    /** Receive the mailbox context.
     */
	u32 (*recv)(u32 mbx_index);
    /** Sync after get the peer ack.
     */
	void (*sync)(void);
    /** Returns local core id.
     */
	unsigned long (*cpuid)(u32 remote);
};

//int mcomm_init(struct mcomm_platform_ops *ops, struct module *module);
void mcomm_exit(void);
long mcomm_ioctl(unsigned int ioctl, void *arg);


#endif /* __MCOMM_H__ */
