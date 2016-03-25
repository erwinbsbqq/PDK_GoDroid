/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: ir6311.c
 *
 *  Description: This file implements the Infra Receiver driver 
 *               by using NEC standard IRC algorithm library.
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2003.1.11   Liu Lan     0.1.000  Initial
 *  2.  2003.2.20   Liu Lan     0.1.001  Tick Configure
 *  3.  2003.6.13   Justin Wu   0.1.002  Stability enhance
 *  4.  2003.7.5    Justin Wu   0.2.000  Update to new algorithm
 *  5.  2004.8.11   Justin Wu   0.2.001  Support new RLC IP (M3327/57)
 *  6.  2011.02.12  ryan.chen  0.2.002  Add RC6 decoder 
 ****************************************************************************/

#include "ali_ir_priv.h"
#include <boot_common.h>
#include <ali_ir.h>
#include <common.h>



#define IR_USE_HSR					1

#define IR_RLC_SIZE					256

#ifdef ALI_ARM_STB
#define IOBASE						0x18018100
#else
#define IOBASE						0xb8018100
#endif

#define INFRA_IRCCFG				(IOBASE + 0x00)
#define INFRA_FIFOCTRL				(IOBASE + 0x01)
#define INFRA_TIMETHR				(IOBASE + 0x02)
#define INFRA_NOISETHR				(IOBASE + 0x03)
#define INFRA_IER					(IOBASE + 0x06)
#define INFRA_ISR					(IOBASE + 0x07)
#define INFRA_RLCBYTE				(IOBASE + 0x08)

#define VALUE_CLK_CYC		8		/* Work clock cycle, in uS */
#define VALUE_TOUT			24000	/* Timeout threshold, in uS */
#define VALUE_NOISETHR		80		/* Noise threshold, in uS */


#define READ_INF_BYTE(addr)			(*(volatile unsigned char *)addr)
#define WRITE_INF_BYTE(addr,data)	(*(volatile unsigned char *)addr = data)


struct pan_hw_info			/* Total 16 bytes */
{	
	UINT32	intv_repeat_first;				/* Repeat interval first in mS */    
	UINT32	intv_repeat;					/* Repeat interval in mS */  
	UINT32	intv_release;					/* Release interval in mS */   
};


static UINT16 bufferin = 0;
static UINT16 bufferout = 0;
static UINT8 infra_buffer[IR_RLC_SIZE];
static UINT8 key_state = PAN_KEY_RELEASE;
static UINT16 g_ali_ir_key_count = 0;
static UINT32 last_act_code = PAN_KEY_INVALID;
static UINT8 g_ali_is_ir_repeat = 0;
static UINT8 g_ali_is_ir_pulse = 0;
UINT16 key_cnt = 0;
static UINT32 ms_tick;
static struct pan_hw_info *local_hw_info;
static UINT32 ir_repeat_width = 0;
static unsigned long g_ali_ir_last_act_tick = 0;


UINT32 irc_get_last_act_code(void)
{
	return last_act_code;
}

void irc_set_ir_repeat(UINT8 repeat)
{
	g_ali_is_ir_repeat = repeat;
}

UINT8 irc_get_ir_repeat(void)
{
	return g_ali_is_ir_repeat;
}

UINT8 irc_get_ir_pulse(void)
{
	return g_ali_is_ir_pulse;
}

void irc_set_ir_pulse(UINT8 pulse)
{
	g_ali_is_ir_pulse = pulse;
}



/*****************************************************************************
 * void irc_init()
 * Description: IR controler init
 *
 * Arguments:
 *
 * Return Value:
 ****************************************************************************/
void irc_init(void)
{	
    /* Initialize driver internal variables */
    /* init temp */
    local_hw_info->intv_repeat_first = 650;
    local_hw_info->intv_repeat = 350;
    local_hw_info->intv_release = 150;
    
    ir_repeat_width = local_hw_info->intv_repeat_first;
    bufferin = bufferout = 0;
	/* Reset internal state, precision is 280uS, repeat timeout is 500mS */
	irc_NEC_mode_set(0, 280, 500000);
	

    WRITE_INF_BYTE(INFRA_IRCCFG, 0);
    /* Working clock expressions:
     * (SB_CLK / (32 * CLK_SEL)) = 1 / VALUE_CLK_CYC, SB_CLK = 12MHz
     * => CLK_SEL = (SB_CLK * VALUE_CLK_CYC / 32)
     */
    WRITE_INF_BYTE(INFRA_IRCCFG, 0x80 | ((12 * VALUE_CLK_CYC) >> 5));

    /* FIFO threshold */
    WRITE_INF_BYTE(INFRA_FIFOCTRL, 0xA0);	/* 32 bytes */

    /* Timeout threshold expressions:
     * ((TIMETHR + 1) * 128 * VALUE_CLK_CYC) = VALUE_TOUT
     * => TIMETHR = (VALUE_TOUT / (128 * VALUE_CLK_CYC)) - 1
     */
    WRITE_INF_BYTE(INFRA_TIMETHR, (VALUE_TOUT / (VALUE_CLK_CYC << 7) - 1));

    /* Noise pulse timeout expressions:
     * Value = VALUE_NOISETHR / VALUE_CLK_CYC
     */
    WRITE_INF_BYTE(INFRA_NOISETHR, VALUE_NOISETHR / VALUE_CLK_CYC);

    /* Ensure no pending interrupt */
    WRITE_INF_BYTE(INFRA_ISR, 3);

    /* Enable IRC Interrupt */
    WRITE_INF_BYTE(INFRA_IER, 3);   
}

/*****************************************************************************
 * void irc_close()
 * Description: IR controler stop
 *
 * Arguments:
 *
 * Return Value:
 ****************************************************************************/
void irc_close()
{
    WRITE_INF_BYTE(INFRA_IRCCFG, 0);	/* Disable IRC */	
}

/*****************************************************************************
 * void generate_code()
 * Description: IR controler generate code
 *
 * Arguments:
 *
 * Return Value:
 ****************************************************************************/
static void generate_code(UINT32 tick)
{
	static UINT32 last_tick = 0;
	UINT32 code, pulse_width, last_width, pulse_polarity;
	UINT8  data;
	struct pan_key key;
	unsigned long repeat_tick = 0;
	static unsigned long last_repeat_tick = 0;	
	unsigned long last_repeat_width;	
	

	memset(&key, 0x00, sizeof(key));
	irc_pulse_to_code((tick - last_tick) * 1000, 1);	/* The time in idle */

	last_tick = tick;

	pulse_width = 0;
	while (bufferin != bufferout)
	{		
		data = infra_buffer[bufferout];

		//printf("br=%d, bw=%d, data=%02x\n", bufferout, bufferin, data);
		/* The MSB bit is the status bit, LSB 7 bits is time ticks,
		 * If some one status is too long, mult-bytes maybe used.
		 */
		pulse_width += ((data & 0x7f) * VALUE_CLK_CYC);	/* Pulse width */
		pulse_polarity = (data & 0x80) ? 1 : 0;
		bufferout = ((bufferout + 1) & (IR_RLC_SIZE - 1));/* Next data */
		/* Long pulse */
		if ((!((data ^ infra_buffer[bufferout]) & 0x80)) && (bufferout != bufferin))
		{
			continue;
		}
		//printf("\t\t\tlevel=%d, width=%d\n", ((data&0x80)>>7), pulse_width);
		if ((code = irc_pulse_to_code(pulse_width, pulse_polarity)) != PAN_KEY_INVALID)
		{
			ms_tick = 0;
			last_width = tick - g_ali_ir_last_act_tick;
			//printf("\t\t\tlast_width=%d\n", last_width);
			if(key_cnt == 0)		/* Receive a new key */
			{
				last_act_code = PAN_KEY_INVALID;
			}
			else if(key_cnt == 1)	/* Receive a continous key */
			{
				ir_repeat_width = local_hw_info->intv_repeat_first;
			}
			else
			{
				ir_repeat_width = local_hw_info->intv_repeat;
			}
			
		    if ((last_act_code != code) || (last_width > ir_repeat_width) || (key_cnt >= 1))
		    {
		    	key_cnt++;
		    	last_act_code = code;
				g_ali_ir_last_act_tick = tick;

		    	if (1 == key_cnt)
		    	{
		    		last_repeat_tick = get_ticks();	
		    		
					key.type = PAN_KEY_TYPE_REMOTE;
					key.state = PAN_KEY_PRESSED;
					key.count = g_ali_ir_key_count;
					key.code = code;
					if(1 == key_cnt)
					{
						key_state = PAN_KEY_PRESSED;
						pan_buff_queue_tail(&key);
					}					

					printf("press code = 0x%08x, state = %d\n", 
						key.code, key.state);
				}
				else if (key_cnt > 1)
				{			
					key_state = PAN_KEY_PRESSED;	
					
					repeat_tick = get_ticks();					
					last_repeat_width = (unsigned long)(((long)repeat_tick - (long)last_repeat_tick));							
					//printf("/t/t/tlast_repeat_width = %d\n", (unsigned int)last_repeat_width);
					if (last_repeat_width > ir_repeat_width)
					{		
						g_ali_ir_key_count++;
						last_repeat_tick = repeat_tick;			
						
						key.type = PAN_KEY_TYPE_REMOTE;
						key.state = PAN_KEY_REPEAT;
						key.count = g_ali_ir_key_count;
						key.code = code;
						if(pan_buff_get_repeat())
						{
							pan_buff_queue_tail(&key);
						}				
						
						printf("repeat code = 0x%08x, state = %d, , tick = %d\n", 
							key.code, key.state, (unsigned int)last_repeat_width);
					}					
				}
			}			
		}
		pulse_width = 0;
	}
    return;
}

/*****************************************************************************
 * void irc_lsr(UINT32 param)
 * Description: IR controler interrupt
 *
 * Arguments:
 *     UINT32 param
 *
 * Return Value:
 ****************************************************************************/
void irc_lsr(void)
{
    volatile unsigned char status, num, num1;
    unsigned long tick;
	unsigned long last_width;
	struct pan_key key;
	static unsigned long last_act_tick = 0;


    while (status = (READ_INF_BYTE(INFRA_ISR) & 3))
    {
    	irc_set_ir_pulse(1);
		WRITE_INF_BYTE(INFRA_ISR, status);

		//printf("[ %s %d ], status 0x%x\n", __FUNCTION__, __LINE__, status);

		switch (status)
		{
			case 0x01:		/* If FIFO trigger, copy data to buffer */
			case 0x02:		/* If timeout, generate IR code in HSR */
			case 0x03:		/* FIFO trigger and timeout */
				do
				{
					num1 = num = READ_INF_BYTE(INFRA_FIFOCTRL) & 0x7F;
					while (num > 0)
					{
						/* Put RLC to buffer */
						infra_buffer[bufferin++] = READ_INF_BYTE(INFRA_RLCBYTE);
						bufferin &= (IR_RLC_SIZE - 1);
						num--;
					};
				} while (num1 > 0);
				break;
			default:
				break;
		}
		if (0 != (status & 0x02))
		{			
			last_act_tick = get_ticks();
			generate_code(get_ticks());
		}
	}

	if ((UINT32)(get_ticks() - last_act_tick) >= local_hw_info->intv_release)
	{
		irc_set_ir_pulse(0);
		irc_set_ir_repeat(0);
	}
	
	
	if((PAN_KEY_PRESSED == key_state) && (key_cnt >= 1))
	{		
		tick = get_ticks();
		last_width = (unsigned long)(((long)tick - (long)g_ali_ir_last_act_tick));								
		//printf("/t/t/tlast_width = %d\n", (unsigned int)last_width);
		if (last_width > local_hw_info->intv_release)				
		{					
			key_cnt = 0;	
			key_state = PAN_KEY_RELEASE;	
			g_ali_ir_key_count = 0;
			irc_set_ir_repeat(0);
						
			key.type = PAN_KEY_TYPE_REMOTE;
			key.state = PAN_KEY_RELEASE;
			key.count = g_ali_ir_key_count;
			key.code = last_act_code;			
			pan_buff_queue_tail(&key);							
			
			printf("release code = 0x%08x, state = %d, tick = %d\n", 
				key.code, key.state, (unsigned int)(get_ticks() - g_ali_ir_last_act_tick));			
		}					
	}

	return;
}


