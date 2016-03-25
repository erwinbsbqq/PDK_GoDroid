/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_fake.h
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of fake trace reader ioctl commands.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/
#ifndef _HLD_FAKE_TRACE_H_
#define _HLD_FAKE_TRACE_H_

RET_CODE fake_kft_stop(void);
unsigned long fake_get_tick(const char *);

#endif