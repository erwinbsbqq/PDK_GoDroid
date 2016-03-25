/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    pan.c
*
*    Description:    This file contains all functions definition
*		             of Front panel driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Apr.21.2003      Justin Wu       Ver 0.1    Create file.
*****************************************************************************/

#include <retcode.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/pan/pan.h>
#include "pan_dbg.h"
#include <hld/pan/pan_dev.h>
#include <hld_cfg.h>
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
#include <linux/input.h>
#include <ali_front_panel_common.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>


static UINT8 g_ali_pan_repeat_enable = 1;
static UINT32 g_ali_pan_ir_format = 0xff;

static INT8 g_ali_pan_front_panel_name[HLD_MAX_NAME_SIZE] = CH455_DEV_NAME;
static INT8 g_ali_pan_display_backup[256] = {0};

static INT32 g_ali_pan_panel_handle = -1;
static INT32 g_ali_pan_ir_handle = -1;
static INT32 g_ali_pan_input_handle_ir = -1; //ir
static INT32 g_ali_pan_input_handle_panel = -1; //panel
static pan_device_status_e g_ali_pan_device_status = PAN_DEV_DETACH;

static UINT32 g_ali_pan_rfk_port_ir = 0;	
static UINT32 g_ali_pan_rfk_port_pan = 0;	

static struct pan_key	g_ali_pan_key = {0, 0, 0, 0};				/* Current input key */
static struct pan_key_index g_ali_pan_key_index = {{0, 0, 0, 0}, 0}; 		/* Current input key and key value index*/
static struct pan_key_info	g_ali_pan_key_info = {0, 0, 0, 0, 0};		/* Current input key */
static struct ali_fp_key_map_cfg g_ali_pan_ir_key_map = {NULL, 0, 0, 0, 0, 0};		/* ir key map */
static struct ali_fp_key_map_cfg g_ali_pan_panel_key_map = {NULL, 0, 0, 0, 0, 0};		/* panel key map */

/* dbg info */
static struct pan_dbg g_ali_pan_dbg;

#define PAN_MAX_PATH_SIZE		32
#define IR_PRINTF(fmt, args...)				\
{										\
	if (0 !=  (g_ali_pan_dbg.ir_level & PAN_DBG_LEVEL_HLD))				\
	{									\
		ADR_DBG_PRINT(fmt, ##args);					\
	}									\
}

#define PANEL_PRINTF(fmt, args...)			\
{										\
	if (0 !=  (g_ali_pan_dbg.panel_level & PAN_DBG_LEVEL_HLD))			\
	{									\
		ADR_DBG_PRINT(fmt, ##args);					\
	}									\
}

#define PAN_PRINTF(fmt, args...)				\
{										\
	if ((0 !=  (g_ali_pan_dbg.ir_level & PAN_DBG_LEVEL_HLD)) || \
		(0 !=  (g_ali_pan_dbg.panel_level &PAN_DBG_LEVEL_HLD)))	\
	{									\
		ADR_DBG_PRINT(fmt, ##args);					\
	}									\
}


#define PAN_KEY_PRINTF(type, fmt, args...)				\
{										\
	if (((0 !=  (g_ali_pan_dbg.ir_level & PAN_DBG_LEVEL_HLD)) && (type == PAN_KEY_TYPE_REMOTE)) || \
		((0 !=  (g_ali_pan_dbg.panel_level &PAN_DBG_LEVEL_HLD)) && (type == PAN_KEY_TYPE_PANEL)))	\
	{									\
		ADR_DBG_PRINT(fmt, ##args);					\
	}									\
}


#define PAN_ERR_PRINTF(fmt, args...)		\
{										\
	ADR_DBG_PRINT(fmt, ##args);						\
}


struct pan_dbg *pan_dbg_get_info(struct pan_device *dev)
{
	g_ali_pan_dbg.repeat_enable= g_ali_pan_repeat_enable;
	g_ali_pan_dbg.stats = g_ali_pan_device_status;

	/* ir */
	g_ali_pan_dbg.ir_handle = g_ali_pan_ir_handle;
	g_ali_pan_dbg.input_handle_ir = g_ali_pan_input_handle_ir;
	if (2 != g_ali_pan_ir_key_map.phy_code)
	{
		if (PAN_KEY_TYPE_REMOTE == g_ali_pan_key.type)
		{
			g_ali_pan_dbg.ir_last_key_high = 0;
			g_ali_pan_dbg.ir_last_key_low = g_ali_pan_key.code;
		}
	}
	else
	{
		if (PAN_KEY_TYPE_REMOTE == g_ali_pan_key_info.type)
		{
			g_ali_pan_dbg.ir_last_key_high = g_ali_pan_key_info.code_high;
			g_ali_pan_dbg.ir_last_key_low = g_ali_pan_key_info.code_low;
		}
	}	
	g_ali_pan_dbg.ir_format = g_ali_pan_ir_format;	
	memcpy(&(g_ali_pan_dbg.ir_key_map), &g_ali_pan_ir_key_map, sizeof(struct ali_fp_key_map_cfg));

	/* panel */
	g_ali_pan_dbg.panel_handle = g_ali_pan_panel_handle;	
	g_ali_pan_dbg.input_handle_panel = g_ali_pan_input_handle_panel;	
	if (2 != g_ali_pan_panel_key_map.phy_code)
	{
		if (PAN_KEY_TYPE_PANEL == g_ali_pan_key.type)
		{
			g_ali_pan_dbg.panel_last_key_high = 0;
			g_ali_pan_dbg.panel_last_key_low = g_ali_pan_key.code;
		}
	}
	else
	{
		if (PAN_KEY_TYPE_PANEL == g_ali_pan_key_info.type)
		{
			g_ali_pan_dbg.panel_last_key_high = g_ali_pan_key_info.code_high;
			g_ali_pan_dbg.panel_last_key_low = g_ali_pan_key_info.code_low;
		}
	}	
	memcpy(&(g_ali_pan_dbg.panel_key_map), &g_ali_pan_panel_key_map, sizeof(struct ali_fp_key_map_cfg));
	
	
	return &g_ali_pan_dbg;
}

INT32 pan_dbg_set_info(struct pan_device *dev, struct pan_dbg *info)
{
	memcpy(&g_ali_pan_dbg, info, sizeof(g_ali_pan_dbg));	

	return SUCCESS;
}

INT32 pan_get_panel_name(INT8 *dev_name)
{
	INT32 fd = -1;	
	INT32 ret = SUCCESS;
	INT32 read_size = 0;	
	INT8 buffer[HLD_MAX_NAME_SIZE];	


	if (NULL == dev_name)
	{		
		PAN_ERR_PRINTF("[ %s %d ], Invalid parameters!\n", __FUNCTION__, __LINE__);		
		return ERR_FAILURE;
	}

	memset(buffer, 0x00, sizeof(buffer));
	fd = open("/proc/panel", O_RDONLY|O_NONBLOCK);
	if (fd < 0)
	{				
		PAN_ERR_PRINTF("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);		
		return ERR_FAILURE;
	}

	read_size = read(fd, buffer, sizeof(buffer));
	if (-1 == read_size)
	{			
		PAN_ERR_PRINTF("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);			
		ret = ERR_FAILURE;		
	}
	else
	{			
		PANEL_PRINTF("[ %s %d ], dev name = %s\n", __FUNCTION__, __LINE__, buffer);			
		if (read_size > (INT32)sizeof(buffer))
		{
			read_size =  (INT32)sizeof(buffer);
		}		
		memcpy(dev_name, buffer, read_size);
	}	
	
	close(fd);

	return ret;
}


INT32 pan_attach(void)
{	
	INT8 dev_name[HLD_MAX_NAME_SIZE];
	struct pan_device *dev = NULL;
	INT32 ret = SUCCESS;


	if (PAN_DEV_DETACH != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d(want %d)\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status, PAN_DEV_ATTACH);		

		return (PAN_DEV_ATTACH == g_ali_pan_device_status) ? SUCCESS : ERR_STATUS;
	}
	

	memset(dev_name, 0x00, sizeof(dev_name));
	if (SUCCESS != pan_get_panel_name(dev_name))
	{
		PAN_ERR_PRINTF("[ %s %d ], No Panel!\n", __FUNCTION__, __LINE__);
		strcpy(g_ali_pan_front_panel_name, "No Panel");	
		//return ERR_FAILURE;
	}
	else 
	{				
		strcpy(g_ali_pan_front_panel_name, dev_name);				
		PANEL_PRINTF("[ %s %d ], panel = %s\n", __FUNCTION__, __LINE__, g_ali_pan_front_panel_name);		
	}
	
	
	(dev) = dev_alloc(g_ali_pan_front_panel_name, HLD_DEV_TYPE_PAN, sizeof(struct pan_device));	
	if ((dev) == NULL){
		PAN_ERR_PRINTF("%s error: Alloc front panel device error!\n", __FUNCTION__);		
		return ERR_NO_MEM;
	}	
	
	/* Add this device to queue */
	if ((ret = dev_register((dev))) != SUCCESS){	
		PAN_ERR_PRINTF("[ %s %d ], error %d: Register front panel device error!\n", __FUNCTION__, __LINE__, ret);
		dev_free((dev));
		return ERR_NO_DEV;
	}	
	
	memset(&g_ali_pan_ir_key_map, 0, sizeof(g_ali_pan_ir_key_map));
	memset(&g_ali_pan_panel_key_map, 0, sizeof(g_ali_pan_panel_key_map));	
	
	//snprintf((dev)->name, sizeof((dev)->name), "/dev/%s", dev_name);		
	PANEL_PRINTF("[ %s %d ], panel path = %s\n", __FUNCTION__, __LINE__, (dev)->name);

	//pan_module_register();
	
	g_ali_pan_device_status = PAN_DEV_ATTACH;
	

	return SUCCESS;
}


void pan_detach(struct pan_device *dev)
{
	if ((PAN_DEV_CLOSE != g_ali_pan_device_status) && (PAN_DEV_ATTACH != g_ali_pan_device_status))
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d(want %d)\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status, PAN_DEV_DETACH);

		return;				
	}
	

	if(g_ali_pan_ir_key_map.map_entry){
		free(g_ali_pan_ir_key_map.map_entry);
		g_ali_pan_ir_key_map.map_entry = NULL;
	}

	if(g_ali_pan_panel_key_map.map_entry){
		free(g_ali_pan_panel_key_map.map_entry);
		g_ali_pan_panel_key_map.map_entry = NULL;
	}
	if (dev)
	{
		dev_free(dev);
	}

	g_ali_pan_device_status = PAN_DEV_DETACH;
}


INT32 pan_get_input_name(INT8 *dev, INT8 *input_name)
{
	INT8 *position = NULL;
	INT8 *end = NULL;
	INT8 buffer[1024];	
	INT32 fd = -1;	
	INT32 ret = -ERR_FAILURE;
	INT32 read_size = 0;		


	if (NULL == dev)
	{
		PAN_ERR_PRINTF("[ %s %d ], device_name is NULL\n", __FUNCTION__, __LINE__);
		return ERR_NO_DEV;
	}
	
	PAN_PRINTF("[ %s %d ], dev = %s\n", __FUNCTION__, __LINE__, dev);	
	
	memset(buffer, 0x00, sizeof(buffer));	
	fd = open("/proc/bus/input/devices", O_RDONLY|O_NONBLOCK);
	if (fd < 0)
	{
		PAN_PRINTF("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		return ERR_FAILURE;
	}
	
	read_size = read(fd, buffer, sizeof(buffer));
	if (-1 == read_size)
	{
		PAN_ERR_PRINTF("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		ret = ERR_FAILURE;
		goto END;		
	}
	
	position = strstr(buffer, dev);
	if (NULL == position)	
	{
		PAN_ERR_PRINTF("[ %s %d ], Fail!, dev = %s\n", __FUNCTION__, __LINE__, dev);
		ret = ERR_FAILURE;
		goto END;
	}
	
	position = strstr(position, "Handlers");
	if (NULL == position)	
	{
		PAN_ERR_PRINTF("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		ret = ERR_FAILURE;
		goto END;
	}	

	position = strstr(position, "event");
	if (NULL == position)	
	{
		PAN_ERR_PRINTF("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		ret = ERR_FAILURE;
		goto END;
	}		

	if (strstr(position, " "))
	{
		end = strstr(position, " ");
	}
	else
	{
		end = strstr(position, "\n");
	}
	if (NULL != end)
	{		
		if ((end - position) <= 16)
		{
			memcpy(input_name, position, end - position);	
		}
		else
		{
			memcpy(input_name, position, 16);	
		}
		PAN_PRINTF("[ %s %d ], input_name = %s\n", __FUNCTION__, __LINE__, input_name);			
	}	
	else
	{
		PAN_ERR_PRINTF("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		ret = ERR_FAILURE;
		goto END;
	}

	ret = SUCCESS;
	

	END:
	{
		close(fd);
		return ret;
	}
}


/*
 * 	Name		:   pan_open()
 *	Description	:   Open a pan device
 *	Parameter	:	struct pan_device *dev		: Device to be openned
 *	Return		:	INT32 						: Return value
 *
 */
INT32 pan_open(struct pan_device *dev)
{	
	INT32 ret_ir = SUCCESS;
	INT32 ret_panel = SUCCESS;
	INT32 ret_pmu_key = SUCCESS;
	INT8 input_name[HLD_MAX_NAME_SIZE];
	INT8 input_path[PAN_MAX_PATH_SIZE];
	INT8 panel_path[PAN_MAX_PATH_SIZE];
	INT32 socket_id = 0;	
	INT32 flags = 0;

	
	if ((PAN_DEV_ATTACH != g_ali_pan_device_status) && (PAN_DEV_CLOSE != g_ali_pan_device_status))
	{		
		libc_printf("[ %s %d ], status error, now = %d(want %d)\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status, PAN_DEV_OPEN);		

		return (PAN_DEV_OPEN == g_ali_pan_device_status) ? SUCCESS : ERR_STATUS;		
	}	
	
	if(NULL==dev){
		libc_printf("%s error:  NULL device node!\n", __FUNCTION__);
		return ERR_PARA;
	}
	
	libc_printf("[ %s %d ], dev->name %s\n", __FUNCTION__, __LINE__, dev->name);		

	g_ali_pan_ir_handle = open("/dev/ali_ir",O_RDWR | O_NONBLOCK | O_CLOEXEC);
	if(g_ali_pan_ir_handle<0){
		libc_printf("%s open %s failed.\n",  __FUNCTION__, "/dev/ali_ir_g2");
		ret_ir |= ERR_NO_DEV;
	}

	snprintf(panel_path, sizeof(panel_path), "/dev/%s", dev->name);
	g_ali_pan_panel_handle = open(panel_path,O_RDWR | O_NONBLOCK | O_CLOEXEC);
	if(g_ali_pan_panel_handle<0){
		libc_printf("%s open %s failed.\n",  __FUNCTION__, panel_path);
		ret_panel |= ERR_NO_DEV;
	}    
    
	memset(input_path, 0x00, sizeof(input_path));
	memset(input_name, 0x00, sizeof(input_name));	
	if (SUCCESS == pan_get_input_name("ali_ir", input_name))
	{		
		#ifdef ADR_ALIDROID
		snprintf(input_path, sizeof(input_path), "/dev/input/%s", input_name);
		#else
		snprintf(input_path, sizeof(input_path), "/dev/%s", input_name);	
		#endif
		libc_printf("[ %s %d ], input_path = %s\n", __FUNCTION__, __LINE__, input_path);	
		
		g_ali_pan_input_handle_ir = open(input_path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);		
		if (g_ali_pan_input_handle_ir < 0)
		{			
			libc_printf("%s failed: g_ali_pan_input_handle_ir %s\n", __FUNCTION__, strerror(errno));
			ret_ir |= ERR_NO_DEV;			
		}        
	}
	
	
	g_ali_pan_rfk_port_ir = rfk_get_port();		
	if(g_ali_pan_rfk_port_ir <= 0)
	{
		libc_printf("%s : rfk get port fail\n", __FUNCTION__);			
		ret_ir |= ERR_NO_DEV;
	}
	else
	{
		socket_id = rfk_get_socket(g_ali_pan_rfk_port_ir);				
		flags = fcntl (socket_id, F_GETFL, 0);
		fcntl (socket_id, F_SETFL, flags | O_NONBLOCK);					
		if (RET_SUCCESS != ioctl(g_ali_pan_ir_handle, ALI_FP_SET_SOCKPORT, (unsigned long)(&g_ali_pan_rfk_port_ir)))
		{
			libc_printf("[ %s %d ], rfk ioctl set port fail\n", __FUNCTION__, __LINE__);	
			ret_ir |= ERR_DEV_ERROR;
		}			
	}
	
	
	memset(input_path, 0x00, sizeof(input_path));
	memset(input_name, 0x00, sizeof(input_name));		
	if (SUCCESS == pan_get_input_name(g_ali_pan_front_panel_name, input_name))
	{			
		#ifdef ADR_ALIDROID
		snprintf(input_path, sizeof(input_path), "/dev/input/%s", input_name);
		#else
		snprintf(input_path, sizeof(input_path), "/dev/%s", input_name);	
		#endif
		libc_printf("[ %s %d ], input_path = %s\n", __FUNCTION__, __LINE__, input_path);	
		
		g_ali_pan_input_handle_panel = open(input_path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);	
		if (g_ali_pan_input_handle_panel < 0)
		{			
			libc_printf("%s failed: g_ali_pan_input_handle_panel %s\n", __FUNCTION__, strerror(errno));
			ret_panel |= ERR_NO_DEV;			
		}        
	}

	g_ali_pan_rfk_port_pan = rfk_get_port();		
	if(g_ali_pan_rfk_port_pan <= 0)
	{
		libc_printf("[ %s %d ], rfk get port fail\n", __FUNCTION__, __LINE__);			
		ret_panel |= ERR_NO_DEV;
	}
	else
	{
		socket_id = rfk_get_socket(g_ali_pan_rfk_port_pan);				
		flags = fcntl (socket_id, F_GETFL, 0);
		fcntl (socket_id, F_SETFL, flags | O_NONBLOCK);					
		if (RET_SUCCESS != ioctl(g_ali_pan_panel_handle, ALI_FP_SET_SOCKPORT, (unsigned long)(&g_ali_pan_rfk_port_pan)))
		{
			libc_printf("[ %s %d ], rfk ioctl set port fail\n", __FUNCTION__, __LINE__);	
			ret_panel |= ERR_DEV_ERROR;
		}			
	}	
	

	g_ali_pan_display_backup[0] = 0;	
	memset(&g_ali_pan_key, 0x00, sizeof(g_ali_pan_key));
	memset(&g_ali_pan_key_info, 0x00, sizeof(g_ali_pan_key_info));
	memset(&g_ali_pan_dbg, 0x00, sizeof(g_ali_pan_dbg));	
	g_ali_pan_dbg.interval = 5000;
	/* Setup init work mode */
	if ((SUCCESS == ret_ir) || (SUCCESS == ret_panel))
	{
		g_ali_pan_device_status = PAN_DEV_OPEN;
	}
	else
	{
		if(g_ali_pan_input_handle_ir >= 0)
		{
			close(g_ali_pan_input_handle_ir);
			g_ali_pan_input_handle_ir = -1;
		}
		if(g_ali_pan_input_handle_panel >= 0)
		{
			close(g_ali_pan_input_handle_panel);
			g_ali_pan_input_handle_panel = -1;
		}		
		if(g_ali_pan_ir_handle >= 0)
		{
			close(g_ali_pan_ir_handle);
			g_ali_pan_ir_handle = -1;
		}
		if(g_ali_pan_panel_handle >= 0)
		{
			close(g_ali_pan_panel_handle);
			g_ali_pan_panel_handle = -1;
		}		
	}

	
	return (ret_ir || ret_panel);
}


/*
 * 	Name		:   pan_close()
 *	Description	:   Close a pan device
 *	Parameter	:	struct pan_device *dev		: Device to be closed
 *	Return		:	INT32 						: Return value
 *
 */
INT32 pan_close(struct pan_device *dev)
{
	INT32 result = SUCCESS;
	

	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d(want %d)\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status, PAN_DEV_CLOSE);
		
		return (PAN_DEV_CLOSE == g_ali_pan_device_status) ? SUCCESS : ERR_STATUS;	
	}	
	
	PAN_PRINTF("[ %s %d ]\n", __FUNCTION__, __LINE__);	
	
	if(NULL==dev){
		PAN_ERR_PRINTF("%s error:  NULL device node!\n", __FUNCTION__);
		return ERR_PARA;
	}	

	/* Stop device */
	if(g_ali_pan_input_handle_ir >= 0)
	{
		close(g_ali_pan_input_handle_ir);
		g_ali_pan_input_handle_ir = -1;
	}
	if(g_ali_pan_input_handle_panel >= 0)
	{
		close(g_ali_pan_input_handle_panel);
		g_ali_pan_input_handle_panel = -1;
	}	
	if(g_ali_pan_ir_handle >= 0)
	{
		close(g_ali_pan_ir_handle);
		g_ali_pan_ir_handle = -1;
	}
	if(g_ali_pan_panel_handle >= 0)
	{
		close(g_ali_pan_panel_handle);
		g_ali_pan_panel_handle = -1;
	}		

	if (g_ali_pan_rfk_port_ir > 0)
	{
		rfk_free_port(g_ali_pan_rfk_port_ir);
	}

	if (g_ali_pan_rfk_port_pan > 0)
	{
		rfk_free_port(g_ali_pan_rfk_port_pan);
	}
	
	/* Update flags */	
	g_ali_pan_device_status = PAN_DEV_CLOSE;
	

	return result;
}


/*
phy_code : 
		0, logic; 
		1, index.
*/
INT32 pan_config_key_map(struct pan_device *dev, UINT8 phy_code, UINT8 *map, UINT32 map_len, UINT32 unit_len)
{
	INT32 ir_rlt = SUCCESS;	
	unsigned char * map_entry;
	
	
	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return ERR_STATUS;
	}	
	
	if(NULL==dev){
		PAN_ERR_PRINTF("[ %s %d ], error:  Invalid parameters!\n", __FUNCTION__, __LINE__);
		return ERR_PARA;
	}	

	if ((NULL== map) && (2 != phy_code))
	{
		PAN_ERR_PRINTF("[ %s %d ], error:  Invalid parameters!\n", __FUNCTION__, __LINE__);
		return ERR_PARA;
	}
	
	
	IR_PRINTF("phy_code = %d, map_len = 0x%x, unit_len = 0x%x\n", 	phy_code, map_len, unit_len);

	g_ali_pan_ir_key_map.phy_code = phy_code;	
	if (2 != g_ali_pan_ir_key_map.phy_code)
	{
		map_entry = malloc(map_len+g_ali_pan_ir_key_map.map_len);
		if(NULL==map_entry){
			PAN_ERR_PRINTF("%s error: no memory!\n", __FUNCTION__);
			return ERR_NO_MEM;
		}
		/* backup key map */
		if (g_ali_pan_ir_key_map.map_entry)
		{
			memcpy(map_entry, g_ali_pan_ir_key_map.map_entry, g_ali_pan_ir_key_map.map_len);
			memcpy(map_entry+g_ali_pan_ir_key_map.map_len, map, map_len);
		}
        else
        {
			memcpy(map_entry, map, map_len);
        }
		
		free(g_ali_pan_ir_key_map.map_entry);
		g_ali_pan_ir_key_map.map_entry = NULL;
				
		g_ali_pan_ir_key_map.map_len += map_len;
		g_ali_pan_ir_key_map.unit_len = unit_len;
		g_ali_pan_ir_key_map.unit_num += (map_len/unit_len);
		g_ali_pan_ir_key_map.map_entry = map_entry;
		
		if (0 !=  g_ali_pan_dbg.ir_level)
		{
			INT32 i ;		
			
			for(i=0; i<(INT32)g_ali_pan_ir_key_map.unit_num; i++)
			{
				UINT8 * buf = &map_entry[i*g_ali_pan_ir_key_map.unit_len];
				UINT32 phy ;
				UINT16 log;
				phy = buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24);
				log = buf[4]|(buf[5]<<8);
				IR_PRINTF("%08x	%04x\n", phy, log);
			}
		}
	}	

	ir_rlt = ioctl(g_ali_pan_ir_handle, ALI_FP_CONFIG_KEY_MAP, (unsigned long)(&g_ali_pan_ir_key_map));
	if(SUCCESS!=ir_rlt)
	{
		PAN_ERR_PRINTF("[ %s %d ] fail, ir %d\n", __FUNCTION__, __LINE__, ir_rlt);
		return ERR_FAILURE;
	}
	
	
	return SUCCESS;
}


INT32 pan_del_key_map(struct pan_device *dev)
{
	INT32 ir_rlt = SUCCESS;		
	
	
	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return ERR_STATUS;
	}	
	
	if(NULL==dev){
		PAN_ERR_PRINTF("[ %s %d ], error:  Invalid parameters!\n", __FUNCTION__, __LINE__);
		return ERR_PARA;
	}	

	if(g_ali_pan_ir_key_map.map_entry)
	{
		free(g_ali_pan_ir_key_map.map_entry);
		g_ali_pan_ir_key_map.map_entry = NULL;
		memset(&g_ali_pan_ir_key_map, 0, sizeof(g_ali_pan_ir_key_map));
	}

	return ir_rlt;
}


/*
phy_code : 0, logic; 1, index.
*/
INT32 pan_config_panel_map(struct pan_device *dev, UINT8 phy_code, UINT8 *map, UINT32 map_len, UINT32 unit_len)
{	
	INT32 pan_rlt = SUCCESS;
	unsigned char * map_entry;
	
	
	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return ERR_STATUS;
	}	
	
	if(NULL==dev){
		PAN_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);
		return ERR_PARA;
	}	

	if ((NULL== map) && (2 != phy_code))
	{
		PAN_ERR_PRINTF("[ %s %d ], error:  Invalid parameters!\n", __FUNCTION__, __LINE__);
		return ERR_PARA;
	}
	
	PANEL_PRINTF("phy_code = %s, map_len = 0x%x, unit_len = 0x%x\n", 
		(0 == phy_code) ? "logic" : "index", map_len, unit_len);	

	g_ali_pan_panel_key_map.phy_code = phy_code;
	if (2 != g_ali_pan_panel_key_map.phy_code)
	{
		map_entry = malloc(map_len);
		if(NULL==map_entry){
			PAN_ERR_PRINTF("%s error: no memory!\n", __FUNCTION__);
			return ERR_NO_MEM;
		}
		memcpy(map_entry, map, map_len);
		if(g_ali_pan_panel_key_map.map_entry)
		{
			free(g_ali_pan_panel_key_map.map_entry);
		}
		g_ali_pan_panel_key_map.map_entry = NULL;
		g_ali_pan_panel_key_map.map_len = map_len;
		g_ali_pan_panel_key_map.unit_len = unit_len;
		g_ali_pan_panel_key_map.unit_num = (map_len/unit_len);	
		g_ali_pan_panel_key_map.map_entry = map_entry;
		if (0 !=  g_ali_pan_dbg.panel_level)
		{
			INT32 i ;		
			
			for(i=0; i<(INT32)g_ali_pan_panel_key_map.unit_num; i++)
			{
				UINT8 * buf = &map_entry[i*g_ali_pan_panel_key_map.unit_len];
				UINT32 phy ;
				UINT16 log;
				phy = buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24);
				log = buf[4]|(buf[5]<<8);
				PANEL_PRINTF("%08x	%04x\n", phy, log);
			}
		}
	}
	
	pan_rlt = ioctl(g_ali_pan_panel_handle, ALI_FP_CONFIG_KEY_MAP, (unsigned long)(&g_ali_pan_panel_key_map));
	if(SUCCESS!=pan_rlt)
	{
		PAN_ERR_PRINTF("[ %s %d ] fail, pan %d\n", __FUNCTION__, __LINE__, pan_rlt);
		return ERR_FAILURE;
	}
	
	
	return SUCCESS;
}


struct pan_key_info *  pan_get_key_info(struct pan_device *dev, UINT32 timeout)
{	
	UINT8 *msg_buf = NULL;	
	INT32 size = sizeof(g_ali_pan_key_info);
	

	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return NULL;
	}	
	
	if (NULL== dev)
	{					
		PAN_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return NULL;
	}	
	
	
	while(1)
	{				
		msg_buf = (UINT8 *)rfk_receive_msg(g_ali_pan_rfk_port_ir);		
		if(NULL != msg_buf)
		{
			g_ali_pan_key_info.type = PAN_KEY_TYPE_REMOTE;
			goto got_key_info;
		}

		msg_buf = (UINT8 *)rfk_receive_msg(g_ali_pan_rfk_port_pan);		
		if(NULL != msg_buf)
		{
			g_ali_pan_key_info.type = PAN_KEY_TYPE_PANEL;
			goto got_key_info;
		}
		
		if (timeout != OSAL_WAIT_FOREVER_TIME)
		{
			if (timeout-- == 0)
			{
				break;
			}
		}
		
		osal_task_sleep(1);
	}
	
	return NULL;	
	

	got_key_info:		
	{						
		g_ali_pan_key_info.code_high = (msg_buf[0] << 24) |(msg_buf[1] << 16) | (msg_buf[2] << 8) | (msg_buf[3]);
		g_ali_pan_key_info.code_low =  (msg_buf[4] << 24) |(msg_buf[5] << 16) | (msg_buf[6] << 8) | (msg_buf[7]);
		g_ali_pan_key_info.protocol = (pan_ir_protocol_e)((msg_buf[8] << 24) |(msg_buf[9] << 16) 
								| (msg_buf[10] << 8) | (msg_buf[11]));
		g_ali_pan_key_info.state = (pan_key_press_e)((msg_buf[12] << 24) |(msg_buf[13] << 16) 
								| (msg_buf[14] << 8) | (msg_buf[15]));		
						
		PAN_KEY_PRINTF(g_ali_pan_key_info.type, "[ %s %d ], code: 0x%08x 0x%08x, protocol: %d, state: %d, type: %s\n\n", __FUNCTION__, __LINE__,
					g_ali_pan_key_info.code_high, g_ali_pan_key_info.code_low, g_ali_pan_key_info.protocol, 
					g_ali_pan_key_info.state, (PAN_KEY_TYPE_PANEL== g_ali_pan_key_info.type) ? "Panel" : "IR");							
			
		
		return &g_ali_pan_key_info;	
	}		
}


struct pan_key * auto_change_channel_check(void)
{		
	#ifdef AUTO_CHANGE_CHANNEL
		if (0x60df7887 == g_ali_pan_key.code)		/* green key, start */
		{
			g_ali_pan_dbg.auto_change = 1;		
		}
		else if (0x60df609f == g_ali_pan_key.code)	/* red key, stop */
		{
			g_ali_pan_dbg.auto_change = 0;
		}
	#endif

	if (1 == g_ali_pan_dbg.auto_change)
	{
		g_ali_pan_key.code = 0x60dfb847;		/* down key */
		g_ali_pan_key.state = PAN_KEY_PRESSED;
		g_ali_pan_key.count = 1;
		g_ali_pan_key.type = PAN_KEY_TYPE_REMOTE;	

		osal_task_sleep(g_ali_pan_dbg.interval);

		IR_PRINTF("[ %s %d ], auto change channel, %s, code: 0x%08x, state: %s\n", __FUNCTION__, __LINE__,
				(PAN_KEY_TYPE_PANEL== g_ali_pan_key.type) ? "Panel" : "IR",	
				g_ali_pan_key.code, 
				(PAN_KEY_PRESSED == g_ali_pan_key.state) ? "pressed" : "release");			

		return &g_ali_pan_key;
	}	

	return NULL;
}

struct pan_key * pan_get_key(struct pan_device *dev, UINT32 timeout)
{
	struct input_event key_event;		
	INT32 read_len = sizeof(struct input_event);	


	
	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return NULL;
	}

	if(NULL==dev)
	{						
		PAN_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return NULL;
	}
	
	
	if ((NULL == g_ali_pan_ir_key_map.map_entry) && (NULL == g_ali_pan_panel_key_map.map_entry))
	{							
		PAN_PRINTF("[ %s %d ], pan device not config key mapping yet!\n", __FUNCTION__, __LINE__);
		return NULL;				
	}		
	
	while(1)
	{				
		memset(&key_event, 0x00, sizeof(struct input_event));		
		
		if ((NULL != g_ali_pan_panel_key_map.map_entry) && (read_len == read(g_ali_pan_input_handle_panel, &key_event, read_len)) 
			&& (EV_SYN != key_event.type) && (EV_REP != key_event.type))			
		{						
			PANEL_PRINTF("[ %s %d ], Panel, type: %d, code: %d, value: %d, panel.phy_code: %d\n", 
				__FUNCTION__, __LINE__, key_event.type, key_event.code, key_event.value, g_ali_pan_panel_key_map.phy_code);						
			
			g_ali_pan_key.type = PAN_KEY_TYPE_PANEL;
			goto got_key;
		}		
		
		if ((NULL != g_ali_pan_ir_key_map.map_entry) && (read_len == read(g_ali_pan_input_handle_ir, &key_event, read_len))	
			&& (EV_SYN != key_event.type) && (EV_REP != key_event.type))
		{								
			IR_PRINTF("[ %s %d ], IR, type: %d, code: %d, value: %d, ir.phy_code: %d\n",
				__FUNCTION__, __LINE__, key_event.type, key_event.code, key_event.value, g_ali_pan_ir_key_map.phy_code);							

			g_ali_pan_key.type = PAN_KEY_TYPE_REMOTE;
			goto got_key;
		}					
		
		
		if (timeout != OSAL_WAIT_FOREVER_TIME)
		{
			if (timeout-- == 0)
			{
				break;
			}
		}
		
		osal_task_sleep(1);
	}
	
	return auto_change_channel_check();
	

got_key:		
	if((1 == g_ali_pan_ir_key_map.phy_code) && (PAN_KEY_TYPE_REMOTE == g_ali_pan_key.type))
	{		
		/* ir index, phy key */
		if (g_ali_pan_ir_key_map.map_entry)
		{
			UINT8 buf[4];
			memcpy(buf, &g_ali_pan_ir_key_map.map_entry[key_event.code*g_ali_pan_ir_key_map.unit_len],
				sizeof(buf));			
			g_ali_pan_key.code = buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24); 
		}
	}
	else if((1 == g_ali_pan_panel_key_map.phy_code) && (PAN_KEY_TYPE_PANEL== g_ali_pan_key.type))
	{		
		/* panel index, phy key */
		if (g_ali_pan_panel_key_map.map_entry)
		{
			UINT8 buf[4];
			memcpy(buf, &g_ali_pan_panel_key_map.map_entry[key_event.code*g_ali_pan_panel_key_map.unit_len],
				sizeof(buf));			
			g_ali_pan_key.code = buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24); 
			g_ali_pan_key.code |= 0xFFFF0000;
		}
	}	
	else if((0 == g_ali_pan_ir_key_map.phy_code) || (0 == g_ali_pan_panel_key_map.phy_code))
	{		
		/* logic key */
		g_ali_pan_key.code = key_event.code;		
	}
	else
	{
		PAN_ERR_PRINTF("[ %s %d ], device config key mapping error!\n", __FUNCTION__);
	}
	
	
	if(0==key_event.value)
	{
		g_ali_pan_key.state = PAN_KEY_RELEASE;
		g_ali_pan_key.count = 0;
	}
	else if (1==key_event.value)
	{
		g_ali_pan_key.state = PAN_KEY_PRESSED;
		g_ali_pan_key.count ++;
	}	
	else
	{
		/* enable repeat key? */
		if (1 == g_ali_pan_repeat_enable)
		{
			g_ali_pan_key.state = PAN_KEY_PRESSED;
			g_ali_pan_key.count ++;
		}
		else
		{
			return NULL;
		}
	}		
					
	PAN_KEY_PRINTF(g_ali_pan_key.type, "[ %s %d ], %s, type: %d, code: 0x%08x, state: %s\n", __FUNCTION__, __LINE__,
				(PAN_KEY_TYPE_PANEL== g_ali_pan_key.type) ? "Panel" : "IR",	
				key_event.type, g_ali_pan_key.code, 
				(PAN_KEY_PRESSED == g_ali_pan_key.state) ? "pressed" : "release");	
	
	auto_change_channel_check();
	
	return &g_ali_pan_key;		
}


struct pan_key_index * pan_get_key_index(struct pan_device *dev, UINT32 timeout)
{
	struct input_event key_event;	
	UINT8 got_pan_key = 0;
	INT32 read_len = sizeof(struct input_event);	
	

	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return NULL;
	}

	if(NULL== dev)
	{						
		PAN_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);		
		return NULL;
	}	
	
	
	if(NULL==g_ali_pan_ir_key_map.map_entry)
	{		
		PAN_ERR_PRINTF("%s error: device not config key mapping yet!\n", __FUNCTION__);		
		return NULL;
	}
	while(1)
	{				
		memset(&key_event, 0x00, sizeof(struct input_event));
		got_pan_key = 0;
		
		if ((read_len == read(g_ali_pan_input_handle_panel, &key_event, read_len)) 
			&& (EV_SYN != key_event.type) && (EV_REP != key_event.type))			
		{					
			PANEL_PRINTF("[ %s %d ], Panel, type: %d, code: %d, value: %d\n", __FUNCTION__, __LINE__,
				key_event.type, key_event.code, key_event.value);			
			
			got_pan_key = 1;
			goto got_key;
		}		
		
		if ((read_len == read(g_ali_pan_input_handle_ir, &key_event, read_len))	
			&& (EV_SYN != key_event.type) && (EV_REP != key_event.type))
		{				
			PAN_PRINTF("[ %s %d ], IR, type: %d, code: %d, value: %d\n", __FUNCTION__, __LINE__,
				key_event.type, key_event.code, key_event.value);								
			
			goto got_key;
		}			
		
		if (timeout != OSAL_WAIT_FOREVER_TIME)
		{
			if (timeout-- == 0)
			{
				break;
			}
		}
		
		osal_task_sleep(1);
	}
	
	return NULL;
	

got_key:
	if((1 == g_ali_pan_ir_key_map.phy_code) || (1 == got_pan_key))
	{
		/* phy key */
		UINT8 buf[4];
		memcpy(buf, &g_ali_pan_ir_key_map.map_entry[key_event.code*g_ali_pan_ir_key_map.unit_len],
			sizeof(buf));			
		g_ali_pan_key_index.key_value.code= buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24); 
		/* index */
		g_ali_pan_key_index.key_index = key_event.code;
	}
	else
	{
		/* logic key */
		g_ali_pan_key_index.key_value.code = key_event.code;
		g_ali_pan_key_index.key_index = key_event.code;
	}
	
	if(0==key_event.value)
	{
		g_ali_pan_key_index.key_value.state = PAN_KEY_RELEASE;
		g_ali_pan_key_index.key_value.count = 0;
	}
	else if (1==key_event.value)
	{
		g_ali_pan_key_index.key_value.state = PAN_KEY_PRESSED;
		g_ali_pan_key_index.key_value.count ++;
	}
	else
	{
		/* enable repeat key? */
		if (1 == g_ali_pan_repeat_enable)
		{
			g_ali_pan_key_index.key_value.state = PAN_KEY_PRESSED;
			g_ali_pan_key_index.key_value.count ++;
		}
		else
		{
			return NULL;
		}
	}	
	
	PAN_PRINTF("[ %s %d ], %s, type: %d, code: 0x%08x, index : 0x%04x, state: %s, count: %d\n", 
				__FUNCTION__, __LINE__,
				(1 == got_pan_key) ? "Panel" : "IR", 
				key_event.type, g_ali_pan_key_index.key_value.code, g_ali_pan_key_index.key_index,
				(PAN_KEY_PRESSED == g_ali_pan_key_index.key_value.state) ? "pressed" : "release",
				g_ali_pan_key_index.key_value.count);			

	
	return &g_ali_pan_key_index;		
}


INT32 pan_send_data(struct pan_device *dev, UINT8 *data, UINT32 len, UINT32 timeout)
{
	struct ali_fp_data_transfer_param tx_param;
	INT32 rlt = SUCCESS;

	
	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return ERR_STATUS;
	}
	
	if(NULL==dev||NULL==data){
		PAN_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);
		return ERR_PARA;
	}
	
	memset(&tx_param, 0, sizeof(tx_param));
	tx_param.tx_buf = data;
	tx_param.tx_len = len;
	tx_param.tmo = timeout;
	rlt = ioctl(g_ali_pan_panel_handle, ALI_FP_DATA_TRANSFER, (unsigned long)(&tx_param));
	if(SUCCESS==rlt){
		while(timeout--){
			rlt = ioctl(g_ali_pan_panel_handle, ALI_FP_TRANSFER_CHECK, 1);
			if(SUCCESS==rlt)
				break;
			osal_task_sleep(1);
		}
	}
	return rlt;
}


INT32 pan_receive_data(struct pan_device *dev, UINT8 *data, UINT32 len, UINT32 timeout)
{
	struct ali_fp_data_transfer_param rx_param;
	INT32 rlt = SUCCESS;

	
	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return ERR_STATUS;
	}
	
	if(NULL==dev||NULL==data){
		PAN_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);
		return ERR_PARA;
	}
	
	memset(&rx_param, 0, sizeof(rx_param));
	rx_param.rx_buf = data;
	rx_param.rx_len = len;
	rx_param.tmo = timeout;
	rlt = ioctl(g_ali_pan_panel_handle, ALI_FP_DATA_TRANSFER, (unsigned long)(&rx_param));
	if(SUCCESS==rlt)
	{
		while(timeout--)
		{
			rlt = ioctl(g_ali_pan_panel_handle, ALI_FP_TRANSFER_CHECK, 0);
			if(SUCCESS==rlt)
			{
				rlt = rx_param.rx_len;
				break;
			}
			osal_task_sleep(1);
		}
	}
	
	return rlt;
}


INT32 pan_buff_clear(struct pan_device *dev)
{
	struct input_event key_event;
	UINT32 max_cnt;
	INT32 read_len = sizeof(struct input_event);

	
	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return ERR_STATUS;
	}	
	
	if(NULL== dev){
		PAN_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);
		return ERR_PARA;
	}
	
	max_cnt = 1000;
	while(max_cnt--)
	{		
		if (read_len != read(g_ali_pan_input_handle_panel, &key_event, read_len))
		{
			break;
		}		
	}
	max_cnt = 1000;
	while(max_cnt--)
	{		
		if (read_len != read(g_ali_pan_input_handle_ir, &key_event, read_len))
		{
			break;
		}		
	}
	
	return 0;
}


void pan_buff_set_repeat(struct pan_device *dev, UINT8 enable_repeat)
{
	g_ali_pan_repeat_enable = enable_repeat;
}


UINT8 pan_buff_get_repeat(struct pan_device *dev)
{
	return g_ali_pan_repeat_enable;
}


INT32 pan_config_ir_rep_interval(struct pan_device *dev, UINT32 delay, UINT32 interval)
{
	UINT32 rep[2] = {0, 0};
	INT32 result = SUCCESS;	
	

	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return ERR_STATUS;
	}	

	if(NULL==dev){
		PAN_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);
		return ERR_PARA;
	}
	
	rep[0] = delay;	
	rep[1] = interval;	
	
	IR_PRINTF("%s %d, rep[0] = %ld, rep[1] = %ld\n", __FUNCTION__, __LINE__, rep[0], rep[1]);	
	
	result |= ioctl(g_ali_pan_input_handle_ir, EVIOCSREP, rep);	
	result |= ioctl(g_ali_pan_ir_handle, ALI_FP_SET_REPEAT_INTERVAL, rep);	
	
	
	return result;
}


INT32 pan_config_pan_rep_interval(struct pan_device *dev, UINT32 delay, UINT32 interval)
{
	UINT32 rep[2] = {0, 0};
	INT32 result = SUCCESS;	


	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return ERR_STATUS;
	}	

	if (NULL == dev)
	{
		PAN_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);
		return ERR_PARA;
	}
	
	rep[0] = delay;	
	rep[1] = interval;
	
	PAN_PRINTF("%s %d, rep[0] = %ld, rep[1] = %ld\n", __FUNCTION__, __LINE__, rep[0], rep[1]);	
	
	result |= ioctl(g_ali_pan_input_handle_panel, EVIOCSREP, rep);	
	result |= ioctl(g_ali_pan_panel_handle, ALI_FP_SET_REPEAT_INTERVAL, rep);	
	
	
	return result;
}


INT32 pan_display(struct pan_device *dev, char *data, UINT32 len)
{
	UINT32 backup_len = 0;

	
	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		libc_printf("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return ERR_STATUS;
	}
	
	if (NULL == dev)
	{
		libc_printf("%s error:  Invalid parameters!\n", __FUNCTION__);
		return ERR_PARA;
	}	
	
	if(len>sizeof(g_ali_pan_display_backup))
	{
		memcpy(g_ali_pan_display_backup, data, sizeof(g_ali_pan_display_backup));
		g_ali_pan_display_backup[sizeof(g_ali_pan_display_backup) - 1] = 0;
	}
	else
	{
		strcpy(g_ali_pan_display_backup, data);
	}
	
	
	return write(g_ali_pan_panel_handle, data, len);
}


INT32 pan_get_display(struct pan_device *dev, char *data, UINT32 * len)
{
	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return ERR_STATUS;
	}	

	if(NULL==dev){
		PAN_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);
		return ERR_PARA;
	}
	
	
	strcpy(data, g_ali_pan_display_backup);	
	
	return SUCCESS;
}


INT32 pan_config_ir_format(struct pan_device *dev, UINT32 format)
{
	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return ERR_STATUS;
	}	
	
	if(NULL==dev){
		PAN_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);
		return ERR_PARA;
	}	

	g_ali_pan_ir_format = format;	
	IR_PRINTF("%s, g_ali_pan_ir_format = 0x%08x\n", __FUNCTION__, g_ali_pan_ir_format);	
	
	return SUCCESS;
}


INT32 pan_io_control(struct pan_device *dev, INT32 cmd, UINT32 param)
{
	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return ERR_STATUS;
	}	
	
	if(NULL== dev){
		PAN_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);
		return ERR_PARA;
	}	
	
	return ioctl(g_ali_pan_panel_handle, cmd, param);
}


INT32 pan_ir_io_control(struct pan_device *dev, INT32 cmd, UINT32 param)
{
	if (PAN_DEV_OPEN != g_ali_pan_device_status)
	{		
		PAN_PRINTF("[ %s %d ], status error, now = %d\n", __FUNCTION__, __LINE__, 
			g_ali_pan_device_status);		

		return ERR_STATUS;
	}

	if(NULL== dev){
		PAN_ERR_PRINTF("%s error:  Invalid parameters!\n", __FUNCTION__);
		return ERR_PARA;
	}	
	
	return ioctl(g_ali_pan_ir_handle, cmd, param);
}



