/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: osddrv.c
 *
 *  Description: Hld osd driver
 *
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2010.03.11  Sam				4.0		Support Linux Driver
 ****************************************************************************/
 
#include <adr_basic_types.h>
#include <adr_mediatypes.h>	
#include <osal/osal.h>
#include <hld_cfg.h>
#include <hld/adr_hld_dev.h>
#include <hld/osd/adr_osddrv.h>
#include <hld/ge/adr_ge.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <err/errno.h>

#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>

#include <linux/fb.h>
#include <ali_video_common.h>
#include <ali_decv_plugin_common.h>
#include <alidefinition/adf_gma.h> 

#ifdef ADR_IPC_ENABLE
#include <hld/misc/adr_ipc.h>
#define OSD_PRF(fmt, args...)  \
			do { \
				if (p_osd_dbg_on != NULL) { \
					if (*p_osd_dbg_on) { \
						ADR_DBG_PRINT(OSD, "%s->%s: L %d: dev %s : "fmt"\n", __FILE__, \
							__FUNCTION__, __LINE__, dev->name, ##args); \
					} \
				} \
			} while(0)


#define MUTEX_LOCK()	adr_ipc_semlock(m_osd_mutex_id)
#define MUTEX_UNLOCK() adr_ipc_semunlock(m_osd_mutex_id)
#else
#define OSD_PRF(fmt, args...)  \
			do { \
				if (g_osd_dbg_on) \
				{ \
					ADR_DBG_PRINT(OSD, "%s->%s: L %d: "fmt"\n", __FILE__, \
						__FUNCTION__, __LINE__, ##args); \
				} \
			} while(0)

#define MUTEX_LOCK()	do{}while(0)
#define MUTEX_UNLOCK() do{}while(0)
#endif

#define OSD_MAX_DEV_NUM		2
#define OSD_MAX_REG_NUM		6

#define GE_SIMU_MUTEX_LOCK()  osal_mutex_lock(dev->sema_opert_osd, OSAL_WAIT_FOREVER_TIME)
#define GE_SIMU_MUTEX_UNLOCK()  osal_mutex_unlock(dev->sema_opert_osd)


struct osd_region_par
{
	int valid;
	struct OSDRect reg_rect;
	int bpp;

	struct OSDPara para;
	
	void *mem_start;
	int mem_size;
	int pitch;		
};

struct osd_private
{
	int index;
	int handle;
	char *file_path;
	
	struct OSDPara default_para;
	int show_on;

	/* phy address of the OSD memory */
	void *mem_start;
	int mem_size;	

	int max_logical_width;
	int max_logical_height;
	int pitch;	

	/* scale parameters */
	int scale_mode; // OSD_SCALE_DUPLICATE ; OSD_SCALE_FILETER
	int h_mul;
	int h_div;
	
	int v_mul;
	int v_div;
	
	struct osd_region_par region_par[OSD_MAX_REG_NUM];
	
#ifdef ADR_IPC_ENABLE
	int opened;
	int inited;
	int dbg_on;

	int shmid;
#endif		
};

struct osd_mem
{
	void *pallete_buf_allocated;
	void *pallete_buf;
	int pallete_size;
	
	/* direct entry to the the OSD memory */
	void *virtual_mem_start;
	int virtual_mem_size;
};

#ifdef ADR_IPC_ENABLE
int *p_osd_dbg_on = NULL;
#else
int g_osd_dbg_on = 0;
#endif

static char m_osd_name[OSD_MAX_DEV_NUM][HLD_MAX_NAME_SIZE] 
	= {"OSD_S36_0", "OSD_S36_1"};

static struct osd_device *m_osd_dev[OSD_MAX_DEV_NUM]= {NULL, NULL};
static struct osd_private *m_osd_priv[OSD_MAX_DEV_NUM] = {NULL, NULL};

static struct osd_mem m_osd_mem[OSD_MAX_DEV_NUM];
	
static UINT8 m_osd_trans_color = 0;
static int m_osd_attached = 0;

#ifdef ADR_ALIDROID
static char fb0_path[] = "/dev/graphics/fb0";
static char fb2_path[] = "/dev/graphics/fb2";
#else
static char fb0_path[] = "/dev/fb0";
static char fb2_path[] = "/dev/fb2";
#endif

#ifdef ADR_IPC_ENABLE
static int m_osd_mutex_id = 0;
#endif

static int m_osd_no_clear_buf = 0;

static int m_osd_file_handle[OSD_MAX_DEV_NUM] = {0, 0};

static void rect_set(INT16 dst_width,
					INT16 dst_height,
					UINT8* dst_buf,
					struct OSDRect *dst_rect,
					UINT32  data,
					UINT32 pitch,
					UINT32 bpp)
{
	UINT16 i,j;
	UINT32 *buf32 = NULL;
	UINT16 *buf16 = NULL;
	
	dst_buf += (pitch)*(dst_rect->uTop )+(dst_rect->uLeft * bpp);

	for(i = 0; i<dst_rect->uHeight;i++)
	{
		for(j = 0;j<dst_rect->uWidth;j++)
		{
			if(bpp == 1)
				*(dst_buf + j) = data & 0xFF;
			else if(bpp == 2)
			{
				*(dst_buf + 2 * j) = data & 0xFF;
				*(dst_buf + 2 * j + 1) = (data>>8) & 0xFF;
			}
			else if(bpp == 4) 
			{
				*(dst_buf + 4 * j) = data & 0xFF;
				*(dst_buf + 4 * j + 1) = (data>>8) & 0xFF;
				*(dst_buf + 4 * j + 2) = (data>>16) & 0xFF;
				*(dst_buf + 4 * j + 3) = (data>>24) & 0xFF;				
			}
		}
		
		dst_buf += pitch;
	}
}

static void rect_cpy(UINT16 dst_width,
					UINT16 dst_height,
					UINT8* dst_buf,
					UINT16 src_width,
					UINT16 src_height,
					UINT8* src_buf,
					struct OSDRect *dst_rect,
					struct OSDRect *src_rect,
					UINT32 pitch_dst,		
					UINT32 pitch_src,
					UINT32 bpp)
{

	UINT16 i;
	UINT32 	uSrcBufOffset,uDstBufOffset;
	
	UINT32 len = dst_rect->uWidth * bpp;
    
	uSrcBufOffset = (pitch_src * src_rect->uTop )+(src_rect->uLeft * bpp);
	uDstBufOffset = (pitch_dst * dst_rect->uTop )+(dst_rect->uLeft * bpp);

	for(i = 0; i<dst_rect->uHeight;i++)
	{
		if(!((UINT32)(dst_buf+uDstBufOffset)&0x3) && !((UINT32)(src_buf+uSrcBufOffset)&0x3) && (0x03 <len))
		{
			UINT32 dwCount, dwLen;
			UINT32 *DestBuf,*SrcBuf;
			UINT8 * DestBufByte, *SrcBufByte;

			dwLen = (len&0xfffffffc)>>2;
			DestBuf=(UINT32 *)(dst_buf+uDstBufOffset);
			SrcBuf=(UINT32 *)(src_buf+uSrcBufOffset);
			for(dwCount=0;dwCount<dwLen;dwCount++) 
				*(DestBuf+dwCount)=*(SrcBuf+dwCount);
			dwLen = len&0x3;
			DestBufByte = (UINT8 *)(dst_buf + uDstBufOffset + (dwCount<<2));
			SrcBufByte = (UINT8 *)(src_buf + uSrcBufOffset +(dwCount<<2));
			for(dwCount=0;dwCount<dwLen;dwCount++) 
				*(DestBufByte+dwCount)=*(SrcBufByte+dwCount);
		}
		else if(!((UINT32)(dst_buf+uDstBufOffset)&0x1) && !((UINT32)(src_buf+uSrcBufOffset)&0x1) && (0x01 < len))
		{
			UINT32 wCount, wLen;
			UINT16 *DestBuf,*SrcBuf;
			UINT8 * DestBufByte, *SrcBufByte;

			wLen = (len&0xfffffffe)>>1;
			DestBuf=(UINT16 *)(dst_buf+uDstBufOffset);
			SrcBuf=(UINT16 *)(src_buf+uSrcBufOffset);
			for(wCount=0;wCount<wLen;wCount++) 
				*(DestBuf+wCount)=*(SrcBuf+wCount);
			wLen = len&0x1;
			DestBufByte = (UINT8 *)(dst_buf + uDstBufOffset +(wCount<<1));
			SrcBufByte = (UINT8 *)(src_buf + uSrcBufOffset +(wCount<<1));
			for(wCount=0;wCount<wLen;wCount++) 
				*(DestBufByte+wCount)=*(SrcBufByte+wCount);
		}
		else
			memcpy(dst_buf+uDstBufOffset, src_buf+uSrcBufOffset, len);

		uSrcBufOffset += pitch_src;
		uDstBufOffset += pitch_dst;
	}
}

static void set_gma_scale_info(struct osd_private *priv, UINT32 scale_cmd)
{
	struct alifbio_gma_scale_info_pars pars;
    struct vpo_io_get_info vpo_info;

    memset(&vpo_info, 0, sizeof(vpo_info));
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32)(&vpo_info));
	
	pars.scale_mode = (priv->scale_mode == OSD_SCALE_DUPLICATE) ? GMA_RSZ_DIRECT_RESIZE
					: GMA_RSZ_ALPHA_COLOR;
	pars.h_src = priv->h_div;
	pars.h_dst = priv->h_mul;
	pars.v_src = priv->v_div;
	pars.v_dst = priv->v_mul;
	pars.tv_sys = vpo_info.tvsys;
    pars.uScaleCmd = scale_cmd;
	ioctl(m_osd_file_handle[priv->index], FBIO_SET_GMA_SCALE_INFO, &pars);	
}

#define ROUND(x) (UINT32)(x + 0.5)
static void scale_rect (struct osd_private *priv, const struct gma_rect *src_rect, struct gma_rect *dst_rect)
{
    float x, y, w, h;
    UINT32 h_div, v_div, h_mul, v_mul;
    
    h_div = priv->h_div;
    v_div = priv->v_div;
    h_mul = priv->h_mul;
    v_mul = priv->v_mul;
    //OSD_PRF("scale rect <%d %d> => <%d %d>\n", h_div, v_div, h_mul, v_mul);

    x = src_rect->x;
    y = src_rect->y;
    w = src_rect->w;
    h = src_rect->h;

    dst_rect->x = ROUND(x * h_mul / h_div);
    dst_rect->y = ROUND(y * v_mul / v_div);
    dst_rect->w = ROUND(w * h_mul / h_div);
    dst_rect->h = ROUND(h * v_mul / v_div);
}

static RET_CODE set_gma_pos_info(struct osd_private *priv)
{
	struct alifbio_move_region_pars pars;
    struct gma_rect src_rect;
    struct gma_rect dst_rect;
    RET_CODE ret = RET_FAILURE;

	memset((void *)&pars, 0, sizeof(pars));

    src_rect.x = priv->region_par[pars.region_id].reg_rect.uLeft;
    src_rect.y = priv->region_par[pars.region_id].reg_rect.uTop;
    src_rect.w = priv->region_par[pars.region_id].reg_rect.uWidth;
    src_rect.h = priv->region_par[pars.region_id].reg_rect.uHeight;
    scale_rect(priv, &src_rect, &dst_rect);
    //OSD_PRF("scale pos <%d %d %d %d> => <%d %d %d %d>\n",
        //src_rect.x, src_rect.y, src_rect.w, src_rect.h,
        //dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h);
    
	pars.pos.x = dst_rect.x;
	pars.pos.y = dst_rect.y;
    pars.pos.w = dst_rect.w;
    pars.pos.h = dst_rect.h;
	if(ioctl(m_osd_file_handle[priv->index], FBIO_MOVE_REGION, &pars) == 0)
	{
		ret = RET_SUCCESS;
	}

    return ret;
}

static int get_bpp(enum OSDColorMode eMode)
{
	int bpp = 1;
	
	switch(eMode)
	{
		case OSD_256_COLOR:
			bpp = 1;
			break;
		case OSD_HD_ARGB1555:
		case OSD_HD_RGB565:
			bpp = 2;
			break;
		case OSD_HD_ARGB8888:
		case OSD_HD_RGB888:
			bpp = 4;
			break;
		default:
			break;
	}

	return bpp;
}

static int get_dis_format(enum OSDColorMode eMode)
{
	int format = DIS_FORMAT_CLUT8;
	
	switch(eMode)
	{
		case OSD_256_COLOR:
			format = DIS_FORMAT_CLUT8;
			break;
		case OSD_HD_ARGB1555:
			format = DIS_FORMAT_ARGB1555;
			break;
		case OSD_HD_RGB565:
			format = DIS_FORMAT_RGB565;
			break;
		case OSD_HD_ARGB8888:
			format = DIS_FORMAT_ARGB8888;
			break;
		default:
			break;
	}

	return format;
}

static void update_virtual_region(HANDLE hDev, int src_reg, int dst_reg, struct OSDRect *rect)
{
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private *priv = (struct osd_private *)dev->priv;
	struct osd_region_par *src_par = &priv->region_par[src_reg];
	struct osd_region_par *dst_par = &priv->region_par[dst_reg];
	struct OSDRect src_rect, dst_rect;

	memcpy((void *)&src_rect, rect, sizeof(src_rect));
	memcpy((void *)&dst_rect, rect, sizeof(dst_rect));

	dst_rect.uLeft = src_rect.uLeft + src_par->reg_rect.uLeft - dst_par->reg_rect.uLeft;
	dst_rect.uTop = src_rect.uTop + src_par->reg_rect.uTop - dst_par->reg_rect.uTop;
	
	rect_cpy(dst_par->reg_rect.uWidth, dst_par->reg_rect.uHeight, dst_par->mem_start
		, src_par->reg_rect.uWidth, src_par->reg_rect.uHeight, src_par->mem_start
		, &dst_rect, &src_rect, dst_par->pitch
		, src_par->pitch, dst_par->bpp);
}

RET_CODE OSDDrv_Open(HANDLE hDev,struct OSDPara*pOpenPara)
{
	INT32 result = RET_SUCCESS;

	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = m_osd_priv[(int)dev->priv];
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif
	struct osd_mem *pmem = NULL;

#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();
#endif

#ifdef ADR_IPC_ENABLE
	if(priv->opened)
#else
	/* If openned already, exit */
	if(dev->flags & HLD_DEV_STATS_UP)
#endif		
	{
		OSD_PRF("osddrv_open: warning - device %s openned already!\n", m_osd_name[priv->index]);
		return RET_SUCCESS;
	}

	MUTEX_LOCK();
	
	ioctl(m_osd_file_handle[priv->index], FBIO_SET_DE_LAYER, (priv->index == 0) ? DE_LAYER2 : DE_LAYER3);

	pmem = m_osd_mem + priv->index;
	memset((void *)pmem, 0, sizeof(*pmem));
	
	{
		struct alifbio_fbinfo_data_pars fbinfo;

		ioctl(m_osd_file_handle[priv->index], FBIO_GET_FBINFO_DATA, &fbinfo);
			
		priv->mem_start = (void *)fbinfo.mem_start;
		priv->mem_size = fbinfo.mem_size;
		priv->max_logical_width = fbinfo.xres_virtual;
		priv->max_logical_height = fbinfo.yres_virtual;
		priv->pitch = fbinfo.line_length;

		pmem->virtual_mem_size = priv->mem_size;
		pmem->virtual_mem_start = mmap(NULL, pmem->virtual_mem_size, PROT_WRITE | PROT_READ
			, MAP_SHARED, m_osd_file_handle[priv->index], 0);
		if(pmem->virtual_mem_start == (void *)(-1))
		{
			MUTEX_UNLOCK();
			
			OSD_PRF("mmap the OSD mem buf fail\n");
			return RET_FAILURE;
		}

		pmem->virtual_mem_start += fbinfo.mem_start & 0x7FF;
		
		OSD_PRF("physical OSD mem buf %x size %x \nvirtual OSD mem buf %x\n"
			, (int)priv->mem_start, (int)priv->mem_size, (int)pmem->virtual_mem_start);
	}
	
	if(pOpenPara != NULL)
	{
		memcpy((void *)&(priv->default_para), (void *)pOpenPara, sizeof(struct OSDPara));
	}
	else
	{
		priv->default_para.eMode = OSD_256_COLOR;
		priv->default_para.uGAlphaEnable = 0;
		priv->default_para.uGAlpha = 0x7F;
		priv->default_para.uPalletteSel = 0;
	}

	priv->scale_mode = OSD_SCALE_DUPLICATE;
	priv->h_div = 1;
	priv->h_mul = 1;
	priv->v_div = 1;
	priv->v_mul = 1;
	priv->show_on = 0;

	OSD_PRF("default emode %d\n", priv->default_para.eMode);
	
	if(priv->default_para.eMode == OSD_256_COLOR)
	{
		pmem->pallete_size = 256;
		pmem->pallete_buf_allocated = (void *)malloc(pmem->pallete_size * 4 + 3);
		if(pmem->pallete_buf_allocated == NULL)
		{
			MUTEX_UNLOCK();

			OSD_PRF("get the pallette buf  fail\n");
			return RET_FAILURE;
		}

		pmem->pallete_buf = (void *)(((UINT32)pmem->pallete_buf_allocated + 3) & ~3);
		memset(pmem->pallete_buf, 0, pmem->pallete_size * 4);

	}

#ifdef ADR_IPC_ENABLE
	priv->opened = 1;
#else
	/* Setup init work mode */
	if (result == RET_SUCCESS)
	{
		dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
	}
#endif

	MUTEX_UNLOCK();

	OSD_PRF("open done\n");
	return result;
}

RET_CODE OSDDrv_Close(HANDLE hDev)
{
	INT32 result = RET_SUCCESS;
	int reg_id = 0;
	
	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = m_osd_priv[(int)dev->priv];
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif
	struct osd_mem *pmem = NULL;

#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		return RET_SUCCESS;
#endif

#ifdef ADR_IPC_ENABLE
	if(priv->opened == 0)
#else
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
#endif		
	{
		OSD_PRF("osddrv_close: warning - device %s closed already!\n", m_osd_name[priv->index]);
		return RET_SUCCESS;		
	}

	MUTEX_LOCK();

#if 0	
	if(priv->show_on)
	{
		priv->show_on = 0;
		ioctl(m_osd_file_handle[priv->index], FBIO_WIN_ONOFF, priv->show_on);	
	}
#endif

	MUTEX_UNLOCK();
	
	for(reg_id = 0;reg_id < OSD_MAX_REG_NUM;reg_id++)
	{
		if(priv->region_par[reg_id].valid)
			OSDDrv_DeleteRegion(hDev, reg_id);
	}

	MUTEX_LOCK();

	pmem = m_osd_mem + priv->index;
	
	if(pmem->virtual_mem_start != NULL)
	{
		munmap(pmem->virtual_mem_start, pmem->virtual_mem_size);
		pmem->virtual_mem_start = NULL;
	}

	if(pmem->pallete_buf_allocated != NULL)
	{
		free(pmem->pallete_buf_allocated);
		pmem->pallete_buf_allocated = NULL;
	}

	MUTEX_UNLOCK();

#ifdef ADR_IPC_ENABLE
	priv->opened = 0;
#else
	/* Update flags */
	dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
#endif

	OSD_PRF("close done\n");	
	return result;
}

RET_CODE OSDDrv_IoCtl(HANDLE hDev,UINT32 dwCmd,UINT32 dwParam)
{
	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = NULL;
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif
	struct osd_mem *pmem = NULL;


#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();

	if(m_osd_priv[(int)dev->priv] == NULL)
		return RET_FAILURE;

	priv = m_osd_priv[(int)dev->priv];
#else
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		OSD_PRF("osd is not opened\n");	
		return RET_FAILURE;
	}
#endif

	MUTEX_LOCK();

	pmem = m_osd_mem + priv->index;
	OSD_PRF("io cmd %x param %d\n", dwCmd, dwParam);
	
	switch(dwCmd)
	{
		case OSD_IO_SET_TRANS_COLOR:
		{
			m_osd_trans_color = (UINT8)dwParam;
			break;
		}
		case OSD_IO_SET_GLOBAL_ALPHA:
		{
			ioctl(m_osd_file_handle[priv->index], FBIO_SET_GLOBAL_ALPHA, (dwParam&0xFF));
			break;
		}
		case OSD_IO_GET_REGION_INFO:
		{
			posd_region_param preg_info = (posd_region_param)dwParam;
			struct osd_region_par *preg_par = NULL;

			preg_par = priv->region_par + preg_info->region_id;
			preg_info->color_format = priv->default_para.eMode;
			preg_info->galpha_enable = priv->default_para.uGAlphaEnable;
			preg_info->global_alpha = priv->default_para.uGAlpha;
			preg_info->region_x = preg_par->reg_rect.uLeft;
			preg_info->region_y = preg_par->reg_rect.uTop;
			preg_info->region_w = preg_par->reg_rect.uWidth;
			preg_info->region_h = preg_par->reg_rect.uHeight;
			preg_info->bitmap_addr = (UINT32)preg_par->mem_start;
			preg_info->phy_addr = (UINT32)(priv->mem_start + (preg_par->mem_start - pmem->virtual_mem_start));
			preg_info->pixel_pitch = preg_par->pitch / preg_par->bpp;
			OSD_PRF("preg_info->bitmap_addr %x w %d h %d phy %x\n", preg_info->bitmap_addr
				, preg_info->region_w, preg_info->region_h, preg_info->phy_addr);
			break;
		}
		case OSD_IO_ADJUST_MEMORY:
		{
#if 0
            		struct fb_var_screeninfo var_info;
            		ioctl(m_osd_file_handle[priv->index], FBIOGET_VSCREENINFO, &var_info);

            		// switch to buffer #0 or #1 on next V-sync
            		if (dwParam == 0)
                		var_info.yoffset = 0;
            		else
                		var_info.yoffset = var_info.yres;
            		var_info.activate = FB_ACTIVATE_VBL;
            		ioctl(m_osd_file_handle[priv->index], FBIOPAN_DISPLAY, &var_info);
            		priv->region_par[0].mem_start = priv->virtual_mem_start + (var_info.yoffset * var_info.xres * (var_info.bits_per_pixel / 8));
#endif
			break;
		}
		case OSD_IO_GET_LAYER_INFO:
		{
			posd_layer_param player_info = (posd_layer_param)dwParam;

			player_info->mode = priv->default_para.eMode;
			player_info->mem_start = priv->mem_start;
			player_info->mem_size = priv->mem_size;
			player_info->virtual_mem_start = pmem->virtual_mem_start;
			player_info->virtual_mem_size = pmem->virtual_mem_size;
			player_info->max_logical_width = priv->max_logical_width;
			player_info->max_logical_height = priv->max_logical_height;
			player_info->pitch = priv->pitch;
			break;
		}
		case OSD_IO_DISABLE_ENHANCE:
		{
			ioctl(m_osd_file_handle[priv->index], FBIO_DISABLE_GMA_ENHANCE, (dwParam&0xFF));
			break;
		}
		case OSD_IO_NO_CLEAR_BUF:
		{
			if(dwParam == TRUE)
				m_osd_no_clear_buf = 1;
			else
				m_osd_no_clear_buf = 0;
			break;
		}
		case OSD_IO_VIDEO_ENHANCE:
		{
			struct ali_fb_video_enhance_pars enhance_pars;
			struct osd_io_video_enhance *out_pars = (struct osd_io_video_enhance *)dwParam;
	
			memset((void *)&enhance_pars, 0, sizeof(enhance_pars));

			switch(out_pars->changed_flag)
			{
				case OSD_SET_ENHANCE_BRIGHTNESS:
					enhance_pars.changed_flag = FBIO_SET_ENHANCE_BRIGHTNESS;
					break;
				case OSD_SET_ENHANCE_CONTRAST:
					enhance_pars.changed_flag = FBIO_SET_ENHANCE_CONTRAST;
					break;
				case OSD_SET_ENHANCE_SATURATION:
					enhance_pars.changed_flag = FBIO_SET_ENHANCE_SATURATION;
					break;
				case OSD_SET_ENHANCE_SHARPNESS:
					enhance_pars.changed_flag = FBIO_SET_ENHANCE_SHARPNESS;
					break;
				case OSD_SET_ENHANCE_HUE:
					enhance_pars.changed_flag = FBIO_SET_ENHANCE_HUE;
					break;
				default:
					goto EXIT;
						
			}
			
			enhance_pars.grade = out_pars->grade;

			OSD_PRF("enhance flag %d grade %d\n", out_pars->changed_flag, out_pars->grade);

			ioctl(m_osd_file_handle[priv->index], FBIO_VIDEO_ENHANCE, (int)&enhance_pars);
			break;
		}
		case OSD_IO_GET_DISPLAY_RECT:
        	{
#if 0
            		struct OSDRect *posd_rect = (struct OSDRect *)dwParam;
            		struct gma_rect ge_rect;
            		RET_CODE ret;
            		if (posd_rect == NULL)
                		return RET_FAILURE;
            		ioctl(m_osd_file_handle[priv->index], FBIO_GET_DISPLAY_RECT, (UINT32)&ge_rect);
            		posd_rect->uLeft = ge_rect.x;
            		posd_rect->uTop = ge_rect.y;
            		posd_rect->uWidth = ge_rect.w;
            		posd_rect->uHeight = ge_rect.h;
#endif
            		break;
        	}
        	case OSD_IO_SET_DISPLAY_RECT:
        	{
#if 0
            		struct OSDRect *posd_rect = (struct OSDRect *)dwParam;
            		struct gma_rect ge_rect;
            		RET_CODE ret;
            		if (posd_rect == NULL)
                		return RET_FAILURE;
            		ge_rect.x= posd_rect->uLeft;
            		ge_rect.y = posd_rect->uTop;
            		ge_rect.w = posd_rect->uWidth;
            		ge_rect.h = posd_rect->uHeight;
            		ioctl(m_osd_file_handle[priv->index], FBIO_SET_DISPLAY_RECT, (unsigned long)&ge_rect);
#endif
            		break;
        	}
		default:
			break;
	}

EXIT:
	MUTEX_UNLOCK();
	
	return RET_SUCCESS;
}

RET_CODE OSDDrv_GetPara(HANDLE hDev,struct OSDPara* ptPara)
{
	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = m_osd_priv[(int)dev->priv];
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif

#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();
#endif

#ifdef ADR_IPC_ENABLE
	if(priv->opened == 0)
#else
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
#endif		
	{
		OSD_PRF("osd is not opened\n");
		return RET_FAILURE;
	}
	
	MUTEX_LOCK();

	memcpy((void *)ptPara, (void *)&(priv->default_para), sizeof(struct OSDPara));

	MUTEX_UNLOCK();
	
	return RET_SUCCESS;
}

RET_CODE OSDDrv_ShowOnOff(HANDLE hDev,UINT8 uOnOff)
{
	struct osd_device *dev = (struct osd_device *)hDev;
        int on = (uOnOff != 0 ? 1 : 0);
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = NULL;
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif

#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();

	if(m_osd_priv[(int)dev->priv] == NULL)
		return RET_FAILURE;

	priv = m_osd_priv[(int)dev->priv];
#else
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		OSD_PRF("osd is not opened\n");	
		return RET_FAILURE;
	}
#endif

	MUTEX_LOCK();
	
	{
		priv->show_on = (on != 0 ? 1 : 0);
		ioctl(m_osd_file_handle[priv->index], FBIO_WIN_ONOFF, on);	

		OSD_PRF("show flag %d\n", uOnOff);
	}

	MUTEX_UNLOCK();
	
	return RET_SUCCESS;
}

RET_CODE OSDDrv_SetPallette(HANDLE hDev,UINT8 *pPallette,UINT16 wColorN,UINT8 bType)
{
	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = m_osd_priv[(int)dev->priv];
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif
	struct alifbio_plt_pars plt_par;
	struct osd_mem *pmem = NULL;
	
#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();
#endif

#ifdef ADR_IPC_ENABLE
	if(priv->opened == 0)
#else
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
#endif		
	{
		OSD_PRF("osd is not opened\n");
		return RET_FAILURE;
	}

	pmem = m_osd_mem + priv->index;	
	
	if((wColorN != 256) || (pPallette == NULL) )
	{
		OSD_PRF("arg fail pallette 0x%08x type %d num %d\n", (int)pPallette, bType, wColorN);
		return RET_FAILURE;
	}

	if(pmem->pallete_buf == NULL)
	{
		OSD_PRF("pallette bufffer is not inited\n");
		return RET_FAILURE;
	}

	MUTEX_LOCK();
	
	if(bType == OSDDRV_YCBCR)
	{
		UINT8 *src = pPallette;
		UINT8 *dst = (UINT8 *)pmem->pallete_buf;
		int i = 0;
				
		for(i = 0;i < pmem->pallete_size;i++)
		{
			*(dst + 2) = *(src + 2);
			*(dst ) = *(src + 1);
			*(dst + 1) = *src;
			*(dst + 3) = ((*(src + 3)<<3) & 0x78) +  ((*(src + 3)>>1) & 0x07);
			
			dst += 4;
			src += 4;
		}
	}
	else
	{
		memcpy((void *)pmem->pallete_buf, pPallette, pmem->pallete_size * 4);
	}

	plt_par.type = (bType == OSDDRV_YCBCR) ? PLT_YUV : PLT_RGB;
	plt_par.level = PLT_ALPHA_256;
	plt_par.pallette_buf = pmem->pallete_buf;
	plt_par.color_num = wColorN;
	
	if(ioctl(m_osd_file_handle[priv->index], FBIO_SET_PALLETTE, &plt_par) == 0)
	{
		MUTEX_UNLOCK();
		
		OSD_PRF("set pallette done\n");
		return RET_SUCCESS;
	}

	MUTEX_UNLOCK();

	OSD_PRF("set pallette fail\n");
	return !RET_SUCCESS;
}

RET_CODE OSDDrv_GetPallette(HANDLE hDev,UINT8 *pPallette,UINT16 wColorN,UINT8 bType)
{
	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = m_osd_priv[(int)dev->priv];
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif
	struct osd_mem *pmem = NULL;

#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();
#endif

#ifdef ADR_IPC_ENABLE
	if(priv->opened == 0)
#else
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
#endif		
	{
		OSD_PRF("osd is not opened\n");
		return RET_FAILURE;
	}

	MUTEX_LOCK();

	pmem = m_osd_mem + priv->index;	
	
	if(bType == OSDDRV_YCBCR)
	{
		UINT8 *src = (UINT8 *)pmem->pallete_buf;
		UINT8 *dst = pPallette;
		int i = 0;

		for(i = 0;i < pmem->pallete_size;i++)
		{
			*dst = *(src + i + 2);
			*(dst + 1) = *(src + i + 1);
			*(dst + 2) = *src;
			*(dst + 3) = (*(src + i + 3)>>3) & 0x0F;
		}
	}
	else
		memcpy(pPallette, (void *)pmem->pallete_buf, pmem->pallete_size * 4);

	MUTEX_UNLOCK();
	
	return RET_SUCCESS;
}

RET_CODE OSDDrv_ModifyPallette(HANDLE hDev,UINT8 uIndex,UINT8 uY,UINT8 uCb,UINT8 uCr,UINT8 uK)
{
	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = m_osd_priv[(int)dev->priv];
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif
	struct alifbio_plt_pars plt_par;	
	struct osd_mem *pmem = NULL;
	
#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();
#endif

#ifdef ADR_IPC_ENABLE
	if(priv->opened == 0)
#else
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
#endif		
	{
		OSD_PRF("osd is not opened\n");	
		return RET_FAILURE;
	}

	MUTEX_LOCK();

	pmem = m_osd_mem + priv->index;	
	
	*(((UINT32 *)pmem->pallete_buf) + uIndex) = (((uK<<3) & 0x78) +  ((uK>>1) & 0x07)<<24) 
		|(uY<<16) | (uCb<<8) | uCr;
	
	plt_par.type = PLT_YUV;
	plt_par.level = PLT_ALPHA_128;
	plt_par.pallette_buf = pmem->pallete_buf;
	plt_par.color_num = pmem->pallete_size;
	ioctl(m_osd_file_handle[priv->index], FBIO_SET_PALLETTE, &plt_par);

	MUTEX_UNLOCK();
	
	return RET_SUCCESS;
}

RET_CODE OSDDrv_CreateRegion(HANDLE hDev,UINT8 uRegionId,struct OSDRect* rect,struct OSDPara*pOpenPara)
{
	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = m_osd_priv[(int)dev->priv];
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif
	struct alifbio_reg_pars reg_par;
	struct osd_region_par *osd_reg_par;
	struct osd_mem *pmem = NULL;	
	int big_rect = 0;
	int ret = 0;

#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();
#endif

#ifdef ADR_IPC_ENABLE
	if(priv->opened == 0)
#else
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
#endif		
	{
		OSD_PRF("osd is not opened\n");	
		return RET_FAILURE;
	}

	if(uRegionId >= OSD_MAX_REG_NUM)
	{
		OSD_PRF("create region id fail %d\n", uRegionId);
		return RET_FAILURE;
	}

	MUTEX_LOCK();
		
	if(priv->region_par[uRegionId].valid != 0)
	{
		MUTEX_UNLOCK();
		
		OSD_PRF("this region %d has been created\n", uRegionId);
		return RET_FAILURE;
	}

	if((priv->default_para.eMode != OSD_256_COLOR) 
		&& (priv->default_para.eMode != OSD_HD_ARGB1555)
		&& (priv->default_para.eMode != OSD_HD_RGB565)
		&& (priv->default_para.eMode != OSD_HD_ARGB8888))
	{
		MUTEX_UNLOCK();
		
		OSD_PRF("don't support the color format %d\n"
			, priv->default_para.eMode);
		return RET_FAILURE;
	}


	osd_reg_par = priv->region_par + uRegionId;
	
	if((rect->uWidth > priv->max_logical_width) 
		|| (rect->uHeight > priv->max_logical_height))
	{
		UINT32 size = 0;
		
		size = (UINT32)rect->uWidth * (UINT32)rect->uHeight * osd_reg_par->bpp;
	
		if(size > (UINT32)priv->mem_size)
		{	
			MUTEX_UNLOCK();
			
			OSD_PRF("don't support rect size w %d h %d max w %d h %d\n"
				, rect->uWidth, rect->uHeight, priv->max_logical_width
				, priv->max_logical_height);
			return RET_FAILURE;
		}		

		big_rect = 1;
	}

	pmem = m_osd_mem + priv->index;
	
	memcpy((void *)&(osd_reg_par->reg_rect), rect, sizeof(*rect));
	osd_reg_par->bpp = get_bpp(priv->default_para.eMode);
	osd_reg_par->pitch = osd_reg_par->bpp * rect->uWidth;
	osd_reg_par->mem_size = osd_reg_par->pitch * rect->uHeight;
	if(uRegionId == 0)
	{
		if(big_rect == 1)
			osd_reg_par->mem_start = pmem->virtual_mem_start;
		else
			osd_reg_par->mem_start = pmem->virtual_mem_start + rect->uTop * priv->pitch;
	}
	else
	{
		if(priv->region_par[0].valid == 0)
		{
			MUTEX_UNLOCK();
		
			OSD_PRF("the user should carete the region 0 firstly\n");
			return RET_FAILURE;
		}

		if(rect->uLeft < priv->region_par[0].reg_rect.uLeft
			|| rect->uTop < priv->region_par[0].reg_rect.uTop
			|| (rect->uLeft + rect->uWidth)
				> (priv->region_par[0].reg_rect.uLeft + priv->region_par[0].reg_rect.uWidth)
			|| (rect->uTop + rect->uHeight)
				> (priv->region_par[0].reg_rect.uTop+ priv->region_par[0].reg_rect.uHeight))
		{
			MUTEX_UNLOCK();
		
			OSD_PRF("region %d rect fail not in the first region\n", uRegionId);
			return RET_FAILURE;
		}
		
		osd_reg_par->mem_start = malloc(osd_reg_par->mem_size);
		if(osd_reg_par->mem_start == NULL)
		{
			MUTEX_UNLOCK();
		
			OSD_PRF("malloc the osd virtual buf fail id %d\n", uRegionId);
			return RET_FAILURE;
		}
	}
	
	if(uRegionId == 0)
	{
		reg_par.index = uRegionId;
		reg_par.rect.x = rect->uLeft;
		reg_par.rect.y = rect->uTop;
		reg_par.rect.w = rect->uWidth;
		reg_par.rect.h = rect->uHeight;
		
		if(big_rect == 1)
			reg_par.mem_start = (void *)(priv->mem_start);
		else
			reg_par.mem_start = (void *)(priv->mem_start + rect->uTop * priv->pitch);
		
		reg_par.mem_size = osd_reg_par->mem_size;
		reg_par.pitch = osd_reg_par->pitch;
		reg_par.dis_format = get_dis_format(priv->default_para.eMode);

		OSD_PRF("reg mem start 0x%08x virtual 0x%08x top %d pitch %d format %d\n", reg_par.mem_start
			, osd_reg_par->mem_start, rect->uTop, osd_reg_par->pitch, reg_par.dis_format);
	
		ret = ioctl(m_osd_file_handle[priv->index], FBIO_CREATE_GMA_REGION, &reg_par);
		if(ret >= 0)		
        {	
			osd_reg_par->valid = 1;
			
			if(ret == 0)
			{
			    set_gma_scale_info(priv, GE_SET_SCALE_PARAM);
                set_gma_pos_info(priv);
			}
			
			{		
				struct OSDRect write_rect;

				if(priv->default_para.eMode != OSD_256_COLOR)
					m_osd_trans_color = 0;

				if(m_osd_no_clear_buf == 0)
				{
					write_rect.uLeft = 0;
					write_rect.uTop = 0;
					write_rect.uWidth = rect->uWidth;
					write_rect.uHeight = rect->uHeight;
					rect_set(osd_reg_par->reg_rect.uWidth, osd_reg_par->reg_rect.uHeight, osd_reg_par->mem_start
						,&write_rect,m_osd_trans_color, osd_reg_par->pitch, osd_reg_par->bpp);		
				}
			}

			MUTEX_UNLOCK();			
		
			OSD_PRF("create region ok %d x %d y %d w %d h %d\n", uRegionId
				, rect->uLeft, rect->uTop, rect->uWidth, rect->uHeight);
			return RET_SUCCESS;
		}
	}
	else
	{
		osd_reg_par->valid = 1;	
		
		{		
			struct OSDRect write_rect;

			write_rect.uLeft = 0;
			write_rect.uTop = 0;
			write_rect.uWidth = rect->uWidth;
			write_rect.uHeight = rect->uHeight;
			rect_set(osd_reg_par->reg_rect.uWidth, osd_reg_par->reg_rect.uHeight, osd_reg_par->mem_start
				,&write_rect,m_osd_trans_color, osd_reg_par->pitch, osd_reg_par->bpp);		

			update_virtual_region(hDev, uRegionId, 0, &write_rect);
		}		
		
		MUTEX_UNLOCK();
		
		OSD_PRF("create region ok %d x %d y %d w %d h %d\n", uRegionId
			, rect->uLeft, rect->uTop, rect->uWidth, rect->uHeight);
		return RET_SUCCESS;
	}

	MUTEX_UNLOCK();
		
	OSD_PRF("create region fail\n");
	return !RET_SUCCESS;
}

RET_CODE OSDDrv_DeleteRegion(HANDLE hDev,UINT8 uRegionId)
{
	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = m_osd_priv[(int)dev->priv];
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif
	struct osd_region_par *osd_reg_par;
	int reg_id = uRegionId;

#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();
#endif

#ifdef ADR_IPC_ENABLE
	if(priv->opened == 0)
#else
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
#endif		
	{
		OSD_PRF("osd is not opened\n");	
		return RET_FAILURE;
	}

	MUTEX_LOCK();
	
	if(priv->region_par[uRegionId].valid == 0)
	{
		MUTEX_UNLOCK();
				
		OSD_PRF("delete none exist region %d\n", uRegionId);
		return RET_FAILURE;
	}

	if(uRegionId == 0)
	{
		if(ioctl(m_osd_file_handle[priv->index], FBIO_DELETE_GMA_REGION, reg_id) == 0)
		{
			priv->region_par[uRegionId].valid = 0;
			
			MUTEX_UNLOCK();
		
			OSD_PRF("delete region ok %d\n", uRegionId);
			return RET_SUCCESS;
		}
	}
	else
	{
		priv->region_par[uRegionId].valid = 0;

		MUTEX_UNLOCK();
		
		OSD_PRF("delete region ok %d\n", uRegionId);
		return RET_SUCCESS;
	}

	MUTEX_UNLOCK();
		
	OSD_PRF("delete region fail %d\n", uRegionId);
	return !RET_SUCCESS;
}

RET_CODE OSDDrv_SetRegionPos(HANDLE hDev,UINT8 uRegionId,struct OSDRect* rect)
{
	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = m_osd_priv[(int)dev->priv];
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif
	RET_CODE ret = RET_FAILURE;		

#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();
#endif

#ifdef ADR_IPC_ENABLE
	if(priv->opened == 0)
#else
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
#endif		
	{
		OSD_PRF("osd is not opened\n");	
		
		return RET_FAILURE;
	}

	if(uRegionId != 0)
		return RET_FAILURE;

#if 0	
	if(priv->region_par[uRegionId].valid == 0)
	{
		OSD_PRF("set region pos fail \n");
		return RET_FAILURE;
	}
#endif

	OSD_PRF("set region pos x %d y %d w %d h %d\n", rect->uLeft
		, rect->uTop, rect->uWidth, rect->uHeight);

	MUTEX_LOCK();
	
	// if(memcmp(rect, &(priv->region_par[uRegionId].reg_rect), sizeof(*rect)) != 0)
	if((rect->uLeft >= 0) && (rect->uTop >= 0))
	{		
		RET_CODE ret = RET_FAILURE;		
#if 0 //poe modify 因为上面有uRegionId != 0就return,所以下面没必要判断uRegionId == 0
		if(uRegionId == 0)
		{
			struct alifbio_move_region_pars pars;

			memset((void *)&pars, 0, sizeof(pars));
			pars.region_id = uRegionId;
			pars.pos.x = rect->uLeft;
			pars.pos.y = rect->uTop;
			if(ioctl(m_osd_file_handle[priv->index], FBIO_MOVE_REGION, &pars) == 0)
			{
				memcpy((void *)&(priv->region_par[uRegionId].reg_rect), (void *)rect,
					sizeof(*rect));
				ret = RET_SUCCESS;
			}
		}
		else
		{
			memcpy((void *)&(priv->region_par[uRegionId].reg_rect), (void *)rect,
				sizeof(*rect));
			ret = RET_SUCCESS;			
		}
#else
			struct alifbio_move_region_pars pars;

			memset((void *)&pars, 0, sizeof(pars));
			pars.region_id = uRegionId;
			pars.pos.x = rect->uLeft;
			pars.pos.y = rect->uTop;
            pars.pos.w = rect->uWidth;
			pars.pos.h = rect->uHeight;
			if(ioctl(m_osd_file_handle[priv->index], FBIO_MOVE_REGION, &pars) == 0)
			{
				memcpy((void *)&(priv->region_par[uRegionId].reg_rect), (void *)rect,
					sizeof(*rect));
				ret = RET_SUCCESS;
			}

#endif
		MUTEX_UNLOCK();

		if(ret == RET_SUCCESS)
			OSD_PRF("set region pos done\n");
		else
			OSD_PRF("set region pos fail ret %d\n", ret);
		
		return ret;
	}
	else
		OSD_PRF("para fail left %d top %d\n", rect->uLeft, rect->uTop);

	MUTEX_UNLOCK();
		
	return ret;
}

RET_CODE OSDDrv_GetRegionPos(HANDLE hDev,UINT8 uRegionId,struct OSDRect* rect)
{
	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = m_osd_priv[(int)dev->priv];
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif

#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();
#endif

#ifdef ADR_IPC_ENABLE
	if(priv->opened == 0)
#else
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
#endif		
	{
		OSD_PRF("osd is not opened\n");	
		return RET_FAILURE;
	}

#if 0	
	if(priv->region_par[uRegionId].valid == 0)
	{
		OSD_PRF("get region pos fail \n");
		return RET_FAILURE;
	}
#endif

	MUTEX_LOCK();

	memcpy(rect, &(priv->region_par[uRegionId].reg_rect), sizeof(*rect));

	MUTEX_UNLOCK();
		
	return RET_SUCCESS;
}


RET_CODE OSDDrv_RegionShow(HANDLE hDev,UINT8 uRegionId,BOOL bOn)
{
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private *priv = (struct osd_private *)dev->priv;

//#ifdef GE_SIMULATE_OSD
#if 0  //christian
    libc_printf("GE OSDDrv_RegionShow()!\n");
    GE_SIMU_MUTEX_LOCK();	
    RET_CODE ret = ge_gma_show_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, bOn);
    GE_SIMU_MUTEX_UNLOCK();		
    return ret;
#endif
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	return !RET_SUCCESS;
}

RET_CODE OSDDrv_RegionWrite(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct OSDRect *rect)
{
	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = m_osd_priv[(int)dev->priv];
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif
	struct osd_region_par *osd_reg_par = &priv->region_par[uRegionId];
	
#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();
#endif

#ifdef ADR_IPC_ENABLE
	if(priv->opened == 0)
#else
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
#endif		
	{
		OSD_PRF("osd is not opened\n");	
		return RET_FAILURE;
	}

	MUTEX_LOCK();
	
	if(osd_reg_par->valid == 0)
	{	
		MUTEX_UNLOCK();

		OSD_PRF("region is not created\n");
		return RET_FAILURE;
	}

	OSD_PRF("region write x %d y %d w %d h %d\n", rect->uLeft, rect->uTop
		, rect->uWidth, rect->uHeight);
	{
		struct OSDRect src, dst;
			
		src.uLeft = rect->uLeft - pVscr->vR.uLeft;
		src.uTop = rect->uTop - pVscr->vR.uTop;
		src.uWidth = rect->uWidth;
		src.uHeight = rect->uHeight;
		dst.uLeft = rect->uLeft;
		dst.uTop = rect->uTop;
		dst.uWidth = rect->uWidth;
		dst.uHeight = rect->uHeight;	
		rect_cpy(osd_reg_par->reg_rect.uWidth, osd_reg_par->reg_rect.uHeight, osd_reg_par->mem_start
			, pVscr->vR.uWidth, pVscr->vR.uHeight, pVscr->lpbScr
			, &dst, &src, osd_reg_par->pitch
			, osd_reg_par->bpp * pVscr->vR.uWidth, osd_reg_par->bpp);	

		if(uRegionId != 0)
			update_virtual_region(hDev, uRegionId, 0, &dst);		
	}

	MUTEX_UNLOCK();
		
	return  RET_SUCCESS;
}

RET_CODE OSDDrv_RegionRead(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct OSDRect *rect)
{
	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = m_osd_priv[(int)dev->priv];
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif
	struct osd_region_par *osd_reg_par = &priv->region_par[uRegionId];

#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();
#endif

#ifdef ADR_IPC_ENABLE
	if(priv->opened == 0)
#else
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
#endif		
	{
		OSD_PRF("osd is not opened\n");	
		return RET_FAILURE;
	}

	MUTEX_LOCK();
	
	if(osd_reg_par->valid == 0)
	{
		MUTEX_UNLOCK();

	
		OSD_PRF("region is not created\n");
		return RET_FAILURE;
	}
	
	OSD_PRF("region read x %d y %d w %d h %d\n", rect->uLeft, rect->uTop
		, rect->uWidth, rect->uHeight);
	
	{
		struct OSDRect src, dst;	
		
		src.uLeft = rect->uLeft;
		src.uTop = rect->uTop;
		src.uWidth = rect->uWidth;
		src.uHeight = rect->uHeight;
		dst.uLeft = rect->uLeft - pVscr->vR.uLeft;
		dst.uTop = rect->uTop - pVscr->vR.uTop;
		dst.uWidth = rect->uWidth;
		dst.uHeight = rect->uHeight;	

		rect_cpy(pVscr->vR.uWidth,pVscr->vR.uHeight,pVscr->lpbScr,
			osd_reg_par->reg_rect.uWidth, osd_reg_par->reg_rect.uHeight, osd_reg_par->mem_start
			,&dst,&src, osd_reg_par->bpp * pVscr->vR.uWidth, osd_reg_par->pitch, osd_reg_par->bpp);	
	}

	MUTEX_UNLOCK();
	
	return  RET_SUCCESS;
}

RET_CODE OSDDrv_RegionFill(HANDLE hDev,UINT8 uRegionId,struct OSDRect *rect, UINT32 uColorData)
{
	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = m_osd_priv[(int)dev->priv];
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif
	struct osd_region_par *osd_reg_par = &priv->region_par[uRegionId];

#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();
#endif

#ifdef ADR_IPC_ENABLE
	if(priv->opened == 0)
#else
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
#endif		
	{
		OSD_PRF("osd is not opened\n");	
		return RET_FAILURE;
	}

	MUTEX_LOCK();

	if(osd_reg_par->valid == 0)
	{
		MUTEX_UNLOCK();
	
		OSD_PRF("region is not created\n");
		return RET_FAILURE;
	}
	
	{	
		rect_set(osd_reg_par->reg_rect.uWidth, osd_reg_par->reg_rect.uHeight, osd_reg_par->mem_start
			,rect,uColorData, osd_reg_par->pitch, osd_reg_par->bpp);	

		if(uRegionId != 0)		
			update_virtual_region(hDev, uRegionId, 0, rect);			
	}

	MUTEX_UNLOCK();

	return RET_SUCCESS;
}

RET_CODE OSDDrv_DrawHorLine(HANDLE hDev, UINT8 uRegionId, UINT32 x, UINT32 y, UINT32 width, UINT32 color)
{
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private *priv = (struct osd_private *)dev->priv;

#ifdef GE_SIMULATE_OSD
    libc_printf("GE OSDDrv_DrawHorLine()!\n");
    ge_cmd_list_hdl cmd_list = m_cmd_list;
    GE_SIMU_MUTEX_LOCK();

    ge_lock(m_osddrv_ge_dev);

    ge_cmd_list_new(m_osddrv_ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);

    RET_CODE ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
	ge_unlock(m_osddrv_ge_dev);    
	
	GE_SIMU_MUTEX_UNLOCK();				
        return ret;
    }

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_FILL_RECT_DRAW_COLOR);
    ge_set_draw_color(m_osddrv_ge_dev, cmd_hdl, color);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST, width, 1);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, x, y);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);

    ge_unlock(m_osddrv_ge_dev);

    GE_SIMU_MUTEX_UNLOCK();

    return RET_SUCCESS;
#endif
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	return !RET_SUCCESS;
}


RET_CODE OSDDrv_Scale(HANDLE hDev,UINT32 uScaleCmd,UINT32 uScaleParam)
{
	struct osd_device *dev = (struct osd_device *)hDev;
#ifdef ADR_IPC_ENABLE
	struct osd_private *priv = NULL;
#else	
	struct osd_private *priv = (struct osd_private *)dev->priv;
#endif
	struct alifbio_gma_scale_info_pars pars;
	enum OSDSys eOSDSys;	

#ifdef ADR_IPC_ENABLE
	if(m_osd_attached == 0)
		HLD_OSDDrv_Attach();

	if(m_osd_priv[(int)dev->priv] == NULL)
		return RET_FAILURE;

	priv = m_osd_priv[(int)dev->priv];
#else
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		OSD_PRF("osd is not opened\n");	
		return RET_FAILURE;
	}
#endif

	OSD_PRF("osd scale command %d\n", uScaleCmd);

	MUTEX_LOCK();
	
	switch(uScaleCmd)
	{
		case OSD_VSCALE_OFF:
		{
			priv->h_div = 1;
			priv->h_mul = 1;
			priv->v_div = 1;
			priv->v_mul = 1;
            uScaleCmd = GE_SET_SCALE_PARAM;
			break;
		}	
		case OSD_VSCALE_TTX_SUBT:
		{
			eOSDSys = *((enum OSDSys*)uScaleParam);				
			if(eOSDSys == OSD_PAL)
			{
				priv->h_div = 1;
				priv->h_mul = 1;
				priv->v_div = 1;
				priv->v_mul = 1;				
			}			
			else if(eOSDSys == OSD_NTSC)
			{
				priv->h_div = 1;
				priv->h_mul = 1;
				priv->v_div = 576;
				priv->v_mul = 480;
			}
            uScaleCmd = GE_SET_SCALE_PARAM;
			break;
		}
		case OSD_VSCALE_GAME:
		{
			eOSDSys = *((enum OSDSys*)uScaleParam);				
			if(eOSDSys == OSD_PAL)
			{
				priv->h_div = 1;
				priv->h_mul = 1;
				priv->v_div = 1;
				priv->v_mul = 2;	
			}
			else
			{
				priv->h_div = 1;
				priv->h_mul = 1;
				priv->v_div = 288;
				priv->v_mul = 480;				
			}
            uScaleCmd = GE_SET_SCALE_PARAM;
			break;
		}
		case OSD_VSCALE_DVIEW:
		{
            uScaleCmd = GE_SET_SCALE_PARAM;
			break;
		}
		case OSD_OUTPUT_1080:
		{
			if(uScaleParam == OSD_SOURCE_PAL) // OSD orignal size is 720*576
				priv->v_div = 576;
			else // OSD orignal size is 720*480
				priv->v_div = 480;
			priv->v_mul = 1080;
			priv->h_div = 720;
			priv->h_mul = 1920;		
            uScaleCmd = GE_SET_SCALE_PARAM;
			break;
		}
		case OSD_OUTPUT_720:
		{
			if(uScaleParam == OSD_SOURCE_PAL) // OSD orignal size is 720*576
				priv->v_div = 576;
			else // OSD orignal size is 720*480
				priv->v_div = 480;
			priv->v_mul = 720;
			priv->h_div = 720;
			priv->h_mul = 1280;	
            uScaleCmd = GE_SET_SCALE_PARAM;
			break;
		}
		case OSD_SET_SCALE_MODE:
		{
			priv->scale_mode = OSD_SCALE_DUPLICATE;//(int)uScaleParam;
			uScaleCmd = GE_SET_SCALE_MODE;
			break;
		}
		case OSD_SCALE_WITH_PARAM:
		{
			osd_scale_param *pscale_para;
			
			pscale_para = (osd_scale_param *)uScaleParam;
			priv->h_div = pscale_para->h_div;
			priv->h_mul = pscale_para->h_mul;
			priv->v_div = pscale_para->v_div;
			priv->v_mul = pscale_para->v_mul;
            uScaleCmd = GE_SET_SCALE_PARAM;
			OSD_PRF("scale param h_div %d h_mul %d v_div %d v_mul %d\n"
				, priv->h_div, priv->h_mul, priv->v_div, priv->v_mul);
			break;
		}
		default:
			break;
	}

	set_gma_scale_info(priv, uScaleCmd);
	
	/*marked by Andreaw.Tang 2014/10/24
	/Android will set x,y position by OSDDrv_SetRegionPos
	/do not need recaculate x,y by set_gma_pos_info
	*/
        //  set_gma_pos_info(priv);

	MUTEX_UNLOCK();
	
	return  RET_SUCCESS;
}


RET_CODE OSDDrv_SetClip(HANDLE hDev,enum CLIPMode clipmode,struct OSDRect *pRect)
{
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private *priv = (struct osd_private *)dev->priv;

#ifdef GE_SIMULATE_OSD
    libc_printf("GE OSDDrv_SetClip()!\n");
    enum GE_CLIP_MODE ge_clip_mode;
	GE_SIMU_MUTEX_LOCK();

    if (clipmode == CLIP_INSIDE_RECT)
        ge_clip_mode = GE_CLIP_INSIDE;
    else if (clipmode == CLIP_OUTSIDE_RECT)
        ge_clip_mode = GE_CLIP_OUTSIDE;
    else
        ge_clip_mode = GE_CLIP_DISABLE;

    ge_gma_set_region_clip_rect(m_osddrv_ge_dev, GMA_SW_LAYER_ID,
        pRect->uLeft, pRect->uTop, pRect->uWidth, pRect->uHeight);

    RET_CODE ret = ge_gma_set_region_clip_mode(m_osddrv_ge_dev, GMA_SW_LAYER_ID, ge_clip_mode);
	GE_SIMU_MUTEX_UNLOCK();
	return ret;
#endif
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	return !RET_SUCCESS;
}

RET_CODE OSDDrv_ClearClip(HANDLE hDev)
{
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private *priv = (struct osd_private *)dev->priv;

#ifdef GE_SIMULATE_OSD
    libc_printf("GE OSDDrv_ClearClip()!\n");
	GE_SIMU_MUTEX_LOCK();
       RET_CODE ret = ge_gma_set_region_clip_mode(m_osddrv_ge_dev, GMA_SW_LAYER_ID, GE_CLIP_DISABLE);
	GE_SIMU_MUTEX_UNLOCK();
	return ret;
#endif
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	return  !RET_SUCCESS;
}

RET_CODE OSDDrv_GetRegionAddr(HANDLE hDev,UINT8 region_idx,UINT16 y, UINT32 *addr)
{
	struct osd_device *dev = (struct osd_device *)hDev;
	struct osd_private *priv = (struct osd_private *)dev->priv;

#ifdef GE_SIMULATE_OSD
    libc_printf("GE OSDDrv_GetRegionAddr()!\n");
    ge_gma_region_t region_param;
    GE_SIMU_MUTEX_LOCK();

    RET_CODE ret = ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_idx, &region_param);

    UINT32 base_addr = region_param.bitmap_addr;
    base_addr += y * osddrv_get_pitch_by_ge_format(region_param.color_format, region_param.pixel_pitch); // todo: calc according to color_format

    *addr = base_addr;
    GE_SIMU_MUTEX_UNLOCK();

    return ret;
#endif
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}

#ifdef GE_DRAW_OSD_LIB //frank add
   *addr = (UINT32*)ge_osd_get_display(0);
#endif


	return  !RET_SUCCESS;
}

static int b_dfb_init_over = 0;
int HLD_OSDDrv_get_dfb_init_status()
{
#ifdef DIRECT_FB_OSD_SUPPORT
	return b_dfb_init_over;
#else
	return 1;
#endif
}

void HLD_OSDDrv_Attach(void)
{
	int i = 0;
#ifdef ADR_IPC_ENABLE	
	int shmid = 0;
#endif

	if(m_osd_attached != 0)
		return;
	
	//OSD_PRF("enter %s\n", __FUNCTION__);	
	memset((void *)&m_osd_dev, 0, sizeof(struct osd_device *) * OSD_MAX_DEV_NUM);
	memset((void *)&m_osd_priv, 0, sizeof(struct osd_private *) * OSD_MAX_DEV_NUM);

#ifdef ADR_IPC_ENABLE
	m_osd_mutex_id = adr_ipc_semget(ADR_IPC_OSD, 0, 1);
	if(m_osd_mutex_id < 0)
	{
		//OSD_PRF("create mutex fail\n");
		return;
	}
#endif
	b_dfb_init_over = 1;

	for(i = 0;i < OSD_MAX_DEV_NUM;i++)
	{
		struct osd_device *dev;
		struct osd_private *priv;

		dev = dev_alloc(m_osd_name[i],HLD_DEV_TYPE_OSD,sizeof(struct osd_device));
		if(dev == NULL)
		{
			OSD_PRF("malloc osd dev fail\n");
			return;
		}

#ifdef ADR_IPC_ENABLE
		MUTEX_LOCK();

		dev->flags = 0;

		if(shmid = adr_ipc_shmget(ADR_IPC_OSD, i, (void **)&priv, sizeof(*priv)) < 0)
		{
			MUTEX_UNLOCK();
			
			OSD_PRF("get shm fail\n");
			return;			
		}

		OSD_PRF("get the shmd %d\n", shmid);
		
		// if(priv->inited == 0)
		{
			memset((void *)priv, 0, sizeof(*priv));
			
			priv->index = i;
			
			priv->shmid = shmid;

#ifdef HLD_DBG_ENABLE
			priv->dbg_on = 1;// just for debug
#else
			priv->dbg_on = 0;// just for debug
#endif

			priv->inited = 1;

			dev->priv = (void *)i;
			OSD_PRF("init shm done\n");
		}
		
		priv->file_path = (i == 0) ? fb0_path : fb2_path;
		
		m_osd_priv[i] = priv;
		p_osd_dbg_on = &(priv->dbg_on);

		MUTEX_UNLOCK();			
#else
		priv = malloc(sizeof(*priv));
		if(priv == NULL)
		{
			OSD_PRF("malloc osd priv fail\n");
		}

		memset((void *)priv, 0, sizeof(*priv));
		priv->file_path = (i == 0) ? fb0_path : fb2_path;
		priv->index = i;
		dev->priv = (void *)priv;
		dev->next = NULL;
		dev->flags = 0;
		strcpy(dev->name, m_osd_name[i]);
#endif

		m_osd_file_handle[i] = open((i == 0) ? fb0_path : fb2_path, O_RDWR | O_SYNC | O_CLOEXEC);
		if(m_osd_file_handle[i] <= 0)
		{
			OSD_PRF("open file fail %s\n", (i == 0) ? fb0_path : fb2_path);
			return;
		}		
	
		OSD_PRF("file path %s\n", priv->file_path);
		
		if(dev_register(dev) != RET_SUCCESS)
		{
			OSD_PRF("register osd dev fail\n");
			return;
		}
				
		m_osd_dev[i] = dev;
		OSD_PRF("attach hld osd dev %d ok\n", i);
	}

	memset((void *)m_osd_mem, 0, sizeof(struct osd_mem) * OSD_MAX_DEV_NUM);
	
	fbdbg_register();
	
	m_osd_attached = 1;
}

void HLD_OSDDrv_Dettach(void)
{
#ifdef ADR_IPC_ENABLE

#else
	struct osd_private *priv;	
#endif
	int i = 0;

	if(m_osd_attached == 0)
		return;

	//OSD_PRF("enter %s\n", __FUNCTION__);
	for(i = 0;i < OSD_MAX_DEV_NUM;i++)
	{
		if(m_osd_dev[i] != 0)
		{
			OSDDrv_Close((HANDLE)m_osd_dev[i]);

#ifdef ADR_IPC_ENABLE

#else
			priv = (struct osd_private *)m_osd_dev[i]->priv;
			if(priv != NULL)
				free(priv);
#endif
			
			dev_free(m_osd_dev[i]);
		}

		
		close(m_osd_file_handle[i]);		
		
		m_osd_dev[i] = NULL;
		// OSD_PRF("dettach hld osd dev %d ok\n", i);
	}	
	m_osd_attached = 0;
}

