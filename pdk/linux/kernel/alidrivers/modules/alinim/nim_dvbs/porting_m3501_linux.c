#include "nim_m3501.h"



/*****************************************************************************
Name:
    multu64div
Description:
    This function implment v1*v2/v3. And v1*v2 is 64 bit
Parameters:
    [IN]
    [OUT]
Return:
***********************************************************************/
//Get HI value from the multple result
#ifdef CONFIG_ARM
DWORD nim_s3501_multu64div(UINT32 v1, UINT32 v2, UINT32 v3)
{
    UINT32 v = 0;
    UINT64 tmp = 0;
    if(v3 == 0)
    {
        return 0;
    }
    tmp = (UINT64)v1 * (UINT64)v2;
    while(tmp >= v3)
    {
        tmp = tmp - v3;
        v++;
    }
    return v; 
}
#else
//kent,ok
# define mult64hi(v1, v2)           \
({  DWORD __ret;            \
    __asm__ volatile(                   \
        "multu  %1, %2\n    "               \
        "mfhi %0\n" \
         : "=r" (__ret)         \
         : "r" (v1), "r"(v2));                  \
    __ret;                      \
})
//Get LO value from the multple result
# define mult64lo(v1, v2)           \
({  DWORD __ret;            \
    __asm__ volatile(                   \
        "multu  %1, %2\n    "               \
        "mflo %0\n" \
         : "=r" (__ret)         \
         : "r" (v1), "r"(v2));                  \
    __ret;                      \
})

DWORD nim_s3501_multu64div(UINT32 v1, UINT32 v2, UINT32 v3)
{
    DWORD                 hi, lo;
    unsigned long long     tmp;
    DWORD                 *tmp64;
    DWORD                 ret;
    if (v3 == 0)
        return 0;
    hi = mult64hi(v1, v2);
    lo = mult64lo(v1, v2);
    tmp64 = ((DWORD *)(&tmp)) + 1;
    *tmp64-- = hi;
    *tmp64 = lo;
    //Few nop here seems required, if no nop is here,
    //then the result wont be correct.
    //I guess maybe this is caused by some GCC bug.
    //Because I had checked the compiled result between with nop and without nop,
    //they are quite different!! I dont have time to search more GCC problem,
    //Therefore, I can only leave the nop here quietly. :(
    //--Michael 2003/10/10
    __asm__("nop; nop; nop; nop");
    do_div(tmp, v3);
    ret = tmp;
    //ret = tmp/v3;  //kent
    return ret;
}
#endif
//-------------------------------------


UINT32 os_get_tick_count()
{
    struct timeval tv;
    do_gettimeofday(&tv);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}





