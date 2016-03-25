#ifndef __ALI_SBM_DBG_H_
#define __ALI_SBM_DBG_H_

__s32 ali_sbm_api_show(char * fmt,...);

__s32 ali_sbm_dbg_init(void);

__s32 ali_sbm_stats_write_go_cnt(struct sbm_dev *dev, __u32 inc_num);
__s32 ali_sbm_stats_write_ill_status_cnt(struct sbm_dev *dev, __u32 inc_num);
__s32 ali_sbm_stats_write_mutex_fail_cnt(struct sbm_dev *dev, __u32 inc_num);
__s32 ali_sbm_stats_write_ok_cnt(struct sbm_dev *dev, __u32 inc_num);

#endif