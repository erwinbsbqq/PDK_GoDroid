/*
Description: to set the registers of M3602 for DE_N and related TVE, while DE_O registers are setted in file decLogo_m36_mini.c
History:
2009-04-08		Costa Wen		Create
*/
//#include "sys_config.h"
//#include <sys_parameters.h>
#if (SYS_SDRAM_SIZE != 2)
#define SYS_WriteByte(uAddr, bVal) \
							do{*(volatile unsigned char *)(uAddr) = (bVal);}while(0)
#define SYS_WriteWord(uAddr, wVal) \
							do{*(volatile unsigned short *)(uAddr) = (wVal);}while(0)
#define SYS_WriteDWord(uAddr, dwVal) \
							do{*(volatile unsigned long *)(uAddr) = (dwVal);}while(0)
#define SYS_ReadByte(uAddr) \
							({ \
								volatile unsigned char bVal; \
								bVal = (*(volatile unsigned char *)(uAddr)); \
								bVal; \
							})
#define SYS_ReadWord(uAddr) \
							({ \
								volatile unsigned short wVal; \
								wVal = (*(volatile unsigned short *)(uAddr)); \
								wVal; \
							})

#define SYS_ReadDWord(uAddr) \
							({ \
								volatile unsigned long dwVal; \
								dwVal = (*(volatile unsigned long *)(uAddr)); \
								dwVal; \
							})
//sdhd vpo hal operation
#define	sdhd_VP_REG_BASEADDR		0xB8006100
#define	sdhd_VOU_REG_BASEADDR	0xB8006000
#define sdhd_vp_WriteWord(dwOffset,dwRegisterValue)		SYS_WriteWord(sdhd_VP_REG_BASEADDR+dwOffset, dwRegisterValue)
#define sdhd_vp_ReadWord(dwOffset)				SYS_ReadWord(sdhd_VP_REG_BASEADDR+dwOffset)
#define sdhd_vp_WriteDword(dwOffset,dwRegisterValue)		SYS_WriteDWord(sdhd_VP_REG_BASEADDR+dwOffset, dwRegisterValue)
#define sdhd_vp_ReadDword(dwOffset)				SYS_ReadDWord(sdhd_VP_REG_BASEADDR+dwOffset)
#define sdhd_vou_WriteDword(dwOffset,dwRegisterValue)	SYS_WriteDWord(sdhd_VOU_REG_BASEADDR+dwOffset, dwRegisterValue)
#define sdhd_vou_ReadDword(dwOffset)				SYS_ReadDWord(sdhd_VOU_REG_BASEADDR+dwOffset)
#define TVE_SDHD_BASE    0xb8008000
#define TVE_WRITEB_SDHD(index, data) SYS_WriteByte(TVE_SDHD_BASE+index, data)
#define TVE_WRITEW_SDHD(index, data) SYS_WriteWord(TVE_SDHD_BASE+index, data)
#define TVE_WRITED_SDHD(index, data) SYS_WriteDWord(TVE_SDHD_BASE+index, data)
#define TVE_READB_SDHD(index) ((SYS_ReadByte(TVE_SDHD_BASE+index)))
#define TVE_READW_SDHD(index) ((SYS_ReadWord(TVE_SDHD_BASE+index)))
#define TVE_READD_SDHD(index) ((SYS_ReadDWord(TVE_SDHD_BASE+index)))
//setting for bit6-32 of vou 0x00 register
#define	VOU_EX_VSYNC_P		0x00	//bit23
#define	VOU_EX_HSYNC_P		0x00	//bit22
#define	VOU_EX_SAVEAV_ONOFF	0x01	//bit21
#define	VOU_BLANKING_SEL		0x00	//bit20	0x00:0x10,0x80;	0x01: 0x80,0x10
#define	VOU_IN_ACTIVE_P		0x01	//bit19
#define	VOU_EX_ACTIVE_P		0x01	//bit18
#define	VOU_IN_VSYNC_P		0x00	//bit17
#define	VOU_IN_HSYNC_P		0x00	//bit11
#define	VOU_BIT_MODE			0x00	//bit9	0x00:10bits mode	0x01:8bits mode
#define	VOU_BTB_ONOFF			0x00	//bit8	Blanck than blank on/off

#define	VOU_OUT_COM			0x00	//bit16
#define 	VOU_VSYNC_POLARIT_HDMI	0x00	//bit14
#define 	VOU_HSYNC_POLARIT_HDMI	0x00	//bit13
#define 	VOU_C_CHANNEL_SEL	0x00	//bit12
#define  	VOU_DATA_CLIP			0x01	//bit10		//changed: 2008/03/06
//default value for vou external output's databits and color mode
#define	VOU_EXT_DATA_BITS		0x02//0x00	//bit4-5	0x00:10bits	0x01:20bits	0x02:30bits
#define	VOU_EXT_COLOR_MODE	0x01//0x00	//bit1	0x00:ycbcr output	0x01:rgb output

#define	DVI_SCAN_PROGRESSIVE		0x01
#define	DVI_SCAN_INTERLACE		0x02
#define	VOU_3601_ISR_FRAME_END		0x01
#define	VOU_3601_ISR_ODD_FIELD_END	0x02
#define	VOU_3601_ISR_VBLNNK			0x04
#define	VOU_3601_ISR_HBLANK			0x08
#define	VOU_3601_ISR_LM1			0x10
#define	VOU_3601_ISR_LM2			0x20
//tve_sdhd
#define BOOL				unsigned char
#define UINT8			unsigned char
#define UINT16			unsigned short
#define UINT32 			unsigned long
#define INT32       		long
#define TRUE 			1
#define FALSE 			0
enum VPOutputData
{
	VP_PAL = 0,
	VP_NTSC,
	VP_720_25,
	VP_720_30,
	VP_1080_25,
	VP_1080_30,
};
// TV system
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
	LINE_1080_30	//added for s3601
};
struct virtual_register
{
	unsigned long	reg_0x80;
	unsigned long	reg_0xf0;
};
struct virtual_register	virtual_reg;
enum tve_dac_output   //confirm to Hardware
{
	DAC_CAV_Y,
	DAC_CAV_PB,
	DAC_CAV_PR,
	DAC_CAV_RGB_R,
	DAC_CAV_RGB_G,
	DAC_CAV_RGB_B,
	DAC_CVBS,
	DAC_YC_Y,
	DAC_YC_C,
	DAC_SD_DAC1,//9
	DAC_SD_DAC2,//10
	DAC_SD_DAC3//11
};
enum DacType
{
	CVBS_1 = 0,
	CVBS_2,
	CVBS_3,
	CVBS_4,
	CVBS_5,
	CVBS_6,	
	SVIDEO_1,
	SVIDEO_2,
	SVIDEO_3,	
	YUV_1,
	YUV_2,	
	RGB_1,
	RGB_2,
	SVIDEO_Y_1,
	SECAM_CVBS1,
	SECAM_CVBS2,
	SECAM_CVBS3,
	SECAM_CVBS4,
	SECAM_CVBS5,
	SECAM_CVBS6,	
	SECAM_SVIDEO1,
	SECAM_SVIDEO2,
	SECAM_SVIDEO3
};
struct MiniDacIndex
{
	unsigned char uDacFirst;     // for all   (CVBS , YC_Y ,YUV_Y,RGB_R)
	unsigned char uDacSecond;   //for SVideo & YUV & RGB	(YC_C ,YUV_U,RGB_G)
	unsigned char uDacThird;   //for YUV & RGB	(YUV_V,RGB_B)
};

enum tve_tv_mode
{
	HD1080I30HZ,
	HD1080I25HZ,
	HD720P60HZ,
	HD720P50HZ,
	SD480P60HZ,
	SD576P50HZ,
	SD480I30HZ,
	SD576I25HZ
}; 
#define	TVE_HD_CAV_YUV	0x00
#define	TVE_HD_CAV_RGB	0x01
//============== the filter coeff for sdhd scale ==========
int mini_g_vp_8tap_32group_coeff[32][8] = 
{
	//posi7 posi6  posi5  posi4 posi3 posi2   posi1  posi0
	//tap7	tap6   tap5   tap4 tap3   tap2    tap1   tap0
	{0,     0,     0,     0,   256,     0,     0,     0},//0
	{0,     1,    -2,     7,   256,    -7,     2,    -1},//1
	{0,     1,    -5,    15,   254,   -13,     4,    -1},//2
	{0,     2,    -7,    23,   252,   -18,     6,    -1},//3
	{-1,     3,   -10,    32,   249,   -23,     7,    -2},//4
	{-1,     3,   -12,    41,   246,   -27,     9,    -2},//5
	{-1,     4,   -15,    51,   241,   -31,    10,    -2},//6
	{-1,     5,   -18,    61,   235,   -34,    11,    -3},//7
	{-1,     6,   -21,    71,   229,   -37,    12,    -3},//8
	{-2,     7,   -24,    82,   222,   -39,    12,    -3},//9
	{-2,     8,   -26,    92,   215,   -40,    13,    -3},//10
	{-2,     9,   -29,   103,   206,   -41,    13,    -3},//11
	{-2,     9,   -31,   114,   197,   -42,    13,    -3},//12
	{-2,    10,   -34,   125,   188,   -42,    13,    -3},//13
	{-2,    11,   -36,   136,   178,   -41,    13,    -3},//14
	{-3,    11,   -37,   147,   168,   -40,    12,    -3},//15
	{-3,    12,   -39,   158,   158,   -39,    12,    -3},//16
	{-3,    12,   -40,   168,   147,   -37,    11,    -3},//17
	{-3,    13,   -41,   178,   136,   -36,    11,    -2},//18
	{-3,    13,   -42,   188,   125,   -34,    10,    -2},//19
	{-3,    13,   -42,   197,   114,   -31,     9,    -2},//20
	{-3,    13,   -41,   206,   103,   -29,     9,    -2},//21
	{-3,    13,   -40,   215,    92,   -26,     8,    -2},//22
	{-3,    12,   -39,   222,    82,   -24,     7,    -2},//23
	{-3,    12,   -37,   229,    71,   -21,     6,    -1},//24
	{-3,    11,   -34,   235,    61,   -18,     5,    -1},//25
	{-2,    10,   -31,   241,    51,   -15,     4,    -1},//26
	{-2,     9,   -27,   246,    41,   -12,     3,    -1},//27
	{-2,     7,   -23,   249,    32,   -10,     3,    -1},//28
	{-1,     6,   -18,   252,    23,    -7,     2,     0},//29
	{-1,     4,   -13,   254,    15,    -5,     1,     0},//30
	{-1,     2,    -7,   256,     7,    -2,     1,     0}//31
};

int mini_g_vp_4tap_32group_coeff[32][4] = 
{
	//posi7 posi6 posi1  posi0
	//tap3  tap2 tap1    tap0
	{0,     0,   256,     0},//0
	{0,     5,   256,    -4},//1
	{-1,    10,   254,    -7},//2
	{-1,    16,   251,   -10},//3
	{-1,    22,   248,   -12},//4
	{-2,    30,   243,   -14},//5
	{-2,    37,   236,   -15},//6
	{-3,    46,   230,   -16},//7
	{-4,    55,   222,   -17},//8
	{-5,    64,   213,   -17},//9
	{-5,    74,   204,   -17},//10
	{-6,    84,   194,   -16},//11
	{-7,    95,   184,   -15},//12
	{-8,   106,   173,   -15},//13
	{-9,   117,   162,   -14},//14
	{-10,   128,   151,   -13},//15
	{-12,   140,   140,   -12},//16
	{-13,   151,   128,   -10},//17
	{-14,   162,   117,    -9},//18
	{-15,   173,   106,    -8},//19
	{-15,   184,    95,    -7},//20
	{-16,   194,    84,    -6},//21
	{-17,   204,    74,    -5},//22
	{-17,   213,    64,    -5},//23
	{-17,   222,    55,    -4},//24
	{-16,   230,    46,    -3},//25
	{-15,   236,    37,    -2},//26
	{-14,   243,    30,    -2},//27
	{-12,   248,    22,    -1},//28
	{-10,   251,    16,    -1},//29
	{-7,   254,    10,    -1},//30
	{-4,   256,     5,     0}//31
};
int mini_g_vp_2tap_32group_coeff[32][2] = 
{
	//posi2 //p1
	{0,   256},
    {1,   255},
    {2,   254},
    {3,   253},
    {4,   252},
    {7,   249},
    {10,   246},
    {14,   242},
    {20,   236},
    {27,   229},
    {36,   220},
    {47,   209},
    {60,   196},
    {75,   181},
    {91,   165},
   {109,  147},
   {128,   128},
   {147,   109},
   {165,    91},
   {181,    75},
   {196,    60},
   {209,    47},
   {220,    36},
   {229,    27},
   {236,    20},
   {242,   14},
   {246,    10},
   {249,     7},
   {252,     4},
   {253,    3},
   {254,     2},
   {255,     1}
};
enum VPOutputData VP_N_OutType;
//the extern  variable for dac and logo setting 
extern unsigned char  g_sdhd_blogo_format ;//bootloader logo parameter
extern unsigned char sdhd_DAC_bProgressive ;  
//the following are from dec_logo_m36_mini.c
extern unsigned char progressive_frame;
extern unsigned char *dec_frm_buf_y;
extern unsigned char *dec_frm_buf_c;
extern unsigned int hd_wOrigHeight,hd_wOrigWidth,sd_wOrigHeight,sd_wOrigWidth;
extern unsigned char *dec_frm_buf_y_hd ;
extern unsigned char *dec_frm_buf_c_hd ;
//0x80 bit9
void mini_hal_component_color_mode(unsigned char  byuv)
{
	virtual_reg.reg_0x80 = (byuv)?(virtual_reg.reg_0x80 | (0x01<<9)):(virtual_reg.reg_0x80 & (~(0x01<<9)));
}
void mini_hal_dac_config(unsigned char  index/*0,1,2,3,4,5*/, unsigned char dac_output)
{
	TVE_WRITEB_SDHD(0x84, (((index+1)&0xF)<<4)|(dac_output&0xF));
}

//0x84 bit8-13 = 0x85 bit0-5
void mini_hal_dac_onoff(unsigned char index,unsigned char bon)
{
	unsigned char	tmp8 = TVE_READB_SDHD(0x85);
	tmp8 = (!bon)?(tmp8| (0x01<<(index))):(tmp8 & (~(0x01<<(index))));
	TVE_WRITEB_SDHD(0x85, tmp8);
}
//0xff bit0-10
void mini_hal_burst_set_phase(unsigned long  phase)
{
	virtual_reg.reg_0xf0 = (virtual_reg.reg_0xf0 & 0xFFFF0000)|(phase&0xFFFF);
}

void mini_hal_input_sync_active_level(unsigned char h_low_active,unsigned char v_low_active)
{
	virtual_reg.reg_0x80 = (v_low_active)?(virtual_reg.reg_0x80 | (0x01<<6)):(virtual_reg.reg_0x80 & (~(0x01<<6)));
	virtual_reg.reg_0x80 = (v_low_active)?(virtual_reg.reg_0x80 | (0x01<<7)):(virtual_reg.reg_0x80 & (~(0x01<<7)));
}
//0x80 bit8
void mini_hal_update_register(void)
{
	virtual_reg.reg_0x80 = virtual_reg.reg_0x80 | (0x01<<8);
}
void mini_sub_update_register(void)
{
	TVE_WRITED_SDHD(0xf0, virtual_reg.reg_0xf0);
	TVE_WRITED_SDHD(0x80, virtual_reg.reg_0x80);
	virtual_reg.reg_0x80 &= 0xFFFFFEEF;//bit4,8
}
//0x80 bit12-13,14
void mini_hal_pal_mode(unsigned char ispal/*bit14*/,unsigned char pal_mode)
{
	virtual_reg.reg_0x80 = (ispal)?(virtual_reg.reg_0x80 | (0x01<<14)):(virtual_reg.reg_0x80 & (~(0x01<<14)));
	virtual_reg.reg_0x80 = (virtual_reg.reg_0x80 & 0xFFFFCFFF)|((pal_mode&0x03)<<12);
}
//0x8c bit30/ level 0x84 bit12-14 / line 0x80 bit16-19,20-23,24-27,28-31
void mini_hal_pedestal(unsigned char  benable,unsigned char  level,unsigned char  start_line_top,unsigned char  end_line_top,unsigned char start_line_bot,unsigned char  end_line_bot)
{
	unsigned long tmp32 = TVE_READD_SDHD(0x8c);
	tmp32 = (benable)?(tmp32 | (0x01<<30)):(tmp32 & (~(0x01<<30)));
	tmp32 = (tmp32 & 0xFFFF8FFF)|((level&0x07)<<12);
	TVE_WRITED_SDHD(0x8c, tmp32);
	virtual_reg.reg_0x80 = (virtual_reg.reg_0x80 & 0xFFFF)|((end_line_top&0x0F)<<16)|((start_line_top&0x0F)<<20)|((end_line_bot&0x0F)<<24)|((start_line_bot&0x0F)<<28);
}
//0x80 bit0-2
inline mini_hal_set_tv_mode(enum tve_tv_mode tv_mode)
{
	virtual_reg.reg_0x80 = (virtual_reg.reg_0x80 & 0xFFFFFFF8)|(tv_mode&0x07);
}
//0x90 bit16-23,24-31,0x94 bit0-7
void mini_hal_component_rgb_level(unsigned char  r_level,unsigned char  g_level,unsigned char  b_level)
{
	TVE_WRITEB_SDHD(0x92, r_level);
	TVE_WRITEB_SDHD(0x93, g_level);
	TVE_WRITEB_SDHD(0x94, b_level);
}
//0x80 bit3
void mini_hal_dac_upsamplen(unsigned char benable)
{
	virtual_reg.reg_0x80 = (benable)?(virtual_reg.reg_0x80 | (0x01<<3)):(virtual_reg.reg_0x80 & (~(0x01<<3)));
}
//0x80 bit10
void mini_hal_black_than_black_enable(unsigned char benable)
{
	virtual_reg.reg_0x80 = (benable)?(virtual_reg.reg_0x80 | (0x01<<10)):(virtual_reg.reg_0x80 & (~(0x01<<10)));
}

//0x90 bit0-7,8-15
void mini_hal_component_yc_level(unsigned char y_level,unsigned char c_level)
{
	TVE_WRITEB_SDHD(0x90, y_level);
	TVE_WRITEB_SDHD(0x91, c_level);
}
void mini_hal_composite_yc_level(unsigned char y_level,unsigned char c_level)
{
	unsigned long tmp32 = TVE_READD_SDHD(0xe4);
	tmp32 = (tmp32 & 0xFFFF)|(y_level<<24)|(c_level<<16);
	TVE_WRITED_SDHD(0xe4, tmp32);
}
//0xe4 bit8-10,12-14
void mini_hal_composite_yc_delay(unsigned char y_delay,unsigned char c_delay)
{
	unsigned long tmp32 = TVE_READD_SDHD(0xe4);
	tmp32 = (tmp32 & 0xFFFF8FFF)|((y_delay&0x07)<<12);
	tmp32 = (tmp32 & 0xFFFFF8FF)|((c_delay&0x07)<<8);
	TVE_WRITED_SDHD(0xe4, tmp32);
}
void mini_hal_component_ycbcr_delay(unsigned char y_delay,unsigned char cb_delay,unsigned char cr_delay)
{
	unsigned long tmp32 = TVE_READD_SDHD(0x8c);
	tmp32 = (tmp32 & 0xF888FFFF)|((y_delay&0x07)<<16)|((cb_delay&0x07)<<20)|((cr_delay&0x07)<<24);
	TVE_WRITED_SDHD(0x8c, tmp32);
}
void mini_hal_composite_filter_mode(unsigned char y_filter,unsigned c_filter)
{
	unsigned long tmp32 = TVE_READD_SDHD(0xe4);
	tmp32 = (y_filter)?(tmp32 | (0x01<<15)):(tmp32 & (~(0x01<<15)));
	tmp32 = (c_filter)?(tmp32 | (0x01<<11)):(tmp32 & (~(0x01<<11)));
	TVE_WRITED_SDHD(0xe4, tmp32);
}

//0x8c bit29,28
void mini_hal_component_filter_mode(unsigned char  y_filter,unsigned char c_filter)
{
	unsigned long tmp32 = TVE_READD_SDHD(0x8c);
	tmp32 = (y_filter)?(tmp32 | (0x01<<28)):(tmp32 & (~(0x01<<28)));
	tmp32 = (c_filter)?(tmp32 | (0x01<<29)):(tmp32 & (~(0x01<<29)));
	TVE_WRITED_SDHD(0x8c, tmp32);
}
//0x8c bit4-7
void mini_hal_component_sync_level(unsigned char sync_level)
{
	unsigned long tmp32 = TVE_READD_SDHD(0x8c);
	tmp32 = (tmp32 & 0xFFFFFF0F)|((sync_level&0x0F)<<4);
	TVE_WRITED_SDHD(0x8c, tmp32);
}
//0xe4 bit4-6
void mini_hal_composite_sync_delay(unsigned char sync_delay)
{
	unsigned long  tmp32 = TVE_READD_SDHD(0xe4);
	tmp32 = (tmp32 & 0xFFFFFF8F)|((sync_delay&0x07)<<4);
	TVE_WRITED_SDHD(0xe4, tmp32);
}

//0xe4 bit0-3
void mini_hal_composite_sync_level(unsigned char  sync_level)
{
	unsigned long tmp32 = TVE_READD_SDHD(0xe4);
	tmp32 = (tmp32 & 0xFFFFFFF0)|(sync_level&0x0F);
	TVE_WRITED_SDHD(0xe4, tmp32);
}
/*0x8c bit1-3*/
void mini_hal_component_rgb_sync_onoff(unsigned char r_sync_on,unsigned char  g_sync_on,unsigned char  b_sync_on)
{
	unsigned long tmp32 = TVE_READD_SDHD(0x8c);
	tmp32 = (r_sync_on)?(tmp32 | (0x01<<1)):(tmp32 & (~(0x01<<1)));
	tmp32 = (g_sync_on)?(tmp32 | (0x01<<2)):(tmp32 & (~(0x01<<2)));
	tmp32 = (b_sync_on)?(tmp32 | (0x01<<3)):(tmp32 & (~(0x01<<3)));
	TVE_WRITED_SDHD(0x8c, tmp32);
}

//0xff bit20 0xe8 bit0-7,8-15
void mini_hal_burst_set_level(unsigned char  bset, unsigned char  cb_level,unsigned char  cr_level)
{
	virtual_reg.reg_0xf0 = (bset)?(virtual_reg.reg_0xf0 | (0x01<<20)):(virtual_reg.reg_0xf0 & (~(0x01<<20)));
	TVE_WRITEW_SDHD(0xe8, (cb_level)|(cr_level<<8));
}
void mini_sub_set_tv_mode_para(enum tve_tv_mode tv_mode, unsigned long sub_mode)
{
	//TVE_SDHD_PRINTF("Into <%s> tv_mode<%d>\n",__FUNCTION__,tv_mode);
	mini_hal_set_tv_mode(tv_mode);
	switch(tv_mode)
	{
		case  HD1080I30HZ:
			TVE_WRITED_SDHD(0x80,0x000000c0);
			
			TVE_WRITED_SDHD(0x90,0x47475052);

			TVE_WRITED_SDHD(0x94,0x012a0047);
			TVE_WRITED_SDHD(0x98,0x00880036);
			TVE_WRITED_SDHD(0x9c,0x01cb021d);
			TVE_WRITED_SDHD(0xc4,0x130aaaaa);
			TVE_WRITED_SDHD(0xcc,0x00600070);
			TVE_WRITED_SDHD(0x80,0x000001c0);		
		//	TVE_WRITED_SDHD(0x8c,TVE_READD_SDHD(0x8c)&0xbfffffff);
			TVE_WRITED_SDHD(0x8c,0x3003B030);
			 mini_hal_component_rgb_level(0x45,0x45,0x45);
			//sub_sdhd_ttx_enable(FALSE);
			//sub_sdhd_cc_enable(FALSE);
                     //sub_sdhd_cgms_enable(g_tve_cgms);
                     //sub_sdhd_wss_enable(FALSE);
			mini_hal_dac_upsamplen(FALSE);
					 
			break;
		case  HD1080I25HZ:
			mini_hal_pal_mode(0,0);
			mini_hal_black_than_black_enable(FALSE);
			mini_hal_component_yc_level(0x052,0x50);
			TVE_WRITED_SDHD(0x94,0x012a0047);
			TVE_WRITED_SDHD(0x98,0x00880036);
			TVE_WRITED_SDHD(0x9c,0x01cb021d);
			mini_hal_component_rgb_level(0x45,0x45,0x45);
		//	TVE_WRITED_SDHD(0x8c,TVE_READD_SDHD(0x8c)&0xbfffffff);
			TVE_WRITED_SDHD(0x8c,0x3003B030);

			//sub_sdhd_ttx_enable(FALSE);
			//sub_sdhd_cc_enable(FALSE);
                     //sub_sdhd_cgms_enable(g_tve_cgms);
                     //sub_sdhd_wss_enable(FALSE);
			mini_hal_dac_upsamplen(FALSE);
			break;
		case  HD720P60HZ:
			mini_hal_pal_mode(0,0);
			mini_hal_black_than_black_enable(FALSE);
			mini_hal_component_yc_level(0x052,0x50);
			TVE_WRITED_SDHD(0x94,0x012a0047);
			TVE_WRITED_SDHD(0x98,0x00880036);
			TVE_WRITED_SDHD(0x9c,0x01cb021d);
			mini_hal_component_rgb_level(0x45,0x45,0x45);
		//	TVE_WRITED_SDHD(0x8c,TVE_READD_SDHD(0x8c)&0xbfffffff);
			TVE_WRITED_SDHD(0x8c,0x3003B030);

			//sub_sdhd_ttx_enable(FALSE);
			//sub_sdhd_cc_enable(FALSE);
                     //sub_sdhd_cgms_enable(g_tve_cgms);
                     //sub_sdhd_wss_enable(FALSE);
			mini_hal_dac_upsamplen(FALSE);
			break;
		case  HD720P50HZ:
			mini_hal_pal_mode(0,0);
			mini_hal_black_than_black_enable(FALSE);
			mini_hal_component_yc_level(0x052,0x50);
			TVE_WRITED_SDHD(0x94,0x012a0047);
			TVE_WRITED_SDHD(0x98,0x00880036);
			TVE_WRITED_SDHD(0x9c,0x01cb021d);
			mini_hal_component_rgb_level(0x45,0x45,0x45);
		//	TVE_WRITED_SDHD(0x8c,TVE_READD_SDHD(0x8c)&0xbfffffff);
			TVE_WRITED_SDHD(0x8c,0x3003B030);

			//sub_sdhd_ttx_enable(FALSE);
			//sub_sdhd_cc_enable(FALSE);
                     //sub_sdhd_cgms_enable(g_tve_cgms);
                     //sub_sdhd_wss_enable(FALSE);
			mini_hal_dac_upsamplen(FALSE);
			break;
		case  SD480P60HZ:
			mini_hal_pal_mode(0,0);
			mini_hal_black_than_black_enable(FALSE);
			mini_hal_component_yc_level(0x052,0x50);
			TVE_WRITED_SDHD(0x94,0x012a0047);
			TVE_WRITED_SDHD(0x98,0x00d00064);
			TVE_WRITED_SDHD(0x9c,0x01980205);
			mini_hal_component_rgb_level(0x45,0x45,0x45);
		//	TVE_WRITED_SDHD(0x8c,TVE_READD_SDHD(0x8c)&0xbfffffff);
			TVE_WRITED_SDHD(0x8c,0x3003B030);
			//sub_sdhd_ttx_enable(FALSE);
			//sub_sdhd_cc_enable(FALSE);
                     //sub_sdhd_cgms_enable(g_tve_cgms);
                     //sub_sdhd_wss_enable(FALSE);
			mini_hal_dac_upsamplen(TRUE);
			break;
		case  SD576P50HZ:
			mini_hal_pal_mode(0,0);
			mini_hal_black_than_black_enable(FALSE);
			mini_hal_component_yc_level(0x052,0x50);
			TVE_WRITED_SDHD(0x94,0x012a0047);
			TVE_WRITED_SDHD(0x98,0x00d00064);
			TVE_WRITED_SDHD(0x9c,0x01980205);
			mini_hal_component_rgb_level(0x45,0x45,0x45);
		//	TVE_WRITED_SDHD(0x8c,TVE_READD_SDHD(0x8c)&0xbfffffff);
			TVE_WRITED_SDHD(0x8c,0x3003B030);
			//sub_sdhd_ttx_enable(FALSE);
			//sub_sdhd_cc_enable(FALSE);
                     //sub_sdhd_cgms_enable(FALSE);
                     //sub_sdhd_wss_enable(FALSE);
			mini_hal_dac_upsamplen(TRUE);
			break;
		case  SD480I30HZ:
			if(sub_mode == 0)  //NTSC
				mini_hal_pal_mode(0,0);
			else if(sub_mode == 1) //N443
				mini_hal_pal_mode(0,3);
			else if(sub_mode == 2) //PAL_M
				mini_hal_pal_mode(1,2);
			else if(sub_mode == 3)	//PAL_60
				mini_hal_pal_mode(1,3);

			mini_hal_black_than_black_enable(FALSE);
			mini_hal_composite_filter_mode(TRUE, FALSE);
			TVE_WRITED_SDHD(0x94,0x012a0047);
			TVE_WRITED_SDHD(0x98,0x00d00064);
			TVE_WRITED_SDHD(0x9c,0x01980205);
#if 0
        		if(m_TVEAdjust != NULL)
        		{
        		       hal_composite_yc_level(m_TVEAdjust[TVE_ADJ_COMPOSITE_LUMA_LEVEL*2+1].value,m_TVEAdjust[TVE_ADJ_COMPOSITE_CHRMA_LEVEL*2+1].value);
        		       hal_composite_yc_delay(m_TVEAdjust[TVE_ADJ_COMPOSITE_Y_DELAY*2+1].value,m_TVEAdjust[TVE_ADJ_COMPOSITE_C_DELAY*2+1].value);
        		       hal_component_ycbcr_delay(m_TVEAdjust[TVE_ADJ_COMPONENT_Y_DELAY*2+1].value,m_TVEAdjust[TVE_ADJ_COMPONENT_CB_DELAY*2+1].value,m_TVEAdjust[TVE_ADJ_COMPONENT_CR_DELAY*2+1].value);
        			hal_burst_set_phase(m_TVEAdjust[TVE_ADJ_PHASE_COMPENSATION*2+1].value);
        		}
        		else
        		{
#endif
        		       mini_hal_composite_yc_level(0x4d,0x63);
        			mini_hal_composite_yc_delay(0x08,0x03);
        			mini_hal_component_ycbcr_delay(0x3,0x0,0x0);
        			mini_hal_burst_set_phase(0x3000);
//        		}
#if 0
        		if(m_TVEAdjustAdv != NULL)
        		{
        			hal_pedestal(m_TVEAdjustAdv[TVE_ADJ_ADV_PEDESTAL_ONOFF*2+1].value,m_TVEAdjustAdv[TVE_ADJ_ADV_COMPONENT_PEDESTAL_LEVEL*2+1].value,0x5,0x7,0x4,0xf);
        			hal_component_yc_level(m_TVEAdjustAdv[TVE_ADJ_ADV_COMPONENT_LUM_LEVEL*2+1].value,m_TVEAdjustAdv[TVE_ADJ_ADV_COMPONENT_CHRMA_LEVEL*2+1].value);
			       hal_component_sync_level(m_TVEAdjustAdv[TVE_ADJ_ADV_COMPONENT_SYNC_LEVEL*2+1].value);
			       hal_component_rgb_level(m_TVEAdjustAdv[TVE_ADJ_ADV_RGB_R_LEVEL*2+1].value,m_TVEAdjustAdv[TVE_ADJ_ADV_RGB_G_LEVEL*2+1].value,m_TVEAdjustAdv[TVE_ADJ_ADV_RGB_B_LEVEL*2+1].value);
			       hal_composite_sync_level(m_TVEAdjustAdv[TVE_ADJ_ADV_COMPOSITE_SYNC_LEVEL*2+1].value);
        		}
        		else
        		{
#endif
			       mini_hal_pedestal(TRUE,0xB,0x5,0x7,0x4,0xf);
			       mini_hal_component_yc_level(0x4c,0x50);
			       mini_hal_component_sync_level(0x3);
			       mini_hal_component_rgb_level(0x45,0x45,0x45);
			       mini_hal_composite_sync_level(0x04);
//        		}
			mini_hal_composite_sync_delay(0x00);
			//TVE_WRITED_SDHD(0x8c,0x7003B04e);
			mini_hal_component_filter_mode(TRUE,TRUE);
			mini_hal_component_rgb_sync_onoff(FALSE, FALSE, FALSE);//(TRUE,TRUE,TRUE);
			//sub_sdhd_ttx_enable(FALSE);
			//sub_sdhd_cc_enable(g_tve_cc);
                     //sub_sdhd_cgms_enable(g_tve_cgms);
                     //sub_sdhd_wss_enable(FALSE);
			if((sub_mode == 2)||(sub_mode == 3))
			 	mini_hal_burst_set_level(1,0x96,0x6a);
			else
				mini_hal_burst_set_level(1,0xd4,0x00);
			mini_hal_dac_upsamplen(TRUE);		 
			break;
		case  SD576I25HZ:
			if(sub_mode == 0)
				mini_hal_pal_mode(1,0);
			else
				mini_hal_pal_mode(1,1);
			mini_hal_black_than_black_enable(FALSE);
			mini_hal_composite_filter_mode(TRUE, FALSE);
			TVE_WRITED_SDHD(0x94,0x012a0047);
			TVE_WRITED_SDHD(0x98,0x00d00064);
			TVE_WRITED_SDHD(0x9c,0x01980205);
#if 0
        		if(m_TVEAdjust != NULL)
        		{
        		       hal_composite_yc_level(m_TVEAdjust[TVE_ADJ_COMPOSITE_LUMA_LEVEL*2].value,m_TVEAdjust[TVE_ADJ_COMPOSITE_CHRMA_LEVEL*2].value);
        		       hal_composite_yc_delay(m_TVEAdjust[TVE_ADJ_COMPOSITE_Y_DELAY*2].value,m_TVEAdjust[TVE_ADJ_COMPOSITE_C_DELAY*2].value);
        		       hal_component_ycbcr_delay(m_TVEAdjust[TVE_ADJ_COMPONENT_Y_DELAY*2].value,m_TVEAdjust[TVE_ADJ_COMPONENT_CB_DELAY*2].value,m_TVEAdjust[TVE_ADJ_COMPONENT_CR_DELAY*2].value);
        			hal_burst_set_phase(m_TVEAdjust[TVE_ADJ_PHASE_COMPENSATION*2].value);
        		}
        		else
        		{
 #endif
			       mini_hal_composite_yc_level(0x52,0x6d);
			       mini_hal_composite_yc_delay(0x00,0x03);
        			mini_hal_component_ycbcr_delay(0x3,0x0,0x0);
        			mini_hal_burst_set_phase(0x2210);
//        		}
#if 0
        		if(m_TVEAdjustAdv != NULL)
        		{
        			hal_pedestal(m_TVEAdjustAdv[TVE_ADJ_ADV_PEDESTAL_ONOFF*2].value,m_TVEAdjustAdv[TVE_ADJ_ADV_COMPONENT_PEDESTAL_LEVEL*2].value,0x6,0x6,0x6,0x9);
        			hal_component_yc_level(m_TVEAdjustAdv[TVE_ADJ_ADV_COMPONENT_LUM_LEVEL*2].value,m_TVEAdjustAdv[TVE_ADJ_ADV_COMPONENT_CHRMA_LEVEL*2].value);
			       hal_component_rgb_level(m_TVEAdjustAdv[TVE_ADJ_ADV_RGB_R_LEVEL*2].value,m_TVEAdjustAdv[TVE_ADJ_ADV_RGB_G_LEVEL*2].value,m_TVEAdjustAdv[TVE_ADJ_ADV_RGB_B_LEVEL*2].value);
			       hal_composite_sync_level(m_TVEAdjustAdv[TVE_ADJ_ADV_COMPOSITE_SYNC_LEVEL*2].value);
        		}
        		else
        		{
#endif
        			mini_hal_pedestal(FALSE,0xB,0x6,0x6,0x6,0x9);
				mini_hal_component_sync_level(0x3);
			       mini_hal_component_yc_level(0x052,0x50);
			       mini_hal_component_rgb_level(0x45,0x45,0x45);
			       mini_hal_composite_sync_level(0x04);
//        		}
			
			mini_hal_composite_sync_delay(0x00);
			//sub_sdhd_ttx_enable(g_tve_ttx);
			//sub_sdhd_cc_enable(FALSE);
                    // sub_sdhd_cgms_enable(FALSE);
                     //sub_sdhd_wss_enable(g_tve_wss);
			mini_hal_burst_set_level(1,0x96,0x6a);
			mini_hal_dac_upsamplen(TRUE);
			break;
		default:
			return ;
	}
		return ;
}

void mini_vp_sub_sdhd_tvsys_vpout(enum TVSystem eTVSys,enum VPOutputData *eOutputMode)
{
	switch(eTVSys)
	{
		case PAL:
		case PAL_N:	
		case PAL_60:
			*eOutputMode = VP_PAL;
			break;
		case PAL_M:
		case NTSC:
		case NTSC_443:
			*eOutputMode = VP_NTSC;
			break;
		case LINE_720_25:
		case LINE_720_30:
			if(eTVSys == LINE_720_25)
				*eOutputMode = VP_720_25;
			else
				*eOutputMode = VP_720_30;
			break;
		case LINE_1080_25:
		case LINE_1080_30:
			if(eTVSys == LINE_1080_25)
				*eOutputMode = VP_1080_25;
			else
				*eOutputMode = VP_1080_30;
			break;
		default:
			break;
	}
	return ;
}

void mini_vp_sdhd_tvsys_screen_size(enum TVSystem eTVSys,unsigned short  *screen_width, unsigned short *screen_height)
{
	//VP_DBG_PRINTF("\n==========<%s>-Into\n",__FUNCTION__);
	//pISRFactor->eTVSys = eTVSys;	

	switch(eTVSys)
	{
		case PAL:
		case PAL_N:	
		case PAL_60:
			*screen_width = 720;
			*screen_height = 576;
			break;
		case PAL_M:
		case NTSC:
		case NTSC_443:
			*screen_width = 720;
			*screen_height = 480;
			break;
		case LINE_720_25:
		case LINE_720_30:
			*screen_width = 1280;
			*screen_height = 720;
			break;
		case LINE_1080_25:
		case LINE_1080_30:
			*screen_width = 1920;
			*screen_height = 1080;
			break;
		default:
			break;
	}
	return ;
}

void mini_sub_convert_internal_mode(enum TVSystem eTVSys,unsigned char  bprogressive,enum tve_tv_mode	*pinternal_mode, unsigned long  *sub_mode)
{
	//TVE_SDHD_PRINTF("Into <%s>:<%d><%d>\n",__FUNCTION__,eTVSys,bprogressive);
	switch(eTVSys)
	{
		case PAL:
			*sub_mode = 0;
			*pinternal_mode = (bprogressive)?SD576P50HZ:SD576I25HZ;
			break;
		case PAL_N:
			*sub_mode = 1;
			*pinternal_mode = (bprogressive)?SD576P50HZ:SD576I25HZ;
			break;
		case NTSC:
			*sub_mode = 0;
			*pinternal_mode = (bprogressive)?SD480P60HZ:SD480I30HZ;
			break;
		case NTSC_443:
			*sub_mode = 1;
			*pinternal_mode = (bprogressive)?SD480P60HZ:SD480I30HZ;
			break;
		case PAL_M:
			*sub_mode = 2;
			*pinternal_mode = (bprogressive)?SD480P60HZ:SD480I30HZ;
			break;
		case PAL_60:
			*sub_mode = 3;
			*pinternal_mode = (bprogressive)?SD480P60HZ:SD480I30HZ;
			break;
		case LINE_720_25:
			if(!bprogressive)
				return ;
			else
				*pinternal_mode = HD720P50HZ;
			break;
		case LINE_720_30:
			if(!bprogressive)
				return ;
			else
				*pinternal_mode = HD720P60HZ;
			break;
		case LINE_1080_25:
			if(bprogressive)
				return ;
			else
				*pinternal_mode = HD1080I25HZ;
			break;
		case LINE_1080_30:
			if(bprogressive)
				return ;
			else
				*pinternal_mode = HD1080I30HZ;
			break;
		default:
			return ;
	}
	return ;
}
void sdhd_tvenc_set_dac(enum DacType eDacType, unsigned char *SelDacInd)
{
	//struct tve_sdhd_priv_info *ppriv_info = (struct tve_sdhd_priv_info *)(dev->priv);
	unsigned char  reg_dac_index1,reg_dac_index2,reg_dac_index3;
	enum tve_tv_mode			tv_internal_mode;
	struct MiniDacIndex *DacIndex,InputDacIndex ;
	DacIndex=&InputDacIndex;
	unsigned long  i=0;
	DacIndex->uDacFirst  = *((unsigned char *)SelDacInd);
	DacIndex->uDacSecond =  *((unsigned char *)SelDacInd+1);
	DacIndex->uDacThird = *((unsigned char *)SelDacInd+2);
	switch(eDacType)
	{
		case CVBS_1:
		case CVBS_2:
		case CVBS_3:
		case CVBS_4:
		case CVBS_5:
		case CVBS_6:
			reg_dac_index1 = DacIndex->uDacFirst;
			mini_hal_dac_config(reg_dac_index1, DAC_CVBS);
			mini_hal_dac_onoff(reg_dac_index1, TRUE);
			break;
		case SVIDEO_1:
		case SVIDEO_2:
		case SVIDEO_3:
			reg_dac_index1 = DacIndex ->uDacFirst;
			reg_dac_index2 = DacIndex ->uDacSecond;
			mini_hal_dac_config(reg_dac_index1, DAC_YC_Y);
			mini_hal_dac_config(reg_dac_index2, DAC_YC_C);
			mini_hal_dac_onoff(reg_dac_index1, TRUE);		
			mini_hal_dac_onoff(reg_dac_index2, TRUE);
			break;
		case YUV_1:
		case YUV_2:
			reg_dac_index1 = DacIndex ->uDacFirst;
			reg_dac_index2 = DacIndex ->uDacSecond;
			reg_dac_index3 = DacIndex ->uDacThird;
			mini_hal_component_color_mode(TVE_HD_CAV_YUV);
			mini_hal_dac_config(reg_dac_index1, DAC_CAV_Y);
			mini_hal_dac_config(reg_dac_index2, DAC_CAV_PB);
			mini_hal_dac_config(reg_dac_index3, DAC_CAV_PR);
			mini_hal_dac_onoff(reg_dac_index1, TRUE);
			mini_hal_dac_onoff(reg_dac_index2, TRUE);
			mini_hal_dac_onoff(reg_dac_index3, TRUE);

			break;			
		case RGB_1:
		case RGB_2:
			reg_dac_index1 = DacIndex ->uDacFirst;
			reg_dac_index2 = DacIndex ->uDacSecond;
			reg_dac_index3 = DacIndex ->uDacThird;
			mini_hal_component_color_mode(TVE_HD_CAV_RGB);
			mini_hal_dac_config(reg_dac_index1, DAC_CAV_RGB_R);
			mini_hal_dac_config(reg_dac_index2, DAC_CAV_RGB_G);
			mini_hal_dac_config(reg_dac_index3, DAC_CAV_RGB_B);
			mini_hal_dac_onoff(reg_dac_index1, TRUE);
			mini_hal_dac_onoff(reg_dac_index2, TRUE);
			mini_hal_dac_onoff(reg_dac_index3, TRUE);
			break;			
		default:
			break;
			//{ASSERT(0);}
	}

	//MEMCPY(&(ppriv_info->dac_info[eDacType]),pInfo,sizeof(struct VP_DacInfo));
	//ppriv_info->bprogressive = pInfo->bProgressive;
	//ppriv_info->dac_mask |= reg_dac_mask;

	mini_hal_update_register();
	mini_sub_update_register();
	//TVE_SDHD_PRINTF("Leave <%s>\n",__FUNCTION__);
	return ;
}
void mini_tve_s3601_sdhd_set_tvsys(enum TVSystem eTVSys,unsigned char  bProgressive)
{
	enum tve_tv_mode      		tv_internal_mode;
	unsigned long  sub_mode = 0xffffffff;
	//TVE_SDHD_PRINTF("Into <%s>\n",__FUNCTION__);
	mini_sub_convert_internal_mode(eTVSys, bProgressive, &tv_internal_mode, &sub_mode);
	mini_sub_set_tv_mode_para(tv_internal_mode, sub_mode);
	mini_hal_update_register();
	mini_sub_update_register();	
	//TVE_SDHD_PRINTF("Leave <%s>\n",__FUNCTION__);
	return ;
}
//the following function is for sdhd_VPN initial registers
//0x4C - 0x5C
//enhancement function
//vp_sub_sdhd_initregister
void mini_sdhd_enhance_write(unsigned char  index, unsigned short value)
{
	sdhd_vp_WriteDword(0x4c, (value<<16)|(0x01<<4)|(index&0x0F));
}
//0x54 adaptive compenstator 1
//vp_sub_sdhd_initregister
void mini_sdhd_adpative_compen_threshold_1(unsigned char strict_frame_diff,unsigned char loose_edge, unsigned char  loose_frame, unsigned char  loose_adjacent_diff, unsigned char  edge_compensate)
{
	sdhd_vp_WriteDword(0x54, (edge_compensate<<24)|((loose_adjacent_diff&0x3F)<<18)|((loose_frame &0x3F)<<12)|((loose_edge &0x3F)<<6)|(strict_frame_diff&0x3F));
}
//0x58 adaptive compenstator 2
//vp_sub_sdhd_initregister
void mini_sdhd_adpative_compen_threshold_2(unsigned char  duplicate_bob,unsigned char mid_vertical, unsigned char  mid_frame_diff, unsigned char  mid_1,unsigned char  mide_2)
{
	sdhd_vp_WriteDword(0x58, ((mide_2&0x3F)<<24)|((mid_1&0x3F)<<18)|((mid_frame_diff &0x3F)<<12)|((mid_vertical &0x3F)<<6)|(duplicate_bob&0x3F));
}
void mini_sdhd_vp_filt_noise_reduc_threshold(unsigned char  hf,unsigned char hf_diff,unsigned char hpf_coeff)
{
	unsigned long	dwTmp = sdhd_vp_ReadDword(0x5c)&0xfff80000;//clear bit 18:0
	dwTmp |= ((hpf_coeff&0x07)<<16)|(hf_diff<<8)|hf;
	sdhd_vp_WriteDword(0x5c, dwTmp);
}
void mini_sdhd_output_mode(enum VPOutputData eOutputMode,unsigned char  boutput_progressive)
{
	unsigned long	dwTmp = sdhd_vp_ReadDword(0x90)&0xFFFFCEFF;//clear bit8,12,13
	unsigned char	output_format = 0;
	if(boutput_progressive) dwTmp |= (0x01<<8);
	switch(eOutputMode)
	{
		case VP_PAL:
		case VP_NTSC:
			output_format = 0;
			break;
		case VP_720_25:
		case VP_720_30:
			output_format = 1;
			break;
		case VP_1080_25:
		case VP_1080_30:
			output_format = 2;
			break;
	}
	dwTmp |= ((output_format&0x03)<<12);
	sdhd_vp_WriteDword(0x90, dwTmp);	
}
void mini_sdhd_filter_group_num(unsigned char y_h, unsigned char c_h,unsigned char y_v, unsigned char  c_v)
{
	sdhd_vp_WriteDword(0x20, (((y_h-1)&0x1F)<<24)|(((y_v-1)&0x1F)<<16)|(((c_h-1)&0x1F)<<8)|((c_v-1)&0x1F));
}
//0x94 bit11-8
//0x98 bit3-0
void mini_hal_sdhd_lat(void)
{
	unsigned char i;
	unsigned long val = 0x00000800;

	for(i = 0; i < 8; i ++)
	{
		sdhd_vp_WriteDword(0x94, (sdhd_vp_ReadDword(0x94)&0xfffff0ff)|val);
		sdhd_vp_WriteDword(0x98, (sdhd_vp_ReadDword(0x98)&0xffffff00)|0x0000000f);
		val += 0x00000100;
	}
}
//0x00:6-23
void mini_vou_hal_control(void)
{
	unsigned long	tmp_32 = sdhd_vou_ReadDword(0x00);

	tmp_32 &= 0x3F; //clear 6-23
	tmp_32 |= (VOU_EX_VSYNC_P<<23)|(VOU_EX_HSYNC_P<<22)|(VOU_EX_ACTIVE_P<<18)|(VOU_EX_SAVEAV_ONOFF<<21);
	tmp_32 |= (VOU_IN_VSYNC_P<<17)|(VOU_IN_HSYNC_P<<11)|(VOU_IN_ACTIVE_P<<19);
	tmp_32 |= (VOU_BLANKING_SEL<<20)|(VOU_VSYNC_POLARIT_HDMI<<14)|(VOU_HSYNC_POLARIT_HDMI<<13);
	tmp_32 |= (VOU_C_CHANNEL_SEL<<12)|(VOU_DATA_CLIP<<10)|(VOU_BIT_MODE<<9)|(VOU_BTB_ONOFF<<8);
	sdhd_vou_WriteDword(0x00, tmp_32);
}
void mini_vou_hal_enable_isr(unsigned char mask_bit)
{
	sdhd_vou_WriteDword(0x04, (sdhd_vou_ReadDword(0x04) & 0xFFFFFF00)|mask_bit);
}
void mini_vou_hal_set_lm_position(unsigned char lm_index,unsigned short lm_position)
{
	unsigned long tmp = sdhd_vou_ReadDword(0x30);
	if(lm_index == 0)
		sdhd_vou_WriteDword(0x30, tmp & 0xFFFF0000|lm_position);
	else if(lm_index == 1)
		sdhd_vou_WriteDword(0x30, tmp & 0x0000FFFF|(lm_position<<16));
}
void mini_vou_hal_extern_sync_polarity(unsigned char  bh_level1,unsigned char bv_level1)
{
	unsigned long	tmp_32 = sdhd_vou_ReadDword(0x00);
	tmp_32 = (bh_level1)?(tmp_32|(0x01<<22)):(tmp_32&(~(0x01<<22)));
	tmp_32 = (bv_level1)?(tmp_32|(0x01<<23)):(tmp_32&(~(0x01<<23)));
	sdhd_vou_WriteDword(0x00, tmp_32);
}
/*
//not support HDMI yet
void mini_vou_hal_sync_polarity_hdmi(unsigned char bh_level1,unsigned char bv_level1)
{
	unsigned long	tmp_32 = sdhd_vou_ReadDword(0x00);
	tmp_32 = (bh_level1)?(tmp_32|(0x01<<13)):(tmp_32&(~(0x01<<13)));
	tmp_32 = (bv_level1)?(tmp_32|(0x01<<14)):(tmp_32&(~(0x01<<14)));
	sdhd_vou_WriteDword(0x00, tmp_32);
}
*/
//added hal function to support ycbcr->rgb
void mini_vou_hal_set_ycbcr2rgb_coef(unsigned char  bhd_output)
{
	if(bhd_output)
	{
		sdhd_vou_WriteDword(0x64, 0x0037012a);
		sdhd_vou_WriteDword(0x68, 0x021d0089);
		sdhd_vou_WriteDword(0x6C, 0x000001cb);
	}
	else
	{
		sdhd_vou_WriteDword(0x64, 0x0064012a);
		sdhd_vou_WriteDword(0x68, 0x020500d0);
		sdhd_vou_WriteDword(0x6C, 0x00000198);
	}
}
//0x00:7
void mini_vou_hal_hd_sd_sel(unsigned char  bhd)
{
	unsigned long tmp_32 = sdhd_vou_ReadDword(0x00);
	tmp_32 = (bhd)?(tmp_32|0x80):(tmp_32&(~0x80));
	sdhd_vou_WriteDword(0x00, tmp_32);
}
//#define VP_S3601_MODIFY_TIMING_FOR_HDMI
void mini_vou_hal_config_para(enum VPOutputData vp_output_mode,unsigned char vou_scan_mode)
{
	
	if(vou_scan_mode == 0)
		vou_scan_mode = DVI_SCAN_INTERLACE;
	SYS_WriteDWord(0xB800006C, (SYS_ReadDWord(0xB800006C)&0xFFFFE0FF)|(0x08<<8));//the delay for external clock to HDMI
	SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFFFFFCFF)|(0x02<<8));//de clock select ->108MHz
	switch(vp_output_mode)
	{
		case VP_PAL:
			mini_vou_hal_extern_sync_polarity(FALSE,FALSE);
			//mini_vou_hal_sync_polarity_hdmi(FALSE,FALSE); not support HDMI yet
			mini_vou_hal_set_ycbcr2rgb_coef(FALSE);
			mini_vou_hal_hd_sd_sel(0);
			if(vou_scan_mode==DVI_SCAN_PROGRESSIVE)
			{
				SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFC3F3CFF) | (0x01<<24) | (0x01<<22)| (0x02<<8));//sd progressive
				SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFFFFFFF3) | (0x00<<2));
				SYS_WriteDWord(0xB80000c0, (SYS_ReadDWord(0xB80000c0)&0xFFCFFFFF) | (0x00<<20));
				sdhd_vou_WriteDword(0x0c,0x027106c0);
				sdhd_vou_WriteDword(0x10,0x008006c0);
				sdhd_vou_WriteDword(0x14,0x06a80104);
				sdhd_vou_WriteDword(0x18,0x00050271);
				sdhd_vou_WriteDword(0x1C,0x07ff07ff);
				sdhd_vou_WriteDword(0x20,0x00001fff);
				sdhd_vou_WriteDword(0x24,0x026c002c);
				sdhd_vou_WriteDword(0x28,0x07ff07ff);
				sdhd_vou_WriteDword(0x2C,0x027107ff);
			}
			else
			{
				SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFC3F3CFF) | (0x00<<24) | (0x00<<22)| (0x02<<8));// sd interlace
				SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFFFFFFF3) | (0x00<<2));
				SYS_WriteDWord(0xB80000c0, (SYS_ReadDWord(0xB80000c0)&0xFFCFFFFF) | (0x00<<20));
				sdhd_vou_WriteDword(0x0c,0x027106c0);
//#ifdef VP_S3601_MODIFY_TIMING_FOR_HDMI  //not support HDMI yet
//				sdhd_vou_WriteDword(0x10,0x007e06c0/*0x008006c0*/);//rachel:7/31:modify for hdmi.h_sync_length: 128->126 //0x008006c0=>0x007e06c0
//#else
				sdhd_vou_WriteDword(0x10,0x008006c0);
//#endif
				sdhd_vou_WriteDword(0x14,0x06a80104);
				sdhd_vou_WriteDword(0x18,0x00030271);//rachel_to_do: should be 8030271 according to pg?
				sdhd_vou_WriteDword(0x1C, 0x093c0939);	//Oncer: modified based on new PG
			//	vou_WriteDword(0x1C,0x013b0139|(0x01<<27)|(0x01<<11)|(0x01<<16));//rachel_to_do: should be 13b0939 according to pg? //bit27/11. set1/1 if interlace and 00 if progressvie
				sdhd_vou_WriteDword(0x20,0x00000360);
				sdhd_vou_WriteDword(0x24,0x01360016);
				sdhd_vou_WriteDword(0x28,0x026f014f);
				sdhd_vou_WriteDword(0x2C,0x02710138);
			}
			break;
		case VP_NTSC:
			mini_vou_hal_extern_sync_polarity(FALSE,FALSE);
			//mini_vou_hal_sync_polarity_hdmi(FALSE,FALSE); not support HDMI yet
			mini_vou_hal_set_ycbcr2rgb_coef(FALSE);
			mini_vou_hal_hd_sd_sel(0);
			if(vou_scan_mode==DVI_SCAN_PROGRESSIVE)
			{
				SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFC3F3CFF) | (0x01<<24) | (0x01<<22)| (0x02<<8));//sd progressive
				SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFFFFFFF3) | (0x00<<2));
				SYS_WriteDWord(0xB80000c0, (SYS_ReadDWord(0xB80000c0)&0xFFCFFFFF) | (0x00<<20));
				sdhd_vou_WriteDword(0x0c,0x020d06b4);
				sdhd_vou_WriteDword(0x10,0x007c06b4);
				sdhd_vou_WriteDword(0x14,0x069400f0);
				sdhd_vou_WriteDword(0x18,0x000c0006);
				sdhd_vou_WriteDword(0x1C,0x07ff07ff);
				sdhd_vou_WriteDword(0x20,0x00001fff);
//#ifdef VP_S3601_MODIFY_TIMING_FOR_HDMI
//				sdhd_vou_WriteDword(0x24,0x020a002a/*0x020d002a*/); //rachel:7/31:modify for hdmi. vertical line 483->480 and then the front porch size is updated from 6 to 9 at the same time //0x020d002a => 0x020a002a
//#else
				sdhd_vou_WriteDword(0x24,0x020d002a);
//#endif
				sdhd_vou_WriteDword(0x28,0x07ff07ff);
				sdhd_vou_WriteDword(0x2C,0x000607ff);
			}
			else
			{
				SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFC3F3CFF) | (0x00<<24) | (0x00<<22)| (0x02<<8));//sd interlace
				SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFFFFFFF3) | (0x00<<2));
				SYS_WriteDWord(0xB80000c0, (SYS_ReadDWord(0xB80000c0)&0xFFCFFFFF) | (0x00<<20));
				sdhd_vou_WriteDword(0x0c,0x020d06b4);
//#ifdef VP_S3601_MODIFY_TIMING_FOR_HDMI   not support HDMI yet
//				sdhd_vou_WriteDword(0x10,0x007c06b4/*0x008006b4*/);//rachel:7/31:modify for hdmi.h_sync_length: 128->124 //0x008006b4=>0x007c06b4
//				sdhd_vou_WriteDword(0x14,0x068E00ea/*0x069400f0*/);//rachel:7/31:modify for hdmi. back porch size = h_active_start - h_synce_end = 116->114. so the h_active_posi move forward = (128-124)+(116-114) = 6. so the front porch size is 6 more than before. 0x069400f0 => 0x069400ea
//#else
				sdhd_vou_WriteDword(0x10,0x008006b4);
				sdhd_vou_WriteDword(0x14,0x069400f0);
//#endif				
				sdhd_vou_WriteDword(0x18,0x00060003);
				sdhd_vou_WriteDword(0x1C,0x010d010a|(0x01<<27)|(0x01<<11));
				sdhd_vou_WriteDword(0x20,0x0000035a);
				sdhd_vou_WriteDword(0x24,0x01050015);
				sdhd_vou_WriteDword(0x28,0x020c011c);
				sdhd_vou_WriteDword(0x2C,0x00030109);
			}
			break;
		case VP_720_25:
			mini_vou_hal_extern_sync_polarity(TRUE,TRUE);
			//mini_vou_hal_sync_polarity_hdmi(TRUE,TRUE); not support HDMI yet
			mini_vou_hal_set_ycbcr2rgb_coef(TRUE);
			mini_vou_hal_hd_sd_sel(1);
			SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFC3F3CFF) | (0x02<<24) | (0x02<<22)| (0x02<<8));//hd 
			SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFFFFFFF3) | (0x02<<2));
			SYS_WriteDWord(0xB80000c0, (SYS_ReadDWord(0xB80000c0)&0xFFCFFFFF) | (0x02<<20));
			//must be progressive
			sdhd_vou_WriteDword(0x0c,0x02ee0f78);
			sdhd_vou_WriteDword(0x10,0x00500f78);
			sdhd_vou_WriteDword(0x14,0x0c080204);
			sdhd_vou_WriteDword(0x18,0x000502ee);
			sdhd_vou_WriteDword(0x1C,0x07ff07ff);
			sdhd_vou_WriteDword(0x20,0x00001fff);
			sdhd_vou_WriteDword(0x24,0x02e90019);
			sdhd_vou_WriteDword(0x28,0x07ff07ff);
			sdhd_vou_WriteDword(0x2C,0x02ee07ff);
			break;
		case VP_720_30:
			mini_vou_hal_extern_sync_polarity(TRUE,TRUE);
			//mini_vou_hal_sync_polarity_hdmi(TRUE,TRUE); not support HDMI yet
			mini_vou_hal_set_ycbcr2rgb_coef(TRUE);
			mini_vou_hal_hd_sd_sel(1);
			SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFC3F3CFF) | (0x02<<24) | (0x02<<22)| (0x02<<8));//hd 
			SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFFFFFFF3) | (0x02<<2));
			SYS_WriteDWord(0xB80000c0, (SYS_ReadDWord(0xB80000c0)&0xFFCFFFFF) | (0x02<<20));
			//must be progressive
			sdhd_vou_WriteDword(0x0c,0x02ee0ce4);
			sdhd_vou_WriteDword(0x10,0x00500ce4);
			sdhd_vou_WriteDword(0x14,0x0c080204);
			sdhd_vou_WriteDword(0x18,0x000502ee);
			sdhd_vou_WriteDword(0x1C,0x07ff07ff);
			sdhd_vou_WriteDword(0x20,0x00001fff);
			sdhd_vou_WriteDword(0x24,0x02e90019);
			sdhd_vou_WriteDword(0x28,0x07ff07ff);
			sdhd_vou_WriteDword(0x2C,0x02ee07ff);
			break;
		case VP_1080_25:
			mini_vou_hal_extern_sync_polarity(TRUE,TRUE);
			//mini_vou_hal_sync_polarity_hdmi(TRUE,TRUE); not support HDMI yet
			mini_vou_hal_set_ycbcr2rgb_coef(TRUE);
			mini_vou_hal_hd_sd_sel(1);
			SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFC3F3CFF) | (0x02<<24) | (0x02<<22)| (0x02<<8));//hd 
			SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFFFFFFF3) | (0x02<<2));
			SYS_WriteDWord(0xB80000c0, (SYS_ReadDWord(0xB80000c0)&0xFFCFFFFF) | (0x02<<20));
			//must be interlace
			sdhd_vou_WriteDword(0x0c,0x046514a0);
			sdhd_vou_WriteDword(0x10,0x005814a0);
			sdhd_vou_WriteDword(0x14,0x1080017c);
			sdhd_vou_WriteDword(0x18,0x00050465);
			sdhd_vou_WriteDword(0x1C, 0x0a380a33);	//Oncer: modified based on new PG
		//	vou_WriteDword(0x1C,0x02370233|(0x01<<27)|(0x01<<11));//rachel_to_do should set 0x02370233 accordiing to pg?
			sdhd_vou_WriteDword(0x20,0x00000a50);
			sdhd_vou_WriteDword(0x24,0x02300014);
			sdhd_vou_WriteDword(0x28,0x04630247);
			sdhd_vou_WriteDword(0x2C,0x04650233);
			break;
		case VP_1080_30:
			mini_vou_hal_extern_sync_polarity(TRUE,TRUE);
			//mini_vou_hal_sync_polarity_hdmi(TRUE,TRUE); not support HDMI yet
			mini_vou_hal_set_ycbcr2rgb_coef(TRUE);
			mini_vou_hal_hd_sd_sel(1);
			SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFC3F3CFF) | (0x02<<24) | (0x02<<22)| (0x02<<8));//hd 
			SYS_WriteDWord(0xB8000078, (SYS_ReadDWord(0xB8000078)&0xFFFFFFF3) | (0x02<<2));
			SYS_WriteDWord(0xB80000c0, (SYS_ReadDWord(0xB80000c0)&0xFFCFFFFF) | (0x02<<20));
			//must be interlace
			sdhd_vou_WriteDword(0x0c,0x04651130);
			sdhd_vou_WriteDword(0x10,0x00581130);
			sdhd_vou_WriteDword(0x14,0x1080017c);
			sdhd_vou_WriteDword(0x18,0x00050465);
			sdhd_vou_WriteDword(0x1C, 0x0a380a33);	//Oncer: modified based on new PG
		//	vou_WriteDword(0x1C,0x02370233|(0x01<<27)|(0x01<<11));//rachel_to_do should set 0x02370233 accordiing to pg?
			sdhd_vou_WriteDword(0x20,0x00000898);			
			sdhd_vou_WriteDword(0x24,0x02300014);		
			sdhd_vou_WriteDword(0x28,0x04630247);
			sdhd_vou_WriteDword(0x2C,0x04650233);
			break;
	}
}
//0x00:1
//added for hdmi input:select rgb or ycbcr
void mini_vou_hal_external_colormode(unsigned char brgb)
{
	unsigned long tmp_32 = sdhd_vou_ReadDword(0x00);
	tmp_32 = (brgb)?(tmp_32|0x02):(tmp_32&(~0x02));
	sdhd_vou_WriteDword(0x00, tmp_32);
	
}
//0x00:4-5
void mini_vou_hal_external_bitsmode(unsigned char output_mode)
{
	unsigned long	tmp_32 = sdhd_vou_ReadDword(0x00);
	tmp_32 &= 0xFFFFFFCF;
	tmp_32 |= (output_mode&0x03)<<4;
	sdhd_vou_WriteDword(0x00, tmp_32);
}
void mini_vou_hal_initreg(enum VPOutputData vp_output_mode,unsigned char  vou_scan_mode,unsigned char isr_enable,unsigned short top_lm,unsigned short bot_lm)
{
	mini_vou_hal_control();
	mini_vou_hal_enable_isr(isr_enable);
	mini_vou_hal_set_lm_position(0,top_lm);
	mini_vou_hal_set_lm_position(1,bot_lm);
	mini_vou_hal_config_para(vp_output_mode,vou_scan_mode);
	mini_vou_hal_external_colormode(VOU_EXT_COLOR_MODE);
	mini_vou_hal_external_bitsmode(VOU_EXT_DATA_BITS);
}

//0x00:0
void mini_vou_hal_Playback(BOOL bplay)
{
	UINT32 tmp_32 = sdhd_vou_ReadDword(0x00);
	tmp_32 = (bplay)?(tmp_32|0x01):(tmp_32&(~0x01));
	sdhd_vou_WriteDword(0x00, tmp_32);
}


// for DE_N scale 
struct mini_VP_Scale_Factor
{
	//From Decoder
	unsigned long 					uSrcHeight;					//Original picture height
	unsigned long 					uSrcWidth;					//Original picture width	
	UINT8 					uPanScanOffset;			//Offset in source picture for Pan & scan mode
	//From API	VPO_SetOutput
	enum VPOutputData		eOutputMode;			//Current Output TV system
	UINT16					screen_width;
	UINT16					screen_height;
	BOOL					bprogressive;

	UINT32					y_v_inc;
	UINT32					c_v_inc;
	INT32					y_v_o_normal;
	INT32					c_v_o_normal;
	UINT16					input_fetch_width;
	UINT16					input_fetch_height;
	UINT16					output_scan_act_width;
	UINT16					output_scan_act_height;

	BOOL					b_frame_fetch;
	BOOL					b_vertical_2tap;
	UINT8					src_bot;
	UINT8					out_bot;
	UINT32					y_o_init;
	UINT32					y_e_init;
	UINT32					c_o_init;
	UINT32					c_e_init;
	UINT32					reg_y_o_init;
	UINT32					reg_y_e_init;
	UINT32					reg_c_o_init;
	UINT32					reg_c_e_init;
	UINT8					scale_init_case;	
	BOOL					bDublicateCoeff;
	//enum DisplayMode eRealDisplayMode;
    int                     sar_width;
    int                     sar_height;
    enum TVSystem 		    src_tv_sys;	

	BOOL					bIFrameNotify;

	BOOL					is_source_output;
	//BOOL					bSwscaleAfd;

	//enum AfdZoomType		afdzoomtype;
	BOOL 					hor_size_same;
	BOOL					dit_enable;
};
#define sdhd_WriteRegisterArrayForZoom(dwIndex,dwRegisterValue) \
	sdhd_vp_WriteDword(dwIndex, dwRegisterValue)
#define sdhd_RegisterArraySetMaskForZoom() \
	sdhd_vp_WriteDword(0x90, sdhd_vp_ReadDword(0x90)&0xfffffffe) | 1)
#define sdhd_RegisterArrayResetMaskForZoom() \
	sdhd_vp_WriteDword(0x90, (sdhd_vp_ReadDword(0x90)&0xfffffffe) | 0)
#define sdhd_WriteZoomRegisters() 

#define	mini_SCALE_H_INIT_LSB2LSB(x)	(x&0x7FF)
#define	mini_SCALE_H_INIT_LSB2MSB(x)	((x>>11)&0x1F)
#define	mini_SCALE_H_INIT_LSB2INT(x)	((x>>16)&0x1F)
#define	mini_SCALE_V_INIT_LSB2LSB(x)	(x&0x7F)
#define	mini_SCALE_V_INIT_LSB2MSB(x)	((x>>7)&0x1F)
#define	mini_SCALE_V_INIT_LSB2INT(x)	(((x>>12)&0x1F)| ((x&0x80000000)>>26))
#define	mini_SCALE_H_INC_LSB2LSB(x)	(x&0x7FF)
#define	mini_SCALE_H_INC_LSB2MSB(x)	((x>>11)&0x1F)
#define	mini_SCALE_H_INC_LSB2INT(x)	((x>>16)&0x1F)
#define	mini_SCALE_V_INC_LSB2LSB(x)	(x&0x7F)
#define	mini_SCALE_V_INC_LSB2MSB(x)	((x>>7)&0x1F)
#define	mini_SCALE_V_INC_LSB2INT(x)	((x>>12)&0x1F)
#define	mini_SCALE_H_INT2LSB(x)		(x<<16)
#define	mini_SCALE_V_INT2LSB(x)		(x<<12)
#define	VPO_COEFF_TYPE_Y_H	0
#define	VPO_COEFF_TYPE_Y_V	1
#define	VPO_COEFF_TYPE_C_H	2
#define	VPO_COEFF_TYPE_C_V	3
void mini_hal_sdhd_scale(unsigned char reg_index,unsigned long value)
{
	sdhd_vp_WriteDword(reg_index,value);
}
//0x90 bit6,7,16
//vp_sub_sdhd_initregister
void mini_hal_sdhd_scale_para(BOOL bduplicate, BOOL v_y_2tap, BOOL v_c_2tap)
{
	unsigned long	dwTmp = sdhd_vp_ReadDword(0x90)&0xFFFEFF3F;//clear bit6,7,16
	//bduplicate = TRUE;
	if(bduplicate) dwTmp |= (0x01<<16);
	if(v_y_2tap) dwTmp |= (0x01<<7);
	if(v_c_2tap) dwTmp |= (0x01<<6);
	sdhd_vp_WriteDword(0x90, dwTmp);	
}
void mini_hal_sdhd_scale_coeffeient(UINT8 reg_index,UINT32 value)
{
	sdhd_vp_WriteDword(reg_index,value);
}

void mini_vp_sub_sdhd_scale_reset_init(struct mini_VP_Scale_Factor *pscale_factor,BOOL output_bot,BOOL src_fetch_bot)
{
	long	y_o_init=0,y_e_init=0,c_o_init=0,c_e_init=0;
	unsigned long	y_v_inc = pscale_factor->y_v_inc;
	unsigned long 	c_v_inc = pscale_factor->c_v_inc;
	unsigned short	intput_fetch_height=0,output_scan_act_height=0;
	
	intput_fetch_height = pscale_factor->input_fetch_height;
	output_scan_act_height = pscale_factor->output_scan_act_height;
	if(pscale_factor->dit_enable == TRUE && !pscale_factor->bprogressive)
	{
		output_scan_act_height = pscale_factor->output_scan_act_height>>1;
	}

	if((!pscale_factor->b_frame_fetch) && (!pscale_factor->bprogressive))
	{
		if(!src_fetch_bot)
		{
			y_o_init = pscale_factor->y_v_o_normal;//0;
			c_o_init = (y_o_init-(mini_SCALE_V_INT2LSB(1)>>2))>>1;//(y_o_init-0.25)/2
			y_e_init = y_o_init+(y_v_inc>>1);//y_o+inc/2=inc/2
			c_e_init = (y_e_init-(mini_SCALE_V_INT2LSB(1)>>2))>>1;//(y_e-0.25)/2
			pscale_factor->scale_init_case = 0;
		}
		else if(src_fetch_bot)
		{
			y_o_init = pscale_factor->y_v_o_normal - (mini_SCALE_V_INT2LSB(1)>>1);//y_o=-0.5
			c_o_init = (y_o_init-0xC00)>>1;//(y_o_init-0.75)/2
			y_e_init = y_o_init + (y_v_inc>>1);//y_o_init+inc/2
			c_e_init = (y_e_init-0xC00)>>1;//(y_e_init-0.75)/2
			pscale_factor->scale_init_case = 1;
		}
	}
	else if((!pscale_factor->b_frame_fetch) && (pscale_factor->bprogressive))
	{
		//fixed setting
		if(!src_fetch_bot)
		{
			y_o_init = pscale_factor->y_v_o_normal;//0;
			c_o_init = (y_o_init-(mini_SCALE_V_INT2LSB(1)>>2))>>1;//(y_o_init-0.25)/2
			pscale_factor->scale_init_case = 2;
		}
		else
		{
			y_o_init = pscale_factor->y_v_o_normal - (mini_SCALE_V_INT2LSB(1)>>1);//y_o=2-0.5
			c_o_init = (y_o_init-0xC00)>>1;//(y_o_init-0.75)/2=-0.625
			pscale_factor->scale_init_case = 3;
		}
	}
	else if((pscale_factor->b_frame_fetch) && (pscale_factor->bprogressive))
	{
		y_o_init = pscale_factor->y_v_o_normal;//0;
		c_o_init = (y_o_init-(mini_SCALE_V_INT2LSB(1)>>1))>>1;//(y_o_init-0.5)/2=-0.25
		pscale_factor->scale_init_case = 4;
	}
	else if((pscale_factor->b_frame_fetch) && (!pscale_factor->bprogressive))
	{
		y_o_init = pscale_factor->y_v_o_normal;//0;
		c_o_init = (y_o_init-(mini_SCALE_V_INT2LSB(1)>>1))>>1;//(y_o_init-0.5)/2=-0.25
		y_e_init = y_o_init + (y_v_inc>>1);//y_o_init+inc/2
		c_e_init = (y_e_init-(mini_SCALE_V_INT2LSB(1)>>1))>>1;//(y_e_init-0.5)/2
		pscale_factor->scale_init_case = 5;
	}
	pscale_factor->src_bot = src_fetch_bot;
	pscale_factor->out_bot = output_bot;
	pscale_factor->y_o_init = y_o_init;
	pscale_factor->y_e_init = y_e_init;
	pscale_factor->c_o_init = c_o_init;
	pscale_factor->c_e_init = c_e_init;
	pscale_factor->reg_y_o_init = ((mini_SCALE_V_INIT_LSB2MSB(y_o_init))<<24)|((mini_SCALE_V_INIT_LSB2LSB(y_o_init))<<8)|(mini_SCALE_V_INIT_LSB2INT(y_o_init));
	pscale_factor->reg_y_e_init = ((mini_SCALE_V_INIT_LSB2MSB(y_e_init))<<24)|((mini_SCALE_V_INIT_LSB2LSB(y_e_init))<<8)|(mini_SCALE_V_INIT_LSB2INT(y_e_init));
	pscale_factor->reg_c_o_init = ((mini_SCALE_V_INIT_LSB2MSB(c_o_init))<<24)|((mini_SCALE_V_INIT_LSB2LSB(c_o_init))<<8)|(mini_SCALE_V_INIT_LSB2INT(c_o_init));
	pscale_factor->reg_c_e_init = ((mini_SCALE_V_INIT_LSB2MSB(c_e_init))<<24)|((mini_SCALE_V_INIT_LSB2LSB(c_e_init))<<8)|(mini_SCALE_V_INIT_LSB2INT(c_e_init));
		
	mini_hal_sdhd_scale(0x04, ((mini_SCALE_V_INIT_LSB2MSB(y_o_init))<<24)|((mini_SCALE_V_INIT_LSB2LSB(y_o_init))<<8)|(mini_SCALE_V_INIT_LSB2INT(y_o_init)));
	mini_hal_sdhd_scale(0xC8, ((mini_SCALE_V_INIT_LSB2MSB(y_e_init))<<24)|((mini_SCALE_V_INIT_LSB2LSB(y_e_init))<<8)|(mini_SCALE_V_INIT_LSB2INT(y_e_init)));
	mini_hal_sdhd_scale(0x0C, ((mini_SCALE_V_INIT_LSB2MSB(c_o_init))<<24)|((mini_SCALE_V_INIT_LSB2LSB(c_o_init))<<8)|(mini_SCALE_V_INIT_LSB2INT(c_o_init)));
	mini_hal_sdhd_scale(0xCC, ((mini_SCALE_V_INIT_LSB2MSB(c_e_init))<<24)|((mini_SCALE_V_INIT_LSB2LSB(c_e_init))<<8)|(mini_SCALE_V_INIT_LSB2INT(c_e_init)));
}
//0x74 main picture base offset in the memory covered in vpo_scale
//vp_sub_sdhd_MP
void mini_hal_sdhd_mp_offset_insource(UINT8 bHorOffset_mb, UINT16 bVerOffset_mb)
{
	sdhd_vp_WriteDword(0x74, bHorOffset_mb|(bVerOffset_mb<<16));
}
void mini_hal_sdhd_mp_size_insource(UINT16 width,UINT16 height)
{
	UINT32 tmp = sdhd_vp_ReadDword(0x90)&0x9FFFFFF3;//clear bit 2,3,29,30
	UINT8	tmp8 = 0;
	
	sdhd_vp_WriteDword(0x7c, (height<<16)|width);
	sdhd_vp_WriteDword(0x78, (sdhd_vp_ReadDword(0x78)&0xFFFF0000) | ((width+15)>>4));
	if(width<=720)
		tmp8 = 1;
	else if((width>720)&&(width<=960))
		tmp8 = 0;
	else if((width>960)&&(width<=1280))
		tmp8 = 2;
	else if((width>1280)&&(width<=1920))
		tmp8 = 3;
	else
		//{ASSERT(0);}
	tmp = tmp | ((tmp8&0x03)<<29);
       tmp8 = 0;
	tmp = tmp | ((tmp8&0x03)<<2);
	sdhd_vp_WriteDword(0x90, tmp);
}
//0x80 0x84 MP position and size on screen covered in vpo_scale 
//no used
void mini_hal_sdhd_mp_rectinscreen(UINT16 start_x, UINT16 end_x, UINT16 start_y, UINT16 end_y)
{
	sdhd_vp_WriteDword(0x80, (start_x<<16)|end_x);
	sdhd_vp_WriteDword(0x84, (start_y<<16)|end_y);
}
void mini_hal_set_scale_coeff_register(unsigned char tap, unsigned char group_num,unsigned char coeff_type,BOOL bDublicateCoeff)
{
	UINT8 i=0,j=0;
	UINT16 tmp1,tmp2,tmp3;
	for(j=0;j<group_num;j++)
	{
	    i=j;
	 	if(((coeff_type==0) || (coeff_type==1))&&(bDublicateCoeff==TRUE))//y_h,y_v
	 	{
	 		i=0;
	 	}
		if(tap == 8)
		{
			tmp2 = (mini_g_vp_8tap_32group_coeff[i][7-7]&0xFF)|((mini_g_vp_8tap_32group_coeff[i][7-7]&0x80000000)>>23);
			tmp1 = (mini_g_vp_8tap_32group_coeff[i][7-0]&0xFF)|((mini_g_vp_8tap_32group_coeff[i][7-0]&0x80000000)>>23);
			mini_hal_sdhd_scale_coeffeient(0x28, ((tmp2&0x1FF)<<16)|(tmp1&0x1FF)); //(position7 = tap7)<<16|(position0 = tap0)
			tmp3 = (mini_g_vp_8tap_32group_coeff[i][7-3]&0x1FF)|((mini_g_vp_8tap_32group_coeff[i][7-3]&0x80000000)>>22);
			tmp2 = (mini_g_vp_8tap_32group_coeff[i][7-2]&0x1FF)|((mini_g_vp_8tap_32group_coeff[i][7-2]&0x80000000)>>22);
			tmp1 = (mini_g_vp_8tap_32group_coeff[i][7-1]&0x1FF)|((mini_g_vp_8tap_32group_coeff[i][7-1]&0x80000000)>>22);
			mini_hal_sdhd_scale_coeffeient(0x2C, ((tmp3&0x3FF)<<20)|((tmp2&0x3FF)<<10)|(tmp1&0x3FF));//(position3<<20)|(position2<<10)|(position1)
			tmp3 = (mini_g_vp_8tap_32group_coeff[i][7-6]&0x1FF)|((mini_g_vp_8tap_32group_coeff[i][7-6]&0x80000000)>>22);
			tmp2 = (mini_g_vp_8tap_32group_coeff[i][7-5]&0x1FF)|((mini_g_vp_8tap_32group_coeff[i][7-5]&0x80000000)>>22);
			tmp1 = (mini_g_vp_8tap_32group_coeff[i][7-4]&0x1FF)|((mini_g_vp_8tap_32group_coeff[i][7-4]&0x80000000)>>22);
			mini_hal_sdhd_scale_coeffeient(0x30, ((tmp3&0x3FF)<<20)|((tmp2&0x3FF)<<10)|(tmp1&0x3FF));//(position6<<20)|(position5<<10)|(position4)
		}
		else if(tap == 4)
		{
			tmp2 = (mini_g_vp_4tap_32group_coeff[i][3-3]&0xFF)|((mini_g_vp_4tap_32group_coeff[i][3-3]&0x80000000)>>23);
			tmp1 = (mini_g_vp_4tap_32group_coeff[i][3-0]&0xFF)|((mini_g_vp_4tap_32group_coeff[i][3-0]&0x80000000)>>23);
			mini_hal_sdhd_scale_coeffeient(0x28, ((tmp2&0x1FF)<<16)|(tmp1&0x1FF));//(position7=tap3)<<16|(position0=tap0)
			tmp1 = (mini_g_vp_4tap_32group_coeff[i][3-1]&0x1FF)|((mini_g_vp_4tap_32group_coeff[i][3-1]&0x80000000)>>22);
			mini_hal_sdhd_scale_coeffeient(0x2C, (tmp1&0x3FF));//(position1=tap1)
			tmp3 = (mini_g_vp_4tap_32group_coeff[i][3-2]&0x1FF)|((mini_g_vp_4tap_32group_coeff[i][3-2]&0x80000000)>>22);
			mini_hal_sdhd_scale_coeffeient(0x30, ((tmp3&0x3FF)<<20));//(position6=tap2<<20)
		}
		else if(tap == 2)
		{
			mini_hal_sdhd_scale_coeffeient(0x28, 0); //position7|position0
			tmp1 = (mini_g_vp_2tap_32group_coeff[i][1-0]&0x1FF)|((mini_g_vp_2tap_32group_coeff[i][1-0]&0x80000000)>>22);
			mini_hal_sdhd_scale_coeffeient(0x2C, tmp1&0x3FF);//position3|position2|position1
			tmp1 = (mini_g_vp_2tap_32group_coeff[i][1-1]&0x1FF)|((mini_g_vp_2tap_32group_coeff[i][1-1]&0x80000000)>>22);
			mini_hal_sdhd_scale_coeffeient(0x30, (tmp1&0x3FF)<<20);//position6|position5|position4
		}
		mini_hal_sdhd_scale_coeffeient(0x24, ((coeff_type&0x03)<<8)|(j&0x1F));
	}
}
void mini_vp_sub_sdhd_scale_simple(int iWidth, int iHeight, enum TVSystem tv_mode)
{
	struct mini_VP_Scale_Factor *pPicInfo,RealpPicInfo;
	unsigned short 	source_selected_left,source_selected_width,source_selected_top,source_selected_height;
	unsigned short	display_left,display_width,display_top,display_height, tv_width, tv_height;
	unsigned short 	input_fetch_width,input_fetch_height,output_scan_act_width,output_scan_act_height;
	unsigned char		h_offset_mb,v_offset_mb;
	unsigned long		y_h_inc,y_v_inc,c_h_inc,c_v_inc, x_ratio, y_ratio;
	long				y_h_init,y_o_init=0,y_e_init=0,c_h_init,c_o_init=0,c_e_init=0;
	long				y_v_o_normal,c_v_o_normal;
	pPicInfo=&RealpPicInfo;
	source_selected_left = 0;
	source_selected_top = 0;
	source_selected_width= iWidth;
	source_selected_height= iHeight;
	unsigned short src_w = iWidth;
	unsigned short src_h = iHeight;
	display_left = 0;
	display_top = 0;
	display_width=  0xFFFF;
	display_height= 0xFFFF;
//set the screen size
	mini_vp_sdhd_tvsys_screen_size(g_sdhd_blogo_format,&display_width, &display_height);
//we use the frame fetch mode for DE_N of Mini driver  
	pPicInfo->b_frame_fetch = TRUE;
	input_fetch_height = source_selected_height;
	input_fetch_width	=source_selected_width;
	output_scan_act_width = display_width;
 //depend on interlace or progressive
	output_scan_act_height=(sdhd_DAC_bProgressive)?display_height:(display_height>>1);
 
 //here we simply the hor_size_same
       unsigned char hor_size_same = FALSE;
 	unsigned char bDublicateCoeff	=FALSE;
	if(display_width == iWidth)
		hor_size_same = TRUE;
	if(input_fetch_height==output_scan_act_height 
		&& input_fetch_width==output_scan_act_width
		&& (tv_mode==PAL || tv_mode ==NTSC)
		&& sdhd_DAC_bProgressive==FALSE)
	{
		bDublicateCoeff=TRUE;
	}
	else
	{
		bDublicateCoeff=FALSE;
	}
//#ifndef VPO_SUPPORT_DYNAMIC_FILTER_COEFFICIENT	
//	vp_hsr_reset_scale_coeff(pPicInfo);
//#else
//set the related configurations
	 pPicInfo->b_vertical_2tap  = 0;
	 pPicInfo->b_vertical_2tap = 0; 
	 pPicInfo->hor_size_same = hor_size_same;
	 pPicInfo->is_source_output = TRUE;
//this choice branch have been simplied             2009.3.25
	if(source_selected_width!=display_width || source_selected_height!= display_height)
		pPicInfo->is_source_output = FALSE;
	mini_vp_sub_sdhd_tvsys_vpout(g_sdhd_blogo_format,&VP_N_OutType);
	pPicInfo->eOutputMode = VP_N_OutType;
	pPicInfo->input_fetch_height = input_fetch_height;
	pPicInfo->input_fetch_width = input_fetch_width;
	pPicInfo->output_scan_act_width = output_scan_act_width;
	pPicInfo->output_scan_act_height = pPicInfo->output_scan_act_height;
	pPicInfo->bDublicateCoeff = bDublicateCoeff;
//now set the dit_enable
	if (progressive_frame ==TRUE)
		pPicInfo->dit_enable =FALSE;
	else if(sdhd_DAC_bProgressive==TRUE)
		pPicInfo ->dit_enable =TRUE;
	pPicInfo->bprogressive=(sdhd_DAC_bProgressive)?TRUE:FALSE;
	//mini_vp_hsr_reset_scale_coeff_dynamic(pPicInfo);
//vp_hsr_reset_scale_coeff(struct VP_Scale_Factor *pPicInfo)
// and y_h 8-tap y_v 4-tap c_v 4-tap 
//void hal_sdhd_scale_para(BOOL bduplicate, BOOL v_y_2tap, BOOL v_c_2tap)
	unsigned long	dwTmp = sdhd_vp_ReadDword(0x90)&0xFFFEFF3F;//clear bit6,7,16
	BOOL 	bduplicate = FALSE, v_y_2tap =FALSE, v_c_2tap =FALSE;
	if(bduplicate) dwTmp |= (0x01<<16);
	if(v_y_2tap) dwTmp |= (0x01<<7);
	if(v_c_2tap) dwTmp |= (0x01<<6);
	sdhd_vp_WriteDword(0x90, dwTmp);	
//hal_set_scale_coeff_register(8,32,0,pPicInfo->bDublicateCoeff);//y_h
	mini_hal_set_scale_coeff_register(8,32,0,pPicInfo->bDublicateCoeff);//y_h
	mini_hal_set_scale_coeff_register(4,32,1,pPicInfo->bDublicateCoeff);//y_v	
	mini_hal_set_scale_coeff_register(4,32,2,pPicInfo->bDublicateCoeff);//c_h
	mini_hal_set_scale_coeff_register(4,32,3,pPicInfo->bDublicateCoeff);//c_v
//end. set filter_coeff
	y_h_inc = (mini_SCALE_H_INT2LSB(input_fetch_width))/output_scan_act_width;
	y_v_inc = (mini_SCALE_V_INT2LSB(input_fetch_height))/output_scan_act_height;
	c_h_inc = (mini_SCALE_H_INT2LSB(input_fetch_width>>1))/output_scan_act_width;
	c_v_inc = (mini_SCALE_V_INT2LSB(input_fetch_height>>1))/output_scan_act_height;
	mini_hal_sdhd_scale(0x10, ((mini_SCALE_H_INC_LSB2MSB(y_h_inc))<<24)|((mini_SCALE_H_INC_LSB2LSB(y_h_inc))<<8)|(mini_SCALE_H_INC_LSB2INT(y_h_inc)));
	mini_hal_sdhd_scale(0x14, ((mini_SCALE_V_INC_LSB2MSB(y_v_inc))<<24)|((mini_SCALE_V_INC_LSB2LSB(y_v_inc))<<8)|(mini_SCALE_V_INC_LSB2INT(y_v_inc)));
	mini_hal_sdhd_scale(0x18, ((mini_SCALE_H_INC_LSB2MSB(c_h_inc))<<24)|((mini_SCALE_H_INC_LSB2LSB(c_h_inc))<<8)|(mini_SCALE_H_INC_LSB2INT(c_h_inc)));
	mini_hal_sdhd_scale(0x1c, ((mini_SCALE_V_INC_LSB2MSB(c_v_inc))<<24)|((mini_SCALE_V_INC_LSB2LSB(c_v_inc))<<8)|(mini_SCALE_V_INC_LSB2INT(c_v_inc)));

	h_offset_mb = source_selected_left>>4;
	v_offset_mb = source_selected_top>>4;
	y_h_init = mini_SCALE_H_INT2LSB(source_selected_left - (h_offset_mb<<4));
	c_h_init = y_h_init>>1;
	y_h_init -= mini_SCALE_H_INT2LSB(2);//y_h initial=-2,c_h initial=0;
	//c_h_init = 0;
	if(hor_size_same)
		mini_hal_sdhd_scale(0x00, 0x1f);
	else
		mini_hal_sdhd_scale(0x00, ((mini_SCALE_H_INIT_LSB2MSB(y_h_init))<<24)|((mini_SCALE_H_INIT_LSB2LSB(y_h_init))<<8)|(mini_SCALE_H_INIT_LSB2INT(y_h_init)));
	mini_hal_sdhd_scale(0x08, ((mini_SCALE_H_INIT_LSB2MSB(c_h_init))<<24)|((mini_SCALE_H_INIT_LSB2LSB(c_h_init))<<8)|(mini_SCALE_H_INIT_LSB2INT(c_h_init)));
	pPicInfo->y_v_o_normal = mini_SCALE_V_INT2LSB(source_selected_top - (v_offset_mb<<4));
	if(!pPicInfo->b_frame_fetch)
	{
		pPicInfo->y_v_o_normal/=2;
	}
	pPicInfo->c_v_o_normal = (pPicInfo->y_v_o_normal)>>1;
	pPicInfo->y_v_inc = y_v_inc;
	pPicInfo->c_v_inc = c_v_inc;
	mini_vp_sub_sdhd_scale_reset_init(pPicInfo,FALSE,FALSE);

	sdhd_vp_WriteDword(0x34,((display_left+display_width-1)&0x7ff)<<16);
	sdhd_vp_WriteDword(0x38,((display_left+display_width-1)&0x7ff)<<16);
	sdhd_vp_WriteDword(0x3C,0);	
	sdhd_vp_WriteDword(0x40,0);
	sdhd_vp_WriteDword(0x44,1);	
	sdhd_vp_WriteDword(0x48,1);	
	
	mini_hal_sdhd_mp_offset_insource(h_offset_mb, v_offset_mb);
	mini_hal_sdhd_mp_size_insource(source_selected_width+(source_selected_left - (h_offset_mb<<4)), source_selected_height+(source_selected_top- (v_offset_mb<<4)));
	display_top = display_top/2*2;
	mini_hal_sdhd_mp_rectinscreen(display_left,display_left+display_width-1,display_top,display_top+display_height-1);
}
void sdhd_vpo_vdec_link()
{
	//set mask
	sdhd_vp_WriteDword(0x90, sdhd_vp_ReadDword(0x90) | 0x1);
//vp_hal_MP_Addr
	sdhd_vp_WriteDword(0x60, 0);
	sdhd_vp_WriteDword(0x64, (unsigned long)dec_frm_buf_y_hd);
	sdhd_vp_WriteDword(0x68, (unsigned long)dec_frm_buf_y_hd);
	sdhd_vp_WriteDword(0x6c, (unsigned long)dec_frm_buf_c_hd);
	sdhd_vp_WriteDword(0x70, (unsigned long)dec_frm_buf_c_hd);
	unsigned char bBufStride = (hd_wOrigWidth+31)>>5/*4 original*/;  //for M3602, the Stride for MPEG-2 is 32, while for H.264 is 16
	sdhd_vp_WriteWord(0x7A,(bBufStride & 0x3F));     
	//clear mask
	sdhd_vp_WriteDword(0x90, sdhd_vp_ReadDword(0x90) & 0xfffffffe);
//	WriteZoomRegisters(); 
	mini_vp_sub_sdhd_scale_simple(hd_wOrigWidth,hd_wOrigHeight,g_sdhd_blogo_format);


}

void sdhd_VPOInitRegs()
{
	int i;
	unsigned long dwTmp;
//3 Init the HW of sdhd VP
//set mask
	sdhd_vp_WriteDword(0x90, sdhd_vp_ReadDword(0x90) | 0x1);
//init regs
//hal_sdhd_insert_zero(0) 
	dwTmp = sdhd_vp_ReadDword(0x90)&0xFFFFFFDF;//clear bit5
	//if(benable) dwTmp |= (0x01<<5);
	sdhd_vp_WriteDword(0x90, dwTmp);
//hal_sdhd_sd_hd_convert(FALSE, FALSE);
//0x90 bit14:15
	dwTmp = sdhd_vp_ReadDword(0x90)&0xFFFF3FFF;//clear bit14,15
	sdhd_vp_WriteDword(0x90, dwTmp);
//hal_sdhd_bypass_gataclock(FALSE);
//0x90 bit22
	dwTmp = sdhd_vp_ReadDword(0x90);
	dwTmp = (dwTmp & ~(0x01<<22));
	sdhd_vp_WriteDword(0x90, dwTmp);
//hal_sdhd_enhance_enable(False); close
//0x90 bit23
	dwTmp = sdhd_vp_ReadDword(0x90);
	dwTmp = (dwTmp | (0x01<<23));
	sdhd_vp_WriteDword(0x90, dwTmp);
//void hal_sdhd_dit_enable(BOOL False)
//0x90 bit4 dit enable
	dwTmp = sdhd_vp_ReadDword(0x90)&0xFFFFFFEF;//clear bit4
	sdhd_vp_WriteDword(0x90, dwTmp);	
//void hal_sdhd_vt_enable(BOOL False)
//0x90 bit27 25,26
	dwTmp = sdhd_vp_ReadDword(0x90);
	dwTmp = (dwTmp & (~(0x07<<25)));
	sdhd_vp_WriteDword(0x90, dwTmp);	
//void hal_sdhd_preframe_enable(BOOL False)
//0x90 bit24
	dwTmp = sdhd_vp_ReadDword(0x90);
	dwTmp =  (dwTmp & (~(0x01<<24)));
	sdhd_vp_WriteDword(0x90, dwTmp);
//void hal_sdhd_vt_new_en(BOOL False)
//0x5c vt high pass filter and noise reduction coeff
	dwTmp = sdhd_vp_ReadDword(0x5c)&0x7fffffff;//clear bit 31
	dwTmp =  (dwTmp & (~(0x01<<31)));
	sdhd_vp_WriteDword(0x5c, dwTmp);
//0x9c mp and bg k value,gma2 globle k value
//vp_sub_sdhd_initregister
//void hal_sdhd_fbc_cache_by_pass_en(BOOL False)
	dwTmp = sdhd_vp_ReadDword(0x9c);
	dwTmp =  (dwTmp & (~(0x01<<25)));
	sdhd_vp_WriteDword(0x9c, dwTmp);
//void hal_sdhd_fbc_en(BOOL False)
	dwTmp = sdhd_vp_ReadDword(0x9c);
	dwTmp =  (dwTmp & (~(0x01<<24)));
	sdhd_vp_WriteDword(0x9c, dwTmp);
//void hal_sdhd_mp_global_k_en(BOOL False)
	dwTmp = sdhd_vp_ReadDword(0x9c);
	dwTmp =  (dwTmp & (~(0x01<<17)));
	sdhd_vp_WriteDword(0x9c, dwTmp);

//void hal_sdhd_gma2_global_k_en(BOOL False)
	dwTmp = sdhd_vp_ReadDword(0x9c);
	dwTmp =  (dwTmp & (~(0x01<<16)));
	sdhd_vp_WriteDword(0x9c, dwTmp);
//void hal_sdhd_mp_global_k(UINT8 0)
	sdhd_vp_WriteDword(0x9c, (sdhd_vp_ReadDword(0x9c) & 0xFFFF00FF)|(0<<8));
//void hal_sdhd_gma2_global_k(UINT8 0)
	sdhd_vp_WriteDword(0x9c, (sdhd_vp_ReadDword(0x9c) & 0xFFFFFF00)|0);
//void hal_sdhd_edge_preserve_enable(BOOL False)
//0x90 bit28
	dwTmp = sdhd_vp_ReadDword(0x90);
	dwTmp =  (dwTmp & (~(0x01<<28)));
	sdhd_vp_WriteDword(0x90, dwTmp);
//init the enhance filter coefficient value ok 
	mini_sdhd_enhance_write(0,0x0000);
	mini_sdhd_enhance_write(1,0x0100);
	mini_sdhd_enhance_write(2,0x0000);
	mini_sdhd_enhance_write(3,0x0000);
	mini_sdhd_enhance_write(4,0x0000);
	mini_sdhd_enhance_write(5,0x0100);
	mini_sdhd_enhance_write(6,0x0000);
	mini_sdhd_enhance_write(7,0x0100);
	mini_sdhd_enhance_write(8,0x0000);
	mini_sdhd_enhance_write(9,0x0100);
//hal_sdhd_edge_preserve_threshold(0x0a,0x00,0x3c,0x14);
	unsigned char hps        = 0x14;
       unsigned char med_1   = 0x0a;
	unsigned char med_2   = 0x00;
	unsigned char min_diff = 0x3c;
       sdhd_vp_WriteDword(0x50, (hps<<24)|(min_diff<<16)|(med_2<<8)|med_1);
	mini_sdhd_adpative_compen_threshold_1(0x0a,0x0a, 0x0a, 0x0a, 0x64);
	mini_sdhd_adpative_compen_threshold_2(0x0a,0x14, 0x0a, 0x0a,0x14);
	mini_sdhd_vp_filt_noise_reduc_threshold(0x64, 0x0a, 0x04);
	sdhd_vp_WriteDword(0x74, 0|(0<<16));
//hal_sdhd_background(0x10,0x80,0x80)
	unsigned long dwControl=0x00108080;   
	sdhd_vp_WriteDword(0x88,dwControl);
	//hal_sdhd_enable_error_isr(0)  
	sdhd_vp_WriteDword(0x8c, (sdhd_vp_ReadDword(0x8c) & 0x0000FFFF)|(0<<16));
//win onoff
	dwTmp = sdhd_vp_ReadDword(0x90);
	dwTmp = dwTmp & ~(0x01<<21);
	sdhd_vp_WriteDword(0x90, dwTmp);  
	mini_sdhd_filter_group_num(32,32,32,32);
	mini_hal_sdhd_lat();	
//DMA fetch mode  FRAME 
	dwTmp = sdhd_vp_ReadDword(0x90);
	unsigned char bFetch=1;
//bFetch = (progressive_frame)?1:0;  
	sdhd_vp_WriteDword(0x90, (dwTmp&0xFFF9FFFF)|((bFetch & 0x03)<<17) ); 
//field based
	sdhd_vp_WriteDword(0x94, 0); 
//--calculate zoom coeff
//clear mask
	sdhd_vp_WriteDword(0x90, sdhd_vp_ReadDword(0x90) & 0xfffffffe);
#if 0
//	WriteZoomRegisters(); 
	mini_vp_sub_sdhd_scale_simple(wOrigWidth,wOrigHeight,g_sdhd_blogo_format);
#endif

//3Init the HW of sdhd DVI
	//set mask //0x00:2
	sdhd_vou_WriteDword(0x00, sdhd_vou_ReadDword(0x00)&(~0x04));
	//dvi_init
	unsigned char VOU_Scan_Mode = (sdhd_DAC_bProgressive)?DVI_SCAN_PROGRESSIVE:DVI_SCAN_INTERLACE;
	unsigned char ISR_format= (VOU_Scan_Mode==DVI_SCAN_PROGRESSIVE)?(VOU_3601_ISR_VBLNNK|VOU_3601_ISR_LM1):(VOU_3601_ISR_VBLNNK|VOU_3601_ISR_LM1|VOU_3601_ISR_LM2);
	unsigned short top_start_line = 33;
	unsigned short bottom_start_line = 346;
	mini_vou_hal_initreg(VP_N_OutType,VOU_Scan_Mode,ISR_format,top_start_line,bottom_start_line);
//	Start to playback
	mini_vou_hal_Playback(TRUE);
	unsigned long tmp_32 = sdhd_vou_ReadDword(0x00);
	tmp_32 = (tmp_32|0x01);
	sdhd_vou_WriteDword(0x00, tmp_32);
//vou_hal_ClearMask();
	tmp_32 = sdhd_vou_ReadDword(0x00);
	tmp_32 |= 0x04;
	sdhd_vou_WriteDword(0x00, tmp_32);
//3 Init the HW of sdhd TVE 
	mini_tve_s3601_sdhd_set_tvsys(g_sdhd_blogo_format,sdhd_DAC_bProgressive);
	mini_vp_sub_sdhd_tvsys_vpout(g_sdhd_blogo_format,&VP_N_OutType);
	mini_sdhd_output_mode(VP_N_OutType,sdhd_DAC_bProgressive);
//3Register ISR	
	tmp_32 = 0;
//not in test mode of TVE
	TVE_WRITED_SDHD(0xe0, tmp_32);
	
	mini_hal_dac_upsamplen(TRUE);
//void hal_input_sync_active_level(BOOL TURE,BOOL TRUE)
//0x80 bit6,7
	mini_hal_input_sync_active_level(TRUE,TRUE);
	mini_hal_update_register();
	mini_sub_update_register();
	mini_hal_component_rgb_sync_onoff(FALSE, FALSE, FALSE);	//close RGB sync tip
//show on sdhd screen
/*
	dwTmp = sdhd_vp_ReadDword(0x90);
	dwTmp = dwTmp | 0x01<<21;
	sdhd_vp_WriteDword(0x90, dwTmp);
*/
}
#endif
