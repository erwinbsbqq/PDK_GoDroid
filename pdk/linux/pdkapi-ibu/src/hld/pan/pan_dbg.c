/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    adr_pan_dbg.c
*
*    Description:    This file contains all diagnostic functions definition
*		             of Front panel driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Oct.15.2012      John Chen       Ver 0.1    Create file.
*****************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <linux/input.h>
#include <osal/osal_task.h>
#include <basic_types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <hld/pan/pan_dev.h>
#include <hld/dbg/dbg_parser.h>
#include <hld/pan/pan.h>
#include <hal/hal_gpio.h>
#include <ali_front_panel_common.h>

#include "pan_dbg.h"

#define PAN_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(fmt, ##args); \
			} while(0)


#define PAN_DBG_INFO			PAN_DBG_PRINT
#define PAN_DBG_ERR				PAN_DBG_PRINT

#define PAN_DBG_TASK_PRIORITY		OSAL_PRI_NORMAL
#define PAN_DBG_TASK_STACKSIZE		0x1000
#define PAN_DBG_TASK_QUANTUM		10

#define OPT_IR_STATE							0x00000001
#define OPT_IR_FORMAT_AVAILABLE			0x00000002
#define OPT_IR_FORMAT_CONFIGED				0x00000004
#define OPT_IR_INT_RECEIVED					0x00000008
#define OPT_IR_KEY_RECEIVED					0x00000010
#define OPT_IR_KEY_MAPPED					0x00000020
#define OPT_PANEL_STATE						0x00000001
#define OPT_PANEL_KEY_RECEIVED			0x00000002
#define OPT_PANEL_KEY_MAPPED				0x00000004
#define IR_OPT_ALL	(OPT_IR_STATE | OPT_IR_FORMAT_AVAILABLE | OPT_IR_FORMAT_CONFIGED 	\
					| OPT_IR_INT_RECEIVED | OPT_IR_KEY_RECEIVED | OPT_IR_KEY_MAPPED)
#define PANEL_OPT_ALL	(OPT_PANEL_STATE |OPT_PANEL_KEY_RECEIVED | OPT_PANEL_KEY_MAPPED)	

static const char *g_ali_pan_ir_format_name[IR_FORMAT_MAX] = 
{
	"nec",
	"lab",
	"50560",
	"kf",
	"logic",
	"src",
	"nse",
	"rc5",
	/*"rc6",*/
};

struct pan_dbg_ctrl
{
	OSAL_ID  taskid;
	BOOL flag;
	BOOL exit;
	UINT32 enable;		/* 0 */
	UINT32 option;		
	UINT32 interval;		/* 5000 */	
};

static struct pan_dbg_ctrl g_ali_pan_ir_dbg_ctrl;
static struct pan_dbg_ctrl g_ali_pan_panel_dbg_ctrl;
static struct pan_hw_info g_ali_pan_hw =
{
	0,				/* type_kb : 2; Key board (array) type */
	1,				/* type_scan : 1; 0: Slot scan, 1: Shadow scan */
	1,				/* type_key: 1; Key exit or not */
	1,				/* type_irp: 3; 0: not IRP, 1: NEC, 2: LAB */
	0,				/* type_mcu: 1; MCU exit or not */
	4,				/* num_com: 4; Number of com PIN, 0 to 8 */   
	1,				/* Position of colon flag, 0 to 7 */
	1,				/* num_scan: 2; Number of scan PIN, 0 to 2 */
	0,				/* rsvd_bits:6; Reserved bits */
	0,              			/* rsvd byte for align pan_info */
	{0, HAL_GPIO_O_DIR,     GPIO_NOT_USED},		/* LATCH PIN */
	{0, HAL_GPIO_O_DIR, 	13},					/* CLOCK PIN */
	{1, HAL_GPIO_O_DIR, 	11},					/* DATA PIN */
	{{0, HAL_GPIO_I_DIR, 	15},					/* SCAN1 PIN */	
	{0, HAL_GPIO_I_DIR,       GPIO_NOT_USED}},	/* SCAN2 PIN */
	{{0, HAL_GPIO_O_DIR, 	7},					/* COM1 PIN */
	{0, HAL_GPIO_O_DIR, 	8},					/* COM2 PIN */
	{0, HAL_GPIO_O_DIR, 	9},					/* COM3 PIN */
	{0, HAL_GPIO_O_DIR, 	10},					/* COM4 PIN */
	{0, HAL_GPIO_O_DIR, 	GPIO_NOT_USED},		/* COM5 PIN */
	{0, HAL_GPIO_O_DIR, 	GPIO_NOT_USED},		/* COM6 PIN */
	{0, HAL_GPIO_O_DIR, 	GPIO_NOT_USED},		/* COM7 PIN */
	{0, HAL_GPIO_O_DIR, 	GPIO_NOT_USED}},	/* COM8 PIN */
	{{1, HAL_GPIO_O_DIR, 	18},					/* POWER PIN, 0: Standby, 1: Work */
	{0, HAL_GPIO_O_DIR, 	14},					/* LOCK PIN */
	{0, HAL_GPIO_O_DIR, 	GPIO_NOT_USED},		/* Extend function LBD */
	{0, HAL_GPIO_O_DIR, 	GPIO_NOT_USED}},	/* Extend function LBD */
	{0, HAL_GPIO_O_DIR, 	GPIO_NOT_USED},        	/* rsvd extend function LBD */      
	300,										/* Intv repeat first */
	250,										/* Intv repeat */
	0,										/* Intv release, 0: disable release key */	
};


static void ir_dbg_task()
{
	UINT8 i = 0;
	INT32 ret = ERR_FAILURE;
	UINT32 count = 0;
	struct pan_dbg *pan_dbg_info;
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);

	
	osal_task_save_thread_info("IR_DBG");	
	while(g_ali_pan_ir_dbg_ctrl.flag)
	{		
		g_ali_pan_ir_dbg_ctrl.exit = FALSE;
		if (0 == g_ali_pan_ir_dbg_ctrl.enable)
		{			
			osal_task_sleep(3 * 1000);
			continue;
		}		

		pan_dbg_info = (struct pan_dbg *)pan_dbg_get_info(pan_dev);			

		PAN_DBG_INFO("\n*******************************************************\n");	
		
		if (0 != (g_ali_pan_ir_dbg_ctrl.option& OPT_IR_STATE))
		{	
			if (PAN_DEV_ATTACH == pan_dbg_info->stats)
			{
				PAN_DBG_INFO("IR state \t\t\t : %s\n", "Attach");	
			}
			else if (PAN_DEV_OPEN == pan_dbg_info->stats)
			{
				PAN_DBG_INFO("IR state \t\t\t : %s\n", "Open");	
			}
			else if (PAN_DEV_CLOSE == pan_dbg_info->stats)
			{
				PAN_DBG_INFO("IR state \t\t\t : %s\n", "Close");	
			}
			else if (PAN_DEV_DETACH == pan_dbg_info->stats)
			{
				PAN_DBG_INFO("IR state \t\t\t : %s\n", "Detach");	
			}
			else
			{
				PAN_DBG_INFO("IR state \t\t\t : %s\n", "Error");
			}
		}			

		if (0 != (g_ali_pan_ir_dbg_ctrl.option & OPT_IR_FORMAT_AVAILABLE))
		{
			PAN_DBG_INFO("all support IR formats \t\t : ");
			for (i=0; i<IR_FORMAT_MAX; i++)
			{									
				PAN_DBG_INFO("%s ", g_ali_pan_ir_format_name[i]);					
			}		
			PAN_DBG_INFO("\n");	
		}			

		if (0 != (g_ali_pan_ir_dbg_ctrl.option & OPT_IR_FORMAT_CONFIGED))
		{
			PAN_DBG_INFO("support IR format now \t\t : ");
			for (i=0; i<IR_FORMAT_MAX; i++)
			{
				if (0x1 == ((pan_dbg_info->ir_format>> i) & 0x00000001))		
				{
					PAN_DBG_INFO("%s ", g_ali_pan_ir_format_name[i]);			
				}
			}	
			PAN_DBG_INFO("\n");	
		}		

		if (0 != (g_ali_pan_ir_dbg_ctrl.option & OPT_IR_INT_RECEIVED))
		{
			count = 0;
			ret = ioctl(pan_dbg_info->ir_handle, ALI_FP_GET_IR_INT_COUNT, (unsigned long)(&count));
			if (SUCCESS == ret)
			{
				PAN_DBG_INFO("IR interrupt signals \t\t : %ld\n", count);		
			}
			else
			{
				PAN_DBG_ERR("[ %s %d ] error:  ioctl fail!\n", __FUNCTION__, __LINE__);	
				perror("");
			}
		}				
		
		if (0 != (g_ali_pan_ir_dbg_ctrl.option & OPT_IR_KEY_RECEIVED))
		{
			count = 0;
			ret = ioctl(pan_dbg_info->ir_handle, ALI_FP_GET_IR_KEY_RECEIVED_COUNT, (unsigned long)(&count));
			if (SUCCESS == ret)
			{
				PAN_DBG_INFO("IR received keys \t\t : %ld\n", count);		
			}
			else
			{
				PAN_DBG_ERR("[ %s %d ] error:  ioctl fail!\n", __FUNCTION__, __LINE__);	
				perror("");
			}
		}				

		if (0 != (g_ali_pan_ir_dbg_ctrl.option & OPT_IR_KEY_MAPPED))
		{
			count = 0;
			ret = ioctl(pan_dbg_info->ir_handle, ALI_FP_GET_IR_KEY_MAPPED_COUNT, (unsigned long)(&count));
			if (SUCCESS == ret)
			{
				PAN_DBG_INFO("IR mapped keys \t\t\t : %ld\n", count);		
			}
			else
			{
				PAN_DBG_ERR("[ %s %d ] error:  ioctl fail!\n", __FUNCTION__, __LINE__);	
				perror("");
			}
		}					
										
		if (!g_ali_pan_ir_dbg_ctrl.flag)
		{
			break;
		}
		PAN_DBG_INFO("*************************** delay %ldms ****************************\n\n", g_ali_pan_ir_dbg_ctrl.interval);
		osal_task_sleep(g_ali_pan_ir_dbg_ctrl.interval);
	}
	
	g_ali_pan_ir_dbg_ctrl.exit = TRUE;
}


static void panel_dbg_task()
{
	UINT8 i = 0;
	INT32 ret = ERR_FAILURE;
	UINT32 count = 0;
	struct pan_dbg *pan_dbg_info;
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);

	
	osal_task_save_thread_info("PAN_DBG");	
	while(1 == g_ali_pan_panel_dbg_ctrl.flag)
	{		
		g_ali_pan_panel_dbg_ctrl.exit = FALSE;
		if (0 == g_ali_pan_panel_dbg_ctrl.enable)
		{					
			osal_task_sleep(3 * 1000);
			continue;
		}		
	
		pan_dbg_info = (struct pan_dbg *)pan_dbg_get_info(pan_dev);	

		PAN_DBG_INFO("\n*******************************************************\n");					
		
		if (0 != (g_ali_pan_panel_dbg_ctrl.option & OPT_PANEL_STATE))
		{	
			if (PAN_DEV_ATTACH == pan_dbg_info->stats)
			{
				PAN_DBG_INFO("panel state \t\t\t : %s\n", "Attach");	
			}
			else if (PAN_DEV_OPEN == pan_dbg_info->stats)
			{
				PAN_DBG_INFO("panel state \t\t\t : %s\n", "Open");	
			}
			else if (PAN_DEV_CLOSE == pan_dbg_info->stats)
			{
				PAN_DBG_INFO("panel state \t\t\t : %s\n", "Close");	
			}
			else if (PAN_DEV_DETACH == pan_dbg_info->stats)
			{
				PAN_DBG_INFO("panel state \t\t\t : %s\n", "Detach");	
			}
			else
			{
				PAN_DBG_INFO("panel state \t\t\t : %s\n", "Error");
			}									
		}				

		if (0 != (g_ali_pan_panel_dbg_ctrl.option & OPT_PANEL_KEY_RECEIVED))
		{		
			count = 0;
			ret = ioctl(pan_dbg_info->panel_handle, ALI_FP_GET_PAN_KEY_RECEIVED_COUNT, (unsigned long)(&count));
			if (SUCCESS == ret)
			{
				PAN_DBG_INFO("panel received keys \t\t : %ld\n", count);		
			}
			else
			{
				PAN_DBG_ERR("[ %s %d ] error:  ioctl fail!\n", __FUNCTION__, __LINE__);	
				perror("");	
			}
		}			

		if (0 != (g_ali_pan_panel_dbg_ctrl.option & OPT_PANEL_KEY_MAPPED))
		{	
			count = 0;
			ret = ioctl(pan_dbg_info->panel_handle, ALI_FP_GET_PAN_KEY_MAPPED_COUNT, (unsigned long)(&count));
			if (SUCCESS == ret)
			{
				PAN_DBG_INFO("panel mapped keys \t\t : %ld\n", count);		
			}
			else
			{
				PAN_DBG_ERR("[ %s %d ] error:  ioctl fail!\n", __FUNCTION__, __LINE__);	
				perror("");
			}
		}	
		
		PAN_DBG_INFO("*************************** delay %ldms ****************************\n\n", g_ali_pan_panel_dbg_ctrl.interval);
		osal_task_sleep(g_ali_pan_panel_dbg_ctrl.interval);
	}
		
	g_ali_pan_panel_dbg_ctrl.exit = TRUE;
}


static INT32 ir_dbg_task_init(void)
{
	OSAL_T_CTSK t_ctsk;		
	memset(&t_ctsk, 0, sizeof(OSAL_T_CTSK));
	t_ctsk.stksz = PAN_DBG_TASK_STACKSIZE;
	t_ctsk.quantum =PAN_DBG_TASK_QUANTUM;
	t_ctsk.itskpri = PAN_DBG_TASK_PRIORITY;
	t_ctsk.name[0] = 'I';
	t_ctsk.name[1] = 'R';
	t_ctsk.name[2] = 'D';
	t_ctsk.task = (FP)ir_dbg_task;
	g_ali_pan_ir_dbg_ctrl.taskid = osal_task_create(&t_ctsk);
	if (g_ali_pan_ir_dbg_ctrl.taskid == OSAL_INVALID_ID)
	{
		return ERR_FAILURE;
	}
	
	return SUCCESS;
}


static INT32 panel_dbg_task_init(void)
{
	OSAL_T_CTSK t_ctsk;	
	memset(&t_ctsk, 0, sizeof(OSAL_T_CTSK));
	t_ctsk.stksz = PAN_DBG_TASK_STACKSIZE;
	t_ctsk.quantum = PAN_DBG_TASK_QUANTUM;
	t_ctsk.itskpri = PAN_DBG_TASK_PRIORITY;
	t_ctsk.name[0] = 'P';
	t_ctsk.name[1] = 'A';
	t_ctsk.name[2] = 'D';
	t_ctsk.task = (FP)panel_dbg_task;
	g_ali_pan_panel_dbg_ctrl.taskid = osal_task_create(&t_ctsk);
	if (g_ali_pan_panel_dbg_ctrl.taskid == OSAL_INVALID_ID)
	{
		return ERR_FAILURE;
	}
	
	return SUCCESS;
}


static INT32 pan_dag_on_off(struct pan_device *dev,UINT8 ir_level, UINT8 panel_level)
{	
	INT32 ret = ERR_FAILURE;
	struct pan_dbg *pan_dbg_info = NULL;
	UINT32 max_level = PAN_DBG_LEVEL_HLD | PAN_DBG_LEVEL_KERNEL;	

	
	if(NULL == dev)
	{
		PAN_DBG_ERR("[ %s ] error:  dev is NULL!\n", __FUNCTION__);
		return ERR_FAILURE;
	}	

	if ((ir_level > max_level) || (panel_level > max_level))
	{
		PAN_DBG_ERR("[ %s ] error : Invalid parameters!, ir_level = %d, panel_level = %d, max_level = %d\n", 
			__FUNCTION__, ir_level, panel_level, max_level);

		return ERR_FAILURE;
	}	

	pan_dbg_info = (struct pan_dbg *)pan_dbg_get_info(dev);	

	pan_dbg_info->ir_level = (pan_dbg_level_e)ir_level;
	pan_dbg_info->panel_level = (pan_dbg_level_e)panel_level;		
	
	ir_level &= PAN_DBG_LEVEL_KERNEL;		
	ret = ioctl (pan_dbg_info->ir_handle, ALI_FP_SET_IR_KEY_DEBUG, &ir_level);
	if (SUCCESS != ret)
	{
		PAN_DBG_ERR("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		perror("");	
	}	
	
	panel_level &= PAN_DBG_LEVEL_KERNEL;
	ret = ioctl (pan_dbg_info->panel_handle, ALI_FP_SET_PAN_KEY_DEBUG, &panel_level);
	if (SUCCESS != ret)
	{
		PAN_DBG_ERR("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		perror("");	
	}	
	
	pan_dbg_set_info(dev, pan_dbg_info);	
	
	
	return SUCCESS;
}


static INT32 pan_dag_ir_polling_on_off(struct pan_device *dev, UINT32 on_off)
{		
	if(NULL == dev)
	{
		PAN_DBG_ERR("[ %s ] error:  dev is NULL!\n", __FUNCTION__);
		return ERR_FAILURE;
	}

	if ((0 != on_off) && (1 != on_off))
	{
		PAN_DBG_ERR("[ %s ] error : Invalid parameters!, on_off = %d\n", 
			__FUNCTION__, on_off);

		return ERR_FAILURE;
	}

	g_ali_pan_ir_dbg_ctrl.enable = on_off;
	PAN_DBG_INFO("[ %s ], enable = %s\n", 
		__FUNCTION__, (1== g_ali_pan_ir_dbg_ctrl.enable) ? "on" : "off");	
	
	if (1 == g_ali_pan_ir_dbg_ctrl.enable)
	{
		PAN_DBG_INFO("waiting ......\n");
	}
	
	if (!g_ali_pan_ir_dbg_ctrl.flag )
	{
		g_ali_pan_ir_dbg_ctrl.flag = TRUE;			
		if (SUCCESS != ir_dbg_task_init())
		{
			g_ali_pan_ir_dbg_ctrl.flag = FALSE;	
			PAN_DBG_ERR("[ %s %d ], Fail\n", __FUNCTION__, __LINE__);

			return ERR_FAILURE;
		}
	}
	
	
	return SUCCESS;
}


static INT32 pan_dag_panel_polling_on_off(struct pan_device *dev, UINT32 on_off)
{		
	if(NULL == dev)
	{
		PAN_DBG_ERR("[ %s ] error:  dev is NULL!\n", __FUNCTION__);
		return ERR_FAILURE;
	}

	if ((0 != on_off) && (1 != on_off))
	{
		PAN_DBG_ERR("[ %s ] error : Invalid parameters!, on_off = %d\n", 
			__FUNCTION__, on_off);

		return ERR_FAILURE;
	}

	g_ali_pan_panel_dbg_ctrl.enable = on_off;
	PAN_DBG_INFO("[ %s ], enable = %s\n", 
		__FUNCTION__, (1== g_ali_pan_panel_dbg_ctrl.enable) ? "on" : "off");	
	
	if (1 == g_ali_pan_panel_dbg_ctrl.enable)
	{
		PAN_DBG_INFO("waiting ......\n");
	}

	if (!g_ali_pan_panel_dbg_ctrl.flag )
	{
		g_ali_pan_panel_dbg_ctrl.flag = TRUE;		
		if (SUCCESS != panel_dbg_task_init())
		{
			g_ali_pan_panel_dbg_ctrl.flag = FALSE;		
			PAN_DBG_ERR("[ %s %d ], Fail\n", __FUNCTION__, __LINE__);

			return ERR_FAILURE;
		}
	}	
	
	
	return SUCCESS;
}


static INT32 pan_dag_ir_polling_interval(struct pan_device *dev, UINT32 interval)
{	
	if(NULL == dev)
	{
		PAN_DBG_ERR("[ %s ] error:  dev is NULL!\n", __FUNCTION__);
		return ERR_FAILURE;
	}

	g_ali_pan_ir_dbg_ctrl.interval = interval;		
	PAN_DBG_INFO("[ %s ], interval = %dms\n", __FUNCTION__, g_ali_pan_ir_dbg_ctrl.interval);	
	
	return SUCCESS;
}



static INT32 pan_dag_panel_polling_interval(struct pan_device *dev, UINT32 interval)
{	
	if(NULL == dev)
	{
		PAN_DBG_ERR("[ %s ] error:  dev is NULL!\n", __FUNCTION__);
		return ERR_FAILURE;
	}

	g_ali_pan_panel_dbg_ctrl.interval = interval;		
	PAN_DBG_INFO("[ %s ], interval = %dms\n", __FUNCTION__, g_ali_pan_panel_dbg_ctrl.interval);	
	
	return SUCCESS;
}


static INT32 pan_dag_ir_polling_option(struct pan_device *dev, UINT32 option)
{	
	if(NULL == dev)
	{
		PAN_DBG_ERR("[ %s ] error:  dev is NULL!\n", __FUNCTION__);
		return ERR_FAILURE;
	}

	if (option > IR_OPT_ALL)
	{		
		PAN_DBG_ERR("[ %s ] error : Invalid parameters!, option = 0x%x, max option = 0x%0x\n", 
			__FUNCTION__, option, IR_OPT_ALL);
		return ERR_FAILURE;
	}

	g_ali_pan_ir_dbg_ctrl.option = option;
	PAN_DBG_INFO("[ %s ], option = 0x%08x\n", __FUNCTION__, g_ali_pan_ir_dbg_ctrl.option);	

	return SUCCESS;
}


static INT32 pan_dag_panel_polling_option(struct pan_device *dev, UINT32 option)
{	
	if(NULL == dev)
	{
		PAN_DBG_ERR("[ %s ] error:  dev is NULL!\n", __FUNCTION__);
		return ERR_FAILURE;
	}

	if (option > PANEL_OPT_ALL)
	{		
		PAN_DBG_ERR("[ %s ] error : Invalid parameters!, option = 0x%x, max option = 0x%0x\n", 
			__FUNCTION__, option, PANEL_OPT_ALL);
		return ERR_FAILURE;
	}

	g_ali_pan_panel_dbg_ctrl.option = option;
	PAN_DBG_INFO("[ %s ], option = 0x%08x\n", __FUNCTION__, g_ali_pan_panel_dbg_ctrl.option);	

	return SUCCESS;
}


static INT32 pan_dag_ir_dump_map(struct pan_device *dev)
{
	UINT32  i = 0;	
	UINT32 phy = 0;
	UINT16 log = 0;	
	struct pan_dbg *pan_dbg_info = NULL;	
	
		
	if(NULL == dev)
	{
		PAN_DBG_ERR("[ %s ] error:  dev is NULL!\n", __FUNCTION__);
		return ERR_FAILURE;
	}	

	pan_dbg_info = (struct pan_dbg *)pan_dbg_get_info(dev);	
	if (NULL == pan_dbg_info->ir_key_map.map_entry)
	{
		PAN_DBG_ERR("[ %s ] error:  ir key map not config!\n", __FUNCTION__);
		return ERR_FAILURE;
	}	

	if (2 != pan_dbg_info->ir_key_map.phy_code)
	{		
		PAN_DBG_INFO("IR unit_num \t\t : %d\n", pan_dbg_info->ir_key_map.unit_num);
		PAN_DBG_INFO("IR map_len \t\t : %d\n", pan_dbg_info->ir_key_map.map_len);		
		PAN_DBG_INFO("IR key type \t\t : %s\n\n", (0 == pan_dbg_info->ir_key_map.phy_code) ? "logic" : "physical");
		PAN_DBG_INFO("{physical,   logic}\n");
		
		for(i=0; i<(UINT32)pan_dbg_info->ir_key_map.unit_num; i++)
		{	
			UINT8 * buf = &(pan_dbg_info->ir_key_map.map_entry[i * pan_dbg_info->ir_key_map.unit_len]);
			
			phy = buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24);
			log = buf[4]|(buf[5]<<8);
			if ((0 == (i % 3)) && (0 != i))
			{
				PAN_DBG_INFO("\n");	
			}
			PAN_DBG_INFO("{0x%08x, 0x%04x}\t", phy, log);		
		}
		PAN_DBG_INFO("\n\n");	
	}
		

	return SUCCESS;	
}


INT32 pan_dag_panel_dump_map(struct pan_device *dev)
{
	UINT32  i = 0;	
	UINT32 phy = 0;
	UINT16 log = 0;	
	struct pan_dbg *pan_dbg_info = NULL;	
	INT8 panel_name[HLD_MAX_NAME_SIZE];			
	
		
	if(NULL == dev)
	{
		PAN_DBG_ERR("[ %s ] error:  dev is NULL!\n", __FUNCTION__);
		return ERR_FAILURE;
	}

	memset(panel_name, 0x00, sizeof(panel_name));
	if (SUCCESS != pan_get_panel_name(panel_name))
	{
		PAN_DBG_ERR("[ %s %d ], No Panel!\n", __FUNCTION__, __LINE__);		
		return ERR_FAILURE;
	}

	pan_dbg_info = (struct pan_dbg *)pan_dbg_get_info(dev);
	if (NULL == pan_dbg_info->panel_key_map.map_entry)
	{
		PAN_DBG_ERR("[ %s ] error:  panel key map not config!\n", __FUNCTION__);
		return ERR_FAILURE;
	}		
	
	if (2 != pan_dbg_info->panel_key_map.phy_code)
	{		
		PAN_DBG_INFO("panel unit_num \t\t : %d\n", pan_dbg_info->panel_key_map.unit_num);
		PAN_DBG_INFO("panel map_len \t\t : %d\n", pan_dbg_info->panel_key_map.map_len);		
		PAN_DBG_INFO("panel key type \t\t : %s\n\n", (0 == pan_dbg_info->panel_key_map.phy_code) ? "logic" : "physical");
		PAN_DBG_INFO("{physical,   logic}\n");
		
		for(i=0; i<(UINT32)pan_dbg_info->panel_key_map.unit_num; i++)
		{	
			UINT8 * buf = &(pan_dbg_info->panel_key_map.map_entry[i * pan_dbg_info->panel_key_map.unit_len]);
			
			phy = buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24);
			log = buf[4]|(buf[5]<<8);			
			PAN_DBG_INFO("{0x%08x, 0x%04x}\t\n", phy, log);	
		}
		PAN_DBG_INFO("\n");
	}	
	

	return SUCCESS;	
}


INT32 panel_dbg_check_exist(void)
{
	INT8 panel_name[HLD_MAX_NAME_SIZE];		

	memset(panel_name, 0x00, sizeof(panel_name));
	if (SUCCESS != pan_get_panel_name(panel_name))
	{		
		return ERR_FAILURE;
	}

	return SUCCESS;
}


static void ir_dbg_show_help(int argc, char **argv)
{	
	PAN_DBG_INFO("Module Name:\n");	
	PAN_DBG_INFO("\tir - ir diagnostic module.\n\n");	

	PAN_DBG_INFO("Synopsis:\n");	
	PAN_DBG_INFO("\tadrdbg ir -cmd [arguments] ...\n\n");
	
	PAN_DBG_INFO("Description:\n");
	PAN_DBG_INFO("\t-h, --help\n");
	PAN_DBG_INFO("\t\tself description of this diagnostic module.\n\n");	
	
	PAN_DBG_INFO("\t-l, --level ir_level\n");
	PAN_DBG_INFO("\t\tset ir debugging printout level (default is 0).\n");
	PAN_DBG_INFO("\t\tlevel 0 \t\t--no printout.\n");
	PAN_DBG_INFO("\t\tlevel 1 \t\t--show HLD printout infomation.\n");
	PAN_DBG_INFO("\t\tlevel 2 \t\t--show Kernel printout infomation.\n");
	PAN_DBG_INFO("\t\tlevel 3 \t\t--show HLD & Kernel printout infomation.\n\n");	

	PAN_DBG_INFO("\t-p, --poll enable\n");
	PAN_DBG_INFO("\t\tenable or disable poll info(default is 0).\n");
	PAN_DBG_INFO("\t\tenable 0 \t\t--disable poll info.\n");	
	PAN_DBG_INFO("\t\tenable 1 \t\t--enable poll info.\n\n");	

	PAN_DBG_INFO("\t-o, --opt option\n");
	PAN_DBG_INFO("\t\tconfig polling option.\n");
	PAN_DBG_INFO("\t\toption  \t\t--what info to poll(default is 0x%x).\n\n", IR_OPT_ALL);

	PAN_DBG_INFO("\t-t, --terv interval\n");
	PAN_DBG_INFO("\t\tconfig polling interval.\n");
	PAN_DBG_INFO("\t\tinterval\t\t--time(ms) to delay(default is 5000ms).\n\n");
	
	PAN_DBG_INFO("\t-i, --info\n");
	PAN_DBG_INFO("\t\tshow ir info.\n\n");	

	PAN_DBG_INFO("\t-d, --dump\n");
	PAN_DBG_INFO("\t\tdump ir key map.\n\n");		

	PAN_DBG_INFO("\t-a, --auto enable interval\n");
	PAN_DBG_INFO("\t\tenable or disable auto change channel, config interval(default is disable, 5000ms).\n");
	PAN_DBG_INFO("\t\tenable 0 \t\t--disable poll info.\n");	
	PAN_DBG_INFO("\t\tenable 1 \t\t--enable poll info.\n\n");	
	PAN_DBG_INFO("\t\tinterval\t\t--time(ms) to delay(default is 5000ms).\n\n");

	PAN_DBG_INFO("\t-e, --exit\n");
	PAN_DBG_INFO("\t\texit ir diagnostic module.\n");		

	PAN_DBG_INFO("\nUsage example:\n");
	PAN_DBG_INFO("\tadrdbg ir -h\n");	
	PAN_DBG_INFO("\tadrdbg ir -l 3\n");
	PAN_DBG_INFO("\tadrdbg ir -p 1\n");
	PAN_DBG_INFO("\tadrdbg ir -o 0x%x\n", IR_OPT_ALL);
	PAN_DBG_INFO("\tadrdbg ir -t 8000\n");
	PAN_DBG_INFO("\tadrdbg ir -i\n");
	PAN_DBG_INFO("\tadrdbg ir -d\n");	
	PAN_DBG_INFO("\tadrdbg ir -a 1 5000\n");	
	PAN_DBG_INFO("\tadrdbg ir -e\n");	
}

static void ir_app_show_help(int argc, char **argv)
{	
	PAN_DBG_INFO("Module Name:\n");	
	PAN_DBG_INFO("\tir - ir diagnostic module.\n\n");	

	PAN_DBG_INFO("Synopsis:\n");	
	PAN_DBG_INFO("\tadrdbg ir -cmd [arguments] ...\n\n");
	
	PAN_DBG_INFO("Description:\n");
	PAN_DBG_INFO("\t-h, --help\n");
	PAN_DBG_INFO("\t\tself description of this diagnostic module.\n\n");	
	
	PAN_DBG_INFO("\t-l, --level ir_level\n");
	PAN_DBG_INFO("\t\tset ir debugging printout level (default is 0).\n");
	PAN_DBG_INFO("\t\tlevel 0 \t\t--no printout.\n");
	PAN_DBG_INFO("\t\tlevel 1 \t\t--show HLD printout infomation.\n");
	PAN_DBG_INFO("\t\tlevel 2 \t\t--show Kernel printout infomation.\n");
	PAN_DBG_INFO("\t\tlevel 3 \t\t--show HLD & Kernel printout infomation.\n\n");	

	PAN_DBG_INFO("\t-i, --info\n");
	PAN_DBG_INFO("\t\tshow ir info.\n\n");	

	PAN_DBG_INFO("\t-d, --dump\n");
	PAN_DBG_INFO("\t\tdump ir key map.\n\n");		

	PAN_DBG_INFO("\t-a, --auto enable interval\n");
	PAN_DBG_INFO("\t\tenable or disable auto change channel, config interval(default is disable, 5000ms).\n");
	PAN_DBG_INFO("\t\tenable 0 \t\t--disable poll info.\n");	
	PAN_DBG_INFO("\t\tenable 1 \t\t--enable poll info.\n\n");	
	PAN_DBG_INFO("\t\tinterval\t\t--time(ms) to delay(default is 5000ms).\n\n");

	PAN_DBG_INFO("\nUsage example:\n");
	PAN_DBG_INFO("\tadrdbg ir -h\n");	
	PAN_DBG_INFO("\tadrdbg ir -l 3\n");
	PAN_DBG_INFO("\tadrdbg ir -i\n");
	PAN_DBG_INFO("\tadrdbg ir -d\n");	
	PAN_DBG_INFO("\tadrdbg ir -a 1 5000\n");	
}

static void panel_dbg_show_help(int argc, char **argv)
{
	if (SUCCESS != panel_dbg_check_exist())
	{
		PAN_DBG_ERR("No Panel\n");
		return;
	}

	
	PAN_DBG_INFO("Module Name:\n");	
	PAN_DBG_INFO("\tpan - panel diagnostic module.\n\n");	

	PAN_DBG_INFO("Synopsis:\n");	
	PAN_DBG_INFO("\tadrdbg pan -cmd [arguments] ...\n\n");
	
	PAN_DBG_INFO("Description:\n");
	PAN_DBG_INFO("\t-h, --help\n");
	PAN_DBG_INFO("\t\tself description of this diagnostic module.\n\n");	
	
	PAN_DBG_INFO("\t-l, --level panel_level\n");
	PAN_DBG_INFO("\t\tset panel debugging printout level (default is 0).\n");
	PAN_DBG_INFO("\t\tlevel 0 \t\t--no printout.\n");
	PAN_DBG_INFO("\t\tlevel 1 \t\t--show HLD printout infomation.\n");
	PAN_DBG_INFO("\t\tlevel 2 \t\t--show Kernel printout infomation.\n");
	PAN_DBG_INFO("\t\tlevel 3 \t\t--show HLD & Kernel printout infomation.\n\n");	

	PAN_DBG_INFO("\t-p, --poll enable\n");
	PAN_DBG_INFO("\t\tenable or disable poll info(default is 0)\n");
	PAN_DBG_INFO("\t\tenable 0 \t\t--disable poll info.\n");	
	PAN_DBG_INFO("\t\tenable 1 \t\t--enable poll info.\n\n");	

	PAN_DBG_INFO("\t-o, --opt option\n");
	PAN_DBG_INFO("\t\tconfig polling option.\n");
	PAN_DBG_INFO("\t\toption  \t\t--what info to poll(default is 0x%x).\n\n", PANEL_OPT_ALL);
	
	PAN_DBG_INFO("\t-t, --terv interval\n");
	PAN_DBG_INFO("\t\tconfig polling interval.\n");
	PAN_DBG_INFO("\t\tinterval\t\t--time(ms) to delay(default is 5000ms).\n\n");
	
	PAN_DBG_INFO("\t-i, --info\n");
	PAN_DBG_INFO("\t\tshow panel info.\n\n");	

	PAN_DBG_INFO("\t-d, --dump\n");
	PAN_DBG_INFO("\t\tdump panel key map.\n\n");	

	PAN_DBG_INFO("\t-e, --exit\n");
	PAN_DBG_INFO("\t\texit panel diagnostic module.\n");		

	PAN_DBG_INFO("\nUsage example:\n");
	PAN_DBG_INFO("\tadrdbg pan -h\n");	
	PAN_DBG_INFO("\tadrdbg pan -l 3\n");
	PAN_DBG_INFO("\tadrdbg pan -p 1\n");
	PAN_DBG_INFO("\tadrdbg pan -o 0x%x\n", PANEL_OPT_ALL);
	PAN_DBG_INFO("\tadrdbg pan -t 7000\n");
	PAN_DBG_INFO("\tadrdbg pan -i\n");
	PAN_DBG_INFO("\tadrdbg pan -d\n");
	PAN_DBG_INFO("\tadrdbg pan -e\n");	
}

static void panel_app_show_help(int argc, char **argv)
{
	if (SUCCESS != panel_dbg_check_exist())
	{
		PAN_DBG_ERR("No Panel\n");
		return;
	}

	
	PAN_DBG_INFO("Module Name:\n");	
	PAN_DBG_INFO("\tpan - panel diagnostic module.\n\n");	

	PAN_DBG_INFO("Synopsis:\n");	
	PAN_DBG_INFO("\tadrdbg pan -cmd [arguments] ...\n\n");
	
	PAN_DBG_INFO("Description:\n");
	PAN_DBG_INFO("\t-h, --help\n");
	PAN_DBG_INFO("\t\tself description of this diagnostic module.\n\n");	
	
	PAN_DBG_INFO("\t-l, --level panel_level\n");
	PAN_DBG_INFO("\t\tset panel debugging printout level (default is 0).\n");
	PAN_DBG_INFO("\t\tlevel 0 \t\t--no printout.\n");
	PAN_DBG_INFO("\t\tlevel 1 \t\t--show HLD printout infomation.\n");
	PAN_DBG_INFO("\t\tlevel 2 \t\t--show Kernel printout infomation.\n");
	PAN_DBG_INFO("\t\tlevel 3 \t\t--show HLD & Kernel printout infomation.\n\n");	

	PAN_DBG_INFO("\t-i, --info\n");
	PAN_DBG_INFO("\t\tshow panel info.\n\n");	

	PAN_DBG_INFO("\t-d, --dump\n");
	PAN_DBG_INFO("\t\tdump panel key map.\n\n");	

	PAN_DBG_INFO("\nUsage example:\n");
	PAN_DBG_INFO("\tadrdbg pan -h\n");	
	PAN_DBG_INFO("\tadrdbg pan -l 3\n");
	PAN_DBG_INFO("\tadrdbg pan -i\n");
	PAN_DBG_INFO("\tadrdbg pan -d\n");
}

static void ir_dbg_set_level(int argc, char **argv)
{
	INT32 ret = ERR_FAILURE;
	UINT32 ir_level = PAN_DBG_LEVEL_DEFAULT;	
	UINT32 max_level = PAN_DBG_LEVEL_HLD | PAN_DBG_LEVEL_KERNEL;	
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
	struct pan_dbg *pan_dbg_info = NULL;	
	
	
	ret = soc_dbg_get_num(1, argv, (int *)(&ir_level), "l");	
	if (SUCCESS != ret)
	{		
		return;
	}
	
	pan_dbg_info = (struct pan_dbg *)pan_dbg_get_info(pan_dev);		

	PAN_DBG_INFO("ir_level = %d\n", ir_level);	
	pan_dag_on_off(pan_dev, (UINT8)ir_level, (UINT8)(pan_dbg_info->panel_level));	
}


static void panel_dbg_set_level(int argc, char **argv)
{
	INT32 ret = ERR_FAILURE;
	UINT32 panel_level = PAN_DBG_LEVEL_DEFAULT;	
	UINT32 max_level = PAN_DBG_LEVEL_HLD | PAN_DBG_LEVEL_KERNEL;	
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
	struct pan_dbg *pan_dbg_info = NULL;	
	

	if (SUCCESS != panel_dbg_check_exist())
	{
		PAN_DBG_ERR("No Panel\n");
		return;
	}
	
	ret = soc_dbg_get_num(1, argv, (int *)&panel_level, "l");	
	if (SUCCESS != ret)
	{		
		return;
	}
	
	pan_dbg_info = (struct pan_dbg *)pan_dbg_get_info(pan_dev);		

	PAN_DBG_INFO("panel_level = %d\n", panel_level);	
	pan_dag_on_off(pan_dev, (UINT8)(pan_dbg_info->ir_level), (UINT8)panel_level);
}


static void ir_dbg_polling(int argc, char **argv)
{
	INT32 ret = ERR_FAILURE;
	UINT32 enable = 0;			
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
	

	ret = soc_dbg_get_num(1, argv, (int *)&enable, "p");	
	if (SUCCESS != ret)
	{		
		return;
	}	
		
	pan_dag_ir_polling_on_off(pan_dev, enable);			
}


static void ir_dbg_polling_set_option(int argc, char **argv)
{
	INT32 ret = ERR_FAILURE;	
	UINT32 opt = 0;			
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
	
	
	ret = soc_dbg_get_num(1, argv, (int *)&opt, "o");	
	if (SUCCESS != ret)
	{		
		return;
	}
		
	pan_dag_ir_polling_option(pan_dev, opt);								
}


static void panel_dbg_polling_set_option(int argc, char **argv)
{
	INT32 ret = ERR_FAILURE;	
	UINT32 opt = 0;			
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);


	if (SUCCESS != panel_dbg_check_exist())
	{
		PAN_DBG_ERR("No Panel\n");
		return;
	}
	
	ret = soc_dbg_get_num(1, argv, (int *)&opt, "o");	
	if (SUCCESS != ret)
	{		
		return;
	}
		
	pan_dag_panel_polling_option(pan_dev, opt);								
}


static void ir_dbg_polling_set_interval(int argc, char **argv)
{
	INT32 ret = ERR_FAILURE;	
	UINT32 interval = 0;	
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
	
	
	ret = soc_dbg_get_num(1, argv, (int *)&interval, "t");	
	if (SUCCESS != ret)
	{		
		return;
	}
	
	pan_dag_ir_polling_interval(pan_dev, interval);			
}


static void panel_dbg_polling_set_interval(int argc, char **argv)
{
	INT32 ret = ERR_FAILURE;	
	UINT32 interval = 0;	
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
	

	if (SUCCESS != panel_dbg_check_exist())
	{
		PAN_DBG_ERR("No Panel\n");
		return;
	}
	
	ret = soc_dbg_get_num(1, argv, (int *)&interval, "t");	
	if (SUCCESS != ret)
	{		
		return;
	}
	
	pan_dag_panel_polling_interval(pan_dev, interval);			
}


static void panel_dbg_polling(int argc, char **argv)
{
	INT32 ret = ERR_FAILURE;
	UINT32 enable[1] = {0};	
	UINT32 opt = 0;
	UINT32 interval = 0;
	INT32 min_len = sizeof(enable) /sizeof(enable[0]);
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
	

	if (SUCCESS != panel_dbg_check_exist())
	{
		PAN_DBG_ERR("No Panel\n");
		return;
	}
	
	ret = soc_dbg_get_num(1, argv, (int *)&enable, "p");	
	if (SUCCESS != ret)
	{		
		return;
	}	
		
	pan_dag_panel_polling_on_off(pan_dev, enable[0]);		
}


static void ir_dbg_show_info(int argc, char **argv)
{	
	struct pan_dbg *pan_dbg_info = NULL;
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
	INT8 panel_name[HLD_MAX_NAME_SIZE];	
	UINT32 ir_rep[2] = {0, 0};				
	

	pan_dbg_info = (struct pan_dbg *)pan_dbg_get_info(pan_dev);	
	
	if (SUCCESS !=  ioctl(pan_dbg_info->input_handle_ir, EVIOCGREP, ir_rep))
	{
		PAN_DBG_ERR("[ %s %d ], Fail to get ir repeat delay and interval!\n", __FUNCTION__, __LINE__);
		perror("");
	}		
	
	PAN_DBG_INFO("Ali IR Info\n");
	PAN_DBG_INFO("last code \t\t : 0x%08x 0x%08x\n", pan_dbg_info->ir_last_key_high, pan_dbg_info->ir_last_key_low);	
	PAN_DBG_INFO("repeat enable \t\t : %d\n", pan_dbg_info->repeat_enable);
	PAN_DBG_INFO("IR delay \t\t : %dms\n", ir_rep[0]);
	PAN_DBG_INFO("IR interval \t\t : %dms\n", ir_rep[1]);		
}


static void panel_hwscan_info_dump(void)
{
	UINT8 i = 0;	
	
	PAN_DBG_INFO("hw info :\n");
	PAN_DBG_INFO("type_kb \t\t : %d\n",  g_ali_pan_hw.type_kb);
	PAN_DBG_INFO("type_scan \t\t : %d\n",  g_ali_pan_hw.type_scan);
	PAN_DBG_INFO("type_key \t\t : %d\n",  g_ali_pan_hw.type_key);
	PAN_DBG_INFO("type_irp \t\t : %d\n",  g_ali_pan_hw.type_irp);
	PAN_DBG_INFO("type_mcu \t\t : %d\n",  g_ali_pan_hw.type_mcu);
	PAN_DBG_INFO("num_com \t\t : %d\n",  g_ali_pan_hw.num_com);
	PAN_DBG_INFO("pos_colon \t\t : %d\n",  g_ali_pan_hw.pos_colon);
	PAN_DBG_INFO("num_scan \t\t : %d\n",  g_ali_pan_hw.num_scan);
	PAN_DBG_INFO("rsvd_bits \t\t : %d\n",  g_ali_pan_hw.rsvd_bits);
	PAN_DBG_INFO("rsvd_byte \t\t : %d\n",  g_ali_pan_hw.rsvd_byte);
	
	PAN_DBG_INFO("flatch    \t\t : %d %d %d\n",  g_ali_pan_hw.flatch.polar, g_ali_pan_hw.flatch.io, g_ali_pan_hw.flatch.position);		
	PAN_DBG_INFO("fclock    \t\t : %d %d %d\n",  g_ali_pan_hw.fclock.polar, g_ali_pan_hw.fclock.io, g_ali_pan_hw.fclock.position);	
	PAN_DBG_INFO("fdata    \t\t : %d %d %d\n",  g_ali_pan_hw.fdata.polar, g_ali_pan_hw.fdata.io, g_ali_pan_hw.fdata.position);	
	for (i=0; i<2; i++)
	{
		PAN_DBG_INFO("scan[%d] \t\t : %d %d %d\n",  i, g_ali_pan_hw.scan[i].polar, g_ali_pan_hw.scan[i].io, g_ali_pan_hw.scan[i].position);
	}
	for (i=0; i<8; i++)
	{
		PAN_DBG_INFO("com[%d]    \t\t : %d %d %d\n",  i, g_ali_pan_hw.com[i].polar, g_ali_pan_hw.com[i].io, g_ali_pan_hw.com[i].position);		
	}
	for (i=0; i<4; i++)
	{
		PAN_DBG_INFO("lbd[%d]    \t\t : %d %d %d\n",  i, g_ali_pan_hw.lbd[i].polar, g_ali_pan_hw.lbd[i].io, g_ali_pan_hw.lbd[i].position);		
	}
	PAN_DBG_INFO("rsvd_hw.polar \t\t : %d %d %d\n",  g_ali_pan_hw.rsvd_hw.polar, g_ali_pan_hw.rsvd_hw.io, g_ali_pan_hw.rsvd_hw.position);		
}


static void panel_dbg_show_info(int argc, char **argv)
{	
	struct pan_dbg *pan_dbg_info = NULL;
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
	INT8 panel_name[HLD_MAX_NAME_SIZE];		
	UINT32 panel_rep[2] = {0, 0};			
	UINT8 i = 0;
	
	
	memset(panel_name, 0x00, sizeof(panel_name));
	if (SUCCESS != pan_get_panel_name(panel_name))
	{
		PAN_DBG_ERR("[ %s %d ], No Panel!\n", __FUNCTION__, __LINE__);		
		return;
	}
	
	PAN_DBG_INFO("Ali %s Info\n", panel_name);

	pan_dbg_info = (struct pan_dbg *)pan_dbg_get_info(pan_dev);	
	
	PAN_DBG_INFO("last code \t\t : 0x%08x 0x%08x\n", 
		pan_dbg_info->panel_last_key_high, pan_dbg_info->panel_last_key_low);	
	PAN_DBG_INFO("repeat enable \t\t : %d\n", pan_dbg_info->repeat_enable);	
	

	if (SUCCESS !=  ioctl(pan_dbg_info->input_handle_panel, EVIOCGREP, panel_rep))
	{
		PAN_DBG_ERR("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);	
		perror("");
	}	
	else
	{
		PAN_DBG_INFO("panel delay \t\t : %dms\n", panel_rep[0]);
		PAN_DBG_INFO("panel interval \t\t : %dms\n", panel_rep[1]);	
	}
	
	if (0 == pan_io_control(pan_dev, PAN_DRIVER_GET_HW_INFO, (UINT32)(&g_ali_pan_hw)))	
	{		
		panel_hwscan_info_dump();
	}		
}


static void ir_dbg_dump_key_map(int argc, char **argv)
{		
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);		
		
	pan_dag_ir_dump_map(pan_dev);		
}


static void ir_dbg_auto_change(int argc, char **argv)
{
	UINT32 data[2];
	INT32 ret = ERR_FAILURE;		
	struct pan_dbg *pan_dbg_info = NULL;
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
	

	memset(data, 0x00, sizeof(data));
	ret = soc_dbg_get_num(2, argv, (int *)&data, "a");	
	if (SUCCESS != ret)
	{		
		return;
	}	

	if ((0 != data[0]) && (1 != data[0]))
	{
		PAN_DBG_ERR("[ %s ] error : Invalid parameters!, on_off = %d\n", 
			__FUNCTION__, data[0]);

		return;
	}

	pan_dbg_info = (struct pan_dbg *)pan_dbg_get_info(pan_dev);	
	pan_dbg_info->auto_change = data[0];
	pan_dbg_info->interval = data[1];	
	pan_dbg_set_info(pan_dev, pan_dbg_info);		

	PAN_DBG_INFO("[ %s ], enable = %s, interval = %dms\n", __FUNCTION__, 
		(1== pan_dbg_info->auto_change) ? "on" : "off", pan_dbg_info->interval);		
}


static void panel_dbg_dump_key_map(int argc, char **argv)
{		
	struct pan_device *pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);				

	if (SUCCESS != panel_dbg_check_exist())
	{
		PAN_DBG_ERR("No Panel\n");
		return;
	}
	
	pan_dag_panel_dump_map(pan_dev);		
}


static void ir_dbg_exit(int argc, char **argv)
{		
	PAN_DBG_ERR("[ %s %d ], wait task exit ... ", __FUNCTION__, __LINE__);
	if (g_ali_pan_ir_dbg_ctrl.taskid != OSAL_INVALID_ID)
	{		
		g_ali_pan_ir_dbg_ctrl.flag = FALSE;	

		#if 0
		while (!g_ali_pan_ir_dbg_ctrl.exit)
		{
			osal_task_sleep(100);
		}
		#endif
		osal_task_delete(g_ali_pan_ir_dbg_ctrl.taskid);
		g_ali_pan_ir_dbg_ctrl.taskid = OSAL_INVALID_ID;		
	}	
	PAN_DBG_ERR("OK!\n", __FUNCTION__, __LINE__);		
}


static void panel_dbg_exit(int argc, char **argv)
{		
	if (SUCCESS != panel_dbg_check_exist())
	{
		PAN_DBG_ERR("No Panel\n");
		return;
	}

	PAN_DBG_ERR("[ %s %d ], wait task exit ... ", __FUNCTION__, __LINE__);
	if (g_ali_pan_panel_dbg_ctrl.taskid != OSAL_INVALID_ID)
	{
		g_ali_pan_panel_dbg_ctrl.flag = FALSE;		

		#if 0
		while (!g_ali_pan_panel_dbg_ctrl.exit)
		{
			osal_task_sleep(100);
		}
		#endif
		osal_task_delete(g_ali_pan_panel_dbg_ctrl.taskid);
		g_ali_pan_panel_dbg_ctrl.taskid = OSAL_INVALID_ID;		
	}
	PAN_DBG_ERR("OK!\n", __FUNCTION__, __LINE__);		
}


static void pan_dbg_init(void)
{
	memset(&g_ali_pan_ir_dbg_ctrl, 0x00, sizeof(g_ali_pan_ir_dbg_ctrl));
	g_ali_pan_ir_dbg_ctrl.taskid = OSAL_INVALID_ID;
	g_ali_pan_ir_dbg_ctrl.flag = FALSE;
	g_ali_pan_ir_dbg_ctrl.exit = TRUE;
	g_ali_pan_ir_dbg_ctrl.option = IR_OPT_ALL;
	g_ali_pan_ir_dbg_ctrl.interval = 5000;			
	
	memset(&g_ali_pan_panel_dbg_ctrl, 0x00, sizeof(g_ali_pan_panel_dbg_ctrl));
	g_ali_pan_panel_dbg_ctrl.taskid = OSAL_INVALID_ID;
	g_ali_pan_panel_dbg_ctrl.flag = FALSE;
	g_ali_pan_panel_dbg_ctrl.exit = TRUE;
	g_ali_pan_panel_dbg_ctrl.option = PANEL_OPT_ALL;
	g_ali_pan_panel_dbg_ctrl.interval = 5000;		
	
	memset(&g_ali_pan_hw, 0x00, sizeof(g_ali_pan_hw));	
}


static PARSE_COMMAND_LIST g_ali_pan_irdbg_command[] = {
	{ { NULL, NULL }, ir_dbg_show_help, soc_dbg_no_param_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ir_dbg_set_level, soc_dbg_one_param_preview, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ir_dbg_polling, soc_dbg_one_param_preview, "poll", "p", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ir_dbg_polling_set_option, soc_dbg_one_param_preview, "opt", "o", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ir_dbg_polling_set_interval, soc_dbg_one_param_preview, "terv", "t", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ir_dbg_show_info, soc_dbg_no_param_preview, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ir_dbg_dump_key_map, soc_dbg_no_param_preview, "dump", "d", 0, 0, NULL, 0, 0, NULL, 0, 0 },	
	{ { NULL, NULL }, ir_dbg_auto_change, soc_dbg_two_param_preview, "auto", "a", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ir_dbg_exit, soc_dbg_no_param_preview, "exit", "e", 0, 0, NULL, 0, 0, NULL, 0, 0 },	
};

static PARSE_COMMAND_LIST g_ali_pan_irdbg_app_command[] = {
	{ { NULL, NULL }, ir_app_show_help, soc_dbg_no_param_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ir_dbg_set_level, soc_dbg_one_param_preview, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ir_dbg_polling, soc_dbg_one_param_preview, "poll", "p", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ir_dbg_polling_set_option, soc_dbg_one_param_preview, "opt", "o", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ir_dbg_polling_set_interval, soc_dbg_one_param_preview, "terv", "t", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ir_dbg_show_info, soc_dbg_no_param_preview, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ir_dbg_dump_key_map, soc_dbg_no_param_preview, "dump", "d", 0, 0, NULL, 0, 0, NULL, 0, 0 },	
	{ { NULL, NULL }, ir_dbg_auto_change, soc_dbg_two_param_preview, "auto", "a", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ir_dbg_exit, soc_dbg_no_param_preview, "exit", "e", 0, 0, NULL, 0, 0, NULL, 0, 0 },	
};


static PARSE_COMMAND_LIST g_ali_pan_paneldbg_command[] = {
	{ { NULL, NULL }, panel_dbg_show_help, soc_dbg_no_param_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, panel_dbg_set_level, soc_dbg_one_param_preview, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, panel_dbg_polling, soc_dbg_one_param_preview, "poll", "p", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, panel_dbg_polling_set_option, soc_dbg_one_param_preview, "opt", "o", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, panel_dbg_polling_set_interval, soc_dbg_one_param_preview, "terv", "t", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, panel_dbg_show_info, soc_dbg_no_param_preview, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, panel_dbg_dump_key_map, soc_dbg_no_param_preview, "dump", "d", 0, 0, NULL, 0, 0, NULL, 0, 0 },	
	{ { NULL, NULL }, panel_dbg_exit, soc_dbg_no_param_preview, "exit", "e", 0, 0, NULL, 0, 0, NULL, 0, 0 },	
};

static PARSE_COMMAND_LIST g_ali_pan_paneldbg_app_command[] = {
	{ { NULL, NULL }, panel_app_show_help, soc_dbg_no_param_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, panel_dbg_set_level, soc_dbg_one_param_preview, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, panel_dbg_polling, soc_dbg_one_param_preview, "poll", "p", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, panel_dbg_polling_set_option, soc_dbg_one_param_preview, "opt", "o", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, panel_dbg_polling_set_interval, soc_dbg_one_param_preview, "terv", "t", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, panel_dbg_show_info, soc_dbg_no_param_preview, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, panel_dbg_dump_key_map, soc_dbg_no_param_preview, "dump", "d", 0, 0, NULL, 0, 0, NULL, 0, 0 },	
	//{ { NULL, NULL }, panel_dbg_exit, soc_dbg_no_param_preview, "exit", "e", 0, 0, NULL, 0, 0, NULL, 0, 0 },	
};



void pan_module_register(void)
{
	pan_dbg_init();
	debug_module_add("ir", &g_ali_pan_irdbg_command[0], ARRAY_SIZE(g_ali_pan_irdbg_command));
	debug_module_add("pan", &g_ali_pan_paneldbg_command[0], ARRAY_SIZE(g_ali_pan_paneldbg_command));
}

INT32 pan_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 *list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &g_ali_pan_paneldbg_command[0];
	*list_cnt = ARRAY_SIZE(g_ali_pan_paneldbg_command);

	pan_dbg_init();

	return RET_SUCCESS;
}

INT32 ir_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &g_ali_pan_irdbg_command[0];
	*list_cnt = ARRAY_SIZE(g_ali_pan_irdbg_command);

	return RET_SUCCESS;
}

INT32 pan_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 *list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &g_ali_pan_paneldbg_app_command[0];
	*list_cnt = ARRAY_SIZE(g_ali_pan_paneldbg_app_command);

	pan_dbg_init();

	return RET_SUCCESS;
}

INT32 ir_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &g_ali_pan_irdbg_app_command[0];
	*list_cnt = ARRAY_SIZE(g_ali_pan_irdbg_app_command);

	return RET_SUCCESS;
}
