#ifndef	__HDMI_API_H__
#define __HDMI_API_H__

#include <adr_retcode.h>
#include <adr_mediatypes.h>
#include <osal/osal.h>
#include <hld/adr_hld_dev.h>
#include <hld/bus/hdmi/m36/hdmi_dev.h>


#ifdef __cplusplus
extern "C" {
#endif

/*! @addtogroup Devciedriver
 *  @{
 */
 
/*! @addtogroup ALiHDMI
 *  @{
 */


/*!@enum HDMI_API_RES
@brief An enum defines HDMI resolutions.
*/
enum HDMI_API_RES
{
	HDMI_RES_INVALID = 0,
	HDMI_RES_480I,
	HDMI_RES_480P,
	HDMI_RES_576I,
	HDMI_RES_576P,
	HDMI_RES_720P_50,
	HDMI_RES_720P_60,
	HDMI_RES_1080I_25,
	HDMI_RES_1080I_30,
	HDMI_RES_1080P_24,
	HDMI_RES_1080P_25,
	HDMI_RES_1080P_30,
	HDMI_RES_1080P_50,
	HDMI_RES_1080P_60,
	HDMI_RES_4096X2160_24,
	HDMI_RES_3840X2160_24,
	HDMI_RES_3840X2160_25,
	HDMI_RES_3840X2160_30,
};

/*!@enum EDID_AUD_FMT_CODE
@brief A enum defines audio information in EDID.
*/
enum EDID_AUD_FMT_CODE
{
	EDID_AUDIO_LPCM 				= 0x0001,
	EDID_AUDIO_AC3					= 0x0002,
	EDID_AUDIO_MPEG1				= 0x0004,
	EDID_AUDIO_MP3					= 0x0008,
	EDID_AUDIO_MPEG2				= 0x0010,
	EDID_AUDIO_AAC					= 0x0020,
	EDID_AUDIO_DTS					= 0x0040,
	EDID_AUDIO_ATRAC				= 0x0080,
	EDID_AUDIO_ONEBITAUDIO			= 0x0100,
	EDID_AUDIO_DD_PLUS				= 0x0200,
	EDID_AUDIO_DTS_HD				= 0x0400,
	EDID_AUDIO_MAT_MLP				= 0x0800,
	EDID_AUDIO_DST					= 0x1000,
	EDID_AUDIO_WMAPRO				= 0x2000,
};

/*!@enum HDMI_API_DEEPCOLOR
@brief An enum defines HDMI DeepColor.
*/
enum HDMI_API_DEEPCOLOR
{	
	HDMI_DEEPCOLOR_24 = 0,	
	HDMI_DEEPCOLOR_30,	
	HDMI_DEEPCOLOR_36,	
	HDMI_DEEPCOLOR_48,
};

/*!@enum HDMI_API_COLOR_SPACE
@brief An enum defines HDMI ColorSpace.
*/
enum HDMI_API_COLOR_SPACE
{
	HDMI_RGB = 0,
	HDMI_YCBCR_422,
	HDMI_YCBCR_444,
};

/*!@enum EDID_DEEPCOLOR_CODE
@brief An enum defines HDMI DeepColor.
*/
enum EDID_DEEPCOLOR_CODE
{	
	EDID_DEEPCOLOR_24 				= 0x01,
	EDID_DEEPCOLOR_30				= 0x02,
	EDID_DEEPCOLOR_36				= 0x04,
	EDID_DEEPCOLOR_48				= 0x08,
	EDID_DEEPCOLOR_Y444				= 0x10,
};

/*!@enum HDMILinkStatus_E
@brief A enum defines HDMI Link Status
*/
typedef enum
{
	HDMI_STATUS_UNLINK = 0x01,				/**< HDMI not link */
	HDMI_STATUS_LINK_HDCP_SUCCESSED = 0x02,
	HDMI_STATUS_LINK_HDCP_FAILED = 0x04,
	HDMI_STATUS_LINK_HDCP_IGNORED = 0x08,
	HDMI_STATUS_MAX = 0x10,
	HDMI_STATUS_LINK = 0x20,
}HDMILinkStatus_E;

/*!
@brief The function is used for opening HDMI driver.
@param[in] dev: A pointer to HDMI device.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE hdmi_tx_open(struct hdmi_device *dev);

/*!
@brief The function is used for attaching HDMI driver.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE hdmi_tx_attach (void);

/*!
@brief An API function from application to close HDMI driver.
@param[in] dev: A pointer to HDMI device.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE hdmi_tx_close(struct hdmi_device *dev);

/*!
@brief The function is used for detaching HDMI driver.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE hdmi_tx_detach (void);

/*!
@brief The function is used for getting EDID support video color format.
@param[in] dev: A pointer to HDMI device.
@param[in] format: A pointer to video color format. Please refer to enum PicFmt.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed ;
*/
RET_CODE api_get_edid_video_format(struct hdmi_device *dev, enum PicFmt *format);

/*!
@brief The function is used for getting video native resolution.
@param[in] dev: A pointer to HDMI device.
@param[in] res: Video reslution. Please refer to enum #HDMI_API_RES.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_get_edid_video_resolution(struct hdmi_device *dev, enum HDMI_API_RES *res);

/*!
@brief The function is used for getting EDID all video resolution.
@param[in] dev: A pointer to HDMI device.
@param[in] native_res_index: A pointer to native resolution index.
@param[in] video_res: Video reslution. Please refer to enum #HDMI_API_RES.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_get_edid_all_video_resolution(struct hdmi_device *dev, UINT32 *native_res_index, enum HDMI_API_RES *video_res);

/*!
@brief The function is used for getting EDID prefer audio coding.
@param[in] dev: A pointer to HDMI device.
@param[in] aud_fmt: Audio format. Please refer to enum #EDID_AUD_FMT_CODE.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_get_edid_audio_out(struct hdmi_device *dev, enum EDID_AUD_FMT_CODE *aud_fmt);

/*!
@brief The function is used for getting EDID all audio coding.
@param[in] dev: A pointer to HDMI device.
@param[in] aud_fmt: Video reslution. Please refer to enum #EDID_AUD_FMT_CODE.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_get_edid_all_audio_out(struct hdmi_device *dev, enum EDID_AUD_FMT_CODE *aud_fmt);

/*!
@brief The function is used for setting HDMI switch on or off.
@param[in] dev: A pointer to HDMI device.
@param[in] bOnOff: HDMI turn on if bOnOff = true; HDMI turn off if bOnOff = false;
*/
void api_set_hdmi_sw_onoff(struct hdmi_device *dev, BOOL bOnOff);

/*!
@brief The function is used for getting HDMI on/off status.
@param[in] dev: A pointer to HDMI device.
@return
@retval TRUE: HDMI turn on;
@retval FALSE: HDMI turn off;
*/
BOOL api_get_hdmi_sw_onoff(struct hdmi_device *dev);

/*!
@brief The function is used for setting HDMI HDCP authentication on/off.
@param[in] dev: A pointer to HDMI device.
@param[in] bOnOff: HDMI HDCP turn on if bOnOff = true; HDMI HDCP turn off if bOnOff = false;
*/
void api_set_hdmi_hdcp_onoff(struct hdmi_device *dev, BOOL bOnOff);

/*!
@brief The function is used for getting HDMI HDCP authentication on/off.
@param[in] dev: A pointer to HDMI device.
@return
@retval TRUE: HDMI HDCP on;
@retval FALSE: HDMI HDCP off;
*/
BOOL api_get_hdmi_hdcp_onoff(struct hdmi_device *dev);

/*!
@brief The function is used for setting HDMI HDCP key mem_sel.
@param[in] dev: A pointer to HDMI device.
@param[in] mem_sel: HDMI HDCP ce_load_key if mem_sel = true; HDMI HDCP se_load_key mem_sel = false;
*/
void api_set_hdmi_mem_sel(struct hdmi_device *dev, BOOL mem_sel);//add by ze for ce_load_key or sw_load_key

/*!
@brief The function is used for setting HDMI CEC feature on/off.
@param[in] dev: A pointer to HDMI device.
@param[in] bOnOff: HDMI CEC turn on if bOnOff = true; HDMI CEC turn off if bOnOff = false;
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_set_hdmi_cec_onoff(struct hdmi_device *dev, BOOL bOnOff);

/*!
@brief An API function from application to getting HDMI CEC status on/off.
@param[in] dev: A pointer to HDMI device.
@return
@retval TRUE: HDMI CEC on;
@retval FALSE: HDMI CEC off;
*/
BOOL api_get_hdmi_cec_onoff(struct hdmi_device *dev);

/*!
@brief The function is used for setting transmitting CEC message.
@param[in] dev: A pointer to HDMI device.
@param[in] message: A pointer to HDMI CEC message;
@param[in] message_length: Message length;
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_hdmi_cec_transmit(struct hdmi_device *dev, UINT8* message, UINT8 message_length);

/*!
@brief The function is used for setting getting the HDMI physical address for CEC.
@param[in] dev: A pointer to HDMI device.
@return
@retval physical Address of HDMI deivce;
@retval 0xFFFF ; The value is 0xFFFF if operation failed.
*/
UINT16	api_get_physical_address(struct hdmi_device *dev);

/*!
@brief An API function from application to set the HDMI logical address for CEC.
@param[in] dev: A pointer to HDMI device.
@param[in] logical_address: Logical address of HDMI device.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_set_logical_address(struct hdmi_device *dev, UINT8 logical_address);

/*!
@brief The function is used for setting getting the HDMI logical address for CEC.
@param[in] dev: A pointer to HDMI device.
@return
@retval logical address of HDMI deivce;
@retval 0xFF; The value is 0xFFFF if operation failed.
*/
UINT8 api_get_logical_address(struct hdmi_device *dev);

/*!
@brief An An API function from application to get HDMI device counter.
@param[in] dev: A pointer to HDMI device.
@param[in] pnHdmiDeviceCount: A pointer to HDMI device counter.
@return
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_hdmi_get_dev_count(struct hdmi_device *dev, int * pnHdmiDeviceCount);

/*!
@brief The function is used for setting HDMI vendor name.
@param[in] dev: A pointer to HDMI device.
@param[in] vendor_name: A pointer to HDMI vendor name.
@param[in] length: Length of vendor name.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_hdmi_set_vendor_name(struct hdmi_device *dev, unsigned char *vendor_name, unsigned char length);

/*!
@brief The function is used for setting setting HDMI product description.
@param[in] dev: A pointer to HDMI device.
@param[in] product_desc: A pointer to HDMI product description.
@param[in] length: Length of product description.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_hdmi_set_product_desc(struct hdmi_device *dev, unsigned char *product_desc, unsigned char length);

/*!
@brief The function is used for getting HDMI vendor name.
@param[in] dev: A pointer to HDMI device.
@param[in] vendor_name: A pointer to HDMI vendor name.
@param[in] length: A pointer to length of vendor name.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_hdmi_get_vendor_name(struct hdmi_device *dev, unsigned char *vendor_name, unsigned char *length);

/*!
@brief The function is used for getting HDMI product description.
@param[in] dev: A pointer to HDMI device.
@param[in] product_desc: A pointer to HDMI product description.
@param[in] length:  A pointer to length of product description.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_hdmi_get_product_desc(struct hdmi_device *dev, unsigned char *product_desc, unsigned char *length);

/*!
@brief The function is used for getting HDMI link status.
@param[in] dev: A pointer to HDMI device.
@param[in] link_status: A pointer to HDMI link_status. Please refer to enum HDMILinkStatus_E
@return
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_hdmi_get_link_status(struct hdmi_device *dev, int *link_status);

/*!
@brief The function is used for registering HDMI call back function.
@param[in] dev: A pointer to HDMI device.
@param[in] cb_func: A pointer to call back function;
@param[in] pvUserData: A pointer to user data.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_hdmi_reg_callback(struct hdmi_device *dev, void *cb_func, void *pvUserData);

/*!
@brief The function is used for deleting HDMI call back function.
@param[in] dev: A pointer to HDMI device.
@param[in] cb_func: A pointer to call back function;
@param[in] pvUserData: A pointer to user data.
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_hdmi_del_callback(struct hdmi_device *dev, void *cb_func, void *pvUserData);

/*!
@brief The function is used for setting HDMI audio on/off.
@param[in] dev: A pointer to HDMI device.
@param[in] bOnOff: HDMI audio turn on if bOnOff = true; HDMI audio turn off if bOnOff = false;
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_hdmi_audio_set_onoff(struct hdmi_device * dev, BOOL bOnOff);

/*!
@brief The function is used for getting HDMI audio on/off status.
@param[in] dev: A pointer to HDMI device.
@param[in] bOnOff: A pointer to HDMI audio status.
@return
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_hdmi_audio_get_onoff(struct hdmi_device * dev, int * bOnOff);


/*!
@brief The function is used for getting HDMI 3D present status.
@param[in] dev: A pointer to HDMI device.
@param[in] present: A pointer to 3D presentation from EDID.
@return
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/

RET_CODE api_hdmi_get_3d_present(struct hdmi_device *dev, int *present);

/*!
@brief The function is used for getting HDMI EDID manufacturer name.
@param[in] dev: A pointer to HDMI device.
@param[in] m_name:  A pointer to manufacturer name from EDID.
@return
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/

RET_CODE api_hdmi_get_edid_manufacturer(struct hdmi_device *dev, unsigned char * m_name);

/*!
@brief The function is used for getting HDMI EDID monitor name.
@param[in] dev: A pointer to HDMI device.
@param[in] m_name: A pointer to monitor name from EDID.
@return
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/

RET_CODE api_hdmi_get_edid_monitor(struct hdmi_device *dev, unsigned char * m_name);

/*!
@brief The function is used for setting HDMI deep color.
@param[in] dev: A pointer to HDMI device.
@param[in] dp_mode:  HDMI deep color mode parameter. Please refer to enum #HDMI_API_DEEPCOLOR;
@return
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/

RET_CODE api_hdmi_set_deep_color(struct hdmi_device *dev, enum HDMI_API_DEEPCOLOR dp_mode);

/*!
@brief The function is used for getting HDMI deep color.
@param[in] dev: A pointer to HDMI device.
@param[in] dp_mode: A pointer to HDMI deep color mode. Please refer to enum #HDMI_API_DEEPCOLOR;
@return
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/

RET_CODE api_hdmi_get_deep_color(struct hdmi_device *dev, enum HDMI_API_DEEPCOLOR *dp_mode);

/*!
@brief The function is used for getting HDMI EDID block data.
@param[in] dev: A pointer to HDMI device.
@param[in] num: EDID data block number.
@param[in] data: A pointer to EDID block information.
@return
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed
*/

RET_CODE api_hdmi_get_edid_block_data(struct hdmi_device *dev, int num, unsigned char *data);

/*!
@brief An API function from application to set HDMI phy clock onoff.
@param[in] dev: A pointer to HDMI device.
@param[in] bOnOff:  HDMI clock turn on if bOnOff = true; HDMI clock turn off if bOnOff = false;
@return
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/

RET_CODE api_hdmi_set_phy_clock_onoff(struct hdmi_device *dev, BOOL bOnOff);

/*!
@brief The function is used for setting HDMI color space.
@param[in] dev: A pointer to HDMI device.
@param[in] color_space: Video color space. Please refer to enum #HDMI_API_COLOR_SPACE;
@return
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/

RET_CODE api_hdmi_set_color_space(struct hdmi_device *dev, enum HDMI_API_COLOR_SPACE color_space);

/*!
@brief The function is used for getting HDMI color space.
@param[in] dev: A pointer to HDMI device.
@param[in] color_space:  A pointer to video color space. Please refer to enum #HDMI_API_COLOR_SPACE;
@return
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/

RET_CODE api_hdmi_get_color_space(struct hdmi_device *dev, enum HDMI_API_DEEPCOLOR *color_space);

/*!
@brief The function is used for getting HDMI deep color from EDID.
@param[in] dev: A pointer to HDMI device.
@param[in] dc_fmt:  A pointer to EDID deep color. Please refer to enum #EDID_DEEPCOLOR_CODE;
@return
@return RET_CODE
@retval RET_SUCCESS;  Operation Succeeded 
@retval RET_FAILURE;  Operation Failed 
*/
RET_CODE api_get_edid_deep_color(struct hdmi_device *dev, enum EDID_DEEPCOLOR_CODE*dc_fmt);

/*!
@}
*/

/*!
@}
*/

#ifdef __cplusplus
}
#endif
#endif // end of __HDMI_API_H__
