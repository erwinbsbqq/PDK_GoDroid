/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: shm_test.c
 *
 *  Description: shm_comm device test case implementation c file.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.02.19      David.L         0.1.000         Initial
 ****************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

typedef unsigned int u32;
typedef unsigned int uint32_t;
typedef unsigned int mcapi_status_t;
typedef unsigned int mcapi_uint32_t;

typedef uint32_t mcomm_core_t;
typedef uint32_t mcomm_mbox_t;

/*
 * MCAPI Status codes
 */
enum mcapi_status_codes {
	MCAPI_SUCCESS = 0,				// Indicates operation was successful
	MCAPI_PENDING,					// Indicates operation is pending without errors
	MCAPI_TIMEOUT,					// The operation timed out
	MCAPI_ERR_PARAMETER,			// Incorrect parameter
	MCAPI_ERR_DOMAIN_INVALID,		// The parameter is not a valid domain
	MCAPI_ERR_NODE_INVALID,			// The parameter is not a valid node
	MCAPI_ERR_NODE_INITFAILED,		// The MCAPI node could not be initialized
	MCAPI_ERR_NODE_INITIALIZED,		// MCAPI node is already initialized
	MCAPI_ERR_NODE_NOTINIT,			// The MCAPI node is not initialized
	MCAPI_ERR_NODE_FINALFAILED,		// The MCAPI could not be finalized
	MCAPI_ERR_PORT_INVALID,			// The parameter is not a valid port
	MCAPI_ERR_ENDP_INVALID,			// The parameter is not a valid endpoint descriptor
	MCAPI_ERR_ENDP_EXISTS,			// The endpoint is already created
	MCAPI_ERR_ENDP_GET_LIMIT,		// Endpoint get reference count is to high
	MCAPI_ERR_ENDP_DELETED,			// The endpoint has been deleted
	MCAPI_ERR_ENDP_NOTOWNER,		// An endpoint can only be deleted by its creator
	MCAPI_ERR_ENDP_REMOTE,			// Certain operations are only allowed on the node local endpoints
	MCAPI_ERR_ATTR_INCOMPATIBLE,	// Connection of endpoints with incompatible attributes not allowed
	MCAPI_ERR_ATTR_SIZE,			// Incorrect attribute size
	MCAPI_ERR_ATTR_NUM,				// Incorrect attribute number
	MCAPI_ERR_ATTR_VALUE,			// Incorrect attribute vale
	MCAPI_ERR_ATTR_NOTSUPPORTED,	// Attribute not supported by the implementation
	MCAPI_ERR_ATTR_READONLY,		// Attribute is read only
	MCAPI_ERR_MSG_SIZE,				// The message size exceeds the maximum size allowed by the MCAPI implementation
	MCAPI_ERR_MSG_TRUNCATED,		// The message size exceeds the buffer size
	MCAPI_ERR_CHAN_OPEN,			// A channel is open, certain operations are not allowed
	MCAPI_ERR_CHAN_TYPE,			// Attempt to open a packet/scalar channel on an endpoint that has been connected with a different channel type
	MCAPI_ERR_CHAN_DIRECTION,		// Attempt to open a send handle on a port that was connected as a receiver, or vice versa
	MCAPI_ERR_CHAN_CONNECTED,		// A channel connection has already been established for one or both of the specified endpoints
	MCAPI_ERR_CHAN_OPENPENDING,		// An open request is pending
	MCAPI_ERR_CHAN_CLOSEPENDING,	// A close request is pending.
	MCAPI_ERR_CHAN_NOTOPEN,			// The channel is not open (cannot be closed)
	MCAPI_ERR_CHAN_INVALID,			// Argument is not a channel handle
	MCAPI_ERR_PKT_SIZE,				// The packet size exceeds the maximum size allowed by the MCAPI implementation
	MCAPI_ERR_TRANSMISSION,			// Transmission failure
	MCAPI_ERR_PRIORITY,				// Incorrect priority level
	MCAPI_ERR_BUF_INVALID,			// Not a valid buffer descriptor
	MCAPI_ERR_MEM_LIMIT,			// Out of memory
	MCAPI_ERR_REQUEST_INVALID,		// Argument is not a valid request handle
	MCAPI_ERR_REQUEST_LIMIT,		// Out of request handles
	MCAPI_ERR_REQUEST_CANCELLED,	// The request was already canceled
	MCAPI_ERR_WAIT_PENDING,			// A wait is pending
	MCAPI_ERR_GENERAL,				// To be used by implementations for error conditions not covered by the other status codes
	MCAPI_OS_ERROR,
	MCAPI_STATUSCODE_END			// This should always be last
};

typedef struct shm_comm_data SHM_COMM_DATA;
struct shm_comm_data {
    u32 remoteId;
    u32 data;
};

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


static pthread_t shm_rx_thread;

/* XXX must close mcomm_fd somewhere */
static int mcomm_fd;
static size_t shm_bytes;

#define MCOMM_DEVICE "/dev/mcomm0"

#define SRC_FILE "./Final.pdf"
#define DST_FILE "./Dst.pdf"
#define MBX_SIZE 4

#define	MAIN_CPU_ID             0x22221111
#define	SEE_CPU_ID              0x22222222
#define	AUD0_CPU_ID             0x22223333
#define	AUD1_CPU_ID             0x22224444

static size_t shm_linux_read_size(void)
{
    unsigned long size;
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

    return size;
}

static mcapi_status_t openmcapi_shm_notify(mcapi_uint32_t core_id, uint32_t data)
{
    mcapi_status_t mcapi_status = MCAPI_SUCCESS;
    int rc;
    SHM_COMM_DATA comm;

    comm.remoteId = core_id;
    comm.data = data;

    rc = ioctl(mcomm_fd, MCOMM_SEND, &comm);
    if (rc)
        mcapi_status = MCAPI_OS_ERROR;

    return mcapi_status;
}

static mcapi_uint32_t openmcapi_shm_localcoreid(void)
{
    mcapi_uint32_t core_id;
    int rc;

    rc = ioctl(mcomm_fd, MCOMM_LOCAL_CPUID, &core_id);
    if (rc)
        return MCAPI_OS_ERROR;

    return core_id;
}

static mcapi_uint32_t openmcapi_shm_remotecoreid(void)
{
    mcapi_uint32_t core_id;
    int rc;

    rc = ioctl(mcomm_fd, MCOMM_REMOTE_CPUID, &core_id);
    if (rc)
        return MCAPI_OS_ERROR;

    return core_id;
}

static int shm_linux_wait_notify(uint32_t *ctxt)
{
    return ioctl(mcomm_fd, MCOMM_WAIT_READ, (void *)ctxt);
}

static int shm_linux_init_device(int fd)
{    
    return ioctl(fd, MCOMM_INIT, 0);
}

static void *openmcapi_shm_map(void)
{
    void *shm;
    int fd;
    int rc;

    shm_bytes = shm_linux_read_size();
    if (shm_bytes <= 0) {
        perror("read shared memory size");
        return NULL;
    }

    fd = open(MCOMM_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open " MCOMM_DEVICE);
        goto out1;
    }

    /* Get the new file descriptor for the initialized device. */
    rc = shm_linux_init_device(fd);
    if (rc < 0) {
        perror("couldn't initialize device");
        goto out2;
    }

    mcomm_fd = rc;

    shm = mmap(NULL, shm_bytes, PROT_READ|PROT_WRITE, MAP_SHARED, mcomm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap shared memory");
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
static void *mcapi_receive_thread(void *data)
{
    uint32_t ctxt;
    int rc;
    int canceltype;
    int dstd, bytes_wr, bytes_len;;
    char *ptr;
    
    dstd = open(DST_FILE, O_RDWR | O_CREAT, 0777);
    if (dstd == -1)
        printf("[TEST] open %s FAILED!\n", DST_FILE);

    do {
        /* Manually check if we're already dead. */
        pthread_testcancel();

        /* Block until data for this node is available or we are canceled with
         * a signal. */
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &canceltype);
        rc = shm_linux_wait_notify(&ctxt);
        if (!rc && (dstd != -1)) {
            ptr = (char *)&ctxt;
            bytes_len = MBX_SIZE;
            while (bytes_wr = write(dstd, ptr, bytes_len)) {
                if (bytes_wr == -1)
                    printf("[TEST] write %s failed!\n", DST_FILE);
                else if (bytes_wr == bytes_len)
                    break;
                else if (bytes_wr > 0) {
                    ptr += bytes_wr;
                    bytes_len -= bytes_wr;
                }
            }
        }

        pthread_setcanceltype(canceltype, NULL);
    } while (1);

    if (dstd != -1)
        close(dstd);

    printf("%s exiting!\n", __func__);
    return NULL;
}

/* Now that SM_Mgmt_Blk has been initialized, we can start the RX thread. */
static mcapi_status_t openmcapi_shm_os_init(void)
{
    mcapi_status_t mcapi_status = MCAPI_SUCCESS;
    int rc;

    rc = pthread_create(&shm_rx_thread, NULL, mcapi_receive_thread, NULL);
    if (rc) {
        perror("couldn't create pthread");
        mcapi_status = MCAPI_OS_ERROR;
    }

    return mcapi_status;
}

/* Finalize the SM driver OS specific layer. */
static mcapi_status_t openmcapi_shm_os_finalize(void)
{
    mcapi_status_t mcapi_status = MCAPI_SUCCESS;
    int rc;

    rc = pthread_cancel(shm_rx_thread);
    if (rc) {
        perror("couldn't cancel thread");
        mcapi_status = MCAPI_OS_ERROR;
    }

    /* Don't return until it's dead. */
    rc = pthread_join(shm_rx_thread, NULL);
    if (rc) {
        perror("couldn't joined canceled thread");
        mcapi_status = MCAPI_OS_ERROR;
    }

    printf("[TEST] openmcapi_shm_os_finalize SUCCESS\n");

    return mcapi_status;
}

static void openmcapi_shm_unmap(void *shm)
{
    munmap(shm, shm_bytes);

    close(mcomm_fd);
}

int main(int argc, char *argv[])
{
    void *shm;
    int srcd;
    int bytes_rd;
    uint32_t datum;

    shm = openmcapi_shm_map();

    openmcapi_shm_os_init();

    srcd = open(SRC_FILE, O_RDONLY);
    while (bytes_rd = read(srcd, &datum, MBX_SIZE)) {
        if (bytes_rd == -1)
            break;
        else if (bytes_rd > 0) {
            switch (bytes_rd) {
            case 1:
                datum &= 0x000000ff;
                printf("[TEST] read last 1 byte is: 0x%x!\n", datum);
                break;
            case 2:
                datum &= 0x0000ffff;
                printf("[TEST] read last 2 byte is: 0x%x!\n", datum);
                break;
            case 3:
                datum &= 0x00ffffff;
                printf("[TEST] read last 3 byte is: 0x%x!\n", datum);
                break;
            default:
                break;
            }
            if (openmcapi_shm_notify(SEE_CPU_ID, datum) != MCAPI_SUCCESS) {
                printf("[TEST] send (0x%x) failed!\n", datum);
            }
        }
    }

    close(srcd);

    /* Wait 2ms for the receiver completion. */
    usleep(2000);
    
    openmcapi_shm_os_finalize();

    openmcapi_shm_unmap(shm);

    return 0;
}

