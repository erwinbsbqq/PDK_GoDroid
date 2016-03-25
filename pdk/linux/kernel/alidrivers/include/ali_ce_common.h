#ifndef  _ALI_CE_COMMON_H_
#define  _ALI_CE_COMMON_H_

#include <alidefinition/adf_ce.h>


/*!@addtogroup ALiKeyLadder
 *  @{
    @~
 */

#define ALI_CE_LINUX_DEV_PATH "/dev/ali_ce_0" //!< KeyLadder device node in kernel.

#define  IO_CE_KEY_GENERATE            						(IO_CE_BASE + 11)     
//!< Generate the single level key, such as generate one KEY_3_X by using KEY_2_X as the key, refer to 'struct CE_DATA_INFO'.
#define  IO_CE_KEY_LOAD                                     			(IO_CE_BASE + 12)
//!< Load the specified OTP root key to KEY_0_X, along with 'struct OTP_PARAM'.
#define  IO_CE_GENERATE_CW_KEY                              		(IO_CE_BASE + 15)
//!< Generate the CW, similar function to IO_CE_KEY_GENERATE, along with 'struct ALI_CE_GEN_CW'.
#define  IO_CE_GENERATE_SINGLE_LEVEL_KEY                    	(IO_CE_BASE + 16)
//!< Same functionality with #IO_CE_KEY_GENERATE.
#define  IO_CE_GET_DEV_HLD                                  		(IO_CE_BASE + 17)
//!< Get the SEE KeyLadder device.

/*! @struct ALI_CE_GEN_CW
 *   @brief For #IO_CE_GENERATE_CW_KEY parameters. @~
 */
typedef struct ALI_CE_GEN_CW
{
	unsigned char *in; //!< Buffer address of the 16 bytes cipher CW
	unsigned char aes_or_des;//!< Specify the KL algorithm, AES or TDES.
	unsigned char lowlev_pos;//!< Specify the source key index.
	unsigned char highlev_pos;//!< Specify the target key index.
} ALI_CE_GEN_CW;

/*!
@}
*/

#endif 

