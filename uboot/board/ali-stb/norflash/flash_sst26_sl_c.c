/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: flash_sst26_sl_c.c
 *
 *  Description: Provide local serial flash raw operations.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2009.8.04   Wen   Liu   0.1.000  Initial
 ****************************************************************************/
#include "sto_dev.h"
#include "sto_flash.h"
#include "flash_data.h"
#include "flash.h"

#define DELAY_MS(ms)  		osal_task_sleep(ms)
#define DELAY_US(us)		osal_delay(us)

#define SF_BASE_ADDR		0xb8000000
#define	SF_INS			(SF_BASE_ADDR + 0x98)
#define	SF_FMT			(SF_BASE_ADDR + 0x99)
#define	SF_DUM			(SF_BASE_ADDR + 0x9A)
#define	SF_CFG			(SF_BASE_ADDR + 0x9B)

#define SF_HIT_DATA		0x01
#define SF_HIT_DUMM		0x02
#define SF_HIT_ADDR		0x04
#define SF_HIT_CODE		0x08
#define SF_CONT_RD		0x40
#define SF_CONT_WR		0x80
#define SF_LAST_READ  0x10
#define SF_LAST_WRITE 0x20
 
#define write_uint8(addr, val)	*((volatile UINT8 *)(addr)) = (val)
#define read_uint8(addr)	*((volatile UINT8 *)(addr))

void sflash_soft_protect_init()
{
}


static void sst26_wait_free()
{
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	/* Set SFLASH_INS to RDSR instruction */
	write_uint8(SF_INS, 0x0a);//0x05->0x0a to fix a hw bug
	/* Read from any address of serial flash. */
	while (1)
	{
		if((m_EnableSoftProtection && ((((volatile UINT8 *)unpro_addr_low)[0] & 0x80) == 0)) || 
			((!m_EnableSoftProtection) && ((((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] & 0x80) == 0)))
		break;
		osal_delay(10);
	}

	/* Reset sflash to common read mode */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_HIT_DUMM);
	write_uint8(SF_INS, 0x03);//0x0c->0x03 to fix a hw bug
}

static void sst26_write_enable(int en)
{
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE);
	/* Set SFLASH_INS to WREN instruction. */
	if (en)
		write_uint8(SF_INS, 0x06);
	else
		write_uint8(SF_INS, 0x02);//0x04->0x02 to fix a hw bug
	/* Write to any address of serial flash. */
	if(m_EnableSoftProtection)
	{
		((volatile UINT8 *)unpro_addr_low)[0] = 0;
	}else{
		((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0;
	};
}


void sst26_sf_get_id(UINT32 *result, UINT32 cmdaddr)
{
	int i;
	if(m_EnableSoftProtection)
	{
		sflash_soft_protect_init();
	}
#ifdef _DEBUG_VERSION_	
      #ifndef NO_POWERDOWN_TESTING
	else
		powerdown_test_init();
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
	/* Read from any address of serial flash. */
	if(m_EnableSoftProtection)
	{
        	result[0] = ((volatile UINT32 *)unpro_addr_low)[0];
	}else{
        	result[0] = ((volatile UINT32 *)SYS_FLASH_BASE_ADDR)[0];
	}

	/* 2. Try EON format get ID command */
	/* Set SFLASH_INS to RDID instruction. */
	write_uint8(SF_INS, 0xAB);
	/* Read from any address of serial flash. */
    	if(m_EnableSoftProtection)
    	{
        	result[1] = ((volatile UINT32 *)unpro_addr_low)[0];
    	}else
    	{
        	result[1] = ((volatile UINT32 *)SYS_FLASH_BASE_ADDR)[0];
	}

	/*Init sst26 sqi*/
	/*Enable sqi*/
	write_uint8(SF_FMT, SF_HIT_CODE);
	write_uint8(SF_INS, 0x38);
	((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0;
	
	/*Set SQI Controller Mode 4*/
	write_uint8(SF_DUM, 0x4);
	
	/*Reset Enable*/
	write_uint8(SF_FMT, SF_HIT_CODE);
	write_uint8(SF_INS, 0x66);
	((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0;
	osal_delay(1);
	/*Reset*/
	write_uint8(SF_INS, 0x99);
	((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0;
	/* Remove all protection bits */
	/* Write Block-Protection Register with ZERO*/
	sst26_write_enable(1);
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA | SF_CONT_WR); 
	write_uint8(SF_INS, 0x44);//0x42->0x44 to fix a hw bug
	for(i = 0; i < ((((result[0]>>16)&0xff) == 0x01)?5:9); i++)
		 ((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0;
	write_uint8(SF_DUM, read_uint8(SF_DUM)|0x20); 
	((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0;
	write_uint8(SF_DUM, read_uint8(SF_DUM)&~0x20); 	
	/*Set Burst Length*/
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	write_uint8(SF_INS, 0xc0);
	((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0x03;
	
	/* Reset sflash to common read mode */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_HIT_DUMM);
	write_uint8(SF_INS, 0x03);//0x0c->0x03 to fix a hw bug
	MUTEX_LEAVE(); 
}



int sst26_sf_erase_chip(void)
{

	return SUCCESS;
}

int sst26_sf_erase_sector(UINT32 sector_addr)
{
	unsigned char data = 0;

	sector_addr = 0xAC000000 + (0x3c00000^sector_addr);
	MUTEX_ENTER();
	sst26_write_enable(1);
	/* Set CODE_HIT and ADDR_HIT to 1, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR);
	/* Set SFLASH_INS to SE instruction. */
	
	/* Write to typical sector start address of serial flash. */
	if(m_EnableSoftProtection)
	{
		sflash_write_soft_protect(sector_addr,&data,1, 0xd1);//0xd8->0xd1 to fix a hw bug
	}else{
	#ifdef _DEBUG_VERSION_
	#ifndef NO_POWERDOWN_TESTING	
		powerdown_test();
	#endif
	#endif	
		write_uint8(SF_INS, 0xd1);//0xd8->0xd1 to fix a hw bug
		((volatile UINT8 *)sector_addr)[0] = 0;
	};

	sst26_wait_free();
  MUTEX_LEAVE();  
	return SUCCESS;
}

int sst26_sf_copy(UINT32 flash_addr, UINT8 *data, UINT32 len)
{
	UINT32 i;
	UINT32 end_cnt;
	UINT32 seg_addr, seg_len;
	int ret = SUCCESS;
	UINT32 rem_len = 0;
	if(!len)
		return SUCCESS;

	//Support absolute addr for old version(<4M/<16M)
	//Only support relative addr for new version	
	if((UINT32)flash_addr < 0x80000000)
	{
	for (; len > 0; flash_addr += seg_len, len -= seg_len)
	{
		seg_len = 0x400000 - ((UINT32)flash_addr&0x3fffff);
		seg_len = (seg_len > len) ? len : seg_len;
		seg_addr = 0xAC000000 + (0x3c00000^(UINT32)flash_addr);
		ret = sst26_sf_copy(seg_addr, data, seg_len);
		if(SUCCESS!=ret)
			return ret;
		data += seg_len;
	}
	return ret;		
	}
	MUTEX_ENTER();
	if (flash_addr & 0x03)
	{
		for(i = 0; i < (4 - (flash_addr & 0x03)) && len > 0; i++)
		{
		       
			sst26_write_enable(1);
			/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
			/* Set SFLASH_INS to PP. */
			/* Write to typical address of serial flash. */
			if(m_EnableSoftProtection)
			{
				sflash_write_soft_protect(flash_addr + i,data,1, 0x04);data++;
			}else{
			#ifdef _DEBUG_VERSION_
			#ifndef NO_POWERDOWN_TESTING	
				powerdown_test();
			#endif
			#endif	
				write_uint8(SF_INS, 0x04);
				((volatile UINT8 *)flash_addr)[i] = *data++;
			};
			len--;
			sst26_wait_free();
		};
		flash_addr += (4 - (flash_addr & 0x03));
  };

	rem_len = len&0x3;
	len = len&(~0x3);

	for (; len > 0; len -= end_cnt)
	{
		end_cnt = ((flash_addr + 0x100) & ~0xff) - flash_addr;	/* Num to align */
		end_cnt = end_cnt > len ? len : end_cnt;
		sst26_write_enable(1);
		/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_CONT_WR);
		/* Set SFLASH_INS to PP. */
		
		/* Write to typical address of serial flash. */
		//Special patch for s-flash chip from ESI
		#ifdef _DEBUG_VERSION_
		#ifndef NO_POWERDOWN_TESTING	
			powerdown_test();
		#endif
		#endif	
			write_uint8(SF_INS, 0x04);
			
			for (i = 0; i < end_cnt - 4; i+=4,data+=4)
			{
				((volatile UINT32 *)flash_addr)[0] = data[0]|(data[1]<<8)|(data[2]<<16)|(data[3]<<24);
			};
			write_uint8(SF_DUM, read_uint8(SF_DUM)|0x20); 
			((volatile UINT32 *)flash_addr)[0] = data[0]|(data[1]<<8)|(data[2]<<16)|(data[3]<<24);
			data += 4;
			write_uint8(SF_DUM, read_uint8(SF_DUM)&~0x20); 		
		  sst26_wait_free();
		  flash_addr += end_cnt;
	};
	if(rem_len)
	{
		for(i = 0; i < rem_len; i++)
		{
			sst26_write_enable(1);
			/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
			/* Set SFLASH_INS to PP. */
			/* Write to typical address of serial flash. */
			if(m_EnableSoftProtection)
			{
				sflash_write_soft_protect(flash_addr + i,data,1, 0x04);data++;
			}else{
			#ifdef _DEBUG_VERSION_
			#ifndef NO_POWERDOWN_TESTING	
				powerdown_test();
			#endif
			#endif	
				write_uint8(SF_INS, 0x04);
				((volatile UINT8 *)flash_addr)[i] = *data++;
			};
			
			sst26_wait_free();
		}
		flash_addr += rem_len;
	}	
	MUTEX_LEAVE();
	return len != 0;
}

int sst26_sf_read(void *buffer, void *flash_addr, UINT32 len)
{
	UINT32 i;
	UINT32 end_cnt;
	UINT32 seg_addr, seg_len;
	int ret = SUCCESS;
	
	if(!len)
		return SUCCESS;
	//Support absolute addr for old version(<4M/<16M)
	//Only support relative addr for new version	
	if((UINT32)flash_addr < 0x80000000)
	{
	for (; len > 0; flash_addr += seg_len, len -= seg_len)
	{
		seg_len = 0x400000 - ((UINT32)flash_addr&0x3fffff);
		seg_len = (seg_len > len) ? len : seg_len;
		seg_addr = 0xAC000000 + (0x3c00000^(UINT32)flash_addr);
		ret = sst26_sf_read(seg_addr, buffer, seg_len);
		if(SUCCESS!=ret)
			return ret;
		buffer += seg_len;
	}
	return ret;		
	}
	//Read burst
	MUTEX_ENTER();
	for (i = 0; len > 0; flash_addr += end_cnt, len -= end_cnt)
	{
		end_cnt = (((UINT32)flash_addr + 0x40) & ~0x3f) - (UINT32)flash_addr;	/* Num to align */
		end_cnt = end_cnt > len ? len : end_cnt;
		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_HIT_DUMM | SF_CONT_RD);
		
		write_uint8(SF_INS, 0x03);
		for (i = 0; i < end_cnt - 1; i++)
		{
		        printf("-----***<sst26_sf_read> Line:370 ***-----i=0x%x,buffer=0x%x-----******-----\n",i,buffer);
#if 0 //the following will result build error in mips-linux-gnu-
			*((UINT8 *)buffer)++ = *((volatile UINT8 *)flash_addr);
#else
			*((UINT8 *)buffer) = *((volatile UINT8 *)flash_addr);
                     buffer++;
#endif
		};
		write_uint8(SF_DUM, read_uint8(SF_DUM)|0x10); 
#if 0
		*((UINT8 *)buffer)++ = *((volatile UINT8 *)flash_addr);
#else
		*((UINT8 *)buffer) = *((volatile UINT8 *)flash_addr);
                 buffer++;
#endif
		write_uint8(SF_DUM, read_uint8(SF_DUM)&~0x10); 
		sst26_wait_free();
	};
	MUTEX_LEAVE();
	return SUCCESS;
}

int sst26_sf_verify(unsigned int flash_addr, unsigned char *data, unsigned int len)
{

	unsigned char dst;
	unsigned char src;
	int ret = SUCCESS;
	unsigned int seg_addr,seg_len;

	if(!len)
		return ret;

	//Support absolute addr for old version(<4M/<16M)
	//Only support relative addr for new version	
	if((UINT32)flash_addr < 0x80000000)
	{
	for (; len > 0; flash_addr += seg_len, len -= seg_len)
	{
		seg_len = 0x400000 - ((UINT32)flash_addr&0x3fffff);
		seg_len = (seg_len > len) ? len : seg_len;
		seg_addr = 0xAC000000 + (0x3c00000^(UINT32)flash_addr);
		ret = sst26_sf_verify(seg_addr, data, seg_len);
		if(SUCCESS!=ret)
			return ret;
		data += seg_len;
	}
	return ret;		
	}
	MUTEX_ENTER();
	/* Read data */
	for (; len > 0; flash_addr++, len--)
	{
		dst = ((volatile UINT8 *)flash_addr)[0];
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
	sst26_wait_free();
	MUTEX_LEAVE();
	return ret;
}

