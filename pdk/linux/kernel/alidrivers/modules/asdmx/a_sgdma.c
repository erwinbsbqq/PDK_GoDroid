
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>

#include "dmx_internal.h"


extern __u32 __G_MM_SGDMA_MEM_START;

struct A_SgdmaDev g_A_SgdmaDev;


__inline __u8 A_SgdmaChHwGetWr
(
    struct A_SgdmaDev    *Sgdma,
    struct A_SdmaChannel *Ch
)
{
    return(ioread8(Ch->NodeWrReg));
}



__inline void A_SgdmaChHwSetWr
(
    struct A_SgdmaDev    *Sgdma,
    struct A_SdmaChannel *Ch,
    __u8                  Wr
)
{ 
    iowrite8(Wr, Ch->NodeWrReg);

    return;
}



__inline __u8 A_SgdmaChHwGetRd
(
    struct A_SgdmaDev    *Sgdma,
    struct A_SdmaChannel *Ch
)
{
    return(ioread8(Ch->NodeRdReg));
}





__inline void A_SgdmaChHwSetRd
(
    struct A_SgdmaDev    *Sgdma,
    struct A_SdmaChannel *Ch,
    __u8                  Rd
)
{
    iowrite8(Rd, Ch->NodeRdReg);

    return;
}





__inline __s32 A_SgdmaChHwIsEnable
(
    struct A_SgdmaDev *Sgdma,
    __u32              ChIdx
)
{
    __u8 OnOffReg;

    OnOffReg = ioread8(Sgdma->ChEnReg);

    if ((OnOffReg & (1 << ChIdx)) != 0)
    {
        /* HW channel is enabled. */
        return(1);
    }
 
    /* HW channel is not enabled. */
    return(0);
}





__inline __s32 A_SgdmaChHwEnable
(
    struct A_SgdmaDev *Sgdma,
    __u32              ChIdx
)
{
    __u8                  OrigReg;
    __u8                  NewReg;
    struct A_SdmaChannel *Ch;

    Ch = &Sgdma->Ch[ChIdx];

    iowrite32(0, Ch->NodeWrReg);

    iowrite32(0, Ch->NodeRdReg);

    OrigReg = ioread8(Sgdma->ChEnReg);

    NewReg = OrigReg | (0x1 << ChIdx);

    printk("%s,I:%x,O:%x,N:%x\n", __FUNCTION__, ChIdx, OrigReg, NewReg);

    iowrite8(NewReg, Sgdma->ChEnReg);

    return(0);
}



__inline __s32 A_SgdmaChHwDisable
(
    struct A_SgdmaDev *Sgdma,
    __u32              ChIdx
)
{
    __u8                  OrigReg;
    __u8                  NewReg;
    struct A_SdmaChannel *Ch;

    Ch = &Sgdma->Ch[ChIdx];

    OrigReg = ioread8(Sgdma->ChEnReg);

    NewReg = OrigReg & (~(0x1 << ChIdx));

    printk("%s,I:%x,O:%x,N:%x\n", __FUNCTION__, ChIdx, OrigReg, NewReg);

    iowrite8(NewReg, Sgdma->ChEnReg);

    dma_cache_wback((__u32)Ch->NodeBufReg, 32);

    return(0);
}





__inline __s32 A_SgdmaChHwNodeSetup
(
    struct A_SgdmaDev    *Sgdma,
    struct A_SdmaChannel *Ch,
    __u8                  NodeIdx,
    void                 *Dest,
    void                 *Src,
    __u32                 Len
)
{
    //__u32       LenAndFlag;
    void __iomem *NodeBase;

    NodeBase = Ch->NodeBuf + (NodeIdx * A_SGDMA_CPY_NODE_LEN);

    iowrite32((__u32)Src & 0x7FFFFFFF, NodeBase + 0);

    iowrite32((__u32)Dest & 0x7FFFFFFF, NodeBase + 4);

    /* TODO: may also config flag regs. */

    iowrite32(Len - 1, NodeBase + 8);

    iowrite32(0, NodeBase + 12);

    return(0);
}



#if 0
__s32 A_SgdmaChHwStartCodeEn
(

)
{




}
#endif




__s32 A_SgdmaChOpen
(
#if 0
    enum A_SGDMA_CH_TYPE       Type,
    struct A_SgdmaStarcodeCfg *ScCfg
#else
    enum A_SGDMA_CH_TYPE       Type

#endif
)
{
    struct A_SgdmaDev    *Sgdma;
    struct A_SdmaChannel *Ch;
    __u32                 ChIdx;
    __s32                 ChIsOn;   

    Sgdma = &g_A_SgdmaDev;

    /* HW channel 0 is reserved for SEE, so we started with channel 1. */
    for (ChIdx = 1; ChIdx < A_SGDMA_CHANNEL_CNT; ChIdx++)
    {
        Ch = &Sgdma->Ch[ChIdx];

        /* Must get HW channel status from HW register since SGDMA HW is shared
         * by main CPU and SEE.
         */
        ChIsOn = A_SgdmaChHwIsEnable(Sgdma, ChIdx);

        if ((0 == ChIsOn) && (Ch->Type == Type))
        {
            break;
        }
    }

    if (ChIdx >= A_SGDMA_CHANNEL_CNT)
    {
        return(-EMFILE);
    }

    /* Init HW registers for this channel.*/
    iowrite32((__u32)Ch->NodeBuf, Ch->NodeBufReg);

#if 0
    iowrite32(Ch->NodeCnt, Ch->NodeCntReg);

    iowrite32(0, Ch->NodeWrReg);

    iowrite32(0, Ch->NodeRdReg);
#else
    iowrite8(Ch->NodeCnt, Ch->NodeCntReg);

    A_SgdmaChHwSetRd(Sgdma, Ch, 0);

    A_SgdmaChHwSetWr(Sgdma, Ch, 0);
#endif

    Ch->FlushSpinCnt = 0;

    Ch->FlushSleepCnt = 0;

    A_SgdmaChHwEnable(Sgdma, ChIdx);



#if 0
    /* Init regs for start code detection if requested. */
    if ((A_SGDMA_CH_TYPE_SC == Ch->Type) && (NULL != ScCfg))
    {
        /* Init HW registers for this channel.*/
        iowrite32((__u32)Ch->StartCodeBuf, Ch->StartCodeBufReg);
    
        iowrite32(Ch->StartCodeBufLen, Ch->StartCodeBufLenReg);
    
        iowrite16(0, Ch->StartCodeBufRdReg);
    
        iowrite16(0, Ch->StartCodeBufWrReg);


        //iowrite16(ScCfg->UpBound, Ch->StartCodeUpBoundReg);
    
        //iowrite16(ScCfg->UpBound, Ch->StartCodeLowBoundReg);

        //TODO: enalbe

        A_SgdmaChHwStartCodeRange(ScCfg);

        A_SgdmaChHwStartCodeEn(Sgdma, ChIdx);
    }
#endif

    Ch->Status = A_SGDMA_CH_STATUS_RUN;

    return(ChIdx);
}





__s32 A_SgdmaChClose
(
    __u32 ChIdx
)
{
    struct A_SgdmaDev    *Sgdma;
    struct A_SdmaChannel *Ch;
    __s32                 ChIsOn;

    Sgdma = &g_A_SgdmaDev;

    if (ChIdx >= A_SGDMA_CHANNEL_CNT)
    {
        return(-EINVAL);
    }

    Ch = &Sgdma->Ch[ChIdx];

    ChIsOn = A_SgdmaChHwIsEnable(Sgdma, ChIdx);

    if (0 == ChIsOn)
    {
        return(0);
    }

    A_SgdmaChHwDisable(Sgdma, ChIdx);

    Ch->Status = A_SGDMA_CH_STATUS_IDLE;

    return(0);
}





__s32 A_SgdmaChCpy
(
    __u32   ChIdx,
    void   *Dest,
    void   *Src,
    __u32   Len
)
{
    struct A_SgdmaDev    *Sgdma;
    struct A_SdmaChannel *Ch;
    __u8                  Wr;
    __u8                  Rd;
    __u8                  NextWr;
    __u32                 Cnt;

    Sgdma = &g_A_SgdmaDev;

    if (ChIdx >= A_SGDMA_CHANNEL_CNT)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(-EINVAL);
    }

    Ch = &Sgdma->Ch[ChIdx];

    if (Ch->Status != A_SGDMA_CH_STATUS_RUN)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(-EINVAL);
    }

    /* SGDMA can only access physical memory adrress below 256M.*/
    if (((__u32)Dest & 0x7FFFFFFF) > 0x10000000)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(-EINVAL);
    }

    /* SGDMA can only access physical memory adrress below 256M.*/
    if (((__u32)Src & 0x7FFFFFFF) > 0x10000000)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(-EINVAL);
    }

    Wr = A_SgdmaChHwGetWr(Sgdma, Ch);

    Rd = A_SgdmaChHwGetRd(Sgdma, Ch);
    
	NextWr = Wr + 1;

    if (NextWr == Ch->NodeCnt)
    {
        NextWr = 0;
    }

    /* Queue full. */
    if (NextWr == Rd)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(-EAGAIN);
    }

    /* The M3701G chip I'm using may have HW bug, a copy node has to be
     * configured at least twice to make it work correctly.
     */
    for (Cnt = 0; Cnt < 2; Cnt++)
    {
        /* Configure once. */
        A_SgdmaChHwNodeSetup(Sgdma, Ch, Wr, Dest, Src, Len);
    }

    /* Tell SGDMA HW to copy this node. */
    A_SgdmaChHwSetWr(Sgdma, Ch, NextWr);

    return(Len);
}



__s32 A_SgdmaChFlush
(
    __u32                   ChIdx,
    enum A_SGDMA_FLUSH_MODE Mode
)
{
    struct A_SgdmaDev    *Sgdma;
    struct A_SdmaChannel *Ch;
    __u8                  Wr;
    __u8                  Rd;

    Sgdma = &g_A_SgdmaDev;

    Ch = &Sgdma->Ch[ChIdx];

    /* Wait until all copy done.(copy queue empty) */
    for (;;)
    {
        Wr = A_SgdmaChHwGetWr(Sgdma, Ch);
    
        Rd = A_SgdmaChHwGetRd(Sgdma, Ch);

        if (Wr == Rd)
        {
            break;
        }
        else if (Mode == ALI_SGDMA_FLUSH_MODE_SPINWAIT)
        {
            Ch->FlushSpinCnt++;
        }  
        else if (Mode == ALI_SGDMA_FLUSH_MODE_SLEEPWAIT)
        {
            Ch->FlushSleepCnt++;

            msleep(2);
        }
    }

    return(0);
}




#if 0

__s32 A_SgdmaChGetStartCode
(

)
{


    Wr = A_SgdmaChHwGetWr(Sgdma, Ch);

    Rd = A_SgdmaChHwGetRd(Sgdma, Ch);
    
	NextWr = Wr + 1;

    if (NextWr == Ch->NodeCnt)
    {
        NextWr = 0;
    }


}





__s32 A_SgdmaChFlushStartCode
(

)
{




}
#endif







__s32 A_SgdmaInit
(
    void
)
{
    __u32                 Idx;
    struct A_SgdmaDev    *Sgdma;
    struct A_SdmaChannel *Ch;
    void __iomem         *NodeBuf;
    void __iomem         *StartCodeBuf;

    Sgdma = &g_A_SgdmaDev;

	Sgdma->Base = (void __iomem *)A_SGDMA_HW_REG_BASE;

    Sgdma->ChEnReg = Sgdma->Base + 0x48;

    Sgdma->ChEmptyReg = Sgdma->Base + 0x46;

    /* Clear up all regs. */
    for (Idx = 0; Idx < A_SGDMA_HW_REG_RANGE; Idx++)
    {
        iowrite8(0, Sgdma->Base + Idx);
    }

    /* Set memory R/W latency to biggest toleration. */
    iowrite16(0xFFFF, Sgdma->Base + 0xC0);

    /* Copy node chain address for this channel. */
    NodeBuf = (void __iomem *)__G_MM_SGDMA_MEM_START;

    /* For startcode detection. */
    StartCodeBuf = NodeBuf + (A_SGDMA_CPY_NODE_CNT * A_SGDMA_CPY_NODE_LEN *
                   A_SGDMA_CHANNEL_CNT); 

    /* Init channels. */
    for (Idx = 0; Idx < A_SGDMA_CHANNEL_CNT; Idx++)
    {
        Ch = &Sgdma->Ch[Idx];

        Ch->Status = A_SGDMA_CH_STATUS_IDLE;

        Ch->id = Idx;

        if (Idx < A_SGDMA_CHANNEL_CNT_SC)
        {
            Ch->Type = A_SGDMA_CH_TYPE_SC;
        }
        else
        {
            Ch->Type = A_SGDMA_CH_TYPE_NO_SC;
        }
   
        /* Copy node chain address for this channel. */
        Ch->NodeBuf = NodeBuf + (Idx * A_SGDMA_CPY_NODE_CNT *
                      A_SGDMA_CPY_NODE_LEN);

        Ch->NodeCnt = A_SGDMA_CPY_NODE_CNT;

        /* HW base address registers for this channel. */
        Ch->NodeBufReg = Sgdma->Base + 0x0 + (Idx * 4);

        Ch->NodeCntReg = Sgdma->Base + 0x20 + Idx;

        Ch->NodeWrReg = Sgdma->Base + 0x28 + Idx;

        Ch->NodeRdReg = Sgdma->Base + 0x30 + Idx;



#if 0
        /* */
        if (Idx < A_SGDMA_CHANNEL_CNT_SC)
        {
            Ch->StartCodeBuf = StartCodeBuf + (Idx * A_SGDMA_STARTCODE_BUF_LEN);

            Ch->StartCodeBufLen = A_SGDMA_STARTCODE_BUF_LEN;


            Ch->StartCodeBufReg = Sgdma->Base + 0x60 + (Idx * 4);

            Ch->StartCodeBufLenReg = Sgdma->Base + 0x70 + (Idx * 2);

            Ch->StartCodeBufRdReg = Sgdma->Base + 0x78 + (Idx * 2);

            Ch->StartCodeBufWrReg = Sgdma->Base + 0x80 + (Idx * 2);
        }
#endif

        
    }

    /* Enalbe SGDMA. */
    iowrite8(1, Sgdma->Base + 0x4B);

    return(0);
}


