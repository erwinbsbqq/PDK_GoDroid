/*
Description: decode and show one picture when system bootup.
		*Simply parse sequence header, get picture size and intra/non-intra quantiser matrix
		*Adaptive for 720*576, 720*480 source pictures coded by MPEG2
		*Adaptive for PAL, PAL_N, NTSC_358, NTSC_443, PAL_M, PAL_60 tv format. Config one format by defination in sys_config.h
History:
2005-05-11		Xionghua Yu		Create
2005-05-17		Xionghua Yu		Modify for varied tv format
2005-06-21		Xionghua Yu		Modify code for different memory mapping:2,4,8 mega, these are from sys_config.h
2005-07-07		Xionghua Yu		Solved a bug about search header 0x000001
2009-04-08          Costa    Wen          modify for M3602 to show logo;
*/
#include <sys_define.h>
#include "../include/sys_parameters.h"
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
#define COMPLEMENT(x)  ((~x)+1)
#define COMPLEMENT_BYTE(x) ((256- x))

//vpo hal operation
#define   sdhd_VP_REG_BASEADDR		0xB8006100
#define	VP_REG_BASEADDR		0xB8020100
#define	DVI_REG_BASEADDR	0xB8020000
#define vp_WriteWord(dwOffset,dwRegisterValue)		SYS_WriteWord(VP_REG_BASEADDR+dwOffset, dwRegisterValue)
#define vp_ReadWord(dwOffset)				SYS_ReadWord(VP_REG_BASEADDR+dwOffset)
#define vp_WriteDword(dwOffset,dwRegisterValue)		SYS_WriteDWord(VP_REG_BASEADDR+dwOffset, dwRegisterValue)
#define vp_ReadDword(dwOffset)				SYS_ReadDWord(VP_REG_BASEADDR+dwOffset)
#define dvi_WriteDword(dwOffset,dwRegisterValue)	SYS_WriteDWord(DVI_REG_BASEADDR+dwOffset, dwRegisterValue)
#define dvi_ReadDword(dwOffset)				SYS_ReadDWord(DVI_REG_BASEADDR+dwOffset)
//tve hal operation
#define TVE_BASE    0xb8008000
#define TVEWriteB(index, data) SYS_WriteByte(TVE_BASE+index, data)
#define TVEWriteW(index, data) SYS_WriteWord(TVE_BASE+index, data)
#define TVEWriteD(index, data) SYS_WriteDWord(TVE_BASE+index, data)
#define TVEReadB(index) ((unsigned char)(SYS_ReadByte(TVE_BASE+index)))
#define TVEReadW(index) ((unsigned short)(SYS_ReadWord(TVE_BASE+index)))
#define TVEReadD(index) ((unsigned long )(SYS_ReadDWord(TVE_BASE+index)))
//vdec hal operation
#define	VDEC27_BASEADDR	0xb8004200
#define	vdec_readuint32(offset)			SYS_ReadDWord(VDEC27_BASEADDR+offset)
#define	vdec_writeuint32(offset,value)	SYS_WriteDWord(VDEC27_BASEADDR+offset,value)
#define	vdec_readuint8(offset)			SYS_ReadByte(VDEC27_BASEADDR+offset)
#define	vdec_writeuint8(offset,value)		SYS_WriteByte(VDEC27_BASEADDR+offset,value)
//hal macro-definition
#define	VDEC27_ISR_FINISH		0x01
#define	VDEC27_ISR_ERROR		0x02
#define	VDEC27_ISR_DATAREQ	0x04
#define	VDEC27_DATA_VALID		0x01
#define	VDEC27_DATA_LAST		0x02
#define	VDEC27_DATA_FIRST		0x04
#define	VDEC_REG_MPHR				0x00
#define	VDEC_REG_FSIZE			0x08
#define	VDEC_REG_MBADDR			0x0c
#define	VDEC_REG_VECTRL			0x10
#define	VDEC_REG_VETRIGGER		0x14
#define	VDEC_REG_VESTAT			0x18
#define	VDEC_REG_CRTMBADDR		0x20
#define	VDEC_REG_VLDBADDR		0x24
#define	VDEC_REG_VLDOFFSET		0x28
#define	VDEC_REG_VLDLEN			0x2c
#define	VDEC_REG_VLDENDADDR		0x30
#define	VDEC_REG_FRM0ADDR		0x40
#define	VDEC_REG_FRM1ADDR		0x44
#define	VDEC_REG_FRM2ADDR		0x48
#define	VDEC_REG_FRM3ADDR		0x4c
#define	VDEC_REG_FRM4ADDR		0x50
#define	VDEC_REG_FRM5ADDR		0x54
#define	VDEC_REG_MAFADDR			0x58
#define	VDEC_REG_MAFADDRSEL		0x5c
#define	VDEC_REG_DVIEWYADDR		0x70
#define	VDEC_REG_DVIEWCADDR		0x74
#define	VDEC_REG_BASEADDR		0x78
#define	VDEC_REG_PICSIZE			0x7c
#define	VDEC_REG_IQMINPUT			0x80
#define	VDEC_REG_QCINPUT			0x88
#define	VDEC_REG_IQIDCTINPUT		0xb0
#define	VDEC_REG_ORDERCTRL		0xc0
#define	VDEC_REG_ERRFLAG			0xc4
#define	VDEC_REG_DFF0				0xc8
#define	VDEC_REG_DFF1				0xcc
#define	VDEC_REG_DFF2				0xd0
#define	VDEC_REG_DFF3				0xd4
#define	VDEC_REG_DVIEWCTRL		0xd8
#define VDEC_REG_EXTRAFUNCFG		0xe0
#define VDEC_REG_EXTRAFUNCONTENT		0xe4
#define	VDEC_REG_DVIEWINBG		0xf0
#define	VDEC_REG_MAFCTRL0		0xf4
#define	VDEC_REG_MAFCTRL1		0xf8
#define	VDEC_REG_MAFCTRL2		0xfc


#define	VE_SEQUENCE_HEADER_CODE		0x000001b3
#define	VE_GROUP_START_CODE			0x000001b8
#define	VE_PICTURE_START_CODE		0x00000100
#define	VE_EXTENSION_START_CODE		0x000001b5
#define	VE_USER_DATA_START_CODE		0x000001b2
#define	VE_SEQ_EXT_ID					1
#define	VE_PIC_EXT_ID					8
#define	VE_SEQ_DISPLAY_EXT_ID			2
#define	VE_SEQ_SCALABLE_EXT_ID		5
#define	VE_PIC_QUANT_MATRIX_EXT_ID	3
#define	VE_PIC_DISPLAY_EXT_ID			7
#define	VE_PIC_SPATIAL_SCALABLE_EXT_ID	9
#define	VE_PIC_TEMPORAL_SCALABLE_EXT_ID	10
static unsigned char iqm_1_array[64]={
	0x08,0x10,0x10,0x13,0x10,0x13,0x16,0x16,0x16,0x16,0x16,0x16,0x1a,0x18,0x1a,0x1b,
	0x1b,0x1b,0x1a,0x1a,0x1a,0x1a,0x1b,0x1b,0x1b,0x1d,0x1d,0x1d,0x22,0x22,0x22,0x1d,
	0x1d,0x1d,0x1b,0x1b,0x1d,0x1d,0x20,0x20,0x22,0x22,0x25,0x26,0x25,0x23,0x23,0x22,
	0x23,0x26,0x26,0x28,0x28,0x28,0x30,0x30,0x2e,0x2e,0x38,0x38,0x3a,0x45,0x45,0x53
};
static unsigned char iqm_0_array[64]={
	0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
	0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
	0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
	0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10
};

typedef int HOR_FILTER_COFF_ARRAY[16][4];
static HOR_FILTER_COFF_ARRAY g_HorFilterCoffArray=
{
	{0,0,256,0},
	{0,10,254,-7},
	{-2,23,247,-12},
	{-4,39,236,-16},
	{-6,58,222,-18},
	{-9,78,205,-19},
	{-11,100,186,-19},
	{-14,122,166,-18},
	{-16,144,144,-16},
	{-18,166,122,-14},
	{-19,186,100,-11},
	{-19,205,78,-9},
	{-18,222,58,-6},
	{-16,236,39,-4},
	{-12,247,23,-2},
	{-7,254,10,0}
};

struct RegisterStruct
{
	unsigned long dwRegisterIndex;
	unsigned long dwRegisterValue;
};
struct
{
	struct RegisterStruct RegisterArrayForZoom[80];//R_L 8*80=640 bytes
	unsigned char bRegisterArrayIndexForZoom;
	unsigned char bRegisterArrayStatusForZoom;// 0 : no command; 1 : command not ready; 2 : command ready
}
g_RegisterArrayForZoomStruct;
struct SequenceHeader{
	unsigned long horizontal_size :12;
	unsigned long vertical_size :12;
	unsigned long aspect_ratio_information :4;
	unsigned long frame_rate_code :4;
	unsigned long bit_rate_value :18;
	unsigned long maker_bit :1;
	unsigned long vbv_buffer_size :10;
	unsigned long constrained_parameters_flag :1;
	unsigned long load_intra_quantizer_matrix :1;
	unsigned char *intra_quantizer_matrix ;
	unsigned long load_non_intra_quantizer_matrix :1;
	unsigned char *non_intra_quantizer_matrix;
};
static struct SequenceHeader sqh;
struct HeaderInfo
{
	unsigned long full_pel_backward_vector	:1;
	unsigned long full_pel_forward_vector	:1;
	unsigned long alternate_scan		:1;
	unsigned long intra_vlc_format	:1;
	unsigned long q_scale_type	:1;
	unsigned long concealment_motion_vectors	:1;
	unsigned long frame_pred_frame_det	:1;
	unsigned long top_field_first		:1;
	unsigned long picture_structure		:2;
	unsigned long intra_dc_precision	:2;
	unsigned long f_code_11			:4;
	unsigned long f_code_10			:4;
	unsigned long f_code_01			:4;
	unsigned long f_code_00			:4;
	unsigned long picture_coding_type		:3;
	unsigned long reserve_0			:1;
};
static unsigned char is_mpeg1;
static unsigned long head_info;
/*static*/ unsigned char progressive_frame;
unsigned int sd_wOrigHeight;
unsigned int sd_wOrigWidth;
unsigned int hd_wOrigHeight;
unsigned int hd_wOrigWidth;
#define FRM_Y_SIZE_SD 			0x67800// the is equal to (720+16)x576  for MPEG2 mapping in S3602
#define FRM_C_SIZE_SD			0x33C00// 0x32c00
//for 1920*1088 test
#define FRM_Y_SIZE_HD                 1920*1088
#define FRM_C_SIZE_HD                 1920*1088/2

#define EXTENSION_START_CODE	0x000001B5
#define SEQUENCE_START_CODE		0x000001B3
#define GROUP_START_CODE		0x000001B8
#define PICTURE_START_CODE		0x00000100
#define SEQUENCE_END_CODE		0x000001B7
#define SEQUENCE_ERROR_CODE		0x000001B4
#define USER_DATA_START_CODE	0x000001B2
#define RESERVED -1
#define SEQUENCE_EXTENSION_ID					1
#define SEQUENCE_DISPLAY_EXTENSION_ID			2
#define QUANT_MATRIX_EXTENSION_ID				3
#define COPYRIGHT_EXTENSION_ID					4
#define SEQUENCE_SCALABLE_EXTENSION_ID			5
#define PICTURE_DISPLAY_EXTENSION_ID			7
#define PICTURE_CODING_EXTENSION_ID				8
#define PICTURE_SPATIAL_SCALABLE_EXTENSION_ID	9
#define PICTURE_TEMPORAL_SCALABLE_EXTENSION_ID	10
static unsigned char *cur_ptr;

//----------------define the variable for dac and logo setting --------------------
extern unsigned short g_blogo_format;//bootloader logo parameters
static unsigned long chip_module = 0;
static unsigned char *dec_mem_top = (unsigned char *)0xA0800000;
static unsigned long dec_mem_size = 0x98000;
unsigned char *dec_frm_buf_y_hd = 0;
unsigned char *dec_frm_buf_c_hd = 0;
unsigned char *dec_frm_buf_y_sd = 0;
unsigned char *dec_frm_buf_c_sd = 0;

void decGetNextStartCode()
{
	unsigned long code=0;
	code = *cur_ptr++;
	code<<=8;
	code+=*cur_ptr++;
	code<<=8;
	code+=*cur_ptr++;
	while((code&0xffffff)!=0x000001)
	{
		code<<=8;
		code+=*cur_ptr++;
	}
	cur_ptr-=3;
}
void decSequenceHeader()
{
	int i;
	unsigned long tmp;
	tmp=*cur_ptr++;	tmp<<=4;tmp+=((*cur_ptr)>>4);
	sqh.horizontal_size = tmp;
	tmp=((*cur_ptr++)&0xf)<<8;	tmp+=(*cur_ptr++);
	sqh.vertical_size = tmp;
	tmp=(*cur_ptr)>>4;
	sqh.aspect_ratio_information    = tmp;
	tmp=((*cur_ptr++)&0xf);
	sqh.frame_rate_code = tmp;
	tmp=((*cur_ptr++)<<10);tmp+=((*cur_ptr++)<<2);tmp+=((*cur_ptr)>>6);
	sqh.bit_rate_value =tmp;
	//markbit :1
	tmp=((*cur_ptr)>>5)&1;
	sqh.maker_bit=tmp;
	tmp= ((*cur_ptr++)&0x1f)<<5;tmp+=(*cur_ptr)>>3;
	sqh.vbv_buffer_size = tmp;
	tmp=((*cur_ptr)>>2)&1;
	sqh.constrained_parameters_flag = tmp;
	tmp=((*cur_ptr)>>1)&1;
	sqh.load_intra_quantizer_matrix =tmp;
	if(sqh.load_intra_quantizer_matrix)
	{
		for (i=0; i<64; i++)
			{
				tmp=((*cur_ptr)&1)<<7;tmp+=(*cur_ptr++)>>1;
				sqh.intra_quantizer_matrix[i] = tmp;
			}
	}
	sqh.load_non_intra_quantizer_matrix =(*cur_ptr++)&1;
	if(sqh.load_non_intra_quantizer_matrix)
	{
		for (i=0; i<64; i++)
			sqh.non_intra_quantizer_matrix[i] = (*cur_ptr++);
	}
	else
	{
		for (i=0; i<64; i++)
			sqh.non_intra_quantizer_matrix[i] = 16;
	}
	//skip extension data
	//extension_and_user_data();
}
void decGroupHeader()
{
	//skip
}
void decPictureHeader()
{
	//skip

}
void decPictureCodingExt()
{
	struct HeaderInfo head;
	unsigned long tmp;
	tmp =(*cur_ptr++)<<24;
	tmp +=(*cur_ptr++)<<16;
	tmp +=(*cur_ptr++)<<8;
	tmp +=(*cur_ptr++);

	head.f_code_00 = (tmp>>24)&0x0F;//bit4
	head.f_code_01 = (tmp>>20)&0x0F;//bit4	
	head.f_code_10 = (tmp>>16)&0x0F;//bit4
	head.f_code_11 = (tmp>>12)&0x0F;//bit4
	head.intra_dc_precision = (tmp>>10)&0x03;//bit2
	head.picture_structure	= (tmp>>8)&0x03;	//bit2 //bit8,9
	head.top_field_first = (tmp>>7)&0x01;//bit1
	head.frame_pred_frame_det = (tmp>>6)&0x01;//bit1
	head.concealment_motion_vectors = (tmp>>5)&0x01;//bit1
	head.q_scale_type = (tmp>>4)&0x01;//bit1
	head.intra_vlc_format = (tmp>>3)&0x01;//bit1
	head.alternate_scan = (tmp>>2)&0x01;//bit1
	head.picture_coding_type = 1;
	head.full_pel_backward_vector = 0;
	head.full_pel_forward_vector = 0;

	progressive_frame = ((*cur_ptr++)>>7)&0x01;//bit1	
	head_info = *((unsigned long *)(&head));
}
void default_for_MPEG1()
{
	struct HeaderInfo head;
		
	head.f_code_00 = 7;//(tmp>>24)&0x0F;//bit4
	head.f_code_01 = 7;//(tmp>>20)&0x0F;//bit4	
	head.f_code_10 = 7;//(tmp>>16)&0x0F;//bit4
	head.f_code_11 = 7;//(tmp>>12)&0x0F;//bit4
	
	head.intra_dc_precision = 0;//bit2
	head.picture_structure	= 3;	//bit2 //bit8,9
	head.top_field_first = 1;//bit1
	head.frame_pred_frame_det = 1;//bit1
	head.concealment_motion_vectors = 0;//bit1
	head.q_scale_type = 0;//bit1
	head.intra_vlc_format = 0;//bit1
	head.alternate_scan = 0;//bit1
	head.picture_coding_type = 1;

	progressive_frame = 1;//bit1	
	head_info = *((unsigned long *)(&head));	
}

int decParseHeader(unsigned char *pb)
{
	unsigned long code;
	unsigned char *org_ptr = cur_ptr;
	sqh.intra_quantizer_matrix=iqm_1_array;
	sqh.non_intra_quantizer_matrix=iqm_0_array;
	is_mpeg1 = 1;
	
	for (;;)
	{
		decGetNextStartCode();
		code = *cur_ptr++;
		code<<=8;
		code+=*cur_ptr++;
		code<<=8;
		code+=*cur_ptr++;
		code<<=8;
		code+=*cur_ptr++;
		switch (code)
		{
		case SEQUENCE_START_CODE:
			decSequenceHeader();
		break;
		case GROUP_START_CODE:
			decGroupHeader();
		break;
		case PICTURE_START_CODE:
			decPictureHeader();
		break;
		case EXTENSION_START_CODE:
			if(((*cur_ptr)>>4) == VE_PIC_EXT_ID)
			{
				is_mpeg1 = 0;
				decPictureCodingExt();
				//return 1;
			}
		break;			
		case SEQUENCE_END_CODE:
			return 0xffffffff;
		break;
		case 0x00000101:		
			if(is_mpeg1)
					default_for_MPEG1();
			return (cur_ptr - org_ptr - 4);
		default:
		break;
		}
	}
}

int  decFindSliceHeader(unsigned char *pb)
{
	unsigned char * pb_ori=pb;
	unsigned long seg=0;
	seg=*pb++;
	seg<<=8;
	seg+=*pb++;
	seg<<=8;
	seg+=*pb++;
	seg<<=8;
	seg+=*pb++;
	while(seg!=0x00000101)
	{
		seg<<=8;
		seg+=*pb++;
	}
	return (pb-pb_ori-4);
}
void mini_hal_dview_onoff(int bon)
{
 unsigned long utmp,utmp1;
 utmp = 0;
 utmp |= 0x0<<31; //config rw flag--r
 utmp |= 0x1<<8;  //function index--dview
 utmp |= 0x6;   //function sub index--dview control
 vdec_writeuint32(VDEC_REG_EXTRAFUNCFG,utmp);
 utmp1 = vdec_readuint32(VDEC_REG_EXTRAFUNCONTENT);

 utmp = 0;
 utmp |= 0x1<<31; //config rw flag--w
 utmp |= 0x1<<8;  //function index--dview
 utmp |= 0x6;   //function sub index--dview control
 vdec_writeuint32(VDEC_REG_EXTRAFUNCFG,utmp);
 if(bon)
 {
  vdec_writeuint32(VDEC_REG_EXTRAFUNCONTENT, utmp1|0x40);
  vdec_writeuint32(VDEC_REG_VECTRL, vdec_readuint32(VDEC_REG_VECTRL)|0x400);
 }
 else
 {
  vdec_writeuint32(VDEC_REG_EXTRAFUNCONTENT, utmp1 & 0xFFFFFFbF);
  if((utmp1&0x80)==0)
   vdec_writeuint32(VDEC_REG_VECTRL, vdec_readuint32(VDEC_REG_VECTRL) & 0xFFFFFBFF);
 }
  
 //if(bon)
 // vdec_writeuint32(VDEC_REG_VECTRL, vdec_readuint32(VDEC_REG_VECTRL)|0x400);
 //else
 // vdec_writeuint32(VDEC_REG_VECTRL, vdec_readuint32(VDEC_REG_VECTRL) & 0xFFFFFBFF);
}
//0xd8
void mini_hal_dview_ctrl(unsigned char uscaledown_bot, unsigned char ucompress_mode, unsigned char bprogressive,unsigned char h_precision,unsigned char v_precision)
{
 unsigned long utmp,utmp1;
 utmp = 0;
 utmp |= 0x0<<31; //config rw flag--r
 utmp |= 0x1<<8;  //function index--dview
 utmp |= 0x6;   //function sub index--dview control
 vdec_writeuint32(VDEC_REG_EXTRAFUNCFG,utmp);
 utmp1 = vdec_readuint32(VDEC_REG_EXTRAFUNCONTENT);

 utmp = 0;
 utmp |= 0x1<<31; //config rw flag--w
 utmp |= 0x1<<8;  //function index--dview
 utmp |= 0x6;   //function sub index--dview control
 vdec_writeuint32(VDEC_REG_EXTRAFUNCFG,utmp);
 utmp = utmp1 & 0xFFFFFFE0;
 if(!bprogressive) utmp |= 0x10;

 if(v_precision==1) utmp |= (0x00<<2);
 else if(v_precision==2) utmp |= (0x01<<2);
 else if(v_precision==4) utmp |= (0x02<<2);
 else if(v_precision==8) utmp |= (0x03<<2);

 if(h_precision==1) utmp |= 0x00;
 else if(h_precision==2) utmp |= 0x01;
 else if(h_precision==4) utmp |= 0x02;
 else if(h_precision==8) utmp |= 0x03; 
 vdec_writeuint32(VDEC_REG_EXTRAFUNCONTENT, utmp);
}
void mini_hal_horflt_onoff(unsigned char bon)
{
 unsigned long utmp,utmp1;
 utmp = 0;
 utmp |= 0x0<<31; //config rw flag--r
 utmp |= 0x1<<8;  //function index--dview
 utmp |= 0x6;   //function sub index--dview control
 vdec_writeuint32(VDEC_REG_EXTRAFUNCFG,utmp);
 utmp1 = vdec_readuint32(VDEC_REG_EXTRAFUNCONTENT);

 utmp = 0;
 utmp |= 0x1<<31; //config rw flag--w
 utmp |= 0x1<<8;  //function index--dview
 utmp |= 0x6;   //function sub index--dview control
 vdec_writeuint32(VDEC_REG_EXTRAFUNCFG,utmp);
 if(bon)
 {
  vdec_writeuint32(VDEC_REG_EXTRAFUNCONTENT, utmp1|0xa0);
  vdec_writeuint32(VDEC_REG_VECTRL, vdec_readuint32(VDEC_REG_VECTRL)|0x400);
 }
 else
 {
  vdec_writeuint32(VDEC_REG_EXTRAFUNCONTENT, utmp1 & 0xFFFFFF5F);
  if((utmp1&0x40)==0)
   vdec_writeuint32(VDEC_REG_VECTRL, vdec_readuint32(VDEC_REG_VECTRL) & 0xFFFFFBFF);
 }
}

unsigned char IsHDLogo = 0;
unsigned char v_precision,h_precision;
void decInitRegs()
{
	unsigned long uc_addr,uy_addr,umem_base,ustart,uend,utmp;
	//switch  the ve to  MP2 for logo decoding
	*(unsigned long *)0xb8000078 = (*(unsigned long *)0xb8000078)&(~0x00002000);
	//framebuffer address
	uy_addr=(unsigned long)dec_frm_buf_y_hd;
	uc_addr=(unsigned long)dec_frm_buf_c_hd;
	vdec_writeuint32(VDEC_REG_FRM0ADDR, (((uc_addr>>9)&0xFFFF)<<16)|((uy_addr>>9)&0xFFFF));
	uy_addr=(unsigned long)dec_frm_buf_y_hd;
	uc_addr=(unsigned long)dec_frm_buf_c_hd;
	vdec_writeuint32(VDEC_REG_FRM0ADDR+4, (((uc_addr>>9)&0xFFFF)<<16)|((uy_addr>>9)&0xFFFF));
	uy_addr=(unsigned long)dec_frm_buf_y_hd;
	uc_addr=(unsigned long)dec_frm_buf_c_hd;
	vdec_writeuint32(VDEC_REG_FRM0ADDR+8, (((uc_addr>>9)&0xFFFF)<<16)|((uy_addr>>9)&0xFFFF));
	//memory base address
	umem_base=((((unsigned long)dec_frm_buf_y_hd&0x0FF00000)>>20)/32*32)<<20;
	vdec_writeuint8(VDEC_REG_BASEADDR, (umem_base>>25)&0x7F); 
	//vbv buffer start/end address
/*	ustart=__MM_VBV_START_ADDR;
	uend=__MM_VBV_START_ADDR + ((__MM_VBV_LEN-4)&0xFFFFFF00) - 1;
	vdec_writeuint32(VDEC_REG_VLDBADDR, (vdec_readuint32(VDEC_REG_VLDBADDR)&0xFF000000)|(ustart&0x00FFFFFF));
	vdec_writeuint32(VDEC_REG_VLDENDADDR, (vdec_readuint32(VDEC_REG_VLDENDADDR)&0xFF000000)|(uend&0x00FFFFFF));
*/
	//decode mode
	vdec_writeuint32(VDEC_REG_VECTRL, vdec_readuint32(VDEC_REG_VECTRL) & 0xDFFFFFFF);
	//iqis mode
	unsigned long dwtmp = vdec_readuint32(VDEC_REG_VECTRL);
	dwtmp = (dwtmp&0xF7FFFFFF);
	dwtmp = (dwtmp&0xFFFDFFFF);
	vdec_writeuint32(VDEC_REG_VECTRL, dwtmp);                                                                     
	// hal_write_rec_pic
	dwtmp = vdec_readuint32(VDEC_REG_VECTRL);
	dwtmp = (dwtmp&(~(0x01<<14)));
	vdec_writeuint32(VDEC_REG_VECTRL, dwtmp);
	//pulldown mode --false
	vdec_writeuint32(VDEC_REG_VECTRL, vdec_readuint32(VDEC_REG_VECTRL) & 0xFFFFF7FF);  
	//adpcm switch
	//vdec_writeuint32(VDEC_REG_VECTRL, vdec_readuint32(VDEC_REG_VECTRL) & 0xFFFFFDFF);//this function is not used @3602

	//disable all interrupt
	vdec_writeuint32(VDEC_REG_VECTRL, (vdec_readuint32(VDEC_REG_VECTRL)&0xFFFFFFC0)); 
	//chorma mode
	vdec_writeuint32(VDEC_REG_VETRIGGER, (vdec_readuint32(VDEC_REG_VETRIGGER)&0xE7FFFFF0));  
	//picture mode
	unsigned int udecode_format;
	if(is_mpeg1)
		udecode_format=1;
	else 
		udecode_format=2;	
	vdec_writeuint32(VDEC_REG_VETRIGGER, (vdec_readuint32(VDEC_REG_VETRIGGER)&0xF8FFFFF0)|((udecode_format&0x07)<<24)); 
#if 0
       //as showing a logo picture , MAF function is non-necessary
	//maf
	unsigned long umaf_addr=(unsigned long)dec_frm_buf_c;
	//
	utmp = 0;
 	utmp |= 0x1<<31; //config rw flag--w
 	utmp |= 0x2<<8;  //function index--laf
 	utmp |= 0x5;   //function sub index--laf flag buf addr
 	vdec_writeuint32(VDEC_REG_EXTRAFUNCFG,utmp);
 	utmp = 0;
 	utmp = umaf_addr;// + (vdec_readuint32(VDEC_REG_BASEADDR)<<25);
 	vdec_writeuint32(VDEC_REG_EXTRAFUNCONTENT,utmp);
	//vdec_writeuint32(VDEC_REG_MAFADDR, ((umaf_addr>>10)&0xFFF)<<16);
#endif
     //the following is for configurating VE scale down function
     if (IsHDLogo)
     {
     	//void vdec_hal_dview_addr(UINT32 uy_addr,UINT32 uc_addr)
     	{
     	//UINT32 utmp;
 	dwtmp = 0;
 	dwtmp |= 0x1<<31; //config rw flag--w
 	dwtmp |= 0x1<<8;  //function index--dview
 	dwtmp |= 0x0;   //function sub index--dview buf addr Y
 	vdec_writeuint32(VDEC_REG_EXTRAFUNCFG,dwtmp);
 	dwtmp = 0;
 	dwtmp = dec_frm_buf_y_sd;//+ (vdec_readuint32(VDEC_REG_BASEADDR)<<25);
 	vdec_writeuint32(VDEC_REG_EXTRAFUNCONTENT,dwtmp);
 	dwtmp = 0;
 	dwtmp |= 0x1<<31; //config rw flag--w
 	dwtmp |= 0x1<<8;  //function index--dview
 	dwtmp |= 0x1;   //function sub index--dview buf addr C
 	vdec_writeuint32(VDEC_REG_EXTRAFUNCFG,dwtmp);
 	dwtmp = 0;
 	dwtmp = dec_frm_buf_c_sd;// + (vdec_readuint32(VDEC_REG_BASEADDR)<<25);
 	vdec_writeuint32(VDEC_REG_EXTRAFUNCONTENT,dwtmp);
     	}
	unsigned char IsProgres = 0;
	if (progressive_frame)
		IsProgres = 1;
	//v_precision = h_precision = 2;
	mini_hal_dview_ctrl(0, 0, IsProgres,h_precision,v_precision);
	mini_hal_dview_onoff(1);
	mini_hal_horflt_onoff(0);
     }
     else
     {
     	mini_hal_dview_onoff(0);
     }
}

#define WriteRegisterArrayForZoom(dwIndex,dwRegisterValue) \
	vp_WriteDword(dwIndex, dwRegisterValue)
#define RegisterArraySetMaskForZoom() \
	vp_WriteDword(0x90, (vp_ReadDword(0x90)&0xfffffffe) | 1)
#define RegisterArrayResetMaskForZoom() \
	vp_WriteDword(0x90, (vp_ReadDword(0x90)&0xfffffffe) | 0)
#define WriteZoomRegisters() 

void mini_vp_sub_zoom(int iWidth, int iHeight, unsigned char tv_mode)
{
	int i;
	
	unsigned short wScreenHeight=0, wScreenWidth=0;
	
	unsigned short x_offset_in,y_offset_in,x_offset_in2,y_offset_in2;
	unsigned short hsize_in,vsize_in,hsize_in2,vsize_in2;
	
	unsigned short x_offset_out,y_offset_out;
	unsigned short hsize_out,vsize_out;	

	unsigned short dst;
	unsigned char bCoffGroupNum;

	unsigned char H_Init_MB,V_Init_MB;

	unsigned char Y_H_Init_Int;
	unsigned char Y_H_Init_FraMsb;
	unsigned short Y_H_Init_FraLsb;

	unsigned char C_H_Init_Int;
	unsigned char C_H_Init_FraMsb;
	unsigned char C_H_Init_FraLsb;	
	
	unsigned char Y_V_O_Init_Int;
	unsigned char Y_V_O_Init_FraMsb;
	unsigned char Y_V_O_Init_FraLsb;

	unsigned char Y_V_E_Init_Int;
	unsigned char Y_V_E_Init_FraMsb;
	unsigned char Y_V_E_Init_FraLsb;

	unsigned char C_V_O_Init_Int;
	unsigned char C_V_O_Init_FraMsb;
	unsigned char C_V_O_Init_FraLsb;
	
	unsigned char C_V_E_Init_Int;
	unsigned char C_V_E_Init_FraMsb;
	unsigned char C_V_E_Init_FraLsb;	

	unsigned char Y_H_Inc_Int;
	unsigned char Y_H_Inc_FraMsb;
	unsigned short Y_H_Inc_FraLsb;//12bit
	
	unsigned char C_H_Inc_Int;
	unsigned char C_H_Inc_FraMsb;
	unsigned char C_H_Inc_FraLsb;

	unsigned char Y_V_Inc_Int;
	unsigned char Y_V_Inc_FraMsb;
	unsigned char Y_V_Inc_FraLsb;

	unsigned char C_V_Inc_Int;
	unsigned char C_V_Inc_FraMsb;
	unsigned char C_V_Inc_FraLsb;

	unsigned short MpStartX;
	unsigned short MpEndX;
	unsigned short MpStartY;
	unsigned short MpEndY;

	unsigned char h_mb_offset,v_mb_offset;
	unsigned long h_pixel_offset , v_pixel_offset;

	unsigned long Base_MB_Offset_RegValue;
	unsigned long Y_H_Init_RegValue;
	unsigned long C_H_Init_RegValue;
	unsigned long Y_V_Init_RegValue;
	unsigned long C_V_Init_RegValue;
	unsigned long Y_H_Inc_RegValue;
	unsigned long C_H_Inc_RegValue;
	unsigned long Y_V_Inc_RegValue;
	unsigned long C_V_Inc_RegValue;
	
	unsigned long Coff_GN_RegValue;
	unsigned long Coff_Index_RegValue=0;
	unsigned long Coff_Msb_RegValue=0;
	unsigned long Coff_Lsb_RegValue=0;	

	unsigned long Mp_X_RegValue;	
	unsigned long Mp_Y_RegValue;	

	unsigned long Y_H_Normal_Pos_RegValue;
	unsigned long C_H_Normal_Pos_RegValue;
	unsigned long Y_H_Inc_Acc_RegValue;
	unsigned long C_H_Inc_Acc_RegValue;	
	unsigned long Y_H_Inc_Bgn_RegValue;
	unsigned long C_H_Inc_Bgn_RegValue;		





	switch(tv_mode)
	{
	case 0://PAL
		wScreenHeight = 576;
		wScreenWidth = 720;
		break;
	case 1://NTSC
		wScreenHeight = 480;
		wScreenWidth = 720;
		break;
	}

//1 1 calc size
	x_offset_in = 0;
	y_offset_in = 0;
	
	hsize_in = iWidth;
	vsize_in = iHeight;

	x_offset_out = 0;
	y_offset_out = 0;

	hsize_out = wScreenWidth;
	vsize_out = wScreenHeight;	


	
//1 2 2pixels align


	x_offset_in = (x_offset_in%2==0)?x_offset_in:x_offset_in-1;
	y_offset_in = (y_offset_in%2==0)?y_offset_in:y_offset_in-1;	

	hsize_in = (hsize_in%2==0)?hsize_in:hsize_in-1;
	vsize_in = (vsize_in%2==0)?vsize_in:vsize_in-1;

	x_offset_out = (x_offset_out%2==0)?x_offset_out:x_offset_out-1;
	y_offset_out = (y_offset_out%2==0)?y_offset_out:y_offset_out-1;	

	hsize_out = (hsize_out%2==0)?hsize_out:hsize_out-1;
	vsize_out = (vsize_out%2==0)?vsize_out:vsize_out-1;

	hsize_in2 = hsize_in/2;
	vsize_in2 = vsize_in/2;


//1 3 calc incre value
//H part

	/*sting bCoffGroupNum to 16 , STeve Lee*/
	/*
	dst =  hsize_out/CalculateGreatestCommonDivisor(hsize_in,hsize_out);
	bCoffGroupNum = (dst <= MAX_HOR_PHASE_Y )? dst : MAX_HOR_PHASE_Y;
	*/
	bCoffGroupNum = 16;
	/*~ sting bCoffGroupNum to 16 , STeve Lee*/



	Y_H_Inc_Int = hsize_in/hsize_out;
	Y_H_Inc_FraMsb = (hsize_in%hsize_out)*bCoffGroupNum/hsize_out;
	Y_H_Inc_FraLsb =  (((hsize_in%hsize_out)*bCoffGroupNum%hsize_out)<<12)/hsize_out;

	//for 4:2:0 only
	C_H_Inc_Int = hsize_in2/hsize_out;
	C_H_Inc_FraMsb = (hsize_in2%hsize_out)*256/hsize_out;
	C_H_Inc_FraLsb =  (((hsize_in2%hsize_out)*256%hsize_out)<<8)/hsize_out;


//V part

	Y_V_Inc_Int = vsize_in/vsize_out;
	Y_V_Inc_FraMsb = (vsize_in%vsize_out)*256/vsize_out;
	Y_V_Inc_FraLsb =  (((vsize_in%hsize_out)*256%vsize_out)*16)/vsize_out;

	//for 4:2:0 only
	C_V_Inc_Int = vsize_in2/vsize_out;
	C_V_Inc_FraMsb = (vsize_in2%vsize_out)*256/vsize_out;
	C_V_Inc_FraLsb =  (((vsize_in2%vsize_out)*256%vsize_out)*16)/vsize_out;



//1 4 calc init value

//H part
	H_Init_MB = h_mb_offset = x_offset_in/16;

	Y_H_Init_Int = h_pixel_offset = x_offset_in%16;

	//for 4:2:0 only
	C_H_Init_Int = (x_offset_in%16)/2;

	Y_H_Init_FraMsb = 0;
	Y_H_Init_FraLsb =  0;	
	C_H_Init_FraMsb = 0;
	C_H_Init_FraLsb =  0;	

//V part	
	V_Init_MB = v_mb_offset = y_offset_in/16;

	Y_V_O_Init_Int = v_pixel_offset =  y_offset_in%16;
	
	Y_V_O_Init_FraMsb = 0;
	Y_V_O_Init_FraLsb =  0;

	Y_V_E_Init_Int = Y_V_O_Init_Int+Y_V_Inc_Int;//(y_offset_in%16)+1;
	Y_V_E_Init_FraMsb = Y_V_Inc_FraMsb;//0;
	Y_V_E_Init_FraLsb =  Y_V_Inc_FraLsb;//0;
	
	//for 4:2:0 only
	C_V_O_Init_Int = (y_offset_in%16)/2;
	C_V_O_Init_FraMsb = 0;
	C_V_O_Init_FraLsb =  0;
	
	C_V_E_Init_Int = C_V_O_Init_Int+C_V_Inc_Int;//(y_offset_in2%16)+1;
	C_V_E_Init_FraMsb = C_V_Inc_FraMsb;//0;
	C_V_E_Init_FraLsb =  C_V_Inc_FraLsb;//0;	


//1 4 calc screen value

	MpStartX = x_offset_out;
	MpEndX = x_offset_out +hsize_out-1;
	MpStartY = y_offset_out;
	MpEndY = y_offset_out +vsize_out-1;	

//1 5 gen register value


//init
	Base_MB_Offset_RegValue = 0;
	Base_MB_Offset_RegValue |= H_Init_MB & 0x3f;
	Base_MB_Offset_RegValue |= (V_Init_MB & 0x3f) << 16;
	
	Y_H_Init_RegValue = 0;
	Y_H_Init_RegValue |= Y_H_Init_Int & 0x1f;
	Y_H_Init_RegValue |= ((Y_H_Init_FraMsb & 0x0f) << 20);
	Y_H_Init_RegValue |= ((Y_H_Init_FraLsb & 0xfff)<< 8);
	
	C_H_Init_RegValue = 0;	
	C_H_Init_RegValue |= (C_H_Init_Int & 0x1f);	
	C_H_Init_RegValue |= ((C_H_Init_FraMsb & 0xff) << 16);
	C_H_Init_RegValue |= ((C_H_Init_FraLsb & 0xff)<< 8);

	Y_V_Init_RegValue = 0;
	Y_V_Init_RegValue |= (((Y_V_E_Init_Int) & 0x0f) << 16);
	Y_V_Init_RegValue |= ((Y_V_E_Init_FraMsb & 0xff) << 24);
	Y_V_Init_RegValue |= ((Y_V_E_Init_FraLsb & 0x0f) << 20);
	Y_V_Init_RegValue |= ((Y_V_O_Init_Int) & 0x0f);  		
	Y_V_Init_RegValue |= ((Y_V_O_Init_FraMsb & 0xff) << 8);
	Y_V_Init_RegValue |= ((Y_V_O_Init_FraLsb & 0x0f) << 4);
	
	C_V_Init_RegValue =0;
	C_V_Init_RegValue |= (((C_V_E_Init_Int) & 0x0f) << 16);
	C_V_Init_RegValue |= ((C_V_E_Init_FraMsb & 0xff) << 24);
	C_V_Init_RegValue |= ((C_V_E_Init_FraLsb & 0x0f) << 20);
	C_V_Init_RegValue |= ((C_V_O_Init_Int) & 0x0f);  
	C_V_Init_RegValue |= ((C_V_O_Init_FraMsb & 0xff) << 8);
	C_V_Init_RegValue |= ((C_V_O_Init_FraLsb & 0x0f) << 4);

//inc
	Y_H_Inc_RegValue = 0;	
	Y_H_Inc_RegValue |= (Y_H_Inc_Int & 0x0f);
	Y_H_Inc_RegValue |= ((Y_H_Inc_FraMsb & 0x0f) << 20);
	Y_H_Inc_RegValue |= ((Y_H_Inc_FraLsb & 0xfff) << 8);
	
	C_H_Inc_RegValue = 0;	
	C_H_Inc_RegValue |= (C_H_Inc_Int & 0x0f);
	C_H_Inc_RegValue |= ((C_H_Inc_FraMsb & 0xff) << 16);
	C_H_Inc_RegValue |= ((C_H_Inc_FraLsb & 0xff) << 8);

	Y_V_Inc_RegValue = 0;
	Y_V_Inc_RegValue |= (Y_V_Inc_Int & 0x0f);
	Y_V_Inc_RegValue |= ((Y_V_Inc_FraMsb & 0xff) << 16);
	Y_V_Inc_RegValue |= ((Y_V_Inc_FraLsb & 0x0f) << 8);
	
	C_V_Inc_RegValue = 0;
	C_V_Inc_RegValue |= (C_V_Inc_Int & 0x0f);
	C_V_Inc_RegValue |= ((C_V_Inc_FraMsb & 0xff) << 16);
	C_V_Inc_RegValue |= ((C_V_Inc_FraLsb & 0x0f) << 8);


	Mp_X_RegValue = 0;
	Mp_X_RegValue |= (MpStartX & 0x3ff)<<16;
	Mp_X_RegValue |= MpEndX & 0x3ff;
	
	Mp_Y_RegValue = 0;
	Mp_Y_RegValue |= (MpStartY & 0x3ff)<<16;
	Mp_Y_RegValue |= MpEndY & 0x3ff;


	Coff_GN_RegValue = 0;
	Coff_GN_RegValue |= bCoffGroupNum-1;


	Y_H_Normal_Pos_RegValue = 0;
	Y_H_Normal_Pos_RegValue |= (MpEndX & 0x3ff)<<16;
	Y_H_Normal_Pos_RegValue |= MpStartX & 0x3ff;
	
	C_H_Normal_Pos_RegValue = Y_H_Normal_Pos_RegValue;
	
	Y_H_Inc_Acc_RegValue = 0;
	C_H_Inc_Acc_RegValue = 0;	
	Y_H_Inc_Bgn_RegValue = 1;
	C_H_Inc_Bgn_RegValue = 1;		


	
//1 5calc hfiltercoff

	/*sting bCoffGroupNum to 16 , STeve Lee*/
	//GenerateHorFilterCoeff(bCoffGroupNum,g_HorFilterCoffArray);
	/*~sting bCoffGroupNum to 16 , STeve Lee*/


//1 5 write to register



	RegisterArraySetMaskForZoom();	


	WriteRegisterArrayForZoom(0x20,Coff_GN_RegValue);

	WriteRegisterArrayForZoom(0x24,Coff_Index_RegValue);
	WriteRegisterArrayForZoom(0x28,Coff_Msb_RegValue);
	WriteRegisterArrayForZoom(0x2C,Coff_Lsb_RegValue);

	for(i=0;i<16;i++)
	{
		//Filter coeff index************************************************************
		Coff_Index_RegValue = i & 0x0f;
		
        	WriteRegisterArrayForZoom(0x24,Coff_Index_RegValue);
		
		//Filter coeff MSB**************************************************************
		Coff_Msb_RegValue = 0;
		
		if(g_HorFilterCoffArray[i][0] >=0)
		{
			Coff_Msb_RegValue |= ((g_HorFilterCoffArray[i][0] & 0x0FF) << 16 );
		}
		else
		{
			Coff_Msb_RegValue |= ((COMPLEMENT_BYTE(COMPLEMENT(g_HorFilterCoffArray[i][0]))) << 16 );
			Coff_Msb_RegValue |= (1 << 24);
		}
		
		if(g_HorFilterCoffArray[i][1] >=0)
		{
			Coff_Msb_RegValue |= ((g_HorFilterCoffArray[i][1] & 0x0FF));
		}
		else
		{
			Coff_Msb_RegValue |= (COMPLEMENT_BYTE(COMPLEMENT(g_HorFilterCoffArray[i][1])));
			Coff_Msb_RegValue |= (1 << 8);
		}
		
		WriteRegisterArrayForZoom(0x28,Coff_Msb_RegValue);
		
		//Filter coeff LSB*******************************************************************
		Coff_Lsb_RegValue = 0;
		
		if(g_HorFilterCoffArray[i][2] >=0)
		{
			Coff_Lsb_RegValue |= ((g_HorFilterCoffArray[i][2] & 0x1FF) << 16 );
		}
		else
		{
			Coff_Lsb_RegValue |= ((COMPLEMENT_BYTE(COMPLEMENT(g_HorFilterCoffArray[i][2]))) << 16 );
			Coff_Lsb_RegValue |= (1 << 25);
		}
		
		if(g_HorFilterCoffArray[i][3] >=0)
		{
			Coff_Lsb_RegValue |= ((g_HorFilterCoffArray[i][3] & 0x0FF));
		}
		else
		{
			Coff_Lsb_RegValue |= (COMPLEMENT_BYTE(COMPLEMENT(g_HorFilterCoffArray[i][3])));
			Coff_Lsb_RegValue |= (1 << 8);
		}
		
		WriteRegisterArrayForZoom(0x2C,Coff_Lsb_RegValue);	
		
	}




	WriteRegisterArrayForZoom(0x6c,Base_MB_Offset_RegValue); 

	WriteRegisterArrayForZoom(0x00,Y_H_Init_RegValue);	
	WriteRegisterArrayForZoom(0x04,Y_V_Init_RegValue);	
	WriteRegisterArrayForZoom(0x08,C_H_Init_RegValue); 	
	WriteRegisterArrayForZoom(0x0c,C_V_Init_RegValue);	

	WriteRegisterArrayForZoom(0x10,Y_H_Inc_RegValue);	
	WriteRegisterArrayForZoom(0x14,Y_V_Inc_RegValue);	
	WriteRegisterArrayForZoom(0x18,C_H_Inc_RegValue);	
	WriteRegisterArrayForZoom(0x1c,C_V_Inc_RegValue);		

	vp_WriteWord(0x78, (h_pixel_offset+hsize_in+15)/16);
	vp_WriteDword(0x7C, ((v_pixel_offset+vsize_in)<<16)|(h_pixel_offset+hsize_in));


	WriteRegisterArrayForZoom(0x80, Mp_X_RegValue);
	WriteRegisterArrayForZoom(0x84, Mp_Y_RegValue);

	RegisterArrayResetMaskForZoom();
	
	return;
}
void vpo_vdec_link()
{
	unsigned long dwTmp;
	//set mask
	vp_WriteDword(0x90, vp_ReadDword(0x90) | 0x1);
#if 1
	//vp_hal_MP_Addr
	vp_WriteDword(0x50, 0);
	vp_WriteDword(0x54, (unsigned long)dec_frm_buf_y_sd);
	vp_WriteDword(0x58, (unsigned long)dec_frm_buf_y_sd);
	vp_WriteDword(0x5c, (unsigned long)dec_frm_buf_c_sd);
	vp_WriteDword(0x60, (unsigned long)dec_frm_buf_c_sd);
#endif

       //vp_hal_Original_size
#if 0
	unsigned long dwTmp = vp_ReadDword(0x78); 
	vp_WriteDword(0x78,(dwTmp&0xffffffc0) | (((wOrigWidth + 15)>>4)&0x3F));
#endif
	vp_WriteDword(0x7C, ((sd_wOrigHeight & 0x3FF)<<16)|(sd_wOrigWidth& 0x3FF));
	//unsigned char bBufStride = (sqh.horizontal_size+31)>>5;//@M3602 the stride for MPEG-2 video is 32 ,while is 16 for H.264 video
	unsigned char bBufStride = (sd_wOrigWidth+31)>>5;//@M3602 the stride for MPEG-2 video is 32 ,while is 16 for H.264 video
	vp_WriteWord(0x7A,(bBufStride & 0x3F));     //set stride
	//fetch mode
	dwTmp = vp_ReadDword(0x90);
	unsigned char bFetch=1;
	bFetch = (progressive_frame)?1:0;
	vp_WriteDword(0x90, (dwTmp&0xFFF9FFFF)|((bFetch & 0x03)<<17) ); 	
	WriteZoomRegisters(); //empty operate
	if((g_blogo_format==LOGO_FORMAT_PAL)||(g_blogo_format==LOGO_FORMAT_PAL_N))
	{
		mini_vp_sub_zoom(sd_wOrigWidth,sd_wOrigHeight,0);
	}
	else
	{
		mini_vp_sub_zoom(sd_wOrigWidth,sd_wOrigHeight,1);
	}
	//clear mask
	vp_WriteDword(0x90, vp_ReadDword(0x90) & 0xfffffffe);
#if 1
	//memory base address
	unsigned long umem_base=((((unsigned long)dec_frm_buf_y_sd&0x0FF00000)>>20)/32*32)<<20;
	vdec_writeuint8(VDEC_REG_BASEADDR, (umem_base>>25)&0x7F); 
	//vdec_writeuint8(VDEC_REG_BASEADDR,((unsigned long)dec_frm_buf_y&0x0FF00000)>>22);
#endif

}
void vpoInitRegs()
{
	int i;
	unsigned long dwTmp;
	//init the video core
	//0xB8000060 Local Device Clock Gating Control Register(s3602)
	*(unsigned char *)0xb8000060 = (*(unsigned char *)0xb8000060)|0x0c;
	*(unsigned char *)0xb8000060 = (*(unsigned char *)0xb8000060)&0xf3;
	//0xB8000080 Local Device Reset Control Register(s3602)
	*(unsigned char *)0xb8000080 = (*(unsigned char *)0xb8000080)|0x0c;
	*(unsigned char *)0xb8000080 = (*(unsigned char *)0xb8000080)&0xf3;
	//vpoCalcZoom();
//3 Init the HW of VP
	//set mask
	vp_WriteDword(0x90, vp_ReadDword(0x90) | 0x1);
//init regs
	//vp_hal_EdgePreserve
	unsigned long dwControl = vp_ReadDword(0x90);
	unsigned char bddValue=3;
	unsigned char bDiffThreshold=20;
	vp_WriteDword(0x4C, ((bddValue&0x1F)<<8)|(bDiffThreshold));
	dwControl = dwControl & ~(0x01<<27);  //disable bob of edge preserve
	dwControl = dwControl | (0x01<<26); //5x2 mode            
	vp_WriteDword(0x90, dwControl);
	//display value
	if((g_blogo_format==LOGO_FORMAT_PAL)||(g_blogo_format==LOGO_FORMAT_PAL_N))
		dwControl=575;
	else
		dwControl=479;
	vp_WriteDword(0x84,dwControl); //to set the MP Vertical position

	dwControl=0x00108080;   //set background, the color is black 
	vp_WriteDword(0x88,dwControl);
	//clip mode
	dwControl = vp_ReadDword(0x90);
	dwControl &= 0xDDFFFFFF; //clear bit29,25 
	vp_WriteDword(0x90, dwControl);//puzzle  :the bit 29,25 are reserved in 3602 SD register
	//close adpcm
	dwControl = vp_ReadDword(0x90);
	vp_WriteDword(0x90, dwControl&0x7FFFFFFF); //as debug ,can compare the that of  VPO_R_DWORD_SD for sd_h264_mapping
	//vp_hal_EnableVGA640480 false
	dwTmp = vp_ReadDword(0x90);
	dwTmp = dwTmp & ~(0x01<<23);
	vp_WriteDword(0x90, dwTmp);   
	//win onoff
	dwTmp = vp_ReadDword(0x90);
	dwTmp = dwTmp & ~(0x01<<21);
	vp_WriteDword(0x90, dwTmp); 
#if 0
	//fetch mode
	dwTmp = vp_ReadDword(0x90);
	unsigned char bFetch=1;
	bFetch = (progressive_frame)?1:0;
	vp_WriteDword(0x90, (dwTmp&0xFFF9FFFF)|((bFetch & 0x03)<<17) ); 
#endif
	//calc mode -0
	dwTmp = vp_ReadDword(0x90);
	vp_WriteDword(0x90, (dwTmp&0xFFFeFFFF));
	//vp_hal_MemReqThreshold
	unsigned char bMode=0;
	vp_WriteDword(0x90, (vp_ReadDword(0x90)&0xFFFF9FFF)|((bMode & 0x03)<<13) );
	//interlace
	dwTmp = vp_ReadDword(0x90);
	dwTmp = dwTmp & ~(0x01<<8);
	vp_WriteDword(0x90, dwTmp);
	SYS_WriteDWord(0xb8000078, SYS_ReadDWord(0xb8000078)&0xfffff3ff|(0x00<<10));//54MHZ
	//field based
	vp_WriteDword(0x94, 0); 
	//--calculate zoom coeff
#if 0	
	WriteZoomRegisters(); //empty operate

	if((g_blogo_format==LOGO_FORMAT_PAL)||(g_blogo_format==LOGO_FORMAT_PAL_N))
	{
		mini_vp_sub_zoom(sqh.horizontal_size,sqh.vertical_size,0);
	}
	else
	{
		mini_vp_sub_zoom(sqh.horizontal_size,sqh.vertical_size,1);
	}
#endif
	//clear mask
	vp_WriteDword(0x90, vp_ReadDword(0x90) & 0xfffffffe);
//3Init the HW of DVI(standard dvi interface)
	//set mask
	dvi_WriteDword(0x00, dvi_ReadDword(0x00)&(~0x04));
	//dvi_init
	//dvi_hal_Others
	unsigned long uTotalControl = dvi_ReadDword(0x00);
	uTotalControl &= 0x11F3805; //clear all the bits this function cares
	uTotalControl |= (0x01<<9);//set the bit9 = 1
	uTotalControl |= (0x01<<22);//22 bit =1 for M3327
	dvi_WriteDword(0x00, uTotalControl);
	//dvi_hal_DisEnableInterrupt
	unsigned char uInterrupt=0x1f;
	dvi_WriteDword(0x04, dvi_ReadDword(0x04)&(~(uInterrupt & 0x1F))); 
	//dvi_hal_SetYUVOrder(eYUVOrder);
	unsigned char eYUVOrder=0;
	dvi_WriteDword(0x00, (dvi_ReadDword(0x00)&0xFFFFC7FF)|((eYUVOrder&0x07)<<11)); 
	//dvi_hal_ScanMode(bScanMode); 
	uTotalControl = dvi_ReadDword(0x00);
	uTotalControl = uTotalControl & (~0x01010000);// DVI_SCAN_INTERLACE
	uTotalControl = uTotalControl | 0x00060000;
	dvi_WriteDword(0x00, uTotalControl);
	//dvi_hal_ConfigOutput(eOutputMode,bScanMode);
	if((g_blogo_format==LOGO_FORMAT_PAL)||(g_blogo_format==LOGO_FORMAT_PAL_N))
	{
		// PAL 601
		/* 
		dvi_WriteDword(0x0c,0x027106c0);
		dvi_WriteDword(0x10,0x008006c0);
		dvi_WriteDword(0x14,0x06a80104);
		dvi_WriteDword(0x18,0x04030271);
		dvi_WriteDword(0x1C,0x013b0539);
		dvi_WriteDword(0x20,0x00000360);
		dvi_WriteDword(0x24,0x01360016);
		dvi_WriteDword(0x28,0x026f014f);
		dvi_WriteDword(0x2C,0x02710139);
		*/
		dvi_WriteDword(0x0c,0x027106c0);
		dvi_WriteDword(0x10,0x008006c0);
		dvi_WriteDword(0x14,0x06a80104);
		dvi_WriteDword(0x18,0x04030271);
		dvi_WriteDword(0x1C,0x013b0539);
		dvi_WriteDword(0x20,0x00000360);
		dvi_WriteDword(0x24,0x01360016);
		dvi_WriteDword(0x28,0x026f014f);
		dvi_WriteDword(0x2C,0x02710139);
	}
	else
	{
//  N 601
              /* 
		dvi_WriteDword(0x0c,0x020d06b4);//interlace
		dvi_WriteDword(0x10,0x008006b4);
		dvi_WriteDword(0x14,0x069400f0);
		dvi_WriteDword(0x18,0x00060003);
		dvi_WriteDword(0x1C,0x050d050a);
		dvi_WriteDword(0x20,0x0000035a);
		dvi_WriteDword(0x24,0x01050015);
		dvi_WriteDword(0x28,0x020c011c);
		dvi_WriteDword(0x2C,0x0003010a);
		*/
		dvi_WriteDword(0x0c,0x020d06b4);//interlace
		dvi_WriteDword(0x10,0x008006b4);
		dvi_WriteDword(0x14,0x069400f0);
		dvi_WriteDword(0x18,0x00060003);
		dvi_WriteDword(0x1C,0x050d050a);
		dvi_WriteDword(0x20,0x0000035a);
		dvi_WriteDword(0x24,0x01050015);
		dvi_WriteDword(0x28,0x020c011c);
		dvi_WriteDword(0x2C,0x0003010a);
	}
	//dvi_hal_linemeetPosi(uLineMeet);
	unsigned int uLineMeet=230;
	dvi_WriteDword(0x08, dvi_ReadDword(0x08)&(~0x3FF)|(uLineMeet & 0x3FF)); 
	//dvi_api_EnableInterrupt(vp_sub_CheckDVIScanMode(pInitInfo->pDacConfig), DVI_ISR_VBLANK|DVI_ISR_LINEMEET);
	//dvi_hal_ClearMask();
	dvi_WriteDword(0x00, dvi_ReadDword(0x00)|0x04);
//3Register ISR
	//osal_interrupt_register_lsr(17, (OSAL_T_LSR_PROC_FUNC_PTR)VPO_ISR, (unsigned long)dev);
//3Start to playback
	//dvi_hal_Playback(TRUE);
	dvi_WriteDword(0x00, dvi_ReadDword(0x00)|0x01); 
//3Init the HW of TVEncoder(tmp tevencoder interface)
	//tve_init();
	//set_tve_mode(TVE_SLAVE);
	unsigned char TVEREG09 = 0x00;
	unsigned char TVEREG03 = 0x00;
	unsigned char TVEREG0A = 0x00;
	unsigned char TVEREG0B = 0x00;
	unsigned char TVEREG0C = 0x00;
	unsigned char TVEREG0D = 0x00;
	unsigned char TVEREG0E = 0x00;

	if((TVEReadB(0x0E)&0x08)==1)
		{
		while(1);
		}

	unsigned char data=1;
	TVEWriteB(0x02,(TVEReadB(0x02)&0xfd)|(data<<1)); 
	data=0;//0 for rgb,1 for yuv
	TVEWriteB(0x02,(TVEReadB(0x02)&0xfb)|(data<<2));
	//set_dac_onoff(0x0f);
	data=0x0f; // dac 0-3 open
	TVEWriteB(0x04,(TVEReadB(0x04)&0xc0)|data);	     
	//set_dac_blank_input(0);
	unsigned char OnOff=0;
	TVEWriteB(0x17,(TVEReadB(0x17)&0x3f)|(OnOff<<6)); //this register is reserved in S3602
	//set_dac_blank_output(0);
	data=0;
	TVEWriteB(0x17,(TVEReadB(0x17)&0xc0)|data);		//this register is reserved in S3602
	//enable_dacclk_delay(1);
	//set_dacclk_delay(1);
	data=1;
	TVEWriteB(0x05,(TVEReadB(0x05)&0x3f)|(data<<6)); 
	//enable_burst_level_setting(0);
	//enable_burst_freq_adjustment(0);
	data=0;
	TVEWriteB(0x42,(TVEReadB(0x42)&0xfb)|(data<<2)); 
	//enable_burst_freq_directsetting(0);
	data=0;
	TVEWriteB(0x42,(TVEReadB(0x42)&0xfd)|(data<<1)); 
	//set_smoother_threshold(0xff);
	data=0xff;
	TVEWriteB(0x13,data);								
	//set_composite_luma_lpf(0);
	//set_composite_chrma_lpf(0);
	data=0;
	TVEWriteB(0x18,(TVEReadB(0x18)&0xfe)|data);         
	//set_component_luma_lpf(0);
	data=0;
	if(sys_ic_get_chip_id() == ALI_M3202C)
		TVEWriteB(0x18,(TVEReadB(0x18)&0xef)|(data<<4));
	else
		TVEWriteB(0x18,(TVEReadB(0x18)&0xdf)|(data<<5/*3*/)); 	

	//set_component_chrma_lpf(0);
	TVEWriteB(0x18,(TVEReadB(0x18)&0xef)|(data<<4/*5*/));		
	//set_rgb_sync_onoff(0,0,0); //bit 2-0							
	TVEWriteB(0x12,(TVEReadB(0x12)&0xfe)|data);
	TVEWriteB(0x12,(TVEReadB(0x12)&0xfd)|(data<<1));
	TVEWriteB(0x12,(TVEReadB(0x12)&0xfb)|(data<<2));
	//set_composite_y_delay(2);
	data=2;
	TVEWriteB(0x0f,(TVEReadB(0x0f)&0x8f)|(data<<4));				
	//set_composite_c_delay(0);
	data=0;
	TVEWriteB(0x0f,(TVEReadB(0x0f)&0xf1)|(data<<1));				
	if(sys_ic_get_chip_id() == ALI_M3202C)
        {
		//set_component_y_delay(3);
	        data=3;
		TVEWriteB(0x14,(TVEReadB(0x14)&0x8f)|(data<<4));
		//set_component_cb_delay(2);
	        data=2;
		TVEWriteB(0x14,(TVEReadB(0x14)&0xf3)|(data<<2));
		//set_component_cr_delay(0);
	        data=0;
		TVEWriteB(0x14,(TVEReadB(0x14)&0xfc)|data);
        }
	else
        {
		//set_component_y_delay(4);
	        data=4;
		TVEWriteB(0x14,(TVEReadB(0x14)&0x8f)|(data<<4));       
		//set_component_cb_delay(0);
	        data=0;
		TVEWriteB(0x14,(TVEReadB(0x14)&0xf3)|(data<<2));
		//set_component_cr_delay(2);
	        data=2;
		TVEWriteB(0x14,(TVEReadB(0x14)&0xfc)|data);	
		//set_component_sync_delay(5);
		data=5;
		TVEWriteB(0x1b,(TVEReadB(0x1b)&0xf0)|data);
        }

	//tve_set_tvsys( pInitInfo->eTVSys); PAL
	//sub_set_tvsys(eTVSys);
	//set_tvsys_mode(1,1);
	if((g_blogo_format==LOGO_FORMAT_PAL)||(g_blogo_format==LOGO_FORMAT_PAL_N))
	{
		if(sys_ic_get_chip_id() == ALI_M3202C)
	        {//set_component_sync_delay(5);
			data=5;
			TVEWriteB(0x1b,(TVEReadB(0x1b)&0xf0)|data);
		}
		//set_tvsys_mode	
		if(g_blogo_format==LOGO_FORMAT_PAL)
		{
			unsigned char submode = 1;
			TVEREG03= (TVEREG03&0xf9)|(submode<<1)|0x01;
		}
		else
		{
			unsigned char submode = 2;
			TVEREG03= (TVEREG03&0xf9)|(submode<<1)|0x01;
		}
		//625 line
		//set_phase_compensation(0x0130);
		data=0x30;TVEREG09= (unsigned char)(data);//TVEREG09= (unsigned char)(data&0x00ff);
		data=0x01;TVEREG0A= (unsigned char)(data);//TVEREG0A= (unsigned char)(data>>8);
		//enable_burst_timing_setting(0);
		unsigned char onoff=0;
		TVEREG0E = (TVEREG0E&0xfb)|(onoff<<2) ;
		//set_burst_timing(0x00, 0x00, 0x00);
		TVEREG0B = 0;
		TVEREG0C = 0;
		TVEREG0D = 0;
		//enable_burst_level_setting(1);
		onoff=1;
		TVEWriteB(0x42,(TVEReadB(0x42)&0xEF)|(onoff<<4));//cr
		TVEWriteB(0x42,(TVEReadB(0x42)&0xF7)|(onoff<<3));//cb
		//set_burst_cb_level(0x85);
		data=0x85;
		TVEWriteB(0x43,data);
		//set_burst_cr_level(0x55);
		data=0x55;
		TVEWriteB(0x44,data);
		//enable_burst_freq_directsetting(0);
		data=0;
		TVEWriteB(0x42,(TVEReadB(0x42)&0xfd)|(data<<1));
		//enable_burst_freq_adjustment(0);
		TVEWriteB(0x42,(TVEReadB(0x42)&0xfb)|(data<<2));
		//set_pedestal_onoff(0);
		data = 0;
		TVEREG03= (TVEREG03&0xf7)|(data<<3);
		//for composite
		//set_composite_pedestal_level(0x00);      
		TVEWriteB(0x4b,data);
		//set_composite_luma_level(0x52);           
		data=0x52;
		TVEWriteB(0x1c,data);             
		//adj_composite_chrma_level(0x13);
		data=0x13;
		TVEWriteB(0x46,data);                           
		//set_composite_sync_level(0x0);
		data=0;
		TVEWriteB(0x1d,(TVEReadB(0x1d)&0xf0)|data);
		//for component
		//set_component_pedestal_level(0x00);
		TVEWriteB(0x1b,(TVEReadB(0x1b)&0x0f)|(data<<4)); 
		//set_component_luma_level(0x53);
		data=0x53;
		TVEWriteB(0x19,data);                                                
		//set_component_chroma_level(0x51);
		data=0x51;
		TVEWriteB(0x1a,data);                                             
		//set_component_sync_level(0x00);
		data=5;
		//TVEWriteB(0x1d,data);
		TVEWriteB(0x1d,(TVEReadB(0x1d)&0x0f)|(data<<4)); 

		
	}
	else if((g_blogo_format==LOGO_FORMAT_NTSC)||(g_blogo_format==LOGO_FORMAT_NTSC_443)||(g_blogo_format==LOGO_FORMAT_PAL_N)||(g_blogo_format==LOGO_FORMAT_PAL_60)
		|| (g_blogo_format == LOGO_FORMAT_PAL_M))
	{
		if(sys_ic_get_chip_id() == ALI_M3202C)
	        {//set_component_sync_delay(a);
			data=0xa;
			TVEWriteB(0x1b,(TVEReadB(0x1b)&0xf0)|data);
		}
		if((g_blogo_format==LOGO_FORMAT_NTSC)||(g_blogo_format==LOGO_FORMAT_NTSC_443))
		{
			//set_tvsys_mode(0,0);
			unsigned char submode = 0;
			TVEREG03= (TVEREG03&0xf8)|(submode<<1);  
		}
		else if(g_blogo_format==LOGO_FORMAT_PAL_N)  
		{
			//set_tvsys_mode(1,0);
			unsigned char submode = 0;
			TVEREG03= (TVEREG03&0xf9)|(submode<<1)|0x01;
		}
		else if(g_blogo_format==LOGO_FORMAT_PAL_60)
		{
			//set_tvsys_mode(1,3);
			unsigned char submode = 3;
			TVEREG03= (TVEREG03&0xf9)|(submode<<1)|0x01;
		}
		else if(g_blogo_format == LOGO_FORMAT_PAL_M)
		{
			//set_tvsys_mode(1,2); 
			unsigned char submode = 2;  
			TVEREG03= (TVEREG03&0xf9)|(submode<<1)|0x01;		
		}	
		//TVE_PRINTF("SYS_525_LINE \n");
		//set_phase_compensation(0x07A0); 
		data=0xa0;TVEREG09= (unsigned char)(data);//TVEREG09= (unsigned char)(data&0x00ff);
		data=0x07;TVEREG0A= (unsigned char)(data);//TVEREG0A= (unsigned char)(data>>8);
		//enable_burst_timing_setting(1);
		unsigned char onoff=1;
		TVEREG0E = (TVEREG0E&0xfb)|(onoff<<2) ; 
		//set_burst_timing(0x2e, 0x72, 0x0c);  
		TVEREG0B = 0x2e;
		TVEREG0C = 0x72;
		TVEREG0D = 0xc;
		//enable_burst_level_setting(0);   
		onoff=0;
		TVEWriteB(0x42,(TVEReadB(0x42)&0xEF)|(onoff<<4));//cr
		TVEWriteB(0x42,(TVEReadB(0x42)&0xF7)|(onoff<<3));//cb
		if(g_blogo_format==LOGO_FORMAT_NTSC_443)
		{
			//enable_burst_freq_directsetting(1);
			data=1;
			TVEWriteB(0x42,(TVEReadB(0x42)&0xfd)|(data<<1));  
			//enable_burst_freq_adjustment(1);
			TVEWriteB(0x42,(TVEReadB(0x42)&0xfb)|(data<<2));  
			//adj_burst_freq(1,0x54131596);  
			data=0x96;
			TVEWriteB(0x47,data);
			data=0x15;
			TVEWriteB(0x48,data);
			data=0x13;
			TVEWriteB(0x49,data);
			data=0x54;
			TVEWriteB(0x4a,data);
			TVEWriteB(0x42,(TVEReadB(0x42)&0xfe)|1);
		}
		else
		{
			//enable_burst_freq_directsetting(0);
			data=0;
			TVEWriteB(0x42,(TVEReadB(0x42)&0xfd)|(data<<1));   
			//enable_burst_freq_adjustment(0);
			TVEWriteB(0x42,(TVEReadB(0x42)&0xfb)|(data<<2));  
		}
		//set_pedestal_onoff(1);
		data = 1;
		TVEREG03= (TVEREG03&0xf7)|(data<<3); //for component 
		//for composite  //the following settings for composite
		//set_composite_pedestal_level(0x2b);
		data=0x2b;
		TVEWriteB(0x4b,data);
		//set_composite_luma_level(0x4e);
		data=0x4e;
		TVEWriteB(0x1c,data);
		//adj_composite_chrma_level(0xc);
		data=0xa;
		TVEWriteB(0x46,data);
		//set_composite_sync_level(0x0);
		data=5;
		TVEWriteB(0x1d,(TVEReadB(0x1d)&0xf0)|data);
		//for component   
		//set_component_pedestal_level(0x00);
		data=9;
		TVEWriteB(0x1b,(TVEReadB(0x1b)&0x0f)|(data<<4));
		//set_component_luma_level(0x54);
		data=0x4f;
		TVEWriteB(0x19,data);
		//set_component_chroma_level(0x51);
		data=0x51;
		TVEWriteB(0x1a,data);
		//set_component_sync_level(0x04);
		data=5;
		TVEWriteB(0x1d,(TVEReadB(0x1d)&0x0f)|(data<<4));
	}	
	//set_vbi_onoff(0,0,0,0); 
	unsigned char wss=0;
	unsigned char cgms=0;
	unsigned char cc=0;
	unsigned char ttx=0;
	TVEWriteB(0x37, (wss<<7)|(cgms<<6)|(cgms<<5)|(cgms<<4)|(cc<<3)|(cc<<1)|cc);
	TVEWriteB(0x68, (TVEReadB(0x68)&0xfe)|ttx);
	//update_register();    
	TVEWriteB(0x03,TVEREG03);
	TVEWriteB(0x09,TVEREG09);
	TVEWriteB(0x0A,TVEREG0A);
	TVEWriteB(0x0B,TVEREG0B);
	TVEWriteB(0x0C,TVEREG0C);
	TVEWriteB(0x0D,TVEREG0D);
	TVEWriteB(0x0E,TVEREG0E|0x08);
}
void tvenc_set_dac(unsigned char num,unsigned char type)
{
	unsigned long data;
	unsigned char tmp8 = 0;
	
//set_dac_config()
	TVEWriteB(0x08, ((num+1)<<4) | type);
//hal_dac_config()
	TVEWriteB(0x84, (((num+1)&0xF)<<4)|(9&0xF));
//hal_dac_onoff()
	tmp8 = TVEReadB(0x85);
	tmp8 = tmp8 & (~(0x01<<(num)));
	TVEWriteB(0x85, tmp8);
	/*
	if(sys_ic_get_chip_id() == ALI_M3202C)
	{
		TVEWriteB(0x08, ((num+1)<<4) | type);
	}
	else// 3329E
	{
		data = TVEReadD(0x84);
		TVEWriteB(0x08, ((num+1)<<4) | type);
		data &= ~(0xff);
		data |= (num+1)<<4;
		data |= 9 + num;
		TVEWriteD(0x84,data);
		TVEWriteB(0x85,(TVEReadB(0x85)&0xc0)|((~dac_msk)&0xf));
	}
	*/
}
void vpoInitGlobalValue()
{
	progressive_frame = 1;
	sqh.vertical_size = 576;
	sqh.horizontal_size = 720;
}
void anaLogoSize(unsigned long logo_width, unsigned long logo_height)
{
	hd_wOrigWidth=logo_width;
	hd_wOrigHeight=logo_height;
	IsHDLogo = 0;
	v_precision = 1;
	h_precision  =1;
	if ((logo_width>720) ||(logo_height>576))
	{
		IsHDLogo = 1;
		if (logo_width >1280)
			h_precision  = 4;
		else if (logo_width>720)
			h_precision = 2;
		if (logo_height >576)
			v_precision = 2;
	}
	sd_wOrigWidth    = logo_width/h_precision;
	sd_wOrigHeight   = logo_height/v_precision;
	if (IsHDLogo)
	{
		dec_frm_buf_y_hd = dec_mem_top - FRM_Y_SIZE_HD;
		dec_frm_buf_c_hd = dec_frm_buf_y_hd - FRM_C_SIZE_HD;
		dec_frm_buf_y_sd = dec_frm_buf_c_hd - FRM_Y_SIZE_SD;
		dec_frm_buf_c_sd = dec_frm_buf_y_sd - FRM_C_SIZE_SD;
	}
	else
	{
		dec_frm_buf_y_hd = dec_mem_top - FRM_Y_SIZE_SD;
		dec_frm_buf_c_hd = dec_frm_buf_y_hd - FRM_C_SIZE_SD;
		dec_frm_buf_y_sd = dec_frm_buf_y_hd;
		dec_frm_buf_c_sd = dec_frm_buf_c_hd;
	}
}
void decLogo(unsigned char *dataAddr,unsigned long dataLen)
{
	unsigned char bcontrolbit;
	unsigned long uoffset = 0,uLength,i,uend,uStart;
	unsigned char *pbuff=dataAddr;
	unsigned long utmp,logo_width,logo_height;

//3  1:parse header
	cur_ptr=dataAddr;
	uoffset=decParseHeader(pbuff);	
	//uoffset=decFindSliceHeader(pbuff);
//3  2:configure some parameters according to the logo size
	logo_width  = sqh.horizontal_size;
	logo_height = sqh.vertical_size;
	anaLogoSize(logo_width,logo_height);
//3  3:build the bridge between vpo with vdec
	vpo_vdec_link();
	sdhd_vpo_vdec_link();
//3  4:init video register
	decInitRegs();
//3  5:config hardware
	//vld
   	pbuff+=uoffset;
	uLength=dataLen-uoffset;
	bcontrolbit=0x07;
	uStart=(unsigned long)dataAddr;
	uend=((unsigned long)dataAddr)+dataLen-1;
	vdec_writeuint32(VDEC_REG_VLDOFFSET, ((uoffset<<3)&0x07FFFFFF));
       vdec_writeuint32(VDEC_REG_VLDENDADDR, (vdec_readuint32(VDEC_REG_VLDENDADDR)&0xFF000000)|(uend&0x00FFFFFF));
	vdec_writeuint32(VDEC_REG_VLDLEN, ((uLength<<3)&0x03FFFFFF));
	vdec_writeuint32(VDEC_REG_VLDBADDR, (vdec_readuint32(VDEC_REG_VLDBADDR)&0xc0000000)|(uStart&0x00FFFFFF)|((uStart&0x07000000)<<3) |((bcontrolbit&0x07)<<24));		
	
	//picture order
	utmp = vdec_readuint32(VDEC_REG_ORDERCTRL);
	unsigned char ur_frm,ufwd_frm,ubwd_frm;
	ur_frm=0;
	ufwd_frm=2;
	ubwd_frm=1;
	utmp =(utmp&0xFFFE00FF)|((ur_frm&0x07)<<14)|((ufwd_frm&0x07)<<11)|((ubwd_frm&0x07)<<8);
	vdec_writeuint32(VDEC_REG_ORDERCTRL, utmp);                          
	//dview off
	//vdec_writeuint32(VDEC_REG_VECTRL, vdec_readuint32(VDEC_REG_VECTRL) & 0xFFFFFBFF);
	//mini_hal_dview_onoff(0); 
	//de ve use diff buffer
	utmp = vdec_readuint32(VDEC_REG_ORDERCTRL);
	utmp = (utmp&0xFEFFFFFF);
	vdec_writeuint32(VDEC_REG_ORDERCTRL, utmp); 
	//head info
	/*
	if(sqh.vertical_size==480)
	vdec_writeuint32(VDEC_REG_MPHR, 0x111113c8);
	else if(sqh.vertical_size==576)
	vdec_writeuint32(VDEC_REG_MPHR, 0x1FFFF398);
	*/
	vdec_writeuint32(VDEC_REG_MPHR,head_info); 
	
	//source size 720/16,480/16
	unsigned char uheight_mb,uwidth_mb;
	uheight_mb=(sqh.vertical_size+15)>>4;
	uwidth_mb=(sqh.horizontal_size+15)>>4;
	vdec_writeuint32(VDEC_REG_FSIZE, uheight_mb|(uwidth_mb<<8)); 
	//iqm                               
	for(i=0;i<64;i++)
	{
		utmp = iqm_1_array[i] |((i&0x3F)<<8) | 0x4000;
		vdec_writeuint32(VDEC_REG_IQMINPUT, utmp);
	}
	for(i=0;i<64;i++)
	{
		utmp = iqm_0_array[i] |(((i&0x3F)<<8) & 0xFFFFBFFF);
		vdec_writeuint32(VDEC_REG_IQMINPUT, utmp);
	}
	//mb count
	vdec_writeuint32(VDEC_REG_CRTMBADDR, 0); 
	//err flag
	vdec_writeuint32(VDEC_REG_ERRFLAG, 0);      
	//reset status
	vdec_writeuint32(VDEC_REG_VESTAT,0xFFFFFFFF); 
	//set mb posi
	unsigned int mbx,mby;
	mbx=0;
	mby=0;
	vdec_writeuint32(VDEC_REG_MBADDR, ((mbx&0x7F)<<8)|(mby&0x7F));   
	//start
	unsigned char ustart_type;
	ustart_type=0xff;
	vdec_writeuint32(VDEC_REG_VETRIGGER, (vdec_readuint32(VDEC_REG_VETRIGGER)&0xFFFFFFF0)|(ustart_type&0x0F));
//3 6:wait decode finish
	do{
		utmp=vdec_readuint32(VDEC_REG_VESTAT) &1;
	}while(utmp==0);
//3 7:show on sd screen
	unsigned long dwTmp = vp_ReadDword(0x90);
	dwTmp = dwTmp | 0x01<<21;
	vp_WriteDword(0x90, dwTmp);
//3 8:show on sdhd screen
	dwTmp = SYS_ReadDWord(sdhd_VP_REG_BASEADDR+0x90);
	dwTmp = dwTmp | 0x01<<21;
	SYS_WriteDWord((sdhd_VP_REG_BASEADDR+0x90), dwTmp);
	
}
void declogo_attach_min(unsigned char *top,unsigned long size)
{
	dec_mem_top = top;
	dec_mem_size = size;
}

#endif

