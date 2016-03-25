#ifndef _ALI_IR_H_
#define _ALI_IR_H_


#include <boot_common.h>


enum pan_key_press
{
	PAN_KEY_RELEASE		= 0,
	PAN_KEY_PRESSED		= 1,
	PAN_KEY_REPEAT		= 2
};

enum pan_key_type
{
	PAN_KEY_TYPE_INVALID	= 0,	/* Invalid key type */
	PAN_KEY_TYPE_REMOTE		= 1,	/* Remote controller */
	PAN_KEY_TYPE_PANEL		= 2,	/* Front panel */
	PAN_KEY_TYPE_JOYSTICK	= 3,	/* Game joy stick */
	PAN_KEY_TYPE_KEYBOARD	= 4		/* Key board */
};


struct pan_key
{
	UINT8  type;					/* The key type */
	UINT8  state;					/* The key press state */
	UINT16 count;					/* The key counter */
	UINT32 code;					/* The value */
};


void irc_init(void);
void irc_lsr(void);
UINT32 irc_get_last_act_code(void);
UINT8 irc_get_ir_repeat(void);
UINT8 irc_get_ir_pulse(void);



#endif /* _ALI_IR_H_ */
