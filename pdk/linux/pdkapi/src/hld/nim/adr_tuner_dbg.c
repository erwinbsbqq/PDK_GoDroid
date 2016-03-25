/*****************************************************************************
*   Copyrights(C) 2012 ALi corp. All Rights Reserved.
*
*   FILE NAME:      adr_tuner_dbg.c
*
*   DESCRIPTION:    Tuner debug module.
*
*   HISTORY:
*                      Date      Author      Version      Notes
*                   ==========  =========   =========   ===========
*                   2012-10-16  Corey.Chi    Ver 1.0    Create File
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

#include <linux/dvb/frontend.h>
#include <osal/osal_task.h>

#include <hld/adr_hld_dev.h>
#include <hld/nim/adr_nim_dev.h>
#include <hld/dbg/adr_dbg_parser.h>
#include "adr_tuner_dbg.h"

#define __ADR_TUNER_DBG
#ifdef __ADR_TUNER_DBG
#define TUNER_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(TUNER, fmt, ##args); \
			} while(0)

#else
#define TUNER_DBG_PRINT(...)
#endif

static const UINT8 *QAM_TYPE[] = {
	"QAM16",
	"QAM32",
	"QAM64",
	"QAM128",
	"QAM256",
	"QAMNULL",
};

static tuner_dbg_stat_ctrl g_dbg_stat_ctrl;

static int tuner_dbg_device_open(void)
{
	unsigned int fd_cfg;
	if (g_dbg_stat_ctrl.dbg_dev_fd)
	{
		TUNER_DBG_PRINT("Tuner diagnostic debugging module has opened.\n");
	}
	else
	{
		g_dbg_stat_ctrl.dbg_dev_fd = open("/dev/ali_m3200_nim0", O_RDONLY | O_CLOEXEC);
		if (g_dbg_stat_ctrl.dbg_dev_fd < 0) 
		{    
			TUNER_DBG_PRINT("Tuner diagnostic debugging module open fail!\n");
			return (-1);
		}
	}	

	return g_dbg_stat_ctrl.dbg_dev_fd;
}

static void tuner_dbg_device_close(void)
{
	if (g_dbg_stat_ctrl.dbg_dev_fd > 0) 
	{    
		close(g_dbg_stat_ctrl.dbg_dev_fd);
	}    
	g_dbg_stat_ctrl.dbg_dev_fd = 0;
}

static void show_basic_info(void)
{
	struct nim_device *hld_dev = NULL;
	int fd = 0;
	UINT8 mode_array[32];
	
	hld_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
	if (NULL == hld_dev)
	{
	    TUNER_DBG_PRINT("NIM hld device is NULL!\n");
		return;
	}
	if ((hld_dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		TUNER_DBG_PRINT("NIM hld device not in work!\n");
		return;
	}
	
	fd = g_dbg_stat_ctrl.dbg_dev_fd;
	if (fd <= 0)
	{   
		TUNER_DBG_PRINT("Invalid tuner debug file descriptor.\n");
		return;
	}

	mode_array[0] = 0xBA;    //cmd head
	ioctl(fd, ALI_NIM_DRIVER_SET_MODE, mode_array);
	TUNER_DBG_PRINT("------------ ALi QAM Tuner Basic Info ------------\n");
	TUNER_DBG_PRINT("CoexistCnt  TunerIndex  TunerModel  DemodMode  DemodBand  I2CChnl  I2CAddr\n");
    TUNER_DBG_PRINT("%-12d", mode_array[5]);
	TUNER_DBG_PRINT("%-12d", mode_array[6]);
	TUNER_DBG_PRINT("%-12s", mode_array+8);
	TUNER_DBG_PRINT("%-11s", (1==mode_array[3])?"J83B":"J83A/C");
	TUNER_DBG_PRINT("%-11s", (0==mode_array[4])?"27M":"54M");
	TUNER_DBG_PRINT("%-9d", mode_array[1]);
	TUNER_DBG_PRINT("0x%-02X", mode_array[2]);
	TUNER_DBG_PRINT("\n");
	g_dbg_stat_ctrl.dbg_i2c_chnl = mode_array[1];
	g_dbg_stat_ctrl.dbg_i2c_addr = mode_array[2];
}

static void show_work_status(void)
{
	int fd = g_dbg_stat_ctrl.dbg_dev_fd;
	UINT32 freq, symb, qam, islock;
	UINT32 ber_val, snr_val, sig_val;
	
	if (fd <= 0)
	{   
		TUNER_DBG_PRINT("Invalid tuner debug file descriptor.\n");
		return;
	}
	freq    = ioctl(fd, ALI_NIM_READ_FREQ, 0);
	symb    = ioctl(fd, ALI_NIM_READ_SYMBOL_RATE, 0);
	qam     = ioctl(fd, ALI_NIM_READ_CODE_RATE, 0);
	islock  = ioctl(fd, ALI_NIM_GET_LOCK_STATUS, 0);
	ber_val = ioctl(fd, ALI_NIM_READ_QPSK_BER, 0);
	snr_val = ioctl(fd, ALI_NIM_GET_CN_VALUE, 0);
	sig_val = ioctl(fd, ALI_NIM_GET_RF_LEVEL, 0);
	TUNER_DBG_PRINT("------------ ALi QAM Tuner Status ------------\n");
	TUNER_DBG_PRINT("IsLock  Frequency  SymbRate  QamType  BER     SNR     SignalLevel\n");
	TUNER_DBG_PRINT("%-8s", islock?"Yes":"No");
	TUNER_DBG_PRINT("%-11u", freq*10);
	TUNER_DBG_PRINT("%-10u", symb);
	TUNER_DBG_PRINT("%-9s", (qam>3&&qam<9)?QAM_TYPE[qam-4]:QAM_TYPE[5]);
	TUNER_DBG_PRINT("%-8u", ber_val);
	TUNER_DBG_PRINT("%-8u", snr_val);
	TUNER_DBG_PRINT("%-u", sig_val);
	TUNER_DBG_PRINT("\n");
}

static int show_ber_monitor(void)
{
	int fd = g_dbg_stat_ctrl.dbg_dev_fd;
	UINT32 ber_count, per_count;
	
	if (fd <= 0)
	{
		TUNER_DBG_PRINT("Invalid tuner debug file descriptor.\n");
		return (-1);
	}
	ber_count = ioctl(fd, ALI_NIM_READ_QPSK_BER, 0);
	per_count = ioctl(fd, ALI_NIM_READ_RSUB, 0);
	TUNER_DBG_PRINT("[M] BER = %d\n", ber_count);
	TUNER_DBG_PRINT("[M] PER = %d\n", per_count);
	return 0;
}

static int show_snr_monitor(void)
{
	int fd = g_dbg_stat_ctrl.dbg_dev_fd;
	UINT32 snr_val;
	
	if (fd <= 0)
	{
		TUNER_DBG_PRINT("Invalid tuner debug file descriptor.\n");
		return (-1);
	}
	snr_val = ioctl(fd, ALI_NIM_GET_CN_VALUE, 0);
	TUNER_DBG_PRINT("[M] SNR = %d\n", snr_val);
	return 0;
}

static int show_signal_level_monitor(void)
{
	int fd = g_dbg_stat_ctrl.dbg_dev_fd;
	UINT32 level;
	
	if (fd <= 0)
    {
		TUNER_DBG_PRINT("Invalid tuner debug file descriptor.\n");
	    return (-1);
    }
	level = ioctl(fd, ALI_NIM_GET_RF_LEVEL, 0);
	TUNER_DBG_PRINT("[M] LEV = %d\n", level);
	return 0;
}

static int show_register_dump_monitor(void)
{
	int fd = g_dbg_stat_ctrl.dbg_dev_fd;
	UINT8 krv_array[16];
	int ret;

	if (fd <= 0)
	{
		TUNER_DBG_PRINT("Invalid tuner debug file descriptor.\n");
		return (-1);
	}

	memset(krv_array, 0, 16);
	krv_array[0] = 1;      //read
	krv_array[1] = 0x56;   //addr
	krv_array[2] = 1;      //len
	ret = ioctl(fd, ALI_NIM_REG_RW, krv_array);
	TUNER_DBG_PRINT(" *CR%02x = 0x%02x\n", krv_array[1], krv_array[3]);

	krv_array[0] = 1;      //read
	krv_array[1] = 0x0A;
	krv_array[2] = 1;
	ret = ioctl(fd, ALI_NIM_REG_RW, krv_array);
	TUNER_DBG_PRINT("  CR%02x = 0x%02x\n", krv_array[1], krv_array[3]);

	krv_array[0] = 1;      //read
	krv_array[1] = 0x14;
	krv_array[2] = 2;
	ret = ioctl(fd, ALI_NIM_REG_RW, krv_array);
	TUNER_DBG_PRINT("  CR%02x = 0x%04x\n", krv_array[1], (krv_array[3]|((krv_array[4]&0x03)<<8)));

	krv_array[0] = 1;      //read
	krv_array[1] = 0x28;
	krv_array[2] = 1;
	ret = ioctl(fd, ALI_NIM_REG_RW, krv_array);
	TUNER_DBG_PRINT("  CR%02x = 0x%02x\n", krv_array[1], krv_array[3]);

	krv_array[0] = 1;      //read
	krv_array[1] = 0x30;
	krv_array[2] = 1;
	ret = ioctl(fd, ALI_NIM_REG_RW, krv_array);
	TUNER_DBG_PRINT("  CR%02x = 0x%02x\n", krv_array[1], krv_array[3]);

	krv_array[0] = 1;      //read
	krv_array[1] = 0xD8;
	krv_array[2] = 1;
	ret = ioctl(fd, ALI_NIM_REG_RW, krv_array);
	TUNER_DBG_PRINT("  CR%02x = 0x%02x\n", krv_array[1], krv_array[3]);

	krv_array[0] = 1;      //read
	krv_array[1] = 0x6C;
	krv_array[2] = 2;
	ret = ioctl(fd, ALI_NIM_REG_RW, krv_array);
	TUNER_DBG_PRINT("  CR%02x = 0x%04x\n", krv_array[1], (krv_array[3]|(krv_array[4]<<8)));

	krv_array[0] = 1;      //read
	krv_array[1] = 0x89;
	krv_array[2] = 1;
	ret = ioctl(fd, ALI_NIM_REG_RW, krv_array);
	TUNER_DBG_PRINT("  CR%02x = 0x%02x\n", krv_array[1], krv_array[3]);

	return 0;
}

static int handshake_using_manual_addr(UINT8 value)
{
    int fd = g_dbg_stat_ctrl.dbg_dev_fd;
	UINT8 addr_list[4];
	if (0xff == g_dbg_stat_ctrl.dbg_i2c_chnl)
	{
        TUNER_DBG_PRINT("Pls run \"tuner -i base\" cmd to get i2c channel first!\n");
		return -1;
	}
	addr_list[0] = 0xDC;    //cmd head
	addr_list[1] = g_dbg_stat_ctrl.dbg_i2c_chnl;
	addr_list[2] = value;
	
	if (fd <= 0)
    {
		TUNER_DBG_PRINT("Invalid tuner debug file descriptor.\n");
	    return (-1);
    }
	if (0 == ioctl(fd, ALI_NIM_DRIVER_SET_MODE, addr_list))
	{
	    TUNER_DBG_PRINT("Tuner I2C address is 0x%02X\n", value);
	}
	else
	{
		TUNER_DBG_PRINT("Tuner I2C address is NOT 0x%02X\n", value);
	}
	return 0;
}

static void tuner_dbg_task(UINT32 param1, UINT32 param2)
{
	UINT32 CurShowSysTickCnt;
	UINT32 PrevShowSysTickCnt = 0;
	UINT32 SysTickInc;
	static UINT8 print_cnt;

	while(1)
	{
		CurShowSysTickCnt = osal_get_tick();
		SysTickInc = CurShowSysTickCnt - PrevShowSysTickCnt;
		PrevShowSysTickCnt = CurShowSysTickCnt;

		if (g_dbg_stat_ctrl.dbg_enable)
		{
			if (g_dbg_stat_ctrl.dbg_ber_en)
			{
				show_ber_monitor();
			}
			if (g_dbg_stat_ctrl.dbg_snr_en)
			{
				show_snr_monitor();
			}
			if (g_dbg_stat_ctrl.dbg_sig_en)
			{
				show_signal_level_monitor();
			}
			if (g_dbg_stat_ctrl.dbg_krv_en)
			{
				show_register_dump_monitor();
				if (24 <= (++print_cnt))
				{
					show_work_status();
					print_cnt = 0;
				}
			}
			if (g_dbg_stat_ctrl.dbg_show_ims > 0 && 0 == --g_dbg_stat_ctrl.dbg_show_ims)
			{
				TUNER_DBG_PRINT("\nTimes up! End of printout.\n");
				g_dbg_stat_ctrl.dbg_enable = FALSE;
			}
		}
		osal_task_sleep(g_dbg_stat_ctrl.dbg_show_intv);
	}
}

static RET_CODE tuner_dbg_task_Init()
{
	OSAL_T_CTSK t_ctsk;
	memset(&t_ctsk, 0, sizeof(OSAL_T_CTSK ));
	t_ctsk.task = (FP)tuner_dbg_task;
	t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 10;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.name[0] = 'T';
	t_ctsk.name[1] = 'D';
	t_ctsk.name[2] = 'T';
	
	g_dbg_stat_ctrl.dbg_task_id = osal_task_create(&t_ctsk);
	if (OSAL_INVALID_ID == g_dbg_stat_ctrl.dbg_task_id)
	{
		TUNER_DBG_PRINT("Tuner debug task init fail!\n");
		return RET_FAILURE;
	}

	return RET_SUCCESS;
}

static void tuner_dbg_init(void)
{
	TUNER_DBG_PRINT("Waiting for tuner debug statistics...\n");
	memset(&g_dbg_stat_ctrl, 0, sizeof(tuner_dbg_stat_ctrl));
	g_dbg_stat_ctrl.dbg_init      = TRUE;
	g_dbg_stat_ctrl.dbg_enable    = TRUE;
	g_dbg_stat_ctrl.dbg_i2c_chnl  = 0xFF;
	g_dbg_stat_ctrl.dbg_show_intv = TUNER_DBG_PRINT_INTRV;

	if (tuner_dbg_device_open() < 0)
	{
		TUNER_DBG_PRINT("Tuner debug device open fail!\n");
		return;
	}

	TUNER_DBG_PRINT("Tuner debug statistics opened.\n");
}

static void tuner_dbg_show_help(int argc, char **argv)
{
	if (argc != 0)
	{
		TUNER_DBG_PRINT("Option \"-h\" should not with any argument(s)!\n");
		return;
	}

	TUNER_DBG_PRINT("\nModule name:\n");
	TUNER_DBG_PRINT("\tTuner/Nim\n");
	TUNER_DBG_PRINT("\nModule Description:\n");
	TUNER_DBG_PRINT("\tTuner/Nim diagnostic debugging module.\n");	
	
	TUNER_DBG_PRINT("\nSyntax:\n");
	TUNER_DBG_PRINT("\ttun -option [arguments]...\n");
	
	TUNER_DBG_PRINT("\nDescription:\n");
	TUNER_DBG_PRINT("\t-h, --help\n");
	TUNER_DBG_PRINT("\t\tSelf description of this debugging moduld.\n");
	TUNER_DBG_PRINT("\t-s, --start\n");
	TUNER_DBG_PRINT("\t\tStart tuner debugging printout task.\n");
	TUNER_DBG_PRINT("\t-p, --pause\n");
	TUNER_DBG_PRINT("\t\tPause tuner debugging printout task.\n");
	TUNER_DBG_PRINT("\t-e, --exit\n");
	TUNER_DBG_PRINT("\t\tExit tuner debugging module.\n");
	TUNER_DBG_PRINT("\t-i, --info items...\n");
	TUNER_DBG_PRINT("\t\tShow tuner basic information and parameters state.\n");
	TUNER_DBG_PRINT("\t\tbase\t--Printout tuner/nim basic information such as tuner model/tuner i2c address, etc.\n");
	TUNER_DBG_PRINT("\t\tstat\t--Printout tuner/nim current parameters status such as frequency/symbol rate/lock status, etc.\n");
	TUNER_DBG_PRINT("\t-t, --try address\n");
	TUNER_DBG_PRINT("\t\tTry to communicate with tuner using a input I2C address(HEX format).\n");
	TUNER_DBG_PRINT("\t-op, --open items...\n");
	TUNER_DBG_PRINT("\t\tOpen debugging item statistics printout.\n");
	TUNER_DBG_PRINT("\t\tall\t--All the items.\n");
	TUNER_DBG_PRINT("\t\tber\t--NIM Bit Error Rate.\n");
	TUNER_DBG_PRINT("\t\tsnr\t--The C/N feedback value from nim.\n");
	TUNER_DBG_PRINT("\t\tsig\t--The RF level feedback value from nim.\n");
	TUNER_DBG_PRINT("\t\tkrv\t--The key register value of nim.\n");
	TUNER_DBG_PRINT("\t-cl, --close items...\n");
	TUNER_DBG_PRINT("\t\tClose debugging item statistics printout.\n");
	TUNER_DBG_PRINT("\t\tall\t--All the items.\n");
	TUNER_DBG_PRINT("\t\tber\t--NIM Bit Error Rate.\n");
	TUNER_DBG_PRINT("\t\tsnr\t--The C/N feedback value from nim.\n");
	TUNER_DBG_PRINT("\t\tsig\t--The RF level feedback value from nim.\n");
	TUNER_DBG_PRINT("\t\tkrv\t--The key register value of nim.\n");
	TUNER_DBG_PRINT("\t-terv number\n");
	TUNER_DBG_PRINT("\t\tSet time interval of debugging printout task(The default value is \"500\").\n");
	TUNER_DBG_PRINT("\t--times number\n");
	TUNER_DBG_PRINT("\t\tSet times of debugging printout task(The default value is \"0\").\n");
	
	TUNER_DBG_PRINT("\nUsage example:\n");
	TUNER_DBG_PRINT("\ttun -h[--help]\n");
	TUNER_DBG_PRINT("\ttun -s\n");
	TUNER_DBG_PRINT("\ttun -p\n");
	TUNER_DBG_PRINT("\ttun -e[--exit]\n");
	TUNER_DBG_PRINT("\ttun -i[--info] base [stat]\n");
	TUNER_DBG_PRINT("\ttun -t[--try] c2\n");
	TUNER_DBG_PRINT("\ttun -op[--open] [all] [ber] [snr] [sig] [krv]\n");
	TUNER_DBG_PRINT("\ttun -cl[--close] [all] [ber] [snr] [sig] [krv]\n");
	TUNER_DBG_PRINT("\ttun -terv 1000\n");
	TUNER_DBG_PRINT("\ttun --times 0\n");

    return;
}

static void tuner_app_show_help(int argc, char **argv)
{
	if (argc != 0)
	{
		TUNER_DBG_PRINT("Option \"-h\" should not with any argument(s)!\n");
		return;
	}

	TUNER_DBG_PRINT("\nModule name:\n");
	TUNER_DBG_PRINT("\tTuner/Nim\n");

	TUNER_DBG_PRINT("\nModule Description:\n");
	TUNER_DBG_PRINT("\tTuner/Nim diagnostic debugging module.\n");	
	
	TUNER_DBG_PRINT("\nSyntax:\n");
	TUNER_DBG_PRINT("\ttun -option [arguments]...\n");
	
	TUNER_DBG_PRINT("\nDescription:\n");
	TUNER_DBG_PRINT("\t-h, --help\n");
	TUNER_DBG_PRINT("\t\tSelf description of this debugging moduld.\n");
	TUNER_DBG_PRINT("\t-e, --exit\n");
	TUNER_DBG_PRINT("\t\tExit tuner debugging module.\n");
	TUNER_DBG_PRINT("\t-i, --info items...\n");
	TUNER_DBG_PRINT("\t\tShow tuner basic information and parameters state.\n");
	TUNER_DBG_PRINT("\t\tbase\t--Printout tuner/nim basic information such as tuner model/tuner i2c address, etc.\n");
	TUNER_DBG_PRINT("\t\tstat\t--Printout tuner/nim current parameters status such as frequency/symbol rate/lock status, etc.\n");
	TUNER_DBG_PRINT("\t-t, --try address\n");
	TUNER_DBG_PRINT("\t\tTry to communicate with tuner using a input I2C address(HEX format).\n");

	TUNER_DBG_PRINT("\nUsage example:\n");
	TUNER_DBG_PRINT("\ttun -h[--help]\n");
	TUNER_DBG_PRINT("\ttun -e[--exit]\n");
	TUNER_DBG_PRINT("\ttun -i[--info] base [stat]\n");
	TUNER_DBG_PRINT("\ttun -t[--try] c2\n");
	
    return;
}

static void tuner_dbg_start(int argc, char **argv)
{
	if (argc != 0)
	{
		TUNER_DBG_PRINT("Option \"-s\" should not with any argument(s)!\n");
		return;
	}
	
	if (!g_dbg_stat_ctrl.dbg_init)
	{
		tuner_dbg_init();
	}

	if (!g_dbg_stat_ctrl.dbg_tsk_init) 
	{
		if (tuner_dbg_task_Init() != RET_SUCCESS)
		{
			TUNER_DBG_PRINT("Tuner debug task initialized fail!\n");
			return;
		}
		g_dbg_stat_ctrl.dbg_tsk_init = 1;
	}

	g_dbg_stat_ctrl.dbg_enable = TRUE;
}

static void tuner_dbg_pause(int argc, char **argv)
{
	if (argc != 0)
	{
		TUNER_DBG_PRINT("Option \"-p\" should not with any argument(s)!\n");
		return;
	}
	
	if (!g_dbg_stat_ctrl.dbg_tsk_init) 
	{
		if (tuner_dbg_task_Init() != RET_SUCCESS)
		{
			TUNER_DBG_PRINT("Tuner debug task initialized fail!\n");
			return;
		}
		g_dbg_stat_ctrl.dbg_tsk_init = 1;
	}


	g_dbg_stat_ctrl.dbg_enable = FALSE;
}

static void tuner_dbg_exit(int argc, char **argv)
{
	if (argc != 0)
	{
		TUNER_DBG_PRINT("Option \"-e\" should not with any argument(s)!\n");
		return;
	}
	
	if (g_dbg_stat_ctrl.dbg_task_id > 0)
	{
		if (g_dbg_stat_ctrl.dbg_tsk_init)
			osal_task_delete(g_dbg_stat_ctrl.dbg_task_id);
		tuner_dbg_device_close();
		memset(&g_dbg_stat_ctrl, 0, sizeof(tuner_dbg_stat_ctrl));
		TUNER_DBG_PRINT("Tuner debug task closed.\n");
	}
}

static void tuner_dbg_show_info(int argc, char **argv)
{
	if (0 == argc)
	{
		TUNER_DBG_PRINT("Lack of valid argument(s)(ex: \"base\" \"stat\")!\n");
		return;
	}

	if (!g_dbg_stat_ctrl.dbg_init)
	{
		tuner_dbg_init();
	}

	while (argc-- > 0)
	{
		if (!strcmp(*argv, "base"))
			show_basic_info();
		else if (!strcmp(*argv, "stat"))
			show_work_status();
		else
			TUNER_DBG_PRINT("Unrecognized argument(s)!\n");

		argv++;
	}

	return;
}

static ARG_CHK tuner_dbg_info_preview(int argc, char **argv, char *option)
{
	if (0 == argc)
	{
		TUNER_DBG_PRINT("Option \"%s\": Lack of valid argument(s)(ex: \"base\" \"stat\")!\n", option);
		return DBG_ARG_INVALID;
	}

	while (argc-- > 0)
	{
		if (strcmp(*argv, "base") &&
			strcmp(*argv, "stat"))
		{
			TUNER_DBG_PRINT("Option \"%s\": Unrecognized argument \"%s\"!\n", option, *argv);
			return DBG_ARG_INVALID;
		}

		argv++;
	}

	return DBG_ARG_VALID;
}

static void tuner_dbg_switch_on(int argc, char **argv)
{
	if (!g_dbg_stat_ctrl.dbg_init)
	{
		tuner_dbg_init();
	}

	if (!g_dbg_stat_ctrl.dbg_tsk_init) 
	{
		if (tuner_dbg_task_Init() != RET_SUCCESS)
		{
			TUNER_DBG_PRINT("Tuner debug task initialized fail!\n");
			return;
		}
		g_dbg_stat_ctrl.dbg_tsk_init = 1;
	}

	if (0 == argc)
		goto TUNER_ON_ALL;

	while (argc-- > 0)
	{
		if (!strcmp(*argv, "all"))
		{
TUNER_ON_ALL:
			g_dbg_stat_ctrl.dbg_ber_en = TRUE;
			g_dbg_stat_ctrl.dbg_snr_en = TRUE;
			g_dbg_stat_ctrl.dbg_sig_en = TRUE;
			g_dbg_stat_ctrl.dbg_krv_en = TRUE;
			TUNER_DBG_PRINT("NIM BER statistics enabled.\n");
			TUNER_DBG_PRINT("NIM SNR statistics enabled.\n");
			TUNER_DBG_PRINT("NIM sigal strength statistics enabled.\n");
			TUNER_DBG_PRINT("NIM key register value statistics enabled.\n");
			break;
		}
		else if (!strcmp(*argv, "ber"))
		{
			g_dbg_stat_ctrl.dbg_ber_en = TRUE;
			TUNER_DBG_PRINT("NIM BER statistics enabled.\n");
		}
		else if (!strcmp(*argv, "snr"))
		{
			g_dbg_stat_ctrl.dbg_snr_en = TRUE;
			TUNER_DBG_PRINT("NIM SNR statistics enabled.\n");
		}
		else if (!strcmp(*argv, "sig"))
	    {
			g_dbg_stat_ctrl.dbg_sig_en = TRUE;
			TUNER_DBG_PRINT("NIM sigal strength statistics enabled.\n");
		}
		else if (!strcmp(*argv, "krv"))
		{
			g_dbg_stat_ctrl.dbg_krv_en = TRUE;
			TUNER_DBG_PRINT("NIM key register value statistics enabled.\n");
		}
		else
		{
			TUNER_DBG_PRINT("Unrecognized argument(s)!\n");
		}

		argv++;
	}

	g_dbg_stat_ctrl.dbg_enable = TRUE;

	return;
}

static void tuner_dbg_switch_off(int argc, char **argv)
{
	if (!g_dbg_stat_ctrl.dbg_init)
	{
		tuner_dbg_init();
	}

	if (!g_dbg_stat_ctrl.dbg_tsk_init) 
	{
		if (tuner_dbg_task_Init() != RET_SUCCESS)
		{
			TUNER_DBG_PRINT("Tuner debug task initialized fail!\n");
			return;
		}
		g_dbg_stat_ctrl.dbg_tsk_init = 1;
	}


	if (0 == argc)
		goto TUNER_OFF_ALL;

	while (argc-- > 0)
	{
		if (!strcmp(*argv, "all"))
		{
TUNER_OFF_ALL:
			g_dbg_stat_ctrl.dbg_ber_en = FALSE;
			g_dbg_stat_ctrl.dbg_snr_en = FALSE;
			g_dbg_stat_ctrl.dbg_sig_en = FALSE;
			g_dbg_stat_ctrl.dbg_krv_en = FALSE;
			TUNER_DBG_PRINT("NIM BER statistics disabled.\n");
			TUNER_DBG_PRINT("NIM SNR statistics disabled.\n");
			TUNER_DBG_PRINT("NIM sigal strength statistics disabled.\n");
			TUNER_DBG_PRINT("NIM key register value statistics disabled.\n");
			break;
		}
		else if (!strcmp(*argv, "ber"))
		{
			g_dbg_stat_ctrl.dbg_ber_en = FALSE;
			TUNER_DBG_PRINT("NIM BER statistics disabled.\n");
		}
		else if (!strcmp(*argv, "snr"))
		{
			g_dbg_stat_ctrl.dbg_snr_en = FALSE;
			TUNER_DBG_PRINT("NIM SNR statistics disabled.\n");
		}
		else if (!strcmp(*argv, "sig"))
		{
			g_dbg_stat_ctrl.dbg_sig_en = FALSE;
			TUNER_DBG_PRINT("NIM sigal strength statistics disabled.\n");
		}
		else if (!strcmp(*argv, "krv"))
        {
            g_dbg_stat_ctrl.dbg_krv_en = FALSE;
            TUNER_DBG_PRINT("NIM key register value statistics disabled.\n");
        }
		else
		{
			TUNER_DBG_PRINT("Unrecognized argument(s)!\n");
		}

		argv++;
	}

	g_dbg_stat_ctrl.dbg_enable = TRUE;

	return;
}

static ARG_CHK tuner_dbg_swi_preview(int argc, char **argv, char *option)
{
	while (argc-- > 0)
	{
		if (strcmp(*argv, "all") &&
			strcmp(*argv, "ber") &&
			strcmp(*argv, "snr") &&
			strcmp(*argv, "sig") &&
			strcmp(*argv, "krv"))
		{
			TUNER_DBG_PRINT("Option \"%s\": Unrecognized argument \"%s\"!\n", option, *argv);
			return DBG_ARG_INVALID;
		}

		argv++;
	}

	return DBG_ARG_VALID;
}

static void tuner_dbg_i2c_try(int argc, char **argv)
{
	unsigned long value;

	if (0 == argc)
	{
		TUNER_DBG_PRINT("Lack of valid argument(ex: \"c2\")!\n");
		return;
	}

	value = strtoul(*argv, 0, 16);
	if (ERANGE == value)
	{
		TUNER_DBG_PRINT("Argument \"%s\" transformed fail!\n", *argv);
		return;
	}
	if (value > 0xff)
	{
		TUNER_DBG_PRINT("Invalid I2C address: 0x%x\n", value);
		return;
	}
	
	if (!g_dbg_stat_ctrl.dbg_init)
	{
		tuner_dbg_init();
	}

	handshake_using_manual_addr((UINT8)value);
}

static ARG_CHK tuner_dbg_hex_preview(int argc, char **argv, char *option)
{
	int ret;
	unsigned long value;

	if (0 == argc)
	{
		TUNER_DBG_PRINT("Option \"%s\": Lack of valid argument(ex: c0)!\n", option);
		return DBG_ARG_INVALID;
	}

	value = strtoul(*argv, 0, 16);
    if (ERANGE == value)
	{
		TUNER_DBG_PRINT("Argument \"%s\" transformed fail!\n", *argv);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}

static void tuner_dbg_set_intv(int argc, char **argv)
{
	int number, ret;

	if (0 == argc)
	{
		TUNER_DBG_PRINT("Lack of valid argument(ex: \"5000\")!\n");
		return;
	}

	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		TUNER_DBG_PRINT("Argument \"%s\" is not a number or less than zero!\n", *argv);
		return;
	}
	if (!g_dbg_stat_ctrl.dbg_init)
	{
		tuner_dbg_init();
	}

	g_dbg_stat_ctrl.dbg_show_intv = number;
	g_dbg_stat_ctrl.dbg_enable = TRUE;

	return;
}

static void tuner_dbg_set_tms(int argc, char **argv)
{
	int number, ret;

	if (0 == argc)
	{
		TUNER_DBG_PRINT("Lack of valid argument(ex: \"100\")!\n");
		return;
	}

	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		TUNER_DBG_PRINT("Argument \"%s\" is not a number or less than zero!\n", *argv);
		return;
	}
	if (!g_dbg_stat_ctrl.dbg_init)
	{
		tuner_dbg_init();
	}

	g_dbg_stat_ctrl.dbg_show_ims = number;
	g_dbg_stat_ctrl.dbg_enable = TRUE;

	return;
}

static ARG_CHK tuner_dbg_num_preview(int argc, char **argv, char *option)
{
	int number, ret;
	char *p;

	if (0 == argc)
	{
		TUNER_DBG_PRINT("Option \"%s\": Lack of valid argument(ex: number)!\n", option);
		return DBG_ARG_INVALID;
	}

	if (argc != 1)
	{
		TUNER_DBG_PRINT("Option \"%s\": Only one argument admitted!\n", option);
		return DBG_ARG_INVALID;
	}
	
	ret = sscanf(*argv, "%d", &number);
	if (0 == ret || number < 0)
	{
		TUNER_DBG_PRINT("Option \"%s\": Argument \"%s\" not a number or less than zero!\n", option, *argv);
		return DBG_ARG_INVALID;
	}
	
	return DBG_ARG_VALID;
}

static PARSE_COMMAND_LIST tunerdbg_command[] = {
	{ { NULL, NULL }, tuner_dbg_show_help, NULL, "help",  "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, tuner_dbg_start,      NULL,  "start", "s",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, tuner_dbg_pause,      NULL,  "pause", "p",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, tuner_dbg_exit,       NULL,  "exit",  "e",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, tuner_dbg_show_info,  tuner_dbg_info_preview,    "info",  "i",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, tuner_dbg_i2c_try,    tuner_dbg_hex_preview,     "try",   "t",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, tuner_dbg_switch_on,  tuner_dbg_swi_preview,     NULL,    "op",   0, 0, NULL, 0, 0, NULL, 0, 0 },
    { { NULL, NULL }, tuner_dbg_switch_off, tuner_dbg_swi_preview,     NULL,    "cl",   0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, tuner_dbg_set_intv,   tuner_dbg_num_preview,     NULL,    "terv", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, tuner_dbg_set_tms,    tuner_dbg_num_preview,     "times", NULL,   0, 0, NULL, 0, 0, NULL, 0, 0 },
};

static PARSE_COMMAND_LIST tunerdbg_app_command[] = {
	{ { NULL, NULL }, tuner_app_show_help, NULL, "help",  "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, tuner_dbg_start,      NULL,  "start", "s",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, tuner_dbg_pause,      NULL,  "pause", "p",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, tuner_dbg_exit,       NULL,  "exit",  "e",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, tuner_dbg_show_info,  tuner_dbg_info_preview,    "info",  "i",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, tuner_dbg_i2c_try,    tuner_dbg_hex_preview,     "try",   "t",    0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, tuner_dbg_switch_on,  tuner_dbg_swi_preview,     NULL,    "op",   0, 0, NULL, 0, 0, NULL, 0, 0 },
    //{ { NULL, NULL }, tuner_dbg_switch_off, tuner_dbg_swi_preview,     NULL,    "cl",   0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, tuner_dbg_set_intv,   tuner_dbg_num_preview,     NULL,    "terv", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	//{ { NULL, NULL }, tuner_dbg_set_tms,    tuner_dbg_num_preview,     "times", NULL,   0, 0, NULL, 0, 0, NULL, 0, 0 },
};

void tunerdbg_module_register(void)
{
    debug_module_add("tun", &tunerdbg_command[0], ARRAY_SIZE(tunerdbg_command));
}

INT32 tun_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &tunerdbg_command[0];
	*list_cnt = ARRAY_SIZE(tunerdbg_command);

	return RET_SUCCESS;
}

INT32 tun_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &tunerdbg_app_command[0];
	*list_cnt = ARRAY_SIZE(tunerdbg_app_command);

	return RET_SUCCESS;
}
