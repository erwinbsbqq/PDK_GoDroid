/*****************************************************************************
*	Copyrights(C) 2012 Acer Laboratries inc. All Rights Reserved.
*
*	FILE NAME:		adr_dbg_adc.c
*
*	DESCRIPTION:	ADC dump debug module.
*
*	HISTORY:
*						Date 	 Author      Version 	  Notes
*					==========	=========	=========	===========
*					2012-12-24	 Leo.Ma      Ver 1.0	Create file.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <hld_cfg.h>
#include <osal/osal_task.h>
#include <hld/adr_hld_dev.h>
#include <hld/nim/adr_nim.h>
#include <hld/dbg/adr_dbg_parser.h>

#if ( SYS_PROJECT_FE == PROJECT_FE_DVBC )
#define __ADR_ADC_DBG 

#ifdef __ADR_ADC_DBG 

#define ADC_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(ADC, fmt, ##args); \
			} while(0)
#else
#define ADC_DBG_PRINT(...)
#endif

#define ADC_REG_SIZ	11

typedef struct adc_dbg_info {
	struct nim_device * dev;
	UINT32 freq;
	UINT32 sym;
	UINT8 fec;
	UINT8 regs[ADC_REG_SIZ];
} ADC_DBG_INFO_S;

static ADC_DBG_INFO_S AdcDbgInfo;

static void adc_reg_get(UINT8 from_hw)
{
	UINT8 i;

	if (from_hw)
	{
		nim_reg_read_ext(AdcDbgInfo.dev, 0x100, AdcDbgInfo.regs, sizeof(AdcDbgInfo.regs) - 1);
		ADC_DBG_PRINT("The adc hw reg[0x100~0x109] are:\n");
	}
	else
	{
		ADC_DBG_PRINT("The adc sw buf[0x100~0x109] are:\n");
	}

	for (i = 0; i < sizeof(AdcDbgInfo.regs) - 1; i++)
		ADC_DBG_PRINT(" %02x", AdcDbgInfo.regs[i]);
	ADC_DBG_PRINT("\n");

	return;
}

static UINT8 adc_is_hex_num(char ** param)
{
	if (NULL == param)
		return 0;

	if ('0' == (*param)[0] && 'x' == ((*param)[1] | 0x20) && isxdigit((*param)[2]))
	{
		*param = &(*param)[2];
		return 1;
	}
	else
		return 0;
}

static void adc_dbg_show_help(int argc, char **argv)
{
	ADC_DBG_PRINT("\nModule name:\n");
	ADC_DBG_PRINT("\tadc\n");
	ADC_DBG_PRINT("\nModule Description:\n");
	ADC_DBG_PRINT("\tADC dump debug module.\n");	
	ADC_DBG_PRINT("\nSyntax:\n");
	ADC_DBG_PRINT("\tadc -option [arguments]\n");
	ADC_DBG_PRINT("\nDescription:\n");
	ADC_DBG_PRINT("\t-h, --help\n");
	ADC_DBG_PRINT("\t\tSelf description of this debugging moduld.\n");
	ADC_DBG_PRINT("\t-s, --start\n");
	ADC_DBG_PRINT("\t\tStart ADC.\n");
	ADC_DBG_PRINT("\t-p, --pause\n");
	ADC_DBG_PRINT("\t\tStop ADC.\n");
	ADC_DBG_PRINT("\t-i, --info\n");
	ADC_DBG_PRINT("\t\tShow ADC config infomation.\n");
	ADC_DBG_PRINT("\t-d, --dump\n");
	ADC_DBG_PRINT("\t\tDump ADC2DMA RF data.\n");
	ADC_DBG_PRINT("\t-r\n");
	ADC_DBG_PRINT("\t\tSet ADC register value.\n");
	ADC_DBG_PRINT("\t-freq\n");
	ADC_DBG_PRINT("\t\tSet ADC frequency.\n");
	ADC_DBG_PRINT("\t-sym\n");
	ADC_DBG_PRINT("\t\tSet ADC symbol rate.\n");
	ADC_DBG_PRINT("\t-fec\n");
	ADC_DBG_PRINT("\t\tSet ADC modulation.\n");
	ADC_DBG_PRINT("\nUsage example:\n");
	ADC_DBG_PRINT("\tadc -h[--help]\n");
	ADC_DBG_PRINT("\tadc -s[--start]\n");
	ADC_DBG_PRINT("\tadc -p[--pause]\n");
	ADC_DBG_PRINT("\tadc -i[--info]\n");
	ADC_DBG_PRINT("\tadc -d[--dump]\n");
	ADC_DBG_PRINT("\tadc -r 5 0xab 0xcd\n");
	ADC_DBG_PRINT("\tadc -freq 538\n");
	ADC_DBG_PRINT("\tadc -sym 6875\n");
	ADC_DBG_PRINT("\tadc -fec 64\n");

	return;
}

static void adc_dbg_lock(int argc, char **argv)
{
	UINT8 lock, lock_cnt;

	AdcDbgInfo.regs[sizeof(AdcDbgInfo.regs) - 1] = '\0';
	adc_reg_get(0);

    nim_quick_channel_change(AdcDbgInfo.dev, AdcDbgInfo.freq, AdcDbgInfo.sym, AdcDbgInfo.fec);

	lock_cnt = 5;
	lock = 0;
	while (!lock && lock_cnt-- > 0)
	{
		nim_get_lock(AdcDbgInfo.dev, &lock);
		osal_task_sleep(1000);
		if (lock)
		{
			ADC_DBG_PRINT("Nim locked!\n");
			break;
		}
		else
		{
			ADC_DBG_PRINT("Nim not locked, try again...\n");
		}
	}

	if (!lock)
	{
		ADC_DBG_PRINT("Nim lock fail!\n");
		return;
	}
	
	return;
}

static void adc_dbg_data_dump(int argc, char **argv)
{
	if (nim_adcdma_start(AdcDbgInfo.dev, AdcDbgInfo.regs))
	{
		ADC_DBG_PRINT("ADC debug start fail!\n");
		return;
	}

	osal_task_sleep(100); //wait...then dump

	if (nim_adcdma_dump_RF_data(AdcDbgInfo.dev))
	{
		ADC_DBG_PRINT("ADC2DMA dump fail!\n");
		return;
	}
	
	return;
}

static void adc_dbg_stop(int argc, char **argv)
{
	if (nim_adcdma_stop(AdcDbgInfo.dev))
	{
		ADC_DBG_PRINT("ADC debug stop failed!\n");
		return;
	}
	
	return;
}

static void adc_dbg_show_info(int argc, char **argv)
{
	ADC_DBG_PRINT("ADC frequency: %uKHz\n", AdcDbgInfo.freq * 10);
	ADC_DBG_PRINT("ADC symbol: %u\n", AdcDbgInfo.sym);
	ADC_DBG_PRINT("ADC modulation: ");
	switch (AdcDbgInfo.fec)
	{
	case QAM16:
		ADC_DBG_PRINT("QAM16\n");
		break;		
	case QAM32:
		ADC_DBG_PRINT("QAM32\n");
		break;	
	case QAM64:
		ADC_DBG_PRINT("QAM64\n");
		break;	
	case QAM128:
		ADC_DBG_PRINT("QAM128\n");
		break;
	case QAM256:
		ADC_DBG_PRINT("QAM256\n");
		break;
	default:
		break;	
	}
	adc_reg_get(1);

	return;
}

static ARG_CHK adc_dbg_no_para_preview(int argc, char **argv, char *option)
{
	if (argc != 0)
	{
		ADC_DBG_PRINT("Option \"%s\": Should not be with any argument(s)!\n", option);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}


static void adc_dbg_reg_set(int argc, char **argv)
{
	UINT8 byte_val[ADC_REG_SIZ];
	int start_index, last_index;

	if (--argc < 1)
	{
		ADC_DBG_PRINT("Option \"-r\": No proper value to be fitted ADC register!\n");
		return;
	}

	adc_reg_get(1);
	
	if (sscanf(*argv, "%d", &start_index) != 1)
	{
		ADC_DBG_PRINT("Option \"-r\": Start index \"%s\" not a decimal number!\n", *argv);
		return;
	}

	if (start_index >= (int)sizeof(AdcDbgInfo.regs) - 1 || argc > (int)sizeof(AdcDbgInfo.regs) - start_index)
	{
		ADC_DBG_PRINT("Option \"-r\": The count of value overflows!\n");
		return;
	}

	last_index = start_index;
	memset(byte_val, 0, sizeof(byte_val));

	while (argc-- > 0)
	{
		if (sscanf(*++argv, "%x", &byte_val[last_index++]) != 1)
		{
			ADC_DBG_PRINT("Option \"-r\": Argument \"%s\" not a valid hex number!\n", *argv);
			return;
		}
	}

	memcpy(&AdcDbgInfo.regs[start_index], &byte_val[start_index], last_index - start_index);

	adc_reg_get(0);

	return;
}

static void adc_dbg_freq_set(int argc, char **argv)
{
	UINT32 number;

	if (!sscanf(*argv, "%u", &number))
	{
		ADC_DBG_PRINT("Argument \"%s\" not a number!\n", *argv);
		return;
	}
	
	AdcDbgInfo.freq = number * 100;

	return;
}

static void adc_dbg_sym_set(int argc, char **argv)
{
	UINT32 number;

	if (!sscanf(*argv, "%u", &number))
	{
		ADC_DBG_PRINT("Argument \"%s\" not a number!\n", *argv);
		return;
	}
	
	AdcDbgInfo.sym = number;

	return;
}

static void adc_dbg_fec_set(int argc, char **argv)
{
	UINT32 number;

	if (!sscanf(*argv, "%u", &number))
	{
		ADC_DBG_PRINT("Argument \"%s\" not a number!\n", *argv);
		return;
	}

	switch (number)
	{
	case 16:
		AdcDbgInfo.fec = QAM16;
		break;		
	case 32:
		AdcDbgInfo.fec = QAM32;
		break;	
	case 64:
		AdcDbgInfo.fec = QAM64;
		break;	
	case 128:
		AdcDbgInfo.fec = QAM128;
		break;	
	case 256:
		AdcDbgInfo.fec = QAM256;
		break;	
	default:
		break;
	}

	return;
}

static ARG_CHK adc_dbg_one_para_preview(int argc, char **argv, char *option)
{
	UINT32 number;

	if (argc != 1)
	{
		ADC_DBG_PRINT("Option \"%s\": Only one argument permitted!\n", option);
		return DBG_ARG_INVALID;
	}

	if (!sscanf(*argv, "%u", &number))
	{
		ADC_DBG_PRINT("Option \"%s\": Argument \"%s\" not a number!\n", option, *argv);
		return DBG_ARG_INVALID;
	}
	else if (!strcmp(option, "freq") && number > 1000)
	{
		ADC_DBG_PRINT("Option \"-freq\": Argument \"%s\" out of range!\n", *argv);
		return DBG_ARG_INVALID;
	}
	else if (!strcmp(option, "fec") && number != 16 && number != 32 && number != 64 && number != 128 && number != 256)
	{
		ADC_DBG_PRINT("Option \"-fec\": QAM value should be \"16\", \"32\", \"64\", \"128\" or \"256\"!\n", *argv);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}

static PARSE_COMMAND_LIST adcdbg_command[] = {
	{ { NULL, NULL }, adc_dbg_lock, adc_dbg_no_para_preview, "lock", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, adc_dbg_stop, adc_dbg_no_para_preview, "pause", "p", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, adc_dbg_show_help, adc_dbg_no_para_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, adc_dbg_show_info, adc_dbg_no_para_preview, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, adc_dbg_data_dump, adc_dbg_no_para_preview, "dump", "d", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, adc_dbg_freq_set, adc_dbg_one_para_preview, NULL, "freq", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, adc_dbg_sym_set, adc_dbg_one_para_preview, NULL, "sym", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, adc_dbg_fec_set, adc_dbg_one_para_preview, NULL, "fec", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, adc_dbg_reg_set, NULL, NULL, "r", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};

INT32 adc_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &adcdbg_command[0];
	*list_cnt = ARRAY_SIZE(adcdbg_command);

	if (NULL == (AdcDbgInfo.dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0)))
		return RET_FAILURE;

	AdcDbgInfo.freq = 0;
	AdcDbgInfo.sym = 6875;
	AdcDbgInfo.fec = QAM64;
	memset(AdcDbgInfo.regs, 0 ,sizeof(AdcDbgInfo.regs));
	adc_reg_get(1);

	return RET_SUCCESS;
}

INT32 adc_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	return adc_dbg_cmd_get(cmd_list, list_cnt);
}
#else
INT32 adc_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	return -1;
}

INT32 adc_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	return -1;
}
#endif
