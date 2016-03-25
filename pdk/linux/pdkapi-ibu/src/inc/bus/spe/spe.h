/****************************************************************************
*
*  ALi (Zhuhai) Corporation, All Rights Reserved. 2007 Copyright (C)
*
*  File: spe.h
*
*  Description: Header file of SPI/UP DMA.
*  History:
*      Date        Author         Version   Comment
*      ====        ======         =======   =======
*  1.  2009.02.09  Wen Liu     0.1.000   Initial
****************************************************************************/

#ifndef __SPE_H__
#define __SPE_H__

#include <basic_types.h>

//Types of interface
#define SPE_SPI 0
#define SPE_UP  1

//Types of device
#define SPE_SFLASH  3
#define SPE_PFLASH  2
#define SPE_SPI_ETH 1
#define SPE_UP_ETH  0

#define SPE_INVALID_ID 0xff

//dma copy flag
#define SPI_DUMMY_ADDR 0x02
#define SPE_BYTE_TRANS 0x04
#define SPE_ADDR_NOINC 0x20
#define SPE_DMA_READ  0x00
#define SPE_DMA_WRITE 0x80
#define SPE_DMA_SYNC  0x01
#define SPE_DMA_ENA_PAUSE 0x1000000

//dma wait flag
#define SPE_DMA_POLLING 0x01
#define SPE_DMA_FOREVER 0x00

//interrupt id
#define SPE_INTERRUPT_ID (24+8)

//Registers definition
#define SPE_INT    0xa0
#define SPE_STATUS 0xa4
#define SPE_CPU_CTRL 0x00
#define SPE_DMA_MEMADDR 0x0
#define SPE_DMA_DEVADDR 0x4
#define SPE_DMA_LEN  0x8
#define SPE_DMA_CTRL 0xc

#define SPE_FLAG_DMA_OVER 0x01

UINT8 spe_get_dev(UINT8 type, UINT32 cfg, UINT32 *map_addr);
void  spe_dma_copy(UINT8 id, UINT32 dev_addr, UINT32 mem_addr, UINT32 len, UINT32 flag);
UINT8 spe_dma_wait(UINT8 id, UINT8 flag);
void  spe_mutex_enter(UINT8 id);
void  spe_mutex_leave(UINT8 id);
void  spe_dma_pause();
void  spe_dma_stop(UINT8 id);
void  spe_register_isr(UINT8 id, ISR_PROC isr, UINT32 param);
#endif /* __SPE_H__ */

