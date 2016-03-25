/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: atomic_arm32.h
 *
 *  Description: CPU arch (arm-32) dependent implementation.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.01.25      David.L         0.1.000         Initial
 ****************************************************************************/

#ifndef ATOMIC_ARM32_H
#define ATOMIC_ARM32_H


#define mcapi_mb()      asm volatile ( "" : : : "memory")

static inline unsigned long mcapi_xchg(unsigned long x, volatile void *ptr)
{
    unsigned long ret;
#if __ARM_ARCH__ >= 6
    unsigned int tmp;
#endif

    mcapi_mb();

#if __LINUX_ARM_ARCH__ >= 6
    asm volatile("@	__xchg4\n"
        "1:	ldrex	%0, [%3]\n"
        "	strex	%1, %2, [%3]\n"
        "	teq	%1, #0\n"
        "	bne	1b"
        : "=&r" (ret), "=&r" (tmp)
        : "r" (x), "r" (ptr)
        : "memory", "cc");
#else
    asm volatile("@	__xchg4\n"
        "	swp	%0, %1, [%2]"
        : "=&r" (ret)
        : "r" (x), "r" (ptr)
        : "memory", "cc");
#endif

    mcapi_mb();

    return ret;
}

#define mcapi_xchg(ptr,x) \
    ((__typeof__(*(ptr)))mcapi_xchg((unsigned long)(x),(ptr)))


#endif // ATOMIC_ARM32_H

