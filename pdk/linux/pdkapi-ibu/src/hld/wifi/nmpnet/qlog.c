/*
* Copyright (c) 2011,Ali Corp. All rights reserved.
* 
*    FileName£ºqlog.c
*     Verison£º0.12
*        Date: 2011-08-15
* Description: This code used to build a mechanism of log level for AP debugging. 
*              The log module supports the dynamic change of log level. 
*              And it provides setting options for AP developers to examine
*              their own log messages: close the printing function,
*              output to a file or console.
*/

#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/mman.h>
#include <errno.h>
#include <limits.h>

#include "qlog.h"

static Log_Level_E g_level = 0;
static char g_module_name[24] ={0};
/*
 * @brief:  Read the log level from the g_dbg_log_property
 * @param:  dbgLevel
 * @return: level message corresponding to dbgLevel
 */
static const char *get_log_level(Log_Level_E dbgLevel)
{
    switch (dbgLevel) {
        case LOG_ALL_CLOSE:    return("LOG_ALL_CLOSE");
        case LOG_ERR:          return("LOG_ERR");
        case LOG_WARN:         return("LOG_WARN");
        case LOG_INFO:         return("LOG_INFO");
        case LOG_DEBUG0:       return("LOG_DEBUG0");
        case LOG_DEBUG1:       return("LOG_DEBUG1");
        case LOG_DEBUG2:       return("LOG_DEBUG2");
        case LOG_DEBUG3:       return("LOG_DEBUG3");
        case LOG_DEBUG4:       return("LOG_DEBUG4");
        case LOG_DEBUG5:       return("LOG_DEBUG5");
        case LOG_DEBUG6:       return("LOG_DEBUG6");
        case LOG_DEBUG7:       return("LOG_DEBUG7");
        case LOG_DEBUG8:       return("LOG_DEBUG8");
        case LOG_DEBUG9:       return("LOG_DEBUG9");
        case LOG_DEBUG10:      return("LOG_DEBUG10");
        case LOG_DEBUG11:      return("LOG_DEBUG11");
        case LOG_DEBUG12:      return("LOG_DEBUG12");
        case LOG_DEBUG13:      return("LOG_DEBUG13");
        case LOG_DEBUG14:      return("LOG_DEBUG14");
        case LOG_DEBUG15:      return("LOG_DEBUG15");
        case LOG_DEBUG16:      return("LOG_DEBUG16");
        case LOG_DEBUG17:      return("LOG_DEBUG17");
        case LOG_DEBUG18:      return("LOG_DEBUG18");
        case LOG_DEBUG19:      return("LOG_DEBUG19");
        case LOG_DEBUG20:      return("LOG_DEBUG20");
        case LOG_DEBUG21:      return("LOG_DEBUG21");
        case LOG_DEBUG22:      return("LOG_DEBUG22");
        case LOG_DEBUG23:      return("LOG_DEBUG23");
        case LOG_DEBUG24:      return("LOG_DEBUG24");
        case LOG_DEBUG25:      return("LOG_DEBUG25");
        case LOG_DEBUG26:      return("LOG_DEBUG26");
        case LOG_DEBUG27:      return("LOG_DEBUG27");
        default:               return("LOG_LEVEL_NONE");
    }
}

/*
 * @brief:  Print the log messages according to the settings.
 * @param:  level, ...
 * @return: int
 */
int dbg_log_print(Log_Level_E level,
                  const char *fmt_str, ...)
{
    if (level & g_level) 
	{
	    va_list ap; 
		char dbg_buff[4096] = {0};
	    time_t curr_time = time(NULL);
	    char asc_time[64];

	    // Assign the address of the first alterable parameter to ap,
	    // then ap point to the list.
	    va_start(ap, fmt_str);
		
	    // All the alterable parameters changed to string and saved in dbg_buff.
	    vsnprintf(dbg_buff, 4096, fmt_str, ap);
	    va_end(ap);

	    // Get time.
	    strcpy(asc_time, asctime(localtime(&curr_time)));
	    asc_time[strlen(asc_time)-1] = '\0';
//deleted by elan for debug
#if 1
        printf("[%s][%s]%s::%s\n", asc_time, g_module_name, get_log_level(level),\
                    dbg_buff);
#endif
		fflush(stdout);	
	}
    return (0);
}
/**
 * @brief:  initialize the log module 
 * @param:  the name of the file which saves log messages.
 * @return: int
 */
int dbg_log_init(char* module_name, Log_Level_E _level)
{
	strncpy(g_module_name, module_name, 24);
	g_level = _level;
	return 0;
}
