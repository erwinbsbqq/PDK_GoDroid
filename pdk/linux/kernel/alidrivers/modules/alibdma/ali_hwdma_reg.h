/*
 *  Copyright (c) 2012, ALi Corporation.
 *  All rights reserved.
 *
 *      File:           ali_hwdma_reg.h
 *      Version:        1.0
 *      Date:           2012-03-09
 *      Description:    ali bdma hw register map define header file.
 */

#ifndef _LINUX_ALI_HWDMA_REG_H
#define _LINUX_ALI_HWDMA_REG_H
#ifdef CONFIG_ARM
#include <../mach-ali3921/include/mach/ali-s3921.h>
#include <asm/pgtable.h>
#endif

/* Define index of HW register */
#define IRQ_BDMA_FPGA                   12
#define IRQ_BDMA_C3701C                 14


#ifdef CONFIG_ARM
#define IRQ_BDMA                        (IRQ_BDMA_C3701C + 32)
#define ALI_SOC_BASE_ADDR               VIRT_SYSTEM
#define ALI_S3921_HWDMA_BASE_ADDR       0x18044000
#else
#ifdef FPGA_MODE // Onlly Used for C3701C FPGA
	#define IRQ_BDMA                        (IRQ_BDMA_FPGA + 8)
#else
#define IRQ_BDMA                        (IRQ_BDMA_C3701C + 8)
#endif
#define ALI_SOC_BASE_ADDR               0xB8000000
#ifdef FPGA_MODE // Onlly Used for C3701C FPGA
	#define ALI_C3701C_HWDMA_BASE_ADDR      0xB800A800
#else
#define ALI_C3701C_HWDMA_BASE_ADDR      0xB8044000
#endif
#endif
#define ALI_FPGA_HWDMA_BASE_ADDR        0xB800A800

#define GE_HWRESET					       ((volatile __iomem *)(ALI_SOC_BASE_ADDR+0x80))
#define DMA_HWRESET					       ((volatile __iomem *)(ALI_SOC_BASE_ADDR+0x84))

#ifdef CONFIG_ARM
static void __iomem *ALI_HWDMA_BASE_ADDR = (void __iomem *) ALI_S3921_HWDMA_BASE_ADDR;
#define pgd_BA_offset 22 - PGDIR_SHIFT
#define PT_LEN PGDIR_SHIFT - PAGE_SHIFT

#define ALI_HWVG_GET_UINT32(reg)		((*((volatile u32*)(0xB800A000+reg))))
#define ALI_HWDMA_GET_UINT32(reg)		((*((volatile u32*)(ALI_S3921_HWDMA_BASE_ADDR+reg))))
#define ALI_HWDMA_SET_UINT32(val, reg)	((*((volatile u32*)(ALI_S3921_HWDMA_BASE_ADDR+reg)))=val)
//#define TRANSFER_VIR_2LEV(adr)			(((adr&(((1<<(32-PGDIR_SHIFT) )-1)<<PGDIR_SHIFT))<<pgd_BA_offset)|(adr&(((1<<PT_LEN)-1)<< PAGE_SHIFT))|(adr&0xFFF) | 0x00200000)
#define TEST1(adr) ((adr&(((1<<(32-PGDIR_SHIFT) )-1)<<PGDIR_SHIFT))<<pgd_BA_offset)

#define TEST2(adr) (adr& (((1<<PT_LEN)-1)<< PAGE_SHIFT))
//#define PD_TABLE_BA(pgd_BA, virt)		(pgd_BA + (8192>>pgd_BA_offset)*(virt>>31))
#define TABLE_UNIT						(8192>>pgd_BA_offset)
#define TABLE_UNIT_NUM(virt)			(pgd_BA_offset)
#define TRANSFER_VIR_2LEV(adr)			(adr)//((adr&0xFFC00000)|(adr&0x003FF000)|(adr&0xFFF))
#define PD_TABLE_BA(pgd_BA, virt)		(pgd_BA)
#else
static void __iomem *ALI_HWDMA_BASE_ADDR = (void __iomem *) ALI_C3701C_HWDMA_BASE_ADDR;

//#define ALI_HWDMA_GET_UINT32(reg)       ioread32((void __iomem *)(ALI_HWDMA_BASE_ADDR + reg))
//#define ALI_HWDMA_SET_UINT32(val, reg)  iowrite32((val), (void __iomem *)(ALI_HWDMA_BASE_ADDR + reg))
#define ALI_HWVG_GET_UINT32(reg)       ((*((volatile u32*)(0xB800A000+reg))))
#define ALI_HWDMA_GET_UINT32(reg)       ((*((volatile u32*)(ALI_C3701C_HWDMA_BASE_ADDR+reg))))
#define ALI_HWDMA_SET_UINT32(val, reg)  ((*((volatile u32*)(ALI_C3701C_HWDMA_BASE_ADDR+reg)))=val)
#define TRANSFER_VIR_2LEV(adr)			(adr)
#endif

//index of register
#define BDMA_IRQ                        (*(volatile BDMAuint *)(ALI_SOC_BASE_ADDR+0x38))

//Wait time out tick to check command finish
#define WAIT_TICKS                      5*HZ

/* 0x00 Command Queue Control Register
[1]     VG_BUSY                         Check this bit to get VG status.
[0]     CMD_LQ_EN_REG                   Set to 1 to enable command queue. Check it be cleared for command queue finish.
 */
#define BDMA_CMD_CTRL                   0x00
#define VG_BUSY_BIT                     1       // Add at version 1 (C3701C)
#define CMD_QUEUE_FINISH_BIT            0
#define CMD_QUEUE_ENABLE                (0x01 << CMD_QUEUE_FINISH_BIT)

/* 0x04 Command Queue Base Address Register */
#define BDMA_CMD_BA                     0x04

/* 0x08 Number of commands in command queue */
#define BDMA_CMD_LEN                    0x08

/* 0x18 BDMA Version Register */
#define BDMA_VER                        0x18 // use 0x60 register to check version, don't use this.

/* 0x20 BDMA Memory Set and Start Register
[31:8]  BDMA_MEM_SET_VAL_REG[23:0]      Setting the memset value of [23:0]
[4]     BDMA_BLIT_2D_EN                 If you want to use BLIT 2D function, set the bit to 1.
[3:2]   BDMA_MEM_SET_SEL_REG            decide memset value length. 0 -> 1 byte, 1 -> 2 byte, 3 -> 4 byte, 2 is not allowed.
[1]     BDMA_MEM_SET_EN_REG             Set to 1 for memset function (ignore memory read and using BDMA_MEM_SET_VAL_REG).
[0]     BDMA_ENABLE_REG                 Set to 1 to start BDMA.
 */
#define BDMA_DMA_CTRL                   0x20
#define MEM_SET_VAL_23_0_BIT            8
#define BLIT_2D_ENABLE_BIT              4
#define MEM_SET_LEN_BIT                 2
#define MEM_SET_ENABLE_BIT              1
#define BDMA_ENABLE_BIT                 0
#define MEM_SET_VAL_23_0_MASK           (0x00FFFFFF << MEM_SET_VAL_23_0_BIT)
#define BLIT_2D_ENABLE                  (0x01 << BLIT_2D_ENABLE_BIT)
#define MEM_SET_LEN_1_BYTE              (0x0 << MEM_SET_LEN_BIT)
#define MEM_SET_LEN_2_BYTE              (0x1 << MEM_SET_LEN_BIT)
#define MEM_SET_LEN_4_BYTE              (0x3 << MEM_SET_LEN_BIT)
#define MEM_SET_ENABLE                  (0x01 << MEM_SET_ENABLE_BIT)
#define BDMA_ENABLE                     (0x01 << BDMA_ENABLE_BIT)

/* 0x24 Memory Read Base Address Register (memory copy source address) */
#define BDMA_DMA_RD                     0x24

/* 0x28 Memory Write Base Address Register (memory copy destination address) */
#define BDMA_DMA_WR                     0x28

/* 0x2C Memory access length Register */
#define BDMA_DMA_LEN                    0x2C

/* 0x30 BDMA System Contrl Register
[31:24] BDMA_MEM_IDLE_CNT               Setting the idle count while use the mem check  // Add at version 1 (C3701C)
[23:16] BDMA_MEM_SET_VAL_REG[31:24]     Setting the memset value of [31:24]             // Add at version 1 (C3701C)
[15:8]  BDMA_LAT_REG                    The fixed latency counter.
[1]     BDMA_INC_MODE_REG               The INC_MODE, (default = 1) important.
[0]     BDMA_HI_PRIORITY_REG            Set to 1, the high priority is controlled by cpu, otherwise by hardware.
 */
#define BDMA_SYS_CTRL                   0x30
#define MEM_SET_VAL_31_24_BIT           16
#define MEM_SET_VAL_31_24_MASK          (0x000000FF << MEM_SET_VAL_31_24_BIT)
#define LATENCY_BIT                     8
#define LATENCY_MASK                    (0x000000FF << LATENCY_BIT)

/* 0x34 RD_MMU_BASE_REG Page Table Base Address for Memory Read Register */
#define BDMA_MMU_RD                     0x34

/* 0x38 RD_MMU_CTRL_REG
[31:22] RD_PDE_INDEX                    This register shows the current PDE index used in reading memory (READ ONLY)
[21:12] RD_PTE_INDEX                    This register shows the current PTE index used in reading memory (READ ONLY)
[1]     RD_MMU_UPDATE_REG               Set to 1 after solving the page fault problem when reading memory is proceed.
[0]     RD_BYPASS_REG                   Set to 1 if MMU disable in memory read is expected.
 */
#define BDMA_MMU_RD_PARAM               0x38

/* 0x3C WR_MMU_BASE_REG Page Table Base Address for Memory Write Register */
#define BDMA_MMU_WR                     0x3C

/* 0x40 WR_MMU_CTRL_REG
[31:22] WR_PDE_INDEX                    This register shows the current PDE index used in writing memory (READ ONLY)
[21:12] WR_PTE_INDEX                    This register shows the current PTE index used in writing memory (READ ONLY)
[1]     WR_MMU_UPDATE_REG               Set to 1 after solving the page fault problem when writing memory is proceed.
[0]     WR_BYPASS_REG                   Set to 1 if MMU disable in memory write is expected.
 */
#define BDMA_MMU_WR_PARAM               0x40

/* 0x44 Interrupt Status and Mask Register
[22]    RD_PDE_NOT_VALID                This bit will be '1' if PDE error occur when memory read is proceed.
[21]    RD_PTE_NOT_VALID                This bit will be '1' if PTE error occur when memory read is proceed.
[20]    WR_PDE_NOT_VALID                This bit will be '1' if PDE error occur when memory write is proceed.
[19]    WR_PTE_NOT_VALID                This bit will be '1' if PTE error occur when memory write is proceed.
[18]    CMD_ERROR_REG                   This bit will be '1' if the command's format in command queue is unknown.
[17]    CMD_LQ_EN_FINISH_REG            This bit will be '1' after the commands in command queue are finished.
[16]    BDMA_FINISH                     This bit will be '1' if BDMA finish its job assigned by IO register (Not by command queue).
[6]     RD_PTE_NOT_VALID_MASK           Set to 1 to enable the interrupt source of RD_PTE_NOT_VALID.
[5]     RD_PDE_NOT_VALID_MASK           Set to 1 to enable the interrupt source of RD_PDE_NOT_VALID.
[4]     WR_PTE_NOT_VALID_MASK           Set to 1 to enable the interrupt source of WR_PTE_NOT_VALID.
[3]     WR_PDE_NOT_VALID_MASK           Set to 1 to enable the interrupt source of WR_PDE_NOT_VALID.
[2]     CMD_ERROR_REG_MASK              Set to 1 to enable the interrupt source of CMD_ERROR_REG.
[1]     CMD_LQ_EN_FINISH_REG_MASK       Set to 1 to enable the interrupt source of CMD_LQ_EN_FINISH_REG.
[0]     BDMA_FINISH_MASK                Set to 1 to enable the interrupt source of BDMA_FINISH.
 */
#define BDMA_INT_STATUS_MASK            0x44
#define INT_RD_PDE_NOT_VALID            (0x01 << 22)
#define INT_RD_PTE_NOT_VALID            (0x01 << 21)
#define INT_WR_PDE_NOT_VALID            (0x01 << 20)
#define INT_WR_PTE_NOT_VALID            (0x01 << 19)
#define INT_CMD_FORMAT_ERROR            (0x01 << 18)
#define INT_CMD_QUEUE_FINISH            (0x01 << 17)
#define INT_BDMA_FINISH                 (0x01 << 16)

/* 0x48 Count Cycle Number When BDMA is busy Register */
#define BUSY_CNT_REG                    0x48

/* 0x4C Count Cycle Number When BDMA is Accessing Memory Register */
#define DRAM_CNT_REG                    0x4C

/* 0x50 BLIT_2D_HEIGHT_SEL_REG
[31:16] BLIT_2D_HEIGHT                  The height of the src picture.
[1:0]   BLIT_2D_PIX_BYTE                0 -> 1 byte, 1 -> 2 byte, 2 -> 3 byte, 3 -> 4 byte,
 */
#define BLIT_2D_HEIGHT_SEL_REG          0x50                                    // Add at version 1 (C3701C)
#define BLIT_2D_HEIGHT_BIT              16
#define BLIT_2D_HEIGHT_MASK             (0x0000FFFF << BLIT_2D_HEIGHT_BIT)

/* 0x54 BLIT_2D_SRC_OFFSET_Y_X_REG
[31:16] BLIT_2D_SRC_OFFSET_Y            The offset y pixel count of src picture. It is signed.
[15:0]  BLIT_2D_SRC_OFFSET_X            The offset x pixel count of src picture. It is signed.
 */
#define BLIT_2D_SRC_OFFSET_Y_X_REG      0x54                                    // Add at version 1 (C3701C)
#define SRC_OFFSET_Y_BIT                16
#define SRC_OFFSET_X_BIT                0
#define SRC_OFFSET_Y_MASK               (0x0000FFFF << SRC_OFFSET_Y_BIT)
#define SRC_OFFSET_X_MASK               (0x0000FFFF << SRC_OFFSET_X_BIT)

/* 0x58 BLIT_2D_SRC_DST_STRIDE_REG
[31:16] BLIT_2D_DST_STRIDE              The offset byte when changing line. It is signed.
[15:0]  BLIT_2D_SRC_STRIDE              The offset byte when changing line. It is signed.
 */
#define BLIT_2D_SRC_DST_STRIDE_REG      0x58                                    // Add at version 1 (C3701C)
#define DST_STRIDE_BIT                  16
#define SRC_STRIDE_BIT                  0
#define DST_STRIDE_MASK                 (0x0000FFFF << DST_STRIDE_BIT)
#define SRC_STRIDE_MASK                 (0x0000FFFF << SRC_STRIDE_BIT)

/* 0x5C BLIT_2D_DST_OFFSET_Y_X_REG
[31:16] BLIT_2D_DST_OFFSET_Y            The offset y pixel count of dest picture. It is signed.
[15:0]  BLIT_2D_DST_OFFSET_X            The offset x pixel count of dest picture. It is signed.
 */
#define BLIT_2D_DST_OFFSET_Y_X_REG      0x5C                                    // Add at version 1 (C3701C)
#define DST_OFFSET_Y_BIT                16
#define DST_OFFSET_X_BIT                0
#define DST_OFFSET_Y_MASK               (0x0000FFFF << DST_OFFSET_Y_BIT)
#define DST_OFFSET_X_MASK               (0x0000FFFF << DST_OFFSET_X_BIT)

/* 0x60 BDMA Version Register */
#ifdef C3821_Verify
#define BDMA_VERSION_REG                0x18                                    // Add at version 1 (C3701C)
#else
#define BDMA_VERSION_REG                0x60                                    // Add at version 1 (C3701C)
#endif
#define BDMA_VER_S3701C                 0x00
#define BDMA_VER_C3701C                 0x01

/* Command Queue Format and Write/Check Macro
1st word:
[31:28] BYTE_WRITE_MASK                 The mask limits which byte in the second word (VALUE) ca be written into target register.
[24:20] CHECK_BIT                       Chose which bit(0-31) both in second word and target register will be compared.
                                        If CMD_ID is ioreg_wr_mode, this region is ignored.
[3:0]   CMD_ID                          Set the command type. There are two types in command queue.
                                        0x2: ioreg_wr_mode
                                            This mode will write the second word's value into target register.
                                        0x5: ioreg_check_mode
                                            This mode will check whether second word's chosen bit is equal to target register's chosen bit.
                                            The next command will be executed until they are equal.
2nd word:
[31:0]  VALUE
 */

/* Command Queue Write/Check Reg Macro */
#define CmdQWriteReg(CmdBuf, RegAddr, Value)                                    \
    *(CmdBuf++) = (0xF0000002 | (( RegAddr & 0x07FFF)<<4) );                    \
    *(CmdBuf++) = (Value);

#define CmdQCheckReg(CmdBuf, RegAddr, BitNum, EqualZero)                        \
    *(CmdBuf++) = (0x00000005 | ((BitNum&0x1F)<<20) | ((RegAddr&0x07FFF)<<4));  \
    *(CmdBuf++) = (EqualZero) ? 0x00000000 : (0x01<<BitNum) ;


#define EQUAL_ZERO  true
#define EQUAL_ONE   false

#endif

