#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>
#include "../seeAlsaSimulator.h"

#include <linux/interrupt.h>
#include "../s39-pcm.h"
#include "../AUD_common.h"
#include "../AUD_reg.h"
/* kernel thread for ali pe */
struct task_struct *see_alsa_thread;
static int see_alsa_thread_live = 0;
char *g_alsa_buf;
#define SEE_ALSA_BUFFER_SIZE 1152*256
#define MIN_BYTES_READ             4096
#define MAX_BYTES_READ            81920
char tempBuff[MAX_BYTES_READ];

#define TXJPRINT KERN_ERR
#define IN_LINUX 1
#if 1
/*
 * some ring data for
 * the bufferisation
 *
 */
static unsigned int s_audioPointer=0;
static unsigned long RING_SIZE;
static int   s_pcm_output_trd;   // when decoded pcm size >= this value, we can  output the pcm
static int   s_pcm_remain_trd;   // when pcm buffer remain size < this vaule, pause audio decoder
static char                *ring;
static unsigned int        m_see_alsa_ring_read_pos;
static unsigned int        m_see_alsa_ring_write_pos;
static unsigned int s_AudioRingDataReceived=0;
static unsigned int s_AudioRingDataConsumed=0;
#ifdef IN_LINUX
#undef libc_printf
#define libc_printf printk

static struct semaphore see_alsa_ring_sema_id; // semaphore
#define ENTER_RING_BUF()    do                                              \
                            {                                               \
                                    down(&see_alsa_ring_sema_id); \
                            }while(0)
#define LEAVE_RING_BUF()    do                                              \
                            {                                               \
                                    up(&see_alsa_ring_sema_id);   \
                            }while(0)
                            
#else
static OSAL_ID see_alsa_ring_sema_id = OSAL_INVALID_ID;
#define ENTER_RING_BUF()    do                                              \
                            {                                               \
                                if(see_alsa_ring_sema_id != OSAL_INVALID_ID)         \
                                    osal_semaphore_capture(see_alsa_ring_sema_id,OSAL_WAIT_FOREVER_TIME); \
                            }while(0)
#define LEAVE_RING_BUF()    do                                              \
                            {                                               \
                                if(see_alsa_ring_sema_id != OSAL_INVALID_ID)         \
                                    osal_semaphore_release(see_alsa_ring_sema_id);   \
                            }while(0)
#endif                            

/*********************************************************************
 *                            FUNCTIONS                              *
 *********************************************************************/
#ifdef IN_LINUX
void see_alsa_ring_init(char *buf, unsigned long size)
{
   sema_init(&see_alsa_ring_sema_id, 1);
    ring = buf;
    RING_SIZE = size;
}
#else
void see_alsa_ring_init(char *buf, unsigned long size)
{
    if (OSAL_INVALID_ID == see_alsa_ring_sema_id)
    {
        see_alsa_ring_sema_id = osal_semaphore_create(1);
        if (OSAL_INVALID_ID == see_alsa_ring_sema_id)
        {
            //MPEG4_PRINTF("Create see_alsa_ring_sema_id failed!\n");
            ASSERT(0);
        }
    }
    ring = buf;
    RING_SIZE = size;
}
#endif
static void see_alsa_ring_set_empty(void)
{
    m_see_alsa_ring_read_pos  = 0xffffffff;
    m_see_alsa_ring_write_pos = 0;
}

static int see_alsa_ring_is_empty(void)
{
    return (m_see_alsa_ring_read_pos == 0xffffffff);
}

static int see_alsa_ring_is_full(void)
{
    return (m_see_alsa_ring_read_pos == m_see_alsa_ring_write_pos);
}

void see_alsa_ring_reset()
{
    ENTER_RING_BUF();
    see_alsa_ring_set_empty();
    LEAVE_RING_BUF();
}

int see_alsa_ring_left_size()
{
    int size;
    ENTER_RING_BUF();

    if (see_alsa_ring_is_empty())
    {
        size = 0;
    }
    else if (see_alsa_ring_is_full())
    {
        size = RING_SIZE;
    }
    else if (m_see_alsa_ring_write_pos < m_see_alsa_ring_read_pos)
    {
        size = RING_SIZE - m_see_alsa_ring_read_pos;
        size += m_see_alsa_ring_write_pos;
    }
    else
    {
        size = m_see_alsa_ring_write_pos - m_see_alsa_ring_read_pos;
    }
    LEAVE_RING_BUF();
    return (RING_SIZE - size);
}

int see_alsa_ring_used_size()
{
    return (RING_SIZE) - see_alsa_ring_left_size();
}

unsigned int see_alsa_ring_size()
{
    return RING_SIZE;
}

int see_alsa_ring_read(char *data, int size)
{
    int real_read = size;

    ENTER_RING_BUF();
    if (see_alsa_ring_is_empty())
    {
        LEAVE_RING_BUF();
        return 0;
    }

    if(m_see_alsa_ring_write_pos <= m_see_alsa_ring_read_pos)
    {
        if(m_see_alsa_ring_read_pos + size <= RING_SIZE)
        {
            memcpy(data, ring + m_see_alsa_ring_read_pos, size);
            m_see_alsa_ring_read_pos += size;
        }
        else
        {
            if (m_see_alsa_ring_read_pos + size <= RING_SIZE + m_see_alsa_ring_write_pos)
            {
                unsigned int before, after;

                before = (RING_SIZE) - m_see_alsa_ring_read_pos;
                after = size - before;

                memcpy(data, ring + m_see_alsa_ring_read_pos, before);
                memcpy(data + before, ring, after);
                m_see_alsa_ring_read_pos = after;
            }
            else
            {
                real_read = 0;
            }
        }
    }
    else
    {
        if(m_see_alsa_ring_read_pos + size <= m_see_alsa_ring_write_pos)
        {
            memcpy(data, ring + m_see_alsa_ring_read_pos, size);
            m_see_alsa_ring_read_pos += size;
        }
        else
            real_read = 0;
    } 
    if (m_see_alsa_ring_read_pos == RING_SIZE)
        m_see_alsa_ring_read_pos = 0;
    if (m_see_alsa_ring_read_pos == m_see_alsa_ring_write_pos)
        see_alsa_ring_set_empty();
    //DBG_Printf("ring read: m_see_alsa_ring_read_pos = %d, m_see_alsa_ring_write_pos = %d\n", m_see_alsa_ring_read_pos, m_see_alsa_ring_write_pos);
    LEAVE_RING_BUF();
    s_AudioRingDataConsumed=s_AudioRingDataConsumed+real_read;
    //libc_printf("!!!s_AudioRingDataConsumed is %d!!!\n",s_AudioRingDataConsumed);
    return real_read;
}

void see_alsa_ring_write(char *data, int size)
{
    if (size <= 0 || see_alsa_ring_left_size() < size)
    {

        libc_printf("ring write overflow<%d,%d>\n",see_alsa_ring_left_size(), size);
        return;
    }

    ENTER_RING_BUF();
    if(m_see_alsa_ring_write_pos > m_see_alsa_ring_read_pos)
    {
        if(m_see_alsa_ring_write_pos + size <= RING_SIZE)
        {
            memcpy(ring + m_see_alsa_ring_write_pos, data, size);
            m_see_alsa_ring_write_pos += size;
        }
        else
        {
            if(m_see_alsa_ring_write_pos + size <= RING_SIZE + m_see_alsa_ring_read_pos)
            {
                unsigned int before, after;
                
                before = (RING_SIZE) - m_see_alsa_ring_write_pos;
                after = size - before;
                
                memcpy(ring + m_see_alsa_ring_write_pos, data, before);
                memcpy(ring, data + before, after);
                
                m_see_alsa_ring_write_pos = after;
            }
        }
    }
    else
    {
        if(m_see_alsa_ring_write_pos + size <= m_see_alsa_ring_read_pos)
        {
            memcpy(ring + m_see_alsa_ring_write_pos, data, size);
            m_see_alsa_ring_write_pos += size;
            if(m_see_alsa_ring_read_pos == 0xffffffff)
                m_see_alsa_ring_read_pos = 0;
        }
    }
    if (m_see_alsa_ring_write_pos == RING_SIZE)
        m_see_alsa_ring_write_pos = 0;
    //DBG_Printf("ring write: m_see_alsa_ring_read_pos = %d, m_see_alsa_ring_write_pos = %d\n", m_see_alsa_ring_read_pos, m_see_alsa_ring_write_pos);
    LEAVE_RING_BUF();
    s_AudioRingDataReceived=s_AudioRingDataReceived+size;
   // libc_printf("!!!s_AudioRingDataReceived is %d!!!\n",s_AudioRingDataReceived);


}

int see_alsa_ring_full(int size)
{
    int ret;
    ENTER_RING_BUF();
    ret = see_alsa_ring_is_full();
    LEAVE_RING_BUF();
    return ret;
}

void see_alsa_ring_get_cur_pos(unsigned int *read_pos, unsigned int *write_pos)
{
    ENTER_RING_BUF();
    *read_pos = m_see_alsa_ring_read_pos;
    *write_pos = m_see_alsa_ring_write_pos; 
    LEAVE_RING_BUF();
}

int see_alsa_ring_output_trd_get()
{
   return  s_pcm_output_trd;
}

void see_alsa_ring_output_trd_set(int trd_value)
{
   s_pcm_output_trd=trd_value;
}

int see_alsa_ring_remain_trd_get()
{
   return  s_pcm_remain_trd;
}

void see_alsa_ring_remain_trd_set( int  trd_value)
{
   s_pcm_remain_trd=trd_value;
}
#endif

void SEE_ALSA_PCM_Write(char *data, int size)
{
#if 0
	see_alsa_ring_write(data,size);
#endif
}

int SEE_ALSA_PCM_Pointer(void)
{
#if 0
	return s_audioPointer;
#endif
}

static int see_alsa_pointer_update(char *data, int size)
{
	int tempPos, tempByteToRead,dmaLastIndex, readCount;
	tempPos =16*AUDREG_GetDMACurrentIndex(AUD_SUB_OUT);
   	dmaLastIndex=16*AUDREG_GetDMALastIndex(AUD_SUB_OUT);
     if(dmaLastIndex==0)
     {
     		printk(TXJPRINT "!!!infunction %s, line %d dmaLastIndex is %d, please check!!!\n",__FUNCTION__,__LINE__,dmaLastIndex);
		return 0;
     }
     tempByteToRead=(dmaLastIndex+tempPos-s_audioPointer)%dmaLastIndex;

     if(tempByteToRead<4096)
	{
		printk(TXJPRINT "!!!infunction %s, line %d so short time to update, just return!!!\n",__FUNCTION__,__LINE__);
		return 0;
	}
     if(tempByteToRead>MAX_BYTES_READ)
	{
		printk(TXJPRINT "!!!infunction %s, line %d the bytes to read is %d, it must be wrong, just return !!!\n",__FUNCTION__,__LINE__,tempByteToRead);
		return 0;
	}
     readCount=see_alsa_ring_read(data,tempByteToRead);
     if(readCount==tempByteToRead)
		s_audioPointer=tempPos;
     else
		printk(TXJPRINT "!!!infunction %s, line %d we can not read out enough data, must check!!!\n",__FUNCTION__,__LINE__);
}
static int SEE_ALSA_Thread(void)
{
	printk(TXJPRINT "!!!infunction %s, line %d sleep and sleep!!!\n",__FUNCTION__,__LINE__);
	
	while( see_alsa_thread_live)
	{
		//printk(TXJPRINT "!!!infunction %s, line %d sleep and sleep!!!\n",__FUNCTION__,__LINE__);
		see_alsa_pointer_update(tempBuff,0);
		msleep( 50);
	}
	printk(TXJPRINT "!!!infunction %s, line %d sleep and sleep!!!\n",__FUNCTION__,__LINE__);

	return -1;
}

int SEE_ALSA_Create(void)
{
	g_alsa_buf=(char *) kmalloc(SEE_ALSA_BUFFER_SIZE, GFP_KERNEL);
     if(g_alsa_buf)
      	see_alsa_ring_init(g_alsa_buf, SEE_ALSA_BUFFER_SIZE);
     else
   		printk(TXJPRINT "!!!infunction %s, line %d we can not allocate g_alsa_buf of %d !!!\n",__FUNCTION__,__LINE__,SEE_ALSA_BUFFER_SIZE);
     see_alsa_thread_live=1;
   	printk(TXJPRINT "!!!infunction %s, line %d sleep and sleep!!!\n",__FUNCTION__,__LINE__);

	see_alsa_thread = kthread_run(
		SEE_ALSA_Thread, NULL, "ali_see_alsa");
   	printk(TXJPRINT "!!!infunction %s, line %d sleep and sleep!!!\n",__FUNCTION__,__LINE__);

}

int SEE_ALSA_Stop(void)
{
	see_alsa_thread_live=0;
   	printk(TXJPRINT "!!!infunction %s, line %d sleep and sleep!!!\n",__FUNCTION__,__LINE__);

	kthread_stop(see_alsa_thread);
   	printk(TXJPRINT "!!!infunction %s, line %d sleep and sleep!!!\n",__FUNCTION__,__LINE__);

}

