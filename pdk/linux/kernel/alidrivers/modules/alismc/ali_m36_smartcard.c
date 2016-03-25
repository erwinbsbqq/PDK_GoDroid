
#include <linux/workqueue.h>
#include <linux/ali_transport.h>
#include <linux/version.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <ali_reg.h>
#include <ali_cache.h>
#include <linux/delay.h>	
#include <linux/slab.h>


#include "ali_m36_smartcard.h"
#include "ali_m36_smc_t1_buf.h"
#if defined(CONFIG_ALI_CHIP_M3921)
#include <ali_interrupt.h>
#else
#include <asm/mach-ali/m36_irq.h>
#endif


static int ali_m36_smartcard_open(struct inode *inode, struct file *file);
static int ali_m36_smartcard_release(struct inode *inode, struct file *file);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static int ali_m36_smartcard_ioctl(struct file *file,
			   unsigned int cmd, unsigned long parg);
#else
static int ali_m36_smartcard_ioctl(struct inode *inode, struct file *file,
			   unsigned int cmd, unsigned long parg);
#endif

static INT32 smc_dev_read(struct smartcard_m36_private *priv, UINT8 *buffer, INT16 size, INT16 *actsize);
static INT32 smc_dev_write(struct smartcard_m36_private *priv, UINT8 *buffer, INT16 size, INT16 *actsize);
static void invert(UINT8 *data, INT32 n);
static INT32 smc_dev_deactive(struct smartcard_m36_private *priv);
static void smc_init_hw(struct smartcard_m36_private *priv);
static INT32 smc_dev_transfer_data(struct smartcard_m36_private *priv, UINT8 *buffer, UINT16 size, UINT8 *recv_buffer, UINT16 reply_num, UINT16 *actsize);


extern int gpio_direction_output(unsigned gpio, int value);


#define ALI_SMARTCARD_DEVICE_NAME  "ali_m36_smartcard_0"

#define SMC_RESET_L_INTERVAL	60 // t >= 40000/f = 11.2ms, tongfang card need t < 500ms

#define WAITTIMEOUT		100000

#define	SMC_DEV_NUM	1

#define FORCE_TX_RX_THLD		2

static struct smartcard_m36_private ali_m36_smartcard_private;
static ca_msg_t ali_smc_msg;
static struct work_struct g_smc_wq;
static UINT32 scr_sys_clk = 0;
static UINT32 pwm_sys_clk = 0;
static UINT16 smc_tx_fifo_size = 256;
static UINT16 smc_rx_fifo_size = 256;


/********************************************
T1 related Macro define
*********************************************/
/* I block */
#define T1_I_SEQ_SHIFT		6

/* R block */
#define T1_IS_ERROR(pcb)	((pcb) & 0x0F)
#define T1_EDC_ERROR		0x01
#define T1_OTHER_ERROR		0x02
#define T1_R_SEQ_SHIFT		4

/* S block stuff */
#define T1_S_IS_RESPONSE(pcb)	((pcb) & T1_S_RESPONSE)
#define T1_S_TYPE(pcb)		((pcb) & 0x0F)
#define T1_S_RESPONSE		0x20
#define T1_S_RESYNC		0x00
#define T1_S_IFS		0x01
#define T1_S_ABORT		0x02
#define T1_S_WTX		0x03

#define swap_nibbles(x) ( (x >> 4) | ((x & 0xF) << 4) )

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif


#define NAD 0
#define PCB 1
#define LEN 2
#define DATA 3

/* internal state, do not mess with it. */
/* should be != DEAD after reset/init */
enum 
{
	SENDING, 
	RECEIVING, 
	RESYNCH, 
	DEAD
};
/********************************************
T1 related Macro define ending 
*********************************************/

static struct /*The default setting is for M3602 serial*/
{
	UINT32 io_base;
	int    irq;
	struct smartcard_m36_private * priv;
	UINT32 pwm_addr;
	UINT8 pwm_sel_ofst;
	UINT8 pwm_seh_ofst;
	UINT8 pwm_gpio_ofst;
	UINT8 pwm_cfg_ofst;
	UINT8 pwm_frac_ofst;
}smc_dev_set[SMC_DEV_NUM] = 
{
	#if defined(CONFIG_ALI_CHIP_M3921)
	{0x18018800,INT_ALI_SCR1, NULL, 0x18000000, 0, 0, 0, 0, 0}//,
	//{0x18018900,INT_ALI_SCR2, NULL, 0x18000000, 0, 0, 0, 0, 0}
	#else
	{0x18018800,M36_SYS_IRQ_BASE + 20, NULL, 0x18000000, 0, 0, 0, 0, 0}//,
	//{0x18018900,M36_SYS_IRQ_BASE + 21, NULL, 0x18000000, 0, 0, 0, 0, 0}
	#endif
};

#define F_RFU	0
#define D_RFU	0
#define I_RFU	0

#define ATR_ID_NUM		16
#define ATR_FI_NUM		16
#define ATR_DI_NUM		16
#define ATR_I_NUM		4
static UINT32 atr_num_ib_table[ATR_ID_NUM] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

static UINT32 atr_f_table[ATR_FI_NUM] ={372,372,558,744,1116,1488,1860,F_RFU,F_RFU,512,768,1024,1536,2048,F_RFU,F_RFU};

static UINT32 atr_d_table[ATR_DI_NUM] ={D_RFU,1,2,4,8,16,32,D_RFU,12,20,D_RFU,D_RFU,D_RFU,D_RFU,D_RFU,D_RFU};

static UINT32 atr_i_table[ATR_I_NUM] = {25, 50, 100, 0};
#ifdef min
#undef min
#define min( a, b )   ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#endif
/* Correct Table? */

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

static unsigned char g_smc_debug = 0;
#define SMC_PRINTK(fmt, args...)			\
{										\
	if (0 !=  g_smc_debug)					\
	{									\
		printk(fmt, ##args);					\
	}									\
}

#define SMC_ERR_PRINTK(fmt, args...)		\
{										\
	printk(fmt, ##args);						\
}

#define SMC_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) SMC_PRINTK(" %02x",*((data)+i)); \
                         SMC_PRINTK("\n"); }


static UINT32 osal_get_tick(void)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	return (tv.tv_sec*1000) + (tv.tv_usec/1000);
}

static INT32 osal_flag_wait_tmp(UINT32 *flgptn, struct smartcard_m36_private *priv, UINT32 flag, UINT32 tmo)
{
	UINT32 tick_tmp;
	INT32 result;
	tick_tmp=osal_get_tick();
	while(1)
	{
		if(0!=(priv->smc_flag_id & flag))
		{
			result=OSAL_E_OK;
			*flgptn=priv->smc_flag_id;
			return result;
		}
		if((0xffffffff!=tmo)&&((osal_get_tick()-tick_tmp)>tmo))
		{
			result=OSAL_E_TIMEOUT;
			*flgptn=priv->smc_flag_id;
			return result;
		}
		else
			msleep(1);
	}
}


static void smc_wq_handler (void *data)
{
	UINT8 msg = 0;
	UINT32 socket_port_id = 0;
	
	
	socket_port_id = ali_m36_smartcard_private.hsr;
	msg = ali_m36_smartcard_private.inserted;			
	ali_transport_send_msg(socket_port_id, &msg, 1);

	SMC_PRINTK("[ %s ], id = %d, inserted = %d\n", __FUNCTION__ , socket_port_id, msg);
}


/* Returns LRC of data.*/
static UINT32 t1_lrc(UINT8* data, UINT32 len, UINT8* rc) 
{
	UINT8	lrc = 0;

	while (len--)
		lrc ^= *data++;

	if (rc)
		*rc = lrc;
	return 1;
	 
}

/* Compute CRC of data.*/
static UINT32 t1_crc(UINT8* data, UINT32 len, UINT8*rc) 
{
	UINT16 v = 0xFFFF;

	while (len--) {
		v = ((v >> 8) & 0xFF) ^ crctab[(v ^ *data++) & 0xFF];
	}

	if (rc) {
		rc[0] = (v >> 8) & 0xFF;
		rc[1] = v & 0xFF;
	}

	return 2;
}

/***********************************************************************
 *
 *Below are functions about ISO/IEC 7816-3 Protocol T=1
 *The main interface for T1 is the function t1_transceive(), it is used to translate/receive
 *T1 data
 *
 *******************************************************************/

/*choose the error check algorithm*/
static void t1_set_checksum(t1_state_t *t1, UINT8 csum)
{
	switch (csum) {
	case IFD_PROTOCOL_T1_CHECKSUM_LRC:
		t1->rc_bytes = 1;
		t1->checksum = t1_lrc;
		break;
	case IFD_PROTOCOL_T1_CHECKSUM_CRC:
		t1->rc_bytes = 2;
		t1->checksum = t1_crc;
		break;
	}
}


 /* Set default T1 protocol parameters*/
static void t1_set_defaults(t1_state_t *t1)
{
	t1->retries  = 3;
	t1->ifsc     = ATR_DEFAULT_IFSC;
	t1->ifsd     = ATR_DEFAULT_IFSD;
	t1->nr	  = 0;
	t1->ns	  = 0;
	t1->wtx	  = 0;
}


/* set parmaters for T1 protocol*/
static INT32 t1_set_param(t1_state_t *t1, INT32 type, INT32 value)
{
	switch (type) 
	{
		case IFD_PROTOCOL_T1_CHECKSUM_LRC:
		case IFD_PROTOCOL_T1_CHECKSUM_CRC:
			t1_set_checksum(t1, type);
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
			T1PRINTF("Unsupported parameter %d\n", type);
			return -1;
	}

	return RET_SUCCESS;
}

#if 0
/* check the block type by PCB*/
static UINT32 t1_block_type(UINT8 pcb)
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

/* set number sequnce for I/R block*/
static UINT32 t1_seq(UINT8 pcb)
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

/* Build checksum*/
static UINT32 t1_compute_checksum(t1_state_t *t1,UINT8 *data, UINT32 len)
{
	return len + t1->checksum(data, len, data + len);
}


/* verify checksum*/
static INT32 t1_verify_checksum(t1_state_t *t1, UINT8*rbuf,UINT32 len)
{
	unsigned char	csum[2];
	int		m, n;

	m = len - t1->rc_bytes;
	n = t1->rc_bytes;

	if (m < 0)
		return 0;

	t1->checksum(rbuf, m, csum);
	if (!memcmp(rbuf + m, csum, n))
		return 1;

	return 0;
}
#endif
/*init T1 */
static INT32 t1_init(t1_state_t *t1)
{
	t1_set_defaults(t1);
	t1_set_param(t1, IFD_PROTOCOL_T1_CHECKSUM_LRC, 0);
	t1_set_param(t1, IFD_PROTOCOL_T1_STATE, SENDING);
	t1_set_param(t1, IFD_PROTOCOL_T1_MORE, FALSE);

	return 0;
}

/*Detach t1 protocol*/
#if 0
static void t1_release(t1_state_t *t1)
{
}

/*update the T1 block wait time when receiving S-wtx request*/
static void t1_update_BWT(t1_state_t *t1, UINT32 wtx)
{

	t1->BWT = wtx * t1->BWT;
	T1PRINTF("New timeout at WTX request: %d sec\n", t1->BWT);
}

static void t1_restore_BWT(struct smartcard_m36_private *tp)
{
	if (tp != NULL)
		/*BWT = (2^BWI*960 + 11)etu*/
		tp->T1.BWT= tp->first_cwt;
}

/*construct the block*/
static UINT32 t1_build(t1_state_t *t1, UINT8 *block, UINT8 dad, UINT8 pcb, t1_buf_t *bp, UINT32 *lenp)
{
	UINT32 len;
	INT8 more = FALSE;

	len = bp? t1_buf_avail(bp) : 0;
	if (len > t1->ifsc) 
	{
		pcb |= T1_MORE_BLOCKS;
		len = t1->ifsc;
		more = TRUE;
	}

	/* Add the sequence number */
	switch (t1_block_type(pcb)) 
	{
		case T1_R_BLOCK:
			pcb |= t1->nr << T1_R_SEQ_SHIFT;
			break;
		case T1_I_BLOCK:
			pcb |= t1->ns << T1_I_SEQ_SHIFT;
			t1->more = more;
			T1PRINTF("more bit: %d\n", more);
			break;
	}

	block[0] = dad;
	block[1] = pcb;
	block[2] = len;

	if (len)
		memcpy(block + 3, t1_buf_head(bp), len);
	if (lenp)
		*lenp = len;

	len = t1_compute_checksum(t1, block, len + 3);

	/* memorize the last sent block */
	/* only 4 bytes since we are only interesed in R-blocks */
	memcpy(t1->previous_block, block, 4);

	return len;
}

/*reconstruct the last sent block*/
static UINT32 t1_rebuild(t1_state_t *t1, UINT8 *block)
{
	UINT8 pcb = t1->previous_block[1];
	
	/* copy the last sent block */
	if (T1_R_BLOCK == t1_block_type(pcb))
		memcpy(block, t1->previous_block, 4);
	else
	{
		T1PRINTF("previous block was not R-Block: %02X\n", pcb);
		return 0;
	}

	return 4;
}


/* Send/receive block*/
static INT32 t1_xcv(struct smartcard_m36_private *priv, UINT8*sblock, UINT32 slen, UINT8 *rblock, UINT32 rmax, UINT32 *ractual)
{
	INT32 n;
	UINT32 rmax_int,m;
	UINT32 actual=0;
	UINT8 dad, dad1;
	struct smartcard_m36_private *tp = priv;
    	t1_state_t *t1 = &tp->T1;
	if(tp->T!=1)
		return -1;
	
	
	//T1PRINTF("sending block : %d bytes need to be send\n ", slen);

	if ((sblock == NULL)||(rblock == NULL))
		return -1;
	if (t1->wtx > 1)
	{
		/*set the new temporary timeout at WTX card request */
		t1_update_BWT(t1,t1->wtx);
	}

/************************************************************
  *Note: For some cards, the block head and body should be readed separately.
  *          If that happens, the below should be modified to write/read twice.
  ***********************************************************/
  	dad = *sblock;
#if 1
	//n = smc_uart_T1_write(dev, slen, sblock);
	n = smc_dev_write(priv, sblock, slen, (INT16 *)(&actual));
	t1->wtx = 0;	/* reset to default value ??????????*/
	if (n != RET_SUCCESS)
	{
		T1PRINTF("SMC write error in t1_xcv!\n");	
		return SMART_WRITE_ERROR;//return n;
	}
#else
	n = smc_uart_fifowrite(dev, sblock, slen, &actual);
	if ((n != RET_SUCCESS))//||(actual != slen))
	{
		T1PRINTF("SMC write error in t1_xcv!\n");	
		return SMART_WRITE_ERROR;//return n;
	}
#endif
	/* Get the response en bloc */
	memset(rblock, 0, rmax);
	rmax_int = rmax;
	//n = smc_uart_T1_read(dev, rmax_int, rblock, &actual);
	n = smc_dev_read(priv, rblock, rmax_int, (INT16 *)(&actual));
	rmax = rmax_int;
		
	if ((n == SMART_PARITY_ERROR)|| (n == SMART_NO_ANSWER))  //current not implemented for parity check
	{
		T1PRINTF("SMC read-no answer!\n");
		return n;
	}	
	if ((n == SMART_NOT_INSERTED) ||(n == SMART_NOT_RESETED))
	{
		T1PRINTF("SMC read fetal error in t1_xcv!\n");
		return -1; //fetal error
	}

	//T1PRINTF("t1_xcv read ok, %d bytes got\n ", actual);
	#if 0
	for (i=0;i<actual; i++)
		libc_printf(" 0x%02x ", rblock[i]);
	libc_printf("\n");
	#endif
	dad1 = *rblock;
	invert(&dad1, 1);
	if ((actual > 0)&&(dad1 == dad))
	{
		m = rblock[2] + 3 + t1->rc_bytes;
		if (m < actual)
			actual = m;
	}
	
	if (actual > 0)
	{
//		T1PRINTF("t1_xcv read OK, %d bytes got!\n", actual);
	}
	*ractual = actual;

	/* Restore initial timeout */
	t1_restore_BWT(tp);
	return RET_SUCCESS;
}

static INT32 t1_negociate_ifsd(struct smartcard_m36_private *priv, UINT32 dad, INT32 ifsd)
{
	t1_buf_t	sbuf;
	UINT8 * sdata = NULL;
	UINT32 slen;
	UINT32 retries;
	UINT32 snd_len;
	INT32 n;
	UINT8 snd_buf[1];
	UINT32 actual =  0;
	struct smartcard_m36_private *tp = priv;
	t1_state_t *t1 = &tp->T1;
	sdata = &(t1->sdata[0]);
	retries = t1->retries;

	/* S-block IFSD request */
	snd_buf[0] = ifsd;
	snd_len = 1;

	/* Initialize send/recv buffer */
	t1_buf_set(&sbuf, (void *) snd_buf, snd_len);

	while (TRUE)
	{
		/* Build the block */
		slen = t1_build(t1, sdata, dad, T1_S_BLOCK | T1_S_IFS, &sbuf, NULL);

		/* Send the block */
		n = t1_xcv(priv, sdata, slen, sdata, T1_BUFFER_SIZE, &actual);

		retries--;
		/* ISO 7816-3 Rule 7.4.2 */
		if (retries == 0)
			goto error;

		if (-1 == n)
		{
			T1PRINTF("fatal: transmit/receive failed\n");
			goto error;
		}

		if ((SMART_NO_ANSWER== n)	|| (SMART_WRITE_ERROR == n)							/* Parity error */
			|| (sdata[DATA] != ifsd)				/* Wrong ifsd received */
			|| (sdata[NAD] != swap_nibbles(dad))	/* wrong NAD */
			|| (!t1_verify_checksum(t1, sdata, actual))	/* checksum failed */
			|| (actual != (UINT32)4 + t1->rc_bytes)				/* wrong frame length */
			|| (sdata[LEN] != 1)					/* wrong data length */
			|| (sdata[PCB] != (T1_S_BLOCK | T1_S_RESPONSE | T1_S_IFS))) /* wrong PCB */
			continue;

		/* no more error */
		goto done;
	}

done:
	#if 0
	for (i=0;i<actual;i++)
		T1PRINTF(" 0x%02x ", sdata[i]);
	#endif 
	return RET_SUCCESS;

error:
	t1->state = DEAD;
	return -1;
}

/* Send an APDU through T=1, rcv_len usually the size of rcv_buf, return the actual size readed to the rcv_buf*/
static INT32 t1_transceive(struct smartcard_m36_private *priv, UINT8 dad, const void *snd_buf, UINT32 snd_len, void *rcv_buf, UINT32 rcv_len)
{
	t1_buf_t	sbuf, rbuf, tbuf;
	UINT8	sblk[5];
	UINT32	slen, retries, resyncs, sent_length = 0;
	UINT32		last_send = 0;
	UINT32 	actual = 0,i;
	UINT8 * sdata = NULL;
	UINT8 * rdata = NULL;
	struct smartcard_m36_private *tp = priv;
	t1_state_t *t1 = &tp->T1;
	sdata = &(t1->sdata[0]);
	rdata = &(t1->rdata[0]);
	if (snd_len == 0)
		return -1;

	/* we can't talk to a dead card / reader. Reset it! */
	if (t1->state == DEAD)
	{
		T1PRINTF("T=1 state machine is DEAD. Reset the card first.\n");
		return -1;
	}

	t1->state = SENDING;
	retries = t1->retries+1;
	resyncs = 3;

	/* Initialize send/recv buffer */
	t1_buf_set(&sbuf, (void *) snd_buf, snd_len);
	t1_buf_init(&rbuf, rcv_buf, rcv_len);

	/* Send the first block */
	slen = t1_build(t1, sdata, dad, T1_I_BLOCK, &sbuf, &last_send);

	while (1) {
		UINT8 pcb;
		INT32 n;

		retries--;
		if (retries == 0)
			goto error;

		n = t1_xcv(priv, sdata, slen, rdata,T1_BUFFER_SIZE, &actual);
		#if 0
		if ((SMART_PARITY_ERROR == n) ) //not support parity error now
		{
			T1PRINTF("Parity error\n");
			/* ISO 7816-3 Rule 7.4.2 */
			if (retries == 0)
				goto resync;

			/* ISO 7816-3 Rule 7.2 */
			if (T1_R_BLOCK == t1_block_type(t1->previous_block[PCB]))
			{
				T1PRINTF("Rule 7.2\n");
				slen = t1_rebuild(t1, sdata);
				continue;
			}

			slen = t1_build(t1, sdata,
					dad, T1_R_BLOCK | T1_EDC_ERROR,
					NULL, NULL);
			continue;
		}
		#endif
		if ((SMART_NO_ANSWER == n)||(SMART_WRITE_ERROR==n))
		{
			if (retries == 0)
				goto error;
			continue; //resend the command
		}

		if (n < 0) 
		{
			T1PRINTF("fatal: transmit/receive failed\n");
			t1->state = DEAD;
			goto error;
		}

		if ((rdata[NAD] != swap_nibbles(dad)) /* wrong NAD */
			|| (rdata[LEN] == 0xFF))	/* length == 0xFF (illegal) */
		{
			T1PRINTF("Bad NAD, retry\n");
			for (i=0; i<actual; i++)
				T1PRINTF(" 0x%02x ", rdata[i]);
			/* ISO 7816-3 Rule 7.4.2 */
			if (retries == 0)
				goto resync;

			/* ISO 7816-3 Rule 7.2 */
			if (T1_R_BLOCK == t1_block_type(t1->previous_block[PCB]))
			{
				T1PRINTF("Rule 7.2\n");
				slen = t1_rebuild(t1, sdata);
				continue;
			}
			#if 0
			slen = t1_build(t1, sdata,
				dad, T1_R_BLOCK | T1_OTHER_ERROR,
				NULL, NULL);
			#endif
			continue;
		}

		if (!t1_verify_checksum(t1, rdata, actual)) 
		{
			T1PRINTF("checksum failed\n");
			for (i=0; i<actual; i++)
				T1PRINTF(" 0x%02x ", rdata[i]);
			/* ISO 7816-3 Rule 7.4.2 */
			if (retries == 0)
				goto resync;

			/* ISO 7816-3 Rule 7.2 */
			if (T1_R_BLOCK == t1_block_type(t1->previous_block[PCB]))
			{
				T1PRINTF("Rule 7.2\n");
				slen = t1_rebuild(t1, sdata);
				continue;
			}

			slen = t1_build(t1, sdata,
				dad, T1_R_BLOCK | T1_EDC_ERROR,
				NULL, NULL);
			continue;
		}

		pcb = rdata[PCB];
		switch (t1_block_type(pcb)) 
		{
		case T1_R_BLOCK:
			if ((rdata[LEN] != 0x00)	/* length != 0x00 (illegal) */
				|| (pcb & 0x20)			/* b6 of pcb is set */
			   )
			{
				T1PRINTF("R-Block required\n");
				/* ISO 7816-3 Rule 7.4.2 */
				if (retries == 0)
					goto resync;

				/* ISO 7816-3 Rule 7.2 */
				if (T1_R_BLOCK == t1_block_type(t1->previous_block[1]))
				{
					T1PRINTF("Rule 7.2\n");
					slen = t1_rebuild(t1, sdata);
					continue;
				}

				slen = t1_build(t1, sdata,
						dad, T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
				continue;
			}

			if (((t1_seq(pcb) != t1->ns)	/* wrong sequence number & no bit more */
					&& ! t1->more)
			   )
			{
				T1PRINTF("received: %d, expected: %d, more: %d\n",
					t1_seq(pcb), t1->ns, t1->more);

				/* ISO 7816-3 Rule 7.2 */
				if (T1_R_BLOCK == t1_block_type(t1->previous_block[PCB]))
				{
					T1PRINTF("Rule 7.2\n");
					slen = t1_rebuild(t1, sdata);
					continue;
				}

				T1PRINTF("R-Block required\n");
				/* ISO 7816-3 Rule 7.4.2 */
				if (retries == 0)
					goto resync;
				slen = t1_build(t1, sdata,
						dad, T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
				continue;
			}

			if (t1->state == RECEIVING) {
				/* ISO 7816-3 Rule 7.2 */
				if (T1_R_BLOCK == t1_block_type(t1->previous_block[1]))
				{
					T1PRINTF("Rule 7.2\n");
					slen = t1_rebuild(t1, sdata);
					continue;
				}

				T1PRINTF("");
				slen = t1_build(t1, sdata,
						dad, T1_R_BLOCK,
						NULL, NULL);
				break;
			}

			/* If the card terminal requests the next
			 * sequence number, it received the previous
			 * block successfully */
			if (t1_seq(pcb) != t1->ns) {
				t1_buf_get(&sbuf, NULL, last_send);
				sent_length += last_send;
				last_send = 0;
				t1->ns ^= 1;
			}

			/* If there's no data available, the ICC
			 * shouldn't be asking for more */
			if (t1_buf_avail(&sbuf) == 0)
				goto resync;

			slen = t1_build(t1, sdata, dad, T1_I_BLOCK,
					&sbuf, &last_send);
			break;

		case T1_I_BLOCK:
			/* The first I-block sent by the ICC indicates
			 * the last block we sent was received successfully. */
			if (t1->state == SENDING) {
				T1PRINTF("");
				t1_buf_get(&sbuf, NULL, last_send);
				last_send = 0;
				t1->ns ^= 1;
			}

			t1->state = RECEIVING;

			/* If the block sent by the card doesn't match
			 * what we expected it to send, reply with
			 * an R block */
			if (t1_seq(pcb) != t1->nr) {
				T1PRINTF("wrong nr\n");
				slen = t1_build(t1, sdata, dad,
						T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
				continue;
			}

			t1->nr ^= 1;

			if (t1_buf_put(&rbuf, rdata + 3, rdata[LEN]) < 0)
			{
				T1PRINTF("buffer overrun by %d bytes\n", rdata[LEN] - (rbuf.size - rbuf.tail));
				goto error;
			}

			if ((pcb & T1_MORE_BLOCKS) == 0)
				goto done;

			slen = t1_build(t1, sdata, dad, T1_R_BLOCK, NULL, NULL);
			break;

		case T1_S_BLOCK:
			if (T1_S_IS_RESPONSE(pcb) && t1->state == RESYNCH) {
				/* ISO 7816-3 Rule 6.2 */
				T1PRINTF("S-Block answer received\n");
				/* ISO 7816-3 Rule 6.3 */
				t1->state = SENDING;
				sent_length =0;
				last_send = 0;
				resyncs = 3;
				retries = t1->retries;
				t1_buf_init(&rbuf, rcv_buf, rcv_len);
				slen = t1_build(t1, sdata, dad, T1_I_BLOCK,
						&sbuf, &last_send);
				continue;
			}

			if (T1_S_IS_RESPONSE(pcb))
			{
				/* ISO 7816-3 Rule 7.4.2 */
				if (retries == 0)
					goto resync;

				/* ISO 7816-3 Rule 7.2 */
				if (T1_R_BLOCK == t1_block_type(t1->previous_block[PCB]))
				{
					T1PRINTF("Rule 7.2\n");
					slen = t1_rebuild(t1, sdata);
					continue;
				}

				T1PRINTF("wrong response S-BLOCK received\n");
				slen = t1_build(t1, sdata,
						dad, T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
				continue;
			}

			t1_buf_init(&tbuf, sblk, sizeof(sblk));

			T1PRINTF("S-Block request received\n");
			switch (T1_S_TYPE(pcb)) {
			case T1_S_RESYNC:
				if (rdata[LEN] != 0)
				{
					T1PRINTF("Wrong length: %d\n", rdata[LEN]);
					slen = t1_build(t1, sdata, dad,
						T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
					continue;
				}

				T1PRINTF("Resync requested\n");
				/* the card is not allowed to send a resync. */
				goto resync;

			case T1_S_ABORT:
				if (rdata[LEN] != 0)
				{
					T1PRINTF("Wrong length: %d\n", rdata[LEN]);
					slen = t1_build(t1, sdata, dad,
						T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
					continue;
				}

				/* ISO 7816-3 Rule 9 */
				T1PRINTF("abort requested\n");
				goto resync;

			case T1_S_IFS:
				if (rdata[LEN] != 1)
				{
					T1PRINTF("Wrong length: %d\n", rdata[LEN]);
					slen = t1_build(t1, sdata, dad,
						T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
					continue;
				}

				T1PRINTF("sent S-block with ifs=%u\n", rdata[DATA]);
				if (rdata[DATA] == 0)
					goto resync;
				t1->ifsc = rdata[DATA];
				t1_buf_putc(&tbuf, rdata[DATA]);
				break;

			case T1_S_WTX:
				if (rdata[LEN] != 1)
				{
					T1PRINTF("Wrong length: %d\n", rdata[LEN]);
					slen = t1_build(t1, sdata, dad,
						T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
					continue;
				}

				T1PRINTF("sent S-block with wtx=%u\n", rdata[DATA]);
				t1->wtx = rdata[DATA];
				t1_buf_putc(&tbuf, rdata[DATA]);
				break;

			default:
				T1PRINTF("T=1: Unknown S block type 0x%02x\n", T1_S_TYPE(pcb));
				goto resync;
			}

			slen = t1_build(t1, sdata, dad,
				T1_S_BLOCK | T1_S_RESPONSE | T1_S_TYPE(pcb),
				&tbuf, NULL);
		}

		/* Everything went just splendid */
		retries = t1->retries;
		continue;

resync:
		/* the number or resyncs is limited, too */
		/* ISO 7816-3 Rule 6.4 */
		if (resyncs == 0)
			goto error;

		/* ISO 7816-3 Rule 6 */
		resyncs--;
		t1->ns = 0;
		t1->nr = 0;
		slen = t1_build(t1, sdata, dad, T1_S_BLOCK|T1_S_RESYNC, NULL,
				NULL);
		t1->state = RESYNCH;
		t1->more = FALSE;
		retries = 1;
		continue;
	}

done:
	return t1_buf_avail(&rbuf);

error:
	t1->state = DEAD;
	return -1;
}
#endif

static void smc_dev_enable(void)
{
}

static void smc_dev_disable(void)
{
}
static void smc_write_tx(UINT32 io_base, UINT16 val)
{

	if(smc_rx_fifo_size>64)
		OUTPUT_UINT16(io_base + REG_TX_CNT, val);
	else
		OUTPUT_UINT8(io_base + REG_TX_CNT, ((UINT8)val));
}

static UINT16 smc_read_tx(UINT32 io_base)
{
	UINT16 val; 
	if(smc_rx_fifo_size>64)
		val = INPUT_UINT16(io_base+ REG_TX_CNT);
	else
		val = INPUT_UINT8(io_base + REG_TX_CNT);
	return val;
}

static void smc_write_rx(UINT32 io_base, UINT16 val)
{

	if(smc_rx_fifo_size>64)
		WRITE_RX_CNT(io_base, val);
	else
		OUTPUT_UINT8(io_base + REG_RFIFO_CNT, ((UINT8)val));
}

static UINT16 smc_read_rx(UINT32 io_base)
{
	UINT16 val; 
	if(smc_rx_fifo_size>64)
		val = READ_RX_CNT(io_base);
	else
		val = INPUT_UINT8(io_base + REG_RFIFO_CNT);
	return val;
}
#if 0
static void smc_dev_hsr_0(UINT32 param)
{
	struct smc_device *dev = smc_dev_set[0].dev;
	if(!dev)
		return;
	struct smartcard_m36_private *tp = (struct smartcard_m36_private *)(dev->priv);
	if(param&SMC_INSERTED)
		osal_flag_clear(tp->smc_flag_id, SMC_REMOVED);
	if(param&SMC_REMOVED)
		osal_flag_clear(tp->smc_flag_id, SMC_INSERTED);
	osal_flag_set(tp->smc_flag_id, param);
}

static void smc_dev_hsr_1(UINT32 param)
{
	struct smc_device *dev = smc_dev_set[1].dev;
	if(!dev)
		return;
	struct smartcard_m36_private *tp = (struct smartcard_m36_private *)(dev->priv);
	if(param&SMC_INSERTED)
		osal_flag_clear(tp->smc_flag_id, SMC_REMOVED);
	if(param&SMC_REMOVED)
		osal_flag_clear(tp->smc_flag_id, SMC_INSERTED);
	osal_flag_set(tp->smc_flag_id, param);
}

void smc_gpio_detect_lsr(UINT32 dev)
{
	struct smartcard_m36_private *tp = (struct smartcard_m36_private *)((struct smc_device *)dev)->priv;
	UINT32 base_addr = ((struct smc_device *)dev)->base_addr;
	
	if(HAL_GPIO_INT_STA_GET(tp->gpio_cd_pos)==0)
		return;	
	HAL_GPIO_INT_CLEAR(tp->gpio_cd_pos);
	if((HAL_GPIO_BIT_GET(tp->gpio_cd_pos)) == tp->gpio_cd_pol)
	{
		if(tp->inserted)
		{
			smc_dev_deactive((struct smc_device *)dev);
		}
	}
}

/******************************************************************************************************
 * 	Name		:	smc_dev_attach()
 *	Description	:	Smart card reader init funciton.
 *	Parameter	:	int dev_id		: Index of smart card slot.
 *	Return		:	INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
INT32 smc_dev_attach(int dev_id, struct smc_dev_config * config_param)
{
	struct smc_device *dev;
	struct smartcard_m36_private *tp;
	void *priv_mem;
	UINT8 dev_num;
	if(0x80000000!=(((UINT32)config_param)&0xf0000000)&&
		0xa0000000!=(((UINT32)config_param)&0xf0000000)&&
		0xb0000000!=(((UINT32)config_param)&0xf0000000))
		return !SUCCESS;
	smc_chip_id = sys_ic_get_chip_id();
	smc_chip_version = sys_ic_get_rev_id();

	if(sys_ic_is_M3101())
	{
		smc_chip_id = ALI_M3101;
	}
    if(smc_chip_id == ALI_S3602 && smc_chip_version >= IC_REV_6)
    {
        smc_chip_id = ALI_S3602F;
        //smc_chip_version = IC_REV_0;
    }

	if(dev_id == 0)
		smc_dev_name[STRLEN(smc_dev_name) - 1] = '0';
	else if(dev_id == 1)
	{
		if(ALI_M3329E==smc_chip_id)
		{
			if(smc_chip_version>=IC_REV_5)
				return ERR_FAILUE;
		}
		smc_dev_name[STRLEN(smc_dev_name) - 1] = '1';	
	}
	else
		return ERR_FAILUE;
		
	SMC_PRINTK("%s\n",smc_dev_name);
	

	if(ALI_S3602==smc_chip_id)
	{
		smc_tx_fifo_size = 8;
		smc_rx_fifo_size = 32;
	}
	if((ALI_M3101==smc_chip_id)||(ALI_M3329E==smc_chip_id && smc_chip_version>=IC_REV_5) || (ALI_S3602F==smc_chip_id))
	{
        smc_tx_fifo_size = smc_rx_fifo_size = 256;
	}
	dev = dev_alloc(smc_dev_name, HLD_DEV_TYPE_SMC,sizeof(struct smc_device));
	
	if (dev == NULL)
	{
		PRINTF("Error: Alloc smart card reader error!\n");
		return ERR_NO_MEM;
	}
	smc_dev_set[dev_id].dev = dev;
	/* Alloc structure space of private */
	priv_mem = (void *)MALLOC(sizeof(struct smartcard_m36_private));

	ASSERT(priv_mem != NULL);
	memset(priv_mem, 0, sizeof(struct smartcard_m36_private));
	
	dev->priv = priv_mem;

	tp = (struct smartcard_m36_private *)dev->priv;
	tp->atr_info = (atr_t *)MALLOC(sizeof(atr_t));
	ASSERT(tp->atr_info!=NULL);
	memset(tp->atr_info, 0, sizeof(atr_t));
	tp->inserted = 0;
	tp->reseted = 0;
	tp->inverse_convention = 0;
	tp->the_last_send = 0;
	if(0==dev_id)
		tp->hsr = smc_dev_hsr_0;
	else
		tp->hsr = smc_dev_hsr_1;
	
	if(config_param->init_clk_trigger&&
		config_param->init_clk_number&&
		config_param->init_clk_array)
	{
		tp->init_clk_number = config_param->init_clk_number;
		tp->init_clk_array = (UINT32 *)MALLOC(tp->init_clk_number*sizeof(UINT32));
		ASSERT(NULL!=tp->init_clk_array);
		memcpy(tp->init_clk_array, config_param->init_clk_array, (tp->init_clk_number*sizeof(UINT32)));
	}
	else
	{
		tp->init_clk_number = 1;
		tp->init_clk_array = &(tp->smc_clock);
		tp->smc_clock = DFT_WORK_CLK;
	}
	if(config_param->gpio_cd_trigger)
	{
		tp->use_gpio_cd = 1;
		tp->gpio_cd_io = config_param->gpio_cd_io;
		tp->gpio_cd_pol = config_param->gpio_cd_pol;
		tp->gpio_cd_pos = config_param->gpio_cd_pos;
	}
	if(config_param->gpio_vpp_trigger)
	{
		tp->use_gpio_vpp = 1;
		if((ALI_M3101==smc_chip_id)||(ALI_M3329E==smc_chip_id && smc_chip_version>=IC_REV_3) || (ALI_S3602F==smc_chip_id))
		{
			tp->internal_ctrl_vpp = 1;
		}
		tp->gpio_vpp_io = config_param->gpio_vpp_io;
		tp->gpio_vpp_pol = config_param->gpio_vpp_pol;
		tp->gpio_vpp_pos = config_param->gpio_vpp_pos;
	}
	if(config_param->def_etu_trigger)
	{
		tp->use_default_etu = 1;
		tp->default_etu = config_param->default_etu;
	}
	
	//check HW auto TX/RX
	if(ALI_M3329E==smc_chip_id)
	{
		if(smc_chip_version>=IC_REV_2)
			tp->auto_tx_rx_triger = 1;
		else
			tp->force_tx_rx_triger = 1;
		
		if(smc_chip_version>=IC_REV_3)
			tp->ts_auto_detect = 1;
		
		if(smc_chip_version>=IC_REV_5)
		{
			tp->invert_power = config_param->invert_power;
			tp->invert_detect = config_param->invert_detect;
		}
	}
    else if((ALI_M3101==smc_chip_id)||(ALI_S3602F==smc_chip_id))
    {
        tp->invert_power = config_param->invert_power;
		tp->invert_detect = config_param->invert_detect;
		 tp->auto_tx_rx_triger = 1;
		 tp->ts_auto_detect = 1;
    }
    
	if(sys_ic_is_M3202())
	{
		if(smc_chip_version>=IC_REV_2)
		{
			tp->auto_tx_rx_triger = 1;
			tp->invert_power = config_param->invert_power;
			tp->invert_detect = config_param->invert_detect;
		}
		else
			tp->force_tx_rx_triger = 1;
	}
	if(ALI_S3602==smc_chip_id)
	{
		if(smc_chip_version>=IC_REV_2)
		{
			tp->auto_tx_rx_triger = 1;
		}
	}
	
	tp->parity_disable = config_param->parity_disable_trigger;
	tp->parity_odd = config_param->parity_odd_trigger;
	tp->apd_disable = config_param->apd_disable_trigger;
	tp->warm_reset_enable = config_param->warm_reset_trigger;
	tp->disable_pps = config_param->disable_pps;
	if(ALI_M3329E==smc_chip_id)
	{
		if(smc_chip_version>=IC_REV_5)
		{
			smc_dev_set[0].io_base = 0xb8001800;
			smc_dev_set[0].irq = 20;
			smc_dev_set[1].io_base = 0xb8001800;
			smc_dev_set[1].irq = 20;
			scr_sys_clk = pwm_sys_clk = 108000000;
		}
		else
		{
			smc_dev_set[0].io_base = 0xb8001800;
			smc_dev_set[0].irq = 14;
			smc_dev_set[0].pwm_addr = 0xb8001430;
			smc_dev_set[0].pwm_sel_ofst = 0x0;
			smc_dev_set[0].pwm_seh_ofst = 0x2;
			smc_dev_set[0].pwm_gpio_ofst = 0x4;
			smc_dev_set[0].pwm_cfg_ofst = 0x5;
			smc_dev_set[0].pwm_frac_ofst = 0x6;
			
			smc_dev_set[1].io_base = 0xb8001900;
			smc_dev_set[1].irq = 15;
			smc_dev_set[1].pwm_addr = 0xb8001460;
			smc_dev_set[1].pwm_sel_ofst = 0x0;
			smc_dev_set[1].pwm_seh_ofst = 0x2;
			smc_dev_set[1].pwm_gpio_ofst = 0x4;
			smc_dev_set[1].pwm_cfg_ofst = 0x5;
			smc_dev_set[1].pwm_frac_ofst = 0x6;
			if(!scr_sys_clk)
			{
				UINT32 tmp;
				tmp = ((*((volatile UINT32 *)0xb8000070))>>2)&0x3;
				if(tmp==0x0)
		 			pwm_sys_clk = 135000000;
		 		else if(tmp==0x1)
		 			pwm_sys_clk = 120000000;
		 		else if(tmp==0x2)
		 			pwm_sys_clk = 166000000;
		 		else
		 			pwm_sys_clk = 154000000;		
	    			scr_sys_clk = 108000000;
			}
		}
	}
	if(sys_ic_is_M3202()==1)
	{
		smc_dev_set[0].io_base = 0xb8001800;
		smc_dev_set[0].irq = 20;
		smc_dev_set[0].pwm_addr = 0xb8001a00;
		smc_dev_set[0].pwm_sel_ofst = 0x0;
		smc_dev_set[0].pwm_seh_ofst = 0x2;
		smc_dev_set[0].pwm_gpio_ofst = 0x6;
		smc_dev_set[0].pwm_cfg_ofst = 0x4;
		smc_dev_set[0].pwm_frac_ofst = 0x5;
		
		smc_dev_set[1].io_base = 0xb8001900;
		smc_dev_set[1].irq = 21;
		smc_dev_set[1].pwm_addr = 0xb8001b00;
		smc_dev_set[1].pwm_sel_ofst = 0x0;
		smc_dev_set[1].pwm_seh_ofst = 0x2;
		smc_dev_set[1].pwm_gpio_ofst = 0x6;
		smc_dev_set[1].pwm_cfg_ofst = 0x4;
		smc_dev_set[1].pwm_frac_ofst = 0x5;
		if(!scr_sys_clk)
		{
			UINT32 tmp;
			if(config_param->sys_clk_trigger)
			{
				scr_sys_clk = config_param->smc_sys_clk;
				if(scr_sys_clk == 166000000)
	 				tmp = 0x3;
	 			else if(scr_sys_clk == 135000000)
	 				tmp = 0x1;
	 			else if(scr_sys_clk == 154000000)
	 				tmp = 0x2;
	 			else
	 			{
	 				tmp = 0x0;	
	 				scr_sys_clk = 108000000;
	 			}
				*((volatile UINT8 *)0xb800007a) &= ~(0x3<<1);
				*((volatile UINT8 *)0xb800007a) |= tmp<<1;
			}
			else
			{
				tmp = ((*((volatile UINT32 *)0xb8000078))>>17)&0x3;
	 			if(tmp==0x0)
	 				scr_sys_clk = 108000000;
	 			else if(tmp==0x1)
	 				scr_sys_clk = 135000000;
	 			else if(tmp==0x2)
	 				scr_sys_clk = 154000000;
	 			else
	 				scr_sys_clk = 166000000;
			}
			pwm_sys_clk = scr_sys_clk;
		}
	}
	if((ALI_S3602==smc_chip_id)\
		||(ALI_S3602F==smc_chip_id))
	{
		if(!scr_sys_clk)
		{
			scr_sys_clk = 108000000;
			pwm_sys_clk = scr_sys_clk;
		}
	}
	if(ALI_M3101==smc_chip_id)
	{
		smc_dev_set[0].io_base = 0xb8001800;
		smc_dev_set[0].irq = 20;
		smc_dev_set[1].io_base = 0xb8001800;
		smc_dev_set[1].irq = 20;
		scr_sys_clk = pwm_sys_clk = 108000000;
	}
	dev->base_addr = smc_dev_set[dev_id].io_base;
	dev->irq = smc_dev_set[dev_id].irq;
	
	/* Function point init */
	dev->open = smc_dev_open;
	dev->close = smc_dev_close;
	dev->card_exist = smc_dev_card_exist;
	dev->reset = smc_dev_reset;
	dev->deactive = smc_dev_deactive;
	dev->raw_read = smc_dev_read;
	dev->raw_write = smc_dev_write;
	dev->raw_fifo_write = smc_dev_write;
	//dev->transmit = smc_dev_transfer_data;
	dev->iso_transfer = smc_dev_iso_transfer;
	dev->iso_transfer_t1 = smc_dev_iso_transfer_t1;
	dev->do_ioctl = smc_dev_ioctl;
	dev->t1_transfer = t1_transceive;
	dev->t1_xcv = t1_xcv;
	dev->t1_negociate_ifsd = t1_negociate_ifsd;

	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS)
	{
		SMC_PRINTK("Error: Register smart card reader device error!\n");
		FREE(priv_mem);
		dev_free(dev);
	}
	SMC_PRINTK("attach ok\n");
	return SUCCESS;
}
#endif
/******************************************************************************************************
 * 	Name		:	smc_dev_interrupt()
 *	Description	:	smart card reader controler interrupt handle.
 *	Parameter	:	struct smartcard_m36_private *priv	: Devcie handle.
 *	Return		:	none
 *
 ******************************************************************************************************/
static irqreturn_t smc_dev_interrupt(int irq, UINT32 param)
{		
	UINT8 isr0_status,isr1_status;
	UINT32 i;
	struct smartcard_m36_private *tp =((struct ali_smartcard_device *)param)->priv;
	UINT32 ioaddr = tp->base_addr;	
	

	isr0_status = INPUT_UINT8(ioaddr + REG_ISR0);
	isr1_status = INPUT_UINT8(ioaddr + REG_ISR1);
	
	tp->isr0_interrupt_status |= isr0_status;
	tp->isr1_interrupt_status |= isr1_status;

	OUTPUT_UINT8(ioaddr + REG_ISR0, isr0_status);
	OUTPUT_UINT8(ioaddr + REG_ISR1, isr1_status);
    	
	if(isr0_status&SMC_ISR0_BYTE_RECV)
	{
//		SMC_PRINTK("byte : %d\n", smc_read_rx(ioaddr));
		
		if((0!=tp->smc_rx_tail)&&(0==tp->got_first_byte))
		{
			tp->isr0_interrupt_status &= (~SMC_ISR0_BYTE_RECV);
			tp->got_first_byte = 1; 
			//OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)&(~SMC_IER0_BYTE_RECV_TRIG));
//			osal_interrupt_register_hsr(tp->hsr, SMC_RX_BYTE_RCV);
			tp->smc_flag_id |= SMC_RX_BYTE_RCV;
		}
		if(tp->smc_rx_buf==tp->smc_rx_tmp_buf)
		{
			UINT16 c;
			UINT32 rem_space = tp->smc_rx_tail - tp->smc_rx_head;
		
			c =smc_read_rx(ioaddr);
//			SMC_PRINTK("rcv %d bytes\n", c);
			c = (c<=rem_space?c:rem_space);			
			
			for(i=0;i<c;i++)
				tp->smc_rx_buf[tp->smc_rx_head+i] = INPUT_UINT8(ioaddr + REG_RBR);
				
			tp->smc_rx_head += c; 
			tp->isr0_interrupt_status &= (~SMC_ISR0_BYTE_RECV);
//			SMC_PRINTK("rcv %d bytes\n", tp->smc_rx_head);
		}
	}
	if((isr0_status&SMC_ISR0_FIFO_TRANS)&&(tp->force_tx_rx_triger||tp->auto_tx_rx_triger))
	{
        	tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_TRANS);
        	if(tp->smc_tx_wr)
        	{
			if(tp->smc_tx_rd==tp->smc_tx_wr)
			{
//				SMC_PRINTK("tx finish:\n");
				/*Once TX finished, set interface device to RX mode immediately*/
				if(tp->force_tx_rx_triger&&1==tp->force_tx_rx_state)
				{
					smc_write_tx(ioaddr, tp->force_tx_rx_thld);
					tp->force_tx_rx_state = 2;
					if(0==smc_read_tx(ioaddr))
					{	
						OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
						SMC_PRINTK("tmo2: too late!\n");
					}
					else
					{
						UINT32 force_loop_tmo;
						UINT8 force_tx_rx;
	
						force_tx_rx = 1;
					
						force_loop_tmo = (smc_read_tx(ioaddr)+1)*tp->char_frm_dura;
					
//						UINT32 tmo = read_tsc();
						//if(0==smc_read_tx(ioaddr))libc_libc_printf("tmo2: too late!\n");
						
						while(0!=smc_read_tx(ioaddr))
						{
							udelay(1);
							force_loop_tmo--;
							if(!force_loop_tmo)
							{
								force_tx_rx = 0;
								SMC_PRINTK("tmo2: tmo %d\n", smc_read_tx(ioaddr));
//								SMC_PRINTK("tmo2: tmo %d\n", smc_read_tx(ioaddr));
								break;
							}
							
							if((INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80)== 0)
							{
								SMC_PRINTK("[ %s %d ], smart card not inserted!\n",
									__FUNCTION__, __LINE__);
								return IRQ_HANDLED;
							}
							
						}
						if(force_tx_rx)
						{
							OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
						}
						//SMC_PRINTK("tmo2: %d, %d\n", force_loop_tmo, (read_tsc()-tmo)/(SYS_CPU_CLOCK / 2000000));
						//SMC_PRINTK("tmo2: %d, %d\n", force_loop_tmo, (read_tsc()-tmo)/(SYS_CPU_CLOCK / 2000000));
					}
				}
				else
				{
					if(!(tp->auto_tx_rx_triger))
            				{	
            					OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);		
					}
//					osal_interrupt_register_hsr(tp->hsr, SMC_TX_FINISHED);
					tp->smc_flag_id |= SMC_TX_FINISHED;
					/*
					if(ALI_S3602==smc_chip_id)
					{
						tp->the_last_send = 0;  
						tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_EMPTY);						
					}
					*/
				}
				
			}
			else
			{
				UINT32 size = tp->smc_tx_wr - tp->smc_tx_rd;
				//OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS));
				if(size>(smc_tx_fifo_size>>1))
				{	
					size = smc_tx_fifo_size>>1;
					if(0) //ALI_S3602==smc_chip_id)
						OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,(INPUT_UINT8(ioaddr + REG_FIFO_CTRL)&0xf0)|(size));
					else
						smc_write_tx(ioaddr,size);
				}
				else
				{
					if(tp->force_tx_rx_triger)
					{
						if(0==smc_read_tx(ioaddr))
						{
							if(size>tp->force_tx_rx_thld)
							{	
								smc_write_tx(ioaddr, size - tp->force_tx_rx_thld);
								tp->force_tx_rx_state = 1;
							}
							else
							{	
								smc_write_tx(ioaddr, size);
								tp->force_tx_rx_state = 2;
							}
						}
						else
						{	
							smc_write_tx(ioaddr, (size+(smc_tx_fifo_size>>1)) - tp->force_tx_rx_thld);
							tp->force_tx_rx_state = 1;
						}
					}
					else
					{	
						if(0) //ALI_S3602==smc_chip_id)
						{
							//OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,INPUT_UINT8(ioaddr + REG_FIFO_CTRL)|(size + (smc_tx_fifo_size>>1)));
		 		 			OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,(INPUT_UINT8(ioaddr + REG_FIFO_CTRL)&0xf0)|(size + (smc_tx_fifo_size>>1)));
							tp->the_last_send = 1;
							tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_EMPTY);
						}
						else
						{
							if(0==smc_read_tx(ioaddr))
								smc_write_tx(ioaddr,size);
							else	
								smc_write_tx(ioaddr,size+(smc_tx_fifo_size>>1));
						}
					}
				}
					
				for(i=0; i<size; i++)
				{
					if((tp->smc_tx_rd+i+1)==tp->smc_tx_wr)
					{
						if(tp->auto_tx_rx_triger)
						{
							OUTPUT_UINT8(ioaddr+REG_ICCSR, 1<<5); //tx->rx auto switch
//							SMC_PRINTK("2: tx->rx auto: rd %d, cnt %d, wr %d\n", tp->smc_tx_rd, i, tp->smc_tx_wr);
						}
					}
					OUTPUT_UINT8(ioaddr + REG_THR, tp->smc_tx_buf[tp->smc_tx_rd+i]);
				}
				//OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,INPUT_UINT8(ioaddr + REG_SCR_CTRL)|SMC_SCR_CTRL_TRANS);
				tp->smc_tx_rd += size;
//				SMC_PRINTK("continue feed data %d \n", size);
				if(tp->smc_tx_rd==tp->smc_tx_wr&&
					1==tp->force_tx_rx_triger&&
					tp->force_tx_rx_state ==2)
				{
					UINT32 force_loop_tmo;
					UINT8 force_tx_rx;
	
					force_tx_rx = 1;
					
					force_loop_tmo = (smc_read_tx(ioaddr)+1)*tp->char_frm_dura;
					
//					UINT32 tmo = read_tsc();
						//if(0==smc_read_tx(ioaddr ))libc_libc_printf("tmo2: too late!\n");
						
					while(0!=smc_read_tx(ioaddr))
					{
						udelay(1);
						force_loop_tmo--;
						if(!force_loop_tmo)
						{
							force_tx_rx = 0;
							SMC_PRINTK("tmo3: tmo %d\n", smc_read_tx(ioaddr));
							break;
						}
						if((INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80)== 0)
						{
							SMC_PRINTK("[ %s %d ], smart card not inserted!\n",
									__FUNCTION__, __LINE__);
							return IRQ_HANDLED;
						}
					}
					
					if(force_tx_rx)
					{
						OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
					}
					//SMC_PRINTK("tmo2: %d, %d\n", force_loop_tmo, (read_tsc()-tmo)/(SYS_CPU_CLOCK / 2000000));
				}
			}
        	}
	}
	if(isr0_status& SMC_ISR0_FIFO_RECV)
	{
		UINT16 c;
		UINT32 rem_space = tp->smc_rx_tail - tp->smc_rx_head;
		tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_RECV);
		if(tp->smc_rx_tail&&tp->smc_rx_buf!=tp->smc_rx_tmp_buf)
		{
			c =smc_read_rx(ioaddr);
			c = (c<=rem_space?c:rem_space);			
		
			for(i=0;i<c;i++)
				tp->smc_rx_buf[tp->smc_rx_head+i] = INPUT_UINT8(ioaddr + REG_RBR);
			tp->smc_rx_head += c; 
	
			if(tp->smc_rx_head == tp->smc_rx_tail)
			{
//				SMC_PRINTK("Notify rx over: %d\n", tp->smc_rx_tail);
				//tp->smc_flag_ptn = SMC_RX_FINISHED;
//				osal_interrupt_register_hsr(tp->hsr, SMC_RX_FINISHED);
				tp->smc_flag_id |= SMC_RX_FINISHED;
			}
			else
			{
				rem_space = tp->smc_rx_tail - tp->smc_rx_head;    
				if(rem_space/smc_rx_fifo_size)
				{
					smc_write_rx(ioaddr,32);
				}
				else
				{
					smc_write_rx(ioaddr,rem_space);	
				}
//				SMC_PRINTK("continue rx %d data\n", rem_space);		
			}
		}
	}
	if(0) //(ALI_S3602==smc_chip_id)&&(isr0_status& SMC_ISR0_FIFO_EMPTY)&&(1==tp->the_last_send))
	{
		if(!(tp->auto_tx_rx_triger))
            	{	
            		OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);		
		}
//		osal_interrupt_register_hsr(tp->hsr, SMC_TX_FINISHED);
		tp->smc_flag_id |= SMC_TX_FINISHED;
		tp->the_last_send = 0;
		tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_EMPTY);						
	}
	 
	if((isr1_status & SMC_ISR1_CARD_INSERT) != 0)
	{
		tp->inserted = 1;
		tp->atr_rlt = SMC_ATR_NONE;
		tp->reseted = 0;
//		osal_interrupt_register_hsr(tp->hsr, SMC_INSERTED);
		tp->smc_flag_id &= (~SMC_REMOVED);
		tp->smc_flag_id |= SMC_INSERTED;

		SMC_PRINTK("[ %s %d ], smart card inserted!\n", __FUNCTION__, __LINE__);	    
	}
	else if((isr1_status & SMC_ISR1_CARD_REMOVE) != 0)
	{
		tp->inserted = 0;
		tp->reseted = 0;
		tp->atr_rlt = SMC_ATR_NONE;
		//smc_dev_deactive((struct smc_device *)param);
		smc_dev_deactive(tp);
		//smc_init_hw((struct smc_device *)param);
		smc_init_hw(tp);
//		osal_interrupt_register_hsr(tp->hsr, SMC_REMOVED);
		tp->smc_flag_id &= (~SMC_INSERTED);
		tp->smc_flag_id |= SMC_REMOVED;
		SMC_PRINTK("[ %s %d ], smart card removed!\n", __FUNCTION__, __LINE__);	       
	}
	else
	{		
		return IRQ_HANDLED;		
	}
			
	schedule_work(&g_smc_wq);	


	return IRQ_HANDLED;
}


static void smc_set_wclk(UINT32 ioaddr, UINT32 clk)
{
	UINT32 scr_div_inte, pwm_div_inte;
	UINT32 scr_div_fract, pwm_div_fract;
	UINT32 dev_id;
	//double db_temp;
	if(ioaddr==smc_dev_set[0].io_base)
		dev_id = 0;
	else
		dev_id = 1;
	#if 0
	db_temp = ((double)scr_sys_clk)/((double)clk);
	scr_div_inte = (UINT32)(db_temp*100);	
	scr_div_fract = scr_div_inte%100;
	scr_div_inte = scr_div_inte/100;
	#else
	scr_div_fract = ((scr_sys_clk%clk)*100)/clk;
	scr_div_inte = scr_sys_clk/clk;
	#endif
	if(scr_div_fract)
	{
		UINT8 fract;
		//db_temp = ((double)pwm_sys_clk)/((double)clk);
		//pwm_div_inte = (UINT32)(db_temp*100);
		pwm_div_inte = (pwm_sys_clk*100)/(clk);
		
		pwm_div_fract = pwm_div_inte%100;
		pwm_div_inte = pwm_div_inte/100;
		fract = 100/pwm_div_fract;
		
		
		OUTPUT_UINT8(ioaddr + REG_CLKH_SEH, scr_div_inte>>1); 	
		OUTPUT_UINT8(ioaddr + REG_CLKL_SEL, (scr_div_inte>>1)+(scr_div_inte&0x1));	
		OUTPUT_UINT16(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_seh_ofst, pwm_div_inte>>1);
		OUTPUT_UINT16(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_sel_ofst, (pwm_div_inte>>1)+(pwm_div_inte&0x1));
		OUTPUT_UINT8(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_frac_ofst, fract);	
		OUTPUT_UINT8(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_cfg_ofst, 0x81);	

		OUTPUT_UINT8(ioaddr + REG_CLK_FRAC, fract); 	
	}
	else
	{
		OUTPUT_UINT8(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_cfg_ofst, 0);	
		OUTPUT_UINT8(ioaddr + REG_CLKH_SEH, scr_div_inte>>1); 	
		OUTPUT_UINT8(ioaddr + REG_CLKL_SEL, (scr_div_inte>>1)+(scr_div_inte&0x1));		
	}
	
}

static void smc_init_hw(struct smartcard_m36_private *priv)
{
	UINT32 rst_msk = 0;
	UINT32 sys_rst_addr = 0x18000000;
	UINT32 ioaddr = priv->base_addr;
	UINT32 i=0;
	struct smartcard_m36_private *tp = priv;	

    if(1) //(ALI_S3602==smc_chip_id) || (ALI_S3602F==smc_chip_id))
	{
		if(priv->base_addr==smc_dev_set[0].io_base)
		{
			rst_msk = 1<<20;	
			/* enable ca1_1 for M3921 */
		    /*
		    		第一组CA：
			    	ca1_1:reg 0xb8000088[0]
				ca1_2:reg 0xb800008c[23]

				第二组CA：
				ca2_1:reg 0xb800008c[4]
				ca2_2:reg 0xb800008c[1]
			*/
			#if defined(CONFIG_ALI_CHIP_M3921)
			__REG32ALI(0x18000088) |= __REG32ALI(0x18000088) | 0x1;			
			#endif
		}
		else
		{
			rst_msk = 1<<21;
		}
		sys_rst_addr = 0x18000080;
	}

	__REG32ALI(sys_rst_addr) |= rst_msk;
	udelay(3);
	__REG32ALI(sys_rst_addr) &= ~rst_msk;
	tp->smc_clock = DFT_WORK_CLK;     //Set default clk, is necessary.
	tp->smc_clock = tp->init_clk_array[0];
	if(tp->use_default_etu)
		tp->smc_etu = tp->default_etu;
	else
		tp->smc_etu = DFT_WORK_ETU;
	tp->inverse_convention = 0;
	//SMC_PRINTK("init smc regiseter\n");
	if(tp->ts_auto_detect)
	{
		OUTPUT_UINT8(ioaddr + REG_DEV_CTRL, 0x10|(tp->invert_detect<<5)|(tp->invert_power<<6));//enable TS auto detecting.
	}
	OUTPUT_UINT8(ioaddr + REG_SCR_CTRL, 0x00);
	//udelay(2000);
	OUTPUT_UINT8(ioaddr + REG_SCR_CTRL, 0x80|(tp->parity_odd<<4)); //enable SCR interface
	if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
	{
		OUTPUT_UINT8(ioaddr + REG_ICCR, 0x1|(tp->parity_disable<<5)|(tp->apd_disable<<4));	
	}
	else	
	{
		OUTPUT_UINT8(ioaddr + REG_ICCR, 0x41|(tp->parity_disable<<5)|(tp->apd_disable<<4));	 // power off
	}
	
	if(tp->open_drain_supported)
	{		
		UINT8 temp_val =  INPUT_UINT8(ioaddr + REG_CLK_VPP);
		temp_val &= 0x9f;			
		temp_val |= (tp->en_power_open_drain<<6);				
		temp_val |= ((tp->en_clk_open_drain|tp->en_data_open_drain|tp->en_rst_open_drain)<<5);
		OUTPUT_UINT8(ioaddr + REG_CLK_VPP, temp_val);			
	}

 	if(tp->use_gpio_vpp&&tp->internal_ctrl_vpp)
 	{
 		UINT8 temp_val =  INPUT_UINT8(ioaddr + REG_CLK_VPP);
 		temp_val &= 0xf3;
 		temp_val |= (tp->gpio_vpp_pol<<3);
 		if(1 ) //(ALI_S3602F==smc_chip_id)	||(ALI_M3101==smc_chip_id))
		{
		/*
		//s3602f SmartCard interface auto disable clock function has problem 
		while meet the smart card without parity bit such as Irdeto.
		need disable this function for s3602f
		*/
			temp_val |= 0x10;
		}		
 		OUTPUT_UINT8(ioaddr + REG_CLK_VPP, temp_val);
 		OUTPUT_UINT8(ioaddr + REG_VPP_GPIO, (tp->gpio_vpp_pos&0x3f)|0x80);
 	}	
	
	//enable interrupt
	OUTPUT_UINT8(ioaddr + REG_IER0, 0x7f);		//enable receive interrupt, enable wait timeout interrupt
	OUTPUT_UINT8(ioaddr + REG_IER1, 0xff);		//detect Card inserting or removal interrupt enable
	i =0;
	while(tp->invert_detect)
	{
		if(INPUT_UINT8(ioaddr + REG_ISR1) & (SMC_ISR1_CARD_REMOVE|SMC_ISR1_CARD_INSERT))
		{	
			SMC_PRINTK("i %d: %02x, isr0: %02x, isr1: %02x\n", i, INPUT_UINT8(ioaddr + REG_DEV_CTRL),INPUT_UINT8(ioaddr + REG_ISR0), INPUT_UINT8(ioaddr + REG_ISR1));
			OUTPUT_UINT8(ioaddr + REG_ISR1, SMC_ISR1_CARD_REMOVE|SMC_ISR1_CARD_INSERT);
			break;
		}
		udelay(1);
		i++;
		if(i>2000)
			break;
	}
	OUTPUT_UINT8(ioaddr + REG_PDBR, 15);		//set de-bounce to 15
	OUTPUT_UINT16(ioaddr + REG_ETU0, tp->smc_etu);		//set etu
	OUTPUT_UINT16(ioaddr + REG_GTR0, 12);//12		//set gtr to 12
	OUTPUT_UINT32(ioaddr + REG_CBWTR0, 12800);//9600	//set wt to 9600
	OUTPUT_UINT8(ioaddr + REG_RCVPR, 0x49);		//set value1_r to 4 and glitch_v to 3
	OUTPUT_UINT8(ioaddr + REG_RXTX_PP, 0x3b);	//set rxpp to 3 and txpp to 3
	
	smc_set_wclk(ioaddr, tp->smc_clock);
}


/******************************************************************************************************
 * 	Name		:	smc_dev_card_exist()
 *	Description	:	Smart card reader controler close funciton.
 *	Parameter	:	struct smartcard_m36_private *priv		: Devcie handle.
 *	Return		:	INT32				: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
static INT32 smc_dev_card_exist(struct smartcard_m36_private *priv)
{
	struct smartcard_m36_private *tp = priv;

	if(tp->inserted)
		return SUCCESS;
	else
		return !SUCCESS;
}


static void invert(UINT8 *data, INT32 n)
{
	INT32 i;
	static UINT8 swaptab[16] = {15, 7, 11, 3, 13, 5, 9, 1, 14, 6, 10, 2, 12, 4, 8, 0};
	for(i=n-1; i>=0; i--)
		data[i] = (swaptab[data[i]&0x0F]<<4) | swaptab[data[i]>>4];
}



//atr_t global_atr;

/******************************************************************************************************
 * 	Name		:	atr_config_parameter()
 *	Description	:	Initilize ATR structure.
 *	Parameter	:	atr_t atr		: Pointer to point ATR structure.
 *				UINT8 *buffer		: ATR buffer.
 *				UINT8 length		: ATR size.
 *	Return		:	INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
static UINT32 atr_init(atr_t * atr, UINT8 *buffer, UINT8 length)
{
	UINT8 TDi;
	UINT8 pointer = 0, pn = 0;
	UINT8 i;
	
	/* Check size of buffer */
	if (length < 2)
	{		
		return SMART_WRONG_ATR;
	}

	if (NULL == atr)
	{
		SMC_ERR_PRINTK("[ %s %d ], atr is NULL\n", __FUNCTION__, __LINE__);
		return ERR_PARA;
	}

	if (NULL == buffer)
	{
		SMC_ERR_PRINTK("[ %s %d ], buffer is NULL\n", __FUNCTION__, __LINE__);
		return ERR_PARA;
	}

	/* Store T0 and TS */
	atr->TS = buffer[0];
	atr->T0 = TDi = buffer[1];
	pointer = 1;

	/* Store number of historical bytes */
	atr->hbn = TDi & 0x0F;

	/* TCK is not present by default */
	(atr->TCK).present = FALSE;

	/* Extract interface bytes */
	while (pointer < length)
	{
		/* Check buffer is long enought */
		if (pointer + atr_num_ib_table[(0xF0 & TDi) >> 4] >= length)
		{			
			return SMART_WRONG_ATR;
		}
		/* Check TAi is present */
		if ((TDi | 0xEF) == 0xFF)
		{
			pointer++;
			atr->ib[pn][ATR_INTERFACE_BYTE_TA].value = buffer[pointer];
			atr->ib[pn][ATR_INTERFACE_BYTE_TA].present = TRUE;
		}
		else
			atr->ib[pn][ATR_INTERFACE_BYTE_TA].present = FALSE;
		/* Check TBi is present */
		if ((TDi | 0xDF) == 0xFF)
		{
			pointer++;
			atr->ib[pn][ATR_INTERFACE_BYTE_TB].value = buffer[pointer];
			atr->ib[pn][ATR_INTERFACE_BYTE_TB].present = TRUE;
		}
		else
			atr->ib[pn][ATR_INTERFACE_BYTE_TB].present = FALSE;

		/* Check TCi is present */
		if ((TDi | 0xBF) == 0xFF)
		{
			pointer++;
			atr->ib[pn][ATR_INTERFACE_BYTE_TC].value = buffer[pointer];
			atr->ib[pn][ATR_INTERFACE_BYTE_TC].present = TRUE;
		}
		else
			atr->ib[pn][ATR_INTERFACE_BYTE_TC].present = FALSE;

		/* Read TDi if present */
		if ((TDi | 0x7F) == 0xFF)
		{
			pointer++;
			TDi = atr->ib[pn][ATR_INTERFACE_BYTE_TD].value = buffer[pointer];
			atr->ib[pn][ATR_INTERFACE_BYTE_TD].present = TRUE;
			(atr->TCK).present = ((TDi & 0x0F) != ATR_PROTOCOL_TYPE_T0);
			if (pn >= ATR_MAX_PROTOCOLS)
			{
				return SMART_WRONG_ATR;
			}
			pn++;
		}
		else
		{
			atr->ib[pn][ATR_INTERFACE_BYTE_TD].present = FALSE;
			break;
		}
	}

	/* Store number of protocols */
	atr->pn = pn + 1;

	/* Store historical bytes */
	if (pointer + atr->hbn >= length)
	{		
		return SMART_WRONG_ATR;
	}

//	memcpy (atr->hb, buffer + pointer + 1, atr->hbn);
	for(i=0; i<atr->hbn; i++)
		*(atr->hb+i) = *(buffer+pointer+1+i);
	pointer += (atr->hbn);

	/* Store TCK  */
	if ((atr->TCK).present)
	{

		if (pointer + 1 >= length)
		{			
			return SMART_WRONG_ATR;
		}

		pointer++;

		(atr->TCK).value = buffer[pointer];
	}

	atr->length = pointer + 1;
	return SMART_NO_ERROR;
}

/******************************************************************************************************
 * 	Name		:	atr_config_parameter()
 *	Description	:	By analyze ATR data to get the card information.
 *	Parameter	:	struct smartcard_private *handler	: Private structure handle.
 *				atr_t atr				: Pointer to point ATR buffer.
 *	Return		:	INT32					: SUCCESS or FAIL.
 *
 ******************************************************************************************************/

static UINT32 atr_config_parameter(struct smartcard_m36_private *handler, atr_t *atr)
{
	UINT8 i,checksum=0;
	UINT8 FI, DI, II, PI1, PI2, N, WI;
	UINT32 F, D, I, P;
	UINT8 T = 0xFF;
	//UINT16 rcnt_etu;
	//UINT16 rcnt_3etu;
	//UINT16 etu;

	if (atr->ib[0][ATR_INTERFACE_BYTE_TA].present)
	{
		FI = (atr->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0) >> 4;
		F = atr_f_table[FI];
		//robbin change
		if(F == F_RFU)
			F = ATR_DEFAULT_F;
		SMC_PRINTK("ATR: Clock Rate Conversion F=%d, FI=%d\n", F, FI);
	}
	else
	{
		F = ATR_DEFAULT_F;
		SMC_PRINTK("ATR: Clock Rate Conversion F=(Default)%d\n", F);
	}
	handler->F = F;
	
	if (atr->ib[0][ATR_INTERFACE_BYTE_TA].present)
	{
		DI = (atr->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F);
		D = atr_d_table[DI];
		if(D == D_RFU)
			D = ATR_DEFAULT_D;
		SMC_PRINTK("ATR: Bit Rate Adjustment Factor D=%d, DI=%d\n", D, DI);
	}
	else
	{
		D = ATR_DEFAULT_D;
		SMC_PRINTK("ATR: Bit Rate Adjustment Factor D=(Default)%d\n", D);
	}
	handler->D = D;

	if (atr->ib[0][ATR_INTERFACE_BYTE_TB].present)
	{
		II = (atr->ib[0][ATR_INTERFACE_BYTE_TB].value & 0x60) >> 5;
		I = atr_i_table[II];
		if(I == I_RFU)
			I = ATR_DEFAULT_I;
		SMC_PRINTK("ATR: Programming Current Factor I=%d, II=%d\n", I, II);
	}
	else
	{
		I= ATR_DEFAULT_I;
		SMC_PRINTK("ATR: Programming Current Factor I=(Default)%d\n", I);	
	}
	handler->I = I;
	
	if (atr->ib[1][ATR_INTERFACE_BYTE_TB].present)
	{
		PI2 = atr->ib[1][ATR_INTERFACE_BYTE_TB].value;
		P = PI2;
		SMC_PRINTK("ATR: Programming Voltage Factor P=%d, PI2=%d\n", P, PI2);
	}
	else if (atr->ib[0][ATR_INTERFACE_BYTE_TB].present)
	{
		PI1 = (atr->ib[0][ATR_INTERFACE_BYTE_TB].value & 0x1F);
		P = PI1;
		SMC_PRINTK("ATR: Programming Voltage Factor P=%d, PI1=%d\n", P, PI1);
	}
	else
	{
		P = ATR_DEFAULT_P;
		SMC_PRINTK("ATR: Programming Voltage Factor P=(Default)%d\n", P);
	}
	handler->P = P;
	
	if (atr->ib[0][ATR_INTERFACE_BYTE_TC].present)
	{
		N = atr->ib[0][ATR_INTERFACE_BYTE_TC].value;
		/*if(N == 0xFF)
			N = 11;
		*/
	}
	else
		N = ATR_DEFAULT_N;
	SMC_PRINTK("ATR: Extra Guardtime N=%d\n", N);
	handler->N = N;
	
	//for (i=0; i<ATR_MAX_PROTOCOLS; i++)
	//robbin change
	for (i=0; i<atr->pn; i++)
		if (atr->ib[i][ATR_INTERFACE_BYTE_TD].present && (0xFF == T))
		{
			/* set to the first protocol byte found */
			T = atr->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F;
			SMC_PRINTK("ATR: default protocol: T=%d\n", T);
		}
	//if has TA2, it indicate the special protocal
	if (atr->ib[1][ATR_INTERFACE_BYTE_TA].present)
	{
		T = atr->ib[1][ATR_INTERFACE_BYTE_TA].value & 0x0F;
		SMC_PRINTK("ATR: specific mode found: T=%d\n", T);
		if (atr->ib[1][ATR_INTERFACE_BYTE_TA].value & 0x10) //check TA(2), bit 5
			handler->TA2_spec = 0;        //use the default value of F/D
		else
			handler->TA2_spec = 1;         //Use the value specified in the ATR 
	}
	
	if (0xFF == T)
	{
		SMC_PRINTK("ATR: no default protocol found in ATR. Using T=0\n");
		T = ATR_PROTOCOL_TYPE_T0;
	}
	handler->T = T;

	if(handler->D != 0)
		handler->smc_etu = handler->F/handler->D;
	handler->first_cwt = FIRST_CWT_VAL; 
	handler->cwt = CWT_VAL;
	if(0 == T)
	{
		if (atr->ib[2][ATR_INTERFACE_BYTE_TC].present)
		{
			WI = atr->ib[2][ATR_INTERFACE_BYTE_TC].value;
		}
		else
			WI = ATR_DEFAULT_WI;
		SMC_PRINTK("ATR: Work Waiting Time WI=%d\n", WI);
		handler->WI = WI;
		
		handler->first_cwt = handler->cwt = (960*WI*handler->F)/(handler->smc_clock/1000)+1;
	}
	else if(1 == T)
	{
		for (i = 1 ; i < atr->pn ; i++) 
    		{
      		  /* check for the first occurance of T=1 in TDi */
       		 if (atr->ib[i][ATR_INTERFACE_BYTE_TD].present && 
            			(atr->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F) == ATR_PROTOCOL_TYPE_T1) 
        		{
         			   /* check if ifsc exist */
           			 if (atr->ib[i + 1][ATR_INTERFACE_BYTE_TA].present)
           			 	handler->T1.ifsc = atr->ib[i + 1][ATR_INTERFACE_BYTE_TA].value;
				else
					handler->T1.ifsc = ATR_DEFAULT_IFSC; /* default 32*/
                
           			 /* Get CWI */
           			 if (atr->ib[i + 1][ATR_INTERFACE_BYTE_TB].present)
            			    	handler->CWI = atr->ib[i + 1][ATR_INTERFACE_BYTE_TB].value & 0x0F;
           			 else
                			handler->CWI = ATR_DEFAULT_CWI; /* default 13*/
                		handler->cwt =  (((1<<(handler->CWI))+11 )*handler->smc_etu)/(handler->smc_clock/1000) + 1; 
           			 /*Get BWI*/
           			 if (atr->ib[i + 1][ATR_INTERFACE_BYTE_TB].present)
            				handler->BWI = (atr->ib[i + 1][ATR_INTERFACE_BYTE_TB].value & 0xF0) >> 4;
            			else
              			handler->BWI = ATR_DEFAULT_BWI; /* default 4*/    
				//handler->first_cwt= (((1<<(handler->BWI))*960+11)*handler->smc_etu)/(handler->smc_clock/1000); 		
                		handler->first_cwt = (11*handler->smc_etu)/(handler->smc_clock/1000)+((1<<(handler->BWI))*960*ATR_DEFAULT_F)/(handler->smc_clock/1000) + 2;	
            			if (atr->ib[i + 1][ATR_INTERFACE_BYTE_TC].present)
                			checksum = atr->ib[i + 1][ATR_INTERFACE_BYTE_TC].value & 0x01;
            			else
                			checksum = ATR_DEFAULT_CHK; /* default - LRC */
				handler->error_check_type = ((checksum==ATR_DEFAULT_CHK)?IFD_PROTOCOL_T1_CHECKSUM_LRC:IFD_PROTOCOL_T1_CHECKSUM_CRC);
            
        		}
    		}
		SMC_PRINTK("T1 special: ifsc: %d,  CWI:%d,  BWI:%d, checksum:%d(3:LRC,2:CRC)\n",
						handler->T1.ifsc, handler->CWI, handler->BWI, handler->error_check_type);
	}	
	
	SMC_PRINTK("First CWT: %d, CWT: %d\n", handler->first_cwt, handler->cwt);
	SMC_PRINTK("ATR: HC:");
	for(i=0; i<atr->hbn; i++)
	{
		SMC_PRINTK("%c ", (atr->hb)[i]);
	}
	SMC_PRINTK("\n");
	
	return SMART_NO_ERROR;
}


static void smc_dev_clear_tx_rx_buf(struct smartcard_m36_private *tp)
{
	tp->smc_tx_buf = NULL;
    	tp->smc_rx_buf = NULL;
	tp->smc_tx_rd = 0;
	tp->smc_tx_wr = 0;
	tp->smc_rx_head = 0;
	tp->got_first_byte = 0;
	tp->smc_rx_tail = 0;
}

/******************************************************************************************************
 * 	Name		:	smc_dev_get_card_etu()
 *	Description	:	To get the smart card ETU.
 *	Parameter	:	struct smartcard_m36_private *priv	: Devcie handle.
 *	Return		:	INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
static INT32 smc_dev_get_card_etu(struct smartcard_m36_private *priv)
{
	//int i=0;
	UINT32 ioaddr = priv->base_addr;
	UINT32 etu=0,etu3=0;
	UINT32 wai_atr_tmo = 0, wai_atr_time = 0;
	UINT8 cc = 0;
	UINT16 rx_cnt =0;
	UINT8 etu_trigger1 = 0;
	UINT8 etu_trigger2 = 0;
	UINT32 old_etu_tick1=0, old_etu_tick2=0;
	struct smartcard_m36_private *tp = priv;

//	SMC_PRINTK("%s ioaddr: %x\n",__FUNCTION__,ioaddr);
	
	OUTPUT_UINT8( ioaddr + REG_SCR_CTRL, SMC_SCR_CTRL_OP|SMC_SCR_CTRL_RECV|(tp->parity_odd<<4));
	udelay(200);
	
	OUTPUT_UINT8(ioaddr + REG_ISR0, 0xff);
	OUTPUT_UINT8(ioaddr + REG_ISR1, 0xff);
	
	//no fifo mode
	//OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,0x00);
	OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,SMC_FIFO_CTRL_EN|SMC_FIFO_CTRL_TX_OP|SMC_FIFO_CTRL_RX_OP);                                         
	smc_write_rx(ioaddr,32);
	local_irq_disable();
	if(tp->inserted != 1)  //card not insert
	{
		SMC_ERR_PRINTK("[ %s %d ], error.\n", __FUNCTION__, __LINE__);
		local_irq_enable();
		return RET_FAILURE;
	}
	if(tp->warm_reset)
	{
		if(tp->apd_disable)
		{
			OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_PRT_EN);
		}
		else
		{
			OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_PRT_EN|0x40);
		}
	}
	else	
	{
		if(tp->apd_disable)
		{
			OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_PRT_EN);
		}
		else	
		{
			OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_PRT_EN|0x40);
		}
		mdelay((ATV_VCC2IO>>1)/1000);
		if(tp->use_gpio_vpp)
		{
			if(tp->internal_ctrl_vpp)
				OUTPUT_UINT8(ioaddr + REG_CLK_VPP, INPUT_UINT8(ioaddr + REG_CLK_VPP)|SMC_RB_CTRL_VPP);
			else
			{				
				gpio_direction_output(tp->gpio_vpp_pos, tp->gpio_vpp_pol);
			}
		}
		mdelay(/*20*/(ATV_VCC2IO>>1)/1000);
		OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_DIO);
	}
	local_irq_enable();
	if(!tp->warm_reset)
	{
		udelay(/*200*/ATV_IO2CLK);
		OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_CLK);
		udelay(200);
	}
	
	while(1)
	{
		if(tp->isr1_interrupt_status & SMC_ISR1_RST_LOW)
		{
			tp->isr1_interrupt_status &= (~SMC_ISR1_RST_LOW);
			SMC_PRINTK("atr_on_low trigger!\n");			
			break;
		}
		if(tp->isr1_interrupt_status & SMC_ISR1_COUNT_ST)
		{
			SMC_PRINTK("set rst to high\n");
			tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
			tp->isr1_interrupt_status &= (~SMC_ISR1_COUNT_ST);
			
			OUTPUT_UINT8(ioaddr + REG_ISR1, INPUT_UINT8(ioaddr + REG_ISR1)|SMC_ISR1_RST_NATR);
			OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_RST);
			while(0==smc_read_rx(ioaddr))
			{			
				if( (tp->isr1_interrupt_status & SMC_ISR1_RST_NATR)&&(etu_trigger1==0))
				{
					tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
					etu_trigger1 = 1;
					old_etu_tick1 = osal_get_tick();
				}
				if(etu_trigger1)
				{
					if((osal_get_tick() - old_etu_tick1) > 100)
					{
						SMC_ERR_PRINTK("[ %s %d ], error.\n", __FUNCTION__, __LINE__);
						return !RET_SUCCESS;
					}
				}
				if(smc_dev_card_exist(priv) != SUCCESS)
				{
					SMC_ERR_PRINTK("[ %s %d ], error.\n", __FUNCTION__, __LINE__);
					return !RET_SUCCESS;
				}
			}
			cc = INPUT_UINT8(ioaddr + REG_RBR);
			if(0x3b==cc)
			{
				tp->inverse_convention = 0;
				SMC_PRINTK("Normal card %02x\n", cc);
			}
			else
			{
				tp->inverse_convention = 1;
				SMC_PRINTK("Inverse card %02x\n", cc);
			}
			while(1)
			{
				if(tp->isr1_interrupt_status & SMC_ISR1_RST_HIGH)
				{
					tp->isr1_interrupt_status &= (~SMC_ISR1_RST_HIGH);
					break;
				}
				
				if( (tp->isr1_interrupt_status & SMC_ISR1_RST_NATR)&&(etu_trigger2==0))
				{
					tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
					etu_trigger2 = 1;
					old_etu_tick2 = osal_get_tick();
				}
				if(etu_trigger2)
				{
					if((osal_get_tick() - old_etu_tick2) > 100)
					{
						SMC_ERR_PRINTK("[ %s %d ], error.\n", __FUNCTION__, __LINE__);
						return !RET_SUCCESS;
					}
				}
				
				if(smc_dev_card_exist(priv) != SUCCESS)
				{
					SMC_ERR_PRINTK("[ %s %d ], error.\n", __FUNCTION__, __LINE__);
					return !RET_SUCCESS;
				}
				//udelay(1);
				//Why there is no time out??
			}
			break;
		}
		if(smc_dev_card_exist(priv) != SUCCESS) 
		{
			SMC_ERR_PRINTK("[ %s %d ], error.\n", __FUNCTION__, __LINE__);
			return !RET_SUCCESS;
		}		
	}
	wai_atr_tmo =  (9600*372*2)/(tp->smc_clock/1000);
	wai_atr_time = osal_get_tick();
	rx_cnt = smc_read_rx(ioaddr); 
	while((tp->isr0_interrupt_status & SMC_ISR0_BYTE_RECV) != SMC_ISR0_BYTE_RECV )
	{
		if(smc_dev_card_exist(priv) != SUCCESS) 
		{
			SMC_ERR_PRINTK("[ %s %d ], error.\n", __FUNCTION__, __LINE__);
			return !RET_SUCCESS;
		}
		if(rx_cnt == smc_read_rx(ioaddr))
		{
			msleep(1);
			wai_atr_time = osal_get_tick() - wai_atr_time;
			if(wai_atr_tmo>=wai_atr_time)
				wai_atr_tmo -= wai_atr_time;
			else
			{
				SMC_PRINTK("Wait ATR time out!\n");
				return !RET_SUCCESS;
			}	
		}
		else
		{
			rx_cnt = smc_read_rx(ioaddr);
			wai_atr_tmo = (9600*372*2)/(tp->smc_clock/1000);
		}
		wai_atr_time = osal_get_tick();
	}
	//Disable receiver mode
	OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_RECV)));
	tp->isr0_interrupt_status &= (~SMC_ISR0_BYTE_RECV);
	SMC_PRINTK("Got card etu is %d, 3etu is %d. \n",INPUT_UINT16(ioaddr + REG_RCNT_ETU),INPUT_UINT16(ioaddr + REG_RCNT_3ETU));
	
	etu = INPUT_UINT16(ioaddr + REG_RCNT_ETU);

	etu3 = (INPUT_UINT16(ioaddr + REG_RCNT_3ETU)/3-15);
	etu = (etu3<DFT_WORK_ETU)? etu : etu3;

	SMC_PRINTK("set card etu: %d\n",etu);
	tp->smc_etu = etu;
	OUTPUT_UINT16(ioaddr + REG_ETU0, etu);			//Set right etu

	return RET_SUCCESS;
}

static INT32 smc_dev_set_pps(struct smartcard_m36_private *priv, UINT8 PPS0, UINT8 FI, UINT8 DI)
{
	UINT8 pps_buf[4], pps_echo[4];
	INT32 rlt = (!RET_SUCCESS);
	UINT16 actsize, rw_len;
	pps_buf[0] = 0xff;
	pps_buf[1] = PPS0;
	if(PPS0&0x10)
	{
		pps_buf[2] = ((FI&0xf)<<4)|(DI&0xf);
		pps_buf[3] = pps_buf[0]^pps_buf[1]^pps_buf[2];
		SMC_PRINTK("ppss %02x pps0 %02x pps1 %02x pck %02x\n", pps_buf[0], pps_buf[1], pps_buf[2], pps_buf[3]);
	}
	else
	{
		pps_buf[2] = pps_buf[0]^pps_buf[1];
		SMC_PRINTK("ppss %02x pps0 %02x pck %02x\n", pps_buf[0], pps_buf[1], pps_buf[2]);
	}
		
	if(PPS0&0x10)
		rw_len = 4;
	else
		rw_len = 3;
	memset(pps_echo, 0x5a, 4);
	smc_dev_transfer_data(priv, pps_buf, rw_len, pps_echo, rw_len, &actsize);
	//smc_dev_write(dev, pps_buf, rw_len, &actsize);
	
	//smc_dev_read(dev, pps_echo, rw_len, &actsize);
	SMC_PRINTK("pps echo: %02x %02x %02x %02x\n", pps_echo[0], pps_echo[1], pps_echo[2], pps_echo[3]);
	if(actsize)
	{
		if(pps_buf[0] == pps_echo[0])
		{
			if((pps_buf[1]&0xf)==(pps_echo[1]&0xf))
			{
				if(PPS0&0x10)
				{
					if((pps_buf[1]&0x10)==(pps_echo[1]&0x10))
					{
						rlt  =RET_SUCCESS;
						SMC_PRINTK("pps SUCCESS!\n");
					}
					//else
					//	SMC_PRINTK("pps: USE default FI, DI\n");
				}
				else
				{
					rlt  =RET_SUCCESS;
					SMC_PRINTK("pps SUCCESS!\n");
				}
			}
			//else
			//	SMC_PRINTK("pps : USE default T!\n");
		}
		//else
		//	SMC_PRINTK("card don't support PPS!\n");
	}
	//else
	//	SMC_PRINTK("pps got NO Response!\n");
	
	return rlt;
}

static INT32 smc_set_pps(struct smartcard_m36_private *priv , UINT32 fi_fd)
{
	
	UINT8 fi,fd;

	fi = (fi_fd&0x000000F0)>>4;
	fd = fi_fd&0x0000000F;
	
	return smc_dev_set_pps(priv,0x10,fi,fd);
}


static INT32 smc_check_atr(UINT32 ioaddr, UINT8 *buffer, UINT8 size, UINT8 start_addr)
{
	UINT8 i = 0;
	atr_t * atr_info =NULL;
	INT32 ret = RET_FAILURE;
	
	if (NULL == buffer)
	{
		SMC_ERR_PRINTK("[ %s %d ] error:  Invalid parameters!\n", __FUNCTION__, __LINE__);
		return RET_FAILURE;
	}

	SMC_PRINTK("[ %s %d ], size = %d, start_addr = %d\n", __FUNCTION__, __LINE__, size, start_addr);


	atr_info = (atr_t *)kmalloc(sizeof(atr_t), GFP_KERNEL);
	if(NULL== atr_info)
	{
		SMC_ERR_PRINTK("[ %s ], Memory allocate failed.\n", __FUNCTION__);
		return RET_FAILURE;
	}
	memset(atr_info, 0, sizeof(atr_t));
	
	for(i=0; i<size; i++)
	{
		buffer[start_addr + i] = INPUT_UINT8(ioaddr + REG_RBR);
		SMC_PRINTK("%d[%02x] ", (start_addr + i), buffer[start_addr + i]);		
	}	
	SMC_PRINTK("\n");

	if(SMART_NO_ERROR == atr_init(atr_info, buffer, size+start_addr))
	{
		ret = SUCCESS;;
	}
	else
	{
		ret = RET_FAILURE;
	}


	if (NULL != atr_info)
	{
		kfree(atr_info);
		atr_info = NULL;		
	}			

	return ret;
}


/******************************************************************************************************
 * 	Name		:	smc_dev_get_card_atr()
 *	Description	:	To get the smart card ATR.
 *	Parameter	:	struct smartcard_m36_private *priv	: Devcie handle.
 *				UINT8 *buffer		: Read data buffer.
 *				UINT16 *atr_size	: ATR data size.
 *	Return		:	INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
static INT32 smc_dev_get_card_atr(struct smartcard_m36_private *priv, UINT8 *buffer, UINT16 *atr_size)
{
	int i=0;	
	UINT32 wai_atr_tmo = 0, wai_atr_time = 0;	
	UINT16 rx_cnt=0;
	unsigned char cnt_trigger = 0;
	UINT32 old_tick = 0;
	UINT32 ioaddr = priv->base_addr;
	struct smartcard_m36_private *tp = priv;
	UINT8 temp_buffer[ATR_MAX_SIZE];
	UINT8 temp_atr_size = 0;
	INT32 ret = RET_FAILURE;	

	
	memset(temp_buffer, 0x00, sizeof(temp_buffer));
	tp->force_tx_rx_thld = FORCE_TX_RX_THLD;
	
	//enable receiver mode and set to the direct mode
	OUTPUT_UINT8( ioaddr + REG_SCR_CTRL, SMC_SCR_CTRL_OP|SMC_SCR_CTRL_RECV|(tp->parity_odd<<4));

	OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,SMC_FIFO_CTRL_EN|SMC_FIFO_CTRL_TX_OP|SMC_FIFO_CTRL_RX_OP);                                         
	smc_write_rx(ioaddr, ATR_MAX_SIZE);
	
	//OUTPUT_UINT8(ioaddr + REG_IER0, 0x30);
	
	// clear interrupt status
	OUTPUT_UINT8(ioaddr + REG_ISR1, INPUT_UINT8(ioaddr + REG_ISR1));
	OUTPUT_UINT8(ioaddr + REG_ISR0, INPUT_UINT8(ioaddr + REG_ISR0));
	                                         
	local_irq_disable();
	if(tp->inserted != 1)  //card not insert
	{
		local_irq_enable();
		return RET_FAILURE;
	}
	
	if(tp->warm_reset)
	{		
		if(tp->inverse_convention)
		{			
			SMC_PRINTK("Already know it's iverse card, dis apd\n");
			OUTPUT_UINT8(ioaddr + REG_ICCR, (INPUT_UINT8(ioaddr + REG_ICCR)&(~0x30))|SMC_RB_ICCR_AUTO_PRT);
		}
		else
		{
			SMC_PRINTK("Already know it's directed card, parity %d, apd %d\n", !tp->parity_disable, !tp->apd_disable);
			OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)&0xcf);
			OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|(tp->parity_disable<<5)|(tp->apd_disable<<4));			
		}
	}
	else	
	{		
		if(tp->inverse_convention)
		{			
			SMC_PRINTK("Already know it's iverse card, dis apd\n");
			if(tp->apd_disable)
			{				
				OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_AUTO_PRT);
			}
			else	
			{				
				OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_AUTO_PRT|0x40);
			}
		}
		else
		{			
			SMC_PRINTK("Already know it's directed card, parity %d, apd %d\n", !tp->parity_disable, !tp->apd_disable);
			if(tp->apd_disable)
			{
				OUTPUT_UINT8(ioaddr + REG_ICCR, (tp->parity_disable<<5)|(tp->apd_disable<<4));
			}
			else	
			{
				OUTPUT_UINT8(ioaddr + REG_ICCR, (tp->parity_disable<<5)|(tp->apd_disable<<4)|0x40);
			}
		}
		mdelay((ATV_VCC2IO>>1)/1000);
		if(tp->use_gpio_vpp)
		{
			if(tp->internal_ctrl_vpp)
				OUTPUT_UINT8(ioaddr + REG_CLK_VPP, INPUT_UINT8(ioaddr + REG_CLK_VPP)|SMC_RB_CTRL_VPP);
			else
			{				
				gpio_direction_output(tp->gpio_vpp_pos, tp->gpio_vpp_pol);
			}
		}
		mdelay(/*20*/(ATV_VCC2IO>>1)/1000);		
		OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_DIO);
	}	
	
	if (1 == tp->parity_disable)
	{
		OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|0x20);		
	}
	
	local_irq_enable();

	if(!tp->warm_reset)
	{
		udelay(/*200*/ATV_IO2CLK);
		OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_CLK);
		udelay(200);
	}
	
	while(1)
	{
		if(tp->isr1_interrupt_status & SMC_ISR1_RST_LOW)
		{
			SMC_PRINTK("atr_on_low trigger!\n");
			tp->isr1_interrupt_status &= (~SMC_ISR1_RST_LOW);
			break;
		}
		if(tp->isr1_interrupt_status & SMC_ISR1_COUNT_ST)
		{			
			SMC_PRINTK("set rst to high\n");
			tp->isr1_interrupt_status &= (~SMC_ISR1_COUNT_ST);
			tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
			
			OUTPUT_UINT8(ioaddr + REG_ISR1, INPUT_UINT8(ioaddr + REG_ISR1)|SMC_ISR1_RST_NATR);
			OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_RST);
			
			while(1)
			{				
				if(tp->isr1_interrupt_status & SMC_ISR1_RST_HIGH)
				{
					tp->isr1_interrupt_status &= (~SMC_ISR1_RST_HIGH);
					break;
				}			
				if(( tp->isr1_interrupt_status & SMC_ISR1_RST_NATR)&&(cnt_trigger == 0))
				{
					tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
					cnt_trigger = 1;
					old_tick = osal_get_tick();
					
				}
				if(cnt_trigger)
				{		
					if((osal_get_tick() - old_tick) > 100)
					{
						tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
						SMC_PRINTK("None ATR!\n");
						return !RET_SUCCESS;
					}
				}
				
				if(smc_dev_card_exist(priv) != SUCCESS)
				{
					return !RET_SUCCESS;
				}				
			}
			break;
		}
		if(smc_dev_card_exist(priv) != SUCCESS)
		{
			return !RET_SUCCESS;	
		}
	}
	wai_atr_tmo =  (9600*2*INPUT_UINT16(ioaddr + REG_ETU0))/(tp->smc_clock/1000);	
	wai_atr_time = osal_get_tick();
	rx_cnt = smc_read_rx(ioaddr); 	
	
	ret = smc_check_atr(ioaddr, temp_buffer, rx_cnt, temp_atr_size);
	temp_atr_size += rx_cnt;
	if (SUCCESS == ret)
	{		
		goto got_atr;
	}	
	
	while(1)
	{		
		if(smc_dev_card_exist(priv) != SUCCESS)
		{
			return !RET_SUCCESS;
		}
		if((tp->isr0_interrupt_status & SMC_ISR0_FIFO_RECV )||( tp->isr0_interrupt_status & SMC_ISR0_TIMEOUT))
		{			
			tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_RECV);
			tp->isr0_interrupt_status &= (~SMC_ISR0_TIMEOUT);		
			SMC_PRINTK("[ %s %d ], timeout\n", __FUNCTION__, __LINE__);
			break;
		}
		if(0==(INPUT_UINT8(ioaddr + REG_ICCR)&0x30)&&(tp->isr0_interrupt_status&SMC_ISR0_PE_RECV))
		{
			SMC_ERR_PRINTK("[ %s ], Get ATR Parity Error!\n", __FUNCTION__);
			tp->isr0_interrupt_status &= (~SMC_ISR0_PE_RECV);
			return !RET_SUCCESS;
		}
		if(rx_cnt == smc_read_rx(ioaddr))
		{
			msleep(1);
			wai_atr_time = osal_get_tick() - wai_atr_time;
			if(wai_atr_tmo>=wai_atr_time)
			{
				wai_atr_tmo -= wai_atr_time;
			}
			else
			{
				if(smc_read_rx(ioaddr))
				{
					SMC_PRINTK("[ %s %d ], got atr\n", __FUNCTION__, __LINE__);
					break;
				}				
				SMC_PRINTK("Wait ATR time out!\n");				
				return !RET_SUCCESS;
			}	
		}
		else
		{
			rx_cnt = smc_read_rx(ioaddr);
			wai_atr_tmo = (9600*2*INPUT_UINT16(ioaddr + REG_ETU0))/(tp->smc_clock/1000);
			
			ret = smc_check_atr(ioaddr, temp_buffer, rx_cnt, temp_atr_size);
			temp_atr_size += rx_cnt;			
			if (SUCCESS == ret)
			{				
				goto got_atr;
			}			
		}
		wai_atr_time = osal_get_tick();
	}
	

	got_atr:
		
	SMC_PRINTK("[ %s %d ], receive %d bytes atr\n", __FUNCTION__, __LINE__, temp_atr_size);
	if ((0 == temp_atr_size) ||(temp_atr_size > ATR_MAX_SIZE))
	{		
		if(temp_atr_size>ATR_MAX_SIZE)
		{
			temp_atr_size = ATR_MAX_SIZE;
		}
		else
		{
			return !RET_SUCCESS;
		}
	}	
	
	tp->atr_size = temp_atr_size;
	*atr_size = temp_atr_size;
	memcpy(buffer, temp_buffer, temp_atr_size);	
	for (i=0; i<temp_atr_size; i++)
	{		
		SMC_PRINTK("%02x ",buffer[i] );		
	}	
	SMC_PRINTK("\n");
		
	
	if(buffer[0]==0x03)
	{
		SMC_PRINTK("Inv card detected!\n");
		tp->inverse_convention = 1;
		//set to the inverse mode.
		OUTPUT_UINT8( ioaddr + REG_SCR_CTRL, SMC_SCR_CTRL_OP|SMC_SCR_CTRL_INVESE|(tp->parity_odd<<4));
		invert(buffer, temp_atr_size);
	}
 	else if((buffer[0]==0x3f)&&(tp->ts_auto_detect==1))
 	{
 		SMC_PRINTK("Inv card auto detected!\n");
 		tp->inverse_convention = 1;
 		//set to the inverse mode.
 		OUTPUT_UINT8( ioaddr + REG_SCR_CTRL, SMC_SCR_CTRL_OP|SMC_SCR_CTRL_INVESE|(tp->parity_odd<<4));
 		//invert(buffer, c);
 	}
	else if(buffer[0]==0x3b)
	{
		SMC_PRINTK("Normal card detected!\n");
		tp->inverse_convention = 0;
	}
	else
	{
		return -EINVAL;
	}
	
	//disable receiver mode.
	OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_RECV)));

	return RET_SUCCESS;
}

static INT32 smc_warm_reset(struct smartcard_m36_private *priv)
{
	UINT32 base_addr = priv->base_addr;
	//struct smartcard_m36_private *tp = priv;

	OUTPUT_UINT8(base_addr + REG_ICCR, (INPUT_UINT8(base_addr + REG_ICCR) & (~SMC_RB_ICCR_RST)));		// RST L
	return RET_SUCCESS;
}

static void smc_set_etu(struct smartcard_m36_private *priv, UINT32 new_etu)
{
	struct smartcard_m36_private *tp = priv;
	UINT32 ioaddr = priv->base_addr;
	UINT8 FI = 1;
	UINT8 DI = 1;

	if (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].present)
	{
		FI = (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0) >> 4;
		DI =  (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F); 
	}
	if(tp->reseted)
	{
		if(tp->T==0)
		{
			UINT8 WI = ATR_DEFAULT_WI;
				
			if (tp->atr_info->ib[2][ATR_INTERFACE_BYTE_TC].present)
				WI = tp->atr_info->ib[2][ATR_INTERFACE_BYTE_TC].value;
			
			OUTPUT_UINT32(ioaddr + REG_CBWTR0, 960*WI*DI);
		}
		else if(tp->T==1)
		{
			UINT8 BWI = tp->BWI; 
			OUTPUT_UINT32(ioaddr + REG_CBWTR0, 11+((960*372*(1<<BWI)*DI)/FI));
		}
		tp->smc_etu = new_etu;
		OUTPUT_UINT16(ioaddr + REG_ETU0, tp->smc_etu);	
	}
	
}


/******************************************************************************************************
 * 	Name		:	smc_dev_reset()
 *	Description	:	Smart card reset.
 *	Parameter	:	struct smartcard_m36_private *priv	: Devcie handle.
 *				UINT8 *buffer		: Read data buffer.
 *				UINT16 *atr_size	: ATR data size.
 *	Return		:	INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
static INT32 smc_dev_reset(struct smartcard_m36_private *priv, UINT8 *buffer, UINT16 *atr_size)
{
	INT32 err = SUCCESS;
	INT32 atr_ret = SUCCESS;
	UINT8 i = 0;
	//UINT8 tmp;
	UINT32 work_etu;
	UINT32 ioaddr = priv->base_addr;
	UINT8 need_reset_etu = 0;
	UINT8 FI = 1;
	UINT8 DI = 1;
	struct smartcard_m36_private *tp = priv;	
	

	mutex_lock(&priv->smc_mutex);

	t1_init(&tp->T1);
	smc_dev_clear_tx_rx_buf(tp);
	if(tp->use_default_etu)
	{
		tp->smc_etu = tp->default_etu;
	}
	else
	{
		tp->smc_etu = DFT_WORK_ETU;
	}
	work_etu = tp->smc_etu;
	OUTPUT_UINT16(ioaddr + REG_ETU0, work_etu);	
	OUTPUT_UINT16(ioaddr + REG_GTR0, 12);//12		//set gtr to 12
	OUTPUT_UINT32(ioaddr + REG_CBWTR0, 12800);
	if(0==tp->inserted)
	{
		SMC_PRINTK("[ %s %d ], smart card not inserted!\n",
									__FUNCTION__, __LINE__);
		mutex_unlock(&priv->smc_mutex);
		return !RET_SUCCESS;
	}
	if(1==tp->reseted)
	{
		tp->reseted = 0;
		if(1==tp->warm_reset_enable)
			tp->warm_reset = 1;
	}
	else
	{
		tp->warm_reset = 0;
	}
	SMC_PRINTK("warm reset %d\n", tp->warm_reset);
	tp->isr0_interrupt_status = 0;
	tp->isr1_interrupt_status = 0;
	OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)&(~SMC_IER0_TRANS_FIFO_EMPY));
	tp->init_clk_idx = 0;
	tp->atr_rlt = SMC_ATR_NONE;
	while(tp->init_clk_idx<tp->init_clk_number)
	{
		if(tp->inserted == 0)
		{
			SMC_PRINTK("[ %s %d ], smart card not inserted!\n",
									__FUNCTION__, __LINE__);
			mutex_unlock(&priv->smc_mutex);
			return !RET_SUCCESS;
		}
		
		if(!tp->warm_reset)
		{
			tp->smc_clock = tp->init_clk_array[tp->init_clk_idx];
			SMC_PRINTK("try init clk %d, No. %d\n", tp->smc_clock, tp->init_clk_idx);
			smc_set_wclk(ioaddr, tp->smc_clock);
			msleep(1);
			smc_dev_deactive(priv);
			msleep(1);
		}
		else
		{
			msleep(1);
			smc_warm_reset(priv);
			msleep(1);
		}
		err = SUCCESS;

		if(tp->use_default_etu)
		{
			if((atr_ret = smc_dev_get_card_atr(priv,buffer,atr_size)) != SUCCESS)
			{
				if(0==tp->inverse_convention)
				{
					if(!tp->warm_reset)
					{
						smc_dev_deactive(priv);
						msleep(1);
					}
					else
					{
						smc_warm_reset(priv);
						msleep(1);
					}
					tp->inverse_convention = 1;
					if((atr_ret = smc_dev_get_card_atr(priv,buffer,atr_size)) != SUCCESS)
					{
						tp->inverse_convention = 0;
						err = (-EINVAL == atr_ret) ? -EINVAL : -ENODEV;
					}
				}
				else
				{
					err = (-EINVAL == atr_ret) ? -EINVAL :  -ENODEV;
				}
			}
		}
		else
		{
			if(smc_dev_get_card_etu(priv) == SUCCESS)
			{
				if(!tp->warm_reset)
				{
					smc_dev_deactive(priv);
					msleep(1);
				}
				else
				{
					smc_warm_reset(priv);
					msleep(1);
				}
				if((atr_ret = smc_dev_get_card_atr(priv,buffer,atr_size)) != SUCCESS)
				{	
					SMC_PRINTK("[ %s %d ], error\n", __FUNCTION__, __LINE__);
					err = (-EINVAL == atr_ret) ? -EINVAL :  -ENODEV;
				}
			}
			else
			{
				SMC_PRINTK("[ %s %d ], error.\n", __FUNCTION__, __LINE__);
				err =  -ENODEV;
			}
		}
		if(SUCCESS != err)
		{
			if(!tp->warm_reset)
			{
				tp->init_clk_idx++;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	if(SUCCESS != err)
	{
		SMC_ERR_PRINTK("[ %s %d ], error.\n", __FUNCTION__, __LINE__);
		smc_dev_deactive(priv);
		mutex_unlock(&priv->smc_mutex);
		return err;
	}
	SMC_DUMP(buffer, *atr_size);
	memset(tp->atr_info, 0, sizeof(atr_t));
	
	if(SMART_WRONG_ATR==atr_init(tp->atr_info, buffer, *atr_size))
	{
		tp->atr_rlt = SMC_ATR_WRONG;
	}
	else
	{
		tp->atr_rlt = SMC_ATR_OK;
	}
	
	atr_config_parameter(tp, tp->atr_info);
	if(tp->T == 1)
	{
		if(tp->D != 0)
		{
			tp->smc_etu = work_etu = tp->F/tp->D;
			SMC_PRINTK("work etu : %d\n",work_etu);
		}
		//OUTPUT_UINT16(ioaddr + REG_ETU0, work_etu);		//set work etu.	
		local_irq_disable();
		if(tp->inserted != 1)  //card not insert
		{
			local_irq_enable();
			mutex_unlock(&priv->smc_mutex);
			return RET_FAILURE;
		}
		if(tp->apd_disable)
			OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_AUTO_PRT|SMC_RB_ICCR_OP);
		else	
			OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_AUTO_PRT|SMC_RB_ICCR_OP|0x40);		//disable auto parity error pull down control for T=1 card.
		local_irq_enable();
	}
	else if(tp->T == 14)
	{
		local_irq_disable();
		if(tp->inserted != 1)  //card not insert
		{
			local_irq_enable();
			mutex_unlock(&priv->smc_mutex);
			return RET_FAILURE;
		}        
		if(tp->apd_disable)
			OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_PRT_EN|SMC_RB_ICCR_OP);
		else	
			OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_PRT_EN|SMC_RB_ICCR_OP|0x40);		//disable parity control for T=14 card.
		local_irq_enable();
	}
	else
	{
		if(tp->D != 0)
		{
			tp->smc_etu = work_etu = tp->F/tp->D;
			SMC_PRINTK("work etu : %d\n",work_etu);
		}
		local_irq_disable();
		if(tp->inserted != 1)  //card not insert
		{
			local_irq_enable();
			mutex_unlock(&priv->smc_mutex);
			return RET_FAILURE;
		}        
		if(tp->apd_disable)
			OUTPUT_UINT8(ioaddr + REG_ICCR, (tp->apd_disable<<4)|SMC_RB_ICCR_OP);	
		else	
			OUTPUT_UINT8(ioaddr + REG_ICCR, (tp->apd_disable<<4)|SMC_RB_ICCR_OP|0x40);	
		//OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_OP|0x40);				//enable parity and auto parity error pull down control.T=0 card.
		local_irq_enable();
	}
	if(tp->N == 0xFF)
	{
		if(tp->T == 1)
			OUTPUT_UINT16(ioaddr + REG_GTR0, 11);		//set guart time value.
		else
			OUTPUT_UINT16(ioaddr + REG_GTR0, 12);	
	}
	else
		OUTPUT_UINT16(ioaddr + REG_GTR0, 12+tp->N);	
	if(buffer != tp->atr)
		memcpy(tp->atr, buffer, *atr_size);
	//calc the bwt,cwt,bgt for T1, in us(1/1000000 second)
	if (tp->T == 1)
	{
		/*BGT = 22 etu*/
		tp->T1.BGT =  (22*tp->smc_etu)/(tp->smc_clock/1000);
		/*CWT = (2^CWI + 11) etu*/
		tp->T1.CWT =  tp->cwt;  
		/*BWT = (2^BWI*960 + 11)etu, 
		  *Attention: BWT in ms, If calc in us, it will overflow for UINT32
		  */
		tp->T1.BWT= tp->first_cwt;

		/*Add error check type*/
		t1_set_checksum(&tp->T1, tp->error_check_type);
		/*reset the T1 state to undead state*/
		//t1_set_param(&tp->T1, IFD_PROTOCOL_T1_STATE, SENDING);
		t1_set_param(&(tp->T1), IFD_PROTOCOL_T1_STATE, SENDING);
		SMC_PRINTK("T1 special: BGT:%d, CWT:%d, us BWT:%d  ms \n",tp->T1.BGT, tp->T1.CWT, tp->T1.BWT);
	}

	tp->reseted = 1;
#ifdef SUPPORT_NDS_CARD
	if((tp->T==0) && (tp->inverse_convention==1))
	{
  		if( (buffer[1] == 0x7F || buffer[1] == 0xFF || buffer[1] == 0xFD ) && 
			( buffer[2] == 0x13 || buffer[2] == 0x11 ) )
  		{
  			tp->atr_info->ib[1][ATR_INTERFACE_BYTE_TA].present = TRUE;
			need_reset_etu = 1;
  		}
	}
#endif
	if(FALSE == tp->atr_info->ib[1][ATR_INTERFACE_BYTE_TA].present)
	{
		UINT8 pps0 = 0;
		UINT32 first_cwt ; 
		UINT32 cwt;
		UINT8 T = 0xff;
		UINT8 multi_protocol = 0;
		UINT8 diff_f_d = 0;
		if(tp->disable_pps)
		{
			msleep(10);	
			mutex_unlock(&priv->smc_mutex);
			return RET_SUCCESS;
		}
		if (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].present)
		{
			UINT32 f, d;
			FI = (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0) >> 4;
			DI =  (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F); 
			f = atr_f_table[FI];
			if(F_RFU==f)
				f = ATR_DEFAULT_F;
			d = atr_d_table[DI];
			if(D_RFU==d)
				d = ATR_DEFAULT_D;
			if(ATR_DEFAULT_F!=f||ATR_DEFAULT_D!=d)
				diff_f_d = 1;
		}
		for (i=0; i<ATR_MAX_PROTOCOLS; i++)
			if (tp->atr_info->ib[i][ATR_INTERFACE_BYTE_TD].present)
			{
				UINT8 T_type = tp->atr_info->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F;
				if(0xf==T_type)
					break;
				if(0xFF == T)
					T = T_type;
				else if(T!=T_type)
				{
					T = T_type;
					multi_protocol = 1;
				}
			}
	
		if(multi_protocol||diff_f_d)
		{
			first_cwt = tp->first_cwt; 
			cwt = tp->cwt;
			if (diff_f_d)
			{
				//FI = (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0) >> 4;
				//DI =  (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F); 
				pps0 |= 0x10;
			}
			SMC_PRINTK("pps0%d T %d, FI %d, DI %d \n ", pps0, tp->T, FI, DI);
			tp->first_cwt = tp->cwt =  (960*ATR_DEFAULT_WI*ATR_DEFAULT_F)/(tp->smc_clock/1000);
		
			if(RET_SUCCESS==smc_dev_set_pps(priv, pps0|(tp->T&0xf), FI, DI))
			{
				tp->first_cwt = first_cwt;
				tp->cwt = cwt;
				need_reset_etu = 1;
				SMC_PRINTK("pps: OK, set etu as %d\n", tp->smc_etu);
			}
		}
		
	}
	else
	{	
		SMC_PRINTK("Specific mode!\n");
		if(!((tp->atr_info->ib[1][ATR_INTERFACE_BYTE_TA].value)&0x10))
			need_reset_etu = 1;
	}
	if(need_reset_etu)
	{
		if(tp->T==0)
		{
			UINT8 WI = ATR_DEFAULT_WI;
				
			if (tp->atr_info->ib[2][ATR_INTERFACE_BYTE_TC].present)
				WI = tp->atr_info->ib[2][ATR_INTERFACE_BYTE_TC].value;
			
			OUTPUT_UINT32(ioaddr + REG_CBWTR0, 960*WI*DI);
		}
		else if(tp->T==1)
		{
			UINT8 BWI = ATR_DEFAULT_BWI; 
			if (tp->atr_info->ib[i + 1][ATR_INTERFACE_BYTE_TB].present)
            			BWI = (tp->atr_info->ib[i + 1][ATR_INTERFACE_BYTE_TB].value & 0xF0) >> 4;
			OUTPUT_UINT32(ioaddr + REG_CBWTR0, 11+((960*372*(1<<BWI)*DI)/FI));
		}
		OUTPUT_UINT16(ioaddr + REG_ETU0, tp->smc_etu);	
	}
	msleep(10);	
	mutex_unlock(&priv->smc_mutex);
	return RET_SUCCESS;
}

/******************************************************************************************************
 * 	Name		:	smc_dev_deactive()
 *	Description	:	Smart card deactive.
 *	Parameter	:	struct smartcard_m36_private *priv		: Devcie handle
 *	Return		:	INT32				: SUCCESS.
 *
 ******************************************************************************************************/
static INT32 smc_dev_deactive(struct smartcard_m36_private *priv)
{
	
	UINT32 base_addr = priv->base_addr;
	struct smartcard_m36_private *tp = priv;

	OUTPUT_UINT8(base_addr + REG_ICCR, INPUT_UINT8(base_addr + REG_ICCR) & (~SMC_RB_ICCR_RST));		// RST L
	udelay(/*100*/DATV_RST2CLK);
	OUTPUT_UINT8(base_addr + REG_ICCR, INPUT_UINT8(base_addr + REG_ICCR) &(~SMC_RB_ICCR_CLK));		// CLK L
	udelay(/*100*/DATV_CLK2IO);
	OUTPUT_UINT8(base_addr + REG_ICCR, INPUT_UINT8(base_addr + REG_ICCR) &(~SMC_RB_ICCR_DIO));// DIO L
	udelay(DATV_IO2VCC>>1);
	if(tp->use_gpio_vpp)
	{	
		if(tp->internal_ctrl_vpp)
			OUTPUT_UINT8(base_addr + REG_CLK_VPP, INPUT_UINT8(base_addr + REG_CLK_VPP)&(~SMC_RB_CTRL_VPP));
		else
		{
			//HAL_GPIO_BIT_SET(tp->gpio_vpp_pos, !(tp->gpio_vpp_pol));
			gpio_direction_output(tp->gpio_vpp_pos, !(tp->gpio_vpp_pol));
		}
		SMC_PRINTK("vpp Low! ");
		//udelay(100);
	}
	udelay(/*100*/DATV_IO2VCC>>1);
	OUTPUT_UINT8(base_addr + REG_ICCR, INPUT_UINT8(base_addr + REG_ICCR) | SMC_RB_ICCR_VCC);		// VCC Inactive
	SMC_PRINTK("%s: over\n", __FUNCTION__);
	tp->reseted = 0;

	return RET_SUCCESS;
}

/******************************************************************************************************
 * 	Name		:   smc_dev_check_transfer_complete()						    
 *	Description	:   To check the transmit whether it is over.
 *	Parameter	:   struct smartcard_m36_private *priv	: Devcie handle.
 *			    UINT8 *buffer		: Transmit data buffer.
 *			    UINT8 trans_mode		: Direction of transmit.
 *			    UINT8 F_T			: Use the FIFO trigger or TIMEOUT trigger 
 *								to detect transfer complete.
 *			    UINT16 *actsize 		: Actual size of response data.
 *	Return		:   INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
#if 0
static INT32 smc_dev_check_transfer_complete(struct smartcard_m36_private *priv, UINT8 *buffer, UINT8 trans_mode, UINT8 F_T, UINT16 *actsize)
{
//	struct smartcard_m36_private *tp = priv;
//	UINT32 ioaddr = priv->base_addr;

	return RET_SUCCESS;

}
#endif
/******************************************************************************************************
 * 	Name		:   smc_dev_transfer_data()						    
 *	Description	:   Send the command or data to the device.
 *	Parameter	:   struct smartcard_m36_private *priv	: Devcie handle
 *			    UINT8 *buffer		: Wrtie data buffer.
 *			    UINT16 size			: Write data size.
 *			    UINT8 *recv_buffer		: Receive buffer to store the response data from the card.
 *			    UINT16 reply_num		: Response number.
 *	Return		:   INT32			: SUCCESS or FAIL
 *
 ******************************************************************************************************/
static INT32 smc_dev_transfer_data(struct smartcard_m36_private *priv, UINT8 *buffer, UINT16 size, UINT8 *recv_buffer, UINT16 reply_num, UINT16 *actsize)
{
	
	struct smartcard_m36_private *tp = priv;
	UINT32 ioaddr = priv->base_addr;
	UINT16 i, j;
	UINT32 loop=0;
	UINT32 flgptn, waitmo = 6000;
	INT32 result;
	UINT16 real_work_etu = INPUT_UINT16(ioaddr + REG_ETU0);
	UINT16 char_frame = INPUT_UINT16(ioaddr + REG_GTR0);
	
	SMC_PRINTK("[ %s ]: in %d, out %d\n", __FUNCTION__, size, reply_num);
	if(tp->inserted == 0)
	{
		SMC_PRINTK("[ %s %d ], smart card not inserted!\n",
									__FUNCTION__, __LINE__);
		return !RET_SUCCESS;
	}
	else if(tp->reseted != 1)
	{
		SMC_PRINTK("smc: smart card not reseted!\n");
		return !RET_SUCCESS;
	}	
	
    	tp->smc_tx_buf = buffer;
    	tp->smc_rx_buf = recv_buffer;
	tp->smc_tx_rd = 0;
	tp->smc_tx_wr = size;
	tp->smc_rx_head = 0;
	tp->got_first_byte = 0;
	if(reply_num)
		tp->smc_rx_tail = reply_num;
	else
		tp->smc_rx_tail = SMC_RX_BUF_SIZE;
    	
	if(size>smc_tx_fifo_size)
		loop = 1;
   
	//SMC_PRINTK("loop %d size %d\n",loop,size);
	//enable transmit mode disable receiver mode
	OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~(SMC_SCR_CTRL_RECV|SMC_SCR_CTRL_TRANS)));
		
    	OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)&(~(SMC_IER0_BYTE_RECV_TRIG|SMC_IER0_BYTE_TRANS_TRIG)));
	if(reply_num)
	{
		if(reply_num/smc_rx_fifo_size)
		{
			smc_write_rx(ioaddr, (smc_rx_fifo_size>>1));
			//SMC_PRINTK("set rx thld %d\n", 32);
		}
		else
		{
			smc_write_rx(ioaddr,reply_num);	
			//SMC_PRINTK("set rx thld %d\n", reply_num);
		}
	}
	else
	{
		smc_write_rx(ioaddr,0);
		//SMC_PRINTK("set rx thld %d\n", 0);
	}
	/*Always enable byte receive int*/
	OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)|SMC_IER0_BYTE_RECV_TRIG);
	
	OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL, SMC_FIFO_CTRL_EN|SMC_FIFO_CTRL_TX_OP|SMC_FIFO_CTRL_RX_OP);
	if((!tp->force_tx_rx_triger)&&(!tp->auto_tx_rx_triger))
	{
		UINT16 loop_cnt = 0;
		UINT16 loop_remain = 0;
		UINT32 tmo;
		//Enable Fifo Empty Interrupt
		OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)|SMC_IER0_TRANS_FIFO_EMPY);
		if(size>5)
		{
			loop_remain = size - 5;
			loop_cnt = loop_remain/smc_tx_fifo_size;
			loop_remain = loop_remain%smc_tx_fifo_size;
			size = 5;
		}
//		osal_flag_clear(tp->smc_flag_id, SMC_RX_BYTE_RCV|SMC_RX_FINISHED|SMC_TX_FINISHED);
		tp->smc_flag_id &= (~(SMC_RX_BYTE_RCV|SMC_RX_FINISHED|SMC_TX_FINISHED));

		OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,INPUT_UINT8(ioaddr + REG_SCR_CTRL)|SMC_SCR_CTRL_TRANS);
		for(i=0;i<loop_cnt;i++)
		{
			for(j=0;j<smc_tx_fifo_size;j++)
			{
				OUTPUT_UINT8(ioaddr + REG_THR,tp->smc_tx_buf[tp->smc_tx_rd+j]);
			}
			tp->smc_tx_rd += smc_tx_fifo_size;
			tmo = (smc_tx_fifo_size+2)*2;
			while(0==(tp->isr0_interrupt_status&SMC_ISR0_FIFO_EMPTY))
			{
				msleep(1);
				tmo--;
				if(0==tmo)
					return !RET_SUCCESS;
				if((INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80)== 0)
				{
					SMC_PRINTK("[ %s %d ], smart card not inserted!\n",
									__FUNCTION__, __LINE__);	
					return !RET_SUCCESS;
				}
			}
			tp->isr0_interrupt_status &= ~SMC_ISR0_FIFO_EMPTY;
		}
		if(loop_remain)
		{
			for(j=0;j<loop_remain;j++)
			{
				OUTPUT_UINT8(ioaddr + REG_THR,tp->smc_tx_buf[tp->smc_tx_rd+j]);
			}
			tp->smc_tx_rd += loop_remain;
			tmo = (loop_remain+2)*2;
			while(0==(tp->isr0_interrupt_status&SMC_ISR0_FIFO_EMPTY))
			{
				msleep(1);
				tmo--;
				if(0==tmo)
					return !RET_SUCCESS;
				if((INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80)== 0)
				{
					SMC_PRINTK("[ %s %d ], smart card not inserted!\n",
									__FUNCTION__, __LINE__);	
					return !RET_SUCCESS;
				}
				
			}
			tp->isr0_interrupt_status &= ~SMC_ISR0_FIFO_EMPTY;
		}
		local_irq_disable();
		for(j=0;j<size;j++)
		{
			OUTPUT_UINT8(ioaddr + REG_THR,tp->smc_tx_buf[tp->smc_tx_rd+j]);
		}
		tp->smc_tx_rd += size;
		tmo = size*2000;
		while(0==(INPUT_UINT8(ioaddr + REG_ISR0)&SMC_ISR0_FIFO_EMPTY))
		{
			udelay(1);
			tmo--;
			if(0==tmo)
			{
				local_irq_enable();
				return !RET_SUCCESS;
			}
			if((INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80)== 0)
			{
				SMC_PRINTK("[ %s %d ], smart card not inserted!\n",
									__FUNCTION__, __LINE__);	
				local_irq_enable();
				return !RET_SUCCESS;
			}
		}
		OUTPUT_UINT8(ioaddr + REG_ISR0, SMC_ISR0_FIFO_EMPTY);
		OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
		local_irq_enable();
		tp->smc_flag_id |= SMC_TX_FINISHED;
	}
	else
	{
		tp->char_frm_dura = char_frame*((real_work_etu*1000)/(tp->smc_clock/1000));
		if(tp->force_tx_rx_triger)
		{
			#define TX_RX_THLD	4000//MS
			tp->force_tx_rx_state = 0;
			/*((thld*char_faram*etu*1000)/(smc_clock/1000))>TX_RX_THLD*/
			tp->force_tx_rx_thld = ((tp->smc_clock/1000)*(TX_RX_THLD/1000))/(real_work_etu*char_frame);
			if(tp->force_tx_rx_thld < 5)
				tp->force_tx_rx_thld = 5;
			//if(tp->force_tx_rx_thld > 10)tp->force_tx_rx_thld = 10;
				
			
			SMC_PRINTK("force tx rx thld %d, char frm dura %d\n", tp->force_tx_rx_thld, tp->char_frm_dura);
		}
		if(loop)
		{
			size = smc_tx_fifo_size;
			if(0) //ALI_S3602==smc_chip_id)
				OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,(INPUT_UINT8(ioaddr + REG_FIFO_CTRL)&0xf0)|(size>>1));
			else
				smc_write_tx(ioaddr, smc_tx_fifo_size>>1);
		}
		else
		{
			if(tp->force_tx_rx_triger&&size>tp->force_tx_rx_thld)
			{
				smc_write_tx(ioaddr,size-tp->force_tx_rx_thld);
				tp->force_tx_rx_state = 1;
			}
			else
			{
				if(0) //ALI_S3602==smc_chip_id)
					OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,(INPUT_UINT8(ioaddr + REG_FIFO_CTRL)&0xf0)|(size));
				else
					smc_write_tx(ioaddr,size);
			}
		}
		//SMC_PRINTK("w fc %x\n",INPUT_UINT8(ioaddr + REG_FIFO_CTRL)|SMC_FIFO_CTRL_TX_OP|SMC_FIFO_CTRL_RX_OP|smc_tx_fifo_size);
			
		for(j=0;j<size;j++)
		{
			if((tp->smc_tx_rd+j+1)==tp->smc_tx_wr)
			{
				if(tp->auto_tx_rx_triger)
				{
					OUTPUT_UINT8(ioaddr+REG_ICCSR, 1<<5); //tx->rx auto switch
					//SMC_PRINTK("1: tx->rx auto: rd %d, cnt %d, wr %d\n", tp->smc_tx_rd, j, tp->smc_tx_wr);
				}
			}
			OUTPUT_UINT8(ioaddr + REG_THR,buffer[j]);
		}
		tp->smc_tx_rd+=size;
//		osal_flag_clear(tp->smc_flag_id, SMC_RX_BYTE_RCV|SMC_RX_FINISHED|SMC_TX_FINISHED);
		tp->smc_flag_id &= (~(SMC_RX_BYTE_RCV|SMC_RX_FINISHED|SMC_TX_FINISHED));
		
	       OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)|SMC_ISR0_FIFO_TRANS);	
	
		local_irq_disable();
		OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,INPUT_UINT8(ioaddr + REG_SCR_CTRL)|SMC_SCR_CTRL_TRANS);
		
		if(tp->force_tx_rx_triger)
		{	
			if((0==loop)&&(0==tp->force_tx_rx_state))
			{
				if(0==smc_read_tx(ioaddr))
				{	
					OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
					SMC_PRINTK("tmo1: too late!\n");
				}
				else
				{
					UINT32 force_loop_tmo = (smc_read_tx(ioaddr)+1)*tp->char_frm_dura;
					UINT8   force_tx_rx = 1;
				
					//libc_printf("tmo1 rdy: %d\n", force_loop_tmo);
//					UINT32 tmo = read_tsc();
					//if(0==smc_read_tx(ioaddr))libc_libc_printf("tmo1: too late!\n");
					
					while(0!=smc_read_tx(ioaddr))
					{
						udelay(1);
						force_loop_tmo--;
						if(!force_loop_tmo)
						{
							force_tx_rx = 0;
							SMC_PRINTK("tmo1: tmo %d\n", smc_read_tx(ioaddr));
							//libc_libc_printf("tmo1: tmo %d\n", smc_read_tx(ioaddr));
							break;
						}
						if((INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80)== 0)
						{
							SMC_PRINTK("[ %s %d ], smart card not inserted!\n",
									__FUNCTION__, __LINE__);
							local_irq_enable();
							return !RET_SUCCESS;
						}
					}
					if(force_tx_rx)
					{
						OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
					}
//					SMC_PRINTK("tmo1: %d, %d\n", force_loop_tmo, (read_tsc()-tmo)/(SYS_CPU_CLOCK / 2000000));
					//libc_printf("tmo1: %d, %d\n", force_loop_tmo, (read_tsc()-tmo)/(SYS_CPU_CLOCK / 2000000));
				}
				local_irq_enable();
			}
			else
				local_irq_enable();   
		}
		else
			local_irq_enable();   
	}
       //SMC_PRINTK("set to trans\n");

	//SMC_PRINTK("w scr %x\n",INPUT_UINT8(ioaddr + REG_SCR_CTRL)|SMC_SCR_CTRL_TRANS);
	//Wait TX ready:
	waitmo = ((tp->smc_tx_wr+1)*tp->char_frm_dura*3)/1000 + 1;
	waitmo = (waitmo<500?waitmo:500);
	flgptn = 0;
	
	//SMC_PRINTK("%s wat tx rdy with tmo %d\n", __FUNCTION__, waitmo);
//	result = osal_flag_wait(&flgptn, tp->smc_flag_id, SMC_REMOVED|SMC_TX_FINISHED, OSAL_TWF_ORW, waitmo);
	result = osal_flag_wait_tmp(&flgptn, tp, SMC_REMOVED|SMC_TX_FINISHED, waitmo);
	
	if(OSAL_E_OK!=result)
	{
		if(tp->smc_tx_wr)
        	{
        		local_irq_disable();
			if(tp->smc_tx_rd==tp->smc_tx_wr)
			{
				if(tp->force_tx_rx_triger&&1==tp->force_tx_rx_state)
				{
					smc_write_tx(ioaddr, tp->force_tx_rx_thld);
					tp->force_tx_rx_state = 2;					
				}
			}
			else
			{
				SMC_PRINTK("tx not finished: w %d: r %d!\n", tp->smc_tx_wr, tp->smc_tx_rd);
				tp->smc_tx_rd=tp->smc_tx_wr;
				if(tp->force_tx_rx_triger)
					tp->force_tx_rx_state = 2;	
				local_irq_enable();
				return !RET_SUCCESS;
			}
			local_irq_enable();
		}	
	}
	
	//SMC_PRINTK("wt tx tmo %d, rdy %d, flgptn %08x\n", (result!=OSAL_E_OK), (0!=(flgptn&SMC_TX_FINISHED)), flgptn);
	if(tp->isr0_interrupt_status&SMC_ISR0_PE_TRANS)
	{
		SMC_PRINTK("TX Parity Error!\n");
		tp->isr0_interrupt_status &= (~SMC_ISR0_PE_TRANS);
	}
	if(tp->smc_rx_tail&&tp->smc_rx_buf!=tp->smc_rx_tmp_buf&&(flgptn&SMC_TX_FINISHED)&&(!(flgptn&SMC_REMOVED)))
	{
		flgptn = 0;
		waitmo = tp->first_cwt;
		//SMC_PRINTK("%s wait 1st byte %d ms\n", __FUNCTION__, waitmo);        
//		result = osal_flag_wait(&flgptn, tp->smc_flag_id, SMC_REMOVED|SMC_RX_FINISHED|SMC_RX_BYTE_RCV, OSAL_TWF_ORW, waitmo);
		result = osal_flag_wait_tmp(&flgptn, tp, SMC_REMOVED|SMC_RX_FINISHED|SMC_RX_BYTE_RCV,  waitmo);

		if(OSAL_E_OK!=result)
		{
			*actsize = tp->smc_rx_head;
			SMC_PRINTK("%s wait 1st byte TMO with %d\n", __FUNCTION__,*actsize);  
//			osal_flag_clear(tp->smc_flag_id, SMC_RX_FINISHED|SMC_TX_FINISHED|SMC_RX_BYTE_RCV);
			tp->smc_flag_id &= (~(SMC_RX_FINISHED|SMC_TX_FINISHED|SMC_RX_BYTE_RCV));
			smc_dev_clear_tx_rx_buf(tp);
			return !RET_SUCCESS;
		}
		if(flgptn&SMC_RX_BYTE_RCV)
		{
			UINT16 current_cnt = smc_read_rx(ioaddr);
			UINT32 current_head = tp->smc_rx_head;
			
//			osal_flag_clear(tp->smc_flag_id, SMC_RX_BYTE_RCV);
			tp->smc_flag_id &= (~SMC_RX_BYTE_RCV);
			waitmo = tp->cwt;
			//waitmo = (waitmo>4000?waitmo: 4000);
			//SMC_PRINTK("%s wait %d bytes: %d ms\n", __FUNCTION__, (tp->smc_rx_tail - 1), waitmo);
			do
			{
				UINT16 tmp_cnt;
				flgptn = 0;
//				result = osal_flag_wait(&flgptn, tp->smc_flag_id, SMC_REMOVED|SMC_RX_FINISHED, OSAL_TWF_ORW, waitmo);
				result = osal_flag_wait_tmp(&flgptn, tp, SMC_REMOVED|SMC_RX_FINISHED, waitmo);

				if(result == OSAL_E_OK)		
					break;
				
				tmp_cnt = smc_read_rx(ioaddr);
				//SMC_PRINTK("loop wat: prv c %d, h %d: cur c%d, h %d\n", current_cnt, current_head, tmp_cnt, tp->smc_rx_head);
				if(tmp_cnt!=current_cnt||current_head!=tp->smc_rx_head)
				{
					current_cnt = tmp_cnt;
					current_head=tp->smc_rx_head;
					result = OSAL_E_OK;
				}
			}while(result!=OSAL_E_TIMEOUT);
		}
	}
	
	if(OSAL_E_OK!=result)
	{
		if(reply_num)
		{
			if(tp->smc_rx_tail)
			{	
				UINT32 rem_space;
				UINT32 i;
				UINT16 c, total_cnt;
				//<== patch for miss tx finish flag
				UINT32  cur_rx_head, smc_rd_tmo, tmp_rd_tick;
				cur_rx_head = tp->smc_rx_head;
				if(cur_rx_head)
					smc_rd_tmo = tp->cwt;
				else
					smc_rd_tmo = tp->first_cwt;
				tmp_rd_tick= osal_get_tick();
				while(tp->smc_rx_head<tp->smc_rx_tail )
				{
					if(tp->inserted == 0)
					{
						SMC_PRINTK("[ %s %d ], smart card not inserted!\n",
									__FUNCTION__, __LINE__);
						return !RET_SUCCESS;
					}
					msleep(1);
					if(cur_rx_head!=tp->smc_rx_head)
					{
						cur_rx_head = tp->smc_rx_head;
						smc_rd_tmo = tp->cwt;
						tmp_rd_tick = osal_get_tick();
					}
					if(smc_rd_tmo<(osal_get_tick() - tmp_rd_tick))
					{	
						smc_rd_tmo = 0;
						break;
					}
				}
				//==>Patch By Goliath
				local_irq_disable();
				rem_space = tp->smc_rx_tail - tp->smc_rx_head;
				total_cnt = c =smc_read_rx(ioaddr);
				c = (c<=rem_space?c:rem_space);			
				total_cnt -= c;
				for(i=0;i<c;i++)
					tp->smc_rx_buf[tp->smc_rx_head+i] = INPUT_UINT8(ioaddr + REG_RBR);
				tp->smc_rx_head += c; 
				local_irq_enable();
				for(i=0; i<total_cnt; i++)		
					c = INPUT_UINT8(ioaddr + REG_RBR);
				SMC_PRINTK("[ %s ], TMO with %d, rem %d \n", __FUNCTION__, tp->smc_rx_head, total_cnt);
			}
			*actsize = tp->smc_rx_head;
		
        		if(tp->smc_rx_head)
        		{
        			smc_dev_clear_tx_rx_buf(tp);
            		return RET_SUCCESS;
        		}
			SMC_PRINTK("[ %s ], Failed.\n", __FUNCTION__); 	
			smc_dev_clear_tx_rx_buf(tp);

			return !RET_SUCCESS;
		}
		else
		{
			if(tp->smc_tx_rd!=tp->smc_tx_wr)
			{
				*actsize = tp->smc_tx_rd;
				SMC_PRINTK("tx tmo %d: w %d: r %d!\n", waitmo, tp->smc_tx_wr, tp->smc_tx_rd);
				return !RET_SUCCESS;
			}
		}
		
	}
	if(flgptn&SMC_REMOVED)
	{
//		osal_flag_clear(tp->smc_flag_id, SMC_REMOVED);
		tp->smc_flag_id &= (~SMC_REMOVED);
		*actsize = 0;
		SMC_PRINTK("%s CARD removed!\n\n", __FUNCTION__);
		smc_dev_clear_tx_rx_buf(tp);
		return !RET_SUCCESS;
	}
	if(reply_num)
	{
		*actsize = tp->smc_rx_head;
	}
	else
	{
		*actsize = tp->smc_tx_rd;
	}
	SMC_PRINTK("%s SUCCESS with %d\n\n", __FUNCTION__, *actsize);
	tp->smc_tx_buf = NULL;
    //	tp->smc_rx_buf = NULL;
	tp->smc_tx_rd = 0;
	tp->smc_tx_wr = 0;
	//tp->smc_rx_head = 0;
	//tp->got_first_byte = 0;
	//tp->smc_rx_tail = 0;
	//smc_dev_clear_tx_rx_buf(tp);
	return RET_SUCCESS;
}

/******************************************************************************************************
 * 	Name		:   smc_dev_read()
 *	Description	:   UART smartcard read.
 *	Parameter	:	struct smartcard_m36_private *priv	: Devcie handle
 *					UINT8 *buffer			: Read data buffer
 *					INT16 size				: Read data size
 *					INT16 *actsize			: Read data size
 *	Return		:	INT32					: return value
 *
 ******************************************************************************************************/
static INT32 smc_dev_read(struct smartcard_m36_private *priv, UINT8 *buffer, INT16 size, INT16 *actsize)
{
	struct smartcard_m36_private *tp = priv;
	UINT32 smc_rd_tmo = tp->first_cwt;
	UINT32 rd_tick = osal_get_tick();
	UINT32 cur_rx_head = 0;
	UINT32 tmp_rd_tick= osal_get_tick();	
	INT16 k = 0;
	
	
	mutex_lock(&priv->smc_mutex);

	if(smc_read_rx(priv->base_addr))
	{
		UINT32 rem_space = 0;
		UINT32 i;
		UINT16 c, total_cnt;
		local_irq_disable();
		rem_space = tp->smc_rx_tail - tp->smc_rx_head;
		total_cnt = c =smc_read_rx(priv->base_addr);
		c = (c<=rem_space?c:rem_space);			
		total_cnt -= c;
		for(i=0;i<c;i++)
			tp->smc_rx_buf[tp->smc_rx_head+i] = INPUT_UINT8(priv->base_addr + REG_RBR);
		tp->smc_rx_head += c; 
		local_irq_enable();
	}
	cur_rx_head = tp->smc_rx_head;
	if(cur_rx_head)
		smc_rd_tmo = tp->cwt;
	while(tp->smc_rx_head<((UINT32)size))
	{
		if(tp->inserted == 0)
		{
			SMC_PRINTK("[ %s %d ], smart card not inserted!\n",
									__FUNCTION__, __LINE__);
			mutex_unlock(&priv->smc_mutex);
			return SMART_NOT_INSERTED;
		}
		else if(tp->reseted != 1)
		{
			SMC_PRINTK("smc: smart card not reseted!\n");
			mutex_unlock(&priv->smc_mutex);
			return SMART_NOT_RESETED;
		}	
		msleep(1);		
		
		/*	
		smc_rd_tmo--;
		if(!smc_rd_tmo)
			break;
		*/	
		if(cur_rx_head!=tp->smc_rx_head)
		{
			cur_rx_head = tp->smc_rx_head;
			smc_rd_tmo = tp->cwt;
			tmp_rd_tick = osal_get_tick();
		}
		if(smc_rd_tmo<(osal_get_tick() - tmp_rd_tick))
		{	
			//libc_printf("tmo: %d, size: %d\n", smc_rd_tmo, size);
			smc_rd_tmo = 0;
			break;
		}
	}
	
	local_irq_disable();
	if(((UINT32)size)>tp->smc_rx_head)
		size = tp->smc_rx_head;
	//if(tp->smc_rx_tmp_buf[0]==0x60)ASSERT(0);
		
	memcpy(buffer, (const void *)tp->smc_rx_tmp_buf, size);
	*actsize = size;
	tp->smc_rx_head -= (UINT32)size;
	if(tp->smc_rx_head)
	{			
		memcpy((void *)tp->smc_rx_tmp_buf, (const void *)(tp->smc_rx_tmp_buf+size), tp->smc_rx_head);
		__CACHE_FLUSH_ALI((unsigned long)(priv->smc_rx_tmp_buf), size+tp->smc_rx_head);		
	}
	else
	{		
		__CACHE_FLUSH_ALI((unsigned long)(priv->smc_rx_tmp_buf), size);		
	}
	local_irq_enable();
	if((0 == smc_rd_tmo) && (0==(*actsize)))
	{
		SMC_PRINTK("smc: read %d bytes TMO with %d!\n", size, osal_get_tick() - rd_tick);
		mutex_unlock(&priv->smc_mutex);
		return SMART_NO_ANSWER;
	}	

	if (0 !=  g_smc_debug)	
	{		
		SMC_PRINTK("[ %s %d ], read %d bytes: ",  __FUNCTION__, __LINE__, *actsize);					
		for(k=0; k<*actsize; k++)
		{
			SMC_PRINTK("%02x ", buffer[k]);
		}		
		SMC_PRINTK("\n");
	}
	SMC_PRINTK("read %d bytes with %d tick\n", *actsize, osal_get_tick() - rd_tick);

	mutex_unlock(&priv->smc_mutex);
	
	return RET_SUCCESS;
}

/******************************************************************************************************
 * 	Name		:   smc_dev_write()
 *	Description	:   UART CAM write.
 *	Parameter	:	struct smartcard_m36_private *priv	: Devcie handle
 *					UINT8 *buffer			: Wrtie data buffer
 *					UINT16 size				: Write data size
 *					UINT16 *actsize			: Write data size
 *	Return		:	INT32					: return value
 *
 ******************************************************************************************************/

static INT32 smc_dev_write(struct smartcard_m36_private *priv, UINT8 *buffer, INT16 size, INT16 *actsize)
{	
	struct smartcard_m36_private * tp = priv;
	INT32 rlt = 0; 			
	INT16 k = 0;
	UINT32 wr_tick = osal_get_tick();

		
	msleep(3);//Patch for S3602F	
	mutex_lock(&priv->smc_mutex);
	
	if (0 !=  g_smc_debug)
	{						
		SMC_PRINTK("[ %s %d ], write %d bytes: ", __FUNCTION__, __LINE__, size);
		for(k=0; k<size; k++)
		{
			SMC_PRINTK("%02x ", buffer[k]);
		}		
		SMC_PRINTK("\n");
	}
	
	rlt = smc_dev_transfer_data(priv, buffer, size, (UINT8 *)tp->smc_rx_tmp_buf, 0, actsize);

	mutex_unlock(&priv->smc_mutex);
	
	SMC_PRINTK("[ %s %d ], write over, cost %d tick\n", __FUNCTION__, __LINE__, osal_get_tick() - wr_tick);
	return rlt;
}

/******************************************************************************************************
 * 	Name		:   smc_dev_transfer_data()						    
 *	Description	:   After command send to analyze the reply data.
 *	Parameter	:   struct smartcard_m36_private *priv	: Devcie handle.
 *			    UINT8 INS			: The instruction code.
 *			    INT16 num_to_transfer	: Transmit data size
 *			    UINT8 *status		: Status buffer pointer.
 *			    INT16 size			: Response size.
 *	Return		:   INT32			: SUCCESS or FAIL
 *
 ******************************************************************************************************/
static INT32 smc_process_procedure_bytes(struct smartcard_m36_private *priv, UINT8 INS, INT16 num_to_transfer, UINT8 status[2])
{
	INT16 r = 0;
	INT16 act_size = 0;
	UINT8 buff = 0;

	
	//SMC_PRINTK("iso: <- PROC:\n ");
	do
	{
		do
		{
			if(smc_dev_read(priv, &buff, 1, &act_size) != RET_SUCCESS) 
			{
				SMC_PRINTK("smc pcss: 1 read error!\n");
				return -1;
			}
			//SMC_PRINTK("%02x ", buff);
		} while(buff == 0x60);	/*NULL, send by the card to reset WWT*/

		if((buff&0xF0)==0x60 || (buff&0xF0)==0x90)	 // SW1/SW2
		{
			status[0]=buff;
			if(smc_dev_read(priv, &buff, 1, &act_size) != RET_SUCCESS)
			{
				SMC_PRINTK("smc pcss: 2 read error!\n");
				return -1;
			}
			//SMC_PRINTK("%02x\n", buff);
			status[1] = buff;
			return 0;
		}
		else
		{
			if((buff ^ INS) == 0)	/* ACK == INS*/
			{
				/*Vpp is idle. All remaining bytes are transfered subsequently.*/
				r = num_to_transfer;
			}
			else if((buff ^ INS) == 0xFF)		/* ACK == ~INS*/
			{
				/*Vpp is idle. Next byte is transfered subsequently.*/
				r = 1;
			}
			else if((buff ^ INS) == 0x01)	/* ACK == INS+1*/
			{
				/*Vpp is active. All remaining bytes are transfered subsequently.*/
				r = num_to_transfer;
			}
			else if((buff ^ INS) == 0xFE)	/* ACK == ~INS+1*/
			{
				/*Vpp is active. Next bytes is transfered subsequently.*/
				r = 1;
			}
//###########################################################			
//seca exceptions
			else if(((buff ^ INS) == 0x3C)||((buff ^ INS) == 0x40))	
			{
				r = num_to_transfer;
			}

//###########################################################			
			else
			{
				SMC_PRINTK("cannot handle procedure %02x (INS=%02x)\n", buff, INS);
				return -1;
			}
			if(r > num_to_transfer)
			{
				SMC_PRINTK("data overrun r=%d num_to_transfer=%d\n", r, num_to_transfer);
				return -1;
			}
		}
	} while(r==0);
	return r;
}

/******************************************************************************************************
 * 	Name		:   	smc_dev_iso_transfer()
 *	Description	:	Combines the functionality of both write and read.
 *					Implement ISO7816-3 command transfer. 
 *	Parameter	:	struct smartcard_m36_private *priv		: Device structuer.
 *					UINT8 *command			: ISO7816 command buffer pointer.
 *					INT16 num_to_write		: Number to transmit.
 *					UINT8 *response, 			: Response data buffer pointer.
 *					INT16 num_to_read		: number to read from SMC, 0 means it's write cmd
 *					INT16 *actual_size			: pointer to the actual size received from SMC
 *	Return		:	INT32 					: SUCCESS or FAIL.
 ******************************************************************************************************/
 #define MAX_LENGTH	256	//max response length
 #define CMD_LENGTH	5
 #define CLA_OFFSET	0
 #define INS_OFFSET	1
 #define P1_OFFSET	2
 #define P2_OFFSET	3
 #define P3_OFFSET	4

static INT32 smc_dev_iso_transfer(struct smartcard_m36_private *priv, UINT8 *command, INT16 num_to_write, 
						UINT8 *response, INT16 num_to_read, INT16 *actual_size)
{
	//INT32 ret;
	INT32 error = SMART_NO_ERROR;
	UINT8 write_flag = 0;
	INT16 size;
	INT16 num_to_transfer;
	INT16 length = 0;
	INT16 temp_length = 0;
	UINT8 status[2] = {0,0};

	struct smartcard_m36_private *tp = priv;

	if(tp->inserted == 0)
	{
		SMC_PRINTK("[ %s %d ], smart card not inserted!\n",
									__FUNCTION__, __LINE__);
		return !RET_SUCCESS;
	}
	else if(tp->reseted != 1)
	{
		SMC_PRINTK("%s: smart card not reseted!\n",__FUNCTION__);
		return !RET_SUCCESS;
	}

	if(tp->T > 1)
		return SMART_INVALID_PROTOCOL;

	//SMC_PRINTK("CMD: IFD -> SMC ");
	//SMC_DUMP(command, CMD_LENGTH);

	*actual_size = 0;
	
	if(num_to_write > CMD_LENGTH)
	{
		write_flag = 1;
		num_to_transfer = command[P3_OFFSET];
		//ASSERT(num_to_transfer == (num_to_write - CMD_LENGTH))
	}
	else if(num_to_write == CMD_LENGTH)
	{
		write_flag = 0;
		//if(response == NULL) 	/* write data to smart card*/
		if(num_to_read==0)
			num_to_transfer = 0;
		else		/*read data from smart card*/
			num_to_transfer = (command[P3_OFFSET] == 0) ? MAX_LENGTH : command[P3_OFFSET];
	}
	else
	{
		SMC_PRINTK("%s: error command length!\n", __FUNCTION__);
		return !RET_SUCCESS;
	}

	SMC_PRINTK("[ %s ], write_flag = %d\n", __FUNCTION__, write_flag);

	/* Check the CLA and INS bytes are valid */
	if (command[CLA_OFFSET] != 0xFF)
	{
		if ((command[INS_OFFSET] & 0xF0) != 0x60 &&
			(command[INS_OFFSET] & 0xF0) != 0x90)
		{
			if(smc_dev_write(priv, command, CMD_LENGTH, &size) != SUCCESS)
			{
				SMC_PRINTK("%s: 1 write cmd error!\n", __FUNCTION__);
				return !RET_SUCCESS;
			}
			
			length = 0;
			while(1)
			{
				temp_length = smc_process_procedure_bytes(priv, command[INS_OFFSET], num_to_transfer - length, status);
				if(temp_length == 0)
				{
					if (NULL != response)
					{
						response[*actual_size] = status[0];
						response[(*actual_size)+1] = status[1];
						*actual_size += 2;
					}
					return RET_SUCCESS;
				}
				else if(temp_length < 0)
				{
					SMC_PRINTK("%s: procedure return error! CMD is:\n", __FUNCTION__);
					SMC_DUMP(command, num_to_write);
					return !RET_SUCCESS;
				}
				
				if(write_flag == 1)
				{
					if(smc_dev_write(priv, command + CMD_LENGTH + length, temp_length, &size) != RET_SUCCESS) 
					{
						SMC_PRINTK("%s: 2 write data error!\n", __FUNCTION__);
						return !RET_SUCCESS;
					}
					SMC_PRINTK("DATA: IFD -> SMC: ");
					SMC_DUMP(command + CMD_LENGTH + length, temp_length);
				}
				else
				{
					if(smc_dev_read(priv, response + length, temp_length, &size) != RET_SUCCESS)
					{
						SMC_PRINTK("%s: data read error!\n", __FUNCTION__);
						return !RET_SUCCESS;
					}
					*actual_size += temp_length;
					SMC_PRINTK("DATA: IFD <- SMC, ");
					SMC_DUMP(response + length, temp_length);
				}
				length += temp_length;
			}
		}
		else
		{
			/* INS is invalid */
			error = SMART_INVALID_CODE;
		}
	}
	else
	{
		/* CLA is invalid */
		error = SMART_INVALID_CLASS;
	}
	return error;
}
#if 0 
static INT32 smc_dev_iso_transfer_t1(struct smartcard_m36_private *priv, UINT8 *command, INT16 num_to_write, 
						UINT8 *response, INT16 num_to_read,INT32 *actual_size)
{
	//INT32 ret;
	INT32 error = RET_SUCCESS;
	//UINT8 write_to_smartcard = 0;
	INT16 size;
	//INT16 num_to_transfer;
	//INT16 length = 0;
	//INT16 temp_length = 0;

	struct smartcard_m36_private *tp = priv;

	if(tp->inserted == 0)
	{
		SMC_PRINTK("smc: smart card not inserted!\n");
		return !RET_SUCCESS;
	}
	else if(tp->reseted != 1)
	{
		SMC_PRINTK("smc: smart card not reseted!\n");
		return !RET_SUCCESS;
	}

//	osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);	
	mutex_lock(&priv->smc_mutex);

	if(smc_dev_transfer_data(priv, command, num_to_write, response, num_to_read, &size) != SUCCESS)
	{
//		osal_semaphore_release(tp->smc_sema_id);
		mutex_unlock(&priv->smc_mutex);
		return !RET_SUCCESS;
	}
	//SMC_DUMP(response,size);
	*actual_size = size;
//    	osal_semaphore_release(tp->smc_sema_id);
	mutex_unlock(&priv->smc_mutex);
	return error;
}
#endif
#if 1
/******************************************************************************************************
 * 	Name		:	smc_dev_ioctl()
 *	Description	:	Smart card reader IO control function.
 *	Parameter	:	struct smartcard_m36_private *priv		: Devcie handle
 *				INT32 cmd			: IO command
 *				UINT32 param			: Command parameter
 *	Return		:	INT32				: return value
 *
 ******************************************************************************************************/
static INT32 smc_dev_ioctl(struct smartcard_m36_private *priv, INT32 cmd, UINT32 param)
{
	INT32 ret_code = RET_SUCCESS;
	struct smartcard_m36_private *tp = priv;
	UINT32 ioaddr = tp->base_addr;


	SMC_PRINTK("[ %s %d ], cmd= %d, param = %d\n", __FUNCTION__, __LINE__, _IOC_NR(cmd), (unsigned int)param);

	switch (cmd)
	{
		case SMC_DRIVER_SET_IO_ONOFF:
			if(param == SMC_IO_ON)
			{
				smc_dev_enable();
			}
			else if(param == SMC_IO_OFF)
			{
				smc_dev_disable();
			}
			break;
		case SMC_DRIVER_CHECK_STATUS:
		{
			UINT8 smc_status;
			if(tp->inserted == 0)
				smc_status = SMC_STATUS_NOT_EXIST;
			else if(tp->reseted == 0)
				smc_status = SMC_STATUS_NOT_RESET;
			else
				smc_status = SMC_STATUS_OK;
			copy_to_user((UINT8*)param, &smc_status, 1);
			SMC_PRINTK("[ %s ]: status = %d\n", __FUNCTION__, smc_status);	
			break;
		}
		case SMC_DRIVER_SET_WWT:
			mutex_lock(&priv->smc_mutex);
			tp->first_cwt = param;
			mutex_unlock(&priv->smc_mutex);
			break;
		case SMC_DRIVER_SET_CWT:
			mutex_lock(&priv->smc_mutex);
			tp->cwt = param;
			mutex_unlock(&priv->smc_mutex);
			break;
		case SMC_DRIVER_SET_ETU:
			mutex_lock(&priv->smc_mutex);
			smc_set_etu(tp, param);
			mutex_unlock(&priv->smc_mutex);
			break;
		case SMC_DRIVER_SEND_PPS:
			mutex_lock(&priv->smc_mutex);
			ret_code = smc_set_pps(tp, param);
			mutex_unlock(&priv->smc_mutex);
			break;
		case SMC_DRIVER_GET_F:
			mutex_lock(&priv->smc_mutex);
			if(!tp->reseted)
				ret_code = !RET_SUCCESS;
			else
			{
				copy_to_user((UINT8*)param, &(tp->F), sizeof(UINT32));
				SMC_PRINTK("[ %s ]: F = %d\n", __FUNCTION__, tp->F);	
			}
			mutex_unlock(&priv->smc_mutex);
			break;
		case SMC_DRIVER_GET_D:
			mutex_lock(&priv->smc_mutex);
			if(!tp->reseted)
				ret_code = !RET_SUCCESS;
			else
			{
				copy_to_user((UINT8*)param, &(tp->D), sizeof(UINT32));
				SMC_PRINTK("[ %s ]: D = %d\n", __FUNCTION__, tp->D);	
			}
			mutex_unlock(&priv->smc_mutex);
			break;	
			
		case SMC_DRIVER_GET_ATR_RESULT:
			mutex_lock(&priv->smc_mutex);
			copy_to_user((UINT8*)param, &(tp->atr_rlt), sizeof(enum smc_atr_result));
			//  *((enum smc_atr_result *)param) = tp->atr_rlt;
			mutex_unlock(&priv->smc_mutex);
			break;

		case SMC_DRIVER_GET_ATR:
			mutex_lock(&priv->smc_mutex);
			copy_to_user((UINT8*)param, &(tp->atr), sizeof(tp->atr));			
			mutex_unlock(&priv->smc_mutex);
			break;
			
		case SMC_DRIVER_GET_PROTOCOL:
		{
			UINT32 T;
			mutex_lock(&priv->smc_mutex);
			if(tp->inserted&&tp->reseted)
			{
				T = tp->T;
			}
			else 
			{
				ret_code = !RET_SUCCESS;
				T = 0xffffffff;
			}
			copy_to_user((UINT8*)param, &T, sizeof(UINT32));
			SMC_PRINTK("[ %s ]: T = t%d\n", __FUNCTION__, T);	
			mutex_unlock(&priv->smc_mutex);
			break;
		}
		case SMC_DRIVER_GET_HB:
			mutex_lock(&priv->smc_mutex);
			{
				UINT8 len = tp->atr_info->hbn;
				//struct smc_hb_t * p = (struct smc_hb_t *)param;
				struct smc_hb_t p;
				if(len>(sizeof(struct smc_hb_t)-1))
					len = sizeof(struct smc_hb_t)-1;
				memcpy((void *)p.hb, tp->atr_info->hb, len);
				p.hbn = len;
				copy_to_user((UINT8*)param, &p, sizeof(struct smc_hb_t));
			}
			mutex_unlock(&priv->smc_mutex);
			break;
		
		case SMC_DRIVER_SET_WCLK:
			mutex_lock(&priv->smc_mutex);
			tp->init_clk_idx = 0;
			tp->init_clk_array[tp->init_clk_idx] = param;
			mutex_unlock(&priv->smc_mutex);
			break;

		case SMC_DRIVER_SET_OPEN_DRAIN:
			mutex_lock(&priv->smc_mutex);					
			tp->open_drain_supported = param & 0x1;			
			tp->open_drain_supported |= (param & 0x2);			
			tp->open_drain_supported |= (param & 0x4);			
			tp->open_drain_supported |= (param & 0x8);	

			tp->en_power_open_drain = param & 0x1;			
			tp->en_clk_open_drain = (param & 0x2) >> 1;			
			tp->en_data_open_drain = (param & 0x4) >> 2;			
			tp->en_rst_open_drain = (param & 0x8) >> 3;			
			
			if(tp->open_drain_supported)
			{		
				UINT8 temp_val =  INPUT_UINT8(ioaddr + REG_CLK_VPP);				
				temp_val &= 0x9f;			
				temp_val |= (tp->en_power_open_drain<<6);				
				temp_val |= ((tp->en_clk_open_drain|tp->en_data_open_drain|tp->en_rst_open_drain)<<5);
				OUTPUT_UINT8(ioaddr + REG_CLK_VPP, temp_val);					
			}
			mutex_unlock(&priv->smc_mutex);
			break;

		case SMC_DRIVER_SET_DEBUG_LEVEL:
		{		
			g_smc_debug = param;			
			SMC_PRINTK("[ %s ]: g_smc_debug = %d\n", __FUNCTION__, g_smc_debug);				
		}
		break;
			
		default:
			break;
	}

	return ret_code;
}
#endif


/******************************************************************************
 * driver registration
 ******************************************************************************/

static struct file_operations ali_m36_smartcard_fops = {
	.owner		= THIS_MODULE,
	// .write		= ali_m36_smartcard_write,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	.unlocked_ioctl = ali_m36_smartcard_ioctl,
#else
	.ioctl		= ali_m36_smartcard_ioctl,
#endif	
	.open		= ali_m36_smartcard_open,
	.release	=  ali_m36_smartcard_release,
	//.poll		= dvb_smartcard_poll,
};

struct ali_smartcard_device ali_m36_smartcard_dev;

struct class *ali_m36_smartcard_class;
struct device *ali_m36_smartcard_dev_node;
static UINT8 smc_iso_xfer_cmd[CA_MSG_MAX_LEN];

static INT32 ali_m36_smc_hw_initialize(struct smartcard_m36_private *priv, struct ali_smc_dev_config *smc_cfg)
{
	UINT32 ioaddr = priv->base_addr;
	UINT8 status;	
	INT32 ret;
	
	
	if(smc_cfg->init_clk_trigger&&
		smc_cfg->init_clk_number&&
		smc_cfg->init_clk_array)
	{
		UINT8 i;
		priv->init_clk_number = smc_cfg->init_clk_number;
		priv->init_clk_array = (UINT32 *)kmalloc(priv->init_clk_number*sizeof(UINT32), GFP_KERNEL);
		if(NULL==priv->init_clk_array)
		{
			ret = -ENOMEM;
			goto err1;
		}
		copy_from_user(priv->init_clk_array, smc_cfg->init_clk_array, (priv->init_clk_number*sizeof(UINT32)));
		SMC_PRINTK("\n%s, use user defined init clock array\n", __FUNCTION__);
		for(i=0; i<priv->init_clk_number; i++)
		{
			SMC_PRINTK("\t %d\n", priv->init_clk_array[i]);
		}
		SMC_PRINTK("\n");
		
	}
	else
	{
		priv->init_clk_number = 1;
		priv->init_clk_array = &(priv->smc_clock);
		priv->smc_clock = DFT_WORK_CLK;
	}
	if(smc_cfg->gpio_cd_trigger)
	{
		priv->use_gpio_cd = 1;
		priv->gpio_cd_io = smc_cfg->gpio_cd_io;
		priv->gpio_cd_pol = smc_cfg->gpio_cd_pol;
		priv->gpio_cd_pos = smc_cfg->gpio_cd_pos;
	}
	if(smc_cfg->gpio_vpp_trigger)
	{
		priv->use_gpio_vpp = 1;
		priv->internal_ctrl_vpp = 1;
		priv->gpio_vpp_io = smc_cfg->gpio_vpp_io;
		priv->gpio_vpp_pol = smc_cfg->gpio_vpp_pol;
		priv->gpio_vpp_pos = smc_cfg->gpio_vpp_pos;
	}
	if(smc_cfg->def_etu_trigger)
	{
		priv->use_default_etu = 1;
		priv->default_etu = smc_cfg->default_etu;
	}
	
	//check HW auto TX/RX
	priv->invert_power = smc_cfg->invert_power;
	priv->invert_detect = smc_cfg->invert_detect;
	priv->auto_tx_rx_triger = 1;
	priv->ts_auto_detect = 1;	
	priv->open_drain_supported = smc_cfg->en_power_open_drain;
	priv->open_drain_supported |= smc_cfg->en_clk_open_drain;
	priv->open_drain_supported |= smc_cfg->en_data_open_drain;
	priv->open_drain_supported |= smc_cfg->en_rst_open_drain;	
	if(priv->open_drain_supported)
	{
		priv->en_power_open_drain = smc_cfg->en_power_open_drain;
		priv->en_clk_open_drain = smc_cfg->en_clk_open_drain;
		priv->en_data_open_drain = smc_cfg->en_data_open_drain;
		priv->en_rst_open_drain = smc_cfg->en_rst_open_drain;
	}
	priv->parity_disable = smc_cfg->parity_disable_trigger;
	priv->parity_odd = smc_cfg->parity_odd_trigger;
	priv->apd_disable = smc_cfg->apd_disable_trigger;
	priv->warm_reset_enable = smc_cfg->warm_reset_trigger;
	priv->disable_pps = smc_cfg->disable_pps;


	priv->smc_rx_tmp_buf_addr = (UINT8 *)kmalloc(SMC_RX_BUF_SIZE, GFP_KERNEL);
	if(NULL==priv->smc_rx_tmp_buf_addr)
	{
		ret = -ENOMEM;
		goto err2;
	}
	priv->smc_rx_tmp_buf = (UINT8 *)((UINT32)(priv->smc_rx_tmp_buf_addr));

	priv->atr_info = (atr_t *)kmalloc(sizeof(atr_t), GFP_KERNEL);
	if(NULL==priv->atr_info)
	{
		SMC_ERR_PRINTK("[ %s ], Memory allocate failed.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto err3;
	}
	memset(priv->atr_info, 0, sizeof(atr_t));
	//when open the card, init the T1 parameter
	t1_init(&priv->T1);
	smc_dev_deactive(priv);
	smc_init_hw(priv);

	status = INPUT_UINT8(ioaddr + REG_ICCSR);

	SMC_PRINTK("to check card insert 0x80 : %x\n",status);
	if((status & 0x80) != 0)
	{
		SMC_PRINTK("card insert\n");
		priv->inserted = 1;
		//dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
		OUTPUT_UINT8(ioaddr + REG_ISR1, SMC_ISR1_CARD_INSERT);		
	}

	if(request_irq(smc_dev_set[0].irq, (irq_handler_t)smc_dev_interrupt, 0, \
		ALI_SMARTCARD_DEVICE_NAME, (void *)(&ali_m36_smartcard_dev))<0)
	{
		ret= -EACCES;
		SMC_PRINTK("%s register interrupt handler failed.\n", __FUNCTION__);
		goto err4;
	}

	return SUCCESS;

err4:
	kfree(priv->atr_info);
	priv->atr_info = NULL;
err3: 
	kfree((const void *)(priv->smc_rx_tmp_buf_addr));
	priv->smc_rx_tmp_buf_addr=NULL;
	priv->smc_rx_tmp_buf=NULL;
err2:
	kfree(priv->init_clk_array);
	priv->init_clk_array=NULL;
err1:
	return ret;


}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static int ali_m36_smartcard_ioctl(struct file *file,
			   unsigned int cmd, unsigned long parg)
#else
static int ali_m36_smartcard_ioctl(struct inode *inode, struct file *file,
			   unsigned int cmd, unsigned long parg)
#endif			   
{
	struct ali_smartcard_device *dev = file->private_data;
	struct smartcard_m36_private *tp=dev->priv;
	//unsigned long arg = (unsigned long) parg;
	//unsigned int flag;
	int ret = 0;


	SMC_PRINTK("[ %s %d ], cmd = %d\n", __FUNCTION__, __LINE__, _IOC_NR(cmd));
	
	switch (cmd) 
	{
	case CA_SMARTCARD_HW_INIT:
	{
		struct ali_smc_dev_config smc_param;
		
		copy_from_user(&smc_param, (void *)parg, sizeof(struct ali_smc_dev_config));
		ret = ali_m36_smc_hw_initialize(tp, &smc_param);
		break;
	}
	case CA_SMARTCARD_SET_SOCKPORT:
	{
		tp->hsr=(int)parg;
		break;
	}
	case CA_RESET:
	{
		ret=smc_dev_reset(tp, tp->atr, &(tp->atr_size));
		break;
	}
	case CA_GET_CAP:
		break;
	case CA_GET_SLOT_INFO:
	{
		ca_slot_info_t smc_slot;
        	//SMC_PRINTK("CA_GET_SLOT_INFO:<-%08x\n", (unsigned int)parg);
		copy_from_user(&smc_slot, (void *)parg, sizeof(ca_slot_info_t));
              //SMC_PRINTK("inserted %02x\n", tp->inserted);
		smc_slot.flags=tp->inserted;
		copy_to_user((void *)parg, &smc_slot, sizeof(ca_slot_info_t));
             // SMC_PRINTK("CA_GET_SLOT_INFO:->%08x\n", (unsigned int)parg);
		ret=SUCCESS;
		break;
	}
	case CA_SET_SLOT_INFO:
		break;
	case CA_GET_MSG:
	{
		copy_from_user(&ali_smc_msg, (void *)parg, 12);
		switch(ali_smc_msg.type)
		{
		case 0:  //ATR
			if(tp->atr_rlt==SMC_ATR_OK)
			{
				memcpy(ali_smc_msg.msg, tp->atr, tp->atr_size);
				ali_smc_msg.length=tp->atr_size;
				ret=SUCCESS;
			}
			else
			{
				ret = (-EIO);
				return ret;
			}
			break;
		case 1: //normal read
		{
			UINT16 actsize;
			ret=smc_dev_read(tp, ali_smc_msg.msg, ali_smc_msg.length, &actsize);
			if(SUCCESS==ret)
			{
				ali_smc_msg.length=actsize;
			}
			else
			{
				return ret;
			}
            break;
		}
		default:
			break;
		}
		copy_to_user((void *)parg, &ali_smc_msg, sizeof(ca_msg_t));
		break;
	}
	case CA_SEND_MSG:
	{
		copy_from_user(&ali_smc_msg, (void *)parg, sizeof(ca_msg_t));
		switch(ali_smc_msg.type)
		{
		case 1: //normal write
		{
			UINT16 actsize;
			ret=smc_dev_write(tp, ali_smc_msg.msg, ali_smc_msg.length, &actsize);
			if(SUCCESS==ret)
			{
				ali_smc_msg.length=actsize;
			}
			else
			{
				return ret;
			}
            break;
		}
		case 2: //T0 iso transfer.
		{
			UINT16 wlen, rlen, actsize;
			wlen=ali_smc_msg.length&0xffff;
			rlen=(ali_smc_msg.length>>16)&0xffff;
			memcpy(smc_iso_xfer_cmd, ali_smc_msg.msg, wlen);
			
			if(0==rlen)
			{
				return smc_dev_iso_transfer(tp, smc_iso_xfer_cmd, wlen, NULL, 0, &actsize);
			}
			else
			{
				ret=smc_dev_iso_transfer(tp, smc_iso_xfer_cmd, wlen, ali_smc_msg.msg, rlen, &actsize);
			}
			
			if(SUCCESS==ret)
			{
				ali_smc_msg.length=actsize;
			}
			else
			{
				return ret;
			}
			break;
		}
		default:
			break;
		}
		copy_to_user((void *)parg, &ali_smc_msg, sizeof(ca_msg_t));
		break;
	}
	case CA_SMARTCARD_DEACTIVE:
		ret=smc_dev_deactive(tp);
		break;
	case CA_SMARTCARD_IO_COMMAND:
	{
		struct ali_smc_ioctl_command io_param;
		copy_from_user(&io_param, (void *)parg, sizeof(struct ali_smc_ioctl_command));
		ret = smc_dev_ioctl(tp, io_param.ioctl_cmd, io_param.param);
		break;
	}
	default:
		ret=-ENOIOCTLCMD;
		break;
	}
	return ret;
}

static int ali_m36_smartcard_open(struct inode *inode, struct file *file)
{			
	struct smartcard_m36_private *priv;
	

	file->private_data=(void*)&ali_m36_smartcard_dev;
	
	memset(&ali_m36_smartcard_private, 0, sizeof(struct smartcard_m36_private));	
	smc_dev_set[0].priv = &ali_m36_smartcard_private;
	memset(&ali_m36_smartcard_private, 0, sizeof(struct smartcard_m36_private));	
	priv = &ali_m36_smartcard_private;
	priv->base_addr = smc_dev_set[0].io_base;
	priv->inserted = 0;
	priv->reseted = 0;
	priv->inverse_convention = 0;
	priv->the_last_send = 0;
	priv->init_clk_number = 1;
	priv->init_clk_array = &(priv->smc_clock);
	priv->smc_clock = DFT_WORK_CLK;	
	mutex_init(&(ali_m36_smartcard_private.smc_mutex));	
	ali_m36_smartcard_dev.priv=&ali_m36_smartcard_private;
	
	INIT_WORK(&g_smc_wq, (void *)smc_wq_handler);

	return 0;
}

static int ali_m36_smartcard_release(struct inode *inode, struct file *file)
{
	struct ali_smartcard_device *dev = file->private_data;
	struct smartcard_m36_private *tp = dev->priv;
	UINT32 ioaddr = tp->base_addr;
	
	
	tp->inserted = 0;
	tp->reseted = 0;

	/* Disable interrupt */
	OUTPUT_UINT8(ioaddr + REG_IER0, 0x00);
	OUTPUT_UINT8(ioaddr + REG_IER1, 0x00);
	//osal_interrupt_unregister_lsr(dev->irq+ 8, smc_dev_interrupt);
	free_irq(smc_dev_set[0].irq, (void *)(&ali_m36_smartcard_dev));
	smc_dev_deactive(tp);
	
	mutex_destroy(&(tp->smc_mutex));
	if(NULL!=tp->init_clk_array)
	{
		kfree(tp->init_clk_array);
		tp->init_clk_array=NULL;
		(&ali_m36_smartcard_private)->init_clk_array = NULL;
	}
	if(NULL!=tp->smc_rx_tmp_buf_addr)
	{
		kfree((const void *)(tp->smc_rx_tmp_buf_addr));
		tp->smc_rx_tmp_buf=NULL;
		(&ali_m36_smartcard_private)->smc_rx_tmp_buf = NULL;
		tp->smc_rx_tmp_buf_addr=NULL;
		(&ali_m36_smartcard_private)->smc_rx_tmp_buf_addr = NULL;
	}
	if(NULL!=tp->atr_info)
	{
		kfree(tp->atr_info);
		tp->atr_info=NULL;
		(&ali_m36_smartcard_private)->atr_info = NULL;
	}	
	dev->priv=NULL;
	
	return 0;
}


static int __devinit ali_m36_smartcard_dev_init(void)
{
	int ret;
	dev_t devno;	


	scr_sys_clk = 108000000;
	pwm_sys_clk = scr_sys_clk;
	
	ret=alloc_chrdev_region(&devno, 0, 1, ALI_SMARTCARD_DEVICE_NAME);
	if(ret<0)
	{
		SMC_ERR_PRINTK("Alloc device region failed, err: %d.\n",ret);
		goto err0;
	}

	cdev_init(&ali_m36_smartcard_dev.cdev, &ali_m36_smartcard_fops);
	ali_m36_smartcard_dev.cdev.owner=THIS_MODULE;
	ali_m36_smartcard_dev.cdev.ops=&ali_m36_smartcard_fops;
	ali_m36_smartcard_dev.priv=&ali_m36_smartcard_private;
	ret=cdev_add(&ali_m36_smartcard_dev.cdev, devno, 1);
	if(ret)
	{
		SMC_ERR_PRINTK("Alloc SmartCard controller device failed, err: %d.\n", ret);
		goto err0;
	}
	
	SMC_PRINTK("register SmartCard controller device end.\n");


	ali_m36_smartcard_class = class_create(THIS_MODULE, "ali_m36_smartcard_class");

	if (IS_ERR(ali_m36_smartcard_class))
	{
		ret = PTR_ERR(ali_m36_smartcard_class);

		goto err1;
	}

	ali_m36_smartcard_dev_node = device_create(ali_m36_smartcard_class, NULL, devno, &ali_m36_smartcard_dev, 
                           ALI_SMARTCARD_DEVICE_NAME);
         if (IS_ERR(ali_m36_smartcard_dev_node))
    {
		printk(KERN_ERR "device_create() failed!\n");

		ret = PTR_ERR(ali_m36_smartcard_dev_node);

		goto err2;
	}

	//mutex_init(&(ali_m36_smartcard_private.smc_mutex));

	SMC_PRINTK("Ali SmartCard controller device register success.\n");

	return ret;

err2:
	class_destroy(ali_m36_smartcard_class);
err1:
	cdev_del(&ali_m36_smartcard_dev.cdev);
err0:


	SMC_ERR_PRINTK("Ali SmartCard controller device register Failed!\n");
	return ret;
}


static void __exit ali_m36_smartcard_dev_exit(void)
{
	printk("%s\n", __FUNCTION__);
	if(ali_m36_smartcard_dev_node != NULL)
	{
		device_del(ali_m36_smartcard_dev_node);
	}

	if(ali_m36_smartcard_class != NULL)
	{
		class_destroy(ali_m36_smartcard_class);
	}
	cdev_del(&ali_m36_smartcard_dev.cdev);	
}

module_init(ali_m36_smartcard_dev_init);
module_exit(ali_m36_smartcard_dev_exit);

MODULE_DESCRIPTION("driver for the Ali M36xx SmartCard controller device");
MODULE_AUTHOR("ALi Corp ShangHai SDK Team, Eric Li");
MODULE_LICENSE("GPL");



