/**
* Copyright (c) 2011,Ali Corp.
* All rights reserved.
*
* FileName     : ir.S
* Verison      : 1.0
* Author       : Zhao Owen
* Date         : 2011-07-28
* Description  : The file is to make standby to support ddr self refresh.
*/

#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>
#include <asm/cacheops.h>
#include "mips.h"
#include "m36_sleep.h"

/* IR decoder related macro */
#define IR_TYPE_NUM             5
#define IR_PATTERN_START        (IR_RLC_BUFFER_START + IR_RLC_BUFFER_SIZE)
#define IR_PATTERN_SIZE         240
#define IR_PATTERN_CNT_START    (IR_PATTERN_START + IR_PATTERN_SIZE)
#define IR_PATTERN_CNT_SIZE     16
#define IR_ATTR_START           (IR_PATTERN_CNT_START + IR_PATTERN_CNT_SIZE)
#define IR_ATTR_SIZE            64
#define IR_DECODER_START        (IR_ATTR_START + (IR_ATTR_SIZE * IR_TYPE_NUM))
#define IR_DECODER_SIZE         40
#define IR_PLS_SUM              (IR_DECODER_START + (IR_DECODER_SIZE * IR_TYPE_NUM))
#define IR_RC6_FLG              (IR_PLS_SUM + 4)
#define IR_CACHE_END            (IR_RC6_FLG + 4)

/* IR bit type define */
#define IR_LEADING              0
#define IR_SPECIAL              1
#define IR_DATA                 2
#define IR_REPEAT_LEADING       3
#define IR_REPEAT_DATA          4
#define IR_STOP                 5
#define IR_END                  6
#define IR_NULL                 0xff

/* bit pattern */
IR_ROM_PATTERN:
# NEC
    .byte \
        IR_LEADING, IR_DATA:32, IR_END
# LAB
    .byte \
        IR_LEADING, IR_SPECIAL, IR_DATA:9, IR_END
# LOGIC
    .byte \
        IR_LEADING, IR_DATA:12, IR_END
# KF
    .byte \
        IR_LEADING, IR_DATA:48, IR_END
# JZ
    .byte \
        IR_LEADING, IR_DATA:24, IR_LEADING, IR_DATA:16, IR_END
IR_ROM_PATTERN_END:

/* pattern count */
IR_ROM_PATTERN_CNT_LIST:
# NEC: 34, LAB: 12, LOGIC: 14, KF: 50, JZ: 43
    .byte \
        0, 34, 46, 60, 110

/* 
 * pulse attribute 
 */
#define BIT_MSB_FIRST           (1 << 0)
#define BYTE_MSB_FIRST          (1 << 1)
#define PULSE_INVERT            (1 << 2)
#define REPEAT_ENABLE           (1 << 3)

#define PULSE_TYPE              0
#define PULSE_LOW               2
#define PULSE_HIGH              4
#define PULSE_TORL              6

#define PULSE_PREC              2
#define PULSE_MAX_WIDTH         4
#define PULSE_LEADING           8
#define PULSE_SPECIAL           16
#define PULSE_DATA0             24
#define PULSE_DATA1             32
#define PULSE_R_LEADING         40
#define PULSE_R_DATA            48
#define PULSE_STOP              56

IR_ROM_ATTR:
# NEC
    .half \
        (BIT_MSB_FIRST | REPEAT_ENABLE), 4, 210, 0, \
        IR_LEADING, 123, 61, 15, \
        IR_NULL, 0, 0, 0, \
        IR_DATA, 8, 8, 4, \
        IR_DATA, 8, 23, 7, \
        IR_REPEAT_LEADING, 123, 31, 15, \
        IR_REPEAT_DATA, 8, 280, 7, \
        IR_STOP, 7, 280, 14
# LAB
    .half \
        BIT_MSB_FIRST, 2, 210, 0, \
        IR_LEADING, 8, 99, 8, \
        IR_SPECIAL, 4, 83, 20, \
        IR_DATA, 4, 65, 10, \
        IR_DATA, 4, 99, 10, \
        IR_NULL, 0, 0, 0, \
        IR_NULL, 0, 0, 0, \
        IR_STOP, 7, 280, 14
# LOGIC
    .half \
        0, 2, 210, 0, \
        IR_LEADING, 70, 70, 10, \
        IR_NULL, 0, 0, 0, \
        IR_DATA, 4, 7, 4, \
        IR_DATA, 7, 14, 4, \
        IR_NULL, 0, 0, 0, \
        IR_NULL, 0, 0, 0, \
        IR_STOP, 7, 280, 14
# KF
    .half \
        BYTE_MSB_FIRST, 2, 280, 0, \
        IR_LEADING, 51, 25, 7, \
        IR_NULL, 0, 0, 0, \
        IR_DATA, 5, 5, 4, \
        IR_DATA, 5, 19, 4, \
        IR_NULL, 0, 0, 0, \
        IR_NULL, 0, 0, 0, \
        IR_STOP, 7, 280, 14
# JZ
    .half \
        (BIT_MSB_FIRST | REPEAT_ENABLE), 2, 840, 0, \
        IR_LEADING, 51, 25, 7, \
        IR_NULL, 0, 0, 0, \
        IR_DATA, 8, 8, 4, \
        IR_DATA, 8, 16, 6, \
        IR_REPEAT_LEADING, 51, 51, 7, \
        IR_REPEAT_DATA, 8, 280, 14, \
        IR_STOP, 7, 280, 14
IR_ROM_ATTR_END:

#define FIRST_HALF_GOT          0
#define DECODE_STEP             1
#define KEY_BIT_CNT             2
#define LAST_PULSE_WIDTH        4
#define KEY_CODE                8
#define LAST_KEY_CODE           24




	.text
	.set    noreorder
	.align  4


/*
fucntion name: init_ir()
used register: t0,t1,t2,t3,t4
global used  : s2,s3
This function is initialize the ir for the ir code run in the cache.
*/
LEAF(init_ir)
	/* init ir related system registers */
	li      t0, SYS_IC_SB_BASE_H
	sb      zero, SYS_IC_SB_IRC_CFG(t0)     # Clear CR

	li      t1, 0x81                        # Enable IRC : 0x84/0x9c
	sb      t1, SYS_IC_SB_IRC_CFG(t0)
	li      t1, 0xa0                        # Init FIFO, threshould is 32 bytes
	sb      t1, SYS_IC_SB_IRC_FCTRL(t0)

	li      t1, 0x05                        # Timeout: 6*128*CLKwork
	sb      t1, SYS_IC_SB_IRC_TTHR(t0)
	li      t1, 0x01                        # Noise filer: 1*CLKwork
	sb      t1, SYS_IC_SB_IRC_NTHR(t0)

	li      t1, 0x03                        # Enable all interrupt
	sb      t1, SYS_IC_SB_IRC_IER(t0)
	li      t1, 0x03                        # Clear all interrupt
	sb      t1, SYS_IC_SB_IRC_ISR(t0)

	/* clear d-cache buffer for ir decoder */
	li      t0, IR_RLC_BUFFER_START
	li      t1, IR_CACHE_END
1:
	sb      zero, (t0)
	sltu    t4, t0, t1
	bnez    t4, 1b
	addiu   t0, 1

	/* copy ir pattern to d-cache */
	la      t0, IR_ROM_PATTERN
	la      t3, IR_ROM_PATTERN_END
	li      t1, IR_PATTERN_START
1:
	lbu     t2, (t0)
	sb      t2, (t1)
	addiu   t0, 1
	addiu   t1, 1
	sltu    t4, t0, t3
	bnez    t4, 1b
	nop

	/* copy ir pattern count to d-cache */
	la      t0, IR_ROM_PATTERN_CNT_LIST
	li      t3, IR_TYPE_NUM
	addu    t3, t0
	li      t1, IR_PATTERN_CNT_START
1:
	lbu     t2, (t0)
	sb      t2, (t1)
	addiu   t0, 1
	addiu   t1, 1
	sltu    t4, t0, t3
	bnez    t4, 1b
	nop

	/* copy ir attribute to d-cache */
	la      t0, IR_ROM_ATTR
	la      t3, IR_ROM_ATTR_END
	li      t1, IR_ATTR_START
1:
	lbu     t2, (t0)
	sb      t2, (t1)
	addiu   t0, 1
	addiu   t1, 1
	sltu    t4, t0, t3
	bnez    t4, 1b
	nop

	/* init ir module global used registers */
	li      s2, 0               # RLC data buffer write pointer
	li      s3, 0               # RLC data buffer read pointer

	jr      ra
	nop
END(init_ir)

/*
fucntion name   : get_ir()
v0 return key code
used register   : t0,t1,t2,t3,t4,t5,t6,t7,t8,t9
global used     : s0,s1,s2,s3,v0,v1
subroutine used : a0,a1,a2,a3

s2: RLC data in cache write pointer
s3: RLC data in cache read pointer
s1: save decoder module such as RC6 / NEC etc. 
s0 : save ra value when call noleaf function
This function is to get the code from ir.
*/

LEAF(get_ir) 
	/* init regs needed */ 
	li      t0, 0
	li      t1, 0
	li      t2, 0
	li      t3, 0
	li      t4, 0
	li      t5, 0
	li      t6, 0
	li      t7, 0
	li      t8, 0
	li      t9, 0
	nop

	move    v1, ra                      # Save ra to v1

	li      t2, IR_RLC_BUFFER_WTMRK
	addu    t2, s3                      # t2 is the write mark

	li      t0, SYS_IC_SB_BASE_H
	lbu     t5, SYS_IC_SB_IRC_ISR(t0)   # Store current interrupt status to t5
	sb      t5, SYS_IC_SB_IRC_ISR(t0)   # Clear interrupt

fetch_fifo:
	lbu     t1, SYS_IC_SB_IRC_FCTRL(t0) # Get IR RLC data number in FIFO 
	and     t1, 0x7f                    # t1 is the fifo data counter
	bnez    t1, 1f                      # If no IR RLC data, check timeout
	nop
	and     t5, 0x02                    # Check timeout?
	bnez    t5, 1f
	nop

	li      v0, 0                       # no timeout & fifo is empty, return 0
	move    ra, v1
	jr      ra
	nop
1:
	li      t3, IR_RLC_BUFFER_START     # if fifo is not empty
	addu    t3, s2
	lbu     t4, SYS_IC_SB_IRC_DATA(t0)  # Read data in FIFO & save to RAM (cache)
	sb      t4, (t3)
	sb	t4,0xb8018300	// print ir hex code
	
	li	v0,0x7f
	bne	 t4,v0,11f
	nop
	addiu a0,-1
	sb	a0,0xb8018300	// print 0x7F count
	bnez	 a0,12f
	nop
	li      v0, 0x7f                      # panel power key press, return 0x7f
	move    ra, v1
	jr      ra
	nop
11:	
	li	a0,0x05
12:
	addiu   s2, 1
	addiu   t1, -1

	li      t4, IR_RLC_BUFFER_SIZE
	sltu    k1, s2, t4
	bnez    k1, 2f    
	nop
	addiu   s2, -IR_RLC_BUFFER_SIZE     # if write pointer > buffer size 
2:                                      # if write pointer < buffer size
	beqz    t1, fetch_fifo              # if t1 is empty, goto fetch_fifo
	nop

	move    t4, s2                      # Check whether reach watermark?
	slt     k1, s3, t4
	bnez    k1, 3f      
	nop
	addiu   t4, IR_RLC_BUFFER_SIZE
3:
	slt     k1, t2, t4
	bnez    k1, accum_pulse         # if write pointer < watermark, wait
	nop

	bnez    t1, 1b
	nop

	b       fetch_fifo                  # Re-check RLC fifo
	nop

accum_pulse: 
	li      v0 , 0 
	bne     s3, s2, 1f
	nop

	li      v0, 0                       # if s2 == s3, return 0
	move    ra, v1
	jr      ra
	nop

1:
	li      t1, IR_RLC_BUFFER_START
	addu    t1, s3
	lbu     t2, (t1)                    # t2 is the data

	addiu   s3, 1
	li      t1, IR_RLC_BUFFER_SIZE
	slt     k1, s3, t1
	bnez    k1, 1f
	nop
	addiu   s3, -IR_RLC_BUFFER_SIZE     

1:
	and     t3, t2, 0x7F           # clear FIFO bit 7 save to t3 , so t3 is the pulse width!
	li      t1, IR_PLS_SUM   # Load pulse width sum from cache
	lh      t4, (t1)
	and     t1, t4, 0xFFFF
	addu    t1, t3
	li      t5, 0x10000                 # max pulse width is 0x10000 , save in t5
	slt     k1, t1, t5
	bnez    k1, 1f    
	nop
#sdbbp                               # Only support no more than 2^16 RLC sample
1:
	addu    t4, t3
	li      t1, IR_PLS_SUM              # save pulse width sum to d-cache
	sh      t4, (t1)                    # t4 is the pulse width sum

	beq     s2, s3, start_decode
	nop

	and     t2, 0x80
	li      t1, IR_RLC_BUFFER_START
	addu    t1, s3
	lbu     t3, (t1)
	and     t3, 0x80
	xor     t2, t3                      # Check current/next RLC polarity same?
	beqz    t2, accum_pulse             # if it is the same, goto start_decode
	nop

start_decode:
	li      t0, IR_PATTERN_START
	li      t1, IR_PATTERN_CNT_START
	addu    t1, t9                      # t9 is the pattern type: NEC
	lbu     t2, (t1)
	addu    t6, t0, t2                  # t6 is the current pattern

	li      t1, IR_ATTR_START
	li      t2, IR_ATTR_SIZE
	multu   t9, t2
	mflo    t2
	addu    t7, t1, t2                  # t7 is the current attribute

	li      t2, IR_DECODER_START
	li      t3, IR_DECODER_SIZE
	multu   t9, t3
	mflo    t3
	addu    t8, t2, t3                  # t8 is the currnt decoder

	lbu     t1, DECODE_STEP(t8)
	addu    t1, t6
	lbu     t5, (t1)                    # t5 is the decoder step

	lbu     t0, FIRST_HALF_GOT(t8)
	bnez    t0, 2f
	nop

	li      t0, IR_LEADING              # For leading pulse should check first half
	bne     t0, t5, 1f                  # if first half is 0, check the step
	nop

	and     a0, t4, 0xFFFF              # if in leading step

	addiu   t0, t7, PULSE_LEADING       # handle the pulse leading
	lhu     a1, PULSE_LOW(t0)
	lhu     a2, PULSE_TORL(t0)
	jal     in_range
	nop
	beqz    v0, err_clear
	nop

1:
	and     t4, 0xFFFF                  # if not in leading step
	sw      t4, LAST_PULSE_WIDTH(t8)

	li      t0, 1
	sb      t0, FIRST_HALF_GOT(t8)
	b       end_decode
	nop

2:
	sb      zero, FIRST_HALF_GOT(t8)    # if first half is not 0

	lw      a0, LAST_PULSE_WIDTH(t8)    
	and     t4, 0xFFFF
	addu    a0, t4                      # a0 is the pulse width sum

	move    t1, t5
	li      t0, IR_REPEAT_LEADING       
	slt     k1, t1, t0
	bnez    k1, 1f     
	nop
	addiu   t1, 1                       # if step >= repeat leading

1:
	sll     t0, t1, 3                   # see the struct ir_attr before
	addu    t0, t7
	addiu   t0, PULSE_LEADING
	lhu     t1, PULSE_LOW(t0)
	lhu     t2, PULSE_HIGH(t0)
	addu    a1, t1, t2
	lhu     a2, PULSE_TORL(t0)

	jal     in_range
	nop

	li      t0, IR_DATA                
	beq     t0, t5, 1f
	nop

	beqz    v0, err_clear               # if not in data step
	nop
	b       2f
	nop

1:
	li      t0, 0
	bnez    v0, 1f                      # if in data step & v0 not equal 0, t0=0 and jump
	nop

	addiu   t0, t7, PULSE_DATA1
	lhu     t1, PULSE_LOW(t0)
	lhu     t2, PULSE_HIGH(t0)
	addu    a1, t1, t2
	lhu     a2, PULSE_TORL(t0)

	jal     in_range
	nop

	beqz    v0, err_clear
	nop

	li      t0, 1                       # if in data step & v0 equal 0, t0=1

1:
	lbu     t2, KEY_BIT_CNT(t8)         # t2 is the key bit count
	beqz    t0, 1f                      
	nop
	and     t3, t2, 0x7
	sllv    t0, t0, t3
	srl     t4, t2, 3
	addiu   t4, KEY_CODE
	addu    t4, t8
	lbu     t3, (t4)
	or      t0, t3
	sb      t0, (t4)
1:
	addiu   t2, 1
	sb      t2, KEY_BIT_CNT(t8)

2:
	lbu     t0, DECODE_STEP(t8)
	addiu   t0, 1
	sb      t0, DECODE_STEP(t8)
	addu    t0, t6
	lbu     t1, (t0)
	li      t2, IR_END
	bne     t1, t2, end_decode
	nop

	/* new key code got */
	lhu     t0, (t7)
	and     t1, t0, BIT_MSB_FIRST
	beqz    t1, 1f
	nop

	addiu   a0, t8, KEY_CODE            # if it is bit msb first
	addiu   a1, t8, LAST_KEY_CODE
	lbu     a2, KEY_BIT_CNT(t8)

	jal     reverse_code_bit
	nop

	addiu   a0, t8, LAST_KEY_CODE
	addiu   a1, t8, KEY_CODE
	lbu     a2, KEY_BIT_CNT(t8)
	addiu   a2, 7
	srl     a2, 3
	jal     copy_code
	nop

1:
	lhu     t0, (t7)
	and     t1, t0, BYTE_MSB_FIRST
	beqz    t1, 1f
	nop

	addiu   a0, t8, KEY_CODE            # if it is byte msb first
	addiu   a1, t8, LAST_KEY_CODE
	lbu     a2, KEY_BIT_CNT(t8)

	jal     reverse_code_byte
	nop

	addiu   a0, t8, LAST_KEY_CODE
	addiu   a1, t8, KEY_CODE
	lbu     a2, KEY_BIT_CNT(t8)
	addiu   a2, 7
	srl     a2, 3
	jal     copy_code
	nop

1:
	addiu   t0, t8, KEY_CODE
	lw      t1, (t0)

	move    a0, t8
	li      a1, IR_DECODER_SIZE
	jal     clear_buffer
	nop
			
	move    v0, t1    
	
9:
	lb	t1,0xB8018305;
	and	t1,0x40
	beqz    t1, 9b	//wait uart empty
	nop
	move    t1, v0   	// print ir key
	sb		t1, 0xb8018300	//bit[7-0]
	nop
	srl 		t1, 8
	sb		t1, 0xb8018300	//bit[15-8]
	nop
	srl 		t1, 8
	sb		t1, 0xb8018300	//bit[23-16]
	nop
	srl 		t1, 8
	sb		t1, 0xb8018300	//bit[31-24]
	nop
9:
	lb	t1,0xB8018305;
	and	t1,0x40
	beqz    t1, 9b	//wait uart empty
	nop
	nop
return_to:
	li      s2, 0
	li      s3, 0
	move    ra, v1
	jr      ra
	nop

err_clear:
	move    a0, t8
	li      a1, IR_DECODER_SIZE
	jal     clear_buffer
	nop

end_decode:
	addiu   t9, 1
	li      t0, IR_TYPE_NUM
	slt     k1, t9, t0
	bnez    k1, start_decode
	nop
	li      t9, 0
	li      t1, IR_PLS_SUM
	sh      zero, (t1)
	b       accum_pulse   # Re-start acc next segment of waveform
	nop

END(get_ir)


/*
fucntion name   : in_range(a0, a1, a2)
a0  input value
a1  absolute value
a2  tolerance
used register   : v0,a0,a1,a2,a3
*/
LEAF(in_range)
	slt     v0, a1, a0              # v0 = (a1 < a0) ? 1 : 0
	bnez    v0, 1f
	nop
	b       2f
	sub     a3, a1, a0              # a1 > a0
1:
	sub     a3, a0, a1              # a1 < a0
2:
	jr      ra
	slt     v0, a3, a2              # If |a1 - a0| < a2, reture 1, else 0
END(in_range)

/*
function name   : copy_code(a0, a1, a2)
a0  input key code (pointer)
a1  output key code (pointer)
a2  byte count
used register   : v0,a0,a1,a2
*/
LEAF(copy_code)
1:
	lbu     v0, (a0)
	sb      v0, (a1)
	addiu   a0, 1
	addiu   a1, 1
	addiu   a2, -1
	bnez    a2, 1b
	nop
	jr      ra
	nop
END(copy_code)

/*
function name   : clear_buffer(a0, a1)
a0  buffer addresss
a1  length in byte
used register   : a0,a1
*/
LEAF(clear_buffer)
1:
	sb      zero, (a0)
	addiu   a0, 1
	addiu   a1, -1
	bnez    a1, 1b
	nop
	jr      ra
	nop
END(clear_buffer)

/*
function name   : reverse_code_bit(a0, a1, a2)
a0  input key code (pointer)
a1  output key code (pointer)
a2  bit count
used register   : v0,a0,a1,a2,a3,t0,t1,t2
*/
LEAF(reverse_code_bit)
	sw      zero, (a1)  # Clear output key code buffer
	addiu   a1, 4
	sw      zero, (a1)
	addiu   a1, 4
	sw      zero, (a1)
	addiu   a1, 4
	sw      zero, (a1)
	addiu   a1, 4
	sw      zero, (a1)
	addiu   a1, -16

	move    t0, zero
1:
	addiu   a2, -1
	srl     a3, a2, 3
	and     v0, a2, 0x07
	addu    a3, a0
	lbu     t2, (a3)
	li      t1, 1
	sllv    v0, t1, v0
	and     v0, t2
	beqz    v0, 2f
	nop
	srl     a3, t0, 3
	and     v0, t0, 0x07
	addu    a3, a1
	lbu     t2, (a3)
	li      t1, 1
	sllv    v0, t1, v0
	or      v0, t2
	sb      v0, (a3)
2:
	bnez    a2, 1b
	addiu   t0, 1
	jr      ra
	nop
END(reverse_code_bit)

/*
function name   : reverse_code_byte(a0, a1, a2)
a0  input key code (pointer)
a1  output key code (pointer)
a2  bit count
used register   : v0,a0,a1,a2,a3
*/
LEAF(reverse_code_byte)
	sw      zero, (a1)  # Clear output key code buffer
	addiu   a1, 4
	sw      zero, (a1)
	addiu   a1, 4
	sw      zero, (a1)
	addiu   a1, 4
	sw      zero, (a1)
	addiu   a1, 4
	sw      zero, (a1)
	addiu   a1, -16

	addiu   a2, 7
	srl     a2, 3
1:
	addiu   a2, -1
	addu    v0, a2, a0
	lbu     a3, (v0)
	sb      a3, (a1)
	bnez    a2, 1b
	addiu   a1, 1
	jr      ra
	nop
END(reverse_code_byte)



