#ifndef  _ALI_TRNG_COMMON_H_
#define  _ALI_TRNG_COMMON_H_

 
/*!@addtogroup ALiTRNG
 *  @{
    @~
 */

#define ALI_TRNG_LINUX_DEV_PATH "/dev/ali_trng_0"  //!< TRNG device node in kernel.

#define ALI_TRNG_MAX_GROUP 16 //!< Max group number for user to get the TRNG data per time.
#define ALI_TRNG_64BITS_SIZE 8  //!< Group unit in byte.

#define ALI_TRNG_BASE	                    (0xC0<<8) //!< TRNG ioctl cmd base.
#define ALI_TRNG_GENERATE_BYTE       (ALI_TRNG_BASE + 0) 
//!< TRNG ioctl cmd to generate 1 byte TRNG data and output the data to caller.
#define ALI_TRNG_GENERATE_64bits     (ALI_TRNG_BASE + 1)
//!< TRNG ioctl cmd to generate 8 bytes TRNG data and output the data to caller.
#define ALI_TRNG_GET_64bits              (ALI_TRNG_BASE + 2)
//!< TRNG ioctl cmd which is used to get 8*n_group() bytes TRNG data, along with 'ALI_TRNG_GET_64BITS'.

/*! @struct ALI_TRNG_GET_64BITS
@brief Specify the parameter for user to get the TRNG data.
*/
typedef struct ALI_TRNG_GET_64BITS
{
	unsigned char *data; 
	//!<Specify the buffer address(user or kernel space) that user wants to store the TRNG data.
	unsigned int n_group;
	//!<Specify the TRNG group number(group unit is #ALI_TRNG_64BITS_SIZE, 8Bytes) that user wants to get from driver.
}ALI_TRNG_GET_64BITS;

/*!
@}
*/

#endif

