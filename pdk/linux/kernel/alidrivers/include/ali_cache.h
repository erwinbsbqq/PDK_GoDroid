
#ifndef _ALI_CACHE_H_
#define _ALI_CACHE_H_

#if defined(CONFIG_ARM)

#include <linux/dma-direction.h>
#include <asm/cacheflush.h>

#define CACHE_LINE_SIZE  32
#define CACHE_L_ALIGN(x) ((unsigned long)(((x)%CACHE_LINE_SIZE)?((x)+CACHE_LINE_SIZE-((x)%CACHE_LINE_SIZE)):(x)))
#define CACHE_A_ALIGN(x) ((const void *)((__u32)x & (~(__u32)(CACHE_LINE_SIZE - 1))))

/* cache operation for arm arch */
#define __CACHE_FLUSH_ALI(start, len)		do \
    { \
        dmac_flush_range(CACHE_A_ALIGN(start), (CACHE_A_ALIGN((unsigned long)start + CACHE_L_ALIGN(len)))); \
        outer_flush_range(__pa(CACHE_A_ALIGN(start)), __pa((unsigned long)CACHE_A_ALIGN(start) + CACHE_L_ALIGN(len))); \
    }while(0)
    
#define __CACHE_INV_ALI(start, len)			do \
    { \
        dmac_map_area((CACHE_A_ALIGN(start)), (unsigned long)CACHE_A_ALIGN(start) + CACHE_L_ALIGN(len), DMA_FROM_DEVICE); \
        outer_inv_range(__pa(CACHE_A_ALIGN(start)), __pa((unsigned long)CACHE_A_ALIGN(start) + CACHE_L_ALIGN(len))); \
    }while(0)
    
#define __CACHE_CLEAN_ALI(start, len)		do \
    { \
        dmac_map_area((CACHE_A_ALIGN(start)), ((unsigned long)CACHE_A_ALIGN(start) + CACHE_L_ALIGN(len)), DMA_TO_DEVICE); \
        outer_clean_range(__pa(CACHE_A_ALIGN(start)), __pa((unsigned long)CACHE_A_ALIGN(start) + CACHE_L_ALIGN(len))); \
    }while(0)
    
#define __CACHE_ADDR_ALI(addr)    ((unsigned int)addr)
#define __NONCACHE_ADDR_ALI(addr) ((unsigned int)addr)

#else

#include <asm/io.h>

/* cache operation for mips arch */
#define __CACHE_FLUSH_ALI(start, len)		dma_cache_wback((unsigned long)(start), len)
#define __CACHE_INV_ALI(start, len)		dma_cache_inv((unsigned long)(start), len)
#define __CACHE_CLEAN_ALI(start, len)		dma_cache_wback_inv((unsigned long)(start), len)

#define __CACHE_ADDR_ALI(addr)    ((unsigned int)addr & 0x9FFFFFFF)
#define __NONCACHE_ADDR_ALI(addr) ((unsigned int)addr |0xA0000000)

#endif

#define __CACHE_ADDR_ALI_SEE(addr)    (((unsigned int)addr & 0x1FFFFFFF) | 0x80000000)
#define __NONCACHE_ADDR_ALI_SEE(addr) ((unsigned int)addr |0xA0000000)

#endif

