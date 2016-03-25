#ifndef _ALI_ALSA_DBG_H_
#define _ALI_ALSA_DBG_H_

int32_t ali_alsa_dbg_init(void);
int32_t ali_alsa_dbg_exit(void);

int32_t ali_alsa_playback_dump_en_get(void);
int32_t ali_alsa_capture_dump_en_get(void);
int32_t ali_alsa_i2so_see2main_dump_en_get(void);
int32_t ali_alsa_i2sirx_see2main_dump_en_get(void);
int32_t ali_alsa_i2sirx_i2so_mix_dump_en_get(void);
int32_t ali_alsa_i2sirx_i2so_mix_out_dump_en_get(void);
int32_t ali_alsa_i2sirx_i2so_mix_drop_ms_get(void);
int32_t ali_alsa_i2sirx_i2so_mix_align_ms_get(void);

int32_t ali_alsa_dump_data(char *file_path, unsigned char *data, const int len);
#endif

