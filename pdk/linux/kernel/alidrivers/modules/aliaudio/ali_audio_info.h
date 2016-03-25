#ifndef _ALI_AUDIO_INFO_H_
#define _ALI_AUDIO_INFO_H_

#include <ali_reg.h>

/* Physical to virtual address and vise visa.
 */
#define ALI_PHYS2VIRT(x) (x - ALI_REGS_PHYS_BASE + ALI_REGS_VIRT_BASE)
#define ALI_VIRT2PHYS(x) (x - ALI_REGS_VIRT_BASE + ALI_REGS_PHYS_BASE)


/* Must keep compartible with SEE:struct snd_info
*/
struct ali_audio_see_snd_info
{
    __u32 i2si_rx_samp_int_cnt;
    __u32 i2si_tx_samp_int_cnt;
    __u32 i2so_samp_int_cnt;
    __u32 spo_samp_int_cnt;
    __u32 ddp_spo_samp_int_cnt;
    
    __u32 i2so_underrun_int_cnt;
    __u32 spo_underrun_int_cnt;
    __u32 ddp_spo_underrun_int_cnt;
    
    __u32 i2so_resume_int_cnt;
    __u32 spo_resume_int_cnt;
    __u32 ddp_spo_resume_int_cnt;

    __u32 i2so_timing_chk_int_cnt;	

	/* avsync statistics.
	*/
	__u32 new_avsync_play_cnt;
	__u32 new_avsync_drop_cnt;
	__u32 new_avsync_hold_cnt;
	__u32 new_avsync_freerun_cnt;
	__u32 new_avsync_hw_hold_cnt;		

	__u32 old_avsync_hold_cnt;
	__u32 old_avsync_drop_cnt;
	__u32 old_avsync_freerun_cnt;	

	/* Buffer full/empty statistics.
	*/
	__u32 pcm_sync_buff_empty_cnt;
	__u32 es_sync_buff_empty_cnt;
	__u32 es_ddp_sync_buff_empty_cnt;
	__u32 pcm_dma_buff_empty_cnt;
	__u32 es_dma_buff_empty_cnt;	
	__u32 es_ddp_dma_buff_empty_cnt;

	__u32 pcm_sync_buff_full_cnt;
	__u32 es_sync_buff_full_cnt;
	__u32 es_ddp_sync_buff_full_cnt;
	__u32 pcm_dma_buff_full_cnt;
	__u32 es_dma_buff_full_cnt;	
	__u32 es_ddp_dma_buff_full_cnt;		

	/* snd_lld_output_task() run count.
	*/
    __u32 snd_lld_input_go_cnt;
    __u32 snd_lld_input_fail_cnt;
    __u32 snd_lld_input_succ_cnt;
    __u32 snd_lld_check_dma_buff_go_cnt;
    __u32 snd_lld_check_dma_buff_fail_cnt;
    __u32 snd_lld_check_dma_buff_succ_cnt;
    __u32 snd_lld_output_go_cnt;
    __u32 snd_lld_output_succ_cnt;	
};



/* Must keep compartible with SEE:enum ALI_SEE_APE_DECA_TASK_STATUS
*/
enum ALI_SEE_APE_DECA_TASK_STATUS 
{
    APE_DECA_TASK_STATUS_IDLE = 0,
    APE_DECA_TASK_STATUS_RUN = 1,
	APE_DECA_TASK_STATUS_PAUSE = 2,   
};


/* Must keep compartible with SEE code:struct ape_info_audio
*/
struct ali_audio_see_ape_info
{
    /* deca task status.
	*/
	enum ALI_SEE_APE_DECA_TASK_STATUS deca_task_status;
	
    /* Decoder task.
	*/
    __u32 deca_task_go_cnt;

    __u32 deca_task_rd_hdr_go_cnt;
	__u32 deca_task_rd_hdr_succ_cnt;
	__u32 deca_task_rd_hdr_fail_cnt;

	__u32 deca_task_rd_data_go_cnt;
    __u32 deca_task_rd_data_fail_cnt;
    __u32 deca_task_rd_data_succ_cnt;
	
	__u32 deca_task_ring_busy_cnt;

	__u32 deca_task_avysync_go_cnt;
	__u32 deca_task_avysync_drop_cnt;
	__u32 deca_task_avysync_ok_cnt;	

	__u32 deca_task_decode_go_cnt;	
	__u32 deca_task_decode_succ_cnt;
	__u32 deca_task_decode_busy_cnt;	
	__u32 deca_task_decode_wr_pcm_fail_cnt;
    __u32 deca_task_decode_buffering_cnt;
	__u32 deca_task_decode_samplerate_chg_cnt;
    __u32 deca_task_decode_ch_num_chg_cnt;
	__u32 deca_task_sleep_cnt;

    /* APE deca configration.
	*/
    __s32 decode_mode;
    __s32 sync_mode;
    __s32 sync_unit;
    __s32 deca_input_sbm;
    __s32 deca_output_sbm;
    __s32 snd_input_sbm;
    __s32 pcm_sbm;
    __s32 codec_id;
    __s32 bits_per_coded_sample;
    __s32 sample_rate;
    __s32 channels;
    __s32 bit_rate;
    __u32 pcm_buf;
    __u32 pcm_buf_size;
    __s32 block_align;

	/* APE interchange buffer configration.
	*/
	__u32 dwRIFF;
	__u32 dwFileLen;
	__u32 dwWAVE;
	__u32 dw_fmt;
	__u32 dwFmtLen;
	__u16 wDataType;
	__u16 wNChannels;
	__u32 dwSamplingRate;
	__u32 dwNBytesPerSec;
	__u16 wAlignment;
	__u16 wNBitsPerSam;
	__u32 dwdata;
	__u32 dwDataLen;

    __u32 pAudioInit;
    __u32 pAudioOutput;
    __u32 pAudioStop;
    __u32 pAudioReset;
    __u32 m_pAdecAvailable;
    __u32 m_pAdecPlayerState;
    __u32 m_pAdecBufEmpty;	
};


__s32 ali_audio_info_init(void);
__s32 ali_audio_info_exit(void);

__s32 ali_audio_api_show(char * fmt,...);

#endif

