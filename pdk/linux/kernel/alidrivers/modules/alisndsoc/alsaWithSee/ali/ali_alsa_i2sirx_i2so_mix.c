#include <linux/syscalls.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <linux/ali_rpc.h>
#include "i2sirx_see2main_ring.h"
#include "i2so_see2main_ring.h"
#include "i2sirx_i2so_mix_ring.h"
#include "ali_alsa_dbg.h"

#define SEE2MAIN_RING_RD_BUF_LEN (32 * 1024)
#define I2SIRX_I2SO_MIX_BUF_LEN (128 * 1024)

/* 10 ms
*/
#define I2SIRX_I2SO_MIX_TIME_INTERVAL 10

enum I2SIRX_I2SO_MIX_STATE
{
    I2SIRX_I2SO_MIX_STATE_STOP,
    I2SIRX_I2SO_MIX_STATE_RUN
};


struct i2sirx_i2so_mix_info
{
    /* Capture info
    */
    enum I2SIRX_I2SO_MIX_STATE state;
    
    /* Engine
    */
    struct timer_list timer;
    
    /* Running ALSA substream
    */
    struct snd_pcm_substream *capture_subs;

    /* Unit: byte
    */
    __u32 period_bytes;     

    struct task_struct *mix_task;  

    uint64_t open_jiffies;

    uint64_t i2so_see2main_1st_jiffies;
};


static struct i2sirx_i2so_mix_info i2sirx_i2so_mix_info;
static __u32 i2sirx_i2so_mix_buf[I2SIRX_I2SO_MIX_BUF_LEN / 4];
static __u32 i2so_see2main_read_buf[SEE2MAIN_RING_RD_BUF_LEN / 4];
static __u32 i2sirx_see2main_read_buf[SEE2MAIN_RING_RD_BUF_LEN / 4];


/*share buffer addr for alsa capture from see*/
extern unsigned long __G_ALI_MM_ALSA_I2SIRX_I2SO_MIX_MEM_SIZE; 

/*share buffer size for alsa capture from see*/
extern unsigned long __G_ALI_MM_ALSA_I2SIRX_I2SO_MIX_START_ADDR;  


/* Identify hardware capture capabilities.
 * Note: We are not use HW buffer directly, but we see ALSA see2main buffer
 * as an emulation.
 */
static const struct snd_pcm_hardware i2sirx_i2so_mix_hwparams =
{
    .info = SNDRV_PCM_INFO_INTERLEAVED |
            SNDRV_PCM_INFO_BLOCK_TRANSFER,
            
    .formats = SNDRV_PCM_FMTBIT_S32_LE,

    .rates = SNDRV_PCM_RATE_44100,
    
    .channels_min = 1,
    .channels_max = 2,

    /* Defines the minimum period size in bytes.
     * (Writing an alsa drivrer, P28.)
     */ 
    .period_bytes_min = PAGE_SIZE * 4,//I2SIRX_I2SO_MIX_BUF_LEN / 16,

    /* Defines the maximum period size in bytes.
     * (Writing an alsa drivrer, P28.)
     */     
    .period_bytes_max = PAGE_SIZE * 8,

    /* Defines the minimum number of periods in the buffer.
     * (Writing an alsa drivrer, P28.)
     */     
    .periods_min = 2,

    /* Defines the maximum number of periods in ALSA buffer.
     * (Writing an alsa drivrer, P28.)
     * Since snd_pcm_period_elapsed() was called from SEE, we support at most
     * 10ms interval for each period to reduce the frequecy of calling to RPC.
     * Also we only support 48K samplerate, singed 16 bits precison, 
     * little endian to reduce the complexity of mixing with APE/DVB sound.
     * Thus the data rate for each ms:
     * (16/2)*48K/1000ms=96 bytes/ms
     * For now we set the max period number for alsa buffer to:
     */ 
    .periods_max = 100,

    /* Defines the maximum buffer size in bytes.
     * (Writing an alsa drivrer, P28.)
     */
    .buffer_bytes_max = I2SIRX_I2SO_MIX_BUF_LEN,
};



/* Simple mix, mix i2so data and i2sirx data to PCM,signed 32 bits,little edian,
 * 44100Hz,stero.
 * TODO: need more advance PCM mix library. 
 * We do not care sample rate  conversion for now, this should be covered by
 * the coming PCM mix library.
*/
static __s32 i2sirx_i2so_mix
(
    //struct i2sirx_i2so_mix_info *mix
    __s32 *i2so_buf,
    __u32 i2so_len,
    __s16 *i2si_buf,
    __u32 i2si_len,  
    __s32 *mix_buf
)
{
    __s32 i;
    __s32 i2si_data;
    __s32 i2so_data;   
    __u32 i2so_idx;
    __u32 i2si_idx;    
    __u32 mix_idx;             

    i2so_idx = 0;
    i2si_idx = 0;
    mix_idx = 0;
      
    /* 2, Mix them.
    */
    for (;;)
    {           
        /* Loop over?
        */
        if ((i2so_idx >= (i2so_len / sizeof(__s32))) && 
            (i2si_idx >= (i2si_len / sizeof(__s16))))
        {                
            return(mix_idx * sizeof(__s32));
        }       
        
        i2si_data = 0;

        /* i2si_data format:PCM,signed 16 bits,little edian,48000Hz,mono,
         */
        if (i2si_idx < (i2si_len / sizeof(__s16)))
        {  
            i2si_data = (i2si_buf[i2si_idx++] << 16);
        }
        
        /* i2so_data format:PCM,signed 32 bits,little edian,44100Hz,stero, 
         */
        for (i = 0; i < 2; i++)
        {
            i2so_data = 0;         

            /* i2si_data format:PCM,signed 16 bits,little edian,48000Hz,mono,
             */
            if (i2so_idx < (i2so_len / sizeof(__s32)))
            {  
                i2so_data = i2so_buf[i2so_idx++];
            }   
            
            mix_buf[mix_idx++] = (i2si_data / 2) + (i2so_data / 2); 
        }
    }

    return(mix_idx * sizeof(__s32));
}



__s32 i2sirx_i2so_mix_engine
(
    void *param
)
{
    __u32                        mix_len;
    __u32                        i2so_len;    
    __u32                        i2sirx_len;  
    __u32                        mix_free;  
    uint32_t                     drop_ms;
    uint32_t                     align_ms;
    struct i2sirx_i2so_mix_info *mix;

    mix = &i2sirx_i2so_mix_info;
    
    for (;;)
    {
        if (kthread_should_stop())
        {
            printk("%s,%d,now exit.\n", __FUNCTION__, __LINE__);

            return(-__LINE__);
        }
        
        if (I2SIRX_I2SO_MIX_STATE_RUN != mix->state)
        {
            msleep_interruptible(40);

            continue;
        }
     
        mix_free = snd_pcm_capture_hw_avail(mix->capture_subs->runtime);
        mix_free = frames_to_bytes(mix->capture_subs->runtime, mix_free);

        if (mix_free < mix->period_bytes)
        {
            msleep_interruptible(40);
            continue;
        }

        i2so_len = i2so_see2main_ring_avail();
        if (i2so_len < mix->period_bytes)
        {
            msleep_interruptible(40);
            continue;
        }   

        i2sirx_len = i2sirx_see2main_ring_avail();
        if (i2sirx_len < mix->period_bytes / 4)
        {
            msleep_interruptible(40);
            continue;
        }

        /* Drop first 500ms data to march up with kalaok pictures.
         * Drop duration(in ms) could be customized by user space.
        */
        drop_ms = ali_alsa_i2sirx_i2so_mix_drop_ms_get();
        if (time_before(jiffies, mix->open_jiffies + drop_ms))
        {
            i2so_see2main_ring_read(i2so_see2main_read_buf, mix->period_bytes);
            i2sirx_see2main_ring_read(i2sirx_see2main_read_buf,
                mix->period_bytes / 4);
            continue;
        }   

        /* Compensate SPO PCM delay caused by SEE audio buffer. 
         * Align duration(in ms) could be customized by user space.
        */
        if (0 == mix->i2so_see2main_1st_jiffies)
        {
            mix->i2so_see2main_1st_jiffies = jiffies;         
        }

        align_ms = ali_alsa_i2sirx_i2so_mix_align_ms_get();
        
        if (time_after(jiffies, mix->i2so_see2main_1st_jiffies + align_ms))
        {
            i2so_see2main_ring_read(i2so_see2main_read_buf, mix->period_bytes);
    
            /* For debug.
            */
            if (1 == ali_alsa_i2so_see2main_dump_en_get())
            {
                ali_alsa_dump_data("/data/data/ali_alsa_i2so_see2main_dump.pcm",
                    i2so_see2main_read_buf, mix->period_bytes);     
            }
        }
        else
        {
            memset(i2so_see2main_read_buf, 0, mix->period_bytes);
        }

        /* Read data from MIC, prepair for mixing.
        */
        i2sirx_len = i2sirx_see2main_ring_read(i2sirx_see2main_read_buf,
            mix->period_bytes / 4);

        /* For debug.
        */
        if (1 == ali_alsa_i2sirx_see2main_dump_en_get())
        {
            ali_alsa_dump_data("/data/data/ali_alsa_i2sirx_see2main_dump.pcm",
                i2sirx_see2main_read_buf, i2sirx_len);    
        }

		/* Mix it.
		 * SPO: 44.1K, 32 bit, LE;
		 * MIC: 44.1K, 16 bit, LE.
		*/
        i2sirx_i2so_mix(i2so_see2main_read_buf, mix->period_bytes,
            i2sirx_see2main_read_buf, i2sirx_len, i2sirx_i2so_mix_buf);  

        /* For debug.
        */
        if (1 == ali_alsa_i2sirx_i2so_mix_dump_en_get())
        {
            ali_alsa_dump_data("/data/data/ali_alsa_i2sirx_i2so_mix_dump.pcm",
                i2sirx_i2so_mix_buf, mix->period_bytes);          
        }
        
        i2sirx_i2so_mix_ring_write(i2sirx_i2so_mix_buf, mix->period_bytes);
        
        snd_pcm_period_elapsed(mix->capture_subs);
    }

    return(0);
}


static void i2sirx_i2so_mix_start
(
    struct snd_pcm_substream *substream
)
{
    int                          ret;
    struct i2sirx_i2so_mix_info *mix;
    
    printk("%s,%d\n", __FUNCTION__, __LINE__);

    //dump_stack();

    mix = (struct i2sirx_i2so_mix_info *)substream->runtime->private_data;
    
    if (SNDRV_PCM_STREAM_PLAYBACK == substream->stream)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
    else
    {    
        mix->state = I2SIRX_I2SO_MIX_STATE_RUN;
        i2sirx_see2main_ring_onoff(1);
        
        i2so_see2main_ring_onoff(1);        
    }

    return;
}


static void i2sirx_i2so_mix_stop
(
    struct snd_pcm_substream *substream
)
{
    int                          ret;
    struct i2sirx_i2so_mix_info *mix;
    
    printk("%s,%d\n", __FUNCTION__, __LINE__);

    //dump_stack();
    
    mix = (struct i2sirx_i2so_mix_info *)substream->runtime->private_data;
    
    if (SNDRV_PCM_STREAM_PLAYBACK == substream->stream)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
    else
    {    
        i2sirx_see2main_ring_onoff(0);
        i2so_see2main_ring_onoff(0);
        mix->state = I2SIRX_I2SO_MIX_STATE_STOP;
    }

    return;
}


static int i2sirx_i2so_mix_hw_params
(
    struct snd_pcm_substream *substream,
    struct snd_pcm_hw_params *params
)
{  
    __s32                        ret;
    struct i2sirx_i2so_mix_info *mix;

    //printk("%s,%d\n", __FUNCTION__, __LINE__);
    
    ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(params));
    if (ret < 0)
    {
        return(ret);
    }

    mix = substream->runtime->private_data;
    
    if (SNDRV_PCM_STREAM_PLAYBACK == substream->stream)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
    else
    {
        printk("%s,%d,area:0x%x,size:%d\n", __FUNCTION__, __LINE__,
            substream->dma_buffer.area, params_buffer_bytes(params));
        
        i2sirx_i2so_mix_ring_init(substream->dma_buffer.area, 
            params_buffer_bytes(params));

        mix->period_bytes = params_period_bytes(params);

        printk("%s,%d,buf len:%d,period_bytes:%d,stop_threshold:%d\n",
            __FUNCTION__,__LINE__, params_buffer_bytes(params), 
            mix->period_bytes, substream->runtime->stop_threshold);     
    }
    
    return(0);
}


/* Release all dynamiclly allocated resources.
*/
static int i2sirx_i2so_mix_hw_free
(
    struct snd_pcm_substream *substream
)
{
    struct i2sirx_i2so_mix_info *mix;
    
    printk("%s,%d\n", __FUNCTION__, __LINE__);

    mix = substream->runtime->private_data;
    
    snd_pcm_set_runtime_buffer(substream, NULL);

    //printk("%s,%d\n", __FUNCTION__, __LINE__);

    return(0);
}



static int i2sirx_i2so_mix_trigger
(
    struct snd_pcm_substream *substream,
    int                       cmd
)
{
    //printk("%s,%d\n", __FUNCTION__, __LINE__);
    
    switch (cmd) 
    {
        case SNDRV_PCM_TRIGGER_START:
        case SNDRV_PCM_TRIGGER_RESUME:
        case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
            i2sirx_i2so_mix_start(substream);
            break;
        case SNDRV_PCM_TRIGGER_STOP:
        case SNDRV_PCM_TRIGGER_SUSPEND:
        case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
            i2sirx_i2so_mix_stop(substream);
            break;
        default:
            break;
    }

    return(0);
}




/* We get ALSA buffer read/write pointer direcly from allocated share memory,
 * this way we can eliminate the using of RPCs to SEE to get current ALSA 
 * buffer HW pointer, hence significantly reduce the heavy system loading caused
 * by RPC.
 * runtime->control.appl_ptr; //unit:frame
 * runtime->status->hw_ptr;//unit:frame
 * runtime->dma_area
 * runtime->buffer_size//unit:byte 
 */
static snd_pcm_uframes_t i2sirx_i2so_mix_pointer
(
    struct snd_pcm_substream *substream
)
{
    snd_pcm_uframes_t       offset;  
    struct snd_pcm_runtime *runtime; 
    
    //printk("%s,%d\n", __FUNCTION__, __LINE__);
    
    runtime = substream->runtime;

    offset = 0;
        
    if (SNDRV_PCM_STREAM_PLAYBACK == substream->stream)
    {
        //offset = bytes_to_frames(runtime, ali_alsa_tx_buf_rd_get(mix));
    }
    else
    {
        offset = bytes_to_frames(runtime, i2sirx_i2so_mix_ring_wr_get());
    }

    return(offset); 
}



static int i2sirx_i2so_mix_prepare
(
    struct snd_pcm_substream *substream
)
{
    printk("%s,%d\n", __FUNCTION__, __LINE__);
    
    return(0);
}


static int i2sirx_i2so_mix_open
(
    struct snd_pcm_substream *substream
)
{    
    uint8_t  *i2so_see2main_ring_addr;
    uint32_t  i2so_see2main_ring_len;
    uint8_t  *i2sirx_see2main_ring_addr;
    uint32_t  i2sirx_see2main_ring_len;
    
    printk("%s,%d\n", __FUNCTION__, __LINE__);
    
    if (SNDRV_PCM_STREAM_PLAYBACK == substream->stream)
    {   
        printk("%s,%d\n", __FUNCTION__, __LINE__);    
    }
    else
    {
        snd_soc_set_runtime_hwparams(substream, &i2sirx_i2so_mix_hwparams);
    }

    i2sirx_i2so_mix_info.open_jiffies = jiffies;
    i2sirx_i2so_mix_info.i2so_see2main_1st_jiffies = 0;

    substream->runtime->private_data = &i2sirx_i2so_mix_info;

    i2so_see2main_ring_addr = __G_ALI_MM_ALSA_I2SIRX_I2SO_MIX_START_ADDR;
    i2so_see2main_ring_len = 1024 * 1024 + sizeof(struct i2so_see2main_ring_info);
    printk("%s,%d,addr:0x%x,len:%d,__G_ALI_MM_ALSA_I2SIRX_I2SO_MIX_MEM_SIZE:%d\n",
        __FUNCTION__, __LINE__, i2so_see2main_ring_addr,
        i2so_see2main_ring_len, __G_ALI_MM_ALSA_I2SIRX_I2SO_MIX_MEM_SIZE);
    i2so_see2main_ring_init(i2so_see2main_ring_addr, i2so_see2main_ring_len);   

    i2sirx_see2main_ring_addr = i2so_see2main_ring_addr + i2so_see2main_ring_len;
    i2sirx_see2main_ring_len = __G_ALI_MM_ALSA_I2SIRX_I2SO_MIX_MEM_SIZE - i2so_see2main_ring_len;
    printk("%s,%d,addr:0x%x,len:%d,__G_ALI_MM_ALSA_I2SIRX_I2SO_MIX_MEM_SIZE:%d\n",
        __FUNCTION__, __LINE__, i2sirx_see2main_ring_addr,
        i2sirx_see2main_ring_len, __G_ALI_MM_ALSA_I2SIRX_I2SO_MIX_MEM_SIZE);
    i2sirx_see2main_ring_init(i2sirx_see2main_ring_addr, i2sirx_see2main_ring_len); 

    printk("%s,%d\n", __FUNCTION__, __LINE__);  
    
    return(0);
}


static int i2sirx_i2so_mix_close
(
    struct snd_pcm_substream *substream
)
{
    printk("%s,%d\n", __FUNCTION__, __LINE__);
    
    /* Do nothing since there is no resource dynamiclly allocated by
     * i2sirx_i2so_mix_open().
     */
    return(0);
}




static int i2sirx_i2so_mix_copy
(
    struct snd_pcm_substream *substream,
    int channel,
    snd_pcm_uframes_t pos,
    void __user *buf,
    snd_pcm_uframes_t frames
)
{
    __u8 *read_src;
    __u32 size_bytes;
    __u32 offset_bytes;
    
    struct snd_pcm_runtime *runtime = substream->runtime;
        
    if (SNDRV_PCM_STREAM_PLAYBACK == substream->stream)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
    else
    {               
        offset_bytes = frames_to_bytes(runtime, pos);
        read_src = i2sirx_i2so_mix_ring_base_get() + offset_bytes;
        size_bytes = frames_to_bytes(runtime, frames);
        
        /* For debug.
        */
        if (1 == ali_alsa_i2sirx_i2so_mix_out_dump_en_get())
        {
            ali_alsa_dump_data("/data/data/ali_alsa_mix_out_dump.pcm", read_src,
                size_bytes);
        }
        
        if (copy_to_user(buf, read_src, size_bytes))
        {
            printk("%s,%d\n", __FUNCTION__, __LINE__);  
        
            return(-EFAULT);        
        }
    }
    
    return 0;
}




/* Delink pre-defined alsa main2see buffer(playback) and alsa see2main buffer
 * (capture) to substream dma_buffer.
 */
static void i2sirx_i2so_mix_free
(
    struct snd_pcm *pcm
)
{
    struct i2sirx_i2so_mix_info *mix;
    struct snd_pcm_substream    *substream;
    struct snd_dma_buffer       *buf;
    __s32                        stream;
    __s32                        ret;

    printk("%s,%d\n", __FUNCTION__, __LINE__);
   
    mix = &i2sirx_i2so_mix_info;
    if (!IS_ERR(mix->mix_task))
    { 
        ret = kthread_stop(mix->mix_task); 
        
        printk("%s,%d,kthread_stop() failed,ret:%d.\n", __FUNCTION__, __LINE__,
            ret);
    }   
    
    /* Free alsa buffers.
    */
    for (stream = 0; stream < SNDRV_PCM_STREAM_LAST; ++stream) 
    {
        substream = pcm->streams[stream].substream;
        if (!substream)
            continue;

        buf = &substream->dma_buffer;
        if (!buf->area)
            continue;

        kfree(buf->area);
        
        buf->area = NULL;
    }   

    return;
}





static int i2sirx_i2so_mix_preallocate_buffer
(
    struct snd_pcm *pcm,
    int stream
)
{
    struct snd_pcm_substream *substream = pcm->streams[stream].substream;
    struct snd_dma_buffer *buf = &substream->dma_buffer;
    size_t size = i2sirx_i2so_mix_hwparams.buffer_bytes_max;

    buf->dev.type = SNDRV_DMA_TYPE_CONTINUOUS;
    buf->dev.dev = pcm->card->dev;
    buf->private_data = NULL;

    buf->area = kmalloc(size, GFP_KERNEL);
    
    if (!buf->area)
    {
        printk("%s,%d,kmalloc failed, len:%d\n", __FUNCTION__, __LINE__, size);
        return(-ENOMEM);
    }

    buf->bytes = size;

    return 0;
}




/* Link pre-defined alsa main2see buffer(playback) and alsa see2main buffer
 * (capture) to substream->dma_buffer.
 */
static int i2sirx_i2so_mix_new
(
    struct snd_soc_pcm_runtime *rtd
)
{
    struct snd_pcm_substream    *substream;
    struct snd_card             *card;
    struct snd_pcm              *pcm;
    __s32                        ret;
    struct i2sirx_i2so_mix_info *mix;
    
    printk("%s,%d\n", __FUNCTION__, __LINE__);
    
    pcm = rtd->pcm;
    card = pcm->card;

    mix = &i2sirx_i2so_mix_info;
    
    substream = pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream;
    if (NULL != substream)
    {
        i2sirx_i2so_mix_preallocate_buffer(pcm, SNDRV_PCM_STREAM_CAPTURE);
        mix->capture_subs = substream;

        /* Init mix task.
        */
        mix->state = I2SIRX_I2SO_MIX_STATE_STOP;
        mix->mix_task = kthread_create(i2sirx_i2so_mix_engine, (void *)NULL,
                                               "ali_alsa_mix_test");
    
        if (IS_ERR(mix->mix_task))
        {
            return(PTR_ERR(mix->mix_task)); 
        }
    
        //sched_setscheduler(i2sirx_i2so_mix_engine, SCHED_RR, &param);
    
        wake_up_process(mix->mix_task);
    }
    
    return(0);
}


static struct snd_pcm_ops asoc_i2sirx_i2so_mix_ops = {
     .open = i2sirx_i2so_mix_open,
     .close = i2sirx_i2so_mix_close,
     .ioctl = snd_pcm_lib_ioctl,
     .hw_params  = i2sirx_i2so_mix_hw_params,
     .hw_free = i2sirx_i2so_mix_hw_free,
     .prepare = i2sirx_i2so_mix_prepare,
     .trigger = i2sirx_i2so_mix_trigger,
     .pointer = i2sirx_i2so_mix_pointer,
     .copy = i2sirx_i2so_mix_copy,
};


static struct snd_soc_platform_driver asoc_i2sirx_i2so_mix_driver = {
    .ops        = &asoc_i2sirx_i2so_mix_ops,
    .pcm_new    = i2sirx_i2so_mix_new,
    .pcm_free   = i2sirx_i2so_mix_free,
};


static int __devinit i2sirx_i2so_mix_dev_probe(struct platform_device *pdev)
{
    printk("%s,%d\n", __FUNCTION__,__LINE__);

    return snd_soc_register_platform(&pdev->dev, &asoc_i2sirx_i2so_mix_driver);
}

static int __devexit i2sirx_i2so_mix_dev_remove(struct platform_device *pdev)
{
    printk("%s,%d\n", __FUNCTION__,__LINE__);

    snd_soc_unregister_platform(&pdev->dev);
    
    return 0;
}

static struct platform_driver platform_i2sirx_i2so_mix_driver = 
{
    .driver = 
	{
        .name = "ali_asoc_i2sirx_i2so_mix_cap",
        .owner = THIS_MODULE,
    },

    .probe = i2sirx_i2so_mix_dev_probe,
    .remove = __devexit_p(i2sirx_i2so_mix_dev_remove),
};

module_platform_driver(platform_i2sirx_i2so_mix_driver);


MODULE_AUTHOR("Jinang Chu");
MODULE_DESCRIPTION("ALI ALSA: CAPTURE I2SI RX AND I2SO DATA MIXED AT SEE");
MODULE_LICENSE("GPL");
//MODULE_SUPPORTED_DEVICE("{{s39XX Series SOC}}");





