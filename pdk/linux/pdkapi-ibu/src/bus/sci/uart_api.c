#include <hld/hld_dev.h>
#include<sys/types.h> 
#include<sys/stat.h> 
#include<fcntl.h>
#include <sys/ioctl.h>

#include <ali_uart_io_common.h>

struct uart_mode
{
	unsigned int bps;
	int parity;
};

static INT32 fd = -1;
static INT32 uart_relay_func(UINT32 cmd, void *fp)
{
    INT32 ret = -1;
	if (fd < 0)
	{
		//fd = open("/dev/ali_uart_io", O_RDONLY);
		fd = open("/dev/ali_uart_io", O_RDWR);	//20150511 modified
		if (fd < 0)
	    {
	        ret = -2;
			return ret;
	    }
	}
    ret = ioctl(fd, cmd, fp);
    
	return ret;	
}

unsigned char api_uart_read()
{
//20150512 modified
//	return (unsigned char)uart_relay_func(ALI_UART_IO_READ, NULL);

	unsigned char ch;
	int ret;
	ret = uart_relay_func(ALI_UART_IO_READ, &ch);
	if (ret == 0)
	{
		return ch;
	}
	else
	{
		//error
		//TODO
		return (unsigned char)(-1);
	}
}

int api_uart_read_timeout(UINT8 *ch, UINT32 tm)
{   
    int ret =-1;
    struct uart_pars pars;
    pars.tm = tm;
	ret = uart_relay_func(ALI_UART_IO_READ_TIMEOUT,&pars);
    *ch = pars.ch; 

    return ret;
}


int api_uart_write(unsigned char ch)
{
	return uart_relay_func(ALI_UART_IO_WRITE, &ch);
}

int api_uart_set_mode(unsigned int bps, int parity)
{
	struct uart_mode paras={bps, parity};
	return uart_relay_func(ALI_UART_IO_SET, &paras);
}

void api_uart_printf_mode(unsigned char ch)
{
	uart_relay_func(ALI_UART_SFU_MODE, &ch);
}

//20150511 modified
void api_uart_set_sci_mode(void)
{
	uart_relay_func(ALI_UART_IO_SET_SCI, NULL);
}

void api_uart_clear_sci_mode(void)
{
	uart_relay_func(ALI_UART_IO_CLEAR_SCI, NULL);
}


