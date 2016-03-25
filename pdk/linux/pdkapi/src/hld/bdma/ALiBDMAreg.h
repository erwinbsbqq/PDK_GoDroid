#ifndef __ALI_OPEN_VG_REG_H
#define __ALI_OPEN_VG_REG_H



//#define USE_TEMP_REG


#define ALI_BDMA_BASE_ADDR	0xB800A000
#define ALI_SOC_BASE_ADDR	0xB8000000

//index of register
#define REG_RD_BASE_INDX		0x24
#define REG_WR_BASE_INDX		0x28
#define REG_DMA_LENS_INDX  	    0x28
#define REG_DMA_CTR_INDX		0x20


//register IO address
#define REG_RD_BASE     (*(volatile UINT32 *)(ALI_SOC_BASE_ADDR+REG_RD_BASE_INDX))
#define REG_WR_BASE 	(*(volatile UINT32 *)(ALI_BDMA_BASE_ADDR+REG_WR_BASE_INDX))
#define REG_DMA_LENS	(*(volatile UINT32 *)(ALI_BDMA_BASE_ADDR+REG_DMA_LENS_INDX))
#define REG_DMA_CTR	(*(volatile UINT32 *)(ALI_BDMA_BASE_ADDR+REG_DMA_CTR_INDX))


#endif /* __ALI_OPEN_VG_H */