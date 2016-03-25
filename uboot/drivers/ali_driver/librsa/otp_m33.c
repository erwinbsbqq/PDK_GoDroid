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

#if 0
#define OTP_DEBUG(...)			do{}while(0)
#else
#define OTP_DEBUG				printf//soc_printf
//#define CHECK_CODEZONE			
#endif

#ifdef ALI_ARM_STB
#define OTP_MEM_BASE		0x18008200
#else
#define OTP_MEM_BASE		0xb8008200
#endif
#define OTP_MEM_SIZE		0x200

#define OTP_REG_TIM		0
#define OTP_REG_CTRL	4
#define OTP_REG_DATA	8
#define OTP_REG_WDATA	6

#define OTP_PROG_TRG	(1<<24)
#define OTP_PROG_BUSY	(1<<25)
#define OTP_READ_TRG	(1<<26)
#define OTP_READ_BUSY	(1<<27)
#define OTP_PROG_EN	(1<<31)

#define OTP_BLOCK_SIZE	64

#define OTP_TPOR	237// 237ns
#define OTP_TPW		310// 310us

#define OTP_CODEZONE_OFST		0x14

#ifdef ALI_ARM_STB
static UINT32 otp_reg_base = 	0x18042000;	
#else
static UINT32 otp_reg_base = 	0xb8042000;	
#endif
static UINT32 otp_ready = 0;
static UINT32 otp_direct_read = 0;
static UINT32 otp_charge_pump = 0;
static OSAL_ID otp_mutex_id = OSAL_INVALID_ID;
static OTP_CONFIG cfg_private;
#define OTP_WRITE32(reg, data)	(*((volatile UINT32 *)(otp_reg_base + reg)) = (data))
#define OTP_READ32(reg)			(*((volatile UINT32 *)(otp_reg_base + reg)))
#define OTP_WRITE8(reg, data)	(*((volatile UINT8 *)(otp_reg_base + reg)) = (data))
#define OTP_READ8(reg)			(*((volatile UINT8 *)(otp_reg_base + reg)))


#ifdef CHECK_CODEZONE
typedef UINT32(*CODEZONE_FUNC)(UINT32, UINT32);
CODEZONE_FUNC my_func = (CODEZONE_FUNC)(OTP_MEM_BASE+OTP_CODEZONE_OFST);
static UINT32 code_data[] =
{
	0x3c08b800, 	//lui		t0,0xb800
	0x35088200, 	//ori		t0,t0,0x8200
	0x8d02000c, 	//lw		v0,12(t0)
	0x00021027, 	//nor		v0,zero,v0
	0x00441024, 	//and		v0,v0,a0
	0x14020003, 	//bne		zero,v0, 80003910 <exit>
	0x8d090010, 	//lw		t1,16(t0)
	0x00094827, 	//nor		t1,zero,t1
	0x01251024, 	//and		v0,t1,a1
	0x03e00008, 	//jr		ra
	0x00000000 	//nop
};
#endif
INT32 otp_m33_read(UINT32 offset, UINT8 * buf, INT32 len);
INT32 otp_m33_write(UINT8 *buf, UINT32 offset, INT32 len);
INT32 otp_m33_lock(UINT32 offset, INT32 len);

static UINT8 obuf[OTP_MEM_SIZE];
static inline UINT32 sys_ic_get_dram_clock()
{
    return 166;
}

INT32 otp_m33_init(OTP_CONFIG * cfg)
{
	UINT32 chip_id = ALI_S3602F;
	UINT32 chip_ver =IC_REV_1;
	UINT8 t_tpw; 
	UINT8 t_tpor;
	UINT32 tmp;
	if(otp_ready)
	{
		OTP_DEBUG("OTP Bus Already Inited!\n");
		return RET_SUCCESS;
	}
	if(ALI_M3329E==chip_id&&chip_ver>=IC_REV_5)
	{
		if((1<<14)&(*((volatile UINT32 *)0xb8000000)))
		{
			otp_ready = 1;
			if(chip_ver>IC_REV_5)
			{
				otp_direct_read = 1;
				tmp = *((volatile UINT32 *)0xb8008100);
				tmp &= ~0x1;
				tmp |= (otp_direct_read&0x1);
				*((volatile UINT32 *)0xb8008100) = tmp;
			}
		}
	}
	if(ALI_S3602F==chip_id)
	{
		otp_ready = 1;
		otp_direct_read = 0;
		if(chip_ver >= IC_REV_1)
			otp_charge_pump = 1;
	}
		
	if(!otp_ready)
	{
		OTP_DEBUG("No OTP: chip %08x, ver %08x, bound %08x\n", chip_id, chip_ver, *((volatile UINT32 *)0xb8000000));
		return OTP_ERR_NOSUPPORT;
	}
	MEMSET((void *)(&cfg_private), 0, sizeof(OTP_CONFIG));
	if(cfg)
	{
		MEMCPY((void *)(&cfg_private), (void *)cfg, sizeof(OTP_CONFIG));
		if(cfg_private.vpp_by_gpio)
		{
			gpio_direction_output(cfg_private.vpp_position,!cfg_private.vpp_polar);
		//	HAL_GPIO_BIT_SET(cfg_private.vpp_position, !cfg_private.vpp_polar);
		//	HAL_GPIO_BIT_DIR_SET(cfg_private.vpp_position, cfg_private.vpp_io);
		}
		else if(NULL!=cfg_private.volctl_cb)
		{
			cfg_private.volctl_cb(OTP_VOLTAGE_1V8);
		}
		osal_delay(10000);
	}
	otp_mutex_id = osal_mutex_create();
	ASSERT(otp_mutex_id!=OSAL_INVALID_ID);
	tmp  = (OTP_TPW*sys_ic_get_dram_clock())/256+1;
	t_tpw = (UINT8)tmp;
	tmp 	= (OTP_TPOR*sys_ic_get_dram_clock())/1000+1;
	t_tpor = (UINT8)tmp;
	tmp = OTP_READ32(OTP_REG_TIM);
	tmp &= ~0xffff00;
	tmp |= ((t_tpw<<16)|(t_tpor<<8));
	OTP_WRITE32(OTP_REG_TIM, tmp);
	OTP_DEBUG("%s: d_r %d, tpw %d, tpor %d\n", __FUNCTION__, otp_direct_read, t_tpw, t_tpor);
#ifdef CHECK_CODEZONE
	if(otp_direct_read)
	{
		for(tmp = 0; tmp<sizeof(code_data); tmp += 4)
			if(code_data[tmp]!= *((UINT32 *)(OTP_MEM_BASE+OTP_CODEZONE_OFST+tmp)))
				break;
	
		if(tmp!=sizeof(code_data))
		{
			OTP_DEBUG("code zone not match: \n");
			for(tmp = 0; tmp<sizeof(code_data); tmp += 4)
				if(0!= *((UINT32 *)(OTP_MEM_BASE+OTP_CODEZONE_OFST+tmp)))
					break; 
			if(tmp==sizeof(code_data))
			{
				OTP_DEBUG("code zone burning: \n");
				otp_m33_write(code_data, OTP_CODEZONE_OFST, sizeof(code_data));
				for(tmp=0; tmp<1000; tmp++)
					osal_delay(1000);
				
			}
			else
			{
				for(tmp = 0; tmp<sizeof(code_data); tmp += 4)
					OTP_DEBUG("0x%08x: 0x%08x\n", code_data[tmp], *((UINT32 *)(OTP_MEM_BASE+OTP_CODEZONE_OFST+tmp)));
			}
			OTP_DEBUG("code zone 0: 0-> %08x\n", my_func(0, 0));
			OTP_DEBUG("code zone 0x5a5aa5a5: 0-> %08x\n", my_func(0x5a5aa5a5, 0));
			OTP_DEBUG("code zone 0: 0x5a5aa5a5-> %08x\n", my_func(0, 0x5a5aa5a5));
			my_func = (CODEZONE_FUNC)code_data;
			OTP_DEBUG("code data 0: 0-> %08x\n", my_func(0, 0));
			OTP_DEBUG("code data 0x5a5aa5a5: 0-> %08x\n", my_func(0x5a5aa5a5, 0));
			OTP_DEBUG("code data 0: 0x5a5aa5a5-> %08x\n", my_func(0, 0x5a5aa5a5));

		}
	}
#endif
	return RET_SUCCESS;
}

INT32 otp_m33_read(UINT32 offset, UINT8 * buf, INT32 len)
{
	UINT32 tmp_ofst;
	INT32 tmp_len;
	UINT32 tmp_data;
	INT32 i;
	INT32 read_len = 0;
	if((!otp_ready)||(!len)||((offset+len)>OTP_MEM_SIZE))
		return 0;
	OTP_DEBUG("%s: ofst %d, buf %08x, len %d\n", __FUNCTION__, offset, buf, len);
	osal_mutex_lock(otp_mutex_id, OSAL_WAIT_FOREVER_TIME);
	if(offset&0x3)
	{
		tmp_ofst = offset&(~0x3);
		if(otp_direct_read)
			tmp_data = *((volatile UINT32 *)(OTP_MEM_BASE+tmp_ofst));
		else
		{
			UINT32 tmo = 100;
			OTP_WRITE32(OTP_REG_CTRL, (tmp_ofst&0xffff));
			OTP_WRITE32(OTP_REG_CTRL, (tmp_ofst&0xffff)|OTP_READ_TRG);
			do
			{
				if(0==tmo)
				{
					osal_mutex_unlock(otp_mutex_id);
					OTP_DEBUG("otp read tmo: %d\n", __LINE__);
					return read_len;
				}
				tmo --;
				osal_delay(1);
			}while(OTP_READ32(OTP_REG_CTRL)&OTP_READ_BUSY);
			tmp_data = OTP_READ32(OTP_REG_DATA);
		}
		tmp_len= offset&0x3;
		for(i=0; i<tmp_len; i++)
			tmp_data = tmp_data>>8;
		tmp_len = 4-tmp_len;
		for(i=0; i<tmp_len; i++)
		{
			*buf = (UINT8)tmp_data;
			buf++;
			tmp_data = tmp_data>>8;
		}
		len -= tmp_len;
		read_len += tmp_len;
		offset = (offset&(~0x3))+4;
	}
	tmp_len = len&0x3;
	len = len-tmp_len;
	for(i=0; i<len; i+= 4)
	{
		if(otp_direct_read)
			tmp_data = *((volatile UINT32 *)(OTP_MEM_BASE+offset));
		else
		{
			UINT32 tmo = 100;
			OTP_WRITE32(OTP_REG_CTRL, (offset&0xffff));
			OTP_WRITE32(OTP_REG_CTRL, (offset&0xffff)|OTP_READ_TRG);
			do
			{
				if(0==tmo)
				{
					osal_mutex_unlock(otp_mutex_id);
					OTP_DEBUG("otp read tmo: ofst %d, num %d\n", offset, i);
					return read_len;
				}
				tmo --;
				osal_delay(1);
			}while(OTP_READ32(OTP_REG_CTRL)&OTP_READ_BUSY);
			tmp_data = OTP_READ32(OTP_REG_DATA);
		}
		offset += 4;
		read_len += 4;
		buf[0] = (UINT8)tmp_data;
		buf[1] = (UINT8)(tmp_data>>8);
		buf[2] = (UINT8)(tmp_data>>16);
		buf[3] = (UINT8)(tmp_data>>24);
		buf += 4;
	}
	if(tmp_len)
	{
		if(otp_direct_read)
			tmp_data = *((volatile UINT32 *)(OTP_MEM_BASE+offset));
		else
		{
			UINT32 tmo = 100;
			OTP_WRITE32(OTP_REG_CTRL, (offset&0xffff));
			OTP_WRITE32(OTP_REG_CTRL, (offset&0xffff)|OTP_READ_TRG);
			do
			{
				if(0==tmo)
				{
					osal_mutex_unlock(otp_mutex_id);
					OTP_DEBUG("otp read tmo: %d\n", __LINE__);
					return read_len;
				}
				tmo --;
				osal_delay(1);
			}while(OTP_READ32(OTP_REG_CTRL)&OTP_READ_BUSY);
			tmp_data = OTP_READ32(OTP_REG_DATA);
		}
		for(i=0; i<tmp_len; i++)
		{
			*buf = (UINT8)tmp_data;
			buf++;
			tmp_data = tmp_data>>8;
		}
		read_len += tmp_len;
	}
	osal_mutex_unlock(otp_mutex_id);
	return read_len;
}

INT32 otp_m33_write(UINT8 *buf, UINT32 offset, INT32 len)
{
	UINT32 tmo, tmp, j;
	INT32 i;
	INT32 write_len = 0;
	UINT8 data;
	
	if((0==cfg_private.vpp_by_gpio)&&(NULL==cfg_private.volctl_cb))
	{
		OTP_DEBUG("OTP Bus can not control program voltage!\n");
		//if (sys_ic_get_rev_id() == IC_REV_0)
		//	return 0;
	}
	if((!otp_ready)||(!len)||((offset+len)>OTP_MEM_SIZE))
	{
		return 0;
	}
	OTP_DEBUG("%s: buf %08x, ofst %d, len %d\n", __FUNCTION__, buf, offset, len);
	
	osal_mutex_lock(otp_mutex_id, OSAL_WAIT_FOREVER_TIME);
	
	if(otp_charge_pump == 1)
	{
		//Read original data from otp
		for(i = 0; i < len; i++)
		{
			tmo = 100;
			if(i == 0 || ((offset + i)&0x3) == 0)
			{
				OTP_WRITE32(OTP_REG_CTRL, ((offset + i)&0xfffc));
				OTP_WRITE32(OTP_REG_CTRL, ((offset + i)&0xfffc)|OTP_READ_TRG);
				do
				{
					if(0==tmo)
					{
						OTP_DEBUG("otp write read tmo: ofst %d, num %d\n", offset, i);
						goto otp_write_exit;
					}
					tmo --;
					osal_delay(1);
				}while(OTP_READ32(OTP_REG_CTRL)&OTP_READ_BUSY);
			}
			obuf[i] = (UINT8)(OTP_READ32(OTP_REG_DATA)>>(((offset + i)&0x3)<<3));
		}
	}
	
	OTP_WRITE32(OTP_REG_CTRL, OTP_PROG_EN);
	osal_delay(3);	
	if(cfg_private.vpp_by_gpio)
	{
		gpio_direction_output(cfg_private.vpp_position,cfg_private.vpp_polar);
//		HAL_GPIO_BIT_SET(cfg_private.vpp_position, cfg_private.vpp_polar);
	}
	else if(NULL!=cfg_private.volctl_cb)
	{
		cfg_private.volctl_cb(OTP_VOLTAGE_6V5);
	}else
	{
		if (otp_charge_pump == 1)
			*(volatile UINT32 *)(0xb80000b0) = ((*(volatile UINT32 *)(0xb80000b0))&(~0xf0000000))|0x60000000;
	}
	osal_delay(100);
	
	if (otp_charge_pump == 0)
	{
		for(i=0; i<len; i++)
		{
			tmo = OTP_TPW*2;
			data = ~(buf[i]);
			
			tmp = OTP_PROG_EN|(data<<16)|((offset+i)&0xffff);
			osal_delay(10);
			OTP_WRITE32(OTP_REG_CTRL, tmp);
			OTP_WRITE32(OTP_REG_CTRL, tmp|OTP_PROG_TRG);
			do
			{
				if(0==tmo)
				{
					OTP_DEBUG("otp prog tmo: ofst %d, num %d\n", offset, i);
					break;
				}
				tmo --;
				osal_delay(1);
			}while(OTP_READ32(OTP_REG_CTRL)&OTP_PROG_BUSY);
			if(0==tmo)
				break;
			//OTP_DEBUG("w: %08x, tmo %d\n", tmp, tmo);
			write_len ++;
		}
		osal_delay(10);
	}else
	{		
		for(i = 0; i < len; i++)
		{
			data = buf[i];				
			tmp = OTP_PROG_EN|((offset+i)&0xffff);
			//write otp bit by bit
			data ^= obuf[i];
			for(j = 1; j < (1<<8) ; j <<= 1)
			{
				if(!(data&j))
					continue;
				if(obuf[i]&j)
					continue; 
				obuf[i]  |= j;	
				OTP_WRITE32(OTP_REG_CTRL, tmp|(~obuf[i]<<16));
				OTP_WRITE32(OTP_REG_CTRL, tmp|(~obuf[i]<<16)|OTP_PROG_TRG);
				tmo = OTP_TPW*2;
				do
				{
					if(0==tmo)
					{
						OTP_DEBUG("otp prog tmo: ofst %d, num %d\n", offset, i);
						goto otp_write_exit;
					}
					tmo --;
					osal_delay(1);
				}while(OTP_READ32(OTP_REG_CTRL)&OTP_PROG_BUSY);
			}
			//OTP_DEBUG("w: %08x, tmo %d\n", tmp, tmo);
			write_len ++;
		}
	}
otp_write_exit:
	if(cfg_private.vpp_by_gpio)
	{
		gpio_direction_output(cfg_private.vpp_position,!cfg_private.vpp_polar);
		//HAL_GPIO_BIT_SET(cfg_private.vpp_position, !cfg_private.vpp_polar);
	}
	else if(NULL!=cfg_private.volctl_cb)
	{
		cfg_private.volctl_cb(OTP_VOLTAGE_1V8);
	}else
	{
		if (otp_charge_pump == 1)
			*(volatile UINT32 *)(0xb80000b0) = ((*(volatile UINT32 *)(0xb80000b0))&(~0xf0000000))|0x30000000;
	}
	
	osal_delay(10000);
	OTP_WRITE32(OTP_REG_CTRL, 0);
	osal_delay(5000);
	osal_mutex_unlock(otp_mutex_id);
	return write_len;
}

INT32 otp_m33_lock(UINT32 offset, INT32 len)
{
	UINT32 tmp, end;
	UINT32 tmo = OTP_TPW*2;
    	UINT32 chip_id = ALI_S3602F;
	UINT32 chip_ver =IC_REV_1;
	if(ALI_S3602F==chip_id)
		return;
	if((0==cfg_private.vpp_by_gpio)&&(NULL==cfg_private.volctl_cb))
	{
		OTP_DEBUG("OTP Bus can not control program voltage!\n");
		return RET_FAILURE;
	}
	if(!otp_ready)
		return OTP_ERR_NOSUPPORT;
	OTP_DEBUG("%s: ofst %d, len %d\n", __FUNCTION__, offset, len);
	osal_mutex_lock(otp_mutex_id, OSAL_WAIT_FOREVER_TIME);
	end = offset+len;
	offset = (offset+OTP_BLOCK_SIZE-1)&(~(OTP_BLOCK_SIZE-1));
	end &=~(OTP_BLOCK_SIZE-1);
	if(end>OTP_MEM_SIZE)
		end = OTP_MEM_SIZE;
	tmp = 0;
	for(; offset<end; offset+= OTP_BLOCK_SIZE)
		tmp |= 1<<(offset/OTP_BLOCK_SIZE);
	OTP_DEBUG("%s: lock bit %08x \n", __FUNCTION__, tmp);
	if(tmp)
	{
		UINT8 data = (UINT8)tmp;
		data = ~data;
		OTP_WRITE32(OTP_REG_CTRL, OTP_PROG_EN);
		osal_delay(3);
		if(cfg_private.vpp_by_gpio)
		{
			gpio_direction_output(cfg_private.vpp_position,cfg_private.vpp_polar);
			//HAL_GPIO_BIT_SET(cfg_private.vpp_position, cfg_private.vpp_polar);
		}
		else if(NULL!=cfg_private.volctl_cb)
		{
			cfg_private.volctl_cb(OTP_VOLTAGE_6V5);
		}
		osal_delay(100);
		tmp = data<<16;
		tmp |= OTP_PROG_EN;
		OTP_WRITE32(OTP_REG_CTRL, tmp);
		OTP_WRITE32(OTP_REG_CTRL, tmp|OTP_PROG_TRG);
		do
		{
			if(0==tmo)
			{
				OTP_DEBUG("otp lock prog tmo!\n");
				break;
			}
			tmo --;
			osal_delay(1);
		}while(OTP_READ32(OTP_REG_CTRL)&OTP_PROG_BUSY);
		osal_delay(10);
		if(cfg_private.vpp_by_gpio)
		{
			gpio_direction_output(cfg_private.vpp_position,!cfg_private.vpp_polar);
	//		HAL_GPIO_BIT_SET(cfg_private.vpp_position, !cfg_private.vpp_polar);
		}
		else if(NULL!=cfg_private.volctl_cb)
		{
			cfg_private.volctl_cb(OTP_VOLTAGE_1V8);
		}
		osal_delay(10000);
		OTP_WRITE32(OTP_REG_CTRL, 0);
		osal_delay(5000);
	}
	osal_mutex_unlock(otp_mutex_id);
	if(0==tmo)
		return OTP_ERR_LOCKTMO;
	return RET_SUCCESS;
}


#ifdef ALI_ARM_STB
#define OTP_BASE            0x18042000
#else
#define OTP_BASE            0xb8042000
#endif
#define OTP_READ_ADDR       0x4
#define OTP_READ_TRIG       0xc
#define OTP_READ_STATUS     0x10
#define OTP_READ_DATA       0x18

#define OTP_READ_TRIG_BIT   (0x100)
#define OTP_READ_STATUS_BIT   (0x100)
UINT32 m37_otp_read(UINT32 addr)
{
    UINT32 output;
    if((ALI_C3701 == sys_ic_get_chip_id() )||(ALI_S3921 == sys_ic_get_chip_id())){
        *(volatile UINT32 *)(OTP_BASE+OTP_READ_ADDR)  = (addr & 0x3ff) ;
        *(volatile UINT32 *)(OTP_BASE+OTP_READ_TRIG)  |= (OTP_READ_TRIG_BIT) ;
        do{
        }while((*(volatile UINT32 *)(OTP_BASE+OTP_READ_STATUS))&OTP_READ_STATUS_BIT);
        output = *(volatile UINT32 *)(OTP_BASE+OTP_READ_DATA);       
    }
    return output;
}

