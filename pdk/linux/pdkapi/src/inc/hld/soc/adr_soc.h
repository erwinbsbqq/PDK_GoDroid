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


/*! @addtogroup �豸����
 *  @{
 */
 
/*! @addtogroup ϵͳ��Ϣ
 *  @{
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <ali_soc_common.h>


/*! @enum SOC_SYS_SHM
@brief ϵͳ��Ϣģ���ϵͳ�����ڴ�����
*/
enum SOC_SYS_SHM
{
	SYS_SHM_AUTO,
	SYS_SHM_OSD2,	
};


/*!
@brief ��ϵͳ��Ϣ�豸��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_open(void);

/*!
@brief �ر�ϵͳ��Ϣ�豸��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_close(void);

/*!
@brief ���оƬid��
@return UINT32��
@retval  оƬid��
*/
UINT32 soc_get_chip_id(void);

/*!
@brief ���оƬ�汾��
@return UINT32��
@retval  оƬ�汾��
*/
UINT32 soc_get_rev_id(void);

/*!
@brief ���cpu ʱ��Ƶ�ʡ�
@return UINT32��
@retval  cpuʱ��Ƶ�ʡ�
*/
UINT32 soc_get_cpu_clock(void);

/*!
@brief ���dram ʱ��Ƶ�ʡ�
@return UINT32��
@retval  dramʱ��Ƶ�ʡ�
*/
UINT32 soc_get_dram_clock(void);

/*!
@brief ���dram ��С��
@return UINT32��
@retval  dram��С��
*/
UINT32 soc_get_dram_size(void);

/*!
@brief ÿ�δӼĴ�����ȡһ���ֽڡ�
@param[in] addr ��Ҫ��ȡ�ļĴ�����ַ��
@param[out] buf �������ݵĵ�ַ��
@param[in] len �������ݵĳ��ȣ���һ���ֽ�Ϊ��λ��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_read8(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief ÿ�δӼĴ�����ȡ�����ֽڡ�
@param[in] addr ��Ҫ��ȡ�ļĴ�����ַ��
@param[out] buf �������ݵĵ�ַ��
@param[in] len �������ݵĳ��ȣ��������ֽ�Ϊ��λ����
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_read16(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief ÿ�δӼĴ�����ȡ�ĸ��ֽڡ�
@param[in] addr ��Ҫ��ȡ�ļĴ�����ַ��
@param[out] buf �������ݵĵ�ַ��
@param[in] len �������ݵĳ��ȣ����ĸ��ֽ�Ϊ��λ����
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_read32(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief ÿ�����Ĵ���дһ���ֽڡ�
@param[in] addr ��Ҫд�ļĴ�����ַ��
@param[in] buf ���Ĵ���д�����ݵ�ַ��
@param[in] len ���Ĵ���д�����ݳ��ȣ���һ���ֽ�Ϊ��λ����
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_write8(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief ÿ�����Ĵ���д�����ֽڡ�
@param[in] addr ��Ҫд�ļĴ�����ַ��
@param[in] buf ���Ĵ���д�����ݵ�ַ��
@param[in] len ���Ĵ���д�����ݳ��ȣ��������ֽ�Ϊ��λ����
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_write16(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief ÿ�����Ĵ���д�ĸ��ֽڡ�
@param[in] addr ��Ҫд�ļĴ�����ַ��
@param[in] buf ���Ĵ���д�����ݵ�ַ��
@param[in] len ���Ĵ���д�����ݳ��ȣ����ĸ��ֽ�Ϊ��λ����
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_write32(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief ���ں˵�ַ��ȡ����
@param[in] from ��Ҫ��ȡ���ں˵�ַ��
@param[out] to �������ݵĵ�ַ��
@param[in] len �������ݵĳ��ȣ����ֽ�Ϊ��λ��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 ali_soc_read(UINT8 * to, UINT8 * from, UINT32 len);


/*!
@brief ���ں˵�ַд���ݡ�
@param[in] to ��Ҫд���ݵ��ں˵�ַ��
@param[in] from ���ݵ�ַ��
@param[in] len ���ݳ��ȣ����ֽ�Ϊ��λ��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 ali_soc_write(UINT8 * to, UINT8 * from, UINT32 len);


/*!
@brief ÿ�δ���Χ�豸�ڴ��ַ��ȡ�ĸ��ֽڡ�
@param[in] addr ��Ҫ��ȡ����Χ�豸�ڴ��ַ��
@param[out] buf �������ݵĵ�ַ��
@param[in] len �������ݵĳ��ȣ����ĸ��ֽ�Ϊ��λ����
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_per_read32(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief ÿ������Χ�豸�ڴ��ַд�ĸ��ֽڡ�
@param[in] addr ��Ҫд����Χ�豸�ڴ��ַ��
@param[in] buf ����Χ�豸�ڴ�д�����ݵ�ַ��
@param[in] len ����Χ�豸�ڴ�д�����ݳ��ȣ����ĸ��ֽ�Ϊ��λ��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_per_write32(UINT32 addr, UINT8 *buf, UINT32 len);

/*!
@brief ���ϵͳ�ڴ�ӳ���
@param[out] smm ��õ�ϵͳ�ڴ�ӳ����ַ��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_get_memory_map(struct soc_memory_map *smm);

/*!
@brief ����ϵͳ��Ϣģ��ĵ��Եȼ���
@param[in] level ��Ҫ���õĵ��Եȼ���
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_set_level(UINT32 level);

/*!
@brief ���see cpu�İ汾��
@param[out] buf ��õİ汾��ַ��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_get_see_ver(UINT8 *buf);

/*!
@brief ����see��ӡ��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_hit_see_heart(void);

/*!
@brief ʹ��see�쳣��ӡ��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_enable_see_exception(void);


/*!
@brief ��ȡ������ص������ڴ档
@param[out] buf_addr�ڴ��ַ ��
@param[out] buf_size�ڴ��С��
*/
void soc_get_reserved_men_buf(UINT32 *buf_addr, UINT32 *buf_size);

/*!
@brief ��ȡmedia player ��ص������ڴ档
@param[out] buf_addr�ڴ��ַ ��
@param[out] buf_size�ڴ��С��
*/
void soc_get_media_buf(UINT32 *buf_addr, UINT32 *buf_size);

/*!
@brief �򿪻��߹ر�see cpu��printf���
@param[in] enable��־��Ϣ��0 -- �رգ�1 -- �򿪡�
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_set_see_printf(INT32 enable);

/*!
@brief ��ȡϵͳ�Ĺ����ڴ�ռ䡣
@param[out] buf_addr�ڴ����͵�ַ ��
@param[out] buf_addr�ڴ��ַ ��
@param[out] buf_size�ڴ��С��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_get_share_mem(enum SOC_SYS_SHM type, void **buf_addr, INT32 *buf_size);

/*!
@brief �رղ���Ҫ��IP clk/dac ,����HDMI DONGLE��ʡ���ġ�
@param[in] power_down �ò�����ʱû��ʹ�á�
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 soc_enable_power_down(INT32 power_down);	

/*!
@brief ��ʾsee �����Ϣ��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
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
