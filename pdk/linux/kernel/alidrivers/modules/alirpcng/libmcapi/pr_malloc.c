/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: pr_malloc.c
 *
 *  Description: Portable runtime os malloc (linux-kernel) implementation file.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.01.25      David.L         0.1.000         Initial
 ****************************************************************************/

#include <linux/slab.h>
#include "pr.h"

/**
 *  In respect to kernel malloc and free, default we use the flag GFP_KERNEL,
 *  which may sleep and swap to free memory. Only allowed in user context, 
 *  but is the most reliable way to allocate memory.
 *  Please refer to the kernel manual for details.
 */

void *PR_Malloc(Size_t n)
{
    return kmalloc((size_t)(n), GFP_KERNEL);
}

void PR_Free(void *ptr)
{
    kfree(ptr);
}



