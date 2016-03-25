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
#ifdef WIN32
DWORD nim_s3501_multu64div(UINT32 v1, UINT32 v2, UINT32 v3)
{
    DWORD v;
    if (v3 == 0)
        return 0;
    v = (UINT64)(v1) * (UINT64)(v2) / v3;
    return v;
}
#else
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
    DWORD hi = 0;
    DWORD lo = 0;
    UINT64 tmp = 0;
    DWORD *tmp64=NULL;
    DWORD ret = 0;

    if (v3 == 0)
    {
        return 0;
    }  
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
    ret = tmp / v3;
    return ret;
}
#endif
//-------------------------------------







