

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

#include "../m39-pcm.h"

#include <linux/dma-mapping.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

//#define CONFIG_HDMI_ALI
#undef CONFIG_HDMI_ALI
#ifdef CONFIG_HDMI_ALI
#include <linux/ali_hdmi.h>
#endif

#include <linux/sched.h>
#include <linux/time.h>
#include <linux/timer.h>

#include "../AUD_common.h"
#include "../AUD_reg.h"
#include <asm/mach-ali/m36_gpio.h>
#define AUD_MUTE_GPIO_NO_QFP  92
#define AUD_MUTE_GPIO_NO_BGA  80

#define LOG(fmt,arg...)		printk(fmt,##arg)
#define ERROR_LOG(fmt,arg...)	printk(fmt,##arg)

//add by alex for spdif out bitstream
#define IEC958_6144     
static u8 g_open_device_id; /*for audio_get_dma_pos() function, calculate pos */
#define SDBBP() asm volatile(".word 0x7000003f; nop");

#define S3901  0
#define C3901  1

//our hardware limited, we can not access high memory
unsigned long g_audio_static_buf =0;  

//following code will be moved 
#if 1
// ============================================================
#define HAL_GPIO_ENABLE  1

#define HAL_GPIO_EN_REG        __REGALIRAW(0x18000430)  /* GPIO  */
#define HAL_GPIO1_EN_REG       __REGALIRAW(0x18000434)  /* GPIO2 */
#define HAL_GPIO2_EN_REG       __REGALIRAW(0x18000438)  /* GPIO3 */
#define HAL_GPIO3_EN_REG       __REGALIRAW(0x1800043c)  /* GPIO4 */
#define HAL_GPIO4_EN_REG	__REGALIRAW(0x18000440)	/* GPIO5 */

#define HAL_GPIO_EN_SET(val)         (*(volatile unsigned long *)HAL_GPIO_EN_REG =(val))
#define HAL_GPIO1_EN_SET(val)        (*(volatile unsigned long *)HAL_GPIO1_EN_REG =(val))
#define HAL_GPIO2_EN_SET(val)        (*(volatile unsigned long *)HAL_GPIO2_EN_REG =(val))
#define HAL_GPIO3_EN_SET(val)        (*(volatile unsigned long *)HAL_GPIO3_EN_REG =(val))
#define HAL_GPIO4_EN_SET(val)        (*(volatile unsigned long *)HAL_GPIO4_EN_REG =(val))

// get input gpio value
#define HAL_GPIO_BIT_GET(pos)			\
			((pos < 32) ? ((HAL_GPIO_READ() >> (pos)) & 1) \
			: ((pos < 64) ? ((HAL_GPIO1_READ() >> (pos - 32)) & 1) \
			: ((pos < 96) ? ((HAL_GPIO2_READ() >> (pos - 64)) & 1) \
			: ((pos < 128) ? ((HAL_GPIO3_READ() >> (pos - 96)) & 1) \
			: ((HAL_GPIO4_READ() >> (pos - 128)) & 1)))))

#define HAL_GPIO_BIT_DIR_GET(pos)		\
			((pos < 32) ? ((HAL_GPIO_DIR_GET() >> (pos)) & 1) \
			: ((pos < 64) ? ((HAL_GPIO1_DIR_GET() >> (pos - 32)) & 1) \
			: ((pos < 96) ? ((HAL_GPIO2_DIR_GET() >> (pos - 64)) & 1) \
			: ((pos < 128) ? ((HAL_GPIO3_DIR_GET() >> (pos - 96)) & 1) \
			: ((HAL_GPIO4_DIR_GET() >> (pos - 128)) & 1))))

#define HAL_GPIO_BIT_DIR_SET(pos, val)	\
		do { \
			((pos < 32) ? HAL_GPIO_DIR_SET((HAL_GPIO_DIR_GET() & ~(1 << (pos))) | ((val) << (pos))) \
			: ((pos < 64) ? HAL_GPIO1_DIR_SET((HAL_GPIO1_DIR_GET() & ~(1 << (pos - 32))) | ((val) << (pos - 32))) \
			: ((pos < 96) ? HAL_GPIO2_DIR_SET((HAL_GPIO2_DIR_GET() & ~(1 << (pos - 64))) | ((val) << (pos - 64))) \
			: ((pos < 128) ? HAL_GPIO3_DIR_SET((HAL_GPIO3_DIR_GET() & ~(1 << (pos - 96))) | ((val) << (pos - 96))) \
			: HAL_GPIO4_DIR_SET((HAL_GPIO4_DIR_GET() & ~(1 << (pos - 128))) | ((val) << (pos - 128)))))));\
		} while (0)

#define HAL_GPIO_BIT_SET(pos, val)		\
		do { \
			((pos < 32)	? HAL_GPIO_WRITE(((*(volatile DWORD *)HAL_GPIO_DO_REG) & ~(1 << (pos))) | ((val) << (pos))) \
			: ((pos < 64) ? HAL_GPIO1_WRITE(((*(volatile DWORD *)HAL_GPIO1_DO_REG) & ~(1 << (pos - 32))) | ((val) << (pos - 32))) \
			: ((pos < 96) ? HAL_GPIO2_WRITE(((*(volatile DWORD *)HAL_GPIO2_DO_REG) & ~(1 << (pos - 64))) | ((val) << (pos - 64))) \
			: ((pos < 128) ? HAL_GPIO3_WRITE(((*(volatile DWORD *)HAL_GPIO3_DO_REG) & ~(1 << (pos - 96))) | ((val) << (pos - 96))) \
			: HAL_GPIO4_WRITE(((*(volatile DWORD *)HAL_GPIO4_DO_REG) & ~(1 << (pos - 128))) | ((val) << (pos - 128))))))); \
		} while (0)

#define HAL_GPIO_INT_SET(pos, en)		\
		do { \
			((pos < 32)					\
			? HAL_GPIO_IER_SET(((*(volatile DWORD *)HAL_GPIO_IER_REG) & ~(1 << (pos))) | ((en) << (pos))) \
			: ((pos < 64) 				\
			? HAL_GPIO1_IER_SET(((*(volatile DWORD *)HAL_GPIO1_IER_REG) & ~(1 << (pos - 32))) | ((en) << (pos - 32))) \
			: ((pos < 96) 				\
			? HAL_GPIO2_IER_SET(((*(volatile DWORD *)HAL_GPIO2_IER_REG) & ~(1 << (pos - 64))) | ((en) << (pos - 64))) \
			: ((pos < 128) 				\
			? HAL_GPIO3_IER_SET(((*(volatile DWORD *)HAL_GPIO3_IER_REG) & ~(1 << (pos - 96))) | ((en) << (pos - 96))) \
			: HAL_GPIO4_IER_SET(((*(volatile DWORD *)HAL_GPIO4_IER_REG) & ~(1 << (pos - 128))) | ((en) << (pos - 128))))))); \
		} while (0)

#define HAL_GPIO_INT_REDG_SET(pos, rise)	\
		do { \
			((pos < 32)					\
			? HAL_GPIO_RER_SET(((*(volatile DWORD *)HAL_GPIO_REC_REG) & ~(1 << (pos))) | ((rise) << (pos))) \
			: ((pos < 64) 				\
			? HAL_GPIO1_RER_SET(((*(volatile DWORD *)HAL_GPIO1_REC_REG) & ~(1 << (pos - 32))) | ((rise) << (pos - 32))) \
			: ((pos < 96) 				\
			? HAL_GPIO2_RER_SET(((*(volatile DWORD *)HAL_GPIO2_REC_REG) & ~(1 << (pos - 64))) | ((rise) << (pos - 64))) \
			: ((pos < 128) 				\
			? HAL_GPIO3_RER_SET(((*(volatile DWORD *)HAL_GPIO3_REC_REG) & ~(1 << (pos - 96))) | ((rise) << (pos - 96))) \
			: HAL_GPIO4_RER_SET(((*(volatile DWORD *)HAL_GPIO4_REC_REG) & ~(1 << (pos - 128))) | ((rise) << (pos - 128))))))); \
		} while (0)

#define HAL_GPIO_INT_FEDG_SET(pos, fall)	\
		do { \
			((pos < 32)					\
			? HAL_GPIO_FER_SET(((*(volatile DWORD *)HAL_GPIO_FEC_REG) & ~(1 << (pos))) | ((fall) << (pos))) \
			: ((pos < 64)				\
			? HAL_GPIO1_FER_SET(((*(volatile DWORD *)HAL_GPIO1_FEC_REG) & ~(1 << (pos - 32))) | ((fall) << (pos - 32))) \
			: ((pos < 96) 				\
			? HAL_GPIO2_FER_SET(((*(volatile DWORD *)HAL_GPIO2_FEC_REG) & ~(1 << (pos - 64))) | ((fall) << (pos - 64))) \
			: ((pos < 128) 				\
			? HAL_GPIO3_FER_SET(((*(volatile DWORD *)HAL_GPIO3_FEC_REG) & ~(1 << (pos - 96))) | ((fall) << (pos - 96))) \
			: HAL_GPIO4_FER_SET(((*(volatile DWORD *)HAL_GPIO4_FEC_REG) & ~(1 << (pos - 128))) | ((fall) << (pos - 128))))))); \
		} while (0)

#define HAL_GPIO_INT_EDG_SET(pos, rise, fall)	\
		do { \
			((pos < 32)					\
			? (HAL_GPIO_RER_SET(((*(volatile DWORD *)HAL_GPIO_REC_REG) & ~(1 << (pos))) | ((rise) << (pos))), \
			  HAL_GPIO_FER_SET(((*(volatile DWORD *)HAL_GPIO_FEC_REG) & ~(1 << (pos))) | ((fall) << (pos)))) \
			: ((pos < 64)				\
			? (HAL_GPIO1_RER_SET(((*(volatile DWORD *)HAL_GPIO1_REC_REG) & ~(1 << (pos - 32))) | ((rise) << (pos - 32))), \
			  HAL_GPIO1_FER_SET(((*(volatile DWORD *)HAL_GPIO1_FEC_REG) & ~(1 << (pos - 32))) | ((fall) << (pos - 32)))) \
			: ((pos < 96)				\
			?  (HAL_GPIO2_RER_SET(((*(volatile DWORD *)HAL_GPIO2_REC_REG) & ~(1 << (pos - 64))) | ((rise) << (pos - 64))), \
			  HAL_GPIO2_FER_SET(((*(volatile DWORD *)HAL_GPIO2_FEC_REG) & ~(1 << (pos - 64))) | ((fall) << (pos - 64)))) \
			  : ((pos < 128)				\
			?  (HAL_GPIO3_RER_SET(((*(volatile DWORD *)HAL_GPIO3_REC_REG) & ~(1 << (pos - 96))) | ((rise) << (pos - 96))), \
			  HAL_GPIO3_FER_SET(((*(volatile DWORD *)HAL_GPIO3_FEC_REG) & ~(1 << (pos - 96))) | ((fall) << (pos - 96)))) \
			:  (HAL_GPIO4_RER_SET(((*(volatile DWORD *)HAL_GPIO4_REC_REG) & ~(1 << (pos - 128))) | ((rise) << (pos - 128))), \
			  HAL_GPIO4_FER_SET(((*(volatile DWORD *)HAL_GPIO4_FEC_REG) & ~(1 << (pos - 128))) | ((fall) << (pos - 128)))))))); \
		} while (0)

#define HAL_GPIO_INT_STA_GET(pos)		\
			((pos < 32) ? ((HAL_GPIO_ISR_GET() >> (pos)) & 1) \
			: ((pos < 64) ? ((HAL_GPIO1_ISR_GET() >> (pos - 32)) & 1) \
			: ((pos < 96) ? ((HAL_GPIO2_ISR_GET() >> (pos - 64)) & 1) \
			: ((pos < 128) ? ((HAL_GPIO3_ISR_GET() >> (pos - 96)) & 1) \
			: ((HAL_GPIO4_ISR_GET() >> (pos - 128)) & 1)))))

#define HAL_GPIO_INT_CLEAR(pos)		\
			((pos < 32) ? (HAL_GPIO_ISR_SET(1 << (pos))) \
			: ((pos < 64) ? (HAL_GPIO1_ISR_SET(1 << (pos-32))) \
			: ((pos < 96) ? (HAL_GPIO2_ISR_SET(1 << (pos-64))) \
			: ((pos < 128) ? (HAL_GPIO2_ISR_SET(1 << (pos-96))) \
			: (HAL_GPIO4_ISR_SET(1 << (pos-128)))))))


#define HAL_GPIO_BIT_ENABLE(pos,en)   \
    do {\
        ((pos < 32)           \
		? HAL_GPIO_EN_SET(((*(volatile unsigned long *)HAL_GPIO_EN_REG)&~(1<<(pos)))|((en)<<(pos)))\
		:((pos < 64)          \
		? HAL_GPIO1_EN_SET(((*(volatile unsigned long *)HAL_GPIO1_EN_REG)&~(1<<(pos - 32)))|((en)<<(pos - 32)))\
		:((pos < 96)          \
		? HAL_GPIO2_EN_SET(((*(volatile unsigned long *)HAL_GPIO2_EN_REG)&~(1<<(pos - 64)))|((en)<<(pos - 64)))\
		:((pos < 128)         \
		? HAL_GPIO3_EN_SET(((*(volatile unsigned long *)HAL_GPIO3_EN_REG)&~(1<<(pos - 96)))|((en)<<(pos - 96)))\
		: HAL_GPIO4_EN_SET(((*(volatile unsigned long *)HAL_GPIO4_EN_REG)&~(1<<(pos - 128)))|((en)<<(pos - 128)))))));\
    } while(0)
    
static inline int sgpio_request(unsigned gpio, const char *label)
{
    
//    HAL_GPIO_FUNC_ENABLE(gpio);
    return 0;
}

static inline int sgpio_direction_input(unsigned gpio)
{

    HAL_GPIO_BIT_DIR_SET(gpio, HAL_GPIO_I_DIR);
    HAL_GPIO_BIT_ENABLE(gpio, HAL_GPIO_ENABLE);
    return 0;
}

static inline int sgpio_direction_output(unsigned gpio, int level)
{


    HAL_GPIO_BIT_DIR_SET(gpio, HAL_GPIO_O_DIR);
    HAL_GPIO_BIT_ENABLE(gpio, HAL_GPIO_ENABLE);
    HAL_GPIO_BIT_SET(gpio, level?1:0);
    return 0;
}

static inline int sgpio_get_value(unsigned int gpio)
{

	return HAL_GPIO_BIT_GET(gpio);
}

static inline void sgpio_set_value(unsigned int gpio, int value)
{


	HAL_GPIO_BIT_SET(gpio, value?1:0);
}

//get interrupt status
static inline int sgpio_get_interrupt_status(int gpio)
{

    return HAL_GPIO_INT_STA_GET(gpio);
}

//clear interrupt status
static inline int sgpio_clear_interrupt_status(int gpio)
{

    HAL_GPIO_INT_CLEAR(gpio);
	return 0;
}

static inline int sgpio_interrupt_enable(int gpio)
{

    HAL_GPIO_INT_SET(gpio, 1);
    HAL_GPIO_INT_EDG_SET(gpio,1,1); //enable Rising and Falling Edge interrupt for SD card detect.
	return 0;
}

static inline int sgpio_interrupt_disable(int gpio)
{

    HAL_GPIO_INT_SET(gpio, 0);
	return 0;
}

static inline void sgpio_free(unsigned gpio)
{
    return ;
}

// ============================================================

#endif

#define MUTE_GPIO_ADDRESS   0xA7F9030C   
#define M39_IRQ_AUDIO            INT_ALI_AUDIO
static u8 g_s_ic_version;
static u32 g_s_mute_gpio_addr;
static u8 g_s_mute_value;

static	struct snd_pcm_substream *g_substream = NULL;
MODULE_AUTHOR("Xavier Chiang <xavier.chiang@alitech.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("M39 Series ASOC for ALSA");
MODULE_SUPPORTED_DEVICE("{{M39XX Series SOC}}");

static char *id;	/* ID for this card */

module_param(id, charp, 0444);
MODULE_PARM_DESC(id, "ID string for M39 Series ASOC.");
#define TXJPRINT KERN_ERR

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

struct m39_soc {
	struct snd_card *card;
	struct snd_pcm *pcm[2];
	long samplerate;
	struct audio_stream s[2];	/* playback & capture */

    //add by alex
    spinlock_t reg_lock;
    unsigned int dig_status;
	unsigned int dig_pcm_status;
};

static unsigned int rates[] = {
	/* 8000, 16000, */		/* M39 internal DAC not support 8k, 16k */ 
	32000, 44100, 48000,
};

static struct snd_pcm_hw_constraint_list hw_constraints_rates = {
	.count	= ARRAY_SIZE(rates),
	.list	= rates,
	.mask	= 0,
};

static struct platform_device *m39_snd_dev;


#undef DEBUG
#ifdef DEBUG
#define DPRINTK( s, arg... )  printk( "dma<%p>: " s, regs , ##arg )
#else
#define DPRINTK( x... )
#endif

static int m39_dev_free(struct snd_device *device)
{
	struct m39_soc *chip = device->device_data;

	printk(KERN_ERR "ALi m39_dev_free\n");
	kfree(chip);
	return 0;
}



/* add by mzhu 20110930 */

void disable_mute(unsigned long disabeMute)
{
    unsigned long *g_chipid_version_addr = NULL;
    unsigned long ul_reg_val = 0;
    int audioMuteGpioNO;

    printk(KERN_ERR "%s,%d\n", __FUNCTION__, __LINE__);

    /* Mapping the CHIP ID and Version register physical address to the virtual address */
    g_chipid_version_addr = (unsigned long*)ioremap((0x18000000 + 0), sizeof(unsigned long));

    /* Get the value of the CHIP ID and Version register */
    ul_reg_val = *g_chipid_version_addr;

    //printk(KERN_ALERT"\nali_s3921_init_machine,  chipid version = 0x%x\n", ul_reg_val);

    /* Get the chip package type value: BGA or QFP */
    ul_reg_val = (ul_reg_val >> 8) & 0x01;

    if (ul_reg_val == 0)
    {
        /* The Chip Package type is QFP, modify the sda pin and  scl pin for I2C driver */    
        audioMuteGpioNO=AUD_MUTE_GPIO_NO_QFP;
        printk(KERN_ERR "\n%s,%d, it is QFP package!!!!\n", __FUNCTION__, __LINE__);        
    }
    else
    {
        //printk(KERN_ALERT"\n ali_s3921_init_machine,  BGA  package \n");
        audioMuteGpioNO=AUD_MUTE_GPIO_NO_BGA;
        printk(KERN_ERR "\n%s,%d , it is BGA package!!!!\n", __FUNCTION__, __LINE__);
    }
    if(disabeMute==1)
   {
      sgpio_direction_output(audioMuteGpioNO, 1);
      sgpio_set_value(audioMuteGpioNO, 1);
      printk(KERN_ERR "\n%s,%d , Gpio number is %d, value is %d !!!!\n", __FUNCTION__, __LINE__, audioMuteGpioNO,disabeMute );
   }
    else
   {
      sgpio_direction_output(audioMuteGpioNO, 1);
      sgpio_set_value(audioMuteGpioNO, 0);
      printk(KERN_ERR "\n%s,%d , Gpio number is %d, value is %d !!!!\n", __FUNCTION__, __LINE__, audioMuteGpioNO,disabeMute );
   }
    
}

static struct timer_list mute_timer = TIMER_INITIALIZER(disable_mute, 0, 0);
static struct timer_list unmute_timer = TIMER_INITIALIZER(disable_mute, 0, 1);
/*end */
/**
 * 	sa1100_start_dma - submit a data buffer for DMA
 * 	@regs: identifier for the channel to use
 * 	@dma_ptr: buffer physical (or bus) start address
 * 	@size: buffer size
 *
 * 	This function hands the given data buffer to the hardware for DMA
 * 	access. If another buffer is already in flight then this buffer
 * 	will be queued so the DMA engine will switch to it automatically
 * 	when the previous one is done.  The DMA engine is actually toggling
 * 	between two buffers so at most 2 successful calls can be made before
 * 	one of them terminates and the callback function is called.
 *
 * 	The @regs identifier is provided by a successful call to
 * 	sa1100_request_dma().
 *
 * 	The @size must not be larger than %MAX_DMA_SIZE.  If a given buffer
 * 	is larger than that then it's the caller's responsibility to split
 * 	it into smaller chunks and submit them separately. If this is the
 * 	case then a @size of %CUT_DMA_SIZE is recommended to avoid ending
 * 	up with too small chunks. The callback function can be used to chain
 * 	submissions of buffer chunks.
 *
 * 	Error return values:
 * 	%-EOVERFLOW:	Given buffer size is too big.
 * 	%-EBUSY:	Both DMA buffers are already in use.
 * 	%-EAGAIN:	Both buffers were busy but one of them just completed
 * 			but the interrupt handler has to execute first.
 *
 * 	This function returs 0 on success.
 **/

static int m39_start_dma(dma_addr_t dma_ptr, u_int size)
{
	unsigned long flags;
	int i2soStatus,spoStatus;
	int ret = 0;
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
	printk(KERN_ERR "ALi m39_start_dma\n") ; 
     
	if (dma_ptr & 3)
		printk(KERN_WARNING "DMA: unaligned start address (0x%08lx)\n",
		       (unsigned long)dma_ptr);

	if (size > PCM_BUFF_SIZE*8)
		return -EOVERFLOW;

	local_irq_save(flags);

	i2soStatus = GetI2SODMAStatus();

	spoStatus = GetSPODMAStatus();
	
	/* If both I2SO SPO DMA buffers are started, there's nothing else we can do. */
	if (i2soStatus && spoStatus) {
		DPRINTK("I2SO and SPO DMA are busy !!!\n" );
		ret = -EBUSY;
		goto out;
	}

	if (M39_SPDIF_INTF == g_open_device_id) {
    
          //enable  SPO interface, disable I2SO Interface
         EnableI2SOInterface(FALSE);
         EnableSPOInterface(TRUE);
	    
	    // enable I2SO SPO DMA,disable I2SO DMA
	    StartSPODMA(FALSE);
         StartI2SODMA(TRUE);
	} else {

          //enable I2SO SPO interface
         EnableI2SOInterface(TRUE);
         EnableSPOInterface(TRUE);
	    
	    // enable I2SO SPO DMA
	    StartSPODMA(TRUE);
         StartI2SODMA(TRUE);
	}
     AudioConfiguratationPatch();

out:
	local_irq_restore(flags);
    mod_timer(&unmute_timer,  jiffies + msecs_to_jiffies(400));
	return ret;
}

static int m39_start_capture_dma(struct audio_stream *s, dma_addr_t dma_ptr, u32 size)
{
    unsigned long flags;

    if (0 != (dma_ptr & 3))
        printk(KERN_WARNING "DMA: unaligned start address (0x%08x)\n",(int)dma_ptr);

    if (size > (PCM_BUFF_SIZE * 8))
        return -EOVERFLOW;  

    if (0 != g_s_mute_gpio_addr) {
        gpio_direction_output(g_s_mute_gpio_addr, (g_s_mute_value == 0 ? 1 : 0));
    }
    mdelay(3);

    local_irq_save(flags);

	//SND_SET_BYTE(R_AUD_CTRL, SND_GET_BYTE(R_AUD_CTRL) | 0x03); //enable I2SO & SPO interface
   //enable I2SO SPO interface
   EnableI2SOInterface(TRUE);
   EnableSPOInterface(TRUE);
   
//	SND_SET_BYTE(R_DMA_CTRL, SND_GET_BYTE(R_DMA_CTRL) | 0x10); // enable I2SO DMA start
   // enable I2SO DMA start
   StartI2SODMA(TRUE);
	
    // I2S in:enable I2S in interface
   // SND_SET_BYTE(R_I2SI_INTF_CTRL_3, SND_GET_BYTE(R_I2SI_INTF_CTRL_3) | 0x01);
   EnableI2SIRXInterface(TRUE);

    local_irq_restore(flags);
    
    return 0;
}



static int audio_dma_request(struct audio_stream *s, void (*callback)(void *))
{
	int ret;
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
	printk(KERN_ERR "ALi audio_dma_request\n");

	//SND_SET_BYTE(R_AUD_INT, SND_GET_BYTE(R_AUD_INT) | (0xF0)); // clear INT status
      // clear INT status
      AUDREG_ClearAllAudioInterrupt();
   
	//SND_SET_BYTE(R_AUD_INT, SND_GET_BYTE(R_AUD_INT) & (0xF0)); // disable all INT
	
	// disable all INT
	AUDREG_DisableAllAudioInterrupt();
	ret = request_irq(M39_IRQ_AUDIO, (irq_handler_t)callback, 0, "audio", s);

	if (ret < 0)
		printk(KERN_ERR "unable to request irq, ret value is %d\n", ret);
	return ret;
}

static void audio_dma_free(struct audio_stream *s)
{
	printk(KERN_ERR "ALi audio_dma_free\n");

	free_irq(M39_IRQ_AUDIO, s);
}

static printBuff(uint8_t *ptr, int count)
{
   int i;
   for(i=0;i<count; i++)
   {
      if(i%16==0)
          printk(TXJPRINT "\n");
      printk(TXJPRINT "%2x,",  *(ptr+i));

   }
    printk(TXJPRINT "\n");
}

static u_int audio_get_dma_pos(struct audio_stream *s)
{
	struct snd_pcm_substream *substream = s->stream;
	struct snd_pcm_runtime *runtime = substream->runtime;
	unsigned int pos = 0;
    unsigned int offset = 0;
	unsigned long flags;
	// this must be called w/ interrupts locked out see dma-sa1100.c in the kernel
	
//	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);

	spin_lock_irqsave(&s->dma_lock, flags);
//	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);


    if (g_open_device_id == M39_PCM_INTF) {
         pos =16*AUDREG_GetDMACurrentIndex(AUD_SUB_OUT);
	    //pos = 2 * 8 * SND_GET_WORD(R_I2SO_DMA_CUR_INDEX);	// 64-bits unit -> 128-bits
    }

    if (g_open_device_id == M39_SPDIF_INTF) {
        //pos = 2 * 8 * SND_GET_WORD(R_SPO_DMA_CUR_INDEX);
        pos =16*AUDREG_GetDMACurrentIndex(AUD_SUB_SPDIFOUT);
    }

	spin_unlock_irqrestore(&s->dma_lock, flags);
//	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);

	offset = bytes_to_frames(runtime, pos);
	if (offset >= runtime->buffer_size)
		offset = 0;
//      printBuff(runtime->dma_area,128);
//     printk(TXJPRINT "!%s, line %d runtime->dma_area %x runtime->dma_area value %x, runtime->dma_addr %x !!!\n",__FUNCTION__,__LINE__,runtime->dma_area, *(runtime->dma_area+3),runtime->dma_addr);
//     PrintAudioConfiguratation();
//	printk(TXJPRINT "the GPIO %d is %d\n",AUD_MUTE_GPIO_NO_BGA,  sgpio_get_value(AUD_MUTE_GPIO_NO_BGA));

//	printk(TXJPRINT "%s  cur_pos(Bytes) = %d, offset(frame) = %d\n",__FUNCTION__,pos, offset);
	return offset;
}

/*
 * this stops the dma and clears the dma ptrs
 */
static void audio_stop_dma(struct audio_stream *s)
{
    //unsigned short vol = 0;
	unsigned long flags;
    struct snd_pcm_substream *substream = s->stream;
    int stream_id;
    int i = 0;
//    printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
    if (NULL == substream)
        return;
    
//    printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);

    stream_id = substream->pstr->stream;
//    printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);

	g_substream = NULL;
//	printk(KERN_ERR "ALi audio_stop_dma\n");
//	spin_lock_irqsave(&s->dma_lock, flags);
//   	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);

	s->active = 0;
	s->period = 0;

    if (M39_SPDIF_INTF == g_open_device_id) 
    {
//        LAST_INDEX = R_SPO_DMA_LAST_INDEX;
//        CUR_INDEX  = R_SPO_DMA_CUR_INDEX;
        printk(KERN_INFO "spdif dma_stop \n");
    }

    if (SNDRV_PCM_STREAM_PLAYBACK == stream_id) {   
        u16 last_index; 
        u16 cur_index;
        printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
        for(i=0; i<15; i++)
        {
            //last_index = SND_GET_WORD(LAST_INDEX);
            last_index = AUDREG_GetDMALastIndex(AUD_SUB_SPDIFOUT);
            cur_index  =AUDREG_GetDMACurrentIndex(AUD_SUB_SPDIFOUT);
            if(cur_index == last_index)
            {
            #if 0
                printk(KERN_INFO "%s :  success i=%d undrrun = %d\n", __func__ 
                        ,i,  SND_GET_BYTE(R_BUF_UNDRUN_CTRL));
                if(SND_GET_BYTE(R_BUF_UNDRUN_CTRL) & 0x1 )
                {
                    SND_SET_BYTE(R_BUF_UNDRUN_CTRL, SND_GET_BYTE(R_BUF_UNDRUN_CTRL) | 0x1); 
                    SND_SET_BYTE(R_AUD_CTRL, SND_GET_BYTE(R_AUD_CTRL) & ~0x03); //disable I2SO SPO interface
                    printk(KERN_INFO "R_BUF_UNDRUN_CTRL = %02x, R_AUD_CTRL =%02x\n", 
                            SND_GET_BYTE(R_BUF_UNDRUN_CTRL),  SND_GET_BYTE(R_AUD_CTRL));
                }
             #else
             
                  printk(KERN_ERR"%s :  success i=%d undrrun = %d\n", __FUNCTION__ 
                        ,i, GetDMABufUnderRunIntStatus());
                if(CheckI2SODMABufUnderRunIntStatus())
                {
                     //disable I2SO SPO interface
                     EnableI2SOInterface(FALSE);
                     EnableSPOInterface(FALSE);
                    printk(KERN_ERR "GetDMABufUnderRunIntStatus = %x, GetInterfaceEnableStatus =%x\n", 
                            GetDMABufUnderRunIntStatus(),  GetInterfaceEnableStatus);
                }           
             #endif
                break;
            }
            mdelay(50);
        }

        //SND_SET_BYTE(R_DMA_CTRL, SND_GET_BYTE(R_DMA_CTRL) & (~0x11));
         // stop I2SO SPO DMA
	    StartSPODMA(FALSE);
         StartI2SODMA(FALSE);
        //SND_SET_BYTE(R_BUF_UNDRUN_CTRL, SND_GET_BYTE(R_BUF_UNDRUN_CTRL) | 0x01);
   } else {
        // disabe the I2S in serial data input enable
        EnableI2SIRXInterface(FALSE);
        StartI2SIRXDMA(FALSE);

        // set the pinmux for I2S in
        //SYS_IOW_DW(0x8c, SYS_IOR_DW(0x8c) & (~(1 << 2)) | 0x0); 
    }

/******************* For DB-M3911-01V02  ********************/
/******************* For DB-M3911-01V02  ********************/
    if (0 != g_s_mute_gpio_addr) {
        gpio_direction_output(g_s_mute_gpio_addr, (g_s_mute_value == 0 ? 0 : 1));
    }
    
 //   mdelay(20);


//    SND_SET_BYTE(R_TARGET_VOLUME, vol);
//	spin_unlock_irqrestore(&s->dma_lock, flags);
 	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);

}

static void audio_process_dma(struct audio_stream *s)
{
	struct snd_pcm_substream *substream = s->stream;
	struct snd_pcm_runtime *runtime;
    int stream_id;
	unsigned int dma_size;		
	unsigned int offset;
	int ret;
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
    if (NULL == substream)
        return;
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);

    runtime = substream->runtime;
    	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);

    stream_id = substream->pstr->stream;
        
	g_substream = substream;
	printk(KERN_ERR "ALi audio_process_dma: runtime->periods = %d, runtime->buffer_size= %ld\n", \
		runtime->periods, runtime->buffer_size);

#if 1
	/* must be set here - only valid for running streams, not for forced_clock dma fills  */
	//while (s->active && s->periods < runtime->periods) 
    if (s->active) {
        	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
		dma_size = frames_to_bytes(runtime, runtime->period_size);
        	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
		if (s->old_offset) {
			/* a little trick, we need resume from old position */
			offset = frames_to_bytes(runtime, s->old_offset - 1);
			s->old_offset = 0;
			s->periods = 0;
			s->period = offset / dma_size;
			offset %= dma_size;
			dma_size = dma_size - offset;
			printk("ALi audio_process_dma: old_offset dma_size = %d\n", dma_size);
			if (!dma_size)
				return;	//continue;		/* special case */
		} else {
			printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
			offset = dma_size * s->period;
		//	snd_BUG_ON(dma_size > (PCM_BUFF_SIZE * 8));//snd_BUG_ON(dma_size > DMA_BUF_SIZE);
				printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
		}

		printk("ALi audio_process_dma: dma_size = %d, s->period = %d, offset = %d\n",\
			dma_size, s->period, offset);
        
        if (stream_id == SNDRV_PCM_STREAM_PLAYBACK) {
            //SND_SET_WORD(R_LAST_VALID_INDEX, (frames_to_bytes(runtime,
            //runtime->buffer_size) / 16) - 1);

            AUDREG_SetDMALastIndex(AUD_SUB_OUT, (frames_to_bytes(runtime,
            runtime->buffer_size) / 16) - 1);
            
            /* M39 128-bits base */
            //SND_SET_WORD(R_SPO_DMA_LAST_INDEX, (runtime->buffer_size / 2 - 1));
            //SND_SET_WORD(R_SPO_DMA_LAST_INDEX, (frames_to_bytes(runtime, 
           // runtime->buffer_size) / 16) - 1);

           AUDREG_SetDMALastIndex(AUD_SUB_SPDIFOUT, (frames_to_bytes(runtime,
            runtime->buffer_size) / 16) - 1);

            ret = m39_start_dma(runtime->dma_addr + offset, dma_size);
        } else {
            ret = m39_start_capture_dma(s, runtime->dma_addr + offset, dma_size);
        }
        
		if (ret) {
			printk(KERN_ERR "audio_process_dma: cannot queue DMA buffer (%i)\n", ret);
			return;
		}

		/* s->period++;
		s->period %= runtime->periods;
		s->periods++; */
	}
#else
	dma_size = frames_to_bytes(runtime, runtime->period_size);
	offset = dma_size * s->period;
	SND_SET_WORD(R_LAST_VALID_INDEX, (offset / 8));
	SND_SET_BYTE(R_AUD_CTRL, SND_GET_BYTE(R_AUD_CTRL) | 0x01); //enable I2SO interface
	SND_SET_BYTE(R_DMA_CTRL, SND_GET_BYTE(R_DMA_CTRL) | 0x10); // enable I2S DMA
#endif
}

irqreturn_t audio_dma_callback(int irq, void *dev_id)
{   
//printk("audio_dma_callback done! \n");

	struct audio_stream *s = dev_id;
	struct snd_pcm_substream *substream = s->stream;
	struct snd_pcm_runtime *runtime = substream->runtime;
	unsigned int write_idx;
	//unsigned int read_idx, diff;	/* for debug */
 	unsigned long flags;
	/* 
	 * If we are getting a callback for an active stream then we inform
	 * the PCM middle layer we've finished a period
	 */
	//LOG(" ==%s===\n",__func__) ; 
	// NOTE: if condition cause buf underrun
//	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);

	spin_lock_irqsave(&s->dma_lock, flags);
//	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);


    //if (SND_GET_BYTE(R_AUD_INT) & 0x11)
#if 0
   if ((SND_GET_BYTE(R_AUD_INT) & 0x11) == 0x11) //I2S int enable and int status
	{   

        /* buffer under run come, disable I2SO_EN SPO_EN*/
	    if ((SND_GET_BYTE(R_BUF_UNDRUN_CTRL) & 0x01) == 0x01) 
        {
            printk("%s :buffer under run come\n", __func__);
            SND_SET_BYTE(R_AUD_CTRL, SND_GET_BYTE(R_AUD_CTRL) & (~ 0x03));
        }
    
		write_idx=runtime->control->appl_ptr%(runtime->buffer_size); 

//        printk("pcm: runtime->status->hw_ptr = 0x%x, runtime->control->appl_ptr = 0x%x, write_idx = 0x%x\n", 
//            runtime->status->hw_ptr, runtime->control->appl_ptr, write_idx);
        
		/* 
		 * Restriction: R_LAST_VALID_INDEX can't be write to zero,
		 * but can be wrote to I2SO_DMA_LEN while normal playing.
		*/ 
		if (write_idx == 0)
			write_idx = runtime->buffer_size;

		/* M39 128-bits base */
		//SND_SET_WORD(R_LAST_VALID_INDEX, write_idx / 2);
        SND_SET_WORD(R_LAST_VALID_INDEX, frames_to_bytes(runtime, write_idx) / 16); //unit:128bits

        /* buffer under run come, enable I2SO_EN SPO_EN & clear the under run status*/
	    if ((SND_GET_BYTE(R_BUF_UNDRUN_CTRL) & 0x01) == 0x01) 
        {
            SND_SET_BYTE(R_AUD_CTRL, SND_GET_BYTE(R_AUD_CTRL) | 0x03);
            SND_SET_BYTE(R_BUF_UNDRUN_CTRL, SND_GET_BYTE(R_BUF_UNDRUN_CTRL) | 0x01); 
        }        
         
		SND_SET_BYTE(R_AUD_INT, SND_GET_BYTE(R_AUD_INT) | 0x10);

	}

	/* spdif sample output threshold */
    if (SND_GET_BYTE(R_AUD_INT) & 0x44){
		write_idx = runtime->control->appl_ptr % (runtime->buffer_size); 

		if (write_idx == 0)
			write_idx = runtime->buffer_size;

        //SND_SET_WORD(R_SPO_DMA_LAST_INDEX, write_idx / 2);
		SND_SET_WORD(R_SPO_DMA_LAST_INDEX, frames_to_bytes(runtime, write_idx) / 16); //unit:128bits

		// write 1 (bit 6) to clear
		SND_SET_BYTE(R_AUD_INT, SND_GET_BYTE(R_AUD_INT) | 0x40);
	}

    /* under run resume interrupt */
	if ((SND_GET_BYTE(R_AUD_INT) & 0x88) == 0x88) 
    {
		SND_SET_BYTE(R_AUD_INT, SND_GET_BYTE(R_AUD_INT) | 0x80);
    }
#else
   if (CheckI2SOSampCountInt()&&CheckI2SOSampCountIntStatus()) //I2S int enable and int status
	{   
       unsigned int i2SODMABufUnderRunIntStatus;
       
        /* buffer under run come, disable I2SO_EN SPO_EN*/
        
        i2SODMABufUnderRunIntStatus=CheckI2SODMABufUnderRunIntStatus();
	    if (i2SODMABufUnderRunIntStatus) 
        {
            printk("%s :buffer under run come\n", __FUNCTION__);
            //disable I2SO SPO interface
            EnableI2SOInterface(FALSE);
            EnableSPOInterface(FALSE);
        }
		write_idx=runtime->control->appl_ptr%(runtime->buffer_size); 

 //       printk("pcm: runtime->status->hw_ptr = 0x%x, runtime->control->appl_ptr = 0x%x, write_idx = 0x%x\n", 
 //           runtime->status->hw_ptr, runtime->control->appl_ptr, write_idx);
        
		/* 
		 * Restriction: R_LAST_VALID_INDEX can't be write to zero,
		 * but can be wrote to I2SO_DMA_LEN while normal playing.
		*/ 
		if (write_idx == 0)
			write_idx = runtime->buffer_size;
//    	    printk(TXJPRINT "!!!infunction %s, line %d write_idx is %d !!!\n",__FUNCTION__,__LINE__, write_idx);

		/* M39 128-bits base */
		//SND_SET_WORD(R_LAST_VALID_INDEX, write_idx / 2);
        AUDREG_SetDMALastIndex(AUD_SUB_OUT,  frames_to_bytes(runtime, write_idx) / 16); //unit:128bits
  
        /* buffer under run come, enable I2SO_EN SPO_EN & clear the under run status*/
	    if (i2SODMABufUnderRunIntStatus) 
        {
            EnableI2SOInterface(TRUE);
            EnableSPOInterface(TRUE);
//            EnableI2SOSampCountInt(TRUE);
//            EnableSPOSampCountInt(TRUE);            
        }     
	}
	/* spdif sample output threshold */
    if (CheckSPOSampCountInt()&&CheckSPOSampCountIntStatus())
    {
		write_idx = runtime->control->appl_ptr % (runtime->buffer_size); 

		if (write_idx == 0)
			write_idx = runtime->buffer_size;

        //SND_SET_WORD(R_SPO_DMA_LAST_INDEX, write_idx / 2);
		AUDREG_SetDMALastIndex(AUD_SUB_SPDIFOUT, frames_to_bytes(runtime, write_idx) / 16); //unit:128bits

	}
    /* under run resume interrupt */
    CheckI2SODMABufUnderRunIntStatus();
    CheckSPODMABufUnderRunIntStatus();
    EnableI2SODMAUnderRunInt(TRUE);
    EnableSPODMAUnderRunInt(TRUE);

#endif
	spin_unlock_irqrestore(&s->dma_lock, flags);
//     CheckI2SOSampCountIntStatus();
//	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
	
	snd_pcm_period_elapsed(substream);

	return IRQ_HANDLED; 
}

/* }}} */

/* {{{ PCM setting */

/* {{{ trigger & timer */

static int snd_m39_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct m39_soc *chip =substream->runtime->private_data;
	int stream_id = substream->pstr->stream;
	struct audio_stream *s = &chip->s[stream_id];
	struct audio_stream *s1 = &chip->s[stream_id ^ 1];
	int err = 0;
	printk(TXJPRINT "!!!infunction %s, line %d cmd is %d !!!\n",__FUNCTION__,__LINE__, cmd);
	/* note local interrupts are already disabled in the midlevel code */
	spin_lock(&s->dma_lock);
//   	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		/* now we need to make sure a record only stream has a clock */
		if (stream_id == SNDRV_PCM_STREAM_CAPTURE && !s1->active) {
			/* we need to force fill the xmit DMA with zeros */
			s1->tx_spin = 1;
			audio_process_dma(s1);
			printk(KERN_ERR "ALi snd_m39_pcm_trigger CAPTURE\n");
		}
		/* this case is when you were recording then you turn on a
		 * playback stream so we stop (also clears it) the dma first,
		 * clear the sync flag and then we let it turned on
		 */		
		else {
 			s->tx_spin = 0;
 		}

		/* requested stream startup */
		s->active = 1;
		printk("snd_m39_pcm_trigger: SNDRV_PCM_TRIGGER_START\n");
		audio_process_dma(s);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		/* requested stream shutdown */
        printk("snd_m39_pcm_trigger: SNDRV_PCM_TRIGGER_STOP s\n");
		audio_stop_dma(s);
		
		/*
		 * now we need to make sure a record only stream has a clock
		 * so if we're stopping a playback with an active capture
		 * we need to turn the 0 fill dma on for the xmit side
		 */
		if (stream_id == SNDRV_PCM_STREAM_PLAYBACK && s1->active) {
			/* we need to force fill the xmit DMA with zeros */
			s->tx_spin = 1;
			printk("snd_m39_pcm_trigger: SNDRV_PCM_TRIGGER_STOP\n");
			audio_process_dma(s);
		}
		/*
		 * we killed a capture only stream, so we should also kill
		 * the zero fill transmit
		 */
		else {
			if (s1->tx_spin) {
				s1->tx_spin = 0;
                printk("snd_m39_pcm_trigger: SNDRV_PCM_TRIGGER_STOP s1\n");
				audio_stop_dma(s1);
			}
		}
		
		break;
	case SNDRV_PCM_TRIGGER_SUSPEND:
		s->active = 0;
		s->old_offset = audio_get_dma_pos(s) + 1;
		s->periods = 0;
		break;
	case SNDRV_PCM_TRIGGER_RESUME:
		s->active = 1;
		s->tx_spin = 0;
		printk("snd_m39_pcm_trigger: SNDRV_PCM_TRIGGER_RESUME\n");
		audio_process_dma(s);
		if (stream_id == SNDRV_PCM_STREAM_CAPTURE && !s1->active) {
			s1->tx_spin = 1;
			audio_process_dma(s1);
		}
		break;
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		s->active = 0;
		if (stream_id == SNDRV_PCM_STREAM_PLAYBACK) {
			if (s1->active) {
				s->tx_spin = 1;
				s->old_offset = audio_get_dma_pos(s) + 1;	
				printk("snd_m39_pcm_trigger: SNDRV_PCM_TRIGGER_PAUSE_PUSH\n");
				audio_process_dma(s);
			}
		} else {
			if (s1->tx_spin) {
				s1->tx_spin = 0;
			}
		}
		break;
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		s->active = 1;
		if (s->old_offset) {
			s->tx_spin = 0;
			printk("snd_m39_pcm_trigger: SNDRV_PCM_TRIGGER_PAUSE_RELEASE\n");
			audio_process_dma(s);
			break;
		}
		if (stream_id == SNDRV_PCM_STREAM_CAPTURE && !s1->active) {
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

static int snd_m39_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct m39_soc *chip = substream->runtime->private_data;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct audio_stream *s = &chip->s[substream->pstr->stream];
    
    snd_pcm_format_t format = runtime->format;
    unsigned int channels = runtime->channels;
    unsigned int sample_bits = runtime->sample_bits;
    int stream_id = substream->pstr->stream;
    
#ifdef CONFIG_HDMI_ALI
    ALSA2HDMI_INFO hdmi_audio_config;
#endif
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);    
  
        /* set requested samplerate */
     chip->samplerate = runtime->rate; //txj add
	s->period = 0;
	s->periods = 0;
	s->old_offset = 0;
    
#ifdef CONFIG_HDMI_ALI
    
    // Test to update Audio       
    hdmi_audio_config.user_audio_out    = AUD_USR_LPCM_OUT;			             
    hdmi_audio_config.coding_type       = AUD_CODING_PCM;        
    hdmi_audio_config.channel_count     = 2;	
    switch(runtime->rate)
    {
        case 32000:  hdmi_audio_config.sample_rate   = AUD_SAMPLERATE_32KHZ;     break;
        case 44100:  hdmi_audio_config.sample_rate   = AUD_SAMPLERATE_44_1KHZ;   break;
        case 88200:  hdmi_audio_config.sample_rate   = AUD_SAMPLERATE_88_2KHZ;   break;
        case 176400: hdmi_audio_config.sample_rate   = AUD_SAMPLERATE_176_4KHZ;  break;     
        case 48000:  hdmi_audio_config.sample_rate   = AUD_SAMPLERATE_48KHZ;     break;
        case 96000:  hdmi_audio_config.sample_rate   = AUD_SAMPLERATE_96KHZ;     break;
        case 192000: hdmi_audio_config.sample_rate   = AUD_SAMPLERATE_192KHZ;    break;   
        default:    printk(KERN_ALERT "%s: hdmi unknown support sampling rate %d Hz\n", __FUNCTION__, runtime->rate); 
    }   
    hdmi_audio_config.i2s_format        = I2S_FMT_I2S;	      
    hdmi_audio_config.lrck_hi_left      = true;	
    if (stream_id == SNDRV_PCM_STREAM_PLAYBACK) {    
        hdmi_audio_config.word_length       = I2S_FMT_WLENGTH_24;		//playback use 24bits
    } else {
        hdmi_audio_config.word_length       = I2S_FMT_WLENGTH_16;		//capture use 16bits
    }
    hdmi_audio_config.channel_status.data_uint32 = SND_GET_DWORD(R_SPO_CS);
    ali_hdmi_set_audio_info(&hdmi_audio_config );   
#endif       

	return 0;
}

static snd_pcm_uframes_t snd_m39_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct m39_soc *chip = substream->runtime->private_data;
//	printk(KERN_ERR "ALi snd_m39_pcm_pointer\n");
	return audio_get_dma_pos(&chip->s[substream->pstr->stream]);
}






////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////// 


static struct snd_pcm_hardware snd_m39_pcm_capture;
static struct snd_pcm_hardware snd_m39_pcm_playback;

static int snd_card_m39_pcm_open(struct snd_pcm_substream *substream)
{
	struct m39_soc *chip = substream->runtime->private_data;
	struct snd_pcm_runtime *runtime = substream->runtime;
  
	int stream_id = substream->pstr->stream;
	int err;
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
	printk("ALi snd_card_m39_pcm_open, stream_id = %d\n", stream_id);
    
	chip->s[stream_id].stream = substream;

    	/* Setup DMA stuff */
	chip->s[SNDRV_PCM_STREAM_PLAYBACK].id = "M39_SND out";
	chip->s[SNDRV_PCM_STREAM_PLAYBACK].stream_id = SNDRV_PCM_STREAM_PLAYBACK;

	chip->s[SNDRV_PCM_STREAM_CAPTURE].id = "M39_I2S in";
	chip->s[SNDRV_PCM_STREAM_CAPTURE].stream_id = SNDRV_PCM_STREAM_CAPTURE;
	if ((err = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS)) < 0)
		return err;
	if ((err = snd_pcm_hw_constraint_list(runtime, 0, SNDRV_PCM_HW_PARAM_RATE, &hw_constraints_rates)) < 0)
		return err;

	return 0;
}



static int snd_card_m39_pcm_close(struct snd_pcm_substream *substream)
{
	struct m39_soc *chip = substream->runtime->private_data;

	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
	chip->s[substream->pstr->stream].stream = NULL;
	return 0;
}

/* {{{ HW params & free */

static int snd_m39_pcm_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *hw_params)
{
	int ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(hw_params));
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct m39_soc *chip = substream->runtime->private_data;
	struct audio_stream *s = &chip->s[substream->pstr->stream];
    int stream_id = substream->pstr->stream;
	//void * spo_dma_addr = NULL;
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
    #if 0
	s->pcm_buff_base = (u32 *)(((u32)runtime->dma_addr + 15) & 0xfffffff0);

    if (stream_id == SNDRV_PCM_STREAM_PLAYBACK) { 
        snd_m39_i2s_channel_base((u32)s->pcm_buff_base);
	printk(TXJPRINT "!!!infunction %s, pcm_buff_base %d !!!\n",__FUNCTION__,(u32)s->pcm_buff_base);
        /* M39 128 bits base */
        snd_m39_i2s_channel_len((runtime->dma_bytes) >> 4);
	printk(TXJPRINT "!!!infunction %s, dma_bytes %d !!!\n",__FUNCTION__,(runtime->dma_bytes) >> 4);

        g_open_device_id = M39_PCM_INTF;

        //SND_SET_WORD(R_LAST_VALID_INDEX, 0);
   
        /*spo_dma_addr = kzalloc(runtime->dma_bytes + 0x0f, GFP_KERNEL);
        s->es_buff_base = (u32*)(((u32)spo_dma_addr + 15) & 0xfffffff0);
        snd_m39_spo_channel_base((u32)s->es_buff_base);*/

        snd_m39_spo_channel_base((u32)s->pcm_buff_base);
        snd_m39_spo_channel_len((runtime->dma_bytes) >> 4);
    } 
#endif
    printk("runtime->dma_area = 0x%08lx dma_addr_t = 0x%08lx dma_size = %d "
    "runtime->min_align %ld\n",
    (unsigned long)runtime->dma_area,
    (unsigned long)runtime->dma_addr, runtime->dma_bytes,
    runtime->min_align);

	printk("ALi snd_m39_pcm_hw_params, re = %d\n", ret);

	return 0;
}

static int snd_m39_pcm_hw_free(struct snd_pcm_substream *substream)
{
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
	return snd_pcm_lib_free_pages(substream);
}




static int snd_m39_pcm_mmap(struct snd_pcm_substream *substream, 
				struct vm_area_struct *vma)
{
#if 0
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
	return remap_pfn_range(vma, vma->vm_start,
		substream->dma_buffer.addr >> PAGE_SHIFT,
		vma->vm_end - vma->vm_start, vma->vm_page_prot);
#else
	struct snd_pcm_runtime *runtime = substream->runtime;

	return dma_mmap_writecombine(substream->pcm->card->dev, vma,
				     runtime->dma_area,
				     runtime->dma_addr,
				     runtime->dma_bytes);
#endif
}

static int snd_m39_pcm_ack(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	unsigned int write_idx;
  	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);  
	write_idx = runtime->control->appl_ptr;//%(runtime->buffer_size);

//printk("%s write_idx = %ld , runtime->status->hw_ptr = %ld \n", __func__, 
//    write_idx, runtime->status->hw_ptr);

    return 0;
}

/* }}} */




#ifdef CONFIG_PM

static int snd_m39_pcm_suspend(struct platform_device *devptr,
				      pm_message_t state)
{
//for phil
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
	//add by martin.zhu 20110727
	if(g_substream != NULL)
	{
		printk("%s: snd_pcm_stop \n", __func__);
		snd_pcm_stop(g_substream, SNDRV_PCM_STATE_PREPARED);
	}
	return 0;
}

static int snd_m39_pcm_resume(struct platform_device *devptr)
{
//for phil    
	//add by martin.zhu 20110727
	return 0;
}
#endif /* COMFIG_PM */

void snd_m39_pcm_free(struct snd_card *card)
{
	struct m39_soc *chip = card->private_data;

	printk(KERN_ERR "ALi snd_m39_pcm_free\n");
	audio_dma_free(&chip->s[SNDRV_PCM_STREAM_PLAYBACK]);
	//audio_dma_free(&chip->s[SNDRV_PCM_STREAM_CAPTURE]);
}

#define KERNEL_2_6_28   0
#define KERNEL_2_6_32   1




#define M39_SND_DRIVER		"m39_snd"


static struct snd_pcm_hardware snd_asoc_platform_m39xx_pcm_playback =
{
	.info			= (SNDRV_PCM_INFO_INTERLEAVED |
				   SNDRV_PCM_INFO_BLOCK_TRANSFER |
				   SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_MMAP_VALID |
				   SNDRV_PCM_INFO_PAUSE | SNDRV_PCM_INFO_RESUME),
	.formats		= (SNDRV_PCM_FMTBIT_U8 | SNDRV_PCM_FMTBIT_S8| 
                                SNDRV_PCM_FMTBIT_S24_LE| 
				   SNDRV_PCM_FMTBIT_S16_LE| 
				   SNDRV_PCM_FMTBIT_S32_LE),

	.rates			= (/* SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |\ */
                   SNDRV_PCM_RATE_32000 |\
				   SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000 |\
				   SNDRV_PCM_RATE_KNOT),
	.rate_min		= 32000,			/* 8000, */
	.rate_max		= 48000,
	.channels_min		= 1,
	.channels_max		= 2,		/* M39 only 2 channels */
	.buffer_bytes_max	=PCM_BUFF_SIZE * 8,
	.periods_min		= 8,		// 2,
	.periods_max		= 10,		// (buffer_bytes_max / period_bytes_min)
	.period_bytes_min	= M39_PERIOD_MIN_BYTES,
	.period_bytes_max	= PCM_BUFF_SIZE * 8,
};

static int asoc_platform_m39xx_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct m39_soc *chip;
    	int stream_id = substream->pstr->stream;
	int err;
    
	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (chip == NULL)
		return -ENOMEM;

	runtime->private_data = chip;
       chip->card=substream->pcm->card;
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
	printk("ALi snd_card_m39_pcm_open, stream_id = %d\n", stream_id);
       
	chip->s[stream_id].stream = substream;

	if (stream_id == SNDRV_PCM_STREAM_PLAYBACK) {
	    snd_soc_set_runtime_hwparams(substream, &snd_asoc_platform_m39xx_pcm_playback);
        /* reset snd for dma occurs error,set 24-bit for playback */
                /* setup DMA controller */
		audio_dma_request(&chip->s[SNDRV_PCM_STREAM_PLAYBACK], (void *)audio_dma_callback);
                
	} else {
        	 snd_soc_set_runtime_hwparams(substream, &snd_asoc_platform_m39xx_pcm_playback);
                /* reset snd for dma occurs error,set 16-bit for playback */
	}
    	/* Setup DMA stuff */
	chip->s[SNDRV_PCM_STREAM_PLAYBACK].id = "M39_SND out";
	chip->s[SNDRV_PCM_STREAM_PLAYBACK].stream_id = SNDRV_PCM_STREAM_PLAYBACK;

	chip->s[SNDRV_PCM_STREAM_CAPTURE].id = "M39_I2S in";
	chip->s[SNDRV_PCM_STREAM_CAPTURE].stream_id = SNDRV_PCM_STREAM_CAPTURE;
    
	if ((err = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS)) < 0)
		return err;
	if ((err = snd_pcm_hw_constraint_list(runtime, 0, SNDRV_PCM_HW_PARAM_RATE, &hw_constraints_rates)) < 0)
		return err;
    
//        g_s_mute_gpio_addr = *(volatile u32 *)MUTE_GPIO_ADDRESS;
//        g_s_mute_value = *(volatile u8 *)(MUTE_GPIO_ADDRESS + 4);

        printk(">>> g_s_mute_gpio_addr = 0x%x, g_s_mute_value = 0x%x \n",
        g_s_mute_gpio_addr, g_s_mute_value);
    
        return 0;
}

static int asoc_platform_m39xx_pcm_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct m39_soc *prtd = runtime->private_data;

	printk(KERN_ERR "ALi snd_m39_pcm_free\n");
	audio_dma_free(&prtd->s[SNDRV_PCM_STREAM_PLAYBACK]);
	kfree(prtd);

	return 0;
}

static struct snd_pcm_ops asoc_platform_m39xx_pcm_playback_ops = {
	.open		= asoc_platform_m39xx_pcm_open,
	.close		= asoc_platform_m39xx_pcm_close,
	.ioctl			= snd_pcm_lib_ioctl,
	.hw_params	= snd_m39_pcm_hw_params,
	.hw_free		= snd_m39_pcm_hw_free,
	.prepare		= snd_m39_pcm_prepare,
	.trigger		= snd_m39_pcm_trigger,
	.pointer		= snd_m39_pcm_pointer,
	.mmap		= snd_m39_pcm_mmap,
};

static struct snd_pcm_ops asoc_platform_m39xx_pcm_capture_ops = {
	.open		= asoc_platform_m39xx_pcm_open,
	.close		= asoc_platform_m39xx_pcm_close,
	.ioctl			= snd_pcm_lib_ioctl,
	.hw_params	= snd_m39_pcm_hw_params,
	.hw_free		= snd_m39_pcm_hw_free,
	.prepare		= snd_m39_pcm_prepare,
	.trigger		= snd_m39_pcm_trigger,
	.pointer		= snd_m39_pcm_pointer,
	.mmap		= snd_m39_pcm_mmap,
};

static u64 m39_pcm_dmamask = DMA_BIT_MASK(32);

static int m39_pcm_preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	size_t size = snd_asoc_platform_m39xx_pcm_playback.buffer_bytes_max;

	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;

   #if 0 //txj mask for we use it for hardware limit
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
     printk(KERN_ERR "in Function %s, line %d , g_audio_static_buf is %x buf->area is %x, buf->addr is %x buf->bytes is %d  \n", __FUNCTION__,__LINE__, g_audio_static_buf,buf->area, buf->addr, buf->bytes);
  
#endif
	return 0;
}



static int asoc_platform_m39xx_pcm_new(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_card *card = rtd->card->snd_card;
	struct snd_pcm *pcm = rtd->pcm;
	int ret = 0;

	if (!card->dev->dma_mask)
		card->dev->dma_mask = &m39_pcm_dmamask;
	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask =DMA_BIT_MASK(32);

	if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream) {
		ret = m39_pcm_preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_PLAYBACK);
		if (ret)
			return ret;
	}

	if (pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream) {
		ret = m39_pcm_preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_CAPTURE);
		if (ret)
			return ret;
	}

	return 0;
}




static void asoc_platform_m39xx_pcm_free(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;
	int stream;

	for (stream = 0; stream < SNDRV_PCM_STREAM_LAST; stream++) {
		substream = pcm->streams[stream].substream;
		if (!substream)
			continue;

		buf = &substream->dma_buffer;
		if (!buf->area)
			continue;

		dma_free_writecombine(pcm->card->dev, buf->bytes,
				      buf->area, buf->addr);
		buf->area = NULL;
	}
}

 static struct snd_soc_platform_driver asoc_platform_m39xx = {
		.ops		= &asoc_platform_m39xx_pcm_playback_ops,
		.pcm_new	= asoc_platform_m39xx_pcm_new,
		.pcm_free	= asoc_platform_m39xx_pcm_free,
};


static int __devinit asoc_m39xx_platform_probe(struct platform_device *pdev)
{
	printk(KERN_ERR "in Function %s, line %d \n", __FUNCTION__,__LINE__);

	return snd_soc_register_platform(&pdev->dev, &asoc_platform_m39xx);
}

static int __devexit asoc_m39xx_platform_remove(struct platform_device *pdev)
{
	printk(KERN_ERR "in Function %s, line %d \n", __FUNCTION__,__LINE__);

	snd_soc_unregister_platform(&pdev->dev);
	return 0;
}

static struct platform_driver asoc_m39xx_pcm_driver = {
	.driver = {
	    	       .name = 	"asoc_platform_m39xx",
			.owner = THIS_MODULE,
	},

	.probe = asoc_m39xx_platform_probe,
	.remove = __devexit_p(asoc_m39xx_platform_remove),
};

module_platform_driver(asoc_m39xx_pcm_driver);


MODULE_AUTHOR("shenzhen os team base on xavier chang ");
MODULE_DESCRIPTION("ALi PCM Asoc driver");
MODULE_LICENSE("GPL");

/* }}} */

/*
 * Local variables:
 * indent-tabs-mode: t
 * End:
 */
