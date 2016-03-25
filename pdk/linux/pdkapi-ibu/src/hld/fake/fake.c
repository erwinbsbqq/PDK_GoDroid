/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: smc.c
 *
 *  Description: This file contains all functions definition
 *		             of fake trace interface driver.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  0.                 Victor Chen            Ref. code
 *  1. 2005.9.8  Zhao Owen     0.1.000    Initial
 *
 ****************************************************************************/

#include <types.h>
#include <retcode.h>
//corei#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
//#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>

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
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>

#include <ali_fake_common.h>

// #define FAKE_TRACE_PERF_DEBUG

static int b_tick_trace_enable = 0;
static int i_tick_print_threshold = 0;

RET_CODE fake_kft_stop(void)
{        
    unsigned long t;
    int fd = open("/dev/ali_fake_trace", O_RDWR);

    if (fd < 0)
    {
    	printf("HLD: fake trace control open fail\n");
        return !SUCCESS;
    }

    ioctl(fd, FAKE_TRACE_KFT_STOP, &t);
    close(fd);
    
    return SUCCESS;
}

inline void fake_set_tick_trace_enable(int enable)
{
    b_tick_trace_enable = enable;
}

inline void fake_set_tick_print_threshold(int threshold)
{
    if (threshold > 0)
        i_tick_print_threshold = threshold;
}

inline unsigned long fake_get_tick(const char *what)
{  
#ifdef FAKE_TRACE_PERF_DEBUG     
    static unsigned long t_last = 0;
    unsigned long t = 0;
    static int fd = -1;

    if (!b_tick_trace_enable)
        return;
    
	if (fd < 0)
		fd = open("/dev/ali_fake_trace", O_RDWR);

    if (fd < 0)
    {
    	printf("HLD: fake trace control open fail\n");
        return -1;
    }

    ioctl(fd, FAKE_TRACE_GET_TICK, &t);
    printf("HLD FAKE: %s at %lu us", \
                what, t);
    if (t - t_last >= i_tick_print_threshold)
        printf(", gap %lu us", t - t_last);

    printf("\n");
    
    t_last = t;
    /* close(fd); */
    
    return t;
#else
	return (unsigned long)-1;
#endif
}
