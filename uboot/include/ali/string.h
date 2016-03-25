#ifndef __ALI_STRING_H__
#define __ALI_STRING_H__

#include <linux/string.h>
#include <malloc.h>

#define MEMCPY(dest, src, len)			memcpy(dest, src, len)
#define MEMCMP(buf1,buf2, len)			memcmp(buf1,buf2, len)
#define MEMSET(dest, c, len)				memset(dest, c, len)

#define STRCPY(dest,src)				strcpy(dest,src)
#define STRCMP(dest,src)				strcmp(dest,src)
#define STRLEN(str)						strlen(str)


#define MALLOC(size)					malloc(size)

#define FREE(ptr)							free(ptr)
#define PRINTF								libc_printf

#define libc_printf(x...) printf(x)

#define SDBBP()		asm volatile(".word	0x7000003f; nop")

static inline void ASSERT(int x)
{
    if( x ==0 )
        SDBBP();
}



#endif 
