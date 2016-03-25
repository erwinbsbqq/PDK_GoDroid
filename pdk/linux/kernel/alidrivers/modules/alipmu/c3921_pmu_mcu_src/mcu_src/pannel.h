#ifndef __PANNEL_H__
#define __PANNEL_H__
#include "sys.h"        

//==========================================================//
enum SHOW_TYPE
{
	SHOW_OFF =0,
	SHOW_TIME,
	SHOW_BANK
};

extern void pannel_init(void);
extern void show_pannel(enum SHOW_TYPE show_type,pRTC_TIMER rtc);
extern char pannel_scan(void);
#endif 
