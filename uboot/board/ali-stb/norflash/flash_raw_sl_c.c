/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: flash_raw_sl.c
 *
 *  Description: Provide local serial flash raw operations.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2006.4.24   Justin Wu   0.1.000  Initial
 *  2.  2006.11.10  Wen   Liu   0.2.000  Cleaning
 ****************************************************************************/
#include "sto_dev.h"
#include "flash.h"
#include "sys_config.h"
#include "../retcode.h"
#include "sto_flash.h"


//#define SUPPORT_GLOBAL_PROTECT
#define FLASHTYPE_SF_SST25VF 0xf5
//#define SYS_SFLASH_FAST_READ_SUPPORT

#define DELAY_MS(ms)  		osal_task_sleep(ms)
#define DELAY_US(us)		osal_delay(us)

#define SF_BASE_ADDR		0xb8000000
unsigned long sflash_reg_addr = SF_BASE_ADDR;
#define	SF_INS			(sflash_reg_addr + 0x98)
#define	SF_FMT			(sflash_reg_addr + 0x99)
#define	SF_DUM			(sflash_reg_addr + 0x9A)
#define	SF_CFG			(sflash_reg_addr + 0x9B)


#define SF_HIT_DATA		0x01
#define SF_HIT_DUMM		0x02
#define SF_HIT_ADDR		0x04
#define SF_HIT_CODE		0x08
#define SF_CONT_RD		0x40
#define SF_CONT_WR		0x80

UINT32 dummy ;
#define write_uint8(addr, val)		do{*((volatile UINT8 *)(addr)) = (val),dummy =*((volatile UINT8 *)(addr)); }while(0)
#define read_uint8(addr)			*((volatile UINT8 *)(addr))
#define write_uint16(addr, val)	do{*((volatile UINT16 *)(addr)) = (val),dummy =*((volatile UINT16 *)(addr)); }while(0)
#define read_uint16(addr)			*((volatile UINT16 *)(addr))
#define write_uint32(addr, val)	do{*((volatile UINT32*)(addr)) = (val),dummy =*((volatile UINT32 *)(addr)); }while(0)
#define read_uint32(addr)			*((volatile UINT32 *)(addr))

#define STD_READ	(0x03|((SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA)<<8))
#define FST_READ	(0x0b|((SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA|SF_HIT_DUMM)<<8))
#define DIO_READ	(0xbb|((SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA|SF_HIT_DUMM)<<8))
#define QIO_READ	(0xeb|((SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA|SF_HIT_DUMM|0x20)<<8))

#define STD_MODE	0
#define FST_MODE	STD_MODE
#define DIO_MODE	0x2
#define QIO_MODE	0x3

unsigned short sflash_devtp = FLASHTYPE_UNKNOWN;
unsigned short sflash_devid = FLASHTYPE_UNKNOWN;
static unsigned short aai_copy_enable = 0;
static UINT16 sflash_default_read = STD_READ;
static UINT8 sflash_default_mode = STD_MODE;
static UINT8 sqi_ctrl_rdy = 0;
static UINT8 qio_enable = 1;	/*1: board layout support 4 IO. 0: #W #Hold disconnected*/
#ifdef SUPPORT_GLOBAL_PROTECT		
static unsigned short global_protect_enable = 0;
static unsigned long gp_en_list[] = {
	0x001540ef, 0x14ef14ef,	//Winbond		25Q16VSIC
	0xbf4125bf, 0x41bf41bf,	//SST		SST25VF016B
	0x0000461f, 0x00000000	//ATMEL		26DF161
};
#endif
static UINT8 pp_ins = 0x02;

void sflash_write_soft_protect(UINT32 a,UINT8 *data,UINT8 c, UINT8 d)
{
}

UINT32 sys_ic_is_3701c(void)
{
	UINT32 chip_id ;
	
	chip_id = * ( volatile UINT32* )0xb8000000;
	if((chip_id & 0x37010000) == 0x37010000)
	{
		return 1;
	}else{
		return 0;
	}

}
	
static void sflash_wait_free()
{
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	/* Set SFLASH_INS to RDSR instruction */
	write_uint8(SF_INS, 0x05);
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|STD_MODE);
	/* Read from any address of serial flash. */
	while (1)
	{
		if((m_EnableSoftProtection && ((((volatile UINT8 *)unpro_addr_low)[0] & 0x01) == 0)) || 
			((!m_EnableSoftProtection) && ((((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] & 0x01) == 0)))
		break;
		osal_delay(10);
	}

	/* Reset sflash to common read mode */
	write_uint16(SF_INS, sflash_default_read);
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|sflash_default_mode);
}

static void sflash_write_enable(int en)
{
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE);
	/* Set SFLASH_INS to WREN instruction. */
	if (en)
		write_uint8(SF_INS, 0x06);
	else
		write_uint8(SF_INS, 0x04);
	
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|STD_MODE);
	/* Write to any address of serial flash. */
	if(m_EnableSoftProtection)
    	{
        	((volatile UINT8 *)unpro_addr_low)[0] = 0;
    	}
	else
	{
        	((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0;
    	}
}

static void sflash_global_protect(UINT8 en)
{
#ifdef SUPPORT_GLOBAL_PROTECT
	UINT8 status_reg;
	if(!global_protect_enable)
		return;
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	write_uint8(SF_INS, 0x05);
	status_reg = ((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] ;
	if(en)
		status_reg |= 0x3c;
	else
		status_reg &= (~0x3c);
	
	sflash_write_enable(1);
	write_uint8(SF_FMT, SF_HIT_DATA | SF_HIT_CODE);
	write_uint8(SF_INS, 0x01);
	 ((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = status_reg;	
	sflash_wait_free();
#endif	
}

static void pd_detect_lsr(UINT32 param)
{
#define PDCNT	3
	static INT32 pd_cnt = PDCNT;
	if(pd_cnt>0)
		pd_cnt --;
	if(pd_cnt==0)
	{	
	#ifdef _DEBUG_VERSION_	
		#ifndef NO_POWERDOWN_TESTING
		powerdown_test();
	 	#endif
	#endif 
		pd_cnt = PDCNT;
	}
}

void sflash_qio_boardset(UINT8 board_support_qio)
{
	qio_enable = board_support_qio;
}

void sflash_set_io(UINT8 io_num, UINT8 fls_idx)
{
	UINT32 board_support_qio = qio_enable;
	UINT32 strap_pin = 0;
	UINT8 tmp = 0;

	if((2!=io_num)&&(4!=io_num))//Not multi IO flash, use old sflash controller
	{
		if(sys_ic_is_M3202()&&(sys_ic_get_rev_id()>IC_REV_1))
		{
			strap_pin = (*((volatile unsigned long *)0xb8000070))&0xb000;
			if(0xb000==strap_pin)
			{
				strap_pin = 0;
				strap_pin |= (0x06<<11);	// bit11-14 flash size,
				*((volatile unsigned long *)0xb8000074) = strap_pin|(0x01<<26); //bit26 trigger
			}
		}
		if((ALI_M3329E==sys_ic_get_chip_id())&&(sys_ic_get_rev_id()>IC_REV_2)
		   ||ALI_M3327C==sys_ic_get_chip_id())
		{
			tmp = *((volatile UINT8 *)0xb8000025);
			if(tmp&0x1)
			{
				tmp &= 0xfe;
				*((volatile UINT8 *)0xb8000025) = tmp;
			}
		}
		if(ALI_S3602F==sys_ic_get_chip_id()||ALI_S3811==sys_ic_get_chip_id()||sys_ic_is_3701c())
			*((volatile UINT8 *)0xb800008d) &= 0xfe;
		return;
	}	
	if(4==io_num&&0==board_support_qio)//If board only support 2IO, then use 2 IO mode
		io_num = 2;
	if(sys_ic_is_M3202()&&(sys_ic_get_rev_id()>IC_REV_1))
	{
		strap_pin = (*((volatile unsigned long *)0xb8000070))&0xb000;
		if(0xb000!=strap_pin)
		{
		#if 0//IC bug, will cause CPU hange up
			strap_pin = 0;
			strap_pin |= (0x0e<<11);	// bit11-14 flash size,
			*((volatile unsigned long *)0xb8000074) = strap_pin|(0x01<<26); //bit26 trigger
		#else
			return;
		#endif	
		}
		sqi_ctrl_rdy = 1;
	}
	if((ALI_M3329E==sys_ic_get_chip_id())&&(sys_ic_get_rev_id()>=IC_REV_5)
	   ||ALI_M3327C==sys_ic_get_chip_id())
	{
		tmp = *((volatile UINT8 *)0xb8000025);
		if(!(tmp&0x1))
		{
		#if 1//IC bug, will cause CPU hange up. Bug solved from S3329E5
			tmp |= 1;
			*((volatile UINT8 *)0xb8000025) = tmp;
		#else
			return;
		#endif
		}
		sqi_ctrl_rdy = 1;
	}
	if(ALI_S3602F==sys_ic_get_chip_id())
	{
    #ifndef M3603_NORMAL_LINUX
		*((volatile UINT8 *)0xb800008d) |= 0x1;
		sqi_ctrl_rdy = 1;
    #endif     
	}
	if(ALI_S3811==sys_ic_get_chip_id()) 
	{
		*((volatile UINT8 *)0xb800008d) |= 0x1;//SPI  error function select
		sqi_ctrl_rdy = 1;
	}
	MUTEX_ENTER();
	if(sqi_ctrl_rdy)
	{
		if(2==io_num)
		{
			sflash_default_read = DIO_READ;
			sflash_default_mode = DIO_MODE;
			if(sflash_devtp == 0x30EF)//W25X
			{
				sflash_default_read = (0x3b|((SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA|SF_HIT_DUMM)<<8));
				sflash_default_mode = 1;
				write_uint8(SF_CFG, (read_uint8(SF_CFG)&0xf0)|0x2);
			}

		}
		else
		{
			sflash_default_read = QIO_READ;
			sflash_default_mode = QIO_MODE;	
			sflash_write_enable(1);
			write_uint8(SF_FMT, SF_HIT_DATA | SF_HIT_CODE);
			write_uint8(SF_INS, 0x01);
			if(QIO_MODE==sflash_default_mode)
			{
				if(sflash_devtp == 0x40EF)
				{
					((volatile UINT16 *)SYS_FLASH_BASE_ADDR)[0] = 0x0200;
					//pp_ins = 0x32; //Wibond 4pp not supported
				}else if (sflash_devtp == 0x24C2||sflash_devtp == 0x5EC2)
				{
					((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0x40;
					pp_ins = 0x38;
				}
                sflash_wait_free();//for four-line mode need wait sflash ready
			}    		
		}
	}
	/* Reset sflash to common read mode */
	write_uint16(SF_INS, sflash_default_read);
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|sflash_default_mode);
	MUTEX_LEAVE();
}

void sflash_get_id(UINT32 *result, UINT32 cmdaddr)
{
	if(ALI_M3329E==sys_ic_get_chip_id()&&sys_ic_get_rev_id()>=IC_REV_5)
		sflash_reg_addr |= 0x8000;
 	else
  		if(ALI_S3602F==sys_ic_get_chip_id()||ALI_S3811==sys_ic_get_chip_id()||sys_ic_is_3701c())
  			sflash_reg_addr = 0xb802e000;
#if 0 // modified for u-boot David.Chen@20120830
	UINT32 tmp = 0;
#ifdef SYS_SFLASH_FAST_READ_SUPPORT
	sflash_default_read = FST_READ;
	sflash_default_mode = FST_MODE;
#else
	tmp = sys_ic_get_dram_clock();
	if(tmp>135)//mem clk is higher than 135M, 
	{
		tmp = *((volatile UINT32 *)0xb8000098);
		tmp = (tmp&0xf000000)>>24;
		tmp = 2*(tmp+1);
			
		if(tmp<5)
		{
			sflash_default_read = FST_READ;
			sflash_default_mode = FST_MODE;
		}
	}
#endif
#endif

	if(m_EnableSoftProtection)
	{
		m_EnableSoftProtection = 0;
		#ifdef _DEBUG_VERSION_	
       	#ifndef NO_POWERDOWN_TESTING
		powerdown_test_init();//sflash_soft_protect_init();
		osal_interrupt_register_lsr(7, pd_detect_lsr, 0);
		#endif
		#endif
	}
#ifdef _DEBUG_VERSION_	
       #ifndef NO_POWERDOWN_TESTING
	else
	{
		powerdown_test_init();
		osal_interrupt_register_lsr(7, pd_detect_lsr, 0);
	}
	#endif
#endif
	MUTEX_ENTER();
	/* Reset sst26 spi mode*/	
	write_uint8(SF_FMT, SF_HIT_CODE);
	write_uint8(SF_INS, 0xff);
	((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0;
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	/* 1. Try ATMEL format get ID command */
	/* Set SFLASH_INS to RDID instruction. */
	write_uint8(SF_INS, 0x9f);
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|STD_MODE);
	/* Read from any address of serial flash. */
	if(m_EnableSoftProtection)
	{
        	result[0] = ((volatile UINT32 *)unpro_addr_low)[0];
	}
	else
	{
        	result[0] = ((volatile UINT32 *)SYS_FLASH_BASE_ADDR)[0];
	}

	/* 2. Try EON format get ID command */
	/* Set SFLASH_INS to RDID instruction. */
	write_uint8(SF_INS, 0xAB);
	/* Read from any address of serial flash. */
    	if(m_EnableSoftProtection)
    	{
        	result[1] = ((volatile UINT32 *)unpro_addr_low)[0];
    	}
	else
    	{
        	result[1] = ((volatile UINT32 *)SYS_FLASH_BASE_ADDR)[0];
	}
#ifdef SUPPORT_GLOBAL_PROTECT	
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA|SF_HIT_DUMM|0x20);
	write_uint8(SF_INS, 0x90);
	if(m_EnableSoftProtection)
    	{
        	result[2] = ((volatile UINT32 *)unpro_addr_low)[0];
    	}else
    	{
        	result[2] = ((volatile UINT32 *)SYS_FLASH_BASE_ADDR)[0];
	}
	{
		unsigned long i;
		global_protect_enable = 0;
		for(i=0; i<(sizeof(gp_en_list)/(4*2));i++)
		{
			if(result[0]==gp_en_list[i*2]&&result[2]==gp_en_list[i*2+1])
				global_protect_enable = 1;
		}
	}
#endif	
	if( (((result[0]>>0)&0xFF)==0xBF && ((result[0]>>8)&0xFF)==0x25) ||
	    (((result[0]>>0)&0xFF)==0x8C && (((result[0]>>8)&0xFF)==0x20 || ((result[0]>>8)&0xFF)==0x21)))
		aai_copy_enable = 1;
	else
		aai_copy_enable = 0;

	sflash_devtp = (short)(result[0]&0xffff);
	/* Remove all protection bits */
	/* Now only ATMEL26 and SST25VF with default protection */
	/* Sector unprotection dedicated from Atmel is unused */

	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	write_uint8(SF_INS, 0x05);
	if(((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] & 0x3c){
		sflash_write_enable(1);
		write_uint8(SF_FMT, SF_HIT_DATA | SF_HIT_CODE);
		write_uint8(SF_INS, 0x01);
	    	((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0x00;
		sflash_wait_free();
	};	
	sflash_global_protect(1);
	/* Reset sflash to common read mode */
	write_uint16(SF_INS, sflash_default_read);
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|sflash_default_mode);
	MUTEX_LEAVE();
}



int sflash_erase_chip(void)
{
	int i, sectors;
	unsigned long sec_addr;
	struct flash_private *tp;
	MUTEX_ENTER();
	sflash_global_protect(0);
	sflash_write_enable(1);
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE);
	/* Set SFLASH_INS to SE instruction. */
	write_uint8(SF_INS, 0xC7);
	
	/* Write to any address of serial flash. */
	if(m_EnableSoftProtection)
	{
		((volatile UINT8 *)unpro_addr_low)[0] = 0;
	}
	else
	{
		((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0;
	}

	sflash_wait_free();
	sflash_global_protect(1);
	MUTEX_LEAVE();
	return SUCCESS;
}

int sflash_erase_sector(UINT32 sector_addr)
{
	unsigned char data = 0;
	unsigned long flash_base_addr = SYS_FLASH_BASE_ADDR;

	/*Support M3329E and M3202 SFlash addr mapping in 8M or 16M*/
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202()||ALI_S3602F==sys_ic_get_chip_id()||ALI_S3811==sys_ic_get_chip_id()||sys_ic_is_3701c())
	{
		flash_base_addr = SYS_FLASH_BASE_ADDR - (sector_addr&0xc00000);
		sector_addr &= 0x3fffff;
	}
	MUTEX_ENTER();
	sflash_global_protect(0);
	sflash_write_enable(1);
	/* Set CODE_HIT and ADDR_HIT to 1, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR);
	/* Set SFLASH_INS to SE instruction. */
	
	/* Write to typical sector start address of serial flash. */
	if(m_EnableSoftProtection&&(sflash_reg_addr == SF_BASE_ADDR))
	{
		sflash_write_soft_protect(sector_addr,&data,1, 0xd8);
	}
	else
	{
	#ifdef _DEBUG_VERSION_
	#ifndef NO_POWERDOWN_TESTING	
		powerdown_test();
	#endif
	#endif
		write_uint8(SF_INS, 0xd8);
		((volatile UINT8 *)flash_base_addr)[sector_addr] = 0;
	};

	sflash_wait_free();
    	sflash_global_protect(1);
	MUTEX_LEAVE();	
	return SUCCESS;
}

int sflash_copy(UINT32 flash_addr, UINT8 *data, UINT32 len)
{
	UINT32 i;
	UINT32 end_cnt;
	UINT32 flash_base_addr = SYS_FLASH_BASE_ADDR;
	UINT32 rem_len = 0;
	if(!len)
		return SUCCESS;
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202()||ALI_S3602F==sys_ic_get_chip_id()||ALI_S3811==sys_ic_get_chip_id()||sys_ic_is_3701c())
	{
		UINT32 cur_seg = (flash_addr&0xc00000)>>22;
		UINT32 tge_seg = ((flash_addr+len-1)&0xc00000)>>22;
		if(cur_seg!=tge_seg)
		{
			UINT32 inter_seg_len;

			inter_seg_len = ((cur_seg+1)<<22) - flash_addr;
			if(SUCCESS!=sflash_copy(flash_addr, data, inter_seg_len))
					return !SUCCESS;
			flash_addr += inter_seg_len;
			data += inter_seg_len;
			len -= inter_seg_len;
			cur_seg++;
			
			while(tge_seg != cur_seg)
			{
				if(SUCCESS!=sflash_copy(flash_addr, data, 0x400000))
					return !SUCCESS;
				flash_addr += 0x400000;
				data += 0x400000;
				len -= 0x400000;
				cur_seg++;
			}
			
			return sflash_copy(flash_addr, data, len);
		}
		else
		{
			flash_base_addr = SYS_FLASH_BASE_ADDR - (flash_addr&0xc00000);
			flash_addr &= 0x3fffff;
		}
	}
	MUTEX_ENTER();
        sflash_global_protect(0);
	#ifdef SYS_PIN_MUX_MODE_04
	{
		// CONT-Write NOT support now!
		for(i = 0; i < len; i=i+1)
		{
			sflash_write_enable(1);
			/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
			/* Set SFLASH_INS to PP. */
    			/* Write to typical address of serial flash. */
			if(m_EnableSoftProtection&&(sflash_reg_addr == SF_BASE_ADDR))
    			{
    				sflash_write_soft_protect(flash_addr + i,data,1, 0x02);data++;
    			}
			else
			{
			#ifdef _DEBUG_VERSION_
			#ifndef NO_POWERDOWN_TESTING	
				powerdown_test();
			#endif
			#endif
    				write_uint8(SF_INS, 0x02);
				((volatile UINT8 *)flash_base_addr)[flash_addr + i] =*data++;
				//osal_delay(50000);
    			}
    			sflash_wait_free();
			//sflash_write_enable(0);
		}
		sflash_global_protect(1);
		MUTEX_LEAVE();
		return SUCCESS;
	}
	#else
 	if(aai_copy_enable) 
 	{
		//align addresses
		if((flash_addr&0x1)!=0) 
		{
   			sflash_write_enable(1);
    			/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
    			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
			if(m_EnableSoftProtection&&(sflash_reg_addr == SF_BASE_ADDR))
   				sflash_write_soft_protect(flash_addr,data,1, 0x02);
			else
    			{
    			#ifdef _DEBUG_VERSION_
			#ifndef NO_POWERDOWN_TESTING		
    				powerdown_test();
			#endif
			#endif	
    				/* Set SFLASH_INS to PP. */
    				write_uint8(SF_INS, 0x02);
    				/* Write to typical address of serial flash. */
				write_uint8(flash_base_addr + flash_addr, *data);
			}
			sflash_wait_free();
			osal_delay(50) ;  // for ESMT F25L016PA
			flash_addr ++; 	
			data ++; 
			len--;
		}
		//start AAI on aligned address with even length
		if(len>=2)
		{
			//start AAI mode
			sflash_write_enable(1);
			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA | SF_HIT_ADDR);
		
			write_uint8(SF_INS, 0xAD);
	   		*((unsigned short *)(flash_base_addr + flash_addr)) = (*data)|(*(data+1))<<8;

	   		flash_addr += 2;
	   		data +=2;
	   		len -=2;

		  	sflash_wait_free();
		   	//continue AAI
		   	end_cnt = len /2 ;
		   	len = len%2;
		   	for(i=0;i<end_cnt;i++)
			{
				write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
				if(m_EnableSoftProtection&&(sflash_reg_addr == SF_BASE_ADDR))
		   			sflash_write_soft_protect(flash_addr,data,2, 0xad);
				else
				{
				#ifdef _DEBUG_VERSION_
				#ifndef NO_POWERDOWN_TESTING	
					powerdown_test();
				#endif
				#endif
					write_uint8(SF_INS, 0xAD);
		   			*((unsigned short *)(flash_base_addr + flash_addr)) = (*data)|(*(data+1))<<8;
				}
		   	   	flash_addr += 2; data +=2;
		  		sflash_wait_free();
				osal_delay(50);  // for ESMT F25L016PA
		   	}

			//terminate AAI mode by issuing WRDI command
			sflash_write_enable(0);
		   	//wait AAI finished
			sflash_wait_free();
		}

		//any byte left ?
		if(len!=0) 
		{
	   		sflash_write_enable(1);
	    		/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
			if(m_EnableSoftProtection&&(sflash_reg_addr == SF_BASE_ADDR))
				sflash_write_soft_protect(flash_addr,data,1, 0x02);
			else
			{
			#ifdef _DEBUG_VERSION_
			#ifndef NO_POWERDOWN_TESTING	
				powerdown_test();
			#endif
			#endif
				/* Set SFLASH_INS to PP. */
				write_uint8(SF_INS, 0x02);
				/* Write to typical address of serial flash. */
				write_uint8(flash_base_addr + flash_addr, *data);
			}
			sflash_wait_free();
		}
		sflash_global_protect(1);
		MUTEX_LEAVE();
		return SUCCESS;
	}

	if (flash_addr & 0x03)
	{
		for(i = 0; i < (4 - (flash_addr & 0x03)) && len > 0; i++)
		{
			sflash_write_enable(1);
			/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
			if(QIO_MODE==sflash_default_mode && sflash_devtp != 0x40EF)
				write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|QIO_MODE);
			/* Set SFLASH_INS to PP. */
			/* Write to typical address of serial flash. */
			if(m_EnableSoftProtection)
			{
				sflash_write_soft_protect(flash_addr + i,data,1, pp_ins);data++;
			}
			else
			{
			#ifdef _DEBUG_VERSION_
			#ifndef NO_POWERDOWN_TESTING	
				powerdown_test();
			#endif
			#endif
				write_uint8(SF_INS, pp_ins);
				((volatile UINT8 *)flash_base_addr)[flash_addr + i] = *data++;
			}
			len--;
			sflash_wait_free();
		};
		flash_addr += (4 - (flash_addr & 0x03));
    	}
	
	rem_len = len&0x3;
	len = len&(~0x3);
	
	for (i = 0; len > 0; flash_addr += end_cnt, len -= end_cnt)
	{
		end_cnt = ((flash_addr + 0x100) & ~0xff) - flash_addr;	/* Num to align */
		end_cnt = end_cnt > len ? len : end_cnt;
		sflash_write_enable(1);
		/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_CONT_WR);
		/* Set SFLASH_INS to PP. */
		
		//Special patch for s-flash chip from ESI
		if(ALI_M3329E==sys_ic_get_chip_id()&&sys_ic_get_rev_id()<=IC_REV_2)
		{
			write_uint8(sflash_reg_addr+0x94, (end_cnt&0xff));
			write_uint8(sflash_reg_addr+0x95, ((end_cnt>>8)&0xff));
			write_uint8(sflash_reg_addr+0x96,  read_uint8(sflash_reg_addr+0x96)|1);
		}
		
		if(QIO_MODE==sflash_default_mode && sflash_devtp != 0x40EF)
			write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|QIO_MODE);
			/* Write to typical address of serial flash. */
		if(m_EnableSoftProtection&&(sflash_reg_addr == SF_BASE_ADDR))
		{
			sflash_write_soft_protect(flash_addr,data,end_cnt - 4, pp_ins); data+= (end_cnt - 4);
		}
		else
		{
		#ifdef _DEBUG_VERSION_
		#ifndef NO_POWERDOWN_TESTING	
			powerdown_test();
		#endif
		#endif
			write_uint8(SF_INS, pp_ins);
			if (((UINT32)data&3) == 0)
			{
				for (i = 0; i < end_cnt - 4; i=i+4)
				{
					*(volatile UINT32 *)((UINT32)(flash_base_addr+flash_addr + i))= *(volatile UINT32 *)((UINT32)data);
					data=data+4;
				}
			}
			else
			{
				for (i = 0; i < end_cnt - 4; i=i+4)
				{
					*(volatile UINT32 *)((UINT32)(flash_base_addr+flash_addr + i))= data[0]|(data[1]<<8)|(data[2]<<16)|(data[3]<<24);
					data=data+4;
				}
			}
		}
		if(sqi_ctrl_rdy)
			write_uint8(SF_DUM, read_uint8(SF_DUM)|0x20);
		if(m_EnableSoftProtection&&(sflash_reg_addr == SF_BASE_ADDR))
		{
			sflash_write_soft_protect(flash_addr + end_cnt - 4, data, 4, pp_ins); data +=  4;
		}
		else
		{
		#ifdef _DEBUG_VERSION_
		#ifndef NO_POWERDOWN_TESTING	
			powerdown_test();
		#endif
		#endif
			*(volatile UINT32 *)((UINT32)(flash_base_addr+flash_addr + end_cnt - 4)) = data[0]|(data[1]<<8)|(data[2]<<16)|(data[3]<<24);
		      	data=data+4;
		}
		if(sqi_ctrl_rdy)
			write_uint8(SF_DUM, read_uint8(SF_DUM)&(~0x20));
		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
		//Special patch for s-flash chip from ESI
		if(ALI_M3329E==sys_ic_get_chip_id()&&sys_ic_get_rev_id()<=IC_REV_2)
		{
			write_uint8(sflash_reg_addr+0x96,  read_uint8(sflash_reg_addr+0x96)&(~1));
		}
		sflash_wait_free();		
	};
	if(rem_len)
	{
		for(i = 0; i < rem_len; i++)
		{
			sflash_write_enable(1);
			/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
			if(QIO_MODE==sflash_default_mode && sflash_devtp != 0x40EF)
				write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|QIO_MODE);
			/* Set SFLASH_INS to PP. */
			/* Write to typical address of serial flash. */
			if(m_EnableSoftProtection&&(sflash_reg_addr == SF_BASE_ADDR))
			{
				sflash_write_soft_protect(flash_addr + i,data,1, pp_ins);data++;
			}
			else
			{
			#ifdef _DEBUG_VERSION_
			#ifndef NO_POWERDOWN_TESTING	
				powerdown_test();
			#endif
			#endif
				write_uint8(SF_INS, pp_ins);
				((volatile UINT8 *)flash_base_addr)[flash_addr + i] = *data++;
			}
			
			sflash_wait_free();
		}
		flash_addr += rem_len;
	}
	sflash_global_protect(1);
	MUTEX_LEAVE();
	return len != 0;
	#endif
}

int sflash_read(void *buffer, void *flash_addr, UINT32 len)
{
	UINT32 i, l, tmp;
	UINT32 flash_base_addr = SYS_FLASH_BASE_ADDR;
	UINT32 flash_offset = (UINT32)flash_addr-SYS_FLASH_BASE_ADDR;

	if(!len)
		return SUCCESS;
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202()||ALI_S3602F==sys_ic_get_chip_id()||ALI_S3811==sys_ic_get_chip_id()||sys_ic_is_3701c())
	{
		UINT32 cur_seg = (flash_offset&0xc00000)>>22;
		UINT32 tge_seg = ((flash_offset+len-1)&0xc00000)>>22;
		if(cur_seg!=tge_seg)
		{
			UINT32 inter_seg_len;

			inter_seg_len = ((cur_seg+1)<<22) - flash_offset;
			sflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), inter_seg_len);
			flash_offset += inter_seg_len;
			buffer = (void *)((UINT32)buffer+inter_seg_len);
			len -= inter_seg_len;
			cur_seg++;
			
			while(tge_seg != cur_seg)
			{
				sflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), 0x400000);
				flash_offset += 0x400000;
				buffer = (void *)((UINT32)buffer+0x400000);
				len -= 0x400000;
				cur_seg++;
			}
			
			return sflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), len);
		}
		else
		{
			flash_base_addr = SYS_FLASH_BASE_ADDR - (flash_offset&0xc00000);
			flash_offset &= 0x3fffff;
		}	
	}
	flash_addr = (void *)(flash_base_addr+flash_offset);
	
	MUTEX_ENTER();
      #ifndef SYS_PIN_MUX_MODE_04
	
	
	/* Read data in head not align with 4 bytes */
	if ((UINT32)flash_addr & 3)
	{
		l = 4 - ((UINT32)flash_addr & 3);
		l = l > len ? len : l;
		MEMCPY(buffer, flash_addr, l);
		buffer=(void *)((UINT32)buffer + l);
		flash_addr=(void *)((UINT32)flash_addr +l);
		len -= l;
	}
	/* Read data in body align with 4 bytes */
	if (len > 3)
	{
		write_uint16(SF_INS, sflash_default_read|(SF_CONT_RD<<8));
		/* Read data */
		if (((UINT32)buffer & 3) == 0)
		{
			for (i = 0; i < ((len&(~0x3)) - 4); i+=4)
			{
				*(UINT32 *)((UINT32)buffer + i) = *(UINT32 *)((UINT32)flash_addr + i);
			}
			if(sqi_ctrl_rdy)
				write_uint8(SF_DUM, read_uint8(SF_DUM)|0x10);
			*(UINT32 *)((UINT32)buffer + i) = *(UINT32 *)((UINT32)flash_addr + i);
			if(sqi_ctrl_rdy)
				write_uint8(SF_DUM, read_uint8(SF_DUM)&(~0x10));
		}
		else
		{
			for (i = 0; i < ((len&(~0x3)) - 4); i+=4)
			{
				tmp = *(UINT32 *)((UINT32)flash_addr + i);
				*(UINT8 *)((UINT32)buffer + i) = tmp & 0xff;
				*(UINT8 *)((UINT32)buffer + i + 1) = ((tmp >> 8) & 0xff);
				*(UINT8 *)((UINT32)buffer + i + 2) = ((tmp >> 16)& 0xff);
				*(UINT8 *)((UINT32)buffer + i + 3) = ((tmp >> 24) & 0xff);
			}
			if(sqi_ctrl_rdy)
				write_uint8(SF_DUM, read_uint8(SF_DUM)|0x10);
			tmp = *(UINT32 *)((UINT32)flash_addr + i);
			*(UINT8 *)((UINT32)buffer + i) = tmp & 0xff;
			*(UINT8 *)((UINT32)buffer + i + 1) = ((tmp >> 8) & 0xff);
			*(UINT8 *)((UINT32)buffer + i + 2) = ((tmp >> 16)& 0xff);
			*(UINT8 *)((UINT32)buffer + i + 3) = ((tmp >> 24) & 0xff);
			if(sqi_ctrl_rdy)
				write_uint8(SF_DUM, read_uint8(SF_DUM)&(~0x10));
		}
		flash_addr = (UINT8 *)flash_addr + (len&(~0x3));
		buffer = (UINT8 *)buffer + (len&(~0x3));
		len -= (len&(~0x3));
		write_uint16(SF_INS, sflash_default_read);
	}
	
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
	sflash_wait_free();
	#endif
	MEMCPY(buffer, flash_addr, len);
	MUTEX_LEAVE();
	return SUCCESS;
}

int sflash_verify(unsigned int flash_addr, unsigned char *data, unsigned int len)
{

	unsigned char dst;
	unsigned char src;
	int ret = SUCCESS;
	UINT32 flash_base_addr = SYS_FLASH_BASE_ADDR;

	if(!len)
		return ret;
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202()||ALI_S3602F==sys_ic_get_chip_id()||ALI_S3811==sys_ic_get_chip_id()||sys_ic_is_3701c())
	{
		UINT32 cur_seg = (flash_addr&0xc00000)>>22;
		UINT32 tge_seg = ((flash_addr+len-1)&0xc00000)>>22;
		if(cur_seg!=tge_seg)
		{
			UINT32 inter_seg_len;

			inter_seg_len = ((cur_seg+1)<<22) - flash_addr;
			ret = sflash_verify(flash_addr, data, inter_seg_len);
			if(SUCCESS!=ret)
					return ret;
			flash_addr += inter_seg_len;
			data += inter_seg_len;
			len -= inter_seg_len;
			cur_seg++;
			
			while(tge_seg != cur_seg)
			{
				ret = sflash_verify(flash_addr, data, 0x400000);
				if(SUCCESS!=ret)
					return ret;
				flash_addr += 0x400000;
				data += 0x400000;
				len -= 0x400000;
				cur_seg++;
			}
			
			return sflash_verify(flash_addr, data, len);
		}
		else
		{
			flash_base_addr = SYS_FLASH_BASE_ADDR - (flash_addr&0xc00000);
			flash_addr &= 0x3fffff;
		}
	}
	MUTEX_ENTER();
	write_uint16(SF_INS, sflash_default_read);
	if(sqi_ctrl_rdy)
		write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|sflash_default_mode);

	/* Read data */
	for (; len > 0; flash_addr++, len--)
	{
		dst = ((volatile UINT8 *)flash_base_addr)[flash_addr];
		src = *data++;
		dst ^= src;
		if (dst == 0)
			continue;
		ret = 1;
		if (dst & src)
		{
			ret = 2;
			break;
		}
	};
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
	sflash_wait_free();
	MUTEX_LEAVE();
	return ret;
}
