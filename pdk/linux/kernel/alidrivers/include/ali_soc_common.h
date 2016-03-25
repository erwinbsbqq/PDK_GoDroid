#ifndef _ALI_SOC_COMMON_H_
#define _ALI_SOC_COMMON_H_


/*! @addtogroup Devicedriver
 *  @{
 */
 
/*! @addtogroup ALiSOC
 *  @{
*/


#include <alidefinition/adf_boot.h>
#include <alidefinition/adf_sysdef.h>
#include <alidefinition/adf_basic.h>
#include <alidefinition/adf_soc.h>
#include <ali_soc.h> 


#define ALI_SOC_CHIP_ID       _IO('S', 3)						//!< Get chip ID 
#define ALI_SOC_PRODUCT_ID    _IO('S', 4)						//!< Get product ID
#define ALI_SOC_GET_BONDING   _IO('S', 5)						//!< Get bonding
#define ALI_SOC_REV_ID        _IO('S', 6)						//!< Get revised ID
#define ALI_SOC_CPU_CLOCK     _IO('S', 7)						//!< Get CPU clock
#define ALI_SOC_DRAM_CLOCK    _IO('S', 8)						//!< Get DRAM clock
#define ALI_SOC_HD_ENABLED    _IO('S', 9)						//!< Support HD or not.
#define ALI_SOC_C3603_PRODUCT _IO('S', 10)						//!< Get c3603 product ID
#define ALI_SOC_USB_NUM       _IO('S', 11)						//!< Get USE number
#define ALI_SOC_USB_PORT_ENABLED   _IOR('S', 12, unsigned long)	//!< Support USB port or not.
#define ALI_SOC_NIM_M3501_SUPPORT  _IO('S' ,13)					//!< M3501 support NIM or not.
#define ALI_SOC_NIM_SUPPORT   _IO('S', 14)						//!< Support NIM or not.
#define ALI_SOC_CI_NUM        _IO('S', 15)						//!< Get CI number
#define ALI_SOC_MAC_NUM       _IO('S', 16)						//!< Get MAC number
#define ALI_SOC_TUNER_NUM     _IO('S', 17)						//!< Get tuner number
#define ALI_SOC_HD_IS_ENABLED    _IO('S', 18)					//!< Support HD or not.
#define ALI_SOC_SATA_EANBLE   _IO('S', 19)						//!< Support SATA or not.
#define ALI_SOC_SCRAM_ENABLE  _IO('S', 20)						//!< Support DRAM scramble or not.
#define ALI_SOC_SECU_ENABLE   _IO('S', 21)						//!< Support security or not.
#define ALI_SOC_SPL_ENABLE    _IO('S', 22)						//!< Support split or not.
#define ALI_SOC_UART_ENABLE   _IO('S', 23)						//!< Support UART or not.
#define ALI_SOC_ETJT_ENABLE   _IO('S', 24)						//!< Support EJTAG or not.
#define ALI_SOC_MV_ENABLE     _IO('S', 25)						//!< Support MV or not.
#define ALI_SOC_AC3_ENABLE    _IO('S', 26)						//!< Support AC3 or not.
#define ALI_SOC_DDP_ENABLE    _IO('S', 27)						//!< Support ddplus or not.
#define ALI_SOC_XD_ENABLE    _IO('S', 28)						//!< Support XD or not.
#define ALI_SOC_XDP_ENABLE   _IO('S', 29)						//!< Support XD plus or not.
#define ALI_SOC_AAC_ENABLE    _IO('S', 30)						//!< Support AAC or not.
#define ALI_SOC_H264_ENABLE   _IO('S', 31)						//!< Support H264 or not.
#define ALI_SOC_MP4_ENABLE    _IO('S', 32)						//!< Support MP4 or not.
#define ALI_SOC_REBOOT_GET_TIMER   _IOR('S', 33, struct reboot_timer)	//!< No longer use
#define ALI_SOC_ENTER_STANDBY      _IOW('S', 34 , struct boot_timer)	//!< No longer use
#define ALI_SOC_SET_DEBUG_LEVEL _IOW('S', 35, struct debug_level_paras)	//!< Set debug level
#define ALI_SOC_GET_MEMORY_MAP _IOW('S', 36, struct soc_memory_map)		//!< Get memory mapping
#define ALI_SOC_READ8  _IOR('S', 37, struct soc_opt_paras8)				//!< Read 8 bits  data from register address
#define ALI_SOC_WRITE8 _IOW('S', 38, struct soc_opt_paras8)				//!< Write 8 bits data to register address
#define ALI_SOC_READ16  _IOR('S', 39, struct soc_opt_paras16)			//!< Read 16 bits data from register address
#define ALI_SOC_WRITE16 _IOW('S', 40, struct soc_opt_paras16)			//!< Write 16 bits data to register address
#define ALI_SOC_READ32  _IOR('S', 41, struct soc_opt_paras32)			//!< Read 32 bits data from register address
#define ALI_SOC_WRITE32 _IOW('S', 42, struct soc_opt_paras32)			//!< Write 32 bits data to register address
#define ALI_SOC_GET_SEE_VER _IOW('S', 43, struct soc_opt_see_ver)		//!< Get see version
#define ALI_SOC_DISABLE_SEE_PRINTF _IOR('S', 44, unsigned long)			//!< Disable see log
#define ALI_SOC_HIT_SEE_HEART	_IO('S', 45)							//!< Hit see heart
#define ALI_SOC_ENABLE_SEE_EXCEPTION	_IO('S', 46)					//!< Enable see exception
#define ALI_SOC_PER_READ32  _IOR('S', 47, struct soc_opt_paras32)		//!< Read 32 bits data from peripheral address
#define ALI_SOC_PER_WRITE32 _IOW('S', 48, struct soc_opt_paras32)		//!< Write 32 bits data to peripheral address
#define ALI_SOC_READ  		_IOR('S', 49,struct soc_op_paras)			//!< Read data 
#define ALI_SOC_WRITE 		_IOW('S', 50,struct soc_op_paras)			//!< Write data
#define ALI_SOC_REBOOT   	_IOR('S', 51, struct reboot_timer)			//!< No longer use
#define ALI_DSC_ACC_CE_DIS   _IO('S', 52)								//!< No longer use
#define ALI_SOC_MS10_ENABLE  _IO('S', 53)								//!< Support MS10 or not.
#define ALI_SOC_MS11_ENABLE  _IO('S', 54)								//!< Support MS11 or not.
#define ALI_SOC_RMVB_ENABLE  _IO('S', 55)								//!< Support RMVB or not.
#define ALI_SOC_VC1_ENABLE   _IO('S', 56)								//!< Support VC1 or not.
#define ALI_SOC_AVS_ENABLE   _IO('S', 57)								//!< Support AVS or not.
#define ALI_SOC_VP8_ENABLE   _IO('S', 58)								//!< Support VP8 or not.
#define ALI_SOC_FLV_ENABLE   _IO('S', 59)								//!< Support FLV or not.
#define ALI_SOC_MG_ENABLE    _IO('S', 60)								//!< Support MG or not.
#define ALI_SOC_GET_BOOT_TYPE   _IO('S', 61)							//!< Get boot type
#define ALI_SOC_ENABLE_POWER_DOWN  _IO('S',62) 							//!< Enable/disable power
#define ALI_SOC_SHOW_SEE_PLUGIN_INFO _IO('S',63) 						//!< Show see plugin info
#define ALI_SOC_DRAM_SIZE   _IO('S', 64)								//!< Get dram size

/*!
@}
*/

/*!
@}
*/


#endif 
