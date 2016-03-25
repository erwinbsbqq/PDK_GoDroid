#include "porting_linux_header.h"

UINT32 osal_get_tick(void)
{
    struct timeval tv;
    do_gettimeofday(&tv);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
