#ifndef  _ADF_CE_H_
#define  _ADF_CE_H_

/*!@addtogroup ALiKeyLadder
 *  @{
    @~
 */
 
#define IO_CE_BASE                                          (0xC0<<8) 
//!< KeyLadder ioctl cmd base.
#define IO_OTP_ROOT_KEY_GET                        (IO_CE_BASE + 0) 
//!< Load the specified OTP root key to KEY_0_X, along with 'struct OTP_PARAM'.
#define IO_CRYPT_DEBUG_GET_KEY                   (IO_CE_BASE + 5) 
//!< Reserved for caller to get the KL debug key.
#define IO_CRYPT_POS_IS_OCCUPY                   (IO_CE_BASE + 6) 
//!< Check if the key idex is occupied or not, along with 'struct CE_POS_STS_PARAM'.
#define IO_CRYPT_POS_SET_USED                     (IO_CE_BASE + 7) 
//!< Set the key index status to busy directly, usually used on the application initialization stage.
#define IO_CRYPT_POS_SET_IDLE                      (IO_CE_BASE + 8) 
//!< Set the key index status to idle.
#define IO_CRYPT_FOUND_FREE_POS                 (IO_CE_BASE + 9)
//!< Get an idle key index and set this index status to busy, along with 'struct CE_FOUND_FREE_POS_PARAM'.
#define IO_DECRYPT_PVR_USER_KEY                 (IO_CE_BASE + 10)
//!< Used to decrypt the PVR cipher key to the specified PVR key index, refer to 'struct CE_PVR_KEY_PARAM', only used in DMX driver.

#define CE_IO_CMD(cmd)  (cmd & 0xFF) //!< Mask the cmd to 8 bits, only 8 bits valid for KL ioctl cmd.

#define AES_CE_KEY_LEN  16 //!< Indicate the AES cipher block length in KL is 16 bytes.
#define TDES_CE_KEY_LEN  8 //!< Indicate the DES cipher block length in KL is 8 bytes.

#define INVALID_ALI_CE_KEY_POS 0xff  //!< Define the invalid KL key index
#define ALI_INVALID_CRYPTO_KEY_POS   INVALID_ALI_CE_KEY_POS //!< Define the invalid KL key index


/*! @enum CE_OTP_KEY_ADDR
 *   @brief OTP physical address of the KL root keys.
 */
enum CE_OTP_KEY_ADDR
{
	OTP_ADDESS_1 = 0x4d, //!< OTP physical address of secure key 0.
	OTP_ADDESS_2 = 0x51, //!< OTP physical address of secure key 1.
	OTP_ADDESS_3 = 0x55, //!< OTP physical address of secure key 2.
	OTP_ADDESS_4 = 0x59, //!< OTP physical address of secure key 3.
	OTP_ADDESS_5 = 0x60, //!< OTP physical address of secure key 4.	
	OTP_ADDESS_6 = 0x64, //!< OTP physical address of secure key 5.	
};

/*! @enum CRYPTO_STATUS
 *   @brief Return values of the KL functions.
 */
enum CRYPTO_STATUS
{
	ALI_CRYPTO_SUCCESS = 0, //!< The intended operation was executed successfully.	
	ALI_CRYPTO_ERROR, //!< The function terminated abnormally. The intended operation failed.
	ALI_CRYPTO_WARNING_DRIVER_ALREADY_INITIALIZED, //!< The SEE KL is already initialized.
	ALI_CRYPTO_ERROR_INVALID_PARAMETERS, //!< The passed parameters are invalid.
	ALI_CRYPTO_ERROR_OPERATION_NOT_ALLOWED,//!< The requested operation is not allowed.
	ALI_CRYPTO_ERROR_OPERATION_NOT_SUPPORTED,//!< The requested operation is not supported.
	ALI_CRYPTO_ERROR_INITIALIZATION_FAILED,//!< The SEE KL initialization failed.
	ALI_CRYPTO_ERROR_DRIVER_NOT_INITIALIZED,//!< The SEE KL has not been initialized.
	ALI_CRYPTO_ERROR_INVALID_ADDR,//!< The passed address is invalid.
	ALI_CRYPTO_ERROR_INVALID_DEV,//!< The passed device pointer is invalid.
};

/*! @enum CE_OTP_KEY_SEL
 *   @brief Define the KL root key index.
 */
enum CE_OTP_KEY_SEL
{
	OTP_KEY_0_0=0,//!<Load root key from OTP 0x4d. #OTP_ADDESS_1
	OTP_KEY_0_1=1,//!<Load root key from OTP 0x51. #OTP_ADDESS_2
	OTP_KEY_0_2=2,//!<Load root key from OTP 0x55 or OTP 0x60. #OTP_ADDESS_3 or #OTP_ADDESS_5
	OTP_KEY_0_3=3,//!<Load root key from OTP 0x59 or OTP 0x64. #OTP_ADDESS_4 or #OTP_ADDESS_6
};

/*! @enum HDCP_DECRYPT_MODE
 *   @brief Define KL SRAM operation mode, which is usually #NOT_FOR_HDCP.
 */
enum  HDCP_DECRYPT_MODE
{
	NOT_FOR_HDCP=0, //!< For KL key operation.
	TARGET_IS_HDCP_KEY_SRAM=(1<<14) //!< For HDCP key operation
};

/*! @enum CE_CRYPT_TARGET
 *   @brief Define the 1st, 2nd, and 3rd level keys. CRYPTO_KEY_X_X is the equivalent to KEY_X_X. 
 *	The first X indicates the key's level, the second X indicates the key's index in current level. \n
 *    CRYPT_KEY_1_X is genereted by OTP_KEY_0_X; CRYPT_KEY_2_X is genereted by CRYPT_KEY_1_X;\n
 *    CRYPT_KEY_3_X is genereted by CRYPT_KEY_2_X;
 */
enum CE_CRYPT_TARGET
{
	CRYPT_KEY_1_0=0x4, //!< 0x4
	CRYPT_KEY_1_1=0x5, //!< 0x5
	CRYPT_KEY_1_2=0x6, //!< 0x6
	CRYPT_KEY_1_3=0x7, //!< 0x7

	CRYPT_KEY_2_0=0x8, //!< 0x8
	CRYPT_KEY_2_1=0x9, //!< 0x9
	CRYPT_KEY_2_2=0xa, //!< 0xA
	CRYPT_KEY_2_3=0xb, //!< 0xB
	CRYPT_KEY_2_4=0xc, //!< 0xC
	CRYPT_KEY_2_5=0xd, //!< 0xD
	CRYPT_KEY_2_6=0xe, //!< 0xE
	CRYPT_KEY_2_7=0xf, //!< 0xF

	CRYPT_KEY_3_0=0x10, //!< 0x10
	CRYPT_KEY_3_1=0x11, //!< 0x11
	CRYPT_KEY_3_2=0x12, //!< 0x12
	CRYPT_KEY_3_3=0x13, //!< 0x13
	CRYPT_KEY_3_4=0x14, //!< 0x14
	CRYPT_KEY_3_5=0x15, //!< 0x15
	CRYPT_KEY_3_6=0x16, //!< 0x16
	CRYPT_KEY_3_7=0x17, //!< 0x17
	CRYPT_KEY_3_8=0x18, //!< 0x18
	CRYPT_KEY_3_9=0x19, //!< 0x19
	CRYPT_KEY_3_10=0x1a, //!< 0x1A
	CRYPT_KEY_3_11=0x1b, //!< 0x1B
	CRYPT_KEY_3_12=0x1c, //!< 0x1C
	CRYPT_KEY_3_13=0x1d, //!< 0x1D
	CRYPT_KEY_3_14=0x1e, //!< 0x1E
	CRYPT_KEY_3_15=0x1f, //!< 0x1F

	CRYPT_KEY_3_16=0x20, //!< 0x20
	CRYPT_KEY_3_17=0x21, //!< 0x21
	CRYPT_KEY_3_18=0x22, //!< 0x22
	CRYPT_KEY_3_19=0x23, //!< 0x23
	CRYPT_KEY_3_20=0x24, //!< 0x24
	CRYPT_KEY_3_21=0x25, //!< 0x25
	CRYPT_KEY_3_22=0x26, //!< 0x26
	CRYPT_KEY_3_23=0x27, //!< 0x27
	CRYPT_KEY_3_24=0x28, //!< 0x28
	CRYPT_KEY_3_25=0x29, //!< 0x29
	CRYPT_KEY_3_26=0x2a, //!< 0x2A
	CRYPT_KEY_3_27=0x2b, //!< 0x2B
	CRYPT_KEY_3_28=0x2c, //!< 0x2C
	CRYPT_KEY_3_29=0x2d, //!< 0x2D
	CRYPT_KEY_3_30=0x2e, //!< 0x2E
	CRYPT_KEY_3_31=0x2f, //!< 0x2F

	CRYPT_KEY_3_32=0x30, //!< 0x30
	CRYPT_KEY_3_33=0x31, //!< 0x31
	CRYPT_KEY_3_34=0x32, //!< 0x32
	CRYPT_KEY_3_35=0x33, //!< 0x33
	CRYPT_KEY_3_36=0x34, //!< 0x34
	CRYPT_KEY_3_37=0x35, //!< 0x35
	CRYPT_KEY_3_38=0x36, //!< 0x36
	CRYPT_KEY_3_39=0x37, //!< 0x37

	CRYPT_KEY_3_40=0x38, //!< 0x38
	CRYPT_KEY_3_41=0x39, //!< 0x39
	CRYPT_KEY_3_42=0x3a, //!< 0x3A
	CRYPT_KEY_3_43=0x3b, //!< 0x3B
	CRYPT_KEY_3_44=0x3c, //!< 0x3C
	CRYPT_KEY_3_45=0x3d, //!< 0x3D
	CRYPT_KEY_3_46=0x3e, //!< 0x3E
	CRYPT_KEY_3_47=0x3f, //!< 0x3F
};

/*! @enum CE_KEY
 *   @brief Define the 1st, 2nd, and 3rd level keys, KEY_X_X is the equivalent to CRYPTO_KEY_X_X. 
 *   The first X indicates the key's level, the second X indicates the key's index in current level.
 */
enum CE_KEY
{
	KEY_0_0=0, //!< 0x0
	KEY_0_1=1, //!< 0x1
	KEY_0_2=2, //!< 0x2
	KEY_0_3=3, //!< 0x3
	KEY_1_0=4, //!< 0x4
	KEY_1_1=5, //!< 0x5
	KEY_1_2=6, //!< 0x6
	KEY_1_3=7, //!< 0x7
	KEY_2_0=8, //!< 0x8
	KEY_2_1=9, //!< 0x9
	KEY_2_2=0xa, //!< 0xA
	KEY_2_3=0xb, //!< 0xB
	KEY_2_4=0xc, //!< 0xC
	KEY_2_5=0xd, //!< 0xD
	KEY_2_6=0xe, //!< 0xE
	KEY_2_7=0xf, //!< 0xF
	KEY_3_0=0x10, //!< 0x10
	KEY_3_1=0x11, //!< 0x11
	KEY_3_2=0x12, //!< 0x12
	KEY_3_3=0x13, //!< 0x13
	KEY_3_4=0x14, //!< 0x14
	KEY_3_5=0x15, //!< 0x15
	KEY_3_6=0x16, //!< 0x16
	KEY_3_7=0x17, //!< 0x17
	KEY_3_8=0x18, //!< 0x18
	KEY_3_9=0x19, //!< 0x19
	KEY_3_10=0x1a, //!< 0x1A
	KEY_3_11=0x1b, //!< 0x1B
	KEY_3_12=0x1c, //!< 0x1C
	KEY_3_13=0x1d, //!< 0x1D
	KEY_3_14=0x1e, //!< 0x1E
	KEY_3_15=0x1f, //!< 0x1F
};

/*! @enum CE_CRYPT_SELECT
 *   @brief Define KL encryption and decryption mode.
 */
enum CE_CRYPT_SELECT
{  
	CE_IS_DECRYPT = 1,//!<Decryption
	CE_IS_ENCRYPT=0//!<Encryption
};

/*! @enum CE_MODULE_SELECT
 *   @brief Define KL algorithm selection.
 */
enum CE_MODULE_SELECT
{  
	CE_SELECT_DES= 0, //!<KL algorithm is TDES.
	CE_SELECT_AES = 1, //!<KL algorithm is AES.
};

/*! @enum HDCP_KEY_SELECT
 *   @brief Specify key type which is used for reading the KL debug key.
 */
enum HDCP_KEY_SELECT
{
	CE_KEY_READ=0, //!<Read KL key
	HDCP_KEY_READ=1 //!<Read HDCP key
};

/*! @enum KEY_LEVEL
 *   @brief Define the key level that API will get from KL.
 */
enum KEY_LEVEL
{
	SKIP_LEVEL = 0 ,//!<Internal reserved. 
	ONE_LEVEL,//!<Get an idle key index from 1st, 2nd and 3rd level keys.
	TWO_LEVEL,//!<Get an idle key index from 2nd and 3rd level keys.
	THREE_LEVEL//!<Get an idle key index from 3rd level keys.
};

/*! @struct CE_PVR_KEY_PARAM
  *   @brief Struct used to change the PVR key. Only used in DMX driver when doing playback.
 */
typedef struct CE_PVR_KEY_PARAM
{
	unsigned char *input_addr;//!<Buffer address of the 16 bytes cipher CW
	unsigned int second_pos;  //!<Specify the target key index.
	unsigned int first_pos;   //!<Specify the source key index.
}CE_PVR_KEY_PARAM, *pCE_PVR_KEY_PARAM;

/*! @struct OTP_PARAM
 *   @brief Struct used to load physical OTP root key to KL KEY_0_X.
 */
typedef struct OTP_PARAM
{
	unsigned char otp_addr;//!<OTP root key physical address.
	enum CE_OTP_KEY_SEL otp_key_pos;   //!<KL root key index, KEY_0_X.
}OTP_PARAM, *pOTP_PARAM;

/*! @struct DATA_PARAM
 *  @brief Struct used to specify the data block information.
 */
typedef struct DATA_PARAM
{
	unsigned int crypt_data[4] ; //!<Input data buffer.
	unsigned int data_len ;  //!<Input data length.
}DATA_PARAM, *pDATA_PARAM;

/*! @struct DES_PARAM
 *  @brief Struct used to specify the cryption mode, algorithm and result location(TDES only).
 */
typedef struct DES_PARAM
{
	enum CE_CRYPT_SELECT crypt_mode;//!<Encryption or decryption selection.
	enum CE_MODULE_SELECT aes_or_des;//!<AES or TDES algorithm selection.
	unsigned char des_low_or_high;//!<Select TDES result location in higher 8bytes or lower 8bytes.
}DES_PARAM, *pDES_PARAM;

/*! @struct CE_KEY_PARAM
 *  @brief Define source key index, target key index and KL SRAM operation mode.
 */
typedef struct CE_KEY_PARAM
{
	enum CE_KEY first_key_pos;//!<Source key index.
	enum CE_CRYPT_TARGET second_key_pos;//!<Target key index.
	enum HDCP_DECRYPT_MODE hdcp_mode ;//!<KL SRAM operation mode.
}CE_KEY_PARAM, *pCE_KEY_PARAM;

/*! @struct CE_DEBUG_KEY_INFO
 *   @brief Reserved.
 */
typedef struct CE_DEBUG_KEY_INFO
{
	enum HDCP_KEY_SELECT sel; //!<Read HDCP or not HDCP.
	unsigned int buffer[4]; //!<Buffer to store the debug key.
	unsigned int len;  //!<Length in byte, equal to algorithm block length, AES-16, TDES-8.
}CE_DEBUG_KEY_INFO, *pCE_DEBUG_KEY_INFO;

/*! @struct CE_DATA_INFO
 *   @brief Struct used for KL to generate the single level key.
 */
typedef struct CE_DATA_INFO
{
	OTP_PARAM otp_info;//!<Load physical OTP root key to KL KEY_0_X.
	DATA_PARAM data_info;//!<Specify the data block information.
	DES_PARAM des_aes_info;//!<Specify the cryption mode, algorithm and result location(TDES only).
	CE_KEY_PARAM key_info; //!<Specify the source key index, target key index and KL SRAM operation mode.
}CE_DATA_INFO, *pCE_DATA_INFO;

/*! @struct CE_POS_STS_PARAM
 *   @brief Struct used to get the status of the specified key index.
 */
typedef struct CE_POS_STS_PARAM
{
	unsigned int pos; //!<Specify the key index.
	unsigned int status; //!<Returned status(busy or idle).
}CE_POS_STS_PARAM, *pCE_POS_STS_PARAM;

/*! @struct CE_FOUND_FREE_POS_PARAM
 *   @brief Struct used to get the idle key index from KL key table.
 */
typedef struct CE_FOUND_FREE_POS_PARAM
{
	unsigned int pos; //!<Idle index returned from key table.
	enum KEY_LEVEL ce_key_level; //!<Specify the initial key level of KL.
	unsigned char number; //!<Specify the key number that caller wants to get, which is usually 1 or 2, with default of 1.
	enum CE_OTP_KEY_SEL root;//!<Specify root key index, and driver will return relevant idle key index.
	/*!<This parameter is valid on M3515B, M3823 and M3733 only.
	*/
}CE_FOUND_FREE_POS_PARAM, *pCE_FOUND_FREE_POS_PARAM;

/*! @struct CE_DEVICE
 *   @brief KL device struct reserved on HLD layer.
 */
typedef struct CE_DEVICE
{
	struct CE_DEVICE *next;
	int type;
	char name[16];

	void *pCePriv;
	unsigned int base_addr;
	unsigned int interrupt_id;

	unsigned short semaphore_id;
	unsigned short semaphore_id2;

	void (*attach)(void);
	void (*detach)( struct CE_DEVICE *);
	int (*open)( struct CE_DEVICE *);
	int (*close)( struct CE_DEVICE *);
	int (*ioctl)(struct CE_DEVICE *,unsigned int ,unsigned int );
	int (*key_generate)(struct CE_DEVICE *,pCE_DATA_INFO );
	int (*key_load)(struct CE_DEVICE *,pOTP_PARAM);	
	int fd;
}CE_DEVICE, *pCE_DEVICE;


/*!
@}
*/

#endif 

