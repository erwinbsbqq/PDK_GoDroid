/******************************************************************************
 * cbuffer.c - circle buffer module, only store integer
 * DESCRIPTION: -
 *     circle buffer module, only store integer.
 *     define the mutex or wait flag in cbuffer.h
 *
 * Modification History
 * --------------------
 * 1. 2012-6-14, Dong yun written
 * --------------------
 ******************************************************************************/

#include "osal/cbuffer.h"
#include <hld_cfg.h>


#define CBUFFER_DEBUG(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(CBUFFER, fmt, ##args); \
			} while(0)

#undef _NEXT
#undef _EMPTY
#undef _FULL
#undef _SET_EMPTY

#define _NEXT(cbuf,x)       (x>=cbuf->size?0:x+1)
#define _EMPTY(buf)         (_NEXT(buf, buf->ard) == buf->wwr)
#define _FULL(buf)          (buf->wwr == buf->ard)
#define _SET_EMPTY(cbuf)    cbuf->wwr = 0; cbuf->ard = cbuf->size; cbuf->count = 0;



/******************************************************************************
 * _cbuffer_get - get a value from circle buffer
 * DESCRIPTION: -
 *    get a value from circle buffer.
 * Input:
 *    cbuf      : circle buffer
 *    timeout_ms: time out, measured in ms.
 *    del       : indicate if delete the value after got it.
 * Output:
 * Returns:
 *    return the value if success, else return 0.
 * Modification History:
 * -------------------------------------------------
 * 1. 2012-6-13, Dong yun Created
 *
 * -------------------------------------------------
 ******************************************************************************/
static unsigned int _cbuffer_get(struct cbuffer *cbuf, int timeout_ms, int del)
{
    int ard = 0, ret = 0;
    unsigned int val = 0;
    int timeout = timeout_ms;

    if (cbuf == NULL)
    {
        CBUFFER_DEBUG("buffer pointer \"cbuf\" is null!!\n");
        return 0;
    }

_GET_WAIT:

    if (_EMPTY(cbuf))
    {
        if (timeout == 0) //not wait just return.
        {
            CBUFFER_DEBUG("circle buffer is empty, don't wait!\n");
            return 0;
        }

        CBUFFER_DEBUG("circle buffer is empty, need wait %dms\n", timeout);
        timeout = cbuf->sync ? CBUFFER_FLAG_WAIT(cbuf->rflag, timeout) : 0;
    }

    ret = cbuf->sync ? CBUFFER_MUTEX_LOCK(cbuf->rmutex) : 0;

    if (_EMPTY(cbuf))
    {
        ret = cbuf->sync ? CBUFFER_MUTEX_UNLOCK(cbuf->rmutex) : 0;

        if (timeout == 0)
        {
            CBUFFER_DEBUG("circle buffer is empty, wait timeout!\n");
            return 0;
        }

        goto _GET_WAIT;
    }

    ard = _NEXT(cbuf, cbuf->ard);
    val = cbuf->qbuf[ard];

    if (del) // delete the value
    {
        cbuf->ard = ard;
        cbuf->count--;
    }

    ret = cbuf->sync ? CBUFFER_MUTEX_UNLOCK(cbuf->rmutex) : 0;

    if (del) //delete the value need notify
    {
        ret = cbuf->sync ? CBUFFER_FLAG_SET(cbuf->wflag) : 0;    //notify that a value has been got out.
    }

    CBUFFER_DEBUG("<<<get a value %d at %d\n", val, ard);
    return val;
}

/******************************************************************************
 * cbuffer_clear - clear buffer, remove all values
 * DESCRIPTION: -
 *    clear buffer, remove all values
 * Input:
 * Output:
 * Returns:
 * Modification History:
 * -------------------------------------------------
 * 1. 2012-6-15, Dong yun Created
 * -------------------------------------------------
 ******************************************************************************/
static void cbuffer_clear(struct cbuffer *cbuf)
{
    int ret = 0;

    if (cbuf == NULL)
    {
        return;
    }

    ret = cbuf->sync ? CBUFFER_MUTEX_LOCK(cbuf->rmutex) : 0; //disable read
    ret = cbuf->sync ? CBUFFER_MUTEX_LOCK(cbuf->wmutex) : 0; //disable write
    ////////////////////////////////////////////////////////
    _SET_EMPTY(cbuf);
    //CBUFFER_MEMSET(cbuf->qbuf, 0 , sizeof(unsigned int) * (cbuf->size)); //clear buffer
    ////////////////////////////////////////////////////////
    ret = cbuf->sync ? CBUFFER_MUTEX_UNLOCK(cbuf->wmutex) : 0; //eable write
    ret = cbuf->sync ? CBUFFER_MUTEX_UNLOCK(cbuf->rmutex) : 0; //eable read
}


/******************************************************************************
 * cbuffer_put - put a value to circle buffer
 * DESCRIPTION: -
 *    put a value to circle buffer
 * Input:
 *    cbuf      : circle buffer
 *    msg       : value must be a integer
 *    timeout_ms: time out, measured in ms.
 *                > 0: wait time out
 *                = 0: not wait,just return.
 *                =-1: wait forever
 * Output:
 * Returns:
 *    return 1 if success, else return 0.
 * Modification History:
 * -------------------------------------------------
 * 1. 2012-6-13, Dong yun Created
 * -------------------------------------------------
 ******************************************************************************/
static unsigned int cbuffer_put(struct cbuffer *cbuf, unsigned int val, int timeout_ms)
{
    int wwr = 0, ret = 0;
    int timeout = timeout_ms;

    if (cbuf == NULL)
    {
        CBUFFER_DEBUG("buffer pointer \"cbuf\" is null!!\n");
        return 0;
    }

    if (val == 0)
    {
        CBUFFER_DEBUG("can not put zero to into buffer, it is a ivalid value!!\n");
        return 0;
    }

_PUT_WAIT:

    if (_FULL(cbuf))
    {
        if (cbuf->overlap) //drop the head data
        {
            _cbuffer_get(cbuf, 0, 1);
        }
        else
        {
            if (timeout == 0) //not wait just return.
            {
                CBUFFER_DEBUG("circle buffer is full, don't wait!\n");
                return 0;
            }

            CBUFFER_DEBUG("circle buffer is full, need wait %dms\n", timeout);
            timeout = cbuf->sync ? CBUFFER_FLAG_WAIT(cbuf->wflag, timeout) : 0;
        }
    }

    ret = cbuf->sync ? CBUFFER_MUTEX_LOCK(cbuf->wmutex) : 0;

    if (_FULL(cbuf))
    {
        ret = cbuf->sync ? CBUFFER_MUTEX_UNLOCK(cbuf->wmutex) : 0;

        if (timeout == 0 && cbuf->overlap == 0)
        {
            CBUFFER_DEBUG("circle buffer is full, wait timeout!\n");
            return 0;
        }

        goto _PUT_WAIT;
    }

    wwr = cbuf->wwr;
    cbuf->qbuf[cbuf->wwr] = val;
    cbuf->wwr = _NEXT(cbuf, cbuf->wwr);
    cbuf->count++;
    ret = cbuf->sync ? CBUFFER_MUTEX_UNLOCK(cbuf->wmutex) : 0;
    ret = cbuf->sync ? CBUFFER_FLAG_SET(cbuf->rflag) : 0; //notify that a value has been put in.
    CBUFFER_DEBUG(">>>put a value %d at %d\n", val, wwr);
    return 1;
}


/******************************************************************************
 * cbuffer_get - get a value from circle buffer
 * DESCRIPTION: -
 *    get a value from circle buffer and remove the value from buffer.
 * Input:
 *    cbuf      : circle buffer
 *    timeout_ms: time out, measured in ms.
 *                > 0: wait time out
 *                = 0: not wait,just return.
 *                =-1: wait forever
 * Output:
 * Returns:
 *    return the value if success, else return 0.
 * Modification History:
 * -------------------------------------------------
 * 1. 2012-6-13, Dong yun Created
 *
 * -------------------------------------------------
 ******************************************************************************/
static unsigned int cbuffer_get(struct cbuffer *cbuf, int timeout_ms)
{
    return _cbuffer_get(cbuf, timeout_ms, 1);
}


/******************************************************************************
 * cbuffer_preget - pretreating get a value from circle buffer
 * DESCRIPTION: -
 *    pretreating get a value from circle buffer, do not delete the value
 * Input:
 *    cbuf      : circle buffer
 *    timeout_ms: time out, measured in ms.
 *                > 0: wait time out
 *                = 0: not wait,just return.
 *                =-1: wait forever
 * Output:
 * Returns:
 *    return the value if success, else return 0.
 * Modification History:
 * -------------------------------------------------
 * 1. 2012-6-13, Dong yun Created
 *
 * -------------------------------------------------
 ******************************************************************************/
static unsigned int cbuffer_preget(struct cbuffer *cbuf, int timeout_ms)
{
    return _cbuffer_get(cbuf, timeout_ms, 0);
}

/******************************************************************************
 * cbuffer_create - create a new circle buffer
 * DESCRIPTION: -
 *    create a new circle buffer
 * Input:
 *    size : buffer size
 *    sync : synchronous flag
 *           0: don't synchronous between get and put,
 *           1: do synchronous between get and put
 * overlap : overlap flag
 *           0: don't remove the head data when buffer is full,
 *           1: remove the head data when buffer is full.
 * Output:
 * Returns:
 *   return the new circle buffer address if success, else return NULL
 * Modification History:
 * -------------------------------------------------
 * 1. 2012-6-13, Dong yun Created
 * -------------------------------------------------
 ******************************************************************************/
struct cbuffer *cbuffer_create(int size, int sync, int overlap)
{
    int ret = 0;
    int _size = 0;
    struct cbuffer *cbuf = NULL;

    if (size < 2)
    {
        CBUFFER_DEBUG("too small buffer size, must more than 2!\n");
        return NULL;
    }

    _size  = sizeof(struct cbuffer) ;
    cbuf = CBUFFER_MALLOC(_size);

    if (cbuf == NULL)
    {
        CBUFFER_DEBUG("malloc failed!\n");
        goto _ERR2;
    }

    CBUFFER_MEMSET(cbuf, 0, _size);

    _size  = sizeof(unsigned int) *size;
    cbuf->qbuf = CBUFFER_MALLOC(_size);

    if (cbuf->qbuf == NULL)
    {
        CBUFFER_DEBUG("malloc failed!\n");
        goto _ERR2;
    }

    CBUFFER_MEMSET(cbuf->qbuf, 0, _size);

    cbuf->size = size;
    cbuf->sync = sync;
    cbuf->overlap = overlap;
    _SET_EMPTY(cbuf);
    cbuf->get = cbuffer_get;
    cbuf->preget = cbuffer_preget;
    cbuf->put = cbuffer_put;
    cbuf->clear = cbuffer_clear;

    if (sync) // synchronous between get and put
    {
        ret = CBUFFER_MUTEX_INIT(cbuf->wmutex);

        if (ret)
        {
            CBUFFER_DEBUG("create mutex failed!\n");
            goto _ERR1;
        }

        ret = CBUFFER_MUTEX_INIT(cbuf->rmutex);

        if (ret)
        {
            ret = CBUFFER_MUTEX_DELETE(cbuf->wmutex);
            CBUFFER_DEBUG("create mutex failed!\n");
            goto _ERR1;
        }

        ret = CBUFFER_FLAG_INIT(cbuf->wflag);

        if (ret)
        {
            ret = CBUFFER_MUTEX_DELETE(cbuf->wmutex);
            ret = CBUFFER_MUTEX_DELETE(cbuf->rmutex);
            CBUFFER_DEBUG("create wait flag failed!\n");
            goto _ERR1;
        }

        ret = CBUFFER_FLAG_INIT(cbuf->rflag);

        if (ret)
        {
            ret = CBUFFER_MUTEX_DELETE(cbuf->wmutex);
            ret = CBUFFER_MUTEX_DELETE(cbuf->rmutex);
            ret = CBUFFER_FLAG_DELETE(cbuf->wflag);
            CBUFFER_DEBUG("create wait flag failed!\n");
            goto _ERR1;
        }
    }

    CBUFFER_DEBUG("create circle buffer success, size:%d [%s]\n", size, sync ? "sync" : "no sync");
    return cbuf;
_ERR1:
    CBUFFER_FREE(cbuf->qbuf);
_ERR2:
    CBUFFER_FREE(cbuf);
    return NULL;
}

/******************************************************************************
 * cbuffer_delete - delete circle buffer and release resource
 * DESCRIPTION: -
 *    delete circle buffer and release resource
 * Input:
 * Output:
 * Returns:
 * Modification History:
 * -------------------------------------------------
 * 1. 2012-6-13, Dong yun Created
 * -------------------------------------------------
 ******************************************************************************/
void cbuffer_delete(struct cbuffer *cbuf)
{
    int ret = 0;

    if (cbuf == NULL)
    {
        return;
    }

    CBUFFER_FREE(cbuf->qbuf);

    if (cbuf->sync)
    {
        ret = CBUFFER_MUTEX_DELETE(cbuf->wmutex);
        ret = CBUFFER_MUTEX_DELETE(cbuf->rmutex);
        ret = CBUFFER_FLAG_DELETE(cbuf->wflag);
        ret = CBUFFER_FLAG_DELETE(cbuf->rflag);
    }

    CBUFFER_FREE(cbuf);
    CBUFFER_DEBUG("delete a circle buffer!\n");
}


#ifdef DEF_WAIT_FLAG //default wait flags
/******************************************************************************
 * wait_flag_init - wait flag init, use pthread mutext and pthread condition variable
 * DESCRIPTION: -
 *    wait flag init, use pthread mutext and pthread condition variable
 * Input:
 *    flag : waitfalg
 * Output:
 * Returns:
 *    return 0 if success, else return non-zero.
 * Modification History:
 * -------------------------------------------------
 * 1. 2012-6-27, Dong yun Created
 * -------------------------------------------------
 ******************************************************************************/
int wait_flag_init(waitflag *flag)
{
    int ret = 0;
    ret = pthread_mutex_init(&flag->_mutex, NULL);

    if (ret)
    {
        return ret;
    }

    ret = pthread_cond_init(&flag->_cond, NULL);

    if (ret)
    {
        return ret;
    }

    return ret;
}

/******************************************************************************
 * wait_flag_delete - delete wait falg
 * DESCRIPTION: -
 *    delete wait falg,delete mutex and condition variable
 * Input:
 *    flag : wait_falg
 * Output:
 * Returns:
 * Modification History:
 * -------------------------------------------------
 * 1. 2012-6-27, Dong yun Created
 * -------------------------------------------------
 ******************************************************************************/
int wait_flag_delete(waitflag *flag)
{
    pthread_mutex_destroy(&flag->_mutex);
    pthread_cond_destroy(&flag->_cond);
    return 1;
}

/******************************************************************************
 * wait_flag_timeout - wait a flag
 * DESCRIPTION: -
 *    wait a flag, it will suspend the task.
 * Input:
 *    flag:       wait flag
 *    timeout_ms: >0:wait time, -1: wait forever, 0:not wait.
 * Output:
 * Returns:
 *    0: wait timeout
 *   >0: get the flag before timeout,and return the remain time_ms.
 * Modification History:
 * -------------------------------------------------
 * 1. 2012-6-27, Dong yun Created
 * -------------------------------------------------
 ******************************************************************************/
int wait_flag_timeout(waitflag *flag, int timeout_ms)
{
    struct timespec ts, new_ts;
    int ret = -1;
    int to = 0;

    if (timeout_ms <= 0)
    {
        return 0;
    }

    //if (timeout_ms > 0)
    {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += (timeout_ms / 1000);
        ts.tv_nsec += (timeout_ms % 1000) * 1000000;

        while (ts.tv_nsec >= 1000000000)
        {
            ts.tv_nsec -= 1000000000;
            ts.tv_sec++;
        }
    }

    pthread_mutex_lock(&flag->_mutex);

    if (timeout_ms == -1) //wait forever
    {
        ret = pthread_cond_wait(&flag->_cond, &flag->_mutex);
    }
    else //wait timeout
    {
        ret = pthread_cond_timedwait(&(flag->_cond), &flag->_mutex, &ts);

        if (ret == 0) //calculate the remain time,maybe it will be used again.
        {
            clock_gettime(CLOCK_REALTIME, &new_ts);

            if (ts.tv_sec >= new_ts.tv_sec || ts.tv_nsec > new_ts.tv_nsec)
            {
                to += 1000 * (ts.tv_sec - new_ts.tv_sec);
                to += (ts.tv_nsec - new_ts.tv_nsec) / 1000000;
            }
        }
    }

    pthread_mutex_unlock(&flag->_mutex);
    return to;
}

/******************************************************************************
 * wait_flag_set - set a flag
 * DESCRIPTION: -
 *    set a flag ,it will notify all tasks who wait flag.
 * Input:
 *    flag: wait flag
 * Output:
 * Returns:
 *    return 1 if success, else return 0.
 * Modification History:
 * -------------------------------------------------
 * 1. 2012-6-27, Dong yun Created
 * -------------------------------------------------
 ******************************************************************************/
int wait_flag_set(waitflag *flag)
{
    pthread_mutex_lock(&flag->_mutex);
    pthread_cond_broadcast(&flag->_cond);
    pthread_mutex_unlock(&flag->_mutex);
    return 1;
}
#endif

#ifdef CB_TEST
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

static struct cbuffer *gcbuffer;
static int rsleep = 0;
static int wsleep = 1000;

void *reader_task(void *arg)
{
    int msg = 0;
    int id = (int) arg;

    while (1)
    {
        msg = gcbuffer->get(gcbuffer, 1000);
        CBUFFER_DEBUG("reader %d read a value:%d\n", id + 1, msg);

        if (rsleep > 0)
        {
            usleep(rsleep * 1000);
        }
    }
}


void *writer_task(void *arg)
{
    int ret = 0;
    int id = (int) arg + 1;
    unsigned int msg = (id * 1000) + 1;

    while (1)
    {
        msg = ret ? msg + 1 : msg;

        if (msg >= (id + 1) * 1000)
        {
            msg = (id * 1000) + 1;
        }

        CBUFFER_DEBUG("writer %d write a value:%d\n", id, msg);
        ret = gcbuffer->put(gcbuffer, msg, 1000);

        if (wsleep > 0)
        {
            usleep(wsleep * 1000);
        }
    }
}


void usage()
{
    CBUFFER_DEBUG("+-------------------------------------------------+\n"\
           "|Circle buffer test.                              |\n"\
           "|Usage:                                           |\n"\
           "|  ./cbuffer 128  2  1  100   200                 |\n"\
           "|             /   |   \\   \\    \\                  |\n"\
           "|    buffer size  |    \\   \\    \\                 |\n"\
           "|                 |     \\   \\    \\                |\n"\
           "|   reader task count    \\   \\    \\               |\n"\
           "|                         \\   \\    \\              |\n"\
           "|           writter task cout  \\    \\             |\n"\
           "|                               \\    \\            |\n"\
           "|                    reader sleep ms  \\           |\n"\
           "|                                      \\          |\n"\
           "|                             writter sleep ms    |\n"\
           "+-------------------------------------------------+\n");
}

int main(int argc, char *argv[])
{
    int rcnt = 1, wcnt = 1;
    int size = 256;
    int i = 0;
    pthread_t pth;
    usage();

    if (argc >= 6)
    {
        size = atoi(argv[1]);
        rcnt = atoi(argv[2]);
        wcnt = atoi(argv[3]);
        rsleep = atoi(argv[4]);
        wsleep = atoi(argv[5]);
    }

    /////////////////////////////////////////////////////
    CBUFFER_DEBUG("Buffer Size:%d\n", size);
    CBUFFER_DEBUG("Readers:\n"\
           "   Count  : %d\n"\
           "   Sleep  : %dms\n"\
           "   Timeout: %dms\n", rcnt, rsleep, 1000);
    CBUFFER_DEBUG("Writters:\n"\
           "   Count  : %d\n"\
           "   Sleep  : %dms\n"\
           "   Timeout: %dms\n", wcnt, wsleep, 1000);
    CBUFFER_DEBUG("===================================================\n");
    /////////////////////////////////////////////////////
    gcbuffer = cbuffer_create(size, 1, 0);

    if (gcbuffer == NULL)
    {
        return 0;
    }

    for (i = 0; i < wcnt; i++)
    {
        pthread_create(&pth, NULL, writer_task,  (void *)i);
        CBUFFER_DEBUG("**writer %d has been create ...\n", i + 1);
    }

    for (i = 0; i < rcnt; i++)
    {
        pthread_create(&pth, NULL, reader_task, (void *)i);
        CBUFFER_DEBUG("**reader %d has been create ...\n", i + 1);
    }

    while (1)
    {
        sleep(100);
    }
}
#endif
