#ifndef _BOOT_COMMON_H_
#define _BOOT_COMMON_H_

#define RELEASE_VERSION

typedef char			INT8;
typedef unsigned char	UINT8;
typedef short			INT16;
typedef unsigned short	UINT16;
typedef long			INT32;
typedef unsigned long	UINT32;


typedef INT32			RET_CODE;

typedef int				BOOL;

#define	FALSE			(0)
#define	TRUE			(!FALSE)

#define RET_SUCCESS		((INT32)0)
#define RET_FAILURE		((INT32)1)

#define NULL ((void *)0)

#ifdef ALI_ARM_STB
#define SYS_BASE_ADDR     0x18000000//0x00800000  //0x18000000
#define CHIP_STRAP_PIN    0x18000070//0x00800070  //0x18000070
#define SD_BASE_ADDR      0x18014000//0x00814000  //0x18014000
#define TWD_BASE_ADDR     0x1bf00600//0x1bf00600  //0x1bf00600
#define NOR_BASE_ADDR     0x1802e000//0x0082e000  //0x1802e000
#define NAND_BASE_ADDR    0x18032000//0x00832000  //0x18032000
#else
#define SYS_BASE_ADDR     0xB8000000//0x00800000  //0x18000000
#define CHIP_STRAP_PIN    0xB8000070//0x00800070  //0x18000070
#define SD_BASE_ADDR      0xB8014000//0x00814000  //0x18014000
#define TWD_BASE_ADDR     0xBbf00600//0x1bf00600  //0x1bf00600
#define NOR_BASE_ADDR     0xB802e000//0x0082e000  //0x1802e000
#define NAND_BASE_ADDR    0xB8032000//0x00832000  //0x18032000
#endif


#define BOOT_DEV(x)       ((x>>15)&0xF)
#define BOOT_DEV_NOR(x)   (((x)&0x8) == 0x0)
#define BOOT_DEV_NF(x)    (((x) == 0x8) || ((x) == 0xA))
#define BOOT_DEV_SD(x)    (((x) == 0x9) || ((x) == 0xB))      
#define BOOT_DEV_EMMC(x)  (((x)&0xC) == 0xC) 

#define PHY_IO_ADDR(x)    (((x)&0x000fffff)|0x18000000)
#define PHY_MEM_ADDR(x)   ((x)&0x0fffffff)
#define ABS_MEM_ADDR(x)   (((x)&0x00ffffff)|0x80000000)
//#define DEC_MEM_ADDR      0x84000000//0x01100000  //0x80500000     //for dsc encrypt BL

#ifndef RELEASE_VERSION
#define TICK_PER_SEC               27000000
#define TIMER_RES_DIVIDER          2                
#define DELAY_1US                  (TICK_PER_SEC/(TIMER_RES_DIVIDER*1000000))
#else
#define BOOT_CPU_CLK               (((GET_WORD(CHIP_STRAP_PIN)>>8)&0x7) * 100 + 800)		// 800MHz - 1500MHz
#define TIMER_RES_DIVIDER         100 		// PHERI_CLK = clk/2  presacler = 50 
#define DELAY_1US                      BOOT_CPU_CLK/TIMER_RES_DIVIDER	
#endif
#define DELAY_1MS                     (DELAY_1US*1000)



#define CFG_CACHE_ADDR    _end

#define BL_BASE_ADDR      0x00100000     //0x0c000000     //BL in flash addr
#define BL_PHY_ADDR(x)    ((x) - BL_BASE_ADDR + 0x0fc00000)

#define CHUNK_ID		  0
#define CHUNK_OFFSET	  8
#define CHUNK_HEADER_SIZE 128
#define FLASH_END_ADDR	  0x300000	// limit to 3MB by default
#define BL_CHUNK_ID       0x07f80100
#define UHEADER_MAGIC     0x44414548

#define GET_BYTE(i)                (*(volatile unsigned char *)(i))
#define SET_BYTE(i,d)              (*(volatile unsigned char *)(i)) = (d)
 
#define GET_HALF(i)                (*(volatile unsigned short *)(i))
#define SET_HALF(i,d)              (*(volatile unsigned short *)(i)) = (d)

#define GET_WORD(i)                (*(volatile unsigned long *)(i))
#define SET_WORD(i,d)              (*(volatile unsigned long *)(i)) = (d)

#define GET_WORD_UNALIGNED(i)      (((volatile unsigned char *)(i))[0]) \
                                   |(((volatile unsigned char *)(i))[1]<<8) \
                                   |(((volatile unsigned char *)(i))[2]<<16) \
                                   |(((volatile unsigned char *)(i))[3]<<24)           

#define GET_UINT32_BE(p)           (((p)[0] << 24) | ((p)[1] << 16) | ((p)[2] << 8) | (p)[3])

#define DEADLOOP                   do{}while(1)
#define CPU_ACCESS_DSC_DIS         {*(volatile UINT8 *)(0x00800110) |= (0x1<<5);}


#define printk(...)
#define dma_cache_wback_inv(x,y)   v7_flush_kern_dcache_area(x,y)
#define dma_cache_inv(x,y)         v7_dma_inv_area(x,y)
//#define memset                     boot_memset
//#define mdelay(x)                  boot_mdelay(x)
//#define udelay(x)                  boot_udelay(x)
#define MEMCMP                     boot_memcmp
#define MEMCPY                     boot_memcpy
#define MEMCPY4                    boot_memcpy4

typedef struct CFG_ADDR
{
    UINT32 off: 30;
    UINT32 region: 2;
}CFADDR;

struct BOOT_CFG
{
    UINT32 info_strapin;
    UINT32 boot_dev;
    UINT32 mcfg_addr;
    UINT32 mcfg_len;
    UINT32 bl_addr;
    UINT32 bl_len;
    UINT32 cp_addr;
    UINT32 cp_len;
    void   (*copy_bl)(UINT32 , UINT32 , UINT32 size);
}; 
//static struct BOOT_CFG *boot_cfg = NULL;

#endif

