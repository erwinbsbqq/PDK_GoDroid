/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2009 Copyright (C)
 *
 *    File:    otp_m33.c
 *
 *    Description:    This file contains all globe micros and functions implementation
 *		             of otp.
 *  History:
 *      Date        	Author      	Version  		Comment
 *      ====        	======      	=======  	=======
 *  1.  2009.8.25   	Goliath Peng   0.1.000  		Initial
 ****************************************************************************/
#if 0
#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_gpio.h>

#include <asm/chip.h>
#endif
#include <asm-generic/gpio.h>

#include <ali/sys_define.h>
#include <ali/basic_types.h>
#include <ali/retcode.h>
#include <ali/hld_dev.h>
#include <ali//sys_config.h>
#include <ali/string.h>
#include <ali/chunk.h>
//#include "../include/hal_gpio.h"


#include <ali/osal.h>

//#include <hal/hal_gpio.h>
#include <ali/otp.h>
#include "otp_drv.h"


#if 1
#define M37_OTP_DEBUG(...)			do{}while(0)
#else
#define M37_OTP_DEBUG				libc_printf//soc_printf
//#define CHECK_CODEZONE			
#endif
#ifdef __ARM__
#define M37_OTP_BASE 0x18042000
#else
#define M37_OTP_BASE 0xb8042000
#endif


#define M37_OTP_VER_ID			0x00
#define M37_OTP_ADDR			0x04
#define M37_OTP_WDATA			0x08
#define M37_OTP_CTRL			0x0c
#define M37_OTP_ST				0x10
#define M37_OTP_INT				0x14
#define M37_OTP_RDATA			0x18
#define M37_OTP_READ_TIM		0x1c
#define M37_OTP_PROG_TIM0		0x20
#define M37_OTP_PROG_TIM1		0x24
#define M37_OTP_PROG_TIM2		0x28
#define M37_OTP_PROG_RETRY		0x2c
#define M37_OTP_TIMING			0x30
#define M37_OTP_ST_PROG			0X34
#define M37_OTP_ST_PROG_TRIG	0X38

#define M37_OTP_PROG_TIM_EXT1  0x60
#define M37_OTP_PROG_TIM_EXT2  0x64

#define M37_OTP_BIT_READ_BUSY	1<<8
#define M37_OTP_BIT_READ_TRIG 	1<<8
#define M37_OTP_BIT_PROG_BUSY 	1<<0
#define M37_OTP_BIT_PROG_TRIG 	1<<0
#define M37_OTP_BIT_PROG_FAIL 	1<<8
#define M37_OTP_BIT_INT_ENABLE 	1<<0

#define M37_OTP_MEM_SIZE    0x400

#define M37_REG_ACCESS  (0x5f<<2)
#define M37_OTP_CONFIG  (0x03<<2)
#define M37_APP_REG_W_ACCESS  (0x80<<2)
#define M37_APP_REG_R_ACCESS  (0x81<<2)

#define REG_ACCESS_DELAY 0

#define M37_OTP_R_TIME_OUT    ((int)-6)
#define M37_OTP_P_TIME_OUT    ((int)-5)
#define M37_OTP_R_FAILURE     ( (int)-4)
#define M37_OTP_P_FAILURE     ( (int)-3)


static UINT32 C3701C_OTP_TIM[3][9]={
	{166*8,0,0x06041440,0x000A0606,0x00082852,0x00011161,0x06070607,0x0050301f,0x00600f50},// 166	-- default	
	{133*8,1,0x06041440,0x000A0606,0x00082852,0x00011161,0x06070607,0x00003711,0x00580948},// 133
	{100*8,1,0x06041440,0x000A0606,0x00082852,0x00011161,0x06070607,0x00002611,0x0040073a},// 100
};

static UINT32 S3503_OTP_TIM[8][9]={
	{20,0,0x06041440,0x000A0606,0x00082852,0x00011161,0x06070607,0x0050301f,0x00600f50},//19.8	
	{528,1,0x06041440,0x000A0606,0x00082852,0x00011161,0x06070607,0x00003711,0x00580948},//264
	{950,0,0x06041A40,0x000d0707,0x000a2972,0x000222b3,0x090a090a,0x0050301f,0x00600f50},//475.2
	{740,1,0x06041A40,0x000d0707,0x000a2972,0x000222b3,0x090a090a,0x00002810,0x00400d38},//369.6
	{1028,0,0x06041A40,0x000d0707,0x000b2a73,0x000222b3,0x090a090a,0x0050301f,0x00600f50},//514.8
	{1108,0,0x06041A40,0x000d0707,0x000b2b73,0x000222b3,0x090a090a,0x0050301f,0x00600f50},//554
	{634,0,0x06041440,0x000A0606,0x00071642,0x00011161,0x06070607,0x0050301f,0x00600f50},//316.8
	{800,0,0x06041440,0x000A0606,0x00082852,0x00011161,0x06070607,0x0050301f,0x00600f50},//396 -- default	
};

static OSAL_ID otp_mutex_id = OSAL_INVALID_ID;
static UINT8 obuf[M37_OTP_MEM_SIZE];
static UINT32 otp_ready =0;

static inline UINT32 delay_us(UINT32 tms)
{
    if( 0 == tms )
    {
        return 0;
    }
    return osal_task_sleep(tms);
}

static UINT32 read_reg_dword(UINT32 offset)
{
	delay_us(REG_ACCESS_DELAY);
	return (*(volatile UINT32 *)(M37_OTP_BASE + offset));
}

static void write_reg_byte(UINT32 offset, UINT8 value)
{
	*(volatile UINT8 *)(M37_OTP_BASE + offset) = value;
	delay_us(REG_ACCESS_DELAY);
}

static void write_reg_dword(UINT32 offset, UINT32 value)
{
	*(volatile UINT32 *)(M37_OTP_BASE + offset) = value;
	delay_us(REG_ACCESS_DELAY);
}

static void set_reg_bit(UINT32 offset, UINT8 bit) //set to 1
{
	UINT8 value = 0;

	value = *(volatile UINT8 *)(M37_OTP_BASE + offset);
	delay_us(REG_ACCESS_DELAY);
	value|=(0x1<<(bit));
	*(volatile UINT8 *)(M37_OTP_BASE + offset) = value;
	delay_us(REG_ACCESS_DELAY);
}

static void clear_reg_bit(UINT32 offset, UINT8 bit) //set to 0
{
	UINT8 value = 0;

	value = *(volatile UINT8 *)(M37_OTP_BASE + offset);
	delay_us(REG_ACCESS_DELAY);
	value&=(~(0x1<<(bit)));
	*(volatile UINT8 *)(M37_OTP_BASE + offset) = value;
	delay_us(REG_ACCESS_DELAY);
}

static int otp_wait_read_free(void)
{
    UINT32 tmo = 400;

    do
	{
		if(0==tmo)
		{
			return M37_OTP_R_TIME_OUT;
		}
		tmo--;
		osal_delay(1);
	}while(read_reg_dword(M37_OTP_ST)&M37_OTP_BIT_READ_BUSY);
    return RET_SUCCESS;    

}

static int otp_wait_write_free(void)
{
    UINT32 tmo = 310 * 12;

    do
	{
		if(0==tmo)
		{
			return M37_OTP_P_TIME_OUT;
		}
		tmo--;
		osal_delay(1);
	}while(read_reg_dword(M37_OTP_ST)&M37_OTP_BIT_PROG_BUSY);
    return RET_SUCCESS;    
}

static int otp_read_dword(UINT16 addr)
{
	if(otp_wait_read_free() != RET_SUCCESS)
	{
        return M37_OTP_R_TIME_OUT;
	}
    else if(otp_wait_write_free() != RET_SUCCESS)
    {
        return M37_OTP_P_TIME_OUT;
    }
    M37_OTP_DEBUG("otp read dw :0x%x\n", addr);
        
	write_reg_dword(M37_OTP_ADDR, addr&0x3ff);
	write_reg_dword(M37_OTP_CTRL, read_reg_dword(M37_OTP_CTRL)|M37_OTP_BIT_READ_TRIG);

    if( otp_wait_read_free() != RET_SUCCESS )
    {
        return M37_OTP_R_TIME_OUT;
    }
	return (read_reg_dword(M37_OTP_RDATA));	
}

static UINT32 otp_write_byte(UINT16 addr, UINT8 data)
{
	//UINT16 value;
	if(otp_wait_read_free() != RET_SUCCESS)
	{
        return M37_OTP_R_TIME_OUT;
	}
    else if(otp_wait_write_free() != RET_SUCCESS)
    {
        return M37_OTP_P_TIME_OUT;
    }

    M37_OTP_DEBUG("otp write bytes :0x%x\n", addr);

    osal_interrupt_disable();
	write_reg_dword(M37_OTP_ADDR, addr&0x3ff);
	write_reg_byte(M37_OTP_WDATA, data);
    M37_OTP_DEBUG("otp_write_byte reg : [0x04] = 0x%x,[0x08] = 0x%x \n", \
		read_reg_dword(M37_OTP_ADDR) ,read_reg_dword(M37_OTP_WDATA));
	write_reg_dword(M37_OTP_CTRL, read_reg_dword(M37_OTP_CTRL)|M37_OTP_BIT_PROG_TRIG);
    osal_interrupt_enable();

	if( otp_wait_write_free() != RET_SUCCESS )
	{
		M37_OTP_DEBUG("otp write bytes time out \n");
		return M37_OTP_P_TIME_OUT ;
	}

	//value = read_reg_dword(OTP_INT);
	if(read_reg_dword(M37_OTP_INT)&M37_OTP_BIT_INT_ENABLE)
	{
		if(read_reg_dword(M37_OTP_INT)&M37_OTP_BIT_PROG_FAIL)
		{
			return M37_OTP_P_FAILURE;
		}
		else
		{
			return RET_SUCCESS;	
		}
	}
	else
	{
		return RET_SUCCESS;
	}
}
static UINT32 sys_ic_get_dram_clock(void)
{
	UINT32  reg ;
	unsigned long strap_pin_reg, dram_clock;
	
	if( sys_ic_get_chip_id() == ALI_C3701 ){
		reg = readl(ALI_SOC_BASE+0x70) & (0x3<<5) ;
		if(reg == 0 )
			return 166*8 ;
		else if(reg == 1)
			return 133*8 ;
		else if(reg == 2)
			return 100*8 ;
		else if(reg == 3)
			return 20 ;
	}else if( sys_ic_get_chip_id() == ALI_S3503 ){
		reg = readl(ALI_SOC_BASE+0x70) & (0x7<<4) ;
		if(reg == 0 )
			return 20 ;
		else if(reg == 1)
			return 264*2 ;
		else if(reg == 2)
			return 475*2 ;
		else if(reg == 3)
			return 370*2 ;
		else if(reg == 4)
			return 514*2 ;
		else if(reg == 5)
			return 554*2 ;
		else if(reg == 6)
			return 316*2 ;
		else if(reg == 7)
			return 396*2 ;
		
	}else if ( sys_ic_get_chip_id() == ALI_S3921 )    {
		strap_pin_reg = readl(ALI_SOC_BASE+0x70);
		strap_pin_reg = (strap_pin_reg>>5)&0x07;
		if(strap_pin_reg == 0)
		{
			dram_clock = 33*2;	// bypass
		}
		else if(strap_pin_reg == 1)
		{
			dram_clock = 264*2;	// 528Mbps
		}
		else if(strap_pin_reg == 2)
		{
			dram_clock = 330*2;	// 688Mbps
		}
		else if(strap_pin_reg == 3)
		{
			dram_clock = 396*2;	// 800Mbps
		}
		else if(strap_pin_reg == 4)
		{
			dram_clock = 528*2;	// 1066Mbps
		}
		else if(strap_pin_reg == 5)
		{
			dram_clock = 660*2;	// 1333Mbps
		}
		else
		{
			dram_clock = 792*2;	// 1600Mbps
		}
		return dram_clock;
    	
    }
    else
		return 0;

}

INT32 otp_m37_init(void)
{
	UINT32 i=0;
	UINT32 mem_clk=0;

	if(1 == otp_ready)
	{
		return RET_SUCCESS;
	}
		
	mem_clk = sys_ic_get_dram_clock() ;
	for(i=0; i<3; i++)
	{
	    if( mem_clk == C3701C_OTP_TIM[i][0] )
		{
			if(C3701C_OTP_TIM[i][1])
			{
				set_reg_bit(M37_OTP_CTRL,16);
			}
			else
			{
				clear_reg_bit(M37_OTP_CTRL,16);
			}
			
			write_reg_dword(M37_OTP_READ_TIM,C3701C_OTP_TIM[i][2]);
			write_reg_dword(M37_OTP_PROG_TIM0,C3701C_OTP_TIM[i][3]);
			write_reg_dword(M37_OTP_PROG_TIM1,C3701C_OTP_TIM[i][4]);
			write_reg_dword(M37_OTP_PROG_TIM2,C3701C_OTP_TIM[i][5]);
			write_reg_dword(M37_OTP_TIMING,C3701C_OTP_TIM[i][6]);

			if(C3701C_OTP_TIM[i][1])
			{
				write_reg_dword(M37_OTP_PROG_TIM_EXT1,C3701C_OTP_TIM[i][7]);
				write_reg_dword(M37_OTP_PROG_TIM_EXT2,C3701C_OTP_TIM[i][8]);
			}
			break;
	    }
	}

	if(otp_mutex_id == OSAL_INVALID_ID)
		otp_mutex_id = osal_mutex_create();

	otp_ready =1;
	return RET_SUCCESS;
}
INT32 otp_m39_init(void)
{
	otp_ready = 1;
	return 0;
}

INT32 otp_m35_init(void)
{
	UINT32 i=0;
	UINT32 mem_clk=0;

	if(1 == otp_ready)
	{
		return RET_SUCCESS;
	}
		
	mem_clk = sys_ic_get_dram_clock() ;
	for(i=0; i<8; i++)
	{
	    if( mem_clk == S3503_OTP_TIM[i][0] )
		{
			if(S3503_OTP_TIM[i][1])
			{
				set_reg_bit(M37_OTP_CTRL,16);
			}
			else
			{
				clear_reg_bit(M37_OTP_CTRL,16);
			}
			
			write_reg_dword(M37_OTP_READ_TIM,S3503_OTP_TIM[i][2]);
			write_reg_dword(M37_OTP_PROG_TIM0,S3503_OTP_TIM[i][3]);
			write_reg_dword(M37_OTP_PROG_TIM1,S3503_OTP_TIM[i][4]);
			write_reg_dword(M37_OTP_PROG_TIM2,S3503_OTP_TIM[i][5]);
			write_reg_dword(M37_OTP_TIMING,S3503_OTP_TIM[i][6]);

			if(S3503_OTP_TIM[i][1])
			{
				write_reg_dword(M37_OTP_PROG_TIM_EXT1,S3503_OTP_TIM[i][7]);
				write_reg_dword(M37_OTP_PROG_TIM_EXT2,S3503_OTP_TIM[i][8]);
			}
			break;
	    }
	}

	if(otp_mutex_id == OSAL_INVALID_ID)
		otp_mutex_id = osal_mutex_create();

	otp_ready =1 ;
	return RET_SUCCESS;
}

INT32 otp_m37_read(UINT32 offset, UINT8 *buf, INT32 len)
{
	UINT32 i = 0;
	UINT32 tmp_data = 0;
	UINT32 chip_num = 0;
	INT32 ret = OTP_READ_ERROR;
	UINT32 tmp_ofset = offset & (~0x3); // dw ofset
	UINT32 tmp_len  = (len & (~0x3))/4 +1; // dw len

	M37_OTP_DEBUG("otp read : buf = 0x%x, ofset = 0x%x, len = 0x%x\n", buf, offset, len);

	if((!otp_ready)||(!len)||(!buf)||((offset+len)>M37_OTP_MEM_SIZE))
	{
		return 0;
	}

	osal_mutex_lock(otp_mutex_id, OSAL_WAIT_FOREVER_TIME);

	M37_OTP_DEBUG("otp read : tmp_len = 0x%x, tmp_ofset = 0x%x\n", tmp_len, tmp_ofset);
	for(i =0; i<tmp_len; i++ )
	{
		ret = otp_read_dword(tmp_ofset+i*4);
		if(( ret != M37_OTP_R_TIME_OUT ) && ( ret != M37_OTP_P_TIME_OUT ) )
		{
		    *((UINT32 *)obuf+i) = ret;
		}
		else
		{
		    osal_mutex_unlock(otp_mutex_id);        
		    M37_OTP_DEBUG("[OTP_ERROR]otp read failed,cause = %x, ofset = 0x%x\n", ret, tmp_ofset+i);
		    return 0;
		}
	}
	osal_mutex_unlock(otp_mutex_id); 

	tmp_data  = obuf[0];
	tmp_data |= obuf[1] << 8;
	tmp_data |= obuf[2] << 16;
	tmp_data |= obuf[3] << 24;
	M37_OTP_DEBUG("otp read : tmp_data = 0x%x\n", tmp_data);
	chip_num = (*(volatile UINT32 *)0xb8000000) >> 16;
	chip_num |= chip_num << 16;
	if(tmp_data == chip_num) //if OTP value equal to chipID
	{
		return OTP_READ_ERROR;  //OTP read protect
	}

	i = offset-tmp_ofset;
	MEMCPY(buf,&obuf[i],len);
	return len ;
}

INT32 otp_m37_write(UINT8 *buf, UINT32 offset, INT32 len)
{
    UINT32 j=0;
    INT32 rd_len = 0;
    INT32 wr_len = 0; 
    INT32 i = 0;
    INT32 k = 0;
	UINT8 *tmp_data = NULL;

	if((!otp_ready)||(!len)||(!buf)||((offset+len)>M37_OTP_MEM_SIZE))
	{
		return 0;
	}
	
    osal_mutex_lock(otp_mutex_id, OSAL_WAIT_FOREVER_TIME);
	
    /* double counters */
    for(i = 0, k = len; ((i < len) && (k > 0)); i++, k--)
	{
		if(otp_write_byte(offset+i, *(buf+i)) !=  RET_SUCCESS)
		{
			//value = read_reg_dword(M37_OTP_ST);
			M37_OTP_DEBUG("[OTP_ERROR]otp program, failed addr is 0x%x\n", offset);
			write_reg_dword(M37_OTP_INT, read_reg_dword(M37_OTP_INT)&(~M37_OTP_BIT_PROG_FAIL));
			break;
		}
        wr_len++;
	}

    osal_mutex_unlock(otp_mutex_id);

    if(wr_len != len)
    {
		return M37_OTP_P_FAILURE;
    }
    
    tmp_data = (UINT8*)malloc(len+0x4);
    if(NULL == tmp_data)
    {
        return 0;
    }
    
    memset(tmp_data, 0x0, len+0x4);
    rd_len = otp_m37_read(offset, tmp_data, len);
    if(rd_len != len)
    {
		return M37_OTP_R_FAILURE;
    }
	
    /* double counters */
    for(i = 0, k = len; ((i < len) && (k > 0)); i++, k--)
    {
        for(j = 0; j < 8; j++)
        {
            if( (0 == ((tmp_data[i]>>j)&0x1)) && (1 == ((buf[i]>>j)&0x1)) )
            {
                M37_OTP_DEBUG("Error: write OTP fail!(%d,%x,%x)\n",i,tmp_data[i], buf[i]);
                free(tmp_data);
                return OTP_WRITE_ERROR;
            }
            /* Redundant check */
            if(((tmp_data[i]>>j)&0x1) < ((buf[i]>>j)&0x1))
            {
                M37_OTP_DEBUG("Error: write OTP fail!(%d,%x,%x)\n",i,tmp_data[i], buf[i]);
                FREE(tmp_data);
                return OTP_WRITE_ERROR;
            }
        }
    }
    free(tmp_data);
    
	return wr_len;
}

INT32 otp_write_dw(UINT8 *buf, UINT32 offset, UINT32 mask)
{
    UINT32 i = 0;
    UINT32 k = 0;
    INT32 wr_len = 0;
    UINT32 rd_data = 0;
    UINT32 wr_data = 0;
    UINT32 len = sizeof(UINT32);

    if ((!otp_ready)||(!len)||((offset+len)>M37_OTP_MEM_SIZE)|| \
        ALI_C3701 > sys_ic_get_chip_id())
    {
        return 0;
    }

    osal_mutex_lock(otp_mutex_id, OSAL_WAIT_FOREVER_TIME);

    wr_data = (UINT32)buf[3]<<24 | (UINT32)buf[2]<<16 | (UINT32)buf[1]<<8 | buf[0];

    if (offset+len > M37_OTP_MEM_SIZE)
    {
        return OTP_WRITE_ERROR;
    }

    /* double counters */
    for(i = 0, k = len; ((i < len) && (k > 0)); i++, k--)
    {
        if(otp_write_byte(offset+i, *(buf+i)) != RET_SUCCESS)
        {
            read_reg_dword(M37_OTP_ST);
            M37_OTP_DEBUG("[OTP_ERROR]otp program, failed addr is 0x%x\n", offset);
            write_reg_dword(M37_OTP_INT, read_reg_dword(M37_OTP_INT)&(~M37_OTP_BIT_PROG_FAIL));
            break;
        }
        wr_len++;
    }

    osal_mutex_unlock(otp_mutex_id);

    if (wr_len != len)
    {
        return M37_OTP_P_FAILURE;
    }

    otp_m37_read(offset, (UINT8 *)&rd_data, len);

    rd_data &= mask;
    wr_data &= mask;

    if (memcmp(&rd_data,&wr_data,len))
    {
        M37_OTP_DEBUG("Error: write OTP fail!(%x,%x)\n",wr_data, rd_data);
        return OTP_WRITE_ERROR;
    }
    if(rd_data != wr_data)
    {
        M37_OTP_DEBUG("Error: write OTP fail!(%x,%x)\n",wr_data, rd_data);
        return OTP_WRITE_ERROR;
    }

    return len;
}

