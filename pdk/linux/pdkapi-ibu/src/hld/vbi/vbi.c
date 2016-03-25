#include <basic_types.h>
#include <mediatypes.h>	
//corei#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
//#include <api/libc/string.h>
#include <osal/osal.h>

//corei#include <api/libttx/lib_ttx.h>
#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
//corei#include <err/errno.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>

//corei#include <ali_vbi_common.h>

static int g_vbi_handle = 0;

char vbi_m36_name[HLD_MAX_NAME_SIZE] = "ali_vbi";
static char *g_vbi_m36_name = NULL;


INT32 vbi_open(struct vbi_device *dev)
{
	INT32 result=SUCCESS;
	int vbi_handle = 0;
	
	/* If openned already, exit */
	if (dev->flags & HLD_DEV_STATS_UP)
	{
		PRINTF("vbi_open: warning - device openned already!\n");
		return SUCCESS;
	}
	
	//strcpy(dev->name, "/dev/ali_vbi");
	vbi_handle = open("/dev/ali_vbi", O_RDONLY|O_NONBLOCK);
	if(vbi_handle < 0)
	{
		PRINTF("vbi_open: fail to open device %s!\n", dev->name);
		result =  !SUCCESS;
	}
	else
	{
		g_vbi_handle = vbi_handle;
		dev->priv = (void *)vbi_handle;
		dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
	}
	
	return result;
}


INT32 vbi_close(struct vbi_device *dev)
{
	int vbi_handle = 0;
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		PRINTF("vbi_close: warning - device %s closed already!\n", dev->name);
		return SUCCESS;
	}
	
	vbi_handle = (int)dev->priv;
	close(vbi_handle);
	dev->priv = (int)0;
	g_vbi_handle = 0;
	
	/* Update flags */
	dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
	
	return SUCCESS;
}


INT32 vbi_ioctl(struct vbi_device *dev, UINT32 cmd, UINT32 param)
{
	INT32 result=SUCCESS;
	int vbi_handle = 0;
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return SUCCESS;
	}
	
	vbi_handle = (int)dev->priv;
	result = ioctl(vbi_handle, cmd, param);
	return result;
}

INT32 vbi_request_write(void * pdev, UINT32 uSizeRequested,void ** ppuData,UINT32* puSizeGot,struct control_block* pDataCtrlBlk)
{
	return SUCCESS;
}

void vbi_update_write(void * pdev, UINT32 uDataSize)
{
}

void vbi_setoutput(struct vbi_device *dev,T_VBIRequest *pVBIRequest)
{
	int vbi_handle = 0;
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return;
	}
	
	vbi_handle = (int)dev->priv;
	ioctl(vbi_handle, CALL_VBI_SETOUTPUT, (void *)pVBIRequest);
}

INT32 vbi_start(struct vbi_device *dev,t_TTXDecCBFunc pCBFunc)
{
	INT32 result=SUCCESS;
	int vbi_handle = 0;
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return !SUCCESS;
	}
	
	vbi_handle = (int)dev->priv;
	result = ioctl(vbi_handle, CALL_VBI_START, (void *)pCBFunc);

	return result;
}

INT32 vbi_stop(struct vbi_device *dev)
{
	INT32 result=SUCCESS;
	int vbi_handle = 0;
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return !SUCCESS;
	}
	
	vbi_handle = (int)dev->priv;
	result = ioctl(vbi_handle, CALL_VBI_STOP, (void *)NULL);

	return result;
}

void ttx_default_g0_set(struct vbi_device *dev, UINT8 default_g0_set)
{
	int vbi_handle = 0;
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
		return;
	
	vbi_handle = (int)dev->priv;
	ioctl(vbi_handle, CALL_TTX_DEFAULT_G0_SET, (void *)NULL);
}


void TTXEng_Init(void)
{
	if (0 == g_vbi_handle)
	{
		return;
	}
	
	ioctl(g_vbi_handle, CALL_TTXENG_INIT, NULL);
}

void  TTXEng_Attach(struct ttx_config_par *pconfig_par)
{
	if (0 == g_vbi_handle)
	{
		return;
	}
	
	ioctl(g_vbi_handle, CALL_TTXENG_ATTACH, (void *)pconfig_par);
}


void enable_vbi_transfer(BOOL enable)
{
	if (0 == g_vbi_handle)
	{
		return;
	}
	
	ioctl(g_vbi_handle, CALL_ENABLE_VBI_TRANSFER, (void *)enable);
}

UINT16 get_inital_page()
{
	INT32 result=SUCCESS;
	
	if (0 == g_vbi_handle)
	{
		return;
	}
	
	result = ioctl(g_vbi_handle, CALL_GET_INITAL_PAGE, NULL);
	return result;
}

UINT8 get_inital_page_status()
{
	INT32 result=SUCCESS;
	
	if (0 == g_vbi_handle)
	{
		return;
	}
	
	result = ioctl(g_vbi_handle, CALL_GET_INITAL_PAGE_STATUS, NULL);
	return result;
}

UINT16 get_first_ttx_page()
{
	INT32 result=SUCCESS;
	
	if (0 == g_vbi_handle)
	{
		return;
	}
	
	result = ioctl(g_vbi_handle, CALL_GET_FIRST_TTX_PAGE, NULL);
	return result;
}

void vbi_m36_attach(void)
{
	struct vbi_device *dev;

	g_vbi_m36_name = (char *)vbi_m36_name;
	
	dev = dev_alloc(g_vbi_m36_name,HLD_DEV_TYPE_VBI,sizeof(struct vbi_device));
	if (dev == NULL)
	{
		PRINTF("Error: Alloc video vbiplay device error!\n");
		ASSERT(0);
	}

	if (dev_register(dev) != RET_SUCCESS)
	{
		PRINTF("Error: Register vbiplay device error!\n");
		dev_free(dev);
		ASSERT(0);
	}

}

void cc_vbi_show_on(BOOL bOn)
{
	
}


#if 0    //corei
/******api for closed caption only******************/
INT32 vbi_cc_init()
{
	if (0 == g_vbi_handle)
	{
		return;
	}
	
	ioctl(g_vbi_handle, CALL_VBI_CC_INIT, NULL);
}
INT32 atsc_cc_show_onoff(BOOL bOn)
{
	if (0 == g_vbi_handle)
	{
		return;
	}
	
	ioctl(g_vbi_handle, CALL_VBI_CC_SHOW_ONOFF, bOn);

}
void cc_vbi_show_on(BOOL bOn)
{
	
}

INT32 set_cc(UINT8 cc_channel)
{
	if (0 == g_vbi_handle)
	{
		return;
	}
	
	ioctl(g_vbi_handle, CALL_VBI_CC_SET_CC, cc_channel);
}
INT32 set_dtv_cc_service(UINT8 dtv_cc_service)
{
	if (0 == g_vbi_handle)
	{
		return;
	}
	
	ioctl(g_vbi_handle, CALL_VBI_CC_SET_DTV_SERVICE, dtv_cc_service);
}
INT32 set_dtv_cc(UINT16 dtv_cc_channel)
{
	if (0 == g_vbi_handle)
	{
		return;
	}
	
	ioctl(g_vbi_handle, CALL_VBI_CC_SET_DTV, dtv_cc_channel);
}
UINT8 get_dtv_cs_number(void)
{
	UINT8 ret;
	
	if (0 == g_vbi_handle)
	{
		return 0;
	}
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	ret = ioctl(g_vbi_handle, CALL_VBI_CC_GET_CS_NUMBER, NULL);
}
UINT32 get_dtv_cs_list1(void)
{
	UINT32 ret;
	
	if (0 == g_vbi_handle)
	{
		return 0;
	}
	
	ret = ioctl(g_vbi_handle, CALL_VBI_CC_GET_CS_LIST1, NULL);
}
UINT32 get_dtv_cs_list2(void)
{
	UINT32 ret;
	
	if (0 == g_vbi_handle)
	{
		return 0;
	}
	
	ret = ioctl(g_vbi_handle, CALL_VBI_CC_GET_CS_LIST2, NULL);
}
UINT8 get_cc_control_byte(void)
{
	UINT8 ret;
	
	if (0 == g_vbi_handle)
	{
		return 0;
	}
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	ret = ioctl(g_vbi_handle, CALL_VBI_CC_GET_CONTROL_BYTE, NULL);
}
void set_eas_onoff(BOOL OnOff, UINT8* text, UINT16 len)
{
	
}
BOOL get_eas_onoff(void)
{
	BOOL ret;
	
	if (0 == g_vbi_handle)
	{
		return 0;
	}
	
	ret = ioctl(g_vbi_handle, CALL_VBI_CC_GET_EAS_ONFF, NULL);
}
UINT32 get_vbi_rating(void)
{
	UINT32 ret;
	
	if (0 == g_vbi_handle)
	{
		return 0;
	}
	
	ret = ioctl(g_vbi_handle, CALL_VBI_CC_GET_RATING, NULL);
}
BOOL get_vbi_cc(void)
{
	BOOL ret;
	
	if (0 == g_vbi_handle)
	{
		return 0;
	}
	
	ret = ioctl(g_vbi_handle, CALL_VBI_CC_GET_CC, NULL);
}
UINT8 get_vbi_cgmsa(void)//for cgmsa
{
	UINT8 ret;
	
	if (0 == g_vbi_handle)
	{
		return 0;
	}
	
	ret = ioctl(g_vbi_handle, CALL_VBI_CC_GET_CGMSA, NULL);
}
void disable_vbioutput(BOOL bOn)
{
	if (0 == g_vbi_handle)
	{
		return 0;
	}
	
	ioctl(g_vbi_handle, CALL_VBI_CC_DISABLE_OUTPUT, bOn);
}
void disable_process_cc(BOOL bOn)
{
	if (0 == g_vbi_handle)
	{
		return 0;
	}
	
	ioctl(g_vbi_handle, CALL_VBI_CC_DISABLE_PROCESS, bOn);
}
void set_vbi_rating_none()
{
	if (0 == g_vbi_handle)
	{
		return 0;
	}
	
	ioctl(g_vbi_handle, CALL_VBI_CC_SET_RATING_NONE, NULL);
}
void set_vbi_cgmsa_none() //for cgmsa
{
	if (0 == g_vbi_handle)
	{
		return 0;
	}
	
	ioctl(g_vbi_handle, CALL_VBI_CC_SET_CGMSA_NONE, NULL);
}
void set_vbi_cc_invalid()
{
	if (0 == g_vbi_handle)
	{
		return 0;
	}
	
	ioctl(g_vbi_handle, CALL_VBI_CC_SET_INVALID, NULL);
}
void set_dtvcc_war(BOOL dtv_cc_war)
{
	if (0 == g_vbi_handle)
	{
		return 0;
	}
	
	ioctl(g_vbi_handle, CALL_VBI_CC_SET_DTV_WAR, NULL);
}
#endif
