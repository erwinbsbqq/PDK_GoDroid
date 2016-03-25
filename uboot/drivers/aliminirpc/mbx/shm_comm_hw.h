/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: shm_comm_hw.h
 *
 *  Description: shm_comm device h/w abstraction head file.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.01.25      David.L         0.1.000         Initial
 *  2.  2013.06.25      Tony.Zh         0.2.000         Changed for S3921
 ****************************************************************************/
#ifndef __SHM_COMM_HW_H__
#define __SHM_COMM_HW_H__


#include "pr_types.h"
#include "ali_reg.h"
#include "ali_interrupt.h"


#define GET_MINIRPC_MSG_TYPE(A)              (A&0xFF)
#define GET_MINIRPC_MSG_CURRENT_INDEX(A)     ((A>>8)&0xFF)
#define GET_MINIRPC_MSG_TOTAL_INDEX(A)       ((A>>16)&0xFF)
#define GET_MINIRPC_MSG_LEN(A)               ((A>>24)&0xFF)


#define MAIN_ACK                0xABCDEF01
#define SEE_ACK                 0xABCDEF02

#define MBX0                    0x01
#define MBX1                    0x02
#define MBX2                    0x04
#define MBX3                    0x08
#define MBX_MASK                0x0f

/* core id, unique id in platform */
#define	MAIN_CPU_ID             0x22221111
#define	SEE_CPU_ID              0x22222222
#define	AUD0_CPU_ID             0x22223333
#define	AUD1_CPU_ID             0x22224444


/* soc chip id register */
#define CORE_ID_REG             (0x18000000)
#define	GET_CPU_ID()            (*(volatile Uint32 *)(CORE_ID_REG))

/*----------------------------------------------------------------------------*/

/* h/w max number of mailbox */
#define MBX_MAX                 4


/* register read/write operations */
#define readb(addr)             (*(volatile unsigned char *)(addr))
#define readw(addr)             (*(volatile unsigned short *)(addr))
#define readl(addr)             (*(volatile unsigned int *)(addr))

#define writeb(b,addr)          (*(volatile unsigned char *)(addr)) = (b)
#define writew(b,addr)          (*(volatile unsigned short *)(addr)) = (b)
#define writel(b,addr)          (*(volatile unsigned int *)(addr)) = (b)

/*----------------------------------------------------------------------------*/
#define CONFIG_ALI_CHIP_M3921

/* interrupt vector of main mailbox */
#if defined(CONFIG_ALI_CHIP_M3921)
	#define MAIN_MBX0_INT           INT_ALI_MBX0 
    #define MAIN_MBX1_INT           INT_ALI_MBX1
    #define MAIN_MBX2_INT           INT_ALI_MBX2
    #define MAIN_MBX3_INT           INT_ALI_MBX3
#else
	#define	MAIN_MBX0_INT           0x47
	#define	MAIN_MBX1_INT           0x46
	#define	MAIN_MBX2_INT           0x45
	#define	MAIN_MBX3_INT           0x44
#endif

/* mailbox reg for send */
#define	REG_MAIN_MBX0_SEND      (0x18000200)
#define	REG_MAIN_MBX1_SEND      (0x18000204)
#define	REG_MAIN_MBX2_SEND      (0x18000208)
#define	REG_MAIN_MBX3_SEND      (0x1800020C)

/* mailbox reg for receive */
#define	REG_MAIN_MBX0_RECV      (0x18040200)
#define	REG_MAIN_MBX1_RECV      (0x18040204)
#define	REG_MAIN_MBX2_RECV      (0x18040208)
#define	REG_MAIN_MBX3_RECV      (0x1804020C)

/* mailbox interrupt enable register */
#define	REG_MAIN_MBX_ENABLE     (0x1800003C)
#define	MAIN_MBX0_ENABLE_FLG    (1<<31)
#define	MAIN_MBX1_ENABLE_FLG    (1<<30)
#define	MAIN_MBX2_ENABLE_FLG    (1<<29)
#define	MAIN_MBX3_ENABLE_FLG    (1<<28)
#define MAIN_MBX_ENABLE_MASK    (MAIN_MBX0_ENABLE_FLG|MAIN_MBX1_ENABLE_FLG|MAIN_MBX2_ENABLE_FLG|MAIN_MBX3_ENABLE_FLG)

#define MAIN_MBX_ENABLE         \
    writel(MAIN_MBX_ENABLE_MASK, REG_MAIN_MBX_ENABLE)

/* interrupt register */
#define	REG_MAIN_MBX_INT        (0x18000034)
#define	MAIN_MBX0_SET_FLG       (1<<27)
#define	MAIN_MBX1_SET_FLG       (1<<26)
#define	MAIN_MBX2_SET_FLG       (1<<25)
#define	MAIN_MBX3_SET_FLG       (1<<24)
#define	MAIN_MBX0_CLR_FLG       (1<<31)
#define	MAIN_MBX1_CLR_FLG       (1<<30)
#define	MAIN_MBX2_CLR_FLG       (1<<29)
#define	MAIN_MBX3_CLR_FLG       (1<<28)

#define MAIN_CLR_SEE_MBX0_INT   writel(MAIN_MBX0_CLR_FLG, REG_MAIN_MBX_INT)
#define MAIN_CLR_SEE_MBX1_INT   writel(MAIN_MBX1_CLR_FLG, REG_MAIN_MBX_INT)
#define MAIN_CLR_SEE_MBX2_INT   writel(MAIN_MBX2_CLR_FLG, REG_MAIN_MBX_INT)
#define MAIN_CLR_SEE_MBX3_INT   writel(MAIN_MBX3_CLR_FLG, REG_MAIN_MBX_INT)

/* sense which mailbox is interruptting */
#define	REG_MAIN_SENSE_MBX      (0x18000034)
#define REG_SEE_SENSE_MAIN      REG_MAIN_SENSE_MBX
#define	MAIN_MBX0_SENSE_FLG     (1<<27)
#define	MAIN_MBX1_SENSE_FLG     (1<<26)
#define	MAIN_MBX2_SENSE_FLG     (1<<25)
#define	MAIN_MBX3_SENSE_FLG     (1<<24)
#define	MAIN_MBX_SENSE_MASK     (MAIN_MBX0_SENSE_FLG|MAIN_MBX1_SENSE_FLG|MAIN_MBX2_SENSE_FLG|MAIN_MBX3_SENSE_FLG)

/*----------------------------------------------------------------------------*/

/* interrupt vector of see mailbox */
#define	SEE_MBX0_INT            0x43
#define	SEE_MBX1_INT            0x42
#define	SEE_MBX2_INT            0x41
#define	SEE_MBX3_INT            0x40

/* mailbox reg for send */
#define	REG_SEE_MBX0_SEND       (0x18040200)
#define	REG_SEE_MBX1_SEND       (0x18040204)
#define	REG_SEE_MBX2_SEND       (0x18040208)
#define	REG_SEE_MBX3_SEND       (0x1804020C)

/* mailbox reg for receive */
#define	REG_SEE_MBX0_RECV       (0x18000200)
#define	REG_SEE_MBX1_RECV       (0x18000204)
#define	REG_SEE_MBX2_RECV       (0x18000208)
#define	REG_SEE_MBX3_RECV       (0x1800020C)

/* mailbox interrupt enable register */
#define	REG_SEE_MBX_ENABLE      (0x1804003C)
#define	SEE_MBX0_ENABLE_FLG		(1<<31)
#define	SEE_MBX1_ENABLE_FLG		(1<<30)
#define	SEE_MBX2_ENABLE_FLG		(1<<29)
#define	SEE_MBX3_ENABLE_FLG		(1<<28)
#define SEE_MBX_ENABLE_MASK     (SEE_MBX0_ENABLE_FLG|SEE_MBX1_ENABLE_FLG|SEE_MBX2_ENABLE_FLG|SEE_MBX3_ENABLE_FLG)

#define SEE_MBX_ENABLE          \
    writel(SEE_MBX_ENABLE_MASK, REG_SEE_MBX_ENABLE)

/* Issue interrupt register */
#define	REG_SEE_MBX_INT         (0x18040034)
#define	SEE_MBX0_SET_FLG        (1<<27)
#define	SEE_MBX1_SET_FLG        (1<<26)
#define	SEE_MBX2_SET_FLG        (1<<25)
#define	SEE_MBX3_SET_FLG        (1<<24)
#define	SEE_MBX0_CLR_FLG        (1<<31)
#define	SEE_MBX1_CLR_FLG        (1<<30)
#define	SEE_MBX2_CLR_FLG        (1<<29)
#define	SEE_MBX3_CLR_FLG        (1<<28)

#define SEE_CLR_MAIN_MBX0_INT   writel(SEE_MBX0_CLR_FLG, REG_SEE_MBX_INT)
#define SEE_CLR_MAIN_MBX1_INT   writel(SEE_MBX1_CLR_FLG, REG_SEE_MBX_INT)
#define SEE_CLR_MAIN_MBX2_INT   writel(SEE_MBX2_CLR_FLG, REG_SEE_MBX_INT)
#define SEE_CLR_MAIN_MBX3_INT   writel(SEE_MBX3_CLR_FLG, REG_SEE_MBX_INT)

/* sense which mailbox is interruptting */
#define	REG_SEE_SENSE_MBX       (0x18040034)
#define REG_MAIN_SENSE_SEE      REG_SEE_SENSE_MBX
#define	SEE_MBX0_SENSE_FLG      (1<<27)
#define	SEE_MBX1_SENSE_FLG      (1<<26)
#define	SEE_MBX2_SENSE_FLG      (1<<25)
#define	SEE_MBX3_SENSE_FLG      (1<<24)
#define	SEE_MBX_SENSE_MASK      (SEE_MBX0_SENSE_FLG|SEE_MBX1_SENSE_FLG|SEE_MBX2_SENSE_FLG|SEE_MBX3_SENSE_FLG)


/*----------------------------------------------------------------------------*/

typedef struct _Mbx_Hal MBX_HAL;
struct _Mbx_Hal {
    void        (*init)(void);
    void        (*deinit)(void);
    void        (*tx)(Uint32 mbxIdx, Uint32 datum);
    Uint32      (*rx)(Uint32 mbxIdx);
};


void MbxHwMain_Init(void);
void MbxHwSee_Init(void);
void MbxHwMain_Deinit(void);
void MbxHwSee_Deinit(void);
void MbxHwMain_Tx(Uint32 mbxIdx, Uint32 datum);
void MbxHwSee_Tx(Uint32 mbxIdx, Uint32 datum);
Uint32 MbxHwMain_Rx(Uint32 mbxIdx);
Uint32 MbxHwSee_Rx(Uint32 mbxIdx);


#endif // __SHM_COMM_HW_H__

