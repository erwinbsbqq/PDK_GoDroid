/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: tsg.h
 *
 *  Description: This file provide TS Gen interface functions.
 *
 *  History:
 *      Date            Author            Version       	Comment
 *      ====          ======        =======  	=======
 *  1.  2005.8.19	Goliath Peng   0.0.1		Create File
 *  2.  2005.11.17  Mengdong Lin  0.0.1		Interface Change.
 ****************************************************************************/

#ifndef	__TSG_H__
#define __TSG_H__

#include <sys_config.h>

//This value defines the M36XX maximum number of packets to handle in one transfer.
//For M3329E, the maximum number is TSG_MAX_XFER_PKTS/2 == 1394;
#define TSG_MAX_XFER_PKTS 2788  

#define TSG_MAX_XFER_WAIT_INTERVAL 2000 


RET_CODE tsg_init(UINT8 clk_sel);
UINT32 tsg_transfer(void *addr, UINT16 pkt_cnt, UINT8 sync, UINT32 bfc);
void tsg_wait(UINT32 xfer_id);
void tsg_set_clk(UINT8 clk_sel);
void tsg_set_clk_async(UINT8 clk_sel);
void tsg_start(UINT32 bitrate);
void tsg_stop(void);
RET_CODE tsg_check_buf_busy(UINT32 buf_addr,UINT32 buf_len);
UINT32 tsg_check_remain_buf();


#endif	/* __TSG_H__ */

