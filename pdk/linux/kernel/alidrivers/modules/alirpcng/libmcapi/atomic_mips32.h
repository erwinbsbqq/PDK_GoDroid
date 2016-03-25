/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: atomic_mips32.h
 *
 *  Description: CPU arch (mips-32) dependent implementation.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.01.25      David.L         0.1.000         Initial
 ****************************************************************************/

#ifndef ATOMIC_MIPS32_H
#define ATOMIC_MIPS32_H


#define mcapi_mb()      asm volatile ( "" : : : "memory")

static inline unsigned long mcapi_xchg_u32(volatile int * m, unsigned int val)
{
    unsigned long retval;
    unsigned long dummy;

    mcapi_mb();

    asm volatile (
        "       .set    mips3                                   \n"
        "1:     ll      %0, %3                  # xchg_u32      \n"
        "       .set    mips0                                   \n"
        "       move    %2, %z4                                 \n"
        "       .set    mips3                                   \n"
        "       sc      %2, %1                                  \n"
        "       beqz    %2, 2f                                  \n"
        "       .subsection 2                                   \n"
        "2:     b       1b                                      \n"
        "       .previous                                       \n"
        "       .set    mips0                                   \n"
        : "=&r" (retval), "=m" (*m), "=&r" (dummy)
        : "R" (*m), "Jr" (val)
        : "memory");

    mcapi_mb();

    return retval;
}


#define mcapi_xchg(ptr, x) \
    ((__typeof__(*(ptr)))mcapi_xchg_u32((int *)(ptr), (unsigned int)(x)))


#endif // ATOMIC_MIPS32_H

