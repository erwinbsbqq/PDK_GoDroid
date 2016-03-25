#ifndef __ADR_TUNER_DBG_H__
#define __ADR_TUNER_DBG_H__

#define TUNER_DBG_CMD_NR      10
#define TUNER_DBG_PRINT_INTRV 500

typedef struct tuner_dbg_stat_ctrl
{
	int      dbg_dev_fd;
	UINT32   dbg_task_id;
	
	UINT8    dbg_init;
	UINT8    dbg_tsk_init;
	UINT8    dbg_enable;
	UINT8    dbg_ber_en;
	UINT8    dbg_snr_en;
	UINT8    dbg_sig_en;
	UINT8    dbg_krv_en;
	
	UINT8    dbg_i2c_chnl;
	UINT8    dbg_i2c_addr;
	UINT32   dbg_show_intv;  //Unit: ms default 5000ms
	UINT32   dbg_show_ims;
} tuner_dbg_stat_ctrl;

#endif

