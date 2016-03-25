#include "ali_pmu.h"
//==================================================//

PMU_EXIT_FLAG get_powerup_status(void);
//==================================================//
static pmu_delay(void)
{
	volatile unsigned long i = 0;
	for(i=0;i<0xfff0;i++);
}

static void output_char(unsigned char c)
{
	pmu_delay();
	WRITE_BYTE(UART_OUTPUT_REG, c);
}

static void output_string(unsigned char *string)
{
	while(*string)
	{
		output_char(*string++);
	}
	output_char(0x0d);
	output_char(0x0a);
}

static char ascii[] = "0123456789ABCDEF";
static void dump_reg(unsigned long addr, unsigned long len)
{
	unsigned long i = 0, j = 0;
	unsigned char index = 0;
	for(i=0; i<len; i++)
	{
		if(i%16 == 0)
		{
			output_char(0x0d);
			output_char(0x0a);
			for(j=0; j<8; j++)
			{
				output_char(ascii[((addr+i)>>(4*(7-j)))&0xF]);
			}
			output_char(':');
		}
		index = *(unsigned char *)(addr+i);
		output_char(ascii[(index>>4)&0xF]);
		output_char(ascii[index&0xF]);
		output_char(' ');
	}
	output_char(0x0d);
	output_char(0x0a);
	return;
}

static inline void pmu_en(void)
{
	WRITE_BYTE(PMU_CFG, READ_BYTE(PMU_CFG) | (0x1 << 7)) ;
}

static inline void pmu_clear(void)
{
	WRITE_BYTE(PMU_CFG, READ_BYTE(PMU_CFG) & (~(0x1 << 7))) ;
}

static inline void pan_en(unsigned int time)
{
	WRITE_BYTE(PRS_KEY_CFG , time & 0xff);
	WRITE_BYTE(PRS_KEY_CFG + 1 , (time >> 0x8) & 0xff);
	SET_BIT(PRS_KEY_CFG + 3, 7);
}

static inline void ir2_en(void)
{
	WRITE_BYTE(PMU_CFG, READ_BYTE(PMU_CFG) | 0x1 << 6) ;
}

/********************************************
*Function: Interface to enable PMU
*Notice:   If call this function,PMU will assert power-off signal to CPU
*          and the whole chip will enter deep-sleep model
*
*********************************************/
void pmu_m36_en(void)
{
	ir2_en();
	pmu_en();
	while(1);
}

static inline void set_pmu_clk(unsigned char clk)
{
	WRITE_BYTE(PMU_CFG, (READ_BYTE(PMU_CFG) & 0xc0) | clk);
}

static inline void set_noisthr(unsigned char clk)
{
	WRITE_BYTE(NOISETHR, (clk & 0xff));
}

static inline void set_timerthr(unsigned char clk)
{
	WRITE_BYTE(TIMETHR, (clk & 0xff));
}

static inline int adjust_rc_osc(void)
{
	unsigned long delay_cnt = 0x8000;
	int ret_v = -1;

	SET_BIT(IR_RC_ADJUST1, 7);
	while(delay_cnt--)
	{
		if( READ_BIT(IR_RC_ADJUST1, 5) )
		{
			ret_v = 0;
			break ;
		}
	}

	return ret_v;
}

static inline unsigned int get_rc_osc(void)
{
	return READ_BYTE(IR_RC_ADJUST) | ((READ_BYTE(IR_RC_ADJUST1) & 0x1f)<<0x8) ;
}

static PMU_IR  pmu_ir_rc5_x =
{
	//.ir_thr={60,140,170,280,0,0,0,0},//unsigned short ir_thr[8]        108  216
	.ir_thr={45,105,127,210,0,0,0,0}, //unsigned short ir_thr[8]        108  216
	.ir_thr_cnt = 4,   // ir_thr_cnt
	.ir_decode_cnt = 21,  //ir_decode_cnt
	.ir_pol={0xaa,0xaa,0x0a,0x00,0,0,0,0,0,0,0,0}    ,  // unsigned char  ir_pol[12]
	.ir_pol_cnt = 21,//ir_pol_cnt
	.ir_donca={0x1C,0,0,0,0,0,0,0,0,0,0,0}   , //ir_donca[12]
	.ir_donca_cnt=3,// ir_donca_cnt ;
	.ir_thr_sel ={0x0,0x0,0x22,0x2,0,0,0,0x20,0,0x2,0,0,0,0,0,0,0,0,0,}, // ir_thr_sel[48]  
	.ir_thr_sel_cnt =21,  // ir_thr_sel_cnt ;
	//12000000, //ir_decoder default clk
	//0x03  , //clk_sel
	//30,//timer_thr 
	//13,// noise_thr
	.flg= 0,  // flg
	.type =IR_TYPE_RC5_X   , // type
};         

static PMU_IR  pmu_ir_nec =
{
	//.ir_thr ={30,140,380,840,0,0,0,0},//unsigned shortr_thr[8]  108  216
	.ir_thr ={22,105,285,630,0,0,0,0}, 
	.ir_thr_cnt = 4,   // ir_thr_cnt
	.ir_decode_cnt = 66,  //ir_decode_cnt
	.ir_pol = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0x2,0,0,0},  //   ir_pol[12]
	.ir_pol_cnt = 66,//ir_pol_cnt
	.ir_donca={0,0,0,0,0,0,0,0,0,0,0,0}   , //ir_donca[12]
	.ir_donca_cnt =0,// ir_donca_cnt ;
	.ir_thr_sel ={0x23,0x0,0x10,0x10,0,0,0,0,0,0x10,0x10,0,0x10,0x10,0x10,0x10,0x10,0,0x10,0x10,0x10,0,0,0,0,0x10,0,0,0,0x10,0x10,0x10,0x10}, //unsigned char  ir_thr_sel[48] ,33 
	.ir_thr_sel_cnt =66,  // ir_thr_sel_cnt ;
	//12000000, //ir_decoder default clk
	//0x03, //clk_sel
	//30,//timer_thr 
	//13, // noise_thr
	.flg  = 0,  //flg
	.type = IR_TYPE_NEC ,//type
};  

static  void set_ir_reg(unsigned char reg ,unsigned char * src, unsigned char len)
{
	unsigned char i = 0;

	for(i=0 ; i<len ; ++i )
	{
		WRITE_BYTE(reg+i,(*(src+i)) & 0xff);
	}
}

void nec_trans_pmu_ir_thr_sel(unsigned long power_key )
{
	unsigned char i = 0;
	unsigned long j = 0x80000000;

	if(power_key != 0x60df708f)
	{
		for (i = 1; i < 33; i++)
		{
			if(power_key & j)
			{
				pmu_ir_nec.ir_thr_sel[i] = 0x10;
			}
			else
			{
				pmu_ir_nec.ir_thr_sel[i] = 0;
			}
			j = j >> 1;
		}
		set_ir_reg(IR1_THR_SEL, (unsigned char *)&(pmu_ir_nec.ir_thr_sel[0]), sizeof(pmu_ir_nec.ir_thr_sel) / sizeof(unsigned char)); // set THR_SEL
	}
}

static int set_ir1_format(PMU_IR *key_format_p)
{
	set_ir_reg(IR1_THR0, (unsigned char *)&(key_format_p->ir_thr[0]), \
		sizeof(key_format_p->ir_thr) / sizeof(unsigned char));//set IR_THR0
	set_ir_reg(IR1_THR_NUM, (unsigned char *)&(key_format_p->ir_thr_cnt), 1);// set IR_THR0_NUM
	set_ir_reg(IR1_DECODE_NUM, (unsigned char *)&(key_format_p->ir_decode_cnt), 1) ; // set decode_num
	set_ir_reg(IR1_POL , (unsigned char *)&(key_format_p->ir_pol[0]), \
		sizeof(key_format_p->ir_pol) / sizeof(unsigned char)); // set IR_POL
	set_ir_reg(IR1_DONCA, (unsigned char *)&(key_format_p->ir_donca[0]), \
		sizeof(key_format_p->ir_donca) / sizeof(unsigned char));//set IR_DONCA
	set_ir_reg(IR1_THR_SEL , (unsigned char *)&(key_format_p->ir_thr_sel[0]), \
		sizeof(key_format_p->ir_thr_sel) / sizeof(unsigned char));// set THR_SEL

	return 0;
}

static int set_ir2_format(PMU_IR * key_format_p)
{
	set_ir_reg(IR2_THR0,(unsigned char *)&(key_format_p->ir_thr[0]),\
		sizeof(key_format_p->ir_thr)/sizeof(unsigned char));// set IR_THR0
	set_ir_reg(IR2_THR_NUM,(unsigned char *)&(key_format_p->ir_thr_cnt),1) ;// set IR_THR0_NUM
	set_ir_reg(IR2_DECODE_NUM,(unsigned char *)&(key_format_p->ir_decode_cnt),1) ;// set decode_num
	set_ir_reg(IR2_POL ,(unsigned char *)&(key_format_p->ir_pol[0]), \
		sizeof(key_format_p->ir_pol)/sizeof(unsigned char));// set IR_POL
	set_ir_reg(IR2_DONCA,(unsigned char *)&(key_format_p->ir_donca[0]),\
		sizeof(key_format_p->ir_donca)/sizeof(unsigned char));// set IR_DONCA
	set_ir_reg(IR2_THR_SEL ,(unsigned char *)&(key_format_p->ir_thr_sel[0]),\
		sizeof(key_format_p->ir_thr_sel)/sizeof(unsigned char));// set THR_SEL
    
	return 0;
}

static void reconfig_key_clk(PMU_IR * pmu )
{
	set_pmu_clk(0xf);
	set_noisthr(0x9);//set_noisthr(0xd);
	set_timerthr(0x16);//set_timerthr(0x1e);
	set_ir1_format(pmu);
	set_ir2_format(&pmu_ir_rc5_x);
	//pan_en(0x8000);
	//shorten the PMU panel key response time
	pan_en(0x4000);
}

static unsigned char Now_Is_Leap_Year(unsigned int year)
{
	if((year%4==0) && (year%100!=0))
	{
		return 1;  //is leap
	}
	else if(year%400==0)
	{
		return 1; //is leap
	}
	else 
	{
		return 0; // not leap
	}
}

static void time_accumulate(struct rtc_base_time *base_time)
{
	rtc_sec += 1;
	if(rtc_sec > 59)
	{
		rtc_min += 1;
		rtc_sec = 0;
	}

	if(rtc_min > 59)
	{
		rtc_hour += 1;
		rtc_min = 0;
	}

	if(rtc_hour > 23)
	{
		rtc_date += 1;
		rtc_hour = 0;
	}

	if(rtc_month == 1 || rtc_month == 3 || rtc_month == 5 || rtc_month == 7 || rtc_month == 8 || rtc_month == 10 || rtc_month == 12)
	{
		if(rtc_date > 31)
		{
			rtc_month += 1;
			rtc_date = 1;
		}
	}
	else
	{
		if(rtc_month == 2)
		{ //month 2
			if(Now_Is_Leap_Year(rtc_year)==0)
			{ //not a leap year
				if(rtc_date > 28)
				{
					rtc_month += 1;
					rtc_date = 1;
				}
			}
			else
			{ //leap year
				if(rtc_date > 29)
				{
					rtc_month += 1;
					rtc_date = 1;
				} 
			}
		}
		else
		{
			if(rtc_date > 30)
			{
				rtc_month += 1;
				rtc_date = 1;
			}
		}
	}

	if(rtc_month > 12)
	{ 
		// a new year begins         
		rtc_year += 1; 
		rtc_month = 1;
	}
}

int ali_pmu_get_time(unsigned long *time_cur)
{
	unsigned char c_year_h=1;
	unsigned char c_year_l=1;
	unsigned int  c_year=1;
	unsigned char c_month=1;
	unsigned char c_day=1;
	unsigned char c_hour=1 ;
	unsigned char c_min=1;
	unsigned char c_sec=1;
	unsigned int c_time = 0;

	c_year_h = READ_BYTE(MAILBOX_GET_YEAR_H);
	c_year_l = READ_BYTE(MAILBOX_GET_YEAR_L);
	c_month = READ_BYTE(MAILBOX_GET_MONTH);
	c_day = READ_BYTE(MAILBOX_GET_DAY);
	c_hour = READ_BYTE(MAILBOX_GET_HOUR);
	c_min = READ_BYTE(MAILBOX_GET_MIN);
	c_sec = READ_BYTE(MAILBOX_GET_SEC);
	c_year  = (c_year_h*100)+c_year_l;
	c_time = (c_sec ) | (c_min << 6 ) | (c_hour << 12 ) | (c_day << 17 ) | ((c_month & 0xF) << 22 ) | (c_year_l << 26);

	if(c_month > 12 || c_day > 31 || c_min > 60 || c_sec > 60)
	{
		*time_cur = 0;
		return 1;
	}
	else
	{
		*time_cur = c_time;
	}

	return 0;
}
EXPORT_SYMBOL(ali_pmu_get_time);

void main_cpu_read_time_init(void)
{
	g_year_h =  READ_BYTE(MAILBOX_GET_YEAR_H);
	g_year_l =  READ_BYTE(MAILBOX_GET_YEAR_L);
	g_month  =  READ_BYTE(MAILBOX_GET_MONTH);
	g_day    =  READ_BYTE(MAILBOX_GET_DAY);
	g_hour   =  READ_BYTE(MAILBOX_GET_HOUR);
	g_min    =  READ_BYTE(MAILBOX_GET_MIN);
	g_sec    =  READ_BYTE(MAILBOX_GET_SEC);
	g_year   =  (g_year_h*100)+g_year_l;
	printk("============>MCU Current Time:[year: %d], [month: %d], [day: %d], [hour: %d], [min: %d], [sec: %d]\n\n\n", \
		g_year, g_month, g_day, g_hour, g_min, g_sec);
}

void enter_standby_reset_mcu(void)
{
#ifdef CONFIG_ARM
	__REG32ALI(PMU_IP_RESET_REG) &= ~(1<<17);
#else
	__REG32ALI(PMU_IP_RESET_REG) |= (1<<25);
#endif
}

void set_pmu_clksel(unsigned char clksel)
{
	PMU_WRITE_BYTE(PMU_CFG_SET, (PMU_READ_BYTE(PMU_CFG_SET) & 0xc0) | clksel);
}

void pan_pmu_power_key_init(unsigned int type1,unsigned int type2 )
{
	unsigned char clk_sel = 0x5;
	unsigned char  reg;

	PMU_WRITE_BYTE(PMU_CFG_SET, PMU_READ_BYTE(PMU_CFG_SET) & (~(0x1<<7))) ; //key  pmu_disable
	PMU_PRINTF("%s:%d\n",__func__,__LINE__);
	set_pmu_clksel(clk_sel);/* clk_sel =5*/

	/*about 0.2s==>>>0x3e80 (UINT8)(4000*4/256)*/
	PMU_WRITE_BYTE(PMU_PRS_KEY_CFG, 0x24);  /*about 0.1s==>>>0x1f40 (UINT8)(4000*2/256)*/
	PMU_WRITE_BYTE(PMU_PRS_KEY_CFG+1,0xf4);
	PMU_WRITE_BYTE(PMU_PRS_KEY_CFG+2,0x00);
	PMU_WRITE_BYTE(PMU_PRS_KEY_CFG+3,PMU_READ_BYTE(PMU_PRS_KEY_CFG+3)&0xf0);
	PMU_PRINTF("%s:%d\n",__func__,__LINE__);
	/*
	*  KEY_DISABLE_VAL=n us*1.5/(4*(5+1))
	*/

	if(type2)
	{
		PMU_WRITE_BYTE(KEY_DISABLE_VAL_LOW0,0x6a);
		PMU_WRITE_BYTE(KEY_DISABLE_VAL_LOW1,0x18); //0.5 second 
		PMU_WRITE_BYTE(KEY_DISABLE_VAL_LOW2,0x00);
		PMU_WRITE_BYTE(KEY_DISABLE_EN,0x80);
		// printk("===>>>step3\n");
		reg = PMU_READ_BYTE(MCU_SYS_IPR);//MCU_Polar
		// reg|=(1<<3)|(1<<6);  /*KEY_INT_STANDBY and KEY_INT_NORM*/
		reg|=(1<<3);
		PMU_WRITE_BYTE(MCU_SYS_IPR, reg);
		reg = PMU_READ_BYTE(MCU_SYS_IER); //MCU_INT_EN
		//  reg|=(1<<3)|(1<<6);  /*KEY_INT_STANDBY and KEY_INT_NORM */
		reg|=(1<<3);
		PMU_WRITE_BYTE(MCU_SYS_IER, reg);
		__REG8ALI(PMU_PRS_KEY_CFG+3) |=(0x1<<7);
		PMU_PRINTF("%s:%d\n",__func__,__LINE__);
	}
	else
	{
		PMU_WRITE_BYTE(PMU_PRS_KEY_CFG+3, PMU_READ_BYTE(PMU_PRS_KEY_CFG+3) & (~(0x1<<7))) ; //key  pmu_disable
	}
}

void Clear_PMU_SRAM(void)
{
	volatile unsigned index=0;

	for(index=0; index<PMU_SRAM_SIZE; index++);
	{
		WRITE_BYTE((PMU_BASE + index), 0);
	}
}

int pmu_hw_init(void)
{
	int ret = 0;
	u8 * mcu_src = NULL;
	int buffer = 0;

	if((chip_id == ALI_C3701) && (rev_id == IC_REV_0))
	{
		pmu_clear();
		if( 0 != (ret =adjust_rc_osc()))
		{
			return -1 ;
		}
		
		reconfig_key_clk(&pmu_ir_nec);
	}
	else if(chip_id == ALI_C3701 && rev_id > IC_REV_0)
	{      
		main_cpu_read_time_init();
		WRITE_DWORD(PMU_IP_RESET_REG, READ_DWORD(PMU_IP_RESET_REG) | (1<<25));
		WRITE_DWORD(PMU_RAM_SWITCH_REG, READ_DWORD(PMU_RAM_SWITCH_REG) & (~(1<<0)));
		mcu_src = (u8*)g_ali_pmu_bin;
		buffer = PMU_SRAM_BASE_ADDR;
		memcpy((u8 *)buffer, mcu_src, sizeof(g_ali_pmu_bin));
		pan_pmu_power_key_init(0, 1);
	}
	else if((ALI_C3921 == chip_id) || (ALI_S3503 == chip_id) || (ALI_S3501 == chip_id) || (ALI_S3821 == chip_id))
	{
		main_cpu_read_time_init();
		Clear_PMU_SRAM();
		enter_standby_reset_mcu();
	}
   
	return 0;
}

/*
	when STB system enter standby,call pmu_mcu_enter_stby_timer_set_value
	function to set pmu time
*/
void pmu_mcu_enter_stby_timer_set_value(struct rtc_time_pmu *base_time)
{
	WRITE_BYTE(MAILBOX_SET_SEC, base_time->sec);
	WRITE_BYTE(MAILBOX_SET_MIN, base_time->min);
	WRITE_BYTE(MAILBOX_SET_HOUR, base_time->hour);
	WRITE_BYTE(MAILBOX_SET_DAY,base_time->date);
	WRITE_BYTE(MAILBOX_SET_MONTH, base_time->month);
	WRITE_BYTE(MAILBOX_SET_YEAR_L, (unsigned char)(base_time->year%100));
	WRITE_BYTE(MAILBOX_SET_YEAR_H, (unsigned char)(base_time->year/100));

	PMU_PRINTF("\n\n\n========>Received current time is [year_h:%d], [year_l:%d], [mon:%d], [day:%d], [hour:%d], [min:%d], [sec:%d]", READ_BYTE(MAILBOX_SET_YEAR_H), READ_BYTE(MAILBOX_SET_YEAR_L), \
		READ_BYTE(MAILBOX_SET_MONTH), READ_BYTE(MAILBOX_SET_DAY), READ_BYTE(MAILBOX_SET_HOUR), READ_BYTE(MAILBOX_SET_MIN), READ_BYTE(MAILBOX_SET_SEC));
}

/*
When STB system work in norm mode,call rtc_s3701_set_value funciton to set current time
*/
void rtc_set_value(struct rtc_time_pmu* base_time, enum SB_TIMER timer_flag)
{   
	spin_lock(&rtc_lock);   
	if((chip_id == ALI_S3501) || ((chip_id == ALI_C3701) && (rev_id > IC_REV_0)))
	{
		rtc_upper = 0xFFFFFFFFFF;
		rtc_onesec = 1000000 * SB_CLK_RTC / RTC_DIV[RTC_DIV_32];
		rtc_onesec = rtc_upper - rtc_onesec; 
		WRITE_DWORD(WR_RTC_L32, (rtc_onesec & 0xFFFFFFFF)); 
		WRITE_BYTE(WR_RTC_H8, (rtc_onesec >> 32)); 
		WRITE_BYTE(RTC_CTL_REG, 0x00);
		//RTC starts to count with 32 divisor clock frequency, and enable interrupt
		//interrupt (enable) and start countings
		WRITE_BYTE(RTC_CTL_REG, (READ_BYTE(RTC_CTL_REG) | 0x14));

		base_time_0.rtc_sec = base_time->sec;
		base_time_0.rtc_min = base_time->min;
		base_time_0.rtc_hour = base_time->hour;
		base_time_0.rtc_date = base_time->date;
		base_time_0.rtc_month = base_time->month;
		base_time_0.rtc_year = base_time->year;
	}
	else if (chip_id == ALI_C3701 && rev_id == IC_REV_0)
	{
		unsigned long rtc_time_set=(1<<31)|(base_time->year<<29)|(base_time->month<<25)|\
		(base_time->date<<20)|(base_time->day<<17)|\
		(base_time->hour<<12)|(base_time->min<<6)|(base_time->sec);	
		WRITE_DWORD(WR_RTC, rtc_time_set);
	}
	else if((ALI_C3921 == chip_id) || (ALI_S3503 == chip_id) || (ALI_S3821 == chip_id))
	{
		//printk("/********%s, %d, RTC_BASE=0x%x", __FUNCTION__, __LINE__, RTC_BASE);
		rtc_upper = 0xFFFFFFFF;
		rtc_onesec = 1000000 * SB_CLK_RTC / RTC_DIV[RTC_DIV_32];
		rtc_onesec = rtc_upper - rtc_onesec;
		WRITE_DWORD(SB_TIMER3_CNT, (rtc_onesec & 0xFFFFFFFF));
		WRITE_BYTE(SB_TIMER3_CTL_REG, 0x00);
		//RTC starts to count with 32 divisor clock frequency, and enable interrupt
		//interrupt (enable) and start countings
		WRITE_BYTE(SB_TIMER3_CTL_REG, (READ_BYTE(SB_TIMER3_CTL_REG) | 0x14));

		rtc_sec = base_time->sec;
		rtc_min = base_time->min;
		rtc_hour = base_time->hour;
		rtc_date = base_time->date;
		rtc_month = base_time->month;
		rtc_year = base_time->year;

		printk("\n\n\n============>APP io-ctrl call [Function: %s, Line %d] set current time:[year: %d], [month: %d], [day: %d], [hour: %d], [min: %d], [sec: %d]\n\n\n", \
			__FUNCTION__, __LINE__, rtc_year, rtc_month, rtc_date, rtc_hour, rtc_min, rtc_sec);
	}
	
	spin_unlock(&rtc_lock); 
}

void rtc_init(void)
{   
	/*C3701 timer equal to C3503 timer2, timer3*/
	if((chip_id == ALI_C3701) && (rev_id >IC_REV_0))//C3701 has only one SB_RTC
	{
		rtc_upper = 0xFFFFFFFFFF;
		rtc_onesec = 1000000 * SB_CLK_RTC / RTC_DIV[RTC_DIV_32];
		rtc_onesec = rtc_upper - rtc_onesec; 
		WRITE_DWORD(WR_RTC_L32, rtc_onesec & 0xFFFFFFFF); 
		WRITE_BYTE(WR_RTC_H8, rtc_onesec >> 32); 
		WRITE_BYTE(RTC_CTL_REG, 0x00);
		//RTC starts to count with 32 divisor clock frequency, and enable interrupt
		//Time0 interrupt (enable) and start countings
		WRITE_BYTE(RTC_CTL_REG, (READ_BYTE(RTC_CTL_REG) | 0x14));
	}
	else if((ALI_C3921 == chip_id) || (ALI_S3503 == chip_id) || (ALI_S3821 == chip_id))
	{
		rtc_upper = 0xFFFFFFFF;
		rtc_onesec = 1000000 * SB_CLK_RTC / RTC_DIV[RTC_DIV_32];
		rtc_onesec = rtc_upper - rtc_onesec; 
		WRITE_DWORD(SB_TIMER3_CNT, rtc_onesec & 0xFFFFFFFF); 
		WRITE_BYTE(SB_TIMER3_CTL_REG, 0x00);

		WRITE_BYTE(SB_TIMER0_CTL_REG, READ_BYTE(SB_TIMER0_CTL_REG) & (~((0x1 << 2) | (0x1 <<4))));
		WRITE_BYTE(SB_TIMER0_CTL_REG, READ_BYTE(SB_TIMER0_CTL_REG) | (1<<3));
		WRITE_BYTE(SB_TIMER1_CTL_REG, READ_BYTE(SB_TIMER1_CTL_REG) & (~((0x1 << 2) | (0x1 <<4))));
		WRITE_BYTE(SB_TIMER1_CTL_REG, READ_BYTE(SB_TIMER1_CTL_REG) | (1<<3));
		WRITE_BYTE(SB_TIMER2_CTL_REG, READ_BYTE(SB_TIMER2_CTL_REG) & (~((0x1 << 2) | (0x1 <<4))));
		WRITE_BYTE(SB_TIMER2_CTL_REG, READ_BYTE(SB_TIMER2_CTL_REG) | (1<<3));
		WRITE_BYTE(SB_TIMER3_CTL_REG, READ_BYTE(SB_TIMER3_CTL_REG) & (~((0x1 << 2) | (0x1 <<4))));
		WRITE_BYTE(SB_TIMER3_CTL_REG, READ_BYTE(SB_TIMER3_CTL_REG) | (1<<3));

		/*Masked by tony because linux kernel will use timer4 as the system timer*/
		if(ALI_C3921 != chip_id)
		{
			WRITE_BYTE(SB_TIMER4_CTL_REG, READ_BYTE(SB_TIMER4_CTL_REG) & (~((0x1 << 2) | (0x1 <<4))));
			WRITE_BYTE(SB_TIMER4_CTL_REG, READ_BYTE(SB_TIMER4_CTL_REG) | (1<<3));
		}
		WRITE_BYTE(SB_TIMER5_CTL_REG, READ_BYTE(SB_TIMER5_CTL_REG) & (~((0x1 << 2) | (0x1 <<4))));
		WRITE_BYTE(SB_TIMER5_CTL_REG, READ_BYTE(SB_TIMER5_CTL_REG) | (1<<3));
		WRITE_BYTE(SB_TIMER6_CTL_REG, READ_BYTE(SB_TIMER6_CTL_REG) & (~((0x1 << 2) | (0x1 <<4))));
		WRITE_BYTE(SB_TIMER6_CTL_REG, READ_BYTE(SB_TIMER6_CTL_REG) | (1<<3));
		WRITE_BYTE(SB_TIMER7_CTL_REG, READ_BYTE(SB_TIMER7_CTL_REG) & (~((0x1 << 2) | (0x1 <<4))));
		WRITE_BYTE(SB_TIMER7_CTL_REG, READ_BYTE(SB_TIMER7_CTL_REG) | (1<<3));
	}
}

void rtc_time_read_value(enum SB_TIMER timer_flag, struct rtc_time_pmu *g_rtc_read)
{
	unsigned long rtc_cur_time = 0;

	spin_lock(&rtc_lock);
	if((chip_id == ALI_C3701 && rev_id > IC_REV_0) || (chip_id == ALI_S3501) ||\
		(chip_id == ALI_C3921) || (chip_id == ALI_S3503) || (chip_id == ALI_S3821))
	{
		g_rtc_read->year = rtc_year;
		g_rtc_read->month = rtc_month;
		g_rtc_read->date = rtc_date;
		g_rtc_read->hour = rtc_hour;
		g_rtc_read->min = rtc_min;
		g_rtc_read->sec = rtc_sec;
		PMU_PRINTF("\n\n\n========>[Function %s, Line %d] : year = %d, month = %d, date = %d, hour = %d, min = %d, sec = %d\n", __FUNCTION__, __LINE__, \
			g_rtc_read->year, g_rtc_read->month, g_rtc_read->date, g_rtc_read->hour, g_rtc_read->min, g_rtc_read->sec);
	}
	else if(chip_id == ALI_C3701 && rev_id == IC_REV_0)
	{
		rtc_cur_time = READ_DWORD(RD_RTC);
		g_rtc_read->month=(rtc_cur_time&CUR_MONTH)>>25;
		g_rtc_read->date=(rtc_cur_time&CUR_DATE)>>20;
		g_rtc_read->day=(rtc_cur_time&CUR_DAY)>>17;
		g_rtc_read->hour=(rtc_cur_time&CUR_HOUR)>>12;
		g_rtc_read->min=(rtc_cur_time&CUR_MIN)>>6;
		g_rtc_read->sec=(rtc_cur_time&CUR_SEC);    
	}

	spin_unlock(&rtc_lock);
}

/*
when STB system enter normer mode from standby mode(standby mode->normer mode),call rtc_time_init_value function to set
STB system time
*/
struct rtc_time_pmu rtc_time_init_value(void)  //from standby mode=>norm mode   
{
	struct rtc_time_pmu base_time;

	memset(&base_time,0,sizeof(struct rtc_time_pmu));
	if(((chip_id == ALI_C3701) && (rev_id > IC_REV_0)) || (chip_id == ALI_S3503) || (chip_id == ALI_S3501) || \
		(chip_id == ALI_C3921) || (chip_id == ALI_S3821))
	{
		base_time.year  = g_year;
		base_time.month = g_month;
		base_time.date  = g_day;
		base_time.day   = 0;  //poe add
		base_time.hour  = g_hour;
		base_time.min   = g_min;
		base_time.sec   = g_sec;
	}

	return base_time;
}

unsigned char  rtc_s3701_read_stdby_status(void)
{
	unsigned char rd_sts=0;

	rd_sts= READ_BYTE(RD_EXIT_STANDBY);
	return rd_sts;
}

unsigned short rtc_s3701_read_ms_value(void)
{
	unsigned short rtc_ms = 0;

	rtc_ms = READ_WORD(RD_RTC_MS);
	return rtc_ms;
}

/*
when STB system enter standby mode from normer mode(normer  mode->standby mode),call pmu_mcu_wakeup_timer_set_min_alarm function to set
STB system wakeup time
*/
void pmu_mcu_wakeup_timer_set_min_alarm(struct min_alarm *alarm, unsigned char num)
{
	if((1 > alarm->month) || (12 < alarm->month) || \
		(1 > alarm->date) || (31 < alarm->date) || \
		(0 > alarm->hour) || (23 < alarm->hour) || \
		(0 > alarm->min) || (59 < alarm->min))
	{
		printk("\n================>The time that you input is not valid!!!!\n");
	}

	WRITE_BYTE(MAILBOX_WAKE_MONTH, alarm->month);
	WRITE_BYTE(MAILBOX_WAKE_DAY, alarm->date);
	WRITE_BYTE(MAILBOX_WAKE_HOUR, alarm->hour);
	WRITE_BYTE(MAILBOX_WAKE_MIN, alarm->min);

	PMU_PRINTF("\nSet wakeup time:[%d]:[%d]:[%d]:[%d]:[%d]\n", READ_BYTE(MAILBOX_WAKE_MONTH), READ_BYTE(MAILBOX_WAKE_DAY), READ_BYTE(MAILBOX_WAKE_HOUR), READ_BYTE(MAILBOX_WAKE_MIN));
}

void rtc_s3701_set_min_alarm(struct min_alarm* alarm,unsigned char num)
{
	unsigned long set_value = 0;
	unsigned long base_add = BASE_ADD;

	if(num<8)
	{
		set_value=(alarm->en_month<<30)|(alarm->en_date<<29)|(alarm->en_sun<<28)|\
			  	  (alarm->en_mon<<27)|(alarm->en_tue<<26)|(alarm->en_wed<<25)|\
			  	  (alarm->en_thr<<24)|(alarm->en_fri<<23)|(alarm->en_sat<<22)|\
			  	  (alarm->month<<16)|(alarm->date<<11)|(alarm->hour<<6)|(alarm->min);
	 	WRITE_DWORD(CONFIG0+num*4, set_value);
	}
}

void rtc_s3701_set_ms_alarm(struct ms_alarm* alarm,unsigned char num)
{
	unsigned long set_value = 0;
	unsigned long base_add = BASE_ADD;

	if((num>=8) && (num<10))
	{
		set_value=(alarm->en_hour<<30)|(alarm->en_min<<29)|(alarm->en_sec<<28)|\
			  	  (alarm->en_ms<<27)|(alarm->hour<<22)|(alarm->min<<16)|\
			  	  (alarm->sec<<10)|(alarm->ms);
	 	WRITE_DWORD(CONFIG0+num*4, set_value);
	} 		
}

void rtc_s3701_en_alarm(unsigned char enable,unsigned char num) /*step 2 */
{
	unsigned long base_add = BASE_ADD;

	if(num<10)
	{	
		WRITE_DWORD(CONFIG0+num*4, READ_DWORD(CONFIG0+num*4) | (enable<<31));
		WRITE_DWORD(IIR, (READ_DWORD(IIR) | (1<<(num+16))) & (~(enable<<(num+16))));
	}
}

static unsigned long  mips_to_mcs51(unsigned long  tmp)
{
	return (((tmp&0xff)<<24)|(((tmp>>8)&0xff)<<16)|(((tmp>>16)&0xff)<<8)|(((tmp>>24)&0xff)));
}

static void pmu_mcu_wakeup_ir_power_key(unsigned long *pmu_ir_key)
{
	unsigned long i = 0;
	unsigned long pmu_ir_key_51[8] = {0};

	for(i=0; i<8; i++)
	{
		if(pmu_ir_key[i] == 0xFFFFFFFF)	//#define PAN_KEY_INVALID	0xFFFFFFFF
		{
			pmu_ir_key_51[i] = 0x5a5a5a5a;
		}
		else
		{
			pmu_ir_key_51[i] = mips_to_mcs51(pmu_ir_key[i]);
		}
	}

	for(i=0; i<8; i++)
	{
		WRITE_DWORD((PMU_SRAM_BASE_ADDR+0x3fc0+i*4), pmu_ir_key_51[i]);
	}
}

void pmu_wakeup_keyval(unsigned int power_key)
{
	WRITE_BYTE(NEC_IR_KEY_SARM_LOW0, (unsigned char)(power_key&0xff));
	WRITE_BYTE(NEC_IR_KEY_SARM_LOW1, (unsigned char)((power_key>>8)&0xff));
	WRITE_BYTE(NEC_IR_KEY_SARM_LOW2, (unsigned char)((power_key>>16)&0xff));
	WRITE_BYTE(NEC_IR_KEY_SARM_LOW3, (unsigned char)((power_key>>24)&0xff));
}

void set_mcu_show_time(unsigned char key)
{
	WRITE_BYTE(STANDBY_SHOW_TIMR_SARM, key);
}

void pmu_enter_standby(void)
{
	unsigned long cnt;
	volatile unsigned long i = 0;

	/*Close watch dog*/
	WRITE_DWORD(WATCHDOG_REG_ADDR, 0x0);

	while(1)
	{
	#ifndef CONFIG_ARM
		if(ALI_S3821 == ali_sys_ic_get_chip_id())
		{
			WRITE_BYTE(M3821_POWER_OFF_ENABLE_REG, 0x0);
		}

		WRITE_DWORD(INTERRUPT_ENABLE_REG1, 0x0);//close all interrupt
		WRITE_DWORD(INTERRUPT_ENABLE_REG2, 0x0);//close all interrupt
		mdelay(5);

		WRITE_DWORD(PMU_IP_RESET_REG, READ_DWORD(PMU_IP_RESET_REG) | (1<<25));
		memcpy((u8*)(PMU_SRAM_BASE_ADDR), (u8*)g_ali_pmu_bin, sizeof(g_ali_pmu_bin));//copy mcu code
		WRITE_DWORD(PMU_RAM_SWITCH_REG, READ_DWORD(PMU_RAM_SWITCH_REG) & (~(1<<0)));//ram switch
		mdelay(5);
		WRITE_DWORD(PMU_IP_RESET_REG, READ_DWORD(PMU_IP_RESET_REG) & (~(1<<25)));//reset MCU
		mdelay(5);
		WRITE_DWORD(CPU_TO_MCU_IE_REG1, READ_DWORD(CPU_TO_MCU_IE_REG1) | (1<<0));//cpu->mcu IE int enable
		mdelay(5);
		WRITE_DWORD(CPU_TO_MCU_IE_REG2, READ_DWORD(CPU_TO_MCU_IE_REG2) | (1<<0));//cpu->mcu IE int enable
		mdelay(100);
	#else
		WRITE_DWORD(INTERRUPT_ENABLE_REG1, 0x0);//close all interrupt
		WRITE_DWORD(INTERRUPT_ENABLE_REG2, 0x0);//close all interrupt
		pmu_delay();
		WRITE_DWORD(PMU_IP_RESET_REG, READ_DWORD(PMU_IP_RESET_REG) & (~(1<<17)));//reset MCU
		WRITE_DWORD(PMU_RAM_SWITCH_REG, READ_DWORD(PMU_RAM_SWITCH_REG) | (1<<0));
		pmu_delay();
		memcpy((u8*)__PREG32ALI(PMU_SRAM_BASE_ADDR), g_ali_pmu_bin, sizeof(g_ali_pmu_bin));
		pmu_delay();
		WRITE_DWORD(PMU_RAM_SWITCH_REG, READ_DWORD(PMU_RAM_SWITCH_REG) & (~(1<<0)));
		WRITE_DWORD(PMU_IP_RESET_REG, READ_DWORD(PMU_IP_RESET_REG) | (1<<17));
		pmu_delay();

		WRITE_BYTE(CPU_TO_MCU_IE_REG1, READ_DWORD(CPU_TO_MCU_IE_REG1) | (1<<0));//cpu->mcu IE int enable
		pmu_delay();
		WRITE_BYTE(CPU_TO_MCU_IE_REG2, READ_DWORD(CPU_TO_MCU_IE_REG2) | (1<<0));//cpu->mcu IE int enable

		for(cnt=0; cnt<0x1000; cnt++)
		{
			for(i=0; i<0xffff0; i++);
		} 
	#endif
	}
}

long ali_pmu_ioctl( struct file * file, unsigned int cmd, unsigned long param)
{
	static unsigned long ir_power_key = 0x60df708f;
	struct rtc_time_pmu base_time;  
	struct min_alarm_num min_alm_num;
	struct ms_alarm_num ms_alm_num;
	unsigned char en_num[2] = {0};
	unsigned short read_ms = 0;
	unsigned char read_status = 0;
	void __user *argp = (void __user *)param;
	struct rtc_time_pmu rtc_read;
	struct rtc_time_pmu rtc_read_init;
	enum MCU_SHOW_PANNEL flag;
	u8 ret = 0;
	u8 powerup_status = 0;

	if((chip_id == ALI_C3701) && (rev_id == IC_REV_0))
	{
		switch (cmd)
		{
			case ALI_PMU_EN: 
				free_irq(PMU_IRQ, pmu_input);
				pmu_hw_init();
				pmu_m36_en();	
				break;

			case ALI_PMU_IR_PROTOL_NEC:
				ret = copy_from_user(&ir_power_key,(unsigned long *)param,sizeof(unsigned long));
				nec_trans_pmu_ir_thr_sel(ir_power_key);
				break;

			case ALI_PMU_RTC_SET_VAL:
				ret = copy_from_user(&base_time,(struct rtc_time_pmu *)param,sizeof(struct rtc_time_pmu));
				rtc_set_value( &base_time, SB_TIMER_0);    
				break;

			case ALI_PMU_RTC_SET_MIN_ALARM:
				ret = copy_from_user(&min_alm_num,(struct min_alarm_num *)param,sizeof(struct min_alarm_num));
				rtc_s3701_set_min_alarm(&(min_alm_num.min_alm),min_alm_num.num);
				break;

			case ALI_PMU_RTC_SET_MS_ALARM:
				ret = copy_from_user(&ms_alm_num,(struct ms_alarm_num *)param,sizeof(struct ms_alarm_num));
				rtc_s3701_set_min_alarm((struct min_alarm*)&(ms_alm_num.ms_alm),ms_alm_num.num);
				break;

			case ALI_PMU_RTC_EN_ALARM:
				ret = copy_from_user(en_num,(unsigned char*)param,sizeof(char)*2);
				rtc_s3701_en_alarm(en_num[0],en_num[1]);
				break;

			case ALI_PMU_RTC_RD_VAL:
				rtc_time_read_value(SB_TIMER_0, &rtc_read);
				ret = copy_to_user(argp, &rtc_read, sizeof(struct rtc_time_pmu));
				break;

			case ALI_PMU_RTC_RD_MS_VAL:
				read_ms=rtc_s3701_read_ms_value();
				ret = copy_to_user( ( unsigned long *) param  ,&read_ms,sizeof(unsigned short));
				break;

			case ALI_PMU_EXIT_STANDBY_STATUS:
				read_status= rtc_s3701_read_stdby_status();
				ret = copy_to_user( ( unsigned long *) param  ,&read_status,sizeof(unsigned char));
				break;

			default:
				//PMU_PRINTF("%s()=>default.cmd==%0x\n", __FUNCTION__,cmd);
				break;
		}
	}
	else if((chip_id == ALI_C3701 && rev_id >IC_REV_0) || (chip_id == ALI_S3503) || \
		(chip_id == ALI_S3501) || (chip_id == ALI_S3821))
	{
		switch (cmd)
		{
			case ALI_PMU_SHOW_TIME_EN:		 	       
				if(get_user(flag, (char __user *)argp))
				{
					return -EFAULT;
				}
				set_mcu_show_time((u8)flag);   
				break ;

			case ALI_PMU_RTC_SET_VAL: //when norm mode use south rtc
				ret = copy_from_user(&base_time,(struct rtc_time_pmu *)param,sizeof(struct rtc_time_pmu));
				//printk("\n\n\n\n/****Current position====>Function:%s====>ALI_PMU_RTC_SET_VAL====>  Line:%d****/\n", __FUNCTION__,__LINE__);
				rtc_set_value( &base_time, SB_TIMER_0);
				break;

			case ALI_PMU_RTC_TIMER3_SET_VAL:
				ret = copy_from_user(&base_time,(struct rtc_time_pmu *)param,sizeof(struct rtc_time_pmu));
				rtc_set_value( &base_time, SB_TIMER_3);    
				break;

			case ALI_PMU_RTC_RD_VAL:
				//printk("\n\n\n\n/****Current position====>Function:%s====>ALI_PMU_RTC_RD_VAL====>  Line:%d****/\n", __FUNCTION__,__LINE__);
				rtc_time_read_value(SB_TIMER_0, &rtc_read);
				ret = copy_to_user(argp, &rtc_read, sizeof(struct rtc_time_pmu));
				break;

			case ALI_PMU_RTC_TIMER3_GET_VAL:
				rtc_time_read_value(SB_TIMER_3, &rtc_read);
				ret = copy_to_user(argp, &rtc_read, sizeof(struct rtc_time_pmu));
				break;

			case  ALI_PMU_MCU_ENTER_STANDBY:
				pmu_mcu_wakeup_ir_power_key(wakeup_power_key);
				printk("\n\n\n========>Received current time is [year_h:%d], [year_l:%d], [mon:%d], [day:%d], [hour:%d], [min:%d], [sec:%d]", READ_BYTE(MAILBOX_SET_YEAR_H), READ_BYTE(MAILBOX_SET_YEAR_L), \
					READ_BYTE(MAILBOX_SET_MONTH), READ_BYTE(MAILBOX_SET_DAY), READ_BYTE(MAILBOX_SET_HOUR), READ_BYTE(MAILBOX_SET_MIN), READ_BYTE(MAILBOX_SET_SEC));
				printk("\n/********Current position====>Function:%s  Line:%d, enter standby!********/\n", __FUNCTION__,__LINE__);
	
				pmu_enter_standby();
				break;

			case ALI_PMU_MCU_SET_TIME:/*through mailbox set system time,when enter standby mode*/
				//printk("\n\n\n\n/****Current position====>Function:%s====>ALI_PMU_MCU_SET_TIME====>  Line:%d****/\n", __FUNCTION__,__LINE__);
				ret = copy_from_user(&base_time,(struct rtc_time_pmu *)param,sizeof(struct rtc_time_pmu));
				pmu_mcu_enter_stby_timer_set_value( &base_time);
				break;

			case ALI_PMU_MCU_READ_TIME:    /* from standy mode -->normer mode,read mcu rtc init value*/
				//printk("\n\n\n\n/****Current position====>Function:%s====>ALI_PMU_MCU_READ_TIME====>  Line:%d****/\n", __FUNCTION__,__LINE__);
				rtc_read_init = rtc_time_init_value();
				ret = copy_to_user(argp, &rtc_read_init, sizeof(struct rtc_time_pmu));
				break;

			case ALI_PMU_MCU_WAKEUP_TIME: /*when normer mode ->standby mode ,set wake up time,through  mailbox*/
				//printk("\n ALI_PMU_MCU_WAKEUP_TIME...\n");
				ret = copy_from_user(&min_alm_num,(struct min_alarm_num *)param,sizeof(struct min_alarm_num));
				pmu_mcu_wakeup_timer_set_min_alarm(&(min_alm_num.min_alm),min_alm_num.num);
				break;

			case ALI_PMU_EXIT_STANDBY_STATUS:
				read_status = g_exit_standby_sts;
				ret = copy_to_user( ( unsigned long *) param  , &read_status, sizeof(unsigned char));
				break;

			case ALI_PMU_IR_PROTOL_NEC:
				copy_from_user(&wakeup_power_key, (unsigned long *)param, 8*sizeof(unsigned long));
				pmu_mcu_wakeup_ir_power_key(wakeup_power_key);
				break;

			case ALI_PMU_REPORT_EXIT_TYPE:
				powerup_status = get_powerup_status();
				ret = copy_to_user((unsigned long *)param, &powerup_status, sizeof(unsigned char));
				break;

			default:
				//printk("%s()=>default.cmd==%0x\n", __FUNCTION__,cmd);
				break;
		}
	}
	else if(chip_id == ALI_C3921)
	{
	        switch (cmd)
	        {
			case ALI_PMU_SHOW_TIME_EN:
				if(get_user(flag, (char __user *)argp))
				{
					return -EFAULT;
				}
				set_mcu_show_time((u8)flag);
				break ;
				
			case ALI_PMU_RTC_SET_VAL: //when norm mode use south rtc.
				ret = copy_from_user(&base_time, (struct rtc_time_pmu *)param, sizeof(struct rtc_time_pmu));
				rtc_set_value( &base_time, SB_TIMER_0);
				break;
				
			case ALI_PMU_RTC_RD_VAL:  //when norm mode use south rtc.
				rtc_time_read_value(SB_TIMER_0, &rtc_read);
				ret = copy_to_user(argp, &rtc_read, sizeof(struct rtc_time_pmu));
				break;
			
			case  ALI_PMU_MCU_ENTER_STANDBY:
				pmu_mcu_wakeup_ir_power_key(wakeup_power_key);
				printk("\n\n\n========>Received current time is [year_h:%d], [year_l:%d], [mon:%d], [day:%d], [hour:%d], [min:%d], [sec:%d]", READ_BYTE(MAILBOX_SET_YEAR_H), READ_BYTE(MAILBOX_SET_YEAR_L), \
					READ_BYTE(MAILBOX_SET_MONTH), READ_BYTE(MAILBOX_SET_DAY), READ_BYTE(MAILBOX_SET_HOUR), READ_BYTE(MAILBOX_SET_MIN), READ_BYTE(MAILBOX_SET_SEC));
				printk("\n/********Current position====>Function:%s  Line:%d, enter standby!********/\n", __FUNCTION__,__LINE__);
				#if defined(CONFIG_ALI_CHIP_M3921)
				asm volatile ( "msr cpsr_cxsf,#0xd3" );
				asm volatile ( "dsb" );
				//output_string("3921 disable interrupt, new!\n");
				#endif

				pmu_enter_standby();
				break;
			
			case ALI_PMU_MCU_SET_TIME:/*through mailbox set system time,when enter standby mode*/
				//printk("\nCurrent position====>Function:%s,  Line:%d, enter standby set time...\n", __FUNCTION__, __LINE__);
				ret = copy_from_user(&base_time,(struct rtc_time_pmu *)param,sizeof(struct rtc_time_pmu));
				pmu_mcu_enter_stby_timer_set_value(&base_time);
				break;

			case ALI_PMU_MCU_READ_TIME:/* from standy mode -->normer mode,read mcu rtc init value*/
				rtc_read_init = rtc_time_init_value();
				ret = copy_to_user(argp, &rtc_read_init, sizeof(struct rtc_time_pmu));
				break;

			case ALI_PMU_MCU_WAKEUP_TIME: /*when normer mode ->standby mode ,set wake up time,through  mailbox*/
				ret = copy_from_user(&min_alm_num,(struct min_alarm_num *)param,sizeof(struct min_alarm_num));
				pmu_mcu_wakeup_timer_set_min_alarm(&(min_alm_num.min_alm),min_alm_num.num);

				break;
				
			case ALI_PMU_IR_PROTOL_NEC:
				copy_from_user(&wakeup_power_key, (unsigned long *)param, 8*sizeof(unsigned long));
				pmu_mcu_wakeup_ir_power_key(wakeup_power_key);
				break;

			case ALI_PMU_REPORT_EXIT_TYPE:
				powerup_status = get_powerup_status();
				ret = copy_to_user((unsigned long *)param, &powerup_status, sizeof(unsigned char));
				break;
				
			default:
				break;
		}
	}

	return 0;
}

static irqreturn_t ali_pmu_interrupt(int irq, void *dev_id)
{
	if((chip_id == ALI_C3921) || (ALI_S3503 == chip_id) || (ALI_S3821 == chip_id))
	{
		if(READ_BYTE(SB_TIMER3_CTL_REG) & RTC_TOV)// SB-Time3 enbale
		{
			WRITE_BYTE(SB_TIMER3_CTL_REG, (READ_BYTE(SB_TIMER3_CTL_REG) & 0xef)); //clear interrupt(disable)
			time_accumulate(NULL);
			WRITE_DWORD(SB_TIMER3_CNT, rtc_onesec & 0xFFFFFFFF);
			WRITE_BYTE(SB_TIMER3_CTL_REG, 0x00);

			//RTC starts to count with 32 divisor clock frequency, and enable interrupt
			/*Changed by tony for disable timer2 interrupt*/
			WRITE_BYTE(SB_TIMER3_CTL_REG, (READ_BYTE(SB_TIMER3_CTL_REG) | 0x14)); //interrupt again(enable)
		}
	}

	return IRQ_HANDLED;
}

int ali_pmu_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int ali_pmu_close(struct inode *inode, struct file *file)
{	
	return 0;
}

static const struct file_operations ali_pmu_fops =
{
	.owner		= THIS_MODULE,
	.open		= ali_pmu_open,
	.release		= ali_pmu_close,
	.unlocked_ioctl			= ali_pmu_ioctl,
};

static struct pmu_device pmu_dev = {
	.dev_name = "ali_pmu",
};

static int __devinit ali_pmu_init(void)
{
	int ret = 0;

	printk("\n\n\n\n%s\n", "ALi PMU Driver");
	spin_lock_init(&rtc_lock);
	chip_id = ali_sys_ic_get_chip_id();

	pmu_hw_init();
	ret = alloc_chrdev_region(&pmu_dev.dev_no, 0, 1, pmu_dev.dev_name);
	if (ret < 0)
	return ret;

	pmu_dev.pmu_class = class_create(THIS_MODULE, "ali_pmu_class");
	if(IS_ERR(pmu_dev.pmu_class))
	{
		ret = PTR_ERR(pmu_dev.pmu_class);
		goto err0;
	}

	cdev_init(&pmu_dev.pmu_cdev, &ali_pmu_fops);
	ret = cdev_add(&pmu_dev.pmu_cdev, pmu_dev.dev_no, 1);
	if (ret < 0)
	{
		goto err1;
	}

	pmu_dev.pmu_device_node = device_create(pmu_dev.pmu_class, NULL, \
	                          pmu_dev.dev_no, &pmu_dev, pmu_dev.dev_name);

	if (IS_ERR(pmu_dev.pmu_device_node))
	{
		ret = PTR_ERR(pmu_dev.pmu_device_node);
		goto err2;
	}

	rtc_init();
	if(ALI_S3503 == chip_id)
	{
		//printk("%s:%d\n",__func__,__LINE__);
		if(request_irq(M36_IRQ_RTC, (irq_handler_t)ali_pmu_interrupt, 0, "ali_pmu", NULL) < 0)
		{
			printk(KERN_ERR "Failed to register ali_pmu interrupt\n");
			goto fail_m36_interrupt;
		}
		
		//printk("%s()=>request_irq end.\n\n\n\n", __FUNCTION__);
		return ret;
		
	fail_m36_interrupt: free_irq(M36_IRQ_RTC, NULL);
	}
	if(ALI_S3821 == chip_id)
	{
		//printk("%s:%d\n",__func__,__LINE__);
		if(request_irq(M36_IRQ_RTC, (irq_handler_t)ali_pmu_interrupt, 0, "ali_pmu", NULL) < 0)
		{
			printk(KERN_ERR "Failed to register ali_pmu interrupt\n");
			goto fail_m3821_interrupt;
		}
		
		//printk("%s()=>request_irq end.\n\n\n\n", __FUNCTION__);
		return ret;
		
	fail_m3821_interrupt: free_irq(M36_IRQ_RTC, NULL);
	}
	else if(ALI_C3921 == chip_id)
	{
		if (request_irq(INT_ALI_IRQPMU, ali_pmu_interrupt, 0, "ali_pmu", &pmu_dev) < 0)
		{
			PMU_PRINTF(KERN_ERR "Failed to register ali_pmu interrupt\n");
			goto fail_C3921_pmu_interrupt;
		}

		if (request_irq(C3921_PMU_RTC0_IRQ, ali_pmu_interrupt, 0, "ali_pmu", &pmu_dev) < 0)
		{
			PMU_PRINTF(KERN_ERR "Failed to register ali_pmu interrupt\n");
			goto fail_C3921_pmu_rtc_interrupt;
		}

		//printk("\n\n/********pmu driver success! Interrupt register success!********/\n\n");
		return ret;

		fail_C3921_pmu_interrupt:	free_irq(INT_ALI_IRQPMU, &pmu_dev);
		fail_C3921_pmu_rtc_interrupt: free_irq(C3921_PMU_RTC0_IRQ, &pmu_dev);
	}

err2:
	cdev_del(&pmu_dev.pmu_cdev);

err1:
	class_destroy(pmu_dev.pmu_class);

err0:
	unregister_chrdev_region(pmu_dev.dev_no, 1);

	return 0;
}

PMU_EXIT_FLAG get_powerup_status(void)
{
	PMU_EXIT_FLAG  pmu_exit_flag = 0xff;
	UINT8 pmu_exit_standby_sts = 0;

#ifdef CONFIG_ARM
	if((READ_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS2_M39X) != 'P')&&\
		(READ_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS1_M39X) != 'M')&&\
		(READ_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS0_M39X) != 'U'))
	{
		pmu_exit_flag = E_PMU_COLD_BOOT;
		printk("\n========>Platform boot type is cold boot!!!!\n");
		return pmu_exit_flag;
	}
#else
	if(ALI_S3503 == ali_sys_ic_get_chip_id())
	{
		if((READ_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS2_M35X) != 'P')&&\
			(READ_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS1_M35X) != 'M')&&\
			(READ_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS0_M35X) != 'U'))
		{
			pmu_exit_flag = E_PMU_COLD_BOOT;
			printk("\n========>Platform boot type is cold boot!!!!\n");
			return pmu_exit_flag;
		}
	}
	else if(ALI_S3821 == ali_sys_ic_get_chip_id())
	{
		if((READ_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS2_M382X) != 'P')&&\
			(READ_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS1_M382X) != 'M')&&\
			(READ_BYTE(MAILBOX_GET_EXIT_STANDBY_STATUS0_M382X) != 'U'))
		{
			pmu_exit_flag = E_PMU_COLD_BOOT;
			printk("\n========>Platform boot type is cold boot!!!!\n");
			return pmu_exit_flag;
		}
	}
#endif

	pmu_exit_standby_sts = __REG8ALI(EXIT_STANDBY_TYPE_REG);
	if(EXIT_STANDBY_TYPE_PANEL == pmu_exit_standby_sts)
	{
		pmu_exit_flag = E_PMU_KEY_EXIT;
		printk("\n========>Platform exit by panel key in the last test process!!!!\n");
	}
	else if(EXIT_STANDBY_TYPE_IR == pmu_exit_standby_sts)
	{
		pmu_exit_flag = E_PMU_IR_EXIT;
		printk("\n========>Platform exit by ir key in the last test process!!!!\n");
	}
	else if(EXIT_STANDBY_TYPE_RTC == pmu_exit_standby_sts)
	{
		pmu_exit_flag = E_PMU_RTC_EXIT;
		printk("\n========>Platform exit by RTC in the last test process!!!!\n");
	}

	return pmu_exit_flag;
}

static void __exit ali_pmu_exit(void)
{
	return;
}

module_init(ali_pmu_init);
module_exit(ali_pmu_exit);

MODULE_AUTHOR("Goilath Peng");
MODULE_DESCRIPTION("ALI PMU Driver");
MODULE_LICENSE("GPL");
