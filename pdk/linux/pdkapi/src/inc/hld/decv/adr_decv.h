/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: decv.h
 *
 *  Description: head file for video decoder device management.
 *
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2005.10.21  Rachel		     0.N.000   Support VDEC27_SUPPORT_HW_COMSUME_RUBBISH
 *  2.  2006.5.16    Rachel				   	Add IO control to support return several free buffer instead of only one label"rachel:support_return_multi_freebuffer"
 ****************************************************************************/
#ifndef  __ADR_DECV_H_
#define  __ADR_DECV_H_

/*! @addtogroup decv
 *  @{
*/
 
#include <adr_basic_types.h>
#include <adr_mediatypes.h>
#include <alidefinition/adf_decv.h>

#ifdef __cplusplus
extern "C" {
#endif
 


/*!
@brief ��ʼ��vdec ģ�顣
*/
void HLD_vdec_attach(void);

/*!
@brief �ͷ�vdec ģ�顣
*/
void HLD_vdec_detach(void);


/*!
@brief ��鵱ǰ���е���Ƶ���������豸ָ�롣
@return struct vdec_device * ��
@retval  !NULL ��õ�vdec ģ��ָ�롣 
@retval  NULL ���� ʧ�ܣ����������״̬����
*/
struct vdec_device * get_current_decoder_device(void);


/*!
@}
*/

#ifdef __cplusplus
}
#endif
#endif  /* _DECV_H_*/

