#ifndef _ALI_OVG_PLATFORM_H
#define _ALI_OVG_PLATFORM_H

#include "ali_ovg.h"
#include "ali_ovg_reg.h"

#ifdef LINUX_VERSION_32
#include <../../mips/ali/m39/mach/map.h>
#endif

extern struct ALiPools aliPoolsID[];

#if  UTILIZE_BOARDSETTING
//#define OVG_BOARD_ADDR 0xA7F9036C
#define OVG_BOARD_GET_BYTE(A)   (*(volatile u8 *)(A))
#define OVG_BOARD_GET_WORD(A)   (*(volatile u32 *)(A))

#else
#define OVG_BOARD_GET_BYTE(A)   ((A))
#define OVG_BOARD_GET_WORD(A)   ((A))
#endif

#if MMU_ENABLED
#define MEMPOOL_MEMORY_BANK_LEN		9*1024*1024
#else
#define MEMPOOL_MEMORY_BANK_LEN		OVG_BOARD_GET_WORD(aliPoolsID[0].size)
#endif

#endif

