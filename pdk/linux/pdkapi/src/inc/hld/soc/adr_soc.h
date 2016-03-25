/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    adr_soc.h
*
*    Description:    This file contains all functions definition
*		             of soc driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	
*****************************************************************************/

#ifndef __ADR_HLD_SOC_H__
#define __ADR_HLD_SOC_H__


/*! @addtogroup 设备驱动
 *  @{
 */
 
/*! @addtogroup 系统信息
 *  @{
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <ali_soc_common.h>


/*! @enum SOC_SYS_SHM
@brief 系统信息模块的系统共享内存类型
*/
enum SOC_SYS_SHM
{
	SYS_SHM_AUTO,
	SYS_SHM_OSD2,	
};


/*!
@brief 打开系统信息设备。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_open(void);

/*!
@brief 关闭系统信息设备。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_close(void);

/*!
@brief 获得芯片id。
@return UINT32。
@retval  芯片id。
*/
UINT32 soc_get_chip_id(void);

/*!
@brief 获得芯片版本。
@return UINT32。
@retval  芯片版本。
*/
UINT32 soc_get_rev_id(void);

/*!
@brief 获得cpu 时钟频率。
@return UINT32。
@retval  cpu时钟频率。
*/
UINT32 soc_get_cpu_clock(void);

/*!
@brief 获得dram 时钟频率。
@return UINT32。
@retval  dram时钟频率。
*/
UINT32 soc_get_dram_clock(void);

/*!
@brief 获得dram 大小。
@return UINT32。
@retval  dram大小。
*/
UINT32 soc_get_dram_size(void);

/*!
@brief 每次从寄存器读取一个字节。
@param[in] addr 所要读取的寄存器地址。
@param[out] buf 接收数据的地址。
@param[in] len 接收数据的长度，以一个字节为单位。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_read8(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief 每次从寄存器读取两个字节。
@param[in] addr 所要读取的寄存器地址。
@param[out] buf 接收数据的地址。
@param[in] len 接收数据的长度，以两个字节为单位。。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_read16(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief 每次从寄存器读取四个字节。
@param[in] addr 所要读取的寄存器地址。
@param[out] buf 接收数据的地址。
@param[in] len 接收数据的长度，以四个字节为单位。。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_read32(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief 每次往寄存器写一个字节。
@param[in] addr 所要写的寄存器地址。
@param[in] buf 往寄存器写的数据地址。
@param[in] len 往寄存器写的数据长度，以一个字节为单位。。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_write8(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief 每次往寄存器写两个字节。
@param[in] addr 所要写的寄存器地址。
@param[in] buf 往寄存器写的数据地址。
@param[in] len 往寄存器写的数据长度，以两个字节为单位。。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_write16(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief 每次往寄存器写四个字节。
@param[in] addr 所要写的寄存器地址。
@param[in] buf 往寄存器写的数据地址。
@param[in] len 往寄存器写的数据长度，以四个字节为单位。。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_write32(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief 从内核地址读取数据
@param[in] from 所要读取的内核地址。
@param[out] to 接收数据的地址。
@param[in] len 接收数据的长度，以字节为单位。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 ali_soc_read(UINT8 * to, UINT8 * from, UINT32 len);


/*!
@brief 往内核地址写数据。
@param[in] to 所要写数据的内核地址。
@param[in] from 数据地址。
@param[in] len 数据长度，以字节为单位。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 ali_soc_write(UINT8 * to, UINT8 * from, UINT32 len);


/*!
@brief 每次从外围设备内存地址读取四个字节。
@param[in] addr 所要读取的外围设备内存地址。
@param[out] buf 接收数据的地址。
@param[in] len 接收数据的长度，以四个字节为单位。。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_per_read32(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief 每次往外围设备内存地址写四个字节。
@param[in] addr 所要写的外围设备内存地址。
@param[in] buf 往外围设备内存写的数据地址。
@param[in] len 往外围设备内存写的数据长度，以四个字节为单位。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_per_write32(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief 获得系统内存映射表。
@param[out] smm 获得的系统内存映射表地址。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_get_memory_map(struct soc_memory_map *smm);

/*!
@brief 设置系统信息模块的调试等级。
@param[in] level 需要设置的调试等级。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_set_level(UINT32 level);

/*!
@brief 获得see cpu的版本。
@param[out] buf 获得的版本地址。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_get_see_ver(UINT8 *buf);

/*!
@brief 启动see打印。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_hit_see_heart(void);

/*!
@brief 使能see异常打印。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_enable_see_exception(void);


/*!
@brief 读取板子相关的配置内存。
@param[out] buf_addr内存地址 。
@param[out] buf_size内存大小。
*/
void soc_get_reserved_men_buf(UINT32 *buf_addr, UINT32 *buf_size);

/*!
@brief 读取media player 相关的配置内存。
@param[out] buf_addr内存地址 。
@param[out] buf_size内存大小。
*/
void soc_get_media_buf(UINT32 *buf_addr, UINT32 *buf_size);

/*!
@brief 打开或者关闭see cpu的printf输出
@param[in] enable标志信息。0 -- 关闭，1 -- 打开。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_set_see_printf(INT32 enable);

/*!
@brief 读取系统的共享内存空间。
@param[out] buf_addr内存类型地址 。
@param[out] buf_addr内存地址 。
@param[out] buf_size内存大小。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_get_share_mem(enum SOC_SYS_SHM type, void **buf_addr, INT32 *buf_size);

/*!
@brief 关闭不需要的IP clk/dac ,用于HDMI DONGLE节省功耗。
@param[in] power_down 该参数暂时没有使用。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_enable_power_down(INT32 power_down);	

/*!
@brief 显示see 插件信息。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 soc_show_see_plugin_info(void);

	
#ifdef __cplusplus
}
#endif


/*!
 @}
 */
 
 /*!
 @}
 */
 
#endif /* __ADR_HLD_SOC_H__ */
