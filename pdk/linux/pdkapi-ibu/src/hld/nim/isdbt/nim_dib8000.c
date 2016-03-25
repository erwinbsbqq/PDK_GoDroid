#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_gpio.h>
//#include <bus/spi/spi.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim_tuner.h>
#include <hld/nim/nim.h>
#include <bus/i2c/i2c.h>
#include <math.h>
#include <api/libosd/osd_lib.h>

#include "nim_dib8000.h"

#include <adapter/frontend.h>
#include <adapter/frontend_tune.h>
#include <adapter/sip.h>
#include <sip/dib8090.h>
#include <demod/dib8000.h>
#include <adapter/databus.h>
#include <adapter/demod.h>
#include <adapter/frontend.h>

#include "dibcom/monitor/monitor.h"

//linux
#include <fcntl.h>
#include <sys/types.h>

//#define CONFIG_DEBUG

#ifdef CONFIG_DEBUG
	#define NIM_PRINTF  printf
	#define ALI_PRINTF  printf
#else
	#define NIM_PRINTF(...)
	#define ALI_PRINTF(...)
#endif
#if 1
	#define US_TICKS        (SYS_CPU_CLOCK / 2000000)
	#define WAIT_1ms        (1000* US_TICKS)
	#define WAIT_100ms	1
//OSAL_ID f_IIC_Sema_ID = OSAL_INVALID_ID;
ID f_dib8000_CC_tracking_task_id = OSAL_INVALID_ID;
/////////////////////////////////////////////////////////////////
static OSAL_ID dib8000_i2c_mutex_id = OSAL_INVALID_ID;
void dib8000_i2c_mutex_enter(void)
{	

	if(dib8000_i2c_mutex_id == OSAL_INVALID_ID)		
		dib8000_i2c_mutex_id = osal_mutex_create();	

	osal_mutex_lock(dib8000_i2c_mutex_id, OSAL_WAIT_FOREVER_TIME);

}

void dib8000_i2c_mutex_exit(void)
{	

	osal_mutex_unlock(dib8000_i2c_mutex_id);
}



static OSAL_ID dib8000_nim_mutex_id = OSAL_INVALID_ID;

void dib8000_nim_mutex_enter(void)
{	

	if(dib8000_nim_mutex_id == OSAL_INVALID_ID)		
		dib8000_nim_mutex_id = osal_mutex_create();	

	osal_mutex_lock(dib8000_nim_mutex_id, OSAL_WAIT_FOREVER_TIME);

}

void dib8000_nim_mutex_exit(void)
{	

	osal_mutex_unlock(dib8000_nim_mutex_id);
}
#endif

#define NIM_MAX_UNLOCK_COUNT              8 
#if 1 /*Fix manualscan will fail to lock again for dib8000 demod*/
	#define NIM_SYR_UNLOCK_MAX_COUNT          1
	#define NIM_TPS_PRF_LK_UNLOCK_MAX_COUNT   1 
#else
	#define NIM_SYR_UNLOCK_MAX_COUNT          8 
	#define NIM_TPS_PRF_LK_UNLOCK_MAX_COUNT   8 
#endif

//static OSAL_ID l_nim_8000_sema_id;
static struct dib8000_Lock_Info   *dib8000_CurChannelInfo;
struct nim_dib8000_private  *gNim_dib8000_private;
UINT8 dib8000_autoscan_stop_flag = 0;

static char nim_dib8000_name[HLD_MAX_NAME_SIZE] = "NIM_DIB8000_0";

static struct dibFrontend frontend[2];
static struct dibDataBusHost *i2c;
struct dibDemodMonitor mon[2];

//static const struct dib7070pa_config nim7070p_config = {
static const struct dib8090_config nim8096md_config[2] = {
    {
        1,
        NULL, // update_lna

        DIB8090_GPIO_DEFAULT_DIRECTIONS,
        DIB8090_GPIO_DEFAULT_VALUES,
        DIB8090_GPIO_DEFAULT_PWM_POS,

        -63,     // dib0070_freq_offset_khz_uhf
        -143,     // dib0070_freq_offset_khz_vhf

        0,
        12000, // clock_khz
        4,
        0,
        0,
        0,
        0,

        0x2d98, // dib8k_drives
        48,   //diversity_delay
        0x31,  //div_cfg
        1, //clkouttobamse
        1, // clkoutdrive
        3, // refclksel
    },
    {
        1,
        NULL, // update_lna

        DIB8090_GPIO_DEFAULT_DIRECTIONS,
        DIB8090_GPIO_DEFAULT_VALUES,
        DIB8090_GPIO_DEFAULT_PWM_POS,

        -63,     // dib0070_freq_offset_khz_uhf
        -143,     // dib0070_freq_offset_khz_vhf

        0,
        12000, // clock_khz
        4,
        0,
        0,
        0,
        0,

        0x2d08, // dib8k_drives
        1,   //diversity_delay
        0x31,  //div_cfg
        1, //clkouttobamse
        1, // clkoutdrive
        3, // refclksel
    },
};

#define BEST_BER 50000
#define ZERO_BER 2097151//10000000
#define DIB_I2C_ID 0

BOOL g_bdevicefail = FALSE;
DiBcom_GetStrength(UINT16 *pu16Strength)
{
	UINT16 v = data_bus_client_read16(demod_get_data_bus_client(&frontend),568);
	UINT8 mpeg_data_lock = ((v >> 5) & 7);
	v = data_bus_client_read16(demod_get_data_bus_client(&frontend), 390);

	//printf("\n",v);
	UINT16 signal_strength = (100 - ((v * 100) / 65535));

	//printf("DiBcom_GetStrength\n");
	*pu16Strength = signal_strength;
	NIM_PRINTF("DiBcom_GetStrength = %d\n", *pu16Strength);

	return 1;
}

DiBcom_GetQuality(UINT16 *pu16Quality)
{
	double tmp = log10(ZERO_BER)-log10(BEST_BER);
	double CUR_BER;

	UINT32 bit_error_rate;
	UINT16 v = data_bus_client_read16(demod_get_data_bus_client(&frontend),568);
	UINT8 mpeg_data_lock = ((v >> 5) & 7);

	  if (mpeg_data_lock)
        	bit_error_rate = data_bus_client_read32(demod_get_data_bus_client(&frontend),560);
    	else
        	bit_error_rate = 2097151;

	//printf("DiBcom_GetQuality\n");

	if (bit_error_rate == 0)
		*pu16Quality = 100;
	else if (bit_error_rate <= BEST_BER)
		*pu16Quality = 99;
	else if (bit_error_rate >= ZERO_BER)
		*pu16Quality = 0;
	else
	{
		CUR_BER = log10(bit_error_rate);
		*pu16Quality = (int)((log10(ZERO_BER) - CUR_BER)/tmp*100); //100;
	}

	return 1;
}


#define I2C_RETRIES	0x0701	/* number of times a device address should be polled when not acknowledging */
#define I2C_TIMEOUT	0x0702	/* set timeout in jiffies - call with int */
/* NOTE: Slave address is 7 or 10 bits, but 10-bit addresses  * are NOT supported! (due to code brokenness) */
#define I2C_SLAVE	0x0703	/* Use this slave address */
#define I2C_SLAVE_FORCE	0x0706	/* Use this slave address, even if it
				   is already in use by a driver! */
#define I2C_TENBIT	0x0704	/* 0 for 7 bit addrs, != 0 for 10 bit */

#define I2C_FUNCS	0x0705	/* Get the adapter functionality mask */

#define I2C_RDWR	0x0707	/* Combined R/W transfer (one STOP only) */

#define I2C_PEC		0x0708	/* != 0 to use PEC with SMBus */
#define I2C_SMBUS	0x0720	/* SMBus transfer */

struct i2c_msg {
	UINT16 addr;	/* slave address			*/
	UINT16 flags;
#define I2C_M_TEN		0x0010	/* this is a ten bit chip address */
#define I2C_M_RD		0x0001	/* read data, from slave to master */
#define I2C_M_REV_DIR_ADDR	0x2000	/* if I2C_FUNC_PROTOCOL_MANGLING */

	UINT16 len;		/* msg length				*/
	UINT8 *buf;		/* pointer to msg data			*/
};

 struct i2c_rdwr_ioctl_data
 {
struct i2c_msg *msgs; /* i2c_msg[]- */
 int nmsgs; /* i2c_msg-秖 */
 };

 
struct i2c_rdwr_ioctl_data work_queue;
struct i2c_rdwr_ioctl_data wr_work_queue;

static int i2c_hdl=0; 
 #define MAX_I2C_MSG     	2 

 extern uint16_t dib8000_identify(struct dibDataBusClient *client);

#if 0
void IIC_check(void)
{
	//i2c
	struct i2c_rdwr_ioctl_data work_queue;
	unsigned int idx;
	unsigned short start_address;
	 int ret;
	 unsigned int fd;
	 
 	fd = open("ali_m36_i2c_0" , O_RDWR);

	if (!fd)
	{
	 	printf("Error on opening the device file\n");
 		return 0;
 	}

 	//sscanf(argv[2], "%x", &start_address);

	work_queue.nmsgs = MAX_I2C_MSG; /* -秖 */

 	work_queue.msgs = (struct i2c_msg*)malloc(work_queue.nmsgs *sizeof(struct i2c_msg));

	if (!work_queue.msgs)
	{
		printf("Memory alloc error\n");
		 close(fd);
 		return 0;
	}

	idx = 0; 
	work_queue.msgs[idx].flags=0;

 	for (idx = 0; idx < work_queue.nmsgs; ++idx)
 	{
		 work_queue.msgs[idx].len = 0;
 		 work_queue.msgs[idx].addr = start_address + idx;
 		 work_queue.msgs[idx].buf = NULL;
	 }

 	//ioctl(fd, I2C_TIMEOUT, 2); /* -竚禬- */
 	//ioctl(fd, I2C_RETRIES, 1); /* -竚-Ω- */

 	ret = ioctl(fd, I2C_RDWR, (unsigned long) &work_queue);

 	if (ret < 0)
 	{
 		printf("Error during I2C_RDWR ioctl with error code: %d\n", ret);
 	}

 	close(fd);
 	return ;
}
#endif
 extern unsigned int  I2c_hfd;

INT8 f_dibcom_read(UINT8 dev_addr, UINT8 *tx, UINT16 tx_len, UINT8 *rx, UINT16 rx_len)
{
	INT8 err;	
	//UINT8 buf[tx_len+rx_len];
	int i;
	//i2c
	//struct i2c_rdwr_ioctl_data work_queue;
	unsigned int idx;	
	//unsigned short start_address;
	 int ret;
	 
	UINT8 pdata[tx_len+rx_len];

	dib8000_i2c_mutex_enter();
	 ALI_PRINTF("RD enter\n");	

	//memset(&work_queue, 0, sizeof(struct i2c_rdwr_ioctl_data));
	work_queue.nmsgs = MAX_I2C_MSG; /* -秖 */
 	//work_queue.msgs = (struct i2c_msg*)malloc(work_queue.nmsgs *sizeof(struct i2c_msg));
 	memset(work_queue.msgs, 0, work_queue.nmsgs *sizeof(struct i2c_msg));
	
	if((0==i2c_hdl)||(0==work_queue.msgs))		
	{	
		dib8000_i2c_mutex_exit();
		return -1;
	}
		
	for (i = 0; i < tx_len+rx_len; i++)	
	{
		pdata[i] =0;			 
	}

	pdata[0]=tx[0];
	pdata[1]=tx[1];

	ALI_PRINTF("dev_addr =%x\n",dev_addr);	
	for(i=0;i<rx_len;i++)
	{
		pdata[i+2]=rx[i];			
	}


	for(i=0;i<rx_len+2;i++)
	 	ALI_PRINTF("data[%d]=%x\n",i,pdata[i]);	

	work_queue.msgs[0].flags=I2C_M_RD|I2C_M_REV_DIR_ADDR;
	work_queue.msgs[0].addr=dev_addr;	
	work_queue.msgs[0].buf=pdata;	
	work_queue.msgs[0].len=rx_len;

	//printf("i2c_read done\n");
	 ioctl(i2c_hdl, I2C_RDWR, (unsigned long) &work_queue);

	for (i=0; i< rx_len;i++)
	{
		rx[i]=work_queue.msgs[0].buf[i];
		ALI_PRINTF("rx[%d]=0x%x  ",i,rx[i]);
	}	
	 ALI_PRINTF("\n");	

	// osal_task_sleep(50);
	//free(work_queue.msgs);
	 ALI_PRINTF("RD end\n");	
	dib8000_i2c_mutex_exit();
	return 0;

}

INT8 f_dibcom_write(UINT8 dev_addr, UINT8 *tx, UINT16 tx_len)
{
	INT32 err;
	int i;
	UINT8  pdata[tx_len];
//i2c
//	struct i2c_rdwr_ioctl_data work_queue;
	unsigned int idx;	
	//unsigned short start_address;
	 int ret; 

	dib8000_i2c_mutex_enter();	
	 ALI_PRINTF("WR enter\n");	
	if((0==i2c_hdl)||(0==wr_work_queue.msgs))	
	{	
		dib8000_i2c_mutex_exit();
		return -1;
	}

	for (i = 0; i < tx_len; i++)	
	{
		pdata[i] =0;			 
	}	

	ALI_PRINTF("dev_addr =%x\n",dev_addr);	
	for (i = 0; i < tx_len; i++)	
	{
		pdata[i] = tx[i];	
		 ALI_PRINTF("data[%d]=%x\n",i,pdata[i]);	
	}	

	//memset(&work_queue, 0, sizeof(struct i2c_rdwr_ioctl_data));
	wr_work_queue.nmsgs = MAX_I2C_MSG; /* -秖 */
 	//work_queue.msgs = (struct i2c_msg*)malloc(work_queue.nmsgs *sizeof(struct i2c_msg));	
 	memset(wr_work_queue.msgs, 0, wr_work_queue.nmsgs *sizeof(struct i2c_msg));

	wr_work_queue.msgs[0].flags=0;	
	wr_work_queue.msgs[0].addr=dev_addr;	
	wr_work_queue.msgs[0].buf=pdata;	
	wr_work_queue.msgs[0].len=tx_len;	

 	ret=ioctl(i2c_hdl, I2C_RDWR, (unsigned long) &wr_work_queue);

 	if (ret < 0)
 	{
 		printf("Error during I2C_RDWR ioctl with error code: %d\n", ret);
 	}
	
	//free(work_queue.msgs);
	//close(fd);
		 ALI_PRINTF("WR end \n");	
	// osal_task_sleep(50);
dib8000_i2c_mutex_exit();
	return 0;	

}

//detect a connected status of a wire
static INT32 f_nim8000_CC_Tracking()
{
	struct dibChannel ch;

    
	//osal_semaphore_capture(l_nim_8000_sema_id, TMO_FEVR);
	 dib8000_nim_mutex_enter();
        
	ALI_PRINTF("f_nim8000_CC_Tracking...\n");
			
	INIT_CHANNEL(&ch, STANDARD_ISDBT);
	ch.RF_kHz           = dib8000_CurChannelInfo->CC_Tracking_Frequency;
	ch.bandwidth_kHz    = dib8000_CurChannelInfo->CC_Tracking_ChannelBW * 1000;

	ALI_PRINTF("Track_freq=%d\n",ch.RF_kHz );
	tune_diversity(&frontend, 1, &ch);

    if(frontend_get_status(&frontend) == FE_STATUS_LOCKED)
    {
		ALI_PRINTF("f_nim8000_CC_Tracking SUCCESS !!!\n");
    	dib8000_CurChannelInfo->Lock_Val = 1;
    }
    else
    {
        dib8000_CurChannelInfo->Lock_Val = 0;
		ALI_PRINTF("f_nim8000_CC_Tracking FAIL !!!\n");
    }

	//osal_semaphore_release(l_nim_8000_sema_id);
	dib8000_nim_mutex_exit();
        
    return SUCCESS;
}

void dib8000_Tracking()
{
	UINT16 v = data_bus_client_read16(demod_get_data_bus_client(frontend),568);
	UINT8 mpeg_data_lock;// = (v >>  6) & 1;
	static UINT8 lock_count = 0;
	static UINT8 TK_Count = 0;
	UINT8 tmccg_sync_lock=0;

	if(( (v >>  5) & 1)||( (v >>  6) & 1)||( (v >>  7) & 1))	
		mpeg_data_lock=1;
	else
		mpeg_data_lock=0;
	
	if( (v >> 3) & 1)
		tmccg_sync_lock=1;
	else
		tmccg_sync_lock=0;
	

	if ((tmccg_sync_lock)&&(mpeg_data_lock))	
	{
		lock_count = 0;
		if (dib8000_CurChannelInfo->Lock_Val == 0)
		{
			dib8000_CurChannelInfo->Lock_Val = 1;
		}
	}
	else
	{
		if (dib8000_CurChannelInfo->Lock_Val == 1)
		{
			if (++lock_count == NIM_MAX_UNLOCK_COUNT)
			{
				dib8000_CurChannelInfo->Lock_Val = 0;
				lock_count = 0;
			}
		}
	}

	if (dib8000_CurChannelInfo->Lock_Val == 1)
		TK_Count = 0;
	else 
    {
		if (++TK_Count == NIM_MAX_UNLOCK_COUNT)
        {
			if (!dib8000_CurChannelInfo->CC_Tracking_flag)
			{
				f_nim8000_CC_Tracking(); //detect a connected status of a wire
			}
 			TK_Count = 0;
        }
    }
}

#if 0
extern CONTAINER win_factoryset_con;
extern CONTAINER g_win_misc_setting;
extern CONTAINER g_win_antenna_set_fixed;
#endif
static INT32 f_dib8000_get_lock_status(struct nim_device *dev, UINT8 *lock)
{
	UINT8 mpeg_data_lock=0;
	UINT8 tmccg_sync_lock=0;
	static UINT32 dd_time=0; 
	UINT32 post_ber=0;

	UINT16 v = data_bus_client_read16(demod_get_data_bus_client(frontend),/*509*/568);
//	printf("MP_LK=%x\n",v);

	if(( (v >>  5) & 1)||( (v >>  6) & 1)||( (v >>  7) & 1))	
		mpeg_data_lock=1;
	else
		mpeg_data_lock=0;

	if( (v >>  3) & 1)	
		tmccg_sync_lock=1;
	else
		tmccg_sync_lock=0;


	if ((tmccg_sync_lock)&&(mpeg_data_lock))
	{	
		*lock = 1;
		dib8000_CurChannelInfo->Lock_Val = 1;
	}
	else
	{	
		*lock = 0;
		dib8000_CurChannelInfo->Lock_Val = 0;
		/*if (menu_stack_get_top() == (POBJECT_HEAD)&g_win_misc_setting || menu_stack_get_top() == (POBJECT_HEAD)&g_win_antenna_set_fixed || menu_stack_get_top() == NULL)
		{
			dib8000_Tracking();
		}*/
	}
	
	if ( osal_get_tick() - dd_time >100)
	{
			     f_dib8000_get_BER(dev, &post_ber);		
			     dd_time=osal_get_tick() ;
	}
	
	return SUCCESS;
}

static UINT32 dib8000_ber = 0;
static UINT32 dib8000_per = 0;
static UINT32 dib8000_per_tot_cnt = 0;


static INT32 f_dib8000_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{
	static UINT32 rec_ber_cnt = 0;
	nim_rec_performance_t * p_nim_rec_performance;
	INT32 ret_val = ERR_FAILUE;
	

	switch (cmd)
	{
	case NIM_DRIVER_STOP_ATUOSCAN:
		dib8000_autoscan_stop_flag = param;
		break;
	case NIM_DRIVER_GET_REC_PERFORMANCE_INFO:
			p_nim_rec_performance = (nim_rec_performance_t *)param;
			f_dib8000_get_lock_status(dev, &(p_nim_rec_performance->lock));

			if (p_nim_rec_performance->lock == 1)
			{			
				if (rec_ber_cnt !=dib8000_per_tot_cnt)
				{
					rec_ber_cnt = dib8000_per_tot_cnt;
					p_nim_rec_performance->ber = dib8000_ber;
					p_nim_rec_performance->per = dib8000_per;
					p_nim_rec_performance->valid = TRUE;
				}
				else
				{
					p_nim_rec_performance->valid = FALSE;					
				}
			}

			ret_val = SUCCESS;
			
			break;	
	default:
		break;
	}

	NIM_PRINTF("Exit f_dib8000_ioctl() normally.\n");
	return SUCCESS;
}

static INT32 nim_internal_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec)
{

	INT32   chsearch;
	
	chsearch = f_dib8000_channel_change(dev, freq, 6, 0, 0, 0, 0, 0, 0, 0);

	return chsearch;
}

INT32 nim_dib8000_ioctl_ext(struct nim_device *dev, INT32 cmd, void *param_list)
{
	//struct nim_s3501_private *priv = (struct nim_s3501_private *) dev->priv;
	NIM_PRINTF("    Enter fuction nim_s3501_event_control\n");

	switch (cmd)
	{	
	
	case NIM_DRIVER_CHANNEL_CHANGE:
		/* Do Channel Change */
		NIM_PRINTF(">>>DIB8000 NIM_DRIVER_CHANNEL_CHANGE\n");
		{
			struct NIM_Channel_Change *nim_param = (struct NIM_Channel_Change *) (param_list);
			return nim_internal_channel_change(dev, nim_param->freq, nim_param->sym, nim_param->fec);
		}
	default:
		break;
	}

	return SUCCESS;
}




#if 1 /*Fix manualscan will fail to lock again for dib8000 demod*/
static BOOL CheckChannelChange(UINT32 freq, UINT8 bandwidth)
{
	if (freq == dib8000_CurChannelInfo->CC_Tracking_Frequency && bandwidth == dib8000_CurChannelInfo->CC_Tracking_ChannelBW)
		return FALSE;
	return TRUE;
}
#endif

static INT32 f_dib8000_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth, 
									   UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT8 priority)
{
	INT32 ret_code;
	UINT32  fft_Mode;
	UINT32  Guard;
	struct dibChannel ch;
	struct nim_dib8000_private *dev_priv;  
	dev_priv = gNim_dib8000_private;
	UINT8 lock=0;


	//osal_semaphore_capture(l_nim_8000_sema_id, TMO_FEVR);
	 dib8000_nim_mutex_enter();	
	
	ALI_PRINTF("dib8000_CG_start! freq=%d\n",freq);

	dib8000_CurChannelInfo->CC_Tracking_staus=0;

	/*while(1)
	{
		 dib8000_identify(demod_get_data_bus_client(&frontend[0]));

		osal_task_sleep(2000);
	}*/

#if 1 /*Fix manualscan will fail to lock again for dib8000 demod*/
	if (dib8000_CurChannelInfo->Lock_Val == 1 && !CheckChannelChange(freq, bandwidth))
	{
		//osal_semaphore_release(l_nim_8000_sema_id);		
		 dib8000_nim_mutex_exit();
		return SUCCESS;
	}
#endif

	dib8000_CurChannelInfo->CC_Tracking_flag = 1; //TRACKING CHECK IF HAVE DONE CC, TRACKING RESET.
	dib8000_CurChannelInfo->Lock_Val = 0;

	INIT_CHANNEL(&ch, STANDARD_ISDBT);
	ch.RF_kHz           = dib8000_CurChannelInfo->Frequency = freq;
	ch.bandwidth_kHz    = bandwidth * 1000;
	dib8000_CurChannelInfo->ChannelBW = bandwidth;

	if (dib8000_autoscan_stop_flag)
	{
			dib8000_autoscan_stop_flag = 0;
			ALI_PRINTF("DIB8000 EXIT CG by USER !!!\n");
			
 			dib8000_nim_mutex_exit();
			//osal_semaphore_release(l_nim_8000_sema_id);
			return ERR_FAILUE;
	}

	tune_diversity(&frontend, 1, &ch);

	f_dib8000_get_lock_status(dev,&lock);
	
	//if (frontend_get_status(&frontend) == FE_STATUS_LOCKED)
	if(lock)
	{
		dib8000_CurChannelInfo->Lock_Val = 1;
		ALI_PRINTF("f_dib8000_channel_change SUCCESS !!!\n");		
		ret_code = SUCCESS;
	}
	else
	{
		dib8000_CurChannelInfo->Lock_Val = 0;
		ALI_PRINTF("f_dib8000_channel_change FAIL !!!\n");		
		ret_code = ERR_FAILED;
	}

	NIM_PRINTF(" freq=%d, bandwidth=%d, guard_interval=%d, fft_mode=%d, modulation=%d, fec=%d, usage_type=%d, inverse=%d\n\n",  freq, bandwidth, guard_interval, fft_mode, modulation, fec, usage_type, inverse);

	if (guard_interval == guard_1_32)
		Guard = DIBTUNER_GUARD_1_32;
	else if (guard_interval == guard_1_16)
		Guard = DIBTUNER_GUARD_1_16;
	else if (guard_interval == guard_1_8)
		Guard = DIBTUNER_GUARD_1_8;
	else
		Guard = DIBTUNER_GUARD_1_4;

	if (fft_mode == FFT_8K)
		fft_Mode = DIBTUNER_MODE_8K;
	else if (fft_mode == FFT_4K)
		fft_Mode = DIBTUNER_MODE_4K;
	else
		fft_Mode = DIBTUNER_MODE_2K;

	dib8000_CurChannelInfo->CC_Tracking_Frequency = freq;  
	dib8000_CurChannelInfo->CC_Tracking_ChannelBW = bandwidth;
	dib8000_CurChannelInfo->CC_Tracking_Modulation = modulation;
	dib8000_CurChannelInfo->CC_Tracking_Mode = fft_Mode;
	dib8000_CurChannelInfo->CC_Tracking_Guard = Guard;
	dib8000_CurChannelInfo->CC_Tracking_Spectrum = inverse;
	dib8000_CurChannelInfo->CC_Tracking_flag = 0;

	ALI_PRINTF("dib8000_CG_EXIT!");

	 dib8000_nim_mutex_exit();
	//osal_semaphore_release(l_nim_8000_sema_id);

	return ret_code;
}

static INT32 f_dib8000_channel_search(struct nim_device *dev, UINT32 freq, UINT32 bandwidth, 
									   UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT16 freq_offset, UINT8 priority)
{
	UINT8   i, j;
	INT32   chsearch;
	UINT32  Center_NPRO, Search_NPRO, tmp_freq;
	UINT16 tmp_data;

	NIM_PRINTF("Enter f_dib8000_channel_search()..., freq = %d, bandwidth = %d\n", freq, bandwidth);

	Center_NPRO = (((freq + 36250)*6)/100); 

	for (i=0; i<1; i++)
	{
		chsearch = ERR_FAILED;
		if (dib8000_autoscan_stop_flag)
		{
			dib8000_autoscan_stop_flag = 0;
			return ERR_FAILUE;
		}

		if (i % 2)
			j = 1;
		else
			j = (-1);

		Search_NPRO = (((i+1)/2)*30*j)+Center_NPRO;
		tmp_freq = (Search_NPRO*100)/6;
		
		if (tmp_freq < 36250)
		{
			NIM_PRINTF("f_dib8000_channel_search(): channel search break!!=%d ==> tmp_freq(%d)\n", i, tmp_freq);
			break;      
		}
		else
			freq = tmp_freq - 36250;
	
		chsearch = f_dib8000_channel_change(dev, freq, bandwidth, guard_interval, fft_mode, modulation, fec, usage_type, inverse, priority);
		if (chsearch == SUCCESS)
		{
			break;
		}
	}
	return chsearch;
}

static INT32 f_dib8000_get_freq(struct nim_device *dev, UINT32 *freq)
{
	*freq = dib8000_CurChannelInfo->Frequency;

	NIM_PRINTF("f_dib8000_get_freq(): freq = %d KHz\n", *freq);
	return SUCCESS;
}

static INT32 f_dib8000_get_AGC(struct nim_device *dev, UINT16 *agc)
{
	static UINT16  Data1=0;
	struct dibChannel ch;


	if (dib8000_CurChannelInfo->Lock_Val == 0)
	{
		*agc = 0; 
		return !SUCCESS;
	}
	else
	{
		DiBcom_GetStrength(&Data1);
		dib8000_CurChannelInfo->AGC_Val= Data1;
		*agc = (dib8000_CurChannelInfo->AGC_Val);
		if(*agc>100 )
		{	
			*agc =100;
			dib8000_CurChannelInfo->AGC_Val=100;
		}
		
		//printf("AG=%d\n",*agc);
	}

	return SUCCESS;
}

static INT32 f_dib8000_get_code_rate(struct nim_device *dev, UINT8* code_rate)
{
	NIM_PRINTF("Enter f_dib8000_get_code_rate()...\n");

	struct dibDemodMonitor dibMonitor;
	struct dibChannel cur_ch;

	demod_get_monitoring(&frontend,&dibMonitor);

	cur_ch = dibMonitor.cur_digital_channel;

	dib8000_CurChannelInfo->HPRates = cur_ch.u.dvbt.code_rate_hp;
	NIM_PRINTF("f_dib8000_get_code_rate(): code_rate = %d\n", dib8000_CurChannelInfo->HPRates);

	switch (dib8000_CurChannelInfo->HPRates)
	{
	case DIBTUNER_FEC_1_2:
		*code_rate = FEC_1_2;
		break;
	case DIBTUNER_FEC_2_3:
		*code_rate = FEC_2_3;
		break;
	case DIBTUNER_FEC_3_4:
		*code_rate = FEC_3_4;
		break;
	case DIBTUNER_FEC_5_6:
		*code_rate = FEC_5_6;
		break;
	case DIBTUNER_FEC_7_8:
		*code_rate = FEC_7_8;
		break;
	default:
		ALI_PRINTF("f_dib8000_get_code_rate() ==> Error\n");
		*code_rate = 0;	/* error */
		break;
	}

	NIM_PRINTF("Exit f_dib8000_get_code_rate() normally.\n");
	return SUCCESS;
}

static INT32 f_dib8000_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
{
	DIBTUNER_Mode_t dib_fft_mode = 0;

	struct dibDemodMonitor dibMonitor;
	struct dibChannel cur_ch;

	demod_get_monitoring(&frontend,&dibMonitor);

	cur_ch = dibMonitor.cur_digital_channel;

	NIM_PRINTF("Enter f_dib8000_get_fftmode()...\n");

	if (cur_ch.u.dvbt.nfft == FFT_2K)
	{
		dib_fft_mode = DIBTUNER_MODE_2K;
	}
	else if (cur_ch.u.dvbt.nfft == FFT_8K)
	{
		dib_fft_mode = DIBTUNER_MODE_8K;
	}
	else if (cur_ch.u.dvbt.nfft == FFT_4K)
	{
		dib_fft_mode = DIBTUNER_MODE_4K;
	}
	dib8000_CurChannelInfo->Mode = dib_fft_mode;
	NIM_PRINTF("f_dib8000_get_fftmode(): fft_mode = %d\n", dib8000_CurChannelInfo->Mode);

	switch (dib8000_CurChannelInfo->Mode)
	{
	case DIBTUNER_MODE_2K:
		*fft_mode = MODE_2K;
		break;
	case DIBTUNER_MODE_8K:
		*fft_mode = MODE_8K;
		break;
	default:
		ALI_PRINTF("f_dib8000_get_fftmode() ==> Error\n");
		*fft_mode = 0xff; /* error */
		break;
	}

	NIM_PRINTF("Exit f_dib8000_get_fftmode() normally.\n");
	return SUCCESS;
}

static INT32 f_dib8000_get_modulation(struct nim_device *dev, UINT8 *modulation)
{
	DIBTUNER_Modulation_t dib_modulation;

	struct dibDemodMonitor dibMonitor;
	struct dibChannel cur_ch;


	demod_get_monitoring(&frontend,&dibMonitor);

	cur_ch = dibMonitor.cur_digital_channel;

	NIM_PRINTF("Enter f_dib8000_get_modulation()...\n");

	dib8000_CurChannelInfo->Modulation = cur_ch.u.dvbt.constellation;
	NIM_PRINTF("f_dib8000_get_modulation(): modulation = %d\n", dib8000_CurChannelInfo->Modulation);

	switch (dib8000_CurChannelInfo->Modulation)
	{
	case QAM_QPSK:
		*modulation = TPS_CONST_QPSK;
		break;
	case QAM_16QAM:
		*modulation = TPS_CONST_16QAM;
		break;
	case QAM_64QAM:
		*modulation = TPS_CONST_64QAM;
		break;
	default:
		ALI_PRINTF("f_dib8000_get_modulation() ==> UNKNOWN\n");
		*modulation = 0xff;	/* error */
		break;
	}

	NIM_PRINTF("Exit f_dib8000_get_modulation() normally.\n");
	return SUCCESS; 
}

static INT32 f_dib8000_get_GI(struct nim_device *dev, UINT8 *guard_interval)
{
	DIBTUNER_Guard_t  dib_guard_interval;

	struct dibDemodMonitor dibMonitor;
	struct dibChannel cur_ch;

	demod_get_monitoring(&frontend,&dibMonitor);

	cur_ch = dibMonitor.cur_digital_channel;

	UINT8 tmp_data = 32 / (1 << cur_ch.u.dvbt.guard);

	NIM_PRINTF("Enter f_dib8000_get_GI()...\n");

	if (tmp_data == 4)
	{
		dib8000_CurChannelInfo->Guard = DIBTUNER_GUARD_1_4;
	}
	else if (tmp_data == 6)
	{
		dib8000_CurChannelInfo->Guard = DIBTUNER_GUARD_1_8;
	}
	else if (tmp_data == 16)
	{
		dib8000_CurChannelInfo->Guard = DIBTUNER_GUARD_1_16;
	}
	else if (tmp_data == 32)
	{
		dib8000_CurChannelInfo->Guard = DIBTUNER_GUARD_1_32;
	}

	NIM_PRINTF("f_dib8000_get_GI(): guard_interval = %d\n", dib8000_CurChannelInfo->Guard);

	switch (dib8000_CurChannelInfo->Guard)
	{
	case DIBTUNER_GUARD_1_32:
		*guard_interval = guard_1_32;
		break;
	case DIBTUNER_GUARD_1_16:
		*guard_interval = guard_1_16;
		break;
	case DIBTUNER_GUARD_1_8:
		*guard_interval = guard_1_8;
		break;
	case DIBTUNER_GUARD_1_4:
		*guard_interval = guard_1_4;
		break;
	default:
		ALI_PRINTF("f_dib8000_get_GI() ==> Guard Error\n");
		*guard_interval = 0xff;	/* error */
		break;
	}

	NIM_PRINTF("Exit f_dib8000_get_GI() normally.\n");
	return SUCCESS;
}

static INT32 f_dib8000_get_specinv(struct nim_device *dev, UINT8 *Inv)
{   
	struct dibDemodMonitor dibMonitor;
	struct dibChannel cur_ch;

	demod_get_monitoring(&frontend,&dibMonitor);

	cur_ch = dibMonitor.cur_digital_channel;

	*Inv = cur_ch.u.dvbt.intlv_native;

	return SUCCESS;
}

static INT32 f_dib8000_close(struct nim_device *dev)
{
	frontend_unregister_components(&frontend);
	host_i2c_release(i2c);

	dib8000_CurChannelInfo->Dis_TS_Output = 1;
	NIM_PRINTF("Exit f_dib8000_close() normally.\n");
	return SUCCESS;
}

static INT32 f_dib8000_disable(struct nim_device *dev)
{
	frontend_unregister_components(&frontend);
	host_i2c_release(i2c);

	dib8000_CurChannelInfo->Dis_TS_Output = 1;
	ALI_PRINTF("Enter f_dib8000_disable()...\n");
	NIM_PRINTF("Exit f_dib8000_disable() normally.\n");
	return SUCCESS;
}


static INT32 f_dib8000_get_BER(struct nim_device *dev, UINT32 *vbber)
{    	
	UINT32 ber_A=0,ber_B=0,ber_C=0;
	UINT16 PacketErrors_A=0,PacketErrors_B=0,PacketErrors_C=0;
	

	if (dib8000_CurChannelInfo->Lock_Val != 1)
	{
		*vbber = 0;
		dib8000_ber=20000;
		dib8000_per=100;
		return SUCCESS; 
	}
	else
	{

	 	ber_A = data_bus_client_read32(demod_get_data_bus_client(&frontend),560);
		ber_B = data_bus_client_read32(demod_get_data_bus_client(&frontend),576);
	 	ber_C = data_bus_client_read32(demod_get_data_bus_client(&frontend),581);
		
		*vbber = ber_B;

		PacketErrors_A = data_bus_client_read16(demod_get_data_bus_client(frontend),562);		
       	        PacketErrors_B   =data_bus_client_read16(demod_get_data_bus_client(&frontend), 578);
       	        PacketErrors_C   = data_bus_client_read16(demod_get_data_bus_client(&frontend), 583);	
	}      
	
	dib8000_CurChannelInfo->BER_Val = *vbber;
	dib8000_ber=*vbber;
	dib8000_per=(UINT32)PacketErrors_B;
	dib8000_per_tot_cnt += 1;
	
	NIM_PRINTF("B=%d   P= %d \n",dib8000_ber,dib8000_per);
	return SUCCESS;
}

static INT32 f_dib8000_get_SNR(struct nim_device *dev, UINT8 *snr)
{

#ifdef NIM_DEBUG
	*snr=100;
	return SUCCESS;
#endif

	UINT32 freq;
	UINT8 bw, module, fft, guard, inverse, priority, usage_type, fec;
	
	//NIM_PRINTF("Enter f_dib8000_get_SNR()...\n");

	static UINT16 tSNR = 0;
	if (dib8000_CurChannelInfo->Lock_Val == 0)
	{
		*snr = 0;
		return SUCCESS;
	}
	else
	{
		//DiBcom_GetQuality(&tSNR);
		//*snr = (UINT8)tSNR;
		if(dib8000_CurChannelInfo->AGC_Val==100)
		*snr =100;
		else
		*snr =dib8000_CurChannelInfo->AGC_Val-2;
	}

	if(*snr >100)
		*snr =100;

	//NIM_PRINTF("f_dib8000_get_SNR()\n");

	//NIM_PRINTF("Exit f_dib8000_get_SNR() normally.\n");
	return SUCCESS;
}

INT32 f_dib8000_attach(struct COFDM_TUNER_CONFIG_API *ptrCOFDM_Tuner)
{
	struct nim_device *dev;
	void  *priv_mem;

	printf("Enter f_dib8000_attach()...\n");

	dev = (struct nim_device *)dev_alloc(nim_dib8000_name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
	if (dev == NULL)
	{
		printf("f_dib8000_attach(): Error, Alloc nim device error!\n");
		return ERR_NO_MEM;
	}

	/* Alloc structure space of private */
	priv_mem = (void *)malloc(sizeof(struct nim_dib8000_private));
	if (priv_mem == NULL)
	{
		dev_free(dev);
		printf("f_dib8000_attach(): Alloc nim device prive memory error!\n");
		return ERR_NO_MEM;
	}

	memcpy(priv_mem, ptrCOFDM_Tuner, sizeof(struct nim_dib8000_private));
	dev->priv = priv_mem;

	i2c_hdl=open("/dev/i2c-0", O_RDWR);
	if(i2c_hdl == 0)
	{
		printf("open i2c handle fail, %d\n",i2c_hdl);
	}
	else
	{
		printf("open i2c handle success, %d\n",i2c_hdl);
	}


	memset(&work_queue, 0, sizeof(struct i2c_rdwr_ioctl_data));
	work_queue.nmsgs = MAX_I2C_MSG; /* 消息数量 */
	work_queue.msgs = (struct i2c_msg*)malloc(work_queue.nmsgs *sizeof(struct i2c_msg));
	memset(work_queue.msgs, 0, work_queue.nmsgs *sizeof(struct i2c_msg));
	if (!work_queue.msgs)
	{
		printf("Memory alloc error\n");
		close(i2c_hdl);
		return 0;
	}

	memset(&wr_work_queue, 0, sizeof(struct i2c_rdwr_ioctl_data));
	wr_work_queue.nmsgs = MAX_I2C_MSG; /* 消息数量 */
	wr_work_queue.msgs = (struct i2c_msg*)malloc(wr_work_queue.nmsgs *sizeof(struct i2c_msg));
	memset(wr_work_queue.msgs, 0, wr_work_queue.nmsgs *sizeof(struct i2c_msg));
	/*if (!work_queue.msgs)
	{
		printf("Memory alloc error\n");
		close(i2c_hdl);
		return 0;
	}*/

	//f_IIC_Sema_ID=osal_semaphore_create(1);

	/* Function point init */
	dev->base_addr = SYS_COFDM_DIB8000_CHIP_ADRRESS;
	dev->init = f_dib8000_attach;
	dev->open = f_dib8000_open;
	dev->stop = f_dib8000_close;
//	dev->disable = f_dib8000_disable;
	dev->do_ioctl = f_dib8000_ioctl;
	dev->channel_change = f_dib8000_channel_change;
	dev->channel_search = f_dib8000_channel_search;
	dev->do_ioctl_ext = nim_dib8000_ioctl_ext;
	//printf("attach nim_dib8000_ioctl_ext\n");
	dev->get_lock = f_dib8000_get_lock_status;
	dev->get_freq = f_dib8000_get_freq;
	dev->get_FEC = f_dib8000_get_code_rate;
	dev->get_AGC = f_dib8000_get_AGC;
	dev->get_SNR = f_dib8000_get_SNR;
	dev->get_BER = f_dib8000_get_BER;

	//added for DVB-T additional elements
	dev->get_guard_interval = f_dib8000_get_GI;
	dev->get_fftmode = f_dib8000_get_fftmode;
	dev->get_modulation = f_dib8000_get_modulation;
//	dev->get_spectrum_inv = f_dib8000_get_specinv;

	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS)
	{
		printf("f_dib8000_attach(): Error, Register nim device error!\n");
		free(priv_mem);
		dev_free(dev);
		return ERR_NO_DEV;
	}

	printf("Exit f_dib8000_attach() normally.\n");
	return SUCCESS;
}

int (*nim8096md_tuner_tune) (struct dibFrontend *fe, struct dibChannel *channel);
static int dib8090_tuner_tune(struct dibFrontend *fe , struct dibChannel *channel)
{
    if (fe->tune_state == CT_TUNER_START) {
        switch(BAND_OF_FREQUENCY(channel->RF_kHz)) {
        default:
            dbgpl(NULL,"Warning : this frequency is not in the supported range, using VHF switch");
        case BAND_VHF: //gpio5 : 0; gpio7 : 0
            demod_set_gpio(fe, 3, 0, 1);
            break;
        case BAND_UHF: //gpio5 : 0; gpio7 : 1
            demod_set_gpio(fe, 3, 0, 0);
            break;
        }
    }

    return nim8096md_tuner_tune(fe, channel);
}



 static int (*tfe8096gp_agc_tune_save)(struct dibFrontend *fe, struct dibChannel *channel);

static int tfe8096gp_agc_tune (struct dibFrontend *fe , struct dibChannel *channel)
{
    int return_value;

    return_value = tfe8096gp_agc_tune_save(fe, channel);
    if (fe->tune_state == CT_AGC_STEP_0)
        demod_set_gpio(fe, 6, 0, 1);
    else if (fe->tune_state == CT_AGC_STEP_1) {
        /* check the high level */
        uint16_t ltgain, rf_gain_limit;
        dib0090_get_current_gain(fe, NULL, NULL, &rf_gain_limit, &ltgain);
        if ((channel_frequency_band(channel->RF_kHz) == BAND_CBAND) &&
                (rf_gain_limit < 2000)) /* activate the external attenuator in case of very high input power */
            demod_set_gpio(fe, 6, 0, 0);
    }
    return return_value;
}


int f_dib8000_hw_init(struct nim_device *dev)
{
	struct dibChannel ch;

	i2c = host_i2c_interface_attach(NULL);

	if (i2c == NULL)
		return 1;

	frontend_init(&frontend[0]); /* initializing the frontend-structure */
	frontend_set_id(&frontend[0], 0); /* assign an absolute ID to the frontend */
	frontend_set_description(&frontend[0], "ISDB-T #0 Master");

	if (dib8090_sip_register(&frontend[0], i2c, 0x90/*dev->base_addr*/, &nim8096md_config[0]) == NULL)
	{
		printf("DiB7070P: attaching demod and tuners failed.\n");
		return DIB_RETURN_ERROR;
	}


	
	/*while(0)
	{
		 dib8000_identify(demod_get_data_bus_client(&frontend[0]));

		osal_task_sleep(2000);
	}*/
	
	if (dib8000_i2c_enumeration(i2c, 1, DIB8090_DEFAULT_I2C_ADDRESS, 0x90/*dev->base_addr*/) != 0)
	{
		printf("dib8000_i2c_enumeration error\n");
		return DIB_RETURN_ERROR;
	}
	
          nim8096md_tuner_tune = frontend[0].tuner_info->ops.tune_digital;
          frontend[0].tuner_info->ops.tune_digital = dib8090_tuner_tune;

	  tfe8096gp_agc_tune_save = frontend[0].demod_info->ops.agc_startup;
          frontend[0].demod_info->ops.agc_startup = tfe8096gp_agc_tune;

         //tfe8096gp_tuner_tune_save = frontend[0].tuner_info->ops.tune_digital;
         //frontend[0].tuner_info->ops.tune_digital = tfe8096gp_tuner_tune;
   
        //frontend[0].demod_info->ops.agc_startup = tfe8096gp_agc_tune;
		  
          dib8090_set_wbd_target(&frontend[0], 250, 425);	
	   frontend_reset(&frontend[0]);
	   INIT_CHANNEL(&ch, STANDARD_ISDBT);
	   printf("f_dib8000_hw_init\n");


	
	   
}

void f_dib8000_CC_tracking_task(UINT32 param1)
{           
    UINT32 freq, Pre_TKCount=0, TKCount=0, LKCNT=0;
    UINT32 bandwidth;   
    UINT8 data=0; 
    UINT8 lock=0;
    UINT8 i=0;
    UINT8 agc, snr, BerCNT;
    UINT8  unlock_cnt=0;
	
    while(1)
    {
	     if ((dib8000_CurChannelInfo->Lock_Val == 0)&&(dib8000_CurChannelInfo->CC_Tracking_Frequency!=0))
            {
            	    dib8000_Tracking();
            }
        osal_task_sleep(300);
    }
}


static INT32 f_dib8000_CC_tracking_task_init(struct nim_device *dev)
{
    ER  ret_val;
    OSAL_T_CTSK t_ctsk;    
    t_ctsk.stksz = 0x1000;
    t_ctsk.quantum = 10;
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.task = (FP)f_dib8000_CC_tracking_task;
    t_ctsk.para1 = (UINT32)dev;

    f_dib8000_CC_tracking_task_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID == f_dib8000_CC_tracking_task_id)
    {
        NIM_PRINTF("create_task nim_dib8000_CC_tracking_task_id failed\n");        
        return FALSE;
    }
    return TRUE;
}


static INT32 f_dib8000_open(struct nim_device *dev)
{
	ALI_PRINTF("Enter f_dib8000_open()...\n");

	//IIC_check();

	/*l_nim_8000_sema_id = osal_semaphore_create(1);
	if (l_nim_8000_sema_id == OSAL_INVALID_ID)
	{
		printf("f_dib8000_open(): Create semaphore fail!\n");
		return ERR_FAILUE;
	}*/
	dib8000_CurChannelInfo = (struct dib8000_Lock_Info *)malloc(sizeof(struct dib8000_Lock_Info));
	if (dib8000_CurChannelInfo == NULL)
	{
		printf("f_dib8000_open(): malloc fail!\n");
		return ERR_FAILUE;
	}
	memset(dib8000_CurChannelInfo, 0, sizeof(struct dib8000_Lock_Info));
	NIM_PRINTF("f_dib8000_open(): => f_dib8000_hw_init()...\n");
	if (f_dib8000_hw_init(dev) == DIB_RETURN_ERROR)
	{
		//return ERR_FAILUE;
	}

	/*if(FALSE == f_dib8000_CC_tracking_task_init(dev))
	{
        	//printf("f_dib8000_hw_init(): Creak NIM tracking task fail!\n");
    	}*/

	printf("Exit f_dib8000_open() normally.\n");
	return SUCCESS;
}
