#ifndef	__ADR_OSDDRV_H_
#define	__ADR_OSDDRV_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <adr_basic_types.h>
#include <hld/adr_hld_dev.h>
#include <alidefinition/adf_osd.h>

/*! @addtogroup osd
 *  @{
 */

/*! @struct osd_device
@brief osd�豸���Ͷ��塣
*/
struct osd_device
{
	struct osd_device *next;//!< �ڲ�ʹ�á�
	UINT32 type; //!< �ڲ�ʹ�á�
	INT8 name[HLD_MAX_NAME_SIZE];//!< �ڲ�ʹ�á�

	UINT32  flags;//!< �ڲ�ʹ�á�
	void	*priv;//!< �ڲ�ʹ�á�
};

/*!
@brief ��ʼ��osd ģ�顣
*/
void HLD_OSDDrv_Attach(void);

/*!
@}
*/

#ifdef __cplusplus
}
#endif

#endif

