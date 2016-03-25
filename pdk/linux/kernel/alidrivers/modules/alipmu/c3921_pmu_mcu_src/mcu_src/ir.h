#ifndef __IR_H__
#define __IR_H__
#include "sys.h"

//=====================================================================================//
#define IR_BIT				                                                                                  0
#define IR_IOBASE		                                                                                         0xE000
#define INTR_REG_STAT		                                                                                  (IR_IOBASE+0x0030)
#define INTR_REG_CTRL		                                                                                  (IR_IOBASE+0x0038)
#define INTR_REG_PRO		                                                                                  (IR_IOBASE+0x0028)
#define IR_REG_CFG		                                                                                         (IR_IOBASE+0x00)
#define IR_REG_FIFOCTRL	                                                                                         (IR_IOBASE+0x01)
#define IR_REG_TIMETHR		                                                                                  (IR_IOBASE+0x02)
#define IR_REG_NOISETHR	                                                                                  (IR_IOBASE+0x03)
#define IR_REG_IER		                                                                                         (IR_IOBASE+0x06)
#define IR_REG_ISR		                                                                                         (IR_IOBASE+0x07)
#define IR_REG_RLCBYTE		                                                                                  (IR_IOBASE+0x08)

//=====================================================================================//
typedef struct
{
	UINT8  ir_key_low0;
	UINT8  ir_key_low1;
	UINT8  ir_key_low2;
	UINT8  ir_key_low3;
}IR_KEY,pIR_KEY;

extern IR_KEY g_set_ir_key;
extern INT8 get_ir(void);
#endif
