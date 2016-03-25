#include <dp8051xp.h>
#include <stdio.h>
#include "ir.h"
#include "sys.h"

//=====================================================================================//
#define IR_BUF_LEN                                                                                 256
#define CLK_CYC_US                                                                                16/* Work clock cycle, in uS */   //===>>>16
#define TIMEOUT_US                                                                                24000/* Timeout threshold, in uS */
#define NOISETHR_US                                                                              80/* Noise threshold, in uS */
#define PAN_KEY_INVALID			                                                  0xFFFFFFFF
#define PULSE_LOW		                                                                       ((unsigned short)0x00 << 15)
#define PULSE_HIGH		                                                                       ((unsigned short)0x01 << 15)
#define PULSE_POL(x)	                                                                              (x & ((unsigned short)0x01 << 15))
#define PULSE_VALUE(x)	                                                                       (x & (~((unsigned short)0x01 << 15)))
#define INRANGE(x, value, tol)		                                                         ((x) > ((value)-(tol)) && (x) < ((value)+(tol)))
#define IRP_CNT		                                                                              8//(sizeof(irc_decoders) / sizeof(struct irc_decoder))
#define COPY_CODE(dst, src)		                                                         do {int i; for(i=0;i<16;i++) *((UINT8 *)dst+i) = *((UINT8 *)src+i);} while(0)
#define CLEAR_CODE(p)			                                                         do {int i; for(i=0;i<16;i++) *((UINT8 *)p+i) = 0;} while(0)

//=====================================================================================//
extern unsigned long xdata  g_ir_power_key[8];
UINT8 key_cnt = 0;
unsigned char bufferin = 0;
unsigned char bufferout = 0;
unsigned char ir_buffer[IR_BUF_LEN];
IR_KEY g_set_ir_key;
static UINT8 g_ir_flag = 0;

static void reverse_code_bit(UINT32 *ir_code, UINT32 bit_cnt);
static void reverse_code_byte(UINT32 *ir_code, UINT8 bit_cnt);
static UINT32 irc_decode(struct irc_decoder *ird, UINT32 pulse_width, UINT32 pulse_polarity);
UINT32 irc_pulse_to_code(UINT32 pulse_width, UINT32 pulse_polarity);
void IR_GenerateCode(void);
void IR_ISR(void);	
void IR_Init(void);

enum irc_decode_ret
{
	IRC_DECODE_SUCCEED,
	IRC_DECODE_DATA0,
	IRC_DECODE_DATA1,
	IRC_DECODE_FAIL,
	IRC_DECODE_STOP
};

enum irp_type
{
	IR_TYPE_NEC=0,
	IR_TYPE_LAB,
	IR_TYPE_50560,
	IR_TYPE_KF,
	IR_TYPE_LOGIC,
	IR_TYPE_SRC,
	IR_TYPE_NSE,
	IR_TYPE_RC5,
	IR_TYPE_RC5_X
};

static const char *irp_strs[] = 
{
	"nec",
	"lab",
	"50560",
	"kf",
	"logic",
	"src",
	"nse",
	"rc5",
	"rc5_x",

};

enum ir_waveform
{
	IR_LEADING=0,
	IR_SPECIAL,
	IR_DATA,
	IR_REPEAT_LEADING,
	IR_REPEAT_DATA,
	IR_STOP,
	IR_END,
	IR_NULL
};

struct irc_pulse
{
	UINT8 type;
	UINT16 fst_half;
	UINT16 scd_half;
	UINT16 tolerance;
};

struct ir_attr
{
	UINT8 type;
	UINT8 bit_msb_first	: 1;
	UINT8 byte_msb_first	: 1;
	UINT8 pulse_invert		: 1;
	UINT8 repeat_enable	: 1;
	UINT8 ignore_lastpulse	: 1;
	UINT8 pulse_prec;
	UINT8 pulse_max_width;
	struct irc_pulse pulse[7];
	enum ir_waveform *normal;
	enum ir_waveform *repeat;
};

struct irc_decoder
{
	struct ir_attr *attr;
	UINT8 first_half_got;
	UINT8 decode_step;
	UINT8 key_bit_cnt;
	UINT16 last_pulse_width;
	UINT32 key_code[4];
	UINT32 last_key_code[4];
};

static enum ir_waveform ir_nec_normal[] = 
{
	IR_LEADING,   						 // start bit
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  //8 bits command 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,   //8 bits address
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  //8 bits ~command 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  //8 bits ~address
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_END, 
};

static enum ir_waveform ir_nec_repeat[] = 
{
	IR_REPEAT_LEADING, 
	IR_END, 
};

static struct ir_attr ir_attr_nec = 
{
	IR_TYPE_NEC,
	1,
	0,
	0,
	1,
	0,
	280,
	15000,
	{
		{IR_LEADING, PULSE_LOW | 9000, PULSE_HIGH | 4500, 500}, // start pulse
		{IR_NULL, 0, 0, 0},
		{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200},  //logic '0'
		{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1680, 300},  // logic '1'
		{IR_REPEAT_LEADING, PULSE_LOW | 9000, PULSE_HIGH | 2250, 500},
		{IR_REPEAT_DATA, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000},
	{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000}, 
	},
	ir_nec_normal, 
	ir_nec_repeat, 
};

static enum ir_waveform ir_lab[] = 
{
	IR_LEADING, 
	IR_SPECIAL, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA,
	IR_END, 
};

static struct ir_attr ir_attr_lab = 
{
	IR_TYPE_LAB,
	1,
	0,
	0,
	0,
	0,
	140,
	15000,
	{
	{IR_LEADING, PULSE_LOW | 280, PULSE_HIGH | 7300, 140},
	{IR_SPECIAL, PULSE_LOW | 280, PULSE_HIGH | 6150, 1500},
	{IR_DATA, PULSE_LOW | 280, PULSE_HIGH | 4780, 500},
	{IR_DATA, PULSE_LOW | 280, PULSE_HIGH | 7300, 500},
	{IR_NULL, 0, 0, 0},
	{IR_NULL, 0, 0, 0},
	{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000}, 
	},
	ir_lab, 
	NULL, 
};

static enum ir_waveform ir_50560[] = 
{
	IR_LEADING, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_SPECIAL,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_SPECIAL,
	IR_END, 
};

static struct ir_attr ir_attr_50560 = 
{
	IR_TYPE_50560,
	1,
	0,
	0,
	1,
	0,
	260,
	15000,
	{
	{IR_LEADING, PULSE_LOW | 8400, PULSE_HIGH | 4200, 500},
	{IR_SPECIAL, PULSE_LOW | 520, PULSE_HIGH | 4200, 500},
	{IR_DATA, PULSE_LOW | 520, PULSE_HIGH | 1050, 500},
	{IR_DATA, PULSE_LOW | 520, PULSE_HIGH | 2100, 500},
	{IR_NULL, 0, 0, 0},
	{IR_NULL, 0, 0, 0},
	{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000}, 
	},
	ir_50560, 
	NULL, 
};

static enum ir_waveform ir_kf[] = 
{
	IR_LEADING, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_END, 
};

static struct ir_attr ir_attr_kf = 
{
	IR_TYPE_KF,
	0,
	1,
	0,
	0,
	0,
	190,
	20000,
	{
	{IR_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 1800, 500},
	{IR_NULL, 0, 0, 0},
	{IR_DATA, PULSE_LOW | 380, PULSE_HIGH | 380, 150},
	{IR_DATA, PULSE_LOW | 380, PULSE_HIGH | 1350, 300},
	{IR_NULL, 0, 0, 0},
	{IR_NULL, 0, 0, 0},
	{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 20000, 1000}, 
	},
	ir_kf, 
	NULL, 
};

static enum ir_waveform ir_logic[] = 
{
	IR_LEADING, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_END, 
};

static struct ir_attr ir_attr_logic = 
{
	IR_TYPE_LOGIC,
	0,
	0,
	0,
	0,
	0,
	125, 
	20000,
	{
	{IR_LEADING, PULSE_LOW | 5000, PULSE_HIGH | 5000, 500},
	{IR_NULL, 0, 0, 0},
	{IR_DATA, PULSE_LOW | 250, PULSE_HIGH | 500, 100},
	{IR_DATA, PULSE_LOW | 500, PULSE_HIGH | 1000, 200},
	{IR_NULL, 0, 0, 0},
	{IR_NULL, 0, 0, 0},
	{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 20000, 1000}, 
	},
	ir_logic, 
	NULL, 
};

static enum ir_waveform ir_src_normal[] = 
{
	IR_LEADING, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_SPECIAL, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_END, 
};

static struct ir_attr ir_attr_src = 
{
	IR_TYPE_SRC,
	1,
	0,
	0,
	1,
	0,
	280,
	60000,
	{
	{IR_LEADING, PULSE_LOW | 6000, PULSE_HIGH | 2000, 400},
	{IR_SPECIAL, PULSE_LOW | 3640, PULSE_HIGH | 3640, 400},
	{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200},
	{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1120, 300},
	{IR_NULL, 0, 0, 0},
	{IR_NULL, 0, 0, 0},
	{IR_STOP, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000}, 
	},
	ir_src_normal, 
	NULL, 
};

static enum ir_waveform ir_nse_normal[] = 
{
	IR_LEADING, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_LEADING, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_END, 
};

static enum ir_waveform ir_nse_repeat[] = 
{
	IR_REPEAT_LEADING, 
	IR_END, 
};

static struct ir_attr ir_attr_nse = 
{
	IR_TYPE_NSE,
	1,
	0,
	0,
	1,
	0,
	280,
	60000,
	{
	{IR_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 1800, 400},
	{IR_NULL, 0, 0, 0},
	{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200},
	{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1120, 300},
	{IR_REPEAT_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 3640, 500},
	{IR_REPEAT_DATA, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000},
	{IR_STOP, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000}, 
	},
	ir_nse_normal, 
	ir_nse_repeat, 
};

static enum ir_waveform ir_rc5[] = 
{
	IR_LEADING, IR_LEADING,   //S1 & S2 bit
	IR_SPECIAL, 			  //toggle bit
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  // 5 bits addr & 6 bits command
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA,
	IR_END,    		 // ???
};

static struct ir_attr ir_attr_rc5 = 
{
	IR_TYPE_RC5,  // type name
	1,			//bits MSB
	0,			
	0,
	0,
	1,			// repeat enable 
	200,			//pulse prec
	15000,		// pulse max width 
	{
		{IR_LEADING, PULSE_HIGH | 830, PULSE_LOW | 830, 200}, // leading logic 
		{IR_SPECIAL, PULSE_HIGH | 830, PULSE_LOW | 830, 200},  // toggle logic 
		{IR_DATA, PULSE_LOW | 830, PULSE_HIGH | 830, 200},	   // data logic '0'
		{IR_DATA, PULSE_HIGH | 830, PULSE_LOW | 830, 200},    // data logic '1'
		{IR_NULL, 0, 0, 0},
		{IR_NULL, 0, 0, 0},
		{IR_STOP, PULSE_HIGH | 830, PULSE_LOW | 15000, 1000}, 
	},
	ir_rc5, 
	NULL, 
};

static enum ir_waveform ir_rc5_x[] = 
{
	IR_LEADING,
    IR_DATA,   //S1 & S2 bit
	IR_DATA, 			 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  // 5 bits addr & 6 bits command
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA,
};

static struct ir_attr ir_attr_rc5_x = 
{
	IR_TYPE_RC5,  // type name
	1,			//bits MSB
	0,			
	0,
	0,
	1,			// repeat enable 
	200,			//pulse prec
	15000,		// pulse max width 
	{
		{IR_LEADING, PULSE_HIGH | 830, PULSE_LOW | 830, 200}, // leading logic 
		{IR_SPECIAL, PULSE_HIGH | 830, PULSE_LOW | 830, 200},  // toggle logic 
		{IR_DATA, PULSE_LOW | 830, PULSE_HIGH | 830, 200},	   // data logic '0'
		{IR_DATA, PULSE_HIGH | 830, PULSE_LOW | 830, 200},    // data logic '1'
		{IR_NULL, 0, 0, 0},
		{IR_NULL, 0, 0, 0},
		{IR_STOP, PULSE_HIGH | 830, PULSE_LOW | 15000, 1000}, 
	},
	ir_rc5_x, 
	NULL, 
};

struct irc_decoder irc_decoders_nec = 
{
	&ir_attr_nec,//.attr = &ir_attr_nec,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_lab = 
{
	&ir_attr_lab,//.attr = &ir_attr_lab,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_50560 = 
{
	 &ir_attr_50560,//.attr = &ir_attr_50560,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_kf = 
{
	&ir_attr_kf,//.attr = &ir_attr_kf,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.key_code = {0,},
	{0,},//.last_pulse_width = 0,
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_logic = 
{
	&ir_attr_logic,//.attr = &ir_attr_logic,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_src = 
{
	&ir_attr_src,//.attr = &ir_attr_src,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_nse = 
{
	 &ir_attr_nse,//.attr = &ir_attr_nse,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_rc5 = 
{
	&ir_attr_rc5,//.attr = &ir_attr_rc5,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_rx5_x = 
{
	&ir_attr_rc5_x,//.attr = &ir_attr_rc5_x,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

static struct irc_decoder *irc_decoders[]=
{
	&irc_decoders_nec,
	&irc_decoders_lab,
	&irc_decoders_50560,
	&irc_decoders_kf,
	&irc_decoders_logic,
	&irc_decoders_src,
	&irc_decoders_nse,
	&irc_decoders_rc5,
	&irc_decoders_rx5_x,
};

//=====================================================================================//
static void reverse_code_bit(UINT32 *ir_code, UINT32 bit_cnt)
{
	UINT8 i = 0;
	UINT32 src_code[4] = {0};
	UINT32 dst_code[4] = {0};
	
	COPY_CODE(src_code, ir_code);

	for (i=0; i<bit_cnt; i++)
	{
		if(src_code[i / 32] & ((unsigned long)0x01L << (i % 32)))
		{
			dst_code[(bit_cnt - 1 - i) / 32] |= (unsigned long)0x01L << ((bit_cnt - 1 - i) % 32);
		}
	}
	
	COPY_CODE(ir_code, dst_code);
}

static void reverse_code_byte(UINT32 *ir_code, UINT8 bit_cnt)
{
	UINT8 i = 0;
	UINT8 src_code[16] = {0,};
	UINT8 dst_code[16] = {0,};
	UINT8 byte_cnt = (bit_cnt  + 7) / 8;

	//sample
	//src:   58 59 4e 48 00 00 57 50
	//mips: 48 4e 59 58 50 57 00 00
	//rev:   57 50 58 59 4e 48 00 00
	//dst:    59 58 50 57 00 00 4e 48

	COPY_CODE(src_code, ir_code);

	dst_code[0] = src_code[3];
	dst_code[1] = src_code[2];
	dst_code[2] = src_code[1];
	dst_code[3] = src_code[0];
	dst_code[4] = src_code[7];
	dst_code[5] = src_code[6];
	dst_code[6] = src_code[5];
	dst_code[7] = src_code[4];
	dst_code[8] = src_code[11];
	dst_code[9] = src_code[10];
	dst_code[10] = src_code[9];
	dst_code[11] = src_code[8];
	dst_code[12] = src_code[15];
	dst_code[13] = src_code[14];
	dst_code[14] = src_code[13];
	dst_code[15] = src_code[12];
	
	COPY_CODE(src_code, dst_code);
	for (i=0; i<byte_cnt; i++)
	{
		dst_code[i] = src_code[byte_cnt - i - 1];
	}

	src_code[0] = dst_code[3];
	src_code[1] = dst_code[2];
	src_code[2] = dst_code[1];
	src_code[3] = dst_code[0];
	src_code[4] = dst_code[7];
	src_code[5] = dst_code[6];
	src_code[6] = dst_code[5];
	src_code[7] = dst_code[4];
	src_code[8] = dst_code[11];
	src_code[9] = dst_code[10];
	src_code[10] = dst_code[9];
	src_code[11] = dst_code[8];
	src_code[12] = dst_code[15];
	src_code[13] = dst_code[14];
	src_code[14] = dst_code[13];
	src_code[15] = dst_code[12];
    
	COPY_CODE(ir_code, src_code);
}

/*****************************************************************************
 * UINT32 irc_decode(UINT32 pulse_width)
 * Description: Translate pulse width to IR code
 *
 * Arguments:
 *	  struct irc_decoder *ird	: IR decode structure
 *    UINT32 pulse_width		: Input pulse width, in uS
 *    UINT32 pulse_polarity		: Input pulse polarity, 
 *									'1' = high level, '0' = low level
 *
 * Return Value:
 *    INT32						: Key code
 ****************************************************************************/
 static UINT32 irc_decode(struct irc_decoder *ird, UINT32 pulse_width, UINT32 pulse_polarity)
{
	UINT32 accum_pulse_width = 0;
	enum irc_decode_ret result = IRC_DECODE_FAIL;
	enum ir_waveform step;
	enum ir_waveform next_step;
	UINT16 step_temp = 0;
	UINT32 last_key = 0, accum_pulse = 0, tolerance_pulse = 0;
	UINT32 same_polarity = 0, accum_pulse_1 = 0, tolerance_pulse_1 = 0;
	UINT32 same_polarity_1 = 0, temp_long = 0x5a5a55aa, half_pol = 0, half_width = 0;

	/* Ignore burr pulses */
	if (pulse_width < ird->attr->pulse_prec)
	{
		ird->first_half_got = 0;
		return PAN_KEY_INVALID;
	}

	if (ird->first_half_got == 0)/* first_half_got is flag  for dule-pulse senser */
	{
		if (ird->decode_step == 0)/*just check leading pulse*/
		{
			if (PULSE_POL(ird->attr->pulse[0].fst_half))/* if leading start with high level */
			{
				half_pol = PULSE_POL(ird->attr->pulse[0].scd_half);
				half_width = PULSE_VALUE(ird->attr->pulse[0].scd_half);
			}
			else
			{
				half_pol = PULSE_POL(ird->attr->pulse[0].fst_half);
				half_width = PULSE_VALUE(ird->attr->pulse[0].fst_half);
			}

			/* Check leading code first half pulse polarity */
			if (half_pol ^ (pulse_polarity << 15))
			{
				return PAN_KEY_INVALID;
			}

			/* Check leading code first half pulse width */
			if (!INRANGE(pulse_width, half_width, ird->attr->pulse[0].tolerance))
			{
				return PAN_KEY_INVALID;
			}
	
		}
		ird->first_half_got = 1;
		ird->last_pulse_width = pulse_width;

		if (ird->decode_step == 0 && (PULSE_POL(ird->attr->pulse[0].fst_half) ? 1 : 0))
		{
			ird->first_half_got = 0;
			ird->last_pulse_width = 0;
			ird->decode_step++;
		}

		return PAN_KEY_INVALID;
	}
	else
	{
		accum_pulse_width = ird->last_pulse_width + pulse_width;
		ird->last_pulse_width = 0;
		ird->first_half_got = 0;

		/* Process normal key */
		if (ird->attr->normal)
		{
			step = ird->attr->normal[ird->decode_step];
			next_step = ird->attr->normal[ird->decode_step + 1];
			if(step > IR_DATA)
			{
				step++;
			}

			accum_pulse = PULSE_VALUE(ird->attr->pulse[step].fst_half) + PULSE_VALUE(ird->attr->pulse[step].scd_half);
			tolerance_pulse = ird->attr->pulse[step].tolerance;
			same_polarity = !(PULSE_POL(ird->attr->pulse[step].scd_half) ^ (pulse_polarity << 15));
				
			if (ird->attr->type == IR_TYPE_RC5)
			{
				accum_pulse += PULSE_VALUE(ird->attr->pulse[next_step].fst_half);
				tolerance_pulse += PULSE_VALUE(ird->attr->pulse[next_step].fst_half);
			}
			
			if (ird->attr->normal[ird->decode_step] == IR_DATA)
			{
				accum_pulse_1 = PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].fst_half) + PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].scd_half);
				tolerance_pulse_1 = ird->attr->pulse[IR_DATA + 1].tolerance;
				same_polarity_1 = !(PULSE_POL(ird->attr->pulse[IR_DATA + 1].scd_half) ^ (pulse_polarity << 15));
				
				if (ird->attr->type == IR_TYPE_RC5)
				{
					accum_pulse_1 += PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].fst_half);
					tolerance_pulse_1 += PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].fst_half);
				}
				if (((ird->attr->ignore_lastpulse && ird->attr->normal[ird->decode_step + 1] == IR_END) || \
					INRANGE(accum_pulse_width, accum_pulse, tolerance_pulse)) && (same_polarity ? 1 : 0))
				{
					result = IRC_DECODE_DATA0;
				}
				else if (((ird->attr->ignore_lastpulse && ird->attr->normal[ird->decode_step + 1] == IR_END) || \
					INRANGE(accum_pulse_width, accum_pulse_1, tolerance_pulse_1)) && (same_polarity_1 ? 1 : 0))
				{
					result = IRC_DECODE_DATA1;
				}
				else
				{
					result = IRC_DECODE_FAIL;
				}
			}
			else
			{
				if (INRANGE(accum_pulse_width, accum_pulse, tolerance_pulse))
				{
					if (ird->attr->type == IR_TYPE_RC5 && step == IR_SPECIAL)
					{
						result = IRC_DECODE_SUCCEED;
					}
					else if (same_polarity)
					{
						result = IRC_DECODE_SUCCEED;
					}
					else
					{
						result = IRC_DECODE_FAIL;
					}
				}
				else
				{
					result = IRC_DECODE_FAIL;
				}
			}
			
			if (result == IRC_DECODE_SUCCEED || result == IRC_DECODE_DATA0 || \
				result == IRC_DECODE_DATA1)
			{
				ird->decode_step++;
				if (result == IRC_DECODE_DATA0 || result == IRC_DECODE_DATA1)
				{
					//ird->key_code[0]  <<=1;
					if (result == IRC_DECODE_DATA1)
					{
						ird->key_code[ird->key_bit_cnt / 32] |= (unsigned long)0x01L << (ird->key_bit_cnt % 32);
					}
					
					ird->key_bit_cnt++;
				}
				
				if (ird->attr->type == IR_TYPE_RC5 && INRANGE(accum_pulse_width, accum_pulse, ird->attr->pulse[next_step].tolerance))
				{
					ird->first_half_got = 1;
					ird->last_pulse_width = PULSE_VALUE(ird->attr->pulse[next_step].fst_half);
				}

				/* deal with bit/byte reverse if need at last */
				if (ird->attr->normal[ird->decode_step] == IR_END)
				{
					last_key = PAN_KEY_INVALID;
					if (ird->attr->bit_msb_first)
					{
						reverse_code_bit(ird->key_code, ird->key_bit_cnt);
					}

					if (ird->attr->byte_msb_first)
					{
						reverse_code_byte(ird->key_code, ird->key_bit_cnt);
					}

					ird->decode_step = 0;
					ird->key_bit_cnt = 0;
					last_key = ird->last_key_code[0];
					COPY_CODE(ird->last_key_code, ird->key_code);
					CLEAR_CODE(ird->key_code);

					if (last_key != ird->last_key_code[0] || ird->attr->repeat != NULL)
					{
						key_cnt = 0;
					}

					return ird->last_key_code[0];
				}
				return PAN_KEY_INVALID;
			}
			else if (result == IRC_DECODE_FAIL)
			{
				step_temp = ird->decode_step;
				ird->decode_step = 0;
				ird->key_bit_cnt = 0;
				CLEAR_CODE(ird->key_code);
				if (ird->attr->normal[step_temp] != IR_LEADING)
				{
					return PAN_KEY_INVALID;
				}
			}
			else
			{
				ird->decode_step = 0;
				ird->key_bit_cnt = 0;
				CLEAR_CODE(ird->key_code);
				return PAN_KEY_INVALID;
			}
		}

		/* Process repeat key */
		if (ird->attr->repeat_enable && ird->attr->repeat)
		{
			enum ir_waveform step = ird->attr->repeat[ird->decode_step] + 1;	
			if (INRANGE(accum_pulse_width, PULSE_VALUE(ird->attr->pulse[step].fst_half) + PULSE_VALUE(ird->attr->pulse[step].scd_half), ird->attr->pulse[step].tolerance))
			{
				result = IRC_DECODE_SUCCEED;
			}
			else
			{
				result = IRC_DECODE_FAIL;
			}
			
			if (result == IRC_DECODE_SUCCEED)
			{
				ird->decode_step++;
				if (ird->attr->repeat[ird->decode_step] == IR_END)
				{
					ird->decode_step = 0;
					ird->key_bit_cnt = 0;
					return ird->last_key_code[0];
				}
				return PAN_KEY_INVALID;
			}
			else if (result == IRC_DECODE_FAIL)
			{
				ird->decode_step = 0;
				ird->key_bit_cnt = 0;
				CLEAR_CODE(ird->key_code);
				return PAN_KEY_INVALID;
			}
			else
			{
				ird->decode_step = 0;
				ird->key_bit_cnt = 0;
				CLEAR_CODE(ird->key_code);
				return PAN_KEY_INVALID;
			}
		}
		else
		{
			if (accum_pulse_width < ird->attr->pulse_max_width)
			{
				ird->last_pulse_width = accum_pulse_width;
			}
			else
			{
				ird->last_pulse_width = 0;
			}
			
			ird->decode_step = 0;
			ird->key_bit_cnt = 0;
			CLEAR_CODE(ird->key_code);
			return PAN_KEY_INVALID;
		}
	}
}

/*****************************************************************************
 * UINT32 irc_pulse_to_code(UINT32 pulse_width)
 * Description: Translate pulse width to IR code
 *
 *From now on, irc_decode support fellow protocols :  
 *NEC   LAB  50560  KF Logic  SRC  NSE  RC5  RC6_mode1  ----2010.2.11 by ryan.chen
 *Arguments:
 *UINT32 pulse_width	: Input pulse width, in uS
 *UINT32 pulse_polarity	: Input pulse polarity, 
 *					  '1' = high level, '0' = low level
 *
 * Return Value:
 *  INT32				: Key code
 ****************************************************************************/
UINT32 irc_pulse_to_code(UINT32 pulse_width, UINT32 pulse_polarity)
{
	UINT8 i = 0;
	UINT32 key_code = PAN_KEY_INVALID;

	for(i = 0; i< IRP_CNT; i++)
	{
		key_code = irc_decode(irc_decoders[i], pulse_width, pulse_polarity);
		if( (key_code != PAN_KEY_INVALID) && (key_code != 0x00000000L))
		{
			return key_code;
		}
	}
 
	if(i == IRP_CNT)
	{
		return PAN_KEY_INVALID;
	}
}

/*----------------------------------------------------------------------
* Function_Name: IR_GenerateCode
*
* Description: IR controler generate code
*
* Arguments: None
*
* Return Value: None
*----------------------------------------------------------------------*/
void IR_GenerateCode(void)
{
	unsigned long cur_code = 0, pulse_width = 0;
	unsigned char pulse_polarity = 0;
	unsigned char rlc_data = 0;
	unsigned char sub_code[4] = {0};
	unsigned char reg = 0;
	static unsigned i = 0;
	struct irc_decoder *ird;
	unsigned char test_ir = 0;

	for (i = 0; i< IRP_CNT; i++)
	{
		ird = irc_decoders[i];
		ird->first_half_got = 0;
		ird->decode_step = 0;
		ird->key_bit_cnt = 0;
		CLEAR_CODE(ird->key_code);
	}

	pulse_width = 0;
	while (bufferin != bufferout)
	{
		rlc_data = ir_buffer[bufferout];
		//printf("    rptr=%bx, wptr=%bx, data=%bx\n", bufferout, bufferin, rlc_data);

		/* The MSB bit is the status bit, LSB 7 bits is time ticks,
		 * If some one status is too long, mult-bytes maybe used.
		 */
		pulse_width += ((rlc_data & 0x7f) * CLK_CYC_US); /* Pulse width */
		pulse_polarity = ((rlc_data&0x80)>>7);
		bufferout ++;
		if(bufferout == 0xff)
		{
			bufferout = 0;
		}

		//bufferout = (++bufferout & (IR_BUF_LEN - 1)); /* Next data */
		/* Long pulse */
		if((!((rlc_data ^ ir_buffer[bufferout]) & 0x80)) && (bufferout != bufferin))
		{
			continue;
		}

		//printf(" rptr=%bx, wptr=%bx, level=%bd, width=%ld\n",  bufferout, bufferin,((rlc_data&0x80)>>7), pulse_width);
		if((cur_code = irc_pulse_to_code(pulse_width,pulse_polarity)) != PAN_KEY_INVALID)
		{
			sub_code[0] = cur_code & 0xff;
			sub_code[1] = (cur_code >> 8) & 0xff;
			sub_code[2] = (cur_code >> 16) & 0xff;
			sub_code[3] = (cur_code >> 24) & 0xff;
			//g_set_ir_key.ir_key_low0 = 0x8f;
			//g_set_ir_key.ir_key_low1 = 0x70;
			//g_set_ir_key.ir_key_low2 = 0xdf;
			for(i=0;i<8;i++)
			{
				if(cur_code== g_ir_power_key[i])
				{
					g_ir_flag = 1;
					//printf("IR_GenerateCode!\n");
				}
			}
		}
		pulse_width = 0;
	}
}

/*----------------------------------------------------------------------
* Function_Name: IR_ISR
* Description: Receive the remote controller signal from Interrupt Requested.
* Arguments: None
*
* Return Value: None
*----------------------------------------------------------------------*/
void IR_ISR(void)
{
	volatile unsigned char status = 0, num = 0, num1 = 0;

	while (status = (READ_BYTE(IR_REG_ISR) & 3))
	{
		WRITE_BYTE(IR_REG_ISR, status);
		switch (status)
		{
			case 0x02:/* If timeout, generate IR code in HSR */
			case 0x01:/* If FIFO trigger, copy data to buffer */
				do
				{
					num1 = num = READ_BYTE(IR_REG_FIFOCTRL) & 0x7F;
					while (num > 0)
					{
						/* Put RLC to buffer */
						ir_buffer[bufferin++] = READ_BYTE(IR_REG_RLCBYTE);
						if(bufferin == 0xff)
						{
							bufferin = 0;
						}
						//bufferin &= (IR_BUF_LEN - 1);
						num--;
					};
				}
				while (num1 > 0);
				break;

			default:
				break;
		}
		if (status == 0x02)
		{
			//printf(" IR_GenerateCode \n ");
			IR_GenerateCode();//add 20120818
			num = READ_BYTE(IR_REG_FIFOCTRL) & 0x7F;
			while (num > 0)
			{
				READ_BYTE(IR_REG_RLCBYTE);
				num--;
			}
			bufferin = 0;
			bufferout = 0;
		}
	}

	return;
}

/*----------------------------------------------------------------------
* Function_Name: IR_Init
* Description: IR service initial routine
* Arguments:
*
* Return Value:
*----------------------------------------------------------------------*/
void IR_Init(void)
{
	unsigned char reg = 0;

	/* Reset IR */
	reg = READ_BYTE(SYS_REG_RST);
	reg &= ~(1 << IR_BIT);
	WRITE_BYTE(SYS_REG_RST, reg);
	reg |= (1 << IR_BIT);
	WRITE_BYTE(SYS_REG_RST, reg);

	WRITE_BYTE(IR_REG_CFG, 0x00);
	/* Working clock expressions:
	 * (IR_CLK / ((CLK_SEL+1)*4)) = 1 / CLK_CYC_US, IR_CLK = 1.5MHz
	 * => CLK_SEL = (IR_CLK*CLK_CYC_US)/4 - 1
	 */
	//reg = 0x80 | ((15*CLK_CYC_US/40)-1);
	reg = ((15 * CLK_CYC_US / 40) - 1);

	//  reg =  ((15*CLK_CYC_US/40)-1);
	WRITE_BYTE(IR_REG_CFG, reg);

	/* FIFO threshold */
	WRITE_BYTE(IR_REG_FIFOCTRL, 0x8f); //16bytes //0xbf);

	/* Timeout threshold expressions:
	 * ((TIMETHR + 1) * 128 * CLK_CYC_US) = TIMEOUT_US
	 * => TIMETHR = (TIMEOUT_US/(128 * CLK_CYC_US)) - 1
	 */
	reg = TIMEOUT_US / (CLK_CYC_US << 7) - 1;
#ifdef PMU_MCU_M3921
	WRITE_BYTE(IR_REG_TIMETHR, reg);
#else
	reg = TIMEOUT_US / (8 << 7) - 1;
	reg = 22;
	WRITE_BYTE(IR_REG_TIMETHR, reg);
#endif

	/* Noise pulse timeout expressions:
	 * Value = VALUE_NOISETHR / CLK_CYC_US
	 */
	reg = NOISETHR_US / CLK_CYC_US;
#ifdef PMU_MCU_M3921
	WRITE_BYTE(IR_REG_NOISETHR, reg);
#else
	WRITE_BYTE(IR_REG_NOISETHR, 10);
#endif

	/* Ensure no pending interrupt */
	WRITE_BYTE(IR_REG_ISR, 0x03);

	/* Enable IR Interrupt */
	WRITE_BYTE(IR_REG_IER, 0x03);

	//IR_ClearRoundBuf();
	reg = READ_BYTE(SYS_REG_IPR);
	reg |= (1 << IR_BIT);
	WRITE_BYTE(SYS_REG_IPR, reg);

	reg = READ_BYTE(SYS_REG_IER);
	reg |= (1 << IR_BIT);

	WRITE_BYTE(SYS_REG_IER, reg);

	EX0 = 1;
}

INT8 get_ir(void)
{
	if(g_ir_flag == 1)
	{
		return SUCCESS;
	}
	else
	{
		return ERROR;
	}
}
