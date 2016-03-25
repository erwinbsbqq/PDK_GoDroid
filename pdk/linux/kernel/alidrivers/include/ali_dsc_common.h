#ifndef _ALI_DSC_COMMON_H_
#define _ALI_DSC_COMMON_H_

#include <alidefinition/adf_dsc.h>

/*! @addtogroup ALiDSC
 *  @{
    @~
 */

#define ALI_CSA_LINUX_DEV_PATH "/dev/ali_csa_0" //!< CSA device node in kernel.
#define ALI_AES_LINUX_DEV_PATH "/dev/ali_aes_0" //!< AES device node in kernel.
#define ALI_DES_LINUX_DEV_PATH "/dev/ali_des_0" //!< DES device node in kernel.
#define ALI_SHA_LINUX_DEV_PATH "/dev/ali_sha_0" //!< SHA device node in kernel.
#define ALI_DSC_LINUX_DEV_PATH "/dev/ali_dsc_0" //!< DSC device node in kernel.

#define DSC_U_MEM_SIZE (1024*1024) //!< DSC kernel memory size reserved for user space.
#define MEM_ALLOC_UNIT (128*1024) //!< DSC memory uint.
#define DSC_MEM_BLOCK_COUNT (8) //!< DSC memory number #DSC_U_MEM_SIZE/#MEM_ALLOC_UNIT.


/*! @struct dsc_see_dev_hld
 *   @brief Specify the device ID to get the device pointer.
*/
struct dsc_see_dev_hld 
{
	unsigned int dsc_dev_hld;  //!< The device pointer in SEE
	unsigned int dsc_dev_id;   //!< The device ID of the sub-algorithm
};

/*! @struct dec_parse_param
 *   @brief Specify the algotirhm and its device pointer for live play or playback.
*/
struct dec_parse_param
{  
	void *dec_dev;   //!< The device pointer of the specified algotirhm.
	unsigned int type;//!< Specify the decryption algorithm.
};

/*! @struct ali_dsc_krec_mem
 *   @brief Struct for user to allocate or free the DSC kernel memory.
*/
typedef struct ali_dsc_krec_mem
{
	unsigned int size;//!< Size to be allocated.
	void *pa_mem;//!< Kernel address of the allocated memory.
	void *va_mem;//!< For user to store the corresponding user space address.
} ALI_DSC_KREC_MEM;

/*! @struct ALI_DSC_CRYPTION
 *   @brief Specify the parameters for decryption and encryption.
*/
typedef struct ALI_DSC_CRYPTION
{
	void *dev; //!< Device ID or device pointer of the DSC sub-algorithm.
	unsigned int stream_id; //!< Stream ID which is used for create the key HANDLE.
	unsigned char *input; //!< Input buffer address.
	unsigned char *output; //!< Output buffer address.
	unsigned int length; //!< Operation length. 
	/*!< It's bytes number for #PURE_DATA_MODE, and packets number for #TS_MODE.
	*/	
} ALI_DSC_CRYPTION;

/*! @struct ALI_DSC_IO_PARAM
 *   @brief Struct for DSC sub-algorithms AES/TDES/CSA/SHA to pass the ioctl parameters to kernel.
 */
typedef struct ALI_DSC_IO_PARAM
{
	void *dev; //!< Device ID or device pointer of the DSC sub-algorithm.
	void *ioc_param; //!< The corresponding parameter according to the ioctl cmd.
 	/*!< KEY_PARAM (#IO_CREAT_CRYPT_STREAM_CMD and #IO_KEY_INFO_UPDATE_CMD), \n
 		AES_INIT_PARAM (#IO_INIT_CMD), CSA_INIT_PARAM (#IO_INIT_CMD), \n
 		DES_INIT_PARAM (#IO_INIT_CMD), SHA_INIT_PARAM (#IO_INIT_CMD).
	*/
} ALI_DSC_IO_PARAM;

/*! @struct ALI_SHA_DIGEST
 *   @brief Specify the parameters for HASH.
 */
typedef struct ALI_SHA_DIGEST
{
	void *dev; //!< Device ID or device pointer of the SHA.
	unsigned char *input;//!<Input data buffer.
	unsigned char *output;//!<HASH buffer.
	unsigned int length;//!<Input data length.
} ALI_SHA_DIGEST;

/*! @struct ALI_DSC_RAM_MON
 *   @brief Specify the parameters for memory content monitor. 
 	If the memory content under monitoring has been changed, the system goes to crash.
 */
typedef struct ALI_DSC_RAM_MON
{
	unsigned int start_addr;//!< Start address to be monitored.
	unsigned int end_addr;//!< End address to be monitored.
	unsigned int interval;//!< Reserved, enforced by driver (10~15s).
	enum SHA_MODE sha_mode;//!< HASH mode, SHA-256.
	int dis_or_en;//!< Reserved, enforced by driver (enable).
} ALI_DSC_RAM_MON, *pALI_DSC_RAM_MON;


/*! @struct ALI_RE_ENCRYPT
 *   @brief Specify the parameters for PVR re-encryption.
 */
typedef struct ALI_RE_ENCRYPT
{
	DEEN_CONFIG *p_deen;//!< Encryption/decryption resource configuration.
	unsigned char *input;//!< Input TS buffer.
	unsigned char *output;//!< Output TS buffer.
	unsigned int length;//!< Packets number.
} ALI_RE_ENCRYPT;

/*! @struct ALI_DSC_ID_INFO
 *   @brief Specify the parameters for DSC to manage its sub-algorithms' AES/TDES/CSA/SHA sub-devices ID.
 	and manage the stream ID.
 */
typedef struct ALI_DSC_ID_INFO
{
	unsigned int mode;//!< Sub-algorithm (#AES/#DES/#CSA/#SHA) for device ID; Data mode (#PURE_DATA_MODE/#TS_MODE for stream ID).
	unsigned int id_number;//!< Specify the pointer to store the device/stream ID or the device ID to be freed.
 	/*!< For #ALI_DSC_IO_GET_FREE_STREAM_ID and #ALI_DSC_IO_GET_FREE_SUB_DEVICE_ID,\n
 		it should be the stream_id and device_id variable's address (id_number = &stream_id / id_number = &device_id).\n
 		
 		For #ALI_DSC_IO_SET_SUB_DEVICE_ID_IDLE and #ALI_DSC_IO_SET_SUB_DEVICE_ID_USED,\n
 		it should be the device_id variable's vaule (id_number = device_id;).\n

 		For #ALI_DSC_IO_SET_STREAM_ID_IDLE and #ALI_DSC_IO_SET_STREAM_ID_USED, user just need to call the \n
 		DSC ioctl directly (pass the stream ID to arg), no need to prepare this struct.\n

 		The sub-device ID range is 0~3 for each DSC sub-algorithm. \n
 		The stream ID range is 4~7 for pure data, 0~3 and 8~15 for TS data.
	*/
} ALI_DSC_ID_INFO;


/* Used for AES/TDES/CSA/SHA ioctl
*/
#define ALI_DSC_IO_DECRYPT								_IOW(IO_DSC_BASE, 0x40, ALI_DSC_CRYPTION)
//!< AES/TDES/CSA decryption, along with ALI_DSC_CRYPTION.
#define ALI_DSC_IO_ENCRYPT								_IOW(IO_DSC_BASE, 0x41, ALI_DSC_CRYPTION)
//!< AES/TDES encryption, along with ALI_DSC_CRYPTION.
#define ALI_DSC_IO_SHA_DIGEST							_IOW(IO_DSC_BASE, 0x42, ALI_SHA_DIGEST)
//!< SHA HASH, along with ALI_SHA_DIGEST.

/* Used for DSC ioctl 
*/
#define ALI_DSC_IO_TRIG_RAM_MON						_IOW(IO_DSC_BASE, 0x43, ALI_DSC_RAM_MON) 
//!< Trigger the memory monitor, along with ALI_DSC_RAM_MON.
#define ALI_DSC_IO_DEENCRYPT							_IOW(IO_DSC_BASE, 0x44, ALI_RE_ENCRYPT)  
//!< Re-encryption, along with ALI_RE_ENCRYPT.
#define ALI_DSC_IO_GET_FREE_STREAM_ID               		_IOWR(IO_DSC_BASE, 0x45, ALI_DSC_ID_INFO)   
//!< Get an idle stream ID based on the data mode, refer to ALI_DSC_ID_INFO.
#define ALI_DSC_IO_GET_FREE_SUB_DEVICE_ID          		_IOWR(IO_DSC_BASE, 0x46, ALI_DSC_ID_INFO)  
//!< Get an idle sub-device ID based on the sub-algorithm, refer to ALI_DSC_ID_INFO.
#define ALI_DSC_IO_SET_SUB_DEVICE_ID_IDLE				_IOW(IO_DSC_BASE, 0x47, ALI_DSC_ID_INFO)  
//!< Set the sub-device ID of the specified sub-algorithm to idle, refer to ALI_DSC_ID_INFO.
#define ALI_DSC_IO_SET_STREAM_ID_IDLE					_IO(IO_DSC_BASE, 0x48)
//!< Set the stream ID to idle.
#define ALI_DSC_IO_SET_STREAM_ID_USED					_IO(IO_DSC_BASE, 0x49) 
//!< Set the stream ID to busy.
#define ALI_DSC_IO_SET_SUB_DEVICE_ID_USED				_IOW(IO_DSC_BASE, 0x4A, ALI_DSC_ID_INFO) 
//!< Set the sub-device ID of the specified sub-algorithm to busy, refer to ALI_DSC_ID_INFO.
#define IO_GET_DEV_HLD									_IO(IO_DSC_BASE, 0x4B)
//!< Get the device pointer of the specified device ID, refer to 'struct dsc_see_dev_hld'.
#define IO_REQUEST_KREC_SPACE							_IO(IO_DSC_BASE, 0x4C)
//!< Allocate the DSC kernel memory, refer to 'struct ALI_DSC_KREC_MEM'.
/*!< Kernel will return an pa_mem to user space, user needs to calc this memory offset by 'pa_mem - kaddr'.
	'kaddr' can be got from #IO_REQUEST_KREC_ADDR, it's the DSC kernel memory base address.
	Then, user can map the DSC memory to user space by using mmap to get the user_base.
	Finally, the va_mem = pa_mem - kaddr + user_base.
*/
#define IO_RELEASE_KREC_SPACE							_IO(IO_DSC_BASE, 0x4D)
//!< Free the DSC kernel memory, refer to 'struct ALI_DSC_KREC_MEM'.
/*!< User needs to pass the pa_mem address to kernel.
*/
#define IO_REQUEST_KREC_ADDR							_IO(IO_DSC_BASE, 0x4E)
//!< Get the DSC kernel memory base address.

/*!
@}
*/

#endif 
