/*****************************************************************************
*   Copyrights(C) 2012 ALi corp. All Rights Reserved.
*
*   FILE NAME:      adr_deca_dbg.c
*
*   DESCRIPTION:    Deca debug module.
*
*   HISTORY:
*                      Date      Author      Version      Notes
*                   ==========  =========   =========   ===========
*                   2012-10-24                                  Ver 1.0                Create File
*****************************************************************************/
#include "adr_deca_dbg.h"


#define DECA_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(DECA, fmt, ##args); \
			} while(0)



extern UINT8 deca_hld_dbg;
UINT32 g_deca_dbg_level;
static deca_dbg_stat_ctrl DecaDbgCtrl;
UINT32 deca_dbg_see_level = DBG_DECA_NO_OUTPUT;

static int deca_dbg_device_open(void)
{
	if (DecaDbgCtrl.dbg_dev_fd)
	{
		DECA_DBG_PRINT("Deca diagnostic debugging module has opened.\n");
	}
	else
	{	
		DecaDbgCtrl.dbg_dev_fd = open("/dev/ali_m36_audio0", O_RDONLY | O_CLOEXEC);
		if (DecaDbgCtrl.dbg_dev_fd < 0) 
		{    
			DECA_DBG_PRINT("Deca diagnostic debugging module open fail!\n");
			return (-1);
		}				
	}

	return DecaDbgCtrl.dbg_dev_fd;
}

static void deca_dbg_device_close(void)
{
    return;
    //keep the dev file fd
}

/*!
@brief 获取设备 deca 的工作状态信息。 
@details 当前仅返回设备状态，等待完善。
*/
static void deca_show_dev_status()
{
	UINT32 deca_flag = 0;;
	DECA_DBG_PRINT("\n#################Show Deca Dev Status##################\n");
	deca_io_control(DecaDbgCtrl.dbg_dev ,DECA_GET_DECA_STATE,(UINT32)&deca_flag);
	switch(deca_flag){
		case DECA_STATE_DETACH:
			DECA_DBG_PRINT("Deca dev flag : DETACH\n");
			break;
		case DECA_STATE_ATTACH:
			DECA_DBG_PRINT("Deca dev flag : ATTACH\n");
			break;            
		case DECA_STATE_IDLE:
			DECA_DBG_PRINT("Deca dev flag : IDLE\n");
			break;
		case DECA_STATE_PLAY:
			DECA_DBG_PRINT("Deca dev flag : PLAY\n");
			break;      
		case DECA_STATE_PAUSE:
			DECA_DBG_PRINT("Deca dev flag : PAUSE\n");
			break;        
		default:
			DECA_DBG_PRINT("Deca in UNKNOW status!!flag %d\n",deca_flag);
			break;
	}  

	struct cur_stream_info str_info;//add some decode error cnt
	memset(&str_info,0,sizeof(struct cur_stream_info));
	deca_io_control(DecaDbgCtrl.dbg_dev ,DECA_GET_PLAY_PARAM,(UINT32)&str_info);

	switch(str_info.str_type){
		case AUDIO_MPEG1:
		case AUDIO_MPEG2:
			DECA_DBG_PRINT("Stream Type : Mpeg1/2\n");
			break;
		case AUDIO_MPEG_AAC:
			DECA_DBG_PRINT("Stream Type : MPEG LATM AAC\n");
			break;            
		case AUDIO_AC3:
			DECA_DBG_PRINT("Stream Type : AC3\n");
			break;
		case AUDIO_DTS:
			DECA_DBG_PRINT("Stream Type : DTS\n");
			break;      
		case AUDIO_PPCM:
			DECA_DBG_PRINT("Stream Type : PPCM\n");
			break;
		case AUDIO_LPCM_V:
			DECA_DBG_PRINT("Stream Type : LPCM_V\n");
			break;            
		case AUDIO_LPCM_A:
			DECA_DBG_PRINT("Stream Type : LPCM_A\n");
			break;
		case AUDIO_PCM:
			DECA_DBG_PRINT("Stream Type : PCM\n");
			break;    
		case AUDIO_WMA:
			DECA_DBG_PRINT("Stream Type : WMA\n");
			break;
		case AUDIO_RA8:
			DECA_DBG_PRINT("Stream Type : RA8\n");
			break;      
		case AUDIO_MPEG_ADTS_AAC:
			DECA_DBG_PRINT("Stream Type : MPEG_ADTS_AAC\n");
			break;    
		case AUDIO_OGG:
			DECA_DBG_PRINT("Stream Type : OGG\n");
			break;
		case AUDIO_EC3:
			DECA_DBG_PRINT("Stream Type : EC3\n");
			break;                 
		default:
			DECA_DBG_PRINT("Invalid STREAM TYPE!!");
			break;
	}  
	DECA_DBG_PRINT("Bits_depth : %d\n",str_info.bit_depth);
	DECA_DBG_PRINT("Sample_rate : %d\n",str_info.sample_rate);
	DECA_DBG_PRINT("Samp_num : %d\n",str_info.samp_num);
	DECA_DBG_PRINT("Chan_num : %d\n",str_info.chan_num);

	DECA_DBG_PRINT("Cur_frm_pts : 0x%08x\n",str_info.cur_frm_pts);
	DECA_DBG_PRINT("Input_ts_cnt : %d\n",str_info.input_ts_cnt);
	DECA_DBG_PRINT("Sync_error_cnt : %d\n",str_info.sync_error_cnt);
	DECA_DBG_PRINT("Sync_success_cnt : %d\n",str_info.sync_success_cnt);
	DECA_DBG_PRINT("Sync_frm_len : %d\n",str_info.sync_frm_len);
	DECA_DBG_PRINT("Decode_error_cnt : %d\n",str_info.decode_error_cnt);
	DECA_DBG_PRINT("Decode_success_cnt : %d\n",str_info.decode_success_cnt);    
}

/*!
@brief 获取deca buf信息。 
@details 等待完善，可以加上cb ,rd信息。
*/
static void deca_show_buf_status_en()
{
	struct deca_buf_info buf_info;// i think it's better to add cb and ptr info,some dump es/pcm info
	memset(&buf_info,0,sizeof(struct deca_buf_info));
	deca_io_control(DecaDbgCtrl.dbg_dev ,DECA_GET_ES_BUFF_STATE,(UINT32)&buf_info);
	DECA_DBG_PRINT("\n#################Show Deca Buf Status ####################\n");

	DECA_DBG_PRINT("Es_buf_base_addr : 0x%08x\n",buf_info.buf_base_addr);
	DECA_DBG_PRINT("Es_buf_len : 0x%08x\n",buf_info.buf_len);
	DECA_DBG_PRINT("Es_used_len : 0x%08x\n",buf_info.used_len);     
	DECA_DBG_PRINT("Es_remain_len : 0x%08x\n",buf_info.remain_len);     
	DECA_DBG_PRINT("Es_cb_rd : 0x%08x\n",buf_info.cb_rd);     
	DECA_DBG_PRINT("Es_cb_wt : 0x%08x\n",buf_info.cb_wt);     
	DECA_DBG_PRINT("Es_rd : 0x%08x\n",buf_info.es_rd);     
	DECA_DBG_PRINT("Es_wt : 0x%08x\n",buf_info.es_wt);     
}

static void deca_dbg_task(UINT32 param1, UINT32 param2)
{
	UINT32 CurShowSysTickCnt;
	UINT32 PrevShowSysTickCnt = 0;
	UINT32 SysTickInc;

	while(1)
	{
		CurShowSysTickCnt = osal_get_tick();
		SysTickInc = CurShowSysTickCnt - PrevShowSysTickCnt;
		PrevShowSysTickCnt = CurShowSysTickCnt;

		if (DecaDbgCtrl.dbg_enable)
		{
			if (DecaDbgCtrl.dbg_get_dev_status_en)
			{
				deca_show_dev_status();
			}
			if (DecaDbgCtrl.dbg_get_buf_status_en)
			{
				deca_show_buf_status_en();
			}                    
			if (DecaDbgCtrl.dbg_show_ims > 0 && 0 == --DecaDbgCtrl.dbg_show_ims)
			{
				DECA_DBG_PRINT("\nTimes up! End of printout.\n");
				DecaDbgCtrl.dbg_enable = FALSE;
			}
		}
		osal_task_sleep(DecaDbgCtrl.dbg_show_intv);
	}
}

static RET_CODE deca_dbg_task_Init()
{
	OSAL_T_CTSK t_ctsk;
	memset(&t_ctsk, 0, sizeof(OSAL_T_CTSK ));
	t_ctsk.task = (FP)deca_dbg_task;
	t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 10;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.name[0] = 'D';
	t_ctsk.name[1] = 'C';
	t_ctsk.name[2] = 'A';
	
	DecaDbgCtrl.dbg_task_id = osal_task_create(&t_ctsk);
	if (OSAL_INVALID_ID == DecaDbgCtrl.dbg_task_id)
	{
		DECA_DBG_PRINT("Deca debug task init fail!\n");
		return RET_FAILURE;
	}

	return RET_SUCCESS;
}

static void deca_dbg_init(void)
{
	memset(&DecaDbgCtrl, 0, sizeof(deca_dbg_stat_ctrl));
	DecaDbgCtrl.dbg_init      = TRUE;
	DecaDbgCtrl.dbg_enable    = FALSE;
	DecaDbgCtrl.dbg_show_intv = DECA_DBG_PRINT_INTRV;
	DecaDbgCtrl.dbg_get_buf_status_en = FALSE;
	DecaDbgCtrl.dbg_get_dev_status_en = FALSE;
	g_deca_dbg_level = 0;
	deca_hld_dbg = 0;

	DecaDbgCtrl.dbg_dev = (struct deca_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_DECA);
	if(DecaDbgCtrl.dbg_dev == NULL)
	{
		//DECA_DBG_PRINT("get deca device failure!\n");
		return;
	}

	if (deca_dbg_device_open() < 0)
	{
		//DECA_DBG_PRINT("Deca debug device open fail!\n");
		return;
	}
}

static void deca_dbg_show_help(int argc, char **argv)
{
	DECA_DBG_PRINT("\nModule name:\n");
	DECA_DBG_PRINT("\tdeca\n");
	DECA_DBG_PRINT("\nModule Description:\n");
	DECA_DBG_PRINT("\nDeca diagnostic debugging module.\n");	
	DECA_DBG_PRINT("\nSyntax:\n");
	DECA_DBG_PRINT("\tdeca -option [arguments]...\n");
	DECA_DBG_PRINT("\nDescription:\n");
    
	DECA_DBG_PRINT("\t-h, --help\n");
	DECA_DBG_PRINT("\t\tSelf description of this debugging moduld.\n");
    
	DECA_DBG_PRINT("\t-s, --start\n");
	DECA_DBG_PRINT("\t\tStart Deca debugging printout.\n");
    
	DECA_DBG_PRINT("\t-p, --pause\n");
	DECA_DBG_PRINT("\t\tPause Deca debugging printout.\n");
    
	DECA_DBG_PRINT("\t-e, --exit\n");
	DECA_DBG_PRINT("\t\tExit Deca debugging module.\n");
    
	DECA_DBG_PRINT("\t-i, --info items...\n");
	DECA_DBG_PRINT("\t\tShow Deca information once!!, such as device flags,stream type,sample rate, es buf rd wt.\n");
	DECA_DBG_PRINT("\t\tall\t--Show All infomation.\n");
	DECA_DBG_PRINT("\t\tdev\t--Device information,show current device status\n");
	DECA_DBG_PRINT("\t\tbuf\t--Audio es buf information,show buf consuming status.\n");
    
	DECA_DBG_PRINT("\t-op, --open items...\n");
	DECA_DBG_PRINT("\t\tOpen debugging printout items.\n");
	DECA_DBG_PRINT("\t\tall\t--Enable All the items print.\n");
	DECA_DBG_PRINT("\t\tdev\t--Enabel Device information print,show current device status.\n");
	DECA_DBG_PRINT("\t\tbuf\t--Enabel Audio es buf information print ,show buf consuming status.\n");
	DECA_DBG_PRINT("\t\thld\t--Enable Main Deca hld print\n");
	DECA_DBG_PRINT("\t\tsee_api\t--Enable See Deca api print\n");
	DECA_DBG_PRINT("\t\tsee_buf\t--Enable See Deca buf print\n");
	DECA_DBG_PRINT("\t\tsee_err\t--Enable See Deca error print\n");

	DECA_DBG_PRINT("\t-cl, --close items...\n");
	DECA_DBG_PRINT("\t\tClose debugging printout items.\n");
	DECA_DBG_PRINT("\t\tall\t--Disable All the items print.\n");
	DECA_DBG_PRINT("\t\tdev\t--Disable Device information print\n");
	DECA_DBG_PRINT("\t\tbuf\t--Disable Audio es buf information print\n");
	DECA_DBG_PRINT("\t\thld\t--Disable Main Deca hld print\n");
	DECA_DBG_PRINT("\t\tsee_api\t--Disable See Deca api print\n");
	DECA_DBG_PRINT("\t\tsee_buf\t--Disable See Deca buf print\n");
	DECA_DBG_PRINT("\t\tsee_err\t--Disable See Deca error print\n");

	DECA_DBG_PRINT("\t-terv number\n");
	DECA_DBG_PRINT("\t\tSet time interval of debugging printout task(The default value is \"5000\").\n");
    
	DECA_DBG_PRINT("\t--times number\n");
	DECA_DBG_PRINT("\t\tSet times of debugging printout task(The default value is \"0\").\n");
    
	DECA_DBG_PRINT("\t-t, --test number\n");
	DECA_DBG_PRINT("\t\tAudio format test.\n");
    
	DECA_DBG_PRINT("\nUsage example:\n");
	DECA_DBG_PRINT("\tdeca -h[--help]\n");
	DECA_DBG_PRINT("\tdeca -s\n");
	DECA_DBG_PRINT("\tdeca -p\n");
	DECA_DBG_PRINT("\tdeca -e[--exit]\n");
	DECA_DBG_PRINT("\tdeca -i[--info] [all] [dec] [dev] [buf]\n");
	DECA_DBG_PRINT("\tdeca -op[--open] [all] [dev] [buf] [hld ][see_api] [see_buf] [see_err]\n");
	DECA_DBG_PRINT("\tdeca -cl[--close] [all]  [dev] [buf] [hld ][see_api] [see_buf] [see_err]\n");
	DECA_DBG_PRINT("\tdeca -terv 5000\n");
	DECA_DBG_PRINT("\tdeca --times 0\n");
	DECA_DBG_PRINT("\tdeca -t  1\n");

	return;
}

static void deca_app_show_help(int argc, char **argv)
{
	DECA_DBG_PRINT("\nModule name:\n");
	DECA_DBG_PRINT("\tdeca\n");
	DECA_DBG_PRINT("\nModule Description:\n");
	DECA_DBG_PRINT("\nDeca diagnostic debugging module.\n");	
	DECA_DBG_PRINT("\nSyntax:\n");
	DECA_DBG_PRINT("\tdeca -option [arguments]...\n");
	DECA_DBG_PRINT("\nDescription:\n");
    
	DECA_DBG_PRINT("\t-h, --help\n");
	DECA_DBG_PRINT("\t\tSelf description of this debugging moduld.\n");
    
	DECA_DBG_PRINT("\t-i, --info items...\n");
	DECA_DBG_PRINT("\t\tShow Deca information once!!, such as device flags,stream type,sample rate, es buf rd wt.\n");
	DECA_DBG_PRINT("\t\tall\t--Show All infomation.\n");
	DECA_DBG_PRINT("\t\tdev\t--Device information,show current device status\n");
	DECA_DBG_PRINT("\t\tbuf\t--Audio es buf information,show buf consuming status.\n");
    
	DECA_DBG_PRINT("\t-t, --test number\n");
	DECA_DBG_PRINT("\t\tAudio format test.\n");

	DECA_DBG_PRINT("\nUsage example:\n");
	DECA_DBG_PRINT("\tdeca -h[--help]\n");
	DECA_DBG_PRINT("\tdeca -i[--info] [all] [dec] [dev] [buf]\n");
	DECA_DBG_PRINT("\tdeca -t  1\n");

	return;
}


static void deca_dbg_start(int argc, char **argv)
{
	if (!DecaDbgCtrl.dbg_task_enable)
	{
		if (deca_dbg_task_Init() != RET_SUCCESS)
		{
			DECA_DBG_PRINT("Deca debug task initialized fail!\n");
			return;
		}
		DecaDbgCtrl.dbg_task_enable = 1;
	}

	DecaDbgCtrl.dbg_enable = TRUE;
	DECA_DBG_PRINT("Deca debug task start.\n");
}


static void deca_dbg_pause(int argc, char **argv)
{
	if (!DecaDbgCtrl.dbg_task_enable)
	{
		if (deca_dbg_task_Init() != RET_SUCCESS)
		{
			DECA_DBG_PRINT("Deca debug task initialized fail!\n");
			return;
		}
		DecaDbgCtrl.dbg_task_enable = 1;
	}

	DecaDbgCtrl.dbg_enable = FALSE;
	DECA_DBG_PRINT("Deca debug task pause.\n");
}

static void deca_dbg_exit(int argc, char **argv)
{
	DecaDbgCtrl.dbg_enable    = FALSE;

	DecaDbgCtrl.dbg_show_intv = DECA_DBG_PRINT_INTRV;
	DecaDbgCtrl.dbg_get_buf_status_en = FALSE;
	DecaDbgCtrl.dbg_get_dev_status_en = FALSE;
	g_deca_dbg_level = 0;
	deca_hld_dbg = 0;

	/*
	   if (DecaDbgCtrl.dbg_task_id > 0)
	   {
	   osal_task_delete(DecaDbgCtrl.dbg_task_id);
	   DecaDbgCtrl.dbg_init = FALSE;
	   DecaDbgCtrl.dbg_enable = FALSE;
	   DECA_DBG_PRINT("Deca debug task closed.\n");
	   }
	   */
}

static ARG_CHK deca_dbg_no_para_preview(int argc, char **argv, char *option)
{
	if (argc != 0)
	{
		DECA_DBG_PRINT("Option \"%s\": Should not be with any argument(s)!\n", option);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}

//only show deca status once
static void deca_dbg_show_info(int argc, char **argv)
{
	if (0 == argc)
		goto DECA_INFO;

	while (argc-- > 0)
	{
		if (!strcmp(*argv, "all"))
		{
DECA_INFO:
			deca_show_dev_status();
			deca_show_buf_status_en();
		}        
		else if (!strcmp(*argv, "dev"))
		{
			deca_show_dev_status();
		}
		else if (!strcmp(*argv, "buf"))
		{
			deca_show_buf_status_en();
		}        
		else
		{
			DECA_DBG_PRINT("Unrecognized argument(s)!\n");
		}

		argv++;
	}

	return;
}


static ARG_CHK deca_dbg_info_preview(int argc, char **argv, char *option)
{
	while (argc-- > 0)
	{
		if (strcmp(*argv, "all") && 
			strcmp(*argv, "dev") && 
			strcmp(*argv, "hld") && 
			strcmp(*argv, "buf")) 
		{
			DECA_DBG_PRINT("Option \"%s\": Unrecognized argument(s) \"%s\"!\n", option, *argv);
			return DBG_ARG_INVALID;
		}

		argv++;
	}

	return DBG_ARG_VALID;
}

static void deca_dbg_switch_on(int argc, char **argv)
{
	if (!DecaDbgCtrl.dbg_task_enable)
	{
		if (deca_dbg_task_Init() != RET_SUCCESS)
		{
			DECA_DBG_PRINT("Deca debug task initialized fail!\n");
			return;
		}
		DecaDbgCtrl.dbg_task_enable = 1;
	}

	if (!argc)
		goto DECA_ON_ALL;

	while (argc-- > 0)
	{
		if (!strcmp(*argv, "all"))
		{
DECA_ON_ALL:
			DecaDbgCtrl.dbg_get_buf_status_en= TRUE;
			DecaDbgCtrl.dbg_get_dev_status_en= TRUE;
			deca_hld_dbg = 1;
			deca_dbg_see_level = DBG_DECA_API|DBG_DECA_BUF|DBG_DECA_ERROR;
			deca_set_dbg_level(DecaDbgCtrl.dbg_dev,deca_dbg_see_level);
		}
		else if (!strcmp(*argv, "dev"))
		{
			DecaDbgCtrl.dbg_get_dev_status_en = TRUE;
		}
		else if (!strcmp(*argv, "buf"))
		{
			DecaDbgCtrl.dbg_get_buf_status_en = TRUE;
		}
		else if (!strcmp(*argv, "hld"))
		{
			deca_hld_dbg = 1;
		}	
		else if (!strcmp(*argv, "see_api"))
		{
			deca_dbg_see_level |= DBG_DECA_API;	
			deca_set_dbg_level(DecaDbgCtrl.dbg_dev,deca_dbg_see_level);
		}      	
		else if (!strcmp(*argv, "see_buf"))
		{
			deca_dbg_see_level |= DBG_DECA_BUF;	
			deca_set_dbg_level(DecaDbgCtrl.dbg_dev,deca_dbg_see_level);			
		}      	
		else if (!strcmp(*argv, "see_err"))
		{
			deca_dbg_see_level |= DBG_DECA_ERROR;	
			deca_set_dbg_level(DecaDbgCtrl.dbg_dev,deca_dbg_see_level);			
		}      	
		else if (!strcmp(*argv, "see_all"))
		{
			deca_dbg_see_level |= DBG_LEV_ALL;	
			deca_set_dbg_level(DecaDbgCtrl.dbg_dev,deca_dbg_see_level);			
		}      	
		else
		{
			DECA_DBG_PRINT("Unrecognized argument(s)!\n");
		}

		argc++;
	}

	return;
}

static void deca_dbg_switch_off(int argc, char **argv)
{
	if (!DecaDbgCtrl.dbg_task_enable)
	{
		if (deca_dbg_task_Init() != RET_SUCCESS)
		{
			DECA_DBG_PRINT("Deca debug task initialized fail!\n");
			return;
		}
		DecaDbgCtrl.dbg_task_enable = 1;
	}

	if (!argc)
		goto DECA_OFF_ALL;

	while (argc-- > 0)
	{
		if (!strcmp(*argv, "all"))
		{
DECA_OFF_ALL:
			DecaDbgCtrl.dbg_get_buf_status_en= FALSE;
			DecaDbgCtrl.dbg_get_dev_status_en= FALSE;
			deca_hld_dbg = 0;
			deca_dbg_see_level &= ~ (DBG_DECA_API|DBG_DECA_BUF|DBG_DECA_ERROR);
			deca_set_dbg_level(DecaDbgCtrl.dbg_dev,deca_dbg_see_level);
		}
		else if (!strcmp(*argv, "dev"))
		{
			DecaDbgCtrl.dbg_get_dev_status_en = FALSE;
		}
		else if (!strcmp(*argv, "buf"))
		{
			DecaDbgCtrl.dbg_get_buf_status_en = FALSE;
		}
		else if (!strcmp(*argv, "hld"))
		{
			deca_hld_dbg = 0;
		}	
		else if (!strcmp(*argv, "see_api"))
		{
			deca_dbg_see_level &= ~DBG_DECA_API;	
			deca_set_dbg_level(DecaDbgCtrl.dbg_dev,deca_dbg_see_level);
		}      	
		else if (!strcmp(*argv, "see_buf"))
		{
			deca_dbg_see_level &= ~DBG_DECA_BUF;	
			deca_set_dbg_level(DecaDbgCtrl.dbg_dev,deca_dbg_see_level);			
		}      	
		else if (!strcmp(*argv, "see_err"))
		{
			deca_dbg_see_level &= ~DBG_DECA_ERROR;	
			deca_set_dbg_level(DecaDbgCtrl.dbg_dev,deca_dbg_see_level);			
		}      	
		else if (!strcmp(*argv, "see_all"))
		{
			deca_dbg_see_level &= ~DBG_LEV_ALL;	
			deca_set_dbg_level(DecaDbgCtrl.dbg_dev,deca_dbg_see_level);			
		}      		
		else
		{
			DECA_DBG_PRINT("Unrecognized argument(s)!\n");
		}

		argv++;
	}

	return;
}

static ARG_CHK deca_dbg_swi_preview(int argc, char **argv, char *option)
{
	while (argc-- > 0)
	{
		if (strcmp(*argv, "all") && 
			strcmp(*argv, "dev") && 
			strcmp(*argv, "hld") && 
			strcmp(*argv, "buf") && 
			strcmp(*argv, "see_api") && 
			strcmp(*argv, "see_buf") && 
			strcmp(*argv, "see_all") && 
			strcmp(*argv, "see_err")) 
		{
			DECA_DBG_PRINT("Option \"%s\": Unrecognized argument(s) \"%s\"!\n", option, *argv);
			return DBG_ARG_INVALID;
		}

		argv++;
	}

	return DBG_ARG_VALID;
}

static void deca_dbg_set_intv(int argc, char **argv)
{
	int number, ret;

	if (0 == argc)
	{
		DECA_DBG_PRINT("Lack of valid argument(ex: \"5000\")!\n");
		return;
	}

	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		DECA_DBG_PRINT("Argument \"%s\" is not a number or less than zero!\n", *argv);
		return;
	}

	DecaDbgCtrl.dbg_show_intv = number;
}

static void deca_dbg_set_tms(int argc, char **argv)
{
	int number, ret;

	if (0 == argc)
	{
		DECA_DBG_PRINT("Lack of valid argument(ex: \"5\")!\n");
		return;
	}

	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		DECA_DBG_PRINT("Argument \"%s\" is not a number or less than zero!\n", *argv);
		return;
	}

	DecaDbgCtrl.dbg_show_ims = number;
}

static ARG_CHK deca_dbg_num_preview(int argc, char **argv, char *option)
{
	int number, ret;

	if (0 == argc)
	{
		DECA_DBG_PRINT("Lack of valid argument(ex: \"5\")!\n");
		return DBG_ARG_INVALID;
	}

	if (argc != 1)
	{
		DECA_DBG_PRINT("Option \"%s\": Only one argument permitted!\n", option);
		return DBG_ARG_INVALID;
	}
    
	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		DECA_DBG_PRINT("Option \"%s\": Argument \"%s\" not a number or less than zero!\n", option, *argv);
		return DBG_ARG_INVALID;
	}
	
	return DBG_ARG_VALID;
}

static void deca_push_es_test()
{
	#define FILEPATH "/mnt/audio.aac"

	FILE *in ;
	if( ( in = fopen( FILEPATH, "rb" ) ) == NULL )
	{
		DECA_DBG_PRINT( "errossdsdr can not read 1.txt.\npress any key to continue..." ); 
		return;
	}

	UINT32 file_len;
	fseek(in,0,SEEK_END);
	file_len = ftell(in);
	fseek(in,0,SEEK_SET);

	deca_io_control(DecaDbgCtrl.dbg_dev,DECA_SET_STR_TYPE,AUDIO_MPEG_ADTS_AAC);
	deca_start(DecaDbgCtrl.dbg_dev,0);
	//snd_set_volume(DecaDbgCtrl.dbg_snd_dev,SND_SUB_OUT,50);
	struct control_block ctk;
	memset(&ctk,0,sizeof(ctk));
		
	UINT32 ret_size;
	UINT8 *req_buf;
	INT32 req_size;
	UINT8 ret_code;
	UINT32 update_len = 0;
	DECA_DBG_PRINT("begin push,update len %d file_len %d\n",update_len,file_len);
	req_size = file_len;
	while(update_len < file_len)
	{
		ret_code = deca_request_write(DecaDbgCtrl.dbg_dev,req_size,(void **)&req_buf,&ret_size,&ctk);

		//DECA_DBG_PRINT("req size %d ret size %d\n",req_size,ret_size);
		//DECA_DBG_PRINT("reqbuf %p ret size %d retcode %d\n",req_buf,ret_size,ret_code);
		if(ret_code == RET_SUCCESS)
		{
		
			fread(req_buf,1,ret_size,in);
			
			deca_update_write(DecaDbgCtrl.dbg_dev,ret_size);
			update_len += 188;
			req_size -= 188;
		
		}
		else
		{
		osal_task_sleep(10);
		}
	}
	DECA_DBG_PRINT(" push over,update len %d file_len %d\n",update_len,file_len);

	if (in)
	{
		fclose(in);
	}
}

static void deca_play_mp3()
{
}
static void deca_play_pcm()
{
}
static void deca_dbg_module_test(int argc, char **argv)
{
	int number, ret;

	if (0 == argc)
	{
		DECA_DBG_PRINT("Lack of valid argument(ex: \"100\")!\n");
		return;
	}

	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		DECA_DBG_PRINT("Argument \"%s\" is not a number or less than zero!\n", *argv);
		return;
	}

	switch(number){
		case PUSH_ES_DATA:
			deca_push_es_test();
			break;
		case PLAY_MP3:
			deca_play_mp3();
			break;
		case PLAY_PCM:
			deca_play_pcm();
			break;
		default:
			break;
	}
}

static PARSE_COMMAND_LIST decadbg_command[] = {
	{ { NULL, NULL }, deca_dbg_show_help,  deca_dbg_no_para_preview,   "help",  "h",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, deca_dbg_start,     deca_dbg_no_para_preview,  "start", "s",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, deca_dbg_pause,      deca_dbg_no_para_preview,  "pause", "p",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, deca_dbg_exit,       deca_dbg_no_para_preview,   "exit",  "e",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, deca_dbg_show_info,  deca_dbg_info_preview,   "info",  "i",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, deca_dbg_switch_on,  deca_dbg_swi_preview,    "open",    "op",   0, 0, NULL, 0, 0, NULL, 0, 0 },
    { { NULL, NULL }, deca_dbg_switch_off, deca_dbg_swi_preview,    "close",    "cl",   0, 0, NULL, 0, 0, NULL, 0, 0 },   
	{ { NULL, NULL }, deca_dbg_set_intv,   deca_dbg_num_preview,    "interval","terv", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, deca_dbg_set_tms,    deca_dbg_num_preview,    "times", NULL,   0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, deca_dbg_module_test,    deca_dbg_num_preview,    "test", "t",   0, 0, NULL, 0, 0, NULL, 0, 0 },
};

static PARSE_COMMAND_LIST decadbg_app_command[] = {
	{ { NULL, NULL }, deca_app_show_help,  deca_dbg_no_para_preview,   "help",  "h",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, deca_dbg_start,     deca_dbg_no_para_preview,  "start", "s",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, deca_dbg_pause,      deca_dbg_no_para_preview,  "pause", "p",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, deca_dbg_exit,       deca_dbg_no_para_preview,   "exit",  "e",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, deca_dbg_show_info,  deca_dbg_info_preview,   "info",  "i",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, deca_dbg_switch_on,  deca_dbg_swi_preview,    "open",    "op",   0, 0, NULL, 0, 0, NULL, 0, 0 },
    //{ { NULL, NULL }, deca_dbg_switch_off, deca_dbg_swi_preview,    "close",    "cl",   0, 0, NULL, 0, 0, NULL, 0, 0 },   
	//{ { NULL, NULL }, deca_dbg_set_intv,   deca_dbg_num_preview,    "interval","terv", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, deca_dbg_set_tms,    deca_dbg_num_preview,    "times", NULL,   0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, deca_dbg_module_test,    deca_dbg_num_preview,    "test", "t",   0, 0, NULL, 0, 0, NULL, 0, 0 },
};

void decadbg_module_register(void)
{             
	deca_dbg_init();
	debug_module_add("deca", &decadbg_command[0], ARRAY_SIZE(decadbg_command));
}

INT32 deca_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &decadbg_command[0];
	*list_cnt = ARRAY_SIZE(decadbg_command);

	if (!DecaDbgCtrl.dbg_init)
       deca_dbg_init();

	return RET_SUCCESS;
}

INT32 deca_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &decadbg_app_command[0];
	*list_cnt = ARRAY_SIZE(decadbg_app_command);

	if (!DecaDbgCtrl.dbg_init)
       deca_dbg_init();

	return RET_SUCCESS;
}
