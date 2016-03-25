#include <osal/osal.h>
#include <hld/adr_hld_dev.h>
#include <hld_cfg.h>
#include <hld/soc/adr_soc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <err/errno.h>
#include <sys/ioctl.h>                
#include <adr_version.h>
#include <ali_soc_common.h>


static INT32 g_ali_soc_fd = -1;
static unsigned char g_ali_soc_see_ver[SEE_VER_MAX_LEN] = "0";
static unsigned char g_ali_soc_debug = 0;
static struct soc_memory_map g_ali_soc_smm;
static int g_ali_soc_opened = 0;

#define SOC_PRINTF(fmt, args...)				\
{										\
	if (0 !=  g_ali_soc_debug)					\
	{									\
		ADR_DBG_PRINT(SOC, fmt, ##args);					\
	}									\
}

#define SOC_ERR_PRINTF(fmt, args...)		\
{										\
	ADR_DBG_PRINT(SOC, fmt, ##args);						\
}

#if 0
static UINT32 soc_relay_func(UINT32 cmd, void *fp)
{
    if(g_ali_soc_fd < 0)
    {
    	g_ali_soc_fd = open("/dev/ali_soc", O_RDONLY);
    	if (g_ali_soc_fd < 0)
    	    return RET_FAILURE;
    }
    return ioctl(g_ali_soc_fd, cmd, fp);
}                
#endif


INT32 soc_open(void)
{		
	if(g_ali_soc_opened == 1)
	{
		return SUCCESS;
	}
	
	if(g_ali_soc_fd < 0)
    	{		
	    	g_ali_soc_fd = open("/dev/ali_soc", O_RDONLY | O_CLOEXEC);
	    	if (g_ali_soc_fd < 0)
	    	{
	    		return RET_FAILURE;
	    	}	    	
    	}
	
	if (SUCCESS != ioctl(g_ali_soc_fd, ALI_SOC_GET_MEMORY_MAP, (UINT32 *)(&g_ali_soc_smm)))
	{
		SOC_ERR_PRINTF("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		perror("soc_get_memory_map");
	}

	if (SUCCESS != ioctl(g_ali_soc_fd, ALI_SOC_GET_SEE_VER, (UINT32 *)g_ali_soc_see_ver))
	{
		SOC_ERR_PRINTF("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		perror("");
	}


	g_ali_soc_opened = 1;

	return SUCCESS;
}


INT32 soc_close(void)
{
	if(g_ali_soc_opened == 0)
	{
		return SUCCESS;
	}
	if (g_ali_soc_fd > 0)
	{
		close(g_ali_soc_fd);
		g_ali_soc_fd = -1;
	}
	g_ali_soc_opened = 0;	
	

	return SUCCESS;
}


/*
 *
 *The API below specified for USER call!
 *
*/

INT32 soc_reboot_get_timer(UINT32 *time_exp, UINT32 *time_cur)
{
    struct reboot_timer paras={time_exp, time_cur};

    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_REBOOT_GET_TIMER, &paras);
}

void soc_enter_standby(unsigned int time_exp, unsigned int time_cur)
{
    struct boot_timer paras={time_exp, time_cur};

    if(g_ali_soc_opened == 0)
		soc_open();
	
    	ioctl(g_ali_soc_fd, ALI_SOC_ENTER_STANDBY, &paras);
}

void soc_get_bonding(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    	ioctl(g_ali_soc_fd, ALI_SOC_GET_BONDING,NULL);
}

UINT32 soc_get_product_id(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_PRODUCT_ID,NULL);
}

UINT32 soc_get_c3603_product(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_C3603_PRODUCT,NULL);
}

UINT32 soc_get_chip_id(void)
{	
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_CHIP_ID,NULL);
}

UINT32 soc_get_rev_id(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_REV_ID,NULL);
}

UINT32 soc_get_cpu_clock(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_CPU_CLOCK,NULL);
}

UINT32 soc_get_dram_clock(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_DRAM_CLOCK,NULL);
}


UINT32 soc_get_dram_size(void)  
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_DRAM_SIZE,NULL);
}


INT32 soc_get_usb_num(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_USB_NUM,NULL);
}

INT32 soc_get_ci_num(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_CI_NUM,NULL);
}

INT32 soc_get_tuner_num(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_TUNER_NUM,NULL);
}

INT32 soc_get_mac_num(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_MAC_NUM,NULL);
}

/////////////////////////////////////////////////////

INT32 soc_dram_scramble_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_MAC_NUM,NULL);
}

INT32 soc_io_security_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_MAC_NUM,NULL);
}

INT32 soc_split_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_MAC_NUM,NULL);
}

INT32 soc_uart_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_MAC_NUM,NULL);
}

INT32 soc_ejtag_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_MAC_NUM,NULL); 
}

INT32 soc_mv_is_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
   return ioctl(g_ali_soc_fd, ALI_SOC_MAC_NUM,NULL);
}

INT32 soc_ac3_is_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
     return ioctl(g_ali_soc_fd, ALI_SOC_MAC_NUM,NULL);
}

INT32 soc_ddplus_is_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_MAC_NUM,NULL);
}

INT32 soc_XD_is_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_MAC_NUM,NULL);
}

INT32 soc_XDplus_is_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
     return ioctl(g_ali_soc_fd, ALI_SOC_MAC_NUM,NULL);
}

INT32 soc_aac_is_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
     return ioctl(g_ali_soc_fd, ALI_SOC_MAC_NUM,NULL);
}


INT32 soc_h264_is_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
     return ioctl(g_ali_soc_fd, ALI_SOC_MAC_NUM,NULL);
}

INT32 soc_mp4_is_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_MAC_NUM,NULL);
}

/////////////////////////////////////////////////////

INT32 soc_get_hd_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_HD_ENABLED,NULL);
}

INT32 soc_hd_is_enabled(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_HD_IS_ENABLED,NULL);
}

INT32 soc_usb_port_enabled(UINT32 port)
{
    struct soc_usb_port usb_port = {port};

    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_USB_PORT_ENABLED,&usb_port);
}

INT32 soc_sata_enable(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_SATA_EANBLE,NULL);
}

INT32 soc_nim_support(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_NIM_SUPPORT,NULL);
}

INT32 soc_nim_m3501_support(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_NIM_M3501_SUPPORT,NULL);
}


INT32 soc_check_addr(UINT32 addr, UINT32 len)
{	
	UINT32 reg_base_addr = 0x18000000;
	UINT32 reg_end_addr = 0x1805ffff;
	
	
	SOC_PRINTF("[ %s %d ], addr = 0x%08x, len = 0x%08x\n", 
		__FUNCTION__, __LINE__, addr, len);
	
	if ((addr >= reg_base_addr) && ((addr + len) <= reg_end_addr))
	{		
	}
	else
	{
		SOC_ERR_PRINTF("[ %s %d ], addr 0x%08x or (addr + len) 0x%08x is invalid\n", __FUNCTION__, __LINE__, 
			addr, (addr + len));
		SOC_ERR_PRINTF("[ %s %d ], valid addr range : 0x%08x-0x%08x\n", 
			__FUNCTION__, __LINE__, reg_base_addr, reg_end_addr);
		
		return ERR_FAILURE;
	}	
	

	return SUCCESS;	
}


INT32 soc_read8(UINT32 addr, UINT8 *buf, UINT32 len)
{	
	struct soc_opt_paras8 reg_opt;	
	UINT32 i = 0;
	
	
    	if(g_ali_soc_opened == 0)
		soc_open();
	
	if (!buf)
	{
		SOC_ERR_PRINTF("[ %s %d ], buf is NULL\n", __FUNCTION__, __LINE__);
		
		return ERR_FAILURE;
	}		

	if (SUCCESS != soc_check_addr(addr, (len-1)))
	{
		return ERR_FAILURE;
	}		
	
	reg_opt.addr = addr;
	SOC_PRINTF("[%s %d ], read %d bytes form 0x%08x : ", __FUNCTION__, __LINE__, len, addr);	
	
	for (i=0; i<len; i++)
	{				
		reg_opt.data = ioctl(g_ali_soc_fd, ALI_SOC_READ8, &reg_opt);
		if (0 == (i % 16))
		{
			SOC_PRINTF("\n%08xh: ", reg_opt.addr);
		}
		SOC_PRINTF("0x%02x ", reg_opt.data);		
		
		reg_opt.addr += 1;		
		buf[i] = reg_opt.data;		
	}
	SOC_PRINTF("\n");	

	return SUCCESS;
}


INT32 soc_read16(UINT32 addr, UINT8 *buf, UINT32 len)
{	
	struct soc_opt_paras16 reg_opt;	
	UINT32 i = 0;
	
    	if(g_ali_soc_opened == 0)
		soc_open();
	
	if (!buf)
	{
		SOC_ERR_PRINTF("[ %s %d ], buf is NULL\n", __FUNCTION__, __LINE__);
		
		return ERR_FAILURE;
	}		

	if (SUCCESS != soc_check_addr(addr, (len * 2 - 1)))
	{
		return ERR_FAILURE;
	}		
	
	reg_opt.addr = addr;
	SOC_PRINTF("[%s %d ], read %d bytes form 0x%08x : ", __FUNCTION__, __LINE__, (len * 2), addr);	
	
	for (i=0; i<len; i++)
	{				
		reg_opt.data = ioctl(g_ali_soc_fd, ALI_SOC_READ16, &reg_opt);
		if (0 == (i % 8))
		{
			SOC_PRINTF("\n%08xh: ", reg_opt.addr);
		}
		SOC_PRINTF("0x%04x ", reg_opt.data);		
		
		reg_opt.addr += 2;		
		buf[i] = (UINT8)(reg_opt.data &0xff);		
		buf[i+1] = (UINT8)((reg_opt.data >> 8) &0xff);
	}
	SOC_PRINTF("\n");	


	return SUCCESS;
}


INT32 soc_read32(UINT32 addr, UINT8 *buf, UINT32 len)
{
	struct soc_opt_paras32 reg_opt;	
	UINT32 i = 0;
	
    	if(g_ali_soc_opened == 0)
		soc_open();
	
	if (!buf)
	{
		SOC_ERR_PRINTF("[ %s %d ], buf is NULL\n", __FUNCTION__, __LINE__);
		
		return ERR_FAILURE;
	}		

	if (SUCCESS != soc_check_addr(addr, (len * 4 - 1)))
	{
		return ERR_FAILURE;
	}		
	
	
	reg_opt.addr = addr;
	SOC_PRINTF("[%s %d ], read %d bytes form 0x%08x : ", __FUNCTION__, __LINE__, (len * 4), addr);	
	
	for (i=0; i<len; i++)
	{				
		reg_opt.data = ioctl(g_ali_soc_fd, ALI_SOC_READ32, &reg_opt);
		if (0 == (i % 4))
		{
			SOC_PRINTF("\n%08xh: ", reg_opt.addr);
		}
		SOC_PRINTF("0x%08x ", reg_opt.data);		
		reg_opt.addr += 4;		
		buf[i*4] = (UINT8)(reg_opt.data &0xff);
		buf[i*4+1] = (UINT8)((reg_opt.data >> 8) &0xff);
		buf[i*4+2] = (UINT8)((reg_opt.data >> 16) &0xff);
		buf[i*4+3] = (UINT8)((reg_opt.data >> 24) &0xff);		
	}
	SOC_PRINTF("\n");		


	return SUCCESS;
}


INT32 soc_write8(UINT32 addr, UINT8 *buf, UINT32 len)
{
	struct soc_opt_paras8 reg_opt;		
	

    	if(g_ali_soc_opened == 0)
		soc_open();
	
	if (!buf)
	{
		SOC_ERR_PRINTF("[ %s %d ], buf is NULL\n", __FUNCTION__, __LINE__);
		
		return ERR_FAILURE;
	}		

	if (SUCCESS != soc_check_addr(addr, (len - 1)))
	{
		return ERR_FAILURE;
	}	
	

	reg_opt.addr = addr;
	reg_opt.data = buf[0];	
	ioctl(g_ali_soc_fd, ALI_SOC_WRITE8, &reg_opt);		
	SOC_PRINTF("[ %s %d ], write *0x%08x = 0x%02x\n", __FUNCTION__, __LINE__, reg_opt.addr, reg_opt.data);	


	return SUCCESS;
}


INT32 soc_write16(UINT32 addr, UINT8 *buf, UINT32 len)
{
	struct soc_opt_paras16 reg_opt;		
	

    	if(g_ali_soc_opened == 0)
		soc_open();
	
	if (!buf)
	{
		SOC_ERR_PRINTF("[ %s %d ], buf is NULL\n", __FUNCTION__, __LINE__);
		
		return ERR_FAILURE;
	}		

	if (SUCCESS != soc_check_addr(addr, (len * 2 - 1)))
	{
		return ERR_FAILURE;
	}	
	

	reg_opt.addr = addr;
	reg_opt.data = (buf[1] << 8) | (buf[0]);	
	ioctl(g_ali_soc_fd, ALI_SOC_WRITE16, &reg_opt);	
	SOC_PRINTF("[ %s %d ], write *0x%08x = 0x%04x\n", __FUNCTION__, __LINE__, reg_opt.addr, reg_opt.data);	


	return SUCCESS;
}


INT32 soc_write32(UINT32 addr, UINT8 *buf, UINT32 len)
{
	struct soc_opt_paras32 reg_opt;	
	UINT32 i = 0;
	

    	if(g_ali_soc_opened == 0)
		soc_open();
	
	if (!buf)
	{
		SOC_ERR_PRINTF("[ %s %d ], buf is NULL\n", __FUNCTION__, __LINE__);
		
		return ERR_FAILURE;
	}		

	if (SUCCESS != soc_check_addr(addr, (len * 4 -1)))
	{
		return ERR_FAILURE;
	}	
	

	reg_opt.addr = addr;
	reg_opt.data = (buf[3] << 24) | (buf[2] << 16) |(buf[1] << 8) | (buf[0]);
	SOC_PRINTF("[ %s %d ], write *0x%08x = 0x%08x\n", __FUNCTION__, __LINE__, reg_opt.addr, reg_opt.data);	
	
	ioctl(g_ali_soc_fd, ALI_SOC_WRITE32, &reg_opt);	


	return SUCCESS;
}


INT32 soc_per_read32(UINT32 addr, UINT8 *buf, UINT32 len)
{
	struct soc_opt_paras32 reg_opt;	
	UINT32 i = 0;

	
    if(g_ali_soc_opened == 0)
		soc_open();
	
	if (!buf)
	{
		SOC_ERR_PRINTF("[ %s %d ], buf is NULL\n", __FUNCTION__, __LINE__);
		
		return ERR_FAILURE;
	}			
	
	
	reg_opt.addr = addr;
	SOC_PRINTF("[%s %d ], read %d bytes form 0x%08x : ", __FUNCTION__, __LINE__, (len * 4), addr);	
	
	for (i=0; i<len; i++)
	{				
		reg_opt.data = ioctl(g_ali_soc_fd, ALI_SOC_PER_READ32, &reg_opt);
		if (0 == (i % 4))
		{
			SOC_PRINTF("\n%08xh: ", reg_opt.addr);
		}
		SOC_PRINTF("0x%08x ", reg_opt.data);		
		reg_opt.addr += 4;		
		buf[i*4] = (UINT8)(reg_opt.data &0xff);
		buf[i*4+1] = (UINT8)((reg_opt.data >> 8) &0xff);
		buf[i*4+2] = (UINT8)((reg_opt.data >> 16) &0xff);
		buf[i*4+3] = (UINT8)((reg_opt.data >> 24) &0xff);		
	}
	SOC_PRINTF("\n");		


	return SUCCESS;
}


INT32 soc_per_write32(UINT32 addr, UINT8 *buf, UINT32 len)
{
	struct soc_opt_paras32 reg_opt;	
	UINT32 i = 0;
	

    if(g_ali_soc_opened == 0)
		soc_open();
	
	if (!buf)
	{
		SOC_ERR_PRINTF("[ %s %d ], buf is NULL\n", __FUNCTION__, __LINE__);
		
		return ERR_FAILURE;
	}			

	reg_opt.addr = addr;
	reg_opt.data = (buf[3] << 24) | (buf[2] << 16) |(buf[1] << 8) | (buf[0]);
	SOC_PRINTF("[ %s %d ], write *0x%08x = 0x%08x\n", __FUNCTION__, __LINE__, reg_opt.addr, reg_opt.data);	
	
	ioctl(g_ali_soc_fd, ALI_SOC_PER_WRITE32, &reg_opt);	


	return SUCCESS;
}


INT32 soc_get_memory_map(struct soc_memory_map *smm)
{
	if (NULL == smm)
	{
		SOC_ERR_PRINTF("[ %s %d ], param is NULL\n", __FUNCTION__, __LINE__);
		
		return ERR_FAILURE;
	}				

    if(g_ali_soc_opened == 0)
		soc_open();
	
	memcpy(smm, &g_ali_soc_smm, sizeof(g_ali_soc_smm));

	return SUCCESS;	
}

void soc_get_reserved_men_buf(UINT32 *buf_addr, UINT32 *buf_size)
{
	if ((NULL == buf_addr) || (NULL == buf_size))
	{
		SOC_ERR_PRINTF("[ %s %d ], param is NULL\n", __FUNCTION__, __LINE__);
		
		return;
	}				

    if(g_ali_soc_opened == 0)
		soc_open();
	
	*buf_addr = g_ali_soc_smm.reserved_mem_addr;
	*buf_size = g_ali_soc_smm.reserved_mem_size;	
}

void soc_get_media_buf(UINT32 *buf_addr, UINT32 *buf_size)
{
	if ((NULL == buf_addr) || (NULL == buf_size))
	{
		SOC_ERR_PRINTF("[ %s %d ], param is NULL\n", __FUNCTION__, __LINE__);
		
		return;
	}			

    if(g_ali_soc_opened == 0)
		soc_open();
	
	*buf_addr = g_ali_soc_smm.media_buf_addr;
	*buf_size = g_ali_soc_smm.media_buf_size;	
}


INT32 soc_set_level(UINT32 level)
{
	struct debug_level_paras paras;

    	if(g_ali_soc_opened == 0)
		soc_open();


	paras.level = level;	
    	return ioctl(g_ali_soc_fd, ALI_SOC_SET_DEBUG_LEVEL, &paras);	
}


INT32 soc_get_see_ver(UINT8 *buf)
{
	struct soc_opt_see_ver ver = {buf};
	INT32 ret = 0;

	if (NULL == buf)
	{
		SOC_ERR_PRINTF("[ %s %d ], buf is NULL\n", __FUNCTION__, __LINE__);
		
		return ERR_FAILURE;
	}		
	
    	if(g_ali_soc_opened == 0)
	{
		soc_open();	
	}		
	
	memcpy(buf, g_ali_soc_see_ver, sizeof(g_ali_soc_see_ver));		
	SOC_PRINTF("[ %s %d ], %s\n", __FUNCTION__, __LINE__, buf);		


	return SUCCESS;
}

INT32 soc_set_see_printf(INT32 enable)
{
	UINT32 disable = (enable) ? 0 : 1;

    if(g_ali_soc_opened == 0)
		soc_open();
	
	ioctl(g_ali_soc_fd, ALI_SOC_DISABLE_SEE_PRINTF, (UINT32)&disable);	
}

INT32 soc_hit_see_heart(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
	ioctl(g_ali_soc_fd, ALI_SOC_HIT_SEE_HEART, 0);

	return SUCCESS;
}

INT32 soc_enable_see_exception(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();
	
	ioctl(g_ali_soc_fd, ALI_SOC_ENABLE_SEE_EXCEPTION, 0);
}

INT32 soc_get_share_mem(enum SOC_SYS_SHM type, void **buf_addr, INT32 *buf_size)
{
	INT32 ret = ERR_FAILURE;
	UINT32 phy_addr = 0;
	INT32 len = 0;

	if(g_ali_soc_opened == 0)
		soc_open();
	
	if(type == SYS_SHM_AUTO)
	{
		phy_addr = g_ali_soc_smm.osd_bk;
		len = 0x3DE000;

		SOC_PRINTF("shared buf 0x%08x size 0x%08x\n", (INT32)phy_addr, (INT32)len);
		ret = SUCCESS;
	}	

	if(ret == SUCCESS)
	{
		*buf_addr = (void *)(phy_addr & 0x1FFFFFFF);
		*buf_size = len;		
	}

	return ret;
}


INT32 soc_reboot(void)
{
	struct soc_op_paras soc_par = {0,0,0};    

    if(g_ali_soc_opened == 0)
		soc_open();
	
    return ioctl(g_ali_soc_fd, ALI_SOC_REBOOT, &soc_par);	       
}


INT32 soc_dsc_access_ce_disable(void)
{
	if(g_ali_soc_opened == 0)
		soc_open();
	
	ioctl(g_ali_soc_fd, ALI_DSC_ACC_CE_DIS, 0);
}

INT32 soc_enable_power_down(INT32 power_down)
{
	if(g_ali_soc_opened == 0)
		soc_open();

	return ioctl(g_ali_soc_fd, ALI_SOC_ENABLE_POWER_DOWN,NULL);
}

INT32 soc_show_see_plugin_info(void)
{
    if(g_ali_soc_opened == 0)
		soc_open();

	return ioctl(g_ali_soc_fd, ALI_SOC_SHOW_SEE_PLUGIN_INFO,NULL);
}


