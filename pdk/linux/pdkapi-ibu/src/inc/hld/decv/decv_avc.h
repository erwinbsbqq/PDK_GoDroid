/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: decv_avc.h
 *
 *  Description: head file for avc video decoder device management.
 *
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2007.7.271  Michael	Xie	     0.0.1   Create
 ****************************************************************************/
#ifndef  _DECV_AVC_H_
#define  _DECV_AVC_H_

#include <basic_types.h>
#include <mediatypes.h>

#include <sys_config.h>

#ifndef __DRIVERS_ALI_VDEC_AVC_MMAP
#define __DRIVERS_ALI_VDEC_AVC_MMAP
struct vdec_avc_memmap
{
	BOOL support_multi_bank;

	UINT32 frame_buffer_base;
	UINT32 frame_buffer_len;

	UINT32 dv_frame_buffer_base;
	UINT32 dv_frame_buffer_len;

	UINT32 mv_buffer_base;
	UINT32 mv_buffer_len;

	UINT32 mb_col_buffer_base;
	UINT32 mb_col_buffer_len;

	UINT32 mb_neighbour_buffer_base;
	UINT32 mb_neighbour_buffer_len;

	UINT32 cmd_queue_buffer_base;
	UINT32 cmd_queue_buffer_len;

	UINT32 vbv_buffer_base;
	UINT32 vbv_buffer_len;

	UINT32 laf_rw_buf;
	UINT32 laf_rw_buffer_len;

	UINT32 laf_flag_buf;
	UINT32 laf_flag_buffer_len;

	BOOL   support_conti_memory;
	UINT32 avc_mem_addr;
	UINT32 avc_mem_len;
};
#endif


#ifndef __DRIVERS_ALI_VDEC_CONF_PAR
#define __DRIVERS_ALI_VDEC_CONF_PAR
struct vdec_avc_config_par
{	
	struct vdec_avc_memmap memmap;
	UINT32 max_additional_fb_num;
#ifdef VIDEO_SEAMLESS_SWITCHING
	UINT8 seamless_enable;
#endif
	UINT8 dtg_afd_parsing;
};
#endif

#ifndef __DRIVERS_ALI_VDEC_M36_ATTACH
#define __DRIVERS_ALI_VDEC_M36_ATTACH
void vdec_avc_attach(struct vdec_avc_config_par *pconfig_par);
#endif

#endif  /* _DECV_H_*/






