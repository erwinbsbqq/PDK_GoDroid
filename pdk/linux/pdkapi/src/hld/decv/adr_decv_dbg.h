#ifndef __ADR_DECV_DBG_INTER_H__
#define __ADR_DECV_DBG_INTER_H__

#define DECV_DBG_LEVEL_HLD			0x01
#define DECV_DBG_LEVEL_KERNEL			0x02
#define DECV_DBG_LEVEL_SEE			0x04

void decvdbg_register(void);

void vdec_set_dbg_level(int hld, int kernel, int see);

#endif

