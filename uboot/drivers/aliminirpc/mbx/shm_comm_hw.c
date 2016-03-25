/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: shm_comm_hw.c
 *
 *  Description: shm_comm device h/w abstraction implementation file.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.01.25      David.L         0.1.000         Initial
 ****************************************************************************/


#include "pr_types.h"

#include "shm_comm_hw.h"

#include "ali_rpc_debug.h"


void MbxHwMain_Init(void)
{
    printf("\n\nMbxHwMain_Init\n\n");
	
    /* Clear unexcepted ints. */
    MAIN_CLR_SEE_MBX0_INT;
    MAIN_CLR_SEE_MBX1_INT;
    MAIN_CLR_SEE_MBX2_INT;
    MAIN_CLR_SEE_MBX3_INT;
    
    /* Enable mailbox h/w. */
    MAIN_MBX_ENABLE;
}

void MbxHwSee_Init(void)
{
    /* Clear unexcepted ints. */
    SEE_CLR_MAIN_MBX0_INT;
    SEE_CLR_MAIN_MBX1_INT;
    SEE_CLR_MAIN_MBX2_INT;
    SEE_CLR_MAIN_MBX3_INT;

    /* Enable mailbox h/w. */
    SEE_MBX_ENABLE;
}

void MbxHwMain_Deinit(void)
{
    Uint32 val;

    /* Disable mailbox h/w. */
    val = readl(REG_MAIN_MBX_ENABLE);
    val &= (~MAIN_MBX_ENABLE_MASK);
    writel(val, REG_MAIN_MBX_ENABLE);

    /* Clear unexcepted ints. */
    MAIN_CLR_SEE_MBX0_INT;
    MAIN_CLR_SEE_MBX1_INT;
    MAIN_CLR_SEE_MBX2_INT;
    MAIN_CLR_SEE_MBX3_INT;
}

void MbxHwSee_Deinit(void)
{
    Uint32 val;

    /* Disable mailbox h/w. */
    val = readl(REG_SEE_MBX_ENABLE);
    val &= (~SEE_MBX_ENABLE_MASK);
    writel(val, REG_SEE_MBX_ENABLE);

    /* Clear unexcepted ints. */
    SEE_CLR_MAIN_MBX0_INT;
    SEE_CLR_MAIN_MBX1_INT;
    SEE_CLR_MAIN_MBX2_INT;
    SEE_CLR_MAIN_MBX3_INT;
}

/** Try to find a free mailbox for delevery when mbxIdx equals PBX_MBX_MASK, else
 *  try the given one.
 */
void MbxHwMain_Tx(Uint32 mbxIdx, Uint32 datum)
{
    Uint32 val;

//	printf("[%s] mbxIdx:%d  datum:0x%08x\n",__func__,mbxIdx,datum);

    if (mbxIdx == MBX_MASK) {
        while (1) {
            val = readl(REG_MAIN_SENSE_MBX);
            if ((val & MAIN_MBX_SENSE_MASK) != MAIN_MBX_SENSE_MASK)
                break;
        }
        if (!(val & MAIN_MBX0_SENSE_FLG)) {
            writel(datum, REG_MAIN_MBX0_SEND);
            writel(MAIN_MBX0_SET_FLG, REG_MAIN_MBX_INT);
        }
        else if (!(val & MAIN_MBX1_SENSE_FLG)) {
            writel(datum, REG_MAIN_MBX1_SEND);
            writel(MAIN_MBX1_SET_FLG, REG_MAIN_MBX_INT);
        }
        else if (!(val & MAIN_MBX2_SENSE_FLG)) {
            writel(datum, REG_MAIN_MBX2_SEND);
            writel(MAIN_MBX2_SET_FLG, REG_MAIN_MBX_INT);
        }
        else if (!(val & MAIN_MBX3_SENSE_FLG)) {
            writel(datum, REG_MAIN_MBX3_SEND);
            writel(MAIN_MBX3_SET_FLG, REG_MAIN_MBX_INT);
        }
    }
    else if (mbxIdx == MBX0) {
        while (1) {
            val = readl(REG_MAIN_SENSE_MBX);
            if (!(val & MAIN_MBX0_SENSE_FLG))
                break;
        }
		
        writel(datum, REG_MAIN_MBX0_SEND);
        writel(MAIN_MBX0_SET_FLG, REG_MAIN_MBX_INT);
    }
    else if (mbxIdx == MBX1) {
        while (1) {
            val = readl(REG_MAIN_SENSE_MBX);
            if (!(val & MAIN_MBX1_SENSE_FLG))
                break;
        }		
        writel(datum, REG_MAIN_MBX1_SEND);
        writel(MAIN_MBX1_SET_FLG, REG_MAIN_MBX_INT);
    }
    else if (mbxIdx == MBX2) {
        while (1) {
            val = readl(REG_MAIN_SENSE_MBX);
            if (!(val & MAIN_MBX2_SENSE_FLG))
                break;
        }		
        writel(datum, REG_MAIN_MBX2_SEND);
        writel(MAIN_MBX2_SET_FLG, REG_MAIN_MBX_INT);
    }
    else if (mbxIdx == MBX3) {
        while (1) {
            val = readl(REG_MAIN_SENSE_MBX);
            if (!(val & MAIN_MBX3_SENSE_FLG))
                break;
        }		
        writel(datum, REG_MAIN_MBX3_SEND);
        writel(MAIN_MBX3_SET_FLG, REG_MAIN_MBX_INT);
    }
}

/** Try to find a free mailbox for delevery when mbxIdx equals PBX_MBX_MASK, else
 *  try the given one.
 */
void MbxHwSee_Tx(Uint32 mbxIdx, Uint32 datum)
{
    Uint32 val;
    
    if (mbxIdx == MBX_MASK) {
        while (1) {
            val = readl(REG_SEE_SENSE_MBX);
            if ((val & SEE_MBX_SENSE_MASK) != SEE_MBX_SENSE_MASK)
                break;
        }
        if (!(val & SEE_MBX0_SENSE_FLG)) {
            writel(datum, REG_SEE_MBX0_SEND);
            writel(SEE_MBX0_SET_FLG, REG_SEE_MBX_INT);
        }
        else if (!(val & SEE_MBX1_SENSE_FLG)) {
            writel(datum, REG_SEE_MBX1_SEND);
            writel(SEE_MBX1_SET_FLG, REG_SEE_MBX_INT);
        }
        else if (!(val & SEE_MBX2_SENSE_FLG)) {
            writel(datum, REG_SEE_MBX2_SEND);
            writel(SEE_MBX2_SET_FLG, REG_SEE_MBX_INT);
        }
        else if (!(val & SEE_MBX3_SENSE_FLG)) {
            writel(datum, REG_SEE_MBX3_SEND);
            writel(SEE_MBX3_SET_FLG, REG_SEE_MBX_INT);
        }
    }
    else if (mbxIdx == MBX0) {
        while (1) {
            val = readl(REG_SEE_SENSE_MBX);
            if (!(val & SEE_MBX0_SENSE_FLG))
                break;
        }
        writel(datum, REG_SEE_MBX0_SEND);
        writel(SEE_MBX0_SET_FLG, REG_SEE_MBX_INT);
    }
    else if (mbxIdx == MBX1) {
        while (1) {
            val = readl(REG_SEE_SENSE_MBX);
            if (!(val & SEE_MBX1_SENSE_FLG))
                break;
        }
        writel(datum, REG_SEE_MBX1_SEND);
        writel(SEE_MBX1_SET_FLG, REG_SEE_MBX_INT);
    }
    else if (mbxIdx == MBX2) {
        while (1) {
            val = readl(REG_SEE_SENSE_MBX);
            if (!(val & SEE_MBX2_SENSE_FLG))
                break;
        }
        writel(datum, REG_SEE_MBX2_SEND);
        writel(SEE_MBX2_SET_FLG, REG_SEE_MBX_INT);
    }
    else if (mbxIdx == MBX3) {
        while (1) {
            val = readl(REG_SEE_SENSE_MBX);
            if (!(val & SEE_MBX3_SENSE_FLG))
                break;
        }
        writel(datum, REG_SEE_MBX3_SEND);
        writel(SEE_MBX3_SET_FLG, REG_SEE_MBX_INT);
    }
}

Uint32 MbxHwMain_Rx(Uint32 mbxIdx)
{
    Uint32 val = 0;

    switch (mbxIdx) {
    case MBX0:
        val = (*(volatile Uint32*)REG_MAIN_MBX0_RECV);
        writel(MAIN_MBX0_CLR_FLG, REG_MAIN_MBX_INT);
        break;

    case MBX1:
        val = (*(volatile Uint32*)REG_MAIN_MBX1_RECV);
        writel(MAIN_MBX1_CLR_FLG, REG_MAIN_MBX_INT);
        break;

    case MBX2:
        val = (*(volatile Uint32*)REG_MAIN_MBX2_RECV);
        writel(MAIN_MBX2_CLR_FLG, REG_MAIN_MBX_INT);
        break;

    case MBX3:
        val = (*(volatile Uint32*)REG_MAIN_MBX3_RECV);
        writel(MAIN_MBX3_CLR_FLG, REG_MAIN_MBX_INT);
        break;

    default:
        break;
    }

    return val;
}

Uint32 MbxHwSee_Rx(Uint32 mbxIdx)
{
    Uint32 val = 0;

    switch (mbxIdx) {
    case MBX0:
        val = (*(volatile Uint32*)REG_SEE_MBX0_RECV);
        writel(SEE_MBX0_CLR_FLG, REG_SEE_MBX_INT);
        break;

    case MBX1:
        val = (*(volatile Uint32*)REG_SEE_MBX1_RECV);
        writel(SEE_MBX1_CLR_FLG, REG_SEE_MBX_INT);
        break;

    case MBX2:
        val = (*(volatile Uint32*)REG_SEE_MBX2_RECV);
        writel(SEE_MBX2_CLR_FLG, REG_SEE_MBX_INT);
        break;

    case MBX3:
        val = (*(volatile Uint32*)REG_SEE_MBX3_RECV);
        writel(SEE_MBX3_CLR_FLG, REG_SEE_MBX_INT);
        break;

    default:
        break;
    }

    return val;
}



