/*
* Copyright (c) 2011,Ali Corp. All rights reserved.
* 
*    FileName£ºqlog.h
*     Verison£º0.12
*      Author£ºjacket (jacket.wu@alitech.com)
*        Date: 2011-08-15
* Description: log module header file.
*/

#ifndef __QLOG_H__
#define __QLOG_H__

/* log level definition */
typedef enum _log_level_e
{
    LOG_ALL_CLOSE = (0),          //level 0
    LOG_ERR = (1<<0),             //level 1
    LOG_WARN = (1<<1),            //level 2
    LOG_INFO = (1<<2),            //level 3
    LOG_DEBUG0 = (1<<3),          //level 4
    LOG_DEBUG1 = (1<<4),          //level 5
    LOG_DEBUG2 = (1<<5),          //level 6
    LOG_DEBUG3 = (1<<6),          //level 7
    LOG_DEBUG4 = (1<<7),          //level 8
    LOG_DEBUG5 = (1<<8),          //level 9
    LOG_DEBUG6 = (1<<9),          //level 10
    LOG_DEBUG7 = (1<<10),         //level 11
    LOG_DEBUG8 = (1<<11),         //level 12
    LOG_DEBUG9 = (1<<12),         //level 13
    LOG_DEBUG10 = (1<<13),        //level 14
    LOG_DEBUG11 = (1<<14),        //level 15
    LOG_DEBUG12 = (1<<15),        //level 16
    LOG_DEBUG13 = (1<<16),        //level 17
    LOG_DEBUG14 = (1<<17),        //level 18
    LOG_DEBUG15 = (1<<18),        //level 19
    LOG_DEBUG16 = (1<<19),        //level 20
    LOG_DEBUG17 = (1<<20),        //level 21
    LOG_DEBUG18 = (1<<21),        //level 22
    LOG_DEBUG19 = (1<<22),        //level 23
    LOG_DEBUG20 = (1<<23),        //level 24
    LOG_DEBUG21 = (1<<24),        //level 25
    LOG_DEBUG22 = (1<<25),        //level 26
    LOG_DEBUG23 = (1<<26),        //level 27
    LOG_DEBUG24 = (1<<27),        //level 28
    LOG_DEBUG25 = (1<<28),        //level 29
    LOG_DEBUG26 = (1<<29),        //level 30
    LOG_DEBUG27 = (1<<30),        //level 31   
} Log_Level_E;

/* log print mode definition */
/* Initialize the log module */
int dbg_log_init(char * module_name, Log_Level_E _level);
/* Print the log messages according to the settings */
int dbg_log_print(Log_Level_E level, 
                  const char *fmt_str, ...);
#endif //__QLOG_H__

