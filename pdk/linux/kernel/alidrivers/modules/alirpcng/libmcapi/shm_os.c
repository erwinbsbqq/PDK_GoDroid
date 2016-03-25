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


#include "mcapi.h"
#include "shm.h"
#include "shm_os.h"

#if defined(__ALI_TDS__)
#include <types.h>
#include <retcode.h>
#include <shm_comm_device.h>

#elif defined(__ALI_LINUX__)
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#elif defined(__ALI_LINUX_KERNEL__)
#include "../../../include/alirpcng/mcomm.h"
#include <ali_shm.h>
#endif

#if defined(__ALI_TDS__)
#include <pr.h>
#elif defined(__ALI_LINUX__) || defined(__ALI_LINUX_KERNEL__)
#include "pr.h"
#endif


#if defined(__ALI_TDS__) || defined(__ALI_LINUX__)

typedef uint32_t u32;
typedef uint32_t mcomm_core_t;
typedef uint32_t mcomm_mbox_t;

typedef struct shm_comm_data SHM_COMM_DATA;
struct shm_comm_data {
    u32 remoteId;
    u32 data;
};

#endif

#if defined(__ALI_LINUX__)

/* Specify the layout of the mailboxes in shared memory */
#define MCOMM_INIT          _IOW('*', 0, int)

/* Get hardware-defined number for the current core */
#define MCOMM_LOCAL_CPUID   _IOW('*', 1, mcomm_core_t)

/* Get hardware-defined number for the current core */
#define MCOMM_REMOTE_CPUID  _IOW('*', 2, mcomm_core_t)

/* Block until data is available for specified mailbox */
#define MCOMM_WAIT_READ     _IOW('*', 3, mcomm_mbox_t)

/* Notify the specified core that data has been made available to it */
#define MCOMM_SEND          _IOR('*', 4, struct shm_comm_data)

/* Send Sync ack  to peer*/
#define MCOMM_SYNCACK       _IOR('*', 5, int )

#endif

static Thread shm_rx_thread;

#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
Thread minirpc_shm_rx_thread;
#endif

#if defined(__ALI_TDS__)
struct shm_comm_device *mcomm_dev;
#elif defined(__ALI_LINUX__)
/* XXX must close mcomm_fd somewhere */
static int mcomm_fd;
#elif defined(__ALI_LINUX_KERNEL__)

/*Changed by tony on 2013/06/07*/
/*
extern unsigned long __G_ALI_MM_SHARED_MEM_TOP_ADDR;
extern unsigned long __G_ALI_MM_SHARED_MEM_START_ADDR;
*/
extern unsigned long __G_ALI_MM_MCAPI_MEM_SIZE;
extern unsigned long __G_ALI_MM_MCAPI_MEM_START_ADDR;
#endif

static size_t shm_bytes;

#define MCOMM_DEVICE "/dev/mcomm0"

static size_t shm_linux_read_size(void)
{
    unsigned long size = 0;

#if defined(__ALI_TDS__)

    Int32 rc;
    void *shm;

    rc = mcomm_dev_attach();
    if (rc != SUCCESS) {
		PR_LOG("%s: mcomm_dev_attach failed!\n", __func__);
        return size;
    }

    mcomm_dev = (struct shm_comm_device *)dev_get_by_id(HLD_DEV_TYPE_MCAPI, 0);
    rc = mcomm_open(mcomm_dev);
    if (rc != SUCCESS) {
		PR_LOG("%s: mcomm_open failed!\n", __func__);
        return size;
    }

    rc = mcomm_ioctl(mcomm_dev, MCOMM_MMAP, (void *)&shm);
    if (rc != SUCCESS) {
		PR_LOG("%s: MCOMM_MMAP failed!\n", __func__);
        return size;
    }

    rc = mcomm_ioctl(mcomm_dev, MCOMM_SHM_SIZE, &size);
    if (rc != SUCCESS) {
        size = 0;
        PR_LOG("%s: MCOMM_SHM_SIZE failed!\n", __func__);
        return size;
    }
    
#elif defined(__ALI_LINUX__)

    FILE *f;
    int rc;

    f = fopen("/sys/devices/platform/mcomm.0/size", "r");
    if (!f)
        f = fopen("/sys/devices/mcomm.0/size", "r");
    if (!f)
        return errno;

    rc = fscanf(f, "%lx", &size);
    if (rc < 0)
        size = errno;

    fclose(f);

#elif defined(__ALI_LINUX_KERNEL__)
    //size = __G_RPC_MM_LEN;
    size = __G_ALI_MM_MCAPI_MEM_SIZE;
#endif

    return size;
}

mcapi_status_t openmcapi_shm_notify(mcapi_uint32_t core_id, uint32_t data)
{
    mcapi_status_t mcapi_status = MCAPI_SUCCESS;
    int rc;
    SHM_COMM_DATA comm;

    comm.remoteId = core_id;
    comm.data = data;

#if defined(__ALI_TDS__)

    rc = mcomm_ioctl(mcomm_dev, MCOMM_SEND, &comm);
    if (rc)
        mcapi_status = MCAPI_OS_ERROR;

#elif defined(__ALI_LINUX__)

    rc = ioctl(mcomm_fd, MCOMM_SEND, &comm);
    if (rc)
        mcapi_status = MCAPI_OS_ERROR;

#elif defined(__ALI_LINUX_KERNEL__)

    rc = mcomm_ioctl(MCOMM_SEND, &comm);
    if (rc)
        mcapi_status = MCAPI_OS_ERROR;

#endif

    return mcapi_status;
}




mcapi_uint32_t openmcapi_shm_localcoreid(void)
{
    mcapi_uint32_t core_id;
    int rc;

#if defined(__ALI_TDS__)

    rc = mcomm_ioctl(mcomm_dev, MCOMM_LOCAL_CPUID, &core_id);
    if (rc)
        return MCAPI_OS_ERROR;

#elif defined(__ALI_LINUX__)

    rc = ioctl(mcomm_fd, MCOMM_LOCAL_CPUID, &core_id);
    if (rc)
        return MCAPI_OS_ERROR;

#elif defined(__ALI_LINUX_KERNEL__)

    rc = mcomm_ioctl(MCOMM_LOCAL_CPUID, &core_id);
    if (rc)
        return MCAPI_OS_ERROR;

#endif

    return core_id;
}

/*Added by tony on 2013/05/22*/
mcapi_uint32_t openmcapi_shm_syncack(void)
{
    int rc;
    int dummydata;

#if defined(__ALI_TDS__)

    rc = mcomm_ioctl(mcomm_dev, MCOMM_SYNCACK, &dummydata);
    if (rc)
        return MCAPI_OS_ERROR;

#elif defined(__ALI_LINUX_KERNEL__)

    rc = mcomm_ioctl(MCOMM_SYNCACK, &dummydata);
    if (rc)
        return MCAPI_OS_ERROR;

#endif

    return MCAPI_SUCCESS;
}

mcapi_uint32_t openmcapi_shm_remotecoreid(void)
{
    mcapi_uint32_t core_id;
    int rc;

#if defined(__ALI_TDS__)

    rc = mcomm_ioctl(mcomm_dev, MCOMM_REMOTE_CPUID, &core_id);
    if (rc)
        return MCAPI_OS_ERROR;

#elif defined(__ALI_LINUX__)

    rc = ioctl(mcomm_fd, MCOMM_REMOTE_CPUID, &core_id);
    if (rc)
        return MCAPI_OS_ERROR;

#elif defined(__ALI_LINUX_KERNEL__)
    
        rc = mcomm_ioctl(MCOMM_REMOTE_CPUID, &core_id);
        if (rc)
            return MCAPI_OS_ERROR;
    
#endif

    return core_id;
}

static int shm_linux_wait_notify(uint32_t *ctxt)
{
#if defined(__ALI_TDS__)

    return mcomm_ioctl(mcomm_dev, MCOMM_WAIT_READ, (void *)ctxt);

#elif defined(__ALI_LINUX__)

    return ioctl(mcomm_fd, MCOMM_WAIT_READ, (void *)ctxt);

#elif defined(__ALI_LINUX_KERNEL__)

    return mcomm_ioctl(MCOMM_WAIT_READ, (void *)ctxt);

#endif
}

static int shm_linux_init_device(int fd)
{
#if defined(__ALI_TDS__)

    return mcomm_ioctl(mcomm_dev, MCOMM_INIT, 0);

#elif defined(__ALI_LINUX__)

    return ioctl(fd, MCOMM_INIT, 0);

#elif defined(__ALI_LINUX_KERNEL__)

    return mcomm_ioctl(MCOMM_INIT, 0);

#endif
}

void *openmcapi_shm_map(void)
{
    void *shm;
    int rc;

#if defined(__ALI_TDS__)

    unsigned int size;

    shm_bytes = shm_linux_read_size();
    if (shm_bytes <= 0) {
        PR_LOG("read shared memory size.\n");
        goto out2;
    }

    rc = shm_linux_init_device(0);
    if (rc < 0) {
        PR_LOG("couldn't initialize device.\n");
        goto out2;
    }

    rc = mcomm_ioctl(mcomm_dev, MCOMM_MMAP, (void *)&shm);
    if (rc) {
        PR_LOG("couldn't map shared memory.\n");
        goto out1;
    }

    return shm;

out2:
    mcomm_ioctl(mcomm_dev, MCOMM_MUNMAP, 0);
out1:
    return NULL;

#elif defined(__ALI_LINUX__)

    int fd;

    shm_bytes = shm_linux_read_size();
    if (shm_bytes <= 0) {
        PR_LOG("read shared memory size.\n");
        return NULL;
    }

    fd = open(MCOMM_DEVICE, O_RDWR);
    if (fd < 0) {
        PR_LOG("open " MCOMM_DEVICE);
        goto out1;
    }

    /* Get the new file descriptor for the initialized device. */
    rc = shm_linux_init_device(fd);
    if (rc < 0) {
        PR_LOG("couldn't initialize device.\n");
        goto out2;
    }

    mcomm_fd = rc;

    shm = mmap(NULL, shm_bytes, PROT_READ|PROT_WRITE, MAP_SHARED, mcomm_fd, 0);
    if (shm == MAP_FAILED) {
        PR_LOG("mmap shared memory.\n");
        goto out3;
    }

    /* Don't need the original fd around any more. */
    close(fd);

    return shm;

out3:
    close(mcomm_fd);
out2:
    close(fd);
out1:
    return NULL;

#elif defined(__ALI_LINUX_KERNEL__)

    shm_bytes = shm_linux_read_size();
    if (shm_bytes <= 0) {
        PR_LOG("read shared memory size.\n");
        return NULL;
    }

    rc = shm_linux_init_device(0);
    if (rc < 0) {
        PR_LOG("couldn't initialize device.\n");
        return NULL;
    }

    //shm = (void *)(__G_ALI_MM_SHARED_MEM_START_ADDR | 0xa0000000);
    shm = (void *)__VMEMALI(__G_ALI_MM_MCAPI_MEM_START_ADDR);

    return shm;

#endif
}

/* pthread cancellation is tricky.
 *
 * First, we need to ensure that if we're blocked in the kernel (ioctl),
 * pthread_cancel must send us a signal so we return to userspace to handle it.
 * This is accomplished with PTHREAD_CANCEL_ASYNCHRONOUS, without which glibc
 * would set some state and then happily wait for us to hit the next
 * cancellation point (see pthreads(7)). However, we wouldn't, because we'd
 * never wake from the kernel.
 *
 * Second, pthread_cancel() could be called any time outside the
 * PTHREAD_CANCEL_ASYNCHRONOUS section. Since we don't necessarily have any
 * cancellation points inside the loop (including the shm_poll() path), we must
 * manually check by calling pthread_testcancel().
 */
static void mcapi_receive_thread(void *data)
{
    uint32_t ctxt;
    int rc;

#if defined(__ALI_TDS__)

    do {
        /* Manually check if we're already dead. */
        if (PR_ThreadTestCancel())
            break;

        rc = shm_linux_wait_notify(&ctxt);
        if (rc) {
            PR_LOG("!!!!!!SEE wait ioctl error.\n");
            continue;
            //break;
        }

        /* Obtain lock so we can safely manipulate the RX_Queue. */
        MCAPI_Lock_RX_Queue();

        /* Process the incoming data. */
        //shm_poll();
        shm_poll(ctxt);

        MCAPI_Unlock_RX_Queue(0);

    } while (1);

#elif defined(__ALI_LINUX__)

    int canceltype;

    do {
        /* Manually check if we're already dead. */
        pthread_testcancel();

        /* Block until data for this node is available or we are canceled with
         * a signal. */
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &canceltype);
        rc = shm_linux_wait_notify(&ctxt);
        if (rc) {
            PR_LOG("!!!!!Linux wait ioctl error.\n");
	    continue;
            //break;
        }
        pthread_setcanceltype(canceltype, NULL);

        /* Obtain lock so we can safely manipulate the RX_Queue. */
        MCAPI_Lock_RX_Queue();

        /* Process the incoming data. */
        //shm_poll();
        shm_poll(ctxt);

        MCAPI_Unlock_RX_Queue(0);

    } while (1);

#elif defined(__ALI_LINUX_KERNEL__)

    do {
        /* Manually check if we're already dead. */
        if (PR_ThreadTestCancel())
            break;

        rc = shm_linux_wait_notify(&ctxt);
        if (rc) {
            PR_LOG("!!!!!!!!MAIN wait ioctl error.\n");
            continue;
            //break;
        }

        /* Obtain lock so we can safely manipulate the RX_Queue. */
        MCAPI_Lock_RX_Queue();

        /* Process the incoming data. */
        //shm_poll();
        shm_poll(ctxt);

        MCAPI_Unlock_RX_Queue(0);

    } while (1);

#endif

}

/* Now that SM_Mgmt_Blk has been initialized, we can start the RX thread. */
mcapi_status_t openmcapi_shm_os_init(void)
{
    mcapi_status_t mcapi_status = MCAPI_SUCCESS;
    int rc;

    rc = PR_ThreadCreate(&shm_rx_thread, mcapi_receive_thread, NULL, 0, 0, NULL);
    if (rc) {
        PR_LOG("couldn't create mcapi_receive_thread.\n");
        mcapi_status = MCAPI_OS_ERROR;
    }
    PR_uSleep(20000);

    return mcapi_status;
}

/* Finalize the SM driver OS specific layer. */
mcapi_status_t openmcapi_shm_os_finalize(void)
{
    mcapi_status_t mcapi_status = MCAPI_SUCCESS;
    int rc;

    rc = PR_ThreadDestroy(&shm_rx_thread);
    if (rc) {
        PR_LOG("couldn't cancel mcapi_receive_thread.\n");
        mcapi_status = MCAPI_OS_ERROR;
    }

    return mcapi_status;
}

void openmcapi_shm_unmap(void *shm)
{
#if defined(__ALI_TDS__)

    mcomm_ioctl(mcomm_dev, MCOMM_MUNMAP, 0);

    mcomm_close(mcomm_dev);
    
#elif defined(__ALI_LINUX__)

    munmap(shm, shm_bytes);

    close(mcomm_fd);

#elif defined(__ALI_LINUX_KERNEL__)
    /* Do nothing. */
#endif
}


#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
int shm_linux_wait_notify_minirpc(uint32_t *ctxt)
{
    return mcomm_ioctl(mcomm_dev, MCOMM_WAIT_READ_MINIRPC, (void *)ctxt);
}

static void minirpc_receive_thread(void *data)
{
    uint32_t ctxt;
    int rc;

    do {
        /* Manually check if we're already dead. */
        if (PR_ThreadTestCancel())
            break;

        rc = shm_linux_wait_notify_minirpc(&ctxt);
        if (rc) 
		{
            PR_LOG("!!!!!!SEE wait ioctl error.\n");
            continue;
            //break;
        }

		MiniRpcRecvProc(ctxt);			
		
    } while (1);

}


int minirpc_create_shm_thread()
{
    int rc=-1;
    rc = PR_ThreadCreate(&minirpc_shm_rx_thread, minirpc_receive_thread, NULL, NULL, NULL, "minirpc");
	if (rc) 
	{
	   perror("couldn't create pthread");
	}

	return rc;
}

int minirpc_shm_notify(mcapi_uint32_t core_id, uint32_t data)
{
    int rc;
    SHM_COMM_DATA comm;

    comm.remoteId = core_id;
    comm.data = data;

#if defined(__ALI_TDS__)

    rc = mcomm_ioctl(mcomm_dev, MCOMM_SEND, &comm);
#endif

    return rc;
}

int minirpc_shm_syncack()
{
    unsigned int status = MCAPI_SUCCESS;
    int rc;
    int dummydata;

#if defined(__ALI_TDS__)

    rc = mcomm_ioctl(mcomm_dev, MCOMM_SYNCACK, &dummydata);
    if (rc)
        status = MCAPI_OS_ERROR;
#endif

    return status;
}
#endif

