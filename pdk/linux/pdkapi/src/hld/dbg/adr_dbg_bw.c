/*****************************************************************************
*	Copyrights(C) 2012 Acer Laboratries inc. All Rights Reserved.
*
*	FILE NAME:		adr_dbg_bw.c
*
*	DESCRIPTION:	Band width debug module.
*
*	HISTORY:
*						Date 	 Author      Version 	  Notes
*					=========	=========	=========	===========
*					2012-12-3	 Leo.Ma      Ver 1.0	Create file.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <osal/osal_task.h>
#include <hld/adr_hld_dev.h>
#include <hld/dbg/adr_dbg_parser.h>

#include <linux/ali_test.h>
#include <hld_cfg.h>

#define __ADR_BW_DBG 

#ifdef __ADR_BW_DBG 
#define BW_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(BW, fmt, ##args); \
			} while(0)

#else
#define BW_DBG_PRINT(...)
#endif

#define BW_FD_PATH	"/dev/ali_m36_ip_bw_0"
#define BW_DBG_POLL_INTRV	20
#define BW_DBG_IP_FLAG	0xffffffff
#define BW_DBG_IP_FLAG_ADD	0x0000003
#define BW_DBG_CHAN_MODE	3
#define avr_inc(x, y, z)	(((x) >= (y)) ? (((x) - (y)) / (++(z))) : (-(((y) - (x)) / (++(z)))))

typedef enum bw_dbg_ip_mode {
	BW_DBG_TOTAL_IP,
	BW_DBG_SINGLE_IP = 1,
	BW_DBG_BOTH_TOT_SIG_IP,
} BW_DBG_IP_MODE_E;

typedef enum bw_dbg_chan_mode {
	BW_DBG_SINGLE_CHAN,
	BW_DBG_DOUBLE_CHAN = 1,
	BW_DBG_FIRST_CHAN,
	BW_DBG_SECOND_CHAN,
} BW_DBG_CHAN_MODE_E;

typedef struct bw_dbg_info {
	int fd;
	UINT32 BwDbgInit;
	INT32 TestCnt;
	BW_DBG_CHAN_MODE_E chan_mode;
	struct test_ip_bw_cfg_info bw_config;
} BW_DBG_INFO_S;

/*
static const char * IP_NAME[] =
{
	"Total",
	"Main CPU",
    "ALI CPU",
    "DE Video",
    "DE Graphic",
    "VE Long",
    "VE Short",
    "Audio",
    "SFlash",
    "Demux1",
    "TsGen",
    "Nand Flash",
    "VIN Do1 Sut",
    "USB2",
    "Descramble",
    "BDMA",
    "SGDMA1",
    "GE",
    "Demux2",
    "TOE",
    "DVBC PHY1",
    "USB1",
    "Deo0 Vin1",
    "SGDMA2",
	"DVBC PHY2"
};
*/

static const char * IP_NAME[35] = 
{
	"TOTAL_IP",
	"MAIN_CPU",
    "ALI_CPU",
    "DE_FXDE0",
    "DE_FXDE1",
    "AUDIO_CPU0",
    "AUDIO_CPU1",
    "AUDIO",
    "PPV",
    "DEMUX1",
    "TSGEN",
    "Nand Flash",
    "VDEC Long",
    "USB0",
    "VENC Long",
    "BDMA",
    "VDEC_CTRL",
    "VDEC SHORT",
    "MALI400",
    "MAC",
	"DESCRAMBLE",
	"USB1",
	"VENC_Short",
    "DVBC PHY1",
    "MAC2",
    "DEMUX2",
    "DE_FXDE3",
	"VENC_CTRL",
	"SGDMA1",
    "SGDMA2",
    "DE_FXDE2",
    "GE_GC",
    "DEMUX3",
    "USB2",   
    "SDIO"   
};

static BW_DBG_INFO_S BwDbgInfo;

static INT32 BwDbgFdOpen()
{
	INT32 Ret;

    BwDbgInfo.fd = open(BW_FD_PATH, O_RDONLY | O_CLOEXEC);

	if (BwDbgInfo.fd < 0)
    {		
        return RET_FAILURE;
    }	
	
	return RET_SUCCESS;
}

static void BwDbgFdClose()
{
	if (BwDbgInfo.fd > 0)
    {
		close(BwDbgInfo.fd);
    }

	memset(&BwDbgInfo, 0, sizeof(BwDbgInfo));

	return;
}

static void bw_dbg_init(void)
{
	if (!BwDbgInfo.BwDbgInit)
	{
		memset(&BwDbgInfo, 0, sizeof(BwDbgInfo));
		BwDbgInfo.BwDbgInit = TRUE;
		BwDbgInfo.bw_config.ip_enable_flag = BW_DBG_IP_FLAG;
		BwDbgInfo.bw_config.ip_enable_flag_add= BW_DBG_IP_FLAG_ADD;
		
		BwDbgInfo.bw_config.ip_mode = BW_DBG_BOTH_TOT_SIG_IP;
		BwDbgInfo.bw_config.time_gap = BW_DBG_POLL_INTRV;
		BwDbgInfo.chan_mode = BW_DBG_DOUBLE_CHAN;
		BwDbgInfo.TestCnt = 5;
		
		if (BwDbgFdOpen() != RET_SUCCESS)
		{
			//BW_DBG_PRINT("Band width debug open fail!\n");
			return;
		}
	}

	return;
}

static void bw_dbg_exit(int argc, char **argv)
{
	BwDbgFdClose();
	BW_DBG_PRINT("Band width debug closed.\n");
	return;
}

static void bw_dbg_start(int argc, char **argv)
{
	INT32 Ret;

	Ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_TASK_START, 0);

    if (Ret < 0)
    {
        BW_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);
    }

	return;
}

static void bw_dbg_pause(int argc, char **argv)
{
	INT32 Ret;
	
	Ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_TASK_STOP, 0);

    if (Ret < 0)
    {
        BW_DBG_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);
    }
	
	return;
}

static void bw_dbg_show_help(int argc, char **argv)
{
	BW_DBG_PRINT("\nModule name:\n");
	BW_DBG_PRINT("\tbw\n");
	BW_DBG_PRINT("\nModule Description:\n");
	BW_DBG_PRINT("\tIP band width testing debug module.\n");	
	BW_DBG_PRINT("\nSyntax:\n");
	BW_DBG_PRINT("\tbw -option [arguments]\n");
	BW_DBG_PRINT("\nDescription:\n");
	BW_DBG_PRINT("\t-h, --help\n");
	BW_DBG_PRINT("\t\tSelf description of this debugging moduld.\n");
	BW_DBG_PRINT("\t-s, --start\n");
	BW_DBG_PRINT("\t\tStart IP band width testing task.\n");
	BW_DBG_PRINT("\t-p, --pause\n");
	BW_DBG_PRINT("\t\tStop IP band width testing task.\n");
	BW_DBG_PRINT("\t-e, --exit\n");
	BW_DBG_PRINT("\t\tExit band width debugging module.\n");
	BW_DBG_PRINT("\t-i, --info\n");
	BW_DBG_PRINT("\t\tShow IP band width testing data.\n");
	BW_DBG_PRINT("\t-ipm number\n");
	BW_DBG_PRINT("\t\tConfig IP mode(The default value is \"0\").\n");
	BW_DBG_PRINT("\t\t0\t--Total IP mode.\n");
	BW_DBG_PRINT("\t\t1\t--Single IP mode.\n");
	BW_DBG_PRINT("\t\t2\t--Both total and single IP mode.\n");
	BW_DBG_PRINT("\t-chm number\n");
	BW_DBG_PRINT("\t\tConfig IP channel mode(The default value is \"1\").\n");
	BW_DBG_PRINT("\t\t0\t--Single channel mode.\n");
	BW_DBG_PRINT("\t\t1\t--Double channel mode.\n");
	BW_DBG_PRINT("\t\t2\t--The first channel in double channel mode.\n");
	BW_DBG_PRINT("\t\t3\t--The second channel in double channel mode.\n");
	BW_DBG_PRINT("\t-f, --flag number\n");
	BW_DBG_PRINT("\t\tConfig single IP band width testing enable flag(The default value is \"0x1\").\n");
	BW_DBG_PRINT("\t-t number\n");
	BW_DBG_PRINT("\t\tConfig IP band width testing time interval(The default value is \"20\").\n");
	BW_DBG_PRINT("\nUsage example:\n");
	BW_DBG_PRINT("\tbw -h[--help]\n");
	BW_DBG_PRINT("\tbw -s[--start]\n");
	BW_DBG_PRINT("\tbw -p[--pause]\n");
	BW_DBG_PRINT("\tbw -e[--exit]\n");
	BW_DBG_PRINT("\tbw -i[--info]\n");
	BW_DBG_PRINT("\tbw -ipm 2\n");
	BW_DBG_PRINT("\tbw -chm 1\n");	
	BW_DBG_PRINT("\tbw -f[--flag] 0xffffff\n");
	BW_DBG_PRINT("\tbw -t 20\n");

	return;
}

static void bw_dbg_show_info(int argc, char **argv)
{
	INT32					ret, k;
	struct test_ip_get_bw	bw_stat;
	UINT32					j;
	INT32 					i;
    UINT32					bw_avr[BW_DBG_CHAN_MODE][MAX_IP_IDX];
	UINT32					bw_cnt[BW_DBG_CHAN_MODE][MAX_IP_IDX];
    UINT32					bw_avr2[BW_DBG_CHAN_MODE][MAX_IP_IDX];
	UINT32					bw_cnt2[BW_DBG_CHAN_MODE][MAX_IP_IDX];

	memset(bw_avr2, 0, sizeof(bw_avr2));
	memset(bw_cnt2, 0, sizeof(bw_cnt2));
	memset(&bw_stat, 0, sizeof(bw_stat));
	bw_stat.ip_mode = BwDbgInfo.bw_config.ip_mode;
	bw_stat.ip_idx_flag = BwDbgInfo.bw_config.ip_enable_flag;
	bw_stat.ip_idx_flag_add= BwDbgInfo.bw_config.ip_enable_flag_add;
	
	bw_stat.ip_chan_mode = BwDbgInfo.chan_mode;
	
	BW_DBG_PRINT("\nTotal %d times test.\n", BwDbgInfo.TestCnt);

	ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_TASK_PAUSE, 0);
	if (ret < 0)
	{
		BW_DBG_PRINT("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);
		return;
	}

	ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_BW_CONFIG, &BwDbgInfo.bw_config);
	if (ret < 0)
	{
	    BW_DBG_PRINT("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);
		return;
	}

	osal_task_sleep(2000);

	ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_TASK_RESUME, 0);
	if (ret < 0)
	{
	    BW_DBG_PRINT("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);
		return;
	}

	for (k = 0; k < BwDbgInfo.TestCnt; k++)
	{	
		osal_task_sleep(2000);

		ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_BW_GET, &bw_stat);   //
				
		if (ret < 0)
		{
	        BW_DBG_PRINT("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);
			return;
		}

		BW_DBG_PRINT("\nTest ");
		switch (k + 1)
		{
			case 1:
				BW_DBG_PRINT("%ust", k + 1);
				break;
			case 2:
				BW_DBG_PRINT("%und", k + 1);
				break;
			case 3:
				BW_DBG_PRINT("%urd", k + 1);
				break;
			default:
				BW_DBG_PRINT("%uth", k + 1);
				break;
		}
		BW_DBG_PRINT(" time...\n");

		memset(bw_avr, 0, sizeof(bw_avr));
		memset(bw_cnt, 0, sizeof(bw_cnt));
	
		for(i = 0; i < MAX_IP_IDX; i++)
		{
			if (0 == i && (!(BW_DBG_TOTAL_IP == bw_stat.ip_mode || BW_DBG_BOTH_TOT_SIG_IP == bw_stat.ip_mode)))
				continue;
				
			if (i > 0 && (!(BW_DBG_SINGLE_IP == bw_stat.ip_mode || BW_DBG_BOTH_TOT_SIG_IP == bw_stat.ip_mode)))
				continue;

			if (i >= 0 && (BW_DBG_SINGLE_IP == bw_stat.ip_mode || BW_DBG_BOTH_TOT_SIG_IP == bw_stat.ip_mode))
			{
				if(i<32)
				{
					if(!(bw_stat.ip_idx_flag&(1 << i)))
						continue;
				}
				else
				{
					if(!((bw_stat.ip_idx_flag_add+0x04)&(1 << (i - 32)))) // include Total IP
						continue;
				}
			}
			
			if (bw_stat.ip_bw_cnt > MAX_BW_GROUP)
				bw_stat.ip_bw_cnt = MAX_BW_GROUP;
			for (j = 0; j < bw_stat.ip_bw_cnt; j++)
			{	
				if (bw_stat.bw_info[i].ip_bw_jiff[j] != 0)
				{
					bw_avr[0][i] += avr_inc(bw_stat.bw_info[i].ip_bw_value[j], bw_avr[0][i], bw_cnt[0][i]);
					bw_avr[1][i] += avr_inc(bw_stat.bw_info[i].ip_bw_value_0[j], bw_avr[1][i], bw_cnt[1][i]);
					bw_avr[2][i] += avr_inc(bw_stat.bw_info[i].ip_bw_value_1[j], bw_avr[2][i], bw_cnt[2][i]);
				}
			}
			bw_avr2[0][i] += avr_inc(bw_avr[0][i], bw_avr2[0][i], bw_cnt2[0][i]);
			bw_avr2[1][i] += avr_inc(bw_avr[1][i], bw_avr2[1][i], bw_cnt2[1][i]);
			bw_avr2[2][i] += avr_inc(bw_avr[2][i], bw_avr2[2][i], bw_cnt2[2][i]);
		}
	}

	for(i = 0; i < MAX_IP_IDX; i++)
	{
		if (0 == i && (!(BW_DBG_TOTAL_IP == bw_stat.ip_mode || BW_DBG_BOTH_TOT_SIG_IP == bw_stat.ip_mode)))
			continue;
			
		if (i > 0 && (!(BW_DBG_SINGLE_IP == bw_stat.ip_mode || BW_DBG_BOTH_TOT_SIG_IP == bw_stat.ip_mode)))
			continue;

		if (i >= 0 &&(BW_DBG_SINGLE_IP == bw_stat.ip_mode || BW_DBG_BOTH_TOT_SIG_IP == bw_stat.ip_mode))
		{
			if(i<32)
			{
				if(!(bw_stat.ip_idx_flag&(1 << i)))
					continue;
			}
			else
			{
				if(!((bw_stat.ip_idx_flag_add + 0x04)&(1 << (i - 32))))  // include Total IP
					continue;
			}
		}
		//BW_DBG_PRINT("\n%s IP BW[%u KBps], CH_0[%u KBps], CH_1[%u KBps]\n", IP_NAME[i], bw_avr2[0][i], bw_avr2[1][i], bw_avr2[2][i]);
		BW_DBG_PRINT("\n%15s IP BW = %8u KBps,     CH_0 = %8u KBps,     CH_1 = %8u KBps\n", IP_NAME[i], bw_avr2[0][i], bw_avr2[1][i], bw_avr2[2][i]);
	}
	
	return;
}

static ARG_CHK bw_dbg_no_para_preview(int argc, char **argv, char *option)
{
	if (argc != 0)
	{
		BW_DBG_PRINT("Option \"%s\": Should not be with any argument(s)!\n", option);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}

static void bw_dbg_chan_mode_set(int argc, char **argv)
{
	INT32 number, ret;

	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0 || number > BW_DBG_CHAN_MODE)
	{
		BW_DBG_PRINT("Argument \"%s\" not a number or out of range 0-3!\n", *argv);
		return;
	}

	if (!BwDbgInfo.BwDbgInit)
		bw_dbg_init();	
	
	BwDbgInfo.chan_mode = number;

	return;
}

static ARG_CHK bw_dbg_chan_mode_preview(int argc, char **argv, char *option)
{
	int number, ret;

	if (0 == argc)
	{
		BW_DBG_PRINT("Option \"%s\": Lack of valid argument(ex: number)!\n", option);
		return DBG_ARG_INVALID;
	}

	if (argc != 1)
	{
		BW_DBG_PRINT("Option \"%s\": Only one argument admitted!\n", option);
		return DBG_ARG_INVALID;
	}

	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0 || number > BW_DBG_CHAN_MODE)
	{
		BW_DBG_PRINT("Option \"%s\": Argument \"%s\" not a number or out of range 0-3!\n", option, *argv);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}

static void bw_dbg_ip_mode_set(int argc, char **argv)
{
	INT32 number, ret;

	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0 || number > BW_DBG_BOTH_TOT_SIG_IP)
	{
		BW_DBG_PRINT("Argument \"%s\" not a number or out of range 0-2!\n", *argv);
		return;
	}

	if (!BwDbgInfo.BwDbgInit)
		bw_dbg_init();
	
	BwDbgInfo.bw_config.ip_mode = number;

	ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_TASK_PAUSE, 0);
	if (ret < 0)
	{
		BW_DBG_PRINT("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);
		return;
	}

	ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_BW_CONFIG, &BwDbgInfo.bw_config);
    if (ret < 0)
    {
        BW_DBG_PRINT("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);
    }

	ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_TASK_RESUME, 0);
	if (ret < 0)
	{
        BW_DBG_PRINT("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);
		return;
	}

	return;
}

static ARG_CHK bw_dbg_ip_mode_preview(int argc, char **argv, char *option)
{
	int number, ret;

	if (0 == argc)
	{
		BW_DBG_PRINT("Option \"%s\": Lack of valid argument(ex: number)!\n", option);
		return DBG_ARG_INVALID;
	}

	if (argc != 1)
	{
		BW_DBG_PRINT("Option \"%s\": Only one argument admitted!\n", option);
		return DBG_ARG_INVALID;
	}
	
	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0 || number > BW_DBG_BOTH_TOT_SIG_IP)
	{
		BW_DBG_PRINT("Option \"%s\": Argument \"%s\" not a number or out of range 0-2!\n", option, *argv);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}

static void bw_dbg_set_intv(int argc, char **argv)
{
	INT32 number, ret;

	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		BW_DBG_PRINT("Argument \"%s\" not a number or less than zero!\n", *argv);
		return;
	}

	if (!BwDbgInfo.BwDbgInit)
		bw_dbg_init();
	
	BwDbgInfo.bw_config.time_gap = number;

	ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_TASK_PAUSE, 0);
	if (ret < 0)
	{
		BW_DBG_PRINT("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);
		return;
	}

	ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_BW_CONFIG, &BwDbgInfo.bw_config);
    if (ret < 0)
    {
        BW_DBG_PRINT("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);
    }

	ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_TASK_RESUME, 0);
	if (ret < 0)
	{
        BW_DBG_PRINT("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);
		return;
	}

	return;
}

static void bw_dbg_set_tms(int argc, char **argv)
{
	INT32 number, ret;

	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		BW_DBG_PRINT("Argument \"%s\" not a number or less than zero!\n", *argv);
		return;
	}

	if (!BwDbgInfo.BwDbgInit)
		bw_dbg_init();
	
	BwDbgInfo.TestCnt = number;

	return;
}

static ARG_CHK bw_dbg_num_preview(int argc, char **argv, char *option)
{
	int number, ret;

	if (0 == argc)
	{
		BW_DBG_PRINT("Option \"%s\": Lack of valid argument(ex: number)!\n", option);
		return DBG_ARG_INVALID;
	}

	if (argc != 1)
	{
		BW_DBG_PRINT("Option \"%s\": Only one argument admitted!\n", option);
		return DBG_ARG_INVALID;
	}
	
	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		BW_DBG_PRINT("Option \"%s\": Argument \"%s\" not a number or less than zero!\n", option, *argv);
		return DBG_ARG_INVALID;
	}
	
	return DBG_ARG_VALID;
}

static void bw_dbg_ip_flag_set(int argc, char **argv)
{
	INT32 number, ret;

	ret = sscanf(*argv, "%x", &number);
	if (0 == ret)
	{
		BW_DBG_PRINT("Argument \"%s\" not a hexadecimal number!\n", *argv);
		return;
	}

	if (!BwDbgInfo.BwDbgInit)
		bw_dbg_init();
	
	BwDbgInfo.bw_config.ip_enable_flag = number;   //

	ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_TASK_PAUSE, 0);
	if (ret < 0)
	{
		BW_DBG_PRINT("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);
		return;
	}

	ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_BW_CONFIG, &BwDbgInfo.bw_config);
    if (ret < 0)
    {
        BW_DBG_PRINT("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);
    }

	ret = ioctl(BwDbgInfo.fd, ALI_TEST_IP_TASK_RESUME, 0);
	if (ret < 0)
	{
        BW_DBG_PRINT("RET=%d %s,%d,%m\n", ret, __FUNCTION__, __LINE__, errno);
		return;
	}

	return;
}

static ARG_CHK bw_dbg_flag_preview(int argc, char **argv, char *option)
{
	int number, ret;

	if (0 == argc)
	{
		BW_DBG_PRINT("Option \"%s\": Lack of valid argument(ex: number)!\n", option);
		return DBG_ARG_INVALID;
	}

	if (argc != 1)
	{
		BW_DBG_PRINT("Option \"%s\": Only one argument admitted!\n", option);
		return DBG_ARG_INVALID;
	}
	
	if (!('0' == *argv[0] && 'x' == (*argv[1] | 0x20) && isxdigit(*argv[2])))
	{
		BW_DBG_PRINT("Option \"%s\": Argument should be hexadecimal number(ex: 0x1)!\n", option);
		return DBG_ARG_INVALID;
	}

	ret = sscanf(*argv, "%x", &number);
	if (0 == ret)
	{
		BW_DBG_PRINT("Option \"%s\": Argument \"%s\" not a hexadecimal number!\n", option, *argv);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}

static PARSE_COMMAND_LIST bwdbg_command[] = {
	{ { NULL, NULL }, bw_dbg_start, bw_dbg_no_para_preview, "start", "s", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, bw_dbg_pause, bw_dbg_no_para_preview, "pause", "p", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, bw_dbg_show_help, bw_dbg_no_para_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, bw_dbg_show_info, bw_dbg_no_para_preview, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, bw_dbg_ip_flag_set, bw_dbg_flag_preview, "flag", "f", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, bw_dbg_ip_mode_set, bw_dbg_ip_mode_preview, NULL, "ipm", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, bw_dbg_chan_mode_set, bw_dbg_chan_mode_preview, NULL, "chm", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, bw_dbg_set_intv, bw_dbg_num_preview, NULL, "t", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, bw_dbg_set_tms, bw_dbg_num_preview, NULL, "tms", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, bw_dbg_exit, bw_dbg_no_para_preview, "exit", "e", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};

INT32 bw_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &bwdbg_command[0];
	*list_cnt = ARRAY_SIZE(bwdbg_command);

	if (!BwDbgInfo.BwDbgInit)
		bw_dbg_init();	

	return RET_SUCCESS;
}

INT32 bw_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	return bw_dbg_cmd_get(cmd_list, list_cnt);
}
