/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_t1.c
 *
 *  Description: This file contains functions to manage T1 send/rcv
 *		             
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  1. 
 *  
 ****************************************************************************/

#include "ali_smartcard_t1.h"

#define swap_nibbles(x) ( (x >> 4) | ((x & 0xF) << 4) )

/* T1 protocol initialization */
static UINT16 crctab[256] = 
{
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

static t1_state_t ali_smc_t1 = 
{
    .retries  = 3,
	.ifsc     = ATR_DEFAULT_IFSC,
	.ifsd     = ATR_DEFAULT_IFSD,
	.nr	      = 0,
	.ns	      = 0,
	.wtx	  = 0,
};

/* Returns LRC of data.*/
static inline UINT32 smc_t1_lrc(UINT8* data, UINT32 len, UINT8* rc) 
{
	UINT8	lrc = 0;

	while (len--)
		lrc ^= *data++;

	if (rc)
		*rc = lrc;

    /* Why return such a magic value ? */
	return 1;	 
}

/* Compute CRC of data.*/
static inline UINT32 smc_t1_crc(UINT8* data, UINT32 len, UINT8*rc) 
{
	UINT16 v = 0xFFFF;

	while (len--) {
		v = ((v >> 8) & 0xFF) ^ crctab[(v ^ *data++) & 0xFF];
	}

	if (rc) {
		rc[0] = (v >> 8) & 0xFF;
		rc[1] = v & 0xFF;
	}

    /* Why return such a magic value ? */
	return 2;
}

void smc_t1_set_checksum(t1_state_t *t1, UINT8 csum)
{
	switch (csum) {
	case IFD_PROTOCOL_T1_CHECKSUM_LRC:
		t1->rc_bytes = 1;
		t1->checksum = smc_t1_lrc;
		break;
	case IFD_PROTOCOL_T1_CHECKSUM_CRC:
		t1->rc_bytes = 2;
		t1->checksum = smc_t1_crc;
		break;
	}
}

INT32 smc_t1_set_param(t1_state_t *t1, INT32 type, INT32 value)
{
    switch (type) 
	{
		case IFD_PROTOCOL_T1_CHECKSUM_LRC:
		case IFD_PROTOCOL_T1_CHECKSUM_CRC:
			smc_t1_set_checksum(t1, type);
			break;
		case IFD_PROTOCOL_T1_IFSC:
			t1->ifsc = value;
			break;
		case IFD_PROTOCOL_T1_IFSD:
			t1->ifsd = value;
			break;
		case IFD_PROTOCOL_T1_STATE:
			t1->state = value;
			break;
		case IFD_PROTOCOL_T1_MORE:
			t1->more = value;
			break;
		default:
			smc_debug(KERN_ERR "SMC T1: In %s Unsupported parameter %ld\n", 
                          __func__, type);
			return -EINVAL;
	}

	return 0;
}

/* Set default T1 protocol parameters*/
static inline void smc_t1_set_defaults(t1_state_t *t1)
{
	t1->retries  = 3;
	t1->ifsc     = ATR_DEFAULT_IFSC;
	t1->ifsd     = ATR_DEFAULT_IFSD;
	t1->nr	  = 0;
	t1->ns	  = 0;
	t1->wtx	  = 0;
}

void smc_t1_init(t1_state_t *t1)
{
	smc_t1_set_param(t1, IFD_PROTOCOL_T1_CHECKSUM_LRC, 0);
	smc_t1_set_param(t1, IFD_PROTOCOL_T1_STATE, SENDING);
	smc_t1_set_param(t1, IFD_PROTOCOL_T1_MORE, FALSE);
}

int smc_t1_get(struct smartcard_private *tp)
{
    BUG_ON(NULL == tp);
    
    if (NULL != tp->T1)
        smc_debug(KERN_WARNING "SMC T1: In %s Already occupied one\n", __func__);

    tp->T1 = &ali_smc_t1;

    smc_t1_init(tp->T1);

    return 0;
}

/* T1 Communication */
static inline void smc_t1_buf_init(t1_buf_t *bp, void *mem, UINT32 len)
{
	memset(bp, 0, sizeof(*bp));
	bp->base = (UINT8*) mem;
	bp->size = len;
}

static inline void smc_t1_buf_set(t1_buf_t *bp, void *mem, UINT32 len)
{
	smc_t1_buf_init(bp, mem, len);
	bp->tail = len;
}

static inline UINT32 smc_t1_buf_available(t1_buf_t *bp)
{
	return bp->tail - bp->head;
}

/* check the block type by PCB*/
static inline UINT32 smc_t1_block_type(UINT8 pcb)
{
	switch (pcb & 0xC0) 
	{
		case T1_R_BLOCK:
			return T1_R_BLOCK;
		case T1_S_BLOCK:
			return T1_S_BLOCK;
		default:
			return T1_I_BLOCK;
	}
}

static inline void *smc_t1_buf_head(t1_buf_t *bp)
{
	return bp->base + bp->head;
}

/* Build checksum*/
static inline UINT32 smc_t1_compute_checksum(t1_state_t *t1,UINT8 *data, UINT32 len)
{
	return len + t1->checksum(data, len, data + len);
}

static inline UINT32 smc_t1_build(t1_state_t *t1, UINT8 *block, UINT8 dad, 
                             UINT8 pcb, t1_buf_t *bp, UINT32 *lenp)
{
	UINT32 len;
	INT8 more = FALSE;

	len = bp ? smc_t1_buf_available(bp) : 0;
	if (len > t1->ifsc) 
	{
		pcb |= T1_MORE_BLOCKS;
		len = t1->ifsc;
		more = TRUE;
	}

	/* Add the sequence number */
	switch (smc_t1_block_type(pcb)) 
	{
		case T1_R_BLOCK:
			pcb |= t1->nr << T1_R_SEQ_SHIFT;
			break;
		case T1_I_BLOCK:
			pcb |= t1->ns << T1_I_SEQ_SHIFT;
			t1->more = more;
			smc_debug(KERN_INFO "SMC T1: In %s More bit: %d\n", __func__, more);
			break;
	}

	block[0] = dad;
	block[1] = pcb;
	block[2] = len;

	if (len)
		memcpy(block + 3, smc_t1_buf_head(bp), len);
	if (lenp)
		*lenp = len;

	len = smc_t1_compute_checksum(t1, block, len + 3);

	/* memorize the last sent block */
	/* only 4 bytes since we are only interesed in R-blocks */
	memcpy(t1->previous_block, block, 4);

	return len;
}

/*update the T1 block wait time when receiving S-wtx request*/
static inline void smc_t1_update_BWT(t1_state_t *t1, UINT32 wtx)
{
	t1->BWT = wtx * t1->BWT;
}

static inline int smc_t1_card_error_check(struct smartcard_private *tp)
{
    if (0 == tp->inserted)
	{
		smc_debug(KERN_ERR "SMC TXRX: In %s Card not inserted!\n", __func__);
		return -EIO;
	}
	else if (1 != tp->reseted)
	{
		smc_debug(KERN_ERR "SMC TXRX: In %s Card not reseted!\n", __func__);
		return -EIO;
	}

    return 0;
}

static inline void smc_t1_restore_BWT(struct smartcard_private *tp)
{
    t1_state_t *t1 = NULL;
    
	if (tp != NULL)
	{
		t1 = (t1_state_t *)tp->T1;
		t1->BWT= tp->first_cwt;
	}
}

/* Send/receive block*/
static inline INT32 __smc_t1_xcv(struct smc_device *dev, UINT8 *sblock, UINT32 slen, 
                            UINT8 *rblock, UINT32 rmax, UINT32 *ractual)
{
	UINT32 m = 0;
	UINT32 u32_actual = 0;
	UINT8 dad = 0, dad1 = 0;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    t1_state_t *t1 = (t1_state_t *)tp->T1;
    
	if (1 != tp->T)
		return -EINVAL;

	if ((NULL == sblock) || (NULL == rblock))
		return -EINVAL;
    
	if (t1->wtx > 1)
	{
		/*set the new temporary timeout at WTX card request */
		smc_t1_update_BWT(t1,t1->wtx);
	}

     /************************************************************
      *Note: For some cards, the block head and body should be readed separately.
      *          If that happens, the below should be modified to write/read twice.
      ***********************************************************/
  	dad = *sblock;
	u32_actual = smc_dev_write(dev, sblock, slen);
	t1->wtx = 0;	/* reset to default value ??????????*/
	if (0 == u32_actual)
	{
		smc_debug(KERN_ERR "SMC T1: In %s Write error!\n", __func__);	
		return SMART_WRITE_ERROR;
	}
    
	/* Get the response en bloc */
	memset(rblock, 0, rmax);
	u32_actual = smc_dev_read(dev, rblock, rmax);
		
	if (0 == u32_actual)  /* current not implemented for parity check */
	{
		smc_debug(KERN_ERR "SMC T1: In %s Read-no answer!\n", __func__);
		return SMART_WRITE_ERROR;;
	}	
    
	if (smc_t1_card_error_check(tp) < 0)
	{
		smc_debug(KERN_ERR "SMC T1: In %s Read fetal error!\n", __func__);
		return -EIO;
	}

	dad1 = *rblock;
	invert(&dad1, 1);
	if ((u32_actual > 0) && (dad1 == dad))
	{
		m = rblock[2] + 3 + t1->rc_bytes;
		if (m < u32_actual)
			u32_actual = m;
	}
	*ractual = u32_actual;

	/* Restore initial timeout */
	smc_t1_restore_BWT(tp);
    
	return 0;
}

/*reconstruct the last sent block*/
static inline UINT32 smc_t1_rebuild(t1_state_t *t1, UINT8 *block)
{
	UINT8 pcb = t1->previous_block[1];
	
	/* copy the last sent block */
	if (T1_R_BLOCK == smc_t1_block_type(pcb))
		memcpy(block, t1->previous_block, 4);
	else
	{
		smc_debug(KERN_ERR "SMC T1: In %s Previous block was not R-Block: %02X\n", __func__, pcb);
		return 0;
	}

	return 4;
}

/* verify checksum*/
static inline INT32 smc_t1_verify_checksum(t1_state_t *t1, UINT8*rbuf,UINT32 len)
{
	unsigned char	csum[2];
	int		m, n;

	m = (int)len - t1->rc_bytes;
	n = t1->rc_bytes;

	if (m < 0)
		return 0;

	t1->checksum(rbuf, m, csum);
	if (!memcmp(rbuf + m, csum, n))
		return 1;

	return 0;
}

/* set number sequnce for I/R block*/
static inline UINT32 smc_t1_seq(UINT8 pcb)
{
	switch (pcb & 0xC0) 
	{
		case T1_R_BLOCK:
			return (pcb >> T1_R_SEQ_SHIFT) & 1;
		case T1_S_BLOCK:
			return 0;
		default:
			return (pcb >> T1_I_SEQ_SHIFT) & 1;
	}
}

static inline INT32 smc_t1_buf_get(t1_buf_t *bp, void *mem, UINT32 len)
{
	if (len > bp->tail - bp->head)
		return -EINVAL;
	if (mem)
		memcpy(mem, bp->base + bp->head, len);
	bp->head += len;
	return len;
}

static inline INT32 smc_t1_buf_put(t1_buf_t *bp, const void *mem, UINT32 len)
{
	if (len > bp->size - bp->tail) {
		bp->overrun = 1;
		return -EINVAL;
	}
	if (mem)
		memcpy(bp->base + bp->tail, mem, len);
	bp->tail += len;
	return len;
}

static inline INT32 smc_t1_buf_putc(t1_buf_t *bp, INT32 byte)
{
	UINT8 c = byte;

	return smc_t1_buf_put(bp, &c, 1);
}

int smc_t1_bi_comm(struct smc_device *dev, smc_t1_trans_t *p_t1_trans)
{
	t1_buf_t sbuf, rbuf, tbuf;
	UINT8	sblk[5];
	UINT32	u32_slen, u32_retries = 0, u32_resyncs, u32_sent_length = 0;
	UINT32	u32_last_send = 0;
	UINT32 	u32_actual = 0;
	UINT8   *sdata = NULL;
	UINT8   *rdata = NULL;    
    UINT8 pcb;
	INT32 n;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	t1_state_t *t1 = (t1_state_t *)tp->T1;

#define NO_MORE_RETRY_ERROR {if (0 == u32_retries) goto error;}
#define NO_MORE_RETRY_RESYNC {if (0 == u32_retries) goto resync;}
#define T1_R_BLOCK_NEED_REBUILD \
    if (T1_R_BLOCK == smc_t1_block_type(t1->previous_block[PCB])) \
	{ \
		u32_slen = smc_t1_rebuild(t1, sdata); \
		continue; \
	}
#define T1_R_BLOCK_BUILD_EDC \
    u32_slen = smc_t1_build(t1, sdata, \
				            p_t1_trans->dad, T1_R_BLOCK | T1_EDC_ERROR, \
				            NULL, NULL);
#define T1_R_BLOCK_BUILD_OTHER \
    u32_slen = smc_t1_build(t1, sdata, \
				            p_t1_trans->dad, T1_R_BLOCK | T1_OTHER_ERROR, \
				            NULL, NULL);
#define T1_R_BLOCK_BUILD \
    u32_slen = smc_t1_build(t1, sdata, \
				            p_t1_trans->dad, T1_R_BLOCK, \
				            NULL, NULL);

	sdata = &(t1->sdata[0]);
	rdata = &(t1->rdata[0]);
    
	if (0 == p_t1_trans->send_len)
		return -EINVAL;

	/* we can't talk to a dead card / reader. Reset it! */
	if (DEAD == t1->state)
	{
		smc_debug(KERN_ERR "SMC T1: In %s T=1 state machine is DEAD. "
                      "Reset the card firstly.\n", __func__);
		return -EIO;
	}

	t1->state = SENDING;
	u32_retries = t1->retries+1;
	u32_resyncs = 3;

	/* Initialize send/recv buffer */
	smc_t1_buf_set(&sbuf, (void *)p_t1_trans->send_buf, p_t1_trans->send_len);
	smc_t1_buf_init(&rbuf, p_t1_trans->rcv_buf, p_t1_trans->rcv_len);

	/* Send the first block */
	u32_slen = smc_t1_build(t1, sdata, p_t1_trans->dad, T1_I_BLOCK, &sbuf, &u32_last_send);

	for ( ; ; )
    {
    	u32_retries--;
		NO_MORE_RETRY_ERROR

		n = __smc_t1_xcv(dev, sdata, u32_slen, rdata, T1_BUFFER_SIZE, &u32_actual);
		if ((SMART_NO_ANSWER == n) || (SMART_WRITE_ERROR==n))
		{
			//NO_MORE_RETRY_ERROR
			continue;
		}

		if (n < 0) 
		{
			smc_debug(KERN_ERR "SMC T1: In %s Transmit / Receive failed\n", __func__);
			t1->state = DEAD;
			goto error;
		}

		if ((rdata[NAD] != swap_nibbles(p_t1_trans->dad)) || /* wrong NAD */
			(0xff == rdata[LEN]))	/* length == 0xFF (illegal) */
		{
			smc_debug(KERN_ERR "SMC T1: In %s Bad NAD, retry\n", __func__);
			/* ISO 7816-3 Rule 7.4.2 */
			//NO_MORE_RETRY_RESYNC
			/* ISO 7816-3 Rule 7.2 */
			T1_R_BLOCK_NEED_REBUILD
			continue;
		}

		if (!smc_t1_verify_checksum(t1, rdata, u32_actual)) 
		{
			/* ISO 7816-3 Rule 7.4.2 */
			//NO_MORE_RETRY_RESYNC
			/* ISO 7816-3 Rule 7.2 */
			T1_R_BLOCK_NEED_REBUILD
			T1_R_BLOCK_BUILD_EDC			
			continue;
		}

		pcb = rdata[PCB];
		switch (smc_t1_block_type(pcb)) 
		{
		case T1_R_BLOCK:
			if (0x00 != (rdata[LEN]) ||	/* length != 0x00 (illegal) */
			   (pcb & 0x20)			    /* b6 of pcb is set */
			   )
			{
				/* ISO 7816-3 Rule 7.4.2 */
				//NO_MORE_RETRY_RESYNC
				/* ISO 7816-3 Rule 7.2 */
				T1_R_BLOCK_NEED_REBUILD
				T1_R_BLOCK_BUILD_OTHER				
				continue;
			}

			if (((smc_t1_seq(pcb) != t1->ns) &&	/* wrong sequence number & no bit more */
				! t1->more)
			   )
			{
				/* ISO 7816-3 Rule 7.2 */
				T1_R_BLOCK_NEED_REBUILD
				/* ISO 7816-3 Rule 7.4.2 */
				//NO_MORE_RETRY_RESYNC
				T1_R_BLOCK_BUILD_OTHER
				continue;
			}

			if (RECEIVING == t1->state) 
            {
				/* ISO 7816-3 Rule 7.2 */
				T1_R_BLOCK_NEED_REBUILD
				T1_R_BLOCK_BUILD
				break;
			}

			/* If the card terminal requests the next
			 * sequence number, it received the previous
			 * block successfully */
			if (smc_t1_seq(pcb) != t1->ns) 
            {
				smc_t1_buf_get(&sbuf, NULL, u32_last_send);
				u32_sent_length += u32_last_send;
				u32_last_send = 0;
				t1->ns ^= 1;
			}

			/* If there's no data available, the ICC
			 * shouldn't be asking for more */
			if (0 == smc_t1_buf_available(&sbuf))
				goto resync;

			u32_slen = smc_t1_build(t1, sdata, p_t1_trans->dad, T1_I_BLOCK,
					                &sbuf, &u32_last_send);
			break;

		case T1_I_BLOCK:
			/* The first I-block sent by the ICC indicates
			 * the last block we sent was received successfully. */
			if (SENDING == t1->state) 
            {
				smc_t1_buf_get(&sbuf, NULL, u32_last_send);
				u32_last_send = 0;
				t1->ns ^= 1;
			}

			t1->state = RECEIVING;

			/* If the block sent by the card doesn't match
			 * what we expected it to send, reply with
			 * an R block */
			if (smc_t1_seq(pcb) != t1->nr) 
            {
				T1_R_BLOCK_BUILD_OTHER
				continue;
			}

			t1->nr ^= 1;

			if (smc_t1_buf_get(&rbuf, rdata + 3, rdata[LEN]) < 0)
			{
				goto error;
			}

			if (0 == (pcb & T1_MORE_BLOCKS))
				goto done;

            T1_R_BLOCK_BUILD
			break;

		case T1_S_BLOCK:
			if (T1_S_IS_RESPONSE(pcb) && RESYNCH == t1->state) 
            {
				/* ISO 7816-3 Rule 6.2 */
				/* ISO 7816-3 Rule 6.3 */
				t1->state = SENDING;
				u32_sent_length =0;
				u32_last_send = 0;
				u32_resyncs = 3;
				u32_retries = t1->retries;
				smc_t1_buf_init(&rbuf, p_t1_trans->rcv_buf, p_t1_trans->rcv_len);
				u32_slen = smc_t1_build(t1, sdata, p_t1_trans->dad, T1_I_BLOCK,
						                &sbuf, &u32_last_send);
				continue;
			}

			if (T1_S_IS_RESPONSE(pcb))
			{
				/* ISO 7816-3 Rule 7.4.2 */
				//NO_MORE_RETRY_RESYNC
				/* ISO 7816-3 Rule 7.2 */
				T1_R_BLOCK_NEED_REBUILD
                T1_R_BLOCK_BUILD_OTHER
				continue;
			}
			smc_t1_buf_init(&tbuf, sblk, sizeof(sblk));
            
			switch (T1_S_TYPE(pcb)) {
			case T1_S_RESYNC:
				if (0 != rdata[LEN])
				{
					T1_R_BLOCK_BUILD_OTHER
					continue;
				}
				/* the card is not allowed to send a resync. */
				goto resync;

			case T1_S_ABORT:
				if (0 != rdata[LEN])
				{
					T1_R_BLOCK_BUILD_OTHER
					continue;
				}
				/* ISO 7816-3 Rule 9 */
				goto resync;

			case T1_S_IFS:
				if (1 != rdata[LEN])
				{
					T1_R_BLOCK_BUILD_OTHER
					continue;
				}

				if (rdata[DATA] == 0)
					goto resync;
				t1->ifsc = rdata[DATA];
				smc_t1_buf_putc(&tbuf, rdata[DATA]);
				break;

			case T1_S_WTX:
				if (rdata[LEN] != 1)
				{
					T1_R_BLOCK_BUILD_OTHER
					continue;
				}

				t1->wtx = rdata[DATA];
				smc_t1_buf_putc(&tbuf, rdata[DATA]);
				break;

			default:
				smc_debug(KERN_INFO "SMC T1: In %s T=1: Unknown S block type 0x%02x\n", 
                              __func__, T1_S_TYPE(pcb));
				goto resync;
			}

			u32_slen = smc_t1_build(t1, sdata, p_t1_trans->dad,
				                    T1_S_BLOCK | T1_S_RESPONSE | T1_S_TYPE(pcb),
				                    &tbuf, NULL);
		}

		/* Everything went just splendid */
		u32_retries = t1->retries;
		continue;

resync:
		/* the number or resyncs is limited, too */
		/* ISO 7816-3 Rule 6.4 */
		if (0 == u32_resyncs)
			goto error;

		/* ISO 7816-3 Rule 6 */
		u32_resyncs--;
		t1->ns = 0;
		t1->nr = 0;
		u32_slen = smc_t1_build(t1, sdata, p_t1_trans->dad, T1_S_BLOCK | T1_S_RESYNC, NULL, NULL);
		t1->state = RESYNCH;
		t1->more = FALSE;
		u32_retries = 1;
		continue;
	}

done:
	return smc_t1_buf_available(&rbuf);

error:
	t1->state = DEAD;
	return -EINVAL;
}

/* T1 xcv */
int smc_t1_xcv(struct smc_device *dev, smc_t1_xcv_t *p_t1_xcv)
{
    return __smc_t1_xcv(dev, p_t1_xcv->sblock, p_t1_xcv->slen,
                        p_t1_xcv->rblock, p_t1_xcv->rmax, &p_t1_xcv->actual_size);
}

/* T1 negociate ifsd */
int smc_t1_negociate_ifsd(struct smc_device*dev, smc_t1_nego_ifsd_t *p_t1_nego_ifsd)
{
	t1_buf_t	sbuf;
	UINT8 * sdata = NULL;
	UINT32 u32_slen;
	UINT32 u32_retries;
	UINT32 snd_len;
	INT32 n;
	UINT8 snd_buf[1];
	UINT32 u32_actual =  0;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	t1_state_t *t1 = (t1_state_t *)tp->T1;

	sdata = &(t1->sdata[0]);
	u32_retries = t1->retries;

	/* S-block IFSD request */
	snd_buf[0] = p_t1_nego_ifsd->ifsd;
	snd_len = 1;

	/* Initialize send/recv buffer */
	smc_t1_buf_set(&sbuf, (void *) snd_buf, snd_len);

	for ( ; ; )
	{
		/* Build the block */
		u32_slen = smc_t1_build(t1, sdata, p_t1_nego_ifsd->dad, T1_S_BLOCK | T1_S_IFS, &sbuf, NULL);
		/* Send the block */
		n = __smc_t1_xcv(dev, sdata, u32_slen, sdata, T1_BUFFER_SIZE, &u32_actual);

		u32_retries--;
		/* ISO 7816-3 Rule 7.4.2 */
		if (0 == u32_retries)
			goto error;

		if (n < 0)
		{
			goto error;
		}

		if ((SMART_NO_ANSWER == n) || (SMART_WRITE_ERROR == n)							/* Parity error */
			|| (sdata[DATA] != p_t1_nego_ifsd->ifsd)				                    /* Wrong ifsd received */
			|| (sdata[NAD] != swap_nibbles(p_t1_nego_ifsd->dad))	                    /* wrong NAD */
			|| (!smc_t1_verify_checksum(t1, sdata, u32_actual))	                                /* checksum failed */
			|| (u32_actual != (UINT32)4 + t1->rc_bytes)				                        /* wrong frame length */
			|| (sdata[LEN] != 1)					                                    /* wrong data length */
			|| (sdata[PCB] != (T1_S_BLOCK | T1_S_RESPONSE | T1_S_IFS)))                 /* wrong PCB */
			continue;

		/* no more error */
		goto done;
	}

done:
	return 0;

error:
	t1->state = DEAD;
	return -EINVAL;
}



