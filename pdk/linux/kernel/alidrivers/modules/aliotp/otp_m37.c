#include <linux/version.h>
#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 35)
#include <linux/slab.h>
#endif
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <ali_otp_common.h>
#include <ali_reg.h>
#include "ali_otp.h"


#define M37_OTP_BASE 			0x18042000
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

#define M37_OTP_BIT_READ_BUSY	1<<8
#define M37_OTP_BIT_READ_TRIG 	1<<8
#define M37_OTP_BIT_PROG_BUSY 	1<<0
#define M37_OTP_BIT_PROG_TRIG 	1<<0
#define M37_OTP_BIT_PROG_FAIL 	1<<8
#define M37_OTP_BIT_INT_ENABLE 	1<<0

#define M37_REG_ACCESS  (0x5f<<2)
#define M37_OTP_CONFIG  (0x03<<2)
#define M37_APP_REG_W_ACCESS  (0x80<<2)
#define M37_APP_REG_R_ACCESS  (0x81<<2)
#define M37_OTP_MEM_SIZE    0x400

#define REG_ACCESS_DELAY 0
#define M37_OTP_R_TIME_OUT    ((int)-6)
#define M37_OTP_P_TIME_OUT    ((int)-5)
#define M37_OTP_R_FAILURE     ( (int)-4)
#define M37_OTP_P_FAILURE     ( (int)-3)

#define M37_OTP_FAILURE     ( (int)-1)
#define M37_OTP_SUCCESS     ( (int)0)

typedef char INT8;
typedef unsigned char UINT8;
typedef short INT16;
typedef unsigned short UINT16;
typedef int INT32;
typedef unsigned int UINT32;

static int g_ali_otp_s3701c_w_clk[4][2]={{133, 0x61532}, {166, 0x71642},{100, 0x41430}, {83, 0x41321}};
/* S3701C opt protect zone map */
/*static UINT16 g_ali_otp_zone[50][2]={{0,4},{1,4},{2,4},{3,4},{4,292},{0x4d,16},\
		{0x51,16},{0x55,16},{0x59,16},{0x5d,8},{0x5f,4},{0x60,16},\
		{0x64,16},{0x68,16},{0x6c,16},{0x70,64},\
        {0x80,16},{0x84,16},{0x88,16},\
        {0x8c,16},{0x90,16},{0x94,16},{0x98,16},{0x9c,16},{0xa0,16},{0xa4,16},{0xa8,16},\
        {0xac,16},{0xb0,16},{0xb4,16},{0xb8,16},{0xbc,16},{0xc0,16},{0xc4,16},{0xc8,16},\
        {0xcc,16},{0xd0,16},{0xd4,16},{0xd8,16},{0xdc,16},{0xe0,16},{0xe4,16},{0xe8,16},\
        {0xec,16},{0xf0,16},{0xf4,16},{0xf8,16},{0xfc,16}};
        */
static UINT8 g_ali_otp_obuf[M37_OTP_MEM_SIZE];
static UINT32 g_ali_otp_ready =0 ;
static unsigned int g_ali_otp_uart_enable = 0;
static unsigned int g_ali_otp_usb_enable = 0;
static unsigned int g_ali_otp_ethernet_enable = 0;
static unsigned char g_ali_otp_debug_level = 0;

#define OTP_PRINTK(fmt, args...)				\
{										\
	if (0 !=  g_ali_otp_debug_level)					\
	{									\
		printk(fmt, ##args);					\
	}									\
}

#define OTP_ERR_PRINTK(fmt, args...)		\
{										\
	printk(fmt, ##args);						\
}

extern unsigned long ali_sys_ic_get_dram_clock(void);


void ali_otp_set_debug(UINT8 debug)
{
	g_ali_otp_debug_level = debug;
	OTP_PRINTK("[ %s ], g_ali_otp_debug_level = %d\n", __FUNCTION__, g_ali_otp_debug_level);
}


static inline UINT32 delay_us(UINT32 tms)
{
    if (0 == tms)
    {
        return 0;
    }
    udelay(tms);
    return 0;
}

/*
static UINT8 ReadRegByte(UINT32 offset)
{
	delay_us(REG_ACCESS_DELAY);
	return (__REG8ALI(M37_OTP_BASE + offset));
}


static UINT16 ReadRegWord(UINT32 offset)
{
	delay_us(REG_ACCESS_DELAY);
	return (__REG16ALI(M37_OTP_BASE + offset));
}
*/

static UINT32 ReadRegDword(UINT32 offset)
{
	delay_us(REG_ACCESS_DELAY);
	return (__REG32ALI(M37_OTP_BASE + offset));
}


static void WriteRegByte(UINT32 offset, UINT8 value)
{
	__REG8ALI(M37_OTP_BASE + offset) = value;
	delay_us(REG_ACCESS_DELAY);
}

/*
static void WriteRegWord(UINT32 offset, UINT16 value)
{
	__REG16ALI(M37_OTP_BASE + offset) = value;
	delay_us(REG_ACCESS_DELAY);
}
*/

static void WriteRegDword(UINT32 offset, UINT32 value)
{
	__REG32ALI(M37_OTP_BASE + offset) = value;
	delay_us(REG_ACCESS_DELAY);
}

/*
static void SetRegBit(UINT32 offset, UINT8 bit) //set to 1
{
	UINT8 value;
	value = __REG8ALI(M37_OTP_BASE + offset);
	delay_us(REG_ACCESS_DELAY);
	value|=(0x1<<(bit));
	__REG8ALI(M37_OTP_BASE + offset) = value;
	delay_us(REG_ACCESS_DELAY);
}
*/
/*
static void ClearRegBit(UINT32 offset, UINT8 bit) //set to 0
{
	UINT8 value;
	value = __REG8ALI(M37_OTP_BASE + offset);
	delay_us(REG_ACCESS_DELAY);
	value&=(~(0x1<<(bit)));
	__REG8ALI(M37_OTP_BASE + offset) = value;
	delay_us(REG_ACCESS_DELAY);
}
*/
/*
static UINT8 ReadRegBit(UINT32 offset, UINT8 bit)//judge if it's 0 or not 0
{
	delay_us(REG_ACCESS_DELAY);
	return (__REG8ALI(M37_OTP_BASE + offset)&(0x1<<(bit)));
}
*/

static int otp_wait_read_free(void)
{
    UINT32 tmo = 100;
    
    do
	{
		if(0==tmo)
		{
			return M37_OTP_R_TIME_OUT;
		}
		tmo --;
		udelay(1);
	}while(ReadRegDword(M37_OTP_ST)&M37_OTP_BIT_READ_BUSY);
	
    return M37_OTP_SUCCESS;    
}

static int otp_wait_write_free(void)
{
    UINT32 tmo = 310 *2 ;
    
    do
	{
		if(0==tmo)
		{
			return M37_OTP_P_TIME_OUT;
		}
		tmo --;
		udelay(1);
	}while(ReadRegDword(M37_OTP_ST)&M37_OTP_BIT_PROG_BUSY);
	
    return M37_OTP_SUCCESS;    
}


static INT32 otp_write_byte(UINT16 addr, UINT8 data)
{
	//UINT16 value;
	if(otp_wait_read_free() != M37_OTP_SUCCESS) 
	{
        return M37_OTP_R_TIME_OUT ;
    }
    else if(otp_wait_write_free() != M37_OTP_SUCCESS) 
    {
        return M37_OTP_P_TIME_OUT ;
    }

	OTP_PRINTK("[ %s ], addr 0x%04x, data 0x%02x\n", __FUNCTION__, addr, data);
    
	WriteRegDword(M37_OTP_ADDR, addr&0x3ff);
	WriteRegByte(M37_OTP_WDATA, data);
	WriteRegDword(M37_OTP_CTRL, ReadRegDword(M37_OTP_CTRL)|M37_OTP_BIT_PROG_TRIG);

    if( otp_wait_write_free() != M37_OTP_SUCCESS )
    {
        OTP_ERR_PRINTK("otp write bytes time out \n");
        return M37_OTP_P_TIME_OUT ;
    }

	//value = ReadRegDword(OTP_INT);
	if(ReadRegDword(M37_OTP_INT)&M37_OTP_BIT_INT_ENABLE)
	{
		if(ReadRegDword(M37_OTP_INT)&M37_OTP_BIT_PROG_FAIL)
		{
			//WriteRegDword(OTP_INT, value&(~OTP_BIT_PROG_FAIL));
			OTP_ERR_PRINTK("[ %s %d ] error, write fail!\n", __FUNCTION__, __LINE__);
			return M37_OTP_P_FAILURE;
		}
		else
		{
			return M37_OTP_SUCCESS;	
		}
	}
	else
	{
		return M37_OTP_SUCCESS;
	}
}


static UINT32 otp_read_dword(UINT16 addr)
{
	UINT32 data = 0;
	
	if(otp_wait_read_free() != M37_OTP_SUCCESS)
	{
        return M37_OTP_R_TIME_OUT ;
    }
    else if(otp_wait_write_free() != M37_OTP_SUCCESS)
    {
        return M37_OTP_P_TIME_OUT ;
    }
        
	WriteRegDword(M37_OTP_ADDR, addr&0x3ff);
	WriteRegDword(M37_OTP_CTRL, ReadRegDword(M37_OTP_CTRL)|M37_OTP_BIT_READ_TRIG);

    if( otp_wait_read_free() != M37_OTP_SUCCESS )
    {
        return M37_OTP_R_TIME_OUT ;
    }

	data = ReadRegDword(M37_OTP_RDATA);	
	OTP_PRINTK("[ %s ], addr 0x%04x, data 0x%08x\n", __FUNCTION__, addr, data);
	
	return data;
}


int ali_otp_hw_init(void)
{
	UINT32 i = 0 ;
	UINT32 mem_clk = ali_sys_ic_get_dram_clock() ;
	
	if (1 == g_ali_otp_ready)
	{
		OTP_PRINTK("[OTP_ERROR]OTP Bus Already Inited!\n");
		return M37_OTP_SUCCESS;
	}
	g_ali_otp_ready = 1 ;     
    
    for( i= 0 ; i<4; i++)
    {
        if (mem_clk == g_ali_otp_s3701c_w_clk[i][0])
        {           
        }
    }
    OTP_PRINTK("OTP mem_clk = 0x%x\n",mem_clk);
    
	mutex_init(&g_ali_otp_device.ioctl_mutex);
	
	return M37_OTP_SUCCESS;
}


int ali_otp_read(unsigned long offset, unsigned char *buf, int len)
{
    UINT32 i = 0;
    int ret = 0;
    UINT32 tmp_ofset = offset & (~0x3) ; // dw ofset
    UINT32 tmp_len = 0; // dw len

    if (0 == (len % 4))    
    {    	
    	tmp_len = (len & (~0x3))/4; // dw len
    }
    else
    {
    	tmp_len = (len & (~0x3))/4 +1; // dw len
    }
    
	if((0 == g_ali_otp_ready) || (NULL == buf) || (0 == len) || ((offset + len) > M37_OTP_MEM_SIZE))
	{
        OTP_ERR_PRINTK("[OTP_ERROR]opt read input failed \n");
		return M37_OTP_FAILURE;
	}

    if (mutex_lock_interruptible(&g_ali_otp_device.ioctl_mutex))
    {
        return(-ERESTARTSYS);
    }
    
    OTP_PRINTK("otp read : ofset = 0x%lx, len = 0x%x\n", offset, len);
    OTP_PRINTK("otp read : tmp_len = 0x%x, tmp_ofset = 0x%x\n", tmp_len, tmp_ofset);
    for(i =0 ; i< tmp_len; i ++ )
    {
        ret = otp_read_dword(tmp_ofset+i*4);
        if ((ret != M37_OTP_R_TIME_OUT) && ( ret != M37_OTP_P_TIME_OUT))
        {
            *((UINT32*)g_ali_otp_obuf+i) = ret ;
        }
        else
        {
            mutex_unlock(&g_ali_otp_device.ioctl_mutex);        
            OTP_ERR_PRINTK("[OTP_ERROR]otp read failed,cause = %x, ofset = 0x%x\n", ret, tmp_ofset+i);
            return 0 ;
        }
    }
    
    mutex_unlock(&g_ali_otp_device.ioctl_mutex);        
    i=offset - tmp_ofset ;
    memcpy(buf,&g_ali_otp_obuf[i],len);
    
    return len ;
}


int ali_otp_write(unsigned char *buf, unsigned long offset, int len)
{
    UINT32 i = 0;
    UINT32 value = 0;
    
	if ((0 == g_ali_otp_ready) || (NULL == buf) || (0 == len) || ((offset + len) > M37_OTP_MEM_SIZE))
	{
        OTP_ERR_PRINTK("[OTP_ERROR]otp write input failed \n");
        return M37_OTP_FAILURE;
	}
    
   	if (mutex_lock_interruptible(&g_ali_otp_device.ioctl_mutex))
    {
        return(-ERESTARTSYS);
    }
    
    OTP_PRINTK("otp write : buf = 0x%x, ofset = 0x%lx, len = 0x%x\n", (unsigned int)buf, offset, len);    
    
	i = 0;	
	while (i < len)	
	{
		if(otp_write_byte(offset++, *buf++)!= M37_OTP_SUCCESS)
		{
			value = ReadRegDword(M37_OTP_ST);
			OTP_ERR_PRINTK("[OTP_ERROR]otp program, failed addr is 0x%lx\n", offset);
			WriteRegDword(M37_OTP_INT, ReadRegDword(M37_OTP_INT)&(~M37_OTP_BIT_PROG_FAIL));
			break;
		}		
		else
		{
			i++;
		}
	}

    mutex_unlock(&g_ali_otp_device.ioctl_mutex);        
	if(i != len)
	{
		return M37_OTP_P_FAILURE;
	}
    
	return len;
}


int otp_m37_lock(UINT32 offset, INT32 len)
{   
	if ((0 == g_ali_otp_ready) || ( 0== len) || ((offset + len) > M37_OTP_MEM_SIZE))
	{
		return 0;
	}

   	if (mutex_lock_interruptible(&g_ali_otp_device.ioctl_mutex))
    {
        return(-ERESTARTSYS);
    } 

    mutex_unlock(&g_ali_otp_device.ioctl_mutex); 
    return 0;
}


unsigned int otp_get_value(unsigned int addr)
{
    return otp_read_dword(addr);
}


unsigned int otp_get_control(void)
{
    int otp_03_val = otp_read_dword(0xc);
    if ((otp_03_val != M37_OTP_R_TIME_OUT) && (otp_03_val != M37_OTP_P_TIME_OUT))
    {    
        g_ali_otp_uart_enable = (otp_03_val & (1 << 12)) ? DISABLE : ENABLE ;
        g_ali_otp_usb_enable = (otp_03_val & (1 << 20)) ? DISABLE: ENABLE ;
        g_ali_otp_ethernet_enable = (otp_03_val & (1 << 22)) ? DISABLE : ENABLE ;
        
        return 0 ;
    }
    else
    {
        return -1 ;  
    }
}

unsigned int is_otp_uart_enable(void)
{
    return  g_ali_otp_uart_enable  ; 
}


unsigned int is_otp_usb_enable(void)
{
    return  g_ali_otp_usb_enable; 
}


unsigned int is_otp_ethernet_enable(void)
{
    return  g_ali_otp_ethernet_enable; 
}


