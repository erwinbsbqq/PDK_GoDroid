/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_txrx.h
 *
 *  Description: Head file of smart card txrx
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  0. 
 ****************************************************************************/

#ifndef  __ALI_SMARTCARD_TXRX_H__
#define  __ALI_SMARTCARD_TXRX_H__

#include "ali_smartcard.h"
#include "ali_smartcard_misc.h"
#include "ali_smartcard_atr.h"

/* Transfer MACROs definitions */
#define WRITE_RX_CNT(io_addr, val)					\
    	do												\
    	{												\
    		writeb(val, io_addr + REG_RFIFO_CNT);	\
    		writeb((readb(io_addr + REG_RX_CNT8) & 0xfe) | \
    		       (((val) & 0x100)>>8),      \
    		       io_addr + REG_RX_CNT8);	\
    	}												\
    	while(0)
#define READ_RX_CNT(io_addr)		\
        (readb(io_addr + REG_RFIFO_CNT) | \
        ((readb(io_addr + REG_RX_CNT8) & 0x1)<<8))

static inline UINT16 smc_read_rx(struct smartcard_private *tp,
                                    void __iomem *p_io_base)
{
	UINT16 val; 
    
	if (tp->smc_rx_fifo_size > 64)
		val = READ_RX_CNT(p_io_base);
	else
		val = readb(p_io_base + REG_RFIFO_CNT);
    
	return val;
}

static inline void smc_write_rx(struct smartcard_private *tp,
                                  void __iomem *p_io_base, UINT16 val)
{
	if (tp->smc_rx_fifo_size > 64)
		WRITE_RX_CNT(p_io_base, val);
	else
		writeb(((UINT8)val), p_io_base + REG_RFIFO_CNT);
}

static inline UINT16 smc_read_tx(struct smartcard_private *tp,
                                   void __iomem *p_io_base)
{
	UINT16 val; 
    
	if (tp->smc_rx_fifo_size > 64)
		val = readw(p_io_base + REG_TX_CNT);
	else
		val = readb(p_io_base + REG_TX_CNT);
    
	return val;
}

static inline void smc_write_tx(struct smartcard_private *tp,
                                  void __iomem *p_io_base, UINT16 val)
{
	if (tp->smc_rx_fifo_size > 64)
		writew(val, p_io_base + REG_TX_CNT);
	else
		writeb(((UINT8)val), p_io_base + REG_TX_CNT);
}

static inline void invert(UINT8 *data, INT32 n)
{
	INT32 i;
	static UINT8 swaptab[16] = {15, 7, 11, 3, 13, 5, 9, 1, 14, 6, 10, 2, 12, 4, 8, 0};
    
	for (i = n - 1; i >= 0; i--)
		data[i] = (swaptab[data[i] & 0x0F]<<4) | swaptab[data[i]>>4];
}


/* Smart card TX / RX buffer allocation */
extern int smc_txrx_buf_alloc(struct smartcard_private *tp);
extern void smc_txrx_buf_free(struct smartcard_private *tp);
extern void smc_txrx_buf_reset(struct smartcard_private *tp);

/* Smart card TX / RX transfer data */
extern int smc_txrx_transfer_data(struct smc_device *dev, void __iomem *p,
                                    UINT8 *buffer, UINT16 size, UINT8 *recv_buffer, 
                                    UINT16 reply_num, UINT16 *actsize);

/* Smart TX / RX data operation */
extern ssize_t smc_txrx_read(struct smartcard_private *tp, void __iomem *p);
extern int smc_txrx_data_timeout(struct smartcard_private *tp, size_t size);

/* IS0 7816 transferation command */
extern int smc_txrx_iso_transfer(struct smc_device *dev, smc_iso_transfer_t *p_iso_transfer);
extern int smc_txrx_iso_transfer_t1(struct smc_device *dev, smc_iso_transfer_t *p_iso_transfer);

#endif


