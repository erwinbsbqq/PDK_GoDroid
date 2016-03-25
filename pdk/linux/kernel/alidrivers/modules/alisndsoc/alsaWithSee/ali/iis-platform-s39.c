

/***************************************************************************************************
*
* To understand what Alsa Drivers should be doing look at "Writing an Alsa Driver" by Takashi Iwai
* available in the Alsa doc section on the website		
* 
* A few notes to make things clearer. The UDA1341 is hooked up to Serial port 4 on the SA1100.
* We are using  SSP mode to talk to the UDA1341. The UDA1341 bit & wordselect clocks are generated
* by this UART. Unfortunately, the clock only runs if the transmit buffer has something in it.
* So, if we are just recording, we feed the transmit DMA stream a bunch of 0x0000 so that the
* transmit buffer is full and the clock keeps going. The zeroes come from FLUSH_BASE_PHYS which
* is a mem loc that always decodes to 0's w/ no off chip access.
*
* Some alsa terminology:
*	frame => num_channels * sample_size  e.g stereo 16 bit is 2 * 16 = 32 bytes
*	period => the least number of bytes that will generate an interrupt e.g. we have a 1024 byte
*             buffer and 4 periods in the runtime structure this means we'll get an int every 256
*             bytes or 4 times per buffer.
*             A number of the sizes are in frames rather than bytes, use frames_to_bytes and
*             bytes_to_frames to convert.  The easiest way to tell the units is to look at the
*             type i.e. runtime-> buffer_size is in frames and its type is snd_pcm_uframes_t
*             
*	Notes about the pointer fxn:
*	The pointer fxn needs to return the offset into the dma buffer in frames.
*	Interrupts must be blocked before calling the dma_get_pos fxn to avoid race with interrupts.
*
*	Notes about pause/resume
*	Implementing this would be complicated so it's skipped.  The problem case is:
*	A full duplex connection is going, then play is paused. At this point you need to start xmitting
*	0's to keep the record active which means you cant just freeze the dma and resume it later you'd
*	need to	save off the dma info, and restore it properly on a resume.  Yeach!
*
*	Notes about transfer methods:
*	The async write calls fail.  I probably need to implement something else to support them?
* 
***************************************************************************************************/
#include <linux/syscalls.h>

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#include <linux/types.h>
#include <sound/control.h>
#include <sound/info.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>

#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/errno.h>

//#include "../s39-pcm.h"

#include <linux/dma-mapping.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

//#define CONFIG_HDMI_ALI
//#undef CONFIG_HDMI_ALI
#ifdef CONFIG_HDMI_ALI
//#include <linux/ali_hdmi.h>
#endif

#include <linux/sched.h>
#include <linux/time.h>
#include <linux/timer.h>

#include <ali_reg.h>

//#include "../AUD_common.h"
//#include "../AUD_reg.h"
//#include "../seeAlsaSimulator.h"
//#include <asm/mach-ali/m36_gpio.h>
#include <linux/ali_rpc.h>

#include "sndrpc.h"
#include "ali_alsa_dbg.h"

#define LOG(fmt,arg...)		printk(fmt,##arg)
#define ERROR_LOG(fmt,arg...)	printk(fmt,##arg)

#define PCM_BUF_FRM_NUM		10		// 10
#define MAX_PCM_FRAME_LEN		6148	// 6148 = (1536*4+4) = 8 channel ac3 audio frame size
#define PCM_BUFF_SIZE			(MAX_PCM_FRAME_LEN*PCM_BUF_FRM_NUM)	// unit: 64 bits, 8*64 bits align
#define RAW_DATA_BUFF_SIZE          (MAX_PCM_FRAME_LEN*PCM_BUF_FRM_NUM)	// unit: 64 bits
#define S39_PERIOD_MIN_BYTES	12288	/* 12288 = 1536*8 */ 

#define M36_PCM_INTF	0
#define M36_SPDIF_INTF	1

/* Physical to virtual address and vise visa.
 */
#define ALI_PHYS2VIRT(x) (x - ALI_REGS_PHYS_BASE + ALI_REGS_VIRT_BASE)
#define ALI_VIRT2PHYS(x) (x - ALI_REGS_VIRT_BASE + ALI_REGS_PHYS_BASE)

static struct timer_list alsa_period_timer;
//static int alsa_timer_should_repeat=0;
static int alsa_playback_active = 0;
static int alsa_capture_active = 0;

//our hardware limited, we can not access high memory
unsigned long g_audio_static_buf =0;  
#define ALSA_WITH_SEE          1

/* Status. only one pcm stream is supported.
 * Added by Joy, date:2014.06.24
 */
enum ALI_ALSA_STREAM_STATUS
{
    ALI_ALSA_STREAM_STATUS_IDLE = 0,
    ALI_ALSA_STREAM_STATUS_RUN  = 1
};


extern unsigned long __G_ALI_MM_DECA_MEM_SIZE;
extern unsigned long __G_ALI_MM_DECA_MEM_START_ADDR;

unsigned int s_see_audio_buffer_address=0;
unsigned int s_see_audio_buffer_size=0;


static char *id;	/* ID for this card */

unsigned int g_buffer_size = 0;
unsigned int g_buffer_pos=0;

/* Status. only one pcm stream is supported.
 * Added by Joy, date:2014.06.24
 */
enum ALI_ALSA_STREAM_STATUS ali_alsa_stream_playback_status = ALI_ALSA_STREAM_STATUS_IDLE;
enum ALI_ALSA_STREAM_STATUS ali_alsa_stream_capture_status = ALI_ALSA_STREAM_STATUS_IDLE;


/************************************Begin:Added by kinson for alsa capture***********************************/
extern unsigned long __G_ALI_MM_ALSA_CAPTURE_MEM_SIZE;     /*share buffer addr for alsa capture from see*/
extern unsigned long __G_ALI_MM_ALSA_CAPTURE_START_ADDR;   /*share buffer size for alsa capture from see*/
static struct timer_list alsa_period_timer_capture;      /*see notify main to updata ptr will use this timer*/
static int alsa_timer_capture_should_repeat = 0;
unsigned int g_see_audio_buffer_address_capture=0;/*alsa capture wr_ptr will write to this ptr*/
static unsigned int g_see_alsa_cur_pos_capture = 0;   /*capture buffer current addr*/
static struct see_alsa_priv_struct g_see_alsa_work_queue_capture_priv;/*work queue for rpc cmd in specifial context*/
static struct workqueue_struct 	*g_see_alsa_work_queue_capture = NULL;
/************************************End:Added by kinson for alsa capture***********************************/


module_param(id, charp, 0444);
MODULE_PARM_DESC(id, "ID string for s39 Series ASOC.");
#define ALSAPRINT KERN_DEBUG

struct audio_stream {
	char *id;		/* identification string */
	int stream_id;		/* numeric identification */	
	unsigned int active:1;	/* we are using this stream for transfer now */
	int period;		/* current transfer period */
	int periods;		/* current count of periods registerd in the DMA engine */
	int tx_spin;		/* are we recoding - flag used to do DMA trans. for sync */
	unsigned int old_offset;
	spinlock_t dma_lock;	/* for locking in DMA operations (see dma-sa1100.c in the kernel) */
	struct snd_pcm_substream *stream;
	u32 *pcm_buff_base;
	u32 *es_buff_base;
};

struct s39_soc {
	struct snd_card *card;
	struct snd_pcm *pcm[2];
	long samplerate;
	struct audio_stream s[2];	/* playback & capture */


	//add by alex
	spinlock_t reg_lock;
	unsigned int dig_status;
	unsigned int dig_pcm_status;
};

struct see_alsa_priv_struct {
	unsigned int stream_id;
	unsigned int cmd;
	unsigned int dataIn;
	unsigned int *pointer;
	struct work_struct see_alsa_work;
   };

enum SEE_ALSA_COMMAND
{
	SEE_ALSA_COMMAND_POINTER =0,
	SEE_ALSA_COMMAND_DMA_START,   
	SEE_ALSA_COMMAND_DMA_STOP,
	SEE_ALSA_COMMAND_MAX,                                                    
};

static struct see_alsa_priv_struct g_see_alsa_work_queue_playback_priv;/*work queue for rpc cmd in specifial context*/
static struct workqueue_struct 	*g_see_alsa_work_queue_playback = NULL;




static unsigned int s_see_alsa_cur_pos = 0;
static unsigned int rates[] = 
{
	/* 8000, 16000, */		/* s39 internal DAC not support 8k, 16k */ 
	32000, 44100, 48000,
};

static struct snd_pcm_hw_constraint_list hw_constraints_rates = {
	.count	= ARRAY_SIZE(rates),
	.list	= rates,
	.mask	= 0,
};

/*
**for debug
*/
static void printBuff(uint8_t *ptr, int count)
{
	int i;
	for(i=0;i<count; i++)
	{
		if(i%16==0)
			printk( "\n");
		printk( "%02x,",  *(ptr+i));

	}
	printk( "\n");
}


static void alsa_timer_handler(unsigned long dev_id)
{   
	struct audio_stream *s = (struct audio_stream *)dev_id;
	struct snd_pcm_substream *substream = NULL;
	//printk("%s,%d\n", __FUNCTION__, __LINE__);

	if(!s)
	{
	    return;
	}
	
	substream = s->stream;
	if(!substream)
	{
	    return;
	}

	//printk("%s,%d\n", __FUNCTION__, __LINE__);

    if (1 == alsa_playback_active)
    {
    	//printk("%s,%d\n", __FUNCTION__, __LINE__);
    
	    snd_pcm_period_elapsed(substream);
	}

	//printk("%s,%d\n", __FUNCTION__, __LINE__);
	//if(alsa_timer_should_repeat > 0)
	{
	    mod_timer(&alsa_period_timer, jiffies + msecs_to_jiffies(10));
	    //add_timer(&alsa_period_timer);
	}

	return;

}


/*
**for debug.
* For now, dump file format:PCM,signed 32 bit, little endian, 48000hz,mono.
*/
static int KernelWriteFilePlayback(unsigned char* inAddr, const int lenData)
{
    
    int fd = -1;

    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);
	
    fd = sys_open("/data/data/ali_alsa_plb_dump.pcm", O_RDWR | O_CREAT|O_APPEND, 0777);
    if ( fd >= 0)
    {
        //printk("KernelWriteFileCapture,lenData:%d!\n", lenData);
        sys_write(fd, (unsigned char*)inAddr, lenData);

    }
    else 
    {
        printk("KernelWriteFileCapture error!\n");
        return -1;    
    }
	
    sys_close(fd);

    set_fs(old_fs);

    #if 0
	ali_alsa_playback_dump_cnt += lenData;

	if (0 == (ali_alsa_playback_dump_cnt % (4 * 1024)))
	{
        printk("%s,%d,ali_alsa_playback_dump_cnt:%d\n",
			__FUNCTION__, __LINE__, ali_alsa_playback_dump_cnt);
	}
	#endif

    return 0;
}

static int KernelWriteFileCapture(unsigned char* inAddr, const int lenData)
{
    
    int fd = -1;

    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);
	
    fd = sys_open("/data/data/ali_alsa_cap_dump.pcm", O_RDWR | O_CREAT|O_APPEND, 0777);
    if ( fd >= 0)
    {
        //printk("KernelWriteFileCapture,lenData:%d!\n", lenData);
        sys_write(fd, (unsigned char*)inAddr, lenData);

    }
    else 
    {
        printk("KernelWriteFileCapture error!\n");
        return -1;    
    }
	
    sys_close(fd);

    set_fs(old_fs);

    #if 0
	ali_alsa_capture_dump_cnt += lenData;

	if (0 == (ali_alsa_capture_dump_cnt % (4 * 1024)))
	{
        printk("%s,%d,ali_alsa_capture_dump_cnt:%d\n",
			__FUNCTION__, __LINE__, ali_alsa_capture_dump_cnt);
	}
	#endif

    return 0;
}


/*
***when see recived data from i2s-in, see will call rpc to active this timer
*/
static void alsa_timer_handler_capture(unsigned long dev_id)
{   
	struct audio_stream *s = (struct audio_stream *)dev_id;
	struct snd_pcm_substream *substream = NULL;
	//printk("%s,%d\n", __FUNCTION__, __LINE__);

	if(!s)
	{
	    return;
	}
	
	substream = s->stream;
	if(!substream)
	{
	    return;
	}

	//printk("%s,%d\n", __FUNCTION__, __LINE__);

    if (1 == alsa_capture_active)
	{
        //printk("%s,%d\n", __FUNCTION__, __LINE__);
	
	    snd_pcm_update_hw_ptr(substream);//updata the capture buffer wr_ptr
	}

	//printk("%s,%d\n", __FUNCTION__, __LINE__);

	//if(alsa_timer_capture_should_repeat > 0)
	{
	    mod_timer(&alsa_period_timer_capture, jiffies + msecs_to_jiffies(10));
	    //mod_timer(&alsa_period_timer_capture, jiffies + msecs_to_jiffies(2));
	    //mod_timer(&alsa_period_timer_capture, jiffies + msecs_to_jiffies(50));
	    //mod_timer(&alsa_period_timer_capture, jiffies + msecs_to_jiffies(300));
	    //add_timer(&alsa_period_timer_capture);
	}

	return;

}


/*************************************************************************************************************/
/************************************Begin:following are RPC Call*********************************************/
/*************************************************************************************************************/
/*
**see_iis_platform_s39_open,call rpc
*/
int see_iis_platform_s39_open(UINT8 cmd,  UINT32 para1)
{
	Int32 ret = -1;    

	RPC_PARAM_CREATE(p1, PARAM_IN,PARAM_UCHAR, sizeof(UINT8), &cmd);   
	RPC_PARAM_CREATE(p2, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &para1);   	

	//RPC_see_iis_platform_s39_open will initialize the see audio hardware, create the task and the ring buffer
	ret = RpcCallCompletion(RPC_see_iis_platform_s39_open,&p1,&p2,NULL);

	printk(ALSAPRINT "!!!infunction %s, line %d the ret is %d !!!\n",__FUNCTION__,__LINE__, ret);
	return ret;
}

/*
**see_iis_platform_s39_close, call rpc
*/
int see_iis_platform_s39_close(UINT8 stream_id,  UINT32 para1)
{
	Int32 ret = -1;  

	RPC_PARAM_CREATE(p1, PARAM_IN,PARAM_UCHAR, sizeof(UINT8), &stream_id);   
	RPC_PARAM_CREATE(p2, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &para1);  	

	//RPC_see_iis_platform_s39_close will make the see audio sink task in sleep and reset the ring buffer
	ret = RpcCallCompletion(RPC_see_iis_platform_s39_close,&p1,&p2,NULL);

	printk(ALSAPRINT "!!!infunction %s, line %d the ret is %d !!!\n",__FUNCTION__,__LINE__, ret);
	
	return ret;
}

/*
**see_iis_platform_s39_prepare_capture, call rpc
*/
int see_iis_platform_s39_prepare(UINT32 stream_id, UINT32 channels , UINT32 rate, UINT32 sample_bits, UINT32 share_buffer_addr, UINT32 run_time_buffer_size)
{
	Int32 ret; 
	
	RPC_PARAM_CREATE(p1, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &stream_id); 
	RPC_PARAM_CREATE(p2, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &channels);   
	RPC_PARAM_CREATE(p3, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &rate); 
	
	RPC_PARAM_CREATE(p4, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &sample_bits);   
	RPC_PARAM_CREATE(p5, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &share_buffer_addr);  
	RPC_PARAM_CREATE(p6, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &run_time_buffer_size);   
	
	//RPC_see_iis_platform_s39_open will configure the audio hardware and prepare to output audio
	ret = RpcCallCompletion(RPC_see_iis_platform_s39_prepare,&p1,&p2,&p3,&p4,&p5,&p6,NULL);

	printk(ALSAPRINT "!!!infunction %s, line %d stream_id:%d, channels:%d, rate:%d, sample_bits:%d, share_buffer_addr:0x%08x, run_time_buffer_size:%d,  the ret is %d !!!\n",__FUNCTION__,__LINE__, stream_id, channels , rate, sample_bits, share_buffer_addr, run_time_buffer_size, ret);
	
	return ret;
}

/*
**see_iis_platform_s39_pointer, call rpc
*/
int see_iis_platform_s39_pointer(UINT32 stream_id , UINT32 *pointer)
{
	Int32 ret = -1; 
	static unsigned int oldValue=0;
	UINT32 *temp_buf = NULL;

    if(SNDRV_PCM_STREAM_PLAYBACK == stream_id)
	{
	   temp_buf = (UINT32 *)(s_see_audio_buffer_address);
	}
    else if(SNDRV_PCM_STREAM_CAPTURE == stream_id)
	{
	   temp_buf = (UINT32 *)(g_see_audio_buffer_address_capture);
	}

	if((!pointer)||(!temp_buf))
	{
	   printk("%s,%d error!ptr is null!\n",__FUNCTION__,__LINE__);
	   return -1;
	}

	//printk("A:%x,V:%d\n", temp_buf, *temp_buf);

	*pointer=*temp_buf;
	if(oldValue!=*pointer)
	{
	   oldValue=*pointer;
	}
	return ret;
}


int see_iis_platform_s39_write_last_jiffies = 0;
int delta_bytes = 4000;

int see_iis_platform_s39_write(UINT32 *dataAddress, UINT32 count)
{
	int ret = -1;  

	RPC_PARAM_CREATE(p1, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &dataAddress);   
	RPC_PARAM_CREATE(p2, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &count);   	

	//printk("D:%d,count\n", jiffies - see_iis_platform_s39_write_last_jiffies, count);

	see_iis_platform_s39_write_last_jiffies = jiffies;
	ret = RpcCallCompletion(RPC_see_iis_platform_s39_write,&p1,&p2,NULL);
	//printk("I:%d\n", jiffies - see_iis_platform_s39_write_last_jiffies);

    #if 0
    #if 0
    if (delta_bytes >= 4000)
    {
	    printk("R,delta_bytes:%d\n", delta_bytes);    
	    msleep(40);

        delta_bytes = 0;
	}
	else
	{
        delta_bytes += count;
	}
	#else
	UINT32 delay;
	//delay = count / 96;
	delay = count / 80;
	//printk("D:%d\n", delay); 	
	msleep(delay);

	#endif
    #endif
	
	return ret;
}


/*
**see_iis_platform_s39_pointer, call rpc
*/
int see_iis_platform_s39_read(UINT32 *dataAddress, UINT32 offset, UINT32 count)
{
	int ret = 0;  

	RPC_PARAM_CREATE(p1, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &dataAddress);   
	RPC_PARAM_CREATE(p2, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &offset); 
	RPC_PARAM_CREATE(p3, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &count);   	
		
	//RPC_see_iis_platform_s39_write will write buffer to the audio buffer in see cpu 
	ret = RpcCallCompletion(RPC_see_iis_platform_s39_read,&p1,&p2,&p3,NULL);

	//if(count!=ret)
		//printk(ALSAPRINT "!!!infunction %s, line %d try to write %d the ret is %d !!!\n",__FUNCTION__,__LINE__,count, ret);
	return ret;
}

/*
***alsa Playback rpc function command queue
*/
static void rpc_see_functions_playback(struct work_struct *work)
{
	int ret = -1;
	struct see_alsa_priv_struct *priv = NULL;
	
	priv = container_of((void*)work, struct see_alsa_priv_struct, see_alsa_work); 
	if(!priv)
	{
	   printk("%s,%d error!\n", __FUNCTION__, __LINE__);
	   return ret;
	}
	
	switch (priv->cmd)
	{
		case SEE_ALSA_COMMAND_DMA_START:	
			printk(ALSAPRINT "%s,%d\n", __FUNCTION__, __LINE__);
			priv->dataIn = 1;
			RPC_PARAM_CREATE(p1, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &priv->stream_id); 
			RPC_PARAM_CREATE(p2, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &priv->cmd);   
			RPC_PARAM_CREATE(p3, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &priv->dataIn); 	
			ret = RpcCallCompletion(RPC_see_iis_platform_s39_start_dma, &p1, &p2, &p3, NULL);
			break;

		case SEE_ALSA_COMMAND_DMA_STOP: 
			printk(ALSAPRINT "%s,%d\n", __FUNCTION__, __LINE__);
			priv->dataIn = 0;
			RPC_PARAM_CREATE(p4, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &priv->stream_id); 
			RPC_PARAM_CREATE(p5, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &priv->cmd);   
			RPC_PARAM_CREATE(p6, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &priv->dataIn); 	
			ret = RpcCallCompletion(RPC_see_iis_platform_s39_start_dma, &p4, &p5, &p6, NULL);
			break;	
		default:
			break;
	}		 

	return;    

}

/*
***alsa Playback rpc function command queue
*/
static void rpc_see_functions_capture(struct work_struct *work)
{
	int ret = -1;
	struct see_alsa_priv_struct *priv = NULL;
	
	priv = container_of((void*)work, struct see_alsa_priv_struct, see_alsa_work); 
	if(!priv)
	{
	   printk("%s,%d error!\n", __FUNCTION__, __LINE__);
	   return ret;
	}
	
	switch (priv->cmd)
	{
		case SEE_ALSA_COMMAND_DMA_START:	
			printk(ALSAPRINT "%s,%d\n", __FUNCTION__, __LINE__);
			priv->dataIn = 1;
			RPC_PARAM_CREATE(p1, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &priv->stream_id); 
			RPC_PARAM_CREATE(p2, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &priv->cmd);   
			RPC_PARAM_CREATE(p3, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &priv->dataIn); 	
			ret = RpcCallCompletion(RPC_see_iis_platform_s39_start_dma, &p1, &p2, &p3, NULL);
			break;

		case SEE_ALSA_COMMAND_DMA_STOP: 
			printk(ALSAPRINT "%s,%d\n", __FUNCTION__, __LINE__);
			priv->dataIn = 0;
			RPC_PARAM_CREATE(p4, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &priv->stream_id); 
			RPC_PARAM_CREATE(p5, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &priv->cmd);   
			RPC_PARAM_CREATE(p6, PARAM_IN,PARAM_UINT32, sizeof(UINT32), &priv->dataIn); 	
			ret = RpcCallCompletion(RPC_see_iis_platform_s39_start_dma, &p4, &p5, &p6, NULL);
			break;	
		default:
			break;
	}		 

	return;    

}


static int see_iis_s39_start_dma(int stream_id, dma_addr_t dma_ptr, u_int size)
{
	int ret = -1;

    if(SNDRV_PCM_STREAM_PLAYBACK == stream_id)
	{
	   //alsa_timer_should_repeat = 1; //enable the repeat timer   
	   //mod_timer(&alsa_period_timer, jiffies + msecs_to_jiffies(10));  
    }
	else
	{
	   g_see_alsa_work_queue_capture_priv.stream_id = SNDRV_PCM_STREAM_CAPTURE;
	   g_see_alsa_work_queue_capture_priv.cmd = SEE_ALSA_COMMAND_DMA_START;

	   ret = queue_work(g_see_alsa_work_queue_capture, &g_see_alsa_work_queue_capture_priv.see_alsa_work);
	}
	
	return ret;
}


static int see_iis_s39_stop_dma(int stream_id, dma_addr_t dma_ptr, u32 size)
{
    unsigned int enable = 0;
	int ret = 0; 

	if(SNDRV_PCM_STREAM_PLAYBACK == stream_id)
	{
	   g_see_alsa_work_queue_playback_priv.stream_id = SNDRV_PCM_STREAM_PLAYBACK;
	   g_see_alsa_work_queue_playback_priv.cmd = SEE_ALSA_COMMAND_DMA_STOP;
	  
	   ret = queue_work(g_see_alsa_work_queue_playback, &g_see_alsa_work_queue_playback_priv.see_alsa_work); 
	}
    else
	{
	   g_see_alsa_work_queue_capture_priv.stream_id = SNDRV_PCM_STREAM_CAPTURE;
	   g_see_alsa_work_queue_capture_priv.cmd = SEE_ALSA_COMMAND_DMA_STOP;
	  
	   ret = queue_work(g_see_alsa_work_queue_capture, &g_see_alsa_work_queue_capture_priv.see_alsa_work); 
	}

	return ret;
}
/*************************************************************************************************************/
/************************************End:following are RPC Call*********************************************/
/*************************************************************************************************************/


/*
** this request the dma
*/
static int audio_dma_request(struct audio_stream *s, void (*callback)(void *))
{
	int ret = 0;
	printk(ALSAPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
	return ret;
}


/*
** this free the dma
*/
static void audio_dma_free(struct audio_stream *s)
{
	printk(ALSAPRINT "ALi audio_dma_free\n");
}


/*
** this get the dma pos
*/
static u_int audio_get_dma_pos(struct audio_stream *s)
{
	struct snd_pcm_substream *substream = s->stream;
	struct snd_pcm_runtime *runtime = substream->runtime;
	unsigned int offset = 0;

	int stream_id = substream->pstr->stream;

    if(SNDRV_PCM_STREAM_PLAYBACK == stream_id)
	{
		see_iis_platform_s39_pointer(stream_id,(UINT32 *)(&s_see_alsa_cur_pos));
		offset = bytes_to_frames(runtime, s_see_alsa_cur_pos);
		if (offset >= runtime->buffer_size)
		{
		   offset = 0;
		}
    }
	else if(SNDRV_PCM_STREAM_CAPTURE == stream_id)
	{
	   	see_iis_platform_s39_pointer(stream_id,(UINT32 *)(&g_see_alsa_cur_pos_capture));	
		offset = bytes_to_frames(runtime, g_see_alsa_cur_pos_capture);
		if (offset >= runtime->buffer_size)
		{
		   offset = 0;
		}
	}

	return offset;
}


/*
** this stops the dma and clears the dma ptrs
*/
static void audio_stop_dma(struct audio_stream *s)
{
	struct snd_pcm_substream *substream = s->stream;
	int stream_id;
	
	if (NULL == substream)
		return;

	stream_id = substream->pstr->stream;

	s->active = 0;
	s->period = 0;

	//alsa_timer_should_repeat = 0; //stop the repeat timer
	
    see_iis_s39_stop_dma(stream_id, NULL, NULL);
	
	printk(ALSAPRINT "!!!infunction %s, line %d stream_id:%d!!!\n",__FUNCTION__,__LINE__,stream_id);
}


/*
** this process the dma
*/
static void audio_process_dma(struct audio_stream *s)
{
	struct snd_pcm_substream *substream = s->stream;
	struct snd_pcm_runtime *runtime;
	int stream_id;
	unsigned int dma_size;		
	unsigned int offset;
	int ret;
    
	if (NULL == substream)
		return;

	runtime = substream->runtime;

	stream_id = substream->pstr->stream;

	if (s->active)
    {
		dma_size = frames_to_bytes(runtime, runtime->period_size);
        
		if (s->old_offset) 
        {
			/* a little trick, we need resume from old position */
			offset = frames_to_bytes(runtime, s->old_offset - 1);
			s->old_offset = 0;
			s->periods = 0;
			s->period = offset / dma_size;
			offset %= dma_size;
			dma_size = dma_size - offset;
			printk(ALSAPRINT "ALi audio_process_dma: old_offset dma_size = %d\n", dma_size);
			if (!dma_size)
				return;	//continue;		/* special case */
		} 
		else
		{
			offset = dma_size * s->period;
		}

		printk(ALSAPRINT "ALi audio_process_dma: dma_size = %d, s->period = %d, offset = %d\n",\
		dma_size, s->period, offset);

		ret = see_iis_s39_start_dma(stream_id, runtime->dma_addr + offset, dma_size);
		
		if (ret) 
        {
			printk(ALSAPRINT "audio_process_dma: cannot queue DMA buffer (%i)\n", ret);
			return;
		}
	}

}


irqreturn_t audio_dma_callback(int irq, void *dev_id)
{   
	return IRQ_HANDLED; 
}
EXPORT_SYMBOL_GPL(audio_dma_callback);


static int iis_platform_s39_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct s39_soc *chip =substream->runtime->private_data;
	int stream_id = substream->pstr->stream;
	struct audio_stream *s = &chip->s[stream_id];
	struct audio_stream *s1 = &chip->s[stream_id ^ 1];
	int err = 0;
	
	//printk("!!!infunction %s, line %d cmd is %d stream_id=%d!!!\n",__FUNCTION__,__LINE__, cmd, stream_id);
	
	/* note local interrupts are already disabled in the midlevel code */
	spin_lock(&s->dma_lock);
	
	switch (cmd) 
	{
		case SNDRV_PCM_TRIGGER_START:

			/* now we need to make sure a record only stream has a clock */
			if (stream_id == SNDRV_PCM_STREAM_CAPTURE && !s1->active) 
			{
			    #if 0
				//init capture timer
         		init_timer(&alsa_period_timer_capture);
         		alsa_period_timer_capture.function = alsa_timer_handler_capture;
         		alsa_period_timer_capture.data = (unsigned long)(&chip->s[SNDRV_PCM_STREAM_CAPTURE]);
				alsa_period_timer_capture.expires = jiffies + msecs_to_jiffies(10);
				add_timer(&alsa_period_timer_capture);
				#endif
				alsa_capture_active = 1;
				/* we need to force fill the xmit DMA with zeros */
				s1->tx_spin = 1;
				audio_process_dma(s1);
                printk("%s,%d\n", __FUNCTION__, __LINE__);
			}
			/* this case is when you were recording then you turn on a
			 * playback stream so we stop (also clears it) the dma first,
			 * clear the sync flag and then we let it turned on
			 */		
			else 
			{
			    #if 0
			    //init playback timer
        		init_timer(&alsa_period_timer);
        		alsa_period_timer.function = alsa_timer_handler;
        		alsa_period_timer.data = (unsigned long)(&chip->s[SNDRV_PCM_STREAM_PLAYBACK]);	
				alsa_period_timer.expires = jiffies + msecs_to_jiffies(10);
				add_timer(&alsa_period_timer);
				#endif
				alsa_playback_active = 1;
	 			s->tx_spin = 0;
                //printk("%s,%d\n", __FUNCTION__, __LINE__);
	 		}

			/* requested stream startup */
			s->active = 1;
			printk(ALSAPRINT "iis_platform_s39_trigger: SNDRV_PCM_TRIGGER_START\n");
			audio_process_dma(s);

			delta_bytes = 0;
			break;
			
		case SNDRV_PCM_TRIGGER_STOP:

			/* requested stream shutdown */
            //printk("%s,%d\n", __FUNCTION__, __LINE__);
			audio_stop_dma(s);

	        //added by kinson
			if (stream_id == SNDRV_PCM_STREAM_PLAYBACK)
			{
			    //printk("%s,%d\n", __FUNCTION__, __LINE__);
			    //alsa_timer_should_repeat = 0;
                alsa_playback_active = 0;
				#if 0
			    del_timer_sync(&alsa_period_timer);
				printk("%s,%d\n", __FUNCTION__, __LINE__);
                #endif
			}
			else
			{
			    printk("%s,%d\n", __FUNCTION__, __LINE__);

			    alsa_capture_active = 0;
			    #if 0
			    alsa_timer_capture_should_repeat = 0;
			    del_timer_sync(&alsa_period_timer_capture);
				#endif
			}

			//printk("%s,%d\n", __FUNCTION__, __LINE__);
			/*
			 * now we need to make sure a record only stream has a clock
			 * so if we're stopping a playback with an active capture
			 * we need to turn the 0 fill dma on for the xmit side
			 */
			if (stream_id == SNDRV_PCM_STREAM_PLAYBACK && s1->active) 
			{
				/* we need to force fill the xmit DMA with zeros */
				s->tx_spin = 1;
				printk(ALSAPRINT"iis_platform_s39_trigger: SNDRV_PCM_TRIGGER_STOP\n");
				audio_process_dma(s);
			}
			/*
			 * we killed a capture only stream, so we should also kill
			 * the zero fill transmit
			 */
			else 
			{
				if (s1->tx_spin) 
				{
					s1->tx_spin = 0;
	                printk(ALSAPRINT"iis_platform_s39_trigger: SNDRV_PCM_TRIGGER_STOP s1 stream_id=%d\n",stream_id);
					audio_stop_dma(s1);

					printk("%s,%d\n", __FUNCTION__, __LINE__);
				}
			}
			
			//printk("%s,%d\n", __FUNCTION__, __LINE__);			
			break;
			
		case SNDRV_PCM_TRIGGER_SUSPEND:
			
			s->active = 0;
			s->old_offset = audio_get_dma_pos(s) + 1;
			s->periods = 0;
			break;
			
		case SNDRV_PCM_TRIGGER_RESUME:
			
			s->active = 1;
			s->tx_spin = 0;
			printk(ALSAPRINT "iis_platform_s39_trigger: SNDRV_PCM_TRIGGER_RESUME\n");
			audio_process_dma(s);
			if (stream_id == SNDRV_PCM_STREAM_CAPTURE && !s1->active) 
			{
				s1->tx_spin = 1;
				audio_process_dma(s1);
			}
			break;
			
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			s->active = 0;
			if (stream_id == SNDRV_PCM_STREAM_PLAYBACK) 
			{
				if (s1->active) 
				{
					s->tx_spin = 1;
					s->old_offset = audio_get_dma_pos(s) + 1;	
					printk("iis_platform_s39_trigger: SNDRV_PCM_TRIGGER_PAUSE_PUSH\n");
					audio_process_dma(s);
				}
			} 
		    else 
			{
				if (s1->tx_spin) 
				{
					s1->tx_spin = 0;
				}
			}
			break;
			
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			
			s->active = 1;
			if (s->old_offset) 
			{
				s->tx_spin = 0;
				printk("iis_platform_s39_trigger: SNDRV_PCM_TRIGGER_PAUSE_RELEASE\n");
				audio_process_dma(s);
				break;
			}
			if (stream_id == SNDRV_PCM_STREAM_CAPTURE && !s1->active) 
			{
				s1->tx_spin = 1;
				audio_process_dma(s1);
			}
			break;
			
		default:
			err = -EINVAL;
			break;
	}
   
	spin_unlock(&s->dma_lock);
	
	return err;
}

static int iis_platform_s39_prepare(struct snd_pcm_substream *substream)
{
	struct s39_soc *chip = substream->runtime->private_data;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct audio_stream *s = &chip->s[substream->pstr->stream];

	unsigned int channels = runtime->channels;
	unsigned int sample_bits = runtime->sample_bits;
	int stream_id = substream->pstr->stream;

	unsigned int share_buf_addr;

	printk(ALSAPRINT "!!!infunction %s, line %d  buffer_size=0x%08x g_buffer_pos=%d!!!\n",__FUNCTION__,__LINE__,runtime->buffer_size,g_see_alsa_cur_pos_capture);    

    if(stream_id == SNDRV_PCM_STREAM_PLAYBACK)
    {
		/* set requested samplerate */
		chip->samplerate = runtime->rate;
		s->period = 0;
		s->periods = 0;
		s->old_offset = 0;

		s_see_audio_buffer_address=__G_ALI_MM_DECA_MEM_START_ADDR;
		s_see_audio_buffer_size=__G_ALI_MM_DECA_MEM_SIZE;
		memset((char *)s_see_audio_buffer_address, 0, s_see_audio_buffer_size);
		
		share_buf_addr =((((UINT32)s_see_audio_buffer_address)&0x0fffffff) | 0xa0000000); 	  			
		see_iis_platform_s39_prepare(stream_id, channels, runtime->rate, sample_bits, share_buf_addr, frames_to_bytes(runtime,runtime->buffer_size));
		
	}
	else
	{
	    alsa_timer_capture_should_repeat = 1; //enable the repeat timer   
	    //mod_timer(&alsa_period_timer_capture, jiffies + msecs_to_jiffies(1)); 
	   
		g_see_audio_buffer_address_capture=__G_ALI_MM_ALSA_CAPTURE_START_ADDR;
		memset(g_see_audio_buffer_address_capture, 0 ,__G_ALI_MM_ALSA_CAPTURE_MEM_SIZE);
				
		share_buf_addr =((((UINT32)g_see_audio_buffer_address_capture)&0x0fffffff) | 0xa0000000); 	  
	
		*(UINT32*)g_see_audio_buffer_address_capture = 0;

		runtime->status->hw_ptr = runtime->control->appl_ptr = 0;
		
		see_iis_platform_s39_prepare(stream_id, channels, runtime->rate, sample_bits, share_buf_addr, frames_to_bytes(runtime,runtime->buffer_size));
	}
	 
	return 0;
}


static snd_pcm_uframes_t iis_platform_s39_pointer(struct snd_pcm_substream *substream)
{
	struct s39_soc *chip = substream->runtime->private_data;
	int ret=0;
	
	ret = audio_get_dma_pos(&chip->s[substream->pstr->stream]);

    if (ret < 0)
    {
	    printk("R:%d\n", ret);
	}
	
	return ret;
}


static int iis_platform_s39_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *hw_params)
{
	int ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(hw_params));
	struct snd_pcm_runtime *runtime = substream->runtime;

	printk(ALSAPRINT "infunction %s, line %d runtime->dma_area = 0x%08lx dma_addr_t = 0x%08lx dma_size = %d "
	"runtime->min_align %ld\n",__FUNCTION__,__LINE__,(unsigned long)runtime->dma_area, (unsigned long)runtime->dma_addr, \
	runtime->dma_bytes, runtime->min_align);

    int buf_size;
	
	buf_size =  params_buffer_bytes(hw_params);

	//printk("%s,%d,buf_size:%d\n", __FUNCTION__, __LINE__, buf_size);

	printk(ALSAPRINT "ALi iis_platform_s39_hw_params, re = %d\n", ret);

    printk("%s,%d,__G_ALI_MM_ALSA_CAPTURE_START_ADDR:0x%x,__G_ALI_MM_ALSA_CAPTURE_MEM_SIZE:%u,cfg size:%d\n",
		__FUNCTION__, __LINE__, __G_ALI_MM_ALSA_CAPTURE_START_ADDR, __G_ALI_MM_ALSA_CAPTURE_MEM_SIZE, params_buffer_bytes(hw_params));

	return 0;
}

static int iis_platform_s39_hw_free(struct snd_pcm_substream *substream)
{
	printk(ALSAPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
	
	return snd_pcm_lib_free_pages(substream);
}


#define S39_SND_DRIVER		"s39_snd"

#if 1
static struct snd_pcm_hardware iis_platform_s39_playback_hardware =
{
	.info			= (SNDRV_PCM_INFO_INTERLEAVED |
				   SNDRV_PCM_INFO_BLOCK_TRANSFER |
				   SNDRV_PCM_INFO_PAUSE | SNDRV_PCM_INFO_RESUME),
	.formats		= ( SNDRV_PCM_FMTBIT_S16_LE),

	.rates			= (/* SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |\ */
                   SNDRV_PCM_RATE_32000 |\
				   SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000 |\
				   SNDRV_PCM_RATE_KNOT),
	.rate_min		= 32000,			/* 8000, */
	.rate_max		= 48000,
	.channels_min		= 1,
	.channels_max		= 2,		/* s39 only 2 channels */
	.buffer_bytes_max	=PCM_BUFF_SIZE * 8,
	.periods_min		= 2,		// 2,
	.periods_max		= 10,		// (buffer_bytes_max / period_bytes_min)
	.period_bytes_min	= S39_PERIOD_MIN_BYTES,//1536*2,//S39_PERIOD_MIN_BYTES,
	.period_bytes_max	= PCM_BUFF_SIZE * 8,
};
#else
/*txj mask for we will not use mmap for alsa driver, we will use see CPU for audio sink */
static struct snd_pcm_hardware iis_platform_s39_playback_hardware =
{
	.info			= (SNDRV_PCM_INFO_INTERLEAVED |
				   SNDRV_PCM_INFO_BLOCK_TRANSFER |
				   SNDRV_PCM_INFO_PAUSE | SNDRV_PCM_INFO_RESUME),
	.formats		= ( SNDRV_PCM_FMTBIT_S16_LE),

	.rates			= (/* SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |\ */
                   SNDRV_PCM_RATE_32000 |\
				   SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000 |\
				   SNDRV_PCM_RATE_KNOT),
	.rate_min		= 32000,			/* 8000, */
	.rate_max		= 48000,
	.channels_min		= 1,
	.channels_max		= 2,		/* s39 only 2 channels */
	.buffer_bytes_max	= 32 * 1024,//PCM_BUFF_SIZE * 8,
	.periods_min		= 2,		// 2,
	.periods_max		= 128,		// (buffer_bytes_max / period_bytes_min)
	.period_bytes_min	= 16,//S39_PERIOD_MIN_BYTES,
	.period_bytes_max	= 1024 * 32,//PCM_BUFF_SIZE * 8,
};

#endif





static int iis_platform_s39_open(struct snd_pcm_substream *substream)
{

	struct s39_soc *chip = NULL;
	int stream_id = substream->pstr->stream;
	struct snd_pcm_runtime *runtime = substream->runtime;
	int err = -1;

	//printk( "%s,%d,stream_id:%d\n",__FUNCTION__,__LINE__,stream_id);

	if (SNDRV_PCM_STREAM_PLAYBACK == stream_id)
	{
	    //printk("%s,%d\n", __FUNCTION__, __LINE__);
	    /* Status checking, only one pcm stream is alllowed.
	     * Added by Joy, date:2014.06.24
	     */
	    if (ALI_ALSA_STREAM_STATUS_IDLE != ali_alsa_stream_playback_status)
	    {
		    printk(ALSAPRINT "%s,%d,status:%x", __FUNCTION__, __LINE__, ali_alsa_stream_playback_status);
			return(-EMFILE);
		}

		chip = kzalloc(sizeof(*chip), GFP_KERNEL);
		if (chip == NULL)
		{
		   return -ENOMEM;
		}

		runtime->private_data = chip;
		chip->card=substream->pcm->card;
		chip->s[stream_id].stream = substream;
	
		/* Setup DMA stuff */
		chip->s[stream_id].id = "s39_SND out";
		chip->s[stream_id].stream_id = SNDRV_PCM_STREAM_PLAYBACK;

		snd_soc_set_runtime_hwparams(substream, &iis_platform_s39_playback_hardware);

		/* reset snd for dma occurs error,set 24-bit for playback */
		/* setup DMA controller */
		audio_dma_request(&chip->s[SNDRV_PCM_STREAM_PLAYBACK], (void *)audio_dma_callback);

		spin_lock_init(&chip->s[0].dma_lock);

		if ((err = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS)) < 0)
			return err;
		
		if ((err = snd_pcm_hw_constraint_list(runtime, 0, SNDRV_PCM_HW_PARAM_RATE, &hw_constraints_rates)) < 0)
			return err;

        #if 0
        //init playback timer
		init_timer(&alsa_period_timer);
		alsa_period_timer.function = alsa_timer_handler;
		alsa_period_timer.data = (unsigned long)(&chip->s[SNDRV_PCM_STREAM_PLAYBACK]);
        #endif


		//printk("%s,%d\n", __FUNCTION__, __LINE__);
		//init playback timer
		init_timer(&alsa_period_timer);
		alsa_period_timer.function = alsa_timer_handler;
		alsa_period_timer.data = (unsigned long)(&chip->s[SNDRV_PCM_STREAM_PLAYBACK]);	
		alsa_period_timer.expires = jiffies + msecs_to_jiffies(10);
		add_timer(&alsa_period_timer);
		//printk("%s,%d\n", __FUNCTION__, __LINE__);
		
        //init playback work queue
	    if(!g_see_alsa_work_queue_playback)
		{
			g_see_alsa_work_queue_playback=create_workqueue("see_alsa_workqueue");
			if (!(g_see_alsa_work_queue_playback))
			{
				printk(ALSAPRINT "!!!infunction %s, line %d we can not create  see_alsa_workqueue!!!\n",__FUNCTION__,__LINE__);
				destroy_workqueue(g_see_alsa_work_queue_playback);
				return -1;
			} 
			
			INIT_WORK(&g_see_alsa_work_queue_playback_priv.see_alsa_work, rpc_see_functions_playback);  
	    }  

		see_iis_platform_s39_open(SNDRV_PCM_STREAM_PLAYBACK,NULL);

		ali_alsa_stream_playback_status = ALI_ALSA_STREAM_STATUS_RUN;
		
	    //printk( "%s,%d,stream_id:%d\n",__FUNCTION__,__LINE__,stream_id);

	} 
	else                                               
	{
	    /* Status checking, only one pcm stream is alllowed.
	     * Added by Joy, date:2014.06.24
	     */
	    if (ALI_ALSA_STREAM_STATUS_IDLE != ali_alsa_stream_capture_status)
	    {
		    printk(ALSAPRINT"%s,%d,status:%x", __FUNCTION__, __LINE__, ali_alsa_stream_capture_status);
			return(-EMFILE);
		}
		
	    chip = kzalloc(sizeof(*chip), GFP_KERNEL);
		if (chip == NULL)
		{
		   return -ENOMEM;
		}

		runtime->private_data = chip;
		chip->card=substream->pcm->card;
		chip->s[stream_id].stream = substream;
		
		chip->s[stream_id].id = "s39_I2S in";
		chip->s[stream_id].stream_id = SNDRV_PCM_STREAM_CAPTURE;

		snd_soc_set_runtime_hwparams(substream, &iis_platform_s39_playback_hardware);

		spin_lock_init(&chip->s[1].dma_lock);

		if ((err = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS)) < 0)
			return err;
		
		if ((err = snd_pcm_hw_constraint_list(runtime, 0, SNDRV_PCM_HW_PARAM_RATE, &hw_constraints_rates)) < 0)
			return err;

        #if 0
        //init capture timer
		init_timer(&alsa_period_timer_capture);
		alsa_period_timer_capture.function = alsa_timer_handler_capture;
		alsa_period_timer_capture.data = (unsigned long)(&chip->s[SNDRV_PCM_STREAM_CAPTURE]);
        #endif

		//printk("%s,%d\n", __FUNCTION__, __LINE__);

		//init capture timer
 		init_timer(&alsa_period_timer_capture);
 		alsa_period_timer_capture.function = alsa_timer_handler_capture;
 		alsa_period_timer_capture.data = (unsigned long)(&chip->s[SNDRV_PCM_STREAM_CAPTURE]);
		alsa_period_timer_capture.expires = jiffies + msecs_to_jiffies(10);
		add_timer(&alsa_period_timer_capture);
		//printk("%s,%d\n", __FUNCTION__, __LINE__);
		
		//init capture work queue
	    if(!g_see_alsa_work_queue_capture)
		{
			g_see_alsa_work_queue_capture=create_workqueue("see_alsa_workqueue");
			if (!(g_see_alsa_work_queue_capture))
			{
				printk(ALSAPRINT "!!!infunction %s, line %d we can not create  see_alsa_workqueue!!!\n",__FUNCTION__,__LINE__);
				destroy_workqueue(g_see_alsa_work_queue_capture);
				return -1;
			} 
			
			INIT_WORK(&g_see_alsa_work_queue_capture_priv.see_alsa_work, rpc_see_functions_capture); 
	    } 

		ali_alsa_stream_capture_status = ALI_ALSA_STREAM_STATUS_RUN;

	    printk( "%s,%d,stream_id:%d\n",__FUNCTION__,__LINE__,stream_id);
	}

	//printk( "%s,%d,stream_id:%d\n",__FUNCTION__,__LINE__,stream_id);
	
	return 0;
}

static int iis_platform_s39_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct s39_soc *prtd = runtime->private_data;
	int stream_id = substream->pstr->stream;

	//printk( "%s,%d,stream_id:%d\n",__FUNCTION__,__LINE__,stream_id);


	if (SNDRV_PCM_STREAM_PLAYBACK == stream_id)
	{ 
	    if (ALI_ALSA_STREAM_STATUS_RUN != ali_alsa_stream_playback_status)
	    {
		    printk("%s,%d,status:%x", __FUNCTION__, __LINE__, ali_alsa_stream_playback_status);
			return(-EPERM);
		}

		#if 1
		
		//printk("%s,%d\n", __FUNCTION__, __LINE__);		
	    //del_timer_sync(&alsa_period_timer);
		del_timer(&alsa_period_timer);
		//printk("%s,%d\n", __FUNCTION__, __LINE__);
        #endif
		 
		audio_dma_free(&prtd->s[SNDRV_PCM_STREAM_PLAYBACK]);
		prtd->s[substream->pstr->stream].stream = NULL;
		kfree(prtd);
		see_iis_platform_s39_close(stream_id, NULL);
		////destroy_workqueue(g_see_alsa_work_queue_playback);
		ali_alsa_stream_playback_status = ALI_ALSA_STREAM_STATUS_IDLE;
	    //printk( "%s,%d,stream_id:%d\n",__FUNCTION__,__LINE__,stream_id);
	}
	else
	{ 
	    if (ALI_ALSA_STREAM_STATUS_RUN != ali_alsa_stream_capture_status)
	    {
		    printk("%s,%d,status:%x", __FUNCTION__, __LINE__, ali_alsa_stream_capture_status);
			return(-EPERM);
		}

	    #if 1
		
		//printk("%s,%d\n", __FUNCTION__, __LINE__);
	    //del_timer_sync(&alsa_period_timer_capture);
	    del_timer(&alsa_period_timer_capture);
		//printk("%s,%d\n", __FUNCTION__, __LINE__);		
		#endif	
		
		see_iis_platform_s39_close(stream_id, NULL);
		mdelay(5);//wait record_thread in see,because see will call timer_capture
		audio_dma_free(&prtd->s[SNDRV_PCM_STREAM_CAPTURE]);
		prtd->s[substream->pstr->stream].stream = NULL;
		kfree(prtd);	
		////destroy_workqueue(g_see_alsa_work_queue_capture);
		ali_alsa_stream_capture_status = ALI_ALSA_STREAM_STATUS_IDLE;
	    printk( "%s,%d,stream_id:%d\n",__FUNCTION__,__LINE__,stream_id);
	}

	//printk( "%s,%d,stream_id:%d\n",__FUNCTION__,__LINE__,stream_id);
	
	return 0;
}




int snd_chk_sample_diff_s16
(
    char *fuction,
    int  line,
    short *data,
    int  size//in bytes
)
{
    int gap;
	int idx;

	for (idx = 0; idx < (size / 4 - 1); idx++)
	{
        gap = abs(data[idx * 2] - data[idx * 2 + 2]);

        if (gap >= 0x100)
        {
            printk("%s,%d,size:%d,idx:%d,data[idx*2]:%x,data[idx*2+2]:%x,gap:%x\n", fuction, line, size, idx, data[idx*2], data[idx*2+2], gap);
    	}	
	}
	
	return(0);
}	




static int iis_platform_s39_copy(struct snd_pcm_substream *substream, int channel,
		    snd_pcm_uframes_t pos,
		    void __user *buf, snd_pcm_uframes_t count)
{
	static int printCount = 0;
	UINT8 *share_buf = NULL;
	int ret = -1;
	struct snd_pcm_runtime *runtime = substream->runtime;
	
	printCount++;
	
	char *hwbuf = runtime->dma_area + frames_to_bytes(runtime, pos);

	int stream_id = substream->pstr->stream;
	
	if (SNDRV_PCM_STREAM_PLAYBACK == stream_id)
	{
		ret = copy_from_user((unsigned char *)((UINT32)s_see_audio_buffer_address+256), buf, frames_to_bytes(runtime, count));
        if (0 != ret)
        {        
            return(-EFAULT);
        }	

		share_buf = (UINT8 *)((((UINT32)s_see_audio_buffer_address+256)&0x0fffffff) | 0xa0000000);

        //if (1 == ali_alsa_playback_dump_en)
        if (1 == ali_alsa_playback_dump_en_get())
        {
            KernelWriteFilePlayback((unsigned char *)((UINT32)s_see_audio_buffer_address+256), frames_to_bytes(runtime, count));
		}
		
		see_iis_platform_s39_write((UINT32 *)share_buf, frames_to_bytes(runtime, count));
		if((printCount%50)==0)
		{
			//printk(ALSAPRINT "!!!%s %s copy pos:0x%lx count:0x%lx\n",__FUNCTION__,
			//substream->stream ? "Capture" : "Playback", pos, count);
			//printk(ALSAPRINT "s_see_audio_buffer_address[0] is %c \n", *((char *)s_see_audio_buffer_address));
		}
	}
	else
	{   
		share_buf =((((UINT32)((char *)__G_ALI_MM_ALSA_CAPTURE_START_ADDR+1024))&0x0fffffff) | 0xa0000000); //reserved 1024byte
		
	    see_iis_platform_s39_read((UINT32 *)share_buf, frames_to_bytes(runtime, pos),frames_to_bytes(runtime, count));

		//snd_chk_sample_diff_s16(__FUNCTION__, __LINE__, (char *)__G_ALI_MM_ALSA_CAPTURE_START_ADDR+1024, frames_to_bytes(runtime, count));

        //if (1 == ali_alsa_capture_dump_en)
        if (1 == ali_alsa_capture_dump_en_get())        
        {
            KernelWriteFileCapture((char *)__G_ALI_MM_ALSA_CAPTURE_START_ADDR+1024, frames_to_bytes(runtime, count));	
		}

		if (copy_to_user(buf, (char *)__G_ALI_MM_ALSA_CAPTURE_START_ADDR+1024, frames_to_bytes(runtime, count)))
		{  
		   printk("%s,%d err!\n",__FUNCTION__,__LINE__);
		   
		   return -EFAULT;
		}

		//printk("%s,%d  pos:%d  size:%d!\n",__FUNCTION__,__LINE__,frames_to_bytes(runtime, pos),frames_to_bytes(runtime, count)); 
	}
	
	return 0;
}








static u64 s39_pcm_dmamask = DMA_BIT_MASK(32);

static int iis_platform_s39_preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	size_t size = iis_platform_s39_playback_hardware.buffer_bytes_max;

	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;

#if 1 //txj mask for we use it for hardware limit
	buf->area = dma_alloc_writecombine(pcm->card->dev, size,
		   &buf->addr, GFP_KERNEL);
	if (!buf->area)
		return -ENOMEM;
	buf->bytes = size;
#else
	g_audio_static_buf= 0xC4653000;
	buf->area=g_audio_static_buf;
	buf->addr=(buf->area-0xC0000000)+0x80000000;
	buf->bytes = size;
	printk(ALSAPRINT "in Function %s, line %d , g_audio_static_buf is %x buf->area is %x, buf->addr is %x buf->bytes is %d  \n", __FUNCTION__,__LINE__, g_audio_static_buf,buf->area, buf->addr, buf->bytes);
#endif
	return 0;
}







static int iis_platform_s39_new(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_card *card = rtd->card->snd_card;
	struct snd_pcm *pcm = rtd->pcm;
	int ret = 0;

	if (!card->dev->dma_mask)
		card->dev->dma_mask = &s39_pcm_dmamask;
	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask =DMA_BIT_MASK(32);

	if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream) 
	{
		ret = iis_platform_s39_preallocate_dma_buffer(pcm, SNDRV_PCM_STREAM_PLAYBACK);
		if (ret)
			return ret;
	}

	if (pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream) 
	{
		ret = iis_platform_s39_preallocate_dma_buffer(pcm, SNDRV_PCM_STREAM_CAPTURE);
		if (ret)
			return ret;
	}

    //printk("%s,%d,succeed.\n", __FUNCTION__, __LINE__);

    ali_alsa_dbg_init();

    printk("%s,%d,succeed.\n", __FUNCTION__, __LINE__);

	return 0;
}




static void iis_platform_s39_free(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;
	int stream;

	for (stream = 0; stream < SNDRV_PCM_STREAM_LAST; stream++) 
	{
		substream = pcm->streams[stream].substream;
		if (!substream)
			continue;

		buf = &substream->dma_buffer;
		if (!buf->area)
			continue;

		dma_free_writecombine(pcm->card->dev, buf->bytes, buf->area, buf->addr);
		buf->area = NULL;
	}
}

 static struct snd_pcm_ops asoc_iis_platform_s39_ops = {
	 .open		 = iis_platform_s39_open,
	 .close 	 = iis_platform_s39_close,
	 .ioctl 		 = snd_pcm_lib_ioctl,
	 .hw_params  = iis_platform_s39_hw_params,
	 .hw_free		 = iis_platform_s39_hw_free,
	 .prepare		 = iis_platform_s39_prepare,
	 .trigger		 = iis_platform_s39_trigger,
	 .pointer		 = iis_platform_s39_pointer,
	 .copy		  = iis_platform_s39_copy,
 };

 static struct snd_soc_platform_driver asoc_iis_platform_s39_driver = {
	.ops		= &asoc_iis_platform_s39_ops,
	.pcm_new	= iis_platform_s39_new,
	.pcm_free	= iis_platform_s39_free,
};


static int __devinit iis_platform_s39_dev_probe(struct platform_device *pdev)
{
	printk(ALSAPRINT "in Function %s, line %d \n", __FUNCTION__,__LINE__);

	return snd_soc_register_platform(&pdev->dev, &asoc_iis_platform_s39_driver);
}

static int __devexit iis_platform_s39_dev_remove(struct platform_device *pdev)
{
	printk(ALSAPRINT "in Function %s, line %d \n", __FUNCTION__,__LINE__);

	snd_soc_unregister_platform(&pdev->dev);
	return 0;
}

static struct platform_driver iis_platform_s39_dev_driver = {
	.driver = {
	    	       .name = 	"asoc_iis_platform_s39_dev",
			.owner = THIS_MODULE,
	},

	.probe = iis_platform_s39_dev_probe,
	.remove = __devexit_p(iis_platform_s39_dev_remove),
};

module_platform_driver(iis_platform_s39_dev_driver);


MODULE_AUTHOR("shenzhen os team base on xavier chang ");
MODULE_DESCRIPTION("ALi PCM Asoc driver with see CPU");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{s39XX Series SOC}}");

