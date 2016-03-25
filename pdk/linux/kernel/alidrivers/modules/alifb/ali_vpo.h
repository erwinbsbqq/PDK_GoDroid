#ifndef __DRIVERS_VIDEO_ALI_VPO_H
#define __DRIVERS_VIDEO_ALI_VPO_H

#define VPO_BASE_ADDR		(0xB8006100)

#define VPO_W8(a, b)	ALIFB_W8(VPO_BASE_ADDR + a, b)
#define VPO_W16(a, b)	ALIFB_W16(VPO_BASE_ADDR + a, b)
#define VPO_W32(a, b)	ALIFB_W32(VPO_BASE_ADDR + a, b)

#define VPO_R8(a)	ALIFB_R8(VPO_BASE_ADDR + a)
#define VPO_R16(a)	ALIFB_R16(VPO_BASE_ADDR + a)
#define VPO_R32(a)	ALIFB_R32(VPO_BASE_ADDR + a)

int ali_vpo_init_GMA(struct alifb_info *info);
int ali_vpo_deinit_GMA(struct alifb_info *info);
int ali_vpo_create_GMA_region(struct alifb_info *info, int region_id);
int ali_vpo_delete_GMA_region(struct alifb_info *info, int region_id);
int ali_vpo_update_GMA_region(struct alifb_info *info, int region_id);
void ali_vpo_show_GMA_layer(struct alifb_info *info, int on);
void ali_vpo_set_GMA_palette(struct alifb_info *info);
void ali_vpo_set_GMA_alpha(struct alifb_info *info, unsigned char alpha);
int ali_vpo_set_full_screen(struct alifb_info *info, int src_width, int src_height);
void ali_vpo_set_region_by(struct alifb_info *info, unsigned char set_region_by);
void ali_osd_set_enhance_pars(struct alifb_info *info, int change_flag, int value);
void ali_gma_set_dbg_flag(struct alifb_info *info, unsigned long flag);
void alifb_set_osd2_position(struct alifb_info *info, unsigned long screen_width, unsigned long screen_height, unsigned long dwParam);

#ifdef CONFIG_RPC_HLD_GMA
int ali_vpo_set_full_screen(struct alifb_info *info, int src_width, int src_height);
#else
int alifb_gma_open(struct alifb_info *info);
int alifb_gma_create_region(struct alifb_info *info, int region_id);
void ali_set_GMA_IOCtl(struct alifb_info *info, unsigned long dwParam);
void ali_get_GMA_IOCtl(struct alifb_info *info, unsigned long dwParam);
#endif

#endif

