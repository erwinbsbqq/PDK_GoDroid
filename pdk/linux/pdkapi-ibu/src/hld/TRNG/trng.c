#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>

#include <hld/trng/trng.h>

#if 1
#define ALI_TRNG_HLD_PRF(...) do{}while(0)
#else
#define ALI_TRNG_HLD_PRF(fmt, arg...) \
    do { \
        printf("TRNG_HLD: In %s "fmt, __func__, ##arg); \
    } while (0)
#endif

static int g_trng_fd = -1;

int trng_generate_byte( UINT8 *data )
{
	int ret = RET_FAILURE;

	ret = ioctl(g_trng_fd, ALI_TRNG_GENERATE_BYTE, (UINT32)data);
	return ret;
}

int trng_generate_64bits( UINT8 *data )
{
	int ret = RET_FAILURE;

	ret = ioctl(g_trng_fd, ALI_TRNG_GENERATE_64bits, (UINT32)data);
	return ret;
}

int trng_get_64bits(UINT8 *data, UINT32 n)
{
	int ret = RET_FAILURE;
	ALI_TRNG_GET_64BITS trng_group;

	trng_group.data = data;
	trng_group.n_group = n;
	
	ret = ioctl(g_trng_fd, ALI_TRNG_GET_64bits, (UINT32)&trng_group);
	return ret;
}

int trng_api_attach(void)
{
	g_trng_fd = open(ALI_TRNG_LINUX_DEV_PATH, O_RDWR);
	if(g_trng_fd<0)
	{
		ALI_TRNG_HLD_PRF("TRNG hld error: open g_trng_fd\n");
		return (RET_FAILURE);
	}
	return RET_SUCCESS;        
	
}
int trng_api_detach(void)
{
	if(g_trng_fd > 0)
	{
		close(g_trng_fd);
		return RET_SUCCESS;
	}
	else
	{
		ALI_TRNG_HLD_PRF("TRNG hld error: wrong g_trng_fd\n");
		return RET_FAILURE;
	}
}