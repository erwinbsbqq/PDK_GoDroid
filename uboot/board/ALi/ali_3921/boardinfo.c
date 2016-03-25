#include <common.h>
#include <alidefinition/adf_boot.h>

enum TVSystem
{
	PAL		= 0	, //	PAL4.43(==PAL_BDGHI)		(Fh=15.625,fv=50)
	NTSC		, //	NTSC3.58					(Fh=15.734,Fv=59.94)
	PAL_M		, //	PAL3.58					(Fh=15.734,Fv=59.94)
	PAL_N		, //	PAL4.43(changed PAL mode)	(Fh=15.625,fv=50)	
	PAL_60		, //							(Fh=15.734,Fv=59.94)
	NTSC_443	, //	NTSC4.43					(Fh=15.734,Fv=59.94)
	SECAM		,
	MAC			,
	LINE_720_25,	//added for s3601
	LINE_720_30,	//added for s3601
	LINE_1080_25,	//added for s3601
	LINE_1080_30,	//added for s3601

	LINE_1080_50,	//added for s3602f
	LINE_1080_60,   //added for s3602f
	LINE_1080_24,	//added for s3602f	
	LINE_1152_ASS,  //added for s3602f
	LINE_1080_ASS,  //added for s3602f
	PAL_NC		, //	PAL3.58(changed PAL mode)	(Fh=15.625,fv=50)

	LINE_576P_50_VESA,
	LINE_720P_60_VESA,
	LINE_1080P_60_VESA,
};

typedef enum
{
	TV_ASPECT_RATIO_43 = 0,
	TV_ASPECT_RATIO_169,
	TV_ASPECT_RATIO_AUTO
} TV_ASPECT_RATIO_TYPE;

typedef enum
{
    DISPLAY_MODE_NORMAL = 0,
    DISPLAY_MODE_LETTERBOX,
    DISPLAY_MODE_PANSCAN,
} DISPLAY_MODE_TYPE;

typedef enum
{
	SCART_CVBS = 0,
	SCART_RGB,
	SCART_SVIDEO,
	SCART_YUV,
} SCART_OUT_TYPE;

/* Define for VDAC configuration */
#define VDAC_NUM_MAX		6
#define VDAC_TYPE_NUM		6
//Type
#define VDAC_TYPE_CVBS		0
#define VDAC_TYPE_SVIDEO	1
#define VDAC_TYPE_YUV		2
#define VDAC_TYPE_RGB		3
#define VDAC_TYPE_SCVBS		4
#define VDAC_TYPE_SSV		5
#define VDAC_TYPE_MAX		6
//Detail
#define VDAC_CVBS		(VDAC_TYPE_CVBS<<2|0)	// 0x0
#define VDAC_SVIDEO_Y		(VDAC_TYPE_SVIDEO<<2|0)	// 0x4
#define VDAC_SVIDEO_C		(VDAC_TYPE_SVIDEO<<2|1)	// 0x5
#define VDAC_YUV_Y		(VDAC_TYPE_YUV<<2|0)	// 0x8
#define VDAC_YUV_U		(VDAC_TYPE_YUV<<2|1)	// 0x9
#define VDAC_YUV_V		(VDAC_TYPE_YUV<<2|2)	// 0xA
#define VDAC_RGB_R		(VDAC_TYPE_RGB<<2|0)	// 0xC
#define VDAC_RGB_G		(VDAC_TYPE_RGB<<2|1)	// 0xD
#define VDAC_RGB_B		(VDAC_TYPE_RGB<<2|2)	// 0xE
#define VDAC_SCVBS		(VDAC_TYPE_SCVBS<<2|0)	// 0x10
#define VDAC_SSV_Y		(VDAC_TYPE_SSV<<2|0)	// 0x14
#define VDAC_SSV_C		(VDAC_TYPE_SSV<<2|1)	// 0x15
#define VDAC_NULL		0xFF

typedef enum
{
	SYS_DIGITAL_FMT_BY_EDID = 0,
	SYS_DIGITAL_FMT_RGB,
	SYS_DIGITAL_FMT_RGB_EXPD,
	SYS_DIGITAL_FMT_YCBCR_444,
	SYS_DIGITAL_FMT_YCBCR_422,
} DIGITAL_FMT_TYPE;

typedef enum
{
	SYS_DIGITAL_AUD_BS = 0,
	SYS_DIGITAL_AUD_LPCM,
	SYS_DIGITAL_AUD_AUTO,		//by TV EDID setting
}DIGITAL_AUD_TYPE;

void dump_print(char *str, unsigned char *p, unsigned int size)
{
	int i, j;
	printf("%s\n",str);

	for (i=0;i<size/32;i++)
	{
		for (j=0;j<16;j++)
			printf("0x%02x ",p[i*16+j]);
		printf("\n");
	}

	for (i=i*16; i<size; i++)
		printf("0x%02x ",p[i]);

	printf("\n");
}

int set_avinfo_default(ADF_BOOT_AVINFO *avinfo)
{
	avinfo->tvSystem = LINE_720_25;
	avinfo->progressive = 0;
	avinfo->tv_ratio = TV_ASPECT_RATIO_AUTO;
	avinfo->display_mode = DISPLAY_MODE_LETTERBOX;
	avinfo->scart_out = SCART_YUV;
	avinfo->vdac_out[0] = VDAC_YUV_V;
	avinfo->vdac_out[1] = VDAC_YUV_U;
	avinfo->vdac_out[2] = VDAC_YUV_Y;
	avinfo->vdac_out[3] = VDAC_CVBS;
	avinfo->vdac_out[4] = VDAC_NULL;
	avinfo->vdac_out[5] = VDAC_NULL;
	avinfo->video_format = SYS_DIGITAL_FMT_RGB;
	avinfo->audio_output = SYS_DIGITAL_AUD_LPCM;
	avinfo->brightness = 50;
	avinfo->contrast = 50;
	avinfo->saturation = 50;
	avinfo->sharpness = 5;
	avinfo->hue = 50;	
}

/**
*Description:use user-defined pinmux setting from bootargs partition;
*call this api to use this function
*detailed see
*/
void set_pinmux_setting(void)
{
	unsigned int i;
	unsigned int j;
	unsigned int value;
	unsigned int mask;
	unsigned int reg_addr;
	unsigned int bits_offset;
	unsigned int bits_size;
	unsigned int bits_value;	

	ADF_BOOT_BOARD_INFO *board_info = (ADF_BOOT_BOARD_INFO *)(PRE_DEFINED_ADF_BOOT_START);
	printf("valid cnt %d!!!!!\n",board_info->reg_info.valid_count );

	if(board_info->reg_info.valid_count !=0)//if have valid count let's do user setting, otherwise use default setting
	{

		for(i=0;i<board_info->reg_info.valid_count;i++)
		{
			if(board_info->reg_info.unit[i].magic == REGISTER_VALID_MAGIC)
			{
				//do pinmux here
				reg_addr = board_info->reg_info.unit[i].addr;
				bits_offset = board_info->reg_info.unit[i].bits_offset;
				bits_size = board_info->reg_info.unit[i].bits_size;
				bits_value = board_info->reg_info.unit[i].bits_value;
				
				value = *(int *)(reg_addr);

				mask = 0;
				for(j=0;j<bits_size;j++)
				{
					mask |= 1<<(bits_offset+j);
				}
				value = value&(~mask);//clean setting bits

				mask = 0;
				mask =  bits_value << bits_offset;
				value = value | mask;//setting bits
				*(int *)(reg_addr) = value;

		//	printf("addr 0x%08x bits_offset %d size %d value %d reg value 0x%08x\n",
		//		reg_addr,bits_offset,bits_size,bits_value,*(int *)(reg_addr));
 			}			
		}

	}

}

int set_boardinfo(int isRecovery)
{
	unsigned int size;
	unsigned char *p;
	
	ADF_BOOT_BOARD_INFO *boardinfo = (ADF_BOOT_BOARD_INFO *)(PRE_DEFINED_ADF_BOOT_START);

	size = get_hdmiinfo(&p);
	memcpy((u_char*)&boardinfo->hdcp, p, size);

	size = get_avinfo(&p);
	if (size)
		memcpy((u_char*)&boardinfo->avinfo, p, size);
	else
		set_avinfo_default(&boardinfo->avinfo);

	size = get_mac(&p);
	memcpy((u_char*)&boardinfo->macinfo, p, size);

	size = get_memmap(&p,isRecovery);
	memcpy((u_char*)&boardinfo->memmap_info, p, size);

	//dump_print("memmap:",p, 32);

	size = get_tveinfo(&p);
	memcpy((u_char*)&boardinfo->tve_info, p, size);

	size = get_registerinfo(&p);
	memcpy((u_char*)&boardinfo->reg_info, p, size);
	
	set_pinmux_setting();
	return 0;
}

