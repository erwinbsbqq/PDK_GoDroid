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


#ifndef OPENMCAPI_SHM_H
#define OPENMCAPI_SHM_H

#if defined(__ALI_TDS__) || defined(__ALI_LINUX__)
typedef unsigned int uint32_t;
#endif

/* These are the only interfaces which must be implemented by every shm_os.c.
 * In addition, shm_os.c is responsible for invoking shm_poll() when new data
 * is available. */

#define MCAPI_OS_ERROR	MCAPI_ERR_GENERAL
mcapi_status_t openmcapi_shm_os_init(void);
mcapi_status_t openmcapi_shm_os_finalize(void);

void *shm_map(void);

int shm_init_device(int fd);

void openmcapi_shm_unmap(void *shm);

mcapi_status_t openmcapi_shm_notify(mcapi_uint32_t core_id, unsigned int data);
unsigned int minirpc_shm_notify(mcapi_uint32_t core_id, unsigned int data);
unsigned int minirpc_shm_syncack();

mcapi_uint32_t openmcapi_shm_localcoreid(void);
mcapi_uint32_t openmcapi_shm_remotecoreid(void);
mcapi_uint32_t openmcapi_shm_syncack(void); /*Added by tony on 2013/05/22*/

#endif /* OPENMCAPI_SHM_H */
