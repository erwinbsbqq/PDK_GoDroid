#ifndef __ADF_OSD__
#define __ADF_OSD__

#ifdef __cplusplus
extern "C" {
#endif

#include "adf_basic.h"

/*! @addtogroup osd
 *  @{
 */

/*! @enum OSDSys
@brief ����ĵ�����ʽ����Scale���ʹ�á�
*/
enum OSDSys
{
	OSD_PAL = 0,
	OSD_NTSC
};

/*! @enum OSDColorMode
@brief ��ɫ��ʽ��
*/
enum OSDColorMode{
	OSD_4_COLOR =	0,
	OSD_16_COLOR,
	OSD_256_COLOR,
	OSD_16_COLOR_PIXEL_ALPHA,
	OSD_HD_ACLUT88,
	OSD_HD_RGB565,
	OSD_HD_RGB888,
	OSD_HD_RGB555,
	OSD_HD_RGB444,
	OSD_HD_ARGB565,
	OSD_HD_ARGB8888,
	OSD_HD_ARGB1555,
	OSD_HD_ARGB4444,
	OSD_HD_AYCbCr8888,
	OSD_HD_YCBCR888,
	OSD_HD_YCBCR422,
	OSD_HD_YCBCR422MB,
	OSD_HD_YCBCR420MB,
	OSD_COLOR_MODE_MAX
};

/*! @enum CLIPMode
@brief CLIPģʽ��
*/
enum CLIPMode
{
	CLIP_INSIDE_RECT = 0,
	CLIP_OUTSIDE_RECT,
	CLIP_OFF
};

#define	OSDDRV_RGB			0x00   //!< RGB ��ɫ��ARGB   byte order is: {B, G , R , A}, A in [0, 255]
#define	OSDDRV_YCBCR			0x01   //!< YUV ��ɫ��AYCbCr byte order is: {Y, Cb, Cr, A}, A in [0, 15 ]

#define	OSDDRV_OFF				0x00 //!< ��ʾ��رա�
#define	OSDDRV_ON					0x01 //!< ��ʾ��򿪡�


/**************** the position and size of OSD frame buffer********************/
#define	OSD_MIN_TV_X		0x20
#define	OSD_MIN_TV_Y		0x10
#define	OSD_MAX_TV_WIDTH	0x2B0
#define	OSD_MAX_TV_HEIGHT	0x1F0 //old value 0x1E0


#define P2N_SCALE_DOWN  	0x00
#define N2P_SCALE_UP		0x01
#define OSD_SCALE_MODE		P2N_SCALE_DOWN


//#define P2N_SCALE_IN_NORMAL_PLAY
#define P2N_SCALE_IN_SUBTITLE_PLAY

// scaler
#define OSD_VSCALE_OFF          0x00
#define OSD_VSCALE_TTX_SUBT     0x01
#define OSD_VSCALE_GAME         0x02
#define OSD_VSCALE_DVIEW        0x03
#define OSD_HDUPLICATE_ON       0x04
#define OSD_HDUPLICATE_OFF      0x05
#define OSD_OUTPUT_1080         0x06 // 720x576(720x480)->1920x1080
#define OSD_OUTPUT_720          0x07 // 720x576(720x480)->1280x720
#define OSD_HDVSCALE_OFF        0x08 // 1280x720->720x576(720x480)
#define OSD_HDOUTPUT_1080       0x09 // 1280x720->1920x1080
#define OSD_HDOUTPUT_720        0x0A // 1280x720->1280x720
#define OSD_SET_SCALE_MODE      0x0B // filter mode or duplicate mode
#define OSD_SCALE_WITH_PARAM    0x0C // Suitable for any case. see struct osd_scale_param
#define OSD_VSCALE_CC_SUBT	0X0D // ATSC CC for HD output scale

#define OSD_SCALE_WITH_PARAM_DEO    0x1000 // set scale parameter for sd output when dual output is enabled. see struct osd_scale_param

#define   OSD_SOURCE_PAL        0
#define   OSD_SOURCE_NTSC       1

#define   OSD_SCALE_DUPLICATE   0
#define   OSD_SCALE_FILTER      1

#define   OSD_SET_ENHANCE_BRIGHTNESS 0x01    // value[0, 100], default 50
#define   OSD_SET_ENHANCE_CONTRAST   0x02    // value[0, 100], default 50
#define   OSD_SET_ENHANCE_SATURATION	0x04    // value[0, 100], default 50
#define   OSD_SET_ENHANCE_SHARPNESS  0x08    // value[0, 10 ], default 5
#define   OSD_SET_ENHANCE_HUE        0x10    // value[0, 100], default 50

// io command
#define	OSD_IO_UPDATE_PALLETTE	0x00
#define OSD_IO_ADJUST_MEMORY    0x01
#define OSD_IO_SET_VFILTER      0x02
#define OSD_IO_RESPOND_API	 0X03
#define OSD_IO_DIS_STATE	 0X04
#define OSD_IO_SET_BUF_CACHEABLE 0X05
#define OSD_IO_16M_MODE			0X06
#define OSD_IO_SET_TRANS_COLOR  0x07
#define OSD_IO_SET_ANTI_FLICK_THRE  0x0F
#define   OSD_IO_ENABLE_ANTIFLICK		0x10
#define   OSD_IO_DISABLE_ANTIFLICK		0x11

#define   OSD_IO_SWITCH_DEO_LAYER		0x12
#define   OSD_IO_SET_DEO_AUTO_SWITCH	0x13

#define   OSD_IO_GET_RESIZE_PARAMATER   0x14
#define   OSD_IO_SET_RESIZE_PARAMATER   0x15


#define OSD_IO_ELEPHANT_BASE 0x10000
#define OSD_IO_SWITH_DATA_TRANSFER_MODE (OSD_IO_ELEPHANT_BASE + 0x01)
#define OSD_IO_SET_ANTIFLK_PARA         (OSD_IO_ELEPHANT_BASE + 0x02)

#define OSD_IO_SET_GLOBAL_ALPHA         (OSD_IO_ELEPHANT_BASE + 0x03) /* dwParam [0x00, 0xff] */

#define OSD_IO_GET_ON_OFF               (OSD_IO_ELEPHANT_BASE + 0x04) /* OSD layer show or hide(dwParam is UINT32 *) */
#define OSD_IO_SET_AUTO_CLEAR_REGION    (OSD_IO_ELEPHANT_BASE + 0x05) /* Enable/Disable filling transparent color in OSDDrv_CreateRegion().
                                                                         After Open(), default is TRUE. Set it before OSDDrv_CreateRegion().*/
/* Enable/Disable GE ouput YCBCR format to DE when source is CLUT8, clut8->ycbcr, not do color reduction
     only used when output is 576i/p or 480i/p*/
#define OSD_IO_SET_YCBCR_OUTPUT    (OSD_IO_ELEPHANT_BASE + 0x06) 

#define OSD_IO_SET_DISPLAY_ADDR    (OSD_IO_ELEPHANT_BASE + 0x07) 
#define OSD_IO_SET_MAX_PIXEL_PITCH	   (OSD_IO_ELEPHANT_BASE + 0x08)
#define OSD_IO_WRITE2_SUPPORT_HD_OSD   (OSD_IO_ELEPHANT_BASE + 0x09)
#define OSD_IO_SUBT_RESOLUTION     (OSD_IO_ELEPHANT_BASE + 0x0A)
#define OSD_IO_CREATE_REGION            (OSD_IO_ELEPHANT_BASE + 0x0b) /* Create region with external buffer, dwParam is pcosd_region_param */
#define OSD_IO_MOVE_REGION              (OSD_IO_ELEPHANT_BASE + 0x0c) /* Move the region created by OSD_IO_CREATE_REGION, dwParam is pcosd_region_param */
#define OSD_IO_GET_REGION_INFO          (OSD_IO_ELEPHANT_BASE + 0x0d) /* Get the region information , dwParam is posd_region_param */
#define OSD_IO_GET_DISPLAY_RECT         (OSD_IO_ELEPHANT_BASE + 0x0e) /* struct OSDRect * */
#define OSD_IO_SET_DISPLAY_RECT         (OSD_IO_ELEPHANT_BASE + 0x0f) /* struct OSDRect * */
#define OSD_IO_SET_ENHANCE_PAR          (OSD_IO_ELEPHANT_BASE + 0x10)

#define OSD_IO_LINUX_API_BASE		(0xF00000)
#define OSD_IO_NO_CLEAR_BUF		(OSD_IO_LINUX_API_BASE + 0x01)
#define OSD_IO_VIDEO_ENHANCE		(OSD_IO_LINUX_API_BASE + 0x02)
#define OSD_IO_DISABLE_ENHANCE	        (OSD_IO_LINUX_API_BASE + 0x03)
#define OSD_IO_GET_LAYER_INFO		(OSD_IO_LINUX_API_BASE + 0x04)

enum OSD_SUBT_RESOLUTION{
	OSD_SUBT_RESO_720X576 = 1,
	OSD_SUBT_RESO_720X480,
	OSD_SUBT_RESO_1280X720,
	OSD_SUBT_RESO_1920X1080
};

/*! @struct OSDPara
@brief ��ʼ��������
*/
struct OSDPara
{
	enum OSDColorMode eMode;//!< ��ɫ��ʽ��
	UINT8 uGAlphaEnable;//!< Ŀǰδʵ�֡�
	UINT8 uGAlpha;//!< Ŀǰδʵ�֡�
	UINT8 uPalletteSel;//!< Ŀǰδʵ�֡�
};

/*! @struct OSDRect
@brief ���γߴ��λ�á�
*/
struct OSDRect
{
	INT16	uLeft;//!< ���λ�á�
	INT16	uTop;//!< �ϱ�λ�á�
	INT16	uWidth;//!< ��ȡ�
	INT16	uHeight;//!< �߶ȡ�
};

/*! @struct tagVScr
@brief ��������
*/
typedef struct tagVScr
{
	struct OSDRect	vR; //!< ���Ρ�
	UINT8 	*lpbScr;//!< ��������ַ��
	UINT8	bBlockID;//!< Ŀǰδʵ�֡�
	BOOL	updatePending;//!< ˢ�±�־��ֻ��APPʹ�á�
    UINT8   bColorMode;//!< Ŀǰδʵ�֡�
	UINT8 bDrawMode;//!< Ŀǰδʵ�֡�
}VSCR,*lpVSCR;

/*! @struct _osd_scale_param
@brief ���Ų�����
*/
typedef struct _osd_scale_param
{
    UINT16 tv_sys;//!< Ŀǰδʵ�֡�
    UINT16 h_div;//!< ˮƽ��ϵ����
    UINT16 v_div;//!< ��ֱ��ϵ����
    UINT16 h_mul;//!< ˮƽ��ϵ����
    UINT16 v_mul;//!< ��ֱ��ϵ����
} osd_scale_param, *posd_scale_param;
typedef const osd_scale_param *pcosd_scale_param;

typedef struct _osd_resize_param
{
    INT32 h_mode;
    INT32 v_mode;
} osd_resize_param, *posd_resize_param;

typedef struct 
{
	UINT8 enable;
	UINT8 layer;
	UINT8 no_temp_buf; // not use temp buffer
	UINT8 reserved;
}osd_clut_ycbcr_out; /*output ycbcr to DE, source is clut format*/

typedef struct 
{
	UINT8 region_id; // region id
	UINT8 reserved[3];
	UINT32 disp_addr;  // buffer address to be displayed 
}osd_disp_addr_cfg;

#define OSD_Resize_Param    osd_resize_param
#define POSD_Resize_Param   posd_resize_param

/*! @struct _osd_region_param
@brief ���������
*/
typedef struct _osd_region_param
{
    UINT8   region_id;//!< ��������ֵ��
    UINT8   color_format;//!< ��ɫ��ʽ��
    UINT8   galpha_enable;//!< Ŀǰδʵ�֡�
    UINT8   global_alpha;//!< Ŀǰδʵ�֡�
    UINT8   pallette_sel;//!< Ŀǰδʵ�֡�
    UINT16  region_x;//!< ��������λ�á�
    UINT16  region_y;//!< ������ϱ�λ�á�
    UINT16  region_w;//!< ����Ŀ�ȡ�
    UINT16  region_h;//!< ����ĸ߶ȡ�
    UINT32  phy_addr;//!< ���򻺳����������ַ��
    UINT32  bitmap_addr;//!< ���򻺳����������ַ��
    UINT32  pixel_pitch;//!< ������гߴ硣��λΪ�ֽڡ�
    UINT32  bitmap_x;//!< Ŀǰδʵ�֡�
    UINT32  bitmap_y;//!< Ŀǰδʵ�֡�
    UINT32  bitmap_w;//!< Ŀǰδʵ�֡�
    UINT32  bitmap_h;//!< Ŀǰδʵ�֡�
} osd_region_param, *posd_region_param;
typedef const osd_region_param *pcosd_region_param;

/*! @struct _osd_layer_param
@brief ��ʾ�������
*/
typedef struct _osd_layer_param
{
	enum OSDColorMode mode;//!< ��ɫ��ʽ��
	
	void *mem_start;//!< �����������ַ��
	int mem_size;//!< ��������С��
	
	void *virtual_mem_start;//!< �����������ַ��
	int virtual_mem_size;//!< �����������ַ��

	int max_logical_width;//!< ����ȡ�
	int max_logical_height;//!< ���߶ȡ�
	int pitch;	//!< �г��ȡ����ֽ�Ϊ��λ��
}osd_layer_param, *posd_layer_param;

/*! @struct osd_io_video_enhance
@brief OSD_IO_VIDEO_ENHANCE��Ҫ�Ĳ������塣����ͼ����ǿ��Ϣ��
*/
struct osd_io_video_enhance
{
	UINT8	changed_flag; //!< ͼ����ǿ�����͡�OSD_IO_SET_ENHANCE_XX��ʾ��ǿ�����͡�
	UINT16   grade; //!< ͼ����ǿ��������ֵ����ΧΪ0 ~ 100��Ĭ��50����ʾ������ǿ����
};

/*!
@brief �ͷ�osd ģ�顣
*/
void HLD_OSDDrv_Dettach(void);

/*!
@brief ��osd ģ�顣
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[in] ptPara ��ʼ��������
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE OSDDrv_Open(HANDLE hDev,struct OSDPara *ptPara);

/*!
@brief �ر�osd  ģ�顣
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE OSDDrv_Close(HANDLE hDev);


/*!
@brief ��ȡ��ʼ��������
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[in] ptPara ��ʼ��������
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE OSDDrv_GetPara(HANDLE hDev,struct OSDPara* ptPara);

/*!
@brief ������ʾ�㡣
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[in] uOnOff �򿪻��߹رյı�־��Ϣ��OSDDRV_OFF��ʾ�رա�OSDDRV_ON��ʾ�򿪡�
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE OSDDrv_ShowOnOff(HANDLE hDev,UINT8 uOnOff);

/*!
@brief ������ɫ��
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[in] puPallette ������ɫ������ָ�롣
@param[in] uColorNum ��ɫ������ֻ֧��256ɫ��
@param[in] uType ��ɫ�����͡�֧��OSDDRV_YCBCR��OSDDRV_RGB��
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE OSDDrv_SetPallette(HANDLE hDev,UINT8 *puPallette,UINT16 uColorNum,UINT8 uType);

/*!
@brief ��ȡ��ɫ��
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[out] puPallette �����ɫ������ָ�롣
@param[in] uColorNum ��ɫ������ֻ֧��256ɫ��
@param[in] uType ��ɫ�����͡�֧��OSDDRV_YCBCR��OSDDRV_RGB��
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE OSDDrv_GetPallette(HANDLE hDev,UINT8 *puPallette,UINT16 uColorNum,UINT8 uType);

/*!
@brief �޸���ɫ��
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[in] uIndex ��ɫ�������ֵ��
@param[in] uY ����ֵ��
@param[in] uCb ��ɫɫ��ֵ��
@param[in] uCr ��ɫɫ��ֵ��
@param[uK] uK ͸����ֵ��
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE OSDDrv_ModifyPallette(HANDLE hDev,UINT8 uIndex,UINT8 uY,UINT8 uCb,UINT8 uCr,UINT8 uK);

/*!
@brief ����osd����
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[in] uRegionId ���������ֵ��
@param[in] rect ����ĳߴ��λ����Ϣ��
@param[in] pOpenPara Ŀǰδʵ�� �����Դ���NULL��
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE OSDDrv_CreateRegion(HANDLE hDev,UINT8 uRegionId,struct OSDRect* rect,struct OSDPara*pOpenPara);

/*!
@brief ɾ��osd����
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[in] uRegionId ���������ֵ��
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE OSDDrv_DeleteRegion(HANDLE hDev,UINT8 uRegionId);

/*!
@brief ����osd�������λ�á�
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[in] uRegionId ���������ֵ��
@param[in] rect �������λ����Ϣ��ֻ�����޸�λ����Ϣ��
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE OSDDrv_SetRegionPos(HANDLE hDev,UINT8 uRegionId,struct OSDRect* rect);

/*!
@brief ��ȡosd�����λ����Ϣ��
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[in] uRegionId ���������ֵ��
@param[out] rect �����λ����Ϣ��
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE OSDDrv_GetRegionPos(HANDLE hDev,UINT8 uRegionId,struct OSDRect* rect);

/*!
@brief ������д�����ݡ�
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[in] uRegionId ���������ֵ��
@param[in] pVscr ����д�����ݵ���������
@param[in] rect д��������εĳߴ��λ����Ϣ��
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE OSDDrv_RegionWrite(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct OSDRect *rect);

/*!
@brief �������ȡ���ݡ�
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[in] uRegionId ���������ֵ��
@param[out] pVscr �����ȡ���ݵ���������
@param[in] rect ��ȡ�������εĳߴ��λ����Ϣ��
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE OSDDrv_RegionRead(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct OSDRect *rect);

/*!
@brief ��������дָ����ɫ��
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[in] uRegionId ���������ֵ��
@param[in] rect д��������εĳߴ��λ����Ϣ��
@param[in] uColorData ָ����ɫ��
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE OSDDrv_RegionFill(HANDLE hDev,UINT8 uRegionId,struct OSDRect *rect, UINT32 uColorData);

/*!
@brief �������Ų�����
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[in] uScaleCmd ���ŵ����
@param[in] uScaleParam ���ŵĲ�����
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
@note  ��������uScaleCmd ����:
<table class="doxtable"  width="800" border="1" style="border-collapse:collapse;table-layout:fixed;word-break:break-all;" >
  <tr>
    <th width="200">����</th>
    <th width="200">����</th>
    <th width="80">��������</th>
    <th width="320">��ע</th>
  </tr>

  <tr align="center">
    <td>OSD_VSCALE_OFF</td>    
    <td>��</td>    
    <td>����</td>
    <td>ʵ��1:1���</td>
  </tr>

   <tr align="center">
    <td>OSD_SET_SCALE_MODE</td>    
    <td>��</td>    
    <td>����</td>
    <td>��������ģʽ��ֻ����Ϊ����ģʽ</td>
  </tr> 

   <tr align="center">
    <td>OSD_SCALE_WITH_PARAM</td>    
    <td>osd_scale_param *</td>    
    <td>����</td>
    <td>�������ű����Ĳ���</td>
  </tr>      
*/
RET_CODE OSDDrv_Scale(HANDLE hDev, UINT32 uScaleCmd,UINT32 uScaleParam);

/*!
@brief osd ģ���io contorl ������
@param[in] hDev ָ��osd ģ�� ��ָ�롣
@param[in] dwCmd ���� ���������͡��ο�OSD_IO_XX���塣
@param[in,out] dwParam �����Ĳ��������ݲ�ͬ��������в�ͬ�Ĳ�����
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
@note  IO����dwCmd ����:
<table class="doxtable"  width="800" border="1" style="border-collapse:collapse;table-layout:fixed;word-break:break-all;" >
  <tr>
    <th width="200">����</th>
    <th width="200">����</th>
    <th width="80">��������</th>
    <th width="320">��ע</th>
  </tr>

  <tr align="center">
    <td>OSD_IO_SET_TRANS_COLOR</td>    
    <td>UINT8</td>    
    <td>����</td>
    <td>����͸��ɫ����ֵ��ֻ����ɫ��ʽΪCLUT8ʱ��Ч</td>
  </tr>

  <tr align="center">
    <td>OSD_IO_SET_GLOBAL_ALPHA</td>    
    <td>UINT8</td>    
    <td>����</td>
    <td>������ʾ���͸���ȡ�0xFF��ʾ��ȫ��͸����0��ʾȫ͸����</td>
  </tr>

  <tr align="center">
    <td>OSD_IO_GET_REGION_INFO</td>    
    <td>posd_region_param </td>    
    <td>���</td>
    <td>��ȡ�������Ϣ</td>
  </tr>  
  
   <tr align="center">
    <td>OSD_IO_GET_LAYER_INFO</td>    
    <td>posd_layer_param</td>    
    <td>���</td>
    <td>��ȡ��ʾ��Ĳ���</td>
  </tr>    
*/
RET_CODE OSDDrv_IoCtl(HANDLE hDev,UINT32 dwCmd,UINT32 dwParam);

/*!
@}
*/

#ifdef __cplusplus
}
#endif

#endif

