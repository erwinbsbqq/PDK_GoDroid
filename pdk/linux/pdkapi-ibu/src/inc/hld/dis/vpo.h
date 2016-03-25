#ifndef	_VPOAPI_H_
#define	_VPOAPI_H_


#include <sys_config.h>
#include <mediatypes.h>
#include <alidefinition/adf_vpo.h>
//#include <bus/hdmi/m36/hdmi_api.h>

#if 0
//

#ifdef	FUNC_MONITOR
//#define	VP_FUNC_PRINTF	DEBUGPRINTF
#define	VP_FUNC_PRINTF(...)

#else
#define	VP_FUNC_PRINTF(...)
#endif

#ifndef  BYTE
#define  BYTE           unsigned char
#endif

#include <basic_types.h>

#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/ge/ge.h>

typedef RET_CODE (*OSD_IO_CTL)(HANDLE ,UINT32 ,UINT32 );
typedef RET_CODE (* cb_ge_show_onoff)(struct ge_device *, ge_surface_desc_t *, UINT8 );
typedef RET_CODE (* cb_ge_blt_ex)(struct ge_device *, ge_surface_desc_t *,   ge_surface_desc_t *,  ge_surface_desc_t *, ge_rect_t *, ge_rect_t *, ge_rect_t *, UINT32 );
typedef ge_surface_desc_t * (*cb_ge_create_surface)(struct ge_device * , ge_surface_desc_t * , UINT32, UINT8);
typedef RET_CODE (*cb_ge_create_region)(struct ge_device * , ge_surface_desc_t * , UINT8 , ge_rect_t * , ge_region_t * );
typedef RET_CODE (*GE_DEO_IO_CTL)(HANDLE ,UINT32 ,UINT32 );

enum VP_display_layer_29e
{
	VPO_LAYER_M = 1,
	VPO_LAYER_PIP,
	VPO_LAYER_OSD,
	VPO_LAYER_ST1,  // 3101F
	VPO_LAYER_ST2,  // 3101F
};

enum VP_LAYER_ORDER
{

    // 3101F
    VPO_MP_PIP_OSD_ST1_ST2 = 0x00,
    VPO_MP_PIP_ST1_OSD_ST2 = 0x01,
};
enum VP_AFD_SOLUTION 
{
	VP_AFD_COMMON,
	VP_AFD_MINGDIG,
};

enum VP_LAYER_BLEND_ORDER
{
	MP_GMAS_GMAF_AUXP =0,
	MP_GMAS_AUXP_GMAF, 
	MP_GMAF_GMAS_AUXP, 
	MP_GMAF_AUXP_GMAS,		
	MP_AUXP_GMAS_GMAF, 	
	MP_AUXP_GMAF_GMAS, 	
};

enum VP_TVESDHD_SOURCE
{
	TVESDHD_SRC_DEN,
	TVESDHD_SRC_DEO,
};


enum VPO_DIT_LEVEL
{
	VPO_DIT_LEVEL_HIGH ,
	VPO_DIT_LEVEL_MEDIUM,
	VPO_DIT_LEVEL_LOW,
};

/*
struct vpo_status_info
{
	UINT8	case_index;
	UINT8	status_index;
	UINT32	display_frm_step;
};*/

//3For tve-end

//3io_control
//#ifdef VIDEO_S3601
/*
struct vp_io_reg_callback
{
	UINT8	callback_type;
	void *	callback_func;
};
*/
//#endif

typedef void (* VP_SRCASPECT_CHANGE)(enum AspRatio, enum TVMode);
typedef void (* VP_SRCASPRATIO_CHANGE)(UINT8);
typedef BOOL (* VP_MPGetMemInfo)(UINT32 *);
//End
typedef void (*VP_CB_VSYNC_CALLBACK)(UINT32 );
struct vpo_callback
{
    VP_SRCASPRATIO_CHANGE	pSrcAspRatioChange_CallBack;
	OSAL_T_HSR_PROC_FUNC_PTR	phdmi_callback;
};


struct VP_Feature_Config
{
	BOOL bOsdMulitLayer;
	BOOL bMHEG5Enable;
	BOOL bAvoidMosaicByFreezScr;
	BOOL bSupportExtraWin;
	BOOL bOvershootSolution;
	BOOL bP2NDisableMAF;
	BOOL bADPCMEnable;
	VP_MPGetMemInfo pMPGetMemInfo;	
	VP_SRCASPRATIO_CHANGE	pSrcAspRatioChange_CallBack;
};

struct VP_direct_output
{
	UINT8	direct_output_array_len;
	UINT8 	reserved;
	UINT16	direct_output_array[5];
};

struct vpo_device
{
	struct vpo_device  *next;  /*next device */

    	UINT32 type;
	INT8 name[32];

	void *priv;		/* Used to be 'private' but that upsets C++ */
	void *priv_pip;
	UINT32 reserved;
	
	RET_CODE	(*open)(struct vpo_device *, struct VP_InitInfo *);
	RET_CODE   	(*close)(struct vpo_device *);
	RET_CODE   	(*ioctl)(struct vpo_device *,UINT32,UINT32);
	RET_CODE   	(*zoom)(struct vpo_device *,struct Rect *, struct Rect *);
	RET_CODE   	(*aspect_mode)(struct vpo_device *,enum TVMode, enum DisplayMode);
	RET_CODE 	(*tvsys)(struct vpo_device *, enum TVSystem, BOOL );
	RET_CODE 	(*win_onoff)(struct vpo_device *, BOOL);
	RET_CODE 	(*win_mode)(struct vpo_device *, BYTE, struct MPSource_CallBack *, struct PIPSource_CallBack *);
	RET_CODE 	(*config_source_window)(struct vpo_device *, struct vp_win_config_para *);
	RET_CODE 	(*set_progres_interl)(struct vpo_device *, BOOL);
	
};
void dvi_api_EnableInterrupt(UINT8 uDVIScanMode, UINT8 uInterrupt);

//Add for VE pause/slow/ff play mode, DE change DIT mode to increase display quality
enum VP_PlayMode
{
	NORMAL_PLAY,
	NORMAL_2_ABNOR,
	ABNOR_2_NORMAL
};

struct vpo_io_register_ge_callback
{
	cb_ge_show_onoff		ge_show_onoff;
	cb_ge_blt_ex			ge_blt_ex;
	cb_ge_create_region		ge_create_region;
	cb_ge_create_surface	ge_create_surface;	
};

struct vpo_io_ttx
{
        UINT8 LineAddr;
        UINT8 Addr;
        UINT8 Data;
};

struct vpo_io_cc
{
		UINT8 FieldParity;
		UINT16 Data;
};

struct vpo_io_global_alpha
{	
	UINT8 value;
	UINT8 valid;
};



/*
#define SYS_576I	0
#define SYS_480I	1
#define SYS_576P	2
#define SYS_480P	3
#define SYS_720P_50		4
#define SYS_720P_60		5
#define SYS_1080I_25	6
#define SYS_1080I_30	7
*/
typedef enum _eTV_SYS
{
    SYS_576I	,       
    SYS_480I	,       
    SYS_576P	,       
    SYS_480P	,       
    SYS_720P_50	,       
    SYS_720P_60	,       
    SYS_1080I_25,       
    SYS_1080I_30,       
                        
    SYS_1080P_24,       
    SYS_1080P_25,       
    SYS_1080P_30,       
                        
    SYS_1152I_25,       
    SYS_1080IASS,       
                        
    SYS_1080P_50,       
    SYS_1080P_60,       

    TVE_SYS_NUM,
}T_eTVE_SYS;




/**************************API SubFunction Begin********************************************/

void HLD_vpo_attach(void);
void HLD_vpo_dettach(void);

RET_CODE vpo_open(struct vpo_device *dev,struct VP_InitInfo *pInitInfo);
RET_CODE vpo_close(struct vpo_device *dev);
RET_CODE vpo_win_onoff(struct vpo_device *dev,BOOL bOn);
RET_CODE vpo_win_onoff_ext(struct vpo_device *dev,BOOL bOn,enum vp_display_layer layer);
RET_CODE vpo_win_mode(struct vpo_device *dev, UINT8 bWinMode, struct MPSource_CallBack *pMPCallBack,struct PIPSource_CallBack *pPIPCallBack);
RET_CODE vpo_zoom(struct vpo_device *dev, struct Rect *pSrcRect , struct Rect *pDstRect);
RET_CODE vpo_zoom_ext(struct vpo_device *dev, struct Rect *pSrcRect , struct Rect *pDstRect,enum vp_display_layer layer);
RET_CODE vpo_aspect_mode(struct vpo_device *dev, enum TVMode eTVAspect, enum DisplayMode e169DisplayMode);

RET_CODE vpo_tvsys_ex(struct vpo_device *dev, enum TVSystem eTVSys,BOOL bprogressive);
RET_CODE vpo_ioctl(struct vpo_device *dev, UINT32 dwCmd, UINT32 dwParam);

RET_CODE vpo_set_progres_interl(struct vpo_device *dev, BOOL bProgressive);

#endif

#endif

