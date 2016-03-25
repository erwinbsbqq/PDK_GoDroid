#ifndef _ADF_DSC_H_
#define _ADF_DSC_H_


/** @page p_history Changes history
 *
 *  - <b> 1.6.2 - 28-Jul-2014 </b>
 *    - More description for KEY_PARAM
 *  
 *  - <b> 1.6.0 - 31-Dec-2013 </b>
 *    - Initial Release
*/

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/** @mainpage Overview
 *  - @subpage p_history
 *  - @subpage p_preface
 *
 *  <hr>Copyright &copy; 2013 ALi Corporation. All rights reserved.\n
 *  ZhuHai, China\n
 *  Tel: +86 756 3392000 \n
 *  http://www.alitech.com
 *
 *  All trademarks and registered trademarks are the property of their respective
 *  owners.
 *
 *  This document is supplied with an understanding that the notice(s) herein or
 *  any other contractual agreement(s) made that instigated the delivery of a
 *  hard copy, electronic copy, facsimile or file transfer of this document are
 *  strictly observed and maintained.
 *
 *  The information contained in this document is subject to change without notice.
 *
 *  <b>Security Policy of ALi Corporation</b>\n
 *  Any recipient of this document, without exception, is subject to a
 *  Non-Disclosure Agreement (NDA) and access authorization.
*/

/* -------------------------------------------------------------------------- */

/** @page p_preface Preface
 *  <h2>Objectives</h2>
 *  This document specifies the Secure Chipset API that gives access to ALi 
 *  security features of the advanced security chipset.
 *  
 *  It includes standard cryptographic functions such as symmetric block ciphers 
 *  (TDES, AES, CSA), digest functions (SHA). Cryptographic operations are based on 
 *  cleartext keys, secret keys protected by hardware key ladders.
 *  
 *
 *  <hr><h2>Audience</h2>
 *  This document is intended for developers in charge of implementing/maintaining the Conditional 
 *  Access System (CAS) and as well as other security applications.
 *
 * <hr><h2>References</h2>
 *  - [DSC] ALi DSC API Programming Guide, Version 1.6.2\n
 *  - [KL] ALi Key Ladder API Programming Guide, Version 1.6.2\n
 *  - [TRNG] ALi TRNG API Programming Guide, Version 1.6.2\n
*/

 
/*! @addtogroup ALiDSC
 *  @{
    @~
 */


#define IO_DSC_BASE                                 (0xC0<<8)
//!< DSC ioctl cmd base.

/* Used for AES/TDES/CSA/SHA ioctl 
*/
#define IO_INIT_CMD                          		(IO_DSC_BASE + 0)
//!< Initialize the AES/TDES/SHA/CSA device. refer to 'aes_init_param des_init_param csa_init_param sha_init_param'.
#define IO_CREAT_CRYPT_STREAM_CMD       (IO_DSC_BASE + 1)
//!< Ioctl for AES/TDES/CSA to create the key HANDLE in DSC, refer to 'struct KEY_PARAM'.
#define IO_DELETE_CRYPT_STREAM_CMD     (IO_DSC_BASE + 2)
//!< Ioctl for AES/TDES/CSA to delete the key HANDLE in DSC.
#define IO_KEY_INFO_UPDATE_CMD             (IO_DSC_BASE + 3)
//!< Update the clear key value for AES/TDES/CSA, refer to 'struct KEY_PARAM'.
/*!< Only the handle/pid/key_length/p_xxx_key_info are needed when updating clear key.
*/

/* Used for DSC ioctl 
*/
#define IO_PARSE_DMX_ID_SET_CMD            (IO_DSC_BASE + 0) 
//!< Set the stream ID for current live play or playback stream.
/*!< The stream ID should be same with the ID used for #IO_INIT_CMD and #IO_CREAT_CRYPT_STREAM_CMD.
	
	This IO cmd is usually used along with the DMX IO cmd as below:\n
	ALI_DMX_SEE_CRYPTO_TYPE_SET(Connect the decryption SEE device pointer and algorithm with DMX driver).\n
	ALI_DMX_SEE_CRYPTO_START (enable the decryption).
*/

#define IO_PARSE_DMX_ID_GET_CMD            (IO_DSC_BASE + 1)    
//!< Get the stream ID of current live play or playback stream, and use this ID to decrypt the stream.
/*!<This ID is set by '#IO_PARSE_DMX_ID_SET_CMD', it contains the key information for decrytion, only used in DMX driver.
*/
#define IO_DSC_GET_DES_HANDLE                (IO_DSC_BASE + 2)
//!< Get the DES device 0 pointer.
#define IO_DSC_GET_AES_HANDLE                (IO_DSC_BASE + 3)  
//!< Get the AES device 0 pointer.
#define IO_DSC_GET_CSA_HANDLE                (IO_DSC_BASE + 4)  
//!< Get the CSA device 0 pointer.
#define IO_DSC_GET_SHA_HANDLE                (IO_DSC_BASE + 5)  
//!< Get the SHA device 0 pointer.
#define IO_DSC_SET_PVR_KEY_PARAM          (IO_DSC_BASE + 6)  
//!< Set the PVR key information to DSC.
/*!< The recording file size is limited by the user configuration, if the file size reach to the max value while recording, 
	PVR will create a new file for the continuous recording, at this time the DSC will update the recording key for 
	this new file (DSC counts the number/size of the re-encrypted packets, if this size reach to the configured file size,
	DSC will update the key). Refer to 'struct DSC_PVR_KEY_PARAM'.
*/
#define IO_DSC_ENCRYTP_BL_UK                  (IO_DSC_BASE + 7) 
//!< Reserved for BL to encrypt the BL universal key.
#define IO_DSC_SET_PVR_KEY_IDLE             (IO_DSC_BASE + 8) 
//!< Delete one set of PVR key based on the record stream ID.
#define IO_DSC_VER_CHECK                        (IO_DSC_BASE + 9)
//!< Reserved for BL to check the system firmware version.
#define IO_DSC_SET_ENCRYPT_PRIORITY      (IO_DSC_BASE + 10)
//!< Change the TS encryption parity (even or odd) when doing PVR, along with 'struct DSC_EN_PRIORITY'.
#define IO_DSC_GET_DRIVER_VERSION         (IO_DSC_BASE + 11)
//!< Get current driver version, max to 20 bytes characters.
#define IO_DSC_SET_CLR_CMDQ_EN             (IO_DSC_BASE + 12)
//!< Enable or disable the DSC command queue mode, CMDQ ensures the high performance when multi-channel.
#define IO_DSC_DELETE_HANDLE_CMD         (IO_DSC_BASE + 13)
//!< Ioctl for DSC to delete the key HANDLE created by AES/TDES/CSA directly, same functionality with #IO_DELETE_CRYPT_STREAM_CMD.
#define IO_DSC_FIXED_DECRYPTION            (IO_DSC_BASE + 30)
//!< Reserved for BL.
#define IO_DSC_SYS_UK_FW                       (IO_DSC_BASE + 31)
//!< Reserved for BL to handle the system firmware or universal key.

#define VIRTUAL_DEV_NUM 4
//!< Define the AES/TDES/SHA/CSA sub-device number.
/*!< The AES/TDES/SHA/CSA are sub-algotithms of DSC(Descramble Scramble Core) 
	and DSC takes the responsibility to manage the device/stream ID and key HANDLE for its sub-algorithms.
	Every algorithm in DSC has 4 (#VIRTUAL_DEV_NUM) sub-devices. ('4' is for multi-channel).
*/

#define SHA1_HASH_SIZE 		20 //!< SHA1 hash size is 20 bytes.
#define SHA224_HASH_SIZE 	28 //!< SHA224 hash size is 28 bytes.
#define SHA256_HASH_SIZE 	32 //!< SHA256 hash size is 32 bytes.
#define SHA384_HASH_SIZE 	48 //!< SHA384 hash size is 48 bytes
#define SHA512_HASH_SIZE 	64 //!< SHA512 hash size is 64 bytes

#define DSC_IO_CMD(cmd)  (cmd & 0xFF) //!< Mask the cmd to 8 bits, only 8 bits valid for DSC ioctl cmd.

#define PURE_DATA_MAX_SIZE  0x100000 
//!< Define max data size(PURE + TS mode) for encryption, decryption and hash when the APIs performed from user space.	
#define TS_MAX_SIZE_TDS  0x10000
//!< Define max TS data packets number for encryption and decryption when the APIs performed from kernel or SEE.
#define PURE_DATA_MAX_SIZE_TDS  0x4000000
//!< Define max PURE data size for encryption, decryption and hash when the APIs performed from kernel or SEE.

#define ALI_INVALID_CRYPTO_STREAM_HANDLE  0xffffffff //!< Define the invalid key HANDLE value.
#define ALI_INVALID_CRYPTO_STREAM_ID		0xff //!< Define the invalid stream iD value.
#define ALI_INVALID_DSC_SUB_DEV_ID          0xff //!< Define the invalid sub-device iD value.
#define INVALID_DSC_SUB_DEV_ID			0xff  //!< Define the invalid sub-device iD value.
#define INVALID_DSC_STREAM_ID			0xff //!< Define the invalid stream iD value.

#define AES_BLOCK_LEN 16 //!< Define the AES algorithm data block length.
#define DES_BLOCK_LEN 8 //!< Define the DES algorithm data block length.

#define SHA_MODE_CHECK(mode)	(((mode) != SHA_SHA_1) && \
		((mode) != SHA_SHA_224) && ((mode) != SHA_SHA_256) && \
		((mode) != SHA_SHA_384 )&& ((mode) != SHA_SHA_512))
//!< Define the macro for SHA input mode checking.		


/*! @enum CSA_VERSION
 *   @brief Define DVB-CSA version.
*/
enum CSA_VERSION
{
	CSA1=1,//!< DVB-CSA1.1
	CSA_1=1,//!< DVB-CSA1.1
	
	CSA2=0,//!<DVB-CSA2.0
	CSA_2=0,//!<DVB-CSA2.0
	
	CSA3=2,//!< DVB-CSA3
	CSA_3=2,//!< DVB-CSA3
};

/*! @enum SHA_MODE
 *   @brief Define SHA hash mode.
*/
enum SHA_MODE
{
	SHA_SHA_1 = 0,//!< SHA1, HASH length is 20 bytes.
	SHA_SHA_224 = (1<<29),//!< SHA224, HASH length is 28 bytes. 
	SHA_SHA_256 = (2<<29),//!< SHA256, HASH length is 32 bytes.
	SHA_SHA_384 = (3<<29),//!< SHA384, HASH length is 48 bytes.
	SHA_SHA_512 = (4<<29),//!< SHA512, HASH length is 64 bytes.
};

/*! @enum SHA_DATA_SOURCE
 *   @brief Define the SHA data source.
*/
enum SHA_DATA_SOURCE
{
	SHA_DATA_SOURCE_FROM_DRAM =0,//!<DRAM
	SHA_DATA_SOURCE_FROM_FLASH =1,//!<NOR FLASH
};

/*! @enum PARITY_MODE
 *  @brief Define encryption and decryption parity mode.
*/
enum PARITY_MODE
{
	EVEN_PARITY_MODE  =0,//!< Even parity, usually used for TS encryption.
	ODD_PARITY_MODE =1, //!< Parity is odd.
	AUTO_PARITY_MODE0= 2, //!< Used in TS decryption, auto detects the TS packet parity.
	AUTO_PARITY_MODE1=3,//!< Used in TS decryption, auto detects the TS packet parity.
	OTP_KEY_FROM_68 = 4,//!< This value is not for parity, it indicates that the key is directly from OTP key6 when #KEY_FROM_OTP.
	OTP_KEY_FROM_6C = 5,//!< This value is not for parity, it indicates that the key is directly from OTP key7 when #KEY_FROM_OTP.
};

/*! @enum KEY_TYPE
 *  @brief Define key source.
*/
enum KEY_TYPE
{
	KEY_FROM_REG=0,//!< Not supprt.
	KEY_FROM_SRAM=1,//!< Key is from DSC internal SRAM, it's a clear key.
	KEY_FROM_CRYPTO=2,//!< Key is from KL, it's an advanced secure key.
	KEY_FROM_OTP = 3,//!<  Key is directly from OTP secure key6/7, it's an advanced secure key.
};

/*! @enum KEY_MAP_MODE
 *  @brief Define the key mode according to the key length.
*/
enum KEY_MAP_MODE
{
	CSA_MODE=0,//!< Identify the key length is 64 bits. 
	DES_MODE=0,//!< Identify the key length is 64 bits. 
	CSA3_MODE=1,//!< Identify the key length is 128 bits. 
	AES_128BITS_MODE=1,//!< Identify the key length is 128 bits. 
	CRYPTO_128BITS_MODE=1,//!< Identify the key length is 128 bits. 
	TDES_ABA_MODE=1,//!< Identify the key length is 128 bits. 
	AES_192BITS_MODE=2,//!< Identify the key length is 192 bits. 
	TDES_ABC_MODE=2,//!<  Identify the key length is 192 bits. 
	AES_256BITS_MODE=3//!< Identify the key length is 256 bits. 
};

/*! @enum KEY_BIT_NUM
 *  @brief Define the bit length of DSC key.
*/
enum KEY_BIT_NUM
{
	BIT_NUMOF_KEY_64 = 64, //!< Identify the bit length is 64 bits. 
	BIT_NUMOF_KEY_128 = 128,//!< Identify the bit length is 128 bits. 
	BIT_NUMOF_KEY_192 = 192,//!< Identify the bit length is 192 bits. 
	BIT_NUMOF_KEY_256 = 256,//!< Identify the bit length is 256 bits. 
};

/*! @enum DMA_MODE
 *  @brief Define the operation data type.
 */
enum DMA_MODE
{
	PURE_DATA_MODE=0,//!< The operation data is raw/pure data. 
	TS_MODE=(1<<24),//!< The operation data is TS data. 
};

/*! @enum RESIDUE_BLOCK
 *   @brief Define residue block handling mode.
 */
enum RESIDUE_BLOCK
{
	RESIDUE_BLOCK_IS_NO_HANDLE = 0,//!< Does not process residue block. residue block data is same with input. 
	RESIDUE_BLOCK_IS_AS_ATSC = (1 << 12),//!< Process residue block as ANSI SCT 52 standard. 
	RESIDUE_BLOCK_IS_HW_CTS = (2 << 12),//!< The residue block handling uses cipher stealing method.
	RESIDUE_BLOCK_IS_RESERVED = (3 << 12),//!< Reserved.
};

/*! @enum WORK_MODE
 *   @brief Define block chaing mode.
 */
enum WORK_MODE
{
	WORK_MODE_IS_CBC = 0, //!< Cipher block chaining.
	WORK_MODE_IS_ECB = (1<<4), //!< Electric codebook. 
	WORK_MODE_IS_OFB = (2<<4), //!< Output feedback.
	WORK_MODE_IS_CFB = (3<<4), //!< Cipher Feedback.
	WORK_MODE_IS_CTR = (4<<4), //!< Counter mode, only available in AES.
};

/*! @enum WORK_SUB_MODULE
 *   @brief Define all sub-algorithms DSC suppoted.
 */
enum WORK_SUB_MODULE
{
	DES=0,//!< Indicates the sub-algorithm is DES. 
	TDES=3,//!< Indicates the sub-algorithm is DES. 
	AES=1,//!<  Indicates the sub-algorithm is AES. 
	SHA=2,//!<  Indicates the sub-algorithm is SHA.
	CSA=4,//!<  Indicates the sub-algorithm is CSA. 
};

/*! @enum CRYPT_SELECT
 *   @brief Encrypt/decrypt selection
 */
enum CRYPT_SELECT
{
	DSC_DECRYPT=1,//!< Indicates the mode is decryption.
	DSC_ENCRYPT=0//!< Indicates the mode is encryption.
};


/*! @enum DSC_STATUS
 *   @brief Return values of the DSC functions.
 */
typedef enum DSC_STATUS
{
	ALI_DSC_SUCCESS = 0, //!< The intended operation was executed successfully.
	ALI_DSC_ERROR,  //!< The function terminated abnormally. The intended operation failed.
	ALI_DSC_WARNING_DRIVER_ALREADY_INITIALIZED, //!< The SEE DSC is already initialized.

	ALI_DSC_ERROR_INVALID_PARAMETERS = 0x1F00, //!< The passed parameters are invalid.
	ALI_DSC_ERROR_OPERATION_NOT_ALLOWED, //!< The requested operation is not allowed.
	ALI_DSC_ERROR_OPERATION_NOT_SUPPORTED,//!< The requested operation is not supported.
	ALI_DSC_ERROR_INITIALIZATION_FAILED, //!< The SEE DSC initialization failed.
	ALI_DSC_ERROR_DRIVER_NOT_INITIALIZED,//!< The SEE DSC isn't initialized.
	ALI_DSC_ERROR_INVALID_ADDR, //!< The passed address is invalid.
	ALI_DSC_ERROR_INVALID_DEV, //!< The passed device pointer is invalid.
	ALI_DSC_ERROR_INVALID_HANDLE, //!< The passed key HANDLE is invalid.
	ALI_DSC_ERROR_NO_IDLE_HANDLE,//!< Key HANDLE table is full.
	ALI_DSC_ERROR_NO_IDLE_PIDRAM,//!< PID table is full.
	ALI_DSC_ERROR_NO_IDLE_KEYRAM,//!< Clear key table is full.
}DSC_STATUS;


/*! @struct des_init_param
 *   @brief Define DES device initialization parameters.
 */
typedef struct des_init_param 
{
	enum PARITY_MODE parity_mode;//!< Parity of key.
	enum KEY_TYPE key_from;//!< Key source.
	unsigned int scramble_control;//!< Reserved item.
	enum KEY_MAP_MODE key_mode;//!< Key mode based on key length.
	unsigned int stream_id;//!< The stream ID for current device.
	enum DMA_MODE dma_mode;//!< The data mode, TS or pure data.
	enum RESIDUE_BLOCK residue_mode;//!< The residue block handling mode.
	enum WORK_MODE work_mode;//!< The cipher chaining mode.
	enum WORK_SUB_MODULE sub_module;//!< The DES sub mode, DES or TDES.
	unsigned int cbc_cts_enable ;  //!< CBC CTS mode control.
}DES_INIT_PARAM, *pDES_INIT_PARAM;

/*! @struct aes_init_param
 *   @brief Define AES device initialization parameters.
 */
typedef struct aes_init_param 
{
	enum PARITY_MODE parity_mode;//!< Parity of key.
	enum KEY_TYPE key_from;//!< Key source.
	unsigned int scramble_control;//!< Reserved item.
	enum KEY_MAP_MODE key_mode;//!< Key mode based on key length. Refer to '#KEY_MAP_MODE'.
	unsigned int stream_id; //!< The stream ID for current device.
	enum DMA_MODE dma_mode;//!< The data mode, TS or pure data.
	enum RESIDUE_BLOCK residue_mode;//!< The residue block handling mode.
	enum WORK_MODE work_mode;//!< The cipher chaining mode.
	unsigned int cbc_cts_enable ; //!< CBC CTS mode control.
}AES_INIT_PARAM, *pAES_INIT_PARAM;

/*! @struct csa_init_param
 *   @brief Define CSA device initialization parameters.
 */
typedef struct csa_init_param 
{
	enum CSA_VERSION version;//!< Version of DVB-CSA.
	enum DMA_MODE dma_mode;//!< The data mode, TS or pure data. 
	unsigned int Dcw[4];//!<Reserved item. 
	unsigned int pes_en;//!<Reserved item. 

	enum PARITY_MODE parity_mode; //!< Parity of key.
	enum KEY_TYPE key_from;//!< Key source.
	unsigned int scramble_control;//!< Reserved item.
	unsigned int stream_id; //!< The stream ID for current device.
}CSA_INIT_PARAM, *pCSA_INIT_PARAM;

/*! @struct sha_init_param
 *   @brief Define SHA device initialization parameters.
 */
typedef struct sha_init_param
{
	enum SHA_MODE sha_work_mode; //!<SHA hash mode.
	enum SHA_DATA_SOURCE sha_data_source;//!<SHA data source.
	unsigned int sha_buf;  //!<Reserved item.      
} SHA_INIT_PARAM, *pSHA_INIT_PARAM;


/*! @struct CRYPTO_128BITS_KEY
 *   @brief Define 128 bits clear key buffer.
 */
typedef struct CRYPTO_128BITS_KEY
{
	unsigned char even_key[16]; //!< 16 bytes even key buffer.
	unsigned char odd_key[16]; //!< 16 bytes odd key buffer.
}CRYPTO_128BITS_KEY;

/*! @struct CRYPTO_128BITS_IV
 *   @brief Define 128 bits iv buffer.
 */
typedef struct CRYPTO_128BITS_IV
{
	unsigned char even_iv[16]; //!< 16 bytes even iv buffer.
	unsigned char odd_iv[16]; //!< 16 bytes odd iv buffer.
}CRYPTO_128BITS_IV;

/*! @struct AES_128Bit_KEY
 *   @brief Define 128 bits AES clear key buffer.
 */
struct AES_128Bit_KEY 
{
	unsigned char even_key[16]; //!< 16 bytes even key buffer.
	unsigned char odd_key[16]; //!< 16 bytes odd key buffer.
};

/*! @struct AES_128BITS_KEY
 *   @brief Define 128 bits AES clear key buffer.
 */
typedef struct AES_128Bit_KEY AES_128BITS_KEY;

/*! @struct AES_192Bit_KEY
 *   @brief Define 192 bits AES clear key buffer.
 */
struct AES_192Bit_KEY 
{
	unsigned char even_key[24]; //!< 24 bytes even key buffer.
	unsigned char odd_key[24]; //!< 24 bytes odd key buffer.
};

/*! @struct AES_192BITS_KEY
 *   @brief Define 192 bits AES clear key buffer.
 */
typedef struct AES_192Bit_KEY AES_192BITS_KEY;

/*! @struct AES_256Bit_KEY
 *   @brief Define 256 bits AES clear key buffer.
 */
struct AES_256Bit_KEY 
{
	unsigned char even_key[32]; //!< 32 bytes even key buffer.
	unsigned char odd_key[32]; //!< 32 bytes ddd key buffer.
};

/*! @struct AES_256BITS_KEY
 *   @brief Define 256 bits AES clear key buffer.
 */
typedef struct AES_256Bit_KEY AES_256BITS_KEY;

/*! @union AES_KEY_PARAM
 *   @brief Define AES clear key buffer.
 */
typedef union AES_KEY_PARAM
{
	struct AES_128Bit_KEY aes_128bit_key;//!< 128 bits key.
	struct AES_192Bit_KEY aes_192bit_key;//!< 192 bits key.
	struct AES_256Bit_KEY aes_256bit_key;//!< 256 bits key.
}AES_KEY_PARAM;

/*! @struct AES_IV_INFO
 *   @brief Define AES initialization vector buffer.
 */
typedef struct AES_IV_INFO
{
	unsigned char even_iv[16]; //!< 16 bytes even iv buffer.
	unsigned char odd_iv[16]; //!< 16 bytes odd iv buffer.
}AES_IV_INFO;


/*! @struct CSA_KEY
 *   @brief Define CSA1.1/CSA2.0 64bits clear key buffer.
 */
struct CSA_KEY
{
	unsigned char OddKey[8];//!< 8 bytes odd key buffer.
	unsigned char EvenKey[8];//!< 8 bytes even key buffer.
};

/*! @struct CSA_64BITS_KEY
 *   @brief Define CSA1.1/CSA2.0 64bits clear key buffer.
 */
typedef struct CSA_KEY CSA_64BITS_KEY;

/*! @struct CSA3_KEY
 *   @brief Define CSA3 128bits clear key buffer.
 */
struct CSA3_KEY
{
	unsigned char OddKey[16]; //!< 16 bytes odd key buffer.
	unsigned char EvenKey[16];//!< 16 bytes even key buffer.
};

/*! @struct CSA3_128BITS_KEY
 *   @brief Define CSA3 128bits clear key buffer.
 */
typedef struct CSA3_KEY CSA3_128BITS_KEY;

/*! @union CSA_KEY_PARAM
 *   @brief Define DVB-CSA clear key buffer. 
 */
typedef union CSA_KEY_PARAM
{
	struct CSA_KEY csa_key; //!< CSA1.1/CSA2.0 64 bits key
	struct CSA3_KEY csa3_key;//!< CSA3 128 bits key
}CSA_KEY_PARAM;

/*! @struct DES_64BITS_KEY_INFO
 *   @brief Define DES 64bits clear key buffer.
 */
struct  DES_64BITS_KEY_INFO
{
	unsigned char OddKey[8];//!< 8 bytes odd key buffer.
	unsigned char EvenKey[8];//!< 8 bytes even key buffer.
};

/*! @struct DES_64BITS_KEY
 *   @brief Define DES 64bits clear key buffer.
 */
typedef struct DES_64BITS_KEY_INFO DES_64BITS_KEY;

/*! @struct DES_128BITS_KEY_INFO
 *   @brief Define TDES 128bits clear key buffer. 
 */
struct DES_128BITS_KEY_INFO
{
	unsigned char OddKey[16];//!< 16 bytes odd key buffer.
	unsigned char EvenKey[16];//!< 16 bytes even key buffer.
};

/*! @struct TDES_128BITS_KEY
 *   @brief Define TDES 128bits clear key buffer. 
 */
typedef struct DES_128BITS_KEY_INFO TDES_128BITS_KEY;

/*! @struct DES_192BITS_KEY_INFO
 *   @brief Define TDES 192bits clear key buffer.
 */
struct DES_192BITS_KEY_INFO
{
	unsigned char OddKey[24]; //!< 16 bytes odd key buffer.
	unsigned char EvenKey[24]; //!< 16 bytes even key buffer.
};

/*! @struct TDES_192BITS_KEY
 *   @brief Define TDES 192bits clear key buffer.
 */
typedef struct DES_192BITS_KEY_INFO TDES_192BITS_KEY;

/*! @union DES_KEY_PARAM
 *   @brief Define DES/TDES clear key buffer.
 */
typedef union DES_KEY_PARAM
{
	struct  DES_64BITS_KEY_INFO des_64bits_key;//!< DES 64 bits key
	struct  DES_128BITS_KEY_INFO des_128bits_key;//!< TDES 128 bits key
	struct  DES_192BITS_KEY_INFO des_192bits_key;//!< TDES 192 bits key
}DES_KEY_PARAM;

/*! @struct DES_IV_INFO
 *   @brief Define DES initialization vector buffer.
 */
typedef struct DES_IV_INFO
{
	unsigned char even_iv[8]; //!< 8 bytes even iv buffer.
	unsigned char odd_iv[8]; //!< 8 bytes odd iv buffer.
}DES_IV_INFO;

/*! @struct KEY_PARAM
 *   @brief Define key parameters. The KEY_PARAM must be initialized to zero before configuration.
 */
typedef struct KEY_PARAM
{
	unsigned int handle ;  //!< Key HANDLE returned from driver.
	unsigned short *pid_list;//!< Specify the PIDs which will be set with this key.
	unsigned short pid_len;  //!< Specify the '*pid_list' elements number, recommand value is 1.
	AES_KEY_PARAM *p_aes_key_info;//!< AES clear key buffer for #KEY_FROM_SRAM mode.
	/*!< Only need to copy the input clear key to the even key buffer if the AES device works in #PURE_DATA_MODE.
	* If the 'pid_len' is greater than '1', this paramter should pointer to the array defined by 'struct AES_128Bit_KEY', 
	'struct AES_192Bit_KEY' or 'struct AES_256Bit_KEY' (based on the actual key length, do not use the AES_KEY_PARAM to define the array), 
	such as 'struct AES_128Bit_KEY clear_key[pid_len];', and user needs to copy the input clear key to this array one by one
	(this functionality allows the different PIDs can use the different keys).
	*/
	CSA_KEY_PARAM *p_csa_key_info;//!< CSA clear key buffer for #KEY_FROM_SRAM mode.
	/*!< If the 'pid_len' is greater than '1', this paramter should pointer to the array defined by 'struct CSA_KEY', 
	or 'struct CSA3_KEY' (based on the actual key length, do not use the CSA_KEY_PARAM to define the array), 
	such as 'struct CSA_KEY clear_key[pid_len];', and user needs to copy the input clear key to this array one by one
	(this functionality allows the different PIDs can use the different keys).
	*/	
	DES_KEY_PARAM *p_des_key_info;//!< DES/TDES clear key buffer for #KEY_FROM_SRAM mode.
	/*!< Only need to copy the input clear key to the even key buffer if the DES device works in #PURE_DATA_MODE.
	* If the 'pid_len' is greater than '1', this paramter should pointer to the array defined by 'struct  DES_64BITS_KEY_INFO', 
	'struct  DES_128BITS_KEY_INFO' or 'struct  DES_192BITS_KEY_INFO' (based on the actual key length, 
	do not use the DES_KEY_PARAM to define the array), such as 'struct DES_128BITS_KEY_INFO clear_key[pid_len];', 
	and user needs to copy the input clear key to this array one by one (this functionality allows the different PIDs can use the different keys).	
	*/	
	unsigned int key_length;//!<Key length (bit).
	AES_IV_INFO *p_aes_iv_info;//!< AES IV buffer.
	/*!< When using #KEY_FROM_OTP mode, the IV pointer is *init_vector. When using #KEY_FROM_SRAM or #KEY_FROM_CRYPTO mode, 
	*	the IV pointer is *p_aes_iv_info. Only need to copy the input iv to the even iv buffer if the AES device works in #PURE_DATA_MODE.
	*/	
	DES_IV_INFO *p_des_iv_info;//!< DES/TDES IV buffer.
	/*!< When using #KEY_FROM_OTP mode, the IV pointer is *init_vector. When using #KEY_FROM_SRAM or #KEY_FROM_CRYPTO mode, 
	*	the IV pointer is *p_des_iv_info. Only need to copy the input iv to the even iv buffer if the DES device works in #PURE_DATA_MODE.
	*/	
	unsigned short stream_id;//!< The corresponding stream ID. Should be same as the stream ID which is used for initializing the device.

	unsigned char *init_vector; //!< The initialization vector pointer in #KEY_FROM_OTP mode.
	unsigned char *ctr_counter;//!< The counter pointer for AES #WORK_MODE_IS_CTR mode.
	unsigned char force_mode; //!< Reserved item.
	unsigned char pos;//!< When using #KEY_FROM_CRYPTO mode, the corresponding key index in KL should be assigned to this pos.
	unsigned char no_even;//!< Even key configuration control.
	/*!< If the 'no_even = 1' is set, the even clear key will not be updated/configured.
	*	If the 'no_even = 0' is set, the even clear key will be updated/configured.
	*/	
	unsigned char no_odd;//!< Odd key configuration control.
	/*!< If the 'no_odd = 1' is set, the odd clear key will not be updated/configured.
		If the 'no_odd = 0' is set, the odd clear key will be updated/configured.
	*/	
	unsigned char not_refresh_iv;//!< IV chaining control.
	/*!< If the 'not_refresh_iv = 1' is set, the new IV value will not be updated/configured. (Using the original chaing register value).
	*	If the 'not_refresh_iv = 0' is set, the new IV value will be updated/configured.
	*/		
}KEY_PARAM, *pKEY_PARAM;

/*! @struct INIT_PARAM_NOS
 *   @brief Reserved item.
 */
typedef struct INIT_PARAM_NOS
{
	enum PARITY_MODE  parity_mode;    
	enum KEY_TYPE key_from;
	unsigned int scramble_control;
	enum KEY_MAP_MODE key_mode;
	unsigned int stream_id; 
	enum DMA_MODE dma_mode;
	enum RESIDUE_BLOCK  residue_mode;
	enum WORK_MODE work_mode;
	enum WORK_SUB_MODULE sub_module;
	unsigned int cbc_cts_enable;

	KEY_PARAM *pkeyParam;
}INIT_PARAM_NOS, *pINIT_PARAM_NOS;


/*! @struct SHA_DEV
 *   @brief Reserved for HLD SHA device structure.
 */
typedef struct SHA_DEV
{
	struct SHA_DEV  *next;
	int type;
	char name[16];
	void *pRoot;
	void *priv ; 
	void (*open)(struct SHA_DEV *);
	void (*close)(struct SHA_DEV *);
	int (*digest)(struct SHA_DEV * , unsigned char *, unsigned char *,unsigned int );
	int  (*Ioctl)( struct SHA_DEV * ,unsigned int cmd , unsigned int param);
	unsigned char id_number;	
	int fd;
}SHA_DEV,*pSHA_DEV;

/*! @struct AES_DEV
 *   @brief Reserved for HLD AES device structure.
 */
typedef struct AES_DEV
{
	struct AES_DEV *next; 
	int type;
	char name[16]; 
	void *pRoot;		
	void *priv ;
	void (*open)(struct AES_DEV *);
	void (*close)(struct AES_DEV *);
	int (*Encrypt)(struct AES_DEV * ,unsigned short, unsigned char *, unsigned char *, unsigned int );
	int (*Decrypt)(struct AES_DEV * ,unsigned short, unsigned char *, unsigned char *, unsigned int);
	int (*Ioctl)(struct AES_DEV *,unsigned int cmd,unsigned int param);
	unsigned char id_number;
	int fd;
}AES_DEV,*pAES_DEV;


/*! @struct DES_DEV
 *   @brief Reserved for HLD DES device structure.
 */
typedef struct DES_DEV
{
	struct DES_DEV *next;
	int type;
	char name[16];
	void *pRoot;		
	void *priv ;
	void (*open)(struct DES_DEV * );
	void (*close)(struct DES_DEV *);
	int (*Encrypt)(struct DES_DEV *, unsigned short,unsigned char *, unsigned char *, unsigned int );
	int (*Decrypt)(struct DES_DEV *, unsigned short,unsigned char *, unsigned char *, unsigned int );
	int (*Ioctl)(struct DES_DEV *,unsigned int cmd,unsigned int param);
	unsigned char id_number;      
	int fd;
}DES_DEV,*pDES_DEV;

/*! @struct CSA_DEV
 *   @brief Reserved for HLD CSA device structure.
 */
typedef struct CSA_DEV
{
	struct CSA_DEV *next; 
	int type;
	char name[16];
	void *pRoot;		
	void *priv;
	void (*open)(struct CSA_DEV*);
	void (*close)(struct CSA_DEV*);
	int (*Decrypt)(struct CSA_DEV*,unsigned short, unsigned char*, unsigned char*, unsigned int );
	int (*Ioctl)(struct CSA_DEV *,unsigned int cmd,unsigned int param);
	unsigned char id_number;
	int fd;
}CSA_DEV,*pCSA_DEV;

/*! @struct DSC_DEV
 *   @brief Reserved for HLD DSC device structure.
 */
typedef struct DSC_DEV
{
	struct DSC_DEV *next; 
	int type;
	char name[16]; 
	void *priv;		
	unsigned int base_addr;
	unsigned int  interrupt_id;

	void      (*attach)(void);
	void      (*detach)( struct DSC_DEV *);
	int	(*open)( struct DSC_DEV *);
	int   	(*close)( struct DSC_DEV *);
	int   (*ioctl)(struct DSC_DEV *, unsigned int , unsigned int );
	int fd;
	void *user_base;
}DSC_DEV, *pDSC_DEV;

/*! @struct DSC_PVR_KEY_PARAM
 *   @brief Define PVR key parameters.
 */
typedef struct DSC_PVR_KEY_PARAM
{
	unsigned int input_addr;//!<PVR cipher key address in memory.
	unsigned int valid_key_num;//!<Specify the total valid cipher key number, one key corresponding to one recorded file.
	unsigned int current_key_num;//!<Cipher key index used for current recording.
	unsigned int pvr_key_length;//!<PVR key bit number, should be set to 128 bits.
	unsigned char pvr_user_key_pos;//!<PVR key index in the KL. It's generated by using the PVR cipher key.
	unsigned int total_quantum_number; //!<Specify the max file size for each recording file, file size is equal to total_quantum_number*47K Bytes.
	unsigned int current_quantum_number;//!<Reserved item.
	unsigned int ts_packet_number;//!<Reserved item.
	unsigned char pvr_key_change_enable;//!<The value is always "true".
	/*!< When the file size reached the specified max file size, DSC will call KL API to generate the next 
	 *	key for recording the next file.
	*/
	unsigned short stream_id;//!<Encryption stream ID for current PVR channel. It should be same as the stream ID when creating the key HANDLE for PVR.
	unsigned char pvr_first_key_pos;//!<User need to configure the 2nd level key index if the KL works in 3-level mode. 
}DSC_PVR_KEY_PARAM,*pDSC_PVR_KEY_PARAM;


/*! @struct DSC_BL_UK_PARAM
 *   @brief Reserved for BL UK.
 */
typedef struct DSC_BL_UK_PARAM
{
	unsigned char *input_key;
	unsigned char *r_key;
	unsigned char *output_key;
	unsigned int crypt_type;
}DSC_BL_UK_PARAM,*pDSC_BL_UK_PARAM;


/*! @struct DSC_VER_CHK_PARAM
 *   @brief Reserved for version check.
 */
typedef struct DSC_VER_CHK_PARAM
{
	unsigned int input_mem;
	unsigned int len;
	unsigned int chk_mode; 
	unsigned int chk_id ;
}DSC_VER_CHK_PARAM,*pDSC_VER_CHK_PARAM;

/*!  @struct DEEN_CONFIG
 *   @brief Define PVR re-encryption configuration parameters.
 */
typedef struct DEEN_CONFIG
{
	unsigned int do_encrypt ;//!<Encryption enabled. 
	/*!< If do_encrypt is true, the decrypted stream will be encrypted by the specified sub-algorithm. 
	* If the stream to be recorded is in scrambled format, the do_encrypt will be forced to true by driver.
	*/
	void *dec_dev;  //!<Assign the sub-algorithm's device pointer for decryption.
	unsigned char Decrypt_Mode;//!<Specify the sub-algorithm for decryption, #CSA, #AES or #TDES. 
	unsigned short dec_dmx_id;//!<Stream ID for decryption.
	unsigned int do_decrypt ;//!<Decryption enable.
	void *enc_dev;   //!<Assign the sub-algorithm's device pointer for encryption.
	unsigned char Encrypt_Mode;//!<Specify the sub-algorithm for encryption, #AES or #TDES.
	unsigned short enc_dmx_id;//!<Stream ID for encryption. This value must not be same as the dec_dmx_id.
}DEEN_CONFIG,*pDEEN_CONFIG;

/*! @struct DSC_EN_PRIORITY
 *   @brief Struct for user to change the encryption parity while do PVR re-encryption.
 */
typedef struct DSC_EN_PRIORITY
{
	enum WORK_SUB_MODULE sub_module;//!<The encryption module.
	enum PARITY_MODE priority;//!<New parity mode.
	unsigned int dev_ptr; //!<The encryption device pointer.
}DSC_EN_PRIORITY,*pDSC_EN_PRIORITY;


/*! @struct DSC_FIXED_CRYPTION
 *   @brief Reserved item.
 */
typedef struct DSC_FIXED_CRYPTION
{
	unsigned char *input;
	unsigned int length; 
	unsigned int pos;
}DSC_FIXED_CRYPTION,*pDSC_FIXED_CRYPTION;

/*! @struct DSC_SYS_UK_FW
 *   @brief Reserved item.
 */
typedef struct DSC_SYS_UK_FW
{
	unsigned char *input;
	unsigned char *output;
	unsigned int length; 
	unsigned char ck[16];
	unsigned char pos;
	enum CRYPT_SELECT mode;
}DSC_SYS_UK_FW,*pDSC_SYS_UK_FW;

/*!
@}
*/

#endif 

