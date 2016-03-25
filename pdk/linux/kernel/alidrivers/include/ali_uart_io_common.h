#ifndef _ALI_UART_COMMON_H_
#define _ALI_UART_COMMON_H_

#include <linux/types.h>

#ifdef __cplusplus
extern "C" {
#endif
	
/*! @addtogroup DeviceDriver 
 *  @{
 */

/*! @addtogroup ALiUARTIO
 *  @{
 */

/*! @struct uart_pars
 @brief ALi uartio read struct
 */
struct uart_pars
{
    unsigned long tm;    //!< Timeout for reading a character
		unsigned char ch;    //!< Character to read
};
#define SCI_16550UART_RX_BUF_SIZE	     2048              //!<2Kbyte buff

#define ALI_UART_IO_READ 	 _IOR('U', 1, __u8)           //!< uart io read ctrl cmd
#define ALI_UART_IO_READ_TM  _IOWR('U',2, __u32)        //!< uart io read ctrl cmd with timeout
#define ALI_UART_IO_WRITE	 _IOW('U', 3, __u8)           //!< uart io write ctrl cmd
#define ALI_UART_IO_SET      _IOW('U', 4, __u32)        //!< uart reg settings
#define ALI_UART_DISABLE     _IOW('U', 5, __u8)         //!< Not used
#define ALI_UART_ENABLE      _IOW('U', 6, __u8)         //!< Not used
#define ALI_UART_IO_READ_TIMEOUT  _IOWR('U', 7, __u32 ) //!< The same with ALI_UART_IO_READ_TM
#define ALI_UART_IO_CLEAR_SCI     _IOW('U', 8, __u32 )  //!< Release aliuart to uartio dev itself
#define ALI_UART_IO_SET_SCI       _IOW('U', 9, __u32 )  //!< Have aliuart to uartio dev itself
#define ALI_UART_SFU_MODE	_IOW('U', 10, __u8)           //!< The same with ALI_UART_IO_SET_SCI
#define ALI_UART_IO_WRITE_TIMEOUT _IOW('U', 11, __u32)  //!< uart io write ctrl cmd with timeout

/*!
@}
*/

/*!
@}
*/

#ifdef __cplusplus
}
#endif

#endif
