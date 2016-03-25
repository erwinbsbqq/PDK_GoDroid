#include <basic_types.h>
#include <mediatypes.h>	
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include <api/libttx/lib_ttx.h>
#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <err/errno.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>

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
