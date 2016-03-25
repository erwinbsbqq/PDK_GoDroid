#include <hld/adr_hld_dev.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>
#include <sys/ioctl.h>

#include <ali_uart_io_common.h>

#include <adr_retcode.h>
#include <osal/osal.h>

struct uart_mode
{
	unsigned int bps;
	int parity;
};

static INT32 fd = -1;
static INT32 uart_relay_func(UINT32 cmd, void *fp)
{	
	if (fd < 0)
	{
		fd = open("/dev/ali_uart_io", O_RDONLY| O_CLOEXEC);
		if (fd < 0)
		{
			return ERR_FAILURE;
		}
	}	

	return ioctl(fd, cmd, fp);	
}

unsigned char api_uart_read()
{
	int ret;
	unsigned char ch;
	ch = 0;

	ret = uart_relay_func(ALI_UART_IO_READ, &ch);
	
	return ch;
}

int api_uart_read_tm(UINT8 *data, UINT32 timeout)
{
	int ret;
	unsigned char ch;

	ret = uart_relay_func(ALI_UART_IO_READ_TM, &timeout);
	if (ret<0)
	{
		return ERR_FAILURE;
	}

	ch = *(UINT8*)&timeout;
	*data = ch;
	return SUCCESS;
}

//Compatibility with ibu interface
int api_uart_read_timeout(UINT8 *ch, UINT32 tm)
{
	int ret = -1;
	struct uart_pars pars;
	pars.tm = tm; 
	ret = uart_relay_func(ALI_UART_IO_READ_TIMEOUT, &pars);
	if (ret<0)
	{
		return ERR_FAILURE;
	}

	*ch = pars.ch; 
	return SUCCESS;
}

int api_uart_write(unsigned char ch)
{
	return uart_relay_func(ALI_UART_IO_WRITE, &ch);
}

int api_uart_set_mode(unsigned int bps, int parity)
{
	struct uart_mode paras = {bps, parity};
	return uart_relay_func(ALI_UART_IO_SET, &paras);
}

int api_uart_disable(unsigned char idx)
{
	return uart_relay_func(ALI_UART_DISABLE, &idx);
}

int api_uart_enable(unsigned char idx)
{
	return uart_relay_func(ALI_UART_ENABLE, &idx);
}

int api_uart_sci_clear(void)
{
	int ch;
	return uart_relay_func(ALI_UART_IO_CLEAR_SCI, &ch);
}

int api_uart_sci_set(void)
{
	int ch;
	return uart_relay_func(ALI_UART_IO_SET_SCI, &ch);
}

void api_uart_printf_mode(unsigned char ch)
{
	if (ch)
	{
		api_uart_sci_set();
	}
	else
	{
		api_uart_sci_clear();
	}
}
