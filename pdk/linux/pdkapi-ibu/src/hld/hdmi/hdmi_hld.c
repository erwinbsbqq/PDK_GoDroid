/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    hdmi_hld.c
*
*    Description:    This file contains all functions definition
*		             of linux emulation TDS HDMI driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Aug.09.2011      Seiya.Cao       Ver 0.1    Create file.
*****************************************************************************/

#include <retcode.h>// for ZHA
//corei#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
#include <osal/osal_int.h>
#include <hld/hld_dev.h>
#include <bus/hdmi/m36/hdmi_dev.h>
#include <bus/hdmi/m36/hdmi_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <misc/rfk.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
//corei#include <hdmi_io_common.h>
#include <rpc_hld/ali_rpc_hld.h>    //corei

#include <linux/poll.h>


#define HDMI_HLD_DBG
#ifdef HDMI_HLD_DBG
#define HDMI_HLD_PRINTF printf
#else
#define HDMI_HLD_PRINTF(...) do{}while(0)
#endif

#define MONITOR_BY_NETLINK
#define MSG_EDID_RDY		0x01
#define MSG_PLUG_IN			0x02
#define MSG_PLUG_OUT		0x03
#define MSG_CEC_RCV			0x04


static struct hdmi_device *m_hdmi_device = NULL;
static int 	m_hdmi_hdl = -1;

static void hdmi_monitor_by_netlink(UINT32 param1, UINT32 param2)
{
	struct hdmi_device *dev = (struct hdmi_device *)param1;
	unsigned char *hdmi_data = NULL;
	UINT8  msg_type   = 0;
	UINT16 msg_length = 0;
	UINT8 *msg_data   = 0;
	
	HDMI_HLD_PRINTF("%s:\n", __FUNCTION__);

	while(1)
	{
		//hdmi_data = rfk_receive_msg(dev->net_id);
		hdmi_data = (unsigned char *)rfk_receive_msg(dev->net_id);//add by ze, 3912 cec test
		if(hdmi_data != NULL)
		{
			msg_type   = hdmi_data[0];
			msg_length = (hdmi_data[1] << 8) | hdmi_data[2];
			msg_data   = &(hdmi_data[3]);
			HDMI_HLD_PRINTF("%s: rfk_receive_msg, type %d len %d,%x \n", __FUNCTION__, msg_type, msg_length,msg_length);
            HDMI_HLD_PRINTF("%s: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __FUNCTION__, hdmi_data[0], hdmi_data[1], hdmi_data[2], hdmi_data[3], hdmi_data[4], hdmi_data[5]);//add by ze

			switch (msg_type)
			{
				default:
					HDMI_HLD_PRINTF("%s: hdmi error notify!\n", __FUNCTION__);
					break;
				case MSG_EDID_RDY:
					HDMI_HLD_PRINTF("%s: hdmi edid ready to user notify!\n", __FUNCTION__);
					if(dev && dev->cb_edid_ready)
						dev->cb_edid_ready();
					break;
				case MSG_PLUG_IN:
					HDMI_HLD_PRINTF("%s: hdmi plug in to user notify!\n", __FUNCTION__);
					if(dev && dev->cb_fn) // for SHA
						dev->cb_fn(HDMI_RECEIVER_PLUGIN, 1, (void *)NULL, dev->cb_fn_data);
					if(dev && dev->cb_hotplug_in)
					    dev->cb_hotplug_in();
					break;
				case MSG_PLUG_OUT:
					HDMI_HLD_PRINTF("%s: hdmi plug out to user notify!\n", __FUNCTION__);
					if(dev && dev->cb_fn) // for SHA
						dev->cb_fn(HDMI_RECEIVER_PLUGOUT, 1, (void *)NULL, dev->cb_fn_data);
					if(dev && dev->cb_hotplug_out)
					    dev->cb_hotplug_out();
					break;
				case MSG_CEC_RCV:
					HDMI_HLD_PRINTF("%s: hdmi rcv cec msg to user notify!\n", __FUNCTION__);
					if(dev && dev->cb_cec_receive)
						dev->cb_cec_receive(msg_data, msg_length);
					break;
			}
		}
		else
		{
			HDMI_HLD_PRINTF("%s: rfk_receive_msg err!!\n", __FUNCTION__);
		}
	}
}

static void hdmi_monitor_by_polling(UINT32 param1, UINT32 param2)
{
	#define 	LONGMAX 	0x7FFFFFFF
	struct hdmi_device *dev = (struct hdmi_device *)param1;

	struct pollfd hdmi_fd =
	{
		.fd 		= (unsigned long)dev->handle,
		.events 	= POLLIN | POLLOUT,
		.revents	= 0,
	};

	while(1)
	{
		if(poll(&hdmi_fd, 1, LONGMAX) > 0)
		{
			if((hdmi_fd.revents & POLLIN) == POLLIN)
			{
				HDMI_HLD_PRINTF("%s: hdmi plug in to user notify!\n", __FUNCTION__);
				if(dev && dev->cb_fn)
				{
					dev->cb_fn(HDMI_RECEIVER_PLUGIN, 1, (void *)NULL, dev->cb_fn_data);
				}
			}
			else if((hdmi_fd.revents & POLLOUT) == POLLOUT)
			{
				HDMI_HLD_PRINTF("%s: hdmi plug out to user notify!\n", __FUNCTION__);
				if(dev && dev->cb_fn)
				{
					dev->cb_fn(HDMI_RECEIVER_PLUGOUT, 1, (void *)NULL, dev->cb_fn_data);
				}
			}
		}
		osal_task_sleep(100);
	}
}

static RET_CODE  hdmi_register_monitor(struct hdmi_device *dev)
{

#ifdef MONITOR_BY_NETLINK
	OSAL_T_CTSK		t_ctsk;
	t_ctsk.stksz	= 0x6000;
	t_ctsk.quantum	= 10;
	t_ctsk.itskpri	= OSAL_PRI_NORMAL;
	t_ctsk.name[0]	= 'H';
	t_ctsk.name[1]	= 'D';
	t_ctsk.name[2]	= 'M';
	t_ctsk.para1 	= (UINT32)dev;
	t_ctsk.para2 	= 0;
	t_ctsk.task 	= (FP)hdmi_monitor_by_netlink;
	
	dev->hdm_mon_tid = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID == dev->hdm_mon_tid)
	{
		HDMI_HLD_PRINTF("cre_tsk sys_data_hdmi_init failed\n");
		return RET_FAILURE;
	}
	osal_task_sleep(5);

#else
	OSAL_T_CTSK		t_ctsk_;
	t_ctsk_.stksz	= 0x6000;
	t_ctsk_.quantum	= 10;
	t_ctsk_.itskpri	= OSAL_PRI_NORMAL;
	t_ctsk_.name[0]	= 'H';
	t_ctsk_.name[1]	= 'D';
	t_ctsk_.name[2]	= 'M';
	t_ctsk_.para1 	= (UINT32)dev;
	t_ctsk_.para2 	= 0;
	t_ctsk_.task 	= (FP)hdmi_monitor_by_polling;
	
	dev->hdm_mon_tid = osal_task_create(&t_ctsk_);
	if(OSAL_INVALID_ID == dev->hdm_mon_tid)
	{
		HDMI_HLD_PRINTF("cre_tsk sys_data_hdmi_init failed\n");
		return RET_FAILURE;
	}
	osal_task_sleep(5);
	
#endif 
	return RET_SUCCESS;
}

static RET_CODE hdmi_hld_ioctl(struct hdmi_device *dev, UINT32 cmd, UINT32 param1, UINT32 param2)
{
	RET_CODE ret = RET_SUCCESS;

	if(!dev)
	{
		HDMI_HLD_PRINTF("%s in: but dev is NULL\n", __FUNCTION__);
	}

	switch(cmd)
	{
		default:
			HDMI_HLD_PRINTF("%s in:hdmi not support cmd!\n", __FUNCTION__);
			ret = RET_FAILURE;
			break;
		case HDMI_CMD_REG_CALLBACK:
		{
			switch(param1)
			{
				case HDMI_CB_EDID_READY:
					dev->cb_edid_ready = (HDMI_EDID_CBFunc) param2;
					break;
				case HDMI_CB_HOT_PLUG_OUT:
					dev->cb_hotplug_out = (HDMI_HTPLG_CBFunc) param2;
					break;
				case HDMI_CB_HOT_PLUG_IN:
					dev->cb_hotplug_in = (HDMI_HTPLG_CBFunc) param2;
					break;
				case HDMI_CB_CEC_MESSAGE:
					dev->cb_cec_receive = (HDMI_CEC_CBFunc) param2;
					break;
				default:
					ret = RET_FAILURE;
					break;
			}
		}
		break;

		case HDMI_CMD_DEL_CALLBACK:
		{
			switch(param1)
			{
				case HDMI_CB_EDID_READY:
					dev->cb_edid_ready = (HDMI_EDID_CBFunc) param2;
					break;
				case HDMI_CB_HOT_PLUG_OUT:
					dev->cb_hotplug_out = (HDMI_HTPLG_CBFunc) param2;
					break;
				case HDMI_CB_HOT_PLUG_IN:
					dev->cb_hotplug_in = (HDMI_HTPLG_CBFunc) param2;
					break;
				case HDMI_CB_CEC_MESSAGE:
					dev->cb_cec_receive = (HDMI_CEC_CBFunc) param2;
					break;
				default:
					ret = RET_FAILURE;
					break;
			}
		}
		break;

		
		case HDMI_CMD_SET_TMDS_DRV_CURRENT:
		{
			switch(param1)
			{
				case HDMI_DRV_CUR_8MA:
				 	if(!ioctl(dev->handle, HDMI_IOC_SET8MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_85MA:
				 	if(!ioctl(dev->handle, HDMI_IOC_SET85MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_9MA:
				 	if(!ioctl(dev->handle, HDMI_IOC_SET9MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_95MA:
				 	if(!ioctl(dev->handle, HDMI_IOC_SET95MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_10MA:
				 	if(!ioctl(dev->handle, HDMI_IOC_SET10MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_105MA:
				 	if(!ioctl(dev->handle, HDMI_IOC_SET105MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_11MA:
				 	if(!ioctl(dev->handle, HDMI_IOC_SET11MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_115MA:
				 	if(!ioctl(dev->handle, HDMI_IOC_SET115MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_12MA:
				 	if(!ioctl(dev->handle, HDMI_IOC_SET12MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_dRV_CUR_125MA:
					 if(!ioctl(dev->handle, HDMI_IOC_SET125MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_13MA:
					 if(!ioctl(dev->handle, HDMI_IOC_SET13MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_135MA:
				 	if(!ioctl(dev->handle, HDMI_IOC_SET135MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_14MA:
				 	if(!ioctl(dev->handle, HDMI_IOC_SET14MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_145MA:
					if(!ioctl(dev->handle, HDMI_IOC_SET145MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_15MA:
					if(!ioctl(dev->handle, HDMI_IOC_SET15MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_155MA:
					if(!ioctl(dev->handle, HDMI_IOC_SET155MA, 0))
						ret = RET_FAILURE;
					break;
				case HDMI_DRV_CUR_16MA:
					if(!ioctl(dev->handle, HDMI_IOC_SET16MA, 0))
						ret = RET_FAILURE;
					break;
				default:
					ret = RET_FAILURE;
					break;
			}
		}
			break;
		case HDMI_CMD_INTERFACE_MODE:
			if(!ioctl(dev->handle, HDMI_IOCG_HDMIMODE, (void*)param1))
				ret = RET_FAILURE;
			break;
	}
	
	return ret;
	
}

RET_CODE hdmi_tx_open(struct hdmi_device *dev)
{
	RET_CODE ret = RET_SUCCESS;
	
	if (dev->flags & HLD_DEV_STATS_UP)
	{
		HDMI_HLD_PRINTF("%s in: warning - device %s openned already!\n", __FUNCTION__, dev->name);
		ret = RET_FAILURE;
		goto  EXIT;
	}
	
	m_hdmi_hdl = open("/dev/ali_hdmi_device", O_RDWR);
	if(m_hdmi_hdl <= 0)
	{
		HDMI_HLD_PRINTF("%s in:open HDMI handle fail\n", __FUNCTION__);
		ret = RET_FAILURE;
		goto EXIT;
	}
	else
	{
		if (0 == (ioctl(m_hdmi_hdl, HDMI_IOCINI_HDMIHW, dev->hdcp_key)))
		{
			HDMI_HLD_PRINTF("%s:Telling - device %s openned successfully!\n", __FUNCTION__, dev->name);
			dev->handle = m_hdmi_hdl;
		}
		else
		{
			HDMI_HLD_PRINTF("%s in:alarm - device %s initialize failed!\n", __FUNCTION__, dev->name);
			close(m_hdmi_hdl);
			ret = RET_FAILURE;
			goto EXIT;
		}
	}
	

	dev->net_id = rfk_get_port();
	if((dev->net_id < 0)||(dev->net_id > MAX_RFK_PORT_NUM))
	{
		HDMI_HLD_PRINTF("%s in: HDMI get port fail\n", __FUNCTION__);
		ret = RET_FAILURE;
		close(m_hdmi_hdl);
		goto EXIT;
	}	
	
	if (0 == ioctl(dev->handle, HDMI_IOCT_NETPORT, dev->net_id))
	{
		HDMI_HLD_PRINTF("%s in: HDMI register netlink port successfully\n", __FUNCTION__);
	}
	else
	{
		HDMI_HLD_PRINTF("%s in: HDMI register netlink port failed\n", __FUNCTION__);
	}

	if(hdmi_register_monitor(dev) == RET_FAILURE)
	{
		HDMI_HLD_PRINTF("%s in:failed to create hdmi monitor thread!\n", __FUNCTION__);
		ret = RET_FAILURE;
		close(m_hdmi_hdl);
		rfk_free_port(dev->net_id);
		goto EXIT;
	}

/* Seiya test
{
	unsigned int addr;
	if ((addr = mmap(0, 4096*2, PROT_READ, MAP_PRIVATE, hdmi_hdl, 0)) == (void *) -1)
	{
		HDMI_HLD_PRINTF("%s in\n", __FUNCTION__);
	}
	HDMI_HLD_PRINTF("%s in addr = 0x%08x data = 0x%08x\n", __FUNCTION__,addr,*((unsigned int*)addr));
	unsigned int *p = (unsigned int*)addr;
	int ip = 0;
	for(ip = 0; ip < 32; ip ++)
	{
		HDMI_HLD_PRINTF("0x%08x\t",*((unsigned int*)(addr + 4 * ip)));
		if (ip%4 == 3)
		{
			HDMI_HLD_PRINTF("\n");
		}
	}
}
*/
	
	dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
EXIT:
	return ret;
}

RET_CODE hdmi_tx_attach (void)
{
	RET_CODE ret = RET_SUCCESS;
	struct hdmi_device *dev;
	int i = 0;
	
	HDMI_HLD_PRINTF("%s !\n", __FUNCTION__);

	dev = (struct hdmi_device *)dev_alloc("HDMI_DEV", HLD_DEV_TYPE_HDMI, sizeof(struct hdmi_device));
	if (dev == NULL)
	{
		HDMI_HLD_PRINTF("%s in: Error: Alloc hdmi device error!\n", __FUNCTION__);
	    ret = RET_FAILURE;
		goto EXIT;
	}

	dev->flags		= 0;
	dev->net_id		= -1;
	dev->init		= NULL;
	dev->stop_tx	= NULL;
	dev->io_control	= hdmi_hld_ioctl;
	dev->open		= hdmi_tx_open;
	dev->close		= hdmi_tx_close;
	dev->start_tx	= NULL;

	dev->cb_fn		= NULL;
	dev->cb_fn_data	= NULL;
	
	/* Add this device to queue */
	if (dev_register(dev) != RET_SUCCESS )
	{
		HDMI_HLD_PRINTF("%s in: Error: Register hdmi device error!\n", __FUNCTION__);
	    dev_free(dev);
	    ret = RET_FAILURE;
		goto EXIT;
	}

	m_hdmi_device = dev;
//	hdmi_tx_open(m_hdmi_device); // For SHA

EXIT:
	return ret;
}

RET_CODE hdmi_tx_close(struct hdmi_device *dev)
{
	INT32 ret = RET_SUCCESS;
	HDMI_HLD_PRINTF("%s !\n", __FUNCTION__);
	if (close(dev->handle))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		ret = RET_FAILURE;
	}
	else
		ret = RET_SUCCESS;
	
	return ret;
}

RET_CODE hdmi_tx_detach (void)
{
	RET_CODE ret = RET_SUCCESS;

	HDMI_HLD_PRINTF("%s !\n", __FUNCTION__);

	if(m_hdmi_device != NULL)
	{
		hdmi_tx_close(m_hdmi_device);
		m_hdmi_device = NULL;
	}
	return ret;
}

/*api function from AP to get EDID support video format */
RET_CODE api_get_edid_video_format(struct hdmi_device *dev, enum PicFmt *format)
{
	HDMI_ioctl_video_format_t cmd;
	if(ioctl(dev->handle, HDMI_IOCT_GET_VIDEO_FORMAT, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	else
	{
		*format = cmd.format;
		return RET_SUCCESS;
	}
}

/*api function from AP to get EDID native video resolution */
RET_CODE api_get_edid_video_resolution(struct hdmi_device *dev, enum HDMI_API_RES *res)
{
	RET_CODE ret = RET_SUCCESS;
	int edid_rdy;
	enum HDMI_API_RES native;

	if(ioctl(dev->handle, HDMI_IOCG_EDIDRDY, (void*)&edid_rdy))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	else
	{
		if (edid_rdy == 0)
		{
			HDMI_HLD_PRINTF("%s in: Error: edid not ready!\n", __FUNCTION__);
			return RET_FAILURE;
		}
		else if(edid_rdy == 1)
		{
			if(ioctl(dev->handle, HDMI_IOCG_NATIVERES, (void*)&native))
			{
				HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
				return RET_FAILURE;
			}
			else
			{
				*res = native;
				return RET_SUCCESS;
			}
		}
	}
}

/*api function from AP to get edid all video resolution. */
RET_CODE api_get_edid_all_video_resolution(struct hdmi_device *dev, UINT32 *native_res_index, enum HDMI_API_RES *video_res)
{
	int edid_rdy;
	int native;
	HDMI_ioctl_edid_res_list_t cmd;

	if(ioctl(dev->handle, HDMI_IOCG_EDIDRDY, (void*)&edid_rdy))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	else
	{
		if (edid_rdy == 0)
		{
			HDMI_HLD_PRINTF("%s in: Error: edid not ready!\n", __FUNCTION__);
			return RET_FAILURE;
		}
		else if(edid_rdy == 1)
		{
			if(ioctl(dev->handle, HDMI_IOCG_GET_ALL_VID_RES, &cmd))
			{
				HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
				return RET_FAILURE;
			}
			else
			{
				*native_res_index = cmd.native_res_index;
				memcpy(video_res, cmd.video_res_list, sizeof(enum HDMI_API_RES) * HDMI_API_RES_SUPPORT_NUM);
				return RET_SUCCESS;
			}
		}
	}
}

/*api function from AP to get EDID prefer audio coding type. */
RET_CODE api_get_edid_audio_out(struct hdmi_device *dev, enum EDID_AUD_FMT_CODE *aud_fmt)
{
	UINT16 aud_fmt_code = 0x00;
	if (ioctl(dev->handle, HDMI_IOCG_HDMIAUDIO, (void*)&aud_fmt_code))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	else
	{
		if ((aud_fmt_code & EDID_AUDIO_AC3))
			*aud_fmt = EDID_AUDIO_AC3;
		else if ((aud_fmt_code & EDID_AUDIO_MPEG1))
			*aud_fmt = EDID_AUDIO_MPEG1;
		else if ((aud_fmt_code & EDID_AUDIO_MP3))
			*aud_fmt = EDID_AUDIO_MP3;
		else if ((aud_fmt_code & EDID_AUDIO_MPEG2))
			*aud_fmt = EDID_AUDIO_MPEG2;
		else if ((aud_fmt_code & EDID_AUDIO_AAC))
			*aud_fmt = EDID_AUDIO_AAC;
		else if ((aud_fmt_code & EDID_AUDIO_DTS))
			*aud_fmt = EDID_AUDIO_DTS;
		else
			*aud_fmt = EDID_AUDIO_LPCM;
		return RET_SUCCESS;
	}
}

/*api function from AP to get edid all audio coding type. */
RET_CODE api_get_edid_all_audio_out(struct hdmi_device *dev, enum EDID_AUD_FMT_CODE *aud_fmt)
{

	UINT16 aud_fmt_code = 0x00;

	if(ioctl(dev->handle, HDMI_IOCG_HDMIAUDIO, (void*)&aud_fmt_code))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	else
	{
		*aud_fmt = aud_fmt_code;
		return RET_SUCCESS;	
	}	
}

void api_set_hdmi_sw_onoff(struct hdmi_device *dev, BOOL bOnOff)
{
	HDMI_ioctl_sw_onoff_state_t cmd;
	cmd.hdmi_status = bOnOff;
	if(ioctl(dev->handle, HDMI_IOCT_SET_ONOFF, &cmd))
	{
		//print error message.
//		printf("errno %d\n", errno);
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
	}
}

BOOL api_get_hdmi_sw_onoff(struct hdmi_device *dev)
{
	HDMI_ioctl_sw_onoff_state_t cmd;

	if(ioctl(dev->handle, HDMI_IOCT_GET_ONOFF, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return FALSE;
	}
	else
	{
		return cmd.hdmi_status;
	}
}

// For HDMI HDCP APIs
/* api function from AP to set hdcp on/off */
void api_set_hdmi_hdcp_onoff(struct hdmi_device *dev, BOOL bOnOff)
{
	HDMI_ioctl_hdcp_state_t cmd;
	cmd.hdcp_status = bOnOff;
	if(ioctl(dev->handle, HDMI_IOCT_HDCP_SET_ONOFF, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
	}
}

/* api function from AP to get hdcp on/off status */
BOOL api_get_hdmi_hdcp_onoff(struct hdmi_device *dev)
{
	HDMI_ioctl_hdcp_state_t cmd;

	if(ioctl(dev->handle, HDMI_IOCT_HDCP_GET_ONOFF, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return FALSE;
	}
	else
	{
		return cmd.hdcp_status;
	}
}

void api_set_hdmi_mem_sel(struct hdmi_device *dev, BOOL mem_sel)//add by ze for ce_load_key or sw_load_key
{
	HDMI_ioctl_mem_sel_t cmd;
	cmd.mem_sel = mem_sel;
	if(ioctl(dev->handle, HDMI_IOCT_HDCP_MEM_SEL, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
	}
	else
	{
		printf("GOOD: dev = 0x%X\n", dev);
	}
}

HDMI_ioctl_hdcp_key_info_t cmd;
/* api function from AP to set hdcp key info */
BOOL api_set_hdmi_hdcp_key_info(struct hdmi_device *dev, unsigned char *key)
{
	cmd.scramble = *key;
	memcpy(cmd.hdcp_ksv, (key+1), 5);
	memcpy(cmd.encrypted_hdcp_keys, (key+6), 280);

	if(ioctl(dev->handle, HDMI_IOCT_HDCP_SET_KEY_INFO, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

// For HDMI CEC APIs
/* api function from AP to set cec on/off */
RET_CODE api_set_hdmi_cec_onoff(struct hdmi_device *dev, BOOL bOnOff)
{
	HDMI_ioctl_cec_state_t cmd;
	cmd.cec_status = bOnOff;
	if(ioctl(dev->handle, HDMI_IOCT_CEC_SET_ONOFF, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	else
	{
		return RET_SUCCESS;
	}
}

/* api function from AP to get cec on/off status */
BOOL api_get_hdmi_cec_onoff(struct hdmi_device *dev)
{
	HDMI_ioctl_cec_state_t cmd;
	if(ioctl(dev->handle, HDMI_IOCT_CEC_GET_ONOFF, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return FALSE;
	}
	else
	{
		return cmd.cec_status;
	}
}

/* api function from AP to transmit cec message */
RET_CODE api_hdmi_cec_transmit(struct hdmi_device *dev, UINT8 *message, UINT8 message_length)
{
	HDMI_ioctl_cec_msg_t cmd;
	memcpy(cmd.message, message, message_length);
	cmd.message_length = message_length;

	if(ioctl(dev->handle, HDMI_IOCT_CEC_TRANSMIT, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	else
	{
		return RET_SUCCESS;
	}
}

/* api function from AP to get the HDMI physical address for cec */
UINT16 api_get_physical_address(struct hdmi_device *dev)
{
	HDMI_ioctl_cec_addr_t cmd;
	if(ioctl(dev->handle, HDMI_IOCT_CEC_GET_PA, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return 0xFFFF;
	}
	else
	{	if(cmd.ret)
			return cmd.cec_addr;
	}
}

/* api function from AP to set the HDMI logical address for cec */
RET_CODE api_set_logical_address(struct hdmi_device *dev, UINT8 logical_address)
{
	HDMI_ioctl_cec_addr_t cmd;
	cmd.cec_addr = logical_address;
	if(ioctl(dev->handle, HDMI_IOCT_CEC_SET_LA, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	else
	{
		return RET_SUCCESS;
	}
}

/* api function from AP to get the HDMI logical address for cec */
UINT8 api_get_logical_address(struct hdmi_device *dev)
{
	HDMI_ioctl_cec_addr_t cmd;
	if(ioctl(dev->handle, HDMI_IOCT_CEC_GET_LA, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return 0xFF;
	}
	else
	{
		return cmd.cec_addr;
	}
}

/* api function from AP to set the HDMI resolution for cec keep resolution */
RET_CODE api_hdmi_set_res(struct hdmi_device *dev, enum HDMI_API_RES res)
{
	HDMI_ioctl_video_res_t cmd;
	cmd.video_res = res;
	if(ioctl(dev->handle, HDMI_IOCT_SET_VID_RES, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	else
		return RET_SUCCESS;

}

/* api function from AP to get the HDMI resolution for cec keep resolution */
RET_CODE api_hdmi_get_res(struct hdmi_device *dev, enum HDMI_API_RES *res)
{
	HDMI_ioctl_video_res_t cmd;

	if(ioctl(dev->handle, HDMI_IOCT_GET_VID_RES, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	else
	{
		*res = cmd.video_res;
		return RET_SUCCESS;
	}
}

#if 1 // for UDI
/* api function from AP to get HDMI device count. */
RET_CODE api_hdmi_get_dev_count(struct hdmi_device *dev, int * pnHdmiDeviceCount)
{
	*pnHdmiDeviceCount = 1;
	return RET_SUCCESS;
}

/* api function from AP to set HDMI vendor name. */
RET_CODE api_hdmi_set_vendor_name(struct hdmi_device *dev, unsigned char *vendor_name, unsigned char length)
{
	HDMI_ioctl_vendor_name_t cmd;

//	printf("vendor_name %s\n", vendor_name);
	if (length > 8)
		length = 8;
	memcpy(cmd.vendor_name, vendor_name, length);
	cmd.length = length;
	
	if(ioctl(dev->handle, HDMI_IOCT_SET_VENDOR_NAME, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	return RET_SUCCESS;
}

/* api function from AP to set HDMI product description. */
RET_CODE api_hdmi_set_product_desc(struct hdmi_device *dev, unsigned char *product_desc, unsigned char length)
{
	HDMI_ioctl_product_desc_t cmd;

//	printf("product_desc %s\n", product_desc);
	if (length > 16)
		length = 16;
	memcpy(cmd.product_desc, product_desc, length);
	cmd.length = length;
		
	if(ioctl(dev->handle, HDMI_IOCT_SET_PRODUCT_DESC, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}

	return RET_SUCCESS;
}

/* api function from AP to get HDMI vendor name. */
RET_CODE api_hdmi_get_vendor_name(struct hdmi_device *dev, unsigned char *vendor_name,  unsigned char *length)
{
	HDMI_ioctl_vendor_name_t cmd;
		
	if(ioctl(dev->handle, HDMI_IOCT_GET_VENDOR_NAME, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}

	memcpy(vendor_name, cmd.vendor_name, cmd.length);
	*length = cmd.length;
//	printf("vendor_name %s\n", vendor_name);

	return RET_SUCCESS;
}

/* api function from AP to get HDMI product description. */
RET_CODE api_hdmi_get_product_desc(struct hdmi_device *dev, unsigned char *product_desc,  unsigned char *length)
{
	HDMI_ioctl_product_desc_t cmd;
		
	if(ioctl(dev->handle, HDMI_IOCT_GET_PRODUCT_DESC, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}

	memcpy(product_desc, cmd.product_desc, cmd.length);
	*length = cmd.length;
//	printf("product_desc %s\n", product_desc);

	return RET_SUCCESS;
}

/* api function from AP to get HDMI link status. */
RET_CODE api_hdmi_get_link_status(struct hdmi_device *dev,	int *link_status)
{
	HDMI_ioctl_link_status_t cmd;
	
	if(ioctl(dev->handle, HDMI_IOCT_GET_LINK_ST, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}

	*link_status = cmd.link_status;
	return RET_SUCCESS;
}


/* api function from AP to register HDMI call back function. */
RET_CODE api_hdmi_reg_callback(struct hdmi_device *dev, void *cb_func, void *pvUserData)
{

	if(dev == NULL)
	{
		HDMI_HLD_PRINTF("%s in: Error!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	
	if((dev->cb_fn == cb_func) && (dev->cb_fn_data == pvUserData))
	{
		HDMI_HLD_PRINTF("%s in: Error!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	else
	{
		dev->cb_fn = cb_func;
		dev->cb_fn_data = pvUserData;
		return RET_SUCCESS;
	}
}

/* api function from AP to delete HDMI call back function. */
RET_CODE api_hdmi_del_callback(struct hdmi_device *dev, void *cb_func, void *pvUserData)
{
	if(dev == NULL)
	{
		HDMI_HLD_PRINTF("%s in: Error!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	
	if((dev->cb_fn == cb_func) && (dev->cb_fn_data == pvUserData))
	{
		dev->cb_fn = NULL;
		dev->cb_fn_data = NULL;
		return RET_SUCCESS;
	}
	else
	{
		HDMI_HLD_PRINTF("%s in: Error!\n", __FUNCTION__);
		return RET_FAILURE;
	}
}

/* api function from AP to set HDMI audio onoff description. */
RET_CODE api_hdmi_audio_set_onoff(struct hdmi_device *dev, BOOL bOnOff)
{
	HDMI_ioctl_hdmi_audio_state_t cmd;
	cmd.hdmi_audio_status= bOnOff;
	if(ioctl(dev->handle, HDMI_IOCT_SET_HDMI_AUDIO_ONOFF, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
              return RET_FAILURE;
	}
       else
              return RET_SUCCESS;
}

/* api function from AP to get HDMI audio onoff description. */
RET_CODE api_hdmi_audio_get_onoff(struct hdmi_device *dev, int *bOnOff)
{
    HDMI_ioctl_hdmi_audio_state_t cmd;

	if(ioctl(dev->handle, HDMI_IOCT_GET_HDMI_AUDIO_ONOFF, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	else
	{
	    *bOnOff = cmd.hdmi_audio_status;
		return RET_SUCCESS;
	}
}

/* api function from AP to get HDMI 3d present description. */
RET_CODE api_hdmi_get_3d_present(struct hdmi_device *dev, int *present)
{
    HDMI_ioctl_3d_status_t cmd;

	if(ioctl(dev->handle, HDMI_IOCT_GET_3D_PRESENT, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	else
	{
	    *present = cmd.present;
		return RET_SUCCESS;
	}
}

/* api function from AP to get HDMI edid block data.  */
RET_CODE api_hdmi_get_edid_block_data(struct hdmi_device *dev, int num, unsigned char *data)
{
	HDMI_ioctl_edid_block_data_t cmd;
	cmd.block_num = num;
	if(ioctl(m_hdmi_hdl, HDMI_IOCT_GET_EDID_BLOCK, &cmd))
	{
		HDMI_HLD_PRINTF("%s in: Error: ioctl!\n", __FUNCTION__);
		return RET_FAILURE;
	}
	else
	{
		memcpy(data, cmd.block, sizeof(cmd.block));
		return RET_SUCCESS;
	}
}

#endif

