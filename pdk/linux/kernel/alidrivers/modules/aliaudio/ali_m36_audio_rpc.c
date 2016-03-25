#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/poll.h>
//#include <linux/byteorder/swabb.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0))
#include <linux/firmware.h>
#endif
#include <linux/crc32.h>
#include <linux/slab.h> 

#include <ali_cache.h>
#include <linux/debugfs.h>

//#include <asm/system.h>
//#include <asm/semaphore.h>

#include "ali_m36_audio_rpc.h"
#include <ali_shm.h> // add by jacket 2013.7.17
#include <ali_board_config.h> // add by jacket 2013.10.23
#include "ali_audio_info.h"

volatile unsigned long *g_ali_audio_rpc_arg[MAX_AUDIO_RPC_ARG_NUM];
volatile unsigned long *g_ali_audio_rpc_tmp;
volatile int g_ali_audio_rpc_arg_size[MAX_AUDIO_RPC_ARG_NUM];

static struct semaphore m_audio_sem;

static RET_CODE ali_m36_audio_open(struct inode *inode, struct file *file);
static RET_CODE ali_m36_audio_release(struct inode *inode, struct file *file);
static ssize_t ali_m36_audio_write(struct file *file, const char __user *buf,
			       size_t count, loff_t *ppos);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long ali_m36_audio_ioctl(struct file *file,
			   unsigned int cmd, unsigned long parg);
#else
static int ali_m36_audio_ioctl(struct inode *inode, struct file *file,
			   unsigned int cmd, unsigned long parg);
#endif

/******************************************************************************
 * driver registration
 ******************************************************************************/

static struct file_operations ali_m36_audio_fops = {
	.owner		= THIS_MODULE,
	.write		= ali_m36_audio_write,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	.unlocked_ioctl = ali_m36_audio_ioctl,        
#else
	.ioctl		= ali_m36_audio_ioctl,
#endif
	.open		= ali_m36_audio_open,
	.release	=  ali_m36_audio_release,
	//.poll		= dvb_audio_poll,
};

struct ali_audio_device ali_m36_audio_dev;

struct class *ali_m36_audio_class;
struct device *ali_m36_audio_dev_node;

static int m_audio_pause = 0;
static int m_audio_start = 0;
static int deca_stop_in_kernel = 0;

static void hld_deca_rpc_release(int force)
{
	if((m_audio_start == 1) || (force == 1))
	{
		deca_stop(ali_m36_audio_dev.deca_dev, 0, ADEC_STOP_IMM);
		snd_stop(ali_m36_audio_dev.snd_dev);		
		
		#if defined(CONFIG_ARM)
		deca_decore_ioctl(ali_m36_audio_dev.deca_dev,DECA_DECORE_RLS,NULL,NULL);
		#endif
		
		m_audio_start = 0;
		if(force == 0)
			deca_stop_in_kernel = 1;
	}
}

void ali_deca_rpc_release(void)
{
	down(&m_audio_sem);
	
	hld_deca_rpc_release(0);

	up(&m_audio_sem);	
}

static RET_CODE audio_rpc_operation(struct ali_audio_device *dev,UINT32 ID)
{
    RET_CODE ret = 0;

	ali_audio_api_show("%s,%d,ID:0x%x\n", __FUNCTION__, __LINE__, ID);
	
    switch(ID)
    {
/*multi args RPC calling*/
       case AUDIO_SND_GET_STC:
                ret = get_stc((UINT32 *)g_ali_audio_rpc_arg[0],*(UINT8 *) g_ali_audio_rpc_arg[1]);
                break;
       case AUDIO_SND_SET_STC:
                set_stc(*(UINT32 *)g_ali_audio_rpc_arg[0],*(UINT8 *) g_ali_audio_rpc_arg[1]);
                break; 
       case AUDIO_SND_PAUSE_STC:
                stc_pause(*(UINT8 *)g_ali_audio_rpc_arg[0],*(UINT8 *) g_ali_audio_rpc_arg[1]);
                break; 
       case AUDIO_SND_SET_SUB_BLK:
                snd_set_sub_blk(dev->snd_dev,*(UINT8 *)g_ali_audio_rpc_arg[0],*(UINT8 *) g_ali_audio_rpc_arg[1]);
                break;
       case AUDIO_SND_ENABLE_EQ:
 		  snd_enable_eq(dev->snd_dev,*(UINT8*)g_ali_audio_rpc_arg[0],(*(UINT8 *)g_ali_audio_rpc_arg[1]));	                   
               break; 
	case AUDIO_DECA_IO_COMMAND_ADV:
 		   deca_io_control(dev->deca_dev,*(UINT32 *)g_ali_audio_rpc_arg[0], (UINT32)g_ali_audio_rpc_arg[1]);	
                break;	
       case AUDIO_SND_IO_COMMAND_ADV:
 		 snd_io_control(dev->snd_dev,*(UINT32 *)g_ali_audio_rpc_arg[0], (UINT32)g_ali_audio_rpc_arg[1]);
              break;
       case RPC_AUDIO_DECORE_IOCTL:
	   	if(*(int *)(g_ali_audio_rpc_arg[0]) == DECA_DECORE_INIT)
	   	{
			hld_deca_rpc_release(0);
			m_audio_pause = 0;
			m_audio_start = 1;
		}
		else if(*(int *)(g_ali_audio_rpc_arg[0]) == DECA_DECORE_PAUSE_DECODE)
		{
			if(*(int *)g_ali_audio_rpc_arg[1])
				m_audio_pause = 1;
			else
				m_audio_pause = 0;

			//printk("<0>""%s : pause flag %d\n", __FUNCTION__, m_audio_pause);			
		}
		else if(*(int *)(g_ali_audio_rpc_arg[0]) == DECA_DECORE_RLS)
		{
			m_audio_pause = 0;
			m_audio_start = 0;
		}
 	    ali_audio_api_show("%s,%d,arg[0]:0x%x,arg[1]:0x%x,arg[2]:0x%x\n",
 			__FUNCTION__, __LINE__, *(int *)g_ali_audio_rpc_arg[0],
 			(void *)g_ali_audio_rpc_arg[1], (void *) g_ali_audio_rpc_arg[2]);
 		
         #if defined(CONFIG_ARM)
        
        ret = deca_decore_ioctl(ali_m36_audio_dev.deca_dev,*(int *)g_ali_audio_rpc_arg[0], (void *)g_ali_audio_rpc_arg[1],(void *) g_ali_audio_rpc_arg[2]);
         #endif

        break;
       case AUDIO_DECA_PROCESS_PCM_SAMPLES:
             {
                UINT8 *tmp_buf = NULL;
				
				#ifndef CONFIG_ARM
                tmp_buf = kmalloc(*(UINT32 *)g_ali_audio_rpc_arg[0],GFP_KERNEL);
                if(tmp_buf == NULL)
    		    {
    			    PRINTK_INFO("kmalloc request buf fail\n");
    			    return RET_FAILURE;
    		    }
				#else
				if (*(UINT32 *)g_ali_audio_rpc_arg[0] > __G_ALI_MM_DECA_MEM_SIZE)
				{
    			    printk("request buf size is too big, pls change it smaller!\n");
    			    return RET_FAILURE;					
				}
				
				tmp_buf = (UINT8 *)__G_ALI_MM_DECA_MEM_START_ADDR;
				#endif
				
                copy_from_user(tmp_buf,(void *)(*(UINT32 *)g_ali_audio_rpc_arg[1]),*(UINT32 *)g_ali_audio_rpc_arg[0]);
                
				#ifndef CONFIG_ARM
                dma_cache_wback((unsigned long)tmp_buf, *(UINT32 *)g_ali_audio_rpc_arg[0]);
				#else
				tmp_buf = (UINT8 *)((((UINT32)tmp_buf)&0x0fffffff) | 0xa0000000);
                #endif
                
                 deca_process_pcm_samples(*(UINT32 *)g_ali_audio_rpc_arg[0], tmp_buf,*(UINT32 *) g_ali_audio_rpc_arg[2],\
                *(UINT32 *)g_ali_audio_rpc_arg[3],*(UINT32 *)g_ali_audio_rpc_arg[4]);

				 #ifndef CONFIG_ARM
                 kfree(tmp_buf);
				 #endif
              }
              break;
        case AUDIO_DECA_PROCESS_PCM_BITSTREAM:
             {
                UINT8 *pcm_buf,*bs_buf;
				
				#ifndef CONFIG_ARM
                pcm_buf = kmalloc(*(UINT32 *)g_ali_audio_rpc_arg[0],GFP_KERNEL);
                if(pcm_buf == NULL)
    		    {
    			    PRINTK_INFO("kmalloc request buf fail\n");
    			    return RET_FAILURE;
    		    }
				#else
				if (((*(UINT32 *)g_ali_audio_rpc_arg[0]) + (*(UINT32 *)g_ali_audio_rpc_arg[2])) > __G_ALI_MM_DECA_MEM_SIZE)
				{
    			    printk("request buf size is too big, pls change it smaller!\n");
    			    return RET_FAILURE;					
				}
				
				pcm_buf = (UINT8 *)__G_ALI_MM_DECA_MEM_START_ADDR;				
				#endif
				
                copy_from_user(pcm_buf,(void *)(*(UINT32 *)g_ali_audio_rpc_arg[1]),*(UINT32 *)g_ali_audio_rpc_arg[0]);
                
				#ifndef CONFIG_ARM
                dma_cache_wback((unsigned long)pcm_buf, *(UINT32 *)g_ali_audio_rpc_arg[0]);
				#else
				pcm_buf = (UINT8 *)((((UINT32)pcm_buf)&0x0fffffff) | 0xa0000000);
				#endif
				
				#ifndef CONFIG_ARM
                bs_buf = kmalloc(*(UINT32 *)g_ali_audio_rpc_arg[2],GFP_KERNEL);
                if(bs_buf == NULL)
    		    {
    			    PRINTK_INFO("kmalloc request buf fail\n");
    			    return RET_FAILURE  ;
    		    }
				#else
				bs_buf = (UINT8 *)__G_ALI_MM_DECA_MEM_START_ADDR + (*(UINT32 *)g_ali_audio_rpc_arg[0]);
				#endif
				
                copy_from_user(bs_buf,(void *)(*(UINT32 *)g_ali_audio_rpc_arg[3]),*(UINT32 *)g_ali_audio_rpc_arg[2]);

				#ifndef CONFIG_ARM
                dma_cache_wback((unsigned long)bs_buf, *(UINT32 *)g_ali_audio_rpc_arg[2]);             
				#else
				bs_buf = (UINT8 *)((((UINT32)bs_buf)&0x0fffffff) | 0xa0000000);
				#endif
				           
                //printk("user%p kernel%p user%p kernel%p\n",*(UINT32 *)g_ali_audio_rpc_arg[1],pcm_buf,*(UINT32 *)g_ali_audio_rpc_arg[3],bs_buf);
                 deca_process_pcm_bitstream(*(UINT32 *)g_ali_audio_rpc_arg[0], pcm_buf, 
                    *(UINT32 *)g_ali_audio_rpc_arg[2], bs_buf, *(UINT32 *)g_ali_audio_rpc_arg[4], 
                    *(UINT32 *)g_ali_audio_rpc_arg[5], *(UINT32 *)g_ali_audio_rpc_arg[6]);
                 
				 #ifndef CONFIG_ARM
                 kfree(pcm_buf);
                 kfree(bs_buf);
				 #endif
              }
              break;             
       default:
                break;
    }
    return ret;
    
}
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long ali_m36_audio_ioctl(struct file *file,
			   unsigned int cmd, unsigned long parg)
#else
static int ali_m36_audio_ioctl(struct inode *inode, struct file *file,
			   unsigned int cmd, unsigned long parg)
#endif
{
	struct ali_audio_device *dev = file->private_data;
	//unsigned long arg = (unsigned long) parg;
    	unsigned char volume;
	RET_CODE ret = 0;
    RET_CODE copy_ret = 0;
	static int tone_voice_status=0;

	ali_audio_api_show("%s,%d,cmd:0x%x,parg:0x%x\n", __FUNCTION__, __LINE__, cmd, parg);
	
	down(&m_audio_sem);
	
	PRINTK_INFO("%s: %p, cmd=%04x\n", __FUNCTION__, dev, cmd);
/*
	if (((file->f_flags & O_ACCMODE) == O_RDONLY) &&
	    (cmd != AUDIO_GET_STATUS))
		return -EPERM;
*/
	switch (cmd) {
		case AUDIO_STOP:
			if(deca_stop_in_kernel)
			{
				/*if deca stop was called in kernel before rather than App/hld ,we need to call deca start first,then
				call deca stop,otherwise deca stop return failure!*/
				deca_start(dev->deca_dev, 0);
				deca_stop_in_kernel = 0;
			}
			ret = deca_stop(dev->deca_dev, 0, ADEC_STOP_IMM);
			break;
			
		case AUDIO_PLAY:
			m_audio_pause = 0;
			ret = deca_start(dev->deca_dev, 0);
			break;
			
		case AUDIO_PAUSE:
			ret = deca_pause(dev->deca_dev);
			break;
			
		case AUDIO_CONTINUE:
			ret = deca_start(dev->deca_dev, 0);
			break;
	        case AUDIO_ASE_INIT:        //added
	        {
	            deca_init_ase(dev->deca_dev);
	            break;
	        }
	        case AUDIO_ASE_STR_PLAY: 
	        {
	    		struct ali_audio_rpc_pars pars;
				
				#ifndef CONFIG_ARM
	            if(g_ali_audio_rpc_tmp == NULL)
	            {
	                g_ali_audio_rpc_tmp = kmalloc(1024*5, GFP_KERNEL);
	                if(g_ali_audio_rpc_tmp == NULL)
					{
	                    printk("---->g_ali_audio_rpc_tmp kmalloc error\n");

						up(&m_audio_sem);		
	                    return -1;
	                }
	            }
				#endif
				
	    		copy_from_user((void *)&pars, (void *)parg, sizeof(pars)); 
	            
				if(pars.arg_num != 0)
	            {
	                copy_from_user((void *)g_ali_audio_rpc_arg[0], pars.arg[0].arg,pars.arg[0].arg_size); 
	            } 

				#ifndef CONFIG_ARM
	            copy_from_user((void *)g_ali_audio_rpc_tmp, ((struct ase_str_play_param *)g_ali_audio_rpc_arg[0])->src,((struct ase_str_play_param *)g_ali_audio_rpc_arg[0])->len); 
	            ((struct ase_str_play_param *)g_ali_audio_rpc_arg[0])->src = (void *)g_ali_audio_rpc_tmp;				
				
				dma_cache_wback((UINT32)g_ali_audio_rpc_tmp, ((struct ase_str_play_param *)g_ali_audio_rpc_arg[0])->len);

				#else
				if ((((struct ase_str_play_param *)g_ali_audio_rpc_arg[0])->len) > __G_ALI_MM_DECA_MEM_SIZE)
				{
					printk("request buf size is too big, pls change it smaller!\n");
					
					up(&m_audio_sem);		
	                return -1;				
				}
				
				g_ali_audio_rpc_tmp = (volatile unsigned long *)__G_ALI_MM_DECA_MEM_START_ADDR;
                
				copy_from_user((void *)g_ali_audio_rpc_tmp, ((struct ase_str_play_param *)g_ali_audio_rpc_arg[0])->src,((struct ase_str_play_param *)g_ali_audio_rpc_arg[0])->len); 
				
	            ((struct ase_str_play_param *)g_ali_audio_rpc_arg[0])->src = (UINT8 *)((((UINT32)g_ali_audio_rpc_tmp)&0x0fffffff) | 0xa0000000);
                #endif
				
	           // ret = deca_io_control(dev->deca_dev, DECA_STR_PLAY, (struct ase_str_play_param *)g_ali_audio_rpc_arg[0]); 
	            ret = deca_io_control(dev->deca_dev, DECA_STR_PLAY, (UINT32)g_ali_audio_rpc_arg[0]);
	            break;
	        }
	        case AUDIO_ASE_STR_STOP: 
	        {
	            ret = deca_io_control(dev->deca_dev, DECA_STR_STOP, 0);
                #ifndef CONFIG_ARM
	            if(g_ali_audio_rpc_tmp != NULL){
	                kfree((void *)g_ali_audio_rpc_tmp);
	                g_ali_audio_rpc_tmp = NULL;
	            }
                #endif
	            break;
	        }
	        case AUDIO_ASE_DECA_BEEP_INTERVAL:
	        {
	            ret = deca_io_control(dev->deca_dev, DECA_BEEP_INTERVAL, parg);
	            break;
	        }		
		case AUDIO_SELECT_SOURCE:
			//av7110->audiostate.stream_source = (audio_stream_source_t) arg;
			break;

		case AUDIO_SET_MUTE:
			ret = snd_set_mute(dev->snd_dev,SND_SUB_OUT, parg);
			break;

		case AUDIO_SET_AV_SYNC:
			ret = deca_set_sync_mode(dev->deca_dev, (enum ADecSyncMode)parg);
			break;

		case AUDIO_SET_BYPASS_MODE:
        {
            #if defined(CONFIG_ARM)
			enum ASndOutSpdifType spdif_type = (enum ASndOutSpdifType)parg;

            //spdif_type = SND_OUT_SPDIF_BS; //add for test

            //printk("\n%s,%d, spdif_type = %d\n\n", __func__, __LINE__, spdif_type);
			
        	snd_io_control(dev->snd_dev, FORCE_SPDIF_TYPE, SND_OUT_SPDIF_INVALID);
        	if (spdif_type == SND_OUT_SPDIF_PCM)
        	{
                //printk("---%s,%d: SND_OUT_SPDIF_PCM ---\n\n", __func__, __LINE__);
        		snd_io_control(dev->snd_dev, FORCE_SPDIF_TYPE, SND_OUT_SPDIF_PCM);
        	}
        	else if (spdif_type == SND_OUT_SPDIF_BS)
        	{
                printk("---%s,%d: SND_OUT_SPDIF_BS ---\n\n", __func__, __LINE__);
        		snd_io_control(dev->snd_dev, FORCE_SPDIF_TYPE, SND_OUT_SPDIF_BS);
        		deca_io_control(dev->deca_dev, DECA_EMPTY_BS_SET, 0x00);
        		deca_io_control(dev->deca_dev, DECA_ADD_BS_SET, AUDIO_AC3);
                deca_io_control(dev->deca_dev, DECA_ADD_BS_SET, AUDIO_EC3);
                deca_io_control(dev->deca_dev, DECA_ADD_BS_SET, AUDIO_DTS);
        	}
            /*
           	else if (spdif_type == SND_OUT_SPDIF_FORCE_DD)
        	{
                printk("---%s,%d---\n\n", __func__, __LINE__);
        	    snd_io_control(dev->snd_dev, FORCE_SPDIF_TYPE, SND_OUT_SPDIF_FORCE_DD);
        		deca_io_control(dev->deca_dev, DECA_EMPTY_BS_SET, 0x00);
        		deca_io_control(dev->deca_dev, DECA_ADD_BS_SET, AUDIO_AC3);
                deca_io_control(dev->deca_dev, DECA_ADD_BS_SET, AUDIO_EC3);
                deca_io_control(dev->deca_dev, DECA_ADD_BS_SET, AUDIO_DTS);
        	}
        	*/
            #else
            ret = snd_set_spdif_type(dev->snd_dev, (enum ASndOutSpdifType)parg);
            #endif
    
			break;
		}

		case AUDIO_CHANNEL_SELECT:
		    printk("%s,%d,channel:%d\n", __FUNCTION__, __LINE__, parg);
			ret = snd_set_duplicate(dev->snd_dev, (enum SndDupChannel)parg);
			break;

		case AUDIO_GET_STATUS:
			//memcpy(parg, &av7110->audiostate, sizeof(struct audio_status));
			break;

		case AUDIO_GET_CAPABILITIES:
	/*		if (FW_VERSION(av7110->arm_app) < 0x2621)
				*(unsigned int *)parg = AUDIO_CAP_LPCM | AUDIO_CAP_MP1 | AUDIO_CAP_MP2;
			else
				*(unsigned int *)parg = AUDIO_CAP_LPCM | AUDIO_CAP_DTS | AUDIO_CAP_AC3 |
							AUDIO_CAP_MP1 | AUDIO_CAP_MP2;
	*/
			break;

		case AUDIO_CLEAR_BUFFER:
	/*		dvb_ringbuffer_flush_spinlock_wakeup(&av7110->aout);
			av7110_ipack_reset(&av7110->ipack[0]);
			if (av7110->playing == RP_AV)
				ret = av7110_fw_cmd(av7110, COMTYPE_REC_PLAY,
						    __Play, 2, AV_PES, 0);
	*/
			break;
			
		case AUDIO_SET_ID:
			break;
			
		case AUDIO_SET_MIXER:
			break;
			
		case AUDIO_SET_STREAMTYPE:
			ret = deca_io_control(dev->deca_dev, DECA_SET_STR_TYPE, (enum AudioStreamType)parg);
			break;
		case AUDIO_SET_VOLUME:
			ret = snd_set_volume(dev->snd_dev, SND_SUB_OUT, parg);
			break;
        case AUDIO_GET_VOLUME:
            volume = snd_get_volume(dev->snd_dev);
            copy_to_user((void *)parg, &volume, sizeof(volume));
            break;
	        
		case AUDIO_GET_INPUT_CALLBACK_ROUTINE:	
			break;
		case AUDIO_GEN_TONE_VOICE:
			deca_tone_voice(dev->deca_dev, parg, tone_voice_status);
			tone_voice_status=1;
			break;
		case AUDIO_STOP_TONE_VOICE:
			deca_stop_tone_voice(dev->deca_dev);
			tone_voice_status=0;
			break;

		case AUDIO_EMPTY_BS_SET:
	 		ret = deca_io_control(dev->deca_dev, DECA_EMPTY_BS_SET, parg);
			break;

		case AUDIO_ADD_BS_SET:
	      	ret = deca_io_control(dev->deca_dev, DECA_ADD_BS_SET,  (enum AudioStreamType)parg);
	        break;

		case AUDIO_DEL_BS_SET:
	      	ret = deca_io_control(dev->deca_dev, DECA_DEL_BS_SET, (enum AudioStreamType)parg);
			break;
		
		case AUDIO_DECA_SET_DBG_LEVEL:
		    deca_set_dbg_level(dev->deca_dev, parg);
		    break;
				
		case AUDIO_SND_SET_DBG_LEVEL:
		    snd_set_dbg_level(dev->snd_dev, parg);
		   break;   

		case AUDIO_SND_START:
		    snd_start(dev->snd_dev);
		    break;
		case AUDIO_SND_STOP:
		    snd_stop(dev->snd_dev);
		    break;
		case AUDIO_SND_STC_INVALID:
		   stc_invalid();
		   break;
		case AUDIO_SND_STC_VALID:
		   stc_valid();
		   break;
		case AUDIO_SET_CTRL_BLK_INFO:
			copy_from_user(&dev->audio_cb,(void *)parg,sizeof(ali_audio_ctrl_blk));
			dev->cb_avail = 1;
			break;
		case AUDIO_DECA_IO_COMMAND:
		{
			struct ali_audio_ioctl_command io_param;

            copy_from_user(&io_param, (void *)parg, sizeof(struct ali_audio_ioctl_command));
            
            if ((DECA_GET_STR_TYPE == io_param.ioctl_cmd) || \
                (DECA_GET_HIGHEST_PTS == io_param.ioctl_cmd) || \
                (DECA_GET_AC3_BSMOD == io_param.ioctl_cmd) || \
                (DECA_CHECK_DECODER_COUNT == io_param.ioctl_cmd) || \
                (DECA_GET_DESC_STATUS == io_param.ioctl_cmd) || \
                (DECA_GET_DECODER_HANDLE == io_param.ioctl_cmd) || \
                (DECA_GET_DECA_STATE == io_param.ioctl_cmd) || \
                (DECA_SOFTDEC_GET_ELAPSE_TIME2 == io_param.ioctl_cmd) || \
                (DECA_DOLBYPLUS_CONVERT_STATUS == io_param.ioctl_cmd) || \
                (DECA_GET_BS_FRAME_LEN == io_param.ioctl_cmd) || \
                (DECA_GET_DDP_INMOD == io_param.ioctl_cmd))
            {
                UINT32 deca_param;
                ret = deca_io_control(dev->deca_dev, io_param.ioctl_cmd, (UINT32)&deca_param);

				if (RET_SUCCESS == ret)
				{
					if((copy_ret = copy_to_user((void *)io_param.param, (void *)&deca_param, sizeof(unsigned int))) !=  0)
					{
						printk("%s error line%d\n", __FUNCTION__, __LINE__);
                        
						// Invalid user space address
						return -EFAULT;
					}
				}
            }
            else
            {
                ret = deca_io_control(dev->deca_dev, io_param.ioctl_cmd, io_param.param);
            }
			
			break;
		}
	        
		case AUDIO_SND_IO_COMMAND:
		{
			struct ali_audio_ioctl_command io_param;
            
			copy_from_user(&io_param, (void *)parg, sizeof(struct ali_audio_ioctl_command));
            
            if ((SND_CHK_DAC_PREC == io_param.ioctl_cmd) || \
                (SND_GET_RAW_PTS == io_param.ioctl_cmd) || \
                (SND_REQ_REM_DATA == io_param.ioctl_cmd) || \
                (SND_GET_TONE_STATUS == io_param.ioctl_cmd) || \
                (SND_CHK_PCM_BUF_DEPTH == io_param.ioctl_cmd) || \
                (SND_GET_SAMPLES_REMAIN == io_param.ioctl_cmd) || \
                (SND_REQ_REM_PCM_DATA == io_param.ioctl_cmd) || \
                (SND_REQ_REM_PCM_DURA == io_param.ioctl_cmd) || \
                (SND_GET_SPDIF_TYPE == io_param.ioctl_cmd) || \
                (SND_GET_MUTE_TH == io_param.ioctl_cmd))
            {
                UINT32 snd_param;
			    ret = snd_io_control(dev->snd_dev, io_param.ioctl_cmd, &snd_param);

				if (RET_SUCCESS == ret)
				{
					if ((copy_ret = copy_to_user((void *)io_param.param, (void *)&snd_param, sizeof(unsigned int))) !=  0)
					{
						printk("%s error line%d\n", __FUNCTION__, __LINE__);
                        
						// Invalid user space address
						return -EFAULT;
					}
				}
            }
            else
            {
			    ret = snd_io_control(dev->snd_dev, io_param.ioctl_cmd, io_param.param);
            }
            
			break;
		}
		
		case AUDIO_RPC_CALL_ADV:
		{  
			struct ali_audio_rpc_pars pars;
			int i = 0;

		    copy_from_user((void *)&pars, (void *)parg, sizeof(pars));//copy

		    if(pars.arg_num > MAX_AUDIO_RPC_ARG_NUM)
		    {
		         PRINTK_INFO("illegal rpc arg_num,max 8\n");

		    up(&m_audio_sem);	
				
			    return -EINVAL;	  
		    }
	            
		    for(i = 0; i < pars.arg_num; i++)
		    {
		        g_ali_audio_rpc_arg_size[i] = pars.arg[i].arg_size;	//check size			
		        if(g_ali_audio_rpc_arg_size[i] > 0)
		        {	
		        	if(g_ali_audio_rpc_arg[i] == NULL || g_ali_audio_rpc_arg_size[i] > MAX_AUDIO_RPC_ARG_SIZE)
		        	{
		        		PRINTK_INFO("ali audio allocate rpc arg buf fail\n");
		                ret = -ENOMEM;
		                goto RPC_EXIT_2;
		        	}
		        	copy_from_user((void *)g_ali_audio_rpc_arg[i], pars.arg[i].arg, g_ali_audio_rpc_arg_size[i]);//check param
		        }   
		    }
	            
		    ret = audio_rpc_operation(dev,pars.API_ID);
		   	
RPC_EXIT_2:
			for(i = 0; i < pars.arg_num;i++)
			{
				if(g_ali_audio_rpc_arg_size[i] > 0)
				{
					if(pars.arg[i].out)
					{
						copy_to_user(pars.arg[i].arg, (void *)g_ali_audio_rpc_arg[i], g_ali_audio_rpc_arg_size[i]);
					}
		        }		
			}
			
			break;
		} 
		
    case AUDIO_DECORE_COMMAND:
    {
		struct ali_audio_rpc_pars pars;
		int i = 0;

        if(down_interruptible(&dev->sem))
        {
		    return -EINVAL;
	    }
		copy_from_user((void *)&pars, (void *)parg, sizeof(pars));
		for(i = 0; i < pars.arg_num; i++)
		{
			g_ali_audio_rpc_arg_size[i] = pars.arg[i].arg_size;
			if(g_ali_audio_rpc_arg_size[i] > 0)
			{
				if(g_ali_audio_rpc_arg[i] == NULL || g_ali_audio_rpc_arg_size[i] > MAX_AUDIO_RPC_ARG_SIZE)
				{
                    ret = -ENOMEM;
				    goto RPC_EXIT;
				}
				copy_from_user((void *)g_ali_audio_rpc_arg[i], pars.arg[i].arg, g_ali_audio_rpc_arg_size[i]);
			}
		}
		#if defined(CONFIG_ARM)
        ret = deca_decore_ioctl(ali_m36_audio_dev.deca_dev, *(int *)g_ali_audio_rpc_arg[0], g_ali_audio_rpc_arg[1], g_ali_audio_rpc_arg[2]);
		#endif
RPC_EXIT:
		for(i = 0; i < pars.arg_num;i++)
		{
			if(g_ali_audio_rpc_arg_size[i] > 0)
			{
				if(pars.arg[i].out)
					copy_to_user(pars.arg[i].arg, (void *)g_ali_audio_rpc_arg[i], g_ali_audio_rpc_arg_size[i]);
			}
		}
        up(&dev->sem);
        break;
    }
    case AUDIO_GET_PTS:
    {
        unsigned long cur_pts = 0;
        cur_pts = *((unsigned long *)0xb8002044);
        copy_to_user((unsigned long *)parg,&cur_pts,4);
        break;
    }
		default:
			ret = -ENOIOCTLCMD;
	}

	up(&m_audio_sem);	

	ali_audio_api_show("%s,%d\n", __FUNCTION__, __LINE__);
	
	return ret;
}

RET_CODE ali_m36_audio_open(struct inode *inode, struct file *file)
{
	struct deca_device *see_deca_dev=NULL;
	struct snd_device *see_snd_dev=NULL;
    int i = 0;

	ali_audio_api_show("%s,%d.\n", __FUNCTION__, __LINE__);	

	down(&m_audio_sem);
	
	if(g_ali_audio_rpc_arg[0] == NULL){
		for(i = 0; i < MAX_AUDIO_RPC_ARG_NUM; i++){
			g_ali_audio_rpc_arg[i] = kmalloc(MAX_AUDIO_RPC_ARG_SIZE, GFP_KERNEL);
			if(g_ali_audio_rpc_arg[i] == NULL){
				PRINTK_INFO("ali audio malloc rpc arg buf fail\n");

				up(&m_audio_sem);
				return -EINVAL;	
			}
		}
	}

    	if((!ali_m36_audio_dev.deca_open_flag) 
			|| (!ali_m36_audio_dev.snd_open_flag)) 
  	{
		see_deca_dev=(struct deca_device*)hld_dev_get_by_type(NULL, HLD_DEV_TYPE_DECA);
		see_snd_dev=(struct snd_device*)hld_dev_get_by_type(NULL, HLD_DEV_TYPE_SND);

		if((NULL==see_deca_dev) ||(NULL==see_snd_dev))
		{
			PRINTK_INFO("%s : see DECA/SND device attach failed.\n", __FUNCTION__);

			up(&m_audio_sem);
			return -ENODEV;
		}

		if(!ali_m36_audio_dev.snd_open_flag)
		{
			ali_m36_audio_dev.snd_open_flag = 1;
			snd_open(see_snd_dev);
		}
        
		//deca_open(see_deca_dev, AUDIO_MPEG2, AUDIO_SAMPLE_RATE_48, AUDIO_QWLEN_24, 2, 0);
		//snd_open(see_snd_dev);
		if(!ali_m36_audio_dev.deca_open_flag)
		{
			ali_m36_audio_dev.deca_open_flag = 1;
			deca_open(see_deca_dev, AUDIO_MPEG2, AUDIO_SAMPLE_RATE_48, AUDIO_QWLEN_24, 2, 0);
		}
		
		//PRINTK_INFO("malloc priv OK: %08x\n", priv);
		ali_m36_audio_dev.deca_dev= see_deca_dev;
		ali_m36_audio_dev.snd_dev= see_snd_dev;
       }
		
	file->private_data = (void*)&ali_m36_audio_dev;

    	ali_m36_audio_dev.open_count++;

   	if(ali_m36_audio_dev.open_count == 1)
 	{
		hld_deca_rpc_release(1);
      }    	
	//printk("<0>""%s : open count %d\n", __FUNCTION__, ali_m36_audio_dev.open_count);
	
	up(&m_audio_sem);

	ali_audio_api_show("%s,%d.\n", __FUNCTION__, __LINE__);
	
	return 0;
}

static RET_CODE ali_m36_audio_release(struct inode *inode, struct file *file)
{
	struct ali_audio_device *dev = file->private_data;
	//struct deca_device *see_deca_dev=NULL;
	//struct snd_device *see_snd_dev=NULL;
   	 int i = 0;
	ali_audio_api_show("%s,%d.\n", __FUNCTION__, __LINE__);

	down(&m_audio_sem);
	
    	dev->open_count--;

   	if(dev->open_count == 0)
 	{
 		m_audio_pause = 0;
		hld_deca_rpc_release(0);
		
		#ifndef CONFIG_ARM
		for (i = 0; i < MAX_AUDIO_RPC_ARG_NUM; i++)
		{
			if (g_ali_audio_rpc_arg[i])
			{
				kfree((void *)g_ali_audio_rpc_arg[i]);
				g_ali_audio_rpc_arg[i] = NULL;
			}
		}
		#endif

		//deca_close(see_deca_dev);
		//snd_close(see_snd_dev);
      }

	//printk("<0>""%s : open count %d\n", __FUNCTION__, ali_m36_audio_dev.open_count);
	
	up(&m_audio_sem);

	ali_audio_api_show("%s,%d.\n", __FUNCTION__, __LINE__);
	
	return 0;
}

static ssize_t ali_m36_audio_write(struct file *file, const char __user *buf,
			       size_t count, loff_t *ppos)
{

   struct ali_audio_device *dev = (struct ali_audio_device *)file->private_data;
    void *req_buf = NULL;
    int req_ret = 0;
    int ret = 0;

	ali_audio_api_show("%s,%d.\n", __FUNCTION__, __LINE__);
	
    unsigned int req_size = count;
    unsigned int ret_size;
    
START:
	
	if(dev->cb_avail)
	{
		req_ret = deca_request_write(dev->deca_dev,req_size, &req_buf, (UINT32 *)&ret_size, &dev->audio_cb);
		dev->cb_avail = 0;
	}
	else
		req_ret =  deca_request_write(dev->deca_dev,req_size, &req_buf, (UINT32 *)&ret_size, NULL);

	if(req_ret == RET_SUCCESS)
	{
		{
				void *tmp_buf = NULL;
				
				#if defined(CONFIG_ARM)
                void *tmp_req_buf = NULL;
				#endif
				
			//	printk("request ok buf %x size 0x%x\n", (int)req_buf, ret_size);
				tmp_buf = kmalloc(ret_size, GFP_KERNEL);
				if(tmp_buf == NULL)
				{
					PRINTK_INFO("kmalloc request buf fail\n");
					ret = -EINVAL;
					goto EXIT;
				}

				copy_from_user(tmp_buf, buf, ret_size);
				
				#if defined(CONFIG_ARM)
				__CACHE_FLUSH_ALI((unsigned long)tmp_buf, ret_size);
				//VIDEO_PRF("start to transfer the data to see src %x dst %x size 0x%x\n"
				//	, (int)tmp_buf, (int)req_buf, req_size);
				/* Main CPU should use below mempy to transfer the data to private 
				memory */
                tmp_req_buf = (void *)(__VSTMALI((unsigned int)req_buf));
				
				//hld_dev_memcpy(req_buf, tmp_buf, ret_size);
				memcpy(tmp_req_buf, tmp_buf, ret_size); // change by jacket for s3921
				#else
				dma_cache_wback((unsigned long)tmp_buf, ret_size);
				//VIDEO_PRF("start to transfer the data to see src %x dst %x size 0x%x\n"
				//	, (int)tmp_buf, (int)req_buf, req_size);
				/* Main CPU should use below mempy to transfer the data to private 
				memory */
				hld_dev_memcpy(req_buf, tmp_buf, ret_size);
				#endif
				
				kfree(tmp_buf);
		}
	
		deca_update_write(dev->deca_dev,ret_size);
		
		ret += ret_size;
		
		if(ret_size < count)
		{
			count = req_size = count - ret_size;
			goto START;
		}
		
	}
	else {
			msleep(10);
			goto START;
			//ret = -EINVAL;
		}

EXIT:	
	ali_audio_api_show("%s,%d.\n", __FUNCTION__, __LINE__);
	return ret;
}

extern UINT32 deca_standby(struct deca_device * device, UINT32 status);

int ali_m36_audio_suspend(struct device *dev, pm_message_t state)
{
    UINT32 pause_dec;
    UINT32 pause_output ;
    struct ali_audio_device *a_dev = dev_get_drvdata(dev);
    struct deca_device *see_deca_dev=NULL;
    see_deca_dev=a_dev->deca_dev;
    
    pause_dec = 1;
    pause_output= 1;
    #if defined(CONFIG_ARM)
    deca_decore_ioctl(see_deca_dev,DECA_DECORE_PAUSE_DECODE,(void *)&pause_dec,(void *)&pause_output);
	#endif
/*
    deca_stop(see_deca_dev, 0, ADEC_STOP_IMM);
    deca_standby(see_deca_dev, 1);
    mdelay(100);
 */   

    return 0;
}

int ali_m36_audio_resume(struct device *dev)
{
    UINT32 pause_dec;
    UINT32 pause_output ;
    struct ali_audio_device *a_dev = dev_get_drvdata(dev);
    struct deca_device *see_deca_dev=NULL;
    see_deca_dev=a_dev->deca_dev;
	
	ali_audio_api_show("%s,%d.\n", __FUNCTION__, __LINE__);

    if(m_audio_pause == 0)
    {
    	pause_dec = 0;
    	pause_output = 0;
    	#if defined(CONFIG_ARM)
    	deca_decore_ioctl(see_deca_dev,DECA_DECORE_PAUSE_DECODE,(void *)&pause_dec,(void *)&pause_output);
    	#endif
    }

/*
    deca_standby(see_deca_dev, 0);
    deca_start(see_deca_dev, 0);
*/
	ali_audio_api_show("%s,%d.\n", __FUNCTION__, __LINE__);

    return 0;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
static int ali_m36_audio_dev_init(void)
#else
static int __devinit ali_m36_audio_dev_init(void)
#endif
{
	int ret;
	INT32   result;
	dev_t devno;
	/*
	struct deca_feature_config deca_cfg;
	struct snd_feature_config snd_cfg;
	*/
	PRINTK_INFO("%s start.\n", __FUNCTION__);

	PRINTK_INFO("register audio device start.\n");

	ali_audio_api_show("%s,%d.\n", __FUNCTION__, __LINE__);
	
	//ret = dvb_register_device(&ali_m36_adapter, &ali_m36_audio_dev,
	//		    &ali_m36_audio_dev_tmp, priv, DVB_DEVICE_AUDIO);

/*
	memset(&deca_cfg,0,sizeof(struct deca_feature_config));
	deca_m36_attach(&deca_cfg);

	memset(&snd_cfg,0,sizeof(struct snd_feature_config));
	snd_cfg.output_config.mute_num = 12;
	snd_cfg.output_config.mute_polar 	= 0;
	snd_cfg.output_config.dac_precision = 24;
	snd_cfg.output_config.dac_format 	= CODEC_I2S;
	snd_cfg.output_config.is_ext_dac 	= 0;
	snd_cfg.output_config.ext_mute_mode = MUTE_BY_GPIO;	
	snd_cfg.support_spdif_mute = 1;
	snd_cfg.output_config.chip_type_config = 1; //QFP
	snd_m36_attach(&snd_cfg);
*/
	ret=alloc_chrdev_region(&devno, 0, 1, ALI_AUDIO_DEVICE_NAME);
	if(ret<0)
	{
		PRINTK_INFO("Alloc device region failed, err: %d.\n",ret);
		return ret;
	}

	memset((void *)&ali_m36_audio_dev, 0, sizeof(ali_m36_audio_dev));
	
	cdev_init(&ali_m36_audio_dev.cdev, &ali_m36_audio_fops);
	ali_m36_audio_dev.cdev.owner=THIS_MODULE;
	ali_m36_audio_dev.cdev.ops=&ali_m36_audio_fops;
	ret=cdev_add(&ali_m36_audio_dev.cdev, devno, 1);
	if(ret)
	{
		PRINTK_INFO("Alloc audio device failed, err: %d.\n", ret);
		return ret;
	}
	
	PRINTK_INFO("register audio device end.\n");


	ali_m36_audio_class = class_create(THIS_MODULE, "ali_m36_audio_class");

	if (IS_ERR(ali_m36_audio_class))
	{
		result = PTR_ERR(ali_m36_audio_class);

		goto err1;
	}

	ali_m36_audio_dev_node = device_create(ali_m36_audio_class, NULL, devno, &ali_m36_audio_dev, 
                           "ali_m36_audio0");
         if (IS_ERR(ali_m36_audio_dev_node))
    {
		printk(KERN_ERR "device_create() failed!\n");

		result = PTR_ERR(ali_m36_audio_dev_node);

		goto err2;
	}
	ali_m36_audio_class->suspend = ali_m36_audio_suspend;
	ali_m36_audio_class->resume = ali_m36_audio_resume;

	//void ali_m36_audio_test(struct file_operations *fop,struct ali_audio_device *dev);
	//ali_m36_audio_test(&ali_m36_audio_fops, &ali_m36_audio_dev);

    sema_init(&m_audio_sem, 1);

	ali_audio_info_init();	

	ali_audio_api_show("%s,%d.ret:%d\n", __FUNCTION__, __LINE__, ret);

	return ret;

err2:
	class_destroy(ali_m36_audio_class);
err1:
	cdev_del(&ali_m36_audio_dev.cdev);
	//kfree(priv);
	ali_audio_api_show("%s,%d.\n", __FUNCTION__, __LINE__);
	
	return -EINVAL;
}


static void __exit ali_m36_audio_dev_exit(void)
{
	ali_audio_api_show("%s,%d.\n", __FUNCTION__, __LINE__);

	if(ali_m36_audio_dev_node != NULL)
		device_del(ali_m36_audio_dev_node);

	if(ali_m36_audio_class != NULL)
		class_destroy(ali_m36_audio_class);
	cdev_del(&ali_m36_audio_dev.cdev);

	ali_audio_info_exit();		

	ali_audio_api_show("%s,%d.\n", __FUNCTION__, __LINE__);
}

module_init(ali_m36_audio_dev_init);
module_exit(ali_m36_audio_dev_exit);

MODULE_DESCRIPTION("driver for the Ali M36xx(Dual Core) audio/sound device");
MODULE_AUTHOR("ALi Corp ShangHai SDK Team, Eric Li");
MODULE_LICENSE("GPL");


