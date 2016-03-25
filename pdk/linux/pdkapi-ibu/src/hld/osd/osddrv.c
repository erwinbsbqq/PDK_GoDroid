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
 *  1.  2010.03.11  Sam         4.0     Support Linux Driver
 ****************************************************************************/
 
#include <basic_types.h>
#include <mediatypes.h> 
#include <osal/osal.h>
#include <sys_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include <hld/hld_dev.h>
#include <hld/osd/osddrv.h>
#include <hld/ge/ge.h>
#include <hld/dis/vpo.h>
#include <hld/osd/gfxdrv.h>
#include <ali_video_common.h>
#include <ali_decv_plugin_common.h>
#include <alidefinition/adf_gma.h>

#ifdef _ALI_OPENVG_ENABLE_
#include <ALiGE_API_common.h>
#endif

#ifdef GE_SIMULATE_OSD
  //current DO not used in Linux system
  #undef GE_SIMULATE_OSD
#endif

#ifdef HLD_DBG_ENABLE
#define OSD_PRF(arg, value...)  \
        {\
            printf("[osd]");\
            printf(arg, ##value);\
        }
#else
#define OSD_PRF(...)    do{}while(0)
#endif

#if 0//def ADR_IPC_ENABLE
#include <hld/misc/adr_ipc.h>
#define MUTEX_LOCK()	adr_ipc_semlock(m_osd_mutex_id)
#define MUTEX_UNLOCK()  adr_ipc_semunlock(m_osd_mutex_id)
#else
#define MUTEX_LOCK()	do{}while(0)
#define MUTEX_UNLOCK()  do{}while(0)
#endif

#define OSD_MAX_DEV_NUM     2
#define OSD_MAX_REG_NUM     6

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

    void *pallete_buf_allocated;
    void *pallete_buf;
    int pallete_size;

    /* phy address of the OSD memory */
    void *mem_start;
    int mem_size;
    
    /* direct entry to the the OSD memory */
    void *virtual_mem_start;
    int virtual_mem_size;

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
};

static char m_osd_name[OSD_MAX_DEV_NUM][HLD_MAX_NAME_SIZE] = {"OSD_S36_0", "OSD_S36_1"};
static struct osd_device *m_osd_dev[OSD_MAX_DEV_NUM];
static struct osd_private *m_osd_priv[OSD_MAX_DEV_NUM];

static int m_cur_handle = 0;
static UINT8 m_osd_trans_color = 0;

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

#ifdef KERNEL_GMA_TEST
static enum GE_GMA_PIXEL_FORMAT osddrv_color_mode_to_ge(UINT8 bColorMode)
{
    enum GE_GMA_PIXEL_FORMAT color_format = GE_GMA_PF_CLUT8;

    switch (bColorMode)
    {
        case OSD_256_COLOR:
            color_format = GE_GMA_PF_CLUT8;
            break;
        case OSD_HD_ARGB1555:
        color_format     = GE_GMA_PF_ARGB1555;
            break;
        case OSD_HD_ARGB4444:
            color_format = GE_GMA_PF_ARGB4444;
            break;
        case OSD_HD_AYCbCr8888:
        case OSD_HD_ARGB8888:
            color_format = GE_GMA_PF_ARGB8888;
            break;
        default:
            break;
    }
   
    return color_format;
}
#endif

#ifdef GE_DRAW_OSD_LIB

#include <hld/ge/ge.h>

//#define GMA_SW_LAYER_ID ((UINT32)dev->priv)
#define GMA_SW_LAYER_ID ((UINT32)dev->flags)
#define ADDR_MAP_CNT        10

#define GE_SIMU_MUTEX_LOCK()  osal_mutex_lock(dev->sema_opert_osd, OSAL_WAIT_FOREVER_TIME)
#define GE_SIMU_MUTEX_UNLOCK()  osal_mutex_unlock(dev->sema_opert_osd)
#define m_cmd_list      m_osd_cmd_list[GMA_SW_LAYER_ID]

typedef struct{
    UINT8 *phy_addr;
    UINT8 *virtual_addr;
}ADDR_MAP_t;

static ADDR_MAP_t m_fb_addr_map[ADDR_MAP_CNT];
static UINT8 m_fb_addr_map_idx = -1;


static struct ge_device *m_osddrv_ge_dev = NULL;
static ge_cmd_list_hdl  m_osd_cmd_list[2];

static void *ge_osd;
static struct fb_fix_screeninfo osd_fb_fix;
static void *ge_fb_display = NULL;
static void *ge_fb_p_display = NULL;

//the second Fb, here we used for virtual screen.
static void *ge_fb_display2 = NULL;
//the physical addr map to ge_fb_display2;
static void *ge_fb_p_display2 = NULL;


BOOL ge_osd_init(struct osd_device *dev)
{
    struct osd_private *priv = (struct osd_private *)dev->priv;
    int fb_hld;

    if ((fb_hld=open(priv->file_path, O_RDWR)) <= 0)
    {
        printf("open %s dev failed\n", priv->file_path);
        return FALSE;
    }

    ge_osd_fb_mem_map_int();

    priv->handle = fb_hld;
    if(ioctl(fb_hld, FBIOGET_FSCREENINFO, &osd_fb_fix)) 
    {
        printf("Err: fixscreeninfo ioctl\n");
        return FALSE;
    }

    ge_fb_p_display = (void*)osd_fb_fix.smem_start;

    ge_fb_p_display2 = (void*) ((UINT32)ge_fb_p_display + (osd_fb_fix.smem_len >> 1));

    
    //map for GE display(like virtual screen)
    if ((ge_fb_display = mmap(NULL, osd_fb_fix.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fb_hld, 0)) == (void *)-1)
    {
        printf("Err: mmap ge failed\n");
        return FALSE;
    }
    ge_fb_display2 = (void*)((UINT32)ge_fb_display + (osd_fb_fix.smem_len >> 1));

    ge_osd_fb_add_mem_map(ge_fb_p_display, ge_fb_display);
    ge_osd_fb_add_mem_map(ge_fb_p_display2, ge_fb_display2);
    
    if ((ge_osd = mmap(MAP_GE_BASE_ADDRESS_START, osd_fb_fix.mmio_len, PROT_READ|PROT_WRITE, MAP_SHARED, fb_hld, osd_fb_fix.smem_len)) == (void *)-1)
    {
        printf("Err: mmap ge failed\n");
        return FALSE;
    }
    OSD_PRF("osd_fb_fix.mmio_len:0x%x, osd_fb_fix.smem_len=0x%x\n", osd_fb_fix.mmio_len, osd_fb_fix.smem_len);    
//  ge_m36f_attach(NULL, 0);
//  m_osddrv_ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);

    return TRUE;
}


void *ge_osd_get_display(UINT8 id)
{
    if (0 == id)
    {
        return ge_fb_p_display;
    }
    else
    {
        return ge_fb_p_display2;
    }
}

//get fb2 virtual addr according to the physical addr
void *ge_osd_fb2_virtual_buf(void *phy_addr)
{
    if ((UINT32)ge_fb_p_display2 == (UINT32)phy_addr)
        return ge_fb_display2;
    else
        return NULL;
}


void ge_osd_fb_mem_map_int()
{
    memset(m_fb_addr_map, 0, sizeof(m_fb_addr_map));
}

BOOL ge_osd_fb_add_mem_map(void *phy_addr, void *virtual_addr)
{
    int i;
    for (i = 0; i < ADDR_MAP_CNT; i ++)
    {
        if (NULL == m_fb_addr_map[i].phy_addr || NULL == m_fb_addr_map[i].virtual_addr ||\
            phy_addr == (void*)(m_fb_addr_map[i].phy_addr) || virtual_addr == (void*)(m_fb_addr_map[i].virtual_addr))
        {
            m_fb_addr_map[i].phy_addr = (UINT8*)phy_addr;
            m_fb_addr_map[i].virtual_addr = (UINT8*)virtual_addr;
            return TRUE;            
        }
    }
    return FALSE;
}

BOOL ge_osd_fb_remove_mem_map(void *phy_addr)
{
    int i;
    for (i = 0; i < ADDR_MAP_CNT; i ++)
    {
        if (phy_addr == (void*)(m_fb_addr_map[i].phy_addr))
        {
            m_fb_addr_map[i].phy_addr = NULL;
            m_fb_addr_map[i].virtual_addr = NULL;
            return TRUE;            
        }
    }
    return FALSE;
}

void *ge_osd_fb_get_virtual_buf(void *phy_addr)
{
    int i;
    for (i = 0; i < ADDR_MAP_CNT; i ++)
    {
        if (phy_addr == (void*)(m_fb_addr_map[i].phy_addr))
        {
            return (void*)(m_fb_addr_map[i].virtual_addr);
        }
    }
    return NULL;
    
}

void  ge_osd_exit(struct osd_device *dev)
{
  
    struct osd_private *priv = (struct osd_private *)dev->priv;

    if (priv->handle > 0) 
        close(priv->handle);

    munmap(ge_osd, osd_fb_fix.mmio_len);
    munmap(ge_fb_display, osd_fb_fix.smem_len);

    //    if (m_osddrv_ge_dev)
    //     ge_m36f_detach(m_osddrv_ge_dev);
}

/*
static enum GE_PIXEL_FORMAT osddrv_color_mode_to_ge(UINT8 bColorMode)
{
  enum GE_PIXEL_FORMAT color_format = GE_PF_CLUT8;
 
  switch (bColorMode)
  {
      case OSD_256_COLOR:
          color_format = GE_PF_CLUT8;
          break;
      case OSD_HD_ARGB1555:
        color_format     = GE_PF_ARGB1555;
          break;
      case OSD_HD_ARGB4444:
          color_format = GE_PF_ARGB4444;
          break;
      case OSD_HD_AYCbCr8888:
      case OSD_HD_ARGB8888:
          color_format = GE_PF_ARGB8888;
          break;
      default:
          break;
  }

  return color_format;
}
 */

static UINT32 osddrv_get_pitch(UINT8 bColorMode, UINT16 uWidth)
{
  switch (bColorMode)
  {
      case OSD_HD_ARGB1555:
      case OSD_HD_ARGB4444:
          uWidth <<= 1;
          break;
      case OSD_HD_AYCbCr8888:
      case OSD_HD_ARGB8888:
          uWidth <<= 2;
          break;
      case OSD_256_COLOR:
      default:
          break;
  }
  return uWidth;
}

static UINT32 osddrv_get_pitch_by_ge_format(UINT8 ge_pixel_fmt, UINT16 uWidth)
{
  switch (ge_pixel_fmt)
  {
      case GE_PF_ARGB1555:
      case GE_PF_ARGB4444:
          uWidth <<= 1;
          break;
      case GE_PF_RGB888:
      case GE_PF_ARGB8888:
          uWidth <<= 2;
          break;
      case GE_PF_CLUT8:
      case GE_PF_CK_CLUT8:
      default:
          break;
  }

  return uWidth;
}
#endif

static void rect_set(INT16 dst_width, INT16 dst_height,
                     UINT8* dst_buf, struct OSDRect *dst_rect,
                     UINT32 data, UINT32 pitch, UINT32 bpp)
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

static void rect_cpy(UINT8* dst_buf,
                    UINT8* src_buf,
                    struct OSDRect *dst_rect,
                    struct OSDRect *src_rect,
                    UINT32 pitch_dst,       
                    UINT32 pitch_src,
                    UINT32 bpp)
{

    UINT16 i;
    UINT32  uSrcBufOffset,uDstBufOffset;
    
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
    
    pars.scale_mode = (priv->scale_mode == OSD_SCALE_DUPLICATE) ? GMA_RSZ_DIRECT_RESIZE : GMA_RSZ_ALPHA_COLOR;
    pars.h_src = priv->h_div;
    pars.h_dst = priv->h_mul;
    pars.v_src = priv->v_div;
    pars.v_dst = priv->v_mul;
    pars.tv_sys = vpo_info.tvsys;
    pars.uScaleCmd = scale_cmd;
    ioctl(priv->handle, FBIO_SET_GMA_SCALE_INFO, &pars);
    OSD_PRF("osd scale <%d %d> => <%d %d>\n",
        pars.h_dst, pars.v_src, pars.v_src, pars.v_dst);
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
    OSD_PRF("scale rect <%d %d> => <%d %d>\n", h_div, v_div, h_mul, v_mul);

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
    OSD_PRF("scale pos <%d %d %d %d> => <%d %d %d %d>\n",
        src_rect.x, src_rect.y, src_rect.w, src_rect.h,
        dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h);
    
    pars.pos.x = dst_rect.x;
    pars.pos.y = dst_rect.y;
    pars.pos.w = dst_rect.w;
    pars.pos.h = dst_rect.h;
    if(ioctl(priv->handle, FBIO_MOVE_REGION, &pars) == 0)
    {
        ret = RET_SUCCESS;
    }
    else
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
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
    
    rect_cpy(dst_par->mem_start, src_par->mem_start,
             &dst_rect, &src_rect, dst_par->pitch,
             src_par->pitch, dst_par->bpp);
}

static int recreate_region(HANDLE hDev, pcosd_region_param pregion_param)
{
    struct OSDRect rect;
    
    if(pregion_param->region_id >= OSD_MAX_REG_NUM)
        goto FAIL;

    OSDDrv_DeleteRegion(hDev, pregion_param->region_id);

    rect.uLeft = pregion_param->region_x;
    rect.uTop = pregion_param->region_y;
    rect.uWidth = pregion_param->region_w;
    rect.uHeight = pregion_param->region_h;
    if(OSDDrv_CreateRegion(hDev, pregion_param->region_id, &rect, NULL) != RET_SUCCESS)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        goto FAIL;
    }

    return 1;

FAIL:
    return -1;
}

static int read_region(HANDLE hDev, posd_region_param pregion_param)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = NULL;
    struct osd_region_par *region = NULL;

    if((pregion_param == NULL) || (pregion_param->region_id > OSD_MAX_REG_NUM))
    {
        return -1;
    }

    if(dev)
    {
        priv = (struct osd_private *)dev->priv;
    }
    
    if(priv == NULL)
    {
        OSD_PRF("osd_gma device fail\n");
        return -1;
    }
    
    region = &priv->region_par[pregion_param->region_id];
    if(region->valid == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        return -1;
    }

    pregion_param->bitmap_addr = (UINT32)region->mem_start;
    pregion_param->color_format = region->para.eMode;
    pregion_param->galpha_enable = region->para.uGAlphaEnable;
    pregion_param->global_alpha = region->para.uGAlpha;
    pregion_param->pallette_sel = region->para.uPalletteSel;
    pregion_param->pixel_pitch = region->pitch;
    pregion_param->region_x = region->reg_rect.uLeft;
    pregion_param->region_y = region->reg_rect.uTop;
    pregion_param->region_w = region->reg_rect.uWidth;
    pregion_param->region_h = region->reg_rect.uHeight;

    return 1;
}

RET_CODE OSDDrv_Open(HANDLE hDev,struct OSDPara*pOpenPara)
{
    INT32 result = RET_SUCCESS;
    
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;
    BOOL bDirectFB = FALSE;
    
    #ifdef GE_DRAW_OSD_LIB
    ge_osd_init(dev);

    result = ge_open(m_osddrv_ge_dev);
    if (m_cmd_list == 0)
        m_cmd_list = ge_cmd_list_create(m_osddrv_ge_dev, 2);
    #endif
  
//#ifdef _ALI_OPENVG_ENABLE_ 
//    ALiOVGOpen(3);
//#endif

    /* If openned already, exit */
    if(dev->flags & HLD_DEV_STATS_UP)
    {
        OSD_PRF("osddrv_open: warning - device %s openned already!\n", dev->name);
        return RET_SUCCESS;
    }

    MUTEX_LOCK();

    priv->handle = open(priv->file_path, O_RDWR);
    if(priv->handle <= 0)
    {
        OSD_PRF("open file fail %s\n", priv->file_path);
        MUTEX_UNLOCK();
        return RET_FAILURE;
    }
    OSD_PRF("osd handle %d\n", priv->handle);

    m_cur_handle = priv->handle;
    
    #ifdef DIRECT_FB_OSD_SUPPORT
    if (priv->index == 1) //fb2, not use directFB
        bDirectFB = FALSE;
    else
        bDirectFB = TRUE;
    #endif

    if (!bDirectFB && (0 == ioctl(priv->handle, FBIO_SET_DE_LAYER, (priv->index == 0) ? DE_LAYER2 : DE_LAYER3)))
    {
        struct alifbio_fbinfo_data_pars fbinfo;

        ioctl(priv->handle, FBIO_GET_FBINFO_DATA, &fbinfo);
            
        priv->mem_start = (void *)fbinfo.mem_start;
        priv->mem_size = fbinfo.mem_size;
        priv->max_logical_width = fbinfo.xres_virtual;
        priv->max_logical_height = fbinfo.yres_virtual;
        priv->pitch = fbinfo.line_length;

        priv->virtual_mem_size = priv->mem_size;
        priv->virtual_mem_start = mmap(NULL, priv->virtual_mem_size, PROT_WRITE | PROT_READ
            , MAP_SHARED, priv->handle, 0);
        if(priv->virtual_mem_start == (void *)(-1))
        {
            OSD_PRF("mmap the OSD mem buf fail\n");
            MUTEX_UNLOCK();
            return RET_FAILURE;
        }

        OSD_PRF("physical OSD mem buf %x \nvirtual OSD mem buf %x, len:0x%x\n"
            , (int)priv->mem_start, (int)priv->virtual_mem_start, fbinfo.mem_size);
    }
    
    if(pOpenPara != NULL)
    {
        memcpy((void *)&(priv->default_para), (void *)pOpenPara, sizeof(struct OSDPara));
        OSD_PRF("osddrv_open color_format = 0x%08lx\n",priv->default_para.eMode);
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

    if(priv->default_para.eMode == OSD_256_COLOR)
    {
        priv->pallete_size = 256;
        priv->pallete_buf_allocated = (void *)malloc(priv->pallete_size * 4 + 3);
        if(priv->pallete_buf_allocated == NULL)
        {
            MUTEX_UNLOCK();
            return RET_FAILURE;
        }

        priv->pallete_buf = (void *)(((UINT32)priv->pallete_buf_allocated + 3) & ~3);
        memset(priv->pallete_buf, 0, priv->pallete_size * 4);
    }
        
    /* Setup init work mode */
    if (result == RET_SUCCESS)
    {
        dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
    }

    OSD_PRF("open done\n");
    MUTEX_UNLOCK();
    
    return result;
}

RET_CODE OSDDrv_Close(HANDLE hDev)
{
    INT32 result = RET_SUCCESS;
    int reg_id = 0;
    
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;
    BOOL bDirectFB = FALSE;

//#ifdef GE_SIMULATE_OSD
#ifdef GE_DRAW_OSD_LIB
    ge_cmd_list_destroy(m_osddrv_ge_dev, m_cmd_list);
    ge_gma_close(m_osddrv_ge_dev, GMA_SW_LAYER_ID);
    m_cmd_list = 0;

    ge_osd_exit(dev);
//    return RET_SUCCESS;
#endif
    
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        OSD_PRF("osddrv_close: warning - device %s closed already!\n", dev->name);
        return RET_SUCCESS;
    }

    for(reg_id = 0;reg_id < OSD_MAX_REG_NUM;reg_id++)
    {
        if(priv->region_par[reg_id].valid)
            OSDDrv_DeleteRegion(hDev, reg_id);
    }

    MUTEX_LOCK();

  #ifdef DIRECT_FB_OSD_SUPPORT
    if (priv->index == 1) //fb2, not use directFB
        bDirectFB = FALSE;
    else
        bDirectFB = TRUE;
  #endif

    if(!bDirectFB && priv->virtual_mem_start != NULL)
        munmap(priv->virtual_mem_start, priv->virtual_mem_size);
    
    close(priv->handle);
    
    if(priv->pallete_buf_allocated != NULL)
    {
        free(priv->pallete_buf_allocated);
    }
    
    /* Update flags */
    dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    OSD_PRF("close done\n");  
    MUTEX_UNLOCK();
    
    return result;
}

RET_CODE OSDDrv_IoCtl(HANDLE hDev,UINT32 dwCmd,UINT32 dwParam)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv; 
    pcosd_region_param pregion_param = NULL;

//#ifdef GE_SIMULATE_OSD
#if 0 //christian
    printf("GE OSDDrv_IoCtl:0x%x\n", dwCmd);
    switch (dwCmd)
    {
        case OSD_IO_SET_GLOBAL_ALPHA:
            dwCmd = GE_IO_SET_GLOBAL_ALPHA;
            break;
        case OSD_IO_SET_TRANS_COLOR:
            dwCmd = GE_IO_SET_TRANS_COLOR;
            break;
        case OSD_IO_ENABLE_ANTIFLICK:
            dwCmd = GE_IO_ENABLE_ANTIFLICK;
            dwParam = TRUE;
            break;
        case OSD_IO_DISABLE_ANTIFLICK:
            dwCmd = GE_IO_ENABLE_ANTIFLICK;
            dwParam = FALSE;
            break;
        case OSD_IO_SET_AUTO_CLEAR_REGION:
            dwCmd = GE_IO_SET_AUTO_CLEAR_REGION;
            break;
        case OSD_IO_GET_ON_OFF:
            dwCmd = GE_IO_GET_LAYER_ON_OFF;
            break;
        case OSD_IO_CREATE_REGION:
        case OSD_IO_MOVE_REGION:
        {
            pcosd_region_param posd_rgn_info = (pcosd_region_param)dwParam;
            ge_gma_region_t ge_rgn_info;
            RET_CODE ret;
            if (posd_rgn_info == NULL)
                return RET_FAILURE;
            ge_rgn_info.color_format = posd_rgn_info->color_format;
            ge_rgn_info.galpha_enable = posd_rgn_info->galpha_enable;
            ge_rgn_info.global_alpha = posd_rgn_info->global_alpha;
            ge_rgn_info.pallette_sel = posd_rgn_info->pallette_sel;
            ge_rgn_info.region_x = posd_rgn_info->region_x;
            ge_rgn_info.region_y = posd_rgn_info->region_y;
            ge_rgn_info.region_w = posd_rgn_info->region_w;
            ge_rgn_info.region_h = posd_rgn_info->region_h;
            ge_rgn_info.bitmap_addr = posd_rgn_info->bitmap_addr;
            ge_rgn_info.pixel_pitch = posd_rgn_info->pixel_pitch;
            ge_rgn_info.bitmap_x = posd_rgn_info->bitmap_x;
            ge_rgn_info.bitmap_y = posd_rgn_info->bitmap_y;
            ge_rgn_info.bitmap_w = posd_rgn_info->bitmap_w;
            ge_rgn_info.bitmap_h = posd_rgn_info->bitmap_h;
            if (dwCmd == OSD_IO_CREATE_REGION)
                ret = ge_gma_create_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, posd_rgn_info->region_id, &ge_rgn_info);
            else
                ret = ge_gma_move_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, posd_rgn_info->region_id, &ge_rgn_info);
            return ret;
        }
        case OSD_IO_GET_REGION_INFO:
        {
            posd_region_param posd_rgn_info = (posd_region_param)dwParam;
            ge_gma_region_t ge_rgn_info;
            RET_CODE ret;
            if (posd_rgn_info == NULL)
                return RET_FAILURE;
            ret = ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, posd_rgn_info->region_id, &ge_rgn_info);
            posd_rgn_info->color_format = ge_rgn_info.color_format;
            posd_rgn_info->galpha_enable = ge_rgn_info.galpha_enable;
            posd_rgn_info->global_alpha = ge_rgn_info.global_alpha;
            posd_rgn_info->pallette_sel = ge_rgn_info.pallette_sel;
            posd_rgn_info->region_x = ge_rgn_info.region_x;
            posd_rgn_info->region_y = ge_rgn_info.region_y;
            posd_rgn_info->region_w = ge_rgn_info.region_w;
            posd_rgn_info->region_h = ge_rgn_info.region_h;
            posd_rgn_info->bitmap_addr = ge_rgn_info.bitmap_addr;
            posd_rgn_info->pixel_pitch = ge_rgn_info.pixel_pitch;
            posd_rgn_info->bitmap_x = ge_rgn_info.bitmap_x;
            posd_rgn_info->bitmap_y = ge_rgn_info.bitmap_y;
            posd_rgn_info->bitmap_w = ge_rgn_info.bitmap_w;
            posd_rgn_info->bitmap_h = ge_rgn_info.bitmap_h;
            return ret;
        }
        case OSD_IO_GET_DISPLAY_RECT:
        {
            struct OSDRect *posd_rect = (struct OSDRect *)dwParam;
            ge_rect_t ge_rect;
            RET_CODE ret;
            if (posd_rect == NULL)
                return RET_FAILURE;
            ret = ge_io_ctrl_ext(m_osddrv_ge_dev, GMA_SW_LAYER_ID, GE_IO_GET_DISPLAY_RECT, (UINT32)&ge_rect);
            posd_rect->uLeft = ge_rect.left;
            posd_rect->uTop = ge_rect.top;
            posd_rect->uWidth = ge_rect.width;
            posd_rect->uHeight = ge_rect.height;
            return ret;
        }
        case OSD_IO_SET_DISPLAY_RECT:
        {
            struct OSDRect *posd_rect = (struct OSDRect *)dwParam;
            ge_rect_t ge_rect;
            RET_CODE ret;
            if (posd_rect == NULL)
                return RET_FAILURE;
            ge_rect.left = posd_rect->uLeft;
            ge_rect.top = posd_rect->uTop;
            ge_rect.width = posd_rect->uWidth;
            ge_rect.height = posd_rect->uHeight;
            ret = ge_io_ctrl_ext(m_osddrv_ge_dev, GMA_SW_LAYER_ID, GE_IO_SET_DISPLAY_RECT, (UINT32)&ge_rect);
            return ret;
        }
        default:
            return RET_SUCCESS;
            break;
    }
    return ge_io_ctrl_ext(m_osddrv_ge_dev, GMA_SW_LAYER_ID, dwCmd, dwParam);
#endif

    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        return RET_FAILURE;
    }

    MUTEX_LOCK();

    switch(dwCmd)
    {
        case OSD_IO_SET_TRANS_COLOR:
        {
            m_osd_trans_color = (UINT8)dwParam;
            break;
        }
        case OSD_IO_SET_GLOBAL_ALPHA:
        {
            ioctl(priv->handle, FBIO_SET_GLOBAL_ALPHA, dwParam);
            break;
        }
        case OSD_IO_ADJUST_MEMORY:
        {
            struct fb_var_screeninfo var_info;
            ioctl(priv->handle, FBIOGET_VSCREENINFO, &var_info);

            // switch to buffer #0 or #1 on next V-sync
            if (dwParam == 0)
                var_info.yoffset = 0;
            else
                var_info.yoffset = var_info.yres;
            var_info.activate = FB_ACTIVATE_VBL;
            ioctl(priv->handle, FBIOPAN_DISPLAY, &var_info);
            priv->region_par[0].mem_start = priv->virtual_mem_start + (var_info.yoffset * var_info.xres * (var_info.bits_per_pixel / 8));
            break;
        }
        case OSD_IO_GET_DISPLAY_RECT:
        {
            struct OSDRect *posd_rect = (struct OSDRect *)dwParam;
            struct gma_rect ge_rect;
            RET_CODE ret;
            if (posd_rect == NULL)
            {
                MUTEX_UNLOCK();
                return RET_FAILURE;
            }
            ioctl(priv->handle, FBIO_GET_DISPLAY_RECT, (UINT32)&ge_rect);
            posd_rect->uLeft = ge_rect.x;
            posd_rect->uTop = ge_rect.y;
            posd_rect->uWidth = ge_rect.w;
            posd_rect->uHeight = ge_rect.h;
            break;
        }
        case OSD_IO_SET_DISPLAY_RECT:
        {
            struct OSDRect *posd_rect = (struct OSDRect *)dwParam;
            struct gma_rect ge_rect;
            RET_CODE ret;
            if (posd_rect == NULL)
            {
                MUTEX_UNLOCK();
                return RET_FAILURE;
            }
            ge_rect.x= posd_rect->uLeft;
            ge_rect.y = posd_rect->uTop;
            ge_rect.w = posd_rect->uWidth;
            ge_rect.h = posd_rect->uHeight;
            ioctl(priv->handle, FBIO_SET_DISPLAY_RECT, (unsigned long)&ge_rect);
            break;
        }
        case OSD_IO_ENABLE_ANTIFLICK:
        {
            //corei//ioctl(priv->handle, FBIO_SET_GMA_ANTIFLICK, dwParam);
            break;
        }
        case OSD_IO_CREATE_REGION:
        {
            MUTEX_UNLOCK();
            pregion_param = (posd_region_param)dwParam;
            if(recreate_region(hDev, pregion_param) < 0)
            {
                OSD_PRF("recreate region fail\n");
                return RET_FAILURE;
            }
            MUTEX_LOCK();
            break;
        }
        case OSD_IO_GET_REGION_INFO:    
        {
            pregion_param = (posd_region_param)dwParam;
            if(read_region(hDev, pregion_param) < 0)
            {
                OSD_PRF("read region fail\n");
                MUTEX_UNLOCK();
                return RET_FAILURE;
            }
            break;
        }
        case OSD_IO_GET_LAYER_INFO:
		{
			posd_layer_param player_info = (posd_layer_param)dwParam;

			player_info->mode = priv->default_para.eMode;
			player_info->mem_start = priv->mem_start;
			player_info->mem_size = priv->mem_size;
			player_info->virtual_mem_start = priv->virtual_mem_start;
			player_info->virtual_mem_size = priv->virtual_mem_size;
			player_info->max_logical_width = priv->max_logical_width;
			player_info->max_logical_height = priv->max_logical_height;
			player_info->pitch = priv->pitch;
			break;
		}
		case OSD_IO_DISABLE_ENHANCE:
		{
			ioctl(priv->handle, FBIO_DISABLE_GMA_ENHANCE, (dwParam&0xFF));
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
			ioctl(priv->handle, FBIO_VIDEO_ENHANCE, (int)&enhance_pars);
            OSD_PRF("enhance flag %d grade %d\n", out_pars->changed_flag, out_pars->grade);
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
    struct osd_private *priv = (struct osd_private *)dev->priv;

    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
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
    struct osd_private *priv = (struct osd_private *)dev->priv; 
    int on = uOnOff;
    
//#ifdef GE_SIMULATE_OSD
#if 0  //christian
    printf("GE OSDDrv_ShowOnOff()!\n");
    GE_SIMU_MUTEX_LOCK();   
    RET_CODE ret = ge_gma_show_onoff(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uOnOff);
    GE_SIMU_MUTEX_UNLOCK();
    return ret; 
#endif
    
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }

    MUTEX_LOCK();
    ioctl(priv->handle, FBIO_WIN_ONOFF, on);
    MUTEX_UNLOCK();
    
    return RET_SUCCESS;
}

RET_CODE OSDDrv_SetPallette(HANDLE hDev, UINT8 *pPallette, UINT16 wColorN, UINT8 bType)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;
    struct alifbio_plt_pars plt_par;

//#ifdef GE_SIMULATE_OSD
#if 0  //christian
    printf("GE OSDDrv_SetPallette()!\n");

    static const ge_pal_attr_t m_rgb_pal_attr =
    {GE_PAL_RGB, GE_RGB_ORDER_ARGB, GE_ALPHA_RANGE_0_255, GE_ALPHA_POLARITY_0};

    GE_SIMU_MUTEX_LOCK();   
    RET_CODE ret = ge_gma_set_pallette(m_osddrv_ge_dev, GMA_SW_LAYER_ID, pPallette, wColorN, (bType == OSDDRV_RGB) ? &m_rgb_pal_attr : NULL);
    GE_SIMU_MUTEX_UNLOCK();
    return ret; 
#endif
    
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }

    if(wColorN != 256)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }

    MUTEX_LOCK();
    
    if(bType == OSDDRV_YCBCR)
    {
        UINT8 *src = pPallette;
        UINT8 *dst = (UINT8 *)priv->pallete_buf;
        int i = 0;

        if((ali_sys_ic_get_chip_id() == ALI_S3821) || (ali_sys_ic_get_chip_id() == ALI_C3921))
        {
            /* conver api YCbCrA to driver CbYCrA */
            for(i = 0;i < priv->pallete_size;i++)
            {
                *(dst + 2) = *(src + 2);
                *(dst) = *(src + 1);
                *(dst + 1) = *src;
                *(dst + 3) = *(src + 3);
                
                dst += 4;
                src += 4;
            }
        }
        else
        {
            memcpy((void *)priv->pallete_buf, pPallette, priv->pallete_size * 4);
        }
        
        plt_par.rgb_order = GE_RGB_ORDER_ACrCbY;
        plt_par.alpha_range = GE_ALPHA_RANGE_0_15;
        plt_par.alpha_pol = GE_ALPHA_POLARITY_0;
        plt_par.level = PLT_ALPHA_16;
    }
    else
    {
        memcpy((void *)priv->pallete_buf, pPallette, priv->pallete_size * 4);
        plt_par.rgb_order = GE_RGB_ORDER_ARGB;
        plt_par.alpha_range = GE_ALPHA_RANGE_0_255;
        plt_par.alpha_pol = GE_ALPHA_POLARITY_0;
        plt_par.level = PLT_ALPHA_256;
    }
    
    plt_par.type = (bType == OSDDRV_YCBCR) ? PLT_YUV : PLT_RGB;
    plt_par.pallette_buf = priv->pallete_buf;
    plt_par.color_num = wColorN;
    
    if(ioctl(priv->handle, FBIO_SET_PALLETTE, &plt_par) == 0)
    {
        OSD_PRF("set pallette type %d num %d\n", plt_par.type, plt_par.color_num);
        MUTEX_UNLOCK();
        return RET_SUCCESS;
    }

    OSD_PRF("set pallette fail\n");
    MUTEX_UNLOCK();
    
    return !RET_SUCCESS;
}

RET_CODE OSDDrv_GetPallette(HANDLE hDev, UINT8 *pPallette, UINT16 wColorN, UINT8 bType)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;

//#ifdef GE_SIMULATE_OSD
#if 0  //christian
    printf("GE OSDDrv_GetPallette()!\n");
    GE_SIMU_MUTEX_LOCK();   
    RET_CODE ret = ge_gma_get_pallette(m_osddrv_ge_dev, GMA_SW_LAYER_ID, pPallette, wColorN, NULL);
    GE_SIMU_MUTEX_UNLOCK();
    return ret; 
#endif
    
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }

    MUTEX_LOCK();
    
    if(bType == OSDDRV_YCBCR)
    {
        UINT8 *src = (UINT8 *)priv->pallete_buf;
        UINT8 *dst = pPallette;
        int i = 0;

        if((ali_sys_ic_get_chip_id() == ALI_S3821) || (ali_sys_ic_get_chip_id() == ALI_C3921))
        {
            /* conver driver CbYCrA to api YCbCrA */
            for(i = 0; i < wColorN; i++)
            {
                *dst = *(src + i + 2);
                *(dst + 1) = *(src + i + 1);
                *(dst + 2) = *src;
                *(dst + 3) = *(src + i + 3);
            }
        }
        else
        {
            memcpy(pPallette, (void *)priv->pallete_buf, priv->pallete_size * 4);
        }
    }
    else
    {
        memcpy(pPallette, (void *)priv->pallete_buf, priv->pallete_size * 4);
    }

    MUTEX_UNLOCK();
    
    return RET_SUCCESS;
}

RET_CODE OSDDrv_ModifyPallette(HANDLE hDev,UINT8 uIndex,UINT8 uY,UINT8 uCb,UINT8 uCr,UINT8 uK)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;
    struct alifbio_plt_pars plt_par;    

//#ifdef GE_SIMULATE_OSD
#if 0  //christian
    printf("GE OSDDrv_ModifyPallette()!\n");
    GE_SIMU_MUTEX_LOCK();   
    RET_CODE ret = ge_gma_modify_pallette(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uIndex, uK, uY, uCb, uCr, NULL);
    GE_SIMU_MUTEX_UNLOCK();
    return ret; 
#endif
    
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }

    MUTEX_LOCK();

    if((ali_sys_ic_get_chip_id() == ALI_S3821) || (ali_sys_ic_get_chip_id() == ALI_C3921))
    {
        /* conver api YCbCrA to driver CbYCrA */
        *(((UINT32 *)priv->pallete_buf) + uIndex) = (uK<<24) | (uY<<8) | uCb | (uCr<<16);
    }
    
    plt_par.type = PLT_YUV;
    plt_par.level = PLT_ALPHA_128;
    plt_par.pallette_buf = priv->pallete_buf;
    plt_par.color_num = priv->pallete_size;
    ioctl(priv->handle, FBIO_SET_PALLETTE, &plt_par);

    MUTEX_UNLOCK();
    
    return RET_SUCCESS;
}

RET_CODE OSDDrv_CreateRegion(HANDLE hDev,UINT8 uRegionId,struct OSDRect* rect,struct OSDPara*pOpenPara)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;
    struct alifbio_reg_pars reg_par;
    struct osd_region_par *osd_reg_par;
    int big_rect = 0;

    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
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
        OSD_PRF("this region %d has been created\n", uRegionId);
        MUTEX_UNLOCK();
        return RET_FAILURE;
    }

    if((priv->default_para.eMode != OSD_256_COLOR) 
        && (priv->default_para.eMode != OSD_HD_ARGB1555)
        && (priv->default_para.eMode != OSD_HD_ARGB8888))
    {
        OSD_PRF("don't support he color format %d\n", priv->default_para.eMode);
        MUTEX_UNLOCK();
        return RET_FAILURE;
    }

    osd_reg_par = priv->region_par + uRegionId;
        
#ifndef DIRECT_FB_OSD_SUPPORT
    if((rect->uWidth > priv->max_logical_width) 
        || (rect->uHeight > priv->max_logical_height))
    {
        UINT32 size = 0;
        
        size = (UINT32)rect->uWidth * (UINT32)rect->uHeight * osd_reg_par->bpp;
    
        if(size > (UINT32)priv->mem_size)
        {               
            OSD_PRF("don't support rect size w %d h %d max w %d h %d\n"
                , rect->uWidth, rect->uHeight, priv->max_logical_width
                , priv->max_logical_height);
            MUTEX_UNLOCK();
            return RET_FAILURE;
        }       

        big_rect = 1;
    }
#endif    
    
    memcpy((void *)&(osd_reg_par->reg_rect), rect, sizeof(*rect));
    osd_reg_par->bpp = get_bpp(priv->default_para.eMode);
    osd_reg_par->pitch = osd_reg_par->bpp * rect->uWidth;
    //osd_reg_par->pitch = priv->pitch;
    osd_reg_par->mem_size = osd_reg_par->pitch * rect->uHeight;

#ifdef DIRECT_FB_OSD_SUPPORT
    if (0 == priv->index)//fb0 use direct FB
    {
        dfb_osd_region_create(uRegionId, rect);
        osd_reg_par->valid = 1;
        MUTEX_UNLOCK();
        return RET_SUCCESS;
    }
#endif

    if(uRegionId == 0)
    {
        if(big_rect == 1)
            osd_reg_par->mem_start = priv->virtual_mem_start;
        else
            osd_reg_par->mem_start = priv->virtual_mem_start + rect->uTop * priv->pitch;
    }
    else
    {
        if(priv->region_par[0].valid == 0)
        {
            OSD_PRF("the user should carete the region 0 firstly\n");
            MUTEX_UNLOCK();
            return RET_FAILURE;
        }

        if(rect->uLeft < priv->region_par[0].reg_rect.uLeft
            || rect->uTop < priv->region_par[0].reg_rect.uTop
            || (rect->uLeft + rect->uWidth)
                > (priv->region_par[0].reg_rect.uLeft + priv->region_par[0].reg_rect.uWidth)
            || (rect->uTop + rect->uHeight)
                > (priv->region_par[0].reg_rect.uTop+ priv->region_par[0].reg_rect.uHeight))
        {
            OSD_PRF("region %d rect fail not in the first region\n", uRegionId);
            MUTEX_UNLOCK();
            return RET_FAILURE;
        }
        
        osd_reg_par->mem_start = malloc(osd_reg_par->mem_size);
        if(osd_reg_par->mem_start == NULL)
        {
            OSD_PRF("malloc the osd virtual buf fail id %d\n", uRegionId);
            MUTEX_UNLOCK();
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
        reg_par.mem_start = (void *)(priv->mem_start + rect->uTop * priv->pitch);
        reg_par.mem_size = osd_reg_par->mem_size;
        reg_par.pitch = osd_reg_par->pitch;
        reg_par.dis_format = get_dis_format(priv->default_para.eMode);
        if(ioctl(priv->handle, FBIO_CREATE_GMA_REGION, &reg_par) == 0)
        {   
            osd_reg_par->valid = 1;
            set_gma_scale_info(priv, GE_SET_SCALE_PARAM);
            set_gma_pos_info(priv);
            
            {       
                struct OSDRect write_rect;

                if(priv->default_para.eMode != OSD_256_COLOR)
                    m_osd_trans_color = 0;
                
                write_rect.uLeft = 0;
                write_rect.uTop = 0;
                write_rect.uWidth = rect->uWidth;
                write_rect.uHeight = rect->uHeight;
                rect_set(osd_reg_par->reg_rect.uWidth, osd_reg_par->reg_rect.uHeight, osd_reg_par->mem_start
                    ,&write_rect, m_osd_trans_color, osd_reg_par->pitch, osd_reg_par->bpp);     
            }
        
            OSD_PRF("create region%d ok x %d y %d w %d h %d\n", uRegionId
                , rect->uLeft, rect->uTop, rect->uWidth, rect->uHeight);

            MUTEX_UNLOCK();

            OSDDrv_IoCtl(hDev, OSD_IO_ADJUST_MEMORY, 1);
            OSDDrv_IoCtl(hDev, OSD_IO_ADJUST_MEMORY, 0);
            
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
        
        OSD_PRF("create region ok %d x %d y %d w %d h %d\n", uRegionId
            , rect->uLeft, rect->uTop, rect->uWidth, rect->uHeight);
        MUTEX_UNLOCK();
        
        return RET_SUCCESS;
    }

    OSD_PRF("create region fail\n");
    MUTEX_UNLOCK();
    
    return !RET_SUCCESS;
}

RET_CODE OSDDrv_DeleteRegion(HANDLE hDev, UINT8 uRegionId)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;
    struct osd_region_par *osd_reg_par;
    int reg_id = uRegionId;

//#ifdef GE_SIMULATE_OSD
#if 0  //christian
    printf("GE OSDDrv_DeleteRegion()!\n");
    GE_SIMU_MUTEX_LOCK();   
    RET_CODE ret = ge_gma_delete_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId);
    GE_SIMU_MUTEX_UNLOCK();     
    return ret; 
#endif
    
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }

    MUTEX_LOCK();
    
    if(priv->region_par[uRegionId].valid == 0)
    {
        OSD_PRF("delete none exist region %d\n", uRegionId);
        MUTEX_UNLOCK();
        return RET_FAILURE;
    }

#ifdef DIRECT_FB_OSD_SUPPORT
    if (0 == priv->index) //fb0 use direct FB
    {
        dfb_osd_region_delete(uRegionId);
        priv->region_par[uRegionId].valid = 0;
        MUTEX_UNLOCK();
        return RET_SUCCESS;
    }
#endif

    if(uRegionId == 0)
    {
        if(ioctl(priv->handle, FBIO_DELETE_GMA_REGION, reg_id) == 0)
        {
            priv->region_par[uRegionId].valid = 0;
            OSD_PRF("delete region ok %d\n", uRegionId);
            MUTEX_UNLOCK();
            return RET_SUCCESS;
        }
    }
    else
    {
        priv->region_par[uRegionId].valid = 0;
        OSD_PRF("delete region ok %d\n", uRegionId);
        MUTEX_UNLOCK();
        return RET_SUCCESS;
    }

    osd_reg_par = priv->region_par + uRegionId;
    free(osd_reg_par->mem_start);

    OSD_PRF("delete region fail\n");
    MUTEX_UNLOCK();
    
    return !RET_SUCCESS;
}

RET_CODE OSDDrv_SetRegionPos(HANDLE hDev, UINT8 uRegionId, struct OSDRect* rect)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;

//#ifdef GE_SIMULATE_OSD
#if 0  //christian
    printf("GE OSDDrv_SetRegionPos()!\n");
    ge_gma_region_t region_param;
    GE_SIMU_MUTEX_LOCK();       

    ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, &region_param);

    region_param.region_x = rect->uLeft;
    region_param.region_y = rect->uTop;
    region_param.region_w = rect->uWidth;
    region_param.region_h = rect->uHeight; 

    //region_param.bitmap_addr = 0;

    RET_CODE ret = ge_gma_move_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, &region_param);
    GE_SIMU_MUTEX_UNLOCK(); 
    return ret;

#endif
    
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }

    if(uRegionId != 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }

    MUTEX_LOCK();
    
    if(priv->region_par[uRegionId].valid == 0)
    {
        OSD_PRF("set region pos fail \n");
        return RET_FAILURE;
    }

    OSD_PRF("set region pos x %d y %d w %d h %d\n", rect->uLeft, rect->uTop, rect->uWidth, rect->uHeight);
    
    if(memcmp(rect, &(priv->region_par[uRegionId].reg_rect), sizeof(*rect)) != 0)
    {
        RET_CODE ret = RET_FAILURE;     
        
        if(uRegionId == 0)
        {
            #ifdef DIRECT_FB_OSD_SUPPORT
            if (0 == priv->index) //fb0 use direct FB
            {
                dfb_osd_set_region_pos(uRegionId, rect);
                memcpy((void *)&(priv->region_par[uRegionId].reg_rect), (void *)rect, sizeof(*rect));
                MUTEX_UNLOCK();
                return RET_SUCCESS;
            }
            #endif  

            struct alifbio_move_region_pars  region_param;       
            memset((void *)&region_param, 0, sizeof(region_param));
            region_param.region_id = uRegionId;
            region_param.pos.x = rect->uLeft;
            region_param.pos.y = rect->uTop;
            region_param.pos.w = rect->uWidth;
            region_param.pos.h = rect->uHeight;
            
            if(ioctl(priv->handle, FBIO_MOVE_REGION, &region_param) == 0)
            {
                memcpy((void *)&(priv->region_par[uRegionId].reg_rect), (void *)rect, sizeof(*rect));
                ret = RET_SUCCESS;
            }           
        }
        else
        {
            memcpy((void *)&(priv->region_par[uRegionId].reg_rect), (void *)rect, sizeof(*rect));
            ret = RET_SUCCESS;          
        }
        
        OSD_PRF("%s : set region pos fail\n", __FUNCTION__);
        MUTEX_UNLOCK();
        
        return ret;
    }

    MUTEX_UNLOCK();
    
    return RET_SUCCESS;
}

RET_CODE OSDDrv_GetRegionPos(HANDLE hDev,UINT8 uRegionId,struct OSDRect* rect)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;

//#ifdef GE_SIMULATE_OSD
#if 0  //christian
    printf("GE OSDDrv_GetRegionPos()!\n");
    ge_gma_region_t region_param;
    GE_SIMU_MUTEX_LOCK();       

    RET_CODE ret = ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, &region_param);

    rect->uLeft = region_param.region_x;
    rect->uTop = region_param.region_y;
    rect->uWidth = region_param.region_w;
    rect->uHeight = region_param.region_h;
    GE_SIMU_MUTEX_UNLOCK(); 
    return ret;
#endif
    
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        return RET_FAILURE;
    }

    MUTEX_LOCK();

    if(priv->region_par[uRegionId].valid == 0)
    {
        OSD_PRF("get region pos fail \n");
        return RET_FAILURE;
    }

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
    printf("GE OSDDrv_RegionShow()!\n");
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

void OSDDrv_CacheFlush(UINT8 *mem_start, int mem_size)
{
    struct alifbio_cache_flush cache_flush;
    cache_flush.mem_start = (UINT32)mem_start;
    cache_flush.mem_size = mem_size;
    //printf("start:0x%x, size:%d\n", mem_start, mem_size);
    if (m_cur_handle > 0)
        ioctl(m_cur_handle, FBIO_SET_UI_CACHE_FLUSH, &cache_flush);
}

RET_CODE OSDDrv_RegionWrite(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct OSDRect *rect)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;
    struct osd_region_par *osd_reg_par = &priv->region_par[uRegionId];
    UINT8* lpbScr;
    
//fdef GE_SIMULATE_OSD
#if 0
    printf("GE OSDDrv_RegionWrite()!\n");
    RET_CODE ret;
    GE_SIMU_MUTEX_LOCK();       

    ge_lock(m_osddrv_ge_dev);
    
    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);

    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
         ge_unlock(m_osddrv_ge_dev);
         GE_SIMU_MUTEX_UNLOCK();                
        return ret;
    }

    UINT32 byte_pitch = osddrv_get_pitch(pVscr->bColorMode, pVscr->vR.uWidth);
    UINT32 ptn_x, ptn_y;

    ptn_x = rect->uLeft - pVscr->vR.uLeft;
    ptn_y = rect->uTop - pVscr->vR.uTop;
    osal_cache_flush(pVscr->lpbScr + byte_pitch * ptn_y, byte_pitch * rect->uHeight);

    ge_cmd_list_hdl cmd_list = m_cmd_list;
    ge_base_addr_t base_addr;

    base_addr.color_format = (enum GE_PIXEL_FORMAT)osddrv_color_mode_to_ge(pVscr->bColorMode);
    base_addr.base_address = (UINT32)pVscr->lpbScr;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = pVscr->vR.uWidth;
    base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_DRAW_BITMAP);
    ge_set_base_addr(m_osddrv_ge_dev, cmd_hdl, GE_PTN, &base_addr);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST_PTN, rect->uWidth, rect->uHeight);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->uLeft, rect->uTop);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_PTN, ptn_x, ptn_y);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);

    ge_unlock(m_osddrv_ge_dev);
    
    GE_SIMU_MUTEX_UNLOCK();     

    return RET_SUCCESS;
#endif
    
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }

    MUTEX_LOCK();

    if(osd_reg_par->valid == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        MUTEX_UNLOCK();
        return RET_FAILURE;
    }

    OSD_PRF("region write x %d y %d w %d h %d\n", rect->uLeft, rect->uTop, rect->uWidth, rect->uHeight);
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

        lpbScr = pVscr->lpbScr;

        #ifdef DIRECT_FB_OSD_SUPPORT
        if (0 == priv->index)//fb0 use direct FB
        {
            dfb_osd_region_write(uRegionId, lpbScr, rect, osd_reg_par->bpp * pVscr->vR.uWidth);
            MUTEX_UNLOCK();
            return  RET_SUCCESS;
        }
        #endif

        #ifdef GE_DRAW_OSD_LIB
        /*
         //usd fb2 for virtual screen buffer for CPU copy
         lpbScr = ge_osd_fb_get_virtual_buf(pVscr->lpbScr);
        if (lpbScr == NULL)
        {
            printf("can not get fb2 virtual buffer!\n");
            return RET_FAILURE;
        }
        */
        /*
        UINT32 byte_pitch = osddrv_get_pitch(pVscr->bColorMode, pVscr->vR.uWidth);
        UINT32 ptn_x, ptn_y;
        ptn_x = rect->uLeft - pVscr->vR.uLeft;
        ptn_y = rect->uTop - pVscr->vR.uTop;
        OSDDrv_CacheFlush(pVscr->lpbScr + byte_pitch * ptn_y, byte_pitch * rect->uHeight);
        */
        OSDLib_GeRegionWrite(pVscr, &dst);
        return RET_SUCCESS;
        #endif    

        rect_cpy(osd_reg_par->mem_start, lpbScr,
                 &dst, &src, 
                 osd_reg_par->pitch, osd_reg_par->bpp * pVscr->vR.uWidth, 
                 osd_reg_par->bpp); 

        if(uRegionId != 0)
            update_virtual_region(hDev, uRegionId, 0, &dst);        
    }

    MUTEX_UNLOCK();
    
    return  RET_SUCCESS;
}

RET_CODE OSDDrv_RegionRead(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct OSDRect *rect)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;
    struct osd_region_par *osd_reg_par = &priv->region_par[uRegionId];
    UINT8* lpbScr;

//#ifdef GE_SIMULATE_OSD
#if 0
    printf("GE OSDDrv_RegionRead()!\n");
    RET_CODE ret;
    GE_SIMU_MUTEX_LOCK();   
    
    ge_lock(m_osddrv_ge_dev);
    
    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
        ge_unlock(m_osddrv_ge_dev);
             
        GE_SIMU_MUTEX_UNLOCK();         
        return ret;
    }
    UINT32 byte_pitch = osddrv_get_pitch(pVscr->bColorMode, pVscr->vR.uWidth);
    UINT32 ptn_x, ptn_y;

    ptn_x = rect->uLeft - pVscr->vR.uLeft;
    ptn_y = rect->uTop - pVscr->vR.uTop;
    osal_cache_invalidate(pVscr->lpbScr + byte_pitch * ptn_y, byte_pitch * rect->uHeight);

    ge_cmd_list_hdl cmd_list = m_cmd_list;
    ge_base_addr_t base_addr;

    base_addr.color_format = (enum GE_PIXEL_FORMAT)osddrv_color_mode_to_ge(pVscr->bColorMode);
    base_addr.base_address = (UINT32)pVscr->lpbScr;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = pVscr->vR.uWidth;
    base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_PRIM_DISABLE);
    ge_set_base_addr(m_osddrv_ge_dev, cmd_hdl, GE_DST, &base_addr);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, ptn_x, ptn_y);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->uWidth, rect->uHeight);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_SRC, rect->uLeft, rect->uTop);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);

    ge_unlock(m_osddrv_ge_dev);

    GE_SIMU_MUTEX_UNLOCK(); 

    return RET_SUCCESS;
#endif
    
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }

    MUTEX_LOCK();

    if(osd_reg_par->valid == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }
    OSD_PRF("region read x %d y %d w %d h %d\n", rect->uLeft, rect->uTop
        , rect->uWidth, rect->uHeight);
    
    struct OSDRect src, dst;    
    
    src.uLeft = rect->uLeft;
    src.uTop = rect->uTop;
    src.uWidth = rect->uWidth;
    src.uHeight = rect->uHeight;
    dst.uLeft = rect->uLeft - pVscr->vR.uLeft;
    dst.uTop = rect->uTop - pVscr->vR.uTop;
    dst.uWidth = rect->uWidth;
    dst.uHeight = rect->uHeight;    
    
    lpbScr = pVscr->lpbScr;

#ifdef DIRECT_FB_OSD_SUPPORT
    if (0 == priv->index)//fb0 use direct FB
    {
        dfb_osd_region_read(uRegionId, lpbScr, rect, osd_reg_par->bpp * pVscr->vR.uWidth);
        MUTEX_UNLOCK();
        return RET_SUCCESS;
    }
#endif
    
#ifdef GE_DRAW_OSD_LIB
    //usd fb2 for virtual screen buffer for CPU copy
    /*
    lpbScr = ge_osd_fb_get_virtual_buf(pVscr->lpbScr);
    if (lpbScr == NULL)
    {
        printf("can not get fb2 virtual buffer!\n");
        return RET_FAILURE;
    }
    */
    OSDLib_GeRegionRead(pVscr, &src);
    return RET_SUCCESS;
#endif        

    rect_cpy(lpbScr, osd_reg_par->mem_start,
             &dst, &src, 
             osd_reg_par->bpp * pVscr->vR.uWidth, osd_reg_par->pitch, 
             osd_reg_par->bpp); 

    MUTEX_UNLOCK();

    return  RET_SUCCESS;
}

RET_CODE OSDDrv_RegionFill(HANDLE hDev,UINT8 uRegionId,struct OSDRect *rect, UINT32 uColorData)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;
    struct osd_region_par *osd_reg_par = &priv->region_par[uRegionId];

#ifdef GE_SIMULATE_OSD
    printf("GE OSDDrv_RegionFill()!\n");
    RET_CODE ret;
    GE_SIMU_MUTEX_LOCK();   

    ge_lock(m_osddrv_ge_dev);
    
    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
       ge_unlock(m_osddrv_ge_dev);     
       
       GE_SIMU_MUTEX_UNLOCK();  
    return ret;
    }
    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, m_cmd_list, GE_FILL_RECT_DRAW_COLOR);
    ge_set_draw_color(m_osddrv_ge_dev, cmd_hdl, uColorData);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->uLeft, rect->uTop);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->uWidth, rect->uHeight);
    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);

    ge_cmd_list_end(m_osddrv_ge_dev, m_cmd_list);

    ge_unlock(m_osddrv_ge_dev); 
    
    GE_SIMU_MUTEX_UNLOCK(); 

    return RET_SUCCESS;
#endif
    
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }

    MUTEX_LOCK();

#ifdef DIRECT_FB_OSD_SUPPORT
    if (0 == priv->index)//fb0 use direct FB
    {
        dfb_osd_region_fill(uRegionId, rect, uColorData);
        MUTEX_UNLOCK();
        return RET_SUCCESS;
    }
#endif

    rect_set(osd_reg_par->reg_rect.uWidth, osd_reg_par->reg_rect.uHeight, osd_reg_par->mem_start,
             rect, uColorData, osd_reg_par->pitch, osd_reg_par->bpp);   

    if(uRegionId != 0)      
        update_virtual_region(hDev, uRegionId, 0, rect);     

    MUTEX_UNLOCK();
    
    return RET_SUCCESS;
}

RET_CODE OSDDrv_RegionWrite2(HANDLE hDev, UINT8 uRegionId, UINT8 *pSrcData, UINT16 uSrcWidth, UINT16 uSrcHeight, 
                                  struct OSDRect *pSrcRect,struct OSDRect *pDestRect)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;
    struct osd_region_par *osd_reg_par = &priv->region_par[uRegionId];
    struct OSDRect src, dst;

#ifdef GE_SIMULATE_OSD
    printf("GE OSDDrv_RegionWrite2()!\n");
    RET_CODE ret;
    ge_gma_region_t region_param;

    UINT32 vscr_width = 1920, vscr_height = 1080; // get from subtitle
    UINT32 reg_width, reg_height;
    GE_SIMU_MUTEX_LOCK();

    ret = ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, &region_param);
    if (ret != RET_SUCCESS)
    {
    GE_SIMU_MUTEX_UNLOCK();     
        return ret;
    }
    if (uSrcWidth > 1280)
    {
        vscr_width = 1920;
        vscr_height = 1080;
    }
    else if (uSrcWidth > 720)
    {
        vscr_width = 1280;
        vscr_height = 720;
    }
    else
    {
        vscr_width = 720;
        vscr_height = 576;
    }

    reg_width = region_param.region_w;
    reg_height = region_param.region_h;


    if (vscr_width > reg_width || vscr_height > reg_height)
    {
        region_param.region_x = 0;
        region_param.region_y = 0;
        region_param.region_w = vscr_width;
        region_param.region_h = vscr_height;
        region_param.bitmap_x = 0;
        region_param.bitmap_y = 0;
        region_param.bitmap_w = vscr_width;
        region_param.bitmap_h = vscr_height;
        region_param.pixel_pitch = vscr_width;
        region_param.bitmap_addr = 0;
        ret = ge_gma_delete_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId);
        if (ret != RET_SUCCESS)
        {
            //SDBBP();
        }
        ret = ge_gma_create_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, &region_param);
        if (ret != RET_SUCCESS)
        {
            //SDBBP();
            return ret;
        }
        ret = ge_gma_scale(m_osddrv_ge_dev, GMA_SW_LAYER_ID, GE_VSCALE_TTX_SUBT, PAL);
        if (ret != RET_SUCCESS)
        {
            //SDBBP();
        }
    }

    ge_lock(m_osddrv_ge_dev);
    
    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
    ge_unlock(m_osddrv_ge_dev);    
    
    GE_SIMU_MUTEX_UNLOCK();             
        return ret;
    }
    UINT32 byte_pitch = osddrv_get_pitch(OSD_256_COLOR, uSrcWidth);

    osal_cache_flush(pSrcData + byte_pitch * pSrcRect->uTop, byte_pitch * pSrcRect->uHeight);

    ge_cmd_list_hdl cmd_list = m_cmd_list;
    ge_base_addr_t base_addr;

    base_addr.color_format = (enum GE_PIXEL_FORMAT)osddrv_color_mode_to_ge(OSD_256_COLOR);
    base_addr.base_address = (UINT32)pSrcData;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = uSrcWidth;
    base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_DRAW_BITMAP);
    ge_set_base_addr(m_osddrv_ge_dev, cmd_hdl, GE_PTN, &base_addr);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST_PTN, pSrcRect->uWidth, pSrcRect->uHeight);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, pDestRect->uLeft, pDestRect->uTop);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_PTN, pSrcRect->uLeft, pSrcRect->uTop);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);

    ge_unlock(m_osddrv_ge_dev); 
    
    GE_SIMU_MUTEX_UNLOCK();

    return RET_SUCCESS;
#endif  

    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        return RET_FAILURE;
    }

    MUTEX_LOCK();
    
    if(osd_reg_par->valid == 0)
    {
        OSD_PRF("%s line %u fail\n", __FUNCTION__, __LINE__);
        MUTEX_UNLOCK();
        return RET_FAILURE;
    }

    src.uLeft = pSrcRect->uLeft;
    src.uTop = pSrcRect->uTop;
    src.uWidth = pSrcRect->uWidth;
    src.uHeight = pSrcRect->uHeight;
    dst.uLeft = pDestRect->uLeft;
    dst.uTop = pDestRect->uTop;
    dst.uWidth = pDestRect->uWidth;
    dst.uHeight = pDestRect->uHeight;

    rect_cpy(osd_reg_par->mem_start, pSrcData,
             &dst, &src, 
             osd_reg_par->pitch, osd_reg_par->bpp * pSrcRect->uWidth, 
             osd_reg_par->bpp);

    MUTEX_UNLOCK();
    
    return  RET_SUCCESS;
}

RET_CODE OSDDrv_DrawHorLine(HANDLE hDev, UINT8 uRegionId, UINT32 x, UINT32 y, UINT32 width, UINT32 color)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;

#ifdef GE_SIMULATE_OSD
    printf("GE OSDDrv_DrawHorLine()!\n");
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
    struct osd_private *priv = (struct osd_private *)dev->priv;
    struct alifbio_gma_scale_info_pars pars;
    gma_scale_param_t *scale_parm;
    enum OSDSys eOSDSys;    
    
#if 0//def KERNEL_GMA_TEST
    static const gma_scale_param_t m_gma_scale_param[] =
    {
        {LINE_720_25, 720, 576, 1280, 720 },
        {LINE_720_30, 720, 480, 1280, 720 },
        {LINE_1080_25, 720, 576, 1920, 1080 },
        {LINE_1080_30, 720, 480, 1920, 1080 },
    };

    static const gma_scale_param_t m_gma_scale_param_src_1280x720[] =
    {
        {PAL,  1280, 720, 720, 576 },
        {NTSC, 1280, 720, 720, 480 },
        {LINE_720_25, 1, 1, 1, 1 },
        {LINE_720_30, 1, 1, 1, 1 },
        {LINE_1080_25, 1280, 720, 1920, 1080 },
        {LINE_1080_30, 1280, 720, 1920, 1080 },
    };

    static const gma_scale_param_t m_gma_multiview[]=
    {
        {PAL,  1, 384, 1, 510 },
        {NTSC, 1, 384, 1, 426},     
    };
  
    switch (uScaleCmd)
    {
        case OSD_SCALE_WITH_PARAM:
            uScaleCmd = GE_SET_SCALE_PARAM;
            break;
        case OSD_VSCALE_OFF:
            uScaleCmd = GE_VSCALE_OFF;
            break;
        case OSD_VSCALE_TTX_SUBT:
            uScaleCmd = GE_VSCALE_TTX_SUBT;
            break;
        case OSD_HDUPLICATE_ON:
            uScaleCmd = GE_H_DUPLICATE_ON_OFF;
            uScaleParam = TRUE;
            break;
        case OSD_HDUPLICATE_OFF:
            uScaleCmd = GE_H_DUPLICATE_ON_OFF;
            uScaleParam = FALSE;
            break;
        case OSD_OUTPUT_720:
            uScaleCmd = GE_SET_SCALE_PARAM;
            if (uScaleParam == OSD_SOURCE_NTSC)
                uScaleParam = (UINT32)&m_gma_scale_param[1];
            else
                uScaleParam = (UINT32)&m_gma_scale_param[0];
            break;
        case OSD_OUTPUT_1080:
            uScaleCmd = GE_SET_SCALE_PARAM;
            if (uScaleParam == OSD_SOURCE_NTSC)
                uScaleParam = (UINT32)&m_gma_scale_param[3];
            else
                uScaleParam = (UINT32)&m_gma_scale_param[2];
            break;
        case OSD_SET_SCALE_MODE:
            uScaleCmd = GE_SET_SCALE_MODE;
            break;
        case OSD_HDVSCALE_OFF:
            uScaleCmd = GE_SET_SCALE_PARAM;
            if (uScaleParam == OSD_SOURCE_NTSC)
                uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[1];
            else
                uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[0];
            break;
        case OSD_HDOUTPUT_720:
                uScaleCmd = GE_SET_SCALE_PARAM;
                if (uScaleParam == OSD_SOURCE_NTSC)
                    uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[3];
                else
                    uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[2];
                break;
        case OSD_HDOUTPUT_1080:
                uScaleCmd = GE_SET_SCALE_PARAM;
                if (uScaleParam == OSD_SOURCE_NTSC)
                    uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[5];
                else
                    uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[4];
                break;
        case OSD_VSCALE_DVIEW:
                uScaleCmd = GE_SET_SCALE_PARAM;
                if (*(enum OSDSys *)uScaleParam == OSD_PAL)
                    uScaleParam = (UINT32)&m_gma_multiview[0];
                else
                    uScaleParam = (UINT32)&m_gma_multiview[1];       
                break;
            default:    
               return RET_SUCCESS;
                break;
        }
    
        if(uScaleCmd == GE_SET_SCALE_PARAM)
        {   
            scale_parm = (const gma_scale_param_t *)uScaleParam;
            pars.tv_sys = scale_parm->tv_sys;
            pars.h_dst = scale_parm->h_mul;
            pars.h_src = scale_parm->h_div;
            pars.v_dst = scale_parm->v_mul;
            pars.v_src = scale_parm->v_div;
            pars.uScaleCmd = uScaleCmd;
            pars.uScaleParam = (UINT32)uScaleParam;
            ioctl(priv->handle, FBIO_SET_GMA_SCALE_INFO, &pars);
        } 
        else 
        {
            pars.uScaleCmd = uScaleCmd;
            pars.uScaleParam = (UINT32)uScaleParam;
            ioctl(priv->handle, FBIO_SET_GMA_SCALE_INFO, &pars);
        } 
        
        return RET_SUCCESS;

#else

    #ifdef GE_SIMULATE_OSD
    printf("GE OSDDrv_Scale()!\n");

    static const ge_scale_param_t m_gma_scale_param[] =
    {
        {LINE_720_25, 720, 576, 1280, 720 },
        {LINE_720_30, 720, 480, 1280, 720 },
        {LINE_1080_25, 720, 576, 1920, 1080 },
        {LINE_1080_30, 720, 480, 1920, 1080 },
    };

    static const ge_scale_param_t m_gma_scale_param_src_1280x720[] =
    {
        {PAL,  1280, 720, 720, 576 },
        {NTSC, 1280, 720, 720, 480 },
        {LINE_720_25, 1, 1, 1, 1 },
        {LINE_720_30, 1, 1, 1, 1 },
        {LINE_1080_25, 1280, 720, 1920, 1080 },
        {LINE_1080_30, 1280, 720, 1920, 1080 },
    };

    static const ge_scale_param_t m_gma_multiview[]=
    {
        {PAL,  1, 384, 1, 510 },
        {NTSC, 1, 384, 1, 426},     
    };
    GE_SIMU_MUTEX_LOCK();
    switch (uScaleCmd)
    {
        case OSD_SCALE_WITH_PARAM:
            uScaleCmd = GE_SET_SCALE_PARAM;
            break;
        case OSD_VSCALE_OFF:
            uScaleCmd = GE_VSCALE_OFF;
            break;
        case OSD_VSCALE_TTX_SUBT:
            uScaleCmd = GE_VSCALE_TTX_SUBT;
            break;
        case OSD_HDUPLICATE_ON:
            uScaleCmd = GE_H_DUPLICATE_ON_OFF;
            uScaleParam = TRUE;
            break;
        case OSD_HDUPLICATE_OFF:
            uScaleCmd = GE_H_DUPLICATE_ON_OFF;
            uScaleParam = FALSE;
            break;
        case OSD_OUTPUT_720:
            uScaleCmd = GE_SET_SCALE_PARAM;
            if (uScaleParam == OSD_SOURCE_NTSC)
                uScaleParam = (UINT32)&m_gma_scale_param[1];
            else
                uScaleParam = (UINT32)&m_gma_scale_param[0];
            break;
        case OSD_OUTPUT_1080:
            uScaleCmd = GE_SET_SCALE_PARAM;
            if (uScaleParam == OSD_SOURCE_NTSC)
                uScaleParam = (UINT32)&m_gma_scale_param[3];
            else
                uScaleParam = (UINT32)&m_gma_scale_param[2];
            break;
        case OSD_SET_SCALE_MODE:
            uScaleCmd = GE_SET_SCALE_MODE;
            break;
        case OSD_HDVSCALE_OFF:
            uScaleCmd = GE_SET_SCALE_PARAM;
            if (uScaleParam == OSD_SOURCE_NTSC)
                uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[1];
            else
                uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[0];
            break;
        case OSD_HDOUTPUT_720:
                uScaleCmd = GE_SET_SCALE_PARAM;
                if (uScaleParam == OSD_SOURCE_NTSC)
                    uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[3];
                else
                    uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[2];
                break;
        case OSD_HDOUTPUT_1080:
                uScaleCmd = GE_SET_SCALE_PARAM;
                if (uScaleParam == OSD_SOURCE_NTSC)
                    uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[5];
                else
                    uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[4];
                break;
        case OSD_VSCALE_DVIEW:
                uScaleCmd = GE_SET_SCALE_PARAM;
                if (*(enum OSDSys *)uScaleParam == OSD_PAL)
                    uScaleParam = (UINT32)&m_gma_multiview[0];
                else
                    uScaleParam = (UINT32)&m_gma_multiview[1];       
                break;
        default:
            GE_SIMU_MUTEX_UNLOCK();
               return RET_SUCCESS;
                break;
    }
    RET_CODE ret = ge_gma_scale(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uScaleCmd, uScaleParam);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;
    #endif
#endif
    
    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        return RET_FAILURE;
    }

    MUTEX_LOCK();
    
    OSD_PRF("osd scale command %d\n", uScaleCmd);
    
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
            break;
        }
        default:
            uScaleCmd = 0;
            break;
    }

    set_gma_scale_info(priv, uScaleCmd);
    set_gma_pos_info(priv);

    MUTEX_UNLOCK();
    
    return  RET_SUCCESS;
}

RET_CODE OSDDrv_SetClip(HANDLE hDev,enum CLIPMode clipmode,struct OSDRect *pRect)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;

#ifdef GE_SIMULATE_OSD
    printf("GE OSDDrv_SetClip()!\n");
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
    printf("GE OSDDrv_ClearClip()!\n");
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

RET_CODE OSDDrv_GetRegionAddr(HANDLE hDev,UINT8 region_idx, UINT16 y, UINT32 *addr)
{
    struct osd_device *dev = (struct osd_device *)hDev;
    struct osd_private *priv = (struct osd_private *)dev->priv;

#ifdef GE_SIMULATE_OSD
    printf("GE OSDDrv_GetRegionAddr()!\n");
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

#ifdef GE_DRAW_OSD_LIB
static int m_fdmem = 0;
static UINT8 *m_v_addr = NULL;
static UINT32 m_map_len = 0;
BOOL OSDDrv_GetMemMap(UINT8 **phy_addr, UINT8 **virtual_addr, UINT32 *size)
{
    int fd;
    void * pbuf;

    struct ali_fb_rsc_mem_map mem_map;
    // to get the UI bitmap map physical address.
    if ((fd = open(fb0_path, O_RDWR)) < 0) 
    {
        printf("Open /dev/mem failed\n");
        return FALSE;
    }
    
    if (ioctl(fd, FBIO_GET_UI_RSC_MAP, (void*)(&mem_map)))
    {
        printf("Get FB memory map error!\n");
        close(fd);
        return FALSE;
    }
    close(fd);
    
    if ((m_fdmem = open("/dev/mem", O_RDWR)) < 0) 
    {
        printf("Open /dev/mem failed\n");
        return FALSE;
    }

    if ((pbuf = mmap(NULL, mem_map.mem_size, PROT_READ|PROT_WRITE, MAP_SHARED, m_fdmem, mem_map.mem_start)) == (void *) -1)
    {
        printf("mmap /dev/mem failed\n");
        close(m_fdmem);
        return FALSE;
    }
    *phy_addr = mem_map.mem_start;
    *virtual_addr = pbuf;
    *size = mem_map.mem_size;
    m_v_addr = pbuf;
    m_map_len = mem_map.mem_size;

    return TRUE;
}

void OSDDrv_ExitMemMap()
{
    if (m_v_addr)
        munmap(m_v_addr, m_map_len);
    close(m_fdmem);    
}
#endif

static int b_dfb_init_over = 0;
int HLD_OSDDrv_get_dfb_init_status()
{
#ifdef DIRECT_FB_OSD_SUPPORT
    return b_dfb_init_over;
#else
    return 1;
#endif
}

#ifdef DIRECT_FB_OSD_SUPPORT
#include <directfb.h>
//Get all dfb windows handles  that used for OSD.
RET_CODE OSDDrv_GetDfbOsdWindowHandles( IDirectFBWindow        **dfb_osd_window_handles)
{
    return  dfb_osd_get_window(dfb_osd_window_handles);
}

extern void dfb_osd_init();

static void dfb_osd_init_wrapper()
{
    dfb_osd_init();
    b_dfb_init_over = 1;
}

static void HLD_OSDDrv_dfb_init()
{
    OSAL_T_CTSK     t_ctsk;
    t_ctsk.stksz    = 0x4000;
    t_ctsk.quantum  = 10;
    t_ctsk.itskpri  = OSAL_PRI_NORMAL;
    t_ctsk.name[0]  = 'D';
    t_ctsk.name[1]  = 'F';
    t_ctsk.name[2]  = 'I';
    t_ctsk.task = (FP)dfb_osd_init_wrapper;
    osal_task_create(&t_ctsk);
}
#endif

void HLD_OSDDrv_Attach(void)
{
    int i = 0;

    memset((void *)&m_osd_dev, 0, sizeof(struct osd_device *) * OSD_MAX_DEV_NUM);
    memset((void *)&m_osd_priv, 0, sizeof(struct osd_private *) * OSD_MAX_DEV_NUM);

#if 0//def ADR_IPC_ENABLE
	m_osd_mutex_id = adr_ipc_semget(ADR_IPC_OSD, 0, 1);
	if(m_osd_mutex_id < 0)
	{
		//OSD_PRF("create mutex fail\n");
		return;
	}
#endif

#ifdef DIRECT_FB_OSD_SUPPORT
#ifndef _CAS9_CA_ENABLE_
#define APP_INIT_PARALLIZE
#endif
    #ifdef APP_INIT_PARALLIZE
    HLD_OSDDrv_dfb_init();
    #else
    dfb_osd_init();
    b_dfb_init_over = 1;
    #endif
#endif

    for(i = 0;i < OSD_MAX_DEV_NUM;i++)
    {
        struct osd_device *osd_dev;
        struct osd_private *priv;

        osd_dev = dev_alloc(m_osd_name[i],HLD_DEV_TYPE_OSD,sizeof(struct osd_device));
        if(osd_dev == NULL)
        {
            OSD_PRF("malloc osd dev fail\n");
            return;
        }

        priv = malloc(sizeof(*priv));
        if(priv == NULL)
        {
            OSD_PRF("malloc osd priv fail\n");
        }
        
        memset((void *)priv, 0, sizeof(*priv));
        priv->file_path = (i == 0) ? fb0_path : fb2_path;
        priv->index = i;
        osd_dev->priv = (void *)priv;
        osd_dev->next = NULL;
        osd_dev->flags = 0;
        
        OSD_PRF("file path %s\n", priv->file_path);
        
        if(dev_register(osd_dev) != RET_SUCCESS)
        {
            OSD_PRF("register osd dev fail\n");
            return;
        }
                
        m_osd_dev[i] = osd_dev;
        OSD_PRF("attach hld osd dev %d ok\n", i);

        #ifdef GE_SIMULATE_OSD
        osd_dev->flags = i;
        #endif
    }

//#ifdef GE_SIMULATE_OSD
#ifdef GE_DRAW_OSD_LIB
    ge_m36f_attach(NULL, 0);
    m_osddrv_ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
#endif
}

void HLD_OSDDrv_Dettach(void)
{
    struct osd_private *priv;   
    int i = 0;
    
#ifdef DIRECT_FB_OSD_SUPPORT
    dfb_osd_exit();
#endif
    
    for(i = 0;i < OSD_MAX_DEV_NUM;i++)
    {
        if(m_osd_dev[i] != 0)
        {
            priv = (struct osd_private *)m_osd_dev[i]->priv;
            close(priv->handle);
            free(priv);
            dev_free(m_osd_dev[i]);
        }
        
        m_osd_dev[i] = NULL;
    }

//#ifdef GE_SIMULATE_OSD
#ifdef GE_DRAW_OSD_LIB
    if (m_osddrv_ge_dev)
        ge_m36f_detach(m_osddrv_ge_dev);

    OSDDrv_ExitMemMap();
#endif    
}

