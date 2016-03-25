/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    i2c_scb.s
*
*    Description:   SCB type of i2c driver for i2c controlled Panel chip CH455.
*		
*
*    History:
*	Date		Athor		Version		Reason
*	=======================================================
*     1.	06.09.2008	Mao Feng		Ver 0.1		Create file.
*     2.    07.14.2008					Add check, delay, retry, timeout func.
******************************************************************************/
//#include <sys_config.h>
//#include <hal/mips.h>
//#include <hal/machine.h>

#include "mips.h"

#define I2C_SCB_BASE_0			0xb8018200	
#define I2C_SCB_BASE_1			0xb8018700

#define SCB_HCR		0x00		
#define SCB_HSR		0x01		
#define SCB_IER		0x02		
#define SCB_ISR		0x03		
#define SCB_SAR		0x04		
#define SCB_SSAR		0x05		

#define SCB_HPCC		0x06		
#define SCB_LPCC		0x07		
#define SCB_PSUR		0x08		
#define SCB_PHDR		0x09		
#define SCB_RSUR		0x0A				
#define SCB_SHDR		0x0B		

#define SCB_FCR		0x0C		
#define SCB_FDR		0x10		
#define SCB_DDC_ADDR	0x0E
#define SCB_SEG_PTR	0x0F

/* SCB_HCR bit define */
#define SCB_HCE		0x80
#define SCB_DNEE		0x40
#define SCB_CP_WT	0x00
#define SCB_CP_CAR	0x04
#define SCB_CP_SER	0x08
#define SCB_CP_STR	0x0c
#define SCB_ST		0x01
#define SCB_EDDC		0x20

/* SCB_HSR bit define */
#define SCB_DB		0x80
#define SCB_DNE		0x40
#define SCB_HB		0x20
#define SCB_FER		0x10
#define SCB_FO		0x08
#define SCB_FU		0x04
#define SCB_FF		0x02
#define SCB_FE		0x01

/* SCB_FCR bit define */
#define SCB_FLUSH	0x80
#define SCB_BC		0x1f

#define SCB_TIMING	21


/***************************************************************
*Function Name:	i2c_scb_init_s
*Arguments: 	a0	0->SCB1, 1->SCB2
*Description:	Using SCB_TIMING as Timing to init SCB type of I2C 
*Notice:		should use t0, t3 in sub-function.
*
***************************************************************/
	.globl	i2c_scb_init_s
	.ent	i2c_scb_init_s
	.set	noreorder
i2c_scb_init_s:
	beqz	a0, _scb0
	nop

_scb1:
	li	t0, I2C_SCB_BASE_1
	b	_scb_mode_set
	nop
_scb0:
	li	t0, I2C_SCB_BASE_0

_scb_mode_set:
	lbu	t1, SCB_IER(t0)		# disable interrupts
	andi	t1, 0xf0
	sb	t1, SCB_IER(t0)

	li	t1, 0x0f			# clear interrupts
	sb	t1, SCB_ISR(t0)

	li	t1, SCB_TIMING		# set timings
	sb	t1, SCB_HPCC(t0)
	sb	t1, SCB_LPCC(t0)	
	sb	t1, SCB_PSUR(t0)
	sb	t1, SCB_PHDR(t0)
	sb	t1, SCB_RSUR(t0)
	sb	t1, SCB_SHDR(t0)

	li	t1, 0x80			# clear fifo
	sb	t1, SCB_FCR(t0)

	li	t1, 0x80			# enable host controller
	sb	t1, SCB_HCR(t0)	

	jr	ra
	nop
	
	.set	reorder
	.end	i2c_scb_init_s



#define	_SCB_TIMES_OUT	1000
#define	_SCB_DELAY	100
#define	_SCB_RETRY	5
#define	_SCB_TX_DONE	100
/***************************************************************
*Function Name:	_scb_wait_host_not_bsy
*Arguments: 	a0	0->SCB1, 1->SCB2
*Return:		v0	0->ok, 1->error
*Description:	wait untill host is not busy.
*Notice:		should not use t0, t3, a1, a2 in this function,
*		using t1, t2, t4, t5.
*		Do not change _SCB_TIMES_OUT and _SCB_DELAY untill 
*		U know the SCB timing and SB clock.
***************************************************************/
	.globl	_scb_wait_host_not_bsy
	.ent	_scb_wait_host_not_bsy
	.set	noreorder
_scb_wait_host_not_bsy:
	
	j	3f
	li	t4, 0
	
_delay:	
	add	t5, 1
	blt	t5, _SCB_DELAY, _delay
	nop
	
	add	t4, 1
	bgt	t4, _SCB_TIMES_OUT, 4f
	li	v0, 1
3:
	lbu	t1, SCB_HSR(t0)
	li	t2, SCB_HB
	and	t1, t2, t1
	bne	t1, zero, _delay
	li	t5, 1

	li	v0, 0
4:	
	j	ra
	nop
	.set	reorder
	.end	_scb_wait_host_not_bsy


/***************************************************************
*Function Name:	_scb_wait_device_ready
*Arguments: 	a0	0->SCB1, 1->SCB2
*Return:		v0	0->ok, 2->error
*Description:	wait untill device is ready
*Notice:		should not use t0, t3, a1, a2 in this function,
*		using t1, t2, t4, t5.
*		Do not change _SCB_TIMES_OUT and _SCB_DELAY untill 
*		U know the SCB timing and SB clock.
***************************************************************/
	.globl	_scb_wait_device_ready
	.ent	_scb_wait_device_ready
	.set	noreorder
_scb_wait_device_ready:
	j	3f
	li	t4, 0

_delay_rdy:	
	add	t5, 1
	blt	t5, _SCB_DELAY, _delay_rdy
	nop
	
	add	t4, 1
	bgt	t4, _SCB_TIMES_OUT, 4f
	li	v0, 2
3:
	lbu	t1, SCB_HSR(t0)
	li	t2, SCB_HB
	andi	t2, SCB_DB
	andi	t2, SCB_DNE
	
	and	t1, t2, t1
	bne	t1, zero, _delay_rdy
	li	t5, 1
	
	li	v0, 0	
4:
	j	ra
	nop
	.set	reorder
	.end	_scb_wait_device_ready


/***************************************************************
*Function Name:	_scb_wait_fifo_ready
*Arguments: 	a0	0->SCB1, 1->SCB2
*		a1          len
*Return:		v0	0->ok, 3->error
*Notice:		should not use t0, t3, a1, a2 in this function,
*		using t1, t2, t4, t5.
*		Do not change _SCB_TIMES_OUT and _SCB_DELAY untill 
*		U know the SCB timing and SB clock.
*Description:	wait untill fifo is ready
*
***************************************************************/
	.globl	_scb_wait_fifo_ready
	.ent	_scb_wait_fifo_ready
	.set	noreorder
_scb_wait_fifo_ready:

	li	t1, 16		# len<=16
	bgtu	a1, t1, 1f
	nop
	b	2f
	nop
	
1:
	li	a1, 16
2:
	j	3f
	li	t4, 0

_delay_fifo:	
	add	t5, 1
	blt	t5, _SCB_DELAY, _delay_fifo
	nop
	
	add	t4, 1
	bgt	t4, _SCB_TIMES_OUT, 4f
	li	v0, 3
3:
	lbu	t1, SCB_FCR(t0)
	li	t2, SCB_BC
	and	t1, t2, t1
	
	bne	t1, a1, _delay_fifo
	li	t5, 1
	
	li	v0, 0	
4:
	j	ra
	nop
	.set	reorder
	.end	_scb_wait_fifo_ready


/***************************************************************
*Function Name:	_scb_wait_tx_done
*Arguments: 	a0	0->SCB1, 1->SCB2
*Return:		v0	0->ok, 4->timeout, 5->err
*Description:	wait untill transfer is done.
*Notice:		should not use t0, t3, a1, a2 in this function,
*		using t1, t2, t4, t5.
*		Do not change _SCB_TIMES_OUT and _SCB_DELAY untill 
*		U know the SCB timing and SB clock.
***************************************************************/
	.globl	_scb_wait_tx_done
	.ent	_scb_wait_tx_done
	.set	noreorder
_scb_wait_tx_done:
	
	j	3f
	li	t4, 0
	li	t5, 1
_delay_tx:	
	add	t5, 1
	blt	t5, _SCB_DELAY, _delay_tx
	nop
	
	add	t4, 1
	bgt	t4, _SCB_TX_DONE, 4f
	li	v0, 4
3:
	lbu	t1, SCB_ISR(t0)
	li	t2, 1
	and	t1, t2, t1
	beqz	t1, _delay_tx
	li	t5, 1

	li	v0, 0
4:	
	lbu	t1, SCB_ISR(t0)
	li	t2, 0x0e
	and	t1, t2, t1
	bnez	t1, _err_tx
	nop
	
	j	ra
	li	v0, 0
_err_tx:
	j	ra
	li	v0, 5
	.set	reorder
	.end	_scb_wait_host_not_bsy


/***************************************************************
*Function Name:	i2c_scb_read_s
*Arguments: 	a0	0->SCB1, 1->SCB2
*		a1	slave address
*		v0	1->host bsy, 2->dev not rdy, 3->fifo not rdy
*			data in byte->ok
*Description:	read a byte per time
*		store return value in t3, retry count in t6
*		so should not use t3 and t6 in this func or its sub-funcs.				
*
***************************************************************/
	.globl	i2c_scb_read_s
	.ent	i2c_scb_read_s
	.set	noreorder
i2c_scb_read_s:
	move	t3, ra			## store ra in t3		
	
	beqz	a0, 2f
	nop

1:
	li	t0, I2C_SCB_BASE_1
	b	3f
	nop
2:
	li	t0, I2C_SCB_BASE_0
3:
	jal 	_scb_wait_host_not_bsy	# 0->ok, 1->host bsy
	nop

	bnez	v0, 5f
	nop

	li	t6, 0
4:
	li	t1, _SCB_RETRY
	bgt	t6, t1, 5f
	add	t6, 1
	nop
	
	lbu	t1, SCB_HCR(t0)		# clear HCR bit6:0
	andi	t1, 0x80
	sb	t1, SCB_HCR(t0)

	li	t1, 0x00			# clear HSR bits
	sb	t1, SCB_HSR(t0)

	lbu	t1, SCB_IER(t0)		# disable interrupts
	andi	t1, 0xf0
	sb	t1, SCB_IER(t0)	

	li	t1, 0x0f			# clear interrupts
	sb	t1, SCB_ISR(t0)

	sb	a1, SCB_SAR(t0)		# set slave address

	li	t1, 1			# read a byte per time
	andi	t1, SCB_BC		# Byte Count
	ori	t1, SCB_FLUSH		# FIFO Flush
	sb	t1, SCB_FCR(t0)

	lbu	t1, SCB_HCR(t0)
	andi	t1, 0x80
	ori	t1, SCB_DNEE		# Device Not Exist Enable
	ori	t1, SCB_CP_CAR		# Current Address Read
	ori	t1, SCB_ST		# Start Transaction
	sb	t1, SCB_HCR(t0)	

	jal 	_scb_wait_device_ready	# 0->ok, 2->dev not rdy
	nop
	li	t1, 2
	beq	v0, t1, 4b
	nop

	li	a1, 1
	jal 	_scb_wait_fifo_ready	# 0->ok, 3->fifo not rdy
	nop
	li	t1, 3
	beq	v0, t1, 4b
	nop

	lb	v0, SCB_FDR(t0)
5:
	move	ra, t3			## fetch ra from t3
	j	ra
	nop
	.set	reorder
	.end	i2c_scb_read_s


/***************************************************************
*Function Name:	i2c_scb_write_s
*Arguments: 	a0	0->SCB1, 1->SCB2
*		a1	slave address
*		a2	data
*Return:		v0	0->ok, 1->host bsy err, 2->dev not rdy err, 4->tx timeout, 5->tx err
*Description:	write a byte per time
*		store return value in t3, retry count in t6
*		so should not use t3 and t6 in this func or its sub-funcs.					
*
***************************************************************/
	.globl	i2c_scb_write_s
	.ent	i2c_scb_write_s
	.set	noreorder
i2c_scb_write_s:
	move	t3, ra			## store ra in t3	

	beqz	a0, 2f
	nop	
1:
	li	t0, I2C_SCB_BASE_1
	b	3f
	nop
2:
	li	t0, I2C_SCB_BASE_0
3:
	jal 	_scb_wait_host_not_bsy	# 0->ok, 1->host bsy
	nop

	bnez	v0, 5f
	nop

	li	t6, 0
4:
	li	t1, _SCB_RETRY
	bgt	t6, t1, 5f
	add	t6, 1
	nop
	
	lbu	t1, SCB_HCR(t0)		# clear HCR bit6:0
	andi	t1, 0x80
	sb	t1, SCB_HCR(t0)

	li	t1, 0x00			# clear HSR bits
	sb	t1, SCB_HSR(t0)

	lbu	t1, SCB_IER(t0)		# disable interrupts
	andi	t1, 0xf0
	sb	t1, SCB_IER(t0)	

	li	t1, 0x0f			# clear interrupts
	sb	t1, SCB_ISR(t0)

	sb	a1, SCB_SAR(t0)		# set slave address
	sb	a2, SCB_SSAR(t0)		# set sub slave address 

	li	t1, 1			# write a byte per time
	andi	t1, SCB_BC		# Byte Count
	ori	t1, SCB_FLUSH		# FIFO Flush
	sb	t1, SCB_FCR(t0)

	sb	a2, SCB_FDR(t0)

	lbu	t1, SCB_HCR(t0)
	andi	t1, 0x80
	ori	t1, SCB_DNEE		# Device Not Exist Enable
	ori	t1, SCB_CP_WT		# Write
	ori	t1, SCB_ST		# Start Transaction
	sb	t1, SCB_HCR(t0)	

	jal 	_scb_wait_device_ready	# 0->ok, 2->dev not rdy
	nop

	li	t1, 2
	beq	v0, t1, 4b
	nop

	jal 	_scb_wait_tx_done		# 0->ok, 4->timeout, 5->err
	nop
	
	li	t1, 5
	beq	v0, t1, 4b
	nop
	
5:
	move	ra, t3			## fetch ra from t3
	j	ra
	nop
	.set	reorder
	.end	i2c_scb_write_s

	