#include <osal/osal.h>
//corei#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
//#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <sys_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <bus/otp/otp.h>

#include <ali_otp_common.h>
#if 0
#define OTP_DEBUG(...)				do{}while(0)
#else
#define OTP_DEBUG				printf 
#endif


static INT32 fd = -1;


#ifdef ADR_ALIDROID
unsigned long ali_atoi(const char * str)
{
	char *pstr = str;
	unsigned long result = 0;

	while ((unsigned int)(*pstr - '0')<10u)
	{
		result = result * 10 + *pstr - '0';
		pstr++;
	}

	return result;
}
#endif

static UINT32 otp_relay_func(UINT32 cmd, void *fp)
{
    if(fd < 0)
    {
    	fd = open("/dev/ali_otp", O_RDONLY);
    	if (fd < 0)
    	    return RET_FAILURE;
    }
    return ioctl(fd, cmd, fp);
}    

int ali_otp_read(unsigned long offset, unsigned char *buf, int len)
{
    struct otp_read_paras paras={offset,buf,len};
    
    return otp_relay_func(ALI_OTP_READ, &paras);
}

int ali_otp_write(unsigned char *buf, unsigned long offset, int len)
{
    struct otp_write_paras paras={buf,offset,len};
    return otp_relay_func(ALI_OTP_WRITE, &paras);
}

int ali_otp_lock(UINT32 offset, INT32 len)
{
    return RET_SUCCESS;
}

#define HW_MONITOR_EN 28
#define EJTAG_INTF_PASSWD_FLOW 27
#define BL_ENCRYPT_FLOW 24
#define OTP_FORCE_CE_KEY 23
#define OTP_CLOSE_USB 20
#define OTP_CLOSE_UART 12
#define OTP_CLOSE_EJTAG 11
#define SEE_BOOT_ROM_EN 8
#define BOOT_SECURITY_EN 1
#define DEBUG_PROTECT 0

/*
The following functions will be used for some HW security feature enable
*/
BOOL enable_boot_signature_check(void)
{
	UINT32 ret_len=0;
	UINT32 buf;
	UINT32 temp_buf;
	
	ret_len = ali_otp_read(0x3*4, &buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read OTP %08x Failed!\n", 0x3*4);
		return FALSE;
	}
	buf = (buf|(0x1<<BOOT_SECURITY_EN)) ;
	ret_len = ali_otp_write((UINT8 *)&buf ,  0x3*4, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Write OTP %08x Failed!\n", 0x3*4);
		return FALSE;
	}
	ret_len = ali_otp_read(0x3*4, &temp_buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read OTP %08x 2nd time Failed!\n", 0x3*4);
		return FALSE;
	}

	if( buf != temp_buf)// check if write otp success 
	{
		OTP_DEBUG("%s: Failed!\n", __FUNCTION__);
		return FALSE;
	}	

	OTP_DEBUG("%s: Success!\n",  __FUNCTION__);
	return TRUE;	
}

BOOL enable_boot_encrypt_flow (void)
{
	UINT32 ret_len=0;
	UINT32 buf;
	UINT32 temp_buf;

	ret_len = ali_otp_read(0x3*4, &buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read OTP %08x Failed!\n", 0x3*4);
		return FALSE;
	}
	buf = (buf|(0x1<<BL_ENCRYPT_FLOW)) ;
	ret_len = ali_otp_write((UINT8 *)&buf ,  0x3*4, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Write OTP %08x Failed!\n", 0x3*4);
		return FALSE;
	}
	ret_len = ali_otp_read(0x3*4, &temp_buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read OTP %08x 2nd time Failed!\n", 0x3*4);
		return FALSE;
	}

	if( buf != temp_buf)// check if write otp success 
	{
		OTP_DEBUG("%s: Failed!\n", __FUNCTION__);
		return FALSE;
	}	

	OTP_DEBUG("%s: Success!\n",  __FUNCTION__);
	return TRUE;	
}

BOOL enable_ejtag_protected (UINT8 mode)
{
	UINT32 ret_len=0;
	UINT32 buf = 0;
	UINT32 temp_buf;
	ret_len = ali_otp_read(0x3*4, &buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read OTP %08x Failed!\n", 0x3*4);
		return FALSE;
	}
	if(0==mode) //disable mode
		buf = (buf|(0x1<<OTP_CLOSE_EJTAG)) ;
	else if(1==mode)
		buf = (buf|(0x1<<DEBUG_PROTECT)|(0x1<<SEE_BOOT_ROM_EN)) ;
  //  else if((2==mode)&&(ALI_S3281 == sys_ic_get_chip_id()))
  //      buf = (buf|(0x1<<DEBUG_PROTECT)|(0x1<<EJTAG_INTF_PASSWD_FLOW));
    else
        return FALSE;
	ret_len = ali_otp_write((UINT8 *)&buf ,  0x3*4, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Write OTP %08x Failed!\n", 0x3*4);
		return FALSE;
	}
	ret_len = ali_otp_read(0x3*4, &temp_buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read OTP %08x 2nd time Failed!\n", 0x3*4);
		return FALSE;
	}

	if( buf != temp_buf)// check if write otp success 
	{
		OTP_DEBUG("%s mode %d: Failed!\n", __FUNCTION__, mode);
		return FALSE;
	}	

	OTP_DEBUG("%s mode %d: Success!\n",  __FUNCTION__, mode);
	return TRUE;	
}

BOOL enable_uart_protected (void)
{
	UINT32 ret_len=0;
	UINT32 buf;
	UINT32 temp_buf;
	
	ret_len = ali_otp_read(0x3*4, &buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read OTP %08x Failed!\n", 0x3*4);
		return FALSE;
	}
	buf = buf|(0x1<<OTP_CLOSE_UART) ;	
	ret_len = ali_otp_write((UINT8 *)&buf ,  0x3*4, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Write OTP %08x Failed!\n", 0x3*4);
		return FALSE;
	}
	ret_len = ali_otp_read(0x3*4, &temp_buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read OTP %08x 2nd time Failed!\n", 0x3*4);
		return FALSE;
	}

	if( buf != temp_buf)// check if write otp success 
	{
		OTP_DEBUG("%s: Failed!\n", __FUNCTION__);
		return FALSE;
	}	

	OTP_DEBUG("%s: Success!\n",  __FUNCTION__);
	return TRUE;	
}

BOOL enable_hw_monitor (void)
{
	UINT32 ret_len=0;
	UINT32 buf;
	UINT32 temp_buf;
  
	ret_len = ali_otp_read(0x3*4, &buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read OTP %08x Failed!\n", 0x3*4);
		return FALSE;
	}
	buf = buf|(0x1<<HW_MONITOR_EN) ;	
	ret_len = ali_otp_write((UINT8 *)&buf ,  0x3*4, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Write OTP %08x Failed!\n", 0x3*4);
		return FALSE;
	}
	ret_len = ali_otp_read(0x3*4, &temp_buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read OTP %08x 2nd time Failed!\n", 0x3*4);
		return FALSE;
	}

	if( buf != temp_buf)// check if write otp success 
	{
		OTP_DEBUG("%s: Failed!\n", __FUNCTION__);
		return FALSE;
	}	

	OTP_DEBUG("%s: Success!\n",  __FUNCTION__);
	return TRUE;	    
}

BOOL enable_cw_mandatory_mode(void)
{
	UINT32 ret_len=0;
	UINT32 buf;
	UINT32 temp_buf;
	
	ret_len = ali_otp_read(0x3*4, &buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read OTP %08x Failed!\n", 0x3*4);
		return FALSE;
	}
	buf = buf|(0x1<<OTP_FORCE_CE_KEY) ;	
	ret_len = ali_otp_write((UINT8 *)&buf ,  0x3*4, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Write OTP %08x Failed!\n", 0x3*4);
		return FALSE;
	}
	ret_len = ali_otp_read(0x3*4, &temp_buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read OTP %08x 2nd time Failed!\n", 0x3*4);
		return FALSE;
	}

	if( buf != temp_buf)// check if write otp success 
	{
		OTP_DEBUG("%s: Failed!\n", __FUNCTION__);
		return FALSE;
	}	

	OTP_DEBUG("%s: Success!\n",  __FUNCTION__);
	return TRUE;	
}

BOOL enable_usb_protected (UINT8 dev_id)
{
	UINT32 ret_len=0;
	UINT32 buf;
	UINT32 temp_buf;
	
	ret_len = ali_otp_read(0x3*4, &buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read OTP %08x Failed!\n", 0x3*4);
		return FALSE;
	}
	buf = buf|(0x1<<OTP_CLOSE_USB) ;
	ret_len = ali_otp_write((UINT8 *)&buf ,  0x3*4, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Write OTP %08x Failed!\n", 0x3*4);
		return FALSE;
	}
	ret_len = ali_otp_read(0x3*4, &temp_buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read OTP %08x 2nd time Failed!\n", 0x3*4);
		return FALSE;
	}

	if( buf != temp_buf)// check if write otp success 
	{
		OTP_DEBUG("%s dev %d: Failed!\n", __FUNCTION__, dev_id);
		return FALSE;
	}	

	OTP_DEBUG("%s dev %d: Success!\n",  __FUNCTION__, dev_id);
	return TRUE;	
}

BOOL write_market_id (unsigned short * p_id)
{
	UINT32 ret_len=0;
	UINT32 buf;
	UINT32 temp_buf;

	ret_len = ali_otp_read(0x2*4, &buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read Market ID %04x Failed!\n", 0x2*4);
		return FALSE;
	}
	if((buf>>16)&0xffff)
	{
		OTP_DEBUG("Market ID already write 0x%04x!\n",(buf>>16)&0xffff );
		return FALSE;
	}		
	buf = *p_id;
	temp_buf = ((buf<<16)&0xffff0000);
	ret_len = ali_otp_write((UINT8 *)&temp_buf ,  0x2*4, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Write Market id %04x Failed!\n", *p_id);
		return FALSE;
	}
	ret_len = ali_otp_read(0x2*4, &buf, 4);
	if(4!=ret_len)
	{
		OTP_DEBUG("Read Market id %04x 2nd time Failed!\n", 0x2*4);
		return FALSE;
	}

	if( (*p_id) != ((buf>>16)&0xffff))// check if write otp success 
	{
		OTP_DEBUG("%s Failed!\n", __FUNCTION__);
		return FALSE;
	}	

	OTP_DEBUG("%s Success!,market id is: 0x%04x\n",  __FUNCTION__, *p_id);
	return TRUE;	
}

BOOL read_market_id (unsigned short * p_id)
{
	INT32 rd_len;
	UINT32 market_id1,market_id2;
 
	rd_len = ali_otp_read(0x2*4, &market_id1, 4);
	if(4!=rd_len)
	{
		OTP_DEBUG("get Market_id 1st time fail %d\n", rd_len);
		return FALSE;
	}
	rd_len = ali_otp_read(0x2*4, &market_id2, 4);
	if(4!=rd_len)
	{
		OTP_DEBUG("get Market_id 2nd time fail %d\n", rd_len);
		return FALSE;
	}
	if(market_id1!=market_id2)
	{
		OTP_DEBUG("get Market_id value fail %d, %d\n", market_id1, market_id2);
		return FALSE;
	}
	p_id = ((market_id1>>16)&0xffff);
	OTP_DEBUG( "get Market_id = 0x%04x\n" , *p_id );
	return TRUE;
}

void security_key_read_protect()
{
	UINT32 ret_len=0;
	UINT32 chip_id,market_id;
	UINT8 sec_key[16];
    UINT32 i;
	ali_otp_read(0x0, &chip_id, 4);
	ali_otp_read(0x2*4, &market_id, 4);
    OTP_DEBUG("Read OTP info:\n");
    OTP_DEBUG("Chip ID is: %d\n",chip_id);
    OTP_DEBUG("Market segmentation ID is: 0x%x\n",(market_id>>16));
    ali_otp_read(0x4d*4, &sec_key, 16);
    OTP_DEBUG("Chipset key(0x00) is:\n");
    for(i=0;i<16;i++)
    {
        OTP_DEBUG("0x%x,",sec_key[i]);
    }
    ali_otp_read(0x55*4, &sec_key, 16);
    OTP_DEBUG("\nChipset key(0x20) is:\n");
    for(i=0;i<16;i++)
    {
        OTP_DEBUG("0x%x,",sec_key[i]);
    }
    ali_otp_read(0x51*4, &sec_key, 16);
    OTP_DEBUG("\nChipset key(0x21) is:\n");
    for(i=0;i<16;i++)
    {
        OTP_DEBUG("0x%x,",sec_key[i]);
    }
    ali_otp_read(0x68*4, &sec_key, 16);
    OTP_DEBUG("\nChipset key(0x22) is:\n");
    for(i=0;i<16;i++)
    {
        OTP_DEBUG("0x%x,",sec_key[i]);
    }
    ali_otp_read(0x5d*4, &sec_key, 8);
    OTP_DEBUG("\nChipset key(0x40) is:\n");
    for(i=0;i<8;i++)
    {
        OTP_DEBUG("0x%x,",sec_key[i]);
    }
    OTP_DEBUG("\n");
}
/*
BOOL get_chip_id(UINT32 *sc_chip_id)
{
	INT32 rd_len;
	UINT32 chip_id_bak;
	rd_len = ali_otp_read(0, &chip_id_bak, 4);
	if(4!=rd_len)
	{
		OTP_DEBUG("get chip_id 1st time fail %d\n", rd_len);
		return FALSE;
	}
	rd_len = ali_otp_read(0, sc_chip_id, 4);
	if(4!=rd_len)
	{
		OTP_DEBUG("get chip_id 2nd time fail %d\n", rd_len);
		return FALSE;
	}
	if(chip_id_bak!=(*sc_chip_id))
	{
		OTP_DEBUG("get chip_id value fail %d, %d\n", chip_id_bak, *sc_chip_id);
		return FALSE;
	}
	return TRUE;
}
*/
BOOL version_update(unsigned long new_version, unsigned char flag)
{
	UINT32 i=0, j=0;
	UINT32 ver[4]={0,0,0,0};
	UINT32 ver_addr = 0;
	UINT32 over_flag=0,count=0;
	
	if(flag == APP_VER)
	{
		ver_addr = 4*OTP_ADDESS_APP_VER;
	}
	else if(flag==SW_VER)
	{
		ver_addr = 4*OTP_ADDESS_SW_VER;
	}
	else if(flag == BL_VER)
	{
		ver_addr = 4*OTP_ADDESS_BL_VER;
	}
    else if(flag == UB_VER)
    {
        ver_addr = 4*OTP_ADDESS_UB_VER;
    }
    else
    {
        return FALSE;
    }
        
	for(i=0;i<4;i++)
	{
		ali_otp_read((ver_addr+4*i),&ver[i],4);		
	}
	for(i=0;i<4;i++)
	{
		if(over_flag)
		    break;
		for(j=0;j<32;j++)
		{
		    if(ver[i]&(0x1<<j))
		    {
		        count++;
		    }
		    else
		    {
		        over_flag = 1;
		        break;
		    }
		}
	}
	if((new_version<=count)||(new_version>MAX_VERSION_NUM)||(count==MAX_VERSION_NUM))
	{
		return FALSE;
	}
	else
	{
		for(i=0;i<4;i++)
			ver[i]=0;
		for(i=0;i<new_version/32;i++)
		{
			ver[i]=0xffffffff;
		}
		for(j=0;j<(new_version%32);j++)
		{
			ver[i]|=(0x1<<j);
		}
		for(i=0;i<4;i++)
		{
			if(ver[i]!=0)
				ali_otp_write(&ver[i],(ver_addr+4*i),4);	
		}
	}
	for(i=0;i<4;i++)
	{
		ver[i]=0;	
	}
	for(i=0;i<4;i++)
	{
		ali_otp_read((ver_addr+4*i),&ver[i],4);		
	}
	count = 0;
        over_flag=0;
	for(i=0;i<4;i++)
	{
		if(over_flag)
		    break;
		for(j=0;j<32;j++)
		{
		    if(ver[i]&(0x1<<j))
		    {
		        count++;
		    }
		    else
		    {
		        over_flag = 1;
		        break;
		    }
		}
	}	
	if(count==new_version)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL version_verify(unsigned long new_version, unsigned char flag)
{
	UINT32 i=0, j=0;
	UINT32 ver[4]={0,0,0,0};
	UINT32 ver_addr = 0;
	UINT32 over_flag=0,count=0;
	
	if(flag == APP_VER)
	{
		ver_addr = 4*OTP_ADDESS_APP_VER;
	}
	else if(flag==SW_VER)
	{
		ver_addr = 4*OTP_ADDESS_SW_VER;
	}
	else if(flag == BL_VER)
	{
		ver_addr = 4*OTP_ADDESS_BL_VER;
	}
    else if(flag == UB_VER)
    {
        ver_addr = 4*OTP_ADDESS_UB_VER;
    }
    else
    {
        return FALSE;
    }

	for(i=0;i<4;i++)
	{
		ali_otp_read((ver_addr+4*i),&ver[i],4);		
	}
	for(i=0;i<4;i++)
	{
		if(over_flag)
		    break;
		for(j=0;j<32;j++)
		{
		    if(ver[i]&(0x1<<j))
		    {
		        count++;
		    }
		    else
		    {
		        over_flag = 1;
		        break;
		    }
		}
	}
	if((new_version<count))
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

UINT32 get_version_from_mem(UINT32 src)
{
    UINT32 chunk_version=0;
    UINT32 d[4],i;
    
    OTP_DEBUG("%s src=0x%x\n",__FUNCTION__,src);
    chunk_version = *(volatile UINT32 *)(src+32+4);

    OTP_DEBUG("%s %d\n",__FUNCTION__,__LINE__);
    if(chunk_version!=0)
    {
        for(i=0;i<4;i++)
        {
            d[i]=((chunk_version>>(8*i))&0xff);
            if(d[i]>=0x30)
                d[i]-=0x30; 
            else
                d[i]=0;
        }
        chunk_version = d[0]*1000+d[1]*100+d[2]*10+d[3];
    }
    OTP_DEBUG("%s %d chunk_version=%d\n",__FUNCTION__,__LINE__,chunk_version);
    return chunk_version;
}

unsigned long version_get(unsigned char flag)
{
	UINT32 i=0, j=0;
	UINT32 ver[4]={0,0,0,0};
	UINT32 ver_addr = 0;
	UINT32 over_flag=0,count=0;
	
	if(flag == APP_VER)
	{
		ver_addr = 4*OTP_ADDESS_APP_VER;
	}
	else if(flag==SW_VER)
	{
		ver_addr = 4*OTP_ADDESS_SW_VER;
	}
	else if(flag == BL_VER)
	{
		ver_addr = 4*OTP_ADDESS_BL_VER;
	}
    else if(flag == UB_VER)
    {
        ver_addr = 4*OTP_ADDESS_UB_VER;
    }
    else
    {
        return FALSE;
    }

	for(i=0;i<4;i++)
	{
		ali_otp_read((ver_addr+4*i),&ver[i],4);		
	}
	for(i=0;i<4;i++)
	{
		if(over_flag)
		    break;
		for(j=0;j<32;j++)
		{
		    if(ver[i]&(0x1<<j))
		    {
		        count++;
		    }
		    else
		    {
		        over_flag = 1;
		        break;
		    }
		}
	}
        OTP_DEBUG("%s count=%d\n",__FUNCTION__,count);
        return count;
}

UINT32 ali_stb_ver_get()
{
    UINT32 flash_ver = 0;
    UINT32 file_size =0;
    UINT32 i=0;
    FILE * ver_file = NULL;
    char * ver_info = NULL;
    INT32 ret=0;
     
    /*get version from rootfs file */
    ver_file = fopen("/version/.version","r");	
    if(ver_file == NULL)
    {
        OTP_DEBUG("open version file failed\n");
        return 0; 
    }
    fseek(ver_file, 0, SEEK_END);
    file_size = ftell(ver_file);
    ver_info = (char*)malloc(file_size);
    if(ver_info == NULL)
    {
        OTP_DEBUG("%s malloc failed!\n",__FUNCTION__);
        fclose(ver_file);
        return 0;
    }
    fseek(ver_file, 0, SEEK_SET);
    ret = fread(ver_info,1,file_size ,ver_file);
    if( ret < file_size )
    {
         OTP_DEBUG("get version info failed\n");
         goto ERROR; 
    }
    OTP_DEBUG("ver_info:0x%x\n",ver_info);
   // DUMP(ver_info,20);

   for(i=0;i<20;i++)
	OTP_DEBUG("0x%x ",*(ver_info+i));	

    char * def_info = "linux_stb:";
    for(i=0 ; i <file_size; i++)
        if(0==memcmp(def_info,ver_info+i,strlen(def_info)))
            break;
    i+= strlen(def_info);
    OTP_DEBUG("i:0x%x\n",i);
    flash_ver=ali_atoi(ver_info+i);
    OTP_DEBUG("flash_ver = 0x%x\n",flash_ver);

    if(ver_file)
        fclose(ver_file);
    if(ver_info)
        free(ver_info);
    
    return flash_ver;

ERROR:
    if(ver_file)
        fclose(ver_file);
    if(ver_info)
        free(ver_info);
    
    return 0;
    
    
}
//#define DUMP(data,len) { const int l=(len); int lo;\
                         for(lo=0 ; lo<l ; lo++) {OTP_DEBUG(" 0x%02x,",*((data)+lo)); \
                         OTP_DUBUG("\n");} }
INT32 ali_stb_ver_check()
{    
    FILE * ver_file = NULL;
    /*get version from rootfs file */
    ver_file = fopen("/version/.version","r");	
    if(ver_file == NULL)
    {
        OTP_DEBUG("open version file failed\n");
        goto cmp_fail; 
    }
    fseek(ver_file, 0, SEEK_END);
	UINT32 file_size = ftell(ver_file);
    char * ver_info = (char*)malloc(file_size);
    fseek(ver_file, 0, SEEK_SET);
    INT32 ret = fread(ver_info,1,file_size ,ver_file);
    if( ret < file_size )
    {
         OTP_DEBUG("get version info failed\n");
         goto cmp_fail; 
    }
    OTP_DEBUG("ver_info:0x%x\n",ver_info);
   // DUMP(ver_info,20);

    UINT32 i = 0 ;
   for(i=0;i<20;i++)
	OTP_DEBUG("0x%x ",*(ver_info+i));	

    char * def_info = "linux_stb:";
    for(i=0 ; i <file_size; i++)
        if(0==memcmp(def_info,ver_info+i,strlen(def_info)))
            break;
    i+= strlen(def_info);
    OTP_DEBUG("i:0x%x\n",i);
    UINT32 flash_ver=ali_atoi(ver_info+i);
    OTP_DEBUG("flash_ver = 0x%x\n",flash_ver);

    UINT32 otp_ver=version_get(APP_VER);
    OTP_DEBUG("otp_ver = 0x%x\n",otp_ver);
    if(flash_ver <otp_ver){              
    	OTP_DEBUG("app version check failed\n");
	goto cmp_fail; 
    }
    free(ver_info);
    return 0 ;
cmp_fail:
   return -1;    
}



