#include <boot_common.h>

void *boot_memset(void *s, int c, unsigned long count)
{
    int i;
    char *ss = (char *)s;
    
    for(i = 0; i < count; i++) ss[i] = c;
    return s;
}

int boot_memcmp(UINT8 *src  , UINT8 *dst, UINT32 len)
{
    unsigned int i ; 
    for(i = 0; i < len; i++)
    {
        if (src[i] != dst[i])
            return -1 ;
    }
    return 0 ;	
		
}

int boot_memcpy(UINT8 *dst  , UINT8 *src, UINT32 len)
{
    unsigned int i ; 
    for(i = 0; i < len; i++)
    {
        dst[i] = src[i];
    }
    return 0 ;			
}

int boot_memcpy4(UINT32 *dst  , UINT32 *src, UINT32 len)
{
    unsigned int i; 
    for(i = 0; i < len/4; i++)
    {
        dst[i] = src[i];
    }
    return 0 ;			
}