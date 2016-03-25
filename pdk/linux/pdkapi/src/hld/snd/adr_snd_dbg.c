/*****************************************************************************
*   Copyrights(C) 2012 ALi corp. All Rights Reserved.
*
*   FILE NAME:      adr_snd_dbg.c
*
*   DESCRIPTION:    Snd debug module.
*
*   HISTORY:
*                      Date      Author      Version      Notes
*                   ==========  =========   =========   ===========
*                   2012-10-24    =.=        Ver 1.0    Create File
*****************************************************************************/
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include <osal/osal_task.h>

#include <hld/adr_hld_dev.h>
#include <adr_mediatypes.h>

#include <osal/osal.h>
#include <hld/adr_hld_dev.h>
#include <hld/deca/adr_deca_dev.h>
#include <hld/deca/adr_deca.h>
#include <hld/snd/adr_snd_dev.h>
#include <hld/snd/adr_snd.h>

#include <hld/dbg/adr_dbg_parser.h>
#include <hld_cfg.h>


#define __ADR_SND_DBG

#ifdef __ADR_SND_DBG
#define SND_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(SND, fmt, ##args); \
			} while(0)

#else
#define SND_DBG_PRINT(...)
#endif

#define SND_DBG_PRINT_INTRV 5000

typedef struct snd_dbg_stat_ctrl
{
	int      dbg_dev_fd;
	UINT32   dbg_task_id;
	struct snd_device *dbg_dev;
    
	UINT8    dbg_init;
	UINT8    dbg_enable;
} snd_dbg_stat_ctrl;

static snd_dbg_stat_ctrl SndDbgCtrl;

static void snd_dbg_init(void)
{
	memset(&SndDbgCtrl, 0, sizeof(snd_dbg_stat_ctrl));

	SndDbgCtrl.dbg_dev = (struct snd_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_SND);
	if(SndDbgCtrl.dbg_dev == NULL)
	{
		//SND_DBG_PRINT("get snd device failure!\n");
		return;
	}

	//SND_DBG_PRINT("Snd debug statistics opened.\n");
}

static void snd_dbg_show_help(int argc, char **argv)
{
	SND_DBG_PRINT("\nModule name:\n");
	SND_DBG_PRINT("\tsnd\n");
	SND_DBG_PRINT("\nModule Description:\n");
	SND_DBG_PRINT("\nSnd diagnostic debugging module.\n");	
	SND_DBG_PRINT("\nSyntax:\n");
	SND_DBG_PRINT("\tsnd -option [arguments]...\n");
	SND_DBG_PRINT("\nDescription:\n");
	SND_DBG_PRINT("\t-h, --help\n");
	SND_DBG_PRINT("\t\tSelf description of this debugging moduld.\n");
    
	SND_DBG_PRINT("\t-i, --info\n");
	SND_DBG_PRINT("\t\tShow Snd information once, such as device flags,sample rate, pcm buf.\n");

 	SND_DBG_PRINT("\t-n items\n");
	SND_DBG_PRINT("\t\tEnable See debugging printout.\n");
	SND_DBG_PRINT("\t\tall\t--All the items.\n");
	SND_DBG_PRINT("\t\tapi\t--SEE Snd Api printout\n");
	SND_DBG_PRINT("\t\tavsync\t--SEE Snd avsync,realtime,\"d\":Drop Frames,\"p\":Play Frames \n");
	SND_DBG_PRINT("\t\tbuf\t--SEE Snd Buf printout,realtime.\n");

 	SND_DBG_PRINT("\t-d items\n");
	SND_DBG_PRINT("\t\tDisable See debugging printout.\n");
	SND_DBG_PRINT("\t\tall\t--All the items.\n");
	SND_DBG_PRINT("\t\tapi\t--SEE Snd Api printout\n");
	SND_DBG_PRINT("\t\tavsync\t--SEE Snd avsync,realtime,\"d\":Drop Frames,\"p\":Play Frames\n");
	SND_DBG_PRINT("\t\tbuf\t--SEE Snd Buf printout,realtime.\n");           
    
	SND_DBG_PRINT("\nUsage example:\n");
	SND_DBG_PRINT("\tsnd -h[--help]\n");
	SND_DBG_PRINT("\tsnd -i[--info]\n");
    SND_DBG_PRINT("\tsnd -n [all] [api] [avsync] [buf]\n");
    SND_DBG_PRINT("\tsnd -d [all] [api] [avsync] [buf]\n");  

	return;
}

static void snd_show_dev_info(int argc, char **argv)
{
      SND_DBG_PRINT("####################Snd Dev Status######################\n");
      struct snd_dev_status snd_status;
      memset(&snd_status,0,sizeof(struct snd_dev_status));
      snd_io_control(SndDbgCtrl.dbg_dev,SND_GET_STATUS,(UINT32)&snd_status);
      
      switch(snd_status.flags) {
        case SND_STATE_DETACH:
            SND_DBG_PRINT("Snd dev flags : DETACH\n");
            break;
        case SND_STATE_ATTACH:
            SND_DBG_PRINT("Snd dev flags : ATTACH\n");
            break;            
        case SND_STATE_IDLE:
            SND_DBG_PRINT("Snd dev flags : IDLE\n");
            break;
        case SND_STATE_PLAY:
            SND_DBG_PRINT("Snd dev flags : PLAY\n");
            break;      
        default:
            SND_DBG_PRINT("Snd in UNKNOW status!!flag %d\n");
            break;
        } 
        SND_DBG_PRINT("Volume : %d\n",snd_status.volume);

        SND_DBG_PRINT("Mute : %s\n",snd_status.in_mute?"True":"False");
        SND_DBG_PRINT("Spdif out : %s\n",snd_status.spdif_out?"Enable":"Disable");
        switch(snd_status.trackmode) {
            case SND_DUP_NONE:
                SND_DBG_PRINT("Track mode : Sterero\n");
                break;
            case SND_DUP_L:
                SND_DBG_PRINT("Track mode : Left Channel\n");
                break;
            case SND_DUP_R:
                SND_DBG_PRINT("Track mode : Right Channel\n");
                break;   
            case SND_DUP_MONO:
                SND_DBG_PRINT("Track mode : Mono\n");
                break;       
             default:
                SND_DBG_PRINT("Track mode error\n");
                break;
        }
        
        SND_DBG_PRINT("Config sample_rate : %d\n",snd_status.samp_rate);
        SND_DBG_PRINT("Config sample_num : %d\n",snd_status.samp_num);
        SND_DBG_PRINT("Config ch_num  : %d\n",snd_status.ch_num);

        SND_DBG_PRINT("Pcm drop cnts : %d\n",snd_status.drop_cnt);
        SND_DBG_PRINT("Pcm play cnts : %d\n",snd_status.play_cnt);
        SND_DBG_PRINT("Pcm Udr cnts : %d \n",snd_status.underrun_cnts);

   //      SND_DBG_PRINT("pcm_out_frames : %d\n",snd_status.pcm_out_frames);

    
         SND_DBG_PRINT("Pcm_dma_base :0x%08x\n",snd_status.pcm_dma_base);
         SND_DBG_PRINT("Pcm_dma_len : %d\n",snd_status.pcm_dma_len); 
         SND_DBG_PRINT("Pcm rd 0x%08x wt 0x%08x used percent %d\%\n",snd_status.pcm_rd,snd_status.pcm_wt,
            (snd_status.pcm_rd<snd_status.pcm_wt)?(snd_status.pcm_wt - snd_status.pcm_rd)*100/snd_status.pcm_dma_len:
            (snd_status.pcm_dma_len-snd_status.pcm_rd +snd_status.pcm_wt)*100/snd_status.pcm_dma_len);
}


static ARG_CHK snd_dbg_no_para_preview(int argc, char **argv, char *option)
{
	if (argc != 0)
	{
		SND_DBG_PRINT("Option \"%s\": Should not be with any argument(s)!\n", option);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}

/*!
@brief  一些issue 需要打开see的打印来定位问题
*/
//following used to enable see print,more useful
UINT32 snd_dbg_see_level = DBG_SND_NO_OUTPUT;
static void snd_dbg_see_enable(int argc, char **argv)
{       
	if (0 == argc)
		goto SEE_EN_ALL;

	while (argc-- > 0)
	{
		if (!strcmp(*argv, "all"))
		{
SEE_EN_ALL:
			snd_dbg_see_level = DBG_SND_API | DBG_SND_AVSYNC |DBG_SND_BUF;
			SND_DBG_PRINT("Enable see snd all\n");
		}
		else if (!strcmp(*argv, "avsync"))
		{
			snd_dbg_see_level |= DBG_SND_AVSYNC;
			SND_DBG_PRINT("Enable see snd avsync dbg\n");
		}
		else if (!strcmp(*argv, "api"))
		{
			snd_dbg_see_level |= DBG_SND_API;    
			SND_DBG_PRINT("Enable see api\n");
		}
		else if (!strcmp(*argv, "buf"))
		{
			snd_dbg_see_level |= DBG_SND_BUF;   
			SND_DBG_PRINT("Enable see buf\n");
		}
		else
		{
			SND_DBG_PRINT("Unrecognized argument(s)!\n");
		}
	}    

	snd_set_dbg_level(SndDbgCtrl.dbg_dev,snd_dbg_see_level);

	return;
}

static void snd_dbg_see_disable(int argc, char **argv)
{
	if (0 == argc)
		goto SEE_DIS_ALL;

	while (argc-- > 0)
	{
		if (!strcmp(*argv, "all"))
		{
SEE_DIS_ALL:
			snd_dbg_see_level &=~ (DBG_SND_API | DBG_SND_AVSYNC|DBG_SND_BUF);
			SND_DBG_PRINT("Disable see all\n");
		}
		else if (!strcmp(*argv, "api"))
		{
			snd_dbg_see_level &= ~DBG_SND_API;
			SND_DBG_PRINT("Disable see api\n");
		}
		else if (!strcmp(*argv, "avsync"))
		{
			snd_dbg_see_level &= ~DBG_SND_AVSYNC;    
			SND_DBG_PRINT("Disable see avsync dbg\n");
		}
		else if (!strcmp(*argv, "buf"))
		{
			snd_dbg_see_level &= ~DBG_SND_BUF;   
			SND_DBG_PRINT("Disable see buf\n");
		}
		else
		{
			SND_DBG_PRINT("Unrecognized argument(s)!\n");
		}

		argv++;
	}    
	snd_set_dbg_level(SndDbgCtrl.dbg_dev,snd_dbg_see_level);

	return;
}

static ARG_CHK snd_dbg_see_preview(int argc, char **argv, char *option)
{
	while (argc-- > 0)
	{
		if (strcmp(*argv, "all") && 
			strcmp(*argv, "avsync") && 
			strcmp(*argv, "api") && 
			strcmp(*argv, "buf")) 
		{
			SND_DBG_PRINT("Option \"%s\": Unrecognized argument(s) \"%s\"!\n", option, *argv);
			return DBG_ARG_INVALID;
		}

		argv++;
	}

	return DBG_ARG_VALID;
}

static PARSE_COMMAND_LIST snddbg_command[] = {
	{ { NULL, NULL }, snd_dbg_show_help,  snd_dbg_no_para_preview,   "help",  "h",    0, 0, NULL, 0, 0, NULL, 0, 0 },
 	{ { NULL, NULL }, snd_show_dev_info,     snd_dbg_no_para_preview  ,   "info",  "i",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, snd_dbg_see_enable,    snd_dbg_see_preview,    "seeEnable",   "n",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, snd_dbg_see_disable,    snd_dbg_see_preview,    "seeDisable",  "d",    0, 0, NULL, 0, 0, NULL, 0, 0 },        
};

void snddbg_module_register(void)
{            
    snd_dbg_init();
	debug_module_add("snd", &snddbg_command[0], ARRAY_SIZE(snddbg_command));
}

INT32 snd_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &snddbg_command[0];
	*list_cnt = ARRAY_SIZE(snddbg_command);

	snd_dbg_init();

	return RET_SUCCESS;
}

INT32 snd_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	return snd_dbg_cmd_get(cmd_list, list_cnt);
}
