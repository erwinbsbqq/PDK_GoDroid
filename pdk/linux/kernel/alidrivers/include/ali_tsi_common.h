
#ifndef _ALI_TSI_COMMON_H_
#define _ALI_TSI_COMMON_H_


/*! @addtogroup DeviceDriver
 *  @{
 */
 
/*! @addtogroup ALiTSI
 *  @{
*/

#include <linux/types.h>


/*! @enum ali_tsi_input_id
    @brief TSI input source id
*/
enum ali_tsi_input_id
{    
    ALI_TSI_INPUT_SPI_0   = 0x0,	//!< Internal SPI	
    ALI_TSI_INPUT_SPI_1   = 0x1,	//!< External SPI

    ALI_TSI_INPUT_TSG     = 0x2,	//!< Internal TSG
	ALI_TSI_INPUT_SPI_3   = 0x3,	//!< DVBS2 SPI
	
    ALI_TSI_INPUT_SSI_0   = 0x4,	//!< External ASSI 2bit data
    ALI_TSI_INPUT_SSI_1   = 0x5,	//!< External ASSI2 2bit data

    ALI_TSI_INPUT_PARA    = 0x6,	//!< Reserved

    ALI_TSI_INPUT_SPI2B_0 = 0x8,	//!< ASSI 0
    ALI_TSI_INPUT_SPI2B_1 = 0x9,	//!< ASSI 1

    ALI_TSI_INPUT_SPI4B_0 = 0xA,	//!< Reserved
    ALI_TSI_INPUT_SPI4B_1 = 0xB,	//!< Reserved

    ALI_TSI_INPUT_SSI_2   = 0xC,	//!< SSI 2
    ALI_TSI_INPUT_SSI_3   = 0xD,	//!< SSI 3

    ALI_TSI_INPUT_SPI2B_2 = 0xE,	//!< ASSI 2
    ALI_TSI_INPUT_SPI2B_3 = 0xF,	//!< ASSI 3

    ALI_TSI_INPUT_SPI4B_2 = 0x10,	//!< Reserved
    ALI_TSI_INPUT_SPI4B_3 = 0x11,	//!< Reserved
	
};

/*! @enum ali_tsi_channel_id
    @brief TSI channel id
*/
enum ali_tsi_channel_id
{
    ALI_TSI_CHANNEL_0,
    ALI_TSI_CHANNEL_1,
    ALI_TSI_CHANNEL_2,
    ALI_TSI_CHANNEL_3,    
};

/*! @enum ali_tsi_output_id
    @brief TSI output id
*/
enum ali_tsi_output_id
{
    ALI_TSI_OUTPUT_DMX_0,
    ALI_TSI_OUTPUT_DMX_1,    
    ALI_TSI_OUTPUT_DMX_2,
    ALI_TSI_OUTPUT_DMX_3,    
};

/*! @enum ali_tsi_ci_id
    @brief TSI ci id
*/
enum ali_tsi_ci_id
{
    ALI_TSI_CI_0,
    ALI_TSI_CI_1,
};

/*! @enum ali_tsi_ci_link_mode
    @brief TSI ci link mode
*/
enum ali_tsi_ci_link_mode
{
    ALI_TSI_CI_LINK_CHAIN,
    ALI_TSI_CI_LINK_PARALLEL,
};

/*!@struct ali_tsi_input_set_param
   @brief TSI input parameter
 */
struct ali_tsi_input_set_param
{
    enum ali_tsi_input_id id;	//!< TSI input source id	
    __u32 attribute;			//!< The specific value for different TSI.
};

/*!@struct ali_tsi_channel_set_param
   @brief TSI channel parameter
 */
struct ali_tsi_channel_set_param
{
    enum ali_tsi_channel_id channel_id;		//!< TSI channel id	
    enum ali_tsi_input_id input_id;			//!< TSI input source id	
};

/*!@struct ali_tsi_output_set_param
   @brief TSI output parameter
 */
struct ali_tsi_output_set_param
{
    enum ali_tsi_output_id output_id;		//!< TSI output id
    enum ali_tsi_channel_id channel_id;		//!< TSI channel id	
};

/*!@struct ali_tsi_ci_bypass_set_param
   @brief TSI ci bypass parameter
 */
struct ali_tsi_ci_bypass_set_param
{
    enum ali_tsi_ci_id ci_id;				//!< TSI ci id
    __u32 is_bypass;						//!< Bypass or not.
};

#define ALI_TSI_IOC_MAGIC  0xA2		//!< TSI magic.

#define ALI_TSI_INPUT_SET _IOW(ALI_TSI_IOC_MAGIC, 41, struct ali_tsi_input_set_param)			//!< TSI input parameter setting.
#define ALI_TSI_CHANNEL_SET _IOW(ALI_TSI_IOC_MAGIC, 42, struct ali_tsi_channel_set_param)		//!< TSI channel parameter setting.
#define ALI_TSI_OUTPUT_SET _IOW(ALI_TSI_IOC_MAGIC, 43, struct ali_tsi_output_set_param)			//!< TSI output parameter setting.
#define ALI_TSI_CI_LINK_MODE_SET _IOW(ALI_TSI_IOC_MAGIC, 44, enum ali_tsi_ci_link_mode)			//!< CI link mode setting.
#define ALI_TSI_CI_BYPASS_SET _IOW(ALI_TSI_IOC_MAGIC, 45, struct ali_tsi_ci_bypass_set_param)	//!<CI bypass parameter setting.
#define ALI_TSI_CI_SPI_0_1_SWAP _IOW(ALI_TSI_IOC_MAGIC, 46, __s32)								//!< CI SPI 0 and SPI 1 swap setting.
#define ALI_TSI_INPUT_GET _IOWR(ALI_TSI_IOC_MAGIC, 81, struct ali_tsi_input_set_param)			//!< Get tsi input parameter.
#define ALI_TSI_CHANNEL_GET _IOWR(ALI_TSI_IOC_MAGIC, 82, struct ali_tsi_channel_set_param)		//!< Get tsi channel parameter.
#define ALI_TSI_OUTPUT_GET _IOWR(ALI_TSI_IOC_MAGIC, 83, struct ali_tsi_output_set_param)		//!< Get tsi output parameter.
#define ALI_TSI_CI_LINK_MODE_GET _IOR(ALI_TSI_IOC_MAGIC, 84, enum ali_tsi_ci_link_mode)			//!< Get CI link mode.
#define ALI_TSI_CI_BYPASS_GET _IOWR(ALI_TSI_IOC_MAGIC, 85, struct ali_tsi_ci_bypass_set_param)	//!< Get CI bypass parameter.

/*!
@}
*/

/*!
@}
*/


#endif

